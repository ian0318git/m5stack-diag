/* $Id: dev_88e1543.h,v 1.6 2020/09/30 09:46:09 alicehua Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/chips/dev_88e1543_marvell/dev_88e1543.h,v $
 * -----------------------------------------------------------------------------
 * File Name: dev_phy_88e1543.h
 *
 * Description: Contains common definitions for the Marvell 88E1543 Quad-PHY.
 *      
 * James Lin - Sep 2011.
 *
 * Copyright (c) 2012 - 2019 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------------------
 */

#ifndef __DEV_PHY_88E1543_H__
#define __DEV_PHY_88E1543_H__

#include "dev_object.h"
#include "common_utils.h"

#define MRV88E1543_ERR_BUF_SIZE                 (80)
#define LINK_UP_TOUT                            (500) /* 10 msecs resolution */

#define MAX_POLLING_LINKUP_TIMES     5
#define WAIT_FOR_LINKUP              1000

/************************* 88E1543 ************************/
#define MRV88E1543_REG_PAGE_0                   0
#define MRV88E1543_REG_PAGE_1                   1
#define MRV88E1543_REG_PAGE_2                   2
#define MRV88E1543_REG_PAGE_3                   3
#define MRV88E1543_REG_PAGE_4                   4
#define MRV88E1543_REG_PAGE_5                   5
#define MRV88E1543_REG_PAGE_6                   6
#define MRV88E1543_REG_PAGE_7                   7
#define MRV88E154x_REG_PAGE_16                  16
#define MRV88E1543_REG_PAGE_18                  18
#define MRV88E1543_REG_PAGE_29                  29
#define MRV88E1543_REG_PAGE_250                 250
#define MRV88E1543_REG_PAGE_254                 254
#define MRV88E1543_REG_PAGE_255                 255
#define MRV88E1543_REG_PAGE_MAX                 256

/* Page 0 Register Offsets - Copper */
#define MRV88E1543_CONTROL_REG                  0
#define MRV88E1543_STATUS_REG                   1
#define MRV88E1543_PHY_ID1                      2
#define MRV88E1543_PHY_ID2                      3
#define MRV88E1543_AUTONEG_ADVR_REG             4
#define MRV88E1543_LINK_PART_AV_REG             5
#define MRV88E1543_AUTONEG_EXPANSION_REG        6
#define MRV88E1543_NEXT_PAGE_REG                7
#define MRV88E1543_LP_NEXT_PAGE_REG             8
#define MRV88E1543_1000B_CNTL_REG               9                              
#define MRV88E1543_1000B_STATUS_REG             10
#define MRV88E1543_EXTENDED_STATUS_REG          15
#define MRV88E1543_SPECIFIC_CONTROL1_REG        16
#define MRV88E1543_SPECIFIC_STATUS1_REG         17
#define MRV88E1543_INT_ENABLE_REG               18
#define MRV88E1543_SPECIFIC_STATUS2_REG         19
#define MRV88E1543_SPECIFIC_CONTROL2_REG        20
#define MRV88E1543_REC_ERROR_COUNTER_REG        21
#define MRV88E1543_PAGE_ADDRESS_REG             22
#define MRV88E1543_GLOBAL_INTERRUPT_REG         23
#define MRV88E1543_SPECIFIC_CONTROL3_REG        26

/* Copper Control Register (Page 0, Reg 0) */
#define MRV88E1543_COOPER_RST                   0x8000
#define MRV88E1543_LPBK_ENA                     0x4000
#define MRV88E1543_SPD_SEL_MASK                 0x2040
#define MRV88E1543_SPD_SEL_1000M                0x0040
#define MRV88E1543_SPD_SEL_100M                 0x2000
#define MRV88E1543_SPD_SEL_10M                  0x0000
#define MRV88E1543_AUTO_NEO_ENA                 0x1000
#define MRV88E1543_PWR_DOWN                     0x0800
#define MRV88E1543_RST_AUTO_NEO                 0x0200
#define MRV88E1543_FULL_DUPLEX                  0x0100

#define COOPER_RST_POLLING_COUNT                300

