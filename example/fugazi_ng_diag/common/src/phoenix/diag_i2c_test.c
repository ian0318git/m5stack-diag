/* $Id: diag_i2c_test.c,v 1.2 2021/04/15 00:52:26 achiu2 Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/phoenix/diag_i2c_test.c,v $
 *------------------------------------------------------------------
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
int phoenix_i2c_scan_test(int);
int build_i2c_scan_menu(boolean);

/*
 * For Phoenix I2C device
 */
static n2g_i2c_if_t cpu_i2c_dev[] = {

    /*
     * I2C 0
     */
    {
     .dev_name = "EEPROM SPD",
     .offset = 0,
     .i2c_bus_type = CPU_I2C0,
     .i2c_dev = MB_I2C_ADDR_SPD,
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
    {
     .dev_name = "SMB PECI Write",
     .offset = -1,
     .i2c_bus_type = IOFPGA_I2C,
     .i2c_dev = MB_I2C_ADDR_PECI_WR,
     .i2c_ctrl = I2C_CTRL_TWO,
     .sub_addr_len = 0,
     .size = sizeof(uint16_t),
     .mux = I2C_MUX_ONE,
     .buf = NULL,
     }
     ,
    {
     .dev_name = "SMB PECI Read",
     .offset = -1,
     .i2c_bus_type = IOFPGA_I2C,
     .i2c_dev = MB_I2C_ADDR_PECI_RD,
     .i2c_ctrl = I2C_CTRL_TWO,
     .sub_addr_len = 0,
     .size = sizeof(uint16_t),
     .mux = I2C_MUX_ONE,
     .buf = NULL,
     }
     ,
    {
     .dev_name = "PSU 0 Microcontroller",
     .offset = 0,
     .i2c_bus_type = IOFPGA_I2C,
     .i2c_dev = MB_I2C_ADDR_PSU_MCCTLR,
     .i2c_ctrl = I2C_CTRL_FOUR,
     .sub_addr_len = 0,
     .size = sizeof(uint8_t),
     .mux = I2C_MUX_ZERO,
     .buf = NULL,
     }
    ,
    {
     .dev_name = "PSU 0 EEPROM",
     .offset = 0,
     .i2c_bus_type = IOFPGA_I2C,
     .i2c_dev = MB_I2C_ADDR_PSU_EEPROM,
     .i2c_ctrl = I2C_CTRL_FOUR,
     .sub_addr_len = 0,
     .size = sizeof(uint8_t),
     .mux = I2C_MUX_ZERO,
     .buf = NULL,
     }
    ,
    {
     .dev_name = "PSU 1 Microcontroller",
     .offset = 0,
     .i2c_bus_type = IOFPGA_I2C,
     .i2c_dev = MB_I2C_ADDR_PSU_MCCTLR,
     .i2c_ctrl = I2C_CTRL_FOUR,
     .sub_addr_len = 0,
     .size = sizeof(uint8_t),
     .mux = I2C_MUX_ONE,
     .buf = NULL,
     }
    ,
    {
     .dev_name = "PSU 1 EEPROM",
     .offset = 0,
     .i2c_bus_type = IOFPGA_I2C,
     .i2c_dev = MB_I2C_ADDR_PSU_EEPROM,
     .i2c_ctrl = I2C_CTRL_FOUR,
     .sub_addr_len = 0,
     .size = sizeof(uint8_t),
     .mux = I2C_MUX_ONE,
     .buf = NULL,
     }
    ,
    {
     .dev_name = "USB-UART controller",
     .offset = -1,  //offset<0 :pure i2c read only, not SMBus(write offset first, then read
     .i2c_bus_type = IOFPGA_I2C,
     .i2c_dev = MB_I2C_ADDR_USB_UART_CTRL_1,
     .i2c_ctrl = I2C_CTRL_FIVE,
     .sub_addr_len = 0,
     .size = sizeof(uint8_t),
     .mux = I2C_MUX_ZERO,
     .buf = NULL,
     }
    ,
    {
     .dev_name = "USB-UART controller(firmware download)",
     .offset = -1,
     .i2c_bus_type = IOFPGA_I2C,
     .i2c_dev = MB_I2C_ADDR_USB_UART_CTRL_2,
     .i2c_ctrl = I2C_CTRL_FIVE,
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

    {"I2C Scan Test",
     (PFT) diag_i2c_scan_test, 0,
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
 * Function   : diag_i2c_scan_test (int option)
 *
 * Description: scan all i2c devices on phoenix
 *
 * Inputs     : option ,for future use.
 *
 * Outputs    : PASSED or FAILED
 *
 ****************************************************************************
 */
int diag_i2c_scan_test (int option)
{
    n2g_i2c_if_t i2c_if;
    uint32_t reg_val = 0, ret_val = FAILED, ret = PASSED;
    uint32_t ix;
    uint8_t now_test = 0, test_end = 0, test_num = 1;
    uchar *tname = (uchar *) "I2C scan";
    uint hw_brd_type=0, hw_brd_rev=0;

    testname("%s", tname);
    prpass(testpass, "%s, ", tname);
    printf("\n\n");

    test_end = sizeof(cpu_i2c_dev) / sizeof(n2g_i2c_if_t);
    phoenix_get_hw_brd_info(&hw_brd_type, &hw_brd_rev);
    if (hw_brd_rev < 2) { //P1A, P1B borad does not connect USB-UART controller I2C
        test_end = test_end-2;
    }

    for (now_test = 0; now_test < test_end; now_test++) {
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
            printf(" [%2d] I2C_%d: %s\n",
                   test_num, i2c_if.i2c_bus_type, i2c_if.dev_name);
        }

        /*
         * Read I2C device Register 0
         */
        for (ix = 0; ix < MAX_RETRY_I2C_SCAN; ix++) {
            ret_val = n2g_i2c_read(&i2c_if);
            if (ret_val != PASSED) {
                printf("I2C scan retry %d\n", ix);
            } else {
                break;
            }
            msleep(DELAY_I2C_SCAN_RETRY);
        }

        if ((NVRAM)->diagflag & D_VERBOSE) {
            printf("Done\n");
        }

        if (ret_val != PASSED) {
            ret = FAILED;
            cterr('f', 0, "%s failed during", i2c_if.dev_name);
            printf("\n");
        }

        test_num++;
    }

    if (ret == PASSED) {
        prcomplete(testpass, errcount, (char *)0);
    }

    return (ret);
}
