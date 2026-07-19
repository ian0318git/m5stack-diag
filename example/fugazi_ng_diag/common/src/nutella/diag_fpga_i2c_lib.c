/* $Id: diag_fpga_i2c_lib.c,v 1.4 2019/07/11 12:31:27 alicehua Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/nutella/diag_fpga_i2c_lib.c,v $
 *------------------------------------------------------------------
 *
 * FILE NAME: diag_fpga_i2c_lib
 *
 * June 2011 mcharon
 *
 * Copyright (c) 2011-2019 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */
#include <stdlib.h>
#include <string.h>
#include "endians.h"
#include "types.h"
#include "proto.h"
#include "free.h"
#include "defs.h"
#include "error.h"
#include "nvmonvars.h"
#include "queryflags.h"
#include "dev_object.h"
#include "common.h"
#include "common_utils.h"
#include "diag_fpga_i2c_lib.h"
#include "byteswap.h"
#include "i2c_api.h"
#include "linux_api.h"
#include "diag_fpga.h"
#include "diag_i2c_addr.h"
#include "dash_fpga.h"


static int err_no = 0;
static int i2c_status = 0;

int fpga_i2c_scan_addr(int);
void fpga_i2c_reset(void);
static uint32_t i2c_dswap4(int);
static void fpga_wr_i2c_data_fifo(fpga_i2c_t *i2c, uint32_t data_len,
                                  uchar *data_p);
static int fpga_i2c_normal_op(fpga_i2c_t *, uint8_t, uint32_t, uint32_t,
                              uint32_t, uint32_t, uint32_t);
static int fpga_i2c_send_reg_offset(fpga_i2c_t *, uint32_t, uint32_t,
                                    uint32_t, uint32_t);
static int i2c_flush_fifo(fpga_i2c_t *i2c, int);


/*******************************************************************************
 *
 * Function   : i2c_dswap4
 *
 * Description: This function swap 4 bytes and return value
 *
 * Inputs     : value 
 *
 * Outputs    : return value 
 *
 *******************************************************************************
 */
static uint32_t i2c_dswap4 (int value)
{
     return (dswap4(value));
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
    uint32_t      ret_val = FAILED, ctr = 0;
    uchar         d32[80];

    memset(&i2c_if, 0, sizeof(i2c_if));


    /* Get I2C controller & MUX number that you want to scan */
    i2c_if.i2c_ctrl = 0;
    i2c_if.mux = 0;
    i2c_if.offset = 0;
    i2c_if.size = 1;

    memset(d32, 0, sizeof(d32));
    i2c_if.buf = (char *)d32;

    /* Now just have AIKIDO FPGA on FPGA I2C bus */
    i2c_if.i2c_dev = MB_I2C_ADDR_AIKIDO_ACT2;
    ret_val = FAILED;

    /* Read I2C device Register 0 */
    ret_val = fpga_i2c_read(&i2c_if);
    if (ret_val == PASSED) {
        printf("\n Find ACT2 on the FPGA I2C Bus:0x%x Address:0x%x.\n", i2c_if.i2c_ctrl, i2c_if.i2c_dev);
        ctr++;
    }

    if (ctr == 0) {
        printf("\n Find nothing on the FPGA I2C Bus.");
    }

    return (ret_val);
}


/*********************************************************************
 *
 * Function:    fpga_i2c_read
 *
 * Description: N2G Generic I2C Read API.
 *
 * Inputs:      i2c_p   - Pointer to the N2G I2C API interface struct. Fields
 *                        needed in the struct are:
 *                        i2c_bus_type, i2c_dev, offset, size, *buf.
 *
 * Outputs:     PASSED - No errors encounterd.
 *              E_I2C_INV_DEV - Invalid device address.
 *              E_I2C_NOT_LOCKED - Device not locked by any process.
 *              E_I2C_LOCKED - Device is locked by another process.
 *              E_I2C_MUX_BUSY - Other device on 1:4 Mux is busy.
 *              Other return codes are provided by the lower device driver.
 *
 * Assumptions:
 *
 *********************************************************************
 */
