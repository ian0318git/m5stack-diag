/* $Id: plug_host_fpga_lib.c,v 1.2 2018/11/23 08:49:52 hondwang Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/tsn/plug_host_fpga_lib.c,v $
 *------------------------------------------------------------------
 *
 * Filename: plug_host_fpga_lib.c
 * Description: functions to return addresses for various fpga components
 * Copyright (c) 2012-2018 by cisco Systems, Inc.
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
#include "plug_common_lib.h"
#include "plug_testcard_host_impl.h"
#include "platform_stub.h"
#include "plug_testcard_phy.h"
#include "plug_host_fpga_lib.h"

int plug_host_slot_module_info(int, uint16_t);
boolean star_plug_is_present(uint); 
static int star_plug_get_max_slot(void);
int plug_pwr_on_util(void); 
int plug_pwr_off_util(void); 
static int plug_pwr_ctrl(int, int);
int i2c_err_no(uint32_t *);
int plug_fpga_i2c_rd(int, uint8_t, uint32_t, int32_t, 
                            uint32_t, uint32_t, uchar *);
int plug_fpga_i2c_wr(int, uint8_t, uint32_t, int32_t, 
                            uint32_t, uint32_t, uchar *);
void plug_i2c_act2_reset(sc_context *);
unsigned long get_plug_fpga_i2c_addr(int);
int show_plug_fpga_ver(int);
int plug_fpga_reg_read(uint, uint *);
int plug_fpga_reg_write(uint, uint);
int plug_fpga_reg_or(uint, uint);
int plug_fpga_reg_nand(uint, uint);
int diag_fpga_reg_bitops(uint, uint, uint);
int fpga_read_32_reg(uint, uint *);
int fpga_write_32_reg(uint, uint );


unsigned long base_plug_fpga;


extern uint tsn_fpga_reg_baseaddr;

extern int err_no;
extern int i2c_status;

int plug_fpga_i2c_rd(int, uint8_t, uint32_t, int32_t, uint32_t, uint32_t, uchar *);
int plug_fpga_i2c_wr(int, uint8_t, uint32_t, int32_t, uint32_t, uint32_t, uchar *);


/*******************************************************************************
 * Function   : star_plug_is_present
 * Description: Function to check if STAR PLUG module is present.
 *              Determined by PLUG FPGA Control/Status Reg.(0x1_3000).
 * Inputs     : slot_num - Pluggable slot number (Start from 1)
 * Outputs    : TRUE / FALSE
 *
 *******************************************************************************
 */
boolean star_plug_is_present (uint slot_num)
{
    uint reg_addr;
    uint reg_val = 0;
    int max_slot_num = star_plug_get_max_slot();

    /* Sanity check whether slot_num exceeds maximum number of slot */
    if (slot_num > max_slot_num) {
        printf("%s: Slot number (%d) exceeds maximum slot number (%d)\n", 
               __func__, slot_num, max_slot_num);
        return (FALSE);
    }

    /* Calculate Register address based on slot number */
    reg_addr = FPGA_PLUG_OFFSET_BY_SLOT(FPGA_PLUG1_STSCTL_REG, slot_num);

    if (fpga_read_32_reg(reg_addr, &reg_val) != PASSED) {
        printf("%s: Failed to read Pluggable FPGA Control/Status Reg(0x%04X).\n",
               __FUNCTION__, reg_addr);
        return (FALSE);
    }
    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("%s: PLUG FPGA @0x%04X: 0x%08X.\n", __FUNCTION__, reg_addr, reg_val);
        printf("%s: slot_num = 0x%08X.\n", __FUNCTION__, slot_num);
    }

    if ((reg_val & PLUG_PRSNT) != PLUG_PRSNT) {
        return (FALSE);
    }
    return (TRUE);
}

/*******************************************************************************
 * Function   : star_plug_get_max_slot
 * Description: Return maximum number of Pluggable Slots based on SKU
 * Inputs     : None
 * Outputs    : Maximum Slot Number
 *
 *******************************************************************************
 */
static int star_plug_get_max_slot (void) 
{
    if(this_is_star_c1109_4p()) { 
       return (MAX_PLUG_SLOT_NUMBER);
    } else {
       return (MAX_PLUG_SLOT_C1101);
    }
}

/*******************************************************************************
 * Function   : plug_pwr_on_util
 * Description: Function to power on pluggable module
 * Inputs     : None 
 * Outputs    : PASSED/FAILED.
 *
 *******************************************************************************
 */
