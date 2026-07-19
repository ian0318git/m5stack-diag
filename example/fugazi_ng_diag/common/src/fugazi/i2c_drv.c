/* $Id: i2c_drv.c,v 1.2 2021/06/02 08:22:35 iachang Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/fugazi/i2c_drv.c,v $
 *------------------------------------------------------------------
 *
 * FILE NAME: i2c_drv.c driver for i2c...port over from goofy
 *
 * June 2011 mcharon
 *
 * Copyright (c) 2011-2020 by Cisco Systems, Inc.
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
#include "dev_print.h"
#include "dev_object.h"
#include "common.h"
#include "common_utils.h"
#include "goofy_i2c.h"
#include "byteswap.h"
#include "i2c_api.h"
#include "linux_api.h"

static int err_no = 0;
static int i2c_status = 0;

#define MCHARON_DEBUG
extern unsigned char i2c_debug;

extern unsigned long dash_fpga;
static void gfy_wr_i2c_data_fifo(goofy_i2c_t *i2c, uint32_t data_len,
                                 uchar *data_p);
static int gfy_i2c_normal_op(goofy_i2c_t *, uint8_t, uint32_t, uint32_t,
                             uint32_t, uint32_t, uint32_t);
static int gfy_i2c_send_reg_offset (goofy_i2c_t *, uint32_t, uint32_t,
                                    uint32_t, uint32_t);
static int i2c_flush_fifo (goofy_i2c_t *i2c, int);


static reg_info_t goofy_i2c_reg_tbl[] = {
    {"I2C MASTER CONTROL",           0x0000, READ_WRITE, {BW_32BITS}, 0x001bffe0, 0x00000000},
    {"I2C MASTER SCRATCH",           0x0004, READ_WRITE, {BW_32BITS}, 0xffffffff, 0xfacedead},
    {"I2C MASTER STATUS",            0x0008, READ_ONLY,  {BW_32BITS}, 0x00000fff, 0x00000001},
    {"I2C MASTER STATUS MASK",       0x000c, READ_WRITE, {BW_32BITS}, 0x00000ffe, 0x00000ffe},
    {"I2C MASTER SLAVE ADDR",        0x0010, READ_WRITE, {BW_32BITS}, 0x000003ff, 0x00000000},
    {"I2C MASTER SLAVE SUB ADDR",    0x0014, READ_WRITE, {BW_32BITS}, 0x00ffffff, 0x00000000},
    {"end", 0x0000, 0, {0}, 0x0, 0x0},
};

#define SCL_DRIVE_TIMES   100

static uint32_t i2c_dswap4(int x)
{
     return dswap4(x);
}

/*****************************************************************
 * Function: gfy_i2c_master_reg_test
 *
 * Description: Test goofy I2C master registers
 *
 * Input: dev_object_t pointer to the Goofy
 *        i2c_num - the I2C master number (0 and up)
 *
 * Returns: PASS/FAIL
 */
int gfy_i2c_master_reg_test (goofy_i2c_t *i2c)
{
    ulong reg_addr;

    reg_addr = (ulong)i2c;

    return (register_tests(reg_addr, goofy_i2c_reg_tbl));
}

/*****************************************************************
 * Function: gfy_i2c_master_reg_display
 *
 * Description: Display goofy I2C master registers
 *
 * Input: dev_object_t pointer to the Goofy
 *        i2c_num - the I2C master number (0 and up)
 *
 * Returns: PASS/FAIL
 */
int gfy_i2c_master_reg_display (goofy_i2c_t *i2c)
{
    ulong reg_addr;

    reg_addr = (ulong)i2c;
    
    printf("\n Goofy I2C Register @%#.8lx:\n", reg_addr);
    
    if (register_display(reg_addr, goofy_i2c_reg_tbl) == FAIL) {
      return(FAIL);
    }
    return(PASS);
}

/**********************************************************************
 *
 * Function: gfy_init_dbgbus
 *
 * Description: Init the debug bus
 *
 * Input:  dev - the goofy chip dev_object_t instance
 *
 * Output:
 *
 */
void gfy_init_dbgbus (dev_object_t *dev)
{
    printf("\n\n****gfy_init_dbgbus not supported*****\n\n");
}

/**********************************/
/*****    I2C Master code     *****/
/**********************************/

