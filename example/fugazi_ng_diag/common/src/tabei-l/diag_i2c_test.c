 /* $Id: diag_i2c_test.c,v 1.3 2019/11/25 08:55:52 kehuang2 Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/tabei-l/diag_i2c_test.c,v $
 *-----------------------------------------------------------------------------
 * diag_i2c_test.c - For I2C test
 *
 *
 * Copyright (c) 2016 ~ 2019 by Cisco Systems, Inc.
 * All rights reserved.
 *-----------------------------------------------------------------------------
 */
#include <stdio.h>
#include <string.h>
#include "proto.h"
#include "queryflags.h"
#include "common.h"
#include "menu.h"
#include "error.h"
#include "nvmonvars.h"
#include "mb_tests.h"
#include "platform_i2c.h"
#include "i2c_api.h"
#include "i2c_dev.h"
#include "diag_i2c_test.h"
#include "diag_i2c_addr.h"
#include "diag_i2c_lib.h"
#include "diag_fpga.h"
#include "diag_common.h"

/*
 * Functional prototype
 */
int tabei_i2c_scan_test(int);
int build_i2c_scan_menu(boolean);

/*
 * For Fortnite I2C device
 */
static n2g_i2c_if_t fortnite_i2c_dev[] = {
    /*
     * I2C 0
     */
    {
     .dev_name = "EEPROM SPD 0",
     .offset = 0,
     .i2c_bus_type = CPU_I2C0,
     .i2c_dev = MB_I2C_ADDR_DIMM0,
     .i2c_ctrl = I2C_CTRL_ZERO,
     .sub_addr_len = 0,
     .size = sizeof(uint8_t),  /* dont' use sub address slave register */
     .mux = I2C_MUX_ZERO,
     .buf = NULL,
     }
    ,
    {
     .dev_name = "EEPROM",
     .offset = 0,
     .i2c_bus_type = CPU_I2C0,
     .i2c_dev = MB_I2C_ADDR_EEPROM,
     .i2c_ctrl = I2C_CTRL_ZERO,
     .sub_addr_len = 0,
     .size = sizeof(uint16_t),
     .mux = I2C_MUX_ZERO,
     .buf = NULL,
     }
    ,
    /*
     * I2C FPGA
     */
    {
     .dev_name = "Barometer",
     .offset = 0,
     .i2c_bus_type = IOFPGA_I2C,
     .i2c_dev = MB_I2C_ADDR_BAROMETER,
     .i2c_ctrl = I2C_CTRL_TWO,
     .sub_addr_len = 0,
     .size = sizeof(uint8_t),
     .mux = I2C_MUX_ZERO,
     .buf = NULL,
     }
    ,
    {
     .dev_name = "ACT 2 Lite Secure Chip",
     .offset = -1,              /* need to be -1 to tell driver not to use offset !!! */
     .i2c_bus_type = IOFPGA_I2C,
     .i2c_dev = MB_I2C_ADDR_ACT2,
     .i2c_ctrl = I2C_CTRL_ZERO,
     .sub_addr_len = 0,
     .size = sizeof(uint16_t),
     .mux = I2C_MUX_ZERO,
     .buf = NULL,
     }
     ,
    {
     .dev_name = "Power Sequencer",
     .offset = 0,
     .i2c_bus_type = IOFPGA_I2C,
     .i2c_dev = MB_I2C_ADDR_PWR_SEQ,
     .i2c_ctrl = I2C_CTRL_TWO,
     .sub_addr_len = 0,
     .size = sizeof(uint16_t),
     .mux = I2C_MUX_ZERO,
     .buf = NULL,
     }
     ,
    {
     .dev_name = "Temperature Sensor  OUTLET1",
     .offset = 0,
     .i2c_bus_type = IOFPGA_I2C,
     .i2c_dev = MB_I2C_ADDR_MB_TEMP1_IN_1,
     .i2c_ctrl = I2C_CTRL_TWO,
     .sub_addr_len = 0,
     .size = sizeof(uint8_t),
     .mux = I2C_MUX_ZERO,
     .buf = NULL,
     }
     ,
    {
     .dev_name = "Temperature Sensor  INLET1",
     .offset = 0,
     .i2c_bus_type = IOFPGA_I2C,
     .i2c_dev = MB_I2C_ADDR_MB_TEMP3_OUT_1,
     .i2c_ctrl = I2C_CTRL_TWO,
     .sub_addr_len = 0,
     .size = sizeof(uint8_t),
     .mux = I2C_MUX_ZERO,
     .buf = NULL,
     }
     ,
    {
     .dev_name = "SFP Mux",
     .offset = 0,
     .i2c_bus_type = IOFPGA_I2C,
     .i2c_dev = MB_I2C_MUX_SFP,
     .i2c_ctrl = I2C_CTRL_FIVE,
     .sub_addr_len = 0,
     .size = sizeof(uint8_t),
     .mux = I2C_MUX_ZERO,
     .buf = NULL,
     }
     ,
    {
     .dev_name = "Top SFP",
     .offset = 0,
     .i2c_bus_type = IOFPGA_I2C,
     .i2c_dev = MB_I2C_SFP_DEV,
     .i2c_ctrl = I2C_CTRL_FIVE,
     .sub_addr_len = 0,
     .size = sizeof(uint8_t),
     .mux = I2C_MUX_ZERO,
     .buf = NULL,
     }
     ,
    {
     .dev_name = "Bottom SFP",
     .offset = 0,
     .i2c_bus_type = IOFPGA_I2C,
     .i2c_dev = MB_I2C_SFP_DEV,
     .i2c_ctrl = I2C_CTRL_FIVE,
     .sub_addr_len = 0,
     .size = sizeof(uint8_t),
     .mux = I2C_MUX_ZERO,
     .buf = NULL,
     }
     ,

};

