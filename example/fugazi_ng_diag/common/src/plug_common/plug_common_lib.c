/* $Id: plug_common_lib.c,v 1.4 2019/08/15 09:27:52 shjung Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/plug_common/plug_common_lib.c,v $
 *------------------------------------------------------------------
 *
 * plug_common_lib.c - Implement PLUGGABLE common lib Functions
 *
 * Copyright (c) 2015 - 2019 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "common.h"
#include "proto.h"
#include "types.h"
#include "nvmonvars.h"
#include "common_utils.h"
#include "cross_platform.h"
#include "endians.h"
#include "types.h"
#include "dev_object.h"     
#include "byteswap.h"
#include "linux_api.h"
#include "cookie_4.h"
#include "plug_common_host_impl.h"
#include "plug_testcard_host_impl.h"
#include "plug_testcard_phy.h"
#include "plug_host_fpga_lib.h"
#include "plug_common_lib.h"
#include "plug_slot.h"

uint32_t i2c_dswap4(int);
static int plug_fpga_i2c_flush_fifo(int, int);
void plug_fpga_rd_i2c_data_fifo(int, uint32_t, uchar *);
int plug_fpga_i2c_normal_op(int, uint8_t, uint32_t, uint32_t,
                                   uint32_t, uint32_t, uint32_t);
void plug_fpga_wr_i2c_data_fifo(int, uint32_t, uchar *);
int plug_fpga_i2c_send_reg_offset (int , uint32_t, uint32_t,
                                          uint32_t, uint32_t);
void plug_fpga_i2c_reset(int);
boolean plug_fpga_chk_i2c_idle(int i2c_addr);
int plug_common_fpga_i2c_ack_check(int, uint8_t, uint32_t, int32_t, 
                                    uint32_t, uint32_t, uchar *);
int plug_slot_get_bd_revision(uchar *, unsigned short *);
int plug_slot_get_pcb_serial(uchar *, char *);

int err_no = 0;
int i2c_status = 0;

plug_module_sku_info plug_module_sku_tbl[] = {
    {"PLUGGABLE_SGMII_TEST_CARD", PLUGGABLE_TEST_CARD},
    {"PLUGGABLE_PCIE_TEST_CARD",  PLUGGABLE_PCIE_TEST_CARD},
    {"PLUGGABLE_LTE_EM",      PLUGGABLE_LTE_EM},
    {"PLUGGABLE_LTE_WP7601",  PLUGGABLE_LTE_WP7601},
    {"PLUGGABLE_LTE_WP7603",  PLUGGABLE_LTE_WP7603},
    {"PLUGGABLE_LTE_WP7605",  PLUGGABLE_LTE_WP7605},
    {"PLUGGABLE_LTE_WP7607",  PLUGGABLE_LTE_WP7607},
    {"PLUGGABLE_LTE_WP7608",  PLUGGABLE_LTE_WP7608},
    {"PLUGGABLE_LTE_WP7609",  PLUGGABLE_LTE_WP7609},
    {"PLUGGABLE_LTE_WP7610",  PLUGGABLE_LTE_WP7610},
    {"PLUGGABLE_SERIAL",      PLUGGABLE_SERIAL},
    {"PLUGGABLE_LTE_TELIT_LM9x0", PLUGGABLE_LTE_TELIT_LM9x0},
    { NULL, 0x0000}
};


/*-----------------------------------------------------------------------------
 *
 * Function get_board_revision
 *
 * This function will return the REVISION number of the HWIC module.
 *
 * Inputs : hwic_num - HWIC Slot Number.
 *          eeprom_data - pointer to eeprom data.
 *
 * Returns : revision number of board.
 */
int plug_slot_get_bd_revision (uchar *eeprom_data, unsigned short *board_rev)
{
    uchar  *data_ptr;
    uchar  num_byte;
    
    /* for polling slots, do not print warning. simply print the content */
    if ((uchar)eeprom_data[0] == CURRENT_FORMAT_VERSION) {
	    if ((data_ptr = (uchar *)search_type_ret_addr_of_first_data
	         ((uchar *)eeprom_data, BOARD_REV, &num_byte, FALSE)) == NULL) {
	        *board_rev =  0xffff;
	    } else {
	        *board_rev = *data_ptr++;
	        *board_rev = *board_rev << 8 | *data_ptr;
	    }
    } else { 
	    /* Get Board Revision Number from PIM EEPROM located at 0x10. */
	    *board_rev =  (ushort)(*(eeprom_data + 0x10));
    }

    return(PASSED);
}

