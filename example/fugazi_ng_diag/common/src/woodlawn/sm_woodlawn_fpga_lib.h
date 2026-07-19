/* $Id: sm_woodlawn_fpga_lib.h,v 1.2 2013/10/08 08:48:26 tirawan Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/woodlawn/sm_woodlawn_fpga_lib.h,v $
 *******************************************************************************
 * File Name: sm_woodlawn.h
 *
 * Description: Woodlawn SM main header file
 *
 * Author: Kody Ko
 *
 * Copyright (c) 2013 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *******************************************************************************
 */

#ifndef SM_WOODLAWN_FPGA_LIB_H
#define SM_WOODLAWN_FPGA_LIB_H

#define FPGA_GPIO_EXP0_OUT_REG      (0x02)
#define FPGA_GPIO_EXP1_OUT_REG      (0x03)
#define FPGA_SCRATCHPAD_REG         (0x4f)
#define FPGA_UPGRADE_VER            (0x7)
#define FPGA_HIGH_VER_REG           (0xff) 
#define FPGA_MODE_MASK              (0x80)
#define FPGA_ID_MASK                (0xcf)
#define FPGA_ID_SKU1                (0x1)
#define FPGA_ID_SKU2                (0x0)
#define WOODLAWN_4GE_1XAUI          (0x10)
#define WOODLAWN_6GE                (0x11)
#define WOODLAWN_6GE_1XAUI          (0x15)
#define WOODLAWN_SLOT_TWO           (0x2)   

extern long util_oir_fpga_reg_read(void *);
extern long util_oir_fpga_reg_write(void *);
extern void fpga_init_i2c(int, void *);
extern long woodlawn_fpga_reg_test(void);
extern int woodlawn_fpga_check_ver(void);
#endif /* SM_WOODLAWN__FPGA_LIBH */

/******** History ********/
/*------------------------------------------------------------------------------
 * $Log: sm_woodlawn_fpga_lib.h,v $
 * Revision 1.2  2013/10/08 08:48:26  tirawan
 * Woodlawn collapsed to main trunk
 *
 * Revision 1.1.4.2  2013/08/20 10:58:49  tirawan
 * Branch into woodlawn-branch2 and port woodlawn code
 *
 * Revision 1.1.2.5  2013/06/17 03:20:34  leschen
 * Update for ctrl front panel leds
 *
 * Revision 1.1.2.4  2013/04/25 07:13:48  kodko
 * Add check FPGA version macros
 *
 * Revision 1.1.2.3  2013/04/10 10:24:04  tirawan
 * Change FPGA register test to test GPIO Expander Output bit
 *
 * Revision 1.1.2.2  2013/03/07 12:55:02  tirawan
 * Update FPGA I2C Address, and add power on sequence for Woodlawn SM
 *
 * Revision 1.1.2.1  2013/02/06 03:04:55  tirawan
 * Woodlawn Support on O2
 *
 *------------------------------------------------------------------------------
 * $Endlog$
 *------------------------------------------------------------------------------
 */

