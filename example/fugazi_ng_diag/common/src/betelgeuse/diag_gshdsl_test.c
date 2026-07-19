/* $Id: diag_gshdsl_test.c,v 1.2 2019/01/10 06:36:22 wilbhuan Exp $ 
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/betelgeuse/diag_gshdsl_test.c,v $
 *------------------------------------------------------------------
 * 
 * diag_gshdsl_test.c
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
#include "platform_cookie.h"
#include "diag_temp_sensor_util.h"
#include "diag_rbcp_test.h"
#include "rbcp_lib.h"
#include "diag_dsl_lib.h"
#include "diag_dsl_test.h"
#include "diag_gshdsl_lib.h"
#include "diag_gshdsl_test.h"
#include "diag_gshdsl_util.h"
#include "diag_uart_lib.h"
#include "linux_api.h"


/*******************************************************************************
 *                             Function Prototypes                             *
 *******************************************************************************
 */
static int diag_gshdsl_main_tests(int);
static int diag_gshdsl_util(int);
int plat_setup_rbcp_ge_env(void);
static long diag_gshdsl_boot_image(int);

/*******************************************************************************
 *                              Global Variables                               *
 *******************************************************************************
 */
extern int do_all_menu_items(struct menuinfo *);
extern int get_ctrl_plane_sgmii_port (void);
extern int setup_eth_dev(char *, int *socket);

/*******************************************************************************
 *                                   Menus                                     *
 *******************************************************************************
 */

/*=========================================
 * GSHDSL Main Tests menu items
 *=========================================
 */
static submenu_xtable_t gshdsl_main_tests_submenu_table[] = {

    { "RBCP Registration Test", (type_t(*)())diag_rbcp_registration_test, 0,
          MF_CONTINUOUS | MF_DOALL, (type_t(*)())0, 0, (type_t(*)())0, 0 },
    { "RBCP SPI Test", (type_t(*)())diag_rbcp_spi_flash_test, 0,
          MF_CONTINUOUS | MF_DOALL, (type_t(*)())0, 0, (type_t(*)())0, 0 },
    { "RBCP ECC Memory Test", (type_t(*)())diag_rbcp_ecc_test, 0,
          MF_CONTINUOUS | MF_DOALL, (type_t(*)())0, 0, (type_t(*)())0, 0 },
    { "RBCP Memory Test", (type_t(*)())diag_rbcp_memory_test, 0,
          MF_CONTINUOUS | MF_DOALL, (type_t(*)())0, 0, (type_t(*)())0, 0 },
    { "RBCP ETSEC1 RMII loopback Test", (type_t(*)())diag_rbcp_etsec1_rmii_lpbk_test, 0,
          MF_CONTINUOUS | MF_DOALL, (type_t(*)())0, 0, (type_t(*)())0, 0 },
    { "RBCP ETSEC3 RMII loopback Test", (type_t(*)())diag_rbcp_etsec3_rmii_lpbk_test, 0,
          MF_CONTINUOUS | MF_DOALL, (type_t(*)())0, 0, (type_t(*)())0, 0 },
    { "RBCP UCC1 RMII loopback Test", (type_t(*)())diag_rbcp_ucc1_rmii_lpbk_test, 0,
          MF_CONTINUOUS | MF_DOALL, (type_t(*)())0, 0, (type_t(*)())0, 0 },
    { "RBCP UCC5 RMII loopback Test", (type_t(*)())diag_rbcp_ucc5_rmii_lpbk_test, 0,
          MF_CONTINUOUS | MF_DOALL, (type_t(*)())0, 0, (type_t(*)())0, 0 },
    { "RBCP UCC3 UTOPIA loopback Test", (type_t(*)())diag_rbcp_ucc3_utopia_lpbk_test, 0,
          MF_CONTINUOUS | MF_DOALL, (type_t(*)())0, 0, (type_t(*)())0, 0 },
    { "RBCP LED Test", (type_t(*)())diag_rbcp_led_test, 0,
     MF_CONTINUOUS | MF_DOALL, (type_t(*)())0, 0, (type_t(*)())0, 0 },
    { "Terminate G.SHDSL RBCP", (type_t(*)())diag_terminate_rbcp_test, TRUE,
      0, (type_t(*)())0, 0, (type_t(*)())0, 0 },
    { "Console switch to GSHDSL", (type_t(*)())gshdsl_console_switch, 0,
      0, (type_t(*)())0, 0, (type_t(*)())0, 0 },
    { "RBCP LED Util", (type_t(*)())gshdsl_led_utils_menu, 0,
      0, (type_t(*)())0, 0, (type_t(*)())0, 0 },
    { "RBCP Check Flash Protection", (type_t(*)())plat_rbcp_spi_flash_protect, 0,
      0, (type_t(*)())0, 0, (type_t(*)())0, 0 },

};

