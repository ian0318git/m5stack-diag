/* $Id: diag_tlk10232_test.c,v 1.4 2015/03/31 07:29:49 leschen Exp $ 
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/woodlawn_linux/diag_tlk10232_test.c,v $
 *-----------------------------------------------------------------------------
 * diag_tlk10232_test.c - Menu for Woodlawn TLk10232
 *
 * January 2013, Leslie Chen
 * Copyright (c) 2015 by Cisco Systems, Inc.
 * All rights reserved.
 *-----------------------------------------------------------------------------
 */
#include "common.h"
#include "types.h"
#include "menu.h"
#include <stdio.h>
#include "diag_common_lib.h"
#include "platform_xaui.h"
#include "platform_smi.h"
#include "platform_smi_lib.h"
#include "common_utils.h"
#include "platform_eth.h"
#include "diag_tlk10232_lib.h"
#include "platform_xaui.h"
#include "diag_fpga_lib.h"

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <linux/types.h>
#include <linux/socket.h>
#include <linux/ioctl.h>
#include <linux/if_ether.h>
#include <linux/ip.h>
#include <linux/udp.h>
#include <net/if.h>
#include <netinet/in.h>
#include <linux/sockios.h>
#include <linux/mii.h>

#include "defs.h"
#include "types.h"
#include "common.h"
#include "monitor.h"
#include "cross_platform.h"
#include "menu.h"
#include "error.h"
#include "queryflags.h"
#include "router_if.h"
#include "sgmii_defs.h"
#include "cvmx.h"
#include "ethernet.h"
#include "linux_eth.h"
#include "platform_eth.h"
#include "cvmx-mdio.h"

/*****************************************************************************
 *  Functions Declaration
 *****************************************************************************/
int tlk_10232_test(int);
int tlk_10232_utility(int);
int tlk10232_do_all_wrapper(void);
static int tlk_10232_register_test(void);
static int tlk_10232_internal_loopback_test(void);
int dump_tlk_10232_registers(void);
int alter_tlk_10232_register(void);

#define MF_1    (MF_CONTINUOUS | MF_DOGRP)
#define MF_2    (MF_1 | MF_DOALL)
#define MF_3    (MF_2 | MF_SHOW_ERRCOUNT)
#define MF_4    (MF_1 | MF_SHOW_ERRCOUNT)

#define MARVELL_PHY_RONLY    (READ_ONLY | REG_ACCESS)
#define MARVEL_PHY_RW        (READ_WRITE | REG_ACCESS)

/*****************************************************************************
 *  For Register Test Use
 *****************************************************************************/
ten_g_phy_t tlk_10232_dev30_smi2 = {TLK_10232_REG_DEVICE_30,
                                    REG_TEST_TLK_10232_SMI2_ADDR};

/* reg_info_t extension for "device 30" access */
static reg_info_t_ext tlk_10232_ext30 = {TLK_10232_REG_LEN,
                                         read_tlk_10232_reg,
                                         write_tlk_10232_reg,
                                         &tlk_10232_dev30_smi2};

/* reg_table for "device 30" */
static reg_info_t tlk_10232_dev30[] = {   /* Device 30*/
    {"EXT_ADDRESS_DATA", 0x800a, MARVEL_PHY_RW,
     {(unsigned long)&tlk_10232_ext30}, 0xffff, 0xbc3c},
    {"end", 0x00, 0, {0}, 0, 0},
};

/******************************************************************************
 *  List of Menu used for TLK 10232
 *****************************************************************************/
static submenu_xtable_t tlk_10232_tests_submenu_table[] = {
   {"TLK 10232 Utility", (type_t(*)())tlk_10232_utility,   FALSE,
       0, NULL, 0, (type_t(*)())tlk_10232_utility,   TRUE},
   {"TLK 10232 Register Test", (type_t(*)())tlk_10232_register_test,   0,
       MF_3, (type_t(*)())0, 0, (type_t(*)())0,   0},
   {"TLK 10232 Internal Loopback Test", (type_t(*)())tlk_10232_internal_loopback_test,   0,
       MF_3, (type_t(*)())0, 0, (type_t(*)())0,   0},
};

/******************************************************************************
 *  List of Utilities used for TLK 10232
 *****************************************************************************/
