/* $Id: dash_fpga.h,v 1.4 2019/07/11 12:31:26 alicehua Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/nutella/dash_fpga.h,v $
 *------------------------------------------------------------------
 * Filename:    dash_fpga.h
 *
 *
 * Copyright (c) 2019 by cisco Systems, Inc.
 * All rights reserved.
 *
 * Nutella does not have Dash FPGA, create this dummy file for
 * common source file cookie_4_core.c needs to include it.
 *
 *------------------------------------------------------------------
 */
#ifndef __DASH_FPGA__
#define  __DASH_FPGA__

//#define FPGA_RST_ACT2   0
#define FPGA_RST_ACT2   0x4

extern unsigned long fpga_ptr;

extern int read_eeprom_block(unsigned int, unsigned int size, unsigned char *);
extern void reset_plat_dev(unsigned int);
extern void unreset_plat_dev(unsigned int);

#endif  /* #if __DASH_FPGA */
/*------------------------------------------------------------------
$Log: dash_fpga.h,v $
Revision 1.4  2019/07/11 12:31:26  alicehua
Collapse Nutella codes into main trunk.

$Endlog$
*/
