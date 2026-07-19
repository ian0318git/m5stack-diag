/* $Id: dev_32021.h,v 1.2 2012/03/28 00:38:07 mcharon Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/chips/dev_idt82v32021/dev_32021.h,v $
 *-----------------------------------------------------------------------------
 * Filename:	dev_32021.h
 *
 * Description:	IDT82V32021 EBU WAN PLL.
 *		This header file defines registers offset, defaults, read &
 *		write bitmasks, & bit locations.
 *
 * Copyright (c) 2009 ~ 2012 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------------------
 *
 */

#ifndef __DEV_32021_H__
#define __DEV_32021_H__

#include "types.h"
#include "i2c_api.h"
#include "common_utils.h"
#include "dev_object.h"

typedef uchar reg_o;	/* Register offset */
typedef uchar reg_d;	/* Register data */

/*
 * dev_error_report message codes
 */
typedef enum {
    IDT32021_DEV_STATE = 0,
    IDT32021_ATTACH,
    IDT32021_SHOW,
    IDT32021_INIT,
    IDT32021_OPER_EN,
    IDT32021_OPER_DIS,
    IDT32021_INTR_EN,
    IDT32021_INTR_DIS,
    IDT32021_ISR,
    IDT32021_I2C_READ,
    IDT32021_I2C_WRITE,
    IDT32021_I2C_DESTROY,
    IDT32021_RESET,
    IDT32021_REG_TEST,
}idt82v32021_report_code_t;

/*
 * device callin function - service provided and defined by the device
 */
typedef struct idt32021_callin_fvt_t_ {
    int (*register_test)(dev_object_t *);
}idt32021_callin_fvt_t;

/*
 * device callout function - service needed by the device and defined by
 *			     platform
 */
typedef struct idt32021_callout_fvt_t_ {
    /*
     * Vectors set by the upper level (eg., platform).
     */
    uint32_t (*open)(n2g_i2c_if_t *);
    uint32_t (*close)(n2g_i2c_if_t *);
    uint32_t (*rd)(n2g_i2c_if_t *);
    uint32_t (*wr)(n2g_i2c_if_t *);
    uint32_t (*reset)(n2g_i2c_if_t *, boolean);
}idt32021_callout_fvt_t;

/*
** Define the EBU WAN PLL device object structure.
*/
typedef struct dev_idt82v32021_object_t_ {
    dev_object_t		base;
    idt32021_callin_fvt_t	*callin_fvt;
    idt32021_callout_fvt_t	*callout_fvt;
    n2g_i2c_if_t		*i2c_p;		/* I2C API interface pointer */
    reg_info_t			*reg_table_p;	/* Register test table pointer*/
}dev_idt82v32021_object_t;

/*
 * Registers initialization struct 
 */
typedef struct dev_idt32021_reg_init_t_ {
    reg_o	offset;		/* Register offset */
    reg_d	data;		/* Init value */
} dev_idt32021_reg_init_t;

/* Registers Map */
/*	Global Control Registers */
#define IDT32021_ID1		0x00	/* ID[7:0]  - Device ID 1 */
#define IDT32021_ID2		0x01	/* ID[15:8] - Device ID 2 */
#define IDT32021_NOM_FREQ_CNFG1	0x04	/* Crystal Osc. Freq. Offset Conf. 1 */
#define IDT32021_NOM_FREQ_CNFG2	0x05	/* Crystal Osc. Freq. Offset Conf. 2 */
#define IDT32021_NOM_FREQ_CNFG3	0x06	/* Crystal Osc. Freq. Offset Conf. 3 */
#define IDT32021_PH_ALRM_TO_CNF	0x08	/* Phase Lock Alarm Timeout Config. */
#define IDT32021_INP_MODE_CNF	0x09	/* Input Mode Configuration */
#define IDT32021_OSCI_CNFG	0x0A	/* Master Clock Configuration */
#define IDT32021_MON_SW_PBO_CNF	0x0B	/* Freq. Mon, Inp Clk Sel & PBO Ctl. */
#define IDT32021_PROTECTION_CNF	0x7E	/* Register Protection Mode Config. */

/*	Interrupt Registers */
#define IDT32021_INTERRUPT_CNFG	0x0C	/* Interrupt Configuration */
#define IDT32021_INTERRUPT1_STS	0x0D	/* Interrupt Status 1 */
#define IDT32021_INTERRUPT2_STS	0x0E	/* Interrupt Status 2 */
#define IDT32021_INTERRUPT3_STS	0x0F	/* Interrupt Status 3 */
#define IDT32021_INTR1_EN_CNFG	0x10	/* Interrupt Control 1 */
#define IDT32021_INTR2_EN_CNFG	0x11	/* Interrupt Control 2 */
#define IDT32021_INTR3_EN_CNFG	0x12	/* Interrupt Control 3 */

