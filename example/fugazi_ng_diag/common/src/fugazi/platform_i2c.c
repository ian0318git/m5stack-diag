/* $Id: platform_i2c.c,v 1.4 2021/08/25 06:31:34 iachang Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/fugazi/platform_i2c.c,v $
 *-----------------------------------------------------------------------------
 * platform_i2c.c - Overlord I2C utility menu. P40t is header for SM1 and P38t is header
 * for SM2
 *
 * Sept. 2007, Simon Yen
 *
 * Copyright (c) 2014-2020 by Cisco Systems, Inc.
 * All rights reserved.
 *-----------------------------------------------------------------------------
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <setjmp.h>
#include "endians.h"
#include "common.h"
#include "types.h"
#include "defs.h"
#include "menu.h"
#include "proto.h"
#include "signals.h"
#include "error.h"
#include "nvmonvars.h"
#include "mon_plat_defs.h"
#include "cross_platform.h"
#include "ngio.h"
#include "pca9545a.h"
#include "n2g_api_rc.h"
#include "queryflags.h"
#include "i2c_dev.h"
#include "i2c_api.h"
#include "plat_defs.h"
#include "platform_i2c.h"
#include "i2c_address.h"
#include "platform_psu.h"
#include "linux_api.h"
#include "byteswap.h"
#include "goofy_i2c.h"
#include "dash_fpga.h"
#include "platform_i2c.h"
#include "mb_tests.h"
#include "platform_temp_sensor.h"

/*
 *  Externs
 */
extern jmp_buf *monjmpptr;
extern int get_i2c_fd(int);
extern boolean is_overlord(void);

/* for utilities submenu. */
extern int show_dimm(int dimm_no);
extern int alter_dimm(int dimm_no);
extern int show_ich_i2c(void);
extern uint32_t get_platform_memsize(void);
extern void build_pwr_seq_menu(int);
extern int build_ts_menu(int);
extern int rtc_init(int);
extern int build_sfp_cookie_menu(int);
extern int build_eeprom_menu(int);
extern int build_i2c_usb_menu(int);
extern int build_mux_menu(uint32_t);
extern void build_pem_menu(uint32_t);
extern void unreset_platform_in_dev(int);
extern int show_barometer_info(void);
extern int display_barometer_reg(void);
extern int fugazi_pcie_clk_i2c_scan_test(char *);
extern int fugazi_sys_clk_i2c_scan_test(char *);
extern int set_mux_channel(n2g_i2c_dev_t *, uint8_t, uint32_t);

/*
 * Functional prototype
 */
boolean has_dimm1(void);

int show_temp(int, int);
extern int show_margins(int);

static int fpga_i2c_scan_addr(int);
static int write_i2c(int);
static int read_i2c(int);
static int platform_i2c_debug(int);

int fugazi_dimm_i2c_scan_test(char *, uint32_t);
int fugazi_tps536xx_i2c_scan_test(char *, uint32_t);
int reset_i2c_controller(int);
int display_mem_info(void);
extern uint32_t get_ngio_pcie_bus_num(void);


/*
 *  Globals
 */
unsigned char i2c_debug = 0;

static int i2c_fd = -1;