uint32_t fpga_i2c_read (n2g_i2c_if_t *i2c_p)
{
    fpga_i2c_t i2c;
    uint32_t rc = 0;

    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("i2c_api.c n2g_i2c_read: %d: IOFGPA_I2C\n",  __LINE__);
        printf("i2c_dev %#x, rd_hd_size %d, offset %#x, size %#x \n\n",
               i2c_p->i2c_dev, i2c_p->rd_hd_size,
               i2c_p->offset, i2c_p->size);
    }

    rc = fpga_i2c_rd(&i2c, i2c_p->mux, i2c_p->i2c_dev,
                     i2c_p->offset,
                     i2c_p->sub_addr_len,
                     i2c_p->size,
                     (unsigned char *) i2c_p->buf);

    if ((NVRAM)->diagflag & D_VERBOSE) { 
        printf("i2c_if_p->i2c_ctrl %d (%s())\n", i2c_p->i2c_ctrl, __FUNCTION__);
        printf("i2c_if_p->mux %d\n", i2c_p->mux);
        printf("i2c_if_p->i2c_dev 0x%X\n", i2c_p->i2c_dev);
        printf("i2c_if_p->offset 0x%X\n", i2c_p->offset);
        printf("i2c_if_p->buf 0x%X\n", *i2c_p->buf);
    }

    /* According to I2C specification, tBUF - "bus free time between a STOP and
     * START condition" is 4.7 us minimum for Standard-mode
     */
    usleep(I2C_BUS_FREE_TIME);
    return (rc);
}

/*********************************************************************
 *
 * Function:    fpga_i2c_write
 *
 * Description: FPGA I2C Write API.
 *
 * Inputs:      i2c_p   - Pointer to the N2G I2C API interface struct. Fields
 *                        needed in the struct are:
 *                        i2c_bus_type, i2c_dev, offset, size, *buf.
 *
 * Outputs:     PASSED - No errors encounterd.
 *              E_I2C_INV_DEV - Invalid device address.
 *              E_I2C_NOT_LOCKED - Device not locked by any process.
 *              E_I2C_LOCKED - Device is locked by another process.
 *              E_I2C_MUX_BUSY - Other device on 1:4 Mux is busy.
 *              Other return codes are provided by the lower device driver.
 *
 * Assumptions:
 *
 *********************************************************************
 */
uint32_t fpga_i2c_write(n2g_i2c_if_t *i2c_p)
{   
    fpga_i2c_t i2c;
    uint rc;

    rc = fpga_i2c_wr(&i2c, i2c_p->mux, i2c_p->i2c_dev,
                     i2c_p->offset,
                     i2c_p->sub_addr_len,
                     i2c_p->size,
                     (unsigned char *)i2c_p->buf);
    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("i2c_if_p->i2c_ctrl %d (%s())\n", i2c_p->i2c_ctrl, __FUNCTION__);
        printf("i2c_if_p->mux %d\n", i2c_p->mux);
        printf("i2c_if_p->i2c_dev 0x%X\n", i2c_p->i2c_dev);
        printf("i2c_if_p->i2c_offset 0x%X\n", i2c_p->offset);
        printf("i2c_if_p->buf 0x%X (%s())\n", *i2c_p->buf, __FUNCTION__);
    }


    /* According to I2C specification, tBUF - "bus free time between a STOP and
     * START condition" is 4.7 us minimum for Standard-mode
     */
    wastetime(I2C_BUS_FREE_TIME);
    return (rc);
}

/**********************************************************************
 *
 * Function: fpga_chk_i2c_idle
 *
 * Description: Check if the i2c master is idle
 *
 * Input: i2c - pointer to goofy i2c master
 *
 * Output: PASSED or FAILED
 *
 *****************************************************************
 */
