/* $Id: wifi_tests.c,v 1.6 2019/01/18 05:54:47 yungchen Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/tsn/wifi_tests.c,v $
 *------------------------------------------------------------------
 *
 * wifi_tests.c - Wifi test wraps.
 * by: leslie 
 * May 2016
 *
 * Copyright (c) 2017 - 2019 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
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
#include "menu.h"
#include "nvmonvars.h"
#include "common.h"
#include "common_utils.h"
#include "proto.h"
#include "wifi_tests.h"
#include "uart_api.h"
#include "tsn_comm.h"
#include "platform_fpga.h"
#include "platform_fru.h"
#include "platform_cookie.h"
#include "platform_wifi.h"
#include "wifi_temp_sensor_test.h"

// unit is miniseconds
#define DELAY_SYSCMD 1000

/* M/B test flag defines */
#define MF_1  (MF_CONTINUOUS | MF_DOGRP)
#define MF_2  (MF_1 | MF_DOALL)
#define MF_3  (MF_2 | MF_SHOW_ERRCOUNT)
#define MF_4  (MF_1 | MF_SHOW_ERRCOUNT)
#define ENHANCE_ERROR_MSG_RDY 1

/*
 * Global variables
 */
int parm1 = 0;
int parm2 = 0;
int parm3 = 0;
int parm4 = 0;
boolean wifi_booted = FALSE;

/*
 * Global extern functions
 */
extern int do_all_menu_items(struct menuinfo *);
extern int  build_wifi_snsr_menu(void);

/*
 * Global variables
 */
static int wifi_utils(int);
static int wifi_mem_test(void);
static int wifi_nor_flash_test(void);
static int wifi_led_test(void);
static int wifi_present_detect(void);
static int wifi_rdy_detec(void);
static int wifi_enable_platform_wifi_eth(void);
static int is_wifi_present(void);
static int wlan_bootup_test(void);
static int put_wlan_in_reset(void);
static int wifi_set_uboot_tftp(char *, char *, char *);
static int wifi_confirm_gpio(char *);
static int host_n_wifi_pid_paring_test(char *);


static wifi_gpio_t tsn_wifi_gpio_tbl[] = {
    /* GPIO_Name  GPIO_Number  GPIO_VAL */
    {"SW Reset",  WIFI_GPIO18, 0x4800},
    {"LED Green", WIFI_GPIO39, 0x4A01},
    {"LED Amber", WIFI_GPIO40, 0x4A00},
    {"LED Red",   WIFI_GPIO45, 0x4A00},
    {"READY",     WIFI_GPIO48, 0x4801},
};

#define TSN_WIFI_PID_LENGTH 32

static host_wifi_pid_map star_host_wifi_pid_map_tbl[] = {
    {STAR_WIFI_ITEMP_STR, STAR_C949_PID_STR},
    {STAR_WIFI_CTEMP_STR, STAR_C941_PID_STR},
};

/*******************************************************************************
 *                                   Menus                                     *
 *******************************************************************************
 */
#define MM_1    (MF_CONTINUOUS | MF_DOGRP)
#define MM_2    (MM_1 | MF_DOALL)
#define MM_3    (MM_2 | MF_SHOW_ERRCOUNT)
#define FLAGS   MF_CONTINUOUS

/*=========================================
 * xDSL Main Tests menu items
 *=========================================
 */
static submenu_xtable_t wifi_main_tests_submenu_table[] = {
    {"Wifi NOR Flash Test",      (PFT)wifi_nor_flash_test,   0, MF_3,
     (type_t(*)())0,         0,         (type_t(*)())0,              0},
};

#define WIFI_MAIN_TESTS_SUBMENU_TABLE_SIZE (sizeof(wifi_main_tests_submenu_table) / \
                                            sizeof(submenu_xtable_t))

/*
 * Primary & secondary submenu items (filled in from xtable)
 */
static mitem_t wifi_main_tests_primary_items[WIFI_MAIN_TESTS_SUBMENU_TABLE_SIZE +
                                             MAX_BASE_ITEMS];
//static mitem_t wifi_main_tests_secondary_items[WIFI_MAIN_TESTS_SUBMENU_TABLE_SIZE +
//                                               MAX_BASE_ITEMS];
menuinfo_t wifi_main_subtest_menu = {
    "%s Subtest Menu",
    0,                          /* mtparam added by init_empty_menu */
    (PFT) show_endnote,         /* notes missing WICs in combos */
    0,                          /* use generic prompt */
    0,                          /* size (bumped by add_menu_item() */
    wifi_main_tests_primary_items,
};

menuinfo_t *wifi_main_submenup = &wifi_main_subtest_menu;

/*=========================================
 * Utilities menu items
 *=========================================
 */
