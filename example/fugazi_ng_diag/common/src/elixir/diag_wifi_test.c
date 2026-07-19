/* $Id: diag_wifi_test.c,v 1.2 2021/09/24 01:21:07 harrchan Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/elixir/diag_wifi_test.c,v $
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
#include "dev_98dxc25x.h"
#include "diag_esw_lib.h"


/*******************************************************************************
 *                            Global Variables
 *******************************************************************************
 */
extern int do_all_menu_items(struct menuinfo *);
boolean wifi_booted = FALSE;


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
int diag_wifi_test (boolean exe_all_testmenu)
{

    dev_98dxc25x_object_t *esw_98dxc25x_obj_p = NULL;
    dev_object_t *dev;


    build_primary_submenu(wifi_test_tbl, WIFI_TEST_TBL_SIZE,
                          "WiFi", &wifi_test_menu_p);
    build_secondary_submenu(wifi_test_tbl, WIFI_TEST_TBL_SIZE,
                            wifi_test_menu_sec_items);

    /* Create 98dxc25x device driver */
    esw_98dxc25x_obj_p = (dev_98dxc25x_object_t *)diag_get_esw_98dxc25x_obj(); 
    if (esw_98dxc25x_obj_p == NULL) {
        cterr('f', 0, "%s: Creating object failed", __func__);
        return (FAILED);
    }
    dev = (dev_object_t *)esw_98dxc25x_obj_p;

    /* Call 98dxc25x config port pve function 
     * AC5 CPU port is 26
     * AC5 Wifi port is 24*/
    if (esw_98dxc25x_obj_p->callin_fvt->esw_config_port_pve(dev, esw_98dxc25x_obj_p->cpss_dev,
                                                            GE_XCAT5_PORT, XCAT5_TO_WIFI_PORT) != PASSED) {
        cterr('f',0,"Failed to configure PVE for port %d", XCAT5_TO_WIFI_PORT);
	    return (FAILED);
    }

    if (exe_all_testmenu == TRUE) {
        do_all_menu_items(wifi_test_menu_p);
    } else {
        menu(&wifi_test_menu, wifi_test_menu_sec_items, 0);
    }

    if (esw_98dxc25x_obj_p->callin_fvt->esw_unconfig_port_pve(dev, esw_98dxc25x_obj_p->cpss_dev,
                                                              GE_XCAT5_PORT, XCAT5_TO_WIFI_PORT) != PASSED) {
        cterr('f',0,"Failed to unconfigure PVE for port %d", XCAT5_TO_WIFI_PORT);
	    return (FAILED);
    }

    return (PASSED);
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

    snprintf(tty_dev, sizeof(tty_dev), "/dev/ttyS2");

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
                                 WAIT_WIFI_PROMPT) == PASSED) {
            prpass(testpass, "Got WiFi u-boot prompt ");
            ret_val = PASSED;
            break;
        }
    }

    if (ret_val != PASSED) {
        cterr('f', 0, "Failed to stop WiFi auto-boot");
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

    /* 2-3: Ping TFTP server IP */
    printf("\nPing test:\n");
    ret_val = FAILED;
    plat_tx_uart(tty_dev, PLAT_WIFI_PING_SERVER);
    plat_tx_uart(tty_dev, PLAT_WIFI_CR_STRING);

    for (ctr = 0; ctr < POLLING_HOST_WIFI_CONNECT_TIME; ctr++) {

        if (plat_rx_polling_uart(tty_dev,
                                 PLAT_WIFI_TFTP_SERVER_ALIVE,
                                 WAIT_WIFI_PROMPT) == PASSED) {
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

    /* 2-4: TFTP download */
    printf("\nStart tftp download:\n");

    plat_tx_uart(tty_dev, PLAT_WIFI_TFTPDIR);
    plat_tx_uart(tty_dev, PLAT_WIFI_CR_STRING);
    plat_tx_uart(tty_dev, PLAT_WIFI_TFTP_DOWNLOAD);
    plat_tx_uart(tty_dev, PLAT_WIFI_CR_STRING);


    /* 3: Check if WLAN is ready
     *    By polling for WiFi ready pin.
     */

    ret_val = FAILED;
    printf("\nBooting WLAN Linux Kernel:\n");
    for (ctr = 0; ctr < POLLING_WIFI_READY_PIN_TIME; ctr++) {
        printf(".");
        fflush(stdout);
        if (wifi_ready_bit_check() == PASSED) {
            prpass(testpass, "WiFi ready pin is high, wifi module is ready\n ");
            ret_val = PASSED;
            break;
        }
        msleep(WAIT_WIFI_LINUX_PROMPT);  /* 1 sec */
    }

    if (ret_val != PASSED) {
        cterr('f', 0, "Wifi ready pin is low, wifi module is not ready. ");
        return (FAILED);
    }

    wifi_booted = TRUE;

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

    snprintf(tty_dev, sizeof(tty_dev), "/dev/ttyS2");

    printf("This Test needs Tester to confirm if LED lights correctly !!\n");
    printf("Will Turn ON LED in RED, GREEN, and BLUE sequentially,\n");
    printf("and each color will light %d seconds.\n", test_time);

    prpass(testpass, "Turn ON LED in RED ");
    plat_tx_uart(tty_dev, PLAT_WIFI_TURN_LED_ON_RED);
    sleep(test_time);
    plat_tx_uart(tty_dev, PLAT_WIFI_TURN_ALL_LEDS_OFF);

    prpass(testpass, "Turn ON LED in GREEN ");
    plat_tx_uart(tty_dev, PLAT_WIFI_TURN_LED_ON_GREEN);
    sleep(test_time);
    plat_tx_uart(tty_dev, PLAT_WIFI_TURN_ALL_LEDS_OFF);

    prpass(testpass, "Turn ON LED in BLUE ");
    plat_tx_uart(tty_dev, PLAT_WIFI_TURN_LED_ON_BLUE);
    sleep(test_time);
    plat_tx_uart(tty_dev, PLAT_WIFI_TURN_ALL_LEDS_OFF);

    printf("WLAN LED test is Done.\n");

    prcomplete(testpass, errcount, (char *)0);
    return (PASSED);
}

/*-------------------------------------------------
 * $Log: diag_wifi_test.c,v $
 * Revision 1.2  2021/09/24 01:21:07  harrchan
 * Collapse Elixir-branch to Main Trunk.
 *
 * Revision 1.1.2.15  2021/05/05 06:53:13  illiu
 * 1. Modify WiFi module Bootup Test process
 * 2. Remove redundant function: wifi_enable_platform_wifi_eth() and wifi_confirm_gpio()
 *
 * Revision 1.1.2.14  2021/04/23 02:44:52  illiu
 * 1. Replace sprintf with snprintf
 * 2. Use variable cpss_dev which is a member of 98dxc25x object, instead of using local variable
 * 3. Modify LED color prompt message
 *
 * Revision 1.1.2.13  2021/04/12 08:45:03  illiu
 * Replace object-create method as object-get method (Device driver object)
 *
 * Revision 1.1.2.12  2021/03/03 01:33:09  illiu
 * Modify wifi bootup kernel from foxconn u-boot version to cisco u-boot version
 *
 * Revision 1.1.2.11  2020/12/08 06:46:55  illiu
 * Add object release in function diag_wifi_test()
 *
 * Revision 1.1.2.10  2020/12/03 01:37:46  illiu
 * Modify the procedure of wifi tftp boot test
 *
 * Revision 1.1.2.9  2020/11/19 09:36:09  illiu
 * Fix wifi tftp boot test
 *
 * Revision 1.1.2.8  2020/11/17 13:14:48  illiu
 * Fix wifi tftp boot test item
 *
 * Revision 1.1.2.7  2020/11/17 10:33:27  illiu
 * Fix wifi tftp boot test item
 *
 * Revision 1.1.2.6  2020/11/13 11:55:10  illiu
 * Add Check wifi ready pin process in wifi bootup test item
 *
 * Revision 1.1.2.5  2020/11/12 06:37:05  illiu
 * 1. Add WiFi module Bootup Test item
 * 2. Add WiFi module Memory Test item
 * 3. Add WiFi module NOR flash Test item
 * 4. Fix WiFi LED control Util item
 *
 * Revision 1.1.2.4  2020/11/06 06:29:49  harrchan
 * Add port pve configuration before testing wifi
 *
 * Revision 1.1.2.3  2020/09/22 05:52:02  harrchan
 * Modify nc commnad string for elixir wifi test
 *
 * Revision 1.1.2.2  2020/09/14 05:49:44  harrchan
 * Remove DSL and GSHDSL relevant part
 *
 * Revision 1.1.2.1  2020/09/09 09:08:08  illiu
 * First version which has been ported with Dreamliner and Marvel CPSS
 *
 * Revision 1.2  2019/01/10 06:36:24  wilbhuan
 * The beginning of Betelgeuse application code.
 *
 *-------------------------------------------------
 */
