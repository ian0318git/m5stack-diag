/* $Id: diag_wifi_test.c,v 1.2 2019/01/10 06:36:24 wilbhuan Exp $ 
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/betelgeuse/diag_wifi_test.c,v $
 *------------------------------------------------------------------
 *
 * diag_wifi_test.c
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
#include "diag_wifi_util.h"
#include "diag_uart_lib.h"
#include "diag_wifi_test.h"
#include "linux_api.h"


/*******************************************************************************
 *                            Global Variables
 *******************************************************************************
 */
extern int do_all_menu_items(struct menuinfo *);


/*******************************************************************************
 *                                   Menus                                     
 *******************************************************************************
 */
/* 
 * WiFi tests Menu
 */
submenu_xtable_t wifi_test_tbl[] = {
    {"Wifi Utilities",                  (PFT)diag_wifi_util,             0,   
     0,
     (type_t(*)())0,                    0,         
     (type_t(*)())0,                    0},
    {"WiFi module Bootup Test",         (PFT)diag_wifi_module_bootup_test,     0,
     (MF_CONTINUOUS | MF_DOGRP | MF_DOALL | MF_SHOW_ERRCOUNT),
     (type_t(*)())0,                    0,         
     (type_t(*)())0,                    0},
    {"WiFi module Memory Test",         (PFT)diag_wifi_memory_test,            0,
     (MF_CONTINUOUS | MF_DOGRP | MF_DOALL | MF_SHOW_ERRCOUNT),
     (type_t(*)())0,                    0,         
     (type_t(*)())0,                    0},
    {"WiFi module NOR flash Test",      (PFT)diag_wifi_nor_flash_test,         0,
     (MF_CONTINUOUS | MF_DOGRP | MF_DOALL | MF_SHOW_ERRCOUNT),
     (type_t(*)())0,                    0,         
     (type_t(*)())0,                    0},
    {"WiFi module Temp. sensor Test",   (PFT)diag_wifi_temp_sensor_reg_test,   0,
     (MF_CONTINUOUS | MF_DOGRP | MF_DOALL | MF_SHOW_ERRCOUNT),
     (type_t(*)())0,                    0,         
     (type_t(*)())0,                    0},
    {"WiFi LED Test",                   (PFT)diag_wifi_led_test,               0,
     (MF_CONTINUOUS | MF_DOGRP | MF_DOALL | MF_SHOW_ERRCOUNT),
     (type_t(*)())0,                    0,         
     (type_t(*)())0,                    0},
};

#define WIFI_TEST_TBL_SIZE (sizeof(wifi_test_tbl) / sizeof(submenu_xtable_t))

/* 
 * Primary & secondary submenu items (filled in from xtable)
 */
static mitem_t wifi_test_menu_pri_items[WIFI_TEST_TBL_SIZE + MAX_BASE_ITEMS];
static mitem_t wifi_test_menu_sec_items[WIFI_TEST_TBL_SIZE + MAX_BASE_ITEMS];

menuinfo_t wifi_test_menu = {
    "%s Test Menu",
    0,                          /* mtparam added by init_empty_menu */
    (PFT) show_endnote,         /* notes missing WICs in combos */
    0,                          /* use generic prompt */
    0,                          /* size (bumped by add_menu_item() */
    wifi_test_menu_pri_items,
};

menuinfo_t *wifi_test_menu_p = &wifi_test_menu;


/*******************************************************************************
 *
 * Function   : diag_wifi_test
 * Description: Entry function of WiFi module Diag.
 * Inputs     : exe_all_testmenu - To decide whether to show test menu(TRUE/FALSE)
 *                              or do all related tests directly
 * Outputs    : None
 *
 *******************************************************************************
 */
void diag_wifi_test (boolean exe_all_testmenu)
{
    build_primary_submenu(wifi_test_tbl, WIFI_TEST_TBL_SIZE,
                          "WiFi", &wifi_test_menu_p);
    build_secondary_submenu(wifi_test_tbl, WIFI_TEST_TBL_SIZE,
                            wifi_test_menu_sec_items);

    if (exe_all_testmenu == TRUE) {
        do_all_menu_items(wifi_test_menu_p);
    } else {
        menu(&wifi_test_menu, wifi_test_menu_sec_items, 0);
    }
}

