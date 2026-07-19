 /* $Id: diag_rtc_test.h,v 1.2 2018/08/06 02:31:51 harrchan Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/viper/diag_rtc_test.h,v $
 *------------------------------------------------------------------
 * Filename: diag_rtc_test.h
 *
 * Description: Diag rtc test header file.
 *
 * Copyright (c) 2018 by cisco Systems, Inc.
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
Revision 1.2  2018/08/06 02:31:51  harrchan
Merge viper E2E to the main trunk (CSCvk28469)

Revision 1.1.2.3  2018/07/03 05:38:55  harrchan
Follow the coding rule to clean up code

Revision 1.1.2.2  2018/06/06 03:16:07  lucywang
Fixed CSCvj80723-RTC register test failed

Revision 1.1.2.1  2018/03/29 01:11:20  lucywang
Added RTC test and utility


$Endlog$
*/