#define MRV88E1543_SPEED_MASK                   0x0007
#define MRV88E1543_SGMII_SPD_1000               0x0006
#define MRV88E1543_SGMII_SPD_100                0x0005
#define MRV88E1543_SGMII_SPD_10                 0x0004

#define MRV88E1543_LPBK_ENA                     0x4000
#define MRV88E1543_AUTO_NEG                     0x1000
#define MRV88E1543_PWR_DOWN                     0x0800


#define MRV88E1543_PHY_OUI_HI                   0x0141  
#define MRV88E1543_PHY_OUI_LO                   0x03
#define MRV88E1543_PHY_OUI_LO_MASK              0xFC00

/* Copper Auto-Nego Advertisement Register (Page 0, Reg 4) */
#define MRV88E1543_10BT_ADV                     0x60    
#define MRV88E1543_100BT_ADV                    0x180
#define MRV88E1543_1000BT_ADV                   0x300

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
#define MRV88E1543_CNTL_FORCE_LINK              0x0400
#define MRV88E1543_P0_R16_PWR_DOWN              0x0004

/* Copper Specific Control Register 1 (Page 0, Reg 17) */
#define MRV88E1543_P0_R17_DTE_NEED_POWER        0x0004
#define MRV88E1543_LINK_SPEED_MASK              0xC000
#define MRV88E1543_LINK_SPEED_1000              0x8000
#define MRV88E1543_LINK_SPEED_100               0x4000
#define MRV88E1543_LINK_SPEED_10                0x0000
#define MRV88E1543_LINK_UP                      0x0400
#define MRV88E1543_SYNC                         0x0020

/* Copper Specific Control Register 1 (Page 0, Reg 18) */
#define MRV88E1543_COPPER_SPEC_INTR_ENABLE      18
#define MRV_88E154X_P0_R18_SPD_CHG_INT          0x4000
#define MRV_88E154X_P0_R18_DISABLE_ALL_INT      0x0

/* Cooper Specific Control Register 3 (page 0, Reg 26) */
#define MRV88E1543_P0_R26_DTE_DETECT            0x0100
#define MRV88E1543_P0_R26_DTE_STATUS_DROP_5S    0x0010
#define MRV88E1543_P0_R26_DTE_STATUS_DROP_MSK   0x00F0
#define MRV88E1543_P0_R26_CLASS_A               0x9000

/* Page 1 Register Offsets - Fiber */
#define MRV88E1543_P1_CONTROL_REG                     0
#define MRV88E1543_P1_FIBER_SPECIFIC_CONTROL_REG      26

/* Fiber Control Register (Page 1, Reg 0) */
#define MRV88E1543_P1_R0_PWR_DOWN               0x0800
#define MRV88E1543_P1_R17_SYNC                  0x0020

/* Fiber INTR Enable Register (Page 1, Reg 18) */
#define MRV88E1543_P1_INTR_EN_REG               18
#define MRV88E1543_P1_AUTO_NEG_INTR_EN          0x800

/* Fiber Specific Control Register 2 (Page 1, Reg 26) */

#define MRV88E1543_P1_R26_SGMII_AMP_14MV        0x0
#define MRV88E1543_P1_R26_SGMII_AMP_112MV       0x1
#define MRV88E1543_P1_R26_SGMII_AMP_210MV       0x2
#define MRV88E1543_P1_R26_SGMII_AMP_308MV       0x3
#define MRV88E1543_P1_R26_SGMII_AMP_406MV       0x4
#define MRV88E1543_P1_R26_SGMII_AMP_504MV       0x5
#define MRV88E1543_P1_R26_SGMII_AMP_602MV       0x6
#define MRV88E1543_P1_R26_SGMII_AMP_700MV       0x7
#define MRV88E1543_P1_R26_SGMII_AMP_CLR_MUSK    0x7

/* Page 2 Register Offsets - MAC */
#define MRV88E1543_MAC_CNTL_REG2                21
#define MRV88E1543_MAC_SPD_MASK                 0x7
#define MRV88E1543_MAC_SPD_1000M                0x6
#define MRV88E1543_MAC_SPD_100M                 0x5
#define MRV88E1543_MAC_SPD_10M                  0x4

