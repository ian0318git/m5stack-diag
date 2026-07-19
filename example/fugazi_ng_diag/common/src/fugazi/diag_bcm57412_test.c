/* $Id: diag_bcm57412_test.c,v 1.2 2021/06/02 08:22:34 iachang Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/fugazi/diag_bcm57412_test.c,v $
 *------------------------------------------------------------------
 * diag_bcm57412_test.c - Diags Test for BCM57412 10G NIC.
 *
 * Jan 2019, Ian Chang
 *
 * Copyright (c) 2019-2021 by Cisco Systems, Inc.
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
#include "diag_bcm57412_test.h"
#include "diag_bcm57412_utils.h"
#include "diag_bcm_lib.h"

#define MF_1    (MF_CONTINUOUS | MF_DOGRP)
#define MF_2    (MF_1 | MF_DOALL)
#define MF_3    (MF_2 | MF_SHOW_ERRCOUNT)
#define MF_4    (MF_1 | MF_SHOW_ERRCOUNT)

static int BCM57412_utility(int);
static void bcm57412_fw_mac_program(void);
static void bcm57412_cfg_mac_program(void);
static void bcm57412_cfg_no_lasi_program(void);
int bcm57412_internal_loopback_test(int);
static void bcm57412_speed_setting(void);
static void bcm57412_display_cap(void);
int bcm57412_sfp_i2c_test(int);
int bcm57412_reg_test(int);
static void dump_sfp_eeprom(void);
static void bcm57412_lcdiag(void);
int bcm57412_sideband_read(int);
int bcm57412_sideband_tx_dis(int);
static int dump_sfp_info_util(void);

/******************************************************************************
 *  List of Menu used for Broadcom BCM57412
 *****************************************************************************/
static submenu_xtable_t BCM57412_tests_submenu_table[] = {
   {"BCM57412 SFP i2c Test", (type_t(*)())bcm57412_sfp_i2c_test,   1,
     MF_3, (type_t(*)())0, 0, (type_t(*)())0,   0},
   {"BCM57412 Register Test", (type_t(*)())bcm57412_reg_test,   1,
     MF_3, (type_t(*)())0, 0, (type_t(*)())0,   0},
   {"BCM57412 Internal Loopback Test", (type_t(*)())bcm57412_internal_loopback_test,   TRUE,
     MF_3, (type_t(*)())0, 0, (type_t(*)())0,   0},
   {"BCM57412 Utility", (type_t(*)())BCM57412_utility,   FALSE,
    0, NULL, 0, (type_t(*)())BCM57412_utility,   TRUE},
};

/******************************************************************************
 *  List of Utilities used for XFI BCM57412
 *****************************************************************************/
