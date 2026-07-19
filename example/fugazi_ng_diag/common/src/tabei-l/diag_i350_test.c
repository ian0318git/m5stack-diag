/* $Id: diag_i350_test.c,v 1.4 2020/08/06 07:54:55 kehuang2 Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/tabei-l/diag_i350_test.c,v $
*-----------------------------------------------------------------------------
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
#include "diag_gephy_test.h"
#include "dnv_eth_lib.h"
#include "mb_tests.h"
#include "dash_fpga.h"
#include "dnv_eth_lib.h"
#include "diag_fpga.h"


#define MF_1    (MF_CONTINUOUS | MF_DOGRP)
#define MF_2    (MF_1 | MF_DOALL)
#define MF_3    (MF_2 | MF_SHOW_ERRCOUNT)
#define MF_4    (MF_1 | MF_SHOW_ERRCOUNT)

static int i350_utility(int);
int i350_check_result(int, char*);
int i350_register_test(int);
int i350_internal_loopback_test(int);
int i350_external_loopback_test(int);
int i350_external_sfp_loopback_test(int);
int i350_read_ctrl_reg (ulong , uint *, uint);
int i350_write_ctrl_reg (ulong , uint, uint);
int check_phy_copper_link_mode (int);
int set_sfp_phy_module (int);
int check_i350_en_dis_sfp_util(int);
int i350_sfp_i2c_test(int);
static void i350_get_nic_number(void);
static void i350_display_cap(void);
static void i350_check_sfp_present(void);
static void i350_en_dis_sfp_util(void);


/******************************************************************************
 *  List of Menu used for I350
 *****************************************************************************/
static submenu_xtable_t I350_tests_submenu_table[] = {
   {"I350 Utility", (type_t(*)())i350_utility,   FALSE,
    0, NULL, 0, (type_t(*)())i350_utility,   TRUE},
   {"I350 Register Test", (type_t(*)())i350_register_test,   0,
     MF_2, (PFT)0, 0, (type_t(*)())0,   0},
   {"I350 Internal Loopback Test", (type_t(*)())i350_internal_loopback_test,   0,
     MF_2, (PFT)0, 0, (type_t(*)())0,   0},
   {"I350 External RJ45 Loopback Test", (type_t(*)())i350_external_loopback_test,   0,
     MF_2, (PFT)0, 0, (type_t(*)())i350_external_loopback_test,   TRUE},
   {"I350 External SFP Loopback Test", (type_t(*)())i350_external_sfp_loopback_test,   0,
     MF_2, (PFT)0, 0, (type_t(*)())i350_external_sfp_loopback_test,   TRUE},
   {"I350 SFP I2C (Intel/FPGA) Test", (type_t(*)())i350_sfp_i2c_test,   0,
     MF_2, (PFT)is_promethium, 0, (type_t(*)())i350_sfp_i2c_test,   TRUE},
};

/******************************************************************************
 *  List of Utilities used for I350
 *****************************************************************************/
