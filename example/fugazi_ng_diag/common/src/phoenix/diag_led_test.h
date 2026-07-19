/* $Id: diag_led_test.h,v 1.2 2021/04/15 00:52:26 achiu2 Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/phoenix/diag_led_test.h,v $
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

#define PORT80_ADDR           0x0080
#define ACCESS_PORT_NUM       1
#define PORT80_ACCESS_ON      1
#define PORT80_ACCESS_OFF     0

#define PORT80_LED_ON         1
#define PORT80_LED_OFF        1
#define PORT80_LED_ALL_OFF    0x00

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

/* For Phoenix */
enum {
    E_PSU_LED_TEST = 0,
    E_STAT_LED_TEST,
    E_FAN_LED_TEST,
    E_TEMP_LED_TEST,
    E_SSD_LED_TEST,
    E_CONSOLE_LED_TEST,
};

#endif                          /* __DIAG_LED_TEST_H__ */