static n2g_i2c_if_t fpga_i2c_dev[] = {
    {
        .dev_name = "PCIe CLK Buf",
        .offset = -1,  /* need to be -1 to tell driver not to use offset !!! */
        .i2c_bus_type = IOFPGA_I2C,
        .i2c_dev = MB_I2C_ADDR_SYNCE_REG,
        .i2c_ctrl = I2C_CTRL_FIVE,
        .sub_addr_len = 0,
        .size    = sizeof(uint16_t),
        .mux = I2C_MUX_ZERO,
        .buf        = NULL,
    },
    {
        .dev_name = "USB Console",
        .offset = 0,
        .i2c_bus_type = IOFPGA_I2C,
        .i2c_dev = MB_I2C_ADDR_USB_CONSOLE,
        .i2c_ctrl = I2C_CTRL_ONE,
        .sub_addr_len = 0,
        .size = sizeof(uint8_t),
        .mux = I2C_MUX_ZERO,
        .buf = NULL,
    },
    {
        .dev_name = "USB Console Fw",
        .i2c_bus_type = IOFPGA_I2C,
        .i2c_dev = MB_I2C_ADDR_USB_CONSOLE_FW_DL,
        .i2c_ctrl = I2C_CTRL_ONE,
        .offset = -1,
        .sub_addr_len = 0,
        .size = sizeof(uint8_t),
        .mux = I2C_MUX_ZERO,
        .buf = NULL,
    },
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
    },
    {
        .dev_name = "ST Barometer",
        .offset = 0,
        .i2c_bus_type = IOFPGA_I2C,
        .i2c_dev = MB_I2C_ADDR_SENSOR,
        .i2c_ctrl = I2C_CTRL_TWO,
        .sub_addr_len = 0,
        .size = sizeof(uint8_t),
        .mux = I2C_MUX_ZERO,
        .buf = NULL,
    },
    {
        .dev_name = "Temp1 INLET1",
        .offset = 0,
        .i2c_bus_type = IOFPGA_I2C,
        .i2c_dev = MB_I2C_ADDR_TEMP1_IN_1,
        .i2c_ctrl = I2C_CTRL_TWO,
        .sub_addr_len = 0,
        .size = sizeof(uint32_t),
        .mux = I2C_MUX_ZERO,
        .buf = NULL,
    },
    {
        .dev_name = "Temp2 INLET2",
        .offset = 0,
        .i2c_bus_type = IOFPGA_I2C,
        .i2c_dev = MB_I2C_ADDR_TEMP2_IN_2,
        .i2c_ctrl = I2C_CTRL_TWO,
        .sub_addr_len = 0,
        .size = sizeof(uint32_t),
        .mux = I2C_MUX_ZERO,
        .buf = NULL,
    },
    {
        .dev_name = "Temp3 OUTLET1",
        .offset = 0,
        .i2c_bus_type = IOFPGA_I2C,
        .i2c_dev = MB_I2C_ADDR_TEMP3_out_1,
        .i2c_ctrl = I2C_CTRL_TWO,
        .sub_addr_len = 0,
        .size = sizeof(uint32_t),
        .mux = I2C_MUX_ZERO,
        .buf = NULL,
    },
    {
        .dev_name = "Temp4 OUTLET2",
        .offset = 0,
        .i2c_bus_type = IOFPGA_I2C,
        .i2c_dev = MB_I2C_ADDR_TEMP4_out_2,
        .i2c_ctrl = I2C_CTRL_TWO,
        .sub_addr_len = 0,
        .size = sizeof(uint32_t),
        .mux = I2C_MUX_ZERO,
        .buf = NULL,
    },
    {
        .dev_name = "PSU1 EEPROM",
        .offset = 0,
        .i2c_bus_type = IOFPGA_I2C,
        .i2c_dev = MB_I2C_ADDR_PSU1_EEPROM,
        .i2c_ctrl = I2C_CTRL_FOUR,
        .sub_addr_len = 0,
        .size = sizeof(uint32_t),
        .rd_hd_size = I2C_SMBUS_BLOCK_DATA,
        .wr_hd_size = I2C_SMBUS_BLOCK_DATA,
        .mux = I2C_MUX_ZERO,
        .buf = NULL,
    },
    {
        .dev_name = "PSU1 Microcontroller",
        .offset = 0,
        .i2c_bus_type = IOFPGA_I2C,
        .i2c_dev = MB_I2C_ADDR_PSU1_MCNTRL,
        .i2c_ctrl = I2C_CTRL_FOUR,
        .sub_addr_len = 0,
        .size = sizeof(uint32_t),
        .rd_hd_size = I2C_SMBUS_WORD_DATA,
        .wr_hd_size = I2C_SMBUS_WORD_DATA,
        .mux = I2C_MUX_ZERO,
        .buf = NULL,
    },
    {
        .dev_name = "PSU2 EEPROM",
        .offset = 0,
        .i2c_bus_type = IOFPGA_I2C,
        .i2c_dev = MB_I2C_ADDR_PSU2_EEPROM,
        .i2c_ctrl = I2C_CTRL_FOUR,
        .sub_addr_len = 0,
        .size = sizeof(uint32_t),
        .rd_hd_size = I2C_SMBUS_BLOCK_DATA,
        .wr_hd_size = I2C_SMBUS_BLOCK_DATA,
        .mux = I2C_MUX_ZERO,
        .buf = NULL,
    },
    {
        .dev_name = "PSU2 Microcontroller",
        .offset = 0,
        .i2c_bus_type = IOFPGA_I2C,
        .i2c_dev = MB_I2C_ADDR_PSU2_MCNTRL,
        .i2c_ctrl = I2C_CTRL_FOUR,
        .sub_addr_len = 0,
        .size = sizeof(uint32_t),
        .rd_hd_size = I2C_SMBUS_WORD_DATA,
        .wr_hd_size = I2C_SMBUS_WORD_DATA,
        .mux = I2C_MUX_ZERO,
        .buf = NULL,
    },
    {
        .dev_name = "USB0 Redriver",
        .offset = 0,
        .i2c_bus_type = IOFPGA_I2C,
        .i2c_dev = MB_I2C_ADDR_USB0_REDRIVER,
        .i2c_ctrl = I2C_CTRL_SIXTEEN,
        .sub_addr_len = 0,
        .size = sizeof(uint32_t),
        .mux = I2C_MUX_ZERO,
        .buf = NULL,
    },
    {
        .dev_name = "USB1 Redriver",
        .offset = 0,
        .i2c_bus_type = IOFPGA_I2C,
        .i2c_dev = MB_I2C_ADDR_USB1_REDRIVER,
        .i2c_ctrl = I2C_CTRL_SIXTEEN,
        .sub_addr_len = 0,
        .size = sizeof(uint32_t),
        .mux = I2C_MUX_ZERO,
        .buf = NULL,
    },
    {
        .dev_name = "SMLink 0",
        .offset = 0,
        .i2c_bus_type = IOFPGA_I2C,
        .i2c_dev = MB_I2C_ADDR_SMLINK_0,
        .i2c_ctrl = I2C_CTRL_TWO,
        .sub_addr_len = 0,
        .size = sizeof(uint32_t),
        .mux = I2C_MUX_TWO,
        .buf = NULL,
    },
    {
        .dev_name = "SMLink 1",
        .offset = 0,
        .i2c_bus_type = IOFPGA_I2C,
        .i2c_dev = MB_I2C_ADDR_SMLINK_1,
        .i2c_ctrl = I2C_CTRL_TWO,
        .sub_addr_len = 0,
        .size = sizeof(uint32_t),
        .mux = I2C_MUX_ONE,
        .buf = NULL,
    },
};

