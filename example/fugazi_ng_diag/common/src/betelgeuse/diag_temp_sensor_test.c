/* $Id: diag_temp_sensor_test.c,v 1.2 2019/01/10 06:36:24 wilbhuan Exp $ 
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/betelgeuse/diag_temp_sensor_test.c,v $
 *------------------------------------------------------------------
 * 
 * diag_temp_sensor_test.c
 *
 * Copyright (c) 2018 - 2019 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------------------
 */

#include <string.h>
#include "common.h"
#include "proto.h"
#include "error.h"
#include "platform_i2c.h"
#include "menu.h"
#include "diag_i2c_lib.h"
#include "nvmonvars.h"
#include "diag_moka_fpga_lib.h"
#include "diag_enhance_err_msg_lib.h"
#include "platform_cookie.h"
#include "i2c_api.h"
#include "dev_object.h"
#include "dev_maxim_max31730.h"
#include "diag_temp_sensor_lib.h"
#include "diag_temp_sensor_test.h"
#include "diag_temp_sensor_util.h"

/*******************************************************************************
 *                            Global Variables
 *******************************************************************************
 */
/*
 * Temperature Sensor Diag Menu
 */
static submenu_xtable_t ts_diag_table[] = {
    {"Temp. sensor utilities",      (PFT)diag_temp_sensor_util, 0,
     0, 
     (type_t(*)())0,                0,
     (type_t(*)())0,                0},
    {"Temp. sensor Register Test",  (PFT)diag_temp_sensor_reg_test,  0,
     (MF_CONTINUOUS | MF_DOALL | MF_SHOW_ERRCOUNT),
     (type_t(*)())0,                0,
     (type_t(*)())0,                0},
    {"Temp. sensor Interrupt Test", (PFT)diag_temp_sensor_intr_test, 0,
     (MF_CONTINUOUS | MF_DOALL | MF_SHOW_ERRCOUNT),
     (type_t(*)())0,                0,
     (type_t(*)())0,                0},
};

#define TS_DIAG_TABLE_SIZE (sizeof(ts_diag_table)/sizeof(submenu_xtable_t))

/*
 * Primary & secondary submenu items (filled in from xtable)
 */
static mitem_t ts_diag_menu_pri_items[TS_DIAG_TABLE_SIZE + MAX_BASE_ITEMS];
static mitem_t ts_diag_menu_sec_items[TS_DIAG_TABLE_SIZE + MAX_BASE_ITEMS];

static struct menuinfo ts_diag_menu = {
    "Temperature sensor Diag Menu",   /* title */
    0,                                /* title string */
    (PFT) menu_show_dflags,           /* shows major flags */
    0,                                /* generic prompt */
    0,                                /* size */
    ts_diag_menu_pri_items,
};

static struct menuinfo *ts_diag_menu_p = &ts_diag_menu;


/*******************************************************************************
 *
 * Function   : diag_temp_sensor_test
 * Description: Entry function of Temperature Sensor(MAX31730) Diag.
 * Inputs     : show_testmenu - To decide whether to show test menu(TRUE/FALSE)
 *                              or do all related tests directly
 * Outputs    : None
 *
 *******************************************************************************
 */
void diag_temp_sensor_test (boolean show_testmenu)
{
    build_primary_submenu(ts_diag_table, TS_DIAG_TABLE_SIZE,
                          "Temperature sensor Diag Menu", &ts_diag_menu_p);
    build_secondary_submenu(ts_diag_table, TS_DIAG_TABLE_SIZE,
                            ts_diag_menu_sec_items);

    if (show_testmenu == TRUE) {
        menu(&ts_diag_menu, ts_diag_menu_sec_items, 0);
    } else {
        do_all_menu_items(ts_diag_menu_p);
    }
}

