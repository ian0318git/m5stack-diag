/* $Id: diag_i2c_util.c,v 1.2 2016/04/20 11:25:25 benchen2 Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/tachi_bmc/diag_i2c_util.c,v $
 *------------------------------------------------------------------
 *
 * diag_i2c_util.c - I2C Utility Functions
 * 
 * June 2015, Times Huang
 *
 * Copyright (c) 2016 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */
 
#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/ioctl.h> 
#include "common.h"
#include "types.h"
#include "error.h"
#include "queryflags.h"
#include "menu.h"
#include "defs.h"
#include "common_utils.h"
#include "i2c_api.h"
#include "diag_i2c_util.h"
#include "diag_i2c_api.h"
#include "diag_temp_sensor_util.h"
#include "diag_mcu_util.h"
#include "diag_barometer_util.h"
#include "diag_pem_lib.h"

int diag_i2c_util(void);

static int diag_i2c_read(int);
static int diag_i2c_write(int);

/* Sub Menu used for FPGA utility.
 */
static submenu_xtable_t i2c_util_submenu_table[] = {
    {"Temperature Sensor Utility", (type_t(*)())diag_temp_sensor_util, 0,
        0, (type_t(*)())0, 0, (type_t(*)())0,   0},
    {"Power Sequencer Utility",  (type_t(*)())diag_mcu_util,           0,
        0, (type_t(*)())0, 0, (type_t(*)())0,   0},
    {"Barometer Utility",          (type_t(*)())diag_barometer_util,   0,
        0, (type_t(*)())0, 0, (type_t(*)())0,   0},
    {"PSU Utility",                (type_t(*)())build_pem_menu,  PSU_ONE,
        0, (type_t(*)())0, 0, (type_t(*)())0,   0},
    {"I2C Read (no offset)",       (type_t(*)())diag_i2c_read,       FALSE,
        0, (type_t(*)())0, 0, (type_t(*)())0,   0},
    {"I2C Write (no offset)",      (type_t(*)())diag_i2c_write,      FALSE,
        0, (type_t(*)())0, 0, (type_t(*)())0,   0},
    {"I2C Read",                   (type_t(*)())diag_i2c_read,        TRUE,
        0, (type_t(*)())0, 0, (type_t(*)())0,   0},
    {"I2C Write",                  (type_t(*)())diag_i2c_write,       TRUE,
        0, (type_t(*)())0, 0, (type_t(*)())0,   0},
};

#define I2C_UTIL_SUBMENU_TABLE_SIZE (sizeof(i2c_util_submenu_table) / \
				       sizeof(submenu_xtable_t))

/*
 * Primary & secondary submenu items (filled in from xtable)
 */
static mitem_t i2c_util_primary_items[I2C_UTIL_SUBMENU_TABLE_SIZE +
				       MAX_BASE_ITEMS];
static mitem_t i2c_util_secondary_items[I2C_UTIL_SUBMENU_TABLE_SIZE +
					 MAX_BASE_ITEMS];

menuinfo_t i2c_util_subtest_menu = {
    "%s Subtest Menu",
    0,                                /* mtparam added by init_empty_menu */
    (PFT)show_endnote,                /* notes missing WICs in combos */
    0,                                /* use generic prompt */
    0,                                /* size (bumped by add_menu_item() */
    i2c_util_primary_items,
};
menuinfo_t *i2c_util_submenup = &i2c_util_subtest_menu;

int diag_i2c_util (void)
{
    build_primary_submenu(i2c_util_submenu_table,
			              I2C_UTIL_SUBMENU_TABLE_SIZE,
                          "I2C Utilities", &i2c_util_submenup);
    build_secondary_submenu(i2c_util_submenu_table,
                            I2C_UTIL_SUBMENU_TABLE_SIZE,
                            i2c_util_secondary_items);    
                            
    menu(i2c_util_submenup, i2c_util_secondary_items, '\0');
    return (PASSED);
}

