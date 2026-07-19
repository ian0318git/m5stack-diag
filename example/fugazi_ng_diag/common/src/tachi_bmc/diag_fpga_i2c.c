/* $Id: diag_fpga_i2c.c,v 1.2 2016/04/20 11:25:30 benchen2 Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/tachi_bmc/diag_fpga_i2c.c,v $
 *------------------------------------------------------------------
 *
 * diag_fpga_i2c.c - FPGA I2C Library
 *
 * June 2015, Times Huang
 *
 * Copyright (c) 2015-2016 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */
 
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/ioctl.h> 
#include "common.h"
#include "types.h"
#include "nvmonvars.h"
#include "proto.h"
#include "error.h"
#include "byteswap.h"
#include "diag_fpga_lib.h"
#include "diag_fpga_i2c.h"
#include "diag_i2c_api.h"
#include "platform_i2c.h"


static int err_no = 0;
static int i2c_status = 0;

static void diag_fpga_i2c_reset(int);
static boolean diag_fpga_check_i2c_idle(int); 
static uint32_t i2c_dswap4(int);
static void diag_fpga_i2c_wr_data_fifo(int, uint32_t, uchar *);
static int diag_fpga_i2c_send_reg_offset(int, uint32_t, uint32_t, uint32_t, uint32_t);
static int diag_fpga_i2c_normal_op(int, uint32_t, uint32_t, uint32_t,
                                   uint32_t, uint32_t, uint32_t);
static int diag_fpga_i2c_flush_fifo(int, int);
static void diag_fpga_i2c_rd_data_fifo(int, uint32_t, uchar *);

int diag_fpga_get_i2c_ctrl_addr(uint8);
int diag_fpga_i2c_write(int, int, int, int, uint, uint, uchar *);
int diag_fpga_i2c_read(int, int, int, int, uint, uint, uchar *);

#ifdef FOXCONN_FPGA
int diag_fxn_fpga_i2c_write(int, int, int, int, uint, uint, uchar *);
int diag_fxn_fpga_i2c_read(int, int, int, int, uint, uint, uchar *);
static void diag_fxn_fpga_i2c_wr_data_fifo(int, uint32_t, uchar *);
static void diag_fxn_fpga_i2c_rd_data_fifo(int, uint32_t, uchar *);
#endif

int diag_fpga_get_i2c_ctrl_addr (uint8 i2c_no)
{
    return (I2C0_CTRL_OFFSET + (i2c_no * 0x100));
}

int diag_fpga_i2c_write (int i2c_ctrl, int mux, int slv_addr, int reg_addr,
                         uint sub_addr_size, uint data_len, uchar *data_buf)
{
    int rc;
    uchar *buf = NULL;

    if (diag_fpga_check_i2c_idle(i2c_ctrl) == FALSE) {
        return (RC_I2C_BUSY);
    }

    /* We should allocate at least 4 bytes */
    buf = malloc(data_len + sizeof(uint32_t));
    memset(buf, 0, data_len + sizeof(uint32_t));

    if (sub_addr_size != 0) {
        diag_fpga_i2c_wr_data_fifo(i2c_ctrl, data_len, data_buf);
    } else {
        /* If required, send reg offset to slave device and flush fifo */
        if (reg_addr >= 0) {
            if ((NVRAM)->diagflag & D_VERBOSE) {
                printf("%s: Reg offset is %d\n", __FUNCTION__, reg_addr);
            }
            buf[0] = (uchar)reg_addr & 0xFF;
            data_len++;
            memcpy(&buf[1], data_buf, data_len);
        } else {
            /* For smart device (ACT2) that don't want address to be sent
             * Here we send data only..no address
             */
            memcpy(&buf[0], data_buf, data_len);
            if ((NVRAM)->diagflag & D_VERBOSE) {
                printf("%s: len=%d xx %#x %#x %#x %#x, offset=%d\n", __FUNCTION__,
                       data_len, buf[0], buf[1], buf[2], buf[3], reg_addr);
            }
        }
        diag_fpga_i2c_wr_data_fifo(i2c_ctrl, data_len, buf); 
    }

    free(buf);

    /* Write data to Fifo... (is it safe to flush fifo here, or do it
     * later after we send out data to slave?)
     */
    rc = diag_fpga_i2c_normal_op(i2c_ctrl, mux, slv_addr, data_len,
                                 sub_addr_size, reg_addr, 
                                 FPGA_I2C_CTRL_WR_MODE);

    return (rc);
}

