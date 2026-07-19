/* $Id: mb_tests.c,v 1.1 2020/08/19 09:49:35 markzha Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/highrise/mb_tests.c,v $
 *------------------------------------------------------------------
 *
 * mb_test.c
 *
 * Copyright (c) 2019-2029 by cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include "common.h"
#include "error.h"
#include "queryflags.h"
#include "nvsysvars.h"
#include "menu.h"
#include "proto.h"
#include "plat_defs.h"
#include "platform_fru.h"
#include "highrise_cpld_diag.h"
#include "diag_lte_test.h"

/* Extern functions */
extern int highrise_display_temp(void);
extern int do_all_menu_items(struct menuinfo *);
extern int linux_memory_tester(int);
extern int cpu_core_test (int do_more_test);
extern int build_cpu_test_menu (int db_test_items_executed);
extern int emmc_slot_tests(int);
extern int spi_slot_tests(int, const char *);
extern int highrise_i2c_scan_test(int);
extern int build_rtc_menu(boolean);
extern void time_validity_test_wrapper (void);
extern int build_ts_menu(int);
extern int diag_phy_test(boolean);
extern int highrise_uart_test(void);
extern int hr_cpld_pwr_st_chk_tst(void);

extern int quiet_launch;


/* Declare functions */
int emmc_tests (int option);
int bootflash_tests (int option);
int nandflash_test(int option);



static submenu_xtable_t mb_test_table[] = {
    {"Linux memory test",
     (PFT)linux_memory_tester, FALSE,      
     MF_CONTINUOUS | MF_DOGRP | MF_DOALL | MF_SHOW_ERRCOUNT,
     (type_t(*)())0, 0,
	 (PFT)linux_memory_tester, TRUE},

    {"CPU core test",
    (PFT) cpu_core_test, FALSE,
    MF_CONTINUOUS | MF_DOGRP | MF_DOALL | MF_SHOW_ERRCOUNT,
    (type_t(*)())0, 0,
    (PFT) build_cpu_test_menu, FALSE},

    {"eMMC test",
    (PFT) emmc_tests, FALSE,
    MF_CONTINUOUS | MF_DOGRP | MF_DOALL | MF_SHOW_ERRCOUNT,
    (type_t(*)())0, 0,
    (PFT) 0, 0},

    {"Bootflash test",
    (PFT) bootflash_tests, FALSE,
    MF_CONTINUOUS | MF_DOGRP | MF_DOALL | MF_SHOW_ERRCOUNT,
    (type_t(*)())0, 0,
    (PFT) 0, 0},

    {"Nand Flash test",
    (PFT) nandflash_test, FALSE,
    MF_CONTINUOUS | MF_DOGRP | MF_DOALL | MF_SHOW_ERRCOUNT,
    (type_t(*)())0, 0,
    (PFT) 0, 0},

    {"CPLD Tests",
    (PFT) build_hr_cpld_menu, FALSE,
    MF_CONTINUOUS | MF_DOGRP | MF_DOALL | MF_SHOW_ERRCOUNT,
    (type_t(*)())0, 0,
    (PFT) build_hr_cpld_menu, TRUE},

    {"I2C scan test",
    (PFT) highrise_i2c_scan_test, FALSE,
    MF_CONTINUOUS | MF_DOGRP |MF_DOALL | MF_SHOW_ERRCOUNT,
    (type_t(*)())0, 0,
    (type_t(*)())0, 0 },

    {"RTC test",
    (PFT) build_rtc_menu, FALSE,
    MF_CONTINUOUS | MF_DOGRP |MF_DOALL | MF_SHOW_ERRCOUNT,
    (type_t(*)())0, 0,
    (PFT) build_rtc_menu, TRUE},

    {"Temp Sensor Test",
    (PFT) build_ts_menu, FALSE,
    MF_CONTINUOUS | MF_DOGRP |MF_DOALL | MF_SHOW_ERRCOUNT,
    (type_t(*)())0, 0,
    (PFT) build_ts_menu, TRUE},

    {"LTE Test",
    (PFT) diag_lte_telit_main, TRUE,
    MF_CONTINUOUS | MF_DOALL | MF_SHOW_ERRCOUNT,
    (type_t(*)())0, 0, (PFT) diag_lte_telit_main , FALSE},

    {"PHY Test",
    (PFT) diag_phy_test, TRUE,
    MF_CONTINUOUS | MF_DOALL | MF_SHOW_ERRCOUNT,
    (type_t(*)())0, 0, (PFT) diag_phy_test, FALSE},

    {"Aux Test",
    (PFT) highrise_uart_test, TRUE,
    MF_CONTINUOUS | MF_DOALL | MF_SHOW_ERRCOUNT,
    (type_t(*)())0, 0, (PFT) highrise_uart_test, FALSE},

    {"Power Status Check Test",
    (PFT) hr_cpld_pwr_st_chk_tst, TRUE,
    MF_CONTINUOUS | MF_SHOW_ERRCOUNT | MF_HIDDEN_EXE,
    (type_t(*)())0, 0, (PFT)NULL, FALSE},
};

