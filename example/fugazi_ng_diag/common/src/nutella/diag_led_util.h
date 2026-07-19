/* $Id: diag_led_util.h,v 1.5 2019/10/16 23:50:47 alicehua Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/nutella/diag_led_util.h,v $
 *------------------------------------------------------------------
 *
 * diag_led_util.h - This file is LED utility header 
 *
 *
 * Copyright (c) 2008-2019 by Cisco Systems, Inc.
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
extern int diag_i350_led_util(void);
extern int diag_i350_led_on_off(int, boolean);
extern int diag_port80_led_util(void);
extern int diag_all_led_on_util(int);
extern int diag_all_led_off_util(int);

#endif                          /* __DIAG_LED_TEST_H__ */
/*-------------------------------------------------
$Log: diag_led_util.h,v $
Revision 1.5  2019/10/16 23:50:47  alicehua
CSCvr68092: Add LED utility (turn on/off all LED).

Revision 1.4  2019/07/11 12:31:28  alicehua
Collapse Nutella codes into main trunk.

$Endlog$
*/
