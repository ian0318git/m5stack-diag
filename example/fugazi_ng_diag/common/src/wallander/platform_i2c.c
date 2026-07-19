/* $Id: platform_i2c.c,v 1.1 2015/02/26 07:18:29 xiaoyizh Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/wallander/platform_i2c.c,v $
 *-----------------------------------------------------------------------------
 * platform_i2c.c - Wallander I2C library
 *
 * Apr. 2014 , Xiaoying Zhang
 * Copyright (c) 2015 by Cisco Systems, Inc.
 * All rights reserved.
 *-----------------------------------------------------------------------------
 */
#include "endians.h"
#include <setjmp.h>
#include "error.h"
#include "common.h"
#include "types.h"
#include "defs.h"
#include "menu.h"
#include "signals.h"
#include "queryflags.h"
#include "platform_sfp_cookie.h"
#include "i2c_dev.h"
#include "i2c_api.h"
#include "platform_i2c.h"
#include "dev_pca9545.h"
#include "pca9545a.h"
#include "proto.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "nvsysvars.h"
// #include "nvmonvars.h"
#include "diag_common_drv.h"

#include "cvmx-twsi.h"

/*
 *  Externs
 */
extern int32_t cavium_i2c_fd0;
extern int32_t cavium_i2c_fd1;
extern int get_board_id(void);
/******************************************************************************
 *  Global Variable
 *****************************************************************************/
char i2c_err_buf[I2C_ERR_BUF_SIZE];

/*
 * Control Register Bit Table
 */
static dev_pca_desc_t mux_bit_desc[] = {
    {"SFP0", "Gated", "Unconnected", MUX9545_PORT0_MASK},
    {"SFP1", "Gated", "Unconnected", MUX9545_PORT1_MASK},
    {"SFP2", "Gated", "Unconnected", MUX9545_PORT2_MASK},
    {"SFP3", "Gated", "Unconnected", MUX9545_PORT3_MASK},
    {0, 0, 0, 0},
};

/******************************************************************************
 * Functional prototype
 *****************************************************************************/
int read_i2c_reg(n2g_i2c_dev_t *, uchar *, uint, uchar);
int write_i2c_reg(n2g_i2c_dev_t *, uchar *, uint, uchar);
uint32_t open_i2c(n2g_i2c_dev_t *, uint, uint8_t);

int cavium_i2c_scan_test(void);
int pca9557_power_margin_ctrl(void);
int pca9557_power_margin_status(void);
/*
 * Functional prototype
 */
int check_i2c_fd(n2g_i2c_dev_t *);

/*
 *  Globals  
 */

/******************************************************************************
 *
 * Function    : open_i2c
 * Description : Open the Cavium I2C bus
 * Input       : i2c_dev  - pointer to the I2C device
 *                  i2c_slave_addr - I2C slave physical address
 *
 * Output: PASSED/FAILED
 *
 *****************************************************************************/
uint32_t open_i2c (n2g_i2c_dev_t *i2c_dev, uint i2c_slave_addr, uint8_t i2c_bus)
{
    uint32_t rc = (PASSED);
    int32_t cavium_i2c_fd;
    /* Setup I2C device data structure */
    i2c_dev->bus_no = i2c_bus;
    i2c_dev->dev_addr = i2c_slave_addr;
    i2c_dev->rd_hd_size = 1;
    i2c_dev->wr_hd_size = 1;

    /* Set I2C device to SLAVE */
    if ((cavium_i2c_fd = (i2c_bus == CPU_I2C0)? cavium_i2c_fd0 : cavium_i2c_fd1)) {
        if (cavium_i2c_fd <= 0) {
            cterr('f', 0, "/dev/i2c-octeon.%d is not opened correctly.", i2c_bus);
            return (FAILED);
        } else {
            if ((rc = ioctl(cavium_i2c_fd, I2C_SLAVE, i2c_dev->dev_addr)) < 0) {
                cterr('f', 0, "%s at %s: unable to connect to device %#x. "
                          "rc = %#x", __FUNCTION__, __FILE__,
                          i2c_dev->dev_addr, rc);
                return (FAILED);
            } else {
                 i2c_dev->fp = cavium_i2c_fd;
            }
        }
    }

    return (rc);
}

/******************************************************************************
 *
 * Function   : read_i2c_reg
 * Description: Read i2c device register
 * Inputs     : *dev - pointer to the I2C device
 *              *data - pointer to the data of the register
 *              offset - I2C device register offset
 * Outputs    : PASSED/FAILED
 *
 ******************************************************************************/
