/* $Id: bcm57412_test.c,v 1.2 2021/01/11 11:03:28 xiaolaya Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/overlord/neptune/curie_2RU/bcm57412_test.c,v $
 *-----------------------------------------------------------------------------
 * bcm57412_test.c - Diags Test for BCM57412 10G NIC.
 *
 * July 2018, Leschen 
 *
 * Copyright (c) 2019 by Cisco Systems, Inc.
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
#include <fcntl.h>

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
#include "bcm57412_test.h"
#include "dash_fpga.h"
#include "curie2ru.h"

#define MF_1    (MF_CONTINUOUS | MF_DOGRP)
#define MF_2    (MF_1 | MF_DOALL)
#define MF_3    (MF_2 | MF_SHOW_ERRCOUNT)
#define MF_4    (MF_1 | MF_SHOW_ERRCOUNT)

static int BCM57412_utility(int);
static void BCM57412_fw_mac_program(void);
int bcm57412_internal_loopback_test(int);
int bcm57412_external_loopback_test(int);
int bcm57412_sfp_plus_i2c_test(int);
int check_bcm57412_driver(void);
int set_bcm57412_1g_speed(int);
static void dump_sfp_eeprom(void);
static void bcm57412_speed_setting(void);
static void bcm57412_display_cap(void);
extern char *strcasestr(char* , char*);
static int bcm57412_sideband_tx_dis (int);

/******************************************************************************
 *  List of Menu used for Broadcom BCM57412 10G Speed
 *****************************************************************************/
static submenu_xtable_t BCM57412_10G_tests_submenu_table[] = {
   {"BCM57412 Port1 Internal Loopback Test", (type_t(*)())bcm57412_internal_loopback_test,   1,
     MF_3, (type_t(*)())0, 0, (type_t(*)())0,   0},
   {"BCM57412 Port2 Internal Loopback Test", (type_t(*)())bcm57412_internal_loopback_test,   2,
     MF_3, (type_t(*)())0, 0, (type_t(*)())0,   0},
   {"BCM57412 Port1 SFP+ External Loopback Test", (type_t(*)())bcm57412_external_loopback_test,   1,
     MF_3, (PFT) is_curie_1ru, 0, (type_t(*)())0,   0},
   {"BCM57412 Port2 SFP+ External Loopback Test", (type_t(*)())bcm57412_external_loopback_test,   2,
     MF_3, (PFT) is_curie_1ru, 0, (type_t(*)())0,   0},
   {"BCM57412 Port1 SFP+ i2c Test", (type_t(*)())bcm57412_sfp_plus_i2c_test,   1,
     MF_3, (type_t(*)())0, 0, (type_t(*)())0,   0},
   {"BCM57412 Port2 SFP+ i2c Test", (type_t(*)())bcm57412_sfp_plus_i2c_test,   2,
     MF_3, (type_t(*)())0, 0, (type_t(*)())0,   0},
   {"BCM57412 Utility", (type_t(*)())BCM57412_utility,   FALSE,
    0, NULL, 0, (type_t(*)())BCM57412_utility,   TRUE},
};

/******************************************************************************
 *  List of Menu used for Broadcom BCM57412 1G Speed
 *****************************************************************************/
static submenu_xtable_t BCM57412_1G_tests_submenu_table[] = {
   {"BCM57412 Port1 Internal Loopback Test", (type_t(*)())bcm57412_internal_loopback_test,   1,
     MF_3, (type_t(*)())0, 0, (type_t(*)())0,   0},
   {"BCM57412 Port2 Internal Loopback Test", (type_t(*)())bcm57412_internal_loopback_test,   2,
     MF_3, (type_t(*)())0, 0, (type_t(*)())0,   0},
   {"BCM57412 Port1 SFP External Loopback Test", (type_t(*)())bcm57412_external_loopback_test,   1,
     MF_3, (PFT) is_curie_1ru, 0, (type_t(*)())0,   0},
   {"BCM57412 Port2 SFP External Loopback Test", (type_t(*)())bcm57412_external_loopback_test,   2,
     MF_3, (PFT) is_curie_1ru, 0, (type_t(*)())0,   0},
   {"BCM57412 Port1 SFP i2c Test", (type_t(*)())bcm57412_sfp_plus_i2c_test,   1,
     MF_3, (type_t(*)())0, 0, (type_t(*)())0,   0},
   {"BCM57412 Port2 SFP i2c Test", (type_t(*)())bcm57412_sfp_plus_i2c_test,   2,
     MF_3, (type_t(*)())0, 0, (type_t(*)())0,   0},
   {"BCM57412 Utility", (type_t(*)())BCM57412_utility,   FALSE,
    0, NULL, 0, (type_t(*)())BCM57412_utility,   TRUE},
};

/******************************************************************************
 *  List of Utilities used for BCM57412
 *****************************************************************************/
static submenu_xtable_t BCM57412_util_items[] = {
    {"BCM57412 Firmware Download", (type_t(*)())BCM57412_fw_mac_program, 0, 0,
     (type_t(*)())0, 0, (type_t(*)())0,   0},
    {"Dump External Module EEPROM", (type_t(*)())dump_sfp_eeprom, 0, 0,
     (type_t(*)())0, 0, (type_t(*)())0, 0},
    {"BCM57412 Speed Switch", (type_t(*)())bcm57412_speed_setting, 0, 0,
     (type_t(*)())0, 0, (type_t(*)())0, 0},
    {"Display BCM57412 ports capability", (type_t(*)())bcm57412_display_cap, 0, 0,
     (type_t(*)())0, 0, (type_t(*)())0, 0},
    {"BCM57412 side band enable tx_dis", (type_t(*)())bcm57412_sideband_tx_dis, ENABLE, 0,
     (type_t(*)())0, 0, (type_t(*)())0, 0},
    {"BCM57412 side band disable tx_dis", (type_t(*)())bcm57412_sideband_tx_dis, DISABLE, 0,
     (type_t(*)())0, 0, (type_t(*)())0, 0},
};

