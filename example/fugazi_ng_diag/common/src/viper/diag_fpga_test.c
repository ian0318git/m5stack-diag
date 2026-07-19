 /* $Id: diag_fpga_test.c,v 1.6 2018/12/06 01:56:09 harrchan Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/viper/diag_fpga_test.c,v $
 *------------------------------------------------------------------
 *
 * diag_fpga_test.c - FPGA Test
 *
 *
 * Copyright (c) 2018 by Cisco Systems, Inc.
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
#include "plat_defs.h"
#include "diag_fpga_i2c_lib.h"
#include "dnv_gpio_lib.h"
#include "diag_common.h"
#include "dnv_eth_lib.h"
#include "diag_gephy_lib.h"
#include "diag_lte_lib.h"
#include "dev_88e6176.h"
#include "diag_esw_test.h"
#include "diag_xdsl_test.h"
#include "diag_lte_test.h"



/*
 * Global variables
 */

static boolean fpga_watchdog_force_stop = FALSE;


/* Local functions */
int diag_fpga_reg_test(void);
int diag_fpga_reg_def_test(void);
int build_fpga_test_menu(boolean);
int fpga_reg_test_read_fn(ulong, int, ulong *, void *);
int fpga_reg_test_write_fn(ulong, int, ulong, void *);
int viper_fpga_utils(int);
int viper_fpga_function_test(int);
int viper_show_fpga_ver(int);
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
int fpga_act2_reset_test(void);
int fpga_i2c_reset_test(void);
int fpga_revision_test(void);
int diag_wdt_disable_test(void);
int diag_wdt_counter_test(void);
int diag_fpga_esw_reset_test(void);
int diag_board_type_test(void);


reg_info_t_ext viper_fpga_reg_ext = {VIPER_FPGA_REG_WIDTH,
                                     fpga_reg_test_read_fn,
                                     fpga_reg_test_write_fn,
                                     0};


static reg_info_t fpga_reg_dump_tbl[] = {
    /* Format: NAME, OFFSET, TYPE, SIZE, MASK, RESET_VAL. */
    {"FPGA External Device Reset",    FPGA_EXTER_DEV_RST_REG,     FPGA_RW,
        {(unsigned long)&viper_fpga_reg_ext},   0x03F83ABA, 0x370FAF2},
    {"Internal Device Reset",         FPGA_INT_DEV_RST_REG,       FPGA_RW,
        {(unsigned long)&viper_fpga_reg_ext},   0xFFFFFFFF, 0x0},
    {"Board Type",                    FPGA_BOARD_TYPE_REG,        FPGA_RONLY,
        {(unsigned long)&viper_fpga_reg_ext},   0x00000000, 0x13},
    {"Master FPGA Revision",          FPGA_MASTER_REV_REG,        FPGA_RONLY,
        {(unsigned long)&viper_fpga_reg_ext},   0x00000000, 0x800103},
    {"FPGA Revision",                 FPGA_REV_REG,               FPGA_RONLY,
        {(unsigned long)&viper_fpga_reg_ext},   0x00000000, 0x16041901},
    {"CPU Mux and USB Power Control", FPGA_CPUMUX_AND_USBPWR_REG, FPGA_RW,
        {(unsigned long)&viper_fpga_reg_ext},   0x00000003, 0x0},
    {"Status and Control",            FPGA_STAT_AND_CTRL_REG,     FPGA_RW,
        {(unsigned long)&viper_fpga_reg_ext},   0x000003EE, 0xE},
    {"Power Status",                  FPGA_PWR_STAT_REG,          FPGA_RONLY,
        {(unsigned long)&viper_fpga_reg_ext},   0x00000000, 0xFFFFFFFF},
    {"LED",                           FPGA_LED_REG,               FPGA_RW,
        {(unsigned long)&viper_fpga_reg_ext},   0x00FCFE03, 0x0},
    {"Watchdog Strobe",               FPGA_WATCHDOG_REG,          FPGA_RONLY,
        {(unsigned long)&viper_fpga_reg_ext},   0x00000000, 0x0},
    {"LTE Control",                   FPGA_LTE_CTL_REG,           FPGA_RW,
        {(unsigned long)&viper_fpga_reg_ext},   0x000000CE, 0x0},
    {"SIM Status and Control",        FPGA_SIM_STATUS_CTL_REG,    FPGA_RW,
        {(unsigned long)&viper_fpga_reg_ext},   0x00000306, 0x0},
    {"xDSL Status and Control",       FPGA_DSL_STATUS_CTL_REG,    FPGA_RONLY,
        {(unsigned long)&viper_fpga_reg_ext},   0x00000000, 0x0},
};

/*
 * FPGA register test
 */
static reg_info_t fpga_reg_test_tbl[] = {
    /* Format: NAME, OFFSET, TYPE, SIZE, MASK, RESET_VAL. */
    {"Access Test Register R/W", FPGA_ACCESS_TEST_REG, FPGA_RW,
     {(unsigned long)&viper_fpga_reg_ext}, 0xFFFF, 0},
    {"END",                                      0x00,       0,
     {0},                                         0x0,        0x0},
};