#ifdef FOXCONN_FPGA
int diag_fxn_fpga_i2c_write (int i2c_ctrl, int mux, int slv_addr, int reg_addr,
                             uint sub_addr_size, uint data_len, uchar *data_buf)
{
    int reg_val, master_sts_reg;
    int ix;

    /* write data to fifo reg*/
    diag_fxn_fpga_i2c_wr_data_fifo(i2c_ctrl, data_len, data_buf);
    
    /* slave address reg*/
    diag_fpga_reg_write(i2c_ctrl + FPGA_I2C_SLAVE_ADDR_REG, slv_addr);
    
    /* slave sub-address reg*/
    diag_fpga_reg_write(i2c_ctrl + FPGA_I2C_SLAVE_SUBADDR_REG, reg_addr);

    /* master control reg*/
    reg_val = FPGA_I2C_CTRL_WR_MODE | FPGA_I2C_CTRL_SPEED_NORMAL_100 |
              FPGA_I2C_CTRL_SLV_ADDR_7 | FPGA_I2C_CTRL_CLK_50 |
              FPGA_I2C_CTRL_ENABLE; 

    /* Fill in sub address length */
    if (sub_addr_size) {
        reg_val |= (sub_addr_size << FPGA_I2C_CTRL_SUBSIZE_SHIFT);
    }
    /* Fill in data size */
    reg_val |= (data_len << FPGA_I2C_CTRL_BYTE_LENGTH_SHIFT);

    diag_fpga_reg_write(i2c_ctrl + FPGA_I2C_MASTER_CTRL_REG, reg_val);

    for (ix = 0; ix < FPGA_I2C_OP_TIMEOUT; ix++) {
        /*master status reg*/
        diag_fpga_reg_read(i2c_ctrl + FPGA_I2C_MASTER_CTRL_REG, &master_sts_reg);

        if (!(master_sts_reg & (FPGA_I2C_BIT_BANG | FPGA_I2C_STANDARD_MODE))) {
            break;
        }
        msleep(1);
    }
    
    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("master status reg is %x\n", master_sts_reg);
    }

    if (ix == FPGA_I2C_OP_TIMEOUT) {
        printf("%s: FPGA I2C OP Timeout (%#x)\n", __func__, master_sts_reg);
        return (RC_I2C_SLV_NACK);
    }

    return (RC_I2C_OP_OK);
}
#endif

int diag_fpga_i2c_read (int i2c_ctrl, int mux, int slv_addr, int reg_addr,
                        uint sub_addr_size, uint data_len, uchar *data_buf)
{
    int rc;

    if (diag_fpga_check_i2c_idle(i2c_ctrl) == FALSE) {
        return (RC_I2C_BUSY);
    }

    /* if required, send reg offset to slave and flush fifo */
    if ((reg_addr >= 0) && (sub_addr_size == 0)) {
        rc = diag_fpga_i2c_send_reg_offset(i2c_ctrl, mux, slv_addr, reg_addr,
                                           sub_addr_size);
        if (rc != RC_I2C_OP_OK) {
            return (rc);
        }
    }

    /* Send Read request to the slave */
    rc = diag_fpga_i2c_normal_op(i2c_ctrl, mux, slv_addr, data_len,
                                 sub_addr_size, reg_addr, 
                                 FPGA_I2C_CTRL_RD_MODE);
    
    if (rc != RC_I2C_OP_OK) {
        return (rc);
    }
    
    /* Read data from fifo and flush */
    diag_fpga_i2c_rd_data_fifo(i2c_ctrl, data_len, data_buf);
    
    return (rc);
}

