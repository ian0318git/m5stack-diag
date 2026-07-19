/* $Id: dsl_tests.c,v 1.5 2018/05/15 09:37:32 steja Exp $ 
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/tsn/dsl_tests.c,v $ 
 *------------------------------------------------------------------
 * 
 *  dsl_tests.c 
 *
 * Copyright (c) 2018 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include "common.h"
#include "types.h"
#include "menu.h"
#include "defs.h"
#include "menu.h"
#include "error.h"
#include "strings.h"
#include "nvmonvars.h"
#include "dsl_tests.h"
#include "common_utils.h"
#include "dsl_libs.h"
#include "tsn_comm.h"
#include "queryflags.h"
#include "linux_ntwk.h" /* tftp_get */
#include "uart_api.h"
#include "platform_fpga.h"
#include "platform_fru.h"
#include "platform_cookie.h"
#include "platform_sensor.h"
#include "plat_defs.h"

/*******************************************************************************
 *                             Function Prototypes                             *
 *******************************************************************************
 */
static int xdsl_check_sku_type(void);
static int xdsl_utils(int);
static int xdsl_param_test(int);
static int xdsl_console_switch(void);
static int xdsl_bcm63168_reset (void);
static int xdsl_bcm63168_led_test (void);
static int xdsl_bcm63168_led_utils (int);
static int xdsl_init_bcm63168 (void);
static int xdsl_config_bcm63168 (void);
static int xdsl_get_bcm63168_config (void);
static int xdsl_get_bcm63168_version (void);
static int xdsl_connection_start (void);
static int xdsl_connection_stop (void);
static int xdsl_get_xdsl_mib_info (void);
static int xdsl_get_xtm_bonding_info (void);
static int xdsl_get_xdsl_info (void);
static int xdsl_get_connection_info (void);
static int xdsl_main_tests(int);
static int xdsl_bootup(void);
static int xdsl_bootflash_test (void);
static int xdsl_interrupt_test(void);
static int xdsl_bcm63168_dram_test (void);
static int xdsl_do_showtime (void);
static int xdsl_do_set_tone (void);
static int xdsl_do_send_all_tone (void);
static int xdsl_do_idle_listen (void);
static int xdsl_select_test_options (void);
static int xdsl_ping_util(void);
static int pri_intf_rdy_chk(void);
static int xdsl_bcm63168_show_profile(void);
static int restore_cfe_param(boolean);
static int xdsl_bcm63168_en_spi_flash_reg(void);
static int xdsl_bcm63168_dis_spi_flash_reg(void);
static int xdsl_bcm63168_show_spi_flash_reg (void);
static int xdsl_bcm63138_set_gfast_relay(void);
static boolean has_gfast_support(void);
int xdsl_first_boot_up(int);

unsigned int tsn_turbo_sku = 0xFFFF;
boolean tsn_gfast_sku = FALSE;
/*******************************************************************************
 *                              Global Variables                               *
 *******************************************************************************
 */
extern uint bcm_op_mode;
extern uint bcm_line_id;
extern uint bcm_qln_monitor_time;
extern uint bcm_qln_monitor_freq;
extern int tsn_module;

extern int do_all_menu_items(struct menuinfo *);
extern int this_is_tsn_dsl_annex_sku(void);
extern int bcm63138_vdsl35b(void);

/*******************************************************************************
 *                                   Menus                                     *
 *******************************************************************************
 */

/*=========================================
 * xDSL Main Tests menu items
 *=========================================
 */
static submenu_xtable_t xdsl_main_tests_submenu_table[] = {
    {"DRAM Test",      (PFT)xdsl_bcm63168_dram_test,   0, MM_3,
     (type_t(*)())0,         0,         (type_t(*)())0,              0},
    {"NOR Flash Test", (PFT)xdsl_bootflash_test,       0, MM_3,
     (type_t(*)())0,         0,         (type_t(*)())0,              0},
    {"Interrupt Test", (PFT)xdsl_interrupt_test,       0, MM_3,
     (type_t(*)())0,         0,         (type_t(*)())0,              0},
};

#define XDSL_MAIN_TESTS_SUBMENU_TABLE_SIZE (sizeof(xdsl_main_tests_submenu_table) / \
                                            sizeof(submenu_xtable_t))

/* 
 * Primary & secondary submenu items (filled in from xtable)
 */
static mitem_t xdsl_main_tests_primary_items[XDSL_MAIN_TESTS_SUBMENU_TABLE_SIZE +
                                             MAX_BASE_ITEMS];
static mitem_t xdsl_main_tests_secondary_items[XDSL_MAIN_TESTS_SUBMENU_TABLE_SIZE +
                                               MAX_BASE_ITEMS];
menuinfo_t xdsl_main_subtest_menu = {
    "%s Subtest Menu",
    0,                          /* mtparam added by init_empty_menu */
    (PFT) show_endnote,         /* notes missing WICs in combos */
    0,                          /* use generic prompt */
    0,                          /* size (bumped by add_menu_item() */
    xdsl_main_tests_primary_items,
};

menuinfo_t *xdsl_main_submenup = &xdsl_main_subtest_menu;

/*=========================================
 * Utilities menu items
 *=========================================
 */
static submenu_xtable_t xdsl_utils_submenu_table[] = {
    {"xDSL Reset",          (PFT)xdsl_bcm63168_reset, 0, 0,
      (type_t(*)())0, 0,   (type_t(*)())0,          0},
    {"Initialize xDSL",         (PFT)xdsl_init_bcm63168, 0, 0,
      (type_t(*)())0, 0,   (type_t(*)())0,          0},
    {"Configure xDSL",          (PFT)xdsl_config_bcm63168, 0, 0,
      (type_t(*)())0, 0,   (type_t(*)())0,          0},
    {"Get xDSL configuration",  (PFT)xdsl_get_bcm63168_config, 0, 0,
      (type_t(*)())0, 0,   (type_t(*)())0,          0},
    {"Get xDSL version",        (PFT)xdsl_get_bcm63168_version, 0, 0,
      (type_t(*)())0, 0,   (type_t(*)())0,          0},
    {"xDSL Connection Start",            (PFT)xdsl_connection_start, 0, 0,
      (type_t(*)())0, 0,   (type_t(*)())0,          0},
    {"xDSL Connection Stop",             (PFT)xdsl_connection_stop, 0, 0,
      (type_t(*)())0, 0,   (type_t(*)())0,          0},
    {"Get xDSL Mib Info",           (PFT)xdsl_get_xdsl_mib_info, 0, 0,
      (type_t(*)())0, 0,   (type_t(*)())0,          0},
    {"Get xDSL Bonding Info",       (PFT)xdsl_get_xtm_bonding_info, 0, 0,
      (type_t(*)())0, 0,   (type_t(*)())0,          0},
    {"Get xDSL Info",               (PFT)xdsl_get_xdsl_info, 0, 0,
      (type_t(*)())0, 0,   (type_t(*)())0,          0},
    {"Get xDSL Connection Info",         (PFT)xdsl_get_connection_info, 0, 0,
      (type_t(*)())0, 0,   (type_t(*)())0,          0},
    {"LED Test",                    (PFT)xdsl_bcm63168_led_test, 0, 0,
      (type_t(*)())0, 0,   (type_t(*)())0,          0},
    {"Show xDSL Profile",    (PFT)xdsl_bcm63168_show_profile, 0, 0,
      (type_t(*)())0, 0,   (type_t(*)())0,          0},
    {"Ping xDSL",    (PFT)xdsl_ping_util, 0, 0,
      (type_t(*)())0, 0,   (type_t(*)())0,          0},
    {"Display xDSL SPI Flash Registers",
      (PFT)xdsl_bcm63168_show_spi_flash_reg, 0, 0,
      (type_t(*)())0, 0,   (type_t(*)())0,          0},
    {"Enable SPI write protect",    (PFT)xdsl_bcm63168_en_spi_flash_reg,
        0, 0, (type_t(*)())0, 0,   (type_t(*)())0,          0},
    {"Disable SPI write protect",    (PFT)xdsl_bcm63168_dis_spi_flash_reg,
        0, 0, (type_t(*)())0, 0,   (type_t(*)())0,          0},
    {"Restore CFE IOS Parameters",
      (PFT)restore_cfe_param, IOS_CFE_PARAM, 0,
      (type_t(*)())0, 0,   (type_t(*)())0,          0},
    {"Restore CFE DIAG Parameters",
      (PFT)restore_cfe_param, DIAG_CFE_PARAM, 0,
      (type_t(*)())0, 0,   (type_t(*)())0,          0},
	{"Set G.Fast Relay",
      (PFT)xdsl_bcm63138_set_gfast_relay, 0, 0,
      (type_t(*)())has_gfast_support, 0,   (type_t(*)())0,          0},
    {"LED OFF",                    (PFT)xdsl_bcm63168_led_utils, LED_OFF, 0,
      (type_t(*)())0, 0,   (type_t(*)())0,          0},
    {"LED Carrier Detect ON",      (PFT)xdsl_bcm63168_led_utils, LED_CD_ON, 0,
      (type_t(*)())0, 0,   (type_t(*)())0,          0},
    {"LED Data ON",                (PFT)xdsl_bcm63168_led_utils, LED_DATA_ON, 0,
      (type_t(*)())0, 0,   (type_t(*)())0,          0},
};

#define xDSL_UTILS_SUBMENU_TABLE_SZ (sizeof(xdsl_utils_submenu_table) / sizeof(submenu_xtable_t))

static mitem_t xdsl_utils_primary_items[xDSL_UTILS_SUBMENU_TABLE_SZ +
                                        MAX_BASE_ITEMS];
static mitem_t xdsl_utils_secondary_items[xDSL_UTILS_SUBMENU_TABLE_SZ +
                                          MAX_BASE_ITEMS];

char xdslutiltitle[50];

menuinfo_t xdsl_util_submenu = {
    xdslutiltitle,
    0,                              /* mtparam added by init_empty_menu */
    (PFT)menu_show_dflags,          /* notes missing WICs in combos */
    0,                              /* use generic prompt */
    0,                              /* size (bumped by add_menu_item() */
    xdsl_utils_primary_items,
};

menuinfo_t *xdsl_util_submenup = &xdsl_util_submenu;

/*=========================================
 * Showtime and Parametric Tests menu items
 *=========================================
 */
static submenu_xtable_t xdsl_pt_submenu_table[] = {
    {"Do Showtime",         (PFT)xdsl_do_showtime,          0,
     MF_CONTINUOUS | MF_SHOW_ERRCOUNT,
     (type_t(*)())0,         0,         (type_t(*)())0,              0},
    {"Do Set Tones",        (PFT)xdsl_do_set_tone,          0,
     MF_SHOW_ERRCOUNT,
     (type_t(*)())0,         0,         (type_t(*)())0,              0},
    {"Do Send All Tones",   (PFT)xdsl_do_send_all_tone,     0,
     MF_SHOW_ERRCOUNT,
     (type_t(*)())0,         0,         (type_t(*)())0,              0},
    {"Do Idle Listen",      (PFT)xdsl_do_idle_listen,       0,
     MF_SHOW_ERRCOUNT,
     (type_t(*)())0,         0,         (type_t(*)())0,              0},
    {"Set Idle Mode",       (PFT)xdsl_connection_stop,      0,
     MF_SHOW_ERRCOUNT,
     (type_t(*)())0,         0,         (type_t(*)())0,              0},     
    {"Select test options", (PFT)xdsl_select_test_options,  0,
     MF_SHOW_ERRCOUNT,
     (type_t(*)())0,         0,         (type_t(*)())0,              0},     
    {"Set VDSL2 35b profile", (PFT)bcm63138_vdsl35b,        0,
     MF_SHOW_ERRCOUNT,
     (type_t(*)())has_gfast_support, 0, (type_t(*)())0,              0},     
};

#define XDSL_PT_SUBMENU_TABLE_SIZE (sizeof(xdsl_pt_submenu_table) / \
                                    sizeof(submenu_xtable_t))

/* 
 * Primary & secondary submenu items (filled in from xtable)
 */
static mitem_t xdsl_pt_tests_primary_items[XDSL_PT_SUBMENU_TABLE_SIZE +
                                           MAX_BASE_ITEMS];
static mitem_t xdsl_pt_tests_secondary_items[XDSL_PT_SUBMENU_TABLE_SIZE +
                                             MAX_BASE_ITEMS];

static menuinfo_t xdsl_pt_subtest_menu = {
    "%s Subtest Menu",
    0,                                /* mtparam added by init_empty_menu */
    (PFT)show_endnote,                /* notes missing WICs in combos */
    0,                                /* use generic prompt */
    0,                                /* size (bumped by add_menu_item()) */
    xdsl_pt_tests_primary_items,
};

