/* $Id: diag_fpga_test.c,v 1.10 2020/12/25 09:24:18 alicehua Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/nutella/diag_fpga_test.c,v $
 *------------------------------------------------------------------
 *
 * diag_fpga_test.c - FPGA Test
 *
 *
 * Copyright (c) 2019 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include "endians.h"
#include "defs.h"
#include "types.h"
#include "nvsysvars.h"
#include "error.h"
#include "pcmap.h"
#include "strings.h"
#include "menu.h"
#include "common.h"
#include "cross_platform.h"
#include "mb_tests.h"
#include "plat_defs.h"
#include "setjmps.h"
#include "proto.h"
#include "diag_fpga.h"
#include "platform_fru.h"
#include "platform_cookie.h"
#include "diag_fpga_upgrade.h"
#include "diag_fpga_lib.h"
#include "plat_defs.h"
#include "diag_fpga_i2c_lib.h"
#include "dnv_gpio_lib.h"
#include "diag_common.h"
#include "dnv_eth_lib.h"
#include "diag_gephy_lib.h"
#include "diag_lte_lib.h"
#include "diag_lte_test.h"
#include "tam_aikido_upgrade.h"
#include "nutella_comm.h"


/*
 * Global variables
 */

static boolean fpga_watchdog_force_stop = FALSE;


/* Local functions */
int diag_fpga_reg_test(void);
int build_fpga_test_menu(boolean);
int fpga_reg_test_read_fn(ulong, int, ulong *, void *);
int fpga_reg_test_write_fn(ulong, int, ulong, void *);
int nutella_fpga_utils(int);
int nutella_fpga_function_test(int);
int nutella_show_fpga_ver(int);
int fpga_reg_rd_util(int);
int fpga_reg_wr_util(int);
int fpga_reg_dump_util(int);
int fpga_reg_dump_def_util(int);
int fpga_sys_reset_util(int);
int fpga_show_reset_reason_util(int);
void fpga_watchdog_timer_counting(void);
int fpga_watchdog_timer_enable_util(int);
int fpga_ios_watchdog_timer_reg_test(int);
int fpga_ios_watchdog_strobe_reg_test(int);
int fpga_phy_reset_test(int);
int fpga_power_status_control_reg_test(int);
int fpga_i2c_reset_test(void);
int fpga_revision_test(void);
int diag_wdt_disable_test(void);
int diag_wdt_counter_test(void);
int diag_fpga_esw_reset_test(void);
int diag_board_type_test(void);
int fpga_lpc_bad_addr_access_test(void);
int fpga_read_default_value_test(void);
int fpga_scratchpad_register_test(void);
int fpga_lpc_cpu_error_stat_test(void);
int fpga_lpc_irq_force_test(int);
int fpga_eobc_or_packet_interrupt_test(int);
int fpga_mcerr_test(int);
int fpga_cpu_prochot_test(int);
int fpga_lpc_reset_ctl_reg_test(void);
int fpga_lpc_debug_ctl_stat_reg_test(void);
int fpga_lpc_flash_led_ctl_reg_test(void);
int fpga_lpc_chassis_manage_ctl_reg_test(void);
int fpga_lpc_alarm_manage_ctl_reg_test(void);
int fpga_corrupt_header_spi_prom(int);

reg_info_t_ext nutella_fpga_reg_ext = {NUTELLA_FPGA_REG_WIDTH,
                                     fpga_reg_test_read_fn,
                                     fpga_reg_test_write_fn,
                                     0};


static reg_info_t fpga_reg_dump_tbl[] = {
    /* Format: NAME, OFFSET, TYPE, SIZE, MASK, RESET_VAL. */
    {"FPGA External Device Reset",    FPGA_EXTER_DEV_RST_REG,     FPGA_RW,
        {(unsigned long)&nutella_fpga_reg_ext},   0x03F83ABA, 0x370FAF2},
    {"Internal Device Reset",         FPGA_INT_DEV_RST_REG,       FPGA_RW,
        {(unsigned long)&nutella_fpga_reg_ext},   0xFFFFFFFF, 0x0},
    {"Board Type",                    FPGA_BOARD_TYPE_REG,        FPGA_RONLY,
        {(unsigned long)&nutella_fpga_reg_ext},   0x00000000, 0x13},
    {"Master FPGA Revision",          FPGA_MASTER_REV_REG,        FPGA_RONLY,
        {(unsigned long)&nutella_fpga_reg_ext},   0x00000000, 0x800103},
    {"FPGA Revision",                 FPGA_REV_REG,               FPGA_RONLY,
        {(unsigned long)&nutella_fpga_reg_ext},   0x00000000, 0x16041901},
    {"CPU Mux and USB Power Control", FPGA_CPUMUX_AND_USBPWR_REG, FPGA_RW,
        {(unsigned long)&nutella_fpga_reg_ext},   0x00000003, 0x0},
    {"Status and Control",            FPGA_STAT_AND_CTRL_REG,     FPGA_RW,
        {(unsigned long)&nutella_fpga_reg_ext},   0x000003EE, 0xE},
    {"Power Status",                  FPGA_PWR_STAT_REG,          FPGA_RONLY,
        {(unsigned long)&nutella_fpga_reg_ext},   0x00000000, 0xFFFFFFFF},
    {"LED",                           FPGA_LED_REG,               FPGA_RW,
        {(unsigned long)&nutella_fpga_reg_ext},   0x00FCFE03, 0x0},
    {"Watchdog Strobe",               FPGA_WATCHDOG_REG,          FPGA_RONLY,
        {(unsigned long)&nutella_fpga_reg_ext},   0x00000000, 0x0},
    {"LTE Control",                   FPGA_LTE_CTL_REG,           FPGA_RW,
        {(unsigned long)&nutella_fpga_reg_ext},   0x000000CE, 0x0},
    {"SIM Status and Control",        FPGA_SIM_STATUS_CTL_REG,    FPGA_RW,
        {(unsigned long)&nutella_fpga_reg_ext},   0x00000306, 0x0},
};

/*
 * FPGA register test
 */
/* vEdge table */
static reg_info_t fpga_reg_test_tbl[] = {
    /* Format: NAME, OFFSET, TYPE, SIZE, MASK, RESET_VAL. */
    {"Access Test Register R/W", FPGA_ACCESS_TEST_REG, FPGA_RW,
     {(unsigned long)&nutella_fpga_reg_ext}, 0xFFFFFFFF, 0},
	{"Scratchpad Register 1", FPGA_SCRATCHPAD_REG_1, FPGA_RW,
	 {(unsigned long)&nutella_fpga_reg_ext}, 0xFFFFFFFF, 0x0},
	{"Scratchpad Register 2", FPGA_SCRATCHPAD_REG_2, FPGA_RW,
	 {(unsigned long)&nutella_fpga_reg_ext}, 0xFFFFFFFF, 0x0},
	{"Scratchpad Register 3", FPGA_SCRATCHPAD_REG_3, FPGA_RW,
	 {(unsigned long)&nutella_fpga_reg_ext}, 0xFFFFFFFF, 0x0},
	{"Scratchpad Register 4", FPGA_SCRATCHPAD_REG_4, FPGA_RW,
	 {(unsigned long)&nutella_fpga_reg_ext}, 0xFFFFFFFF, 0x0},
	{"Scratchpad Register 5", FPGA_SCRATCHPAD_REG_5, FPGA_RW,
	 {(unsigned long)&nutella_fpga_reg_ext}, 0xFFFFFFFF, 0x0},
	{"Scratchpad Register 6", FPGA_SCRATCHPAD_REG_6, FPGA_RW,
	 {(unsigned long)&nutella_fpga_reg_ext}, 0xFFFFFFFF, 0x0},
    {"END",                                      0x00,       0,
     {0},                                         0x0,        0x0},
};

/* cEdge table */
static reg_info_t fpga_reg_test_tb2[] = {
    /* Format: NAME, OFFSET, TYPE, SIZE, MASK, RESET_VAL. */
    {"Access Test Register R/W", FPGA_ACCESS_TEST_REG, FPGA_RW,
     {(unsigned long)&nutella_fpga_reg_ext}, 0xFFFFFFFF, 0},
	{"Scratchpad Register 1", CEDGE_FPGA_SCRATCHPAD_REG_1, FPGA_RW,
	 {(unsigned long)&nutella_fpga_reg_ext}, 0xFFFFFFFF, 0x0},
	{"Scratchpad Register 2", CEDGE_FPGA_SCRATCHPAD_REG_2, FPGA_RW,
	 {(unsigned long)&nutella_fpga_reg_ext}, 0xFFFFFFFF, 0x0},
	{"Scratchpad Register 3", CEDGE_FPGA_SCRATCHPAD_REG_3, FPGA_RW,
	 {(unsigned long)&nutella_fpga_reg_ext}, 0xFFFFFFFF, 0x0},
	{"Scratchpad Register 4", CEDGE_FPGA_SCRATCHPAD_REG_4, FPGA_RW,
	 {(unsigned long)&nutella_fpga_reg_ext}, 0xFFFFFFFF, 0x0},
	{"Scratchpad Register 5", CEDGE_FPGA_SCRATCHPAD_REG_5, FPGA_RW,
	 {(unsigned long)&nutella_fpga_reg_ext}, 0xFFFFFFFF, 0x0},
	{"Scratchpad Register 6", CEDGE_FPGA_SCRATCHPAD_REG_6, FPGA_RW,
	 {(unsigned long)&nutella_fpga_reg_ext}, 0xFFFFFFFF, 0x0},
    {"END",                                      0x00,       0,
     {0},                                         0x0,        0x0},
};

/* 
 * FPGA register default value
 */
static reg_info_t fpga_reg_def_tbl[] = {
	/* Format: NAME, OFFSET, TYPE, SIZE, MASK, DEFAULT_VAL. */
	{"SPI Control Register", FPGA_SPI_CONTROL_REG, FPGA_RW,
	 {(unsigned long)&nutella_fpga_reg_ext}, 0xC0000000, 0x0},
	{"CPU Boot Mux Register", FPGA_CPUMUX_AND_USBPWR_REG, FPGA_RW,
	 {(unsigned long)&nutella_fpga_reg_ext}, 0x00000060, 0x0},
	{"IOS Watchdog Strobe Register", FPGA_WATCHDOG_REG, FPGA_RW,
	 {(unsigned long)&nutella_fpga_reg_ext}, 0xFFFFFFFF, 0x0},
	{"I2C Scratch Pad Register", FPGA_I2C_SCRATCH_PAD, FPGA_RW,
	 {(unsigned long)&nutella_fpga_reg_ext}, 0xFFFFFFFF, 0xFACEDEAD},
	{"SPI PROM Control Register", FPGA_SPI_CTRL_REG, FPGA_RW,
	 {(unsigned long)&nutella_fpga_reg_ext}, 0x000087FF, 0x00000010},
	{"SPI PROM Status Register", FPGA_SPI_STAT_REG, FPGA_RW,
	 {(unsigned long)&nutella_fpga_reg_ext}, 0x0000801F, 0x0000000A},
	{"SPI PROM Read Size Register", FPGA_SPI_RD_SIZE_REG, FPGA_RW,
	 {(unsigned long)&nutella_fpga_reg_ext}, 0x000000FF, 0x0},
	{"SPI PROM Read/Write Data Register", FPGA_SPI_RW_DATA_REG, FPGA_RW,
	 {(unsigned long)&nutella_fpga_reg_ext}, 0x000000FF, 0x0},
	{"SPI PROM Opcode/Address Register", FPGA_SPI_OP_ADDR_REG, FPGA_RW,
	 {(unsigned long)&nutella_fpga_reg_ext}, 0xFFFFFFFF, 0x0},
	{"FPGA Reconfiguration Control Register", FPGA_RECONFIG_CTRL_REG, FPGA_RW,
	 {(unsigned long)&nutella_fpga_reg_ext}, 0x00000012, 0x0},
	{"Firmware Status Register", FPGA_FIRMWARE_STATUS_REG, FPGA_RW,
	 {(unsigned long)&nutella_fpga_reg_ext}, 0xFFFFFFFF, 0x0},
	{"Soft Secure Boot Status Register", FPGA_SOFT_SECURE_BOOT_STATUS_REG, FPGA_RW,
	 {(unsigned long)&nutella_fpga_reg_ext}, 0xFFFFFFFF, 0x0},
	{"Scratchpad Register 1", FPGA_SCRATCHPAD_REG_1, FPGA_RW,
	 {(unsigned long)&nutella_fpga_reg_ext}, 0xFFFFFFFF, 0x0},
	{"Scratchpad Register 2", FPGA_SCRATCHPAD_REG_2, FPGA_RW,
	 {(unsigned long)&nutella_fpga_reg_ext}, 0xFFFFFFFF, 0x0},
	{"Scratchpad Register 3", FPGA_SCRATCHPAD_REG_3, FPGA_RW,
	 {(unsigned long)&nutella_fpga_reg_ext}, 0xFFFFFFFF, 0x0},
	{"Scratchpad Register 4", FPGA_SCRATCHPAD_REG_4, FPGA_RW,
	 {(unsigned long)&nutella_fpga_reg_ext}, 0xFFFFFFFF, 0x0},
	{"Scratchpad Register 5", FPGA_SCRATCHPAD_REG_5, FPGA_RW,
	 {(unsigned long)&nutella_fpga_reg_ext}, 0xFFFFFFFF, 0x0},
	{"Scratchpad Register 6", FPGA_SCRATCHPAD_REG_6, FPGA_RW,
	 {(unsigned long)&nutella_fpga_reg_ext}, 0xFFFFFFFF, 0x0},
	{"END",                                      0x00,       0,
     {0},                                         0x0,        0x0},
};