/*******************************************************************************
 *
 * Function   : diag_wifi_module_bootup_test
 * Description: This function to boot wifi module up to Diag Kernel.
 * Inputs     : None
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
int diag_wifi_module_bootup_test (void)
{
    uchar mb_get_loc[FRU_SIZE] = {0};
    uchar mb_get_pid[FRU_SIZE] = {0};
#ifdef ENHANCE_ERROR_MSG_RDY
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
    cterr_add_component("Marvell A7040",
                        "Marvell 88E6176",
                        "Broadcom IPQ4019");

    /* Segment 5: register and memory dump */
    //cterr_add_reg_dump((PFV)bcm63168_get_xdsl_profile);

    /* Segment 6: Platform Environment initialized here*/
    //cterr_add_env_dump((PFV)show_margins_cterr_wrapper);

    /* Segment 7: Top 3 Debugging Steps */
    cterr_add_debug("Switch console to WiFi side and see WiFi in what sate."
                    "Confirm Host and WiFi interface by ping Host using "
                    "command:\"ping 192.168.1.100\". "
                    "If ping fails, first confirm Host and WiFi in"
                    " same IP domain. "
                    "Then check the interface between Host switch"
                    "(Marvell 88E6176) and WiFi module. "
                    "If there is no problem on these interfaces, replace one "
                    "WiFi module and redo the test.");
#endif
    int  ret_val = FAILED;
    int  ctr = 0;
    char *tname = "WLAN boot up";
    char tty_dev[32];
    size_t size = 0;

    if (file_exist(WIFI_KERNEL_FILE, &size) == 0) {
    	cterr('f', 0, "WiFi kernel file doesn't exist!!");
        return (FAILED);
    }

    testname(tname);

    sprintf(tty_dev, "/dev/ttyS2");

    /* 1. Reset WLAN module */
    if (reset_wifi() != PASSED) {
    	cterr('f', 0, "Failed to reset WiFi module");
        return (FAILED);
    }

    /* 2. TFTPboot WLAN module */
    /* 2-1: Stop WiFi module boot up process at u-boot */
    for (ctr = 0; ctr < POLLING_WIFI_UBOOT_TIME; ctr++) {
        plat_tx_uart(tty_dev, PLAT_UART_SEND_ESC_KEY);
        plat_tx_uart(tty_dev, PLAT_WIFI_CR_STRING);

        if (plat_rx_polling_uart(tty_dev,
                                PLAT_WIFI_UBOOT_PROMPT_STRING,
                                1000) == PASSED) {
            prpass(testpass, "Got WiFi u-boot prompt ");
            ret_val = PASSED;
            break;
        }
    }

    if (ret_val != PASSED) {
        cterr('f', 0, "Failed to stop WiFi auto-boot");
        return (FAILED);
    }

    /* 2-2: Config for TFTP download in u-boot */
    if (wifi_enable_platform_wifi_eth()) {
    	cterr('f', 0,"Failed to enable WiFi ethernet\n");
    	return (FAILED);
    }

    /* 2-2-1 Config Netmask */
    if (wifi_set_uboot_tftp(tty_dev, WIFI_UBOOT_NETMASK,
                            PLAT_WIFI_NETMASK) != PASSED) {
        cterr('f', 0,"Failed to set WiFi u-boot %s ", WIFI_UBOOT_NETMASK);
    	return (FAILED);
    }

    /* 2-2-2 Config IP addr. */
    if (wifi_set_uboot_tftp(tty_dev, WIFI_UBOOT_IPADDR,
                            PLAT_WIFI_IPADDR) != PASSED) {
        cterr('f', 0,"Failed to set WiFi u-boot %s ", WIFI_UBOOT_IPADDR);
    	return (FAILED);
    }

    /* 2-2-3 Config Gateway IP addr. */
    if (wifi_set_uboot_tftp(tty_dev, WIFI_UBOOT_GATEWAYIP,
                            PLAT_WIFI_GATEWAYIP) != PASSED) {
        cterr('f', 0,"Failed to set WiFi u-boot %s ", WIFI_UBOOT_GATEWAYIP);
    	return (FAILED);
    }

    /* 2-2-4 Config Server IP addr. */
    if (wifi_set_uboot_tftp(tty_dev, WIFI_UBOOT_SERVERIP,
                            PLAT_WIFI_SERVERIP) != PASSED) {
        cterr('f', 0,"Failed to set WiFi u-boot %s ", WIFI_UBOOT_SERVERIP);
    	return (FAILED);
    }

    /* 2-2-5 Config GPIO */
    if (wifi_confirm_gpio(tty_dev) != PASSED) {
        cterr('f', 0,"Failed to confirm WiFi GPIOs ");
    	return (FAILED);
    }

    /* 2-2-6 Config Server IP addr. */
    if (wifi_set_uboot_tftp(tty_dev, WIFI_UBOOT_BOOTFILE,
                            PLAT_WIFI_BOOTFILE) != PASSED) {
        cterr('f', 0,"Failed to set WiFi u-boot %s ", WIFI_UBOOT_BOOTFILE);
    	return (FAILED);
    }

    /* 2-3: Ping TFTP server IP */
    ret_val = FAILED;
    for (ctr = 0; ctr < POLLING_HOST_WIFI_CONNECT_TIME; ctr++) {
        plat_tx_uart(tty_dev, PLAT_WIFI_PING_SERVER);

        if (plat_rx_polling_uart(tty_dev,
                                PLAT_WIFI_TFTP_SERVER_ALIVE,
                                1000) == PASSED) {
            prpass(testpass, "TFTP server is alive ");
            ret_val = PASSED;
            break;
        }
        msleep(WAIT_WIFI_ACCESS_TIME);   /* 300ms */
    }

    if (ret_val != PASSED) {
        cterr('f', 0, "Failed to ping Host side");
        prcomplete(testpass, errcount, (char *)0);
        return (FAILED);
    }

    /* 2-4: TFTPboot Diag kernel */
    plat_tx_uart(tty_dev, PLAT_WIFI_TFTPBOOT_KERNEL);

    /* 3: Check if WLAN is ready
     *    By polling for WiFi Linux Kernel prompt.
     */
    ret_val = FAILED;
    printf("\nBooting WLAN Linux Kernel");
    for (ctr = 0; ctr < POLLING_WIFI_KERNEL_PROMPT_TIME; ctr++) {
        printf(".");
        fflush(stdout);
        plat_tx_uart(tty_dev, PLAT_WIFI_CR_STRING);
        if (plat_rx_polling_uart(tty_dev,
                                PLAT_WIFI_LINUX_PROMPT_STRING,
                                1000) == PASSED) {
            printf("\n");
            prpass(testpass, "Got WiFi Linux Kernel prompt ");
            ret_val = PASSED;
            break;
        }
        msleep(WAIT_WIFI_LINUX_PROMPT);   /* 1 sec */
    }

    if (ret_val != PASSED) {
        cterr('f', 0, "Failed to get WLAN Linux prompt.");
        return (FAILED);
    }
    
    return (PASSED);
}

