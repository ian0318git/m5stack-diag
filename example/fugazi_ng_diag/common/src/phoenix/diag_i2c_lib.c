/* $Id: diag_i2c_lib.c,v 1.2 2021/04/15 00:52:26 achiu2 Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/phoenix/diag_i2c_lib.c,v $
 *------------------------------------------------------------------
 * diag_i2c_lib.c - I2C library
 *
 *
 * Copyright (c) 2016 - 2019 by Cisco Systems, Inc.
 * All rights reserved.
 *-----------------------------------------------------------------------------
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
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
#include "platform_i2c.h"
#include "diag_i2c_lib.h"
#include "diag_ddr4_lib.h"
#include "diag_i2c_addr.h"
#include "platform_pwr_seq.h"
#include "tam_act2_api_drv_support.h"
#include "ngio.h"
#include "platform_barometer.h"

/*
 * Functional prototype
 */
int read_i2c_reg_util(int);
int write_i2c_reg_util(int);
uint32_t n2g_i2c_init(n2g_i2c_if_t *);
uint32_t n2g_i2c_open(n2g_i2c_if_t *);
uint32_t n2g_i2c_close(n2g_i2c_if_t *);
void *platform_get_wic_act2(int);
int diag_peci_get_temp(int);

static int i2c_fd0 = -1;
static n2g_i2c_if_t wic_act2[MAX_WIC+FIRST_SLOT];
static uint8_t wic_i2c_ctrl[] = {0,  WIC1_I2C_CTRL, WIC2_I2C_CTRL, WIC3_I2C_CTRL};

void *platform_get_sm_act2(int);
static n2g_i2c_if_t sm_act2[MAX_SM+FIRST_SLOT];
static uint8_t sm_i2c_ctrl[] = {0,  SM1_I2C_CTRL, SM2_I2C_CTRL, SM3_I2C_CTRL, SM4_I2C_CTRL};

