/* $Id: ngwic_goldschlager.c,v 1.16 2020/05/22 02:28:30 qingcwan Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/goldschlager/ngwic_goldschlager.c,v $
 *------------------------------------------------------------------------------
 *
 * ngwic_goldschlager.c: This file contains functions for goldschlager NIM.
 *
 * Oct. 2013 - James Lin
 *
 * Copyright (c) 2013-2019 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------------------
 */
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h> /* stat */
#include <sys/time.h>
#include "common.h"
#include "types.h"
#include "menu.h"
#include "defs.h"
#include "setjmps.h"
#include "signals.h"
#include "menu.h"
#include "router_if.h"
#include "error.h"
#include "proto.h"
#include "strings.h"
#include "nvmonvars.h"
#include "dev_ltc4215.h"
#include "oir_ltc4215_api.h"
#include "platform_i2c.h"
#include "ngwic_goldschlager.h"
#include "ngio.h"
#include "pca.h"
#include "slot.h"
#include "cookie_4.h"
#include "plat_defs.h"
#include "dash_fpga.h"
#include "common_utils.h"
#include "linux_api.h"
#include "hwic_slot.h"
#include <stdlib.h>
#include "bcm63268_lib.h"
#include "ngwic_goldschlager_comm.h"
#include "queryflags.h"
#include "cross_platform.h" /* board_type */
#include "platform_fru.h"
#include "linux_ntwk.h" /* tftp_get */
#ifdef TACHI
#include "diag_console_util.h"
#endif

/*******************************************************************************
 *                             Function Prototypes                             *
 *******************************************************************************
 */
static int ltc4215_register_test (void);
static int ltc4215_led_test(void);
static int oir_ltc4215_tests(int);
static int ltc4215_reg_write(void);
static int ltc4215_reg_read(void);
static int pri_intf_rdy_chk(void);
static int goldschlager_check_sku_type(void);
static int pca9557_reg_dump_util(void);
static int pca9557_reg_read_util(void);
static int pca9557_reg_write_util(void);
static int goldschlager_power_off (void);
static int goldschlager_pwr_off (void);
static int goldschlager_pwr_on (void);
static int goldschlager_pwr_cycle (void);
static int goldschlager_utils(void);
static int goldschlager_param_test(int);
static int goldschlager_ioe_reg_test(void);
static int goldschlager_console_switch(void);
static int goldschlager_uart_test (void);
static int goldschlager_bcm963268_reset (void);
static int goldschlager_bcm63268_led_test (void);
static int goldschlager_init_bcm63268 (void);
static int goldschlager_config_bcm63268 (void);
static int goldschlager_get_bcm63268_config (void);
static int goldschlager_get_bcm63268_version (void);
static int goldschlager_connection_start (void);
static int goldschlager_connection_stop (void);
static int goldschlager_get_xdsl_mib_info (void);
static int goldschlager_get_xtm_bonding_info (void);
static int goldschlager_get_xdsl_info (void);
static int goldschlager_get_connection_info (void);
static int goldschlager_bcm63268_volt_normal (void);
static int goldschlager_bcm63268_volt_high (void);
static int goldschlager_bcm63268_volt_low (void);
static int goldschlager_nim_tests(int);
static int goldschlager_bootup(void);
static int goldschlager_bootflash_test (void);
static int goldschlager_mrvl1512_reg_test (void);
static int goldschlager_bcm63268_dram_test (void);
static int goldschlager_do_showtime (void);
static int goldschlager_do_set_tone (void);
static int goldschlager_do_send_all_tone (void);
static int goldschlager_do_idle_listen (void);
static int goldschlager_select_test_options (void);
static int util_goldschlager_uart_baud_rate_set(void);
static int goldschlager_boot_sel_pin_test(void);
static int goldschlager_bcm63268_show_profile(void);
static int goldschlager_bcm63268_show_spi_flash_reg (void);

static void (*goldschlager_saved_diag_exec)(void) = NULL;
static char pca_buff0[256];
static void *oir_if;

static speed_t slot1_uart = GOLDSCHLAGER_B9600;
static speed_t slot2_uart = GOLDSCHLAGER_B9600;
static speed_t slot3_uart = GOLDSCHLAGER_B9600;
static uart_baud_info goldschlager_uart_baud[] = {
    {"115200",  B115200},
    {"9600",    B9600}
};

int goldschlager_test_slot = 1;
unsigned short goldschlager_sku = 0xFFFF;
ushort goldschlager_board_id;
    
/* addr of 8bit 0x38H >> 1; 16bit 0x48H >> 1 */
static n2g_i2c_if_t pca_i2c[] = {
    {
        .i2c_dev = 0x1C,  
    },
    {
        .i2c_dev = 0x24,
    },
};

static reg_info_t pca9557_reg_tbl[]=
{
    {"Input port",                  PCA9557_IN_PORT_REG,
     (SAVE_RESTORE | REG_ACCESS),   {0},
     0x00, 0x00},
    {"Output port",                 PCA9557_OUT_PORT_REG,
     (SAVE_RESTORE | REG_ACCESS),   {0},
     0xFF, 0xFF},
    {"Polarity Inversion port",     PCA9557_POLAR_INV_P_REG,
     (SAVE_RESTORE | REG_ACCESS),   {0},
     0xFF, 0x00},
    {"Configuration port",          PCA9557_CFG_PORT_REG,
     (SAVE_RESTORE | REG_ACCESS),   {0},
     0xFF, 0xFF},
};

/*******************************************************************************
 *                              Global Variables                               *
 *******************************************************************************
 */
extern uint bcm_op_mode;
extern uint bcm_idle_listen_params;
extern uint bcm_line_id;
extern int do_all_menu_items(struct menuinfo *);
extern unsigned int fru_table_offset;

static struct ngio_intf_t *goldschlager_wic_iface;

/*******************************************************************************
 *                                   Menus                                     *
 *******************************************************************************
 */

#define MM_1    (MF_CONTINUOUS | MF_DOGRP)
#define MM_2    (MM_1 | MF_DOALL)
#define MM_3    (MM_2 | MF_SHOW_ERRCOUNT)
#define FLAGS   MF_CONTINUOUS

/*=========================================
 * LTC4215 OIR menu items
 *=========================================
 */
static submenu_xtable_t oir_submenu_table[] = {
    {"LTC4215 OIR reg. Read util",      (PFT)ltc4215_reg_read,       0,
     MF_SHOW_ERRCOUNT,
     (type_t(*)())0,         0,         (type_t(*)())0,              0},
    {"LTC4215 OIR reg. Write util",     (PFT)ltc4215_reg_write,      0,
     MF_SHOW_ERRCOUNT,
     (type_t(*)())0,         0,         (type_t(*)())0,              0},
    {"LTC4215 OIR Register Test",       (PFT)ltc4215_register_test,  0,
     MF_CONTINUOUS | MF_DOALL | MF_SHOW_ERRCOUNT,
     (type_t(*)())0,         0,         (type_t(*)())0,              0},
    {"LTC4215 OIR LED Test",            (PFT)ltc4215_led_test,       0,
     MF_CONTINUOUS | MF_DOALL | MF_SHOW_ERRCOUNT,
     (type_t(*)())0,         0,         (type_t(*)())0,              0},
};

#define OIR_SUBMENU_TABLE_SIZE (sizeof(oir_submenu_table) / \
                                sizeof(submenu_xtable_t))

/* 
 * Primary & secondary submenu items (filled in from xtable)
 */
static mitem_t oir_tests_primary_items[OIR_SUBMENU_TABLE_SIZE + 
                                       MAX_BASE_ITEMS];
static mitem_t oir_tests_secondary_items[OIR_SUBMENU_TABLE_SIZE +
                                         MAX_BASE_ITEMS];

static menuinfo_t oir_subtest_menu = {
    "%s Subtest Menu",
    0,                                /* mtparam added by init_empty_menu */
    (PFT)show_endnote,                /* notes missing WICs in combos */
    0,                                /* use generic prompt */
    0,                                /* size (bumped by add_menu_item()) */
    oir_tests_primary_items,
};

static menuinfo_t *oir_submenup = &oir_subtest_menu;

/*=========================================
 * Goldschlager NIM Tests menu items
 *=========================================
 */
static submenu_xtable_t gs_nim_submenu_table[] = {
    {"BCM63168 DRAM Test",      (PFT)goldschlager_bcm63268_dram_test,   0, MM_3,
     (type_t(*)())0,         0,         (type_t(*)())0,              0},
    {"BCM63168 NOR Flash Test", (PFT)goldschlager_bootflash_test,       0, MM_3,
     (type_t(*)())0,         0,         (type_t(*)())0,              0},
    {"Marvell1512 Register Test", (PFT)goldschlager_mrvl1512_reg_test,  0, MM_3,
     (type_t(*)())0,         0,         (type_t(*)())0,              0},
    {"Boot Select Pin Test",    (PFT)goldschlager_boot_sel_pin_test,    0, MM_3,
     (type_t(*)())0,         0,         (type_t(*)())0,              0},
};

#define GS_NIM_SUBMENU_TABLE_SIZE (sizeof(gs_nim_submenu_table) / \
                                sizeof(submenu_xtable_t))

/* 
 * Primary & secondary submenu items (filled in from xtable)
 */
static mitem_t gs_nim_tests_primary_items[GS_NIM_SUBMENU_TABLE_SIZE + 
                                          MAX_BASE_ITEMS];
static mitem_t gs_nim_tests_secondary_items[GS_NIM_SUBMENU_TABLE_SIZE +
                                            MAX_BASE_ITEMS];

static menuinfo_t gs_nim_subtest_menu = {
    "%s Subtest Menu",
    0,                                /* mtparam added by init_empty_menu */
    (PFT)show_endnote,                /* notes missing WICs in combos */
    0,                                /* use generic prompt */
    0,                                /* size (bumped by add_menu_item()) */
    gs_nim_tests_primary_items,
};

static menuinfo_t *gs_nim_submenup = &gs_nim_subtest_menu;


/*=========================================
 * Utilities menu items
 *=========================================
 */
static submenu_xtable_t goldschlager_utils_submenu_table[] = {
    {"Power off goldschlager NIM",      (PFT)goldschlager_power_off,    0,   0,
     (type_t(*)())0, 0,         (type_t(*)())0,          0},
    {"Power on goldschlager NIM",       (PFT)goldschlager_pwr_on,       0,   0,
     (type_t(*)())0, 0,         (type_t(*)())0,          0},
    {"Power cycle goldschlager NIM",    (PFT)goldschlager_pwr_cycle,    0,   0,
     (type_t(*)())0, 0,         (type_t(*)())0,          0},
    {"UART Test",                       (PFT)goldschlager_uart_test,    0,   0,
     (type_t(*)())0, 0,         (type_t(*)())0,          0},
    {"LTC4215 Register Read",           (PFT)ltc4215_reg_read,          0,   0,
     (type_t(*)())0, 0,         (type_t(*)())0,          0},
    {"LTC4215 Register Write",          (PFT)ltc4215_reg_write,         0,   0,
     (type_t(*)())0, 0,         (type_t(*)())0,          0},       
    {"IO Expander(PCA9557) Reg. Dump",  (PFT)pca9557_reg_dump_util,     0,   0,
     (type_t(*)())0, 0,         (type_t(*)())0,          0}, 
    {"IO Expander(PCA9557) Reg. Read",  (PFT)pca9557_reg_read_util,     0,   0,
     (type_t(*)())0, 0,         (type_t(*)())0,          0}, 
    {"IO Expander(PCA9557) Reg. Write", (PFT)pca9557_reg_write_util,    0,   0,
     (type_t(*)())0, 0,         (type_t(*)())0,          0},  
    {"BCM963268 Reset",             (PFT)goldschlager_bcm963268_reset,  0,  0,
      (type_t(*)())0, 0,   (type_t(*)())0,          0},
    {"Initialize BCM63268",         (PFT)goldschlager_init_bcm63268,    0,  0,
      (type_t(*)())0, 0,   (type_t(*)())0,          0},
    {"Configure BCM63268",          (PFT)goldschlager_config_bcm63268,  0,  0,
      (type_t(*)())0, 0,   (type_t(*)())0,          0},
    {"Get BCM63268 configuration",  (PFT)goldschlager_get_bcm63268_config,0,0,
      (type_t(*)())0, 0,   (type_t(*)())0,          0},
    {"Get BCM63268 version",        (PFT)goldschlager_get_bcm63268_version,0,0,
      (type_t(*)())0, 0,   (type_t(*)())0,          0},
    {"Connection Start",            (PFT)goldschlager_connection_start, 0,  0,
      (type_t(*)())0, 0,   (type_t(*)())0,          0},
    {"Connection Stop",             (PFT)goldschlager_connection_stop,  0,  0,
      (type_t(*)())0, 0,   (type_t(*)())0,          0},
    {"Get xDSL Mib Info",           (PFT)goldschlager_get_xdsl_mib_info,0,  0,
      (type_t(*)())0, 0,   (type_t(*)())0,          0},
    {"Get xDSL Bonding Info",       (PFT)goldschlager_get_xtm_bonding_info,0,  0,
      (type_t(*)())0, 0,   (type_t(*)())0,          0},
    {"Get xDSL Info",               (PFT)goldschlager_get_xdsl_info,    0,  0,
      (type_t(*)())0, 0,   (type_t(*)())0,          0},
    {"Get Connection Info",         (PFT)goldschlager_get_connection_info,0,0,
      (type_t(*)())0, 0,   (type_t(*)())0,          0},
    {"Voltage Normal",              (PFT)goldschlager_bcm63268_volt_normal,0,0,
      (type_t(*)())0, 0,   (type_t(*)())0,          0},
    {"Voltage High",                (PFT)goldschlager_bcm63268_volt_high,0, 0,
      (type_t(*)())0, 0,   (type_t(*)())0,          0},
    {"Voltage Low",                 (PFT)goldschlager_bcm63268_volt_low,0,  0,
      (type_t(*)())0, 0,   (type_t(*)())0,          0},
    {"LED Test",                    (PFT)goldschlager_bcm63268_led_test,0,  0,
      (type_t(*)())0, 0,   (type_t(*)())0,          0},
    {"BCM63268 get xDSL Profile",    (PFT)goldschlager_bcm63268_show_profile,0,0,
      (type_t(*)())0, 0,   (type_t(*)())0,          0},
    {"Set GS UART Baud Rate",    (PFT)util_goldschlager_uart_baud_rate_set,0,0,
      (type_t(*)())0, 0,   (type_t(*)())0,          0},
    {"Display BCM63268 SPI Flash Registers",
      (PFT)goldschlager_bcm63268_show_spi_flash_reg, 0, 0,
      (type_t(*)())0, 0,   (type_t(*)())0,          0},
};

#define goldschlager_UTILS_SUBMENU_TABLE_SZ (sizeof(goldschlager_utils_submenu_table) / sizeof(submenu_xtable_t))

static mitem_t gold_utils_primary_items[goldschlager_UTILS_SUBMENU_TABLE_SZ + 
                                        MAX_BASE_ITEMS];
