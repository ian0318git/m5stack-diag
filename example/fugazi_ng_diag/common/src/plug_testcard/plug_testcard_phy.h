 /* $Id: plug_testcard_phy.h,v 1.3 2018/11/23 09:10:40 hondwang Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/plug_testcard/plug_testcard_phy.h,v $
 *-----------------------------------------------------------------------------
 *
 * Filename   : plug_testcard_phy.h
 * Description: Head file of PLUG testcard GE PHY(Marvell 88E1112) Diag.
 *
 * Copyright (c) 2018 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *-----------------------------------------------------------------------------
 */

#ifndef __DIAG_PLUG_TC_GE_PHY_H__
#define __DIAG_PLUG_TC_GE_PHY_H__ 

/* setup delay time for driver to read the PHY reg. */
#define ETH_DRIVER_DELAY    1

/* Common */
#define SPD_10MBPS    10
#define SPD_100MBPS   100
#define SPD_1000MBPS   1000

#define I2CPHY_REG_WRITE_DELAY   1000


/* definition of PHY signal */
#define SIG_COPPER 0
#define SIG_FIBER 1

#define DISABLE_SIG   0
#define ENABLE_SIG    1

#define INT_LPBK 0
#define EXT_LPBK 1

#define AUTONEG_OFF  0
#define AUTONEG_ON   1

#define HALF_DUPLEX  0
#define FULL_DUPLEX  1

enum loopback_num {
    SGMII_EXT_LPBK = 0,
    SGMII_INT_EXT_LPBK,
    E_1000BASEX_INT_EXT_LPBK,
};

/* define for PHY setting */
#define SET_PHY_BIT15     0x8000
#define SET_PHY_BIT14     0x4000
#define SET_PHY_BIT13     0x2000
#define SET_PHY_BIT11     0x0800
#define SET_PHY_BIT10     0x0400
#define SET_PHY_BIT3       0x0008
#define SET_AUTO_MEDIA     0x0007
#define SET_ENG_DETECT     0x0300

#define TX_RX_SYNC_TIME       10

#define COP_CTRL_REG0         0
#define COP_STATUS_REG1       1
#define COP_AUTONEG_ADV_REG4  4
#define COP_SPEC_CTRL_REG16   16
#define COP_STATUS_REG17      17
#define MAC_SPEC_CTRL2_REG21  21
#define COP_SPEC_CTRL_REG2    26

#define FIBER_SPECIFIC_STATUS_REG 17
#define FIBER_STATUS_REG1     1
#define GEN_CONT_REG_1        20

#define MAC_CTRL_REG0  0
#define MAC_SPEC_CTRL_REG2      26

#define FIB_CTRL_REG0  0
#define FIB_STATUS_REG1       1
#define FIB_SPEC_CTRL_REG2      26
#define FIB_OUTPUT_AMP_MSK      0x7
#define FIB_OUTPUT_AMP_VAL504   0x5

#define CHECKER_CTRL_REG18  18
#define GENERAL_CTRL_REG20  20  /*page 6*/
#define GENERAL_CTRL1_REG20  20 /*page 18*/
#define GENERAL_CTRL2_REG27  27 /*page 18*/

/* Marvell phy register number and bit mask
 */
#define PHY_REG(x) (x)
#define PHY_PAGE(x) (x)
#define PHY_REG_BIT(x) (1 << (x))

#define PLUG_TC_GEPHY_CONFIG_TIME   1000


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


extern int plug_tc_i2cphy_reg_rd(int, ushort *);
extern int plug_tc_i2cphy_reg_wr(int, ushort);
extern int plug_testcard_sgmii_loopback_test(int);
extern int plug_tc_host_check_ext_lpbk_flag(void);
extern int plug_tc_gephy_reg_wr(int, int, int, ushort);
extern int plug_tc_gephy_reg_rd(int, int, int, ushort*);
extern int getdec_answer(char *, uint, uint, uint);
extern int plug_tc_gephy_config_w_rst(int, int, int , ushort);


#endif   /* __DIAG_PLUG_TC_GE_PHY_H__ */


/*-------------------------------------------------
 * $Log: plug_testcard_phy.h,v $
 * Revision 1.3  2018/11/23 09:10:40  hondwang
 * Re-instruct pluggable common code with CDETs CSCvn17216
 *
 * Revision 1.2.62.1  2018/10/15 06:50:50  hondwang
 * pluggable common code re-instruct modify code
 *
 * Revision 1.2  2018/01/20 05:01:10  hondwang
 * prepare merge star-branch-c9xx to main trunk
 *
 * Revision 1.1.4.2  2017/08/08 07:44:28  hondwang
 * add pluggable testcard for star-branch-c9xx
 *
 * Revision 1.1.2.1  2017/07/13 06:32:21  tirawan
 * Reorganize Star Pluggable directory structure
 *
 * Revision 1.1.2.2  2017/06/17 14:15:10  hondwang
 * fix login submenu issue
 *
 * Revision 1.1.2.1  2017/06/17 12:09:49  hondwang
 * Add test card phy testing function
 *
 *
 * $Endlog$
 *-------------------------------------------------
 */
