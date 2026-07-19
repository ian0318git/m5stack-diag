 /* $Id: diag_gephy_test.h,v 1.2 2019/10/17 02:16:21 kehuang2 Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/tabei-l/diag_gephy_test.h,v $
 *-----------------------------------------------------------------------------
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


/*-------------------------------------------------
 * $Log: diag_gephy_test.h,v $
 * Revision 1.2  2019/10/17 02:16:21  kehuang2
 * Collapse Tabei-L into main trunk
 *
 * Revision 1.1.4.3  2018/11/16 05:42:11  olin2
 * Clean up code
 *
 * Revision 1.1.4.2  2018/10/02 01:49:59  harrchan
 * Initial commit for Tabei-L P1A bring up.
 *
 * $Endlog$
 *-------------------------------------------------
 */

