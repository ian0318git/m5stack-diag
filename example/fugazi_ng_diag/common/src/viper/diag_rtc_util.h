 /* $Id: diag_rtc_util.h,v 1.3 2020/07/14 06:45:56 harrchan Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/viper/diag_rtc_util.h,v $
 *------------------------------------------------------------------
 * Filename: diag_rtc_util.h
 *
 * Description: Diag rtc utility header file.
 *
 * Copyright (c) 2018 by cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */


#ifndef __DIAG_RTC_UTIL_H__
#define __DIAG_RTC_UTIL_H__

#include "diag_rtc_lib.h"

/*
 * Main menu test flag defines
 */
#define RTC_MM_1    (MF_CONTINUOUS)
#define RTC_MM_2    (RTC_MM_1 | MF_DOALL)
#define RTC_MM_3    (RTC_MM_2 | MF_SHOW_ERRCOUNT)

extern int build_rtc_utils_menu(boolean rtc_util_items_executed);
extern int utility_set_rtc(int);
extern int utility_get_rtc(int);


#endif /* __DIAG_RTC_UTIL_H__ */

/******** History ********
$Log: diag_rtc_util.h,v $
Revision 1.3  2020/07/14 06:45:56  harrchan
Fixed bug of RTC set/display utility(CSCvu92708)

Revision 1.2  2018/08/06 02:31:51  harrchan
Merge viper E2E to the main trunk (CSCvk28469)

Revision 1.1.2.2  2018/06/06 03:16:07  lucywang
Fixed CSCvj80723-RTC register test failed

Revision 1.1.2.1  2018/03/29 01:11:20  lucywang
Added RTC test and utility


$Endlog$
*/
