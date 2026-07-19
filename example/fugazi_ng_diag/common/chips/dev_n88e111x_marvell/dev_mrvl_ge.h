/* $Id: dev_mrvl_ge.h,v 1.3 2021/09/24 01:22:45 harrchan Exp $ 
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/chips/dev_n88e111x_marvell/dev_mrvl_ge.h,v $
 *------------------------------------------------------------------
 * 
 * dev_mrvl_ge.h
 *
 * Copyright (c) 2018 - 2019 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#ifndef __DEV_MRVL_GE_H__
#define __DEV_MRVL_GE_H__

#include "dev_object.h"
#include "common_utils.h"

#define MRV88E1112_PHY_SEC_TO_MICROSEC     1000000.0
#define MRV88E1112_PHY_MAX_POLLING_TIME    1000000   /* 1sec */
#define MRV88E1112_PHY_COP_SPD_10MBPS      10
#define MRV88E1112_PHY_COP_SPD_100MBPS     100
#define MRV88E1112_PHY_COP_SPD_1000MBPS    1000
#define MRV88E1112_PHY_STUB_BUFFERING      4 /* 4 second */
#define MRV88E1112_PHY_INTR_BUFFERING      3 /* 3 second */

#define MRV88E1112_SOFT_RST_BUFFERING      500 /* 500ms */
#define MRV88E1112_SPD_CFG_BUFFERING       1   /* 1 second */
#define MRV88E1112_PWR_CFG_BUFFERING       100 /* 1ms */

#define INTR_POLLING_PERIOD        5 /* 5ms */
#define INTR_POLLING_ROUND         1000

#define MRV88E1112_PHY_ID_1_VALUE               0x0141
#define MRV88E1112_PHY_ID_2_VALUE               0x0C90
typedef uint16_t	smi_t;	/* SMI data type */

typedef union {
    pid_t	lock_pid;	/* Locked PID */
} smi_rt_if_t;

typedef struct smi_iface {
    smi_rt_if_t ret;		/* Return info struct */
    smi_t	*buf;		/* Read/write buffer pointer */
    uint8_t	offset;		/* SMI device register or memory offset */
    uint8_t	smi_dev;	/* MB_SMI_DEVICE device ID */
    uint8_t	smi_speed;	/* SMI bus speed used during init */
} smi_if_t;

extern int  getdec_answer(char *, uint, uint, uint);
extern int if_mvl_ge_rd_reg(dev_object_t *, smi_t, uchar, smi_t *);
extern int if_mvl_ge_wr_reg(dev_object_t *, smi_t, uchar, smi_t *);

typedef struct mrvl_ge_callout_fvt_ {
    uint32_t (*open)              (smi_if_t *smi_if_p);
    uint32_t (*close)             (smi_if_t *smi_if_p);
    uint32_t (*rd)                (smi_if_t *smi_if_p);
    uint32_t (*wr)                (smi_if_t *smi_if_p);
    int      (*dev_reset)         (boolean);
    int      (*sfp_setup)         (int);
    int      (*sfp_get_media_mode)(smi_t *);
    int      (*sfp_tx_enable)     (void);
    int      (*sfp_tx_disable)    (void);
    int      (*cpu_phy_mac_setup) (int);
    int      (*cpu_phy_mac_check_linkup) (void);
    int      (*cpu_phy_mac_autoneg_setup) (void);
    int      (*ge_phy_tx_rx_test) (void);
    int      (*chk_intr_assert)   (void);
    int      (*chk_intr_deassert) (void);
} mrvl_ge_callout_fvt_t;