/**********************************************************************
 *
 * Function: gfy_wr_i2c_reg
 *
 * Description: Write value into a I2C master register
 *
 * Input: i2c - pointer to goofy i2c master
 *        addr_offset - address offset to the register
 *        val - value written to the register
 *
 * Output: void
 */
void gfy_wr_i2c_reg (goofy_i2c_t *i2c, uint32_t addr_offset, uint32_t val)
{
    volatile uint32_t *reg_p;

    reg_p = (volatile uint32_t *)((ulong)i2c->i2c_control + addr_offset);
    *reg_p = val;
}


/**********************************************************************
 *
 * Function: gfy_display_i2c_reg
 *
 * Description: Show the i2c registers
 *
 * Input: i2c - pointer to goofy i2c master
 *
 * Output: void
 *
 */
int gfy_display_i2c_reg (goofy_i2c_t *i2c)
{
    printf("i2c_control @ %p = %#.8x\n", &i2c->i2c_control, i2c->i2c_control);
    printf("i2c_scratch @ %p = %#.8x\n", &i2c->i2c_scratch, i2c->i2c_scratch);
    printf("i2c_status @ %p = %#.8x\n", &i2c->i2c_status, i2c->i2c_status);
    printf("i2c_status_mask @ %p = %#.8x\n", &i2c->i2c_status_mask, i2c->i2c_status_mask);
    printf("i2c_slave_addr @ %p = %#.8x\n", &i2c->i2c_slave_addr, i2c->i2c_slave_addr);
    printf("i2c_slave_sub_addr @ %p = %#.8x\n", &i2c->i2c_slave_sub_addr, i2c->i2c_slave_sub_addr);

    return(PASS);
}

/**********************************************************************
 *
 * Function: gfy_chk_i2c_idle
 *
 * Description: Check if the i2c master is idle
 *
 * Input: i2c - pointer to goofy i2c master
 *
 * Output: TRUE or FALSE
 *
 */
boolean gfy_chk_i2c_idle(goofy_i2c_t *i2c)
{
    uint32_t i, timeout_val, retry;

    timeout_val = 30;//GFY_I2C_XFER_BIT_COUNT(2);
    for (retry = 0 ;  retry < 5; retry++) {
        for (i=0; i < timeout_val; i++) {
            if (i2c->i2c_status & MSK_GFY_I2C_STAT_NOT_ACTIVE) {
                return (TRUE);
            }
            msleep(10);
        }
        gfy_i2c_reset(i2c);
    }
    printf("i2c failure: data is still being trasferred. too long to complete.\n");
    print_offset_val("", dash_fpga, (ulong)&i2c->i2c_status, __LINE__, 0);
    return (FALSE);
}

/**********************************************************************
 *
 * Function: i2c_flush_fifo
 *
 * Description: flush out data inside fifo
 *
 * Input: i2c - pointer to goofy i2c master
 *         byte -- number of byte to flush
 * Output: TRUE or FALSE
 *
 */
static int i2c_flush_fifo (goofy_i2c_t *i2c, int byte)
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
 * Function: gfy_rd_i2c_data_fifo
 *
 * Description: Read data bytes from the i2c data fifo
 *
 * Input: i2c - pointer to goofy i2c master
 *        data_len - The number of bytes to xfer
 *        data_p - pointer to the buffer holding the data being xfer
 *
 * Output: void
 */
void gfy_rd_i2c_data_fifo(goofy_i2c_t *i2c, uint32_t data_len,
                          uchar *data_p)
{
    uint32_t i, j, word_count, byte_count;
    uint32_t dword;
    uchar *byte_p;

#ifdef DEBUG
    printf("\nDBG: goofy i2c data fifo read\n");
    printf("i2c->i2c_byte_count %#.8x\n", i2c->i2c_byte_count);
    printf("i2c->i2c_data_fifo_rw_ptr %#.8x\n", i2c->i2c_data_fifo_rw_ptr);
#endif

    word_count = data_len / 4;
    byte_count = data_len % 4;

    for (i=0; i < word_count; i++) {
	dword = i2c_dswap4(i2c->i2c_data_fifo);
        byte_p = (uchar *)&dword;
        for (j=0; j < 4; j++) {
	    *data_p++ = *byte_p++;
	}
#ifdef MCHARON_DEBUG
        if (i2c_debug) {
            print_offset("reading from data fifo @", dash_fpga, (unsigned long)&i2c->i2c_data_fifo, __LINE__, 0);
            printf("= %#x; word %d ;\n ", (dword), i);
        }
#endif
    }

    if (byte_count > 0) {
	dword = i2c_dswap4(i2c->i2c_data_fifo);
	byte_p = (uchar *)&dword;
	for (i=0; i < byte_count; i++) {
	    *data_p++ = *byte_p++;
	}
#ifdef MCHARON_DEBUG
        if (i2c_debug) {
            print_offset("reading last byte from data fifo @", dash_fpga,
                         (unsigned long)&i2c->i2c_data_fifo, __LINE__, 0);
            printf("= %#x; bytesh %d ;\n ", (dword), byte_count);
        }
#endif

    }

    /* flush again just to be safe */
    i2c_flush_fifo(i2c, data_len);

}

