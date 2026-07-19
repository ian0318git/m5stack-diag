/* $Id: diag_rtc_util.c,v 1.2 2019/01/10 06:36:24 wilbhuan Exp $ 
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/betelgeuse/diag_rtc_util.c,v $
 *------------------------------------------------------------------
 * 
 * diag_rtc_util.c
 *
 * Copyright (c) 2018 - 2019 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#include "common.h"
#include "types.h"
#include "menu.h"
#include "dev_1337.h"
#include "diag_rtc_lib.h"
#include "diag_rtc_util.h"


/*******************************************************************************
 *                          Function Declaration
 *******************************************************************************
 */
void build_rtc_utils_menu(void);
int display_rtc_date_time_util(void);
int set_rtc_date_time_util(void);
int alter_rtc_reg_util(void);
int dump_rtc_regs_util(void);


/*******************************************************************************
 *                                  Menu
 *******************************************************************************
 */
/*
 * RTC utils submenu
 */
static submenu_xtable_t rtc_util_tbl[] = {
    {"Display RTC Date/Time", (PFT)display_rtc_date_time_util, 0,
     0,                       (type_t(*)())0,                  0, 
     (type_t(*)())0,          0},
    {"Set RTC Date/Time",     (PFT)set_rtc_date_time_util,     0,
     0,                       (type_t(*)())0,                  0, 
     (type_t(*)())0,          0},
    {"Alter RTC Register",    (PFT)alter_rtc_reg_util,         0,
     0,                       (type_t(*)())0,                  0, 
     (type_t(*)())0,          0},
    {"Dump RTC Registers",    (PFT)dump_rtc_regs_util,         0,
     0,                       (type_t(*)())0,                  0, 
     (type_t(*)())0,          0},
};

#define RTC_UTIL_TBL_SIZE (sizeof(rtc_util_tbl) / sizeof(submenu_xtable_t))

/*
 * Primary & secondary submenu items (filled in from xtable)
 */
static mitem_t rtc_util_pri_items[RTC_UTIL_TBL_SIZE + MAX_BASE_ITEMS];
static mitem_t rtc_util_sec_items[RTC_UTIL_TBL_SIZE + MAX_BASE_ITEMS];

static struct menuinfo rtc_utilmenu = {
    "RTC Utility Menu",        /* title */
    0,                         /* title string added by init_empty_menu */
    (PFT)menu_show_dflags,     /* shows major flags */
    0,                         /* generic prompt */
    0,                         /* size -- bumped by add_menu_item() */
    rtc_util_pri_items,
};

static struct menuinfo *rtc_utilmenu_p = &rtc_utilmenu;


/*******************************************************************************
 *
 * Function   : build_rtc_utils_menu
 * Description: Entry of RTC, Maxim ds1337, chip utilities.
 * Inputs     : None
 * Outputs    : None
 *
 *******************************************************************************
 */
void build_rtc_utils_menu (void)
{

    build_primary_submenu(rtc_util_tbl, RTC_UTIL_TBL_SIZE,
                          "RTC Utility Menu", &rtc_utilmenu_p);
    build_secondary_submenu(rtc_util_tbl, RTC_UTIL_TBL_SIZE,
                            rtc_util_sec_items);

    menu(rtc_utilmenu_p, rtc_util_sec_items, '\0' );
}

/*******************************************************************************
 *
 * Function   : display_rtc_date_time_util
 * Description:	Function to display RTC, Maxim ds1337, date/time.
 * Inputs     : None
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
int display_rtc_date_time_util (void)
{
    dev_ds1337_object_t rtc_obj;
    dev_ds1337_object_t *rtc_obj_p = &rtc_obj;
    dev_object_t *dev = (dev_object_t *)rtc_obj_p;
    n2g_i2c_if_t rtc_i2c_if;
    int ret_val = FAILED;

    /* Create RTC device object for test */
    if (diag_rtc_dev_create(rtc_obj_p, &rtc_i2c_if) != PASSED) {
        printf("%s: Failed to create RTC, Maxim ds1337, device object.\n",
               __func__);
        return (FAILED);
    }

    /* Execute display RTC Date/Time */
    ret_val = rtc_obj_p->callin_fvt->display_rtc(dev);
    if (ret_val != PASSED) {
        printf("%s: Failed to display RTC date/time.\n", __func__);
    }

    /* Destroy the created RTC device object after test */
    rtc_obj_p->base.dev_object_fvt->dev_destroy((dev_object_t **)&rtc_obj_p);

    return (ret_val);
}