#define GSHDSL_MAIN_TESTS_SUBMENU_TABLE_SIZE (sizeof(gshdsl_main_tests_submenu_table) / \
                                              sizeof(submenu_xtable_t))

/* 
 * Primary & secondary submenu items (filled in from xtable)
 */
static mitem_t gshdsl_main_tests_primary_items[GSHDSL_MAIN_TESTS_SUBMENU_TABLE_SIZE +
                                               MAX_BASE_ITEMS];
static mitem_t gshdsl_main_tests_secondary_items[GSHDSL_MAIN_TESTS_SUBMENU_TABLE_SIZE +
                                                 MAX_BASE_ITEMS];
menuinfo_t gshdsl_main_subtest_menu = {
    "%s Subtest Menu",
    0,                          /* mtparam added by init_empty_menu */
    (PFT) show_endnote,         /* notes missing WICs in combos */
    0,                          /* use generic prompt */
    0,                          /* size (bumped by add_menu_item() */
    gshdsl_main_tests_primary_items,
};

menuinfo_t *gshdsl_main_submenup = &gshdsl_main_subtest_menu;

/*=========================================
 * Utilities menu items
 *=========================================
 */
static submenu_xtable_t diag_gshdsl_util_submenu_table[] = {
    {"GSHDSL Reset",          (PFT)gshdsl_util_reset, 0, 0,
      (type_t(*)())0, 0,   (type_t(*)())0,          0},
    {"Terminate RBCP",          (PFT)diag_terminate_rbcp_test, 0, 0,
      (type_t(*)())0, 0,   (type_t(*)())0,          0},
};

#define GSHDSL_UTILS_SUBMENU_TABLE_SZ (sizeof(diag_gshdsl_util_submenu_table) / sizeof(submenu_xtable_t))

static mitem_t diag_gshdsl_util_primary_items[GSHDSL_UTILS_SUBMENU_TABLE_SZ +
                                          MAX_BASE_ITEMS];
static mitem_t diag_gshdsl_util_secondary_items[GSHDSL_UTILS_SUBMENU_TABLE_SZ +
                                            MAX_BASE_ITEMS];

char gshdslutiltitle[50];

menuinfo_t gshdsl_util_submenu = {
    gshdslutiltitle,
    0,                              /* mtparam added by init_empty_menu */
    (PFT)menu_show_dflags,          /* notes missing WICs in combos */
    0,                              /* use generic prompt */
    0,                              /* size (bumped by add_menu_item() */
    diag_gshdsl_util_primary_items,
};

menuinfo_t *gshdsl_util_submenup = &gshdsl_util_submenu;

/*=========================================
 * Main menu items
 *=========================================
 */
static submenu_xtable_t gshdsl_tests_submenu_table[] = {
    {"GSHDSL Utilities",      (PFT)diag_gshdsl_util,        0,   0,
     (type_t(*)())0, 0,         (type_t(*)())diag_gshdsl_util,    0},
    {"Boot Module Image",       (PFT)diag_gshdsl_boot_image, 0, MM_3,
     (long(*)())0, 0,            (long(*)())0, 0 },
    {"GSHDSL Main Tests",      (PFT)diag_gshdsl_main_tests,    0,  MM_3,
     (type_t(*)())0, 0,         (PFT)diag_gshdsl_main_tests,        TRUE},
    {"GSHDSL Switch Console", (PFT)gshdsl_console_switch,    0,   0,
     (type_t(*)())0, 0,         (type_t(*)())0,                     0},
};

#define GSHDSL_TESTS_SUBMENU_TABLE_SIZE \
        (sizeof(gshdsl_tests_submenu_table) / sizeof(submenu_xtable_t))
        
static mitem_t gshdsl_tests_primary_items[GSHDSL_TESTS_SUBMENU_TABLE_SIZE + MAX_BASE_ITEMS];
static mitem_t gshdsl_tests_secondary_items[GSHDSL_TESTS_SUBMENU_TABLE_SIZE + MAX_BASE_ITEMS];

menuinfo_t gshdsl_subtest_menu = {
    "%s Subtest Menu",
    0,                          /* mtparam added by init_empty_menu */
    (PFT) show_endnote,         /* notes missing WICs in combos */
    0,                          /* use generic prompt */
    0,                          /* size (bumped by add_menu_item() */
    gshdsl_tests_primary_items,
};

menuinfo_t *gshdsl_submenup = &gshdsl_subtest_menu;