static menuinfo_t *xdsl_pt_submenup = &xdsl_pt_subtest_menu;


/*=========================================
 * Main menu items
 *=========================================
 */
static submenu_xtable_t xdsl_tests_submenu_table[] = {
    {"xDSL Utilities",      (PFT)xdsl_utils,        0,   0,
     (type_t(*)())0, 0,         (type_t(*)())xdsl_utils,    0},
    {"xDSL Bootup",         (PFT)xdsl_bootup,       0,  MM_3,
     (type_t(*)())0, 0,         (type_t(*)())0,                     0},
    {"xDSL Main Tests",      (PFT)xdsl_main_tests,    0,  MM_3,
     (type_t(*)())0, 0,         (PFT)xdsl_main_tests,        TRUE},
    {"Showtime & Parametric Tests", (PFT)xdsl_param_test,       0,   0,
     (type_t(*)())0, 0,         (type_t(*)())xdsl_param_test,   TRUE},
    {"LED Test",                (PFT)xdsl_bcm63168_led_test,0,   0,
     (type_t(*)())0, 0,         (type_t(*)())0,                     0},
    {"xDSL Switch Console", (PFT)xdsl_console_switch,    0,   0,
     (type_t(*)())0, 0,         (type_t(*)())0,                     0},
};

#define XDSL_TESTS_SUBMENU_TABLE_SIZE \
        (sizeof(xdsl_tests_submenu_table) / sizeof(submenu_xtable_t))
        
static mitem_t xdsl_tests_primary_items[XDSL_TESTS_SUBMENU_TABLE_SIZE + MAX_BASE_ITEMS];
static mitem_t xdsl_tests_secondary_items[XDSL_TESTS_SUBMENU_TABLE_SIZE + MAX_BASE_ITEMS];

menuinfo_t xdsl_subtest_menu = {
    "%s Subtest Menu",
    0,                          /* mtparam added by init_empty_menu */
    (PFT) show_endnote,         /* notes missing WICs in combos */
    0,                          /* use generic prompt */
    0,                          /* size (bumped by add_menu_item() */
    xdsl_tests_primary_items,
};

menuinfo_t *xdsl_submenup = &xdsl_subtest_menu;

/*******************************************************************************
 *
 * Function: xdsl_print_spining_wheel
 *
 * Description: Display the spining wheel during the waiting time.
 *  
 * Input:  Ring cycle
 *
 * Output: none
 *
 *******************************************************************************
 */
static void xdsl_print_spining_wheel (int pass)
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

static int xdsl_linux_cret (void)
{
    const int maxlen = TSN_DSL_MAX_LENGTH;
    char tty[maxlen];
    int fd;

    printf("\n");
    fflush(stdout);

    snprintf(tty, maxlen-1, TSN_DSL_UART_DEV_STR);
    fd = open(tty, O_RDWR|O_NOCTTY);
    if (tty < 0) {
        perror("open tty failed");
        fflush(stdout);
        return (FAILED);
    }

    if (tsn_uart_tx(fd, TSN_DSL_CR_STRING) == FAILED) {
        goto exit_linux_cret_failed;
    }

    if (tsn_uart_tx(fd, TSN_DSL_CR_STRING) == FAILED) {
        goto exit_linux_cret_failed;
    }
    
    close(fd);
    return (PASSED);

exit_linux_cret_failed:
    close(fd);
    return (FAILED);
}

/*******************************************************************************
 *
 * Function: xdsl_linux_ip_set
 *
 * Description: This function sets xDSL module linux IP address
 *
 * Input : None
 *
 * Output: PASSED/FAILED
 *
 *******************************************************************************
 */
static int xdsl_linux_ip_set (void)
{
    int ix;
    const int maxlen = TSN_DSL_MAX_LENGTH;
    char tty[maxlen];
    char linux_ip_str[maxlen];
    int fd, res;
    int setip_wait_retry = TSN_DSL_LINUX_PROMPT_RETRY;

    prpass(testpass, "Set xdsl Linux IP address,");
    printf("\n");

    printf("Start Linux UART...\n");
    fflush(stdout);

    snprintf(tty, maxlen-1, TSN_DSL_UART_DEV_STR);
    fd = open(tty, O_RDWR|O_NOCTTY);
    if (tty < 0) {
        perror("open tty failed");
        fflush(stdout);
        return (FAILED);
    }

    /* Step 1 : Find ">" prompt */
    for (ix = 0; ix < setip_wait_retry; ix++) {
        if (tsn_uart_tx(fd, TSN_DSL_CR_STRING) == FAILED) {
            goto exit_linux_ip_set_failed;
        }
        res = tsn_uart_rx_polling(fd, TSN_DSL_LINUX_PROMPT,
        		                   DSL_LINUX_TIMEOUT);
        if (res != FALSE) {
            printf("Found : %s\n", TSN_DSL_LINUX_PROMPT);
            fflush(stdout);
            break;
        }
        msleep(WAIT_DSL_LINUX_PROMPT);
    }
    if (ix == setip_wait_retry) {
        cterr('f',0,"Failed to get Linux prompt");
        close(fd);
        return (FAILED);
    }

    /* Step 2: Set Linux IP address */
    sprintf(linux_ip_str, "%s%s.%d", TSN_DSL_IFCONFIG_BR0_STRING,
    		TSN_DIAG_DSL_SUBNET_STR, TSN_DIAG_MODULE_IP_ADDR);
    if (tsn_uart_tx(fd, linux_ip_str) == FAILED) {
        goto exit_linux_ip_set_failed;
    }
    printf("Send Linux IP [%s]\n", linux_ip_str);
    fflush(stdout);
    if (tsn_uart_tx(fd, TSN_DSL_CR_STRING) == FAILED) {
        goto exit_linux_ip_set_failed;
    }

    /* Step 3: Enter shell mode */
    if (tsn_uart_tx(fd, TSN_DSL_SHELL_STRING) == FAILED) {
        goto exit_linux_ip_set_failed;
    }
    printf("Send [%s]\n", TSN_DSL_SHELL_STRING);
    fflush(stdout);
    if (tsn_uart_tx(fd, TSN_DSL_CR_STRING) == FAILED) {
        goto exit_linux_ip_set_failed;
    }

    close(fd);
    return (PASSED);

exit_linux_ip_set_failed:
    close(fd);
    return (FAILED);
}

/*******************************************************************************
 *
 * Function: xdsl_cfe_parms_set
 *
 * Description: This function set cfe parameters for kernel bootup.
 *
 * Input : os_param = IOS or Diag params
 *
 * Output: PASSED/FAILED
 *
 *******************************************************************************
 */