typedef struct mrvl_ge_callin_fvt_ {
    int  (*ge_register_test)           (dev_object_t *); /* PHY register test */
    int  (*ge_phy_mac_lpbk_test)       (dev_object_t *); /* CPU MAC to PHY MAC loopback test */
    int  (*ge_ext_lpbk_test)           (dev_object_t *); /* External loopback test */
    int  (*ge_phy_sfp_ext_lpbk_test)   (dev_object_t *); /* SFP External loopback test */
    int  (*ge_phy_intr_test)           (dev_object_t *); /* Interrupt test */
    int  (*ge_phy_mac_lpbk_test_1gbps) (dev_object_t *); /* CPU MAC to PHY MAC loopback test (1GBPS)*/
    int  (*ge_ext_lpbk_test_1gbps)     (dev_object_t *); /* External loopback test (1GBPS)*/

    int  (*util_ge_rd_reg)           (dev_object_t *); /* Utility to read GE PHY register */
    int  (*util_ge_wr_reg)           (dev_object_t *); /* Utility to write GE PHY register */
    int  (*util_ge_set_test_mode)    (dev_object_t *); /* Utility to set test mode in 10000BASE-T Control Register(Page:0, Reg:9) */
    int  (*util_ge_set_tx_type)      (dev_object_t *); /* Utility to set Tx type in Copper Specific Control Register 2 (Page:0 Reg:26) */
    int  (*util_ge_set_vod)          (dev_object_t *); /* Utility to set VOD adjustments in Reg:29 & Reg:30 */

    int  (*if_ge_rd_reg)                (dev_object_t *, smi_t, uchar, smi_t *); /* interface to read GE PHY register */
    int  (*if_ge_wr_reg)                (dev_object_t *, smi_t, uchar, smi_t *); /* interface to write GE PHY register */
    int  (*if_ge_set_tx_type)           (dev_object_t *, int);                   /* interface to set GE PHY Tx type */
} mrvl_ge_callin_fvt_t;

/*
 * dev_error_report message codes
 */
typedef enum {
    MRVL_GE_DEV_STATE = 0,
    MRVL_GE_ATTACH,
    MRVL_GE_DETACH,
    MRVL_GE_INIT,
    MRVL_GE_SHOW,
    MRVL_GE_DESTROY,
    MRVL_GE_REG_TEST,
    MRVL_GE_RESET,
    MRVL_GE_READ,
    MRVL_GE_WRITE,
    MRVL_GE_ALTER,
    MRVL_GE_LPBK,
    MRVL_GE_PAGE,
    MRVL_GE_TEST_MODE,
}idt6v4900n_report_code_t;

/*
 * Marvell GE PHYs have multiple pages. This struct defines the registers
 * and the page associate with them.
 */
typedef struct dev_mrvl_reg_info_t_ {
    smi_t 	page;		/* page number */
    reg_info_t	*reg_p;		/* Registers table */
} dev_mrvl_reg_info_t;

/*
 * PHY registers changed for loopback.
 */
typedef struct dev_mrvl_lpbk_regs_t_ {
    smi_t page;			/* Page number */
    smi_t sp_ctl1;		/* Specific Control Register 1 */
    smi_t control;		/* Control Register */
    smi_t mac_ctrl;		/* MAC Control Register */
    smi_t mac_sp_ctl1;		/* MAC Specific Control Register 1 */
    smi_t pkt_gen;		/* Packet Generation */
} dev_mrvl_lpbk_regs_t;

/*
 * Define the Marvell GE device object structure.
 */
typedef struct dev_mrvl_ge_object_t_ {
    dev_object_t	base;
    mrvl_ge_callout_fvt_t *callout_fvt;
    mrvl_ge_callin_fvt_t  *callin_fvt;
    smi_if_t		*smi_p;		/* SMI API interface struct */
    dev_mrvl_reg_info_t *reg_info_p;	/* Registers table */
    dev_mrvl_lpbk_regs_t regs;		/* Saved registers for loopback */
    uint8_t		type;		/* PHY device type */
}dev_mrvl_ge_object_t;

/* GE type */
typedef enum {
    MRVL_GE_PHY_1112 = 0,	/* GE PHY with Copper and Optic */
    MRVL_GE_PHY_1114,		/* GE PHY with Copper only */
    MRVL_GE_PHY_INVALID,	/* Invalid GE PHY */
} dev_mrvl_ge_type_t;

typedef enum {
    ETH_MODE_FE10 = 0,
    ETH_MODE_FE100,
    ETH_MODE_GE,
    ETH_MODE_INVALID,
} dev_mrvl_ge_mode_t;

typedef enum {
    LOOPBACK_NONE = 0,
    LOOPBACK_MAC,		/* internal loopback at CPU */
    LOOPBACK_PHY,		/* internal loopback at marvell ge phy */
    LOOPBACK_EXT,		/* RJ45 external loopback */
    LOOPBACK_SFP,		/* SFP external loopback */
    LOOPBACK_INVALID,		/* Invalid loopback */
} dev_mrvl_ge_loopback_t;