/*******************************************************************************
 *
 * Function   : diag_wifi_memory_test
 * Description: Function to execute WiFi module memory test from Host by NC.
 * Inputs     : None
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
int diag_wifi_memory_test (void)
{
    uchar mb_get_loc[FRU_SIZE] = {0};
    uchar mb_get_pid[FRU_SIZE] = {0};
#ifdef ENHANCE_ERROR_MSG_RDY
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
    cterr_add_component("Marvell A7040",
                        "Marvell 88E6176",
                        "Broadcom IPQ4019");

    /* Segment 5: register and memory dump */
    //cterr_add_reg_dump((PFV)bcm63168_get_xdsl_profile);

    /* Segment 6: Platform Environment initialized here*/
    //cterr_add_env_dump((PFV)show_margins_cterr_wrapper);

    /* Segment 7: Top 3 Debugging Steps */
    cterr_add_debug("Switch console to WiFi side and see WiFi in what sate."
                    "Confirm Host and WiFi interface by ping Host using "
                    "command:\"ping 192.168.1.100\". "
                    "If ping fails, first confirm Host and WiFi in"
                    " same IP domain. "
                    "Then check the interface between Host switch"
                    "(Marvell 88E6176) and WiFi module. "
                    "If there is no problem on these interfaces, replace one "
                    "WiFi module and redo the test.");
#endif
    char *tname = "WiFi Memory";

    testname(tname);
    prpass(testpass, "%s, ", tname);

    if (plat_wifi_nc_dispatch_comm(WIFI_DIAG_MEM_TEST_NC) != PASSED) {
        cterr('f', 0, "WiFi memory test failed");
        return (FAILED);
    }
    return (PASSED);
}

/*******************************************************************************
 *
 * Function   : diag_wifi_nor_flash_test
 * Description: Function to execute WiFi module NOR flash test from Host by NC.
 * Inputs     : None
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
int diag_wifi_nor_flash_test (void)
{ 
    uchar mb_get_loc[FRU_SIZE] = {0};
    uchar mb_get_pid[FRU_SIZE] = {0};
#ifdef ENHANCE_ERROR_MSG_RDY
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
    cterr_add_component("Marvell A7040",
                        "Marvell 88E6176",
                        "Broadcom IPQ4019");

    /* Segment 5: register and memory dump */
    //cterr_add_reg_dump((PFV)bcm63168_get_xdsl_profile);

    /* Segment 6: Platform Environment initialized here*/
    //cterr_add_env_dump((PFV)show_margins_cterr_wrapper);

    /* Segment 7: Top 3 Debugging Steps */
    cterr_add_debug("Switch console to WiFi side and see WiFi in what sate."
                    "Confirm Host and WiFi interface by ping Host using "
                    "command:\"ping 192.168.1.100\". "
                    "If ping fails, first confirm Host and WiFi in"
                    " same IP domain. "
                    "Then check the interface between Host switch"
                    "(Marvell 88E6176) and WiFi module. "
                    "If there is no problem on these interfaces, replace one "
                    "WiFi module and redo the test.");
