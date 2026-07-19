/* $Id: diag_ge_phy_lib.h,v 1.2 2021/09/24 01:21:06 harrchan Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/elixir/diag_ge_phy_lib.h,v $
 *------------------------------------------------------------------
 * 
 * diag_ge_phy_lib.h
 *
 * Copyright (c) 2018 - 2019 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#include "dev_object.h"

typedef uint16_t	smi_t;	/* SMI data type */

/* create device object */
dev_object_t    *diag_get_88e11112_obj (int);
extern int diag_ge_phy_setting_reg (int, smi_t, uchar, smi_t);

extern int diag_88e1112_ge_set_txtype(int, ushort);

extern int diag_ge_phy_reg_rd_if(int, smi_t, uchar, smi_t *);
extern int diag_ge_phy_reg_wr_if(int, smi_t, uchar, smi_t *);

extern int diag_ge_phy_all_led_off(int);
extern int diag_ge_phy_all_led_on(int);

extern int diag_ge_phy_config_gephy_fiber(int);
extern int diag_read_sfp_cookie (int);

extern int gephy_set_1000basex_mode(void);
extern int gephy_set_loopback_mode(int);
extern int gephy_get_loopback_mode(void);
#define CPU_MAC_1000BASEX_MODE_REG   (0x02)

#define LINK_STATUS_POLLING_PERIOD 100 /* 100ms */
#define LINK_STATUS_POLLING_ROUND  100
#define BUFFER_TIME 100 /* 100ms */

#define PHY_88E1112_GE0_SMIADDR  0x1C /* instead of PLAT_GE0_SMIADDR */
#define PHY_88E1112_GE1_SMIADDR  0x1D /* instead of PLAT_GE1_SMIADDR */

#define GE0    0
#define GE1    1
#define GEESW  2

#define PLAT_GE0_ETHNUM        2
#define PLAT_GE1_ETHNUM        1

#define GE0_ETH    "eth2"
#define GE1_ETH    "eth1"
#define ESW_ETH    "eth0"

#define PLAT_LED_GE0     0
#define PLAT_LED_GE1     1

#define GEWAN_TXTYPE_A   1
#define GEWAN_TXTYPE_B   0

#define GE_LED_ON  0x8989
#define GE_LED_OFF 0x9898

#define LPBK_PKG    3

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

#define ESW_REG_PAGE_252 0xfc
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

#define COPPER_SPD_MASK            (uint16_t)0x2040
#define COPPER_SPD_10Mbps          (uint16_t)0x0000
#define COPPER_SPD_100Mbps         (uint16_t)0x2000
#define COPPER_SPD_1000Mbps        (uint16_t)0x0040

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

#define ERR_BUF_SIZE				80

#define NUMBER_OF_SFP	        2
#define INVALID_PID	            -1
#define SFP_I_INIT_TIME	        330	    /* t_init - 300 ms in MSA */
#define SFP_I2C_DEV_ADDR        (0xAC >> 1) /* 0xAC > 1 = 0x56  */

/* SFP Encoding - 0x0B */
#define SFP_ENCODE_UNKNOWN	    0	    /* Upspecified */
#define SFP_ENCODE_8B10B	    1	    /* 8B10B */
#define SFP_ENCODE_4B5B		    2	    /* 4B5B */
#define SFP_ENCODE_NRZ		    3	    /* NRZ */
#define SFP_ENCODE_MANCHESTER	4	    /* Manchester */
#define SFP_ENCODE_SONET	    5	    /* SONET Scrambled */
#define SFP_ENCODE_INVALID	    0xFF	/* Reserved */

/* SFP Ethernet Compliance codes */
#define SFP_ETH_COMP_CODES      0x6
#define SFP_1000BASE_SX         0x01
#define SFP_1000BASE_LX         0x02
#define SFP_1000BASE_CX         0x04
#define SFP_1000BASE_T          0x08
#define SFP_100BASE_LX10        0x10
#define SFP_100BASE_FX          0x20
#define SFP_BASE_BX10           0x40
#define SFP_BASE_PX             0x80