/*******************************************************************************
 *
 * Function: diag_gshdsl_test
 *
 * Description: This function is the main entrance for gshdsl module test .
 *
 * Input: TRUE/FALSE
 *
 * Output: PASSED
 *
 *******************************************************************************
 */
int diag_gshdsl_test (boolean gshdsl_test_items_executed)
{
    const int maxlen = PLAT_DSL_MAX_LENGTH;
    char tty_dev[maxlen];
    char *tname = "GSHDSL";
    int slot = 0;

    testname(tname);

    /* Set gshdsl tty configuration */
    snprintf(tty_dev, maxlen-1, PLAT_DSL_UART_DEV_STR);
    if (plat_uart_setup(tty_dev) == FAILED) {
        printf("\nFailed to setup UART\n");
        return (FAILED);
    }

    build_primary_submenu(gshdsl_tests_submenu_table,
                          GSHDSL_TESTS_SUBMENU_TABLE_SIZE, "GSHDSL",
                          &gshdsl_submenup);

    build_secondary_submenu(gshdsl_tests_submenu_table,
                            GSHDSL_TESTS_SUBMENU_TABLE_SIZE,
                            gshdsl_tests_secondary_items);

    /* Setup RBCP GE switch environment and Mac address */
    if (plat_setup_rbcp_ge_env() == FAILED) {
            return (FAILED);
    }
    rbcp_get_mac(slot);
 
    if (gshdsl_test_items_executed) {
        do_all_menu_items(&gshdsl_subtest_menu);
    } else {
        menu(&gshdsl_subtest_menu, gshdsl_tests_secondary_items, '\0');
    }

    return (PASSED);
}

/*******************************************************************************
 *
 *  Function: diag_gshdsl_util
 *
 *  Description: gshdsl Utitlities menu
 *
 *  Input: None 
 *
 *  Returns: PASSED
 *
 *******************************************************************************
 */
static int diag_gshdsl_util (int show_menu)
{
    sprintf(gshdslutiltitle, "GSHDSL Utilities Menu");

    build_primary_submenu(diag_gshdsl_util_submenu_table,
                          GSHDSL_UTILS_SUBMENU_TABLE_SZ,
                          gshdslutiltitle, &gshdsl_util_submenup);

    build_secondary_submenu(diag_gshdsl_util_submenu_table,
                            GSHDSL_UTILS_SUBMENU_TABLE_SZ,
                            diag_gshdsl_util_secondary_items);

    menu(gshdsl_util_submenup, diag_gshdsl_util_secondary_items, '\0');

    return (PASSED);
}