int read_i2c_reg (n2g_i2c_dev_t *dev, uchar *data, uint offset, uchar data_size)
{
    uint32 rc = (FAILED);
    n2g_i2c_if_t i2c_if;

    /* Clear error buffer */
    memset(i2c_err_buf, 0, sizeof(i2c_err_buf));

    /* Setup I2C API interface struct */
    i2c_if.size = data_size;
    i2c_if.buf = (char *)data;
    i2c_if.i2c_bus_type = dev->bus_no;
    i2c_if.i2c_dev = dev->dev_addr;
    i2c_if.offset = offset;

    rc = api_mb_i2c_read(dev, i2c_if.offset, i2c_if.size, (char *)i2c_if.buf);

    if (rc != PASSED) {
        sprintf(i2c_err_buf, "%s:%d I2C read failed(rc = %#x).",
                             __FUNCTION__, __LINE__, rc);
        return (FAILED);
    } /* endof if rc */

    return (PASSED);
}

/******************************************************************************
 *
 * Function   : write_i2c_reg
 * Description: Write i2c register
 * Inputs     : *dev - pointer to the I2C device
 *              *data - pointer to the data of the register
 *              offset - I2C device register offset
 * Outputs    : PASSED/FAILED
 *
 ******************************************************************************/
int write_i2c_reg (n2g_i2c_dev_t *dev, uchar *data, uint offset, uchar data_size)
{
    uint32 rc = (FAILED);
    n2g_i2c_if_t i2c_if;

    /* Clear error buffer */
    memset(i2c_err_buf, 0, sizeof(i2c_err_buf));

    /* Setup I2C API interface struct */
    i2c_if.size = data_size;
    i2c_if.buf = (char *)data;
    i2c_if.i2c_bus_type = dev->bus_no;
    i2c_if.i2c_dev = dev->dev_addr;
    i2c_if.offset = offset;

//     printf("api_mb_i2c_write(dev, %d, %d, buf)\n", i2c_if.offset, i2c_if.size);
    rc = api_mb_i2c_write(dev, i2c_if.offset, i2c_if.size, (char *)i2c_if.buf);
    if (rc != PASSED) {
        sprintf(i2c_err_buf, "%s:%d I2C write failed(rc = %#x).",
                             __FUNCTION__, __LINE__, rc);
        return (FAILED);
    } /* endof if rc */

    return (PASSED);
}

/*****************************************************************************
 * 
 * Function   : init_pca9545a
 * Description: Initilialize PCA9545A control register.
 * Inputs     : n2g_i2c_dev_t pointer to the PCA9545A device
 *              Control registeControl register to be initialized to is 
 *              also passed in
 * Outputs    : PASSED/FAILED
 *
 *****************************************************************************/
static uint32 init_pca9545a (n2g_i2c_dev_t *dev, pca_t init)
{
    uint32 rc = FAILED;
    n2g_i2c_if_t i2c_if;
    pca_t ctrl;

    /* Clear MUX error buffer */
    memset(&i2c_err_buf[0], 0, sizeof(i2c_err_buf));

    /* Setup I2C API interface struct */
    i2c_if.size = sizeof(ctrl);
    i2c_if.buf = (char *)&ctrl;
    i2c_if.i2c_bus_type = dev->bus_no;
    i2c_if.i2c_dev = dev->dev_addr;

    ctrl = init;

    rc = api_mb_i2c_write(dev, i2c_if.offset, i2c_if.size, (char *)i2c_if.buf);
    if (rc != PASSED) {
        sprintf(i2c_err_buf, "%s:%d I2C write failed(rc = %#x).",
                             __FUNCTION__, __LINE__, rc);
        return (FAILED);
    } /* endof if rc */
    return (PASSED);
}

/*****************************************************************************
 *
 * Function   : init_mux
 * Description: Wrap function to init 1:4 Mux Control Register.
 * Inputs     : pattern - Data pattern to be initialized.
 * Outputs    : PASSED/FAILED.
 *
 ******************************************************************************/
int init_mux (/*n2g_i2c_dev_t *i2c_dev, */int pattern)
{
    uint32_t rc = FAILED;
    int32_t cavium_i2c_fd;
    n2g_i2c_dev_t i2c_dev;

    /* Setup I2C device data structure */
    i2c_dev.bus_no = CPU_I2C0;
    i2c_dev.dev_addr = CAVIUM_I2C_FPGA_MUX1;
    i2c_dev.rd_hd_size = 0;
    i2c_dev.wr_hd_size = 0;

    /* Set I2C device to SLAVE mode */
    if ((cavium_i2c_fd = (i2c_dev.bus_no == CPU_I2C0)? 
        cavium_i2c_fd0 : cavium_i2c_fd1)) {
        if (cavium_i2c_fd <= 0) {
            cterr('f', 0, "/dev/i2c-octeon.%d is not opened correctly.", i2c_dev.bus_no);
            return (FAILED);
        } else {
            if ((rc = ioctl(cavium_i2c_fd, I2C_SLAVE, i2c_dev.dev_addr)) < 0) {
                cterr('f', 0, "%s at %s: unable to connect to device %#x. "
                          "rc = %#x", __FUNCTION__, __FILE__,
                          i2c_dev.dev_addr, rc);
                return (FAILED);
            } else {
                 i2c_dev.fp = cavium_i2c_fd;
            }
        }
    }

    /* Init MUX (TI PCA9545a) */
    rc = init_pca9545a(&i2c_dev, (pca_t)pattern);
    if (rc != PASSED) {
        cterr('f', 0, "%s", i2c_err_buf);
    }

    return (rc);
}


