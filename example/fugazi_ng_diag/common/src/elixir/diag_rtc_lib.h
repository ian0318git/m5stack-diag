/* $Id: diag_rtc_lib.h,v 1.2 2021/09/24 01:21:07 harrchan Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/elixir/diag_rtc_lib.h,v $
 *------------------------------------------------------------------
 *
 * diag_rtc_lib.h
 *
 * Copyright (c) 2018 - 2019 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#ifndef __DIAG_RTC_LIB_H__
#define __DIAG_RTC_LIB_H__

/* Common */
#define POLL_INVL_10MS          10     /* 10ms */
#define POLL_TIME_1SEC          1      /* 1sec */
#define SEC_TO_MS               1000   /* 1sec = 1000ms */
#define POLL_TIME_1SEC_IN_MS    (POLL_TIME_1SEC * SEC_TO_MS)
#define POLL_1SEC_W_10MS_INVL_COUNT   (POLL_TIME_1SEC_IN_MS / POLL_INVL_10MS)

/* Externs */
extern int diag_rtc_dev_create(dev_ds1337_object_t *, n2g_i2c_if_t *);
extern int clear_rtc_osf_bit(void);

#endif   /* __DIAG_RTC_LIB_H__ */

/*-------------------------------------------------
 * $Log: diag_rtc_lib.h,v $
 * Revision 1.2  2021/09/24 01:21:07  harrchan
 * Collapse Elixir-branch to Main Trunk.
 *
 * Revision 1.1.2.1  2020/09/09 09:09:53  illiu
 * First version which has been ported with Dreamliner and Marvell CPSS
 *
 * Revision 1.2  2019/01/10 06:36:27  wilbhuan
 * The beginning of Betelgeuse application code.
 *
 *-------------------------------------------------
 */
