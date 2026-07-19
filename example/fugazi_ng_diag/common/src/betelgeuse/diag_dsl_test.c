/* $Id: diag_dsl_test.c,v 1.3 2019/05/21 07:44:19 wilbhuan Exp $ 
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/betelgeuse/diag_dsl_test.c,v $
 *------------------------------------------------------------------
 * 
 * diag_dsl_test.c
 *
 * Copyright (c) 2018 - 2019 by Cisco Systems, Inc.
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
#include "common_utils.h"
#include "queryflags.h"
#include "linux_ntwk.h" /* tftp_get */
#include "diag_moka_fpga_lib.h"
#include "diag_uart_lib.h"
#include "diag_enhance_err_msg_lib.h"
#include "platform_cookie.h"
#include "diag_temp_sensor_util.h"
#include "diag_dsl_lib.h"
#include "diag_dsl_test.h"
#include "diag_dsl_util.h"


/*******************************************************************************
 *                             Function Prototypes                             *
 *******************************************************************************
 */
static int diag_xdsl_util(int);
static int xdsl_param_test(int);
static int diag_xdsl_main_tests(int);
static int diag_xdsl_bootup(void);
static int diag_xdsl_bootflash_test (void);
static int diag_xdsl_interrupt_test(void);
static int diag_xdsl_bcm63168_dram_test (void);
static int diag_xdsl_do_showtime (void);
static int diag_xdsl_do_set_tone (void);
static int diag_xdsl_do_send_all_tone (void);
static int diag_xdsl_do_idle_listen (void);
static int diag_xdsl_select_test_options (void);

unsigned int plat_turbo_sku = 0xFFFF;
/*******************************************************************************
 *                              Global Variables                               *
 *******************************************************************************
 */
extern uint bcm_op_mode;
extern uint bcm_line_id;
extern uint bcm_qln_monitor_time;
extern uint bcm_qln_monitor_freq;

