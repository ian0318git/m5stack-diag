 /* $Id: diag_i2c_lib.c,v 1.4 2020/08/06 07:54:55 kehuang2 Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/tabei-l/diag_i2c_lib.c,v $
 *-----------------------------------------------------------------------------
 * diag_i2c_lib.c - I2C library
 *
 *
 * Copyright (c) 2016 - 2019 by Cisco Systems, Inc.
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
int switch_sfp_mux(int);
int switch_sfp_mux_util(void);
uint32_t n2g_i2c_init(n2g_i2c_if_t *);
uint32_t n2g_i2c_open(n2g_i2c_if_t *);
uint32_t n2g_i2c_close(n2g_i2c_if_t *);
void *platform_get_wic_act2(int);
static int i2c_fd0 = -1;
static n2g_i2c_if_t wic_act2[MAX_WIC+FIRST_SLOT];
static uint8_t wic_i2c_ctrl[] = {0,  WIC1_I2C_CTRL, WIC2_I2C_CTRL, WIC3_I2C_CTRL};

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

};

static n2g_i2c_if_t plug_fpga_tc_i2c_dev[] = {

    /* I2C Device for Pluggable FPGA */
    {
     .dev_name = "Pluggable Test Card Temperature Sensor(LM75BDP)",
     .offset = 0,
     .i2c_bus_type = IOFPGA_I2C,
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
     .i2c_bus_type = IOFPGA_I2C,
     .i2c_dev = PLUG_I2C_ADDR_ACT2,
     .i2c_ctrl = I2C_CTRL_ZERO,
     .sub_addr_len = 0,
     .size = sizeof(uint8_t),
     .mux = I2C_MUX_ZERO,
     .buf = NULL,
     }
    ,
    {
     .dev_name = "Pluggable (SGMII) Test Card GPIO Expander",
     .offset = 0,
     .i2c_bus_type = IOFPGA_I2C,
     .i2c_dev = PLUG_SGMII_TC_I2C_ADDR_GPIO_EXP,
     .i2c_ctrl = I2C_CTRL_ZERO,
     .sub_addr_len = 0,
     .size = sizeof(uint8_t),
     .mux = I2C_MUX_ZERO,
     .buf = NULL,
     }
    ,
    {
     .dev_name = "Pluggable (PCIe) Test Card GPIO Expander",
     .offset = 0,
     .i2c_bus_type = IOFPGA_I2C,
     .i2c_dev = PLUG_PCIE_TC_I2C_ADDR_GPIO_EXP,
     .i2c_ctrl = I2C_CTRL_ZERO,
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
    {"SFP Mux Switch", (PFT) switch_sfp_mux_util, 0,
     0, 0, 0,
     (PFT) 0, 0},
    {"Show barometer info", (PFT) show_barometer_info, TRUE,
     0, (type_t(*)())has_barometer, 0,
     (PFT) 0, 0},
    {"Display barometer register", (PFT) display_barometer_reg, TRUE,
     0, (type_t(*)())has_barometer, 0,
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
    unsigned int choose_dimm;

    i2c_dev->bus_no = CPU_I2C0;
    i2c_dev->rd_hd_size = HD_SIZE_1;
    i2c_dev->wr_hd_size = HD_SIZE_1;
    i2c_fd0 = get_i2c_fd(0);

    if (is_fortnite() == TRUE) {
        i2c_dev->dev_addr = MB_I2C_ADDR_DIMM0;
    } else {
        choose_dimm = getdec_answer("\nEnter which DIMM"
                      "(0.DIMM0 1.DIMM1)",0,0,1);
        if (choose_dimm == 0) {
            i2c_dev->dev_addr = MB_I2C_ADDR_DIMM0;
        } else {
            i2c_dev->dev_addr = MB_I2C_ADDR_DIMM1;
        }
    }
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
    unsigned int addr, offset, rc, ix, bus, ctrl;
    uint16_t size;
    uchar d32[80];

    memset(&i2c_if, 0, sizeof(i2c_if));

    bus = getdec_answer("\nEnter i2c bus number(I2C0 - 0, I2C1 - 1, IOFPGA - 3)", 0, 0, 3);

    i2c_if.i2c_bus_type = bus;

    ctrl = getdec_answer("\nEnter i2c controller number", 0, 0, 20);

    i2c_if.i2c_ctrl = ctrl;

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
    unsigned int addr, size, rc, ix, bus, ctrl;
    int offset;
    char msg[80];
    uint8_t d8[32];

    memset(&i2c_if, 0, sizeof(i2c_if));

    bus = getdec_answer("\nEnter i2c bus number(0-I2C0, 1-I2C1, 3-IOFPGA)", 0, 0, 3);

    i2c_if.i2c_bus_type = bus;

    ctrl = getdec_answer("\nEnter i2c controller number", 0, 0, 20);

    i2c_if.i2c_ctrl = ctrl;

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
 * Function   : switch_sfp_mux
 *
 * Description: Switch Mux PCA9543A for access sfp module
 *
 * Inputs     : which_sfp -  SFP0: SFP 0
 *                           SFP1: SFP 1
 *
 * Outputs    : PASSED or FAILED
 *
 *******************************************************************************
 */
int switch_sfp_mux (int which_sfp)
{
    n2g_i2c_if_t i2c_if;
    int rc;
    uint8_t d8[1] = {0};

    memset(&i2c_if, 0, sizeof(i2c_if));

    i2c_if.i2c_bus_type = IOFPGA_I2C;
    i2c_if.i2c_ctrl = I2C_CTRL_FIVE;
    i2c_if.mux = I2C_MUX_ZERO;
    i2c_if.i2c_dev = MB_I2C_MUX_SFP;
    i2c_if.offset = 0;
    i2c_if.size = MUX_9543_COMMAND_SIZE;

    /* According to Spec of PCA9543A to Switch corresponding Mux */
    if (which_sfp == SFP0) {
        d8[0] = MUX_9543_EN_CHANNEL0;
        printf("Switch to SFP0\n");
    } else {
        d8[0] = MUX_9543_EN_CHANNEL1;
        printf("Switch to SFP1\n");
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
 * Function   : switch_sfp_mux_util
 *
 * Description: Utility to switch Mux PCA9543A for access sfp module
 *
 * Inputs     : None
 *
 * Outputs    : PASSED or FAILED
 *
 *******************************************************************************
 */
int switch_sfp_mux_util (void)
{
    int which_sfp;
    which_sfp = gethex_answer("Switch to which SFP (0.SFP0 1.SFP1)", 0, 0, 1);
    if (which_sfp == SFP0) {
        if (switch_sfp_mux(SFP0) == FAILED) {
            printf("Can't switch sfp mux\n");
            return (FAILED);
        }
    } else {
        if (switch_sfp_mux(SFP1) == FAILED) {
            printf("Can't switch sfp mux\n");
            return (FAILED);
        }
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
    n2g_i2c_if_t *i2c;
    int size = (sizeof(cpu_i2c_dev) / sizeof(n2g_i2c_if_t));
    int size_tc_plug_fpga = (sizeof(plug_fpga_tc_i2c_dev) / sizeof(n2g_i2c_if_t));   

    /* Motherboard */
    for (ix = 0; ix < size; ix++) {
        if (cpu_i2c_dev[ix].i2c_dev == addr) {
            cpu_i2c_dev[ix].i2c_ctrl = ctrl_no;
            return ((void *) (&cpu_i2c_dev[ix]));
        }
    }

    /* PIM modules */
    for (ix = 0; ix < size_tc_plug_fpga; ix++) {
        if (plug_fpga_tc_i2c_dev[ix].i2c_dev == addr) {
            plug_fpga_tc_i2c_dev[ix].i2c_ctrl = ctrl_no;
            return ((void *) (&plug_fpga_tc_i2c_dev[ix]));
        }
    }

    /* WIC modules */
    if (addr == NGIOWIC_I2C_ADDR_ACT2) {
        i2c = (n2g_i2c_if_t *)platform_get_wic_act2(FIRST_SLOT);
        if (i2c->i2c_ctrl == ctrl_no){
            return((void*)i2c);
        }
    }

    /*
     * check mdoules now
     */
    printf("Software has no support for device at addr %#x; ctrl_no = %#x\n]",
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

/* end of file */
/******** History ********
*---------------------------------------------------
$Log: diag_i2c_lib.c,v $
Revision 1.4  2020/08/06 07:54:55  kehuang2
Collapse Promethium into main trunk

Revision 1.3  2019/11/25 08:55:51  kehuang2
Collapse Tabei-L into main trunk

Revision 1.2  2019/10/17 02:16:22  kehuang2
Collapse Tabei-L into main trunk

Revision 1.1.2.17  2019/09/20 07:00:34  kehuang2
Clean up code base on review comment

Revision 1.1.2.16  2019/08/26 07:55:00  kehuang2
Clean up code by the comment of code review

Revision 1.1.2.15  2019/08/06 07:20:28  kehuang2
Update present function base on the comment of code review

Revision 1.1.2.14  2019/07/15 11:28:47  kehuang2
Support Barometer test and utility

Revision 1.1.2.13  2019/07/09 06:11:31  kehuang2
Update I2C bus change and enhence I2C scan coverage

Revision 1.1.2.12  2019/05/29 11:36:45  kehuang2
Update DIMM info for different sinario

Revision 1.1.2.11  2019/05/24 09:56:11  kehuang2

1.Update Temp Interrupt test
2.Clean up code

Revision 1.1.2.10  2019/05/21 09:18:51  kehuang2
Support Port80 LED

Revision 1.1.2.9  2019/05/21 03:18:00  kehuang2

1.SFP EN LED Support base on PreP2B respin
2.Support SFP Mux access utility

Revision 1.1.2.8  2019/04/29 08:14:26  kehuang2
Clean up code

Revision 1.1.2.7  2019/02/25 07:11:50  meho
Support new PIM test-card (PCIe).

Revision 1.1.2.6  2018/11/02 02:39:03  kodko
Support cookie read for NIM and PIM modules.

Revision 1.1.2.5  2018/10/25 09:55:24  harrchan
Add MCU utility in I2C utility

Revision 1.1.2.4  2018/10/19 01:44:19  harrchan
I2C scan test

Revision 1.1.2.3  2018/10/18 03:17:30  olin2
Clean up redefined MACRO

Revision 1.1.2.2  2018/10/09 09:22:04  olin2
Initial commit for NIM test

Revision 1.1.2.1  2018/10/02 01:49:59  harrchan
Initial commit for Tabei-L P1A bring up.

$Endlog$
*/