/* 
 * FPGA register default value
 */
static reg_info_t fpga_reg_def_tbl[] = {
	/* Format: NAME, OFFSET, TYPE, SIZE, MASK, DEFAULT_VAL. */
	{"SPI Control Register", FPGA_SPI_CONTROL_REG, FPGA_RW,
	 {(unsigned long)&viper_fpga_reg_ext}, 0xC0000000, 0x0},
	{"CPU Boot Mux Register", FPGA_CPUMUX_AND_USBPWR_REG, FPGA_RW,
	 {(unsigned long)&viper_fpga_reg_ext}, 0x00000060, 0x0},
	{"IOS Watchdog Strobe Register", FPGA_WATCHDOG_REG, FPGA_RW,
	 {(unsigned long)&viper_fpga_reg_ext}, 0xFFFFFFFF, 0x0},
	{"I2C Scratch Pad Register", FPGA_I2C_SCRATCH_PAD, FPGA_RW,
	 {(unsigned long)&viper_fpga_reg_ext}, 0xFFFFFFFF, 0xFACEDEAD},
	{"SPI PROM Control Register", FPGA_SPI_CTRL_REG, FPGA_RW,
	 {(unsigned long)&viper_fpga_reg_ext}, 0x000087FF, 0x00000010},
	{"SPI PROM Status Register", FPGA_SPI_STAT_REG, FPGA_RW,
	 {(unsigned long)&viper_fpga_reg_ext}, 0x0000801F, 0x0000000A},
	{"SPI PROM Read Size Register", FPGA_SPI_RD_SIZE_REG, FPGA_RW,
	 {(unsigned long)&viper_fpga_reg_ext}, 0x000000FF, 0x0},
	{"SPI PROM Read/Write Data Register", FPGA_SPI_RW_DATA_REG, FPGA_RW,
	 {(unsigned long)&viper_fpga_reg_ext}, 0x000000FF, 0x0},
	{"SPI PROM Opcode/Address Register", FPGA_SPI_OP_ADDR_REG, FPGA_RW,
	 {(unsigned long)&viper_fpga_reg_ext}, 0xFFFFFFFF, 0x0},
	{"FPGA Reconfiguration Control Register", FPGA_RECONFIG_CTRL_REG, FPGA_RW,
	 {(unsigned long)&viper_fpga_reg_ext}, 0x00000012, 0x0},
	{"Firmware Status Register", FPGA_FIRMWARE_STATUS_REG, FPGA_RW,
	 {(unsigned long)&viper_fpga_reg_ext}, 0xFFFFFFFF, 0x0},
	{"Soft Secure Boot Status Register", FPGA_SOFT_SECURE_BOOT_STATUS_REG, FPGA_RW,
	 {(unsigned long)&viper_fpga_reg_ext}, 0xFFFFFFFF, 0x0},
	{"Scratchpad Register 1", FPGA_SCRATCHPAD_REG_1, FPGA_RW,
	 {(unsigned long)&viper_fpga_reg_ext}, 0xFFFFFFFF, 0x0},
	{"Scratchpad Register 2", FPGA_SCRATCHPAD_REG_2, FPGA_RW,
	 {(unsigned long)&viper_fpga_reg_ext}, 0xFFFFFFFF, 0x0},
	{"Scratchpad Register 3", FPGA_SCRATCHPAD_REG_3, FPGA_RW,
	 {(unsigned long)&viper_fpga_reg_ext}, 0xFFFFFFFF, 0x0},
	{"Scratchpad Register 4", FPGA_SCRATCHPAD_REG_4, FPGA_RW,
	 {(unsigned long)&viper_fpga_reg_ext}, 0xFFFFFFFF, 0x0},
	{"Scratchpad Register 5", FPGA_SCRATCHPAD_REG_5, FPGA_RW,
	 {(unsigned long)&viper_fpga_reg_ext}, 0xFFFFFFFF, 0x0},
	{"Scratchpad Register 6", FPGA_SCRATCHPAD_REG_6, FPGA_RW,
	 {(unsigned long)&viper_fpga_reg_ext}, 0xFFFFFFFF, 0x0},
	{"END",                                      0x00,       0,
     {0},                                         0x0,        0x0},
};


/*
 * Sub Menu used for "FPGA  test -> FPGA submenu test"
 */
