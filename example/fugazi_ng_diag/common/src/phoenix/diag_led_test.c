/* $Id: diag_led_test.c,v 1.2 2021/04/15 00:52:26 achiu2 Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/phoenix/diag_led_test.c,v $
 *------------------------------------------------------------------
 *
 * diag_led_test.c - This file is for LED test.
 *
 *
 * Copyright (c) 2008-2019 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "endians.h"
#include "defs.h"
#include "types.h"
#include "nvsysvars.h"
#include "error.h"
#include "pcmap.h"
#include "strings.h"
#include "menu.h"
#include "common.h"
#include "cross_platform.h"
#include "mb_tests.h"
#include "plat_defs.h"
#include "setjmps.h"
#include "proto.h"
#include "diag_fpga.h"
#include "plat_defs.h"
#include "dnv_eth_lib.h"
#include "diag_led_util.h"
#include "diag_led_test.h"
#include "diag_cpld_lib.h"
#include "sys/io.h"

extern int pwr_seq_pwr_led_green(void);
extern int pwr_seq_pwr_led_amber(void);
extern int pwr_seq_pwr_led_off(void);
extern int pwr_seq_pwr_led_reg_read(uint16_t *);
extern int pwr_seq_pwr_led_reg_write(uint16_t);

/*
 * Global variables
 */

/* Local functions */
int build_led_util_menu(boolean);
int diag_led_test(boolean);
int diag_psu_led_test(void);
int diag_pwr_led_test(void);
int diag_stat_led_test(void);
int diag_fan_led_test(void);
int diag_temp_led_test(void);
int diag_ssd_led_test(void);
int diag_console_led_test(void);
int diag_I350_RJ45_led_test(void);
int diag_port80_led_test(void);

/*
 * Sub Menu used for "Main menu -> LED test"
 */
submenu_xtable_t led_tests_submenu_table[] = {
    {"LED Utilities",
     (PFT) build_led_util_menu, FALSE, 0,
     (type_t(*)())0, 0, (PFT) build_led_util_menu, TRUE},

    {"PSU LED Test",
    (PFT) diag_psu_led_test, FALSE,
    MF_CONTINUOUS | MF_DOGRP | MF_DOALL,
    (type_t(*)())0, 0,
    (PFT) diag_psu_led_test, TRUE},

    {"PWR LED Test",
    (PFT) diag_pwr_led_test, FALSE,
    MF_CONTINUOUS | MF_DOGRP | MF_DOALL,
    (type_t(*)())0, 0,
    (PFT) diag_pwr_led_test, TRUE},

    {"STAT LED Test",
    (PFT) diag_stat_led_test, FALSE,
    MF_CONTINUOUS | MF_DOGRP | MF_DOALL,
    (type_t(*)())0, 0,
    (PFT) diag_stat_led_test, TRUE},

    {"FAN LED Test",
    (PFT) diag_fan_led_test, FALSE,
    MF_CONTINUOUS | MF_DOGRP | MF_DOALL,
    (type_t(*)())0, 0,
    (PFT) diag_fan_led_test, TRUE},

    {"TEMP LED Test",
    (PFT) diag_temp_led_test, FALSE,
    MF_CONTINUOUS | MF_DOGRP | MF_DOALL,
    (type_t(*)())0, 0,
    (PFT) diag_temp_led_test, TRUE},

    {"SSD LED Test",
    (PFT) diag_ssd_led_test, FALSE,
    MF_CONTINUOUS | MF_DOGRP | MF_DOALL,
    (type_t(*)())has_m2_device, 0,
    (PFT) diag_ssd_led_test, TRUE},

    {"Console LED Test",
    (PFT) diag_console_led_test, FALSE,
    MF_CONTINUOUS | MF_DOGRP | MF_DOALL,
    (type_t(*)())0, 0,
    (PFT) diag_console_led_test, TRUE},

    {"I350 RJ45 LED Test",
    (PFT) diag_I350_RJ45_led_test, FALSE,
    MF_CONTINUOUS | MF_DOGRP | MF_DOALL,
    (type_t(*)())0, 0,
    (PFT) diag_I350_RJ45_led_test, TRUE},

    {"Port80 LED Test",
    (PFT) diag_port80_led_test, FALSE,
    MF_CONTINUOUS | MF_DOGRP | MF_DOALL,
    (type_t(*)())0, 0,
    (PFT) diag_port80_led_test, TRUE},

};

