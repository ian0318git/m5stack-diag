/* $Id: i350_test.c,v 1.1 2020/01/09 01:01:59 jiajliu Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/overlord/neptune/curie_2RU/i350_test.c,v $
 *-----------------------------------------------------------------------------
 * i350_test.c - Diag Test for Intel I350.
 *
 * July 2018, Leschen 
 *
 * Copyright (c) 2018 by Cisco Systems, Inc.
 * All rights reserved.
 *-----------------------------------------------------------------------------
 */
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
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>

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
#include "queryflags.h" /* for query user functions */  
#include "i350_test.h"
#include "dash_fpga.h"
#include "curie2ru_common.h"
#include "eth_traf.h"

#define MF_1    (MF_CONTINUOUS | MF_DOGRP)
#define MF_2    (MF_1 | MF_DOALL)
#define MF_3    (MF_2 | MF_SHOW_ERRCOUNT)
#define MF_4    (MF_1 | MF_SHOW_ERRCOUNT)

static int I350_utility(int);
static void I350_fw_download(void);
static int i350_port_link_status_detect(const char *iface);
static int i350_is_download_port(const char *iface);
static int i350_eth_traf_external_test(int port);
int i350_check_result(int, char*);
int i350_register_test(int);
int i350_internal_loopback_test(int);
int i350_external_loopback_test(int);
int i350_int_ext_loopback_test(int);
static void i350_get_nic_number(void);
static void i350_show_running_port(void);
static void i350_display_cap(void);

/******************************************************************************
 *  List of Menu used for Intel I350
 *****************************************************************************/
static submenu_xtable_t I350_tests_submenu_table[] = {
   {"I350 Port 0 Register Test", (type_t(*)())i350_register_test,   0,
     MF_3, (PFT)is_curie_1ru_4ge_port, 0, (type_t(*)())0,   0},
   {"I350 Port 1/2/3 Register Test", (type_t(*)())i350_register_test,   1,
     MF_3, (PFT)is_curie_1ru_4ge_port, 0, (type_t(*)())0,   0},
   {"I350 Port 0 Int/External Loopback Test", (type_t(*)())i350_int_ext_loopback_test,   0,
     MF_3, (PFT)is_curie_1ru_4ge_port, 0, (type_t(*)())0,   0},
   {"I350 Port 1/2/3 Int/External Loopback Test", (type_t(*)())i350_int_ext_loopback_test,   1,
     MF_3, (PFT)is_curie_1ru_4ge_port, 0, (type_t(*)())0,   0},
   {"I350 Utility", (type_t(*)())I350_utility,   FALSE,
    0, NULL, 0, (type_t(*)())I350_utility,   TRUE},

   {"I350 Port 0 Register Test", (type_t(*)())i350_register_test,   0,
     MF_3, (PFT)is_curie_2ru, 0, (type_t(*)())0,   0},
   {"I350 Port 1/2/3 Register Test", (type_t(*)())i350_register_test,   1,
     MF_3, (PFT)is_curie_2ru, 0, (type_t(*)())0,   0},
   {"I350 Port 0 Int/External Loopback Test", (type_t(*)())i350_int_ext_loopback_test,   0,
     MF_3, (PFT)is_curie_2ru, 0, (type_t(*)())0,   0},
   {"I350 Port 1/2/3 Int/External Loopback Test", (type_t(*)())i350_int_ext_loopback_test,   1,
     MF_3, (PFT)is_curie_2ru, 0, (type_t(*)())0,   0},
   {"I350 Port 0 traf External Loopback Test", (type_t(*)())i350_eth_traf_external_test,   0,
     MF_3, (PFT)is_curie_2ru, 0, (type_t(*)())0,   0},
   {"I350 Port 1 traf External Loopback Test", (type_t(*)())i350_eth_traf_external_test,   1,
     MF_3, (PFT)is_curie_2ru, 0, (type_t(*)())0,   0},
   {"I350 Port 2 traf External Loopback Test", (type_t(*)())i350_eth_traf_external_test,   2,
     MF_3, (PFT)is_curie_2ru, 0, (type_t(*)())0,   0},
   {"I350 Port 3 traf External Loopback Test", (type_t(*)())i350_eth_traf_external_test,   3,
     MF_3, (PFT)is_curie_2ru, 0, (type_t(*)())0,   0},
};