/* used for general act2 devices, mb, non-ngio modules */
static n2g_i2c_if_t general_act2[] = {
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

/*
 * I2C Utility Menu.
 */
static submenu_xtable_t i2c_menu_table[] = {
    {"Display Memory Info",            (PFT)display_mem_info,  TRUE,
        0,                             (PFT)0,                          0,
      (PFT)0,                          0},
    {"Show Barometer Info",            (PFT)show_barometer_info,        TRUE,
        0,                             (PFT)0,                          0,
      (PFT)0,                          0},
    {"Display Barometer Registers",    (PFT)display_barometer_reg,        TRUE,
        0,                             (PFT)0,                          0,
      (PFT)0,                          0},
    {"Temperature Sensor",             (PFT)build_ts_menu,            TRUE,
        0,                             (PFT)0,                          0,
      (PFT)0,                          0},
    {"Power Sequencer",                (PFT)build_pwr_seq_menu,         TRUE,
        0,                             (PFT)0,                          0,
      (PFT)0,                          0},
    {"PSU MUX Utility",	               (PFT)build_mux_menu,             1,
        0,                             (PFT)0,                          0,
      (PFT)0,                          0},
    {"PSU1",                           (PFT)build_pem_menu,             FUGAZI_PSU1_TRUE,
        0,                             (PFT)0,                          0,
      (PFT)0,                          0},
    {"PSU2",                           (PFT)build_pem_menu,             FUGAZI_PSU2_TRUE,
        0,                             (PFT)0,                          0,
      (PFT)0,                          0},
    {"USB Console",                    (PFT)build_i2c_usb_menu,         TRUE,
        0,                             (PFT)0,                          0,
      (PFT)0,                          0},
    {"FPGA I2C scan addr.",            (PFT)fpga_i2c_scan_addr,         FALSE,
        0,                             (PFT)0,                          0,
      (PFT)0,                          0},
    {"I2C read (no offset)",           (PFT)read_i2c,                   FALSE,
        0,                             (PFT)0,                          0,
      (PFT)0,                          0},
    {"I2C write (no offset)",          (PFT)write_i2c,                  FALSE,
        0,                             (PFT)0,                          0,
      (PFT)0,                          0},
    {"I2C read",                       (PFT)read_i2c,                   TRUE,
        0,                             (PFT)0,                          0,
      (PFT)0,                          0},
    {"I2C write",                      (PFT)write_i2c,                  TRUE,
        0,                             (PFT)0,                          0,
      (PFT)0,                          0},
    {"Toggle i2c debug flag",          (PFT)platform_i2c_debug,         TRUE,
        0,                             (PFT)0,                          0,
      (PFT)0,                          0},
    {"Reset I2C controller",           (PFT)reset_i2c_controller,       TRUE,
        0,                             (PFT)0,                          0,
      (PFT)0,                          0},
};

#define I2C_MENU_TABLE_SIZE \
        (sizeof(i2c_menu_table) / sizeof(submenu_xtable_t))

/*
 * Primary & secondary submenu items (filled in from xtable)
 */
static mitem_t i2c_menu_primary_items[I2C_MENU_TABLE_SIZE + MAX_BASE_ITEMS];
static mitem_t i2c_menu_secondary_items[I2C_MENU_TABLE_SIZE + MAX_BASE_ITEMS];

static struct menuinfo i2cdiag = {
  "I2C Utility Menu",		    /* title */
  0,                                /* title string added by init_empty_menu */
  (PFT)menu_show_dflags,            /* shows major flags */
  0,                                /* generic prompt */
  0,                                /* size -- bumped by add_menu_item() */
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

/**********************************************************************
 *
 * Function:	platform_cpu_i2c_init
 *
 * Description:	Initialize CPU I2C bus controllers.
 *
 * Inputs:	None.
 *
 * Outputs:	PASSED/FAILED.
 *
 **********************************************************************
 */
int platform_cpu_i2c_init(void)
{
    uint32_t rc = PASSED;
    int i;
    n2g_i2c_if_t i2c_if;

    /*currently not used??? */
    for (i = CPU_I2C1; i <= CPU_I2C1; i++) {
	i2c_if.i2c_bus_type = i;	/* Setup the bus controller number */
	/* CPU I2C controller uses 100 KHz */
	i2c_if.i2c_speed = N2G_I2C_100KHZ;
	if (n2g_i2c_init(&i2c_if) != PASSED) {
	    rc = FAILED;
	}
    }
    return (rc);
}

/**********************************************************************
 *
 * Function:	show_temp
 *
 * Description:	Display temperatures.
 *
 * Inputs:	err_log - TRUE to cterr. FALSE to printf.
 *		format - Display format of display_format_t in common.h
 *
 * Outputs:	PASSED/FAILED.
 *
 **********************************************************************
 */
int show_temp(int err_log, int format)
{
    int rc = FAILED;

    rc = show_temperature_all();
    return (rc);
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
uint32_t init_dimm_i2c_struct (n2g_i2c_dev_t *i2c_dev, uint32_t dimm_no) {
    uint32_t rc = FAILED;

    i2c_dev->rd_hd_size = 1;
    i2c_dev->wr_hd_size = 1;

    switch(dimm_no) {
    case MB_I2C_DIMM0:
        i2c_dev->bus_no = CPU_I2C0;
        i2c_dev->dev_addr = MB_I2C_ADDR_DIMM0;
        break;
    case MB_I2C_DIMM1:
        i2c_dev->bus_no = CPU_I2C0;
        i2c_dev->dev_addr = MB_I2C_ADDR_DIMM1;
        break;
    default:
        printf("%s: Unknown DIMM no. = %d.\n", __FUNCTION__, dimm_no);
        return (FAILED);
        break;
    }

    i2c_fd = get_i2c_fd(0);

    /* Set I2C device to SLAVE mode */
    if (i2c_fd <= 0) {
         cterr('f', 0, "/dev/i2c-0 is not opened correctly.");
         return (FAILED);
    } else {
        if ((rc = ioctl(i2c_fd, I2C_SLAVE, i2c_dev->dev_addr)) < 0) {
            cterr('f', 0, "%s at %s: unable to connect to device %#x. "
                          "rc = %#x", __FUNCTION__, __FILE__,
                          i2c_dev->dev_addr, rc);
            return (FAILED);
        } else {
            i2c_dev->fp = i2c_fd;
        }
    }
    return (PASSED);
}

/******************************************************************************
 *
 * Function   : init_tps536xx_i2c_struct
 * Description: To init i2c_dev structure.
 * Inputs     : dev_object_t *i2c_dev;
 *              uint32_t dimm_no.
 * Outputs    : PASSED/FAILED
 *
 ******************************************************************************/
uint32_t init_tps536xx_i2c_struct (n2g_i2c_dev_t *i2c_dev, uint32_t chip_no)
{
    uint32_t rc = FAILED;

    i2c_dev->bus_no = CPU_I2C0;

    /* default read header is 0, user need to
     * write address to chip then read data */
    i2c_dev->rd_hd_size = 0;
    i2c_dev->wr_hd_size = 1;

    switch(chip_no) {
    case MB_I2C_TPS53659_VCORE:
        i2c_dev->dev_addr = MB_I2C_ADDR_53659_VCORE; 
        break;
    case MB_I2C_TPS53622_1P05V:
        i2c_dev->dev_addr = MB_I2C_ADDR_53622_1P05V; 
        break;
    case MB_I2C_TPS53622_0P9VNN:  
        i2c_dev->dev_addr = MB_I2C_ADDR_53622_0P9VNN; 
        break;
    case MB_I2C_TPS53622_1V:    
        i2c_dev->dev_addr = MB_I2C_ADDR_53622_1V; 
        break;
    case MB_I2C_TPS53659_3P3V:
        i2c_dev->dev_addr = MB_I2C_ADDR_53659_3P3V; 
        break;
    default:
        printf("%s: Unknown TPS536xx no. = %d.\n", __FUNCTION__, chip_no);
        return (FAILED);
        break;
    }

    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf(" %-20s(0x%.2X)... ", "", (i2c_dev->dev_addr << 1));
    }

    i2c_fd = get_i2c_fd(0);

    /* Set I2C device to SLAVE mode */
    if (i2c_fd <= 0) {
         cterr('f', 0, "/dev/i2c-0 is not opened correctly.");
         return (FAILED);
    } else {
        if ((rc = ioctl(i2c_fd, I2C_SLAVE, i2c_dev->dev_addr)) < 0) {
            cterr('f', 0, "%s at %s: unable to connect to device %#x. "
                          "rc = %#x", __FUNCTION__, __FILE__,
                          i2c_dev->dev_addr, rc);
            return (FAILED);
        } else {
            i2c_dev->fp = i2c_fd;
        }
    }
    return (PASSED);
}

/*******************************************************************************
 *
 * Function   : reset_i2c_controller (int option)
 * Description: flush i2c device by writing to bit bang i2c register
 *              i2c register.
 * Inputs     : option , not used
 *
 * Outputs    : PASSED or FAILED
 *
 *******************************************************************************
 */
int reset_i2c_controller (int option)
{
    n2g_i2c_if_t i2c_if;
    unsigned long i2c_ctrl_addr = 0;

    i2c_if.i2c_ctrl = getdec_answer("\nEnter ctrl number", 12, 0, 16);

    /* reset I2C controller */
    i2c_ctrl_addr = get_platform_i2c_addr(i2c_if.i2c_ctrl);
    gfy_i2c_reset((goofy_i2c_t *)i2c_ctrl_addr);

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
int
read_i2c_reg (int yes_offset)
{
    n2g_i2c_if_t i2c_if;
    unsigned int addr, size, offset, rc, i;
    uchar d32[80];

    memset(&i2c_if, 0, sizeof(i2c_if));

    i2c_if.i2c_bus_type = IOFPGA_I2C;

    i2c_if.i2c_ctrl = getdec_answer("\nEnter ctrl number", I2C_CTRL_MAX, 0, I2C_CTRL_MAX );
    i2c_if.mux = getdec_answer("Enter mux number", 0, 0, I2C_MUX_MAX);
    addr = gethex_answer("Enter 7 bit slave address ", 0x7F, 0x0, 0xFF);
    i2c_if.i2c_dev = addr;

    if (yes_offset) {
        offset = gethex_answer("Enter reg offset", 0, 0, 0xFF);
        i2c_if.offset = offset;
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

    return PASSED;
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
    unsigned int addr, size, rc, i;
    int offset;
    char msg[80];
    uint8_t d8[32];

    memset(&i2c_if, 0, sizeof(i2c_if));

    i2c_if.i2c_bus_type = IOFPGA_I2C;
    
    i2c_if.i2c_ctrl = getdec_answer("\nEnter ctrl number", I2C_CTRL_MAX, 0, I2C_CTRL_MAX);

    i2c_if.mux = getdec_answer("Enter mux number", 0, 0, I2C_MUX_MAX);
    
    addr = gethex_answer("Enter 7 bit slave address (on mux 0)", 0x7F, 1, 0xFF);
    i2c_if.i2c_dev = addr;

    if (yes_offset) {
        offset = gethex_answer("Enter reg offset", 0x0, 0, 0xFF);
        i2c_if.offset = offset;
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

    return PASSED;
}

/*****************************************************************************
 *
 * Function   : fugazi_i2c_scan_test (int option)
 * Description: scan all i2c devices on overlord
 *
 * Inputs     : optoin , not used.
 *
 * Outputs    : PASSED or FAILED
 *
 ****************************************************************************
 */
int fugazi_i2c_scan_test (int option)
{
    n2g_i2c_if_t  i2c_if;
    goofy_i2c_t *i2c;
    uint32_t      reg_val = 0, ret_val = FAILED, fail_ctr = 0, psu_skip = 0, psu_pre = 0;
    uint32_t      ix, max_retry, status;
    uint8_t       now_test = 0, test_end = 0, test_num = 1;
    uchar         *tname = (uchar *)"I2C scan";
    char          errbuf[FUGAZI_BUF_SIZE];
    int err_code = 0, test_ctr;
    unsigned int board_rev;

    testname("%s", tname);

    /* Detect board type is p1b or later */
    get_platform_bd_rev(&board_rev);

    /* Setup end of test by calculate all FPGA I2C device number */
    test_end = (sizeof(fpga_i2c_dev)/sizeof(n2g_i2c_if_t));

    for (now_test = 0, test_num = 1; now_test < test_end; now_test++) {
        /* Get I2C device structure */
        memcpy(&i2c_if, &fpga_i2c_dev[now_test], sizeof(n2g_i2c_if_t));

        /* check PSU and POE PSU is available or not.
         */
        /* we check both present and stat because:
         * 1. the psu_stat might pass even though there is no psu,
         * 2. the psu is installed, but not connected with power line(stat)
         */
        if (fpga_i2c_dev[now_test].i2c_ctrl == I2C_CTRL_FOUR) {
            if ((strcmp(fpga_i2c_dev[now_test].dev_name,"PSU2 EEPROM") == 0) ||
                (strcmp(fpga_i2c_dev[now_test].dev_name,"PSU2 Microcontroller") == 0)) {
                psu_skip = check_psu_stat(FUGAZI_PSU2);
                psu_pre = check_psu_present(FUGAZI_PSU2);
                set_mux_channel(NULL, I2C1_MUX_PORT1_MASK, FUGAZI_PSU_I2C_MUX);
            } else {
                psu_skip = check_psu_stat(FUGAZI_PSU1);
                psu_pre = check_psu_present(FUGAZI_PSU1);
                set_mux_channel(NULL, I2C1_MUX_PORT0_MASK, FUGAZI_PSU_I2C_MUX);
            }

           /* device is not present or without power */
           if (psu_skip == TRUE) {
               if ((fpga_i2c_dev[now_test].mux == I2C_MUX_ZERO) ||
                   (fpga_i2c_dev[now_test].mux == I2C_MUX_ONE)) {
                   /* mux are for PSU  */
                   if (psu_pre != TRUE) {
                       /* psu is not present, skipped */
                       continue;
                   } else {
                       /* use fix number 0x5D(IIN_OC_WARN_LIMIT) for ucontroller */
                       i2c_if.offset = 0x5D;
                       i2c_if.size = 2;
                   }
               }
           } else {
               /* psu or poe stat is not good, skipped */
               continue;
           }
        }

        if (fpga_i2c_dev[now_test].i2c_ctrl == I2C_CTRL_FOUR ||
            fpga_i2c_dev[now_test].i2c_ctrl == I2C_CTRL_FIVE) {
            i2c = (goofy_i2c_t *)get_platform_i2c_addr(fpga_i2c_dev[now_test].i2c_ctrl); 
            gfy_i2c_reset((goofy_i2c_t *)i2c);
            max_retry = 3;  /* for PSU and PSU2, retry may be necessary */
        } else if ((strcmp(fpga_i2c_dev[now_test].dev_name,"SMLink 1") == 0) ||
                   (strcmp(fpga_i2c_dev[now_test].dev_name,"SMLink 0") == 0)) {
            /* CSCvt54911 : HW replay that PCH temperature on SMLINK1 on address 0x48, 
               the ME is also using this bus and sometimes there is a conflict on the bus.  
               We need to ignore this and HW mention the collision time was one sec */
            max_retry = 35; 
        } else {
            max_retry = 1;
        }

        i2c_if.buf = (char *)&reg_val;
        if ((NVRAM)->diagflag & D_VERBOSE) {
            printf("Now testing %2d: I2C ctrl %2d, Mux %d, %-29s(0x%.2X)... ",
                   now_test, i2c_if.i2c_ctrl, i2c_if.mux, i2c_if.dev_name,
                   (i2c_if.i2c_dev << 1));
        } else {
            prpass(testpass, "[%2d] %s, ", test_num, i2c_if.dev_name);
        }

        /* Read I2C device Register 0 */
        for (fail_ctr = ix = 0; ix < max_retry; ix++) {
            ret_val = n2g_i2c_read(&i2c_if);
            if (ret_val != PASSED) {
                fail_ctr++;
                err_code = i2c_err_no(&status);
                if (max_retry > 1)
                      printf("\nwarning: %s failed %s [i2c_status=%#x] during pass %d\n",
                      i2c_if.dev_name,
                      i2c_err_str(err_code), status, ix);
            } else {
                break;
            }
            msleep(30);
        }

        if (ret_val != PASSED) {
            err_code = i2c_err_no(&status);
            cterr('f', 0, "%s %s failed %s [i2c_status=%#x]",
                  tname, i2c_if.dev_name,
                  i2c_err_str(err_code), status );
            //return(FAILED);
        }

        if ((NVRAM)->diagflag & D_VERBOSE)
            printf("Done\n");

        test_num++;
        if (i2c_if.i2c_dev == MB_I2C_ADDR_PSU2_EEPROM ||
            i2c_if.i2c_dev == MB_I2C_ADDR_PSU2_MCNTRL ||
            i2c_if.i2c_dev == MB_I2C_ADDR_PSU1_EEPROM ||
            i2c_if.i2c_dev == MB_I2C_ADDR_PSU1_MCNTRL ||
            i2c_if.i2c_dev == MB_I2C_ADDR_PEM0_EEPROM ||
            i2c_if.i2c_dev == MB_I2C_ADDR_PEM0_MCNTRL ||
            i2c_if.i2c_dev == MB_I2C_ADDR_PEM1_EEPROM ||
            i2c_if.i2c_dev == MB_I2C_ADDR_PEM1_MCNTRL ) {

            msleep(30);
        }
    }

    /* TPS536XX, I2C devices on CPU I2C bus */
    for (test_ctr = 0; test_ctr < 5; test_ctr++) {
        if ((NVRAM)->diagflag & D_VERBOSE) {
            cterr_db_print("Fugazi I2C device: TPS536XX-%d I2C scan test.",
            test_ctr);
        } else {
            prpass(testpass, "[%2d] CPU I2C devices: TPS536XX-%d, ",
                             test_num, test_ctr);
        }
        test_num++;
        memset(errbuf, 0, sizeof(FUGAZI_BUF_SIZE));
        if (fugazi_tps536xx_i2c_scan_test(errbuf, test_ctr) != PASSED) {
            cterr('f', 0, "TPS536XX-%d I2C scan failed %s", test_ctr, errbuf);
            ret_val = FAILED;  /* test all the TPS536XX */
        }
        if ((NVRAM)->diagflag & D_VERBOSE)
            printf("Done\n");
    }


    prpass(testpass, NULL);
    return (ret_val);
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
    int i; 
    int size = (sizeof(fpga_i2c_dev)/sizeof(n2g_i2c_if_t));

    for (i = 0; i < size; i++) {
        if (fpga_i2c_dev[i].i2c_dev == addr) {
            /* to support different type of module/motherboard, etc...*/
            memcpy(&general_act2[0], &fpga_i2c_dev[i], sizeof(n2g_i2c_if_t));

            general_act2[0].i2c_ctrl = ctrl_no;
            return ((void *)(&general_act2[0]));
        }
    }

    /* check mdoules now */
    printf("Software has no support for device at addr %#x; ctrl_no = %#x\n]",
           addr, ctrl_no);

    return (void*)NULL;
}

/*******************************************************************************
 *
 * Function   : get_n2g_i2c_if
 * Description: ret
 *
 * Inputs     : optin ...not used
 *
 * Outputs    : PASSED or FAILED
 *
 *******************************************************************************
 */
void *get_n2g_i2c_if (uint8_t i2c, uint8_t mux, uint8_t addr)
{
    int i;
    int size = (sizeof(fpga_i2c_dev)/sizeof(n2g_i2c_if_t));
    for (i = 0; i < size; i++) {
        if (fpga_i2c_dev[i].i2c_dev == addr &&
            fpga_i2c_dev[i].mux == mux &&
            fpga_i2c_dev[i].i2c_ctrl == i2c) {
            return ((void *)(&fpga_i2c_dev[i]));
        }
    }
    printf("problem trying to get n2g_i2c_if; i2c_ctrl=%d, mux=%d, addr=%#x\n",
           i2c, mux, addr);
    fflush(stdout);
    return (void*)NULL;
}

/*******************************************************************************
 *
 * Function   : fpga_i2c_scan_addr (int option)
 * Description: scan i2c devices on fpga
 *
 * Inputs     : optin ...not used
 *
 * Outputs    : PASSED or FAILED
 *
 *******************************************************************************
 */
int fpga_i2c_scan_addr (int option)
{
    n2g_i2c_if_t  i2c_if;
    uint32_t      ret_val = FAILED, now_addr = 0, mask = 0, ctr = 0;
    uchar         d32[80];

    memset(&i2c_if, 0, sizeof(i2c_if));

    i2c_if.i2c_bus_type = IOFPGA_I2C;

    /* Out-of-rest all I2C controllers */
    mask = (FPGA_IN_I2C_0_RST | FPGA_IN_I2C_2_RST | FPGA_IN_I2C_4_RST |
            FPGA_IN_I2C_8_RST | FPGA_IN_I2C_10_RST | FPGA_IN_I2C_11_RST |
            FPGA_IN_I2C_12_RST | FPGA_IN_I2C_13_RST | FPGA_IN_I2C_14_RST |
            FPGA_IN_I2C_15_RST);
    unreset_platform_in_dev(mask);

    /* Get I2C controller & MUX number that you want to scan */
    i2c_if.i2c_ctrl = getdec_answer("\nEnter ctrl number", 12, 0, 20);
    i2c_if.mux = getdec_answer("Enter mux number", 0, 0, 4);

    i2c_if.offset = 0;
    i2c_if.size = 1;

    memset(d32, 0, sizeof(d32));
    i2c_if.buf = (char *)d32;

    printf("\nI2C Ctrl %d, Mux %d, available addr. =", i2c_if.i2c_ctrl, i2c_if.mux);

    for (now_addr = 0x00; now_addr <= 0x7F; now_addr++) {
        i2c_if.i2c_dev = now_addr;
        ret_val = FAILED;

        /* Read I2C device Register 0 */
        ret_val = n2g_i2c_read(&i2c_if);
        if (ret_val == PASSED) {
            printf(" %#x", now_addr);
            ctr++;
        }
    }

    if (ctr == 0) {
        printf(" None");
    }

    return (ret_val);
}

/*******************************************************************************
 *
 * Function   : fugazi_dimm_i2c_scan_test
 * Description: This function to check Overlord DIMM
 *              by reading register through I2C interface.
 * Inputs     : errbuf  - buffer to put error messages
 *              dimm_no - number of DIMM
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
int fugazi_dimm_i2c_scan_test (char *errbuf, uint32_t dimm_no)
{
    n2g_i2c_dev_t i2c_dev;
    n2g_i2c_if_t  i2c_if;
    uint32_t      reg_val = 0, rc = FAILED;

    /* Init device structure */
    if (init_dimm_i2c_struct(&i2c_dev, dimm_no) != PASSED) {
        sprintf(errbuf, "Init DIMM%d i2c_dev struct failed.", dimm_no);
        return (FAILED);
    }

    /* Get Registers value */
    /* Setup the interface struct for I2C API read */
    i2c_if.i2c_bus_type = i2c_dev.bus_no;
    i2c_if.i2c_dev = i2c_dev.dev_addr;

    /* Read the bytes from SPD DIMM */
    i2c_if.size = sizeof(uint32_t);	  /* Read 4 bytes at a time */
    i2c_if.offset = 0;
    i2c_if.buf = (char *)&reg_val;

    rc = api_mb_i2c_read(&i2c_dev, i2c_if.offset, i2c_if.size,
                         (char *)i2c_if.buf);
    if (rc != PASSED) {
        /* Read failed */
        if (rc == E_I2C_INV_ACK) {
            sprintf(errbuf, "%s: Dimm%d is not installed.\n",
                            __FUNCTION__, dimm_no);
        } else {
            sprintf(errbuf, "%s: I2C read failed(rc = %#.8x).\n",
                            __FUNCTION__, rc);
        }
        return(FAILED);
    }

    return (PASSED);
}

/*******************************************************************************
 *
 * Function   : fugazi_tps536xx_i2c_scan_test
 * Description: This function to check Curie TPS536XX
 *              by reading register through I2C interface.
 * Inputs     : errbuf  - buffer to put error messages
 *              chip_no - number of TPS536xx
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
int fugazi_tps536xx_i2c_scan_test (char *errbuf, uint32_t chip_no)
{
    n2g_i2c_dev_t i2c_dev;
    uint32_t      reg_val = 0;
    int rc = FAILED;

    /* Init device structure */
    if (init_tps536xx_i2c_struct(&i2c_dev, chip_no) != PASSED) {
        sprintf(errbuf, "Init TPS536XX%d i2c_dev struct failed.", chip_no);
        return (FAILED);
    }

    rc = i2c_smbus_read_byte(i2c_dev.fp, (__u8 *)&reg_val);

    if (rc < 0) {
       sprintf(errbuf, "i2c_addr 0x%.2X.", i2c_dev.dev_addr << 1);
       return (FAILED); 
    } else {
       return (PASSED); 
    }
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
static int write_i2c(int has_offset)
{
    write_i2c_reg(has_offset);
    return (PASSED); 
}

/**********************************************************************
 *
 * Function: platform_i2c_debug
 *
 * Description: set global i2c debug flag. this will cause i2c driver
 *              to spit out a lot of debugging messages
 *
 * Input : d -- not used
 *
 * Output: PASSED
 *
 **********************************************************************
 */
static int platform_i2c_debug (int d)
{
    i2c_debug ^= 1;
    printf("i2c_debug flag is %d\n", i2c_debug);
    return PASSED;
}

/**********************************************************************
 *
 * Function: display_mem_info
 *
 * Description: Display memory/ram information
 *
 * Input : NONE
 *
 * Output: PASSED
 *
 **********************************************************************
 */
int display_mem_info (void)
{
    char cmd[128];
    sprintf(cmd, "dmidecode -t memory");
    system(cmd);

    return (PASSED);
}
/* end of file */


/*-------------------------------------------------
 * $Log: platform_i2c.c,v $
 * Revision 1.4  2021/08/25 06:31:34  iachang
 * CSCvo59196-34 : I2C scan display include TPS536XX dev address
 *
 * Revision 1.3  2021/06/17 07:22:35  iachang
 * CSCvt54911 : Fixed I2C scan SMLink 0/1 fail issue.
 *
 * Revision 1.2  2021/06/02 08:22:35  iachang
 * CSCvo59196-33 : Merge Fugazi from ASR1K-main-branch to ISR main trunk
 *
 * Revision 1.1.8.2  2020/08/26 02:37:51  iachang
 * Merge Fugazi code into main trunk
 *
 * Revision 1.1.6.12  2020/08/04 08:37:06  iachang
 * Update Copyright to 2020
 *
 * Revision 1.1.6.11  2020/06/29 02:12:01  iachang
 * Correct the I2C scan test pass judgment
 *
 * Revision 1.1.6.10  2020/05/04 06:10:40  iachang
 * SMLink_1 retry up to a timeout of 1 second.
 *
 * Revision 1.1.6.9  2020/04/24 07:09:43  iachang
 *  CSCvt54911 : HW mention the collision time was one sec
 *
 * Revision 1.1.6.8  2020/03/25 03:15:37  iachang
 * CSCvt54911 : Fixed I2C scan SMLink 1 failed
 *
 * Revision 1.1.6.7  2019/08/06 05:59:32  letsai
 * Removed I2C scan for ACT2 due to FPGA/HW disconnect ACT2 device.
 *
 * Revision 1.1.6.6  2019/04/06 01:36:14  letsai
 * 1. Remove unused functions and files.
 * 2. Fix BCM54194 SFP External loopback test.
 * 3. Fix BCM54194 Register test.
 * 4. Fix Voltage Margin Utility.
 * 5. Add function to show system information.
 *
 * Revision 1.1.6.5  2019/04/03 01:24:45  iachang
 * Add SMLink 0/1 into I2C scan.
 *
 * Revision 1.1.6.4  2019/03/30 00:56:02  letsai
 * 1. Add USB console detect utility.
 * 2. Modify FAN utility.
 * 3. Remove unused items.
 * 4. Fix BCM54194 phy register test.
 *
 * Revision 1.1.6.3  2019/03/18 09:22:23  letsai
 * Fixed 1.Boot flash test 2.I2C scan test 3. FPGA interrupt test
 *
 * Revision 1.1.6.2  2019/03/14 03:48:36  letsai
 * Initial check in.
 *
 *
 *
 *
 * $Endlog$
 */