typedef enum {
    LPBKOP_SAVE = 0,
    LPBKOP_RESTORE,
    LPBKOP_SET,
} dev_mrvl_ge_loopback_op_t;

typedef enum {
    PHY_READ = 0,
    PHY_WRITE,
} dev_mrvl_ge_smi_op_t;

typedef enum {
    MRVL_COPPER_TYPE = 0,
    MRVL_FIBER_TYPE,
    MRVL_MAC_TYPE,
} dev_mrvl_page_type_t;

typedef enum {
    MRVL_PWR_OFF = 0,
    MRVL_PWR_ON,
} dev_mrvl_pwr_type_t;

typedef enum {
    MRVL_DISABLE = 0,
    MRVL_ENABLE,
} dev_mrvl_option_type_t;
/*
 * General Defines
 */
#define SW_LOG_BUF_SIZE                         384
#define INCREMENT_COUNT                         1
#define DECREMENT_COUNT                         -1
#define CONSTANT_COUNT                          0
#define ERR_BUF_SIZE				80
#define MAX_SMI_REGS				0x20
#define SMI_SHOW_MASK				0xF
#define PHY_RESET_DELAY			100	/* 100 milliseconds delay */
/* Refer to 88E1114 Rev C2 Release Notes. Section 4.8 -
 * If Auto=-Neg is turned off and the forced speed of 10 Mbps or 100 Mbps is
 * programmed by writing to Register 0, then there will be a 637 ms delay
 * before the PHY switches to the programmed speed. There is no delay if
 * switching to gigabit speed.
 * ... (Wait at least 700ms to give a little margin on top of the 637ms).
 * Diag will have the PHY_AUTO_NEG_DELAY regardless of the speed.
 */
#define PHY_AUTO_NEG_DELAY		1000	/* 700 milliseconds delay. But
						 * we extend it to 1 second */

#define SFP_ENABLE_WAIT			500	/* SFP MSA recommends 300ms */
#define GE_100FX_SETUP_TIME		10	/* Time to setup GE 100 FX */

/* Refer to "Time to link up and link down for 10/100/1000 PHYs" White Paper
 * from Marvell (December 20, 2004). -
 * The quick answer to the question is that there is no theoretical upper
 * bound, but typically link should come up within 3 seconds.
 */
#define LINK_MAX_TIME			3000	/* Link up in 3 seconds. */
#define LINK_DELAY			1	/* 1 millisecond delay */

/* 
 * 88E1112 Register Map:
 * PA: Page Any
 * P0 ~ P6: Page 0 ~ Page 6
 * R0 ~ R31: Register 2 ~ Register 31 
 */
/* Shared PHY (88E series) Registers Offsets */
#define MRV88E111N_PA_R2_PHYID_1_REG                    2
#define MRV88E111N_PA_R3_PHYID_2_REG                    3
#define MRV88E111N_PA_R22_PAGE_ADDRESS_REG             22
#define MRV88E111N_PA_R29_FACTORY_TEST_MODES0          29
#define MRV88E111N_PA_R30_FACTORY_TEST_MODES1          30
#define MRV88E111N_PA_R31_FACTORY_TEST_MODES2          31

#define MRV88E111N_PA_R30_VOD_MASK                 0x0007 /* Bit[2:0]=111 */             

/* Page 0 Register Offsets - Copper */
#define MRV88E111N_P0_R0_CONTROL_REG                    0
#define MRV88E111N_P0_R1_STATUS_REG                     1
#define MRV88E111N_P0_R4_AUTONEG_ADVR_REG               4
#define MRV88E111N_P0_R5_LINK_PART_AV_REG               5
#define MRV88E111N_P0_R6_AUTONEG_EXPANSION_REG          6
#define MRV88E111N_P0_R7_NEXT_PAGE_REG                  7
#define MRV88E111N_P0_R8_LP_NEXT_PAGE_REG               8
#define MRV88E111N_P0_R9_1000B_CNTL_REG                 9                              
#define MRV88E111N_P0_R10_1000B_STATUS_REG             10
#define MRV88E111N_P0_R15_EXTENDED_STATUS_REG          15
#define MRV88E111N_P0_R16_SPECIFIC_CONTROL1_REG        16
#define MRV88E111N_P0_R17_SPECIFIC_STATUS1_REG         17
#define MRV88E111N_P0_R18_INT_ENABLE_REG               18
#define MRV88E111N_P0_R19_SPECIFIC_STATUS2_REG         19
#define MRV88E111N_P0_R21_REC_ERROR_COUNTER_REG        21
#define MRV88E111N_P0_R26_SPECIFIC_CONTROL2_REG        26

