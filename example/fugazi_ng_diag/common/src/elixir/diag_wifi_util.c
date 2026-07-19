/* $Id: diag_wifi_util.c,v 1.2 2021/09/24 01:21:07 harrchan Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/elixir/diag_wifi_util.c,v $
 *------------------------------------------------------------------
 *
 * diag_wifi_util.c
 *
 * Copyright (c) 2018 - 2019 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------------------
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>
#include "types.h"
#include "error.h"
#include "errno.h"
#include "nvmonvars.h"
#include "common.h"
#include "common_utils.h"
#include "proto.h"
#include "diag_moka_fpga_lib.h"
#include "diag_enhance_err_msg_lib.h"
#include "platform_cookie.h"
#include "menu.h"
#include "dev_nxp_lm75b.h"
#include "diag_wifi_lib.h"
#include "diag_uart_lib.h"
#include "diag_moka_fpga_util.h"


/*******************************************************************************
 *                          Function Declaration
 *******************************************************************************
 */
int wifi_console_redirect(void);
int wifi_ready_bit_check(void);
int wifi_temp_sensor_show_reg(void);
int wifi_temp_sensor_alter_reg(void);
int wifi_temp_sensor_dump_reg(void);
int wifi_temp_sensor_show_temp(void);
int wifi_led_control_util(void);
int reset_wifi(void);


/*******************************************************************************
 *                            Global Variables
 *******************************************************************************
 */
static dev_lm75b_object_t wifi_ts_obj;
static dev_lm75b_object_t *wifi_ts_obj_p = &wifi_ts_obj;


/*******************************************************************************
 *                                   Menus                                     
 *******************************************************************************
 */
/* WiFi Utilities Menu */
static submenu_xtable_t wifi_utils_tbl[] = {
    {"WiFi Console redirect Util",          (PFT)wifi_console_redirect,       0,
     0,
     (type_t(*)())0,                        0,
     (type_t(*)())0,                        0},
    {"FPGA register Read Util",             (PFT)fpga_reg_rd_util,            0,
     0,
     (type_t(*)())0,                        0,
     (type_t(*)())0,                        0},
    {"FPGA register Write Util",            (PFT)fpga_reg_wr_util,            0,
     0,
     (type_t(*)())0,                        0,
     (type_t(*)())0,                        0},
    {"WiFi Ready bit check Util",           (PFT)wifi_ready_bit_check,        0,
     MF_CONTINUOUS,
     (type_t(*)())0,                        0,
     (type_t(*)())0,                        0},
    {"WiFi Temp. sensor reg. read Util",    (PFT)wifi_temp_sensor_show_reg,   0,
     MF_CONTINUOUS,
     (type_t(*)())0,                        0,
     (type_t(*)())0,                        0},
    {"WiFi Temp. sensor reg. write Util",   (PFT)wifi_temp_sensor_alter_reg,  0,
     MF_CONTINUOUS,
     (type_t(*)())0,                        0,
     (type_t(*)())0,                        0},
    {"WiFi Temp. sensor reg. dump Util",    (PFT)wifi_temp_sensor_dump_reg,   0,
     MF_CONTINUOUS,
     (type_t(*)())0,                        0,
     (type_t(*)())0,                        0},
    {"WiFi Temp. sensor show temp. Util",   (PFT)wifi_temp_sensor_show_temp,  0,
     MF_CONTINUOUS,
     (type_t(*)())0,                        0,
     (type_t(*)())0,                        0},
    {"WiFi LED control Util",               (PFT)wifi_led_control_util,       0,
     0,
     (type_t(*)())0,                        0,
     (type_t(*)())0,                        0},
    {"Reset WiFi module Util",              (PFT)reset_wifi,                  0,
     0,
     (type_t(*)())0,                        0,
     (type_t(*)())0,                        0},
};

#define WIFI_UTILS_TBL_SIZE (sizeof(wifi_utils_tbl) / sizeof(submenu_xtable_t))

static mitem_t wifi_utils_menu_pri_items[WIFI_UTILS_TBL_SIZE + MAX_BASE_ITEMS];
static mitem_t wifi_utils_menu_sec_items[WIFI_UTILS_TBL_SIZE + MAX_BASE_ITEMS];

menuinfo_t wifi_utils_menu = {
    "%s Utilities Menu",
    0,                              /* mtparam added by init_empty_menu */
    (PFT)menu_show_dflags,          /* notes missing WICs in combos */
    0,                              /* use generic prompt */
    0,                              /* size (bumped by add_menu_item() */
    wifi_utils_menu_pri_items,
};

