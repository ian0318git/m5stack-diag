/* $Id: xpoint_test.c,v 1.2 2021/04/12 13:37:17 xiaolaya Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/overlord/neptune/curie_2RU/xpoint_test.c,v $
 *------------------------------------------------------------------
 *
 * xpoint_test.c - Curie2RU Crosspoint Test
 *
 * Dec. 2018, Jiajia Liu <jiajliu@cisco.com>
 *
 * Copyright (c) 2018 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <linux/types.h>
#include <linux/socket.h>
#include <sys/ioctl.h>
#include <linux/if_ether.h>
#include <linux/ip.h>
#include <linux/udp.h>
#include <net/if.h>
#include <netinet/in.h>
#include <linux/sockios.h>
#include <linux/mii.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <net/if.h>
#include <linux/if_packet.h>
#include <net/ethernet.h>

#include "defs.h"
#include "types.h"
#include "proto.h"
#include "common.h"
#include "common_utils.h"
#include "monitor.h"
#include "menu.h"
#include "nvmonvars.h"
#include "error.h"
#include "router_if.h"
#include "sgmii_defs.h"
#include "ethernet.h"
#include "linux_eth.h"
#include "queryflags.h"
#include "dash_fpga.h"
#include "intel_gpio.h"
#include "curie2ru_common.h"
#include "xpoint_module.h"

#define SM1_GE1_INTF_IDX    9
#define SM2_GE1_INTF_IDX    8
#define SM1_GE1_INTF    (1 << 9)
#define SM2_GE1_INTF    (1 << 8)
#define XPOINT_PING_COUNT   30

#define RECVTIMEOUT    1000
#define MAXPACKETSZ    1522

#define CPU_GPP_G_COMMUNITY     5
#define CPU_GPP_G20             20
#define CPU_GPP_G21             21
#define XPOINT_SELPIN           CPU_GPP_G20

static void *xpoint_gpio;

static int mac_ping_test(int inf1, int inf2, size_t loop);

extern boolean eth_is_linkup(int port);

static int xpoint_mac_ping_test(uint32_t nic_mask)
{
    int nic = 0, i, intf[32];

    testname("Crosspoint host side loopback");

    for (i = 0; i < 32; i++) {
        if (nic_mask & (1 << i))
            intf[nic++] = i;
    }

    if (nic != 2) {
        cterr('f', 0, "invalid nic mask %#x", nic_mask);
        return FAILED;
    }

    printf("Packet test between eth%d and eth%d\n",
            intf[0], intf[1]);

    if (mac_ping_test(intf[0], intf[1], XPOINT_PING_COUNT)) {
        cterr('f', 0, "Packet test failed");
        return FAILED;
    }

    prpass(testpass, "Packet test passed, ");

    return PASSED;
}

static int xpoint_sel_pin_status(int data)
{
    if (xpoint_gpio)
        intel_gpio_dbg_show(xpoint_gpio, CPU_GPP_G20);
    return 0;
}

static int xpoint_enable_sel_pin(int enable)
{
    if (xpoint_gpio == NULL)
        return -1;

    if (enable) {
        intel_gpio_set_direction(xpoint_gpio, XPOINT_SELPIN, GPIO_OUTPUT);
        intel_gpio_set(xpoint_gpio, XPOINT_SELPIN, 1);
    } else {
        intel_gpio_set_direction(xpoint_gpio, XPOINT_SELPIN, GPIO_OUTPUT);
        intel_gpio_set(xpoint_gpio, CPU_GPP_G20, 0);
    }

    return 0;
}

#define MF_1    (MF_CONTINUOUS | MF_DOGRP)
#define MF_2    (MF_1 | MF_DOALL)
#define MF_3    (MF_2 | MF_SHOW_ERRCOUNT)
#define MF_4    (MF_1 | MF_SHOW_ERRCOUNT)

submenu_xtable_t xpoint_tests_submenu_table[] = {
    {"Crosspoint SEL pin status",
     (PFT)xpoint_sel_pin_status, 0, 0,
     NULL, 0, NULL, 0},
    {"Crosspoint Enable SEL",
     (PFT)xpoint_enable_sel_pin, 1, 0,
     NULL, 0, NULL, 0},
    {"Crosspoint Disable SEL",
     (PFT)xpoint_enable_sel_pin, 0, 0,
     NULL, 0, NULL, 0},
    {"Crosspoint host side loopback test",
     (PFT)xpoint_mac_ping_test, (SM1_GE1_INTF | SM2_GE1_INTF),
     MF_3, NULL, 0, NULL, 0},
    {"Crosspoint module side test",
     (PFT)xpoint_module_prbs_test, TRUE,
     MF_2, NULL, 0,
     (PFT)xpoint_module_prbs_test, FALSE},
};

#define XPOINT_TESTS_SUBMENU_TABLE_SIZE \
    (sizeof(xpoint_tests_submenu_table) / sizeof(submenu_xtable_t))

static mitem_t xpoint_tests_primary_items[XPOINT_TESTS_SUBMENU_TABLE_SIZE +
                                          MAX_BASE_ITEMS];
static mitem_t xpoint_tests_secondary_items[XPOINT_TESTS_SUBMENU_TABLE_SIZE +
                                            MAX_BASE_ITEMS];

menuinfo_t xpoint_subtest_menu = {
    "Crosspoint Subtest Menu",
    0,                                /* mtparam added by init_empty_menu */
    (PFT)show_endnote,                /* notes missing WICs in combos */
    0,                                /* use generic prompt */
    0,                                /* size (bumped by add_menu_item() */
    xpoint_tests_primary_items,
};
menuinfo_t *xpoint_submenup = &xpoint_subtest_menu;

