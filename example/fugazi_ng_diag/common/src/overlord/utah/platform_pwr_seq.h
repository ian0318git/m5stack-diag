/* $Id: platform_pwr_seq.h,v 1.10 2014/05/29 00:41:28 mcharon Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/overlord/utah/platform_pwr_seq.h,v $
 *------------------------------------------------------------------
 * Filename:	platform_pwr_seq.h
 *
 * Description: Informers Power Sequencer. This file is  based on EDCS-618748.
 *
 * Copyright (c) 2014 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#ifndef __PLATFORM_PWR_SEQ_H__
#define __PLATFORM_PWR_SEQ_H__

#include "dev_csco_10698.h"


/* Common defines */
#define OVLD_PWR_CONSUMP_CONST   614

#define PWR_SEQ_RTC_TEST_TIME	10	/* Run Time Counter wait time */
#define PWR_SEQ_STAT_CHECK_FAIL	(FAILED + 1)
#define PWR_SEQ_RUNTIME_RETRY	3	/* Wraparound retry count */
#define PWR_SEQ_BUF_SIZE 32

#define PWR_SEQ_SHOW_MRGN   1
#define PWR_SEQ_SPE_OP_OFF  8
#define PWR_SEQ_DEVICE_OFF  4
#define PWR_SEQ_MARGIN_OFF  0

/* Utah, Sword, Dagger VREF: 2.702 */
#define PWR_SEQ_VOLT_PER_TICK   0.000659668  /* (VREF/4096) */
#define MARGIN_DELAY_1V_1_2V    14 /* in secs */
#define MARGIN_DELAY_DDR        2  /* in secs */
#define SREC_LINE_LEN           1024 /* in bytes */

typedef enum {
    PWR_SEQ_VP3P3 = 0,  
    PWR_SEQ_VP2P5,    
    PWR_SEQ_DDR,     
    PWR_SEQ_VP1P2,  
    PWR_SEQ_VP1P0, 
} select_margin_t;

typedef enum {
    MARGIN_LO = 0, 
    MARGIN_NORM,
    MARGIN_HI, 
} margin_range_t;

#define PWR_SEQ_3_3_MRGN_HI   ((PWR_SEQ_VP3P3 << PWR_SEQ_DEVICE_OFF) | MARGIN_HI)
#define PWR_SEQ_3_3_MRGN_LO   ((PWR_SEQ_VP3P3 << PWR_SEQ_DEVICE_OFF) | MARGIN_LO)
#define PWR_SEQ_3_3_MRGN_NORM ((PWR_SEQ_VP3P3 << PWR_SEQ_DEVICE_OFF) | MARGIN_NORM)
#define PWR_SEQ_2_5_MRGN_HI   ((PWR_SEQ_VP2P5 << PWR_SEQ_DEVICE_OFF) | MARGIN_HI)
#define PWR_SEQ_2_5_MRGN_LO   ((PWR_SEQ_VP2P5 << PWR_SEQ_DEVICE_OFF) | MARGIN_LO)
#define PWR_SEQ_2_5_MRGN_NORM ((PWR_SEQ_VP2P5 << PWR_SEQ_DEVICE_OFF) | MARGIN_NORM)
#define PWR_SEQ_DDR_MRGN_HI   ((PWR_SEQ_DDR << PWR_SEQ_DEVICE_OFF) | MARGIN_HI)
#define PWR_SEQ_DDR_MRGN_LO   ((PWR_SEQ_DDR << PWR_SEQ_DEVICE_OFF) | MARGIN_LO)
#define PWR_SEQ_DDR_MRGN_NORM ((PWR_SEQ_DDR << PWR_SEQ_DEVICE_OFF) | MARGIN_NORM)
#define PWR_SEQ_1_2_MRGN_HI   ((PWR_SEQ_VP1P2 << PWR_SEQ_DEVICE_OFF) | MARGIN_HI)
#define PWR_SEQ_1_2_MRGN_LO   ((PWR_SEQ_VP1P2 << PWR_SEQ_DEVICE_OFF) | MARGIN_LO)
#define PWR_SEQ_1_2_MRGN_NORM ((PWR_SEQ_VP1P2 << PWR_SEQ_DEVICE_OFF) | MARGIN_NORM)
#define PWR_SEQ_1_0_MRGN_HI   ((PWR_SEQ_VP1P0 << PWR_SEQ_DEVICE_OFF) | MARGIN_HI)
#define PWR_SEQ_1_0_MRGN_LO   ((PWR_SEQ_VP1P0 << PWR_SEQ_DEVICE_OFF) | MARGIN_LO)
#define PWR_SEQ_1_0_MRGN_NORM ((PWR_SEQ_VP1P0 << PWR_SEQ_DEVICE_OFF) | MARGIN_NORM)
/* 
 * EDCS-1263891 from Victory  Platform PSEQ
 */