/*-----------------------------------------------------------------------------
 *
 * Function get_pcb_serial
 *
 * This function will return the SERIAL number of the HWIC module.
 *
 * Inputs : hwic_num - HWIC Slot Number.
 *          eeprom_data - pointer to eeprom data.
 *
 * Returns : serial number of board.
 */
int plug_slot_get_pcb_serial (uchar *eeprom_data, char *serial)
{
    uchar *data_ptr;
    uchar num_byte;

    if ((uchar)eeprom_data[0] == CURRENT_FORMAT_VERSION) {
	    /* for polling slots, do not print warning. simply print the content */
	    if ((data_ptr = (uchar *)search_type_ret_addr_of_first_data
	         ((uchar *)eeprom_data, PCB_SERIAL_NUM, &num_byte, FALSE)) == NULL) {
                sprintf(serial, "NO PCB NUM");
	    } else {
                memcpy(serial, data_ptr, 12);
	    }
	    return(0);
    } else {
	    /* Get PCB Serial Number from PIM EEPROM located at 0x4 to 0x7 */
	    return(*(int *)(eeprom_data + 0x4));
    }
}

/*******************************************************************************
 *
 * Function   : plug_slot_module_info 
 * Description: show pluggable slot module info by cookie 
 * Inputs     : slot number
 *              module cookie
 * Outputs    : PASSED or FAILED
 *******************************************************************************
 */
int plug_slot_module_info (int slot, uint16_t cookid)
{
    plug_module_sku_info *plugp;
    plugp = plug_module_sku_tbl;
    /* Search match platform cookie table */
    while (plugp->plug_module_name != NULL) {
        if (plugp->cook_contype == cookid) {
            printf("PIM%1d: %s cookie id = 0x%4x.\n", slot, plugp->plug_module_name, cookid);
            break;
        }
        plugp++;
    }
    if (plugp->plug_module_name == NULL) {
        printf("*** WARNING: Could not find correct PLUG module SKU info.\n");
        return (FAILED);
    }
    return (PASSED);

}

/**********************************************************************
 *
 * Function:i2c_dswap4 
 *
 * Description: 4-byte data swap 
 *
 * Input: ix - data
 *        
 * Output: swaped data
 *
 **********************************************************************/
uint32_t i2c_dswap4(int ix)
{
     return (dswap4(ix));
}

/**********************************************************************
 *
 * Function: plug_fpga_i2c_flush_fifo
 *
 * Description: flush out data inside fifo
 *
 * Input: i2c - pointer to pluggable FPGA i2c master
 *        byte - number of byte to flush
 * Output: RC_I2C_OP_OK
 *
 **********************************************************************/
static int plug_fpga_i2c_flush_fifo (int i2c_addr, int byte)
{
    unsigned int tmp;

    while (byte--) {
        plug_common_host_plug_fpga_reg_read(i2c_addr + PLUG_I2C_DATA_FIFO_OFFSET, (uint *)&tmp);
        tmp++;
    }
    /* do it a couple of more times just to be safe */
    plug_common_host_plug_fpga_reg_read(i2c_addr + PLUG_I2C_DATA_FIFO_OFFSET, (uint *)&tmp);
    plug_common_host_plug_fpga_reg_read(i2c_addr + PLUG_I2C_DATA_FIFO_OFFSET, (uint *)&tmp);

    return (RC_I2C_OP_OK);

}

/**********************************************************************
 *
 * Function: plug_fpga_rd_i2c_data_fifo
 *
 * Description: Read data bytes from the i2c data fifo
 *
 * Input: i2c_addr - pointer to pluggable FPGA i2c master
 *        data_len - The number of bytes to xfer
 *        data_p - pointer to the buffer holding the data being xfer
 *
 * Output: void
 *
 ***********************************************************************/