#define LED_TESTS_SUBMENU_TABLE_SIZE (sizeof(led_tests_submenu_table) / \
                                     sizeof(submenu_xtable_t))

/*
 * "Main menu -> led test" primary & secondary submenu items (filled in from xtable)
 */
static mitem_t led_tests_primary_items[LED_TESTS_SUBMENU_TABLE_SIZE +
                                      MAX_BASE_ITEMS];
static mitem_t led_tests_secondary_items[LED_TESTS_SUBMENU_TABLE_SIZE +
                                        MAX_BASE_ITEMS];

menuinfo_t led_subtest_menu = {
    "%s Subtest Menu",
    0,                          /* mtparam added by init_empty_menu */
    (PFT) show_endnote,         /* notes missing WICs in combos */
    0,                          /* use generic prompt */
    0,                          /* size (bumped by add_menu_item() */
    led_tests_primary_items,
};

menuinfo_t *led_submenup = &led_subtest_menu;

/*
 * Sub Menu used for "LED test -> LED utility submenu"
 */

submenu_xtable_t led_util_submenu_table[] = {
    {"PSU LED Utility",
     (PFT) diag_psu_led_util, FALSE, 0,
     (type_t(*)())0, 0, (PFT) 0, 0},

    {"PWR LED Utility",
     (PFT) diag_pwr_led_util, FALSE, 0,
     (type_t(*)())0, 0, (PFT) 0, 0},

    {"STAT LED Utility",
     (PFT) diag_stat_led_util, FALSE, 0,
     (type_t(*)())0, 0, (PFT) 0, 0},

    {"FAN LED Utility",
     (PFT) diag_fan_led_util, FALSE, 0,
     (type_t(*)())0, 0, (PFT) 0, 0},

    {"TEMP LED Utility",
     (PFT) diag_temp_led_util, FALSE, 0,
     (type_t(*)())0, 0, (PFT) 0, 0},

    {"SSD LED Utility",
     (PFT) diag_ssd_led_util, FALSE, 0,
     (type_t(*)())has_m2_device, 0, (PFT) 0, 0},

    {"Console LED Utility",
     (PFT) diag_console_led_util, FALSE, 0,
     (type_t(*)())0, 0, (PFT) 0, 0},

    {"I350 RJ45 LED Utility",
     (PFT) diag_I350_RJ45_led_util, FALSE, 0,
     (type_t(*)())0, 0, (PFT) 0, 0},

    {"Port80 LED Utility",
     (PFT) diag_port80_led_util, FALSE, 0,
     (type_t(*)())0, 0, (PFT) 0, 0},

    {"Debug LED Utility",
     (PFT) diag_debug_led_util, FALSE, 0,
     (type_t(*)())0, 0, (PFT) 0, 0},

};

#define LED_UTIL_SUBMENU_TABLE_SIZE (sizeof(led_util_submenu_table) / \
                                     sizeof(submenu_xtable_t))

static mitem_t led_util_primary_items[LED_UTIL_SUBMENU_TABLE_SIZE +
                                      MAX_BASE_ITEMS];
static mitem_t led_util_secondary_items[LED_UTIL_SUBMENU_TABLE_SIZE +
                                        MAX_BASE_ITEMS];

menuinfo_t led_util_subtest_menu = {
    "%s Subtest Menu",
    0,                          /* mtparam added by init_empty_menu */
    (PFT) show_endnote,         /* notes missing WICs in combos */
    0,                          /* use generic prompt */
    0,                          /* size (bumped by add_menu_item() */
    led_util_primary_items,
};

menuinfo_t *led_util_submenup = &led_util_subtest_menu;

/*******************************************************************************
 *
 * Function   : build_led_util_menu
 * Description: build LED utility menu.
 * Inputs     : Test/Menu 
 * Outputs    : None
 *
 *******************************************************************************
 */
int build_led_util_menu (boolean mb_temp_test_items_executed)
{
    char *tname = "LED utility";
    testname(tname);

    build_primary_submenu(led_util_submenu_table, LED_UTIL_SUBMENU_TABLE_SIZE,
                          "LED util SubMenu", &led_util_submenup);
    build_secondary_submenu(led_util_submenu_table, LED_UTIL_SUBMENU_TABLE_SIZE,
                            led_util_secondary_items);
    menu(&led_util_subtest_menu, led_util_secondary_items, 0);
    return (PASSED);
}


