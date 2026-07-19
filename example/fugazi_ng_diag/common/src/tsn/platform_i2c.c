/* $Id: platform_i2c.c,v 1.9 2020/08/19 09:51:25 markzha Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/tsn/platform_i2c.c,v $
 *-----------------------------------------------------------------------------
 * platform_i2c.c - Overlord I2C utility menu. P40t is header for SM1 and P38t is header
 * for SM2
 *
 * Sept. 2007, Simon Yen
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
#include "i2c_dev.h"
#include "i2c_api.h"
#include "i2c_address.h"
#include "plat_defs.h"
#include "platform_i2c.h"
#include "platform_fpga.h"
#include "platform_fru.h"
#include "platform_cookie.h"
#include "platform_sensor.h"
#include "platform_dimm.h"
#include "platform_mcu.h"
#include "platform_wifi.h"

/*disable cterr*/
boolean g_i2c_read_cterr = FALSE;

/*
 * Functional prototype
 */
static int write_i2c(int);
static int read_i2c(int);
static int i2c_fd0 = -1;

unsigned char i2c_debug = 0;

static n2g_i2c_if_t cpu_i2c_dev[] = {
    /*
     * I2C 0
     */
    {
     .dev_name = "EEPROM",
     .offset = 0,
     .i2c_bus_type = CPU_I2C0,
     .i2c_dev = MB_I2C_ADDR_EEPROM,
     .i2c_ctrl = I2C_CTRL_ZERO,
     .sub_addr_len = 0,
     .size = sizeof(uint8_t),  /* dont' use sub address slave register */
     .mux = I2C_MUX_ZERO,
     .buf = NULL,
     }
    ,
#ifdef SUPPORT_DISCRETE_AIKIDO_ACT2
    {
     .dev_name = "ACT 2 Lite Secure Chip",
     .offset = -1,              /* need to be -1 to tell driver not to use offset !!! */
     .i2c_bus_type = CPU_I2C0,
     .i2c_dev = MB_I2C_ADDR_ACT2,
     .i2c_ctrl = I2C_CTRL_ZERO,
     .sub_addr_len = 0,
     .size = sizeof(uint16_t),
     .mux = I2C_MUX_ZERO,
     .buf = NULL,
     }
     ,
#endif
    {
     .dev_name = "Aikido ACT 2 Lite Secure Chip",
     .offset = -1,              /* need to be -1 to tell driver not to use offset !!! */
     .i2c_bus_type = CPU_I2C0,
     .i2c_dev = MB_I2C_ADDR_AIKIDO_ACT2,
     .i2c_ctrl = I2C_CTRL_ZERO,
     .sub_addr_len = 0,
     .size = sizeof(uint16_t),
     .mux = I2C_MUX_ZERO,
     .buf = NULL,
     }
     ,
    /*
     * I2C 1
     */
    {
     .dev_name = "Temperature Sensor(MAX31730AUB+T)",
     .offset = 0,
     .i2c_bus_type = CPU_I2C1,
     .i2c_dev = MB_I2C_ADDR_MB_TEMP,
     .i2c_ctrl = I2C_CTRL_ONE,
     .sub_addr_len = 0,
     .size = sizeof(uint8_t),
     .mux = I2C_MUX_ZERO,
     .buf = NULL,
     }
    ,
    {
     .dev_name = "SFP Module",
     .offset = 0,
     .i2c_bus_type = CPU_I2C1,
     .i2c_dev = MB_I2C_ADDR_SFP0,
     .i2c_ctrl = I2C_CTRL_ONE,
     .sub_addr_len = 0,
     .size = sizeof(uint8_t),
     .mux = I2C_MUX_ZERO,
     .buf = NULL,
     }
    ,
    /*
     * I2C 2
     */
    {
     .dev_name = "RTC DS1337",
     .offset = 0,
     .i2c_bus_type = CPU_I2C2,
     .i2c_dev = MB_I2C_ADDR_RTC,
     .i2c_ctrl = I2C_CTRL_TWO,
     .sub_addr_len = 0,
     .size = sizeof(uint8_t),
     .mux = I2C_MUX_ZERO,
     .buf = NULL,
     }
    ,
    {
     .dev_name = "PoE DC Controller",
     .offset = 0,
     .i2c_bus_type = CPU_I2C2,
     .i2c_dev = MB_I2C_ADDR_POE_30W_CTRLER,
     .i2c_ctrl = I2C_CTRL_TWO,
     .sub_addr_len = 0,
     .size = sizeof(uint8_t),
     .mux = I2C_MUX_ZERO,
     .buf = NULL,
     }
    ,
    {
     .dev_name = "PoE DC EEPROM",
     .offset = 0,
     .i2c_bus_type = CPU_I2C2,
     .i2c_dev = MB_I2C_ADDR_POE_EEPROM,
     .i2c_ctrl = I2C_CTRL_TWO,
     .sub_addr_len = 0,
     .size = sizeof(uint8_t),
     .mux = I2C_MUX_ZERO,
     .buf = NULL,
     }
    ,
    {
     .dev_name = "MCU",
     .offset = 0,
     .i2c_bus_type = CPU_I2C2,
     .i2c_dev = MB_I2C2_MCU,
     .i2c_ctrl = I2C_CTRL_TWO,
     .sub_addr_len = 0,
     .size = sizeof(uint16_t),
     .mux = I2C_MUX_ZERO,
     .buf = NULL,
     }
    ,
    {
     .dev_name = "MCU Bootloader",
     .offset = 0,
     .i2c_bus_type = CPU_I2C2,
     .i2c_dev = MB_I2C2_MCU_BOOTLOADER,
     .i2c_ctrl = I2C_CTRL_TWO,
     .sub_addr_len = 0,
     .size = sizeof(uint16_t),
     .mux = I2C_MUX_ZERO,
     .buf = NULL,
     }
    ,
    {
     .dev_name = "WiFi ACT2",
     .offset = -1,              /* need to be -1 to tell driver not to use offset !!! */
     .i2c_bus_type = CPU_I2C2,
     .i2c_dev = WIFI_I2C_ADDR_ACT2,
     .i2c_ctrl = I2C_CTRL_TWO,
     .sub_addr_len = 0,
     .size = sizeof(uint16_t),
     .mux = I2C_MUX_ZERO,
     .buf = NULL,
     }
    ,
    {
     .dev_name = "WiFi Temperature Sensor(MAX31730AUB+T)",
     .offset = 0,
     .i2c_bus_type = CPU_I2C2,
     .i2c_dev = WIFI_I2C_ADDR_TEMP,
     .i2c_ctrl = I2C_CTRL_TWO,
     .sub_addr_len = 0,
     .size = sizeof(uint8_t),
     .mux = I2C_MUX_ZERO,
     .buf = NULL,
     }
    ,
};