/*
 * For Tabei-L / Promethium I2C device
 */
static n2g_i2c_if_t cpu_i2c_dev[] = {

    /*
     * I2C 0
     */
    {
     .dev_name = "EEPROM SPD 0",
     .offset = 0,
     .i2c_bus_type = CPU_I2C0,
     .i2c_dev = MB_I2C_ADDR_DIMM0,
     .i2c_ctrl = I2C_CTRL_ZERO,
     .sub_addr_len = 0,
     .size = sizeof(uint8_t),  /* dont' use sub address slave register */
     .mux = I2C_MUX_ZERO,
     .buf = NULL,
     }
    ,
    {
     .dev_name = "EEPROM SPD 1",
     .offset = 0,
     .i2c_bus_type = CPU_I2C0,
     .i2c_dev = MB_I2C_ADDR_DIMM1,
     .i2c_ctrl = I2C_CTRL_ZERO,
     .sub_addr_len = 0,
     .size = sizeof(uint8_t),  /* dont' use sub address slave register */
     .mux = I2C_MUX_ZERO,
     .buf = NULL,
     }
    ,
    {
     .dev_name = "EEPROM",
     .offset = 0,
     .i2c_bus_type = CPU_I2C0,
     .i2c_dev = MB_I2C_ADDR_EEPROM,
     .i2c_ctrl = I2C_CTRL_ZERO,
     .sub_addr_len = 0,
     .size = sizeof(uint16_t),
     .mux = I2C_MUX_ZERO,
     .buf = NULL,
     }
    ,


    /*
     * I2C FPGA
     */
    {
     .dev_name = "Power Sequencer",
     .offset = 0,
     .i2c_bus_type = IOFPGA_I2C,
     .i2c_dev = MB_I2C_ADDR_PWR_SEQ,
     .i2c_ctrl = I2C_CTRL_TWO,
     .sub_addr_len = 0,
     .size = sizeof(uint16_t),
     .mux = I2C_MUX_ZERO,
     .buf = NULL,
     }
     ,
    {
     .dev_name = "Temperature Sensor  INLET1",
     .offset = 0,
     .i2c_bus_type = IOFPGA_I2C,
     .i2c_dev = MB_I2C_ADDR_MB_TEMP1_IN_1,
     .i2c_ctrl = I2C_CTRL_TWO,
     .sub_addr_len = 0,
     .size = sizeof(uint8_t),
     .mux = I2C_MUX_ZERO,
     .buf = NULL,
     }
     ,
    {
     .dev_name = "Temperature Sensor  OUTLET1",
     .offset = 0,
     .i2c_bus_type = IOFPGA_I2C,
     .i2c_dev = MB_I2C_ADDR_MB_TEMP3_OUT_1,
     .i2c_ctrl = I2C_CTRL_TWO,
     .sub_addr_len = 0,
     .size = sizeof(uint8_t),
     .mux = I2C_MUX_ZERO,
     .buf = NULL,
     }
     ,
    {
     .dev_name = "Temperature Sensor  INLET2",
     .offset = 0,
     .i2c_bus_type = IOFPGA_I2C,
     .i2c_dev = MB_I2C_ADDR_MB_TEMP2_IN_2,
     .i2c_ctrl = I2C_CTRL_TWO,
     .sub_addr_len = 0,
     .size = sizeof(uint8_t),
     .mux = I2C_MUX_ZERO,
     .buf = NULL,
     }
     ,
    {
     .dev_name = "Temperature Sensor  OUTLET2",
     .offset = 0,
     .i2c_bus_type = IOFPGA_I2C,
     .i2c_dev = MB_I2C_ADDR_MB_TEMP4_OUT_2,
     .i2c_ctrl = I2C_CTRL_TWO,
     .sub_addr_len = 0,
     .size = sizeof(uint8_t),
     .mux = I2C_MUX_ZERO,
     .buf = NULL,
     }
    ,

};

