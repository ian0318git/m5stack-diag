/* $Id: diag_temperature_sensor_test.c,v 1.2 2013/10/08 08:48:29 tirawan Exp $ 
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/woodlawn_linux/diag_temperature_sensor_test.c,v $ 
 *-----------------------------------------------------------------------------
 * diag_temperature_sensor_test.c - Menus for Woodlawn Temp. Sensor
 *
 * February 2012, Leslie Chen
 * Copyright (c) 2013 by Cisco Systems, Inc.
 * All rights reserved.
 *-----------------------------------------------------------------------------
 */
#include "error.h"
#include "common.h"
#include "types.h"
#include "menu.h"
#include <stdio.h>
#include "dev_tmp421.h"
#include "diag_temperature_sensor_lib.h"

#define MF_1    (MF_CONTINUOUS | MF_DOGRP)
#define MF_2    (MF_1 | MF_DOALL)
#define MF_3    (MF_2 | MF_SHOW_ERRCOUNT)
#define MF_4    (MF_1 | MF_SHOW_ERRCOUNT)

/*****************************************************************************
 *  Functions Declaration
 *****************************************************************************/
int temperature_sensor_test(int);
int temperature_sensor_utility(int);
int tmp421_register_test(void);
int tmp421_id_check_test(void);
long show_temperature(void);
long dump_tmp421_registers(void);
int temp_sensor_do_all_wrapper(void);

/******************************************************************************
 *  List of Temperature Sensor Utilities
 *****************************************************************************/
static submenu_xtable_t temperature_sensor_util_items[] = {
    {"Show Temperature", (type_t(*)())show_temperature, 0, 0, (type_t(*)())0,
     0, (type_t(*)())0, 0},
    {"Dump TMP421AID Registers", (type_t(*)())dump_tmp421_registers, 0, 0,
     (type_t(*)())0, 0, (type_t(*)())0, 0},
};

/******************************************************************************
 *  Macro Definitions
 *****************************************************************************/
#define TEMPERATURE_SENSOR_TESTS_UTIL_SIZE (sizeof(temperature_sensor_util_items) / \
            sizeof(submenu_xtable_t))

#define TEMPERATURE_SENSOR_TESTS_SUBMENU_TABLE_SIZE (sizeof(temperature_sensor_tests_submenu_table) / \
            sizeof(submenu_xtable_t))

/******************************************************************************
 *  Global Variable
 *****************************************************************************/

/******************************************************************************
 * temperature sensor util items (filled in from xtable)
 *****************************************************************************/
static mitem_t temperature_sensor_tests_primary_util_items[TEMPERATURE_SENSOR_TESTS_UTIL_SIZE +
                     MAX_BASE_ITEMS];
static mitem_t temperature_sensor_tests_secondary_util_items[TEMPERATURE_SENSOR_TESTS_UTIL_SIZE +
                     MAX_BASE_ITEMS];

/******************************************************************************
 * Temperature Sensor Utils submenu
 *****************************************************************************/
menuinfo_t temperature_sensor_util_menu = {
    "Temperature Sensor Utility Menu",
    0,
    (PFT)show_endnote,
    0,
    0,
    temperature_sensor_tests_primary_util_items,
};

menuinfo_t *temperature_sensor_util_menup = &temperature_sensor_util_menu;

/******************************************************************************
 *  Sub Menu used for Temperature sensor tests.
 *****************************************************************************/
static submenu_xtable_t temperature_sensor_tests_submenu_table[] = {
   {"Temperature Sensor Utilities", (type_t(*)()) temperature_sensor_utility,   FALSE,
       0, NULL, 0, (type_t(*)()) temperature_sensor_utility,   TRUE}, 
   {"Temperature Sensor Register Test", (type_t(*)()) tmp421_register_test,   0,
       MF_3, (type_t(*)())0, 0, (type_t(*)())0,   0},
   {"Temperature Sensor ID Check Test", (type_t(*)()) tmp421_id_check_test,   0,
       MF_3, (type_t(*)())0, 0, (type_t(*)())0,   0},
};

