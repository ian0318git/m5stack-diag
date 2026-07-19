/* $Id: diag_dimm_util.c,v 1.2 2019/01/10 06:36:22 wilbhuan Exp $ 
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/betelgeuse/diag_dimm_util.c,v $
 *------------------------------------------------------------------
 * 
 * diag_dimm_util.c
 *
 * Copyright (c) 2018 - 2019 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#include <assert.h>
#include <stdio.h>
#include "endians.h"
#include "common.h"
#include "types.h"
#include "defs.h"
#include "menu.h"
#include "fcntl.h"
#include "proto.h"
#include "stdlib.h"
#include "signals.h"
#include "string.h"
#include "error.h"
#include "cross_platform.h"
#include "i2c_api.h"
#include "n2g_api_rc.h"
#include "platform_i2c.h"
#include "queryflags.h"

#include "diag_dimm_lib.h"
#include "diag_dimm_util.h"

/******************************************************************************
 *                             Function protos
 ******************************************************************************/
void dimm_util_entry(void);
int dump_dimm_info(void);
int dump_dimm_raw_data(void);


/******************************************************************************
 *                             Global Variables
 ******************************************************************************/


/******************************************************************************
 *                                 Menus
 ******************************************************************************/
/*
 * DIMM Utility menu
 */
static submenu_xtable_t dimm_util_tbl[] = {
    {"Show Dimm info",          (PFT)dump_dimm_info,       0,
     0, 
     (type_t(*)())0,            0,
     (type_t(*)())0,            0},
    {"Show Dimm data in RAW",   (PFT)dump_dimm_raw_data,   0,
     0, 
     (type_t(*)())0,            0,
     (type_t(*)())0,            0},
};

#define DIMM_UTIL_TBL_SIZE (sizeof(dimm_util_tbl) / sizeof(submenu_xtable_t))

/*
 * Primary & secondary submenu items (filled in from xtable)
 */
static mitem_t dimm_util_menu_pri_items[DIMM_UTIL_TBL_SIZE + MAX_BASE_ITEMS];
static mitem_t dimm_util_menu_sec_items[DIMM_UTIL_TBL_SIZE + MAX_BASE_ITEMS];

static menuinfo_t dimm_util_menu = {
    "%s Utility Menu",
    0,                          /* mtparam added by init_empty_menu */
    (PFT) show_endnote,         /* notes missing WICs in combos */
    0,                          /* use generic prompt */
    0,                          /* size (bumped by add_menu_item() */
    dimm_util_menu_pri_items,
};

menuinfo_t *dimm_util_menu_p = &dimm_util_menu;


/*******************************************************************************
 *
 * Function   : dimm_util_entry
 * Description: Entry functon of DIMM utility.
 * Inputs     : None
 * Outputs    : None
 *
 *******************************************************************************
 */
void dimm_util_entry (void)
{
    build_primary_submenu(dimm_util_tbl, DIMM_UTIL_TBL_SIZE,
                          "DIMM", &dimm_util_menu_p);
    build_secondary_submenu(dimm_util_tbl, DIMM_UTIL_TBL_SIZE,
                            dimm_util_menu_sec_items);

    menu(dimm_util_menu_p, dimm_util_menu_sec_items, 0);
}

/*******************************************************************************
 *
 * Function   : dump_dimm_info
 * Description: Function to dump DIMM info.
 * Inputs     : None
 * Outputs    : PASSED / FAILED
 *
 *******************************************************************************
 */
int dump_dimm_info (void)
{
    n2g_i2c_dev_t dimm_i2c;

    /*
     * Init I2C device structure 
     */
    if (init_dimm_i2c_struct(&dimm_i2c) != PASSED) {
        printf("%s(): Failed to init DIMM i2c_dev struct.\n", __func__);
        return (FAILED);
    }

    /*
     * Display the registers 
     */
    if (dump_dimm_data(&dimm_i2c, DIMM_DUMP_ALL) != PASSED) {
        printf("%s(): Failed to dump DIMM info.\n", __func__);
        return (FAILED);
    }
    return (PASSED);
}


/*******************************************************************************
 *
 * Function   : dump_dimm_raw_data
 * Description: Function to dump DIMM RAW data.
 * Inputs     : None
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
int dump_dimm_raw_data (void)
{
    n2g_i2c_dev_t dimm_i2c;

    /*
     * Init I2C device structure 
     */
    if (init_dimm_i2c_struct(&dimm_i2c) != PASSED) {
        printf("%s(): Failed to init DIMM i2c_dev struct.\n", __func__);
        return (FAILED);
    }

    /*
     * Dump DIMM RAW data 
     */
    if (dump_dimm_data(&dimm_i2c, DIMM_DUMP_RAW) != PASSED) {
        printf("%s(): Failed to dump DIMM RAW data.\n", __func__);
        return (FAILED);
    }
    return (PASSED);
}

/*-------------------------------------------------
 * $Log: diag_dimm_util.c,v $
 * Revision 1.2  2019/01/10 06:36:22  wilbhuan
 * The beginning of Betelgeuse application code.
 *
 *-------------------------------------------------
 */
