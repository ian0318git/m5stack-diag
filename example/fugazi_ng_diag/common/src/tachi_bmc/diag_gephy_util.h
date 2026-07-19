/* $Id: diag_gephy_util.h,v 1.3 2017/03/30 08:30:53 hondwang Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/tachi_bmc/diag_gephy_util.h,v $
 *------------------------------------------------------------------
 *
 * diag_gephy_util.h - Header file for GE PHY Utility
 *
 * June 2015, Times Huang
 *
 * Copyright (c) 2017 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */
 
#ifndef __DIAG_GEPHY_UTIL__
#define __DIAG_GEPHY_UTIL__

extern int diag_gephy_util(void);

#define MRV88E1512_REG_PAGE_0			0
#define MRV88E1512_REG_PAGE_1			1
#define MRV88E1512_REG_PAGE_2			2
#define MRV88E1512_REG_PAGE_3			3
#define MRV88E1512_REG_PAGE_4			4
#define MRV88E1512_REG_PAGE_5			5
#define MRV88E1512_REG_PAGE_6			6
#define MRV88E1512_REG_PAGE_18		    18

/* Marvell phy address */
#define MRV88E1512_PHY_ID_1_VALUE           0x0141
#define MRV88E1512_PHY_ID_2_VALUE           0x0dd1
#define MRV88E1512_PHY_ID                   0x0
#define MRV88E1512_PHY_ID_NCSI              0x1

#define MRV88E1512_CONTROL_PHY_RESET_VALUE	0x8000
#define MRV88E1512_CONTROL_PHY_RESET		0x8000
#define MRV88E1512_CONTROL_LOOPBACK		    0x4000
#define MRV88E1512_CONTROL_FORCE_10_L		0x0000
#define MRV88E1512_CONTROL_FORCE_100_L      0x2000
#define MRV88E1512_CONTROL_FORCE_1000_L		0x0040
#define MRV88E1512_CONTROL_AUTONEG_ENABLE	0x1000
#define MRV88E1512_CONTROL_POWER_DOWN		0x0800
#define MRV88E1512_CONTROL_ISOLATE		    0x0400
#define MRV88E1512_CONTROL_RESTART_AUTONEG	0x0200
#define MRV88E1512_CONTROL_FULL_DUPLEX		0x0100
#define MRV88E1512_CONTROL_COLLISION_TEST	0x0080
#define MRV88E1512_CONTROL_FORCE_1000_M		0x0040
#define MRV88E1512_CONTROL_FORCE_10_M		0x0000
#define MRV88E1512_CONTROL_SPEED_MASK		0x2040

/* Page 0 Register Offsets - Copper */
#define MRV88E1512C_CONTROL_REG             0
#define MRV88E1512C_STATUS_REG              1
#define MRV88E1512C_AUTONEG_ADVR_REG        4
#define MRV88E1512C_LINK_PART_AV_REG        5
#define MRV88E1512C_AUTONEG_EXPANSION_REG   6
#define MRV88E1512C_NEXT_PAGE_REG           7
#define MRV88E1512C_LP_NEXT_PAGE_REG        8
#define MRV88E1512C_1000B_CNTL_REG          9
#define MRV88E1512C_1000B_STATUS_REG        10
#define MRV88E1512C_XMDIO_MMD_CONTROL_REG   13
#define MRV88E1512C_XMDIO_MMD_ADDR_DATA_REG 14
#define MRV88E1512C_EXTENDED_STATUS_REG     15
#define MRV88E1512C_SPECIFIC_CONTROL1_REG   16
#define MRV88E1512C_SPECIFIC_STATUS1_REG    17
#define MRV88E1512C_SPECIFIC_INT_ENABLE_REG 18
#define MRV88E1512C_INT_STATUS_REG          19
#define MRV88E1512C_SPECIFIC_CONTROL2_REG   20
#define MRV88E1512C_REC_ERROR_COUNTER_REG   21
#define MRV88E1512C_GLOBAL_INT_STATUS_REG   23
#define MRV88E1512C_SPECIFIC_CONTROL3_REG   26

