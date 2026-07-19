 /* $Id: mb_tests.c,v 1.4 2018/09/21 02:48:54 harrchan Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/viper/mb_tests.c,v $
 *------------------------------------------------------------------
 *
 * mb_tests.c - M/B test wraps.
 *
 *
 * Copyright (c) 2008-2018 by Cisco Systems, Inc.
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
    (PFT) viper_i2c_scan_test, FALSE,
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

    {"GE PHY 0 Test",
    (PFT) build_gephy0_test_menu, FALSE,
    MF_CONTINUOUS | MF_DOGRP | MF_DOALL | MF_SHOW_ERRCOUNT,
    (type_t(*)())0, 0, 
    (PFT) build_gephy0_test_menu, TRUE},

    {"GE PHY 1 Test",
    (PFT) build_gephy1_test_menu, FALSE,
    MF_CONTINUOUS | MF_DOGRP | MF_DOALL | MF_SHOW_ERRCOUNT,
    (type_t(*)())has_ge1_sku, 0, 
    (PFT) build_gephy1_test_menu, TRUE},

    {"Ethernet Switch Test",
    (PFT) build_esw_test_menu, FALSE,
    MF_CONTINUOUS | MF_DOGRP | MF_DOALL | MF_SHOW_ERRCOUNT,
    (type_t(*)())0, 0, 
    (PFT) build_esw_test_menu, TRUE},

    {"LED Test",
    (PFT) diag_led_test , TRUE,
    MF_CONTINUOUS | MF_DOGRP  | MF_SHOW_ERRCOUNT,
    (type_t(*)())0, 0,(PFT) diag_led_test, FALSE},

    {"RTC test",
    (PFT) build_rtc_menu, TRUE,
    MF_CONTINUOUS | MF_DOGRP | MF_SHOW_ERRCOUNT,
    (type_t(*)())0, 0,
    (PFT) build_rtc_menu, FALSE},
    
    {"Reset button test",
    (PFT)viper_reset_button_test, FALSE,
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
 * $Log: mb_tests.c,v $
 * Revision 1.4  2018/09/21 02:48:54  harrchan
 * Merge viper DSL to the main trunk (CSCvm57542)
 *
 * Revision 1.3  2018/08/31 03:59:30  chieyang
 * Add SPI flash utility, show memory size and xdsl test modification. Merge from viper-branch2
 *
 * Revision 1.2  2018/08/06 02:31:52  harrchan
 * Merge viper E2E to the main trunk (CSCvk28469)
 *
 * Revision 1.1.2.9  2018/07/03 05:38:55  harrchan
 * Follow the coding rule to clean up code
 *
 * Revision 1.1.2.8  2018/05/29 01:47:03  harrchan
 * Add ECC error report
 *
 * Revision 1.1.2.7  2018/05/09 07:11:26  olin2
 * 1. Move GE and DSL init to the beginning. 2. Add has GE1. 3. Show cookie info
 *
 * Revision 1.1.2.6  2018/03/29 10:35:44  lucywang
 * Added Reset button test
 *
 * Revision 1.1.2.5  2018/03/29 01:11:19  lucywang
 * Added RTC test and utility
 *
 * Revision 1.1.2.4  2018/03/28 09:18:13  lucywang
 * Added CPU test
 *
 * Revision 1.1.2.3  2018/03/27 09:16:54  harrchan
 * Move led test to mother board test menu
 *
 * Revision 1.1.2.2  2018/03/27 07:12:10  lucywang
 * Modified USB test for 2.0 and 3.0
 *
 * Revision 1.1.2.1  2018/02/27 08:06:50  harrchan
 * Initial viper application code base
 *
 *
 * $Endlog$
 *-------------------------------------------------
 */
