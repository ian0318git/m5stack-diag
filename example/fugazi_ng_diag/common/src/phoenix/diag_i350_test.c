/* $Id: diag_i350_test.c,v 1.2 2021/04/15 00:52:26 achiu2 Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/phoenix/diag_i350_test.c,v $
 *------------------------------------------------------------------
* diag_i350_test.c - Diag Test for Intel I350.
*
* July 2018, Leschen 
*
* Copyright (c) 2018-2019 by Cisco Systems, Inc.
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
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>

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
#include "ethernet.h"
#include "linux_eth.h"
#include "queryflags.h" /* for query user functions */  
#include "diag_i350_test.h"
#include "diag_eth_pkt_txrx.h"
#include "dnv_eth_lib.h"
#include "mb_tests.h"
#include "dash_fpga.h"


#define MF_1    (MF_CONTINUOUS | MF_DOGRP)
#define MF_2    (MF_1 | MF_DOALL)
#define MF_3    (MF_2 | MF_SHOW_ERRCOUNT)
#define MF_4    (MF_1 | MF_SHOW_ERRCOUNT)

static int i350_utility(int);
int build_i350_test_menu(boolean);
int i350_check_result(int, char*);
int i350_register_test(int);
int i350_internal_port_loopback_test(int);
int i350_external_port_loopback_test(int);
int i350_external_port_1G_loopback_test(int);
static void i350_get_nic_number(void);
static void i350_display_cap(void);

/******************************************************************************
 *  List of Menu used for I350
 *****************************************************************************/
static submenu_xtable_t I350_tests_submenu_table[] = {
   {"I350 Utility",
    (type_t(*)())i350_utility,   FALSE,
    0, NULL, 0, 
    (type_t(*)())i350_utility,   TRUE},

   {"I350 Register Test",
    (type_t(*)())i350_register_test,   0,
     MF_CONTINUOUS | MF_DOGRP | MF_DOALL,
    (PFT)0, 0, (type_t(*)())0,   0},

   {"I350 Internal Loopback Port 0 Test", 
    (type_t(*)())i350_internal_port_loopback_test,   I350_PORT0,
     MF_CONTINUOUS | MF_DOGRP | MF_DOALL,
    (PFT)0, 0, (type_t(*)())0,   0},

   {"I350 Internal Loopback Port 1 Test", 
    (type_t(*)())i350_internal_port_loopback_test,   I350_PORT1,
     MF_CONTINUOUS | MF_DOGRP | MF_DOALL,
    (PFT)0, 0, (type_t(*)())0,   0},

   {"I350 External RJ45 Loopback Port 0 Test", 
    (type_t(*)())i350_external_port_loopback_test,   I350_PORT0,
     MF_CONTINUOUS | MF_DOGRP | MF_DOALL,
    (PFT)0, 0, (type_t(*)())0,   0},

   {"I350 External RJ45 Loopback Port 1 Test", 
    (type_t(*)())i350_external_port_loopback_test,   I350_PORT1,
     MF_CONTINUOUS | MF_DOGRP | MF_DOALL,
    (PFT)0, 0, (type_t(*)())0,   0},

   {"I350 External 1G Speed Loopback Port 0 Test", 
    (type_t(*)())i350_external_port_1G_loopback_test,   I350_PORT0,
     MF_CONTINUOUS | MF_DOGRP,
    (PFT)0, 0, (type_t(*)())0,   0},

   {"I350 External 1G Speed Loopback Port 1 Test", 
    (type_t(*)())i350_external_port_1G_loopback_test,   I350_PORT1,
     MF_CONTINUOUS | MF_DOGRP,
    (PFT)0, 0, (type_t(*)())0,   0},
};

/******************************************************************************
 *  List of Utilities used for I350
 *****************************************************************************/