/* enable/disable crosspoint */
static int xpoint_sel_enable(int enable)
{
    void *gpio;

    if (enable) {
        gpio = intel_gpio_ioremap(CPU_GPP_G_COMMUNITY);
        if (!gpio)
            return -1;

        xpoint_gpio = gpio;
        intel_gpio_dbg_show(gpio, XPOINT_SELPIN);

        intel_gpio_request_enable(gpio, XPOINT_SELPIN);
        intel_gpio_set_direction(gpio, XPOINT_SELPIN, GPIO_OUTPUT);
        intel_gpio_set(gpio, XPOINT_SELPIN, 1);

        intel_gpio_dbg_show(gpio, XPOINT_SELPIN);
    } else if (xpoint_gpio) {
        gpio = xpoint_gpio;
        intel_gpio_set(gpio, XPOINT_SELPIN, 0);
        intel_gpio_iounmap(gpio);
        xpoint_gpio = NULL;
    }

    return 0;
}

static void ifconfig_eth_up(int port)
{
    char cmd[64] = {0,};
    sprintf(cmd, "ifconfig eth%d up", port);
    system(cmd);
}

int xpoint_wait_link_up(void)
{
    int i;

    ifconfig_eth_up(SM1_GE1_INTF_IDX);
    ifconfig_eth_up(SM2_GE1_INTF_IDX);

    for (i = 0; i < 8; i++) {
        msleep(500);
        if (eth_is_linkup(SM1_GE1_INTF_IDX) &&
            eth_is_linkup(SM2_GE1_INTF_IDX)) {
            printf("eth%d and eth%d are linking up\n", SM1_GE1_INTF_IDX,
                    SM2_GE1_INTF_IDX);
            return TRUE;
        }
    }

    log_err("eth%d and eth%d have no link", SM1_GE1_INTF_IDX, SM2_GE1_INTF_IDX);

    return FALSE;
}

static void xpoint_remove_from_br0(void)
{
    char cmd[256];

    snprintf(cmd, sizeof(cmd), "brctl delif br0 eth%d", SM1_GE1_INTF_IDX);
    system(cmd);
    snprintf(cmd, sizeof(cmd), "brctl delif br0 eth%d", SM2_GE1_INTF_IDX);
    system(cmd);
}