static submenu_xtable_t wifi_utils_submenu_table[] = {
    {"Wifi Module Reset",          (PFT)tsn_reset_wifi, 0, MF_3,
     (type_t(*)())0, 0,   (type_t(*)())0,          0},
    {"Wifi Present Detection",     (PFT)wifi_present_detect, 0, MF_3,
      (type_t(*)())0, 0,   (type_t(*)())0,          0},
    {"Wifi Ready Detection",       (PFT)wifi_rdy_detec, 0, MF_3,
      (type_t(*)())0, 0,   (type_t(*)())0,          0},
    {"WLAN Module LED ON in RED",  (PFT)tsn_wifi_led_control, WIFI_LED_RED, 0,
     (type_t(*)())0, 0,   (type_t(*)())0,          0},
    {"WLAN Module LED ON in GREEN",  (PFT)tsn_wifi_led_control, WIFI_LED_GREEN, 0,
     (type_t(*)())0, 0,   (type_t(*)())0,          0},
    {"WLAN Module LED ON in AMBER",  (PFT)tsn_wifi_led_control, WIFI_LED_AMBER, 0,
     (type_t(*)())0, 0,   (type_t(*)())0,          0},
    {"WLAN Module LED OFF",  (PFT)tsn_wifi_led_control, WIFI_LED_OFF, 0,
     (type_t(*)())0, 0,   (type_t(*)())0,          0},
    {"Put WLAN Module in Reset",     (PFT)put_wlan_in_reset, 0, 0,
     (type_t(*)())0, 0,              (type_t(*)())0,         0},
};

#define WIFI_UTILS_SUBMENU_TABLE_SZ (sizeof(wifi_utils_submenu_table) / \
                                     sizeof(submenu_xtable_t))

static mitem_t wifi_utils_primary_items[WIFI_UTILS_SUBMENU_TABLE_SZ +
                                        MAX_BASE_ITEMS];
static mitem_t wifi_utils_secondary_items[WIFI_UTILS_SUBMENU_TABLE_SZ +
                                          MAX_BASE_ITEMS];

char wifiutiltitle[50];

menuinfo_t wifi_util_submenu = {
    wifiutiltitle,
    0,                              /* mtparam added by init_empty_menu */
    (PFT)menu_show_dflags,          /* notes missing WICs in combos */
    0,                              /* use generic prompt */
    0,                              /* size (bumped by add_menu_item() */
    wifi_utils_primary_items,
};

menuinfo_t *wifi_util_submenup = &wifi_util_submenu;

/* 
 * Sub Menu used for "Main menu -> wifi test"
 */
submenu_xtable_t wifi_tests_submenu_table[] = {
    {"Wifi Utilities",             (PFT)wifi_utils,              0,   
     0,
     (type_t(*)())0,               0,         
     (type_t(*)())wifi_utils,      0},
    {"WLAN module Bootup Test", (PFT)wlan_bootup_test,       0,
     (MF_CONTINUOUS | MF_DOGRP | MF_DOALL | MF_SHOW_ERRCOUNT),
     (type_t(*)())0,            0,
     (type_t(*)())0,            0},
    {"WLAN module NOR flash Test", (PFT)wifi_nor_flash_test,    0,
     (MF_CONTINUOUS | MF_DOGRP | MF_DOALL | MF_SHOW_ERRCOUNT),
     (type_t(*)())0,            0,
     (type_t(*)())0,            0},
    {"WLAN module memory Test",    (PFT)wifi_mem_test,    0,
     (MF_CONTINUOUS | MF_DOGRP | MF_DOALL | MF_SHOW_ERRCOUNT),
     (type_t(*)())0,            0,
     (type_t(*)())0,            0},
    {"WLAN module LED Test",    (PFT)wifi_led_test,          0,
     (MF_CONTINUOUS | MF_DOGRP | MF_DOALL | MF_SHOW_ERRCOUNT),
     (type_t(*)())0,            0,
     (type_t(*)())0,            0},
    {"WLAN Temp sensor Test",      (PFT)wifi_temp_sensor_test,   TRUE,
     (MF_CONTINUOUS | MF_DOGRP | MF_DOALL | MF_SHOW_ERRCOUNT),
     (type_t(*)())0,            0,
     (PFT)wifi_temp_sensor_test,   FALSE},
    {"WLAN module Console",     (PFT)tsn_wifi_console_switch, 0,
     0,
     (type_t(*)())0,            0,
     (type_t(*)())0,            0},
};

#define WIFI_TESTS_SUBMENU_TABLE_SIZE (sizeof(wifi_tests_submenu_table) / \
                                       sizeof(submenu_xtable_t))

/* 
 * "Main menu -> motherboard test" primary & secondary submenu items (filled in from xtable)
 */
static mitem_t wifi_tests_primary_items[WIFI_TESTS_SUBMENU_TABLE_SIZE +
                                        MAX_BASE_ITEMS];
static mitem_t wifi_tests_secondary_items[WIFI_TESTS_SUBMENU_TABLE_SIZE +
                                          MAX_BASE_ITEMS];

