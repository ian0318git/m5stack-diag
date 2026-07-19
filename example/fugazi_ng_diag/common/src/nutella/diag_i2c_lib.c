/* $Id: diag_i2c_lib.c,v 1.4 2019/07/11 12:31:28 alicehua Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/nutella/diag_i2c_lib.c,v $
 *-----------------------------------------------------------------------------
 * diag_i2c_lib.c - I2C library
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
#include "i2c_api.h"
#include "i2c_dev.h"
#include "n2g_api_rc.h"
#include "plat_defs.h"
#include "diag_fpga.h"
#include "platform_fru.h"
#include "diag_i2c_lib.h"
#include "diag_ddr4_lib.h"
#include "diag_i2c_addr.h"
#include "tam_act2_api_drv_support.h"

/*
 * Functional prototype
 */
int read_i2c_reg_util(int);
int write_i2c_reg_util(int);
static int i2c_fd0 = -1;

static n2g_i2c_if_t cpu_i2c_dev[] = {
    /*
     * I2C 0
     */
    {
     .dev_name = "EEPROM SPD",
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
     .dev_name = "NXP LM75BD",
     .offset = 0,
     .i2c_bus_type = CPU_I2C0,
     .i2c_dev = MB_I2C_ADDR_MB_TEMP_LM75,
     .i2c_ctrl = I2C_CTRL_ZERO,
     .sub_addr_len = 0,
     .size = sizeof(uint8_t),
     .mux = I2C_MUX_ZERO,
     .buf = NULL,
     }
    , 
    /*
     * I2C FPGA
     */
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
    /*
     * I2C Aikido FPGA
     */
    {
     .dev_name = "ACT 2 Aikido Chip",
     .offset = -1,              /* need to be -1 to tell driver not to use offset !!! */
     .i2c_bus_type = IOFPGA_I2C,
     .i2c_dev = MB_I2C_ADDR_AIKIDO_ACT2,
     .i2c_ctrl = I2C_CTRL_ZERO,
     .sub_addr_len = 0,
     .size = sizeof(uint16_t),
     .mux = I2C_MUX_ZERO,
     .buf = NULL,
     }
     ,
};

/*
 * I2C Utility Menu.
 */
static submenu_xtable_t i2c_menu_table[] = {
    {"SPD utility", (PFT) build_plat_dimm_util_menu, 0,
     0, (PFT) 0, 0,
     (PFT) 0, 0},
    {"I2C read (no offset)", (PFT) read_i2c_reg_util, FALSE,
     0, (PFT) 0, 0,
     (PFT) 0, 0},
    {"I2C write (no offset)", (PFT) write_i2c_reg_util, FALSE,
     0, (PFT) 0, 0,
     (PFT) 0, 0},
    {"I2C read", (PFT) read_i2c_reg_util, TRUE,
     0, (PFT) 0, 0,
     (PFT) 0, 0},
    {"I2C write", (PFT) write_i2c_reg_util, TRUE,
     0, (PFT) 0, 0,
     (PFT) 0, 0},

};

#define I2C_MENU_TABLE_SIZE \
        (sizeof(i2c_menu_table) / sizeof(submenu_xtable_t))

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
 * Function: build_i2c_util_menu
 *
 * Description: Build I2C menu.
 *
 * Inputs: None.
 *
 * Outputs: None.
 *
 **********************************************************************
 */
void build_i2c_util_menu (void)
{
    build_primary_submenu(i2c_menu_table, I2C_MENU_TABLE_SIZE,
                          "I2C Utility Menu", &i2cdiagp);
    build_secondary_submenu(i2c_menu_table, I2C_MENU_TABLE_SIZE,
                            i2c_menu_secondary_items);
    menu(&i2cdiag, i2c_menu_secondary_items, 0);
}

/******************************************************************************
 *
 * Function   : init_dimm_i2c_struct
 * Description: To init i2c_dev structure.
 * Inputs     : dev_object_t *i2c_dev;
 *              uint32_t dimm_no.
 * Outputs    : PASSED/FAILED
 *
 ******************************************************************************/
