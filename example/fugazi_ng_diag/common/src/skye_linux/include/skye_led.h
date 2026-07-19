/* $Id: skye_led.h,v 1.2 2015/05/25 03:59:11 steja Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/skye_linux/include/skye_led.h,v $
 *------------------------------------------------------------------
 * Filename: skye_led.h
 *
 * Description: Header file of Skye LEDs.
 *
 * Jul 2014 - Paul Lin(palin2) ported from Shrinkray.
 *
 * Copyright (c) 2013-2015 by cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#ifndef __SHRINKRAY_LED_H__
#define __SHRINKRAY_LED_H__

/* Common */
/* LED light Type */
#define AMBER_LED   0x2


/* LED Control Type */
#define LED_OFF   0x0
#define LED_ON    0x1
#define LED_BLINK 0x2

/* Definition of Skye GE LED control type */
#define GE_LINK_GREEN_SOLID     0
#define GE_LINK_GREEN_BLINK     1
#define GE_LINK_OFF             2
#define GE_SPEED_GREEN_SOLID    3
#define GE_SPEED_AMBER_SOLID    4
#define GE_SPEED_OFF            5

/* Definition of Skye FPGA LED control type */
#define SYS_LED_GREEN_BLINK     1
#define SYS_LED_YELLOW_BLINK    2
#define SYS_LED_GREEN_SOLID     3
#define SYS_LED_YELLOW_SOLID    4
#define SYS_LED_TURN_OFF        5
#define EUSB_LED_TEST_DIS       6
#define EUSB_LED_TURN_ON        7
#define EUSB_LED_TURN_OFF       8


/* Externs */
extern int  skye_ge_led_ctrl(int);
extern void skye_ge_led_util(int);
extern void skye_fpga_led_util(int);


#endif /* __SHRINKRAY_LED_H__ */

/*------------------------------------------------------------------
$Log: skye_led.h,v $
Revision 1.2  2015/05/25 03:59:11  steja
Add Support Skye SM

Revision 1.1.4.2  2015/04/29 11:36:28  steja
Code check-in to skye-branch2 for ER code review


-------------------------------------------------------------------
Revision 1.1.2.1  2014/07/21 01:56:39  palin2
Initial check-in Skye module side Diag code.

--------------------------------------------------------------------
shrinkray_led.h:
Revision 1.2.8.1  2014/06/06 11:54:20  steja
Add Shrinkray LED Test

Revision 1.2  2014/02/27 15:01:09  palin2
Initial check-in ShrinkRay SM side Diag code.

Revision 1.1.4.2  2013/09/13 07:00:00  palin2
Initial check-in ShrinkRay SM side Diag code.

Revision 1.1.2.1  2013/08/14 11:34:31  palin2
Initial check-in for ShrinkRay LED related test and utilities.

--------------------------------------------------------------------
$Endlog$
*/

