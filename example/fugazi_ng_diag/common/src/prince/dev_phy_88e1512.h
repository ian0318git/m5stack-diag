/* $Id: dev_phy_88e1512.h,v 1.1 2013/04/19 07:17:49 xiaoyizh Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/prince/dev_phy_88e1512.h,v $
 * -----------------------------------------------------------------------------
 * File Name: dev_phy_88e1512.h
 *
 * Description: Contains common definitions for the Marvell 88E1512 PHY.
 *      
 * Xiaoying Zhang - Dec 2012
 *
 * Copyright (c) 2012-2013 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------------------
 */

#ifndef __DEV_PHY_88E1512_H__
#define __DEV_PHY_88E1512_H__

/************************* 88E1512 ************************/
#define MRV88E1512_REG_PAGE_0                   0
#define MRV88E1512_REG_PAGE_1                   1
#define MRV88E1512_REG_PAGE_2                   2
#define MRV88E1512_REG_PAGE_3                   3
#define MRV88E1512_REG_PAGE_4                   4
#define MRV88E1512_REG_PAGE_5                   5
#define MRV88E1512_REG_PAGE_6                   6
#define MRV88E1512_REG_PAGE_7                   7
#define MRV88E1512_REG_PAGE_8                   8
#define MRV88E1512_REG_PAGE_9                   9
#define MRV88E1512_REG_PAGE_12                  12
#define MRV88E1512_REG_PAGE_14                  14
#define MRV88E1512_REG_PAGE_17                  17
#define MRV88E1512_REG_PAGE_18                  18
#define MRV88E1512_REG_PAGE_29                  29
#define MRV88E1512_REG_PAGE_MAX                 256

#if 0
/* Page 0 Register Offsets - Copper */
#define MRV88E1512_CONTROL_REG                  0
#define MRV88E1512_STATUS_REG                   1
#define MRV88E1512_PHY_ID1                      2
#define MRV88E1512_PHY_ID2                      3
#define MRV88E1512_AUTONEG_ADVR_REG             4
#define MRV88E1512_LINK_PART_AV_REG             5
#define MRV88E1512_AUTONEG_EXPANSION_REG        6
#define MRV88E1512_NEXT_PAGE_REG                7
#define MRV88E1512_LP_NEXT_PAGE_REG             8
#define MRV88E1512_1000B_CNTL_REG               9
#define MRV88E1512_1000B_STATUS_REG             10
#define MRV88E1512_EXTENDED_STATUS_REG          15
#define MRV88E1512_SPECIFIC_CONTROL1_REG        16
#define MRV88E1512_SPECIFIC_STATUS1_REG         17
#define MRV88E1512_INT_ENABLE_REG               18
#define MRV88E1512_INT_STATUS_REG               19
#define MRV88E1512_SPECIFIC_CONTROL2_REG        20
#define MRV88E1512_REC_ERROR_COUNTER_REG        21
#define MRV88E1512_PAGE_ADDRESS_REG             22
#define MRV88E1512_GLOBAL_INTERRUPT_REG         23
#define MRV88E1512_SPECIFIC_CONTROL3_REG        26
#endif

/* Page 1 Register Offsets - Fiber */
#define MRV88E1512_CONTROL_REG                  0
#define MRV88E1512_STATUS_REG                   1
#define MRV88E1512_PHY_ID1                      2
#define MRV88E1512_PHY_ID2                      3
#define MRV88E1512_AUTONEG_ADVR_REG             4
#define MRV88E1512_LINK_PART_AV_REG             5
#define MRV88E1512_AUTONEG_EXPANSION_REG        6
#define MRV88E1512_NEXT_PAGE_REG                7
#define MRV88E1512_LP_NEXT_PAGE_REG             8
#define MRV88E1512_1000B_CNTL_REG               9
#define MRV88E1512_1000B_STATUS_REG             10
#define MRV88E1512_EXTENDED_STATUS_REG          15
#define MRV88E1512_SPECIFIC_CONTROL1_REG        16
#define MRV88E1512_SPECIFIC_STATUS1_REG         17
#define MRV88E1512_INT_ENABLE_REG               18
#define MRV88E1512_INT_STATUS_REG               19
#define MRV88E1512_REC_ERROR_COUNTER_REG        21
#define MRV88E1512_PAGE_ADDRESS_REG             22
#define MRV88E1512_GLOBAL_INTERRUPT_REG         23
#define MRV88E1512_PRBS_ERR_CNT_LSB             24
#define MRV88E1512_PRBS_ERR_CNT_MSB             25
#define MRV88E1512_SPECIFIC_CONTROL2_REG        26

