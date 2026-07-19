 /* $Id: mb_tests.c,v 1.2 2019/10/17 02:16:25 kehuang2 Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/tabei-l/mb_tests.c,v $
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
#include "diag_usb_lib.h"
#include "diag_usb_test.h"
#include "diag_temp_snsr_test.h"
#include "diag_gephy_test.h"
#include "diag_led_test.h"
#include "diag_rtc_test.h"
#include "platform_cpu.h"
#include "diag_i350_test.h"
#include "diag_hdd_test.h"
#include "diag_m2_test.h"
#include "diag_tpm_test.h"
#include "platform_pwr_seq.h"
#include "platform_stub.h"

/*
 * Global variables
 */
fru_table_t platform_fru_table[];

/* FRU PID and Location Strings */
uchar mb_pid[] = "MB-PID";
uchar dimm_pid[] = "DIMM-PID";
uchar sfp_pid[] = "SFP-PID";
uchar psu_pid[] = "PSU-PID";
uchar rps_pid[] = "RPS-PID";
uchar pvdm_pid[] = "PVDM-PID";
uchar backplane_pid[] = "Backplane-PID";
uchar risercard_pid[] = "RiserCard-PID";
uchar mb_emmc_loc[] = "MB/eMMC";
uchar mb_eusb_loc[] = "MB/eUSB";
uchar mb_msata_loc[] = "MB/mSATA";
uchar sm_pid[] = "SM-PID";
uchar wic_pid[] = "WIC-PID";
uchar dc_pid[] = "DC-PID";
uchar mb_loc[] = "MB";
uchar dimm0_loc[] = "MB/DIMM0";
uchar dimm1_loc[] = "MB/DIMM1";
uchar sfp0_loc[] = "MB/SFP0";
uchar sfp1_loc[] = "MB/SFP1";
uchar sfp2_loc[] = "MB/SFP2";
uchar sfp3_loc[] = "MB/SFP3";
uchar psu0_loc[] = "MB/PSU0";
uchar psu1_loc[] = "MB/PSU1";
uchar rps_loc[] = "MB/RPS";
uchar pvdm0_loc[] = "MB/PVDM0";
uchar backplane_loc[] = "MB/Backplane";
uchar risercard_loc[] = "MB/RiserCard";
uchar sm0_loc[] = "MB/SM0";
uchar sm1_loc[] = "MB/SM1";
uchar wic0_loc[] = "MB/WIC0";
uchar wic1_loc[] = "MB/WIC1";
uchar wic2_loc[] = "MB/WIC2";
uchar sm0wic_loc[] = "SM0/WIC";
uchar sm1wic_loc[] = "SM1/WIC";
uchar sm0pvdm0_loc[] = "SM0/PVDM0";
uchar sm0pvdm1_loc[] = "SM0/PVDM1";
uchar sm0pvdm2_loc[] = "SM0/PVDM2";
uchar sm1pvdm0_loc[] = "SM1/PVDM0";
uchar sm1pvdm1_loc[] = "SM1/PVDM1";
uchar sm1pvdm2_loc[] = "SM1/PVDM2";
uchar sm0wic0dc_loc[] = "SM0/WIC0/DC";
uchar sm1wic0dc_loc[] = "SM1/WIC0/DC";
uchar sm0wic1dc_loc[] = "SM0/WIC1/DC";
uchar sm1wic1dc_loc[] = "SM1/WIC1/DC";
uchar sm0dc_loc[] = "SM0/DC";
uchar sm1dc_loc[] = "SM1/DC";