/******************************************************************************
 *  List of Utilities used for Intel I350
 *****************************************************************************/
static submenu_xtable_t I350_util_items[] = {
    {"I350 Firmware Download", (type_t(*)())I350_fw_download, 0, 0,
     (type_t(*)())0, 0, (type_t(*)())0,   0},
    {"Get I350 NIC Number", (type_t(*)())i350_get_nic_number, 0, 0,
     (type_t(*)())0, 0, (type_t(*)())0, 0},
    {"Display I350 ports capability", (type_t(*)())i350_display_cap, 0, 0,
     (type_t(*)())0, 0, (type_t(*)())0, 0},
    {"Display Currently Running ETH Port", (type_t(*)())i350_show_running_port, 0, 0,
     (type_t(*)())0, 0, (type_t(*)())0, 0},
    {"I350 Do Port 0 Register Test(for debug purpose)", (type_t(*)())i350_register_test, 2, 0,
     (type_t(*)())0, 0, (type_t(*)())0,   0},
    {"I350 Do Port 0 Int/External Loopback Test(for debug purpose)", (type_t(*)())i350_int_ext_loopback_test, 2, 0,
     (type_t(*)())0, 0, (type_t(*)())0,   0},
};

/******************************************************************************
 *  Macro Definitions
 *****************************************************************************/
#define I350_TESTS_SUBMENU_TABLE_SIZE (sizeof(I350_tests_submenu_table) / \
                                           sizeof(submenu_xtable_t))

#define I350_TESTS_UTIL_SIZE (sizeof(I350_util_items) / \
                                  sizeof(submenu_xtable_t))

/******************************************************************************
 *  Global Variable
 *****************************************************************************/
/******************************************************************************
 * Primary & secondary submenu items (filled in from xtable)
 *****************************************************************************/
static mitem_t I350_tests_primary_items[I350_TESTS_SUBMENU_TABLE_SIZE +
                                            MAX_BASE_ITEMS];
static mitem_t I350_tests_secondary_items[I350_TESTS_SUBMENU_TABLE_SIZE +
                                              MAX_BASE_ITEMS];

/******************************************************************************
 * Primary & secondary utilities menu items (filled in from xtable)
 *****************************************************************************/
static mitem_t I350_tests_primary_util_items[I350_TESTS_UTIL_SIZE +
                                                 MAX_BASE_ITEMS];
static mitem_t I350_tests_secondary_util_items[I350_TESTS_UTIL_SIZE +
                                                   MAX_BASE_ITEMS];

/******************************************************************************
 *I350 Utils submenu
 *****************************************************************************/
menuinfo_t I350_util_menu = {
    "I350 Utility Menu",
    0,
    (PFT)show_endnote,
    0,
    0,
    I350_tests_primary_util_items,
};
menuinfo_t *I350_util_menup = &I350_util_menu;

menuinfo_t I350_subtest_menu = {
    "%s Subtest Menu",
    0,                                /* mtparam added by init_empty_menu */
    (PFT)show_endnote,                /* notes missing WICs in combos */
    0,                                /* use generic prompt */
    0,                                /* size (bumped by add_menu_item() */
    I350_tests_primary_items,
};
menuinfo_t *I350_submenup = &I350_subtest_menu;

/******************************************************************************
 * The menu driven diagnostics are called from either the low level
 * power-on routine or from the monitor.  Frommon will be non-zero if
 * called from the monitor.  Note that this value is actually the
 * argc parameter (a little slight of hand here), which is always
 * greater than zero.
 *
 * The main menu is now defined in an _xtable_.  Both the primary items
 * and the secondary (shadow) items are built with function calls that
 * operate on it and insert the appropriate base items into the menu.
 ******************************************************************************/