/*
 * Sub Menu used for "FPGA  test -> FPGA submenu test"
 */
submenu_xtable_t fpga_submenu_table[] = {
    {"FPGA Utility",  
     (PFT) nutella_fpga_utils,      FALSE,
     0, (type_t(*)())0,                0,
     (type_t(*)())nutella_fpga_utils,   TRUE},

    {"FPGA Register Test",
     (PFT) diag_fpga_reg_test, FALSE,
     MF_CONTINUOUS | MF_DOGRP | MF_DOALL | MF_SHOW_ERRCOUNT,
     (type_t(*)())0, 0, (PFT) 0, 0},
    
     {"FPGA Function Test",  
     (PFT) nutella_fpga_function_test,      TRUE,
     MF_CONTINUOUS | MF_DOGRP | MF_DOALL | MF_SHOW_ERRCOUNT,
     (type_t(*)())0, 0,
     (type_t(*)())nutella_fpga_function_test,   FALSE},

};

#define FPGA_SUBMENU_TABLE_SIZE (sizeof(fpga_submenu_table) / \
                                     sizeof(submenu_xtable_t))

/*
 * "motherboard test -> fpga test" primary & secondary submenu items (filled in from xtable)
 */
static mitem_t fpga_primary_items[FPGA_SUBMENU_TABLE_SIZE +
                                  MAX_BASE_ITEMS];
static mitem_t fpga_secondary_items[FPGA_SUBMENU_TABLE_SIZE +
                                    MAX_BASE_ITEMS];

menuinfo_t fpga_subtest_menu = {
    "%s Subtest Menu",
    0,                          /* mtparam added by init_empty_menu */
    (PFT) show_endnote,         /* notes missing WICs in combos */
    0,                          /* use generic prompt */
    0,                          /* size (bumped by add_menu_item() */
    fpga_primary_items,
};

menuinfo_t *fpga_submenup = &fpga_subtest_menu;


/*
 * FPGA Utilities
 */
static submenu_xtable_t fpga_utils_tbl[] = {
    {"Show FPGA version",   (type_t(*)())nutella_show_fpga_ver,  0, 0,
     (type_t(*)())0, 0,     (type_t(*)())0, 0},
    {"FPGA register Read",  (type_t(*)())fpga_reg_rd_util,   0, 0,
     (type_t(*)())0, 0,     (type_t(*)())0, 0},
    {"FPGA register Write", (type_t(*)())fpga_reg_wr_util,   0, 0,
     (type_t(*)())0, 0,     (type_t(*)())0, 0},
    {"FPGA I2C scan",       (type_t(*)())fpga_i2c_scan_addr,   0, 0,
     (type_t(*)())0, 0,     (type_t(*)())0, 0},
    {"FPGA I2C reset",      (type_t(*)())fpga_i2c_reset,   0, 0,
     (type_t(*)())0, 0,     (type_t(*)())0, 0},
    {"FPGA registers Dump", (type_t(*)())fpga_reg_dump_util, 0, 0,
     (type_t(*)())0, 0,     (type_t(*)())0, 0},
    {"Program FPGA Golden image without header", (type_t(*)())program_reggio_spi_prom_old, 0, 0,
     (type_t(*)())0, 0, (type_t(*)())0,   0},
    {"Program FPGA Upgrade image with header", (type_t(*)())program_reggio_spi_prom_old, 1, 0,
     (type_t(*)())0, 0, (type_t(*)())0,   0},
    {"Erase FPGA Upgrade image header", (type_t(*)())erase_header_spi_prom_image, 1, 0,
     (type_t(*)())0, 0, (type_t(*)())0,   0},
    {"Set revision and date in the header of FPGA Upgrade image", (type_t(*)())set_date_revision, 1, 0,
     (type_t(*)())0, 0,     (type_t(*)())0, 0},
    {"FPGA set system reset", (type_t(*)())fpga_sys_reset_util, 0, 0,
     (type_t(*)())0, 0,     (type_t(*)())0, 0},
    {"FPGA show reset reason", (type_t(*)())fpga_show_reset_reason_util, 0, 0,
     (type_t(*)())0, 0,     (type_t(*)())0, 0},
    {"FPGA watchdog timer enable", (type_t(*)())fpga_watchdog_timer_enable_util, 0, 0,
     (type_t(*)())0, 0,     (type_t(*)())0, 0},
    {"Dump FPGA register default value", 
     (type_t(*)())fpga_reg_dump_def_util, 0, 0,
     (type_t(*)())0, 0,     (type_t(*)())0, 0}, 
    {"Aikido - Program FPGA SPI PROM image", (type_t(*)())program_reggio_spi_prom_with_mailbox, 0, 0,
     (type_t(*)())0, 0,     (type_t(*)())0, 0}, 
    {"toggle flag : FPGA upgrade flag", (type_t(*)())program_image_update_type, 0, 0,
     (type_t(*)())0, 0,     (type_t(*)())0, 0}, 
    {"FPGA MCERR_N test",  (type_t(*)())fpga_mcerr_test,   0, 0,
     (type_t(*)())is_bootloader_rommon, 0,     (type_t(*)())0, 0},
    {"FPGA CPU_PROCHOT_N test",  (type_t(*)())fpga_cpu_prochot_test,   0, 0,
     (type_t(*)())is_bootloader_rommon, 0,     (type_t(*)())0, 0},
    {"Corrupt the header for testing multi-boot",
     (type_t(*)())fpga_corrupt_header_spi_prom, 0, 0,
     (type_t(*)())0, 0,     (type_t(*)())0, 0}, 

};


#define FPGA_UTILS_TBL_SIZE (sizeof(fpga_utils_tbl) / sizeof(submenu_xtable_t))

/* FPGA Utils items (filled in from xtable) */
static mitem_t fpga_utils_pri_items[FPGA_UTILS_TBL_SIZE + MAX_BASE_ITEMS];
static mitem_t fpga_utils_sec_items[FPGA_UTILS_TBL_SIZE + MAX_BASE_ITEMS];

/* FPGA Utils submenu */
menuinfo_t fpga_utils_menu = {
    "%s Menu",
    0,
    (PFT)show_endnote,
    0,
    0,
    fpga_utils_pri_items,
};
menuinfo_t *fpga_utils_menup = &fpga_utils_menu;

/* FPGA Function test menu */
static submenu_xtable_t fpga_func_test_tbl[] = {
    {"Watch Dog Boot Timer Disable(0x64) Test", (type_t(*)())diag_wdt_disable_test, TRUE,
     MF_CONTINUOUS | MF_DOALL | MF_SHOW_ERRCOUNT,
      (type_t(*)())0, 0, (type_t(*)())0, 0}, 
    {"Watch Dog Counter Test", (type_t(*)())diag_wdt_counter_test, TRUE,
     MF_CONTINUOUS | MF_DOALL | MF_SHOW_ERRCOUNT,
      (type_t(*)())0, 0, (type_t(*)())0, 0},
    {"FPGA IOS Watchdog Timer Test", (type_t(*)())fpga_ios_watchdog_timer_reg_test, TRUE,
     MF_CONTINUOUS | MF_DOALL | MF_SHOW_ERRCOUNT,
      (type_t(*)())0, 0, (type_t(*)())0, 0},
    {"FPGA I2C Reset Test", (type_t(*)())fpga_i2c_reset_test, TRUE,
     MF_CONTINUOUS | MF_DOALL | MF_SHOW_ERRCOUNT,
      (type_t(*)())0, 0, (type_t(*)())0, 0},
    {"FPGA PHY Reset Test", (type_t(*)())fpga_phy_reset_test, TRUE,
     MF_CONTINUOUS | MF_DOALL | MF_SHOW_ERRCOUNT,
      (type_t(*)())0, 0, (type_t(*)())0, 0},
    {"FPGA Revision Test", (type_t(*)())fpga_revision_test, TRUE,
     MF_CONTINUOUS | MF_DOALL | MF_SHOW_ERRCOUNT,
      (type_t(*)())0, 0, (type_t(*)())0, 0},
    {"Board type test", (type_t(*)())diag_board_type_test, TRUE,
     MF_CONTINUOUS | MF_DOALL | MF_SHOW_ERRCOUNT,
      (type_t(*)())0, 0, (type_t(*)())0, 0}, 
    {"FPGA Power Status and Control Test", (type_t(*)())fpga_power_status_control_reg_test, TRUE,
     MF_CONTINUOUS | MF_DOALL | MF_SHOW_ERRCOUNT,
      (type_t(*)())0, 0, (type_t(*)())0, 0},
    {"FPGA IOS Watchdog Strobe(0x924) Test", (type_t(*)())fpga_ios_watchdog_strobe_reg_test, TRUE,
     MF_CONTINUOUS | MF_DOALL | MF_SHOW_ERRCOUNT,
      (type_t(*)())0, 0, (type_t(*)())0, 0},
    {"FPGA LPC Bad Address Access(0x688) Test", (type_t(*)())fpga_lpc_bad_addr_access_test, TRUE,
     MF_CONTINUOUS | MF_DOALL | MF_SHOW_ERRCOUNT,
      (type_t(*)())0, 0, (type_t(*)())0, 0},
    {"FPGA Read Default Value Test",
     (type_t(*)())fpga_read_default_value_test, TRUE,
     MF_CONTINUOUS | MF_DOALL | MF_SHOW_ERRCOUNT,
     (type_t(*)())fpga_version_is_more_than_3p0, 0,
     (type_t(*)())fpga_read_default_value_test, 0},
    {"FPGA Scratchpad Register Test",
     (type_t(*)())fpga_scratchpad_register_test, TRUE,
     MF_CONTINUOUS | MF_DOALL | MF_SHOW_ERRCOUNT,
     (type_t(*)())fpga_version_is_more_than_3p0, 0,
     (type_t(*)())fpga_scratchpad_register_test, 0},
    {"FPGA LPC CPU Error Status & Response Register Test",
     (type_t(*)())fpga_lpc_cpu_error_stat_test, TRUE,
     MF_CONTINUOUS | MF_DOALL | MF_SHOW_ERRCOUNT,
     (type_t(*)())fpga_version_is_more_than_3p0, 0,
     (type_t(*)())fpga_lpc_cpu_error_stat_test, 0},
    {"FPGA LPC IRQ 0 Force Test",
     (type_t(*)())fpga_lpc_irq_force_test, LPC_IRQ0,
     MF_CONTINUOUS | MF_DOALL | MF_SHOW_ERRCOUNT,
     (type_t(*)())is_bootloader_rommon, 0,
     (type_t(*)())fpga_lpc_irq_force_test, 0},
    {"FPGA LPC IRQ 6 Force Test",
     (type_t(*)())fpga_lpc_irq_force_test, LPC_IRQ6,
     MF_CONTINUOUS | MF_DOALL | MF_SHOW_ERRCOUNT,
     (type_t(*)())is_bootloader_rommon, 0,
     (type_t(*)())fpga_lpc_irq_force_test, 0},
    {"FPGA IRQ6 CC CP Reset Test",
     (type_t(*)())fpga_eobc_or_packet_interrupt_test, CCCP_RESET,
     MF_CONTINUOUS | MF_DOALL | MF_SHOW_ERRCOUNT,
     (type_t(*)())is_bootloader_rommon, 0,
     (type_t(*)())fpga_eobc_or_packet_interrupt_test, 0},
    {"FPGA IRQ6 FP CP Reset Test",
     (type_t(*)())fpga_eobc_or_packet_interrupt_test, FPCP_RESET,
     MF_CONTINUOUS | MF_DOALL | MF_SHOW_ERRCOUNT,
     (type_t(*)())is_bootloader_rommon, 0,
     (type_t(*)())fpga_eobc_or_packet_interrupt_test, 0},
    {"FPGA IRQ6 FP Reset Test",
     (type_t(*)())fpga_eobc_or_packet_interrupt_test, FP_RESET,
     MF_CONTINUOUS | MF_DOALL | MF_SHOW_ERRCOUNT,
     (type_t(*)())is_bootloader_rommon, 0,
     (type_t(*)())fpga_eobc_or_packet_interrupt_test, 0},
    {"FPGA LPC Reset Control Register Test",
     (type_t(*)())fpga_lpc_reset_ctl_reg_test, TRUE,
     MF_CONTINUOUS | MF_DOALL | MF_SHOW_ERRCOUNT,
     (type_t(*)())fpga_version_is_more_than_3p0, 0,
     (type_t(*)())fpga_lpc_reset_ctl_reg_test, 0},
    {"FPGA LPC Debug Control/Status Register Test",
     (type_t(*)())fpga_lpc_debug_ctl_stat_reg_test, TRUE,
     MF_CONTINUOUS | MF_DOALL | MF_SHOW_ERRCOUNT,
     (type_t(*)())fpga_version_is_more_than_3p0, 0,
     (type_t(*)())fpga_lpc_debug_ctl_stat_reg_test, 0},
    {"FPGA LPC Flash LED Control Register Test",
     (type_t(*)())fpga_lpc_flash_led_ctl_reg_test, TRUE,
     MF_CONTINUOUS | MF_DOALL | MF_SHOW_ERRCOUNT,
     (type_t(*)())fpga_version_is_more_than_3p0, 0,
     (type_t(*)())fpga_lpc_flash_led_ctl_reg_test, 0},
    {"FPGA LPC Chassis Management Control Register Test",
     (type_t(*)())fpga_lpc_chassis_manage_ctl_reg_test, TRUE,
     MF_CONTINUOUS | MF_DOALL | MF_SHOW_ERRCOUNT,
     (type_t(*)())fpga_version_is_more_than_3p0, 0,
     (type_t(*)())fpga_lpc_chassis_manage_ctl_reg_test, 0},
    {"FPGA LPC Alarm Management Control Register Test",
     (type_t(*)())fpga_lpc_alarm_manage_ctl_reg_test, TRUE,
     MF_CONTINUOUS | MF_DOALL | MF_SHOW_ERRCOUNT,
     (type_t(*)())fpga_version_is_more_than_3p0, 0,
     (type_t(*)())fpga_lpc_alarm_manage_ctl_reg_test, 0},
};
 