static int xdsl_cfe_parms_set (boolean os_param)
{
    int ix;
    const int maxlen = TSN_DSL_MAX_LENGTH;
    char tty[maxlen];
    char linux_file_str[maxlen];
    char linux_ip_str[maxlen];
    int result = PASSED;
    int tty_desc;
    char *dsl_img_filename;

    if (os_param == IOS_CFE_PARAM) { 
        prpass(testpass, "Set xdsl CFE IOS Parameters,");
    } else {
        prpass(testpass, "Set xdsl CFE DIAG Parameters,");
    }
    printf("\n");

    printf("Start CFE UART...\n");
    fflush(stdout);
    fflush(stderr);

    snprintf(tty, maxlen-1, TSN_DSL_UART_DEV_STR);
    tty_desc = open(tty, O_RDWR | O_NOCTTY);
    if (tty_desc < 0) {
        perror("open tty failed");
        fflush(stdout);
        return (FAILED);
    }

    /* Step 0 : Catch "Press any key" */
    result = tsn_uart_rx_polling(tty_desc, TSN_DSL_PRESS_KEY_STRING,
    		                      DSL_CFE_PRESS_KEY_TIMEOUT);
    if (result != FALSE) {
        printf("Found : %s\n", TSN_DSL_PRESS_KEY_STRING);
        fflush(stdout);
    } else {
        if ((tsn_gfast_sku == TRUE) && (os_param == DIAG_CFE_PARAM)) {
            /* In GFAST case first article of CFE doesn't include pre-programmed
             * board id then CFE will prompt user to configure board parameters
             * first, like board ID, MAC Address, etc.
             */
            if (xdsl_first_boot_up(tty_desc) == FAILED) {
                printf("[%s] Not Found.\n", TSN_DSL_PRESS_KEY_STRING);
                fflush(stdout);
                goto exit_cfe_parms_set_failed;
            }
            /* Hard reset */
            printf("Set board ID completed, reset xdsl.\n");
            xdsl_bcm63168_reset();
            /* Catch "Press any key" */
            result = tsn_uart_rx_polling(tty_desc, TSN_DSL_PRESS_KEY_STRING,
                                         DSL_CFE_PRESS_KEY_TIMEOUT);
            if (result != FALSE) {
                printf("Found : %s\n", TSN_DSL_PRESS_KEY_STRING);
                fflush(stdout);
            } else {
                printf("[%s] Not Found.\n", TSN_DSL_PRESS_KEY_STRING);
                fflush(stdout);
                goto exit_cfe_parms_set_failed;
            }
        } else {
            printf("[%s] Not Found.\n", TSN_DSL_PRESS_KEY_STRING);
            fflush(stdout);
            goto exit_cfe_parms_set_failed;
        }     
    } 

    /* Step 1 : Find "CFE>" */
    for (ix = 0; ix < DSL_CFE_RETRY_TIMES; ix++) {
        if (tsn_uart_tx(tty_desc, TSN_DSL_CR_STRING) == FAILED) {
            goto exit_cfe_parms_set_failed;
        }
        result = tsn_uart_rx_polling(tty_desc, TSN_DSL_CFE_STRING,
        		                      DSL_CFE_TIMEOUT);
        if (result != FALSE) {
            printf("Found : %s\n", TSN_DSL_CFE_STRING);
            fflush(stdout);
            break;
        }
    }

    if (ix == DSL_CFE_RETRY_TIMES) {
        printf("[%s] Not Found.\n", TSN_DSL_CFE_STRING);
        fflush(stdout);
        goto exit_cfe_parms_set_failed;
    }
    msleep(WAIT_DSL_CFE_PROMPT);

    /* Step 2 : Set board parameters   */
    if (tsn_uart_tx(tty_desc, TSN_DSL_CHANGE_CFE_PARMS_STRING) == FAILED) {
        goto exit_cfe_parms_set_failed;
    }    
    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("Send bdpm [%s]\n", TSN_DSL_CHANGE_CFE_PARMS_STRING);
        fflush(stdout);
    }
    if (tsn_uart_tx(tty_desc, TSN_DSL_CR_STRING) == FAILED) {
        goto exit_cfe_parms_set_failed;
    }
    msleep(WAIT_DSL_UART_TX);

    if (os_param == IOS_CFE_PARAM) { 
        sprintf(linux_ip_str, "%s.%d:ffffff00", TSN_IOS_DSL_SUBNET_STR,
                TSN_IOS_MODULE_IP_ADDR);
    } else {
        sprintf(linux_ip_str, "%s.%d", TSN_DIAG_DSL_SUBNET_STR,
                TSN_DIAG_MODULE_IP_ADDR);
    }
    /* Step 3: Set Board IP */
    if (tsn_uart_tx(tty_desc, linux_ip_str) == FAILED) {
        goto exit_cfe_parms_set_failed;
    } 
    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("Send Board IP[%s]\n", linux_ip_str);
        fflush(stdout);
    }
    if (tsn_uart_tx(tty_desc, TSN_DSL_CR_STRING) == FAILED) {
        goto exit_cfe_parms_set_failed;
    }
    msleep(WAIT_DSL_UART_TX);

    if (os_param == IOS_CFE_PARAM) { 
        sprintf(linux_ip_str, "%s.%d", TSN_IOS_DSL_SUBNET_STR,
                TSN_IOS_HOST_IP_ADDR);
    } else {
        sprintf(linux_ip_str, "%s.%d", TSN_DIAG_DSL_SUBNET_STR,
                TSN_DIAG_HOST_IP_ADDR);
    }
    /* Step 4: Set Host IP */
    if (tsn_uart_tx(tty_desc, linux_ip_str) == FAILED) {
        goto exit_cfe_parms_set_failed;
    } 
    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("Send Host IP[%s]\r", linux_ip_str);
        fflush(stdout);
    }
    if (tsn_uart_tx(tty_desc, TSN_DSL_CR_STRING) == FAILED) {
        goto exit_cfe_parms_set_failed;
    }
    msleep(WAIT_DSL_UART_TX);

    if (os_param == IOS_CFE_PARAM) { 
        sprintf(linux_ip_str, "%s.%d", TSN_IOS_DSL_SUBNET_STR,
                TSN_IOS_GATEWAY_IP_ADDR);
    } else {
        sprintf(linux_ip_str, "%s.%d", TSN_DIAG_DSL_SUBNET_STR,
                TSN_DIAG_GATEWAY_IP_ADDR);
    }
    /* Step 5: Set Gateway IP */
    if (tsn_uart_tx(tty_desc, linux_ip_str) == FAILED) {
        goto exit_cfe_parms_set_failed;
    } 
    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("Send GW IP[%s]\n", linux_ip_str);
        fflush(stdout);
    }
    if (tsn_uart_tx(tty_desc, TSN_DSL_CR_STRING) == FAILED) {
        goto exit_cfe_parms_set_failed;
    }
    msleep(WAIT_DSL_UART_TX);
    
    if (FLAG_CFE_BOOT_TWO_IMG == TRUE) { 
        /* Step 6: Set where image to run from */
        if (tsn_uart_tx(tty_desc, TSN_DSL_RUN_IMAGE_LOCATION) == FAILED) {
            goto exit_cfe_parms_set_failed;
        }    
        if ((NVRAM)->diagflag & D_VERBOSE) {
            printf("Send run image location[%s]\n", TSN_DSL_RUN_IMAGE_LOCATION);
            fflush(stdout);
        }
        if (tsn_uart_tx(tty_desc, TSN_DSL_CR_STRING) == FAILED) {
            goto exit_cfe_parms_set_failed;
        }
        msleep(WAIT_DSL_UART_TX);

        /* Step 7: Set default linux file name */
        sprintf(linux_file_str, "%s", GFAST_HOST_RUN_FILE);
        if (tsn_uart_tx(tty_desc, linux_file_str) == FAILED) {
            goto exit_cfe_parms_set_failed;
        }
        printf("host run image [%s]\n", linux_file_str);
        fflush(stdout);
        if (tsn_uart_tx(tty_desc, TSN_DSL_CR_STRING) == FAILED) {
            goto exit_cfe_parms_set_failed;
        }
        msleep(WAIT_DSL_UART_TX);
        /* host flash file name */ 
        if (tsn_uart_tx(tty_desc, TSN_DSL_CR_STRING) == FAILED) {
            goto exit_cfe_parms_set_failed;
        }
        msleep(WAIT_DSL_UART_TX);
        /* boot delay */
        if (tsn_uart_tx(tty_desc, TSN_DSL_CR_STRING) == FAILED) {
            goto exit_cfe_parms_set_failed;
        }
        msleep(WAIT_DSL_UART_TX);
        
        /* Step 8: Set ramdisk store address */
        sprintf(linux_file_str, "%s", GFAST_HOST_RAMDISK_FILE);
        if (tsn_uart_tx(tty_desc, linux_file_str) == FAILED) {
            goto exit_cfe_parms_set_failed;
        }
        printf("host ramdisk image [%s]\n", linux_file_str);
        fflush(stdout);
        if (tsn_uart_tx(tty_desc, TSN_DSL_CR_STRING) == FAILED) {
            goto exit_cfe_parms_set_failed;
        }
        msleep(WAIT_DSL_UART_TX);

        /* Step 8: Set ramdisk store address */
        if (tsn_uart_tx(tty_desc, GFAST_RAMDISK_STORE_ADDR) == FAILED) {
            goto exit_cfe_parms_set_failed;
        }    
        if ((NVRAM)->diagflag & D_VERBOSE) {
            printf("Send run image location[%s]\n", GFAST_RAMDISK_STORE_ADDR);
            fflush(stdout);
        }
        if (tsn_uart_tx(tty_desc, TSN_DSL_CR_STRING) == FAILED) {
            goto exit_cfe_parms_set_failed;
        }
        msleep(WAIT_DSL_UART_TX);

    } else { /* CFE BOOT SINGLE IMG */
        /* Step 6: Set where image to run from */
        if (tsn_uart_tx(tty_desc, TSN_DSL_RUN_IMAGE_LOCATION_H) == FAILED) {
            goto exit_cfe_parms_set_failed;
        }    
        if ((NVRAM)->diagflag & D_VERBOSE) {
            printf("Send run image location[%s]\n", TSN_DSL_RUN_IMAGE_LOCATION);
            fflush(stdout);
        }

        /* Step 6: one more carrier return */
        if (tsn_uart_tx(tty_desc, TSN_DSL_CR_STRING) == FAILED) {
            goto exit_cfe_parms_set_failed;
        } 
        msleep(WAIT_DSL_UART_TX);

        /* Step 7: Set default linux file name */
        /* Check environment parameter, if not use default filename*/ 
        if (os_param == IOS_CFE_PARAM) { 
            dsl_img_filename = getenv(IOS_IMAGE_NAME);
        } else {
            dsl_img_filename = getenv(DSL_IMAGE_NAME);
        }

        if (dsl_img_filename == NULL || !strcmp("none", dsl_img_filename)) {
            if (os_param == IOS_CFE_PARAM) { 
                printf("%s: Env variable %s is not set\n", __func__, IOS_IMAGE_NAME);
            } else {
                printf("%s: Env variable %s is not set\n", __func__, DSL_IMAGE_NAME);
            }
            goto exit_cfe_parms_set_failed;
        } else {
            if (os_param == IOS_CFE_PARAM) { 
                printf("IOS Image Filename: %s\n", dsl_img_filename);
                sprintf(linux_file_str, "%s%s", 
                        TSN_IOS_HOST_FIRMWARE_FOLDER_STRING, dsl_img_filename);
            } else {
                printf("DSL Image Filename: %s\n", dsl_img_filename);
                sprintf(linux_file_str, "%s", dsl_img_filename);
            }
        }
        
        if (tsn_uart_tx(tty_desc, linux_file_str) == FAILED) {
            goto exit_cfe_parms_set_failed;
        }
        printf("Diag image [%s]\n", linux_file_str);
        fflush(stdout);
        if (tsn_uart_tx(tty_desc, TSN_DSL_CR_STRING) == FAILED) {
            goto exit_cfe_parms_set_failed;
        }
        msleep(WAIT_DSL_UART_TX);
    }  /* End of setup boot CFE with one or two images */

    if (os_param == IOS_CFE_PARAM) { 
        /* Set host flash file */
        if (tsn_uart_tx(tty_desc, BCM963XX_FS_KERNEL) == FAILED) {
            goto exit_cfe_parms_set_failed;
        }    
        if ((NVRAM)->diagflag & D_VERBOSE) {
            printf("Send DSL host flash image [%s]\n", BCM963XX_FS_KERNEL);
            fflush(stdout);
        }
        if (tsn_uart_tx(tty_desc, TSN_DSL_CR_STRING) == FAILED) {
            goto exit_cfe_parms_set_failed;
        } 
        msleep(WAIT_DSL_UART_TX);

        if (tsn_uart_tx(tty_desc, TSN_DSL_CR_STRING) == FAILED) {
            goto exit_cfe_parms_set_failed;
        } 
        msleep(WAIT_DSL_UART_TX);

        /* Clear ramdisk file after booting up by diag*/
        if (tsn_uart_tx(tty_desc, CLEAR_PARAM) == FAILED) {
            goto exit_cfe_parms_set_failed;
        }    
        if ((NVRAM)->diagflag & D_VERBOSE) {
            printf("Remove ramdisk image [%s]\n", CLEAR_PARAM);
            fflush(stdout);
        }
        if (tsn_uart_tx(tty_desc, TSN_DSL_CR_STRING) == FAILED) {
            goto exit_cfe_parms_set_failed;
        } 
        msleep(WAIT_DSL_UART_TX);
       
        /* ramdisk store address  */ 
        if (tsn_gfast_sku == FALSE) { 
            /* For TSN-M clear ramdisk store address */
            if (tsn_uart_tx(tty_desc, CLEAR_PARAM) == FAILED) {
                goto exit_cfe_parms_set_failed;
            }    
            if ((NVRAM)->diagflag & D_VERBOSE) {
                printf("Remove ramdisk store addr [%s]\n", CLEAR_PARAM);
                fflush(stdout);
            }
        } else {
            /* For TSN GFast set ramdisk store address */
            if (tsn_uart_tx(tty_desc, GFAST_RAMDISK_STORE_ADDR) == FAILED) {
                goto exit_cfe_parms_set_failed;
            }    
            if ((NVRAM)->diagflag & D_VERBOSE) {
                printf("Set ramdisk store addr [%s]\n", GFAST_RAMDISK_STORE_ADDR);
                fflush(stdout);
            }
        }

        if (tsn_uart_tx(tty_desc, TSN_DSL_CR_STRING) == FAILED) {
            goto exit_cfe_parms_set_failed;
        } 
        msleep(WAIT_DSL_UART_TX);
    }

    /* Step 8 : Find "command status" */
    for (ix = 0; ix < DSL_CFE_RETRY_TIMES; ix++) {
        if (tsn_uart_tx(tty_desc, TSN_DSL_CR_STRING) == FAILED) {
            goto exit_cfe_parms_set_failed;
        }
        result = tsn_uart_rx_polling(tty_desc, TSN_DSL_COMMAND_STATUS_STRING,
        		                      DSL_CFE_COMMAND_STATUS_TIMEOUT);
        if (result != FALSE) {
            printf("Found : %s\n", TSN_DSL_COMMAND_STATUS_STRING);
            fflush(stdout);
            break;
        }
        msleep(WAIT_DSL_CFE_COMMAND_STATUS);
    }

    if (ix == DSL_CFE_RETRY_TIMES) {
        printf("[%s] Not Found.\n", TSN_DSL_COMMAND_STATUS_STRING);
        fflush(stdout);
        goto exit_cfe_parms_set_failed;
    }
    msleep(WAIT_DSL_CFE_COMMAND_STATUS);

    /* Step 9 : Find "CFE>" */
    for (ix = 0; ix < DSL_CFE_RETRY_TIMES; ix++) {
        if (tsn_uart_tx(tty_desc, TSN_DSL_CR_STRING) == FAILED) {
            goto exit_cfe_parms_set_failed;
        }
        result = tsn_uart_rx_polling(tty_desc, TSN_DSL_CFE_STRING,
        		                      DSL_CFE_TIMEOUT);
        if (result != FALSE) {
            printf("Found : %s\n", TSN_DSL_CFE_STRING);
            fflush(stdout);
            break;
        } 
    }

    if (ix == DSL_CFE_RETRY_TIMES) {
        printf("[%s] Not Found.\n", TSN_DSL_CFE_STRING);
        fflush(stdout);
        goto exit_cfe_parms_set_failed;
    }

    msleep(WAIT_FOR_FLASH_WRITE);

    close(tty_desc);
    return (PASSED);
    
exit_cfe_parms_set_failed:
    close(tty_desc);
    return (FAILED);
}


/*******************************************************************************
 *
 * Function: xdsl_first_boot_up
 *
 * Description: This function set cfe parameters 
 *              while first time boot up DSL chip.
 *
 * Input : None
 *
 * Output: PASSED/FAILED
 *
 *******************************************************************************
 */