/* Page 2 Register Offsets - MAC */
#define MRV88E1512M_SPECIFIC_CONTROL1_REG           16
#define MRV88E1512M_SPECIFIC_INT_ENABLE_REG         18
#define MRV88E1512M_SPECIFIC_STATUS_REG             19
#define MRV88E1512M_COPPER_RXER_BYTE_CAPTURE_REG    20
#define MRV88E1512M_SPECIFIC_CONTROL2_REG           21
#define MRV88E1512M_RGMII_OUT_IMP_CALI_OV_REG       24
#define MRV88E1512M_RGMII_OUT_IMP_TARGET_REG        25
#define MRV88E1512_PAGE_ADDRESS_REG		            22
#define MRV88E1512_P2_R18_OVER_EN                   0x0080
#define MRV88E1512_P2_R18_IDLE_INS_EN               0x0008
#define MRV88E1512_P2_R18_IDLE_DEL_EN               0x0004

/* Page 6 Register Offsets */
#define MRV88E1512_CHECK_CONTROL_REG		   18
#define MRV88E1512_ENABLE_STUB_TEST		       0x0008
#define MRVL88E1512_TEST_MODE_1                (1)
#define MRVL88E1512_TEST_MODE_4                (4)
#define MRVL88E1512_TEST_MODE_WAVEFORM_TEST    (0x2000)
#define MRVL88E1512_TEST_MODE_DISTORTION_TEST  (0x8000)

/* Page 18 Register Offsets */
#define MRVL88E1512_GENERAL_CONTROL_REG        (20)
#define MRV88E1512_P18_R20_MODE                (0x0007)
#define MRV88E1512_P18_R20_RST                 (0x8000)
#define MRVL88E1512_GEN_CTRL_MODE_RGMII_COPPER (0x0)

/* 1000BASE-T Control Register (offset 9) Bits defines */
#define PHY_GT_CTL_TEST_MASK    0xE000  /* Test Modes */
#define PHY_GT_CTL_NORMAL   0x0000  /* Normal mode */
#define PHY_GT_CTL_WV_TEST  0x2000  /* Test mode 1 - Waveform test */
#define PHY_GT_CTL_JT_MS    0x4000  /* Test mode 2 - Jitter (Master) */
#define PHY_GT_CTL_JT_SL    0x6000  /* Test mode 3 - Jitter (Slave) */
#define PHY_GT_CTL_DS_TEST  0x8000  /* Test mode 4 - Distortion test */
#define PHY_GT_CTL_TEST_MAX PHY_GT_CTL_DS_TEST
#define PHY_GT_CTL_TEST_SHIFT   13  /* Test mode bits shift counts */

extern boolean mgmt_gephy;
extern int diag_gephy_reg_value_get(int, int);

#define PHY_ID (mgmt_gephy ? MRV88E1512_PHY_ID : MRV88E1512_PHY_ID_NCSI)    
#endif /* __DIAG_GEPHY_UTIL__ */

/*---------------------------------------------------------------
$Log: diag_gephy_util.h,v $
Revision 1.3  2017/03/30 08:30:53  hondwang
Tachi-L brach merge

Revision 1.2.14.1  2016/11/04 19:08:55  benchen2
Modify Enhanced error message

Revision 1.2  2016/04/20 11:25:32  benchen2
add tachi fru portion

Revision 1.1.2.5  2015/12/22 17:26:03  huanngo
Support Tachi-High tests and menus CSCux15587

Revision 1.1.2.4  2015/12/04 11:59:17  benchen2
fix gephy textmode

Revision 1.1.2.3  2015/09/14 07:30:43  benchen2
phy 1512 utility

Revision 1.1.2.2  2015/07/31 07:32:26  hondwang
gephy util

Revision 1.1.2.1  2015/06/11 02:01:08  tirawan
Add files for Tachi BMC project


$Endlog$
*/
