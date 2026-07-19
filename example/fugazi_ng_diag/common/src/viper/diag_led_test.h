 /* $Id: diag_led_test.h,v 1.2 2018/08/06 02:31:50 harrchan Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/viper/diag_led_test.h,v $
 *------------------------------------------------------------------
 *
 * diag_led_test.h - 
 *
 *
 * Copyright (c) 2008-2018 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#ifndef __DIAG_LED_TEST_H__
#define __DIAG_LED_TEST_H__

extern int diag_led_test(boolean);
extern int build_led_util_menu (boolean);

#define GEPHY0 0
#define GEPHY1 1

#endif                          /* __DIAG_LED_TEST_H__ */
/*-------------------------------------------------
 * $Log: diag_led_test.h,v $
 * Revision 1.2  2018/08/06 02:31:50  harrchan
 * Merge viper E2E to the main trunk (CSCvk28469)
 *
 * Revision 1.1.2.3  2018/04/17 11:16:25  lucywang
 * Fixed LED issue for ViperJ
 *
 * Revision 1.1.2.2  2018/03/26 09:21:03  harrchan
 * Add led utility
 *
 * Revision 1.1.2.1  2018/02/27 08:06:44  harrchan
 * Initial viper application code base
 *
 *
 * $Endlog$
 *-------------------------------------------------
 */