static submenu_xtable_t BCM57412_util_items[] = {
    {"10G MAC BCM57412 Firmware Download", (type_t(*)())bcm57412_fw_mac_program, 0, 0,
     (type_t(*)())0, 0, (type_t(*)())0,   0},
    {"10G MAC BCM57412 Config Program", (type_t(*)())bcm57412_cfg_mac_program, 0, 0,
     (type_t(*)())0, 0, (type_t(*)())0,   0},
    {"10G MAC speed Config", (type_t(*)())bcm57412_speed_setting, 0, 0,
     (type_t(*)())0, 0, (type_t(*)())0, 0},
    {"Display ports capability", (type_t(*)())bcm57412_display_cap, 0, 0,
     (type_t(*)())0, 0, (type_t(*)())0, 0},
    {"BCM57412 Signal Port Internal Loopback Test", (type_t(*)())bcm57412_internal_loopback_test, FALSE, 0,
     (type_t(*)())0, 0, (type_t(*)())0, 0},
    {"Dump SFP Module EEPROM", (type_t(*)())dump_sfp_eeprom, 0, 0,
     (type_t(*)())0, 0, (type_t(*)())0, 0},
    {"BCM57412 lcdiag utility", (type_t(*)())bcm57412_lcdiag, 0, 0,
     (type_t(*)())0, 0, (type_t(*)())0, 0},
    {"BCM57412 sideband read", (type_t(*)())bcm57412_sideband_read, 0, 0,
     (type_t(*)())0, 0, (type_t(*)())0, 0},
    {"BCM57412 sideband tx_dis", (type_t(*)())bcm57412_sideband_tx_dis, 0, 0,
     (type_t(*)())0, 0, (type_t(*)())0, 0},
    {"BCM57412 MDIO BUS release", (type_t(*)())bcm57412_mdio_bus_release, 0, 0,
     (type_t(*)())0, 0, (type_t(*)())0, 0},
    {"BCM57412 MDIO BUS acquire", (type_t(*)())bcm57412_mdio_bus_acquire, 0, 0,
     (type_t(*)())0, 0, (type_t(*)())0, 0},
    {"10G MAC BCM57412 Config Program (No LASI)", (type_t(*)())bcm57412_cfg_no_lasi_program, 0, 0,
     (type_t(*)())0, 0, (type_t(*)())0,   0},
    {"Dump SFP Module Info", (type_t(*)())dump_sfp_info_util, 0, 0,
     (type_t(*)())0, 0, (type_t(*)())0, 0},
};

/******************************************************************************
 *  Macro Definitions
 *****************************************************************************/
#define BCM57412_TESTS_SUBMENU_TABLE_SIZE (sizeof(BCM57412_tests_submenu_table) / \
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
 * XFI BCM57412 Utils submenu
 *****************************************************************************/
