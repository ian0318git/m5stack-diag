/* $Id: diag_tlk10232_lib.h,v 1.2 2015/05/25 03:59:10 steja Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/skye_linux/include/diag_tlk10232_lib.h,v $
 *-----------------------------------------------------------------------------
 * diag_TLK10232_lib.h
 *
 * Ported from Woodlawn Project
 * May 2013, steja
 * Copyright (c) 2013 ~ 2015 by Cisco Systems, Inc.
 * All rights reserved.
 *-----------------------------------------------------------------------------
 */
 
#ifndef __DIAG_TLK10232_LIB_H__
#define __DIAG_TLK10232_LIB_H__

#define TLK_10232_REG_LEN           (2)
#define TLK_10232_REG_DEVICE_30     (0x1E)

                                        
/* TLK 10232 phy address = 0x10, select channel A by setting LSB of phy address = 0,
     channel B by setting LSB of phy address = 1 */
#define TLK_10232_PHY_ADDR_CHANNEL_A         (0x10)
#define TLK_10232_PHY_ADDR_CHANNEL_B         (0x11)

/* TLK 10232 Device Address */
#define TLK_10232_HS_CH_CTRL_1_DEV           (0x1E)
#define TLK_10232_CHANNEL_CTRL_1_DEV         (0x1E)
#define TLK_10232_HS_SERDES_CTRL_1_DEV       (0x1E)
#define TLK_10232_LS_SERDES_CTRL_1_DEV       (0x1E)
#define TLK_10232_AN_CTRL                    (0x07)
#define TLK_10232_LT_TRAIN_CTRL              (0x01)
#define TLK_10232_TI_RESERVED_CTRL           (0x1E)
#define TLK_10232_RESET_CTRL                 (0x1E)
#define TLK_10232_DSR_CTRL_1                 (0x1E)
#define TLK_10232_LPBK_TP_CTRL               (0x1E)
#define TLK_10232_PCS_CTRL                   (0x03)

/* TLK 10232 Register Address */
#define TLK_10232_HS_CH_CTRL_1_REG           (0x001D)
#define TLK_10232_DSR_CHANNEL_CTRL_1_REG     (0x0019)
#define TLK_10232_HS_SERDES_CTRL_1_REG       (0x0002)
#define TLK_10232_DST_CONTROL_1_REG          (0x0017)
#define TLK_10232_DST_CONTROL_2_REG          (0x0018)
#define TLK_10232_DSR_CONTROL_2_REG          (0x001A)
#define TLK_10232_GLOBAL_CTRL_REG            (0x0000)
#define TLK_10232_AN_CTRL_REG                (0x0000)
#define TLK_10232_LT_TRAIN_CTRL_REG          (0x0096)
#define TLK_10232_TI_RESERVED_CTRL_REG       (0x8021)
#define TLK_10232_RESET_CTRL_REG             (0x000E)
#define TLK_10232_DSR_CTRL_1_REG             (0x0019)
#define TLK_10232_LPBK_TP_REG                (0x000B)
#define TLK_10232_HS_SERDES_CTRL_2           (0x0003)
#define TLK_10232_HS_SERDES_CTRL_3           (0x0004)
#define TLK_10232_CLK_CTRL                   (0x000D)
#define TLK_10232_LS_SERDES_CTRL_1_REG       (0x0006)
#define TLK_10232_LS_SERDES_CTRL_2_REG       (0x0007)
#define TLK_10232_LS_SERDES_CTRL_3_REG       (0x0008)
#define TLK_10232_HS_SERDES_CTRL_4_REG       (0x0005)
#define TLK_10232_LS_STATUS_1_REG            (0x0015)
#define TLK_10232_LS_LN0_ERROR_COUNTER_REG   (0x0011)
#define TLK_10232_LS_LN1_ERROR_COUNTER_REG   (0x0012)
#define TLK_10232_LS_LN2_ERROR_COUNTER_REG   (0x0013)
#define TLK_10232_LS_LN3_ERROR_COUNTER_REG   (0x0014)
#define TLK_10232_LS_CONFIG_CONTROL_REG      (0x000C)
#define TLK_10232_CHANNEL_STATUS_1_REG       (0x000F)
#define TLK_10232_KR_FIFO_CTRL_1_REG         (0x8001)
#define TLK_10232_PMA_CTRL_1_REG             (0x0000)
#define TLK_10232_TI_RSVD_CTRL2              (0x9001)
#define TLK_10232_TI_RSVD_CTRL3              (0x9002)
#define TLK_10232_TI_RSVD_CTRL4              (0x9003)
#define TLK_10232_TI_RSVD_CTRL6              (0x9005)
#define TLK_10232_CH_CTRL_1_REG              (0x0001)
#define TLK_10232_PCS_CTRL_REG               (0x0000)
#define TLK_10232_KR_FEC_CTRL_REG            (0x00AB)
#define TLK_10232_TI_RSVD_1_CTRL_REG         (0x8100)
#define TLK_10232_TI_RSVD_2_CTRL_REG         (0x8101)


