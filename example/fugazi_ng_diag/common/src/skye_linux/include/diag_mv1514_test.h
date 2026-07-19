/* $Id: diag_mv1514_test.h,v 1.2 2015/05/25 03:59:10 steja Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/skye_linux/include/diag_mv1514_test.h,v $
 *-----------------------------------------------------------------------------
 * diag_mv1514_test.h
 *
 * June 2013, steja
 * Copyright (c) 2013~2015 by Cisco Systems, Inc.
 * All rights reserved.
 *-----------------------------------------------------------------------------
 */
 
#ifndef __DIAG_MV1514_TEST_H__
#define __DIAG_MV1514_TEST_H__

#define LINK_MAX_TIME               3000    /* Link up in 3 seconds. */
#define LINK_DELAY                  1   /* 1 millisecond delay */
#define MRV88E1512_RESET_TIMEOUT    15  /* TRESET 10 ms min */
#define PHY_AUTO_NEG_DELAY          1000  /* 700 milliseconds delay. But*/

/* Marvell phy address */
#define MRV88E1512_PHY_ID_1_VALUE               0x0141
#define MRV88E1512_PHY_ID_2_VALUE               0x0dd1
#define MRV88E1512_PHY_ID                       0x0

/* Shared PHY (88E series) Registers Offsets */
#define MRV88E1512_PHYID_1_REG          2
#define MRV88E1512_PHYID_2_REG          3
#define MRV88E1512_PAGE_ADDRESS_REG     22

/* Page 0 Register Offsets - Copper */
#define PAGE_0                              0x00
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

/* Page 1 Register Offsets - Fiber */
#define PAGE_1                                      0x01
#define MRV88E1512F_CONTROL_REG                     0
#define MRV88E1512F_STATUS_REG                      1
#define MRV88E1512F_AUTONEG_ADVR_REG                4
#define MRV88E1512F_LINK_PART_AV_REG                5
#define MRV88E1512F_AUTONEG_EXPANSION_REG           6
#define MRV88E1512F_NEXT_PAGE_REG                   7
#define MRV88E1512F_LP_NEXT_PAGE_REG                8
#define MRV88E1512F_EXTENDED_STATUS_REG             15
#define MRV88E1512F_SPECIFIC_CONTROL1_REG           16
#define MRV88E1512F_SPECIFIC_STATUS_REG             17
#define MRV88E1512F_INT_ENABLE_REG                  18
#define MRV88E1512F_INT_STATUS_REG                  19
#define MRV88E1512F_REC_ERROR_COUNTER_REG           21
#define MRV88E1512F_PRBS_CONTROL_REG                23
#define MRV88E1512F_PRBS_ERROR_CONNTER_LSB_REG      24
#define MRV88E1512F_PRBS_ERROR_CONNTER_MSB_REG      25
#define MRV88E1512F_SPECIFIC_CONTROL2_REG           26

/* Page 2 Register Offsets - MAC */
#define PAGE_2                                      0x02
#define MRV88E1512M_SPECIFIC_CONTROL1_REG           16
#define MRV88E1512M_SPECIFIC_INT_ENABLE_REG         18
#define MRV88E1512M_SPECIFIC_STATUS_REG             19
#define MRV88E1512M_COPPER_RXER_BYTE_CAPTURE_REG    20
#define MRV88E1512M_SPECIFIC_CONTROL2_REG           21
#define MRV88E1512M_RGMII_OUT_IMP_CALI_OV_REG       24
#define MRV88E1512M_RGMII_OUT_IMP_TARGET_REG        25
#define MRV88E1512_MAC_SPD_MASK                 0x2040
#define MRV88E1512_MAC_SPD_1000M                0x0040
#define MRV88E1512_MAC_SPD_100M                 0x2000
#define MRV88E1512_MAC_SPD_10M                  0x0000

/* Page 3 Register Offsets - LED */
#define PAGE_3                                   0x03
#define MRV88E1512M_LED_FUNCTION_CONTROL_REG     16
#define MRV88E1512M_LED_POLARITY_CONTROL_REG     17

/* Page 6 Register Offsets */
#define PAGE_6                          0x06
#define MRV88E1512_PACKET_GEN_REG       16
#define MRV88E1512_CRC_COUNTER_REG      17
#define MRV88E1512_CRC_CHKR_REG         18
#define MRV88E1512_GEN_CONTROL_REG      20