/**********************************************************************
 *
 * Function: gfy_wr_i2c_data_fifo
 *
 * Description: Write data bytes to the i2c data fifo
 *
 * Input: i2c - pointer to goofy i2c master
 *        data_len - The number of bytes to xfer
 *        data_p - pointer to the buffer holding the data being xfer
 *
 * Output: void
 *
 */
void gfy_wr_i2c_data_fifo(goofy_i2c_t *i2c, uint32_t data_len,
		                  uchar *data_p)
{
    uint32_t i, j, word_count, byte_count;
    uint32_t dword;
    uchar *byte_p;

    /*ZZZ */
    /* do i need to read until i get an underflow condition to
       make sure fifo is empty before writing to fifo? */

    word_count = data_len / 4;
    byte_count = data_len % 4;

#ifdef MCHARON_DEBUG
    if (i2c_debug) {
        printf("word %d ; byte %d; line %d\n", word_count, byte_count, __LINE__);
    }
#endif

    for (i=0; i < word_count; i++) {
        byte_p = (uchar *)&dword;
        for (j=0; j < 4; j++) {
	    *byte_p++ = *data_p++;
	}
	i2c->i2c_data_fifo = i2c_dswap4(dword);
#ifdef MCHARON_DEBUG
        if (i2c_debug) {
            print_offset("writing to data fifo @", dash_fpga,
                         (unsigned long)&i2c->i2c_data_fifo, __LINE__, 0);
            printf("= 0x%08x; word %d \n; ", i2c_dswap4(dword), i);
        }
#endif
    }

    if (byte_count) {
        dword = 0;
        byte_p = (uchar *)&dword;
        for (i=0; i < byte_count; i++) {
            *byte_p++ = *data_p++;
        }
        i2c->i2c_data_fifo = i2c_dswap4(dword);
#ifdef MCHARON_DEBUG
        if (i2c_debug) {
            print_offset("writing last byte to data fifo @", dash_fpga,
                         (unsigned long)&i2c->i2c_data_fifo, __LINE__, 0);
            printf("= 0x%08x; bytes %d ;\n ", i2c_dswap4(dword), byte_count);
        }
#endif
    }

}

/**********************************************************************
 *
 * Function: gfy_i2c_normal_op
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
 */
