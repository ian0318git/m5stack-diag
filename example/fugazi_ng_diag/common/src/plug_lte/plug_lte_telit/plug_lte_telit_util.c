/* $Id: plug_lte_telit_util.c,v 1.6 2019/07/01 10:05:25 sherliu2 Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/plug_lte/plug_lte_telit/plug_lte_telit_util.c,v $
 *------------------------------------------------------------------
 *
 * plug_lte_telit_util.c - Pluggable LTE Telit Utility
 *
 * Copyright (c) 2018 - 2019 by Cisco Systems, Inc.
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
#include "plug_lte_telit_util.h"
#include "plug_lte_telit_lib.h"

int plug_lte_telit_util(void);
int plug_lte_telit_modem_temp_util(int);

static int plug_lte_telit_run_at_cmd(int);
static int plug_lte_telit_man_gpio_exp_util(void);
static int plug_lte_telit_opt_gpio_exp_util(void);
static int plug_lte_telit_ext_usb_util(int);
static int plug_lte_telit_show_temp(int);
static int plug_lte_telit_ts_util(int);
static int plug_lte_telit_modem_hd_reset(int);
static int plug_lte_telit_modem_reboot_util(int);
static int plug_lte_telit_led_ctrl_util(int);
static int plug_lte_telit_multi_led_ctrl_util(int);
static int plug_lte_telit_simdetect_pin_test(int);
static int plug_lte_telit_show_simdetect_pin_status(int);
static int plug_lte_telit_enable_led_utils(int);
static int plug_lte_telit_sim_status_led_utils(int);
static int plug_lte_telit_gps_status_led_utils(int);
static int plug_lte_telit_rssi_led_utils(int);
static int plug_lte_telit_wan_led_util(int);
static int plug_lte_telit_wan_led_on_off_util(int);
static int plug_lte_telit_wan_led_sel_util(int);
static int lte_telit_simdet_pin_test(int, boolean, boolean);
static int plug_lte_telit_usb_mode_switch_util(int);
static int plug_lte_telit_show_testmode_stat_util(int);
static int plug_lte_telit_show_modem_info_util(int);
static int plug_lte_telit_enable_op_mode_util(int);
static int plug_lte_telit_enable_fast_shdn_util(void);
static int plug_lte_telit_enable_dying_gasp_util(void);
static int plug_lte_telit_dump_modem_usb_connection_util(void);
static int plug_lte_telit_set_modem_power_saving_mode_util(void);

static submenu_xtable_t pluggable_lte_telit_utils[] = {
    {"AT Command Utility", 
     (type_t(*)())plug_lte_telit_run_at_cmd, TRUE,
     0, (type_t(*)())0, 0, (type_t(*)())0, 0},
    {"Mandatory PCA9555 (0x4E) Register Read/Write Utility", 
     (type_t(*)())plug_lte_telit_man_gpio_exp_util, TRUE,
     0, (type_t(*)())0, 0, (type_t(*)())0, 0},
    {"Optional PCA9555 (0x4C) Register Read/Write Utility", 
     (type_t(*)())plug_lte_telit_opt_gpio_exp_util, TRUE,
     0, (type_t(*)())0, 0, (type_t(*)())0, 0},
    {"External USB Debug bus Enable/Disable Utility", 
     (type_t(*)())plug_lte_telit_ext_usb_util, TRUE,
     0, (type_t(*)())0, 0, (type_t(*)())0, 0},
    {"Temperature Display via Thermal Sensor Utility", 
     (type_t(*)())plug_lte_telit_show_temp, TRUE,
     0, (type_t(*)())plug_lte_telit_has_temp_sensor, 0, (type_t(*)())0, 0},
    {"Temperature Sensor Register Read/Write Utility", 
     (type_t(*)())plug_lte_telit_ts_util, TRUE,
     0, (type_t(*)())plug_lte_telit_has_temp_sensor, 0, (type_t(*)())0, 0},
    {"LTE Modem Temperature Display Utility", 
     (type_t(*)())plug_lte_telit_modem_temp_util, TRUE,
     0, (type_t(*)())0, 0, (type_t(*)())0, 0},
    {"LTE Modem Hard Reset Utility(Emergency use)", 
     (type_t(*)())plug_lte_telit_modem_hd_reset, TRUE,
     0, (type_t(*)())0, 0, (type_t(*)())0, 0},
    {"LTE Modem Reboot Utility", 
     (type_t(*)())plug_lte_telit_modem_reboot_util, TRUE,
     0, (type_t(*)())0, 0, (type_t(*)())0, 0},
    {"LED Control Utilities", 
     (type_t(*)())plug_lte_telit_led_ctrl_util, TRUE,
     0, (type_t(*)())0, 0, (type_t(*)())0, 0},
    {"LTE Modem SIM_DETECT pin Test(SIM0)", 
     (type_t(*)())plug_lte_telit_simdetect_pin_test, SIM0,
     0, (type_t(*)())0, 0, (type_t(*)())0, 0},
    {"LTE Modem SIM_DETECT pin Test(SIM1)", 
     (type_t(*)())plug_lte_telit_simdetect_pin_test, SIM1,
     0, (type_t(*)())0, 0, (type_t(*)())0, 0},
    {"Show LTE Modem SIM_DETECT pin Status(SIM0)", 
     (type_t(*)())plug_lte_telit_show_simdetect_pin_status, SIM0,
     0, (type_t(*)())0, 0, (type_t(*)())0, 0},
    {"Show LTE Modem SIM_DETECT pin Status(SIM1)", 
     (type_t(*)())plug_lte_telit_show_simdetect_pin_status, SIM1,
     0, (type_t(*)())0, 0, (type_t(*)())0, 0},
    {"Modem USB Mode Switching Utility", 
     (type_t(*)())plug_lte_telit_usb_mode_switch_util, TRUE,
     0, (type_t(*)())0, 0, (type_t(*)())0, 0},
    {"Show Modem TestMode Status Utility", 
     (type_t(*)())plug_lte_telit_show_testmode_stat_util, TRUE,
     0, (type_t(*)())0, 0, (type_t(*)())0, 0},
    {"Enable Modem Operation Mode Utility", 
     (type_t(*)())plug_lte_telit_enable_op_mode_util, TRUE,
     0, (type_t(*)())0, 0, (type_t(*)())0, 0},
    {"Enable Modem Fast Shutdown Utility", 
     (type_t(*)())plug_lte_telit_enable_fast_shdn_util, TRUE,
     0, (type_t(*)())0, 0, (type_t(*)())0, 0},
    {"Enable Modem Dying Gasp Utility", 
     (type_t(*)())plug_lte_telit_enable_dying_gasp_util, TRUE,
     0, (type_t(*)())0, 0, (type_t(*)())0, 0},
    {"Dump Modem USB Connection Info Utility", 
     (type_t(*)())plug_lte_telit_dump_modem_usb_connection_util, TRUE,
     0, (type_t(*)())0, 0, (type_t(*)())0, 0},
    {"Show Modem Information Utility", 
     (type_t(*)())plug_lte_telit_show_modem_info_util, TRUE,
     0, (type_t(*)())0, 0, (type_t(*)())0, 0},
    {"Configure Modem Power Saving Mode Utility", 
     (type_t(*)())plug_lte_telit_set_modem_power_saving_mode_util, TRUE,
     0, (type_t(*)())0, 0, (type_t(*)())0, 0},
};
#define PLUG_LTE_TELIT_UTIL_TABLE_SZ \
        (sizeof(pluggable_lte_telit_utils) / sizeof(submenu_xtable_t))

static mitem_t plug_lte_telit_pri_util_items[PLUG_LTE_TELIT_UTIL_TABLE_SZ+
                                             MAX_BASE_ITEMS];
static mitem_t plug_lte_telit_sec_util_items[PLUG_LTE_TELIT_UTIL_TABLE_SZ+
                                             MAX_BASE_ITEMS];

static menuinfo_t plug_lte_telit_util_menu = {
    "Pluggable LTE Telit Utilities Menu",
    0,                            /* mtparam added by init_empty_menu */
    (PFT)menu_show_dflags,        /* shows major flags */
    0,                            /* use generic prompt */
    0,                            /* size (bumped by add_menu_item() */
    plug_lte_telit_pri_util_items,
};
static menuinfo_t *plug_lte_telit_util_menup = &plug_lte_telit_util_menu;