/******************************************************************************
 *  Macro Definitions
 *****************************************************************************/
#define BCM57412_TESTS_SUBMENU_TABLE_SIZE (sizeof(BCM57412_10G_tests_submenu_table) / \
                                           sizeof(submenu_xtable_t))
#define BCM57412_TESTS_UTIL_SIZE (sizeof(BCM57412_util_items) / \
                                  sizeof(submenu_xtable_t))

/******************************************************************************
 *  Global Variable
 *****************************************************************************/
/******************************************************************************
 * Primary & secondary submenu items (filled in from xtable)
 *****************************************************************************/
static mitem_t BCM57412_tests_primary_items[BCM57412_TESTS_SUBMENU_TABLE_SIZE +
                                            MAX_BASE_ITEMS];
static mitem_t BCM57412_tests_secondary_items[BCM57412_TESTS_SUBMENU_TABLE_SIZE +
                                              MAX_BASE_ITEMS];

/******************************************************************************
 * Primary & secondary utilities menu items (filled in from xtable)
 *****************************************************************************/
static mitem_t BCM57412_tests_primary_util_items[BCM57412_TESTS_UTIL_SIZE +
                                                 MAX_BASE_ITEMS];
static mitem_t BCM57412_tests_secondary_util_items[BCM57412_TESTS_UTIL_SIZE +
                                                   MAX_BASE_ITEMS];

/******************************************************************************
 * BCM57412 Utils submenu
 *****************************************************************************/
menuinfo_t BCM57412_util_menu = {
    "BCM57412 Utility Menu",
    0,
    (PFT)show_endnote,
    0,
    0,
    BCM57412_tests_primary_util_items,
};
menuinfo_t *BCM57412_util_menup = &BCM57412_util_menu;

menuinfo_t BCM57412_subtest_menu = {
    "%s Subtest Menu",
    0,                                /* mtparam added by init_empty_menu */
    (PFT)show_endnote,                /* notes missing WICs in combos */
    0,                                /* use generic prompt */
    0,                                /* size (bumped by add_menu_item() */
    BCM57412_tests_primary_items,
};
menuinfo_t *BCM57412_submenup = &BCM57412_subtest_menu;

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
int ten_g_bcm57412_test (int show_menu)
{
    if (is_thorium()) {
        /* Thorium BCM57412 two ports are running at speed 1G */
        if (set_bcm57412_1g_speed(MB_57412_PORT1) == FAILED) {
            printf("Failed to set Thallium bcm57412 port1 at speed 1G\n");
            system(DISPLAY_PORT1_CAP);
        } 

        if (set_bcm57412_1g_speed(MB_57412_PORT2) == FAILED) {
            printf("Failed to set Thallium bcm57412 port2 at speed 1G\n");
            system(DISPLAY_PORT2_CAP);
        }
    } else if (is_uranium()) {
        system(BCM57412_PORT1_10G_SPEED);
        system(BCM57412_PORT2_10G_SPEED);
    }

    if (is_uranium()) {
        build_primary_submenu(BCM57412_10G_tests_submenu_table,
                              BCM57412_TESTS_SUBMENU_TABLE_SIZE,
                              "10G NIC BCM57412", &BCM57412_submenup);
        build_secondary_submenu(BCM57412_10G_tests_submenu_table,
                                BCM57412_TESTS_SUBMENU_TABLE_SIZE,
                                BCM57412_tests_secondary_items);
    } else {
        build_primary_submenu(BCM57412_1G_tests_submenu_table,
                              BCM57412_TESTS_SUBMENU_TABLE_SIZE,
                              "1G NIC BCM57412", &BCM57412_submenup);
        build_secondary_submenu(BCM57412_1G_tests_submenu_table,
                                BCM57412_TESTS_SUBMENU_TABLE_SIZE,
                                BCM57412_tests_secondary_items);
    }

    if (show_menu) {
        exec_doall_menu_items(BCM57412_submenup);
    } else {
        menu(BCM57412_submenup, BCM57412_tests_secondary_items, '\0' );
    }

    return (PASSED);
}

/*******************************************************************************
 *
 * Function    : BCM57412_utility
 * Description :
 * Inputs      : menu_option - display menu instead of running all XAUI 88X2222M
 *               tests.
 * Outputs     : PASSED / FAILED
 *
 ******************************************************************************/
static int BCM57412_utility (int show_menu)
{
    build_primary_submenu(BCM57412_util_items, BCM57412_TESTS_UTIL_SIZE,
                          "BCM57412 Utilities Menu", &BCM57412_util_menup);
    build_secondary_submenu(BCM57412_util_items, BCM57412_TESTS_UTIL_SIZE,
                            BCM57412_tests_secondary_util_items);

    menu(BCM57412_util_menup, BCM57412_tests_secondary_util_items, '\0' );

    return (PASSED);
}