/* used for ngio */
static n2g_i2c_if_t ngio_act2[] = {

    {
        .dev_name = "ACT2",
        .offset = -1,  /* need to be -1 to tell driver not to use offset !!! */
        .i2c_bus_type = IOFPGA_I2C,
        .sub_addr_len = 0,
        .size    = sizeof(uint16_t),
        .mux = I2C_MUX_ZERO,
        .buf        = NULL,
    },
};

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
     .dev_name = "Aikido Secure Chip",
     .offset = -1,              /* need to be -1 to tell driver not to use offset !!! */
     .i2c_bus_type = IOFPGA_I2C,
     .i2c_dev = MB_I2C_ADDR_AIKIDO,
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
     .dev_name = "Temperature Sensor 1 INLET1",
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
     .dev_name = "Temperature Sensor 2 INLET2",
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
     .dev_name = "Temperature Sensor 3 OUTLET1",
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
     .dev_name = "Temperature Sensor 4 OUTLET2",
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
     .offset = -1,
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
    {"Show barometer info", (PFT) show_barometer_info, TRUE,
     0, (type_t(*)())0, 0,
     (PFT) 0, 0},
    {"Display barometer registers", (PFT) display_barometer_reg, TRUE,
     0, (type_t(*)())0, 0,
     (PFT) 0, 0},
    {"Get CPU Temperature via PECI proxy command", (PFT) diag_peci_get_temp, TRUE,
     0, (type_t(*)())0, 0,
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
    i2c_fd0 = get_i2c_fd(0);
    i2c_dev->dev_addr = MB_I2C_ADDR_SPD;
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
    unsigned int addr, offset, rc, ix, bus, ctrl, mux;
    uint16_t size;
    uchar d32[80];

    memset(&i2c_if, 0, sizeof(i2c_if));

    bus = getdec_answer("\nEnter i2c bus number(I2C0 - 0, I2C1 - 1, IOFPGA - 3)", 0, 0, 3);

    i2c_if.i2c_bus_type = bus;

    ctrl = getdec_answer("\nEnter i2c controller number", 0, 0, 20);

    i2c_if.i2c_ctrl = ctrl;

    mux = getdec_answer("\nEnter i2c controller mux", 0, 0, 20);

    i2c_if.mux = mux;

    if ((i2c_if.i2c_bus_type == CPU_I2C0) || (i2c_if.i2c_bus_type == CPU_I2C1)) {
        addr =
            gethex_answer("Enter 7 bit slave address (SPD - 0x50, Thermal - 0x4A)",
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
    unsigned int addr, size, rc, ix, bus, ctrl, mux;
    int offset;
    char msg[80];
    uint8_t d8[32];

    memset(&i2c_if, 0, sizeof(i2c_if));

    bus = getdec_answer("\nEnter i2c bus number(0-I2C0, 1-I2C1, 3-IOFPGA)", 0, 0, 3);

    i2c_if.i2c_bus_type = bus;

    ctrl = getdec_answer("\nEnter i2c controller number", 0, 0, 20);

    i2c_if.i2c_ctrl = ctrl;

    mux = getdec_answer("\nEnter i2c controller mux", 0, 0, 20);

    i2c_if.mux = mux;
    
    if ((i2c_if.i2c_bus_type == CPU_I2C0) || (i2c_if.i2c_bus_type == CPU_I2C1)) {
        addr = gethex_answer("Enter 7 bit slave address (SPD-0x50, Thermal-0x1C)", 
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

    uint hw_brd_type=0, hw_brd_rev=0;
    phoenix_get_hw_brd_info(&hw_brd_type, &hw_brd_rev);
    if (hw_brd_rev < 2) { //P1A, P1B borad does not connect USB-UART controller I2C
        size = size-2;
    }

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
    n2g_i2c_if_t *i2c;
    int size = (sizeof(cpu_i2c_dev) / sizeof(n2g_i2c_if_t));

    uint hw_brd_type=0, hw_brd_rev=0;
    phoenix_get_hw_brd_info(&hw_brd_type, &hw_brd_rev);
    if (hw_brd_rev < 2) { //P1A, P1B borad does not connect USB-UART controller I2C
        size = size-2;
    }

    /* Motherboard */
    for (ix = 0; ix < size; ix++) {
        if (cpu_i2c_dev[ix].i2c_dev == addr) {
            cpu_i2c_dev[ix].i2c_ctrl = ctrl_no;
            return ((void *) (&cpu_i2c_dev[ix]));
        }
    }

    /* WIC modules */
    if (addr == NGIOWIC_I2C_ADDR_ACT2) {
        for (ix = FIRST_SLOT; ix <= SECOND_SLOT; ix++) {
            i2c = (n2g_i2c_if_t *)platform_get_wic_act2(ix);
            if (i2c->i2c_ctrl == ctrl_no){
                return((void*)i2c);
            }
        }
    }
    
    /* SM modules */
    if (addr == NGIOSM_I2C_ADDR_ACT2) {
        i2c = (n2g_i2c_if_t *)platform_get_sm_act2(FIRST_SLOT);
        if (i2c->i2c_ctrl == ctrl_no){
            return((void*)i2c);
        }
    }

    /*
     * check mdoules now
     */
    printf("Software has no support for device at addr %#x; ctrl_no = %#x\n",
           addr, ctrl_no);

    return (void *) NULL;
}

/*********************************************************************
 *
 * Function:    n2g_i2c_open
 *
 * Description: legacy code. not used.
 *             
 */
uint32_t n2g_i2c_open (n2g_i2c_if_t *i2c_p)
{   
    return (PASSED);
}


/*********************************************************************
 *
 * Function:    n2g_i2c_close
 *
 * Description: legacy code. not used.
 *             
 */
uint32_t n2g_i2c_close (n2g_i2c_if_t *i2c_p)
{
    return (PASSED);
}

/*********************************************************************
 *
 * Function:    n2g_i2c_init
 *
 * Description: N2G I2C API for init. This API only initialize the controller,
 *              not the I2C devices, except Goofy port 5 1:4 Mux.
 *
 * Inputs:      i2c_p   - Pointer to the N2G I2C API interface struct. Fields
 *                        needed in the struct are:
 *                              i2c_bus_type, i2c_speed.
 *
 * Outputs:     PASSED - No errors encounterd.
 *              Other return codes are provided by the lower device driver.
 *
 * Assumptions:
 *
 *********************************************************************
 */
uint32_t n2g_i2c_init (n2g_i2c_if_t *i2c_p)
{
    uint32_t rc = PASSED;

    /* Call the lower device driver */
    switch(i2c_p->i2c_bus_type) {
    case IOFPGA_I2C:
        return (PASSED);
    }

    return (rc);
}

/*******************************************************************************
 *
 * Function   : platform_get_wic_act2
 * Description: returns act2 WIC struture
 *
 * Inputs     : slot number
 *
 * Outputs    : pointer to act2 struct for wic
 *
 *******************************************************************************
 */
void *platform_get_wic_act2 (int slot)
{
    if (slot == 0) {
        assert(!"slot is 0");
    }
    memcpy(&wic_act2[slot], ngio_act2, sizeof(n2g_i2c_if_t));
    wic_act2[slot].i2c_ctrl = wic_i2c_ctrl[slot];
    wic_act2[slot].i2c_dev = NGIOWIC_I2C_ADDR_ACT2;
    return (void *)&wic_act2[slot];
}

/*******************************************************************************
 *
 * Function   : platform_get_sm_act2
 * Description: returns act2 SM struture
 *
 * Inputs     : slot number
 *
 * Outputs    : pointer to act2 struct for sm
 *
 *******************************************************************************
 */
void *platform_get_sm_act2 (int slot)
{
    if (slot == 0) {
        assert(!"slot is 0");
    }
    memcpy(&sm_act2[slot], ngio_act2, sizeof(n2g_i2c_if_t));
    sm_act2[slot].i2c_ctrl = sm_i2c_ctrl[slot];
    sm_act2[slot].i2c_dev = NGIOSM_I2C_ADDR_ACT2;
    return (void *)&sm_act2[slot];
}

/*******************************************************************************
 *
 * Function   : get_cpu_ref_temp
 * Description: Use rdmsr tool to get CPU REF temperature.
 * Inputs     : None.
 * Outputs    : int - CPU REF. temperature.
 *
 *******************************************************************************
 */
static int get_cpu_ref_temp(void)
{
    int fd = 0, ret, size = 0;
    char check[9];
    int temp;

    ret = system(RDMSR_CPU_REF_TEMP_CMD);
    if (ret != 0) {
        printf("%s(): execute command failed, %d\n", __func__, ret);
        goto ref_temp_err_2;
    }

    fd = open(REF_TEMP_TXT_FILE_PATH, O_RDONLY);
    if (fd <= 0) {
        printf("%s(): open file %s failed\n", __func__, REF_TEMP_TXT_FILE_PATH);
        goto ref_temp_err_2;
    }

    lseek(fd, 0, SEEK_SET);
    ret = read(fd, &check, 2);
    if (ret <= 0) {
        printf("%s(): read check failed\n", __func__);
        goto ref_temp_err_1;
    }

    check[2] = '\0';
    if (!strncmp(check, "-s", 2)) {
        printf("%s(): Send rdmsr failed\n", __func__);
        goto ref_temp_err_1;
    }

    while (1) {
        lseek(fd, size, SEEK_SET);
        ret = read(fd, &check[size], 1);

        if (size > 8) {
            printf("%s(): Get rdmsr result failed\n", __func__);
            goto ref_temp_err_1;
        }

        if (ret) {
            size++;
        } else {
            check[size+1] = '\0';
            close(fd);
            break;
        }
    }

    sscanf(check, "%x", &temp);
    temp = (temp & REF_TEMP_MASK) >> REF_TEMP_SHIFT_BIT;

    return (temp);

ref_temp_err_1:
    close(fd);

ref_temp_err_2:
    return (PHOENIX_PROCESSOR_TJMAX);
}


/*******************************************************************************
 *
 * Function   : diag_peci_get_temp (int opt)
 * Description: Send peci proxy command, GetTemp, to get CPU tempature.
 * Inputs     : opt -- Not Used.
 *
 * Outputs    : PASSED or FAILED
 *
 *******************************************************************************
 */
int diag_peci_get_temp (int opt)
{
    n2g_i2c_if_t i2c_if;
    unsigned int ix, rc;

    /* PECI control data and PECI command for GetTemp of PECI proxy write */
    char wbuf[SMBUS_PECI_PROXY_GETTEMP_WSIZE] = { 0x5, 0x0, 0x30, 0x1, 0x2, 0x1 };

    char rbuf[SMBUS_PECI_PROXY_GETTEMP_RSIZE];
    short temp;
    int ref_temp = 0;

    memset(&i2c_if, 0, sizeof(i2c_if));

    i2c_if.i2c_bus_type = IOFPGA_I2C;
    i2c_if.i2c_ctrl = I2C_CTRL_TWO;
    i2c_if.mux = I2C_MUX_ONE;
    i2c_if.i2c_dev = SMBUS_PECI_PROXY_WRITE_ADDR;
    i2c_if.offset = SMBUS_PECI_MODE_CMD_CODE;
    i2c_if.sub_addr_len = 1;
    i2c_if.size = SMBUS_PECI_PROXY_GETTEMP_WSIZE;
    i2c_if.buf = wbuf;

    rc = n2g_i2c_write(&i2c_if);
    if (rc != PASSED) {
        printf("%s at %s: unable to write i2c %#x. "
              "rc = %#x", __func__, __FILE__, i2c_if.i2c_dev, rc);
        return (FAILED);
    }

    memset(rbuf, 0, sizeof(rbuf));

    i2c_if.i2c_dev = SMBUS_PECI_PROXY_READ_ADDR;
    i2c_if.offset = SMBUS_READ_CMD_CODE;
    i2c_if.size = SMBUS_PECI_PROXY_GETTEMP_RSIZE;
    i2c_if.buf = rbuf;

    rc = n2g_i2c_read(&i2c_if);
    if (rc != PASSED) {
        printf("%s at %s: unable to read i2c %#x. "
              "rc = %#x", __func__, __FILE__, i2c_if.i2c_dev, rc);
        return (FAILED);
    }

    if (rbuf[0] == SMBUS_PECI_CMD_BUSY) {
        printf("%s at %s: SMBus PECI bus is busy.\n", __func__, __FILE__);
        return (FAILED);
    }

    if (rbuf[0] == SMBUS_PECI_CMD_ERR) {
        printf("%s at %s: PECI proxy command error(0x%X).\n",
               __func__, __FILE__, rbuf[1]);
        return (FAILED);
    }

    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("\n");
        for (ix = 0; ix < SMBUS_PECI_PROXY_GETTEMP_RSIZE; ix++) {
            printf("0x%02x ", rbuf[ix]&0xff);
        }
        printf("\n");
    }

    temp = rbuf[3] + (rbuf[4] << 8);
    temp = temp >> 6;

    ref_temp = get_cpu_ref_temp();

    printf("\nPhoenix CPU REF_TEMP = %d\n", ref_temp);
    printf("CPU Temperature: %d Celcius\n", ref_temp + temp);
    printf("\n");
    system("sensors");

    return (PASSED);
}

/* end of file */