int plug_pwr_on_util (void)
{   
    int slot;
    int pwr_opt;

    pwr_opt = PLUG_SLOT_PWR_ON;
    slot = getdec_answer("Power on which slot? (1-2):", PLUG_SLOT_1,
                         PLUG_SLOT_1, PLUG_SLOT_2);
    plug_pwr_ctrl(slot, pwr_opt);

    return (PASSED);
}


/*******************************************************************************
 * Function   : plug_pwr_off_util
 * Description: Function to power off pluggable module
 * Inputs     : None 
 * Outputs    : PASSED/FAILED.
 *
 *******************************************************************************
 */
int plug_pwr_off_util (void)
{   
    int slot;
    int pwr_opt;

    pwr_opt = PLUG_SLOT_PWR_OFF;
    slot = getdec_answer("Power off which slot? (1-2):", PLUG_SLOT_1,
                         PLUG_SLOT_1, PLUG_SLOT_2);
    
    plug_pwr_ctrl(slot, pwr_opt);

    return (PASSED);
}


/*******************************************************************************
 * Function   : plug_pwr_ctrl
 * Description: Function to power on/off pluggable module
 * Inputs     : slot - which pluggable slot
 *              value - 1 for power on 
 *                      0 for power off
 * Outputs    : PASSED/FAILED.
 *
 *******************************************************************************
 */
static int plug_pwr_ctrl (int slot, int value)
{   
    uint reg;

    reg = FPGA_PLUG_OFFSET_BY_SLOT(FPGA_PLUG1_STSCTL_REG, slot);
    if (value == PLUG_SLOT_PWR_OFF) {
        diag_fpga_reg_bitops(FPGA_BIT_OPS_OFF, reg, PLUG_PWR_EN_BIT);
    } else {
        diag_fpga_reg_bitops(FPGA_BIT_OPS_ON, reg, PLUG_PWR_EN_BIT);
    }

    return (PASSED);
}

/**********************************************************************
 *
 * Function: i2c_err_no 
 *
 * Description: return I2C  error number
 *
 * Input: status 
 *        
 * Output: RC_I2C_OP_OK
 *
 **********************************************************************/
int i2c_err_no(uint32_t * status)
{
    *status = i2c_status;
    return (err_no);
}

/**********************************************************************
 *
 * Function: plug_fpga_i2c_rd
 *
 * Description: Read data from the I2C slave device to the data buffer
 *              in normal I2C mode
 *
 * Input: i2c_addr - Offset to I2C Controller Address
 *        slv_addr - The i2c slave's address on the i2c bus
 *        sub_addr_sz - byte size of the reg_addr (0 to 3 bytes)
 *        reg_addr - device register address
 *        data_len - The number of bytes to be xfer
 *        data_buf - The buffer to store the data being xfer
 *
 * Output: rc = RC_I2C_OP_OK, RC_I2C_BUSY, RC_I2C_SLV_NACK or RC_I2C_TIMEOUT.
 *
 ************************************************************************/
int plug_fpga_i2c_rd (int i2c_addr, uint8_t mux, uint32_t slv_addr,
                      int32_t reg_addr, uint32_t sub_addr_sz,
                      uint32_t data_len, uchar *data_buf)
{
    int rc;

    /* Unmasking I2C Master Status Register */
    plug_fpga_reg_write(i2c_addr + PLUG_I2C_MSTR_STS_MASK_OFFSET, 0);

    if (plug_fpga_chk_i2c_idle(i2c_addr) == FALSE) {
        return(RC_I2C_BUSY);
    }
    /* if required, send reg offset to slave and flush fifo */
    if ((reg_addr >= 0) && (sub_addr_sz == 0)) {
        rc = plug_fpga_i2c_send_reg_offset(i2c_addr, mux, slv_addr,
                                           reg_addr, sub_addr_sz);
        if (rc != RC_I2C_OP_OK) {
            return rc;
        }
    }

    /* send read request to the slave */
    rc = (plug_fpga_i2c_normal_op(i2c_addr, mux, slv_addr, data_len, sub_addr_sz,
                                  reg_addr, PLUG_I2C_CTRL_RD_MODE));
    if (rc != RC_I2C_OP_OK) {
        return rc;
    }

    /* read data from fifo and flush */
    plug_fpga_rd_i2c_data_fifo(i2c_addr, data_len, data_buf);

    return rc;

}