/*	Input Clock Frequency & Priority Configuration Registers */
#define IDT32021_IN1_CMOS_CNFG	0x16	/* CMOS Input Clock 1 Configuration */
#define IDT32021_IN2_CMOS_CNFG	0x17	/* CMOS Input Clock 2 Configuration */
#define IDT32021_PRE_DIV_C_CNFG	0x23	/* DivN Divider Channel Selection */
#define IDT32021_PRE_DIVN1_CNFG	0x24	/* DivN Divider Division Factor Cfg 1 */
#define IDT32021_PRE_DIVN2_CNFG	0x25	/* DivN Divider Division Factor Cfg 2 */
#define IDT32021_IN1_2_CMOS_SEL	0x27	/* CMOS Input Clock 1 & 2 Priority Cfg*/

/*	Input clock Quality Monitoring Configuration & Status Registers */
#define IDT32021_FREQ_MON_FACT	0x2E	/* Factor of Frequency Monitor Config */
#define IDT32021_ALL_FREQ_MON_T	0x2F	/* Frequency Monitor Threshold for All*/
#define IDT32021_UP_THR_0_CNFG	0x31	/* Upper Threshold for Leaky B Cfg 0 */
#define IDT32021_LO_THR_0_CNFG	0x32	/* Lower Threshold for Leaky B Cfg 0 */
#define IDT32021_BUCKET_SIZE_0	0x33	/* Bucket Size for Lecky Bucket Cfg 0 */
#define IDT32021_DECAY_RATE_0_C	0x34	/* Decay Rate for Leaky Bucket Cfg 0 */
#define IDT32021_UP_THR_1_CNFG	0x35	/* Upper Threshold for Leaky B Cfg 1 */
#define IDT32021_LO_THR_1_CNFG	0x36	/* Lower Threshold for Leaky B Cfg 1 */
#define IDT32021_BUCKET_SIZE_1	0x37	/* Bucket Size for Leaky Bucket Cfg 1 */
#define IDT32021_DECAY_RATE_1_C	0x38	/* Decay Rate for leaky Bucket Cfg 1 */
#define IDT32021_UP_THR_2_CNFG	0x39	/* Upper Threshold for Leaky B Cfg 2 */
#define IDT32021_LO_THR_2_CNFG	0x3A	/* Lower Threshold for Leaky B Cfg 2 */
#define IDT32021_BUCKET_SIZE_2	0x3B	/* Bucket Size for Leaky Bucket Cfg 2 */
#define IDT32021_DECAY_RATE_2_C	0x3C	/* Decay Rate for leaky Bucket Cfg 2 */
#define IDT32021_UP_THR_3_CNFG	0x3D	/* Upper Threshold for Leaky B Cfg 3 */
#define IDT32021_LO_THR_3_CNFG	0x3E	/* Lower Threshold for Leaky B Cfg 3 */
#define IDT32021_BUCKET_SIZE_3	0x3F	/* Bucket Size for Leaky Bucket Cfg 3 */
#define IDT32021_DECAY_RATE_3_C	0x40	/* Decay Rate for leaky Bucket Cfg 3 */
#define IDT32021_IN_FREQ_R_CH_C	0x41	/* Input Clock Frequency Read Channel */
#define IDT32021_IN_FREQ_READ_S	0x42	/* Input Clock Frequency Read Value */
#define IDT32021_IN1_2_CMOS_STS	0x44	/* CMOS Input Clock 1 & 2 Status */

/*	T0 DPLL Input Clock Selection Registers */
#define IDT32021_INPUT_VALID1_S	0x4A	/* Input Clocks Validity 1 */
#define IDT32021_PRIO_TBL1_STS	0x4E	/* Priority Status 1 */
#define IDT32021_PRIO_TBL2_STS	0x4F	/* Priority Status 2 */
#define IDT32021_T0_INPUT_SEL_C	0x50	/* T0 Selected Input Clock Config. */

/*	T0 DPLL State Machine Control Registers */
#define IDT32021_OPERATING_STS	0x52	/* DPLL Operating Status */
#define IDT32021_T0_OP_MODE_CFG	0x53	/* T0 DPLL Operating Mode Config. */

