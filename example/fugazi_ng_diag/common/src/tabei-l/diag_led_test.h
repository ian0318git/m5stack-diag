 /* $Id: diag_led_test.h,v 1.3 2019/12/30 06:02:00 kehuang2 Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/tabei-l/diag_led_test.h,v $
 *------------------------------------------------------------------
 *
 * diag_led_test.h - 
 *
 *
 * Copyright (c) 2008-2019 by Cisco Systems, Inc.
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

#define PORT80_ADDR           0x0080
#define ACCESS_PORT_NUM       1
#define PORT80_ACCESS_ON      1
#define PORT80_ACCESS_OFF     0
#define GEPHY_ACCESS_ON       1
#define GEPHY_ACCESS_OFF      0


#define PORT80_LED_ON         1
#define PORT80_LED_OFF        1
#define PORT80_LED_ALL_ON     0xFF
#define PORT80_LED_ALL_OFF    0x00

enum port80_led_bit {
    PORT80_BIT0 = 0,
    PORT80_BIT1,
    PORT80_BIT2,
    PORT80_BIT3,
    PORT80_BIT4,
    PORT80_BIT5,
    PORT80_BIT6,
    PORT80_BIT7,
    PORT80_ALL
};

#endif                          /* __DIAG_LED_TEST_H__ */
/*-------------------------------------------------
 * $Log: diag_led_test.h,v $
 * Revision 1.3  2019/12/30 06:02:00  kehuang2
 * CSCvs55860: Support All LED ON/OFF
 *
 * Revision 1.2  2019/10/17 02:16:22  kehuang2
 * Collapse Tabei-L into main trunk
 *
 * Revision 1.1.4.3  2019/05/21 09:18:51  kehuang2
 * Support Port80 LED
 *
 * Revision 1.1.4.2  2018/10/02 01:49:59  harrchan
 * Initial commit for Tabei-L P1A bring up.
 *
 * $Endlog$
 *-------------------------------------------------
 */
