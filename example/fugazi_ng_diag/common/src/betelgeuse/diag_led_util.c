/* $Id: diag_led_util.c,v 1.2 2019/01/10 06:36:23 wilbhuan Exp $ 
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/betelgeuse/diag_led_util.c,v $
 *------------------------------------------------------------------
 *
 * diag_led_util.c
 *
 * Copyright (c) 2018 - 2019 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------------------
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include "common.h"
#include "error.h"
#include "queryflags.h"
#include "nvsysvars.h"
#include "menu.h"
#include "diag_moka_fpga_lib.h"
#include "diag_sirius_fpga_lib.h"
#include "platform_cookie.h"
#include "diag_ge_phy_lib.h"
#include "diag_esw_lib.h"
#include "diag_moka_fpga_util.h"
#include "diag_led_util.h"


/*******************************************************************************
 *                          Function Declaration
 *******************************************************************************
 */
void diag_led_util(void);

/*******************************************************************************
 *                                   Menus                                     
 *******************************************************************************
 */

/* LED Utility Menu */
static submenu_xtable_t led_util_tbl[] = {
    {"FPGA register Read Util",    (PFT)fpga_reg_rd_util,          0,
     0, 
     (type_t(*)())0,               0,
     (type_t(*)())0,               0},
    {"FPGA register Write Util",   (PFT)fpga_reg_wr_util,          0,
     0, 
     (type_t(*)())0,               0,
     (type_t(*)())0,               0},
    {"Turn all Green LED ON",      (PFT)diag_led_all_green_on_util,    0,
     0, 
     (type_t(*)())0,               0,
     (type_t(*)())0,               0},
    {"Turn all Yellow LED ON",     (PFT)diag_led_all_yellow_on_util,   0,
     0, 
     (type_t(*)())0,               0,
     (type_t(*)())0,               0},
    {"Turn all LED OFF",           (PFT)diag_led_all_off_util,         0,
     0, 
     (type_t(*)())0,               0,
     (type_t(*)())0,               0},
};

#define LED_UTIL_TBL_SIZE (sizeof(led_util_tbl) / sizeof(submenu_xtable_t))

/*
 * primary & secondary submenu items (filled in from xtable)
 */
static mitem_t led_util_menu_pri_items[LED_UTIL_TBL_SIZE + MAX_BASE_ITEMS];
static mitem_t led_util_menu_sec_items[LED_UTIL_TBL_SIZE + MAX_BASE_ITEMS];

static menuinfo_t led_util_menu = {
    "%s Utility Menu",
    0,                            /* mtparam added by init_empty_menu */
    (PFT)menu_show_dflags,        /* shows major flags */
    0,                            /* use generic prompt */
    0,                            /* size (bumped by add_menu_item() */
    led_util_menu_pri_items,
};
static menuinfo_t *led_util_menu_p = &led_util_menu;


/*******************************************************************************
 *
 * Function   : diag_led_util
 * Description: Entry function of LED Diag Utilities.
 * Inputs     : None
 * Outputs    : None
 *
 *******************************************************************************
 */
void diag_led_util (void)
{
    build_primary_submenu(led_util_tbl, LED_UTIL_TBL_SIZE,
                          "LED", &led_util_menu_p);
    build_secondary_submenu(led_util_tbl, LED_UTIL_TBL_SIZE,
                            led_util_menu_sec_items);

    menu(&led_util_menu, led_util_menu_sec_items, 0);
}

/*******************************************************************************
 *
 * Function    : diag_led_all_green_on_util
 * Description : Function to turn all Green LEDs ON.
 * Inputs      : None
 * Outputs     : PASSED / FAILED
 *
 *******************************************************************************
 */
int diag_led_all_green_on_util (void)
{
    uint reg_offset = 0, reg_val = 0;

    /* Turn all LEDs OFF */
    if (diag_led_all_off_util() != PASSED) {
        printf("%s: Failed to turn all LEDs OFF.\n", __func__);
        return (FAILED);
    }

    reg_offset = (uint)FPGA_LED_REG;
    reg_val = (uint)(POE_PRESENT_LED_G | 
                     POE_STAT_LED |
                     MICRO_USB_LED | 
                     USB_LED | 
                     VPN_OK_LED | 
                     CONSOLE_LED);
    if (fpga_write_32_reg(reg_offset, reg_val) != PASSED) {
        printf("%s: Failed to write FPGA Reg0x%#X.\n", __func__, reg_offset);
    }


    /* Turn Status LED to Green */
    reg_offset = (uint)FPGA_LPC_STAT_LED_CTRL_REG;
    reg_val = STAT_LED_G;
    if (fpga_write_32_reg(reg_offset, reg_val) != PASSED) {
        printf("%s: Failed to write FPGA Reg0x%#X.\n", __func__, reg_offset);
    }

    if (diag_ge_phy_all_led_on(PLAT_GE0_ETHNUM) != PASSED) {
        printf("%s: Failed to turn all GE0 LEDs ON.\n", __func__);
    }

    if (platform_has_2nd_ge() == TRUE) {
        if (platform_has_xdsl() == FALSE) { 
            if (diag_ge_phy_all_led_on(PLAT_GE1_ETHNUM) != PASSED) {
                printf("%s: Failed to turn all GE1 LEDs ON.\n", __func__);
            }
        }
    }

    if (diag_esw_force_led_onoff((int)ALL_ESW_LEDS,
                                (boolean)ESW_LED_F_ON) != PASSED) {
            printf("%s: Failed to turn all ESW LEDs ON.\n", __func__);
    }

    /* If this is Star with pluggable slot, turn on pluggable FPGA debug LEDs */
    if (platform_has_pluggable()) {
        reg_offset = (uint) PLUG_FPGA_DBG_LED_ADDR_REG;
        reg_val = 0x00;
        reg_val = PLUG_DBG_LED_ON;
        if (fpga_write_32_reg(reg_offset, reg_val) != PASSED) {
            printf("%s: Failed to write FPGA Reg0x%#X.\n", __func__, reg_offset);
        }
    }

    return (PASSED);
}