int xdsl_first_boot_up (int tty_desc)
{
    int ix;
    int result = PASSED;
    printf("xdsl first boot up.\n");

    if (tsn_uart_tx(tty_desc, TSN_DSL_CR_STRING) == FAILED) {
        return (FAILED);
    } 
    msleep(WAIT_DSL_UART_TX);

    result = tsn_uart_rx_polling(tty_desc, TSN_DSL_BOARD_ID, 
                                          DSL_BOARD_ID_TIMEOUT);
    if (result !=FALSE) {
    /* Set board id */
        if (tsn_uart_tx(tty_desc, TSN_DSL_BOARD_ID_NUM) == FAILED) {
            return (FAILED);
        }    
        if ((NVRAM)->diagflag & D_VERBOSE) {
            printf("Send bdpm [%s]\n", TSN_DSL_BOARD_ID_NUM);
            fflush(stdout);
        }
        if (tsn_uart_tx(tty_desc, TSN_DSL_CR_STRING) == FAILED) {
            return (FAILED);
        }
        msleep(WAIT_DSL_UART_TX);
        printf("Set board id %s.\n", TSN_DSL_BOARD_ID_NUM);

        /* Set MAC address */
        if (tsn_uart_tx(tty_desc, TSN_DSL_MAC_NUM) == FAILED) {
            return (FAILED);
        }    
        if ((NVRAM)->diagflag & D_VERBOSE) {
            printf("Send bdpm [%s]\n", TSN_DSL_MAC_NUM);
            fflush(stdout);
        }
        if (tsn_uart_tx(tty_desc, TSN_DSL_CR_STRING) == FAILED) {
            return (FAILED);
        }
        msleep(WAIT_DSL_UART_TX);
        printf("Set num of MAC address %s.\n", TSN_DSL_MAC_NUM);
        if (tsn_uart_tx(tty_desc, TSN_DSL_MAC_ADDR) == FAILED) {
            return (FAILED);
        }    
        if ((NVRAM)->diagflag & D_VERBOSE) {
            printf("send bdpm [%s]\n", TSN_DSL_MAC_ADDR);
            fflush(stdout);
        }
        if (tsn_uart_tx(tty_desc, TSN_DSL_CR_STRING) == FAILED) {
            return (FAILED);
        }
        msleep(WAIT_DSL_UART_TX);
        printf("Set mac address %s.\n", TSN_DSL_MAC_ADDR);
            
        /* set psi size */
        if (tsn_uart_tx(tty_desc, TSN_DSL_PSI_SIZ) == FAILED) {
            return (FAILED);
        }    
        if ((NVRAM)->diagflag & D_VERBOSE) {
            printf("send bdpm [%s]\n", TSN_DSL_PSI_SIZ);
            fflush(stdout);
        }
        if (tsn_uart_tx(tty_desc, TSN_DSL_CR_STRING) == FAILED) {
            return (FAILED);
        }
        msleep(WAIT_DSL_UART_TX);

        /* find "CFE>" */
        for (ix = 0; ix < DSL_CFE_CR_TIMES; ix++) {
            if (tsn_uart_tx(tty_desc, TSN_DSL_CR_STRING) == FAILED) {
                return (FAILED);
            }
            result = tsn_uart_rx_polling(tty_desc, TSN_DSL_CFE_STRING,
        		                                DSL_CFE_TIMEOUT);
            if (result != FALSE) {
                printf("found : %s\n", TSN_DSL_CFE_STRING);
                fflush(stdout);
                break;
            } 
        }
        if (ix == DSL_CFE_RETRY_TIMES) {
            printf("[%s] Not Found.\n", TSN_DSL_CFE_STRING);
            fflush(stdout);
            return (FAILED);
        }
        msleep(WAIT_DSL_CFE_PROMPT);

        /* Set board parameters */
        if (tsn_uart_tx(tty_desc, TSN_DSL_CHANGE_CFE_PARMS_STRING) == FAILED) {
            return (FAILED);
        }    
        if (tsn_uart_tx(tty_desc, TSN_DSL_CR_STRING) == FAILED) {
            return (FAILED);
        }
        msleep(WAIT_DSL_UART_TX);

        /* Skip IP Address, Server IP Address, etc since 'Boot up test'
         * will take care of it.
         * Keep pressing enter until we reach option (f/h/c)
         */
        if (tsn_uart_tx(tty_desc, TSN_DSL_CR_STRING) == FAILED) {
            return (FAILED);
        } 
        msleep(WAIT_DSL_UART_TX);
                
        if (tsn_uart_tx(tty_desc, TSN_DSL_CR_STRING) == FAILED) {
            return (FAILED);
        } 
        msleep(WAIT_DSL_UART_TX);
        
        if (tsn_uart_tx(tty_desc, TSN_DSL_CR_STRING) == FAILED) {
            return (FAILED);
        } 
        msleep(WAIT_DSL_UART_TX);
                
        /* set dsl kernel image location */
        if (tsn_uart_tx(tty_desc, TSN_DSL_RUN_IMAGE_LOCATION_H) == FAILED) {
            return (FAILED);
        }    
        if ((NVRAM)->diagflag & D_VERBOSE) {
            printf("send run image location[%s]\n", TSN_DSL_RUN_IMAGE_LOCATION_H);
            fflush(stdout);
        }
        if (tsn_uart_tx(tty_desc, TSN_DSL_CR_STRING) == FAILED) {
            return (FAILED);
        }
        msleep(WAIT_DSL_UART_TX);
        printf("Set image boot from [%s].\n", TSN_DSL_RUN_IMAGE_LOCATION_H);
    
        /* find "command status" */
        for (ix = 0; ix < DSL_CFE_RETRY_TIMES; ix++) {
            if (tsn_uart_tx(tty_desc, TSN_DSL_CR_STRING) == FAILED) {
                return (FAILED);
            }
            result = tsn_uart_rx_polling(tty_desc, TSN_DSL_COMMAND_STATUS_STRING,
        		                        DSL_CFE_COMMAND_STATUS_TIMEOUT);
            if (result != FALSE) {
                printf("found : %s\n", TSN_DSL_COMMAND_STATUS_STRING);
                fflush(stdout);
                break;
            }
            msleep(WAIT_DSL_CFE_COMMAND_STATUS);
        }

        if (ix == DSL_CFE_RETRY_TIMES) {
            printf("first boot [%s] not found.\n", TSN_DSL_COMMAND_STATUS_STRING);
            fflush(stdout);
            return (FAILED);
        }
        msleep(WAIT_DSL_CFE_COMMAND_STATUS); 

        /* find "CFE>" */
        for (ix = 0; ix < DSL_CFE_RETRY_TIMES; ix++) {
            if (tsn_uart_tx(tty_desc, TSN_DSL_CR_STRING) == FAILED) {
                return (FAILED);
            }
            result = tsn_uart_rx_polling(tty_desc, TSN_DSL_CFE_STRING,
        		                      DSL_CFE_TIMEOUT);
            if (result != FALSE) {
                printf("found : %s\n", TSN_DSL_CFE_STRING);
                fflush(stdout);
                break;
            } 
        }

        if (ix == DSL_CFE_RETRY_TIMES) {
            printf("first boot [%s] Not Found.\n", TSN_DSL_CFE_STRING);
            fflush(stdout);
            return (FAILED);
        }
        msleep(WAIT_DSL_CFE_PROMPT);
    } else {
        printf("[%s] Not Found.\n", TSN_DSL_BOARD_ID);
        fflush(stdout);
        return (FAILED);
    }
    msleep(WAIT_FOR_FLASH_WRITE);
    return (PASSED);
}

/*******************************************************************************
 *
 * Function: xdsl_bootup.
 *
 * Description: This function let xdsl boot up to diag.
 *
 * Input : None
 *
 * Output: PASSED/FAILED
 *
 *******************************************************************************
 */
static int xdsl_bootup (void)
{
    int  ret_val;
    int ix, pass = 0;
    const int maxlen = TSN_DSL_MAX_LENGTH;
    char tname[maxlen];

    if (tsn_gfast_sku == TRUE) {
        sprintf(tname,"%s", "xdsl/G.Fast boot up");
    } else {
        sprintf(tname,"%s","xdsl boot up");
    }
    testname(tname);

    /* Step1: Reset */
    xdsl_bcm63168_reset();

    /* Step2: Set CFE IPs and Filename */ 
    if (xdsl_cfe_parms_set(DIAG_CFE_PARAM) == FAILED) {
        cterr('f',0,"CFE boot parameters set failed.\n");
        return (FAILED);
    }

    /* Step3: Reset */
    xdsl_bcm63168_reset();

    /* Step4: Check if DSL is ready (Timeout 90 secs) */
    for (ix = 0; ix < DSL_LINUX_WAIT_RETRY_TIMES; ix++) {
        if (pri_intf_rdy_chk() == PASSED) {
            printf ("\nxdsl module is ready.\n");
            fflush(stdout);
            break;
        }
        /* wait for xDSL module to get CR and respond with prompt */
        xdsl_print_spining_wheel(pass++);
        msleep(WAIT_DSL_LINUX_PROMPT);
        if ((ix == (30)) || (ix == (35))) {
            xdsl_linux_cret();
        }
    }
    if (ix == DSL_LINUX_WAIT_RETRY_TIMES) {
        cterr('f',0,"Failed to get xDSL module ready");
        return (FAILED);
    }

    /* Step5: Set Linux IPs */
    ret_val = xdsl_linux_ip_set();
    if (ret_val != PASSED) {
    	cterr('f',0,"Failed to set xDSL module IP.");
    	return (FAILED);
    }

    /* Check if xDSL module has identical sku type(BCM GPIO) */
    msleep(WAIT_DSL_IP_SETUP);
    ret_val = xdsl_ping_util();
    if (ret_val != PASSED) {
    	cterr('f',0,"Failed to ping BCM by nc.");
    	return (FAILED);
    }

    ret_val = xdsl_check_sku_type();
    if (ret_val != PASSED) {
        cterr('f',0,"Failed to check SKU type.");
        return (FAILED);
    }

    return (ret_val);
}

/*******************************************************************************
 *
 * Function: dsl_tests().
 *
 * Description: This function is the main entrance for xdsl module test .
 *
 * Input:
 *
 * Output: PASSED
 *
 *******************************************************************************
 */
int dsl_tests (boolean dsl_test_items_executed)
{
    int ret_val = PASSED;
    char cmd[128];
    struct stat sts;
    char tsn_dest_diag_img[64] = TSN_HOST_FIRMWARE_FOLDER_STRING;
    char tsn_src_diag_img[32] = DSL_AM_DIAG_IMG;
    const int maxlen = TSN_DSL_MAX_LENGTH;
    char tty_dev[maxlen];
    char *dsl_img_filename;
    char tname[maxlen];

    if (tsn_gfast_sku == TRUE) {
        sprintf(tname,"%s", "xDSL/G.Fast(BCM63138)");
    } else {
        sprintf(tname,"%s","xDSL(BCM63168)");
    }

    testname(tname);

    tsn_module = TSN_DSL_MODULE;

    tsn_turbo_sku = 0;
    if ((this_is_tsn_dsl_annex_sku() == (DSL138_SKU_GFAST | DSL138_SKU_ANNEX_A)) && (tsn_gfast_sku == TRUE)) {
        printf("G.Fast Annex A\n"); 
        tsn_turbo_sku = DSL138_SKU_GFAST | DSL138_SKU_ANNEX_A;
        tsn_gfast_sku = TRUE;
    } else
    if ((this_is_tsn_dsl_annex_sku() == (DSL138_SKU_GFAST | DSL138_SKU_ANNEX_B)) && (tsn_gfast_sku == TRUE)) {
        printf("G.Fast Annex B/J\n");
        tsn_turbo_sku = DSL138_SKU_GFAST | DSL138_SKU_ANNEX_B;
        tsn_gfast_sku = TRUE;
    } else
    if ((this_is_tsn_dsl_annex_sku() == (DSL138_SKU_GFAST | DSL138_SKU_ANNEX_M)) && (tsn_gfast_sku == TRUE)) {
        printf("G.Fast Annex M\n");
        tsn_turbo_sku = DSL138_SKU_GFAST | DSL138_SKU_ANNEX_M;
        tsn_gfast_sku = TRUE;
    } else
    if ((this_is_tsn_dsl_annex_sku() == DSL_SKU_ANNEX_A) && (tsn_gfast_sku == FALSE)) {
        printf("xDSL Annex A\n"); 
        tsn_turbo_sku = DSL_SKU_ANNEX_A;
        tsn_gfast_sku = FALSE;
    } else
    if ((this_is_tsn_dsl_annex_sku() == DSL_SKU_ANNEX_B) && (tsn_gfast_sku == FALSE)) {
        printf("xDSL Annex B/J\n");
        tsn_turbo_sku = DSL_SKU_ANNEX_B;
        tsn_gfast_sku = FALSE;
    } else
    if ((this_is_tsn_dsl_annex_sku() == DSL_SKU_ANNEX_M) && (tsn_gfast_sku == FALSE)) {
        printf("xDSL Annex M\n");
        tsn_turbo_sku = DSL_SKU_ANNEX_M;
        tsn_gfast_sku = FALSE;
    } else {
        printf("Unknown xDSL Annex??\n");
    }

    /* Get DSL Image file name from environment */
    dsl_img_filename = getenv(DSL_IMAGE_NAME);

    if (dsl_img_filename == NULL || !strcmp("none", dsl_img_filename)) {
        printf("%s: Env variable %s is not set\n", __func__, DSL_IMAGE_NAME);
        cterr('f', 0, "Failed to set env export DSL_IMG_FILE");
        return (FAILED);
    } else {
        printf("DSL Image Filename: %s\n", dsl_img_filename);
        sprintf(tsn_src_diag_img, "%s", dsl_img_filename);
    }
    
    /* Download FW image from the network for the first time */
    sprintf(tsn_dest_diag_img, "%s%s", tsn_dest_diag_img, tsn_src_diag_img);
    if (stat(tsn_dest_diag_img, &sts) == -1) {
        if (tftp_get(0 , tsn_src_diag_img, 0, tsn_dest_diag_img, 0) != PASSED) {
            sprintf(cmd, "rm -f %s", tsn_dest_diag_img);
            system(cmd);
            fflush(stdout);
            cterr('f', 0, "Failed to tftp download firmware to local host");
            return (FAILED);
        }
    }

    if (FLAG_CFE_BOOT_TWO_IMG == TRUE) { 
        /*uncompress combo image*/
        sprintf(cmd, "%s %s -C %s", UNCOMPRESS_CMD, tsn_dest_diag_img, 
                TSN_HOST_FIRMWARE_FOLDER_STRING);
        system(cmd);
    }

    /* Set xdsl tty configuration */
    snprintf(tty_dev, maxlen-1, TSN_DSL_UART_DEV_STR);
    if (tsn_uart_setup(tty_dev) == FAILED) {
        printf("\nFailed to setup UART\n");
    }

    build_primary_submenu(xdsl_tests_submenu_table,
                          XDSL_TESTS_SUBMENU_TABLE_SIZE, tname,
                          &xdsl_submenup);

    build_secondary_submenu(xdsl_tests_submenu_table,
                            XDSL_TESTS_SUBMENU_TABLE_SIZE,
                            xdsl_tests_secondary_items);

    if (dsl_test_items_executed) {
        do_all_menu_items(&xdsl_subtest_menu);
    } else {
        menu(&xdsl_subtest_menu, xdsl_tests_secondary_items, '\0');
    }

    /* Restore CFE diag param become IOS param */
    ret_val = restore_cfe_param(IOS_CFE_PARAM);
    return (ret_val);
}

