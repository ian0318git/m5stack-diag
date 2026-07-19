/* $Id: diag_aikido_fpga_lib.h,v 1.2 2019/01/10 06:36:25 wilbhuan Exp $ 
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/betelgeuse/diag_aikido_fpga_lib.h,v $
 *------------------------------------------------------------------
 * 
 * diag_aikido_fpga_lib.h
 *
 * Copyright (c) 2018 - 2019 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

int aikido_read_32_reg(uint, uint *);
int aikido_write_32_reg(uint, uint);

/*-------------------------------------------------
 * $Log: diag_aikido_fpga_lib.h,v $
 * Revision 1.2  2019/01/10 06:36:25  wilbhuan
 * The beginning of Betelgeuse application code.
 *
 *-------------------------------------------------
 */
