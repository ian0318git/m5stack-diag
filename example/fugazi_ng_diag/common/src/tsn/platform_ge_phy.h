/* $Id: platform_ge_phy.h,v 1.4 2018/04/15 22:03:31 palin2 Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/tsn/platform_ge_phy.h,v $
 *-----------------------------------------------------------------------------
 *
 * Filename   : platform_ge_phy.h
 * Description: Head file of TSN GE PHY(Marvell 88E1112) platform.
 *
 * Copyright (c) 2017~2018 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *-----------------------------------------------------------------------------
 */

#ifndef __PLATFORM_GE_PHY_H__
#define __PLATFORM_GE_PHY_H__

#include "smi_api.h"
#include "dev_object.h"

/* Common */
typedef enum {
    GEPHY_COP = 0,
    GEPHY_FIB,
} gephy_intf;

typedef enum {
    GEPHY_LINKDOWN = 0,
    GEPHY_LINKUP,
} gephy_linkopt;

typedef enum {
    GEPHY_DIS = 0,
    GEPHY_EN,
} gephy_onoff;

#define REG_ADDR(x) (x)
#define REG_PAGE(x) (x)
#define REG_BIT(x)  (1 << (x))

#define GEPHY_REG_RESET         (1 << 15)

#define MRVL_1112C_PHY_PAGE2  2
#define MRVL_1112C_PHY_PAGE22  22
#define SET_1112C_REMOTE_LPBK_VAL 0x5040
#define CLEAR_1112C_REMOTE_LPBK_VAL 0x1040
#define SET_1112C_LPBK_BIT 1
#define CLEAR_1112C_LPBK_BIT 0 

#define MAC_CTRL_REG0  0

#define GE_PHY_SUBMENU      0x80000000    /* Submenu */

#define GEPHY_MAX_RETRY   100

#define GEPHY_REG_TEST         "Register"
#define GEPHY_MAC_LPBK_TEST    "MAC loopback"
#define LED_MAGIC_NUMBER        0x7181

/* Copper Cotrol Reg.(0_0) */
#define MRVL1112_COP_CTRL_REG   0x0    /* 0_0 */
#define MRVL1112_CCR_REGPAGE    (int)0
#define MRVL1112_CCR_REGADDR    (int)0
#define COP_CTRL_RESET          (uint16_t)(1 << 15)
#define COP_CTRL_LPBK           (uint16_t)(1 << 14)
#define COP_CTRL_SPD_LSB        (uint16_t)(1 << 13)
#define COP_CTRL_AUTONEG        (uint16_t)(1 << 12)
#define COP_CTRL_DUPLEX_FULL    (uint16_t)(1 << 8)
#define COP_CTRL_SPD_MSB        (uint16_t)(1 << 6)

#define COP_SPD_MASK            (uint16_t)0x2040
#define COP_SPD_10Mbps          (uint16_t)0x0000
#define COP_SPD_100Mbps         (uint16_t)0x2000
#define COP_SPD_1000Mbps        (uint16_t)0x0040

/* Copper Specific Cotrol Reg.1(16_0) */
#define MRVL1112_CSCR1_REGPAGE  (int)0
#define MRVL1112_CSCR1_REGADDR  (int)16
#define CSCR1_FORCE_COP_LINK    (uint16_t)(1 << 10)

/* Copper Specific Status Reg.1(17_0) */
#define MRVL1112_CSSR1_REGPAGE  (int)0
#define MRVL1112_CSSR1_REGADDR  (int)17
#define CSSR1_COP_LINKUP        (uint16_t)(1 << 10)

/* Fiber Control Reg.(0_1) */
#define MRVL1112_FCR_REGPAGE    (int)1
#define MRVL1112_FCR_REGADDR    (int)0
#define FCR_RESET               (uint16_t)(1 << 15)
#define FCR_LPBK                (uint16_t)(1 << 14)
#define FCR_AUTONEG             (uint16_t)(1 << 12)
#define FCR_DUPLEX_FULL         (uint16_t)(1 << 8)

#define FCR_SPD_MASK            (uint16_t)0x2040
#define FCR_SPD_10Mbps          (uint16_t)0x0000
#define FCR_SPD_100Mbps         (uint16_t)0x2000
#define FCR_SPD_1000Mbps        (uint16_t)0x0040