#ifdef FOXCONN_FPGA
int diag_fxn_fpga_i2c_read (int i2c_ctrl, int mux, int slv_addr, int reg_addr,
                            uint sub_addr_size, uint data_len, uchar *data_buf)
{
    int reg_val, master_sts_reg;
    int ix;

    /* slave address reg*/
    diag_fpga_reg_write(i2c_ctrl + FPGA_I2C_SLAVE_ADDR_REG, slv_addr);
    
    /* slave sub-address reg*/
    diag_fpga_reg_write (i2c_ctrl + FPGA_I2C_SLAVE_SUBADDR_REG, reg_addr);
    
    /* master control reg*/
    reg_val = FPGA_I2C_CTRL_RD_MODE | FPGA_I2C_CTRL_SPEED_NORMAL_100 |
              FPGA_I2C_CTRL_SLV_ADDR_7 | FPGA_I2C_CTRL_CLK_50 |
              FPGA_I2C_CTRL_ENABLE;

    /* Fill in sub address length */
    if (sub_addr_size) {
        reg_val |= (sub_addr_size << FPGA_I2C_CTRL_SUBSIZE_SHIFT);
    }

    /* Fill in data size */
    reg_val |= (data_len << FPGA_I2C_CTRL_BYTE_LENGTH_SHIFT);

    diag_fpga_reg_write(i2c_ctrl + FPGA_I2C_MASTER_CTRL_REG, reg_val);
    
    for (ix = 0; ix < FPGA_I2C_OP_TIMEOUT; ix++) {
        /*master status reg*/
        diag_fpga_reg_read(i2c_ctrl + FPGA_I2C_MASTER_CTRL_REG, &master_sts_reg);

        if (!(master_sts_reg & (FPGA_I2C_BIT_BANG | FPGA_I2C_STANDARD_MODE))) {
            break;
        }
        msleep(1);
    }
    
    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("master status reg is %x\n", master_sts_reg);
    }

    if (ix == FPGA_I2C_OP_TIMEOUT) {
        printf("%s: FPGA I2C OP Timeout (%#x)\n", __func__, master_sts_reg);
        return (RC_I2C_SLV_NACK);
    }
    diag_fxn_fpga_i2c_rd_data_fifo(i2c_ctrl, data_len, data_buf);
    return (RC_I2C_OP_OK);
}
#endif

static void diag_fpga_i2c_rd_data_fifo (int i2c_ctrl, uint32_t data_len, 
                                        uchar *data_p)
{
    uint32_t ix, jx, word_count, byte_count;
    uint32_t dword;
    uchar * byte_p;
    int data_in;

    if ((NVRAM)->diagflag & D_VERBOSE) {
        diag_fpga_reg_read(i2c_ctrl + FPGA_I2C_BYTECNT_REG, &data_in);
        printf("%s: Byte Count: %#.8x\n", __FUNCTION__, data_in);
        diag_fpga_reg_read(i2c_ctrl + FPGA_I2C_DATA_FIFO_PTR_REG, &data_in);
        printf("%s: Data FIFO RW PTR: %#.8x\n", __FUNCTION__, data_in);
    }

    word_count = data_len / 4;
    byte_count = data_len % 4;

    for (ix = 0; ix < word_count; ix++) {
        diag_fpga_reg_read(i2c_ctrl + FPGA_I2C_DATA_FIFO_REG, &data_in);
        dword = i2c_dswap4(data_in);
        byte_p = (uchar *)&dword;
        for (jx = 0; jx < 4; jx++) {
            *data_p++ = *byte_p++;
        }
    }

    if (byte_count > 0) {
        diag_fpga_reg_read(i2c_ctrl + FPGA_I2C_DATA_FIFO_REG, &data_in);
        dword = i2c_dswap4(data_in);
        byte_p = (uchar *)&dword;
        for (ix = 0; ix < byte_count; ix++) {
            *data_p++ = *byte_p++;
        }
    }

    /* Flush again just to be safe */
    diag_fpga_i2c_flush_fifo(i2c_ctrl, data_len);
}

