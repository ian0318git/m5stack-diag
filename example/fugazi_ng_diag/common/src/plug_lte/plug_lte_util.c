/* $Id: plug_lte_util.c,v 1.11 2020/01/17 03:06:05 sherliu2 Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/plug_lte/plug_lte_util.c,v $
 *------------------------------------------------------------------
 *
 * plug_lte_util.c - PLUGGABLE LTE Utility
 *
 * Copyright (c) 2015 - 2019 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <fcntl.h>
#include "common.h"
#include "types.h"
#include "error.h"
#include "queryflags.h"
#include "menu.h"
#include "proto.h"
#include "plug_slot.h"
#include "plug_gpio_exp_test.h"
#include "plug_gpio_exp_lib.h"
#include "plug_temp_sensor_test.h"
#include "plug_temp_sensor_lib.h"
#include "plug_lte_util.h"
#include "plug_lte_lib.h"
#include "plug_lte_at.h"
#include "plug_lte_test.h"

static int plug_lte_run_at_cmd(int);
static int plug_lte_show_temp(int);
static int plug_lte_ts_util(int);
static int plug_lte_man_gpio_exp_util(void);
static int plug_lte_opt_gpio_exp_util(void);
static int plug_lte_led_util(int);
static int plug_lte_ext_usb_util(int);
static int plug_lte_modem_hd_reset(int);
static int plug_lte_modem_soft_reset_util(int);
static int plug_lte_led_ctrl_util(int);
static int plug_lte_enable_led_utils(int);
static int plug_lte_sim_status_led_utils(int);
static int plug_lte_gps_status_led_utils(int);
static int plug_lte_rssi_led_utils(int);
static int plug_lte_simdetect_pin_test(int);
static int lte_simdetect_pin_test(int, boolean, boolean);
static int plug_lte_show_simdetect_pin_status(int);
static int plug_lte_shutdown_modem_utils(int);

int plug_lte_util(void);
int plug_lte_modem_temp_util(int);

static submenu_xtable_t pluggable_lte_utils[] = {
    {"AT Command Utility", (type_t(*)())plug_lte_run_at_cmd, TRUE,
     0, (type_t(*)())0, 0, (type_t(*)())0, 0},
    {"Mandatory PCA9557 (0x4E) Register Read/Write Utility", (type_t(*)())plug_lte_man_gpio_exp_util, TRUE,
     0, (type_t(*)())0, 0, (type_t(*)())0, 0},
    {"Optional PCA9557 (0x4C) Register Read/Write Utility", (type_t(*)())plug_lte_opt_gpio_exp_util, TRUE,
     0, (type_t(*)())0, 0, (type_t(*)())0, 0},
    {"External USB Debug Enable/Disable Utility", (type_t(*)())plug_lte_ext_usb_util, TRUE,
     0, (type_t(*)())0, 0, (type_t(*)())0, 0},
    {"LEDs All On/Off Utility", (type_t(*)())plug_lte_led_util, TRUE,
     0, (type_t(*)())0, 0, (type_t(*)())0, 0},
    {"Temperature Display via Temperature Sensor Utility", (type_t(*)())plug_lte_show_temp, TRUE,
     0, (type_t(*)())plug_lte_has_temp_sensor, 0, (type_t(*)())0, 0},
    {"Temperature Sensor Register Read/Write Utility", (type_t(*)())plug_lte_ts_util, TRUE,
     0, (type_t(*)())plug_lte_has_temp_sensor, 0, (type_t(*)())0, 0},
    {"LTE Modem Temperature Display Utility", (type_t(*)())plug_lte_modem_temp_util, TRUE,
     0, (type_t(*)())0, 0, (type_t(*)())0, 0},
    {"LTE Modem Hard Reset Utility(Emergency use)", (type_t(*)())plug_lte_modem_hd_reset, TRUE,
     0, (type_t(*)())0, 0, (type_t(*)())0, 0},
    {"LTE Modem Soft Reset Utility", (type_t(*)())plug_lte_modem_soft_reset_util, TRUE,
     0, (type_t(*)())0, 0, (type_t(*)())0, 0},
    {"LED Control Utilities", (type_t(*)())plug_lte_led_ctrl_util, TRUE,
     0, (type_t(*)())0, 0, (type_t(*)())0, 0},
    {"LTE Modem SIM_DETECT pin Test(SIM0)", (type_t(*)())plug_lte_simdetect_pin_test, SIM0,
     0, (type_t(*)())0, 0, (type_t(*)())0, 0},
    {"LTE Modem SIM_DETECT pin Test(SIM1)", (type_t(*)())plug_lte_simdetect_pin_test, SIM1,
     0, (type_t(*)())plug_lte_has_2_sim_slot, 0, (type_t(*)())0, 0},
    {"Show LTE Modem SIM_DETECT pin Status(SIM0)", (type_t(*)())plug_lte_show_simdetect_pin_status, SIM0,
     0, (type_t(*)())0, 0, (type_t(*)())0, 0},
    {"Show LTE Modem SIM_DETECT pin Status(SIM1)", (type_t(*)())plug_lte_show_simdetect_pin_status, SIM1,
     0, (type_t(*)())plug_lte_has_2_sim_slot, 0, (type_t(*)())0, 0},
    {"Shutdown modem", (type_t(*)())plug_lte_shutdown_modem_utils, TRUE,
     0, (type_t(*)())0, 0, (type_t(*)())0, 0}
};

#define PLUG_LTE_UTIL_TABLE_SZ \
        (sizeof(pluggable_lte_utils) / sizeof(submenu_xtable_t))


static mitem_t plug_lte_pri_util_items[PLUG_LTE_UTIL_TABLE_SZ+ MAX_BASE_ITEMS];
static mitem_t plug_lte_sec_util_items[PLUG_LTE_UTIL_TABLE_SZ+ MAX_BASE_ITEMS];

static menuinfo_t plug_lte_util_menu = {
    "Pluggable LTE Utilities Menu",
    0,                            /* mtparam added by init_empty_menu */
    (PFT)menu_show_dflags,        /* shows major flags */
    0,                            /* use generic prompt */
    0,                            /* size (bumped by add_menu_item() */
    plug_lte_pri_util_items,
};
static menuinfo_t *plug_lte_util_menup = &plug_lte_util_menu;

