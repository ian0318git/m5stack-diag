/* $Id: ngsm_oakenshield.c,v 1.7 2020/01/09 01:02:17 jiajliu Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/oakenshield/ngsm_oakenshield.c,v $ 
 *******************************************************************************
 * ngsm_oakenshield.c : Oakenshield code
 *
 * Sep 2016 - Owen Lin
 *
 * Copyright (c) 2019 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *******************************************************************************
 */
#include <fcntl.h>
#include <net/if.h>
#include <string.h>
#include <pthread.h>
#include "common.h"
#include "stdio.h"
#include "stdlib.h"
#include "types.h"
#include "assert.h"
#include "defs.h"
#include "setjmps.h"
#include "signals.h"
#include "menu.h"
#include "error.h"
#include "proto.h"
#include "nvmonvars.h"
#include "router_if.h"
#include "platform_i2c.h"
#include "cli_cmd.h"
#include "nmc93c46.h"
#include "platform_cookie.h"
#include "ngio.h"
#include "cross_platform.h"
#include "pca.h"
#include "slot.h"
#include "smart_cookie.h"
#include "cookie_4.h"
#include "plat_defs.h"
#include "sgmii_defs.h"
#include "i2c_api.h"
#include "ngsm_oakenshield.h"
#include "common_utils.h"
#include "platform_eth_pkt_txrx.h"
#include "linux_api.h"
#include "bcm_gesw_defs.h"
#include "dash_fpga.h"
#include "dev_ltc4215.h"
#include "oir_ltc4215_api.h"

extern int skip_oakenshield_ge1_lpbk(void);
extern int tftp_get (unsigned char *dir, unsigned char *file,
     unsigned char *server_ip, unsigned char *dest, unsigned int check);
static struct ngio_intf_t *ngio_ptr;
static ngsm_entity_t oak_lsi_entity[MAX_NUM_NGSM];
static char pca_buff[256];
static void *oir_if;
static unsigned char fpga_image[0x100000];

int oak_dsp_test(int , int , int , int);
static int oak_wait_result_packet(uint8_t *, int, uint16_t, int);
static int oakenshield_con_utils(void);
static int oak_dsp_ddr3_sdram_test_wrapper(void);
static int oak_ge_lpbk_test_wrapper(int);
static int enable_bp_ge_lpbk(int);
static int disable_bp_ge_lpbk(int);
static int oak_tdm_ext_lpbk_test_wrapper(void);
static int ecc_mem_test(void);
static int oak_arm11_cpu1_boot_test_wrapper(void);
static int oak_dss_core0_sanity_test_wrapper(void);
static int oak_dss_core1_sanity_test_wrapper(void);
static int oak_dss_core2_sanity_test_wrapper(void);
static int oak_dss_core3_sanity_test_wrapper(void);
static int oakenshield_pwr_on(void);
static int oak_disp_mac(void);
static int oak_disp_mem(void);
static int oak_disp_fw_ver(void);
int oak_gpio_resistor_test(void);
static int gpio_exp_read(void);
static int gpio_exp_write(void);
static int oak_dac_1dot5sm_high(void);
static int oak_dac_1dot5sm_low(void);
static int oak_dac_no_1dot5sm(void);
static int oak_dac_3dot3sm_high(void);
static int oak_dac_3dot3sm_low(void);
static int oak_dac_no_3dot3sm(void);
static int oak_dac_1DOT5_show(void);
static int oak_dac_3DOT3_show(void);
static int oak_select_test(int, int, int, int);
static int oakenshield_test(int);
static int oakenshield_utils(void);
static int oak_uart_test(void);
static int oak_gpio_exp_test(void);
static int oakenshield_bringup_dsp(void);
static int oakenshield_ready_test(int);
static int oak_setup_ge_env(void);
static void oak_clear_rx_buf(uchar *, int);
static void oak_build_command_packet(uint16_t, uint32_t, uint32_t, uint8_t);
static void oak_build_config_packet(void);
static void (*oakenshield_saved_diag_exec)(void) = NULL;
static int oak_send_command_packet(int dsp_no);
static int oak_cleanup_ge_env(void);
static int oak_eth_frames_test(uint32, uint32, int, mac_addr_t, int);
static int oak_wait_for_ge_packet(uchar *, int, int, int, int);
static int ltc4215_register_test (void);
static int ltc4215_led_test(void);
static int oir_ltc4215_tests(int);
static int ltc4215_reg_write(void);
static int ltc4215_reg_read(void);
static long util_show_oir_ltc4215_regs (void);
static int fxs_si32261_lpbk_test(void);
static int fxo_si3050_lpbk_test(void);
static int fxs_si32261_common_mode_calibration (int);
static int fxs_si32261_ring (void);
static boolean is_fxo (void);
static int oakenshield_pwr_off (void);
static void oak_ngio_off (void);
static void *ngsm_p;
static int fpga_reg_test (void);
static int fpga_mem_test (void);
static int fpga_int_test (void);
static int fpga_tdmsw_force_test (void);
static int led_utility (void);
static int fail_over_port_utility (void);
static int oakenshield_run_test (void);
static int vol_margin_utility (int);
static int fpga_reg_utility (int);
static int read_fpga_dir_reg (void);
static int write_fpga_dir_reg (void);
static int read_fpga_indir_reg (void);
static int write_fpga_indir_reg (void);
static int fpga_upgrade_utility (void);
ushort oak_board_id;
char fru_board_id[10];

#define FXS_MAX_CALIBRATION_DATA_SIZE  (72*4*2)
/* A mask bit to check if the cookie size is 512 */
#define COOKIE_SIZE_IS_512      (0x1 << 31)
#define LONG_CALI_TYPE_EX       0xf7 
#define ENHANCED_ERR_MSG_EXAMPLE 1
#define TIME_OUT 1000000 /* fgpa upgrade time out */


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
 * Voltage Margin menu items
 *=========================================
 */
static submenu_xtable_t vol_submenu_table[] = {
    {"1.5 Voltage Margin High", (PFT)oak_dac_1dot5sm_high, 0,0,(type_t(*)())0,
                                    0, (type_t(*)())0, 0},
    {"1.5 Voltage Margin Low",  (PFT)oak_dac_1dot5sm_low,  0,0,(type_t(*)())0,
                                    0, (type_t(*)())0, 0},
    {"1.5 No Voltage Margin",   (PFT)oak_dac_no_1dot5sm,    0,0,(type_t(*)())0,
                                    0, (type_t(*)())0, 0},
    {"3.3 Voltage Margin High", (PFT)oak_dac_3dot3sm_high, 0,0,(type_t(*)())0,
                                    0, (type_t(*)())0, 0},
    {"3.3 Voltage Margin Low",  (PFT)oak_dac_3dot3sm_low,  0,0,(type_t(*)())0,
                                    0, (type_t(*)())0, 0},
    {"3.3 No Voltage Margin",   (PFT)oak_dac_no_3dot3sm,    0,0,(type_t(*)())0,
                                    0, (type_t(*)())0, 0},
    {"Show 1.5 Voltage Margin", (PFT)oak_dac_1DOT5_show,    0,0,(type_t(*)())0,
                                    0, (type_t(*)())0, 0},
    {"Show 3.3 Voltage Margin", (PFT)oak_dac_3DOT3_show,    0,0,(type_t(*)())0,
                                    0, (type_t(*)())0, 0},
};

#define VOL_SUBMENU_TABLE_SIZE (sizeof(vol_submenu_table) / \
                                sizeof(submenu_xtable_t))

/* 
 * Primary & secondary submenu items (filled in from xtable)
 */
static mitem_t vol_tests_primary_items[VOL_SUBMENU_TABLE_SIZE + 
                                       MAX_BASE_ITEMS];
static mitem_t vol_tests_secondary_items[VOL_SUBMENU_TABLE_SIZE +
                                         MAX_BASE_ITEMS];

static menuinfo_t vol_subtest_menu = {
    "%s Subtest Menu",
    0,                                /* mtparam added by init_empty_menu */
    (PFT)show_endnote,                /* notes missing WICs in combos */
    0,                                /* use generic prompt */
    0,                                /* size (bumped by add_menu_item()) */
    vol_tests_primary_items,
};

static menuinfo_t *vol_submenup = &vol_subtest_menu;

/*=========================================
 * FPGA Register menu items
 *=========================================
 */
static submenu_xtable_t fpga_submenu_table[] = {
    {"Read FPGA direct Register",   (PFT)read_fpga_dir_reg,    0,0,(type_t(*)())0,
                                    0, (type_t(*)())0, 0},
    {"Write FPGA direct Register",  (PFT)write_fpga_dir_reg,    0,0,(type_t(*)())0,
                                    0, (type_t(*)())0, 0},
    {"Read FPGA indirect Register", (PFT)read_fpga_indir_reg,    0,0,(type_t(*)())0,
                                    0, (type_t(*)())0, 0},
    {"Write FPGA indirect Register",(PFT)write_fpga_indir_reg,    0,0,(type_t(*)())0,
                                    0, (type_t(*)())0, 0},

};

#define FPGA_SUBMENU_TABLE_SIZE (sizeof(fpga_submenu_table) / \
                                sizeof(submenu_xtable_t))

/* 
 * Primary & secondary submenu items (filled in from xtable)
 */
static mitem_t fpga_tests_primary_items[FPGA_SUBMENU_TABLE_SIZE + 
                                       MAX_BASE_ITEMS];
static mitem_t fpga_tests_secondary_items[FPGA_SUBMENU_TABLE_SIZE +
                                         MAX_BASE_ITEMS];

static menuinfo_t fpga_subtest_menu = {
    "%s Subtest Menu",
    0,                                /* mtparam added by init_empty_menu */
    (PFT)show_endnote,                /* notes missing WICs in combos */
    0,                                /* use generic prompt */
    0,                                /* size (bumped by add_menu_item()) */
    fpga_tests_primary_items,
};

static menuinfo_t *fpga_submenup = &fpga_subtest_menu;


/* Variable to select to run tests on host using ethernet interface or run 
 * tests on the DSP using uart interface */
int oak_dsp_tests_use_enet = 1;

submenu_xtable_t oak_tests_submenu_table[] = {
    {"Oakenshield Utilities",   (PFT)oakenshield_utils,    0,0,(type_t(*)())0,
                                    0, (type_t(*)())0, 0},
    {"DDR3 Memory test", (PFT)oak_dsp_ddr3_sdram_test_wrapper,         0,MM_2, 
                         (type_t(*)())0, 0, (type_t(*)())0, 0},
    {"EMAC0 loopback test", (PFT)oak_ge_lpbk_test_wrapper,    0,MM_2, 
                         (type_t(*)())0, 0,     (type_t(*)())0, 0},
    {"EMAC1 loopback test", (PFT)oak_ge_lpbk_test_wrapper,    1,MM_2, 
                         (type_t(*)())0, 0,     (type_t(*)())0, 0},
    {"ARM11 CPU1 Boot test ", (PFT)oak_arm11_cpu1_boot_test_wrapper, 0,MM_2, 
                         (type_t(*)())0, 0,     (type_t(*)())0, 0},
    {"DSS Core0 Sanity test ", (PFT)oak_dss_core0_sanity_test_wrapper, 0,MM_2, 
                         (type_t(*)())0, 0,     (type_t(*)())0, 0},
    {"DSS Core1 Sanity test ", (PFT)oak_dss_core1_sanity_test_wrapper, 0,MM_2, 
                         (type_t(*)())0, 0,     (type_t(*)())0, 0},
    {"DSS Core2 Sanity test ", (PFT)oak_dss_core2_sanity_test_wrapper, 0,MM_2, 
                         (type_t(*)())0, 0,     (type_t(*)())0, 0},
    {"DSS Core3 Sanity test ", (PFT)oak_dss_core3_sanity_test_wrapper, 0,MM_2, 
                         (type_t(*)())0, 0,     (type_t(*)())0, 0},
    {"ECC Memory test ",            (PFT)ecc_mem_test,                 0,MM_2, 
                         (type_t(*)())0, 0,     (type_t(*)())0, 0},
    {"FPGA Register test",         (PFT)fpga_reg_test,                 0,MM_2, 
                         (type_t(*)())0, 0,     (type_t(*)())0, 0},
    {"FPGA Memory test",           (PFT)fpga_mem_test,                 0,MM_2, 
                         (type_t(*)())0, 0,     (type_t(*)())0, 0},
    {"FPGA interrupt test",        (PFT)fpga_int_test,                 0,MM_2, 
                         (type_t(*)())0, 0,     (type_t(*)())0, 0},
    {"FPGA TDMSW16 force byte test", (PFT)fpga_tdmsw_force_test,       0,MM_2, 
                         (type_t(*)())0, 0,     (type_t(*)())0, 0},
    {"FPGA TDM Loopback test", (PFT)oak_tdm_ext_lpbk_test_wrapper,0,MM_2, 
                         (type_t(*)())0, 0,     (type_t(*)())0, 0},
    {"SI32261 Loopback test",       (PFT)fxs_si32261_lpbk_test,        0,MM_2,
                         (type_t(*)())0, 0,     (type_t(*)())0},
    {"SI3050 Loopack test",         (PFT)fxo_si3050_lpbk_test,         0,MM_2,
                         (type_t(*)())is_fxo, 0,     (type_t(*)())0},
    {"Uart Loopack test",           (PFT)oak_uart_test,                0,MM_2,
                         (type_t(*)())0, 0,     (type_t(*)())0},
};


#define OAK_TESTS_SUBMENU_TABLE_SIZE (sizeof(oak_tests_submenu_table) / \
                                       sizeof(submenu_xtable_t))

/* 
 * Primary & secondary submenu items (filled in from xtable)
 */
static mitem_t oak_tests_primary_items[OAK_TESTS_SUBMENU_TABLE_SIZE + 10];
static mitem_t oak_tests_secondary_items[OAK_TESTS_SUBMENU_TABLE_SIZE + 10];

menuinfo_t oak_subtest_menu = {
    "%s Subtest Menu",
    0,                                /* mtparam added by init_empty_menu */
    (PFT)show_endnote,                /* notes missing WICs in combos */
    0,                                /* use generic prompt */
    0,                                /* size (bumped by add_menu_item() */
    oak_tests_primary_items,
};
menuinfo_t *oak_submenup = &oak_subtest_menu;

submenu_xtable_t oak_utils_submenu_table[] = {
    {"Oakenshield ngio on",           (PFT)oakenshield_pwr_on,0,0,(type_t(*)())0,
                                    0, (type_t(*)())0},
    {"Oakenshield ngio off",          (PFT)oak_ngio_off,  0,0,(type_t(*)())0,
                                    0, (type_t(*)())0},
    {"Switch Console",                 (PFT)oakenshield_con_utils,  0,0,(type_t(*)())0,
                                    0, (type_t(*)())0},
    {"Display Oakenshield MAC",        (PFT)oak_disp_mac,     0,0,(type_t(*)())0, 
                                    0, (type_t(*)())0},
    {"Display Oakenshield Firmware Version",     
                                    (PFT)oak_disp_fw_ver,  0,0,(type_t(*)())0,
                                    0, (type_t(*)())0},
    {"Display Oakenshield Memory",     (PFT)oak_disp_mem,     0,0,(type_t(*)())0,
                                    0, (type_t(*)())0},
    {"PCA9557 Register Read",       (PFT)gpio_exp_read,      0,0,(type_t(*)())0, 
                                    0, (type_t(*)())0},
    {"PCA9557 Register Write",      (PFT)gpio_exp_write,     0,0,(type_t(*)())0, 
                                    0, (type_t(*)())0},
    {"LTC4215 Register Test",       (PFT)ltc4215_register_test, 0,   0,(type_t(*)())0,
                                    0, (type_t(*)())0,          0},
    {"LTC4215 Register Read",       (PFT)ltc4215_reg_read,      0,   0,(type_t(*)())0,
                                    0,         (type_t(*)())0,          0},
    {"LTC4215 Register Write",      (PFT)ltc4215_reg_write,     0,   0,(type_t(*)())0,
                                    0,         (type_t(*)())0,          0},
    {"LTC4215 Register show",       (PFT)util_show_oir_ltc4215_regs, 0,  0,(type_t(*)())0,
                                    0,         (type_t(*)())0,          0},
    {"Enable GE0 Loopback",         (PFT)enable_bp_ge_lpbk,  0,0,(type_t(*)())0,
                                    0, (type_t(*)())0},
    {"Disable GE0 Loopback",        (PFT)disable_bp_ge_lpbk, 0,0,(type_t(*)())0,
                                    0, (type_t(*)())0},
    {"Enable GE1 Loopback",         (PFT)enable_bp_ge_lpbk,  1,0,(type_t(*)())0,
                                    0, (type_t(*)())0},
    {"Disable GE1 Loopback",        (PFT)disable_bp_ge_lpbk, 1,0,(type_t(*)())0,
                                    0, (type_t(*)())0},
    {"Voltage Margin Utility",      (PFT)vol_margin_utility,  0,0,(type_t(*)())0,
                                    0, (type_t(*)())0, 0},
    {"Failed Over Port Utility",    (PFT)fail_over_port_utility,   0,0,(type_t(*)())0,
                                    0, (type_t(*)())0, 0},
    {"LED Utility",                 (PFT)led_utility,   0,0,(type_t(*)())0,
                                    0, (type_t(*)())0, 0},
    {"FXS Calibration Utility",     (PFT)fxs_si32261_common_mode_calibration,  0,0,(type_t(*)())0,
                                    0, (type_t(*)())0, 0},
    {"FXS Ring Utility",            (PFT)fxs_si32261_ring,  0,0,(type_t(*)())0,
                                    0, (type_t(*)())0, 0},
    {"FPGA Register Utility",        (PFT)fpga_reg_utility,   0,0,(type_t(*)())0,
                                    0, (type_t(*)())0, 0},
    {"FPGA Upgrade Utility",        (PFT)fpga_upgrade_utility,  0,0,(type_t(*)())0,
                                    0, (type_t(*)())0, 0},
};

#define OAK_UTILS_SUBMENU_TABLE_SZ (sizeof(oak_utils_submenu_table) / \
                                       sizeof(submenu_xtable_t))

/* 
 * Primary & secondary submenu items (filled in from xtable)
 */
static mitem_t oak_utils_primary_items[OAK_UTILS_SUBMENU_TABLE_SZ + MAX_BASE_ITEMS];
static mitem_t oak_utils_secondary_items[OAK_UTILS_SUBMENU_TABLE_SZ + MAX_BASE_ITEMS];
char oakutiltitle[50];

menuinfo_t oak_util_submenu = {
    oakutiltitle,
    0,                                /* mtparam added by init_empty_menu */
    (PFT)menu_show_dflags,                /* notes missing WICs in combos */
    0,                                /* use generic prompt */
    0,                                /* size (bumped by add_menu_item() */
    oak_utils_primary_items,
};

menuinfo_t *oak_util_submenup = &oak_util_submenu;

static submenu_xtable_t oakenshield_mainmenu_tbl[] = {
    {"Oakenshield Utilities",        (PFT)oakenshield_utils,    0,0,(type_t(*)())0,
                                    0,    (type_t(*)())0},
    {"GPIO Expander test",        (PFT)oak_gpio_exp_test,      0,MM_2,(type_t(*)())0,
                                    0,    (type_t(*)())0},
    {"LTC4215 OIR Test",          (PFT)oir_ltc4215_tests,         0,MM_2,(type_t(*)())0, 
                                    0,    (PFT)oir_ltc4215_tests,             TRUE},
    {"Oakenshield test",            (PFT)oakenshield_test,   TRUE,MM_2,(type_t(*)())0,
                                    0,    (PFT)oakenshield_test, FALSE},
};

#define OAKENSHIELD_MAINMENU_TBL_SIZE \
        (sizeof(oakenshield_mainmenu_tbl) / sizeof(submenu_xtable_t))

/*
 * Primary & secondary submenu items (filled in from xtable)
 */
static mitem_t main_menu_primary_items[OAKENSHIELD_MAINMENU_TBL_SIZE + MAX_BASE_ITEMS];
static mitem_t main_menu_secondary_items[OAKENSHIELD_MAINMENU_TBL_SIZE + MAX_BASE_ITEMS];

static title_buf_t  maindiag_header;

char oakmainmenutitle[50];
char oaksubmenutitle[50];
static struct menuinfo oakenshield_mainmenu = {
    /*"Oakenshield SM Main Menu",      *//* title */
    oakmainmenutitle,          /* title */
    0,                          /* title string added by init_empty_menu */
    (PFT)menu_show_dflags,      /* shows major flags */
    0,                          /* generic prompt */
    0,                          /* size -- bumped by add_menu_item() */
    main_menu_primary_items,
};

static struct menuinfo *maindiagp = &oakenshield_mainmenu;