menuinfo_t BCM57412_util_menu = {
    "10G MAC BCM57412 Utility Menu",
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

    build_primary_submenu(BCM57412_tests_submenu_table,
                          BCM57412_TESTS_SUBMENU_TABLE_SIZE,
                          "10G MAC BCM57412", &BCM57412_submenup);
    build_secondary_submenu(BCM57412_tests_submenu_table,
                            BCM57412_TESTS_SUBMENU_TABLE_SIZE,
                            BCM57412_tests_secondary_items);

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
 * Inputs      : menu_option - display menu instead of running all BCM57412 
 *               tests.
 * Outputs     : PASSED / FAILED
 *
 ******************************************************************************/
static int BCM57412_utility (int show_menu)
{
    build_primary_submenu(BCM57412_util_items, BCM57412_TESTS_UTIL_SIZE,
                          "10G MAC BCM57412 Utilities Menu", &BCM57412_util_menup);
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
    uint port = 0, speed = 0;
    char speed_get[128];
    char speed_set[128];

    port = getdec_answer("\nPlease enter port number (0~5) selection : ", 0, 0, 5);
    speed = getdec_answer("\nSpeed? (0-1G, 1-10G): ", 0, 0, 1);

    system(SUPPRESS_MESG);
    printf("Currently BCM57412 port %d(eth%d) is running at speed -\n", port, 
            BCM57412_PORT_STAR + port);
    sprintf(speed_get, "%s%d %s", BCM57412_PORT_SPEED_GET, 
            BCM57412_PORT_STAR + port, BCM57412_PORT_SPEED_GET_TAIL);
    system(speed_get);
    if (speed == BCM57412_1G) {
        sprintf(speed_set, "%s %d %s", BCM57412_PORT_SPEED, 
                BCM57412_PORT_STAR + port, BCM57412_PORT1_1G_SPEED_TAIL);
        system(speed_set);
    } else if (speed == BCM57412_10G) {
        sprintf(speed_set, "%s %d %s", BCM57412_PORT_SPEED, 
                BCM57412_PORT_STAR + port, BCM57412_PORT1_10G_SPEED_TAIL);
        system(speed_set);
    } else {
        printf("****Warning - Not support option %d\n", opt);
    }
    printf("After setting BCM57412 port %d(eth%d) is running at speed -\n", 
            port, BCM57412_PORT_STAR + port);
    sprintf(speed_get, "%s%d %s", BCM57412_PORT_SPEED_GET, 
            BCM57412_PORT_STAR + port, BCM57412_PORT_SPEED_GET_TAIL);
    system(speed_get);

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
    int ix;
    char display_cap[128];
    for (ix = 0; ix < BCM57412_PORT; ix++) {
        sprintf(display_cap, "%s%d", DISPLAY_PORT_CAP, BCM57412_PORT_STAR + ix);
        system(display_cap);
    }
}

/******************************************************************************
 *
 * Function: bcm57412_fw_mac_program
 *
 * Description: This function guides BCM57412 firmware programming.
 *              Execute the script : /diag_utils/fugazi/scripts/function.sh
 * Inputs      : NONE
 *
 * Outputs     : NONE
 *
 ********************************************************************************/
static void bcm57412_fw_mac_program (void)
{
    printf("This utility uses Broadcom CDiag tool by executing script load_fw.sh\n");
    system(FUGAZI_UPDATE_BCM57412_FW);
}

/******************************************************************************
 *
 * Function: bcm57412_cfg_mac_program
 *
 * Description: This function guides BCM57412 config programming.
 *              Execute the script : /diag_utils/fugazi/scripts/function.sh
 * Inputs      : NONE
 *
 * Outputs     : NONE
 *
 ********************************************************************************/
static void bcm57412_cfg_mac_program (void)
{
    printf("This utility uses Broadcom bnxtmt tool by executing script load_fw.sh\n");
    system(FUGAZI_UPDATE_BCM57412_CFG);
    system(FUGAZI_UPDATE_BCM57412_PHYADD_CFG);
}

/******************************************************************************
 *
 * Function: bcm57412_cfg_no_lasi_program
 *
 * Description: This function guides BCM57412 config programming(No LASI).
 *              Execute the script : /diag_utils/fugazi/scripts/function.sh
 * Inputs      : NONE
 *
 * Outputs     : NONE
 *
 ********************************************************************************/
static void bcm57412_cfg_no_lasi_program (void)
{
    printf("This utility uses Broadcom bnxtmt tool by executing script load_fw.sh\n");
    system(FUGAZI_UPDATE_BCM57412_NO_LASI_CFG);
}
/******************************************************************************
 *
 * Function: BCM57412_internal_loopback_test
 *
 * Description: This function perform the internal loopback test
 *              from CPU to BCM57412 MAC and PHY.
 *
 * Inputs      : mode - dummy 
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/
int bcm57412_internal_loopback_test (int mode)
{
    char *tname = "BCM57412 internal loopback";
    char buf[128] = "NULL";
    char loopback_cmd[128];
    FILE *fp;
    int retval = PASSED;
    int ix, star_port, end_port;

    system(SUPPRESS_MESG);
    testname("%s", tname);

    if (mode == TRUE) {
        star_port = 0;
        end_port = BCM57412_PORT;
    } else {
        star_port = getdec_answer("Test BCM57412 port (0~11): ", 0, 0, 11);
        end_port = star_port + 1;
    }
    for (ix = star_port; ix < end_port; ix++) {
        prpass(testpass, "BCM57412 port %d ", ix);
        sprintf(loopback_cmd, "%s %d %s", BCM57412_PORT_LPBK, 
                BCM57412_PORT_STAR + ix, BCM57412_PORT_INT_LPBK_TAIL);
        system(loopback_cmd);
        msleep(200);

        fp = fopen(BCM57412_LPBK_RESULT, "r");
        if (fp == NULL) {
            system(OPEN_MESG);
            return (FAILED);
        }
    
        while (!feof(fp)) {
            fgets(buf, sizeof(buf), fp);
            if (strstr(buf, BCM57412_FAIL) != NULL) {
                cterr('f',0, "port %d failed, ", ix);
                retval = FAILED;
                break;
            } else {
                prpass(testpass, "port %d passed, ", ix);
                break;
            }
        }
    
        fclose(fp);
    }
    system(SUPPRESS_MESG);
    return (retval);
}
/******************************************************************************
 *
 * Function: BCM57412_sfp_i2c_test
 *
 * Description: This function checks sfp+ cookie byte 0 and byte 1 to make  
 *              sure the i2c bus between BCM57412 and SFP module is good.
 *
 * Inputs      : mode - dummy
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/
int bcm57412_sfp_i2c_test (int mode)
{
    /*------------ netlink -------------*/
    char sfp_buf[BNXT_BUF_MAX] = "NULL";
    int retval = PASSED;
    int ix = 0, jx = 0;
    int sfp_id;
    int addr, page_num, offset, len, ifindex;
    char *tname = "BCM57412 sfp i2c";
    testname("%s", tname);
    /* HW / MFG decide the SFP and loopback cable look as one module, Will not be used independently */
    if ((NVRAM)->diagflag & D_EXT_LOOPBACK) {
        prt("External loopback flag is off, skip the test\n");
        return (PASSED);
    }
    bnxt_impl_init_netlink();
    addr = I2C_DEV_ADDR_A0;
    offset = 0x0;
    page_num = 0x0;
    len = FIXED_ID_LEN;
    for (ix = 0; ix < BCM57412_SFP_PORT; ix++) {
        ifindex = fugazi_eth_get_ifindex(ix);
        if ((NVRAM)->diagflag & D_VERBOSE) {
            printf("\nifindex = %x, \n", ifindex);
        }
        if (bnxt_sfp_detect(ix, ifindex) == FAILED) {
            printf("\nPort %d: SFP is not present", ix);
        }
        if (bnxt_netlink_i2c_read(addr, ifindex, page_num,
                                  ix, offset, len, (uint8_t *)sfp_buf)) {
            cterr('f',0,"Port %d: eeprom read: Target not responding", ix);
            retval = FAILED;
            continue;
        }
        if ((NVRAM)->diagflag & D_VERBOSE) {
            for (jx = 0; jx < len; jx ++) {
                printf("sfp_buf = %x, SFP_PLUS_FIXED_ID = %x\n", sfp_buf[jx], 
                        SFP_PLUS_FIXED_ID);
            }
        }
        sfp_id = ((sfp_buf[0] << 8) & 0xff00) | (sfp_buf[1] & 0xff);
        if (sfp_id != SFP_PLUS_FIXED_ID) {
            cterr('f',0, "port %d failed, Expect SFP ID %x, Read %x, "
                  "please check whether has sfp module", ix, 
                   SFP_PLUS_FIXED_ID, sfp_id);
            retval = FAILED;
        } else {
            prpass(testpass, "port %d passed, ", ix);
            printf("\n");
        }
    }
    bnxt_impl_deinit_netlink();
    return (retval);
#ifdef CDIAG
    char *tname = "BCM57412 sfp i2c\n";
    char buf[128] = "NULL";
    char bcm57412_i2c_cmd[128];
    FILE *fp;
    int retval = PASSED;
    int ix;
    
    testname("%s", tname);

    system(REMOVE_BCM57412_DRIVER);
    chdir(ENTER_BCM57412_SCRIPT_DIR);
    system(SUPPRESS_MESG);
    for (ix = 0; ix < BCM57412_SFP_PORT; ix++) {
        prpass(testpass, "BCM57412 port %d ", ix);
        sprintf(bcm57412_i2c_cmd, "%s %d %s", BCM57412_I2C,
                BCM57412_PORT1 + ix, BCM57412_I2C_TAIL);
        system(bcm57412_i2c_cmd);

        fp = fopen(BCM57412_I2C_RESULT, "r");
        if (fp == NULL) {
            system(OPEN_MESG);
            system(MODPROBE_BCM57412_DRIVER);
            return (FAILED);
        }
    
        while (!feof(fp)) {
            fgets(buf, sizeof(buf), fp);
            if (strstr(buf, SFP_PLUS_FIXED_ID) == NULL) {
                cterr('f',0, "port %d failed, Expect SFP ID %x, Read %s, "
                      "please check whether has sfp module", ix, 
                       SFP_PLUS_FIXED_ID, buf);
                retval = FAILED;
                break;
            } else {
                prpass(testpass, "port %d passed, ", ix);
                break;
            }
        }
        system(REMOVE_BCM57412_I2C_RESULT);

        fclose(fp);
    }
    system(SUPPRESS_MESG);
    system(MODPROBE_BCM57412_DRIVER);
    system(REMOVE_BCM57412_DRIVER);   /* Reload driver to fix BCM57412 load issue*/
    system(MODPROBE_BCM57412_DRIVER);
    return (retval);
#endif
}