/* Fiber Control Register (Page 1, Reg 0) */
#define MRV88E1512_FIBER_RST                    0x8000
#define MRV88E1512_LPBK_ENA                     0x4000
#define MRV88E1512_SPD_SEL_MASK                 0x2040
#define MRV88E1512_SPD_SEL_1000M                0x0040
#define MRV88E1512_SPD_SEL_100M                 0x2000
#define MRV88E1512_SPD_SEL_10M                  0x0000
#define MRV88E1512_AUTO_NEO_ENA                 0x1000
#define MRV88E1512_PWR_DOWN                     0x0800
#define MRV88E1512_RST_AUTO_NEO                 0x0200
#define MRV88E1512_FULL_DUPLEX                  0x0100

#define MRV88E1512_SPEED_MASK                   0x0007
#define MRV88E1512_SGMII_SPD_1000               0x0006
#define MRV88E1512_SGMII_SPD_100                0x0005
#define MRV88E1512_SGMII_SPD_10                 0x0004

#define MRV88E1512_PHY_OUI_HI                   0x0141  
#define MRV88E1512_PHY_OUI_LO                   0x03
#define MRV88E1512_PHY_OUI_LO_MASK              0xFC00

/* Copper Auto-Nego Advertisement Register (Page 0, Reg 4) */
#define MRV88E1512_10BT_ADV                     0x60    
#define MRV88E1512_100BT_ADV                    0x180
#define MRV88E1512_1000BT_ADV                   0x300

/* 1000BASE-T Control Register (offset 9) Bits defines */
#define PHY_GT_CTL_TEST_MASK    0xE000  /* Test Modes */
#define PHY_GT_CTL_NORMAL       0x0000  /* Normal mode */
#define PHY_GT_CTL_WV_TEST      0x2000  /* Test mode 1 - Waveform test */
#define PHY_GT_CTL_JT_MS        0x4000  /* Test mode 2 - Jitter (Master) */
#define PHY_GT_CTL_JT_SL        0x6000  /* Test mode 3 - Jitter (Slave) */
#define PHY_GT_CTL_DS_TEST      0x8000  /* Test mode 4 - Distortion test */
#define PHY_GT_CTL_TEST_MAX     PHY_GT_CTL_DS_TEST
#define PHY_GT_CTL_TEST_SHIFT   13      /* Test mode bits shift counts */

/* Copper Specific Control Register (Page 0, Reg 16) */
#define MRV88E1512_CNTL_FORCE_LINK              0x0400
#define MRV88E1512_P0_R16_PWR_DOWN              0x0004

/* Copper Specific Control Register 1 (Page 0, Reg 17) */
#define MRV88E1512_P0_R17_DTE_NEED_POWER        0x0004
#define MRV88E1512_LINK_SPEED_MASK              0xC000
#define MRV88E1512_LINK_SPEED_1000              0x8000
#define MRV88E1512_LINK_SPEED_100               0x4000
#define MRV88E1512_LINK_SPEED_10                0x0000
#define MRV88E1512_LINK_UP                      0x0400
#define MRV88E1512_SYNC                         0x0020