#define F_GRP        (MF_CONTINUOUS | MF_DOGRP)
#define F_GRP_E      (F_GRP | MF_SHOW_ERRCOUNT)
#define F_ALL        (F_GRP | MF_DOALL)
#define F_ALL_E      (F_ALL | MF_SHOW_ERRCOUNT)

/*
 * Function: bcm57412_speed_setting
 *
 * Description: bcm57412 speed configuration
 *
 * Input: none
 *
 * Return: none
 */
static void bcm57412_speed_setting (void)
{
    uint opt = 0;

    printf("1 - Config port 1 speed to 1G\n");
    printf("2 - Config port 1 speed to 10G\n");
    printf("3 - Config port 2 speed to 1G\n");
    printf("4 - Config port 2 speed to 10G\n");
    opt = getdec_answer("\nPlease enter your selection : ", 1, 0, 10);

    system(SUPPRESS_MESG);
    if (opt == 1) {
        printf("Currently BCM57412 port 1(eth4) is running at speed -\n");
        system(BCM57412_PORT1_SPEED_GET);
        system(BCM57412_PORT1_1G_SPEED);
        printf("After setting BCM57412 port 1(eth4) is running at speed -\n");
        system(BCM57412_PORT1_SPEED_GET);
    } else if (opt == 2) {
        printf("Currently BCM57412 port 1(eth4) is running at speed -\n");
        system(BCM57412_PORT1_SPEED_GET);
        system(BCM57412_PORT1_10G_SPEED);
        printf("After setting BCM57412 port 1(eth4) is running at speed -\n");
        system(BCM57412_PORT1_SPEED_GET);
    } else if (opt == 3) {
        printf("Currently BCM57412 port 2(eth5) is running at speed -\n");
        system(BCM57412_PORT2_SPEED_GET);
        system(BCM57412_PORT2_1G_SPEED);
        printf("After setting BCM57412 port 2(eth5) is running at speed -\n");
        system(BCM57412_PORT2_SPEED_GET);
    } else if (opt == 4) {
        printf("Currently BCM57412 port 2(eth5) is running at speed -\n");
        system(BCM57412_PORT2_SPEED_GET);
        system(BCM57412_PORT2_10G_SPEED);
        printf("After setting BCM57412 port 2(eth5) is running at speed -\n");
        system(BCM57412_PORT2_SPEED_GET);
    } else {
        printf("****Warning - Not support option %d\n", opt);
    }
}

/*
 * Function: check_bcm57412_driver 
 *
 * Description: Insert bnxt_en driver if it doesn't exist 
 *
 * Input: none
 *
 * Return: PASSED/FAILED 
 */
int check_bcm57412_driver (void)
{
    char *check_driver_file = "/curie-1RU-diag/check_57412_driver";
    char *driver_name = "bnxt_en";
    char cmd[128], rd_driver[128];
    int rc = FAILED;
    FILE *fp;

    sprintf(cmd, "lsmod | grep bnxt_en | sed 's/..........$//'> %s", check_driver_file);
    system(cmd);

    fp = fopen(check_driver_file, "r");
    if (fp == NULL) {
        printf("Unable to open file - %s\n", check_driver_file);
        return (rc);
    }

    while (!feof(fp)) {
        fgets(rd_driver, sizeof(rd_driver), fp);
        if (strcasestr(rd_driver, driver_name) != NULL) {
            rc = PASSED;
        }
    }

    sprintf(cmd, "rm -f %s", check_driver_file);
    system(cmd);
    fclose(fp);

    if (rc != PASSED) {
        printf("modprobe bnxt_en\n");
        system(MODPROBE_BCM57412_DRIVER);
        msleep(1000);
        rc = PASSED;
    }

    return (rc);
}

/*
 * Function: set_bcm57412_1g_speed 
 *
 * Description: Config bcm57412 ports at 1G speed 
 *
 * Input: port - bcm57412 port number 
 *
 * Return: PASSED/FAILED 
 */
int set_bcm57412_1g_speed (int port)
{
    char *check_speed_file = "/tmp/bcm57412_1g_speed";
    char *speed = "1000";
    char cmd[128], rd_speed[128];
    int rc = FAILED, ix, max_num = 4;
    FILE *fp;

    printf("Config MB Bcm57412 port%d at speed 1G\n", port);
    for (ix = 0; ix <= max_num; ix++) {
        if (port == MB_57412_PORT1) {
            system(BCM57412_PORT1_1G_SPEED);
            msleep(500);
        } else if (port == MB_57412_PORT2) {
            system(BCM57412_PORT2_1G_SPEED);
            msleep(500);
        } else {
            printf("Unkonwn port - %d\n", port);
            return (rc);
        }

        sprintf(cmd, "ethtool eth%d | grep Speed | sed 's/....$//' | sed s/[[:space:]]//g | sed 's/^......//'> %s", port, check_speed_file);
        system(cmd);

        fp = fopen(check_speed_file, "r");
        if (fp == NULL) {
            printf("Unable to open file - %s\n", check_speed_file);
            return (rc);
        }

        while (!feof(fp)) {
            fgets(rd_speed, sizeof(rd_speed), fp);
            if (strcasestr(rd_speed, speed) != NULL) {
                rc = PASSED;
            }
        }

        sprintf(cmd, "rm -f %s", check_speed_file);
        system(cmd);
        fclose(fp);

        if (rc != PASSED) {
            printf("****Reconfig bcm57412 port%d at speed 1G\n", port);
        } else {
            break;
        }
    }

    return (rc);
}

