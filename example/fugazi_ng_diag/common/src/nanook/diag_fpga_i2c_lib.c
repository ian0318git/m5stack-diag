 /* $Id: diag_fpga_i2c_lib.c,v 1.2 2019/12/11 10:10:29 lucywang Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/nanook/diag_fpga_i2c_lib.c,v $
 *------------------------------------------------------------------
 *
 * FILE NAME: diag_fpga_i2c_lib
 *
 * June 2011 mcharon
 *
 * Copyright (c) 2019 by Cisco Systems, Inc.
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
#include "dash_fpga.h"
#include "diag_fpga_i2c_lib.h"
#include "byteswap.h"
#include "platform_i2c.h"
#include "i2c_api.h"
#include "linux_api.h"
#include "diag_i2c_addr.h"
#include "diag_i2c_lib.h"
#include "ngio.h"


extern unsigned long dash_fpga;
static void gfy_wr_i2c_data_fifo(fpga_i2c_t *i2c, uint32_t data_len,
                                 uchar *data_p);
static int gfy_i2c_normal_op(fpga_i2c_t *, uint8_t, uint32_t, uint32_t,
                             uint32_t, uint32_t, uint32_t);
static int gfy_i2c_send_reg_offset (fpga_i2c_t *, uint32_t, uint32_t,
                                    uint32_t, uint32_t);
static int i2c_flush_fifo (fpga_i2c_t *i2c, int);

int fpga_i2c_scan_addr(int);

static n2g_i2c_if_t wic_oir[MAX_WIC+FIRST_SLOT];
static char wic_oir_buf[MAX_WIC+FIRST_SLOT][256];

static uint8_t wic_i2c_ctrl[] = {0,  WIC1_I2C_CTRL, WIC2_I2C_CTRL, WIC3_I2C_CTRL};

static int err_no = 0;
static int i2c_status = 0;

unsigned char i2c_debug;

static n2g_i2c_if_t ngio_oir[] = {
    {   
        .dev_name = "OIR",
        .offset = 0,
        .i2c_bus_type = IOFPGA_I2C,
        .size    = sizeof(uint16_t),
        .sub_addr_len = 1,
        .mux = I2C_MUX_ZERO,
        .buf        = NULL,
    },
};


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

#if 0
/*-------------------------------------------------------------------
 *
 * Function: unreset_platform_in_dev
 *  sys level register; offset 0x8
 * 
 * unreset int devices on platform
 *
 * Input: bit; bit mask representig device to be reset
 * bit is defined as
 *  FPGA_IN_NIOS_RST              0x1000000 
 *  FPGA_IN_I2C_15_RST            0x8000
 *  .
 *  .
 *  .
 *  FPGA_IN_I2C_0_RST             0x0001
 *  OUTPUT: none
 *
 *-------------------------------------------------------------------
 */
void
unreset_platform_in_dev (int bit)
{
    assert(dash_fpga);
    sys_lvl_t *sys = (sys_lvl_t *)dash_fpga;
    sys->in_rst &= ~bit;

}

/*-------------------------------------------------------------------
 *
 * Function: get_platform_i2c_addr
 * Description: get i2c address
 *
 * Input: NONE
 *
 * Output: address
 *
 *-------------------------------------------------------------------
 */