/* Page 3 Register Offsets - LOS, INIT, STATUS[1:0] */
#define MRV88E1543_FUNC_CONTROL_REG             16
#define MRV88E1543_POL_CONTROL_REG              17
#define MRV88E1543_TMR_CONTROL_REG              18
#define MRV88E1543_PTP_LED_FN_CONTROL_REG       19
#define MRV88E1543_DEFAULT_FUNC_CONTROL         0x1777
#define MRV88E1543_FORCE_ON_LED                 0x9999
#define MRV88E1543_FORCE_OFF_LED                0x8888

/* Page 3 Register 18 LED Timer Control Register */
#define PHY_TIMER_CNTRL_FORCE_INT       (0x8000)
#define PHY_TIMER_CNTRL_INTR_EN         (0x0080)

/* Page 4 Register Offsets */
#define MRV_88E154X_P4_R18_QSGMII_INT_EN_REG    18
#define MRV_88E154X_P4_R18_SPD_CHG_INT          0x4000
#define MRV_88E154X_P4_R27_QSGMII_CNTL_REG_2    27
#define MRV_88E154X_P4_R27_DISABLE_CLOCK        0x3E80
#define MRV_88E154X_P4_R27_QSGMII_LOOPBACK      0x4000
#define MRV_88E154X_P4_R27_QSGMII_CROSSOVER     0x0003

/* Page 6 Register Offsets */
#define MRV88E1543_P6_COPPER_PKT_GEN            16
#define MRV88E1543_P6_CHECKER_CTRL              18
#define MRV88E1543_P6_MISC_TEST                 26

/* Checker Control Register (Page 6, Reg 18) */
#define MRV88E1543_P6_R18_ENA_STUB_TEST         0x0008

/* MISC Test Register (Page 6, Reg 26) */
#define MRV88E1543_P6_R26_TEMP_THRESHOLD_MASK   0x1F00
#define MRV88E1543_P6_R26_ENA_TEMP_SENSOR_INTR  0x0080
#define MRV88E1543_P6_R26_TEMP_NEG_25C          0x0000
#define MRV88E1543_P6_R26_TEMP_0C               0x0500
#define MRV88E1543_P6_R26_TEMP_100C             0x1900
#define MRV_88E154X_P6_R26_ENABLE_TX_TCLK       0x8000

/* Page 29 Register Offsets */
#define MRV88E1543_REG_30                       30

/* Page 254 Register Offsets */
#define MRV88E1543_REG_24                       24
#define MRV88E1543_REG_26                       26

/* Page 16 Register Offsets */
#define MRV88E154X_P16_R0_LINK_CRYPT_READ_ADDR    0
#define MRV88E154X_P16_R1_LINK_CRYPT_WRITE_ADDR   1
#define MRV88E154X_P16_R2_LINK_CRYPT_DATA_LO      2
#define MRV88E154X_P16_R3_LINK_CRYPT_DATA_HI      3

/* Page 18 Register Offsets */
#define MRV88E154X_P18_R27_GEN_CON_REG2          27
#define MRV88E154X_P18_R27_ENABLE_MACSEC         0x2000

/* Marvell 151x PHY - 1000BaseT Control Reg. (Page0, Register 9) */
#define MRVL1548_1000T_CTRL_REG   0x9
#define PHY_TESTMODE_MSK          0xE000
#define PHY_TESTMODE_OFF          13
#define PHY_TESTMODE_NORMAL       0x0   /* Normal Mode */
#define PHY_TESTMODE_1            0x1   /* TestMode 1-Transmit Waveform Test */
#define PHY_TESTMODE_2            0x2   /* TestMode 2-Transmit Jitter Test (Master) */
#define PHY_TESTMODE_3            0x3   /* TestMode 3-Transmit Jitter Test (Slave) */
#define PHY_TESTMODE_4            0x4   /* TestMode 4-Transmit Distortion Test */
#define PHY_TESTMODE_5            0x5   /* 10M TestMode  */
#define PHY_TESTMODE_6            0x6   /* 10M Data 0/1 TestMode */
#define PHY_TESTMODE_7            0x7   /* 100M TestMode */
#define PHY_TESTMODE_8            0x8   /* 100 TestMode */
#define PHY_TESTMODE_1_REG_VAL    0x3F00
#define PHY_TESTMODE_2_REG_VAL    0x5F00
#define PHY_TESTMODE_3_REG_VAL    0x7700
#define PHY_TESTMODE_4_REG_VAL    0x9F00