/*
 * Function: bcm57412_display_cap
 *
 * Description: bcm57412 display capability
 *
 * Input: none
 *
 * Return: none
 */
static void bcm57412_display_cap (void)
{
    system(DISPLAY_PORT1_CAP);
    system(DISPLAY_PORT2_CAP);
}

/******************************************************************************
 *
 * Function: BCM57412_fw_mac_program
 *
 * Description: This function guides BCM57412 firmware and mac programming.
 *
 * Inputs      : NONE
 *
 * Outputs     : NONE
 *
 ********************************************************************************/
static void BCM57412_fw_mac_program (void)
{
    printf("This utility uses Broadcom CDiag tool by executing script load.sh\n");
    printf("Please refer to /doc/bcm57412_nic_mac_eeprom_program.doc to operate firmware and mac programming\n"); 
}

static int has_firmware_216_1_58_0(int idx)
{
    int rc;
    char cmd[256];

    snprintf(cmd, sizeof(cmd), "ethtool -i eth%d | grep 'pkg 216.1.58.0'", idx);
    rc = system(cmd);
    if (rc < 0) {
        printf("system returned %d\n", rc);
        return 0;
    }

    return WEXITSTATUS(rc) == 0;
}

/******************************************************************************
 *
 * Function: BCM57412_internal_loopback_test
 *
 * Description: This function perform the internal loopback test
 *              from CPU to BCM57412 MAC and PHY.
 *
 * Inputs      : port - port number 
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/
int bcm57412_internal_loopback_test (int port)
{
    char *tname = "BCM57412 internal loopback";
    char buf[128];
    FILE *fp;
    int retval = PASSED;

    system(SUPPRESS_MESG);
    testname("%s port %d", tname, port);

    if (has_firmware_216_1_58_0(MB_57412_PORT1 + (port - 1))) {
        printf("Port %d: found Broadcom firmware 216.1.58.0 that has a known "
                "loopback bug.\n", port);
        printf("Skipping internal loopback test\n");
        return PASSED;
    }

    memset(buf, 0, BUFFER_ARRAY_SIZE);

    if (port == BCM57412_PORT1) {
        system(BCM57412_PORT1_INT_LPBK);
        msleep(200);
    } else {
        system(BCM57412_PORT2_INT_LPBK);
        msleep(200);
    }

    fp = fopen(BCM57412_LPBK_RESULT, "r");
    if (fp == NULL) {
        system(OPEN_MESG);
        cterr('f',0, "Failed to open file bcm57412_lpbk.txt");
        return (FALSE);
    }

    while (!feof(fp)) {
        fgets(buf, sizeof(buf), fp);
        if (strstr(buf, BCM57412_FAIL) != NULL) {
            cterr('f',0, "port %d failed, ", port);
            retval = FAILED;
            break;
        } else {
            prpass(testpass, "port %d passed, ", port);
            break;
        }
    }

    fclose(fp);
    system(OPEN_MESG);
    return (retval);
}

/******************************************************************************
 *
 * Function: BCM57412_external_loopback_test
 *
 * Description: This function perform the external loopback test from CPU to
 *              BCM57412 SFP+ (for Curium)/BCM57412 SFP (for Thallium)
 *
 * Inputs      : port - port number 
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/
int bcm57412_external_loopback_test (int port)
{
    char *tname = "BCM57412 external loopback";
    char buf[128];
    char cmd[128];
    FILE *fp;
    int retval = PASSED;

    testname("%s port %d", tname, port);

    if ((NVRAM)->diagflag & D_EXT_LOOPBACK) {
        printf("External loopback flag is off, skip the BCM57412 port %d external loopback test\n", port);
        return (retval);
    }

    memset(buf, 0, BUFFER_ARRAY_SIZE);

    system(SUPPRESS_MESG);

    if (port == BCM57412_PORT1) {
        port = MB_57412_PORT1;
        sprintf(cmd, "\nifconfig eth%d up\n", port);
        system(cmd);
        msleep(1000);

        system(BCM57412_PORT1_EXT_LPBK);
        msleep(200);
    } else {
        port = MB_57412_PORT2;
        sprintf(cmd, "\nifconfig eth%d up\n", port);
        system(cmd);
        msleep(1000);

        system(BCM57412_PORT2_EXT_LPBK);
        msleep(200);
    }

    fp = fopen(BCM57412_LPBK_RESULT, "r");
    if (fp == NULL) {
        system(OPEN_MESG);
        cterr('f',0, "Failed to open file bcm57412_lpbk.txt");
        return (FALSE);
    }

    while (!feof(fp)) {
        fgets(buf, sizeof(buf), fp);
        if (strstr(buf, BCM57412_FAIL) != NULL) {
            cterr('f',0, "port %d failed, ", port);
            retval = FAILED;

            if (port == BCM57412_PORT1) {
                port = MB_57412_PORT1;
            } else {
                port = MB_57412_PORT2;
            }

            sprintf(cmd, "\nethtool -S eth%d\n", port);
            system(cmd);
            sprintf(cmd, "\nifconfig eth%d\n", port);
            system(cmd);
            sprintf(cmd, "\nethtool eth%d\n", port);
            system(cmd);
            sprintf(cmd, "\ndmesg | tail -n 20\n");
            system(cmd);

            break;
        } else {
            prpass(testpass, "port %d passed, ", port);
            break;
        }
    }

    fclose(fp);
    system(OPEN_MESG);
    return (retval);
}

#define BCM57412_SFP_I2C_TEST_SCRIPT    \
"#!/bin/sh\n" \
"\n" \
"if [ -z \"$1\" ]; then\n" \
"        echo \"Please specify an ethernet interface\"\n" \
"        exit 1\n" \
"fi\n" \
"\n" \
"ether=$1\n" \
"data=$(ethtool -m $ether offset 0 length 512 hex on 2>&1)\n" \
"if echo \"$data\" | grep -q 'Cannot get module EEPROM information:'; then\n" \
"        ifconfig $ether down; ifconfig $ether up\n" \
"        data=$(ethtool -m $ether offset 0 length 512 hex on 2>&1)\n" \
"fi\n" \
"\n" \
"if echo \"$data\" | grep -q 'Cannot get module EEPROM information:'; then\n" \
"        echo \"Error: $ether: No SFP module found, please check\"\n" \
"        exit 2\n" \
"fi\n" \
"\n" \
"if echo \"$data\" | grep -q '^0x0000:\\s\\+03 04'; then\n" \
"        type=$(ethtool -m $ether offset 0 length 512 | grep \"Transceiver type\" | cut -d ':' -f 2-)\n" \
"        echo \"$ether: found transceiver: $type\"\n" \
"        exit 0\n" \
"fi\n" \
"\n" \
"echo \"Error: $ether: unidentified data, SFP unplugged? Need to check\"\n" \
"echo \"$data\" | head -n 3\n" \
"exit 3"

static int c2ru_bcm57412_sfp_i2c_test(int port)
{
    int fd, rc, ret;
    const char *i2c_script = "/tmp/bcm57412_i2c_test.sh";
    const char *ether = (port == BCM57412_PORT1) ? "eth4" : "eth5";
    size_t script_len = sizeof(BCM57412_SFP_I2C_TEST_SCRIPT) - 1;
    ssize_t len;
    char cmd[1024];

    snprintf(cmd, sizeof(cmd), "rm -f %s", i2c_script);
    system(cmd);

    fd = open(i2c_script, O_RDWR | O_CREAT, 0644);
    if (fd == -1) {
        cterr('f', 0, "failed to open %s, ", i2c_script);
        return FAILED;
    }

    len = write(fd, BCM57412_SFP_I2C_TEST_SCRIPT, script_len);
    if (len != script_len) {
        close(fd);
        cterr('f', 0, "failed to write %s: error %d, ", i2c_script, len);
        return FAILED;
    }

    close(fd);

    snprintf(cmd, sizeof(cmd), "sh %s %s", i2c_script, ether);
	ret = system(cmd);
	if (ret < 0) {
		cterr('f', 0, "failed to execute %s\n: error %d, ", i2c_script, ret);
		return FAILED;
	}

	rc = WEXITSTATUS(ret);
	if (rc != 0) {
		cterr('f', 0, "test failed, error %d, ", rc);
        return FAILED;
    }

    prpass(testpass, "port %d passed, ", port);

    return PASSED;
}

/******************************************************************************
 *
 * Function: BCM57412_sfp_plus_i2c_test
 *
 * Description: This function checks SFP+(for Curium)/SFP(for Thallium)
 *              cookie byte 0 and byte 1 to make sure the i2c bus 
 *              between BCM57412 and SFP+/SFP module is good.
 *
 * Inputs      : port - port number 
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/
int bcm57412_sfp_plus_i2c_test (int port)
{
    char *tname = "BCM57412 i2c";
    char buf[128];
    FILE *fp;
    int retval = PASSED;

    testname("%s port %d", tname, port);

    memset(buf, 0, BUFFER_ARRAY_SIZE);

    if ((NVRAM)->diagflag & D_EXT_LOOPBACK) {
        printf("External loopback flag is off, skip the BCM57412 port %d i2c test\n", port);
        return (retval);
    }

    if (is_curie_2ru()) {
        return c2ru_bcm57412_sfp_i2c_test(port);
    }

    system(REMOVE_BCM57412_DRIVER);
    msleep(500);
    if (is_curie_2ru())
        chdir(ENTER_2RU_BCM57412_SCRIPT_DIR);
    else
        chdir(ENTER_BCM57412_SCRIPT_DIR);
    system(SUPPRESS_MESG);
    if (port == BCM57412_PORT1) {
        system(BCM57412_PORT1_I2C);
    } else {
        system(BCM57412_PORT2_I2C);
    }

    fp = fopen(BCM57412_I2C_RESULT, "r");
    if (fp == NULL) {
        system(OPEN_MESG);
        system(MODPROBE_BCM57412_DRIVER);
        msleep(1000);
        cterr('f',0, "Failed to open file bcm57412_i2c_test.txt");
        return (FALSE);
    }

    while (!feof(fp)) {
        fgets(buf, sizeof(buf), fp);
        if (strstr(buf, SFP_PLUS_FIXED_ID) == NULL) {
            cterr('f',0, "port %d failed, please check whether has external loopback module", port);
            retval = FAILED;
            break;
        } else {
            prpass(testpass, "port %d passed, ", port);
            break;
        }
    }

    fclose(fp);
    system(OPEN_MESG);
    system(MODPROBE_BCM57412_DRIVER);
    msleep(1000);
    return (retval);
}

static void dump_sfp_eeprom (void)
{
    system(BCM57412_SFP_PLUS_EEPROM_DUMP);
}


/******************************************************************************
 * bcm57412 sideband tx disable function */