/*
 * LED Control 
 */
static submenu_xtable_t plug_lte_led_ctrl_tbl[] = {
    {"Enable LED utils", (type_t(*)())plug_lte_enable_led_utils, 
      TRUE, 0, (type_t(*)())0, 0, (type_t(*)())0, 0},
    {"SIM Status LED utils", (type_t(*)())plug_lte_sim_status_led_utils, 
      TRUE, 0, (type_t(*)())0, 0, (type_t(*)())0, 0},
    {"GPS Status LED utils", (type_t(*)())plug_lte_gps_status_led_utils, 
      TRUE, 0, (type_t(*)())0, 0, (type_t(*)())0, 0},
    {"RSSI Status LED utils", (type_t(*)())plug_lte_rssi_led_utils, 
      TRUE, 0, (type_t(*)())0, 0, (type_t(*)())0, 0},
};
    
#define PLUG_LTE_LED_CTRL_TBL_SIZE (sizeof(plug_lte_led_ctrl_tbl) / sizeof(submenu_xtable_t))

/* LED Control Utils items (filled in from xtable) */
static mitem_t plug_lte_led_ctrl_pri_items[PLUG_LTE_LED_CTRL_TBL_SIZE + 
                                           MAX_BASE_ITEMS];
static mitem_t plug_lte_led_ctrl_sec_items[PLUG_LTE_LED_CTRL_TBL_SIZE + 
                                           MAX_BASE_ITEMS];

/* LED Control Utils submenu */
menuinfo_t plug_lte_led_ctrl_menu = {
    "%s Menu",
    0,
    (PFT)show_endnote,
    0,
    0,
    plug_lte_led_ctrl_pri_items,
};
menuinfo_t *plug_lte_led_ctrl_menup = &plug_lte_led_ctrl_menu;

static plug_lte_led_dev plug_lte_led_g_table[] = {
       {ENABLE_LED_GREEN , HIGH, LOW},
       {LED_SIM0_OK , HIGH, LOW},
       {LED_SIM1_OK , HIGH, LOW},
       {LED_GPS_OK , HIGH, LOW}
};
static plug_lte_led_dev plug_lte_led_y_table[] = {
       {ENABLE_LED_YELLOW , HIGH, LOW},
       {LED_SIM0_NOT_OK , HIGH, LOW},
       {LED_SIM1_NOT_OK , HIGH, LOW},
       {LED_GPS_NOT_OK , HIGH, LOW}
};
static plug_lte_led_dev plug_lte_led_rssi_table[] = {
       {LED_RSSI0 , HIGH, LOW},
       {LED_RSSI1 , HIGH, LOW},
       {LED_RSSI2 , HIGH, LOW},
       {LED_RSSI3 , HIGH, LOW}
};

int plug_lte_g_siz = sizeof(plug_lte_led_g_table)/sizeof(plug_lte_led_dev);
int plug_lte_y_siz = sizeof(plug_lte_led_y_table)/sizeof(plug_lte_led_dev);
int plug_lte_rssi_siz = sizeof(plug_lte_led_rssi_table)/sizeof(plug_lte_led_dev);

/*******************************************************************************
 * Function   : plug_lte_util
 * Description: Main Entry point for Pluggable LTE Utilities
 * Inputs     : None
 * Outputs    : PASSED or FAILED
 *******************************************************************************
 */
int plug_lte_util (void)
{   
    build_primary_submenu(pluggable_lte_utils, PLUG_LTE_UTIL_TABLE_SZ, 
                         "Pluggable LTE", &plug_lte_util_menup);

    build_secondary_submenu(pluggable_lte_utils, PLUG_LTE_UTIL_TABLE_SZ,
                            plug_lte_sec_util_items);

    menu(&plug_lte_util_menu, plug_lte_sec_util_items, '\0');

    return (PASSED);
}


/*******************************************************************************
 *
 * Function    : plug_lte_led_ctrl_util
 * Description : Function to show Pluggable LTE LED Control utilities submenu.
 * Inputs      : opt - reserved for future use.
 * Outputs     : PASSED / FAILED
 *
 *******************************************************************************
 */
int plug_lte_led_ctrl_util (int opt)
{
    build_primary_submenu(plug_lte_led_ctrl_tbl, PLUG_LTE_LED_CTRL_TBL_SIZE,
                          "Plug LTE LED Control Utilities", &plug_lte_led_ctrl_menup);
    build_secondary_submenu(plug_lte_led_ctrl_tbl, PLUG_LTE_LED_CTRL_TBL_SIZE,
                            plug_lte_led_ctrl_sec_items);

    menu(plug_lte_led_ctrl_menup, plug_lte_led_ctrl_sec_items, '\0');

    return (PASSED);
}

