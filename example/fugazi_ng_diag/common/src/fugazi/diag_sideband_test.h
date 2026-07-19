/* $Id: diag_sideband_test.h,v 1.2 2021/06/02 08:22:34 iachang Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/fugazi/diag_sideband_test.h,v $
 *------------------------------------------------------------------
 *
 * diag_sideband_test.h - Fugazi Side Band Diag test definitions.
 *
 * Mar. 2020, Ian Chang <iachang@cisco.com>
 *
 * Copyright (c) 2020 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#ifndef __FUGAZI_SIDEBAND_TEST_H__
#define __FUGAZI_SIDEBAND_TEST_H__

#define SIDEBAND_ASSERT_TIME    100
#define SIDEBAND_TIMEOUT        10
extern int fugazi_sideband_test(int);
#endif /* __FUGAZI_SIDEBAND_TEST_H__ */
/*
 *------------------------------------------------------------------
 * $Log: diag_sideband_test.h,v $
 * Revision 1.2  2021/06/02 08:22:34  iachang
 * CSCvo59196-33 : Merge Fugazi from ASR1K-main-branch to ISR main trunk
 *
 * Revision 1.1.4.2  2020/08/26 02:37:49  iachang
 * Merge Fugazi code into main trunk
 *
 * Revision 1.1.2.2  2020/07/24 03:44:58  iachang
 * tx_dis chang, add time out for rx_los checking
 *
 * Revision 1.1.2.1  2020/03/19 06:31:41  iachang
 * Support Fugazi Side Band test
 *
 *
 *
 *------------------------------------------------------------------
 * $Endlog$
 */
