/* $Id: diag_cpu_util.h,v 1.2 2021/09/24 01:21:05 harrchan Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/elixir/diag_cpu_util.h,v $
 *------------------------------------------------------------------
 * 
 * diag_cpu_util.h
 *
 * Copyright (c) 2018 - 2019 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#ifndef __DIAG_CPU_UTIL_H__
#define __DIAG_CPU_UTIL_H__


/* Externs */
extern void diag_cpu_util(void);
extern void diag_cpu_system_show_devbus_info_util(void);
extern void diag_cpu_system_ecc_err_injection_util(void);
extern int diag_cpu_reg_rd_util(int);
extern int diag_cpu_reg_wr_util(int);
extern int diag_cpu_ondie_temp_util(int);

#endif /* __DIAG_CPU_UTIL_H__ */

/*-------------------------------------------------
 * $Log: diag_cpu_util.h,v $
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