/* LinkCrypt Memory Map */
#define MRV88E154X_GENERAL_PORT_OFFSET 0x800
#define MRV88E154X_CRYPT_IGR_GEN       0xb    /* for disabe drop bad tag */
#define MRV88E154X_CRYPT_ISC_GEN       0x10   /* for VFL setting */
#define MRV88E154X_ELU_TBL_OFF4        0x104  /* egress default match, encrypt+auth */
#define MRV88E154X_ILU_TBL_OFF6        0x206  /* ingress decrypt+auth and VFL setting*/ 
#define MRV88E154X_ILU_TBL_OFF7        0x207  /* igr default match */ 
#define MRV88E154X_EGR_CTXT_OFF0       0x300  /* set sci[31:0] */
#define MRV88E154X_EGR_CTXT_OFF1       0x301  /* set sci[63:32] */
#define MRV88E154X_EGR_CTXT_OFF3       0x303  /* set tci[7:0] */
#define MRV88E154X_ENC_KEY_OFF0        0x400  /* set encrypt key */
#define MRV88E154X_ENC_KEY_OFF1        0x401  
#define MRV88E154X_ENC_KEY_OFF2        0x402  
#define MRV88E154X_ENC_KEY_OFF3        0x403  
#define MRV88E154X_EGR_HKEY_OFF0       0x480  /* set egres hash key */
#define MRV88E154X_EGR_HKEY_OFF1       0x481  
#define MRV88E154X_EGR_HKEY_OFF2       0x482  
#define MRV88E154X_EGR_HKEY_OFF3       0x483  
#define MRV88E154X_DEC_KEY_OFF0        0x500  /*  set decrypt key */
#define MRV88E154X_DEC_KEY_OFF1        0x501  
#define MRV88E154X_DEC_KEY_OFF2        0x502  
#define MRV88E154X_DEC_KEY_OFF3        0x503  
#define MRV88E154X_IGR_HKEY_OFF0       0x580  /*  set ingress hash key */
#define MRV88E154X_IGR_HKEY_OFF1       0x581  
#define MRV88E154X_IGR_HKEY_OFF2       0x582  
#define MRV88E154X_IGR_HKEY_OFF3       0x583  

/* MACsec Counters */
#define MRV88E154X_MACSEC_CNT_OFFSET   0x150
#define MRV88E154X_IGR_HIT             0x2800
#define MRV88E154X_IGR_OK              0x2820
#define MRV88E154X_IGR_UNCHK           0x2840
#define MRV88E154X_IGR_DELAY           0x2860
#define MRV88E154X_IGR_LATE            0x2880
#define MRV88E154X_IGR_INVLD           0x28A0
#define MRV88E154X_IGR_NOTVLD          0x28C0
#define MRV88E154X_EGR_PKT_PORT        0x28E0
#define MRV88E154X_EGR_PKT_ENC         0x2900
#define MRV88E154X_EGR_HIT             0x2920
#define MRV88E154X_IGR_OCT_VAL         0x2940
#define MRV88E154X_IGR_OCT_DEC         0x2941
#define MRV88E154X_IGR_UNTAG           0x2942
#define MRV88E154X_IGR_NOTAG           0x2943
#define MRV88E154X_IGR_BADTAG          0x2944
#define MRV88E154X_IGR_UNKSCI          0x2945
#define MRV88E154X_IGR_NOSCI           0x2946
#define MRV88E154X_IGR_UNUSSA          0x2947
#define MRV88E154X_IGR_NOUSSA          0x2948
#define MRV88E154X_IGR_OCT_TOT         0x2949
#define MRV88E154X_EGR_OCT_PORT        0x294A
#define MRV88E154X_EGR_OCT_ENC         0x294B
#define MRV88E154X_EGR_OCT_TOT         0x294C
#define MRV88E154X_IGR_MISS            0x294D
#define MRV88E154X_EGR_MISS            0x294E
#define MRV88E154X_IGR_REDIR           0x294F