#define FPGA_FUNC_TEST_TBL_SIZE (sizeof(fpga_func_test_tbl) / sizeof(submenu_xtable_t))

static mitem_t fpga_func_test_pri_items[FPGA_FUNC_TEST_TBL_SIZE + MAX_BASE_ITEMS];
static mitem_t fpga_func_test_sec_items[FPGA_FUNC_TEST_TBL_SIZE + MAX_BASE_ITEMS];

menuinfo_t fpga_func_test_menu = {
    "%s Menu",
    0,
    (PFT)show_endnote,
    0,
    0,
    fpga_func_test_pri_items,
};
menuinfo_t *fpga_func_test_menup = &fpga_func_test_menu;

/*******************************************************************************
 *
 * Function   : build_fpga_test_menu
 * Description: build fpga test submenu 
 * Inputs     : Test/Menu 
 * Outputs    : PASSED
 *
 *******************************************************************************
 */
int build_fpga_test_menu (boolean mb_temp_test_items_executed)
{
    char *tname = "FPGA Test";
    testname(tname);

    build_primary_submenu(fpga_submenu_table, FPGA_SUBMENU_TABLE_SIZE,
                          "FPGA test SubMenu", &fpga_submenup);
    build_secondary_submenu(fpga_submenu_table, FPGA_SUBMENU_TABLE_SIZE,
                            fpga_secondary_items);
    if (mb_temp_test_items_executed) {
        menu(&fpga_subtest_menu, fpga_secondary_items, 0);
    } else {
        do_all_menu_items(fpga_submenup);
    }
    return (PASSED);
}


/*******************************************************************************
 *
 * Function    : nutella_fpga_utils
 * Description : Function to show NUTELLA FPGA utilities submenu.
 * Inputs      : opt - reserved for future use.
 * Outputs     : PASSED / FAILED
 *
 *******************************************************************************
 */
int nutella_fpga_utils (int opt)
{
    build_primary_submenu(fpga_utils_tbl, FPGA_UTILS_TBL_SIZE,
                          "FPGA Utilities", &fpga_utils_menup);
    build_secondary_submenu(fpga_utils_tbl, FPGA_UTILS_TBL_SIZE,
                            fpga_utils_sec_items);

    menu(fpga_utils_menup, fpga_utils_sec_items, '\0');

    return (PASSED);
}


/*******************************************************************************
 *
 * Function    : nutella_fpga_function_test
 * Description : Function to show NUTELLA FPGA function test submenu.
 * Inputs      : opt - reserved for future use.
 * Outputs     : PASSED / FAILED
 *
 *******************************************************************************
 */
int nutella_fpga_function_test (int opt)
{
    build_primary_submenu(fpga_func_test_tbl, FPGA_FUNC_TEST_TBL_SIZE,
                          "FPGA function test", &fpga_func_test_menup);
    build_secondary_submenu(fpga_func_test_tbl, FPGA_FUNC_TEST_TBL_SIZE,
                            fpga_func_test_sec_items);

    if (opt) {
        do_all_menu_items(&fpga_func_test_menu);
    } else {
        menu(fpga_func_test_menup, fpga_func_test_sec_items, '\0');
    }

    return (PASSED);
}


/******************************************************************************
 *
 * Function: diag_fpga_reg_test
 *
 * Description: Function to test FPGA register R/W
 *
 * Inputs      : None
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/
int diag_fpga_reg_test (void)
{
    char *tname ="FPGA Register";
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
    cterr_add_component("Intel Denverton SOC C3558", "LPC", "FPGA");

    /* Segment 5: register and memory dump */

    /* Segment 6: Platform Environment initialized here*/

    /* Segment 7: Top 3 Debugging Steps */
    cterr_add_debug("Check the LPC interface between host CPU"
                    "and FPGA", "If there is no problem for "
                    "these interfaces replace a new FPGA chip");

    testname(tname);
    prpass(testpass, "%s, ", tname);

    if (fpga_version_is_more_than_2p0() == FALSE) {
        if (register_tests(0, fpga_reg_test_tbl) != PASSED) {
            cterr('f', 0, "FPGA Register test Failed");
            prcomplete(testpass, errcount, (char *)0);
            return (FAILED);
        }

    } else {
        if (register_tests(0, fpga_reg_test_tb2) != PASSED) {
            cterr('f', 0, "FPGA Register test Failed");
            prcomplete(testpass, errcount, (char *)0);
            return (FAILED);
        }
    }
    
    prcomplete(testpass, errcount, (char *)0);

    return (PASSED);
}

/*******************************************************************************
 *
 * Function   : fpga_reg_test_read_fn
 * Description: FPGA register read function for register test.
 * Inputs     : addr   - FPGA register offset
 *              size   - FPGA register size
 *              *buf   - pointer to read buffer
 *              *param - pointer to param
 * Outputs    : PASSED / FAILED
 *
 *******************************************************************************
 */
int fpga_reg_test_read_fn (ulong addr, int size, ulong *buf, void *param)
{
    if (fpga_read_reg((uint)addr, (uint *)buf) != PASSED) {
        printf("%s: Failed to read NUTELLA FPGA Reg(0x%lx).\n",
               __FUNCTION__, addr);
        return (FAILED);
    }
    return (PASSED);
}

/*******************************************************************************
 *
 * Function   : fpga_reg_test_write_fn
 * Description: FPGA register write function for register test.
 * Inputs     : addr   - FPGA register offset
 *              size   - FPGA register size
 *              data   - write in data
 *              *param - pointer to param
 * Outputs    : PASSED / FAILED
 *
 *******************************************************************************
 */
int fpga_reg_test_write_fn (ulong addr, int size, ulong data, void *param)
{   
    if (fpga_write_reg((uint)addr, (uint)data) != PASSED) {
        printf("%s: Failed to write FPGA Reg(0x%lx).\n",
               __FUNCTION__, addr);
        return (FAILED);
    }
    return (PASSED);
}


/*******************************************************************************
 *
 * Function   : nutella_show_fpga_ver
 * Description: Function to show FPGA version.
 *              This is by reading FPGA Revision Reg(0x88C).
 * Inputs     : opt - reserved for future use
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
int nutella_show_fpga_ver (int opt)
{
    uint reg_addr = (uint)FPGA_REV_REG;
    uint fpga_ver = 0;

    if (fpga_read_reg(reg_addr, &fpga_ver) != PASSED) {
        printf("Failed to read FPGA Revision Reg(0x%04X).\n", reg_addr);
        return (FAILED);
    }
    printf("FPGA version: %08X\n", fpga_ver);

    return (PASSED);
}


/*******************************************************************************
 *
 * Function    : fpga_reg_rd_util
 * Description : Utility to read FPGA register.
 * Inputs      : opt - reserved for future use
 * Outputs     : PASSED/FAILED
 *
 *******************************************************************************
 */
int fpga_reg_rd_util (int opt)
{
    uint32_t reg_offset = 0, reg_val = 0;

    reg_offset = gethex_answer("Enter register address (0x0 ~ 0xffff): ",
                               FPGA_REV_REG, 0, 0xffff);

    if (fpga_read_reg(reg_offset, &reg_val) != PASSED) {
        printf("Failed to read FPGA register 0x%04X.\n", reg_offset);
        return (FAILED);
    } else {
        printf("FPGA register(0x%04X) = 0x%08X\n", reg_offset, reg_val);
    }
    return (PASSED);
}



/*******************************************************************************
 *
 * Function    : fpga_reg_wr_util
 * Description : Utility to write FPGA register.
 * Inputs      : opt - reserved for future use
 * Outputs     : PASSED/FAILED
 *
 *******************************************************************************
 */
int fpga_reg_wr_util (int opt)
{
    uint reg_offset = 0, orig_val = 0, reg_val = 0;

    reg_offset = gethex_answer("Enter register address(0x0 ~ 0xffff): ",
                               0, 0, 0xffff);
    if (fpga_read_reg(reg_offset, &orig_val) != PASSED) {
        return (FAILED);
    }
    reg_val = gethex_answer("Enter write-in data(hex): ",
                            orig_val, 0, 0xffffffff);

    if (fpga_write_reg(reg_offset, reg_val) != PASSED) {
        return (FAILED);
    } else {
        printf("Done writing 0x%08X to FPGA register(0x%04X).\n",
               reg_val, reg_offset);
    }
    return (PASSED);
}


/*******************************************************************************
 *
 * Function    : fpga_reg_dump_util
 * Description : Utility to dump FPGA all registers.
 * Inputs      : opt - reserved for future use
 * Outputs     : PASSED/FAILED
 *
 *******************************************************************************
 */
int fpga_reg_dump_util (int opt)
{   
    uint       reg_val = 0;
    reg_info_t *reg_p = 0;
    int        ctr = 0, total_reg_num = 0;

    reg_p = &fpga_reg_dump_tbl[0];
    total_reg_num = (sizeof(fpga_reg_dump_tbl) / sizeof(reg_info_t));

    for (ctr = 0; ctr < total_reg_num; ctr++, reg_p++) {
        reg_val = 0;
        if (fpga_read_reg(reg_p->offset, &reg_val) != PASSED) {
            printf("Failed to read FPGA %s Reg(0x%04X).\n",
                   reg_p->name, reg_p->offset);
            return (FAILED);
        } else {
            printf("%-29s Reg(0x%04X): 0x%08X\n",
                   reg_p->name, reg_p->offset, reg_val);
        }
    }
    return (PASSED);
}

/*******************************************************************************
 *
 * Function    : fpga_reg_dump_def_util
 * Description : Utility to dump FPGA registers and their default value.
 * Inputs      : opt - reserved for future use
 * Outputs     : PASSED/FAILED
 *
 *******************************************************************************
 */
int fpga_reg_dump_def_util (int opt)
{   
    uint       reg_val = 0;
    reg_info_t *reg_p = 0;
    int        ctr = 0, total_reg_num = 0;

    reg_p = &fpga_reg_def_tbl[0];
    total_reg_num = (sizeof(fpga_reg_def_tbl) / sizeof(reg_info_t));

    for (ctr = 0; ctr < total_reg_num; ctr++, reg_p++) {
        reg_val = 0;
        if (fpga_read_reg(reg_p->offset, &reg_val) != PASSED) {
            printf("Failed to read FPGA %s Reg(0x%04X).\n",
                   reg_p->name, reg_p->offset);
            return (FAILED);
        } else {
            printf("%-29s Reg(0x%04X): 0x%08X    0x%08X    0x%08X\n",
                   reg_p->name, reg_p->offset, reg_val, reg_p->mask, reg_p->reset_val);
        }
    }
    return (PASSED);
}

/*******************************************************************************
 *
 * Function    : fpga_vol_margin
 * Description : Utility to do voltage margin
 * Inputs      : Which setting
 * Outputs     : PASSED/FAILED
 *
 *******************************************************************************
 */
int fpga_vol_margin (uint8_t v_status)
{   
    uint       reg_val = 0;

    switch (v_status) {
    case POWER_MARGIN_N:
        reg_val = 0;
        break;
    case POWER_MARGIN_LO:
        reg_val = VCC_MARG_LO_VALUE;
        break;
    case POWER_MARGIN_HI:
        reg_val = VCC_MARG_HI_VALUE;
        break;
    default:
        printf("Wrong Voltage setting\n");
    }

    if (fpga_write_reg(POWER_MARGIN_CTL_REG, reg_val) != PASSED) {
        return (FAILED);
    } else {
        printf("Done writing 0x%08X to FPGA register(0x%04X).\n",
               reg_val, POWER_MARGIN_CTL_REG);
    }

    return (PASSED);
}

/*******************************************************************************
 *
 * Function    : fpga_sys_reset_util
 * Description : Utility to do CPU system reset
 * Inputs      : opt - reserved for future use
 * Outputs     : PASSED/FAILED
 *
 *******************************************************************************
 */
int fpga_sys_reset_util (int opt)
{
    uint32_t get_ans = 0, reg_val = 0;

    printf("Warning!! Do you want to do system reset?\n");
    get_ans = gethex_answer("Yes: 0xFF ", 0, 0, 0xFF);

    if (get_ans == 0xFF) {
        if (fpga_version_is_more_than_2p0() == FALSE) {
            if (fpga_read_reg(FPGA_EXTER_DEV_RST_REG, &reg_val) != PASSED) {
                printf("Failed to read FPGA register 0x%04X.\n", FPGA_EXTER_DEV_RST_REG);
                return (FAILED);
            }

            reg_val |= EXT_CPU_SYS_RESET;

            if (fpga_write_reg(FPGA_EXTER_DEV_RST_REG, reg_val) != PASSED) {
                printf("Failed to write FPGA register 0x%04X.\n", FPGA_EXTER_DEV_RST_REG);
                return (FAILED);
            }
        } else {
            /* According FPGA spec write 0x94CB2F01 to RSMRST CPU */
            reg_val = 0x94CB2F01;
            if (fpga_write_reg(CEDGE_LPC_SCRATCHPAD_REG, reg_val) != PASSED) {
                printf("Failed to write FPGA register 0x%04X.\n", CEDGE_LPC_SCRATCHPAD_REG);
                return (FAILED);
            }
        }
    }
    return (PASSED);
}