static test_commands_t oakenshield_command[] = {
    {SELECT_DSP_SANITY,        "SELECT_DSP_SANITY"},       /* 01 DSP sanity */
    {SELECT_DSP_SDRAM,         "SELECT_DSP_SDRAM"},        /* 02 SDRAM test */
    {SELECT_DSS0_SANITY,       "SELECT_DSS0_SANITY"},      /* 03 DSS Integrity */
    {SELECT_DSS1_SANITY,       "SELECT_DSS1_SANITY"},      /* 04 */
    {SELECT_DSS2_SANITY,       "SELECT_DSS2_SANITY"},      /* 05 */
    {SELECT_DSS3_SANITY,       "SELECT_DSS3_SANITY"},      /* 06 */
    {SELECT_UART_TEST,         "SELECT_UART_TEST"},        /* 07 Test uart intf */
    {SELECT_DAC_1DOT5SM_HIGH,  "SELECT_DAC_1DOT5SM_HIGH"}, /* 0x0008 */
    {SELECT_DAC_1DOT5SM_LOW,   "SELECT_DAC_1DOT5SM_LOW"},  /* 0x0009 */
    {SELECT_DAC_NO_1DOT5SM,    "SELECT_DAC_NO_1DOT5SM"},   /* 0x000A */
    {SELECT_ECC_MEM,           "SELECT_ECC_MEM"},          /* 0x000B */
    {SELECT_DSP_CONSOLE,       "SELECT_DSP_CONSOLE"},      /* 0x000C */
    {SELECT_UART_LPBK,         "SELECT_UART_LPBK"},        /* 0x000D */
    {SELECT_UART_LPBK_RESULT,  "SELECT_UART_LPBK_RESULT"}, /* 0x0011 */
    {SELECT_UART_LPBK_STOP,    "SELECT_UART_LPBK_STOP"},   /* 0x0012 */
    {SELECT_UART_LPBK_RX,      "SELECT_UART_LPBK_RX"},     /* 0x0013 */
    {SELECT_DAC_3DOT3SM_HIGH,  "SELECT_DAC_3DOT3SM_HIGH"}, /* 0x0014 */
    {SELECT_DAC_3DOT3SM_LOW,   "SELECT_DAC_3DOT3SM_LOW"},  /* 0x0015 */
    {SELECT_DAC_NO_3DOT3SM,    "SELECT_DAC_NO_3DOT3SM"},   /* 0x0016 */
    {SELECT_ARM11CPU1_BOOT,    "SELECT_ARM11CPU1_BOOT"},   /* 0x0017 */
    {SELECT_DAC_1DOT5_SHOW,    "SELECT_DAC_1DOT5_SHOW"},   /* 0x0018 */
    {SELECT_DAC_3DOT3_SHOW,    "SELECT_DAC_3DOT3_SHOW"},   /* 0x0019 */
    {SELECT_GE0_LPBK,          "SELECT_GE0_LPBK"},         /* 0x20 Host-DSP Ge0 lpbk */
    {SELECT_GE1_LPBK,          "SELECT_GE1_LPBK"},         /* 0x21 Host-DSP GE1 lpbk */
    {SELECT_MEM_DISP,          "SELECT_MEM_DISP"},         /* 0x22 */
    {SELECT_FW_VER_DISP,       "SELECT_FW_VER_DISP"},      /* 0x23 */
    {SELECT_GE_LPBK_PF,        "SELECT_GE_LPBK_PF"},       /* 0x40 */
    {SELECT_GE0_LPBK_PT,       "SELECT_GE0_LPBK_PT"},      /* 0x80 pass through */
    {SELECT_GE1_LPBK_PT,       "SELECT_GE1_LPBK_PT"},      /* 0x81 pass through */
    {SELECT_INTF_SYNC,         "SELECT_INTF_SYNC"},        /* 0x82 SYNC signals */
    {SELECT_TDM_INTLPBK,       "SELECT_TDM_INTLPBK"},      /* 0x100 TDM int lpbk */
    {SELECT_TDM_EXTLPBK,       "SELECT_TDM_EXTLPBK"},      /* 0x200 TDM ext lpbk */
    {SELECT_READY,             "SELECT_READY"},            /* 0x300 DSP is ready */
    {SELECT_FPGA_REG_TEST,     "SELECT_FPGA_REG_TEST"},
    {SELECT_FPGA_MEM_TEST,     "SELECT_FPGA_MEM_TEST"},
    {SELECT_FPGA_INT_TEST,     "SELECT_FPGA_INT_TEST"},
    {SELECT_FPGA_TDMSW_FORCE_BYTE_TEST,     "SELECT_FPGA_TDMSW_FORCE_BYTE_TEST"},
    {SELECT_CODEC_SI32261_DIGITAL_LOOPBACK, "SELECT_SI32261_LOOPBACK"},
    {SELECT_CODEC_SI3050_DIGITAL_LOOPBACK, "SELECT_SI3050_LOOPBACK"},
    {SELECT_CODEC_SI32261_SET_RING, "SELECT_CODEC_SI32261_SET_RING"},
    {SELECT_CODEC_SI32261_STOP_RING, "SELECT_CODEC_SI32261_STOP_RING"},
    {SELECT_CODEC_SI32261_REG_READ, "SELECT_CODEC_SI32261_REG_READ"},
    {SELECT_CODEC_SI32261_REG_WRITE, "SELECT_CODEC_SI32261_REG_WRITE"},
    {SELECT_CODEC_SI32261_CALIBRATION, " SELECT_CODEC_SI32261_CALIBRATION"},
    {SELECT_CODEC_SI32261_CALIBRATE_RESULT, " SELECT_CODEC_SI32261_CALIBRATE_RESULT"},
    {SELECT_CODEC_SI32261_CALIBRATE_SAVE, " SELECT_CODEC_SI32261_CALIBRATE_SAVE"},
    {SELECT_CODEC_SI32261_RAM_READ, "SELECT_CODEC_SI32261_RAM_READ"},
    {SELECT_CODEC_SI32261_RAM_WRITE, "SELECT_CODEC_SI32261_RAM_WRITE"},
    {SELECT_CODEC_SI32261_PROTECTED, "SELECT_CODEC_SI32261_PROTECTED"},
    {SELECT_CODEC_SI3050_INIT, "SELECT_CODEC_SI3050_INIT"},
    {SELECT_CODEC_SI3050_REG_WR, "SELECT_CODEC_SI3050_REG_WR"},
    {SELECT_CODEC_SI3050_REG_RD, "SELECT_CODEC_SI3050_REG_RD"},
    {SELECT_FXS_FXO_LED, "SELECT_FXS_FXO_LED"},
    {SELECT_FAIL_OVERPORT, "SELECT_FAIL_OVERPORT"},
    {SELECT_CODEC_SET_FAIL_OVER_PORT, "SELECT_CODEC_SET_FAIL_OVER_PORT"},
    {SELECT_NULL,              "UNKNOWN_TEST"},       /* */
};


static pid_info_t PID_table[] = {
    {.pid = PID_8FXS_12FXO, .fxs_port = 8},
    {.pid = PID_16FXS_2FXO, .fxs_port = 16},
    {.pid = PID_24FXS_4FXO, .fxs_port = 24},
    {.pid = PID_72FXS,      .fxs_port = 72},
};
/*******************************************************************************
 *
 * Function: get_buf_message
 *
 * Description: This function will print the buffer which is dsp message. 
 *
 * Input : None 
 *
 * Output: None
 *
 *******************************************************************************
 */
