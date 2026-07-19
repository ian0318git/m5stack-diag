/* $Id: diag_tlk10232_test.c,v 1.2 2015/05/25 03:59:15 steja Exp $ 
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/skye_linux/src/diag_tlk10232_test.c,v $
 *-----------------------------------------------------------------------------
 * diag_tlk10232_test.c - Menu for Skye TLk10232
 * 
 * Ported from Woodlawn Project
 * May 2013, Steja
 * Copyright (c) 2015 by Cisco Systems, Inc.
 * All rights reserved.
 *-----------------------------------------------------------------------------
 */

#include "common.h"
#include "types.h"
#include "menu.h"
#include <stdio.h>
#include "diag_common_lib.h"
#include "skye_smi_lib.h"
#include "common_utils.h"
#include "diag_backplane_xaui_test.h"
#include "diag_tlk10232_lib.h"
#include "skye_xaui.h"
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
#include "menu.h"
#include "error.h"
#include "queryflags.h"
#include "router_if.h"
#include "sgmii_defs.h"
#include "ethernet.h"
#include "linux_eth.h"
#include "skye_eth.h"
#include <gxio/mpipe.h>
#include "nvmonvars.h"
#include "platform_fru.h"

/*****************************************************************************
 *  Functions Declaration
 *****************************************************************************/
int tlk_10232_test(int);
int tlk_10232_utility(int);
int tlk10232_do_all_wrapper(void);
static int tlk_10232_register_test(void);
static int tlk_10232_internal_loopback_test(void);
static int tlk_cpu0_to_chb_to_cpu0_polarity_test(void);
static int tlk_cpu0_to_chb_to_cpu0_no_polarity_test(void);
static int dump_tlk_10232_registers(void);
static int alter_tlk_10232_register(void);
extern int tlk10232_set_ch_a_loopback(void);
extern int tlk10232_set_ch_b_loopback(void);
extern int tlk10232_set_ch_a_host_ge_loopback(void);
extern int tlk10232_ge_host_lpbk_setup(boolean);
extern int tlk10232_xaui_host_lpbk_setup(void);
extern int tlk10232_set_ch_b_alt_loopback(void);
extern int xaui_cpu0_chb_to_cha_loopback_test(void);
extern int tlk10232_xaui_host_to_tlk_cha_to_chb_lpbk_setup(void);
extern int tlk_init_config(int);
extern int tlk10232_check_status(void);
extern int tlk10232_dump_all_reg(void);
extern void msleep(unsigned long);
extern int cpu0_xaui_bp_lp_test(void);
extern int tlk_init_config_3(boolean, boolean);
extern int xaui_cpu0_chb_to_cha_polarity_test(void);
extern int xaui_cpu0_chb_to_cha_no_polarity_test(void);
extern int xaui_cpu0_chb_to_cha_to_bp_polarity_test(void);
extern int xaui_cpu0_chb_to_cha_to_bp_no_polarity_test(void);
extern int tlk10232_xaui_host_polarity_set(void);
extern int tlk10232_xaui_host_no_polarity_set(void);
extern int host_to_cha_to_chb_to_cha_to_host_polarity_set(void);
extern int host_to_cha_to_chb_to_cha_to_host_no_polarity_set(void);
extern int host_full_path_to_CPU0_polarity_set(void);
extern int host_full_path_to_CPU0_no_polarity_set(void);
extern int tlk10232_setup_data_path_xaui_b_to_xaui_a(void);
extern int tlk10232_set_ch_b_deep_local_loopback(void);
extern int tlk10232_set_ch_b_shallow_local_loopback(void);
extern int tlk10232_set_ch_b_host_ge_loopback(boolean);
extern int tlk10232_set_ch_b_pcs_loopback(void);
extern int tlk10232_set_ch_b_pma_loopback(void);
extern boolean check_10gcap(void);
extern boolean check_non10gcap(void);
extern int tlk_init_config_10gkr_for_host_lbpk(boolean);
static int tlk_run_ping_test_to_host(void);
extern int is_host_xgbe2_up(boolean);
extern int tlk10232_optimize_ch_a(void);

#define MF_1    (MF_CONTINUOUS | MF_DOGRP)
#define MF_2    (MF_1 | MF_DOALL)
#define MF_3    (MF_2 | MF_SHOW_ERRCOUNT)
#define MF_4    (MF_1 | MF_SHOW_ERRCOUNT)

#define TI_PHY_RONLY    (READ_ONLY | REG_ACCESS)
#define TI_PHY_RW        (READ_WRITE | REG_ACCESS)

/*****************************************************************************
 *  For Register Test Use
 *****************************************************************************/
