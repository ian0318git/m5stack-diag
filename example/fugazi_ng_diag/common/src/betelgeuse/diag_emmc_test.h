/* $Id: diag_emmc_test.h,v 1.2 2019/01/10 06:36:26 wilbhuan Exp $ 
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/betelgeuse/diag_emmc_test.h,v $
 *------------------------------------------------------------------
 *
 * diag_emmc_test.h
 *
 * Copyright (c) 2018 - 2019 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------------------
 */

#ifndef __DIAG_EMMC_TEST_H__
#define __DIAG_EMMC_TEST_H__

/* Extern */
extern boolean emmc_force_stop;
extern void diag_emmc_test(boolean);
extern int diag_emmc_rw_test(void);
extern int diag_emmc_full_size_rw_test(void);

#endif   /* __DIAG_EMMC_TEST_H__ */

/*-------------------------------------------------
 * $Log: diag_emmc_test.h,v $
 * Revision 1.2  2019/01/10 06:36:26  wilbhuan
 * The beginning of Betelgeuse application code.
 *
 *-------------------------------------------------
 */
