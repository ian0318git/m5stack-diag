/* $Id: diag_led_util.h,v 1.2 2021/04/15 00:52:26 achiu2 Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/phoenix/diag_led_util.h,v $
 *------------------------------------------------------------------
 *
 * diag_led_util.h - This file is LED utility header 
 *
 *
 * Copyright (c) 2008-2020 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#ifndef __DIAG_LED_UTIL_H__
#define __DIAG_LED_UTIL_H__

extern int diag_psu_led_util(void);
extern int diag_pwr_led_util(void);
extern int diag_stat_led_util(void);
extern int diag_fan_led_util(void);
extern int diag_temp_led_util(void);
extern int diag_ssd_led_util(void);
extern int diag_console_led_util(void);
extern int diag_I350_RJ45_led_util(void);
extern int diag_port80_led_util(void);
extern int diag_debug_led_util(void);

#endif /* __DIAG_LED_TEST_H__ */
