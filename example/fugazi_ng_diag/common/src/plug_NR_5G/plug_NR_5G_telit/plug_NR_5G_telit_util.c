/* 
 * $Id: plug_NR_5G_telit_util.c,v 1.2 2021/06/02 02:56:20 alpeng Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/plug_NR_5G/plug_NR_5G_telit/plug_NR_5G_telit_util.c,v $
 *------------------------------------------------------------------
 *
 * plug_NR_5g_telit_util.c - Pluggable NR_5G Telit Utility
 *
 * Copyright (c) 2020 by Cisco Systems, Inc.
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
#include "plug_NR_5G_telit_util.h"
#include "plug_NR_5G_telit_lib.h"
#include "dev_NR_5G_telit_at.h"

int plug_NR_5g_telit_util(void);
int plug_NR_5g_telit_modem_temp_util(int);
int plug_NR_5g_telit_led_ctrl_util(int);

static int plug_NR_5g_telit_run_at_cmd(int);
static int plug_NR_5g_telit_man_gpio_exp_util(void);
static int plug_NR_5g_telit_opt_gpio_exp_util(void);
static int plug_NR_5g_telit_ext_usb_util(int);
static int plug_NR_5g_telit_show_temp(int);
static int plug_NR_5g_telit_ts_util(int);
static int plug_NR_5g_telit_modem_hd_reset(int);
static int plug_NR_5g_telit_modem_reboot_util(int);
static int plug_NR_5g_telit_multi_led_ctrl_util(int);
static int plug_NR_5g_telit_simdetect_pin_test(int);
static int plug_NR_5g_telit_show_simdetect_pin_status(int);
static int plug_NR_5g_telit_enable_led_utils(int);
static int plug_NR_5g_telit_sim_status_led_utils(int);
static int plug_NR_5g_telit_gps_status_led_utils(int);
static int plug_NR_5g_telit_rssi_led_utils(int);
static int plug_NR_5g_telit_wan_led_util(int);
static int plug_NR_5g_telit_wan_led_on_off_util(int);
static int plug_NR_5g_telit_wan_led_sel_util(int);
static int plug_NR_5G_telit_simdet_pin_test(int, boolean, boolean);
static int plug_NR_5g_telit_usb_mode_switch_util(int);
static int plug_NR_5g_telit_show_testmode_stat_util(int);
static int plug_NR_5g_telit_show_modem_info_util(int);
static int plug_NR_5g_telit_enable_fast_shdn_util(void);
static int plug_NR_5g_telit_dump_modem_usb_connection_util(void);
static int plug_NR_5g_telit_set_modem_power_saving_mode_util(void);
static int plug_NR_5g_telit_select_band_to_test_rssi(void);
boolean plug_test_not_supported (void);
int plug_NR_5g_telit_enable_op_mode_util(int);
extern int band_tbl_size;
extern int band_to_test;
extern nr_sub6_band_struct nr_sub6_band_tbl[];

static submenu_xtable_t pluggable_NR_5G_telit_utils[] = {
    {"AT Command Utility", 
     (type_t(*)())plug_NR_5g_telit_run_at_cmd, TRUE,
     0, (type_t(*)())0, 0, (type_t(*)())0, 0},
    {"Mandatory PCA9555 (0x4E) Register Read/Write Utility", 
     (type_t(*)())plug_NR_5g_telit_man_gpio_exp_util, TRUE,
     0, (type_t(*)())0, 0, (type_t(*)())0, 0},
    {"Optional PCA9555 (0x4C) Register Read/Write Utility", 
     (type_t(*)())plug_NR_5g_telit_opt_gpio_exp_util, TRUE,
     0, (type_t(*)())0, 0, (type_t(*)())0, 0},
    {"External USB Debug bus Enable/Disable Utility", 
     (type_t(*)())plug_NR_5g_telit_ext_usb_util, TRUE,
     0, (type_t(*)())0, 0, (type_t(*)())0, 0},
    {"Temperature Display via Thermal Sensor Utility", 
     (type_t(*)())plug_NR_5g_telit_show_temp, TRUE,
     0, (type_t(*)())plug_NR_5g_telit_has_temp_sensor, 0, (type_t(*)())0, 0},
    {"Temperature Sensor Register Read/Write Utility", 
     (type_t(*)())plug_NR_5g_telit_ts_util, TRUE,
     0, (type_t(*)())plug_NR_5g_telit_has_temp_sensor, 0, (type_t(*)())0, 0},
    {"Modem Temperature Display Utility", 
     (type_t(*)())plug_NR_5g_telit_modem_temp_util, TRUE,
     0, (type_t(*)())0, 0, (type_t(*)())0, 0},
    {"Modem Hard Reset Utility(Emergency use)", 
     (type_t(*)())plug_NR_5g_telit_modem_hd_reset, TRUE,
     0, (type_t(*)())0, 0, (type_t(*)())0, 0},
    {"Modem Reboot Utility", 
     (type_t(*)())plug_NR_5g_telit_modem_reboot_util, TRUE,
     0, (type_t(*)())0, 0, (type_t(*)())0, 0},
    {"Modem SIM_DETECT pin Test(SIM0)", 
     (type_t(*)())plug_NR_5g_telit_simdetect_pin_test, SIM0,
     0, (type_t(*)())0, 0, (type_t(*)())0, 0},
    {"Modem SIM_DETECT pin Test(SIM1)", 
     (type_t(*)())plug_NR_5g_telit_simdetect_pin_test, SIM1,
     0, (type_t(*)())0, 0, (type_t(*)())0, 0},
    {"Show Modem SIM_DETECT pin Status(SIM0)", 
     (type_t(*)())plug_NR_5g_telit_show_simdetect_pin_status, SIM0,
     0, (type_t(*)())0, 0, (type_t(*)())0, 0},
    {"Show Modem SIM_DETECT pin Status(SIM1)", 
     (type_t(*)())plug_NR_5g_telit_show_simdetect_pin_status, SIM1,
     0, (type_t(*)())0, 0, (type_t(*)())0, 0},
    {"Modem USB Mode Switching Utility", 
     (type_t(*)())plug_NR_5g_telit_usb_mode_switch_util, TRUE,
     0, (type_t(*)())0, 0, (type_t(*)())0, 0},
    {"Show Modem TestMode Status Utility", 
     (type_t(*)())plug_NR_5g_telit_show_testmode_stat_util, TRUE,
     0, (type_t(*)())0, 0, (type_t(*)())0, 0},
    {"Enable Modem Operation Mode Utility", 
     (type_t(*)())plug_NR_5g_telit_enable_op_mode_util, TRUE,
     0, (type_t(*)())0, 0, (type_t(*)())0, 0},
    {"Dump Modem USB Connection Info Utility", 
     (type_t(*)())plug_NR_5g_telit_dump_modem_usb_connection_util, TRUE,
     0, (type_t(*)())0, 0, (type_t(*)())0, 0},
    {"Show Modem Information Utility", 
     (type_t(*)())plug_NR_5g_telit_show_modem_info_util, TRUE,
     0, (type_t(*)())0, 0, (type_t(*)())0, 0},
    {"Configure Modem Power Saving Mode Utility", 
     (type_t(*)())plug_NR_5g_telit_set_modem_power_saving_mode_util, TRUE,
     0, (type_t(*)())0, 0, (type_t(*)())0, 0},
    {"Enable Modem Fast Shutdown Utility", 
     (type_t(*)())plug_NR_5g_telit_enable_fast_shdn_util, TRUE,
     0, (type_t(*)())plug_test_not_supported, 0, (type_t(*)())0, 0},
    {"Select Band for RSSI Test", 
     (type_t(*)())plug_NR_5g_telit_select_band_to_test_rssi, TRUE,
     0, (type_t(*)())0, 0, (type_t(*)())0, 0},
};
#define PLUG_5G_NR_TELIT_UTIL_TABLE_SZ \
        (sizeof(pluggable_NR_5G_telit_utils) / sizeof(submenu_xtable_t))

static mitem_t plug_NR_5g_telit_pri_util_items[PLUG_5G_NR_TELIT_UTIL_TABLE_SZ+
                                             MAX_BASE_ITEMS];
static mitem_t plug_NR_5g_telit_sec_util_items[PLUG_5G_NR_TELIT_UTIL_TABLE_SZ+
                                             MAX_BASE_ITEMS];

static menuinfo_t plug_NR_5g_telit_util_menu = {
    "Pluggable NR_5G Telit module Utilities Menu",
    0,                            /* mtparam added by init_empty_menu */
    (PFT)menu_show_dflags,        /* shows major flags */
    0,                            /* use generic prompt */
    0,                            /* size (bumped by add_menu_item() */
    plug_NR_5g_telit_pri_util_items,
};
static menuinfo_t *plug_NR_5g_telit_util_menup = &plug_NR_5g_telit_util_menu;

