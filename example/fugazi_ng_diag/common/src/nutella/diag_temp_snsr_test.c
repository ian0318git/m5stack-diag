/* $Id: diag_temp_snsr_test.c,v 1.4 2019/07/11 12:31:30 alicehua Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/nutella/diag_temp_snsr_test.c,v $
 *------------------------------------------------------------------
 * Filename:  diag_temp_snsr_test.c
 *
 * Description: Nutella NXP LM75BD Sensor I2C device.
 *
 * Copyright (c) 2016 ~ 2019 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */
#include <string.h>
#include "common.h"
#include "proto.h"
#include "error.h"
#include "menu.h"
#include "nvmonvars.h"
#include "platform_fru.h"
#include "i2c_api.h"
#include "dev_object.h"
#include "diag_i2c_addr.h"
#include "plat_defs.h"
#include "common_utils.h"
#include "mb_tests.h"
#include "diag_i2c_lib.h"
#include "diag_fpga.h"
#include "platform_cookie.h"
#include "dnv_gpio_lib.h"
#include "diag_temp_snsr_test.h"
#include "diag_temp_snsr_lib.h"

int diag_temp_sensor_reg_test(void);
int diag_temp_sensor_show_reg(void);
int diag_temp_sensor_alter_reg(void);
int diag_temp_sensor_dump_reg(void);
int diag_temp_sensor_show_temp(void);
int diag_temp_sensor_int_test (void);

/*******************************************************************************
 *                                   Menus                                     *
 *******************************************************************************
 */
/* 
 * Sub Menu used for MB Temperature Sensor tests
 */
submenu_xtable_t diag_ts_submenu_table[] = {
    {"Temperature sensor reg. read Util",    (PFT)diag_temp_sensor_show_reg,    0,
     MF_CONTINUOUS,
     (type_t(*)())0,            0,
     (type_t(*)())0,            0},
    {"Temperature sensor reg. write Util",   (PFT)diag_temp_sensor_alter_reg,   0,
     MF_CONTINUOUS,
     (type_t(*)())0,            0,
     (type_t(*)())0,            0},
    {"Temperature sensor reg. dump Util",    (PFT)diag_temp_sensor_dump_reg,    0,
     MF_CONTINUOUS,
     (type_t(*)())0,            0,
     (type_t(*)())0,            0},
    {"Temperature sensor show temp. Util",   (PFT)diag_temp_sensor_show_temp,   0,
     MF_CONTINUOUS,
     (type_t(*)())0,            0,
     (type_t(*)())0,            0},
    {"Temperature Sensor Reg. Test", (PFT)diag_temp_sensor_reg_test,   0,
     (MF_CONTINUOUS | MF_DOGRP | MF_DOALL | MF_SHOW_ERRCOUNT),
     (type_t(*)())0,            0,
     (type_t(*)())0,            0},
     {"Temperature Sensor Interrupt Test", (PFT)diag_temp_sensor_int_test,   0,
     (MF_CONTINUOUS | MF_DOGRP | MF_DOALL | MF_SHOW_ERRCOUNT),
     (type_t(*)())0,            0,
     (type_t(*)())0,            0},
};

#define MB_TS_SUBMENU_TABLE_SIZE (sizeof(diag_ts_submenu_table) / \
                                    sizeof(submenu_xtable_t))

static mitem_t diag_ts_pri_items[MB_TS_SUBMENU_TABLE_SIZE + MAX_BASE_ITEMS];
static mitem_t diag_ts_sec_items[MB_TS_SUBMENU_TABLE_SIZE + MAX_BASE_ITEMS];

static struct menuinfo diag_ts_submenu = {
    "%s Menu",               /* title */
    0,                       /* title string added by init_empty_menu */
    0,                       /* do not show major flags */
    0,                       /* generic prompt */
    0,                       /* size -- bumped by add_menu_item() */
    diag_ts_pri_items,
};

static struct menuinfo *diag_ts_submenup = &diag_ts_submenu;

/*******************************************************************************
 *
 * Function   : diag_temp_sensor_test
 * Description: Entry function of MB Temperature Sensor test
 * Inputs     : Test/Menu
 * Outputs    : PASSED
 *
 *******************************************************************************
 */
int build_snsr_menu (boolean diag_ts_items_executed)
{
    char *tname = "Temperature Sensor";

    testname(tname);

    build_primary_submenu(diag_ts_submenu_table,
                          MB_TS_SUBMENU_TABLE_SIZE, "Temperature Sensor",
                          &diag_ts_submenup);

    build_secondary_submenu(diag_ts_submenu_table,
                            MB_TS_SUBMENU_TABLE_SIZE,
                            diag_ts_sec_items);

    if (diag_ts_items_executed) {
        menu(&diag_ts_submenu, diag_ts_sec_items, 0);
    } else {
        do_all_menu_items(diag_ts_submenup);
    }

    return (PASSED);
}

/*******************************************************************************
 *
 * Function    : diag_temp_sensor_reg_test
 * Description : Function to execute MB Temperature Sensor Register Test
 * Inputs      : None
 * Outputs     : PASSED / FAILED
 *
 *******************************************************************************
 */