/******************************************************************************
 * Primary & secondary submenu items (filled in from xtable)
 *****************************************************************************/
static mitem_t temperature_sensor_tests_primary_items[TEMPERATURE_SENSOR_TESTS_SUBMENU_TABLE_SIZE +
    MAX_BASE_ITEMS];
static mitem_t temperature_sensor_tests_secondary_items[TEMPERATURE_SENSOR_TESTS_SUBMENU_TABLE_SIZE + 
    MAX_BASE_ITEMS];

menuinfo_t temperature_sensor_subtest_menu = {
    "%s Subtest Menu",
    0,                                /* mtparam added by init_empty_menu */
    (PFT)show_endnote,                /* notes missing WICs in combos */
    0,                                /* use generic prompt */
    0,                                /* size (bumped by add_menu_item() */
    temperature_sensor_tests_primary_items,
};
menuinfo_t *temperature_sensor_submenup = &temperature_sensor_subtest_menu;

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
 * Inputs: show_menu - FALSE for tests. TRUE for submenu.
 *****************************************************************************/
int temperature_sensor_test (int show_menu)
{
    build_primary_submenu(temperature_sensor_tests_submenu_table,
                            TEMPERATURE_SENSOR_TESTS_SUBMENU_TABLE_SIZE,
                          "Temperature sensor", &temperature_sensor_submenup);
    build_secondary_submenu(temperature_sensor_tests_submenu_table,
                            TEMPERATURE_SENSOR_TESTS_SUBMENU_TABLE_SIZE,
                            temperature_sensor_tests_secondary_items);
    
    if (show_menu) {
        menu(temperature_sensor_submenup, temperature_sensor_tests_secondary_items,
             '\0' );
    } else {
        menu_exec_doall_diags(temperature_sensor_submenup);
    }
    return (PASSED);
}

/*******************************************************************************
 *
 * Function    : temp_sensor_do_all_wrapper
 * Description : Wrapper for Temperature Sensor do all test items
 * Inputs      : none
 * Outputs     : PASSED / FAILED
 *
 *******************************************************************************
 */
int temp_sensor_do_all_wrapper (void)
{
    int rc = PASSED;

    if (tmp421_register_test() == FAILED) {
        rc = FAILED;
    }

    if (tmp421_id_check_test() == FAILED) {
        rc = FAILED;
    }

    return (rc);
}

/*******************************************************************************
 *
 * Function    : temperature_sensor_utility
 * Description :
 * Inputs      : menu_option - display menu instead of running all temperature
 *                             sensor tests. FALSE for tests. TRUE for submenu.
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/
int temperature_sensor_utility (int show_menu)
{

    build_primary_submenu(temperature_sensor_util_items, TEMPERATURE_SENSOR_TESTS_UTIL_SIZE,
                          "TEMPERATURE SENSUR Utilities Menu", &temperature_sensor_util_menup);
    build_secondary_submenu(temperature_sensor_util_items, TEMPERATURE_SENSOR_TESTS_UTIL_SIZE,
                            temperature_sensor_tests_secondary_util_items);

    if (show_menu) {
        menu(temperature_sensor_util_menup,
                  temperature_sensor_tests_secondary_util_items, '\0' );
     } else {
        menu_exec_doall_diags(temperature_sensor_util_menup);
        prcomplete(testpass, errcount, (char *)0);
     }

    return (PASSED);
}

/******************************************************************************
 *
 * Function: tmp421_register_test
 *
 * Description: This function performs the TMP421 register test.
 *
 * Inputs      : None
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/
int tmp421_register_test(void)
{
    int ret_val = PASSED;

    dev_tmp421_object_t *tmp421_obj;

    /* Get the PCA9541 object and assign the I2C related information. */
    tmp421_obj = (dev_tmp421_object_t *)get_tmp421_obj();

    if (tmp421_obj == NULL) {
        cterr('f', 0, "%s: TMP421 Null Object", __FUNCTION__);
        return (FAILED);
    }

    if (tmp421_obj->callin_fvt->register_test((dev_object_t *)tmp421_obj)
        == FAILED) {
        ret_val = FAILED;
    }
    prpass(testpass, "tmp421 register test success");
    return (ret_val);
}