/*******************************************************************************
 *
 *  Function: xdsl_utils
 *
 *  Description: xdsl Utitlities menu
 *
 *  Input: None 
 *
 *  Returns: PASSED
 *
 *******************************************************************************
 */
static int xdsl_utils (int show_menu)
{
    if (tsn_gfast_sku == TRUE) {
        sprintf(xdslutiltitle, "xDSL/G.Fast Utilities Menu");
    } else { 
        sprintf(xdslutiltitle, "xDSL Utilities Menu");
    }

    build_primary_submenu(xdsl_utils_submenu_table,
                          xDSL_UTILS_SUBMENU_TABLE_SZ,
                          xdslutiltitle, &xdsl_util_submenup);

    build_secondary_submenu(xdsl_utils_submenu_table,
                            xDSL_UTILS_SUBMENU_TABLE_SZ,
                            xdsl_utils_secondary_items);

    menu(xdsl_util_submenup, xdsl_utils_secondary_items, '\0');

    return (PASSED);
}

/*******************************************************************************
 *
 * Function    : xdsl_param_test
 *
 * Description : This function builds menu for showtime and parametric tests
 *
 * Input       : NONE
 *
 * Output      : PASSED
 *
 *******************************************************************************
 */
static int xdsl_param_test (int show_menu)
{
    const int maxlen = TSN_DSL_MAX_LENGTH;
    char tname[maxlen];

    if (tsn_gfast_sku == TRUE) {
        sprintf(tname,"%s", "xDSL/G.Fast Showtime and Parametric");
    } else {
        sprintf(tname,"%s","xDSL Showtime and Parametric");
    }

    testname(tname);

    /* Check xdsl is ready */
    if (pri_intf_rdy_chk() != PASSED) {
        printf ("\nxdsl module is not ready."
                "Please boot up xdsl module first.\n");
        return (PASSED);
    }
    
    build_primary_submenu(xdsl_pt_submenu_table,
                          XDSL_PT_SUBMENU_TABLE_SIZE,
                          tname, &xdsl_pt_submenup);
    build_secondary_submenu(xdsl_pt_submenu_table,
                            XDSL_PT_SUBMENU_TABLE_SIZE,
                            xdsl_pt_tests_secondary_items);

    if (show_menu) {
        menu(xdsl_pt_submenup, xdsl_pt_tests_secondary_items, '\0');
    } else {
        do_all_menu_items(xdsl_pt_submenup);
    }

    return (PASSED);
}

/*******************************************************************************
 *
 * Function   : xdsl_main_tests
 *
 * Description: Entry function of xdsl Diag tests and utilities.
 *
 * Inputs     : show xdsl main test menu option
 *
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
static int xdsl_main_tests (int show_menu)
{
    const int maxlen = TSN_DSL_MAX_LENGTH;
    char tname[maxlen];

    if (tsn_gfast_sku == TRUE) {
        sprintf(tname,"%s", "xDSL/G.Fast Main");
    } else {
        sprintf(tname,"%s","xDSL Main");
    }

    testname(tname);

    /* Check xdsl is ready */
    if (pri_intf_rdy_chk() != PASSED) {
        printf ("\nxdsl module is not ready."
                "Please boot up xdsl module first.\n");
        return (PASSED);
    }

    build_primary_submenu(xdsl_main_tests_submenu_table,
                          XDSL_MAIN_TESTS_SUBMENU_TABLE_SIZE,
                          tname, &xdsl_main_submenup);
    
    build_secondary_submenu(xdsl_main_tests_submenu_table,
                            XDSL_MAIN_TESTS_SUBMENU_TABLE_SIZE,
                            xdsl_main_tests_secondary_items);

    if (show_menu) {
        menu(xdsl_main_submenup, xdsl_main_tests_secondary_items, '\0');
    } else {
        do_all_menu_items(xdsl_main_submenup);
    }
    
    return (PASSED);
}

/*******************************************************************************
 *
 * Function: xdsl_bootflash_test
 *
 * Description: This function to test SPI boot flash.
 *
 * Input : none
 *
 * Output: PASSED/FAILED
 *
 *******************************************************************************
 */
static int xdsl_bootflash_test (void)
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
    cterr_add_component("Marvell Armada 7040", "BCM63xxx", "SPI Flash");

    /* Segment 5: register and memory dump */
    //cterr_add_reg_dump((PFV)bcm63168_get_xdsl_profile);

    /* Segment 6: Platform Environment initialized here*/
    cterr_add_env_dump((PFV)tsn_display_temp_errormsg);

    /* Segment 7: Top 3 Debugging Steps */
    cterr_add_debug("If NC issue, Do the Ping xDSL utility to verify the RGMII path and "
                    "refer to Ping xDSLutility for more failure analysis.",
                    "If not NC issue, switch console to xDSL SoC and use the "
                    "\"diag_tsn spi_write\" command to write the "
                    "known pattern into scratch pad of NOR flash.",
                    "Run \"diag_tsn spi_read\" to read the data "
                    "from scratch pad of NOR flash. "
                    "Dump all data in scratch pad and "
                    "compare it with the data we wrote.",
                    "Pattern:\n"
                        "\t1. 80000000\n"
                        "\t2. 80000001\n"
                        "\t3. 80000002\n"
                        "\t4. 80000003\n"
                        "\t5. 80000004\n"
                        "\t6. 80000005\n"
                        "\t        .\n"
                        "\t        .\n"
                        "\t        .\n"
                        "\t123. 8000007a\n"
                        "\t124. 8000007b\n"
                        "\t125. 8000007c\n"
                        "\t126. 8000007d\n"
                        "\t127. 8000007e\n"
                        "\t128. 8000007f\n",
                    "If step 3 is good, maybe there is one sector in SPI flash "
                    "corrupt. Replace one SPI flash and redo the test. "
                    "If step 3 fail, check the path between "
                    "the xDSL SoC and SPI flash.");
#endif

    char tx_str[TSN_NC_MAX_STR_SIZE];
    const int maxlen = TSN_DSL_MAX_LENGTH;
    char tname[maxlen];

    if (tsn_gfast_sku == TRUE) {
        sprintf(tname,"%s", "BCM63138 SPI Boot Flash");
    } else {
        sprintf(tname,"%s","BCM63168 SPI Boot Flash");
    }
    
    testname(tname);
    prpass(testpass, "%s, ", tname);

    tsn_nc_init_parms_file();

    sprintf(tx_str, "%d,%d,%d,%d",
            bcm_op_mode, bcm_line_id, bcm_qln_monitor_time, bcm_qln_monitor_freq);

    tsn_nc_dispatch_comm(DIAG_BCM63268_FLASH_TEST, tx_str);

    if (tsn_nc_dispatch_comm_is_ok() != PASSED) {
    	cterr('f', 0, "%s failed.", __FUNCTION__);
        prcomplete(testpass, errcount, (char *)0);
        return (FAILED);
    }

    prcomplete(testpass, errcount, (char *)0);
    return (PASSED);
}

/*******************************************************************************
 *
 * Function   : xdsl_bcm63168_dram_test
 *
 * Description: The function to test DDR3 memory.
 *
 * Inputs     : NONE 
 *             
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
static int xdsl_bcm63168_dram_test (void)
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
    cterr_add_component("Marvell Armada 7040", "BCM63xxx", "DDR RAM");

    /* Segment 5: register and memory dump */
    //cterr_add_reg_dump((PFV)bcm63168_get_xdsl_profile);

    /* Segment 6: Platform Environment initialized here*/
    cterr_add_env_dump((PFV)tsn_display_temp_errormsg);

    /* Segment 7: Top 3 Debugging Steps */
    cterr_add_debug("If, NC issue, please do the Ping xDSL utility to verify the RGMII path and "
                    "refer to Ping xDSL utility for more failure analysis. If not NC issue, switch console"
                    "to xDSL prompt # and use CLI to test the DDR test by type \" diag_tsn dram_test\"",
                    "Check the hardware interface between the xDSL SoC and the DDR3.",
                    "If there is no problem on these hardware interfaces, "
                    "replace one DDR3 and redo the test.");
#endif

    char tx_str[TSN_NC_MAX_STR_SIZE];
    const int maxlen = TSN_DSL_MAX_LENGTH;
    char tname[maxlen];

    if (tsn_gfast_sku == TRUE) {
        sprintf(tname,"%s", "BCM63138 DRAM");
    } else {
        sprintf(tname,"%s","BCM63168 DRAM");
    }
    testname(tname);
    prpass(testpass, "%s, ", tname);

    tsn_nc_init_parms_file();

    sprintf(tx_str, "%d,%d,%d,%d",
            bcm_op_mode, bcm_line_id, bcm_qln_monitor_time, bcm_qln_monitor_freq);

    tsn_nc_dispatch_comm(DIAG_BCM63268_DRAM_TEST, tx_str);

    if (tsn_nc_dispatch_comm_is_ok() != PASSED) {
        cterr('f', 0, "%s failed.", __FUNCTION__);
        prcomplete(testpass, errcount, (char *)0);
       	return (FAILED);
    }

    prcomplete(testpass, errcount, (char *)0);
    return (PASSED);
}

/*******************************************************************************
 *
 * Function: xdsl_interrupt_test
 *
 * Description: This function to test interrupt pin.
 *
 * Input : none
 *
 * Output: PASSED/FAILED
 *
 *******************************************************************************
 */
static int xdsl_interrupt_test (void)
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
    cterr_add_component("Marvell Armada 7040", "BCM63xxx", "FPGA");

    /* Segment 5: register and memory dump */
    //cterr_add_reg_dump((PFV)bcm63168_get_xdsl_profile);

    /* Segment 6: Platform Environment initialized here*/
    cterr_add_env_dump((PFV)tsn_display_temp_errormsg);

    /* Segment 7: Top 3 Debugging Steps */
    cterr_add_debug("Use \"FPGA registers Dump\" utility to check the FPGA Ext." 
                    "Interrupt Pending (register 0x1128), if the interrupt pending"
                    "then measure the DSL GPIO-35 whether it keep trigger the interrupt.",
                    "Check if the FPGA xDSL interrupt pending can be clear, before run"
                    "the interrupt test.",
                    "Check the interrupt path DSL connected to system FPGA.");