/*******************************************************************************
 *
 * Function   : set_rtc_date_time_util
 * Description:	Function to set RTC, Maxim ds1337, date/time.
 * Inputs     : None
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
int set_rtc_date_time_util (void)
{
    dev_ds1337_object_t rtc_obj;
    dev_ds1337_object_t *rtc_obj_p = &rtc_obj;
    dev_object_t *dev = (dev_object_t *)rtc_obj_p;
    n2g_i2c_if_t rtc_i2c_if;
    int ret_val = FAILED;

    /* Create RTC device object for test */
    if (diag_rtc_dev_create(rtc_obj_p, &rtc_i2c_if) != PASSED) {
        printf("%s: Failed to create RTC, Maxim ds1337, device object.\n",
               __func__);
        return (FAILED);
    }

    /* Execute set RTC Date/Time */
    ret_val = rtc_obj_p->callin_fvt->set_rtc(dev);
    if (ret_val != PASSED) {
        printf("%s: Failed to set RTC date/time.\n", __func__);
    }

    /* Destroy the created RTC device object after test */
    rtc_obj_p->base.dev_object_fvt->dev_destroy((dev_object_t **)&rtc_obj_p);

    return (ret_val);
}

/*******************************************************************************
 *
 * Function   : alter_rtc_reg_util
 * Description: Function to alter RTC, Maxim ds1337, chip register.
 * Inputs     : None
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
int alter_rtc_reg_util (void)
{
    dev_ds1337_object_t rtc_obj;
    dev_ds1337_object_t *rtc_obj_p = &rtc_obj;
    dev_object_t *dev = (dev_object_t *)rtc_obj_p;
    n2g_i2c_if_t rtc_i2c_if;
    int ret_val = FAILED;

    /* Create RTC device object for test */
    if (diag_rtc_dev_create(rtc_obj_p, &rtc_i2c_if) != PASSED) {
        printf("%s: Failed to create RTC, Maxim ds1337, device object.\n",
               __func__);
        return (FAILED);
    }

    /* Execute alter RTC register */
    ret_val = rtc_obj_p->callin_fvt->peek_n_poke(dev, (print_fn_t)&printf);
    if (ret_val != PASSED) {
        printf("%s: Failed to alter RTC register.\n", __func__);
    }

    /* Destroy the created RTC device object after test */
    rtc_obj_p->base.dev_object_fvt->dev_destroy((dev_object_t **)&rtc_obj_p);

    return (ret_val);
}

/*******************************************************************************
 *
 * Function   : dump_rtc_regs_util
 * Description: Function to dump RTC, Maxim ds1337, chip all registers.
 * Inputs     : None
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
int dump_rtc_regs_util (void)
{
    dev_ds1337_object_t rtc_obj;
    dev_ds1337_object_t *rtc_obj_p = &rtc_obj;
    dev_object_t *dev = (dev_object_t *)rtc_obj_p;
    n2g_i2c_if_t rtc_i2c_if;
    int ret_val = FAILED;

    /* Create RTC device object for test */
    if (diag_rtc_dev_create(rtc_obj_p, &rtc_i2c_if) != PASSED) {
        printf("%s: Failed to create RTC, Maxim ds1337, device object.\n",
               __func__);
        return (FAILED);
    }

    /* Execute alter RTC register */
    ret_val = rtc_obj_p->base.dev_object_fvt->dev_show(dev, (print_fn_t)&printf,
                                                       DEV_SHOW_REGISTERS);
    if (ret_val != PASSED) {
        printf("%s: Failed to dump RTC all registers.\n", __func__);
    }

    /* Destroy the created RTC device object after test */
    rtc_obj_p->base.dev_object_fvt->dev_destroy((dev_object_t **)&rtc_obj_p);

    return (ret_val);
}


/*-------------------------------------------------
 * $Log: diag_rtc_util.c,v $
 * Revision 1.2  2019/01/10 06:36:24  wilbhuan
 * The beginning of Betelgeuse application code.
 *
 *-------------------------------------------------
 */
