/* $Id: diag_poe_psu_test.c,v 1.2 2019/01/10 06:36:23 wilbhuan Exp $ 
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/betelgeuse/diag_poe_psu_test.c,v $
 *------------------------------------------------------------------
 * 
 * diag_poe_psu_test.c
 *
 * Copyright (c) 2018 - 2019 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#include <string.h>
#include <stdio.h>
#include "common.h"
#include "proto.h"
#include "queryflags.h"
#include "menu.h"
#include "error.h"
#include "diag_enhance_err_msg_lib.h"
#include "diag_i2c_lib.h"
#include "platform_i2c.h"
#include "diag_poe_psu_lib.h"
#include "diag_poe_psu_util.h"
#include "diag_poe_psu_test.h"
#include "dev_tps2386b.h"
#include "platform_cookie.h"

/******************************************************************************
 *                              Global Variables                               
 ******************************************************************************/
static dev_tps2386b_object_t poe_obj;
static dev_tps2386b_object_t *poe_obj_p = &poe_obj;
static n2g_i2c_if_t poe_i2c_if;
static n2g_i2c_if_t *poe_i2c_if_p = &poe_i2c_if;

/******************************************************************************
 *                              Static Function                               
 ******************************************************************************/
static int diag_psu_intr_test(int);
/******************************************************************************
 *                                   Menus                                     
 ******************************************************************************/
/* PoE PSU Diag Menu */
static submenu_xtable_t psu_diag_tbl[] = {
    {"TPS2386B utilities",
     (type_t(*)())diag_poe_psu_util, TRUE, 
      0, (type_t(*)())0, 0,
     (type_t(*)())diag_poe_psu_util, FALSE},
    {"TPS2386B register test",
     (type_t(*)())diag_psu_reg_test, TRUE,
     (MF_CONTINUOUS | MF_DOGRP | MF_DOALL | MF_SHOW_ERRCOUNT),
     (type_t(*)())0, 0, (type_t(*)())0, 0},
    {"TPS2386B interrupt test", 
     (type_t(*)())diag_psu_intr_test, TRUE,
     (MF_CONTINUOUS | MF_DOGRP | MF_DOALL | MF_SHOW_ERRCOUNT),
     (type_t(*)())0, 0, (type_t(*)())0, 0},
};

#define PSU_DIAG_TBL_SIZE (sizeof(psu_diag_tbl) / sizeof(submenu_xtable_t))

/* Primary & secondary submenu items (filled in from xtable) */
static mitem_t psu_diag_pri_items[PSU_DIAG_TBL_SIZE + MAX_BASE_ITEMS];
static mitem_t psu_diag_sec_items[PSU_DIAG_TBL_SIZE + MAX_BASE_ITEMS];

static struct menuinfo psu_diag = {
    "%s Menu",               /* title */
    0,                       /* title string added by init_empty_menu */
    0,                       /* do not show major flags */
    0,                       /* generic prompt */
    0,                       /* size -- bumped by add_menu_item() */
    psu_diag_pri_items,
};

static struct menuinfo *psu_diag_p = &psu_diag;

/******************************************************************************
 * Function   : diag_poe_psu_test
 *
 * Description:	Function performs PoE PSU Diag tests or utilities.
 * Inputs     : opt - Option to determine to run Diag tests / show submenu
 * Outputs    : None
 ******************************************************************************/
void diag_poe_psu_test (int opt)
{
    /* Build Menu */
    build_primary_submenu(psu_diag_tbl, PSU_DIAG_TBL_SIZE,
			  "PoE PSU", &psu_diag_p);
    build_secondary_submenu(psu_diag_tbl, PSU_DIAG_TBL_SIZE,
			    psu_diag_sec_items);

    if (opt) {
        menu_exec_doall_diags(psu_diag_p);
    } else {
        /* Entered with submenu */
        menu(&psu_diag, psu_diag_sec_items, 0);
    }
}