/*****************************************************************************
 *
 * Function   : show_pca9545a_reg
 * Description: Provide platforms with a mechanism to display some common
 *              device information via the device print function argument.
 * Inputs     : n2g_i2c_dev_t pointer to the PCA9545A device
 *              dev_pca_desc_t pointer
 * Outputs    : PASSED/FAILED
 *
 *****************************************************************************/
static uint32 show_pca9545a_reg (n2g_i2c_dev_t *dev, dev_pca_desc_t *pdesc)
{
    uint32 rc = FAILED;
    pca_t ctrl; /* Control register from PCA9545A */
    n2g_i2c_if_t i2c_if;
    
    /* Clear MUX error buffer */
    memset(&i2c_err_buf[0], 0, sizeof(i2c_err_buf));

    /* Setup I2C API interface struct */
    i2c_if.i2c_bus_type = dev->bus_no;
    i2c_if.i2c_dev = dev->dev_addr;
    i2c_if.buf = (char *)&ctrl;
    i2c_if.size = sizeof(ctrl);

    /* Read the control register of PCA9545A */
    rc = api_mb_i2c_read(dev, i2c_if.offset, i2c_if.size, (char *)i2c_if.buf);

    if (rc != PASSED) {
        sprintf(i2c_err_buf, "%s:%d I2C read failed(rc = %#x).",
                             __FUNCTION__, __LINE__, rc);
        return (FAILED);
    }

    printf("\nPCA9545a 1:4 Mux Control register = %02x:\n", ctrl);

    while(pdesc && pdesc->name) {
        printf(" %s - ", pdesc->name);
        if (ctrl & pdesc->mask) {
            printf("%s\n", pdesc->true);
        } else {
            printf("%s\n", pdesc->false);
        }
        pdesc++;   /* points to the next field */
    } /* endof while */

    return (PASSED);
}


/*****************************************************************************
 *
 * Function   : show_mux
 * Description: Display 1:4 Mux Control Register.
 * Inputs     : None
 * Outputs    : PASSED/FAILED
 *
 *****************************************************************************/
int show_mux (void)
{
    uint32_t rc = FAILED;
    n2g_i2c_dev_t i2c_dev;
    dev_pca_desc_t *pca_desc_p;
//     int dummy = 0;

    /* Setup I2C device data structure */
    i2c_dev.bus_no = CPU_I2C0;
    i2c_dev.dev_addr = CAVIUM_I2C_FPGA_MUX1;
    i2c_dev.rd_hd_size = 0;
    i2c_dev.wr_hd_size = 0;

    /* Set I2C device to SLAVE mode */
    if (cavium_i2c_fd0 <= 0) {
         cterr('f', 0, "/dev/i2c-octeon.1/ is not opened correctly.");
         return (FAILED);
    } else {
        /* Set I2C device to SLAVE */
        if ((rc = ioctl(cavium_i2c_fd0, I2C_SLAVE, i2c_dev.dev_addr)) < 0) {
            cterr('f', 0, "%s at %s: unable to connect to device %#x. "
                          "rc = %#x", __FUNCTION__, __FILE__,
                          i2c_dev.dev_addr, rc);
            return (FAILED);
        } else {
            i2c_dev.fp = cavium_i2c_fd0;
        }
    }

    /* Setup descriptor table */
    pca_desc_p = (dev_pca_desc_t *)&mux_bit_desc;

    /* Display register contents */
    rc = show_pca9545a_reg(&i2c_dev, pca_desc_p);
    if (rc != PASSED) {
        cterr('f', 0, "%s", i2c_err_buf);
    }
    return (rc);
}

/*****************************************************************************
 *
 * Function   : alter_pca9545a_ctrl_reg
 * Description: Peek-n-poke PCA9545A control register.
 * Inputs     : n2g_i2c_dev_t pointer to the PCA9545A device
 * Outputs    : PASSED/FAILED
 *
 *****************************************************************************/
