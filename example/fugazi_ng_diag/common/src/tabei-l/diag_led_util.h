 /* $Id: diag_led_util.h,v 1.4 2019/12/30 06:02:00 kehuang2 Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/tabei-l/diag_led_util.h,v $
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

extern int diag_env_led_util(void);
extern int diag_status_led_util(void);
extern int diag_I350_RJ45_led_util(void);
extern int diag_I350_SFP_led_util(void);
extern int diag_hdd_led_util(void);
extern int diag_gephy_led_util(int);
extern int diag_port80_led_util(void);
extern int diag_all_led_util (void);
extern int access_port80_led(int, int);
extern int access_gephy_led(int, int);

#endif                          /* __DIAG_LED_TEST_H__ */
/*-------------------------------------------------
 * $Log: diag_led_util.h,v $
 * Revision 1.4  2019/12/30 06:02:00  kehuang2
 * CSCvs55860: Support All LED ON/OFF
 *
 * Revision 1.3  2019/11/25 08:55:52  kehuang2
 * Collapse Tabei-L into main trunk
 *
 * Revision 1.2  2019/10/17 02:16:22  kehuang2
 * Collapse Tabei-L into main trunk
 *
 * Revision 1.1.2.5  2019/05/21 09:18:51  kehuang2
 * Support Port80 LED
 *
 * Revision 1.1.2.4  2019/04/19 03:15:29  kehuang2
 * 1.Support CPLD access 2.Support new FPGA 3.Clean up code
 *
 * Revision 1.1.2.3  2018/12/26 03:48:33  harrchan
 * LED Test
 *
 * Revision 1.1.2.2  2018/11/16 05:42:11  olin2
 * Clean up code
 *
 * Revision 1.1.2.1  2018/10/02 01:49:59  harrchan
 * Initial commit for Tabei-L P1A bring up.
 *
 * $Endlog$
 *-------------------------------------------------
 */