menuinfo_t wifi_subtest_menu = {
    "%s Subtest Menu",
    0,                          /* mtparam added by init_empty_menu */
    (PFT) show_endnote,         /* notes missing WICs in combos */
    0,                          /* use generic prompt */
    0,                          /* size (bumped by add_menu_item() */
    wifi_tests_primary_items,
};

menuinfo_t *wifi_submenup = &wifi_subtest_menu;

/*-------------------------------------------------------------------
 *
 * Function: wifi_tests()
 *
 * First build the primary & secondary submenus for the motherboard
 * diags based on the _xtable_ mb_tests_submenu_table.  If the given
 * arg is TRUE, execute all the tests in the menu flagged with
 * MF_DOALL, and return the result.  Otherwise, present the menu to the
 * user for interaction.
 *
 */
int wifi_tests(boolean wifi_test_items_executed)
{
    int rc = FAILED;
    char *tname = "Wifi";
    
    testname(tname);

    tsn_module = TSN_WIFI_MODULE;

    build_primary_submenu(wifi_tests_submenu_table,
                          WIFI_TESTS_SUBMENU_TABLE_SIZE, "Wifi",
                          &wifi_submenup);

    build_secondary_submenu(wifi_tests_submenu_table,
                            WIFI_TESTS_SUBMENU_TABLE_SIZE,
                            wifi_tests_secondary_items);

    if (wifi_test_items_executed) {
        do_all_menu_items(&wifi_subtest_menu);
    } else {
        menu(&wifi_subtest_menu, wifi_tests_secondary_items, '\0');
    }

    return (rc);
}

/*******************************************************************************
 *
 *  Function: wifi_utils
 *
 *  Description: Wifi Utitlities menu
 *
 *  Input: None
 *
 *  Returns: PASSED
 *
 *******************************************************************************
 */
static int wifi_utils (int show_menu)
{
    sprintf(wifiutiltitle, "Wifi Utilities Menu");

    build_primary_submenu(wifi_utils_submenu_table,
                          WIFI_UTILS_SUBMENU_TABLE_SZ,
                          wifiutiltitle, &wifi_util_submenup);

    build_secondary_submenu(wifi_utils_submenu_table,
                            WIFI_UTILS_SUBMENU_TABLE_SZ,
                            wifi_utils_secondary_items);

    menu(wifi_util_submenup, wifi_utils_secondary_items, '\0');

    return (PASSED);
}

/*******************************************************************************
 *
 * Function: wifi_print_spining_wheel
 *
 * Description: Display the spining wheel during the waiting time.
 *
 * Input:  Ring cycle
 *
 * Output: none
 *
 *******************************************************************************
 */
static void wifi_print_spining_wheel (int pass)
{
    printf("\b");
    switch (pass%8) {
    case 0:
        printf("|");
        break;
    case 1:
        printf("/");
        break;
    case 2:
        printf("-");
        break;
    case 3:
        printf("\\");
        break;
    case 4:
        printf("|");
        break;
    case 5:
        printf("/");
        break;
    case 6:
        printf("-");
        break;
    case 7:
        printf("\\");
        break;
    default:
        break;
    }
    fflush(stdout);
    printf("\r");
}

