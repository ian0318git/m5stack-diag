/* $Id: diag_i211_test.c,v 1.2 2021/06/02 08:22:34 iachang Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/fugazi/diag_i211_test.c,v $
 *------------------------------------------------------------------
 * i2110_test.c - Diag Test for Intel I211.
 *
 * Jan 2019, Ian Chang
 *
 * Copyright (c) 2019-2020 by Cisco Systems, Inc.
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
#include "diag_i211_test.h"
#include "dash_fpga.h"

#define MF_1    (MF_CONTINUOUS | MF_DOGRP)
#define MF_2    (MF_1 | MF_DOALL)
#define MF_3    (MF_2 | MF_SHOW_ERRCOUNT)
#define MF_4    (MF_1 | MF_SHOW_ERRCOUNT)

static int I211_utility(int);
static void I211_fw_download(void);
int i211_check_result(int, char*);
int i211_register_test(int);
int i211_eeprom_test(int);
int i211_internal_loopback_test(int);
int i211_external_loopback_test(int);
int i211_int_ext_loopback_test(int);
static void i211_get_nic_number(void);
static void i211_show_running_port(void);
static void i211_display_cap(void);

/******************************************************************************
 *  List of Menu used for Broadcom I211
 *****************************************************************************/
static submenu_xtable_t I211_tests_submenu_table[] = {
   {"I211 Register Test", (type_t(*)())i211_register_test,   0,
     MF_3, (type_t(*)())0, 0, (type_t(*)())0,   0},
   {"I211 Int/External Loopback Test", (type_t(*)())i211_int_ext_loopback_test,   0,
     0, (type_t(*)())0, 0, (type_t(*)())0,   0},
   {"I211 Utility", (type_t(*)())I211_utility,   FALSE,
    0, NULL, 0, (type_t(*)())I211_utility,   TRUE},
};

/******************************************************************************
 *  List of Utilities used for XFI I211
 *****************************************************************************/
static submenu_xtable_t I211_util_items[] = {
    {"I211 Firmware Download", (type_t(*)())I211_fw_download, 0, 0,
     (type_t(*)())0, 0, (type_t(*)())0,   0},
    {"Get I211 NIC Number", (type_t(*)())i211_get_nic_number, 0, 0,
     (type_t(*)())0, 0, (type_t(*)())0, 0},
    {"Display I211 ports capability", (type_t(*)())i211_display_cap, 0, 0,
     (type_t(*)())0, 0, (type_t(*)())0, 0},
    {"Display Currently Running ETH Port", (type_t(*)())i211_show_running_port, 0, 0,
     (type_t(*)())0, 0, (type_t(*)())0, 0},
};

/******************************************************************************
 *  Macro Definitions
 *****************************************************************************/
#define I211_TESTS_SUBMENU_TABLE_SIZE (sizeof(I211_tests_submenu_table) / \
                                       sizeof(submenu_xtable_t))

#define I211_TESTS_UTIL_SIZE (sizeof(I211_util_items) / \
                              sizeof(submenu_xtable_t))

/******************************************************************************
 *  Global Variable
 *****************************************************************************/
/******************************************************************************
 * Primary & secondary submenu items (filled in from xtable)
 *****************************************************************************/
static mitem_t I211_tests_primary_items[I211_TESTS_SUBMENU_TABLE_SIZE +
                                        MAX_BASE_ITEMS];
static mitem_t I211_tests_secondary_items[I211_TESTS_SUBMENU_TABLE_SIZE +
                                          MAX_BASE_ITEMS];

/******************************************************************************
 * Primary & secondary utilities menu items (filled in from xtable)
 *****************************************************************************/
static mitem_t I211_tests_primary_util_items[I211_TESTS_UTIL_SIZE +
                                                 MAX_BASE_ITEMS];
static mitem_t I211_tests_secondary_util_items[I211_TESTS_UTIL_SIZE +
                                                   MAX_BASE_ITEMS];

/******************************************************************************
 *I211 Utils submenu
 *****************************************************************************/
menuinfo_t I211_util_menu = {
    "I211 Utility Menu",
    0,
    (PFT)show_endnote,
    0,
    0,
    I211_tests_primary_util_items,
};
menuinfo_t *I211_util_menup = &I211_util_menu;

