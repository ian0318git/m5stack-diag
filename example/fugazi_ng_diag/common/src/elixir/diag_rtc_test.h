/* $Id: diag_rtc_test.h,v 1.2 2021/09/24 01:21:07 harrchan Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/elixir/diag_rtc_test.h,v $
 *------------------------------------------------------------------
 *
 * diag_rtc_test.h
 *
 * Copyright (c) 2018 - 2019 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#ifndef __DIAG_RTC_TEST_H__
#define __DIAG_RTC_TEST_H__

/* Externs */
extern void diag_rtc_test(boolean);
extern int diag_rtc_init_test(void);
extern int diag_rtc_reg_test(void);
extern int diag_rtc_time_validity_test(void);

#endif   /* __DIAG_RTC_TEST_H__ */

/*-------------------------------------------------
 * $Log: diag_rtc_test.h,v $
 * Revision 1.2  2021/09/24 01:21:07  harrchan
 * Collapse Elixir-branch to Main Trunk.
 *
 * Revision 1.1.2.1  2020/09/09 09:09:53  illiu
 * First version which has been ported with Dreamliner and Marvell CPSS
 *
 * Revision 1.2  2019/01/10 06:36:27  wilbhuan
 * The beginning of Betelgeuse application code.
 *
 *-------------------------------------------------
 */