#endif

    char tx_str[TSN_NC_MAX_STR_SIZE];
    uint reg_val;
    int ix = 0;
    const int maxlen = TSN_DSL_MAX_LENGTH;
    char tname[maxlen];

    if (tsn_gfast_sku == TRUE) {
        sprintf(tname,"%s", "BCM63138 Interrupt Test");
    } else {
        sprintf(tname,"%s","BCM63168 Interrupt Test");
    }
    testname(tname);
    prpass(testpass, "%s, ", tname);

    sprintf(tx_str, "%d,%d,%d,%d",
            bcm_op_mode, bcm_line_id, bcm_qln_monitor_time, bcm_qln_monitor_freq);

    prpass(testpass, "Clear the DSL interrupt pending, ");
    tsn_nc_init_parms_file();
    /* Disable the DSL interrupt signal, do not trigger interrupt to FPGA
     * before testing. */
    tsn_nc_dispatch_comm(DIAG_BCM63268_INTR_DISABLE, tx_str);
    if (tsn_nc_dispatch_comm_is_ok() != PASSED) {
    	cterr('f', 0, "%s failed.", __FUNCTION__);
        prcomplete(testpass, errcount, (char *)0);
        return (FAILED);
    }
    mdelay(WAIT_FOR_CTRL_DSL_INTR);
    /* Based on new FPGA design we need to write 1 to clear the interrupt pending */
    reg_val = XDSL_INTERRUPT_PENDING; 
    if (fpga_write_32_reg(FPGA_EXTER_INT_PENDING_REG, reg_val) == FAILED) {
        cterr('f', 0, "Failed to write FPGA register.");
        prcomplete(testpass, errcount, (char *)0);
        return (FAILED);
    }

    /* Check the external interrupt pending register to make sure
     * there is no interrupt pending at DSL_FPGA_INT_L pin. */
    for (ix = 0; ix < CHECK_FPGA_DSL_PENDING_TIMES; ix++) {
        if (fpga_read_32_reg(FPGA_EXTER_INT_PENDING_REG, &reg_val) == FAILED) {
    	    cterr('f', 0, "Failed to read FPGA register.");
            prcomplete(testpass, errcount, (char *)0);
            return (FAILED);
        }
        if (!(reg_val & XDSL_INTERRUPT_PENDING)) {
            break;
        }
        mdelay(WAIT_FOR_CTRL_DSL_INTR);
    }

    if (ix == CHECK_FPGA_DSL_PENDING_TIMES) {
	    cterr('f', 0, "xDSL interrupt pending before testing.");
        prcomplete(testpass, errcount, (char *)0);
	    return (FAILED);
    }

    /* Generate the DSL interrupt signal, trigger interrupt to FPGA */
    prpass(testpass, "Generate the DSL interrupt signal to FPGA, ");
    tsn_nc_init_parms_file();
    tsn_nc_dispatch_comm(DIAG_BCM63268_INTR_ENABLE, tx_str);
    if (tsn_nc_dispatch_comm_is_ok() != PASSED) {
    	cterr('f', 0, "%s failed.", __FUNCTION__);
        prcomplete(testpass, errcount, (char *)0);
        return (FAILED);
    }
    mdelay(WAIT_FOR_CTRL_DSL_INTR);

    /* Based on new FPGA design we need to write 1 to clear the interrupt pending */
    reg_val = XDSL_INTERRUPT_PENDING; 
    if (fpga_write_32_reg(FPGA_EXTER_INT_PENDING_REG, reg_val) == FAILED) {
        cterr('f', 0, "Failed to write FPGA register.");
        prcomplete(testpass, errcount, (char *)0);
        return (FAILED);
    }
    prpass(testpass, "Check FPGA DSL interrupt pending register, ");
    /* Check FPGA DSL interrupt pending. */
    if (fpga_read_32_reg(FPGA_EXTER_INT_PENDING_REG, &reg_val) == FAILED) {
    	cterr('f', 0, "Failed to read FPGA register.");
        prcomplete(testpass, errcount, (char *)0);
        return (FAILED);
    }
    if (!(reg_val & XDSL_INTERRUPT_PENDING)) {
    	cterr('f', 0, "xDSL interrupt test failed.");
        prcomplete(testpass, errcount, (char *)0);
    	return (FAILED);
    }

    prpass(testpass, "Stop generating the DSL interrupt signal to FPGA, ");
    tsn_nc_init_parms_file();
    tsn_nc_dispatch_comm(DIAG_BCM63268_INTR_DISABLE, tx_str);
    if (tsn_nc_dispatch_comm_is_ok() != PASSED) {
    	cterr('f', 0, "%s failed.", __FUNCTION__);
        prcomplete(testpass, errcount, (char *)0);
        return (FAILED);
    }
    mdelay(WAIT_FOR_CTRL_DSL_INTR);

    /* Based on new FPGA design we need to write 1 to clear the interrupt pending */
    reg_val = XDSL_INTERRUPT_PENDING; 
    if (fpga_write_32_reg(FPGA_EXTER_INT_PENDING_REG, reg_val) == FAILED) {
        cterr('f', 0, "Failed to write FPGA register.");
        prcomplete(testpass, errcount, (char *)0);
        return (FAILED);
    }
    /* Check FPGA DSL interrupt pending. */
    for (ix = 0; ix < CHECK_FPGA_DSL_PENDING_TIMES; ix++) {
        if (fpga_read_32_reg(FPGA_EXTER_INT_PENDING_REG, &reg_val) == FAILED) {
    	    cterr('f', 0, "Failed to read FPGA register.");
            prcomplete(testpass, errcount, (char *)0);
            return (FAILED);
        }
        if (!(reg_val & XDSL_INTERRUPT_PENDING)) {
            break;
        }
        mdelay(WAIT_FOR_CTRL_DSL_INTR);
    }

    if (ix == CHECK_FPGA_DSL_PENDING_TIMES) {
	    cterr('f', 0, "xDSL interrupt pending after testing.");
        prcomplete(testpass, errcount, (char *)0);
	    return (FAILED);
    }

    prpass(testpass, "DSL interrupt test passed, ");
    prcomplete(testpass, errcount, (char *)0);
    return (PASSED);
}

/*******************************************************************************
 *
 * Function   : xdsl_do_showtime
 *
 * Description: The function to train w/ DSLAM.
 *
 * Inputs     : NONE 
 *             
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
static int xdsl_do_showtime (void)
{
    int retval = FAILED;

    if ((retval = bcm63168_showtime_test()) != PASSED) {
        cterr('f', 0, "%s failed.", __FUNCTION__);
        return (retval);
    }

    printf("%s passed.\n", __FUNCTION__);
    return (retval);
}

/*******************************************************************************
 *
 * Function   : xdsl_do_set_tones
 *
 * Description: The function to configure DSL parameters.
 *
 * Inputs     : NONE 
 *             
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
static int xdsl_do_set_tone (void)
{
    int retval = FAILED;

    if ((retval = bcm63168_set_tone()) != PASSED) {
        cterr('f', 0, "%s failed.", __FUNCTION__);
        return (retval);
    }
    
    printf("%s passed.\n", __FUNCTION__);
    return (retval);
}

/*******************************************************************************
 *
 * Function   : xdsl_do_send_all_tone
 *
 * Description: The function to perform set all tone test.
 *
 * Inputs     : NONE 
 *             
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
static int xdsl_do_send_all_tone (void)
{
    int retval = FAILED;

    if ((retval = bcm63168_send_all_tone()) != PASSED) {
        cterr('f', 0, "%s failed.", __FUNCTION__);
        return (retval);
    }
    
    printf("%s passed.\n", __FUNCTION__);
    return (retval);
}

/*******************************************************************************
 *
 * Function   : xdsl_do_idle_listen
 *
 * Description: The function to perform do idle listen test.
 *
 * Inputs     : NONE 
 *             
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
static int xdsl_do_idle_listen (void)
{
    int retval = FAILED;

    if ((retval = bcm63168_idle_listen()) != PASSED) {
        cterr('f', 0, "%s failed.", __FUNCTION__);
        return (retval);
    }
    
    printf("%s passed.\n", __FUNCTION__);
    return (retval);
}

/*******************************************************************************
 *
 * Function   : xdsl_select_test_options
 *
 * Description: The function to configure DSL parameters.
 *
 * Inputs     : NONE 
 *             
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
static int xdsl_select_test_options (void)
{
    int retval = FAILED;

    if ((retval = bcm63168_vdsl_test_option_select()) != PASSED) {
        cterr('f', 0, "%s failed.", __FUNCTION__);
        return (retval);
    }
    
    return (retval);
}

/*******************************************************************************
 *
 * Function   : xdsl_console_switch
 *
 * Description: A utility to Console Switch to xDSL BRCM console.
 *
 * Inputs     : NONE 
 *             
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
static int xdsl_console_switch (void)
{
    struct uart_parm picocom;
    picocom.tty_dev = TSN_DSL_UART_DEV_STR;
    picocom.baudrate = 9600;    
    picocom.databit = 8;
    picocom.parity = "1";
    picocom.flow = "n";

    if (tsn_console_switch(&picocom) != PASSED) {
        return (FAILED);
    }

    return(PASSED);
}

/*******************************************************************************
 *
 * Function    : xdsl_bcm63168_reset
 * Description : This utility performs a reset to the BCM63168 on the xDSL module
 * Input       : NONE
 * Output      : PASSED/FAILED
 *
 *******************************************************************************
 */
int xdsl_bcm63168_reset (void)
{
    /* DSL SKU does the dsl module initialization reset sequence */
    if (fpga_reset_32_api(FPGA_EXTER_DEV_RST_REG, EXT_DSL_CHIP_RESET, TRUE,
                          WAITTIME_20_MS)
        == FAILED) {
    	return (FAILED);
    }
    /* DSL SKU un-reset the dsl module */
    if (fpga_reset_32_api(FPGA_EXTER_DEV_RST_REG, EXT_DSL_CHIP_RESET, FALSE,
                          WAITTIME_20_MS)
        == FAILED) {
        return (FAILED);
    }

    return (PASSED);
}

/*******************************************************************************
 *
 * Function   : xdsl_init_bcm63168
 *
 * Description: The functions to init bcm63168.
 *
 * Inputs     : NONE 
 *             
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
static int xdsl_init_bcm63168 (void)
{
    int retval = FAILED;
    
    if ((retval = bcm63168_initialize()) != PASSED) {
        printf("%s fail\n", __FUNCTION__);
        return (retval);
    }
    
    return (retval);
}

/*******************************************************************************
 *
 * Function   : xdsl_config_bcm63168
 *
 * Description: The function to configure DSL profile
 *
 * Inputs     : NONE 
 *             
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
static int xdsl_config_bcm63168 (void)
{
    int retval = FAILED;
    printf("Fix later.\n");    
    if ((retval = bcm63168_configure()) != PASSED) {
        printf("%s fail\n", __FUNCTION__);
        return (retval);
    }
    
    return (retval);
}

/*******************************************************************************
 *
 * Function   : xdsl_get_bcm63168_config
 *
 * Description: The functions to get DSL profile.
 *
 * Inputs     : NONE 
 *             
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
static int xdsl_get_bcm63168_config (void)
{
    int retval = FAILED;
    
    if ((retval = bcm63168_get_configure()) != PASSED) {
        printf("%s fail\n", __FUNCTION__);
        return (retval);
    }
    
    return (retval);
}

/*******************************************************************************
 *
 * Function   : xdsl_get_bcm63168_version
 *
 * Description: The functions to get DSL driver version.
 *
 * Inputs     : NONE 
 *             
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
static int xdsl_get_bcm63168_version (void)
{
    int retval = FAILED;
    
    if ((retval = bcm63168_get_version()) != PASSED) {
        printf("%s fail\n", __FUNCTION__);
        return (retval);
    }
    
    return (retval);
}

/*******************************************************************************
 *
 * Function   : xdsl_connection_start
 *
 * Description: The functions to start to train with DSLAM.
 *
 * Inputs     : NONE 
 *             
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
static int xdsl_connection_start (void)
{
    int retval = FAILED;
    
    if ((retval = bcm63168_do_showtime()) != PASSED) {
        printf("%s fail\n", __FUNCTION__);
        return (retval);
    }
    
    return (retval);
}

/*******************************************************************************
 *
 * Function   : xdsl_connection_stop
 *
 * Description: The functions to stop DSL from showtime.
 *
 * Inputs     : NONE 
 *             
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
static int xdsl_connection_stop (void)
{
    int retval = FAILED;

    if ((retval = bcm63168_connection_stop()) != PASSED) {
        printf("%s fail\n", __FUNCTION__);
        return (retval);
    }
    
    return (retval);
}

/*******************************************************************************
 *
 * Function   : xdsl_get_xdsl_mib_info
 *
 * Description: The functions to get DSL inforamtion.
 *
 * Inputs     : NONE 
 *             
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
static int xdsl_get_xdsl_mib_info (void)
{
    int retval = FAILED;
    
    if ((retval = bcm63168_get_adslmib_info()) != PASSED) {
        printf("%s fail\n", __FUNCTION__);
        return (retval);
    }
    
    return (retval);
}

/*******************************************************************************
 *
 * Function   : xdsl_get_xtm_bonding_info
 *
 * Description: The functions to get DSL bonding status.
 *
 * Inputs     : NONE 
 *             
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
static int xdsl_get_xtm_bonding_info (void)
{
    int retval = FAILED;
    printf("TSN doesn't have bonding.\n");    
    return (retval);
    if ((retval = bcm63168_get_xtm_bonding_info()) != PASSED) {
        printf("%s fail\n", __FUNCTION__);
        return (retval);
    }
    
    return (retval);
}

/*******************************************************************************
 *
 * Function   : xdsl_get_xdsl_info
 *
 * Description: The functions to get DSL inforamtion.
 *
 * Inputs     : NONE 
 *             
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
static int xdsl_get_xdsl_info (void)
{
    int retval = FAILED;
    
    if ((retval = bcm63168_get_xdsl_info()) != PASSED) {
        printf("%s fail\n", __FUNCTION__);
        return (retval);
    }
    
    return (retval);
}

/*******************************************************************************
 *
 * Function   : xdsl_get_connection_info
 *
 * Description: The function to get connection information.
 *
 * Inputs     : NONE 
 *             
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
static int xdsl_get_connection_info (void)
{
    int retval = FAILED;
    
    if ((retval = bcm63168_get_conn_info()) != PASSED) {
        printf("%s fail\n", __FUNCTION__);
        return (retval);
    }
    
    return (retval);
}

/*******************************************************************************
 *
 * Function   : xdsl_bcm63168_led_test
 *
 * Description: The function to perform Link LEDs test
 *
 * Inputs     : NONE 
 *             
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
static int xdsl_bcm63168_led_test (void)
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
    cterr_add_component("Marvell Armada 7040", "BCM63xxx", "LED");

    /* Segment 5: register and memory dump */
    //cterr_add_reg_dump((PFV)bcm63168_get_xdsl_profile);

    /* Segment 6: Platform Environment initialized here*/
    cterr_add_env_dump((PFV)tsn_display_temp_errormsg);

    /* Segment 7: Top 3 Debugging Steps */
    cterr_add_debug("Check the LED GPIO to see if its' implementation "
                    "is identical to BCM63xxx specification."
                    "and the failed I2C devices.",
                    "If step a is OK, check the LED interface "
                    "between BCM.");