uint32_t init_dimm_i2c_struct (n2g_i2c_dev_t *i2c_dev)
{
    uint32_t rc = FAILED;

    i2c_dev->bus_no = CPU_I2C0;
    i2c_dev->rd_hd_size = HD_SIZE_1;
    i2c_dev->wr_hd_size = HD_SIZE_1;
    i2c_dev->dev_addr = MB_I2C_ADDR_DIMM0;
    i2c_fd0 = get_i2c_fd(0);

    /*
     * Set I2C device to SLAVE mode 
     */
    if (i2c_fd0 <= 0) {
        cterr('f', 0, "%s is not opened correctly.", I2CBUS0);
        return (FAILED);
    } else {
        if ((rc = ioctl(i2c_fd0, I2C_SLAVE, i2c_dev->dev_addr)) < 0) {
            cterr('f', 0, "%s at %s: unable to connect to device %#x. "
                  "rc = %#x", __FUNCTION__, __FILE__,
                  i2c_dev->dev_addr, rc);
            return (FAILED);
        } else {
            i2c_dev->fp = i2c_fd0;
        }
    }
    return (PASSED);
}

/*******************************************************************************
 *
 * Function   : read_i2c_reg_util (int yes_offset)
 * Description: generic i2c read funtcion, allowing user to manually read from
 *              i2c register.
 * Inputs     : yes_offset.
 *
 * Outputs    : PASSED or FAILED
 *
 *******************************************************************************
 */
