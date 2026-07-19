/* $Id: diag_mother_board_test.c,v 1.2 2021/09/24 01:21:06 harrchan Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/elixir/diag_mother_board_test.c,v $
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
#include "usb_dongle_common_test.h"
#include "usb_dongle_common_host_impl.h"

/* FRU PID and Location Strings */
uchar mb_pid[] = "MB-PID";
uchar mb_loc[] = "MB";

static int diag_console_rts_cts_test(int opt);
static int diag_console_dtr_dsr_test(int opt);

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
    (PFT) usb_dongle_test_entry, USB_SLOT0,
    MF_CONTINUOUS | MF_DOGRP | MF_DOALL | MF_SHOW_ERRCOUNT,
    (type_t(*)())0, 0,
    (PFT) usb_dongle_test_entry, USB_SLOT0 + USB_MAX_SLOT_NO},

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
     (type_t(*)())0, 0,
     (type_t(*)())diag_88e1112_ge1_test, TRUE},

    {"Ethernet Switch test",
     (type_t(*)())diag_ac5_test, FALSE,
     MF_CONTINUOUS | MF_DOGRP | MF_DOALL | MF_SHOW_ERRCOUNT,
     (type_t(*)())0, 0,
     (type_t(*)())diag_ac5_test, TRUE},

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

    {"Console RTS/CTS test",
    (PFT)diag_console_rts_cts_test, TRUE,
    MF_CONTINUOUS | MF_DOGRP | MF_DOALL | MF_SHOW_ERRCOUNT,
    (type_t(*)())0, 0,
    (PFT)diag_console_rts_cts_test, FALSE},

    {"Console DTR/DSR test",
    (PFT)diag_console_dtr_dsr_test, TRUE,
    MF_CONTINUOUS | MF_DOGRP | MF_DOALL | MF_SHOW_ERRCOUNT,
    (type_t(*)())0, 0,
    (PFT)diag_console_dtr_dsr_test, FALSE},

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

/******************************************************
 *
 * Function   : diag_console_rts_cts_test
 * Description: Function to test RTS pin and CTS pin loopback
 * Inputs     : opt for reserve
 * Outputs    : PASSED/FAILED
 *
 ******************************************************
 */