static submenu_xtable_t I350_util_items[] = {
    {"Get I350 NIC Number", (type_t(*)())i350_get_nic_number, 0, 0,
     (type_t(*)())0, 0, (type_t(*)())0, 0},
    {"Display I350 ports capability", (type_t(*)())i350_display_cap, 0, 0,
     (type_t(*)())0, 0, (type_t(*)())0, 0},
    {"Check I350 SFP ports present pin", (type_t(*)())i350_check_sfp_present, 0, 0,
     (type_t(*)())0, 0, (type_t(*)())0, 0},
    {"Assert SFP DISABLE", (type_t(*)())i350_en_dis_sfp_util, 0, 0,
     (type_t(*)())0, 0, (type_t(*)())0, 0},
    {"Read I350 SFP EEPROM", (type_t(*)())igb_read_sfp_eeprom_util, 0, 0,
     (type_t(*)())0, 0, (type_t(*)())0, 0},
    {"Dump I350 SFP EEPROM", (type_t(*)())igb_dump_sfp_eeprom_util, 0, 0,
     (type_t(*)())0, 0, (type_t(*)())0, 0},
    {"Read I350 Specific SFP PHY", (type_t(*)())igb_read_sfp_phy_util, 0, 0,
     (type_t(*)())0, 0, (type_t(*)())0, 0},
    {"Write I350 Specific SFP PHY", (type_t(*)())igb_write_sfp_phy_util, 0, 0,
     (type_t(*)())0, 0, (type_t(*)())0, 0},
    {"Check I350 enable/disable SFP utility", (type_t(*)())check_i350_en_dis_sfp_util, 0, 0,
     (type_t(*)())0, 0, (type_t(*)())check_i350_en_dis_sfp_util, TRUE},
    {"Dump I350 SFP EEPROM via FPGA", (type_t(*)())diag_fpga_i2c_dump_sfp_util, 0, 0,
     (type_t(*)())is_promethium, 0, (type_t(*)())diag_fpga_i2c_dump_sfp_util, TRUE},
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
int diag_i350_test (int show_menu)
{

    build_primary_submenu(I350_tests_submenu_table,
                          I350_TESTS_SUBMENU_TABLE_SIZE,
                          "I350 ", &I350_submenup);
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
    system(DISPLAY_I350_PORT2_CAP);
    system(DISPLAY_I350_PORT3_CAP);
}

/*
 * Function: i350_check_sfp_present
 *
 * Description: Check I350 SFP present PIN
 *
 * Input: none
 *
 * Return: none
 */
static void i350_check_sfp_present (void)
{
    uint i350_ctrl_val = 0;

    i350_read_ctrl_reg(I350_PORT2, &i350_ctrl_val, TABEIL_I350_CTRL);
    if (!(i350_ctrl_val & TABEIL_I350_SFP_PRESENT)) {
        printf("I350 Port: %d, Detect SFP Present\n", (int)I350_PORT2);
    } else {
        printf("I350 Port: %d, Not Detect SFP Present\n", (int)I350_PORT2);
    }

    i350_read_ctrl_reg(I350_PORT3, &i350_ctrl_val, TABEIL_I350_CTRL);
    if (!(i350_ctrl_val & TABEIL_I350_SFP_PRESENT)) {
        printf("I350 Port: %d, Detect SFP Present\n", (int)I350_PORT3);
    } else {
        printf("I350 Port: %d, Not Detect SFP Present\n", (int)I350_PORT3);
    }

}


/*
 * Function: i350_en_dis_sfp_util
 *
 * Description: I350 enable/disable SFP 
 *
 * Input: none
 *
 * Return: none
 */
static void i350_en_dis_sfp_util (void)
{
    uint i350_ctrl_val = 0;
    int set_value = 0, which_sfp = 0;

    which_sfp = getdec_answer("\nEnter I350 Port 2/3 SFP ", 2, 2, 3);
    set_value = gethex_answer("\nDisable SFP: 1; Enable SFP: 0 ", 0, 0, 1);

    if (set_value == 1) {
        i350_read_ctrl_reg(which_sfp, &i350_ctrl_val, TABEIL_I350_CTRL);
        i350_ctrl_val |= TABEIL_I350_SFP_DISABLE;
        i350_write_ctrl_reg(which_sfp, i350_ctrl_val, TABEIL_I350_CTRL);
        printf("I350 Port: %d, Disable SFP\n", which_sfp);

    } else {
        i350_read_ctrl_reg(which_sfp, &i350_ctrl_val, TABEIL_I350_CTRL);
        i350_ctrl_val &= ~(TABEIL_I350_SFP_DISABLE);
        i350_write_ctrl_reg(which_sfp, i350_ctrl_val, TABEIL_I350_CTRL);
        printf("I350 Port: %d, Enable SFP\n", which_sfp);
    }

}

/*
 *
 * Function: check_i350_en_dis_sfp_util
 *
 * Description: This function is to check the function of SFP disable pin
 *
 * Inputs      : option - TRUE: verify port2 or port3 only
 *                        FALSE: verify both port2 and port3
 *
 * Outputs     : PASSED / FAILED
 *
 */
int  check_i350_en_dis_sfp_util(int option)
{

    char *tname = "I350 SFP disable pin";
    uint i350_ctrl_val = 0;
    char iface_name_t[16];
    char *iface_name = iface_name_t;
    int test_port, start_port, end_port, which_sfp = 0;

    testname("%s", tname);

    if (option) {
        which_sfp = getdec_answer("\nEnter I350 Port 2/3 SFP ", 2, 2, 3);
        start_port = which_sfp;
        end_port = which_sfp;
    } else {
        start_port = I350_PORT2;
        end_port = I350_PORT3;
    }

    for (test_port = start_port; test_port <= end_port; test_port++) {
        /* Detect SFP Present */
        prpass(testpass, "Detect SFP module present ");
        i350_read_ctrl_reg(test_port, &i350_ctrl_val, TABEIL_I350_CTRL);
        if (i350_ctrl_val & TABEIL_I350_SFP_PRESENT) {
            cterr('f', 0, "I350 Port: %d, Not detect SFP module present\n", test_port);
            return (FAILED);
        } 

        /* Disable SFP */
        i350_read_ctrl_reg(test_port, &i350_ctrl_val, TABEIL_I350_CTRL);
        i350_ctrl_val |= TABEIL_I350_SFP_DISABLE;
        i350_write_ctrl_reg(test_port, i350_ctrl_val, TABEIL_I350_CTRL);
        printf("I350 Port: %d, Disable SFP\n", test_port);
    
        if (check_phy_copper_link_mode(test_port) == TRUE) {
            set_sfp_phy_module(test_port);
        }

        switch (test_port) {
        case I350_PORT2:
            sprintf(iface_name, TABEI_I350_SFP_P2_IFACE_NAME);

            printf ("\nChecking linkup ...");
            system(I350_PORT2_UP);

            msleep(WAIT_I350_ETH_TEST);

            /* check Linux ethernet interface status */
            if (chk_linux_eth_linkup(TABEI_I350_SFP_PORT2, TRUE) == PASSED) {
                cterr('f',0, "Ethernet %s link up in disable assert status", iface_name);
                prcomplete(testpass, errcount, (char *)0);
                return (FAILED);
            } else {
                printf ("\n%s: link is not detected as expected.\n", iface_name);
            }
            break;
        case I350_PORT3:
            sprintf(iface_name, TABEI_I350_SFP_P3_IFACE_NAME);

            printf ("\nChecking linkup ...");
            system(I350_PORT3_UP);

            msleep(WAIT_I350_ETH_TEST);

            /* check Linux ethernet interface status */
            if (chk_linux_eth_linkup(TABEI_I350_SFP_PORT3, TRUE) == PASSED) {
                cterr('f',0, "Ethernet %s link up in disable assert status", iface_name);
                prcomplete(testpass, errcount, (char *)0);
                return (FAILED);
            } else {
                printf ("\n%s: link is not detected as expected\n", iface_name);
            }
            break;
        default:
            cterr('f', 0, "%s: Wrong test port %d ",
                  __FUNCTION__, test_port);
            return (FAILED);
        }

        /* Enable SFP */
        i350_read_ctrl_reg(test_port, &i350_ctrl_val, TABEIL_I350_CTRL);
        i350_ctrl_val &= ~(TABEIL_I350_SFP_DISABLE);
        i350_write_ctrl_reg(test_port, i350_ctrl_val, TABEIL_I350_CTRL);
        printf("I350 Port: %d, Enable SFP\n", test_port);
    }

    prcomplete(testpass, errcount, (char *)0);
    return (PASSED);
}

/******************************************************************************
 *
 * Function: I350_check_result
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
    int test_port = 0;

    testname("%s", tname);

    for (test_port = I350_PORT0; test_port <= I350_PORT3; test_port++) {
        prpass(testpass, "Port %d ", test_port);

        switch (test_port) {
        case I350_PORT0:
            system(I350_PORT0_REG_TEST);
            break;
        case I350_PORT1:
            system(I350_PORT1_REG_TEST);
            break;
        case I350_PORT2:
            system(I350_PORT2_REG_TEST);
            break;
        case I350_PORT3:
            system(I350_PORT3_REG_TEST);
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
 * Function: I350_internal_loopback_test
 *
 * Description: This function perform the internal loopback test
 *              from CPU to I350 MAC and PHY.
 *
 * Inputs      : dummy
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/
int i350_internal_loopback_test (int dummy)
{
    char *tname = "I350 internal lpbk";
    int retval = FAILED;
    int test_port = 0;

    testname("%s", tname);

    for (test_port = I350_PORT0; test_port <= I350_PORT3; test_port++) {
        prpass(testpass, "Port %d ", test_port);

        switch (test_port) {
        case I350_PORT0:
            system(I350_PORT0_INT_LPBK);
            break;
        case I350_PORT1:
            system(I350_PORT1_INT_LPBK);
            break;
        case I350_PORT2:
            system(I350_PORT2_INT_LPBK);
            break;
        case I350_PORT3:
            system(I350_PORT3_INT_LPBK);
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
    }

    prcomplete(testpass, errcount, (char *)0);

    return (retval);
}

/******************************************************************************
 *
 * Function: I350_external_loopback_test
 *
 * Description: This function performs the external loopback test from CPU to
 *              I350 rj45 module
 *
 * Inputs      : option
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/
int i350_external_loopback_test (int option)
{
    char *tname = "I350 external lpbk";
    int retval = FAILED;
    int test_port = 0;
    int start_port, end_port, which_copper = 0;

    testname("%s",tname);

    /* according to menu_show_dflags(), D_EXT_LPBK is inverse flag */
    if ((NVRAM)->diagflag & D_EXT_LOOPBACK) {
        printf("External Loopback flag is off. Skip the test\n");
        return (PASSED);
    } 

    if (option) {
        which_copper = getdec_answer("\nEnter I350 Port 0/1 Copper ", 0, 0, 1);
        start_port = which_copper;
        end_port = which_copper;
    } else {
        start_port = I350_PORT0;
        end_port = I350_PORT1;
    }

    for (test_port = start_port; test_port <= end_port; test_port++) {
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
    }

    prcomplete(testpass, errcount, (char *)0);
    return (retval);
}

