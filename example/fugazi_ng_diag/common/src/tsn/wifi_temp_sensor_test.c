/* $Id: wifi_temp_sensor_test.c,v 1.3 2019/01/18 05:54:47 yungchen Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/tsn/wifi_temp_sensor_test.c,v $
 *-----------------------------------------------------------------------------
 * 
 * Filename   : wifi_temp_sensor_test
 * Description: WiFi module Temperature Sensor Test Functions
 *
 * Copyright (c) 2018 ~ 2019 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *-----------------------------------------------------------------------------
 */
#include "error.h"
#include "common.h"
#include "types.h"
#include "menu.h"
#include <stdio.h>

#include <fcntl.h>
#include <asm/ioctl.h>
#include <sys/stat.h>
#include <signal.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <sys/mman.h>
#include <sys/stat.h>

#include "byteswap.h"
#include "common_utils.h"
#include "nvmonvars.h"
#include "i2c_address.h"
#include "i2c_api.h"
#include "proto.h"
#include "platform_i2c.h"
#include "platform_fpga.h"
#include "wifi_temp_sensor_test.h"
#include "wifi_temp_sensor_lib.h"
#include "platform_sensor.h"

int wifi_temp_sensor_reg_test(void);
int wifi_temp_sensor_show_reg(void);
int wifi_temp_sensor_alter_reg(void);
int wifi_temp_sensor_dump_reg(void);
int wifi_temp_sensor_show_temp(void);

/*******************************************************************************
 *                                   Menus                                     *
 *******************************************************************************
 */
/* 
 * Sub Menu used for WiFi Temperature Sensor tests
 */
submenu_xtable_t wifi_ts_submenu_table[] = {
    {"WiFi Temp. sensor reg. read Util",    (PFT)wifi_temp_sensor_show_reg,    0,
     MF_CONTINUOUS,
     (type_t(*)())0,            0,
     (type_t(*)())0,            0},
    {"WiFi Temp. sensor reg. write Util",   (PFT)wifi_temp_sensor_alter_reg,   0,
     MF_CONTINUOUS,
     (type_t(*)())0,            0,
     (type_t(*)())0,            0},
    {"WiFi Temp. sensor reg. dump Util",    (PFT)wifi_temp_sensor_dump_reg,    0,
     MF_CONTINUOUS,
     (type_t(*)())0,            0,
     (type_t(*)())0,            0},
    {"WiFi Temp. sensor show temp. Util",   (PFT)wifi_temp_sensor_show_temp,   0,
     MF_CONTINUOUS,
     (type_t(*)())0,            0,
     (type_t(*)())0,            0},
    {"WiFi Temperature Sensor Reg. Test", (PFT)wifi_temp_sensor_reg_test,   0,
     (MF_CONTINUOUS | MF_DOGRP | MF_DOALL | MF_SHOW_ERRCOUNT),
     (type_t(*)())0,            0,
     (type_t(*)())0,            0},
};

#define WIFI_TS_SUBMENU_TABLE_SIZE (sizeof(wifi_ts_submenu_table) / \
                                    sizeof(submenu_xtable_t))

static mitem_t wifi_ts_pri_items[WIFI_TS_SUBMENU_TABLE_SIZE + MAX_BASE_ITEMS];
static mitem_t wifi_ts_sec_items[WIFI_TS_SUBMENU_TABLE_SIZE + MAX_BASE_ITEMS];

static struct menuinfo wifi_ts_submenu = {
    "%s Menu",               /* title */
    0,                       /* title string added by init_empty_menu */
    0,                       /* do not show major flags */
    0,                       /* generic prompt */
    0,                       /* size -- bumped by add_menu_item() */
    wifi_ts_pri_items,
};

static struct menuinfo *wifi_ts_submenup = &wifi_ts_submenu;