int read_i2c_reg_util (int yes_offset)
{
    n2g_i2c_if_t i2c_if;
    unsigned int addr, offset, rc, ix, bus;
    uint16_t size;
    uchar d32[80];

    memset(&i2c_if, 0, sizeof(i2c_if));

    bus = getdec_answer("\nEnter i2c bus number(I2C0 - 0, I2C1 - 1, IOFPGA - 3)", 0, 0, 3);

    i2c_if.i2c_bus_type = bus;

    i2c_if.i2c_ctrl = i2c_if.i2c_bus_type;

    i2c_if.mux = 0;

    if ((i2c_if.i2c_bus_type == CPU_I2C0) || (i2c_if.i2c_bus_type == CPU_I2C1)) {
        addr =
            gethex_answer("Enter 7 bit slave address (DIMM - 0x50, Thermal - 0x4A)",
                          0x0, 1, 0xFF);
        i2c_if.i2c_dev = addr;
    } else {
        addr =
            gethex_answer("Enter 7 bit slave address (ACT2 - 0x70)", 0x0, 1, 0xFF);
        i2c_if.i2c_dev = addr;

    }
    if (yes_offset) {
        offset = gethex_answer("Enter reg offset", 0, 0, 0xFF);
        i2c_if.offset = offset;
    } else if (i2c_if.i2c_bus_type == IOFPGA_I2C) {
        i2c_if.offset = -1;
    } else {
        i2c_if.offset = 0;
    }

    size = gethex_answer("Enter length you want to read(in bytes)", 0x0, 1, 10);
    i2c_if.size = size;
    
    memset(d32, 0, sizeof(d32));
    i2c_if.buf = (char *) d32;
    rc = n2g_i2c_read(&i2c_if);
    if (rc != PASSED) {
        cterr('f', 0, "%s at %s: unable to read i2c %#x. "
              "rc = %#x", __FUNCTION__, __FILE__, i2c_if.i2c_dev, rc);
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
 * Function   : write_i2c_reg_util (int yes_offset)
 * Description: generic i2c write funtcion, allowing user to manually write to
 *              i2c register.
 *
 *
 * Inputs     : yes_offset, flag set if user is expected to enter register offset
 *
 * Outputs    : PASSED or FAILED
 *
 *******************************************************************************
 */
int write_i2c_reg_util (int yes_offset)
{
    n2g_i2c_if_t i2c_if;
    unsigned int addr, size, rc, ix, bus;
    int offset;
    char msg[80];
    uint8_t d8[32];

    memset(&i2c_if, 0, sizeof(i2c_if));

    bus = getdec_answer("\nEnter i2c bus number(0-I2C0, 1-I2C1, 3-IOFPGA)", 0, 0, 3);

    i2c_if.i2c_bus_type = bus;

    i2c_if.i2c_ctrl = i2c_if.i2c_bus_type;

    i2c_if.mux = 0;
    
    if ((i2c_if.i2c_bus_type == CPU_I2C0) || (i2c_if.i2c_bus_type == CPU_I2C1)) {
        addr = gethex_answer("Enter 7 bit slave address (DIMM-0x50, Thermal-0x1C)", 
                             0x0, 1, 0xFF);
        i2c_if.i2c_dev = addr;
    } else {
        addr = gethex_answer("Enter 7 bit slave address (ACT2-0x70)", 0x0, 1, 0xFF);
        i2c_if.i2c_dev = addr;
    }
    if (yes_offset) {
        offset = gethex_answer("Enter reg offset", 0x0, 0, 0xFF);
        i2c_if.offset = offset;
    } else if (i2c_if.i2c_bus_type == IOFPGA_I2C) {
        i2c_if.offset = -1;
    } else {
        i2c_if.offset = 0;
    }

    size = gethex_answer("Enter length you want to write", 2, 1, 20);
    i2c_if.size = size;

    for (ix = 0; ix < size; ix++) {
        sprintf(msg, "Enter bytes %d", ix);
        d8[ix] = gethex_answer(msg, 0x0, 0, 0xFF);
    }


    i2c_if.buf = (char *) &d8[0];

    rc = n2g_i2c_write(&i2c_if);
    if (rc != PASSED) {
        cterr('f', 0, "%s at %s: unable to write i2c %#x. "
              "rc = %#x", __FUNCTION__, __FILE__, i2c_if.i2c_dev, rc);
        return (FAILED);
    }

    return (PASSED);
}

/*******************************************************************************
 *
 * Function   : get_n2g_i2c_if
 *
 * Description: return i2c structure
 *
 * Inputs     : i2c, mux, addr 
 *
 * Outputs    : i2c structure pointer or NULL
 *
 *******************************************************************************
 */
void *get_n2g_i2c_if (uint8_t i2c, uint8_t mux, uint8_t addr)
{
    int ix;
    int size = (sizeof(cpu_i2c_dev) / sizeof(n2g_i2c_if_t));
    for (ix = 0; ix < size; ix++) {
        if (cpu_i2c_dev[ix].i2c_dev == addr &&
            cpu_i2c_dev[ix].mux == mux && cpu_i2c_dev[ix].i2c_ctrl == i2c) {
            return ((void *) (&cpu_i2c_dev[ix]));
        }
    }
    printf("problem trying to get n2g_i2c_if; i2c_ctrl=%d, mux=%d, addr=%#x\n",
           i2c, mux, addr);
    fflush(stdout);
    return (void *) NULL;
}


/*******************************************************************************
 *
 * Function   : platform_i2c_get_quack (uint8_t addr, uint8_t ctrl_no)
 * Description: give address and controller number, return i2c structure
 *
 * Inputs     : addr: i2c addres; ctrl_no: i2c controller number
 *
 * Outputs    : pointer to i2c structure, or NULL if i2c struct is not found.
 *
 *******************************************************************************
 */
void *platform_i2c_get_quack (uint8_t addr, uint8_t ctrl_no)
{
    int ix;
    int size = (sizeof(cpu_i2c_dev) / sizeof(n2g_i2c_if_t));

    for (ix = 0; ix < size; ix++) {
        if (cpu_i2c_dev[ix].i2c_dev == addr) {
            /*
             * to support different type of module/motherboard, etc...
             */
            cpu_i2c_dev[ix].i2c_ctrl = ctrl_no;
            return ((void *) (&cpu_i2c_dev[ix]));
        }
    }

    /*
     * check mdoules now
     */
    printf("Software has no support for device at addr %#x; ctrl_no = %#x\n]",
           addr, ctrl_no);

    return (void *) NULL;
}

/* end of file */
/******** History ********
*---------------------------------------------------
$Log: diag_i2c_lib.c,v $
Revision 1.4  2019/07/11 12:31:28  alicehua
Collapse Nutella codes into main trunk.

$Endlog$
*/