/* reg_info_t extension for "device 30" access */
static reg_info_t_ext tlk_10232_ext30 = {TLK_10232_REG_LEN,
                                         read_tlk_10232_reg,
                                         write_tlk_10232_reg,
                                         0};

/* reg_table for "device 31" */
static reg_info_t tlk_10232_dev30[] = {   /* Device 30*/
    {"Loopback TP Control", 0x000B, TI_PHY_RW,
     {(unsigned long)&tlk_10232_ext30}, 0xffff, 0x0D10},
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
   {"TLK 10232 Internal Loopback Test (CPU0 <--> TLK CH-B LS)",
       (type_t(*)())tlk_10232_internal_loopback_test,   0,
       MF_3, (type_t(*)())0, 0, (type_t(*)())0,   0},
   {"TLK10232 XAUI backplane loopback Test",
       (type_t(*)())xaui_backplane_loopback_test,   0,
       MF_3, (type_t(*)())check_non10gcap, 0, (type_t(*)())0,   0},
   {"Run ping test TLK10232 10G-KR to host",
       (type_t(*)())tlk_run_ping_test_to_host,   TRUE,
       MF_3, (type_t(*)())check_10gcap, 0, (type_t(*)())0,   0},
#ifdef DBG_10GKR
   {"TLK 10232 Internal Loopback Test (10G) (CPU0 <--> TLK CH-B HS )",
       (type_t(*)())tlk_10232_internal_loopback_test_10g,   0,
       0, (type_t(*)())0, 0, (type_t(*)())0,   0},
   {"TLK10232 10G-KR backplane loopback Test",
       (type_t(*)())ten_g_kr_bp_loopback_test,   0,
       0, (type_t(*)())check_10gcap, 0, (type_t(*)())0,   0},
#endif
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
    {"TLK10232 Check Status", (type_t(*)())tlk10232_check_status,   0,
      0, (type_t(*)())0, 0, (type_t(*)())0,   0},
    {"TLK10232 Dump Reg", (type_t(*)())tlk10232_dump_all_reg,   0,
      0, (type_t(*)())0, 0, (type_t(*)())0,   0},
    {"TLK 10232 GE host <--> TLK CH-B HS loopback setup", (type_t(*)())tlk10232_ge_host_lpbk_setup, 1, 0, (type_t(*)())0,
     0, (type_t(*)())0, 0},
    {"TLK 10232 XAUI host <--> TLK CH-A LS loopback setup", (type_t(*)())tlk10232_xaui_host_lpbk_setup, 0, 0, (type_t(*)())0,
     0, (type_t(*)())0, 0},
    {"TLK 10232 XAUI host <--> TLK CH-A LS <--> TLK CH-B LS loopback setup", (type_t(*)())tlk10232_xaui_host_to_tlk_cha_to_chb_lpbk_setup, 0, 0, (type_t(*)())0,
     0, (type_t(*)())0, 0},
    {"Setup BP XAUI <--> TLK CH-A LS <--> BP XAUI with polarity", (type_t(*)())tlk10232_xaui_host_polarity_set, 0, 0, (type_t(*)())0,
      0, (type_t(*)())0, 0},
    {"Setup BP XAUI <--> TLK CH-A LS <--> BP XAUI no polarity", (type_t(*)())tlk10232_xaui_host_no_polarity_set, 0, 0, (type_t(*)())0,
      0, (type_t(*)())0, 0},
    {"Setup BP XAUI <->CH-A<->CH-B<->CH-A <-> BP XAUI with polarity", (type_t(*)())host_to_cha_to_chb_to_cha_to_host_polarity_set, 0, 0, (type_t(*)())0,
        0, (type_t(*)())0, 0},
    {"Setup BP XAUI <->CH-A<->CH-B<->CH-A <-> BP XAUI no polarity", (type_t(*)())host_to_cha_to_chb_to_cha_to_host_no_polarity_set, 0, 0, (type_t(*)())0,
        0, (type_t(*)())0, 0},
    {"Setup BP XAUI <->CH-A<->CH-B<->GX CPU0 <->CH-B<->CH-A<-> BP XAUI with polarity", (type_t(*)())host_full_path_to_CPU0_polarity_set, 0, 0, (type_t(*)())0,
        0, (type_t(*)())0, 0},
    {"Setup BP XAUI <->CH-A<->CH-B<->GX CPU0 <->CH-B<->CH-A<-> BP XAUI no polarity", (type_t(*)())host_full_path_to_CPU0_no_polarity_set, 0, 0, (type_t(*)())0,
        0, (type_t(*)())0, 0},
    {"TLK (CPU0 <--> TLK CH-B LS <--> CPU0) Polarity set", (type_t(*)())tlk_cpu0_to_chb_to_cpu0_polarity_test, 0, 0,
     (type_t(*)())0, 0, (type_t(*)())0, 0},
    {"TLK (CPU0 <--> TLK CH-B LS <--> CPU0) No Polarity set", (type_t(*)())tlk_cpu0_to_chb_to_cpu0_no_polarity_test, 0,
      0, (type_t(*)())0, 0, (type_t(*)())0, 0},
    {"TLK (CPU0 <--> CH-B LS <-> CH-A LS) Polarity set", (type_t(*)())xaui_cpu0_chb_to_cha_polarity_test, 0,
      0, (type_t(*)())0, 0, (type_t(*)())0, 0},
    {"TLK (CPU0 <--> CH-B LS <-> CH-A LS) No Polarity set", (type_t(*)())xaui_cpu0_chb_to_cha_no_polarity_test, 0,
      0, (type_t(*)())0, 0, (type_t(*)())0, 0},
    {"TLK (CPU0 <--> CH-B LS <-> CH-A LS <-> Host BP <-> CH-A LS <-> CH-B LS <-> CPU0) Polarity set", (type_t(*)())xaui_cpu0_chb_to_cha_to_bp_polarity_test,   0,
      0, (type_t(*)())0, 0, (type_t(*)())0, 0},
    {"TLK (CPU0 <--> CH-B LS <-> CH-A LS <-> Host BP <-> CH-A LS <-> CH-B LS <-> CPU0) No Polarity set", (type_t(*)())xaui_cpu0_chb_to_cha_to_bp_no_polarity_test,   0,
      0, (type_t(*)())0, 0, (type_t(*)())0, 0},
    {"TLK10232 CPU0 <--> TLK CH-B LS <--> CH-A LS loopback Test", (type_t(*)())xaui_cpu0_chb_to_cha_loopback_test,   0,
      0, (type_t(*)())0, 0, (type_t(*)())0, 0},
    {"CPU0 XAUI Backplane Loopback Test",  (type_t(*)())cpu0_xaui_bp_lp_test,   0,
      0, (type_t(*)())0, 0, (type_t(*)())0, 0},
    {"Enable the optimization channel A LS",  (type_t(*)())tlk10232_optimize_ch_a,   0,
      0, (type_t(*)())0, 0, (type_t(*)())0, 0},
    {"Setup TLK10232 10G-KR from backplane loopback",
      (type_t(*)())tlk_init_config_10gkr_for_host_lbpk,   0,
      0, (type_t(*)())0, 0, (type_t(*)())0,   0},
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

    system("ifconfig xgbe2 up");

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
int
tlk10232_do_all_wrapper (void)
{
    int rc = PASSED;

    system("ifconfig xgbe2 up");

    if (tlk_10232_register_test() == FAILED) {
        return (FAILED);
    }

    if (tlk_10232_internal_loopback_test() == FAILED) {
        return (FAILED);
    }
    /* Has no 10G KR capability */
    if (check_10gcap() == FALSE) {
        if (xaui_backplane_loopback_test() == FAILED) {
            return (FAILED);
        }
    }
#if DBG_10GKR
    /* Has 10G KR capability */
    if (check_10gcap() == TRUE) {
        if (tlk_run_ping_test_to_host() == FAILED) {
            return (FAILED);
        }
    }
#endif

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
int
tlk_10232_utility (int show_menu)
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
static int
tlk_10232_register_test (void)
{
    testname("TLK 10232 Register");
    prpass(testpass, "TLK 10232 Register Test");
#ifdef SKYE_ENHANCED_ERR_MSG
    /*
     * 1. Subtests of the test function will reuse all variables
     * 2. All variables will be cleared automatically when
     *    entering and leaving each menu item.
     */
    /* Segment 1: PID | Unique_string : slot_info */
    fru_table_offset = SKYE_TLK10232;

    /* Segment 2: Test step captured from prpass */
    /* Segment 3: Failure message captured from cterr */

    /* Segment 4: Components used */
    cterr_add_component("Tilera CPU", "TLK 10232 Chips");

    /* Segment 5: register and memory dump */
    cterr_add_reg_dump((PFV)tlk10232_dump_all_reg);

    /* Segment 6: Platform Environment initialized here*/
    cterr_add_env_dump((PFV)skye_dump_volt_margins,
                       (PFV)skye_dump_temps);

    /* Segment 7: Top 3 Debugging Steps */
    cterr_add_debug("Check TLK10232 Register dump on segment no.5",
                    "Check TLK10232 Data specs which bit are not write able.",
                    "Check TLK10232 by manual register read & write.",
                    "Check TLK10232 H/W connection to CPU is ok.");
#endif   /* SKYE_ENHANCED_ERR_MSG */
    if (register_tests(0, tlk_10232_dev30) == FAILED) {
        cterr ('f', 0, "TLK Register Failed.");
        return (FAILED);
    }
    
    prcomplete(testpass, errcount, (char *)0);
    return (PASSED);
}

/******************************************************************************
 *
 * Function: tlk_10232_internal_loopback_test
 *
 * Description: This function perform the internal loopback test
 *                   from Tilera to tlk_10232.
 *
 * Inputs      : None
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/
static int
tlk_10232_internal_loopback_test (void)
{
    char cmd[32];
    testname("TLK 10232 internal loopback test");
    prpass(testpass, "TLK 10232 Internal Loopback Test");

#ifdef SKYE_ENHANCED_ERR_MSG
    /*
     * 1. Subtests of the test function will reuse all variables
     * 2. All variables will be cleared automatically when
     *    entering and leaving each menu item.
     */
    /* Segment 1: PID | Unique_string : slot_info */
    fru_table_offset = SKYE_TLK10232;

    /* Segment 2: Test step captured from prpass */
    /* Segment 3: Failure message captured from cterr */

    /* Segment 4: Components used */
    cterr_add_component("Tilera CPU", "TLK 10232 Ch-B LS");

    /* Segment 5: register and memory dump */
    cterr_add_reg_dump((PFV)tlk10232_dump_all_reg);

    /* Segment 6: Platform Environment initialized here*/
    cterr_add_env_dump((PFV)skye_dump_volt_margins,
                       (PFV)skye_dump_temps);

    /* Segment 7: Top 3 Debugging Steps */
    cterr_add_debug("Check TLK10232 Register dump on segment no.5",
                    "Check TLK10232 configured properly.",
                    "Check TLK10232 by manual register read & write.",
                    "Check TLK10232 H/W connection to CPU is ok.");
#endif   /* SKYE_ENHANCED_ERR_MSG */
    sprintf(cmd, "ifconfig xgbe2 up");
    system(cmd);
    sprintf(cmd, "ifconfig gbe4 down");
    system(cmd);
    msleep(100);

    if (diagflag_xram & D_TRACE) {
        printf("*** Before run the test \n");
        if (tlk10232_check_status() != PASSED) {
            cterr('f', 0, "failed tlk10232_check_status");
            return (FAILED);
        }
    }

    /* Configure TLK10232 to operate in XAUI mode.
        To loop the XAUI data coming from the Tilera processor back on itself,
        change bits 15:14 of register 0x1E.001A (DSR_DATA_SRC_SEL[1:0]) to 00
        to select the same channel's LS input as the LS output data source. */
    if (config_tlk_10232_mode(XAUIB_TO_XAUIB) == FAILED) {
        cterr('f', 0, "Config TLK10232 into XAUIB <->  XAUIB mode failed");
        return (FAILED);
    }

    if (diagflag_xram & D_TRACE) {
        printf("*** Before run send data \n");
        if (tlk10232_check_status() != PASSED) {
            cterr('f', 0, "failed tlk10232_check_status");
            return (FAILED);
        }
    }

    if (cpu0_xaui_bp_lp_test() == FAILED) {
        cterr('f', 0, "cpu0_xaui_bp_lp_test failed");
        return (FAILED);
    }

    if (diagflag_xram & D_TRACE) {
        printf("*** After run send data \n");
        if (tlk10232_check_status() != PASSED) {
            cterr('f', 0, "failed tlk10232_check_status");
            return (FAILED);
        }
    }

    sprintf(cmd, "ifconfig xgbe2 down");
    system(cmd);
    sprintf(cmd, "ifconfig gbe4 up");
    system(cmd);

    prcomplete(testpass, errcount, (char *)0);
    return (PASSED);
}

/******************************************************************************
 *
 * Function: tlk_cpu0_to_chb_to_cpu0_polarity_test
 *
 * Description: This function perform the loopback test
 *                   from Tilera CPU0 to TLK Channel B back to CPU0 has polarity.
 *
 * Inputs      : None
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/
static int
tlk_cpu0_to_chb_to_cpu0_polarity_test (void)
{
    testname("TLK CPU0 <--> CH-B LS <--> CPU0 (With Polarity) ");

    /* Init TLK10232 for XAUI Backplane Loopback */
    /* With Polarity */
    if(tlk_init_config_3(TRUE, TRUE) != PASSED) {
        cterr('f', 0, "failed tlk_init_config");
        return (FAILED);
    }
    msleep(1000);
    /* XAUI B to XAUI B */
    if(tlk10232_set_ch_b_loopback() != PASSED) {
        cterr('f', 0, "failed tlk10232_set_ch_b_loopback");
        return (FAILED);
    }

    if (cpu0_xaui_bp_lp_test() == FAILED) {
        cterr('f', 0, "cpu0_xaui_bp_lp_test failed");
        return (FAILED);
    }

    return (PASSED);
}

