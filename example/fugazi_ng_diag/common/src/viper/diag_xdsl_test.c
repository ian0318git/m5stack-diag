 /* $Id: diag_xdsl_test.c,v 1.4 2018/09/21 02:48:54 harrchan Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/viper/diag_xdsl_test.c,v $
 *------------------------------------------------------------------
 * 
 *  diag_xdsl_test.c 
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
#include "diag_xdsl_test.h"
#include "common_utils.h"
#include "diag_dsl_libs.h"
#include "queryflags.h"
#include "linux_ntwk.h" /* tftp_get */
#include "diag_fpga.h"
#include "platform_fru.h"
#include "platform_cookie.h"
#include "diag_temp_snsr_test.h"
#include "plat_defs.h"
#include "dnv_gpio_lib.h"
#include "diag_uart_lib.h"
#include "diag_nc_lib.h"
#include "dnv_eth_lib.h"
#include "diag_dsl_util.h"

/*******************************************************************************
 *                             Function Prototypes                             *
 *******************************************************************************
 */
static int xdsl_check_sku_type(void);
static int xdsl_utils(int);
static int xdsl_param_test(int);
static int xdsl_console_switch(void);
static int xdsl_bcm63168_reset(void);
static int xdsl_bcm63168_led_test(void);
static int xdsl_init_bcm63168(void);
static int xdsl_config_bcm63168(void);
static int xdsl_get_bcm63168_config(void);
static int xdsl_get_bcm63168_version(void);
static int xdsl_connection_start(void);
static int xdsl_connection_stop(void);
static int xdsl_get_xdsl_mib_info(void);
static int xdsl_get_xtm_bonding_info(void);
static int xdsl_get_xdsl_info(void);
static int xdsl_get_connection_info(void);
static int xdsl_main_tests(int);
static int xdsl_bootup(void);
static int xdsl_bootflash_test(void);
static int xdsl_interrupt_test(void);
static int xdsl_bcm63168_dram_test(void);
static int xdsl_do_showtime(void);
static int xdsl_do_set_tone(void);
static int xdsl_do_send_all_tone(void);
static int xdsl_do_idle_listen(void);
static int xdsl_select_test_options(void);
static int xdsl_ping_util(void);
static int pri_intf_rdy_chk(void);
static int xdsl_bcm63168_show_profile(void);
static int xdsl_bcm63168_show_spi_flash_reg(void);
static int xdsl_cfe_ios_parms_set(void);
static int restore_cfe_ios_param(void);
static int xdsl_bcm63168_en_spi_flash_reg(void);
static int xdsl_bcm63168_dis_spi_flash_reg(void);
static int xdsl_mrvl1512_reg_test(void);
void viper_dsl_env_setup(void);
int wrap_pri_intf_rdy_chk(void);

unsigned int viper_dsl_sku = 0xFF;

/*******************************************************************************
 *                              Global Variables                               *
 *******************************************************************************
 */
extern uint bcm_op_mode;
extern uint bcm_line_id;
extern uint bcm_qln_monitor_time;
extern uint bcm_qln_monitor_freq;

extern int do_all_menu_items(struct menuinfo *);

/*******************************************************************************
 *                                   Menus                                     *
 *******************************************************************************
 */

/*=========================================
 * xDSL Main Tests menu items
 *=========================================
 */
