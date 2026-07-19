 /* $Id: diag_gephy_test.h,v 1.2 2018/08/06 02:31:50 harrchan Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/viper/diag_gephy_test.h,v $
 *-----------------------------------------------------------------------------
 *
 * Filename   : diag_gephy_test.h
 * Description: Header file of Viper GE PHY(Marvell 1514) platform.
 *
 * Copyright (c) 2018 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *-----------------------------------------------------------------------------
 */

#ifndef __DIAG_GE_PHY_TEST_H__
#define __DIAG_GE_PHY_TEST_H__

#define LPBKTEST_PKT_CNT                (3)
#define LPBK_LINK_UP_TOUT               (500)

/* Common */
#define REG_BIT(x)  (1 << (x))
#define VIPER_MARV_1514_GE0             DNV_GPIO_9
#define SKY_MARV_1514_GE1               DNV_GPIO_6

extern int build_gephy0_test_menu(boolean);
extern int build_gephy1_test_menu(boolean);

#endif   /* __DIAG_GE_PHY_TEST_H__ */


/*-------------------------------------------------
 * $Log: diag_gephy_test.h,v $
 * Revision 1.2  2018/08/06 02:31:50  harrchan
 * Merge viper E2E to the main trunk (CSCvk28469)
 *
 * Revision 1.1.2.1  2018/02/27 08:06:43  harrchan
 * Initial viper application code base
 *
 * $Endlog$
 *-------------------------------------------------
 */