menuinfo_t I211_subtest_menu = {
    "%s Subtest Menu",
    0,                                /* mtparam added by init_empty_menu */
    (PFT)show_endnote,                /* notes missing WICs in combos */
    0,                                /* use generic prompt */
    0,                                /* size (bumped by add_menu_item() */
    I211_tests_primary_items,
};
menuinfo_t *I211_submenup = &I211_subtest_menu;

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
int i211_test (int show_menu)
{
    build_primary_submenu(I211_tests_submenu_table,
                          I211_TESTS_SUBMENU_TABLE_SIZE,
                          "1G NIC I211", &I211_submenup);
    build_secondary_submenu(I211_tests_submenu_table,
                            I211_TESTS_SUBMENU_TABLE_SIZE,
                            I211_tests_secondary_items);

    if (show_menu) {
        exec_doall_menu_items(I211_submenup);
    } else {
        menu(I211_submenup, I211_tests_secondary_items, '\0' );
    }

    return (PASSED);
}

/*******************************************************************************
 *
 * Function    : I211_utility
 * Description :
 * Inputs      : menu_option - display menu instead of running I211 test
 *               
 * Outputs     : PASSED / FAILED
 *
 ******************************************************************************/
static int I211_utility (int show_menu)
{
    build_primary_submenu(I211_util_items, I211_TESTS_UTIL_SIZE,
                          "I211 Utilities Menu", &I211_util_menup);
    build_secondary_submenu(I211_util_items, I211_TESTS_UTIL_SIZE,
                            I211_tests_secondary_util_items);

    menu(I211_util_menup, I211_tests_secondary_util_items, '\0' );

    return (PASSED);
}

#define F_GRP        (MF_CONTINUOUS | MF_DOGRP)
#define F_GRP_E      (F_GRP | MF_SHOW_ERRCOUNT)
#define F_ALL        (F_GRP | MF_DOALL)
#define F_ALL_E      (F_ALL | MF_SHOW_ERRCOUNT)

/*
 * Function: i211_get_nic_number
 *
 * Description: To get I211 four port's NIC number 
 *
 * Input: none
 *
 * Return: none
 */
static void i211_get_nic_number (void)
{
    system(I211_GET_NIC);
}

/*
 * Function: i211_show_running_port
 *
 * Description: To display all the running ETH ports 
 *              including BCM57412 and Intel ineternal 10 ports
 *
 * Input: none
 *
 * Return: none
 */
static void i211_show_running_port (void)
{
    system(I211_SHOW_RUNNING_ETH);
}

/*
 * Function: i211_display_cap
 *
 * Description: Display I211 ports capability
 *
 * Input: none
 *
 * Return: none
 */
static void i211_display_cap (void)
{
    system(DISPLAY_I211_PORT1_CAP);
}

/******************************************************************************
 *
 * Function: I211_fw_download
 *
 * Description: This function performs the I211 firmware download and init.
 *
 * Inputs      : NONE
 *
 * Outputs     : NONE
 *
 ********************************************************************************/
static void I211_fw_download (void)
{
    printf("This utility uses eeupdate64e tool to upgrade I211 firmware\n");
    system(I211_FW_PROGRAMMING);
}

/******************************************************************************
 *
 * Function: I211_check_result
 *
 * Description: This function performs uses to check test result
 *
 * Inputs      : port - I211 port number 
                 test_name - what kind of test be checked
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/
int i211_check_result (int port, char *test_name)
{
    char buf[128] = "NULL";
    FILE *fp;
    int retval = FAILED;

    printf("Checking I211 port %d %s test result\n", port, test_name);
    fp = fopen(I211_TEST_RESULT, "r");
    if (fp == NULL) {
        cterr('f',0, "Port %d can't open file i211_test_result.txt", port);
        return (retval);
    }

    while (!feof(fp)) {
        fgets(buf, sizeof(buf), fp);
        if (strstr(buf, I211_PASS) == NULL) {
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
 * Function: i211_register_test
 *
 * Description: This function performs the register test
 *
 * Inputs      : port - I211 port number 
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/
int i211_register_test (int port)
{
    char *tname = "I211 register";
    int retval = FAILED;

    testname("%s", tname);

    if (port == I211_PORT0) {
        printf("I211 is management port so please note the register test "
                "is not the default test\n");
        system(I211_PORT1_REG_TEST);
        msleep(200);
        retval = i211_check_result(I211_PORT0, tname);
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
 * Function: i211_eeprom_test
 *
 * Description: This function performs the EEPROM test
 *
 * Inputs      : port - I211 port number 
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/
int i211_eeprom_test (int port)
{
    char *tname = "I211 EEPROM";
    int retval = FAILED;

    testname("%s", tname);

    if (port == I211_PORT0) {
        printf("I211 is management port so please note the EEPROM test "
                "is not the default test\n");
        system(I211_PORT1_EEPROM_TEST);
        msleep(200);
        retval = i211_check_result(I211_PORT0, tname);
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
 * Function: I211_int_ext_loopback_test
 *
 * Description: This function performs the internal/external loopback test
 *
 * Inputs      : port - i211 port number 
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/
int i211_int_ext_loopback_test (int port)
{
    char *tname = "I211 int/external lpbk";
    int retval = FAILED;
    int skip_ext_lpbk = FALSE;

    testname("%s", tname);

    if ((NVRAM)->diagflag & D_EXT_LOOPBACK) {
        printf("External loopback flag is off, skip the external loopback test\n");
        skip_ext_lpbk = TRUE;
    }

    if (port == I211_PORT0) {
        printf("I211 is management port so please note the int/external lpbk "
                "test is not the default test\n");
        /* Do internal loopback only */
        if (skip_ext_lpbk == TRUE) {
            retval = i211_internal_loopback_test(port);
        } else {
            /* Do internal/external loopback */
            retval = i211_internal_loopback_test(port);
            retval = i211_external_loopback_test(port);
        }
    } else {
        cterr('f',0, "Do not support this test");
        return (retval);
    }

    return (retval);
}