boolean fpga_chk_i2c_idle (fpga_i2c_t *i2c)
{
    uint32_t ix, timeout_val, retry, read_buf;

    timeout_val = I2C_IDLE_TIMEOUT_VAL;
    for (retry = 0 ;  retry < I2C_IDLE_TIMEOUT_RETRY; retry++) {
        for (ix = 0; ix < timeout_val; ix++) {
            fpga_read_reg(FPGA_I2C_STAT_REG, &read_buf);
            i2c->i2c_status = read_buf;
            if (i2c->i2c_status & MSK_GFY_I2C_STAT_NOT_ACTIVE) {
                return (PASSED);
            }
            msleep(10);
        }
        fpga_i2c_reset();
    }
    printf("i2c failure: data is still being trasferred. too long to complete.\n");
    print_offset_val("", fpga_ptr, (ulong)&i2c->i2c_status, __LINE__, 0);
    return (FAILED);
}

/**********************************************************************
 *
 * Function: i2c_flush_fifo
 *
 * Description: flush out data inside fifo
 *
 * Input: i2c - pointer to goofy i2c master
 *        byte -- number of byte to flush
 * Output: RC_I2C_OP_OK for 0
 *
 *****************************************************************
 */
static int i2c_flush_fifo (fpga_i2c_t *i2c, int byte)
{
    unsigned int tmp;
    
    while (byte--) {
        tmp = i2c->i2c_data_fifo;
        tmp++;
    }
    /* do it a couple of more times just to be safe */
    tmp = i2c->i2c_data_fifo;
    tmp = i2c->i2c_data_fifo;

    return (RC_I2C_OP_OK);

}

/**********************************************************************
 *
 * Function: fpga_rd_i2c_data_fifo
 *
 * Description: Read data bytes from the i2c data fifo
 *
 * Input: i2c - pointer to goofy i2c master
 *        data_len - The number of bytes to xfer
 *        data_p - pointer to the buffer holding the data being xfer
 *
 * Output: void
 *****************************************************************
 */
void fpga_rd_i2c_data_fifo(fpga_i2c_t *i2c, uint32_t data_len,
		                   uchar *data_p)
{
    uint32_t ix, jx, word_count, byte_count;
    uint32_t dword, read_buf;
    uchar *byte_p;

    word_count = data_len / 4;
    byte_count = data_len % 4;

    for (ix = 0; ix < word_count; ix++) {
        fpga_read_reg(FPGA_I2C_DATA_FIFO_REG, &read_buf);
        i2c->i2c_data_fifo = read_buf;
	    dword = i2c_dswap4(i2c->i2c_data_fifo);
        byte_p = (uchar *)&dword;
        for (jx = 0; jx < 4; jx++) {
	        *data_p++ = *byte_p++;
	    }

        if ((NVRAM)->diagflag & D_VERBOSE) {
            print_offset("reading from data fifo @", fpga_ptr, 
                         (unsigned long)&i2c->i2c_data_fifo, __LINE__, 0);
            printf("= %#x; word %d ;\n ", (dword), ix);
        }
    }

    if (byte_count > 0) {
        fpga_read_reg(FPGA_I2C_DATA_FIFO_REG, &read_buf);
        i2c->i2c_data_fifo = read_buf;
	    dword = i2c_dswap4(i2c->i2c_data_fifo);
	    byte_p = (uchar *)&dword;
	    for (ix = 0; ix < byte_count; ix++) {
	        *data_p++ = *byte_p++;
	    }

        if ((NVRAM)->diagflag & D_VERBOSE) {
            print_offset("reading last byte from data fifo @", fpga_ptr,
                         (unsigned long)&i2c->i2c_data_fifo, __LINE__, 0);
            printf("= %#x; bytesh %d ;\n ", (dword), byte_count);
        }
    }

    /* flush again just to be safe */
    i2c_flush_fifo(i2c, data_len);

}