static mitem_t gold_utils_secondary_items[goldschlager_UTILS_SUBMENU_TABLE_SZ + 
                                          MAX_BASE_ITEMS];

char goldschlagerutiltitle[50];

menuinfo_t goldschlager_util_submenu = {
    goldschlagerutiltitle,
    0,                              /* mtparam added by init_empty_menu */
    (PFT)menu_show_dflags,          /* notes missing WICs in combos */
    0,                              /* use generic prompt */
    0,                              /* size (bumped by add_menu_item() */
    gold_utils_primary_items,
};

menuinfo_t *goldschlager_util_submenup = &goldschlager_util_submenu;

/*=========================================
 * Showtime and PT menu items
 *=========================================
 */
static submenu_xtable_t pt_submenu_table[] = {
    {"Do Showtime",         (PFT)goldschlager_do_showtime,          0,
     MF_CONTINUOUS | MF_SHOW_ERRCOUNT,
     (type_t(*)())0,         0,         (type_t(*)())0,              0},
    {"Do Set Tones",        (PFT)goldschlager_do_set_tone,          0,
     MF_SHOW_ERRCOUNT,
     (type_t(*)())0,         0,         (type_t(*)())0,              0},
    {"Do Send All Tones",   (PFT)goldschlager_do_send_all_tone,     0,
     MF_SHOW_ERRCOUNT,
     (type_t(*)())0,         0,         (type_t(*)())0,              0},
    {"Do Idle Listen",      (PFT)goldschlager_do_idle_listen,       0,
     MF_SHOW_ERRCOUNT,
     (type_t(*)())0,         0,         (type_t(*)())0,              0},
    {"Set Idle Mode",       (PFT)goldschlager_connection_stop,      0,
     MF_SHOW_ERRCOUNT,
     (type_t(*)())0,         0,         (type_t(*)())0,              0},     
    {"Select test options", (PFT)goldschlager_select_test_options,  0,
     MF_SHOW_ERRCOUNT,
     (type_t(*)())0,         0,         (type_t(*)())0,              0},     
};

#define PT_SUBMENU_TABLE_SIZE (sizeof(pt_submenu_table) / \
                                sizeof(submenu_xtable_t))

/* 
 * Primary & secondary submenu items (filled in from xtable)
 */
static mitem_t pt_tests_primary_items[PT_SUBMENU_TABLE_SIZE + 
                                       MAX_BASE_ITEMS];
static mitem_t pt_tests_secondary_items[PT_SUBMENU_TABLE_SIZE +
                                         MAX_BASE_ITEMS];

static menuinfo_t pt_subtest_menu = {
    "%s Subtest Menu",
    0,                                /* mtparam added by init_empty_menu */
    (PFT)show_endnote,                /* notes missing WICs in combos */
    0,                                /* use generic prompt */
    0,                                /* size (bumped by add_menu_item()) */
    pt_tests_primary_items,
};

static menuinfo_t *pt_submenup = &pt_subtest_menu;


/*=========================================
 * Main menu items
 *=========================================
 */
static submenu_xtable_t main_menu_table[] = {
    {"Goldschlager Utilities",      (PFT)goldschlager_utils,        0,   0,
     (type_t(*)())0, 0,         (type_t(*)())goldschlager_utils,    0},
    {"Goldschlager Bootup",         (PFT)goldschlager_bootup,       0,  MM_3,
     (type_t(*)())0, 0,         (type_t(*)())0,                     0}, 
    {"LTC4215 OIR Test",            (PFT)oir_ltc4215_tests,         0,  MM_3,
     (type_t(*)())0, 0,         (PFT)oir_ltc4215_tests,             TRUE},
    {"PCA9557 Register Test",       (PFT)goldschlager_ioe_reg_test, 0,  MM_3,
     (type_t(*)())0, 0,         (type_t(*)())0,                     0}, 
    {"Goldschlager NIM Tests",      (PFT)goldschlager_nim_tests,    0,  MM_3,
     (type_t(*)())0, 0,         (PFT)goldschlager_nim_tests,        TRUE},
    {"Showtime & Parametric Tests", (PFT)goldschlager_param_test,       0,   0,
     (type_t(*)())0, 0,         (type_t(*)())goldschlager_param_test,   TRUE},
    {"LED Test",                (PFT)goldschlager_bcm63268_led_test,0,   0,
     (type_t(*)())0, 0,         (type_t(*)())0,                     0},
    {"Goldschlager Switch Console", (PFT)goldschlager_console_switch,    0,   0,
     (type_t(*)())0, 0,         (type_t(*)())0,                     0},
};

#define MAIN_MENU_TABLE_SIZE \
        (sizeof(main_menu_table) / sizeof(submenu_xtable_t))
        
static mitem_t main_menu_primary_items[MAIN_MENU_TABLE_SIZE + MAX_BASE_ITEMS];
static mitem_t main_menu_secondary_items[MAIN_MENU_TABLE_SIZE + MAX_BASE_ITEMS];

static struct menuinfo maindiag = {
    "Goldschlager Main Menu",   /* title */
    0,                          /* title string added by init_empty_menu */
    (PFT)menu_show_dflags,      /* shows major flags */
    0,                          /* generic prompt */
    0,                          /* size -- bumped by add_menu_item() */
    main_menu_primary_items,
};
static struct menuinfo *maindiagp = &maindiag;

/*******************************************************************************
 *
 * Function: slot_get_bd_pid
 *
 * Description: This function will return the Product ID(PID) of NIM module.
 *
 * Inputs : eeprom_data - pointer to eeprom data.
 *          board_pid - pointer to NIM product id.
 *
 * Returns : PID of board.
 *******************************************************************************
 */
static int slot_get_bd_pid (uchar *eeprom_data, char *board_pid, uchar *num_byte)
{
    uchar *data_ptr;

    if (eeprom_data[0] == CURRENT_FORMAT_VERSION) {
        if ((data_ptr = (uchar *)search_type_ret_addr_of_first_data
            (eeprom_data, PRODUCT_ID, num_byte, FALSE)) == NULL) {
            sprintf((char *)board_pid, (char *)"NO PID");
        } else {
            memcpy(board_pid, data_ptr, *num_byte);
        }
        return(PASSED);
    } else {
        sprintf((char *)board_pid, (char *)"NO PID");
        return(FAILED);
    }
}

/*******************************************************************************
 *
 * Function: goldschlager_cleanup()
 *
 * Description: This function performs the cleanup task before exiting
 *              the test.
 *
 * Input:  None
 *
 * Output: None
 *******************************************************************************
 */
static void goldschlager_cleanup (void)
{
    assert(goldschlager_wic_iface);

    if (goldschlager_saved_diag_exec) {
        pre_diag_exec = goldschlager_saved_diag_exec;
        goldschlager_saved_diag_exec = NULL;
    }
}

/*****************************************************************
 *
 * Function: gs_rx_polling_uart
 *
 * Description: This function reads data from uart controller, and return
 *              pass if the input string is found. If the string can't be
 *              found after timeout, then return failure.
 *
 * Input:  tty_dev: device string, (ie /dev/ttyDASH0, .../dev/ttyDASH7)
 *         comp_str: compared string
 *         timeout: timeout value (ms)
 *
 * Output: PASSED
 *         FAILED
 *
 *****************************************************************
 */
int gs_rx_polling_uart (int uart_fd, char *comp_str, int timeout)
{
    int cnt;
    struct timeval read_timeout;
    fd_set set;
    char buf[1024];
    char *search_str;
    int rc;
    struct timeval start_time, curr_time;
    int elapsed_time_in_ms;

    memset(buf, 0, 1024);

    gettimeofday(&start_time, NULL);

    do {
        /* Set timeout on file descriptor */
        FD_ZERO(&set);
        FD_SET(uart_fd, &set);

        read_timeout.tv_sec  = 1;
        read_timeout.tv_usec = 0;

        rc = select(uart_fd + 1, &set, NULL, NULL, &read_timeout);

        if (rc == -1) { /* Error occured */
            perror("select");
            fflush(stdout);
            goto exit_poll_uart;
        }

        if (FD_ISSET(uart_fd, &set)) {
            /* Now, we read the buffer */
            cnt = read(uart_fd, buf, 255);
            if (cnt < 0) {
                perror("Read error");
                fflush(stdout);
                goto exit_poll_uart;
            }
            /* Check if compared string can be found in the incoming string */
            search_str = strstr(buf, comp_str);
            if (search_str != NULL) { /* Found the string */
                return (PASSED);
            }
        }

        /* Now check if elapsed time exceeds timeout value */
        gettimeofday(&curr_time, NULL);

        elapsed_time_in_ms = (curr_time.tv_sec - start_time.tv_sec) * 1000;
        elapsed_time_in_ms += (curr_time.tv_usec - start_time.tv_usec) / 1000;

        if (elapsed_time_in_ms > timeout) {
            goto exit_poll_uart;
        }

        msleep(1);
    } while (1);

exit_poll_uart:

    return (FAILED);
}

/*****************************************************************
 *
 * Function: gs_tx_uart
 *
 * Description: This function transmits strings into tty
 *
 * Input:  tty_dev: device string, (ie /dev/ttyDASH0, .../dev/ttyDASH7)
 *         out_str: compared string
 *
 * Output: PASSED
 *         FAILED
 *
 *****************************************************************
 */
int gs_tx_uart (int uart_fd, char *out_str)
{
    int cnt;
    int rc = PASSED;

    cnt = write(uart_fd, out_str, strlen(out_str));

    if (cnt < 0) {
        perror("tx_uart: write failed\n");
        rc = FAILED;
    }

    return (rc);
}

/*******************************************************************************
 *
 * Function: setup_uart
 *
 * Description: Setup UART Interface Parameter
 *  
 * Input:  None
 *
 * Output: None 
 *
 *******************************************************************************
 */
static void setup_uart (void)
{
    const int maxlen = 128;
    char tty[maxlen];
    int fd, slot;
    struct termios oldtio, newtio;
    int new_baud = 0;

    assert(goldschlager_wic_iface);
    slot = goldschlager_wic_iface->slot;
    assert((slot == 1) || (slot == 2) || (slot == 3));

    snprintf(tty, maxlen-1, "/dev/ttyDASH%d",
             goldschlager_wic_iface->uart_ctrl); 

#ifdef TACHI
    /* Setup FPGA BMC UART connection */
    if ((diag_uart_to_nim_cnnt(slot)) == FAILED) {
        printf("%s: Fails in NIM UART connection\n", __FUNCTION__);
    }
    if (goldschlager_wic_iface->mod_type == SM_DAUGHTER_CARD) {
        snprintf(tty, maxlen-1, "/dev/ttyS%d",slot-1);
    } else {
        snprintf(tty, maxlen-1, "/dev/ttyS%d",slot+1);
    }
#endif

    fd = open(tty, O_RDWR|O_NOCTTY);
    if (fd < 0) {
      perror(tty);
      exit(1);
    }

    tcgetattr(fd, &oldtio);
    bzero(&newtio, sizeof(newtio));

    /* Get current BAUD setting */
    if (slot == GOLDSCHLAGER_NIM_SLOT1) {
        new_baud = slot1_uart;
    } else if (slot == GOLDSCHLAGER_NIM_SLOT2) {
        new_baud = slot2_uart;
    } else if (slot == GOLDSCHLAGER_NIM_SLOT3) {
        new_baud = slot3_uart;
    } else {
        cterr('f',0,"Invalid slot number: %d.", slot);
        close(fd);
        return;
    }
   
    if ( new_baud == GOLDSCHLAGER_B115200) {
        newtio.c_cflag = B115200|CS8|CLOCAL|CREAD; /* control mode flags */
    } else if (new_baud == GOLDSCHLAGER_B9600) {
        newtio.c_cflag = B9600|CS8|CLOCAL|CREAD;   /* control mode flags */
    } else {
        cterr('f',0,"Invalid Baud: %d.", new_baud);
        close(fd);
        return;
    }

    /* IGNPAR : Ignore framing errors and parity errors*/
    /* ICRNL  : Translate carriage return to newline on input (unless IGNCR is set). */
    /* ICANON : Enable canonical input (else raw) */
    newtio.c_iflag = IGNPAR | ICRNL;  /* input mode flags  */
    newtio.c_oflag = 0;               /* output mode flags */
    newtio.c_lflag = ICANON;          /* local mode flags  */

    tcflush(fd, TCIFLUSH);
    tcflush(fd, TCOFLUSH);
    tcsetattr(fd, TCSANOW, &newtio);
    close(fd);
    return;
}

/*******************************************************************************
 *
 * Function: goldschlager_print_spining_wheel
 *
 * Description: Display the spining wheel during the waiting time.
 *  
 * Input:  Ring cycle
 *
 * Output: none
 *
 *******************************************************************************
 */
static void goldschlager_print_spining_wheel (int pass)
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

static int goldschlager_linux_cret (void)
{
    int slot;
    const int maxlen = 128;
    char tty[maxlen];
    char cret_buff[] = "\r";
    int fd; 

    assert(goldschlager_wic_iface);
    slot = goldschlager_wic_iface->slot;
    assert((slot == 1) || (slot == 2) || (slot == 3));

    printf("\n");
    fflush(stdout);

    snprintf(tty, maxlen-1, "/dev/ttyDASH%d",
             goldschlager_wic_iface->uart_ctrl); 

#ifdef TACHI
    /* Setup FPGA BMC UART connection */
    if ((diag_uart_to_nim_cnnt(slot)) == FAILED) {
        printf("%s: Fails in NIM UART connection\n", __FUNCTION__);
        return (FAILED);
    }
    if (goldschlager_wic_iface->mod_type == SM_DAUGHTER_CARD) {
        snprintf(tty, maxlen-1, "/dev/ttyS%d",slot-1);
    } else {
        snprintf(tty, maxlen-1, "/dev/ttyS%d",slot+1);
    }
#endif

    fd = open(tty, O_RDWR|O_NOCTTY);

    /* Step 2: Set Linux IP address */
    fflush(stdout);
    write(fd, cret_buff, strlen(cret_buff));
    fflush(stdout);
    write(fd, cret_buff, strlen(cret_buff));
    fflush(stdout);
    
    close(fd);
    return (PASS);

}

/*******************************************************************************
 *
 * Function: goldschlager_linux_ip_set
 *
 * Description: This function set linux IP address
 *
 * Input : None
 *
 * Output: PASSED/FAILED
 *
 *******************************************************************************
 */