/* Fiber Specific Cotrol Reg.1(16_1) */
#define MRVL1112_FSCR1_REGPAGE  (int)1
#define MRVL1112_FSCR1_REGADDR  (int)16
#define FSCR1_FORCE_FIB_LINK    (uint16_t)(1 << 10)

/* Copper Specific Status Reg.1(17_0) */
/* Fiber Specific Status Reg.1(17_1) */
#define MRVL1112_FSSR1_REGPAGE  (int)1
#define MRVL1112_FSSR1_REGADDR  (int)17
#define FSSR1_1000BASEX_LINKUP  (uint16_t)(1 << 10)

/* MAC Control Reg.(0_2) */
#define MRVL1112_MAC_CTRL_REG   0x0   /* 0_2 */
#define MRVL1112_MCR_REGPAGE    (int)2
#define MRVL1112_MCR_REGADDR    (int)0
#define MCR_PHY_RESET           (1 << 15)
#define MCR_SPD_LSB             (1 << 13)
#define MCR_MAC_AN_EN           (1 << 12)
#define MCR_PWR_DOWN            (1 << 11)
#define MCR_SPD_MSB             (1 << 6)

#define MCR_SPD_MASK            0x2040
#define MCR_SPD_10Mbps          0x0000
#define MCR_SPD_100Mbps         0x2000
#define MCR_SPD_1000Mbps        0x0040

/* MAC Specific Control Reg.1(16_2) */
#define MRVL1112_MSCR1_REGPAGE         2   /* 16_2 */
#define MRVL1112_MSCR1_REGADDR         16  /* 16_2 */
#define MSCR1_MOD_OFFSET               7   /* [9:7]Mode select */
#define MSCR1_MOD_MSK                  0x7 /* [9:7]Mode select */
#define MSCR1_MOD_100BASE_FX           0   /* 000 = 100BASE-FX */
#define MSCR1_MOD_COP_GBIC             1   /* 001 = Copper GBIC */
#define MSCR1_MOD_AUTO_COP_SGMII       2   /* 010 = Auto Copper/SGMII */
#define MSCR1_MOD_AUTO_COP_1000BASEX   3   /* 011 = Auto Copper/1000Base-X */
#define MSCR1_MOD_RESV                 4   /* 100 = Reserved */
#define MSCR1_MOD_COP_ONLY             5   /* 101 = Copper only */
#define MSCR1_MOD_SGMII_ONLY           6   /* 110 = SGMII only */
#define MSCR1_MOD_1000BASEX_ONLY       7   /* 111 = 1000Base-X only */

/* 1000Base-T Control Reg.(9_0) */
#define GEPHY_1000T_CNTL_REG   0x9
#define ONEK_CNTL_TESTMODE_SHIFT     13
#define ONEK_CNTL_TESTMODE_MASK      (7 << 13)   /* 0xE000 */

#define OCR_TESTMODE_NORMAL          0

#define MRVL88E1112_PAGE_ADDR_REG    22

/* Packet Generation Reg.(16_6) */
#define PG_P6R16_STUB_EN       (1 << 5)

/* Copper Specific Control Reg.(26_0) */
#define MRVL88E1112_CSC_REG2   26
#define CSCR2_TXTYPE_MASK      (1 << 15)
#define CSCR2_TXTYPE_OFFSET    15
#define CSCR2_TXTYPE_A         (1 << 15)

/* Misc Test Reg.(26_6) */
#define GEPHY_MISC_TEST_REG    0x26
#define MTR_TX_TCLK_EN         (1 << 15)

/* VOD Adjustments */
/* VOD Adjust Reg.(29_X, X: don't care) */
#define GEPHY_MFG_R29_REG      29
#define GEPHY_MFG_R29_VOD      0x0004
#define GEPHY_VOD_ADJ_REG      30
#define GEPHY_VOD_ADJ_MASK     0x7   /* bit[2:0] of Reg.30 */

/* VOD Adjust Value */
enum gephy_vod_adj {
    VOD_ADJ_PLUS8 = 0,
    VOD_ADJ_PLUS6,
    VOD_ADJ_PLUS4,
    VOD_ADJ_PLUS2,
    VOD_ADJ_DEFAULT,
    VOD_ADJ_MINUS2,
    VOD_ADJ_MINUS4,
    VOD_ADJ_MINUS6,
};

