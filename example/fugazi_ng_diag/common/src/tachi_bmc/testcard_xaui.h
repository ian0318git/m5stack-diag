/* $Id: testcard_xaui.h,v 1.2 2016/04/20 11:25:33 benchen2 Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/tachi_bmc/testcard_xaui.h,v $
 *--------------------------------------------------------------------
 * Filename   : testcard_xaui.h
 *
 * Description: Head file for TestCard XAUI related definition.
 *
 * Copyright (c) 2013-2016 by Cisco Systems, Inc.
 * All rights reserved.
 *--------------------------------------------------------------------
 */

#ifndef __TESTCARD_XAUI_H__
#define __TESTCARD_XAUI_H__

/* Common */
#define XAUI_MAX_RETRY      10

/* SMI PHY Address */
#define TC_XAUI_PHY_ADDR    0x0
#define NEW_TC_XAUI_PHY_ADDR    0x1

/* Device Type Option */
#define PMAPMD_OPT   1
#define PCS_OPT      2
#define PHYXS_OPT    3
#define CL37AN_OPT   4

/* Device Type */
#define BCM8727_PHY_PMAPMD   0x1
#define BCM8727_PHY_PCS      0x3
#define BCM8727_PHY_PHYXS    0x4
#define BCM8727_PHY_CL37AN   0x7

/* Loopback Type */
#define BCM8727_EXT_LPBK        0x1
#define BCM8727_SYS_LPBK        0x2
#define BCM8727_IND_LANE_LPBK   0x3
#define BCM8727_PCSPMD_LPBK     0x4
#define BCM8727_XGXS_LPBK       0x5
#define BCM8727_LINE_LPBK       0x6
#define BCM8727_PRBS_LPBK       0x7

/* Register Definition of BCM8727 */
/* PMA/PMD Control Reg. (1.0000) */
#define PMAPMD_CTRL_REG_OFF      0x0000
#define PMAPMD_CTRL_RST          0x8000
#define PMAPMD_CTRL_SPEED_SEL1   0x2000
#define PMAPMD_CTRL_LOW_PWR      0x0800
#define PMAPMD_CTRL_SPEED_SEL2   0x0040
#define PMAPMD_CTRL_SPEED_MSK    0x003C
#define PMAPMD_CTRL_PMA_LPBK     0x0001

/* Lock Detect Reg. (1.CA7B) */
#define BCM8727_LOCK_DET_REG_OFF   0xCA7B
#define BCM8727_LOCK_DET_BYPASS    0x4000


/* PCS Control 1 Reg. (3.0000) */
#define PCS_CTRL1_REG_OFF      0x0000
#define PCS_CTRL1_RST          0x8000
#define PCS_CTRL1_LPBK         0x4000
#define PCS_CTRL1_SPEED_SEL1   0x2000
#define PCS_CTRL1_LOW_PWR      0x0800
#define PCS_CTRL1_SPEED_SEL2   0x0040
#define PCS_CTRL1_SPEED_MSK    0x003C


/* Externs */
extern int tc_xaui_phy_reg_rd(int, uint16_t, uint16_t, uint16_t *);
extern int tc_xaui_phy_reg_wr(int, uint16_t, uint16_t, uint16_t);
extern int new_tc_xaui_lpbk_test(void);

#endif /* __TESTCARD_XAUI_H__ */

/* ------- End of file ------- */

/******** History ******** 
$Log: testcard_xaui.h,v $
Revision 1.2  2016/04/20 11:25:33  benchen2
add tachi fru portion

Revision 1.1.2.1  2015/07/31 10:42:41  alpeng
add testcard xaui header file

Revision 1.4  2014/07/25 01:36:57  alpeng
support xaui loopback and sort out the test item for new testcard

Revision 1.3  2014/07/22 09:40:13  alpeng
support 10g-kr on ge0 and 1g-kx on ge1 for new testcard

Revision 1.2  2014/07/02 08:09:43  alpeng
add new testcard id for en/disable menu item and select smi addr

Revision 1.1  2013/05/09 05:42:40  alpeng
moving overlord common code from x86

Revision 1.2  2012/09/14 08:22:32  palin2
1. Add Test "XAUI PCS/PMD internal loopback test" support.
2. Add Utility "Alter XAUI PHY register" support.
3. Use XAUI PCS/PMD internal loopback test as default test.

Revision 1.1  2012/08/22 16:32:50  palin2
First check-in to supprt XAUI test on TestCard.

$Endlog$
*/

