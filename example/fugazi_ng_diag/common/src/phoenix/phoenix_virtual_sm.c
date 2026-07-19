/* $Id: phoenix_virtual_sm.c,v 1.2 2021/04/15 00:52:27 achiu2 Exp $ 
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/phoenix/phoenix_virtual_sm.c,v $ 
 *******************************************************************************
 * phoenix_virtual_sm.c : Phoenix virtual SM code
 *
 * Dec 2019 - Honda Wang
 *
 * Copyright (c) 2020 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *******************************************************************************
 */
#include <termios.h>
#include <sys/time.h>
#include <sys/select.h>
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
#include "i2c_api.h"
#include "phoenix_virtual_sm.h"
#include "common_utils.h"
#include "diag_eth_pkt_txrx.h"
#include "diag_eth_pkt_txrx_api.h"
#include "diag_eth_pkt_txrx_utils.h"
#include "diag_fpga.h"
#include "linux_api.h"
#include "bcm_gesw_defs.h"
#include "dash_fpga.h"
#include "dev_ltc4215.h"
#include "oir_ltc4215_api.h"
#include "slot.h"

extern int tftp_get(unsigned char *, unsigned char *,
    unsigned char *, unsigned char *, unsigned int);
static struct ngio_intf_t *ngio_ptr;
static ngsm_entity_t phoenix_lsi_entity[MAX_NUM_NGSM];
static char pca_buff[256];
static void *oir_if;
static unsigned char fpga_image[0x200000];
static unsigned char cpld_image[0x10000];

int phoenix_dsp_test(int , int , int , int);
static int phoenix_wait_result_packet(uint8_t *, int, uint16_t, int);
static int phoenix_con_utils(void);
static int phoenix_dsp_ddr3_sdram_test_wrapper(void);
static int phoenix_ge_lpbk_test_wrapper(int);
static int phoenix_tdm_ext_lpbk_test_wrapper(void);
static int ecc_mem_test(void);
static int phoenix_arm11_cpu1_boot_test_wrapper(void);
static int phoenix_dss_core_sanity_test_wrapper(int);
int phoenix_pwr_on(void);
static int phoenix_disp_mac(void);
static int phoenix_disp_mem(void);
static int phoenix_disp_fw_ver(void);
int phoenix_gpio_resistor_test(void);
static int gpio_exp_read(void);
static int gpio_exp_write(void);
static int phoenix_dac_1dot5sm_high(void);
static int phoenix_dac_1dot5sm_low(void);
static int phoenix_dac_no_1dot5sm(void);
static int phoenix_dac_3dot3sm_high(void);
static int phoenix_dac_3dot3sm_low(void);
static int phoenix_dac_no_3dot3sm(void);
static int phoenix_dac_1DOT5_show(void);
static int phoenix_dac_3DOT3_show(void);
static int phoenix_select_test(int, int, int, int);
static int phoenix_test(int);
static int phoenix_utils(void);
static int phoenix_uart_test(void);
static int phoenix_tdm_codec_reset_test(int);
static int phoenix_gpio_exp_test(void);
static int phoenix_bringup_dsp(void);
static int phoenix_ready_test(int);
static int phoenix_setup_ge_env(void);
static void phoenix_clear_rx_buf(uchar *, int);
static void phoenix_build_command_packet(uint16_t, uint32_t, uint32_t, uint8_t);
static void phoenix_build_config_packet(void);
static void (*phoenix_saved_diag_exec)(void) = NULL;
static int phoenix_send_command_packet(int dsp_no);
static int phoenix_cleanup_ge_env(void);
static int phoenix_eth_frames_test(uint32, uint32, int, mac_addr_t, int, int);
static int phoenix_wait_for_ge_packet(uchar *, int, int, int, int);
static int fxs_si32261_lpbk_test(int);
static int fxo_si3050_lpbk_test(void);
static int fxs_si32261_common_mode_calibration (int);
int phoenix_pwr_off(void);
static void phoenix_ngio_off(void);
static void *ngsm_p;
static int fpga_reg_test(void);
static int fpga_mem_test(void);
static int fpga_int_test(void);
static int fpga_tdmsw_force_test(void);
static int fail_over_port_utility(void);
static int phoenix_run_test(void);
static int vol_margin_utility(int);
static int fpga_reg_utility(int);
static int read_fpga_dir_reg(void);
static int write_fpga_dir_reg(void);
static int read_fpga_indir_reg(void);
static int write_fpga_indir_reg(void);
static int fpga_upgrade_utility(void);
static int phoenix_vir_setup_hw_brd_type(void);
void toggle_sep_test_dbx_flag(void);
ushort phoenix_board_id;
char fru_board_id[10];
uint32_t phoenix_sep_test_dbx_flag = PHOENIX_TEST_ALL_BOARD;
static int cpld_upgrade_utility(void);

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

#define OIR_SUBMENU_TABLE_SIZE (sizeof(oir_submenu_table) / \
                                sizeof(submenu_xtable_t))

/* 
 * Primary & secondary submenu items (filled in from xtable)
 */

/*=========================================
 * Voltage Margin menu items
 *=========================================
 */