#endif

    int retval = FAILED;
    const int maxlen = TSN_DSL_MAX_LENGTH;
    char tname[maxlen];

    if (tsn_gfast_sku == TRUE) {
        sprintf(tname,"%s", "xDSL/G.Fast LED");
    } else {
        sprintf(tname,"%s","xDSL LED");
    }
    testname(tname);
    prpass(testpass, "%s, ", tname);

    /* Check xdsl is ready */
    if (pri_intf_rdy_chk() != PASSED) {
        printf ("\nxdsl module is not ready."
                "Please boot up xdsl module first.\n");
        return (PASSED);
    }
    
    if ((retval = bcm63168_led_test()) != PASSED) {
        cterr('f', 0, "%s failed.", __FUNCTION__);
        return (retval);
    }
    
    prcomplete(testpass, errcount, (char *)0);
    return (retval);
}

/*******************************************************************************
 *
 * Function   : xdsl_bcm63168_show_profile
 *
 * Description: The function to show DSL profile
 *
 * Inputs     : NONE 
 *             
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
static int xdsl_bcm63168_show_profile (void)
{
    int retval = FAILED;
    
    if ((retval = bcm63168_show_profile()) != PASSED) {
        printf("%s fail\n", __FUNCTION__);
        return (retval);
    }
    
    return (retval);
}

/*******************************************************************************
 *
 * Function   : xdsl_ping_util
 *
 * Description: The function is going to ping DSL module.
 *
 * Inputs     : NONE
 *
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
static int xdsl_ping_util (void)
{
    char tx_str[TSN_NC_MAX_STR_SIZE];

    tsn_nc_init_parms_file();

    sprintf(tx_str, "%d,%d,%d,%d",
            bcm_op_mode, bcm_line_id, bcm_qln_monitor_time, bcm_qln_monitor_freq);

    tsn_nc_dispatch_comm(DIAG_PING_BCM63268, tx_str);

    if (tsn_nc_dispatch_comm_is_ok() != PASSED) {
        printf("%s fail\n", __FUNCTION__);
        return (FAILED);
    }

    return (PASSED);
}

/*******************************************************************************
 *
 * Function   : pri_intf_rdy_chk
 *
 * Description: Check if DSL_FPGA_EXP_PRI_RDY is asserted.
 *
 * Inputs     : None
 *
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
static int pri_intf_rdy_chk (void)
{
    unsigned int data;

    if (fpga_read_32_reg(FPGA_DSL_STATUS_CTL_REG, &data) == FAILED) {
    	cterr('f', 0, "Failed to read CPLD register.");
    	return (FAILED);
    }

    /* Check EXP_PRI_RDY is high or low */
    if (data & DSL_FPGA_EXP_PRI_RDY) {
        return (PASSED);
    } else {
        return (FAILED);
    }
}

/*******************************************************************************
 *
 * Function   : xdsl_check_sku_type
 *
 * Description: Wrap function to check if SKU ID in xdsl module and in ACT2
 *              are identical.
 *
 * Inputs     : NONE 
 *             
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
static int xdsl_check_sku_type (void)
{
    if ((bcm63168_check_sku_type()) != PASSED) {
        printf("%s failed.", __FUNCTION__);
        return (FAILED);
    }

    return (PASSED);
}

/*******************************************************************************
 *
 * Function: xdsl_bcm63168_show_spi_flash_reg
 *
 * Description: This function displays xdsl SPI flash registers.
 *
 * Input : None
 *
 * Output: PASSED/FAILED.
 *
 *******************************************************************************
 */
static int xdsl_bcm63168_show_spi_flash_reg (void)
{
    int retval = FAILED;

    if ((retval = bcm63168_show_spi_flash_reg()) != PASSED) {
        printf("%s fail\n", __FUNCTION__);
        return (retval);
    }

    return (retval);
}

/*******************************************************************************
 *
 * Function: xdsl_confirm_cfe_parms_set
 *
 * Description: This function set cfe parameters.
 *
 * Input : boolean os_param = diag or ios
 *
 * Output: PASSED/FAILED
 *
 *******************************************************************************
 */
static int xdsl_confirm_cfe_parms_set (boolean os_param)
{
    int ix;
    const int maxlen = TSN_DSL_MAX_LENGTH;
    char tty[maxlen];
    char linux_file_str[maxlen];
    char linux_ip_str[maxlen];
    int result = PASSED;
    int tty_desc;
    char *dsl_img_filename;

    if (os_param == IOS_CFE_PARAM) { 
        prpass(testpass, "Confirming CFE ios Parameters,");
    } else {
        prpass(testpass, "Confirming CFE diag Parameters,");
    }
    printf("\n");

    printf("Start CFE UART...\n");
    fflush(stdout);
    fflush(stderr);

    snprintf(tty, maxlen-1, TSN_DSL_UART_DEV_STR);
    tty_desc = open(tty, O_RDWR | O_NOCTTY);
    if (tty_desc < 0) {
        perror("open tty failed");
        fflush(stdout);
        return (FAILED);
    }

    /* Step 0 : Catch "Press any key" */
    result = tsn_uart_rx_polling(tty_desc, TSN_DSL_PRESS_KEY_STRING,
    		                      DSL_CFE_PRESS_KEY_TIMEOUT);
    if (result == TRUE) {
        printf("Found : %s\n", TSN_DSL_PRESS_KEY_STRING);
        fflush(stdout);
    } else {
        printf("[%s] Not Found.\n", TSN_DSL_PRESS_KEY_STRING);
        fflush(stdout);
        goto exit_cfe_parms_set_failed;
    } 

    /* Step 1 : Find "CFE>" */
    for (ix = 0; ix < DSL_CFE_RETRY_TIMES; ix++) {
        if (tsn_uart_tx(tty_desc, TSN_DSL_CR_STRING) == FAILED) {
            goto exit_cfe_parms_set_failed;
        }
        result = tsn_uart_rx_polling(tty_desc, TSN_DSL_CFE_STRING,
        		                      DSL_CFE_TIMEOUT);
        if (result == TRUE) {
            printf("Found : %s\n", TSN_DSL_CFE_STRING);
            fflush(stdout);
            break;
        }
    }

    if (ix == DSL_CFE_RETRY_TIMES) {
        printf("[%s] Not Found.\n", TSN_DSL_CFE_STRING);
        fflush(stdout);
        goto exit_cfe_parms_set_failed;
    }
    msleep(WAIT_DSL_CFE_PROMPT);

    /* Step 2 : Check Set board parameters   */
    if (tsn_uart_tx(tty_desc, TSN_DSL_PRINT_CFE_PARMS_STRING) == FAILED) {
        goto exit_cfe_parms_set_failed;
    }    
    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("Send bdpm [%s]\n", TSN_DSL_PRINT_CFE_PARMS_STRING);
        fflush(stdout);
    }

    if (tsn_uart_tx(tty_desc, TSN_DSL_CR_STRING) == FAILED) {
        goto exit_cfe_parms_set_failed;
    }
    if (os_param == IOS_CFE_PARAM) { 
        sprintf(linux_ip_str, "%s.%d:ffffff00", TSN_IOS_DSL_SUBNET_STR,
                TSN_IOS_MODULE_IP_ADDR);
    } else {    
        sprintf(linux_ip_str, "%s.%d", TSN_DIAG_DSL_SUBNET_STR,
                TSN_DIAG_MODULE_IP_ADDR);
    }
    /* Step 3: Check Board IP */
    for (ix = 0; ix < DSL_CFE_RETRY_TIMES; ix++) {
        result = tsn_uart_rx_polling(tty_desc, linux_ip_str,
        		                      DSL_CFE_TIMEOUT);
        if (result == TRUE) {
            printf("Found : %s\n", linux_ip_str);
            fflush(stdout);
            break;
        }
    }

    if (ix == DSL_CFE_RETRY_TIMES) {
        printf("[%s] Not Found.\n", linux_ip_str);
        fflush(stdout);
        goto exit_cfe_parms_set_failed;
    }

    if (os_param == IOS_CFE_PARAM) { 
        sprintf(linux_ip_str, "%s.%d", TSN_IOS_DSL_SUBNET_STR,
                TSN_IOS_HOST_IP_ADDR);
    } else {
        sprintf(linux_ip_str, "%s.%d", TSN_DIAG_DSL_SUBNET_STR,
                TSN_DIAG_HOST_IP_ADDR);
    }
    
    /* Step 4: Check Host IP */
    for (ix = 0; ix < DSL_CFE_RETRY_TIMES; ix++) {
        result = tsn_uart_rx_polling(tty_desc, linux_ip_str,
        		                      DSL_CFE_TIMEOUT);
        if (result == TRUE) {
            printf("Found : %s\n", linux_ip_str);
            fflush(stdout);
            break;
        }
    }

    if (ix == DSL_CFE_RETRY_TIMES) {
        printf("[%s] Not Found.\n", linux_ip_str);
        fflush(stdout);
        goto exit_cfe_parms_set_failed;
    }

    if (os_param == IOS_CFE_PARAM) { 
        sprintf(linux_ip_str, "%s.%d", TSN_IOS_DSL_SUBNET_STR,
                TSN_IOS_GATEWAY_IP_ADDR);
    } else {
        sprintf(linux_ip_str, "%s.%d", TSN_DIAG_DSL_SUBNET_STR,
                TSN_DIAG_GATEWAY_IP_ADDR);
    }
    /* Step 5: Check Gateway IP */
    for (ix = 0; ix < DSL_CFE_RETRY_TIMES; ix++) {
        result = tsn_uart_rx_polling(tty_desc, linux_ip_str,
        		                      DSL_CFE_TIMEOUT);
        if (result == TRUE) {
            printf("Found : %s\n", linux_ip_str);
            fflush(stdout);
            break;
        }
    }

    if (ix == DSL_CFE_RETRY_TIMES) {
        printf("[%s] Not Found.\n", linux_ip_str);
        fflush(stdout);
        goto exit_cfe_parms_set_failed;
    }
    

    /* Step 6: Check default linux file name */
    if (os_param == IOS_CFE_PARAM) { 
        dsl_img_filename = getenv(IOS_IMAGE_NAME);
    } else {
        dsl_img_filename = getenv(DSL_IMAGE_NAME);
    }

    if (dsl_img_filename == NULL || !strcmp("none", dsl_img_filename)) {
        if (os_param == IOS_CFE_PARAM) { 
            printf("%s: Env variable %s is not set\n", __func__, IOS_IMAGE_NAME);
        } else {
            printf("%s: Env variable %s is not set\n", __func__, DSL_IMAGE_NAME);
        } 
        goto exit_cfe_parms_set_failed;
    } else {
        if (os_param == IOS_CFE_PARAM) { 
            printf("IOS Image Filename: %s\n", dsl_img_filename);
            sprintf(linux_file_str, "%s%s",
                TSN_IOS_HOST_FIRMWARE_FOLDER_STRING, dsl_img_filename);
        } else {
            printf("DIAG Image Filename: %s\n", dsl_img_filename);
            sprintf(linux_file_str, "%s", dsl_img_filename);
        }
    }
    
    for (ix = 0; ix < DSL_CFE_RETRY_TIMES; ix++) {
        result = tsn_uart_rx_polling(tty_desc, linux_file_str,
        		                      DSL_CFE_TIMEOUT);
        if (result == TRUE) {
            printf("Found : %s\n", linux_file_str);
            fflush(stdout);
            break;
        }
    }

    if (ix == DSL_CFE_RETRY_TIMES) {
        printf("[%s] Not Found.\n", linux_file_str);
        fflush(stdout);
        goto exit_cfe_parms_set_failed;
    }

    /* Step 8 : Find "command status" */
    for (ix = 0; ix < DSL_CFE_RETRY_TIMES; ix++) {
        if (tsn_uart_tx(tty_desc, TSN_DSL_CR_STRING) == FAILED) {
            goto exit_cfe_parms_set_failed;
        }
        result = tsn_uart_rx_polling(tty_desc, TSN_DSL_COMMAND_STATUS_STRING,
        		                      DSL_CFE_COMMAND_STATUS_TIMEOUT);
        if (result == TRUE) {
            printf("Found : %s\n", TSN_DSL_COMMAND_STATUS_STRING);
            fflush(stdout);
            break;
        }
        msleep(WAIT_DSL_CFE_COMMAND_STATUS);
    }

    if (ix == DSL_CFE_RETRY_TIMES) {
        printf("[%s] Not Found.\n", TSN_DSL_COMMAND_STATUS_STRING);
        fflush(stdout);
        goto exit_cfe_parms_set_failed;
    }
    msleep(WAIT_DSL_CFE_COMMAND_STATUS);

    /* Step 9 : Find "CFE>" */
    for (ix = 0; ix < DSL_CFE_RETRY_TIMES; ix++) {
        if (tsn_uart_tx(tty_desc, TSN_DSL_CR_STRING) == FAILED) {
            goto exit_cfe_parms_set_failed;
        }
        result = tsn_uart_rx_polling(tty_desc, TSN_DSL_CFE_STRING,
        		                      DSL_CFE_TIMEOUT);
        if (result == TRUE) {
            printf("Found : %s\n", TSN_DSL_CFE_STRING);
            fflush(stdout);
            break;
        } 
    }

    if (ix == DSL_CFE_RETRY_TIMES) {
        printf("[%s] Not Found.\n", TSN_DSL_CFE_STRING);
        fflush(stdout);
        goto exit_cfe_parms_set_failed;
    }

    msleep(WAIT_FOR_FLASH_WRITE);

    close(tty_desc);
    return (PASSED);
    