/*******************************************************************************
 *
 * Function   : wlan_bootup_test
 * Description: This function let wifi boot up to diag.
 * Inputs     : None
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
static int wlan_bootup_test (void)
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
    int  pass = 0;
    char *tname = "WLAN boot up";
    char tty_dev[32];
    char err_msg[WIFI_ERRMSG_SIZE];

    memset(err_msg, 0, sizeof(err_msg));

    testname(tname);

    sprintf(tty_dev, "/dev/ttyS2");

    /* 1. Reset WLAN module */
    if (tsn_reset_wifi() != PASSED) {
    	cterr('f', 0, "Failed to reset WiFi module");
        return (FAILED);
    }

    /* 2. TFTPboot WLAN module */
    /* 2-1: Stop WiFi module boot up process at u-boot */
    for (ctr = 0; ctr < POLLING_WIFI_UBOOT_TIME; ctr++) {
        tsn_tx_uart(tty_dev, TSN_UART_SEND_ESC_KEY);
        tsn_tx_uart(tty_dev, TSN_WIFI_CR_STRING);

        if (tsn_rx_polling_uart(tty_dev,
                                TSN_WIFI_UBOOT_PROMPT_STRING,
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
                            TSN_WIFI_NETMASK) != PASSED) {
        cterr('f', 0,"Failed to set WiFi u-boot %s ", WIFI_UBOOT_NETMASK);
    	return (FAILED);
    }

    /* 2-2-2 Config IP addr. */
    if (wifi_set_uboot_tftp(tty_dev, WIFI_UBOOT_IPADDR,
                            TSN_WIFI_IPADDR) != PASSED) {
        cterr('f', 0,"Failed to set WiFi u-boot %s ", WIFI_UBOOT_IPADDR);
    	return (FAILED);
    }

    /* 2-2-3 Config Gateway IP addr. */
    if (wifi_set_uboot_tftp(tty_dev, WIFI_UBOOT_GATEWAYIP,
                            TSN_WIFI_GATEWAYIP) != PASSED) {
        cterr('f', 0,"Failed to set WiFi u-boot %s ", WIFI_UBOOT_GATEWAYIP);
    	return (FAILED);
    }

    /* 2-2-4 Config Server IP addr. */
    if (wifi_set_uboot_tftp(tty_dev, WIFI_UBOOT_SERVERIP,
                            TSN_WIFI_SERVERIP) != PASSED) {
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
                            TSN_WIFI_BOOTFILE) != PASSED) {
        cterr('f', 0,"Failed to set WiFi u-boot %s ", WIFI_UBOOT_BOOTFILE);
    	return (FAILED);
    }

    /* 2-3: Ping TFTP server IP */
    ret_val = FAILED;
    for (ctr = 0; ctr < POLLING_HOST_WIFI_CONNECT_TIME; ctr++) {
        tsn_tx_uart(tty_dev, TSN_WIFI_PING_SERVER);

        if (tsn_rx_polling_uart(tty_dev,
                                TSN_WIFI_TFTP_SERVER_ALIVE,
                                1000) == PASSED) {
            prpass(testpass, "TSN TFTP server is alive ");
            ret_val = PASSED;
            break;
        }
        msleep(WAIT_WIFI_ACCESS_TIME);   /* 300ms */
    }

    if (ret_val != PASSED) {
        cterr('f', 0, "Failed to ping Host side(TSN)");
        prcomplete(testpass, errcount, (char *)0);
        return (FAILED);
    }

    /* 2-4: TFTPboot Diag kernel */
    tsn_tx_uart(tty_dev, TSN_WIFI_TFTPBOOT_KERNEL);

    /* 3: Check if WLAN is ready
     *    By polling for WiFi Linux Kernel prompt.
     */
    ret_val = FAILED;
    for (ctr = 0; ctr < POLLING_WIFI_KERNEL_PROMPT_TIME; ctr++) {
        wifi_print_spining_wheel(pass++);
        tsn_tx_uart(tty_dev, TSN_WIFI_CR_STRING);
        if (tsn_rx_polling_uart(tty_dev,
                                TSN_WIFI_LINUX_PROMPT_STRING,
                                1000) == PASSED) {
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

    /* For Star project, there're totally two WiFi modules and 
     * each one maps to different Host.
     *       Star WiFi PIDs                       Star Host PIDs   
     * ==============================         ======================
     * WiFi (iTemp): ISR-AP1101AC-I-x maps to Star C949: C1109-xxxx;
     * WiFi (cTemp): ISR-AP1101AC-x   maps to Star C941: C1101-xxxx.
     * 
     * For TSN project, there's only ONE WiFi module.
     * So Host and WiFi PID paring check is no needed on TSN.
     */
    if ((this_is_star() == TRUE) || (this_is_supernova() == TRUE)) {
        if (host_n_wifi_pid_paring_test(err_msg) != PASSED) {
            cterr('f', 0, "%s", err_msg);
            return (FAILED);
        }
    }
    
    wifi_booted = TRUE;

    return (PASSED);
}

/*******************************************************************************
 *
 * Function   : wifi_mem_test
 * Description: Function to execute TSN WiFi memory test from Host by NC.
 * Inputs     : None
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
static int wifi_mem_test (void)
{
    char *tname = "WiFi Memory";

    testname(tname);
    prpass(testpass, "%s, ", tname);

    if (tsn_wifi_nc_dispatch_comm(WIFI_DIAG_MEM_TEST_NC) != PASSED) {
        cterr('f', 0, "WiFi memory test failed");
        return (FAILED);
    }
    return (PASSED);
}

/*******************************************************************************
 *
 * Function   : wifi_nor_flash_test
 *
 * Description:
 *
 * Inputs     :
 *
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
static int wifi_nor_flash_test (void)
{ 
    char *tname = "WiFi NOR Flash";

    testname(tname);
    prpass(testpass, "%s, ", tname);

    if (tsn_wifi_nc_dispatch_comm(WIFI_DIAG_NOR_TEST_NC) != PASSED) {
        cterr('f', 0, "WiFi NOR flash test failed");
        return (FAILED);
    }

    return (PASSED);
}

/*******************************************************************************
 *
 * Function   : wifi_led_test
 *
 * Description:
 *
 * Inputs     :
 *
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
static int wifi_led_test (void)
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
    tsn_tx_uart(tty_dev, TSN_WIFI_TURN_LED_ON_RED);
    sleep(test_time);
    tsn_tx_uart(tty_dev, TSN_WIFI_TURN_ALL_LEDS_OFF);

    prpass(testpass, "Turn ON LED in GREEN ");
    tsn_tx_uart(tty_dev, TSN_WIFI_TURN_LED_ON_GREEN);
    sleep(test_time);
    tsn_tx_uart(tty_dev, TSN_WIFI_TURN_ALL_LEDS_OFF);

    prpass(testpass, "Turn ON LED in Amber ");
    printf("Turn ON LED in Amber.\n");
    tsn_tx_uart(tty_dev, TSN_WIFI_TURN_LED_ON_AMBER);
    sleep(test_time);
    tsn_tx_uart(tty_dev, TSN_WIFI_TURN_ALL_LEDS_OFF);

    printf("WLAN LED test is Done.\n");

    prcomplete(testpass, errcount, (char *)0);
    return (PASSED);
}

/*******************************************************************************
 *
 * Function   : is_wifi_present
 * Description: Check if WiFi module is detected or not.
 *              There is no presence pin to determine whether the module
 *              is present or not.
 * Inputs     : None
 * Outputs    : TRUE - present, FALSE - not present
 *
 *******************************************************************************
 */
static int is_wifi_present (void)
{
    uint data;

    if (fpga_read_32_reg(FPGA_CARD_PWR_PRE_REG, &data)
        == FAILED) {
        return (FAILED);
    }

    /* 0: WLAN card is present ; 1: WLAN card is not present */
    if (data & WLAN_MODULE_PRESENT) {
        cterr('f', 0, "Wireless LAN module is absent or not responding!!!");
        return(FAILED);
    }
    else {
        printf("\n Wireless LAN module is installed. \n");
    }
    
    return (PASSED);
}
/*******************************************************************************
 *
 * Function   : wifi_present_detect
 *
 * Description: To detect Wifi module is exist or not
 *
 * Inputs     : None
 *
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
static int wifi_present_detect (void)
{
    char *tname = "Wifi Present Detection";

    testname(tname);
    prpass(testpass, "%s, ", tname);

    /* Check if Wifi module is present */
    if(is_wifi_present() == FAILED) {
        return(FAILED);
    }   

    return (PASSED);
}