static submenu_xtable_t I350_util_items[] = {
    {"Get I350 NIC Number", (type_t(*)())i350_get_nic_number, 0, 0,
     (type_t(*)())0, 0, (type_t(*)())0, 0},
    {"Display I350 ports capability", (type_t(*)())i350_display_cap, 0, 0,
     (type_t(*)())0, 0, (type_t(*)())0, 0},
    {"Show I350 firmware version", (type_t(*)())phoenix_show_i350_ver, 0, 0,
     (type_t(*)())0, 0, (type_t(*)())0, 0},
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

/*******************************************************************************
 *
 * Function   : build_i350_test_menu
 * Description: build i350 test submenu 
 * Inputs     : Test/Menu 
 * Outputs    : PASSED
 *
 *******************************************************************************
 */
int build_i350_test_menu (boolean show_menu)
{
    char *tname = "I350 Test";
    testname(tname);

    build_primary_submenu(I350_tests_submenu_table,
                          I350_TESTS_SUBMENU_TABLE_SIZE,
                          "I350 test SubMenu", &I350_submenup);
    build_secondary_submenu(I350_tests_submenu_table,
                            I350_TESTS_SUBMENU_TABLE_SIZE,
                            I350_tests_secondary_items);

    if (show_menu) {
        menu(I350_submenup, I350_tests_secondary_items, '\0' );
    } else {
        do_all_menu_items(I350_submenup);
    }

    return (PASSED);
}

/*******************************************************************************
 *
 * Function    : i350_utility
 * Description :
 * Inputs      : menu_option - display menu instead of running all XAUI 88X2222M
 *               tests.
 * Outputs     : PASSED / FAILED
 *
 ******************************************************************************/
static int i350_utility (int show_menu)
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
    system(DISPLAY_I350_PORT0_CAP);
    system(DISPLAY_I350_PORT1_CAP);
}

/*
 * Function: phoenix_show_i350_ver
 *
 * Description: Display I350 firmware version
 *
 * Input: none
 *
 * Return: none
 */
void phoenix_show_i350_ver (void)
{
    int ix;
    char cmd[256];

    for (ix = I350_PORT0; ix <= I350_PORT1; ix++) {
        sprintf(cmd, "echo I350 port %d ", ix);
        system(cmd); 
        sprintf(cmd, "celo64e /nic=%d /EEPROMVER | grep 'EEPROM Image Version'", ix+1);
        system(cmd);
    }
}