/* Page 1 Register Offsets - Fiber */
#define MRV88E111F_P1_R0_CONTROL_REG                    0
#define MRV88E111F_P1_R1_STATUS_REG                     1
#define MRV88E111F_P1_R4_AUTONEG_ADVR_REG               4
#define MRV88E111F_P1_R5_LINK_PART_AV_REG               5
#define MRV88E111F_P1_R6_AUTONEG_EXPANSION_REG          6
#define MRV88E111F_P1_R7_NEXT_PAGE_REG                  7
#define MRV88E111F_P1_R8_LP_NEXT_PAGE_REG               8
#define MRV88E111F_P1_R15_EXTENDED_STATUS_REG          15
#define MRV88E111F_P1_R16_SPECIFIC_CONTROL1_REG        16
#define MRV88E111F_P1_R17_SPECIFIC_STATUS1_REG         17
#define MRV88E111F_P1_R18_INT_ENABLE_REG               18
#define MRV88E111F_P1_R19_SPECIFIC_STATUS2_REG         19
#define MRV88E111F_P1_R21_REC_ERROR_COUNTER_REG        21
#define MRV88E111F_P1_R26_SPECIFIC_CONTROL2_REG        26

/* Page 2 Register Offsets - MAC */
#define MRV88E111M_P2_R0_CONTROL_REG                    0
#define MRV88E111M_P2_R16_SPECIFIC_CONTROL1_REG        16
#define MRV88E111M_P2_R17_SPECIFIC_STATUS1_REG         17
#define MRV88E111M_P2_R18_INT_ENABLE_REG               18
#define MRV88E111M_P2_R19_SPECIFIC_STATUS2_REG         19
#define MRV88E111M_P2_R26_SPECIFIC_CONTROL2_REG        26

/* Page 3 Register Offsets - LOS, INIT, STATUS[1:0] */
#define MRV88E111L_P3_R16_FUNC_CONTROL_REG             16
#define MRV88E111L_P3_R17_POL_CONTROL_REG              17
#define MRV88E111L_P3_R18_TMR_CONTROL_REG              18

/* Page 4 Register Offsets - Non-Volatile Memory */
#define MRV88E111NV_P4_R16_ADDRESS                     16
#define MRV88E111NV_P4_R17_READ_DATA_STATUS            17
#define MRV88E111NV_P4_R18_WRITE_DATA_CONTROL          18
#define MRV88E111NV_P4_R19_RAM_WRITE_DATA_CONTROL      19
#define MRV88E111NV_P4_R20_RAM_ADDRESS                 20

/* Page 5 Register Offsets - VCT */
#define MRV88E111N_P5_R16_VCT_STATUS_MDI0_REG          16
#define MRV88E111N_P5_R17_VCT_STATUS_MDI1_REG          17
#define MRV88E111N_P5_R18_VCT_STATUS_MDI2_REG          18
#define MRV88E111N_P5_R19_VCT_STATUS_MDI3_REG          19
#define MRV88E111N_P5_R20_VCT_PAIR_SKEW_REG            20
#define MRV88E111N_P5_R21_VCT_PAIR_SWP_POL_REG         21
#define MRV88E111N_P5_R26_VCT_DSP_DISTANCE             26

/* Page 6 Register Offsets */
#define MRV88E111N_P6_R16_PACKET_GEN_REG               16
#define MRV88E111N_P6_R17_CRC_CHKR_REG                 17

