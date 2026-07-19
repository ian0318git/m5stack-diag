/* $Id: dev_88e151x.h,v 1.4 2019/10/17 02:16:14 kehuang2 Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/chips/dev_88e151x_marvell/dev_88e151x.h,v $
 * Filename:    dev_88e151x.h
 *
 * Description:    Marvell 88E151x Device Driver Header File
 *
 * Copyright (c) 2018-2019 by cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#ifndef __DEV_88E151X_H__
#define __DEV_88E151X_H__

#include "types.h"
#include "common_utils.h"
#include "dev_object.h"

/* Common defines */
#define MRV88E151X_ERR_BUF_SIZE                 (80)

#define MRV88E151X_PHY_ID_1_VALUE               (0x0141)

#define MRV88E151X_RESET_CLR_TOUT               (300) /* 3 secs */
#define MRV88E151X_LINK_UP_TOUT                 (300) /* 3 secs */
#define MRV88E151X_LPBK_SET_TOUT                (300) /* 3 secs */

/* PHY (88E series) Registers Offsets */

#define MRV88E151X_PHYID_1_REG                   2
#define MRV88E151X_PHYID_2_REG                   3 
#define MRV88E151X_PAGE_ADDRESS_REG              22

/* Page 0 Register Offsets - Copper */

#define MRV88E151XN_CONTROL_REG                  0  
#define MRV88E151XN_STATUS_REG                   1  
#define MRV88E151XN_AUTONEG_ADVR_REG             4  
#define MRV88E151XN_LINK_PART_AV_REG             5  
#define MRV88E151XN_AUTONEG_EXPANSION_REG        6  
#define MRV88E151XN_NEXT_PAGE_REG                7  
#define MRV88E151XN_LP_NEXT_PAGE_REG             8  
#define MRV88E151XN_1000B_CNTL_REG               9  
#define MRV88E151XN_1000B_STATUS_REG             10 
#define MRV88E151XN_XMDIO_MMD_CNTL_REG           13 
#define MRV88E151XN_XMDIO_MMD_ADRS_DATA_REG      14 
#define MRV88E151XN_EXTENDED_STATUS_REG          15 
#define MRV88E151XN_SPECIFIC_CONTROL1_REG        16 
#define MRV88E151XN_SPECIFIC_STATUS1_REG         17 
#define MRV88E151XN_INT_ENABLE_REG               18 
#define MRV88E151XN_SPECIFIC_INT_STATUS_REG      19 
#define MRV88E151XN_SPECIFIC_CONTROL2_REG        20 
#define MRV88E151XN_REC_ERROR_COUNTER_REG        21 
#define MRV88E151XN_GL_INT_STATUS_REG            23 
#define MRV88E151XN_SPECIFIC_CONTROL3_REG        26 

/* Page 1 Register Offsets - Fiber */

#define MRV88E151XF_CONTROL_REG                  0     
#define MRV88E151XF_STATUS_REG                   1  
#define MRV88E151XF_PHY_ID_1_REG                 2  
#define MRV88E151XF_PHY_ID_2_REG                 3  
#define MRV88E151XF_AUTONEG_ADVR_REG             4  
#define MRV88E151XF_LINK_PART_AV_REG             5  
#define MRV88E151XF_AUTONEG_EXPANSION_REG        6  
#define MRV88E151XF_NEXT_PAGE_REG                7  
#define MRV88E151XF_LP_NEXT_PAGE_REG             8  
#define MRV88E151XF_EXTENDED_STATUS_REG          15  
#define MRV88E151XF_SPECIFIC_CONTROL1_REG        16 
#define MRV88E151XF_SPECIFIC_STATUS_REG          17 
#define MRV88E151XF_INT_ENABLE_REG               18 
#define MRV88E151XF_INT_STATUS_REG               19 
#define MRV88E151XF_REC_ERROR_COUNTER_REG        21 
#define MRV88E151XF_PRBS_CONTROL_REG             23 
#define MRV88E151XF_PRBS_ERR_COUNTER_LSB_REG     24 
#define MRV88E151XF_PRBS_ERR_COUNTER_MSB_REG     25 
#define MRV88E151XF_SPECIFIC_CONTROL2_REG        26 