/*
 * Sub Menu used for "I2C scan test -> I2C scan submenu test"
 */
submenu_xtable_t i2c_scan_submenu_table[] = {

    {"DIMM0 I2C Scan Test",
     (PFT) tabei_i2c_scan_test, DIMM0_TEST, 0,
     (type_t(*)())has_dimm_slot, 0, 
     (PFT) 0, 0},

    {"DIMM1 I2C Scan Test",
     (PFT) tabei_i2c_scan_test, DIMM1_TEST, 0,
     (type_t(*)())has_dimm_slot, 0, 
     (PFT) 0, 0},

    {"Barometer Scan Test",
     (PFT) tabei_i2c_scan_test, BARO_TEST,
     MF_CONTINUOUS | MF_DOGRP | MF_DOALL,
     (type_t(*)())has_barometer, 0, 
     (PFT) 0, 0},

    {"Others I2C Device Scan Test",
     (PFT) tabei_i2c_scan_test, DEFAULT_TEST,
     MF_CONTINUOUS | MF_DOGRP | MF_DOALL,
     (type_t(*)())has_dimm_slot, 0, 
     (PFT) 0, 0},

    {"I2C Device Scan Test",
     (PFT) tabei_i2c_scan_test, FORTNITE_TEST,
     MF_CONTINUOUS | MF_DOGRP | MF_DOALL,
     (type_t(*)())is_fortnite, 0, 
     (PFT) 0, 0},

    {"SFP Mux Scan Test",
     (PFT) tabei_i2c_scan_test, SFP_TEST, 0,
     (type_t(*)())is_promethium, 0, 
     (PFT) 0, 0},

};

#define I2C_SCAN_SUBMENU_TABLE_SIZE (sizeof(i2c_scan_submenu_table) / \
                                       sizeof(submenu_xtable_t))

/*
 * "motherboard test -> i2c scan test" primary & secondary submenu items (filled in from xtable)
 */
static mitem_t i2c_scan_primary_items[I2C_SCAN_SUBMENU_TABLE_SIZE +
                                      MAX_BASE_ITEMS];
static mitem_t i2c_scan_secondary_items[I2C_SCAN_SUBMENU_TABLE_SIZE +
                                        MAX_BASE_ITEMS];

menuinfo_t scan_subtest_menu = {
    "%s Subtest Menu",
    0,                          /* mtparam added by init_empty_menu */
    (PFT) show_endnote,         /* notes missing WICs in combos */
    0,                          /* use generic prompt */
    0,                          /* size (bumped by add_menu_item() */
    i2c_scan_primary_items,
};

menuinfo_t *scan_submenup = &scan_subtest_menu;


/*******************************************************************************
 *
 * Function   : build_i2c_scan_menu
 * Description: build the menu of i2c scan 
 * Inputs     : Test/Menu 
 * Outputs    : PASSED
 *
 *******************************************************************************
 */