/* Test mode set*/
#define MRV_88E154X_NORMAL_MODE                    0x9140
#define MRV_88E154X_TEST_MODE_1                    0x3f00
#define MRV_88E154X_TEST_MODE_2                    0x5f00
#define MRV_88E154X_TEST_MODE_3                    0x7700
#define MRV_88E154X_TEST_MODE_4                    0x9f00
#define MRV_88E154X_SET_PHY_TO_MASTER_MODE         0x1F00
#define MRV_88E154X_SET_PHY_TO_SLAVE_MODE          0x1700
#define MRV_88E154X_PHY_SOFT_RESET                 0x9140



/*
 * General Defines
 */

#define MRV88E1543_PORTS                        4
#define PHY_LOG_BUF_SIZE                        384
#define MRVL_PHONE_DETECT_TIME                  5
#define MRV88E1543_ERR_MSG_LEN                  500
#define MRV88E1543_PHY_ADDR_DECR                0  /* decremental phy addr */
#define MRV88E1543_PHY_ADDR_INCR                1  /* incremental phy addr */

/* 88E1543: 0x2a, 88E1543M/88E1548L: 0x2b */
/* Page0 Register3 bits 9:4 */
#define MRV88E1543_PHY_MODEL_NUM                0x2a
#define MRV88E1543_PHY_MODEL_NUM_SHIFT          4
#define MRV88E1543_PHY_MODEL_MASK               0x3F
#define MRV88E1548L_PHY_MODEL_NUM                0x2b


/* Page18 Register20 General Control Register 1 :bits 2:0 */
#define MRV88E1543_GENERAL_CONTROL_REG           20

#define MRV88E1543_MODE_SGMII_TO_QSGMII          0x0005
#define MRV88E1543_MODE_QSGMII_TO_AUTO_DETECT    0x0007
#define MRV88E1543_MODE_QSGMII_MASK              0x0007
#define MRV88E1543_MODE_RESET                    0x8000

#define SIG_COPPER 0
#define SIG_FIBER 1

#define DISABLE_SIG   0
#define ENABLE_SIG    1

#define MIN_TEST 1

/* define for PHY setting */
#define SET_PHY_BIT11     0x0800

#define SMIREAD(d, x, y, z, u)           ((dev_88e1543_object_t *)(d))->  \
                                         callout_fvt->smi_read            \
                                         ((uint)(x), (uint)(y), (uint)(z),\
                                          (uint *)(u))
#define SMIWRITE(d, x, y, z, u)          ((dev_88e1543_object_t *)(d))->  \
                                         callout_fvt->smi_write           \
                                         ((uint)(x), (uint)(y), (uint)(z),\
                                          (uint)(u))

#define MVL_88E1543_REG_TEST(d)          ((dev_88e1543_object_t *)(d))->  \
                                         callin_fvt->register_test        \
                                         ((dev_object_t *)(d))

#define MVL_88E1543_SET_LPBK(d, x, y, z) ((dev_88e1543_object_t *)(d))->  \
                                         callin_fvt->set_loopback         \
                                         ((dev_object_t *)(d), (int)(x),  \
                                          (int)(y), (int)(z))

#define MVL_88E1543_SET_INT_QSGMII_LPBK(d, x, y, z)                           \
                                         ((dev_88e1543_object_t *)(d))->  \
                                         callin_fvt->set_qsgmii_int_loopback  \
                                         ((dev_object_t *)(d), (int)(x),  \
                                          (int)(y), (int)(z))