static int diag_i2c_read (int yes_offset) 
{
    n2g_i2c_if_t i2c_if;
    unsigned int addr, size, offset, rc, i, sub_addr_len;
    uchar d32[80];

    memset(&i2c_if, 0, sizeof(i2c_if));

    printf("Enter bus type\n");
    printf("CPU I2C0-7 : 0-7\n");
    printf("IOFPGA_I2C : 8\n");
    i2c_if.i2c_bus_type = getdec_answer("\nType", 8, 0, 8);

    i2c_if.i2c_ctrl = getdec_answer("\nEnter ctrl number", 12, 0, 16);
    i2c_if.mux = getdec_answer("Enter mux number", 0, 0, 4);
    addr = gethex_answer("Enter 7 bit slave address ", 0x75, 0x0, 0xFF);
    i2c_if.i2c_dev = addr;

    if (yes_offset) {
        offset = gethex_answer("Enter reg offset", 0, 0, 0xFF);
        i2c_if.offset = offset;
        sub_addr_len = getdec_answer("Enter sub addr len", 0, 0, 3);
        i2c_if.sub_addr_len = sub_addr_len;
    }
    else {
        i2c_if.offset = -1;
    }

    size = gethex_answer("Enter length (in bytes)", 2, 1, 10);
    i2c_if.size = size;

    memset(d32, 0, sizeof(d32));
    i2c_if.buf = (char *)d32;

    rc = n2g_i2c_read(&i2c_if);
    if (rc != RC_I2C_OP_OK) {
        printf("unable to read i2c\n");
        return FAILED;
    }

    printf("\n");
    for (i = 0; i < size ; i++) {
        printf("0x%02x ", d32[i]);
    }

    return (PASSED);
}

static int diag_i2c_write (int yes_offset)
{
    n2g_i2c_if_t i2c_if;
    unsigned int addr, size, rc, i, sub_addr_len;
    int offset;
    char msg[80];
    uint8_t d8[32];

    memset(&i2c_if, 0, sizeof(i2c_if));

    printf("Enter bus type\n");
    printf("CPU I2C0-7 : 0-7\n");
    printf("IOFPGA_I2C : 8\n");
    i2c_if.i2c_bus_type = getdec_answer("\nType", 8, 0, 8);

    i2c_if.i2c_ctrl = getdec_answer("\nEnter ctrl number", 12, 0, 16);

    i2c_if.mux = getdec_answer("Enter mux number", 0, 0, 3);

    addr = gethex_answer("Enter 7 bit slave address (on mux 0)", 0x75, 1, 0xFF);
    i2c_if.i2c_dev = addr;

    if (yes_offset) {
        offset = gethex_answer("Enter reg offset", 0x0, 0, 0xFF);
        i2c_if.offset = offset;
        sub_addr_len = getdec_answer("Enter sub addr len", 0, 0, 3);
        i2c_if.sub_addr_len = sub_addr_len;
    } else {
        i2c_if.offset = -1;
    }

    size = gethex_answer("Enter length ", 2, 1, 20);
    i2c_if.size = size;

    for (i = 0; i < size; i++) {
        sprintf(msg, "Enter bytes %d", i);
        d8[i] = gethex_answer(msg, 0x0, 0, 0xFF);
    }

    i2c_if.buf = (char *)&d8[0];

    rc = n2g_i2c_write(&i2c_if);
    if (rc != RC_I2C_OP_OK) {
        printf("unable to write i2c.\n");
        return FAILED;
    }

    return (PASSED);
}


/*---------------------------------------------------------------
$Log: diag_i2c_util.c,v $
Revision 1.2  2016/04/20 11:25:25  benchen2
add tachi fru portion

Revision 1.1.2.7  2015/12/23 11:16:13  alpeng
support PEM(PSU) utility and its fan utils

Revision 1.1.2.6  2015/11/13 07:42:04  benchen2
remove raid utility

Revision 1.1.2.5  2015/10/14 10:30:27  benchen2
add sub addr length option

Revision 1.1.2.4  2015/09/25 02:18:24  tirawan
Correct MCU reg read/write (not to byte swap) and display MCU version

Revision 1.1.2.3  2015/08/27 01:24:26  alpeng
update i2c utils; add ngio init on linux_main.c

Revision 1.1.2.2  2015/07/31 07:47:40  hondwang
add barometer item

Revision 1.1.2.1  2015/06/11 02:01:08  tirawan
Add files for Tachi BMC project


$Endlog$
*/