#define MRV88E111N_CONTROL_PHY_RESET		0x8000 /* Bit[15]   = 1, 1 << 15 */
#define MRV88E111N_CONTROL_LOOPBACK		0x4000 /* Bit[14]   = 1, 1 << 14 */
#define MRV88E111N_CONTROL_FORCE_10_L		0x0000 /* Bit[6,13] = 00, 10Mbps   */
#define MRV88E111N_CONTROL_FORCE_100_L          0x2000 /* Bit[6,13] = 01, 100Mbps  */
#define MRV88E111N_CONTROL_FORCE_1000_L		0x0040 /* Bit[6,13] = 10, 1000Mbps */
#define MRV88E111N_CONTROL_AUTONEG_ENABLE	0x1000 /* Bit[12]   = 1, 1 << 12 */
#define MRV88E111N_CONTROL_POWER_DOWN		0x0800 /* Bit[11]   = 1, 1 << 11 */
#define MRV88E111N_CONTROL_ISOLATE		0x0400 /* Bit[10]   = 1, 1 << 10 */
#define MRV88E111N_CONTROL_RESTART_AUTONEG	0x0200 /* Bit[9]    = 1, 1 << 9 */
#define MRV88E111N_CONTROL_FULL_DUPLEX		0x0100 /* Bit[8]    = 1, 1 << 8 */
#define MRV88E111N_CONTROL_COLLISION_TEST	0x0080 /* Bit[7]    = 1, 1 << 7 */
#define MRV88E111N_CONTROL_SPEED_MASK		0x2040 /* mask of Bit[6,13]        */

/* Page 0 Register 0, Field offsets */
#define MRV88E111M_P0_R0_B8_FULL_DUPLEX               (uint16_t)0x1 << 8

/* Page 0,1,2 Register 0, Field offsets */
#define MRV88E111M_PX_R0_B15_RESET                    (uint16_t)0x1 << 15
#define MRV88E111M_PX_R0_B15_RESET_DONE                0x7fff             /* Bit[15]=0 */
#define MRV88E111N_PX_R0_B14_LPBK_ENA                 (uint16_t)0x1 << 14
#define MRV88E111N_PX_R0_B14_LPBK_ENA_MASK             0xbfff             /* Bit[14]=0 */
#define MRV88E111M_PX_R0_B13_MEDIA_SPD_10Mbps_LSB     (uint16_t)0x0 << 13
#define MRV88E111M_PX_R0_B13_MEDIA_SPD_100Mbps_LSB    (uint16_t)0x1 << 13
#define MRV88E111M_PX_R0_B13_MEDIA_SPD_1000Mbps_LSB   (uint16_t)0x0 << 13
#define MRV88E111M_PX_R0_B12_AUTO_NEGO_ENA            (uint16_t)0x1 << 12
#define MRV88E111M_PX_R0_B12_AUTO_NEGO_DIS             0xefff             /* Bit[12]=0 */
#define MRV88E111M_PX_R0_B6_MEDIA_SPD_10Mbps_MSB      (uint16_t)0x0 << 6
#define MRV88E111M_PX_R0_B6_MEDIA_SPD_100Mbps_MSB     (uint16_t)0x0 << 6
#define MRV88E111M_PX_R0_B6_MEDIA_SPD_1000Mbps_MSB    (uint16_t)0x1 << 6
#define MRV88E111M_PX_R0_B6_B13_MEDIA_SPD_MASK         0xdfbf;            /* Bit[6,13]=00 */


/* Page 2 Register 16, Field offsets */
#define MRV88E111M_P2_R16_B9_B7_MODE_100BASE_FX        (uint16_t)0x0 << 7
#define MRV88E111M_P2_R16_B9_B7_MODE_COP_GBIC          (uint16_t)0x1 << 7
#define MRV88E111M_P2_R16_B9_B7_MODE_AUTO_COP_SGMII    (uint16_t)0x2 << 7
#define MRV88E111M_P2_R16_B9_B7_MODE_AUTO_COP_100BASEX (uint16_t)0x3 << 7
#define MRV88E111M_P2_R16_B9_B7_MODE_RSV               (uint16_t)0x4 << 7
#define MRV88E111M_P2_R16_B9_B7_MODE_COP_ONLY          (uint16_t)0x5 << 7
#define MRV88E111M_P2_R16_B9_B7_MODE_SGMII_ONLY        (uint16_t)0x6 << 7
#define MRV88E111M_P2_R16_B9_B7_MODE_1000BASEX_ONLY    (uint16_t)0x7 << 7