static int alter_pca9545a_ctrl_reg (n2g_i2c_dev_t *dev)
{
    n2g_i2c_if_t i2c_if;
    uint32_t rc = FAILED;
    pca_t old_ctrl, new_ctrl;

    /* Clear MUX error buffer */
    memset(&i2c_err_buf[0], 0, sizeof(i2c_err_buf));

    /* Setup I2C API interface struct */
    i2c_if.size = sizeof(pca_t);
    i2c_if.i2c_bus_type = dev->bus_no;
    i2c_if.i2c_dev = dev->dev_addr;

    /* Read the register first. */
    i2c_if.buf = (char *)&old_ctrl;

    rc = api_mb_i2c_read(dev, i2c_if.offset, i2c_if.size, (char *)i2c_if.buf);
    if (rc != PASSED) {
        sprintf(i2c_err_buf, "%s:%d I2C read failed(rc = %#x).",
                             __FUNCTION__, __LINE__, rc);
        return (FAILED);
    } /* endof if rc */

    new_ctrl = gethex_answer("Enter the new control register", old_ctrl,
                             PCA9545_CTRL_MIN, PCA9545_CTRL_MAX);

    /* Write the new data */
    i2c_if.buf = (char *)&new_ctrl;

    rc = api_mb_i2c_write(dev, i2c_if.offset, i2c_if.size, (char *)i2c_if.buf);
    if (rc != PASSED) {
        sprintf(i2c_err_buf, "%s:%d I2C write failed(rc = %#x).",
                             __FUNCTION__, __LINE__, rc);
        return (FAILED);
    } /* endof if rc */
    return (PASSED);
}


/*****************************************************************************
 *
 * Function   : alter_mux
 * Description: Alter 1:4 Mux Control Register.
 * Inputs     : None
 * Outputs    : PASSED/FAILED.
 *
 *****************************************************************************/
int alter_mux (void)
{
    uint32_t rc = FAILED;
    n2g_i2c_dev_t i2c_dev;
//     int dummy = 0;

//     dummy = opt;

    /* Setup I2C device data structure */
    i2c_dev.bus_no = CPU_I2C0;
    i2c_dev.dev_addr = CAVIUM_I2C_FPGA_MUX1;
    i2c_dev.rd_hd_size = 0;
    i2c_dev.wr_hd_size = 0;

    /* Set I2C device to SLAVE mode */
    if (cavium_i2c_fd0 <= 0) {
         cterr('f', 0, "/dev/i2c-octeon.1/ is not opened correctly.");
         return (FAILED);
    } else {
        /* Set I2C device to SLAVE */
        if ((rc = ioctl(cavium_i2c_fd0, I2C_SLAVE, i2c_dev.dev_addr)) < 0) {
            cterr('f', 0, "%s at %s: unable to connect to device %#x. "
                          "rc = %#x", __FUNCTION__, __FILE__,
                          i2c_dev.dev_addr, rc);
            return (FAILED);
        } else {
            i2c_dev.fp = /*cavium_i2c_fd1*/cavium_i2c_fd0;
        }
    }

    /* Alter PCA9545a Ctrl Reg */
    rc = alter_pca9545a_ctrl_reg(&i2c_dev);
    if (rc != PASSED) {
        cterr('f', 0, "%s", i2c_err_buf);
    }
    return (rc);
}

/******************************************************************************
 *
 * Function   : set_mux_channel
 * Description: This function set the mux channel for the slave device.
 * Inputs     : i2c_p - Pointer to the N2G I2C API interface struct
 *                      Fields needed in the struct are:
 *                      i2c_bus_type, i2c_dev
 *              mux_mask - data for mux setup
 * Outputs    : PASSED or I2C error code
 *
 ******************************************************************************/
int set_mux_channel (n2g_i2c_dev_t *i2c_dev, uint8_t mux_mask)
{
    uint rc = FAILED;
//     n2g_i2c_dev_t i2c_dev;

    /* Init Mux */
    rc = init_mux(/*&i2c_dev,*/ 0);
    if (rc != PASSED) {
        /* Unable to init Mux */
        cterr('f', 0, "%s:%d Mux init failed (rc = %#x).",
                      __FUNCTION__, __LINE__, rc);
        return (rc);
    }

    /* set Mux channel */
//     printf("mux_mask = %#x\n", mux_mask);
    rc = api_mb_i2c_write(i2c_dev, /*0*/mux_mask, 1, (char *)&mux_mask);
    if (rc != PASSED) {
        /* Unable to set Mux */
        cterr('f', 0, "%s:%d set Mux failed (rc = %#x).",
                      __FUNCTION__, __LINE__, rc);
    }
    return (rc);
}