/******************************************************************************
 *
 * Function: I350_external_sfp_loopback_test
 *
 * Description: This function performs the external loopback test from CPU to
 *              I350 SFP module
 *
 * Inputs      : option
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/
int i350_external_sfp_loopback_test (int option)
{
    char *tname = "I350 external SFP lpbk";
    int retval = FAILED;
    char iface_name_t[16];
    char *iface_name = iface_name_t;
    int test_port = 0;
    uint i350_ctrl_val = 0;
    int start_port, end_port, which_sfp = 0;
    char id_data[32] = {0};

    testname("%s", tname);

    /* according to menu_show_dflags(), D_EXT_LPBK is inverse flag */
    if ((NVRAM)->diagflag & D_EXT_LOOPBACK) {
        printf("External Loopback flag is off. Skip the test\n");
        return (PASSED);
    } 

    if (option) {
        which_sfp = getdec_answer("\nEnter I350 Port 2/3 SFP ", 2, 2, 3);
        start_port = which_sfp;
        end_port = which_sfp;
    } else {
        start_port = I350_PORT2;
        end_port = I350_PORT3;
    }


    for (test_port = start_port; test_port <= end_port; test_port++) {
        prpass(testpass, "SFP port %d ", test_port);

        prpass(testpass, "Detect SFP module present ");
        i350_read_ctrl_reg(test_port, &i350_ctrl_val, TABEIL_I350_CTRL);
        if (i350_ctrl_val & TABEIL_I350_SFP_PRESENT) {
            cterr('f', 0, "I350 Port: %d, Not detect SFP module present\n", test_port);
            return (FAILED);
        } 

        prpass(testpass, "Detect SFP module TX Fault ");
        i350_read_ctrl_reg(test_port, &i350_ctrl_val, TABEIL_I350_CTRL_EXT);
        if (i350_ctrl_val & TABEIL_I350_SFP_TX_FAULT) {
            printf("Detect TX fault, please check optical loppback or SFP module\n");
            cterr('w', 0, "I350 Port: %d, Detect SFP TX Fault\n", test_port);
        } 

        prpass(testpass, "Read SFP vendor name ");
        if (igb_read_sfp_vendor_name(test_port, id_data) == FAILED) {
            cterr('f', 0, "I350 Port: %d, Cannot read correct SFP Info\n", test_port);
            return (FAILED);
        }

        printf("\nSFP Vendor Name: %s\n", id_data);

        if (check_phy_copper_link_mode(test_port) == TRUE) {
            set_sfp_phy_module(test_port);
        }

        prpass(testpass, "Running Loopback ");
        switch (test_port) {
        case I350_PORT2:
            sprintf(iface_name, TABEI_I350_SFP_P2_IFACE_NAME);

            system(I350_PORT2_UP);

            msleep(WAIT_I350_ETH_TEST);

            /* check Linux ethernet interface status */
            if (chk_linux_eth_linkup(TABEI_I350_SFP_PORT2, TRUE) == FAILED) {
                cterr('f',0, "Ethernet %s cannot link up", iface_name);
                prcomplete(testpass, errcount, (char *)0);
                return (FAILED);
            }

            break;
        case I350_PORT3:
            sprintf(iface_name, TABEI_I350_SFP_P3_IFACE_NAME);

            system(I350_PORT3_UP);

            msleep(WAIT_I350_ETH_TEST);

            /* check Linux ethernet interface status */
            if (chk_linux_eth_linkup(TABEI_I350_SFP_PORT3, TRUE) == FAILED) {
                cterr('f',0, "Ethernet %s cannot link up", iface_name);
                prcomplete(testpass, errcount, (char *)0);
                return (FAILED);
            }

            break;
        default:
            cterr('f', 0, "%s: Wrong test port %d ",
                  __FUNCTION__, test_port);
            return (retval);
        }

        retval = eth_pkt_txrx(iface_name, LPBKTEST_PKT_CNT, FALSE);

        if (retval != PASSED) {
            cterr('f', 0, "%s: Failed To do loopback test "
                          "(from I350 SFP port %s)",
                          __FUNCTION__, iface_name);
            prcomplete(testpass, errcount, (char *)0);
            return (FAILED);
        } 

        switch (test_port) {
        case I350_PORT2:
            system(I350_PORT2_DOWN);
            break;
        case I350_PORT3:
            system(I350_PORT3_DOWN);
            break;
        default:
            printf("Wrong test port: %d\n", test_port);
        }

    }

    prcomplete(testpass, errcount, (char *)0);
    return (retval);
}