/*******************************************************************************
 *
 * Function   : wifi_rdy_detec
 *
 * Description: To check Wifi module complete the booting sequence.
 *
 * Inputs     : None
 *
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
static int wifi_rdy_detec (void)
{
    uint data;

    if (fpga_read_32_reg(FPGA_CARD_PWR_PRE_REG, &data)
        == FAILED) {
        printf("\nFailed to read FPGA Primary ready!!! \n");
        return (FAILED);
    }
    if (data & WLAN_MODULE_STATUS) {
        printf("\nWireless LAN module is ready to use. \n");
    }
    else {
        printf("\nWireless LAN module is not ready!!! \n");
        return(FAILED);
    }

    return (PASSED);
}

/*******************************************************************************
 *
 * Function   : wifi_enable_platform_wifi_eth
 *
 * Description: Configures Marvell 98DX3133 Ethernet Switch such that 
 *              the network path between platform and Wifi module 
 *              (through Marvell 98DX3133 Ethernet Switch) is working.
 *
 * Inputs     : None
 *
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
static int wifi_enable_platform_wifi_eth (void)
{
    /* Bring up Switch interface(eth1) with IP */

    return (PASSED);
}

/************************************************************************
 * Function: wifi_reset_init()
 * Description : Do the Wifi reset initialization sequence
 *
 * Input: None
 *
 * Output: PASSED/FAILED
 *************************************************************************
 */
int wifi_reset_init (void)
{
    /* WiFi module initialization reset sequence */
    if (fpga_reset_32_api(FPGA_EXTER_DEV_RST_REG, EXT_WLAN_RESET, TRUE,
                          RESET_20_MILLISECONDS) == FAILED) {
        return (FAILED);
    }

    /* Un-reset the wifi module */
    if (fpga_reset_32_api(FPGA_EXTER_DEV_RST_REG, EXT_WLAN_RESET, FALSE,
                          UNRESET_20_MILLISECONDS) == FAILED) {
        return (FAILED);
    }

    return (PASSED);
}

/*******************************************************************************
 *
 * Function   : put_wlan_in_reset
 * Description: Utility to put WLAN module in Reset. 
 * Inputs     : None
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
static int put_wlan_in_reset (void)
{
    uint reg_addr = (uint)FPGA_EXTER_DEV_RST_REG;
    uint reg_val = 0;

    /* Read FPGA interface reset register. */
    if (fpga_read_32_reg(reg_addr, &reg_val) != PASSED) {
        printf("Failed to read FPGA reg.(0x%04X).\n", reg_addr);
        return (FAILED);
    }

    if ((reg_val & (uint)EXT_WLAN_RESET) != (uint)EXT_WLAN_RESET) {
        reg_val |= (uint)EXT_WLAN_RESET;

        if (fpga_write_32_reg(reg_addr, reg_val) != PASSED) {
            printf("Failed to write FPGA reg.(0x%04X).\n", reg_addr);
            return (FAILED);
        }
    }
    printf("WLAN module in RESET now.\n");

    return (PASSED);
}