/*
 * LED Control
 */
static submenu_xtable_t plug_lte_telit_led_ctrl_tbl[] = {
    {"Enable LED utils", (type_t(*)())plug_lte_telit_enable_led_utils, 
      TRUE, 0, (type_t(*)())0, 0, (type_t(*)())0, 0},
    {"SIM Status LED utils", (type_t(*)())plug_lte_telit_sim_status_led_utils, 
      TRUE, 0, (type_t(*)())0, 0, (type_t(*)())0, 0},
    {"GPS Status LED utils", (type_t(*)())plug_lte_telit_gps_status_led_utils, 
      TRUE, 0, (type_t(*)())plug_lte_telit_has_dedicated_gps_antenna, 0,
      (type_t(*)())0, 0},
    {"RSSI Status LED utils", (type_t(*)())plug_lte_telit_rssi_led_utils, 
      TRUE, 0, (type_t(*)())0, 0, (type_t(*)())0, 0},
    {"WAN LED utils", (type_t(*)())plug_lte_telit_wan_led_util, 
      TRUE, 0, (type_t(*)())0, 0, (type_t(*)())0, 0},
    {"Multi LED Control Utility", (type_t(*)())plug_lte_telit_multi_led_ctrl_util, 
      TRUE, 0, (type_t(*)())0, 0, (type_t(*)())0, 0},
};

#define PLUG_LTE_TELIT_LED_CTRL_TBL_SZ \
        (sizeof(plug_lte_telit_led_ctrl_tbl) / sizeof(submenu_xtable_t))

static mitem_t plug_lte_telit_led_ctrl_pri_items[PLUG_LTE_TELIT_LED_CTRL_TBL_SZ+
                                                 MAX_BASE_ITEMS];
static mitem_t plug_lte_telit_led_ctrl_sec_items[PLUG_LTE_TELIT_LED_CTRL_TBL_SZ+
                                                 MAX_BASE_ITEMS];               

menuinfo_t plug_lte_telit_led_ctrl_menu = {
    "%s Menu",
    0,
    (PFT)show_endnote,
    0,
    0,
    plug_lte_telit_led_ctrl_pri_items,
};
menuinfo_t *plug_lte_telit_led_ctrl_menup = &plug_lte_telit_led_ctrl_menu;

/*
 * WAN LED Control
 */
static submenu_xtable_t plug_lte_telit_wan_led_ctrl_tbl[] = {
    {"WAN LED on/off utils", (type_t(*)())plug_lte_telit_wan_led_on_off_util, 
      TRUE, 0, (type_t(*)())0, 0, (type_t(*)())0, 0},
    {"WAN LED select pin utils", (type_t(*)())plug_lte_telit_wan_led_sel_util, 
      TRUE, 0, (type_t(*)())0, 0, (type_t(*)())0, 0},
};

#define PLUG_LTE_TELIT_WAN_LED_CTRL_TBL_SZ \
        (sizeof(plug_lte_telit_wan_led_ctrl_tbl) / sizeof(submenu_xtable_t))

static mitem_t plug_lte_telit_wan_led_ctrl_pri_items[
                                           PLUG_LTE_TELIT_WAN_LED_CTRL_TBL_SZ+
                                           MAX_BASE_ITEMS];
static mitem_t plug_lte_telit_wan_led_ctrl_sec_items[
                                           PLUG_LTE_TELIT_WAN_LED_CTRL_TBL_SZ+
                                           MAX_BASE_ITEMS];               

menuinfo_t plug_lte_telit_wan_led_ctrl_menu = {
    "%s Menu",
    0,
    (PFT)show_endnote,
    0,
    0,
    plug_lte_telit_wan_led_ctrl_pri_items,
};
menuinfo_t *plug_lte_telit_wan_led_ctrl_menup =
                                       &plug_lte_telit_wan_led_ctrl_menu;

static plug_lte_led_dev plug_lte_telit_g_led_table[] = {
    {ENABLE_LED_GREEN, HIGH, LOW},
    {LED_SIM0_OK, HIGH, LOW},
    {LED_SIM1_OK, HIGH, LOW},
    {LED_GPS_OK, HIGH, LOW}
};
static plug_lte_led_dev plug_lte_telit_y_led_table[] = {
    {ENABLE_LED_YELLOW, HIGH, LOW},
    {LED_SIM0_NOT_OK, HIGH, LOW},
    {LED_SIM1_NOT_OK, HIGH, LOW},
    {LED_GPS_NOT_OK, HIGH, LOW}
};
static plug_lte_led_dev plug_lte_telit_rssi_led_table[] = {
    {LED_RSSI0 , HIGH, LOW},
    {LED_RSSI1 , HIGH, LOW},
    {LED_RSSI2 , HIGH, LOW},
    {LED_RSSI3 , HIGH, LOW}
};

static plug_lte_led_dev plug_lte_telit_sim_led_table[] = {
    {LED_SIM0_OK, HIGH, LOW},
    {LED_SIM1_OK, HIGH, LOW},
    {LED_SIM0_NOT_OK, HIGH, LOW},
    {LED_SIM1_NOT_OK, HIGH, LOW},
};

int plug_lte_telit_g_led_sz = sizeof(plug_lte_telit_g_led_table)/
                              sizeof(plug_lte_led_dev);
int plug_lte_telit_y_led_sz = sizeof(plug_lte_telit_y_led_table)/
                              sizeof(plug_lte_led_dev);
int plug_lte_telit_rssi_led_sz = sizeof(plug_lte_telit_rssi_led_table)/
                                 sizeof(plug_lte_led_dev);
int plug_lte_telit_sim_led_sz = sizeof(plug_lte_telit_sim_led_table)/
                                sizeof(plug_lte_led_dev);

/*******************************************************************************
 * Function   : plug_lte_telit_util
 * Description: Main Entry point for Pluggable LTE Utilities
 * Inputs     : None
 * Outputs    : PASSED or FAILED
 *******************************************************************************
 */
int plug_lte_telit_util (void)
{   
    build_primary_submenu(pluggable_lte_telit_utils, 
                          PLUG_LTE_TELIT_UTIL_TABLE_SZ, 
                          "Pluggable LTE", &plug_lte_telit_util_menup);

    build_secondary_submenu(pluggable_lte_telit_utils, 
                            PLUG_LTE_TELIT_UTIL_TABLE_SZ,
                            plug_lte_telit_sec_util_items);

    menu(&plug_lte_telit_util_menu, plug_lte_telit_sec_util_items, '\0');

    return (PASSED);
}


