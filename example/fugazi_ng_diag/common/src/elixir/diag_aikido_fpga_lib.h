/* $Id: diag_aikido_fpga_lib.h,v 1.2 2021/09/24 01:21:05 harrchan Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/elixir/diag_aikido_fpga_lib.h,v $
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
 * Revision 1.2  2021/09/24 01:21:05  harrchan
 * Collapse Elixir-branch to Main Trunk.
 *
 * Revision 1.1.2.1  2020/09/09 09:09:50  illiu
 * First version which has been ported with Dreamliner and Marvell CPSS
 *
 * Revision 1.2  2019/01/10 06:36:25  wilbhuan
 * The beginning of Betelgeuse application code.
 *
 *-------------------------------------------------
 */