/* Externs */
extern uint32_t     tsn_ge_phy_smi_rd(smi_if_t *);
extern uint32_t     tsn_ge_phy_smi_wr(smi_if_t *);
extern uint32_t     tsn_ge_phy_smi_open(smi_if_t *);
extern uint32_t     tsn_ge_phy_smi_close(smi_if_t *);
extern void         tsn_ge_phy_sfp_operation(int, int);
extern dev_object_t *diag_get_88e11112c_obj(int);
extern int          tsn_ge_phy_reset(boolean);
extern int          setting_1112_lpbk_bit(int);
extern int          tsn_ge_phy_mii_rd(int, int, int, int *);
extern int          tsn_ge_phy_mii_wr(int, int, int, int);

extern int tsn_gephy_reg_rd(int, int, int, ushort *);
extern int tsn_gephy_reg_wr(int, int, int, ushort);
extern int tsn_set_gephy_reg(int, int, int, ushort);
extern int tsn_config_gephy_fiber(void);
extern int tsn_gephy_reg_wr_util(int);
extern int tsn_gephy_reg_rd_util(int);
extern int tsn_gephy_config_w_rst(int, int, int, ushort);
extern int gephy_vod_adj_util(int);
extern int tsn_gephy_get_media_mode(int, uint *);
extern int tsn_gephy_set_media_mode(int, uint);
extern int tsn_gephy_check_linkstat(int, boolean, boolean);
extern int tsn_gephy_force_linkgood(int, boolean, boolean);
extern int tsn_gephy_set_macspeed(int, ushort);
extern int tsn_gephy_set_macloopback(int, boolean, boolean);
extern int tsn_gephy_config_media(int, boolean, uint16_t, boolean);

#endif   /* __PLATFORM_GE_PHY_H__ */


/*-------------------------------------------------
 * $Log: platform_ge_phy.h,v $
 * Revision 1.4  2018/04/15 22:03:31  palin2
 * Merged Vulcan back to maintrunk.
 *
 * Revision 1.3  2018/04/13 08:52:58  palin2
 * To fix CSCvi96469: Potential issue on GEWAN0(Copper + SFP) loopback test.
 *
 * Revision 1.2.36.2  2018/04/09 21:35:56  palin2
 * Added soft-reset after set Marvell 88e1112  media mode based on datasheet.
 *
 * Revision 1.2.36.1  2018/04/09 20:57:00  palin2
 * Enhanced GEWAN PHY Diag tests by config testing media accordingly.
 *
 * Revision 1.2  2017/08/02 14:21:48  steja
 * Support TSN-H/M platform code
 *
 * Revision 1.1.8.3  2017/07/31 16:33:17  palin2
 * Added utiltiy to support GE WAN PHY VOD adjustments.
 *
 * Revision 1.1.8.2  2017/07/29 03:41:20  steja
 * tsn-branch5 synced with Maintrunk repositories
 *
 * Revision 1.1.6.2  2017/07/20 13:38:07  steja
 * tsn-branch4 merge with maintrunk
 *
 * Revision 1.1.4.5.2.1  2017/07/05 14:05:24  steja
 * Enhance code readability
 *
 * Revision 1.1.4.5  2016/09/13 08:14:23  palin2
 * Added CPU to GE PHY MAC loopback test.
 *
 * Revision 1.1.4.4  2016/07/17 10:52:56  palin2
 * 1. Added function and utility to set GE WAN PHY Transmitter Type.
 * 2. Clean up code.
 *
 * Revision 1.1.4.3  2016/06/30 14:06:32  steja
 * Pick up the latest from tsn-branch1
 *
 * Revision 1.1.4.2  2016/06/30 06:22:50  steja
 * tsn-branch2 sync with main trunk
 *
 * Revision 1.1.2.4  2016/06/29 12:08:37  palin2
 * Added utility to set GE WAN PHY 1000Base-T Test mode.
 *
 * Revision 1.1.2.3  2016/04/26 20:48:49  palin2
 * Updated code after bring up SFP external loopback test.
 *
 * Revision 1.1.2.2  2016/04/22 12:28:36  palin2
 * Updated code after bring up GE PHY external loopback test.
 *
 * Revision 1.1.2.1  2016/03/29 02:50:03  palin2
 * Added GE PHY Diag.
 *
 * $Endlog$
 *-------------------------------------------------
 */

