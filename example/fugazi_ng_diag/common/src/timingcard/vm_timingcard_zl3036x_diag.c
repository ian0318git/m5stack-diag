/* $Id: vm_timingcard_zl3036x_diag.c,v 1.3 2017/07/14 02:51:39 alpeng Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/timingcard/vm_timingcard_zl3036x_diag.c,v $
 *------------------------------------------------------------------
 * Filename: vm_timingcard_zl3036x_diag.c
 *
 * Description: The Timing Card ZL3036X main source code
 * Author: Kody Ko
 *
 * Copyright (c) 2013 - 2017 by cisco Systems, Inc.
 * All rights reserved.
 *------------------------------------------------------------------
 */

#include "common.h"
#include "types.h"
#include "proto.h"
#include "strings.h"
#include "menu.h"
#include "nvsysvars.h"
#include "error.h"
#include "i2c_api.h"
#include "platform_i2c.h"
#include "common_utils.h"
#include "platform_fru.h"
#include "platform_cookie.h"
#include "vm_timingcard_cpld_diag.h"
#include "vm_timingcard_cpld_lib.h"
#include "vm_timingcard_zl3036x_lib.h"
#include "vm_timingcard_zl3036x_diag.h"

#include <stdio.h>
#include <string.h>

/***********************************************************************
 *  Macro Definitions
 ************************************************************************/

/***********************************************************************
 *  Static Functions Declaration
 ************************************************************************/
static long timingcard_zl3036x_reg_test(void);
static long timingcard_ref0_clock_input_test(void);
static long timingcard_ref1_clock_input_test(void);
static long timingcard_ref2_clock_input_test(void);
static long timingcard_hpoutclk3_clock_lpbk_test(void);
static long timingcard_hpoutclk4_clock_lpbk_test(void);
static long timingcard_hpoutclk0_clock_lpbk_test(void);
#ifdef CANNOT_VERIFY
static long timingcard_hpoutclk2_clock_lpbk_test(void);
#endif
static long timingcard_zl3036x_reg_alter(void);
static long timingcard_zl3036x_reg_display(void);
static long sel_zl3036x_ref_clock(void);
static long zl3036x_clock_out_to_o2(void);
static long timingcard_hpoutclk1_1pps_clock_lpbk_test(void);

/***********************************************************************
 *  Functions Declaration
 ************************************************************************/
long build_timingcard_3036x_menu(int);
long timingcard_zl3036x_utility_submenu(int);
void enhance_error_code(char *, char *, char *, char *);

/***********************************************************************
 * Extern function prototypes
 ***********************************************************************/
extern int do_all_menu_items(struct menuinfo *);
extern int timingcard_init_seq(void);
extern int timingcard_clk_trig_verify_test(int);
extern int get_timingcard_sku_id(void);

/***********************************************************************
 *  Global Variable
 ************************************************************************/

static submenu_xtable_t timingcard_30363_submenu_tbl[] = {
    { "ZL3036X utility", (type_t(*)())timingcard_zl3036x_utility_submenu, 0,
      0, (type_t(*)())0, 0, (type_t(*)())0, 0 },
    { "ZL3036X Register Test", (type_t(*)())timingcard_zl3036x_reg_test, 0,
     MF_CONTINUOUS | MF_DOALL, (type_t(*)())0, 0, (type_t(*)())0, 0 },
    { "REF 0 Clock Input Test", (type_t(*)())timingcard_ref0_clock_input_test, 0,
     MF_CONTINUOUS | MF_DOALL, (type_t(*)())0, 0, (type_t(*)())0, 0 },
    { "REF 1 Clock Input Test", (type_t(*)())timingcard_ref1_clock_input_test, 0,
     MF_CONTINUOUS | MF_DOALL, (type_t(*)())0, 0, (type_t(*)())0, 0 },
    { "ZL30363 HPOUTCLK3 Clock Loopback Test", (type_t(*)())timingcard_hpoutclk3_clock_lpbk_test, 0,
     MF_CONTINUOUS | MF_DOALL, (type_t(*)())0, 0, (type_t(*)())0, 0 },
    { "ZL30363 HPOUTCLK4 Clock Loopback Test", (type_t(*)())timingcard_hpoutclk4_clock_lpbk_test, 0,
     MF_CONTINUOUS | MF_DOALL, (type_t(*)())0, 0, (type_t(*)())0, 0 },
    { "ZL30363 HPOUTCLK0 Clock Loopback Test", (type_t(*)())timingcard_hpoutclk0_clock_lpbk_test, 0,
     MF_CONTINUOUS | MF_DOALL, (type_t(*)())0, 0, (type_t(*)())0, 0 },
    { "ZL30363 HPOUTCLK1 1PPS Clock Loopback Test", (type_t(*)())timingcard_hpoutclk1_1pps_clock_lpbk_test, 0,
     MF_CONTINUOUS | MF_DOALL, (type_t(*)())0, 0, (type_t(*)())0, 0 },
    { "Timingcard clk/trig verification", (type_t(*)())timingcard_clk_trig_verify_test, 0,
      MF_CONTINUOUS | MF_DOALL, (type_t(*)())0, 0, (type_t(*)())0, 0 },
};

