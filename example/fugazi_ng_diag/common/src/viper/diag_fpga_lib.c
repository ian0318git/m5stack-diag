 /* $Id: diag_fpga_lib.c,v 1.2 2018/08/06 02:31:50 harrchan Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/viper/diag_fpga_lib.c,v $
 *-----------------------------------------------------------------------------
 *
 * Filename   : diag_fpga_lib.c
 * Description: FPGA Library.
 *
 * Copyright (c) 2018 by Cisco Systems, Inc.
 * All rights reserved.
 *-----------------------------------------------------------------------------
 */
#include <stdio.h>
#include <stdlib.h>
#include <sys/io.h>
#include <time.h>
#include <sys/mman.h>
#include <fcntl.h>
#include "defs.h"
#include "error.h"
#include "common.h"
#include "types.h"
#include "nvsysvars.h"
#include <unistd.h>
#include <strings.h>
#include <stdlib.h>
#include <assert.h>
#include "queryflags.h"
#include "common_utils.h"
#include "queryflags.h"
#include "menu.h"
#include "proto.h"
#include "diag_fpga_lib.h"
#include "diag_fpga.h"


unsigned long  dash_fpga;

/*******************************************************************************
 *                          Function Declaration
 *******************************************************************************
 */
int fpga_read_reg(uint, uint *);
int fpga_write_reg(uint, uint);
int fpga_reset_api(uint, uint, uint, uint);
int open_ioperm(void);
int close_ioperm(void);
int has_ge1_sku(void);
int this_is_viper_foxconn(void);


/*******************************************************************************
 *
 * Function    : fpga_read_reg
 * Description : Function to read FPGA register.
 * Inputs      : reg_offset - register offset
 *               *buf       - buffer to put read back register value
 * Outputs     : PASSED/FAILED
 *
 *******************************************************************************
 */
int fpga_read_reg (uint reg_offset, uint32_t *buf)
{
    *buf = *((unsigned int *)((long)dash_fpga + reg_offset));
    return (PASSED);
}


/*******************************************************************************
 *
 * Function    : fpga_write_reg
 * Description : Function performs FPGA register write.
 * Inputs      : reg_offset - register offset
 *               wr_data    - data for write
 * Outputs     : PASSED/FAILED
 *
 *******************************************************************************
 */
int fpga_write_reg (uint reg_offset, uint wr_data)
{
    *((unsigned int *)((long)dash_fpga + reg_offset)) = wr_data;
    return (PASSED);
}


/*******************************************************************************
 *
 * Function    : fpga_reset_api
 * Description : Function of FPGA to reset/unreset interface.
 * Inputs      : r_offset  - register offset
 *               r_bit     - reset bit of register
 *               r_opt     - reset(TRUE)/un-reset(FALSE)
 *               r_time_ms - the reset time interval(millisecond)
 * Outputs     : PASSED/FAILED
 *
 *******************************************************************************
 */
int fpga_reset_api (uint r_offset, uint r_bit, uint r_opt, uint r_time_ms)
{
    uint reg_val = 0;

    /* Read FPGA interface reset register. */
    if (fpga_read_reg(r_offset, &reg_val) != PASSED) {
        printf("%s: Failed to read FPGA reg.(0x%04X).\n",
               __FUNCTION__, r_offset);
        return (FAILED);
    }

    if (r_opt == TRUE) {
        /* Set the Reset bit. */
        reg_val |= r_bit;
    } else if (r_opt == FALSE) {
        /* Clear the reset bit. */
        reg_val &= (uint)(~r_bit);
    } else {
        printf("%s: Invalid Reset option(%#x).\n", __FUNCTION__, r_opt);
        return (FAILED);
    }

    /* Write the reset/un-reset into the corresponding register bit. */
    if (fpga_write_reg(r_offset, reg_val) != PASSED) {
        printf("%s: Failed to write FPGA reg.(0x%04X).\n",
               __FUNCTION__, r_offset);
        return (FAILED);
    }

    /* Delay milliseconds after reset/un-reset */
    msleep(r_time_ms);

    /* Confirm the change to FPGA interface reset register. */
    reg_val = 0;
    if (fpga_read_reg(r_offset, &reg_val) != PASSED) {
        printf("%s: Failed to read FPGA reg.(0x%04X).\n",
               __FUNCTION__, r_offset);
        return (FAILED);
    }

    if (((r_opt == TRUE) && ((reg_val & r_bit) != r_bit)) ||
        ((r_opt == FALSE) && ((reg_val & r_bit) != 0))) {
        printf("%s: Failed to %s reset bit in FPGA reg.(0x%04X).\n",
               __FUNCTION__, (r_opt == TRUE) ? "set" : "clear", r_offset);
        return (FAILED);
    }


     return (PASSED);
}

/*******************************************************************************
 *
 * Function   : this_is_viper_j
 * Description: Function to distinguish sku feature with Viper
 * Inputs     : None 
 * Outputs    : TRUE / FALSE
 *
 *******************************************************************************
 */