/* Page 0,1,2 Register 17, Field offsets */
#define MRV88E111M_PX_R17_B10_LINK_MASK                (uint16_t)~(0x1 << 10) /* Link State */
#define MRV88E111M_PX_R17_B10_LINKUP                   (uint16_t)0x1 << 10    /* Link Up */
#define MRV88E111M_PX_R0_B11_PWR_CTRL_MASK             (uint16_t)~(0x1 << 11) /* Power Control mask */
#define MRV88E111M_PX_R0_B11_PWR_ON                    (uint16_t)~(0x1 << 11)    /* power on */
#define MRV88E111M_PX_R0_B11_PWR_OFF                   (uint16_t)0x1 << 11       /* power off */

/* Page 0 Register 16, Field offsets */
#define MRV88E111M_P0_R16_FORCE_LINK_GOOD_DIS           0xfbff                /* Bit[10]=0 */ 

/* Page 0 Register 9, Field offsets */
#define MRV88E111M_P0_R9_TEST_MODE_MASK                 0xe000 /* Bit[15:13]=111 */
#define MRV88E111M_P0_R9_MODE_SHIFT                     13
#define MRV88E111M_P0_R9_NORMAL_MODE                    0x0000 /* Bit[15:13]=000, Normal Mode */
#define MRV88E111M_P0_R9_TEST_MODE1                     0x2000 /* Bit[15:13]=001, Test Mode 1, Transmit Waveform Test */
#define MRV88E111M_P0_R9_TEST_MODE2                     0x4000 /* Bit[15:13]=010, Test Mode 2, Transmit Jitter Test (Master Mode) */
#define MRV88E111M_P0_R9_TEST_MODE3                     0x6000 /* Bit[15:13]=011, Test Mode 3, Transmit Jitter Test (Slave Mode) */
#define MRV88E111M_P0_R9_TEST_MODE4                     0x8000 /* Bit[15:13]=100, Test Mode 4, Transmit Distortion Test */

/* Page 0 Register 26, Field offsets */
#define MRV88E111M_P0_R26_TX_TYPE_MASK                  0x8000 /* Bit[15]=1 */
#define MRV88E111M_P0_R26_TX_TYPE_SHIFT                 15
#define MRV88E111M_P0_R26_TX_TYPE_B                     0x0000 /* Bit[15]=0 */
#define MRV88E111M_P0_R26_TX_TYPE_A                     0x8000 /* Bit[15]=1 */

/* Page 6 Register 16, Field offsets */
#define MRV88E111M_P6_R16_B5_STUB_ENA                  (uint16_t)0x1 << 5     /* enable stub test */
#define MRV88E111M_P6_R16_B5_STUB_DIS                  (uint16_t)~(0x1 << 5)  /* disable stub test */

#define MRV88E111N_PHY_ID_1_VALUE		0x0141

#define MRV88E111N_STATUS_AUTONEG_CHK		0x0008
#define MRV88E111N_STATUS_LINK_UP		0x0004

#define MRV88E111N_STATUS1_SPEED		0xC000
#define MRV88E111N_STATUS1_SPEED_1000		0x8000
#define MRV88E111N_STATUS1_SPEED_100		0x4000
#define MRV88E111N_STATUS1_SPEED_10		0x0000
#define MRV88E111N_STATUS1_LINK			0x0400
#define MRV88E111N_RESET_TIMEOUT		15	/* TRESET 10 ms min */

#define MRV88E111N_SPEED_1000BT			1000
#define MRV88E111N_SPEED_100BT			100
#define MRV88E111N_SPEED_10BT			10

#define MRV88E111N_REG_PAGE_0			0
#define MRV88E111N_REG_PAGE_1			1
#define MRV88E111N_REG_PAGE_2			2
#define MRV88E111N_REG_PAGE_3			3
#define MRV88E111N_REG_PAGE_4			4
#define MRV88E111N_REG_PAGE_5			5
#define MRV88E111N_REG_PAGE_6			6