/******************************************************************************
 *
 * Function: diag_led_test
 *
 * Description: Show led test submenu 
 *
 * Inputs      : None
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/
int diag_led_test (boolean led_test_items_executed)
{

    build_primary_submenu(led_tests_submenu_table,
                          LED_TESTS_SUBMENU_TABLE_SIZE, "LED",
                          &led_submenup);

    build_secondary_submenu(led_tests_submenu_table,
                            LED_TESTS_SUBMENU_TABLE_SIZE,
                            led_tests_secondary_items);

    if (led_test_items_executed) {
        do_all_menu_items(&led_subtest_menu);
    } else {
        menu(&led_subtest_menu, led_tests_secondary_items, '\0');
    }

    return (PASSED);
}


/******************************************************************************
 *
 * Function: diag_led_2color_test
 *
 * Description: 2-color LEDs test, behavior is as below.
 *              1. Save original setting.
 *              2. Turn on 1'st color led(s) only.
 *              3. Turn on 2'nd color led(s) only.
 *              4. Turn off all led(s).
 *              5. Retore original setting.
 *
 * Inputs      : led - which LED to test
 *
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/
static int diag_led_2color_test(int led)
{
    uint32_t orig_val = 0;
    uint offset = 0;
    uint mask = PHOENIX_LED_OFF;
    uint val_1 = 0, val_2 = 0, val_off = PHOENIX_LED_OFF;

    switch (led) {
        case E_PSU_LED_TEST:
            offset = FPGA_PSU_LED;
            val_1 = PWR_PSU_LED_GREEN;
            val_2 = PWR_PSU_LED_YELLOW;
            break;

        case E_FAN_LED_TEST:
            offset = FPGA_ENV_LED;
            mask = ~ENV_LED_FAN_MASK;
            val_1 = ENV_LED_FAN_GREEN;
            val_2 = ENV_LED_FAN_YELLOW;
            break;

        case E_TEMP_LED_TEST:
            offset = FPGA_ENV_LED;
            mask = ~ENV_LED_TEMP_MASK;
            val_1 = ENV_LED_TEMP_GREEN;
            val_2 = ENV_LED_TEMP_YELLOW;
            break;

        case E_SSD_LED_TEST:
            offset = FPGA_SSD_LED;
            mask = ~SSD_LED_MASK;
            val_1 = SSD_LED_GREEN;
            val_2 = SSD_LED_YELLOW;
            break;

        case E_CONSOLE_LED_TEST:
            offset = FPGA_CONSOLE_MULTIPLEXER;
            mask = ~CONSOLE_LED_MASK;
            val_1 = SERIAL_CONSOLE_LED_ON;
            val_2 = USB_CONSOLE_LED_ON;
            val_off = CONSOLE_LED_FORCE_MODE;
            break;

        case E_STAT_LED_TEST:
        default:
            cterr('t', 0, "%d:Unsupported LED test(%d).\n", __LINE__, led);
            return (FAILED);
    }

    /* Back up original data */
    if (fpga_read_reg(offset, &orig_val) != PASSED) {
        cterr('t', 0, "%d:Failed to write FPGA register.\n", __LINE__ );
        return (FAILED);
    }

    /* Turn on 1'st color LED(s) only */
    if (fpga_register_operation(offset, mask, val_1) != PASSED) {
        cterr('t', 0, "%d:Failed to write FPGA register.\n", __LINE__ );
        return (FAILED);
    }

    /* Turn on 2'nd color LED(s) only */
    if (fpga_register_operation(offset, mask, val_2) != PASSED) {
        cterr('t', 0, "%d:Failed to write FPGA register.\n", __LINE__ );
        return (FAILED);
    }

    /* Turn off both LEDs */
    if (fpga_register_operation(offset, mask, val_off) != PASSED) {
        cterr('t', 0, "%d:Failed to write FPGA register.\n", __LINE__ );
        return (FAILED);
    }

    /* Restore original data */
    if (fpga_write_reg(offset, orig_val) != PASSED) {
        cterr('t', 0, "%d:Failed to write FPGA register.\n", __LINE__ );
        return (FAILED);
    }

    return (PASSED);
}


