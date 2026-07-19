/* $Id: diag_rtc_test.c,v 1.2 2021/09/24 01:21:07 harrchan Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/elixir/diag_rtc_test.c,v $
 *------------------------------------------------------------------
 * 
 * diag_rtc_test.c
 *
 * Copyright (c) 2018 - 2019 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#include <string.h>
#include "common.h"
#include "types.h"
#include "menu.h"
#include "error.h"
#include "dev_1337.h"
#include "diag_rtc_lib.h"
#include "diag_rtc_test.h"
#include "diag_rtc_util.h"
#include "diag_enhance_err_msg_lib.h"
#include "platform_cookie.h"
#include "diag_temp_sensor_util.h"

/*******************************************************************************
 *                            Global Variables
 *******************************************************************************
 */
/*
 * RTC main menu
 */
static submenu_xtable_t rtc_test_tbl[] = {
    {"RTC utilities",      (PFT)build_rtc_utils_menu,   0,
     0, 
     (type_t(*)())0,       0,
     (type_t(*)())0,       0},
    {"RTC Init",           (PFT)diag_rtc_init_test,          0,
     (MF_CONTINUOUS | MF_DOALL | MF_SHOW_ERRCOUNT),
     (type_t(*)())0,       0,
     (type_t(*)())0,       0},
    {"Register Test",      (PFT)diag_rtc_reg_test,           0,
     (MF_CONTINUOUS | MF_DOALL | MF_SHOW_ERRCOUNT),
     (type_t(*)())0,       0,
     (type_t(*)())0,       0},
    {"Time Validity Test", (PFT)diag_rtc_time_validity_test, 0,
     (MF_CONTINUOUS | MF_DOALL | MF_SHOW_ERRCOUNT),
     (type_t(*)())0,       0,
     (type_t(*)())0,       0},
};

#define RTC_TEST_TBL_SIZE (sizeof(rtc_test_tbl) / sizeof(submenu_xtable_t))

/*
 * Primary & secondary submenu items (filled in from xtable)
 */
static mitem_t rtc_test_pri_items[RTC_TEST_TBL_SIZE + MAX_BASE_ITEMS];
static mitem_t rtc_test_sec_items[RTC_TEST_TBL_SIZE + MAX_BASE_ITEMS];


static struct menuinfo rtc_diagmenu = {
  "RTC Test Main Menu",      /* title */
  0,                         /* title string added by init_empty_menu */
  (PFT)menu_show_dflags,     /* shows major flags */
  0,                         /* generic prompt */
  0,                         /* size -- bumped by add_menu_item() */
  rtc_test_pri_items,
};

static struct menuinfo *rtc_diagmenu_p = &rtc_diagmenu;


/*******************************************************************************
 *
 * Function   : diag_rtc_test
 * Description: Entry of RTC, Maxim ds1337, diag tests/utilities.
 * Inputs     : show_testmenu - To decide whether to show test menu(TRUE/FALSE)
 *                              or do all related tests directly
 * Outputs    : None
 *
 *******************************************************************************
 */
void diag_rtc_test (boolean show_testmenu)
{
    build_primary_submenu(rtc_test_tbl, RTC_TEST_TBL_SIZE,
                          "RTC Tests Main Menu", &rtc_diagmenu_p);
    build_secondary_submenu(rtc_test_tbl, RTC_TEST_TBL_SIZE,
                            rtc_test_sec_items);

    if (show_testmenu == TRUE) {
        menu(rtc_diagmenu_p, rtc_test_sec_items, '\0');
    } else {
        do_all_menu_items(rtc_diagmenu_p);
    }
}