int i350_test (int show_menu)
{
    build_primary_submenu(I350_tests_submenu_table,
                          I350_TESTS_SUBMENU_TABLE_SIZE,
                          "1G NIC I350", &I350_submenup);
    build_secondary_submenu(I350_tests_submenu_table,
                            I350_TESTS_SUBMENU_TABLE_SIZE,
                            I350_tests_secondary_items);

    if (show_menu) {
        exec_doall_menu_items(I350_submenup);
    } else {
        menu(I350_submenup, I350_tests_secondary_items, '\0' );
    }

    return (PASSED);
}

/*******************************************************************************
 *
 * Function    : I350_utility
 * Description :
 * Inputs      : menu_option - display menu instead of running all I350 
 *               tests.
 * Outputs     : PASSED / FAILED
 *
 ******************************************************************************/
static int I350_utility (int show_menu)
{
    build_primary_submenu(I350_util_items, I350_TESTS_UTIL_SIZE,
                          "I350 Utilities Menu", &I350_util_menup);
    build_secondary_submenu(I350_util_items, I350_TESTS_UTIL_SIZE,
                            I350_tests_secondary_util_items);

    menu(I350_util_menup, I350_tests_secondary_util_items, '\0' );

    return (PASSED);
}

/*
 * Function: i350_get_nic_number
 *
 * Description: To get I350 four port's NIC number 
 *
 * Input: none
 *
 * Return: none
 */
static void i350_get_nic_number (void)
{
    system(I350_GET_NIC);
}

/*
 * Function: i350_show_running_port
 *
 * Description: To display all the running ETH ports 
 *              including BCM57412 and Intel ineternal 10 ports
 *
 * Input: none
 *
 * Return: none
 */
static void i350_show_running_port (void)
{
    system(I350_SHOW_RUNNING_ETH);
}

/*
 * Function: i350_display_cap
 *
 * Description: Display I350 ports capability
 *
 * Input: none
 *
 * Return: none
 */
static void i350_display_cap (void)
{
    system(DISPLAY_I350_PORT1_CAP);
    system(DISPLAY_I350_PORT2_CAP);
    system(DISPLAY_I350_PORT3_CAP);
    system(DISPLAY_I350_PORT4_CAP);
}

/******************************************************************************
 *
 * Function: I350_fw_download
 *
 * Description: This function performs the I350 firmware download and init.
 *
 * Inputs      : NONE
 *
 * Outputs     : NONE
 *
 ********************************************************************************/
static void I350_fw_download (void)
{
    system(I350_FW_PROGRAMMING);
}

/******************************************************************************
 *
 * Function: I350_check_result
 *
 * Description: This function is used to check test result
 *
 * Inputs      : port - I350 port number 
                 test_name - what kind of test be checked
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/
int i350_check_result (int port, char *test_name)
{
    char buf[128];
    FILE *fp;
    int retval = FAILED;

    memset(buf, 0, BUFFER_ARRAY_SIZE);

    printf("Checking I350 port %d %s test result\n", port, test_name);
    fp = fopen(I350_TEST_RESULT, "r");
    if (fp == NULL) {
        cterr('f',0, "Port %d can't open file i350_test_result.txt", port);
        return (retval);
    }

    while (!feof(fp)) {
        fgets(buf, sizeof(buf), fp);
        if (strstr(buf, I350_PASS) == NULL) {
            cterr('f',0, "port %d %s test failed, ", port, test_name);
            break;
        } else {
            prpass(testpass, "port %d %s test passed, ", port, test_name);
            retval = PASSED;
            break;
        }
    }

    fclose(fp);
    return (retval);
}

/******************************************************************************
 *
 * Function: I350_register_test
 *
 * Description: This function performs the register test
 *
 * Inputs      : port - I350 port number 
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/
int i350_register_test (int port)
{
    char *tname = "I350 register";
    int retval = FAILED;

    testname("%s", tname);

    if (port == I350_SKIP_PORT0) {
        printf("I350 port0 is management port so skip the register test\n");
        return (PASSED);
    } else if (port == I350_REST_PORT) {
        system(I350_PORT2_REG_TEST);
        msleep(200);
        retval = i350_check_result(I350_PORT1, tname);
        if (retval == FAILED) {
            return (retval);
        }

        system(I350_PORT3_REG_TEST);
        msleep(200);
        retval = i350_check_result(I350_PORT2, tname);
        if (retval == FAILED) {
            return (retval);
        }

        system(I350_PORT4_REG_TEST);
        msleep(200);
        retval = i350_check_result(I350_PORT3, tname);
        if (retval == FAILED) {
            return (retval);
        }
    } else if (port == DO_I350_PORT0) {
        /* Port 0 is management port and is mapping to NIC 1 */
        system(I350_PORT1_REG_TEST);
        msleep(200);
        retval = i350_check_result(I350_PORT0, tname);
        if (retval == FAILED) {
            return (retval);
        }
    } else {
        cterr('f',0, "Do not support this test");
        return (retval);
    }

    return (retval);
}