/******************************************************************************
 *
 * Function: bcm57412_reg_test
 *
 * Description: This Tests verify that registers access through 
 *              the PCI/PCI-E interface implement the expected read-only or 
 *              read/write attributes by attempting to modify those registers
 *              The test used Boradcom netlink driver 
 *
 * Inputs      : mode - dummy
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/
int bcm57412_reg_test (int mode)
{
    /*------------ netlink -------------*/
    char *tname = "BCM57412 Register";
    int retval = PASSED;
    int ix, ifindex;
    
    testname("%s", tname);

    bnxt_impl_init_netlink();
    for (ix = 0; ix < BCM57412_PORT; ix++) {
        ifindex = fugazi_eth_get_ifindex(ix);
        prpass(testpass, "BCM57412 port %d ", ix);
        printf("\n");
        if (bnxt_netlink_reg_test(ix, ifindex)) {
            cterr('f',0, "port %d BDERBD block register test failed,", ix);
            retval = FAILED;
            continue;
        }
    }
    bnxt_impl_deinit_netlink();
    return (retval);
#ifdef CDIAG
    char *tname = "BCM57412 Register\n";
    char buf[128] = "NULL";
    char bcm57412_reg_cmd[128];
    FILE *fp;
    int retval = PASSED;
    int ix, cdiag_count = 0;
    
    testname("%s", tname);

    system(REMOVE_BCM57412_DRIVER);
    chdir(ENTER_BCM57412_SCRIPT_DIR);
    system(SUPPRESS_MESG);
    for (ix = 0; ix < BCM57412_PORT; ix++) {
        prpass(testpass, "BCM57412 port %d ", ix);
        sprintf(bcm57412_reg_cmd, "%s %d %s", BCM57412_REG,
                BCM57412_PORT1 + ix, BCM57412_REG_TAIL);
        system(bcm57412_reg_cmd);

        fp = fopen(BCM57412_REG_RESULT, "r");
        if (fp == NULL) {
            system(OPEN_MESG);
            system(MODPROBE_BCM57412_DRIVER);
            cterr('f',0, "open %s failed,", BCM57412_REG_RESULT);
            return (FAILED);
        }
  
        while (!feof(fp)) {
            fgets(buf, sizeof(buf), fp);
            if (strstr(buf, BCM57412_REG_NULL) != NULL) {
                /* Result file NULL, that cdiag didn't execute */
                ix -= 1;
                cdiag_count++;
                break;
            } else if (strstr(buf, BCM57412_REG_PASS) == NULL) {
                cterr('f',0, "port %d BDERBD block register test failed,", ix);
                retval = FAILED;
                break;
            } else {
                prpass(testpass, "port %d passed,", ix);
                break;
            }
        } 
        if (cdiag_count > CDIAG_RETRY) {
            cterr('f',0, "port %d cdiag execute failed,", ix);
            return (FAILED);
        }
       
        system(REMOVE_BCM57412_REG_RESULT);

        fclose(fp);
    }
    system(SUPPRESS_MESG);
    system(MODPROBE_BCM57412_DRIVER);
    system(REMOVE_BCM57412_DRIVER);   /* Reload driver to fix BCM57412 load issue*/
    system(MODPROBE_BCM57412_DRIVER);
    return (retval);