/*******************************************************************************
 *
 * Function    : i350_read_ctrl_reg
 *
 * Description : Function to read Device Control Register in I350
 *               
 * Inputs      : which_port - 2/3
 *               *val   - data buffer 
 *               offset - register offset
 * Outputs     : PASSED/FAILED
 *
 *******************************************************************************
 */
int i350_read_ctrl_reg (ulong which_port, uint *val, uint offset)
{   
    int      fd = -1;
    void     *map_base, *virt_addr;
    off_t    target = 0;
    unsigned map_size;
    char     ethaddr[128];

    map_size = MAP_SIZE;

    target = (off_t)0;
    
    if (is_tabeil() == TRUE) {
        if (which_port == I350_PORT2) {
            sprintf(ethaddr, "%s%s", TABEIL_I350_PCIE_BUS, TABEIL_I350_PCIE_PORT2);
        } else if (which_port == I350_PORT3) {
            sprintf(ethaddr, "%s%s", TABEIL_I350_PCIE_BUS, TABEIL_I350_PCIE_PORT3);
        } else {
            printf("Wrong port number: %d\n", (int)which_port);
            return (FAILED);
        }
    } else if (is_promethium() == TRUE) {
        if (which_port == I350_PORT2) {
            sprintf(ethaddr, "%s%s", PROMETHIUM_I350_PCIE_BUS, PROMETHIUM_I350_PCIE_PORT2);
        } else if (which_port == I350_PORT3) {
            sprintf(ethaddr, "%s%s", PROMETHIUM_I350_PCIE_BUS, PROMETHIUM_I350_PCIE_PORT3);
        } else {
            printf("Wrong port number: %d\n", (int)which_port);
            return (FAILED);
        }
    } else {
        printf("Wrong Platform type\n");
        return (FAILED);
    }

    fd = open(ethaddr, O_RDWR | O_SYNC);
    if (fd < 0) {
        printf("%s: Failed to open MEM device.\n", __FUNCTION__);
        return (FAILED);
    }

    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("mmap(%d, %ld, 0x%x, 0x%x, %d, 0x%x)\n", 0, MAP_SIZE,
                PROT_READ | PROT_WRITE, MAP_SHARED, fd, (int) target);
        fflush(stdout);
    }

    map_base = mmap(NULL, map_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd,
                    target & ~MAP_MASK);
    if (map_base == MAP_FAILED) {
        printf("%s: Failed to map in virtual address space.\n",
               __FUNCTION__);
         close(fd);
         return (FAILED);
    }
    virt_addr = map_base + offset;

    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("I350 Port :%d, Device Control Register - CTRL: %x\n", 
               (int)which_port, *(volatile uint32_t*)virt_addr);
    }

    *val = *(volatile uint32_t*)virt_addr;

    if (munmap(map_base, MAP_SIZE) == -1) {
        printf("%s: Failed to munmap.\n", __FUNCTION__);
        close(fd);
        return (FAILED);
    }
    close(fd);



    return (PASSED);
}