/******************************************************************************
 *
 * Function: I350_int_ext_loopback_test
 *
 * Description: This function performs the internal/external loopback test
 *
 * Inputs      : port - i350 port number 
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/
int i350_int_ext_loopback_test (int port)
{
    char *tname = "I350 int/external lpbk";
    int retval = FAILED;
    int skip_ext_lpbk = FALSE;

    testname("%s", tname);

    if ((NVRAM)->diagflag & D_EXT_LOOPBACK) {
        printf("External loopback flag is off, skip the external loopback test\n");
        skip_ext_lpbk = TRUE;
    }

    if (port == I350_SKIP_PORT0) {
        printf("I350 port0 is management port so skip the int/external lpbk test\n");
        return (PASSED);
    } else if (port == I350_REST_PORT) {
        /* Do internal loopback only */
        if (skip_ext_lpbk == TRUE) {
            retval = i350_internal_loopback_test(port);
        } else {
            /* Do internal/external loopback */
            retval = i350_internal_loopback_test(port);
            retval = i350_external_loopback_test(port);
        }
    } else if (port == DO_I350_PORT0) {
        /* Do internal loopback only */
        if (skip_ext_lpbk == TRUE) {
            retval = i350_internal_loopback_test(port);
        } else {
            /* Do internal/external loopback */
            retval = i350_internal_loopback_test(port);
            retval = i350_external_loopback_test(port);
        }
    } else {
        cterr('f',0, "Do not support this test");
        return (retval);
    }

    return (retval);
}

/******************************************************************************
 *
 * Function: I350_internal_loopback_test
 *
 * Description: This function performs the internal loopback test
 *              from CPU to I350 MAC and PHY.
 *
 * Inputs      : port - i350 port number 
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/
int i350_internal_loopback_test (int port)
{
    char *tname = "I350 internal lpbk";
    int retval = FAILED;

    if (port == I350_REST_PORT) {
        system(I350_PORT2_INT_LPBK);
        msleep(200);
        retval = i350_check_result(I350_PORT1, tname);
        if (retval == FAILED) {
            return (retval);
        }

        system(I350_PORT3_INT_LPBK);
        msleep(200);
        retval = i350_check_result(I350_PORT2, tname);
        if (retval == FAILED) {
            return (retval);
        }

        system(I350_PORT4_INT_LPBK);
        msleep(200);
        retval = i350_check_result(I350_PORT3, tname);
        if (retval == FAILED) {
            return (retval);
        }
    } else if (port == DO_I350_PORT0) {
        system(I350_PORT1_INT_LPBK);
        msleep(200);
        retval = i350_check_result(I350_PORT0, tname);
        if (retval == FAILED) {
            return (retval);
        }
    } else {
        cterr('f',0, "Do not support this test");
        return (retval);
    }

    return (retval);
}

/******************************************************************************
 *
 * Function: I350_external_loopback_test
 *
 * Description: This function performs the external loopback test from CPU to
 *              I350 rj45 module
 *
 * Inputs      : port - I350 port number 
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/
int i350_external_loopback_test (int port)
{
    char *tname = "I350 external lpbk";
    int retval = FAILED;

    if (port == I350_REST_PORT) {
        system(I350_PORT2_EXT_LPBK);
        msleep(200);
        retval = i350_check_result(I350_PORT1, tname);
        if (retval == FAILED) {
            return (retval);
        }

        system(I350_PORT3_EXT_LPBK);
        msleep(200);
        retval = i350_check_result(I350_PORT2, tname);
        if (retval == FAILED) {
            return (retval);
        }

        system(I350_PORT4_EXT_LPBK);
        msleep(200);
        retval = i350_check_result(I350_PORT3, tname);
        if (retval == FAILED) {
            return (retval);
        }
    } else if (port == DO_I350_PORT0) {
        system(I350_PORT1_EXT_LPBK);
        msleep(200);
        retval = i350_check_result(I350_PORT0, tname);
        if (retval == FAILED) {
            return (retval);
        }
    } else {
        cterr('f',0, "Do not support this test");
        return (retval);
    }

    return (retval);
}

static int i350_port_link_status_detect(const char *iface)
{
    struct ifreq ifr;
    struct ethtool_value evalue;
    int sockfd;

    evalue.cmd = ETHTOOL_GLINK;
    evalue.data = 0;
    strcpy(ifr.ifr_name, iface);
    ifr.ifr_data = (char *)&evalue;

    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd == -1) {
        perror("create socket err\n");
        return -1;
    }

    if (ioctl(sockfd, SIOCETHTOOL, &ifr) == -1) {
        perror("ioctl SIOCETHTOOL fail\n");
        return -1;
    }

    close(sockfd);
    return evalue.data;
}

static int i350_is_download_port(const char *iface)
{
    int sockfd;
    struct ifreq ifr;

    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd == -1) {
        perror("i350 socket err\n");
        return -2;
    }

    strncpy(ifr.ifr_name, iface, IFNAMSIZ);
    ifr.ifr_name[IFNAMSIZ-1] = 0;

    if (ioctl(sockfd, SIOCGIFADDR, &ifr) < 0) {
        /* errno99 : Cannot assign requested address */
        if (errno == 99) {
            close(sockfd);
            return -1;
        } else {
            close(sockfd);
            return -2;
        }
    }

    close(sockfd);
    return 0;
}