#endif
}
/******************************************************************************
 *
 * Function: dump_sfp_eeprom
 *
 * Description: This utility dump the SFP cookie via Boradcom tool 
 *
 * Inputs      : none
 * Outputs     : none
 *
 *****************************************************************************/
static void dump_sfp_eeprom (void)
{
    int port;
    char sfp_dump_cmd[128];

    port = getdec_answer("\nPlease enter port number (1~12), (0 for all) : ", 0, 0, 12);

    sprintf(sfp_dump_cmd, "%s %d", BCM57412_SFP_PLUS_EEPROM_DUMP, port);
    system(sfp_dump_cmd);
}

/******************************************************************************
 *
 * Function: bcm57412_lcdiag
 *
 * Description: This utility execute the Broadcom lcdiag tool 
 *
 * Inputs      : none
 * Outputs     : none
 *
 *****************************************************************************/
static void bcm57412_lcdiag (void)
{
    printf("To back to Menu, please type exit from Shell.\n\n");
    system(BCM57412_LCDIAG);
}
/******************************************************************************
 *
 * Function: bcm57412_sideband_read
 *
 * Description: This utility display the sideband GPIO value 
 *
 * Inputs      : none
 * Outputs     : none
 *
 *****************************************************************************/
int bcm57412_sideband_read (int mode)
{
    int retval = PASSED;
    int ix = 0;
    int addr, page_num, offset, len, ifindex, start_port, end_port;
    char *tname = "BCM57412 sideband ";
    testname("%s", tname);
    int port;

    port = gethex_answer("\nPlease enter port number (0~11), (0xff for all) : ", 0xff, 0, 0xff);

    bnxt_impl_init_netlink();
    addr = I2C_DEV_ADDR_A0;
    offset = 0x0;
    page_num = 0x0;
    len = FIXED_ID_LEN;
    if (port == 0xff) {
        start_port = 0;
        end_port = BCM57412_SFP_PORT;
    } else {
        start_port = port ;
        end_port = port + 1;
    }
    for (ix = start_port; ix < end_port; ix++) {
        ifindex = fugazi_eth_get_ifindex(ix);
        if (diagflag_xram & D_DEBUG_OPTIONS) {
            printf("\nifindex = %x, \n", ifindex);
        }
        if (bnxt_netlink_sideband_read(ix, ifindex)) {
            cterr('f',0,"Port %d: sideband read: Target not responding", ix);
            retval = FAILED;
            continue;
        }
    }
    bnxt_impl_deinit_netlink();
    return (retval);
}
/******************************************************************************
 *
 * Function: bcm57412_sideband_tx_dis_setup
 *
 * Description: This function enalbe/disable the sideband tx_dis GPIO value 
 *
 * Inputs      : port - port number
 *             : enable - 1: enable "tx_dis" to SFP, 0: disable "tx_dis" to SFP (enable SFP TX).
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/
int bcm57412_sideband_tx_dis_setup (int port, int enable)
{
    int retval = PASSED;
    int ix = 0;
    int addr, page_num, offset, len, ifindex, start_port, end_port;

    bnxt_impl_init_netlink();
    addr = I2C_DEV_ADDR_A0;
    offset = 0x0;
    page_num = 0x0;
    len = FIXED_ID_LEN;
    if (port == 0xff) {
        start_port = 0;
        end_port = BCM57412_SFP_PORT;
    } else {
        start_port = port ;
        end_port = port + 1;
    }
    for (ix = start_port; ix < end_port; ix++) {
        ifindex = fugazi_eth_get_ifindex(ix);
        if (diagflag_xram & D_DEBUG_OPTIONS) {
            printf("\nifindex = %x, \n", ifindex);
        }
        if (bnxt_netlink_sideband_tx_dis(ix, ifindex, enable)) {
            cterr('f',0,"Port %d: sideband read: Target not responding", ix);
            retval = FAILED;
            continue;
        }
    }
    bnxt_impl_deinit_netlink();
    return (retval);
}
/******************************************************************************
 *
 * Function: bcm57412_sideband_tx_dis
 *
 * Description: This utility enalbe/disable the sideband tx_dis GPIO value 
 *
 * Inputs      : none
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/
int bcm57412_sideband_tx_dis (int mode)
{
    int retval = PASSED;
    char *tname = "BCM57412 sideband tx_dis ";
    testname("%s", tname);
    uint enable = 0;
    int port;

    port = gethex_answer("\nPlease enter port number (0~11), (0xff for all) : ", 0xff, 0, 0xff);
    enable = getdec_answer("\nPlease enter tx_dis selection (0)Disable (1)Enable" 
                           ": ", 0, 0, 1);
    
    retval = bcm57412_sideband_tx_dis_setup(port, enable);
    return (retval);
}

/******************************************************************************
 *
 * Function: dump_sfp_info_util
 *
 * Description: This utility dump the SFP cookie via netlink
 *
 * Inputs      : none
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/
static int dump_sfp_info_util (void)
{
    int f_sfp_info, port=0xff;

    f_sfp_info = getdec_answer("\nDisplay SFP cookie info(0: SFP cookie,  1: SFP info) : ", 1, 0, 1);

    if (f_sfp_info) {
        return (sfp_display_info());
    } else {
        port = gethex_answer("\nPlease enter port number (0~11), (0xff for all) : ", 0xff, 0, 0xff);
        return (sfp_display_cookie(port));
    }
}

/*-------------------------------------------------
 * $Log: diag_bcm57412_test.c,v $
 * Revision 1.2  2021/06/02 08:22:34  iachang
 * CSCvo59196-33 : Merge Fugazi from ASR1K-main-branch to ISR main trunk
 *
 * Revision 1.1.8.5  2021/03/19 18:34:40  pdoong
 * Add Dump SFP Module Info utility to display SFP Vendor PN, S/N, Vendor name, PID
 *
 * Revision 1.1.8.4  2020/11/06 03:27:01  iachang
 * CSCvo59196-21:Support BCM57412 LASI/No-LASI config program.
 *
 * Revision 1.1.8.3  2020/09/04 08:19:51  iachang
 * BCM57412 Config Program add phy_address cfg update.
 *
 * Revision 1.1.8.2  2020/08/26 02:37:47  iachang
 * Merge Fugazi code into main trunk
 *
 * Revision 1.1.6.31  2020/08/20 06:36:20  iachang
 * PRRQ CSCvo59196-5 : BCM57412 MAC code review
 *
 * Revision 1.1.6.30  2020/08/05 09:02:41  iachang
 * Update Copyright to 2020
 *
 * Revision 1.1.6.29  2020/07/03 07:34:20  iachang
 * Support bcm57412_mdio_bus_release() and bcm57412_mdio_bus_acquire()
 * Move those funcitons from diag_bcm57412_test.c to diag_bcm57412_utils.c
 *
 * Revision 1.1.6.28  2020/06/12 02:35:06  iachang
 * Support bcm57412_mdio_bus_acquire and bcm57412_mdio_bus_release for all GE port
 *
 * Revision 1.1.6.27  2020/04/22 07:11:04  iachang
 * Add BCM57412 mdio_bus_acquire function
 *
 * Revision 1.1.6.26  2020/03/06 05:53:15  iachang
 * Implement BCM57412 sideband tx_dis setup function.
 *
 * Revision 1.1.6.25  2020/03/03 08:38:09  iachang
 * BCM57412 I2C test : HW / MFG decide the SFP and loopback cable look as one module, Will not be used independently
 *
 * Revision 1.1.6.24  2020/02/25 07:59:48  iachang
 * Add debug mode in BCM57412 I2C test to skip SFP not present.(HW request)
 *
 * Revision 1.1.6.23  2020/01/17 06:30:26  iachang
 * Skip BCM82757 initial with SyncE and BCM57412 submenu and add in SyncE Recovered Clock Test.
 *
 * Revision 1.1.6.22  2020/01/15 07:30:08  iachang
 * Skip BCM82757 fw download with Diag initial. It can save Diag menu boot up time, and help debug.
 *
 * Revision 1.1.6.21  2020/01/09 08:16:11  iachang
 * Add bcm57412_mdio_bus_release function, and modify BCM57412 sideband tx_dis utility.
 *
 * Revision 1.1.6.20  2019/11/13 03:29:44  iachang
 * Fixed BCM57412 SFP i2c Test Segmentation fault issue.
 *
 * Revision 1.1.6.19  2019/11/06 02:23:53  iachang
 * Modify BCM57412 utility port mapping.
 *
 * Revision 1.1.6.18  2019/10/24 06:26:03  iachang
 * Check SFP present before "BCM57412 SFP i2c Test"
 *
 * Revision 1.1.6.17  2019/10/22 02:05:53  iachang
 * CS7829107: One BCM82757 chip randomly init fail after firmware download completely
 *
 * Revision 1.1.6.16  2019/10/16 08:53:06  iachang
 * Port BCM57412 Register test via Netlink to replace Broadcom Cdiag tool
 *
 * Revision 1.1.6.15  2019/09/27 08:03:58  iachang
 * Changed "BCM57412 SFP i2c Test" from Broadcom CDiag to Netlink.
 *
 * Revision 1.1.6.14  2019/08/22 01:52:00  iachang
 * Support BCM57412 Signal Port Internal Loopback Test utility
 *
 * Revision 1.1.6.13  2019/08/09 02:49:18  iachang
 * Modify BCM57412 SFP-i2c and Register testname
 *
 * Revision 1.1.6.12  2019/06/17 09:34:19  iachang
 * Disable the kernel message when test completed
 *
 * Revision 1.1.6.11  2019/06/12 08:00:35  iachang
 * Merge BCM82757 signal port loopback test item.
 * Add more information with error message.
 *
 * Revision 1.1.6.10  2019/06/06 02:58:45  iachang
 * Fixed Module bnxt_en is not currently loaded issue
 *
 * Revision 1.1.6.9  2019/05/28 06:04:44  iachang
 * Separated BCM57412 FW/Cfg program at two items.
 *
 * Revision 1.1.6.8  2019/05/07 08:01:52  iachang
 * Fixed BCM57412 register test intermittent issue.
 *
 * Revision 1.1.6.7  2019/04/25 03:11:41  iachang
 * Add Broadcom lcdiag tool in utility
 *
 * Revision 1.1.6.6  2019/04/23 07:36:29  iachang
 * Correct BCM57412 menu
 *
 * Revision 1.1.6.5  2019/04/11 19:31:42  iachang
 * Support BCM57412 register test
 *
 * Revision 1.1.6.4  2019/03/29 18:43:26  iachang
 * Support BCM57412 firmware upgrade utility.
 *
 * Revision 1.1.6.3  2019/03/14 18:42:04  iachang
 * Bring up BCM57412 on Fugazi
 *
 * Revision 1.1.6.2  2019/03/14 03:48:35  letsai
 * Initial check in.
 *
 * Revision 1.1.2.3  2019/01/14 09:30:56  iachang
 * Add BCM57412 SFP I2C interface test
 *
 * Revision 1.1.2.2  2019/01/10 09:07:22  iachang
 * Modify BCM57412 test for all ports
 *
 * $Endlog$
 * */