/******************************************************************************
 *
 * Function    : ds4424_i2c_read
 * Description : DS4424 Register Read through Cavium I2C Bus
 * Input       : addr  - register offset.
 *               buf   - read buffer
 *
 * Output: PASSED/FAILED
 *
 *****************************************************************************/
int ds4424_i2c_read (int addr, uchar *buf)
{
    n2g_i2c_dev_t i2c_dev;
    uint i2c_slave_addr;
    i2c_slave_addr = CAVIUM_I2C_DS4424;

    i2c_dev.rd_hd_size = 1;
    i2c_dev.wr_hd_size = 1;

    /* Open the Cavium I2C bus 1 */
    if (open_i2c(&i2c_dev, i2c_slave_addr, CPU_I2C1) == FAILED) {
        return (FAILED);
    }

    /* Call the I2C common I2C api to read back the register value */
//     printf("read_i2c_reg(&i2c_dev, (uchar *)buf, %#x, 1)\n", addr);
    return (read_i2c_reg(&i2c_dev, (uchar *)buf, addr, 1));
}

/******************************************************************************
 *
 * Function    : ds4424_i2c_write
 * Description : DS4424 Register Write through Cavium I2C iface
 * Input       : addr  - register offset.
 *               data  - data for write
 *
 * Output: PASSED/FAILED
 *
 *****************************************************************************/
int ds4424_i2c_write (int addr, uchar data)
{
    n2g_i2c_dev_t i2c_dev;
    uint i2c_slave_addr;
    i2c_slave_addr = CAVIUM_I2C_DS4424;

    i2c_dev.rd_hd_size = 1;
    i2c_dev.wr_hd_size = 1;

    /* Open the Cavium I2C bus 1 */
    if (open_i2c(&i2c_dev, i2c_slave_addr, CPU_I2C1) == FAILED) {
        return (FAILED);
    }

    /* Call the I2C common I2C api to write the register value */
//     printf("write_i2c_reg(&i2c_dev, (uchar *)&data, %#x, 1)", addr);
    return (write_i2c_reg(&i2c_dev, (uchar *)&data, addr, 1));
}

int i2c_rd_util()
{
    n2g_i2c_dev_t i2c_dev;
    ushort i2c_bus = 0;
    uint dev_addr = 0;
    uint offset = 0;
    uchar read_data = 0;

    i2c_bus = gethex_answer("\nEnter i2c bus[0/1]:",
               0, 0, 1);

    dev_addr = gethex_answer("\nEnter device address:",
               0, 0, 0xFF);

    offset = gethex_answer("\nEnter offset:",
               0, 0, 0xFFFF);

    i2c_dev.rd_hd_size = 1;
    i2c_dev.wr_hd_size = 1;

    /* Open the Cavium I2C bus */
    if (open_i2c(&i2c_dev, dev_addr, i2c_bus) == FAILED) {
        return (FAILED);
    }

    if (read_i2c_reg(&i2c_dev, &read_data, offset, 1) == FAILED) {
        printf("Failed to read I2c Bus-%d Device Address %#x Offset %#x\n", 
            i2c_bus, dev_addr, offset);
        return (FAILED);
    } else {
        printf("I2c Bus-%d Device Address %#x Offset %#x Value = %#x\n", 
            i2c_bus, dev_addr, offset, read_data);
        return (PASSED);
    }
}


int i2c_wr_util()
{
    n2g_i2c_dev_t i2c_dev;
    ushort i2c_bus = 0;
    uint dev_addr = 0;
    uint offset = 0;
    uchar write_data = 0;
    uchar read_data = 0;

    i2c_bus = gethex_answer("\nEnter i2c bus[0/1]:",
               0, 0, 1);

    dev_addr = gethex_answer("\nEnter device address:",
               0, 0, 0xFF);

    offset = gethex_answer("\nEnter offset:",
               0, 0, 0xFFFF);

    i2c_dev.rd_hd_size = 1;
    i2c_dev.wr_hd_size = 1;

    /* Open the Cavium I2C bus */
    if (open_i2c(&i2c_dev, dev_addr, i2c_bus) == FAILED) {
        return (FAILED);
    }

    if (write_i2c_reg(&i2c_dev, &write_data, offset, 1) == FAILED) {
        return (FAILED);
    }

    printf("I2c Bus-%d Device Address %#x Offset %#x Write Value = %#x\n", 
        i2c_bus, dev_addr, offset, write_data);

    if (read_i2c_reg(&i2c_dev, &read_data, offset, 1) == FAILED) {
        return (FAILED);
    }

    printf("Read back: Value = %#x\n", read_data);
    return (PASSED);
}