/* Page 2 Register Offsets - MAC */
#define MRV88E151XM_SPECIFIC_CONTROL1_REG        16  
#define MRV88E151XM_INT_ENABLE_REG               18
#define MRV88E151XM_SPECIFIC_STATUS_REG          19
#define MRV88E151XM_RX_ER_BYTE_CAP_REG           20
#define MRV88E151XM_SPECIFIC_CONTROL2_REG        21
#define MRV88E151XM_OP_IMP_CAL_OVR_REG           24 
#define MRV88E151XM_OP_IMP_TARGET_REG            25

/* Page 3 Register Offsets - LOS, INIT, STATUS[1:0] */
#define MRV88E151XL_FUNC_CONTROL_REG             16
#define MRV88E151XL_POL_CONTROL_REG              17
#define MRV88E151XL_TMR_CONTROL_REG              18
#define MRV88E151XL_PTP_LED_FN_CONTROL_REG       19
#define MRV88E151XL_DEFAULT_FUNC_CONTROL         0x101e
#define MRV88E151XL_FORCE_ON_LED                 0x999
#define MRV88E151XL_FORCE_ON_LED_0               0x9
#define MRV88E151XL_FORCE_ON_LED_1               0x90
#define MRV88E151XL_LED_0                        0       
#define MRV88E151XL_LED_1                        1       

/* Page 4 Register Offsets */
#define MRV88E151XR_RGMII_RX_ER_BYTE_CAP_REG     20
#define MRV88E151XR_PAGE4_REG27                  27

/* Page 5 Register Offsets - VCT */
#define MRV88E151XN_VCT_TX_MDI0_RX_COUP_REG      16  
#define MRV88E151XN_VCT_TX_MDI1_RX_COUP_REG      17  
#define MRV88E151XN_VCT_TX_MDI2_RX_COUP_REG      18  
#define MRV88E151XN_VCT_TX_MDI3_RX_COUP_REG      19  
#define MRV88E151XN_VCT_PAIR_SKEW_REG            20
#define MRV88E151XN_VCT_PAIR_SWP_POL_REG         21
#define MRV88E151XN_VCT_CONTROL_REG              23 
#define MRV88E151XN_VCT_SAMPLE_PT_DIST_REG       24 
#define MRV88E151XN_VCT_CROSS_PAIR_POS_TH_REG    25 
#define MRV88E151XN_VCT_SP_IM_POS_TH_0_1_REG     26 
#define MRV88E151XN_VCT_SP_IM_POS_TH_2_3_REG     27 
#define MRV88E151XN_VCT_SP_IM_POS_TH_4_TX_PULSE  28 

/* Page 6 Register Offsets */
#define MRV88E151XN_PRT_PACKET_GEN_REG           16 
#define MRV88E151XN_PRT_CRC_CHKR_REG             17
#define MRV88E151XN_PRT_CHKR_CONTROL_REG         18 
#define MRV88E151XN_PRT_PACKET_GEN_REG2          19  
#define MRV88E151XN_GEN_CONTROL_REG              20  
#define MRV88E151XN_COLL_CNT_1_2_REG             23   
#define MRV88E151XN_COLL_CNT_3_4_REG             24 
#define MRV88E151XN_COLL_AD_LNK_DC_REG           25 
#define MRV88E151XN_MISC_TEST_REG                26 
#define MRV88E151XN_MISC2_TEST_REG               27   

/* Page 7 Register Offsets */
#define MRV88E151XN_DIAG_PAIR0_LEN_REG           16
#define MRV88E151XN_DIAG_PAIR1_LEN_REG           17
#define MRV88E151XN_DIAG_PAIR2_LEN_REG           18
#define MRV88E151XN_DIAG_PAIR3_LEN_REG           19
#define MRV88E151XN_DIAG_RESULT_REG              20  
#define MRV88E151XN_DIAG_CONTROL_REG             21
#define MRV88E151XN_VCT_CROSS_PAIR_NEG_TH_REG    25
#define MRV88E151XN_VCT_SP_IM_NEG_TH_0_1         26   
#define MRV88E151XN_VCT_SP_IM_NEG_TH_2_3         27     
#define MRV88E151XN_VCT_SP_IM_NEG_TH_4           28   