static int goldschlager_linux_ip_set (void)
{
    int slot, ix;
    const int maxlen = 128;
    char tty[maxlen];
    char linux_str[] = ">";
    char linux_ip_str[] = "ifconfig br0 192.123.123.10";
    char shell_str[] = "sh";
    char platform_str[] = "";
    char cret_buff[] = "\r";
    int fd, platform_id = BDTYPE_UNKNOWN;
    int setip_wait_retry = 10;
    char buf[256];

    assert(goldschlager_wic_iface);
    slot = goldschlager_wic_iface->slot;
    assert((slot == 1) || (slot == 2) || (slot == 3));

    prpass(testpass, "Set Goldschlager Linux IP address,");
    printf("\n");

    printf("Start Linux UART...\n");
    fflush(stdout);

    snprintf(tty, maxlen-1, "/dev/ttyDASH%d",
             goldschlager_wic_iface->uart_ctrl); 

#ifdef TACHI
    /* Setup FPGA BMC UART connection */
    if ((diag_uart_to_nim_cnnt(slot)) == FAILED) {
        printf("%s: Fails in NIM UART connection\n", __FUNCTION__);
        return (FAILED);
    }
    if (goldschlager_wic_iface->mod_type == SM_DAUGHTER_CARD) {
        snprintf(tty, maxlen-1, "/dev/ttyS%d",slot-1);
    } else {
        snprintf(tty, maxlen-1, "/dev/ttyS%d",slot+1);
    }
#endif

    fd = open(tty, O_RDWR|O_NOCTTY);

    /* Step 1 : Find ">" prompt */
    for (ix = 0; ix < setip_wait_retry; ix++) {
        write(fd, cret_buff, strlen(cret_buff));
        read(fd, buf, 255);
        if (strstr(buf, linux_str) != NULL) {
            printf("Found : %s\n", linux_str);
            fflush(stdout);
            break;      /* proceed when prompt string found */
        }
        msleep(1000);
    }
    if (ix == setip_wait_retry) {
        cterr('f',0,"Failed to get Linux prompt");
        close(fd);
        return (FAILED);
    }

    /* Step 2: Set Linux IP address */
    sprintf(linux_ip_str, "%s%d", linux_ip_str, slot);
    write(fd, linux_ip_str, strlen(linux_ip_str));
    printf("Send Linux IP [%s]\n", linux_ip_str);
    fflush(stdout);
    write(fd, cret_buff, strlen(cret_buff));

    /* Step 3: Enter shell mode */
    write(fd, shell_str, strlen(shell_str));
    printf("Send [%s]\n", shell_str);
    fflush(stdout);
    write(fd, cret_buff, strlen(cret_buff));

    /* Step 4: Set environment variable - platform type */
#ifdef TACHI
   platform_id = BDTYPE_TACHI_ENTRY;
#elif TABEIL
   platform_id = BDTYPE_TABEI_L;
#else
    platform_id = mb_board_type();
    switch(platform_id) {
    case BDTYPE_OVERLORD:
        sprintf(platform_str, "echo %s > /tmp/platform", "OVERLORD");
        break;
    case BDTYPE_JUNO:
        sprintf(platform_str, "echo %s > /tmp/platform", "JUNO");
        break;
    case BDTYPE_UTAH:
        sprintf(platform_str, "echo %s > /tmp/platform", "UTAH");
        break;
    case BDTYPE_SWORD:
        sprintf(platform_str, "echo %s > /tmp/platform", "SWORD");
        break;
    case BDTYPE_DAGGER:
        sprintf(platform_str, "echo %s > /tmp/platform", "DAGGER");
        break;
    }
#endif

    write(fd, platform_str, strlen(platform_str));
    printf("Send [%s]\n", platform_str);
    fflush(stdout);
    write(fd, cret_buff, strlen(cret_buff));

    close(fd);
    return (PASS);

}

/*******************************************************************************
 *
 * Function: goldschlager_cfe_pm_set
 *
 * Description: This function set cfe parameters.
 *
 * Input : None
 *
 * Output: PASSED/FAILED
 *
 *******************************************************************************
 */
static int goldschlager_cfe_pm_set (void)
{
    int slot, ix;
    const int maxlen = 128;
    char tty[maxlen];
    char press_key_str[] = "Press any key";
    char cfe_str[] = "CFE>";
    char change_parameters_str[] = "c";
    char command_status_buff[] = "command status";
    char board_ip_str[] = "192.123.123.10";
    char host_ip_str[] = "192.123.123.";
    char gw_ip_str[] = "192.123.123.";
    char *linux_file_str;
    char linux_annexA_file_str[] = "firmware/nim_gs_a_diag.img";    
    char linux_annexB_file_str[] = "firmware/nim_gs_b_diag.img";    
    char linux_annexM_file_str[] = "firmware/nim_gs_m_diag.img";    
    char cret_buff[] = "\r";
    int result = PASSED;
    int tty_desc;

    assert(goldschlager_wic_iface);
    slot = goldschlager_wic_iface->slot;
    assert((slot == 1) || (slot == 2) || (slot == 3));

    prpass(testpass, "Set Goldschlager CFE Parameters,");
    printf("\n");

    printf("Start CFE UART...\n");
    fflush(stdout);
    fflush(stderr);

    snprintf(tty, maxlen-1, "/dev/ttyDASH%d",
             goldschlager_wic_iface->uart_ctrl); 
#ifdef TACHI
    /* Setup FPGA BMC UART connection */
    if ((diag_uart_to_nim_cnnt(slot)) == FAILED) {
        printf("%s: Fails in NIM UART connection\n", __FUNCTION__);
        return (FAILED);
    }
    if (goldschlager_wic_iface->mod_type == SM_DAUGHTER_CARD) {
        snprintf(tty, maxlen-1, "/dev/ttyS%d",slot-1);
    } else {
        snprintf(tty, maxlen-1, "/dev/ttyS%d",slot+1);
    }
#endif

    tty_desc = open(tty, O_RDWR | O_NOCTTY);
    if (tty_desc < 0) {
        perror("open tty failed");
        fflush(stdout);
        return (FAILED);
    }

    /* Step 0 : Catch "Press any key" */
    result = gs_rx_polling_uart(tty_desc, press_key_str, GS_CFE_WAIT_TIME_30S);
    if (result != FAILED) {
        printf("Found : %s\n", press_key_str);
        fflush(stdout);
    } else {
        printf("[%s] Not Found.\n", press_key_str);
        fflush(stdout);
        goto exit_cfe_pm_set_failed;
    } 

    /* Step 1 : Find "CFE>" */
    for (ix = 0; ix < GS_CFE_RETRY_30T; ix++) {
        if (gs_tx_uart(tty_desc, cret_buff) == FAILED) {
            goto exit_cfe_pm_set_failed;
        }
        result = gs_rx_polling_uart(tty_desc, cfe_str, GS_CFE_TOUT_100MS);
        if (result != FAILED) {
            printf("Found : %s\n", cfe_str);
            fflush(stdout);
            break;
        }
    }

    if (ix == GS_CFE_RETRY_30T) {
        printf("[%s] Not Found.\n", cfe_str);
        fflush(stdout);
        goto exit_cfe_pm_set_failed;
    }

    msleep(500);

    /* Step 2 : Set board parameters   */
    if (gs_tx_uart(tty_desc, change_parameters_str) == FAILED) {
        goto exit_cfe_pm_set_failed;
    }    
    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("Send bdpm [%s]\n", change_parameters_str);
        fflush(stdout);
    }
    msleep(500);
    if (gs_tx_uart(tty_desc, cret_buff) == FAILED) {
        goto exit_cfe_pm_set_failed;
    }

    /* Step 3: Set Board IP */
    sprintf(board_ip_str, "%s%d", board_ip_str, slot);

    if (gs_tx_uart(tty_desc, board_ip_str) == FAILED) {
        goto exit_cfe_pm_set_failed;
    } 
    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("Send Board IP[%s]\n", board_ip_str);
        fflush(stdout);
    }
    msleep(500);
    if (gs_tx_uart(tty_desc, cret_buff) == FAILED) {
        goto exit_cfe_pm_set_failed;
    }

    /* Step 4: Set Host IP */
#ifdef TACHI
    return (FALSE);
#elif TABEIL
    sprintf(host_ip_str, "%s1", host_ip_str);
#else
    if (is_overlord() || is_juno()){
        sprintf(host_ip_str, "%s1", host_ip_str);
    } else {    /* Utah, Sword, Dagger, Dagger-Lite */
        sprintf(host_ip_str, "%s1", host_ip_str);
    }
#endif
    if (gs_tx_uart(tty_desc, host_ip_str) == FAILED) {
        goto exit_cfe_pm_set_failed;
    } 
    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("Send Host IP[%s]\r", host_ip_str);
        fflush(stdout);
    }
    msleep(500);
    if (gs_tx_uart(tty_desc, cret_buff) == FAILED) {
        goto exit_cfe_pm_set_failed;
    }

    /* Step 5: Set Geteway IP */
#ifdef TACHI
    return (FALSE);
#elif TABEIL
    sprintf(gw_ip_str, "%s1", gw_ip_str);
#else
    if (is_overlord() || is_juno()){
        sprintf(gw_ip_str, "%s1", gw_ip_str);
    } else {    /* Utah, Sword, Dagger, Dagger-Lite */
        sprintf(gw_ip_str, "%s1", gw_ip_str);
    }
#endif
    if (gs_tx_uart(tty_desc, gw_ip_str) == FAILED) {
        goto exit_cfe_pm_set_failed;
    } 
    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("Send GW IP[%s]\n", gw_ip_str);
        fflush(stdout);
    }
    msleep(500);
    if (gs_tx_uart(tty_desc, cret_buff) == FAILED) {
        goto exit_cfe_pm_set_failed;
    }
    
    /* Step 6: one more carrier return */
    if (gs_tx_uart(tty_desc, cret_buff) == FAILED) {
        goto exit_cfe_pm_set_failed;
    } 
    msleep(500);

    /* Step 7: Set default linux file name */
    if (goldschlager_sku == GS_NIM_VAB_A) {
        linux_file_str = &linux_annexA_file_str[0];
    } else if (goldschlager_sku == GS_NIM_VA_B) {
        linux_file_str = &linux_annexB_file_str[0];
    } else {
        linux_file_str = &linux_annexM_file_str[0];
    }

    msleep(500);
    if (gs_tx_uart(tty_desc, linux_file_str) == FAILED) {
        goto exit_cfe_pm_set_failed;
    }
    printf("Diag image [%s]\n", linux_file_str);
    fflush(stdout);
    msleep(500);
    if (gs_tx_uart(tty_desc, cret_buff) == FAILED) {
        goto exit_cfe_pm_set_failed;
    }

    /* Step 8 : Find "command status" */
    for (ix = 0; ix < GS_CFE_RETRY_30T; ix++) {
        if (gs_tx_uart(tty_desc, cret_buff) == FAILED) {
            goto exit_cfe_pm_set_failed;
        }
        result = gs_rx_polling_uart(tty_desc, command_status_buff, GS_CFE_TOUT_1S);
        if (result != FAILED) {
            printf("Found : %s\n", command_status_buff);
            fflush(stdout);
            break;
        }
        msleep(200);
    }

    if (ix == GS_CFE_RETRY_30T) {
        printf("[%s] Not Found.\n", command_status_buff);
        fflush(stdout);
        goto exit_cfe_pm_set_failed;
    }
    msleep(500);

    /* Step 9 : Find "CFE>" */
    for (ix = 0; ix < GS_CFE_RETRY_30T; ix++) {
        if (gs_tx_uart(tty_desc, cret_buff) == FAILED) {
            goto exit_cfe_pm_set_failed;
        }
        result = gs_rx_polling_uart(tty_desc, cfe_str, GS_CFE_TOUT_100MS);
        if (result != FAILED) {
            printf("Found : %s\n", cfe_str);
            fflush(stdout);
            break;
        } 
    }

    if (ix == GS_CFE_RETRY_30T) {
        printf("[%s] Not Found.\n", cfe_str);
        fflush(stdout);
        goto exit_cfe_pm_set_failed;
    }

    msleep(WAIT_FOR_FLASH_WRITE);

    close(tty_desc);
    return (PASSED);
    
exit_cfe_pm_set_failed:
    close(tty_desc);
    return (FAILED);

}

/*******************************************************************************
 *
 * Function: goldschlager_bootup.
 *
 * Description: This function let goldschlager boot up to diag.
 *
 * Input : None
 *
 * Output: PASSED/FAILED
 *
 *******************************************************************************
 */
static int goldschlager_bootup (void)
{
    int  ret_val;
    int ix, slot, pass = 0, linux_wait_retry = 90;
    int jx = 0, set_ip_retry = 5;

    testname("Goldschlager boot up");

    assert(goldschlager_wic_iface);
    slot = goldschlager_wic_iface->slot;
    assert((slot == 1) || (slot == 2) || (slot == 3));

    /* Step1: Reset */
    goldschlager_bcm963268_reset();

    /* Step2: Set CFE IPs and Filename */ 
    if (goldschlager_cfe_pm_set() == FAILED) {
        cterr('f',0,"CFE boot parameters set failed.\n");
        return (FAILED);
    }

    for (jx = 0; jx <= set_ip_retry; jx++) {
        /* Step3: Reset */
        goldschlager_bcm963268_reset();

        /* Step4: Check if GS is ready (Timeout 90 secs) */
        for (ix = 0; ix < linux_wait_retry; ix++) {
            if (pri_intf_rdy_chk() == PASSED) {
                printf ("\nGoldschlager NIM is ready.\n");
                fflush(stdout);
                break;
            }
            /* wait for Lebowski to get CR and respond with prompt */
            goldschlager_print_spining_wheel(pass++);
            msleep(1000);
            if ((ix == 25) || (ix == 30)) {
                goldschlager_linux_cret();
            }
        }
        if (ix == linux_wait_retry) {
            cterr('f',0,"Failed to get GS NIM ready");
            return (FAILED);
        }

        /* Step5: Set Linux IPs */
        ret_val = goldschlager_linux_ip_set();

        /* Check if GS has identical sku type(BCM GPIO)*/
        msleep (1000);
        ret_val = goldschlager_check_sku_type();
        if (ret_val == PASS) {
             break;
        } 

        if (jx == set_ip_retry) {
            cterr('f',0,"Failed to check SKU type. %jx retry.", jx);
            return (FAILED);
        }
    }
    
    return (ret_val);
}


/*******************************************************************************
 *
 * Function: goldschlager_iface_test
 *
 * Description: Test entry for goldschlager interface test.
 *              covered: I2C, GE0, UART.
 *
 * Input : None
 *
 * Output: PASSED/FAILED
 *
 *******************************************************************************
 */
int goldschlager_iface_test ()
{
    /* Testing the I2C interface */
    if (ltc4215_register_test()) {
        return (FAILED);
    }

    /* Testing UART and SGMII interfaces */
    printf ("\nWait for Goldschlager side to boot up and download diag.\n");
    if (goldschlager_bootup() == FAILED) {
        return (FAILED);
    }

    prcomplete (testpass, errcount, 0);
    return (PASSED);
}

/*******************************************************************************
 *
 * Function: goldschlager_ngwic_test().
 *
 * Description: This function is the main entrance for Goldschlager NGWIC test .
 *
 * Input:  wic - pointer to ngio_intf_t struct
 *
 * Output: PASSED/FAILED.
 *
 *******************************************************************************
 */