/**********************************************************************
 *
 * Function: fpga_wr_i2c_data_fifo
 *
 * Description: Write data bytes to the i2c data fifo
 *
 * Input: i2c - pointer to goofy i2c master
 *        data_len - The number of bytes to xfer
 *        data_p - pointer to the buffer holding the data being xfer
 *
 * Output: void
 *
 *****************************************************************
 */
void fpga_wr_i2c_data_fifo (fpga_i2c_t *i2c, uint32_t data_len,
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
        printf("word %d ; byte %d; line %d\n", word_count, byte_count, __LINE__);
    }

    for (ix = 0; ix < word_count; ix++) {
        byte_p = (uchar *)&dword;
        for (jx = 0; jx < 4; jx++) {
	        *byte_p++ = *data_p++;
	    }
	    i2c->i2c_data_fifo = i2c_dswap4(dword);
        fpga_write_reg(FPGA_I2C_DATA_FIFO_REG, i2c->i2c_data_fifo);

        if ((NVRAM)->diagflag & D_VERBOSE) {
            print_offset("writing to data fifo @", fpga_ptr,
                         (unsigned long)&i2c->i2c_data_fifo, __LINE__, 0);
            printf("= 0x%08x; word %d \n; ", i2c_dswap4(dword), ix);
        }
    }

    if (byte_count) {
        dword = 0;
        byte_p = (uchar *)&dword;
        for (ix = 0; ix < byte_count; ix++) {
            *byte_p++ = *data_p++;
        }
        i2c->i2c_data_fifo = i2c_dswap4(dword);
        fpga_write_reg(FPGA_I2C_DATA_FIFO_REG, i2c->i2c_data_fifo);

        if ((NVRAM)->diagflag & D_VERBOSE) {
            print_offset("writing last byte to data fifo @", fpga_ptr,
                         (unsigned long)&i2c->i2c_data_fifo, __LINE__, 0);
            printf("= 0x%08x; bytes %d ;\n ", i2c_dswap4(dword), byte_count);
        }
    }

}

/**********************************************************************
 *
 * Function: fpga_i2c_normal_op
 *
 * Description: Perform standard I2C read or write operation on the
 *              I2C slave device
 *
 * Input: i2c - pointer to goofy i2c reg
 *        mux - mux port
 *        slv_addr - The i2c slave's address on the i2c bus
 *        rd_wr_mode - Read or write
 *        sub_addr_sz - byte size of the reg_addr (0 to 3 bytes)
 *        reg_addr - device register address
 *        data_len - The number of bytes to be xfer
 *        data_buf - The buffer to store the data being xfer
 *
 * Output: rc = RC_I2C_OP_OK, RC_I2C_BUSY, RC_I2C_SLV_NACK or RC_I2C_TIMEOUT.
 *
 *****************************************************************
 */