#ifdef FOXCONN_FPGA
static void diag_fxn_fpga_i2c_rd_data_fifo (int i2c_ctrl, uint32_t data_len, 
                                            uchar *data_p)
{
    uint32_t ix;
    int data_in;

    for (ix = 0; ix < data_len; ix++) {
        diag_fpga_reg_read(i2c_ctrl + FPGA_I2C_DATA_FIFO_REG, &data_in);
        *data_p++ = (data_in & 0xFF);
    }
}
#endif

static int diag_fpga_i2c_flush_fifo (int i2c_ctrl, int byte)
{
    int data_in;
    
    while (byte--) {
        diag_fpga_reg_read(i2c_ctrl + FPGA_I2C_DATA_FIFO_REG, &data_in);
    }

    /* Do it couple times just to be safe */
    diag_fpga_reg_read(i2c_ctrl + FPGA_I2C_DATA_FIFO_REG, &data_in);
    diag_fpga_reg_read(i2c_ctrl + FPGA_I2C_DATA_FIFO_REG, &data_in);
    
    return (RC_I2C_OP_OK);
}

static int diag_fpga_i2c_send_reg_offset (int i2c_ctrl, uint32_t mux, 
                                          uint32_t slv_addr, uint32_t reg_addr,
                                          uint32_t sub_addr_size)
{
    uint32_t rc;
    uint32_t addr_size = 1;
    
    /* Write 1 byte reg offset into data fifo */
    diag_fpga_i2c_wr_data_fifo(i2c_ctrl, addr_size, (unsigned char *)&reg_addr);

    /* Initialize write transaction to send offset onto the bus */
    rc = diag_fpga_i2c_normal_op(i2c_ctrl, mux, slv_addr, addr_size, 
                                 sub_addr_size, reg_addr, FPGA_I2C_CTRL_WR_MODE);

    if (rc != RC_I2C_OP_OK) {
        if ((NVRAM)->diagflag & D_VERBOSE) {
            printf("%s: Sending offset failed: %d\n", __FUNCTION__, rc);
        }
    }
    
    return (rc);
}