static n2g_i2c_if_t star_cpu_i2c_dev[] = {
    /*
     * I2C 0
     */
    {
     .dev_name = "EEPROM",
     .offset = 0,
     .i2c_bus_type = CPU_I2C0,
     .i2c_dev = MB_I2C_ADDR_EEPROM,
     .i2c_ctrl = I2C_CTRL_ZERO,
     .sub_addr_len = 0,
     .size = sizeof(uint8_t),  /* dont' use sub address slave register */
     .mux = I2C_MUX_ZERO,
     .buf = NULL,
     }
    ,
#ifdef SUPPORT_DISCRETE_AIKIDO_ACT2
    {
     .dev_name = "ACT 2 Lite Secure Chip",
     .offset = -1,              /* need to be -1 to tell driver not to use offset !!! */
     .i2c_bus_type = CPU_I2C0,
     .i2c_dev = MB_I2C_ADDR_ACT2,
     .i2c_ctrl = I2C_CTRL_ZERO,
     .sub_addr_len = 0,
     .size = sizeof(uint16_t),
     .mux = I2C_MUX_ZERO,
     .buf = NULL,
     }
     ,
#endif
    {
     .dev_name = "Aikido ACT 2 Lite Secure Chip",
     .offset = -1,              /* need to be -1 to tell driver not to use offset !!! */
     .i2c_bus_type = CPU_I2C0,
     .i2c_dev = MB_I2C_ADDR_AIKIDO_ACT2,
     .i2c_ctrl = I2C_CTRL_ZERO,
     .sub_addr_len = 0,
     .size = sizeof(uint16_t),
     .mux = I2C_MUX_ZERO,
     .buf = NULL,
     }
     ,
    /*
     * I2C 1
     */
    {
     .dev_name = "Temperature Sensor(MAX31730AUB+T)",
     .offset = 0,
     .i2c_bus_type = CPU_I2C1,
     .i2c_dev = MB_I2C_ADDR_MB_TEMP,
     .i2c_ctrl = I2C_CTRL_ONE,
     .sub_addr_len = 0,
     .size = sizeof(uint8_t),
     .mux = I2C_MUX_ZERO,
     .buf = NULL,
     }
    ,
    {
     .dev_name = "SFP Module",
     .offset = 0,
     .i2c_bus_type = CPU_I2C1,
     .i2c_dev = MB_I2C_ADDR_SFP0,
     .i2c_ctrl = I2C_CTRL_ONE,
     .sub_addr_len = 0,
     .size = sizeof(uint8_t),
     .mux = I2C_MUX_ZERO,
     .buf = NULL,
     }
    ,
    /*
     * I2C 2
     */
    {
     .dev_name = "RTC DS1337",
     .offset = 0,
     .i2c_bus_type = CPU_I2C2,
     .i2c_dev = MB_I2C_ADDR_RTC,
     .i2c_ctrl = I2C_CTRL_TWO,
     .sub_addr_len = 0,
     .size = sizeof(uint8_t),
     .mux = I2C_MUX_ZERO,
     .buf = NULL,
     }
    ,
    {
     .dev_name = "PoE DC Controller",
     .offset = 0,
     .i2c_bus_type = CPU_I2C2,
     .i2c_dev = MB_I2C_ADDR_POE_30W_CTRLER,
     .i2c_ctrl = I2C_CTRL_TWO,
     .sub_addr_len = 0,
     .size = sizeof(uint8_t),
     .mux = I2C_MUX_ZERO,
     .buf = NULL,
     }
    ,
    {
     .dev_name = "PoE DC EEPROM",
     .offset = 0,
     .i2c_bus_type = CPU_I2C2,
     .i2c_dev = MB_I2C_ADDR_POE_EEPROM,
     .i2c_ctrl = I2C_CTRL_TWO,
     .sub_addr_len = 0,
     .size = sizeof(uint8_t),
     .mux = I2C_MUX_ZERO,
     .buf = NULL,
     }
    ,
    {
     .dev_name = "MCU",
     .offset = 0,
     .i2c_bus_type = CPU_I2C2,
     .i2c_dev = MB_I2C2_MCU,
     .i2c_ctrl = I2C_CTRL_TWO,
     .sub_addr_len = 0,
     .size = sizeof(uint16_t),
     .mux = I2C_MUX_ZERO,
     .buf = NULL,
     }
    ,
    {
     .dev_name = "MCU Bootloader",
     .offset = 0,
     .i2c_bus_type = CPU_I2C2,
     .i2c_dev = MB_I2C2_MCU_BOOTLOADER,
     .i2c_ctrl = I2C_CTRL_TWO,
     .sub_addr_len = 0,
     .size = sizeof(uint16_t),
     .mux = I2C_MUX_ZERO,
     .buf = NULL,
     }
    ,
    {
     .dev_name = "WiFi ACT2",
     .offset = -1,              /* need to be -1 to tell driver not to use offset !!! */
     .i2c_bus_type = CPU_I2C2,
     .i2c_dev = WIFI_I2C_ADDR_ACT2,
     .i2c_ctrl = I2C_CTRL_TWO,
     .sub_addr_len = 0,
     .size = sizeof(uint16_t),
     .mux = I2C_MUX_ZERO,
     .buf = NULL,
     }
    ,
     {
     .dev_name = "WiFi Temperature Sensor(MAX31730AUB+T)",
     .offset = 0,
     .i2c_bus_type = CPU_I2C2,
     .i2c_dev = WIFI_I2C_STAR_ADDR_TEMP,
     .i2c_ctrl = I2C_CTRL_TWO,
     .sub_addr_len = 0,
     .size = sizeof(uint8_t),
     .mux = I2C_MUX_ZERO,
     .buf = NULL,
     }
    ,
};