void plug_fpga_rd_i2c_data_fifo (int i2c_addr, uint32_t data_len,
                                 uchar *data_p)
{
    uint32_t ix, jx, word_count, byte_count;
    uint32_t dword;
    uchar *byte_p;
    int data_in;

    word_count = data_len / 4;
    byte_count = data_len % 4;

    for (ix = 0; ix < word_count; ix++) {
        plug_common_host_plug_fpga_reg_read(i2c_addr + PLUG_I2C_DATA_FIFO_OFFSET, (uint *)&data_in);
        dword = i2c_dswap4(data_in);
        byte_p = (uchar *)&dword;
        for (jx = 0; jx < 4; jx++) {
            *data_p++ = *byte_p++;
        }
    }

    if (byte_count > 0) {
        plug_common_host_plug_fpga_reg_read(i2c_addr + PLUG_I2C_DATA_FIFO_OFFSET, (uint *)&data_in);
        dword = i2c_dswap4(data_in);
        byte_p = (uchar *)&dword;
        for (ix = 0; ix < byte_count; ix++) {
            *data_p++ = *byte_p++;
        }
    }

    /* flush again just to be safe */
    plug_fpga_i2c_flush_fifo(i2c_addr, data_len);

}


/**********************************************************************
 *
 * Function: plug_fpga_i2c_normal_op
 *
 * Description: Perform standard I2C read or write operation on the
 *              I2C slave device
 *
 * Input: i2c - Pluggable FPGA i2c reg address
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
 **********************************************************************/
int plug_fpga_i2c_normal_op (int i2c_addr, uint8_t mux, uint32_t slv_addr,
                             uint32_t data_len, uint32_t sub_addr_sz,
                             uint32_t reg_addr, uint32_t rd_wr_mode)
{
    uint32_t reg_val, ix, timeout_val, temp_val;
    err_no = 0;
    i2c_status = 0;

    /* Set up control register and the slave address register */
    reg_val = PLUG_I2C_CTRL_CLK_50 | PLUG_I2C_CTRL_CLK_50 |
              PLUG_I2C_CTRL_SPEED_NORMAL_100 | rd_wr_mode |
             (data_len << L_SHFT_PLUG_I2C_CTRL_BYTE_LEN) |
             (mux << L_SHFT_PLUG_I2C_CTRL_MUX);

    if (mux >= 4) {
        assert("mux has to be less than 4");
    }

    /* write mode end */
    /* Enable the normal operation */
    plug_common_host_plug_fpga_reg_write(i2c_addr + PLUG_I2C_MSTR_SLAVE_ADDR_OFFSET, slv_addr);
    if (sub_addr_sz != 0) {
        plug_common_host_plug_fpga_reg_write(i2c_addr + PLUG_I2C_MSTR_SUBSL_ADDR_OFFSET, reg_addr);
    }

    reg_val |= (PLUG_I2C_CTRL_NORMAL | (sub_addr_sz) << 24);
    plug_common_host_plug_fpga_reg_write(i2c_addr + PLUG_I2C_MSTR_CTRL_OFFSET, reg_val);

    /* give time for device to send acknowlegement..especiall when talking to quack */
    msleep(PLUG_FPGA_I2C_OP_DELAY);

    /* Monitor the done bit in status register. Add 10 satety bytes for
     * wait time calculation due to I2C protocol is slow and have gaps
     */
    timeout_val = PLUG_FPGA_I2C_OP_TOUT;

    /* Wait one byte time to let the i2c op to start before polling status */
    /* if no delay we might miss the no ack */
    for (ix = 0; ix <= timeout_val; ix++) {
        plug_common_host_plug_fpga_reg_read(i2c_addr + PLUG_I2C_MSTR_STS_OFFSET, &reg_val);
        temp_val = reg_val;
        if (temp_val & MSK_PLUG_I2C_STAT_NO_SLV) { /* check bit 2 */
            err_no = (RC_I2C_SLV_NACK);
            printf("\n\n");
            printf("device shown below is not acknowledging; is it installed? "
                   " [i2c status @%#x=%#x %d]\n",
                   i2c_addr + PLUG_I2C_MSTR_STS_OFFSET, reg_val, err_no);
            return (RC_I2C_SLV_NACK);
        } 

        /* if slave device does not answer, return busy status bit 4A */
        if ((reg_val & MSK_PLUG_I2C_STAT_STD_DONE) != 0) {
            break;
        }
        msleep(8);
    }
    if (ix > timeout_val) {
        err_no = (RC_I2C_TIMEOUT);
        printf("device shown below is not acknowledging; is it installed? "
               " [i2c status @%#x=%#x %d]\n",
               i2c_addr + PLUG_I2C_MSTR_STS_OFFSET, reg_val, err_no);
        return (RC_I2C_TIMEOUT);
    }

    return (RC_I2C_OP_OK);
}