typedef struct bnxt_ipc_mng_ {
    int      socket_fd;
    int      cpr_kernel_session;
    boolean  is_init_was_done;
} bnxt_ipc_mng_t;

static bnxt_ipc_mng_t g_mng;
static boolean validate_next_buf(int remaining, int next_len)
{
    return (remaining >= (NLA_HDRLEN + next_len));
}

/******************************************************************************
 *
 * Function: get_gelmnsg_nla_data
 *
 * Description: get message from nal data
 *
 * Inputs      : na: netlink attribute
 *
 * Outputs     : Data address offset.
 *
 *****************************************************************************/
static inline void *get_gelmnsg_nla_data(struct nlattr *na)
{
    return ((void *)((char*)(na) + NLA_HDRLEN));
}

/******************************************************************************
 *
 * Function: send_gnl_msg
 *
 * Description: This function checks sfp+ cookie byte 0 and byte 1 to make
 *              sure the i2c bus between BCM57412 and SFP module is good.
 *
 * Inputs      : sd   - Socket ID
 *               *msg - command message
 *               *ans - response message
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/
static int send_gnl_msg(int sd,  bcm_nl_request_msg_t *msg,
                        bcm_nl_request_msg_t *ans)
{
    struct sockaddr_nl nladdr;
    memset(&nladdr, 0, sizeof(nladdr));
    nladdr.nl_family = AF_NETLINK;

    char *buf = (char *)msg;
    int left_to_send = msg->n.nlmsg_len;
    int resp_len = 0;

    while (left_to_send) {
        int sent_len = sendto(sd, buf, left_to_send, 0,
                             (struct sockaddr *) &nladdr,
                              sizeof(nladdr));
        if (sent_len > left_to_send) {
            printf("\nFUGAZI_NL: send_gnl_msg, sent_len > left_to_send");
            return (FAILED);
        }

         if (sent_len <= 0) {
            if (errno == EAGAIN) {
                continue;
            } else {
                printf("FUGAZI_NL: sendto returned error");
                return (FAILED);
            }
        }

        buf += (sent_len);
        left_to_send -= (sent_len);
    }

    resp_len = recv(sd, ans, sizeof(bcm_nl_request_msg_t), 0);
    if (resp_len < 0){
        printf("\nFUGAZI_NL: recv failed");
        return (FAILED);
    }

     /* Validate response message */
     if (!NLMSG_OK((&ans->n), (uint32_t)resp_len)){
        printf("\nFUGAZI_NL: invalid reply message\n");
        return (FAILED);
     }

    if (ans->n.nlmsg_type == NLMSG_ERROR) { /* error */
        printf("\nFUGAZI_NL: received error\n");
        return (FAILED);
     }

     return (PASSED);
}

