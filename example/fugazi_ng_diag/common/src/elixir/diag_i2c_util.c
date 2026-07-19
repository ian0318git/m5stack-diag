/* $Id: diag_i2c_util.c,v 1.2 2021/09/24 01:21:06 harrchan Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/elixir/diag_i2c_util.c,v $
 *------------------------------------------------------------------
 *
 * diag_i2c_util.c
 *
 * Copyright (c) 2018 - 2019 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#include <stdio.h>
#include <string.h>
#include "proto.h"
#include "queryflags.h"
#include "common.h"
#include "menu.h"
#include "error.h"
#include "nvmonvars.h"
#include "i2c_dev.h"
#include "i2c_api.h"
#include "diag_i2c_lib.h"
#include "platform_i2c.h"
#include "diag_moka_fpga_lib.h"
#include "diag_enhance_err_msg_lib.h"
#include "platform_cookie.h"
#include "diag_dimm_util.h"
#include "diag_mcu_util.h"
#include "diag_temp_sensor_test.h"
#include "diag_temp_sensor_util.h"
#include "diag_poe_psu_lib.h"

static int i2c_reg_rd_util(int);
static int i2c_reg_wr_util(int);

/*
 * I2C Utility Menu.
 */
static submenu_xtable_t i2c_menu_table[] = {
    {"DIMM0", (PFT)dimm_util_entry, 0,
     0, (PFT) 0, 0,
     (PFT) 0, 0},
    {"I2C read (no offset)", (PFT) i2c_reg_rd_util, FALSE,
     0, (PFT) 0, 0,
     (PFT) 0, 0},
    {"I2C write (no offset)", (PFT) i2c_reg_wr_util, FALSE,
     0, (PFT) 0, 0,
     (PFT) 0, 0},
    {"I2C read", (PFT) i2c_reg_rd_util, TRUE,
     0, (PFT) 0, 0,
     (PFT) 0, 0},
    {"I2C write", (PFT) i2c_reg_wr_util, TRUE,
     0, (PFT) 0, 0,
     (PFT) 0, 0},
};

#define I2C_MENU_TABLE_SIZE \
        (sizeof(i2c_menu_table) / sizeof(submenu_xtable_t))
#define ENHANCE_ERROR_MSG_RDY 1

/*
 * Primary & secondary submenu items (filled in from xtable)
 */
static mitem_t i2c_menu_primary_items[I2C_MENU_TABLE_SIZE +
                                      MAX_BASE_ITEMS];
static mitem_t i2c_menu_secondary_items[I2C_MENU_TABLE_SIZE +
                                        MAX_BASE_ITEMS];

static struct menuinfo i2cdiag = {
    "I2C Utility Menu",         /* title */
    0,                          /* title string added by init_empty_menu */
    (PFT) menu_show_dflags,     /* shows major flags */
    0,                          /* generic prompt */
    0,                          /* size -- bumped by add_menu_item() */
    i2c_menu_primary_items,
};

static struct menuinfo *i2cdiagp = &i2cdiag;




/**********************************************************************
 *
 * Function: diag_i2c_util
 *
 * Description: Build I2C menu.
 *
 * Inputs: None.
 *
 * Outputs: None.
 *
 **********************************************************************
 */
void diag_i2c_util (void)
{
    build_primary_submenu(i2c_menu_table, I2C_MENU_TABLE_SIZE,
                          "I2C Utility Menu", &i2cdiagp);
    build_secondary_submenu(i2c_menu_table, I2C_MENU_TABLE_SIZE,
                            i2c_menu_secondary_items);
    menu(&i2cdiag, i2c_menu_secondary_items, 0);
}

/*******************************************************************************
 *
 * Function   : i2c_reg_rd_util (int has_offset)
 * Description: generic i2c read funtcion, allowing user to manually read from
 *              i2c register.
 * Inputs     : has_offset. not used
 *
 * Outputs    : PASSED or FAILED
 *
 *******************************************************************************
 */