/**********************************************************************
 *
 * Function: plug_fpga_wr_i2c_data_fifo
 *
 * Description: Write data bytes to the i2c data fifo
 *
 * Input: i2c - pointer to pluggable FPGA i2c master
 *        data_len - The number of bytes to xfer
 *        data_p - pointer to the buffer holding the data being xfer
 *
 * Output: void
 *
 ***********************************************************************/
void plug_fpga_wr_i2c_data_fifo (int i2c_addr, uint32_t data_len,
                                 uchar *data_p)
{
    uint32_t ix, jx, word_count, byte_count;
    uint32_t dword;
    uchar *byte_p;

    /* do i need to read until i get an underflow condition to
       make sure fifo is empty before writing to fifo? */

    word_count = data_len / 4;
    byte_count = data_len % 4;

    for (ix = 0; ix < word_count; ix++) {
        byte_p = (uchar *)&dword;
        for (jx = 0; jx < 4; jx++) {
            *byte_p++ = *data_p++;
        }
        plug_common_host_plug_fpga_reg_write(i2c_addr + PLUG_I2C_DATA_FIFO_OFFSET,
                            (uint )i2c_dswap4(dword));
    }

    if (byte_count) {
        dword = 0;
        byte_p = (uchar *)&dword;
        for (ix = 0; ix < byte_count; ix++) {
            *byte_p++ = *data_p++;
        }
        plug_common_host_plug_fpga_reg_write(i2c_addr + PLUG_I2C_DATA_FIFO_OFFSET,
                            (uint )i2c_dswap4(dword));
    }
}

/**********************************************************************
 *
 * Function: plug_fpga_i2c_send_reg_offset
 *
 * Description: send register offset to plug FPGA i2c bus 
 *
 * Input: i2c - point to pluggable fpga i2c master
 *        slv_addr - point to pluggable i2c slave device
 *        reg_addr - register address
 *         
 * Output: return OK
 *
 **********************************************************************/