#define PWR_SEQ_REV	0x00
#define PWR_SEQ_PIN_CODE	0x01
#define PWR_SEQ_STA_S	0x02
#define PWR_SEQ_STA_C	0x03
#define PWR_SEQ_F0_S	0x04
#define PWR_SEQ_F1_S	0x05
#define PWR_SEQ_F0_C	0x06
#define PWR_SEQ_F1_C	0x07
#define PWR_SEQ_SCR0	0x08
#define PWR_SEQ_SCR1	0x09
#define PWR_SEQ_SCR2	0x0A
#define PWR_SEQ_SCR3	0x0B
#define PWR_SEQ_SPARE	0x0C
#define PWR_SEQ_RTC_YM	0x0D
#define PWR_SEQ_RTC_DH	0x0E
#define PWR_SEQ_RTC_MS	0x0F

#define PWR_SEQ_BB_C	0x10
#define PWR_SEQ_BB_D0	0x11
#define PWR_SEQ_BB_D1	0x12
#define PWR_SEQ_BB_D2	0x13
#define PWR_SEQ_DD_D3	0x14
#define PWR_SEQ_3_3_MAR	0x15
#define PWR_SEQ_2_5_MAR	0x16
#define PWR_SEQ_DDR_MAR	0x17
#define PWR_SEQ_1_2_MAR	0x18
#define PWR_SEQ_1_0_MAR	0x19
#define PWR_SEQ_WD_EN	0x1A
#define PWR_SEQ_WD_REF	0x1B
#define PWR_SEQ_WD_TO	0x1C
#define PWR_SEQ_PR_D_EN	0x1D
#define PWR_SEQ_PW_D_T	0x1E
/*0x1F - 0x7F Reserved */

#define PWR_SEQ_12_0_LR	    0x20
#define PWR_SEQ_12_0_MAX	0x21
#define PWR_SEQ_12_0_MIN	0x22
#define PWR_SEQ_12_0_RPT	0x23
#define PWR_SEQ_12_0_C_LR	0x24
#define PWR_SEQ_12_0_C_MAX	0x25
#define PWR_SEQ_12_0_C_MIN	0x26
#define PWR_SEQ_12_0_C_RPT	0x27
#define PWR_SEQ_5_0_LR	0x28
#define PWR_SEQ_5_0_MAX	0x29
#define PWR_SEQ_5_0_MIN	0x2A
#define PWR_SEQ_5_0_RPT	0x2B
#define PWR_SEQ_3_3_LR	0x2C
#define PWR_SEQ_3_3_MAX	0x2D
#define PWR_SEQ_3_3_MIN	0x2E
#define PWR_SEQ_3_3_RPT	0x2F

#define PWR_SEQ_2_5_LR	0x30
#define PWR_SEQ_2_5_MAX	0x31
#define PWR_SEQ_2_5_MIN	0x32
#define PWR_SEQ_2_5_RPT	0x33
#define PWR_SEQ_1_8_LR	0x34
#define PWR_SEQ_1_8_MAX	0x35
#define PWR_SEQ_1_8_MIN	0x36
#define PWR_SEQ_1_8_RPT	0x37
#define PWR_SEQ_1_5_LR	0x38
#define PWR_SEQ_1_5_MAX	0x39
#define PWR_SEQ_1_5_MIN	0x3A
#define PWR_SEQ_1_5_RPT	0x3B
#define PWR_SEQ_1_2_LR	0x3C
#define PWR_SEQ_1_2_MAX	0x3D
#define PWR_SEQ_1_2_MIN	0x3E
#define PWR_SEQ_1_2_RPT	0x3F

#define PWR_SEQ_1_0_LR	0x40
#define PWR_SEQ_1_0_MAX	0x41
#define PWR_SEQ_1_0_MIN	0x42
#define PWR_SEQ_1_0_RPT	0x43
#define PWR_SEQ_0_75_DDR_LR	    0x44
#define PWR_SEQ_0_75_DDR_MAX	0x45
#define PWR_SEQ_0_75_DDR_MIN	0x46
#define PWR_SEQ_0_75_DDR_RPT	0x47
#define PWR_SEQ_1_5_DDR_LR	0x48
#define PWR_SEQ_1_5_DDR_MAX	0x49
#define PWR_SEQ_1_5_DDR_MIN	0x4A
#define PWR_SEQ_1_5_DDR_RPT	0x4B
#define PWR_SEQ_1_35_CPU_LR	0x4C
#define PWR_SEQ_1_35_CPU_MAX	0x4D
#define PWR_SEQ_1_35_CPU_MIN	0x4E
#define PWR_SEQ_1_35_CPU_RPT	0x4F

#define PWR_SEQ_1_0_VCC_LR	0x50
#define PWR_SEQ_1_0_VCC_MAX	0x51
#define PWR_SEQ_1_0_VCC_MIN	0x52
#define PWR_SEQ_1_0_VCC_RPT	0x53
#define PWR_SEQ_1_0_VNN_LR	0x54
#define PWR_SEQ_1_0_VNN_MAX	0x55
#define PWR_SEQ_1_0_VNN_MIN	0x56
#define PWR_SEQ_1_0_VNN_RPT	0x57
#define PWR_SEQ_1_8_CPU_LR	0x58
#define PWR_SEQ_1_8_CPU_MAX	0x59
#define PWR_SEQ_1_8_CPU_MIN	0x5A
#define PWR_SEQ_1_8_CPU_RPT	0x5B
#define PWR_SEQ_1_1_CPU_LR	0x5C
#define PWR_SEQ_1_1_CPU_MAX	0x5D
#define PWR_SEQ_1_1_CPU_MIN	0x5E
#define PWR_SEQ_1_1_CPU_RPT	0x5F