/*******************************************************************************
 *
 * Function    : i350_write_ctrl_reg
 *
 * Description : Function to write Device Control Register in I350
 *               
 * Inputs      : which_port - 2/3
 *               val   - data buffer 
 *               offset - register offset
 * Outputs     : PASSED/FAILED
 *
 *******************************************************************************
 */
int i350_write_ctrl_reg (ulong which_port, uint val, uint offset)
{   
    int      fd = -1;
    void     *map_base, *virt_addr;
    off_t    target = 0;
    unsigned map_size;
    char     ethaddr[128];

    map_size = MAP_SIZE;

    target = (off_t)0;
    
    if (is_tabeil() == TRUE) {
        if (which_port == I350_PORT2) {
            sprintf(ethaddr, "%s%s", TABEIL_I350_PCIE_BUS, TABEIL_I350_PCIE_PORT2);
        } else if (which_port == I350_PORT3) {
            sprintf(ethaddr, "%s%s", TABEIL_I350_PCIE_BUS, TABEIL_I350_PCIE_PORT3);
        } else {
            printf("Wrong port number: %d\n", (int)which_port);
            return (FAILED);
        }
    } else if (is_promethium() == TRUE) {
        if (which_port == I350_PORT2) {
            sprintf(ethaddr, "%s%s", PROMETHIUM_I350_PCIE_BUS, PROMETHIUM_I350_PCIE_PORT2);
        } else if (which_port == I350_PORT3) {
            sprintf(ethaddr, "%s%s", PROMETHIUM_I350_PCIE_BUS, PROMETHIUM_I350_PCIE_PORT3);
        } else {
            printf("Wrong port number: %d\n", (int)which_port);
            return (FAILED);
        }
    } else {
        printf("Wrong Platform type\n");
        return (FAILED);
    }

    fd = open(ethaddr, O_RDWR | O_SYNC);
    if (fd < 0) {
        printf("%s: Failed to open MEM device.\n", __FUNCTION__);
        return (FAILED);
    }

    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("mmap(%d, %ld, 0x%x, 0x%x, %d, 0x%x)\n", 0, MAP_SIZE,
                PROT_READ | PROT_WRITE, MAP_SHARED, fd, (int) target);
        fflush(stdout);
    }

    map_base = mmap(NULL, map_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd,
                    target & ~MAP_MASK);
    if (map_base == MAP_FAILED) {
        printf("%s: Failed to map in virtual address space.\n",
               __FUNCTION__);
         close(fd);
         return (FAILED);
    }
    virt_addr = map_base + offset;

    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("I350 Port :%d, Device Control Register - CTRL: %x\n", 
               (int)which_port, *(volatile uint32_t*)virt_addr);
    }

    *(volatile uint32_t*)virt_addr = val;

    if (munmap(map_base, MAP_SIZE) == -1) {
        printf("%s: Failed to munmap.\n", __FUNCTION__);
        close(fd);
        return (FAILED);
    }
    close(fd);



    return (PASSED);
}
/******************************************************************************
 *
 * Function: check_phy_copper_link_mode
 *
 * Description: This function to check the media type of sfp
 *
 * Inputs      : which_port - which sfp port 
 * Outputs     : TRUE / FALSE
 *
 *****************************************************************************/