static submenu_xtable_t tlk_10232_util_items[] = {
    {"Switch TLK 10232 Mode", (type_t(*)())tlk10232_mode_select, 0, 0, (type_t(*)())0,
     0, (type_t(*)())0, 0},
    {"TLK 10232 global reset", (type_t(*)())tlk10232_global_reset, 0, 0, (type_t(*)())0,
     0, (type_t(*)())0, 0},
    {"TLK 10232 path reset", (type_t(*)())tlk10232_path_reset, 0, 0, (type_t(*)())0,
     0, (type_t(*)())0, 0},
    {"TLK 10232 configuration", (type_t(*)())tlk10232_xaui_to_xaui_configuration, 0, 0, (type_t(*)())0,
     0, (type_t(*)())0, 0},
    {"Dump TLK 10232 Register", (type_t(*)())dump_tlk_10232_registers, 0, 0, (type_t(*)())0,
     0, (type_t(*)())0, 0},
    {"Alter TLK 10232 Register", (type_t(*)())alter_tlk_10232_register, 0, 0,
     (type_t(*)())0, 0, (type_t(*)())0, 0},
};

/******************************************************************************
 *  Macro Definitions
 *****************************************************************************/
#define TLK_10232_TESTS_SUBMENU_TABLE_SIZE (sizeof(tlk_10232_tests_submenu_table) / \
                                                sizeof(submenu_xtable_t))
                                                
#define TLK_10232_TESTS_UTIL_SIZE (sizeof(tlk_10232_util_items) / \
                                     sizeof(submenu_xtable_t))
                                     
/******************************************************************************
 *  Global Variable
 *****************************************************************************/
/******************************************************************************
 * Primary & secondary submenu items (filled in from xtable)
 *****************************************************************************/
static mitem_t tlk_10232_tests_primary_items[TLK_10232_TESTS_SUBMENU_TABLE_SIZE +
                                                 MAX_BASE_ITEMS];
static mitem_t tlk_10232_tests_secondary_items[TLK_10232_TESTS_SUBMENU_TABLE_SIZE +
                                                   MAX_BASE_ITEMS];

/******************************************************************************
 * Primary & secondary utilities menu items (filled in from xtable)
 *****************************************************************************/
static mitem_t tlk_10232_tests_primary_util_items[TLK_10232_TESTS_UTIL_SIZE +
                                                      MAX_BASE_ITEMS];
static mitem_t tlk_10232_tests_secondary_util_items[TLK_10232_TESTS_UTIL_SIZE +
                                                        MAX_BASE_ITEMS];

/******************************************************************************
 * TLK 10232 Utils submenu
 *****************************************************************************/
menuinfo_t tlk_10232_util_menu = {
    "TLK 10232 Utility Menu",
    0,
    (PFT)show_endnote,
    0,
    0,
    tlk_10232_tests_primary_util_items,
};
menuinfo_t *tlk_10232_util_menup = &tlk_10232_util_menu;

menuinfo_t tlk_10232_subtest_menu = {
    "%s Subtest Menu",
    0,                                /* mtparam added by init_empty_menu */
    (PFT)show_endnote,                /* notes missing WICs in combos */
    0,                                /* use generic prompt */
    0,                                /* size (bumped by add_menu_item() */
    tlk_10232_tests_primary_items,
};
menuinfo_t *tlk_10232_submenup = &tlk_10232_subtest_menu;

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
int tlk_10232_test (int show_menu)
{
    build_primary_submenu(tlk_10232_tests_submenu_table,
                            TLK_10232_TESTS_SUBMENU_TABLE_SIZE,
                            "TLK 10232", &tlk_10232_submenup);
    build_secondary_submenu(tlk_10232_tests_submenu_table,
                            TLK_10232_TESTS_SUBMENU_TABLE_SIZE,
                            tlk_10232_tests_secondary_items);

    if (show_menu) {
        menu(tlk_10232_submenup, tlk_10232_tests_secondary_items, '\0' );
    } else {
        menu_exec_doall_diags(tlk_10232_submenup);
    }
    return (PASSED);
}


/*******************************************************************************
 *
 * Function    : tlk10232_do_all_wrapper
 * Description : Wrapper for TLK10232 do all test items
 * Inputs      : none
 * Outputs     : PASSED / FAILED
 *
 *******************************************************************************
 */