/*******************************************************************************
 *
 * Function    : fpga_show_reset_reason_util
 * Description : Utility to show reset reason
 * Inputs      : opt - reserved for future use
 * Outputs     : PASSED/FAILED
 *
 *******************************************************************************
 */
int fpga_show_reset_reason_util (int opt)
{
    uint32_t reg_val = 0, reg_offset = 0;

    printf("Show reset reason\n");

    if (fpga_version_is_more_than_2p0() == FALSE) {
        reg_offset = FPGA_RESET_REASON_REG;
    } else {
        reg_offset = CEDGE_FPGA_RESET_REASON_REG;
    }

    if (fpga_read_reg(reg_offset, &reg_val) != PASSED) {
        printf("Failed to read FPGA register 0x%04X.\n", reg_offset);
        return (FAILED);
    }
    printf("Reason(0x%x): ", reg_val);

    switch (reg_val) {
    case (REASON_POWER_ON_RESET):
        printf("Power-On Reset\n");
        break;

    case (REASON_SOFTWARE_REQUEST_RESET):
        printf("Software requested reset by CPU\n");
        break;

    case (REASON_CEDGE_BOARD_RESET):
        printf("Board Reset (LPC Power Cycle Register)\n");
        break;
    
    case (REASON_CPU_THERMAL_RESET):
        if (fpga_version_is_more_than_2p0() == FALSE) {
            printf("CPU reset from Thermal Trip\n");
            
            /* Write 1 to bit 4 to clear (W1C) */
            reg_val |= REASON_CPU_THERMAL_RESET;
            if (fpga_write_reg(reg_offset, reg_val) != PASSED) {
                printf("Failed to write FPGA register 0x%04X.\n", reg_offset);
                return (FAILED);
            }
        } else {
            printf("Flash Reset\n");
        }
        break;

    case (REASON_BOOT_FAIL):
        printf("Boot fail\n");
        break;

    case (REASON_IOS_WATCHDOG_TIMEOUT):
        printf("IOS Watchdog Timeout\n");
        break;

    case (REASON_CEDGE_CPU_THERMAL_RESET):
        printf("CPU reset from Thermal Trip\n");
        break;
    
    case (REASON_CEDGE_CATASTROPHIC_ERROR_RESET):
        if (fpga_version_is_more_than_2p0() == FALSE) {
            printf("Unknow reset reason, %x\n", reg_val);
            
        } else {
            printf("Catastrophic ERROR Reset\n");
            
        }
        break;
    
    case (REASON_INTEL_RESET_REQ_SOFT_RESET):
        printf("Intel Reset Request (PLTRST) - Soft Reset\n");
        break;

    case (REASON_INTEL_POWER_CYCLE_REQ_HARD_RESET):
        printf("Intel Reset Cycle Request (SLP3, SLP_S45, PLTRST) - Hard Reset\n");
        break;

    case (REASON_BOOT_TIMER_TIMEOUT):
        printf("Boot Timer Timeout\n");
        break;

    case (REASON_SHORT_PRESS):
        printf("Short Press (<10 second)\n");
        break;
    
    case (REASON_LONG_PRESS):
        printf("Long Press (>10 second)\n");
        break;
    
    default:
        printf("Unknow reset reason, %x\n", reg_val);
        break;
    }


    return (PASSED);
}

/*******************************************************************************
 *
 * Function    : fpga_watchdog_timer_counting
 * Description : FPGA watchdog timer counting
 * Inputs      : none
 * Outputs     : none
 *
 *******************************************************************************
 */
void fpga_watchdog_timer_counting (void)
{
    uint32_t reg_val = 0, reg_addr = 0;
    uint32_t time_val =0; 

    if (fpga_version_is_more_than_2p0() == FALSE) {
        reg_addr = FPGA_IOS_WATCHDOG_TIMER_REG;
    } else {
        reg_addr = CEDGE_FPGA_IOS_WATCHDOG_TIMER_REG;
    }

    while (fpga_watchdog_force_stop == FALSE ) {
        if (fpga_read_reg(reg_addr, &reg_val) != PASSED) {
            printf("Failed to read FPGA register 0x%04X.\n", FPGA_IOS_WATCHDOG_TIMER_REG);
        }
	 
	    time_val = ( reg_val & FPGA_IOS_WDT_COUNT_MASK)/1000;
	    if (time_val < TIMEVALUE_FOR_WATCHDOG && time_val > 0 ) {
	        prpass(testpass, "%u seconds for watchdog timeout and reset.", time_val);
	    } else if ( time_val == 0 ) {
	        //Timeout and break;
	        break;
	    }
	    msleep(500);
    }

    if (fpga_watchdog_force_stop == FALSE) {
        printf("\nWatchdog timeout!!! If system not reset, please check with FPGA firmware\n");
    }
    
}

/*******************************************************************************
 *
 * Function    : fpga_watchdog_timer_enable_util
 * Description : Utility to enalbe FPGA watchdog timer
 * Inputs      : opt - reserved for future use
 * Outputs     : PASSED/FAILED
 *
 *******************************************************************************
 */
int fpga_watchdog_timer_enable_util (int opt)
{

    uint32_t get_ans = 0, reg_addr = 0;
    pthread_t watchdog_thread;
    void *ret;

    if (fpga_version_is_more_than_2p0() == FALSE) {
        reg_addr = FPGA_IOS_WATCHDOG_TIMER_REG;
    } else {
        reg_addr = CEDGE_FPGA_IOS_WATCHDOG_TIMER_REG;
    }

    printf("Warning!!System will reset after enable watchdog timer 60 seconds.\n");
    printf("Do you want to enable watchdog timer?\n");
    get_ans = gethex_answer("Yes: 0xFF ", 0, 0, 0xFF);
	
    if (get_ans == 0xFF) {

        printf("Do you want to stop watchdog timer? Please input y to confirm.\n");		
        fpga_watchdog_force_stop = FALSE;	
        if (pthread_create(&watchdog_thread, NULL, (void *)fpga_watchdog_timer_counting, NULL)) {
            cterr('f',0, "pthread_create failed");
            return (FAILED);
        }
		
	    if (fpga_write_reg(reg_addr, FPGA_IOS_WDT_ENABLE_KEY) != PASSED) {
            printf("Failed to write FPGA register 0x%04X.\n", reg_addr);
	        fpga_watchdog_force_stop = TRUE;
            return (FAILED);
        }
	 
        while (getc_answer("", "yn", 'n') != 'y');

        if (fpga_write_reg(reg_addr, FPGA_IOS_WDT_DISABLE_KEY) != PASSED) {
            printf("Failed to write FPGA register 0x%04X.\n", reg_addr);
	        fpga_watchdog_force_stop = TRUE;
            return (FAILED);
        }
		
	 fpga_watchdog_force_stop = TRUE;
	 pthread_join(watchdog_thread, &ret);
	 printf("Watchdog timer stop!\n");
	 
    }

    return (PASSED);
}

/******************************************************************************
 *
 * Function: fpga_i2c_reset_test
 *
 * Description: Function to test FPGA I2C reset test
 *
 * Inputs      : None
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/
int fpga_i2c_reset_test (void) 
{
    char *tname ="FPGA I2C reset";
    int dummy = 0;

    testname(tname);
    prpass(testpass, "%s, ", tname);

    printf("\nReset FPGA I2C\n");

    /* Reset the FPGA I2C */
    fpga_reset_api(FPGA_INT_DEV_RST_REG, INT_I2C_RESET, TRUE,
                   WAITTIME_5_MS);

    if (fpga_i2c_scan_addr(dummy) == PASSED) {
        cterr('f', 0, "FPGA I2C reset test Failed");
        prcomplete(testpass, errcount, (char *)0);
        return (FAILED);
    }

    printf("\nUn-Reset FPGA I2C\n");
    /* un-reset the FPGA I2C */
    fpga_reset_api(FPGA_INT_DEV_RST_REG, INT_I2C_RESET, FALSE,
                   WAITTIME_5_MS);

    if (fpga_i2c_scan_addr(dummy) == FAILED) {
        cterr('f', 0, "FPGA I2C un-reset test Failed");
        prcomplete(testpass, errcount, (char *)0);
        return (FAILED);
    }


    return (PASSED);
}

/******************************************************************************
 *
 * Function: fpga_revesion_test
 *
 * Description: Function to test FPGA revision test
 *
 * Inputs      : None
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/
int fpga_revision_test (void) 
{
    char *tname ="FPGA Revision";
    uint master_revision_data = 0;
    uint revision_data = 0;
    unsigned char master_rev_major_data = 0, master_rev_minor_data = 0;
    unsigned char rev_major_data = 0, rev_minor_data = 0;
    uint32_t mas_reg_addr = 0, rev_reg_addr = 0;

    testname(tname);
    prpass(testpass, "%s, ", tname);

    if (fpga_version_is_more_than_2p0() == FALSE) {
        mas_reg_addr = FPGA_MASTER_REV_REG;
        rev_reg_addr = FPGA_REV_REG;
    } else {
        mas_reg_addr = CEDGE_FPGA_MASTER_REV_REG;
        rev_reg_addr = CEDGE_FPGA_REV_REG;
    }

old_revision:

    fpga_read_reg(mas_reg_addr, &master_revision_data);
    printf("\nMaster Revision(0x%x) val: 0x%x\n", mas_reg_addr, master_revision_data);
    fpga_read_reg(rev_reg_addr, &revision_data);
    printf("Revision(0x%x) val: 0x%x\n", rev_reg_addr, revision_data);

    /* Check debug bit */
    printf("1. Check Debug (bit 23)\n");
    if (master_revision_data & MASTER_FPGA_DEBUG_MASK) {
        cterr('f', 0, "FPGA debug bit test Failed");
        prcomplete(testpass, errcount, (char *)0);
        return (FAILED);
    }

    /* Compare Master revision and revision */
    printf("2. Compare Master revision(0x%x) and revision(0x%x)\n",
           mas_reg_addr, rev_reg_addr);
    master_rev_major_data = ((master_revision_data & MASTER_FPGA_MAJOR_REV_MASK) >> 
                              MASTER_FPGA_MAJOR_REV_SHIFT);
    master_rev_minor_data = ((master_revision_data & MASTER_FPGA_MINOR_REV_MASK) >>
                              MASTER_FPGA_MINOR_REV_SHIFT);
    rev_major_data = ((revision_data & FPGA_MAJOR_REV_MASK) >>
                              FPGA_MAJOR_REV_SHIFT);
    rev_minor_data = (revision_data & FPGA_MINOR_REV_MASK);

    printf("Master Revision Major: %x, Minor: %x\n", master_rev_major_data,
            master_rev_minor_data);
    printf("Revision Major: %x, Minor: %x\n", rev_major_data, rev_minor_data);

    if ((master_rev_major_data != rev_major_data) || 
        (master_rev_minor_data != rev_minor_data)) {
        cterr('f', 0, "FPGA revision test Failed");
        prcomplete(testpass, errcount, (char *)0);
        return (FAILED);
    }

    /* For cEdge FPGA, it should also check Master FPGA Revision Register (0x884)
     * and FPGA Revision Register (0x88C).*/
    if ((fpga_version_is_more_than_2p0() == TRUE) &&
        (mas_reg_addr == CEDGE_FPGA_MASTER_REV_REG)) {
        mas_reg_addr = FPGA_MASTER_REV_REG;
        rev_reg_addr = FPGA_REV_REG;
        goto old_revision;
    } 

    return (PASSED);
}

/*******************************************************************************
 *
 * Function    : fpga_ios_watchdog_timer_reg_test
 * Description : Register test for FPGA IOS Watchdog Timer Register (0x084)
 * Inputs      : opt - reserved for future use
 * Outputs     : PASSED/FAILED
 *
 *******************************************************************************
 */