/*******************************************************************************
 * Function   : plug_lte_run_at_cmd
 * Description: To execute AT command for Pluggable LTE
 * Inputs     : input - Not used 
 * Outputs    : PASSED or FAILED
 *******************************************************************************
 */
static int plug_lte_run_at_cmd (int input)
{
    const int maxlen = 128;
    char cmd[maxlen];
    char usb_tty_dev[256];
    char usb_tty[15];

    printf("\n\n ### NOTE: Type CTRL-x "
                              "to switch back to host's console\n\n");
    fflush(stdout);
    fflush(stderr);
    msleep(AT_COMMAND_UTIL_DELAY); 

    /* Dynamically get the corresponding ttyUSB number in case usb device
     * attaches to different ttyUSB 
     */
    if (plug_lte_get_tty_devname(usb_tty_dev) != PASSED) {
        printf("%s:Can't get ttyUSB number\n", __func__);
        return (FAILED);
    }
    sprintf(usb_tty, "%s%s", USB_TTY_PATH, usb_tty_dev);

    snprintf(cmd, maxlen-1, "microcom %s", usb_tty);

    system(cmd);

    return (PASSED);
}


/*******************************************************************************
 * Function   : plug_lte_man_gpio_exp_util
 * Description: GPIO Expander Utility for Pluggable LTE
 * Inputs     : none 
 * Outputs    : PASSED or FAILED
 *******************************************************************************
 */
static int plug_lte_man_gpio_exp_util (void)
{
    int opt;

    printf("Mandatory PCA9557 (0x4E) Register Read/Write Utility\n");
    opt = getdec_answer("Read/Write Register? (0-Read, 1-Write):", OPT_READ,
                         OPT_READ, OPT_WRITE);

    if (opt == OPT_READ) {
        return (plug_gpio_exp_show_reg(MANDATORY));
    } else {
        return (plug_gpio_exp_alter_reg(MANDATORY));
    }
}


/*******************************************************************************
 * Function   : plug_lte_opt_gpio_exp_util
 * Description: GPIO Expander Utility for Pluggable LTE
 * Inputs     : none
 * Outputs    : PASSED or FAILED
 *******************************************************************************
 */
static int plug_lte_opt_gpio_exp_util (void)
{
    int opt;

    printf("Optional PCA9557 (0x4C) Register Read/Write Utility\n");
    opt = getdec_answer("Read/Write Register? (0-Read, 1-Write):", OPT_READ,
                         OPT_READ, OPT_WRITE);

    if (opt == OPT_READ) {
        return (plug_gpio_exp_show_reg(OPTIONAL));
    } else {
        return (plug_gpio_exp_alter_reg(OPTIONAL));
    }
}


/*******************************************************************************
 * Function   : plug_lte_ext_usb_util
 * Description: External USB Debug Enable/Disable Utility for Pluggable LTE
 * Inputs     : input - Not used
 * Outputs    : PASSED or FAILED
 *******************************************************************************
 */
static int plug_lte_ext_usb_util (int input)
{
    int opt;

    printf("Pluggable LTE External USB Debug Enable/Disable Utility.\n");
    opt = getdec_answer("Enable/Disable Debug USB? (0-Disable, 1-Enable):",
                        OPT_DISABLE, OPT_DISABLE, OPT_ENABLE);

    if (opt == OPT_ENABLE) {
        printf("Enable Debug USB.\n");
        plug_lte_usb_deb_enable(OPT_ENABLE); 
    } else {
        printf("Disable Debug USB.\n");
        plug_lte_usb_deb_enable(OPT_DISABLE); 
    }

    return (PASSED);
}


/*******************************************************************************
 * Function   : plug_lte_led_util
 * Description: LED Utility for Pluggable LTE
 * Inputs     : input - Not used
 * Outputs    : PASSED or FAILED
 *******************************************************************************
 */
