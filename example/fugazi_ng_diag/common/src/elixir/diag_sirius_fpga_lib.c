/* $Id: diag_sirius_fpga_lib.c,v 1.2 2021/09/24 01:21:07 harrchan Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/elixir/diag_sirius_fpga_lib.c,v $
 *------------------------------------------------------------------
 * 
 * diag_sirius_fpga_lib.c
 *
 * Copyright (c) 2018 - 2019 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "common.h"
#include "proto.h"
#include "types.h"
#include "nvmonvars.h"
#include "common_utils.h"
#include "diag_cpu_lib.h"
#include "diag_sirius_fpga_lib.h"
#include "diag_moka_fpga_lib.h"

unsigned long base_plug_fpga;

unsigned long get_plug_fpga_i2c_addr(int);

int diag_fpga_reg_bitops(uint, uint, uint);
int plug_fpga_reg_read(uint, uint *);
int plug_fpga_reg_write(uint, uint);
int diag_fpga_reg_or(uint, uint);
int diag_fpga_reg_nand(uint, uint);

extern uint plat_fpga_reg_baseaddr;

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
    base_plug_fpga = plat_fpga_reg_baseaddr + PLUG_I2C_CTRL_OFFSET;

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

/*******************************************************************************
 * Function    : diag_plug_module_is_present
 *
 * Description : Function to check whether pluggable module is present
 * Inputs      : NONE
 * Outputs     : PASSED/FAILED
 * Note        : According to Sirius FPGA HFS(EDCS:11602504), 
 *               the NGIO Status Register(0x1_3000 + 0x50) bit[7] is used to 
 *               identify whether plugganle module is present.
 *               if Module Present bit,
 *               = 0, pluggable module is not present
 *               = 1, pluggable module is present
 *******************************************************************************
 */
int diag_plug_module_is_present (void)
{
    uint offset = FPGA_PLUG1_STSCTL_REG, rd_buf;
    if (plug_fpga_reg_read(offset, &rd_buf) != PASSED) {
        printf("%s:%d: Failed to read Sirius FPGA with offset:0x%x\n",
               __FUNCTION__, __LINE__, offset);
    }
    if ((rd_buf & PLUG_MODULE_IS_PRESENT) != PLUG_MODULE_IS_PRESENT) {
        printf("%s:%d: Pluggable module is not present!!\n",
               __FUNCTION__, __LINE__);
        return (FAILED);
    } else {
        return (PASSED);
    }
}

/*-------------------------------------------------
 * $Log: diag_sirius_fpga_lib.c,v $
 * Revision 1.2  2021/09/24 01:21:07  harrchan
 * Collapse Elixir-branch to Main Trunk.
 *
 * Revision 1.1.2.1  2020/09/09 09:08:07  illiu
 * First version which has been ported with Dreamliner and Marvel CPSS
 *
 * Revision 1.2  2019/01/10 06:36:24  wilbhuan
 * The beginning of Betelgeuse application code.
 *
 *-------------------------------------------------
 */
