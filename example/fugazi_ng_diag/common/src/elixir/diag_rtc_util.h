/* $Id: diag_rtc_util.h,v 1.2 2021/09/24 01:21:07 harrchan Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/elixir/diag_rtc_util.h,v $
 *------------------------------------------------------------------
 *
 * diag_rtc_util.h
 *
 * Copyright (c) 2018 - 2019 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#ifndef __DIAG_RTC_UTIL_H__
#define __DIAG_RTC_UTIL_H__

/* Externs */
extern void build_rtc_utils_menu(void);
extern int display_rtc_date_time_util(void);
extern int set_rtc_date_time_util(void);
extern int alter_rtc_reg_util(void);

#endif   /* __DIAG_RTC_UTIL_H__ */

/*-------------------------------------------------
 * $Log: diag_rtc_util.h,v $
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