menuinfo_t *wifi_utils_menu_p = &wifi_utils_menu;


/*******************************************************************************
 *
 * Function   : diag_wifi_util
 * Description: Entry function of WiFi Diag utilities.
 * Inputs     : None
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
int diag_wifi_util (void)
{
    build_primary_submenu(wifi_utils_tbl, WIFI_UTILS_TBL_SIZE,
                          "WiFi", &wifi_utils_menu_p);
    build_secondary_submenu(wifi_utils_tbl, WIFI_UTILS_TBL_SIZE,
                            wifi_utils_menu_sec_items);

    menu(&wifi_utils_menu, wifi_utils_menu_sec_items, 0);

    return (PASSED);
}

/*******************************************************************************
 *
 * Function   : wifi_console_redirect
 * Description: Utility to redirect console to WiFi by Picocom.
 * Inputs     : None
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
int wifi_console_redirect (void)
{
    struct uart_parm picocom;
    picocom.tty_dev = PLAT_WIFI_UART_DEV_STR;
    picocom.baudrate = 9600;
    picocom.databit = 8;
    picocom.parity = "1";
    picocom.flow = "n";

    /* Console Switch to WiFi module */
    if (plat_console_switch(&picocom) != PASSED) {
        return (FAILED);
    }
    return (PASSED);
}

/*******************************************************************************
 *
 * Function   : wifi_ready_bit_check
 * Description: To check Wifi module complete the booting sequence.
 * Inputs     : None
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
int wifi_ready_bit_check (void)
{
    uint reg_offset = (uint)FPGA_CARD_PWR_PRE_REG;
    uint reg_val = 0;

    if (fpga_read_32_reg(reg_offset, &reg_val) != PASSED) {
        printf("\n%s(): Failed to read FPGA reg. 0x%04X.\n",
               __func__, reg_offset);
        return (FAILED);
    }

    if (reg_val & WLAN_MODULE_STATUS) {
        printf("\nWiFi module is ready to be used.\n");
    } else {
        return (FAILED);
    }
    return (PASSED);
}

/*******************************************************************************
 *
 * Function    : wifi_temp_sensor_show_reg
 * Description : Function to display WiFi module temperature sensor Register
 * Inputs      : None
 * Outputs     : PASSED / FAILED
 *
 *******************************************************************************
 */
int wifi_temp_sensor_show_reg (void)
{
    /* Sanity check */
    if (wifi_ts_obj_p->callin_fvt == NULL) {
        /* Create WiFi Temperature sensor dev object for utils. */
        if (wifi_ts_dev_create(wifi_ts_obj_p) != PASSED) {
            printf("%s(): Failed to create WiFi Temp. sensor device object.\n",
                   __func__);
            return (FAILED);
        }
    }

    dev_object_t *dev = (dev_object_t *)wifi_ts_obj_p;

    /* Show WiFi Temperature sensor register */
    if (wifi_ts_obj_p->callin_fvt->show_register(dev) != PASSED) {
        printf("%s(): Failed to show WiFi Temp. sensor register.\n", __func__);
        return (FAILED);
    }
    return (PASSED);
}

/*******************************************************************************
 *
 * Function    : wifi_temp_sensor_alter_reg
 * Description : Function to alter WiFi module temperature sensor Register
 * Inputs      : None
 * Outputs     : PASSED / FAILED
 *
 *******************************************************************************
 */
int wifi_temp_sensor_alter_reg (void)
{
    /* Sanity check */
    if (wifi_ts_obj_p->callin_fvt == NULL) {
        /* Create WiFi Temperature sensor dev object for utils. */
        if (wifi_ts_dev_create(wifi_ts_obj_p) != PASSED) {
            printf("%s(): Failed to create WiFi Temp. sensor device object.\n",
                   __func__);
            return (FAILED);
        }
    }

    dev_object_t *dev = (dev_object_t *)wifi_ts_obj_p;

    if (wifi_ts_obj_p->callin_fvt->alter_register(dev) != PASSED) {
        printf("%s(): Failed to alter WiFi Temp. sensor register.\n", __func__);
        return (FAILED);
    }
    return (PASSED);
}

/*******************************************************************************
 *
 * Function    : wifi_temp_sensor_dump_reg
 * Description : Function to dump WiFi module temperature sensor Registers
 * Inputs      : None
 * Outputs     : PASSED / FAILED
 *
 *******************************************************************************
 */