int fpga_i2c_normal_op (fpga_i2c_t *i2c, uint8_t mux,
                        uint32_t slv_addr, uint32_t data_len,
                        uint32_t sub_addr_sz, uint32_t reg_addr,
                        uint32_t rd_wr_mode)
{
    uint32_t reg_val, ix, timeout_val, temp_val, read_buf;
    
    err_no = 0;
    i2c_status = 0;

    /* Set up control register and the slave address register */
    reg_val = GFY_I2C_CTRL_CLK_50 | GFY_I2C_CTRL_SLV_ADDR_7 |
              GFY_I2C_CTRL_SPEED_NORMAL_100 | rd_wr_mode |
              (data_len << L_SHFT_GFY_I2C_CTRL_BYTE_LEN) |
              (mux << L_SHFT_GFY_I2C_CTRL_MUX);

    if (mux >= 4) {
        assert(!"mu3;.cxx has to be less than 4");
    }
    
    if ((NVRAM)->diagflag & D_VERBOSE) {
        /* 0x0 0x 1 0x1 0 0 */
        printf("i2c_drv.c len %d...; line%d\n", data_len, __LINE__);     
    }
    /* write mode end */
    /* Enable the normal operation */
    i2c->i2c_slave_addr = slv_addr;
    fpga_write_reg(FPGA_I2C_SLA_ADDR_REG, i2c->i2c_slave_addr);

    if (sub_addr_sz != 0) {
        i2c->i2c_slave_sub_addr = reg_addr;
        fpga_write_reg(FPGA_I2C_SLA_SUBADDR_REG, i2c->i2c_slave_sub_addr);

        if ((NVRAM)->diagflag & D_VERBOSE) {
            printf("i2c drv.c i2c_slave_sub_addr %#x\n", i2c->i2c_slave_sub_addr);
        }
    }

    i2c->i2c_control = (reg_val | GFY_I2C_CTRL_NORMAL | (sub_addr_sz) << 24);
    fpga_write_reg(FPGA_I2C_CTL_REG, i2c->i2c_control);


    if ((NVRAM)->diagflag & D_VERBOSE) {
        print_offset("i2c_control @", fpga_ptr,
                     (unsigned long)&i2c->i2c_control, __LINE__, 0);
        printf("= %#x, i2c_slav_addr %#x; \n",
               i2c->i2c_control,
               i2c->i2c_slave_addr);
    }
    /* give time for device to send acknowlegement..especiall when talking to quack */
    /* wait = (data_len * 10); */ /* defined but not used, removed. */
    /*10 byte address @ 100Khz */
    msleep(3);

    /* Monitor the done bit in status register. Add 10 satety bytes for 
     * wait time calculation due to I2C protocol is slow and have gaps
     */
    timeout_val = 500;

    /* Wait one byte time to let the i2c op to start before polling status */
    /* if no delay we might miss the no ack */
    for (ix = 0; ix <= timeout_val; ix++) {
        fpga_read_reg(FPGA_I2C_STAT_REG, &read_buf);
        i2c->i2c_status = read_buf;
        temp_val = reg_val = i2c->i2c_status;
        if (temp_val & MSK_GFY_I2C_STAT_NO_SLV) { /*check bit 2*/
            err_no = (RC_I2C_SLV_NACK);
            i2c_status = reg_val;
            printf("\n\n");
            printf("device shown below is not acknowledging; is it installed? "
                   " [i2c status @%#x=%#x %d]\n",
                   (uint)((ulong)&i2c->i2c_status - (ulong)fpga_ptr), reg_val,
                   err_no);
            return (RC_I2C_SLV_NACK);
        }

        /* if slave device does not answer, return busy status bit 4A */
        if ((reg_val & MSK_GFY_I2C_STAT_STD_DONE) != 0) {
	        break;
	    }
        msleep(8);
    }
    if (ix > timeout_val) {
        err_no = (RC_I2C_TIMEOUT);
        i2c_status = reg_val;
        printf("\n\ndone bit of device shown below is not set. "
               " [i2c status @%#x=%#x %d]\n",
               (uint)((ulong)&i2c->i2c_status - (ulong)fpga_ptr), reg_val,
               err_no);
        return (RC_I2C_TIMEOUT);
    }
    
    return (RC_I2C_OP_OK);
}

/**********************************************************************
 *
 * Function: fpga_i2c_reset
 *
 * Description: Reset an I2C master module
 *
 * Input: i2c - pointer to goofy i2c master
 *
 * Output: PASSED or FAILED
 *****************************************************************
 */