#define TIMINGCARD_ZL3036X_SUBMENU_TABLE_SZ \
                (sizeof(timingcard_30363_submenu_tbl)/sizeof(submenu_xtable_t))

/*
 * Primary & secondary submenu items (filled in from xtable)
 */
static mitem_t timingcard_30363_primary_items[TIMINGCARD_ZL3036X_SUBMENU_TABLE_SZ +
                                             MAX_BASE_ITEMS];
static mitem_t timingcard_30363_secondary_items[TIMINGCARD_ZL3036X_SUBMENU_TABLE_SZ +
                                               MAX_BASE_ITEMS];

static menuinfo_t timingcard_zl30363_main_menu = {
    "Timing Card ZL30363 Menu",
    0,                        /* mtparam added by init_empty_menu */
    0,                        /* notes missing WICs in combos */
    0,                        /* use generic prompt */
    0,                        /* size (bumped by add_menu_item() */
    timingcard_30363_primary_items,
};
static menuinfo_t *timingcard_zl30363_menup = &timingcard_zl30363_main_menu;

/*
 * Timing Card utilities menu on Overlord platform
 */
static mitem_t timingcard_zl3036x_util_submenu_table[] = {
    { "Alter ZL3036X Register", 0, 0, timingcard_zl3036x_reg_alter,
      (long *)&zero, 0, (type_t(*)())0, 0 },
    { "Display ZL3036X Register", 0, 0, timingcard_zl3036x_reg_display,
      (long *)&zero, 0, (type_t(*)())0, 0 },
    { "Select ZL3036X Ref Clock Utility", 0, 0, sel_zl3036x_ref_clock,
      (long *)&zero, 0, (type_t(*)())0, 0 },
    { "ZL3036X Clock Path to O2 Utility", 0, 0, zl3036x_clock_out_to_o2,
      (long *)&zero, 0, (type_t(*)())0, 0 },

};

#define TIMINGCARD_ZL3036X_UTIL_SUBMENU_TABLE_SZ \
        (sizeof(timingcard_zl3036x_util_submenu_table)/sizeof(mitem_t))

static menuinfo_t timingcard_zl3036x_util_subtest_menu = {
    "Timing Card 3036X Utilities Menu",
    0,                                  /* title param */
    0,                                  /* show diag flags */
    0,
    TIMINGCARD_ZL3036X_UTIL_SUBMENU_TABLE_SZ,
    timingcard_zl3036x_util_submenu_table,
};

static menuinfo_t *timingcard_zl3036x_util_submenup = &timingcard_zl3036x_util_subtest_menu;

static submenu_xtable_t timingcard_30361_submenu_tbl[] = {
    { "ZL3036X utility", (type_t(*)())timingcard_zl3036x_utility_submenu, 0,
      0, (type_t(*)())0, 0, (type_t(*)())0, 0 },
    { "ZL3036X Register Test", (type_t(*)())timingcard_zl3036x_reg_test, 0,
     MF_CONTINUOUS | MF_DOALL, (type_t(*)())0, 0, (type_t(*)())0, 0 },
    { "REF 0 Clock Input Test", (type_t(*)())timingcard_ref0_clock_input_test, 0,
     MF_CONTINUOUS | MF_DOALL, (type_t(*)())0, 0, (type_t(*)())0, 0 },
    { "REF 1 Clock Input Test", (type_t(*)())timingcard_ref1_clock_input_test, 0,
     MF_CONTINUOUS | MF_DOALL, (type_t(*)())0, 0, (type_t(*)())0, 0 },
    { "REF 2 Clock Input Test", (type_t(*)())timingcard_ref2_clock_input_test, 0,
     MF_CONTINUOUS | MF_DOALL, (type_t(*)())0, 0, (type_t(*)())0, 0 },
    { "Timingcard clk/trig verification", (type_t(*)())timingcard_clk_trig_verify_test, 0,
      MF_CONTINUOUS | MF_DOALL, (type_t(*)())0, 0, (type_t(*)())0, 0 },
};

#define TIMINGCARD_ZL30361_SUBMENU_TABLE_SZ \
                (sizeof(timingcard_30361_submenu_tbl)/sizeof(submenu_xtable_t))

/*
 * Primary & secondary submenu items (filled in from xtable)
 */
static mitem_t timingcard_30361_primary_items[TIMINGCARD_ZL30361_SUBMENU_TABLE_SZ +
                                             MAX_BASE_ITEMS];
