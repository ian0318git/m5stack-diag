/* $Id: diag_temp_sensor_util.c,v 1.2 2021/09/24 01:21:07 harrchan Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/elixir/diag_temp_sensor_util.c,v $
 *------------------------------------------------------------------
 * 
 * diag_temp_sensor_util.c
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
#include "diag_temp_sensor_util.h"


/*******************************************************************************
 *                          Function Declaration
 *******************************************************************************
 */
static int show_ts_reg_util(void);
static int alter_ts_reg_util(void);
static int dump_ts_reg_util(void);
int show_plat_curr_temps(void);


/*******************************************************************************
 *                            Global Variables
 *******************************************************************************
 */
static dev_max31730_object_t ts_obj;
static dev_max31730_object_t *ts_obj_p = &ts_obj;
static n2g_i2c_if_t ts_i2c_if;
static n2g_i2c_if_t *ts_i2c_if_p = &ts_i2c_if;

/*
 * Temperature sensor Utility Menu
 */
static submenu_xtable_t ts_util_table[] = {
    {"Show register",                (PFT)show_ts_reg_util,      0,
     MF_CONTINUOUS,                  (type_t(*)())0,             0,
     (type_t(*)())0,                 0},
    {"Alter register",               (PFT)alter_ts_reg_util,     0,
     MF_CONTINUOUS,                  (type_t(*)())0,             0,
     (type_t(*)())0,                 0},
    {"Dump registers",               (PFT)dump_ts_reg_util,      0,
     MF_CONTINUOUS,                  (type_t(*)())0,             0,
     (type_t(*)())0,                 0},
    {"Show current sensed Temp.",    (PFT)show_plat_curr_temps,  0,
     MF_CONTINUOUS,                  (type_t(*)())0,             0,
     (type_t(*)())0,                 0},
};

#define TS_UTIL_TABLE_SIZE (sizeof(ts_util_table)/sizeof(submenu_xtable_t))

/*
 * Primary & secondary submenu items (filled in from xtable)
 */
static mitem_t ts_util_menu_pri_items[TS_UTIL_TABLE_SIZE + MAX_BASE_ITEMS];
static mitem_t ts_util_menu_sec_items[TS_UTIL_TABLE_SIZE + MAX_BASE_ITEMS];

static struct menuinfo ts_util_menu = {
    "Temp. sensor(MAX31730) Utils Menu",   /* title */
    0,                                     /* title string */
    (PFT) menu_show_dflags,                /* shows major flags */
    0,                                     /* generic prompt */
    0,                                     /* size */
    ts_util_menu_pri_items,
};

static struct menuinfo *ts_util_menu_p = &ts_util_menu;


/*******************************************************************************
 *
 * Function   : diag_temp_sensor_util
 * Description: To build Temperature Sensor (MAX31730) menu.
 * Inputs     : None 
 * Outputs    : PASSED / FAILED
 *
 *******************************************************************************
 */
int diag_temp_sensor_util (void)
{
    build_primary_submenu(ts_util_table, TS_UTIL_TABLE_SIZE,
                          "Temperature sensor Utils Menu", &ts_util_menu_p);
    build_secondary_submenu(ts_util_table, TS_UTIL_TABLE_SIZE,
                            ts_util_menu_sec_items);

    /* Create Temperature sensor dev object for utils. */
    if (diag_ts_dev_create(ts_obj_p, ts_i2c_if_p) != PASSED) {
        printf("%s(): Failed to create Temperature sensor device object.\n",
               __func__);
        return (FAILED);
    }

    menu(&ts_util_menu, ts_util_menu_sec_items, 0);

    /* Destroy the created Temperature sensor dev object before leave utils. */
    ts_obj_p->base.dev_object_fvt->dev_destroy((dev_object_t **)&ts_obj_p);

    return (PASSED);
}

/*******************************************************************************
 *
 * Function   : show_ts_reg_util
 * Description: Utility to show Temperature sensor register value.
 * Inputs     : None
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
static int show_ts_reg_util (void)
{
    /* Sanity check */
    if (ts_obj_p->callin_fvt == NULL) {
        /* Create Temperature sensor dev object for utils. */
        if (diag_ts_dev_create(ts_obj_p, ts_i2c_if_p) != PASSED) {
            printf("%s(): Failed to create Temperature sensor device object.\n",
                   __func__);
            return (FAILED);
        }
    }

    dev_object_t *dev = (dev_object_t *)ts_obj_p;

    /* Show Temperature sensor register */
    if (ts_obj_p->callin_fvt->show_register(dev) != PASSED) {
        printf("%s(): Failed to show Temp. sensor register.\n", __func__);
        return (FAILED);
    }
    return (PASSED);
}

/*******************************************************************************
 *
 * Function   : dump_ts_reg_util
 * Description: Utility to dump Temperature sensor registers.
 * Inputs     : NONE
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
static int dump_ts_reg_util (void)
{
    /* Sanity check */
    if (ts_obj_p->callin_fvt == NULL) {
        /* Create Temperature sensor dev object for utils. */
        if (diag_ts_dev_create(ts_obj_p, ts_i2c_if_p) != PASSED) {
            printf("%s(): Failed to create Temperature sensor device object.\n",
                   __func__);
            return (FAILED);
        }
    }

    dev_object_t *dev = (dev_object_t *)ts_obj_p;

    /* Dump Temperature sensor registers */
    if (ts_obj_p->callin_fvt->dump_register(dev) != PASSED) {
        printf("%s(): Failed to dump Temp. sensor registers.\n", __func__);
        return (FAILED);
    }
    return (PASSED);
}

/*******************************************************************************
 *
 * Function   : alter_ts_reg_util
 * Description: Utility to alter Temperature sensor register.
 * Inputs     : NONE 
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
static int alter_ts_reg_util (void)
{
    /* Sanity check */
    if (ts_obj_p->callin_fvt == NULL) {
        /* Create Temperature sensor dev object for utils. */
        if (diag_ts_dev_create(ts_obj_p, ts_i2c_if_p) != PASSED) {
            printf("%s(): Failed to create Temperature sensor device object.\n",
                   __func__);
            return (FAILED);
        }
    }

    dev_object_t *dev = (dev_object_t *)ts_obj_p;

    /* Dump Temperature sensor registers */
    if (ts_obj_p->callin_fvt->alter_register(dev) != PASSED) {
        printf("%s(): Failed to dump Temp. sensor registers.\n", __func__);
        return (FAILED);
    }
    return (PASSED);
}

/*******************************************************************************
 *
 * Function   : show_plat_curr_temps
 * Description: Utility to show current sensed Temperature.
 * Inputs     : NONE 
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
int show_plat_curr_temps (void)
{
    /* Sanity check */
    if (ts_obj_p->callin_fvt == NULL) {
        /* Create Temperature sensor dev object for utils. */
        if (diag_ts_dev_create(ts_obj_p, ts_i2c_if_p) != PASSED) {
            printf("%s(): Failed to create Temperature sensor device object.\n",
                   __func__);
            return (FAILED);
        }
    }

    dev_object_t *dev = (dev_object_t *)ts_obj_p;

    /* Show reference Temperature */
    if (ts_obj_p->callin_fvt->show_temp(dev) != PASSED) {
        printf("%s(): Failed to show reference Temperature.\n", __func__);
        return (FAILED);
    }
    return (PASSED);
}

/*-------------------------------------------------
 * $Log: diag_temp_sensor_util.c,v $
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
