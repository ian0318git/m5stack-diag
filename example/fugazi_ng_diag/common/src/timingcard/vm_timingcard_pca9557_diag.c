/* $Id: vm_timingcard_pca9557_diag.c,v 1.2 2015/02/14 12:48:42 kodko Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/timingcard/vm_timingcard_pca9557_diag.c,v $
 *------------------------------------------------------------------
 * Filename: vm_timingcard_pca9557_diag.c
 *
 * Description: The Timing Card PCA9557 main source code
 * Author: Kody Ko
 *
 * Copyright (c) 2014 - 2015 by cisco Systems, Inc.
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
#include "platform_i2c.h"
#include "i2c_api.h"
#include "common_utils.h"
#include "platform_fru.h"
#include "platform_cookie.h"
#include "vm_timingcard_pca9557_lib.h"

#include <stdio.h>
#include <string.h>

/***********************************************************************
 *  Macro Definitions
 ************************************************************************/

/***********************************************************************
 *  Static Functions Declaration
 ************************************************************************/
static long timingcard_pca9557_reg_test(void);
static long timingcard_pca9557_reg_alter(void);
static long timingcard_pca9557_reg_dump(void);

/***********************************************************************
 *  Functions Declaration
 ************************************************************************/
long build_timingcard_pca9557_menu(int);
long timingcard_pca9557_utility_submenu(int);

/***********************************************************************
 * Extern function prototypes
 ***********************************************************************/
extern int do_all_menu_items(struct menuinfo *);

/***********************************************************************
 *  Global Variable
 ************************************************************************/
static submenu_xtable_t timingcard_pca9557_submenu_tbl[] = {
    { "PCA9557 utility", (type_t(*)())timingcard_pca9557_utility_submenu, 0,
      0, (type_t(*)())0, 0, (type_t(*)())0, 0 },
    { "PCA9557 Register Test", (type_t(*)())timingcard_pca9557_reg_test, 0,
     MF_CONTINUOUS | MF_DOALL, (type_t(*)())0, 0, (type_t(*)())0, 0 },
};

#define TIMINGCARD_PCA9557_SUBMENU_TABLE_SZ \
                (sizeof(timingcard_pca9557_submenu_tbl)/sizeof(submenu_xtable_t))

/***********************************************************************
 * Primary & secondary submenu items (filled in from xtable)
 ************************************************************************/
static mitem_t timingcard_pca9557_primary_items[TIMINGCARD_PCA9557_SUBMENU_TABLE_SZ +
                                                MAX_BASE_ITEMS];
static mitem_t timingcard_pca9557_secondary_items[TIMINGCARD_PCA9557_SUBMENU_TABLE_SZ +
                                                  MAX_BASE_ITEMS];

static menuinfo_t timingcard_pca9557_main_menu = {
    "Timing Card PCA9557 Menu",
    0,                        /* mtparam added by init_empty_menu */
    0,                        /* notes missing WICs in combos */
    0,                        /* use generic prompt */
    0,                        /* size (bumped by add_menu_item() */
    timingcard_pca9557_primary_items,
};
static menuinfo_t *timingcard_pca9557_menup = &timingcard_pca9557_main_menu;

/***********************************************************************
 * Timing Card utilities menu on Overlord platform
 ************************************************************************/
static mitem_t timingcard_pca9557_util_submenu_table[] = {
    { "Alter PCA9557 Register", 0, 0, timingcard_pca9557_reg_alter,
      (long *)&zero, 0, (type_t(*)())0, 0 },
    { "Dump PCA9557 Registers", 0, 0, timingcard_pca9557_reg_dump,
      (long *)&zero, 0, (type_t(*)())0, 0 },
};

#define TIMINGCARD_PCA9557_UTIL_SUBMENU_TABLE_SZ \
        (sizeof(timingcard_pca9557_util_submenu_table)/sizeof(mitem_t))