static int diag_fpga_i2c_normal_op (int i2c_ctrl, uint32_t mux, 
                                    uint32_t slv_addr, uint32_t data_len,
                                    uint32_t sub_addr_size, uint32_t reg_addr, 
                                    uint32_t rd_wr_mode)
{
    uint32_t reg_val, ix, timeout_val;
#ifdef NO_NEED_THIS
    uint32_t wait;
#endif
    int data_in;
    int ack, op_done;

    err_no = 0;
    i2c_status = 0;

    /* Set up control register and slave address register */
    reg_val = FPGA_I2C_CTRL_CLK_50 | FPGA_I2C_CTRL_SLV_ADDR_7 |
              FPGA_I2C_CTRL_SPEED_NORMAL_100 | rd_wr_mode |
              (data_len << L_SHFT_FPGA_I2C_CTRL_BYTE_LEN) |
              (mux << L_SHFT_FPGA_I2C_CTRL_MUX);

    if (mux >= 4) {
        printf("%s: Mux has to be less than 4. (%d)\n", __FUNCTION__, mux);
        return (RC_I2C_SLV_NACK);
    }

    /* Write mode end */
    /* Enable the normal operation */
    diag_fpga_reg_write(i2c_ctrl + FPGA_I2C_SLAVE_ADDR_REG, slv_addr);

    if (sub_addr_size != 0) {
        diag_fpga_reg_write (i2c_ctrl + FPGA_I2C_SLAVE_SUBADDR_REG, reg_addr);
        if ((NVRAM)->diagflag & D_VERBOSE) {
            printf("%s: i2c slave sub addri %#x\n", __FUNCTION__, reg_addr);
        }
    }

    diag_fpga_reg_write(i2c_ctrl + FPGA_I2C_MASTER_CTRL_REG, 
                        reg_val | FPGA_I2C_CTRL_NORMAL | (sub_addr_size) << 24);

    /* Give time for device to send acknowledgement... especially when 
     * talking to ACT2 */
#ifdef DONT_NEED_THIS
    wait = (data_len * 10); /* 10 byte address @ 100Khz */
    msleep(3);
#endif
    msleep(3);

    /* Monitor the done bit in status register. Add 10 safety bytes for wait time
     * calculation due to I2C protocol is slow and have gaps
     */
    ack = op_done = 0;
    timeout_val = 500;

    /* Check if slave acks the I2C operation */
    for (ix = 0; ix < timeout_val; ix++) {
        diag_fpga_reg_read(i2c_ctrl + FPGA_I2C_MASTER_STS_REG, &data_in);
        
#ifdef MB_ACT2_ISSUE
        if (!(data_in & FPGA_I2C_STAT_NO_SLV)) {
            ack = 1;
        }
#endif
        if (!(data_in & FPGA_I2C_STAT_NO_SLV)) {
            ack = 1;
        } else {
            ix = timeout_val; /* Return immediately */
        }

        if ((data_in & FPGA_I2C_STAT_STD_DONE) != 0) {
            op_done = 1;
        }

        if (ack && op_done) {
            break;
        }
        msleep(1);
    }

    if (ix >= timeout_val) {
        if (ack == 0) {
            err_no = (RC_I2C_SLV_NACK);
            i2c_status = reg_val;
            diag_fpga_reg_read(i2c_ctrl + FPGA_I2C_MASTER_STS_REG, &data_in);
            printf("\n\n");
            printf("device shown below is not acknowledging; is it installed? "
                   "Address: %#x, status: %#x, err=%d\n", 
                   i2c_ctrl + FPGA_I2C_MASTER_STS_REG, data_in, err_no);
            return (RC_I2C_SLV_NACK);
        } else {
            err_no = (RC_I2C_TIMEOUT);
            i2c_status = reg_val;
            diag_fpga_reg_read(i2c_ctrl + FPGA_I2C_MASTER_STS_REG, &data_in);
            printf("\n\ndone bit of device shown below is not set. "
                   "Address: %#x, status: %#x, err=%d\n",
                   i2c_ctrl + FPGA_I2C_MASTER_STS_REG, data_in, err_no);
            return (RC_I2C_TIMEOUT);
        }
    }

    return (RC_I2C_OP_OK);
}


static void diag_fpga_i2c_wr_data_fifo (int i2c_ctrl, uint32_t data_len, 
                                        uchar *data_p)
{
    uint32_t ix, jx, word_count, byte_count;
    uint32_t dword;
    uchar *byte_p;

    /* do i need to read until i get an underflow condition to
       make sure fifo is empty before writing to fifo? */
    word_count = data_len / 4;
    byte_count = data_len % 4;

    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("%s: word %d, byte %d\n:", __FUNCTION__, word_count, byte_count);
    }

    for (ix = 0; ix < word_count; ix++) {
        byte_p = (uchar *)&dword;
        for (jx = 0; jx < 4; jx++) {
            *byte_p++ = *data_p++;
        }
        diag_fpga_reg_write(i2c_ctrl + FPGA_I2C_DATA_FIFO_REG, i2c_dswap4(dword));
        if ((NVRAM)->diagflag & D_VERBOSE) {
            /* To be implemented */
        }
    }

    if (byte_count) {
        dword = 0;
        byte_p = (uchar *)&dword;
        
        for (ix = 0; ix < byte_count; ix++) {
            *byte_p++ = *data_p++;
        }
        diag_fpga_reg_write(i2c_ctrl + FPGA_I2C_DATA_FIFO_REG, i2c_dswap4(dword));
        if ((NVRAM)->diagflag & D_VERBOSE) {
            /* To be implemented */
        }
    }
}

#ifdef FOXCONN_FPGA
static void diag_fxn_fpga_i2c_wr_data_fifo (int i2c_ctrl, uint32_t data_len,
                                            uchar *data_p)
{
    uint32_t ix, dword;

    /* Only bit 7:0 is valid in data fifo reg */
    for (ix = 0; ix < data_len; ix++) {
        dword = (uint32_t)*data_p++;
        diag_fpga_reg_write(i2c_ctrl + FPGA_I2C_DATA_FIFO_REG, dword);
    }
}
#endif