#define MB_TEST_TABLE_SZ \
        (sizeof(mb_test_table) / sizeof(submenu_xtable_t))

/*
 * primary & secondary submenu items (filled in from xtable)
 */
static mitem_t mb_pri_test_items[MB_TEST_TABLE_SZ + MAX_BASE_ITEMS];
static mitem_t mb_sec_test_items[MB_TEST_TABLE_SZ + MAX_BASE_ITEMS];

static menuinfo_t mb_test_menu = {
    "Mainboard Test Menu",
    0,                            /* mtparam added by init_empty_menu */
    (PFT)menu_show_dflags,        /* shows major flags */
    0,                            /* use generic prompt */
    0,                            /* size (bumped by add_menu_item() */
    mb_pri_test_items,
};
static menuinfo_t *mb_test_menup = &mb_test_menu;


/**********************************************************************
 *
 * Function: build_mb_test_menu
 *
 * Description: Build CPU test menu.
 *
 * Inputs: db_test_items_executed - TRUE for do all of tests. FALSE for submenu.
 *
 * Outputs: PASSED/FAILED.
 *
 **********************************************************************
 */
int build_mb_test_menu (int db_test_items_executed)
{
    int rc = FAILED;
    char *tname = "Mainboard";

    testname(tname);

    build_primary_submenu(mb_test_table,
                          MB_TEST_TABLE_SZ, "Mainboard",
                          &mb_test_menup);

    build_secondary_submenu(mb_test_table,
                            MB_TEST_TABLE_SZ,
                            mb_sec_test_items);

    if (db_test_items_executed) {
        do_all_menu_items(&mb_test_menu);
    } else {
        menu(&mb_test_menu, mb_sec_test_items, '\0');
    }

    return (rc);
}


static void display_no_reg (void)
{
    cterr_db_print("This item has no reg to display\n"); 
}

/**************************************************************
 * Enhance Error Function
 * 1. Subtests of the test function will reuse all variables
 * 2. All variables will be cleared automatically when
 *    entering and leaving each menu item.
 * Segment 1: PID | Unique_string : slot_info 
 *      fru_table_offset should be set, otherwise, it will not 
 *      go to enhanced error message format in cterr() 
 *      set fru_table_offset to get the predefine value 
 *      or change mb_pid & mb_loc 
 * Segment 2: Test step captured from prpass 
 * Segment 3: Failure message captured from cterr 
 * Segment 4: Components used 
 * Segment 5: register and memory dump 
 * Segment 6: Platform Environment initialized here
 * Segment 7: Top 3 Debugging Steps 
 **************************************************************/