#define PWR_SEQ_1_0_CPU_LR	0x60
#define PWR_SEQ_1_0_CPU_MAX	0x61
#define PWR_SEQ_1_0_CPU_MIN	0x62
#define PWR_SEQ_1_0_CPU_RPT	0x63
#define PWR_SEQ_NIM_CR_LR	0x64
#define PWR_SEQ_NIM_CR_MAX	0x65
#define PWR_SEQ_NIM_CR_MIN	0x66
#define PWR_SEQ_NIM_CR_RPT	0x67
#define PWR_SEQ_FAN_CR_LR	0x68
#define PWR_SEQ_FAN_CR_MAX	0x69
#define PWR_SEQ_FAN_CR_MIN	0x6A
#define PWR_SEQ_FAN_CR_RPT	0x6B
#define PWR_SEQ_3_3_STV_LR	0x6C
#define PWR_SEQ_3_3_STV_MAX	0x6D
#define PWR_SEQ_3_3_STV_MIN	0x6E
#define PWR_SEQ_3_3_STV_RPT	0x6F

#define PWR_SEQ_3_0_LR		0x70
#define PWR_SEQ_3_0_MAX		0x71
#define PWR_SEQ_3_0_MIN		0x72
#define PWR_SEQ_3_0_RPT		0x73
#define PWR_SEQ_MON21_LR	0x74
#define PWR_SEQ_MON21_MAX	0x75
#define PWR_SEQ_MON21_MIN	0x76
#define PWR_SEQ_MON21_RPT	0x77
#define PWR_SEQ_DEBUG_LR	0x78
#define PWR_SEQ_DEBUG_MAX	0x79
#define PWR_SEQ_DEBUG_MIN	0x7A
#define PWR_SEQ_DEBUG_RPT	0x7B
#define PWR_SEQ_MON23_LR	0x7C
#define PWR_SEQ_MON23_MAX	0x7D
#define PWR_SEQ_MON23_MIN	0x7E
#define PWR_SEQ_MON23_RPT	0x7F

#define PWR_SEQ_VREF_V_LR	0x80
#define PWR_SEQ_VREF_V_MAX	0x81
#define PWR_SEQ_VREF_V_MIN	0x82
#define PWR_SEQ_VREF_V_RPT	0x83
#define PWR_SEQ_TMP_LR	0x84
#define PWR_SEQ_TMP_MAX	0x85
#define PWR_SEQ_TMP_MIN	0x86
#define PWR_SEQ_TMP_RPT	0x87

/*
 * Limit Register - OFF, ON, Over, Under
 */
#define PWR_SEQ_12_0_POFF	0x88
#define PWR_SEQ_12_0_PON	0x89
#define PWR_SEQ_12_0_OVER	0x8A
#define PWR_SEQ_12_0_UNDER	0x8B
#define PWR_SEQ_12_0_C_POFF	0x8C
#define PWR_SEQ_12_0_C_PON	0x8D
#define PWR_SEQ_12_0_C_OVER	0x8E
#define PWR_SEQ_12_0_C_UNDER	0x8F
#define PWR_SEQ_5_0_POFF	0x90
#define PWR_SEQ_5_0_PON	0x91
#define PWR_SEQ_5_0_OVER	0x92
#define PWR_SEQ_5_0_UNDER	0x93
#define PWR_SEQ_3_3_POFF	0x94
#define PWR_SEQ_3_3_PON	0x95
#define PWR_SEQ_3_3_OVER	0x96
#define PWR_SEQ_3_3_UNDER	0x97

#define PWR_SEQ_2_5_POFF	0x98
#define PWR_SEQ_2_5_PON	0x99
#define PWR_SEQ_2_5_OVER	0x9A
#define PWR_SEQ_2_5_UNDER	0x9B
#define PWR_SEQ_1_8_POFF	0x9C
#define PWR_SEQ_1_8_PON	0x9D
#define PWR_SEQ_1_8_OVER	0x9E
#define PWR_SEQ_1_8_UNDER	0x9F
#define PWR_SEQ_1_5_POFF	0xA0
#define PWR_SEQ_1_5_PON	0xA1
#define PWR_SEQ_1_5_OVER	0xA2
#define PWR_SEQ_1_5_UNDER	0xA3
#define PWR_SEQ_1_2_POFF	0xA4
#define PWR_SEQ_1_2_PON	0xA5
#define PWR_SEQ_1_2_OVER	0xA6
#define PWR_SEQ_1_2_UNDER	0xA7

#define PWR_SEQ_1_0_POFF	0xA8
#define PWR_SEQ_1_0_PON	0xA9
#define PWR_SEQ_1_0_OVER	0xAA
#define PWR_SEQ_1_0_UNDER	0xAB
#define PWR_SEQ_0_75_DDR_POFF	0xAC
#define PWR_SEQ_0_75_DDR_PON	0xAD
#define PWR_SEQ_0_75_DDR_OVER	0xAE
#define PWR_SEQ_0_75_DDR_UNDER	0xAF
#define PWR_SEQ_1_5_DDR_POFF	0xB0
#define PWR_SEQ_1_5_DDR_PON	0xB1
#define PWR_SEQ_1_5_DDR_OVER	0xB2
#define PWR_SEQ_1_5_DDR_UNDER	0xB3
#define PWR_SEQ_1_35_CPU_POFF	0xB4
#define PWR_SEQ_1_35_CPU_PON	0xB5
#define PWR_SEQ_1_35_CPU_OVER	0xB6
#define PWR_SEQ_1_35_CPU_UNDER	0xB7