/******************************************************************************
 * Function   : diag_psu_reg_test
 *
 * Description: Function performs PoE PSU register test.
 * Inputs     : opt - reserved for future use
 * Outputs    : PASSED/FAILED
 ******************************************************************************/
int diag_psu_reg_test (int opt)
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
    cterr_add_component("Marvell Armada 88F7040", "TI 2386B", "DDR RAM");

    /* Segment 5: register and memory dump */
    //cterr_add_reg_dump((PFV)bcm63168_get_xdsl_profile);

    /* Segment 6: Platform Environment initialized here*/
    //cterr_add_env_dump((PFV)show_margins_cterr_wrapper);

    /* Segment 7: Top 3 Debugging Steps */
    cterr_add_debug("Confirm related reigstess are read- and writeable by "
                    "using utility to access them manually. "
                    "Swap PoE module with Golden sample. "
                    "Check if I2C interface between CPU and PoE PSU controler"
                    " is good.");

    char * tname = "PoE Registers";
    testname(tname);
    prpass(testpass, "%s, ", tname);
    printf("\n");

    /* Sanity check */
    if (poe_obj_p->callin_fvt == NULL) {
        /* Create dev object */
        if (diag_poe_dev_create(poe_obj_p, poe_i2c_if_p) != PASSED) {
            printf("%s:%d: Failed to create TPS2386B device object\n",
                   __func__, __LINE__);
            return (FAILED);
        }
    }

    dev_object_t *dev = (dev_object_t *)poe_obj_p;

    /* call utility from driver */
    if (poe_obj_p->callin_fvt->register_test(dev) != PASSED) {
        printf("%s:%d: Failed at TPS2386B register read utility\n",
                __func__, __LINE__);
        return (FAILED);
    }

    return (PASSED);
}

/******************************************************************************
 * Function   : diag_psu_intr_test
 *
 * Description: Function performs PoE PSU interrupt test.
 * Inputs     : opt - reserved for future use
 * Outputs    : PASSED/FAILED
 ******************************************************************************/
static int diag_psu_intr_test (int opt)
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
    cterr_add_component("Marvell Armada 88F7040", "TI 2386B", "FPGA");

    /* Segment 5: register and memory dump */
    //cterr_add_reg_dump((PFV)bcm63168_get_xdsl_profile);

    /* Segment 6: Platform Environment initialized here*/
    //cterr_add_env_dump((PFV)show_margins_cterr_wrapper);

    /* Segment 7: Top 3 Debugging Steps */
    cterr_add_debug("Confirm related reigstess are read- and writeable by "
                    "using utility to access them manually. "
                    "Swap PoE module with Golden sample. "
                    "Check if I2C interface between CPU and PoE PSU controler"
                    " is good. "
                    "Check if interrupt path between FPGA and PoE PSU controler"
                    " is good.");

    char *tname = "PoE interrupt";
    testname(tname);
    prpass(testpass, "%s, ", tname);
    printf("\n");

    /* Sanity check */
    if (poe_obj_p->callin_fvt == NULL) {
        /* Create dev object */
        if (diag_poe_dev_create(poe_obj_p, poe_i2c_if_p) != PASSED) {
            cterr('f', 0, "%s:%d: Failed to create TPS2386B device object\n",
                           __FUNCTION__, __LINE__);
            return (FAILED);
        }
    }

    dev_object_t *dev = (dev_object_t *)poe_obj_p;

    /* call utility from driver */
    if (poe_obj_p->callin_fvt->interrupt_test(dev) != PASSED) {
        cterr('f', 0, "%s:%d: Failed at TPS2386B register read utility\n",
                       __FUNCTION__, __LINE__);
        return (FAILED);
    }

    return (PASSED);
}

/*-------------------------------------------------
 * $Log: diag_poe_psu_test.c,v $
 * Revision 1.2  2019/01/10 06:36:23  wilbhuan
 * The beginning of Betelgeuse application code.
 *
 *-------------------------------------------------
 */