int diag_console_rts_cts_test (int opt)
{
    uint cpu_mpp = 0, gpio_dout_en = 0;
    uint gpio_din = 0, gpio_dout = 0;
    int polling;
    
    /* Confirmed that CP_MPP11 is configured as GPIO_OUT */
    if (plat_mem_read32((uint)CP_MPP_CTRL_REG(1), &cpu_mpp) != PASSED) {
        cterr('f', 0, "%s:%d Failed to check CPU side MPP info.",
                      __FUNCTION__, __LINE__);
        return (FAILED);
    }

    if ((cpu_mpp & 0xf000) != 0) {
        cpu_mpp &= (uint)(~0xf000);
        if (plat_mem_write32((uint)CP_MPP_CTRL_REG(1), cpu_mpp) != PASSED) {
            cterr('f', 0, "%s: Failed to set CPU MPP.", __FUNCTION__);
            return (FAILED);
        }
    }

    if (plat_mem_read32((uint)CP_GPIO_DATA_OUT_EN_REG(0),
                       &gpio_dout_en) != PASSED) {
        cterr('f', 0, "%s:%d Failed to check CPU side GPIO Data OUT enable info.",
                      __FUNCTION__, __LINE__);
        return (FAILED);
    }

    if ((gpio_dout_en & (1 << 11)) == (1<<11)) {
        gpio_dout_en &= ~(1 << 11);
        if (plat_mem_write32((uint)CP_GPIO_DATA_OUT_EN_REG(0),
                            gpio_dout_en) != PASSED) {
            cterr('f', 0, "%s: Failed to set CPU GPIO Data OUT enable.",
                          __FUNCTION__);
            return (FAILED);
        }
    }

    /* Confirmed that CP_MPP10 is configured as GPIO_IN */
    if (plat_mem_read32((uint)CP_MPP_CTRL_REG(1), &cpu_mpp) != PASSED) {
        cterr('f', 0, "%s:%d Failed to check CPU side MPP info.",
                      __FUNCTION__, __LINE__);
        return (FAILED);
    }

    if ((cpu_mpp & 0xf00) != 0) {
        cpu_mpp &= (~0xf00);
        if (plat_mem_write32((uint)CP_MPP_CTRL_REG(1), cpu_mpp) != PASSED) {
            cterr('f', 0, "%s: Failed to set CPU MPP.", __FUNCTION__);
            return (FAILED);
        }
    }

    if (plat_mem_read32((uint)CP_GPIO_DATA_OUT_EN_REG(0),
                       &gpio_dout_en) != PASSED) {
        cterr('f', 0, "%s:%d Failed to check CPU side GPIO Data OUT enable info.",
                      __FUNCTION__, __LINE__);
        return (FAILED);
    }

    if ((gpio_dout_en & (uint)(1 << 10)) != (1 << 10)) {
        gpio_dout_en |= (uint)(1 << 10);
        if (plat_mem_write32((uint)CP_GPIO_DATA_OUT_EN_REG(0),
                            gpio_dout_en) != PASSED) {
            cterr('f', 0, "%s: Failed to set CPU GPIO Data OUT enable.",
                          __FUNCTION__);
            return (FAILED);
        }
    }

    /* CPP11 pull high */
    if (plat_mem_read32((uint)CP_GPIO_DATA_OUT_REG(0),
                       &gpio_dout) != PASSED) {
        cterr('f', 0, "%s:%d Failed to check CPU side GPIO Data OUT info.",
                      __FUNCTION__, __LINE__);
        return (FAILED);
    }

    gpio_dout |= (1 << 11);

    printf("Write value of CPP11 = %x\n",gpio_dout);
    if (plat_mem_write32((uint)CP_GPIO_DATA_OUT_REG(0), gpio_dout) != PASSED) {
        cterr('f', 0, "%s:%d Failed to write CPU side GPIO Data OUT info.",
                      __FUNCTION__, __LINE__);
        return (FAILED);
    }


    /* Check CPP10 should be high */
    polling = 10;
    while (polling >= 0) {
        polling--;
        msleep(100);

        if (plat_mem_read32((uint)CP_GPIO_DATA_IN_REG(0), &gpio_din) != PASSED) {
            cterr('f', 0, "%s:%d Failed to check CPU side GPIO Data IN info.",
                      __FUNCTION__, __LINE__);
            return (FAILED);
        }

        printf("Read value of CPP10 = %x\n",gpio_din);
        if ((gpio_din & (uint)(1 << 10)) == (1 << 10)) {
            break;
        }
        if (polling == -1) {
            cterr('f', 0, "%s: Failed !! CPU detected CPP10 not high value.",
                      __FUNCTION__);
            return (FAILED);
        }
    }

    /* CPP11 pull low */
    if (plat_mem_read32((uint)CP_GPIO_DATA_OUT_REG(0),
                       &gpio_dout) != PASSED) {
        cterr('f', 0, "%s:%d Failed to check CPU side GPIO Data OUT info.",
                      __FUNCTION__, __LINE__);
        return (FAILED);
    }

    gpio_dout &= ~(1 << 11);

    printf("Write value of CPP11 = %x\n",gpio_dout);
    if (plat_mem_write32((uint)CP_GPIO_DATA_OUT_REG(0), gpio_dout) != PASSED) {
        cterr('f', 0, "%s:%d Failed to write CPU side GPIO Data OUT info.",
                      __FUNCTION__, __LINE__);
        return (FAILED);
    }


    /* Check CPP10 should be low*/
    polling = 10;
    while (polling >= 0) {
        polling--;
        msleep(100);

        if (plat_mem_read32((uint)CP_GPIO_DATA_IN_REG(0),
                       &gpio_din) != PASSED) {
            cterr('f', 0, "%s:%d Failed to check CPU side GPIO Data IN info.",
                  __FUNCTION__, __LINE__);
            return (FAILED);
        }

        printf("Read value of CPP10 = %x\n",gpio_din);
        if ((gpio_din & (uint)(1 << 10)) != (1 << 10)) {
            break;
        }
        if (polling == -1) {
            cterr('f', 0, "%s: Failed !! CPU detected CPP10 not low value.",
                  __FUNCTION__);
            return (FAILED);
        }

    }

    return (PASSED);
}

/******************************************************
 *
 * Function   : diag_console_dtr_dsr_test
 * Description: Function to test DTR pin and DSR pin loopback
 * Inputs     : opt for reserve
 * Outputs    : PASSED/FAILED
 *
 ******************************************************
 */