int build_i2c_scan_menu (boolean mb_temp_test_items_executed)
{
    char *tname = "I2C Scan Test";
    testname(tname);

    build_primary_submenu(i2c_scan_submenu_table, I2C_SCAN_SUBMENU_TABLE_SIZE,
                          "I2C Scan test", &scan_submenup);
    build_secondary_submenu(i2c_scan_submenu_table, I2C_SCAN_SUBMENU_TABLE_SIZE,
                            i2c_scan_secondary_items);
    if (mb_temp_test_items_executed) {
        menu(&scan_subtest_menu, i2c_scan_secondary_items, 0);
    } else {
        do_all_menu_items(scan_submenup);
    }
    return (PASSED);
}


/*****************************************************************************
 *
 * Function   : tabei_i2c_scan_test (int option)
 *
 * Description: scan all i2c devices on tabei
 *
 * Inputs     : option ,for future use.
 *
 * Outputs    : PASSED or FAILED
 *
 ****************************************************************************
 */
int tabei_i2c_scan_test (int option)
{

    n2g_i2c_if_t i2c_if;
    uint32_t reg_val = 0, ret_val = FAILED;
    uint32_t ix, max_retry;
    uint8_t now_test = 0, test_end = 0, test_num = 1;
    uchar *tname = (uchar *) "I2C scan";
    int err_code = 0;
    max_retry = MAX_RETRY;

    testname("%s", tname);
    prpass(testpass, "%s, ", tname);
    switch (option) {
        /*
         * Just Test DIMM0, cause DIMM slot is pluggable on Tabei-L
         */
        case DIMM0_TEST:
            now_test = DIMM0_TEST;
            test_end = DIMM0_TEST + 1;
            break;
        /*
         * Just Test DIMM1, cause DIMM slot is pluggable on Tabei-L
         */
        case DIMM1_TEST:
            now_test = DIMM1_TEST;
            test_end = DIMM1_TEST + 1;
            break;
        /*
         * Just Test Barometer, cause not all sku have it
         */
        case BARO_TEST:
            now_test = BARO_TEST;
            test_end = BARO_TEST + 1;
            break;

        /*
         * Test others onboard I2C devices on Tabei-L
         */
        case DEFAULT_TEST:
            now_test = DEFAULT_TEST;
            test_end = sizeof(cpu_i2c_dev) / sizeof(n2g_i2c_if_t);
            break;
        /*
         * Test the I2C bus on SFP, which are the last 2 I2C devices
         */
        case SFP_TEST:
            now_test = (sizeof(fortnite_i2c_dev) / sizeof(n2g_i2c_if_t))
                        - SFP_DEVICES;
            test_end = sizeof(fortnite_i2c_dev) / sizeof(n2g_i2c_if_t);
            switch_sfp_mux(SFP0);
            break;
        /*
         * Test every on board I2C devices, expects the 2 SFP I2C devices
         */
        case FORTNITE_TEST:
        default:
            now_test = DIMM0_TEST;
            test_end = (sizeof(fortnite_i2c_dev) / sizeof(n2g_i2c_if_t))
                       - SFP_DEVICES;
            break;
    }
    for (; now_test < test_end; now_test++) {
        /*
         * Get I2C device structure
         */
        switch (option) {
            case DIMM0_TEST:
            case DIMM1_TEST:
            case DEFAULT_TEST:
                memcpy(&i2c_if, &cpu_i2c_dev[now_test], sizeof(n2g_i2c_if_t));
                break;
            case SFP_TEST:
                memcpy(&i2c_if, &fortnite_i2c_dev[now_test], sizeof(n2g_i2c_if_t));
                /* Switch Mux to Bottom SFP to Test the Bottom SFP device  */
                if (now_test == (test_end - 1)) {
                    switch_sfp_mux(SFP1);
                }
                /* Wait for Mux to switch, Reference to Texas Instrument PCA9543A */
                msleep(SLEEP_1);   
                break;
            case BARO_TEST:
            case FORTNITE_TEST:
            default:
                memcpy(&i2c_if, &fortnite_i2c_dev[now_test], sizeof(n2g_i2c_if_t));
                break;
        }
        i2c_if.buf = (char *) &reg_val;
        if ((NVRAM)->diagflag & D_VERBOSE) {
            printf("Now testing %2d: I2C bus %2d, Mux %d, %-29s(0x%.2X)... ",
                   now_test, i2c_if.i2c_bus_type, i2c_if.mux, i2c_if.dev_name,
                   (i2c_if.i2c_dev << 1));
        } else {
            printf("[%2d] I2C_%d: %s\n ",
                   test_num, i2c_if.i2c_bus_type, i2c_if.dev_name);
        }

        /*
         * Read I2C device Register 0
         */
        for (ix = 0; ix < max_retry; ix++) {
            ret_val = n2g_i2c_read(&i2c_if);
            if (ret_val != PASSED) {
                /*
                 * Mux will access another SFP if retry
                 */
                if (option == SFP_TEST) {
                     cterr('f', 0, "%s failed %s", i2c_if.dev_name, 
                            i2c_err_str(err_code));
                     break;
                 }
                printf("I2C scan retry %d\n", ix);
            } else {
                break;
            }
            msleep(DELAY_I2C_SCAN_RETRY);
        }
        if ((NVRAM)->diagflag & D_VERBOSE) {
            printf("Done\n");
        }

        test_num++;
    }
    /* Enable cterr */
    if (ret_val != PASSED) {
        cterr('f', 0, "%s failed %s", i2c_if.dev_name, i2c_err_str(err_code));
    }
    prcomplete(testpass, errcount, (char *)0);
    return (ret_val);
}