/* Page 8 Register Offsets */
#define MRV88E151XP_PTP_ARV_PRT_CONFG0_REG       0   
#define MRV88E151XP_PTP_ARV_PRT_CONFG1_REG       1   
#define MRV88E151XP_PTP_ARV_PRT_CONFG2_REG       2   
#define MRV88E151XP_PTP_ARV_PRT_CONFG3_REG       3   
#define MRV88E151XP_PTP_ARV0_TIME_STATUS_REG     8 
#define MRV88E151XP_PTP_ARV0_TIME_BYTE_1_0       9 
#define MRV88E151XP_PTP_ARV0_TIME_BYTE_3_2       10
#define MRV88E151XP_PTP_ARV0_SEQ_IDN_REG         11
#define MRV88E151XP_PTP_ARV1_TIME_STATUS_REG     12
#define MRV88E151XP_PTP_ARV1_TIME_BYTE_1_0       13
#define MRV88E151XP_PTP_ARV1_TIME_BYTE_3_2       14
#define MRV88E151XP_PTP_ARV1_SEQ_IDN_REG         15

/* Page 9 Register Offsets */
#define MRV88E151XP_PTP_DEP_TIME_STATUS_REG      0  
#define MRV88E151XP_PTP_DEP_TIME_BYTE_1_0        1
#define MRV88E151XP_PTP_DEP_TIME_BYTE_3_2        2
#define MRV88E151XP_PTP_DEP_SEQ_IDN_REG          3
#define MRV88E151XP_PTP_DISCARD_CNTR_REG         5

/* Page 12 Register Offsets */
#define MRV88E151XT_TAI_GL_CONFIG0_REG           0   
#define MRV88E151XT_TAI_GL_CONFIG1_REG           1 
#define MRV88E151XT_TAI_GL_CONFIG2_REG           2 
#define MRV88E151XT_TAI_GL_CONFIG3_REG           3 
#define MRV88E151XT_TAI_GL_CONFIG4_REG           4 
#define MRV88E151XT_TAI_GL_CONFIG5_REG           5 
#define MRV88E151XT_TAI_GL_CONFIG6_REG           6 
#define MRV88E151XT_TAI_GL_CONFIG7_REG           7 
#define MRV88E151XT_TAI_GL_CONFIG8_REG           8 
#define MRV88E151XT_TAI_GL_CONFIG9_REG           9 
#define MRV88E151XT_EVNT_CPTR_BYTE_1_0           10 
#define MRV88E151XT_EVNT_CPTR_BYTE_3_2           11
#define MRV88E151XT_TAI_GL_CONFIG12_REG          12
#define MRV88E151XT_TAI_GL_CONFIG13_REG          13
#define MRV88E151XT_PTP_GL_TIME_BYTE_1_0         14  
#define MRV88E151XT_PTP_GL_TIME_BYTE_3_2         15  
#define MRV88E151XT_PAGE12_REG16                 16

/* Page 14 Register Offsets */
#define MRV88E151XP_PTP_GL_CONFIG0_REG           0    
#define MRV88E151XP_PTP_GL_CONFIG1_REG           1    
#define MRV88E151XP_PTP_GL_CONFIG2_REG           2 
#define MRV88E151XP_PTP_GL_CONFIG3_REG           3  
#define MRV88E151XP_PTP_GL_STATUS_REG            8 
#define MRV88E151XP_RD_PLUS_CMND_REG             14
#define MRV88E151XP_RD_PLUS_DATA_REG             15

/* Page 17 Register Offsets */
#define MRV88E151XW_WOL_CONTROL_REG              16 
#define MRV88E151XW_WOL_STATUS_REG               17
#define MRV88E151XW_SRAM_PCKT_7_6_LEN_REG        18
#define MRV88E151XW_SRAM_PCKT_5_4_LEN_REG        19 
#define MRV88E151XW_SRAM_PCKT_3_2_LEN_REG        20
#define MRV88E151XW_SRAM_PCKT_1_0_LEN_REG        21
#define MRV88E151XW_MAGIC_PKT_DA_WRD2_REG        23
#define MRV88E151XW_MAGIC_PKT_DA_WRD1_REG        24
#define MRV88E151XW_MAGIC_PKT_DA_WRD0_REG        25   
#define MRV88E151XW_SRAM_BYTE_ADRS_CNTRL_REG     26
#define MRV88E151XW_SRAM_BYTE_DATA_CNTRL_REG     27
#define MRV88E151XW_SRAM_READ_CNTRL_REG          28