int diag_console_dtr_dsr_test (int opt)
{
    uint fpga_read_val = 0, fpga_write_val = 0;
    int polling;
    
    /* FPGA set register 0x110C bit9 DTR SIGNAL to high */
    if (fpga_read_32_reg(FPGA_STAT_AND_CTRL_REG, &fpga_read_val) != PASSED) {
        cterr('f', 0, "%s:%d Failed to read FPGA reg:%x",
                      __FUNCTION__, __LINE__, FPGA_STAT_AND_CTRL_REG);
        return (FAILED);
    }

    fpga_write_val = fpga_read_val | (1 << 9);

    printf("Write value of fpga reg 0x110c = %x\n",fpga_write_val);
    if (fpga_write_32_reg(FPGA_STAT_AND_CTRL_REG, fpga_write_val) != PASSED) {
        cterr('f', 0, "%s:%d Failed to write FPGA reg:%x",
                      __FUNCTION__, __LINE__, FPGA_STAT_AND_CTRL_REG);
        return (FAILED);
    }


    polling = 10;
    while (polling >= 0) {
        polling--;
        msleep(100);
        /* Check FPGA register 0x110C bit8 DSR SIGNAL should be high level */
        if (fpga_read_32_reg(FPGA_STAT_AND_CTRL_REG, &fpga_read_val) != PASSED) {
            cterr('f', 0, "%s:%d Failed to read FPGA reg:%x",
                          __FUNCTION__, __LINE__, FPGA_STAT_AND_CTRL_REG);
            return (FAILED);
        }

        printf("Read value of fpga reg 0x110c = %x\n", fpga_read_val);
        if ((fpga_read_val & (uint)(1 << 8)) == (1 << 8)) {
            break;
        }
        if (polling == -1) {
            cterr('f', 0, "%s: Failed !! FPGA detected reg 0x110c bit 8 not high value.",
                          __FUNCTION__);
            return (FAILED);
        }
    }

    /* FPGA set register 0x110C bit9 DTR SIGNAL to low */
    if (fpga_read_32_reg(FPGA_STAT_AND_CTRL_REG, &fpga_read_val) != PASSED) {
        cterr('f', 0, "%s:%d Failed to read FPGA reg:%x",
                      __FUNCTION__, __LINE__, FPGA_STAT_AND_CTRL_REG);
        return (FAILED);
    }

    fpga_write_val = fpga_read_val & ~(1 << 9);

    printf("Write value of fpga reg 0x110c = %x\n",fpga_write_val);
    if (fpga_write_32_reg(FPGA_STAT_AND_CTRL_REG, fpga_write_val) != PASSED) {
        cterr('f', 0, "%s:%d Failed to write FPGA reg:%x",
                      __FUNCTION__, __LINE__, FPGA_STAT_AND_CTRL_REG);
        return (FAILED);
    }
    
    polling = 10;
    while (polling >= 0) {
        polling--;
        msleep(100);
        /* Check FPGA register 0x110C bit8 DSR SIGNAL should be low level */
        if (fpga_read_32_reg(FPGA_STAT_AND_CTRL_REG, &fpga_read_val) != PASSED) {
            cterr('f', 0, "%s:%d Failed to read FPGA reg:%x",
                          __FUNCTION__, __LINE__, FPGA_STAT_AND_CTRL_REG);
            return (FAILED);
        }

        printf("Read value of fpga reg 0x110c = %x\n", fpga_read_val);
        if ((fpga_read_val & (uint)(1 << 8)) != (1 << 8)) {
            break;
        }
        if (polling == -1) { 
            cterr('f', 0, "%s: Failed !! FPGA detected reg 0x110c bit 8 not low value.",
                          __FUNCTION__);
            return (FAILED);
        }
    }


    return (PASSED);
}

/*-------------------------------------------------
 * $Log: diag_mother_board_test.c,v $
 * Revision 1.2  2021/09/24 01:21:06  harrchan
 * Collapse Elixir-branch to Main Trunk.
 *
 * Revision 1.1.2.8  2021/09/07 01:22:05  harrchan
 * Add polling mechanism in DTR/DSR and RTS/CTS test
 *
 * Revision 1.1.2.7  2020/11/27 03:27:53  illiu
 * Add usb dongle feature into test item(External USB test)
 *
 * Revision 1.1.2.6  2020/11/12 08:56:54  harrchan
 * Add CTS/RTS DTR/DSR pin test
 *
 * Revision 1.1.2.5  2020/10/26 07:08:28  harrchan
 * 1.Changed PID table in platform_i2c.c
 * 2.Modify menu item to match up Elixir hardware design.
 *
 * Revision 1.1.2.4  2020/10/07 11:22:15  illiu
 * Clean up code
 *
 * Revision 1.1.2.3  2020/10/06 02:06:47  illiu
 * Transform calling objects from AC3 file/function to AC5 file/finction (dev_98dxc323.c -> dev_98dxc25x.c)
 *
 * Revision 1.1.2.2  2020/09/15 09:30:36  illiu
 * Clean up code
 *
 * Revision 1.1.2.1  2020/09/09 09:08:06  illiu
 * First version which has been ported with Dreamliner and Marvel CPSS
 *
 * Revision 1.2  2019/01/10 06:36:23  wilbhuan
 * The beginning of Betelgeuse application code.
 *
 *-------------------------------------------------
 */