/******************************************************************************
 *
 * Function: tlk_cpu0_to_chb_to_cpu0_no_polarity_test
 *
 * Description: This function perform the loopback test
 *              from Tilera CPU0 to TLK Channel B back to CPU0 has no polarity.
 *
 * Inputs      : None
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/
static int
tlk_cpu0_to_chb_to_cpu0_no_polarity_test (void)
{
    testname("TLK CPU0 <--> CH-B LS <--> CPU0 (No Polarity) ");

    /* Init TLK10232 for XAUI Backplane Loopback */
    /* No Polarity */
    if(tlk_init_config_3(FALSE, TRUE) != PASSED) {
        cterr('f', 0, "failed tlk_init_config");
        return (FAILED);
    }
    msleep(1000);
    /* XAUI B to XAUI B */
    if(tlk10232_set_ch_b_loopback() != PASSED) {
        cterr('f', 0, "failed tlk10232_set_ch_b_loopback");
        return (FAILED);
    }

    if (cpu0_xaui_bp_lp_test() == FAILED) {
        cterr('f', 0, "cpu0_xaui_bp_lp_test failed");
        return (FAILED);
    }

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
static int
dump_tlk_10232_registers (void)
{
    printf("Dump TLK 10232 Register.\n");
    int port, phy_id, dev_id, reg_addr, mii_value, channel;
    
    /* TLK 10232 xgbe2 */
    port = 2;
    channel = getdec_answer("Select channel(0 - channel A, 1 - channel B) : ", 0, 0, 1);
    dev_id = gethex_answer("Select device address : ", 0, 0, 0xFF);
    reg_addr = gethex_answer("Select register address : ", 0, 0, 0xFFFF);

    if (channel == 0) {
        phy_id = TLK_10232_PHY_ADDR_CHANNEL_A;
    } else {
        phy_id = TLK_10232_PHY_ADDR_CHANNEL_B;
    }

    mii_value = skye_tlk_reg_rd(port, phy_id, dev_id, reg_addr);
    if (mii_value < 0) {
        printf("Read error from device %u(0x%x)", dev_id, phy_id);
        return (FAILED);
    } else {
        printf("Phy %#.2x reg %#.4x, data %#.4x\n", phy_id, reg_addr, mii_value);
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
static int
alter_tlk_10232_register (void)
{
    printf("Alter TLK 10232 Register.\n");
    int port, phy_id, dev_id, reg_addr, mii_value, reg_val, channel;

    /* TLK 10232 xgbe2 */
    port = 2;
    channel = getdec_answer("Select channel(0 - channel A, 1 - channel B) : ", 0, 0, 1);
    dev_id = gethex_answer("Select device address : ", 0, 0, 0xFF);
    reg_addr = gethex_answer("Select register address : ", 0, 0, 0xFFFF);

    if (channel == 0) {
        phy_id = TLK_10232_PHY_ADDR_CHANNEL_A;
    } else {
        phy_id = TLK_10232_PHY_ADDR_CHANNEL_B;
    }
    
    mii_value = skye_tlk_reg_rd(port, phy_id, dev_id, reg_addr);
    if (mii_value < 0) {
        printf("Read error from device %u(0x%x)", dev_id, phy_id);
        return (FAILED);
    } else {
        printf("Original Value = %#.4x\n", mii_value);
    }
    
    /* Alter register with new value */
    reg_val = gethex_answer("Enter the new data (hex): ", 0, 0, 0xFFFF);

    /* Alter the current register with new vlaue */
    mii_value = skye_tlk_reg_wr(port, phy_id, dev_id, reg_addr, reg_val);
    if (mii_value < 0) {
        printf("Write error from device %u(0x%x)", dev_id, phy_id);
        return (FAILED);
    } 

    /* Get new current register vlaue */
    mii_value = skye_tlk_reg_rd(port, phy_id, dev_id, reg_addr);
    if (mii_value < 0) {
        printf("Read error from device %u(0x%x)", dev_id, phy_id);
        return (FAILED);
    } else {
        printf("New Value = %#.4x\n", mii_value);
        return (PASSED);
    }
}

#ifdef DBG_10GKR
/******************************************************************************
 *
 * Function: tlk_10232_internal_loopback_test_10g
 *
 * Description: This function perform the internal loopback test
 *                   from Tilera to tlk_10232 (CHB LS <-> CHBHS).
 *
 * Inputs      : None
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/
static int
tlk_10232_internal_loopback_test_10g (void)
{
    boolean check1 = FALSE;  /* enable deep local loopback */
    boolean check2 = FALSE;  /* enable shallow local loopback */
    boolean check3 = FALSE;   /* switch ch-b to ch-b LS loopback*/
    boolean check4 = TRUE;  /* enable deep remote loopback */
    boolean check6 = FALSE;  /* enable pma loopback */

    testname("TLK 10232 internal loopback test 10G");
    prpass(testpass, "TLK 10232 Internal Loopback Test 10G");

#ifdef SKYE_ENHANCED_ERR_MSG
    /*
     * 1. Subtests of the test function will reuse all variables
     * 2. All variables will be cleared automatically when
     *    entering and leaving each menu item.
     */
    /* Segment 1: PID | Unique_string : slot_info */
    fru_table_offset = SKYE_TLK10232;

    /* Segment 2: Test step captured from prpass */
    /* Segment 3: Failure message captured from cterr */

    /* Segment 4: Components used */
    cterr_add_component("Tilera CPU", "TLK 10232 Ch-B LS", "TLK 10232 Ch-B HS");

    /* Segment 5: register and memory dump */
    cterr_add_reg_dump((PFV)tlk10232_dump_all_reg);

    /* Segment 6: Platform Environment initialized here*/
    cterr_add_env_dump((PFV)skye_dump_volt_margins,
                       (PFV)skye_dump_temps);

    /* Segment 7: Top 3 Debugging Steps */
    cterr_add_debug("Check TLK10232 Register dump on segment no.5",
                    "Check TLK10232 configured properly.",
                    "Check TLK10232 by manual register read & write.",
                    "Check TLK10232 H/W connection to CPU is ok.");
#endif   /* SKYE_ENHANCED_ERR_MSG */
    if (diagflag_xram & D_SET_OPTIONS) {
        if (getc_answer("enable deep local loopback?(y/n)", "yn",'n') == 'y')
            check1 = TRUE;
        else
            check1 = FALSE;

        if (getc_answer("enable shallow local loopback ?(y/n)", "yn",'n') == 'y')
            check2 = TRUE;
        else
            check2 = FALSE;

        if (getc_answer("enable deep remote loopback?(y/n)", "yn",'n') == 'y')
            check4 = TRUE;
        else
            check4 = FALSE;

        if (getc_answer("switch ch-b to ch-b LS loopback ?(y/n)", "yn",'n') == 'y')
            check3 = TRUE;
        else
            check3 = FALSE;

        if (getc_answer("enable pma loopback ?(y/n)", "yn",'n') == 'y')
            check6 = TRUE;
        else
            check6 = FALSE;

    }
#ifdef DEBUG
    /* Configure TLK10232 to operate in XAUI mode.
        To loop the XAUI data coming from the Tilera processor back on itself,
        change bits 15:14 of register 0x1E.001A (DSR_DATA_SRC_SEL[1:0]) to 00
        to select the same channel's LS input as the LS output data source. */
    if (config_tlk_10232_mode(XAUIB_TO_XAUIB) == FAILED) {
        cterr('f', 0, "Config TLK10232 into XAUIB <->  XAUIB mode failed");
        return (FAILED);
    }
#endif
    if (diagflag_xram & D_TRACE) {
        printf("*** Before run the test \n");
        if (tlk10232_check_status() != PASSED) {
            cterr('f', 0, "failed tlk10232_check_status");
            return (FAILED);
        }
    }

    /* Init TLK10232 for XAUI Backplane Loopback */
    if(tlk_init_config(NONTLK10G) != PASSED) {
        cterr('f', 0, "failed tlk_init_config");
        return (FAILED);
    }
    msleep(1000);

    if (check3 == TRUE) {
        /* XAUI B to XAUI B */
        if(tlk10232_set_ch_b_loopback() != PASSED) {
            cterr('f', 0, "failed tlk10232_set_ch_b_loopback");
            return (FAILED);
        }
        msleep(10);
    }

    if (check1 == TRUE) {
        if(tlk10232_set_ch_b_deep_local_loopback() != PASSED) {
            cterr('f', 0, "failed tlk10232_set_ch_b_deep_local_loopback");
            return (FAILED);
        }
    }

    if (check2 == TRUE) {
        if(tlk10232_set_ch_b_shallow_local_loopback() != PASSED) {
            cterr('f', 0, "failed tlk10232_set_ch_b_shallow_local_loopback");
            return (FAILED);
        }
    }

    if (check4 == TRUE) {
        if(tlk10232_set_ch_b_host_ge_loopback(TRUE) != PASSED) {
            cterr('f', 0, "failed tlk10232_set_ch_b_deep_remote_loopback");
            return (FAILED);
        }
    }

    if (check6 == TRUE) {
        if(tlk10232_set_ch_b_pma_loopback() != PASSED) {
            cterr('f', 0, "failed tlk10232_set_ch_b_pcs_loopback");
            return (FAILED);
        }
    }

    if (diagflag_xram & D_TRACE) {
        printf("*** Before run send data \n");
        if (tlk10232_check_status() != PASSED) {
            cterr('f', 0, "failed tlk10232_check_status");
            return (FAILED);
        }
    }

    if (cpu0_xaui_bp_lp_test() == FAILED) {
        cterr('f', 0, "cpu0_xaui_bp_lp_test failed");
        return (FAILED);
    }

    if (diagflag_xram & D_TRACE) {
        printf("*** After run send data \n");
        if (tlk10232_check_status() != PASSED) {
            cterr('f', 0, "failed tlk10232_check_status");
            return (FAILED);
        }
    }

    return (PASSED);
}
#endif

/******************************************************************************
 *
 * Function: tlk_run_ping_test_to_host
 *
 * Description: This function perform the ping interface test
 *                   from Tilera to Host GE SW (SKYE CPU <-> TLK <-> Host GE SW).
 *
 * Inputs      : None
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/
static int tlk_run_ping_test_to_host (void)
{
    char cmd[32];
    int retry_ping=5;

    testname("TLK 10232 Ping interface test");
    prpass(testpass, "TLK 10232 test 10G-KR");

#ifdef SKYE_ENHANCED_ERR_MSG
    /*
     * 1. Subtests of the test function will reuse all variables
     * 2. All variables will be cleared automatically when
     *    entering and leaving each menu item.
     */
    /* Segment 1: PID | Unique_string : slot_info */
    fru_table_offset = SKYE_TLK10232;

    /* Segment 2: Test step captured from prpass */
    /* Segment 3: Failure message captured from cterr */

    /* Segment 4: Components used */
    cterr_add_component("Tilera CPU", "TLK 10232 Ch-B LS", "TLK 10232 Ch-B HS", "HOST GE SW");

    /* Segment 5: register and memory dump */
    cterr_add_reg_dump((PFV)tlk10232_dump_all_reg);

    /* Segment 6: Platform Environment initialized here*/
    cterr_add_env_dump((PFV)skye_dump_volt_margins,
                       (PFV)skye_dump_temps);

    /* Segment 7: Top 3 Debugging Steps */
    cterr_add_debug("Check TLK10232 Register dump on segment no.5",
                    "Check TLK10232 configured properly.",
                    "Check TLK10232 by manual register read & write.",
                    "Check TLK10232 H/W connection to CPU is ok.");
#endif   /* SKYE_ENHANCED_ERR_MSG */
    sprintf(cmd, "ifconfig xgbe2 up");
    system(cmd);
    sprintf(cmd, "ifconfig gbe4 down");
    system(cmd);

retry_again:
    if (tlk_init_config_10gkr_for_host_lbpk(FALSE) != PASSED) {
        cterr('f', 0, "failed tlk_init_config_10gkr_for_host_lbpk");
        return (FAILED);
    }
    /* Delay 3 sec to wait the connection is up */
    msleep(3000);

    if (is_host_xgbe2_up(TRUE) != PASSED) {
        if (retry_ping == 0) {
            cterr('f', 0, "failed is_host_xgbe2_up");
            return (FAILED);
        } else {
            retry_ping--;
            printf("retry to ping %d times\n", retry_ping);
            goto retry_again;
        }
    }

    sprintf(cmd, "ifconfig xgbe2 down");
    system(cmd);
    sprintf(cmd, "ifconfig gbe4 up");
    system(cmd);

    prcomplete(testpass, errcount, (char *)0);
    return (PASSED);
}

