/* $Id: plug_testcard_util.c,v 1.2 2018/01/20 05:01:10 hondwang Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/plug_testcard/plug_testcard_util.c,v $
 *------------------------------------------------------------------
 *
 * plug_testcard_util.c - PLUGGABLE Test Card Utility
 *
 * Copyright (c) 2015 - 2018 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <fcntl.h>
#include "common.h"
#include "types.h"
#include "error.h"
#include "menu.h"
#include "queryflags.h"
#include "plug_slot.h"
#include "plug_testcard_util.h"
#include "plug_testcard_gpio_exp_lib.h"
#include "plug_temp_sensor_test.h"
#include "plug_temp_sensor_lib.h"

static int plug_testcard_show_temp(int);
static int plug_testcard_ts_util(int);
static int plug_testcard_gpio_exp_util(int);

int plug_testcard_util(void);

static submenu_xtable_t pluggable_testcard_utils[] = {
    {"GPIO Expander Register Read/Write Utility", (type_t(*)())plug_testcard_gpio_exp_util, TRUE,
     0, (type_t(*)())0, 0, (type_t(*)())0, 0},
    {"Temperature Display Utility", (type_t(*)())plug_testcard_show_temp, TRUE,
     0, (type_t(*)())0, 0, (type_t(*)())0, 0},
    {"Temperature Sensor Register Read/Write Utility", (type_t(*)())plug_testcard_ts_util, TRUE,
     0, (type_t(*)())0, 0, (type_t(*)())0, 0},
};

#define PLUG_TESTCARD_UTIL_TABLE_SZ \
        (sizeof(pluggable_testcard_utils) / sizeof(submenu_xtable_t))


static mitem_t plug_tc_pri_util_items[PLUG_TESTCARD_UTIL_TABLE_SZ+ MAX_BASE_ITEMS];
static mitem_t plug_tc_sec_util_items[PLUG_TESTCARD_UTIL_TABLE_SZ+ MAX_BASE_ITEMS];

static menuinfo_t plug_tc_util_menu = {
    "Pluggable Test Card Utilities Menu",
    0,                            /* mtparam added by init_empty_menu */
    (PFT)menu_show_dflags,        /* shows major flags */
    0,                            /* use generic prompt */
    0,                            /* size (bumped by add_menu_item() */
    plug_tc_pri_util_items,
};
static menuinfo_t *plug_tc_util_menup = &plug_tc_util_menu;

extern struct plug_intf_t *plug_test_if;

/*******************************************************************************
 * Function   : plug_testcard_util
 * Description: Main Entry point for Pluggable Test card Utilities
 * Inputs     : None
 * Outputs    : PASSED or FAILED
 *******************************************************************************
 */
int plug_testcard_util (void)
{   
    char title[128];

    sprintf(title, "Pluggable Test Card Utilities - Slot %d", plug_test_if->slot);

    build_primary_submenu(pluggable_testcard_utils, PLUG_TESTCARD_UTIL_TABLE_SZ, 
                          title, &plug_tc_util_menup);

    build_secondary_submenu(pluggable_testcard_utils, PLUG_TESTCARD_UTIL_TABLE_SZ,
                            plug_tc_sec_util_items);

    menu(&plug_tc_util_menu, plug_tc_sec_util_items, '\0');

    return (PASSED);
}



/*******************************************************************************
 * Function   : plug_testcard_gpio_exp_util
 * Description: GPIO Expander Utility for Pluggable Test Card
 * Inputs     : input - Not used
 * Outputs    : PASSED or FAILED
 *******************************************************************************
 */
static int plug_testcard_gpio_exp_util (int input)
{
    dev_pca9557_object_t pca_data;
    dev_pca9557_object_t *pca_obj = &pca_data;
    int opt;

    plug_tc_gpio_exp_dev_create(pca_obj);

    if (pca_obj == NULL) {
        cterr('f', 0, "%s: Null Object\n", __func__);
        return (FAILED);
    }

    printf("GPIO Expander Register Read/Write Utility\n");
    opt = getdec_answer("Read/Write Register? (0-Read, 1-Write):", OPT_READ, 
                         OPT_READ, OPT_WRITE);

    if (opt == OPT_READ) {
        pca_obj->callin_fvt->dump_register((dev_object_t *)pca_obj); 
    } else {
        pca_obj->callin_fvt->alter_register((dev_object_t *)pca_obj); 
    }
    pca_obj->base.dev_object_fvt->dev_destroy((dev_object_t **)&pca_obj);

    return (PASSED);
}


/*******************************************************************************
 * Function   : plug_testcard_ts_util
 * Description: Thermal Sensor Utility for Pluggable Test Card
 * Inputs     : input - Not used
 * Outputs    : PASSED or FAILED
 *******************************************************************************
 */
static int plug_testcard_ts_util (int input)
{
    int opt;

    printf("Temperature Sensor Register Read/Write Utility\n");
    opt = getdec_answer("Read/Write Register? (0-Read, 1-Write):", OPT_READ, 
                         OPT_READ, OPT_WRITE);

    if (opt == OPT_READ) {
        return (plug_temp_sensor_show_reg());
    } else {
        return (plug_temp_sensor_alter_reg());
    }
}


/*******************************************************************************
 * Function   : plug_testcard_show_temp
 * Description: This function display temperature detected by temperature sensor
 * Inputs     : input - Not used
 * Outputs    : PASSED or FAILED
 *******************************************************************************
 */
static int plug_testcard_show_temp (int input)
{
    return (plug_ts_show_temp());
}

/*-------------------------------------------------
$Log: plug_testcard_util.c,v $
Revision 1.2  2018/01/20 05:01:10  hondwang
prepare merge star-branch-c9xx to main trunk

Revision 1.1.4.2  2017/08/08 07:44:28  hondwang
add pluggable testcard for star-branch-c9xx

Revision 1.1.2.1  2017/07/13 06:32:22  tirawan
Reorganize Star Pluggable directory structure

Revision 1.1.2.3  2017/06/22 19:27:12  tirawan
Add LTE Test items and add log section at the bottom of the code


*/