static int plug_lte_led_util (int input)
{
    int ix = 0;
    int opt, color_opt;

    printf("Pluggable LTE LEDs utility.\n");
    opt = getdec_answer("Light up/off Register? (0-off, 1-on):", OPT_OFF, 
                        OPT_OFF, OPT_ON);
    if (opt == OPT_ON) {    
        color_opt = getdec_answer("Yellow/Green? (0-green, 1-yellow):",
                                  OPT_GREEN, OPT_GREEN, OPT_YELLOW);

        if (color_opt == OPT_GREEN) {
            printf("Turn all pluggable lte green LEDs on.\n");
            /* Pull yellow led registers low */
            for (ix = 0; ix < plug_lte_y_siz; ix++) {
                plug_lte_toggle_led(plug_lte_led_y_table[ix].dev_fun,
                                    plug_lte_led_y_table[ix].driv_off);
            }
            for (ix = 0; ix < plug_lte_g_siz; ix++) {
                plug_lte_toggle_led(plug_lte_led_g_table[ix].dev_fun,
                                    plug_lte_led_g_table[ix].driv_on);
            }
            /* Change RSSI to 3G mode to select LED color(green) */
            plug_lte_toggle_led(LED_4G_3G, HIGH);
            for (ix = 0; ix < plug_lte_rssi_siz; ix++) {
                plug_lte_toggle_led(plug_lte_led_rssi_table[ix].dev_fun,
                                    plug_lte_led_rssi_table[ix].driv_on); 
            }
        } else {
            printf("Turn all pluggable lte yellow LEDs on.\n");
            /* Pull green led registers low */
            for (ix = 0; ix < plug_lte_g_siz; ix++) {
                plug_lte_toggle_led(plug_lte_led_g_table[ix].dev_fun,
                                    plug_lte_led_g_table[ix].driv_off);
            }
            for (ix = 0; ix < plug_lte_y_siz; ix++) {
                plug_lte_toggle_led(plug_lte_led_y_table[ix].dev_fun,
                                    plug_lte_led_y_table[ix].driv_on); 
            }
            /* Change RSSI to 4G mode to select LED color(yellow) */
            plug_lte_toggle_led(LED_4G_3G, LOW);
            for (ix = 0; ix < plug_lte_rssi_siz; ix++) {
                plug_lte_toggle_led(plug_lte_led_rssi_table[ix].dev_fun,
                                    plug_lte_led_rssi_table[ix].driv_on); 
            }  
        }
    } else {
        printf("Turn all pluggable lte LEDs off.\n");
        for (ix = 0; ix < plug_lte_g_siz; ix++) {
            plug_lte_toggle_led(plug_lte_led_g_table[ix].dev_fun,
                                plug_lte_led_g_table[ix].driv_off);
        }
        for (ix = 0; ix < plug_lte_y_siz; ix++) {
            plug_lte_toggle_led(plug_lte_led_y_table[ix].dev_fun,
                                plug_lte_led_y_table[ix].driv_off);
        }
        for (ix = 0; ix < plug_lte_rssi_siz; ix++) {
            plug_lte_toggle_led(plug_lte_led_rssi_table[ix].dev_fun,
                                plug_lte_led_rssi_table[ix].driv_off);
        }
    }
    return (PASSED);
}


/*******************************************************************************
 *
 * Function    : plug_lte_enable_led_utils
 * Description : Function to turn Pluggable LTE modem enable LED ON/OFF.
 * Inputs      : opt - reserved for future use.
 * Outputs     : PASSED / FAILED
 *
 *******************************************************************************
 */
int plug_lte_enable_led_utils (int opt)
{
    uint option = 0;
    int ix;
    plug_lte_led_ctrl plug_lte_led_ctrl_reqt[] = {
        {ENABLE_LED_YELLOW, LOW},
        {ENABLE_LED_GREEN, LOW}
    };

    int max_enable_led_ctrl_bit = 
        sizeof(plug_lte_led_ctrl_reqt)/sizeof(plug_lte_led_ctrl);

    printf("\n"); 
    printf("Pluggable LTE Enable LED utils: \n"); 
    printf("0. OFF\n");
    printf("1. Yellow\n");
    printf("2. Green\n");
    option = getdec_answer("Select Toogle (0 ~ 2): ", 0, 0, 2);
    
    if (option == 0) { 
        plug_lte_led_ctrl_reqt[0].val = LOW;
        plug_lte_led_ctrl_reqt[1].val = LOW;
    } else if (option == 1) {
        plug_lte_led_ctrl_reqt[0].val = HIGH;
        plug_lte_led_ctrl_reqt[1].val = LOW;
    } else if (option == 2) {
        plug_lte_led_ctrl_reqt[0].val = LOW;
        plug_lte_led_ctrl_reqt[1].val = HIGH;
    } else {
        printf("No selection toggle\n");
        return (FAILED);
    }

    for (ix = 0; ix < max_enable_led_ctrl_bit; ix++) {
        if (plug_lte_toggle_led(plug_lte_led_ctrl_reqt[ix].dev, 
                                plug_lte_led_ctrl_reqt[ix].val) == FAILED) {
            printf("%s: Failed to toggle GPIO exp.\n", __func__);
            return(FAILED);
        }
    }

    return (PASSED);
}


/*******************************************************************************
 *
 * Function    : plug_lte_sim_status_led_utils
 * Description : Function to turn Pluggable LTE modem SIM0/1 Status LED ON/OFF.
 * Inputs      : opt - reserved for future use.
 * Outputs     : PASSED / FAILED
 *
 *******************************************************************************
 */
int plug_lte_sim_status_led_utils (int opt)
{
    uint option = 0;
    int ix, sim_opt = 0;
    plug_lte_led_ctrl plug_lte_led_ctrl_reqt[] = {
        {LED_SIM0_NOT_OK, LOW},
        {LED_SIM0_OK, LOW}
    };

    printf("\n"); 
    printf("Pluggable LTE SIM Status LED utils: \n"); 
    if (plug_lte_has_2_sim_slot() != TRUE) {
        sim_opt = 0;
    } else {
        sim_opt = getdec_answer("Control which LED? (0-SIM0, 1-SIM1):", 
                                0, 0, 1);
    }
    if (sim_opt == 0) {
        plug_lte_led_ctrl_reqt[0].dev = LED_SIM0_NOT_OK;
        plug_lte_led_ctrl_reqt[1].dev = LED_SIM0_OK;
    } else {
        plug_lte_led_ctrl_reqt[0].dev = LED_SIM1_NOT_OK;
        plug_lte_led_ctrl_reqt[1].dev = LED_SIM1_OK;
    }
    int max_sim_led_ctrl_bit = 
        sizeof(plug_lte_led_ctrl_reqt)/sizeof(plug_lte_led_ctrl);

    printf("0. OFF\n");
    printf("1. Yellow\n");
    printf("2. Green\n");
    option = getdec_answer("Select Toogle (0 ~ 2): ", 0, 0, 2);
    
    if (option == 0) { 
        plug_lte_led_ctrl_reqt[0].val = LOW;
        plug_lte_led_ctrl_reqt[1].val = LOW;
    } else if (option == 1) {
        plug_lte_led_ctrl_reqt[0].val = HIGH;
        plug_lte_led_ctrl_reqt[1].val = LOW;
    } else if (option == 2) {
        plug_lte_led_ctrl_reqt[0].val = LOW;
        plug_lte_led_ctrl_reqt[1].val = HIGH;
    } else {
        printf("No selection toggle\n");
        return (FAILED);
    }

    for (ix = 0; ix < max_sim_led_ctrl_bit; ix++) {
        if (plug_lte_toggle_led(plug_lte_led_ctrl_reqt[ix].dev, 
                                plug_lte_led_ctrl_reqt[ix].val) == FAILED) {
            printf("%s: Failed to toggle GPIO exp.\n", __func__);
            return(FAILED);
        }
    }

    return (PASSED);
}