/*	T0 DPLL & T0 APLL Configuration Registers */
#define IDT32021_T0_D_APLL_PATH	0x55	/* T0 DPLL & APLL Path Configuration */
#define IDT32021_T0_DPLL_ST_BW	0x56	/* T0 DPLL Start bandwidth & Damping */
#define IDT32021_T0_DPLL_ACQ_BW	0x57	/* T0 DPLL Acquisition Bandwidth & D */
#define IDT32021_T0_DPLL_LCK_BW	0x58	/* T0 DPLL Locked Bandwidth & Damping */
#define IDT32021_T0_BW_OVERSHOT	0x59	/* T0 DPLL Bandwidth Overshoot Config */
#define IDT32021_PHASE_LOSS_CRS	0x5A	/* Phase Loss Coarse Detector Limit */
#define IDT32021_PHASE_LOSS_FIN	0x5B	/* Phase Loss Fine Detector Limit Cfg */
#define IDT32021_T0_HOLD_MODE_C	0x5C	/* T0 DPLL Holdover Mode Config. */
#define IDT32021_T0_HOLD_FREQ_1	0x5D	/* T0 DPLL Holdover Freq Config. 1 */
#define IDT32021_T0_HOLD_FREQ_2	0x5E	/* T0 DPLL Holdover Freq Config. 2 */
#define IDT32021_T0_HOLD_FREQ_3	0x5F	/* T0 DPLL Holdover Freq Config. 3 */
#define IDT32021_CURR_FREQ_1_S	0x62	/* DPLL Current Frequency Status 1 */
#define IDT32021_CURR_FREQ_2_S	0x63	/* DPLL Current Frequency Status 2 */
#define IDT32021_CURR_FREQ_3_S	0x64	/* DPLL Current Frequency Status 3 */
#define IDT32021_DPLL_FREQ_SOFT	0x65	/* DPLL Soft Limit Configuration */
#define IDT32021_DPLL_FREQ_HRD1	0x66	/* DPLL Hard Limit Configuration 1 */
#define IDT32021_DPLL_FREQ_HRD2	0x67	/* DPLL Hard Limit Configuration 2 */
#define IDT32021_CURR_DPLL_P1_S	0x68	/* DPLL Current Phase Status 1 */
#define IDT32021_CURR_DPLL_P2_S	0x69	/* DPLL Current Phase Status 2 */
#define IDT32021_T0_APLL_BW_CFG	0x6A	/* T0 APLL Bandwidth Configuration */

/*	Output Configuration Registers */
#define IDT32021_OUT1_FREQ_CNFG	0x6D	/* Output Clock 1 Frequency Config. */
#define IDT32021_OUT1_INV_CNFG	0x73	/* Output Clock 1 Invert Config. */
#define IDT32021_FR_SYNC_CNFG	0x74	/* Frame Sync Output Configuration */

/*	PBO & Phase Offset Control Registers */
#define IDT32021_PH_MON_PBO_CFG	0x78	/* Phase Transient Monitor & PBO Cfg. */

/*	Synchronization Configuration Registers */
#define IDT32021_SYNC_MON_CNFG	0x7C	/* Sync Monitor Configuration */
#define IDT32021_SYNC_PHASE_CFG	0x7D	/* Sync Phase Configuration */

/* Device ID 1 */
#define IDT32021_DEVID1		0x88	/* Device ID 1 */
/* Device ID 2 */
#define IDT32021_DEVID2		0x11	/* Device ID 2 */

/* Interrupt Control/Status 1 */
#define IDT32021_INTR_IN2_CMOS	0x08	/* IN2 CMOS Interrupt Enable */
#define IDT32021_INTR_IN1_CMOS	0x04	/* IN1 CMOS Interrupt Enable */

/* Interrupt Control/Status 2 */
#define IDT32021_INTR_T0_OP_MOD	0x80	/* T0 DPLL Operating mode Intr Enable */
#define IDT32021_INTR_T0_MAIN_R	0x40	/* T0 selected input clock fail
					 * interrupt enable */

/* Interrupt Control/Status 3 */
#define IDT32021_INTR_EX_SYNC_A	0x80	/* External sync alarm intr enable */

/* T0 DPLL State Machine Control Registers (DPLL Operating Status) */
#define IDT32021_OP_STS_EX_SYNC	0x80	/* External sync alarm status */
#define IDT32021_OP_STS_SOFT_F	0x20	/* Soft alarm status */
#define IDT32021_OP_STS_DPLL_L	0x08	/* T0 DPLL Locked */
#define IDT32021_OP_STS_FREE	0x01	/* Free-Run */
#define IDT32021_OP_STS_HOLD	0x02	/* Holdover */
#define IDT32021_OP_STS_LOCK	0x04	/* Locked */
#define IDT32021_OP_STS_P_L2	0x05	/* Pre-Locked2 */
#define IDT32021_OP_STS_P_L	0x06	/* Pre-Locked */
#define IDT32021_OP_STS_LOST	0x07	/* Lost-Phase */
#define IDT32021_OP_STS_MASK	0x07	/* DPLL Operating mode mask */

/* Miscellaneous defines */
#define IDT32021_DELAY_AFTER_RESET 500	/* Wait time after reset release */
#define IDT32021_RESET_TIME	50	/* Reset assert minimum time */
#define IDT32021_SHOW_LINE_BREAK 4	/* Print 8 registers per line */
#define IDT32021_NUM_INTRS (IDT32021_INTR3_EN_CNFG - IDT32021_INTR1_EN_CNFG + 1)
#define TDM_PLL_LOCK_POLLING_INTERVAL	2	/* Milliseconds */
#define TDM_PLL_LOCK_TIMEOUT	7 * 1000	/* 7 seconds */

/* Functions prototype */
extern void idt32021_dev_create(dev_object_t *, dev_error_report_t);


#endif /* __DEV_32021_H__ */

/*------------------------------------------------------------------
$Log: dev_32021.h,v $
Revision 1.2  2012/03/28 00:38:07  mcharon
remove forward slash from second line

Revision 1.1.1.1  2012/03/23 23:02:01  ptong
Initial archive of ng_diag module


$Endlog$
*/