int check_phy_copper_link_mode (int which_port)
{
    uint link_mode, i350_ctrl_val = 0;
    ushort phy_val = 0; 

    i350_read_ctrl_reg(which_port, &i350_ctrl_val, TABEIL_I350_CTRL_EXT);
    link_mode = i350_ctrl_val & TABEIL_I350_CTRL_EXT_LINK_MODE_MASK;
    /* Make sure from both I350 reg and SFP PHY chip */
    if (link_mode == TABEIL_I350_CTRL_EXT_LINK_MODE_SGMII) {
        igb_read_sfp_phy(which_port, PHY_IDENTIFIER_REG, &phy_val);
        phy_val &= PHY_IDENTIFIER_MASK; 
         if (phy_val == PHY88E1111_IDENTIFIER) {
           return (TRUE);
        } else {
           return (FALSE);
        }
    } else {
        return (FALSE);
    }
}

/******************************************************************************
 *
 * Function: i350_sfp_i2c_test
 *
 * Description: This function to verify FPGA I2C to SFP path
 *
 * Inputs      : option
 * Outputs     : PASSED/FAILED
 *
 *****************************************************************************/
int i350_sfp_i2c_test(int option)
{
    char fpga_vendor_data[32] = {0};
    char intel_vendor_data[32] = {0};
    int ix, index, start_port, end_port;
    char *tname = "I350 SFP I2C";
    int which_port;

    testname("%s", tname);

    /* according to menu_show_dflags(), D_EXT_LPBK is inverse flag */
    if ((NVRAM)->diagflag & D_EXT_LOOPBACK) {
        printf("External Loopback flag is off. Skip the test\n");
        return (PASSED);
    } 

    if (option) {
        which_port = getdec_answer("\nEnter I350 Port 2/3 SFP ", 2, 2, 3);
        start_port = which_port;
        end_port = which_port;
    } else {
        start_port = I350_PORT2;
        end_port = I350_PORT3;
    }


    for (which_port = start_port; which_port <= end_port; which_port++) {
        prpass(testpass, "SFP port %d ", which_port);


        if (diag_fpga_i2c_read_sfp_vendor_name(which_port, fpga_vendor_data) != PASSED) {
            cterr('f', 0, "%s, Cannot read SFP vendor name via FPGA\n", __FUNCTION__);
            return (FAILED);
        }

        if (igb_read_sfp_vendor_name(which_port, intel_vendor_data) != PASSED) {
            cterr('f', 0, "%s, Cannot read SFP vendor name via Intel\n", __FUNCTION__);
            return (FAILED);
        }

        for (ix = SFP_VENDOR_NAME_20; ix <= SFP_VENDOR_NAME_35; ix++) {
            index = ix - SFP_VENDOR_NAME_20;
            if (fpga_vendor_data[index] != intel_vendor_data[index]) {
                printf("FPGA read vendor name index %d: %x\n", index, fpga_vendor_data[index]);
                printf("Intel read vendor name: index %d: %x\n",index, intel_vendor_data[index]);
                cterr('f', 0, "%s, Verify FPGA I2C failed\n", __FUNCTION__);
                return (FAILED);
            }
        }

        if ((NVRAM)->diagflag & D_VERBOSE) {
            printf("FPGA read vendor name: %s\n", fpga_vendor_data);
            printf("Intel read vendor name: %s\n", intel_vendor_data);
        }  
    }
    
    prcomplete(testpass, errcount, (char *)0);

    return (PASSED);
}