/*******************************************************************************
 *
 * Function   : wifi_temp_sensor_test
 * Description: Entry function of WiFi Temperature Sensor test
 * Inputs     : None
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
int wifi_temp_sensor_test (boolean wifi_ts_items_executed)
{
    char *tname = "WiFi Temperature Sensor";

    if ((this_is_star() == TRUE) || (this_is_supernova() == TRUE)) {
    if ((tsn_fpga_check_dev_present(FPGA_CPP_WLAN_PRESENT)) != TRUE) {
        cterr('f', 0, "%s: WiFi module is NOT present.", __func__);
        return (FAILED);
    }

    testname(tname);

    build_primary_submenu(wifi_ts_submenu_table,
                          WIFI_TS_SUBMENU_TABLE_SIZE, "Wifi",
                          &wifi_ts_submenup);

    build_secondary_submenu(wifi_ts_submenu_table,
                            WIFI_TS_SUBMENU_TABLE_SIZE,
                            wifi_ts_sec_items);

    if (wifi_ts_items_executed) {
        menu_exec_doall_diags(wifi_ts_submenup);
    } else {
        /* Entered with submenu */
        menu(&wifi_ts_submenu, wifi_ts_sec_items, 0);
    }
    } else {
        /* TSN */
        if (wifi_ts_items_executed == FALSE) {
            build_wifi_snsr_menu(TRUE);
        } else {
            build_wifi_snsr_menu(FALSE);
        }
    }

    return (PASSED);
}

/*******************************************************************************
 *
 * Function    : wifi_temp_sensor_reg_test
 * Description : Function to execute WiFi Temperature Sensor Register Test
 * Inputs      : None
 * Outputs     : PASSED / FAILED
 *
 *******************************************************************************
 */
int wifi_temp_sensor_reg_test (void)
{
    dev_lm75b_object_t ts_data;
    dev_lm75b_object_t *ts_obj = &ts_data;

    testname("WiFi Temperature Sensor");

    wifi_ts_dev_create(ts_obj);

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
 * Function    : wifi_temp_sensor_show_reg
 * Description : Function to display WiFi module temperature sensor Register
 * Inputs      : None
 * Outputs     : PASSED / FAILED
 *
 *******************************************************************************
 */
int wifi_temp_sensor_show_reg (void)
{
    dev_lm75b_object_t ts_data;
    dev_lm75b_object_t *ts_obj = &ts_data;
    int                ret_val = FAILED;

    wifi_ts_dev_create(ts_obj);

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
 * Function    : wifi_temp_sensor_alter_reg
 * Description : Function to alter WiFi module temperature sensor Register
 * Inputs      : None
 * Outputs     : PASSED / FAILED
 *
 *******************************************************************************
 */
int wifi_temp_sensor_alter_reg (void)
{
    dev_lm75b_object_t ts_data;
    dev_lm75b_object_t *ts_obj = &ts_data;
    int                ret_val = FAILED;

    wifi_ts_dev_create(ts_obj);

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
 * Function    : wifi_temp_sensor_dump_reg
 * Description : Function to dump WiFi module temperature sensor Registers
 * Inputs      : None
 * Outputs     : PASSED / FAILED
 *
 *******************************************************************************
 */
int wifi_temp_sensor_dump_reg (void)
{
    dev_lm75b_object_t ts_data;
    dev_lm75b_object_t *ts_obj = &ts_data;
    int                ret_val = FAILED;

    wifi_ts_dev_create(ts_obj);

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
 * Function    : wifi_temp_sensor_show_temp
 * Description : Function to show WiFi module temperature sensor
 *               detected temperature.
 * Inputs      : None
 * Outputs     : PASSED / FAILED
 *
 *******************************************************************************
 */
int wifi_temp_sensor_show_temp (void)
{
    dev_lm75b_object_t ts_data;
    dev_lm75b_object_t *ts_obj = &ts_data;
    int                ret_val = FAILED;

    wifi_ts_dev_create(ts_obj);

    if (ts_obj == NULL) {
        printf("%s: Null Object\n", __func__);
        return (FAILED);
    }

    ret_val = ts_obj->callin_fvt->show_temp((dev_object_t *)ts_obj);

    ts_obj->base.dev_object_fvt->dev_destroy((dev_object_t **)&ts_obj);

    return (ret_val);
}


/*------------------------------------------------------------------
$Log: wifi_temp_sensor_test.c,v $
Revision 1.3  2019/01/18 05:54:47  yungchen
Merge Supernova branch to the main trunk (CSCvn79871)

Revision 1.2  2018/02/09 09:56:57  hondwang
Merge Star branch star-branch-c9xx to main trunk

Revision 1.1.6.2  2018/01/20 05:57:50  hondwang
prepare merge star-branch-c9xx to main trunk

Revision 1.1.4.2  2017/08/15 14:18:40  hondwang
star branch c9xx initial check in

Revision 1.1.2.1  2017/07/04 15:08:39  palin2
Added Star wifi temperature sensor diag tests.

$Endlog$
*/