int goldschlager_ngwic_test (void *wic)
{ 
    int slot;
    ushort board_id = 0;
    int ret_val = PASSED;
    char cmd[128];
    char board_pid[20];
    uchar num_byte;
    struct stat sts;
    char gs_src_diag_img[32];
    char gs_dest_diag_img[64] = "/firmware/";
    char gs_wic_info[64];
    uint32_t module_type;
    ngio_eth_speed_t new_speed, old_speed;
    n2g_i2c_if_t *goldschlager_pca;

    assert (wic);

    goldschlager_wic_iface = (struct ngio_intf_t *)wic;

    slot = goldschlager_wic_iface->slot;
    goldschlager_test_slot = goldschlager_wic_iface->slot;
    board_id = goldschlager_wic_iface->id;
    goldschlager_board_id = goldschlager_wic_iface->id; 

    /* Curie 2RU: Force eth port to 1Gb/s for 10G MAC BCM57412 which has
     * not the ability of auto-negotiation between 1G and 10G */
    module_type = goldschlager_wic_iface->mod_type;
    new_speed = NGIO_ETH_SPEED_1G;
    ngio_cfg_eth_port_speed(module_type, slot, &new_speed, &old_speed);

    /* Goldschlager 3 SKUs share one CID, identify SKUs by PID */
    slot_get_bd_pid(goldschlager_wic_iface->cookie, board_pid, &num_byte);
    num_byte = 9; /* P1A2 MFG pads PID with 7 white spaces. */

    if (!strncmp(board_pid, (char*)"NIM-VAB-A", num_byte)) {
        goldschlager_sku = GS_NIM_VAB_A;
        sprintf(gs_src_diag_img, "%s", GOLDSCHLAGER_SRC_DIAG_A_IMG);
        sprintf(gs_wic_info, "\n%s, Board_ID 0x%04X, Slot %d.\n",
                "NIM-VAB-A", board_id, slot);
    } else if (!strncmp(board_pid, (char*)"NIM-VA-B", num_byte - 1)) {
        goldschlager_sku = GS_NIM_VA_B;
        sprintf(gs_src_diag_img, "%s", GOLDSCHLAGER_SRC_DIAG_B_IMG);
        sprintf(gs_wic_info, "\n%s, Board_ID 0x%04X, Slot %d.\n",
                "NIM-VA-B", board_id, slot);
    } else if (!strncmp(board_pid, (char*)"NIM-VAB-M", num_byte)) {
        goldschlager_sku = GS_NIM_VAB_M;
        sprintf(gs_src_diag_img, "%s", GOLDSCHLAGER_SRC_DIAG_M_IMG);
        sprintf(gs_wic_info, "\n%s, Board_ID 0x%04X, Slot %d.\n",
                "NIM-VAB-M", board_id, slot);
    } else {
        cterr('f', 0, "Not supported Goldschlager PID %s", board_pid);
        return (FAILED);
    }

    sprintf(gs_dest_diag_img, "%s%s", gs_dest_diag_img, gs_src_diag_img);

    /* Download FW image from the network for the first time  */
    if (stat(gs_dest_diag_img, &sts) == -1) {
        if (tftp_get(0 , gs_src_diag_img, 0, gs_dest_diag_img, 0) < 0) {
            sprintf(cmd, "rm -f %s", gs_dest_diag_img);
            system(cmd);
            fflush(stdout);
            cterr('f', 0, "Failed to tftp download firmware to local host");
            return (FAILED);
        }
    }

    goldschlager_wic_iface->uart_on (wic); 

    printf ("%s", gs_wic_info);

    testname ("Slot%d Goldschlager NIM ", slot);

    pca_init_i2c((void *)&pca_i2c[0]);
    pca_i2c[0].i2c_ctrl = goldschlager_wic_iface->i2c_ctrl;
    pca_i2c[0].i2c_dev = NGWIC_I2C_ADDR_IO_PORT;
    pca_i2c[0].buf   = pca_buff0;

    goldschlager_pca = (n2g_i2c_if_t *)goldschlager_wic_iface->pca;
    pca_i2c[0].i2c_bus_type = goldschlager_pca->i2c_bus_type;
    pca_i2c[0].i2c_base = goldschlager_pca->i2c_base;

    oir_if = (void *)(goldschlager_wic_iface->oir);

    /* turn on the green light */
    if (util_oir_ltc4215_led(oir_if, OIR_LED_GREEN_ONLY)) {
        return (FAILED);
    }

    /* Set Goldschlager boot up parameters */
    setup_uart();

    /*
     * pm_subtest_menu now built.  Display and interact with user until
     * <ESC><RET> back to main menu.
     *
     * To prevent freeing up allocated memory prematurely,
     * save the pre_diag_exec function and set it to NULL.
     * This will prevent menu() marking the needed memory freed.
     */
    goldschlager_saved_diag_exec = pre_diag_exec;
    pre_diag_exec = NULL;

    build_primary_submenu(main_menu_table, MAIN_MENU_TABLE_SIZE, dgmenustr,
                          &maindiagp);
    build_secondary_submenu(main_menu_table, MAIN_MENU_TABLE_SIZE,
                            main_menu_secondary_items);

    if (goldschlager_wic_iface->test_type == IFACE_TEST) {
        ret_val = goldschlager_iface_test();
    } else {
        if (goldschlager_wic_iface->menu_display == TRUE) {
            menu(maindiagp, main_menu_secondary_items, '\0');
        } else {
            do_all_menu_items(maindiagp);
        }
    }

    goldschlager_cleanup ();

    /* Curie 2RU: restore eth configuration */
    ngio_cfg_eth_port_speed(module_type, slot, &old_speed, NULL);

    return (ret_val);
}

/*******************************************************************************
 *
 * Function: goldschlager_get_wic_ip_addr
 *
 * Description: This function returns IP Address of WIC card based
 *              on slot number
 *
 * Input:  ip_addr - Buffer to put ip address
 *
 * Output: None
 *
 *******************************************************************************
 */
void goldschlager_get_wic_ip_addr (char *ip_addr)
{
    /* Sanity check */
    if (ip_addr == NULL) {
        printf("%s: NULL Pointer\n", __FUNCTION__);
        return;
    }

    sprintf(ip_addr, "%s.%d", GOLDSCHLAGER_DIAG_IP_ADDR_SUBNET,
            GOLDSCHLAGER_DIAG_IP_ADDR_BASE + goldschlager_test_slot);
}

/*******************************************************************************
 *
 *  Function: goldschlager_utils
 *
 *  Description: goldschlager Utitlities menu
 *
 *  Input: None 
 *
 *  Returns: PASSED
 *
 *******************************************************************************
 */
static int goldschlager_utils (void)
{
    assert(goldschlager_wic_iface);

    sprintf(goldschlagerutiltitle, "goldschlager Slot %d Utilities Menu", 
            goldschlager_wic_iface->slot);
    build_primary_submenu(goldschlager_utils_submenu_table,
                          goldschlager_UTILS_SUBMENU_TABLE_SZ,
                          goldschlagerutiltitle, &goldschlager_util_submenup);

    build_secondary_submenu(goldschlager_utils_submenu_table,
                            goldschlager_UTILS_SUBMENU_TABLE_SZ,
                            gold_utils_secondary_items);

    menu(goldschlager_util_submenup, gold_utils_secondary_items, '\0');

    return (PASSED);
}

/*******************************************************************************
 *
 * Function    : goldschlager_param_test
 *
 * Description : This function builds menu for showtime and parametric tests
 *
 * Input       : NONE
 *
 * Output      : NONE
 *
 *******************************************************************************
 */
static int goldschlager_param_test (int show_menu)
{
    testname("Showtime and Parametric");

    /* Check Goldschlager is ready */
    if (pri_intf_rdy_chk() != PASSED) {
        printf ("\nGoldschlager NIM is not ready."
                "Please boot up Goldschlager first.\n");
        return (PASSED);
    }
    
    build_primary_submenu(pt_submenu_table, 
                          PT_SUBMENU_TABLE_SIZE,
                          "Showtime and Parametric", &pt_submenup);
    build_secondary_submenu(pt_submenu_table,
                            PT_SUBMENU_TABLE_SIZE,
                            pt_tests_secondary_items);

    if (show_menu) {
        menu(pt_submenup, pt_tests_secondary_items, '\0');
    } else {
        do_all_menu_items(pt_submenup);
    }

    return (PASSED);
}


/*******************************************************************************
 *
 * Function   : goldschlager_ioe_reg_test
 *
 * Description: Wrapped function to do Goldschlager IO Expander register test.
 *
 * Inputs     : None
 *
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
static int goldschlager_ioe_reg_test (void)
{
    uint32_t         ctr = 0, test_ctr = 0, total_reg_num = 0;
    uchar            orig_val = 0, test_data = 0, check_data = 0;
    reg_info_t       *reg_p = 0;
    n2g_i2c_if_t     *io_exp;
#if ENHANCED_ERR_MSG_EXAMPLE
    uchar ngwic_get_loc[FRU_SIZE] = {0};
    uchar ngwic_get_pid[FRU_SIZE] = {0};
#endif 

    testname("PCA9557 IO Expender Register");
    
#ifdef ENHANCED_ERR_MSG_EXAMPLE
    /*
     * 1. Subtests of the test function will reuse all variables
     * 2. All variables will be cleared automatically when
     *    entering and leaving each menu item.
     */
    /* Segment 1: PID | Unique_string : slot_info */
    fru_table_offset = WIC0 + goldschlager_test_slot - 1;
    /* fru_table_offset should be set, otherwise, it will not */
    /* go to enhanced error message format in cterr() */
    /* set fru_table_offset to get the predefine value */
    /* or change mb_pid & mb_loc below */
    memcpy(ngwic_get_pid,(char*)&goldschlager_board_id, 2); 
    strcpy((char *)ngwic_get_loc, "MB/VDSL2+ WIC");
    platform_fru_table[fru_table_offset].pid_string = ngwic_get_pid; 
    platform_fru_table[fru_table_offset].location_string = ngwic_get_loc;

    /* Segment 2: Test step captured from prpass */
    /* Segment 3: Failure message captured from cterr */

    /* Segment 4: Components used */
    cterr_add_component("NGIO", "PCA9557");

    /* Segment 5: register and memory dump */
    //cterr_add_reg_dump((PFV)bcm63168_get_xdsl_profile);

    /* Segment 6: Platform Environment initialized here*/
    //cterr_add_env_dump((PFV)show_margins_cterr_wrapper);

    /* Segment 7: Top 3 Debugging Steps */
    cterr_add_debug("Using \"IO Expander(PCA9557) Reg. Write\" utility to "
                    "write the value 0x0 into offset 0x3 register.",
                    "Using \"IO Expander(PCA9557) Reg. Dump\" utility to "
                    "dump all register in PCA9557. Check the the value in "
                    "offset 0x3 register is 0x0. If not, check the path "
                    "between the host and PCA9557.",
                    "If there is no problem on I2C interface, replace one "
                    "PCA9557 and redo the test."
                    ); 
#endif 

    io_exp = &pca_i2c[0];

    reg_p = &pca9557_reg_tbl[0];
    total_reg_num = (sizeof(pca9557_reg_tbl) / sizeof(reg_info_t));

    for (ctr = 0; ctr < total_reg_num; ctr++, reg_p++) {
        /* Skip Input port registers & Output port registers
         * Based on PCA9557 datasheet, Input port registers are input-only,
         * writes to these registers have no effect.
         * And skip Output port registers to avoid to change the system set-ups.
         * Like cause ShrinkRay alien sub-module be put in reset(GPIO[2] = 0).
         */
        if ((reg_p->offset == PCA9557_IN_PORT_REG) ||
            (reg_p->offset == PCA9557_OUT_PORT_REG)) {
            continue;
        }

        if ((reg_p->type & SAVE_RESTORE) == SAVE_RESTORE) {
            /* Backup Original value */
            if (io_port_8bit_i2c_read(io_exp, ctr, &orig_val, TRUE)) {
                cterr('f', 0, "%s: Failed to read IO Expander Reg %#x"
                              " as restore value.",
                              __FUNCTION__, reg_p->offset);
                return (FAILED);
            }

            /*
             * Ripple 1 test
             */
            for (test_ctr = 0; test_ctr < (sizeof(test_data) * 8); test_ctr++) {
                test_data = ((1 << test_ctr) & reg_p->mask);
                if (!test_data) {
                    continue;
                }

                /* Write Test Data in */
                if (io_port_8bit_i2c_write(io_exp, ctr, &test_data)) {
                    cterr('f', 0, "%s: Failed to wrote 0x%02X "
                                  "to IO Expander Reg. %#x in Ripple 1 test.",
                                  __FUNCTION__, test_data, reg_p->offset);
                    return (FAILED);
                }

                /* Read the register value back for double check */
                if (io_port_8bit_i2c_read(io_exp, ctr, &check_data, TRUE)) {
                    cterr('f', 0, "%s: Failed to read IO Expander Reg. %#x "
                                  "in Ripple 1 test",
                                  __FUNCTION__, reg_p->offset);
                    return (FAILED);
                }

                /* Data Comparation */
                if (check_data != test_data) {
                    cterr('f', 0, "%s Reg. Ripple 1 test FAILED, "
                                  "read back = 0x%02X and expected = 0x%02X.",
                                  reg_p->name, check_data, test_data);
                    return (FAILED);
                }
            }   /* End of Ripple 1 Test */

            check_data = 0;
            test_data = 0;

            /*
             * Ripple 0 test
             */
            for (test_ctr = 0; test_ctr < (sizeof(test_data) * 8); test_ctr++) {
                test_data = (1 << test_ctr);
                if (!test_data) {
                    continue;
                }

                test_data = ((uchar)(~(1 << test_ctr)) & reg_p->mask);

                /* Write Test Data in */
                if (io_port_8bit_i2c_write(io_exp, ctr, &test_data)) {
                    cterr('f', 0, "%s: Failed to wrote 0x%02X "
                                  "to IO Expander Reg. %#x in Ripple 0 test.",
                                  __FUNCTION__, test_data, reg_p->offset);
                    return (FAILED);
                }

                /* Read the register value back for double check */
                if (io_port_8bit_i2c_read(io_exp, ctr, &check_data, TRUE)) {
                    cterr('f', 0, "%s: Failed to read IO Expander Reg. %#x "
                                  "in Ripple 0 test.",
                          __FUNCTION__, reg_p->offset);
                    return (FAILED);
                }

                /* Data Comparation */
                if (check_data != test_data) {
                    cterr('f', 0, "%s Reg. Ripple 0 test FAILED, "
                                  "read back = 0x%02X and expected = 0x%02X.",
                                  reg_p->name, check_data, test_data);
                    return (FAILED);
                }
            }   /* End of Ripple 0 Test */

            /* Restore the value before test */
            if (io_port_8bit_i2c_write(io_exp, ctr, &orig_val)) {
                cterr('f', 0, "%s: Failed to write the restore value 0x%02X "
                              "back to IO Expander Reg. %#x.",
                              __FUNCTION__, test_data, reg_p->offset);
                return (FAILED);
            }
        }
    }

    return (PASSED);
}