static n2g_i2c_if_t plug_fpga_tc_i2c_dev[] = {
    
    /* I2C Device for Pluggable FPGA */
    {
     .dev_name = "Pluggable Test Card Temperature Sensor(LM75BDP)",
     .offset = 0,
     .i2c_bus_type = PLUG_FPGA,
     .i2c_dev = PLUG_I2C_ADDR_TEMP,
     .i2c_ctrl = I2C_CTRL_ZERO,
     .sub_addr_len = 0,
     .size = sizeof(uint8_t),  
     .mux = I2C_MUX_ZERO,
     .buf = NULL,
     }
    ,
    {
     .dev_name = "Pluggable Test Card ACT2",
     .offset = -1,
     .i2c_bus_type = PLUG_FPGA,
     .i2c_dev = PLUG_I2C_ADDR_ACT2,
     .i2c_ctrl = I2C_CTRL_ZERO,
     .sub_addr_len = 0,
     .size = sizeof(uint8_t),  
     .mux = I2C_MUX_ZERO,
     .buf = NULL,
     }
    ,
    {
     .dev_name = "Pluggable Test Card GPIO Expander",
     .offset = 0,
     .i2c_bus_type = PLUG_FPGA,
     .i2c_dev = PLUG_TC_I2C_ADDR_GPIO_EXP,
     .i2c_ctrl = I2C_CTRL_ZERO,
     .sub_addr_len = 0,
     .size = sizeof(uint8_t),  
     .mux = I2C_MUX_ZERO,
     .buf = NULL,
     }
    ,
};