static mitem_t timingcard_30361_secondary_items[TIMINGCARD_ZL30361_SUBMENU_TABLE_SZ +
                                               MAX_BASE_ITEMS];

static menuinfo_t timingcard_zl30361_main_menu = {
    "Timing Card ZL30361 Menu",
    0,                        /* mtparam added by init_empty_menu */
    0,                        /* notes missing WICs in combos */
    0,                        /* use generic prompt */
    0,                        /* size (bumped by add_menu_item() */
    timingcard_30361_primary_items,
};
static menuinfo_t *timingcard_zl30361_menup = &timingcard_zl30361_main_menu;

/***********************************************************************
 *  Functions
 ************************************************************************/

/**********************************************************************
 *
 * Function: build_timingcard_3036x_menu
 *
 * Description: Build Timing Card 3036X tests and utilities menu.
 *
 * Inputs:  show_menu - FALSE for tests. TRUE for submenu.
 *
 * Outputs: PASSED/FAILED.
 *
 **********************************************************************
 */
long build_timingcard_3036x_menu (int show_menu)
{
    if (get_timingcard_sku_id() == SKU_30361) {
        /* SKU 30361 menu */
        build_primary_submenu(timingcard_30361_submenu_tbl, TIMINGCARD_ZL30361_SUBMENU_TABLE_SZ,
                              "30361 Main Menu", &timingcard_zl30361_menup);
        build_secondary_submenu(timingcard_30361_submenu_tbl, TIMINGCARD_ZL30361_SUBMENU_TABLE_SZ,
                                timingcard_30361_secondary_items);

        if (show_menu) {
            /* Entered with submenu */
            menu(timingcard_zl30361_menup, timingcard_30361_secondary_items, 0);
        } else {
            /* Invoked the test from main menu */
            do_all_menu_items(timingcard_zl30361_menup);
        }
    } else {
        /* SKU 30363 menu */
        build_primary_submenu(timingcard_30363_submenu_tbl, TIMINGCARD_ZL3036X_SUBMENU_TABLE_SZ,
                              "30363 Main Menu", &timingcard_zl30363_menup);
        build_secondary_submenu(timingcard_30363_submenu_tbl, TIMINGCARD_ZL3036X_SUBMENU_TABLE_SZ,
                                timingcard_30363_secondary_items);

        if (show_menu) {
            /* Entered with submenu */
            menu(timingcard_zl30363_menup, timingcard_30363_secondary_items, 0);
        } else {
            /* Invoked the test from main menu */
            do_all_menu_items(timingcard_zl30363_menup);
        }
    }

    return(PASSED);
}

/**********************************************************************
 *
 * Function: timingcard_zl3036x_utility_submenu().
 *
 * This function implements the Timing Card ZL3036X test/menu
 *
 * Input: show menu option
 *
 * Output: PASSED/FAILED.
 *
 **********************************************************************
 */
long timingcard_zl3036x_utility_submenu (int menu_option)
{
    menu(timingcard_zl3036x_util_submenup, timingcard_zl3036x_util_submenu_table, '\0');

    return (PASSED);
}

/**********************************************************************
 *
 * Function: enhance_error_code
 *
 * Wrapper for enhance error code message function
 *
 * Input : *tanme - pointer to the test name
 *         *debug_step1 - pointer to the step 1 debug message
 *         *debug_step2 - pointer to the step 2 debug message
 *         *debug_step3 - pointer to the step 3 debug message
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
void enhance_error_code (char *tname, char *debug_step1, char *debug_step2,
                         char *debug_step3)
{
    char mb_get_pid[FRU_SIZE] = {0};
    char mb_get_loc[FRU_SIZE] = {0};

    /*
     * 1. Subtests of the test function will reuse all variables
     * 2. All variables will be cleared automatically when
     *    entering and leaving each menu item.
     */
    /* Segment 1: PID | Unique_string : slot_info */
    fru_table_offset = VM;
    /* fru_table_offset should be set, otherwise, it will not */
    /* go to enhanced error message format in cterr() */
    /* set fru_table_offset to get the predefine value */
    /* or change mb_pid & mb_loc below */

    get_mb_pid(mb_get_pid);
    strcpy(mb_get_loc, "MB-TimingCard");
    platform_fru_table[fru_table_offset].pid_string = (uchar *)mb_get_pid ;
    platform_fru_table[fru_table_offset].location_string = (uchar *)mb_get_loc;

    /* Segment 2: Test step captured from prpass */
    /* Segment 3: Failure message captured from cterr */

    /* Segment 4: Components used */
    cterr_add_component("ZL3036X");