/*-------------------------------------------------
 * $Log: diag_tlk10232_test.c,v $
 * Revision 1.2  2015/05/25 03:59:15  steja
 * Add Support Skye SM
 *
 * Revision 1.1.4.5  2015/05/20 09:43:14  steja
 * Fix TLK missing code after code review <CDETS CSCuu01237>
 *
 * Revision 1.1.4.4  2015/05/05 11:53:12  steja
 * CDETS[CSCuu01237] Solving TLK intermittent loopback issue on GH platform.
 *
 * Revision 1.1.4.3  2015/04/29 13:30:38  steja
 * Update TLK 10G-KR test path
 *
 * Revision 1.1.4.2  2015/04/29 11:36:32  steja
 * Code check-in to skye-branch2 for ER code review
 *
 *-------------------------------------------------
 * Revision 1.1.2.21  2015/03/27 06:46:59  steja
 * Optimization XAUI Channel A LS based on HW register
 *
 * Revision 1.1.2.20  2015/02/12 12:42:04  steja
 * Code clean up
 *
 * Revision 1.1.2.19  2014/11/27 02:32:50  steja
 * 1.Fix the intermittent failure to run do all test(CSCur27613)
 * 2.Update TLK full data path by ping test.
 *
 * Revision 1.1.2.18  2014/11/21 09:37:34  steja
 * Support Full data path loopback for 10G-KR by ping test
 *
 * Revision 1.1.2.17  2014/11/11 07:59:01  steja
 * Fix the enable loopback function.
 *
 * Revision 1.1.2.16  2014/11/10 09:42:44  steja
 * Update TLK10232 10G KR loopback setup
 *
 * Revision 1.1.2.15  2014/10/14 06:31:10  steja
 * Fix appropriate return failed for do all test.
 *
 * Revision 1.1.2.14  2014/09/26 09:05:53  steja
 * (CSCuq98591)Fix GBE4 link issue
 *
 * Revision 1.1.2.13  2014/09/18 07:18:43  steja
 * 1.Update NC command codei
 * 2.Update enhanced error message
 *
 * Revision 1.1.2.12  2014/09/17 11:13:15  palin2
 * Removed unused code.
 *
 * Revision 1.1.2.11  2014/09/15 07:58:59  steja
 * Code Clean up
 *
 * Revision 1.1.2.10  2014/09/12 14:38:42  steja
 * Update code for CPU do all test
 *
 * Revision 1.1.2.9  2014/08/31 23:01:13  palin2
 * Updated enhanced error message FRU table offset.
 *
 * Revision 1.1.2.8  2014/08/31 15:59:28  steja
 * Add enhanced error messages
 *
 * Revision 1.1.2.7  2014/08/28 02:54:26  steja
 * Support Do all test for NC command
 *
 * Revision 1.1.2.6  2014/08/25 11:55:49  steja
 * Update Code for BST Testing
 *
 * Revision 1.1.2.5  2014/08/13 11:51:49  steja
 * Add 10GKR determine function
 *
 * Revision 1.1.2.4  2014/08/12 12:33:14  steja
 * Update 10GKR loopback test code
 *
 * Revision 1.1.2.3  2014/08/08 11:49:57  steja
 * Add 10G-KR loopback test
 *
 * Revision 1.1.2.2  2014/08/08 09:47:52  steja
 * Fix internal loopback test [CSCup56604]
 *
 * Revision 1.1.2.1  2014/07/21 01:56:53  palin2
 * Initial check-in Skye module side Diag code.
 *
 *-------------------------------------------------
 * Revision 1.2.8.2  2014/05/13 03:00:31  steja
 * Update the sequence to setup host to TLK loopback.
 *
 * Revision 1.2  2014/02/27 15:01:47  palin2
 * Initial check-in ShrinkRay SM side Diag code.
 *
 * Revision 1.1.4.13  2014/02/27 04:13:17  steja
 * 1. move cpu0 tlk to tlk submenu utility
 * 2. modify msleep for pse2 and dual cpu lopback test
 *
 * Revision 1.1.4.12  2014/02/07 18:49:06  steja
 * Update menu
 *
 * Revision 1.1.4.10  2014/01/06 13:00:33  steja
 * 1. clean up code
 * 2. Add header TLK code
 *
 * Revision 1.1.4.9  2013/11/29 07:08:55  steja
 * 1. Fix the full data path TLK working.
 * 2. add USB test
 * 3. add read BIB MAC utility
 *
 * Revision 1.1.4.8  2013/11/19 14:36:47  steja
 * Provide TLK utility for debugging
 * Update the BTK TLK into coded
 *
 * Revision 1.1.4.7  2013/11/05 09:17:54  steja
 * 1. Fix the MDIO not stable issue
 * 2. debug tlk log
 *
 * Revision 1.1.4.6  2013/10/10 00:36:22  steja
 * 1. Add TLK Utility PLL and Polarity TX RX switch
 * 2. Code update
 *
 * Revision 1.1.4.5  2013/10/05 06:20:24  steja
 * Update for debug
 *
 * Revision 1.1.4.4  2013/09/27 07:25:13  steja
 * update code for bringup
 *
 * Revision 1.1.4.3  2013/09/16 09:50:15  iachang
 * Code review and update
 *
 * Revision 1.1.4.2  2013/09/13 07:00:07  palin2
 * Initial check-in ShrinkRay SM side Diag code.
 *
 * Revision 1.1.2.1  2013/06/24 09:03:34  steja
 * Checkin :
 * - Support TLK10323 Loopback test & Utility
 * - Support MV1514 Loopback test
 *
 *-------------------------------------------------
 * $Endlog$
 */

