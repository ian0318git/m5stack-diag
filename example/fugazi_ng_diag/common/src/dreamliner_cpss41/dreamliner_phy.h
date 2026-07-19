/* $Id: dreamliner_phy.h,v 1.2 2019/12/11 10:10:26 lucywang Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/dreamliner_cpss41/dreamliner_phy.h,v $
 *------------------------------------------------------------------
 *
 * dreamliner_phy.h - This file contains all the definitions 
 *                    for Marvell PHY.
 *
 * Christine Wen -- Feb. 2014
 *
 * Copyright (c) 2014-2019 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#define GE0_INTER_LPBK  0
#define GE1_INTER_LPBK  1
#define GE0_EXTER_LPBK  2

#define PHY_CONTROL_REG                  0
#define PHY_SPECIFIC_CONTROL1_REG        16
#define PHY_SPECIFIC_STATUS1_REG         17
#define PHY_SPECIFIC_CONTROL2_REG        20
#define PHY_GLOBAL_INTR_STATUS_REG       23
#define PHY_SPECIFIC_CONTROL3_REG        26

#define PHY_QSGMII_INTR_ENA_REG          18
#define PHY_QSGMII_INTR_STATUS_REG       19

/* Copper Control Register (Page 0, Reg 0) */
#define PHY_COOPER_RST                   0x8000
#define PHY_LPBK_ENA                     0x4000
#define PHY_SPD_SEL_MASK                 0x2040
#define PHY_SPD_SEL_1000M                0x0040
#define PHY_SPD_SEL_100M                 0x2000
#define PHY_SPD_SEL_10M                  0x0000
#define PHY_AUTO_NEO_ENA                 0x1000
#define PHY_PWR_DOWN                     0x0800
#define PHY_RST_AUTO_NEO                 0x0200
#define PHY_COPPER_FULL_DUPLEX           0x0100

#define PHY_SPEED_MASK                   0x0007
#define PHY_SGMII_SPD_1000               0x0006
#define PHY_SGMII_SPD_100                0x0005
#define PHY_SGMII_SPD_10                 0x0004

/* Copper Auto-Nego Advertisement Register (Page 0, Reg 4) */
#define PHY_10BT_ADV                     0x60    
#define PHY_100BT_ADV                    0x180

/* 1000BASE-T control register (page 0, reg 9) */
#define PHY_1000BT_ADV                   0x300

/* Copper specific status register 1 (page 0, reg 17) */
#define PHY_COPPER_LINK                  0x0400

/* Cooper Specific Control Register 3 (page 0, Reg 26) */
#define PHY_P0_R26_DTE_DETECT            0x0100
#define PHY_P0_R26_DTE_STATUS_DROP_5S    0x0010
#define PHY_P0_R26_DTE_STATUS_DROP_MSK   0x00F0
#define PHY_P0_R26_CLASS_A               0x9000
#define PHY_P0_R17_DTE_NEED_POWER        0x0004

/* MAC specific control register 2 (page 2, reg 21) */
#define PHY_MAC_SPD_MASK                 0x7
#define PHY_MAC_SPD_1000M                0x6
#define PHY_MAC_SPD_100M                 0x5
#define PHY_MAC_SPD_10M                  0x4

/* QSGMII specific status register (page 4, reg 17) */
#define PHY_SYNC                         0x0020
#define PHY_LINK_SPEED_1000              0x8000
#define PHY_LINK_SPEED_MASK              0xc000
#define PHY_FULL_DUPLEX                  0x2000
 
/* QSGMII Interrupt Enable/Status Register (page 4, reg 18/19) */
#define PHY_QSGMII_SPEED_CHANGED         0x4000
#define PHY_QSGMII_FORCE_PIN             0x8000
#define PHY_QSGMII_LINK_STATUS_CHANGED   0x400

#define MRVL_PHONE_DETECT_TIME           5

/* check control (page 6, reg 18) */
#define PHY_ENA_STUB_TEST                0x8

#define PHY_INTR_DELAY                   500

#define GT_OK                    (0x00) /* Operation succeeded */
#define GT_FAIL                  (0x01) /* Operation failed    */
/*
 *------------------------------------------------------------------
 * $Log: dreamliner_phy.h,v $
 * Revision 1.2  2019/12/11 10:10:26  lucywang
 * Merged Nanook to main trunk
 *
 * 
 *------------------------------------------------------------------
 * $Endlog$
 */