/******************************************************************************
 *
 * Function   : pca9557_power_margin_ctrl
 * Description: By setting PCA9557 to control DC/DC output voltage
 * Inputs     : none
 * Outputs    : PASSED/FAILED
 *
 ******************************************************************************/
int pca9557_power_margin_ctrl (void)
{
    unsigned long voltage, margin;
    n2g_i2c_dev_t i2c_dev;
    uint i2c_slave_addr;
    uchar buf, offset_val;
    i2c_slave_addr = CAVIUM_PCA9557;

    /* Open the Cavium I2C bus 1 */
    if (open_i2c(&i2c_dev, i2c_slave_addr, CPU_I2C1) == FAILED) {
        printf("Fail to open the i2c interface\n");
        return (FAILED);
    }

    offset_val = PCA9557_CONFIGURATION_VAL;
    /* Configure to output pin */
    if (write_i2c_reg(&i2c_dev, &offset_val, PCA9557_CONFIGURATION_REG, 
                      sizeof(pca9557)) != PASSED) {
        printf("Write val %x to reg %x failed\n", offset_val, 
               PCA9557_CONFIGURATION_REG);
        return (FAILED);
    }

    /* Read original val of output reg */
    if (read_i2c_reg(&i2c_dev, &buf, PCA9557_OUTPUT_REG, 
                     sizeof(pca9557)) != PASSED) {
        printf("Read reg %x fail\n", PCA9557_OUTPUT_REG);
        return (FAILED);
    }

    printf("Select Voltage Rail\n");
    printf("Enter 0 - 1.0V Rail\n");
    printf("Enter 1 - 1.1V Rail\n");
    printf("Enter 2 - 1.5V Rail\n");
    printf("Enter 3 - 3.3V Rail\n");
    fflush(stdout);
    voltage = gethex_answer("Enter Voltage Rail: ", 0, 0x0, 0x3);

    printf("\nSelect Margin\n");
    printf("Enter 0 - Normal\n");
    printf("Enter 1 - Margin High\n");
    printf("Enter 2 - Margin Low\n");
    fflush(stdout);
    margin = gethex_answer("Enter Voltage Rail: ", 0, 0x0, 2);

    switch (voltage) {
    case 0: /* 1.0 V */
        buf &= ~(PCA9557_VAL_0_1 | PCA9557_VAL_0_2);
        switch (margin) {
        case 0: /* Normal */
            break;
        case 1: /* High */
            buf |= PCA9557_VAL_0_1;
            break;
        case 2: /* Low */
            buf |= PCA9557_VAL_0_2;
            break;
        default :
            printf("Enter wrong val - %ld\n", margin);
            return (FAILED);
        }
        break;
    case 1: /* 1.1 V */
        buf &= ~(PCA9557_VAL_1_1 | PCA9557_VAL_1_2);
        switch (margin) {
        case 0: /* Normal */
            break;
        case 1: /* High */
            buf |= PCA9557_VAL_1_1;
            break;
        case 2: /* Low */
            buf |= PCA9557_VAL_1_2;
            break;
        default :
            printf("Enter wrong val - %ld\n", margin);
            return (FAILED);
        }
        break;
    case 2: /* 1.5 V */
        buf &= ~(PCA9557_VAL_2_1 | PCA9557_VAL_2_2);
        switch (margin) {
        case 0: /* Normal */
            break;
        case 1: /* High */
            buf |= PCA9557_VAL_2_1;
            break;
        case 2: /* Low */
            buf |= PCA9557_VAL_2_2;
            break;
        default :
            printf("Enter wrong val - %ld\n", margin);
            return (FAILED);
        }
        break;
    case 3: /* 3.3 V */
        buf &= ~(PCA9557_VAL_3_1 | PCA9557_VAL_3_2);
        switch (margin) {
        case 0: /* Normal */
            break;
        case 1: /* High */
            buf |= PCA9557_VAL_3_1;
            break;
        case 2: /* Low */
            buf |= PCA9557_VAL_3_2;
            break;
        default :
            printf("Enter wrong val - %ld\n", margin);
            return (FAILED);
        }
            break;
        default :
            cterr('f', 0, "Not support this mode - %ld", margin);
            return (FAILED);
    }

    /* Call the I2C common I2C api to write the register value */
    if (write_i2c_reg(&i2c_dev, &buf, PCA9557_OUTPUT_REG, sizeof(pca9557)) != PASSED) {
        printf("Write val %x to reg %x failed\n", offset_val, PCA9557_OUTPUT_REG);
        return (FAILED);
    }

    return (PASSED);
}

/******************************************************************************
 *
 * Function   : pca9557_power_margin_status
 * Description: Show PCA9557 power margin status
 * Inputs     : none
 * Outputs    : PASSED/FAILED
 *
 ******************************************************************************/
