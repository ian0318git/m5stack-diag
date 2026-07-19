/* $Id: bcm57412_test.c,v 1.3 2020/12/29 03:09:03 leschen Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/overlord/neptune/curie_1RU/bcm57412_test.c,v $
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
#include "bcm57412_lib.h"

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

/******************************************************************************
 *  List of Menu used for Broadcom BCM57412 10G Speed
 *****************************************************************************/
static submenu_xtable_t BCM57412_10G_tests_submenu_table[] = {
   {"BCM57412 Port1 Internal Loopback Test", (type_t(*)())bcm57412_internal_loopback_test,   1,
     MF_3, (type_t(*)())0, 0, (type_t(*)())0,   0},
   {"BCM57412 Port2 Internal Loopback Test", (type_t(*)())bcm57412_internal_loopback_test,   2,
     MF_3, (type_t(*)())0, 0, (type_t(*)())0,   0},
   {"BCM57412 Port1 SFP+ External Loopback Test", (type_t(*)())bcm57412_external_loopback_test,   1,
     MF_3, (PFT)is_curie_1ru_p1c_and_older, 0, (type_t(*)())0,   0},
   {"BCM57412 Port2 SFP+ External Loopback Test", (type_t(*)())bcm57412_external_loopback_test,   2,
     MF_3, (PFT)is_curie_1ru_p1c_and_older, 0, (type_t(*)())0,   0},
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
     MF_3, (PFT)is_curie_1ru_p1c_and_older, 0, (type_t(*)())0,   0},
   {"BCM57412 Port2 SFP External Loopback Test", (type_t(*)())bcm57412_external_loopback_test,   2,
     MF_3, (PFT)is_curie_1ru_p1c_and_older, 0, (type_t(*)())0,   0},
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
    if (is_thallium()) {
        /* Thallium BCM57412 two ports are running at speed 1G */
        if (set_bcm57412_1g_speed(MB_57412_PORT1) == FAILED) {
            printf("Failed to set Thallium bcm57412 port1 at speed 1G\n");
            system(DISPLAY_PORT1_CAP);
        } 

        if (set_bcm57412_1g_speed(MB_57412_PORT2) == FAILED) {
            printf("Failed to set Thallium bcm57412 port2 at speed 1G\n");
            system(DISPLAY_PORT2_CAP);
        }
    }

    if (is_radium()) {
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
        menu(BCM57412_submenup, BCM57412_tests_secondary_items, '\0' );
    } else {
        exec_doall_menu_items(BCM57412_submenup);
    }

    return (PASSED);
}

/*******************************************************************************
 *
 * Function    : BCM57412_utility
 * Description :
 * Inputs      : menu_option - display menu instead of running all BCM57412 
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
    char *check_speed_file = "/curie-1RU-diag/bcm57412_1g_speed";
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

    system(SUPPRESS_MESG);
    if (port == BCM57412_PORT1) {
        system(BCM57412_PORT1_I2C);
    } else {
        system(BCM57412_PORT2_I2C);
    }

    fp = fopen(BCM57412_I2C_RESULT, "r");
    if (fp == NULL) {
        system(OPEN_MESG);
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
    return (retval);
}

static void dump_sfp_eeprom (void)
{
    printf("\nDisplay MB BCM57412 port 1 SFP+ EEPROM\n");
    system(BCM57412_PORT1_SFP_PLUS_EEPROM_DUMP);
    printf("\nDisplay MB BCM57412 port 2 SFP+ EEPROM\n");
    system(BCM57412_PORT2_SFP_PLUS_EEPROM_DUMP);
}

/*-------------------------------------------------
$Log: bcm57412_test.c,v $
Revision 1.3  2020/12/29 03:09:03  leschen
Remove bnxt_en operations.

Revision 1.2  2019/08/06 06:56:11  alpeng
merge curie, switzer and nightwatch to trunk

Revision 1.1.2.12  2019/03/20 03:41:29  leschen
Hide bcm57412 ext lpbk tests for P2 build and bcm82752 tests for P1C and older build

Revision 1.1.2.11  2019/03/12 07:52:26  leschen
Support BCM82752

Revision 1.1.2.10  2019/01/28 07:21:02  leschen
Enhancement 1G speed settings to fix the bcm57412 external loopback issue on Thallium.

Revision 1.1.2.9  2019/01/19 06:51:14  leschen
Add 1G speed setting and checking mechanism.

Revision 1.1.2.8  2019/01/17 08:52:50  leschen
Config speed to 1G for Thallium and remove driver when tests finish for bcm57412 sm.

Revision 1.1.2.7  2019/01/15 09:40:45  leschen
Add codes to check bcm57412 driver status.

Revision 1.1.2.6  2018/12/27 09:45:56  leschen
Based on PRRQ CSCvn30794-2 comments to modify the codes.

Revision 1.1.2.5  2018/12/26 08:36:49  leschen
Add new table to support Thallium which only run 1G speed.

Revision 1.1.2.4  2018/09/28 07:57:37  leschen
Add i2c test to verify the path between bcm57412 and sfp+

Revision 1.1.2.3  2018/09/17 06:14:53  leschen
Skip the external loopback when external loopback flag is off

Revision 1.1.2.2  2018/08/13 20:23:38  leschen
Completing BCM57412 tests and utility

Revision 1.1.2.1  2018/08/02 08:44:11  leschen
Initial check in bcm57412 files


$Endlog$
*/