exit_cfe_parms_set_failed:
    close(tty_desc);
    return (FAILED);
}


/*******************************************************************************
 *
 * Function: restore_cfe_param 
 *
 * Description: This function restore cfe ios or diag param.
 *
 * Input : os_param TRUE = IOS
 *         os_param FALSE = DIAG 
 *
 * Output: PASSED/FAILED.
 *
 *******************************************************************************
 */
static int restore_cfe_param (boolean os_param)
{
    int retval = FAILED;
    
    /* Step1: Reset */
    xdsl_bcm63168_reset();
    
    if (os_param == IOS_CFE_PARAM) {
        /* Step2: Set CFE IPs and Filename */ 
        if (xdsl_cfe_parms_set(IOS_CFE_PARAM) != PASSED) {
            printf("%s fail\n", __FUNCTION__);
            return (retval);
        }

        /* Step3: Reset */
        xdsl_bcm63168_reset();
        if (xdsl_confirm_cfe_parms_set(IOS_CFE_PARAM) == PASSED) {
            printf("\nRestore CFE IOS Param Passed\n");
            retval = PASSED;
        } else  {
            printf("\nRestore CFE IOS Param Failed\n");
        }
    } else {
        /* Step2: Set CFE IPs and Filename */ 
        if (xdsl_cfe_parms_set(DIAG_CFE_PARAM) != PASSED) {
            printf("%s fail\n", __FUNCTION__);
            return (retval);
        }

        /* Step3: Reset */
        xdsl_bcm63168_reset();
        if (xdsl_confirm_cfe_parms_set(DIAG_CFE_PARAM) == PASSED) {
            printf("\nRestore CFE DIAG Param Passed\n");
            retval = PASSED;
        } else  {
            printf("\nRestore CFE DIAG Param Failed\n");
        }
    }

    return (retval);
}


/*******************************************************************************
 *
 * Function: xdsl_bcm63138_set_gfast_relay
 *
 * Description: This function sets relay to G.Fast or non G.Fast
 *
 * Input : None
 *
 * Output: PASSED/FAILED.
 *
 *******************************************************************************
 */
static int xdsl_bcm63138_set_gfast_relay (void)
{
    int retval = FAILED;

    if ((retval = bcm63138_set_gfast_relay()) != PASSED) {
        printf("%s fail\n", __FUNCTION__);
        return (retval);
    }

    return (retval);
}

/*******************************************************************************
 *
 * Function: xdsl_bcm63168_en_spi_flash_reg
 *
 * Description: This function enable xdsl SPI flash write protect registers.
 *
 * Input : None
 *
 * Output: PASSED/FAILED.
 *
 *******************************************************************************
 */
static int xdsl_bcm63168_en_spi_flash_reg (void)
{
    int retval = FAILED;

    if ((retval =bcm63168_en_wp_spi_flash_reg()) != PASSED) {
        printf("%s fail\n", __FUNCTION__);
        return (retval);
    }

    return (retval);
}

/*******************************************************************************
 *
 * Function: xdsl_bcm63168_dis_spi_flash_reg
 *
 * Description: This function disable xdsl SPI flash write protect registers.
 *
 * Input : None
 *
 * Output: PASSED/FAILED.
 *
 *******************************************************************************
 */
static int xdsl_bcm63168_dis_spi_flash_reg (void)
{
    int retval = FAILED;

    if ((retval =bcm63168_dis_wp_spi_flash_reg()) != PASSED) {
        printf("%s fail\n", __FUNCTION__);
        return (retval);
    }

    return (retval);
}

/*******************************************************************************
 *
 * Function: has_gfast_support 
 *
 * Description: This function to show existance gfast supported. 
 *
 * Input : None
 *
 * Output: PASSED/FAILED.
 *
 *******************************************************************************
 */
static boolean has_gfast_support (void)
{
    return (tsn_gfast_sku);
}

/******************************************************************************* *
 * Function   : xdsl_bcm63168_led_utils
 *
 * Description: The function to perform Link LEDs utils 
 *
 * Inputs     : option for off/carrier detect on /data on 
 *             
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
static int xdsl_bcm63168_led_utils (int opt)
{
    int retval = FAILED;
    const int maxlen = TSN_DSL_MAX_LENGTH;
    char tname[maxlen];

    if (tsn_gfast_sku == TRUE) {
        sprintf(tname,"%s", "xDSL/G.Fast LED Utility");
    } else {
        sprintf(tname,"%s","xDSL LED Utility");
    }
    testname(tname);
    prpass(testpass, "%s, ", tname);

    /* Check xdsl is ready */
    if (pri_intf_rdy_chk() != PASSED) {
        printf ("\nxdsl module is not ready."
                "Please boot up xdsl module first.\n");
        return (PASSED);
    }
    
    if ((retval = bcm63168_led_utils(opt)) != PASSED) {
        printf("%s led utils failed.\n", __FUNCTION__);
        return (retval);
    }
    
    prcomplete(testpass, errcount, (char *)0);
    return (retval);
}

/******************************************************************************* *
 * Function   : wrap_bcm63168_led_utils
 *
 * Description: The wrap function to perform Link LEDs utils 
 *
 * Inputs     : option for off/carrier detect on /data on 
 *             
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
int wrap_bcm63168_led_utils (int opt)
{
    return (xdsl_bcm63168_led_utils(opt));
}

/*-------------------------------------------------
$Log: dsl_tests.c,v $
Revision 1.5  2018/05/15 09:37:32  steja
CSCvj38863: Enhanced LED single test utility

Revision 1.4  2018/01/23 11:38:18  steja
Merge tsn-gfast-branch4 code to maintrunk for support TSN-G.Fast (CSCvh40981)

Revision 1.3.14.3  2018/01/12 02:56:49  steja
Update code based on code review comment

Revision 1.3.14.2  2018/01/11 08:07:56  steja
Fix compile issue

Revision 1.3.14.1  2018/01/11 08:02:45  steja
gfast branch4 sync with maintrunk

Revision 1.3.8.4  2018/01/02 13:12:24  steja
Update code base DFS review comments

Revision 1.3.8.3  2017/12/19 06:48:55  shjung
Updated the SKUs for new PID

Revision 1.3.8.2  2017/12/19 00:42:58  steja
TSN not suport bonding.

Revision 1.3.8.1  2017/10/20 11:42:40  steja
Sync Gfast  with the latest main trunk

Revision 1.3  2017/08/25 10:03:57  steja
1.Add Utility to restore back CFE IOS parameter for DF site(CSCvf70937)
2.Add Utility for SPI Write protect

Revision 1.2  2017/08/02 14:21:45  steja
Support TSN-H/M platform code

Revision 1.1.8.2  2017/07/29 03:41:02  steja
tsn-branch5 synced with Maintrunk repositories

Revision 1.1.6.3  2017/07/25 08:31:55  steja
1. Remove unused code.
2. Verified before check-in

Revision 1.1.6.2  2017/07/20 13:38:04  steja
tsn-branch4 merge with maintrunk

Revision 1.1.4.7.2.4  2017/07/18 06:10:36  steja
Code cleanup

Revision 1.1.4.7.2.3  2017/05/25 02:14:48  steja
Fix the DSL baudrate to 9600

Revision 1.1.4.7.2.2.2.11  2017/09/15 03:17:44  shjung
Change boot up method with single combined image as well as h option in CFE.

Revision 1.1.4.7.2.2.2.10  2017/09/14 03:26:30  steja
1. Support Utility for CFE SPI write protect.
2. Fix CFE restore verification bug.

Revision 1.1.4.7.2.2.2.9  2017/09/04 09:50:20  shjung
Fix restore back CFE IOS parameter utility for G.Fast

Revision 1.1.4.7.2.2.2.8  2017/09/01 11:43:15  shjung
Add Utility to restore back CFE IOS parameter for DF site

Revision 1.1.4.7.2.2.2.7  2017/07/04 10:45:25  steja
Fix selected showtime option wrong annex mode

Revision 1.1.4.7.2.2.2.6  2017/05/19 08:10:21  shjung
Modified board id configuration to fit Cisco CFE for first time bootup

Revision 1.1.4.7.2.2.2.5  2017/05/19 06:12:27  shjung
Modified board id configuration to fit Cisco CFE at first time bootup

Revision 1.1.4.7.2.2.2.4  2017/05/10 08:37:18  shjung
Configure board id at first time boot up DSL chip

Revision 1.1.4.7.2.2.2.3  2017/05/03 02:07:11  steja
Add Relay pin switch utilities

Revision 1.1.4.7.2.2.2.2  2017/04/30 08:30:02  steja
1. Fix Sku type mismatch
2. Add VDSL35b support

Revision 1.1.4.7.2.2.2.1  2017/04/28 15:16:43  steja
Add Support GFast DSL firmware boot up


Revision 1.1.4.7.2.2  2017/04/14 00:52:47  steja
Add define for GFAST

Revision 1.1.4.7.2.1  2017/03/21 14:22:11  steja
Based on new FPGA register, update interrupt test

Revision 1.1.4.7  2016/11/15 13:19:09  petteng
Add enhanced error message

Revision 1.1.4.6  2016/11/09 06:49:43  petteng
Add enhanced error message

Revision 1.1.4.5  2016/11/01 07:29:19  petteng
Add enhanced error message

Revision 1.1.4.4  2016/10/07 13:07:55  steja
1. Add Check xDSL sku type
2. Support Annex B

Revision 1.1.4.3  2016/10/04 06:39:08  petteng
Add enhanced error message

Revision 1.1.4.2  2016/06/30 06:22:48  steja
tsn-branch2 sync with main trunk


*/