/*******************************************************************************
 *
 * Function    : diag_led_all_yellow_on_util
 * Description : Function to turn all Yellow LEDs ON.
 * Inputs      : None
 * Outputs     : PASSED / FAILED
 *
 *******************************************************************************
 */
int diag_led_all_yellow_on_util (void)
{
    uint reg_offset = 0, reg_val = 0;

    /* Turn all LEDs OFF */
    if (diag_led_all_off_util() != PASSED) {
        printf("%s: Failed to turn all LEDs OFF.\n", __func__);
        return (FAILED);
    }

    if (platform_has_2nd_ge() == TRUE) {
        if (platform_has_xdsl() == FALSE) { 
            if (diag_ge_phy_all_led_off(PLAT_GE1_ETHNUM) != PASSED) {
                printf("%s: Failed to turn all GE1 LEDs ON.\n", __func__);
            }
        }
    }

    /* Turn Status LED to Yellow */
    reg_offset = (uint)FPGA_LPC_STAT_LED_CTRL_REG;
    reg_val = STAT_LED_Y;
    if (fpga_write_32_reg(reg_offset, reg_val) != PASSED) {
        printf("%s: Failed to write FPGA Reg0x%#X.\n", __func__, reg_offset);
    }

    if (diag_ge_phy_all_led_off(PLAT_GE0_ETHNUM) != PASSED) {
        printf("%s: Failed to turn all GE0 LEDs ON.\n", __func__);
    }

    reg_offset = (uint)FPGA_LED_REG;
    reg_val = (uint)(
                     POE_PRESENT_LED_Y | 
                     POE_P0_LED |
                     POE_P1_LED | 
                     POE_P2_LED | 
                     POE_P3_LED | 
                     AUX_LED
                     );
    if (fpga_write_32_reg(reg_offset, reg_val) != PASSED) {
        printf("%s: Failed to write FPGA Reg0x%#X.\n", __func__, reg_offset);
    }

    return (PASSED);
}

/*******************************************************************************
 *
 * Function    : diag_led_all_off_util
 * Description : Function to turn all LEDs OFF.
 * Inputs      : None
 * Outputs     : PASSED / FAILED
 *
 *******************************************************************************
 */
int diag_led_all_off_util (void)
{
    uint reg_offset = 0, reg_val = 0;

    reg_offset = (uint)FPGA_LED_REG;
    reg_val = 0x000000;
    if (fpga_write_32_reg(reg_offset, reg_val) != PASSED) {
        printf("%s: Failed to write FPGA Reg0x%#X.\n", __func__, reg_offset);
    }

    /* Turn Power OK and Status LED off */
    reg_offset = (uint)FPGA_LPC_STAT_LED_CTRL_REG;
    reg_val = (uint)(PWR_OK_LED_OFF | STAT_LED_OFF);
    if (fpga_write_32_reg(reg_offset, reg_val) != PASSED) {
        printf("%s: Failed to write FPGA Reg0x%#X.\n", __func__, reg_offset);
    }

    if (diag_ge_phy_all_led_off(PLAT_GE0_ETHNUM) != PASSED) {
        printf("%s: Failed to turn all GE0 LEDs OFF.\n", __func__);
    }

    if (platform_has_2nd_ge() == TRUE) {
        if (platform_has_xdsl() == FALSE) { 
            if (diag_ge_phy_all_led_off(PLAT_GE1_ETHNUM) != PASSED) {
                printf("%s: Failed to turn all GE1 LEDs OFF.\n", __func__);
            }
        }
    }

    if (diag_esw_force_led_onoff((int)ALL_ESW_LEDS,
                                (boolean)ESW_LED_F_OFF) != PASSED) {
            printf("%s: Failed to turn all ESW LEDs OFF.\n", __func__);
    }

    /* If this sku with pluggable slot, turn off pluggable FPGA debug LEDs */
    if (platform_has_pluggable()) {
        reg_offset = (uint) PLUG_FPGA_DBG_LED_ADDR_REG;
        reg_val = PLUG_DBG_LED_OFF;
        if (fpga_write_32_reg(reg_offset, reg_val) != PASSED) {
            printf("%s: Failed to write FPGA Reg0x%#X.\n", __func__, reg_offset);
        }
    }

    return (PASSED);
}

/*-------------------------------------------------
 * $Log: diag_led_util.c,v $
 * Revision 1.2  2019/01/10 06:36:23  wilbhuan
 * The beginning of Betelgeuse application code.
 *
 *-------------------------------------------------
 */