int fpga_ios_watchdog_timer_reg_test (int opt)
{

    uint32_t reg_val = 0, reg_val_2nd = 0, reg_addr = 0;
    int ix;
    char *tname ="FPGA IOS Watchdog Timer Register";
    testname(tname);
    prpass(testpass, "%s, ", tname);

    if (fpga_version_is_more_than_2p0() == FALSE) {
        reg_addr = FPGA_IOS_WATCHDOG_TIMER_REG;
    } else {
        reg_addr = CEDGE_FPGA_IOS_WATCHDOG_TIMER_REG;
    }

    /* Check if IOS Watchdog Timer Counter is the default value.*/
    if (fpga_read_reg(reg_addr, &reg_val) != PASSED) {
        printf("Failed to read FPGA register 0x%04X.\n", reg_addr);
        cterr('f', 0, "FPGA IOS Watchdog Timer Register Test Failed");
        return (FAILED);
    }

    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("IOS watchdog timer count = %d,\n", reg_val);
    }

    if (reg_val != FPGA_IOS_WDT_DEFAULT_VALUE) {
        printf("Failed to get IOS watchdog timer default value 0x%x, get 0x%x.\n", FPGA_IOS_WDT_DEFAULT_VALUE, reg_val);
        cterr('f', 0, "FPGA IOS Watchdog Timer Register Test Failed");
        return (FAILED);
    }

    /* Test it twice, make sure IOS Watchdog Timer can be triggered normally.*/
    for (ix = 0; ix < FPGA_IOS_WDT_TEST_TIMES; ix++) {

        if (fpga_write_reg(reg_addr, FPGA_IOS_WDT_ENABLE_KEY) != PASSED) {
            printf("Failed to write FPGA register 0x%04X.\n", reg_addr);
            cterr('f', 0, "FPGA IOS Watchdog Timer Register Test Failed");
            return (FAILED);
        }

        msleep(WAITTIME_250_MS);
		
        /* Check if counter is in the expected range 60s ~ 59s. */
        if (fpga_read_reg(reg_addr, &reg_val) != PASSED) {
            printf("Failed to read FPGA register 0x%04X.\n", reg_addr);
            cterr('f', 0, "FPGA IOS Watchdog Timer Register Test Failed");
            return (FAILED);
        }

        if ((NVRAM)->diagflag & D_VERBOSE) {
            printf("IOS watchdog timer count = %d,\n", reg_val);
        }

        if (!((reg_val < FPGA_IOS_WDT_DEFAULT_VALUE) && (reg_val >= WAITTIME_59000_MS))) {
            printf("Get timer counter 0x%x out of expected range.\n", reg_val);
            cterr('f', 0, "FPGA IOS Watchdog Timer Register Test Failed");
	     return (FAILED);
        }

        msleep(WAITTIME_1000_MS);

        /* Disable IOS Watchdog Timer */
        if (fpga_write_reg(reg_addr, FPGA_IOS_WDT_DISABLE_KEY) != PASSED) {
            printf("Failed to write FPGA register 0x%04X.\n", reg_addr);
            cterr('f', 0, "FPGA IOS Watchdog Timer Register Test Failed");
            return (FAILED);
        }

        msleep(WAITTIME_250_MS);

        /* Read IOS Watchdog Timer Counter twice. Make sure the IOS Watchdog Timer is not counting.  */
        if (fpga_read_reg(reg_addr, &reg_val) != PASSED) {
            printf("Failed to read FPGA register 0x%04X.\n", reg_addr);
            cterr('f', 0, "FPGA IOS Watchdog Timer Register Test Failed");
            return (FAILED);
        }
        
        msleep(WAITTIME_250_MS);

        if (fpga_read_reg(reg_addr, &reg_val_2nd) != PASSED) {
            printf("Failed to read FPGA register 0x%04X.\n", reg_addr);
            cterr('f', 0, "FPGA IOS Watchdog Timer Register Test Failed");
            return (FAILED);
        }

        if ((NVRAM)->diagflag & D_VERBOSE) {
           printf("IOS watchdog count 1st = %d, 2nd = %d\n", reg_val, reg_val_2nd);
        }

        if (reg_val != reg_val_2nd) {
            printf("Watchdog timer is still counting after disable it, please check. Get counter 1st time:0x%x, get counter 2nd time:0x%x\n", reg_val, reg_val_2nd);
            cterr('f', 0, "FPGA IOS Watchdog Timer Register Test Failed");
            return (FAILED);
        }

    }

    prcomplete(testpass, errcount, (char *)0);
    return (PASSED);

}

/*******************************************************************************
 *
 * Function    : fpga_ios_watchdog_strobe_reg_test
 * Description : Register test for FPGA IOS Watchdog Strobe Register (0x924)
 * Inputs      : opt - reserved for future use
 * Outputs     : PASSED/FAILED
 *
 *******************************************************************************
 */
int fpga_ios_watchdog_strobe_reg_test (int opt)
{

    uint32_t reg_val = 0, reg_val_2nd = 0, reg_addr = 0;
    char *tname ="FPGA IOS Watchdog Strobe Register";
    testname(tname);
    prpass(testpass, "%s, ", tname);

    if (fpga_version_is_more_than_2p0() == FALSE) {
        reg_addr = FPGA_IOS_WATCHDOG_TIMER_REG;
    } else {
        reg_addr = CEDGE_FPGA_IOS_WATCHDOG_TIMER_REG;
    }

    /* Enable IOS Watchdog Timer */
    if (fpga_write_reg(reg_addr, FPGA_IOS_WDT_ENABLE_KEY) != PASSED) {
        printf("Failed to write FPGA register 0x%04X.\n", reg_addr);
        cterr('f', 0, "FPGA IOS Watchdog Strobe Register Test Failed");
        return (FAILED);
    }

    msleep(WAITTIME_2000_MS);

    /* Wait 2 seconds, then read back IOS Watchdog Timer Counter, make sure it is counting. */
    if (fpga_read_reg(reg_addr, &reg_val) != PASSED) {
        printf("Failed to read FPGA register 0x%04X.\n", reg_addr);
        cterr('f', 0, "FPGA IOS Watchdog Strobe Register Test Failed");
        return (FAILED);
    }

    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("IOS watchdog timer count = %d,\n", reg_val);
    }

    if (!(reg_val <= (FPGA_IOS_WDT_DEFAULT_VALUE - WAITTIME_1500_MS))) {
        printf("Failed to get unexpected IOS watchdog timer counting.\n");
        cterr('f', 0, "FPGA IOS Watchdog Strobe Register Test Failed");
        return (FAILED);
    }

    /* Strobe IOS Watchdog Timer. */
    if (fpga_write_reg(FPGA_WATCHDOG_REG, FPGA_IOS_WDT_STROBE_VALUE) != PASSED) {
        printf("Failed to write FPGA register 0x%04X.\n", FPGA_WATCHDOG_REG);
        cterr('f', 0, "FPGA IOS Watchdog Strobe Register Test Failed");
        return (FAILED);
    }

    msleep(WAITTIME_250_MS);

    /* Check if counter is in the expected range 60s ~ 59s. */
    if (fpga_read_reg(reg_addr, &reg_val) != PASSED) {
        printf("Failed to read FPGA register 0x%04X.\n", reg_addr);
        cterr('f', 0, "FPGA IOS Watchdog Strobe Register Test Failed");
        return (FAILED);
    }

    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("IOS watchdog timer count = %d,\n", reg_val);
    }

    if (!((reg_val < FPGA_IOS_WDT_DEFAULT_VALUE) && (reg_val >= WAITTIME_59000_MS))) {
        printf("Get timer counter 0x%x out of expected range.\n", reg_val);
        cterr('f', 0, "FPGA IOS Watchdog Strobe Register Test Failed");
        return (FAILED);
    }

    /* Disable IOS Watchdog Timer */
    if (fpga_write_reg(reg_addr, FPGA_IOS_WDT_DISABLE_KEY) != PASSED) {
        printf("Failed to write FPGA register 0x%04X.\n", reg_addr);
        cterr('f', 0, "FPGA IOS Watchdog Strobe Register Test Failed");
        return (FAILED);
    }

    /* Strobe IOS Watchdog Timer. */
    if (fpga_write_reg(FPGA_WATCHDOG_REG, FPGA_IOS_WDT_STROBE_VALUE) != PASSED) {
        printf("Failed to write FPGA register 0x%04X.\n", FPGA_WATCHDOG_REG);
        cterr('f', 0, "FPGA IOS Watchdog Strobe Register Test Failed");
        return (FAILED);
    }

    msleep(WAITTIME_250_MS);

    /* Read IOS Watchdog Timer Counter twice. Make sure the IOS Watchdog Timer is not counting.  */
    if (fpga_read_reg(reg_addr, &reg_val) != PASSED) {
        printf("Failed to read FPGA register 0x%04X.\n", reg_addr);
        cterr('f', 0, "FPGA IOS Watchdog Strobe Register Test Failed");
        return (FAILED);
    }

    msleep(WAITTIME_250_MS);

    if (fpga_read_reg(reg_addr, &reg_val_2nd) != PASSED) {
        printf("Failed to read FPGA register 0x%04X.\n", reg_addr);
        cterr('f', 0, "FPGA IOS Watchdog Strobe Register Test Failed");
        return (FAILED);
    }

    if ((NVRAM)->diagflag & D_VERBOSE) {
       printf("IOS watchdog count 1st = %d, 2nd = %d\n", reg_val, reg_val_2nd);
    }

    if (reg_val != reg_val_2nd) {
        printf("Watchdog timer is still counting after disable it, please check. Get counter 1st time:0x%x, get counter 2nd time:0x%x\n", reg_val, reg_val_2nd);
        cterr('f', 0, "FPGA IOS Watchdog Strobe Register Test Failed");
        return (FAILED);
    }

    prcomplete(testpass, errcount, (char *)0);
    return (PASSED);

}

/*******************************************************************************
 *
 * Function    : fpga_phy_reset_test
 * Description : Register test for External Device Reset Register (0x804) - Bit 21 PHY Reset
 * Inputs      : opt - reserved for future use
 * Outputs     : PASSED/FAILED
 *
 *******************************************************************************
 */
int fpga_phy_reset_test (int opt)
{

    uint reg_val = 0;
    int portnum;
    char *tname ="FPGA External Device Reset Register - PHY Reset";
    testname(tname);
    prpass(testpass, "%s, ", tname);

    for (portnum = NUTELLA_1543_P0_PHY_ADDR; portnum <= NUTELLA_1543_P3_PHY_ADDR; portnum++) {
        
        /* Write GE PHY 0 Port 0-3 register 22 to 0, set page 0. */
        if (dnv_write_phy_reg(portnum, portnum, NUTELLA_1543_PAGE_REG, NUTELLA_1543_PAGE_0) != PASSED) {
            printf("Failed to write PHY Port %d register.\n", portnum);
            cterr('f', 0, "FPGA External Device Reset Register - PHY Reset Test Failed");
            return (FAILED);
        }
        
        msleep(WAITTIME_250_MS);
        
        /* Read GE PHY 0 Port 0-3 Page 0 register 2 to get PHYID. */
        if (diag_gephy_smi_rd(portnum, NUTELLA_1543_PAGE_0, NUTELLA_1543_PHYID_REG, &reg_val) != PASSED) {
            printf("Failed to read PHY0 Port %d register.\n", portnum);
            cterr('f', 0, "FPGA External Device Reset Register - PHY Reset Test Failed");
            return (FAILED);
        }
        
        /* Check if get correct PHYID before phy reset, make sure that phy works normal.*/
        if (reg_val != NUTELLA_1543_PHYID) {
            printf("Read PHY0 Port  %d PHYID failed.\n", portnum);
            cterr('f', 0, "FPGA External Device Reset Register - PHY Reset Test Failed");
            return (FAILED);
        }
    }

    /* GE PHY 0 Reset. */
    if (fpga_reset_api(FPGA_EXTER_DEV_RST_REG, FPGA_GEWAN0_RESET, TRUE,
                       WAITTIME_20_MS) != PASSED) {
        cterr('f', 0, "%s: Failed to put GE0 PHY in Reset.\n", __FUNCTION__);
        return (FAILED);
    }
    
    /* Read GE PHY 0 register 22. */
    for (portnum = NUTELLA_1543_P0_PHY_ADDR; portnum <= NUTELLA_1543_P3_PHY_ADDR; portnum++) {
        if (diag_gephy_smi_rd(portnum, NUTELLA_1543_PAGE_0, NUTELLA_1543_PAGE_REG, &reg_val) != PASSED) {
            printf("Failed to read PHY Port  %d register.\n", portnum);
            cterr('f', 0, "FPGA External Device Reset Register - PHY Reset Test Failed");
            return (FAILED);
        }

        if ((NVRAM)->diagflag & D_VERBOSE) {
            printf("Read PHY register value: 0x%x\n", reg_val);
        }
            
        /* Expect to get 0xFFFF data after phy reset.*/
        if (reg_val != NUTELLA_1543_PAGE_REG_VAL_FFFF) {
            printf("Still read PHY Port %d register after phy reset.\n", portnum);
            cterr('f', 0, "FPGA External Device Reset Register - PHY Reset Test Failed");
            return (FAILED);
        }
    }

    /* Since GE PHY has been reset, need to re-init GE PHY.*/
    if (diag_gephy_init() != PASSED) {
        printf("Failed to initialize GE PHY.\n");
        cterr('f', 0, "FPGA External Device Reset Register - PHY Reset Test Failed");
        return (FAILED);
    }

    prcomplete(testpass, errcount, (char *)0);
    return (PASSED);

}

/*******************************************************************************
 *
 * Function    : fpga_power_status_control_reg_test
 * Description : Register test for Power Status and Control Register (0x910)
 * Inputs      : opt - reserved for future use
 * Outputs     : PASSED/FAILED
 *
 *******************************************************************************
 */