#define MVL_88E1543_LPBK_MODE(d, x, y)   ((dev_88e1543_object_t *)(d))->  \
                                         callin_fvt->lpbk_mode            \
                                         ((dev_object_t *)(d), (int)(x),  \
                                          (int)(y))

#define MVL_88E1543_CLR_LPBK(d, x)       ((dev_88e1543_object_t *)(d))->  \
                                         callin_fvt->cleanup_loopback     \
                                         ((dev_object_t *)(d), (uint)(x))

#define MVL_88E1543_PWR_UP(d, x, y)      ((dev_88e1543_object_t *)(d))->  \
                                         callin_fvt->power_up             \
                                         ((dev_object_t *)(d), (uint)(x), \
                                          (uint)(y))

#define MVL_88E1543_INTR_GEN(d, x)       ((dev_88e1543_object_t *)(d))->  \
                                         callin_fvt->intr_gen             \
                                         ((dev_object_t *)(d), (uint)(x))
                                         
#define MVL_88E1543_INTR_GEN_FIBER(d, x) ((dev_88e1543_object_t *)(d))->  \
                                         callin_fvt->intr_gen_fiber       \
                                         ((dev_object_t *)(d), (uint)(x))

#define MVL_88E1543_INTR_DISABLE(d, x)   ((dev_88e1543_object_t *)(d))->  \
                                         callin_fvt->intr_disable         \
                                         ((dev_object_t *)(d), (uint)(x))

#define MVL_88E1543_INTR_CLR_FIBER(d, x) ((dev_88e1543_object_t *)(d))->  \
                                         callin_fvt->intr_clr_fiber       \
                                         ((dev_object_t *)(d), (uint)(x))

#define MVL_88E1543_LOCKUP_FIX(d, x)     ((dev_88e1543_object_t *)(d))->  \
                                         callin_fvt->lockup_fix           \
                                         ((dev_object_t *)(d), (uint)(x))

#define MVL_88E1543_SHOW_REG(d, x, y)    ((dev_88e1543_object_t *)(d))->  \
                                         callin_fvt->show_reg             \
                                         ((dev_object_t *)(d),            \
                                          (print_fn_t)(x), (uint)(y))

#define MVL_88E1543_ALTER_REG(d)         ((dev_88e1543_object_t *)(d))->  \
                                         callin_fvt->alter_reg            \
                                         ((dev_object_t *)(d))

#define MVL_88E1543_DISPLAY_REG(d)       ((dev_88e1543_object_t *)(d))->  \
                                         callin_fvt->display_reg            \
                                         ((dev_object_t *)(d))

#define MVL_88E1543_PHONE_DETECT(d, x)   ((dev_88e1543_object_t *)(d))->  \
                                         callin_fvt->phone_detect         \
                                         ((dev_object_t *)(d), (uint)(x))

#define MVL_88E1543_SET_TEST_MODE(d, x)  ((dev_88e1543_object_t *)(d))->  \
                                         callin_fvt->set_test_mode        \
                                         ((dev_object_t *)(d), (uint)(x))

#define MVL_88E1548X_SET_TEST_MODE(d, x, y)      ((dev_88e1543_object_t *)(d))->  \
                                                 callin_fvt->set_1548x_test_mode  \
                                                 ((dev_object_t *)(d), (uint)(x), \
                                                 (print_fn_t)(y))
                                         
#define MVL_88E154X_CLR_MACSEC_CNT(d, x, y)      ((dev_88e1543_object_t *)(d))->  \
                                                 callin_fvt->clr_macsec_cnt       \
                                                 ((dev_object_t *)(d),            \
                                                 (int)(x), (int)(y)) 

#define MVL_88E154X_SET_MACSEC(d, x, y)          ((dev_88e1543_object_t *)(d))->  \
                                                 callin_fvt->set_macsec           \
                                                 ((dev_object_t *)(d),            \
                                                 (int)(x), (boolean)(y)) 

#define MVL_88E154X_INIT_MACSEC(d, x, y)         ((dev_88e1543_object_t *)(d))->  \
                                                 callin_fvt->init_macsec          \
                                                 ((dev_object_t *)(d),            \
                                                 (int)(x), (int)(y)) 

