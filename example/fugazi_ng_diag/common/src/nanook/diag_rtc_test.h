 /* $Id: diag_rtc_test.h,v 1.2 2019/12/11 10:10:31 lucywang Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/nanook/diag_rtc_test.h,v $
 *------------------------------------------------------------------
 * Filename: diag_rtc_test.h
 *
 * Description: Diag rtc test header file.
 *
 * Copyright (c) 2019 by cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#ifndef __DIAG_RTC_TEST_H__
#define __DIAG_RTC_TEST_H__

#include "diag_rtc_lib.h"

/*
 * Main menu test flag defines
 */
#define RTC_MM_1    (MF_CONTINUOUS)
#define RTC_MM_2    (RTC_MM_1 | MF_DOALL)
#define RTC_MM_3    (RTC_MM_2 | MF_SHOW_ERRCOUNT)

#define BUF_MAX	256
#define GET_SYS_RTC_TIME	1
#define GET_SYS_RTC_DATE	2
#define SET_HW_DATE			3
#define SET_HW_TIME			4
#define GET_HW_DATE			5
#define GET_HW_TIME			6
#define SET_HW_CLOCK		7

#define RTC_TEST_WAIT_TIME  3000

extern int rtc_test_regs(void);
extern int build_rtc_menu(boolean rtc_items_executed);
extern int diag_rtc_exec(int test, unsigned int *param1, unsigned int *param2, unsigned int *param3, char *param4);

#endif /* __DIAG_RTC_TEST_H__ */

/******** History ********
$Log: diag_rtc_test.h,v $
Revision 1.2  2019/12/11 10:10:31  lucywang
Merged Nanook to main trunk


$Endlog$
*/