/******************************************************************************
 *
 * Function: get_genlmsg_data
 *
 * Description: get message from netlink attribute stream
 *
 * Inputs      : msg: message attribute stream
 *
 * Outputs     : Data address offset.
 *
 *****************************************************************************/
static inline uint8_t *get_genlmsg_data(bcm_nl_request_msg_t *msg) {
    uint8_t *t =(uint8_t *)NLMSG_DATA(msg);
    return (t + GENL_HDRLEN);
}

/******************************************************************************
 *
 * Function: create_nl_socket
 *
 * Description: This function create a netlink socket
 *
 * Inputs      : protocol - netlonk protocol
 * Inputs      : groups   - netlonk group
 * Outputs     : Socket id
 *
 *****************************************************************************/
static int create_nl_socket(int protocol, int groups)
{
    int fd;
    struct sockaddr_nl local;

    fd = socket(AF_NETLINK, SOCK_RAW, protocol);
    if (fd < 0){
        printf("\nFUGAZI_NL: socket create error");
        return (-1);
    }

    memset(&local, 0, sizeof(local));
    local.nl_family = AF_NETLINK;
    local.nl_groups = groups;
    if (bind(fd, (struct sockaddr *) &local, sizeof(local)) < 0) {
        close(fd);
        printf("\nFUGAZI_NL: socket bind error");
        return (-1);
    }
    return fd;
}

/*
 * Probe the controller in genetlink to find the family id
 * for the CONTROL_EXMPL family
 */
/******************************************************************************
 *
 * Function: get_family_id
 *
 * Description: This function get the netlink family id
 *
 * Inputs      : sd - Socket ID
 * Outputs     : Family ID
 *
 *****************************************************************************/
static int get_family_id (int sd)
{
    bcm_nl_request_msg_t family_req;
    bcm_nl_request_msg_t ans;
    memset(&ans,0 ,sizeof(bcm_nl_request_msg_t));
    int fam_id = -1;

    /* Get family name */
    family_req.n.nlmsg_type  = GENL_ID_CTRL;
    family_req.n.nlmsg_flags = NLM_F_REQUEST;
    family_req.n.nlmsg_seq   = 0;
    family_req.n.nlmsg_pid   = getpid();
    family_req.n.nlmsg_len   = NLMSG_LENGTH(GENL_HDRLEN);
    family_req.g.cmd         = CTRL_CMD_GETFAMILY;
    family_req.g.version     = 0x1;

    struct nlattr *na = (struct nlattr *) get_genlmsg_data(&family_req);
    na->nla_type = CTRL_ATTR_FAMILY_NAME;

    na->nla_len = strlen(BNXT_NL_NAME) + 1 + NLA_HDRLEN;

    strcpy((char *)get_gelmnsg_nla_data(na), BNXT_NL_NAME);

    family_req.n.nlmsg_len += NLMSG_ALIGN(na->nla_len);

    if (send_gnl_msg(sd, &family_req, &ans) < 0) {
        return (FAILED);
    }

    na = (struct nlattr *) get_genlmsg_data(&ans);
    na = (struct nlattr *) ((char *) na + NLA_ALIGN(na->nla_len));
    if (na->nla_type == CTRL_ATTR_FAMILY_ID) {
        fam_id = *(__u16 *) get_gelmnsg_nla_data(na);
    }

    return fam_id;
}