int fpga_power_status_control_reg_test (int opt)
{

    uint32_t reg_val = 0;
    char *tname ="FPGA Power Status and Control Register";
    testname(tname);
    prpass(testpass, "%s, ", tname);

    if (fpga_read_reg(FPGA_PWR_STAT_REG, &reg_val) != PASSED) {
        printf("Failed to read FPGA register 0x%04X.\n", FPGA_PWR_STAT_REG);
        cterr('f', 0, "FPGA Power Status and Control Register Test Failed");
        return (FAILED);
    }

    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("Read Power Status and Control Register = 0x%x.\n", reg_val);
    }

    reg_val = (reg_val & FPGA_PLATFORM_PWR_STATUS_BIT);


    /* Check Power Status and Control BIT 1, platform power status. */
    if (reg_val != FPGA_PLATFORM_PWR_STATUS_BIT) {
        printf("Some power rails are not in a good state, please check.\n");
        cterr('f', 0, "FPGA Power Status and Control Register Test Failed");
        return (FAILED);
    }

    if (has_lte_sku()) {
        /* Out of reset LTE */
        if (fpga_reset_api(FPGA_EXTER_DEV_RST_REG, EXT_PRI_LTE_RESET, FALSE, 
                           WAITTIME_20_MS) != PASSED) {
            cterr('f', 0, "%s:Failed to release LTE from Reset.\n",__FUNCTION__);
        }

        /* Power on LTE module. */
        if (diag_lte_pwr_on(TRUE) != PASSED) {
            printf("Powering up LTE Module fails\n");
            cterr('f', 0, "FPGA Power Status and Control Register Test Failed");
            return (FAILED);
        }

        /* Check if SWI tty USB device comes up */
        printf("Detecting ttyUSB device...");
        if (diag_lte_is_usb_found(TRUE, LTE_USB_DETECT_TOUT) == FALSE) {
            cterr('f', 0, "LTE USB Device can't be found");
            return (FAILED);
        }

        if (fpga_read_reg(FPGA_PWR_STAT_REG, &reg_val) != PASSED) {
            printf("Failed to read FPGA register 0x%04X.\n", FPGA_PWR_STAT_REG);
            cterr('f', 0, "FPGA Power Status and Control Register Test Failed");
            return (FAILED);
        }

        if ((NVRAM)->diagflag & D_VERBOSE) {
            printf("Read Power Status and Control Register = 0x%x.\n", reg_val);
        }

        reg_val = (reg_val & FPGA_LTE_3V7_BIT);

        /* Check Power Status and Control BIT 0, LTE 3V7*/
        if (reg_val != FPGA_LTE_3V7_BIT) {
            printf("The LTE +3.7V rail is not good, please check.\n");
            cterr('f', 0, "FPGA Power Status and Control Register Test Failed");
            return (FAILED);
        }

         /* Power down LTE module. */
        if (diag_lte_pwr_on(FALSE) != PASSED) {
            printf("Powering down LTE Module fails\n");
            cterr('f', 0, "FPGA Power Status and Control Register Test Failed");
            return (FAILED);
        }
    }

    prcomplete(testpass, errcount, (char *)0);
    return (PASSED);

}

/*******************************************************************************
 *
 * Function    : diag_wdt_disable_test
 * Description : Test for disable FPGA watchdog timer
 * Inputs      : none
 * Outputs     : PASSED/FAILED
 *
 *******************************************************************************
 */
int diag_wdt_disable_test (void)
{
    uint32_t reg_val = 0;
    char *tname ="FPGA Disable FPGA Watchdog Timer";
    testname(tname);
    prpass(testpass, "%s, ", tname);

    /* Write register(0xD10) to enable Watch Dog boot timer */
    /* 0xABCD0001 is pattern to enable WDT */
    reg_val = FOXCONN_WDT_ENABLE_PATTERN;
    fpga_write_reg(FPGA_WDT_ENABLE_REG, reg_val);

    /* Waiting for WDT count down to 295000ms */
    msleep(WAITTIME_5000_MS);
    /* Read back the register(0x64) to verify WDT is count down */
    fpga_read_reg(FPGA_WATCHDOG_BOOT_TIMER, &reg_val);
    /* If WDT count down the value should be around 295000ms */
    if (reg_val == 0 || reg_val >= TIME_299000MS) {
        cterr('f', 0, "%s: FPGA timer didn't count down\n", __FUNCTION__);
        return(FAILED);
    }
    
    /* Write pattern 0xBD000000 to reg 0x64 to disable WDT */
    reg_val = WDT_DISABLE_PATTERN;
    fpga_write_reg(FPGA_WATCHDOG_BOOT_TIMER, reg_val);

    /* Read back the register(0x64) to verify WDT is zero for disable */
    msleep(WAITTIME_2000_MS);
    fpga_read_reg(FPGA_WATCHDOG_BOOT_TIMER, &reg_val);
    if (reg_val != 0) {
        cterr('f', 0, "%s: FPGA WDT disable failed\n", __FUNCTION__);
        return(FAILED);
    }

    /* Write pattern 0xAA000000 to reg 0x64 to verify there isn't 
     * any pattern can enable WDT*/
    reg_val = WDT_DISABLE_TEST_PATTERN;
    fpga_write_reg(FPGA_WATCHDOG_BOOT_TIMER, reg_val);

    /* Read back the register(0x64) to verify WDT is zero for disable  */
    msleep(WAITTIME_2000_MS);
    fpga_read_reg(FPGA_WATCHDOG_BOOT_TIMER, &reg_val);
    if (reg_val != 0) {
        cterr('f', 0, "%s: There is a pattern can enable FPGA WDT again\n", __FUNCTION__);
        return(FAILED);
    }

    return (PASSED);
}

/*******************************************************************************
 *
 * Function    : diag_wdt_counter_test
 * Description : Test for FPGA watchdog timer counter test
 * Inputs      : none
 * Outputs     : PASSED/FAILED
 *
 *******************************************************************************
 */
int diag_wdt_counter_test (void)
{
    uint32_t reg_val = 0;
    char *tname ="FPGA Watchdog Counter Test";
    testname(tname);
    prpass(testpass, "%s, ", tname);
    /* BIOS will disable WDT by default so WDT vaule is zero */
    /* Read back the register(0x64) to verify WDT is zero for disable  */
    fpga_read_reg(FPGA_WATCHDOG_BOOT_TIMER, &reg_val);
    if (reg_val != 0) {
        cterr('f', 0, "%s: FPGA WDT disable failed\n", __FUNCTION__);
        return(FAILED);
    }

    return (PASSED);
}

/*******************************************************************************
 *
 * Function    : diag_board_type_test
 * Description : This test will compared cookie board tpye and FPGA reg
 * Inputs      : none
 * Outputs     : PASSED/FAILED
 *
 *******************************************************************************
 */
int diag_board_type_test (void)
{ 
    char mb_get_pid[64] = {0};
    uint32_t reg_val = 0;
    char *tname ="FPGA Board Type Test";
    testname(tname);
    prpass(testpass, "%s, ", tname);

    /* Get PID */
    platform_get_pid((char *)mb_get_pid);
    /* Get FPGA board type */
    fpga_read_reg(FPGA_BOARD_TYPE_REG, &reg_val);

    /* Check board reg 0x8C0 bit [5:0] is 0x24, 0x25 or 0x26 */
    if (reg_val == FPGA_BTYPE_PRODUCT_SKU_101B) {
        /* Get PID from cookie to check, then print out PID */
        if ((strncmp(mb_get_pid, "ISR1100-4G", strlen("ISR1100-4G")) == 0) ||
            (strncmp(mb_get_pid, "ISR1100X-4G", strlen("ISR1100X-4G")) == 0 )) {
            printf("PID:%s\n", mb_get_pid); 
        } else {
            printf("The PID and FPGA Board type are not match\n");
            printf("PID:%s\n", mb_get_pid); 
            cterr('f', 0, "FPGA  Board Type test Failed");
            return (FAILED);
        }
    } else if (reg_val == FPGA_BTYPE_PRODUCT_SKU_101M) {
        /* Get PID from cookie to check, then print out PID */
        if ((strncmp(mb_get_pid, "ISR1100-4GLTENA", strlen("ISR1100-4GLTENA")) == 0) ||
            (strncmp(mb_get_pid, "ISR1100-4GLTEGB", strlen("ISR1100-4GLTEGB")) == 0)) {
            printf("PID:%s\n", mb_get_pid); 
        } else {
            printf("The PID and FPGA Board type are not match\n");
            printf("PID:%s\n", mb_get_pid); 
            cterr('f', 0, "FPGA  Board Type test Failed");
            return (FAILED);
        }
    } else if (reg_val == FPGA_BTYPE_PRODUCT_SKU_1001) {
        /* Get PID from cookie to check, then print out PID */
        if ((strncmp(mb_get_pid, "ISR1100-6G", strlen("ISR1100-6G")) == 0) ||
            (strncmp(mb_get_pid, "ISR1100X-6G", strlen("ISR1100X-6G")) == 0)) {
            printf("PID:%s\n", mb_get_pid); 
        } else {
            printf("The PID and FPGA Board type are not match\n");
            printf("PID:%s\n", mb_get_pid); 
            cterr('f', 0, "FPGA  Board Type test Failed");
            return (FAILED);
        }
    } else {
        printf("The FPGA Board type are not support\n");
        cterr('f', 0, "FPGA  Board Type test Failed");
        return (FAILED);
    }

    prcomplete(testpass, errcount, (char *)0);
    return (PASSED);
}

/*******************************************************************************
 *
 * Function    : fpga_lpc_bad_addr_access_test
 * Description : Test for LPC Bad Address Access 
 * Inputs      : none
 * Outputs     : PASSED/FAILED
 *
 *******************************************************************************
 */
int fpga_lpc_bad_addr_access_test (void)
{    
    uint32_t reg_val = 0, expected_val;
    char *tname ="FPGA LPC Bad Address Access";
    testname(tname);
    prpass(testpass, "%s, ", tname);

    /* CSCvw90893: For new FPGA image release (V3.3.2),
     *             it redefines default value of address not defined in 0x000~0xFFF,
     *             0x0D00~0x0DF0 and 0x0F00~0x0FF0, change value from 0x0 to 0xDEADBEEF,
     *             so need to make sure after V3.3.2 will get 0xDEADBEEF,
     *             and before V3.2.0 will get 0x0.
     */
    uint master_revision_data = 0, master_rev_minor_data = 0;

    fpga_read_reg((uint)FPGA_MASTER_REV_REG, &master_revision_data);

    master_rev_minor_data = ((master_revision_data & MASTER_FPGA_MINOR_REV_MASK) >> 
                              MASTER_FPGA_MINOR_REV_SHIFT);

    if ((fpga_version_is_more_than_3p0() == TRUE) && 
        (master_rev_minor_data >= CEDGE_FPGA_MAJ_VER_3)) {
        expected_val = 0xDEADBEEF;
    } else {
        expected_val = 0;
    }
    
    /* Read register(0x688) to check it is 0x0/0xDEADBEEF */
    fpga_read_reg(FPGA_LPC_BAD_ADDR_ACCESS_REG, &reg_val);
    if (reg_val != expected_val) {
        cterr('f', 0, "%s : FPGA LPC Bad Address is not 0x%08X, value equals to 0x%08X.\n",
              __FUNCTION__, expected_val, reg_val);
        return (FAILED);
    }

    /* Write register(0x688) to 0xAA55BBCC */
    reg_val |= FPGA_LPC_BAD_ADDR_RANDOM_VALUE;
    if (fpga_write_reg(FPGA_LPC_BAD_ADDR_ACCESS_REG, reg_val) != PASSED) {
        cterr('f', 0, "%s : Failed to write FPGA register 0x%04X.\n", __FUNCTION__,
              FPGA_LPC_BAD_ADDR_ACCESS_REG);
        return (FAILED);
    }
    
    /* Read register(0x688) again to check it's still 0x0/0xDEADBEEF */
    fpga_read_reg(FPGA_LPC_BAD_ADDR_ACCESS_REG, &reg_val);
    if (reg_val != expected_val) {
        cterr('f', 0, "%s : FPGA LPC Bad Address is not 0x%08X, value equals to 0x%08X.\n",
              __FUNCTION__, expected_val, reg_val);
        return (FAILED);
    }
    
    return (PASSED);
}

/*******************************************************************************
 *
 * Function    : fpga_read_default_value_test
 * Description : Verify the default value of registers
 * Inputs      : none
 * Outputs     : PASSED/FAILED
 *
 *******************************************************************************
 */
int fpga_read_default_value_test (void)
{    
    uint32_t reg_val = 0, default_value = 0;
    uint test_registers[] = {PHASE2_FPGA_LPC_BOARD_TYPE_REG,
                             PHASE2_FPGA_LPC_NIOS_VER_REG,
                             PHASE2_FPGA_EXT_PIN_CTL_REG,
                             PHASE2_FPGA_BOARD_TYPE_REG,
                             PHASE2_FPGA_SLAVE_REV_REG,
                             PHASE2_FPGA_NIOS_VER_REG,
                             PHASE2_FPGA_INTR_IRQ0_STAT_REG,
                             PHASE2_FPGA_INTR_IRQ0_MASK_REG,
                             PHASE2_FPGA_LPC_CPU_ERROR_STAT_REG,
                             PHASE2_FPGA_INTR_IRQ6_STAT_REG,
                             PHASE2_FPGA_INTR_IRQ6_MASK_REG,
                             PHASE2_FPGA_CCCP_RST_CTL_REG,
                             PHASE2_FPGA_FPCP_RST_CTL_REG,
                             PHASE2_FPGA_FP_RST_CTL_REG,
                             PHASE2_SECURE_JTAG_STAT_REG};
    int ix;
    char *tname ="FPGA Read Default Value";
    testname(tname);
    prpass(testpass, "%s, ", tname);

    /* Read register to check it is default value */
    for (ix = 0; ix < (sizeof(test_registers) / sizeof(test_registers[0])); ix++) {
        if (fpga_read_reg(test_registers[ix], &reg_val) != PASSED) {
            printf("Failed to read FPGA register 0x%04X.\n",
                   test_registers[ix]);
            return (FAILED);
        }
        switch (ix) {
            case 0:
            case 3:
                default_value = DEFAULT_BOARD_TYPE_REG;
                break;
            case 2:
                default_value = DEFAULT_FPGA_EXT_PIN_CTL_REG;
                break;
            case 7:
                default_value = DEFAULT_IRQ0_MASK_REG;
                break;
            case 8:
                default_value = DEFAULT_LPC_CPU_ERROR_STAT_REG ;
                break;
            case 9:
                default_value = DEFAULT_IRQ6_STAT_REG;
                break;
            case 10:
                default_value = DEFAULT_IRQ6_MASK_REG;
                break;
            case 1:
            case 4:
            case 5:
            case 6:
            case 11:
            case 12:
            case 13:
            case 14:
                default_value = DEFAULT_VALUE_IS_ZERO;
                break;
        }

        if (reg_val != default_value) {
            cterr('f', 0, "%s : FPGA Register(0x%04X) default value "
                  "is not 0x%08X, value equals to 0x%08X.\n",
                  __FUNCTION__, test_registers[ix], default_value, reg_val);
            return (FAILED);
        }
    }
    
    return (PASSED);
}