static void xpoint_add_to_br0(void)
{
    char cmd[256];
    snprintf(cmd, sizeof(cmd), "brctl addif br0 eth%d", SM1_GE1_INTF_IDX);
    system(cmd);
    snprintf(cmd, sizeof(cmd), "brctl addif br0 eth%d", SM2_GE1_INTF_IDX);
    system(cmd);
}

/* entry for crosspoint tests and utilities */
int xpoint_test(boolean test_items_executed)
{
    xpoint_remove_from_br0();
    xpoint_sel_enable(1);
    xpoint_wait_link_up();

    build_primary_submenu(xpoint_tests_submenu_table, XPOINT_TESTS_SUBMENU_TABLE_SIZE,
                          "Crosspoint", &xpoint_submenup);
    build_secondary_submenu(xpoint_tests_submenu_table, XPOINT_TESTS_SUBMENU_TABLE_SIZE,
                            xpoint_tests_secondary_items);

    if (!test_items_executed)
        menu(&xpoint_subtest_menu, xpoint_tests_secondary_items, '\0');
    else
        exec_doall_menu_items(&xpoint_subtest_menu);

    xpoint_sel_enable(0);
    xpoint_add_to_br0();

    return PASSED;
}

static int create_packet_socket(int ether, uint8_t *mac_addr)
{
    int fd;
    struct ifreq ifr;
    struct timeval timev;
    struct sockaddr_ll sll;

    fd = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
    if (fd == -1) {
        log_err("error: socket: %s\n", strerror(errno));
        return -errno;
    }

    memset(&ifr, 0, sizeof(ifr));
    snprintf(ifr.ifr_name, sizeof(ifr.ifr_name), "eth%d", ether);

    if (ioctl(fd, SIOCGIFHWADDR, &ifr) < 0) {
        log_err("error: ioctl SIOCGIFHWADDR: %s\n", strerror(errno));
        return -errno;
    }

    if (mac_addr)
        memcpy(mac_addr, ifr.ifr_hwaddr.sa_data, 6);

    if (ioctl(fd, SIOCGIFINDEX, &ifr) < 0) {
        log_err("error: ioctl SIOCGIFINDEX: %s\n", strerror(errno));
        return -errno;
    }

    memset(&sll, 0, sizeof(sll));
    sll.sll_family = AF_PACKET;
    sll.sll_protocol = htons(ETH_P_ALL);
    sll.sll_ifindex = ifr.ifr_ifindex;

    if (bind(fd, (struct sockaddr *)&sll, sizeof(sll)) < 0) {
        log_err("error: bind: %s\n", strerror(errno));
        return -errno;
    }

    if (ioctl(fd, SIOCGIFFLAGS, &ifr) < 0) {
        log_err("error: ioctl SIOCGIFFLAGS: %s", strerror(errno));
        return -errno;
    }

    ifr.ifr_flags |= IFF_PROMISC;
    if (ioctl(fd, SIOCSIFFLAGS, &ifr) < 0) {
        log_err("error: set promisc mode: %s\n", strerror(errno));
        return -errno;
    }

    timev.tv_sec = 0;
    timev.tv_usec = 300000;
    if (setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &timev, sizeof(timev)) < 0) {
        log_err("error: setsockopt SO_RCVTIMO: %s\n", strerror(errno));
        return -errno;
    }

    return fd;
}

static int release_packet_socket(int ether, int fd)
{
    struct ifreq ifr;

    memset(&ifr, 0, sizeof(ifr));
    snprintf(ifr.ifr_name, sizeof(ifr.ifr_name), "eth%d", ether);

    if (ioctl(fd, SIOCGIFFLAGS, &ifr) < 0) {
        log_err("error: eth%d ioctl SIOCGIFFLAGS: %s\n", ether, strerror(errno));
        goto err;
    }

    ifr.ifr_flags &= ~IFF_PROMISC;
    if (ioctl(fd, SIOCSIFFLAGS, &ifr) < 0) {
        log_err("error: eth%d cancel promisc: %s\n", ether, strerror(errno));
        goto err;
    }

err:
    close(fd);

    return -errno;
}