#define PWR_SEQ_1_0_VCC_POFF	0xB8
#define PWR_SEQ_1_0_VCC_PON	0xB9
#define PWR_SEQ_1_0_VCC_OVER	0xBA
#define PWR_SEQ_1_0_VCC_UNDER	0xBB
#define PWR_SEQ_1_0_VNN_POFF	0xBC
#define PWR_SEQ_1_0_VNN_PON	0xBD
#define PWR_SEQ_1_0_VNN_OVER	0xBE
#define PWR_SEQ_1_0_VNN_UNDER	0xBF
#define PWR_SEQ_1_8_CPU_POFF	0xC0
#define PWR_SEQ_1_8_CPU_PON	0xC1
#define PWR_SEQ_1_8_CPU_OVER	0xC2
#define PWR_SEQ_1_8_CPU_UNDER	0xC3
#define PWR_SEQ_1_1_CPU_POFF	0xC4
#define PWR_SEQ_1_1_CPU_PON	0xC5
#define PWR_SEQ_1_1_CPU_OVER	0xC6
#define PWR_SEQ_1_1_CPU_UNDER	0xC7

#define PWR_SEQ_1_0_CPU_POFF	0xC8
#define PWR_SEQ_1_0_CPU_PON	0xC9
#define PWR_SEQ_1_0_CPU_OVER	0xCA
#define PWR_SEQ_1_0_CPU_UNDER	0xCB
#define PWR_SEQ_NIM_CR_POFF	0xCC
#define PWR_SEQ_NIM_CR_PON	0xCD
#define PWR_SEQ_NIM_CR_OVER	0xCE
#define PWR_SEQ_NIM_CR_UNDER	0xCF
#define PWR_SEQ_FAN_CR_POFF	0xD0
#define PWR_SEQ_FAN_CR_PON	0xD1
#define PWR_SEQ_FAN_CR_OVER	0xD2
#define PWR_SEQ_FAN_CR_UNDER	0xD3
#define PWR_SEQ_3_3_STV_POFF	0xD4
#define PWR_SEQ_3_3_STV_PON	0xD5
#define PWR_SEQ_3_3_STV_OVER	0xD6
#define PWR_SEQ_3_3_STV_UNDER	0xD7

#define PWR_SEQ_3_0_POFF		0xD8
#define PWR_SEQ_3_0_PON		0xD9
#define PWR_SEQ_3_0_OVER		0xDA
#define PWR_SEQ_3_0_UNDER		0xDB
#define PWR_SEQ_MON21_POFF	0xDC
#define PWR_SEQ_MON21_PON	0xDD
#define PWR_SEQ_MON21_OVER	0xDE
#define PWR_SEQ_MON21_UNDER	0xDF
#define PWR_SEQ_DEBUG_POFF	0xE0
#define PWR_SEQ_DEBUG_PON	0xE1
#define PWR_SEQ_DEBUG_OVER	0xE2
#define PWR_SEQ_DEBUG_UNDER	0xE3
#define PWR_SEQ_MON23_POFF	0xE4
#define PWR_SEQ_MON23_PON	0xE5
#define PWR_SEQ_MON23_OVER	0xE6
#define PWR_SEQ_MON23_UNDER	0xE7

#define PWR_SEQ_VREF_V_POFF	0xE8
#define PWR_SEQ_VREF_V_PON	0xE9
#define PWR_SEQ_VREF_V_OVER	0xEA
#define PWR_SEQ_VREF_V_UNDER	0xEB
#define PWR_SEQ_TMP_POFF	0xEC
#define PWR_SEQ_TMP_PON	0xED
#define PWR_SEQ_TMP_OVER	0xEE
#define PWR_SEQ_TMP_UNDER	0xEF

/* 0xF0 - 0xFF reserved */

/* Power sequencer FW upgrade */
#define PWR_SEQ_FW_CMD_REG  0xFE
#define PWR_SEQ_FW_DATA_REG 0xFF
#define PWR_SEQ_CMD_UPDATE  0xFFEE
#define PWR_SEQ_CMD_REBOOT  0xFFED
#define PWR_SEQ_CMD_MAX_ADDR    0xFBFF
#define PWR_SEQ_EEPROM_END  0xFC00

/*
 * Voltage test parameter struct
 */
typedef struct pwr_seq_v_t_ {
    char *volt_p;               /* Voltage text pointer */
    ren_t over_v;               /* Over  Voltage ADC */
    ren_t under_v;              /* Under Voltage ADC */
    ren_o rt_o;                 /* Real time register offset */
    ren_o max_o;                /* Maximum register offset */
    ren_o min_o;                /* Minimum register offset */
} pwr_seq_v_t;


#define PWR_SEQ_PWR_CTRL              0x48