/*-------------------------------------------------
 * $Log: diag_i2c_test.c,v $
 * Revision 1.3  2019/11/25 08:55:52  kehuang2
 * Collapse Tabei-L into main trunk
 *
 * Revision 1.2  2019/10/17 02:16:22  kehuang2
 * Collapse Tabei-L into main trunk
 *
 * Revision 1.1.4.22  2019/09/20 07:00:34  kehuang2
 * Clean up code base on review comment
 *
 * Revision 1.1.4.21  2019/08/26 07:55:00  kehuang2
 * Clean up code by the comment of code review
 *
 * Revision 1.1.4.20  2019/08/06 07:20:28  kehuang2
 * Update present function base on the comment of code review
 *
 * Revision 1.1.4.19  2019/07/30 06:56:29  kehuang2
 * Clean up code
 *
 * Revision 1.1.4.18  2019/07/16 11:15:51  kehuang2
 * Remove ACT 2 component in default test
 *
 * Revision 1.1.4.17  2019/07/15 11:28:47  kehuang2
 * Support Barometer test and utility
 *
 * Revision 1.1.4.16  2019/07/10 06:11:45  kehuang2
 * Update test item for Promethium
 *
 * Revision 1.1.4.15  2019/07/09 06:11:31  kehuang2
 * Update I2C bus change and enhence I2C scan coverage
 *
 * Revision 1.1.4.14  2019/05/29 12:01:20  kehuang2
 * Rename function by the platform name
 *
 * Revision 1.1.4.13  2019/05/29 03:16:17  kehuang2
 *
 * 1.Merge image according to official board type.
 * 2.Reform the structure of diag menu
 *
 * Revision 1.1.4.12  2019/05/16 08:48:13  kehuang2
 * Clean up code by the comment of code review.
 *
 * Revision 1.1.4.11  2019/04/29 08:14:26  kehuang2
 * Clean up code
 *
 * Revision 1.1.4.10  2019/03/19 09:26:26  kehuang2
 * Merge Sku1 and Sku2 into same image
 *
 * Revision 1.1.4.9  2019/01/25 08:48:26  harrchan
 * Merge sku1 and sku2 function
 *
 * Revision 1.1.4.8  2019/01/25 07:42:24  harrchan
 * Add SKU1 in Makefile for seperature sku in future
 *
 * Revision 1.1.4.7  2018/12/27 07:30:38  harrchan
 * Update I2C scan test
 *
 * Revision 1.1.4.6  2018/10/25 09:55:24  harrchan
 * Add MCU utility in I2C utility
 *
 * Revision 1.1.4.5  2018/10/24 10:45:17  harrchan
 * Seperate DIMM test from other I2C device
 *
 * Revision 1.1.4.4  2018/10/19 01:44:19  harrchan
 * I2C scan test
 *
 * Revision 1.1.4.3  2018/10/18 03:17:30  olin2
 * Clean up redefined MACRO
 *
 * Revision 1.1.4.2  2018/10/02 01:49:59  harrchan
 * Initial commit for Tabei-L P1A bring up.
 *
 * $Endlog$
 *-------------------------------------------------
 */
