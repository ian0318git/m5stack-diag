/* $Id: diag_rtc_test.h,v 1.2 2016/04/20 11:25:33 benchen2 Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/tachi_bmc/diag_rtc_test.h,v $
 *------------------------------------------------------------------
 *
 * diag_rtc_test.h - Header file for RTC Test
 *
 * June 2015, Times Huang
 *
 * Copyright (c) 2015-2016 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */
 
#ifndef __DIAG_RTC_TEST__
#define __DIAG_RTC_TEST__

extern int diag_rtc_test (boolean);
extern int diag_rtc_utils (boolean);
extern int rtc_init(int);
extern void display_rtc_wrapper(void);
extern void time_validity_test_wrapper (void);
extern int rtc_register_test_wrapper (void);
extern int utility_display_rtc(int);

#endif /* __DIAG_RTC_TEST__ */

/*---------------------------------------------------------------
$Log: diag_rtc_test.h,v $
Revision 1.2  2016/04/20 11:25:33  benchen2
add tachi fru portion

Revision 1.1.2.3  2016/04/01 08:01:59  benchen2
add rtc/fan info

Revision 1.1.2.2  2015/08/04 03:32:20  meho
Added RTC tests.

Revision 1.1.2.1  2015/06/11 02:01:09  tirawan
Add files for Tachi BMC project


$Endlog$
*/