/**********************************************************************
 *
 * Function: plug_fpga_i2c_wr
 *
 * Description: send i2c reg offset and data into data fifo, then
 *              write to pluggable FPGA control register to flush data on to the bus
 *              the first byte on the bus will be reg offset.
 *
 * Input: i2c_addr - Pluggable FPGA i2c master address
 *        slv_addr - The i2c slave's address on the i2c bus
 *        sub_addr_size - byte size of the reg_addr (0 to 3 bytes)
 *        reg_offset - device register address
 *        data_len - The number of bytes to be xfer
 *        data_buf - The buffer to store the data being xfer
 *
 * Output: rc = RC_I2C_OP_OK, RC_I2C_BUSY, RC_I2C_SLV_NACK or RC_I2C_TIMEOUT.
 *
 ************************************************************************/
/* plug_fpga_i2c_wr: send i2c reg offset and data into data fifo, then
   write to pluggable FPGA control register to flush data on to the bus
   the first byte on the bus will be reg offset.
*/
int plug_fpga_i2c_wr (int i2c_addr, uint8_t mux, uint32_t slv_addr,
                      int32_t reg_offset, uint32_t sub_addr_size,
                      uint32_t data_len, uchar *data_buf)
{
    unsigned int rc;
    unsigned char *buf = NULL;

    if (plug_fpga_chk_i2c_idle(i2c_addr) == FALSE) {
        return(RC_I2C_BUSY);
    }
    buf = malloc(data_len+sizeof(uint32_t));  /* we should allocate at least 4 bytes */
    memset(buf, 0, data_len+sizeof(uint32_t));
    if (sub_addr_size != 0) {
        plug_fpga_wr_i2c_data_fifo(i2c_addr, data_len, data_buf);
    }
    else {
        /* if required, send reg offset to slave device and flush fifo */
        if (reg_offset >= 0) {
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
        }
        plug_fpga_wr_i2c_data_fifo(i2c_addr, data_len, buf);
    }

    free(buf);

    /* write data to fifo ...(is it safe to flush fifo here, or do it
       later after we send out data to slave?) */

    /* initialiate write transaction */
    rc = (plug_fpga_i2c_normal_op(i2c_addr, mux, slv_addr, data_len,
                                  sub_addr_size, reg_offset,
                                  PLUG_I2C_CTRL_WR_MODE));

    return rc;
}

/**************************************************************************
 *
 * Name: plug_i2c_act2_reset
 *
 * Description: This function implementes a reset to Quack chip by
 *              reset the line for 50ms then unreset it
 *
 * Inputs: con - pointer to sc_context
 *
 * Outputs: None
 *
 *************************************************************************/
void plug_i2c_act2_reset (sc_context *con_p)
{
    unsigned int reg, reset, slot; 
    if (con_p->type == PLUGGABLE_CARD)  {
        slot = con_p->slot;
        switch (slot) {
        case PLUG_SLOT_1: 
            reg = FPGA_PLUG1_STSCTL_REG; 
            reset = PLUG_I2C_RESET; 
        break; 
        case PLUG_SLOT_2: 
            reg = FPGA_PLUG2_STSCTL_REG; 
            reset = PLUG_I2C_RESET; 
        break; 
        }    
        printf("Resetting ACT2 PLUG%d...", slot);
        fflush(stdout);
 
        plug_fpga_reg_or(reg, reset); 
        msleep(ACT2_RESET_UNRESET_DELAY);
        plug_fpga_reg_nand(reg, reset); 
 
        /* ACT2 unreset delay implement in tam_lib_platform_read */
        msleep(ACT2_UNRESET_DELAY);
        printf("Done\n");
        fflush(stdout);
    }
    return;
}

/*-------------------------------------------------------------------
 *
 * Function: get_plug_fpga_i2c_addr
 * Description: get pluggable FPGA i2c address
 *
 * Input: I2C Controller
 *
 * Output: Virtual Address of Pluggable FPGA I2C Controller
 *
 *-------------------------------------------------------------------
 */
unsigned long get_plug_fpga_i2c_addr (int ctrl)
{
    base_plug_fpga = tsn_fpga_reg_baseaddr + PLUG_I2C_CTRL_OFFSET;

    return (PLUG_I2C_CTRL_OFFSET + (ctrl * PLUG_FPGA_I2C_OFFSET));
}