/*******************************************************************************
 *
 * Function   : wifi_uart_rx
 * Description: Function to configure TSN WiFi u-boot TFTP parameters.
 * Inputs     : dev
 *              size
 *              *uart_buf
 *              timeout
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
static void *wifi_uart_rx (void *input)
{
    int timeout = 5; /*in secs */
    int size = 0; /* when size = 0, read all bytes from uart controller */

    tsn_uart *uart = (tsn_uart *)input;

    if (tsn_rx_uart(uart->dev, size, (char *)uart->buf, timeout) < 0) {

    }
    pthread_exit(NULL);
}

/*******************************************************************************
 *
 * Function   : wifi_set_uboot_tftp
 * Description: Function to configure TSN WiFi u-boot TFTP parameters.
 * Inputs     : tty_dev
 *              set_type
 *              ip_str
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
static int wifi_set_uboot_tftp (char *tty_dev, char *set_type, char *ip_str)
{
    char      cmd_str[TSN_WIFI_PARMS_LENGTH];
    char      chk_cmd[TSN_WIFI_PARMS_LENGTH];
    int       ctr = 0, is_configed = FALSE;
    pthread_t threads;
    tsn_uart  uart;

    uart.dev = tty_dev;

    memset(cmd_str, 0, sizeof(cmd_str));
    memset(chk_cmd, 0, sizeof(chk_cmd));

    sprintf(cmd_str, "setenv %s %s\n", set_type, ip_str);
    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("cmd_str = %s", cmd_str);
    }

    sprintf(chk_cmd, "printenv %s\n", set_type);
    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("chk_cmd = %s", chk_cmd);
    }

    for (ctr = 0; ctr < TSN_WIFI_MAX_RETRY; ctr++) { 
        memset(uart.buf, 0, sizeof(uart.buf));

        if (tsn_tx_uart(tty_dev, cmd_str) != PASSED) {
            printf("%s: Failed to set %s to %s.\n",
                   __FUNCTION__, set_type, ip_str);
            return (FAILED);
        }
        msleep(WAIT_WIFI_ACCESS_TIME);   /* 300ms */

        if(pthread_create(&threads, NULL, wifi_uart_rx, (void *)&uart)) {
            /* Should never occur */
            printf("%s: pthread_create failed.\n", __FUNCTION__);
            return (FAILED);
        }
        msleep(WAIT_WIFI_ACCESS_TIME);   /* 300ms */

        if (tsn_tx_uart(tty_dev, chk_cmd) != PASSED) {
            printf("%s: Failed to print %s.\n", __FUNCTION__, set_type);
            return (FAILED);
        }

        pthread_join(threads, NULL);

        if (!strlen(uart.buf)) {
            printf("%s: [%d]No data received.\n", __FUNCTION__, ctr);
            break;
        }
        if ((NVRAM)->diagflag & D_VERBOSE) {
            printf("RX = %s", uart.buf);
        }

        /* Confirm set-up by checked RX data */
        if (strstr(uart.buf, ip_str) != NULL) {
            is_configed = TRUE;
            break;
        }
    }
    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("Total retry = %d.\n", ctr);
    }

    if (is_configed != TRUE) {
        printf("%s: Failed to set TSN WiFi %s.\n", __FUNCTION__, set_type);
        return (FAILED);
    }
    return (PASSED);
}

/********************************************************************
 *
 * Function: nc_check_test_status
 *
 * Description:  open a file /tmp/nc_reselt for check the status 
 *               is pass or fail 
 *
 * Inputs      : None
 * Outputs     : PASSED/FAILED
 *
 **********************************************************************/
int nc_check_test_status (void)
{
    return (PASSED);
}

/********************************************************************
 *
 * Function: wlan_io_test
 *
 * Description:  wrapper for wlan_bootup test 
 *
 * Inputs      : None
 * Outputs     : PASSED/FAILED
 *
 **********************************************************************/
int wlan_io_test (void)
{
    return (wlan_bootup_test());
}