int gfy_i2c_normal_op (goofy_i2c_t *i2c, uint8_t mux,
                       uint32_t slv_addr, uint32_t data_len,
                       uint32_t sub_addr_sz, uint32_t reg_addr,
                       uint32_t rd_wr_mode)
{
    uint32_t reg_val, i, timeout_val, temp_val;
    
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
    
#ifdef MCHARON_DEBUG
    if (i2c_debug) {
        printf("i2c_drv.c len %d...; line%d\n", data_len, __LINE__); /* 0x0 0x 1 0x1 0 0 */
    }

#endif
    /* write mode end */
    /* Enable the normal operation */
    i2c->i2c_slave_addr = slv_addr;

    if (sub_addr_sz != 0) {
        i2c->i2c_slave_sub_addr = reg_addr;
#ifdef MCHARON_DEBUG
        if (i2c_debug) {
            printf("i2c drv.c i2c_slave_sub_addr %#x\n", i2c->i2c_slave_sub_addr);
        }
#endif
    }

    i2c->i2c_control = (reg_val | GFY_I2C_CTRL_NORMAL | (sub_addr_sz) << 24);


#ifdef MCHARON_DEBUG
    if (i2c_debug) {
        print_offset("i2c_control @", dash_fpga,
                     (unsigned long)&i2c->i2c_control, __LINE__, 0);
        printf("= %#x, i2c_slav_addr %#x; \n",
               i2c->i2c_control,
               i2c->i2c_slave_addr);
    }

#endif
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
    for (i=0; i <= timeout_val; i++) {
        temp_val = reg_val = i2c->i2c_status;
        if (temp_val & MSK_GFY_I2C_STAT_NO_SLV) { /*check bit 2*/
            err_no = (RC_I2C_SLV_NACK);
            i2c_status = reg_val;
            printf("\n\n");
            printf("device (i2c addr 0x%x) is not acknowledging; is it installed? "
                   " [i2c status @%#x=%#x %d]\n", i2c->i2c_slave_addr,
                   (uint)((ulong)&i2c->i2c_status - (ulong)dash_fpga), reg_val,
                   err_no);
            return (RC_I2C_SLV_NACK);
        } else {

        }

        /* if slave device does not answer, return busy status bit 4A */
        if ((reg_val & MSK_GFY_I2C_STAT_STD_DONE) != 0) {
	    break;
	}
        msleep(8);
    }
    if (i > timeout_val) {
        err_no = (RC_I2C_TIMEOUT);
        i2c_status = reg_val;
        printf("\n\ndone bit of device (i2c addr 0x%x) is not set. "
               " [i2c status @%#x=%#x %d]\n", i2c->i2c_slave_addr,
               (uint)((ulong)&i2c->i2c_status - (ulong)dash_fpga), reg_val,
               err_no);
        return (RC_I2C_TIMEOUT);
    }
    
    return (RC_I2C_OP_OK);
}

/**********************************************************************
 *
 * Function: gfy_i2c_reset
 *
 * Description: Reset an I2C master module
 *
 * Input: i2c - pointer to goofy i2c master
 *
 * Output: PASSED or FAILED
 */
void gfy_i2c_reset (goofy_i2c_t *i2c)
{
    int ctr = 0;

    i2c->i2c_control |= GFY_I2C_CTRL_SOFT_RESET;
    usleep(1000);

    /* goes into bitbang mode */
    i2c->i2c_control |= GFY_I2C_CTRL_BITBANG; 

    /* drives the SDA lines low */
    i2c->bb &= ~(I2C_BITBANG_SDA_DRIVER);

    /* keeps driving SCL until it recovers */
    for (ctr = 0; ctr < SCL_DRIVE_TIMES; ctr++) {
       i2c->bb &= ~(I2C_BITBANG_SCL_DRIVER);
       msleep(1);
       i2c->bb |= I2C_BITBANG_SCL_DRIVER;
       msleep(1); 
    }

    /* drives the SDA lines High */
    i2c->bb |= I2C_BITBANG_SDA_DRIVER;

    /* leave bitbang mode */
    i2c->i2c_control &= ~(GFY_I2C_CTRL_BITBANG); 

    return;
}