unsigned long
get_platform_i2c_addr (int ctrl)
{
    unsigned long addr;
    assert(dash_fpga);

    addr = ((unsigned long)dash_fpga) + FPGA_I2C_BASE +
        (ctrl * FPGA_I2C_OFFSET);


    return addr;

}
#endif

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
        ret_val = fpga_i2c_read(&i2c_if);
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
    uint32_t rc = 0;
    unsigned long addr = 0;

    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("i2c_api.c n2g_i2c_read: %d: IOFGPA_I2C\n",  __LINE__);
        printf("i2c_dev %#x, rd_hd_size %d, offset %#x, size %#x \n\n",
               i2c_p->i2c_dev, i2c_p->rd_hd_size,
               i2c_p->offset, i2c_p->size);
    }

    addr = get_platform_i2c_addr(i2c_p->i2c_ctrl);

    rc = fpga_i2c_rd((fpga_i2c_t *)addr, i2c_p->mux, i2c_p->i2c_dev,
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
    uint rc;

    unsigned long addr = 0;

    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("i2c_api.c n2g_i2c_read: %d: IOFGPA_I2C\n",  __LINE__);
        printf("i2c_dev %#x, rd_hd_size %d, offset %#x, size %#x \n\n",
               i2c_p->i2c_dev, i2c_p->rd_hd_size,
               i2c_p->offset, i2c_p->size);
    }

    addr = get_platform_i2c_addr(i2c_p->i2c_ctrl);

    rc = fpga_i2c_wr((fpga_i2c_t *)addr, i2c_p->mux, i2c_p->i2c_dev,
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
 * Function: gfy_i2c_reset
 *
 * Description: Reset an I2C master module
 *
 * Input: i2c - pointer to goofy i2c master
 *
 * Output: PASSED or FAILED
 */
void
gfy_i2c_reset (fpga_i2c_t *i2c)
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
boolean
gfy_chk_i2c_idle(fpga_i2c_t *i2c)
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
static void
gfy_wr_i2c_data_fifo(fpga_i2c_t *i2c, uint32_t data_len,
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

static int
gfy_i2c_send_reg_offset (fpga_i2c_t *i2c, uint32_t mux, uint32_t slv_addr,
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
static int
gfy_i2c_normal_op (fpga_i2c_t *i2c, uint8_t mux,
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
    if (mux >= 4) {
        assert(!"mu3;.cxx has to be less than 4");
    }

    if (i2c_debug) {
        printf("i2c_drv.c len %d...; line%d\n", data_len, __LINE__); /* 0x0 0x 1 0x1 0 0 */
    }

    /* write mode end */
    /* Enable the normal operation */
    i2c->i2c_slave_addr = slv_addr;

    if (sub_addr_sz != 0) {
        i2c->i2c_slave_sub_addr = reg_addr;
        if (i2c_debug) {
            printf("i2c drv.c i2c_slave_sub_addr %#x\n", i2c->i2c_slave_sub_addr);
        }
    }

    i2c->i2c_control = (reg_val | GFY_I2C_CTRL_NORMAL | (sub_addr_sz) << 24);


    if (i2c_debug) {
        print_offset("i2c_control @", dash_fpga,
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
 * Function: i2c_flush_fifo
 *
 * Description: flush out data inside fifo
 *
 * Input: i2c - pointer to goofy i2c master
 *         byte -- number of byte to flush
 * Output: TRUE or FALSE
 *
 */
static int
i2c_flush_fifo (fpga_i2c_t *i2c, int byte)
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
*
 */
void
gfy_rd_i2c_data_fifo(fpga_i2c_t *i2c, uint32_t data_len,
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
    if (gfy_chk_i2c_idle(i2c) == FALSE) {
        return(RC_I2C_BUSY);
    }

    if (i2c_debug) {
        printf("READING %d:\n", data_len);
    }

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

/*******************************************************************************
 *
 * Function   : platform_get_wic_oir
 * Description: returns WIC OIR
 *
 * Inputs     : slot number
 *
 * Outputs    : pointer to OIR
 *
 *******************************************************************************
 */
void *
platform_get_wic_oir (int slot)
{   
    if (slot == 0) {
        assert(!"slot is 0");
    }
    memcpy(&wic_oir[slot], ngio_oir, sizeof(n2g_i2c_if_t));
    wic_oir[slot].buf = wic_oir_buf[slot];
    wic_oir[slot].i2c_ctrl = get_wic_i2c_ctrl(slot);
    wic_oir[slot].i2c_dev = NIM_I2C_ADDR_OIR;
    return (void *)&wic_oir[slot];
}

/*************************************************************************
 *
 * Function   : get_wic_i2c_ctrl (int slot)
 * Description: returns i2c address of wic i2c controller
 *
 * Inputs     : slot number
 *
 * Outputs    : i2c address
 *
 *************************************************************************
 */
uint8_t get_wic_i2c_ctrl (int slot)
{
    if (slot == 0) {
        assert(!"get_wic_i2c_ctrl");
    }
    return (wic_i2c_ctrl[slot]);

}

void *platform_get_carrier_wic_oir (int slot)
{   
    return 0;
}

uint8_t get_sm_i2c_ctrl (int slot)
{   
    return 0;
}

void *platform_get_sm_oir (int slot)
{   
    return 0;
}


void *platform_get_vm_oir (int slot)
{
    return 0;
}



/* end of file */

/******** History ******** 
$Log: diag_fpga_i2c_lib.c,v $
Revision 1.2  2019/12/11 10:10:29  lucywang
Merged Nanook to main trunk


$Endlog$
*/
