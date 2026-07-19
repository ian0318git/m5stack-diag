/* $Id: mb_tests.c,v 1.4 2019/07/11 12:31:31 alicehua Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/nutella/mb_tests.c,v $
 *------------------------------------------------------------------
 *
 * mb_tests.c - M/B test wraps.
 *
 *
 * Copyright (c) 2008-2019 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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
#include "plat_defs.h"
#include "i2c_api.h"
#include "diag_ddr4_lib.h"
#include "diag_i2c_lib.h"
#include "diag_storage_lib.h"
#include "diag_usb_lib.h"
#include "diag_usb_test.h"
#include "diag_temp_snsr_test.h"
#include "diag_gephy_test.h"
#include "diag_led_test.h"
#include "diag_rtc_test.h"
#include "platform_cpu.h"
#include "diag_reset_button.h"
#include "diag_i350_test.h"
#include "diag_i2c_test.h"

/*
 * Global variables
 */
fru_table_t platform_fru_table[];


/* FRU PID and Location Strings */
uchar mb_pid[] = "MB-PID";
uchar mb_loc[] = "MB";

fru_table_t platform_fru_table[] = {
    {mb_pid, mb_loc},
};

/*
 * Sub Menu used for "Main menu -> motherboard test"
 */
submenu_xtable_t mb_tests_submenu_table[] = {
    {"CPU core test",
    (PFT) cpu_core_test, FALSE,
    MF_CONTINUOUS | MF_DOGRP | MF_DOALL | MF_SHOW_ERRCOUNT,
    (type_t(*)())0, 0,
    (PFT) build_cpu_test_menu, FALSE},

    {"Main Memory test with cache on and ECC checking",
    (PFT) linux_memory_tester_with_ecc_check, FALSE,
    MF_CONTINUOUS | MF_DOGRP | MF_DOALL | MF_SHOW_ERRCOUNT,
    (type_t(*)())0, 0, (PFT) linux_memory_tester_with_ecc_check, TRUE},

    {"Boot flash test",
    (PFT) build_boot_flash_menu, FALSE,
    MF_CONTINUOUS | MF_DOGRP | MF_DOALL| MF_SHOW_ERRCOUNT,
    (type_t(*)())0, 0, 
    (PFT) build_boot_flash_menu, TRUE},

    {"I2C scan test",
    (PFT) diag_i2c_scan_test, FALSE,
    MF_CONTINUOUS | MF_DOGRP | MF_DOALL | MF_SHOW_ERRCOUNT,
    (type_t(*)())0, 0,
    (type_t(*)())0, 0},

    {"eMMC test",
    (PFT) build_emmc_test_menu, FALSE,
    MF_CONTINUOUS | MF_DOGRP | MF_DOALL | MF_SHOW_ERRCOUNT,
    (type_t(*)())0, 0,
    (PFT) build_emmc_test_menu, TRUE},

    {"FPGA test",
    (PFT) build_fpga_test_menu, FALSE,
    MF_CONTINUOUS | MF_DOGRP | MF_DOALL | MF_SHOW_ERRCOUNT,
    (type_t(*)())0, 0,
    (PFT) build_fpga_test_menu, TRUE},

    {"Thermal Sensor test",
    (PFT) build_snsr_menu, FALSE,
    MF_CONTINUOUS | MF_DOGRP | MF_DOALL | MF_SHOW_ERRCOUNT,
    (type_t(*)())0, 0,
    (PFT) build_snsr_menu, TRUE},

    {"External USB test",
    (PFT) diag_ext_usb_test, FALSE,
    MF_CONTINUOUS | MF_DOGRP | MF_DOALL | MF_SHOW_ERRCOUNT,
    (type_t(*)())0, 0,
    (PFT) diag_ext_usb_test, TRUE},

    {"MRV88E1543 GE PHY Test",
    (PFT) build_gephy_test_menu, FALSE,
    MF_CONTINUOUS | MF_DOGRP | MF_DOALL | MF_SHOW_ERRCOUNT,
    (type_t(*)())0, 0, 
    (PFT) build_gephy_test_menu, TRUE},

    {"I350 Test",
    (PFT) diag_i350_test , FALSE,
    MF_CONTINUOUS | MF_DOGRP  | MF_DOALL | MF_SHOW_ERRCOUNT,
    (type_t(*)())has_sfp_sku, 0,
    (PFT) diag_i350_test, TRUE},

    {"LED Test",
    (PFT) diag_led_test , TRUE,
    MF_CONTINUOUS | MF_DOGRP  | MF_SHOW_ERRCOUNT,
    (type_t(*)())0, 0,(PFT) diag_led_test, FALSE},

    {"RTC test",
    (PFT) build_rtc_menu, TRUE,
    MF_CONTINUOUS | MF_SHOW_ERRCOUNT,
    (type_t(*)())0, 0,
    (PFT) build_rtc_menu, FALSE},
    
    {"Reset button test",
    (PFT)nutella_reset_button_test, FALSE,
    MF_CONTINUOUS | MF_SHOW_ERRCOUNT,
    (type_t(*)())0, 0,
    (PFT)0,         0},
};

#define MB_TESTS_SUBMENU_TABLE_SIZE (sizeof(mb_tests_submenu_table) / \
                                     sizeof(submenu_xtable_t))

/*
 * "Main menu -> motherboard test" primary & secondary submenu items (filled in from xtable)
 */
static mitem_t mb_tests_primary_items[MB_TESTS_SUBMENU_TABLE_SIZE +
                                      MAX_BASE_ITEMS];
static mitem_t mb_tests_secondary_items[MB_TESTS_SUBMENU_TABLE_SIZE +
                                        MAX_BASE_ITEMS];

menuinfo_t mb_subtest_menu = {
    "%s Subtest Menu",
    0,                          /* mtparam added by init_empty_menu */
    (PFT) show_endnote,         /* notes missing WICs in combos */
    0,                          /* use generic prompt */
    0,                          /* size (bumped by add_menu_item() */
    mb_tests_primary_items,
};

menuinfo_t *mb_submenup = &mb_subtest_menu;


/*-------------------------------------------------------------------
 *
 * Function: mb_tests()
 *
 * First build the primary & secondary submenus for the motherboard
 * diags based on the _xtable_ mb_tests_submenu_table.  If the given
 * arg is TRUE, execute all the tests in the menu flagged with
 * MF_DOALL, and return the result.  Otherwise, present the menu to the
 * user for interaction.
 *
 */
int mb_tests (boolean mb_test_items_executed)
{
    int rc = FAILED;

    build_primary_submenu(mb_tests_submenu_table,
                          MB_TESTS_SUBMENU_TABLE_SIZE, "Motherboard",
                          &mb_submenup);

    build_secondary_submenu(mb_tests_submenu_table,
                            MB_TESTS_SUBMENU_TABLE_SIZE,
                            mb_tests_secondary_items);

    if (mb_test_items_executed) {
        do_all_menu_items(&mb_subtest_menu);
    } else {
        menu(&mb_subtest_menu, mb_tests_secondary_items, '\0');
    }

    return (rc);
}

/*-------------------------------------------------
$Log: mb_tests.c,v $
Revision 1.4  2019/07/11 12:31:31  alicehua
Collapse Nutella codes into main trunk.

$Endlog$
*/