/*
 * LED Control
 */
static submenu_xtable_t plug_NR_5g_telit_led_ctrl_tbl[] = {
    {"Enable LED utils", (type_t(*)())plug_NR_5g_telit_enable_led_utils,
      TRUE, 0, (type_t(*)())0, 0, (type_t(*)())0, 0},
    {"SIM Status LED utils", (type_t(*)())plug_NR_5g_telit_sim_status_led_utils,
      TRUE, 0, (type_t(*)())0, 0, (type_t(*)())0, 0},
    {"GPS Status LED utils", (type_t(*)())plug_NR_5g_telit_gps_status_led_utils,
      TRUE, 0, (type_t(*)())plug_NR_5g_telit_has_dedicated_gps_antenna, 0,
      (type_t(*)())0, 0},
    {"Service Indication LED utils", (type_t(*)())plug_NR_5g_telit_rssi_led_utils,
      TRUE, 0, (type_t(*)())0, 0, (type_t(*)())0, 0},
    {"WAN LED utils", (type_t(*)())plug_NR_5g_telit_wan_led_util,
      TRUE, 0, (type_t(*)())0, 0, (type_t(*)())0, 0},
    {"Multi LED Control Utility", (type_t(*)())plug_NR_5g_telit_multi_led_ctrl_util,
      TRUE, 0, (type_t(*)())0, 0, (type_t(*)())0, 0},
};

#define PLUG_5G_NR_SUB6_TELIT_LED_CTRL_TBL_SZ \
        (sizeof(plug_NR_5g_telit_led_ctrl_tbl) / sizeof(submenu_xtable_t))

static mitem_t plug_NR_5g_telit_led_ctrl_pri_items[PLUG_5G_NR_SUB6_TELIT_LED_CTRL_TBL_SZ+
                                                 MAX_BASE_ITEMS];
static mitem_t plug_NR_5g_telit_led_ctrl_sec_items[PLUG_5G_NR_SUB6_TELIT_LED_CTRL_TBL_SZ+
                                                 MAX_BASE_ITEMS];               

menuinfo_t plug_NR_5g_telit_led_ctrl_menu = {
    "%s Menu",
    0,
    (PFT)show_endnote,
    0,
    0,
    plug_NR_5g_telit_led_ctrl_pri_items,
};
menuinfo_t *plug_NR_5g_telit_led_ctrl_menup = &plug_NR_5g_telit_led_ctrl_menu;

/*
 * WAN LED Control
 */
static submenu_xtable_t plug_NR_5g_telit_wan_led_ctrl_tbl[] = {
    {"WAN LED on/off utils", (type_t(*)())plug_NR_5g_telit_wan_led_on_off_util,
      TRUE, 0, (type_t(*)())0, 0, (type_t(*)())0, 0},
    {"WAN LED select pin utils", (type_t(*)())plug_NR_5g_telit_wan_led_sel_util,
      TRUE, 0, (type_t(*)())0, 0, (type_t(*)())0, 0},
};

#define PLUG_5G_NR_SUB6_TELIT_WAN_LED_CTRL_TBL_SZ \
        (sizeof(plug_NR_5g_telit_wan_led_ctrl_tbl) / sizeof(submenu_xtable_t))

static mitem_t plug_NR_5g_telit_wan_led_ctrl_pri_items[
                                           PLUG_5G_NR_SUB6_TELIT_WAN_LED_CTRL_TBL_SZ+
                                           MAX_BASE_ITEMS];
static mitem_t plug_NR_5g_telit_wan_led_ctrl_sec_items[
                                           PLUG_5G_NR_SUB6_TELIT_WAN_LED_CTRL_TBL_SZ+
                                           MAX_BASE_ITEMS];               

menuinfo_t plug_NR_5g_telit_wan_led_ctrl_menu = {
    "%s Menu",
    0,
    (PFT)show_endnote,
    0,
    0,
    plug_NR_5g_telit_wan_led_ctrl_pri_items,
};
menuinfo_t *plug_NR_5g_telit_wan_led_ctrl_menup =
                                       &plug_NR_5g_telit_wan_led_ctrl_menu;

static plug_NR_5G_led_dev plug_NR_5g_telit_g_led_table[] = {
    {ENABLE_LED_GREEN, HIGH, LOW},
    {LED_SIM0_OK, HIGH, LOW},
    {LED_SIM1_OK, HIGH, LOW},
    {LED_GPS_OK, HIGH, LOW}
};
static plug_NR_5G_led_dev plug_NR_5g_telit_y_led_table[] = {
    {ENABLE_LED_YELLOW, HIGH, LOW},
    {LED_SIM0_NOT_OK, HIGH, LOW},
    {LED_SIM1_NOT_OK, HIGH, LOW},
    {LED_GPS_NOT_OK, HIGH, LOW}
};
static plug_NR_5G_led_dev plug_NR_5g_telit_rssi_led_table[] = {
    {LED_RSSI0 , HIGH, LOW}, //Green
    {LED_RSSI1 , HIGH, LOW}, //Yellow
    {LED_RSSI2 , HIGH, LOW}, //Blue
};

static plug_NR_5G_led_dev plug_NR_5g_telit_sim_led_table[] = {
    {LED_SIM0_OK, HIGH, LOW},
    {LED_SIM1_OK, HIGH, LOW},
    {LED_SIM0_NOT_OK, HIGH, LOW},
    {LED_SIM1_NOT_OK, HIGH, LOW},
};

int plug_NR_5g_telit_g_led_sz = sizeof(plug_NR_5g_telit_g_led_table)/
                              sizeof(plug_NR_5G_led_dev);
int plug_NR_5g_telit_y_led_sz = sizeof(plug_NR_5g_telit_y_led_table)/
                              sizeof(plug_NR_5G_led_dev);
int plug_NR_5g_telit_rssi_led_sz = sizeof(plug_NR_5g_telit_rssi_led_table)/
                                 sizeof(plug_NR_5G_led_dev);
int plug_NR_5g_telit_sim_led_sz = sizeof(plug_NR_5g_telit_sim_led_table)/
                                sizeof(plug_NR_5G_led_dev);

/*******************************************************************************
 * Function   : plug_NR_5g_telit_util
 * Description: Main Entry point for Pluggable module Utilities
 * Inputs     : None
 * Outputs    : PASSED or FAILED
 *******************************************************************************
 */