/*******************************************************************************
 *
 * Function    : plug_lte_gps_status_led_utils
 * Description : Function to turn Pluggable LTE modem GPS LED ON/OFF.
 * Inputs      : opt - reserved for future use.
 * Outputs     : PASSED / FAILED
 *
 *******************************************************************************
 */
int plug_lte_gps_status_led_utils (int opt)
{
    uint option = 0;
    int ix;
    plug_lte_led_ctrl plug_lte_led_ctrl_reqt[] = {
        {LED_GPS_NOT_OK, LOW},
        {LED_GPS_OK, LOW}
    };

    int max_gps_led_ctrl_bit = 
        sizeof(plug_lte_led_ctrl_reqt)/sizeof(plug_lte_led_ctrl);

    printf("\n"); 
    printf("Pluggable LTE GPS LED utils: \n"); 
    printf("0. OFF\n");
    printf("1. Yellow\n");
    printf("2. Green\n");
    option = getdec_answer("Select Toogle (0 ~ 2): ", 0, 0, 2);
    
    if (option == 0) { 
        plug_lte_led_ctrl_reqt[0].val = LOW;
        plug_lte_led_ctrl_reqt[1].val = LOW;
    } else if (option == 1 ) {
        plug_lte_led_ctrl_reqt[0].val = HIGH;
        plug_lte_led_ctrl_reqt[1].val = LOW;
    } else if (option == 2 ) {
        plug_lte_led_ctrl_reqt[0].val = LOW;
        plug_lte_led_ctrl_reqt[1].val = HIGH;
    } else {
        printf("No selection toggle\n");
        return (FAILED);
    }

    for (ix = 0; ix < max_gps_led_ctrl_bit; ix++) {
        if (plug_lte_toggle_led(plug_lte_led_ctrl_reqt[ix].dev, 
                                plug_lte_led_ctrl_reqt[ix].val) == FAILED) {
            printf("%s: Failed to toggle GPIO exp.\n", __func__);
            return(FAILED);
        }
    }

    return (PASSED);
}


/*******************************************************************************
 *
 * Function    : plug_lte_rssi_led_utils
 * Description : Function to turn Pluggable LTE modem RSSI0/1/2/3 LED ON/OFF.
 * Inputs      : opt - reserved for future use.
 * Outputs     : PASSED / FAILED
 *
 *******************************************************************************
 */
int plug_lte_rssi_led_utils (int opt)
{
    uint option = 0;
    int ix;
    plug_lte_led_ctrl plug_lte_led_ctrl_reqt[] = {
        {LED_4G_3G, LOW},
        {LED_RSSI0, LOW},
        {LED_RSSI1, LOW},
        {LED_RSSI2, LOW},
        {LED_RSSI3, LOW}
    };
    int max_rssi_led_ctrl_bit = 
        sizeof(plug_lte_led_ctrl_reqt)/sizeof(plug_lte_led_ctrl);

    printf("\n"); 
    printf("Pluggable LTE RSSI Status LED utils: \n"); 
    printf("0. 2G/3G & LTE RSSI OFF\n");
    printf("1. 2G/3G RSSI\n");
    printf("2. 2G/3G RSSI Low\n");
    printf("3. 2G/3G RSSI Medium\n");
    printf("4. 2G/3G RSSI High\n");
    printf("5. LTE RSSI\n");
    printf("6. LTE RSSI Low\n");
    printf("7. LTE RSSI Medium\n");
    printf("8. LTE RSSI High\n");
    option = getdec_answer("Select Toogle (0 ~ 8): ", 0, 0, 8);
   
    /* Select LED color for 4G LTE mode */
    if (option > 4) {
       plug_lte_led_ctrl_reqt[0].val = HIGH;
    }

    switch (option) {
        case 0:
            break;
        case 1:
        case 5:
            plug_lte_led_ctrl_reqt[1].val = HIGH;
            break;
        case 2:
        case 6:
            plug_lte_led_ctrl_reqt[1].val = HIGH;
            plug_lte_led_ctrl_reqt[2].val = HIGH;
            break;
        case 3:
        case 7:
            plug_lte_led_ctrl_reqt[1].val = HIGH;
            plug_lte_led_ctrl_reqt[2].val = HIGH;
            plug_lte_led_ctrl_reqt[3].val = HIGH;
            break;
        case 4:
        case 8:
            plug_lte_led_ctrl_reqt[1].val = HIGH;
            plug_lte_led_ctrl_reqt[2].val = HIGH;
            plug_lte_led_ctrl_reqt[3].val = HIGH;
            plug_lte_led_ctrl_reqt[4].val = HIGH;
            break;
        default:
            printf("%s: No selection toggle\n", __func__);
            return (FAILED);
    }

    /* Trun off all RSSI LEDs first */
    for (ix = 1; ix < max_rssi_led_ctrl_bit; ix++) {
        if (plug_lte_toggle_led(plug_lte_led_ctrl_reqt[ix].dev, 
                                LOW) == FAILED) {
            printf("%s: Failed to toggle GPIO exp.\n", __func__);
            return(FAILED);
        }
    }

    for (ix = 0; ix < max_rssi_led_ctrl_bit; ix++) {
        if (plug_lte_toggle_led(plug_lte_led_ctrl_reqt[ix].dev, 
                                plug_lte_led_ctrl_reqt[ix].val) == FAILED) {
            printf("%s: Failed to toggle GPIO exp.\n", __func__);
            return(FAILED);
        }
    }

    return (PASSED);
}