int plug_fpga_i2c_send_reg_offset (int i2c_addr, uint32_t mux,
                                          uint32_t slv_addr,uint32_t reg_addr,
                                          uint32_t sub_addr_sz)
{
    uint32_t rc;
    uint32_t addr_size = 1;

    /* write 1 byte reg offset into data fifo */
    plug_fpga_wr_i2c_data_fifo(i2c_addr, addr_size, (unsigned char *)
                               &reg_addr);

    /* initializte write transaction to send off set onto the bus */
    rc = (plug_fpga_i2c_normal_op(i2c_addr, mux, slv_addr, addr_size,
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
 * Function: plug_fpga_i2c_reset
 *
 * Description: Reset an I2C master module
 *
 * Input: none
 *
 * Output: PASSED or FAILED
 *
 *********************************************************************/
void plug_fpga_i2c_reset (int i2c_addr)
{
    int ctr = 0;
    int i2c_ctrl, i2c_bb;

    plug_common_host_plug_fpga_reg_read(i2c_addr + PLUG_I2C_MSTR_CTRL_OFFSET, (uint *)&i2c_ctrl);
    i2c_ctrl |= PLUG_I2C_CTRL_SOFT_RESET;
    plug_common_host_plug_fpga_reg_write(i2c_addr + PLUG_I2C_MSTR_CTRL_OFFSET, i2c_ctrl);
    usleep(PLUG_FPGA_REG_WRITE_DELAY);

    /* goes into bitbang mode */
    plug_common_host_plug_fpga_reg_read(i2c_addr + PLUG_I2C_MSTR_CTRL_OFFSET, (uint *)&i2c_ctrl);
    i2c_ctrl |= PLUG_I2C_CTRL_BITBANG;
    plug_common_host_plug_fpga_reg_write(i2c_addr + PLUG_I2C_MSTR_CTRL_OFFSET, i2c_ctrl);

    /* drives the SDA lines low */
    plug_common_host_plug_fpga_reg_read(i2c_addr + PLUG_I2C_BITBANG_OFFSET, (uint *)&i2c_bb);
    i2c_bb &= ~(PLUG_I2C_BITBANG_SDA_DRIVER);
    plug_common_host_plug_fpga_reg_write(i2c_addr + PLUG_I2C_BITBANG_OFFSET, i2c_bb);


    /* keeps driving SCL until it recovers */
    for (ctr = 0; ctr < SCL_DRIVE_TIMES; ctr++) {
        plug_common_host_plug_fpga_reg_read(i2c_addr + PLUG_I2C_BITBANG_OFFSET, (uint *)&i2c_bb);
        i2c_bb &= ~(PLUG_I2C_BITBANG_SCL_DRIVER);
        plug_common_host_plug_fpga_reg_write(i2c_addr + PLUG_I2C_BITBANG_OFFSET, i2c_bb);
        msleep(1);
        plug_common_host_plug_fpga_reg_read(i2c_addr + PLUG_I2C_BITBANG_OFFSET, (uint *)&i2c_bb);
        i2c_bb |= PLUG_I2C_BITBANG_SCL_DRIVER;
        plug_common_host_plug_fpga_reg_write(i2c_addr + PLUG_I2C_BITBANG_OFFSET, i2c_bb);
        msleep(1);
    }   

    /* drives the SDA lines High */
    plug_common_host_plug_fpga_reg_read(i2c_addr + PLUG_I2C_BITBANG_OFFSET, (uint *)&i2c_bb);
    i2c_bb |= PLUG_I2C_BITBANG_SDA_DRIVER;

    /* leave bitbang mode */
    plug_common_host_plug_fpga_reg_read(i2c_addr + PLUG_I2C_MSTR_CTRL_OFFSET, (uint *)&i2c_ctrl);
    i2c_ctrl &= ~(PLUG_I2C_CTRL_BITBANG);
    plug_common_host_plug_fpga_reg_write(i2c_addr + PLUG_I2C_MSTR_CTRL_OFFSET, i2c_ctrl);

}


/**********************************************************************                                      
 *
 * Function: plug_fpga_chk_i2c_idle
 *
 * Description: Check if the i2c master is idle
 *
 * Input: none
 *
 * Output: TRUE or FALSE
 *
 **********************************************************************/
boolean plug_fpga_chk_i2c_idle (int i2c_addr)
{
    uint32_t ix, timeout_val, retry;
    volatile uint32_t i2c_status;

    timeout_val = PLUG_FPGA_I2C_IDLE_TIMEOUT;//GFY_I2C_XFER_BIT_COUNT(2);
    for (retry = 0 ;  retry < 5; retry++) {
        for (ix = 0; ix < timeout_val; ix++) {
            plug_common_host_plug_fpga_reg_read(i2c_addr + PLUG_I2C_MSTR_STS_OFFSET, (uint *)&i2c_status);
            if (i2c_status & MSK_PLUG_I2C_STAT_NOT_ACTIVE) {
                return (TRUE);
            }   
            msleep(10);
        }   
        plug_fpga_i2c_reset(i2c_addr);
    }   
    printf("i2c failure: data is still being trasferred. too long to complete.\n");
    return (FALSE);
}


/**********************************************************************
 *
 * Function: plug_common_fpga_i2c_ack_check
 *
 * Description: Check Plug FPGA I2C device have ack or not
 *
 * Input: i2c_addr - Offset to I2C Controller Address
 *        slv_addr - The i2c slave's address on the i2c bus
 *        sub_addr_sz - byte size of the reg_addr (0 to 3 bytes)
 *        reg_addr - device register address
 *        data_len - The number of bytes to be xfer
 *        data_buf - The buffer to store the data being xfer
 *
 * Output: rc = PASSED have ACK
 *            = FAILED no ACK
 *
 **********************************************************************/
int plug_common_fpga_i2c_ack_check (int i2c_addr, uint8_t mux, uint32_t slv_addr,
                      int32_t reg_addr, uint32_t sub_addr_sz,
                      uint32_t data_len, uchar *data_buf)
{

    uint32_t reg_val, ix, timeout_val, temp_val; 
    err_no = 0;
    i2c_status = 0;
    int rc;

    /* Unmasking I2C Master Status Register */
    plug_common_host_plug_fpga_reg_write(i2c_addr + PLUG_I2C_MSTR_STS_MASK_OFFSET, 0);

    if (plug_fpga_chk_i2c_idle(i2c_addr) == FALSE) {
        return (FAILED);
    }
    /* if required, send reg offset to slave and flush fifo */
    if ((reg_addr >= 0) && (sub_addr_sz == 0)) {
        rc = plug_fpga_i2c_send_reg_offset(i2c_addr, mux, slv_addr,
                                           reg_addr, sub_addr_sz);
        if (rc != RC_I2C_OP_OK) {
            return (FAILED);
        }
    }
    
    /* Set up control register and the slave address register */
    reg_val = PLUG_I2C_CTRL_CLK_50 | PLUG_I2C_CTRL_CLK_50 |
              PLUG_I2C_CTRL_SPEED_NORMAL_100 | PLUG_I2C_CTRL_RD_MODE |
             (data_len << L_SHFT_PLUG_I2C_CTRL_BYTE_LEN) |
             (mux << L_SHFT_PLUG_I2C_CTRL_MUX);

    if (mux >= 4) {
        assert("mux has to be less than 4");
    }

    /* write mode end */
    /* Enable the normal operation */
    plug_common_host_plug_fpga_reg_write(i2c_addr + PLUG_I2C_MSTR_SLAVE_ADDR_OFFSET, slv_addr);
    if (sub_addr_sz != 0) {
        plug_common_host_plug_fpga_reg_write(i2c_addr + PLUG_I2C_MSTR_SUBSL_ADDR_OFFSET, reg_addr);
    }

    reg_val |= (PLUG_I2C_CTRL_NORMAL | (sub_addr_sz) << 24);
    plug_common_host_plug_fpga_reg_write(i2c_addr + PLUG_I2C_MSTR_CTRL_OFFSET, reg_val);

    /* give time for device to send acknowlegement..especiall when talking to quack */
    msleep(PLUG_FPGA_I2C_OP_DELAY);

    /* Monitor the done bit in status register. Add 10 satety bytes for
     * wait time calculation due to I2C protocol is slow and have gaps
     */
    timeout_val = PLUG_FPGA_I2C_OP_TOUT;

    /* Wait one byte time to let the i2c op to start before polling status */
    /* if no delay we might miss the no ack */
    for (ix = 0; ix <= timeout_val; ix++) {
        plug_common_host_plug_fpga_reg_read(i2c_addr + PLUG_I2C_MSTR_STS_OFFSET, &reg_val);
        temp_val = reg_val;
        if (temp_val & MSK_PLUG_I2C_STAT_NO_SLV) { /* check bit 2 */
            err_no = (RC_I2C_SLV_NACK);
            return (FAILED);
        } 

        /* if slave device does not answer, return busy status bit 4A */
        if ((reg_val & MSK_PLUG_I2C_STAT_STD_DONE) != 0) {
            break;
        }
        msleep(8);
    }
    if (ix > timeout_val) {
        err_no = (RC_I2C_TIMEOUT);
        return (FAILED);
    }

    /* read data from fifo and flush */
    plug_fpga_rd_i2c_data_fifo(i2c_addr, data_len, data_buf);

    return (PASSED);
}


/*-------------------------------------------------
$Log: plug_common_lib.c,v $
Revision 1.4  2019/08/15 09:27:52  shjung
Supported WP7610 PIM

Revision 1.3  2019/08/06 06:56:16  alpeng
merge curie, switzer and nightwatch to trunk

Revision 1.2  2018/11/23 09:02:32  hondwang
Re-instruct pluggable common code with CDETs CSCvn17216

Revision 1.1.2.2  2018/10/22 09:39:01  hondwang
move plug_slot_module_info to common code

Revision 1.1.2.1  2018/10/15 06:44:31  hondwang
pluggable common code re-instruct add and remove files



$Endlog$
*/