/* SFP GLC-GE-100FX equates */
#define SFP_GE_100FX_REG	    0x1C	/* Register offset to be updated */
#define SFP_GE_100FX_REG_FX_L	0x10	/* FX mode enable bit */
#define SFP_GE_100FX_REG_FX_H	0x88	/* Write and shadow page */
#define SFP_GE_100FX_REG18      0x18    /* Edge Control Register offset */
#define SFP_GE_100FX_REG_EC_L   0x30    /* 100Base-T 0ns */
#define SFP_GE_100FX_REG_EC_H   0x04    /* Normal Tx mode */

/* Fields offset defines */
#define SFP_COO_ID	            0x00
#define SFP_COO_X_ID	        0x01
#define SFP_COO_CNT	            0x02
#define SFP_COO_XVR	            0x03
#define SFP_COO_GECC	        0x06	/* Gigabit Ethernet Compliance Codes */
#define SFP_COO_ENC	            0x0B
#define SFP_COO_BR_N	        0x0C
#define SFP_COO_L_9KM	        0x0E
#define SFP_COO_L_9M	        0x0F
#define SFP_COO_L_50	        0x10
#define SFP_COO_L_62	        0x11
#define SFP_COO_L_CU	        0x12
#define SFP_COO_VEND	        0x14
#define SFP_COO_CH_S	        0x24
#define SFP_COO_VEN_O	        0x25
#define SFP_COO_VEN_PN	        0x28
#define SFP_COO_VEN_R	        0x38
#define SFP_COO_LSR_W	        0x3C
#define SFP_COO_DWDM_W	        0x3E
#define SFP_COO_CC_B	        0x3F
#define SFP_COO_OPT	            0x40
#define SFP_COO_BR_MAX	        0x42
#define SFP_COO_BR_MIN	        0x43
#define SFP_COO_VEN_SN	        0x44
#define SFP_COO_DATE	        0x54
#define SFP_COO_DIAG	        0x5C
#define SFP_COO_ENH	            0x5D
#define SFP_COO_CC_X	        0x5F
#define SFP_COO_VEND_SP	        0x60
#define SFP_COO_XID	            0x60

/* Fields Size */
#define SFP_COO_ID_L	        1
#define SFP_COO_XID_L	        1
#define SFP_COO_CNT_L	        1
#define SFP_COO_XVR_L	        8
#define SFP_COO_GECC_L	        1
#define SFP_COO_ENC_L	        1
#define SFP_COO_BR_N_L	        1
#define SFP_COO_L_9KM_L	        1
#define SFP_COO_L_9M_L	        1
#define SFP_COO_L_50_L	        1
#define SFP_COO_L_62_L	        1
#define SFP_COO_L_CU_L	        1
#define SFP_COO_VEND_L	        16
#define SFP_COO_CH_S_L	        1
#define SFP_COO_VEN_O_L	        3
#define SFP_COO_VEN_P_L	        16
#define SFP_COO_VEN_R_L	        4
#define SFP_COO_LSR_W_L	        2
#define SFP_COO_DWDM_L	        1
#define SFP_COO_CC_B_L	        1
#define SFP_COO_OPT_L	        2
#define SFP_COO_BR_MX_L	        1
#define SFP_COO_BR_MN_L	        1
#define SFP_COO_VEN_S_L	        16
#define SFP_COO_DATE_L	        8
#define SFP_COO_DIAG_L	        1
#define SFP_COO_ENH_L	        1
#define SFP_COO_CC_X_L	        1
#define SFP_COO_VN_SP_L	        32
#define SFP_COO_XID_L	        1

/* SFP Extended ID (GBIC) - 0x60 */
#define SFP_XID_GE_100FX	    0x2A
#define SFP_XID_FE_100FX	    0x2B
#define SFP_XID_FE_100LX	    0x2C

/* SFP Type */
#define SFP_DEFAULT             0 
#define SFP_GE_100FX            1
#define SFP_FE_100FX            2
#define SFP_GLC_TE              3