#define MVL_88E154X_MACSEC_CHECK_STATUS(d, x, y, z) ((dev_88e1543_object_t *)(d))->  \
                                                    callin_fvt->check_status         \
                                                    ((dev_object_t *)(d),            \
                                                    (int)(x), (int)(y), (print_fn_t)(z)) 

#define MVL_88E154X_DUMP_STATISTIC(d, x, y, z)      ((dev_88e1543_object_t *)(d))->  \
                                                    callin_fvt->dump_statistic       \
                                                    ((dev_object_t *)(d),            \
                                                    (int)(x), (int)(y), (print_fn_t)(z)) 

#define MVL_88E1548L_INIT(d)                     ((dev_88e1543_object_t *)(d))->  \
                                                 callin_fvt->phy_88e1548l_init       \
                                                 ((dev_object_t *)(d))

typedef struct {
    int  reg_page;  /* page of register */
    int  reg_off;   /* offset of register */
    uint16_t  val;  /* value to set */
    uint16_t  mask; /* mask of register r/w capability */
} mrvl_88e1543_phy_setup_t;

enum
{
    ETH_MODE_GE,
    ETH_MODE_FE100,
    ETH_MODE_FE10,
};

typedef enum mrvl_88e1543_link_status_t_ {
    LINK_UP,
    LINK_DOWN
} mrvl_88e1543_link_status_t;

typedef enum mrvl_88e1543_qsgmii_int_en_t_ {
    FIFO_OVER_UNDERFLOW_INT_EN,
    FALSE_CARRIER_INT_EN,
    SYMBOL_ERROR_INT_EN,
    LINK_STS_CHANGE_INT_EN,
    AUTO_NEGO_COMPLETED_INT_EN,
    PAGE_RECVD_INT_EN,
    DUPLEX_CHGD_INT_EN,
    SPEED_CHGD_INT_EN
} mrvl_88e1543_qsgmii_int_en_t;

enum {
    DEV_88E154X_DISABLE,
    DEV_88E154X_ENABLE
};

/* define loopback mode */
enum
{
    SGMII_PHY_LPBK_EXTERNAL,   /* no loopback */
    SGMII_LPBK_MAC,            /* internal loopback at ppc etsec */
    SGMII_LPBK_PCS,            /* internal loopback at cavium pcs */
    SGMII_LPBK_QLM,            /* line loopback at cavium QLM */
    SGMII_SW_LPBK_INTERNAL,    /* internal loopback at marvell GE switch */
    SGMII_PHY_LPBK_INTERNAL,   /* internal loopback at marvell GE PHY */
};

/* define qsgmii mode */
enum
{
    SGMII_TO_QSGMII,
    QSGMII_TO_AUTO_DETECT,
};


typedef enum mrvl_88e1548x_test_mode_t_ {
    NORMAL_MODE,
    TRANSMIT_WAVEFORM_TEST,
    TRANSMIT_JITTER_TEST_MASTER_MODE,
    TRANSMIT_JITTER_TEST_SLAVE_MODE,
    TRANSMIT_DISTORTION_TEST,
} mrvl_88e1548x_test_mode_t;

typedef struct mrvl_88e1543_phy_regs_t_
{
    const char *pagename;
    uint32_t    pagenum;
    const reg_info_t *pageregs;
} mrvl_88e1543_phy_regs_t;


typedef struct mrvl_88e1543_macsec_regs_info_t_
{
    char         *name;
    ushort        offset;
} mrvl_88e1543_macsec_regs_info_t;

/*
 * dev_error_report message codes
 */