extern int do_all_menu_items(struct menuinfo *);
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
    {"DRAM Test",      (PFT)diag_xdsl_bcm63168_dram_test,   0, MM_3,
     (type_t(*)())0,         0,         (type_t(*)())0,              0},
    {"NOR Flash Test", (PFT)diag_xdsl_bootflash_test,       0, MM_3,
     (type_t(*)())0,         0,         (type_t(*)())0,              0},
    {"Interrupt Test", (PFT)diag_xdsl_interrupt_test,       0, MM_3,
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
static submenu_xtable_t diag_xdsl_util_submenu_table[] = {
    {"xDSL Reset",          (PFT)xdsl_util_bcm63168_reset, 0, 0,
      (type_t(*)())0, 0,   (type_t(*)())0,          0},
    {"Initialize xDSL",         (PFT)xdsl_util_init_bcm63168, 0, 0,
      (type_t(*)())0, 0,   (type_t(*)())0,          0},
    {"Configure xDSL",          (PFT)xdsl_util_config_bcm63168, 0, 0,
      (type_t(*)())0, 0,   (type_t(*)())0,          0},
    {"Get xDSL configuration",  (PFT)xdsl_util_get_bcm63168_config, 0, 0,
      (type_t(*)())0, 0,   (type_t(*)())0,          0},
    {"Get xDSL version",        (PFT)xdsl_util_get_bcm63168_version, 0, 0,
      (type_t(*)())0, 0,   (type_t(*)())0,          0},
    {"xDSL Connection Start",            (PFT)xdsl_util_connection_start, 0, 0,
      (type_t(*)())0, 0,   (type_t(*)())0,          0},
    {"xDSL Connection Stop",             (PFT)xdsl_util_connection_stop, 0, 0,
      (type_t(*)())0, 0,   (type_t(*)())0,          0},
    {"Get xDSL Mib Info",           (PFT)xdsl_util_get_xdsl_mib_info, 0, 0,
      (type_t(*)())0, 0,   (type_t(*)())0,          0},
    {"Get xDSL Bonding Info",       (PFT)xdsl_util_get_xtm_bonding_info, 0, 0,
      (type_t(*)())0, 0,   (type_t(*)())0,          0},
    {"Get xDSL Info",               (PFT)xdsl_util_get_xdsl_info, 0, 0,
      (type_t(*)())0, 0,   (type_t(*)())0,          0},
    {"Get xDSL Connection Info",         (PFT)xdsl_util_get_connection_info, 0, 0,
      (type_t(*)())0, 0,   (type_t(*)())0,          0},
    {"LED Test",                    (PFT)xdsl_util_bcm63168_led_test, 0, 0,
      (type_t(*)())0, 0,   (type_t(*)())0,          0},
    {"Show xDSL Profile",    (PFT)xdsl_util_bcm63168_show_profile, 0, 0,
      (type_t(*)())0, 0,   (type_t(*)())0,          0},
    {"Ping xDSL",    (PFT)xdsl_util_ping, 0, 0,
      (type_t(*)())0, 0,   (type_t(*)())0,          0},
    {"Display xDSL SPI Flash Registers",
      (PFT)xdsl_util_bcm63168_show_spi_flash_reg, 0, 0,
      (type_t(*)())0, 0,   (type_t(*)())0,          0},
    {"Enable SPI write protect",    (PFT)xdsl_util_bcm63168_en_spi_flash_reg,
        0, 0, (type_t(*)())0, 0,   (type_t(*)())0,          0},
    {"Disable SPI write protect",    (PFT)xdsl_util_bcm63168_dis_spi_flash_reg,
        0, 0, (type_t(*)())0, 0,   (type_t(*)())0,          0},
    {"Restore CFE IOS Parameters",
      (PFT)xdsl_util_restore_cfe_param, IOS_CFE_PARAM, 0,
      (type_t(*)())0, 0,   (type_t(*)())0,          0},
    {"Restore CFE DIAG Parameters",
      (PFT)xdsl_util_restore_cfe_param, DIAG_CFE_PARAM, 0,
      (type_t(*)())0, 0,   (type_t(*)())0,          0},
    {"LED OFF",                    (PFT)xdsl_util_bcm63168_led, LED_OFF, 0,
      (type_t(*)())0, 0,   (type_t(*)())0,          0},
    {"LED Carrier Detect ON",      (PFT)xdsl_util_bcm63168_led, LED_CD_ON, 0,
      (type_t(*)())0, 0,   (type_t(*)())0,          0},
    {"LED Data ON",                (PFT)xdsl_util_bcm63168_led, LED_DATA_ON, 0,
      (type_t(*)())0, 0,   (type_t(*)())0,          0},
    {"Check xDSL SPI Flash protection",
      (PFT)xdsl_util_bcm63168_chk_spi_flash_protect, 0, 0,
      (type_t(*)())0, 0,   (type_t(*)())0,          0},
};

#define xDSL_UTILS_SUBMENU_TABLE_SZ (sizeof(diag_xdsl_util_submenu_table) / sizeof(submenu_xtable_t))

static mitem_t diag_xdsl_util_primary_items[xDSL_UTILS_SUBMENU_TABLE_SZ +
                                        MAX_BASE_ITEMS];
static mitem_t diag_xdsl_util_secondary_items[xDSL_UTILS_SUBMENU_TABLE_SZ +
                                          MAX_BASE_ITEMS];

char xdslutiltitle[50];

menuinfo_t xdsl_util_submenu = {
    xdslutiltitle,
    0,                              /* mtparam added by init_empty_menu */
    (PFT)menu_show_dflags,          /* notes missing WICs in combos */
    0,                              /* use generic prompt */
    0,                              /* size (bumped by add_menu_item() */
    diag_xdsl_util_primary_items,
};

menuinfo_t *xdsl_util_submenup = &xdsl_util_submenu;

/*=========================================
 * Showtime and Parametric Tests menu items
 *=========================================
 */
static submenu_xtable_t xdsl_pt_submenu_table[] = {
    {"Do Showtime",
     (PFT)diag_xdsl_do_showtime,         
     0, MF_CONTINUOUS | MF_SHOW_ERRCOUNT,
     (type_t(*)())0, 0, 
     (type_t(*)())0, 0},

    {"Do Set Tones",
     (PFT)diag_xdsl_do_set_tone,
     0, MF_SHOW_ERRCOUNT,
     (type_t(*)())0, 0, (type_t(*)())0, 0},

    {"Do Send All Tones",
     (PFT)diag_xdsl_do_send_all_tone,
     0, MF_SHOW_ERRCOUNT,
     (type_t(*)())0, 0, (type_t(*)())0, 0},

    {"Do Idle Listen",
     (PFT)diag_xdsl_do_idle_listen,
     0, MF_SHOW_ERRCOUNT,
     (type_t(*)())0, 0, (type_t(*)())0, 0},

    {"Set Idle Mode",
     (PFT)xdsl_util_connection_stop,
     0, MF_SHOW_ERRCOUNT,
     (type_t(*)())0, 0, (type_t(*)())0, 0},     

    {"Select test options",
     (PFT)diag_xdsl_select_test_options,
     0, MF_SHOW_ERRCOUNT,
     (type_t(*)())0, 0, (type_t(*)())0, 0},     
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
    {"xDSL Utilities",      (PFT)diag_xdsl_util,        0,   0,
     (type_t(*)())0, 0,         (type_t(*)())diag_xdsl_util,    0},
    {"xDSL Bootup",         (PFT)diag_xdsl_bootup,       0,  MM_3,
     (type_t(*)())0, 0,         (type_t(*)())0,                     0},
    {"xDSL Main Tests",      (PFT)diag_xdsl_main_tests,    0,  MM_3,
     (type_t(*)())0, 0,         (PFT)diag_xdsl_main_tests,        TRUE},
    {"Showtime & Parametric Tests", (PFT)xdsl_param_test,       0,   0,
     (type_t(*)())0, 0,         (type_t(*)())xdsl_param_test,   TRUE},
    {"LED Test",                (PFT)xdsl_util_bcm63168_led_test,0,   0,
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
 * Function: xdsl_linux_cret
 *
 * Description: This function is used to response that module is ready 
 *
 * Input : None
 *
 * Output: PASSED/FAILED
 *
 *******************************************************************************
 */
static int xdsl_linux_cret (void)
{
    const int maxlen = PLAT_DSL_MAX_LENGTH;
    char tty[maxlen];
    int fd;

    printf("\n");
    fflush(stdout);

    snprintf(tty, maxlen-1, PLAT_DSL_UART_DEV_STR);
    fd = open(tty, O_RDWR|O_NOCTTY);
    if (tty < 0) {
        perror("open tty failed");
        fflush(stdout);
        return (FAILED);
    }

    if (plat_uart_tx(fd, PLAT_DSL_CR_STRING) == FAILED) {
        goto exit_linux_cret_failed;
    }

    if (plat_uart_tx(fd, PLAT_DSL_CR_STRING) == FAILED) {
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
    const int maxlen = PLAT_DSL_MAX_LENGTH;
    char tty[maxlen];
    char linux_ip_str[maxlen];
    int fd, res;
    int setip_wait_retry = PLAT_DSL_LINUX_PROMPT_RETRY;

    prpass(testpass, "Set xdsl Linux IP address,");
    printf("\n");

    printf("Start Linux UART...\n");
    fflush(stdout);

    snprintf(tty, maxlen-1, PLAT_DSL_UART_DEV_STR);
    fd = open(tty, O_RDWR|O_NOCTTY);
    if (tty < 0) {
        perror("open tty failed");
        fflush(stdout);
        return (FAILED);
    }

    /* Step 1 : Find ">" prompt */
    for (ix = 0; ix < setip_wait_retry; ix++) {
        if (plat_uart_tx(fd, PLAT_DSL_CR_STRING) == FAILED) {
            goto exit_linux_ip_set_failed;
        }
        res = plat_uart_rx_polling(fd, PLAT_DSL_LINUX_PROMPT,
        		                   DSL_LINUX_TIMEOUT);
        if (res != FALSE) {
            printf("Found : %s\n", PLAT_DSL_LINUX_PROMPT);
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
    sprintf(linux_ip_str, "%s%s.%d", PLAT_DSL_IFCONFIG_BR0_STRING,
    		PLAT_DIAG_DSL_SUBNET_STR, PLAT_DIAG_MODULE_IP_ADDR);
    if (plat_uart_tx(fd, linux_ip_str) == FAILED) {
        goto exit_linux_ip_set_failed;
    }
    printf("Send Linux IP [%s]\n", linux_ip_str);
    fflush(stdout);
    if (plat_uart_tx(fd, PLAT_DSL_CR_STRING) == FAILED) {
        goto exit_linux_ip_set_failed;
    }

    /* Step 3: Enter shell mode */
    if (plat_uart_tx(fd, PLAT_DSL_SHELL_STRING) == FAILED) {
        goto exit_linux_ip_set_failed;
    }
    printf("Send [%s]\n", PLAT_DSL_SHELL_STRING);
    fflush(stdout);
    if (plat_uart_tx(fd, PLAT_DSL_CR_STRING) == FAILED) {
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
 * Function: diag_xdsl_bootup.
 *
 * Description: This function let xdsl boot up to diag.
 *
 * Input : None
 *
 * Output: PASSED/FAILED
 *
 *******************************************************************************
 */
static int diag_xdsl_bootup (void)
{
    int  ret_val;
    int ix;
    char *tname = "xdsl boot up";

    testname(tname);

    /* Step1: Reset */
    xdsl_util_bcm63168_reset();

    /* Step2: Set CFE IPs and Filename */ 
    if (xdsl_cfe_parms_set(DIAG_CFE_PARAM) == FAILED) {
        cterr('f',0,"CFE boot parameters set failed.\n");
        return (FAILED);
    }

    /* Step3: Reset */
    xdsl_util_bcm63168_reset();

    /* Step4: Check if DSL is ready (Timeout 90 secs) */
    printf("\nBooting xDSL Linux Kernel");
    for (ix = 0; ix < DSL_LINUX_WAIT_RETRY_TIMES; ix++) {
        printf(".");
        fflush(stdout);
        if (pri_intf_rdy_chk() == PASSED) {
            printf ("\nxdsl module is ready.\n");
            fflush(stdout);
            break;
        }
        /* wait for xDSL module to get CR and respond with prompt */
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
    ret_val = xdsl_util_ping();
    if (ret_val != PASSED) {
    	cterr('f',0,"Failed to ping BCM by nc.");
    	return (FAILED);
    }

    ret_val = bcm63168_check_sku_type();
    if (ret_val != PASSED) {
        cterr('f',0,"Failed to check SKU type.");
        return (FAILED);
    }

    return (ret_val);
}

/*******************************************************************************
 *
 * Function: diag_dsl_test
 *
 * Description: This function is the main entrance for xdsl module test .
 *
 * Input:
 *
 * Output: PASSED
 *
 *******************************************************************************
 */
int diag_dsl_test (boolean dsl_test_items_executed)
{
    int ret_val = PASSED;
    char cmd[128];
    struct stat sts;
    char plat_dest_diag_img[64] = PLAT_HOST_FIRMWARE_FOLDER_STRING;
    char plat_src_diag_img[32] = DSL_AM_DIAG_IMG;
    const int maxlen = PLAT_DSL_MAX_LENGTH;
    char tty_dev[maxlen];
    char *dsl_img_filename;
    char *tname = "xDSL(BCM63168)";

    testname(tname);

    plat_turbo_sku = 0;

    printf("\n\n========== Show Pluggable xDSL Control Type ==========\n");
    switch(xdsl_control_type()) {

        case XDSL_CTRL_TYPE_ANNEX_A:
            plat_turbo_sku = DSL_SKU_ANNEX_A;
            printf("xDSL Annex A\n");
        break;

        case XDSL_CTRL_TYPE_ANNEX_M:
            plat_turbo_sku = DSL_SKU_ANNEX_M;
            printf("xDSL Annex M\n");
        break;

        case XDSL_CTRL_TYPE_ANNEX_BJ:
            plat_turbo_sku = DSL_SKU_ANNEX_B;
            printf("xDSL Annex BJ\n");
        break;

        default:
            printf("xDSL Annex Uknown\n");
            return (FAILED);
        break;

    }
    /* Get DSL Image file name from environment */
    dsl_img_filename = getenv(DSL_IMAGE_NAME);

    if (dsl_img_filename == NULL || !strcmp("none", dsl_img_filename)) {
        printf("%s: Env variable %s is not set\n", __func__, DSL_IMAGE_NAME);
        cterr('f', 0, "Failed to set env export DSL_IMG_FILE");
        return (FAILED);
    } else {
        printf("DSL Image Filename: %s\n", dsl_img_filename);
        sprintf(plat_src_diag_img, "%s", dsl_img_filename);
    }
    
    /* Download FW image from the network for the first time */
    sprintf(plat_dest_diag_img, "%s%s", plat_dest_diag_img, plat_src_diag_img);
    if (stat(plat_dest_diag_img, &sts) == -1) {
        if (tftp_get(0 , plat_src_diag_img, 0, plat_dest_diag_img, 0) != PASSED) {
            sprintf(cmd, "rm -f %s", plat_dest_diag_img);
            system(cmd);
            fflush(stdout);
            cterr('f', 0, "Failed to tftp download firmware to local host");
            return (FAILED);
        }
    }

    if (FLAG_CFE_BOOT_TWO_IMG == TRUE) { 
        /*uncompress combo image*/
        sprintf(cmd, "%s %s -C %s", UNCOMPRESS_CMD, plat_dest_diag_img, 
                PLAT_HOST_FIRMWARE_FOLDER_STRING);
        system(cmd);
    }

    /* Set xdsl tty configuration */
    snprintf(tty_dev, maxlen-1, PLAT_DSL_UART_DEV_STR);
    if (plat_uart_setup(tty_dev) == FAILED) {
        cterr('f', 0, "Failed to setup UART");
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
    ret_val = xdsl_util_restore_cfe_param(IOS_CFE_PARAM);
    return (ret_val);
}

/*******************************************************************************
 *
 *  Function: diag_xdsl_util
 *
 *  Description: xdsl Utitlities menu
 *
 *  Input: None 
 *
 *  Returns: PASSED
 *
 *******************************************************************************
 */
static int diag_xdsl_util (int show_menu)
{
    sprintf(xdslutiltitle, "xDSL Utilities Menu");

    build_primary_submenu(diag_xdsl_util_submenu_table,
                          xDSL_UTILS_SUBMENU_TABLE_SZ,
                          xdslutiltitle, &xdsl_util_submenup);

    build_secondary_submenu(diag_xdsl_util_submenu_table,
                            xDSL_UTILS_SUBMENU_TABLE_SZ,
                            diag_xdsl_util_secondary_items);

    menu(xdsl_util_submenup, diag_xdsl_util_secondary_items, '\0');

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
    char *tname = "xDSL Showtime and Parametric";

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
 * Function   : diag_xdsl_main_tests
 *
 * Description: Entry function of xdsl Diag tests and utilities.
 *
 * Inputs     : show xdsl main test menu option
 *
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
static int diag_xdsl_main_tests (int show_menu)
{
    char *tname = "xDSL Main";

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
 * Function: diag_xdsl_bootflash_test
 *
 * Description: This function to test SPI boot flash.
 *
 * Input : none
 *
 * Output: PASSED/FAILED
 *
 *******************************************************************************
 */
static int diag_xdsl_bootflash_test (void)
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
    cterr_add_env_dump((PFV)show_plat_curr_temps);

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

    char tx_str[PLAT_NC_MAX_STR_SIZE];
    char *tname = "BCM63168 SPI Boot Flash";

    testname(tname);
    prpass(testpass, "%s, ", tname);

    plat_nc_init_parms_file();

    sprintf(tx_str, "%d,%d,%d,%d",
            bcm_op_mode, bcm_line_id, bcm_qln_monitor_time, bcm_qln_monitor_freq);

    plat_nc_dispatch_comm(DIAG_BCM63268_FLASH_TEST, tx_str);

    if (plat_nc_dispatch_comm_is_ok() != PASSED) {
    	cterr('f', 0, "%s failed.", __FUNCTION__);
        prcomplete(testpass, errcount, (char *)0);
        return (FAILED);
    }

    prcomplete(testpass, errcount, (char *)0);
    return (PASSED);
}

/*******************************************************************************
 *
 * Function   : diag_xdsl_bcm63168_dram_test
 *
 * Description: The function to test DDR3 memory.
 *
 * Inputs     : NONE 
 *             
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
static int diag_xdsl_bcm63168_dram_test (void)
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
    cterr_add_env_dump((PFV)show_plat_curr_temps);

    /* Segment 7: Top 3 Debugging Steps */
    cterr_add_debug("If, NC issue, please do the Ping xDSL utility to verify the RGMII path and "
                    "refer to Ping xDSL utility for more failure analysis. If not NC issue, switch console"
                    "to xDSL prompt # and use CLI to test the DDR test by type \" diag_tsn dram_test\"",
                    "Check the hardware interface between the xDSL SoC and the DDR3.",
                    "If there is no problem on these hardware interfaces, "
                    "replace one DDR3 and redo the test.");
#endif

    char tx_str[PLAT_NC_MAX_STR_SIZE];
    char *tname = "BCM63168 DRAM";

    testname(tname);
    prpass(testpass, "%s, ", tname);

    plat_nc_init_parms_file();

    sprintf(tx_str, "%d,%d,%d,%d",
            bcm_op_mode, bcm_line_id, bcm_qln_monitor_time, bcm_qln_monitor_freq);

    plat_nc_dispatch_comm(DIAG_BCM63268_DRAM_TEST, tx_str);

    if (plat_nc_dispatch_comm_is_ok() != PASSED) {
        cterr('f', 0, "%s failed.", __FUNCTION__);
        prcomplete(testpass, errcount, (char *)0);
       	return (FAILED);
    }

    prcomplete(testpass, errcount, (char *)0);
    return (PASSED);
}

/*******************************************************************************
 *
 * Function: diag_xdsl_interrupt_test
 *
 * Description: This function to test interrupt pin.
 *
 * Input : none
 *
 * Output: PASSED/FAILED
 *
 *******************************************************************************
 */
static int diag_xdsl_interrupt_test (void)
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
    cterr_add_env_dump((PFV)show_plat_curr_temps);

    /* Segment 7: Top 3 Debugging Steps */
    cterr_add_debug("Use \"FPGA registers Dump\" utility to check the FPGA Ext." 
                    "Interrupt Pending (register 0x1128), if the interrupt pending"
                    "then measure the DSL GPIO-35 whether it keep trigger the interrupt.",
                    "Check if the FPGA xDSL interrupt pending can be clear, before run"
                    "the interrupt test.",
                    "Check the interrupt path DSL connected to system FPGA.");
#endif

    char tx_str[PLAT_NC_MAX_STR_SIZE];
    uint reg_val;
    int ix = 0;
    char *tname = "VDSL Interrupt Test";

    testname(tname);
    prpass(testpass, "%s, ", tname);

    sprintf(tx_str, "%d,%d,%d,%d",
            bcm_op_mode, bcm_line_id, bcm_qln_monitor_time, bcm_qln_monitor_freq);

    prpass(testpass, "Clear the DSL interrupt pending, ");
    plat_nc_init_parms_file();
    /* Disable the DSL interrupt signal, do not trigger interrupt to FPGA
     * before testing. */
    plat_nc_dispatch_comm(DIAG_BCM63268_INTR_DISABLE, tx_str);
    if (plat_nc_dispatch_comm_is_ok() != PASSED) {
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
    plat_nc_init_parms_file();
    plat_nc_dispatch_comm(DIAG_BCM63268_INTR_ENABLE, tx_str);
    if (plat_nc_dispatch_comm_is_ok() != PASSED) {
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
    plat_nc_init_parms_file();
    plat_nc_dispatch_comm(DIAG_BCM63268_INTR_DISABLE, tx_str);
    if (plat_nc_dispatch_comm_is_ok() != PASSED) {
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
 * Function   : diag_xdsl_do_showtime
 *
 * Description: The function to train w/ DSLAM.
 *
 * Inputs     : NONE 
 *             
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
static int diag_xdsl_do_showtime (void)
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
 * Function   : diag_xdsl_do_set_tone
 *
 * Description: The function to configure DSL parameters.
 *
 * Inputs     : NONE 
 *             
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
static int diag_xdsl_do_set_tone (void)
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
 * Function   : diag_xdsl_do_send_all_tone
 *
 * Description: The function to perform set all tone test.
 *
 * Inputs     : NONE 
 *             
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
static int diag_xdsl_do_send_all_tone (void)
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
 * Function   : diag_xdsl_do_idle_listen
 *
 * Description: The function to perform do idle listen test.
 *
 * Inputs     : NONE 
 *             
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
static int diag_xdsl_do_idle_listen (void)
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
 * Function   : diag_xdsl_select_test_options
 *
 * Description: The function to configure DSL parameters.
 *
 * Inputs     : NONE 
 *             
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
static int diag_xdsl_select_test_options (void)
{
    int retval = FAILED;

    if ((retval = bcm63168_vdsl_test_option_select()) != PASSED) {
        cterr('f', 0, "%s failed.", __FUNCTION__);
        return (retval);
    }
    
    return (retval);
}

/*-------------------------------------------------
 * $Log: diag_dsl_test.c,v $
 * Revision 1.3  2019/05/21 07:44:19  wilbhuan
 * Add a new xDSL utility to check the SPI Flash protection.
 *
 * Revision 1.2  2019/01/10 06:36:22  wilbhuan
 * The beginning of Betelgeuse application code.
 *
 *-------------------------------------------------
 */