submenu_xtable_t fpga_submenu_table[] = {
    {"FPGA Utility",  
     (PFT) viper_fpga_utils,      FALSE,
     0, (type_t(*)())0,                0,
     (type_t(*)())viper_fpga_utils,   TRUE},

    {"FPGA Register Test",
     (PFT) diag_fpga_reg_test, FALSE,
     MF_CONTINUOUS | MF_DOGRP | MF_DOALL | MF_SHOW_ERRCOUNT,
     (type_t(*)())0, 0, (PFT) 0, 0},
    
     {"FPGA Function Test",  
     (PFT) viper_fpga_function_test,      TRUE,
     MF_CONTINUOUS | MF_DOGRP | MF_DOALL | MF_SHOW_ERRCOUNT,
     (type_t(*)())0, 0,
     (type_t(*)())viper_fpga_function_test,   FALSE},

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
    {"Show FPGA version",   (type_t(*)())viper_show_fpga_ver,  0, 0,
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
    {"Program FPGA Golden image without header", (type_t(*)())program_reggio_spi_prom, 0, 0,
     (type_t(*)())0, 0, (type_t(*)())0,   0},
    {"Program FPGA Upgrade image with header", (type_t(*)())program_reggio_spi_prom, 1, 0,
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
    {"FPGA registers Dump default value", (type_t(*)())fpga_reg_dump_def_util, 0, 0,
     (type_t(*)())0, 0,     (type_t(*)())0, 0}, 
    {"FPGA Register Default Test", (type_t(*)())diag_fpga_reg_def_test, 0, 0,
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
    {"Watch Dog Counter Test(0x64)", (type_t(*)())diag_wdt_counter_test, TRUE,
     MF_CONTINUOUS | MF_DOALL | MF_SHOW_ERRCOUNT,
      (type_t(*)())0, 0, (type_t(*)())0, 0},
    {"FPGA IOS Watchdog Timer(0x084) Test", (type_t(*)())fpga_ios_watchdog_timer_reg_test, TRUE,
     MF_CONTINUOUS | MF_DOALL | MF_SHOW_ERRCOUNT,
      (type_t(*)())0, 0, (type_t(*)())0, 0},
    {"FPGA ACT2 Reset(0x804) Test", (type_t(*)())fpga_act2_reset_test, TRUE,
     MF_CONTINUOUS | MF_DOALL | MF_SHOW_ERRCOUNT,
      (type_t(*)())0, 0, (type_t(*)())0, 0},
    {"Ethernet Switch Reset(0x804) Test", (type_t(*)())diag_fpga_esw_reset_test, TRUE,
     MF_CONTINUOUS | MF_DOALL | MF_SHOW_ERRCOUNT,
      (type_t(*)())0, 0, (type_t(*)())0, 0},
    {"FPGA I2C Reset(0x808) Test", (type_t(*)())fpga_i2c_reset_test, TRUE,
     MF_CONTINUOUS | MF_DOALL | MF_SHOW_ERRCOUNT,
      (type_t(*)())0, 0, (type_t(*)())0, 0},
    {"FPGA PHY Reset(0x808) Test", (type_t(*)())fpga_phy_reset_test, TRUE,
     MF_CONTINUOUS | MF_DOALL | MF_SHOW_ERRCOUNT,
      (type_t(*)())0, 0, (type_t(*)())0, 0},
    {"FPGA Revision(0x884&0x88C) Test", (type_t(*)())fpga_revision_test, TRUE,
     MF_CONTINUOUS | MF_DOALL | MF_SHOW_ERRCOUNT,
      (type_t(*)())0, 0, (type_t(*)())0, 0},
    {"Board type(0x8C0) test", (type_t(*)())diag_board_type_test, TRUE,
     MF_CONTINUOUS | MF_DOALL | MF_SHOW_ERRCOUNT,
      (type_t(*)())0, 0, (type_t(*)())0, 0}, 
    {"FPGA Power Status and Control(0x910) Test", (type_t(*)())fpga_power_status_control_reg_test, TRUE,
     MF_CONTINUOUS | MF_DOALL | MF_SHOW_ERRCOUNT,
      (type_t(*)())0, 0, (type_t(*)())0, 0},
    {"FPGA IOS Watchdog Strobe(0x924) Test", (type_t(*)())fpga_ios_watchdog_strobe_reg_test, TRUE,
     MF_CONTINUOUS | MF_DOALL | MF_SHOW_ERRCOUNT,
      (type_t(*)())0, 0, (type_t(*)())0, 0},
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
 * Function    : viper_fpga_utils
 * Description : Function to show VIPER FPGA utilities submenu.
 * Inputs      : opt - reserved for future use.
 * Outputs     : PASSED / FAILED
 *
 *******************************************************************************
 */
int viper_fpga_utils (int opt)
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
 * Function    : viper_fpga_function_test
 * Description : Function to show VIPER FPGA function test submenu.
 * Inputs      : opt - reserved for future use.
 * Outputs     : PASSED / FAILED
 *
 *******************************************************************************
 */
int viper_fpga_function_test (int opt)
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

    if (register_tests(0, fpga_reg_test_tbl) != PASSED) {
        cterr('f', 0, "FPGA Register test Failed");
        prcomplete(testpass, errcount, (char *)0);
        return (FAILED);
    }

    prcomplete(testpass, errcount, (char *)0);

    return (PASSED);
}

/******************************************************************************
 *
 * Function: diag_fpga_reg_def_test
 *
 * Description: Function to test FPGA register Default Value
 *
 * Inputs      : None
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/
int diag_fpga_reg_def_test (void)
{
    char *tname ="FPGA Register Default Value";
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

    if (register_def_tests(0, fpga_reg_def_tbl) != PASSED) {
        cterr('f', 0, "FPGA Register default value test Failed");
        prcomplete(testpass, errcount, (char *)0);
        return (FAILED);
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
        printf("%s: Failed to read VIPER FPGA Reg(0x%lx).\n",
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
 * Function   : viper_show_fpga_ver
 * Description: Function to show FPGA version.
 *              This is by reading FPGA Revision Reg(0x88C).
 * Inputs     : opt - reserved for future use
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
int viper_show_fpga_ver (int opt)
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
        if (fpga_read_reg(FPGA_EXTER_DEV_RST_REG, &reg_val) != PASSED) {
            printf("Failed to read FPGA register 0x%04X.\n", FPGA_EXTER_DEV_RST_REG);
            return (FAILED);
        }

        reg_val |= EXT_CPU_SYS_RESET;

        if (fpga_write_reg(FPGA_EXTER_DEV_RST_REG, reg_val) != PASSED) {
            printf("Failed to write FPGA register 0x%04X.\n", FPGA_EXTER_DEV_RST_REG);
            return (FAILED);
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
    uint32_t reg_val = 0;

    printf("Show reset reason\n");

    if (fpga_read_reg(FPGA_RESET_REASON_REG, &reg_val) != PASSED) {
        printf("Failed to read FPGA register 0x%04X.\n", FPGA_RESET_REASON_REG);
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

    case (REASON_CPU_THERMAL_RESET):
        printf("CPU reset from Thermal Trip\n");
        break;

    case (REASON_BOOT_FAIL):
        printf("Boot fail\n");
        break;

    case (REASON_IOS_WATCHDOG_TIMEOUT):
        printf("IOS Watchdog Timeout\n");
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
    uint32_t reg_val = 0;
    uint32_t time_val =0; 
    while (fpga_watchdog_force_stop == FALSE ) {
        if (fpga_read_reg(FPGA_IOS_WATCHDOG_TIMER_REG, &reg_val) != PASSED) {
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

    uint32_t get_ans = 0;
    pthread_t watchdog_thread;
    void *ret;

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
		
	    if (fpga_write_reg(FPGA_IOS_WATCHDOG_TIMER_REG, FPGA_IOS_WDT_ENABLE_KEY) != PASSED) {
            printf("Failed to write FPGA register 0x%04X.\n", FPGA_IOS_WATCHDOG_TIMER_REG);
	        fpga_watchdog_force_stop = TRUE;
            return (FAILED);
        }
	 
        while (getc_answer("", "yn", 'n') != 'y');

        if (fpga_write_reg(FPGA_IOS_WATCHDOG_TIMER_REG, FPGA_IOS_WDT_DISABLE_KEY) != PASSED) {
            printf("Failed to write FPGA register 0x%04X.\n", FPGA_IOS_WATCHDOG_TIMER_REG);
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
 * Function: fpga_act2_reset_test
 *
 * Description: Function to test FPGA ACT2 reset test
 *
 * Inputs      : None
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/
int fpga_act2_reset_test (void) 
{
    char *tname ="FPGA ACT2 reset";
    int dummy = 0;

    testname(tname);
    prpass(testpass, "%s, ", tname);

    printf("\nReset ACT2\n");

    /* Reset the ACT2 */
    fpga_reset_api(FPGA_EXTER_DEV_RST_REG, FPGA_ACT2_RST_L, TRUE,
                   WAITTIME_5_MS);

    if (fpga_i2c_scan_addr(dummy) == PASSED) {
        cterr('f', 0, "FPGA ACT2 reset test Failed");
        prcomplete(testpass, errcount, (char *)0);
        return (FAILED);
    }

    printf("\nUn-Reset ACT2\n");
    /* un-reset the ACT2 */
    fpga_reset_api(FPGA_EXTER_DEV_RST_REG, FPGA_ACT2_RST_L, FALSE,
                   SLEEP_5S);

    if (fpga_i2c_scan_addr(dummy) == FAILED) {
        cterr('f', 0, "FPGA ACT2 un-reset test Failed");
        prcomplete(testpass, errcount, (char *)0);
        return (FAILED);

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

    testname(tname);
    prpass(testpass, "%s, ", tname);

    fpga_read_reg((uint)FPGA_MASTER_REV_REG, &master_revision_data);
    printf("\nMaster Revision(0x%x) val: 0x%x\n",FPGA_MASTER_REV_REG, master_revision_data);
    fpga_read_reg((uint)FPGA_REV_REG, &revision_data);
    printf("Revision(0x%x) val: 0x%x\n", FPGA_REV_REG, revision_data);

    /* Check debug bit */
    printf("1. Check Debug bit(0x884 bit 23)\n");
    if (master_revision_data & MASTER_FPGA_DEBUG_MASK) {
        cterr('f', 0, "FPGA debug bit test Failed");
        prcomplete(testpass, errcount, (char *)0);
        return (FAILED);
    }

    /* Compare Master revision and revision */
    printf("2. Compare Master revision(0x884) and revision(0x88C)\n");
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

    uint32_t reg_val = 0, reg_val_2nd = 0;
    int ix;
    char *tname ="FPGA IOS Watchdog Timer Register";
    testname(tname);
    prpass(testpass, "%s, ", tname);

    /* Check if IOS Watchdog Timer Counter is the default value.*/
    if (fpga_read_reg(FPGA_IOS_WATCHDOG_TIMER_REG, &reg_val) != PASSED) {
        printf("Failed to read FPGA register 0x%04X.\n", FPGA_IOS_WATCHDOG_TIMER_REG);
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

        if (fpga_write_reg(FPGA_IOS_WATCHDOG_TIMER_REG, FPGA_IOS_WDT_ENABLE_KEY) != PASSED) {
            printf("Failed to write FPGA register 0x%04X.\n", FPGA_IOS_WATCHDOG_TIMER_REG);
            cterr('f', 0, "FPGA IOS Watchdog Timer Register Test Failed");
            return (FAILED);
        }

        msleep(WAITTIME_250_MS);
		
        /* Check if counter is in the expected range 60s ~ 59s. */
        if (fpga_read_reg(FPGA_IOS_WATCHDOG_TIMER_REG, &reg_val) != PASSED) {
            printf("Failed to read FPGA register 0x%04X.\n", FPGA_IOS_WATCHDOG_TIMER_REG);
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
        if (fpga_write_reg(FPGA_IOS_WATCHDOG_TIMER_REG, FPGA_IOS_WDT_DISABLE_KEY) != PASSED) {
            printf("Failed to write FPGA register 0x%04X.\n", FPGA_IOS_WATCHDOG_TIMER_REG);
            cterr('f', 0, "FPGA IOS Watchdog Timer Register Test Failed");
            return (FAILED);
        }

        msleep(WAITTIME_250_MS);

        /* Read IOS Watchdog Timer Counter twice. Make sure the IOS Watchdog Timer is not counting.  */
        if (fpga_read_reg(FPGA_IOS_WATCHDOG_TIMER_REG, &reg_val) != PASSED) {
            printf("Failed to read FPGA register 0x%04X.\n", FPGA_IOS_WATCHDOG_TIMER_REG);
            cterr('f', 0, "FPGA IOS Watchdog Timer Register Test Failed");
            return (FAILED);
        }
        
        msleep(WAITTIME_250_MS);

        if (fpga_read_reg(FPGA_IOS_WATCHDOG_TIMER_REG, &reg_val_2nd) != PASSED) {
            printf("Failed to read FPGA register 0x%04X.\n", FPGA_IOS_WATCHDOG_TIMER_REG);
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

    uint32_t reg_val = 0, reg_val_2nd = 0;
    char *tname ="FPGA IOS Watchdog Strobe Register";
    testname(tname);
    prpass(testpass, "%s, ", tname);

    /* Enable IOS Watchdog Timer */
    if (fpga_write_reg(FPGA_IOS_WATCHDOG_TIMER_REG, FPGA_IOS_WDT_ENABLE_KEY) != PASSED) {
        printf("Failed to write FPGA register 0x%04X.\n", FPGA_IOS_WATCHDOG_TIMER_REG);
        cterr('f', 0, "FPGA IOS Watchdog Strobe Register Test Failed");
        return (FAILED);
    }

    msleep(WAITTIME_2000_MS);

    /* Wait 2 seconds, then read back IOS Watchdog Timer Counter, make sure it is counting. */
    if (fpga_read_reg(FPGA_IOS_WATCHDOG_TIMER_REG, &reg_val) != PASSED) {
        printf("Failed to read FPGA register 0x%04X.\n", FPGA_IOS_WATCHDOG_TIMER_REG);
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
    if (fpga_read_reg(FPGA_IOS_WATCHDOG_TIMER_REG, &reg_val) != PASSED) {
        printf("Failed to read FPGA register 0x%04X.\n", FPGA_IOS_WATCHDOG_TIMER_REG);
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
    if (fpga_write_reg(FPGA_IOS_WATCHDOG_TIMER_REG, FPGA_IOS_WDT_DISABLE_KEY) != PASSED) {
        printf("Failed to write FPGA register 0x%04X.\n", FPGA_IOS_WATCHDOG_TIMER_REG);
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
    if (fpga_read_reg(FPGA_IOS_WATCHDOG_TIMER_REG, &reg_val) != PASSED) {
        printf("Failed to read FPGA register 0x%04X.\n", FPGA_IOS_WATCHDOG_TIMER_REG);
        cterr('f', 0, "FPGA IOS Watchdog Strobe Register Test Failed");
        return (FAILED);
    }

    msleep(WAITTIME_250_MS);

    if (fpga_read_reg(FPGA_IOS_WATCHDOG_TIMER_REG, &reg_val_2nd) != PASSED) {
        printf("Failed to read FPGA register 0x%04X.\n", FPGA_IOS_WATCHDOG_TIMER_REG);
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
 * Description : Register test for External Device Reset Register (0x804) - Bit 21 & 22 PHY Reset
 * Inputs      : opt - reserved for future use
 * Outputs     : PASSED/FAILED
 *
 *******************************************************************************
 */
int fpga_phy_reset_test (int opt)
{

    ushort reg_val = 0;
    char *tname ="FPGA External Device Reset Register - PHY Reset";
    testname(tname);
    prpass(testpass, "%s, ", tname);

    /* Write GE PHY 0 register 22 to 0, set page 0. */
    if (dnv_write_phy_reg(DNV_LAN0_PORT0, VIPER_1514_GE0_PHY_ADDR, VIPER_1514_PAGE_REG, VIPER_1514_PAGE_0) != PASSED) {
        printf("Failed to write PHY %d register.\n", DNV_LAN0_PORT0);
        cterr('f', 0, "FPGA External Device Reset Register - PHY Reset Test Failed");
        return (FAILED);
    }

    msleep(WAITTIME_250_MS);

    /* Read GE PHY 0 register 2 to get PHYID. */
    if (dnv_read_phy_reg(DNV_LAN0_PORT0, VIPER_1514_GE0_PHY_ADDR, VIPER_1514_PHYID_REG, &reg_val) != PASSED) {
        printf("Failed to read PHY %d register.\n", DNV_LAN0_PORT0);
        cterr('f', 0, "FPGA External Device Reset Register - PHY Reset Test Failed");
        return (FAILED);
    }
    
    /* Check if get correct PHYID before phy reset, make sure that phy works normal.*/
    if (reg_val != VIPER_1514_PHYID) {
        printf("Read PHY %d PHYID failed.\n", DNV_LAN0_PORT0);
        cterr('f', 0, "FPGA External Device Reset Register - PHY Reset Test Failed");
        return (FAILED);
    }

    if ((has_dsl_sku() == FALSE)) {
    
        /* Write GE PHY 1 register 22 to 0, set page 0. */
        if (dnv_write_phy_reg(DNV_LAN0_PORT1, VIPER_1514_GE1_PHY_ADDR, VIPER_1514_PAGE_REG, VIPER_1514_PAGE_0) != PASSED) {
            printf("Failed to write PHY %d register.\n", DNV_LAN0_PORT1);
            cterr('f', 0, "FPGA External Device Reset Register - PHY Reset Test Failed");
            return (FAILED);
        }

        msleep(WAITTIME_250_MS);

        /* Read GE PHY 1 register 2 to get PHYID. */
        if (dnv_read_phy_reg(DNV_LAN0_PORT1, VIPER_1514_GE1_PHY_ADDR, VIPER_1514_PHYID_REG, &reg_val) != PASSED) {
            printf("Failed to read PHY %d register.\n", DNV_LAN0_PORT1);
            cterr('f', 0, "FPGA External Device Reset Register - PHY Reset Test Failed");
            return (FAILED);
        }
    
        /* Check if get correct PHYID before phy reset, make sure that phy works normal.*/
        if (reg_val != VIPER_1514_PHYID) {
            printf("Read PHY %d PHYID failed.\n", DNV_LAN0_PORT1);
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

    /* Viper Intel -> SKU with DSL have no GE1 */
    if ((has_dsl_sku() == FALSE)) {	
        /* GE PHY 1 Reset. */
        if (fpga_reset_api(FPGA_EXTER_DEV_RST_REG, FPGA_GEWAN1_RESET, TRUE,
                       WAITTIME_20_MS) != PASSED) {
            cterr('f', 0, "%s: Failed to put GE1 PHY in Reset.\n", __FUNCTION__);
            return (FAILED);
        }
    }
 
    /* Read GE PHY 0 register 22. */
    if (dnv_read_phy_reg(DNV_LAN0_PORT0, VIPER_1514_GE0_PHY_ADDR, VIPER_1514_PAGE_REG, &reg_val) != PASSED) {
        printf("Failed to read PHY %d register.\n", DNV_LAN0_PORT0);
        cterr('f', 0, "FPGA External Device Reset Register - PHY Reset Test Failed");
        return (FAILED);
    }

    if ((NVRAM)->diagflag & D_VERBOSE) {
       printf("Read PHY register value: 0x%x\n", reg_val);
    }

    /* Expect to get 0xFFFF data after phy reset.*/
    if (reg_val != VIPER_1514_PAGE_REG_VAL_FFFF) {
        printf("Still read PHY %d register after phy reset.\n", DNV_LAN0_PORT0);
        cterr('f', 0, "FPGA External Device Reset Register - PHY Reset Test Failed");
        return (FAILED);
    }


    /* Viper Intel -> SKU with DSL have no GE1 */
    if ((has_dsl_sku() == FALSE)) {	
        /* Read GE PHY 1 register 22. */
        if (dnv_read_phy_reg(DNV_LAN0_PORT1, VIPER_1514_GE1_PHY_ADDR, VIPER_1514_PAGE_REG, &reg_val) != PASSED) {
            printf("Failed to read PHY %d register.\n", DNV_LAN0_PORT1);
            cterr('f', 0, "FPGA External Device Reset Register - PHY Reset Test Failed");
            return (FAILED);
        }

        if ((NVRAM)->diagflag & D_VERBOSE) {
           printf("Read PHY register value: 0x%x\n", reg_val);
        }

        /* Expect to get 0xFFFF data after phy reset.*/
        if (reg_val != VIPER_1514_PAGE_REG_VAL_FFFF) {
            printf("Still read PHY %d register after phy reset.\n", DNV_LAN0_PORT1);
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

    /* Set up DSL Ethernet ENV because already re-init  linux module ixgbe.ko*/
    if (has_dsl_sku() == TRUE) {
        viper_dsl_env_setup();
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

    /* Write register(0xD10) to enable Wadtch Dog boot timer  */
    /* 0xABCD0001 is pattern to enable WDT */
    if (this_is_viper_j() == FALSE) {
        reg_val = FOXCONN_WDT_ENABLE_PATTERN;
        fpga_write_reg(FPGA_WDT_ENABLE_REG, reg_val);
    } else {
        reg_val = PEGATRON_WDT_ENABLE_PATTERN;
        fpga_write_reg(FPGA_WDT_ENABLE_REG, reg_val);
        msleep(WAITTIME_250_MS);
        /* Need to write 0 to disable register (0xD10), otherwise, it will keep re-trigger WDT.*/
        reg_val = PEGATRON_WDT_DISABLE_PATTERN;
        fpga_write_reg(FPGA_WDT_ENABLE_REG, reg_val);
    }

    /* Waiting for WDT count down to 295000ms */
    msleep(WAITTIME_5000_MS);
    /* Read back the register(0x64) to verify WDT is count down  */
    fpga_read_reg(FPGA_WATCHDOG_BOOT_TIMER, &reg_val);
    /* If WDT count down the value should be around 295000ms*/
    if (reg_val >= TIME_299000MS) {
        cterr('f', 0, "%s: FPGA timer didn't count down\n", __FUNCTION__);
        return(FAILED);
    }
    
    /* Write pattern 0xBD000000 to reg 0x64 to disable WDT */
    reg_val = WDT_DISABLE_PATTERN;
    fpga_write_reg(FPGA_WATCHDOG_BOOT_TIMER, reg_val);

    /* Read back the register(0x64) to verify WDT is zero for disable  */
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
    /* Rommon will disable WDT by default so WDT vaule is zero */
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
 * Function    : diag_fpga_esw_reset_test
 * Description : Test for ethernet switch reset test
 * Inputs      : none
 * Outputs     : PASSED/FAILED
 *
 *******************************************************************************
 */
int diag_fpga_esw_reset_test (void)
{
    uint ret_val = PASSED;
    ushort read_back_buff = 0;
    char *tname ="FPGA Ethernet Switch Reset Test";
    testname(tname);
    prpass(testpass, "%s, ", tname);
    
    /* Run SMI Register Test */
    ret_val = diag_smi_reg_test();
    if (ret_val == FAILED) {
        cterr('f', 0, "%s: Failed to ESW register test.\n", __FUNCTION__);
        return (FAILED);
    }
    /* Ethernet Switch reset */
    if (fpga_reset_api(FPGA_EXTER_DEV_RST_REG, EXT_ESW_RESET, TRUE,
                       WAITTIME_20_MS) != PASSED) {
        cterr('f', 0, "%s: Failed to put GE0 PHY in Reset.\n", __FUNCTION__);
        return (FAILED);
    }
    /* Delay 5 sec to make sure ESW is under reset status  */
    msleep(WAITTIME_5000_MS);
    
    /* Confirm SMI bus is ready for access.
     * Since switch(Marvell 88E6176) is set to multi chip address mode,
     * this is by checking SMIBusy(bit15) of SMI Command Register(0x0).
     */
    if (dnv_read_phy_reg(DNV_LAN1_PORT0, VIPER_6176_PHY_ADDR, ESW_SMI_CMD_REG,
                         &read_back_buff) != PASSED) {
        printf("Failed to read ESW PHY  register.\n");
        cterr('f', 0, "FPGA External Device Reset Register - ESW Reset Test Failed");
        return (FAILED);
    }

    /* Expect to get 0xFFFF data after ESW reset */
    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("SMI Command Reg = %x\n", read_back_buff);
    }
    if (read_back_buff != VIPER_6176_SMI_REG_VAL_FFFF) {
        cterr('f', 0, "Still read ESW SMI register after esw reset."
                      "FPGA fail to reset Ethernet Switch");
        ret_val = FAILED;
    }
 

    /* Ethernet Switch un-reset */
    if (fpga_reset_api(FPGA_EXTER_DEV_RST_REG, EXT_ESW_RESET, FALSE,
                       WAITTIME_20_MS) != PASSED) {
        cterr('f', 0, "%s: Failed to put Ethernet Switch in Reset.\n", __FUNCTION__);
        return (FAILED);
    }

    prcomplete(testpass, errcount, (char *)0);
    return (ret_val);

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
    /* Get FPGa board type */
    fpga_read_reg(FPGA_BOARD_TYPE_REG, &reg_val);

    if ((reg_val & FPGA_BTYPE_PRODUCT_SKU_BIT) == FPGA_BTYPE_PRODUCT_SKU_BIT) {
        /* Foxconn board reg 0x8C0 bit 5 is 1 */
        if((strncmp(mb_get_pid, "C921J-4P", strlen("C921J-4P")) == 0) || 
           (strncmp(mb_get_pid, "C931J-4P", strlen("C931J-4P")) == 0) ||
           (strncmp(mb_get_pid, "C926-4P", strlen("C926-4P")) == 0) ||
           (strncmp(mb_get_pid, "C927-4P", strlen("C927-4P")) == 0) ||
           (strncmp(mb_get_pid, "C927-4PM", strlen("C927-4PM")) == 0) ||
           (strncmp(mb_get_pid, "C926-4PLTEGB", strlen("C926-4PLTEGB")) == 0) ||
           (strncmp(mb_get_pid, "C927-4PLTEGB", strlen("C927-4PLTEGB")) == 0) ||
           (strncmp(mb_get_pid, "C927-4PMLTEGB", strlen("C927-4PMLTEGB")) == 0) ||
           (strncmp(mb_get_pid, "C927-4PLTEAU", strlen("C927-4PLTEAU")) == 0)) {
            printf("PID:%s\n",mb_get_pid); 
        } else {
            printf("The PID and FPGA Board type are not match\n");
            printf("PID:%s\n",mb_get_pid); 
            cterr('f', 0, "FPGA  Board Type test Failed");
            return (FAILED);
        }

    } else if ((reg_val & FPGA_BTYPE_PRODUCT_SKU_BIT) == 0) {
        /* Pegatron board reg 0x8C0 bit 5 is 0 */
        if((strncmp(mb_get_pid, "C941J-4P", strlen("C941J-4P")) == 0) || 
           (strncmp(mb_get_pid, "C921-4P", strlen("C921-4P")) == 0) ||
           (strncmp(mb_get_pid, "C931-4P", strlen("C931-4P")) == 0) ||
           (strncmp(mb_get_pid, "C921-4PLTEGB", strlen("C921-4PLTEGB")) == 0) ||
           (strncmp(mb_get_pid, "C921-4PLTEAS", strlen("C921-4PLTEAS")) == 0) ||
           (strncmp(mb_get_pid, "C921-4PLTEAU", strlen("C921-4PLTEAU")) == 0) ||
           (strncmp(mb_get_pid, "C921-4PLTENA", strlen("C921-4PLTENA")) == 0)) {
            printf("PID:%s\n",mb_get_pid); 
        } else {
            printf("The PID and FPGA Board type are not match\n");
            printf("PID:%s\n",mb_get_pid); 
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

/******** History ******** 
 * $Log: diag_fpga_test.c,v $
 * Revision 1.6  2018/12/06 01:56:09  harrchan
 * Add modem detection on power status and control test (CSCvn46827)
 *
 * Revision 1.5  2018/11/09 07:33:24  yungchen
 * Merge viper branch4 to the main trunk (CSCvn11857)
 *
 * Revision 1.4  2018/10/11 06:02:59  harrchan
 * Add FPGA function test (CSCvm72986)
 *
 * Revision 1.3  2018/08/31 03:59:30  chieyang
 * Add SPI flash utility, show memory size and xdsl test modification. Merge from viper-branch2
 *
 * Revision 1.2  2018/08/06 02:31:50  harrchan
 * Merge viper E2E to the main trunk (CSCvk28469)
 *
 * Revision 1.1.2.16  2018/07/06 02:54:08  harrchan
 * Add enhance error message
 *
 * Revision 1.1.2.15  2018/07/03 05:38:55  harrchan
 * Follow the coding rule to clean up code
 *
 * Revision 1.1.2.14  2018/06/27 06:27:49  harrchan
 * Follow the coding rule to clean up code
 *
 * Revision 1.1.2.13  2018/06/25 11:35:44  lucywang
 * Modified FPGA utility name
 *
 * Revision 1.1.2.12  2018/06/11 10:57:53  lucywang
 * Added utility to enable FPGA IOS Watchdog
 *
 * Revision 1.1.2.11  2018/05/25 08:08:13  olin2
 * Support system reset and display reset reason util
 *
 * Revision 1.1.2.10  2018/05/24 08:43:49  lucywang
 * Modified FPGA upgrade utility name to be clearer
 *
 * Revision 1.1.2.9  2018/05/15 05:36:37  lucywang
 * Modified for ViperJ based on Cisco FPGA
 *
 * Revision 1.1.2.8  2018/05/10 05:51:21  olin2
 * Support voltage margin util for Viper-Intel
 *
 * Revision 1.1.2.7  2018/04/13 11:19:12  lucywang
 * Modified to use Cisco FPGA : 1) Upgrade 2) LED 3) FPGA register 4) FPGA I2C reset
 *
 * Revision 1.1.2.6  2018/04/10 06:17:15  harrchan
 * Modify FPGA register address
 *
 * Revision 1.1.2.5  2018/03/29 10:47:31  lucywang
 * Changed FPGA regster test for ViperJ temporarily
 *
 * Revision 1.1.2.4  2018/03/16 06:51:04  harrchan
 * Update FPGA register table
 *
 * Revision 1.1.2.3  2018/03/16 02:12:29  harrchan
 * Change FPGA register test offset to watchdog boot timer(0x864)
 *
 * Revision 1.1.2.2  2018/03/15 08:26:16  harrchan
 * Change I/O access to memory map
 *
 * Revision 1.1.2.1  2018/02/27 08:06:42  harrchan
 * Initial viper application code base
 *
 *
 * $Endlog$
 * */