int diag_temp_sensor_reg_test (void)
{
    dev_lm75b_object_t ts_data;
    dev_lm75b_object_t *ts_obj = &ts_data;
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
    cterr_add_component("Intel Denverton SOC C3558", "I2C", 
                        "Thernal sensor NXP LM75B");

    /* Segment 5: register and memory dump */

    /* Segment 6: Platform Environment initialized here*/

    /* Segment 7: Top 3 Debugging Steps */
    cterr_add_debug("Check the interface between the Host SoC"
                    " and the failed I2C devices.", "If there"
                    " is no problem for these interfaces,"
                    " replace one I2C device and redo the test.");

    testname("Temperature Sensor Register");

    diag_ts_dev_create(ts_obj);

    if (ts_obj == NULL) {
        cterr('f', 0, "%s: Null Object\n", __func__);
        return (FAILED);
    }

    if (ts_obj->callin_fvt->register_test((dev_object_t *)ts_obj) != PASSED) {
        cterr('f', 0, "%s: Failed to do registers R/W ", __func__);
        return (FAILED);
    }

    ts_obj->base.dev_object_fvt->dev_destroy((dev_object_t **)&ts_obj);

    return (PASSED);
}


/*******************************************************************************
 *
 * Function    : diag_temp_sensor_show_reg
 * Description : Function to display MB module temperature sensor Register
 * Inputs      : None
 * Outputs     : PASSED / FAILED
 *
 *******************************************************************************
 */
int diag_temp_sensor_show_reg (void)
{
    dev_lm75b_object_t ts_data;
    dev_lm75b_object_t *ts_obj = &ts_data;
    int                ret_val = FAILED;

    diag_ts_dev_create(ts_obj);

    if (ts_obj == NULL) {
        printf("%s: Null Object\n", __func__);
        return (FAILED);
    }

    ret_val = ts_obj->callin_fvt->show_register((dev_object_t *)ts_obj);

    ts_obj->base.dev_object_fvt->dev_destroy((dev_object_t **)&ts_obj);

    return (ret_val);
}

/*******************************************************************************
 *
 * Function    : diag_temp_sensor_alter_reg
 * Description : Function to alter MB module temperature sensor Register
 * Inputs      : None
 * Outputs     : PASSED / FAILED
 *
 *******************************************************************************
 */
int diag_temp_sensor_alter_reg (void)
{
    dev_lm75b_object_t ts_data;
    dev_lm75b_object_t *ts_obj = &ts_data;
    int                ret_val = FAILED;

    diag_ts_dev_create(ts_obj);

    if (ts_obj == NULL) {
        printf("%s: Null Object\n", __func__);
        return (FAILED);
    }

    ret_val = ts_obj->callin_fvt->alter_register((dev_object_t *)ts_obj);

    ts_obj->base.dev_object_fvt->dev_destroy((dev_object_t **)&ts_obj);

    return (ret_val);
}

/*******************************************************************************
 *
 * Function    : diag_temp_sensor_dump_reg
 * Description : Function to dump MB module temperature sensor Registers
 * Inputs      : None
 * Outputs     : PASSED / FAILED
 *
 *******************************************************************************
 */
int diag_temp_sensor_dump_reg (void)
{
    dev_lm75b_object_t ts_data;
    dev_lm75b_object_t *ts_obj = &ts_data;
    int                ret_val = FAILED;

    diag_ts_dev_create(ts_obj);

    if (ts_obj == NULL) {
        printf("%s: Null Object\n", __func__);
        return (FAILED);
    }

    ret_val = ts_obj->callin_fvt->dump_register((dev_object_t *)ts_obj);

    ts_obj->base.dev_object_fvt->dev_destroy((dev_object_t **)&ts_obj);

    return (ret_val);
}

/*******************************************************************************
 *
 * Function    : diag_temp_sensor_show_temp
 * Description : Function to show MB module temperature sensor
 *               detected temperature.
 * Inputs      : None
 * Outputs     : PASSED / FAILED
 *
 *******************************************************************************
 */
int diag_temp_sensor_show_temp (void)
{
    dev_lm75b_object_t ts_data;
    dev_lm75b_object_t *ts_obj = &ts_data;
    int                ret_val = FAILED;

    diag_ts_dev_create(ts_obj);

    if (ts_obj == NULL) {
        printf("%s: Null Object\n", __func__);
        return (FAILED);
    }

    ret_val = ts_obj->callin_fvt->show_temp((dev_object_t *)ts_obj);

    ts_obj->base.dev_object_fvt->dev_destroy((dev_object_t **)&ts_obj);

    return (ret_val);
}

/*******************************************************************************
 *
 * Function    : diag_temp_sensor_int_test
 * Description : Function to execute Temperature Sensor Interrupt Test
 * Inputs      : None
 * Outputs     : PASSED / FAILED
 *
 *******************************************************************************
 */