/*******************************************************************************
 *
 * Function   : wifi_confirm_gpio
 * Description: Function to confirm TSN WiFi GPIO configuration.
 * Inputs     : tty_dev
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
static int wifi_confirm_gpio (char *tty_dev)
{
    char cmd_str[TSN_WIFI_PARMS_LENGTH];
    char chk_cmd[TSN_WIFI_PARMS_LENGTH];
    char cmp_str[TSN_WIFI_PARMS_LENGTH];
    uint32_t addr = 0;
    uint16_t value = 0;
    int ctr = 0, result = FAILED;
    int set_ctr = 0, total_num = 0;
    pthread_t threads;
    tsn_uart uart;
    wifi_gpio_t *gpio_p;

    uart.dev = tty_dev;

    total_num = (int)(sizeof(tsn_wifi_gpio_tbl) / sizeof(wifi_gpio_t));
    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("total_num = %d", total_num);
    }

    for (set_ctr = 0; set_ctr < total_num; set_ctr++) {
        result = FAILED;
        memset(cmd_str, 0, sizeof(cmd_str));
        memset(chk_cmd, 0, sizeof(chk_cmd));
        memset(cmp_str, 0, sizeof(cmp_str));
        gpio_p = &tsn_wifi_gpio_tbl[set_ctr];

        addr = (uint32_t)(TLMM_GPIO_CONF_REG_OFFSET(gpio_p->gpio_num));
        value = gpio_p->value;

        sprintf(cmd_str, "mw %#x %#x\n", addr, value);
        if ((NVRAM)->diagflag & D_VERBOSE) {
            printf("cmd_str = %s", cmd_str);
        }

        sprintf(chk_cmd, "md %#x 1\n", addr);
        if ((NVRAM)->diagflag & D_VERBOSE) {
            printf("chk_cmd = %s", chk_cmd);
        }

        sprintf(cmp_str, "%04x", value);
        if ((NVRAM)->diagflag & D_VERBOSE) {
            printf("cmp_str = %s", cmp_str);
        }

        for (ctr = 0; ctr < TSN_WIFI_MAX_RETRY; ctr++) { 
            memset(uart.buf, 0, sizeof(uart.buf));

            if (tsn_tx_uart(tty_dev, cmd_str) != PASSED) {
                printf("%s: Failed to set GPIO %d.\n",
                       __func__, gpio_p->gpio_num);
                return (FAILED);
            }
            msleep(WAIT_WIFI_ACCESS_TIME);   /* 300ms */

            if(pthread_create(&threads, NULL, wifi_uart_rx, (void *)&uart)) {
                /* Should never occur */
                printf("%s: pthread_create failed.\n", __func__);
                return (FAILED);
            }
            msleep(WAIT_WIFI_ACCESS_TIME);   /* 300ms */

            if (tsn_tx_uart(tty_dev, chk_cmd) != PASSED) {
                printf("%s: Failed to print GPIO %d config.\n",
                       __func__, gpio_p->gpio_num);
                return (FAILED);
            }

            pthread_join(threads, NULL);

            if (!strlen(uart.buf)) {
                printf("%s: [%d]No data received.\n", __func__, ctr);
                break;
            }
            if ((NVRAM)->diagflag & D_VERBOSE) {
                printf("RX = %s", uart.buf);
            }

            /* Confirm set-up by checked RX data */
            if (strstr(uart.buf, cmp_str) != NULL) {
                result = PASSED;
                break;
            }
        }
        if ((NVRAM)->diagflag & D_VERBOSE) {
            printf("Total retry = %d.\n", ctr);
        }

        if (result != PASSED) {
            printf("%s: Failed to configure TSN WiFi %s GPIO.\n",
                   __func__, gpio_p->name);
            return (FAILED);
        }
    }
    return (PASSED);
}

/*******************************************************************************
 *
 * Function   : host_n_wifi_pid_paring_test
 * Description: Function to confirm that if Host and wlan PID is a pair.
 * Inputs     : None
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
static int host_n_wifi_pid_paring_test (char *err_msg)
{
    FILE              *fp;
    int               ret_val = FAILED;
    char              *r_buf = NULL;
    size_t            r_len = 0;
    ssize_t           r_size = -1;
    char              *chk_str = "PASSED";
    char              *token;
    char              host_pid[FRU_SIZE] = {0};
    char              wifi_pid[TSN_WIFI_PID_LENGTH];
    host_wifi_pid_map *map_tbl_p;
    int               map_tbl_size = 0, ctr = 0;

    /* Define Mapping table info */
    map_tbl_p = star_host_wifi_pid_map_tbl;
    map_tbl_size = (int)(sizeof(star_host_wifi_pid_map_tbl) / 
                         sizeof(host_wifi_pid_map));

    /* Get Host PID */
    platform_get_pid(host_pid);

    /* Get WiFi module PID */
    /* Initialize WiFi PID buffer */
    memset(wifi_pid, 0, sizeof(wifi_pid));

    prpass(testpass, "Getting WiFi PID through NC ");
    if (tsn_wifi_nc_dispatch_comm(GET_WIFI_PID_NC) != PASSED) {
        sprintf(err_msg, "Failed to get WiFi PID through NC command.");
        return (FAILED);
    }

    if ((fp = fopen(TSN_NC_DONE_FILE, "r")) == NULL) {
        sprintf(err_msg, "Failed to open file %s to get the read back WiFi PID.",
                         TSN_NC_DONE_FILE);
        return (FAILED);
    }

    ret_val = FAILED;
    while ((r_size = getline(&r_buf, &r_len, fp)) != -1) {
        if (strstr(r_buf, chk_str) != NULL) {
            token = strtok(r_buf, WIFI_NC_RETDATA_DELIMITER);

            while (token != NULL) {
                token = strtok(NULL, WIFI_NC_RETDATA_DELIMITER);
                break;
            }
            strcpy(wifi_pid, token);
            ret_val = PASSED;
            break;
        }
    }

    if (fclose(fp) != 0) {
        sprintf(err_msg, "Filed to close %s - %s.",
                         TSN_NC_DONE_FILE, strerror(errno));
        return (FAILED);
    }

    if (ret_val != PASSED) {
        sprintf(err_msg, "Filed to get WiFi PID.");
        return (FAILED);
    }

    /* Confirm if Host and WiFi PID is a pair. */
    ret_val = FAILED;
    for (ctr = 0; ctr < map_tbl_size; ctr++, map_tbl_p++) {

        if ((NVRAM)->diagflag & D_VERBOSE) {
            printf("\n[DBG][%d]: WiFi PID is %s; wifi_pid_str is %s\n",
                   ctr, wifi_pid, map_tbl_p->wifi_pid_str);
            printf("           Host PID is %s; host_pid_str is %s\n\n",
                   host_pid, map_tbl_p->host_pid_str);
        }

        if (strstr(wifi_pid, map_tbl_p->wifi_pid_str) != NULL) {
            if (strstr((const char *)host_pid,
                       map_tbl_p->host_pid_str) != NULL) {
                ret_val = PASSED;
                break;
            }
        }
    } 

    if (ret_val != PASSED) {
        sprintf(err_msg, "Installed wrong WiFi module"
                         "(Current Host PID is %s, but WiFi PID is %s).",
                         host_pid, wifi_pid);
    }
    return (ret_val);
}