static boolean diag_fpga_check_i2c_idle (int i2c_ctrl) 
{
    uint32_t ix, timeout_val, retry;
    int data_in;

    timeout_val = FPGA_I2C_IDLE_TIMEOUT;
    
    for (retry = 0; retry < FPGA_I2C_IDLE_RETRY; retry++) {
        for (ix = 0; ix < timeout_val; ix++) {
            diag_fpga_reg_read(i2c_ctrl + FPGA_I2C_MASTER_STS_REG, &data_in);
            if (data_in & FPGA_I2C_STAT_NOT_ACTIVE) {
                return (TRUE);
            }
            msleep(10);
        }
        diag_fpga_i2c_reset(i2c_ctrl);
    }
    
    return (FALSE);
}

static void diag_fpga_i2c_reset (int i2c_ctrl)
{
    int ctr = 0;

    diag_fpga_reg_or(i2c_ctrl + FPGA_I2C_MASTER_CTRL_REG, FPGA_I2C_CTRL_SOFT_RESET);
    msleep(10);

    /* Goes into bitbang mode */
    diag_fpga_reg_or(i2c_ctrl + FPGA_I2C_MASTER_CTRL_REG, FPGA_I2C_CTRL_BITBANG);

    /* Drives SDA Lines low */
    diag_fpga_reg_nand(i2c_ctrl + FPGA_I2C_BITBANG_REG, I2C_BITBANG_SDA_DRIVER);

    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("Dumping I2C Register... To be Implemented...\n");
    }

    /* Keeps driving SCL until it recovers */
    for (ctr = 0; ctr < SCL_DRIVE_TIMES; ctr++) {
        diag_fpga_reg_nand(i2c_ctrl + FPGA_I2C_BITBANG_REG, 
                            I2C_BITBANG_SCL_DRIVER);
        msleep(1);
        diag_fpga_reg_or(i2c_ctrl + FPGA_I2C_BITBANG_REG, I2C_BITBANG_SCL_DRIVER);
        msleep(1);
    }

    /* Drives the SDA line high */
    diag_fpga_reg_or(i2c_ctrl + FPGA_I2C_BITBANG_REG, I2C_BITBANG_SDA_DRIVER);

    /* Leave bitbang mode */
    diag_fpga_reg_nand(i2c_ctrl + FPGA_I2C_MASTER_CTRL_REG, FPGA_I2C_CTRL_BITBANG);
}

static uint32_t i2c_dswap4 (int x)
{
    return dswap4(x);
}


/*---------------------------------------------------------------
$Log: diag_fpga_i2c.c,v $
Revision 1.2  2016/04/20 11:25:30  benchen2
add tachi fru portion

Revision 1.1.2.9  2016/01/14 08:16:45  benchen2
fix MB ACT2 issue

Revision 1.1.2.8  2015/12/16 01:55:53  huanngo
Add support for FPGA I2C device scan utility

Revision 1.1.2.7  2015/10/23 11:25:10  tirawan
Add 3 ms delay in I2C normal operation and return immediately if no ack is detected, for ACT2 programming issue

Revision 1.1.2.6  2015/09/18 06:51:02  tirawan
Fix I2C normal operation for FPGA I2C transaction

Revision 1.1.2.5  2015/09/17 13:04:34  tirawan
Support Cisco FPGA firmware upgrade

Revision 1.1.2.4  2015/08/30 05:57:35  tirawan
To support NIM ACT2 R/W access using TAM library

Revision 1.1.2.3  2015/08/28 02:33:52  tirawan
To support ACT2 M/B cookie programming using Foxconn FPGA

Revision 1.1.2.2  2015/08/21 10:38:30  benchen2
Add foxconn FPGA I2C R/W Function

Revision 1.1.2.1  2015/07/24 03:39:36  tirawan
Add FPGA I2C read/write function, ACT2 cookie read/write function



$Endlog$
*/