/******************************************************************************
 *
 * Function: I211_internal_loopback_test
 *
 * Description: This function perform the internal loopback test
 *              from CPU to I211 MAC and PHY.
 *
 * Inputs      : port - i211 port number 
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/
int i211_internal_loopback_test (int port)
{
    char *tname = "I211 internal lpbk";
    int retval = FAILED;

    if (port == I211_REST_PORT) {
        system(I211_PORT2_INT_LPBK);
        msleep(200);
        retval = i211_check_result(I211_PORT1, tname);
        if (retval == FAILED) {
            return (retval);
        }

        system(I211_PORT3_INT_LPBK);
        msleep(200);
        retval = i211_check_result(I211_PORT2, tname);
        if (retval == FAILED) {
            return (retval);
        }

        system(I211_PORT4_INT_LPBK);
        msleep(200);
        retval = i211_check_result(I211_PORT3, tname);
        if (retval == FAILED) {
            return (retval);
        }
    } else if (port == DO_I211_PORT0) {
        system(I211_PORT1_INT_LPBK);
        msleep(200);
        retval = i211_check_result(I211_PORT0, tname);
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
 * Function: I211_external_loopback_test
 *
 * Description: This function performs the external loopback test from CPU to
 *              I211 rj45 module
 *
 * Inputs      : port - I211 port number 
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/
int i211_external_loopback_test (int port)
{
    char *tname = "I211 external lpbk";
    int retval = FAILED;

    if (port == I211_REST_PORT) {
        system(I211_PORT2_EXT_LPBK);
        msleep(200);
        retval = i211_check_result(I211_PORT1, tname);
        if (retval == FAILED) {
            return (retval);
        }

        system(I211_PORT3_EXT_LPBK);
        msleep(200);
        retval = i211_check_result(I211_PORT2, tname);
        if (retval == FAILED) {
            return (retval);
        }

        system(I211_PORT4_EXT_LPBK);
        msleep(200);
        retval = i211_check_result(I211_PORT3, tname);
        if (retval == FAILED) {
            return (retval);
        }
    } else if (port == DO_I211_PORT0) {
        system(I211_PORT1_EXT_LPBK);
        msleep(200);
        retval = i211_check_result(I211_PORT0, tname);
        if (retval == FAILED) {
            return (retval);
        }
    } else {
        cterr('f',0, "Do not support this test");
        return (retval);
    }

    return (retval);
}


/*-------------------------------------------------
 * $Log: diag_i211_test.c,v $
 * Revision 1.2  2021/06/02 08:22:34  iachang
 * CSCvo59196-33 : Merge Fugazi from ASR1K-main-branch to ISR main trunk
 *
 * Revision 1.1.8.2  2020/08/26 02:37:48  iachang
 * Merge Fugazi code into main trunk
 *
 * Revision 1.1.6.7  2020/08/21 02:49:00  iachang
 * PRRQ CSCvo59196-9 : i211 PHY code review
 *
 * Revision 1.1.6.6  2020/08/03 09:25:40  iachang
 * Code clean up.
 *
 * Revision 1.1.6.5  2019/03/29 18:28:44  iachang
 * support i211 firmware upgrade
 *
 * Revision 1.1.6.4  2019/03/28 06:06:44  iachang
 * celo64e didn't support I211 EEPROM Test.
 *
 * Revision 1.1.6.3  2019/03/21 21:44:12  iachang
 * Bring up I211 test, skip loopabck test with default test.
 *
 * Revision 1.1.6.2  2019/03/14 03:48:35  letsai
 * Initial check in.
 *
 *
 *
 *
 * $Endlog$
 * */