/* Switch (98DX series) Registers Offsets */
#define MRV98DX_STATUS		0x1F	/* SMI Read-Write Status Register */
#define MRV98DX_WR_ADDR_MSB	0x00	/* SMI Write Address MSBs Register */
#define MRV98DX_WR_ADDR_LSB	0x01	/* SMI Write Address LSBs Register */
#define MRV98DX_WR_DATA_MSB	0x02	/* SMI Write Data MSBs Register */
#define MRV98DX_WR_DATA_LSB	0x03	/* SMI Write Data LSBs Register */
#define MRV98DX_RD_ADDR_MSB	0x04	/* SMI Read Address MSBs Register */
#define MRV98DX_RD_ADDR_LSB	0x05	/* SMI Read Address LSBs Register */
#define MRV98DX_RD_DATA_MSB	0x06	/* SMI Read Data MSBs Register */
#define MRV98DX_RD_DATA_LSB	0x07	/* SMI Read Data LSBs Register */

/* Bits defines for SMI Read-Write Status Register */
#define MRV98DX_SMI_WR_DONE	0x0002	/* SMI Write Done */
#define MRV98DX_SMI_RD_RDY	0x0001	/* SMI Read Ready */

/* Control Register (offset 0) Bits defines */
#define PHY_CTRL_RESET		MRV88E111N_CONTROL_PHY_RESET
#define PHY_CTRL_LOOPBACK	MRV88E111N_CONTROL_LOOPBACK
#define PHY_CTRL_MASK		MRV88E111N_CONTROL_SPEED_MASK
#define PHY_CTRL_1000		MRV88E111N_CONTROL_FORCE_1000_L
#define PHY_CTRL_100		MRV88E111N_CONTROL_FORCE_100_L
#define PHY_CTRL_10		MRV88E111N_CONTROL_FORCE_10_L
#define PHY_CTRL_DUPLEX		MRV88E111N_CONTROL_FULL_DUPLEX
#define PHY_CTRL_AUTO_NEG	MRV88E111N_CONTROL_AUTONEG_ENABLE
#define PHY_CTRL_PWR_DWN	MRV88E111N_CONTROL_POWER_DOWN

/* Link Partner Ability Register (offset 5 - Page 1) Bits defines */
#define LPA_SGMII_LINK		0x8000	/* SGMII Link up */

/* 1000BASE-T Control Register (offset 9) Bits defines */
#define PHY_GT_CTL_TEST_MASK	0xE000	/* Test Modes */
#define PHY_GT_CTL_NORMAL	0x0000	/* Normal mode */
#define PHY_GT_CTL_WV_TEST	0x2000	/* Test mode 1 - Waveform test */
#define PHY_GT_CTL_JT_MS	0x4000	/* Test mode 2 - Jitter (Master) */
#define PHY_GT_CTL_JT_SL	0x6000	/* Test mode 3 - Jitter (Slave) */
#define PHY_GT_CTL_DS_TEST	0x8000	/* Test mode 4 - Distortion test */
#define PHY_GT_CTL_TEST_MAX	PHY_GT_CTL_DS_TEST
#define PHY_GT_CTL_TEST_SHIFT	13	/* Test mode bits shift counts */

/* Specific Control 1 Register (offset 16) Bits defines */
#define PHY_SP_CTL1_SIGLOS	0x0200	/* SIGDET polarity for SFP */
#define PHY_SP_CTL1_ENERGY_DET	0x0180	/* Energy detect mask */
#define PHY_SP_CTL1_MDIX_MASK	0x0060	/* MDI Crossover mask */

#define MAC_SP_CTL1_MODE_MASK	0x0380	/* Mode select mask */
#define MAC_SP_CTL1_FX_MODE	0x0000	/* 100Base-FX */
#define MAC_SP_CTL1_GBIC_MODE	0x0080	/* Copper GBIC */
#define MAC_SP_CTL1_A_SGM_MODE	0x0100	/* Auto Copper/SGMII media interface */
#define MAC_SP_CTL1_A_X_MODE	0x0180	/* Auto Copper/1000Base-X */
#define MAC_SP_CTL1_CU_MODE	0x0280	/* Copper only */
#define MAC_SP_CTL1_SGM_MODE	0x0300	/* SGMII media interface only */
#define MAC_SP_CTL1_X_MODE	0x0380	/* 1000Base-X only */

/* Specific Status 1 Register (offset 17) Bits defines */
#define PHY_SP_STA1_LINKUP	MRV88E111N_STATUS1_LINK

/* Packet Generation Register (offset 16 - Page 6) Bits defines */
#define PHY_PKT_STUB_EN		0x0020	/* Enable stub test */
#define PHY_PKT_CRC_EN		0x0010	/* Enable CRC checker */