static n2g_i2c_if_t c1109_2p_cpu_i2c_dev[] = {
    /*
     * I2C 0
     */
    {
     .dev_name = "EEPROM",
     .offset = 0,
     .i2c_bus_type = CPU_I2C0,
     .i2c_dev = MB_I2C_ADDR_EEPROM,
     .i2c_ctrl = I2C_CTRL_ZERO,
     .sub_addr_len = 0,
     .size = sizeof(uint8_t),  /* dont' use sub address slave register */
     .mux = I2C_MUX_ZERO,
     .buf = NULL,
     }
    ,
#ifdef SUPPORT_DISCRETE_AIKIDO_ACT2
    {
     .dev_name = "ACT 2 Lite Secure Chip",
     .offset = -1,              /* need to be -1 to tell driver not to use offset !!! */
     .i2c_bus_type = CPU_I2C0,
     .i2c_dev = MB_I2C_ADDR_ACT2,
     .i2c_ctrl = I2C_CTRL_ZERO,
     .sub_addr_len = 0,
     .size = sizeof(uint16_t),
     .mux = I2C_MUX_ZERO,
     .buf = NULL,
     }
     ,
#endif
    {
     .dev_name = "Aikido ACT 2 Lite Secure Chip",
     .offset = -1,              /* need to be -1 to tell driver not to use offset !!! */
     .i2c_bus_type = CPU_I2C0,
     .i2c_dev = MB_I2C_ADDR_AIKIDO_ACT2,
     .i2c_ctrl = I2C_CTRL_ZERO,
     .sub_addr_len = 0,
     .size = sizeof(uint16_t),
     .mux = I2C_MUX_ZERO,
     .buf = NULL,
     }
     ,
    /*
     * I2C 1
     */
    {
     .dev_name = "Temperature Sensor(MAX31730AUB+T)",
     .offset = 0,
     .i2c_bus_type = CPU_I2C1,
     .i2c_dev = MB_I2C_ADDR_MB_TEMP,
     .i2c_ctrl = I2C_CTRL_ONE,
     .sub_addr_len = 0,
     .size = sizeof(uint8_t),
     .mux = I2C_MUX_ZERO,
     .buf = NULL,
     }
    ,
    /*
     * I2C 2
     */
    {
     .dev_name = "RTC DS1337",
     .offset = 0,
     .i2c_bus_type = CPU_I2C2,
     .i2c_dev = MB_I2C_ADDR_RTC,
     .i2c_ctrl = I2C_CTRL_TWO,
     .sub_addr_len = 0,
     .size = sizeof(uint8_t),
     .mux = I2C_MUX_ZERO,
     .buf = NULL,
     }
    ,
    {
     .dev_name = "MCU",
     .offset = 0,
     .i2c_bus_type = CPU_I2C2,
     .i2c_dev = MB_I2C2_MCU,
     .i2c_ctrl = I2C_CTRL_TWO,
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
    {"DIMM0", (PFT) build_plat_dimm_util_menu, 0,
     0, (PFT) 0, 0,
     (PFT) 0, 0},
    {"I2C read (no offset)", (PFT) read_i2c, FALSE,
     0, (PFT) 0, 0,
     (PFT) 0, 0},
    {"I2C write (no offset)", (PFT) write_i2c, FALSE,
     0, (PFT) 0, 0,
     (PFT) 0, 0},
    {"I2C read", (PFT) read_i2c, TRUE,
     0, (PFT) 0, 0,
     (PFT) 0, 0},
    {"I2C write", (PFT) write_i2c, TRUE,
     0, (PFT) 0, 0,
     (PFT) 0, 0},
    {"MCU utilities", (PFT)tsn_mcu_utils, FALSE, 
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
 * Function: build_i2c_menu
 *
 * Description: Build I2C menu.
 *
 * Inputs: None.
 *
 * Outputs: None.
 *
 **********************************************************************
 */
void build_i2c_menu (void)
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
uint32_t init_dimm_i2c_struct (n2g_i2c_dev_t * i2c_dev, uint32_t dimm_no)
{
    uint32_t rc = FAILED;

    i2c_dev->bus_no = CPU_I2C0;
    i2c_dev->rd_hd_size = HD_SIZE_2;
    i2c_dev->wr_hd_size = HD_SIZE_2;
    i2c_dev->dev_addr = MB_I2C_ADDR_EEPROM;
    i2c_fd0 = get_i2c_fd(0);

    /*
     * Set I2C device to SLAVE mode 
     */
    if (i2c_fd0 <= 0) {
        cterr('f', 0, "/dev/i2c-0 is not opened correctly.");
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
 * Function   : read_i2c_reg (int yes_offset)
 * Description: generic i2c read funtcion, allowing user to manually read from
 *              i2c register.
 * Inputs     : yes_offset. not used
 *
 * Outputs    : PASSED or FAILED
 *
 *******************************************************************************
 */
int read_i2c_reg (int yes_offset)
{
    n2g_i2c_if_t i2c_if;
    unsigned int addr, size, offset, rc, ix, bus;
    uchar d32[80];

    memset(&i2c_if, 0, sizeof(i2c_if));

    bus = getdec_answer("\nEnter i2c bus number", 2, 0, 3);

    i2c_if.i2c_bus_type = bus;

    i2c_if.i2c_ctrl = i2c_if.i2c_bus_type;

    i2c_if.mux = 0;

    addr = gethex_answer("Enter 7 bit slave address ", 0x0, 0x0, 0xFF);
    i2c_if.i2c_dev = addr;

    if (yes_offset) {
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
 * Function   : write_i2c_reg (int yes_offset)
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
 * Inputs     : yes_offset, flag set if user is expected to enter register offset
 *
 * Outputs    : PASSED or FAILED
 *
 *******************************************************************************
 */
int write_i2c_reg (int yes_offset)
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
        gethex_answer("Enter 7 bit slave address (on mux 0)", 0x0, 1, 0xFF);
    i2c_if.i2c_dev = addr;

    if (yes_offset) {
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
        cterr('f', 0, "%s at %s: unable to write i2c %#x. "
              "rc = %#x", __FUNCTION__, __FILE__, i2c_if.i2c_dev, rc);
        return (FAILED);
    }

    return (PASSED);
}


/**********************************************************************
 *
 * Function: read_i2c
 *
 * Description: entry point to read i2c device
 *
 * Input : has_offset -- flag set if user wants to specify reg
 *                       offset when sending request
 *
 * Output: PASSED
 *
 **********************************************************************
 */
static int read_i2c (int has_offset)
{
    read_i2c_reg(has_offset);
    return (PASSED);
}


/**********************************************************************
 *
 * Function: write_i2c
 *
 * Description: entry point to write i2c device
 *
 * Input : has_offset -- flag set if user wants to specify reg
 *                       offset when sending request
 *
 * Output: PASSED
 *
 **********************************************************************
 */
static int write_i2c (int has_offset)
{
    write_i2c_reg(has_offset);
    return (PASSED);
}

/*****************************************************************************
 *
 * Function   : tsn_i2c_reg_temp_reg_rw_test (int option)
 *
 * Description: do Temperature register R/W test for i2c compontents
 *              (not all registers are W/R register)
 *
 * Inputs     : option , for future use.
 *
 * Outputs    : PASSED or FAILED
 *
 ****************************************************************************
 */
int tsn_i2c_temp_reg_rw_test (int option)
{
    uint32_t ret_val = FAILED;
    uchar *tname = (uchar *) "Temperature register R/W test";

    testname("%s", tname);
    printf("\n");
    if(max31730_register_test() == FAILED) {
        ret_val = FAILED;
    }

    prpass(testpass, "%s, ", tname);

    return (ret_val);
}


/*-------------------------------------------------------------------
 *
 * Function : is_cterr_print_on
 * Description: Return TRUE if i2c read cterr is turned on
 * INPUT:  dummy -- not used.
 * OUTPUT: return TRUE or FALSE
 * -------------------------------------------------------------------
 */
int is_cterr_print_on (void)
{
    return (g_i2c_read_cterr);
}

/*****************************************************************************
 *
 * Function   : tsn_x64_i2c_scan_test (int option)
 *
 * Description: scan all i2c devices on tsn
 *
 * Inputs     : option ,for future use.
 *
 * Outputs    : PASSED or FAILED
 *
 ****************************************************************************
 */
int tsn_x64_i2c_scan_test (int option)
{
    uchar mb_get_loc[FRU_SIZE] = {0};
    uchar mb_get_pid[FRU_SIZE] = {0};
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
    platform_get_pid((char *)mb_get_pid);
    strcpy((char *)mb_get_loc, "MB");
    platform_fru_table[fru_table_offset].pid_string = mb_get_pid;
    platform_fru_table[fru_table_offset].location_string = mb_get_loc;

    /* Segment 2: Test step captured from prpass */
    /* Segment 3: Failure message captured from cterr */

    /* Segment 4: Components used */
    cterr_add_component("Marvell Armada 7040", "I2C", "ACT2/TAM, Boot Strap I2C EEPROM, RTC, SFP, or POE");

    /* Segment 5: register and memory dump */
    //cterr_add_reg_dump((PFV)bcm63168_get_xdsl_profile);

    /* Segment 6: Platform Environment initialized here*/
    cterr_add_env_dump((PFV)tsn_display_temp_errormsg);

    /* Segment 7: Top 3 Debugging Steps */
    cterr_add_debug("Check the interface between the Host SoC "
                    "and the failed I2C devices.",
                    "If there is no problem for these interfaces, "
                    "replace one I2C device and redo the test.");
#endif

    n2g_i2c_if_t i2c_if;
    uint32_t reg_val = 0, ret_val = FAILED, fail_ctr = 0;
    uint32_t ix, max_retry, status;
    uint8_t now_test = 0, test_end = 0, test_num = 1;
    uint8_t cpu_i2c_test_end = 0, c1109_2p_cpu_i2c_test_end = 0;
    uint8_t star_cpu_i2c_test_end = 0;
    uchar *tname = (uchar *) "I2C scan";
    int err_code = 0;
    boolean sfp_is_detected = FALSE; 
    boolean wifi_act2_test = FALSE;
    /*Disable Cterr*/ 
    g_i2c_read_cterr = FALSE;

    max_retry = MAX_RETRY;

    testname("%s", tname);
    prpass(testpass, "%s, ", tname);

    /*
     * Setup end of test by calculate all I2C device number
     */
    c1109_2p_cpu_i2c_test_end = (sizeof(c1109_2p_cpu_i2c_dev) / sizeof(n2g_i2c_if_t));
    star_cpu_i2c_test_end = (sizeof(star_cpu_i2c_dev) / sizeof(n2g_i2c_if_t));
    cpu_i2c_test_end = (sizeof(cpu_i2c_dev) / sizeof(n2g_i2c_if_t));
    if (this_is_star_c1109_2p() || this_is_supernova_c959_2p()) {
        test_end = c1109_2p_cpu_i2c_test_end;
    } else if (this_is_star() || this_is_supernova()) {
        test_end = star_cpu_i2c_test_end;
    } else {
        test_end = cpu_i2c_test_end;
    }

    for (now_test = 0; now_test < test_end; now_test++) {
        /*
         * Get I2C device structure
         */
        if (this_is_star_c1109_2p() || this_is_supernova_c959_2p()) {
            memcpy(&i2c_if, &c1109_2p_cpu_i2c_dev[now_test], sizeof(n2g_i2c_if_t));
        } else if (this_is_star() || this_is_supernova()) {
             memcpy(&i2c_if, &star_cpu_i2c_dev[now_test], sizeof(n2g_i2c_if_t));
        } else {
        memcpy(&i2c_if, &cpu_i2c_dev[now_test], sizeof(n2g_i2c_if_t));
        }
        i2c_if.buf = (char *) &reg_val;

        if ((NVRAM)->diagflag & D_VERBOSE) {
            printf
                ("Now testing %2d: I2C bus %2d, Mux %d, %-29s(0x%.2X)... ",
                 now_test, i2c_if.i2c_bus_type, i2c_if.mux, i2c_if.dev_name,
                 (i2c_if.i2c_dev << 1));
        } else {
            prpass(testpass, "[%2d] I2C_%d: %s, ",
                test_num, i2c_if.i2c_bus_type, i2c_if.dev_name);
        }

        /* Skipped to check SFP if no SFP module present
         */
        /* Check if SFP is available. if not, return failed */
        if (is_sfp_present(&sfp_is_detected) != PASSED) {
            printf("Skipped SFP Scan because SFP module is not detected.\n");
            continue;
        }

        if ((i2c_if.i2c_dev == MB_I2C_ADDR_SFP0) &&
            (sfp_is_detected != TRUE)) {
            printf("Skipped SFP Scan because SFP module is not present.\n");
            continue;
        }

        if ((i2c_if.i2c_bus_type == CPU_I2C0) && (i2c_if.i2c_dev == MB_I2C_ADDR_ACT2)) {
            printf("Skipped Discrete ACT2 since it should be removed in MP.\n");
            continue;
        }

        /* Skipped to check PoE if no PoE module present,
         * this is because PoE DC is optional for TSN-H.
         */
        if ((i2c_if.i2c_dev == MB_I2C_ADDR_POE_30W_CTRLER) &&
            (tsn_has_poe(0) != TRUE)) {
            printf("Skipped PoE because PoE DC(optional) is not present.\n");
            continue;
        }

        /* Skipped to check PoE if no PoE EEPROM module present,
         * this is because PoE DC is optional for TSN-H.
         */
        if ((i2c_if.i2c_dev == MB_I2C_ADDR_POE_EEPROM) &&
            (tsn_has_poe(0) != TRUE)) {
            printf("Skipped PoE because PoE EEPROM is not present.\n");
            continue;
        }
        /* Skipped if WiFi module is NOT present */
        if (i2c_if.i2c_bus_type == CPU_I2C2) {
            if ((i2c_if.i2c_dev == WIFI_I2C_ADDR_ACT2) ||
                (i2c_if.i2c_dev == WIFI_I2C_ADDR_TEMP)) {
                if (tsn_fpga_check_dev_present(FPGA_CPP_WLAN_PRESENT) != TRUE) {
                    printf("Skipped %s because WiFi module is not present.\n",
                           i2c_if.dev_name);
                    continue;
                } else if (this_is_star()) { 
                    printf("Skipped Wifi ACT because Star Wifi not ACT2.\n");
                    continue;
                } else if (this_is_supernova()) { 
                    printf("Skipped Wifi ACT because Supernova Wifi not ACT2.\n");
                    continue;
                } else {
                    /* Confirm WiFi is out of RESET before test */ 
                    if (tsn_release_wifi_from_reset() != PASSED) {
                        printf("%s(%d): Failed to release WiFi from RESET.\n",
                               __func__, __LINE__);
                        return (FAILED);
                    }
                    /* Wifi ACT2 needs some time to be out of reset */
                    wifi_act2_test = TRUE;
                    max_retry = WIFI_ACT2_MAX_RETRY;
                }
            }
        } 
        if (this_is_star() || this_is_supernova()) { 
            /* Skipped to check STAR/SUPERNOVA Wifi Temp if no Wifi DC present */
            if ((i2c_if.i2c_dev == WIFI_I2C_STAR_ADDR_TEMP) && (i2c_if.i2c_bus_type == CPU_I2C2) &&
               (tsn_fpga_check_dev_present(FPGA_CPP_WLAN_PRESENT)) != TRUE) {
               printf("Skipped Wifi Temp because Wifi DC is not present.\n");
               continue;
            }
        } else {
            /* Skipped to check Wifi Temp if no Wifi DC present */
            if ((i2c_if.i2c_dev == WIFI_I2C_ADDR_TEMP) && (i2c_if.i2c_bus_type == CPU_I2C2) &&
                (tsn_fpga_check_dev_present(FPGA_CPP_WLAN_PRESENT)) != TRUE) {
                printf("Skipped Wifi Temp because Wifi DC is not present.\n");
                continue;
            }  
        }
        /* Skipped SFP for Star/Supernova */
        if ((i2c_if.i2c_dev == MB_I2C_ADDR_SFP0) && (this_is_star() || this_is_supernova())) {
            if (this_is_star()) {
                printf("Skipped SFP with Star serial proiect.\n");
            } else {
                printf("Skipped SFP with Supernova serial proiect.\n");
            }
            continue;
        } 

        /* Skipped MCU bootloader mode */
        if ((i2c_if.i2c_dev == MB_I2C2_MCU_BOOTLOADER) && (i2c_if.i2c_bus_type == CPU_I2C2)) {
            printf("Skipped MCU bootloader mode.\n");
            continue;
        } 
        /*
         * Read I2C device Register 0
         */
        for (fail_ctr = ix = 0; ix < max_retry; ix++) {
            ret_val = n2g_i2c_read(&i2c_if);
            if (ret_val != PASSED) {
                if (wifi_act2_test == FALSE) { /* Do not count as failure if it wifi act2 test */
                    fail_ctr++;
                    err_code = i2c_err_no(&status);
                    if (max_retry > 1)
                        printf("warning: %s failed %s [i2c_status=%#x] during pass %d",
                                 i2c_if.dev_name, i2c_err_str(err_code), status,
                                 ix);
                }
            } else {
                break;
            }
            msleep(30);
        }
        if ((NVRAM)->diagflag & D_VERBOSE) {
            printf("Done\n");
        }

        test_num++;
    
        if (ret_val != PASSED) {
            cterr('f', 0, "%s failed %s [i2c_status=%#x]",
                     i2c_if.dev_name, i2c_err_str(err_code), status);
        } 
    
    }
        /* Enable cterr */
        g_i2c_read_cterr = TRUE;
        prpass(testpass, "%s, ", tname);
        prcomplete(testpass, errcount, (char *)0);
        return (ret_val);
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
    int size_cpu = 0;
    int size_tc_plug_fpga = (sizeof(plug_fpga_tc_i2c_dev) / sizeof(n2g_i2c_if_t));

    if (this_is_star_c1109_2p() || this_is_supernova_c959_2p()) {
        size_cpu = (sizeof(c1109_2p_cpu_i2c_dev) / sizeof(n2g_i2c_if_t));
        for (ix = 0; ix < size_cpu; ix++) {
            if (c1109_2p_cpu_i2c_dev[ix].i2c_dev == addr &&
                c1109_2p_cpu_i2c_dev[ix].mux == mux &&
                c1109_2p_cpu_i2c_dev[ix].i2c_ctrl == i2c) {
                return ((void *) (&c1109_2p_cpu_i2c_dev[ix]));
            }
        }
    } else if (this_is_star() || this_is_supernova()) {
        size_cpu = (sizeof(star_cpu_i2c_dev) / sizeof(n2g_i2c_if_t));
        for (ix = 0; ix < size_cpu; ix++) {
            if (star_cpu_i2c_dev[ix].i2c_dev == addr &&
                star_cpu_i2c_dev[ix].mux == mux &&
                star_cpu_i2c_dev[ix].i2c_ctrl == i2c) {
                    return ((void *) (&star_cpu_i2c_dev[ix]));
                }
        }
    } else {
        size_cpu = (sizeof(cpu_i2c_dev) / sizeof(n2g_i2c_if_t));
        for (ix = 0; ix < size_cpu; ix++) {
        if (cpu_i2c_dev[ix].i2c_dev == addr &&
                cpu_i2c_dev[ix].mux == mux &&
                cpu_i2c_dev[ix].i2c_ctrl == i2c) {
            return ((void *) (&cpu_i2c_dev[ix]));
        }
    }
    }
    for (ix = 0; ix < size_tc_plug_fpga; ix++) {
        if (plug_fpga_tc_i2c_dev[ix].i2c_dev == addr &&
            plug_fpga_tc_i2c_dev[ix].mux == mux && 
            plug_fpga_tc_i2c_dev[ix].i2c_ctrl == i2c) {
            return ((void *) (&plug_fpga_tc_i2c_dev[ix]));
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
    int size = 0;
    int size_tc_plug_fpga = (sizeof(plug_fpga_tc_i2c_dev) / sizeof(n2g_i2c_if_t));

    if (this_is_star_c1109_2p() || this_is_supernova_c959_2p()) {
        size = (sizeof(c1109_2p_cpu_i2c_dev) / sizeof(n2g_i2c_if_t));
        for (ix = 0; ix < size; ix++) {
            if (c1109_2p_cpu_i2c_dev[ix].i2c_dev == addr) {
                /*
                 * to support different type of module/motherboard, etc...
                 */
                c1109_2p_cpu_i2c_dev[ix].i2c_ctrl = ctrl_no;
                return ((void *) (&c1109_2p_cpu_i2c_dev[ix]));
            }
        }
    } else {
        size = (sizeof(cpu_i2c_dev) / sizeof(n2g_i2c_if_t));
    for (ix = 0; ix < size; ix++) {
        if (cpu_i2c_dev[ix].i2c_dev == addr) {
            /*
             * to support different type of module/motherboard, etc...
             */
            cpu_i2c_dev[ix].i2c_ctrl = ctrl_no;
            return ((void *) (&cpu_i2c_dev[ix]));
        }
    }
    }

    for (ix = 0; ix < size_tc_plug_fpga; ix++) {
        if (plug_fpga_tc_i2c_dev[ix].i2c_dev == addr) {
            /*
             * to support different type of module/motherboard, etc...
             */
            plug_fpga_tc_i2c_dev[ix].i2c_ctrl = ctrl_no;
            return ((void *) (&plug_fpga_tc_i2c_dev[ix]));
        }
    }
    /*
     * check mdoules now
     */
    printf("Software has no support for device at addr %#x; ctrl_no = %#x\n]",
         addr, ctrl_no);

    return (void *) NULL;
}


/*-------------------------------------------------------------------
 *
 * Function : is_need_dswap
 * Description: for declare is need dswap on this TSN platform 
 * INPUT:  dummy -- not used.
 * OUTPUT: return TRUE 
 * -------------------------------------------------------------------
 */
boolean is_need_dswap (void)
{
    return (TRUE);
}

/* end of file */
/******** History ********
*---------------------------------------------------
$Log: platform_i2c.c,v $
Revision 1.9  2020/08/19 09:51:25  markzha
*** empty log message ***

Revision 1.8  2019/03/07 09:51:32  lucywang
[Supernova] PID changed : C1101L-4P --> C951-4P, C1109L-2P --> C959-2P

Revision 1.7  2019/01/18 05:54:47  yungchen
Merge Supernova branch to the main trunk (CSCvn79871)

Revision 1.6  2018/03/27 12:46:38  hondwang
Code modify for Star_C1101_4PLTEP_4PLTEPWX and Pluggable LTE EM7455, WP7601, WP7603 ER

Revision 1.5  2018/02/27 11:18:32  palin2
Fixed TSN I2C scan test failed after Star merged in.

Revision 1.4.2.1  2018/03/01 06:54:35  lucywang
C1109-2P : added Discrete ACT2 back temporarily since P2 still keep to have it

Revision 1.4  2018/02/09 09:56:55  hondwang
Merge Star branch star-branch-c9xx to main trunk

Revision 1.3.16.2  2018/02/01 12:58:29  steja
Add platform function for common code

Revision 1.3.16.1  2018/01/20 06:27:24  hondwang
prepare merge star-branch-c9xx to main trunk

Revision 1.3  2017/09/06 12:14:20  steja
1.Fix TSN WIFI ACT2 i2c scan test failed at first time after power on (CSCvf83218)
2. Remove Discrete ACT2 utility and I2C Scan for Discrete ACT2 only for Development phase(CSCvf81035)

Revision 1.2.4.5  2017/11/20 07:54:32  lucywang
Changed PID to C1101/C1109-2P/C1109-4P

Revision 1.2.4.4  2017/11/10 08:15:28  shjung
Removed discrete ACT 2

Revision 1.2.4.3  2017/08/28 07:45:58  shjung
Removed pluggable card I2C scan test

Revision 1.2.4.2  2017/08/28 03:34:13  lucywang
modified for C949-2P

Revision 1.2.4.1  2017/08/15 14:18:39  hondwang
star branch c9xx initial check in

Revision 1.2  2017/08/02 14:21:48  steja
Support TSN-H/M platform code

Revision 1.1.8.2  2017/07/29 03:41:20  steja
tsn-branch5 synced with Maintrunk repositories

Revision 1.1.6.3  2017/07/24 14:14:11  palin2
1. To improve code readability.
2. All changes are verified before check-in.

Revision 1.1.6.2  2017/07/20 13:38:07  steja
tsn-branch4 merge with maintrunk

Revision 1.1.4.7.2.4  2017/07/20 11:29:02  steja
Code cleanup

Revision 1.1.4.7.2.3  2017/07/18 14:23:37  steja
Code Cleanup

Revision 1.1.4.7.2.2  2017/07/11 13:46:07  steja
1. Add Check Motherboard Aikido cookie ID
2. Add Check Aikido I2C Scan
2. Add Check SFP present I2C Scan

Revision 1.1.4.7.2.1  2017/02/23 11:03:16  palin2
Updated code based on FPGA changes. These updates are verified on P2A TSN.

Revision 1.1.4.7  2016/11/15 13:19:09  petteng
Add enhanced error message

Revision 1.1.4.6  2016/10/04 06:39:08  petteng
Add enhanced error message

Revision 1.1.4.5  2016/07/25 09:32:06  steja
Add Wlan DC present or not present

Revision 1.1.4.4  2016/07/21 14:26:32  steja
1. Update Temperature sensor function
2. Add Wifi Temperature sensor to basic utilities
3. Add LTE Extended Feature Test
4. Display Chasis Temperature When Boot up
5. Update I2C scan function
6. Update POE cookie check card present

Revision 1.1.4.3  2016/07/18 13:14:29  steja
1. Move M/B Temperature sensor register test to run as default test.
2. Move M/B Temperature utilities under basic utilities.

Revision 1.1.4.2  2016/06/30 06:22:50  steja
tsn-branch2 sync with main trunk

Revision 1.1.2.9  2016/06/21 04:36:33  palin2
Added voltage margin utility and MCU register R/W utilities.

Revision 1.1.2.8  2016/06/16 07:51:11  palin2
Updated PoE related utilities and code.

Revision 1.1.2.7  2016/05/24 01:18:11  palin2
Updated Thermal sensor and ACT2 chip I2C bus number based on P1A HW changes

Revision 1.1.2.6  2016/05/06 16:10:18  steja
Bring up I2C-2 for RTC

Revision 1.1.2.5  2016/04/23 15:00:29  steja
Check in for fix SPD Read RAW

Revision 1.1.2.4  2016/04/22 11:34:00  steja
check-in for first release

Revision 1.1.2.3  2016/03/27 14:17:34  steja
update based on code review comment 3/25/2016

Revision 1.1.2.2  2016/03/24 10:35:04  steja
Add Cookie and Act2 programming

Revision 1.1.2.1  2016/03/20 05:32:40  steja
1. Add i2c scan and i2c read write utility
2. Add sensor temperature and eeprom utility


$Endlog$
*/