#ifdef ENABLE_TO_USE
    /* Segment 5: register and memory dump */
    cterr_add_reg_dump((PFV)display_uart_regs_cterr_wrapper,
                        (PFI)display_multiboot);

    /* Segment 6: Platform Environment initialized here*/
    cterr_add_env_dump((PFV)show_margins_cterr_wrapper,
                        (PFV)show_temp_cterr_wrapper);
#endif

    /* Segment 7: Top 3 Debugging Steps */
    cterr_add_debug(debug_step1, debug_step2, debug_step3);

    testname("%s", tname);
    prpass(testpass, "%s, ", tname);
}

/**********************************************************************
 *
 * Function: timingcard_zl3036x_reg_test
 *
 * Wrapper for ZL3036X Register test.
 *
 * Input : None
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
static long timingcard_zl3036x_reg_test (void)
{
    char *tname = "Timing Card ZL3036X Register";
    char *debug_step1 = "Do the CPLD register test; this will clarify if the "
                        "path from Overlord Intel processor to timing card "
                        "CPLD and the CPLD firmware are working well.";
    char *debug_step2 = "If step 1 is passed, try to replace ZL3036X (U2A, "
                        "U2B, U2C and U2D).";
    char *debug_step3 = "None.";

    /* Initialize the enhance error code. */
    enhance_error_code(tname, debug_step1, debug_step2, debug_step3);

    /* Initialize the timging card */
    if (timingcard_init_seq() == FAILED) {
        return (FAILED);
    }

    if (timingcard_zl3036x_reg_test_lib() == FAILED) {
        return (FAILED);
    }

    return (PASSED);
}

/**********************************************************************
 *
 * Function: timingcard_ref0_clock_input_test
 *
 * Description: This function perform the reference 0 clock input test.
 *              Check if the ZL3036X locks the clock.
 *
 * Input : None
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
static long timingcard_ref0_clock_input_test (void)
{
    int ret = PASSED, ix = 0;

    char *tname = "Timing Card ZL3036X Reference 0 Clock Input";
    char *debug_step1 = "Do the ZL3036X register test; this will clarify if "
                        "the ZL3036X chipset is good.";
    char *debug_step2 = "If step 1 passes, do the CPLD register test; this "
                        "will clarify if the CPLD is good.";
    char *debug_step3 = "If step 1 and step 2 pass, then checks if the clock "
                        "source from platform CPLD SYNC_OUT_CPLD is good.";

    /* Initialize the enhance error code. */
    enhance_error_code(tname, debug_step1, debug_step2, debug_step3);

    /* Initialize the timging card */
    if (timingcard_init_seq() == FAILED) {
        return (FAILED);
    }

    for (ix = ZL3036X_8K_HZ; ix <= ZL3036X_25M_HZ; ix++) {
        if (ix == ZL3036X_8K_HZ) {
            prpass(testpass, "Testing 8KHz");
        } else {
            prpass(testpass, "Testing 25MHz");
        }

        /* Configures the O2 dash fpga SYNC/TRIG Control Register
         * to select the clock frequency that outputs to timing card. */
        timingcard_o2_set_sync_trig_out(ix, DASH_FPGA_SYNC_INTER_PLL, FALSE);

        /* Perform the ZL3036X reference 0 clock input test */
        ret = timingcard_zl3036x_ref_x_lib(ZL3036X_REF_0, ix);

        /* Restore to the original value. */
        timingcard_o2_disable_sync_trig_out();
    }

    return (ret);
}

/**********************************************************************
 *
 * Function: timingcard_hpoutclk3_clock_lpbk_test
 *
 * Description: This function perform the HPOUTCLK3 clock loopback test.
 *              Check if the ZL3036X locks the loopback clock.
 *              Clock Path:
 *              O2 FPGA SYN_OUT -> CPLD -> ZL3036X REF0 -> DPLL1 -> Synth 1
 *              -> HPOUTCLK3 -> ZL3036X REF 3 -> DPLL0 (check if locks)
 *
 * Input : None
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
static long timingcard_hpoutclk3_clock_lpbk_test (void)
{
    int ret = PASSED, sel_freq, ix = 0;

    char *tname = "ZL30363 HPOUTCLK3 Clock Loopback Test";
    char *debug_step1 = "Do the ZL3036X register test; this will clarify if "
                        "the ZL3036X chipset is good.";
    char *debug_step2 = "If step 1 passes, do the CPLD register test; this "
                        "will clarify if the CPLD is good.";
    char *debug_step3 = "If step 1 and step 2 pass, then checks if the clock "
                        "source from platform CPLD SYNC_OUT_CPLD is good.";

    /* Initialize the enhance error code. */
    enhance_error_code(tname, debug_step1, debug_step2, debug_step3);

    /* Initialize the timging card */
    if (timingcard_init_seq() == FAILED) {
        return (FAILED);
    }

    for (ix = ZL3036X_8K_HZ; ix <= ZL3036X_25M_HZ; ix++) {
        if (ix == ZL3036X_8K_HZ) {
            sel_freq = ZL3036X_8K_HZ;
            prpass(testpass, "Testing 8KHz");
        } else {
            sel_freq = ZL3036X_25M_HZ;
            prpass(testpass, "Testing 25MHz");
        }

        /* Configures the O2 dash fpga SYNC/TRIG Control Register
         * to select the clock frequency that outputs to timing card. */
        timingcard_o2_set_sync_trig_out(sel_freq, DASH_FPGA_SYNC_INTER_PLL, FALSE);

        /* Perform the ZL3036X reference 0 clock input test */
        ret = timingcard_zl3036x_ref_x_lpbk_lib(ZL3036X_REF_0, sel_freq, TRUE, FALSE);

        /* Restore to the original value. */
        timingcard_o2_disable_sync_trig_out();
    }

    return (ret);
}