void get_buf_message (void) 
{
    ngsm_entity_t *ep_p;
    fe_packet_t   *recv_packet_p;
    dspif_ether_t *result_packet_p;
    uint8_t bufmsg[128];

    assert(ngio_ptr);

    ep_p = (ngsm_entity_t *)ngio_ptr->priv;
    assert(ep_p);

    recv_packet_p = &(ep_p->recv_packet);
    result_packet_p = &(ep_p->result_packet);

    memcpy((char *)result_packet_p, (char *)(&(recv_packet_p->data[0])+0),
           sizeof(dspif_ether_t));

    memcpy(bufmsg, result_packet_p->dspif_info.bufmsg, sizeof(bufmsg));
    printf("\n %s\n", bufmsg);
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

    testname("LTC4215 OIR Register");
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

/**********************************************************************
 *
 * Function: show_oir_ltc4215_regs
 *
 * Display LTC4215 registers
 *
 * Input : none
 *
 * Output: none
 *
 **********************************************************************
 */
static void show_oir_ltc4215_regs (void)
{
    uint8_t data;

    printf("\nLTC4215 registers:\n");
    if (oir_ltc4215_reg_read(oir_if, LTC4215_CONTROL_REG, &data)) {
        printf(" Failed to read Canis LTC4215 CONTROL register\n");
    } else {
        printf(" LTC4215 CONTROL: 0x%02x\n", data);
    }
    if (oir_ltc4215_reg_read(oir_if, LTC4215_ALERT_REG, &data)) {
        printf(" Failed to read Canis LTC4215 ALERT register\n");
    } else {
        printf(" LTC4215 ALERT:   0x%02x\n", data);
    }
    if (oir_ltc4215_reg_read(oir_if, LTC4215_STATUS_REG, &data)) {
        printf(" Failed to read Canis LTC4215 STATUS register\n");
    } else {
        printf(" LTC4215 STATUS:  0x%02x\n", data);
    }
    if (oir_ltc4215_reg_read(oir_if, LTC4215_FAULT_REG, &data)) {
        printf(" Failed to read Canis LTC4215 FAULT register\n");
    } else {
        printf(" LTC4215 FAULT:   0x%02x\n", data);
    }
    if (oir_ltc4215_reg_read(oir_if, LTC4215_SENSE_REG, &data)) {
        printf(" Failed to read Canis LTC4215 SENSE register\n");
    } else {
        printf(" LTC4215 SENSE:   0x%02x\n", data);
    }
    if (oir_ltc4215_reg_read(oir_if, LTC4215_SOURCE_REG, &data)) {
        printf(" Failed to read Canis LTC4215 SOURCE register\n");
    } else {
        printf(" LTC4215 SOURCE:  0x%02x\n", data);
    }
}

/**********************************************************************
 *
 * Function: util_show_oir_ltc4215_regs
 *
 * Utility to display LTC4215 registers
 *
 * Input : none
 *
 * Output: PASSED
 *
 **********************************************************************
 */
static long util_show_oir_ltc4215_regs (void)
{
    show_oir_ltc4215_regs();

    return (PASSED);
}

/**********************************************************************
 *
 * Function: read_fpga_dir_reg
 *
 * Utility to display FPGA registers
 *
 * Input : none
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
static int read_fpga_dir_reg (void)
{
    int size;
    uint32_t offset;

    printf("Read FPGA direct Register \n");
    offset = gethex_answer("\nEnter FPGA general register offset[0x00 to 0x340]:",
                  0,0,0x340);
    size = gethex_answer("Size: ", 0, 0, 0xf);
    if (oak_select_test(SELECT_READ_FPGA_DIR_REG, offset, size, 100) == FAILED) {
        return (FAILED);
    }

    get_buf_message();     

    return (PASSED);

}

/**********************************************************************
 *
 * Function: write_fpga_dir_reg
 *
 * Utility to write FPGA registers
 *
 * Input : none
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
static int write_fpga_dir_reg (void)
{
    uint32_t value, offset, param, size;

    printf("Write FPGA direct Register \n");
    offset = gethex_answer("\nEnter FPGA general register offset[0x00 to 0x340]:",
                  0,0,0x340);
    size  = gethex_answer("\nSize: ", 0, 0, 0xf);
    value = gethex_answer("\nValue: ", 0, 0, 0xffff);
    param = (offset << 4) | size;

    if (oak_select_test(SELECT_WRITE_FPGA_DIR_REG, param, value, 100) == FAILED) {
        return(FAILED);
    }

    get_buf_message();     

    return (PASSED);

}

/**********************************************************************
 *
 * Function: read_fpga_indir_reg
 *
 * Utility to display FPGA registers
 *
 * Input : none
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
static int read_fpga_indir_reg (void)
{
    
    uint32_t addr;

    printf("Read FPGA indirect Register \n");
    addr = gethex_answer("Address: ", 0, 0x0, 0x81FC);
    if (oak_select_test(SELECT_READ_FPGA_INDIR_REG, addr, 0, 100) == FAILED) {
        return (FAILED);
    }

    get_buf_message();     

    return (PASSED);

}

/**********************************************************************
 *
 * Function: write_fpga_indir_reg
 *
 * Utility to write FPGA registers
 *
 * Input : none
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
static int write_fpga_indir_reg (void)
{
    uint32_t addr, value;

    printf("Write FPGA indirect Register \n");
    addr  = gethex_answer("Address: ", 0, 0x0, 0x81FC);
    value = gethex_answer("\nValue: ", 0, 0, 0xffff);
    if (oak_select_test(SELECT_WRITE_FPGA_INDIR_REG, addr, value, 100) == FAILED) {
        return (FAILED);
    }

    get_buf_message();     
    return (PASSED);

}


/*******************************************************************************
 *
 * Function   : vol_margin_utility
 *
 * Description: Entry function of Voltage Margin
 *
 * Inputs     : show menu option
 *
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
static int vol_margin_utility (int show_menu)
{

    build_primary_submenu(vol_submenu_table, 
                          VOL_SUBMENU_TABLE_SIZE,
                          "Voltage Margin", &vol_submenup);
    build_secondary_submenu(vol_submenu_table,
                            VOL_SUBMENU_TABLE_SIZE,
                            vol_tests_secondary_items);

    menu(vol_submenup, vol_tests_secondary_items, '\0');


    return (PASSED);
}


/*******************************************************************************
 *
 * Function   : fpga_register_utility
 *
 * Description: Entry function of FPGA Register
 *
 * Inputs     : show menu option
 *
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
static int fpga_reg_utility (int show_menu)
{
    build_primary_submenu(fpga_submenu_table, 
                          FPGA_SUBMENU_TABLE_SIZE,
                          "FPGA Register", &fpga_submenup);
    build_secondary_submenu(fpga_submenu_table,
                            FPGA_SUBMENU_TABLE_SIZE,
                            fpga_tests_secondary_items);
    menu(fpga_submenup, fpga_tests_secondary_items, '\0');
    return (PASSED);
}

/*******************************************************************************
 *
 * Function   : oir_ltc4215_tests
 *
 * Description: Entry function of OIR(LTC4215)
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


/**********************************************************************
 *
 * Function: oak_ngio_off
 *
 * This function power off Oakenshield
 *
 * Input : None
 *
 * Output: None
 *  
 **********************************************************************
 */
static void oak_ngio_off (void)
{
    ngsm_entity_t *oak_ep;
    int dsp;

    assert(ngio_ptr);

    ngio_ptr->off(ngio_ptr);
    msleep(5000);               /* HW need  */

    oak_ep = (ngsm_entity_t *)ngio_ptr->priv;

    for (dsp=0; dsp<MAX_DSPS_PER_NGSM; dsp++) {
        oak_ep->dsp_downloaded[dsp] = FALSE;
    }

}

/**********************************************************************
 *
 * Function: oakenshield_pwr_off
 *
 * This function reset the LSI SP27XX SM
 *
 * Input : None
 *
 * Output: PASSED/FAILED/ERROR
 *  
 **********************************************************************
 */
static int oakenshield_pwr_off (void)
{
    ngsm_entity_t *oakenshield_ep;
    uint8_t data = 0;
    int dsp;

    assert (oir_if);

    printf ("\nPower Off the Oakenshield DSP.\n");

    if (util_oir_ltc4215_led(oir_if, OIR_LED_OFF)) {
        return (FAILED);
    }

    if (oir_ltc4215_reg_read(oir_if, LTC4215_CONTROL_REG, &data)) {
        return (FAILED);
    }

    /* power off NGSM module */
    data &= ~LTC4215_FET_ON_CONTROL;
    if (oir_ltc4215_reg_write(oir_if, LTC4215_CONTROL_REG, &data)) {
        return (FAILED);
    }

    oakenshield_ep = (ngsm_entity_t *)ngio_ptr->priv;

    for (dsp = 0; dsp < MAX_DSPS_PER_NGSM; dsp++) {
        oakenshield_ep->dsp_downloaded[dsp] = FALSE;
    }

    return (PASSED);

}

/**********************************************************************
 *
 * Function: oakenshield_pwr_on
 *
 * This function unresets the LSI SP27XX SM
 *
 * Input : None
 *
 * Output: PASSED/FAILED/ERROR
 *  
 **********************************************************************
 */
static int oakenshield_pwr_on (void)
{
    uint8_t  data = 0;

    printf("\nPower On the Oakenshield NGSM.\n");

    assert(ngio_ptr);
    assert(oir_if);

    /* turn on board power and take I2C out of reset */
    slot_i2c_unreset(ngio_ptr, ngio_ptr->slot, "SM");

    if (util_oir_ltc4215_led(oir_if, OIR_LED_AMBER_ONLY)) {
        return (FAILED);
    }

    if (oir_ltc4215_reg_read(oir_if, LTC4215_CONTROL_REG, &data)) {
        return (FAILED);
    }

    /* power on NGSM module */
    data |= LTC4215_FET_ON_CONTROL;
    if (oir_ltc4215_reg_write(oir_if, LTC4215_CONTROL_REG, &data)) {
        return (FAILED);
    }
    msleep(200);

    /* make sure the power is output good */
    if (oir_ltc4215_reg_read(oir_if, LTC4215_STATUS_REG, &data)) {
        return (FAILED);
    }
    if (!(data & LTC4215_FET_ON_STATUS)) {
        printf ("FET CANNOT be Turned On.\n");
        return (FAILED);
    }
    if (data & LTC4215_POWER_BAD_STATUS) {
        printf ("Power CANNOT be Turned On.\n");
        return (FAILED);
    }

    printf ("Waiting for NGSM to Power-Up.\n");
    msleep (2000);

    /* turn on the green light */
    if (util_oir_ltc4215_led(oir_if, OIR_LED_GREEN_ONLY)) {
        printf ("fail to turn on green light.\n");
        return (FAILED);
    }

    ngio_ptr->uart_on(ngio_ptr);    

    /* Take  NGSM out of reset */
    ngio_ptr->unreset(ngio_ptr);

    if (oir_ltc4215_reg_read(oir_if, LTC4215_FAULT_REG, &data)) { 
        printf ("Read register LTC4215_FAULT_reg fail.\n");
        return (FAILED);
    }

    /* Set GPIO 3 */
    data &= ~(LTC4215_GPIO3_OUTPUT);
    if (oir_ltc4215_reg_write(oir_if, LTC4215_FAULT_REG, &data)) {
        printf ("Write register LTC4215_FAULT_reg fail.\n");
        return (FAILED);
    }

    /* Enable the ngio_sync_out_enable   */
    ngio_sync_out_enable (ngsm_p, ENABLE_PRE_SCALER_DIV_3125 |   SYNC_OUT_ENABLE );

    printf("NGSM is powered up.\n");

    return (PASSED);

}

/**********************************************************************
 *
 * Function: oak_disp_mac
 *  
 *  
 * Description: Display Oakenshield and Host MAC
 *
 * Input : None.
 *
 * Output: PASSED/FAILED   
 *  
 **********************************************************************
 */
static int oak_disp_mac (void)
{
    ngsm_entity_t *ep;

    assert(ngio_ptr);

    ep = (ngsm_entity_t *)ngio_ptr->priv;
    assert(ep);

    /* Display the NGSM module details */
    printf("\n%s", ep->name);

    printf("\nPID: %s ", ep->pid);


    printf("\n Oakenshield module MAC : 0x%02x:0x%02x:0x%02x:0x%02x:0x%02x:0x%02x ",
               (uchar)ep->eth_hdr[0].dest_addr[0], (uchar)ep->eth_hdr[0].dest_addr[1],
               (uchar)ep->eth_hdr[0].dest_addr[2], (uchar)ep->eth_hdr[0].dest_addr[3], 
               (uchar)ep->eth_hdr[0].dest_addr[4], (uchar)ep->eth_hdr[0].dest_addr[5]);

    printf("\n Host  MAC              : 0x%02x:0x%02x:0x%x02:0x%02x:0x%02x:0x%02x ",
               (uchar)ep->eth_hdr[0].src_addr[0], (uchar)ep->eth_hdr[0].src_addr[1],
               (uchar)ep->eth_hdr[0].src_addr[2], (uchar)ep->eth_hdr[0].src_addr[3],
               (uchar)ep->eth_hdr[0].src_addr[4], (uchar)ep->eth_hdr[0].src_addr[5]);

    return (PASSED);
}

/**********************************************************************
 *
 * Function: oak_disp_fw_ver
 *  
 *  
 * Description: Display Firmware version 
 *
 * Input : None.
 *
 * Output: PASSED/FAILED   
 *  
 **********************************************************************
 */
static int oak_disp_fw_ver (void)
{
    ngsm_entity_t *ep;
    fe_packet_t   *recv_packet_p;
    dspif_ether_t *result_packet_p;
    int ret_val;
    uint8_t bufmsg[128];

    assert(ngio_ptr);

    ep = (ngsm_entity_t *)ngio_ptr->priv;
    assert(ep);

    /* Bringup DSP */
    if (oakenshield_bringup_dsp() == FAILED) {
        cterr('f', 0, "%s(): \n DSP READY PIN not set. Please check if "
               "\"FIRMWARE_PATH\" exists.\n", __FUNCTION__);
        return (FAILED);
    }

    if (oak_setup_ge_env() == FAILED) {
        return (FAILED);
    }

    ret_val = oak_dsp_test(SELECT_FW_VER_DISP, 0, 0, 100);
    if (ret_val == PASSED) {
        recv_packet_p = &(ep->recv_packet);
        result_packet_p = &(ep->result_packet);

        memcpy((char *)result_packet_p, (char *)(&(recv_packet_p->data[0])+0),
               sizeof(dspif_ether_t));

        memcpy(bufmsg, result_packet_p->dspif_info.bufmsg, sizeof(bufmsg));
        printf("\n %s\n", bufmsg);
    }
    return (ret_val);
}

/**********************************************************************
 *
 * Function: oak_disp_mem
 *  
 * Description: Display SP2704 memory
 *
 * Input : None.
 *
 * Output: PASSED/FAILED   
 *  
 **********************************************************************
 */
static int oak_disp_mem (void)
{
    ngsm_entity_t *ep;
    dspif_mem_t   *mem_p;
    int ret_val, offset, len;
 
    /* Bringup DSP */
    if (oakenshield_bringup_dsp() == FAILED) {
        cterr('f', 0, "%s(): \n DSP READY PIN not set. Please check if "
               "\"FIRMWARE_PATH\" exists.\n", __FUNCTION__);
        return (FAILED);
    }

    offset = gethex_answer("Memory Start Address: ", 0, 0, 0xffffFD7C);
    len = gethex_answer("Length : ", 0, 0, 644); /* 0x284 */

    if (oak_setup_ge_env() == FAILED) {
        return (FAILED);
    }

    ret_val = oak_dsp_test(SELECT_MEM_DISP, offset, len, 100);
    if (ret_val == PASSED) {
        assert(ngio_ptr);

        ep = (ngsm_entity_t *)ngio_ptr->priv;
        assert(ep);

        mem_p = (dspif_mem_t *)&(ep->recv_packet.data);
        printf("\n DSP address = 0x%x, size = %d\n", mem_p->dspif_info.param1, 
               mem_p->dspif_info.param2);
        dismem((uchar *)(mem_p->pkt_data), mem_p->dspif_info.param2, 
               (ulong)(mem_p->pkt_data), 4);
    }
    return (ret_val);
}

/**********************************************************************
 *
 * Function: gpio_exp_read
 *  
 * Description: PCA9557 (GPIO expander) Register Read utility.
 *
 * Input : None.
 *
 * Output: PASSED/FAILED   
 *  
 **********************************************************************
 */
static int gpio_exp_read (void)
{
    n2g_i2c_if_t *pca;
    uchar data = 0;
    int offset;
    
    assert(ngio_ptr);

    pca = (n2g_i2c_if_t *)ngio_ptr->pca;
    assert(pca);

    pca->i2c_dev = SM_I2C_ADDR_IO_PORT1;

    for (offset = 0; offset <= 0x3; offset++) {
        if (io_port_8bit_i2c_read(pca, offset, &data, TRUE) == FAILED) {
            
            cterr('f', 0, "%s(): Unable to read PCA9557 register @ %#x\n", 
                  offset, __FUNCTION__);
            return (FAILED);
        }
        printf("\nRegister @ %#x = %#x\n", offset, data);
    }
    
    return (PASSED);
}

/**********************************************************************
 *
 * Function: gpio_exp_write
 *
 * Description: PCA9557 (GPIO expander) Register Write utility.
 *
 * Input : None.
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
static int gpio_exp_write (void)
{   
    n2g_i2c_if_t *pca;
    uchar data = 0;
    int offset;

    assert(ngio_ptr);

    pca = (n2g_i2c_if_t *)ngio_ptr->pca;
    assert(pca);

    pca->i2c_dev = SM_I2C_ADDR_IO_PORT1;


    offset = gethex_answer("Reg offset to write: ", 1, 1, 0x3);
    data = gethex_answer("Data to write", data, 0, 0xff);

    if (io_port_8bit_i2c_write(pca, offset, &data) == FAILED) {
        cterr('f', 0, "%s(): Unable to write PCA9557 register @ %#x\n", offset,
              __FUNCTION__);
        return (FAILED);
    }
    
    return (PASSED);
}

/*
 **********************************************************************
 *  
 *  Function: enable_bp_ge_lpbk
 *  
 *  Description: Enable GE switch loopback.
 *
 *  Input: None
 *
 *  Returns: PASSED 
 *
 **********************************************************************
 */
static int enable_bp_ge_lpbk (int port)
{
    ngsm_entity_t *ngsm_ep;
    int ge_port;

    assert(ngio_ptr);

    ngsm_ep = (ngsm_entity_t *)ngio_ptr->priv;
    assert(ngsm_ep);

    ge_port = ovld_get_ge_sw_port_num(ngsm_ep->pslot, 
                                      ngsm_ep->ge_tgt_dev, port);
    printf("\n NGSM eth port = %d, parent slot = %d, tgt_dev = %d \n", ge_port,
           ngsm_ep->pslot, ngsm_ep->ge_tgt_dev);
 
    set_gesw_line_loopback(ge_port, 1);

    return (PASSED);
}

/*
 **********************************************************************
 *  
 *  Function: disable_bp_ge_lpbk
 *  
 *  Description: This function disables the GE switch loopback mode
 *
 *  Input: None
 *
 *  Returns: PASSED 
 *
 **********************************************************************
 */
static int disable_bp_ge_lpbk (int port)
{
    ngsm_entity_t *ngsm_ep;
    int ge_port;

    assert(ngio_ptr);

    ngsm_ep = (ngsm_entity_t *)ngio_ptr->priv;
    assert(ngsm_ep);

    ge_port = ovld_get_ge_sw_port_num(ngsm_ep->pslot, 
                                      ngsm_ep->ge_tgt_dev, port);
    set_gesw_line_loopback(ge_port, 0);

    return (PASSED);
}

/**********************************************************************
 *
 * Function: lsi_sp27xx_sm_reset
 *
 * This function reset and unrerset the LSI SP27XX SM
 *
 * Input : None
 *
 * Output: PASSED/FAILED
 *  
 **********************************************************************
 */
int lsi_sp27xx_sm_reset (void)
{
    ngsm_entity_t *oakenshield_ep;
    int dsp;

    printf(" LSI SP27XX SM reset ");

    oak_ngio_off();
    msleep(1000);   /* Hardware need  time to off NGSM */ 
    oakenshield_ep = (ngsm_entity_t *)ngio_ptr->priv;

    for (dsp = 0; dsp < MAX_DSPS_PER_NGSM; dsp++) {
        oakenshield_ep->dsp_downloaded[dsp] = FALSE;
    }

    oakenshield_pwr_on();
    msleep(50);
    
    return (PASSED);
}

/*
 **********************************************************************
 *  
 *  Function: lsi_sp27xx_init_ngsm
 *  
 *  Description: This function initialize internal stuctures/data
 *
 *  Input: None
 *
 *  Returns: PASSED 
 *
 **********************************************************************
 */
int lsi_sp27xx_init_ngsm (void)
{
    ngsm_entity_t *ep;
    int ix = 0, jx;
    uint8_t print_mac[6] = {0};

    assert(ngio_ptr);
    
    /* Need to store the MAC address of the NGSM and host */ 
    ep = (ngsm_entity_t *)ngio_ptr->priv;
    assert(ep);

    ep->num_dsp = NUM_DSP_OAKENSHIELD;

    get_ngio_mac_addr(ep->pslot, ngio_ptr->mod_type,  &print_mac[0]);

    for (jx = 0; jx < 6; jx++) {
        ep->eth_hdr[ix].dest_addr[jx] = (uint8_t)print_mac[jx];
    }

    printf("\nOakenshield module MAC : 0x%02x:0x%02x:0x%02x:0x%02x:0x%02x:0x%02x ",
           (uchar)ep->eth_hdr[ix].dest_addr[0], (uchar)ep->eth_hdr[ix].dest_addr[1],  
           (uchar)ep->eth_hdr[ix].dest_addr[2], (uchar)ep->eth_hdr[ix].dest_addr[3],  
           (uchar)ep->eth_hdr[ix].dest_addr[4], (uchar)ep->eth_hdr[ix].dest_addr[5]);

    get_host_mac_addr(0, (uchar *)&src_mac[0]);

    for (jx = 0; jx < 6; jx++) {
        ep->eth_hdr[ix].src_addr[jx] = (uint8_t)src_mac[jx]; 
    }
    printf("\nHost  MAC           : 0x%02x:0x%02x:0x%02x:0x%02x:0x%02x:0x%02x ",
           (uchar)ep->eth_hdr[ix].src_addr[0], (uchar)ep->eth_hdr[ix].src_addr[1],  
           (uchar)ep->eth_hdr[ix].src_addr[2], (uchar)ep->eth_hdr[ix].src_addr[3],  
           (uchar)ep->eth_hdr[ix].src_addr[4], (uchar)ep->eth_hdr[ix].src_addr[5]);


    return (PASSED);
}

/*
 **********************************************************************
 *
 *  Function: lsi_sp27xx_get_sm_pid
 *
 *  Description: This function returns the PID string
 *
 *  Input: char *
 *
 *  Returns: PASSED/FAILED 
 *
 **********************************************************************
 */
static int lsi_sp27xx_get_sm_pid (char *pid)
{
    uchar ix, num_byte, *data_ptr;

    assert(ngio_ptr);

    if ((data_ptr = (uchar *) search_type_ret_addr_of_first_data
        (ngio_ptr->cookie, (uchar) PRODUCT_ID,
        &num_byte, FALSE)) == (uchar *) NULL) {
            /*Search CONTROLLER_TYPE failed. */
            pid[0] = 0;                /* illegal code */
            return (FAILED);
        } else {
            for (ix = 0; ix < num_byte; ix++) {
                pid[ix] = *data_ptr++;
            }

        }
    return (PASSED);
}

/*
 **********************************************************************
 *
 *  Function: init_oak_ngio_priv_struct
 *
 *  Description: This function initializes the ngsm specific structures.
 *
 *  Input: None
 *
 *  Returns:  PASSED - otherwise
 *
 **********************************************************************
 */
static int init_oak_ngio_priv_struct (void)
{
    ngsm_entity_t *oakenshield_ep;
    int dsp, ngsm_num, tgt_dev, i2c_dev, pslot;
    char slot_type_str[20], tty_dev[20];

    assert(ngio_ptr);
    
    ngsm_num = ngio_ptr->slot;

    sprintf(slot_type_str, "Oakenshield");

    tgt_dev = TGT_DEV_NGSM;

    sprintf(tty_dev, "/dev/ttyDASH%d", ngio_ptr->uart_ctrl);
    i2c_dev = SM_I2C_ADDR_IO_PORT1;
    pslot = ngio_ptr->slot;

    testname(slot_type_str);
    /* point to Oakenshield structure */
    ngio_ptr->priv = (void *) &oak_lsi_entity[0];

    /* Init parameters for I2C access to the IO expander */
    pca_init_i2c((void *)&pca_i2c[0]);
    pca_i2c[0].i2c_ctrl = ngio_ptr->i2c_ctrl;
    pca_i2c[0].i2c_dev = i2c_dev;
    pca_i2c[0].buf = &pca_buff[0];
    ngio_ptr->pca = (void *) &pca_i2c[0];

    oir_if = (void *)(ngio_ptr->oir);

    /* Init Oakenshield struct parameters */
    oakenshield_ep = (ngsm_entity_t *) ngio_ptr->priv; 
    /* Will populate id after firmware download */
    oakenshield_ep->cookie_id = ngio_ptr->id; 
    oakenshield_ep->ngsm_id = PFUSE123_UNKNOWN; 
    oakenshield_ep->plat_ngsm_num = ngsm_num; 
    oakenshield_ep->ge_tgt_dev = tgt_dev; 
    oakenshield_ep->pslot = pslot; 
    sprintf(oakenshield_ep->tty_dev, "%s", tty_dev);
    sprintf(oakenshield_ep->slot_type_str, "%s", slot_type_str);

    sprintf(oakenshield_ep->name, "OAKENSHIELD SM LSI SP27XX, board_id %#x, "
              "%s slot %d", ngio_ptr->id, slot_type_str, 
              ngio_ptr->slot);

    for (dsp = 0; dsp < MAX_DSPS_PER_NGSM; dsp++) {
        oakenshield_ep->dsp_downloaded[dsp] = FALSE;
    }
    return (PASSED);
}

/*
 **********************************************************************
 *
 *  Function: oakenshield_init_fp
 *
 *  Description: This function initialize callout functions
 *
 *  Input: None
 *
 *  Returns: None 
 *
 **********************************************************************
 */
static void oakenshield_init_fp (void)
{
    ngsm_entity_t *ep;

    assert(ngio_ptr);

    ep = (ngsm_entity_t *)ngio_ptr->priv;
    assert(ep);

    ep->reset_ngsm = lsi_sp27xx_sm_reset;
    ep->init_ngsm = lsi_sp27xx_init_ngsm;
    ep->get_pid = lsi_sp27xx_get_sm_pid;

}

/**********************************************************************
 * Function: oakenshield_sm_cleanup
 *
 * Description: This function performs the cleanup task before exiting
 *              the test.
 *
 * Input:  None
 *
 * Output: None
 **********************************************************************
 */
static void oakenshield_sm_cleanup (void)
{
    assert(ngio_ptr);

    if (oakenshield_saved_diag_exec) {
        pre_diag_exec = oakenshield_saved_diag_exec;
        oakenshield_saved_diag_exec = NULL;
    }
}


/**********************************************************************
 * Function: uart_lpbk
 *
 * Description: This function performs the uart interface test for the 
                NGSM
 *
 * Input:  None
 *
 * Output: PASSED/FAILED
 **********************************************************************
 */
static int oak_uart_test (void)
{
    ngsm_entity_t *ngsm_ep;
    int ret = PASSED;
    int port, dsp;
    char *tx_str = "abcdefgh";
    int tx_len = strlen(tx_str);
    int rx_sz;
    char rx_str[64];

#ifdef OAKENSHIELD_USE_UART_MENU
    printf("\n uart is implicity tested with the menu running on NGSM "
           "module");
    return (PASSED);
#endif

    prpass(testpass, "Uart Loopback - ");
    assert(ngio_ptr);

    ngsm_ep = (ngsm_entity_t *)ngio_ptr->priv;
    assert(ngsm_ep);

    if (oakenshield_bringup_dsp() == FAILED) {
        cterr('f', 0, "%s(): \n DSP READY PIN not set. Please check if "
               "\"/firmware/dsp_sp2700_fw.img\" exists.\n", __FUNCTION__);
        return (FAILED);
    }
    /* Run uart loopback test when the DSP diag application is booted */
    /* Wait dsp ready  message. Set 100 sec for time out.*/
    oak_select_test(SELECT_UART_TEST, 0, 0, 100);  
    fflush(stdout);
    fflush(stderr);
    assert(ngio_ptr);
    assert(ngio_ptr->priv);
    ngsm_ep = (ngsm_entity_t *) ngio_ptr->priv;

    port = ngio_ptr->uart_ctrl;
    printf("Uart port: %d\n", port);
    memset(rx_str, 0, sizeof(rx_str));
    rx_sz = 0;
    if ((ret = uart_lpbk_txrx(port, tx_str, tx_len, rx_str, &rx_sz,
                              9600, 0)) == FAILED) {
        //Do not reset if test fails oakenshield_reset_en();
        cterr('f', 0, "%s(): uart_lpbk: failed to tx and rx.\
                Please reset/unreset NGSM. ", __FUNCTION__);
    } else {
        if (!strstr(rx_str, tx_str)) {
            ret = FAILED;
            cterr('f', 0, "rx/tx string differ [rx = %s] [expect str = %s] [tx = %s].",
                  rx_str, tx_str, tx_str);
        } else {
            printf("uart loopback test pass!");
        }

    }

    fflush(stdout);
    fflush(stderr);
    printf("\nPower Cycle DSP to Normal Mode!!!\n\n");

    for (dsp = 0; dsp < MAX_DSPS_PER_NGSM; dsp++) {
        ngsm_ep->dsp_downloaded[dsp] = FALSE;
    }

    /* Bringup DSP */
    if (oakenshield_bringup_dsp() == FAILED) {
        cterr('f', 0, "%s(): \n DSP READY PIN not set. Please check if "
               "\"/firmware/dsp_sp2700_fw.img\" exists.\n", __FUNCTION__);
        return (FAILED);
    }

    return (ret);

}

/**********************************************************************
 * Function: oak_gpio_resistor_test
 *
 * Description: This function test the resistor circuitry for the GPIO 
 *
 * Input:  None
 *
 * Output: PASSED/FAILED
 **********************************************************************
 */
int oak_gpio_resistor_test (void)
{
    n2g_i2c_if_t  *pca;        
    int i2c_dev, ret;
    uchar data, config, inv, output;

    assert(ngio_ptr);
    assert(ngio_ptr->priv);

    ret = PASSED;
    prpass(testpass, "GPIO Resistor - ");

    /* This test should be conducted with the NGSM in reset to test
       the resistor stuffing used for pull up/down */
    oakenshield_pwr_off();

    /* Get the correct I2C address to use */
    pca = (n2g_i2c_if_t *)ngio_ptr->pca;
    i2c_dev = pca->i2c_dev; 

    /* Please set the GPIO register to default first */
    if (io_port_8bit_i2c_read(pca, CONFIGURATION_REG, &config, TRUE) == FAILED) {
        pca->i2c_dev = i2c_dev;
        cterr('f', 0, "%s(): Unable to read PCA9557 CONFIGURATION_REG, \n",
              __FUNCTION__);
        ret = (FAILED);
    }
    if (io_port_8bit_i2c_read(pca, POLARITY_INV_REG, &inv, TRUE) == FAILED) {
        pca->i2c_dev = i2c_dev;
        cterr('f', 0, "%s(): Unable to read PCA9557 POLARITY_INV_REG, \n",
              __FUNCTION__);
        ret = (FAILED);
    }
    if (io_port_8bit_i2c_read(pca, OUTPUT_PORT_REG, &output, TRUE) == FAILED) {
        pca->i2c_dev = i2c_dev;
        cterr('f', 0, "%s(): Unable to read PCA9557 OUTPUT_PORT_REG, \n",
              __FUNCTION__);
        ret = (FAILED);
    }
    data = 0xFF;  /* write 0xFF base on spec */
    if (io_port_8bit_i2c_write(pca, CONFIGURATION_REG, &data) == FAILED) {
        pca->i2c_dev = i2c_dev;
        cterr('f', 0, "%s(): Unable to write PCA9557 config register \n", 
              __FUNCTION__);
        ret = (FAILED);
    }
    data = 0xF0; /*  write 0xF0 base on spec */
    if (io_port_8bit_i2c_write(pca, POLARITY_INV_REG, &data) == FAILED) {
        pca->i2c_dev = i2c_dev;
        cterr('f', 0, "%s(): Unable to write PCA9557 Polarity Inversion "
              "register \n", __FUNCTION__);
        ret = (FAILED);
    }
    if (io_port_8bit_i2c_read(pca, INPUT_PORT_REG, &data, TRUE) == FAILED) {
        pca->i2c_dev = i2c_dev;
        cterr('f', 0, "%s(): Unable to read PCA9557 INPUT_PORT_REG \n",
              __FUNCTION__);
        ret = (FAILED);
    }
    if (data != 0xF3) {
        pca->i2c_dev = i2c_dev;
        cterr('f', 0, "%s(): GPIO Resistor test failed, expected 0xF3 read"
              " 0x%x \n", __FUNCTION__, data);
        ret = (FAILED);
    }
    /* Restore save values */
    if (io_port_8bit_i2c_write(pca, CONFIGURATION_REG, &config) == FAILED) {
        pca->i2c_dev = i2c_dev;
        cterr('f', 0, "%s(): Unable to write PCA9557 config register \n", 
              __FUNCTION__);
        ret = (FAILED);
    }
    if (io_port_8bit_i2c_write(pca, POLARITY_INV_REG, &inv) == FAILED) {
        pca->i2c_dev = i2c_dev;
        cterr('f', 0, "%s(): Unable to write PCA9557 Polarity Inversion "
              "register \n", __FUNCTION__);
        ret = (FAILED);
    }
    if (io_port_8bit_i2c_write(pca, OUTPUT_PORT_REG, &output) == FAILED) {
        pca->i2c_dev = i2c_dev;
        cterr('f', 0, "%s(): Unable to write PCA9557 OUTPUT_PORT_REG,"
              "register \n", __FUNCTION__);
        ret = (FAILED);
    }
    pca->i2c_dev = i2c_dev;
    return (ret);
     
}

/**********************************************************************
 * Function: oak_gpio_exp_test
 *
 * Description: Test the output pins 1 and 7 of the GPIO Expander 
 *              Register.
 *
 * Input:  None
 *
 * Output: PASSED/FAILED
 **********************************************************************
 */
static int oak_gpio_exp_test (void)
{
    n2g_i2c_if_t  *pca;        
    uchar orig_val = 0, test_data = 0, check_data = 0;
    uint32_t ctr = 0, test_ctr = 0, total_reg_num = 0;
    reg_info_t  *reg_p = 0;

    reg_p = &pca9557_reg_tbl[0];
    total_reg_num = (sizeof(pca9557_reg_tbl) / sizeof(reg_info_t));

    prpass(testpass, "GPIO Expander Register test");

    assert(ngio_ptr);
    assert(ngio_ptr->priv);

    pca = (n2g_i2c_if_t *)ngio_ptr->pca;
    pca->i2c_dev = SM_I2C_ADDR_IO_PORT1;

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
            if (io_port_8bit_i2c_read(pca, ctr, &orig_val, TRUE)) {
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
                if (io_port_8bit_i2c_write(pca, ctr, &test_data)) {
                    cterr('f', 0, "%s: Failed to wrote 0x%02X "
                                  "to IO Expander Reg. %#x in Ripple 1 test.",
                                  __FUNCTION__, test_data, reg_p->offset);
                    return (FAILED);
                }

                /* Read the register value back for double check */
                if (io_port_8bit_i2c_read(pca, ctr, &check_data, TRUE)) {
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
                if (io_port_8bit_i2c_write(pca, ctr, &test_data)) {
                    cterr('f', 0, "%s: Failed to wrote 0x%02X "
                                  "to IO Expander Reg. %#x in Ripple 0 test.",
                                  __FUNCTION__, test_data, reg_p->offset);
                    return (FAILED);
                }

                /* Read the register value back for double check */
                if (io_port_8bit_i2c_read(pca, ctr, &check_data, TRUE)) {
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
            if (io_port_8bit_i2c_write(pca, ctr, &orig_val)) {
                cterr('f', 0, "%s: Failed to write the restore value 0x%02X "
                              "back to IO Expander Reg. %#x.",
                              __FUNCTION__, test_data, reg_p->offset);
                return (FAILED);
            }
        }
    }


    return (PASSED);
}


/*
 **********************************************************************
 *
 *  Function: oak_ngsm_main_test
 *
 *  Description: This is the test entry point for Oakenshield NGSM.
 *
 *  Input: none
 *  
 *  Returns: PASSED if successful; 
 *           FAILED, otherwise
 *  
 **********************************************************************
 */
static int oak_ngsm_main_test (void)
{
    int  retval = PASSED;

    /* Will test the Host-Side FPGA Reg, Interrupts, Uart, GDF Int lpbk,
       pcie link */

    if (retval == PASSED) {
        retval = oak_gpio_exp_test();
    }

    if (retval == PASSED) {
        retval = oakenshield_test(TRUE);
    }

    return (retval);
}


/*
 **********************************************************************
 *
 *  Function: oakenshield_bringup_dsp
 *
 *  Description: Unreset DSP, dnld, firmware and check if dsp is ready
 *
 *  Input: None
 *  
 *  Returns: PASSED if successful; 
 *           FAILED  - (I2C GPIO access error or DSP not set READY PIN)
 *                     or DSP up but not in READY mode or DSP up but
 *                     firmware not correct.
 *
 **********************************************************************
 */
static int oakenshield_bringup_dsp (void)
{
    n2g_i2c_if_t  *pca;
    ngsm_entity_t *ngsm_ep;
    int i2c_dev, i;
    uchar  data = 0;

    assert(ngio_ptr);

    /* For ethernet mode loopback is never enabled */
    if (oak_dsp_tests_use_enet == 0) {  /* for uart mode */
        printf("\n Make sure GE Loopback mode is disabled.");
        disable_bp_ge_lpbk(0);
        disable_bp_ge_lpbk(1);
    }

    /* check if DSP is already booted  only one dsp on Oakenshield */
    ngsm_ep = (ngsm_entity_t *)ngio_ptr->priv;
    assert(ngsm_ep);

    if (ngsm_ep->dsp_downloaded[0] == TRUE) {
        return (PASSED);
    }
    /* reset PVDM */
    ngsm_ep->reset_ngsm();

    printf("\r Please wait for the Oakenshield Bootloader to dhcp the "
           "firmware");
    fflush(stdout);

    pca = (n2g_i2c_if_t *)ngio_ptr->pca;
    i2c_dev = pca->i2c_dev; 
    pca->i2c_dev = SM_I2C_ADDR_IO_PORT1;
    for (i = 0; i < 60; i++) {
        if (io_port_8bit_i2c_read(pca, INPUT_PORT_REG, &data, TRUE) == FAILED) {
            cterr('f', 0, "Unable to read PCA9557 register @ %#x\n", 
                  INPUT_PORT_REG);
            return (FAILED);
        }
        if (data & PRIM_INTF_READY) {
            printf("\r GPIO Expander bit 3(READY Bit) Set by Oakenshield");
            break;
        } else if (i == 59) {
            printf("\n*** %s(): GPIO Expander bit 3 not set by DSP\n"
                   "Oakenshield not booted up, Tests will fail.", __FUNCTION__);
            return (FAILED);
        } 
        switch (i%8) {
        case 0:
            printf("\r|");
            break;
        case 1:
            printf("\r/");
            break;
        case 2:
            printf("\r-");
            break;
        case 3:
            printf("\r\\");
            break;
        case 4:
            printf("\r|");
            break;
        case 5:
            printf("\r/");
            break;
        case 6:
            printf("\r-");
            break;
        case 7:
            printf("\r\\");
            break;
        default:
            break;
        }
        fflush(stdout);
        msleep(1000);
    }
    pca->i2c_dev = i2c_dev;
    msleep(1000); /* Wait 1000ms for DSP ready. */
    printf("Send Ready Test to DSP\n");
    if ((oakenshield_ready_test(0)) == PASSED)  {
        ngsm_ep->dsp_downloaded[0] = TRUE;
    } else {
        printf("\n %s(): oakenshield_ready_test() failed\n", __FUNCTION__);
        return (FAILED);
    }

    if (oak_dsp_tests_use_enet == 0) {
        printf("\n Enabling GE loopback mode ");
        enable_bp_ge_lpbk(0);
    }
    return (PASSED);

}

/*
 **********************************************************************
 *
 *  Function: oak_sm_test
 *
 *  Description: This is the test entry point for Oakenshield NGSM.
 *
 *  Input: ngio interface struct *
 *  
 *  Returns: PASSED if successful; 
 *           FAILED, otherwise
 *
 **********************************************************************
 */ 
int oak_ngsm_test (void *ngsm)
{
    ngsm_entity_t *ngsm_ep;
    int ret_val, ngsm_num;
    ngsm_p = ngsm;

    assert(ngsm);

    ngio_ptr = (struct ngio_intf_t *)ngsm;
    oak_board_id = ngio_ptr->id;

    init_oak_ngio_priv_struct();

    /* initialize Oakenshield internal entities for operations */
    oakenshield_init_fp();

    ngio_ptr->uart_on(ngio_ptr);

    assert(ngio_ptr->priv);
    ngsm_ep = (ngsm_entity_t *)ngio_ptr->priv;

    /* Display the NGSM module details */
    printf("\r %s\n", ngsm_ep->name);

    ngsm_num = ngsm_ep->ngsm_num;

    /* The actual number if all the slots(onboard, NGIOs)  
       were populated with NGSM */
    printf("\n NGSM_NUM = %d\n", ngsm_num);


    if (tftp_get(0, (unsigned char *)"oakenshield_dsp_diag.img",
                 0, (unsigned char *)"/firmware/sm_dsp_sp2700_fw.img", 1) < 0) {
        cterr('f', 0, "Failed to tftp download firmware to local host");
        return(FAILED);

    }

    /* Find and display the pid for the NGSM */
    if ((ngsm_ep->get_pid(ngsm_ep->pid)) == FAILED) {
        cterr('f', 0, "%s(): Invalid NGSM PID %s", __FUNCTION__, ngsm_ep->pid);
        return (FAILED);
    }

    sprintf(maindiag_header.title, "%s", ngsm_ep->pid);
    prpass(testpass, " PID: %s ", ngsm_ep->pid);

    if (oakenshield_pwr_on() == FAILED) {
        cterr('f', '0', " Power on failed\n");
        return (FAILED);
    }


    /* Init the NGSM module specific parameters mac etc */
    if (ngsm_ep->init_ngsm()) {
        return (FAILED);
    }

    /* Enable the ngio_sync_out_enable   */
    ngio_sync_out_enable (ngsm, ENABLE_PRE_SCALER_DIV_3125 |   SYNC_OUT_ENABLE );

    oakenshield_saved_diag_exec = pre_diag_exec;
    pre_diag_exec = NULL;
    sprintf(oakmainmenutitle, "%s Main Menu", ngsm_ep->slot_type_str);
    sprintf(oaksubmenutitle, "%s", ngsm_ep->slot_type_str);
    build_primary_submenu(oakenshield_mainmenu_tbl, OAKENSHIELD_MAINMENU_TBL_SIZE, 
                          "Oakenshield Menu", &maindiagp);
    build_secondary_submenu(oakenshield_mainmenu_tbl, OAKENSHIELD_MAINMENU_TBL_SIZE,
                            main_menu_secondary_items);

    if (ngio_ptr->menu_display) {
        menu(&oakenshield_mainmenu, main_menu_secondary_items, '\0');
        pre_diag_exec = oakenshield_saved_diag_exec;
        ret_val = PASSED;
    } else {
        ret_val = oak_ngsm_main_test();
    }

    oakenshield_sm_cleanup();
    return (ret_val);

}

/**********************************************************************
 *
 * Function: oakenshield_ready_test 
 * This function test to make sure the Oakenshield is ready to accept commands
 *  
 * Input : dsp #
 *  
 * Output: PASSED/FAILED
 *         FAILED - tx READY command failed, did not recv resp, resp 
 *                  recevied does not match READY opcode, dsp firmware 
 *                  version returned is incorrect, ge setup/cleanup 
 *                  failed
 *
 **********************************************************************
 */ 
static int oakenshield_ready_test (int dsp_no)
{
    ngsm_entity_t *ep; 
    fe_packet_t   *recv_packet_p;
    dspif_ready_t *ready_msg_p;
    dspif_ready_t ready_msg;
    int retval = PASSED;
    
    printf("Send Ready Packet to Oakenshield\n");

    assert(ngio_ptr);

    ep = (ngsm_entity_t *)ngio_ptr->priv;
    assert(ep);

    recv_packet_p = &(ep->recv_packet);
    ready_msg_p = &ready_msg;

    printf("Set up GE env\n");
    if (oak_setup_ge_env() == FAILED) {
        return (FAILED);
    }
    
    oak_clear_rx_buf((uchar *)ready_msg_p, (int)sizeof(dspif_ready_t));
    oak_clear_rx_buf((uchar *)recv_packet_p, (int)sizeof(fe_packet_t));
    
    printf("Send Ready Command to DSP\n");
    oak_build_command_packet(SELECT_READY, 0, 0, DSS_CORE0);
    oak_build_config_packet();
    /* need to know which DSP on the PVDM */
    if ((retval = oak_send_command_packet(dsp_no)) != PASSED) {
        cterr('f', 0, "\n %s(): oak_send_command_packet() returned tx error "
              "%d\n", __FUNCTION__, retval);
        return (FAILED);
    }
    /* wait for result packet */
    printf("Wait for DSP feedback\n");
    if (oak_wait_result_packet((uint8_t *)recv_packet_p, ep->socket_gl, 360,
                                 dsp_no)) {
        cterr('f', 0, "\n%s(): Timed out waiting for READY message from DSP.", 
               __FUNCTION__);
        return (FAILED);
    }
    memcpy((char *)ready_msg_p, (char *)(&(recv_packet_p->data[0])+0),
               sizeof(dspif_ready_t));
    /* parse result */
    if (ready_msg_p->dspif_hdr.op_type != OP_READY) {
        cterr('f', 0, "%s(): Ready test failed op_type: 0x%x: expected 0x%x",
              ready_msg_p->dspif_hdr.op_type, OP_READY, __FUNCTION__);
        return (FAILED);
    }
    /* check if the firmware version is correct */
    if ((ready_msg_p->fw_ver.major_num != DIAGFW_MAJ_REL) || 
        (ready_msg_p->fw_ver.minor_num != DIAGFW_MIN_REL) ||
        (ready_msg_p->fw_ver.debug_num != DIAGFW_DEBUG_VER)) {
        printf("\n DSP FW version does not match 0x%x:0x%x:0x%x, expected "
               "0x%x:0x%x:0x%x", ready_msg_p->fw_ver.major_num,
               ready_msg_p->fw_ver.minor_num, ready_msg_p->fw_ver.debug_num,
               DIAGFW_MAJ_REL, DIAGFW_MIN_REL, DIAGFW_DEBUG_VER);
        cterr('f', 0, "%s(): Ready test failed DSP diag FW version does not "
              "match", __FUNCTION__);
        return (FAILED);
    }
    /* Only one dsp */
    ep->major_rel[0] = ready_msg_p->fw_ver.major_num;
    ep->minor_rel[0] = ready_msg_p->fw_ver.minor_num;
    ep->debug_ver[0] = ready_msg_p->fw_ver.debug_num;

    if (oak_cleanup_ge_env() == FAILED) {
        cterr('f', 0, "%s(): oak_cleanup_ge_env() failed", __FUNCTION__);
        return (FAILED);
    }

    ep->ngsm_id = ready_msg_p->dsp_device.core_id;

    if (ready_msg_p->dsp_device.core_id == PFUSE123_SP2702) {
        printf("\r Received Ready Response from Oakenshield SP2702     ");
    } else if (ready_msg_p->dsp_device.core_id == PFUSE123_SP2704) {
        printf("\r Received Ready Response from Oakenshield SP2704     ");
    } else {
        printf("\r Received Ready Response from Oakenshield (unknown SP27XX) ");
    }
    return (retval);

}

/***********************************************************************
 * Name: oak_setup_ge_env (common)
 *
 * Description:
 *      This test will set up GE operation environment. 
 *
 * Input: None
 *
 * Output: PASSED or FAILED.
 *
 ***********************************************************************
 */
static int oak_setup_ge_env (void)
{
    ngsm_entity_t *ep;
    int status = PASSED;
    int sgmii_port = 0;
    char if_name[IFNAMSIZ];
    
    assert(ngio_ptr);
    ep = (ngsm_entity_t *) ngio_ptr->priv;
    assert(ep);

    if (ep->ge_setup_flag == TRUE) {
        /* Linux socket already setup return */
        return (PASSED);
    }

    sgmii_port = get_sgmii_port_num(0, TYPE_SWITCH);
    sprintf(if_name, "eth%d", sgmii_port);
    status = setup_eth_dev(if_name, &(ep->socket_gl));

#ifdef GE_COMM
    printf("\n socket = %d", ep->socket_gl);
#endif

    if (status) {
        cterr('f', 0, "Setup: Failed, status = 0x%x", status);
        return (FAILED);
    }

    ep->ge_setup_flag = TRUE;

    return (PASSED);
}

/***********************************************************************
 * Name: oak_cleanup_ge_env (common)
 *
 * Description:
 *      This test will clean up the GE operation environment.
 *
 * Input: None
 *
 * Output: PASSED or FAILED.
 *
 ***********************************************************************
 */
int oak_cleanup_ge_env (void)
{
    ngsm_entity_t *ep;
    int status = PASSED;
    int sgmii_port = 0;
    char if_name[IFNAMSIZ];

    assert(ngio_ptr);
    ep = (ngsm_entity_t *)ngio_ptr->priv;
    assert(ep);

    if (ep->ge_setup_flag == TRUE) {
        ep->ge_setup_flag = FALSE;

	    sgmii_port = get_sgmii_port_num(0, TYPE_SWITCH);
        sprintf(if_name, "eth%d", sgmii_port);
        status = cleanup_eth_dev(if_name, ep->socket_gl);

        if (status) {
            cterr('f', 0, "cleanup: Failed, status = 0x%x", status);
            return (FAILED);
        }
    }
    return (PASSED);

}

/***********************************************************************
  * Name: oak_clear_rx_buf (common)
  *
  * Description:
  *      Clear receiver buffer before testing.
  *
  * Input: char * - array ptr
  *        int    - number of bytes to clear.
  *
  * Output: NONE.
  *
  ***********************************************************************
  */
void oak_clear_rx_buf (uchar *c_ptr, int num_bytes)
{
    int ix;

    for (ix = 0; ix < num_bytes; ix++) {
        *c_ptr++ = 0;
    }
}

/*
 **********************************************************************
 *
 *  Function: oak_build_config_packet
 *
 *  Description: PID info etc 
 *
 *  Input: None 
 *
 *  Returns: None
 *
 **********************************************************************
 */
void oak_build_config_packet (void)
{
    ngsm_entity_t *ep;
    dspif_ether_t *cmd_packet_p;
    uint ix;
    uchar *ngsm_pid;

    assert(ngio_ptr);

    ep = ngio_ptr->priv;
    assert(ep);

    cmd_packet_p = &(ep->cmd_packet);
    ngsm_pid = (uchar *)&(cmd_packet_p->dspif_info.bufmsg[0]);
    for (ix = 0; ix < 128; ix++) {
        ngsm_pid[ix] = ep->pid[ix];
    }
#ifdef DSP_DEBUG
    printf("\n In oak_build_config_packet() pid = %s\n", ngsm_pid);
#endif
    /* 0 - oak_dsp_tests_use_enet : 1= enet intf 0= uart intf
     * 1 - menu_display   : If uart intf, display menu [1] or run the tests [0]
     * 2 - module_type  : 6= SM_MODULE etc
     * 3 - parent slot  : applicable if DC
     */
    cmd_packet_p->dspif_info.errmsg[2] = ngio_ptr->mod_type;
    cmd_packet_p->dspif_info.errmsg[0] = oak_dsp_tests_use_enet; //eth or uart interface
    cmd_packet_p->dspif_info.errmsg[1] = ngio_ptr->menu_display; /*if uart then run tests or just display the menu */
    oak_dsp_tests_use_enet = cmd_packet_p->dspif_info.errmsg[0]; 
#ifdef DSP_DEBUG
    dismem((char *)cmd_packet_p, sizeof(dspif_ether_t), (ulong)cmd_packet_p, 1);
    printf("\n In oak_build_config_packet() dc_slot = %d\n", 
           cmd_packet_p->dspif_info.errmsg[0]);
#endif
}

/*
 **********************************************************************
 *
 *  Function: oak_build_command_packet
 *
 *  Description: build test command
 *
 *  Input: selected test - select command
 *         param1 - parameter 1 pass to module
 *         param2 - parameter 2 pass to module
 *         core_id - DSP core
 *  Returns: None
 *
 **********************************************************************
 */
void oak_build_command_packet (uint16_t select_test, uint32_t param1, 
                                 uint32_t param2, uint8_t core_id)
{
    ngsm_entity_t *ep;
    dspif_ether_t *cmd_packet_p;

    assert(ngio_ptr);

    ep = ngio_ptr->priv;
    assert(ep);

    cmd_packet_p = &(ep->cmd_packet);
    cmd_packet_p->dspif_hdr.src_id = SWAP32(HOST_ID);
    cmd_packet_p->dspif_hdr.dest_id = SWAP32(ep->plat_ngsm_num);

    cmd_packet_p->dspif_hdr.op_type = (OP_TEST_REQUEST);
    cmd_packet_p->dspif_hdr.data_len = (sizeof(dspif_info_t));
    cmd_packet_p->dspif_info.command = 0;
    cmd_packet_p->dspif_info.result = SWAP32(RESULT_RUNNING);
    cmd_packet_p->dspif_info.flags = SWAP32(FLAG_NULL);
    cmd_packet_p->dspif_info.select = select_test;
    cmd_packet_p->dspif_info.faults = 0;
    cmd_packet_p->dspif_info.location = 0;
    cmd_packet_p->dspif_info.expected = 0;
    cmd_packet_p->dspif_info.actual = 0;
    cmd_packet_p->dspif_info.extra = 0;
    cmd_packet_p->dspif_info.errorcount = 0;
    cmd_packet_p->dspif_info.testcounter = 0;
    cmd_packet_p->dspif_info.ReadyOnTest = 0;
    cmd_packet_p->dspif_info.TestCtrl = 0;
    cmd_packet_p->dspif_info.WhoAmI = 0;
    cmd_packet_p->dspif_info.ver_no = 0;
    cmd_packet_p->dspif_info.wait_states = 0;
    cmd_packet_p->dspif_info.param1 = param1;
    cmd_packet_p->dspif_info.param2 = param2;
    cmd_packet_p->dspif_info.param3 = 0;
    cmd_packet_p->dspif_info.param4 = 0;
#ifdef GE_DEBUG
    printf("\n cmd_packet_p->dspif_hdr.src_id = 0x%x", SWAP32(HOST_ID));
    printf("\n cmd_packet_p->dspif_hdr.dest_id = 0x%x", SWAP32(core_id));
    printf("\n cmd_packet_p->dspif_hdr.op_type = 0x%x", SWAP32(OP_TEST_REQUEST));
    printf("\n cmd_packet_p->dspif_hdr.data_len = 0x%x", SWAP32(sizeof(dspif_info_t)));
#endif
    memset((cmd_packet_p->dspif_info.bufmsg), 0, 128);
    memset((cmd_packet_p->dspif_info.errmsg), 0, 128);
}

/*
 **********************************************************************
 *
 *  Function: oak_send_command_packet (not common specific)
 *
 *  Description: build test command
 *
 *  Input: dsp_no - DSP number
 *
 *  Returns: PASSED
 *           FAILED - some tx error
 *
 **********************************************************************
 */
int oak_send_command_packet (int dsp_no)
{
    eth_tx_pkt_t tx_pkt;
    eth_tx_pkt_t *tx_pkt_p = &tx_pkt;
    ngsm_entity_t *ep;
    int ret_val = PASSED;

    assert(ngio_ptr);

    ep = (ngsm_entity_t *)ngio_ptr->priv;
    assert(ep);

    memset((uchar *)tx_pkt_p, 0, sizeof(eth_tx_pkt_t));
 
    /* Write to routine to read the MAC address fromt the cookie from the 
     * NGSM. Till then use the bcast address.
     */
    memcpy((uchar *)(tx_pkt_p->dest_addr), 
           (uchar *)&(ep->eth_hdr[dsp_no].dest_addr),
           sizeof(mac_addr_t));
    memcpy((uchar *)(tx_pkt_p->src_addr), 
           (uchar *)&(ep->eth_hdr[dsp_no].src_addr), 
           sizeof(mac_addr_t)); /* host MAC */
    tx_pkt_p->pkt_type = PKT_TYPE_IPV4;
    tx_pkt_p->payload_size = sizeof(dspif_ether_t);
    tx_pkt_p->bufr_st_addr = (uchar *)&(ep->cmd_packet);
    tx_pkt_p->pkt_num = 0;
    tx_pkt_p->socket = ep->socket_gl;

#if 0
    printf("\n Packet to Send \n");
    printf("\n pkt_type = 0x%x", tx_pkt_p->pkt_type);
    printf("\n payload_size = 0x%x", tx_pkt_p->payload_size);
    printf("\n sending the packet\n");
#endif
    ret_val = eth_pkt_tx(tx_pkt_p);

    return (ret_val);
}

/*
 **********************************************************************
 *
 *  Function: oak_sgmii_dsp_lpbk_test
 *
 *  Description: test ge loopback with different frames 
 *
 *  Input:  dsp number
 *          GE port number
 *
 *  Returns: PASSED if successful;
 *           FAILED, otherwise
 *
 **********************************************************************
 */
int oak_sgmii_dsp_lpbk_test (int dsp_no, int port_num)
{
    ngsm_entity_t *ep;
    int retval = PASSED;
    uint32 frame_num, frm_size;
    mac_addr_t mac_da;
    unsigned short pak_size[32] = {64, 108, 512, 256,
                1500, 65, 1511, 128,
                66, 719, 100, 1513,
                1000, 200, 78, 1514,
                300, 400, 312, 168,
                67, 955, 60, 512,
                333, 888, 83, 128,
                135, 531, 99, 1024};

    retval = PASSED;

    ep = (ngsm_entity_t *)ngio_ptr->priv;
    memcpy((uchar *)mac_da, (uchar *)&(ep->eth_hdr[dsp_no].dest_addr),
           sizeof(mac_addr_t));

    for (frame_num = 0; frame_num < 3; frame_num++) {
        frm_size = pak_size[frame_num];
        prpass(testpass,"GE%d Ext Loopback test, frame%d, size %d ",
               port_num, frame_num, frm_size);
        if (oak_eth_frames_test(frame_num, frm_size, ep->socket_gl,
                                mac_da, dsp_no)) {
            retval = FAILED;
            break;
        }
    }

    return (retval);
}

/*
 **********************************************************************
 *
 *  Function: oak_build_lpbk_frame
 *
 *  Description: build the test data frame for ge loopback
 *
 *  Input: frame buffer pointer; desitnation MAC, size, test base value
 *                and  increment value;
 *
 *  Returns: PASSED if successful;
 *           FAILED, otherwise
 *
 **********************************************************************
 */
int oak_build_lpbk_frame (dspif_lpbk_t *frame_ptr, mac_addr_t dst_mac_addr,
                            uint16 frm_size, char base_val, char inc_val)
{
    uint32 data_len, count;
    uchar data;
    uchar *datap;
    dspif_lpbk_t *framep;

    /* build ethernet frame header */
    framep = (dspif_lpbk_t *)frame_ptr;

    /* build ethernet frame payload */

    data = base_val;
    datap = (uchar *)&framep->data;
    data_len = frm_size - sizeof(ether_hdr_t);
    for (count = 0; count < data_len; count++) {
        *datap++ = data;
        data += inc_val;
    }

    return (PASSED);
}

/*
 **********************************************************************
 *
 *  Function: oak_check_rx_frame
 *
 *  Description: Check the recevied frame test data 
 *
 *  Input: test frame pointer; recv frame pointer; packet number and size  
 *
 *  Returns: PASSED if successful;
 *           FAILED, otherwise
 *
 **********************************************************************
 */
int oak_check_rx_frame (volatile dspif_lpbk_t * test_frame_p, 
                          volatile uchar *recv_frame_p, int packet_num, 
                          uint32 frm_size)
{
    int count, error = 0;
    uchar *rd_ptr, *wr_ptr;

    /*
     * verify that we received the correct number of bytes
     * the byte count in tx_bd->length does not include 4 bytes of CRC
     * while the byte count in rx_bd->length does include it
     */
        rd_ptr = (uchar *)(recv_frame_p + sizeof(ether_hdr_t));
        wr_ptr = (uchar *)(test_frame_p->data);
  
        for (count = 0; count < (frm_size - sizeof(ether_hdr_t)); count++) {
            if (*rd_ptr != *wr_ptr) {
                /* dump packet data */
                printf("\n Ether header for received pkt-");
                dismem((uchar *)recv_frame_p, sizeof(ether_hdr_t), 
                       (ulong)(recv_frame_p), 1);
                printf("\n Rx pkt data -");
                dismem((uchar *)(recv_frame_p + sizeof(ether_hdr_t)), count+4,
                       (ulong)(recv_frame_p+ sizeof(ether_hdr_t)), 1);
                printf("\n Tx pkt data -");
                dismem((uchar *)test_frame_p->data, count+4,
                       (ulong)(test_frame_p->data), 1);
                cterr('f', 0, "Packet%d data mismatch at offset %#x, "
                      "sent %#.8x, rcvd %#.8x\ntx bd @%#.8x, rx bd @%#.8x",
                      packet_num, count, *wr_ptr, *rd_ptr, test_frame_p, recv_frame_p);
                error = FAILED;
                break;
            }
            rd_ptr++;
            wr_ptr++;
        }

    return (error);
}

/*
 **********************************************************************
 *
 *  Function: c2ru_oak_check_rx_frame
 *
 *  Description: Check if the recevied frame test data is expected.
 *               Curie 2RU uses this function to drop unrelated packets.
 *
 *  Input: test frame pointer; recv frame pointer; packet number and size  
 *
 *  Returns: PASSED if successful;
 *           FAILED, otherwise
 *
 **********************************************************************
 */
int c2ru_oak_check_rx_frame(volatile dspif_lpbk_t * test_frame_p,
                            volatile uchar *recv_frame_p, int packet_num,
                            uint32 frm_size)
{
    int count;
    uchar *rd_ptr, *wr_ptr;

    if (!is_curie_2ru())
        return PASSED;

    rd_ptr = (uchar *)(recv_frame_p + sizeof(ether_hdr_t));
    wr_ptr = (uchar *)(test_frame_p->data);

    for (count = 0; count < (frm_size - sizeof(ether_hdr_t)); count++) {
        if (*rd_ptr != *wr_ptr) {
            printf("\ndrop unrelated packet: %x: %02x %02x %02x %02x\n",
                   count, rd_ptr[0], rd_ptr[1], rd_ptr[2], rd_ptr[3]);
            return FAILED;
        }
        rd_ptr++;
        wr_ptr++;
    }

    return PASSED;
}

/*
 **********************************************************************
 *
 *  Function: oak_eth_frames_test
 *
 *  Description: send and check received test frames
 *
 *  Input: frame_num -  frame number
 *         frm_size - frame size
 *         socket_gl - socket type
 *         dst_mac_addr - destination mac address
 *         dsp - dsp number
 *
 *  Returns: PASSED if successful;
 *           FAILED, otherwise     
 *
 **********************************************************************
 */
int oak_eth_frames_test (uint32 frame_num, uint32 frm_size, int socket_gl, 
                           mac_addr_t dst_mac_addr, int dsp)
{
    mac_addr_t   src_mac_addr;
    dspif_lpbk_t test_frame;
    dspif_lpbk_t *test_frame_p = &test_frame;
    eth_tx_pkt_t tx_pkt;
    eth_tx_pkt_t *tx_pkt_p = &tx_pkt;
    int   result = PASSED, wait_time = 0;
    uchar recv_frame_p[2048];
    char  base_val, inc_val;
    int retry_count;

    memset((uchar *)test_frame_p, 0, sizeof(dspif_lpbk_t));
    memset((uchar *)recv_frame_p, 0, sizeof(recv_frame_p));

    /* make ethernet frame, data pat used dependent on odd/even frame size */
    if (frm_size & 1) {
        base_val = 0;
        inc_val = 1;
    } else {
        base_val = 0xff;
        inc_val = -1;
    }

    /* make ethernet frame, data pat dependent on odd/even frame size */
    if (oak_build_lpbk_frame(test_frame_p, dst_mac_addr, frm_size,
                        base_val, inc_val) == FAILED) {
        return (FAILED);
    }

    get_host_mac_addr(0, (uchar *)&src_mac_addr[0]);
   
    memset((uchar *)tx_pkt_p, 0, sizeof(eth_tx_pkt_t));
    memcpy((uchar *)(tx_pkt_p->dest_addr), "FFFFFFFFFFFF", 
           sizeof(mac_addr_t));
    memset((uchar *)(tx_pkt_p->src_addr), 0, sizeof(mac_addr_t)); /* host MAC */
    memcpy((uchar *)(tx_pkt_p->src_addr), src_mac_addr, sizeof(mac_addr_t)); /* host MAC */

    tx_pkt_p->pkt_type = PKT_TYPE_IPV4;
 
    tx_pkt_p->payload_size = frm_size - sizeof(ether_hdr_t); /* payload size */
    tx_pkt_p->bufr_st_addr = (uchar *)&(test_frame_p->data); /* payload */
    tx_pkt_p->pkt_num = 0;
    tx_pkt_p->socket = socket_gl;

    msleep(2000); /* Wait till DSP is ready for the loopback frames */
    msleep(2000); /* Wait till DSP is ready for the loopback frames */
    prpass(testpass, "Sending lpbk frame#%d, size = %d ... ", frame_num, frm_size);
    result = eth_pkt_tx(tx_pkt_p);
    if (result != ETH_PKT_TX_OK ) {
        cterr('f', 0, "%s: Failed to TX lpbk Frame#%d, size = %d : ret = 0x%x,"
              " status = 0x%x", __FUNCTION__, frame_num, frm_size, result, 
              tx_pkt_p->tx_status);
        return (FAILED);
    }

    retry_count = 0;
retry:
    wait_time = 1000 * 60;
    prpass(testpass, "Waiting for lpbk response for frame#%d ", frame_num);
    while ((result = oak_wait_for_ge_packet(recv_frame_p, socket_gl, 
            INTR_MODE, dsp, wait_time)) == FAILED) {
        if (--wait_time <= 0) {
            cterr('f', 0, "Failed to RX lpbk Frame#%d, size = %d \n", frame_num, frm_size);
            return (FAILED);
        }
        msleep(1);
    }

    /* Curie 2RU: Found one unrelated packet received. We display and
     * drop it, and continue to receive next packet. if there is no next
     * packet received, timeout will be returned at above location */
    if (c2ru_oak_check_rx_frame(test_frame_p, recv_frame_p, frame_num,
                                frm_size) != PASSED) {
        retry_count++;

        if (retry_count < 5)
            goto retry;
    }

    if (result == PASSED) {
        result = oak_check_rx_frame (test_frame_p, recv_frame_p, frame_num, 
                                       frm_size);
        if (result == PASSED) {
            prpass(testpass, "RX frame #%d matches for size %d ", frame_num, 
                   frm_size);
        }
    }
    printf("\r                                                               ");
    return (result);
}

/*
 **********************************************************************
 *
 *  Function: oak_build_stop_command_packet
 *          
 *  Description: build test command
 *
 *  Input: select_test - Selected command test
 *         code_id - which dsp core
 *          
 *  Returns: None
 *
 **********************************************************************
 */
void oak_build_stop_command_packet (uint16_t select_test, uint8_t core_id)
{
    ngsm_entity_t *ep;
    dspif_ether_t *cmd_packet_p;

    assert(ngio_ptr);
    ep = ngio_ptr->priv;

    assert(ep);
    cmd_packet_p = &(ep->cmd_packet);

    cmd_packet_p->dspif_hdr.src_id = SWAP32(HOST_ID);
    cmd_packet_p->dspif_hdr.dest_id = SWAP32(ep->plat_ngsm_num);
    cmd_packet_p->dspif_hdr.op_type = (OP_TEST_STOP);
    cmd_packet_p->dspif_info.command = 0;
    cmd_packet_p->dspif_info.result = SWAP32(RESULT_RUNNING);
    cmd_packet_p->dspif_info.flags = SWAP32(FLAG_NULL);
    cmd_packet_p->dspif_info.select = (select_test);
    cmd_packet_p->dspif_info.faults = 0;
    cmd_packet_p->dspif_info.location = 0;
    cmd_packet_p->dspif_info.expected = 0;
    cmd_packet_p->dspif_info.actual = 0;
    cmd_packet_p->dspif_info.extra = 0;
    cmd_packet_p->dspif_info.errorcount = 0;
    cmd_packet_p->dspif_info.testcounter = 0;
    cmd_packet_p->dspif_info.ReadyOnTest = 0;
    cmd_packet_p->dspif_info.TestCtrl = 0;
    cmd_packet_p->dspif_info.WhoAmI = 0;
    cmd_packet_p->dspif_info.ver_no = 0;
    cmd_packet_p->dspif_info.wait_states = 0;
    cmd_packet_p->dspif_info.param1 = 0;
    cmd_packet_p->dspif_info.param2 = 0;
    cmd_packet_p->dspif_info.param3 = 0;
    cmd_packet_p->dspif_info.param4 = 0;
    memset((cmd_packet_p->dspif_info.bufmsg), 0, 128);
    memset((cmd_packet_p->dspif_info.errmsg), 0, 128);
}

/*
 **********************************************************************
 *
 *  Function: oak_wait_result_packet
 *
 *  Description: wait for result
 *
 *  Input: *pak - packet buffer
 *         socket_gl - socket type
 *         retry - retry times
 *         dsp - dsp number
 *
 *  Returns: PASSED - pkt received
 *           FAILED - pkt not received within the delay time
 *
 **********************************************************************
 */
int oak_wait_result_packet (uint8_t *pak, int socket_gl, uint16_t retry, int dsp)
{
    int ret_val = PASSED;
    int count = retry;

    memset(pak, 0, sizeof(dspif_ether_t));

    if (count < 2) {  /* warning programmer only */
        printf("retry count too small, not recommend!!!\n");
        return (FAILED);
    }

    /* Wait for Gige packet */
    while (count) {
        if ((oak_wait_for_ge_packet(pak, socket_gl, INTR_MODE, dsp, count)) == PASSED) {
            break;
        }
        count--;
        msleep(1000);
    }

    if (count == 0) {
        ret_val = FAILED;
    }

    return (ret_val);
}

/*
 **********************************************************************
 *
 *  Function: oak_ge_lpbk_test
 *
 *  Description: Send ge lpbk command and invoke DSP prepare to echo
 *               input packet.
 *
 *  Input: local_port - local ge port
 *
 *  Returns: PASSED if successful;
 *           FAILED, otherwise
 *
 **********************************************************************
 */
int oak_ge_lpbk_test (int local_port)
{
    ngsm_entity_t *ep;
    fe_packet_t   *recv_packet_p;
    dspif_ether_t *result_packet_p;
    int retval = PASSED;

    prpass(testpass, "Host <--> Oakenshield GE%d Ext Loopback test", local_port);

    assert(ngio_ptr);

    ep = (ngsm_entity_t *)ngio_ptr->priv;
    assert(ep);

    recv_packet_p = &(ep->recv_packet);
    result_packet_p = &(ep->result_packet);

#ifdef SR_ADD_LATER
    if (ep->fw_dnlded == FALSE) {
        if (lsi_sp27xx_app_fw_dnld(ngsm_num)) {
            return(FAILED);
        }
    }
#endif
    /* SR add rx check for mac as well as slot id */
    oak_clear_rx_buf((uchar *)result_packet_p, (int)sizeof(dspif_ether_t));
    oak_clear_rx_buf((uchar *)recv_packet_p, (int)sizeof(fe_packet_t));
    /*
     * build command packet to run dsp sanity test
     * test result will be handle in processing receiving packets
     */
    /* DSS_CORE0 does not matter here */
    if (local_port == 0)
        oak_build_command_packet(SELECT_GE0_LPBK, 0, 0, DSS_CORE0);
    else
        oak_build_command_packet(SELECT_GE1_LPBK, 0, 0, DSS_CORE0);
    /* need to know which DSP on the PVDM */
    if ((retval = oak_send_command_packet(0)) != PASSED) {
        printf("\n %s(): oak_send_command_packet() returned tx error %d\n",
               __FUNCTION__, retval);
        return (FAILED);
    }
    /* allow DSP to set up loopback connection */
    msleep(10);

    retval = oak_sgmii_dsp_lpbk_test(0, local_port);

    if (local_port == 0) {
        oak_build_stop_command_packet(SELECT_GE0_LPBK, DSS_CORE0);
    } else {
        oak_build_stop_command_packet(SELECT_GE1_LPBK, DSS_CORE0);
    }

    if ((retval = oak_send_command_packet(0)) != PASSED) {
        printf("\n %s(): oak_send_command_packet() returned tx error %d\n",
               __FUNCTION__, retval);
        return (FAILED);
    }
    prpass(testpass, " Sent STOP GE %d Loopback command packet ", local_port);

    if (oak_wait_result_packet((uint8_t *)recv_packet_p, ep->socket_gl, 20, 0)) {
        cterr('f', 0, "Timed out waiting for GE loopback test result.");
        return (FAILED);
    }
    if (retval != PASSED) {
        /* SGMII loopback failed */
        cterr('f', 0, "oak_ge_lpbk_test() SGMII loopback failed retval = %#x",
               retval);
        return (FAILED);
    }
    prpass(testpass, " Host <--> Oakenshield GE %d Loopback completed ", local_port);
    return (PASSED);
}


/*
 **********************************************************************
 *
 *  Function: load_fpga_data 
 *
 *  Description: Load FPGA image and use fopen to open fpga image file. 
 *          
 *  Input: *data - fpga image buffer 
 *
 *  Returns: PASSED if successful;
 *
 **********************************************************************
 */
int load_fpga_data (unsigned char * data) 
{
    FILE *pFile;
    int c, ix;
    unsigned int val;

    pFile = fopen("/firmware/fpga_image.h", "r");

    if (!pFile) {
        printf("\n\nCan't open file, please check the folder whether \
                            the fpga_image.h is exit or not\n\n");
        exit(0);
    }

    ix = 1;
    while (!feof(pFile)) {
        if (fscanf(pFile, "%x", &val) == EOF) {
            printf("End of file, %d bytes\n", ix);
            goto out;
        }

        data[ix - 1] = (unsigned char)val;

        if ((c = fgetc(pFile)) == EOF) {
            printf("End of file, %d bytes\n", ix);
            goto out;
        } else {
            if (c == ',') {
            } else {
                printf("File read done, %d bytes\n", ix);
                goto out;
            }
        }
        ix++;
    }

    out:
        fclose(pFile);
        printf("close file\n");
    return (PASSED);
}


/*
 **********************************************************************
 *
 *  Function: tx_data_to_dsp 
 *  
 *  Description: transfer fpga image to dsp. 
 *          
 *  Input: socket, tx_buffer_addr, tx_data_size 
 *
 *  Returns: PASSED if successful;
 *           FAILED otherwise;
 **********************************************************************
 */
int tx_data_to_dsp (int socket, uchar *tx_buffer_addr, int tx_data_size) 
{
    ngsm_entity_t *ep;
    eth_tx_pkt_t tx_pkt;
    eth_tx_pkt_t *tx_pkt_p = &tx_pkt;
    mac_addr_t   src_mac_addr;
    int retval;

    ep = (ngsm_entity_t *)ngio_ptr->priv;

    get_host_mac_addr(0, (uchar *)&src_mac_addr[0]);

    memset((uchar *)tx_pkt_p, 0, sizeof(eth_tx_pkt_t));
    memcpy((uchar *)(tx_pkt_p->dest_addr),
           (uchar *)&(ep->eth_hdr[0].dest_addr),
           sizeof(mac_addr_t));
    memset((uchar *)(tx_pkt_p->src_addr), 0, sizeof(mac_addr_t)); /* host MAC */
    memcpy((uchar *)(tx_pkt_p->src_addr), src_mac_addr, sizeof(mac_addr_t)); /* host MAC */

    tx_pkt_p->pkt_type = PKT_TYPE_IPV4;
    tx_pkt_p->payload_size = tx_data_size;
    tx_pkt_p->bufr_st_addr = tx_buffer_addr; /* Packet addr */ 
    tx_pkt_p->pkt_num = 0;
    tx_pkt_p->socket = socket;

    retval = eth_pkt_tx(tx_pkt_p);

    return (retval);
}

/*
 **********************************************************************
 *
 *  Function: rx_data_from_dsp 
 *  
 *  Description: receive fpga image from dsp. 
 *          
 *  Input: socket, recv_buffer_addr, buffer_size 
 *
 *  Returns: PASSED if successful;
 *           FAILED otherwise;
 **********************************************************************
 */
int rx_data_from_dsp (int socket, uchar *recv_buffer_addr, int buffer_size) 
{
    int retval = PASSED;
    eth_rx_pkt_t rx_pkt;
    eth_rx_pkt_t *rx_pkt_p = &rx_pkt;
    int wait_count = 1000;

    /* setup rx stucture for receiving */
    rx_pkt_p->bufr_st_addr = recv_buffer_addr;
    rx_pkt_p->rx_bufr_size = buffer_size;
    rx_pkt_p->pkt_num = 0;
    rx_pkt_p->wait_time = wait_count;
    rx_pkt_p->socket = socket;
    rx_pkt_p->rx_chk = 1;

    retval = eth_pkt_rx(rx_pkt_p);

    return (retval);
}

/*
 **********************************************************************
 *
 *  Function: txrx_dsp_data
 *
 *  Description: combine tx_data_to_dsp and rx_data_from_dsp to implement
 *               fpga upgrade.
 *
 *  Input: None
 *
 *  Returns: PASSED if successful; 
 *           FAILED, otherwise
 *
 **********************************************************************
 */
void *txrx_dsp_data (void)
{
    ngsm_entity_t *ep;
    int ix;
    mac_addr_t   src_mac_addr;
    uchar recv_buffer[1600] = {0};
    /* Because packet is 1000 bytes and first bytes is type so need 1001.*/
    uchar tx_buffer[1001] = {0};
    int timeout = TIME_OUT, tx_fpga_data_size;
    unsigned int fpga_image_size = 0x100000;
    unsigned int fpga_data_index = 0;

    oak_setup_ge_env();

    ep = (ngsm_entity_t *)ngio_ptr->priv;
    get_host_mac_addr(0, (uchar *)&src_mac_addr[0]);

    while (timeout--) {
        /* clear buffer before use */
        memset((uchar *)recv_buffer, 0, sizeof(recv_buffer));

        rx_data_from_dsp(ep->socket_gl, recv_buffer, sizeof(recv_buffer));

        if ((recv_buffer[14]) == PACKET_FROM_DSP) {
            break;
        }
    }
    if (timeout == 0) {
        printf("!!Don't receive packet from DSP\n");
        pthread_exit((void *)PASSED);
    }

    while (fpga_image_size != 0) {

        memset((uchar *)tx_buffer, 0, sizeof(tx_buffer));

        if (fpga_image_size >= 1000) {
            tx_fpga_data_size = 1000;
            fpga_image_size -= 1000;
        } else {
            tx_fpga_data_size = fpga_image_size;
            fpga_image_size = 0;
        }

        tx_buffer[0] = PACKET_TO_DSP;
        for (ix = 0; ix < tx_fpga_data_size; ix++) {
            tx_buffer[ix+1] = fpga_image[fpga_data_index];
            fpga_data_index++;
        }

        tx_data_to_dsp(ep->socket_gl, tx_buffer, sizeof(tx_buffer)); 

        timeout = TIME_OUT;
        while (timeout--) {
            /* clear buffer before use */
            memset((uchar *)recv_buffer, 0, sizeof(recv_buffer));

            rx_data_from_dsp(ep->socket_gl, recv_buffer, sizeof(recv_buffer));

            if ((recv_buffer[14]) == PACKET_FROM_DSP) {
                break;
            }
        }
        if (timeout == 0) {
            printf("!!Don't receive packet from DSP\n");
            pthread_exit((void *)PASSED);
        }
    }

    printf("Send FPGA Image to DSP done!!\n");
    pthread_exit((void *)PASSED);
}

/*
 **********************************************************************
 *
 *  Function: fpga_upgrade_utility 
 *
 *  Description: FPGA Upgrade Utility
 *
 *  Input: None
 *
 *  Returns: PASSED if successful; 
 *           FAILED, otherwise
 *
 **********************************************************************
 */
static int fpga_upgrade_utility (void)
{
    int rc;
    pthread_t threads;
    printf("FPGA Upgrade\n");

    printf("Loading FPGA Image\n");
    if (load_fpga_data(fpga_image) == FAILED) {
        printf("Load FPGA data Failed!!\n");
        return (FAILED);
    }

    printf("Create pthread for sending FPGA Image to DSP\n");
    rc = pthread_create (&threads, NULL, (void *)txrx_dsp_data, NULL);
    if (rc != PASSED) {
        printf("pthread_create failed \n");
        return (FAILED);
    }

    oakenshield_con_utils();
    pthread_cancel(threads);
    printf("Remove the thread!\n");
    return (PASSED);
}


/*
 **********************************************************************
 *
 *  Function: oak_dac_1dot5sm_high
 *
 *  Description: Set 1.5 voltage margin high.
 *
 *  Input: None
 *
 *  Returns: PASSED if successful; 
 *           FAILED, otherwise
 *
 **********************************************************************
 */
static int oak_dac_1dot5sm_high (void)
{
    printf("1.5 Voltage Margin High\n");
    return (oak_select_test(SELECT_DAC_1DOT5SM_HIGH, 0, 0, 100));
}

/*
 **********************************************************************
 *
 *  Function: oak_dac_1dot5sm_low
 *
 *  Description: Set 1.5 voltage margin low.
 *
 *  Input: None
 *
 *  Returns: PASSED if successful; 
 *           FAILED, otherwise
 *
 **********************************************************************
 */
static int oak_dac_1dot5sm_low (void)
{
    printf("1.5 Voltage Margin Low\n");
    return (oak_select_test(SELECT_DAC_1DOT5SM_LOW, 0, 0, 100));
}

/*
 **********************************************************************
 *
 *  Function: oak_dac_no_1dot5sm
 *
 *  Description: No 1.5 voltage margin.
 *
 *  Input: None
 *
 *  Returns: PASSED if successful; 
 *           FAILED, otherwise
 *
 **********************************************************************
 */
static int oak_dac_no_1dot5sm (void)
{
    printf("1.5 No Voltage Margin\n");
    return (oak_select_test(SELECT_DAC_NO_1DOT5SM, 0, 0, 100));
}

/*
 **********************************************************************
 *
 *  Function: oak_dac_3dot3sm_high
 *
 *  Description: Set 3.3 voltage margin high.
 *
 *  Input: None
 *
 *  Returns: PASSED if successful; 
 *           FAILED, otherwise
 *
 **********************************************************************
 */
static int oak_dac_3dot3sm_high (void)
{
    printf("3.3 Voltage Margin High\n");
    return (oak_select_test(SELECT_DAC_3DOT3SM_HIGH, 0, 0, 100));
}

/*
 **********************************************************************
 *
 *  Function: oak_dac_3dot3sm_low
 *
 *  Description: Set 3.3 voltage margin low.
 *
 *  Input: None
 *
 *  Returns: PASSED if successful; 
 *           FAILED, otherwise
 *
 **********************************************************************
 */
static int oak_dac_3dot3sm_low (void)
{
    printf("3.3 Voltage Margin Low\n");
    return (oak_select_test(SELECT_DAC_3DOT3SM_LOW, 0, 0, 100));
}

/*
 **********************************************************************
 *
 *  Function: oak_dac_no_3dot3sm
 *
 *  Description: No 3.3 voltage margin.
 *
 *  Input: None
 *
 *  Returns: PASSED if successful; 
 *           FAILED, otherwise
 *
 **********************************************************************
 */
static int oak_dac_no_3dot3sm (void)
{
    printf("3.3 No Voltage Margin\n");
    return (oak_select_test(SELECT_DAC_NO_3DOT3SM, 0, 0, 100));
}


/*
 **********************************************************************
 *
 *  Function: oak_dac_1DOT5_show
 *
 *  Description: Show voltage margin.
 *
 *  Input: None
 *
 *  Returns: PASSED if successful; 
 *           FAILED, otherwise
 *
 **********************************************************************
 */
static int oak_dac_1DOT5_show (void)
{
    ngsm_entity_t *ep_p;
    fe_packet_t   *recv_packet_p;
    dspif_ether_t *result_packet_p;
    uint8_t bufmsg[128];
    assert(ngio_ptr);

    ep_p = (ngsm_entity_t *)ngio_ptr->priv;

    assert(ep_p);

    printf("Show Voltage Margin\n");

    if (oak_select_test(SELECT_DAC_1DOT5_SHOW, 0, 0, 100) == FAILED) {
        return (FAILED);
    }
    recv_packet_p = &(ep_p->recv_packet);
    result_packet_p = &(ep_p->result_packet);

    memcpy((char *)result_packet_p, (char *)(&(recv_packet_p->data[0])+0),
          sizeof(dspif_ether_t));

    memcpy(bufmsg, result_packet_p->dspif_info.bufmsg, sizeof(bufmsg));

    printf("%s",bufmsg);

    return (PASSED);

}


/*
 **********************************************************************
 *
 *  Function: oak_dac_3DOT3_show
 *
 *  Description: Show voltage margin.
 *
 *  Input: None
 *
 *  Returns: PASSED if successful; 
 *           FAILED, otherwise
 *
 **********************************************************************
 */
static int oak_dac_3DOT3_show (void)
{
    ngsm_entity_t *ep_p;
    fe_packet_t   *recv_packet_p;
    dspif_ether_t *result_packet_p;
    uint8_t bufmsg[128];
    assert(ngio_ptr);

    ep_p = (ngsm_entity_t *)ngio_ptr->priv;

    assert(ep_p);

    printf("Show Voltage Margin\n");

    if (oak_select_test(SELECT_DAC_3DOT3_SHOW, 0, 0, 100) == FAILED) {

        return(FAILED);
    }
    recv_packet_p = &(ep_p->recv_packet);
    result_packet_p = &(ep_p->result_packet);

    memcpy((char *)result_packet_p, (char *)(&(recv_packet_p->data[0])+0),
          sizeof(dspif_ether_t));

    memcpy(bufmsg, result_packet_p->dspif_info.bufmsg, sizeof(bufmsg));

    printf("%s",bufmsg);

    return (PASSED);

}

/*
 **********************************************************************
 *
 *  Function: oak_dsp_ddr3_sdram_test_wrapper
 *
 *  Description: wrapper so we can set up and clear up ge.
 *
 *  Input: slot - slot number
 *
 *  Returns: PASSED if successful; 
 *           FAILED, otherwise
 *
 **********************************************************************
 */
static int oak_dsp_ddr3_sdram_test_wrapper (void)
{
    prpass(testpass, "DDR3 Memory - ");
    return (oak_select_test(SELECT_DSP_SDRAM, 0, 0, 800));

}


int oak_dsp_debug (int test, int wait_time)
{
    ngsm_entity_t *ep;
    dspif_mem_t   *mem_p;
    uint32_t ret_val, param1, param2;

    /* Register, buffer display for GE0, GE1 loopback, TDM Loopback,
       DDR3 memory, ECC memory */
    switch (test) {
    case SELECT_GE1_LPBK: 
        /* 		MAC0 0x30044000
           		MAC1 0x3004C000
           MAC Control and Status Registers : 0x0 - 0x60 
           MAC Transmit and Receive Counters : 0x800 - 0x818 
           MAC Receive Only Counters : 0x81C - 0x85C 
           MAC Transmit Only Counters : 0x860 - 0x8AC 
           MAC Counter Carry and Interrupt Mask Registers : 0x8B0 - 0x8C0 
         */

        /* 		PCE0 0x30040000
           		PCE1 0x30048000
           DLT<0-2047> : 0x0 - 0x1FFC
           Configuration Control Registers : 0x2000 - 0x208C
           Status Registers : 0x2200 - 0x2228
           Counter Registers : 0x2240 - 0x22D4
           Queue Register Set 0-8 : 0x2400-241C, 2420-243C, ... 24A0-24BC
           Match and Mask RAM : 0x2800-28FC
           UDL Look-Up table<0-255> : 0x2C00-2FFC
         */

        /* 		Ethernet Transmit DMA
           		Ethernet TXD0 Registers 0x30043000
           		Ethernet TXD1 Registers 0x3004B000
            0x0 - 0x10C
            0x800 - 0xA18 TXD Queue Register Set 0-8 (800-818, 840-858 ...)
         */
        param1 = 0x30044000; /* Specify Address */
        param2 = 0x60; /* Size in bytes */
        break;
    case SELECT_TDM_EXTLPBK:
        /*
          		TDM SIU
          SIU<0-5> : 0x98010000, 0x98010100 ... 0x98010500
          Registers : 0x0 - FC
         */
        /*
          		TDM SWTU
          Status and control Registers<0-5> : 0x98010800, 0x98010900 ... 0x98010D00
            Registers : 0x0 - 2C

          Source Channel Register Set<0-5> : 0x98020000, 0x98022000 ... 0x9802A000
            Register Set<0-255> : 0x0-0C, 0x10-1C, 0x20-2C ... 0xFF0-FFC

          Destination Channel Register Set<0-5> : 0x98021000, 0x98023000 ... 0x9802B000
            Register Set<0-255> : 0x0-0C, 0x10-1C, 0x20-2C ... 0xFF0-FFC
         */
        /*
          		TDM Interrupt control: 0x98011000

          Interrupt Registers for DSS0: 0x00-14
          Interrupt Registers for DSS1: 0x18-2C
          Interrupt Registers for DSS2: 0x30-44
          Interrupt Registers for DSS3: 0x48-5C
          Interrupt Registers for PPB: 0x60-80
          Port Synchronization control Registers: 0x90-94
            Registers : 0x0 - 2C
         */
        /*
          		TDM Universal Counters: 0x98011000
           Universal counter Register Set 0: 0x100-104
           Universal counter Register Set 1: 0x108-10C
           ...
           Universal counter Register Set 31: 0x1F8-0x1FC
         */
        param1 = 0x98010000;
        param2 = 0xFC;
        break;
    }
    ret_val = oak_dsp_test(SELECT_MEM_DISP, param1, param2, wait_time);
    if (ret_val == PASSED) {
        assert(ngio_ptr);

        ep = (ngsm_entity_t *)ngio_ptr->priv;
        assert(ep);

        mem_p = (dspif_mem_t *)&(ep->recv_packet.data);
        printf("\n DSP param1 = 0x%x, param2 = %d\n", mem_p->dspif_info.param1, 
               mem_p->dspif_info.param2);
        dismem((uchar *)(mem_p->pkt_data), param2, 
               (ulong)(mem_p->pkt_data), 4);
    }

    oak_cleanup_ge_env();

    return (PASSED);
}

/*
 **********************************************************************
 *
 *  Function: oak_get_testcommand_id
 *
 *  Description: get the index of the test command
 *
 *  Input: SELECT command id 
 *  
 *  Returns: index in teh oakenshield_command table
 *
 **********************************************************************
 */
int oak_get_testcommand_id (int test)
{
    int ix;

    for (ix = 0; ix < sizeof(oakenshield_command); ix++) {
        if (oakenshield_command[ix].test_id == test)
            return ix;
   }
   return (ix);
}

/*
 **********************************************************************
 *
 *  Function: oak_dsp_test
 *
 *  Description: Send command and invoke DSP DDR2 SDRAM test on DSPs.
 *
 *  Input: test - test command
 *         param0 - parameter 0
 *         param1 - parameter 1
 *         wait_time - wait time unit is  second
 *  
 *  Returns: PASSED if successful; 
 *           FAILED - tx of command failed, rx time out, test failed
 *                    (RESULT_FAILED), test of rx not the same as test 
 *                    of tx 
 *
 **********************************************************************
 */
int oak_dsp_test (int test, int param0, int param1, int wait_time)
{  
    ngsm_entity_t *ep;
    fe_packet_t   *recv_packet_p;
    dspif_ether_t *result_packet_p;
    int i, num_dsp = 0, dsp = 0, retval;
    uint8_t errbuf[128];

    assert(ngio_ptr);

    ep = (ngsm_entity_t *)ngio_ptr->priv;
    assert(ep);

    result_packet_p = &(ep->result_packet);
    recv_packet_p = &(ep->recv_packet);

    num_dsp = ep->num_dsp;

    for (dsp = 0; dsp < num_dsp; dsp++) {
        oak_clear_rx_buf((uchar *)result_packet_p, 
                           (int)sizeof(dspif_ether_t));
        oak_clear_rx_buf((uchar *)recv_packet_p, 
                           (int)sizeof(fe_packet_t));
        /* 
         * build command packet to run dsp sanity test 
         * test result will be handle in processing receiving packets
         */
        oak_build_command_packet(test, param0, param1, DSS_CORE0);
        /* need to know which DSP on the PVDM */
        if ((retval = oak_send_command_packet(dsp)) != PASSED) {
            cterr('f', 0,"%s(): oak_send_command_packet() returned tx "
                  "error %d for test %d\n", __FUNCTION__, retval, test);
            return (FAILED);
        }
        if (wait_time == 0) { /* do not need reply */
            return (PASSED);
        }
        /* wait for result packet */
        if (oak_wait_result_packet((uint8_t *)recv_packet_p, ep->socket_gl, 
                                     wait_time, dsp)) {
            i = oak_get_testcommand_id(test);
            /* DSP probably not responding */
            cterr('f', 0, "%s(): Timed out waiting for DSP test(0x%x):%s result."
                  " Waiting for %d secs", __FUNCTION__, test, oakenshield_command[i].test_name, (wait_time));
            return (FAILED);
        }
        memcpy((char *)result_packet_p, (char *)(&(recv_packet_p->data[0])+0),
               sizeof(dspif_ether_t));
        /* parse result */
        if (result_packet_p->dspif_info.result != (RESULT_SUCCESSFUL)) {
            /* need to copy out errmsg and display here */
            memcpy(errbuf, result_packet_p->dspif_info.errmsg, sizeof(errbuf));
            /* Call routine to dump registers/memory for debug */
            cterr('f', 0, "%s(): Failed on dsp%d test(%d), result: 0x%x: %s",
                  __FUNCTION__, dsp, test, (result_packet_p->dspif_info.result)
                  , errbuf);
            return (result_packet_p->dspif_info.result);
        }
        /* parse select command */
        if (result_packet_p->dspif_info.select != (test)) {
            printf("\n dspif_info.select = 0x%x, expected 0x%x", 
                   result_packet_p->dspif_info.select, test);
            /* need to copy out errmsg and display here */
            memcpy(errbuf, result_packet_p->dspif_info.errmsg, sizeof(errbuf));
            cterr('f', 0, "%s(): Failed on dsp%d, test command: 0x%x: %s",
                  __FUNCTION__, dsp, (result_packet_p->dspif_info.select), 
                  errbuf);
            return (FAILED);
        }
    }
    return (PASSED);

}

/*
 **********************************************************************
 *
 *  Function: oak_select_test
 *
 *  Description: wrapper so we can set up and clear up ge.
 *
 *  Input: test - test to run on DSP
 *         param1, param2 - parameters if any for the test
 *         wait_time - response time for the result from DSP
 *
 *  Returns: PASSED if successful; 
 *           FAILED, otherwise
 *
 **********************************************************************
 */
static int oak_select_test (int test, int param1, int param2, int wait_time)
{
    int ret_val = PASSED;

    assert(ngio_ptr);

    /* Bringup DSP */
    if (oakenshield_bringup_dsp() == FAILED) {
        cterr('f', 0, "%s(): \n DSP READY PIN not set. Please check if "
               "\"/firmware/dsp_sp2700_fw.img\" exists.\n", __FUNCTION__);
        return (FAILED);
    }

    if (oak_setup_ge_env() == FAILED) {
        return (FAILED);
    }

    ret_val = oak_dsp_test(test, param1, param2, wait_time);

    if (ret_val == RESULT_FAILED) {
        /* Please get, display register/memory dump to debug failure */
        oak_dsp_debug(test, wait_time);
        ret_val = FAILED;
    }

    oak_cleanup_ge_env();

    if (is_curie_2ru()) {
        /* Curie2RU: EMAC0 failed if running after DDR3 RAM test without any
         * interval. Add 10 seconds delay */
        if (test == SELECT_DSP_SDRAM) {
            msleep(10000);
        }
    }

    return (ret_val);

}

/*
 **********************************************************************
 *
 *  Function: oak_tdm_ext_lpbk_test_wrapper
 *
 *  Description: wrapper so we can set up and clear up ge.
 *
 *  Input: slot - slot number
 *
 *  Returns: PASSED if successful; 
 *           FAILED, otherwise 
 *
 **********************************************************************
 */
static int oak_tdm_ext_lpbk_test_wrapper (void)
{ 
#ifdef ENHANCED_ERR_MSG_EXAMPLE
    uchar ngsm_get_pid[FRU_SIZE] = {0};
    uchar ngsm_get_loc[FRU_SIZE] = {0};
#endif

#ifdef ENHANCED_ERR_MSG_EXAMPLE
    /*
     *      * 1. Subtests of the test function will reuse all variables
     *      * 2. All variables will be cleared automatically when
     *           entering and leaving each menu item.
     * */
    /* Segment 1: PID | Unique_string : slot_info */
    fru_table_offset = MB;
    /* fru_table_offset should be set, otherwise, it will not */
    /* go to enhanced error message format in cterr() */
    /* set fru_table_offset to get the predefine value */
    /* or change mb_pid & mb_loc below */

    sprintf(fru_board_id , "%x", oak_board_id);
    memcpy(ngsm_get_pid,(char*)&fru_board_id,5); 
    strcpy((char *)ngsm_get_loc, "TDM External Loopback test");

    platform_fru_table[fru_table_offset].pid_string = ngsm_get_pid ; 
    platform_fru_table[fru_table_offset].location_string = ngsm_get_loc;
 
    /* Segment 2: Test step captured from prpass */
    /* Segment 3: Failure message captured from cterr */

    /* Segment 4: Components used */
    cterr_add_component("DSP -> FPGA -> DSP");

    /* Segment 5: register and memory dump */

    /* Segment 6: Platform Environment initialized here*/

    /* Segment 7: Top 3 Debugging Steps */
    cterr_add_debug("Do FPGA register test, to check whether the SPI interface work.",
                    "Do FPGA memory test to check wheher the memory field can be accessed.",
                    "Measuring the signal of TDM interface, it might something problem on hardware.");
#endif
    prpass(testpass, " TDM External Loopback - ");

    assert(ngio_ptr);
 
    if(oak_select_test(SELECT_TDM_EXTLPBK, 0, 0, 100) == FAILED) {
        get_buf_message();     
        return (FAILED);
    }

    return (PASSED);
}

/*
 **********************************************************************
 *
 *  Function: ecc_mem_test 
 *
 *  Description: wrapper to run the ECC memory test
 *
 *  Input:none
 *
 *  Returns: PASSED if successful; 
 *           FAILED, otherwise 
 *
 **********************************************************************
 */
static int ecc_mem_test (void)
{
    prpass(testpass, " ECC Memory - ");

    assert(ngio_ptr);

    return (oak_select_test(SELECT_ECC_MEM, 0, 0, 100));
}



/**********************************************************************
 *
 * Function: fxs_si32261_common_mode_calibration
 *
 * Description: The routine performs fxs calibration on all ports
 * 
 * Input :  dummy - dont care
 *
 * Output:  PASSED/FAILED
 *
 *********************************************************************
*/
static int fxs_si32261_common_mode_calibration (int dummy)
{
    int result, dsp; 
    uchar ch;
    ngsm_entity_t *ngsm_ep;

    ngsm_ep = (ngsm_entity_t *)ngio_ptr->priv;

    printf("\nFXS LB Calibration\n");

    printf("For the calibration, user need remove all the attached connector!!\n");
    printf("Continue ? (y/n) ");
    ch = getchar();
    printf(" %c\n", ch);
    if ((ch == 'n') || (ch == 'N')) {
        printf("Ignore do claibrationn\n");
        return (PASSED);

    } else {
        for (dsp = 0; dsp < MAX_DSPS_PER_NGSM; dsp++) {
            ngsm_ep->dsp_downloaded[dsp] = FALSE;
        }

        /* Bringup DSP */
        if (oakenshield_bringup_dsp() == FAILED) {
            cterr('f', 0, "%s(): \n DSP READY PIN not set. Please check if "
                   "\"/firmware/dsp_sp2700_fw.img\" exists.\n", __FUNCTION__);
            return (FAILED);
        }

        printf("Do FXS initialization\n");
        printf("(This procedure takes a while. Please be patient)\n\n");
        result = oak_select_test(SELECT_CODEC_SI32261_CALIBRATION, 0, 0, 3000);
        if (result == FAILED) {
            cterr('f', 0,"%s(): SI32261 do calibration fail", __FUNCTION__);
            get_buf_message();
            return (FAILED);
        }


        printf("\n\nNow collecting calibration result ...\n");

        result = oak_select_test(SELECT_CODEC_SI32261_CALIBRATE_SAVE, 0, 0, 1000);
        if (result == FAILED) {
            cterr('f', 0,"%s(): Save calibration data fail", __FUNCTION__);
            get_buf_message();
            return (FAILED);
        }

        for (dsp = 0; dsp < MAX_DSPS_PER_NGSM; dsp++) {
            ngsm_ep->dsp_downloaded[dsp] = FALSE;
        }

    }

    return (PASSED);

}

/**********************************************************************
 *
 * Function: fxs_si32261_ring
 *
 * Description:This function will set fxs port ring.
 * 
 * Input :  none
 *
 * Output:  PASSED/FAILED
 *
 *********************************************************************
*/
static int fxs_si32261_ring (void)
{
    char *oak_pid;
    int fxs_port = 0, port, ix;
    ngsm_entity_t *ep;

    assert(ngio_ptr);

    ep = (ngsm_entity_t *)ngio_ptr->priv;
    assert(ep);

    oak_pid = ep->pid;

    for (ix = 0; ix < (sizeof(PID_table)/sizeof(pid_info_t)); ix++) {
        if (strncmp(oak_pid, PID_table[ix].pid, strlen(PID_table[ix].pid)) == 0) {
            fxs_port = PID_table[ix].fxs_port;
        }
    }
    if (fxs_port == 0) {
        cterr('f', 0,"%s(): oakenshield PID wrong PID: %s.",__FUNCTION__, oak_pid);
        return (FAILED);
    }
    printf("FXS Ring Utility\n"); 
    port = gethex_answer("Ring port: ", 0, 0x0, fxs_port);
    if (oak_select_test(SELECT_CODEC_SI32261_RING, port, 0, 200) == FAILED) {
        return (FAILED);
    }

    get_buf_message();     
    return (PASSED);
}

/**********************************************************************
 *
 * Function: is_fxo
 *
 * Description: This function will read pid and return true 
 *              if the skew have fxo port else return false.
 * 
 * Input :  dummy - dont care
 *
 * Output:  TRUE/FALSE
 *
 *********************************************************************
*/
static boolean is_fxo (void)
{
    char *oak_pid;
    ngsm_entity_t *ep;

    assert(ngio_ptr);

    ep = (ngsm_entity_t *)ngio_ptr->priv;
    assert(ep);

    oak_pid = ep->pid;
    /* Just compare 72FXS SKU and can know whether the SKU has FXO or not.*/
    if (strncmp(oak_pid, PID_table[3].pid, strlen(PID_table[3].pid)) == 0) {
        return (FALSE);
    } else {
        return (TRUE);
    }

}
/*
 **********************************************************************
 *
 *  Function: fail_over_port_utility
 *
 *  Description: wrapper to run failed over port
 *
 *  Input: None
 *
 *  Returns: PASSED if successful; 
 *           FAILED, otherwise 
 *
 **********************************************************************
 */
static int fail_over_port_utility (void)
{
    uchar ch;

    prpass(testpass, "FAILED OVER PORT- ");

    printf("FAILED OVER PORT, \n");
    printf("Enable ? (y/n) ");
    ch = getchar();
    printf(" %c\n", ch);
    if ((ch == 'n') || (ch == 'N')) {
        oak_select_test(SELECT_CODEC_SET_FAIL_OVER_PORT, 0, 0, 100);
    } else {
        oak_select_test(SELECT_CODEC_SET_FAIL_OVER_PORT, TRUE, 0, 100);
    }

    return (PASSED);
}


/*
 **********************************************************************
 *
 *  Function:  led_utility
 *
 *  Description: wrapper to run LED test
 *
 *  Input: None
 *
 *  Returns: PASSED if successful; 
 *           FAILED, otherwise 
 *
 **********************************************************************
 */
static int led_utility (void)
{
    prpass(testpass, "FXS/FXO LED - ");

    assert(ngio_ptr);
    printf("LTC4215 OIR LED\n");
    oir_ltc4215_leds_test(oir_if);

    printf("Turn on/off ACT LED\n");
    oak_select_test(SELECT_FXS_FXO_LED, 0, 0, 100);

    return (PASSED);
}

/*
 **********************************************************************
 *
 *  Function:  fpga_reg_test
 *
 *  Description: wrapper to run FPGA Register test
 *
 *  Input: None
 *
 *  Returns: PASSED if successful; 
 *           FAILED, otherwise 
 *
 **********************************************************************
 */
static int fpga_reg_test (void)
{
#ifdef ENHANCED_ERR_MSG_EXAMPLE
    uchar ngsm_get_pid[FRU_SIZE] = {0};
    uchar ngsm_get_loc[FRU_SIZE] = {0};
#endif

#ifdef ENHANCED_ERR_MSG_EXAMPLE
    /*
     *      * 1. Subtests of the test function will reuse all variables
     *        2. All variables will be cleared automatically when
     *           entering and leaving each menu item
     * */
    /* Segment 1: PID | Unique_string : slot_info */
    fru_table_offset = MB;
    /* fru_table_offset should be set, otherwise, it will not */
    /* go to enhanced error message format in cterr() */
    /* set fru_table_offset to get the predefine value */
    /* or change mb_pid & mb_loc below */

    sprintf(fru_board_id , "%x", oak_board_id);
    memcpy(ngsm_get_pid,(char*)&fru_board_id,5); 
    strcpy((char *)ngsm_get_loc, "FPGA Register Test");

    platform_fru_table[fru_table_offset].pid_string = ngsm_get_pid ; 
    platform_fru_table[fru_table_offset].location_string = ngsm_get_loc;
 
    /* Segment 2: Test step captured from prpass */
    /* Segment 3: Failure message captured from cterr */

    /* Segment 4: Components used */
    cterr_add_component("DSP -> SPI Interface ->FPGA registers -> SPI interface -> DSP");

    /* Segment 5: register and memory dump */

    /* Segment 6: Platform Environment initialized here*/

    /* Segment 7: Top 3 Debugging Steps */
    cterr_add_debug("Check SPI connection between DSP and FPGA.",
                    "If there is no problem on the interface, replace FPGA and re-test it.");
#endif
    prpass(testpass, " FPGA Register Test - ");

    
    if (oak_select_test(SELECT_FPGA_REG_TEST, 0, 0, 100) == FAILED) {
        return(FAILED);
    }
    
    get_buf_message();     

    return (PASSED);
}

/*
 **********************************************************************
 *
 *  Function:  fpga_mem_test
 *
 *  Description: wrapper to run FPGA Memory Test
 *
 *  Input: None
 *
 *  Returns: PASSED if successful; 
 *           FAILED, otherwise 
 *
 **********************************************************************
 */
static int fpga_mem_test (void)
{

#ifdef ENHANCED_ERR_MSG_EXAMPLE
    uchar ngsm_get_pid[FRU_SIZE] = {0};
    uchar ngsm_get_loc[FRU_SIZE] = {0};
#endif

#ifdef ENHANCED_ERR_MSG_EXAMPLE
    /*
     *      * 1. Subtests of the test function will reuse all variables
     *        2. All variables will be cleared automatically when
     *           entering and leaving each menu item.
     */
    /* Segment 1: PID | Unique_string : slot_info */
    fru_table_offset = MB;
    /* fru_table_offset should be set, otherwise, it will not */
    /* go to enhanced error message format in cterr() */
    /* set fru_table_offset to get the predefine value */
    /* or change mb_pid & mb_loc below */

    sprintf(fru_board_id , "%x", oak_board_id);
    memcpy(ngsm_get_pid,(char*)&fru_board_id,5); 
    strcpy((char *)ngsm_get_loc, "FPGA Memory Test");

    platform_fru_table[fru_table_offset].pid_string = ngsm_get_pid ; 
    platform_fru_table[fru_table_offset].location_string = ngsm_get_loc;
 
    /* Segment 2: Test step captured from prpass */
    /* Segment 3: Failure message captured from cterr */

    /* Segment 4: Components used */
    cterr_add_component("DSP -> SPI Interface ->TDMSW16 within FPGA -> SPI interface -> DSP");

    /* Segment 5: register and memory dump */

    /* Segment 6: Platform Environment initialized here*/

    /* Segment 7: Top 3 Debugging Steps */
    cterr_add_debug("Do FPGA register test first, because the TDMSW_CMD_STATUS/ TDMSW_ADR/ TDMSW_DATA registers provide indirect access to TDMSW16 registers.",
                    "Check SPI connection between DSP and FPGA.",
                    "If there is no problem on the interface, replace FPGA and re-test it.");
#endif
    prpass(testpass, " FPGA Memory Test - ");

    if(oak_select_test(SELECT_FPGA_MEM_TEST, 0, 0, 100) == FAILED) {
        get_buf_message();     
        return (FAILED);
    }

    return (PASSED);
}

/*
 **********************************************************************
 *
 *  Function:  fpga_int_test
 *
 *  Description: wrapper to run FPGA Memory Test
 *
 *  Input: None
 *
 *  Returns: PASSED if successful; 
 *           FAILED, otherwise 
 *
 **********************************************************************
 */
static int fpga_int_test (void)
{

#ifdef ENHANCED_ERR_MSG_EXAMPLE
    uchar ngsm_get_pid[FRU_SIZE] = {0};
    uchar ngsm_get_loc[FRU_SIZE] = {0};
#endif
#ifdef ENHANCED_ERR_MSG_EXAMPLE
    /*
     *      * 1. Subtests of the test function will reuse all variables
     *        2. All variables will be cleared automatically when
     *           entering and leaving each menu item.
     */
    /* Segment 1: PID | Unique_string : slot_info */
    fru_table_offset = MB;
    /* fru_table_offset should be set, otherwise, it will not */
    /* go to enhanced error message format in cterr() */
    /* set fru_table_offset to get the predefine value */
    /* or change mb_pid & mb_loc below */

    sprintf(fru_board_id , "%x", oak_board_id);
    memcpy(ngsm_get_pid,(char*)&fru_board_id,5); 
    strcpy((char *)ngsm_get_loc, "FPGA interrupt Test");

    platform_fru_table[fru_table_offset].pid_string = ngsm_get_pid ; 
    platform_fru_table[fru_table_offset].location_string = ngsm_get_loc;
 
    /* Segment 2: Test step captured from prpass */
    /* Segment 3: Failure message captured from cterr */

    /* Segment 4: Components used */
    cterr_add_component("DSP -> SPI Interface -> FPGA -> SPI interface -> DSP");

    /* Segment 5: register and memory dump */

    /* Segment 6: Platform Environment initialized here*/

    /* Segment 7: Top 3 Debugging Steps */
    cterr_add_debug("Do FPGA register test first, because the FPGA_INT_EVENT/ FPGA_INT DIAG_TEST/ FPGA_INT_EVENT_ENA registers provide FPGA to interrupt.",
                    "Measuring  the signal  of interrupt GPIO pin 0x4 signal, it should be high when FPGA interrupt trigger.",
                    "If there is no problem on the interface, replace FPGA and re-test it.");
#endif
    prpass(testpass, " FPGA Interrupt Test - ");

    if(oak_select_test(SELECT_FPGA_INT_TEST, 0, 0, 100) == FAILED) {
        get_buf_message();     
        return (FAILED);
    }

    return (PASSED);
}

/*
 **********************************************************************
 *
 *  Function:  fpga_tdmsw_force_test
 *
 *  Description: wrapper to run fpga tdmsw16 force test
 *
 *  Input: None
 *
 *  Returns: PASSED if successful; 
 *           FAILED, otherwise 
 *
 **********************************************************************
 */
static int fpga_tdmsw_force_test (void)
{

#ifdef ENHANCED_ERR_MSG_EXAMPLE
    uchar ngsm_get_pid[FRU_SIZE] = {0};
    uchar ngsm_get_loc[FRU_SIZE] = {0};
#endif

#ifdef ENHANCED_ERR_MSG_EXAMPLE
    /*
     *      * 1. Subtests of the test function will reuse all variables
     *        2. All variables will be cleared automatically when
     *           entering and leaving each menu item.
     */
    /* Segment 1: PID | Unique_string : slot_info */
    fru_table_offset = MB;
    /* fru_table_offset should be set, otherwise, it will not */
    /* go to enhanced error message format in cterr() */
    /* set fru_table_offset to get the predefine value */
    /* or change mb_pid & mb_loc below */

    sprintf(fru_board_id , "%x", oak_board_id);
    memcpy(ngsm_get_pid,(char*)&fru_board_id,5); 
    strcpy((char *)ngsm_get_loc, "FPGA TDMSW16 force byte test");

    platform_fru_table[fru_table_offset].pid_string = ngsm_get_pid ; 
    platform_fru_table[fru_table_offset].location_string = ngsm_get_loc;
 
    /* Segment 2: Test step captured from prpass */
    /* Segment 3: Failure message captured from cterr */

    /* Segment 4: Components used */
    cterr_add_component("DSP -> SPI Interface ->TDMSW16 within FPGA -> SPI interface -> DSP");

    /* Segment 5: register and memory dump */

    /* Segment 6: Platform Environment initialized here*/

    /* Segment 7: Top 3 Debugging Steps */
    cterr_add_debug("Do FPGA register test first, because the TDMSW_CMD_STATUS/ TDMSW_ADR/ TDMSW_DATA registers provide indirect access to TDMSW16 registers.",
                    "Check SPI connection between DSP and FPGA.",
                    "If there is no problem on the interface, replace FPGA and re-test it.");
#endif
    prpass(testpass, " FPGA TDMSW16 Force Byte Test - ");

    assert(ngio_ptr);

    if(oak_select_test(SELECT_FPGA_TDMSW_FORCE_BYTE_TEST, 0, 0, 100) == FAILED) {
        get_buf_message();     
        return (FAILED);
    }

    return (PASSED);
}

/*
 **********************************************************************
 *
 *  Function: si32261_loopback_test
 *
 *  Description: wrapper to run FXS SI32261 Loopback test
 *
 *  Input: None
 *
 *  Returns: PASSED if successful; 
 *           FAILED, otherwise 
 *
 **********************************************************************
 */
static int fxs_si32261_lpbk_test (void)
{

#ifdef ENHANCED_ERR_MSG_EXAMPLE
    uchar ngsm_get_pid[FRU_SIZE] = {0};
    uchar ngsm_get_loc[FRU_SIZE] = {0};
#endif

#ifdef ENHANCED_ERR_MSG_EXAMPLE
    /*
     *      * 1. Subtests of the test function will reuse all variables
     *        2. All variables will be cleared automatically when
     *           entering and leaving each menu item.
     */
    /* Segment 1: PID | Unique_string : slot_info */
    fru_table_offset = MB;
    /* fru_table_offset should be set, otherwise, it will not */
    /* go to enhanced error message format in cterr() */
    /* set fru_table_offset to get the predefine value */
    /* or change mb_pid & mb_loc below */

    sprintf(fru_board_id , "%x", oak_board_id);
    memcpy(ngsm_get_pid,(char*)&fru_board_id,5); 
    strcpy((char *)ngsm_get_loc, "SI32261 Loopback test");

    platform_fru_table[fru_table_offset].pid_string = ngsm_get_pid ; 
    platform_fru_table[fru_table_offset].location_string = ngsm_get_loc;
 
    /* Segment 2: Test step captured from prpass */
    /* Segment 3: Failure message captured from cterr */

    /* Segment 4: Components used */
    cterr_add_component("DSP -> FPGA -> FXS -> FPGA -> DSP");

    /* Segment 5: register and memory dump */

    /* Segment 6: Platform Environment initialized here*/

    /* Segment 7: Top 3 Debugging Steps */
    cterr_add_debug("Do TDM External Loopback test , verify TDM bus between DSP and FPGA.",
                    "Read the Si32261/Si3050 chip ID, you can use this utility in FXS/FXO utility.",
                    "Measuring the signal of TDM interface, it might something problem on hardware");
#endif
    int result = PASSED;

    prpass(testpass, " FXS SI32261 Loopback - ");

    assert(ngio_ptr);

    result = oak_select_test(SELECT_CODEC_SI32261_DIGITAL_LOOPBACK, 0, 0, 100);

    get_buf_message();

    return (result);
}

/*
 **********************************************************************
 *
 *  Function: fxo_si3050_loopback_test
 *
 *  Description: wrapper to run FXS SI32261 Loopback test
 *
 *  Input: None
 *
 *  Returns: PASSED if successful; 
 *           FAILED, otherwise 
 *
 **********************************************************************
 */
static int fxo_si3050_lpbk_test (void)
{

#ifdef ENHANCED_ERR_MSG_EXAMPLE
    uchar ngsm_get_pid[FRU_SIZE] = {0};
    uchar ngsm_get_loc[FRU_SIZE] = {0};
#endif

#ifdef ENHANCED_ERR_MSG_EXAMPLE
    /*
     *      * 1. Subtests of the test function will reuse all variables
     *        2. All variables will be cleared automatically when
     *           entering and leaving each menu item.
     */
    /* Segment 1: PID | Unique_string : slot_info */
    fru_table_offset = MB;
    /* fru_table_offset should be set, otherwise, it will not */
    /* go to enhanced error message format in cterr() */
    /* set fru_table_offset to get the predefine value */
    /* or change mb_pid & mb_loc below */
    sprintf(fru_board_id , "%x", oak_board_id);
    memcpy(ngsm_get_pid,(char*)&fru_board_id,5); 
    strcpy((char *)ngsm_get_loc, "SI3050 Loopack test");

    platform_fru_table[fru_table_offset].pid_string = ngsm_get_pid; 
    platform_fru_table[fru_table_offset].location_string = ngsm_get_loc;
 
    /* Segment 2: Test step captured from prpass */
    /* Segment 3: Failure message captured from cterr */

    /* Segment 4: Components used */
    cterr_add_component("DSP -> FPGA -> FXO -> FPGA -> DSP");

    /* Segment 5: register and memory dump */

    /* Segment 6: Platform Environment initialized here*/

    /* Segment 7: Top 3 Debugging Steps */
    cterr_add_debug("Do TDM External Loopback test , verify TDM bus between DSP and FPGA.",
                    "Read the Si32261/Si3050 chip ID, you can use this utility in FXS/FXO utility.",
                    "Measuring the signal of TDM interface, it might something problem on hardware");
#endif
    int result = FAILED;
    prpass(testpass, " FXO SI3050 Loopback - ");

    assert(ngio_ptr);
    
    result = oak_select_test(SELECT_CODEC_SI3050_DIGITAL_LOOPBACK, 0, 0, 100);
     
    get_buf_message();

    return (result);
}


/*  
 **********************************************************************
 *  
 *  Function: oak_arm11_cpu1_boot_test_wrapper
 *  
 *  Description: wrapper to test the CPU1 for boot up
 *
 *  Input: None
 *
 *  Returns: PASSED if successful; 
 *           FAILED, otherwise
 *
 **********************************************************************
 */
static int oak_arm11_cpu1_boot_test_wrapper (void)
{
    prpass(testpass, " ARM11 CPU1 Boot - ");
    return (oak_select_test(SELECT_ARM11CPU1_BOOT, 0, 0, 50));
}

/*  
 **********************************************************************
 *  
 *  Function: oak_dss_core0_sanity_test_wrapper
 *  
 *  Description: wrapper so we can set up dss core0.
 *
 *  Input: none
 *
 *  Returns: PASSED if successful; 
 *           FAILED, otherwise
 *
 **********************************************************************
 */
static int oak_dss_core0_sanity_test_wrapper (void)
{
    prpass(testpass, " DSS Core0 Sanity - ");
    return (oak_select_test(SELECT_DSS0_SANITY, 0, 0, 50));
}

/*  
 **********************************************************************
 *  
 *  Function: oak_dss_core1_sanity_test_wrapper
 *  
 *  Description: wrapper so we can set up dss core1.
 *
 *  Input: none
 *
 *  Returns: PASSED if successful; 
 *           FAILED, otherwise
 *
 **********************************************************************
 */ 
static int oak_dss_core1_sanity_test_wrapper (void)
{   
    prpass(testpass, " DSS Core1 Sanity - ");
    return (oak_select_test(SELECT_DSS1_SANITY, 0, 0, 50));
}   

/*  
 **********************************************************************
 *  
 *  Function: oak_dss_core2_sanity_test_wrapper
 *  
 *  Description: wrapper so we can set up dss core2.
 *
 *  Input: none
 *
 *  Returns: PASSED if successful; 
 *           FAILED, otherwise
 *
 **********************************************************************
 */ 
static int oak_dss_core2_sanity_test_wrapper (void)
{   
    ngsm_entity_t *ngsm_ep;

    assert(ngio_ptr);
    ngsm_ep = (ngsm_entity_t *)ngio_ptr->priv;
    assert(ngsm_ep);

    prpass(testpass, " DSS Core2 Sanity - ");
    if (ngsm_ep->ngsm_id == PFUSE123_SP2702) {
        printf("\r LSI SP2702 does not support DSS2");
        return (PASSED);
    }
    return (oak_select_test(SELECT_DSS2_SANITY, 0, 0, 50));
}   

/*  
 **********************************************************************
 *  
 *  Function: oak_dss_core3_sanity_test_wrapper
 *  
 *  Description: wrapper so we can set up core3.
 *
 *  Input: none
 *
 *  Returns: PASSED if successful; 
 *           FAILED, otherwise
 *
 **********************************************************************
 */ 
static int oak_dss_core3_sanity_test_wrapper (void)
{   
    ngsm_entity_t *ngsm_ep;

    assert(ngio_ptr);
    ngsm_ep = (ngsm_entity_t *)ngio_ptr->priv;
    assert(ngsm_ep);

    prpass(testpass, " DSS Core3 Sanity - ");
    if (ngsm_ep->ngsm_id == PFUSE123_SP2702) {
        printf("\r LSI SP2702 does not support DSS3");
        return (PASSED);
    }

    return (oak_select_test(SELECT_DSS3_SANITY, 0, 0, 50));
}   


/*
 **********************************************************************
 *
 *  Function: oak_ge_lpbk_test_wrapper
 *
 *  Description: wrapper so we can test ge loopback.
 *
 *  Input: port - ge0 or ge1
 *
 *  Returns: PASSED if successful;
 *           FAILED, otherwise
 *
 **********************************************************************
 */
static int oak_ge_lpbk_test_wrapper (int port)
{
    /* Curie platform skips GE1 lpbk test */
    if (port == 0x1 && skip_oakenshield_ge1_lpbk()) {
        return (PASSED);
    }

    int ret_val = PASSED;
 
    assert(ngio_ptr);

    if (oak_setup_ge_env() == FAILED) {
        return (FAILED);
    }

    if (oak_ge_lpbk_test(port)) {
        ret_val = FAILED;
    }
    oak_cleanup_ge_env();

    return (ret_val);
}

/*
 **********************************************************************
 *
 *  Function: oakenshield_utils
 *
 *  Description: NGSM utilities menu
 *
 *  Input: None 
 *
 *  Returns: PASSED
 *
 **********************************************************************
 */
static int oakenshield_utils (void)
{
    ngsm_entity_t *ngsm_ep;

    assert(ngio_ptr);
    ngsm_ep = (ngsm_entity_t *)ngio_ptr->priv;
    assert(ngsm_ep);

    sprintf(oakutiltitle, "%s Utilities Menu", ngsm_ep->slot_type_str);
    build_primary_submenu(oak_utils_submenu_table,
                          OAK_UTILS_SUBMENU_TABLE_SZ,
                          oakutiltitle, &oak_util_submenup);

    build_secondary_submenu(oak_utils_submenu_table,
                            OAK_UTILS_SUBMENU_TABLE_SZ,
                            oak_utils_secondary_items);

    menu(oak_util_submenup, oak_utils_secondary_items, '\0');

    return (PASSED);
}

/*
 **********************************************************************
 *
 *  Function: oak_iface_test
 *
 *  Description: Run interface test 
 *
 *  Input: None 
 *
 *  Returns: PASSED if successful;
 *           FAILED - otherwise 
 *
 **********************************************************************
 */
int oak_iface_test (void)
{
    int retval = PASSED;

    retval = oak_gpio_exp_test();

    if (retval == PASSED) {
        retval = ltc4215_register_test();
    }

    if (retval == PASSED) {
        retval = oak_ge_lpbk_test_wrapper(1);
    }

    if (retval == PASSED) {
        retval = oak_uart_test();
    }

    return (retval);
}


/*
 **********************************************************************
 *
 *  Function: oakenshield_run_test
 *
 *  Description: Run all the Oakenshield NGSM specific tests.
 *
 *  Input: none
 *  
 *  Returns: PASSED if successful; 
 *           FAILED, otherwise
 *  
 **********************************************************************
 */
static int oakenshield_run_test (void)
{
    int  retval = PASSED;

    assert(ngio_ptr);

    /* All these tests are executed by sending a ethernet test command
       packet to the Oakenshield ARM CPU0 */
    retval = oak_dsp_ddr3_sdram_test_wrapper();

    if (retval == PASSED) {
        retval = oak_ge_lpbk_test_wrapper(0);
    }
    if (retval == PASSED) {
        retval = oak_ge_lpbk_test_wrapper(1);
    }
    if (retval == PASSED) {
        retval = oak_arm11_cpu1_boot_test_wrapper();
    }
    if (retval == PASSED) {
        retval = oak_dss_core0_sanity_test_wrapper();
    }
    if (retval == PASSED) {
        retval = oak_dss_core1_sanity_test_wrapper();
    }
    if (retval == PASSED) {
        retval = oak_dss_core2_sanity_test_wrapper();
    }
    if (retval == PASSED) {
        retval = oak_dss_core3_sanity_test_wrapper();
    }
    if (retval == PASSED) {
        retval = ecc_mem_test();
    }
    if (retval == PASSED) {
        retval = fpga_reg_test();
    }
    if (retval == PASSED) {
        retval = fpga_mem_test();
    }
    if (retval == PASSED) {
        retval = fpga_int_test();
    }
    if (retval == PASSED) {
        retval = oak_tdm_ext_lpbk_test_wrapper();
    }
    if (retval == PASSED) {
        retval = fpga_tdmsw_force_test();
    }
    if (retval == PASSED) {
        retval = fxs_si32261_lpbk_test();
    }
    if ((retval == PASSED) && (is_fxo() == TRUE)) {
        retval = fxo_si3050_lpbk_test();
    }
    if (retval == PASSED) {
        retval = oak_uart_test();
    }

    return (retval);
}


/*
 **********************************************************************
 *
 *  Function: oakenshield_test
 *
 *  Description: show oakenshield test submenu or    
 *
 *  Input: 
 *
 *  Returns: PASSED if successful;
 *           FAILED - DSP ready pin not set, the test failed
 *
 **********************************************************************
 */
static int oakenshield_test (int run_tests)
{
    int retval = PASSED;

    testname ("Oakenshield NGSM");

    /* Bringup DSP */
    if (oakenshield_bringup_dsp() == FAILED) {
        cterr('f', 0, "%s(): \n DSP READY PIN not set. Please check if "
               "\"/firmware/dsp_sp2700_fw.img\" exists.\n", __FUNCTION__);
        return (FAILED);
    }

    if (ngio_ptr->test_type == IFACE_TEST)
        return(oak_iface_test());

    if (oak_dsp_tests_use_enet == 1) {
        if (run_tests == 1) {
            retval = oakenshield_run_test();
        } else {
            build_primary_submenu(oak_tests_submenu_table,
                                  OAK_TESTS_SUBMENU_TABLE_SIZE,
                                  oaksubmenutitle, &oak_submenup);
            build_secondary_submenu(oak_tests_submenu_table,
                                    OAK_TESTS_SUBMENU_TABLE_SIZE,
                                    oak_tests_secondary_items);

            menu(&oak_subtest_menu, oak_tests_secondary_items, '\0');
        }
    } else {
        oakenshield_con_utils();
    }
    return retval;
}


/*
 **********************************************************************
 *
 *  Function: oakenshield_con_utils
 *
 *  Description: wrapper so we can set up and clear up ge.
 *
 *  Input: None 
 *
 *  Returns: PASSED if successful;
 *
 **********************************************************************
 */
static int oakenshield_con_utils (void)
{
    ngsm_entity_t *ep;
    char disp[128]; 

    assert(ngio_ptr);

    ep = (ngsm_entity_t *)ngio_ptr->priv;
    assert(ep);

    /* Connect to the LSI SP27XX console using uart interface. 
     * LSI SP27XX is out of resest and its bootloader has downloaded the 
     * application firmware and diags menu is up. 
     */
    printf("\n\n Type <ctrl-a> <ctrl-x> to return to host console\n");
    fflush(stdout);
    fflush(stderr);
    msleep(1000);

    sprintf(disp, "picocom -b 9600 -d8 -pn -fn %s", ep->tty_dev);
    printf("\n %s\n\n\n", disp);
    fflush(stdout);
    fflush(stderr);
    msleep(1000);
    system(disp);
    return (PASSED);
}

/*
 **********************************************************************
 *
 *  Function: oak_wait_for_ge_packet
 *
 *  Description: Wait for Ethernet packets
 *
 *  Input: pak - received packet buffer
 *         mode - POLL_MODE or INTR_MODE
 *
 *  Returns: PASSED if successful; 
 *           FAILED - If no pkt recvd, rx errors, or MAC of recvd pkt 
 *                    does not match expected
 *
 **********************************************************************
 */
int oak_wait_for_ge_packet (uchar *pak, int socket_gl, int mode, int dsp,
                              int disp_err)
{
    ngsm_entity_t *ep;
    eth_rx_pkt_t rx_pkt;
    eth_rx_pkt_t * rx_pkt_p = &rx_pkt;
    int wait_count = 1000;
    int status; 
    uchar recv_buffer[1600]; /* need to take largest packet (1518) */

    assert(ngio_ptr);

    /* Need to store the MAC address of the NGSM and host */
    ep = (ngsm_entity_t *)ngio_ptr->priv;
    assert(ep);

    /* clear buffer before use */
    memset((uchar *)pak, 0, sizeof(fe_packet_t));
    memset((uchar *)rx_pkt_p, 0, sizeof(eth_rx_pkt_t));

    /* setup rx stucture for receiving */
    rx_pkt_p->bufr_st_addr = recv_buffer;
    rx_pkt_p->rx_bufr_size = sizeof(recv_buffer);
    rx_pkt_p->pkt_num = 0;
    rx_pkt_p->wait_time = wait_count;
    rx_pkt_p->socket = socket_gl;
    rx_pkt_p->rx_chk = 1;
    /* now wait for rx from GEMAC attached to backplane GE switch 
     * return can be ETH_NO_PKT_RX (0x4), ETH_PKT_RX_ERR (0x1)
     * or ETH_PKT_RX_OK (0x0). Error message for ETH_PKT_RX_ERR will
     * be printed by the source function.
     */
    status = eth_pkt_rx(rx_pkt_p);
    if (status) {
        /* disp_err will always be a non-zero value */ {
        if (disp_err == 1) 
            printf("\n %s(): eth_pkt_rx() returned error (no rx or rx errors) "
                   "%d\n", __FUNCTION__, status);
        }
        return (FAILED); /* retry is provided by caller */
    }

    /* Make sure we received packet from the expected destination */
    if (chk_macaddr(recv_buffer, (uchar *)&(ep->eth_hdr[dsp].src_addr[0])) != 0) {
#ifdef DEBUG_RE_PACKET
        printf("\n Received MAC does not match MAC in cookie. Try again");
        printf("\n Expected dest MAC : 0x%x:0x%x:0x%x:0x%x:0x%x:0x%x", 
               ep->eth_hdr[dsp].dest_addr[0], ep->eth_hdr[dsp].dest_addr[1],
               ep->eth_hdr[dsp].dest_addr[2], ep->eth_hdr[dsp].dest_addr[3],
               ep->eth_hdr[dsp].dest_addr[4], 
               ep->eth_hdr[dsp].dest_addr[5]);
        printf("\n Received MAC : ");
        printf("0x%x:0x%x:0x%x:0x%x:0x%x:0x%x\n",  recv_buffer[6],
               recv_buffer[7],  recv_buffer[8],
               recv_buffer[9],  recv_buffer[10],
               recv_buffer[11]);

        printf("\n Expected source MAC : 0x%x:0x%x:0x%x:0x%x:0x%x:0x%x", 
               ep->eth_hdr[dsp].src_addr[0], ep->eth_hdr[dsp].src_addr[1],
               ep->eth_hdr[dsp].src_addr[2], ep->eth_hdr[dsp].src_addr[3],
               ep->eth_hdr[dsp].src_addr[4], 
               ep->eth_hdr[dsp].src_addr[5]);
        printf("\n Received MAC : ");
        printf("0x%x:0x%x:0x%x:0x%x:0x%x:0x%x\n",  recv_buffer[0],
               recv_buffer[1],  recv_buffer[2],
               recv_buffer[3],  recv_buffer[4],
               recv_buffer[5]);
#endif
        return (FAILED);
    }

#ifdef DEBUG_RE_PACKET
    printf("\n Expected MAC : 0x%x:0x%x:0x%x:0x%x:0x%x:0x%x", 
           ep->eth_hdr[dsp].src_addr[0], ep->eth_hdr[dsp].src_addr[1],
           ep->eth_hdr[dsp].src_addr[2], ep->eth_hdr[dsp].src_addr[3],
           ep->eth_hdr[dsp].src_addr[4], 
           ep->eth_hdr[dsp].src_addr[5]);
    printf("\n Received MAC : ");
    printf("0x%x:0x%x:0x%x:0x%x:0x%x:0x%x\n",  recv_buffer[0],
           recv_buffer[1],  recv_buffer[2],
           recv_buffer[3],  recv_buffer[4],
           recv_buffer[5]);
#endif

    /* copy received to user pak */
    memcpy ((char *)pak, (uchar *)recv_buffer, sizeof(fe_packet_t));

    return (PASSED);
}

/******** History ********
$Log: ngsm_oakenshield.c,v $
Revision 1.7  2020/01/09 01:02:17  jiajliu
Merge Curie 2RU to main trunk

Revision 1.6  2019/08/06 06:56:09  alpeng
merge curie, switzer and nightwatch to trunk

Revision 1.5.32.1  2018/10/16 07:32:58  leschen
Using function skip_oakenshield_ge1_lpbk for Curie to skip ge1 lpbk.

Revision 1.5  2017/09/29 03:04:55  harrchan
CSCvg10075: Fix the bug of PID string compare

Revision 1.4  2017/09/26 07:48:14  harrchan
CSCvg04479: Support oakenshield firmware tftp download

Revision 1.3  2017/08/14 08:29:07  harrchan
Rename TDM External loopback test.(CSCvf54988)

Revision 1.2  2017/07/28 07:49:43  harrchan
Collapse Oakenshield-branch to Main Trunk.

Revision 1.1.2.7  2017/05/11 01:37:25  harrchan
Add enhanced error message.

Revision 1.1.2.6  2017/03/30 10:27:42  harrchan
Add fpga upgrade utility

Revision 1.1.2.5  2017/03/09 07:48:36  harrchan
Support oakenshield double wide case

Revision 1.1.2.4  2017/02/20 08:27:58  olin2
Add show Voltage Margin utility

Revision 1.1.2.3  2017/02/09 06:29:49  olin2
Add voltage margin and fail over port utility

Revision 1.1.2.2  2017/01/17 05:01:28  olin2
Support do test at platform side

Revision 1.1.2.1  2016/12/21 01:05:17  olin2
Initial commit code for Oakenshield



$Endlog$
*/