int diag_temp_sensor_int_test (void)
{
    dev_lm75b_object_t ts_data;
    dev_lm75b_object_t *ts_obj = &ts_data;
	int ix, max_retry = 20, rc = 0;
    uint gpio_value;
    ushort reg_val = 0, reg_val2 = 0, tos_val = 0, thyst_val = 0;
    unsigned char ts_gpio_pin = DNV_GPIO_5;
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
   // platform_get_pid((char *)mb_get_pid);
    strcpy((char *)mb_get_loc, "MB");
    platform_fru_table[fru_table_offset].pid_string = mb_get_pid;
    platform_fru_table[fru_table_offset].location_string = mb_get_loc;

    /* Segment 2: Test step captured from prpass */
    /* Segment 3: Failure message captured from cterr */

    /* Segment 4: Components used */
    cterr_add_component("Intel Denverton SOC C3558", "I2C", 
                        "Thernal sensor NXP LM75B");

    /* Segment 5: register and memory dump */

    /* Segment 6: Platform Environment initialized here*/

    /* Segment 7: Top 3 Debugging Steps */
    cterr_add_debug("Check the interface between the Host SoC"
                    " and the failed I2C devices.", "If there"
                    " is no problem for these interfaces,"
                    " replace one I2C device and redo the test.");

    testname("Temperature Sensor Interrupt");
	 
    diag_ts_dev_create(ts_obj);

    if (ts_obj == NULL) {
        printf("%s: Null Object\n", __func__);
        return (FAILED);
    }

    if ((ts_obj->callout_fvt->rd((uint32)LM75B_TOS, (ushort *)&tos_val) != PASSED) || 
                                 (ts_obj->callout_fvt->rd((uint32)LM75B_THYST, 
                                 (ushort *)&thyst_val) != PASSED)) {
      printf("Can not read register\n");
      return (FAILED);
   }
	
    if (dnv_gpio_read_rx_val(ts_gpio_pin, &gpio_value) != PASSED) {
        cterr('f', 0, "Can not read CPU GPIO");
        return (FAILED);
    }
    /* Check if interrupt is already asserted before the test */
    if (gpio_value == GPIO_LOW) {
        printf("Temperature Sensor Interrupt is already asserted\n");
	    reg_val = CLEAR_INTERRUPT_LM75;
        reg_val2 = CLEAR_INTERRUPT_LM75;
	   
	   	if((ts_obj->callout_fvt->wr((uint32)LM75B_TOS, (ushort *)&reg_val) != PASSED) || 
	                                (ts_obj->callout_fvt->wr((uint32)LM75B_THYST, 
                                    (ushort *)&reg_val2)!= PASSED)) {
            printf("Clear Interrupt failed\n");
            return (FAILED);
        }

        prpass(testpass, "Check CPU Interrupt again(Before the test)");
	 
        for (ix = 0; ix < max_retry; ix++) {
            rc = dnv_gpio_read_rx_val(ts_gpio_pin, &gpio_value);
            if (rc != PASSED) {
                cterr('f', 0, "Read GPIO Value Fails");
            }

            if (gpio_value == GPIO_HIGH) {
                break;
            }
            msleep (POLL_DELAY);
        }

        if (ix == max_retry) {
            cterr('f', 0, "Interrupt is not clear");
            return (FAILED);
        }
   }

   prpass(testpass, "Enable and Force Interrupt");
   reg_val = FORCE_INTERRUPT_LM75;
   reg_val2 = FORCE_INTERRUPT_LM75;
	
	if((ts_obj->callout_fvt->wr((uint32)LM75B_TOS, (ushort *)&reg_val) != PASSED) || 
	                            (ts_obj->callout_fvt->wr((uint32)LM75B_THYST,
                                (ushort *)&reg_val2)!= PASSED)) {
      printf("Enable and Force Interrupt failed\n");
      return (FAILED);
   }

    prpass(testpass, "Check CPU Interrupt (After the test)");
	 
    for (ix = 0; ix < max_retry; ix++) {
        rc = dnv_gpio_read_rx_val(ts_gpio_pin, &gpio_value);
        if (rc != PASSED) {
            cterr('f', 0, "Read GPIO Value Fails");
            return (FAILED);
        }

        if (gpio_value == GPIO_LOW) {
            break;
        }
        msleep (100);
    }

    if (ix == max_retry) {
        cterr('f', 0, "Interrupt is not detected");
        return (FAILED);
    }

    /* Disable Interrupt */
    prpass(testpass, "Disable Interrupt");
    if((ts_obj->callout_fvt->wr((uint32)LM75B_TOS, (ushort *)&tos_val) != PASSED) || 
	                            (ts_obj->callout_fvt->wr((uint32)LM75B_THYST, 
                                (ushort *)&thyst_val)!= PASSED)) {
      printf("Disable Interrupt failed\n");
      return (FAILED);
   }

    ts_obj->base.dev_object_fvt->dev_destroy((dev_object_t **)&ts_obj);
    return (PASSED);
}

/*------------------------------------------------------------------
$Log: diag_temp_snsr_test.c,v $
Revision 1.4  2019/07/11 12:31:30  alicehua
Collapse Nutella codes into main trunk.

$Endlog$
*/