static int i350_eth_traf_external_test(int port)
{
    int rc, i;
    char *tname = "I350 traf external test";
    struct eth_traf_tx_task_settings tx_set;
    struct eth_traf_rx_task_settings rx_set;
    char iface[32];
    char errstr[64];

    testname("%s", tname);
    sprintf(iface, "eth%d", port);

    if ((NVRAM)->diagflag & D_EXT_LOOPBACK) {
        printf("External loopback flag is off, skip the port %d external loopback test\n", port);
        return PASSED;
    }

    if (port == I350_SKIP_PORT0) {
        printf("I350 port0 is management port so skip the traf external test\n");
        return (PASSED);
    }

    for (i = 0; i < 6; i++) {
        rc = i350_port_link_status_detect(iface);
        if (rc == -1) {
            sprintf(errstr, "I350 port %d traf external test fail\n", port);
            cterr('f',0, errstr);
            return FAILED;
        }
        if (rc == 0) {
            curie2ru_mdelay(5000);
            continue;
        }
        if (rc > 0)
            break;
    }

    if (rc == 0) {
        sprintf(errstr, "I350 port %d link down\n", port);
        cterr('f',0, errstr);
        return FAILED;
    }

    rc = i350_is_download_port(iface);
    if (!rc) {
        printf("port %d is download port, skip\n", port);
        return PASSED;
    } else if (rc == -2) {
        sprintf(errstr, "I350 port %d traf external test fail\n", port);
        cterr('f',0, errstr);
        return FAILED;
    }

    tx_set.mode = ETH_TRAF_TX_MODE_RADOM;
    tx_set.check = ETH_TRAF_TX_CHECK_BIT_ADD_YES;
    tx_set.burst = 100;
    tx_set.interval = 5;
    rx_set.chk_mode = ETH_TRAF_RX_MODE_CHECK_BIT;

    if (eth_traf_util_test_using_source_mac(iface, iface, &tx_set, &rx_set, 1)) {
        sprintf(errstr, "I350 port %d traf external test fail\n", port);
        cterr('f',0, errstr);
        return FAILED;
    }

    return PASSED;
}

/*
 *-----------------------------------------------------------------------------
$Log: i350_test.c,v $
Revision 1.1  2020/01/09 01:01:59  jiajliu
Merge Curie 2RU to main trunk

$Endlog$
 *-----------------------------------------------------------------------------
 */