/*******************************************************************************
 *
 * Function   : diag_rtc_init_test
 * Description: Function to do initilize RTC, Maxim ds1337, test.
 * Inputs     : None
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
int diag_rtc_init_test (void)
{
    uchar mb_get_loc[FRU_SIZE] = {0};
    uchar mb_get_pid[FRU_SIZE] = {0};

    /*
     * 1. Subtests of the test function will reuse all variables
     * 2. All variables will be cleared automatically when
     *    entering and leaving each menu item.
     */
    /* Segment 1: PID | Unique_string : slot_info */
    fru_table_offset = MB;
    /* fru_table_offset should be set, otherwise, it will not */
    /* go to enhanced error message format in cterr() */
    /* set fru_table_offset to get the predefine value */
    /* or change mb_pid & mb_loc below */
    platform_get_pid((char *)mb_get_pid);
    strcpy((char *)mb_get_loc, "MB");
    platform_fru_table[fru_table_offset].pid_string = mb_get_pid;
    platform_fru_table[fru_table_offset].location_string = mb_get_loc;

    /* Segment 2: Test step captured from prpass */
    /* Segment 3: Failure message captured from cterr */

    /* Segment 4: Components used */
    cterr_add_component("Marvell Armada 7040", "I2C", "RTC");

    /* Segment 5: register and memory dump */
    //cterr_add_reg_dump((PFV)bcm63168_get_xdsl_profile);

    /* Segment 6: Platform Environment initialized here*/
    cterr_add_env_dump((PFV)show_plat_curr_temps);

    /* Segment 7: Top 3 Debugging Steps */
    cterr_add_debug("Do the \"I2C scan test\" to see if "
                    "we can recognize the RTC device.",
                    "If step a is OK, consult with HW to "
                    "verify the I2C interface functionality.");

    dev_ds1337_object_t rtc_obj;
    dev_ds1337_object_t *rtc_obj_p = &rtc_obj;
    dev_object_t *dev = (dev_object_t *)rtc_obj_p;
    n2g_i2c_if_t rtc_i2c_if;
    int ret_val = FAILED;
    char *curr_testname = "RTC Init";

    testname(curr_testname);
    prpass(testpass, "%s, ", curr_testname);

    /* Create RTC device object for test */
    if (diag_rtc_dev_create(rtc_obj_p, &rtc_i2c_if) != PASSED) {
        cterr('f', 0, "Failed to create RTC, Maxim ds1337, device object");
        return (FAILED);
    }

    /* Based on DS1337 datasheet, a logic 1 in OSF bit indicates that the
     * oscillator either is stopped or was stopped for some period of time
     * and may be used to judge the validity of the clock and calendar data.
     * This bit is set to logic 1 anytime that the oscillator stops.
     *
     * The following are examples that can cause the OSF bit be set:
     * 1) The first time power is applied.
     * 2) The voltage present on Vcc is insufficient to support oscillation.
     * 3) The EOSC bit is turned off.
     * 4) External influences on the crystal (e.g., noise, leakage, etc.).
     *
     * OSF bit remains at logic 1 until written to logic 0.
     */
    /* CSCvn27900: Diag app. v0.2.0 failed at RTC init test when
     *             the first time power is applied on unit
     *
     * Added below function to clear the OSF bit that was set by the first time
     * power is applied before doing RTC init test.
     */
    if (clear_rtc_osf_bit() != PASSED) {
        cterr('f', 0, "Failed to clear OSF bit of RTC status reg.(0xF, bit7)");
        goto RTC_INIT_DESTROY;
    }

    /* Execute RTC init test */
    ret_val = rtc_obj_p->base.dev_object_fvt->dev_init(dev);
    if (ret_val != PASSED) {
        cterr('f', 0, "Failed to init RTC");
    }

RTC_INIT_DESTROY:
    /* Destroy the created RTC device object after test */
    rtc_obj_p->base.dev_object_fvt->dev_destroy((dev_object_t **)&rtc_obj_p);

    return (ret_val);
}

/*******************************************************************************
 *
 * Function   : diag_rtc_reg_test
 * Description: Function to test the register read/write functionality of RTC,
 *              Maxim ds1337, chip and its I2C interface.
 * Inputs     : None
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
int diag_rtc_reg_test (void)
{
    uchar mb_get_loc[FRU_SIZE] = {0};
    uchar mb_get_pid[FRU_SIZE] = {0};

    /*
     * 1. Subtests of the test function will reuse all variables
     * 2. All variables will be cleared automatically when
     *    entering and leaving each menu item.
     */
    /* Segment 1: PID | Unique_string : slot_info */
    fru_table_offset = MB;
    /* fru_table_offset should be set, otherwise, it will not */
    /* go to enhanced error message format in cterr() */
    /* set fru_table_offset to get the predefine value */
    /* or change mb_pid & mb_loc below */
    platform_get_pid((char *)mb_get_pid);
    strcpy((char *)mb_get_loc, "MB");
    platform_fru_table[fru_table_offset].pid_string = mb_get_pid;
    platform_fru_table[fru_table_offset].location_string = mb_get_loc;

    /* Segment 2: Test step captured from prpass */
    /* Segment 3: Failure message captured from cterr */

    /* Segment 4: Components used */
    cterr_add_component("Marvell Armada 7040", "I2C", "RTC");

    /* Segment 5: register and memory dump */
    //cterr_add_reg_dump((PFV)bcm63168_get_xdsl_profile);

    /* Segment 6: Platform Environment initialized here*/
    cterr_add_env_dump((PFV)show_plat_curr_temps);

    /* Segment 7: Top 3 Debugging Steps */
    cterr_add_debug("Do the \"I2C scan test\" to see if "
                    "we can recognize the RTC device.",
                    "If step a is OK, consult with HW to "
                    "verify the I2C interface functionality.");

    dev_ds1337_object_t rtc_obj;
    dev_ds1337_object_t *rtc_obj_p = &rtc_obj;
    dev_object_t *dev = (dev_object_t *)rtc_obj_p;
    n2g_i2c_if_t rtc_i2c_if;
    int ret_val = FAILED;
    char *curr_testname = "RTC Register";

    testname(curr_testname);
    prpass(testpass, "%s, ", curr_testname);

    /* Create RTC device object for test */
    if (diag_rtc_dev_create(rtc_obj_p, &rtc_i2c_if) != PASSED) {
        cterr('f', 0, "Failed to create RTC, Maxim ds1337, device object");
        return (FAILED);
    }

    /* Execute RTC register test */
    ret_val = rtc_obj_p->callin_fvt->register_test(dev);
    if (ret_val != PASSED) {
        cterr('f', 0, "Registers test failed");
    }

    /* Destroy the created RTC device object after test */
    rtc_obj_p->base.dev_object_fvt->dev_destroy((dev_object_t **)&rtc_obj_p);

    return (ret_val);
}