/* Direct set up value */
#define TLK_10232_HS_CH_CTRL_1_VAL              (0x2000)
#define TLK_10232_DSR_CHANNEL_CTRL_1_VAL        (0x0300)
#define TLK_10232_HS_SERDES_CTRL_1_VAL          (0x8317)
#define TLK_10232_GLOBAL_CTRL_VAL               (0x8610)
#define TLK_10232_GLOBAL_RESET                  (0x8000)
#define TLK_10232_GLOBAL_RESET_DEFAULT          (0x0610)
#define TLK_10232_AN_CTRL_VAL                   (0x2000)
#define TLK_10232_AN_DISABLE                    (0x1000)
#define TLK_10232_LT_TRAIN_CTRL_VAL             (0x0000)
#define TLK_10232_LINK_TRAIN_DISABLE            (0x0003)
#define TLK_10232_TI_RESERVED_CTRL_VAL          (0x003f)
#define TLK_10232_HS_AUTO_STATUS_CHECK_DISABLE  (0x0070)
#define TLK_10232_TI_RESERVED_CTRL_VAL_KX       (0x001f)
#define TLK_10232_PATH_RESET                    (0x0008)
#define TLK_10232_RESET_CTRL_VAL                (0x0000)
#define TLK_10232_XAUIB_TO_XAUIB_DSR_CTRL_1_VAL (0x2000)
#define TLK_10232_XAUIB_TO_XAUIA_DSR_CTRL_1_VAL (0x2a00)
#define TLK_10232_DST_CTRL_2_VAL                (0x4C20)
#define TLK_10232_LPBK_TP_VAL                   (0x0008)
#define TLK_10232_LPBK_TP_GE_HOST_LPBK_VAL      (0x0d18)
#define TLK_10232_HS_SERDES_CTRL_2_VAL_3_11     (0xa040)
#define TLK_10232_CLK_CTRL_DISABLE              (0x1000)
#define TLK_10232_CLK_CTRL_CLK_OUT_DISABLE      (0x3f80)
#define TLK_10232_LS_SERDES_CTRL_1_LANE0_PLL    (0x1001)
#define TLK_10232_LS_SERDES_CTRL_1_VAL          (0x1111)
#define TLK_10232_LS_SERDES_CTRL_3_INVERT_POLAR (0x8000)
#define TLK_10232_LS_SERDES_CTRL_3_VAL          (0x800D)
#define TLK_10232_LS_SERDES_CTRL_1_LANE1_RX     (0x2111)
#define TLK_10232_LS_SERDES_CTRL_1_LANE1_TX     (0x2111)
#define TLK_10232_LS_SERDES_CTRL_1_LANE1_RX_TX  (0x8111)
#define TLK_10232_LS_SERDES_CTRL_1_LANE2_TX     (0x4111)
#define TLK_10232_LS_SERDES_CTRL_1_LANE3_RX     (0x8111)
#define TLK_10232_LS_SERDES_CTRL_3_LANE1_TX     (0x400D)
#define TLK_10232_LS_SERDES_CTRL_3_LANE1_RX     (0x800D)
#define TLK_10232_LS_SERDES_CTRL_3_LANE2_TX     (0x400D)
#define TLK_10232_LS_SERDES_CTRL_3_LANE3_RX     (0x800D)
#define TLK_10232_LS_SERDES_CTRL_3_LANE1_RX_TX  (0xC00D)
#define TLK_10232_LS_SERDES_CTRL_1_PLL          (0xF111)
#define TLK_10232_HS_CH_CTRL_1_REF_CLK          (0x1000)
#define TLK_SET_DATA_PATH_XAUI_A_TO_XAUI_B_CHA  (0xac20)
//#define TLK_SET_DATA_PATH_XAUI_A_TO_XAUI_B_CHB  (0x8c20)
#define TLK_SET_DATA_PATH_XAUI_A_TO_XAUI_B_CHB  (0xac20)
#define TLK_DISABLE_AUTO_NEGOTIATION            (0x2000)
#define TLK_10232_DSR_CTRL_LOOPBACK             (0x0c20)
#define TLK_10232_HS_SERDES_CTRL_4_LPBK         (0xc000)
#define TLK_10232_HS_SERDES_CTRL_4_VAL          (0xe000)
#define TLK_10232_DSR_DAT_SW_MODE_ANYDATA       (0x2000)
#define TLK_10232_DSR_ANY_CTRL_LBPK \
        (TLK_10232_DSR_DAT_SW_MODE_ANYDATA + TLK_10232_DSR_CTRL_LOOPBACK)