/* SFP Copper ABCU-5710RZ-CS2 register offset */
#define SFP_COPPER_CONTROL      0x0
#define SFP_COPPER_STATUS       0x1
#define SFP_COPPER_AO_NG_AD     0x4
#define SFP_COPPER_AO_NG_LPA    0x5
#define SFP_COPPER_AO_EXP       0x6
#define SFP_COPPER_AO_NPT       0x7
#define SFP_COPPER_AO_LPRNT     0x8
#define SFP_COPPER_MA_SL_CR     0x9
#define SFP_COPPER_MA_SL_SR     0xA
#define SFP_COPPER_EC1          0x10
#define SFP_COPPER_ES1          0x11
#define SFP_COPPER_INT_REG      0x12
#define SFP_COPPER_EC2          0x14
#define SFP_COPPER_REC          0x15
#define SFP_COPPER_CD1          0x16
#define SFP_COPPER_EC3          0x1A
#define SFP_COPPER_ES2          0x1B
#define SFP_COPPER_CD2          0x1C
#define CISCO_AVAGO_SFP         {0x43,0x49,0x53,0x43,0x4f,0x2d,0x41,0x56,0x41, \
                                 0x47,0x4f,0x20,0x20,0x20,0x20,0x20}

#define SFP_CLR_INT_H           0x0
#define SFP_CLR_INT_L           0x0
#define SFP_FRC_MASTER_H        0x1B
#define SFP_FRC_MASTER_L        0x00
#define SFP_RES_EN_AUTO_NEG_H   0x91 
#define SFP_RES_EN_AUTO_NEG_L   0x40
#define SFP_SEL_P7_REG30_H      0x00 
#define SFP_SEL_P7_REG30_L      0x07
#define SFP_FRC_GBPS_MODE_H     0x08 
#define SFP_FRC_GBPS_MODE_L     0x08 
#define SFP_SEL_P16_REG30_H     0x00 
#define SFP_SEL_P16_REG30_L     0x10 
#define SFP_EN_LBPK_STUB_H      0x00 
#define SFP_EN_LBPK_STUB_L      0x02 
#define SFP_SEL_P18_REG30_H     0x00 
#define SFP_SEL_P18_REG30_L     0x12 
#define SFP_DIS_NEXT_H          0x80 
#define SFP_DIS_NEXT_L          0x01 

#define SFP_PHY_RESET_DELAY 3000
#define SFP_BUFFER_256         256
#define SFP_EEPROM_16_LENGTH   (16+1)
#define SFP_EEPROM_8_LENGTH    (8+1)
#define SFP_EEPROM_SIZE 256


#define SPD_10MBPS   10
#define SPD_100MBPS  100
#define SPD_1000MBPS 1000

/*-------------------------------------------------
 * $Log: diag_ge_phy_lib.h,v $
 * Revision 1.2  2021/09/24 01:21:06  harrchan
 * Collapse Elixir-branch to Main Trunk.
 *
 * Revision 1.1.2.5  2021/05/31 10:48:35  illiu
 * Add macro
 *
 * Revision 1.1.2.4  2020/11/13 11:48:34  illiu
 * Fix GE0/1 Led
 *
 * Revision 1.1.2.3  2020/11/05 06:40:37  harrchan
 * 1.According elixir fpga spec to change the definition of SFP present and SFP transmitter disable bit.
 * 2.Add GE PHY1 SFP external loopback test
 *
 * Revision 1.1.2.2  2020/09/16 02:25:35  harrchan
 * Support GE1 SFP test
 *
 * Revision 1.1.2.1  2020/09/09 09:09:51  illiu
 * First version which has been ported with Dreamliner and Marvell CPSS
 *
 * Revision 1.3  2019/06/24 07:21:37  wilbhuan
 * Supported Pluggable Serial Module.
 *
 * Revision 1.2  2019/01/10 06:36:26  wilbhuan
 * The beginning of Betelgeuse application code.
 *
 *-------------------------------------------------
 */