/**********************************************************************
 *
 * Function: timingcard_hpoutclk1_1pps_clock_lpbk_test
 *
 * Description: This function perform the HPOUTCLK1 1PPS clock loopback test.
 *              Check if the ZL3036X locks the loopback clock.
 *              Clock Path:
 *              O2 FPGA SYN_OUT -> CPLD -> ZL3036X REF0 -> DPLL0 -> Synth 0
 *              -> HPOUTCLK1 -> ZL3036X REF 2 -> DPLL1 (check if locks)
 *
 * Input : None
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
static long timingcard_hpoutclk1_1pps_clock_lpbk_test (void)
{
    int ret = PASSED, sel_freq;

    char *tname = "ZL30363 HPOUTCLK1 1PPS Clock Loopback Test";
    char *debug_step1 = "Do the ZL3036X register test; this will clarify if "
                        "the ZL3036X chipset is good.";
    char *debug_step2 = "If step 1 passes, do the CPLD register test; this "
                        "will clarify if the CPLD is good.";
    char *debug_step3 = "If step 1 and step 2 pass, then checks if the clock "
                        "source from platform CPLD SYNC_OUT_CPLD is good.";

    /* Initialize the enhance error code. */
    enhance_error_code(tname, debug_step1, debug_step2, debug_step3);

    /* Initialize the timging card */
    if (timingcard_init_seq() == FAILED) {
        return (FAILED);
    }

    /* O2 FPGA SYN_OUT output 8KHz to CPLD and ZL3036X REF0.  */
    sel_freq = ZL3036X_8K_HZ;

    prpass(testpass, "Testing 8KHz");

    /* Configures the O2 dash fpga SYNC/TRIG Control Register
     * to select the clock frequency that outputs to timing card. */
    timingcard_o2_set_sync_trig_out(sel_freq, DASH_FPGA_SYNC_INTER_PLL, TRUE);

    /* Perform the ZL3036X reference 0 clock input test */
    ret = timingcard_zl3036x_ref_x_lpbk_lib(ZL3036X_REF_0, sel_freq, TRUE, TRUE);

    /* Configure the ZL3036X reference clock from O2 FPGA sync_trig_out and
     * checks if the DPLL 1 locks the reference clock. */
    if (timingcard_zl3036x_outx_lpbk_dpll_check(ZL3036X_HP_COMS_EN_1,
                                                sel_freq, TRUE) == FAILED) {
        return (FAILED);
    }

    return (ret);
}

