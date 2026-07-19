/* $Id: curie2ru_eth_traf_utility.c,v 1.1 2020/01/09 01:01:57 jiajliu Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/overlord/neptune/curie_2RU/curie2ru_eth_traf_utility.c,v $
 *------------------------------------------------------------------
 *
 * curie2ru_eth_traf_utility.c - Curie2ru traf utility 
 *
 * Sep. 2018, Nocken Zou <yozou@cisco.com>
 *
 * Copyright (c) 2018 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/time.h>

#include "common.h"
#include "types.h"
#include "defs.h"
#include "menu.h"
#include "queryflags.h"
#include "dev_ltc4215.h"
#include "oir_ltc4215_api.h"
#include "curie2ru.h"
#include "eth_traf.h"

struct task_list {
    struct eth_task *task;
    struct task_list *next;
};

struct intf_node {
    int port_num;
    struct timeval start;
    uint64_t tx_size;
    uint64_t rx_size;
    struct eth_traf *eth_intf;
    struct task_list *tx_task;
    struct task_list *rx_task;
    struct intf_node *next;
};

static struct intf_node *eth_list = NULL;

static struct intf_node * curie2ru_eth_search(int port)
{
    struct intf_node *eth_tmp = NULL;
    struct intf_node *eth_intf = NULL;

    if (eth_list == NULL) {
        return NULL;
    }

    eth_tmp = eth_list;
    while (eth_tmp != NULL) {
        if (eth_tmp->port_num == port) {
            eth_intf = eth_tmp;
            break;
        }
        eth_tmp = eth_tmp->next;
    }
    return eth_intf;
}

static struct intf_node * curie2ru_eth_intf_create(void)
{
    struct intf_node *eth_intf = NULL;
    eth_intf = (struct intf_node *)malloc(sizeof(struct intf_node));
    if (eth_intf == NULL) {
        perror("malloc err\n");
        return NULL;
    }
    eth_intf->eth_intf = NULL;
    eth_intf->tx_size = 0;
    eth_intf->rx_size = 0;
    eth_intf->tx_task = NULL;
    eth_intf->rx_task = NULL;
    eth_intf->next = NULL;
    return eth_intf;
}

static long curie2ru_eth_traf_init(void)
{
    int port;
    char iface[64];
    struct intf_node *eth_intf = NULL;
    struct intf_node *eth_tmp = NULL;

    port = getdec_answer("Enter port number(0, 11)", 0, 0, 11);
    eth_intf = curie2ru_eth_search(port);
    if (eth_intf == NULL) {
        eth_tmp = curie2ru_eth_intf_create();
        if (eth_tmp == NULL) {
            cterr('f', 0, "eth intf create failed\n");
            return FAILED;
        }
        eth_tmp->port_num = port;
        sprintf(iface, "eth%d", port);
        eth_tmp->eth_intf = eth_traf_init(iface, 256);
        if (eth_list == NULL) {
            eth_list = eth_tmp;
        } else {
            eth_tmp->next = eth_list->next;
            eth_list->next = eth_tmp;
        }
    } else {
        prt("eth%d has already been initialized\n", port);
        return PASSED;
    }
    return PASSED;
}

static void unit_str(uint64_t count, char *ptr)
{
	const char *fmts[] = {"%llu", "%lluK", "%lluM", "%lluG"};
	const char **fmt = fmts;

	while (count / 1000) {
		count /= 1000;
		fmt++;
	}

	sprintf(ptr, *fmt, count);
}

static int curie2ru_eth_traf_tx_start(struct eth_traf_tx_task_settings tx_settings, struct intf_node *tx_eth_intf, int if_using_source_mac)
{
    struct task_list *new_tx_task = NULL;
    new_tx_task = (struct task_list *)malloc(sizeof(struct task_list));
    new_tx_task->next = NULL;
    if (if_using_source_mac) {
        new_tx_task->task = eth_traf_tx_task_start_using_source_mac(&tx_settings);
    } else {
        new_tx_task->task = eth_traf_tx_task_start(&tx_settings);
    }
    if (new_tx_task->task == NULL) {
        prt("warn: tx task start failed\n");
        return -1;
    }

    if (tx_eth_intf->tx_task == NULL) {
        tx_eth_intf->tx_task = new_tx_task;
    } else {
        new_tx_task->next = tx_eth_intf->tx_task->next;
        tx_eth_intf->tx_task->next = new_tx_task;
    }
    return 0;
}

static void curie2ru_eth_traf_tx_stop(struct intf_node *tx_eth_intf)
{
    struct task_list *task_tmp = NULL;

    if (tx_eth_intf->tx_task != NULL) {
        while (tx_eth_intf->tx_task->next != NULL) {
            task_tmp = tx_eth_intf->tx_task->next;
            tx_eth_intf->tx_task->next = task_tmp->next;
            eth_traf_tx_stop(task_tmp->task);
            free(task_tmp);
        }
        eth_traf_tx_stop(tx_eth_intf->tx_task->task);
        free(tx_eth_intf->tx_task);
        tx_eth_intf->tx_task = NULL;
    }
}


static int curie2ru_eth_traf_rx_start(struct eth_traf_rx_task_settings rx_settings, struct intf_node *rx_eth_intf)
{
    struct task_list *new_rx_task = NULL;
    new_rx_task = (struct task_list *)malloc(sizeof(struct task_list));
    new_rx_task->next = NULL;
    new_rx_task->task = eth_traf_rx_task_start(&rx_settings);
    if (new_rx_task->task == NULL) {
        prt("warn: rx task start failed\n");
        return -1;
    }
    if (rx_eth_intf->rx_task == NULL) {
        rx_eth_intf->rx_task = new_rx_task;
    } else {
        new_rx_task->next = rx_eth_intf->rx_task->next;
        rx_eth_intf->rx_task->next = new_rx_task;
    }
    return 0;
}

static void curie2ru_eth_traf_rx_stop(struct intf_node *rx_eth_intf)
{
    struct task_list *task_tmp = NULL;

    if (rx_eth_intf->rx_task != NULL) {
        while (rx_eth_intf->rx_task->next != NULL) {
            task_tmp = rx_eth_intf->rx_task->next;
            rx_eth_intf->rx_task->next = task_tmp->next;
            eth_traf_rx_stop(task_tmp->task);
            free(task_tmp);
        }
        eth_traf_rx_stop(rx_eth_intf->rx_task->task);
        free(rx_eth_intf->rx_task);
        rx_eth_intf->rx_task = NULL;
    }
}


static long curie2ru_eth_traf_stop(void)
{
    uint64_t tx_size = 0, rx_size = 0;
    int tx_speed = 0, rx_speed = 0, tx_port, rx_port;
    char tx_sptr[1024], rx_sptr[1024];
    struct eth_traf_statistic_info tx_info, rx_info;
    struct intf_node *tx_eth_intf = NULL, *rx_eth_intf = NULL;
	struct timeval end;

    tx_port = getdec_answer("Enter tx port number(0, 11)", 0, 0, 11);
    rx_port = getdec_answer("Enter rx port number(0, 11)", 0, 0, 11);
    tx_eth_intf = curie2ru_eth_search(tx_port);
    rx_eth_intf = curie2ru_eth_search(rx_port);

    if (tx_eth_intf != NULL) {
        curie2ru_eth_traf_tx_stop(tx_eth_intf);
    }

    curie2ru_mdelay(1);
    if (rx_eth_intf != NULL) {
        curie2ru_eth_traf_rx_stop(rx_eth_intf);
    }

    gettimeofday(&end, NULL);

    if (tx_eth_intf != NULL) {
        eth_traf_statistic(tx_eth_intf->eth_intf, &tx_info, 1);
        tx_size = tx_info.tx_size - tx_eth_intf->tx_size;
        tx_speed = tx_size / ((end.tv_sec - tx_eth_intf->start.tv_sec) ? : 1);
        tx_eth_intf->tx_size = tx_info.tx_size;
    }
    if (rx_eth_intf != NULL) {
        eth_traf_statistic(rx_eth_intf->eth_intf, &rx_info, 1);
        rx_size = rx_info.rx_size - rx_eth_intf->rx_size;
        rx_speed = rx_size / ((end.tv_sec - rx_eth_intf->start.tv_sec) ? : 1);
        rx_eth_intf->rx_size = rx_info.rx_size;
    }
    unit_str(tx_speed, tx_sptr);
    unit_str(rx_speed, rx_sptr);
    printf("send Speed %sB/s, recv speed %sB/s\n", tx_sptr, rx_sptr);
    return PASSED;
}

static struct intf_node * curie2ru_eth_tx_setting(struct eth_traf_tx_task_settings *tx_settings)
{
    int tx_port, mode, check, pkt_len, burst, delay;
    struct intf_node *tx_eth_intf = NULL;

    if (eth_list == NULL) {
        prt("warn: not any port initialized, please init first\n");
        return NULL;
    }

    tx_port = getdec_answer("Enter tx port number(0, 11)", 0, 0, 11);

    mode = getdec_answer("Enter tx mode(FIXED:0, RANDOM:1)", 0, 0, 1);
    check = getdec_answer("add check bit(NO:0, YES:1)", 0, 0, 1);
    pkt_len = getdec_answer("Enter pkt len(64, 1500)", 1500, 64, 1500);
    burst = getdec_answer("Enter burst(1, 15000)", 100, 1, 15000);
    delay = getdec_answer("Enter delay between burst(0, 15000)", 0, 0, 1500);

    if (mode)
        tx_settings->mode = ETH_TRAF_TX_MODE_RADOM;
    else
        tx_settings->mode = ETH_TRAF_TX_MODE_FIXED;
    if (check)
        tx_settings->check = ETH_TRAF_TX_CHECK_BIT_ADD_YES;
    else
        tx_settings->check = ETH_TRAF_TX_CHECK_BIT_ADD_NO;

    tx_settings->len = pkt_len;
    tx_settings->burst = burst;
    tx_settings->interval = delay;

    tx_eth_intf = curie2ru_eth_search(tx_port);
    if (tx_eth_intf == NULL) {
        prt("warn: eth%d has not been initialized\n", tx_port);
        return NULL;
    }
    tx_settings->traf = tx_eth_intf->eth_intf;

    return tx_eth_intf;
}

static struct intf_node * curie2ru_eth_rx_setting(struct eth_traf_rx_task_settings *rx_settings)
{
    int rx_port, chk_mode;
    struct intf_node *rx_eth_intf = NULL;

    if (eth_list == NULL) {
        prt("warn: not any port initialized, please init first\n");
        return NULL;
    }

    rx_port = getdec_answer("Enter rx port number(0, 11)", 0, 0, 11);
    chk_mode = getdec_answer("Enter rx check mode(check bit: 0, check len: 1 )", 1, 0, 1);
    if (chk_mode) {
        rx_settings->chk_mode = ETH_TRAF_RX_MODE_CHECK_LEN;
    } else {
        rx_settings->chk_mode = ETH_TRAF_RX_MODE_CHECK_BIT;
    }

    rx_eth_intf = curie2ru_eth_search(rx_port);
    if (rx_eth_intf == NULL) {
        prt("warn: eth%d has not been initialized\n", rx_port);
        return NULL;
    }
    rx_settings->traf = rx_eth_intf->eth_intf;
    return rx_eth_intf;
}

static long curie2ru_eth_traf_start(void)
{
    int i, tx_task_count = 0, rx_task_count = 0, tx_enable, rx_enable;
    int if_using_source_mac;
    struct eth_traf_tx_task_settings tx_settings;
    struct eth_traf_rx_task_settings rx_settings;
    struct intf_node *tx_eth_intf = NULL;
    struct intf_node *rx_eth_intf = NULL;

    tx_enable = getdec_answer("Enable tx(Disable: 0, Enable: 1)", 1, 0, 1);
    rx_enable = getdec_answer("Enable rx(Disable: 0, Enable: 1)", 1, 0, 1);

    if (tx_enable) {
        tx_task_count = getdec_answer("tx task count(1 - 10)", 1, 1, 10);
        if_using_source_mac = getdec_answer("using source mac addr(1 : YES, 0 : NO)", 1, 0, 1);
        tx_eth_intf = curie2ru_eth_tx_setting(&tx_settings);
        if (tx_eth_intf == NULL) {
            cterr('f', 0, "eth tx setting failed\n");
            return FAILED;
        }
    }

    if (rx_enable) {
        rx_task_count = getdec_answer("rx task count (1 - 10)", 2, 1, 10);
        rx_eth_intf = curie2ru_eth_rx_setting(&rx_settings);
        if (rx_eth_intf == NULL) {
            cterr('f', 0, "eth rx setting failed\n");
            return FAILED;
        }
    }

    if (rx_enable) {
        if (rx_eth_intf->rx_task == NULL) {
	        gettimeofday(&rx_eth_intf->start, NULL);
        }
    }
    if (tx_enable) {
        if (tx_eth_intf->tx_task == NULL) {
	        gettimeofday(&tx_eth_intf->start, NULL);
        }
    }
    for (i = 0; i < rx_task_count; i++) {
        if (curie2ru_eth_traf_rx_start(rx_settings, rx_eth_intf)) {
            prt("warn: rx start failed\n");
        }
    }

    curie2ru_mdelay(1);
    for (i = 0; i < tx_task_count; i++) {
        if (curie2ru_eth_traf_tx_start(tx_settings, tx_eth_intf, if_using_source_mac)) {
            prt("warn: tx start failed\n");
        }
    }
    return PASSED;
}

static long curie2ru_eth_traf_exit(void)
{
    struct intf_node *eth_tmp = NULL;

    if (eth_list != NULL) {
        eth_tmp = eth_list->next;
        while (eth_tmp != NULL) {
            eth_list->next = eth_tmp->next;
            curie2ru_eth_traf_tx_stop(eth_tmp);
            curie2ru_eth_traf_rx_stop(eth_tmp);
            eth_traf_exit(eth_tmp->eth_intf);
            free(eth_tmp);
            eth_tmp = eth_list->next;
        }
        curie2ru_eth_traf_tx_stop(eth_list);
        curie2ru_eth_traf_rx_stop(eth_list);
        free(eth_list);
        eth_list = NULL;
    }
    return PASSED;
}


static submenu_xtable_t eth_traf_submenu_table[] = {
    {"ETH Traf Init", curie2ru_eth_traf_init, 0,
     0, NULL, 0, NULL, 0},
    {"ETH Traf Task Start", curie2ru_eth_traf_start, 0,
     0, NULL, 0, NULL, 0},
    {"ETH Traf Task Stop", curie2ru_eth_traf_stop, 0,
     0, NULL, 0, NULL, 0},
    {"ETH Traf Exit", curie2ru_eth_traf_exit, 0,
     0, NULL, 0, NULL, 0},
};

#define ETH_TRAF_SUBMENU_TABLE_SZ (sizeof(eth_traf_submenu_table) / sizeof(submenu_xtable_t))

static mitem_t eth_traf_submenu_primary_items[ETH_TRAF_SUBMENU_TABLE_SZ+MAX_BASE_ITEMS];
static mitem_t eth_traf_submenu_secondary_items[ETH_TRAF_SUBMENU_TABLE_SZ+MAX_BASE_ITEMS];
static char eth_traf_submenu_title[] = "ETH TRAF Subutility Menu";

static menuinfo_t eth_traf_submenu = {
    eth_traf_submenu_title,
    0,                          /* mtparam added by init_empty_menu */
    (PFT)show_endnote,          /* notes missing WICs in combos */
    0,                          /* use generic prompt */
    0,                          /* size (bumped by add_menu_item() */
    eth_traf_submenu_primary_items,
};

static menuinfo_t *eth_traf_submenup = &eth_traf_submenu;

int curie2ru_eth_traf_utility(void)
{
    build_primary_submenu(eth_traf_submenu_table, ETH_TRAF_SUBMENU_TABLE_SZ,
                          eth_traf_submenu_title, &eth_traf_submenup);
    build_secondary_submenu(eth_traf_submenu_table, ETH_TRAF_SUBMENU_TABLE_SZ,
                            eth_traf_submenu_secondary_items);
    menu(eth_traf_submenup, eth_traf_submenu_secondary_items, '\0');
    return PASSED;
}

/*
 *-----------------------------------------------------------------------------
$Log: curie2ru_eth_traf_utility.c,v $
Revision 1.1  2020/01/09 01:01:57  jiajliu
Merge Curie 2RU to main trunk

$Endlog$
 *-----------------------------------------------------------------------------
 */