/*******************************************************************************
 *
 * Function    : fpga_scratchpad_register_test
 * Description : Check all bits are readable and writable in this register.
 * Inputs      : none
 * Outputs     : PASSED/FAILED
 *
 *******************************************************************************
 */
int fpga_scratchpad_register_test (void)
{    
    uint32_t reg_val = 0;
    uint expected[TEST_PATTERN_SIZE];
    
    char *tname ="FPGA Scratchpad Register";
    testname(tname);
    prpass(testpass, "%s, ", tname);

    /* Read register(0x800) to check it is 0x0 */
    if (fpga_read_reg(PHASE2_FPGA_SCRATCHPAD_REG, &reg_val) != PASSED) {
        printf("Failed to read FPGA register 0x%04X.\n",
               PHASE2_FPGA_SCRATCHPAD_REG);
        return (FAILED);
    }
    if (reg_val != DEFAULT_VALUE_IS_ZERO) {
        cterr('f', 0, "%s : FPGA Scratchpad Register is not 0, "
              "value equals to 0x%08X.\n", __FUNCTION__, reg_val);
        return (FAILED);
    }

    /* Write register(0x800) to test pattern, and read back to check */
    if (fpga_test_pattern_test(PHASE2_FPGA_SCRATCHPAD_REG, &expected[0],
                               SAME, DEFAULT_VALUE_IS_ZERO) != PASSED) {
        cterr('f', 0, "%s : Test Pattern failed.\n ", __FUNCTION__);
        return (FAILED);
    }
    
    return (PASSED);
}

/*******************************************************************************
 *
 * Function    : fpga_lpc_cpu_error_stat_test
 * Description : Check all bits are readable and writable in this register.
 * Inputs      : none
 * Outputs     : PASSED/FAILED
 *
 *******************************************************************************
 */
int fpga_lpc_cpu_error_stat_test (void)
{    
    uint32_t reg_val = 0;
    uint test_pattern[] = {LPC_CPU_ERROR_STAT_REG_TEST_PATTERN_1,
                            LPC_CPU_ERROR_STAT_REG_TEST_PATTERN_2,
                            DEFAULT_LPC_CPU_ERROR_STAT_REG};
    int ix;
    char *tname ="FPGA LPC CPU Error & Response Register";
    testname(tname);
    prpass(testpass, "%s, ", tname);

    /* Read register(0x078) to check it is 0x57C00000 */
    if (fpga_read_reg(PHASE2_FPGA_LPC_CPU_ERROR_STAT_REG, &reg_val) != PASSED) {
        printf("Failed to read FPGA register 0x%04X.\n",
               PHASE2_FPGA_LPC_CPU_ERROR_STAT_REG);
        return (FAILED);
    }
    if (reg_val != DEFAULT_LPC_CPU_ERROR_STAT_REG) {
        cterr('f', 0, "%s : FPGA LPC CPU Error & Response Register "
              "is not 0x%08X, value equals to 0x%08X.\n", __FUNCTION__,
              DEFAULT_LPC_CPU_ERROR_STAT_REG, reg_val);
        return (FAILED);
    }

    /* Write register(0x078) to test pattern, and read back to check */
    for (ix = 0; ix < (sizeof(test_pattern) / sizeof(test_pattern[0])); ix++) {
        reg_val = test_pattern[ix];
        if (fpga_write_reg(PHASE2_FPGA_LPC_CPU_ERROR_STAT_REG, reg_val) != PASSED) {
            cterr('f', 0, "%s : Failed to write FPGA register 0x%04X.\n",
                  __FUNCTION__, PHASE2_FPGA_LPC_CPU_ERROR_STAT_REG);
            return (FAILED);
        }

        if (fpga_read_reg(PHASE2_FPGA_LPC_CPU_ERROR_STAT_REG, &reg_val) != PASSED) {
            printf("Failed to read FPGA register 0x%04X.\n",
                   PHASE2_FPGA_LPC_CPU_ERROR_STAT_REG);
            return (FAILED);
        }
        if (reg_val != test_pattern[ix]) {
            cterr('f', 0, "%s : After write FPGA LPC CPU Error & Response Register "
                  "and read it again is not test value 0x%08X, "
                  "value equals to 0x%08X.\n", __FUNCTION__,
                  test_pattern[ix], reg_val);
            return (FAILED);
        }
    }
    return (PASSED);
}

/*******************************************************************************
 *
 * Function    : fpga_lpc_irq_force_test 
 * Description : Test the IRQ interrupt function of LPC Chassis Test Register 
 *               internal registers in the RST_CPLD section of the FPGA. 
 * Inputs      : IRQ number
 * Outputs     : PASSED/FAILED
 *
 *******************************************************************************
 */
int fpga_lpc_irq_force_test (int irq_num)
{
    char *tname = "FPGA LPC IRQ 0 Force";
    char *tname2 = "FPGA LPC IRQ 6 Force";

    if (irq_num == LPC_IRQ0){
        testname("%s", tname);
        prpass(testpass, "%s, ", tname);
    
    } else {
        testname("%s", tname2);
        prpass(testpass, "%s, ", tname2);
    }

    if (toggle_driver_irq_flag(IRQ_ENABLE) == FAILED) {
        return (FAILED);
    }

    if (lpc_irq_force_test(irq_num) == FAILED) {
        cterr('f', 0, "%s : FPGA LPC IRQ %d Chassis Test Register failed\n", 
              __FUNCTION__, irq_num);
        if (toggle_driver_irq_flag(IRQ_DISABLE) == FAILED) {
            return (FAILED);
        }
        return (FAILED);
    }

    if (toggle_driver_irq_flag(IRQ_DISABLE) == FAILED) {
        return (FAILED);
    }

    return (PASSED);
}

/*******************************************************************************
 *
 * Function    : fpga_eobc_or_packet_interrupt_test 
 * Description : Test the IRQ6 interrupt by setting IRQ6 mask Register and
 *               CC/FP CP or FP Reset Control Register to check FPGA will send
 *               interrupt to CPU.
 * Inputs      : reset_num - CP or FP reset control 
 * Outputs     : PASSED/FAILED
 *
 *******************************************************************************
 */
int fpga_eobc_or_packet_interrupt_test (int reset_num)
{
    char tname[50];

    if (reset_num == CCCP_RESET){
        strcpy(tname, "FPGA CC CP Ready");
    
    } else if (reset_num == FPCP_RESET) {
        strcpy(tname, "FPGA FP CP Ready");
    
    } else {
        strcpy(tname, "FPGA FP Packet Ready");
    
    }
    testname("%s", tname);
    prpass(testpass, "%s, ", tname);

    if (toggle_driver_irq_flag(IRQ_ENABLE) == FAILED) {
        return (FAILED);
    }

    if (eobc_or_packet_interrupt_test(reset_num) == FAILED) {
        cterr('f', 0, "%s : %s test failed\n", 
              __FUNCTION__, tname);
        if (toggle_driver_irq_flag(IRQ_DISABLE) == FAILED) {
            return (FAILED);
        }
        return (FAILED);
    }

    if (toggle_driver_irq_flag(IRQ_DISABLE) == FAILED) {
        return (FAILED);
    }

    return (PASSED);
}

/*******************************************************************************
 *
 * Function    : fpga_mcerr_test
 * Description : Utility to test FPGA MCERR_N intrrupt.
 * Inputs      : opt - reserved for future use
 * Outputs     : PASSED/FAILED
 *
 *******************************************************************************
 */
int fpga_mcerr_test (int opt)
{
    uint32_t reg_val = 0;
    char usr_input = 0;
    int ix, jx, check_mode;
    
    if (toggle_driver_irq_flag(IRQ_ENABLE) == FAILED) {
        return (FAILED);
    }

    for (ix = MASK_ENABLE; ix<= MASK_DISABLE; ix++) {
        if (fpga_read_reg(PHASE2_FPGA_INTR_IRQ0_MASK_REG, &reg_val) != PASSED) {
            printf("Failed to read FPGA register 0x%04X.\n",
                   PHASE2_FPGA_INTR_IRQ0_MASK_REG);
            return (FAILED);
        }

        if (ix == MASK_ENABLE) {
            reg_val &= ~(TRUE << FPGA_MCERR_BIT9);
            check_mode = TRUE;
        } else {
            reg_val |= (TRUE << FPGA_MCERR_BIT9);
            check_mode = FALSE;
        }
        
        /* Write IRQ0 Mask Register bit 9 to 0 or 1 */
        if (fpga_write_reg(PHASE2_FPGA_INTR_IRQ0_MASK_REG, reg_val) != PASSED) {
            printf("Failed to write FPGA register 0x%04X.\n",
                   PHASE2_FPGA_INTR_IRQ0_MASK_REG);
            return (FAILED);
        }

	    /* Check IRQ0 Mask Register bit 9 is 0 or 1 */
        if (fpga_bit_to_check(PHASE2_FPGA_INTR_IRQ0_MASK_REG, FPGA_MCERR_BIT9,
                              ix) != PASSED) {
            printf("Failed to set FPGA IRQ0 Mask register 0x%04X.\n",
                   PHASE2_FPGA_INTR_IRQ0_MASK_REG);
            return (FAILED);
        
        }
	
        printf("\nPlease toogle the MCERR_N pin for interrupt IRQ0\n");
        do {
            printf("\r### Press 'y' to continue the Test: ");
            usr_input = getchar();
            if (usr_input == 'y') {
                break;
            }   
        } while (usr_input != 'y');
	
        /* Check SERIRQ is asserted or disasserted. */
        if (fpga_check_serirq(LPC_IRQ0, check_mode) != PASSED) {
            printf("%s : Failed to receive IRQ %d interrupt, "
                   "SERIRQ is not asserted or disasserted.\n",
                   __FUNCTION__, LPC_IRQ0);
            return (FAILED);
        }
        
        /* Clear CPU SERIRQ status. */
        if (fpga_clear_cpu_serirq_status() != PASSED) {
            printf("%s : Failed to clear CPU SERIRQ status.\n",
                   __FUNCTION__);
            return (FAILED);
        }
    
        /* Read Interrupt register IRQ0 Status twice to clear the interrupt */
        for (jx = CHECK_TWICE; jx >= 0; jx--) {
            if (fpga_bit_to_check(PHASE2_FPGA_INTR_IRQ0_STAT_REG, FPGA_MCERR_BIT9,
                             jx) != PASSED) {
                if (jx == CHECK_TWICE){
                    printf("After receiving interrupt the IRQ0 status register"
                           "bit 9 should be 1 is failed.\n");
                } else {
                    printf("Second read IRQ0 status register should clear register"
                           "is failed.\n");
                }
                
                return (FAILED);
            }
        } /* for check IRQ0 status twice */

	    /* Read LPC CPU Error Status & respond register. The bit 3 should be 1*/ 
        if (fpga_bit_to_check(PHASE2_FPGA_LPC_CPU_ERROR_STAT_REG, FPGA_MCERR_BIT3,
                         TRUE) != PASSED) {
            printf("Failed to read FPGA register 0x%04X.\n",
                   PHASE2_FPGA_LPC_CPU_ERROR_STAT_REG);
            return (FAILED);
        
        }
        
        reg_val |=  (TRUE << FPGA_MCERR_BIT3);
	    /* Write bit 3 to clear LPC CPU Error Status & respond register.*/ 
        if (fpga_write_reg(PHASE2_FPGA_LPC_CPU_ERROR_STAT_REG, reg_val) != PASSED) {
            printf("Failed to read FPGA register 0x%04X.\n",
                   PHASE2_FPGA_LPC_CPU_ERROR_STAT_REG);
            return (FAILED);
        }
	    
        /* Read LPC CPU Error Status & respond register. The bit 3 should be 0 */ 
        if (fpga_bit_to_check(PHASE2_FPGA_LPC_CPU_ERROR_STAT_REG, FPGA_MCERR_BIT3,
                         FALSE) != PASSED) {
            printf("Failed to read FPGA register 0x%04X.\n",
                   PHASE2_FPGA_LPC_CPU_ERROR_STAT_REG);
            return (FAILED);
        }

    } /* for enable IRQ0 Mask */

	printf("\nPass the MCERR_N test.\n");

    if (toggle_driver_irq_flag(IRQ_DISABLE) == FAILED) {
        return (FAILED);
    }
	
    return (PASSED);
}

/*******************************************************************************
 *
 * Function    : fpga_cpu_prochot_test
 * Description : Utility to test FPGA CPU_PROCHOT_N intrrupt.
 * Inputs      : opt - reserved for future use
 * Outputs     : PASSED/FAILED
 *
 *******************************************************************************
 */
