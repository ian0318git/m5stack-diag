/* $Id: diag_poe_psu_util.c,v 1.2 2021/09/24 01:21:06 harrchan Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/elixir/diag_poe_psu_util.c,v $
 *------------------------------------------------------------------
 * 
 * diag_poe_psu_util.c
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
#include "diag_i2c_lib.h"
#include "platform_i2c.h"
#include "diag_poe_psu_lib.h"
#include "diag_poe_psu_util.h"
#include "dev_tps2386b.h"
#include "platform_cookie.h"

/******************************************************************************
 *                          Global variables
 ******************************************************************************/
static dev_tps2386b_object_t poe_obj;
static dev_tps2386b_object_t *poe_obj_p = &poe_obj;
static n2g_i2c_if_t poe_i2c_if;
static n2g_i2c_if_t *poe_i2c_if_p = &poe_i2c_if;

/******************************************************************************
 *                          Function Declaration
 ******************************************************************************/

int poe_all_util(int);

/* PoE PSU Utilities */
static submenu_xtable_t psu_utils_tbl[] = {
    {"TPS2386B Detect Port Power",
     (type_t(*)())poe_all_util,
     UTIL_DET_PORT_PWR, 0, (type_t(*)())0, 0, (type_t(*)())0, 0},

    {"TPS2386B Register Read",
     (type_t(*)())poe_all_util,
     UTIL_REG_READ, 0, (type_t(*)())0, 0, (type_t(*)())0, 0},

    {"TPS2386B Register Write",
     (type_t(*)())poe_all_util,
     UTIL_REG_WRITE, 0, (type_t(*)())0, 0, (type_t(*)())0, 0},

    {"TPS2386B Register Dump",
     (type_t(*)())poe_all_util,
     UTIL_REG_DUMP, 0, (type_t(*)())0, 0, (type_t(*)())0, 0},

    {"TPS2386B Show Prot Status",
     (type_t(*)())poe_all_util,
     UTIL_SHOW_PORT_STAT, 0, (type_t(*)())0, 0, (type_t(*)())0, 0},
};

#define PSU_UTILS_TBL_SIZE (sizeof(psu_utils_tbl) / sizeof(submenu_xtable_t))

INFO_TB *util_name_tb[PSU_UTILS_TBL_SIZE] = {"Power Detect",
                                             "Register Read",
                                             "Register Write",
                                             "Register Dump",
                                             "Show Port Power Status"};

/* PoE PSU Utilities items (filled in from xtable) */
static mitem_t psu_utils_pri_items[PSU_UTILS_TBL_SIZE + MAX_BASE_ITEMS];
static mitem_t psu_utils_sec_items[PSU_UTILS_TBL_SIZE + MAX_BASE_ITEMS];

/* PoE PSU Utils submenu */
menuinfo_t psu_utils_menu = {
    "%s Menu",
    0,
    (PFT)show_endnote,
    0,
    0,
    psu_utils_pri_items,
};
menuinfo_t *psu_utils_menup = &psu_utils_menu;

/******************************************************************************
 * Function    : diag_poe_psu_util
 *
 * Description : Function to show PoE PSU utilities submenu.
 * Inputs      : opt - reserved for future use.
 * Outputs     : PASSED / FAILED
 ******************************************************************************/
int diag_poe_psu_util (int opt)
{
    build_primary_submenu(psu_utils_tbl, PSU_UTILS_TBL_SIZE,
                          "PoE PSU Utilities", &psu_utils_menup);
    build_secondary_submenu(psu_utils_tbl, PSU_UTILS_TBL_SIZE,
                            psu_utils_sec_items);

    menu(psu_utils_menup, psu_utils_sec_items, '\0');

    return (PASSED);
}

/******************************************************************************
 * Function   : poe_all_util
 *
 * Description:	Function to perform the following utilities:
 *              1. Detect port power
 *              2. Register read
 *              3. Register write
 *              4. Register dump
 *              5. Show port power status
 * Inputs     : util_item - use to select specific utility
 * Outputs    : PASSED/FAILED
 ******************************************************************************/
int poe_all_util (int util_item) 
{
    int rc = FAILED, start_port, end_port;

    if (CHK_VALID_UTIL(util_item) != TRUE) {
        printf("%s:%d:Invalid utility item\n", __func__, __LINE__);
    }

    start_port = TPS2386B_PORT1;
    end_port = TPS2386B_PORT4;

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
    switch (util_item) 
    {
        case UTIL_DET_PORT_PWR:
            rc = poe_obj_p->callin_fvt->util_detect_pwr(dev, 
                                                        start_port, 
                                                        end_port);
        break;

        case UTIL_REG_READ:  
            rc = poe_obj_p->callin_fvt->util_read_reg(dev);
        break;

        case UTIL_REG_WRITE:
            rc = poe_obj_p->callin_fvt->util_write_reg(dev);
        break;

        case UTIL_REG_DUMP:
            rc = poe_obj_p->callin_fvt->util_dump_register(dev);
        break;

        case UTIL_SHOW_PORT_STAT:
            rc = poe_obj_p->callin_fvt->util_show_pwr_stat(dev,
                                                           start_port, 
                                                           end_port);
        break;

        default:
            printf("%s:%d:Invalid utility item\n", __func__, __LINE__);
            return (rc);
        break;
    }

    if (rc != PASSED) {
        printf("%s:%d: Failed at TPS2386B %s utility\n",
                __func__, __LINE__, util_name_tb[util_item]);
        return (rc);
    }

    return (rc);
}

/*-------------------------------------------------
 * $Log: diag_poe_psu_util.c,v $
 * Revision 1.2  2021/09/24 01:21:06  harrchan
 * Collapse Elixir-branch to Main Trunk.
 *
 * Revision 1.1.2.2  2020/10/26 07:08:28  harrchan
 * 1.Changed PID table in platform_i2c.c
 * 2.Modify menu item to match up Elixir hardware design.
 *
 * Revision 1.1.2.1  2020/09/09 09:08:07  illiu
 * First version which has been ported with Dreamliner and Marvel CPSS
 *
 * Revision 1.2  2019/01/10 06:36:23  wilbhuan
 * The beginning of Betelgeuse application code.
 *
 *-------------------------------------------------
 */