static submenu_xtable_t vol_submenu_table[] = {
    {"1.5 Voltage Margin High", (PFT)phoenix_dac_1dot5sm_high, 0,0,(type_t(*)())0,
                                    0, (type_t(*)())0, 0},
    {"1.5 Voltage Margin Low",  (PFT)phoenix_dac_1dot5sm_low,  0,0,(type_t(*)())0,
                                    0, (type_t(*)())0, 0},
    {"1.5 No Voltage Margin",   (PFT)phoenix_dac_no_1dot5sm,    0,0,(type_t(*)())0,
                                    0, (type_t(*)())0, 0},
    {"3.3 Voltage Margin High", (PFT)phoenix_dac_3dot3sm_high, 0,0,(type_t(*)())0,
                                    0, (type_t(*)())0, 0},
    {"3.3 Voltage Margin Low",  (PFT)phoenix_dac_3dot3sm_low,  0,0,(type_t(*)())0,
                                    0, (type_t(*)())0, 0},
    {"3.3 No Voltage Margin",   (PFT)phoenix_dac_no_3dot3sm,    0,0,(type_t(*)())0,
                                    0, (type_t(*)())0, 0},
    {"Show 1.5 Voltage Margin", (PFT)phoenix_dac_1DOT5_show,    0,0,(type_t(*)())0,
                                    0, (type_t(*)())0, 0},
    {"Show 3.3 Voltage Margin", (PFT)phoenix_dac_3DOT3_show,    0,0,(type_t(*)())0,
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
int phoenix_dsp_tests_use_enet = 1;

submenu_xtable_t phoenix_tests_submenu_table[] = {
    {"Virtual SM Utilities",   (PFT)phoenix_utils,    0,0,(type_t(*)())0,
                                    0, (type_t(*)())0, 0},
    {"DDR3 Memory test", (PFT)phoenix_dsp_ddr3_sdram_test_wrapper,         0,MM_2, 
                         (type_t(*)())0, 0, (type_t(*)())0, 0},
    {"EMAC0 loopback test", (PFT)phoenix_ge_lpbk_test_wrapper,    0,MM_2, 
                         (type_t(*)())0, 0,     (type_t(*)())0, 0},
    {"ARM11 CPU1 Boot test ", (PFT)phoenix_arm11_cpu1_boot_test_wrapper, 0,MM_2, 
                         (type_t(*)())0, 0,     (type_t(*)())0, 0},
    {"DSS Core0 Sanity test ", (PFT)phoenix_dss_core_sanity_test_wrapper, DSS_CORE0, MM_2, 
                         (type_t(*)())0, 0,     (type_t(*)())0, 0},
    {"DSS Core1 Sanity test ", (PFT)phoenix_dss_core_sanity_test_wrapper, DSS_CORE1, MM_2, 
                         (type_t(*)())0, 0,     (type_t(*)())0, 0},
    {"DSS Core2 Sanity test ", (PFT)phoenix_dss_core_sanity_test_wrapper, DSS_CORE2, MM_2, 
                         (type_t(*)())0, 0,     (type_t(*)())0, 0},
    {"DSS Core3 Sanity test ", (PFT)phoenix_dss_core_sanity_test_wrapper, DSS_CORE3, MM_2, 
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
    {"FPGA TDM Loopback test", (PFT)phoenix_tdm_ext_lpbk_test_wrapper,0,MM_2, 
                         (type_t(*)())0, 0,     (type_t(*)())0, 0},
    {"MB SI32261 Loopback test",  (PFT)fxs_si32261_lpbk_test, PHOENIX_TEST_ONLY_MB, MM_2,
                         (type_t(*)())0, 0,     (type_t(*)())0, 0},
    {"DB1 SI32261 Loopback test", (PFT)fxs_si32261_lpbk_test, PHOENIX_TEST_ONLY_DB1, MM_2,
                         (type_t(*)())has_db1, 0,     (type_t(*)())0, 0},
    {"DB2 SI32261 Loopback test", (PFT)fxs_si32261_lpbk_test, PHOENIX_TEST_ONLY_DB2, MM_2,
                         (type_t(*)())has_db2, 0,     (type_t(*)())0, 0},
    {"DB3 SI32261 Loopback test", (PFT)fxs_si32261_lpbk_test, PHOENIX_TEST_ONLY_DB3, MM_2,
                         (type_t(*)())has_db3, 0,     (type_t(*)())0, 0},
    {"DB1 SI3050 Loopack test",         (PFT)fxo_si3050_lpbk_test,         0,MM_2,
                         (type_t(*)())db1_has_fxo, 0,     (type_t(*)())0, 0},
    {"Uart Loopack test",           (PFT)phoenix_uart_test,                0,MM_2,
                         (type_t(*)())0, 0,     (type_t(*)())0, 0},
    {"MB TDM Codec Reset test", (PFT)phoenix_tdm_codec_reset_test, PHOENIX_TEST_ONLY_MB, MM_2,
                        (type_t(*)())0, 0, (type_t(*)())0, 0},
    {"DB1 TDM Codec Reset test", (PFT)phoenix_tdm_codec_reset_test, PHOENIX_TEST_ONLY_DB1, MM_2,
                        (type_t(*)())has_db1, 0, (type_t(*)())0, 0},
    {"DB2 TDM Codec Reset test", (PFT)phoenix_tdm_codec_reset_test, PHOENIX_TEST_ONLY_DB2, MM_2,
                        (type_t(*)())has_db2, 0, (type_t(*)())0, 0},
    {"DB3 TDM Codec Reset test", (PFT)phoenix_tdm_codec_reset_test, PHOENIX_TEST_ONLY_DB3, MM_2,
                        (type_t(*)())has_db3, 0, (type_t(*)())0, 0},

};


#define PHOENIX_TESTS_SUBMENU_TABLE_SIZE (sizeof(phoenix_tests_submenu_table) / \
                                       sizeof(submenu_xtable_t))

/* 
 * Primary & secondary submenu items (filled in from xtable)
 */
static mitem_t phoenix_tests_primary_items[PHOENIX_TESTS_SUBMENU_TABLE_SIZE + 10];
static mitem_t phoenix_tests_secondary_items[PHOENIX_TESTS_SUBMENU_TABLE_SIZE + 10];

menuinfo_t phoenix_subtest_menu = {
    "%s Subtest Menu",
    0,                                /* mtparam added by init_empty_menu */
    (PFT)show_endnote,                /* notes missing WICs in combos */
    0,                                /* use generic prompt */
    0,                                /* size (bumped by add_menu_item() */
    phoenix_tests_primary_items,
};
menuinfo_t *phoenix_submenup = &phoenix_subtest_menu;

submenu_xtable_t phoenix_utils_submenu_table[] = {
    {"Switch Console",                 (PFT)phoenix_con_utils,  0,0,(type_t(*)())0,
                                    0, (type_t(*)())0},
    {"Display Virtual SM MAC",        (PFT)phoenix_disp_mac,     0,0,(type_t(*)())0,
                                    0, (type_t(*)())0},
    {"Display Virtual SM Firmware Version",
                                    (PFT)phoenix_disp_fw_ver,  0,0,(type_t(*)())0,
                                    0, (type_t(*)())0},
    {"Display Virtual SM Memory",     (PFT)phoenix_disp_mem,     0,0,(type_t(*)())0,
                                    0, (type_t(*)())0},
    {"PCA9557 Register Read",       (PFT)gpio_exp_read,      0,0,(type_t(*)())0, 
                                    0, (type_t(*)())0},
    {"PCA9557 Register Write",      (PFT)gpio_exp_write,     0,0,(type_t(*)())0, 
                                    0, (type_t(*)())0},
    {"Voltage Margin Utility",      (PFT)vol_margin_utility,  0,0,(type_t(*)())not_phoenix,
                                    0, (type_t(*)())0, 0},
    {"Failed Over Port Utility",    (PFT)fail_over_port_utility,   0,0,(type_t(*)())0,
                                    0, (type_t(*)())0, 0},
    {"FXS Calibration All with SPI ROM read/write check Utility",
        (PFT)fxs_si32261_common_mode_calibration,  PHOENIX_TEST_ALL_BOARD, 0,
        (type_t(*)())0, 0, (type_t(*)())0, 0},
    {"FXS Calibration MB with SPI ROM read/write check Utility",
        (PFT)fxs_si32261_common_mode_calibration,  PHOENIX_TEST_ONLY_MB, 0,
        (type_t(*)())0, 0, (type_t(*)())0, 0},
    {"FXS Calibration DB1 with SPI ROM read/write check Utility",
        (PFT)fxs_si32261_common_mode_calibration,  PHOENIX_TEST_ONLY_DB1, 0,
        (type_t(*)())has_db1, 0, (type_t(*)())0, 0},
    {"FXS Calibration DB2 with SPI ROM read/write check Utility",
        (PFT)fxs_si32261_common_mode_calibration,  PHOENIX_TEST_ONLY_DB2, 0,
        (type_t(*)())has_db2, 0, (type_t(*)())0, 0},
    {"FXS Calibration DB3 with SPI ROM read/write check Utility",
        (PFT)fxs_si32261_common_mode_calibration,  PHOENIX_TEST_ONLY_DB3, 0,
        (type_t(*)())has_db3, 0, (type_t(*)())0, 0},
    {"FPGA Register Utility",        (PFT)fpga_reg_utility,   0,0,(type_t(*)())0,
                                    0, (type_t(*)())0, 0},
    {"FPGA Upgrade Utility",        (PFT)fpga_upgrade_utility,  0,0,(type_t(*)())0,
                                    0, (type_t(*)())0, 0},
    {"toggle flag : phoenix_sep_test_dbx_flag",
                                    (PFT)toggle_sep_test_dbx_flag,  0,0,(type_t(*)())0,
                                    0, (type_t(*)())0, 0},
    {"CPLD Upgrade Utility",        (PFT)cpld_upgrade_utility,  0,0,(type_t(*)())0,
                                    0, (type_t(*)())0, 0},
};

#define PHOENIX_UTILS_SUBMENU_TABLE_SZ (sizeof(phoenix_utils_submenu_table) / \
                                       sizeof(submenu_xtable_t))

/* 
 * Primary & secondary submenu items (filled in from xtable)
 */
static mitem_t phoenix_utils_primary_items[PHOENIX_UTILS_SUBMENU_TABLE_SZ + MAX_BASE_ITEMS];
static mitem_t phoenix_utils_secondary_items[PHOENIX_UTILS_SUBMENU_TABLE_SZ + MAX_BASE_ITEMS];
char phoenixutiltitle[50];

menuinfo_t phoenix_util_submenu = {
    phoenixutiltitle,
    0,                                /* mtparam added by init_empty_menu */
    (PFT)menu_show_dflags,                /* notes missing WICs in combos */
    0,                                /* use generic prompt */
    0,                                /* size (bumped by add_menu_item() */
    phoenix_utils_primary_items,
};

menuinfo_t *phoenix_util_submenup = &phoenix_util_submenu;

static submenu_xtable_t phoenix_mainmenu_tbl[] = {
    {"Virtual SM Utilities",        (PFT)phoenix_utils,    0,0,(type_t(*)())0,
                                    0,    (type_t(*)())0},
    {"GPIO Expander test",        (PFT)phoenix_gpio_exp_test,      0,MM_2,(type_t(*)())0,
                                    0,    (type_t(*)())0},
    {"Virtual SM test",            (PFT)phoenix_test,   TRUE,MM_2,(type_t(*)())0,
                                    0,    (PFT)phoenix_test, FALSE},
};

#define PHOENIX_MAINMENU_TBL_SIZE \
        (sizeof(phoenix_mainmenu_tbl) / sizeof(submenu_xtable_t))

/*
 * Primary & secondary submenu items (filled in from xtable)
 */
static mitem_t main_menu_primary_items[PHOENIX_MAINMENU_TBL_SIZE + MAX_BASE_ITEMS];
static mitem_t main_menu_secondary_items[PHOENIX_MAINMENU_TBL_SIZE + MAX_BASE_ITEMS];

static title_buf_t  maindiag_header;

char phoenixmainmenutitle[50];
char phoenixsubmenutitle[50];
static struct menuinfo phoenix_mainmenu = {
    /*"PHOENIX  Main Menu",      *//* title */
    phoenixmainmenutitle,          /* title */
    0,                          /* title string added by init_empty_menu */
    (PFT)menu_show_dflags,      /* shows major flags */
    0,                          /* generic prompt */
    0,                          /* size -- bumped by add_menu_item() */
    main_menu_primary_items,
};

static struct menuinfo *maindiagp = &phoenix_mainmenu;

static test_commands_t phoenix_command[] = {
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
    {SET_TOGGLE_SEP_MB_DBX_TEST_FLAG, "SET_TOGGLE_SEP_MB_DBX_TEST_FLAG"},
    {SELECT_TDM_CODEC_RST, "SELECT_TDM_CODEC_RST"},
    {SELECT_NULL,              "UNKNOWN_TEST"},       /* */
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
    offset = gethex_answer("\nEnter FPGA general register offset[0x00 to 0x380]:",
                  0,0,0x380);
    size = gethex_answer("Size: ", 0, 0, 0xf);
    if (phoenix_select_test(SELECT_READ_FPGA_DIR_REG, offset, size, 100) == FAILED) {
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
    offset = gethex_answer("\nEnter FPGA general register offset[0x00 to 0x380]:",
                  0,0,0x380);
    size  = gethex_answer("\nSize: ", 0, 0, 0xf);
    value = gethex_answer("\nValue: ", 0, 0, 0xffffffff);
    param = (offset << 4) | size;

    if (phoenix_select_test(SELECT_WRITE_FPGA_DIR_REG, param, value, 100) == FAILED) {
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
    addr = gethex_answer("Address: ", 0, 0x0, 0x8FFC);
    if (phoenix_select_test(SELECT_READ_FPGA_INDIR_REG, addr, 0, 100) == FAILED) {
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
    addr  = gethex_answer("Address: ", 0, 0x0, 0x8FFC);
    value = gethex_answer("\nValue: ", 0, 0, 0xffffffff);
    if (phoenix_select_test(SELECT_WRITE_FPGA_INDIR_REG, addr, value, 100) == FAILED) {
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


/**********************************************************************
 *
 * Function: phoenix_ngio_off
 *
 * This function power off Phoenix
 *
 * Input : None
 *
 * Output: None
 *  
 **********************************************************************
 */
static void phoenix_ngio_off (void)
{
    ngsm_entity_t *phoenix_ep;
    int dsp;

    assert(ngio_ptr);

    ngio_ptr->off(ngio_ptr);
    msleep(NGIO_WAIT_TIME);               /* HW need  */

    phoenix_ep = (ngsm_entity_t *)ngio_ptr->priv;

    for (dsp = 0; dsp < MAX_DSPS_PER_NGSM; dsp++) {
        phoenix_ep->dsp_downloaded[dsp] = FALSE;
    }

}

/**********************************************************************
 *
 * Function: phoenix_pwr_off
 *
 * This function reset the LSI SP27XX SM
 *
 * Input : None
 *
 * Output: PASSED/FAILED/ERROR
 *  
 **********************************************************************
 */
int phoenix_pwr_off (void)
{
    ngsm_entity_t *phoenix_ep;
    int dsp;

    assert (oir_if);

    printf ("\nPower Off the Virtual SM DSP.\n");
    phoenix_ep = (ngsm_entity_t *)ngio_ptr->priv;

    for (dsp = 0; dsp < MAX_DSPS_PER_NGSM; dsp++) {
        phoenix_ep->dsp_downloaded[dsp] = FALSE;
    }

    return (PASSED);

}

/**********************************************************************
 *
 * Function: phoenix_pwr_on
 *
 * This function unresets the LSI SP27XX SM
 *
 * Input : None
 *
 * Output: PASSED/FAILED/ERROR
 *  
 **********************************************************************
 */
int phoenix_pwr_on (void)
{

    printf("\nPower On the Virtual SM.\n");

    assert(ngio_ptr);
    assert(oir_if);

    /* turn on board power and take I2C out of reset */
    slot_i2c_unreset(ngio_ptr, ngio_ptr->slot, "SM");
    ngio_ptr->uart_on(ngio_ptr);    

    /* Take  NGSM out of reset */
    ngio_ptr->unreset(ngio_ptr);
    /* Enable the ngio_sync_out_enable   */
    ngio_sync_out_enable (ngsm_p, ENABLE_PRE_SCALER_DIV_3125 |   SYNC_OUT_ENABLE );

    printf("Virtual SM is powered up.\n");

    return (PASSED);

}

/**********************************************************************
 *
 * Function: phoenix_disp_mac
 *  
 *  
 * Description: Display Phoenix and Host MAC
 *
 * Input : None.
 *
 * Output: PASSED/FAILED   
 *  
 **********************************************************************
 */
static int phoenix_disp_mac (void)
{
    ngsm_entity_t *ep;

    assert(ngio_ptr);

    ep = (ngsm_entity_t *)ngio_ptr->priv;
    assert(ep);

    /* Display the Phoenix Virtual Nim details */
    printf("\n%s", ep->name);

    printf("\nPID: %s ", ep->pid);


    printf("\n Virtual SM MAC : 0x%02x:0x%02x:0x%02x:0x%02x:0x%02x:0x%02x ",
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
 * Function: phoenix_disp_fw_ver
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
static int phoenix_disp_fw_ver (void)
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
    if (phoenix_bringup_dsp() == FAILED) {
        cterr('f', 0, "%s(): \n DSP READY PIN not set. Please check if "
               "\"FIRMWARE_PATH\" exists.\n", __FUNCTION__);
        return (FAILED);
    }

    if (phoenix_setup_ge_env() == FAILED) {
        return (FAILED);
    }

    ret_val = phoenix_dsp_test(SELECT_FW_VER_DISP, 0, 0, 100);
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
 * Function: phoenix_disp_mem
 *  
 * Description: Display Sp2704 memory
 *
 * Input : None.
 *
 * Output: PASSED/FAILED   
 *  
 **********************************************************************
 */
static int phoenix_disp_mem (void)
{
    ngsm_entity_t *ep;
    dspif_mem_t   *mem_p;
    int ret_val, offset, len;
 
    /* Bringup DSP */
    if (phoenix_bringup_dsp() == FAILED) {
        cterr('f', 0, "%s(): \n DSP READY PIN not set. Please check if "
               "\"FIRMWARE_PATH\" exists.\n", __FUNCTION__);
        return (FAILED);
    }

    offset = gethex_answer("Memory Start Address: ", 0, 0, 0xffffFD7C);
    len = gethex_answer("Length : ", 0, 0, 644); /* 0x284 */

    if (phoenix_setup_ge_env() == FAILED) {
        return (FAILED);
    }

    ret_val = phoenix_dsp_test(SELECT_MEM_DISP, offset, len, 100);
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
    ngsm_entity_t *phoenix_ep;
    int dsp;

    printf(" LSI SP27XX reset ");

    phoenix_ngio_off();
    msleep(PHOENIX_OFF_TIME);   /* Hardware need time to off Virtual SM */ 
    phoenix_ep = (ngsm_entity_t *)ngio_ptr->priv;

    for (dsp = 0; dsp < MAX_DSPS_PER_NGSM; dsp++) {
        phoenix_ep->dsp_downloaded[dsp] = FALSE;
    }

    phoenix_pwr_on();
    msleep(PWR_WAIT_TIME);
    
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
    int ix = 0, jx, ngsm_slot = 0;
    uint8_t print_mac[6] = {0x00, 0x11, 0x22, 0x33, 0x44, 0x77};

    assert(ngio_ptr);
    ngsm_slot = ngio_ptr->slot;
    
    /* Need to store the MAC address of the Virtual SM and host */ 
    ep = (ngsm_entity_t *)ngio_ptr->priv;
    assert(ep);

    ep->num_dsp = NUM_DSP_PHOENIX;

    for (jx = 0; jx < 6; jx++) {
        ep->eth_hdr[ix].dest_addr[jx] = (uint8_t)print_mac[jx];

    }

    printf("\nVirtual SM module destination MAC : 0x%02x:0x%02x:0x%02x:0x%02x:0x%02x:0x%02x ",
           (uchar)ep->eth_hdr[ix].dest_addr[0], (uchar)ep->eth_hdr[ix].dest_addr[1],  
           (uchar)ep->eth_hdr[ix].dest_addr[2], (uchar)ep->eth_hdr[ix].dest_addr[3],  
           (uchar)ep->eth_hdr[ix].dest_addr[4], (uchar)ep->eth_hdr[ix].dest_addr[5]);
    
    local_mac_addrs_init();
    
    get_host_mac_addr(DSP_SLOT0_ETH_PORT, (uchar *)&src_mac[0]);

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
 *  Function: init_phoenix_ngio_priv_struct
 *
 *  Description: This function initializes the ngsm specific structures.
 *
 *  Input: None
 *
 *  Returns:  PASSED - otherwise
 *
 **********************************************************************
 */
static int init_phoenix_ngio_priv_struct (void)
{
    ngsm_entity_t *phoenix_ep;
    int dsp, ngsm_num, tgt_dev, i2c_dev, pslot;
    char slot_type_str[20], tty_dev[20];

    assert(ngio_ptr);
    
    ngsm_num = ngio_ptr->slot;

    sprintf(slot_type_str, "Virtual SM");

    tgt_dev = TGT_DEV_NGSM;

    sprintf(tty_dev, "/dev/ttyDASH%d", ngio_ptr->uart_ctrl);
    i2c_dev = SM_I2C_ADDR_IO_PORT1;
    pslot = ngio_ptr->slot;

    testname(slot_type_str);
    /* point to Phoenix structure */
    ngio_ptr->priv = (void *) &phoenix_lsi_entity[pslot-1];

    /* Init parameters for I2C access to the IO expander */
    pca_init_i2c((void *)&pca_i2c[0]);
    pca_i2c[0].i2c_ctrl = ngio_ptr->i2c_ctrl;
    pca_i2c[0].i2c_dev = i2c_dev;
    pca_i2c[0].buf = &pca_buff[0];
    ngio_ptr->pca = (void *) &pca_i2c[0];

    oir_if = (void *)(ngio_ptr->oir);

    /* Init Phoenix struct parameters */
    phoenix_ep = (ngsm_entity_t *) ngio_ptr->priv; 
    /* Will populate id after firmware download */
    phoenix_ep->cookie_id = ngio_ptr->id; 
    phoenix_ep->ngsm_id = PFUSE123_UNKNOWN; 
    phoenix_ep->plat_ngsm_num = ngsm_num; 
    phoenix_ep->ge_tgt_dev = tgt_dev; 
    phoenix_ep->pslot = pslot; 
    sprintf(phoenix_ep->tty_dev, "%s", tty_dev);
    sprintf(phoenix_ep->slot_type_str, "%s", slot_type_str);

    sprintf(phoenix_ep->name, "PHOENIX LSI SP27XX, board_id %#x, "
              "%s slot %d", ngio_ptr->id, slot_type_str, 
              ngio_ptr->slot);

    for (dsp = 0; dsp < MAX_DSPS_PER_NGSM; dsp++) {
        phoenix_ep->dsp_downloaded[dsp] = FALSE;
    }
    return (PASSED);
}

/*
 **********************************************************************
 *
 *  Function: phoenix_init_fp
 *
 *  Description: This function initialize callout functions
 *
 *  Input: None
 *
 *  Returns: None 
 *
 **********************************************************************
 */
static void phoenix_init_fp (void)
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
 * Function: phoenix_cleanup
 *
 * Description: This function performs the cleanup task before exiting
 *              the test.
 *
 * Input:  None
 *
 * Output: None
 **********************************************************************
 */
static void phoenix_cleanup (void)
{
    assert(ngio_ptr);

    if (phoenix_saved_diag_exec) {
        pre_diag_exec = phoenix_saved_diag_exec;
        phoenix_saved_diag_exec = NULL;
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
static int phoenix_uart_test (void)
{
    ngsm_entity_t *ngsm_ep;
    int ret = PASSED;
    int port, dsp;
    char *tx_str = "abcdefgh";
    int tx_len = strlen(tx_str);
    int rx_sz;
    char rx_str[64];

    prpass(testpass, "Uart Loopback - ");
    assert(ngio_ptr);

    ngsm_ep = (ngsm_entity_t *)ngio_ptr->priv;
    assert(ngsm_ep);
    /* Run uart loopback test when the DSP diag application is booted */
    /* Wait dsp ready  message. Set 100 sec for time out.*/
    phoenix_select_test(SELECT_UART_TEST, 0, 0, 100);  
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
        //Do not reset if test fails phoenix_reset_en();
        cterr('f', 0, "%s(): uart_lpbk: failed to tx and rx.\
                Please reset/unreset Virtual Nim. ", __FUNCTION__);
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
    if (phoenix_bringup_dsp() == FAILED) {
        cterr('f', 0, "%s(): \n DSP READY PIN not set. Please check if "
               "\"/firmware/dsp_sp2700_fw.img\" exists.\n", __FUNCTION__);
        return (FAILED);
    }

    return (ret);

}

/**********************************************************************
 * Function: phoenix_gpio_resistor_test
 *
 * Description: This function test the resistor circuitry for the GPIO 
 *
 * Input:  None
 *
 * Output: PASSED/FAILED
 **********************************************************************
 */
int phoenix_gpio_resistor_test (void)
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
    phoenix_pwr_off();

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
 * Function: phoenix_gpio_exp_test
 *
 * Description: Test the output pins 1 and 7 of the GPIO Expander 
 *              Register.
 *
 * Input:  None
 *
 * Output: PASSED/FAILED
 **********************************************************************
 */
static int phoenix_gpio_exp_test (void)
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
 *  Function: phoenix_main_test
 *
 *  Description: This is the test entry point for Phoenix.
 *
 *  Input: none
 *  
 *  Returns: PASSED if successful; 
 *           FAILED, otherwise
 *  
 **********************************************************************
 */
static int phoenix_main_test (void)
{
    int  retval = PASSED;

    /* Will test the Host-Side FPGA Reg, Interrupts, Uart, GDF Int lpbk,
       pcie link */

    if (retval == PASSED) {
        retval = phoenix_gpio_exp_test();
    }

    if (retval == PASSED) {
        retval = phoenix_test(TRUE);
    }

    return (retval);
}


/*
 **********************************************************************
 *
 *  Function: phoenix_bringup_dsp
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
static int phoenix_bringup_dsp (void)
{
    n2g_i2c_if_t  *pca;
    ngsm_entity_t *ngsm_ep;
    int i2c_dev, ix;
    uchar  data = 0;

    assert(ngio_ptr);

    /* check if DSP is already booted  only one dsp on Phoenix */
    ngsm_ep = (ngsm_entity_t *)ngio_ptr->priv;
    assert(ngsm_ep);

    if (ngsm_ep->dsp_downloaded[0] == TRUE) {
        return (PASSED);
    }
    /* reset PVDM */
    ngsm_ep->reset_ngsm();

    printf("\r Please wait for the Virtual SM Bootloader to dhcp the "
           "firmware");
    fflush(stdout);

    pca = (n2g_i2c_if_t *)ngio_ptr->pca;
    i2c_dev = pca->i2c_dev; 
    pca->i2c_dev = SM_I2C_ADDR_IO_PORT1;

    /* Increase time for DSP boot up */
    for (ix = 0; ix < DSP_BOOT_TIME; ix++) {
        if (io_port_8bit_i2c_read(pca, INPUT_PORT_REG, &data, TRUE) == FAILED) {
            cterr('f', 0, "Unable to read PCA9557 register @ %#x\n", 
                  INPUT_PORT_REG);
            return (FAILED);
        }

        if (data & PRIM_INTF_READY) {
            printf("\r GPIO Expander bit 3(READY Bit) Set by Virtual SM");
            break;
        } else if (ix == DSP_TIME_OUT) {
            printf("\n*** %s(): GPIO Expander bit 3 not set by DSP\n"
                   "Virtual SM DSP not booted up, Tests will fail.", __FUNCTION__);
            return (FAILED);
        } 

        switch (ix % 8) {
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
        msleep(DSP_WAIT_TIME);
 
    }

    pca->i2c_dev = i2c_dev;
    msleep(DSP_WAIT_TIME); /* Wait 1000ms for DSP ready. */
    printf("Send Ready Test to DSP\n");
    if ((phoenix_ready_test(0)) == PASSED)  {
        ngsm_ep->dsp_downloaded[0] = TRUE;
    } else {
        printf("\n %s(): phoenix_ready_test() failed\n", __FUNCTION__);
        return (FAILED);
    }

    return (PASSED);

}



/*******************************************************************************
 *
 * Function   : phoenix_vir_setup_hw_brd_type
 * Description: Function to send hardward board strap type to DSP
 * Inputs     : None
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
static int phoenix_vir_setup_hw_brd_type (void)
{
    uint reg_addr = FPGA_VERTYPE;
    uint fpga_ver = 0;
    uint hw_brd_type = 0;
    int result;

    if (fpga_read_reg(reg_addr, &fpga_ver) != PASSED) {
        printf("Failed to read FPGA LPC Main FPGA Version Reg(0x%04X).\n", reg_addr);
        return (FAILED);
    }

    hw_brd_type = HW_BRD_TYPE_STRAP(fpga_ver);

    /* send hardware board strape type info to DSP */
    result = phoenix_select_test(SELECT_HW_BRD_TYPE_FLAG,
                              hw_brd_type, 0, 100);
    if (result == FAILED) {
       printf("ERROR: Set host_hw_brd_type_flag to DSP failed\n");
       return (FAILED);
    }

    return (PASSED);
}
/*
 **********************************************************************
 *
 *  Function: phoenix_vir_test
 *
 *  Description: This is the test entry point for Phoenix test
 *
 *  Input: ngio interface struct *
 *  
 *  Returns: PASSED if successful; 
 *           FAILED, otherwise
 *
 **********************************************************************
 */ 
int phoenix_vir_test (void *sm)
{
    ngsm_entity_t *ngsm_ep;
    int ret_val, ngsm_num;
    ngsm_p = sm;

    assert(sm);
    ngio_ptr = (struct ngio_intf_t *)sm;
    phoenix_board_id = ngio_ptr->id;

    init_phoenix_ngio_priv_struct();

    /* initialize Phoenix internal entities for operations */
    phoenix_init_fp();
    ngio_ptr->uart_on(ngio_ptr);

    assert(ngio_ptr->priv);
    ngsm_ep = (ngsm_entity_t *)ngio_ptr->priv;

    /* Display the Phoenix Virtuak SM details */
    printf("\r %s\n", ngsm_ep->name);

    ngsm_num = ngsm_ep->ngsm_num;

    /* The actual number if all the slots(onboard, NGIOs)  
       were populated with Virtual Nim */
    printf("\n VIRTUAL_SM_NUM = %d\n", ngsm_num);


    if (tftp_get(0, (unsigned char *)TFTP_IMG_NAME,
                 0, (unsigned char *)IMG_PATH, 1) < 0) {
        cterr('f', 0, "Failed to tftp download firmware to local host");
        return(FAILED);

    }
    sprintf(maindiag_header.title, "%s", ngsm_ep->pid);
    prpass(testpass, " PID: %s ", ngsm_ep->pid);

    if (phoenix_pwr_on() == FAILED) {
        cterr('f', '0', " Power on failed\n");
        return (FAILED);
    }
    
    /* Init the NGSM module specific parameters mac etc */
    if (ngsm_ep->init_ngsm()) {
        return (FAILED);
    }

    /* Enable the ngio_sync_out_enable   */
    ngio_sync_out_enable (sm, ENABLE_PRE_SCALER_DIV_3125 |   SYNC_OUT_ENABLE );

    phoenix_saved_diag_exec = pre_diag_exec;
    pre_diag_exec = NULL;
    sprintf(phoenixmainmenutitle, "Virtual SM Main Menu");
    sprintf(phoenixsubmenutitle, "Virtual SM");
    
    build_primary_submenu(phoenix_mainmenu_tbl, PHOENIX_MAINMENU_TBL_SIZE, 
                          "Virtual SM Menu", &maindiagp);
    build_secondary_submenu(phoenix_mainmenu_tbl,PHOENIX_MAINMENU_TBL_SIZE,
                            main_menu_secondary_items);

    if (ngio_ptr->menu_display) {
        menu(&phoenix_mainmenu, main_menu_secondary_items, '\0');
        pre_diag_exec = phoenix_saved_diag_exec;
        ret_val = PASSED;
    } else {
        ret_val = phoenix_main_test();
    }

    phoenix_cleanup();
    return (ret_val);

}

/**********************************************************************
 *
 * Function: phoenix_ready_test 
 * This function test to make sure the Phoenix is ready to accept commands
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
static int phoenix_ready_test (int dsp_no)
{
    ngsm_entity_t *ep; 
    fe_packet_t   *recv_packet_p;
    dspif_ready_t *ready_msg_p;
    dspif_ready_t ready_msg;
    int retval = PASSED;
    
    printf("Send Ready Packet to Virtual SM\n");

    assert(ngio_ptr);

    ep = (ngsm_entity_t *)ngio_ptr->priv;
    assert(ep);

    recv_packet_p = &(ep->recv_packet);
    ready_msg_p = &ready_msg;

    printf("Set up GE env\n");
    if (phoenix_setup_ge_env() == FAILED) {
        return (FAILED);
    }
    
    phoenix_clear_rx_buf((uchar *)ready_msg_p, (int)sizeof(dspif_ready_t));
    phoenix_clear_rx_buf((uchar *)recv_packet_p, (int)sizeof(fe_packet_t));
    
    printf("Send Ready Command to DSP\n");
    phoenix_build_command_packet(SELECT_READY, 0, 0, DSS_CORE0);
    phoenix_build_config_packet();
    /* need to know which DSP on the PVDM */
    if ((retval = phoenix_send_command_packet(dsp_no)) != PASSED) {
        cterr('f', 0, "\n %s(): phoenix_send_command_packet() returned tx error "
              "%d\n", __FUNCTION__, retval);
        return (FAILED);
    }
    /* wait for result packet */
    printf("Wait for DSP feedback\n");
    if (phoenix_wait_result_packet((uint8_t *)recv_packet_p, ep->socket_gl, 10,
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

    if (phoenix_cleanup_ge_env() == FAILED) {
        cterr('f', 0, "%s(): phoenix_cleanup_ge_env() failed", __FUNCTION__);
        return (FAILED);
    }

    ep->ngsm_id = ready_msg_p->dsp_device.core_id;

    if (ready_msg_p->dsp_device.core_id == PFUSE123_SP2704) {
        printf("\r Received Ready Response from Virtual SM SP2704     ");
    } else {
        printf("\r Received Ready Response from Virtual SM (unknown SP27XX) ");
    }
    return (retval);

}

/***********************************************************************
 * Name: phoenix_setup_ge_env (common)
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
static int phoenix_setup_ge_env (void)
{
    ngsm_entity_t *ep;
    int status = PASSED;
    int sgmii_port = 0, ngsm_slot = 0;
    char if_name[IFNAMSIZ];
    
    assert(ngio_ptr);
    ep = (ngsm_entity_t *) ngio_ptr->priv;
    assert(ep);

    if (ep->ge_setup_flag == TRUE) {
        /* Linux socket already setup return */
        return (PASSED);
    }
    ngsm_slot = ngio_ptr->slot;

    sgmii_port = get_sgmii_port_num(ngsm_slot, TYPE_SWITCH);
    sprintf(if_name, "eth%d", sgmii_port);
    status = setup_eth_dev(if_name, &(ep->socket_gl));

    if (status) {
        cterr('f', 0, "Setup: Failed, status = 0x%x", status);
        return (FAILED);
    }

    ep->ge_setup_flag = TRUE;

    return (PASSED);
}


/***********************************************************************
 * Name: phoenix_cleanup_ge_env (common)
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
int phoenix_cleanup_ge_env (void)
{
    ngsm_entity_t *ep;
    int status = PASSED;
    int sgmii_port = 0, ngsm_slot = 0;
    char if_name[IFNAMSIZ];

    assert(ngio_ptr);
    ep = (ngsm_entity_t *)ngio_ptr->priv;
    assert(ep);

    if (ep->ge_setup_flag == TRUE) {
        ep->ge_setup_flag = FALSE;

        ngsm_slot = ngio_ptr->slot; 

	    sgmii_port = get_sgmii_port_num(ngsm_slot, TYPE_SWITCH);
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
  * Name: phoenix_clear_rx_buf (common)
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
void phoenix_clear_rx_buf (uchar *c_ptr, int num_bytes)
{
    int ix;

    for (ix = 0; ix < num_bytes; ix++) {
        *c_ptr++ = 0;
    }
}

/*
 **********************************************************************
 *
 *  Function: phoenix_build_config_packet
 *
 *  Description: PID info etc 
 *
 *  Input: None 
 *
 *  Returns: None
 *
 **********************************************************************
 */
void phoenix_build_config_packet (void)
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
    /* 0 - phoenix_dsp_tests_use_enet : 1= enet intf 0= uart intf
     * 1 - menu_display   : If uart intf, display menu [1] or run the tests [0]
     * 2 - module_type  : 6= SM_MODULE etc
     * 3 - parent slot  : applicable if DC
     */
    cmd_packet_p->dspif_info.errmsg[3] = ngio_ptr->slot;
    cmd_packet_p->dspif_info.errmsg[2] = ngio_ptr->mod_type;
    cmd_packet_p->dspif_info.errmsg[0] = phoenix_dsp_tests_use_enet; //eth or uart interface
    cmd_packet_p->dspif_info.errmsg[1] = ngio_ptr->menu_display; /*if uart then run tests or just display the menu */
    phoenix_dsp_tests_use_enet = cmd_packet_p->dspif_info.errmsg[0]; 
}

/*
 **********************************************************************
 *
 *  Function: phoenix_build_command_packet
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
void phoenix_build_command_packet (uint16_t select_test, uint32_t param1, 
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

    memset((cmd_packet_p->dspif_info.bufmsg), 0, 128);
    memset((cmd_packet_p->dspif_info.errmsg), 0, 128);
}

/*
 **********************************************************************
 *
 *  Function: phoenix_send_command_packet (not common specific)
 *
 *  Description: send test command
 *
 *  Input: dsp_no - DSP number
 *
 *  Returns: PASSED
 *           FAILED - some tx error
 *
 **********************************************************************
 */
int phoenix_send_command_packet (int dsp_no)
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

    ret_val = eth_pkt_tx(tx_pkt_p);

    return (ret_val);
}

/*
 **********************************************************************
 *
 *  Function: phoenix_sgmii_dsp_lpbk_test
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
int phoenix_sgmii_dsp_lpbk_test (int dsp_no, int port_num)
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
        if (phoenix_eth_frames_test(frame_num, frm_size, ep->socket_gl,
                                mac_da, dsp_no, port_num)) {
            retval = (FAILED);
            break;
        }
    }

    return (retval);
}

/*
 **********************************************************************
 *
 *  Function: phoenix_build_lpbk_frame
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
int phoenix_build_lpbk_frame (dspif_lpbk_t *frame_ptr, mac_addr_t dst_mac_addr,
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
 *  Function: phoenix_check_rx_frame
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
int phoenix_check_rx_frame (volatile dspif_lpbk_t * test_frame_p, 
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

#ifdef DEBUG_RE_PACKET
                printf("\n cterr will force stop DSP loopback test. \n");
                cterr('f', 0, "Packet%d data mismatch at offset %#x, "
                      "sent %#.8x, rcvd %#.8x\ntx bd @%#.8x, rx bd @%#.8x",
                      packet_num, count, *wr_ptr, *rd_ptr, test_frame_p, recv_frame_p);
#endif
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
 *  Function: phoenix_eth_frames_test
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
int phoenix_eth_frames_test (uint32 frame_num, uint32 frm_size, int socket_gl, 
                           mac_addr_t dst_mac_addr, int dsp, int port)
{
    mac_addr_t   src_mac_addr;
    dspif_lpbk_t test_frame;
    dspif_lpbk_t *test_frame_p = &test_frame;
    eth_tx_pkt_t tx_pkt;
    eth_tx_pkt_t *tx_pkt_p = &tx_pkt;
    int   result = PASSED, wait_time = 0;
    uchar recv_frame_p[2048];
    char  base_val, inc_val;
    int gar_pkt;

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
    if (phoenix_build_lpbk_frame(test_frame_p, dst_mac_addr, frm_size,
                        base_val, inc_val) == FAILED) {
        return (FAILED);
    }
    
    memset((uchar *)tx_pkt_p, 0, sizeof(eth_tx_pkt_t));
    memcpy((uchar *)(tx_pkt_p->dest_addr), "FFFFFFFFFFFF", 
           sizeof(mac_addr_t));

    get_host_mac_addr(DSP_SLOT0_ETH_PORT, (uchar *)&src_mac_addr[0]); 
    memset((uchar *)(tx_pkt_p->src_addr), 0, sizeof(mac_addr_t)); /* host MAC */
    memcpy((uchar *)(tx_pkt_p->src_addr), src_mac_addr, sizeof(mac_addr_t)); /* host MAC */

    tx_pkt_p->pkt_type = PKT_TYPE_IPV4;
 
    tx_pkt_p->payload_size = frm_size - sizeof(ether_hdr_t); /* payload size */
    tx_pkt_p->bufr_st_addr = (uchar *)&(test_frame_p->data); /* payload */
    tx_pkt_p->pkt_num = 0;
    tx_pkt_p->socket = socket_gl;


    msleep(DSP_READY_TIME); /* Wait till DSP is ready for the loopback frames */
    msleep(DSP_READY_TIME); /* Wait till DSP is ready for the loopback frames */

    prpass(testpass, "Sending lpbk frame#%d, size = %d ... ", frame_num, frm_size);
    result = eth_pkt_tx(tx_pkt_p);
    if (result != ETH_PKT_TX_OK ) {
        cterr('f', 0, "%s: Failed to TX lpbk Frame#%d, size = %d : ret = 0x%x,"
              " status = 0x%x", __FUNCTION__, frame_num, frm_size, result, 
              tx_pkt_p->tx_status);
        return (FAILED);
    }

    wait_time = 1000 * 60;
    gar_pkt = 0;
    prpass(testpass, "Waiting for lpbk response for frame#%d ", frame_num);

    while (wait_time > 0) {
        /* Check receive correct GE packet */
        result = phoenix_wait_for_ge_packet(recv_frame_p, socket_gl, INTR_MODE, dsp, wait_time);
        if (result == PASSED) {
            result = phoenix_check_rx_frame (test_frame_p, recv_frame_p, frame_num,frm_size);
            if (result == PASSED) {
                 prpass(testpass, "RX frame #%d matches for size %d ", frame_num, frm_size);
                 break;
            } else if (gar_pkt >= PHOENIX_DSP_LB_GAR_PACKET_ALLOW) {
                cterr('f', 0, "Packet%d data mismatch over allow.\n", frame_num);
                break;
            }
            gar_pkt++;
            printf("\n %s:%d Receive garbage %d packet.\n", __FUNCTION__, __LINE__, gar_pkt);
        }
        wait_time--;
        msleep(1);
    }
    if (wait_time <= 0) {
        cterr('f', 0, "Failed to RX lpbk Frame#%d, size = %d \n", frame_num, frm_size);
        return (FAILED);
    }

    printf("\r                                                               ");
    return (result);
}

/*
 **********************************************************************
 *
 *  Function: phoenix_build_stop_command_packet
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
void phoenix_build_stop_command_packet (uint16_t select_test, uint8_t core_id)
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
 *  Function: phoenix_wait_result_packet
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
int phoenix_wait_result_packet (uint8_t *pak, int socket_gl, uint16_t retry, int dsp)
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
        if ((phoenix_wait_for_ge_packet(pak, socket_gl, INTR_MODE, dsp, count)) == PASSED) {
            break;
        }
        count--;
        msleep(PKG_WAIT_TIME);
    }

    if (count == 0) {
        ret_val = FAILED;
    }

    return (ret_val);
}

/*
 **********************************************************************
 *
 *  Function: phoenix_ge_lpbk_test
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
int phoenix_ge_lpbk_test (int local_port)
{
    ngsm_entity_t *ep;
    fe_packet_t   *recv_packet_p;
    dspif_ether_t *result_packet_p;
    int retval = PASSED;

    prpass(testpass, "Host <--> Virtual SM GE%d Ext Loopback test", local_port);

    assert(ngio_ptr);

    ep = (ngsm_entity_t *)ngio_ptr->priv;
    assert(ep);

    recv_packet_p = &(ep->recv_packet);
    result_packet_p = &(ep->result_packet);

    /* SR add rx check for mac as well as slot id */
    phoenix_clear_rx_buf((uchar *)result_packet_p, (int)sizeof(dspif_ether_t));
    phoenix_clear_rx_buf((uchar *)recv_packet_p, (int)sizeof(fe_packet_t));
    /*
     * build command packet to run dsp sanity test
     * test result will be handle in processing receiving packets
     */
    /* DSS_CORE0 does not matter here */
    phoenix_build_command_packet(SELECT_GE0_LPBK, 0, 0, DSS_CORE0);
    
    /* need to know which DSP on the PVDM */
    if ((retval = phoenix_send_command_packet(0)) != PASSED) {
        printf("\n %s(): phoenix_send_command_packet() returned tx error %d\n",
               __FUNCTION__, retval);
        return (FAILED);
    }
    /* allow DSP to set up loopback connection */
    msleep(DSP_SETUP_TIME);

    retval = phoenix_sgmii_dsp_lpbk_test(0, local_port);
    if (retval != PASSED) {
        cterr('f', 0, "phoenix_sgmii_dsp_lpbk_test() DSP loopback test failed retval = %#x",
               retval);
        return (FAILED);
    }
    phoenix_build_stop_command_packet(SELECT_GE0_LPBK, DSS_CORE0);

    if ((retval = phoenix_send_command_packet(0)) != PASSED) {
        printf("\n %s(): phoenix_send_command_packet() returned tx error %d\n",
               __FUNCTION__, retval);
        return (FAILED);
    }

    prpass(testpass, " Sent STOP GE %d Loopback command packet ", local_port);

    if (phoenix_wait_result_packet((uint8_t *)recv_packet_p, ep->socket_gl, 20, 0)) {
        cterr('f', 0, "Timed out waiting for GE loopback test result.");
        return (FAILED);
    }
    
    prpass(testpass, " Host <--> Virtual SM GE %d Loopback completed ", local_port);
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
        exit (0);
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
    int retval, ngsm_slot = 0;

    ep = (ngsm_entity_t *)ngio_ptr->priv;
    ngsm_slot = ngio_ptr->slot;
    get_host_mac_addr(DSP_SLOT0_ETH_PORT, (uchar *)&src_mac_addr[0]);

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
    int ix, ngsm_slot = 0;
    mac_addr_t   src_mac_addr;
    uchar recv_buffer[1600] = {0};
    /* Because packet is 1000 bytes and first bytes is type so need 1001.*/
    uchar tx_buffer[1001] = {0};
    int timeout = TIME_OUT, tx_fpga_data_size;
    unsigned int fpga_image_size = 0x200000;
    unsigned int fpga_data_index = 0;

    phoenix_setup_ge_env();

    ep = (ngsm_entity_t *)ngio_ptr->priv;
    ngsm_slot = ngio_ptr->slot;
    get_host_mac_addr(DSP_SLOT0_ETH_PORT, (uchar *)&src_mac_addr[0]);

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

        if (fpga_image_size >= DSP_FPGA_SIZE) {
            tx_fpga_data_size = DSP_FPGA_SIZE;
            fpga_image_size -= DSP_FPGA_SIZE;
        } else {
            tx_fpga_data_size = fpga_image_size;
            fpga_image_size = 0;
        }

        tx_buffer[0] = PACKET_TO_DSP;
        for (ix = 0; ix < tx_fpga_data_size; ix++) {
            tx_buffer[ix + 1] = fpga_image[fpga_data_index];
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

    phoenix_con_utils();
    pthread_cancel(threads);
    printf("Remove the thread!\n");
    return (PASSED);
}


/*
 **********************************************************************
 *
 *  Function: phoenix_dac_1dot5sm_high
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
static int phoenix_dac_1dot5sm_high (void)
{
    printf("1.5 Voltage Margin High\n");
    return (phoenix_select_test(SELECT_DAC_1DOT5SM_HIGH, 0, 0, 100));
}

/*
 **********************************************************************
 *
 *  Function: phoenix_dac_1dot5sm_low
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
static int phoenix_dac_1dot5sm_low (void)
{
    printf("1.5 Voltage Margin Low\n");
    return (phoenix_select_test(SELECT_DAC_1DOT5SM_LOW, 0, 0, 100));
}

/*
 **********************************************************************
 *
 *  Function: phoenix_dac_no_1dot5sm
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
static int phoenix_dac_no_1dot5sm (void)
{
    printf("1.5 No Voltage Margin\n");
    return (phoenix_select_test(SELECT_DAC_NO_1DOT5SM, 0, 0, 100));
}

/*
 **********************************************************************
 *
 *  Function: phoenix_dac_3dot3sm_high
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
static int phoenix_dac_3dot3sm_high (void)
{
    printf("3.3 Voltage Margin High\n");
    return (phoenix_select_test(SELECT_DAC_3DOT3SM_HIGH, 0, 0, 100));
}

/*
 **********************************************************************
 *
 *  Function: phoenix_dac_3dot3sm_low
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
static int phoenix_dac_3dot3sm_low (void)
{
    printf("3.3 Voltage Margin Low\n");
    return (phoenix_select_test(SELECT_DAC_3DOT3SM_LOW, 0, 0, 100));
}

/*
 **********************************************************************
 *
 *  Function: phoenix_dac_no_3dot3sm
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
static int phoenix_dac_no_3dot3sm (void)
{
    printf("3.3 No Voltage Margin\n");
    return (phoenix_select_test(SELECT_DAC_NO_3DOT3SM, 0, 0, 100));
}


/*
 **********************************************************************
 *
 *  Function: phoenix_dac_1DOT5_show
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
static int phoenix_dac_1DOT5_show (void)
{
    ngsm_entity_t *ep_p;
    fe_packet_t   *recv_packet_p;
    dspif_ether_t *result_packet_p;
    uint8_t bufmsg[128];
    assert(ngio_ptr);

    ep_p = (ngsm_entity_t *)ngio_ptr->priv;

    assert(ep_p);

    printf("Show Voltage Margin\n");

    if (phoenix_select_test(SELECT_DAC_1DOT5_SHOW, 0, 0, 100) == FAILED) {
        return (FAILED);
    }
    recv_packet_p = &(ep_p->recv_packet);
    result_packet_p = &(ep_p->result_packet);

    memcpy((char *)result_packet_p, (char *)(&(recv_packet_p->data[0]) + 0),
          sizeof(dspif_ether_t));

    memcpy(bufmsg, result_packet_p->dspif_info.bufmsg, sizeof(bufmsg));

    printf("%s",bufmsg);

    return (PASSED);

}


/*
 **********************************************************************
 *
 *  Function: phoenix_dac_3DOT3_show
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
static int phoenix_dac_3DOT3_show (void)
{
    ngsm_entity_t *ep_p;
    fe_packet_t   *recv_packet_p;
    dspif_ether_t *result_packet_p;
    uint8_t bufmsg[128];
    assert(ngio_ptr);

    ep_p = (ngsm_entity_t *)ngio_ptr->priv;

    assert(ep_p);

    printf("Show Voltage Margin\n");

    if (phoenix_select_test(SELECT_DAC_3DOT3_SHOW, 0, 0, 100) == FAILED) {

        return(FAILED);
    }
    recv_packet_p = &(ep_p->recv_packet);
    result_packet_p = &(ep_p->result_packet);

    memcpy((char *)result_packet_p, (char *)(&(recv_packet_p->data[0]) + 0),
          sizeof(dspif_ether_t));

    memcpy(bufmsg, result_packet_p->dspif_info.bufmsg, sizeof(bufmsg));

    printf("%s",bufmsg);

    return (PASSED);

}

/*
 **********************************************************************
 *
 *  Function: phoenix_dsp_ddr3_sdram_test_wrapper
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
static int phoenix_dsp_ddr3_sdram_test_wrapper (void)
{
    prpass(testpass, "DDR3 Memory - ");
    return (phoenix_select_test(SELECT_DSP_SDRAM, 0, 0, 800));

}


/*
 **********************************************************************
 *
 *  Function: phoenix_dsp_debug
 *
 *  Description: select case to use dsp debug
 *
 *  Input: test , wait time 
 *
 *  Returns: PASSED if successful; 
 *           FAILED, otherwise
 *
 **********************************************************************
 */
int phoenix_dsp_debug (int test, int wait_time)
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
    ret_val = phoenix_dsp_test(SELECT_MEM_DISP, param1, param2, wait_time);
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

    phoenix_cleanup_ge_env();

    return (PASSED);
}

/*
 **********************************************************************
 *
 *  Function: phoenix_get_testcommand_id
 *
 *  Description: get the index of the test command
 *
 *  Input: SELECT command id 
 *  
 *  Returns: index in teh phoenix_command table
 *
 **********************************************************************
 */
int phoenix_get_testcommand_id (int test)
{
    int ix;

    for (ix = 0; ix < sizeof(phoenix_command); ix++) {
        if (phoenix_command[ix].test_id == test)
            return ix;
   }
   return (ix);
}

/*
 **********************************************************************
 *
 *  Function: phoenix_dsp_test
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
int phoenix_dsp_test (int test, int param0, int param1, int wait_time)
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
        phoenix_clear_rx_buf((uchar *)result_packet_p, 
                           (int)sizeof(dspif_ether_t));
        phoenix_clear_rx_buf((uchar *)recv_packet_p, 
                           (int)sizeof(fe_packet_t));
        /* 
         * build command packet to run dsp sanity test 
         * test result will be handle in processing receiving packets
         */
        phoenix_build_command_packet(test, param0, param1, DSS_CORE0);
        /* need to know which DSP on the PVDM */
        if ((retval = phoenix_send_command_packet(dsp)) != PASSED) {
            cterr('f', 0,"%s(): phoenix_send_command_packet() returned tx "
                  "error %d for test %d\n", __FUNCTION__, retval, test);
            return (FAILED);
        }
        if (wait_time == 0) { /* do not need reply */
            return (PASSED);
        }
        /* wait for result packet */
        if (phoenix_wait_result_packet((uint8_t *)recv_packet_p, ep->socket_gl, 
                                     wait_time, dsp)) {
            i = phoenix_get_testcommand_id(test);
            /* DSP probably not responding */
            cterr('f', 0, "%s(): Timed out waiting for DSP test(0x%x):%s result."
                  " Waiting for %d secs", __FUNCTION__, test, phoenix_command[i].test_name, (wait_time));
            return (FAILED);
        }
        memcpy((char *)result_packet_p, (char *)(&(recv_packet_p->data[0]) + 0),
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
 *  Function: phoenix_select_test
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
static int phoenix_select_test (int test, int param1, int param2, int wait_time)
{
    int ret_val = PASSED;

    assert(ngio_ptr);

    /* Bringup DSP */
    if (phoenix_bringup_dsp() == FAILED) {
        cterr('f', 0, "%s(): \n DSP READY PIN not set. Please check if "
               "\"/firmware/dsp_sp2700_fw.img\" exists.\n", __FUNCTION__);
        return (FAILED);
    }

    if (phoenix_setup_ge_env() == FAILED) {
        return (FAILED);
    }
    
    ret_val = phoenix_dsp_test(test, param1, param2, wait_time);

    if (ret_val == RESULT_FAILED) {
        /* Please get, display register/memory dump to debug failure */
        phoenix_dsp_debug(test, wait_time);
        ret_val = FAILED;
    }

    phoenix_cleanup_ge_env();
    return (ret_val);

}

/*
 **********************************************************************
 *
 *  Function: phoenix_tdm_ext_lpbk_test_wrapper
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
static int phoenix_tdm_ext_lpbk_test_wrapper (void)
{ 
#ifdef ENHANCED_ERR_MSG_EXAMPLE
    uchar phoenix_get_pid[FRU_SIZE] = {0};
    uchar phoenix_get_loc[FRU_SIZE] = {0};
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

    sprintf(fru_board_id , "%x", phoenix_board_id);
    memcpy(phoenix_get_pid,(char*)&fru_board_id,5); 
    strcpy((char *)phoenix_get_loc, "TDM External Loopback test");

    platform_fru_table[fru_table_offset].pid_string = phoenix_get_pid ; 
    platform_fru_table[fru_table_offset].location_string = phoenix_get_loc;
 
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
 
    if(phoenix_select_test(SELECT_TDM_EXTLPBK, 0, 0, 240) == FAILED) {
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

    return (phoenix_select_test(SELECT_ECC_MEM, 0, 0, 100));
}




/**********************************************************************
 *
 * Function: phoenix_tx_uart
 *
 * Description: This function transmits strings into tty
 * 
 * Input :  tty_dev: device string, (ie /dev/ttyDASH0, .../dev/ttyDASH7)
 *          out_str: compared string
 *
 * Output:  PASSED/FAILED
 *
 *********************************************************************
*/
int phoenix_tx_uart (char *tty_dev, char *out_str)    
{
    int uart_fd, cnt;
    int rc = PASSED;

    /* Sanity check */
    if (tty_dev == NULL || out_str == NULL) {
        printf("%s: Null pointer\n", __FUNCTION__);
        return (FAILED);
    }

    uart_fd = open(tty_dev, O_WRONLY);

    if (uart_fd < 0) {
        perror("polling uart: open tty failed");
        fflush(stdout);
        return (FAILED);
    }

    cnt = write(uart_fd, out_str, strlen(out_str));

    if (cnt < 0) {
        perror("tx_uart: write failed\n");
        rc = FAILED;
    }

    close(uart_fd);
    return (rc);
}



/**********************************************************************
 *
 * Function: phoenix_rx_polling_uart
 *
 * Description: This function reads data from uart controller, and return
 *              pass if the input string is found. If the string can't be
 *              found after timeout, then return failure.
 * 
 * Input :  tty_dev: device string, (ie /dev/ttyDASH0, .../dev/ttyDASH7)
 *          comp_str: compared string
 *          timeout: timeout value (ms)
 *
 * Output:  PASSED/FAILED
 *
 *********************************************************************
*/
int phoenix_rx_polling_uart (char *tty_dev, char *comp_str, int timeout)
{
    int uart_fd, cnt;
    struct timeval read_timeout;
    fd_set set;
    char buf[1024];
    char *search_str;
    int rc;
    struct timeval start_time, curr_time;
    int elapsed_time_in_ms;

    /* Sanity check */
    if (tty_dev == NULL || comp_str == NULL) {
        printf("%s: Null pointer\n", __FUNCTION__);
        return (FAILED);
    }

    uart_fd = open(tty_dev, O_RDWR|O_NOCTTY);

    if (uart_fd < 0) {
        perror("polling uart: open tty failed");
        fflush(stdout);
        return (FAILED);
    }

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
            if ((NVRAM)->diagflag & D_VERBOSE) {
                printf("buf =<%s>",buf);
            }
            /* Check if compared string can be found in the incoming string */
            search_str = strstr(buf, comp_str);
            if (search_str != NULL) { /* Found the string */
                close(uart_fd);
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
    close(uart_fd);

    return (FAILED);
}


/**********************************************************************
 *
 * Function: fxs_si32261_common_mode_calibration
 *
 * Description: The routine performs fxs calibration on all ports
 * 
 * Input :  tboard - test with all or separate MB/DBx board
 *
 * Output:  PASSED/FAILED
 *
 *********************************************************************
*/
static int fxs_si32261_common_mode_calibration (int tboard)
{
    int  dsp, wait_time, rc, result = PASSED; 
    ngsm_entity_t *ngsm_ep;
    char tty_dev[20], bufmsg[128];;
    char load_linux_str[256]; 
    fe_packet_t   *recv_packet_p;
    dspif_ether_t *result_packet_p;

    ngsm_ep = (ngsm_entity_t *)ngio_ptr->priv;

    printf("\nFXS LB Calibration\n");

    for (dsp = 0; dsp < MAX_DSPS_PER_NGSM; dsp++) {
        ngsm_ep->dsp_downloaded[dsp] = FALSE;
    }
 
    /* Bringup DSP */
    if (phoenix_bringup_dsp() == FAILED) {
        cterr('f', 0, "%s(): \n DSP READY PIN not set. Please check if "
               "\"/firmware/dsp_sp2700_fw.img\" exists.\n", __FUNCTION__);
        return (FAILED);
    }

    /* setup hardware board strape type to DSP */
    if (phoenix_vir_setup_hw_brd_type() == FAILED) {
       printf("ERROR: Set phoenix_hw_brd_type_flag to DSP failed\n");
       return (FAILED);
    }

    /* separate testing with different board by tboard */
    if (tboard == PHOENIX_TEST_ONLY_MB) {
        printf("\nFXS LB Calibration with MB\n");
        result = phoenix_select_test(SET_TOGGLE_SEP_MB_DBX_TEST_FLAG,
                                  PHOENIX_TEST_ONLY_MB, 0, 100);
    } else if (tboard == PHOENIX_TEST_ONLY_DB1) {
        printf("\nFXS LB Calibration with DB1\n");
        result = phoenix_select_test(SET_TOGGLE_SEP_MB_DBX_TEST_FLAG,
                                  PHOENIX_TEST_ONLY_DB1, 0, 100);
    } else if (tboard == PHOENIX_TEST_ONLY_DB2) {
        printf("\nFXS LB Calibration with DB2\n");
        result = phoenix_select_test(SET_TOGGLE_SEP_MB_DBX_TEST_FLAG,
                                  PHOENIX_TEST_ONLY_DB2, 0, 100);
    } else if (tboard == PHOENIX_TEST_ONLY_DB3) {
        printf("\nFXS LB Calibration with DB3\n");
        result = phoenix_select_test(SET_TOGGLE_SEP_MB_DBX_TEST_FLAG,
                                  PHOENIX_TEST_ONLY_DB3, 0, 100);
    } else {
        /* Default DSP power on will run all, so run all do not need to set */
        printf("\nFXS LB Calibration with all board\n");
    }

    if (result == FAILED) {
       printf("ERROR: Set phoenix_sep_test_dbx_flag to DSP failed\n");
       return (FAILED);
    }

    printf("Do FXS initialization\n");
    printf("(This procedure takes a while. Please be patient)\n\n");

    result = phoenix_select_test(SELECT_CODEC_SI32261_CALIBRATION, 0, 0, DSP_SI32261_CAL_TIME);
    if (result == FAILED) {
        cterr('f', 0,"%s(): SI32261 do calibration fail", __FUNCTION__);
        get_buf_message();
        return (FAILED);
    }

    printf("\n\nNow collecting calibration result ...\n");

    result = phoenix_select_test(SELECT_CODEC_SI32261_CALIBRATE_SAVE, 0, 0, 1000);
    if (result == FAILED) {
        cterr('f', 0,"%s(): Save calibration data fail", __FUNCTION__);
        get_buf_message();
        return (FAILED);
    }

    if (result == PASSED) {
        recv_packet_p = &(ngsm_ep->recv_packet);
        result_packet_p = &(ngsm_ep->result_packet);
        memcpy((char *)result_packet_p, (char *)(&(recv_packet_p->data[0])+0),
                sizeof(dspif_ether_t));
        memcpy(bufmsg, result_packet_p->dspif_info.bufmsg, sizeof(bufmsg));
        printf("\n %s\n", bufmsg);
    }

    for (dsp = 0; dsp < MAX_DSPS_PER_NGSM; dsp++) {
        ngsm_ep->dsp_downloaded[dsp] = FALSE;
    }

    printf("\n\nNow checking calibration value in SPI ROM ...\n");

    sprintf(tty_dev, "/dev/ttyDASH%d", PHOENIX_UART );

    /* Reset DSP for entering the console to check calibration value*/
    lsi_sp27xx_sm_reset();

    wait_time = TIME_GET_STRING;
    do {
        phoenix_tx_uart(tty_dev, "\003"); /* issue ctrl+C to stop autoboot */
        phoenix_tx_uart(tty_dev, "\015");
        rc = (phoenix_rx_polling_uart(tty_dev,"BLDR", 100));
        if (rc == PASSED) {
            printf("Found string BLDR> \n ");
            fflush(stdout);
            break;

        }
        if ((wait_time == 0 ) && (rc != PASSED)) {
                cterr('f', 0,"%s(): Comparing string fail", __FUNCTION__);
        }
    } while (wait_time--);


    wait_time = TIME_GET_CAL_VAL;
    msleep(UART_WAIT_TIME); /* Wait Uart print message */
    do {
        if ((NVRAM)->diagflag & D_VERBOSE) {
            printf("bufmsg =<%s>",bufmsg);
        }
        phoenix_tx_uart(tty_dev, "\015");
        sprintf(load_linux_str, "set");
        phoenix_tx_uart(tty_dev, load_linux_str);
        phoenix_tx_uart(tty_dev, "\015");
        rc = (phoenix_rx_polling_uart(tty_dev,bufmsg, BUF_WAIT_TIME));
        if (rc == PASSED) {
            printf("\n\n Calibration value verified in SPI ROM PASSED...\n");
            fflush(stdout);
            break;
        }
        if ((wait_time == 0 ) && (rc != PASSED)) {
            cterr('f', 0,"%s(): Writing calibration value to SPI ROM Failed", __FUNCTION__);
        }
    } while (wait_time--);


    return (PASSED);

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
        phoenix_select_test(SELECT_CODEC_SET_FAIL_OVER_PORT, 0, 0, 100);
    } else {
        phoenix_select_test(SELECT_CODEC_SET_FAIL_OVER_PORT, TRUE, 0, 100);
    }

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
    uchar phoenix_get_pid[FRU_SIZE] = {0};
    uchar phoenix_get_loc[FRU_SIZE] = {0};
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

    sprintf(fru_board_id , "%x", phoenix_board_id);
    memcpy(phoenix_get_pid,(char*)&fru_board_id,5); 
    strcpy((char *)phoenix_get_loc, "FPGA Register Test");

    platform_fru_table[fru_table_offset].pid_string = phoenix_get_pid ; 
    platform_fru_table[fru_table_offset].location_string = phoenix_get_loc;
 
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

    
    if (phoenix_select_test(SELECT_FPGA_REG_TEST, 0, 0, 100) == FAILED) {
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
    uchar phoenix_get_pid[FRU_SIZE] = {0};
    uchar phoenix_get_loc[FRU_SIZE] = {0};
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

    sprintf(fru_board_id , "%x", phoenix_board_id);
    memcpy(phoenix_get_pid,(char*)&fru_board_id,5); 
    strcpy((char *)phoenix_get_loc, "FPGA Memory Test");

    platform_fru_table[fru_table_offset].pid_string = phoenix_get_pid ; 
    platform_fru_table[fru_table_offset].location_string = phoenix_get_loc;
 
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

    if(phoenix_select_test(SELECT_FPGA_MEM_TEST, 0, 0, 100) == FAILED) {
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
    uchar phoenix_get_pid[FRU_SIZE] = {0};
    uchar phoenix_get_loc[FRU_SIZE] = {0};
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

    sprintf(fru_board_id , "%x", phoenix_board_id);
    memcpy(phoenix_get_pid,(char*)&fru_board_id,5); 
    strcpy((char *)phoenix_get_loc, "FPGA interrupt Test");

    platform_fru_table[fru_table_offset].pid_string = phoenix_get_pid ; 
    platform_fru_table[fru_table_offset].location_string = phoenix_get_loc;
 
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

    if(phoenix_select_test(SELECT_FPGA_INT_TEST, 0, 0, 100) == FAILED) {
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
    uchar phoenix_get_pid[FRU_SIZE] = {0};
    uchar phoenix_get_loc[FRU_SIZE] = {0};
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

    sprintf(fru_board_id , "%x", phoenix_board_id);
    memcpy(phoenix_get_pid,(char*)&fru_board_id,5); 
    strcpy((char *)phoenix_get_loc, "FPGA TDMSW16 force byte test");

    platform_fru_table[fru_table_offset].pid_string = phoenix_get_pid ; 
    platform_fru_table[fru_table_offset].location_string = phoenix_get_loc;
 
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

    if(phoenix_select_test(SELECT_FPGA_TDMSW_FORCE_BYTE_TEST, 0, 0, 100) == FAILED) {
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
 *  Input: tboard : test with all or separate board
 *
 *  Returns: PASSED if successful; 
 *           FAILED, otherwise 
 *
 **********************************************************************
 */
static int fxs_si32261_lpbk_test (int tboard)
{

#ifdef ENHANCED_ERR_MSG_EXAMPLE
    uchar phoenix_get_pid[FRU_SIZE] = {0};
    uchar phoenix_get_loc[FRU_SIZE] = {0};
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

    sprintf(fru_board_id , "%x", phoenix_board_id);
    memcpy(phoenix_get_pid,(char*)&fru_board_id,5); 
    strcpy((char *)phoenix_get_loc, "SI32261 Loopback test");

    platform_fru_table[fru_table_offset].pid_string = phoenix_get_pid ; 
    platform_fru_table[fru_table_offset].location_string = phoenix_get_loc;
 
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
    
    if (tboard == PHOENIX_TEST_ONLY_MB) {
        prpass(testpass, " MB FXS SI32261 Loopback - ");
        result = phoenix_select_test(SET_TOGGLE_SEP_MB_DBX_TEST_FLAG, 
                                  PHOENIX_TEST_ONLY_MB, 0, 100);
    } else if (tboard == PHOENIX_TEST_ONLY_DB1) {
        prpass(testpass, " DB1 FXS SI32261 Loopback - ");
        result = phoenix_select_test(SET_TOGGLE_SEP_MB_DBX_TEST_FLAG, 
                                  PHOENIX_TEST_ONLY_DB1, 0, 100);
    } else if (tboard == PHOENIX_TEST_ONLY_DB2) {
        prpass(testpass, " DB2 FXS SI32261 Loopback - ");
        result = phoenix_select_test(SET_TOGGLE_SEP_MB_DBX_TEST_FLAG, 
                                  PHOENIX_TEST_ONLY_DB2, 0, 100);
    } else if (tboard == PHOENIX_TEST_ONLY_DB3) {
        prpass(testpass, " DB3 FXS SI32261 Loopback - ");
        result = phoenix_select_test(SET_TOGGLE_SEP_MB_DBX_TEST_FLAG, 
                                  PHOENIX_TEST_ONLY_DB3, 0, 100);
    } else {
        prpass(testpass, " FXS SI32261 Loopback with all board - ");
    }
    if (result == FAILED) {
       printf("ERROR: Set phoenix_sep_test_dbx_flag to DSP failed\n");
       return (FAILED);
    }

    assert(ngio_ptr);

    result = phoenix_select_test(SELECT_CODEC_SI32261_DIGITAL_LOOPBACK, 0, 0, 100);

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
    uchar phoenix_get_pid[FRU_SIZE] = {0};
    uchar phoenix_get_loc[FRU_SIZE] = {0};
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
    sprintf(fru_board_id , "%x", phoenix_board_id);
    memcpy(phoenix_get_pid,(char*)&fru_board_id,5); 
    strcpy((char *)phoenix_get_loc, "DB1 SI3050 Loopack test");

    platform_fru_table[fru_table_offset].pid_string = phoenix_get_pid; 
    platform_fru_table[fru_table_offset].location_string = phoenix_get_loc;
 
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
    
    result = phoenix_select_test(SELECT_CODEC_SI3050_DIGITAL_LOOPBACK, 0, 0, 100);
     
    get_buf_message();

    return (result);
}


/*  
 **********************************************************************
 *  
 *  Function: phoenix_arm11_cpu1_boot_test_wrapper
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
static int phoenix_arm11_cpu1_boot_test_wrapper (void)
{
    prpass(testpass, " ARM11 CPU1 Boot - ");
    return (phoenix_select_test(SELECT_ARM11CPU1_BOOT, 0, 0, 50));
}

/*  
 **********************************************************************
 *  
 *  Function: phoenix_dss_core_sanity_test_wrapper
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
static int phoenix_dss_core_sanity_test_wrapper (int core)
{
    if (core == DSS_CORE0) {
        prpass(testpass, " DSS Core0 Sanity - ");
        return (phoenix_select_test(SELECT_DSS0_SANITY, 0, 0, 50));
    } else if (core == DSS_CORE1) {
        prpass(testpass, " DSS Core1 Sanity - ");
        return (phoenix_select_test(SELECT_DSS1_SANITY, 0, 0, 50));
    } else if (core == DSS_CORE2) {
        prpass(testpass, " DSS Core2 Sanity - ");
        return (phoenix_select_test(SELECT_DSS2_SANITY, 0, 0, 50));
    } else {
        prpass(testpass, " DSS Core3 Sanity - ");
        return (phoenix_select_test(SELECT_DSS3_SANITY, 0, 0, 50));
    }
}

/*
 **********************************************************************
 *
 *  Function: phoenix_ge_lpbk_test_wrapper
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
static int phoenix_ge_lpbk_test_wrapper (int port)
{
    int ret_val = PASSED;
 
    assert(ngio_ptr);

    if (phoenix_setup_ge_env() == FAILED) {
        return (FAILED);
    }

    if (phoenix_ge_lpbk_test(port)) {
        ret_val = FAILED;
    }
    phoenix_cleanup_ge_env();

    return (ret_val);
}

/*
 **********************************************************************
 *
 *  Function: phoenix_utils
 *
 *  Description: Phoenix utilities menu
 *
 *  Input: None 
 *
 *  Returns: PASSED
 *
 **********************************************************************
 */
static int phoenix_utils (void)
{
    ngsm_entity_t *ngsm_ep;

    assert(ngio_ptr);
    ngsm_ep = (ngsm_entity_t *)ngio_ptr->priv;
    assert(ngsm_ep);

    sprintf(phoenixutiltitle, "Virtual SM Utilities Menu");
    build_primary_submenu(phoenix_utils_submenu_table,
                          PHOENIX_UTILS_SUBMENU_TABLE_SZ,
                          phoenixutiltitle, &phoenix_util_submenup);

    build_secondary_submenu(phoenix_utils_submenu_table,
                            PHOENIX_UTILS_SUBMENU_TABLE_SZ,
                            phoenix_utils_secondary_items);

    menu(phoenix_util_submenup, phoenix_utils_secondary_items, '\0');

    return (PASSED);
}

/*
 **********************************************************************
 *
 *  Function: phoenix_iface_test
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
int phoenix_iface_test (void)
{
    int ngsm_slot = 0;
    int retval = PASSED;

    assert(ngio_ptr);
    ngsm_slot = ngio_ptr->slot;

    retval = phoenix_gpio_exp_test();

    if (retval == PASSED) {
        retval = phoenix_ge_lpbk_test_wrapper(DSP_SLOT0_ETH_PORT);
    }
    if (retval == PASSED) {
        retval = phoenix_uart_test();
    }

    return (retval);
}


/*
 **********************************************************************
 *
 *  Function: phoenix_run_test
 *
 *  Description: Run all the Phoenix specific tests.
 *
 *  Input: none
 *  
 *  Returns: PASSED if successful; 
 *           FAILED, otherwise
 *  
 **********************************************************************
 */
static int phoenix_run_test (void)
{
    int  retval = PASSED;

    assert(ngio_ptr);
    
    /* All these tests are executed by sending a ethernet test command
       packet to the PHOENIX ARM CPU0 */
    retval = phoenix_dsp_ddr3_sdram_test_wrapper();
    if (retval == PASSED) {
        retval = phoenix_ge_lpbk_test_wrapper(0);
    }
    if (retval == PASSED) {
        retval = phoenix_ge_lpbk_test_wrapper(1);
    }
    if (retval == PASSED) {
        retval = phoenix_arm11_cpu1_boot_test_wrapper();
    }
    if (retval == PASSED) {
        retval = phoenix_dss_core_sanity_test_wrapper(DSS_CORE0);
    }
    if (retval == PASSED) {
        retval = phoenix_dss_core_sanity_test_wrapper(DSS_CORE1);
    }
    if (retval == PASSED) {
        retval = phoenix_dss_core_sanity_test_wrapper(DSS_CORE2);
    }
    if (retval == PASSED) {
        retval = phoenix_dss_core_sanity_test_wrapper(DSS_CORE3);
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
        retval = phoenix_tdm_ext_lpbk_test_wrapper();
    }
    if (retval == PASSED) {
        retval = fpga_tdmsw_force_test();
    }
    if (retval == PASSED) {
        retval = fxs_si32261_lpbk_test(PHOENIX_TEST_ALL_BOARD);
    }
    if (retval == PASSED) {
        if (db1_has_fxo() == TRUE) {
            retval = fxo_si3050_lpbk_test();
        } else {
            printf("No FXO port \n");
        }
    }
    if (retval == PASSED) {
        retval = phoenix_uart_test();
    }
    if (retval == PASSED) {
        retval = phoenix_tdm_codec_reset_test(PHOENIX_TEST_ALL_BOARD);
    }

    return (retval);
}


/*
 **********************************************************************
 *
 *  Function: phoenix_test
 *
 *  Description: show Phoenix test submenu or    
 *
 *  Input: 
 *
 *  Returns: PASSED if successful;
 *           FAILED - DSP ready pin not set, the test failed
 *
 **********************************************************************
 */
static int phoenix_test (int run_tests)
{
    int retval = PASSED;

    testname ("PHOENIX Virtual SM");
    /* Bringup DSP */
    if (phoenix_bringup_dsp() == FAILED) {
        cterr('f', 0, "%s(): \n DSP READY PIN not set. Please check if "
               "\"/firmware/dsp_sp2700_fw.img\" exists.\n", __FUNCTION__);
        return (FAILED);
    }

    /* setup hardware board strape type to DSP */
    if (phoenix_vir_setup_hw_brd_type() == FAILED) {
       printf("ERROR: Set phoenix_hw_brd_type_flag to DSP failed\n");
       return (FAILED);
    }

    if (ngio_ptr->test_type == IFACE_TEST)
        return(phoenix_iface_test());

    if (phoenix_dsp_tests_use_enet == 1) {
        if (run_tests == 1) {
            retval = phoenix_run_test();
        } else {
            build_primary_submenu(phoenix_tests_submenu_table,
                                  PHOENIX_TESTS_SUBMENU_TABLE_SIZE,
                                  phoenixsubmenutitle, &phoenix_submenup);
            build_secondary_submenu(phoenix_tests_submenu_table,
                                    PHOENIX_TESTS_SUBMENU_TABLE_SIZE,
                                    phoenix_tests_secondary_items);

            menu(&phoenix_subtest_menu, phoenix_tests_secondary_items, '\0');
        }
    } else {
        phoenix_con_utils();
    }
    return retval;
}


/*
 **********************************************************************
 *
 *  Function: phoenix_con_utils
 *
 *  Description: wrapper so we can set up and clear up console.
 *
 *  Input: None 
 *
 *  Returns: PASSED if successful;
 *
 **********************************************************************
 */
static int phoenix_con_utils (void)
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
    msleep(UART_READY_TIME);

    sprintf(disp, "picocom -b 9600 -d8 -pn -fn %s", ep->tty_dev);
    printf("\n %s\n\n\n", disp);
    fflush(stdout);
    fflush(stderr);
    msleep(UART_READY_TIME);
    system(disp);
    return (PASSED);
}

/*
 **********************************************************************
 *
 *  Function: phoenix_wait_for_ge_packet
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
int phoenix_wait_for_ge_packet (uchar *pak, int socket_gl, int mode, int dsp,
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

/*
 **********************************************************************
 *
 *  Function: toggle_sep_test_dbx_flag
 *
 *  Description: This function will set phoenix_sep_test_dbx_flag
 *               The flag use to separate MB or DBx testing.
 *
 *  Input: none
 *
 *  Returns: none
 *
 **********************************************************************
 */
void toggle_sep_test_dbx_flag (void)
{
    int ret = FAILED;
    uint32_t flag;

    printf("Set flag phoenix_sep_test_dbx_flag to run separate MB or DBx testing \n"
           "Example: \n"
           "0xF, binary 1111 run MB, DB1, DB2 and DB3 \n"
           "0x8, binary 1000 run MB only, \n"
           "0x4, binary 0100 run DB1 only, \n"
           "0x2, binary 0010 run DB2 only, \n"
           "0x1, binary 0001 run DB3 only, \n"
           "0xC, binary 1100 run MB, DB1 only, \n"
           "0x7, binary 0111 run DB1, DB2, DB3 only.\n");

    flag = gethex_answer("\nEnter separate test MB or DBx flag[0x1 to 0xF]:", 0xf, 0x1, 0xf);
  
    if ((flag <= PHOENIX_TEST_SPE_BOARD_MIN) || 
        (flag > PHOENIX_TEST_SPE_BOARD_MAX)) {
        printf("Invalid input\n");
        return;
    }

    phoenix_sep_test_dbx_flag = flag; 
    
    ret = phoenix_select_test(SET_TOGGLE_SEP_MB_DBX_TEST_FLAG, phoenix_sep_test_dbx_flag, 0, 100);
    if (ret == FAILED) {
        printf("ERROR: Set phoenix_sep_test_dbx_flag to DSP failed\n");
    } else {
        printf("\nSet phoenix_sep_test_dbx_flag = 0x%x\n", phoenix_sep_test_dbx_flag);
    }
}

/**********************************************************************
 * Function: phoenix_tdm_codec_reset_test
 *
 * Description: This function performs the TDM codec reset test for
 *              MB, DB1, DB2 and DB3.
 *
 * Input:  None
 *
 * Output: PASSED/FAILED
 **********************************************************************
 */
static int phoenix_tdm_codec_reset_test (int tboard)
{
    int result = FAILED;

    if (tboard == PHOENIX_TEST_ONLY_MB) {
        prpass(testpass, "MB TDM Codec Reset test - ");
        result = phoenix_select_test(SET_TOGGLE_SEP_MB_DBX_TEST_FLAG, 
                                  PHOENIX_TEST_ONLY_MB, 0, 100);
        if (result == FAILED) {
           printf("ERROR: Set phoenix_sep_test_dbx_flag to DSP failed\n");
           return (FAILED);
        }
    } else if (tboard == PHOENIX_TEST_ONLY_DB1) {
        prpass(testpass, "DB1 TDM Codec Reset test - ");
        result = phoenix_select_test(SET_TOGGLE_SEP_MB_DBX_TEST_FLAG, 
                                  PHOENIX_TEST_ONLY_DB1, 0, 100);
        if (result == FAILED) {
           printf("ERROR: Set phoenix_sep_test_dbx_flag to DSP failed\n");
           return (FAILED);
        }
    } else if (tboard == PHOENIX_TEST_ONLY_DB2) {
        prpass(testpass, "DB2 TDM Codec Reset test - ");
        result = phoenix_select_test(SET_TOGGLE_SEP_MB_DBX_TEST_FLAG, 
                                  PHOENIX_TEST_ONLY_DB2, 0, 100);
        if (result == FAILED) {
           printf("ERROR: Set phoenix_sep_test_dbx_flag to DSP failed\n");
           return (FAILED);
        }
    } else if (tboard == PHOENIX_TEST_ONLY_DB3) {
        prpass(testpass, "DB3 TDM Codec Reset test - ");
        result = phoenix_select_test(SET_TOGGLE_SEP_MB_DBX_TEST_FLAG, 
                                  PHOENIX_TEST_ONLY_DB3, 0, 100);
        if (result == FAILED) {
           printf("ERROR: Set phoenix_sep_test_dbx_flag to DSP failed\n");
           return (FAILED);
        }
    } else {
        prpass(testpass, " TDM Codec Reset test with all board - ");
    }

    result = phoenix_select_test(SELECT_TDM_CODEC_RST, 0, 0, 100);

    return (result);
}

/*
 **********************************************************************
 *
 *  Function: txrx_cpld_data
 *
 *  Description: combine tx_data_to_dsp and rx_data_from_dsp to implement
 *               cpld upgrade.
 *
 *  Input: None
 *
 *  Returns: PASSED if successful; 
 *           FAILED, otherwise
 *
 **********************************************************************
 */
void *txrx_cpld_data (void)
{
    ngsm_entity_t *ep;
    int ix, ngsm_slot = 0;
    mac_addr_t   src_mac_addr;
    uchar recv_buffer[PHOENIX_CPLD_RECV_BUFF] = {0};
    /* Because packet is 1000 bytes and first bytes is type so need 1001.*/
    uchar tx_buffer[PHOENIX_CPLD_TX_BUFF] = {0};
    int timeout = TIME_OUT, tx_cpld_data_size;
    unsigned int cpld_image_size = PHOENIX_CPLD_IMAGE_SIZE;
    unsigned int cpld_data_index = 0;

    phoenix_setup_ge_env();

    ep = (ngsm_entity_t *)ngio_ptr->priv;
    ngsm_slot = ngio_ptr->slot;
    get_host_mac_addr(DSP_SLOT0_ETH_PORT, (uchar *)&src_mac_addr[0]);

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

    while (cpld_image_size != 0) {

        memset((uchar *)tx_buffer, 0, sizeof(tx_buffer));

        if (cpld_image_size >= DSP_FPGA_SIZE) {
            tx_cpld_data_size = DSP_FPGA_SIZE;
            cpld_image_size -= DSP_FPGA_SIZE;
        } else {
            tx_cpld_data_size = cpld_image_size;
            cpld_image_size = 0;
        }

        tx_buffer[0] = PACKET_TO_DSP;
        for (ix = 0; ix < tx_cpld_data_size; ix++) {
            tx_buffer[ix + 1] = cpld_image[cpld_data_index];
            cpld_data_index++;
        }

        tx_data_to_dsp(ep->socket_gl, tx_buffer, sizeof(tx_buffer)); 

        timeout = TIME_OUT;
        while (timeout--) {
            /* clear buffer before use */
            memset((uchar *)recv_buffer, 0, sizeof(recv_buffer));

            rx_data_from_dsp(ep->socket_gl, recv_buffer, sizeof(recv_buffer));

            if ((recv_buffer[PHOENIX_CPLD_RECV_FOR_DSP]) == PACKET_FROM_DSP) {
                break;
            }
        }
        if (timeout == 0) {
            printf("!!Don't receive packet from DSP\n");
            pthread_exit((void *)PASSED);
        }
    }

    printf("Send CPLD Image to DSP done!!\n");
    pthread_exit((void *)PASSED);
}
/*
 **********************************************************************
 *
 *  Function: load_cpld_data 
 *
 *  Description: Load CPLD image and use fopen to open cpld image file. 
 *          
 *  Input: *data - cpld image buffer 
 *
 *  Returns: PASSED if successful;
 *
 **********************************************************************
 */
int load_cpld_data (unsigned char * data) 
{
    FILE *pFile;
    int c, ix;
    unsigned int val;

    pFile = fopen("/firmware/cpld_image.h", "r"); 

    if (!pFile) {
        printf("\n\nCan't open file, please check the folder whether \
                            the cpld_image.h is exit or not\n\n");
        return (FAILED);
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
 *  Function: cpld_upgrade_utility 
 *
 *  Description: CPLD Upgrade Utility
 *
 *  Input: None
 *
 *  Returns: PASSED if successful; 
 *           FAILED, otherwise
 *
 **********************************************************************
 */
static int cpld_upgrade_utility (void)
{
    int rc;
    pthread_t threads;
    printf("CPLD Upgrade\n");

    printf("Loading CPLD Image\n");
    if (load_cpld_data(cpld_image) == FAILED) {
        printf("Load CPLD data Failed!!\n");
        return (FAILED);
    }

    printf("Create pthread for sending CPLD Image to DSP\n");
    rc = pthread_create (&threads, NULL, (void *)txrx_cpld_data, NULL);
    if (rc != PASSED) {
        printf("pthread_create failed \n");
        return (FAILED);
    }

    phoenix_con_utils();
    pthread_cancel(threads);
    printf("Remove the thread!\n");
    return (PASSED);
}

/******** History ********
$Log: phoenix_virtual_sm.c,v $
Revision 1.2  2021/04/15 00:52:27  achiu2
[PRRQ:CSCvx56970-2]Phoenix code review for ER

Revision 1.1.2.4  2020/08/05 10:23:16  achiu2
porting back "Replace is_fxo function with db1_has_fxo." modification



$Endlog$
*/