int fpga_cpu_prochot_test (int opt)
{
    uint32_t reg_val = 0;
    char usr_input = 0;
    int ix, jx, check_mode;
    
    if (toggle_driver_irq_flag(IRQ_ENABLE) == FAILED) {
        return (FAILED);
    }

    for (ix = MASK_ENABLE; ix<= MASK_DISABLE; ix++) {
        if (fpga_read_reg(PHASE2_FPGA_INTR_IRQ0_MASK_REG, &reg_val) != PASSED) {
            printf("Failed to read FPGA register 0x%04X.\n",
                   PHASE2_FPGA_INTR_IRQ0_MASK_REG);
            return (FAILED);
        }

        if (ix == MASK_ENABLE) {
            reg_val &= ~(TRUE << FPGA_PROCHOT_BIT13);
            check_mode = TRUE;
        } else {
            reg_val |= (TRUE << FPGA_PROCHOT_BIT13);
            check_mode = FALSE;
        }
        
        /* Write IRQ0 Mask Register bit 13 to 0 or 1 */
        if (fpga_write_reg(PHASE2_FPGA_INTR_IRQ0_MASK_REG, reg_val) != PASSED) {
            printf("Failed to write FPGA register 0x%04X.\n",
                   PHASE2_FPGA_INTR_IRQ0_MASK_REG);
            return (FAILED);
        }

	    /* Check IRQ0 Mask Register bit 13 is 0 or 1 */
        if (fpga_bit_to_check(PHASE2_FPGA_INTR_IRQ0_MASK_REG, FPGA_PROCHOT_BIT13,
                         ix) != PASSED) {
            printf("Failed to set FPGA IRQ0 Mask register 0x%04X.\n",
                   PHASE2_FPGA_INTR_IRQ0_MASK_REG);
            return (FAILED);
        
        }
	
        printf("\nPlease toogle the CPU_PROCHOT_N pin for interrupt IRQ0\n");
        do {
            printf("\r### Press 'y' to continue the Test: ");
            usr_input = getchar();
            if (usr_input == 'y') {
                break;
            }   
        } while (usr_input != 'y');
	
        /* Check SERIRQ is asserted or disasserted. */
        if (fpga_check_serirq(LPC_IRQ0, check_mode) != PASSED) {
            printf("%s : Failed to receive IRQ %d interrupt, "
                   "SERIRQ is not asserted or disasserted.\n",
                   __FUNCTION__, LPC_IRQ0);
            return (FAILED);
        }
        
        /* Clear CPU SERIRQ status. */
        if (fpga_clear_cpu_serirq_status() != PASSED) {
            printf("%s : Failed to clear CPU SERIRQ status.\n",
                   __FUNCTION__);
            return (FAILED);
        }
    
        /* Read Interrupt register IRQ0 Status twice to clear the interrupt */
        for (jx = CHECK_TWICE; jx >= 0; jx--) {
            if (fpga_bit_to_check(PHASE2_FPGA_INTR_IRQ0_STAT_REG, FPGA_PROCHOT_BIT13,
                             jx) != PASSED) {
                if (jx == CHECK_TWICE){
                    printf("After receiving interrupt the IRQ0 status register"
                           "bit 13 should be 1 is failed.\n");
                } else {
                    printf("Second read IRQ0 status register should clear register"
                           "is failed.\n");
                }
                
                return (FAILED);
            }
        } /* for check IRQ0 status twice */
    } /* for enable IRQ0 Mask */

	printf("\nPass the CPU_PROCHOT_N test.\n");

    if (toggle_driver_irq_flag(IRQ_DISABLE) == FAILED) {
        return (FAILED);
    }
	
    return (PASSED);
}

/*******************************************************************************
 *
 * Function    : fpga_lpc_reset_ctl_reg_test
 * Description : Check bit 7:0 are readable and writeable in this register.
 * Inputs      : NONE 
 * Outputs     : PASSED/FAILED
 *
 *******************************************************************************
 */
int fpga_lpc_reset_ctl_reg_test (void)
{
    uint32_t reg_val = 0, read_val = 0;
    uint expected[TEST_PATTERN_SIZE];
    
    char *tname ="FPGA LPC Reset Control Register";
    testname(tname);
    prpass(testpass, "%s, ", tname);

    /* Read register(0x010) to check bit 7:1 is 0x0101100 */
    if (fpga_read_reg(PHASE2_FPGA_LPC_RESET_CONTROL_REG, &read_val) != PASSED) {
        printf("Failed to read FPGA register 0x%04X.\n",
               PHASE2_FPGA_LPC_RESET_CONTROL_REG);
        return (FAILED);
    }
    reg_val = read_val;
    if ((reg_val &= DEFAULT_LPC_RESET_CTL_REG) != DEFAULT_LPC_RESET_CTL_REG) {
        cterr('f', 0, "%s : FPGA LPC Reset Control Register is not 0x%08X, "
              "value equals to 0x%08X.\n", __FUNCTION__,
              DEFAULT_LPC_RESET_CTL_REG, reg_val);
        return (FAILED);
    }

    /* Write register(0x010) to test pattern, and read back to check */
    if (fpga_test_pattern_test(PHASE2_FPGA_LPC_RESET_CONTROL_REG, &expected[0],
                               LAST_BYTE, read_val) != PASSED) {
        cterr('f', 0, "%s : Test Pattern failed.\n ", __FUNCTION__);
        return (FAILED);
    }

    return (PASSED);
}

/*******************************************************************************
 *
 * Function    : fpga_lpc_debug_ctl_stat_reg_test
 * Description : Check bit 10:8 are readable and writeable in this register.
 * Inputs      : NONE 
 * Outputs     : PASSED/FAILED
 *
 *******************************************************************************
 */
int fpga_lpc_debug_ctl_stat_reg_test (void)
{
    uint32_t reg_val = 0;
    uint expected[] = {READ_BACK_VALUE_1,
                       READ_BACK_VALUE_2,
                       READ_BACK_VALUE_3,
                       READ_BACK_VALUE_4,
                       READ_BACK_VALUE_5};
    
    char *tname ="FPGA LPC Debug Control/Status Register";
    testname(tname);
    prpass(testpass, "%s, ", tname);

    /* Read register(0x018) to check it is 0x0 */
    if (fpga_read_reg(PHASE2_FPGA_LPC_DEBUG_CTL_STAT_REG, &reg_val) != PASSED) {
        printf("Failed to read FPGA register 0x%04X.\n",
               PHASE2_FPGA_LPC_DEBUG_CTL_STAT_REG);
        return (FAILED);
    }
    if (reg_val != DEFAULT_VALUE_IS_ZERO) {
        cterr('f', 0, "%s : FPGA LPC Debug Control/Status Register is not 0, "
              "value equals to 0x%08X.\n", __FUNCTION__, reg_val);
        return (FAILED);
    }

    /* Write register(0x018) to test pattern, and read back to check */
    if (fpga_test_pattern_test(PHASE2_FPGA_LPC_DEBUG_CTL_STAT_REG, &expected[0],
                               EXPECTED, DEFAULT_VALUE_IS_ZERO) != PASSED) {
        cterr('f', 0, "%s : Test Pattern failed.\n ", __FUNCTION__);
        return (FAILED);
    }

    return (PASSED);
}

/*******************************************************************************
 *
 * Function    : fpga_lpc_flash_led_ctl_reg_test
 * Description : Check bit 3:0 are readable and writeable in this register.
 * Inputs      : NONE 
 * Outputs     : PASSED/FAILED
 *
 *******************************************************************************
 */
int fpga_lpc_flash_led_ctl_reg_test (void)
{
    uint32_t reg_val = 0;
    uint expected[TEST_PATTERN_SIZE];
    
    char *tname ="FPGA LPC Flash LED Control Register";
    testname(tname);
    prpass(testpass, "%s, ", tname);

    /* Read register(0x020) to check it is 0x0 */
    if (fpga_read_reg(PHASE2_FPGA_LPC_FLASH_LED_CTL_REG, &reg_val) != PASSED) {
        printf("Failed to read FPGA register 0x%04X.\n",
               PHASE2_FPGA_LPC_FLASH_LED_CTL_REG);
        return (FAILED);
    }
    if (reg_val != DEFAULT_VALUE_IS_ZERO) {
        cterr('f', 0, "%s : FPGA LPC Flash LED Control Register is not 0, "
              "value equals to 0x%08X.\n", __FUNCTION__, reg_val);
        return (FAILED);
    }

    /* Write register(0x020) to test pattern, and read back to check */
    if (fpga_test_pattern_test(PHASE2_FPGA_LPC_FLASH_LED_CTL_REG, &expected[0],
                               LAST_FOUR_BIT, DEFAULT_VALUE_IS_ZERO) != PASSED) {
        cterr('f', 0, "%s : Test Pattern failed.\n ", __FUNCTION__);
        return (FAILED);
    }

    return (PASSED);
}

/*******************************************************************************
 *
 * Function    : fpga_lpc_chassis_manage_ctl_reg_test
 * Description : Check bit 31:16, 14:10, 8:2 ang 0 are readable and writeable
 *               in this register.
 * Inputs      : NONE 
 * Outputs     : PASSED/FAILED
 *
 *******************************************************************************
 */
int fpga_lpc_chassis_manage_ctl_reg_test (void)
{
    uint32_t reg_val = 0;
    uint expected[] = {CHASSIS_MANAGE_READ_BACK_VALUE_1,
                        CHASSIS_MANAGE_READ_BACK_VALUE_2,
                        CHASSIS_MANAGE_READ_BACK_VALUE_3,
                        CHASSIS_MANAGE_READ_BACK_VALUE_4,
                        CHASSIS_MANAGE_READ_BACK_VALUE_5};
    
    char *tname ="FPGA LPC Chassis Management Control Register";
    testname(tname);
    prpass(testpass, "%s, ", tname);

    /* Read register(0x044) to check it is 0x0 */
    if (fpga_read_reg(PHASE2_FPGA_LPC_CHASSIS_MANAGE_CTL_REG, &reg_val) != PASSED) {
        printf("Failed to read FPGA register 0x%04X.\n",
               PHASE2_FPGA_LPC_CHASSIS_MANAGE_CTL_REG);
        return (FAILED);
    }
    if (reg_val != DEFAULT_VALUE_IS_ZERO) {
        cterr('f', 0, "%s : FPGA LPC Chassis Management Control Register is not 0, "
              "value equals to 0x%08X.\n", __FUNCTION__, reg_val);
        return (FAILED);
    }

    /* Write register(0x044) to test pattern, and read back to check */
    if (fpga_test_pattern_test(PHASE2_FPGA_LPC_CHASSIS_MANAGE_CTL_REG, &expected[0],
                               EXPECTED, DEFAULT_VALUE_IS_ZERO) != PASSED) {
        cterr('f', 0, "%s : Test Pattern failed.\n ", __FUNCTION__);
        return (FAILED);
    }
    
    return (PASSED);
}

/*******************************************************************************
 *
 * Function    : fpga_lpc_alarm_manage_ctl_reg_test
 * Description : Check bit 10:8 and 2:0 are readable and writeable in this register.
 * Inputs      : NONE 
 * Outputs     : PASSED/FAILED
 *
 *******************************************************************************
 */
int fpga_lpc_alarm_manage_ctl_reg_test (void)
{
    uint32_t reg_val = 0;
    uint expected[] = {READ_BACK_VALUE_1,
                       READ_BACK_VALUE_2,
                       READ_BACK_VALUE_3,
                       READ_BACK_VALUE_4,
                       READ_BACK_VALUE_5};
    
    char *tname ="FPGA LPC Alarm Management Control Register";
    testname(tname);
    prpass(testpass, "%s, ", tname);

    /* Read register(0x048) to check it is 0x00000100 */
    if (fpga_read_reg(PHASE2_FPGA_LPC_ALARM_MANAGE_CTL_REG, &reg_val) != PASSED) {
        printf("Failed to read FPGA register 0x%04X.\n",
               PHASE2_FPGA_LPC_ALARM_MANAGE_CTL_REG);
        return (FAILED);
    }
    if (reg_val != DEFAULT_LPC_ALARM_MANAGE_CTL_REG) {
        cterr('f', 0, "%s : FPGA LPC Alarm Management Control Register is not "
              "0x%08X, value equals to 0x%08X.\n", __FUNCTION__,
              DEFAULT_LPC_ALARM_MANAGE_CTL_REG, reg_val);
        return (FAILED);
    }

    /* Write register(0x048) to test pattern, and read back to check */
    if (fpga_test_pattern_test(PHASE2_FPGA_LPC_ALARM_MANAGE_CTL_REG,
                               &expected[0], EXPECTED,
                               DEFAULT_LPC_ALARM_MANAGE_CTL_REG) != PASSED) {
        cterr('f', 0, "%s : Test Pattern failed.\n ", __FUNCTION__);
        return (FAILED);
    }
   
    return (PASSED);
}

/******** History ******** 
$Log: diag_fpga_test.c,v $
Revision 1.10  2020/12/25 09:24:18  alicehua
CSCvw90893: For New FPGA image release.

Revision 1.9  2020/05/07 01:34:56  alicehua
CSCvu09464: Add utility and modify test case for FPGA test plan.

Revision 1.8  2020/03/06 07:42:32  alicehua
CSCvt28948:
1. Modify codes for FPGA register default value changing issue.
   With new ROMMON (17.3(03d)), FPGA register (0x010) will get 0x59,
   so we just check bit 7:0, ignore bit 0.
2. Add a function to distinguish bootloader is BIOS or ROMMON,
   so that we can hide IRQ test items if bootloader is BIOS.

Revision 1.7  2020/03/04 00:02:50  alicehua
CSCvt24819: Enable IRQ test items for XE build.

Revision 1.6  2020/02/04 08:49:42  alicehua
CSCvs68364: Add and modify codes for FPGA Phase2.

Revision 1.5  2020/01/14 08:19:14  alicehua
CSCvs65650: Add new PID in FPGA board type test for 8GB SKU.

Revision 1.4  2019/07/11 12:31:27  alicehua
Collapse Nutella codes into main trunk.

$Endlog$
*/
