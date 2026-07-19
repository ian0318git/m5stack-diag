/* $Id: diag_gephy_1543_test.h,v 1.2 2019/12/11 10:10:30 lucywang Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/nanook/diag_gephy_1543_test.h,v $
 *-----------------------------------------------------------------------------
 *
 * Filename   : diag_gephy_test.h
 * Description: Header file of GE PHY platform.
 *
 * Copyright (c) 2019 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *-----------------------------------------------------------------------------
 */

#ifndef __DIAG_GE_PHY_1543_TEST_H__
#define __DIAG_GE_PHY_1543_TEST_H__

#define LPBKTEST_PKT_CNT                (3)
#define LPBK_LINK_UP_TOUT               (1000)
#define CHECK_LINK_UP_DELAY             (10)
#define SET_PHY_DELAY                   (1000)
#define SLEEP_1MS                       (1)

/* Common */
#define NANOOK_PHY_QSGMII_PORT0               8
#define NANOOK_PHY_AUTO_PORT0                 9
#define NANOOK_PHY_QSGMII_PORT1               10
#define NANOOK_PHY_AUTO_PORT1                 11
#define REG_BIT(x)  (1 << (x))

enum {
    SFP0,
    SFP1
};

enum {
    GE0,
    GE1
};

extern int build_gephy_1543_test_menu(boolean);
extern int diag_gephy_1543_sfp_force_100 (void);
extern int check_sfp_speed_100(int);

#endif   /* __DIAG_GE_PHY_1543_TEST_H__ */


/*-------------------------------------------------
$Log: diag_gephy_1543_test.h,v $
Revision 1.2  2019/12/11 10:10:30  lucywang
Merged Nanook to main trunk


$Endlog$
*/