/******************************************************************************
 *
 * Function: set_sfp_phy_module
 *
 * Description: This function to setting the phy configuration
 *              on the sfp module.
 *
 * Inputs      : which_port - which sfp port 
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/
int set_sfp_phy_module (int which_port)
{
    uint ret_val = FAILED;
    ushort data = 0;

    ret_val = igb_read_sfp_phy(which_port, PHY88E1111_EXTEND_PHY_SPE_STATE_REG, &data);
    if (ret_val != PASSED) {
        return (ret_val);
    }
   
    /* Disable auto-neg 1000BASE-X */
    /* Change HWCFG MODE to non-GBIC */
    data = PHY88E1111_CHANGE_NON_BGIC;
    ret_val = igb_write_sfp_phy(which_port, PHY88E1111_EXTEND_PHY_SPE_STATE_REG, data);
    if (ret_val != PASSED) {
        return (ret_val);
    }

    /* Apply Software Reset */
    data = PHY88E1111_SOFTWARE_INIT; 
    ret_val = igb_write_sfp_phy(which_port, PHY88E1111_CTRL_REG, data);
    if (ret_val != PASSED) {
        return (ret_val);
    }
    /* Set register 0x0 bit 14 to 1, disable loopback register */
    ret_val = igb_read_sfp_phy(which_port, PHY88E1111_CTRL_REG, &data);
    data |= PHY88E1111_DIASABLE_LPBK; 
    ret_val = igb_write_sfp_phy(which_port, PHY88E1111_CTRL_REG, data);
    if (ret_val != PASSED) {
        return (ret_val);
    }
    /* Enable 1000 Mbps stub */
    ret_val = igb_write_sfp_phy(which_port, PHY88E1111_INTERRUPT_EN_REG, 
                                PHY88E1111_DISABLE_INTERRUPT);

    ret_val = igb_read_sfp_phy(which_port, PHY88E1111_1000BASET_CTRL_REG, &data);
    data |= PHY88E1111_MASTER_CONFIG;
    ret_val = igb_write_sfp_phy(which_port, PHY88E1111_1000BASET_CTRL_REG, data);

    ret_val = igb_read_sfp_phy(which_port, PHY88E1111_CTRL_REG, &data);
    data |= PHY88E1111_SOFTWARE_RESET;
    ret_val = igb_write_sfp_phy(which_port, PHY88E1111_CTRL_REG, data);

    ret_val = igb_write_sfp_phy(which_port, PHY88E1111_EXTEND_ADDR_REG, 
                                PHY88E1111_EXTEND_PAGE7);

    ret_val = igb_read_sfp_phy(which_port, PHY88E1111_EXTENDED_REG, &data);
    data |= PHY88E1111_FORCE_GIGABIT_MODE;
    ret_val = igb_write_sfp_phy(which_port, PHY88E1111_EXTENDED_REG, data);

    ret_val = igb_write_sfp_phy(which_port, PHY88E1111_EXTEND_ADDR_REG, 
                                PHY88E1111_EXTEND_PAGE16);
    
    ret_val = igb_read_sfp_phy(which_port, PHY88E1111_EXTENDED_REG, &data);
    data |= PHY88E1111_EN_GIGABIT_STUB_LPBK;
    ret_val = igb_write_sfp_phy(which_port, PHY88E1111_EXTENDED_REG, data);

    ret_val = igb_write_sfp_phy(which_port, PHY88E1111_EXTEND_ADDR_REG, 
                                PHY88E1111_EXTEND_PAGE18);

    ret_val = igb_read_sfp_phy(which_port, PHY88E1111_EXTENDED_REG, &data);
    data |= PHY88E1111_DISABLE_NEXT_CANCELLER;
    ret_val = igb_write_sfp_phy(which_port, PHY88E1111_EXTENDED_REG, data);

    return (PASSED);

}