/* Page 18 Register Offsets */
#define MRV88E151XB_EEE_BUFFR_CNTRL1_REG         0  
#define MRV88E151XB_EEE_BUFFR_CNTRL2_REG         1  
#define MRV88E151XB_EEE_BUFFR_CNTRL3_REG         2  
#define MRV88E151XB_PACKET_GEN_REG               16 
#define MRV88E151XB_CRC_COUNTER_REG              17 
#define MRV88E151XB_CHCKR_CONTROL_REG            18 
#define MRV88E151XB_PACKET_GEN2_REG              19 
#define MRV88E151XB_GN_CONTROL1_REG              20 
#define MRV88E151XB_LNK_DISCONNECT_CNT_REG       25 
#define MRV88E151XB_SRDES_RX_ER_BYT_CAPTURE      26 

#define MRV88E151X_REG_PAGE_0            0 
#define MRV88E151X_REG_PAGE_1            1
#define MRV88E151X_REG_PAGE_2            2
#define MRV88E151X_REG_PAGE_3            3
#define MRV88E151X_REG_PAGE_4            4
#define MRV88E151X_REG_PAGE_5            5
#define MRV88E151X_REG_PAGE_6            6
#define MRV88E151X_REG_PAGE_7            7
#define MRV88E151X_REG_PAGE_8            8
#define MRV88E151X_REG_PAGE_9            9
#define MRV88E151X_REG_PAGE_12           12
#define MRV88E151X_REG_PAGE_14           14
#define MRV88E151X_REG_PAGE_17           17
#define MRV88E151X_REG_PAGE_18           18

#define MRV88E151X_REG_PAGE_251           251
#define MRV88E151X_REG_PAGE_255           255

#define MRV88E151X_SPD_10               (0x00)
#define MRV88E151X_SPD_100              (0x2000)
#define MRV88E151X_SPD_1000             (0x0040)
#define MRV88E151X_SPD_MASK             (MRV88E151X_SPD_100 | MRV88E151X_SPD_1000)
#define MRV88E151X_AN_ENABLED           (0x1000)
#define MRV88E151X_LPBK_ENABLED         (0x4000)
#define MRV88E151X_SW_RESET             (0x8000)

/* Specific Control 1 Register (offset 16) Bits defines */
#define PHY_SPCR1_DIS_LINK_PULSE        (0x8000)
#define PHY_SPCR1_FORCE_LINK            (0x0400)
#define PHY_SPCR1_EN_DETECT             (0x0100)
#define PHY_SPCR1_MDIX_MASK             (0x0060)
#define PHY_SPCR1_COP_TX_DIS            (0x0008)
#define PHY_SPCR1_POW_DOWN              (0x0004)
#define PHY_SPCR1_POL_REV_DIS           (0x0002)
#define PHY_SPCR1_DIS_JABBER            (0x0001)

/* Page 6 Register 18 Checker Control Bits defines */
#define PRT_CHKR_PKT_STUB_EN            (0x0008)

/* Page 0 Register 1 Copper Status Register */

/* Page 0 Register 17 Copper Specific Status Bits defines */
#define PHY_COP_SPSTS_DUPLEX            (0x2000)
#define PHY_COP_SPSTS_PAGE_RCVD         (0x1000)
#define PHY_COP_SPSTS_SPD_RSLVD         (0x0800)
#define PHY_COP_SPSTS_COP_LINK_UP       (0x0400)
#define PHY_COP_SPSTS_TPAUSE_EN         (0x0200)
#define PHY_COP_SPSTS_RPAUSE_EN         (0x0100)
#define PHY_COP_SPSTS_MDI_STS           (0x0040)
#define PHY_COP_SPSTS_COP_EN_STS        (0x0010)
#define PHY_COP_SPSTS_GLOB_LINK_UP      (0x0008)
#define PHY_COP_SPSTS_POL_REV           (0x0002)
#define PHY_COP_SPSTS_JABBER            (0x0001)

/* Page 3 Register 18 LED Timer Control Register */
#define PHY_TIMER_CNTRL_FORCE_INT       (0x8000)
#define PHY_TIMER_CNTRL_INTR_EN         (0x0080)


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

/* Page 18 Register 20 */
#define P20_R18_MODE_RGMII_COPPER 0x1
#define P20_R18_MODE_MASK         0x7
#define P20_R18_RESET             0x8000

#define P1_R26_AMP_MASK           0x3


typedef struct dev_mrvl_reg_info_t_ {
    ushort      page;		/* page number */
    reg_info_t	*reg_p;		/* Registers table */
} dev_mrvl_reg_info_t;


