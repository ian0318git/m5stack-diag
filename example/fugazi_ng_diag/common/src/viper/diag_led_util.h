 /* $Id: diag_led_util.h,v 1.2 2018/08/06 02:31:51 harrchan Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/viper/diag_led_util.h,v $
 *------------------------------------------------------------------
 *
 * diag_led_util.h - This file is LED utility header 
 *
 *
 * Copyright (c) 2008-2018 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#ifndef __DIAG_LED_UTIL_H__
#define __DIAG_LED_UTIL_H__

extern int diag_sys_led_util(void);
extern int diag_vpn_led_util(void);
extern int diag_lte_rssi_led_util(void);
extern int diag_lte_sim_led_util(void);
extern int diag_esw_led_util(void);
extern int diag_gephy_led_util(int);
extern int diag_all_leds_off (void);
extern int diag_all_green_leds_on (void);
extern int diag_all_yellow_leds_on (void);

#endif                          /* __DIAG_LED_TEST_H__ */
/*-------------------------------------------------
 * $Log: diag_led_util.h,v $
 * Revision 1.2  2018/08/06 02:31:51  harrchan
 * Merge viper E2E to the main trunk (CSCvk28469)
 *
 * Revision 1.1.2.3  2018/06/27 06:27:49  harrchan
 * Follow the coding rule to clean up code
 *
 * Revision 1.1.2.2  2018/03/29 12:56:06  lucywang
 * Added LED utilities to turn on/off all green/amber LEDs
 *
 * Revision 1.1.2.1  2018/03/26 09:21:03  harrchan
 * Add led utility
 *
 * Revision 1.1.2.1  2018/02/27 08:06:44  harrchan
 * Initial viper application code base
 *
 *
 * $Endlog$
 *-------------------------------------------------
 */
