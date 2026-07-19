/* $Id: diag_cpu_test.h,v 1.2 2021/09/24 01:21:05 harrchan Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/elixir/diag_cpu_test.h,v $
 *------------------------------------------------------------------
 * 
 * diag_cpu_test.h
 *
 * Copyright (c) 2018 - 2019 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#ifndef __DIAG_CPU_TEST_H__
#define __DIAG_CPU_TEST_H__

#define NUMBER_OF_PROCESS 400

/* Externs */
extern void diag_cpu_test(boolean);
extern int diag_cpu_core_test(boolean);

#endif /* __DIAG_CPU_TEST_H__ */

/*-------------------------------------------------
 * $Log: diag_cpu_test.h,v $
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