/******************************************************************************
 *
 * Function: nla_next
 *
 * Description: next netlink attribute in attribute stream
 *
 * Inputs      : nla       - netlink attribute
 *             : remaining - number of bytes remaining in attribute stream
 * Outputs     : Returns the next netlink attribute in the attribute stream and
 *               decrements remaining by the size of the current attribute.
 *****************************************************************************/
static inline struct nlattr *nla_next (struct nlattr *nla, int *remaining)
{
        unsigned int totlen = NLA_ALIGN(nla->nla_len);

        *remaining -= totlen;
        return (struct nlattr *) ((char *) nla + totlen);
}

/******************************************************************************
 *
 * Function: construct_hdrs
 *
 * Description: This function construct the netlink.
 *
 * Inputs      : req       - Send command structure
 *               *remaining - remaining parameter
 *               *naddr     - na pointer to starting of next header
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/
static int construct_hdrs (bcm_nl_request_msg_t* req,
                           uint32_t ifindex, int *remaining,
                           struct nlattr **naddr)
{
    uint32_t data = 0;
    int next_len;
    struct nlattr *na;

    /* Send command needed */
    req->n.nlmsg_type   = g_mng.cpr_kernel_session;
    req->n.nlmsg_flags  = NLM_F_REQUEST;
    req->n.nlmsg_seq    = 0;
    req->n.nlmsg_pid    = getpid();
    req->n.nlmsg_len    = NLMSG_LENGTH(GENL_HDRLEN);
    req->g.cmd          = BNXT_CMD_HWRM;

    /* compose message */
    /* Add PID for get to the name-space */
    na = (struct nlattr *) get_genlmsg_data(req);
    na->nla_type = BNXT_ATTR_PID;
    na->nla_len = NLA_HDRLEN + sizeof(data);
    data = getpid();
    memcpy((char *)get_gelmnsg_nla_data(na), &data, sizeof(data));
    req->n.nlmsg_len += NLMSG_ALIGN(na->nla_len);

   /* Add IF_INDEX of the interface */
    na = nla_next(na, remaining);
    next_len = sizeof(data) + NLA_HDRLEN;
    if (validate_next_buf(*remaining, next_len)) {
        na->nla_type = BNXT_ATTR_IF_INDEX;
        na->nla_len = next_len; /* Message length */
        data = ifindex;
        memcpy((char *)get_gelmnsg_nla_data(na), &data, sizeof(data));
        req->n.nlmsg_len += NLMSG_ALIGN(na->nla_len);
    } else {
        printf("\nFUGAZI_NL: construct_hdrs: Failed to insert IF_INDEX");
        return (FAILED);
    }

    /* Set the na pointer to starting of next header */
    na = nla_next(na, remaining);
    *naddr = na;
    return (PASSED);
}

/******************************************************************************
 *
 * Function: bnxt_impl_deinit_netlink
 *
 * Description: This function deinit the netlink
 *
 * Inputs      : None
 * Outputs     : None
 *
 *****************************************************************************/
static void bnxt_impl_deinit_netlink (void)
{
    if (!g_mng.is_init_was_done) {
        return ;
    }
    if (g_mng.socket_fd > 0) {
        close(g_mng.socket_fd);
        g_mng.socket_fd = 0;
    }
    g_mng.is_init_was_done = FALSE;
}

