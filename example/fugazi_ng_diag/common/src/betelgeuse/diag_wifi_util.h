/* $Id: diag_wifi_util.h,v 1.2 2019/01/10 06:36:28 wilbhuan Exp $ 
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/betelgeuse/diag_wifi_util.h,v $
 *------------------------------------------------------------------
 *
 * diag_wifi_util.h
 *
 * Copyright (c) 2018 - 2019 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------------------
 */

#ifndef __DIAG_WIFI_UTIL_H__
#define __DIAG_WIFI_UTIL_H__

/* Extern */
extern int diag_wifi_util(void);
extern int wifi_console_redirect(void);
extern int wifi_ready_bit_check(void);
extern int wifi_temp_sensor_show_reg(void);
extern int wifi_temp_sensor_alter_reg(void);
extern int wifi_temp_sensor_dump_reg(void);
extern int wifi_temp_sensor_show_temp(void);
extern int wifi_led_control_util(void);
extern int reset_wifi(void);

#endif   /* __DIAG_WIFI_UTIL_H__ */

/*-------------------------------------------------
 * $Log: diag_wifi_util.h,v $
 * Revision 1.2  2019/01/10 06:36:28  wilbhuan
 * The beginning of Betelgeuse application code.
 *
 *-------------------------------------------------
 */