/*******************************************************************************
 * Function   : plug_lte_ts_util
 * Description: Thermal Sensor Utility for Pluggable LTE
 * Inputs     : input - Not used
 * Outputs    : PASSED or FAILED
 *******************************************************************************
 */
static int plug_lte_ts_util (int input)
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
 * Function   : plug_lte_show_temp
 * Description: This function display temperature detected by temperature sensor
 * Inputs     : input - Not used
 * Outputs    : PASSED or FAILED
 *******************************************************************************
 */
static int plug_lte_show_temp (int input)
{
    return (plug_ts_show_temp());
}


/********************************************************************
 *
 * Function   : plug_lte_modem_temp_util 
 * Description: Function to detect LTE modem current temperature
 * Inputs     : input - Not used
 * Outputs    : PASSED/FAILED 
 *
 ********************************************************************/
int plug_lte_modem_temp_util (int input)
{
    if (plug_lte_at_run_cmd(DUMP_LTE_MODEM_TEMP) != PASSED) {
        printf("Failed to dump Pluggable LTE modem temperature\n");
        return (FAILED);
    }

    return (PASSED);
}


/*******************************************************************************
 * Function   : plug_lte_modem_soft_reset_util
 * Description: Do the LTE reset initialization sequence via AT command 
 *              for Pluggable LTE
 * Inputs     : input - Not used
 * Outputs    : PASSED or FAILED
 *******************************************************************************
 */
static int plug_lte_modem_soft_reset_util (int input)
{
    if (plug_lte_modem_soft_reset(0) != PASSED) {
        return (FAILED);
    }

    return (PASSED);
}


/*******************************************************************************
 * Function   : plug_lte_modem_hd_reset
 * Description: Function to hard reset the LTE modem via GPIO exp.
 *              Only for emergency use(host cannot communicate with modem).
 *              This hard reset may cause damage to the modem.
 * Inputs     : input - Not used
 * Outputs    : PASSED or FAILED
 *******************************************************************************
 */
static int plug_lte_modem_hd_reset (int input)
{
    printf("Hard reset LTE modem through GPIO exp. \
            \nThis utility should only be used while host cannot communicate \
            \nwith modem.\n");

    plug_lte_em_hard_reset();

    return (PASSED);
}


/*******************************************************************************
 *
 * Function   : lte_simdetect_pin_test
 * Description: Wrapped function to test LTE WP76xx and EM74xx SIM_DETECT pin.
 *              This function is to test
 *              (EM74xx) SIM_DETECT and SIM_DETECT_2
 *              (WP76xx) LTE UIM1_DET and pluggable module
 *                       SIM0_DETECT/SIM1_DETECT pin 
 *              by check if the state that AT!BSGPIO read back is as expected.
 *              Besides, this function also provides usr_prompt parameter for
 *              user prompt display enable/disable.
 * Inputs     : sim_num - SIM number(0/1)
 *              exp_sim_stat - Expected SIM status: PRESENT(1)/NOT_PRESENT(0)
 *              usr_prompt - To ENABLE/DISABLE user prompt
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
static int lte_simdetect_pin_test (int sim_num, boolean exp_sim_stat,
                                      boolean usr_prompt)
{
    char usr_input = 0;
    char usr_act_str[LTE_TESTMSG_BUFSZ];
    int at_cmd_test = 0;

    memset(usr_act_str, 0, sizeof(usr_act_str));

    /* Based on Pluggable LTE design,
     * If SIM card is present, LTE UIM1_DET(WP)/SIM_DETECT(EM) signal 
     * should be HIGH.
     * If SIM card is removed, LTE UIM1_DET(WP)/SIM_DETECT_2(EM) signal 
     * should be LOW.
     */

    if (is_plug_lte_wp()) {
        if (exp_sim_stat == SIM_PRESENT) {
            sprintf(usr_act_str, "install SIM card to");
            at_cmd_test = WP76XX_UIM1DET_H;
        } else {
            sprintf(usr_act_str, "remove SIM card from");
            at_cmd_test = WP76XX_UIM1DET_L;
        }
    }

    /* Switch external SIM mux to the testing SIM */
    switch (sim_num) {
    case SIM0:
        if (is_plug_lte_wp()) {
            plug_lte_sim_sel(SIM0);
        } else {
            if (exp_sim_stat == SIM_PRESENT) {
                sprintf(usr_act_str, "install SIM card to");
                at_cmd_test = EM74XX_SIMDETECT_H;
            } else {
                sprintf(usr_act_str, "remove SIM card from");
                at_cmd_test = EM74XX_SIMDETECT_L;
            }
        }
        break;
    case SIM1:
        if (is_plug_lte_wp()) {
            plug_lte_sim_sel(SIM1);
        } else {
            if (exp_sim_stat == SIM_PRESENT) {
                sprintf(usr_act_str, "install SIM card to");
                at_cmd_test = EM74XX_SIMDETECT2_H;
            } else {
                sprintf(usr_act_str, "remove SIM card from");
                at_cmd_test = EM74XX_SIMDETECT2_L;
            }
        }
        break;
    default:
        printf("%s(%d) Unsupported SIM number: %d\n",
               __func__, __LINE__, sim_num);
        return (FAILED);
    }

    /* Print out user prompt if needed */
    if (usr_prompt == ENABLE) {
        printf("\n\n### Please %s SIM slot %d.\n", usr_act_str, sim_num);
        do {
            printf("\r### Press 'y' to continue the Test: ");
            usr_input = getchar();
            if (usr_input == 'y') {
                break;
            }
        } while (usr_input != 'y');
    }

    /* Confirm the status of WP76xx LTE modem SIM_DETECT pin */
    if (plug_lte_at_run_cmd(at_cmd_test) != PASSED) {
        printf("%s(%d) SIM_DETECT pin status is not as expected.\n",
               __func__, __LINE__);
        return (FAILED);
    }
    return (PASSED);
}