/*******************************************************************************
 *
 * Function: goldschlager_bootflash_test
 *
 * Description: This function to test SPI boot flash.
 *
 * Input : none
 *
 * Output: PASSED/FAILED
 *
 *******************************************************************************
 */
static int goldschlager_bootflash_test (void)
{
    int retval = FAILED;

#if ENHANCED_ERR_MSG_EXAMPLE
    uchar ngwic_get_loc[FRU_SIZE] = {0};
    uchar ngwic_get_pid[FRU_SIZE] = {0};
#endif 

    testname("BCM863168 NOR Flash");
    
#ifdef ENHANCED_ERR_MSG_EXAMPLE
    /*
     * 1. Subtests of the test function will reuse all variables
     * 2. All variables will be cleared automatically when
     *    entering and leaving each menu item.
     */
    /* Segment 1: PID | Unique_string : slot_info */
    fru_table_offset = WIC0 + goldschlager_test_slot - 1;
    /* fru_table_offset should be set, otherwise, it will not */
    /* go to enhanced error message format in cterr() */
    /* set fru_table_offset to get the predefine value */
    /* or change mb_pid & mb_loc below */
    memcpy(ngwic_get_pid,(char*)&goldschlager_board_id, 2);
    strcpy((char *)ngwic_get_loc, "MB/VDSL2+ WIC");
    platform_fru_table[fru_table_offset].pid_string = ngwic_get_pid; 
    platform_fru_table[fru_table_offset].location_string = ngwic_get_loc;

    /* Segment 2: Test step captured from prpass */
    /* Segment 3: Failure message captured from cterr */

    /* Segment 4: Components used */
    cterr_add_component("NGIO", "88E1512", "BCM63168", "SPI Flash");

    /* Segment 5: register and memory dump */
    //cterr_add_reg_dump((PFV)bcm63168_get_xdsl_profile);

    /* Segment 6: Platform Environment initialized here*/
    //cterr_add_env_dump((PFV)show_margins_cterr_wrapper);

    /* Segment 7: Top 3 Debugging Steps */
    cterr_add_debug("Switch console to BCM side, and make sure BCM can reach "
                    "Host by typing \"ping 192.123.123.1\". "
                    "If ping fails, check two paths. "
                    "One is the interface between host and Marvell1512, the "
                    "other one is RGMII between the Marvell1512 and BCM63168.",
                    "Switch console to BCM side and use the "
                    "\"diag_gs spi_write\" command to write the known pattern "
                    "into scratch pad of NOR flash.",
                    "\"diag_gs spi_read\" to read the data from scratch pad "
                    "of NOR flash. Dump all data in scratch pad and compare it "
                    "with the data we wrote.\n"
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
                    "If step3 is good, maybe there is one sector in SPI flash "
                    "corrupt. Replace one SPI flash and redo the test. "
                    "If step3 fail, check the SPI between the BCM63168 and "
                    "SPI flash."
                    ); 
#endif 

    retval = goldschlager_nc_dispatch_comm(DIAG_COMMAND_BCM63268_FLASH_TEST,
                                           bcm_op_mode,
                                           bcm_idle_listen_params, 
                                           bcm_line_id);

    if (retval != PASSED) {
        cterr('f', 0, "%s failed.", __FUNCTION__);
        return (retval);
    }

    return (retval);
}

/*******************************************************************************
 *
 * Function   : goldschlager_mrvl_reg_test
 *
 * Description: The function to test Marvell 88E1512 PHY registers.
 *
 * Inputs     : NONE 
 *             
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
static int goldschlager_mrvl1512_reg_test (void)
{
    int retval = FAILED;
#if ENHANCED_ERR_MSG_EXAMPLE
    uchar ngwic_get_loc[FRU_SIZE] = {0};
    uchar ngwic_get_pid[FRU_SIZE] = {0};
#endif 

    testname("Marvell1512 Register");
    
#ifdef ENHANCED_ERR_MSG_EXAMPLE
    /*
     * 1. Subtests of the test function will reuse all variables
     * 2. All variables will be cleared automatically when
     *    entering and leaving each menu item.
     */
    /* Segment 1: PID | Unique_string : slot_info */
    fru_table_offset = WIC0 + goldschlager_test_slot - 1;
    /* fru_table_offset should be set, otherwise, it will not */
    /* go to enhanced error message format in cterr() */
    /* set fru_table_offset to get the predefine value */
    /* or change mb_pid & mb_loc below */
    memcpy(ngwic_get_pid,(char*)&goldschlager_board_id, 2);
    strcpy((char *)ngwic_get_loc, "MB/VDSL2+ WIC");
    platform_fru_table[fru_table_offset].pid_string = ngwic_get_pid; 
    platform_fru_table[fru_table_offset].location_string = ngwic_get_loc;

    /* Segment 2: Test step captured from prpass */
    /* Segment 3: Failure message captured from cterr */

    /* Segment 4: Components used */
    cterr_add_component("NGIO", "88E1512", "BCM63168");

    /* Segment 5: register and memory dump */
    //cterr_add_reg_dump((PFV)bcm63168_get_xdsl_profile);

    /* Segment 6: Platform Environment initialized here*/
    //cterr_add_env_dump((PFV)show_margins_cterr_wrapper);

    /* Segment 7: Top 3 Debugging Steps */
    cterr_add_debug("Switch console to BCM side, and make sure BCM can reach "
                    "Host by typing \"ping 192.123.123.1\". "
                    "If ping fails, check two paths. "
                    "One is the interface between host and Marvell1512, the "
                    "other one is RGMII between the Marvell1512 and BCM63168.",
                    "Using BCM command \"ethctl phy ext 0 22 0x0\" and "
                    "\"ethctl phy ext 0 18 0xFFFF\" to write the value(0xFFFF) "
                    "into register.",
                    "Use command \"ethctl phy ext 0 18\" to read the register "
                    "value and compare it against the written value(0xFFFF).",
                    "If step3 is good, maybe there is one register corrupt. "
                    "Replace one Marvell 1512 chip and redo the test. If step3 "
                    "fail, check the SMI between the BCM63168 and Marvell1512."
                    ); 
   
#endif 
    retval = goldschlager_nc_dispatch_comm(DIAG_COMMAND_MRVL1512_REG_TEST,
                                           bcm_op_mode,
                                           bcm_idle_listen_params, 
                                           bcm_line_id);

    if (retval != PASSED) {
        cterr('f', 0, "%s failed.", __FUNCTION__);
        return (retval);
    }

    return (retval);
}

/*******************************************************************************
 *
 * Function   : goldschlager_bcm63268_dram_test
 *
 * Description: The function to test DDR3 memory.
 *
 * Inputs     : NONE 
 *             
 * Outputs    : NONE
 *
 *******************************************************************************
 */
static int goldschlager_bcm63268_dram_test (void)
{
    int retval = FAILED;
#if ENHANCED_ERR_MSG_EXAMPLE
    uchar ngwic_get_loc[FRU_SIZE] = {0};
    uchar ngwic_get_pid[FRU_SIZE] = {0};
#endif 

    testname("BCM863168 DRAM");
    
#ifdef ENHANCED_ERR_MSG_EXAMPLE
    /*
     * 1. Subtests of the test function will reuse all variables
     * 2. All variables will be cleared automatically when
     *    entering and leaving each menu item.
     */
    /* Segment 1: PID | Unique_string : slot_info */
    fru_table_offset = WIC0 + goldschlager_test_slot - 1;
    /* fru_table_offset should be set, otherwise, it will not */
    /* go to enhanced error message format in cterr() */
    /* set fru_table_offset to get the predefine value */
    /* or change mb_pid & mb_loc below */
    memcpy(ngwic_get_pid,(char*)&goldschlager_board_id, 2);
    strcpy((char *)ngwic_get_loc, "MB/VDSL2+ WIC");
    platform_fru_table[fru_table_offset].pid_string = ngwic_get_pid; 
    platform_fru_table[fru_table_offset].location_string = ngwic_get_loc;

    /* Segment 2: Test step captured from prpass */
    /* Segment 3: Failure message captured from cterr */

    /* Segment 4: Components used */
    cterr_add_component("NGIO", "88E1512", "BCM63168", "DDR RAM");

    /* Segment 5: register and memory dump */
    //cterr_add_reg_dump((PFV)bcm63168_get_xdsl_profile);

    /* Segment 6: Platform Environment initialized here*/
    //cterr_add_env_dump((PFV)show_margins_cterr_wrapper);

    /* Segment 7: Top 3 Debugging Steps */
    cterr_add_debug("Switch console to BCM side, and make sure BCM can reach "
                    "Host by typing \"ping 192.123.123.1\". "
                    "If ping fails, check two paths. "
                    "One is the interface between host and Marvell1512, the "
                    "other one is RGMII between the Marvell1512 and BCM63168.",
                    "Check the interface between the BCM63168 and the DDR3.",
                    "If there is no problem on these interfaces, replace one "
                    "DDR3 and redo the test."
                    ); 
#endif 

    retval = goldschlager_nc_dispatch_comm(DIAG_COMMAND_BCM63268_DRAM_TEST,
                                           bcm_op_mode,
                                           bcm_idle_listen_params, 
                                           bcm_line_id);

    if (retval != PASSED) {
        cterr('f', 0, "%s failed.", __FUNCTION__);
        return (retval);
    }
    
    return (retval);
}

/*******************************************************************************
 *
 * Function   : goldschlager_do_showtime
 *
 * Description: The function to train w/ DSLAM.
 *
 * Inputs     : NONE 
 *             
 * Outputs    : NONE
 *
 *******************************************************************************
 */
static int goldschlager_do_showtime (void)
{
    int retval = FAILED;
#if ENHANCED_ERR_MSG_EXAMPLE
    uchar ngwic_get_loc[FRU_SIZE] = {0};
    uchar ngwic_get_pid[FRU_SIZE] = {0};
#endif 

    testname("Goldschlager Showtime");
    
#ifdef ENHANCED_ERR_MSG_EXAMPLE

    /*
     * 1. Subtests of the test function will reuse all variables
     * 2. All variables will be cleared automatically when
     *    entering and leaving each menu item.
     */
    /* Segment 1: PID | Unique_string : slot_info */
    fru_table_offset = WIC0 + goldschlager_test_slot - 1;
    /* fru_table_offset should be set, otherwise, it will not */
    /* go to enhanced error message format in cterr() */
    /* set fru_table_offset to get the predefine value */
    /* or change mb_pid & mb_loc below */
    memcpy(ngwic_get_pid,(char*)&goldschlager_board_id, 2);
    strcpy((char *)ngwic_get_loc, "MB/VDSL2+ WIC");
    platform_fru_table[fru_table_offset].pid_string = ngwic_get_pid; 
    platform_fru_table[fru_table_offset].location_string = ngwic_get_loc;

    /* Segment 2: Test step captured from prpass */
    /* Segment 3: Failure message captured from cterr */

    /* Segment 4: Components used */
    cterr_add_component("NGIO", "88E1512", "BCM63168");

    /* Segment 5: register and memory dump */
    //cterr_add_reg_dump((PFV)bcm63168_get_xdsl_profile);

    /* Segment 6: Platform Environment initialized here*/
    //cterr_add_env_dump((PFV)show_margins_cterr_wrapper);

    /* Segment 7: Top 3 Debugging Steps */
    cterr_add_debug("Switch console to BCM side, and make sure BCM can reach "
                    "Host by typing \"ping 192.123.123.1\". "
                    "If ping fails, check two paths. "
                    "One is the interface between host and Marvell1512, the "
                    "other one is RGMII between the Marvell1512 and BCM63168.",
                    "Switch console to the BCM side, type "
                    "\"xdslctl profile --show\" and \"xdslctl1 profile --show\""
                    "to show the xdsl mode setting and configuration at "
                    "internal and external channel. Check the image type "
                    "and default AFE are in expected mode.",
                    "Check the DSLAM configuration.",
                    "Make sure the cable is plugged in properly.",
                    "Execute the \"Do Send All Tones\" test to verify the TX "
                    "of AFE component is good. If the test fails, check the "
                    "interface between the BCM63168 and RJ14 port.",
                    "Execute the \"Do Idle Listen\" test to verify the RX of "
                    "AFE component is good. If the test fails, check the "
                    "interface between the BCM63168 and RJ14 port."
                    ); 
#endif 

    if ((retval = bcm63268_showtime(goldschlager_wic_iface)) != PASSED) {
        cterr('f', 0, "%s failed.", __FUNCTION__);
        return (retval);
    }

    printf("%s passed.\n", __FUNCTION__);
    return (retval);
}

/*******************************************************************************
 *
 * Function   : goldschlager_do_set_tones
 *
 * Description: The function to configure DSL parameters.
 *
 * Inputs     : NONE 
 *             
 * Outputs    : NONE
 *
 *******************************************************************************
 */
static int goldschlager_do_set_tone (void)
{
    int retval = FAILED;

    if ((retval = bcm63268_set_tone(goldschlager_wic_iface)) != PASSED) {
        cterr('f', 0, "%s failed.", __FUNCTION__);
        return (retval);
    }
    
    printf("%s passed.\n", __FUNCTION__);
    return (retval);
}

/*******************************************************************************
 *
 * Function   : goldschlager_do_send_all_tone
 *
 * Description: The function to perform set all tone test.
 *
 * Inputs     : NONE 
 *             
 * Outputs    : NONE
 *
 *******************************************************************************
 */
static int goldschlager_do_send_all_tone (void)
{
    int retval = FAILED;

    if ((retval = bcm63268_send_all_tone(goldschlager_wic_iface)) != PASSED) {
        cterr('f', 0, "%s failed.", __FUNCTION__);
        return (retval);
    }
    
    printf("%s passed.\n", __FUNCTION__);
    return (retval);
}

/*******************************************************************************
 *
 * Function   : goldschlager_do_idle_listen
 *
 * Description: The function to perform do idle listen test.
 *
 * Inputs     : NONE 
 *             
 * Outputs    : NONE
 *
 *******************************************************************************
 */
static int goldschlager_do_idle_listen (void)
{
    int retval = FAILED;

    if ((retval = bcm63268_idle_listen(goldschlager_wic_iface)) != PASSED) {
        cterr('f', 0, "%s failed.", __FUNCTION__);
        return (retval);
    }
    
    printf("%s passed.\n", __FUNCTION__);
    return (retval);
}

/*******************************************************************************
 *
 * Function   : goldschlager_select_test_options
 *
 * Description: The function to configure DSL parameters.
 *
 * Inputs     : NONE 
 *             
 * Outputs    : NONE
 *
 *******************************************************************************
 */
static int goldschlager_select_test_options (void)
{
    int retval = FAILED;

    if ((retval = bcm63268_vdsl_test_option_select(goldschlager_wic_iface))
        != PASSED) {
        cterr('f', 0, "%s failed.", __FUNCTION__);
        return (retval);
    }
    
    return (retval);
}