/******************************************************************************
 *
 * Function: i350_check_result
 *
 * Description: This function performs uses to check test result
 *
 * Inputs      : port - I350 port number 
                 test_name - what kind of test be checked
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/
int i350_check_result (int port, char *test_name)
{
    FILE *fp;
    int retval = FAILED;
    char buf[512] = {0};

    printf("Checking I350 port %d %s test result\n", port, test_name);
    fp = fopen(I350_TEST_RESULT, "r");
    if (fp == NULL) {
        cterr('f',0, "Port %d can't open file %s", port, I350_TEST_RESULT);
        return (retval);
    }

    while (!feof(fp)) {
        fgets(buf, sizeof(buf), fp);
        if (strstr(buf, I350_FAIL) != NULL) {
            printf("port %d %s test failed, ", port, test_name);
            fclose(fp);
            printf("\n");
            system(I350_GET_TEST_RESULT);
            printf("\n");
            system(I350_GET_TEST_LOG);
            printf("\n");
            return (retval);
        } else {
            printf("port %d test passed, ", port);
            printf("\n--------- CELO Result --------\n");
            system(I350_GET_TEST_PASS_LOG);
            printf("\n");
            retval = PASSED;
            break;
        }
    }

    fclose(fp);
    return (retval);
}

/******************************************************************************
 *
 * Function: i350_register_test
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
    int test_port = 0;

    testname("%s", tname);
    for (test_port = I350_PORT0; test_port <= I350_PORT1; test_port++) {
        prpass(testpass, "Port %d ", test_port);

        switch (test_port) {
            case I350_PORT0:
                system(I350_PORT0_REG_TEST);
                break;
            case I350_PORT1:
                system(I350_PORT1_REG_TEST);
                break;
            default:
                cterr('f', 0, "%s: Wrong test port %d ",
                      __FUNCTION__, test_port);
                return (retval);
        }

        msleep(WAIT_I350_REG_TEST);

        retval = i350_check_result(test_port, tname);
        if (retval == FAILED) {
            cterr('f', 0, "%s: Failed To do Register test "
                          "(from I350 port %d)",
                          __FUNCTION__, test_port);
            prcomplete(testpass, errcount, (char *)0);
            return (retval);
        }
    }

    prcomplete(testpass, errcount, (char *)0);

    return (retval);
}


/******************************************************************************
 *
 * Function: i350_internal_port_loopback_test
 *
 * Description: This function perform the internal loopback test
 *              from CPU to I350 MAC and PHY by single port.
 *
 * Inputs      : lpbk_port
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/
int i350_internal_port_loopback_test (int lpbk_port)
{
    char *tname = "I350 internal lpbk";
    int retval = FAILED;
    int test_port = 0;

    testname("%s", tname);

    test_port = lpbk_port;
    prpass(testpass, "Port %d ", test_port);

    switch (test_port) {
        case I350_PORT0:
            system(I350_PORT0_INT_LPBK);
            break;
        case I350_PORT1:
            system(I350_PORT1_INT_LPBK);
            break;
        default:
            cterr('f', 0, "%s: Wrong test port %d ",
                    __FUNCTION__, test_port);
            return (retval);
    }

    msleep(WAIT_I350_ETH_TEST);

    retval = i350_check_result(test_port, tname);
    if (retval == FAILED) {
        cterr('f', 0, "%s: Failed To do internal loopback test "
                      "(from I350 port %d)",
                      __FUNCTION__, test_port);
        prcomplete(testpass, errcount, (char *)0);
        return (retval);
    }

    prcomplete(testpass, errcount, (char *)0);

    return (retval);
}

/******************************************************************************
 *
 * Function: i350_external_port_loopback_test
 *
 * Description: This function performs the external loopback test from CPU to
 *              I350 rj45 module by single port
 *
 * Inputs      : lpbk_port
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/
int i350_external_port_loopback_test (int lpbk_port)
{
    char *tname = "I350 external lpbk";
    int retval = FAILED;
    int test_port = 0;

    testname("%s",tname);

    /* according to menu_show_dflags(), D_EXT_LPBK is inverse flag */
    if ((NVRAM)->diagflag & D_EXT_LOOPBACK) {
        printf("External Loopback flag is off. Skip the test\n");
        return (PASSED);
    } 

    test_port = lpbk_port;
    prpass(testpass, "Port %d ", test_port);

    switch (test_port) {
        case I350_PORT0:
            system(I350_PORT0_EXT_LPBK);
            break;
        case I350_PORT1:
            system(I350_PORT1_EXT_LPBK);
            break;
        default:
            cterr('f', 0, "%s: Wrong test port %d ",
                    __FUNCTION__, test_port);
            return (retval);
    }

    msleep(WAIT_I350_ETH_TEST);
    retval = i350_check_result(test_port, tname);
    if (retval == FAILED) {
        cterr('f', 0, "%s: Failed To do loopback test "
                        "(from I350 RJ45 port %d)",
                        __FUNCTION__, test_port);
        prcomplete(testpass, errcount, (char *)0);
        return (retval);
    }

    prcomplete(testpass, errcount, (char *)0);
    return (retval);
}

/******************************************************************************
 *
 * Function: i350_external_port_1G_loopback_test
 *
 * Description: This function performs the external loopback test from CPU to
 *              I350 rj45 module by single port
 *
 * Inputs      : lpbk_port
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/
int i350_external_port_1G_loopback_test (int lpbk_port)
{
    char *tname = "I350 external lpbk - 1G speed";
    int retval = FAILED;
    int test_port = 0;

    testname("%s",tname);

    /* according to menu_show_dflags(), D_EXT_LPBK is inverse flag */
    if ((NVRAM)->diagflag & D_EXT_LOOPBACK) {
        printf("External Loopback flag is off. Skip the test\n");
        return (PASSED);
    } 

    test_port = lpbk_port;
    prpass(testpass, "Port %d ", test_port);

    switch (test_port) {
        case I350_PORT0:
            system(I350_PORT0_1G_EXT_LPBK);
            break;
        case I350_PORT1:
            system(I350_PORT1_1G_EXT_LPBK);
            break;
        default:
            cterr('f', 0, "%s: Wrong test port %d ",
                    __FUNCTION__, test_port);
            return (retval);
    }

    msleep(WAIT_I350_ETH_TEST);
    retval = i350_check_result(test_port, tname);
    if (retval == FAILED) {
        cterr('f', 0, "%s: Failed To do 1G loopback test "
                        "(from I350 RJ45 port %d)",
                        __FUNCTION__, test_port);
        prcomplete(testpass, errcount, (char *)0);
        return (retval);
    }

    prcomplete(testpass, errcount, (char *)0);
    return (retval);
}

