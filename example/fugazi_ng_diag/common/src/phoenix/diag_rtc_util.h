/* $Id: diag_rtc_util.h,v 1.2 2021/04/15 00:52:26 achiu2 Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/phoenix/diag_rtc_util.h,v $
 *------------------------------------------------------------------
 * Filename: diag_rtc_util.h
 *
 * Description: Diag rtc utility header file.
 *
 * Copyright (c) 2018-2019 by cisco Systems, Inc.
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

