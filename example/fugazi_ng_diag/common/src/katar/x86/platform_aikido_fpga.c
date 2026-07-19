/* $Id: platform_aikido_fpga.c,v 1.2 2019/06/14 05:24:49 mikech2 Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/katar/x86/platform_aikido_fpga.c,v $
 *------------------------------------------------------------------
 * Filename:    katar_platform_aikido_fpga.c
 *
 * Description: Aikido FPGA Diag tests
 *
 * Copyright (c) 2019 by cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */
#include "platform_aikido.h"


/*******************************************************************************
 *                                Externs
 *******************************************************************************
 */
extern int aikido_reg_test (void); 

extern unsigned long dash_aikido;


/*******************************************************************************
 *                          Function Declaration
 *******************************************************************************
 */
int        aikido_reg_test (void); 
int        aikido_read_32_reg (uint32_t, uint32_t *);
int        aikido_write_32_reg (uint32_t, uint32_t);


/*******************************************************************************
 *  
 * Function    : aikido_reg_test
 * Description : Utility to test aikido register.
 * Inputs      : NONE
 * Outputs     : PASSED/FAILED
 *
 *******************************************************************************
 */
int aikido_reg_test (void)
{
    uint32_t reg_offset = FPGA_AIKIDO_REG, rd_reg_val = 0x0, wr_reg_val = 0x1;

    if (aikido_read_32_reg(reg_offset, &rd_reg_val) != PASSED) {
        cterr('f', 0, "%s: Failed to read Aikido Register.",
              __FUNCTION__);
        return (FAILED);
    }
    
    // 1. write 0x1
    if (aikido_write_32_reg(reg_offset, wr_reg_val) != PASSED) {
        cterr('f', 0, "%s: Failed to write Aikido Register.",
              __FUNCTION__);
        return (FAILED);
    } else {
        printf("Done writing 0x%08X to FPGA register(0x%04X).\n",
               wr_reg_val, reg_offset);
    }

    if (aikido_read_32_reg(reg_offset, &rd_reg_val) != PASSED) {
        cterr('f', 0, "%s: Failed to read Aikido Register.",
              __FUNCTION__);
        return (FAILED);
    }

    if (rd_reg_val != 0x1) {
        cterr('f', 0, "%s: Aikido Register test failed; expect 0x1 instead of 0x%x",
              __FUNCTION__, rd_reg_val);
        return (FAILED);
    }


    // 2. write 0x0
    wr_reg_val = 0x0;

    if (aikido_write_32_reg(reg_offset, wr_reg_val) != PASSED) {
        cterr('f', 0, "%s: Failed to write Aikido Register.",
              __FUNCTION__);
        return (FAILED);
    } else {
        printf("Done writing 0x%08X to FPGA register(0x%04X).\n",
               wr_reg_val, reg_offset);
    }

    if (aikido_read_32_reg(reg_offset, &rd_reg_val) != PASSED) {
        cterr('f', 0, "%s: Failed to read Aikido Register.",
              __FUNCTION__);
        return (FAILED);
    }

    if (rd_reg_val == 0x0) {
        return (PASSED);
    } else {
        cterr('f', 0, "%s: Aikido Register test failed; expect 0x0 instead of 0x%x",
              __FUNCTION__, rd_reg_val);
        return (FAILED);
    }
}

/*******************************************************************************
 *
 * Function    : aikido_read_32_reg
 * Description : Function to read Aikido register.
 * Inputs      : reg_offset - register offset
 *               *buf       - buffer to put read back register value
 * Outputs     : PASSED/FAILED
 *
 *******************************************************************************
 */
int aikido_read_32_reg (uint32_t reg_offset, uint32_t *buf)
{
    unsigned long addr; 

    assert(dash_aikido);

    addr = dash_aikido;

    *buf = *( (volatile unsigned int *)(addr + reg_offset) );

    if (diagflag_xram & D_DEBUG_OPTIONS) {
        printf("0x%04lx=%#x\n", (addr+reg_offset), *buf);
    }

    return (PASSED);
}



/*******************************************************************************
 *
 * Function    : aikido_write_32_reg
 * Description : Function performs Aikido register write.
 * Inputs      : reg_offset - register offset
 *               wr_data    - data for write
 * Outputs     : PASSED/FAILED
 *
 *******************************************************************************
 */
int aikido_write_32_reg (uint32_t reg_offset, uint32_t wr_data)
{
    unsigned long addr = 0;  

    assert(dash_aikido);

    addr = dash_aikido;

    //origin_value = *( (unsigned int *)(addr + reg_offset) );

    register_write((addr + reg_offset), wr_data, BW_32BITS);

    //printf("write 0x%04lx = %#x (from %#x)\n", (addr + reg_offset), wr_data, origin_value);

    return (PASSED);
}







/*
 *------------------------------------------------------------------
 * $Log: platform_aikido_fpga.c,v $
 * Revision 1.2  2019/06/14 05:24:49  mikech2
 * Collapse katar-branch00 to Main Trunk
 *
 * Revision 1.1.2.3  2019/06/06 02:44:25  mikech2
 * clean out the unnecessary definitions and TSN string
 *
 * Revision 1.1.2.2  2019/02/12 08:06:29  mikech2
 * rename katar_*.h files
 *
 * Revision 1.1.2.1  2019/01/29 01:54:20  mikech2
 * rename katar_* files
 *
 * Revision 1.1.2.6  2018/12/27 02:26:10  peteteng
 * Update Aikido LPC scratchpad reg. addr.
 *
 * Revision 1.1.2.5  2018/12/12 02:03:39  peteteng
 * Add Aikido FW upgrade through LPC
 *
 * Revision 1.1.2.4  2018/12/07 14:41:42  peteteng
 * Modify addr. of Aikido LPC scratchpad test
 *
 * Revision 1.1.2.3  2018/11/23 03:45:34  peteteng
 * Fix I2C Util write multiple bytes issue
 *
 * Revision 1.1.2.2  2018/11/22 02:50:49  peteteng
 * Add Aikido register read/write utility
 *
 * Revision 1.1.2.1  2018/11/14 08:14:58  peteteng
 * Add Aikido FPGA register test
 *
 *
 *
 *------------------------------------------------------------------
 * $Endlog$
 */