static void add_emmc_err_report(void) 
{
    fru_table_offset = MB;
    platform_fru_table[fru_table_offset].pid_string = mb_pid;
    platform_fru_table[fru_table_offset].location_string = mb_loc;

    cterr_add_component("Marvell Armada 7040", "eMMC Storage Flash");
    cterr_add_reg_dump((PFV)display_no_reg);
    cterr_add_env_dump((PFV)highrise_display_temp);
    cterr_add_debug("Check the interface between the Host SoC and the eMMC.",
                    "If there is no problem for these interfaces, "
                    "replace one eMMC and redo the test.");
}


/**************************************************************
 * Function: emmc_tests
 * Description : emmc r/w tests.
 * Inputs: option for future use
 * Output: PASSED/FAILED
 **************************************************************/
int emmc_tests (int option)
{
    if (get_enhance_err_flag()) {
        add_emmc_err_report();
    }

    int rc = FAILED;
    char *tname = "eMMC";
    testname(tname);
    prpass(testpass, "%s, ", tname);

    if (!quiet_launch) {
        prpass(testpass, "%s read/write, ", tname);
    }
    /*
     * testname is printed on usb_slot_tests
     */
    rc = emmc_slot_tests(option);
    if (rc == FAILED) {
        cterr('f', 0, "%s test failed.", tname);
        return (rc);
    }
    if (!quiet_launch) {
        prpass(testpass, "%s read/write test passed, ", tname);
    }
    return (rc);
}


/**************************************************************
 * Enhance Error Function
 * 1. Subtests of the test function will reuse all variables
 * 2. All variables will be cleared automatically when
 *    entering and leaving each menu item.
 * Segment 1: PID | Unique_string : slot_info 
 *      fru_table_offset should be set, otherwise, it will not 
 *      go to enhanced error message format in cterr() 
 *      set fru_table_offset to get the predefine value 
 *      or change mb_pid & mb_loc 
 * Segment 2: Test step captured from prpass 
 * Segment 3: Failure message captured from cterr 
 * Segment 4: Components used 
 * Segment 5: register and memory dump 
 * Segment 6: Platform Environment initialized here
 * Segment 7: Top 3 Debugging Steps 
 **************************************************************/
static void add_bootflash_err_report(void)
{
    fru_table_offset = MB;
    platform_fru_table[fru_table_offset].pid_string = mb_pid;
    platform_fru_table[fru_table_offset].location_string = mb_loc;

    cterr_add_component("Marvell Armada 7040", "SPI", "BootFlash");
    cterr_add_reg_dump((PFV)display_no_reg );
    cterr_add_env_dump((PFV)highrise_display_temp);
    cterr_add_debug("Boot up is OK means interfaces between Host SoC and flash is OK.",
                    "If there is no problem for these interfaces, "
                    "replace one flash and redo the test.");
}



/**************************************************************
 * Function: bootflash_tests
 * Description : bootflash r/w tests.
 * Inputs: option - for future use
 * Output: PASSED/FAILED
 **************************************************************/
int bootflash_tests (int option)
{
    if (get_enhance_err_flag()) {
        add_bootflash_err_report();
    }

    char *tname = "Bootflash";
    testname("%s", tname);
    prpass(testpass, "%s, ", tname);

    if (spi_slot_tests(option, BOOTFLASH_BLK) != PASSED) {
        cterr('f', 0, "SPI bootflash test failed.");
        return (FAILED);
    }
    prpass(testpass, "%s test passed, ", tname);
    return (PASSED);
}

int nandflash_test(int option)
{
    char *tname = "NandFlash";
    testname("%s", tname);
    prpass(testpass, "%s, ", tname);

    if (access(NAND_FLASH_MTD_BLK, F_OK) == 0) {
        if (spi_slot_tests(option, NAND_FLASH_MTD_BLK) != PASSED) {
            cterr('f', 0, "SPI Nand Falsh test failed.\n");
            return FAILED;
        }
        printf("\nPASSED\n");
    }
    else {
        printf("\nNand flash removed from P1A board, skip\n");
    }
    return PASSED;
}