/**********************************************************************
 *
 * Function: timingcard_hpoutclk0_clock_lpbk_test
 *
 * Description: This function perform the HPOUTCLK0 clock loopback test.
 *              Check if the ZL3036X locks the loopback clock.
 *              Clock Path:
 *              O2 FPGA SYN_OUT -> CPLD -> ZL3036X REF0 -> DPLL0 -> Synth 0
 *              -> HPOUTCLK0 -> O2 FPGA SYN_OUT1 (clock source select NGVM)
 *              -> ZL3036X REF1 -> DPLL1 (check if locks)
 *
 * Input : None
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
static long timingcard_hpoutclk0_clock_lpbk_test (void)
{
    int ret = PASSED, sel_freq, ix = 0;

    char *tname = "ZL30363 HPOUTCLK0 Clock Loopback";
    char *debug_step1 = "Do the ZL3036X register test; this will clarify if "
                        "the ZL3036X chipset is good.";
    char *debug_step2 = "If step 1 passes, do the CPLD register test; this "
                        "will clarify if the CPLD is good.";
    char *debug_step3 = "If step 1 and step 2 pass, then checks if the clock "
                        "source from platform CPLD SYNC_OUT_CPLD is good.";

    /* Initialize the enhance error code. */
    enhance_error_code(tname, debug_step1, debug_step2, debug_step3);

    /* Initialize the timging card */
    if (timingcard_init_seq() == FAILED) {
        return (FAILED);
    }

    for (ix = ZL3036X_8K_HZ; ix <= ZL3036X_125M_HZ; ix++) {
        if (ix == ZL3036X_8K_HZ) {
            sel_freq = ZL3036X_8K_HZ;
            prpass(testpass, "Testing 8KHz");
        } else if (ix == ZL3036X_25M_HZ) {
            sel_freq = ZL3036X_25M_HZ;
            prpass(testpass, "Testing 25MHz");
        } else {
            sel_freq = ZL3036X_125M_HZ;
            prpass(testpass, "Testing 125MHz");
        }

        /* Configures the O2 dash fpga SYNC/TRIG Control Register
         * to select the clock frequency that outputs to timing card. */
        timingcard_o2_set_sync_trig_out(sel_freq, DASH_FPGA_SYNC_INTER_PLL, FALSE);

        /* Perform the ZL3036X reference 0 clock input test to HPOUTCLK0 */
        if (timingcard_zl3036x_conf_ref_path(ZL3036X_REF_0, ZL3036X_HP_COMS_EN_0,
                                             sel_freq, TRUE, FALSE)
            == FAILED) {
            return (FAILED);
        }

        /* Configures the O2 dash fpga SYNC/TRIG Control Register
         * to select the clock frequency from NGVM timing card. */
        timingcard_o2_set_sync1_out(sel_freq, DASH_FPGA_SEL_NGVM);

        /* Configure the ZL3036X reference clock from O2 FPGA sync1_out and
         * checks if the DPLL 1 locks the reference clock. */
        if (timingcard_zl3036x_outx_lpbk_dpll_check(ZL3036X_HP_COMS_EN_0,
                                                    sel_freq, FALSE) == FAILED) {
            return (FAILED);
        }
    }

    return (ret);
}

#ifdef CANNOT_VERIFY
/**********************************************************************
 *
 * Function: timingcard_hpoutclk2_clock_lpbk_test
 *
 * Description: This function perform the HPOUTCLK2 clock loopback test.
 *              Check if the ZL3036X locks the loopback clock.
 *              Clock Path:
 *              O2 FPGA SYN1_OUT -> CPLD -> ZL3036X REF1 -> DPLL1 -> Synth 1
 *              -> HPOUTCLK2 -> O2 FPGA SYN_OUT1 (clock source select NGVM)
 *              -> ZL3036X REF 0 -> DPLL0 (Check if locks)
 *
 * Input : None
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
static long timingcard_hpoutclk2_clock_lpbk_test (void)
{
    int ret = PASSED, sel_freq, ix = 0;

    char *tname = "ZL30363 HPOUTCLK2 Clock Loopback";
    char *debug_step1 = "Do the ZL3036X register test; this will clarify if "
                        "the ZL3036X chipset is good.";
    char *debug_step2 = "If step 1 passes, do the CPLD register test; this "
                        "will clarify if the CPLD is good.";
    char *debug_step3 = "If step 1 and step 2 pass, then checks if the clock "
                        "source from platform CPLD SYNC_OUT_CPLD is good.";

    /* Initialize the enhance error code. */
    enhance_error_code(tname, debug_step1, debug_step2, debug_step3);

    for (ix = ZL3036X_8K_HZ; ix <= ZL3036X_125M_HZ; ix++) {
        if (ix == ZL3036X_8K_HZ) {
            sel_freq = ZL3036X_8K_HZ;
            prpass(testpass, "Testing 8KHz");
        } else if (ix == ZL3036X_25M_HZ) {
            sel_freq = ZL3036X_25M_HZ;
            prpass(testpass, "Testing 25MHz");
        } else {
            sel_freq = ZL3036X_125M_HZ;
            prpass(testpass, "Testing 25MHz");
        }

        /* Configures the O2 dash fpga SYNC/TRIG Control Register
         * to select the clock frequency from NGVM timing card. */
        timingcard_o2_set_sync1_out(sel_freq, DASH_FPGA_SYNC_INTER_PLL);

        /* Perform the ZL3036X reference 1 clock input test to HPOUTCLK2 */
        if (timingcard_zl3036x_conf_ref_path(ZL3036X_REF_1, ZL3036X_HP_COMS_EN_2,
                                             sel_freq, TRUE, FALSE)
            == FAILED) {
            return (FAILED);
        }

        /* Configures the O2 dash fpga SYNC/TRIG Control Register
         * to select the clock frequency that outputs to timing card. */
        timingcard_o2_set_sync_trig_out(sel_freq, DASH_FPGA_SEL_NGVM);

        /* Configure the ZL3036X reference clock from O2 FPGA sync1_out and
         * checks if the DPLL 1 locks the reference clock. */
        if (timingcard_zl3036x_out0_out2_lpbk_dpll_check(ZL3036X_HP_COMS_EN_2,
                                                         sel_freq) == FAILED) {
            return (FAILED);
        }
    }

    return (ret);
}
#endif