fru_table_t platform_fru_table[] = {
    { mb_pid,        mb_loc },
    { mb_pid,        mb_emmc_loc},
    { mb_pid,        mb_eusb_loc},
    { mb_pid,        mb_msata_loc},
    { dimm_pid,      dimm0_loc },
    { dimm_pid,      dimm1_loc },
    { sfp_pid,       sfp0_loc },
    { sfp_pid,       sfp1_loc },
    { sfp_pid,       sfp2_loc },
    { sfp_pid,       sfp3_loc },
    { psu_pid,       psu0_loc },
    { psu_pid,       psu1_loc },
    { rps_pid,       rps_loc },
    { pvdm_pid,      pvdm0_loc },
    { backplane_pid, backplane_loc },
    { risercard_pid, risercard_loc },
    { sm_pid,        sm0_loc },
    { sm_pid,        sm1_loc },
    { wic_pid,       wic0_loc },
    { wic_pid,       wic1_loc },
    { wic_pid,       wic2_loc },
    { wic_pid,       sm0wic_loc },
    { wic_pid,       sm1wic_loc },
    { pvdm_pid,      sm0pvdm0_loc },
    { pvdm_pid,      sm0pvdm1_loc },
    { pvdm_pid,      sm0pvdm2_loc },
    { pvdm_pid,      sm1pvdm0_loc },
    { pvdm_pid,      sm1pvdm1_loc },
    { pvdm_pid,      sm1pvdm2_loc },
    { dc_pid,        sm0wic0dc_loc },
    { dc_pid,        sm1wic0dc_loc },
    { dc_pid,        sm0wic1dc_loc },
    { dc_pid,        sm1wic1dc_loc },
    { dc_pid,        sm0dc_loc },
    { dc_pid,        sm1dc_loc },
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
    (PFT) build_i2c_scan_menu, FALSE,
    MF_CONTINUOUS | MF_DOGRP | MF_DOALL | MF_SHOW_ERRCOUNT,
    (type_t(*)())0, 0,
    (type_t(*)()) build_i2c_scan_menu, TRUE},

    {"eMMC test",
    (PFT) build_emmc_test_menu, FALSE,
    MF_CONTINUOUS | MF_DOGRP | MF_DOALL | MF_SHOW_ERRCOUNT,
    (type_t(*)())has_emmc, 0,
    (PFT) build_emmc_test_menu, TRUE},

    {"FPGA test",
    (PFT) build_tabeil_fpga_test_menu, FALSE,
    MF_CONTINUOUS | MF_DOGRP | MF_DOALL | MF_SHOW_ERRCOUNT,
    (type_t(*)())0, 0,
    (PFT) build_tabeil_fpga_test_menu, TRUE},

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
    (type_t(*)())has_phy1514, 0, 
    (PFT) build_gephy0_test_menu, TRUE},

    {"GE PHY 1 Test",
    (PFT) build_gephy1_test_menu, FALSE,
    MF_CONTINUOUS | MF_DOGRP | MF_DOALL | MF_SHOW_ERRCOUNT,
    (type_t(*)())has_phy1514, 0, 
    (PFT) build_gephy1_test_menu, TRUE},
    
    {"GE PHY 88E1543 Test",
    (PFT) fortnite_is_not_support, FALSE,
    MF_CONTINUOUS | MF_DOGRP | MF_DOALL | MF_SHOW_ERRCOUNT,
    (type_t(*)())has_phy1543, 0, 
    (PFT) fortnite_is_not_support, TRUE},

    {"ESW 88E6390 test",
    (PFT) fortnite_is_not_support, FALSE,
    MF_CONTINUOUS | MF_DOGRP | MF_DOALL | MF_SHOW_ERRCOUNT,
    (type_t(*)())has_esw6390, 0,
    (PFT) fortnite_is_not_support, TRUE},
    
    {"I350 test",
    (PFT) diag_i350_test, FALSE,
    MF_CONTINUOUS | MF_DOGRP | MF_DOALL | MF_SHOW_ERRCOUNT,
    (type_t(*)())has_i350, 0,
    (PFT) diag_i350_test, TRUE},
    
    {"M.2 Device test",
    (PFT) build_m2_test_menu, FALSE,
    MF_CONTINUOUS | MF_DOGRP | MF_DOALL | MF_SHOW_ERRCOUNT,
    (type_t(*)())0, 0,
    (PFT) build_m2_test_menu, TRUE},

    {"HDD sata test",
    (PFT) diag_hdd_test, TRUE,
    MF_CONTINUOUS | MF_DOGRP | MF_DOALL | MF_SHOW_ERRCOUNT,
    (type_t(*)())has_hdd, 0,
    (PFT) diag_hdd_test, FALSE},
    
    {"TPM test",
    (PFT) diag_tpm_test, TRUE,
    MF_CONTINUOUS | MF_DOGRP | MF_DOALL | MF_SHOW_ERRCOUNT,
    (type_t(*)())has_tpm, 0,
    (PFT) diag_tpm_test, FALSE},

    {"LED Test",
    (PFT) diag_led_test , TRUE,
    MF_CONTINUOUS | MF_DOGRP  | MF_SHOW_ERRCOUNT,
    (type_t(*)())0, 0,(PFT) diag_led_test, FALSE},

    {"RTC test",
    (PFT) build_rtc_menu, TRUE,
    MF_CONTINUOUS | MF_DOGRP | MF_SHOW_ERRCOUNT,
    (type_t(*)())0, 0,
    (PFT) build_rtc_menu, FALSE},

    {"MCU Test",
     (PFT) build_pwr_seq_menu, FALSE,
     MF_CONTINUOUS | MF_DOGRP | MF_SHOW_ERRCOUNT,
     (type_t(*)())0, 0,
     (PFT) build_pwr_seq_menu, TRUE},

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
 * Revision 1.2  2019/10/17 02:16:25  kehuang2
 * Collapse Tabei-L into main trunk
 *
 * Revision 1.1.4.33  2019/10/01 03:00:49  kehuang2
 * Clean up code
 *
 * Revision 1.1.4.32  2019/09/02 08:42:57  kehuang2
 * Clean up code by the comment of code review
 *
 * Revision 1.1.4.31  2019/08/26 07:55:00  kehuang2
 * Clean up code by the comment of code review
 *
 * Revision 1.1.4.30  2019/08/06 07:20:28  kehuang2
 * Update present function base on the comment of code review
 *
 * Revision 1.1.4.29  2019/07/10 06:11:45  kehuang2
 * Update test item for Promethium
 *
 * Revision 1.1.4.28  2019/07/04 03:23:36  kehuang2
 * Combine Tabei-L sereies image together(Fortnite, Tabei-L, Promethium)
 *
 * Revision 1.1.4.27  2019/06/20 06:21:13  kehuang2
 *
 * 1. Support linux_block_test function
 * 2. Update Diag menu item base on currently project information
 *
 * Revision 1.1.4.26  2019/05/29 03:16:18  kehuang2
 *
 * 1.Merge image according to official board type.
 * 2.Reform the structure of diag menu
 *
 * Revision 1.1.4.25  2019/03/26 06:09:16  olin2
 * Support Dreamliner on Tabei-L
 *
 * Revision 1.1.4.24  2019/03/13 08:57:44  olin2
 * Update FRU PID for WIC
 *
 * Revision 1.1.4.23  2019/01/25 07:42:24  harrchan
 * Add SKU1 in Makefile for seperature sku in future
 *
 * Revision 1.1.4.22  2019/01/25 03:21:06  wilbhuan
 * 1. Added ESW(Ethernet Switch) test with 88E6390 PHY device.
 * 2. The scope of ESW test as following:
 *    (1) Register test
 *    (2) MAC loopback test
 *    (3) External loopback test
 *    (4) Interrupt test
 *
 * Revision 1.1.4.21  2019/01/18 05:55:13  harrchan
 * Clean up code
 *
 * Revision 1.1.4.20  2019/01/18 02:30:16  olin2
 * Clean up code
 *
 * Revision 1.1.4.19  2019/01/16 04:03:45  harrchan
 * Init phy1543 test
 *
 * Revision 1.1.4.18  2019/01/03 03:16:48  harrchan
 * Add distinguish sku function
 *
 * Revision 1.1.4.17  2018/12/26 03:48:33  harrchan
 * LED Test
 *
 * Revision 1.1.4.16  2018/12/25 02:06:29  kodko
 * Add TPM chip test that is verified by vendor provided tool.
 *
 * Revision 1.1.4.15  2018/12/21 07:09:47  olin2
 * Update M.2 device menu
 *
 * Revision 1.1.4.14  2018/12/21 02:27:11  olin2
 * Update menu
 *
 * Revision 1.1.4.13  2018/12/07 01:41:22  olin2
 * Clean up menu
 *
 * Revision 1.1.4.12  2018/12/06 06:12:37  olin2
 * Support M.2 PCIE
 *
 * Revision 1.1.4.11  2018/11/13 09:51:58  olin2
 * Support M.2 USB test
 *
 * Revision 1.1.4.10  2018/11/07 12:20:40  olin2
 * Update test menu
 *
 * Revision 1.1.4.9  2018/10/25 02:37:57  harrchan
 * eMMC Test
 *
 * Revision 1.1.4.8  2018/10/24 10:45:17  harrchan
 * Seperate DIMM test from other I2C device
 *
 * Revision 1.1.4.7  2018/10/24 02:47:27  harrchan
 * 88E1514 GEPHY test
 *
 * Revision 1.1.4.6  2018/10/16 03:50:02  harrchan
 * Add HDD test
 *
 * Revision 1.1.4.5  2018/10/16 01:22:14  harrchan
 * add SATA test
 *
 * Revision 1.1.4.4  2018/10/15 12:17:38  harrchan
 * Add M.2 sata test
 *
 * Revision 1.1.4.3  2018/10/03 06:06:38  olin2
 * Initial commit for I350 test
 *
 * Revision 1.1.4.2  2018/10/02 01:50:02  harrchan
 * Initial commit for Tabei-L P1A bring up.
 *
 * $Endlog$
 *-------------------------------------------------
 */
