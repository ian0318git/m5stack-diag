/* $Id: diag_spi_test.h,v 1.2 2021/09/24 01:21:07 harrchan Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/elixir/diag_spi_test.h,v $
 *------------------------------------------------------------------
 * 
 * diag_spi_test.h
 *
 * Copyright (c) 2018 - 2019 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#define PLAT_BF_BUSNUM   0

extern int quiet_launch;
extern int show_plat_curr_temps(void);

extern int spi_slot_tests(int);
extern int diag_bootflash_test(int);

/*-------------------------------------------------
 * $Log: diag_spi_test.h,v $
 * Revision 1.2  2021/09/24 01:21:07  harrchan
 * Collapse Elixir-branch to Main Trunk.
 *
 * Revision 1.1.2.1  2020/09/09 09:09:53  illiu
 * First version which has been ported with Dreamliner and Marvell CPSS
 *
 * Revision 1.2  2019/01/10 06:36:28  wilbhuan
 * The beginning of Betelgeuse application code.
 *
 *-------------------------------------------------
 */