/*******************************************************************************
 *
 * Function: goldschlager_uart_test
 *
 * Description: Test the UART connection from the host to Goldschlager.
 *              Also test the GE0 interface by checking diag image 
 *              download successful or not.
 *
 * Input : None
 *
 * Output: PASSED/FAILED
 *
 *******************************************************************************
 */
static int goldschlager_uart_test ()
{
    const int maxlen = 28;
    char test_if[maxlen];;
    int rv; 

    testname("Goldschlager UART");  
    
    /* 'n\n' for trigger goldschlager side diag sub-item,
     * which will invoke 'uname -a'.
     */
    assert(goldschlager_wic_iface);

    /* Reset BCM */
    goldschlager_bcm963268_reset();
    msleep(2000);

    snprintf(test_if, maxlen-1, "/dev/ttyDASH%d",
             goldschlager_wic_iface->uart_ctrl); 

#ifdef TACHI
    /* Setup FPGA BMC UART connection */
    if ((diag_uart_to_nim_cnnt(goldschlager_wic_iface->slot)) == FAILED) {
        printf("%s: Fails in NIM UART connection\n", __FUNCTION__);
        return (FAILED);
    }
    if (goldschlager_wic_iface->mod_type == SM_DAUGHTER_CARD) {
        snprintf(test_if, maxlen-1, "/dev/ttyS%d",goldschlager_wic_iface->slot-1);
    } else {
        snprintf(test_if, maxlen-1, "/dev/ttyS%d",goldschlager_wic_iface->slot+1);
    }
#endif

    prpass(testpass, "Goldschlager UART,");

    rv = uart_msg_exh_test(test_if, "\n", "Auto", TRIG_DIAG_M); 
    if (rv == FAILED) {
        cterr('f',0,"Goldschlager UART test failed\n");
    }

    rv = uart_msg_exh_test(test_if, "\n\n", "CFE>", TRIG_DIAG_M); 
    if (rv == FAILED) {
        cterr('f',0,"Goldschlager UART test failed\n");
    }
    return (rv);
}

/*******************************************************************************
 *
 * Function: ltc4215_register_test
 *
 * Description: A wrapper function for LTC4215 register test.
 *
 * Input : None 
 *
 * Output: PASSED/FAILED
 *
 *******************************************************************************
 */
static int ltc4215_register_test (void)
{
#if ENHANCED_ERR_MSG_EXAMPLE
    uchar ngwic_get_loc[FRU_SIZE] = {0};
    uchar ngwic_get_pid[FRU_SIZE] = {0};
#endif 

    testname("LTC4215 OIR Register");    
    
#ifdef ENHANCED_ERR_MSG_EXAMPLE
    /*
     * 1. Subtests of the test function will reuse all variables
     * 2. All variables will be cleared automatically when
     *    entering and leaving each menu item.
     */
    /* Segment 1: PID | Unique_string : slot_info */
    fru_table_offset = WIC0 + goldschlager_test_slot - 1;
    /* fru_table_offset should be set, otherwise, it will not */
    /* go to enhanced error message format in cterr() */
    /* set fru_table_offset to get the predefine value */
    /* or change mb_pid & mb_loc below */
    memcpy(ngwic_get_pid,(char*)&goldschlager_board_id, 2);
    strcpy((char *)ngwic_get_loc, "MB/VDSL2+ WIC");
    platform_fru_table[fru_table_offset].pid_string = ngwic_get_pid; 
    platform_fru_table[fru_table_offset].location_string = ngwic_get_loc;

    /* Segment 2: Test step captured from prpass */
    /* Segment 3: Failure message captured from cterr */

    /* Segment 4: Components used */
    cterr_add_component("NGIO", "LTC4215");

    /* Segment 5: register and memory dump */
    //cterr_add_reg_dump((PFV)bcm63168_get_xdsl_profile);

    /* Segment 6: Platform Environment initialized here*/
    //cterr_add_env_dump((PFV)show_margins_cterr_wrapper);

    /* Segment 7: Top 3 Debugging Steps */
    cterr_add_debug("Using \"LTC4215 Register Write\" utility to write value "
                    "0x55 into offset 0x1 register in LTC4215.",
                    "Using \"LTC4215 Register Read\" to check the value in "
                    "offset 0x1 register is 0x55. If not, check the I2C "
                    "interface between the host and LTC4215.",
                    "Back to step1 and use value 0xaa redo the debugging step.",
                    "If there is no problem on I2C interface, replace one "
                    "LTC4215 and redo the test."
                    ); 
#endif 

    return (oir_ltc4215_register_test(oir_if));
}


/*******************************************************************************
 *
 * Function: ltc4215_reg_write
 *
 * Description: LTC4215 Register Write utility.
 *
 * Input : None.
 *
 * Output: PASSED/FAILED
 *
 *******************************************************************************
 */
static int ltc4215_reg_write(void)
{
    return (util_oir_ltc4215_reg_write(oir_if));
}

/*******************************************************************************
 *
 * Function: ltc4215_reg_read
 *
 * Description: LTC4215 Register Read utility.
 *
 * Input : None.
 *
 * Output: PASSED/FAILED
 *
 *******************************************************************************
 */
static int ltc4215_reg_read(void)
{
    return (util_oir_ltc4215_reg_read(oir_if));
}

/*******************************************************************************
 *
 * Function   : ltc4215_led_test
 *
 * Description: Wrapped function to do LTC4215 LED test.
 *
 * Inputs     : None 
 *
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
static int ltc4215_led_test (void)
{
    testname("LTC4215 OIR LED");
    return (oir_ltc4215_leds_test(oir_if));
}

/*******************************************************************************
 *
 * Function   : oir_ltc4215_tests
 *
 * Description: Entry function of Goldschlager OIR(LTC4215)
 *              Diag tests and utilities.
 *
 * Inputs     : show menu option
 *
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
static int oir_ltc4215_tests (int show_menu)
{
    testname("LTC4215 OIR");

    build_primary_submenu(oir_submenu_table, 
                          OIR_SUBMENU_TABLE_SIZE,
                          "LTC4215 OIR", &oir_submenup);
    build_secondary_submenu(oir_submenu_table,
                            OIR_SUBMENU_TABLE_SIZE,
                            oir_tests_secondary_items);

    if (show_menu) {
        menu(oir_submenup, oir_tests_secondary_items, '\0');
    } else {
        do_all_menu_items(oir_submenup);
    }

    return (PASSED);
}

/*******************************************************************************
 *
 * Function   : goldschlager_nim_tests
 *
 * Description: Entry function of Goldschlager Diag tests and utilities.
 *
 * Inputs     : show menu option
 *
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
static int goldschlager_nim_tests (int show_menu)
{
    testname("Goldschlager NIM");

    /* Check Goldschlager is ready */
    if (pri_intf_rdy_chk() != PASSED) {
        printf ("\nGoldschlager NIM is not ready."
                "Please boot up Goldschlager first.\n");
        return (PASSED);
    }

    build_primary_submenu(gs_nim_submenu_table, 
                          GS_NIM_SUBMENU_TABLE_SIZE,
                          "Goldschlager NIM", &gs_nim_submenup);
    build_secondary_submenu(gs_nim_submenu_table,
                            GS_NIM_SUBMENU_TABLE_SIZE,
                            gs_nim_tests_secondary_items);

    if (show_menu) {
        menu(gs_nim_submenup, gs_nim_tests_secondary_items, '\0');
    } else {
        do_all_menu_items(gs_nim_submenup);
    }

    return (PASSED);
}


/*******************************************************************************
 *
 * Function: goldschlager_pwr_off
 *
 * Description: This function power off goldschlager NIM.
 *
 * Input :  None
 *
 * Output: PASSED/FAILED
 *
 *******************************************************************************
 */
static int goldschlager_pwr_off (void)
{
    uint8_t data = 0;

    assert (oir_if);

    printf ("\nPower Off the goldschlager NGWIC.\n");

    if (util_oir_ltc4215_led(oir_if, OIR_LED_OFF)) {
        return (FAILED);
    }

    if (oir_ltc4215_reg_read(oir_if, LTC4215_CONTROL_REG, &data)) {
        return(FAILED);
    }

    /* power off NGWIC module */
    data &= ~LTC4215_FET_ON_CONTROL;
    if (oir_ltc4215_reg_write(oir_if, LTC4215_CONTROL_REG, &data)) {
        return (FAILED);
    }

    return (PASSED);
}

/*******************************************************************************
 *
 * Function: goldschlager_power_off
 *
 * Description: This function is a wrapper to power off goldschlager NIM.
 *
 * Input :  None
 *
 * Output: PASSED/FAILED
 *
 *******************************************************************************
 */
static int goldschlager_power_off (void)
{
    uint8_t ans;

    printf ("\n\nProceed with Power Off? (y/n) ");
    ans = getchar();
    putchar (ans);
    printf("\n\n");
    
    if (ans != 'y' && ans != 'Y') {
        printf("\nPower Off ABORT! goldschlager NIM Still Power On.\n\n");
        return (PASSED);
    }

    return (goldschlager_pwr_off());
}

/*******************************************************************************
 *
 * Function: goldschlager_pwr_on
 *
 * Description: This function power on goldschlager NIM.
 *
 * Input :    None
 *
 * Output: PASSED/FAILED
 *
 *******************************************************************************
 */
static int goldschlager_pwr_on (void)
{
    uint8_t  data = 0;

    printf("\nPower On the goldschlager NGWIC.\n");

    assert(goldschlager_wic_iface);
    assert(oir_if);

    /* turn on board power and take I2C out of reset */
    slot_i2c_unreset(goldschlager_wic_iface, goldschlager_wic_iface->slot, "WIC");

    if (util_oir_ltc4215_led(oir_if, OIR_LED_AMBER_ONLY)) {
        return (FAILED);
    }

    if (oir_ltc4215_reg_read(oir_if, LTC4215_CONTROL_REG, &data)) {
        return (FAILED);
    }

    /* power on NGWIC module */
    data |= LTC4215_FET_ON_CONTROL;
    if (oir_ltc4215_reg_write(oir_if, LTC4215_CONTROL_REG, &data)) {
        return (FAILED);
    }
    msleep(200);

    /* make sure the power is output good */
    if (oir_ltc4215_reg_read(oir_if, LTC4215_STATUS_REG, &data)) {
        return(FAILED);
    }
    if (!(data & LTC4215_FET_ON_STATUS)) {
        printf ("FET CANNOT be Turned On.\n");
        return (FAILED);
    }
    if (data & LTC4215_POWER_BAD_STATUS) {
        printf ("Power CANNOT be Turned On.\n");
        return (FAILED);
    }

    printf ("Waiting for goldschlager NGWIC to Power-Up.\n");
    msleep (2000);

    /* turn on the green light */
    if (util_oir_ltc4215_led(oir_if, OIR_LED_GREEN_ONLY)) {
        return (FAILED);
    }

    goldschlager_wic_iface->uart_on(goldschlager_wic_iface);    

    /* take goldschlager NGWIC out of reset */
    goldschlager_wic_iface->unreset(goldschlager_wic_iface);

    printf("Goldschlager NIM is powered up.\n");

    return (PASSED);
}

/*******************************************************************************
 *
 * Function: goldschlager_pwr_cycle
 *
 * Description: A wrapper function for LTC4215 Power Cycle test.
 *
 * Input : None 
 *
 * Output: PASSED/FAILED
 *
 *******************************************************************************
 */
static int goldschlager_pwr_cycle (void)
{
    uint8_t i, ans;

    printf("\n");
    printf("Power Cycle the goldschlager NGWIC");

    printf("\n\nProceed with Power Cycle? (y/n) ");
    ans = getchar ();
    putchar (ans);
    printf("\n\n");
    if (ans != 'y' && ans != 'Y') {
        printf("\nPower Cycle ABORT! "
                "goldschlager is not Power Cycled.\n\n");
        return (PASSED);
    }

    if (goldschlager_pwr_off()) {
        cterr('f', 0, "Failed to Power Off the goldschlager NGWIC");
        return (FAILED);
    }

    /* msleep for 10 seconds. */
    for (i = 0; i < 10; i++) {
        printf (".");
        msleep (1000);
    }

    if (goldschlager_pwr_on()) {
        cterr('f', 0, "Failed to Power On the Goldschlager NIM");
        return (FAILED);
    }

    return (PASSED);
}

/*******************************************************************************
 *
 * Function   : goldschlager_console_switch
 *
 * Description: A utility to Console Switch to Goldschlger BRCM console.
 *
 * Inputs     : NONE 
 *             
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
static int goldschlager_console_switch (void)
{
    int slot;
    const int maxlen = 128;
    char cmd[maxlen];
    speed_t new_baud = 0;

    assert (goldschlager_wic_iface);
    slot = goldschlager_wic_iface->slot;
    assert ((slot == 1) || (slot == 2) || (slot == 3));

    printf("\n\n ### NOTE: Type CTRL-a followed by CTRL-x "
                          "to switch back to host's console\n\n");
    fflush(stdout);
    fflush(stderr);
    msleep(1000); // pause a second for the NOTE:

    if (slot == GOLDSCHLAGER_NIM_SLOT1) {
        new_baud = slot1_uart;
    } else if (slot == GOLDSCHLAGER_NIM_SLOT2){
        new_baud = slot2_uart;
    } else {
        new_baud = slot3_uart;
    }    

#ifdef TACHI
    /* Setup FPGA BMC UART connection */
    if ((diag_uart_to_nim_cnnt(slot)) == FAILED) {
        printf("%s: Fails in NIM UART connection\n", __FUNCTION__);
        return (FAILED);
    }
    snprintf(cmd, maxlen-1, "picocom -b9600 -d8 -pn -fn /dev/ttyS2");
#else
    snprintf(cmd, maxlen-1, "picocom -%s -d8 -pn -fn /dev/ttyDASH%d",
             new_baud ? "b9600" : "b115200", goldschlager_wic_iface->uart_ctrl); 
#endif

#if DEBUG_UARTCOM
    printf("cmd=%s\n", cmd);
#endif
    system(cmd);

    return(PASSED);
}