/* Firmware Revision - 0x00 */
#define PS_REV_MAJOR_MSK     0xFF00 /* Major Revision */
#define PS_REV_MAJOR_SHIFT        8 /* Major Revision Shift */
#define PS_REV_MINOR_MSK     0x00FF /* Minor Revision */

/* Debug Pin Code - 0x01 */
#define PS_DEBUG_PIN_CODE_MSK     0x0007 /* Minor Revision */

/* Status Registers, Sticky/Clearable - 0x02/0x03 */
#define PS_STAT_SYS_DWN_VIA_SWITCH_MSK   0x2000 /* System powered down via Switch */
#define PS_STAT_WDOG_TO                  0x1000 /* Watch Dog Time out Counter Shift */
#define PS_STAT_12V_SHORT_DET_MSK        0x0C00 /* 12V short circuit detect during pwoer up */
#define PS_STAT_12V_SHORT_DET_SHIFT          10 /* 12v short circuit detect during power up  Shift */
#define PS_STAT_VTG_FAULT_CNTR_MSK       0x0300 /* Voltage Fault Counter */
#define PS_STAT_VTG_FAULT_CNTR_SHIFT          8 /* Voltage Fault Counter Shift */
#define PS_STAT_VTG_FAULT_PU_MSK         0x0080 /* Voltage Fault During Power Up */
#define PS_STAT_VTG_FAULT_OPT_MSK        0x0040	/* Voltage Fault During Operation */
#define PS_STAT_SYS_PWR_DWN_MSK          0x0020	/* System Power Down */
#define PS_STAT_WDOG_TO_MSK              0x0010 /* Watchdog Timeout */
#define PS_STAT_SYS_PWR_CYC_MSK          0x000F /* System Power cycle by FPGA */

/* Voltage Fault Registers 0/1, Sticky/Clearable - 0x04 - 0x07 */
/* register 0 */
#define PS_VF_TEMP_FAULT_MSK         0x0200 /* TEMP Fault */
#define PS_VF_VREF_FAULT_MSK         0x0100 /* VREF Fault */
#define PS_VF_MON23_FAULT_MSK         0x0080 /* Spare 23 Fault */
#define PS_VF_DEBUG_FAULT_MSK         0x0040 /* Debug Fault */
#define PS_VF_MON21_FAULT_MSK         0x0020 /* Spare 21 Fault */
#define PS_VF_3_0V_FAULT_MSK            0x0010 /* 3.0V Fault */
#define PS_VF_3_3V_ST_FAULT_MSK         0x0008 /* 3.3V StandBy Fault */
#define PS_VF_12V_FAN_CURR_FAULT_MSK     0x0004 /* 12V FAN Current Fault */
#define PS_VF_12V_NIM_CURR_FAULT_MSK     0x0002 /* 12V NIM Current Fault */
#define PS_VF_1_0V_CPU_FAULT_MSK         0x0001 /* 1.0V CPU Fault */
/* register 1 */
#define PS_VF_1_1V_CPU_FAULT_MSK         0x8000 /* 1.1V CPU Fault */
#define PS_VF_1_8V_CPU_FAULT_MSK         0x4000 /* 1.8V CPU Fault */
#define PS_VF_1_0V_VNN_FAULT_MSK         0x2000 /* 1.0V VNN Fault */
#define PS_VF_1_0V_VCC_FAULT_MSK         0x1000 /* 1.0V VCC Fault */
#define PS_VF_1_35V_CPU_FAULT_MSK        0x0800 /* 1.35V CPU Fault */
#define PS_VF_1_5V_DDR_FAULT_MSK         0x0400 /* 1.5V DDR Fault */
#define PS_VF_0_75V_DDR_VTT_FAULT_MSK    0x0200 /* 0.75V DDR VTT Fault */
#define PS_VF_1_0V_FAULT_MSK             0x0100 /* 1.0V Fault */
#define PS_VF_1_2V_FAULT_MSK             0x0080 /* 1.2V Fault */
#define PS_VF_1_5V_FAULT_MSK             0x0040 /* 1.5V Fault */
#define PS_VF_1_8V_FAULT_MSK             0x0020 /* 1.8V Fault */
#define PS_VF_2_5V_PCH_FAULT_MSK         0x0010 /* 2.5V PCH Fault */
#define PS_VF_3_3V_FAULT_MSK             0x0008 /* 3.3V Fault */
#define PS_VF_5V_FAULT_MSK               0x0004 /* 5V Fault */
#define PS_VF_12V_CURR_FAULT_MSK         0x0002 /* 12V Current Fault */
#define PS_VF_12V_FAULT_MSK              0x0001 /* 12V Fault */

#if 0
/* Run Time Counter - 0x05-07 */
#define PS_RTC_MSK   0xFFFF /* Real Time Counter */

/* Voltage Registers (12 Volts, 5.0 Volts, 3.3 Volts, 1.8 Volts PCH, 1.5 Volts PCH
 * 1.5 Volts CCPU, 1.5 Volts ICPU, 1.05 Volts , 1.0 Volts PCH) 
 * Real Time/Maximum/Minimum - 0x08-25
 */
#define PS_VC_ADC_MSK   0x03FF /* ADC Reading */