/**********************************************************************
 *
 * Function: timingcard_ref1_clock_input_test
 *
 * Description: This function perform the reference 1 clock input test.
 *              Check if the ZL3036X locks the clock.
 *
 * Input : None
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
static long timingcard_ref1_clock_input_test (void)
{
    int ret = PASSED, sel_freq, ix = 0;

    char *tname = "Timing Card ZL3036X Reference 1 Clock Input";
    char *debug_step1 = "Do the ZL3036X register test; this will clarify if "
                        "the ZL3036X chipset is good.";
    char *debug_step2 = "If step 1 passes, do the CPLD register test; this "
                        "will clarify if the CPLD is good.";
    char *debug_step3 = "If step 1 and step 2 pass, checks if the clock source "
                        "from platform CPLD SYNC_OUT1_CPLD is good.";

    /* Initialize the enhance error code. */
    enhance_error_code(tname, debug_step1, debug_step2, debug_step3);

    /* Initialize the timging card */
    if (timingcard_init_seq() == FAILED) {
        return (FAILED);
    }

    for (ix = ZL3036X_8K_HZ; ix <= ZL3036X_25M_HZ; ix++) {
        if (ix == ZL3036X_8K_HZ) {
            prpass(testpass, "Testing 8KHz");
            sel_freq = ZL3036X_8K_HZ;
        } else {
            prpass(testpass, "Testing 25MHz");
            sel_freq = ZL3036X_25M_HZ;
        }

        /* Configures the O2 dash fpga SYNC/TRIG Control Register
         * to select the clock frequency that outputs to timing card. */
        timingcard_o2_set_sync1_out(sel_freq, DASH_FPGA_SYNC_INTER_PLL);

        /* Perform the ZL3036X reference 0 clock input test */
        ret = timingcard_zl3036x_ref_x_lib(ZL3036X_REF_1, sel_freq);

        /* Restore to the original value. */
        timingcard_o2_disable_sync1_out();
    }

    return (ret);
}

/**********************************************************************
 *
 * Function: timingcard_ref2_clock_input_test
 *
 * Description: This function perform the reference 2 clock input test.
 *              Check if the ZL3036X locks the clock.
 *
 * Input : None
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
static long timingcard_ref2_clock_input_test (void)
{
    uchar data_buf;
    int ret = PASSED, ix = 0;

    char *tname = "Timing Card ZL3036X Reference 2 Clock Input";
    char *debug_step1 = "Do the ZL3036X register test; this will clarify if "
                        "the ZL3036X chipset is good.";
    char *debug_step2 = "If step 1 passes, do the CPLD register test; this "
                        "will clarify if the CPLD is good.";
    char *debug_step3 = "If step 1 and step 2 pass, checks if the clock source "
                        "from platform CPLD SYNC_OUT1_CPLD is good.";

    /* Initialize the enhance error code. */
    enhance_error_code(tname, debug_step1, debug_step2, debug_step3);

    /* Initialize the timging card */
    if (timingcard_init_seq() == FAILED) {
        return (FAILED);
    }

    /* Perform the ZL3036X reference 2 clock input test */
    ret = timingcard_zl3036x_ref_x_lib(ZL3036X_REF_2, ZL3036X_1_HZ);

    /* Enable the trigger out and check the GST, SCM and SCM masks. */
    for (ix = 0; ix < WAIT_MAX_180_SECONDS; ix++) {
        timingcard_o2_sync_trig_out_ctrl(TRUE);
        msleep(500);
        timingcard_o2_sync_trig_out_ctrl(FALSE);
        msleep(500);
        if (ix > WAIT_20_SECONDS) {
            ret = timingzard_zl30361_ref2_check(&data_buf);
            if (ret == PASSED) {
                break;
            }
        }
    }

    if (ret == FAILED) {
        cterr('f', 0, "Ref2 check 1Hz input failed, read ZL3036X_REF_MON_FAIL_2 "
              "is %#.2x" , data_buf);
    }

    return (ret);
}