/******************************************************************************
 *
 * Function: tmp421_id_check_test
 *
 * Description: This function check the TMP421 chip ID.
 *
 * Inputs      : None
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/
int tmp421_id_check_test(void)
{
    int ret_val = PASSED;

    dev_tmp421_object_t *tmp421_obj;

    /* Get the PCA9541 object and assign the I2C related information. */
    tmp421_obj = (dev_tmp421_object_t *)get_tmp421_obj();

    if (tmp421_obj == NULL) {
        cterr('f', 0, "%s: TMP421 Null Object", __FUNCTION__);
        return (FAILED);
    }

    if (tmp421_obj->callin_fvt->check_chip_id((dev_object_t *)tmp421_obj)
        == FAILED) {
        ret_val = FAILED;
    }

    return (ret_val);
}

/******************************************************************************
 *
 * Function: dump_tmp421_registers
 *
 * Description: This function dump the TMP421 registers
 *
 * Inputs      : None
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/
long dump_tmp421_registers(void)
{
    int ret_val = PASSED;

    dev_tmp421_object_t *tmp421_obj;

    /* Get the PCA9541 object and assign the I2C related information. */
    tmp421_obj = (dev_tmp421_object_t *)get_tmp421_obj();

    if (tmp421_obj == NULL) {
        cterr('f', 0, "%s: TMP421 Null Object", __FUNCTION__);
        return (FAILED);
    }

    if (tmp421_obj->callin_fvt->dump_register((dev_object_t *)tmp421_obj)
        == FAILED) {
        ret_val = FAILED;
    }

    return (ret_val);
}

/******************************************************************************
 *
 * Function: show_temperature
 *
 * Description: This function show the TMP421 temperature
 *
 * Inputs      : None
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/
long show_temperature(void)
{
    int ret_val = PASSED;

    dev_tmp421_object_t *tmp421_obj;

    /* Get the PCA9541 object and assign the I2C related information. */
    tmp421_obj = (dev_tmp421_object_t *)get_tmp421_obj();

    if (tmp421_obj == NULL) {
        cterr('f', 0, "%s: TMP421 Null Object", __FUNCTION__);
        return (FAILED);
    }

    if (tmp421_obj->callin_fvt->show_temp((dev_object_t *)tmp421_obj)
        == FAILED) {
        ret_val = FAILED;
    }

    return (ret_val);
}

/*-------------------------------------------------
 * $Log: diag_temperature_sensor_test.c,v $
 * Revision 1.2  2013/10/08 08:48:29  tirawan
 * Woodlawn collapsed to main trunk
 *
 * Revision 1.1.4.2  2013/08/20 10:58:54  tirawan
 * Branch into woodlawn-branch2 and port woodlawn code
 *
 * Revision 1.1.2.1  2013/04/24 10:37:18  tirawan
 * Initial check-in for woodlawn linux code
 *
 * Revision 1.2  2013/04/10 09:48:48  kuangik
 * Implement do all wrapper for host run sm test
 *
 * Revision 1.1  2013/03/13 06:42:53  kuangik
 * Add for the first time
 *
 * Revision 1.9  2013/03/12 11:22:34  leslie
 * Fix submenu flag
 *
 * Revision 1.8  2013/03/07 02:24:04  kuangik
 * Add Show error count
 *
 * Revision 1.7  2012/08/30 06:35:06  leslie
 * Add pass message.
 *
 * Revision 1.6  2012/08/03 10:16:55  evanli
 * Mapping to latest O2 source code on 20120726
 *
 * Revision 1.4  2012/07/19 06:16:50  leslie
 * Use cterr instead of use err
 *
 * Revision 1.3  2012/03/26 07:20:42  kody
 * Modify and add TMP421 temperature sensor test code.
 *
 * Revision 1.2  2012/02/13 03:32:20  leslie
 * Add function prototype.
 *
 * Revision 1.1  2012/02/10 07:04:40  leslie
 * Add Woodlawn temperature sensor test.
 * 
 *
 * $Endlog$
 *-------------------------------------------------
 */
