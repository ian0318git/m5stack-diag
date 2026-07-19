/* $Id: diag_gephy_test.h,v 1.2 2021/04/15 00:52:26 achiu2 Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/phoenix/diag_gephy_test.h,v $
 *------------------------------------------------------------------
 *
 * Filename   : diag_gephy_test.h
 * Description: Header file of GE PHY(Marvell 1514) platform.
 *
 * Copyright (c) 2018-2019 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *-----------------------------------------------------------------------------
 */

#ifndef __DIAG_GE_PHY_TEST_H__
#define __DIAG_GE_PHY_TEST_H__

#define LPBKTEST_PKT_CNT                (3)
#define LPBK_LINK_UP_TOUT               (500)

#define WAITING_DRIVER_FINISH           (2000)

/* Common */
#define REG_BIT(x)  (1 << (x))

extern int build_gephy0_test_menu(boolean);
extern int build_gephy1_test_menu(boolean);

#endif   /* __DIAG_GE_PHY_TEST_H__ */