static int gfy_i2c_send_reg_offset (goofy_i2c_t *i2c, uint32_t mux, uint32_t slv_addr,
                                    uint32_t reg_addr, uint32_t sub_addr_sz)
{
    uint32_t rc;
    uint32_t addr_size = 1;
    
    /* write 1 byte reg offset into data fifo */
    gfy_wr_i2c_data_fifo(i2c, addr_size, (unsigned char *)
                         &reg_addr);

    /* initializte write transaction to send off set onto the bus */
    rc = (gfy_i2c_normal_op(i2c, mux, slv_addr, addr_size,
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
 * Function: gfy_i2c_rd
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
 */
int gfy_i2c_rd (goofy_i2c_t *i2c, uint8_t mux, uint32_t slv_addr, 
	            int32_t reg_addr, uint32_t sub_addr_sz,
                uint32_t data_len, uchar *data_buf)
{
    int rc;
    if (gfy_chk_i2c_idle(i2c) == FALSE) {
        return(RC_I2C_BUSY);
    }
#ifdef MCHARON_DEBUG
    if (i2c_debug) { 
        printf("READING %d:\n", data_len);
    }
#endif
    /* if required, send reg offset to slave and flush fifo */
    if ((reg_addr >= 0) && (sub_addr_sz == 0)) {
        rc = gfy_i2c_send_reg_offset(i2c, mux, slv_addr, reg_addr, sub_addr_sz);
        if (rc != RC_I2C_OP_OK) {
            return rc;
        }
    }

    /* send read request to the slave */
    rc = (gfy_i2c_normal_op(i2c, mux, slv_addr, data_len, sub_addr_sz, reg_addr,
                            GFY_I2C_CTRL_RD_MODE));
    if (rc != RC_I2C_OP_OK) {
        return rc;
    }

    /* read data from fifo and flush */
    gfy_rd_i2c_data_fifo(i2c, data_len, data_buf);

    return rc;
    
}

/* gfy_i2c_wr: send i2c reg offset and data into data fifo, then
   write to goofy control register to flush data on to the bus
   the first byte on the bus will be reg offset.
*/
int gfy_i2c_wr (goofy_i2c_t *i2c, uint8_t mux, uint32_t slv_addr,
            int32_t reg_offset, uint32_t sub_addr_size, uint32_t data_len,
            uchar *data_buf)
{
    unsigned int rc;
    unsigned char *buf = NULL;

    if (gfy_chk_i2c_idle(i2c) == FALSE) {
        return(RC_I2C_BUSY);
    }
    buf = malloc(data_len+sizeof(uint32_t));  /* we should allocate at least 4 bytes */
    memset(buf, 0, data_len+sizeof(uint32_t));
#ifdef MCHARON_DEBUG
    if (i2c_debug) {
        printf("WRITING:\n");
    }
#endif
    if (sub_addr_size != 0) {
        gfy_wr_i2c_data_fifo(i2c, data_len, data_buf);
    } else { 
        /* if required, send reg offset to slave device and flush fifo */
        if (reg_offset >= 0) {
#ifdef MCHARON_DEBUG
            if (i2c_debug) {
                printf("reg_offset is %d %d\n", reg_offset, __LINE__);
            }
#endif
            buf[0] = (unsigned char )reg_offset & 0xFF;
            data_len++;
            /*
            if (reg_offset & 0xFF00) {
                buf[1] = (unsigned char )((reg_offset & 0xFF00) >> 8);
                data_len++;
                memcpy(&buf[2], data_buf, data_len);
            } else {
                memcpy(&buf[1], data_buf, data_len);
            }
            */
            memcpy(&buf[1], data_buf, data_len);

        } else {
            /*  for smart devices (ie ACT2) that dont' want address to be sent */
            /* here we send data only ...no address */
            memcpy(&buf[0], data_buf, data_len);
#ifdef MCHARON_DEBUG
            if (i2c_debug) {
                printf("i2c_drv.c: len=%d xx %#x %#x %#x %#x; offset=%d %d; \n", data_len, buf[0], buf[1], buf[2],
                       buf[3],reg_offset, __LINE__);
            }
#endif

        }
        gfy_wr_i2c_data_fifo(i2c, data_len, buf);
    }

    free(buf);

#ifdef MCHARON_DEBUG
    if (i2c_debug) {
        printf("gfy_i2c_wr slave %#x mux %d ; line %d\n", slv_addr, mux , __LINE__);
    }
 
#endif

    /* write data to fifo ...(is it safe to flush fifo here, or do it
       later after we send out data to slave?) */

    /* initialiate write transaction */
    rc = (gfy_i2c_normal_op(i2c, mux, slv_addr, data_len,
                            sub_addr_size, reg_offset, 
                            GFY_I2C_CTRL_WR_MODE));

    return rc;
}

int i2c_err_no (uint32_t *status)
{
    *status = i2c_status;
    return err_no;
}

/* end of file */

/*-------------------------------------------------
 * $Log: i2c_drv.c,v $
 * Revision 1.2  2021/06/02 08:22:35  iachang
 * CSCvo59196-33 : Merge Fugazi from ASR1K-main-branch to ISR main trunk
 *
 * Revision 1.1.8.2  2020/08/26 02:37:49  iachang
 * Merge Fugazi code into main trunk
 *
 * Revision 1.1.6.3  2020/08/04 08:37:06  iachang
 * Update Copyright to 2020
 *
 * Revision 1.1.6.2  2019/03/14 03:48:36  letsai
 * Initial check in.
 *
 *
 *
 * $Endlog$
 * */