/*-------------------------------------------------
$Log: wifi_tests.c,v $
Revision 1.6  2019/01/18 05:54:47  yungchen
Merge Supernova branch to the main trunk (CSCvn79871)

Revision 1.5  2018/05/15 09:37:32  steja
CSCvj38863: Enhanced LED single test utility

Revision 1.4  2018/02/09 09:56:56  hondwang
Merge Star branch star-branch-c9xx to main trunk

Revision 1.3  2017/10/19 13:41:11  palin2
Fixed CSCvg23616: TSN PoE link down intermittently when connect to iPorter PoE tester.

Revision 1.2  2017/08/02 14:21:50  steja
Support TSN-H/M platform code

Revision 1.1.8.4  2017/08/01 14:02:05  steja
Enhanced Wifi Diag Kernel boot up

Revision 1.1.8.3  2017/07/31 16:35:47  palin2
Updated WiFi Diag kernel boot up process based on Cisco WiFi bootloader.

Revision 1.1.8.2  2017/07/29 03:41:21  steja
tsn-branch5 synced with Maintrunk repositories

Revision 1.1.6.5  2017/07/25 08:31:56  steja
1. Remove unused code.
2. Verified before check-in

Revision 1.1.6.4  2017/07/24 14:14:11  palin2
1. To improve code readability.
2. All changes are verified before check-in.

Revision 1.1.6.3  2017/07/21 10:46:04  steja
Update based on code review comment

Revision 1.1.6.2  2017/07/20 13:38:08  steja
tsn-branch4 merge with maintrunk

Revision 1.1.4.10.2.3  2017/07/18 14:23:37  steja
Code Cleanup

Revision 1.1.4.10.2.2  2017/07/17 13:54:44  palin2
Code cleanup.

Revision 1.1.4.10.2.1  2017/07/08 07:27:27  steja
Code Clean up

Revision 1.1.4.10  2016/12/23 11:00:15  steja
Add IO interface test

Revision 1.1.4.9  2016/11/16 06:20:50  palin2
Added netcat support for TSN WiFi tests.

Revision 1.1.4.8  2016/10/04 06:39:08  petteng
Add enhanced error message

Revision 1.1.4.7  2016/10/02 20:32:27  palin2
Enhanced WiFi uart code to fix CSCvb53793.

Revision 1.1.4.6  2016/09/07 15:12:52  steja
Add wifi temperature interrupt test

Revision 1.1.4.5  2016/08/16 03:08:18  palin2
Unified test pass print outs.

Revision 1.1.4.4  2016/08/10 12:41:20  palin2
Updated WiFi testing procedure.

Revision 1.1.4.3  2016/07/22 14:52:02  palin2
Added auto release WLAN module from reset in WLAN console switch utility.

Revision 1.1.4.2  2016/06/30 06:22:52  steja
tsn-branch2 sync with main trunk

Revision 1.1.2.3  2016/06/17 15:26:25  palin2
Added WLAN module diags and utilities.

Revision 1.1.2.2  2016/05/24 01:19:26  palin2
Updated WiFi UART console switch setup.

Revision 1.1.2.1  2016/05/20 02:33:07  leschen
Check in wifi codes

*
*/