/*******************************************************************************
 *
 * Function   : diag_rtc_time_validity_test
 * Description: Function to test the time validity of RTC, Maxim ds1337, chip.
 * Inputs     : None
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
int diag_rtc_time_validity_test (void)
{
    uchar mb_get_loc[FRU_SIZE] = {0};
    uchar mb_get_pid[FRU_SIZE] = {0};

    /*
     * 1. Subtests of the test function will reuse all variables
     * 2. All variables will be cleared automatically when
     *    entering and leaving each menu item.
     */
    /* Segment 1: PID | Unique_string : slot_info */
    fru_table_offset = MB;
    /* fru_table_offset should be set, otherwise, it will not */
    /* go to enhanced error message format in cterr() */
    /* set fru_table_offset to get the predefine value */
    /* or change mb_pid & mb_loc below */
    platform_get_pid((char *)mb_get_pid);
    strcpy((char *)mb_get_loc, "MB");
    platform_fru_table[fru_table_offset].pid_string = mb_get_pid;
    platform_fru_table[fru_table_offset].location_string = mb_get_loc;

    /* Segment 2: Test step captured from prpass */
    /* Segment 3: Failure message captured from cterr */

    /* Segment 4: Components used */
    cterr_add_component("Marvell Armada 7040", "I2C", "RTC");

    /* Segment 5: register and memory dump */
    //cterr_add_reg_dump((PFV)bcm63168_get_xdsl_profile);

    /* Segment 6: Platform Environment initialized here*/
    cterr_add_env_dump((PFV)show_plat_curr_temps);

    /* Segment 7: Top 3 Debugging Steps */
    cterr_add_debug("Do the \"I2C scan test\" to see if "
                    "we can recognize the RTC device.",
                    "If step a is OK, consult with HW to "
                    "verify the I2C interface functionality.");

    dev_ds1337_object_t rtc_obj;
    dev_ds1337_object_t *rtc_obj_p = &rtc_obj;
    dev_object_t *dev = (dev_object_t *)rtc_obj_p;
    n2g_i2c_if_t rtc_i2c_if;
    int ret_val = FAILED;
    char *curr_testname = "RTC Time Validity";

    testname(curr_testname);
    prpass(testpass, "%s, ", curr_testname);

    /* Create RTC device object for test */
    if (diag_rtc_dev_create(rtc_obj_p, &rtc_i2c_if) != PASSED) {
        cterr('f', 0, "Failed to create RTC, Maxim ds1337, device object");
        return (FAILED);
    }

    /* Execute RTC time validity test */
    ret_val = rtc_obj_p->callin_fvt->time_validity_test(dev);
    if (ret_val != PASSED) {
        cterr('f', 0, "Registers test failed");
    }

    /* Destroy the created RTC device object after test */
    rtc_obj_p->base.dev_object_fvt->dev_destroy((dev_object_t **)&rtc_obj_p);

    return (ret_val);
}

/*-------------------------------------------------
 * $Log: diag_rtc_test.c,v $
 * Revision 1.2  2021/09/24 01:21:07  harrchan
 * Collapse Elixir-branch to Main Trunk.
 *
 * Revision 1.1.2.1  2020/09/09 09:08:07  illiu
 * First version which has been ported with Dreamliner and Marvel CPSS
 *
 * Revision 1.2  2019/01/10 06:36:24  wilbhuan
 * The beginning of Betelgeuse application code.
 *
 *-------------------------------------------------
 */