/*******************************************************************************
 * Function   : plug_lte_simdetect_pin_test
 * Description: Function to test LTE modem SIM_DETECT pin.
 *              On EM74xx modems: This test verifies SIM_DETECT pin and 
 *                                SIM_DETECT_2 pin
 *              On WP76xx modems: This test verifies UIM1_DET pin, SIM mux, 
 *                                SIM0_DETECT pin and SIM1_DETECT pin
 * Inputs     : sim_num - SIM number(0/1)
 * Outputs    : PASSED or FAILED
 *******************************************************************************
 */
static int plug_lte_simdetect_pin_test (int sim_num)
{
    /* CSCvk20378: Pluggable LTE SIM_DETECT pin issue.
     *
     * This utility test is to enhance the coverage of pluggable LTE SIM_DETECT
     * pin. It requires users to insert and remove SIM card during the test.
     *
     * Based on comment from SWI(Sierra wireless)
     * On EM74xx:
     *   - AT!ENTERCND="A710" is required to enable the AT!BSGPIO command.
     *   - AT!BSGPIO?77 can be used to check the state of SIM_DETECT signal.
     *   - AT!BSGPIO?15 can be used to check the state of SIM_DETECT_2 signal.
     * On WP76xx:
     *   - AT!ENTERCND="A710" is required to enable the AT!BSGPIO command.
     *   - AT!BSGPIO?34 can be used to check the state of UIM1_DET signal.
     */ 

    char test_name[LTE_TESTMSG_BUFSZ];

    memset(test_name, 0, sizeof(test_name));

    switch (sim_num) {
    case SIM0:
        if (is_plug_lte_wp()) {
            sprintf(test_name, "WP76xx LTE UIM1_DET pin and "
                    "pluggable module SIM0_DETECT pin");
        } else {
            sprintf(test_name, "EM74xx LTE SIM_DETECT pin");
        }
        break;
    case SIM1:
        if (is_plug_lte_wp()) {
            sprintf(test_name, "WP76xx LTE UIM1_DET pin and "
                    "pluggable module SIM1_DETECT pin");
        } else {
            sprintf(test_name, "EM74xx LTE SIM_DETECT_2 pin");
        }
        break;
    default:
        cterr('f', 0, "Failed, got unsupported SIM number: %d ", sim_num);
        return (FAILED);
    }

    testname(test_name);
    prpass(testpass, "SIM%d, ", sim_num);

    /* Test SIM_DETECT pin when SIM is present */
    if (lte_simdetect_pin_test(sim_num, SIM_PRESENT, ENABLE) != PASSED) {
        cterr('f', 0, "Failed, SIM%d is inserted "
              "but SIM_DETECT state is Low.", sim_num);
        return (FAILED);
    }

    /* Test SIM_DETECT pin when SIM is NOT present */
    if (lte_simdetect_pin_test(sim_num, SIM_NOT_PRESENT, ENABLE) != PASSED) {
        cterr('f', 0, "Failed, SIM%d is NOT inserted "
              "but SIM_DETECT state is High.", sim_num);
        return (FAILED);
    }

    return (PASSED);
}


/*******************************************************************************
 * Function   : plug_lte_show_simdetect_pin_status
 * Description: Function to dump the status of LTE modem SIM_DETECT pin.
 *              Based on comment from SWI(Sierra wireless):
 *              On EM74xx:
 *              AT!BSGPIO?77 can be used to check the state of SIM_DETECT pin. 
 *              AT!BSGPIO?15 can be used to check the state of SIM_DETECT_2 pin
 *              On WP76xx:
 *              AT!BSGPIO?34 can be used to check the state of UIM1_DET pin 
 * Inputs     : sim_num - SIM number(0/1)
 * Outputs    : PASSED or FAILED
 *******************************************************************************
 */