/*******************************************************************************
 *
 * Function   : diag_temp_sensor_reg_test
 * Description: Function to test I2C interface between Host and
 *              Temperature sensor, Maxim max31730.
 *              It's done by accessing Temperature sensor's register(s).
 * Inputs     : None
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
int diag_temp_sensor_reg_test (void)
{
    uchar mb_get_loc[FRU_SIZE] = {0};
    uchar mb_get_pid[FRU_SIZE] = {0};
#ifdef ENHANCE_ERROR_MSG_RDY
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
    cterr_add_component("Marvell Armada 7040", "I2C", "MB Thermal");

    /* Segment 5: register and memory dump */
    //cterr_add_reg_dump((PFV)bcm63168_get_xdsl_profile);

    /* Segment 6: Platform Environment initialized here*/
    cterr_add_env_dump((PFV)show_plat_curr_temps);

    /* Segment 7: Top 3 Debugging Steps */
    cterr_add_debug("Check the interface between the Host "
                    "SoC and the failed I2C devices.",
                    "If there is no problem for these interfaces, "
                    "replace one I2C device and redo the test.");
#endif

    dev_max31730_object_t ts_obj;
    dev_max31730_object_t *ts_obj_p = &ts_obj;
    dev_object_t *dev = (dev_object_t *)ts_obj_p;
    n2g_i2c_if_t ts_i2c_if;
    int ret_val = FAILED;
    char *curr_testname = "Temperature sensor Register";

    testname(curr_testname);
    prpass(testpass, "%s, ", curr_testname);

    /* Create Temperature sensor device object for test */
    if (diag_ts_dev_create(ts_obj_p, &ts_i2c_if) != PASSED) {
        cterr('f', 0, "Failed to create Temperature sensor, Maxim max31730,"
                      " device object");
        return (FAILED);
    }

    /* Execute Temperature sensor Register test */
    ret_val = ts_obj_p->callin_fvt->register_test(dev);
    if (ret_val != PASSED) {
        cterr('f', 0, "Registers test failed");
    }

    /* Destroy the created RTC device object after test */
    ts_obj_p->base.dev_object_fvt->dev_destroy((dev_object_t **)&ts_obj_p);

    return (ret_val);
}

/*******************************************************************************
 *
 * Function   : diag_temp_sensor_intr_test
 * Description: Function to test the interrupt functionality
 *              between Host and Temperature sensor, Maxim max31730.
 * Inputs     : None
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
int diag_temp_sensor_intr_test (void)
{
    uchar mb_get_loc[FRU_SIZE] = {0};
    uchar mb_get_pid[FRU_SIZE] = {0};
#ifdef ENHANCE_ERROR_MSG_RDY
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
    cterr_add_component("Marvell Armada 7040", "I2C", "MB Thermal");

    /* Segment 5: register and memory dump */
    //cterr_add_reg_dump((PFV)bcm63168_get_xdsl_profile);

    /* Segment 6: Platform Environment initialized here*/
    cterr_add_env_dump((PFV)show_plat_curr_temps);

    /* Segment 7: Top 3 Debugging Steps */
    cterr_add_debug("Check the interface between the Host "
                    "SoC and the failed I2C devices.",
                    "If there is no problem for these interfaces, "
                    "replace one I2C device and redo the test.");
#endif

    dev_max31730_object_t ts_obj;
    dev_max31730_object_t *ts_obj_p = &ts_obj;
    dev_object_t *dev = (dev_object_t *)ts_obj_p;
    n2g_i2c_if_t ts_i2c_if;
    int ret_val = FAILED;
    char *curr_testname = "Temperature sensor Interrupt";

    testname(curr_testname);
    prpass(testpass, "%s, ", curr_testname);

    /* Create Temperature sensor device object for test */
    if (diag_ts_dev_create(ts_obj_p, &ts_i2c_if) != PASSED) {
        cterr('f', 0, "Failed to create Temperature sensor, Maxim max31730,"
                      " device object");
        return (FAILED);
    }

    /* Execute Temperature sensor Register test */
    ret_val = ts_obj_p->callin_fvt->interrupt_test(dev, MAX31730_REMOTE2);
    if (ret_val != PASSED) {
        cterr('f', 0, "Interrupt test failed");
    }

    /* Destroy the created RTC device object after test */
    ts_obj_p->base.dev_object_fvt->dev_destroy((dev_object_t **)&ts_obj_p);

    return (ret_val);
}

/*-------------------------------------------------
 * $Log: diag_temp_sensor_test.c,v $
 * Revision 1.2  2019/01/10 06:36:24  wilbhuan
 * The beginning of Betelgeuse application code.
 *
 *-------------------------------------------------
 */