/******************************************************************************
 *
 * Function: diag_psu_led_test
 *
 * Description: PSU LED test
 *
 * Inputs      : None
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/
int diag_psu_led_test(void)
{
    char *tname ="PSU LED Test";
    int ret;

    testname(tname);
    prpass(testpass, "%s, ", tname);

    ret = diag_led_2color_test(E_PSU_LED_TEST);

    if (ret == PASSED) {
        prcomplete(testpass, errcount, (char *)0);
    }
    return (ret);
}


/******************************************************************************
 *
 * Function: diag_fan_led_test
 *
 * Description: FAN LED test
 *
 * Inputs      : None
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/
int diag_fan_led_test (void)
{
    char *tname ="FAN LED Test";
    int ret;

    testname(tname);
    prpass(testpass, "%s, ", tname);

    ret = diag_led_2color_test(E_FAN_LED_TEST);

    if (ret == PASSED) {
        prcomplete(testpass, errcount, (char *)0);
    }
    return (ret);
}


/******************************************************************************
 *
 * Function: diag_temp_led_test
 *
 * Description: TEMP LED test
 *
 * Inputs      : None
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/
int diag_temp_led_test (void)
{
    char *tname ="TEMP LED Test";
    int ret;

    testname(tname);
    prpass(testpass, "%s, ", tname);

    ret = diag_led_2color_test(E_TEMP_LED_TEST);

    if (ret == PASSED) {
        prcomplete(testpass, errcount, (char *)0);
    }
    return (ret);
}


/******************************************************************************
 *
 * Function: diag_ssd_led_test
 *
 * Description: SSD LED test
 *
 * Inputs      : None
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/
int diag_ssd_led_test(void)
{
    char *tname ="SSD LED Test";
    int ret;

    testname(tname);
    prpass(testpass, "%s, ", tname);

    ret = diag_led_2color_test(E_SSD_LED_TEST);

    if (ret == PASSED) {
        prcomplete(testpass, errcount, (char *)0);
    }
    return (ret);
}


/******************************************************************************
 *
 * Function: diag_stat_led_test
 *
 * Description: System Status LED test
 *
 * Inputs      : None
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/
int diag_stat_led_test (void)
{
    char *tname ="STAT LED Test";
    uint32_t origin_val = 0;

    testname(tname);
    prpass(testpass, "%s, ", tname);
    
    /* Back up data*/
    if (cpld_read_reg(CPLD_STATUS_LED_REG, &origin_val) != PASSED) {
        cterr('t', 0, "%d:Failed to write CPLD register.\n", __LINE__ );
        return (FAILED);
    }

    /* LED Turn Green */
    if (cpld_register_operation(CPLD_STATUS_LED_REG, PHOENIX_LED_OFF, 
                                STATUS_LED_GREEN) != PASSED) {
        cterr('t', 0, "%d:Failed to write CPLD register.\n", __LINE__ );
        return (FAILED);
    }

    /* LED Turn Yellow */
    if (cpld_register_operation(CPLD_STATUS_LED_REG, PHOENIX_LED_OFF, 
                                STATUS_LED_YELLOW) != PASSED) {
        cterr('t', 0, "%d:Failed to write CPLD register.\n", __LINE__ );
        return (FAILED);
    }

    /* LED Turn OFF */
    if (cpld_register_operation(CPLD_STATUS_LED_REG, PHOENIX_LED_OFF, 
                                PHOENIX_LED_OFF) != PASSED) {
        cterr('t', 0, "%d:Failed to write CPLD register.\n", __LINE__ );
        return (FAILED);
    }

    /* Restore Data */
    if (cpld_write_reg(CPLD_STATUS_LED_REG, origin_val) != PASSED) {
        cterr('t', 0, "%d:Failed to write CPLD register.\n", __LINE__ );
        return (FAILED);
    }

    prcomplete(testpass, errcount, (char *)0);
    return (PASSED);
}


/******************************************************************************
 *
 * Function: diag_I350_RJ45_led_test
 *
 * Description: I350 RJ45 LED test
 *
 * Inputs      : None
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/
int diag_I350_RJ45_led_test (void)
{
    char *tname ="I350 RJ45 LED Test";

    testname(tname);
    prpass(testpass, "%s, ", tname);

    /* I350 RJ45 LED Turn Green */
    if (fpga_register_operation(FPGA_I350_RJ45_LED, PHOENIX_LED_OFF, 
                                RJ45_LED_ON) != PASSED) {
        cterr('t', 0, "%d:Failed to write FPGA register.\n", __LINE__ );
        return (FAILED);
    }

    /* I350 RJ45 LED Turn OFF */
    if (fpga_register_operation(FPGA_I350_RJ45_LED, PHOENIX_LED_OFF, 
                                PHOENIX_LED_OFF) != PASSED) {
        cterr('t', 0, "%d:Failed to write FPGA register.\n", __LINE__ );
        return (FAILED);
    }

    prcomplete(testpass, errcount, (char *)0);
    return (PASSED);
}