/**********************************************************************
 *
 * Function: timingcard_hpoutclk4_clock_lpbk_test
 *
 * Description: This function perform the HPOUTCLK4 clock loopback test.
 *              Check if the ZL3036X locks the loopback clock.
 *              Clock Path:
 *              O2 FPGA SYN_OUT1 -> CPLD -> ZL3036X REF1 -> DPLL0 -> Synth 2
 *              -> HPOUTCLK4 -> ZL3036X REF 4 -> DPLL1 (check if locks)
 *
 * Input : None
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
static long timingcard_hpoutclk4_clock_lpbk_test (void)
{
    int ret = PASSED, sel_freq, ix = 0;

    char *tname = "ZL30363 HPOUTCLK4 Clock Loopback Test";
    char *debug_step1 = "Do the ZL3036X register test; this will clarify if "
                        "the ZL3036X chipset is good";
    char *debug_step2 = "If step 1 passes, do the CPLD register test; this "
                        "will clarify if the CPLD is good.";
    char *debug_step3 = "If step 1 and step 2 pass, then checks if the clock "
                        "source from platform CPLD SYNC_OUT_CPLD is good.";

    /* Initialize the enhance error code. */
    enhance_error_code(tname, debug_step1, debug_step2, debug_step3);

    /* Initialize the timging card */
    if (timingcard_init_seq() == FAILED) {
        return (FAILED);
    }

    for (ix = ZL3036X_8K_HZ; ix <= ZL3036X_25M_HZ; ix++) {
        if (ix == ZL3036X_8K_HZ) {
            prpass(testpass, "Testing 8KHz");
            sel_freq = ZL3036X_8K_HZ;
        } else {
            prpass(testpass, "Testing 25MHz");
            sel_freq = ZL3036X_25M_HZ;
        }

        /* Configures the O2 dash fpga SYNC/TRIG Control Register
         * to select the clock frequency that outputs to timing card. */
        timingcard_o2_set_sync1_out(sel_freq, DASH_FPGA_SYNC_INTER_PLL);

        /* Perform the ZL3036X reference 0 clock input test */
        ret = timingcard_zl3036x_ref_x_lpbk_lib(ZL3036X_REF_1, sel_freq, TRUE, FALSE);

        /* Restore to the original value. */
        timingcard_o2_disable_sync_trig_out();
    }

    return (ret);
}

/**********************************************************************
 *
 * Function: timingcard_zl3036x_reg_alter
 *
 * Wrapper for ZL3036X Register write utility.
 *
 * Input : None
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
static long timingcard_zl3036x_reg_alter (void)
{
    /* Initialize the timging card */
    if (timingcard_init_seq() == FAILED) {
        return (FAILED);
    }

    return util_oir_zl3036x_reg_write();
}

/**********************************************************************
 *
 * Function: timingcard_zl3036x_reg_display
 *
 * Wrapper for ZL3036X Register Read utility.
 *
 * Input : None
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
static long timingcard_zl3036x_reg_display (void)
{
    /* Initialize the timging card */
    if (timingcard_init_seq() == FAILED) {
        return (FAILED);
    }

    return util_oir_zl3036x_reg_read();
}

/**********************************************************************
 *
 * Function: sel_zl3036x_ref_clock
 *
 * Description: This function select the ZL3036X reference clock source.
 *
 * Input : None
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
static long sel_zl3036x_ref_clock (void)
{
    /* Initialize the timging card */
    if (timingcard_init_seq() == FAILED) {
        return (FAILED);
    }

    return sel_zl3036x_ref_clock_lib();
}

/**********************************************************************
 *
 * Function: zl3036x_clock_out_to_o2
 *
 * Description: This function select the ZL3036X reference clock source.
 *
 * Input : None
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
static long zl3036x_clock_out_to_o2 (void)
{
    /* Initialize the timging card */
    if (timingcard_init_seq() == FAILED) {
        return (FAILED);
    }

    return clock_verification_path_lib(NGSM_SLOT_ONE);
}

/******** History ********/
/*------------------------------------------------------------------------------
 * $Log: vm_timingcard_zl3036x_diag.c,v $
 * Revision 1.3  2017/07/14 02:51:39  alpeng
 * fixed compiler warning, due to cross-compiler version was updated.
 *
 * Revision 1.2  2015/02/14 12:48:43  kodko
 * Collapse timing card branch code into main trunk.
 *
 * Revision 1.1.2.7  2014/04/25 06:56:34  kodko
 * Support ZL30361 reference 2 clock input test.
 *
 * Revision 1.1.2.6  2014/04/22 06:06:03  kodko
 * Support ZL30361 SKU.
 *
 * Revision 1.1.2.5  2014/03/11 02:52:32  leschen
 * Add 1588 clk/trig test item.
 *
 * Revision 1.1.2.4  2014/03/07 07:44:55  kodko
 * Add check if the DPLL locks the correct reference pin.
 *
 * Revision 1.1.2.3  2014/02/24 09:02:43  kodko
 * Initial bring up for CPLD firmware upgrade by CPLD it-self and IO Exapnder.
 *
 * Revision 1.1.2.2  2014/01/13 10:33:45  kodko
 * Initial bring up for timing card.
 *
 * Revision 1.1.2.1  2013/12/25 09:03:06  kodko
 * Initial check-in for NGVM  Timing Card.
 *
 *------------------------------------------------------------------------------
 * $Endlog $
 *------------------------------------------------------------------------------
 */