int tlk10232_do_all_wrapper (void)
{
    int rc = PASSED;

    if (tlk_10232_register_test() == FAILED) {
        rc = FAILED;
    }

    if (tlk_10232_internal_loopback_test() == FAILED) {
        rc = FAILED;
    }

    return (rc);
}

/*******************************************************************************
 *
 * Function    : tlk_10232_utility
 * Description :
 * Inputs      : menu_option - display menu instead of running all TLK 10232
 *               tests.
 * Outputs     : PASSED / FAILED
 *
 ******************************************************************************/
int tlk_10232_utility (int show_menu)
{
    build_primary_submenu(tlk_10232_util_items, TLK_10232_TESTS_UTIL_SIZE,
                          "TLK 10232 Utilities Menu", &tlk_10232_util_menup);
    build_secondary_submenu(tlk_10232_util_items, TLK_10232_TESTS_UTIL_SIZE,
                            tlk_10232_tests_secondary_util_items);

    if (show_menu) {
       menu(tlk_10232_util_menup, tlk_10232_tests_secondary_util_items, '\0' );
    } else {
       menu_exec_doall_diags(tlk_10232_util_menup);
       prcomplete(testpass, errcount, (char *)0);
    }

    return (PASSED);
}

/******************************************************************************
 *
 * Function: tlk_10232_register_test
 *
 * Description: This function performs the TLK 10232 register test.
 *
 * Inputs      : None
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/
static int tlk_10232_register_test (void)
{
    testname("TLK 10232 Register");

    if (register_tests(0, tlk_10232_dev30) == FAILED) {
        return (FAILED);
    }
    
    prpass(testpass, "TLK 10232 Register Test Success");
    return (PASSED);
}

/******************************************************************************
 *
 * Function: tlk_10232_internal_loopback_test
 *
 * Description: This function perform the internal loopback test
 *                   from Cavium to tlk_10232.
 *
 * Inputs      : None
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/
static int tlk_10232_internal_loopback_test (void)
{
    testname("TLK 10232 Internal Looopback");
    
    /* Configure TLK10232 to operate in XAUI mode.
        To loop the XAUI data coming from the Cavium processor back on itself, 
        change bits 15:14 of register 0x1E.001A (DSR_DATA_SRC_SEL[1:0]) to 00 
        to select the same channel's LS input as the LS output data source. */
    if (config_tlk_10232_mode(XAUIB_TO_XAUIB) == FAILED) {
        cterr('f', 0, "Config TLK10232 into XAUIB <->  XAUIB mode failed");
        fflush(stdout);
        return (FAILED);
    }

    /* Perform TLK10232 internal loopback test, use the same XAUI port as backplane use */
    if (xaui_lpbk_test(LOOP_XAUI_BP) == FAILED) {
        cterr('f', 0, "TLK10232 internal loopback test failed");

        /* Recover to initial TLK10232 configuration */
        run_tlk10232_script();

        return (FAILED);
    }

    /* Recover to initial TLK10232 configuration */
    run_tlk10232_script();
            
    return (PASSED);
}