/*******************************************************************************
 * Function   : plug_lte_telit_run_at_cmd
 * Description: To execute AT command for Pluggable LTE
 * Inputs     : input - Not used 
 * Outputs    : PASSED/FAILED
 *******************************************************************************
 */
static int plug_lte_telit_run_at_cmd (int input)
{
    const int maxlen = 128;
    char cmd[maxlen];
    char usb_tty_dev[256] = {0,};
    char usb_tty[64] = {0,};
    int uport;

    printf("\n\n ###NOTE: Typer CTRL-x "
           "to seitch back to host's console\n\n");
    fflush(stdout);
    fflush(stderr);
    msleep(AT_COMMAND_UTIL_DELAY);
    
    /* Get modem current USB port */
    plug_lte_telit_get_current_usb_port(&uport);

    /* Dynamically get the corresponding ttyUSB number in case usb device
     * attaches to different ttyUSB */
    if (plug_lte_telit_get_tty_devname(usb_tty_dev) != PASSED) {
        printf("%s: Can't get ttyUSB number\n", __func__);
        return (FAILED);
    }
    sprintf(usb_tty, "%s/%s", USB_TTY_PATH, usb_tty_dev);

    snprintf(cmd, maxlen-1, "microcom %s", usb_tty);

    system(cmd);

    return (PASSED);
}


/*******************************************************************************
 * Function   : plug_lte_telit_man_gpio_exp_util
 * Description: GPIO Expander Utility for Pluggable LTE
 * Inputs     : none 
 * Outputs    : PASSED/FAILED
 *******************************************************************************
 */
static int plug_lte_telit_man_gpio_exp_util (void)
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
 * Function   : plug_lte_telit_opt_gpio_exp_util
 * Description: GPIO Expander Utility for Pluggable LTE
 * Inputs     : none
 * Outputs    : PASSED/FAILED
 *******************************************************************************
 */
static int plug_lte_telit_opt_gpio_exp_util (void)
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
 * Function   : plug_lte_telit_ext_usb_util
 * Description: External USB Debug Enable/Disable Utility for Pluggable LTE
 * Inputs     : input - Not used
 * Outputs    : PASSED/FAILED
 *******************************************************************************
 */
static int plug_lte_telit_ext_usb_util (int input)
{
    int opt;

    printf("Pluggable LTE External USB Enable/Disable Utility.\n");
    opt = getdec_answer("Enable/Disable Debug USB? (0-Disable, 1-Enable): ",
                        OPT_DISABLE, OPT_DISABLE, OPT_ENABLE);

    if (opt == OPT_ENABLE)  {
        printf("Enable Debug USB.\n");
        plug_lte_telit_usb_deb_enable(OPT_ENABLE);
    } else {
        printf("Disable Debug USB.\n");
        plug_lte_telit_usb_deb_enable(OPT_DISABLE);
    }

    return (PASSED);
}


/*******************************************************************************
 * Function   : plug_lte_telit_show_temp
 * Description: This function display temperature detected by temperature sensor
 * Inputs     : input - Not used
 * Outputs    : PASSED/FAILED
 *******************************************************************************
 */
static int plug_lte_telit_show_temp (int input)
{
    return (plug_ts_show_temp());
}


/*******************************************************************************
 * Function   : plug_lte_telit_ts_util
 * Description: Thermal Sensor Utility for Pluggable LTE
 * Inputs     : input - Not used
 * Outputs    : PASSED/FAILED
 *******************************************************************************
 */
static int plug_lte_telit_ts_util (int input)
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
 * Function   : plug_lte_telit_modem_temp_util
 * Description: Function to return the current temperature of Telit LTE modem
 * Inputs     : input - Not used
 * Outputs    : PASSED/FAILED
 *******************************************************************************
 */
int plug_lte_telit_modem_temp_util (int input)
{
    printf("LTE modem temperature info:\n");
    return (plug_lte_telit_dump_modem_temp());
}


/*******************************************************************************
 * Function   : plug_lte_telit_modem_hd_reset
 * Description: Function to hard reset the LTE modem via GPIO exp.
 *              Only for emergency use(host cannot communicate with modem).
 *              This hard reset may cause damage to the modem.
 * Inputs     : input - Not used
 * Outputs    : PASSED/FAILED
 *******************************************************************************
 */
static int plug_lte_telit_modem_hd_reset (int input)
{
    int rc = FAILED;

    printf("Hard reset LTE modem through GPIO exp. "
           "This utility should only used while host cannot communicate "
           "with modem.\n");

    rc = plug_lte_telit_hard_reset();

    return (rc);
}


/*******************************************************************************
 * Function   : plug_lte_telit_modem_reboot_util
 * Description: Function to reboot LTE modem via AT command 
 * Inputs     : input - Not used
 * Outputs    : PASSED/FAILED
 *******************************************************************************
 */
static int plug_lte_telit_modem_reboot_util (int input)
{
    int rc = FAILED;

    printf("LTE Modem Soft reboot utility.\n");

    rc = plug_lte_telit_soft_reboot(USB3P0);

    return (rc);
}


/*******************************************************************************
 * Function   : plug_lte_telit_simdetect_pin_test
 * Description: Function to test LTE modem SIM_DETECT pin.
 *              This test verifies SIM0_DETECT pin and SIM1_DETECT pin
 * Inputs     : sim_no - SIM slot number(0/1)
 * Outputs    : PASSED/FAILED
 *******************************************************************************
 */
static int plug_lte_telit_simdetect_pin_test (int sim_no)
{
    /* This utility test is to enhance the coverage of pluggable LTE SIM_DETECT
     * pin. It requires users to insert and remove SIM card during the test.
     */
    char test_name[LTE_TESTMSG_BUFSZ];

    memset(test_name, 0, sizeof(test_name));
    sprintf(test_name, "LTE SIM%d_DETECT pin", sim_no);
    testname(test_name);
    prpass(testpass, "SIM%d, ", sim_no);

    /* Test SIM_DETECT pin when SIM is present */
    if (lte_telit_simdet_pin_test(sim_no, SIM_PRESENT, ENABLE)
                                  != PASSED) {
        cterr('f', 0, "Failed, SIM%d is inserted "
              "but SIM_DETECT state is Low.", sim_no);
        return (FAILED);
    }

    /* Test SIM_DETECT pin when SIM is NOT present */
    if (lte_telit_simdet_pin_test(sim_no, SIM_NOT_PRESENT, ENABLE)
                                  != PASSED) {
        cterr('f', 0, "Failed, SIM%d is NOT inserted "
              "but SIM_DETECT state is High.", sim_no);
        return (FAILED);
    }

    return (PASSED);
}


