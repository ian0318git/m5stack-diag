/* $Id: diag_led_test.h,v 1.5 2019/10/16 23:50:47 alicehua Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/nutella/diag_led_test.h,v $
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

#define GEPHYP0 0
#define GEPHYP1 1
#define GEPHYP2 2
#define GEPHYP3 3

#define PORT80_ADDR           0x0080
#define ACCESS_PORT_NUM       1
#define PORT80_ACCESS_ON      1
#define PORT80_ACCESS_OFF     0

#define PORT80_LED_ON         1
#define PORT80_LED_OFF        1
#define PORT80_LED_ALL_OFF    0x00

#define GREEN  0
#define YELLOW 1

enum port80_led_bit {
    PORT80_BIT0 = 0,
    PORT80_BIT1,
    PORT80_BIT2,
    PORT80_BIT3,
    PORT80_BIT4,
    PORT80_BIT5,
    PORT80_BIT6,
    PORT80_BIT7
};

#endif                          /* __DIAG_LED_TEST_H__ */
/*-------------------------------------------------
$Log: diag_led_test.h,v $
Revision 1.5  2019/10/16 23:50:47  alicehua
CSCvr68092: Add LED utility (turn on/off all LED).

Revision 1.4  2019/07/11 12:31:28  alicehua
Collapse Nutella codes into main trunk.

$Endlog$
*/