/* Scratch Pad Register - 0x26-29 */
#define PS_SCR_PAD_MSK  0xFFFF /* Scratch Pad */

/* Disable Voltage Fault Detection Register - 0x2A */
#define PS_DVFD_DIS_VTG_SD_KEY_MSK     0xFE00 /* Disable Voltage Shutdown Key */
#define PS_DVFD_DIS_VTG_SD_KEY_SHIFT        9 /* Disable Voltage Shutdown Key Shift */
#define PS_DVFD_DIS_1_0V_PCH_SD_MSK    0x0100 /* Disable 1.0V PCH Shutdown */
#define PS_DVFD_DIS_1_05V_SD_MSK       0x0080 /* Disable 1.05V Shutdown */
#define PS_DVFD_DIS_1_5V_ICPU_SD_MSK   0x0040 /* Disable 1.5V ICPU Shutdown */
#define PS_DVFD_DIS_1_5V_CCPU_SD_MSK   0x0020 /* Disable 1.5V CCPU Shutdown */
#define PS_DVFD_DIS_1_5V_PCH_SD_MSK    0x0010 /* Disable 1.5V PCH Shutdown */
#define PS_DVFD_DIS_1_8V_PCH_SD_MSK    0x0008 /* Disable 1.8V PCH Shutdown */
#define PS_DVFD_DIS_3_3V_SD_MSK        0x0004 /* Disable 3.3V Shutdown */
#define PS_DVFD_DIS_5V_SD_MSK          0x0002 /* Disable 5V Shutdown */
#define PS_DVFD_DIS_12V_SD_MSK         0x0001 /* Disable 12V Shutdown */

/* Enable Double Sampling Regisgter - 0x2B */
#define PS_EDS_EN_DS_KEY_MSK      0xFE00 /* Enable Double Sampling Key */
#define PS_EDS_EN_DS_KEY_SHIFT         9 /* Enable Double Sampling Key Shift */
#define PS_EDS_1_0V_PCH_DS_MSK    0x0100 /* 1.0V PCH core Double Sampling */
#define PS_EDS_1_05V_DS_MSK       0x0080 /* 1.05V Double Sampling */
#define PS_EDS_1_5V_ICPU_DS_MSK   0x0040 /* 1.5V ICPU Double Sampling */
#define PS_EDS_1_5V_CCPU_DS_MSK   0x0020 /* 1.5V CCPU Double Sampling */
#define PS_EDS_1_5V_PCH_DS_MSK    0x0010 /* 1.5V PCH Double Sampling */
#define PS_EDS_1_8V_PCH_DS_MSK    0x0008 /* 1.8V PCH Double Sampling */
#define PS_EDS_3_3V_DS_MSK        0x0004 /* 3.3V Double Sampling */
#define PS_EDS_5V_DS_MSK          0x0002 /* 5V Double Sampling */
#define PS_EDS_12V_DS_MSK         0x0001 /* 12V Double Sampling */

/* Watchdog Enable Register - 0x2C */
#define PS_WDE_EN_WDOG_KEY_MSK       0x01FF /* Enable Watchdog Key */

/* Watchdog Refresh Register - 0x2D */
#define PS_WDR_WDOG_REFRESH_MSK      0xFFFF /* Watchdog Refresh */

/* Watchdog Timeout Value Register - 0x2E */
#define PS_WDTO_WDOG_TO_MSK          0xFFFF /* Watchdog Timeout Value in seconds */

/* Power Down Enable Register - 0x2F */
#define PS_PDE_EN_PWR_DWN_MSK        0xFFFF /* Power Down Enable */

/* Power Down Time Value Register - 0x30 */
#define PS_PDT_PWR_DWN_TIME_MSK      0xFFFF /* Power Down Time Value in seconds */

/* Voltage Ramp Time Registers - 0x31 to 0x39 */
#define PS_VRT_MAX_WAIT_MSK          0xFF00 /* Max wait time for Power up */
#define PS_VRT_MAX_WAIT_SHIFT             8 /* Max wait time for Power up Shift */
#define PS_VRT_TIME_TO_PWR_UP_MSK    0x00FF /* Time to Power up */

/* Number of Power Up Loops Register - 0x3A, 0x40 */
#define PS_NUM_12V_CHK_LP_MSK        0xFFFF /* Number of 12V turned on */

/* Power Up Flow Register - 0x3B, 0x41 */
#define PS_SHORT_CHK_LP_CNTR_MSK     0xFF00 /* Short check loop count */
#define PS_SHORT_CHK_LP_CNTR_SHIFT        8 /* Short check loop count shift */
#define PS_DET_HI_CUR_VTG_2_MSK      0x0008 /* Detect High Current & Voltage (Second) */
#define PS_DET_HI_CUR_VTG_1_MSK      0x0004 /* Detect High Current & Voltage (First) */
#define PS_DET_HI_CUR_NO_VTG_MSK     0x0002 /* Detect High Current & No Voltage */
#define PS_DET_HI_CUR_MSK            0x0001 /* Detect High Current */

/* Max. 12V Power Up Current Register - 0x3C, 0x42 */
#define PS_MAX_12V_PU_CUR_MSK        0x03FF /* Max. 12V Power up Current */