int plug_NR_5g_telit_util (void)
{   
    build_primary_submenu(pluggable_NR_5G_telit_utils, 
                          PLUG_5G_NR_TELIT_UTIL_TABLE_SZ, 
                          "Pluggable NR_5G Telit module", &plug_NR_5g_telit_util_menup);

    build_secondary_submenu(pluggable_NR_5G_telit_utils, 
                            PLUG_5G_NR_TELIT_UTIL_TABLE_SZ,
                            plug_NR_5g_telit_sec_util_items);

    menu(&plug_NR_5g_telit_util_menu, plug_NR_5g_telit_sec_util_items, '\0');

    return (PASSED);
}


/*******************************************************************************
 * Function   : plug_NR_5g_telit_run_at_cmd
 * Description: To execute AT command for Pluggable module
 * Inputs     : input - Not used 
 * Outputs    : PASSED/FAILED
 *******************************************************************************
 */
static int plug_NR_5g_telit_run_at_cmd (int input)
{
    const int maxlen = 128;
    char cmd[maxlen];
    char usb_tty_dev[256] = {0,};
    char usb_tty[64] = {0,};

    printf("\n\n ###NOTE: Typer CTRL-x "
           "to switch back to host's console\n\n");
    fflush(stdout);
    fflush(stderr);
    msleep(AT_COMMAND_UTIL_DELAY);
    
    /* Dynamically get the corresponding ttyUSB number in case usb device
     * attaches to different ttyUSB */
    if (plug_NR_5g_telit_get_tty_devname(usb_tty_dev) != PASSED) {
        printf("%s: Can't get ttyUSB number\n", __func__);
        return (FAILED);
    }
    sprintf(usb_tty, "%s/%s", USB_TTY_PATH, usb_tty_dev);

    snprintf(cmd, maxlen-1, "microcom %s", usb_tty);

    system(cmd);

    return (PASSED);
}


/*******************************************************************************
 * Function   : plug_NR_5g_telit_man_gpio_exp_util
 * Description: GPIO Expander Utility for Pluggable module
 * Inputs     : none 
 * Outputs    : PASSED/FAILED
 *******************************************************************************
 */
static int plug_NR_5g_telit_man_gpio_exp_util (void)
{
    int opt;

    printf("Mandatory PCA9555 (0x4E) Register Read/Write Utility\n");
    opt = getdec_answer("Read/Write Register? (0-Read, 1-Write):", OPT_READ,
                        OPT_READ, OPT_WRITE);
        
    if (opt == OPT_READ) {
        return (plug_gpio_exp_show_reg(MANDATORY));
    } else {
        return (plug_gpio_exp_alter_reg(MANDATORY));
    }
}


/*******************************************************************************
 * Function   : plug_NR_5g_telit_opt_gpio_exp_util
 * Description: GPIO Expander Utility for Pluggable module
 * Inputs     : none
 * Outputs    : PASSED/FAILED
 *******************************************************************************
 */
static int plug_NR_5g_telit_opt_gpio_exp_util (void)
{
    int opt;

    printf("Optional PCA9555 (0x4C) Register Read/Write Utility\n");
    opt = getdec_answer("Read/Write Register? (0-Read, 1-Write):", OPT_READ,
                         OPT_READ, OPT_WRITE);

    if (opt == OPT_READ) {
        return (plug_gpio_exp_show_reg(OPTIONAL));
    } else {
        return (plug_gpio_exp_alter_reg(OPTIONAL));
    }
}


/*******************************************************************************
 * Function   : plug_NR_5g_telit_ext_usb_util
 * Description: External USB Debug Enable/Disable Utility for Pluggable module
 * Inputs     : input - Not used
 * Outputs    : PASSED/FAILED
 *******************************************************************************
 */
static int plug_NR_5g_telit_ext_usb_util (int input)
{
    int opt;

    printf("Pluggable External USB Enable/Disable Utility.\n");
    opt = getdec_answer("Enable/Disable Debug USB? (0-Disable, 1-Enable): ",
                        OPT_DISABLE, OPT_DISABLE, OPT_ENABLE);

    if (opt == OPT_ENABLE)  {
        printf("Enable Debug USB.\n");
        plug_NR_5g_telit_usb_deb_enable(OPT_ENABLE);
    } else {
        printf("Disable Debug USB.\n");
        plug_NR_5g_telit_usb_deb_enable(OPT_DISABLE);
    }

    return (PASSED);
}


/*******************************************************************************
 * Function   : plug_NR_5g_telit_show_temp
 * Description: This function display temperature detected by temperature sensor
 * Inputs     : input - Not used
 * Outputs    : PASSED/FAILED
 *******************************************************************************
 */
static int plug_NR_5g_telit_show_temp (int input)
{
    return (plug_ts_show_temp());
}


/*******************************************************************************
 * Function   : plug_NR_5g_telit_ts_util
 * Description: Thermal Sensor Utility for Pluggable module
 * Inputs     : input - Not used
 * Outputs    : PASSED/FAILED
 *******************************************************************************
 */
static int plug_NR_5g_telit_ts_util (int input)
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
 * Function   : plug_NR_5g_telit_modem_temp_util
 * Description: Function to return the current temperature of Telit modem
 * Inputs     : input - Not used
 * Outputs    : PASSED/FAILED
 *******************************************************************************
 */
int plug_NR_5g_telit_modem_temp_util (int input)
{
    printf("Modem temperature info:\n");
    return (plug_NR_5g_telit_dump_modem_temp());
}


/*******************************************************************************
 * Function   : plug_NR_5g_telit_modem_hd_reset
 * Description: Function to hard reset the modem via GPIO exp.
 *              Only for emergency use(host cannot communicate with modem).
 *              This hard reset may cause damage to the modem.
 * Inputs     : input - Not used
 * Outputs    : PASSED/FAILED
 *******************************************************************************
 */
static int plug_NR_5g_telit_modem_hd_reset (int input)
{
    int rc = FAILED;

    printf("Hard reset the modem through GPIO exp. "
           "This utility should only used while host cannot communicate "
           "with modem.\n");

    rc = plug_NR_5g_telit_hard_reset();

    return (rc);
}


/*******************************************************************************
 * Function   : plug_NR_5g_telit_modem_reboot_util
 * Description: Function to reboot modem via AT command 
 * Inputs     : input - Not used
 * Outputs    : PASSED/FAILED
 *******************************************************************************
 */
static int plug_NR_5g_telit_modem_reboot_util (int input)
{
    int rc = FAILED;

    printf("Modem Soft reboot utility.\n");

    rc = plug_NR_5g_telit_soft_reboot(USB3P0);

    return (rc);
}


/*******************************************************************************
 * Function   : plug_NR_5g_telit_simdetect_pin_test
 * Description: Function to test modem SIM_DETECT pin.
 *              This test verifies SIM0_DETECT pin and SIM1_DETECT pin
 * Inputs     : sim_no - SIM slot number(0/1)
 * Outputs    : PASSED/FAILED
 *******************************************************************************
 */
static int plug_NR_5g_telit_simdetect_pin_test (int sim_no)
{
    /* This utility test is to enhance the coverage of pluggable SIM_DETECT
     * pin. It requires users to insert and remove SIM card during the test.
     */
    char test_name[TESTMSG_BUFSZ];

    memset(test_name, 0, sizeof(test_name));
    sprintf(test_name, "SIM%d_DETECT pin", sim_no);
    testname(test_name);
    prpass(testpass, "SIM%d, ", sim_no);

    plug_NR_5G_telit_sim_sel(sim_no);

    /* Test SIM_DETECT pin when SIM is present */
    if (plug_NR_5G_telit_simdet_pin_test(sim_no, SIM_PRESENT, ENABLE)
                                  != PASSED) {
        cterr('f', 0, "Failed, SIM%d is inserted "
              "but SIM_DETECT state is Low.", sim_no);
        return (FAILED);
    }

    /* Test SIM_DETECT pin when SIM is NOT present */
    if (plug_NR_5G_telit_simdet_pin_test(sim_no, SIM_NOT_PRESENT, ENABLE)
                                  != PASSED) {
        cterr('f', 0, "Failed, SIM%d is NOT inserted "
              "but SIM_DETECT state is High.", sim_no);
        return (FAILED);
    }

    return (PASSED);
}


