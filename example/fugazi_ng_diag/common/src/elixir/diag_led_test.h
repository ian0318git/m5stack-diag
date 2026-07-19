/* $Id: diag_led_test.h,v 1.2 2021/09/24 01:21:06 harrchan Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/elixir/diag_led_test.h,v $
 *------------------------------------------------------------------
 *
 * diag_led_test.h
 *
 * Copyright (c) 2018 - 2019 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------------------
 */

#ifndef __DIAG_LED_TEST_H__
#define __DIAG_LED_TEST_H__

/* Externs */
extern void diag_led_test(boolean);
extern int diag_all_led_test(void);

#define LED_TEST_PERIOD 1

#endif   /* __DIAG_LED_TEST_H__ */

/*-------------------------------------------------
 * $Log: diag_led_test.h,v $
 * Revision 1.2  2021/09/24 01:21:06  harrchan
 * Collapse Elixir-branch to Main Trunk.
 *
 * Revision 1.1.2.1  2020/09/09 09:09:51  illiu
 * First version which has been ported with Dreamliner and Marvell CPSS
 *
 * Revision 1.2  2019/01/10 06:36:26  wilbhuan
 * The beginning of Betelgeuse application code.
 *
 *-------------------------------------------------
 */