/* Page 18 Register Offsets - Miscellaneous */
#define PAGE_18                               0x12
#define MRV88E1512_EEE_Buffer_CONTROL1_REG    0
#define MRV88E1512_EEE_Buffer_CONTROL2_REG    1
#define MRV88E1512_EEE_Buffer_CONTROL3_REG    2
#define MRV88E1512_PACKET_GEN_REG             16
#define MRV88E1512_CRC_COUNTER_REG            17
#define MRV88E1512_CHKR_CONTROL_REG           18
#define MRV88E1512_GEN_CONTROL1_REG           20
#define MRV88E1512_Link_DISCON_COUNT_REG      25
#define MRV88E1512_SERDES_RX_ER_BYTE_CAP_REG  26

#define MRV88E1512_REG_PAGE_0           0
#define MRV88E1512_REG_PAGE_1           1
#define MRV88E1512_REG_PAGE_2           2
#define MRV88E1512_REG_PAGE_3           3
#define MRV88E1512_REG_PAGE_4           4
#define MRV88E1512_REG_PAGE_5           5
#define MRV88E1512_REG_PAGE_6           6
#define MRV88E1512_REG_PAGE_18          18

#define MRV88E1512_COP_SPEC_CTRL_REG1_FORCE_LINK    0x0400

#define MRV88E1512_GEN_CTRL_REG_RESET               0x8000
#define MRV88E1512_GEN_CTRL_REG_SGMII_TO_COPPER     0x0001

#define MRV88E1512_CONTROL_PHY_RESET_VALUE          0x8000
#define MRV88E1512_CONTROL_PHY_RESET                0x8000
#define MRV88E1512_CONTROL_LOOPBACK                 0x4000
#define MRV88E1512_CONTROL_FORCE_10_L               0x0000
#define MRV88E1512_CONTROL_FORCE_100_L              0x2000
#define MRV88E1512_CONTROL_FORCE_1000_L             0x0040
#define MRV88E1512_CONTROL_AUTONEG_ENABLE           0x1000
#define MRV88E1512_CONTROL_POWER_DOWN               0x0800
#define MRV88E1512_CONTROL1_POWER_DOWN              0x0040
#define MRV88E1512_CONTROL_ISOLATE                  0x0400
#define MRV88E1512_CONTROL_RESTART_AUTONEG          0x0200
#define MRV88E1512_CONTROL_FULL_DUPLEX              0x0100
#define MRV88E1512_CONTROL_COLLISION_TEST           0x0080
#define MRV88E1512_CONTROL_FORCE_1000_M             0x0040
#define MRV88E1512_CONTROL_FORCE_10_M               0x0000
#define MRV88E1512_CONTROL_SPEED_MASK               0x2040

#define MRV88E1512_STATUS_AUTONEG_CHK               0x0008
#define MRV88E1512_STATUS_LINK_UP                   0x0004

#define MRV88E1512_STATUS1_SPEED                    0xC000
#define MRV88E1512_STATUS1_SPEED_1000               0x8000
#define MRV88E1512_STATUS1_SPEED_100                0x4000
#define MRV88E1512_STATUS1_SPEED_10                 0x0000
#define MRV88E1512_STATUS1_LINK                     0x0400
#define MRV88E1512_RESET_TIMEOUT                    15  /* TRESET 10 ms min */

#define MRV88E1512_SPEED_1000BT         1000
#define MRV88E1512_SPEED_100BT          100
#define MRV88E1512_SPEED_10BT           10

#define MRV88E1512_SPEED_10          0x00       /* LSB: 0x00   MSB : 0x00  */
#define MRV88E1512_SPEED_100         0x40       /*      0x40         0x200 */
#define MRV88E1512_SPEED_1000        0x2000     /*      0x2000       0x4   */

#define MRV88E1512_SPD_SEL_MASK                 0x2040
#define MRV88E1512_SPD_SEL_1000M                0x0040
#define MRV88E1512_SPD_SEL_100M                 0x2000
#define MRV88E1512_SPD_SEL_10M                  0x0000

/* Copper Auto-Nego Advertisement Register (Page 0, Reg 4) */
#define MRV88E1512_10BT_ADV                     0x60
#define MRV88E1512_100BT_ADV                    0x180
/* Copper Auto-Nego Advertisement Register (Page 0, Reg 9) */
#define MRV88E1512_1000BT_ADV                   0x300