#define TLK_10232_CHK_LS_STATUS_LANE0           (0x0330)
#define TLK_10232_LANE1                         (0x1000)
#define TLK_10232_CHK_LS_STATUS_LANE1 \
        (TLK_10232_CHK_LS_STATUS_LANE0 + TLK_10232_LANE1)
#define TLK_10232_LANE2                         (0x2000)
#define TLK_10232_CHK_LS_STATUS_LANE2 \
        (TLK_10232_CHK_LS_STATUS_LANE0 + TLK_10232_LANE2)
#define TLK_10232_LANE3                         (0x3000)
#define TLK_10232_CHK_LS_STATUS_LANE3 \
        (TLK_10232_CHK_LS_STATUS_LANE0 + TLK_10232_LANE3)
#define TLK_10232_CTC_BYPASS_DISABLE            (0x0c00)
#define TLK_10232_KR_FIFO_CTRL                  (0xa020)
#define TLK_10232_LS_LOS_DISABLE                (0xD800)
#define TLK_10232_DEEP_LBPK                     (0x0002)
#define TLK_10232_LBPK_TP_CTRL                  (0x0d10)
#define TLK_10232_LBPK_TP_CTRL_DEEP_LBPK  \
        (TLK_10232_LBPK_TP_CTRL + TLK_10232_DEEP_LBPK)
#define TLK_10232_SHALLOW_LOCAL_LBPK            (0x0001)
#define TLK_10232_LBPK_TP_CTRL_SHALLOW_LBPK \
        (TLK_10232_LBPK_TP_CTRL + TLK_10232_SHALLOW_LOCAL_LBPK)
#define TLK_10232_PMA_CTRL_1_LBPK              (0x0001)
#define TLK_10232_HS_TWPOST2_RX_TX             (0xc005)
#define TLK_10232_RSVD_CTRL_VAL                (0x0001)
#define TLK_10232_PWR_DOWN_CHA                 (0x8024)
#define TLK_10232_CH_CTRL_1_VAL                (0x0B00)
#define TLK_10232_CH_CTRL_1_PWR_DOWN_CHA       \
        (TLK_10232_PWR_DOWN_CHA + TLK_10232_CH_CTRL_1_VAL)
#define TLK_10232_DISABLE_PMA                  (0x0800)
#define TLK_10232_PCS_CTRL_EN_PCS              (0x0800)
#define TLK_10232_PLL_MULTI_HS_LOOP            (0x811c)
#define TLK_10232_ADAPTIVE_GAIN_AUTO_ZERO      (0xe888)
#define TLK_10232_SET_HS_SERDES_CTRL_3         (0x5252)
#define TLK_10232_EN_FEC                       (0x0001)
#define TLK_10232_DIS_LINK_TRAIN               (0x0000)
#define TLK_10232_DIS_LINK_TRAIN_2             (0x0000)
#define TLK_10232_DIS_EN_LOAD_TX_DEFAULT       (0x0004)
#define TLK_10232_EN_LOAD_TX_DEF_3             (0x0000)
#define TLK_10232_SET_PRBS                     (0x1c00)
#define TLK_10232_EN_LINK_TRAIN                (0x0003)
#define TLK_10232_LT_TRAIN_CTRL_DEFAULT        (0x0002)