/*******************************************************************************
 * Function   : lte_telit_simdet_pin_test
 * Description: Wrapped function to test LTE modem SIM_DETECT pin.
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
static int lte_telit_simdet_pin_test (int sim_num, boolean exp_sim_stat,
                                      boolean usr_prompt)
{
    char usr_input = 0;
    char usr_act_str[LTE_TESTMSG_BUFSZ];
    dev_lte_telit_object_t plug_lte_telit_obj;
    dev_lte_telit_object_t *plug_lte_telit_obj_p = &plug_lte_telit_obj;
    int rc = FAILED;

    memset(usr_act_str, 0, sizeof(usr_act_str));

    if (plug_lte_telit_dev_create(plug_lte_telit_obj_p) != PASSED) {
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
    rc = plug_lte_telit_obj_p->callin_fvt->modem_simin_pin_present(
                                           (dev_object_t *)&plug_lte_telit_obj,
                                            sim_num);
    if (exp_sim_stat == SIM_NOT_PRESENT) {
        if (rc == PASSED) {
            rc = FAILED;
        } else {
            rc = PASSED;
        }
    }

    plug_lte_telit_obj_p->base.dev_object_fvt->dev_destroy((dev_object_t **)
                                                           &plug_lte_telit_obj);

    if (rc != PASSED) {
        cterr('f', 0, "Unexpected SIM%d_DETECT pin status.", sim_num);
    }
    return (rc);
}


/*******************************************************************************
 * Function   : plug_lte_show_simdetect_pin_status
 * Description: Function to dump the status of LTE modem SIM_DETECT pin.
 * Inputs     : sim_num - SIM number(0/1)
 * Outputs    : PASSED or FAILED
 *******************************************************************************
 */
static int plug_lte_telit_show_simdetect_pin_status (int sim_num)
{
    dev_lte_telit_object_t plug_lte_telit_obj;
    dev_lte_telit_object_t *plug_lte_telit_obj_p = &plug_lte_telit_obj;
    int rc = FAILED;

    if (plug_lte_telit_dev_create(plug_lte_telit_obj_p) != PASSED) {
        cterr('f', 0, "Create Telit Dev Object Fails");
        return (FAILED);
    }

    rc = plug_lte_telit_obj_p->callin_fvt->modem_dump_simin_pin_status(
                                           (dev_object_t *)&plug_lte_telit_obj,
                                            sim_num);

    plug_lte_telit_obj_p->base.dev_object_fvt->dev_destroy((dev_object_t **)
                                                           &plug_lte_telit_obj);
    return (rc);
}


/*******************************************************************************
 * Function    : plug_lte_telit_usb_mode_switch_util
 * Description : Function to switch Pluggable LTE modem USB mode.
 * Inputs      : opt - reserved for future use.
 * Outputs     : PASSED/FAILED
 *******************************************************************************
 */
static int plug_lte_telit_usb_mode_switch_util (int opt)
{
    int mode_opt;
    int rc = FAILED;
    int modem_found = FALSE, modem_uport = -1;
    dev_lte_telit_object_t plug_lte_telit_obj;
    dev_lte_telit_object_t *plug_lte_telit_obj_p = &plug_lte_telit_obj;

    printf("Modem current USB connection info:\n");
    plug_lte_telit_modem_searching(&modem_found, &modem_uport);

    printf("\nModem USB mode: %s \nModem connects to: %s.\n",
           (modem_uport == USB3P0)? SUPER_SPD_STR:HIGH_SPD_STR,
           (modem_uport == DEBUG_USB)? DPORT_STR:HOST_CONN_STR);

    printf("Pluggable LTE modem USB mode switching utility:\n");
    mode_opt = getdec_answer("Switch to which mode? (0-Super Speed(USB3.0), "
                             "1-High Speed(USB2.0)):", SUPER_SPD_USB,
                             SUPER_SPD_USB, HIGH_SPD_USB);

    plug_lte_telit_set_current_usb_port(modem_uport);

    /* Create device object */
    if (plug_lte_telit_dev_create(plug_lte_telit_obj_p) != PASSED) {
        printf("Create Telit Dev Object Fails\n");
        goto __exit;
    }

    /* Switch modem USB mode */
    rc = plug_lte_telit_obj_p->callin_fvt->modem_switch_usb_mode(
                                           (dev_object_t *)&plug_lte_telit_obj,
                                            mode_opt);
    if (rc != PASSED) {
        printf("Failed to switch modem USB mode\n");
        goto __exit;
    }

    plug_lte_telit_obj_p->base.dev_object_fvt->dev_destroy((dev_object_t **)
                                                           &plug_lte_telit_obj);
    plug_lte_telit_set_current_usb_port(USB3P0);

    return (PASSED);

__exit:
    plug_lte_telit_obj_p->base.dev_object_fvt->dev_destroy((dev_object_t **)
                                                           &plug_lte_telit_obj);
    plug_lte_telit_set_current_usb_port(USB3P0);
    return (FAILED);

}


/*******************************************************************************
 * Function    : plug_lte_telit_show_testmode_stat_util
 * Description : Function to show Telit modem testmode status.
 * Inputs      : opt - reserved for future use.
 * Outputs     : PASSED/FAILED
 *******************************************************************************
 */
static int plug_lte_telit_show_testmode_stat_util (int opt)
{
    int rc = FALSE;
    dev_lte_telit_object_t plug_lte_telit_obj;
    dev_lte_telit_object_t *plug_lte_telit_obj_p = &plug_lte_telit_obj;

    /* Create device object */
    if (plug_lte_telit_dev_create(plug_lte_telit_obj_p) != PASSED) {
        printf("Create Telit Dev Object Fails\n");
        return (FAILED);
    }

    /* Show modem testmode status */
    rc = plug_lte_telit_obj_p->callin_fvt->modem_in_operation_mode(
                                           (dev_object_t *)&plug_lte_telit_obj);
    if (rc == TRUE) {
        printf("Modem is in operation mode.\n");
    } else {
        printf("Modem is in test mode.\n");
    }

    plug_lte_telit_obj_p->base.dev_object_fvt->dev_destroy((dev_object_t **)
                                                           &plug_lte_telit_obj);

    return (PASSED);
}


/*******************************************************************************
 * Function   : plug_lte_show_modem_info_util
 * Description: Function to dump the modem info(FW / Carrier / PRI / modem SN)
 * Inputs     : input - not used
 * Outputs    : PASSED or FAILED
 *******************************************************************************
 */
static int plug_lte_telit_show_modem_info_util (int input)
{
    dev_lte_telit_object_t plug_lte_telit_obj;
    dev_lte_telit_object_t *plug_lte_telit_obj_p = &plug_lte_telit_obj;
    int rc = FAILED;

    if (plug_lte_telit_dev_create(plug_lte_telit_obj_p) != PASSED) {
        cterr('f', 0, "Create Telit Dev Object Fails");
        return (FAILED);
    }

    rc = plug_lte_telit_obj_p->callin_fvt->modem_dump_info(
                                           (dev_object_t *)&plug_lte_telit_obj);

    plug_lte_telit_obj_p->base.dev_object_fvt->dev_destroy((dev_object_t **)
                                                           &plug_lte_telit_obj);
    return (rc);
}


/*******************************************************************************
 * Function    : plug_lte_telit_enable_op_mode_util
 * Description : Function to enable Telit modem operation mode.
 * Inputs      : opt - reserved for future use.
 * Outputs     : PASSED/FAILED
 *******************************************************************************
 */
static int plug_lte_telit_enable_op_mode_util (int opt)
{
    int rc = FAILED;
    dev_lte_telit_object_t plug_lte_telit_obj;
    dev_lte_telit_object_t *plug_lte_telit_obj_p = &plug_lte_telit_obj;

    /* Create device object */
    if (plug_lte_telit_dev_create(plug_lte_telit_obj_p) != PASSED) {
        printf("Create Telit Dev Object Fails\n");
        return (FAILED);
    }

    rc = plug_lte_telit_obj_p->callin_fvt->modem_enable_op_mode(
                                           (dev_object_t *)&plug_lte_telit_obj);
    if (rc != PASSED) {
        printf("Failed to enable modem operation mode\n");
    } else {
        printf("\nDone.\n");
    }

    plug_lte_telit_obj_p->base.dev_object_fvt->dev_destroy((dev_object_t **)
                                                           &plug_lte_telit_obj);
    /* Modem will reboot while switching modem test mode status */
    if (rc == PASSED) {
        printf("Wait %d seconds to reset modem...\n", TELIT_LTE_RST_DELAY);
        sleep(TELIT_LTE_RST_DELAY);
    }

    return (rc);
}