/*******************************************************************************
 *
 * Function   : goldschlager_bcm963268_reset
 *
 * Description: The functions to reset BCM63268.
 *
 * Inputs     : NONE 
 *             
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
int goldschlager_bcm963268_reset (void)
{
    goldschlager_wic_iface->reset(goldschlager_wic_iface);
    msleep (500);
    goldschlager_wic_iface->unreset(goldschlager_wic_iface);
    msleep (500);
    
    return (PASSED);
}

/*******************************************************************************
 *
 * Function   : goldschlager_init_bcm63268
 *
 * Description: The functions to init BCM63268.
 *
 * Inputs     : NONE 
 *             
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
static int goldschlager_init_bcm63268 (void)
{
    int retval = FAILED;
    assert (goldschlager_wic_iface);
    
    if ((retval = bcm63268_initialize(goldschlager_wic_iface)) != PASSED) {
        printf("%s fail\n", __FUNCTION__);
        return (retval);
    }
    
    return (retval);
}

/*******************************************************************************
 *
 * Function   : goldschlager_config_bcm63268
 *
 * Description: The function to configure DSL profile
 *
 * Inputs     : NONE 
 *             
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
static int goldschlager_config_bcm63268 (void)
{
    int retval = FAILED;
    assert (goldschlager_wic_iface);
    
    if ((retval = bcm63268_configure(goldschlager_wic_iface)) != PASSED) {
        printf("%s fail\n", __FUNCTION__);
        return (retval);
        }
    
    return (retval);
}

/*******************************************************************************
 *
 * Function   : goldschlager_get_bcm63268_config
 *
 * Description: The functions to get DSL profile.
 *
 * Inputs     : NONE 
 *             
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
static int goldschlager_get_bcm63268_config (void)
{
    int retval = FAILED;
    assert (goldschlager_wic_iface);
    
    if ((retval = bcm63268_get_configure(goldschlager_wic_iface)) != PASSED) {
        printf("%s fail\n", __FUNCTION__);
        return (retval);
    }
    
    return (retval);
}

/*******************************************************************************
 *
 * Function   : goldschlager_get_bcm63268_version
 *
 * Description: The functions to get DSL driver version.
 *
 * Inputs     : NONE 
 *             
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
static int goldschlager_get_bcm63268_version (void)
{
    int retval = FAILED;
    assert (goldschlager_wic_iface);
    
    if ((retval = bcm63268_get_version(goldschlager_wic_iface)) != PASSED) {
        printf("%s fail\n", __FUNCTION__);
        return (retval);
    }
    
    return (retval);
}

/*******************************************************************************
 *
 * Function   : goldschlager_connection_start
 *
 * Description: The functions to start to train with DSLAM.
 *
 * Inputs     : NONE 
 *             
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
static int goldschlager_connection_start (void)
{
    int retval = FAILED;
    assert (goldschlager_wic_iface);
    
    if ((retval = bcm63268_do_showtime(goldschlager_wic_iface)) != PASSED) {
        printf("%s fail\n", __FUNCTION__);
        return (retval);
    }
    
    return (retval);
}

/*******************************************************************************
 *
 * Function   : goldschlager_connection_stop
 *
 * Description: The functions to stop DSL from showtime.
 *
 * Inputs     : NONE 
 *             
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
static int goldschlager_connection_stop (void)
{
    int retval = FAILED;
    assert (goldschlager_wic_iface);
    
    if ((retval = bcm63268_connection_stop(goldschlager_wic_iface)) != PASSED) {
        printf("%s fail\n", __FUNCTION__);
        return (retval);
    }
    
    return (retval);
}

/*******************************************************************************
 *
 * Function   : goldschlager_get_xdsl_mib_info
 *
 * Description: The functions to get DSL inforamtion.
 *
 * Inputs     : NONE 
 *             
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
static int goldschlager_get_xdsl_mib_info (void)
{
    int retval = FAILED;
    assert (goldschlager_wic_iface);
    
    if ((retval = bcm63268_get_adslmib_info(goldschlager_wic_iface)) != PASSED) {
        printf("%s fail\n", __FUNCTION__);
        return (retval);
    }
    
    return (retval);
}

/*******************************************************************************
 *
 * Function   : goldschlager_get_xtm_bonding_info
 *
 * Description: The functions to get DSL bonding status.
 *
 * Inputs     : NONE 
 *             
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
static int goldschlager_get_xtm_bonding_info (void)
{
    int retval = FAILED;
    assert (goldschlager_wic_iface);
    
    if ((retval = bcm63268_get_xtm_bonding_info(goldschlager_wic_iface)) != PASSED) {
        printf("%s fail\n", __FUNCTION__);
        return (retval);
    }
    
    return (retval);
}

/*******************************************************************************
 *
 * Function   : goldschlager_get_xdsl_info
 *
 * Description: The functions to get DSL inforamtion.
 *
 * Inputs     : NONE 
 *             
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
static int goldschlager_get_xdsl_info (void)
{
    int retval = FAILED;
    assert (goldschlager_wic_iface);
    
    if ((retval = bcm63268_get_xdsl_info(goldschlager_wic_iface)) != PASSED) {
        printf("%s fail\n", __FUNCTION__);
        return (retval);
    }
    
    return (retval);
}

/*******************************************************************************
 *
 * Function   : goldschlager_get_connection_info
 *
 * Description: The function to get connection information.
 *
 * Inputs     : NONE 
 *             
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
static int goldschlager_get_connection_info (void)
{
    int retval = FAILED;
    assert (goldschlager_wic_iface);
    
    if ((retval = bcm63268_get_conn_info(goldschlager_wic_iface)) != PASSED) {
        printf("%s fail\n", __FUNCTION__);
        return (retval);
    }
    
    return (retval);
}

/*******************************************************************************
 *
 * Function   : goldschlager_bcm63268_volt_normal
 *
 * Description: The function set voltage margin to normal
 *
 * Inputs     : NONE 
 *             
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
static int goldschlager_bcm63268_volt_normal (void)
{
    int retval = FAILED;
    assert (goldschlager_wic_iface);

    if ((retval = bcm63268_volt_normal(goldschlager_wic_iface)) != PASSED) {
        printf("%s fail\n", __FUNCTION__);
        return (retval);
    }
    
    return (retval);
}

/*******************************************************************************
 *
 * Function   : goldschlager_bcm63268_volt_high
 *
 * Description: The function set voltage margin high
 *
 * Inputs     : NONE 
 *             
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
static int goldschlager_bcm63268_volt_high (void)
{
    int retval = FAILED;
    assert (goldschlager_wic_iface);
    
    if ((retval = bcm63268_volt_high(goldschlager_wic_iface)) != PASSED) {
        printf("%s fail\n", __FUNCTION__);
        return (retval);
    }
    
    return (retval);
}

/*******************************************************************************
 *
 * Function   : goldschlager_bcm63268_volt_low
 *
 * Description: The function set voltage margin low
 *
 * Inputs     : NONE 
 *             
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
static int goldschlager_bcm63268_volt_low (void)
{
    int retval = FAILED;
    assert (goldschlager_wic_iface);
    
    if ((retval = bcm63268_volt_low(goldschlager_wic_iface)) != PASSED) {
        printf("%s fail\n", __FUNCTION__);
        return (retval);
    }
    
    return (retval);
}

/*******************************************************************************
 *
 * Function   : goldschlager_bcm63268_led_test
 *
 * Description: The fucntion to perform Link LEDs test
 *
 * Inputs     : NONE 
 *             
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
static int goldschlager_bcm63268_led_test (void)
{
    int retval = FAILED;
    assert (goldschlager_wic_iface);
    
    /* Check Goldschlager is ready */
    if (pri_intf_rdy_chk() != PASSED) {
        printf ("\nGoldschlager NIM is not ready."
                "Please boot up Goldschlager first.\n");
        return (PASSED);
    }
    
    if ((retval = bcm63268_led_test(goldschlager_wic_iface)) != PASSED) {
        cterr('f', 0, "%s failed.", __FUNCTION__);
        return (retval);
    }
    
    return (retval);
}

/*******************************************************************************
 *
 * Function   : goldschlager_bcm63268_show_profile
 *
 * Description: The fucntion to show DSL profile
 *
 * Inputs     : NONE 
 *             
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
static int goldschlager_bcm63268_show_profile (void)
{
    int retval = FAILED;
    assert (goldschlager_wic_iface);
    
    if ((retval = bcm63268_show_profile(goldschlager_wic_iface)) != PASSED) {
        printf("%s fail\n", __FUNCTION__);
        return (retval);
    }
    
    return (retval);
}

/*******************************************************************************
 *
 * Function   : goldschlager_check_sku_type
 *
 * Description: Wrap function to check if SKU ID in GS HW and in ACT2 
 *              are identical.
 *
 * Inputs     : NONE 
 *             
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
static int goldschlager_check_sku_type (void)
{
    int retval = FAILED;
    assert (goldschlager_wic_iface);

    if ((retval = bcm63268_check_sku_type(goldschlager_wic_iface)) != PASSED) {
        printf("%s failed.", __FUNCTION__);
        return (retval);
    }

    return (retval);
}

/*******************************************************************************
 *
 * Function   : boot_sel_pin_test
 *
 * Description: Test EXP_BOOT_SEL(IO1) on GPIO Expander(PCA9557PW).
 *
 * Inputs     : None
 *
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
static int goldschlager_boot_sel_pin_test (void)
{
    int retval = FAILED;
    uchar            orig_cfg = 0, orig_out = 0, data = 0, boot_sel_mask = 0x02;
    n2g_i2c_if_t     *io_exp;
#if ENHANCED_ERR_MSG_EXAMPLE
    uchar ngwic_get_loc[FRU_SIZE] = {0};
    uchar ngwic_get_pid[FRU_SIZE] = {0};
#endif 

#ifdef ENHANCED_ERR_MSG_EXAMPLE
    /*
     * 1. Subtests of the test function will reuse all variables
     * 2. All variables will be cleared automatically when
     *    entering and leaving each menu item.
     */
    /* Segment 1: PID | Unique_string : slot_info */
    fru_table_offset = WIC0 + goldschlager_test_slot - 1;
    /* fru_table_offset should be set, otherwise, it will not */
    /* go to enhanced error message format in cterr() */
    /* set fru_table_offset to get the predefine value */
    /* or change mb_pid & mb_loc below */
    memcpy(ngwic_get_pid,(char*)&goldschlager_board_id, 2);
    strcpy((char *)ngwic_get_loc, "MB/VDSL2+ WIC");
    platform_fru_table[fru_table_offset].pid_string = ngwic_get_pid; 
    platform_fru_table[fru_table_offset].location_string = ngwic_get_loc;

    /* Segment 2: Test step captured from prpass */
    /* Segment 3: Failure message captured from cterr */

    /* Segment 4: Components used */
    cterr_add_component("NGIO", "PCA9557", "BCM963168");

    /* Segment 5: register and memory dump */
    //cterr_add_reg_dump((PFV)bcm63168_get_xdsl_profile);

    /* Segment 6: Platform Environment initialized here*/
    //cterr_add_env_dump((PFV)show_margins_cterr_wrapper);

    /* Segment 7: Top 3 Debugging Steps */
    cterr_add_debug("Switch console to BCM side, and make sure BCM can reach "
                    "Host by typing \"ping 192.123.123.1\". "
                    "If ping fails, check two paths. "
                    "One is the interface between host and Marvell1512, the "
                    "other one is RGMII between the Marvell1512 and BCM63168.",
                    "Using \"IO Expander(PCA9557) Reg. Write\" utility to write "
                    "the value 0xFD into offset 0x3 register. And write 0x00 "
                    "into offset 0x1 register.",
                    "Using \"IO Expander(PCA9557) Reg. Dump\" utility to dump "
                    "all register in PCA9557. Check the the value in offset "
                    "0x3 register is 0xFD and the value in offset 0x1 register "
                    "is 0x00. If not, check the path between the host and GPIO "
                    "Expander(PCA9557PW).",
                    "Switch console to BCM side, and type \"dw 0xb00000cc 1\" "
                    "to dump the memory value, check whether the value is "
                    "0xFFEFFEFF. If not, check the GPIO20 on BCM63168."
                    ); 
#endif 

    testname("Boot Select Pin Test");

    io_exp = &pca_i2c[0];

    /* Read GPIO_20 by BCM63168 to check the value 
     *  Make sure GPIO_20 is High by default.
     */
    retval = goldschlager_nc_dispatch_comm(DIAG_COMMAND_BOOT_SEL_PIN_HIGH,
                                           bcm_op_mode,
                                           bcm_idle_listen_params, 
                                           bcm_line_id);

    if (retval != PASSED) {
        cterr('f', 0, "%s failed. Default value of GPIO20 is not high.\n", __FUNCTION__);
        return (retval);
    }

    /* Backup original value */
    if (io_port_8bit_i2c_read(io_exp, PCA9557_OUT_PORT_REG, &orig_out, TRUE)) {
        cterr('f', 0, "\n\nFailed to read IO Expander(PCA9557) OUT register\n");
        return (FAILED);
    }

    if (io_port_8bit_i2c_read(io_exp, PCA9557_CFG_PORT_REG, &orig_cfg, TRUE)) {
        cterr('f', 0, "\n\nFailed to read IO Expander(PCA9557) CFG register\n");
        return (FAILED);
    }

    /* Config EXP_BOOT_SEL(IO1) as output*/
    data = orig_cfg & ~(boot_sel_mask);

    if (io_port_8bit_i2c_write(io_exp, PCA9557_CFG_PORT_REG, &data)) {
        cterr('f', 0, "%s: Failed to wrote 0x%02X to IO Expander CFG Reg.",
                      __FUNCTION__, data);
        return (FAILED);
    }

    /* Pull EXP_BOOT_SEL(IO1) low */
        data = orig_out & ~(boot_sel_mask);

    if (io_port_8bit_i2c_write(io_exp, PCA9557_OUT_PORT_REG, &data)) {
        cterr('f', 0, "%s: Failed to wrote 0x%02X to IO Expander OUT Reg.",
                      __FUNCTION__, data);
        return (FAILED);
    }

    /* Read GPIO_20 by BCM63168 to check whether the value is low */
    retval = goldschlager_nc_dispatch_comm(DIAG_COMMAND_BOOT_SEL_PIN_LOW,
                                           bcm_op_mode,
                                           bcm_idle_listen_params, 
                                           bcm_line_id);

    if (retval != PASSED) {
        cterr('f', 0, "%s failed. GPIO20 is not low.\n", __FUNCTION__);
        return (retval);
    }

    /* Restore original value */
    if (io_port_8bit_i2c_write(io_exp, PCA9557_OUT_PORT_REG, &orig_out)) {
        cterr('f', 0, "%s: Failed to restore 0x%02X to IO Expander OUT Reg.",
                      __FUNCTION__, orig_out);
        return (FAILED);
    }

    if (io_port_8bit_i2c_write(io_exp, PCA9557_CFG_PORT_REG, &orig_cfg)) {
        cterr('f', 0, "%s: Failed to restore 0x%02X to IO Expander CFG Reg.",
                      __FUNCTION__, orig_cfg);
        return (FAILED);
    }

    /* Read GPIO_20 by BCM63168 to check whether the value is high */
    retval = goldschlager_nc_dispatch_comm(DIAG_COMMAND_BOOT_SEL_PIN_HIGH,
                                           bcm_op_mode,
                                           bcm_idle_listen_params, 
                                           bcm_line_id);

    if (retval != PASSED) {
        cterr('f', 0, "%s failed. GPIO20 is not high.\n", __FUNCTION__);
        return (retval);
    }

    return (retval);
}