static int plug_lte_show_simdetect_pin_status (int sim_num)
{
    int at_cmd_test;

    switch (sim_num) {
    case SIM0:
        if (is_plug_lte_wp()) {
            at_cmd_test = WP76XX_UIM1DET_STAT;
            plug_lte_sim_sel(SIM0);
        } else {
            at_cmd_test = EM74XX_SIMDETECT_STAT;
        }
        break;
    case SIM1:
        if (is_plug_lte_wp()) {
            at_cmd_test = WP76XX_UIM1DET_STAT;
            plug_lte_sim_sel(SIM1);
        } else {
            at_cmd_test = EM74XX_SIMDETECT2_STAT;
        }
        break;
    default:
        printf("%s(%d) Unsupported SIM number: %d\n",
               __func__, __LINE__, sim_num);
        return (FAILED);
    }

    if (plug_lte_at_run_cmd(at_cmd_test) != PASSED) {
        printf("\n%s:Failed to get SIM%d SIM_DETECT state.\n", 
               __func__, sim_num);
        return (FAILED);
    }

    return (PASSED);
}


/*******************************************************************************
 * Function   : plug_lte_shutdown_modem_utils
 * Description: Function to shutdown modem through utility.
 * Inputs     : input - not used
 * Outputs    : PASSED or FAILED
 *******************************************************************************
 */
static int plug_lte_shutdown_modem_utils (int input)
{
    if (modem_is_shutdown == FALSE) {
        /* Shutdown modem */
        if (plug_lte_modem_shutdown() != PASSED) {
            return (FAILED);
        }
        /* After shutdown modem, set flag "modem_is_shutdown" to TRUE */
        modem_is_shutdown = TRUE;
    } else {
        printf("\n!!! Modem has already shutdown !!!\n");
    }
    
    printf("\nPlease press 'Esc' return to the Main Menu.\n");
    return (PASSED);
}


/*-------------------------------------------------
$Log: plug_lte_util.c,v $
Revision 1.11  2020/01/17 03:06:05  sherliu2
Add function to check pluggable modem carrier is matched before testing

Revision 1.10  2019/06/14 05:48:11  shjung
Supported WP7605 modules

Revision 1.9  2018/11/23 09:15:08  hondwang
Re-instruct pluggable common code with CDETs CSCvn17216

Revision 1.8.14.3  2018/10/30 06:49:39  shjung
Code clean up

Revision 1.8.14.2  2018/10/15 07:43:27  shjung
Re-struct for pluggable-LTE common codes

Revision 1.8.14.1  2018/10/15 06:51:05  hondwang
pluggable common code re-instruct modify code

Revision 1.8  2018/07/12 09:33:41  shjung
Fixed CSCvk20378:Covered pluggable LTE modem SIM_DETECT pin

Revision 1.7  2018/06/05 09:54:08  lucywang
Merge Star branch star-branch-c110x to main trunk

Revision 1.6  2018/03/29 10:26:53  shjung
Remove modem reset test

Revision 1.5  2018/03/27 12:46:36  hondwang
Code modify for Star_C1101_4PLTEP_4PLTEPWX and Pluggable LTE EM7455, WP7601, WP7603 ER

Revision 1.4  2018/02/26 09:56:43  shjung
Code clean up

Revision 1.3.2.2  2018/03/16 07:47:28  shjung
1. Correct the default value of WP SAFE_POWER_REMOVAL and implement WP modem power-off function 2. Check modem status after hard reset

Revision 1.3.2.1  2018/03/02 03:29:32  shjung
Remove debug port test from default test items and code clean up

Revision 1.3  2018/02/09 09:15:46  hondwang
Merge Star branch star-branch-c9xx to main trunk

Revision 1.2.2.3  2018/02/01 23:41:02  shjung

1. Added USB2.0 Detection Tset via AT command
2. Adjusted LTE modem power on/off timing as SWI recommanded
3. Added modem temperature reading utility and modem hard-reset utility
4. Hide SIM Slot 1 Detection Test for WP7601 due to HW changes
5. Extended delay time while checking modem usb device status to avoid tty resource is occupied
6. Added modem status check mechanism to ensure modem is ready after power-cycle
7. Added delay time in pluggable LTE modem power on/off function
8. Added WP7607 RSSI test configuration

Revision 1.2.2.2  2018/01/20 06:56:37  hondwang
prepare merge star-branch-c9xx to main trunk

Revision 1.2  2018/01/20 05:01:09  hondwang
prepare merge star-branch-c9xx to main trunk

Revision 1.1.4.4  2017/12/08 12:28:46  shjung
Check if usb device attaches to tty successfully before capture corresponding ttyUSB number

Revision 1.1.4.3  2017/12/06 13:23:13  shjung
Dynamically get the according ttyUSB number in case usb device attaches to different ttyUSB

Revision 1.1.4.2  2017/08/08 07:42:14  hondwang
add pluggable LTE for star-branch-c9xx

Revision 1.1.2.1  2017/07/13 06:32:21  tirawan
Reorganize Star Pluggable directory structure

Revision 1.1.2.6  2017/07/11 18:29:34  tirawan
Add AT command utility and change the RSSI frequency to 944.5 Mhz

Revision 1.1.2.5  2017/06/28 00:46:18  shjung
Add pluggable LTE debug usb utility

Revision 1.1.2.4  2017/06/27 22:45:21  shjung
Add pluggable LTE LED utility

Revision 1.1.2.3  2017/06/22 19:27:11  tirawan
Add LTE Test items and add log section at the bottom of the code


*/