/* Power up Current Count Register - 0x3D, 0x43 */
#define PS_PWR_UP_CUR_CNT_MSK        0x00FF /* Power up Current Count */

/* Power Up First Voltage Level Register - 0x3E, 0x44 */
#define PS_PU_FST_VTG_MSK            0x03FF /* Power up first Voltage Level */

/* Power up Voltage Count Register - 0x3F, 0x45 */
#define PS_PWR_UP_VTG_CNT_MSK        0xFFFF /* Power up Votage Count */

/* Power LED Control Register - 0x46 */
#define PS_PLC_YELLOW_TIME_MSK       0xFE00 /* Yellow LED ON/OFF Time (ms) */
#define PS_PLC_YELLOW_TIME_SHIFT          9 /* Yellow LED ON/OFF Time (ms) shift */
#define PS_PLC_GREEN_TIME_MSK        0x01FC /* Green LED ON/OFF Time (ms) */
#define PS_PLC_GREEN_TIME_SHIFT           2 /* Yellow LED ON/OFF Time (ms) shift */
#define PS_PLC_YELLOW_CTRL_MSK       0x0002 /* Yellow LED Control */
#define PS_PLC_GREEN_CTRL_MSK        0x0001 /* Green LED Control */

/* Power Cycle Request Count Register - 0x47 */
#define PS_PCRC_PWR_CYC_REQ_CNT_MSK  0xFFFF /* Power Cycle Request Count */

#endif 
/* Power Control Register - 0x48 */
#define PS_PC_POE_PWR_CTRL           0x0010 /* bit[4]: PoE Power Control Bit */
#define PS_PC_POE_PWR_CTRL_SHIFT          4 /* PoE Power Control Bit shift */
#define PS_PC_GSHUT_DWN_TIMER_MSK    0x000F /* bit[3:0]: Graceful Shutdown Timer */




/* Update from Overlord_Power_Sequencer_Spec 
 * Voltage Tests defines. These values are derived from HFS (EDCS-618748) Rev.
 * 4.0, Table 6 and Table 7.
 * Numbers in comments are: Voltage - Over/Under Voltage - 7% Over/Under Voltage.
 * According to HFS, 7% over/under will be flagged as error
 */
#if 0
/* Utah */ /* 0xDEAD == TBD */
#define PWR_12V_OVER		0x52F6 /* 12.15 - 13.97 - 13.00 */
#define PWR_12V_UNDER		0X3555 /* 12.15 - 8.99  - 8.99  */
#define PWR_5V_OVER		    0x2097 /* 5.15  - 5.92  - 5.51  */
#define PWR_5V_UNDER		0x1AAA // 02B7	/* 5.15  - 4.37  - 4.79  */
#define PWR_3P3V_OVER		0x1582 // 033A	/* 3.30  - 3.79  - 3.53  */
#define PWR_3P3V_UNDER	    0x1199 // 02CE	/* 3.30  - 2.80  - 3.07  */
#define PWR_2P5V_OVER		0x104B
#define PWR_2P5V_UNDER		0x0D55
#define PWR_1P8V_OVER		0x0BBB
#define PWR_1P8V_UNDER		0x0999
#define PWR_1P5V_OVER		0x09C7
#define PWR_1P5V_UNDER		0x07FF
#define PWR_1P2V_OVER		0x07D2
#define PWR_1P2V_UNDER		0x0666
#define PWR_0P75V_DDR_OVER		0x14E3 
#define PWR_0P75V_DDR_UNDER		0x03FF
#define PWR_1P0V_OVER		0x0684
#define PWR_1P0V_UNDER		0x0555

#define PWR_1P5V_DDR_OVER   0x09C7
#define PWR_1P5V_DDR_UNDER   0x7FF
#define PWR_1P35V_CPU_OVER   0x08CC
#define PWR_1P35V_CPU_UNDER   0x0733
#define PWR_1P0V_VCC_OVER   0x07B4
#define PWR_1P0V_VCC_UNDER   0x02F6
#define PWR_1P0V_VNN_OVER   0x07B4
#define PWR_1P0V_VNN_UNDER   0x02F6
#define PWR_1P8V_CPU_OVER   0x0BBB
#define PWR_1P8V_CPU_UNDER   0x0999
#define PWR_1P1V_CPU_OVER   0x072B
#define PWR_1P1V_CPU_UNDER   0x05DD
#define PWR_1P0V_CPU_OVER   0x0684
#define PWR_1P0V_CPU_UNDER   0x0555
#define PWR_NIM_CR_OVER   0x30DA
#define PWR_NIM_CR_UNDER   0x0000
#define PWR_FAN_CR_OVER   0x30DA
#define PWR_FAN_CR_UNDER   0x0000
#define PWR_3P3V_STV_OVER   0x1582
#define PWR_3P3V_STV_UNDER   0x1199
#define PWR_3P0V_OVER   0x138E
#define PWR_3P0V_UNDER   0x0FFF
#define PWR_MON21_OVER   0xFFFF
#define PWR_MON21_UNDER   0x0000
#define PWR_DEBUG_OVER   0xFFFF
#define PWR_DEBUG_UNDER   0x0000
#define PWR_MON23_OVER   0xFFFF
#define PWR_MON23_UNDER   0x0000
#define PWR_VREF_V_OVER   0x0794
#define PWR_VREF_V_UNDER   0x06E8
#define PWR_TMP_OVER    0x03BB
#define PWR_TMP_UNDER   0x022A