int wifi_temp_sensor_dump_reg (void)
{
    /* Sanity check */
    if (wifi_ts_obj_p->callin_fvt == NULL) {
        /* Create WiFi Temperature sensor dev object for utils. */
        if (wifi_ts_dev_create(wifi_ts_obj_p) != PASSED) {
            printf("%s(): Failed to create WiFi Temp. sensor device object.\n",
                   __func__);
            return (FAILED);
        }
    }

    dev_object_t *dev = (dev_object_t *)wifi_ts_obj_p;

    if (wifi_ts_obj_p->callin_fvt->dump_register(dev) != PASSED) {
        printf("%s(): Failed to dump WiFi Temp. sensor register.\n", __func__);
        return (FAILED);
    }
    return (PASSED);
}

/*******************************************************************************
 *
 * Function    : wifi_temp_sensor_show_temp
 * Description : Function to show WiFi module temperature sensor
 *               detected temperature.
 * Inputs      : None
 * Outputs     : PASSED / FAILED
 *
 *******************************************************************************
 */
int wifi_temp_sensor_show_temp (void)
{
    /* Sanity check */
    if (wifi_ts_obj_p->callin_fvt == NULL) {
        /* Create WiFi Temperature sensor dev object for utils. */
        if (wifi_ts_dev_create(wifi_ts_obj_p) != PASSED) {
            printf("%s(): Failed to create WiFi Temp. sensor device object.\n",
                   __func__);
            return (FAILED);
        }
    }

    dev_object_t *dev = (dev_object_t *)wifi_ts_obj_p;

    if (wifi_ts_obj_p->callin_fvt->show_temp(dev) != PASSED) {
        printf("%s(): Failed to show sensed WiFi Temp.\n", __func__);
        return (FAILED);
    }
    return (PASSED);
}

/*******************************************************************************
 *
 * Function   : wifi_led_control_util
 * Description: Utility to control WiFi LED.
 * Inputs     : None
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
int wifi_led_control_util (void)
{
    int cmd = (int)WIFI_LED_OFF;

    printf("\nWiFi LED control Utility:\n");
    printf("0: Turn WiFi LED Off\n");
    printf("1: Turn WiFi LED Green\n");
    printf("2: Turn WiFi LED Red\n");
    printf("3: Turn WiFi LED Blue\n");
    cmd = (int)gethex_answer("\nEnter your choice(0 ~ 3)",
                             cmd, WIFI_LED_OFF, WIFI_LED_BLUE);

    if (wifi_led_control(cmd) != PASSED) {
        printf("%s(): Failed to control WiFi LED.\n", __func__);
        return (FAILED);
    }
    return (PASSED);
}

/*******************************************************************************
 *
 * Function   : reset_wifi
 * Description: Reset WiFi module by FPGA
 * Inputs     : None
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
int reset_wifi (void)
{
    if (put_wifi_in_reset() != PASSED) {
        printf("%s(): Failed to put WiFi in RESET.\n", __func__);
        return (FAILED);
    }

    msleep(WIFI_RESET_INTERVAL);

    if (release_wifi_from_reset() != PASSED) {
        printf("%s(): Failed to release WiFi from RESET.\n", __func__);
        return (FAILED);
    }
    return (PASSED);
}

/*-------------------------------------------------
 * $Log: diag_wifi_util.c,v $
 * Revision 1.2  2021/09/24 01:21:07  harrchan
 * Collapse Elixir-branch to Main Trunk.
 *
 * Revision 1.1.2.5  2021/05/05 06:17:59  illiu
 * Change Wifi Led color keyword: AMBER to BLUE
 *
 * Revision 1.1.2.4  2021/04/23 02:39:19  illiu
 * Clean up code
 *
 * Revision 1.1.2.3  2020/12/03 01:38:04  illiu
 * Modify the procedure of wifi tftp boot test
 *
 * Revision 1.1.2.2  2020/11/13 11:53:12  illiu
 * Add return FAILED at wifi check bit function
 *
 * Revision 1.1.2.1  2020/09/09 09:08:08  illiu
 * First version which has been ported with Dreamliner and Marvel CPSS
 *
 * Revision 1.2  2019/01/10 06:36:24  wilbhuan
 * The beginning of Betelgeuse application code.
 *
 *-------------------------------------------------
 */