void fpga_i2c_reset (void)
{
    int ctr = 0;
    fpga_i2c_t i2c;

    i2c.i2c_control |= GFY_I2C_CTRL_SOFT_RESET;
    fpga_write_reg(FPGA_I2C_CTL_REG, i2c.i2c_control);
    usleep(1000);

    /* goes into bitbang mode */
    i2c.i2c_control |= GFY_I2C_CTRL_BITBANG; 
    fpga_write_reg(FPGA_I2C_CTL_REG, i2c.i2c_control);

    /* drives the SDA lines low */
    i2c.bb &= ~(I2C_BITBANG_SDA_DRIVER);
    fpga_write_reg(FPGA_I2C_BIT_BANG_REG, i2c.bb);

    if ((NVRAM)->diagflag & D_VERBOSE) {
        print_offset_val("", fpga_ptr, (ulong)i2c.bb, __LINE__, 0);
    }

    /* keeps driving SCL until it recovers */
    for (ctr = 0; ctr < SCL_DRIVE_TIMES; ctr++) {
       i2c.bb &= ~(I2C_BITBANG_SCL_DRIVER);
       fpga_write_reg(FPGA_I2C_BIT_BANG_REG, i2c.bb);
       msleep(1);
       i2c.bb |= I2C_BITBANG_SCL_DRIVER;
       fpga_write_reg(FPGA_I2C_BIT_BANG_REG, i2c.bb);
       msleep(1); 
    }

    /* drives the SDA lines High */
    i2c.bb |= I2C_BITBANG_SDA_DRIVER;
    fpga_write_reg(FPGA_I2C_BIT_BANG_REG, i2c.bb);

    /* leave bitbang mode */
    i2c.i2c_control &= ~(GFY_I2C_CTRL_BITBANG); 
    fpga_write_reg(FPGA_I2C_CTL_REG, i2c.i2c_control);

    printf("\n Reset FPGA I2C master module done.\n");

}

/**********************************************************************
 *
 * Function: fpga_i2c_send_reg_offset
 *
 * Description: 
 *
 * Input: i2c - pointer to goofy i2c master
 *
 * Output: PASSED or FAILED
 *****************************************************************
 */
static int fpga_i2c_send_reg_offset (fpga_i2c_t *i2c, uint32_t mux, 
                                     uint32_t slv_addr,uint32_t reg_addr,
                                     uint32_t sub_addr_sz)
{
    uint32_t rc;
    uint32_t addr_size = 1;
    
    /* write 1 byte reg offset into data fifo */
    fpga_wr_i2c_data_fifo(i2c, addr_size, (unsigned char *)
                          &reg_addr);

    /* initializte write transaction to send off set onto the bus */
    rc = (fpga_i2c_normal_op(i2c, mux, slv_addr, addr_size,
                             sub_addr_sz, reg_addr,
                             GFY_I2C_CTRL_WR_MODE));

    if (rc != RC_I2C_OP_OK) {
        if ((NVRAM)->diagflag & D_VERBOSE) {
            printf("i2c_drv.c sending offfset failed %d; line %d\n", rc, __LINE__);
        }
    }

    return rc;

}
/**********************************************************************
 *
 * Function: fpga_i2c_rd
 *
 * Description: Read data from the I2C slave device to the data buffer
 *              in normal I2C mode
 *
 * Input: i2c - pointer to goofy i2c master
 *        slv_addr - The i2c slave's address on the i2c bus
 *        sub_addr_sz - byte size of the reg_addr (0 to 3 bytes)
 *        reg_addr - device register address
 *        data_len - The number of bytes to be xfer
 *        data_buf - The buffer to store the data being xfer
 *
 * Output: rc = RC_I2C_OP_OK, RC_I2C_BUSY, RC_I2C_SLV_NACK or RC_I2C_TIMEOUT.
 *
 *****************************************************************
 */