#define PWR_1P8V_PCH_OVER		0x02DA	/* 1.80  - 2.07  - 1.93  */
#define PWR_1P8V_PCH_UNDER	0x027A	/* 1.80  - 1.53  - 1.67  */
#define PWR_1P5V_PCH_OVER	  0x0262	/* 1.50  - 1.73  - 1.61  */
#define PWR_1P5V_PCH_UNDER	0x0211	/* 1.50  - 1.275 - 1.40  */
#define PWR_1P5V_ICPU_OVER	0x0262	/* 1.50  - 1.73  - 1.61  */
#endif
#if 0 /* Overlord */
#define PWR_12V_OVER		0x0335	/* 12.15 - 13.97 - 13.00 */
#define PWR_12V_UNDER		0x0238	/* 12.15 - 8.99  - 8.99  */
#define PWR_5V_OVER		  0x031F	/* 5.15  - 5.92  - 5.51  */
#define PWR_5V_UNDER		0x02B7	/* 5.15  - 4.37  - 4.79  */
#define PWR_3P3V_OVER		0x033A	/* 3.30  - 3.79  - 3.53  */
#define PWR_3P3V_UNDER	0x02CE	/* 3.30  - 2.80  - 3.07  */

#define PWR_1P8V_PCH_OVER		0x02DA	/* 1.80  - 2.07  - 1.93  */
#define PWR_1P8V_PCH_UNDER	0x027A	/* 1.80  - 1.53  - 1.67  */
#define PWR_1P5V_PCH_OVER	  0x0262	/* 1.50  - 1.73  - 1.61  */
#define PWR_1P5V_PCH_UNDER	0x0211	/* 1.50  - 1.275 - 1.40  */
#define PWR_1P5V_ICPU_OVER	0x0262	/* 1.50  - 1.73  - 1.61  */
#define PWR_1P5V_ICPU_UNDER	0x0211	/* 1.50  - 1.275 - 1.40  */
#define PWR_1P5V_CCPU_OVER	0x0262	/* 1.50  - 1.73  - 1.61  */
#define PWR_1P5V_CCPU_UNDER	0x0211	/* 1.50  - 1.275 - 1.40  */
#define PWR_1P05V_OVER	    0x01A9	/* 1.05  - 1.21  - 1.12  */
#define PWR_1P05V_UNDER	    0x0172	/* 1.05  - 0.8925- 0.98  */
#define PWR_1P0V_PCH_OVER	  0x0196	/* 1.00  - 1.15  - 1.07  */
#define PWR_1P0V_PCH_UNDER	0x0160	/* 1.00  - 0.85  - 0.93  */

#endif 

/* Functions prototype */
extern int init_pwr_seq(int err_log);
extern void build_pwr_tst_menu(int submenu);
extern int vtg_mrgn(int, int);

#endif /* __PLATFORM_PWR_SEQ_H__ */

/*------------------------------------------------------------------
$Log: platform_pwr_seq.h,v $
Revision 1.10  2014/05/29 00:41:28  mcharon
support programming of pwr seq fw using srec file

Revision 1.9  2014/03/04 03:07:23  hroni
power sequencer update utility supports srec file

Revision 1.8  2014/02/27 07:56:30  hroni
add update power sequencer utility

Revision 1.7  2014/02/11 09:30:41  hroni
modify margin delay value

Revision 1.6  2014/02/10 09:09:27  hroni
add 1 sec delay for ddr margining

Revision 1.5  2014/01/23 07:55:03  hroni
turn on PWR_SEQ_PWR_CTRL macro definition for controlling the power like POE PSU

Revision 1.4  2013/10/03 01:39:35  hroni
add transitional delay 2secs for margining 1.0V and 1.2V.

Revision 1.3  2013/09/09 06:35:02  hroni
1. use 5% margining on 1.2v and 1.0v, will recover to 9% after HW confirm it is safe
2. add show latest read voltage after and before doing margining
3. turn off byte swap in pwr_write()

Revision 1.2  2013/08/07 22:52:57  hroni
1. reset NIOS during diag init. 2. fix power sequencer utility

Revision 1.1  2013/06/14 10:25:49  alpeng
support voltage margin

Revision 1.1  2013/05/09 05:42:40  alpeng
moving overlord common code from x86

Revision 1.6  2012/07/19 07:02:14  palin2
Update Register 0x48, Power Control Register.

Revision 1.5  2012/06/26 12:18:42  palin2
Support to show the Power consumption of Overlord motherboard.

Revision 1.4  2012/06/26 03:59:45  palin2
Update registers map.

Revision 1.3  2012/04/16 15:29:26  palin2
Update 12V PoE PSU tests and utilities based on HW team's request:
1) Add "Registers test" support.
2) Add "PoE PSU" info into bootlog message.
3) Add utility to verified FPGA related PoE PSU detect function.

Revision 1.2  2012/03/28 00:38:24  mcharon
remove forward slash from second line

Revision 1.1.1.1  2012/03/23 23:02:10  ptong
Initial archive of ng_diag module


$Endlog$
*/