/*******************************************************************************
 * Function    : plug_lte_telit_enable_dying_gasp_util
 * Description : Function to enable Telit modem dying gasp feature.
 * Inputs      : None.
 * Outputs     : PASSED/FAILED
 *******************************************************************************
 */
static int plug_lte_telit_enable_dying_gasp_util (void)
{
    int rc = FAILED;
    dev_lte_telit_object_t plug_lte_telit_obj;
    dev_lte_telit_object_t *plug_lte_telit_obj_p = &plug_lte_telit_obj;

    if (plug_lte_telit_dev_create(plug_lte_telit_obj_p) != PASSED) {
        printf("Create Telit Dev Object Fails\n");
        return (FAILED);
    }

    rc = plug_lte_telit_obj_p->callin_fvt->modem_enable_dying_gasp(
                                           (dev_object_t *)&plug_lte_telit_obj);

    plug_lte_telit_obj_p->base.dev_object_fvt->dev_destroy((dev_object_t **)
                                                           &plug_lte_telit_obj);

    if (rc != PASSED) {
        printf("Failed to enable modem dying gasp feature.\n");
        return (FAILED);
    }

    printf("Modem dying gasp feature is enable.\n");

    return (PASSED);
}


/*******************************************************************************
 * Function    : plug_lte_telit_enable_fast_shdn_util
 * Description : Function to enable Telit modem fast shutdown feature.
 * Inputs      : None.
 * Outputs     : PASSED/FAILED
 *******************************************************************************
 */
static int plug_lte_telit_enable_fast_shdn_util (void)
{
    int rc = FAILED;
    dev_lte_telit_object_t plug_lte_telit_obj;
    dev_lte_telit_object_t *plug_lte_telit_obj_p = &plug_lte_telit_obj;

    if (plug_lte_telit_dev_create(plug_lte_telit_obj_p) != PASSED) {
        printf("Create Telit Dev Object Fails\n");
        return (FAILED);
    }

    rc = plug_lte_telit_obj_p->callin_fvt->modem_enable_fast_shutdown(
                                           (dev_object_t *)&plug_lte_telit_obj);

    plug_lte_telit_obj_p->base.dev_object_fvt->dev_destroy((dev_object_t **)
                                                           &plug_lte_telit_obj);

    if (rc != PASSED) {
        printf("Failed to enable modem fast shutdown feature.\n");
        return (FAILED);
    }

    printf("Modem fast shutdown feature is enable.\n");

    return (PASSED);
}


/*******************************************************************************
 * Function    : plug_lte_telit_dump_modem_usb_connection_util
 * Description : Function to dump modem basic info.
 * Inputs      : None.
 * Outputs     : PASSED/FAILED
 *******************************************************************************
 */
