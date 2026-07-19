/* $Id: diag_led_util.h,v 1.2 2021/09/24 01:21:06 harrchan Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/elixir/diag_led_util.h,v $
 *------------------------------------------------------------------
 *
 * diag_led_util.h
 *
 * Copyright (c) 2018 - 2019 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------------------
 */

#ifndef __DIAG_LED_UTIL_H__
#define __DIAG_LED_UTIL_H__

/* Externs */
extern void diag_led_util(void);
extern int diag_led_all_green_on_util(void);
extern int diag_led_all_yellow_on_util(void);
extern int diag_led_all_off_util(void);

#define LEAVE_MENU    0xF
typedef enum {
    TURN_ON_GREEN = 0,
    TURN_ON_YELLOW,
    TURN_OFF
} phy_led_control_t;

typedef enum {
    SINGLE_LED = 0,
    ALL_LED
} phy_led_mode_t;

typedef enum {
   PHY_PORT0_LED = 0,
   PHY_PORT1_LED,
   PHY_PORT2_LED,
   PHY_PORT3_LED,
   PHY_PORT_MAX_LED
} phy_port_led_t;

#endif   /* __DIAG_LED_UTIL_H__ */

/*-------------------------------------------------
 * $Log: diag_led_util.h,v $
 * Revision 1.2  2021/09/24 01:21:06  harrchan
 * Collapse Elixir-branch to Main Trunk.
 *
 * Revision 1.1.2.2  2021/05/31 10:50:29  illiu
 * Add macro and enum
 *
 * Revision 1.1.2.1  2020/09/09 09:09:51  illiu
 * First version which has been ported with Dreamliner and Marvell CPSS
 *
 * Revision 1.2  2019/01/10 06:36:26  wilbhuan
 * The beginning of Betelgeuse application code.
 *
 *-------------------------------------------------
 */
