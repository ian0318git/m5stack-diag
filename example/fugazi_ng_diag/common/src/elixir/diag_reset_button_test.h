/* $Id: diag_reset_button_test.h,v 1.2 2021/09/24 01:21:07 harrchan Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/elixir/diag_reset_button_test.h,v $
 *------------------------------------------------------------------
 * 
 * diag_reset_button_test.h
 *
 * Copyright (c) 2018 - 2019 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#ifndef __DIAG_RESET_BUTTON_TEST_H__
#define __DIAG_RESET_BUTTON_TEST_H__

#define SEC_TO_MICROSEC        1000000
#define MAX_POLLINGTIME_USEC   60000000   /* 60sec */
#define MAX_CHECKTIME_USEC     5000000    /* 5sec */

/* Extern */
extern int diag_reset_button_test(void);

#endif /* __DIAG_RESET_BUTTON_TEST_H__ */

/*-------------------------------------------------
 * $Log: diag_reset_button_test.h,v $
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