int pca9557_power_margin_status (void)
{   
    n2g_i2c_dev_t i2c_dev;
    uint i2c_slave_addr;
    uchar buf, mode_0, mode_1, mode_2, mode_3;
    i2c_slave_addr = CAVIUM_PCA9557;

    /* Open the Cavium I2C bus 1 */
    if (open_i2c(&i2c_dev, i2c_slave_addr, CPU_I2C1) == FAILED) {
        printf("Fail to open the i2c interface\n");
        return (FAILED);
    }

    /* Read original val of output reg */
    if (read_i2c_reg(&i2c_dev, &buf, PCA9557_OUTPUT_REG, 
                     sizeof(pca9557)) != PASSED) {
        printf("Read reg %x fail\n", PCA9557_OUTPUT_REG);
        return (FAILED);
    }

    printf("1.0V - ");
    mode_0 = buf & (PCA9557_VAL_0_1 | PCA9557_VAL_0_2);
    if (mode_0 == PCA9557_VAL_0_1) {
        printf("Margin high\n");
    } else if (mode_0 == PCA9557_VAL_0_2) {
        printf("Margin low\n");
    } else {
        printf("Margin normal\n");
    }

    printf("1.1V - ");
    mode_1 = buf & (PCA9557_VAL_1_1 | PCA9557_VAL_1_2);
    if (mode_1 == PCA9557_VAL_1_1) {
        printf("Margin high\n");
    } else if (mode_1 == PCA9557_VAL_1_2) {
        printf("Margin low\n");
    } else {
        printf("Margin normal\n");
    }

    printf("1.5V - ");
    mode_2 = buf & (PCA9557_VAL_2_1 | PCA9557_VAL_2_2);
    if (mode_2 == PCA9557_VAL_2_1) {
        printf("Margin high\n");
    } else if (mode_2 == PCA9557_VAL_2_2) {
        printf("Margin low\n");
    } else {
        printf("Margin normal\n");
    }

    printf("3.3V - ");
    mode_3 = buf & (PCA9557_VAL_3_1 | PCA9557_VAL_3_2);
    if (mode_3 == PCA9557_VAL_3_1) {
        printf("Margin high\n");
    } else if (mode_3 == PCA9557_VAL_3_2) {
        printf("Margin low\n");
    } else {
        printf("Margin normal\n");
    }
    return (PASSED);
}


int voltage_no_margin(void)
{
    if(ds4424_i2c_write(OUT0_33, VOLTAGE_33_NO)) {
        return (FAILED);
    }
    if(ds4424_i2c_write(OUT1_15, VOLTAGE_15_NO)) {
        return (FAILED);
    }
    if(ds4424_i2c_write(OUT3_10, VOLTAGE_10_NO)) {
        return (FAILED);
    }

    if (get_board_id() == 1) {
        if(ds4424_i2c_write(OUT2_092, VOLTAGE_092_NO)) {
            return (FAILED);
        }
    } else {
        if(ds4424_i2c_write(OUT2_085, VOLTAGE_085_NO)) {
            return (FAILED);
        }
    }
    return (PASSED);
}

int voltage_margin_high(void)
{
    if(ds4424_i2c_write(OUT0_33, VOLTAGE_33_HIGH)) {
        return (FAILED);
    }
    if(ds4424_i2c_write(OUT1_15, VOLTAGE_15_HIGH)) {
        return (FAILED);
    }
    if(ds4424_i2c_write(OUT3_10, VOLTAGE_10_HIGH)) {
        return (FAILED);
    }

    if (get_board_id() == 1) {
        if(ds4424_i2c_write(OUT2_092, VOLTAGE_092_HIGH)) {
            return (FAILED);
        }
    } else {
        if(ds4424_i2c_write(OUT2_085, VOLTAGE_085_HIGH)) {
            return (FAILED);
        }
    }

    return (PASSED);
}


int voltage_margin_nom(void)
{
    if(ds4424_i2c_write(OUT0_33, VOLTAGE_33_NOM)) {
        return (FAILED);
    }
    if(ds4424_i2c_write(OUT1_15, VOLTAGE_15_NOM)) {
        return (FAILED);
    }
    if(ds4424_i2c_write(OUT3_10, VOLTAGE_10_NOM)) {
        return (FAILED);
    }

    if (get_board_id() == 1) {
        if(ds4424_i2c_write(OUT2_092, VOLTAGE_092_NOM)) {
            return (FAILED);
        }
    } else {
        if(ds4424_i2c_write(OUT2_085, VOLTAGE_085_NOM)) {
            return (FAILED);
        }
    }

    return (PASSED);
}

