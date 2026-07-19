/* $Id: diag_lasi_test.h,v 1.2 2021/06/02 08:22:34 iachang Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/fugazi/diag_lasi_test.h,v $
 *------------------------------------------------------------------
 *
 * diag_lasi_test.h - Fugazi LASI Diag test definitions.
 *
 * Mar. 2020, Ian Chang <iachang@cisco.com>
 *
 * Copyright (c) 2020 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#ifndef __FUGAZI_LASI_TEST_H__
#define __FUGAZI_LASI_TEST_H__

#define SIOCSMIILASI                0x89F1  /* IOCTL : Read LASI event count. */
#define LASI_EVENT                  0x1     /* LASI event counter */

extern int fugazi_lasi_test(int);
#endif /* __FUGAZI_LASI_TEST_H__ */
/*
 *------------------------------------------------------------------
 * $Log: diag_lasi_test.h,v $
 * Revision 1.2  2021/06/02 08:22:34  iachang
 * CSCvo59196-33 : Merge Fugazi from ASR1K-main-branch to ISR main trunk
 *
 * Revision 1.1.4.2  2020/08/26 02:37:48  iachang
 * Merge Fugazi code into main trunk
 *
 * Revision 1.1.2.2  2020/07/20 00:52:32  iachang
 * Define LASI_EVENT macro
 *
 * Revision 1.1.2.1  2020/03/18 06:51:44  iachang
 * Create independent file for LASI test
 *
 *
 *------------------------------------------------------------------
 * $Endlog$
 */
