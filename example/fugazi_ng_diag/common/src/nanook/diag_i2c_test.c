 /* $Id: diag_i2c_test.c,v 1.2 2019/12/11 10:10:30 lucywang Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/nanook/diag_i2c_test.c,v $
 *-----------------------------------------------------------------------------
 * diag_i2c_test.c - For I2C test
 *
 *
 * Copyright (c) 2019 by Cisco Systems, Inc.
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

/*
 * Functional prototype
 */
int nanook_i2c_scan_test(int);
int build_i2c_scan_menu(boolean);

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

    /*
     * I2C FPGA
     */
    {
     .dev_name = "Power Sequencer(MCU)",
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
     .dev_name = "Temperature Sensor 1",
     .offset = 0,
     .i2c_bus_type = IOFPGA_I2C,
     .i2c_dev = MB_I2C_ADDR_MB_TEMP1,
     .i2c_ctrl = I2C_CTRL_TWO,
     .sub_addr_len = 0,
     .size = sizeof(uint8_t),
     .mux = I2C_MUX_ZERO,
     .buf = NULL,
     }
    ,
    {
     .dev_name = "Temperature Sensor 2",
     .offset = 0,
     .i2c_bus_type = IOFPGA_I2C,
     .i2c_dev = MB_I2C_ADDR_MB_TEMP2,
     .i2c_ctrl = I2C_CTRL_TWO,
     .sub_addr_len = 0,
     .size = sizeof(uint8_t),
     .mux = I2C_MUX_ZERO,
     .buf = NULL,
     }
    ,
    {
     .dev_name = "Pressure Sensor",
     .offset = 0,
     .i2c_bus_type = IOFPGA_I2C,
     .i2c_dev = MB_I2C_ADDR_PRESSURE,
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
     (PFT) nanook_i2c_scan_test, DIMM0_TEST,
     MF_CONTINUOUS | MF_DOGRP | MF_DOALL,
     (type_t(*)())0, 0, 
     (PFT) 0, 0},

    /*{"DIMM1 I2C Scan Test",
     (PFT) nanook_i2c_scan_test, DIMM1_TEST, 0,
     (type_t(*)())0, 0, 
     (PFT) 0, 0},*/
    
    {"Others I2C Device Scan Test",
     (PFT) nanook_i2c_scan_test, DEFAULT_TEST,
     MF_CONTINUOUS | MF_DOGRP | MF_DOALL,
     (type_t(*)())0, 0, 
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
 * Function   : nanook_i2c_scan_test (int option)
 *
 * Description: scan all i2c devices on nanook
 *
 * Inputs     : option ,for future use.
 *
 * Outputs    : PASSED or FAILED
 *
 ****************************************************************************
 */
int nanook_i2c_scan_test (int option)
{
    //uchar mb_get_loc[FRU_SIZE] = {0};
    //uchar mb_get_pid[FRU_SIZE] = {0};
#ifdef ENHANCE_ERROR_MSG_RDY
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
    //platform_get_pid((char *)mb_get_pid);
    //strcpy((char *)mb_get_loc, "MB");
    //platform_fru_table[fru_table_offset].pid_string = mb_get_pid;
    //platform_fru_table[fru_table_offset].location_string = mb_get_loc;

    /* Segment 2: Test step captured from prpass */
    /* Segment 3: Failure message captured from cterr */

    /* Segment 4: Components used */
    //cterr_add_component("Marvell Armada 7040", "I2C", "ACT2/TAM, Boot Strap I2C EEPROM, RTC, SFP, or POE");

    /* Segment 5: register and memory dump */

    /* Segment 6: Platform Environment initialized here*/

    /* Segment 7: Top 3 Debugging Steps */
    //cterr_add_debug("Check the interface between the Host SoC "
                    //"and the failed I2C devices.",
                    //"If there is no problem for these interfaces, "
                    //"replace one I2C device and redo the test.");
#endif
    

    n2g_i2c_if_t i2c_if;
    uint32_t reg_val = 0, ret_val = FAILED;
    uint32_t ix, max_retry;
    uint8_t now_test = 0, test_end = 0, test_num = 1;
    uchar *tname = (uchar *) "I2C scan";
    int err_code = 0;
    max_retry = MAX_RETRY;

    testname("%s", tname);
#ifndef SKU1
    switch (option) {
        case DIMM0_TEST:
    /*
     * DIMM0 is 1st item and independently from other I2C device
     */
            now_test = DIMM0_TEST;
            test_end = DIMM0_TEST + 1;
            break;
        case DIMM1_TEST:
    /*
     * DIMM1 is 2nd item and independently from other I2C device
     */
            now_test = DIMM1_TEST;
            test_end = DIMM1_TEST + 1;
            break;
        case DEFAULT_TEST:
        default:
    /*
     * Do the rest of the scan except DIMM
     * Setup end of test by calculate all I2C device number
     */
            now_test = DEFAULT_TEST;
            test_end = (sizeof(cpu_i2c_dev) / sizeof(n2g_i2c_if_t));
            break;
    }
#else
    now_test = DIMM0_TEST;
    test_end = (sizeof(cpu_i2c_dev) / sizeof(n2g_i2c_if_t));
#endif
    for (; now_test < test_end; now_test++) {
        /*
         * Get I2C device structure
         */
        memcpy(&i2c_if, &cpu_i2c_dev[now_test], sizeof(n2g_i2c_if_t));
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
                printf("I2C scan retry %d\n", ix);
                if (ix == max_retry - 1) {
                    break;
                }
            } else {
                break;
            }
            msleep(DELAY_I2C_SCAN_RETRY);
        }
        if ((NVRAM)->diagflag & D_VERBOSE) {
            printf("Done\n");
        }

        if (ret_val != PASSED) {
            break;
        }

        test_num++;
    }
    /* Enable cterr */
    if (ret_val != PASSED) {
        cterr('f', 0, "%s failed %s", i2c_if.dev_name, i2c_err_str(err_code));
    } else {
        prpass(testpass, "%s, ", tname);
        prcomplete(testpass, errcount, (char *)0);
    }
    return (ret_val);
}

/*-------------------------------------------------
 * $Log: diag_i2c_test.c,v $
 * Revision 1.2  2019/12/11 10:10:30  lucywang
 * Merged Nanook to main trunk
 *
 *
 * $Endlog$
 *-------------------------------------------------
 */