typedef enum {
    DEV_88E151X_DEV_STATE = 0,
    DEV_88E151X_ATTACH,
    DEV_88E151X_DETACH,
    DEV_88E151X_INIT,
    DEV_88E151X_SHOW,
    DEV_88E151X_DESTROY,
    DEV_88E151X_ALTER,
    DEV_88E151X_ALERT,
    DEV_88E151X_DISPLAY,
    DEV_88E151X_REG_TEST,
    DEV_88E151X_READ,
    DEV_88E151X_WRITE,
    DEV_88E151X_LPBK,
    DEV_88E151X_INT_TEST,
    DEV_88E151X_LED_ON,
    DEV_88E151X_LED_OFF,
    DEV_88E151X_SET_SGMII_AMP
} dev_88e151x_report_code_t;

typedef enum {
    DEV_88E151X_INT_LPBK = 0,
    DEV_88E151X_EXT_LPBK,
    DEV_88E151X_LINE_LPBK,
    DEV_88E151X_DIS_LPBK
} dev_88e151x_loopback_t;

typedef enum {
    DEV_88E151X_SPD_10,
    DEV_88E151X_SPD_100,
    DEV_88E151X_SPD_1000
} dev_88e151x_speed_t;


typedef struct {
    int  reg_page;  /* page of register */
    int  reg_off;   /* offset of register */
    uint16_t  val;  /* value to set */
    uint16_t  mask; /* mask of register r/w capability */
} mrvl_phy_setup_t;

typedef struct dev_88e151x_callin_fvt_t_ {
    int (*register_test)(dev_object_t *dev);            /* Register Test */
    int (*alter_register)(dev_object_t *dev);    /* Alter register */
    int (*display_register)(dev_object_t *dev);    /* Dump register */
    int (*dump_register)(dev_object_t *dev);    /* Dump register */
    int (*set_spd)(dev_object_t *dev, int);  /* Set Speed */
    int (*set_lpbk)(dev_object_t *dev, int, int);  /* Set loopback */
    int (*set_testmode)(dev_object_t *dev);        /* Set test mode */
    int (*set_eee)(dev_object_t *dev);  	/* Set EEE */
    int (*enable_force_interrupt)(dev_object_t *dev); /* Enable and force Interrupt */
    int (*disable_interrupt)(dev_object_t *dev);
    int (*gephy_set_led_on)(dev_object_t *dev);
    int (*gephy_set_led_single_on)(dev_object_t *dev);
    int (*gephy_set_led_off)(dev_object_t *dev);
    int (*init_phy)(dev_object_t *dev);
    int (*set_sgmii_output_amp)(dev_object_t *dev, int);
} dev_88e151x_callin_fvt_t;

/*
 * device callout function - service needed by the device and defined by
 *                 platform
 */
typedef struct dev_88e151x_callout_fvt_t_ {
    /*
     * Vectors set by the upper level (eg., platform).
     */
    uint32_t (*rd)(uint32, ushort*);
    uint32_t (*wr)(uint32, ushort);
} dev_88e151x_callout_fvt_t;

/*
 * Define the 88E151X device object structure
 */
typedef struct dev_88e151x_object_t {
    dev_object_t        base;
    dev_88e151x_callin_fvt_t        *callin_fvt;
    dev_88e151x_callout_fvt_t       *callout_fvt;
} dev_88e151x_object_t;

extern void mrv88e151x_dev_create(dev_object_t *, dev_error_report_t);

#endif
/*------------------------------------------------------------------
$Log: dev_88e151x.h,v $
Revision 1.4  2019/10/17 02:16:14  kehuang2
Collapse Tabei-L into main trunk

Revision 1.3  2018/11/09 07:33:21  yungchen
Merge viper branch4 to the main trunk (CSCvn11857)

Revision 1.2  2018/08/06 02:30:59  harrchan
Merge viper E2E to the main trunk (CSCvk28469)

Revision 1.1.2.6  2018/07/03 05:39:26  harrchan
Follow the coding rule to clean up code

Revision 1.1.2.5  2018/04/26 08:14:14  lucywang
Added utility to set 88E1514 EEE

Revision 1.1.2.4  2018/03/16 02:01:02  olin2
Support GE PHY testmode util

Revision 1.1.2.3  2018/03/15 08:31:00  harrchan
Add new function for led test

Revision 1.1.2.2  2018/02/27 09:10:32  harrchan
Initial viper application code base


$Endlog$
*/