/*******************************************************************************
 *
 * Function   : show_plug_fpga_ver
 * Description: Function to show plug FPGA version.
 *              This is by reading Pluggable FPGA Revision Reg(0x1_008C).
 * Inputs     : opt - reserved for future use
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
int show_plug_fpga_ver (int opt) 
{
    uint reg_addr = (uint)PLUG_FPGA_SYS_SEC_REV_REG;
    uint fpga_ver = 0; 

    if (fpga_read_32_reg(reg_addr, &fpga_ver) != PASSED) {
        printf("Failed to read FPGA Revision Reg(0x%04X).\n", reg_addr);
        return (FAILED);
    }    
    printf("Pluggable FPGA version: %08X\n", fpga_ver);

    return (PASSED);
}


/*******************************************************************************
 * Function    : plug_fpga_reg_read 
 * Description : Function to read Pluggable FPGA Register
 * Inputs      : offset
 *               *data_in - Pointer to buffer
 * Outputs     : PASSED / FAILED
 *
 *******************************************************************************
 */
int plug_fpga_reg_read (uint offset, uint *data_in)
{  
    if (fpga_read_32_reg(offset, data_in) != PASSED) {
        printf("%s: Failed to read PLUG FPGA Reg(0x%04X).\n",
                 __FUNCTION__, offset);
        return (FAILED);
    }
    return (PASSED);
}


/*******************************************************************************
 * Function    : plug_fpga_reg_write
 * Description : Function to write Pluggable FPGA Register
 * Inputs      : offset
 *               data_out - Data to be written to Pluggable FPGA
 * Outputs     : PASSED / FAILED
 *
 *******************************************************************************
 */
int plug_fpga_reg_write (uint offset, uint data_out)
{
    return (fpga_write_32_reg(offset, data_out));
}


/*******************************************************************************
 * Function    : plug_fpga_reg_or
 * Description : Function to perform OR bit ops on FPGA Register
 * Inputs      : offset
 *               bit
 * Outputs     : PASSED / FAILED
 *
 *******************************************************************************
 */
int plug_fpga_reg_or (uint offset, uint bit)
{
    uint data_in;
    
    if (plug_fpga_reg_read(offset, &data_in) == FAILED) {
        return (FAILED);
    }
    data_in |= bit;

    return (plug_fpga_reg_write(offset, data_in));
}


/*******************************************************************************
 * Function    : plug_fpga_reg_nand
 * Description : Function to perform NAND bit ops on FPGA Register
 * Inputs      : offset
 *               bit
 * Outputs     : PASSED / FAILED
 *
 *******************************************************************************
 */
int plug_fpga_reg_nand (uint offset, uint bit)
{
    uint data_in;

    if (plug_fpga_reg_read(offset, &data_in) == FAILED) {
        return (FAILED);
    }
    data_in &= ~(bit);
    return (plug_fpga_reg_write(offset, data_in));
}


/*******************************************************************************
 * Function    : plug_fpga_reg_bitops
 * Description : Function to turn on/off bit on FPGA Register
 * Inputs      : ops - ON or OFF
 *               offset 
 *               bit
 * Outputs     : PASSED / FAILED
 *
 *******************************************************************************
 */
int diag_fpga_reg_bitops (uint ops, uint offset, uint bit)
{
    uint data;
    
    if (plug_fpga_reg_read(offset, &data) != PASSED) {
        return (FAILED);
    }
    
    switch (ops) {
    case FPGA_BIT_OPS_ON:
        data |= (0x1 << bit);
        break;
    case FPGA_BIT_OPS_OFF:
        data &= ~(0x1 << bit);
        break;
    default:
        printf("Not recognized bit ops (%d)\n", ops);
        return (FAILED);
    }
    
    return (plug_fpga_reg_write(offset, data));
}


/*-------------------------------------------------
$Log: plug_host_fpga_lib.c,v $
Revision 1.2  2018/11/23 08:49:52  hondwang
Re-instruct pluggable common code with CDETs CSCvn17216

Revision 1.1.2.2  2018/10/22 09:38:07  hondwang
move plug_slot_module_info to common codeplug_common/plug_common_lib.c

Revision 1.1.2.1  2018/10/15 06:44:32  hondwang
pluggable common code re-instruct add and remove files


$Endlog$
*/