static menuinfo_t timingcard_pca9557_util_subtest_menu = {
    "Timing Card PCA9557 Utilities Menu",
    0,                                  /* title param */
    0,                                  /* show diag flags */
    0,
    TIMINGCARD_PCA9557_UTIL_SUBMENU_TABLE_SZ,
    timingcard_pca9557_util_submenu_table,
};

static menuinfo_t *timingcard_pca9557_util_submenup = &timingcard_pca9557_util_subtest_menu;


/***********************************************************************
 *  Functions
 ************************************************************************/

/**********************************************************************
 *
 * Function: build_timingcard_pca9557_menu
 *
 * Description: Build Timing Card PCA9557 tests and utilities menu.
 *
 * Inputs:  show_menu - FALSE for tests. TRUE for submenu.
 *
 * Outputs: PASSED/FAILED.
 *
 **********************************************************************
 */
long build_timingcard_pca9557_menu (int show_menu)
{

    build_primary_submenu(timingcard_pca9557_submenu_tbl, TIMINGCARD_PCA9557_SUBMENU_TABLE_SZ,
                          "PCA9557 Main Menu", &timingcard_pca9557_menup);
    build_secondary_submenu(timingcard_pca9557_submenu_tbl, TIMINGCARD_PCA9557_SUBMENU_TABLE_SZ,
                            timingcard_pca9557_secondary_items);

    if (show_menu) {
        /* Entered with submenu */
        menu(timingcard_pca9557_menup, timingcard_pca9557_secondary_items, 0);
    } else {
        /* Invoked the test from main menu */
        do_all_menu_items(timingcard_pca9557_menup);
    }

    return(PASSED);
}

/**********************************************************************
 *
 * Function: timingcard_pca9557_utility_submenu
 *
 * This function implements the Timing Card CPLD test/menu
 *
 * Input: menu_option - show menu option
 *
 * Output: PASSED/FAILED.
 *
 **********************************************************************
 */
long timingcard_pca9557_utility_submenu (int menu_option)
{
    menu(timingcard_pca9557_util_submenup, timingcard_pca9557_util_submenu_table, '\0');

    return (PASSED);
}

/**********************************************************************
 *
 * Function: timingcard_pca9557_reg_test
 *
 * Wrapper for PCA9557 Register test.
 *
 * Input : none
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
static long timingcard_pca9557_reg_test (void)
{
    char *tname = "Timing Card PCA9557 Register";
    char *debug_step1 = "Do the ZL3036X register test; this will clarify if"
                        " the path from Overlord FPGA to timing card is good. "
                        "If it passes then the path from Overlord Intel "
                        "processor to ZL3036X is good.";
    char *debug_step2 = "If step 1 is passed, try to replace PCA9557.";
    char *debug_step3 = "None.";

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
    cterr_add_component("PCA9557");

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

    if (timingcard_pca9557_reg_test_lib() == FAILED) {
        return (FAILED);
    }

    return (PASSED);
}

/**********************************************************************
 *
 * Function: timingcard_pca9557_reg_alter
 *
 * Wrapper for PCA9557 Register write utility.
 *
 * Input : None
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
static long timingcard_pca9557_reg_alter (void)
{
    /* Initialize the PCA9557. */
    if (timingcard_pca9557_init() == FAILED) {
        return (FAILED);
    }

    return util_oir_pca9557_reg_write();
}

/**********************************************************************
 *
 * Function: timingcard_pca9557_reg_dump
 *
 * Wrapper for PCA9557 Register Read utility.
 *
 * Input : None
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
static long timingcard_pca9557_reg_dump (void)
{
    return util_oir_pca9557_reg_read();
}

/******** History ********/
/*------------------------------------------------------------------------------
 * $Log: vm_timingcard_pca9557_diag.c,v $
 * Revision 1.2  2015/02/14 12:48:42  kodko
 * Collapse timing card branch code into main trunk.
 *
 * Revision 1.1.2.1  2014/02/24 09:02:44  kodko
 * Initial bring up for CPLD firmware upgrade by CPLD it-self and IO Exapnder.
 *
 *------------------------------------------------------------------------------
 * $Endlog $
 *------------------------------------------------------------------------------
 */
