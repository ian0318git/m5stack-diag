/* $Id: diag_fpga_test.h,v 1.1 2015/02/26 07:18:29 xiaoyizh Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/wallander/diag_fpga_test.h,v $
 *------------------------------------------------------------------
 * diag_fpga_test.h 
 * 
 * Apr 2014, Xiaoying Zhang
 *
 * Copyright (c) 2014-2015 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */
#ifndef __DIAG_FPGA_TEST_H__
#define __DIAG_FPGA_TEST_H__

/* FPGA interrupt pin is wired to Cavium GPIO bit 10
 */
#define FPGA_INTR_GPIO10_MASK   0x00000400

extern int fpga_test(int);
extern int fpga_do_all_wrapper(void);

#endif
/*-------------------------------------------------
 * $Log: diag_fpga_test.h,v $
 * Revision 1.1  2015/02/26 07:18:29  xiaoyizh
 * Initial check in for Wallander.
 *
 * 
 * $Endlog$
 *-------------------------------------------------
 */