/*******************************************************************************
 * Function   : plug_NR_5G_telit_simdet_pin_test
 * Description: Wrapped function to test modem SIM_DETECT pin.
 *              This test verifies SIM0_DETECT pin and SIM1_DETECT pin by
 *              reading modem SIMIN0 and SIMIN1 status through AT command
 *              Topology: SIM slot 1 <-(SIM0_DETECT)-> mPCIe connector
 *                        <-Telit modem(SIMIN0)
 *                        SIM slot 2 <-(SIM1_DETECT)-> mPCIe connector
 *                        <-Telit modem(SIMIN1)
 * Inputs     : sim_num - SIM number(0/1)
 *              exp_sim_stat - expected status of SIM_DETECT pin
 *              usr_prompt - flag to determine whether to show up user prompt
 * Outputs    : PASSED/FAILED
 *******************************************************************************
 */
static int plug_NR_5G_telit_simdet_pin_test (int sim_num, boolean exp_sim_stat,
                                      boolean usr_prompt)
{
    char usr_input = 0;
    char usr_act_str[TESTMSG_BUFSZ];
    dev_NR_5g_telit_object_t plug_NR_5g_telit_obj;
    dev_NR_5g_telit_object_t *plug_NR_5g_telit_obj_p = &plug_NR_5g_telit_obj;
    int rc = FAILED;

    memset(usr_act_str, 0, sizeof(usr_act_str));


    if (plug_NR_5g_telit_dev_create(plug_NR_5g_telit_obj_p) != PASSED) {
        cterr('f', 0, "Create Telit Dev Object Fails");
        return (FAILED);
    }

    /* Set user prompt string */
    if (exp_sim_stat == SIM_PRESENT) {
        sprintf(usr_act_str, "install SIM card to");
    } else {
        sprintf(usr_act_str, "remove SIM card from");
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

    /* Check whether the SIM_DETECT pin is present */
    rc = plug_NR_5g_telit_obj_p->callin_fvt->modem_simin_pin_present(
                                           (dev_object_t *)&plug_NR_5g_telit_obj,
                                            sim_num);
    if (exp_sim_stat == SIM_NOT_PRESENT) {
        if (rc == PASSED) {
            rc = FAILED;
        } else {
            rc = PASSED;
        }
    }

    plug_NR_5g_telit_obj_p->base.dev_object_fvt->dev_destroy((dev_object_t **)
                                                           &plug_NR_5g_telit_obj);

    if (rc != PASSED) {
        cterr('f', 0, "Unexpected SIM%d_DETECT pin status.", sim_num);
    }
    return (rc);
}


/*******************************************************************************
 * Function   : plug_NR_5G_show_simdetect_pin_status
 * Description: Function to dump the status of modem SIM_DETECT pin.
 * Inputs     : sim_num - SIM number(0/1)
 * Outputs    : PASSED or FAILED
 *******************************************************************************
 */
static int plug_NR_5g_telit_show_simdetect_pin_status (int sim_num)
{
    dev_NR_5g_telit_object_t plug_NR_5g_telit_obj;
    dev_NR_5g_telit_object_t *plug_NR_5g_telit_obj_p = &plug_NR_5g_telit_obj;
    int rc = FAILED;
    
    plug_NR_5G_telit_sim_sel(sim_num);
    msleep(DELAY_1000_MS);

    if (plug_NR_5g_telit_dev_create(plug_NR_5g_telit_obj_p) != PASSED) {
        cterr('f', 0, "Create Telit Dev Object Fails");
        return (FAILED);
    }

    rc = plug_NR_5g_telit_obj_p->callin_fvt->modem_dump_simin_pin_status(
                                           (dev_object_t *)&plug_NR_5g_telit_obj,
                                            sim_num);

    plug_NR_5g_telit_obj_p->base.dev_object_fvt->dev_destroy((dev_object_t **)
                                                           &plug_NR_5g_telit_obj);
    return (rc);
}


/*******************************************************************************
 * Function    : plug_NR_5g_telit_usb_mode_switch_util
 * Description : Function to switch Pluggable modem USB mode.
 * Inputs      : opt - reserved for future use.
 * Outputs     : PASSED/FAILED
 *******************************************************************************
 */
static int plug_NR_5g_telit_usb_mode_switch_util (int opt)
{
    int mode_opt;
    int rc = FAILED;
    int modem_found = FALSE, modem_uport = -1;
    dev_NR_5g_telit_object_t plug_NR_5g_telit_obj;
    dev_NR_5g_telit_object_t *plug_NR_5g_telit_obj_p = &plug_NR_5g_telit_obj;

    printf("Modem current USB connection info:\n");
    plug_NR_5g_telit_modem_searching(&modem_found, &modem_uport);

    printf("\nModem USB mode: %s \nModem connects to: %s.\n",
           (modem_uport == USB3P0)? SUPER_SPD_STR:HIGH_SPD_STR,
           (modem_uport == DEBUG_USB)? DPORT_STR:HOST_CONN_STR);

    mode_opt = getdec_answer("Switch to which mode? (0-Super Speed(USB3.0), "
                             "1-High Speed(USB2.0)):", SUPER_SPD_USB,
                             SUPER_SPD_USB, HIGH_SPD_USB);

    plug_NR_5g_telit_set_current_usb_port(modem_uport);

    /* Create device object */
    if (plug_NR_5g_telit_dev_create(plug_NR_5g_telit_obj_p) != PASSED) {
        printf("Create Telit Dev Object Fails\n");
        goto __exit;
    }

    /* Switch modem USB mode */
    rc = plug_NR_5g_telit_obj_p->callin_fvt->modem_switch_usb_mode(
                                           (dev_object_t *)&plug_NR_5g_telit_obj,
                                            mode_opt);
    if (rc != PASSED) {
        printf("Failed to switch modem USB mode\n");
        goto __exit;
    }

    plug_NR_5g_telit_obj_p->base.dev_object_fvt->dev_destroy((dev_object_t **)
                                                           &plug_NR_5g_telit_obj);
    plug_NR_5g_telit_set_current_usb_port(USB3P0);

    return (PASSED);

__exit:
    plug_NR_5g_telit_obj_p->base.dev_object_fvt->dev_destroy((dev_object_t **)
                                                           &plug_NR_5g_telit_obj);
    plug_NR_5g_telit_set_current_usb_port(USB3P0);
    return (FAILED);

}


/*******************************************************************************
 * Function    : plug_NR_5g_telit_show_testmode_stat_util
 * Description : Function to show Telit modem testmode status.
 * Inputs      : opt - reserved for future use.
 * Outputs     : PASSED/FAILED
 *******************************************************************************
 */
static int plug_NR_5g_telit_show_testmode_stat_util (int opt)
{
    int rc = FALSE;
    dev_NR_5g_telit_object_t plug_NR_5g_telit_obj;
    dev_NR_5g_telit_object_t *plug_NR_5g_telit_obj_p = &plug_NR_5g_telit_obj;

    /* Create device object */
    if (plug_NR_5g_telit_dev_create(plug_NR_5g_telit_obj_p) != PASSED) {
        printf("Create Telit Dev Object Fails\n");
        return (FAILED);
    }

    /* Show modem testmode status */
    rc = plug_NR_5g_telit_obj_p->callin_fvt->modem_in_operation_mode(
                                           (dev_object_t *)&plug_NR_5g_telit_obj);
    if (rc == TRUE) {
        printf("Modem is in operation mode.\n");
    } else {
        printf("Modem is in test mode.\n");
    }

    plug_NR_5g_telit_obj_p->base.dev_object_fvt->dev_destroy((dev_object_t **)
                                                           &plug_NR_5g_telit_obj);

    return (PASSED);
}


/*******************************************************************************
 * Function   : plug_NR_5G_show_modem_info_util
 * Description: Function to dump the modem info(FW / Carrier / PRI / modem SN)
 * Inputs     : input - not used
 * Outputs    : PASSED or FAILED
 *******************************************************************************
 */
static int plug_NR_5g_telit_show_modem_info_util (int input)
{
    dev_NR_5g_telit_object_t plug_NR_5g_telit_obj;
    dev_NR_5g_telit_object_t *plug_NR_5g_telit_obj_p = &plug_NR_5g_telit_obj;
    int rc = FAILED;

    if (plug_NR_5g_telit_dev_create(plug_NR_5g_telit_obj_p) != PASSED) {
        cterr('f', 0, "Create Telit Dev Object Fails");
        return (FAILED);
    }

    rc = plug_NR_5g_telit_obj_p->callin_fvt->modem_dump_info(
                                           (dev_object_t *)&plug_NR_5g_telit_obj);

    plug_NR_5g_telit_obj_p->base.dev_object_fvt->dev_destroy((dev_object_t **)
                                                           &plug_NR_5g_telit_obj);
    return (rc);
}


/*******************************************************************************
 * Function    : plug_NR_5g_telit_enable_op_mode_util
 * Description : Function to enable Telit modem operation mode.
 * Inputs      : opt - reserved for future use.
 * Outputs     : PASSED/FAILED
 *******************************************************************************
 */
int plug_NR_5g_telit_enable_op_mode_util (int opt)
{
    int rc = FAILED;
    dev_NR_5g_telit_object_t plug_NR_5g_telit_obj;
    dev_NR_5g_telit_object_t *plug_NR_5g_telit_obj_p = &plug_NR_5g_telit_obj;

    /* Create device object */
    if (plug_NR_5g_telit_dev_create(plug_NR_5g_telit_obj_p) != PASSED) {
        printf("Create Telit Dev Object Fails\n");
        return (FAILED);
    }

    rc = plug_NR_5g_telit_obj_p->callin_fvt->modem_enable_op_mode(
                                           (dev_object_t *)&plug_NR_5g_telit_obj);
    if (rc != PASSED) {
        printf("Failed to enable modem operation mode\n");
    } else {
        printf("\nDone.\n");
    }

    plug_NR_5g_telit_obj_p->base.dev_object_fvt->dev_destroy((dev_object_t **)
                                                           &plug_NR_5g_telit_obj);
    /* Modem will reboot while switching modem test mode status */
    if (rc == PASSED) {
        printf("Wait %d seconds to reset modem...\n", TELIT_NR_5G_RST_DELAY);
        sleep(TELIT_NR_5G_RST_DELAY);
    }

    return (rc);
}

/*******************************************************************************
 * Function    : plug_NR_5g_telit_enable_fast_shdn_util
 * Description : Function to enable Telit modem fast shutdown feature.
 * Inputs      : None.
 * Outputs     : PASSED/FAILED
 *******************************************************************************
 */
static int plug_NR_5g_telit_enable_fast_shdn_util (void)
{
    printf ("\nNot supported");
    return (PASSED);
}


/*******************************************************************************
 * Function    : plug_NR_5g_telit_dump_modem_usb_connection_util
 * Description : Function to dump modem basic info.
 * Inputs      : None.
 * Outputs     : PASSED/FAILED
 *******************************************************************************
 */
static int plug_NR_5g_telit_dump_modem_usb_connection_util (void)
{
    int modem_found = FALSE, modem_uport = -1;

    /* Searching modem */
    plug_NR_5g_telit_modem_searching(&modem_found, &modem_uport);

    if (modem_found != TRUE) {
        printf("Modem is not detected.\n");
        return (FAILED);
    }

    printf("\nModem USB mode: %s \nModem connects to: %s.\n",
           (modem_uport == USB3P0)? SUPER_SPD_STR:HIGH_SPD_STR,
           (modem_uport == DEBUG_USB)? DPORT_STR:HOST_CONN_STR);

    return (PASSED);
}


/*******************************************************************************
 * Function    : plug_NR_5g_telit_set_modem_power_saving_mode_util
 * Description : Function to configure modem power saving mode.
 * Inputs      : None.
 * Outputs     : PASSED/FAILED
 *******************************************************************************
 */
static int plug_NR_5g_telit_set_modem_power_saving_mode_util (void)
{
    uint opt = 0;
    int set_psav_rc = FAILED;
    dev_NR_5g_telit_object_t plug_NR_5g_telit_obj;
    dev_NR_5g_telit_object_t *plug_NR_5g_telit_obj_p = &plug_NR_5g_telit_obj;

    printf("\n");
    printf("Pluggable Telit Configure Modem Power Saving Mode util:\n");
    printf("0. LPM(Low Power Mode)\n");
    printf("1. Power saving mode\n");
    printf("2. ignore on W_DISABLE\n");
    printf("10. Dying Gasp\n");
    printf("Others to exit\n");
    opt = getdec_answer("Select Toggle (0~2 or 10): ", 0, 0, 10);
    if ((opt != MODEM_LPM) && (opt != MODEM_PWR_SAV_MODE) &&
        (opt != MODEM_IGNORE) && (opt != MODEM_DYING_GASP)) {
        printf("Exit.\n");
        return (FAILED);
    }

    if (plug_NR_5g_telit_dev_create(plug_NR_5g_telit_obj_p) != PASSED) {
        printf("Create Telit Dev Object Fails\n");
        return (FAILED);
    }
    
    set_psav_rc = plug_NR_5g_telit_obj_p->callin_fvt->modem_pwrsaving_mode_ctrl(
                                                    (dev_object_t *)
                                                    &plug_NR_5g_telit_obj,
                                                    opt);
    if (set_psav_rc != PASSED) {
        printf("Failed to set modem power saving mode.\n");
    }
    
    plug_NR_5g_telit_obj_p->base.dev_object_fvt->dev_destroy((dev_object_t **)
                                                           &plug_NR_5g_telit_obj);
    return (set_psav_rc);
}


/*******************************************************************************
 * Function   : plug_NR_5g_telit_multi_led_ctrl_util
 * Description: LED Utility to control all LEDs on Pluggable module
 * Inputs     : input - Not used
 * Outputs    : PASSED or FAILED
 *******************************************************************************
 */
static int plug_NR_5g_telit_multi_led_ctrl_util (int input)
{
    int ix = 0;
    int opt, color_opt;

    printf("Pluggable module Multi LEDs control utility.\n");
    opt = getdec_answer("Light up/off Register? (0-off, 1-on):", OPT_OFF, 
                        OPT_OFF, OPT_ON);
    if (opt == OPT_ON) {    
        color_opt = getdec_answer("Yellow/Green? (0-green, 1-yellow):",
                                  OPT_GREEN, OPT_GREEN, OPT_YELLOW);

        if (color_opt == OPT_GREEN) {
            printf("Turn all pluggable NR_5G green LEDs on.\n");
            /* Pull yellow led registers low */
            for (ix = 0; ix < plug_NR_5g_telit_y_led_sz; ix++) {
                plug_NR_5g_telit_toggle_led(
                               plug_NR_5g_telit_y_led_table[ix].dev_fun,
                               plug_NR_5g_telit_y_led_table[ix].drive_off);
            }
            for (ix = 0; ix < plug_NR_5g_telit_g_led_sz; ix++) {
                plug_NR_5g_telit_toggle_led(
                               plug_NR_5g_telit_g_led_table[ix].dev_fun,
                               plug_NR_5g_telit_g_led_table[ix].drive_on);
            }

            for (ix = 0; ix < plug_NR_5g_telit_rssi_led_sz; ix++) {
                plug_NR_5g_telit_toggle_led(
                               plug_NR_5g_telit_rssi_led_table[ix].dev_fun,
                               plug_NR_5g_telit_rssi_led_table[ix].drive_off);
            }
            plug_NR_5g_telit_toggle_led(
                           plug_NR_5g_telit_rssi_led_table[0].dev_fun,
                           plug_NR_5g_telit_rssi_led_table[0].drive_on);

        } else {
            printf("Turn all pluggable NR_5G yellow LEDs on.\n");
            /* Pull green led registers low */
            for (ix = 0; ix < plug_NR_5g_telit_g_led_sz; ix++) {
                plug_NR_5g_telit_toggle_led(
                               plug_NR_5g_telit_g_led_table[ix].dev_fun,
                               plug_NR_5g_telit_g_led_table[ix].drive_off);
            }
            for (ix = 0; ix < plug_NR_5g_telit_y_led_sz; ix++) {
                plug_NR_5g_telit_toggle_led(
                               plug_NR_5g_telit_y_led_table[ix].dev_fun,
                               plug_NR_5g_telit_y_led_table[ix].drive_on);
            }

            for (ix = 0; ix < plug_NR_5g_telit_rssi_led_sz; ix++) {
                plug_NR_5g_telit_toggle_led(
                               plug_NR_5g_telit_rssi_led_table[ix].dev_fun,
                               plug_NR_5g_telit_rssi_led_table[ix].drive_off);
            }  
            plug_NR_5g_telit_toggle_led(
                           plug_NR_5g_telit_rssi_led_table[1].dev_fun,
                           plug_NR_5g_telit_rssi_led_table[1].drive_on);

        }
    } else {
        printf("Turn all pluggable NR_5G LEDs off.\n");
        for (ix = 0; ix < plug_NR_5g_telit_g_led_sz; ix++) {
            plug_NR_5g_telit_toggle_led(
                           plug_NR_5g_telit_g_led_table[ix].dev_fun,
                           plug_NR_5g_telit_g_led_table[ix].drive_off);
        }
        for (ix = 0; ix < plug_NR_5g_telit_y_led_sz; ix++) {
            plug_NR_5g_telit_toggle_led(
                           plug_NR_5g_telit_y_led_table[ix].dev_fun,
                           plug_NR_5g_telit_y_led_table[ix].drive_off);
        }

        for (ix = 0; ix < plug_NR_5g_telit_rssi_led_sz; ix++) {
            plug_NR_5g_telit_toggle_led(
                           plug_NR_5g_telit_rssi_led_table[ix].dev_fun,
                           plug_NR_5g_telit_rssi_led_table[ix].drive_off);
        }

    }
    return (PASSED);
}


/*******************************************************************************
 * Function    : plug_NR_5G_led_ctrl_util
 * Description : Function to show Pluggable LED Control utilities submenu.
 * Inputs      : opt - reserved for future use.
 * Outputs     : PASSED/FAILED
 *******************************************************************************
 */
int plug_NR_5g_telit_led_ctrl_util (int opt)
{
    build_primary_submenu(plug_NR_5g_telit_led_ctrl_tbl,
                          PLUG_5G_NR_SUB6_TELIT_LED_CTRL_TBL_SZ,
                          "Plug LED Control Utilities",
                          &plug_NR_5g_telit_led_ctrl_menup);

    build_secondary_submenu(plug_NR_5g_telit_led_ctrl_tbl,
                            PLUG_5G_NR_SUB6_TELIT_LED_CTRL_TBL_SZ,
                            plug_NR_5g_telit_led_ctrl_sec_items);

    menu(plug_NR_5g_telit_led_ctrl_menup, plug_NR_5g_telit_led_ctrl_sec_items, '\0');

    return (PASSED);
}


/*******************************************************************************
 * Function    : plug_NR_5g_telit_enable_led_utils
 * Description : Function to turn Pluggable modem enable LED ON/OFF.
 * Inputs      : opt - reserved for future use.
 * Outputs     : PASSED/FAILED
 *******************************************************************************
 */
int plug_NR_5g_telit_enable_led_utils (int opt)
{
    uint option = 0;
    int ix;

    plug_NR_5G_led_ctrl plug_NR_5g_telit_led_ctrl_reqt[] = {
        {ENABLE_LED_YELLOW, LOW},
        {ENABLE_LED_GREEN, LOW}
    };

    int max_enable_led_ctrl_bit = 
        sizeof(plug_NR_5g_telit_led_ctrl_reqt)/sizeof(plug_NR_5G_led_ctrl);

    printf("\n");
    printf("Pluggable Enable LED utils:\n");
    printf("0. OFF\n");
    printf("1. Yellow\n");
    printf("2. Green\n");
    option = getdec_answer("Select Toggle (0~2): ", 0, 0, 2);

    if (option == 0){
        plug_NR_5g_telit_led_ctrl_reqt[0].val = LOW;
        plug_NR_5g_telit_led_ctrl_reqt[1].val = LOW;
    } else if (option == 1) {
        plug_NR_5g_telit_led_ctrl_reqt[0].val = HIGH;
        plug_NR_5g_telit_led_ctrl_reqt[1].val = LOW;
    } else if (option == 2) {
        plug_NR_5g_telit_led_ctrl_reqt[0].val = LOW;
        plug_NR_5g_telit_led_ctrl_reqt[1].val = HIGH;
    } else {
        printf("No selection toggle\n");
        return (FAILED);
    }

    for (ix = 0; ix < max_enable_led_ctrl_bit; ix++) {
        if (plug_NR_5g_telit_toggle_led(plug_NR_5g_telit_led_ctrl_reqt[ix].dev,
                                      plug_NR_5g_telit_led_ctrl_reqt[ix].val)
                                      == FAILED) {
            printf("%s: Failed to toggle GPIO exp.\n", __func__);
            return (FAILED);
        }    
    }

    return (PASSED);
}


/*******************************************************************************
 * Function    : plug_NR_5g_telit_sim_status_led_utils
 * Description : Function to turn Pluggable modem SIM0/1 Status LED ON/OFF.
 * Inputs      : opt - reserved for future use.
 * Outputs     : PASSED/FAILED
 *******************************************************************************
 */
int plug_NR_5g_telit_sim_status_led_utils (int opt)
{
    uint option = 0;
    int ix, sim_opt = 0;

    plug_NR_5G_led_ctrl plug_NR_5g_telit_led_ctrl_reqt[] = {
        {LED_SIM0_NOT_OK, LOW},
        {LED_SIM0_OK, LOW}
    };

    printf("\n");
    printf("Pluggable SIM Status LED utils: \n");
    sim_opt = getdec_answer("Control which LED? (0-SIM0, 1-SIM1):",
                            0, 0, 1);
    

    if (sim_opt == 0) {
        plug_NR_5g_telit_led_ctrl_reqt[0].dev = LED_SIM0_NOT_OK;
        plug_NR_5g_telit_led_ctrl_reqt[1].dev = LED_SIM0_OK;
    } else {
        plug_NR_5g_telit_led_ctrl_reqt[0].dev = LED_SIM1_NOT_OK;
        plug_NR_5g_telit_led_ctrl_reqt[1].dev = LED_SIM1_OK;
    }

    int max_sim_led_ctrl_bit = 
        sizeof(plug_NR_5g_telit_led_ctrl_reqt)/sizeof(plug_NR_5G_led_ctrl);

    printf("0. OFF\n");
    printf("1. Yellow\n");
    printf("2. Green\n");
    option = getdec_answer("Select Toggle (0~2):", 0, 0, 2);

    if (option == 0) {
        plug_NR_5g_telit_led_ctrl_reqt[0].val = LOW;
        plug_NR_5g_telit_led_ctrl_reqt[1].val = LOW;
    } else if (option == 1) {
        plug_NR_5g_telit_led_ctrl_reqt[0].val = HIGH;
        plug_NR_5g_telit_led_ctrl_reqt[1].val = LOW;
    } else if (option == 2) {
        plug_NR_5g_telit_led_ctrl_reqt[0].val = LOW;
        plug_NR_5g_telit_led_ctrl_reqt[1].val = HIGH;
    } else {
        printf("No selection toggle\n");
        return (FAILED);
    }

    for (ix = 0; ix < max_sim_led_ctrl_bit; ix++) {
        if (plug_NR_5g_telit_toggle_led(plug_NR_5g_telit_led_ctrl_reqt[ix].dev,
                                      plug_NR_5g_telit_led_ctrl_reqt[ix].val)
                                      == FAILED) {
            printf("%s: Failed to toggle GPIO exp.\n", __func__);
            return (FAILED);
        }
    }

    return (PASSED);
}


/*******************************************************************************
 * Function    : plug_NR_5g_telit_gps_status_led_utils
 * Description : Function to turn Pluggable modem GPS LED ON/OFF.
 * Inputs      : opt - reserved for future use.
 * Outputs     : PASSED/FAILED
 *******************************************************************************
 */
int plug_NR_5g_telit_gps_status_led_utils (int opt)
{
    uint option = 0;
    int ix;

    plug_NR_5G_led_ctrl plug_NR_5g_telit_led_ctrl_reqt[] = {
        {LED_GPS_NOT_OK, LOW},
        {LED_GPS_OK, LOW}
    };

    int max_gps_led_ctrl_bit =
        sizeof(plug_NR_5g_telit_led_ctrl_reqt)/sizeof(plug_NR_5G_led_ctrl);

    printf("\n");
    printf("Pluggable GPS LED utils: \n");
    printf("0. OFF\n");
    printf("1. Yellow\n");
    printf("2. Green\n");
    option = getdec_answer("Select Toggle (0~2): ", 0, 0, 2);

    if (option == 0) {
        plug_NR_5g_telit_led_ctrl_reqt[0].val = LOW;
        plug_NR_5g_telit_led_ctrl_reqt[1].val = LOW;
    } else if (option == 1 ) {
        plug_NR_5g_telit_led_ctrl_reqt[0].val = HIGH;
        plug_NR_5g_telit_led_ctrl_reqt[1].val = LOW;
    } else if (option == 2 ) {
        plug_NR_5g_telit_led_ctrl_reqt[0].val = LOW;
        plug_NR_5g_telit_led_ctrl_reqt[1].val = HIGH;
    } else {
        printf("No selection toggle\n");
        return (FAILED);
    }

    for (ix = 0; ix < max_gps_led_ctrl_bit; ix++) {
        if (plug_NR_5g_telit_toggle_led(plug_NR_5g_telit_led_ctrl_reqt[ix].dev,
                                      plug_NR_5g_telit_led_ctrl_reqt[ix].val)
                                      == FAILED) {
            printf("%s: Failed to toggle GPIO exp.\n", __func__);
            return (FAILED);
        }
    }

    return (PASSED);
}


/*******************************************************************************
 * Function    : plug_NR_5g_telit_rssi_led_utils
 * Description : Function to turn Pluggable modem RSSI0/1/2/3 LED ON/OFF.
 * Inputs      : opt - reserved for future use.
 * Outputs     : PASSED / FAILED
 *******************************************************************************
 */
int plug_NR_5g_telit_rssi_led_utils (int opt)
{
    uint option = 0;
    int ix;

    plug_NR_5G_led_ctrl plug_NR_5g_telit_led_ctrl_reqt[] = {
        {LED_RSSI0, LOW}, //Green
        {LED_RSSI1, LOW}, //Red
        {LED_RSSI2, LOW}, //Blue
    };

    int max_rssi_led_ctrl_bit =
        sizeof(plug_NR_5g_telit_led_ctrl_reqt)/sizeof(plug_NR_5G_led_ctrl);

    printf("\n"); 
    printf("Pluggable RSSI Status LED utils: \n"); 
    printf("0. Service Indication LED OFF\n");
    printf("1. Service Indication LED GREEN\n");
    printf("2. Service Indication LED RSSI YELLOW\n");
    printf("3. Service Indication LED RSSI BLUE\n");

    option = getdec_answer("Select Toggle (0 ~ 3): ", 0, 0, 3);
   

    switch (option) {
        case 0:
            //Do nothing. 
            break;
        case 1:
            plug_NR_5g_telit_led_ctrl_reqt[0].val = HIGH;
            break;
        case 2:
            plug_NR_5g_telit_led_ctrl_reqt[1].val = HIGH;
            break;
        case 3:
            plug_NR_5g_telit_led_ctrl_reqt[2].val = HIGH;
            break;
        default:
            printf("%s: No selection toggle\n", __func__);
            return (FAILED);
    }

    /* Turn off all RSSI LEDs first */
    for (ix = 1; ix < max_rssi_led_ctrl_bit; ix++) {
        if (plug_NR_5g_telit_toggle_led(plug_NR_5g_telit_led_ctrl_reqt[ix].dev,
                                      LOW) == FAILED) {
            printf("%s: Failed to toggle GPIO exp.\n", __func__);
            return(FAILED);
        }
    }

    for (ix = 0; ix < max_rssi_led_ctrl_bit; ix++) {
        if (plug_NR_5g_telit_toggle_led(plug_NR_5g_telit_led_ctrl_reqt[ix].dev,
                                      plug_NR_5g_telit_led_ctrl_reqt[ix].val)
                                      == FAILED) {
            printf("%s: Failed to toggle GPIO exp.\n", __func__);
            return(FAILED);
        }
    }
    
    return (PASSED);
}


/*******************************************************************************
 * Function    : plug_NR_5g_telit_wan_led_util
 * Description : Function to show Pluggable WAN LED Control utilities
 *               submenu.
 *               Due to the AT command to control WAN_LED signal is to set the
 *               LED blinking pattern for each network mode, thus, we set the 
 *               LED blinking pattern for LPM and switch modem to LPM to verify
 *               WAN_LED.
 * Inputs      : opt - reserved for future use.
 * Outputs     : PASSED/FAILED
 *******************************************************************************
 */
static int plug_NR_5g_telit_wan_led_util (int opt)
{
    int ix, rc = FAILED;
    int set_psav_rc = FAILED;
    dev_NR_5g_telit_object_t plug_NR_5g_telit_obj;
    dev_NR_5g_telit_object_t *plug_NR_5g_telit_obj_p = &plug_NR_5g_telit_obj;

    testname("WWAN_LED control utility");

    /* Disable WWAN_LED */
    plug_NR_5g_telit_wwan_led_output_enable_ctrl(HIGH);

    /* Set the output of GPIO expander which connects to SIM0/1 LEDs to low */
    for (ix = 0; ix < plug_NR_5g_telit_sim_led_sz; ix++) {
       plug_NR_5g_telit_toggle_led(plug_NR_5g_telit_sim_led_table[ix].dev_fun,
                                 plug_NR_5g_telit_sim_led_table[ix].drive_off);
    }
    
    /* Switch modem to LPM */
    /* Since we can only set the WAN_LED blinking pattern for each modem 
     * network status, and that need use LPM modem for diag testing. */
    if (plug_NR_5g_telit_dev_create(plug_NR_5g_telit_obj_p) != PASSED) {
        printf("Create Telit Dev Object Fails\n");
        return (FAILED);
    }

    /* W_DISABLE1# pin is used to switch modem between operation mode and power
     * saving mode. And the modem power saving mode can be configured as LPM
     * (Low Power Mode), Power Saving Mode, DG(Dying Gasp) or no event to be
     * performed. In this test, we configured modem power saving mode as LPM */
    /* Check whether the modem power saving modem is configured as LPM */
    prpass(testpass, "Set modem power saving configuration...");
    set_psav_rc = plug_NR_5g_telit_obj_p->callin_fvt->
                                        modem_pwrsaving_mode_ctrl(
                                        (dev_object_t *)
                                        &plug_NR_5g_telit_obj,
                                        MODEM_LPM);
    if (set_psav_rc != PASSED) {
        printf("Failed to set modem power saving mode.\n");
        plug_NR_5g_telit_obj_p->base.dev_object_fvt->dev_destroy(
                                                   (dev_object_t **)
                                                   &plug_NR_5g_telit_obj);
        return (FAILED);
    }

    plug_NR_5g_telit_w_disable1_ctrl(HIGH);

    build_primary_submenu(plug_NR_5g_telit_wan_led_ctrl_tbl,
                          PLUG_5G_NR_SUB6_TELIT_WAN_LED_CTRL_TBL_SZ,
                          "Plug WAN LED Control Utilities",
                          &plug_NR_5g_telit_wan_led_ctrl_menup);

    build_secondary_submenu(plug_NR_5g_telit_wan_led_ctrl_tbl,
                            PLUG_5G_NR_SUB6_TELIT_WAN_LED_CTRL_TBL_SZ,
                            plug_NR_5g_telit_wan_led_ctrl_sec_items);

    menu(plug_NR_5g_telit_wan_led_ctrl_menup,
         plug_NR_5g_telit_wan_led_ctrl_sec_items, '\0');

    /* Switch modem back to operation mode */
    plug_NR_5g_telit_w_disable1_ctrl(LOW);

    /* Disable WWAN_LED */
    plug_NR_5g_telit_wwan_led_output_enable_ctrl(HIGH);

    /* Restore WWAN_LED blinking pattern to default */
    if (plug_NR_5g_telit_dev_create(plug_NR_5g_telit_obj_p) != PASSED) {
        printf("Create Telit Dev Object Fails\n");
        return (FAILED);
    }

    rc = plug_NR_5g_telit_obj_p->callin_fvt->modem_lpm_wwan_led_ctrl(
                                           (dev_object_t *)&plug_NR_5g_telit_obj,
                                            LED_DEFAULT);
    if (rc != PASSED) {
        printf("Failed to set WAN_LED blinking pattern\n");
    }

    plug_NR_5g_telit_obj_p->base.dev_object_fvt->dev_destroy((dev_object_t **)
                                                           &plug_NR_5g_telit_obj);

    return (rc);
}


/*******************************************************************************
 * Function   : plug_NR_5g_telit_wan_led_on_off_util
 * Description: LED Utility to turn on/off WAN LED on Pluggable module
 *              WAN_LED is a output signal driven by modem to SIM0/1 LED,
 *              which is determined by WWAN_LED_SIM_SEL pin on GPIO expander.
 * Inputs     : input - Not used
 * Outputs    : PASSED or FAILED
 *******************************************************************************
 */
static int plug_NR_5g_telit_wan_led_on_off_util (int input)
{
    int opt, rc = FAILED;
    dev_NR_5g_telit_object_t plug_NR_5g_telit_obj;
    dev_NR_5g_telit_object_t *plug_NR_5g_telit_obj_p = &plug_NR_5g_telit_obj;

    printf("Pluggable WAN LED control utility.\n");
    opt = getdec_answer("Turn on/off WWAN_LED? (0-off, 1-on):", LED_OFF, 
                        LED_OFF, LED_ON);

    if (plug_NR_5g_telit_dev_create(plug_NR_5g_telit_obj_p) != PASSED) {
        printf("Create Telit Dev Object Fails\n");
        return (FAILED);
    }

    rc = plug_NR_5g_telit_obj_p->callin_fvt->modem_lpm_wwan_led_ctrl(
                                           (dev_object_t *)&plug_NR_5g_telit_obj,
                                            opt);
    if (rc != PASSED) {
        printf("Failed to set WAN_LED blinking pattern\n");
    }

    /* Enable WWAN_LED signal */
    printf("Enable WWAN_LED\n");
    plug_NR_5g_telit_wwan_led_output_enable_ctrl(LOW);

    plug_NR_5g_telit_obj_p->base.dev_object_fvt->dev_destroy((dev_object_t **)
                                                           &plug_NR_5g_telit_obj);

    return (rc);
}


/*******************************************************************************
 * Function   : plug_NR_5g_telit_wan_led_sel_util
 * Description: LED Utility to verify the WWAN_LED_SIM_SEL pin
 * Inputs     : input - Not used
 * Outputs    : PASSED or FAILED
 *******************************************************************************
 */
static int plug_NR_5g_telit_wan_led_sel_util (int input)
{
    int rc = FAILED;
    int opt;

    printf("Pluggable WWAN_LED_SEL pin utility.\n");
    opt = getdec_answer("Route WWAN_LED signal to which SIM LED? (0-SIM0, "
                        "1-SIM1):", SIM_0, SIM_0, SIM_1);

    rc = plug_NR_5g_telit_wwan_led_sim_select(opt);

    if (rc != PASSED) {
        printf("Failed to set WWAN_LED_SIM_SEL pin\n");
    }

    return (rc);
}

/*******************************************************************************
 * Function   : plug_NR_5g_telit_select_band_to_test_rssi
 * Description: Select Band to test Antennas in modem antenna test
 * Inputs     : input - Not used
 * Outputs    : PASSED or FAILED
 *******************************************************************************
 */
static int plug_NR_5g_telit_select_band_to_test_rssi(void){
    int i;

    printf ("Supported NR-SUB6 bands are : \n");
    
    for (i =0 ;i < band_tbl_size; i++){
        printf ("%d\t", nr_sub6_band_tbl[i].band_num);
    }

    band_to_test = getdec_answer("\nEnter band number to test ANT RX " , 41, 1, 79);
    
    return (PASSED);
}

/*******************************************************************************
 * Function   : plug_test_not_supported
 * Description: Function to mask the test in the test list
 * Inputs     : not used
 * Outputs    : return FALSE 
 *******************************************************************************
 */
boolean plug_test_not_supported (void){
    return FALSE;
}

/*********************************************************************
 * $Log: plug_NR_5G_telit_util.c,v $
 * Revision 1.2  2021/06/02 02:56:20  alpeng
 * merge sears into trunk
 *
 * Revision 1.1.2.5  2021/02/27 00:43:08  tshanmug
 * Sears code cleanup
 *
 * Revision 1.1.2.4  2021/02/12 01:08:19  tshanmug
 * Sears multi band test support
 *
 * Revision 1.1.2.3  2020/12/02 03:57:23  tshanmug
 * Sears Antenna test updated
 *
 *
 * $Endlog$
 */
