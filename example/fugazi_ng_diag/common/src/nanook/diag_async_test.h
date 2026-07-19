/* $Id: diag_async_test.h,v 1.2 2019/12/11 10:10:27 lucywang Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/nanook/diag_async_test.h,v $
 *------------------------------------------------------------------
 *
 * diag_async_test.h - This file is for ASYNC port test
 *
 * Copyright (c) 2019 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */


#ifndef __DIAG_ASYNC_TEST_H__
#define __DIAG_ASYNC_TEST_H__

#define MB_ASYNC_CHAN_NUM                     32
#define DB_ASYNC_CHAN_NUM                     16
#define MB_SCC_BASE                           0x10000000
#define DB_SCC_BASE                           0x20000000
#define MB_XDMA_PATH						"/dev/xdma0_c2h_0"
#define DB_XDMA_PATH						"/dev/xdma1_c2h_0"
#define MB_PCIE_DEVICE_ID                    0x01e7
#define DB_PCIE_DEVICE_ID                    0x01e8

#define MAX_SUBTEST_ITEMS 75

extern int build_mb_async_test_menu(boolean);
extern int build_db_async_test_menu(boolean);

#endif

/*-------------------------------------------------
 * $Log: diag_async_test.h,v $
 * Revision 1.2  2019/12/11 10:10:27  lucywang
 * Merged Nanook to main trunk
 *
 *
 * $Endlog$
 *-------------------------------------------------
 */