typedef enum {
    MRVL_88E1543_DEV_STATE = 0,
    MRVL_88E1543_ATTACH,
    MRVL_88E1543_DETACH,
    MRVL_88E1543_INIT,
    MRVL_88E1543_SHOW,
    MRVL_88E1543_DESTROY,
    MRVL_88E1543_REG_TEST,
    MRVL_88E1543_ALTER_REG,
    MRVL_88E1543_SET_LPBK,
    MRVL_88E1543_CLN_LPBK,
    MRVL_88E1543_POWER_UP,
    MRVL_88E1543_INTR_GEN,
    MRVL_88E1543_INTR_GEN_FIBER,
    MRVL_88E1543_INTR_DISABLE,
    MRVL_88E1543_INTR_CLR_FIBER,
    MRVL_88E1543_LOCKUP_FIX,
    MRVL_88E1543_PHONE_DETECT,   
    MRVL_88E1543_SET_TEST_MODE,
    MRVL_88E1543_LED_ON,
    MRVL_88E1543_LED_OFF,
    MRVL_88E1543_LINKUP_STAT
}mrvl_88e1543_report_code_t;
 

/*
 * device callin function - service provided and defined by the device
 */
typedef struct dev_88e1543_callin_fvt_ {
    int  (*register_test)(dev_object_t *);
    int  (*show_reg)(dev_object_t *, print_fn_t, uint);
    int  (*alter_reg)(dev_object_t *);
    int  (*display_reg)(dev_object_t *);
    int  (*set_loopback)(dev_object_t *, int, int, int);
    int  (*set_qsgmii_int_loopback)(dev_object_t *, int, int, int);
    int  (*lpbk_mode)(dev_object_t *, int, int);
    int  (*cleanup_loopback)(dev_object_t *, uint);
    int  (*power_up)(dev_object_t *, uint, uint);
    int  (*intr_gen)(dev_object_t *, uint);
    int  (*intr_gen_fiber)(dev_object_t *, uint);
    int  (*intr_disable)(dev_object_t *, uint);
    int  (*intr_clr_fiber)(dev_object_t *, uint);
    int  (*lockup_fix)(dev_object_t *, uint);
    int  (*phone_detect)(dev_object_t *, uint);
    int  (*set_test_mode)(dev_object_t *, uint);
    int  (*set_1548x_test_mode) (dev_object_t*, int, print_fn_t) ;
    int  (*clr_macsec_cnt)(dev_object_t *, int, int);
    int  (*set_macsec)(dev_object_t *, int, boolean);
    int  (*init_macsec)(dev_object_t *, int, int);    
    int  (*check_status)(dev_object_t *, int, int, print_fn_t);
    void (*dump_statistic)(dev_object_t *, int, int, print_fn_t);
    int  (*gephy_set_led_on)(dev_object_t *, uint);
    int  (*gephy_set_led_off)(dev_object_t *, uint);
    int  (*gephy_set_led_default)(dev_object_t *, uint);
    int  (*gephy_check_if_plugged_with_cable)(dev_object_t *, uint);
    uint32_t  (*phy_88e1548l_init)(dev_object_t *); 
    
} dev_88e1543_callin_fvt_t;

/*
 * device callout function - service needed by the device
 *                           and defined by platform
 */
typedef struct dev_88e1543_callout_fvt_ {
    int (*smi_read)(uint, uint, uint, uint *);
    int (*smi_write)(uint, uint, uint, uint);
    int (*get_linkup_status)(uint, uint *);
} dev_88e1543_callout_fvt_t;

/*
 * Define the Marvell GE device object structure.
 */
typedef struct dev_88e1543_object_t_ {
    dev_object_t              base;
    dev_88e1543_callout_fvt_t *callout_fvt;
    dev_88e1543_callin_fvt_t  *callin_fvt;
    uint32_t                  addr_seq;
    uint32_t                  start_phy_addr;
} dev_88e1543_object_t;

/* Prototypes */
extern void dev_88e1543_create (dev_object_t *, dev_error_report_t);

extern uint all_port_link_status[];

#endif /* __DEV_PHY_88E1543_H__ */

/******** History ******** 
$Log: dev_88e1543.h,v $
Revision 1.6  2020/09/30 09:46:09  alicehua
CSCvv85097: Marvell 88e1543 Register test failed when port is plugged with cable

Revision 1.5  2019/12/11 10:10:21  lucywang
Merged Nanook to main trunk

Revision 1.4  2019/07/11 12:34:41  alicehua
Collapse Nutella codes into main trunk

$Endlog$
*/
