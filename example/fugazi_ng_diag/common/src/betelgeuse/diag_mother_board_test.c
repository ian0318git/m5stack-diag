/* $Id: diag_mother_board_test.c,v 1.2 2019/01/10 06:36:23 wilbhuan Exp $ 
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/betelgeuse/diag_mother_board_test.c,v $
 *------------------------------------------------------------------
 * 
 * diag_mother_board_test.c
 *
 * Copyright (c) 2018 - 2019 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/un.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <linux/types.h>
#include <linux/netlink.h>
#include <errno.h>
#include <unistd.h>
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
#include "diag_mother_board_test.h"
#include "setjmps.h"
#include "proto.h"
#include "diag_moka_fpga_lib.h"
#include "diag_enhance_err_msg_lib.h"
#include "platform_cookie.h"
#include "diag_sirius_fpga_test.h"
#include "diag_reset_button_test.h"
#include "diag_ge_phy_test.h"
#include "diag_rtc_test.h"
#include "diag_temp_sensor_test.h"
#include "diag_esw_test.h"
#include "diag_emmc_test.h"
#include "diag_led_test.h"
#include "diag_moka_fpga_test.h"
#include "diag_cpu_test.h"
#include "diag_cpu_lib.h"
#include "diag_i2c_test.h"
#include "diag_usb_lib.h"
#include "diag_usb_test.h"
#include "diag_spi_test.h"

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
    {"Main memory test with cache on",
     (PFT) linux_memory_tester, FALSE,
     MF_CONTINUOUS | MF_DOGRP | MF_DOALL | MF_SHOW_ERRCOUNT,
     (type_t(*)())0, 0, (PFT) linux_memory_tester, TRUE},

    {"External USB test",
    (PFT) diag_usb_test, USB_SLOT0,
    MF_CONTINUOUS | MF_DOGRP | MF_DOALL | MF_SHOW_ERRCOUNT,
    (type_t(*)())0, 0,
    (PFT) 0, 0},
    
    {"eMMC test",
    (PFT)diag_emmc_test, TRUE,
    (MF_CONTINUOUS | MF_DOGRP | MF_DOALL | MF_SHOW_ERRCOUNT),
    (type_t(*)())0, 0,
    (PFT)diag_emmc_test, FALSE},
    
    {"Bootflash test",
    (PFT) diag_bootflash_test, PLAT_BF_BUSNUM,
    MF_CONTINUOUS | MF_DOGRP | MF_DOALL | MF_SHOW_ERRCOUNT,
    (type_t(*)())0, 0,
    (PFT) 0, 0},
    
    {"I2C scan test",
    (PFT) diag_i2c_scan_test, FALSE,
    MF_CONTINUOUS | MF_DOGRP | MF_DOALL | MF_SHOW_ERRCOUNT,
    (type_t(*)())0, 0,
    (type_t(*)())0, 0},

    {"M/B Temperature test",
    (PFT)diag_temp_sensor_test, FALSE,
    MF_CONTINUOUS | MF_DOGRP | MF_DOALL | MF_SHOW_ERRCOUNT,
    (type_t(*)())0, 0,
    (PFT)diag_temp_sensor_test, TRUE},

    {"RTC test",
    (PFT)diag_rtc_test, FALSE,
    MF_CONTINUOUS | MF_DOGRP | MF_DOALL | MF_SHOW_ERRCOUNT,
    (type_t(*)())0, 0,
    (PFT)diag_rtc_test, TRUE},

    {"GE PHY 0 test",
     (type_t(*)())diag_88e1112_ge0_test, FALSE,
     MF_CONTINUOUS | MF_DOGRP | MF_DOALL | MF_SHOW_ERRCOUNT,
     (type_t(*)())0, 0,
     (type_t(*)())diag_88e1112_ge0_test, TRUE},

    {"GE PHY 1 test",
     (type_t(*)())diag_88e1112_ge1_test, FALSE,
     MF_CONTINUOUS | MF_DOGRP | MF_DOALL | MF_SHOW_ERRCOUNT,
     (type_t(*)())platform_has_2nd_ge, 0,
     (type_t(*)())diag_88e1112_ge1_test, TRUE},

    {"Ethernet Switch test",
     (type_t(*)())diag_esw_test, FALSE,
     MF_CONTINUOUS | MF_DOGRP | MF_DOALL | MF_SHOW_ERRCOUNT,
     (type_t(*)())0, 0,
     (type_t(*)())diag_esw_test, TRUE},

    {"CPU test",
    (PFT)diag_cpu_test, TRUE,
    MF_CONTINUOUS | MF_DOGRP | MF_DOALL | MF_SHOW_ERRCOUNT,
    (type_t(*)())0, 0,
    (PFT)diag_cpu_test, FALSE},

    {"LED test",
    (PFT)diag_led_test, TRUE,
    MF_CONTINUOUS | MF_DOGRP | MF_DOALL | MF_SHOW_ERRCOUNT,
    (type_t(*)())0, 0,
    (PFT)diag_led_test, FALSE},

    {"Reset button test",
    (PFT)diag_reset_button_test, FALSE,
    MF_CONTINUOUS | MF_SHOW_ERRCOUNT,
    (type_t(*)())0, 0,
    (PFT)0,         0},

    {"Platform FPGA test(MOKA & Aikido)",
     (type_t(*)())diag_fpga_test, FALSE,
     MF_CONTINUOUS | MF_DOGRP | MF_DOALL | MF_SHOW_ERRCOUNT,
     (type_t(*)())0, 0,
     (type_t(*)())diag_fpga_test, TRUE},

    {"Pluggabble FPGA test(Sirius)",
     (type_t(*)())diag_plug_fpga_test, FALSE,
     MF_CONTINUOUS | MF_DOGRP | MF_DOALL | MF_SHOW_ERRCOUNT,
     (type_t(*)())platform_has_pluggable, 0,
     (type_t(*)())diag_plug_fpga_test, TRUE},
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
 * Function: diag_mother_board_test()
 *
 * First build the primary & secondary submenus for the motherboard
 * diags based on the _xtable_ mb_tests_submenu_table.  If the given
 * arg is TRUE, execute all the tests in the menu flagged with
 * MF_DOALL, and return the result.  Otherwise, present the menu to the
 * user for interaction.
 *
 */
int diag_mother_board_test (boolean mb_test_items_executed)
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
 * $Log: diag_mother_board_test.c,v $
 * Revision 1.2  2019/01/10 06:36:23  wilbhuan
 * The beginning of Betelgeuse application code.
 *
 *-------------------------------------------------
 */