int fpga_i2c_rd (fpga_i2c_t *i2c, uint8_t mux, uint32_t slv_addr, 
	             int32_t reg_addr, uint32_t sub_addr_sz,
                 uint32_t data_len, uchar *data_buf)
{
    int rc;
    if (fpga_chk_i2c_idle(i2c) == FAILED) {
        return(RC_I2C_BUSY);
    }
    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("READING %d:\n", data_len);
    }
    /* if required, send reg offset to slave and flush fifo */
    if ((reg_addr >= 0) && (sub_addr_sz == 0)) {
        rc = fpga_i2c_send_reg_offset(i2c, mux, slv_addr, reg_addr, sub_addr_sz);
        if (rc != RC_I2C_OP_OK) {
            return (rc);
        }
    }

    /* send read request to the slave */
    rc = (fpga_i2c_normal_op(i2c, mux, slv_addr, data_len, sub_addr_sz, reg_addr,
                             GFY_I2C_CTRL_RD_MODE));
    if (rc != RC_I2C_OP_OK) {
        return (rc);
    }

    /* read data from fifo and flush */
    fpga_rd_i2c_data_fifo(i2c, data_len, data_buf);

    return (rc);
    
}

/**********************************************************************
 *
 * Function: fpga_i2c_wr
 *
 * Description: send i2c reg offset and data into data fifo, then
 *              write to goofy control register to flush data on to the bus
 *              the first byte on the bus will be reg offset.
 *
 * Input: i2c - pointer to goofy i2c master
 *        slv_addr - The i2c slave's address on the i2c bus
 *        sub_addr_sz - byte size of the reg_addr (0 to 3 bytes)
 *        reg_addr - device register address
 *        data_len - The number of bytes to be xfer
 *        data_buf - The buffer to store the data being xfer
 *
 * Output: rc = RC_I2C_OP_OK, RC_I2C_BUSY, RC_I2C_SLV_NACK or RC_I2C_TIMEOUT.
 *
 *****************************************************************
 */
int fpga_i2c_wr (fpga_i2c_t *i2c, uint8_t mux, uint32_t slv_addr,
            int32_t reg_offset, uint32_t sub_addr_size, uint32_t data_len,
            uchar *data_buf)
{
    unsigned int rc;
    unsigned char *buf = NULL;

    if (fpga_chk_i2c_idle(i2c) == FAILED) {
        return(RC_I2C_BUSY);
    }
    buf = malloc(data_len+sizeof(uint32_t));  /* we should allocate at least 4 bytes */
    memset(buf, 0, data_len+sizeof(uint32_t));

    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("WRITING:\n");
    }

    if (sub_addr_size != 0) {
        fpga_wr_i2c_data_fifo(i2c, data_len, data_buf);
    } else { 
        /* if required, send reg offset to slave device and flush fifo */
        if (reg_offset >= 0) {

            if ((NVRAM)->diagflag & D_VERBOSE) {
                printf("reg_offset is %d %d\n", reg_offset, __LINE__);
            }
            buf[0] = (unsigned char )reg_offset & 0xFF;
            data_len++;
            memcpy(&buf[1], data_buf, data_len);

        } else {
            /*  for smart devices (ie ACT2) that dont' want address to be sent */
            /* here we send data only ...no address */
            memcpy(&buf[0], data_buf, data_len);

            if ((NVRAM)->diagflag & D_VERBOSE) {
                printf("i2c_drv.c: len=%d xx %#x %#x %#x %#x; offset=%d %d; \n",
                       data_len, buf[0], buf[1], buf[2],
                       buf[3],reg_offset, __LINE__);
            }

        }
        fpga_wr_i2c_data_fifo(i2c, data_len, buf);
    }

    free(buf);

    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("fpga_i2c_wr slave %#x mux %d ; line %d\n", slv_addr, mux , __LINE__);
    } 

    /* write data to fifo ...(is it safe to flush fifo here, or do it
       later after we send out data to slave?) */

    /* initialiate write transaction */
    rc = (fpga_i2c_normal_op(i2c, mux, slv_addr, data_len,
                             sub_addr_size, reg_offset, 
                             GFY_I2C_CTRL_WR_MODE));

    return rc;
}


/* end of file */

/******** History ******** 
$Log: diag_fpga_i2c_lib.c,v $
Revision 1.4  2019/07/11 12:31:27  alicehua
Collapse Nutella codes into main trunk.

$Endlog$
*/