/*-------------------------------------------------
$Log: diag_i350_test.c,v $
Revision 1.4  2020/08/06 07:54:55  kehuang2
Collapse Promethium into main trunk

Revision 1.3  2019/12/30 06:00:20  kehuang2
CSCvs55860: Implement I350 disable SFP test

Revision 1.2  2019/10/17 02:16:22  kehuang2
Collapse Tabei-L into main trunk

Revision 1.1.2.18  2019/09/20 07:00:34  kehuang2
Clean up code base on review comment

Revision 1.1.2.17  2019/09/18 01:27:06  kehuang2
Suppout the PHY chip config on sfp for I350

Revision 1.1.2.16  2019/09/10 06:15:42  olin2
Code clean up

Revision 1.1.2.15  2019/09/03 09:04:03  olin2
Enhance SFP loopback test

Revision 1.1.2.14  2019/08/29 07:29:37  olin2
Support read spcific SFP PHY util

Revision 1.1.2.13  2019/08/26 08:13:04  olin2
Support read SFP EEPROM

Revision 1.1.2.12  2019/07/30 02:03:44  olin2
Enhance SFP loopback test

Revision 1.1.2.11  2019/07/26 08:25:33  olin2
Code clean up

Revision 1.1.2.10  2019/07/04 03:20:19  kehuang2
Update with udev modified

Revision 1.1.2.9  2019/03/21 06:17:38  olin2
Remove Error character from celo result

Revision 1.1.2.8  2018/12/21 07:09:11  olin2
Correct I350 menu

Revision 1.1.2.7  2018/12/06 02:10:09  olin2
Code clean up

Revision 1.1.2.6  2018/12/05 11:06:24  olin2
Support External loopback flag

Revision 1.1.2.5  2018/12/04 08:12:45  olin2
Update check link

Revision 1.1.2.4  2018/12/04 03:11:19  olin2
Update I350 interface name for Cisco BIOS

Revision 1.1.2.3  2018/11/09 07:16:43  olin2
Update I350 ethernet name

Revision 1.1.2.2  2018/11/01 01:48:26  olin2
Support I350 test

Revision 1.1.2.1  2018/10/03 06:06:38  olin2
Initial commit for I350 test



$Endlog$
*/
