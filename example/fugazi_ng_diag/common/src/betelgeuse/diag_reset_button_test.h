/* $Id: diag_reset_button_test.h,v 1.2 2019/01/10 06:36:27 wilbhuan Exp $ 
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/betelgeuse/diag_reset_button_test.h,v $
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
 * Revision 1.2  2019/01/10 06:36:27  wilbhuan
 * The beginning of Betelgeuse application code.
 *
 *-------------------------------------------------
 */