/*******************************************************************************
 *
 * Function   : pri_intf_rdy_chk
 *
 * Description: Check if EXP_PRI_RDY(IO3) is asserted.
 *
 * Inputs     : None
 *
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
static int pri_intf_rdy_chk (void)
{
    uint32_t         ctr = 0;
    uchar            data = 0, pri_rdy_mask = 0x08;
    n2g_i2c_if_t     *io_exp;

    io_exp = &pca_i2c[0];

    if (io_port_8bit_i2c_read(io_exp, PCA9557_IN_PORT_REG, &data, TRUE)) {
        printf("\n\nFailed to read IO Expander(PCA9557)"
               " register 0x%02X.\n\n", ctr);
        return (FAILED);
    }

    /* Check EXP_PRI_RDY(IO3) */
    if (data & pri_rdy_mask) {
        return (PASSED);
    } else {
        return (FAILED);
    }

}

/*******************************************************************************
 *
 * Function   : pca9557_reg_dump_util
 *
 * Description: Wrap utility to dump all registers of PCA9557
 *              IO Expander(PCA9557).
 *
 * Inputs     : None
 *
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
static int pca9557_reg_dump_util (void)
{
    uint32_t         ctr = 0, total_reg_num = 0;
    uchar            data = 0, reg_data[8];
    reg_info_t       *reg_p = 0;
    n2g_i2c_if_t     *io_exp;

    io_exp = &pca_i2c[0];

    memset((uchar *)reg_data, 0, (sizeof(reg_data)/sizeof(uchar))); 
    total_reg_num = (sizeof(pca9557_reg_tbl) / sizeof(reg_info_t));

    for (ctr = 0; ctr < total_reg_num; ctr++) {
        data = 0;

        if (io_port_8bit_i2c_read(io_exp, ctr, &data, TRUE)) {
            printf("\n\nFailed to read IO Expander(PCA9557)"
                   " register 0x%02X.\n\n", ctr);
            return (FAILED);
        }
        reg_data[ctr] = data;
    }

    printf("\nGoldschlager IO Expander(PCA9557) registers dump:\n");
    for (ctr = 0; ctr < total_reg_num; ctr++) {
        reg_p = &pca9557_reg_tbl[ctr];
        printf("%-25s Reg.(0x%01X) = 0x%02X.\n",
               reg_p->name, reg_p->offset, reg_data[ctr]);
    }

    return (PASSED);
}

/*******************************************************************************
 *
 * Function   : pca9557_reg_read_util
 *
 * Description: Wrap utility to read specific register of PCA9557
 *              IO Expander(PCA9557).
 *
 * Inputs     : None
 *
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
static int pca9557_reg_read_util (void)
{
    uint32_t         offset = 0;
    uchar            data = 0;
    reg_info_t       *reg_p = 0;
    n2g_i2c_if_t     *io_exp;

    io_exp = &pca_i2c[0];

    offset = (uint32_t)gethex_answer("Enter offset of register:", 0, 0, 0x7);

    if (io_port_8bit_i2c_read(io_exp, offset, &data, TRUE)) {
        printf("\n\nFailed to read IO Expander(PCA9557) register 0x%02X.\n\n",
               offset);
        return (FAILED);
    }

    reg_p = &pca9557_reg_tbl[offset];
    printf("\nIO Expander(PCA9557) %s Reg.(0x%01X): 0x%02X.\n\n",
           reg_p->name, reg_p->offset, data);

    return (PASSED);
}

/*******************************************************************************
 *
 * Function   : pca9557_reg_write_util
 *
 * Description: Wrap utility to write specific register of PCA9557
 *              IO Expander(PCA9557).
 *
 * Inputs     : None
 *
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
static int pca9557_reg_write_util (void)
{
    uint32_t         offset = 0;
    uchar            data = 0;
    reg_info_t       *reg_p = 0;
    n2g_i2c_if_t     *io_exp;

    io_exp = &pca_i2c[0];

    offset = (uint32_t)gethex_answer("Enter offset of register:", 0, 0, 0x7);

    reg_p = &pca9557_reg_tbl[offset];

    if (reg_p->offset == PCA9557_IN_PORT_REG) {
        printf("\n\n %s Reg.(0x%01X) is an input-only register"
               " it's prohibited to write this register.\n\n",
               reg_p->name, offset);
        return (PASSED);
    }

    data = (uchar)gethex_answer("Enter write-in Data:", 0, 0, 0xFF);

    if (io_port_8bit_i2c_write(io_exp, offset, &data)) {
        printf("\n\nFailed to write 0x%02X to IO Expander(PCA9557)"
               " register 0x%02X.\n\n", data, offset);
        return (FAILED);
    }

    printf("\nDone write 0x%02X to IO Expander(PCA9557) %s Reg.(0x%01X).\n\n",
           data, reg_p->name, reg_p->offset);

    return (PASSED);
}

/*******************************************************************************
 *
 * Function: util_goldschlager_uart_baud_rate_set
 *
 * Description: This function sets the uart baud rate
 *
 * Input : None 
 *
 * Output: PASSED/FAILED.
 *
 *******************************************************************************
 */
static int util_goldschlager_uart_baud_rate_set (void)
{
    const   int maxlen = 128;
    char    tty[maxlen];
    int     fd, slot;
    struct  termios  newtio, ori_conf;
    speed_t new_baud = 0, uart_baud_rate = 0;

    slot = goldschlager_wic_iface->slot;
    assert((slot == 1) || (slot == 2) || (slot == 3));

    snprintf(tty, maxlen-1, "/dev/ttyDASH%d",
             goldschlager_wic_iface->uart_ctrl);

#ifdef TACHI
    /* Setup FPGA BMC UART connection */
    if ((diag_uart_to_nim_cnnt(slot)) == FAILED) {
        printf("%s: Fails in NIM UART connection\n", __FUNCTION__);
        return (FAILED);
    }
    if (goldschlager_wic_iface->mod_type == SM_DAUGHTER_CARD) {
        snprintf(tty, maxlen-1, "/dev/ttyS%d",slot-1);
    } else {
        snprintf(tty, maxlen-1, "/dev/ttyS%d",slot+1);
    }
#endif

    fd = open(tty, O_RDWR|O_NOCTTY);
    if (fd < 0) {
        perror(tty);
        exit(1);
    }
    tcgetattr(fd, &newtio);

    printf("\n\n Set Goldschlager UART Baud Rate: \n");
    uart_baud_rate = getdec_answer("\nBaudrate (0-115200, 1-9600):", 0, 0, 1);
    if (slot == GOLDSCHLAGER_NIM_SLOT1) {
        slot1_uart = uart_baud_rate;
    } else if (slot == GOLDSCHLAGER_NIM_SLOT2) {
        slot2_uart = uart_baud_rate;
    } else {
        slot3_uart = uart_baud_rate;
    }
    
    new_baud = goldschlager_uart_baud[uart_baud_rate].baud_rate;

    /* Backup default config for recover after test */
    memcpy(&ori_conf, &newtio, sizeof(newtio));

    if ((newtio.c_cflag & CBAUD) != new_baud) {
        if (cfsetospeed(&newtio, new_baud) < 0) {
            tcsetattr(fd, TCSAFLUSH, &ori_conf);
            close(fd);
            cterr('f', 0, "Failed to set output speed.");
            return (FAILED);
        }

        if (cfsetispeed(&newtio, new_baud) < 0) {
            tcsetattr(fd, TCSAFLUSH, &ori_conf);
            close(fd);
            cterr('f', 0, "Failed to set intput speed.");
            return (FAILED);
        }
    }
    close(fd);
    return (PASSED);
}

/*******************************************************************************
 *
 * Function: goldschlager_bcm63268_show_spi_flash_reg
 *
 * Description: This function displays Goldschlager SPI flash registers.
 *
 * Input : None
 *
 * Output: PASSED/FAILED.
 *
 *******************************************************************************
 */
static int goldschlager_bcm63268_show_spi_flash_reg (void)
{
    int retval = FAILED;
    assert (goldschlager_wic_iface);

    if ((retval = bcm63268_show_spi_flash_reg(goldschlager_wic_iface)) != PASSED) {
        printf("%s fail\n", __FUNCTION__);
        return (retval);
    }

    return (retval);
}

/******** History ********/
/*------------------------------------------------------------------------------
 * $Log: ngwic_goldschlager.c,v $
 * Revision 1.16  2020/05/22 02:28:30  qingcwan
 * Merge switzer-carrier code into main chunk.
 *
 * Revision 1.15  2020/01/09 01:02:12  jiajliu
 * Merge Curie 2RU to main trunk
 *
 * Revision 1.14  2019/10/17 02:16:18  kehuang2
 * Collapse Tabei-L into main trunk
 *
 * Revision 1.13  2019/08/06 06:56:09  alpeng
 * merge curie, switzer and nightwatch to trunk
 *
 * Revision 1.12  2018/12/18 13:47:23  hondwang
 * Fix make issue
 *
 * Revision 1.11  2018/12/18 13:29:11  hondwang
 * Fix CDETs CSCvn58971 with Goldschlager and Wallander on Tachi-L
 *
 * Revision 1.10  2018/05/22 02:31:11  alpeng
 * fixed compiler warning, CSCvj57934
 *
 * Revision 1.9  2018/05/18 09:24:50  alpeng
 *  Neptune merge to trunk with tag <neptune-branch-0518>
 *
 * Revision 1.8  2017/07/14 02:51:38  alpeng
 * fixed compiler warning, due to cross-compiler version was updated.
 *
 * Revision 1.7  2017/03/30 09:00:41  hondwang
 * Tachi-L brach merge
 *
 * Revision 1.6.34.3  2017/03/30 08:30:52  hondwang
 * Tachi-L brach merge
 *
 * Revision 1.6.34.2  2016/12/22 08:46:29  haohsu
 * Add Goldschlager on Tachi-l
 *
 * Revision 1.6.34.1  2016/12/22 01:09:08  haohsu
 * Add Goldshlager on Tachi-l
 *
 * Revision 1.6.22.2  2018/05/17 10:50:22  alpeng
 *  sync with trunk <trunk-051618>
 *
 * Revision 1.6.22.1  2016/11/09 07:18:33  alpeng
 * update goldschlager uart func; code reviewed by mecca
 *
 * Revision 1.8  2017/07/14 02:51:38  alpeng
 * fixed compiler warning, due to cross-compiler version was updated.
 *
 * Revision 1.7  2017/03/30 09:00:41  hondwang
 * Tachi-L brach merge
 *
 * Revision 1.6.34.3  2017/03/30 08:30:52  hondwang
 * Tachi-L brach merge
 *
 * Revision 1.6.34.2  2016/12/22 08:46:29  haohsu
 * Add Goldschlager on Tachi-l
 *
 * Revision 1.6.34.1  2016/12/22 01:09:08  haohsu
 * Add Goldshlager on Tachi-l
 *
 * Revision 1.6  2015/02/14 03:43:11  jamlin
 * adding close uart fd before return
 *
 * Revision 1.5  2015/02/13 12:26:49  meho
 * Added the utility to read Boardcom SPI flash registers
 *
 * Revision 1.4  2015/02/12 13:39:41  jamlin
 * CFE parameter set enhancement and bug fix.
 *
 * Revision 1.3  2014/10/13 06:58:09  bowang3
 * Make NIM support NGSM carrier card Thule
 *
 * Revision 1.2  2014/09/17 03:32:16  jamlin
 * Add support for Goldschlager NIM.
 *
 * Revision 1.1.6.6  2014/08/27 02:39:12  jamlin
 * Change fflush(0) to fflush(stdout).
 *
 * Revision 1.1.6.5  2014/08/15 12:03:12  jamlin
 * Fix GS interface test bug.
 *
 * Revision 1.1.6.4  2014/08/15 12:00:42  jamlin
 * Enhance boot up message.
 *
 * Revision 1.1.6.3  2014/08/15 11:57:42  jamlin
 * Fix uart test util bug.
 *
 * Revision 1.1.6.2  2014/08/08 02:43:57  jamlin
 * goladschlager-branch3 initail commit.
 *
 * Revision 1.1.4.17  2014/06/30 09:59:52  jamlin
 * restore oir register test
 *
 * Revision 1.1.4.16  2014/06/30 08:35:22  jamlin
 * fix return value.
 *
 * Revision 1.1.4.15  2014/06/26 11:40:22  jamlin
 * add GS bootup item in submenu.
 *
 * Revision 1.1.4.14  2014/06/12 07:05:33  jamlin
 * Change diag name to nim_gs_x_diag.img
 *
 * Revision 1.1.4.13  2014/05/30 10:03:06  jamlin
 * Fix bug while executing slot test.
 *
 * Revision 1.1.4.12  2014/05/27 06:26:50  jamlin
 * Enhance the PID check.
 *
 * Revision 1.1.4.11  2014/04/11 03:50:04  jamlin
 * GS annexB PID changes from NIM-VAB-B tp NIM-VA-B
 *
 * Revision 1.1.4.10  2014/04/08 13:12:31  jamlin
 * Checkin enhanced error message.
 *
 * Revision 1.1.4.9  2014/03/11 09:48:03  jamlin
 * added check NG-Module image exist in firmware directory function
 *
 * Revision 1.1.4.8  2014/02/10 04:49:53  jamlin
 * added boot_sel_pin_test and clean up the test menu
 *
 * Revision 1.1.4.7  2014/02/10 04:17:34  jamlin
 * added get_xdsl_profile function
 *
 * Revision 1.1.4.6  2014/02/10 04:03:18  jamlin
 * rename nc_dispatch_linkstatus to nc_dispatch_return_value function and added check_sku_type function
 *
 * Revision 1.1.4.5  2014/02/10 03:32:21  jamlin
 * added bcm_bonding_state_get function and fixed showtime bonding issue
 *
 * Revision 1.1.4.4  2014/01/23 09:18:40  jamlin
 * Let GS know which platform it resides in.
 *
 * Revision 1.1.4.3  2014/01/07 07:27:41  jamlin
 * Add Goldschlager support on Utah platform.
 *
 * Revision 1.1.4.2  2014/01/07 01:54:52  jamlin
 * Goldschlager new branch goldschlager-branch2
 *
 * Revision 1.1.2.7  2013/12/25 08:54:33  jamlin
 * Add primary interface ready check before performing NC.
 *
 * Revision 1.1.2.6  2013/12/04 06:25:29  jamlin
 * Fix P1A2 PID extra 7 white space impact.
 *
 * Revision 1.1.2.5  2013/12/04 01:38:52  jamlin
 * Support Bonding channels showtime status display.
 *
 * Revision 1.1.2.4  2013/11/22 12:50:21  jamlin
 * Add PID check to differentiate GS SKUs.
 *
 * Revision 1.1.2.3  2013/11/13 01:30:19  jamlin
 * Add set idle mode utility in PT test.
 *
 * Revision 1.1.2.2  2013/11/07 10:29:49  jamlin
 * Fix showtime test submenu problem.
 *
 * Revision 1.1.2.1  2013/11/02 13:39:51  jamlin
 * Initial commit for bringup.
 *
 *
 *------------------------------------------------------------------------------
 * $Endlog$
 *------------------------------------------------------------------------------
 */
