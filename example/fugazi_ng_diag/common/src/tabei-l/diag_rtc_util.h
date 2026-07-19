 /* $Id: diag_rtc_util.h,v 1.3 2020/08/06 08:06:04 kehuang2 Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/tabei-l/diag_rtc_util.h,v $
 *------------------------------------------------------------------
 * Filename: diag_rtc_util.h
 *
 * Description: Diag rtc utility header file.
 *
 * Copyright (c) 2018-2020 by cisco Systems, Inc.
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
extern int utility_get_rtc(int);
extern int utility_set_rtc(int);


#endif /* __DIAG_RTC_UTIL_H__ */

/******** History ********
$Log: diag_rtc_util.h,v $
Revision 1.3  2020/08/06 08:06:04  kehuang2
Collapse Promethium into main trunk

Revision 1.2  2019/10/17 02:16:23  kehuang2
Collapse Tabei-L into main trunk

Revision 1.1.2.1  2018/10/02 01:50:00  harrchan
Initial commit for Tabei-L P1A bring up.

$Endlog$
*/