/******************************************************************************
 *
 * Function: dump_tlk_10232_registers
 *
 * Description: This function dumps the TLK 10232 registers
 *
 * Inputs      : None
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/
int dump_tlk_10232_registers (void)
{
    printf("Dump TLK 10232 Register.\n");
    fflush(stdout);
    int bus_id, phy_id, dev_id, reg_addr, mii_value, channel;
    
    bus_id = TLK_10232_SMI2_ADDR;
    channel = getdec_answer("Select channel(0 - channel A, 1 - channel B) : ", 0, 0, 1);
    dev_id = gethex_answer("Select device address : ", 0, 0, 0xFF);
    reg_addr = gethex_answer("Select register address : ", 0, 0, 0xFFFF);

    if (channel == 0) {
        phy_id = TLK_10232_PHY_ADDR_CHANNEL_A;
    } else {
        phy_id = TLK_10232_PHY_ADDR_CHANNEL_B;
    }
    
    mii_value = cvmx_mdio_45_read(bus_id, phy_id, dev_id, reg_addr);

    if (mii_value < 0) {
        printf("Read error from device %u(0x%x)", dev_id, phy_id);
        return (FAILED);
    } else {
        printf("Phy %#.2x reg %#.8x, data %#.8x\n", phy_id, reg_addr, mii_value);
        return (PASSED);
    }
}

/******************************************************************************
 *
 * Function: alter_tlk_10232_register
 *
 * Description: This function alters the TLK 10232 registers
 *
 * Inputs      : None
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/
int alter_tlk_10232_register (void)
{
    printf("Alter TLK 10232 Register.\n");
    fflush(stdout);
    int bus_id, phy_id, dev_id, reg_addr, mii_value, reg_val, channel;

    bus_id = TLK_10232_SMI2_ADDR;
    channel = getdec_answer("Select channel(0 - channel A, 1 - channel B) : ", 0, 0, 1);
    dev_id = gethex_answer("Select device address : ", 0, 0, 0xFF);
    reg_addr = gethex_answer("Select register address : ", 0, 0, 0xFFFF);

    if (channel == 0) {
        phy_id = TLK_10232_PHY_ADDR_CHANNEL_A;
    } else {
        phy_id = TLK_10232_PHY_ADDR_CHANNEL_B;
    }
    
    /* Get the current register vlaue */
    mii_value = cvmx_mdio_45_read(bus_id, phy_id, dev_id, reg_addr);
    if (mii_value < 0) {
        printf("Read error from device %u(0x%x)", dev_id, phy_id);
        return (FAILED);
    } else {
        printf("Original Value = %#.8x\n", mii_value);
    }
    
    /* Alter register with new value */
    reg_val = gethex_answer("Enter the new data (hex): ", 0, 0, 0xFFFF);

    /* Alter the current register with new vlaue */
    mii_value = cvmx_mdio_45_write(bus_id, phy_id, dev_id, reg_addr, reg_val);
    if (mii_value < 0) {
        printf("Write error from device %u(0x%x)", dev_id, phy_id);
        return (FAILED);
    } 

    /* Get new current register vlaue */
    mii_value = cvmx_mdio_45_read(bus_id, phy_id, dev_id, reg_addr);
    if (mii_value < 0) {
        printf("Read error from device %u(0x%x)", dev_id, phy_id);
        return (FAILED);
    } else {
        printf("New Value = %#.8x\n", mii_value);
        return (PASSED);
    }
}

/*-------------------------------------------------
 * $Log: diag_tlk10232_test.c,v $
 * Revision 1.4  2015/03/31 07:29:49  leschen
 * Change register which will not impact KR status for tlk10232 register testing.
 *
 * Revision 1.3  2014/11/12 06:29:14  leschen
 * Support Greyhound tlk10232 10gkr
 *
 * Revision 1.2  2013/10/08 08:48:29  tirawan
 * Woodlawn collapsed to main trunk
 *
 * Revision 1.1.4.2  2013/08/20 10:58:54  tirawan
 * Branch into woodlawn-branch2 and port woodlawn code
 *
 * Revision 1.1.2.2  2013/06/17 11:06:26  leschen
 * Remove static declaration from alter and dump utility
 *
 * Revision 1.1.2.1  2013/04/24 10:37:19  tirawan
 * Initial check-in for woodlawn linux code
 *
 * Revision 1.4  2013/04/10 09:48:48  kuangik
 * Implement do all wrapper for host run sm test
 *
 * Revision 1.3  2013/03/29 03:30:24  kuangik
 * Assign IP Address of BP XAUI, initialize TLK10232 and restore TLK10232 path after running internal loopback
 *
 * Revision 1.7  2013/03/12 11:25:12  leslie
 * Fix submenu flag for show error number message
 *
 * Revision 1.6  2013/03/07 12:40:41  leslie
 * Add TLK10232 utilities.
 *
 * Revision 1.5  2013/01/18 06:33:10  leslie
 * Call library to configure TLK10232 before do internal loopback.
 *
 * Revision 1.4  2013/01/16 02:07:42  leslie
 * Remove config TLK10232 function.
 *
 * Revision 1.2  2013/01/13 23:20:41  leslie
 * Fix configuration of XAUI B <-> XAUI A mode.
 *
 * Revision 1.1  2013/01/13 23:12:50  leslie
 * Initial check in TLK10232 code.
 *
 * $Endlog$
 *-------------------------------------------------
 */

