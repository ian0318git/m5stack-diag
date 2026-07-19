/* $Id: diag_sirius_fpga_test.h,v 1.2 2019/01/10 06:36:27 wilbhuan Exp $ 
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/betelgeuse/diag_sirius_fpga_test.h,v $
 *------------------------------------------------------------------
 * 
 * diag_sirius_fpga_test.h
 *
 * Copyright (c) 2018 - 2019 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#ifndef __DIAG_PLUG_FPGA_H__
#define __DIAG_PLUG_FPGA_H__
 
#define PLUG_FPGA_REG_WIDTH                          (4)
/* Check with HW 1ms enough */
#define PLUG_INT_ACT_WAIT                            (1)

extern int diag_plug_fpga_test(int);

#endif

/*-------------------------------------------------
 * $Log: diag_sirius_fpga_test.h,v $
 * Revision 1.2  2019/01/10 06:36:27  wilbhuan
 * The beginning of Betelgeuse application code.
 *
 *-------------------------------------------------
 */