/******************************************************************************
 *
 * Function: diag_port80_led_test
 *
 * Description: Port80 LED test
 *
 * Inputs      : None
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/
int diag_port80_led_test (void)
{
    char *tname ="Port80 LED Test";
    uint32_t reg_val = 0;
    int port80_bit;

    testname(tname);
    prpass(testpass, "%s, ", tname);
    
    /* Set read/write permission start from Port80 on, only one port*/
    if (ioperm(PORT80_ADDR, ACCESS_PORT_NUM, PORT80_ACCESS_ON) < 0) {
        cterr('f', 0, "Failed to set access of Port80 on.");
        return (FAILED);
    }

    for (port80_bit = PORT80_BIT0; port80_bit <= PORT80_BIT7; port80_bit++) {
        /* Turn on Port80 led with green*/
        reg_val = inb(PORT80_ADDR);
        outb(reg_val | (PORT80_LED_ON << port80_bit), PORT80_ADDR);
        msleep(DELAY_FOR_LED_TEST);
        
        /* Turn off Port80 led */
        reg_val = inb(PORT80_ADDR);
        outb(reg_val & PORT80_LED_ALL_OFF, PORT80_ADDR);
        msleep(DELAY_FOR_LED_TEST);
    }
    
    /* Set read/write permission start from Port80 off, only one port*/
    if (ioperm(PORT80_ADDR, ACCESS_PORT_NUM, PORT80_ACCESS_OFF) < 0) {
        cterr('f', 0, "Failed to set access of Port80 off");
        return (FAILED);
    }

    prcomplete(testpass, errcount, (char *)0);
    return (PASSED);
}


/******************************************************************************
 *
 * Function: diag_pwr_led_test
 *
 * Description: PWR LED test
 *
 * Inputs      : None
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/
int diag_pwr_led_test(void)
{
    char *tname ="PWR LED Test";
    uint16_t data_orig;

    testname(tname);
    prpass(testpass, "%s, ", tname);

    /* Back up data */
    if (PASSED != pwr_seq_pwr_led_reg_read(&data_orig)) {
        printf("%s:%d: Failed to access MCU register!!\n", __func__, __LINE__);
        return (FAILED);
    }

    /* Turn on green LED */
    if (PASSED != pwr_seq_pwr_led_green()) {
        printf("%s:%d: Failed to access MCU register!!\n", __func__, __LINE__);
        return (FAILED);
    }
    msleep(DELAY_FOR_OPERATION);

    /* Turn on amber LED */
    if (PASSED != pwr_seq_pwr_led_amber()) {
        printf("%s:%d: Failed to access MCU register!!\n", __func__, __LINE__);
        return (FAILED);
    }
    msleep(DELAY_FOR_OPERATION);

    /* Turn off LED */
    if (PASSED != pwr_seq_pwr_led_off()) {
        printf("%s:%d: Failed to access MCU register!!\n", __func__, __LINE__);
        return (FAILED);
    }
    msleep(DELAY_FOR_OPERATION);

    /* Restore data */
    if (PASSED != pwr_seq_pwr_led_reg_write(data_orig)) {
        printf("%s:%d: Failed to access MCU register!!\n", __func__, __LINE__);
        return (FAILED);
    }

    prcomplete(testpass, errcount, (char *)0);

    return (PASSED);
}


/******************************************************************************
 *
 * Function: diag_console_led_test
 *
 * Description: console LED test
 *
 * Inputs      : None
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/
int diag_console_led_test (void)
{
    char *tname ="Console LED Test";
    int ret;

    testname(tname);
    prpass(testpass, "%s, ", tname);

    ret = diag_led_2color_test(E_CONSOLE_LED_TEST);

    if (ret == PASSED) {
        prcomplete(testpass, errcount, (char *)0);
    }
    return (ret);
}