static submenu_xtable_t xdsl_main_tests_submenu_table[] = {
    {"BCM63168 DRAM Test",      (PFT)xdsl_bcm63168_dram_test,   0, MM_3,
     (type_t(*)())0,         0,         (type_t(*)())0,              0},
    {"BCM63168 NOR Flash Test", (PFT)xdsl_bootflash_test,       0, MM_3,
     (type_t(*)())0,         0,         (type_t(*)())0,              0},
    {"BCM63168 Interrupt Test", (PFT)xdsl_interrupt_test,       0, MM_3,
     (type_t(*)())0,         0,         (type_t(*)())0,              0},
    {"Marvell1512 Register Test", (PFT)xdsl_mrvl1512_reg_test,  0, MM_3,
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
      (PFT)restore_cfe_ios_param, 0, 0,
      (type_t(*)())0, 0,   (type_t(*)())0,          0},
    {"Check SKU tpye",
      (PFT)bcm63168_check_sku_type, 0, 0,
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
    {"xDSL Utilities",          (PFT)xdsl_utils,            0,   0,
     (type_t(*)())0, 0,         (type_t(*)())xdsl_utils,            0},
    {"xDSL Bootup",             (PFT)xdsl_bootup,            0,  MM_3,
     (type_t(*)())0, 0,         (type_t(*)())0,                     0},
    {"xDSL Main Tests",         (PFT)xdsl_main_tests,        0,  MM_3,
     (type_t(*)())0, 0,         (PFT)xdsl_main_tests,            TRUE},
    {"Showtime & Parametric Tests", (PFT)xdsl_param_test,       0,   0,
     (type_t(*)())0, 0,         (type_t(*)())xdsl_param_test,    TRUE},
    {"LED Test",                (PFT)xdsl_bcm63168_led_test,    0,   0,
     (type_t(*)())0, 0,         (type_t(*)())0,                     0},
    {"xDSL Switch Console",     (PFT)xdsl_console_switch,       0,   0,
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

/*******************************************************************************
 *
 * Function: xdsl_linux_cret
 *
 * Description: Transfer carriage return in xdsl uart
 *  
 * Input:  none
 *
 * Output: PASSED/FAILED
 *
 *******************************************************************************
 */
static int xdsl_linux_cret (void)
{
    const int maxlen = VIPER_DSL_MAX_LENGTH;
    char tty[maxlen];
    int fd = -1;

    printf("\n");
    fflush(stdout);

    snprintf(tty, maxlen-1, VIPER_DSL_UART_DEV_STR);
    fd = open(tty, O_RDWR|O_NOCTTY);
    if (fd < 0) {
        perror("open tty failed");
        fflush(stdout);
        return (FAILED);
    }

    if (diag_uart_tx(fd, VIPER_DSL_CR_STRING) == FAILED) {
        goto exit_linux_cret_failed;
    }

    if (diag_uart_tx(fd, VIPER_DSL_CR_STRING) == FAILED) {
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
    const int maxlen = VIPER_DSL_MAX_LENGTH;
    char tty[maxlen];
    char linux_ip_str[maxlen];
    int fd = -1, res;
    int setip_wait_retry = VIPER_DSL_LINUX_PROMPT_RETRY;

    prpass(testpass, "Set xdsl Linux IP address,");
    printf("\n");

    printf("Start Linux UART...\n");
    fflush(stdout);

    snprintf(tty, maxlen-1, VIPER_DSL_UART_DEV_STR);
    fd = open(tty, O_RDWR|O_NOCTTY);
    if (fd < 0) {
        perror("open tty failed");
        fflush(stdout);
        return (FAILED);
    }

    /* Step 1 : Find ">" prompt */
    for (ix = 0; ix < setip_wait_retry; ix++) {
        if (diag_uart_tx(fd, VIPER_DSL_CR_STRING) == FAILED) {
            goto exit_linux_ip_set_failed;
        }
        res = diag_uart_rx_polling(fd, VIPER_DSL_LINUX_PROMPT,
        		                   DSL_LINUX_TIMEOUT);
        if (res != FALSE) {
            printf("Found : %s\n", VIPER_DSL_LINUX_PROMPT);
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
    sprintf(linux_ip_str, "%s%s.%d", VIPER_DSL_IFCONFIG_BR0_STRING,
    		DIAG_DSL_SUBNET_STR, DIAG_MODULE_IP_ADDR);
    if (diag_uart_tx(fd, linux_ip_str) == FAILED) {
        goto exit_linux_ip_set_failed;
    }
    printf("Send Linux IP [%s]\n", linux_ip_str);
    fflush(stdout);
    if (diag_uart_tx(fd, VIPER_DSL_CR_STRING) == FAILED) {
        goto exit_linux_ip_set_failed;
    }

    /* Step 3: Enter shell mode */
    if (diag_uart_tx(fd, VIPER_DSL_SHELL_STRING) == FAILED) {
        goto exit_linux_ip_set_failed;
    }
    printf("Send [%s]\n", VIPER_DSL_SHELL_STRING);
    fflush(stdout);
    if (diag_uart_tx(fd, VIPER_DSL_CR_STRING) == FAILED) {
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
 * Description: This function set cfe parameters.
 *
 * Input : None
 *
 * Output: PASSED/FAILED
 *
 *******************************************************************************
 */
static int xdsl_cfe_parms_set (void)
{
    int ix;
    const int maxlen = VIPER_DSL_MAX_LENGTH;
    char tty[maxlen];
    char linux_file_str[maxlen];
    char linux_ip_str[maxlen];
    int result = PASSED;
    int tty_desc;
    char *dsl_img_filename;

    prpass(testpass, "Set xdsl CFE Parameters,");
    printf("\n");

    printf("Start CFE UART...\n");
    fflush(stdout);
    fflush(stderr);

    snprintf(tty, maxlen-1, VIPER_DSL_UART_DEV_STR);
    tty_desc = open(tty, O_RDWR | O_NOCTTY);
    if (tty_desc < 0) {
        perror("open tty failed");
        fflush(stdout);
        return (FAILED);
    }

    /* Step 0 : Catch "Press any key" */
    result = diag_uart_rx_polling(tty_desc, VIPER_DSL_PRESS_KEY_STRING,
    		                      DSL_CFE_PRESS_KEY_TIMEOUT);
    if (result != FALSE) {
        printf("Found : %s\n", VIPER_DSL_PRESS_KEY_STRING);
        fflush(stdout);
    } else {
        printf("[%s] Not Found.\n", VIPER_DSL_PRESS_KEY_STRING);
        fflush(stdout);
        goto exit_cfe_parms_set_failed;
    } 

    /* Step 1 : Find "CFE>" */
    for (ix = 0; ix < DSL_CFE_RETRY_TIMES; ix++) {
        if (diag_uart_tx(tty_desc, VIPER_DSL_CR_STRING) == FAILED) {
            goto exit_cfe_parms_set_failed;
        }
        result = diag_uart_rx_polling(tty_desc, VIPER_DSL_CFE_STRING,
        		                      DSL_CFE_TIMEOUT);
        if (result != FALSE) {
            printf("Found : %s\n", VIPER_DSL_CFE_STRING);
            fflush(stdout);
            break;
        }
    }

    if (ix == DSL_CFE_RETRY_TIMES) {
        printf("[%s] Not Found.\n", VIPER_DSL_CFE_STRING);
        fflush(stdout);
        goto exit_cfe_parms_set_failed;
    }
    msleep(WAIT_DSL_CFE_PROMPT);

    /* Step 2 : Set board parameters   */
    if (diag_uart_tx(tty_desc, VIPER_DSL_CHANGE_CFE_PARMS_STRING) == FAILED) {
        goto exit_cfe_parms_set_failed;
    }    
    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("Send bdpm [%s]\n", VIPER_DSL_CHANGE_CFE_PARMS_STRING);
        fflush(stdout);
    }
    if (diag_uart_tx(tty_desc, VIPER_DSL_CR_STRING) == FAILED) {
        goto exit_cfe_parms_set_failed;
    }
    msleep(WAIT_DSL_UART_TX);
    sprintf(linux_ip_str, "%s.%d:ffffff00", DIAG_DSL_SUBNET_STR,
            DIAG_MODULE_IP_ADDR);
    /* Step 3: Set Board IP */
    /* Set 192.168.2.101 */
    if (diag_uart_tx(tty_desc, linux_ip_str) == FAILED) {
        goto exit_cfe_parms_set_failed;
    } 
    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("Send Board IP[%s]\n", linux_ip_str);
        fflush(stdout);
    }
    if (diag_uart_tx(tty_desc, VIPER_DSL_CR_STRING) == FAILED) {
        goto exit_cfe_parms_set_failed;
    }
    msleep(WAIT_DSL_UART_TX);

    sprintf(linux_ip_str, "%s.%d", DIAG_DSL_SUBNET_STR,
            DIAG_HOST_IP_ADDR);
    /* Step 4: Set Host IP */
    /* Set 192.168.2.100 */
    if (diag_uart_tx(tty_desc, linux_ip_str) == FAILED) {
        goto exit_cfe_parms_set_failed;
    } 
    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("Send Host IP[%s]\r", linux_ip_str);
        fflush(stdout);
    }
    if (diag_uart_tx(tty_desc, VIPER_DSL_CR_STRING) == FAILED) {
        goto exit_cfe_parms_set_failed;
    }

    msleep(WAIT_DSL_UART_TX);

    sprintf(linux_ip_str, "%s.%d", DIAG_DSL_SUBNET_STR,
    		DIAG_GATEWAY_IP_ADDR);
    /* Step 5: Set Gateway IP */
    /* Set 192.168.2.100 */
    if (diag_uart_tx(tty_desc, linux_ip_str) == FAILED) {
        goto exit_cfe_parms_set_failed;
    } 
    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("Send GW IP[%s]\n", linux_ip_str);
        fflush(stdout);
    }
    if (diag_uart_tx(tty_desc, VIPER_DSL_CR_STRING) == FAILED) {
        goto exit_cfe_parms_set_failed;
    }
    msleep(WAIT_DSL_UART_TX);

    
    /* Step 6: one more carrier return */
    if (diag_uart_tx(tty_desc, VIPER_DSL_CR_STRING) == FAILED) {
        goto exit_cfe_parms_set_failed;
    } 
    msleep(WAIT_DSL_UART_TX);

    /* Step 7: Set default linux file name */
    /* Check environment paramter, if not use default filename*/ 
    dsl_img_filename = getenv(DSL_IMAGE_NAME);

    if (dsl_img_filename == NULL || !strcmp("none", dsl_img_filename)) {
        printf("%s: Env variable %s is not set\n", __func__, DSL_IMAGE_NAME);
        sprintf(linux_file_str, "%s%s", VIPER_HOST_FIRMWARE_FOLDER_STRING,
                DSL_DEFAULT_IMG_FILE);
        printf("Default DSL Image Filename: %s\n", linux_file_str);
    } else {
        printf("DSL Image Filename: %s\n", dsl_img_filename);
        sprintf(linux_file_str, "%s%s", VIPER_HOST_FIRMWARE_FOLDER_STRING, dsl_img_filename);
    }
    
    if (diag_uart_tx(tty_desc, linux_file_str) == FAILED) {
        goto exit_cfe_parms_set_failed;
    }
    printf("Diag image [%s]\n", linux_file_str);
    fflush(stdout);
    if (diag_uart_tx(tty_desc, VIPER_DSL_CR_STRING) == FAILED) {
        goto exit_cfe_parms_set_failed;
    }
    msleep(WAIT_DSL_UART_TX);

    /* Step 8 : Find "command status" */
    for (ix = 0; ix < DSL_CFE_RETRY_TIMES; ix++) {
        if (diag_uart_tx(tty_desc, VIPER_DSL_CR_STRING) == FAILED) {
            goto exit_cfe_parms_set_failed;
        }
        result = diag_uart_rx_polling(tty_desc, VIPER_DSL_COMMAND_STATUS_STRING,
        		                      DSL_CFE_COMMAND_STATUS_TIMEOUT);
        if (result != FALSE) {
            printf("Found : %s\n", VIPER_DSL_COMMAND_STATUS_STRING);
            fflush(stdout);
            break;
        }
        msleep(WAIT_DSL_CFE_COMMAND_STATUS);
    }

    if (ix == DSL_CFE_RETRY_TIMES) {
        printf("[%s] Not Found.\n", VIPER_DSL_COMMAND_STATUS_STRING);
        fflush(stdout);
        goto exit_cfe_parms_set_failed;
    }
    msleep(WAIT_DSL_CFE_COMMAND_STATUS);

    /* Step 9 : Find "CFE>" */
    for (ix = 0; ix < DSL_CFE_RETRY_TIMES; ix++) {
        if (diag_uart_tx(tty_desc, VIPER_DSL_CR_STRING) == FAILED) {
            goto exit_cfe_parms_set_failed;
        }
        result = diag_uart_rx_polling(tty_desc, VIPER_DSL_CFE_STRING,
        		                      DSL_CFE_TIMEOUT);
        if (result != FALSE) {
            printf("Found : %s\n", VIPER_DSL_CFE_STRING);
            fflush(stdout);
            break;
        } 
    }

    if (ix == DSL_CFE_RETRY_TIMES) {
        printf("[%s] Not Found.\n", VIPER_DSL_CFE_STRING);
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
 * Function: xdsl_cfe_ios_parms_set
 *
 * Description: This function set cfe ios parameters.
 *
 * Input : None
 *
 * Output: PASSED/FAILED
 *
 *******************************************************************************
 */
static int xdsl_cfe_ios_parms_set (void)
{
    int ix;
    const int maxlen = VIPER_DSL_MAX_LENGTH;
    char tty[maxlen];
    char linux_file_str[maxlen];
    char linux_ip_str[maxlen];
    int result = PASSED;
    int tty_desc;
    char *dsl_img_filename;

    prpass(testpass, "Set xdsl CFE ios Parameters,");
    printf("\n");

    printf("Start CFE UART...\n");
    fflush(stdout);
    fflush(stderr);

    snprintf(tty, maxlen-1, VIPER_DSL_UART_DEV_STR);
    tty_desc = open(tty, O_RDWR | O_NOCTTY);
    if (tty_desc < 0) {
        perror("open tty failed");
        fflush(stdout);
        return (FAILED);
    }

    /* Step 0 : Catch "Press any key" */
    result = diag_uart_rx_polling(tty_desc, VIPER_DSL_PRESS_KEY_STRING,
    		                      DSL_CFE_PRESS_KEY_TIMEOUT);
    if (result == TRUE) {
        printf("Found : %s\n", VIPER_DSL_PRESS_KEY_STRING);
        fflush(stdout);
    } else {
        printf("[%s] Not Found.\n", VIPER_DSL_PRESS_KEY_STRING);
        fflush(stdout);
        goto exit_cfe_parms_set_failed;
    } 

    /* Step 1 : Find "CFE>" */
    for (ix = 0; ix < DSL_CFE_RETRY_TIMES; ix++) {
        if (diag_uart_tx(tty_desc, VIPER_DSL_CR_STRING) == FAILED) {
            goto exit_cfe_parms_set_failed;
        }
        result = diag_uart_rx_polling(tty_desc, VIPER_DSL_CFE_STRING,
        		                      DSL_CFE_TIMEOUT);
        if (result == TRUE) {
            printf("Found : %s\n", VIPER_DSL_CFE_STRING);
            fflush(stdout);
            break;
        }
    }

    if (ix == DSL_CFE_RETRY_TIMES) {
        printf("[%s] Not Found.\n", VIPER_DSL_CFE_STRING);
        fflush(stdout);
        goto exit_cfe_parms_set_failed;
    }
    msleep(WAIT_DSL_CFE_PROMPT);

    /* Step 2 : Set board parameters   */
    if (diag_uart_tx(tty_desc, VIPER_DSL_CHANGE_CFE_PARMS_STRING) == FAILED) {
        goto exit_cfe_parms_set_failed;
    }    
    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("Send bdpm [%s]\n", VIPER_DSL_CHANGE_CFE_PARMS_STRING);
        fflush(stdout);
    }
    if (diag_uart_tx(tty_desc, VIPER_DSL_CR_STRING) == FAILED) {
        goto exit_cfe_parms_set_failed;
    }
    msleep(WAIT_DSL_UART_TX);

    sprintf(linux_ip_str, "%s.%d:ffffff00", VIPER_IOS_DSL_SUBNET_STR,
            VIPER_IOS_MODULE_IP_ADDR);
    /* Step 3: Set Board IP */
    if (diag_uart_tx(tty_desc, linux_ip_str) == FAILED) {
        goto exit_cfe_parms_set_failed;
    } 
    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("Send Board IP[%s]\n", linux_ip_str);
        fflush(stdout);
    }
    if (diag_uart_tx(tty_desc, VIPER_DSL_CR_STRING) == FAILED) {
        goto exit_cfe_parms_set_failed;
    }
    msleep(WAIT_DSL_UART_TX);

    sprintf(linux_ip_str, "%s.%d", VIPER_IOS_DSL_SUBNET_STR,
            VIPER_IOS_HOST_IP_ADDR);
    /* Step 4: Set Host IP */
    if (diag_uart_tx(tty_desc, linux_ip_str) == FAILED) {
        goto exit_cfe_parms_set_failed;
    } 
    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("Send Host IP[%s]\r", linux_ip_str);
        fflush(stdout);
    }
    if (diag_uart_tx(tty_desc, VIPER_DSL_CR_STRING) == FAILED) {
        goto exit_cfe_parms_set_failed;
    }
    msleep(WAIT_DSL_UART_TX);

    sprintf(linux_ip_str, "%s.%d", VIPER_IOS_DSL_SUBNET_STR,
    		VIPER_IOS_GATEWAY_IP_ADDR);
    /* Step 5: Set Gateway IP */
    if (diag_uart_tx(tty_desc, linux_ip_str) == FAILED) {
        goto exit_cfe_parms_set_failed;
    } 
    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("Send GW IP[%s]\n", linux_ip_str);
        fflush(stdout);
    }
    if (diag_uart_tx(tty_desc, VIPER_DSL_CR_STRING) == FAILED) {
        goto exit_cfe_parms_set_failed;
    }
    msleep(WAIT_DSL_UART_TX);
    
    /* Step 6: one more carrier return */
    if (diag_uart_tx(tty_desc, VIPER_DSL_CR_STRING) == FAILED) {
        goto exit_cfe_parms_set_failed;
    } 
    msleep(WAIT_DSL_UART_TX);

    /* Step 7: Set default linux file name */
    dsl_img_filename = getenv(IOS_IMAGE_NAME);

    if (dsl_img_filename == NULL || !strcmp("none", dsl_img_filename)) {
        printf("%s: Env variable %s is not set\n", __func__, IOS_IMAGE_NAME);
        sprintf(linux_file_str, "%s%s", VIPER_IOS_HOST_FIRMWARE_FOLDER_STRING,
                VIPER_IOS_VADSL_FIRMWARE_STRING);
        printf("Default IOS Image Filename: %s\n", VIPER_IOS_VADSL_FIRMWARE_STRING);
    } else {
        printf("IOS Image Filename: %s\n", dsl_img_filename);
        sprintf(linux_file_str, "%s%s", VIPER_IOS_HOST_FIRMWARE_FOLDER_STRING,
                dsl_img_filename);
    }
    
    if (diag_uart_tx(tty_desc, linux_file_str) == FAILED) {
        goto exit_cfe_parms_set_failed;
    }
    printf("Diag image [%s]\n", linux_file_str);
    fflush(stdout);
    if (diag_uart_tx(tty_desc, VIPER_DSL_CR_STRING) == FAILED) {
        goto exit_cfe_parms_set_failed;
    }
    msleep(WAIT_DSL_UART_TX);

    /* Step 8 : Find "command status" */
    for (ix = 0; ix < DSL_CFE_RETRY_TIMES; ix++) {
        if (diag_uart_tx(tty_desc, VIPER_DSL_CR_STRING) == FAILED) {
            goto exit_cfe_parms_set_failed;
        }
        result = diag_uart_rx_polling(tty_desc, VIPER_DSL_COMMAND_STATUS_STRING,
        		                      DSL_CFE_COMMAND_STATUS_TIMEOUT);
        if (result == TRUE) {
            printf("Found : %s\n", VIPER_DSL_COMMAND_STATUS_STRING);
            fflush(stdout);
            break;
        }
        msleep(WAIT_DSL_CFE_COMMAND_STATUS);
    }

    if (ix == DSL_CFE_RETRY_TIMES) {
        printf("[%s] Not Found.\n", VIPER_DSL_COMMAND_STATUS_STRING);
        fflush(stdout);
        goto exit_cfe_parms_set_failed;
    }
    msleep(WAIT_DSL_CFE_COMMAND_STATUS);

    /* Step 9 : Find "CFE>" */
    for (ix = 0; ix < DSL_CFE_RETRY_TIMES; ix++) {
        if (diag_uart_tx(tty_desc, VIPER_DSL_CR_STRING) == FAILED) {
            goto exit_cfe_parms_set_failed;
        }
        result = diag_uart_rx_polling(tty_desc, VIPER_DSL_CFE_STRING,
        		                      DSL_CFE_TIMEOUT);
        if (result == TRUE) {
            printf("Found : %s\n", VIPER_DSL_CFE_STRING);
            fflush(stdout);
            break;
        } 
    }

    if (ix == DSL_CFE_RETRY_TIMES) {
        printf("[%s] Not Found.\n", VIPER_DSL_CFE_STRING);
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
    char *tname = "xdsl boot up";
    uchar mb_get_loc[FRU_SIZE] = {0};
    uchar mb_get_pid[FRU_SIZE] = {0};
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
    cterr_add_component("Intel Denverton SOC C3558", "SGMII",
                        "SMI", "BCM63168", "Boot flash");

    /* Segment 5: register and memory dump */

    /* Segment 6: Platform Environment initialized here*/

    /* Segment 7: Top 3 Debugging Steps */
    cterr_add_debug("Console switch to the xDSL SoC and check "
                    "whether boot up to bootloader. If not, verify"
                    " the bootloader image which is stored in xDSL SPI flash",
                    "Check the ethernet path between M/B and the "
                    "SoC of the xDSL module.",
                    "Check the interface between xDSL SoC and the xDSL DRAM.", 
                    "Refer to Ping xDSL utiltiy for more detail.");


    testname(tname);

    /* Step1: Reset */
    xdsl_bcm63168_reset();

    /* Step2: Set CFE IPs and Filename */ 
    if (xdsl_cfe_parms_set() == FAILED) {
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
        if ((ix == (DSL_BOOT_UP_WAIT_30)) || (ix == (DSL_BOOT_UP_WAIT_35))) {
            xdsl_linux_cret();
        }
    }
    if (ix == DSL_LINUX_WAIT_RETRY_TIMES) {
        cterr('f',0,"Failed to get xDSL module ready");
        return (FAILED);
    }

    /* Step5: Set DSL Linux IPs */
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
 * Function: diag_xdsl_tests().
 *
 * Description: This function is the main entrance for xdsl module test .
 *
 * Input: menu option 
 *
 * Output: PASSED
 *
 *******************************************************************************
 */
int diag_xdsl_tests (boolean dsl_test_items_executed)
{
    int ret_val = PASSED;
    char viper_src_diag_img[32] = {0};
    const int maxlen = VIPER_DSL_MAX_LENGTH;
    char tty_dev[maxlen];
    char *tname = "xDSL";
    char dsl_image_filename[128] = {0};
    char *dsl_img_filename = dsl_image_filename;

    testname(tname);

    if (get_dsl_annex_sku_id() == DSL_SKU_ANNEX_A) {
        printf("xDSL Annex A\n"); 
        viper_dsl_sku = DSL_SKU_ANNEX_A;
    } else if (get_dsl_annex_sku_id() == DSL_SKU_ANNEX_B) {
        printf("xDSL Annex B/J\n");
        viper_dsl_sku = DSL_SKU_ANNEX_B;
    } else if (get_dsl_annex_sku_id() == DSL_SKU_ANNEX_M) {
        printf("xDSL Annex M\n");
        viper_dsl_sku = DSL_SKU_ANNEX_M;
    } else {
        printf("Unknown xDSL Annex??\n");
    }

    /* Check environment paramter, if not use default filename*/ 
    dsl_img_filename = getenv(DSL_IMAGE_NAME);

    if (dsl_img_filename == NULL || !strcmp("none", dsl_img_filename)) {
        printf("%s: Env variable %s is not set\n", __func__, DSL_IMAGE_NAME);
        if ((viper_dsl_sku == DSL_SKU_ANNEX_A) || 
            (viper_dsl_sku == DSL_SKU_ANNEX_M)) { 
            sprintf(viper_src_diag_img, "%s",
                    DSL_AM_DIAG_IMG);
        } else {
            sprintf(viper_src_diag_img, "%s",
                    DSL_BJ_DIAG_IMG);
        }
        printf("Default DSL Image Filename: %s\n", viper_src_diag_img);
    } else {
        printf("DSL Image Filename: %s\n", dsl_img_filename);
        sprintf(viper_src_diag_img, "%s", dsl_img_filename);
    }
    
    /* Set xdsl tty configuration */
    snprintf(tty_dev, maxlen-1, VIPER_DSL_UART_DEV_STR);
    if (diag_uart_setup(tty_dev) == FAILED) {
        printf("\nFailed to setup UART\n");
    }


    build_primary_submenu(xdsl_tests_submenu_table,
                          XDSL_TESTS_SUBMENU_TABLE_SIZE, "xDSL",
                          &xdsl_submenup);

    build_secondary_submenu(xdsl_tests_submenu_table,
                            XDSL_TESTS_SUBMENU_TABLE_SIZE,
                            xdsl_tests_secondary_items);

    if (dsl_test_items_executed) {
        do_all_menu_items(&xdsl_subtest_menu);
    } else {
        menu(&xdsl_subtest_menu, xdsl_tests_secondary_items, '\0');
    }


    return (ret_val);
}

/*******************************************************************************
 *
 *  Function: xdsl_utils
 *
 *  Description: xdsl Utitlities menu
 *
 *  Input: menu option
 *
 *  Returns: PASSED
 *
 *******************************************************************************
 */
static int xdsl_utils (int show_menu)
{
    sprintf(xdslutiltitle, "xDSL Utilities Menu");

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
 * Input       : menu option
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
                          "Showtime and Parametric", &xdsl_pt_submenup);
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
    char *tname = "xDSL Main Tests Menu";

    testname(tname);

    /* Check xdsl is ready */
    if (pri_intf_rdy_chk() != PASSED) {
        printf ("\nxdsl module is not ready."
                "Please boot up xdsl module first.\n");
        return (PASSED);
    }

    build_primary_submenu(xdsl_main_tests_submenu_table,
                          XDSL_MAIN_TESTS_SUBMENU_TABLE_SIZE,
                          "xDSL Main", &xdsl_main_submenup);
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
    cterr_add_component("Intel Denverton SOC C3558", "BCM63168", "SPI Flash");

    /* Segment 5: register and memory dump */

    /* Segment 6: Platform Environment initialized here*/

    /* Segment 7: Top 3 Debugging Steps */
    cterr_add_debug("Do the Ping xDSLutility to verify the RGMII path and "
                    "refer to Ping xDSLutility for more failure analysis.",
                    "Switch console to xDSL SoC and use the "
                    "\"diag_gs spi_write\" command to write the "
                    "known pattern into scratch pad of NOR flash.",
                    "Run \"diag_gs spi_read\" to read the data "
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
                    "If step c is good, maybe there is one sector in SPI flash "
                    "corrupt. Replace one SPI flash and redo the test. "
                    "If step c fail, check the path between "
                    "the xDSL SoC and SPI flash.");

    char tx_str[VIPER_NC_MAX_STR_SIZE];
    char *tname = "BCM63168 SPI Boot Flash";

    testname(tname);
    prpass(testpass, "%s, ", tname);

    viper_nc_init_parms_file();

    sprintf(tx_str, "%d,%d,%d,%d",
            bcm_op_mode, bcm_line_id, bcm_qln_monitor_time, bcm_qln_monitor_freq);

    viper_nc_dispatch_comm(DIAG_BCM63268_FLASH_TEST, tx_str);

    if (viper_nc_dispatch_comm_is_ok() != PASSED) {
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
    cterr_add_component("Intel Denverton SOC C3558", "BCM63168", "DDR RAM");

    /* Segment 5: register and memory dump */

    /* Segment 6: Platform Environment initialized here*/

    /* Segment 7: Top 3 Debugging Steps */
    cterr_add_debug("Do the Ping xDSLutility to verify the RGMII path and "
                    "refer to Ping xDSLutility for more failure analysis.",
                    "Check the interface between the xDSL SoC and the DDR3.",
                    "If there is no problem on these interfaces, "
                    "replace one DDR3 and redo the test.");

    char tx_str[VIPER_NC_MAX_STR_SIZE];
    char *tname = "BCM63168 DRAM";

    testname(tname);
    prpass(testpass, "%s, ", tname);

    viper_nc_init_parms_file();

    sprintf(tx_str, "%d,%d,%d,%d",
            bcm_op_mode, bcm_line_id, bcm_qln_monitor_time, bcm_qln_monitor_freq);

    viper_nc_dispatch_comm(DIAG_BCM63268_DRAM_TEST, tx_str);

    if (viper_nc_dispatch_comm_is_ok() != PASSED) {
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
    cterr_add_component("Intel Denverton SOC C3558", "BCM63168", "GPIO");

    /* Segment 5: register and memory dump */

    /* Segment 6: Platform Environment initialized here*/

    /* Segment 7: Top 3 Debugging Steps */
    cterr_add_debug("Do the Ping xDSL utility to verify the ethernet"
                    " path and refer to Ping xDSL utility for more failure analysis",
                    "If step a. fail please check connection between xDSL and CPU");

    char tx_str[VIPER_NC_MAX_STR_SIZE];
    uint cpu_intr;
    int ix = 0;
    char *tname = "BCM63168 Interrupt Test";
    unsigned char dsl_gpio_pin = DNV_GPIO_4;

    testname(tname);
    prpass(testpass, "%s, ", tname);

    sprintf(tx_str, "%d,%d,%d,%d",
            bcm_op_mode, bcm_line_id, bcm_qln_monitor_time, bcm_qln_monitor_freq);

    prpass(testpass, "Clear the DSL interrupt pending, ");
    viper_nc_init_parms_file();
    /* Disable the DSL interrupt signal, do not trigger interrupt to FPGA
     * before testing. */
    viper_nc_dispatch_comm(DIAG_BCM63268_INTR_DISABLE, tx_str);
    if (viper_nc_dispatch_comm_is_ok() != PASSED) {
    	cterr('f', 0, "%s failed.", __FUNCTION__);
	    prcomplete(testpass, errcount, (char *)0);
        return (FAILED);
    }
    mdelay(WAIT_FOR_CTRL_DSL_INTR);

    /* Generate the DSL interrupt signal, trigger interrupt to FPGA */
    prpass(testpass, "Generate the DSL interrupt signal to FPGA, ");
    viper_nc_init_parms_file();
    viper_nc_dispatch_comm(DIAG_BCM63268_INTR_ENABLE, tx_str);
    if (viper_nc_dispatch_comm_is_ok() != PASSED) {
    	cterr('f', 0, "%s failed.", __FUNCTION__);
	    prcomplete(testpass, errcount, (char *)0);
        return (FAILED);
    }
    mdelay(WAIT_FOR_CTRL_DSL_INTR);


    for (ix = 0; ix < VIPER_DSL_INT_RETRY; ix++) {
        if (dnv_gpio_read_rx_val(dsl_gpio_pin, &cpu_intr) != PASSED) {
            cterr('f', 0, "%s:%d Failed to check CPU side interrupt info.",
                          __FUNCTION__, __LINE__);
            return (FAILED);
        }

        if ((NVRAM)->diagflag & D_VERBOSE) {
            printf("After set interrupt: %x\n", cpu_intr);
        }
        if (cpu_intr == GPIO_LOW) {
            break;
        } else {
            msleep(100);
        }
    }

    if (ix == VIPER_DSL_INT_RETRY) {
        cterr('f', 0,
              "Timeout waiting for interrupt.");
        return (FAILED);
    }


    prpass(testpass, "Stop generating the DSL interrupt signal to FPGA, ");
    viper_nc_init_parms_file();
    viper_nc_dispatch_comm(DIAG_BCM63268_INTR_DISABLE, tx_str);
    if (viper_nc_dispatch_comm_is_ok() != PASSED) {
    	cterr('f', 0, "%s failed.", __FUNCTION__);
	prcomplete(testpass, errcount, (char *)0);
        return (FAILED);
    }
    mdelay(WAIT_FOR_CTRL_DSL_INTR);

    for (ix = 0; ix < VIPER_DSL_INT_RETRY; ix++) {
        if (dnv_gpio_read_rx_val(dsl_gpio_pin, &cpu_intr) != PASSED) {
            cterr('f', 0, "%s:%d Failed to check CPU side interrupt info.",
                          __FUNCTION__, __LINE__);
            return (FAILED);
        }

        printf("clean interrupt: %x\n", cpu_intr);

        if (cpu_intr == GPIO_HIGH) {
            break;
        } else {
            msleep(100);
        }
    }

    if (ix == VIPER_DSL_INT_RETRY) {
        cterr('f', 0,
              "Timeout waiting for interrupt to clean.");
        return (FAILED);
    }

    prpass(testpass, "DSL interrupt test passed, ");
    prcomplete(testpass, errcount, (char *)0);
    return (PASSED);
}

/*******************************************************************************
 *
 * Function   : xdsl_mrvl1512_reg_test
 *
 * Description: The function to test 1512 register
 *
 * Inputs     : NONE 
 *             
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
static int xdsl_mrvl1512_reg_test (void)
{
    uchar mb_get_loc[FRU_SIZE] = {0};
    uchar mb_get_pid[FRU_SIZE] = {0};
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
    cterr_add_component("Intel", "BCM83168", "Marvell 1512");

    /* Segment 5: register and memory dump */

    /* Segment 6: Platform Environment initialized here*/

   /* Segment 7: Top 3 Debugging Steps */
    cterr_add_debug("Switch console to BCM side, and make sure BCM can reach "
                    "Host by typing \"ping 192.168.2.101\". "
                    "If ping fails, check two paths. "
                    "One is the interface between host and Marvell1512, the "
                    "other one is RGMII between the Marvell1512 and BCM63168.",
                    "Using BCM command \"ethctl phy ext 0 22 0x0\" and "
                    "\"ethctl phy ext 0 18 0xFFFF\" to write the value(0xFFFF) "
                    "into register.",
                    "Use command \"ethctl phy ext 0 18\" to read the register "
                    "value and compare it against the written value(0xFFFF).",
                    "If step3 is good, maybe there is one register corrupt. "
                    "Replace one Marvell 1512 chip and redo the test. If step c. "
                    "fail, check the SMI between the BCM63168 and Marvell1512."
                    );

    char tx_str[VIPER_NC_MAX_STR_SIZE];
    char *tname = "Marvell 1512 register";

    testname(tname);
    prpass(testpass, "%s, ", tname);

    viper_nc_init_parms_file();

    sprintf(tx_str, "%d,%d,%d,%d",
            bcm_op_mode, bcm_line_id, bcm_qln_monitor_time, bcm_qln_monitor_freq);

    viper_nc_dispatch_comm(DIAG_COMMAND_MRVL1512_REG_TEST, tx_str);

    if (viper_nc_dispatch_comm_is_ok() != PASSED) {
        cterr('f', 0, "%s failed.", __FUNCTION__);
    	prcomplete(testpass, errcount, (char *)0);
       	return (FAILED);
    }

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
    picocom.tty_dev = VIPER_DSL_UART_DEV_STR;
    picocom.baudrate = DSL_BAUDRATE_9600;    
    picocom.databit = DSL_DATABIT_8;
    picocom.parity = DSL_PARITY_1;
    picocom.flow = DSL_FLOW_N;

    if (diag_console_switch(&picocom) != PASSED) {
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
    /* FGPA 0x804 bit 20 */
    if (fpga_reset_api(FPGA_EXTER_DEV_RST_REG, EXT_DSL_CHIP_RESET, TRUE,
                       WAITTIME_20_MS) == FAILED) {
    	return (FAILED);
    }
    /* DSL SKU un-reset the dsl module */
    if (fpga_reset_api(FPGA_EXTER_DEV_RST_REG, EXT_DSL_CHIP_RESET, FALSE,
                       WAITTIME_20_MS) == FAILED) {
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
 * Description: The functions to get DSL information.
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
 * Description: The functions to get DSL information.
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
    int retval = FAILED;
    char *tname = "xDSL LED";

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
    char tx_str[VIPER_NC_MAX_STR_SIZE];

    viper_nc_init_parms_file();

    sprintf(tx_str, "%d,%d,%d,%d",
            bcm_op_mode, bcm_line_id, bcm_qln_monitor_time, bcm_qln_monitor_freq);

    viper_nc_dispatch_comm(DIAG_PING_BCM63268, tx_str);

    if (viper_nc_dispatch_comm_is_ok() != PASSED) {
        printf("%s fail\n", __FUNCTION__);
        return (FAILED);
    }

    return (PASSED);
}

/*******************************************************************************
 *
 * Function   : wrap_pri_intf_rdy_chk
 *
 * Description: this wrap function to perform pri_intf_rdy_chk
 *
 * Inputs     : None
 *
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
int wrap_pri_intf_rdy_chk (void)
{


    return (pri_intf_rdy_chk());

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

    if (fpga_read_reg(FPGA_DSL_STATUS_CTL_REG, &data) == FAILED) {
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
 * Description: Wrap function to check if SKU ID in xdsl module and in FPGA
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
 * Function: xdsl_confirm_cfe_ios_parms_set
 *
 * Description: This function set cfe ios parameters.
 *
 * Input : None
 *
 * Output: PASSED/FAILED
 *
 *******************************************************************************
 */
static int xdsl_confirm_cfe_ios_parms_set (void)
{
    int ix;
    const int maxlen = VIPER_DSL_MAX_LENGTH;
    char tty[maxlen];
    char linux_file_str[maxlen];
    char linux_ip_str[maxlen];
    int result = PASSED;
    int tty_desc;
    char dsl_image_filename[128] = {0};
    char *dsl_img_filename = dsl_image_filename;

    prpass(testpass, "Confirming CFE ios Parameters,");
    printf("\n");

    printf("Start CFE UART...\n");
    fflush(stdout);
    fflush(stderr);

    snprintf(tty, maxlen-1, VIPER_DSL_UART_DEV_STR);
    tty_desc = open(tty, O_RDWR | O_NOCTTY);
    if (tty_desc < 0) {
        perror("open tty failed");
        fflush(stdout);
        return (FAILED);
    }

    /* Step 0 : Catch "Press any key" */
    result = diag_uart_rx_polling(tty_desc, VIPER_DSL_PRESS_KEY_STRING,
    		                      DSL_CFE_PRESS_KEY_TIMEOUT);
    if (result == TRUE) {
        printf("Found : %s\n", VIPER_DSL_PRESS_KEY_STRING);
        fflush(stdout);
    } else {
        printf("[%s] Not Found.\n", VIPER_DSL_PRESS_KEY_STRING);
        fflush(stdout);
        goto exit_cfe_parms_set_failed;
    } 

    /* Step 1 : Find "CFE>" */
    for (ix = 0; ix < DSL_CFE_RETRY_TIMES; ix++) {
        if (diag_uart_tx(tty_desc, VIPER_DSL_CR_STRING) == FAILED) {
            goto exit_cfe_parms_set_failed;
        }
        result = diag_uart_rx_polling(tty_desc, VIPER_DSL_CFE_STRING,
        		                      DSL_CFE_TIMEOUT);
        if (result == TRUE) {
            printf("Found : %s\n", VIPER_DSL_CFE_STRING);
            fflush(stdout);
            break;
        }
    }

    if (ix == DSL_CFE_RETRY_TIMES) {
        printf("[%s] Not Found.\n", VIPER_DSL_CFE_STRING);
        fflush(stdout);
        goto exit_cfe_parms_set_failed;
    }
    msleep(WAIT_DSL_CFE_PROMPT);

    /* Step 2 : Check Set board parameters   */
    if (diag_uart_tx(tty_desc, VIPER_DSL_PRINT_CFE_PARMS_STRING) == FAILED) {
        goto exit_cfe_parms_set_failed;
    }    
    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("Send bdpm [%s]\n", VIPER_DSL_PRINT_CFE_PARMS_STRING);
        fflush(stdout);
    }

    if (diag_uart_tx(tty_desc, VIPER_DSL_CR_STRING) == FAILED) {
        goto exit_cfe_parms_set_failed;
    }
    sprintf(linux_ip_str, "%s.%d:ffffff00", VIPER_IOS_DSL_SUBNET_STR,
            VIPER_IOS_MODULE_IP_ADDR);
    /* Step 3: Check Board IP */
    for (ix = 0; ix < DSL_CFE_RETRY_TIMES; ix++) {
        result = diag_uart_rx_polling(tty_desc, linux_ip_str,
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

    sprintf(linux_ip_str, "%s.%d", VIPER_IOS_DSL_SUBNET_STR,
            VIPER_IOS_HOST_IP_ADDR);
    /* Step 4: Check Host IP */
    for (ix = 0; ix < DSL_CFE_RETRY_TIMES; ix++) {
        result = diag_uart_rx_polling(tty_desc, linux_ip_str,
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

    sprintf(linux_ip_str, "%s.%d", VIPER_IOS_DSL_SUBNET_STR,
            VIPER_IOS_GATEWAY_IP_ADDR);
    /* Step 5: Check Gateway IP */
    for (ix = 0; ix < DSL_CFE_RETRY_TIMES; ix++) {
        result = diag_uart_rx_polling(tty_desc, linux_ip_str,
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
    dsl_img_filename = getenv(IOS_IMAGE_NAME);

    if (dsl_img_filename == NULL || !strcmp("none", dsl_img_filename)) {
        printf("%s: Env variable %s is not set\n", __func__, IOS_IMAGE_NAME);
        sprintf(linux_file_str, "%s%s",VIPER_IOS_HOST_FIRMWARE_FOLDER_STRING,
                VIPER_IOS_VADSL_FIRMWARE_STRING);
        printf("Default IOS Image Filename: %s\n", VIPER_IOS_VADSL_FIRMWARE_STRING);
    } else {
        printf("IOS Image Filename: %s\n", dsl_img_filename);
        sprintf(linux_file_str, "%s%s", VIPER_IOS_HOST_FIRMWARE_FOLDER_STRING,
                dsl_img_filename);
    }
    
    for (ix = 0; ix < DSL_CFE_RETRY_TIMES; ix++) {
        result = diag_uart_rx_polling(tty_desc, linux_file_str,
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
        if (diag_uart_tx(tty_desc, VIPER_DSL_CR_STRING) == FAILED) {
            goto exit_cfe_parms_set_failed;
        }
        result = diag_uart_rx_polling(tty_desc, VIPER_DSL_COMMAND_STATUS_STRING,
        		                      DSL_CFE_COMMAND_STATUS_TIMEOUT);
        if (result == TRUE) {
            printf("Found : %s\n", VIPER_DSL_COMMAND_STATUS_STRING);
            fflush(stdout);
            break;
        }
        msleep(WAIT_DSL_CFE_COMMAND_STATUS);
    }

    if (ix == DSL_CFE_RETRY_TIMES) {
        printf("[%s] Not Found.\n", VIPER_DSL_COMMAND_STATUS_STRING);
        fflush(stdout);
        goto exit_cfe_parms_set_failed;
    }
    msleep(WAIT_DSL_CFE_COMMAND_STATUS);

    /* Step 9 : Find "CFE>" */
    for (ix = 0; ix < DSL_CFE_RETRY_TIMES; ix++) {
        if (diag_uart_tx(tty_desc, VIPER_DSL_CR_STRING) == FAILED) {
            goto exit_cfe_parms_set_failed;
        }
        result = diag_uart_rx_polling(tty_desc, VIPER_DSL_CFE_STRING,
        		                      DSL_CFE_TIMEOUT);
        if (result == TRUE) {
            printf("Found : %s\n", VIPER_DSL_CFE_STRING);
            fflush(stdout);
            break;
        } 
    }

    if (ix == DSL_CFE_RETRY_TIMES) {
        printf("[%s] Not Found.\n", VIPER_DSL_CFE_STRING);
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
 * Function: restore_cfe_ios_param 
 *
 * Description: This function restore cfe ios param.
 *
 * Input : None
 *
 * Output: PASSED/FAILED.
 *
 *******************************************************************************
 */
static int restore_cfe_ios_param (void)
{
    int retval = FAILED;

    /* Step1: Reset */
    xdsl_bcm63168_reset();

    /* Step2: Set CFE IPs and Filename */ 
    if (xdsl_cfe_ios_parms_set() != PASSED) {
        printf("%s fail\n", __FUNCTION__);
        return (retval);
    }

    /* Step3: Reset */
    xdsl_bcm63168_reset();
    if (xdsl_confirm_cfe_ios_parms_set() == PASSED) {
        printf("\nRestore CFE IOS Param Passed\n");
        retval = PASSED;
    } else  {
        printf("\nRestore CFE IOS Param Failed\n");
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

    if ((retval = bcm63168_en_wp_spi_flash_reg()) != PASSED) {
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

    if ((retval = bcm63168_dis_wp_spi_flash_reg()) != PASSED) {
        printf("%s fail\n", __FUNCTION__);
        return (retval);
    }


    return (retval);
}

/*******************************************************************************
 *
 * Function: viper_dsl_env_setup
 *
 * Description: This function set up ethernet, DHCP and tftp server
 *
 * Input : None
 *
 * Output: None
 *
 *******************************************************************************
 */
void viper_dsl_env_setup (void)
{
    system (VIPER_ETH_DSL_DOWN);

    /* Release MRV 1512 reset */
    marvell_1512_reset();

    /* Set up ethernet up */
    system (VIPER_ETH_DSL_UP);
    system (VIPER_ETH_DSL_SET_IP);
    printf("Set up DHCP and TFTP for download DSL firmware\n");
    system (VIPER_DSL_KILL_DHCPD);
    system (VIPER_DSL_KILL_OPENTFTP);
    system (VIPER_DSL_DHCPD);
    system (VIPER_DSL_OPENTFTP);
}





/*-------------------------------------------------
 * $Log: diag_xdsl_test.c,v $
 * Revision 1.4  2018/09/21 02:48:54  harrchan
 * Merge viper DSL to the main trunk (CSCvm57542)
 *
 * Revision 1.3  2018/08/31 03:59:30  chieyang
 * Add SPI flash utility, show memory size and xdsl test modification. Merge from viper-branch2
 *
 * Revision 1.2  2018/08/06 02:31:51  harrchan
 * Merge viper E2E to the main trunk (CSCvk28469)
 *
 * Revision 1.1.2.9  2018/07/06 02:54:08  harrchan
 * Add enhance error message
 *
 * Revision 1.1.2.8  2018/07/03 05:38:55  harrchan
 * Follow the coding rule to clean up code
 *
 * Revision 1.1.2.7  2018/06/27 06:27:52  harrchan
 * Follow the coding rule to clean up code
 *
 * Revision 1.1.2.6  2018/06/25 07:02:35  olin2
 * Remove Viper-Intel P0 DSL support
 *
 * Revision 1.1.2.5  2018/05/21 08:42:36  olin2
 * Support DSL LED on/off utility
 *
 * Revision 1.1.2.4  2018/05/09 07:11:26  olin2
 * 1. Move GE and DSL init to the beginning. 2. Add has GE1. 3. Show cookie info
 *
 * Revision 1.1.2.3  2018/04/17 02:41:55  olin2
 * set dhcp and tftp
 *
 * Revision 1.1.2.2  2018/04/16 08:41:44  olin2
 * Support DSL test
 *
 * Revision 1.1.2.1  2018/02/27 08:06:48  harrchan
 * Initial viper application code base
 *
 *
 * $Endlog$
 *-------------------------------------------------
 */