/* Cooper Specific Control Register 3 (page 0, Reg 26) */
#define MRV88E1512_P0_R26_DTE_DETECT            0x0100
#define MRV88E1512_P0_R26_DTE_STATUS_DROP_5S    0x0010
#define MRV88E1512_P0_R26_DTE_STATUS_DROP_MSK   0x00F0
#define MRV88E1512_P0_R26_CLASS_A               0x9000

/* Page 1 Register Offsets - Fiber */
#define MRV88E1512_P1_CONTROL_REG               0

/* Fiber Specific Control Register 2 (Page 1, Reg 26) */
#define MRV88E1512_FIBER_FORCE_INT              0x8000

/* Fiber Control Register (Page 1, Reg 0) */
#define MRV88E1512_P1_R0_PWR_DOWN               0x0800
#define MRV88E1512_P1_R17_SYNC                  0x0020

/* Page 2 Register Offsets - MAC */
#define MRV88E1512_MAC_SPD_MASK                 0x7
#define MRV88E1512_MAC_SPD_1000M                0x6
#define MRV88E1512_MAC_SPD_100M                 0x5
#define MRV88E1512_MAC_SPD_10M                  0x4

/* Page 2 MAC Specific Control Reg */
#define MRV88E1512_MAC_CNTL_REG2                21
#define MRV88E1512_P2_R21_RX_CTRL               0x0020
#define MRV88E1512_P2_R21_TX_CTRL               0x0010

/* Page 2 MAC Specific Interrupt Enable */
#define MRV88E1512_MAC_SPEC_INT_EN              18
#define MRV88E1512_P2_R18_OVER_EN               0x0080
#define MRV88E1512_P2_R18_IDLE_INS_EN           0x0008
#define MRV88E1512_P2_R18_IDLE_DEL_EN           0x0004

/* Page 3 LED Timer Contrl */
#define MRV88E1512_LED_TIMER_CTRL               18
#define MRV88E1512_P3_R18_INT_POLARITY          0x0800
#define MRV88E1512_P3_R18_INT_EN                0x0080

/* Page 18 General Control */
#define MRV88E1512_GEN_CTRL1                    20
#define MRV88E1512_P18_R20_RST                  0x8000
#define MRV88E1512_P18_R20_MODE                 0x0007

#define MRV88E1512_ERR_MSG_LEN                  500

enum {
    WARNING = 0x00010000,
    RETRY   = 0x00020000,
    FATAL   = 0xFFFF0000,
};

/*
 * dev_error_report message codes
 */
typedef enum {
    MRVL_88E1512_DEV_STATE = 0,
    MRVL_88E1512_ATTACH,
    MRVL_88E1512_DETACH,
    MRVL_88E1512_INIT,
    MRVL_88E1512_SHOW,
    MRVL_88E1512_DESTROY,
    MRVL_88E1512_REG_TEST,
    MRVL_88E1512_ALTER_REG,
    MRVL_88E1512_SET_LPBK,
    MRVL_88E1512_CLN_LPBK,
    MRVL_88E1512_POWER_UP,
    MRVL_88E1512_INTR_GEN,   
    MRVL_88E1512_INTR_CLR,
    MRVL_88E1512_LOCKUP_FIX,
    MRVL_88E1512_PHONE_DETECT,   
    MRVL_88E1512_SET_TEST_MODE,
}mrvl_88e1512_report_code_t;

enum
{
    ETH_MODE_GE,
    ETH_MODE_FE100,
    ETH_MODE_FE10,
};

/* define loopback mode */
enum
{
    SGMII_LPBK_NONE,           /* no loopback */
    SGMII_PHY_LPBK_INTERNAL,   /* internal loopback at marvell GE PHY */
};

extern int dev_phy_read_reg(ushort, ushort, ushort *);
extern int dev_phy_write_reg(ushort, ushort, ushort *);

#endif //__DEV_PHY_88E1512_H__

/******** History ********
$Log: dev_phy_88e1512.h,v $
Revision 1.1  2013/04/19 07:17:49  xiaoyizh


Initial check in for Prince NIM.


$Endlog$
*/
