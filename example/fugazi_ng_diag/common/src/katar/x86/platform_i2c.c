/* $Id: platform_i2c.c,v 1.2 2019/06/14 05:24:50 mikech2 Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/katar/x86/platform_i2c.c,v $
 *-----------------------------------------------------------------------------
 * katar_platform_i2c.c - Overlord I2C utility menu. P40t is header for SM1 and P38t is header
 * for SM2
 *
 * Sept. 2007, Simon Yen
 *
 * Copyright (c) 2014-2019 by Cisco Systems, Inc.
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
#include "signals.h"
#include "error.h"
#include "nvmonvars.h"
#include "mon_plat_defs.h"
#include "cross_platform.h"
#include "pca9545a.h"
#include "n2g_api_rc.h"
#include "queryflags.h"
#include "i2c_dev.h"
#include "i2c_api.h"
#include "platform_i2c.h"
#include "platform_env.h"
#include "platform_i2c_usb.h"
#include "proto.h"
#include "platform_psu.h"
#include "byteswap.h"
#include "i2c_address.h"
#include "platform_i2c.h"
#include "platform_slot.h"
#include "mb_tests.h"
#include "platform_pci.h"
#include "platform_temp_sensor.h"
#include "platform_poe.h"

/*
 *  Externs
 */
extern jmp_buf *monjmpptr;
extern int get_i2c_fd(int);

/* for utilities submenu. */
extern int build_ts_menu(int);
extern uint32_t katar_n2g_i2c_read(n2g_i2c_if_t *);
extern uint32_t katar_n2g_i2c_write(n2g_i2c_if_t *);
extern uint32_t ich_i2c_init(uint8_t i2c_ctl, char i2c_speed);

/*
 * Functional prototype
 */
int show_temp(int, int);
extern int show_margins(int);

static int write_i2c(int);
static int read_i2c(int);
static int platform_i2c_debug(int);

int katar_pse_scan_test (char *errbuf);
int katar_temp_scan_test (char *errbuf);
int display_mem_info(void);

int read_i2c_reg_aikido_1byte (uint8_t *);
int write_i2c_reg_aikido (uint8_t *, uint32_t);
int read_fpga_i2c (int);
int write_fpga_i2c (int);


/*
 *  Globals  
 */
#define MB_I2C_ADDR_AIKIDO_ACT2  0x77
unsigned char i2c_debug = 0;

static int i2c_fd = -1;

static n2g_i2c_if_t fpga_i2c_dev[] = {
    /*  
     * I2C FPGA
     */
    {   
        .dev_name = "ACT2",
        .offset = -1,              /* need to be -1 to tell driver not to use offset !!! */
        .i2c_bus_type = IOFPGA_I2C,
        .i2c_dev = MB_I2C_ADDR_AIKIDO_ACT2,
        .i2c_ctrl = I2C_CTRL_ZERO,
        .sub_addr_len = 0,
        .size = sizeof(uint16_t),
        .mux = I2C_MUX_ZERO,
        .buf = NULL,
    }, 
};


/*
 * I2C Utility Menu.
 */