int voltage_margin_low(void)
{
    if(ds4424_i2c_write(OUT0_33, VOLTAGE_33_LOW)) {
        return (FAILED);
    }
    if(ds4424_i2c_write(OUT1_15, VOLTAGE_15_LOW)) {
        return (FAILED);
    }
    if(ds4424_i2c_write(OUT3_10, VOLTAGE_10_LOW)) {
        return (FAILED);
    }

    if (get_board_id() == 1) {
        if(ds4424_i2c_write(OUT2_092, VOLTAGE_092_LOW)) {
            return (FAILED);
        }
    } else {
        if(ds4424_i2c_write(OUT2_085, VOLTAGE_085_LOW)) {
            return (FAILED);
        }
    }

    return (PASSED);
}


/**********************************************************************
 * Function: voltage_margin_specific
 * Description: allow the user to specify the voltage and margin value
 *
 * Input : none
 * Output: PASSED/FAILED
 *
 **********************************************************************/
int voltage_margin_specific(void)
{
    char buffer[4];
    int ret = 0;
    uchar val;

    while (1) {
        printf("\nSelect which voltage to margin\n");
        printf("\na: 3.3V\n");
        printf("b: 1.5V\n");
        printf("c: 1.0V\n");
        if (get_board_id() == 1) {
            printf("d: 0.92V\n");
        } else {
            printf("d: 0.85V\n");
        }
        printf("e: exit\n");
        printf("\nplease input: ");

        get_line(buffer,sizeof(buffer));

        if(buffer[0] == 'e') {
            break;
        }
        val = getdec_answer("please input value to margin : ", 0, 0, 255);

        switch(buffer[0]) {
        case 'a':
            ret = ds4424_i2c_write(OUT0_33, val);
            break;
        case 'b':
            ret = ds4424_i2c_write(OUT1_15, val);
            break;
        case 'c':
            ret = ds4424_i2c_write(OUT3_10, val);
            break;
        case 'd':
            ret = ds4424_i2c_write(OUT2_092, val);
            break;
        default:
            break;
         }
        if (ret) {
            cterr('f', 0, "Margin set error\n");
            return (FAILED);
        }
    }
    return (PASSED);
}

/***************************************************************************
 * Function: voltage_margin_display
 * Description: This function shows current margins of all kinds of voltage
 * Input : None
 * Output: None
 *
 ***************************************************************************/
void voltage_margin_display(void)
{
    uchar volt;

    if(!ds4424_i2c_read(OUT0_33, &volt)) {
        printf("\n3.3V current margin: %d\n", volt);
    } else {
        printf("\nMargin read error\n");
    }

    if(!ds4424_i2c_read(OUT1_15, &volt)) {
        printf("1.5V current margin: %d\n", volt);
    } else {
        printf("Margin read error\n");
    }

    if(!ds4424_i2c_read(OUT3_10, &volt)) {
        printf("1.0V current margin: %d\n", volt);
    } else {
        printf("Margin read error\n");
    }

    if (get_board_id() == 1) {
        if(!ds4424_i2c_read(OUT2_092, &volt)) {
            printf("0.92V current margin: %d\n", volt);
        } else {
            printf("Margin read error\n");
        }
    } else {
        if(!ds4424_i2c_read(OUT2_085, &volt)) {
            printf("0.85V current margin: %d\n", volt);
        } else {
            printf("Margin read error\n");
        }
    }
}

/***************************************************************************
 * Function: margin_test
 * 
 * Input : None
 * Output: PASSED/FAILED
 *
 ***************************************************************************/
int margin_test(void)
{
    uchar volt = 0;

    prpass(testpass, "Margin test.");

    voltage_margin_nom();

    ds4424_i2c_read(OUT0_33, &volt);
    if(volt != VOLTAGE_33_NOM) {
        cterr('f', 0, "3.3V margin error.");
        return (FAILED);
    }

    ds4424_i2c_read(OUT1_15, &volt);
    if(volt != VOLTAGE_15_NOM) {
        cterr('f', 0, "1.5V margin error.");
        return (FAILED);
    }

    ds4424_i2c_read(OUT3_10, &volt);
    if(volt != VOLTAGE_10_NOM) {
        cterr('f', 0, "1.0V margin error.");
        return (FAILED);
    }

    ds4424_i2c_read(OUT2_092, &volt);
    if(volt != VOLTAGE_092_NOM) {
        cterr('f', 0, "0.92V margin error.");
        return (FAILED);
    }

    voltage_no_margin();

    return (PASSED);
}

/******** History ********
*---------------------------------------------------
* $Log: platform_i2c.c,v $
* Revision 1.1  2015/02/26 07:18:29  xiaoyizh
* Initial check in for Wallander.
*
*
*---------------------------------------------------
* $Endlog$
*/