#endif
    char *tname = "WiFi NOR Flash";

    testname(tname);
    prpass(testpass, "%s, ", tname);

    if (plat_wifi_nc_dispatch_comm(WIFI_DIAG_NOR_TEST_NC) != PASSED) {
        cterr('f', 0, "WiFi NOR flash test failed");
        return (FAILED);
    }

    return (PASSED);
}

/*******************************************************************************
 *
 * Function    : diag_wifi_temp_sensor_reg_test
 * Description : Function to execute WiFi module Temp. sensor register test.
 * Inputs      : None
 * Outputs     : PASSED/FAILED
 *
 *******************************************************************************
 */
int diag_wifi_temp_sensor_reg_test (void)
{
    dev_lm75b_object_t ts_data;
    dev_lm75b_object_t *ts_obj = &ts_data;

    testname("WiFi Temperature Sensor");

    wifi_ts_dev_create(ts_obj);

    printf("WiFi Temperature Sensor test\n");

    if (ts_obj == NULL) {
        cterr('f', 0, "%s: Null Object\n", __func__);
        return (FAILED);
    }

    if (ts_obj->callin_fvt->register_test((dev_object_t *)ts_obj) != PASSED) {
        cterr('f', 0, "%s: Failed to do registers R/W ", __func__);
        return (FAILED);
    }

    ts_obj->base.dev_object_fvt->dev_destroy((dev_object_t **)&ts_obj);

    prcomplete(testpass, errcount, (char *)0);
    return (PASSED);
}

/*******************************************************************************
 *
 * Function   : diag_wifi_led_test
 * Description: Function to test WiFi LEDs.
 * Inputs     : None
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
int diag_wifi_led_test (void)
{
    uchar mb_get_loc[FRU_SIZE] = {0};
    uchar mb_get_pid[FRU_SIZE] = {0};
#ifdef ENHANCE_ERROR_MSG_RDY
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
    cterr_add_component("Marvell A7040",
                        "Marvell 88E6176",
                        "Broadcom IPQ4019");

    /* Segment 5: register and memory dump */
    //cterr_add_reg_dump((PFV)bcm63168_get_xdsl_profile);

    /* Segment 6: Platform Environment initialized here*/
    //cterr_add_env_dump((PFV)show_margins_cterr_wrapper);

    /* Segment 7: Top 3 Debugging Steps */
    cterr_add_debug("Confirm WiFi tri-color LED is installed well. "
                    "Switch console to WiFi side and see WiFi in what sate."
                    "Manually turn WiFi LED to RED by using "
                    "command:\"gpio_test --led --red\". "
                    "If still can't see WiFi LED be turned on to RED, "
                    "then replace another WiFi module and redo the test.");
#endif

    char *tname = "WiFi LED";
    char tty_dev[32];
    int  test_time = 3;
    testname(tname);

    sprintf(tty_dev, "/dev/ttyS2");

    printf("This Test needs Tester to confirm if LED lights correctly !!\n");
    printf("Will Turn ON LED in RED, GREEN, and AMBER sequentially,\n");
    printf("and each color will light %d seconds.\n", test_time);

    prpass(testpass, "Turn ON LED in RED ");
    plat_tx_uart(tty_dev, PLAT_WIFI_TURN_LED_ON_RED);
    sleep(test_time);
    plat_tx_uart(tty_dev, PLAT_WIFI_TURN_ALL_LEDS_OFF);

    prpass(testpass, "Turn ON LED in GREEN ");
    plat_tx_uart(tty_dev, PLAT_WIFI_TURN_LED_ON_GREEN);
    sleep(test_time);
    plat_tx_uart(tty_dev, PLAT_WIFI_TURN_ALL_LEDS_OFF);

    prpass(testpass, "Turn ON LED in Amber ");
    printf("Turn ON LED in Amber.\n");
    plat_tx_uart(tty_dev, PLAT_WIFI_TURN_LED_ON_AMBER);
    sleep(test_time);
    plat_tx_uart(tty_dev, PLAT_WIFI_TURN_ALL_LEDS_OFF);

    printf("WLAN LED test is Done.\n");

    prcomplete(testpass, errcount, (char *)0);
    return (PASSED);
}

/*-------------------------------------------------
 * $Log: diag_wifi_test.c,v $
 * Revision 1.2  2019/01/10 06:36:24  wilbhuan
 * The beginning of Betelgeuse application code.
 *
 *-------------------------------------------------
 */