/* Copper Specific Status Register 2 (offset 19 - Page 0) Bits defines */
#define PG0_SP_ST2_AN_ERR       0x8000  /* Copper Auto-Negotiation Error */
#define PG0_SP_ST2_SP_CHG	0x4000	/* Copper Speed Changed */
#define PG0_SP_ST2_DPX_CHG	0x2000	/* Copper Duplex Changed */
#define PG0_SP_ST2_PG_RX	0x1000	/* Copper Page Received */
#define PG0_SP_ST2_AN_CMP	0x0800	/* Copper Auto-Negotiation Completed */
#define PG0_SP_ST2_LNK_CHG	0x0400	/* Copper Link Status Changed */
#define PG0_SP_ST2_SYM_ERR	0x0200	/* Copper Symbol Error */
#define PG0_SP_ST2_FLS_CRR	0x0100	/* Copper False Carrier */
#define PG0_SP_ST2_MDI_CHG	0x0040	/* MDI Crossover Changed */
#define PG0_SP_ST2_DNS_INT	0x0020	/* Downshift Interrupt */
#define PG0_SP_ST2_ED_CHG	0x0010	/* Energy Detect Chagned */
#define PG0_SP_ST2_DTE_CHG	0x0004	/* DTE power detection status changed */
#define PG0_SP_ST2_POL_CHG	0x0002	/* Polarity Changed */
#define PG0_SP_ST2_JAB		0x0001	/* Jabber */
#define PG0_SP_ST2_ERR_MSK	(PG0_SP_ST2_AN_ERR | PG0_SP_ST2_SYM_ERR | \
				 PG0_SP_ST2_FLS_CRR | PG0_SP_ST2_JAB)

/* Copper Specific Control Register 2 (offset 26 - Page 0) Bits defines */
#define PG0_SP_CTL2_TX_MASK	0x8000	/* Transmitter type mask */
#define PG0_SP_CTL2_TX_A	0x8000	/* Class A */
#define PG0_SP_CTL2_TX_B	0x0000	/* Class B */

/* MAC Specific Control Register 2 (offset 26 - Page 2) Bits defines */
/* SGMII MAC Interface Output Amplitude defines */
#define MAC_SP_CTL2_AMP_P5	0x0000	/* 0.50V */
#define MAC_SP_CTL2_AMP_P6	0x0001	/* 0.60V */
#define MAC_SP_CTL2_AMP_P7	0x0002	/* 0.70V */
#define MAC_SP_CTL2_AMP_P8	0x0003	/* 0.80V */
#define MAC_SP_CTL2_AMP_P9	0x0004	/* 0.90V */
#define MAC_SP_CTL2_AMP_1	0x0005	/* 1.00V */
#define MAC_SP_CTL2_AMP_1P1	0x0006	/* 1.10V */
#define MAC_SP_CTL2_AMP_1P2	0x0007	/* 1.20V */

#define ENABLE_INTR  0xe1e
#define DISABLE_INTR 0x21e

#define NULL_REG_TABLE NULL

#define MRV88E111M_TX_TYPE_B 0  /* CLASS B */
#define MRV88E111M_TX_TYPE_A 1  /* CLASS A */

#define COMPARE_AND     0 /* read data & pattern == 0       */
#define COMPARE_EQL     1 /* read data           == pattern */
#define COMPARE_AND_EQL 2 /* read data & pattern == pattern */
#define MAX_POLLING_ROUND          2000
#define POLLING_PERIOD             5 /* 5ms */

/* Prototypes */
extern int mrvl_n88e111x_dev_create (dev_object_t *dev,
				dev_error_report_t error_report_fn);

#endif /* __DEV_MRVL_GE_H__ */

/*-------------------------------------------------
 * $Log: dev_mrvl_ge.h,v $
 * Revision 1.3  2021/09/24 01:22:45  harrchan
 * Collapse Elixir-branch to Main Trunk.
 *
 * Revision 1.2  2019/01/10 06:26:40  wilbhuan
 * 1. The beginning of Marvell 88E111X Gigabit Ethernet Transceiver device driver.
 * 2. Supported PHYs:
 *    (1)88E1112
 *
 *-------------------------------------------------
 */