static int plug_lte_telit_dump_modem_usb_connection_util (void)
{
    int modem_found = FALSE, modem_uport = -1;

    /* Searching modem */
    plug_lte_telit_modem_searching(&modem_found, &modem_uport);

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
 * Function    : plug_lte_telit_set_modem_power_saving_mode_util
 * Description : Function to configure modem power saving mode.
 * Inputs      : None.
 * Outputs     : PASSED/FAILED
 *******************************************************************************
 */
static int plug_lte_telit_set_modem_power_saving_mode_util (void)
{
    uint opt = 0;
    int set_psav_rc = FAILED;
    dev_lte_telit_object_t plug_lte_telit_obj;
    dev_lte_telit_object_t *plug_lte_telit_obj_p = &plug_lte_telit_obj;

    printf("\n");
    printf("Pluggable LTE Telit Configure Modem Power Saving Mode util:\n");
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

    if (plug_lte_telit_dev_create(plug_lte_telit_obj_p) != PASSED) {
        printf("Create Telit Dev Object Fails\n");
        return (FAILED);
    }
    
    set_psav_rc = plug_lte_telit_obj_p->callin_fvt->modem_pwrsaving_mode_ctrl(
                                                    (dev_object_t *)
                                                    &plug_lte_telit_obj,
                                                    opt);
    if (set_psav_rc != PASSED) {
        printf("Failed to set modem power saving mode.\n");
    }
    
    plug_lte_telit_obj_p->base.dev_object_fvt->dev_destroy((dev_object_t **)
                                                           &plug_lte_telit_obj);
    return (set_psav_rc);
}


/*******************************************************************************
 * Function   : plug_lte_telit_multi_led_ctrl_util
 * Description: LED Utility to control all LEDs on Pluggable LTE module
 * Inputs     : input - Not used
 * Outputs    : PASSED or FAILED
 *******************************************************************************
 */
static int plug_lte_telit_multi_led_ctrl_util (int input)
{
    int ix = 0;
    int opt, color_opt;

    printf("Pluggable LTE Multi LEDs control utility.\n");
    opt = getdec_answer("Light up/off Register? (0-off, 1-on):", OPT_OFF, 
                        OPT_OFF, OPT_ON);
    if (opt == OPT_ON) {    
        color_opt = getdec_answer("Yellow/Green? (0-green, 1-yellow):",
                                  OPT_GREEN, OPT_GREEN, OPT_YELLOW);

        if (color_opt == OPT_GREEN) {
            printf("Turn all pluggable lte green LEDs on.\n");
            /* Pull yellow led registers low */
            for (ix = 0; ix < plug_lte_telit_y_led_sz; ix++) {
                plug_lte_telit_toggle_led(
                               plug_lte_telit_y_led_table[ix].dev_fun,
                               plug_lte_telit_y_led_table[ix].drive_off);
            }
            for (ix = 0; ix < plug_lte_telit_g_led_sz; ix++) {
                plug_lte_telit_toggle_led(
                               plug_lte_telit_g_led_table[ix].dev_fun,
                               plug_lte_telit_g_led_table[ix].drive_on);
            }
            /* Change RSSI to 3G mode to select LED color(green) */
            plug_lte_telit_toggle_led(LED_4G_3G, HIGH);
            for (ix = 0; ix < plug_lte_telit_rssi_led_sz; ix++) {
                plug_lte_telit_toggle_led(
                               plug_lte_telit_rssi_led_table[ix].dev_fun,
                               plug_lte_telit_rssi_led_table[ix].drive_on); 
            }
        } else {
            printf("Turn all pluggable lte yellow LEDs on.\n");
            /* Pull green led registers low */
            for (ix = 0; ix < plug_lte_telit_g_led_sz; ix++) {
                plug_lte_telit_toggle_led(
                               plug_lte_telit_g_led_table[ix].dev_fun,
                               plug_lte_telit_g_led_table[ix].drive_off);
            }
            for (ix = 0; ix < plug_lte_telit_y_led_sz; ix++) {
                plug_lte_telit_toggle_led(
                               plug_lte_telit_y_led_table[ix].dev_fun,
                               plug_lte_telit_y_led_table[ix].drive_on); 
            }
            /* Change RSSI to 4G mode to select LED color(yellow) */
            plug_lte_telit_toggle_led(LED_4G_3G, LOW);
            for (ix = 0; ix < plug_lte_telit_rssi_led_sz; ix++) {
                plug_lte_telit_toggle_led(
                               plug_lte_telit_rssi_led_table[ix].dev_fun,
                               plug_lte_telit_rssi_led_table[ix].drive_on); 
            }  
        }
    } else {
        printf("Turn all pluggable lte LEDs off.\n");
        for (ix = 0; ix < plug_lte_telit_g_led_sz; ix++) {
            plug_lte_telit_toggle_led(
                           plug_lte_telit_g_led_table[ix].dev_fun,
                           plug_lte_telit_g_led_table[ix].drive_off);
        }
        for (ix = 0; ix < plug_lte_telit_y_led_sz; ix++) {
            plug_lte_telit_toggle_led(
                           plug_lte_telit_y_led_table[ix].dev_fun,
                           plug_lte_telit_y_led_table[ix].drive_off);
        }
        for (ix = 0; ix < plug_lte_telit_rssi_led_sz; ix++) {
            plug_lte_telit_toggle_led(
                           plug_lte_telit_rssi_led_table[ix].dev_fun,
                           plug_lte_telit_rssi_led_table[ix].drive_off);
        }
    }
    return (PASSED);
}


/*******************************************************************************
 * Function    : plug_lte_led_ctrl_util
 * Description : Function to show Pluggable LTE LED Control utilities submenu.
 * Inputs      : opt - reserved for future use.
 * Outputs     : PASSED/FAILED
 *******************************************************************************
 */
static int plug_lte_telit_led_ctrl_util (int opt)
{
    build_primary_submenu(plug_lte_telit_led_ctrl_tbl,
                          PLUG_LTE_TELIT_LED_CTRL_TBL_SZ,
                          "Plug LTE LED Control Utilities",
                          &plug_lte_telit_led_ctrl_menup);

    build_secondary_submenu(plug_lte_telit_led_ctrl_tbl,
                            PLUG_LTE_TELIT_LED_CTRL_TBL_SZ,
                            plug_lte_telit_led_ctrl_sec_items);

    menu(plug_lte_telit_led_ctrl_menup, plug_lte_telit_led_ctrl_sec_items, '\0');

    return (PASSED);
}


/*******************************************************************************
 * Function    : plug_lte_telit_enable_led_utils
 * Description : Function to turn Pluggable LTE modem enable LED ON/OFF.
 * Inputs      : opt - reserved for future use.
 * Outputs     : PASSED/FAILED
 *******************************************************************************
 */
int plug_lte_telit_enable_led_utils (int opt)
{
    uint option = 0;
    int ix;

    plug_lte_led_ctrl plug_lte_telit_led_ctrl_reqt[] = {
        {ENABLE_LED_YELLOW, LOW},
        {ENABLE_LED_GREEN, LOW}
    };

    int max_enable_led_ctrl_bit = 
        sizeof(plug_lte_telit_led_ctrl_reqt)/sizeof(plug_lte_led_ctrl);

    printf("\n");
    printf("Pluggable LTE Enable LED utils:\n");
    printf("0. OFF\n");
    printf("1. Yellow\n");
    printf("2. Green\n");
    option = getdec_answer("Select Toggle (0~2): ", 0, 0, 2);

    if (option == 0){
        plug_lte_telit_led_ctrl_reqt[0].val = LOW;
        plug_lte_telit_led_ctrl_reqt[1].val = LOW;
    } else if (option == 1) {
        plug_lte_telit_led_ctrl_reqt[0].val = HIGH;
        plug_lte_telit_led_ctrl_reqt[1].val = LOW;
    } else if (option == 2) {
        plug_lte_telit_led_ctrl_reqt[0].val = LOW;
        plug_lte_telit_led_ctrl_reqt[1].val = HIGH;
    } else {
        printf("No selection toggle\n");
        return (FAILED);
    }

    for (ix = 0; ix < max_enable_led_ctrl_bit; ix++) {
        if (plug_lte_telit_toggle_led(plug_lte_telit_led_ctrl_reqt[ix].dev,
                                      plug_lte_telit_led_ctrl_reqt[ix].val)
                                      == FAILED) {
            printf("%s: Failed to toggle GPIO exp.\n", __func__);
            return (FAILED);
        }    
    }

    return (PASSED);
}


/*******************************************************************************
 * Function    : plug_lte_telit_sim_status_led_utils
 * Description : Function to turn Pluggable LTE modem SIM0/1 Status LED ON/OFF.
 * Inputs      : opt - reserved for future use.
 * Outputs     : PASSED/FAILED
 *******************************************************************************
 */
int plug_lte_telit_sim_status_led_utils (int opt)
{
    uint option = 0;
    int ix, sim_opt = 0;

    plug_lte_led_ctrl plug_lte_telit_led_ctrl_reqt[] = {
        {LED_SIM0_NOT_OK, LOW},
        {LED_SIM0_OK, LOW}
    };

    printf("\n");
    printf("Pluggable LTE SIM Status LED utils: \n");
    sim_opt = getdec_answer("Control which LED? (0-SIM0, 1-SIM1):",
                            0, 0, 1);
    

    if (sim_opt == 0) {
        plug_lte_telit_led_ctrl_reqt[0].dev = LED_SIM0_NOT_OK;
        plug_lte_telit_led_ctrl_reqt[1].dev = LED_SIM0_OK;
    } else {
        plug_lte_telit_led_ctrl_reqt[0].dev = LED_SIM1_NOT_OK;
        plug_lte_telit_led_ctrl_reqt[1].dev = LED_SIM1_OK;
    }

    int max_sim_led_ctrl_bit = 
        sizeof(plug_lte_telit_led_ctrl_reqt)/sizeof(plug_lte_led_ctrl);

    printf("0. OFF\n");
    printf("1. Yellow\n");
    printf("2. Green\n");
    option = getdec_answer("Select Toggle (0~2):", 0, 0, 2);

    if (option == 0) {
        plug_lte_telit_led_ctrl_reqt[0].val = LOW;
        plug_lte_telit_led_ctrl_reqt[1].val = LOW;
    } else if (option == 1) {
        plug_lte_telit_led_ctrl_reqt[0].val = HIGH;
        plug_lte_telit_led_ctrl_reqt[1].val = LOW;
    } else if (option == 2) {
        plug_lte_telit_led_ctrl_reqt[0].val = LOW;
        plug_lte_telit_led_ctrl_reqt[1].val = HIGH;
    } else {
        printf("No selection toggle\n");
        return (FAILED);
    }

    for (ix = 0; ix < max_sim_led_ctrl_bit; ix++) {
        if (plug_lte_telit_toggle_led(plug_lte_telit_led_ctrl_reqt[ix].dev,
                                      plug_lte_telit_led_ctrl_reqt[ix].val)
                                      == FAILED) {
            printf("%s: Failed to toggle GPIO exp.\n", __func__);
            return (FAILED);
        }
    }

    return (PASSED);
}


/*******************************************************************************
 * Function    : plug_lte_telit_gps_status_led_utils
 * Description : Function to turn Pluggable LTE modem GPS LED ON/OFF.
 * Inputs      : opt - reserved for future use.
 * Outputs     : PASSED/FAILED
 *******************************************************************************
 */
int plug_lte_telit_gps_status_led_utils (int opt)
{
    uint option = 0;
    int ix;

    plug_lte_led_ctrl plug_lte_telit_led_ctrl_reqt[] = {
        {LED_GPS_NOT_OK, LOW},
        {LED_GPS_OK, LOW}
    };

    int max_gps_led_ctrl_bit =
        sizeof(plug_lte_telit_led_ctrl_reqt)/sizeof(plug_lte_led_ctrl);

    printf("\n");
    printf("Pluggable LTE GPS LED utils: \n");
    printf("0. OFF\n");
    printf("1. Yellow\n");
    printf("2. Green\n");
    option = getdec_answer("Select Toggle (0~2): ", 0, 0, 2);

    if (option == 0) {
        plug_lte_telit_led_ctrl_reqt[0].val = LOW;
        plug_lte_telit_led_ctrl_reqt[1].val = LOW;
    } else if (option == 1 ) {
        plug_lte_telit_led_ctrl_reqt[0].val = HIGH;
        plug_lte_telit_led_ctrl_reqt[1].val = LOW;
    } else if (option == 2 ) {
        plug_lte_telit_led_ctrl_reqt[0].val = LOW;
        plug_lte_telit_led_ctrl_reqt[1].val = HIGH;
    } else {
        printf("No selection toggle\n");
        return (FAILED);
    }

    for (ix = 0; ix < max_gps_led_ctrl_bit; ix++) {
        if (plug_lte_telit_toggle_led(plug_lte_telit_led_ctrl_reqt[ix].dev,
                                      plug_lte_telit_led_ctrl_reqt[ix].val)
                                      == FAILED) {
            printf("%s: Failed to toggle GPIO exp.\n", __func__);
            return (FAILED);
        }
    }

    return (PASSED);
}


/*******************************************************************************
 * Function    : plug_lte_telit_rssi_led_utils
 * Description : Function to turn Pluggable LTE modem RSSI0/1/2/3 LED ON/OFF.
 * Inputs      : opt - reserved for future use.
 * Outputs     : PASSED / FAILED
 *******************************************************************************
 */
int plug_lte_telit_rssi_led_utils (int opt)
{
    uint option = 0;
    int ix;

    plug_lte_led_ctrl plug_lte_telit_led_ctrl_reqt[] = {
        {LED_4G_3G, LOW},
        {LED_RSSI0, LOW},
        {LED_RSSI1, LOW},
        {LED_RSSI2, LOW},
        {LED_RSSI3, LOW}
    };

    int max_rssi_led_ctrl_bit =
        sizeof(plug_lte_telit_led_ctrl_reqt)/sizeof(plug_lte_led_ctrl);

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
    option = getdec_answer("Select Toggle (0 ~ 8): ", 0, 0, 8);
   
    /* Select LED color for 4G LTE mode */
    if (option > 4) {
       plug_lte_telit_led_ctrl_reqt[0].val = HIGH;
    }

    switch (option) {
        case 0:
            break;
        case 1:
        case 5:
            plug_lte_telit_led_ctrl_reqt[1].val = HIGH;
            break;
        case 2:
        case 6:
            plug_lte_telit_led_ctrl_reqt[1].val = HIGH;
            plug_lte_telit_led_ctrl_reqt[2].val = HIGH;
            break;
        case 3:
        case 7:
            plug_lte_telit_led_ctrl_reqt[1].val = HIGH;
            plug_lte_telit_led_ctrl_reqt[2].val = HIGH;
            plug_lte_telit_led_ctrl_reqt[3].val = HIGH;
            break;
        case 4:
        case 8:
            plug_lte_telit_led_ctrl_reqt[1].val = HIGH;
            plug_lte_telit_led_ctrl_reqt[2].val = HIGH;
            plug_lte_telit_led_ctrl_reqt[3].val = HIGH;
            plug_lte_telit_led_ctrl_reqt[4].val = HIGH;
            break;
        default:
            printf("%s: No selection toggle\n", __func__);
            return (FAILED);
    }

    /* Turn off all RSSI LEDs first */
    for (ix = 1; ix < max_rssi_led_ctrl_bit; ix++) {
        if (plug_lte_telit_toggle_led(plug_lte_telit_led_ctrl_reqt[ix].dev, 
                                      LOW) == FAILED) {
            printf("%s: Failed to toggle GPIO exp.\n", __func__);
            return(FAILED);
        }
    }

    for (ix = 0; ix < max_rssi_led_ctrl_bit; ix++) {
        if (plug_lte_telit_toggle_led(plug_lte_telit_led_ctrl_reqt[ix].dev, 
                                      plug_lte_telit_led_ctrl_reqt[ix].val) 
                                      == FAILED) {
            printf("%s: Failed to toggle GPIO exp.\n", __func__);
            return(FAILED);
        }
    }
    
    return (PASSED);
}


/*******************************************************************************
 * Function    : plug_lte_telit_wan_led_util
 * Description : Function to show Pluggable LTE WAN LED Control utilities
 *               submenu.
 *               Due to the AT command to control WAN_LED signal is to set the
 *               LED blinking pattern for each network mode, thus, we set the 
 *               LED blinking pattern for LPM and switch modem to LPM to verify
 *               WAN_LED.
 * Inputs      : opt - reserved for future use.
 * Outputs     : PASSED/FAILED
 *******************************************************************************
 */
static int plug_lte_telit_wan_led_util (int opt)
{
    int ix, rc = FAILED;
    int set_psav_rc = FAILED;
    dev_lte_telit_object_t plug_lte_telit_obj;
    dev_lte_telit_object_t *plug_lte_telit_obj_p = &plug_lte_telit_obj;

    testname("WWAN_LED control utility");

    /* Disable WWAN_LED */
    plug_lte_telit_wwan_led_output_enable_ctrl(HIGH);

    /* Set the output of GPIO expander which connects to SIM0/1 LEDs to low */
    for (ix = 0; ix < plug_lte_telit_sim_led_sz; ix++) {
       plug_lte_telit_toggle_led(plug_lte_telit_sim_led_table[ix].dev_fun,
                                 plug_lte_telit_sim_led_table[ix].drive_off);
    }
    
    /* Switch modem to LPM */
    /* Since we can only set the WAN_LED blinking pattern for each modem 
     * network status, and that need use LPM modem for diag testing. */
    if (plug_lte_telit_dev_create(plug_lte_telit_obj_p) != PASSED) {
        printf("Create Telit Dev Object Fails\n");
        return (FAILED);
    }

    /* W_DISABLE1# pin is used to switch modem between operation mode and power
     * saving mode. And the modem power saving mode can be configured as LPM
     * (Low Power Mode), Power Saving Mode, DG(Dying Gasp) or no event to be
     * performed. In this test, we configured modem power saving mode as LPM */
    /* Check whether the modem power saving modem is configured as LPM */
    prpass(testpass, "Set modem power saving configuration...");
    set_psav_rc = plug_lte_telit_obj_p->callin_fvt->
                                        modem_pwrsaving_mode_ctrl(
                                        (dev_object_t *)
                                        &plug_lte_telit_obj,
                                        MODEM_LPM);
    if (set_psav_rc != PASSED) {
        printf("Failed to set modem power saving mode.\n");
        plug_lte_telit_obj_p->base.dev_object_fvt->dev_destroy(
                                                   (dev_object_t **)
                                                   &plug_lte_telit_obj);
        return (FAILED);
    }

    plug_lte_telit_w_disable1_ctrl(HIGH);

    build_primary_submenu(plug_lte_telit_wan_led_ctrl_tbl,
                          PLUG_LTE_TELIT_WAN_LED_CTRL_TBL_SZ,
                          "Plug LTE WAN LED Control Utilities",
                          &plug_lte_telit_wan_led_ctrl_menup);

    build_secondary_submenu(plug_lte_telit_wan_led_ctrl_tbl,
                            PLUG_LTE_TELIT_WAN_LED_CTRL_TBL_SZ,
                            plug_lte_telit_wan_led_ctrl_sec_items);

    menu(plug_lte_telit_wan_led_ctrl_menup,
         plug_lte_telit_wan_led_ctrl_sec_items, '\0');

    /* Switch modem back to operation mode */
    plug_lte_telit_w_disable1_ctrl(LOW);

    /* Disable WWAN_LED */
    plug_lte_telit_wwan_led_output_enable_ctrl(HIGH);

    /* Restore WWAN_LED blinking pattern to default */
    if (plug_lte_telit_dev_create(plug_lte_telit_obj_p) != PASSED) {
        printf("Create Telit Dev Object Fails\n");
        return (FAILED);
    }

    rc = plug_lte_telit_obj_p->callin_fvt->modem_lpm_wwan_led_ctrl(
                                           (dev_object_t *)&plug_lte_telit_obj,
                                            LED_DEFAULT);
    if (rc != PASSED) {
        printf("Failed to set WAN_LED blinking pattern\n");
    }

    plug_lte_telit_obj_p->base.dev_object_fvt->dev_destroy((dev_object_t **)
                                                           &plug_lte_telit_obj);

    return (rc);
}


/*******************************************************************************
 * Function   : plug_lte_telit_wan_led_on_off_util
 * Description: LED Utility to turn on/off WAN LED on Pluggable LTE module
 *              WAN_LED is a output signal driven by LTE modem to SIM0/1 LED,
 *              which is determined by WWAN_LED_SIM_SEL pin on GPIO expander.
 * Inputs     : input - Not used
 * Outputs    : PASSED or FAILED
 *******************************************************************************
 */
static int plug_lte_telit_wan_led_on_off_util (int input)
{
    int opt, rc = FAILED;
    dev_lte_telit_object_t plug_lte_telit_obj;
    dev_lte_telit_object_t *plug_lte_telit_obj_p = &plug_lte_telit_obj;

    printf("Pluggable LTE WAN LED control utility.\n");
    opt = getdec_answer("Turn on/off WWAN_LED? (0-off, 1-on):", LED_OFF, 
                        LED_OFF, LED_ON);

    if (plug_lte_telit_dev_create(plug_lte_telit_obj_p) != PASSED) {
        printf("Create Telit Dev Object Fails\n");
        return (FAILED);
    }

    rc = plug_lte_telit_obj_p->callin_fvt->modem_lpm_wwan_led_ctrl(
                                           (dev_object_t *)&plug_lte_telit_obj,
                                            opt);
    if (rc != PASSED) {
        printf("Failed to set WAN_LED blinking pattern\n");
    }

    /* Enable WWAN_LED signal */
    printf("Enable WWAN_LED\n");
    plug_lte_telit_wwan_led_output_enable_ctrl(LOW);

    plug_lte_telit_obj_p->base.dev_object_fvt->dev_destroy((dev_object_t **)
                                                           &plug_lte_telit_obj);

    return (rc);
}


/*******************************************************************************
 * Function   : plug_lte_telit_wan_led_sel_util
 * Description: LED Utility to verify the WWAN_LED_SIM_SEL pin
 * Inputs     : input - Not used
 * Outputs    : PASSED or FAILED
 *******************************************************************************
 */
static int plug_lte_telit_wan_led_sel_util (int input)
{
    int rc = FAILED;
    int opt;

    printf("Pluggable LTE WWAN_LED_SEL pin utility.\n");
    opt = getdec_answer("Route WWAN_LED signal to which SIM LED? (0-SIM0, "
                        "1-SIM1):", SIM_0, SIM_0, SIM_1);

    rc = plug_lte_telit_wwan_led_sim_select(opt);

    if (rc != PASSED) {
        printf("Failed to set WWAN_LED_SIM_SEL pin\n");
    }

    return (rc);
}


/*------------------------------------------------------------------
$Log: plug_lte_telit_util.c,v $
Revision 1.6  2019/07/01 10:05:25  sherliu2
Supported mdev for Hyperloop

Revision 1.5  2019/06/26 03:52:55  shjung
1. Added dump modem basic info utility
2. Due to the default configuration of modem power saving modem is changed in B018 FW, modified W_DISABLE pin related functions
3. Added modem power savinf mode control utility
4. Modified pluggable slot init sequence, instead of powering down all pluggable modules before testing, simply power off non-testing pluggable modules

Revision 1.4  2019/06/14 05:46:08  shjung
Fixed CSCvq12342, disable modem dying gasp before toggling Modem_Power_ON pin to avoid modem perform fast shutdown and added enabling dying gasp utility

Revision 1.3  2019/05/20 07:28:06  shjung
1. Replace USB serial driver option.ko/usb_wwan.ko with GobiSerial.ko
2. Changes based on last code review comments.
3. Use poll mechanism to query modem functionality level in W_DISABLE pin test
4. Add 1 second delay after close tty device, which is following Telit's test script process.

Revision 1.2  2019/05/14 08:48:37  sherliu2
Support hyperloop

Revision 1.1.2.15  2019/05/09 07:50:19  sherliu2
1. Added Dump Modem USB Connection Info Utility \n 2. Based on review comments to clean up code

Revision 1.1.2.14  2019/05/02 06:13:35  sherliu2
1. Added enable modem fast shutdown utlity. 2. Added restore modem back to the default testing setup(super speed mode).

Revision 1.1.2.13  2019/04/17 10:09:11  sherliu2
remove mdev related

Revision 1.1.2.12  2019/04/10 11:24:44  shjung
Code clean up

Revision 1.1.2.11  2019/04/08 09:54:25  sherliu2
Modified tty device name to symbolic name generated by mdev

Revision 1.1.2.10  2019/03/12 02:53:52  shjung

1. Added query modem testmode status utility
2. Added enable OP mode utility

Revision 1.1.2.9  2019/02/20 02:04:53  shjung
Corrected the structure of SIM LEDs

Revision 1.1.2.8  2019/02/20 01:34:27  shjung
Integrated multi LEDs control utility into LED control utilities sub-menu

Revision 1.1.2.7  2019/02/15 02:59:49  shjung
Added WWAN_LED control utility

Revision 1.1.2.6  2019/01/18 13:40:28  shjung
Added utility to control all LEDs

Revision 1.1.2.5  2019/01/18 06:15:31  shjung

1. Added W_DISABLE pin test
2. Added modem USB mode switching utility
3. Added delay in modem reboot function based on spec
4. Removed USB mode resotre operation when debug port test failed
5. Code clean up

Revision 1.1.2.4  2019/01/15 10:22:19  shjung
Modified the mechanism to get modem USB device info

Revision 1.1.2.3  2019/01/02 02:09:27  shjung
Restore LTE back to USB3.0 mode while debug port test failed

Revision 1.1.2.2  2018/12/28 06:23:19  shjung
Temporarily remove PCIe test from I/O interface test since PCIe interface is not yet supported by Telit

Revision 1.1.2.1  2018/12/14 00:50:16  shjung
Initial check-in for Hyperloop



$Endlog$
*/