static int i2c_reg_rd_util (int has_offset)
{
    n2g_i2c_if_t i2c_if;
    unsigned int addr, size, offset, rc, ix, bus;
    uchar d32[80];

    memset(&i2c_if, 0, sizeof(i2c_if));

    bus = getdec_answer("\nEnter i2c bus number", 2, 0, 3);

    i2c_if.i2c_bus_type = bus;

    i2c_if.i2c_ctrl = i2c_if.i2c_bus_type;

    i2c_if.mux = 0;

    addr = gethex_answer("Enter 7 bit slave address ", 0x1c, 0x0, 0xFF);
    i2c_if.i2c_dev = addr;

    if (has_offset) {
        offset = gethex_answer("Enter reg offset", 0, 0, 0xFF);
        i2c_if.offset = offset;
    } else {
        i2c_if.offset = 0;
    }

    size =
        gethex_answer("Enter length you want to read(in bytes)", 2, 1, 10);
    i2c_if.size = size;

    memset(d32, 0, sizeof(d32));
    i2c_if.buf = (char *) d32;

    rc = n2g_i2c_read(&i2c_if);
    if (rc != PASSED) {
        printf("%s:%d:Unable to read i2c %#x. rc = %#x\n", 
               __FUNCTION__, __LINE__, i2c_if.i2c_dev, rc);
        return (FAILED);
    }

    printf("\n");
    for (ix = 0; ix < size; ix++) {
        printf("0x%02x ", d32[ix]);
    }

    return (PASSED);
}

/*******************************************************************************
 *
 * Function   : i2c_reg_wr_util (int has_offset)
 * Description: generic i2c write funtcion, allowing user to manually write to
 *              i2c register.
 *                example:
 *                    byte3 -  byte0 = 00 12 34 56
 *                    *buf is 0x56;  (little endian)
 *                    *(buf+1) is 0x34
 *                    *(buf+2) is 0x12
 *                    *(buf+3) is 0x0
 *    if we want to send data out, we need to shift to the left by 8 bits.
 *    we copy this to the data fifo. data fifo.
 *    format of fifo is byte0 to byte3.
 *    we will copy *(buf+3) to byte0 of fifo, *(buf+2) to byte1 of fifo. etc...
 *
 *
 * Inputs     : has_offset, flag set if user is expected to enter register offset
 *
 * Outputs    : PASSED or FAILED
 *
 *******************************************************************************
 */
static int i2c_reg_wr_util (int has_offset)
{
    n2g_i2c_if_t i2c_if;
    unsigned int addr, size, rc, ix, bus;
    int offset;
    char msg[80];
    uint8_t d8[32];

    memset(&i2c_if, 0, sizeof(i2c_if));

    bus = getdec_answer("\nEnter i2c bus number", 2, 0, 3);

    i2c_if.i2c_bus_type = bus;

    i2c_if.i2c_ctrl = i2c_if.i2c_bus_type;

    i2c_if.mux = 0;

    addr =
        gethex_answer("Enter 7 bit slave address (on mux 0)", 0x1c, 1,
                      0xFF);
    i2c_if.i2c_dev = addr;

    if (has_offset) {
        offset = gethex_answer("Enter reg offset", 0x0, 0, 0xFF);
        i2c_if.offset = offset;
    } else {
        i2c_if.offset = 0;
    }

    size = gethex_answer("Enter length you want to write", 2, 1, 20);
    i2c_if.size = size;

    for (ix = 0; ix < size; ix++) {
        sprintf(msg, "Enter bytes %d", ix);
        d8[ix] = gethex_answer(msg, 0x0, 0, 0xFF);
    }

    if (i2c_if.i2c_dev == MB_I2C_ADDR_SYS_EEPROM0
        || i2c_if.i2c_dev == MB_I2C_ADDR_SYS_EEPROM1) {
        i2c_if.size = size * 2;
    }

    i2c_if.buf = (char *) &d8[0];

    rc = n2g_i2c_write(&i2c_if);
    if (rc != PASSED) {
        printf("%s:%d:Unable to write i2c %#x. rc = %#x\n", 
               __FUNCTION__, __LINE__, i2c_if.i2c_dev, rc);
        return (FAILED);
    }

    return (PASSED);
}

/*-------------------------------------------------
 * $Log: diag_i2c_util.c,v $
 * Revision 1.2  2021/09/24 01:21:06  harrchan
 * Collapse Elixir-branch to Main Trunk.
 *
 * Revision 1.1.2.2  2021/07/30 09:45:05  illiu
 * Move MCU Utility from I2C Utility submenu to basic utilities submenu
 *
 * Revision 1.1.2.1  2020/09/09 09:08:06  illiu
 * First version which has been ported with Dreamliner and Marvel CPSS
 *
 * Revision 1.2  2019/01/10 06:36:23  wilbhuan
 * The beginning of Betelgeuse application code.
 *
 *-------------------------------------------------
 */