static submenu_xtable_t i2c_menu_table[] = {
    {"Display Memory Info",            (PFT)display_mem_info,  TRUE,
        0,                             (PFT)0,                          0,
      (PFT)0,                          0},
    {"Temperature Sensor",             (PFT)build_ts_menu,              TRUE,
        0,                             (PFT)0,                          0,
      (PFT)0,                          0},
#ifdef ENABLE_POE_MODULE
    {"POE Utility",                        (PFT)poe_utils,             TRUE,
        0,                             (PFT)0,                          0,
      (PFT)0,                          0},
#endif
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
    {"FPGA I2C read",                  (PFT)read_fpga_i2c,              TRUE,
        0,                             (PFT)0,                          0,
      (PFT)0,                          0},
    {"FPGA I2C write",                 (PFT)write_fpga_i2c,             TRUE,
        0,                             (PFT)0,                          0,
      (PFT)0,                          0},
    {"Toggle i2c debug flag",          (PFT)platform_i2c_debug,         TRUE,
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
  "I2C Utility Menu",               /* title */
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
void
build_i2c_menu (void)
{

    build_primary_submenu(i2c_menu_table, I2C_MENU_TABLE_SIZE,
                          "I2C Utility Menu", &i2cdiagp);
    build_secondary_submenu(i2c_menu_table, I2C_MENU_TABLE_SIZE,
                            i2c_menu_secondary_items);
    menu(&i2cdiag, i2c_menu_secondary_items, 0);
}

/**********************************************************************
 *
 * Function:    platform_cpu_i2c_init
 *
 * Description: Initialize CPU I2C bus controllers.
 *
 * Inputs:      None.
 *
 * Outputs:     PASSED/FAILED.
 *
 **********************************************************************
 */
int
platform_cpu_i2c_init(void)
{
    uint32_t rc = PASSED;
    int i;
    n2g_i2c_if_t i2c_if;

    /*currently not used??? */
    for (i = CPU_I2C1; i <= CPU_I2C1; i++) {
        i2c_if.i2c_bus_type = i;        /* Setup the bus controller number */
        /* CPU I2C controller uses 100 KHz */
        i2c_if.i2c_speed = N2G_I2C_100KHZ;
		rc = ich_i2c_init(i2c_if.i2c_bus_type, i2c_if.i2c_speed);
    }
    return (rc);
}

/**********************************************************************
 *
 * Function:    show_temp
 *
 * Description: Display temperatures.
 *
 * Inputs:      err_log - TRUE to cterr. FALSE to printf.
 *              format - Display format of display_format_t in common.h
 *
 * Outputs:     PASSED/FAILED.
 *
 **********************************************************************
 */
int
show_temp(int err_log, int format)
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

    i2c_dev->bus_no = CPU_I2C0;
    i2c_dev->rd_hd_size = 1;
    i2c_dev->wr_hd_size = 1;

    switch(dimm_no) {
    case MB_I2C_DIMM0:
        i2c_dev->dev_addr = MB_I2C_ADDR_DIMM0;
        break;
    case MB_I2C_DIMM1:
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
    n2g_i2c_dev_t i2c_dev;
    n2g_i2c_if_t  i2c_if;
    unsigned int addr, size, offset, rc, hd_size, i;
    //uchar d32[80];
    unsigned char d32[256];
    char          errbuf[256];

    addr = gethex_answer("Enter 7 bit slave address ", 0x7F, 0x0, 0xFF);
    i2c_if.i2c_dev = addr;

    if (yes_offset) {
        offset = gethex_answer("Enter reg offset", 0, 0, 0xFF);
        i2c_if.offset = offset;
    }
    else {
        i2c_if.offset = -1;
    }

    size = gethex_answer("Enter length (in bytes)", 2, 1, 256);
    i2c_if.size = size;

    hd_size = gethex_answer("Enter read hd_size length (in bytes)", 0, 0, 0xFF);

    memset(d32, 0, sizeof(d32));
    i2c_if.buf = (char *)d32;

    /* Init device structure */
    i2c_dev.bus_no = CPU_I2C0;
    i2c_dev.rd_hd_size = hd_size;
    i2c_dev.wr_hd_size = 0;
    i2c_dev.dev_addr = addr;

    i2c_fd = get_i2c_fd(0);

    /* Set I2C device to SLAVE mode */
    if (i2c_fd <= 0) {
         cterr('f', 0, "/dev/i2c-0 is not opened correctly.");
         return (FAILED);
    } else {
        if ((rc = ioctl(i2c_fd, I2C_SLAVE, i2c_dev.dev_addr)) < 0) {
            cterr('f', 0, "%s at %s: unable to connect to device %#x. "
                          "rc = %#x", __FUNCTION__, __FILE__,
                          i2c_dev.dev_addr, rc);
            return (FAILED);
        } else {
            i2c_dev.fp = i2c_fd;
        }
    }


    /* Get Registers value */
    /* Setup the interface struct for I2C API read */
    i2c_if.i2c_bus_type = i2c_dev.bus_no;
    i2c_if.i2c_dev = i2c_dev.dev_addr;

    rc = api_mb_i2c_read(&i2c_dev, i2c_if.offset, i2c_if.size,
                         (char *)i2c_if.buf);
    if (rc != PASSED) {
        /* Read failed */
        if (rc == E_I2C_INV_ACK) {
            sprintf(errbuf, "%s: I2C device is not installed.\n",
                            __FUNCTION__);
        } else {
            sprintf(errbuf, "%s: I2C read failed(rc = %#.8x).\n",
                            __FUNCTION__, rc);
        }
        return(FAILED);
    }

    printf("\n");
    for (i = 0; i < size ; i++) {
        if (!(i % 16)) {
            printf("\n  0x%02x:", i); }
        printf("0x%02x ", d32[i]);
    }

    return (PASSED);
}


int read_i2c_reg_aikido_1byte (uint8_t * read_buffer_1byte)
{
    int rc;
	uint8	buff = -1;
    char          errbuf[256];

    i2c_fd = get_i2c_fd(0);
	/* Read a byte with SMBus */
    rc = i2c_smbus_read_byte(i2c_fd, &buff);
	if (rc < 0)
	{
		/* Read failed */
        if (rc == E_I2C_INV_ACK) {
            sprintf(errbuf, "%s: I2C device is not installed.\n", __FUNCTION__);
        } else {
            sprintf(errbuf, "%s: I2C read failed(rc = %#.8x).\n", __FUNCTION__, rc);
        }
        return(FAILED);
	}
    read_buffer_1byte[0] = buff;

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
int
write_i2c_reg (int yes_offset)
{
    n2g_i2c_dev_t i2c_dev;
    n2g_i2c_if_t  i2c_if;
    unsigned int addr, size, offset, rc, i, hd_size;
    char msg[80];
    uint8_t d8[256];
    char          errbuf[256];

    memset(&i2c_if, 0, sizeof(i2c_if));

    addr = gethex_answer("Enter 7 bit slave address ", 0x7F, 0x0, 0xFF);
    i2c_if.i2c_dev = addr;

    if (yes_offset) {
        offset = gethex_answer("Enter reg offset", 0, 0, 0xFF);
        i2c_if.offset = offset;
    }
    else {
        i2c_if.offset = -1;
    }

    hd_size = gethex_answer("Enter write hd_size length (in bytes)", 0, 0, 0xFF);

    size = gethex_answer("Enter length (in bytes)", 2, 1, 256);
    i2c_if.size = size;

    for (i = 0; i < size; i++) {
        sprintf(msg, "Enter bytes %d", i);
        d8[i] = gethex_answer(msg, 0x0, 0, 0xFF);
    }
    i2c_if.buf = (char *)&d8[0];

    /* Init device structure */
    i2c_dev.bus_no = CPU_I2C0;
    i2c_dev.rd_hd_size = 0;
    i2c_dev.wr_hd_size = hd_size;
    i2c_dev.dev_addr = addr;

    i2c_fd = get_i2c_fd(0);

    /* Set I2C device to SLAVE mode */
    if (i2c_fd <= 0) {
         cterr('f', 0, "/dev/i2c-0 is not opened correctly.");
         return (FAILED);
    } else {
        if ((rc = ioctl(i2c_fd, I2C_SLAVE, i2c_dev.dev_addr)) < 0) {
            cterr('f', 0, "%s at %s: unable to connect to device %#x. "
                          "rc = %#x", __FUNCTION__, __FILE__,
                          i2c_dev.dev_addr, rc);
            return (FAILED);
        } else {
            i2c_dev.fp = i2c_fd;
        }
    }


    /* Get Registers value */
    /* Setup the interface struct for I2C API read */
    i2c_if.i2c_bus_type = i2c_dev.bus_no;
    i2c_if.i2c_dev = i2c_dev.dev_addr;


    if (diagflag_xram & D_DEBUG_OPTIONS) {
        printf("offset = %u\n", i2c_dev.bus_no);
        printf("dev_addr = %u\n", i2c_dev.dev_addr);
        printf("rd_hd_size = %u\n", i2c_dev.rd_hd_size);
        printf("wr_hd_size = %u\n", i2c_dev.wr_hd_size);
        printf("fp = %d\n", i2c_dev.fp);
        printf("offset = %u\n", i2c_if.offset);
        printf("size = %u\n", i2c_if.size);
        printf("buf = [");
        int ix;
        for (ix = 0; ix < i2c_if.size; ix++) {
            printf(" %02x", i2c_if.buf[ix]);
        }
        printf("]\n");
     }


    //rc = api_mb_i2c_write(&i2c_dev, i2c_if.offset, i2c_if.size, (char *)i2c_if.buf);
    rc = api_mb_i2c_write(&i2c_dev, i2c_if.offset, i2c_if.size, i2c_if.buf);
    if (rc != PASSED) {
        /* Read failed */
        if (rc == E_I2C_INV_ACK) {
            sprintf(errbuf, "%s: I2C device is not installed.\n",
                            __FUNCTION__);
        } else {
            sprintf(errbuf, "%s: I2C read failed(rc = %#.8x).\n",
                            __FUNCTION__, rc);
        }
        return(FAILED);
    }

    return (PASSED);
}


int write_i2c_reg_aikido (uint8_t * send_buffer, uint32_t length)
{
    n2g_i2c_dev_t i2c_dev;
    n2g_i2c_if_t  i2c_if;
    unsigned int  hd_size;
    char          errbuf[256];

    memset(&i2c_if, 0, sizeof(i2c_if));

    i2c_if.i2c_dev = MB_I2C_ADDR_AIKIDO_ACT2;

    i2c_if.offset = -1;

    hd_size = 0;

    i2c_if.size = length;

    i2c_if.buf = (char *) send_buffer;

    /* Init device structure */
    i2c_dev.bus_no = CPU_I2C0;
    i2c_dev.rd_hd_size = 0;
    i2c_dev.wr_hd_size = hd_size;
    //i2c_dev.dev_addr = addr;
    i2c_dev.dev_addr = i2c_if.i2c_dev;

    i2c_fd = get_i2c_fd(0);

    /* Set I2C device to SLAVE mode */
    /* already done at katar_diagact2_lib_initialize; no need to do it again
    if (i2c_fd <= 0) { 
         cterr('f', 0, "/dev/i2c-0 is not opened correctly.");
         return (FAILED);
    } else {
        if ((rc = ioctl(i2c_fd, I2C_SLAVE, i2c_dev.dev_addr)) < 0) { 
            cterr('f', 0, "%s at %s: unable to connect to device %#x. "
                          "rc = %#x", __FUNCTION__, __FILE__,
                          i2c_dev.dev_addr, rc); 
            return (FAILED);
        } else {
            i2c_dev.fp = i2c_fd;
        }    
    }
    */    
    i2c_dev.fp = i2c_fd;


    /* Get Registers value */
    /* Setup the interface struct for I2C API read */
    i2c_if.i2c_bus_type = i2c_dev.bus_no;
    i2c_if.i2c_dev = i2c_dev.dev_addr;


        //printf("Inside tam_lib_platform_write. \n");
        //printf("buf = [");
        //printf("write buf = [");

    /*
    if (diagflag_xram & D_DEBUG_OPTIONS) {
        printf("offset = %u\n", i2c_dev.bus_no);
        printf("dev_addr = %u\n", i2c_dev.dev_addr);
        printf("rd_hd_size = %u\n", i2c_dev.rd_hd_size);
        printf("wr_hd_size = %u\n", i2c_dev.wr_hd_size);
        printf("fp = %d\n", i2c_dev.fp);
        printf("offset = %u\n", i2c_if.offset);
        printf("size = %u\n", i2c_if.size);
    }
    */

//    rc = api_mb_i2c_write(&i2c_dev, i2c_if.offset, i2c_if.size, i2c_if.buf);
    __s32 res;
    res = i2c_smbus_write_i2c_block_data(i2c_dev.fp, i2c_if.buf[0], (i2c_if.size - 1), (__u8 *)&i2c_if.buf[1]);
    //if (rc != PASSED) {
    if (res < 0) {
        /* Read failed */
        //if (rc == E_I2C_INV_ACK) {
        //    sprintf(errbuf, "%s: I2C device is not installed.\n",
        //                    __FUNCTION__);
        //} else {
        //    sprintf(errbuf, "%s: I2C read failed(rc = %#.8x).\n",
        //                    __FUNCTION__, rc); 
        //}    
        sprintf(errbuf, "%s: I2C read failed(rc = %#.8x).\n",__FUNCTION__, res); 
        return(FAILED);
    }    

    return (PASSED);
}


/*******************************************************************************
 *
 * Function   : read_fpga_i2c ()
 * Description: generic fpga i2c read funtcion, allowing user to manually read from
 *              fpga i2c register.
 * Inputs     : option.  
 * Outputs    : PASSED or FAILED
 *  
 *******************************************************************************
 */  
int read_fpga_i2c (int option)
{
    n2g_i2c_if_t i2c_if;
    unsigned int addr, rc, ix, bus, ctrl, mux;
    //int offset;
    uint16_t size;
    uchar d32[80];

    memset(&i2c_if, 0, sizeof(i2c_if));

    //bus = getdec_answer("\nEnter i2c bus number(I2C0 - 0, I2C1 - 1, IOFPGA - 3)", 0, 0, 3);
    bus = getdec_answer("\nEnter i2c bus number", 2, 0, 2);
    i2c_if.i2c_bus_type = bus;

    ctrl = getdec_answer("\nEnter i2c control", 0, 0, 3);
    //i2c_if.i2c_ctrl = i2c_if.i2c_bus_type;
    i2c_if.i2c_ctrl = ctrl;

    mux = getdec_answer("\nEnter i2c mux", 0, 0, 3);
    //i2c_if.mux = 0;
    i2c_if.mux = mux;

    //if ((i2c_if.i2c_bus_type == CPU_I2C0) || (i2c_if.i2c_bus_type == CPU_I2C1)) {
    //    addr =
    //        gethex_answer("Enter 7 bit slave address (DIMM - 0x50, Thermal - 0x1C)",
    //                      0x0, 1, 0xFF);
    //    i2c_if.i2c_dev = addr;
    //} else {
    //    addr =
    //        gethex_answer("Enter 7 bit slave address (ACT2 - 0x70)", 0x0, 1, 0xFF);
    addr = gethex_answer("Enter 7 bit slave address", 0x77, 1, 0xFF);
    i2c_if.i2c_dev = addr;

    //}
    //if (yes_offset) {
    //    offset = gethex_answer("Enter reg offset", 0, 0, 0xFF);
    //    i2c_if.offset = offset;
    //} else if (i2c_if.i2c_bus_type == IOFPGA_I2C) {
        i2c_if.offset = -1;
    //} else {
    //    i2c_if.offset = 0;
    //}

    size = gethex_answer("Enter length you want to read(in bytes)", 0x0, 1, 10);
    i2c_if.size = size;

    memset(d32, 0, sizeof(d32));
    i2c_if.buf = (char *) d32;
    rc = katar_n2g_i2c_read(&i2c_if);
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
 * Function   : write_fpga_i2c (int option)
 * Description: generic fpga i2c write funtcion, allowing user to manually write to
 *              fpga i2c register.
 * Inputs     : option
 * Outputs    : PASSED or FAILED
 *
 *******************************************************************************
 */
int write_fpga_i2c (int option)
{
    n2g_i2c_if_t i2c_if;
    unsigned int addr, size, rc, ix, bus, ctrl, mux;
    //int offset;
    char msg[80];
    uint8_t d8[32];

    memset(&i2c_if, 0, sizeof(i2c_if));

    //bus = getdec_answer("\nEnter i2c bus number(I2C0 - 0, I2C1 - 1, IOFPGA - 3)", 0, 0, 3);
    bus = getdec_answer("\nEnter i2c bus number", 2, 0, 2);
    i2c_if.i2c_bus_type = bus;

    ctrl = getdec_answer("\nEnter i2c control", 0, 0, 3);
    //i2c_if.i2c_ctrl = i2c_if.i2c_bus_type;
    i2c_if.i2c_ctrl = ctrl;

    mux = getdec_answer("\nEnter i2c mux", 0, 0, 3);
    //i2c_if.mux = 0;
    i2c_if.mux = mux;

    //if ((i2c_if.i2c_bus_type == CPU_I2C0) || (i2c_if.i2c_bus_type == CPU_I2C1)) {
    //    addr =
    //        gethex_answer("Enter 7 bit slave address (DIMM - 0x50, Thermal - 0x1C)",
    //                      0x0, 1, 0xFF);
    //    i2c_if.i2c_dev = addr;
    //} else {
    //    addr =
    //        gethex_answer("Enter 7 bit slave address (ACT2 - 0x70)", 0x0, 1, 0xFF);
    addr = gethex_answer("Enter 7 bit slave address", 0x77, 1, 0xFF);
    i2c_if.i2c_dev = addr;

    //}
    //if (yes_offset) {
    //    offset = gethex_answer("Enter reg offset", 0, 0, 0xFF);
    //    i2c_if.offset = offset;
    //} else if (i2c_if.i2c_bus_type == IOFPGA_I2C) {
        i2c_if.offset = -1;
    //} else {
    //    i2c_if.offset = 0;
    //}

    size = gethex_answer("Enter length you want to write", 2, 1, 20);
    i2c_if.size = size;

    for (ix = 0; ix < size; ix++) {
        sprintf(msg, "Enter bytes %d", ix);
        d8[ix] = gethex_answer(msg, 0x0, 0, 0xFF);
    }

    i2c_if.buf = (char *) &d8[0];

    rc = katar_n2g_i2c_write(&i2c_if);
    if (rc != PASSED) {
        cterr('f', 0, "%s at %s: unable to write i2c %#x. "
              "rc = %#x", __FUNCTION__, __FILE__, i2c_if.i2c_dev, rc);
        return (FAILED);
    }

    return (PASSED);
}


/*****************************************************************************
 *
 * Function   : katar_x86_i2c_scan_test (int option)
 * Description: scan all i2c devices
 *
 * Inputs     : optoin , not used.
 *
 * Outputs    : PASSED or FAILED
 *
 ****************************************************************************
 */
int katar_x86_i2c_scan_test (int option)
{
    uint32_t      fail_ctr = 0;
    char          errbuf[256];
	char *tname = "I2C scan";

    testname("%s", tname);   

#ifdef ENABLE_POE_MODULE
    if (katar_pse_scan_test(errbuf) != PASSED) {        
        fail_ctr++;
        cterr('f', 0, "PSE I2C scan failed %s",errbuf);
    }
#endif
    if (katar_temp_scan_test(errbuf) != PASSED) {        
        fail_ctr++;
        cterr('f', 0, "temp sensor I2C scan failed %s ",errbuf);
    }

    if ( fail_ctr != 0 )
        return FAILED;
    else
    {
        prpass(testpass, NULL);
        return PASSED;
    }
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
void *
platform_i2c_get_quack (uint8_t addr, uint8_t ctrl_no)
{
	int i;
    int size = (sizeof(fpga_i2c_dev)/sizeof(n2g_i2c_if_t));
   
    for (i = 0; i < size; i++) {
        if (fpga_i2c_dev[i].i2c_dev == addr) {
            /* to support different type of module/motherboard, etc...*/
            fpga_i2c_dev[i].i2c_ctrl = ctrl_no;
            return ((void *)(&fpga_i2c_dev[i]));
        }
    }

    /* check mdoules now */
    printf("Software has no support for device at addr %#x; ctrl_no = %#x\n]",
           addr, ctrl_no);

    return (void*)NULL;
}

/*******************************************************************************
 *
 * Function   : katar_temp_scan_test
 * Description: This function to check temperature
 *              by reading register through I2C interface.
 * Inputs     : errbuf  - buffer to put error messages
 *              dimm_no - number of DIMM
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
extern uint32_t init_temp_i2c_struct (n2g_i2c_dev_t *i2c_dev, uint32_t ts_id) ;

int katar_temp_scan_test (char *errbuf)
{
    n2g_i2c_dev_t i2c_dev;
    n2g_i2c_if_t  i2c_if;
    uint32_t      reg_val = 0, rc = FAILED;
    uint32_t i;

    for ( i = TS_BEZEL_SIDE0;  i <= TS_BEZEL_SIDE1 ; i++)
    {
            /* Init device structure */
            if (init_temp_i2c_struct(&i2c_dev, i) != PASSED) {
                sprintf(errbuf, "Init PSE i2c_dev struct failed.");
                return (FAILED);
            }

            /* Get Registers value */
            /* Setup the interface struct for I2C API read */
            i2c_if.i2c_bus_type = i2c_dev.bus_no;
            i2c_if.i2c_dev = i2c_dev.dev_addr;

            /* Read the bytes from SPD DIMM */
            i2c_if.size = sizeof(uint16_t);       /* Read 2 bytes at a time */
            i2c_if.offset = 0;
            i2c_if.buf = (char *)&reg_val;

            rc = api_mb_i2c_read(&i2c_dev, i2c_if.offset, i2c_if.size,
                                 (char *)i2c_if.buf);
            if (rc != PASSED) {
                /* Read failed */
                if (rc == E_I2C_INV_ACK) {
                    sprintf(errbuf, "%s: Temp side%d is not installed.\n",
                                    __FUNCTION__, i);
                } else {
                    sprintf(errbuf, "%s: Temp side%d I2C read failed(rc = %#.8x).\n",
                                    __FUNCTION__, i, rc);
                }
                return(FAILED);
            }
    }

    return (PASSED);
}


/*******************************************************************************
 *
 * Function   : katar_pse_scan_test
 * Description: This function to check PSE
 *              by reading register through I2C interface.
 * Inputs     : errbuf  - buffer to put error messages
 *              dimm_no - number of DIMM
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
extern uint32_t init_poe_i2c_struct (n2g_i2c_dev_t *i2c_dev) ;

int katar_pse_scan_test (char *errbuf)
{
#ifdef ENABLE_POE_MODULE
    n2g_i2c_dev_t i2c_dev;
    n2g_i2c_if_t  i2c_if;
    uint32_t      reg_val = 0, rc = FAILED;

    /* Init device structure */
    if (init_poe_i2c_struct(&i2c_dev) != PASSED) {
        sprintf(errbuf, "Init PSE i2c_dev struct failed.");
        return (FAILED);
    }

    /* Get Registers value */
    /* Setup the interface struct for I2C API read */
    i2c_if.i2c_bus_type = i2c_dev.bus_no;
    i2c_if.i2c_dev = i2c_dev.dev_addr;

    /* Read the bytes from SPD DIMM */
    i2c_if.size = sizeof(uint8_t);       /* Read 1 bytes at a time */
    i2c_if.offset = 0;
    i2c_if.buf = (char *)&reg_val;

    rc = api_mb_i2c_read(&i2c_dev, i2c_if.offset, i2c_if.size,
                         (char *)i2c_if.buf);
    if (rc != PASSED) {
        /* Read failed */
        if (rc == E_I2C_INV_ACK) {
            sprintf(errbuf, "%s: PSE is not installed.\n",
                            __FUNCTION__);
        } else {
            sprintf(errbuf, "%s: I2C read failed(rc = %#.8x).\n",
                            __FUNCTION__, rc);
        }
        return(FAILED);
    }
#endif
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
static
int read_i2c (int has_offset)
{
    read_i2c_reg(has_offset);
    return PASSED;
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
static
int write_i2c(int has_offset)
{
    write_i2c_reg(has_offset);
    return PASSED;
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
static
int platform_i2c_debug (int d)
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

/*
 *------------------------------------------------------------------
 * $Log: platform_i2c.c,v $
 * Revision 1.2  2019/06/14 05:24:50  mikech2
 * Collapse katar-branch00 to Main Trunk
 *
 * Revision 1.1.2.5  2019/04/30 06:06:59  mikech2
 * Code cleanup
 *
 * Revision 1.1.2.4  2019/04/26 01:17:34  mikech2
 * clean up Makefile
 *
 * Revision 1.1.2.3  2019/02/12 08:06:29  mikech2
 * rename katar_*.h files
 *
 * Revision 1.1.2.2  2019/01/29 08:02:05  mikech2
 * remove POE test for katar P2 build
 *
 * Revision 1.1.2.1  2019/01/29 01:54:21  mikech2
 * rename katar_* files
 *
 * Revision 1.1.2.10  2018/12/27 09:01:28  peteteng
 * Clean cookie warning log
 *
 * Revision 1.1.2.9  2018/12/27 03:49:00  mikech2
 * Modify prpass usage
 *
 * Revision 1.1.2.8  2018/12/20 09:10:57  peteteng
 * Add FPGA I2C read/write/scan/reset util
 *
 * Revision 1.1.2.7  2018/12/14 02:06:04  mikech2
 * Fix Akido FPGA SMBus read issue
 *
 * Revision 1.1.2.6  2018/12/06 08:32:25  mikech2
 * Fine-tune Aikido I2C r/w and fix Aikido update FW utility
 *
 * Revision 1.1.2.5  2018/12/01 10:39:01  peteteng
 * Speed up Aikido cookie
 *
 * Revision 1.1.2.4  2018/11/29 03:19:58  peteteng
 * Fix Aikido cookie - read one byte
 *
 * Revision 1.1.2.3  2018/11/23 03:45:34  peteteng
 * Fix I2C Util write multiple bytes issue
 *
 * Revision 1.1.2.2  2018/11/17 11:09:49  peteteng
 * Fix Aikido cookie issue
 *
 * Revision 1.1.2.1  2018/10/22 08:02:28  mikech2
 * Move project folder to common/src/katar/x86
 *
 * Revision 1.1.2.5  2018/10/08 03:36:17  mikech2
 * Modify pcie scan for different AQC100 FW
 *
 * Revision 1.1.2.4  2018/09/20 06:44:59  peteteng
 * Add load/read 256-byte cookie on EEPROM in cookie utility
 *
 * Revision 1.1.2.3  2018/09/04 06:09:08  mikech2
 * Fix I2C util , realtek port & get_pcie_cap_struct_ptr return error issue
 *
 * Revision 1.1.2.2  2018/08/27 08:28:47  mikech2
 * Fix I2C & pcie scan test
 *
 * Revision 1.1.2.1  2018/07/17 11:30:22  benlu
 * For i2c scan test
 *
 *
 *
 *------------------------------------------------------------------
 * $Endlog$
 */