/*******************************************************************************
 *
 * Function   : diag_gshdsl_main_tests
 *
 * Description: Entry function of gshdsl Diag tests and utilities.
 *
 * Inputs     : show gshdsl main test menu option
 *
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
static int diag_gshdsl_main_tests (int show_menu)
{
    char *tname = "GSHDSL RBCP Test";
    testname(tname);

    build_primary_submenu(gshdsl_main_tests_submenu_table,
                          GSHDSL_MAIN_TESTS_SUBMENU_TABLE_SIZE,
                          "GSHDSL RBCP", &gshdsl_main_submenup);
    build_secondary_submenu(gshdsl_main_tests_submenu_table,
                            GSHDSL_MAIN_TESTS_SUBMENU_TABLE_SIZE,
                            gshdsl_main_tests_secondary_items);

    if (show_menu) {
        menu(gshdsl_main_submenup, gshdsl_main_tests_secondary_items, '\0');
    } else {
        do_all_menu_items(gshdsl_main_submenup);
    }

    return (PASSED);
}

 /**********************************************************************
  *
  * Function: diag_gshdsl_boot_image
  *
  * This function interrupts Uboot, download image through TFTP, and
  * load the image
  *
  * Input : show_menu - Not used
  *
  * Output: PASSED/FAILED
  *
  **********************************************************************
  */
 static long diag_gshdsl_boot_image (int show_menu)
 {
    int boot_timeout, ping_timeout, go_timeout;
    const int maxlen = 128;
    char tty_dev[maxlen];
    size_t size = 0;

    if (file_exist(GSHDSL_KERNEL_FILE, &size) == 0) {
    	cterr('f', 0, "G.SHDSL kernel file doesn't exist!!");
        return (FAILED);
    }

     /* reset and un-reset G.SHDSL*/
     gshdsl_util_reset();
     msleep(GSHDSL_POWER_UP_DELAY);

     /* Setup UART toward G.SHDSL board */
     printf("Setup UART...");
     sprintf(tty_dev, GSHDSL_SET_UART);

     /* Polling if bootloader is up, we need to type tftpdnld command
      * through UART interface since G.SHDSL bootloader doesn't boot
      * linux by default. (It boots up SE instead)
      */
     printf("Looking for bootloader prompt (1)...");
     fflush(stdout);

     boot_timeout = GSHDSL_BL_PROMPT_TOUT;
     do {
         /* Transmit New Line */
         plat_tx_uart(tty_dev, GSHDSL_CR_STRING);
         plat_tx_uart(tty_dev, GSHDSL_CR_STRING);
         plat_tx_uart(tty_dev, GSHDSL_CR_STRING);

         if (plat_rx_polling_uart(tty_dev, GSHDSL_BL_PROMPT, 100) == PASSED) {
             printf("OK\n");
             fflush(stdout);
             break;
         }
     } while (boot_timeout--);

     if (boot_timeout <= 0) {
         printf("FAIL\n");
         fflush(stdout);
         cterr('f', 0, "Failed to get '%s' bootloader prompt", GSHDSL_BL_PROMPT);
         return (FAILED);
     }
     msleep(500);


     /* Now, we can do tftp download in Uboot prompt */
     printf("Set ENV ...");
     plat_tx_uart(tty_dev, GSHDSL_SET_GATEWAY);
     msleep(500);
     plat_tx_uart(tty_dev, GSHDSL_CR_STRING);
     msleep(500);
     plat_tx_uart(tty_dev, GSHDSL_SET_IPADDR);
     msleep(500);
     plat_tx_uart(tty_dev, GSHDSL_SET_NETMASK);
     msleep(500);
     plat_tx_uart(tty_dev, GSHDSL_SET_GETWAY);
     msleep(500);
     plat_tx_uart(tty_dev, GSHDSL_SET_SERVERIP);
     msleep(500);
     plat_tx_uart(tty_dev, GSHDSL_SET_ETH1);
     msleep(500);
     plat_tx_uart(tty_dev, GSHDSL_SET_ETH2);
     msleep(500);
     plat_tx_uart(tty_dev, GSHDSL_SET_ETH);
     msleep(500);
     plat_tx_uart(tty_dev, GSHDSL_SET_ETHACT);
     msleep(500);

     /* Now ping the server ip */
     printf("Ping TFTP Server from Backplane ...");
     fflush(stdout);

     plat_tx_uart(tty_dev, GSHDSL_PING_SERVER);
     plat_tx_uart(tty_dev, GSHDSL_CR_STRING);
     ping_timeout = GSHDSL_PING_TOUT;
     do{
         if (plat_rx_polling_uart(tty_dev, GSHDSL_PING_ALIVE, 1000) == PASSED) {
             printf("OK\n");
             fflush(stdout);
             break;
         }
     } while (ping_timeout--);

     if (ping_timeout <= 0) {
         printf("FAIL\n");
         fflush(stdout);
         cterr('f', 0, "Failed to ping TFTP Server");
         return (FAILED);
     }

     /* Now, boot image using TFTP download */
     printf("Loading image ...");
     fflush(stdout);

     plat_tx_uart(tty_dev, GSHDSL_SET_FILENAME);
     plat_tx_uart(tty_dev, GSHDSL_CR_STRING);
     plat_tx_uart(tty_dev, GSHDSL_BOOT_UP_CMD);

     boot_timeout = GSHDSL_BOOT_TOUT;
     do{
         if (plat_rx_polling_uart(tty_dev, GSHDSL_BOOT_DONE, 1000) == PASSED) {
             printf("OK\n");
             msleep(500);
             fflush(stdout);
             break;
         }
     } while (boot_timeout--);

     if (boot_timeout <= 0) {
         printf("FAIL\n");
         fflush(stdout);
         cterr('f', 0, "Failed to load image");
         return (FAILED);
     }


     printf("Booting image ...");
     fflush(stdout);

     plat_tx_uart(tty_dev, GSHDSL_GO_ADDR);

     go_timeout = GSHDSL_GO_TOUT;
     do{
         if (plat_rx_polling_uart(tty_dev, GSHDSL_RBCP_LOOP, 1000) == PASSED) {
             printf("OK\n");
             fflush(stdout);
             break;
         }
     } while (go_timeout--);

     if (go_timeout <= 0) {
         printf("FAIL\n");
         fflush(stdout);
         cterr('f', 0, "Failed to boot up image");
         return (FAILED);
     }
    return (PASSED);
 }

/*-------------------------------------------------
 * $Log: diag_gshdsl_test.c,v $
 * Revision 1.2  2019/01/10 06:36:22  wilbhuan
 * The beginning of Betelgeuse application code.
 *
 *-------------------------------------------------
 */