/* Copper Specific Control Register 1 (Page 0, Reg 17) */
#define MRV88E1512_P0_R17_DTE_NEED_POWER        0x0004
#define MRV88E1512_LINK_SPEED_MASK              0xC000
#define MRV88E1512_LINK_SPEED_1000              0x8000
#define MRV88E1512_LINK_SPEED_100               0x4000
#define MRV88E1512_LINK_SPEED_10                0x0000
#define MRV88E1512_LINK_UP                      0x0400
#define MRV88E1512_SYNC                         0x0020

#define MRV88E1512_ENABLE_STUB_TEST    0x0008

#define MRV88E1512M_LED_POLARITY_MASK             0xFF
#define MRV88E1512M_LED0_POLARITY_LOW_HIGH        0x00
#define MRV88E1512M_LED0_POLARITY_HIGH_LOW        0x01
#define MRV88E1512M_LED0_POLARITY_LOW_TRISTATE    0x02
#define MRV88E1512M_LED0_POLARITY_HIGH_TRISTATE   0x03

#define MRV88E1512M_LED1_POLARITY_LOW_HIGH        0x00
#define MRV88E1512M_LED1_POLARITY_HIGH_LOW        0x04
#define MRV88E1512M_LED1_POLARITY_LOW_TRISTATE    0x08
#define MRV88E1512M_LED1_POLARITY_HIGH_TRISTATE   0x0C

#define MRV88E1512M_LED2_POLARITY_LOW_HIGH        0x00
#define MRV88E1512M_LED2_POLARITY_HIGH_LOW        0x10
#define MRV88E1512M_LED2_POLARITY_LOW_TRISTATE    0x20
#define MRV88E1512M_LED2_POLARITY_HIGH_TRISTATE   0x30

#define MRV88E1512M_LED_NORMAL           0x0000
#define MRV88E1512M_LED0_FORCE_OFF       0x0008
#define MRV88E1512M_LED1_FORCE_OFF       0x0080
#define MRV88E1512M_LED2_FORCE_OFF       0x0800
#define MRV88E1512M_LED0_FORCE_ON        0x0009
#define MRV88E1512M_LED1_FORCE_ON        0x0090
#define MRV88E1512M_LED2_FORCE_ON        0x0900
#define MRV88E1512M_LED0_FORCE_BLINK     0x000B
#define MRV88E1512M_LED1_FORCE_BLINK     0x00B0
#define MRV88E1512M_LED2_FORCE_BLINK     0x0B00
#define MRV88E1512M_LED_RST_VALUE        0x001E

#define MRV88E1512M_LED_CONFIG_MASK      0x0FFF

#define GE_FP_CPU0_PORT  5
#define GE_FP_CPU1_PORT  2

enum {
    SOLID = 0,
    BLINK,
    FORCE_LED_OFF,
};

#define LED0      1    /* Speed status */
#define LED1      2    /* Speed status */
#define LED2      3   /* Link status */
#define ALL_LED   4
#define RST_VAL   5
#define ALL_GREEN 6

enum {
    LOW_HIGH = 0,
    HIGH_LOW,
    LOW_TRISTATE,
    HIGH_TRISTATE
};
/*
 * Marvell GE PHYs have multiple pages. This struct defines the registers
 * and the page associate with them.
 */
typedef struct dev_mrvl_reg_info_t_ {
    uint16_t 	page;		/* page number */
    reg_info_t	*reg_p;		/* Registers table */
} dev_mrvl_reg_info_t;

extern int mv1514_test(int);
extern int reset_phy_88E1514_register(void);
extern int select_phy_page_reg(int);
extern int enable_phy_lpbk(int);
extern int enable_phy_ext_lpbk(int);
extern int phy_lpbk_speed(int);
extern int led_polarity_test(int, int, int);
extern int led_function_test(int, int);
extern boolean is_cpu0(void);

#endif  /*__DIAG_MV1514_TEST_H__ */


/*-------------------------------------------------
 * $Log: diag_mv1514_test.h,v $
 * Revision 1.2  2015/05/25 03:59:10  steja
 * Add Support Skye SM
 *
 * Revision 1.1.4.2  2015/04/29 11:36:24  steja
 * Code check-in to skye-branch2 for ER code review
 *
 *
 * ------------------------------------------------------------------
 * Revision 1.1.2.1  2014/07/21 01:56:36  palin2
 * Initial check-in Skye module side Diag code.
 *
 * $Endlog$
 */