int this_is_viper_j (void)
{
    uint reg_addr = (uint)FPGA_BOARD_TYPE_REG;
    uint btype = 0;

    if (fpga_read_reg(reg_addr, &btype) != PASSED) {
        printf("%s: Failed to read FPGA Borad Type Reg(0x%04X).\n",
               __FUNCTION__, reg_addr);
        return (FAILED);
    }

    btype = ((btype & FPGA_BTYPE_PRODUCT_SKU_BIT)>>FPGA_BTYPE_PRODUCT_SKU_BIT_SHIFT);
    
    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("%s: FPGA board type %08X.\n", __FUNCTION__, btype);
    }

    if (btype == FPGA_BTYPE_VIPER_J) {
        return (TRUE);
    }
    return (FALSE);
}

/*******************************************************************************
 *
 * Function   : this_is_viper_foxconn
 * Description: Function to distinguish sku feature with Viper
 * Inputs     : None 
 * Outputs    : TRUE / FALSE
 *
 *******************************************************************************
 */
int this_is_viper_foxconn (void)
{

    if (this_is_viper_j() == TRUE) {
        return (FALSE);
    }

    return (TRUE);

}


/*******************************************************************************
 *
 * Function   : get_dsl_annex_sku_id
 * Description: Function to distinguish DSL feature 
 * Inputs     : None 
 * Outputs    : DSL SKU
 *
 *******************************************************************************
 */
int get_dsl_annex_sku_id (void)
{
    uint reg_addr = (uint)FPGA_BOARD_TYPE_REG;
    uint btype = 0;
    unsigned char dsl_sku = 0xFF;

    fpga_read_reg(reg_addr, &btype);

    dsl_sku = ((btype & FPGA_BTYPE_DSL_SKU_MASK) >> FPGA_BTYPE_DSL_SKU_BIT_SHIFT);
    
    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("%s: DSL SKU type %08X.\n", __FUNCTION__, btype);
    }

    return (dsl_sku);
}

/*******************************************************************************
 *
 * Function   : has_ge1_sku
 * Description: Function to distinguish GE1
 * Inputs     : None 
 * Outputs    : TRUE/FALSE
 *
 *******************************************************************************
 */
int has_ge1_sku (void)
{
    /* 
     * Viper Intel -> SKU with DSL have no GE1
     * ViperJ      -> All SKU have GE1
     */

    if ((has_dsl_sku() == FALSE)) {
        return (TRUE);
    }

    return (FALSE);

}

/*******************************************************************************
 *
 * Function   : has_lte_sku
 * Description: Function to distinguish sku feature with Viper
 * Inputs     : None 
 * Outputs    : TRUE / FALSE
 *
 *******************************************************************************
 */
int has_lte_sku (void)
{
    uint reg_addr = (uint)FPGA_BOARD_TYPE_REG;
    uint btype = 0;

    if (fpga_read_reg(reg_addr, &btype) != PASSED) {
        printf("%s: Failed to read FPGA Borad Type Reg(0x%04X).\n",
               __FUNCTION__, reg_addr);
        return (FAILED);
    }
    
    btype = (btype & FPGA_BTYPE_LTE_SKU_BIT);
    
    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("%s: FPGA board type %08X.\n", __FUNCTION__, btype);
    }

    if (btype == FPGA_BTYPE_LTE_SKU_BIT) {
        return (TRUE);
    }
    return (FALSE);
}

/*******************************************************************************
 *
 * Function   : has_dsl_sku
 * Description: Function to distinguish sku feature with Viper
 * Inputs     : None 
 * Outputs    : TRUE / FALSE
 *
 *******************************************************************************
 */
int has_dsl_sku (void)
{
    uint reg_addr = (uint)FPGA_BOARD_TYPE_REG;
    uint btype = 0;

    if (fpga_read_reg(reg_addr, &btype) != PASSED) {
        printf("%s: Failed to read FPGA Borad Type Reg(0x%04X).\n",
               __FUNCTION__, reg_addr);
        return (FAILED);
    }
    
    btype = ((btype & FPGA_BTYPE_DSL_SKU_BIT)>>FPGA_BTYPE_DSL_BIT_SHIFT);
    
    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("%s: FPGA board type %08X.\n", __FUNCTION__, btype);
    }

    if (btype == FPGA_BTYPE_DSL) {
        return (TRUE);
    }
    return (FALSE);
}

/*-------------------------------------------------
 * $Log: diag_fpga_lib.c,v $
 * Revision 1.2  2018/08/06 02:31:50  harrchan
 * Merge viper E2E to the main trunk (CSCvk28469)
 *
 * Revision 1.1.2.7  2018/05/10 05:51:21  olin2
 * Support voltage margin util for Viper-Intel
 *
 * Revision 1.1.2.6  2018/05/09 07:11:25  olin2
 * 1. Move GE and DSL init to the beginning. 2. Add has GE1. 3. Show cookie info
 *
 * Revision 1.1.2.5  2018/04/20 03:05:49  lucywang
 * Based on FPGA Board Type Register to show LTE/DLS test item
 *
 * Revision 1.1.2.4  2018/04/16 08:41:43  olin2
 * Support DSL test
 *
 * Revision 1.1.2.3  2018/03/28 07:03:51  lucywang
 * Added API to check SKU ViperJ and changed interface name for ViperJ
 *
 * Revision 1.1.2.2  2018/03/15 08:26:16  harrchan
 * Change I/O access to memory map
 *
 * Revision 1.1.2.1  2018/02/27 08:06:41  harrchan
 * Initial viper application code base
 *
 *
 * $Endlog$
 *-------------------------------------------------
 */