/* simple packet test for host side crosspoint */
static int mac_ping(int txfd, uint8_t *macaddr, int rxfd, size_t pktsize)
{
    size_t i;
    int c = 0;
    size_t count = 0;
    ssize_t len;
    uint8_t buffer[2048];
    uint8_t packet[2048];
    uint8_t dsthwaddr[6] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };

    if (pktsize < 64 || pktsize > MAXPACKETSZ)
        return -EINVAL;

    memcpy(packet, dsthwaddr, 6);
    memcpy(packet + 6, macaddr, 6);

    for (i = 12; i < pktsize; i++)
        packet[i] = i;

    len = write(txfd, packet, pktsize);
    if (len != pktsize) {
        if (len < 0) {
            log_err("error: write: %s\n", strerror(errno));
            return -errno;
        }
        log_err("error: write: unexpected return %zd\n", len);
        return -ENOBUFS;
    }

    for (;;) {
        c++;
        len = read(rxfd, buffer, sizeof(buffer));
        if (len < 0) {
            log_err("error: read socket %d: %s\n", rxfd, strerror(errno));
            log_err("Failed at loop %d, received %zd bytes data\n", c, count);
            return -errno;
        }

        count += len;
        if (len != pktsize) {
            if ((NVRAM)->diagflag & D_VERBOSE) {
                log_warn("warning: %dth: received %zd bytes\n", c, len);
                curie2ru_hex_dump(buffer, pktsize, 0);
            }
            continue;
        }

        if (memcmp(buffer, dsthwaddr, 6) != 0) {
            if ((NVRAM)->diagflag & D_VERBOSE) {
                log_err("error: %dth: invalid dest mac address\n", c);
                curie2ru_hex_dump(buffer, 6, 0);
            }
            continue;
        }

        if (memcmp(buffer + 6, macaddr, 6) != 0) {
            if ((NVRAM)->diagflag & D_VERBOSE) {
                log_err("error: %dth: invalid source mac address\n", c);
                curie2ru_hex_dump(buffer + 6, 6, 0);
            }
            continue;
        }

        for (i = 12; i < pktsize; i++)
            if (buffer[i] != (i & 0xff))
                break;

        if (i == pktsize)
            break;


        if ((NVRAM)->diagflag & D_VERBOSE) {
            log_err("error: %dth: invalid data at %zu\n", c, i);
            curie2ru_hex_dump(buffer, pktsize, 0);
        }
    }

    return 0;
}

#define DFT_MAC_PING_PKTSIZE    512

static int mac_ping_test(int inf1, int inf2, size_t loop)
{
    size_t i;
    int fd1, fd2, err = 0;
    uint8_t mac1[6], mac2[6];

    fd1 = create_packet_socket(inf1, mac1);
    if (fd1 < 0) {
        log_err("error: eth%d: failed to create packet socket\n", inf1);
        return -fd1;
    }

    if (inf1 != inf2)
        fd2 = create_packet_socket(inf2, mac2);
    else
        fd2 = fd1;

    if (fd2 < 0) {
        log_err("error: eth%d: failed to create packet socket\n", inf2);
        release_packet_socket(inf1, fd1);
        return -fd2;
    }

    for (i = 0; i < loop; i++) {
        err = mac_ping(fd1, mac1, fd2, DFT_MAC_PING_PKTSIZE);
        if (err != 0) {
            log_err("error: eth%d -> eth%d failed at loop %zu\n", inf1, inf2, i);
            break;
        }

        err = mac_ping(fd2, mac2, fd1, DFT_MAC_PING_PKTSIZE);
        if (err != 0) {
            log_err("error: eth%d -> eth%d failed at loop %zu\n", inf2, inf1, i);
            break;
        }
    }

    release_packet_socket(inf1, fd1);
    release_packet_socket(inf2, fd2);

    return 0 - err;
}

/*
 *-----------------------------------------------------------------------------
$Log: xpoint_test.c,v $
Revision 1.2  2021/04/12 13:37:17  xiaolaya
*** empty log message ***

Revision 1.1  2020/01/09 01:02:07  jiajliu
Merge Curie 2RU to main trunk

$Endlog$
 *-----------------------------------------------------------------------------
 */