/* mask value */
#define TLK_10232_DSR_CONTROL_2_MASK_VAL_1      (0xC000)
#define TLK_10232_DSR_CONTROL_2_MASK_VAL_2      (0x4000)
#define TLK_10232_DSR_CONTROL_3_MASK_VAL_3_CHA  (0x5fff)
//#define TLK_10232_DSR_CONTROL_3_MASK_VAL_3_CHB  (0x7fff)
#define TLK_10232_DSR_CONTROL_3_MASK_VAL_3_CHB  (0x5fff)
#define TLK_10232_HS_SERDES_CTRL_2_MASK         (0xFFFF)
#define TLK_10232_LS_SERDES_CTRL_1_MASK         (0xF005)
#define TLK_10232_LS_SERDES_CTRL_1_PLL_MASK     (0xFFFF)
#define TLK_10232_LS_SERDES_CTRL_3_PLL_MASK     (0xFFFF)
#define TLK_HS_CH_CTRL1_MASK                    (0xFFFF)
#define TLK_10232_DSR_CTRL_LOOPBACK_MASK        (0xF000)
#define TLK_10232_KR_FIFO_CTRL1_MASK            (0x8000)
#define TLK_10232_LS_SERDES_CTRL_2_MASK         (0xFBFB)
#define TLK_10232_HS_SERDES_CTRL_4_MASK         (0x2000)
#define TLK_10232_PLL_MULTI_HS_LOOP_MASK        (0x0201)
#define TLK_10232_ADAPTIVE_GAIN_AUTO_ZERO_MASK  (0x0040)
#define TLK_10232_EQPRE_CDRFMULT_CDRTHR_PK_MASK (0x1500)
#define TLK_10232_KR_FEC_CTRL_MASK              (0x0000)
#define TLK_10232_TI_RSD_CTRL_3_MASK            (0x1335)
#define TLK_10232_TI_RSD_CTRL_4_MASK            (0x5E29)
#define TLK_10232_EN_LOAD_TX_DEFAULT_MASK       (0xFFFF)


/* default value */
#define DSR_CONTROL_2_REG_DEFAULT_VAL    (0x4C20)

enum tlk10232 {
  XAUIB_TO_10GKR = 1,
  XAUIB_TO_XAUIB,
  XAUIB_TO_XAUIA,
  XAUIB_TO_1GKX,
  SGMII_TO_1GKX,
};

#define PATH_RESET_TIME    (1000)
/* Declare Extern Functions */
extern int config_tlk_10232_mode(int);
extern int read_tlk_10232_reg(int, uint *);
extern int write_tlk_10232_reg(int, int );
extern int tlk10232_xaui_to_xaui_configuration(int);
extern int set_tlk10232_lpbk_bit(void);
extern int tlk10232_mode_select(void);
extern int tlk10232_global_reset(void);
extern int skye_tlk_reg_rd(int, int, int, int);
extern int skye_tlk_reg_wr(int, int, int, int, int);
int tlk10232_path_reset(void);
#endif


/*
 * $Log: diag_tlk10232_lib.h,v $
 * Revision 1.2  2015/05/25 03:59:10  steja
 * Add Support Skye SM
 *
 * Revision 1.1.4.2  2015/04/29 11:36:25  steja
 * Code check-in to skye-branch2 for ER code review
 *
 *
 * ------------------------------------------------------------------
 * Revision 1.1.2.3  2015/02/12 12:41:57  steja
 * Code clean up
 *
 * Revision 1.1.2.2  2014/09/15 07:58:48  steja
 * Code Clean up
 *
 * Revision 1.1.2.1  2014/07/21 01:56:37  palin2
 * Initial check-in Skye module side Diag code.
 *
 * $Endlog$
 */