/******************************************************************************
 *
 * Function: bnxt_impl_init_netlink
 *
 * Description: This function initial the netlink
 *
 * Inputs      : None
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/
static boolean bnxt_impl_init_netlink (void)
{
    if (g_mng.is_init_was_done) {
        return (PASSED);
    }

    g_mng.socket_fd = create_nl_socket(NETLINK_GENERIC, 0);
    if( g_mng.socket_fd  < 0){
        printf("\nFUGAZI_NL: create_nl_socket failed");
        return (FAILED);
    }

    g_mng.cpr_kernel_session = get_family_id(g_mng.socket_fd);

    if( g_mng.cpr_kernel_session < 0){
        printf("\nFUGAZI_NL: get_family_id failed");
        return(FAILED);
    }
    g_mng.is_init_was_done = TRUE;
    return (PASSED);
}

/******************************************************************************
 *
 * Function: bnxt_netlink_sideband_tx_dis
 *
 * Description: The firmware processes the HWRM command and payload contained
 *              in the message. Enable / Disable tx_dis sideband.
 *
 * Inputs      : port_id - Port ID
 *             : ifindex - network index
 *             : enable - enable / disable flag
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/
static int bnxt_netlink_sideband_tx_dis (uint16_t port_id, uint32_t ifindex,
                                  uint16_t enable)
{
    bcm_nl_request_msg_t req, ans;
    hwrm_port_sfp_sideband_cfg_input_t sideband_write_req;
    hwrm_port_sfp_sideband_cfg_output_t *sideband_write_resp;
    struct nlattr *na = NULL;
    int next_len = 0, remaining = BNXT_BUF_MAX;

    memset(&sideband_write_req, 0, sizeof(sideband_write_req));
    memset(&req, 0, sizeof(req));
    memset(&ans, 0, sizeof(ans));

    if (construct_hdrs(&req, ifindex, &remaining, &na)) {
        printf("\nFUGAZI_NL %d: construct_hdrs failed", port_id);
        return (FAILED);
    }

    /* Add actual request */
    next_len = sizeof(sideband_write_req) + NLA_HDRLEN;
    if (validate_next_buf(remaining, next_len)) {
        na->nla_type = BNXT_ATTR_REQUEST;
        na->nla_len = next_len; /* Message length */
        sideband_write_req.req_type = HWRM_PORT_SFP_SIDEBAND_CFG;
        sideband_write_req.port_id = port_id;
        sideband_write_req.cmpl_ring = 0;
        sideband_write_req.seq_id = 0;
        sideband_write_req.target_id = 0;
        sideband_write_req.resp_addr = 0;
        sideband_write_req.enables |= PORT_SFP_SIDEBAND_CFG_REQ_ENABLES_TX_DIS;
        if (enable) {
            sideband_write_req.flags |= PORT_SFP_SIDEBAND_CFG_REQ_FLAGS_TX_DIS;
        } else {
            sideband_write_req.flags &= PORT_SFP_SIDEBAND_CFG_REQ_FLAGS_TX_DIS;
        }
        memcpy((char *)get_gelmnsg_nla_data(na), &sideband_write_req,
               sizeof(sideband_write_req));
        req.n.nlmsg_len += NLA_ALIGN(na->nla_len);
    } else {
        printf("\nFUGAZI_NL %d: xcvr_detect: Failed to insert REQUEST",
                port_id);
        return (FAILED);
    }

    if (send_gnl_msg(g_mng.socket_fd, &req, &ans) < 0) {
        printf("\nFUGAZI_NL %d: xcvr_detect: send_gnl_msg returned error",
                port_id);
        return (FAILED);
    }

    /* Parse the reply message */
    na = (struct nlattr *) get_genlmsg_data(&ans);
    sideband_write_resp = (hwrm_port_sfp_sideband_cfg_output_t *)
                           get_gelmnsg_nla_data(na);

    if (sideband_write_resp->error_code) {
        printf("\nFUGAZI_NL %d: write error_code 0x%x",
                port_id, sideband_write_resp->error_code);
        return (FAILED);
    }

    if (diagflag_xram & D_DEBUG_OPTIONS) {
        printf("\n FUGAZI Port %d: tx_dis %s", port_id,
                enable ? "enable" : "disable" );
    }

    return (PASSED);
}

/******************************************************************************
 *
 * Function: curie_eth_get_ifindex
 *
 * Description: This function get the network index from /sys/class/net/%s/ifindex
 *
 * Inputs      : port - ethernet port number
 * Outputs     : ifindex
 *
 *****************************************************************************/
static int curie_eth_get_ifindex (uint16_t port)
{
    char path[DEV_IFINDEX_PATH_SIZE];
    char dev_name[DEV_IFINDEX_PATH_SIZE];
    int ifindex;
    FILE *fp;

    sprintf(dev_name, "eth%d", port);
    snprintf(path, DEV_IFINDEX_PATH_SIZE, SYS_IFINDEX_PATH, dev_name);
    fp = fopen(path, "r");
    fscanf(fp, "%d", &ifindex);
    fclose(fp);
    return ifindex;
}

/******************************************************************************
 *
 * Function: bcm57412_port_sideband_tx_dis
 *
 * Description: This function enalbe/disable the sideband tx_dis GPIO value
 *
 * Inputs      : port - port number
 *             : enable - 1: enable "tx_dis" to SFP, 0: disable "tx_dis" to SFP (enable SFP TX).
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/
int bcm57412_port_sideband_tx_dis (int port, int enable)
{
    int retval = PASSED;
    int ifindex;
    int eth_list[] = {ETH4, ETH5};

    bnxt_impl_init_netlink();

    /* To verify eth4 and eth5 */
    ifindex = curie_eth_get_ifindex(eth_list[port]);
    if (bnxt_netlink_sideband_tx_dis(eth_list[port], ifindex, enable)) {
        cterr('f',0,"eth%d: sideband read: Target not responding", eth_list[port]);
        retval = FAILED;
    }

    bnxt_impl_deinit_netlink();
    return (retval);
}

/******************************************************************************
 *
 * Function: bcm57412_sideband_tx_dis
 *
 * Description: This function enalbe/disable the sideband tx_dis GPIO value
 *
 * Inputs      : enable - 1: enable "tx_dis" to SFP, 0: disable "tx_dis" to SFP (enable SFP TX).
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/
static int bcm57412_sideband_tx_dis (int enable)
{
    int retval = PASSED;
    int port;

    for (port = 0; port < 2; port ++) {
        retval |= bcm57412_port_sideband_tx_dis(port, enable);
    }
    return retval;
}

/*
 *-----------------------------------------------------------------------------
$Log: bcm57412_test.c,v $
Revision 1.2  2021/01/11 11:03:28  xiaolaya
*** empty log message ***

Revision 1.1  2020/01/09 01:01:55  jiajliu
Merge Curie 2RU to main trunk

$Endlog$
 *-----------------------------------------------------------------------------
 */
