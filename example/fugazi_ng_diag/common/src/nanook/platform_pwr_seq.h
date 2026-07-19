/* $Id: platform_pwr_seq.h,v 1.2 2019/12/11 10:10:34 lucywang Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/nanook/platform_pwr_seq.h,v $
 *------------------------------------------------------------------
 * Filename:    platform_pwr_seq.h
 *
 * Description: Informers Power Sequencer. This file is  based on EDCS-618748.
 *
 * Copyright (c) 2019 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#ifndef __PLATFORM_PWR_SEQ_H__
#define __PLATFORM_PWR_SEQ_H__

#include "dev_csco_10698.h"

/* HW Version */
#define Nanook_PLUS_HW_VERSION_P1	1
#define Nanook_PLUS_HW_VERSION_P2	2
#define Nanook_HW_VERSION_P1		3
#define Nanook_HW_VERSION_P2		4

/* Common defines */
#define BUF_SIZE                 256

#define PWR_SEQ_RTC_TEST_TIME    10    /* Run Time Counter wait time */
#define PWR_SEQ_STAT_CHECK_FAIL    (FAILED + 1)
#define PWR_SEQ_RUNTIME_RETRY    3    /* Wraparound retry count */
#define PWR_SEQ_BUF_SIZE 32

/* Utah, Sword, Dagger VREF: 2.702 */
#define PWR_SEQ_VOLT_PER_TICK   0.000659668  /* (VREF/4096) */
#define MARGIN_DELAY_1V_1_2V    14 /* in secs */
#define MARGIN_DELAY_DDR        2  /* in secs */
#define SREC_LINE_LEN           1024 /* in bytes */

#define PWR_SEQ_REV               0x00
#define PWR_SEQ_PIN_CODE       0x01
#define PWR_SEQ_STA_S           0x02
#define PWR_SEQ_STA_C           0x03
#define PWR_SEQ_F0_S           0x04
#define PWR_SEQ_F1_S           0x05
#define PWR_SEQ_F0_C           0x06
#define PWR_SEQ_F1_C           0x07
#define PWR_SEQ_SCR0           0x08
#define PWR_SEQ_SCR1           0x09
#define PWR_SEQ_SCR2           0x0A
#define PWR_SEQ_SCR3           0x0B
#define PWR_SEQ_FW_UP_ST     0x0C
#define PWR_SEQ_RTC_YM           0x0D
#define PWR_SEQ_RTC_DH           0x0E
#define PWR_SEQ_RTC_MS        0x0F
#define PWR_SEQ_BB_C        0x10
#define PWR_SEQ_BB_D0        0x11
#define PWR_SEQ_BB_D1        0x12
#define PWR_SEQ_BB_D2        0x13
#define PWR_SEQ_BB_D3        0x14
#define PWR_SEQ_SW_RST      0x15
#define PWR_SEQ_WD_EN        0x1A
#define PWR_SEQ_WD_REF        0x1B
#define PWR_SEQ_WD_TO        0x1C
#define PWR_SEQ_PR_D_EN        0x1D
#define PWR_SEQ_PW_D_T        0x1E
#define PWR_SEQ_PM            0x1F

#define PWR_SEQ_P3V3_MCU_LR        0x20
#define PWR_SEQ_P3V3_MCU_MAX    0x21
#define PWR_SEQ_P3V3_MCU_MIN    0x22
#define PWR_SEQ_P3V3_MCU_RPT    0x23
#define PWR_SEQ_P12V0_STBY_LR    0x24
#define PWR_SEQ_P12V0_STBY_MAX    0x25
#define PWR_SEQ_P12V0_STBY_MIN    0x26
#define PWR_SEQ_P12V0_STBY_RPT    0x27
#define PWR_SEQ_P5V0_STBY_LR    0x28
#define PWR_SEQ_P5V0_STBY_MAX    0x29
#define PWR_SEQ_P5V0_STBY_MIN    0x2A
#define PWR_SEQ_P5V0_STBY_RPT    0x2B
#define PWR_SEQ_P3V3_STBY_LR    0x2C
#define PWR_SEQ_P3V3_STBY_MAX    0x2D
#define PWR_SEQ_P3V3_STBY_MIN    0x2E
#define PWR_SEQ_P3V3_STBY_RPT    0x2F
#define PWR_SEQ_P1V8_STBY_LR    0x30
#define PWR_SEQ_P1V8_STBY_MAX    0x31
#define PWR_SEQ_P1V8_STBY_MIN    0x32
#define PWR_SEQ_P1V8_STBY_RPT    0x33
#define PWR_SEQ_PVNN_LR            0x34
#define PWR_SEQ_PVNN_MAX        0x35
#define PWR_SEQ_PVNN_MIN        0x36
#define PWR_SEQ_PVNN_RPT        0x37
#define PWR_SEQ_P1V05_LR        0x38
#define PWR_SEQ_P1V05_MAX        0x39
#define PWR_SEQ_P1V05_MIN        0x3A
#define PWR_SEQ_P1V05_RPT        0x3B
#define PWR_SEQ_P1V02_ETH_LR    0x3C 
#define PWR_SEQ_P1V02_ETH_MAX    0x3D
#define PWR_SEQ_P1V02_ETH_MIN    0x3E
#define PWR_SEQ_P1V02_ETH_RPT    0x3F
#define PWR_SEQ_PVPP_LR            0x40
#define PWR_SEQ_PVPP_MAX        0x41
#define PWR_SEQ_PVPP_MIN        0x42
#define PWR_SEQ_PVPP_RPT        0x43
#define PWR_SEQ_P1V2_VDDQ_LR    0x44
#define PWR_SEQ_P1V2_VDDQ_MAX    0x45
#define PWR_SEQ_P1V2_VDDQ_MIN    0x46
#define PWR_SEQ_P1V2_VDDQ_RPT    0x47
#define PWR_SEQ_PVTT_LR            0x48
#define PWR_SEQ_PVTT_MAX        0x49
#define PWR_SEQ_PVTT_MIN        0x4A
#define PWR_SEQ_PVTT_RPT        0x4B
#define PWR_SEQ_PVCCREF_LR        0x4C
#define PWR_SEQ_PVCCREF_MAX        0x4D
#define PWR_SEQ_PVCCREF_MIN        0x4E
#define PWR_SEQ_PVCCREF_RPT        0x4F
#define PWR_SEQ_PVCCSRAM_LR        0x50
#define PWR_SEQ_PVCCSRAM_MAX    0x51
#define PWR_SEQ_PVCCSRAM_MIN    0x52
#define PWR_SEQ_PVCCSRAM_RPT    0x53
#define PWR_SEQ_PVCCP_LR        0x54 
#define PWR_SEQ_PVCCP_MAX        0x55
#define PWR_SEQ_PVCCP_MIN        0x56
#define PWR_SEQ_PVCCP_RPT        0x57
#define PWR_SEQ_P1V0_A7_FPGA_LR                0x58
#define PWR_SEQ_P1V0_A7_FPGA_MAX            0x59
#define PWR_SEQ_P1V0_A7_FPGA_MIN            0x5A
#define PWR_SEQ_P1V0_A7_FPGA_RPT            0x5B
#define PWR_SEQ_P1V0_A7_MGTAVCC_FPGA_LR        0x5C
#define PWR_SEQ_P1V0_A7_MGTAVCC_FPGA_MAX    0x5D
#define PWR_SEQ_P1V0_A7_MGTAVCC_FPGA_MIN    0x5E
#define PWR_SEQ_P1V0_A7_MGTAVCC_FPGA_RPT    0x5F
#define PWR_SEQ_P1V2_A7_MGTAVTT_FPGA_LR        0x60
#define PWR_SEQ_P1V2_A7_MGTAVTT_FPGA_MAX    0x61
#define PWR_SEQ_P1V2_A7_MGTAVTT_FPGA_MIN    0x62
#define PWR_SEQ_P1V2_A7_MGTAVTT_FPGA_RPT    0x63
#define PWR_SEQ_P1V8_A7_FPGA_LR                0x64
#define PWR_SEQ_P1V8_A7_FPGA_MAX            0x65
#define PWR_SEQ_P1V8_A7_FPGA_MIN            0x66
#define PWR_SEQ_P1V8_A7_FPGA_RPT            0x67
#define PWR_SEQ_DB_P1V0_MGTAVCC_LR            0x68
#define PWR_SEQ_DB_P1V0_MGTAVCC_MAX            0x69
#define PWR_SEQ_DB_P1V0_MGTAVCC_MIN            0x6A
#define PWR_SEQ_DB_P1V0_MGTAVCC_RPT            0x6B
#define PWR_SEQ_DB_P1V2_MGTAVTT_LR            0x6C
#define PWR_SEQ_DB_P1V2_MGTAVTT_MAX            0x6D
#define PWR_SEQ_DB_P1V2_MGTAVTT_MIN            0x6E
#define PWR_SEQ_DB_P1V2_MGTAVTT_RPT            0x6F
#define PWR_SEQ_DEBUG_LR    0x70
#define PWR_SEQ_DEBUG_MAX    0x71
#define PWR_SEQ_DEBUG_MIN    0x72
#define PWR_SEQ_DEBUG_RPT    0x73
#define PWR_SEQ_P0V9_ETH_LR    0x74
#define PWR_SEQ_P0V9_ETH_MAX    0x75
#define PWR_SEQ_P0V9_ETH_MIN    0x76
#define PWR_SEQ_P0V9_ETH_RPT    0x77
#define PWR_SEQ_P1V0_ETH_LR    0x78
#define PWR_SEQ_P1V0_ETH_MAX    0x79
#define PWR_SEQ_P1V0_ETH_MIN    0x7A
#define PWR_SEQ_P1V0_ETH_RPT    0x7B
#define PWR_SEQ_P1V2_CIV_SM_FPGA_LR    0x7C
#define PWR_SEQ_P1V2_CIV_SM_FPGA_MAX    0x7D
#define PWR_SEQ_P1V2_CIV_SM_FPGA_MIN    0x7E
#define PWR_SEQ_P1V2_CIV_SM_FPGA_RPT    0x7F
#define PWR_SEQ_P3V3_A7_FPGA_LR    0x80
#define PWR_SEQ_P3V3_A7_FPGA_MAX    0x81
#define PWR_SEQ_P3V3_A7_FPGA_MIN    0x82
#define PWR_SEQ_P3V3_A7_FPGA_RPT    0x83
//0x84~0x87 Temperature

/*
 * Limit Register - OFF, ON, Over, Under
 */
#define PWR_SEQ_P3V3_MCU_PW_ON_MIN_THRE   0x88
#define PWR_SEQ_P3V3_MCU_PW_OFF_MIN_THRE  0x89
#define PWR_SEQ_P3V3_MCU_MAX_OP_THRE      0x8A
#define PWR_SEQ_P3V3_MCU_MIN_OP_THRE      0x8B
#define PWR_SEQ_P12V0_PW_ON_MIN_THRE   0x8C
#define PWR_SEQ_P12V0_PW_OFF_MIN_THRE  0x8D
#define PWR_SEQ_P12V0_MAX_OP_THRE      0x8E
#define PWR_SEQ_P12V0_MIN_OP_THRE      0x8F
#define PWR_SEQ_P5V0_PW_ON_MIN_THRE   0x90
#define PWR_SEQ_P5V0_PW_OFF_MIN_THRE  0x91
#define PWR_SEQ_P5V0_MAX_OP_THRE      0x92
#define PWR_SEQ_P5V0_MIN_OP_THRE      0x93
#define PWR_SEQ_P3V3_STBY_PW_ON_MIN_THRE   0x94
#define PWR_SEQ_P3V3_STBY_PW_OFF_MIN_THRE  0x95
#define PWR_SEQ_P3V3_STBY_MAX_OP_THRE      0x96
#define PWR_SEQ_P3V3_STBY_MIN_OP_THRE      0x97
#define PWR_SEQ_P1V8_STBY_PW_ON_MIN_THRE   0x98
#define PWR_SEQ_P1V8_STBY_PW_OFF_MIN_THRE  0x99
#define PWR_SEQ_P1V8_STBY_MAX_OP_THRE      0x9A
#define PWR_SEQ_P1V8_STBY_MIN_OP_THRE      0x9B
#define PWR_SEQ_PVNN_PW_ON_MIN_THRE   0x9C
#define PWR_SEQ_PVNN_PW_OFF_MIN_THRE  0x9D
#define PWR_SEQ_PVNN_MAX_OP_THRE      0x9E
#define PWR_SEQ_PVNN_MIN_OP_THRE      0x9F
#define PWR_SEQ_P1V05_PW_ON_MIN_THRE   0xA0
#define PWR_SEQ_P1V05_PW_OFF_MIN_THRE  0xA1
#define PWR_SEQ_P1V05_MAX_OP_THRE      0xA2
#define PWR_SEQ_P1V05_MIN_OP_THRE      0xA3
#define PWR_SEQ_P1V02_ETH_PW_ON_MIN_THRE   0xA4
#define PWR_SEQ_P1V02_ETH_PW_OFF_MIN_THRE  0xA5
#define PWR_SEQ_P1V02_ETH_MAX_OP_THRE      0xA6
#define PWR_SEQ_P1V02_ETH_MIN_OP_THRE      0xA7
#define PWR_SEQ_PVPP_PW_ON_MIN_THRE   0xA8
#define PWR_SEQ_PVPP_PW_OFF_MIN_THRE  0xA9
#define PWR_SEQ_PVPP_MAX_OP_THRE      0xAA
#define PWR_SEQ_PVPP_MIN_OP_THRE      0xAB
#define PWR_SEQ_P1V2_VDDQ_PW_ON_MIN_THRE   0xAC
#define PWR_SEQ_P1V2_VDDQ_PW_OFF_MIN_THRE  0xAD
#define PWR_SEQ_P1V2_VDDQ_MAX_OP_THRE      0xAE
#define PWR_SEQ_P1V2_VDDQ_MIN_OP_THRE      0xAF
#define PWR_SEQ_PVTT_PW_ON_MIN_THRE   0xB0
#define PWR_SEQ_PVTT_PW_OFF_MIN_THRE  0xB1
#define PWR_SEQ_PVTT_MAX_OP_THRE      0xB2
#define PWR_SEQ_PVTT_MIN_OP_THRE      0xB3
#define PWR_SEQ_PVCCREF_PW_ON_MIN_THRE   0xB4
#define PWR_SEQ_PVCCREF_PW_OFF_MIN_THRE  0xB5
#define PWR_SEQ_PVCCREF_MAX_OP_THRE      0xB6
#define PWR_SEQ_PVCCREF_MIN_OP_THRE      0xB7
#define PWR_SEQ_PVCCSRAM_PW_ON_MIN_THRE   0xB8
#define PWR_SEQ_PVCCSRAM_PW_OFF_MIN_THRE  0xB9
#define PWR_SEQ_PVCCSRAM_MAX_OP_THRE      0xBA
#define PWR_SEQ_PVCCSRAM_MIN_OP_THRE      0xBB
#define PWR_SEQ_PVCCP_PW_ON_MIN_THRE   0xBC
#define PWR_SEQ_PVCCP_PW_OFF_MIN_THRE  0xBD
#define PWR_SEQ_PVCCP_MAX_OP_THRE      0xBE
#define PWR_SEQ_PVCCP_MIN_OP_THRE      0xBF
#define PWR_SEQ_P1V0_A7_FPGA_PW_ON_MIN_THRE   0xC0
#define PWR_SEQ_P1V0_A7_FPGA_PW_OFF_MIN_THRE  0xC1
#define PWR_SEQ_P1V0_A7_FPGA_MAX_OP_THRE      0xC2
#define PWR_SEQ_P1V0_A7_FPGA_MIN_OP_THRE      0xC3
#define PWR_SEQ_P1V0_A7_MGTAVCC_FPGA_PW_ON_MIN_THRE   0xC4
#define PWR_SEQ_P1V0_A7_MGTAVCC_FPGA_PW_OFF_MIN_THRE  0xC5
#define PWR_SEQ_P1V0_A7_MGTAVCC_FPGA_MAX_OP_THRE      0xC6
#define PWR_SEQ_P1V0_A7_MGTAVCC_FPGA_MIN_OP_THRE      0xC7
#define PWR_SEQ_P1V2_A7_MGTAVTT_FPGA_PW_ON_MIN_THRE   0xC8
#define PWR_SEQ_P1V2_A7_MGTAVTT_FPGA_PW_OFF_MIN_THRE  0xC9
#define PWR_SEQ_P1V2_A7_MGTAVTT_FPGA_MAX_OP_THRE      0xCA
#define PWR_SEQ_P1V2_A7_MGTAVTT_FPGA_MIN_OP_THRE      0xCB
#define PWR_SEQ_P1V8_A7_FPGA_PW_ON_MIN_THRE   0xCC
#define PWR_SEQ_P1V8_A7_FPGA_PW_OFF_MIN_THRE  0xCD
#define PWR_SEQ_P1V8_A7_FPGA_MAX_OP_THRE      0xCE
#define PWR_SEQ_P1V8_A7_FPGA_MIN_OP_THRE      0xCF
#define PWR_SEQ_DB_P1V0_MGTAVCC_PW_ON_MIN_THRE   0xD0
#define PWR_SEQ_DB_P1V0_MGTAVCC_PW_OFF_MIN_THRE  0xD1
#define PWR_SEQ_DB_P1V0_MGTAVCC_MAX_OP_THRE      0xD2
#define PWR_SEQ_DB_P1V0_MGTAVCC_MIN_OP_THRE      0xD3
#define PWR_SEQ_DB_P1V2_MGTAVTT_PW_ON_MIN_THRE   0xD4
#define PWR_SEQ_DB_P1V2_MGTAVTT_PW_OFF_MIN_THRE  0xD5
#define PWR_SEQ_DB_P1V2_MGTAVTT_MAX_OP_THRE      0xD6
#define PWR_SEQ_DB_P1V2_MGTAVTT_MIN_OP_THRE      0xD7
#define PWR_SEQ_DEBUG_PW_ON_MIN_THRE   0xD8
#define PWR_SEQ_DEBUG_PW_OFF_MIN_THRE  0xD9
#define PWR_SEQ_DEBUG_MAX_OP_THRE      0xDA
#define PWR_SEQ_DEBUG_MIN_OP_THRE      0xDB
#define PWR_SEQ_P0V9_ETH_PW_ON_MIN_THRE   0xDC
#define PWR_SEQ_P0V9_ETH_PW_OFF_MIN_THRE  0xDD
#define PWR_SEQ_P0V9_ETH_MAX_OP_THRE      0xDE
#define PWR_SEQ_P0V9_ETH_MIN_OP_THRE      0xDF
#define PWR_SEQ_P1V0_ETH_PW_ON_MIN_THRE   0xE0
#define PWR_SEQ_P1V0_ETH_PW_OFF_MIN_THRE  0xE1
#define PWR_SEQ_P1V0_ETH_MAX_OP_THRE      0xE2
#define PWR_SEQ_P1V0_ETH_MIN_OP_THRE      0xE3
#define PWR_SEQ_P1V2_CIV_SM_FPGA_PW_ON_MIN_THRE   0xE4
#define PWR_SEQ_P1V2_CIV_SM_FPGA_PW_OFF_MIN_THRE  0xE5
#define PWR_SEQ_P1V2_CIV_SM_FPGA_MAX_OP_THRE      0xE6
#define PWR_SEQ_P1V2_CIV_SM_FPGA_MIN_OP_THRE      0xE7
#define PWR_SEQ_P3V3_A7_FPGA_PW_ON_MIN_THRE   0xE8
#define PWR_SEQ_P3V3_A7_FPGA_PW_OFF_MIN_THRE  0xE9
#define PWR_SEQ_P3V3_A7_FPGA_MAX_OP_THRE      0xEA
#define PWR_SEQ_P3V3_A7_FPGA_MIN_OP_THRE      0XEB

/* Power sequencer FW upgrade */
#define PWR_SEQ_FW_CMD_REG  0xFE
#define PWR_SEQ_FW_DATA_REG 0xFF
#define PWR_SEQ_CMD_UPDATE  0xFFEE
#define PWR_SEQ_CMD_REBOOT  0xFFED
#define PWR_SEQ_CMD_MAX_ADDR    0xFBFF
#define PWR_SEQ_EEPROM_END  0xFC00

/* NIOS Mailbox Voltage Register */
#define NIOS_MAILBOX_VOLTAGE_P12V0_STBY_OFFSET           0x34200
#define NIOS_MAILBOX_VOLTAGE_P5V0_STBY_OFFSET            0x34202
#define NIOS_MAILBOX_VOLTAGE_P3V3_MCU_OFFSET             0x34204
#define NIOS_MAILBOX_VOLTAGE_P1V02_ETH_OFFSET            0x34206
#define NIOS_MAILBOX_VOLTAGE_P3V3_STBY_OFFSET            0x34208
#define NIOS_MAILBOX_VOLTAGE_PVNN_OFFSET                 0x3420a
#define NIOS_MAILBOX_VOLTAGE_PVPP_OFFSET                 0x3420c
#define NIOS_MAILBOX_VOLTAGE_PVCCREF_OFFSET              0x3420e
#define NIOS_MAILBOX_VOLTAGE_P1V8_STBY_OFFSET            0x34210
#define NIOS_MAILBOX_VOLTAGE_PVCCSRAM_OFFSET             0x34212
#define NIOS_MAILBOX_VOLTAGE_PVCCP_OFFSET                0x34214
#define NIOS_MAILBOX_VOLTAGE_P1V0_A7_FPGA_OFFSET         0x34216
#define NIOS_MAILBOX_VOLTAGE_P1V2_VDDQ_OFFSET            0x34218
#define NIOS_MAILBOX_VOLTAGE_P1V2_A7_MGTAVTT_OFFSET      0x3421a
#define NIOS_MAILBOX_VOLTAGE_P1V8_A7_FPGA_OFFSET         0x3421c
#define NIOS_MAILBOX_VOLTAGE_P1V05_OFFSET                0x3421e
#define NIOS_MAILBOX_VOLTAGE_P1V0_A7_MGTAVCC_FPGA_OFFSET 0x34220
#define NIOS_MAILBOX_VOLTAGE_PVTT_OFFSET                 0x34222
#define NIOS_MAILBOX_VOLTAGE_DB_P1V2_MGTAVTT_OFFSET      0x34224
#define NIOS_MAILBOX_VOLTAGE_DB_P1V0_MGTAVCC_OFFSET      0x34226

/* NIOS_Mailbox Voltage limit */
#define NIOS_MAILBOX_VOLTAGE_P12V0_STBY_MIN           0x2A30
#define NIOS_MAILBOX_VOLTAGE_P12V0_STBY_MAX           0x3390
#define NIOS_MAILBOX_VOLTAGE_P5V0_STBY_MIN            0x1194
#define NIOS_MAILBOX_VOLTAGE_P5V0_STBY_MAX            0x157C
#define NIOS_MAILBOX_VOLTAGE_P3V3_MCU_MIN             0x0B9A
#define NIOS_MAILBOX_VOLTAGE_P3V3_MCU_MAX             0x0E2E
#define NIOS_MAILBOX_VOLTAGE_P1V02_ETH_MIN            0x0396
#define NIOS_MAILBOX_VOLTAGE_P1V02_ETH_MAX            0x0462
#define NIOS_MAILBOX_VOLTAGE_P3V3_STBY_MIN            0x0B9A
#define NIOS_MAILBOX_VOLTAGE_P3V3_STBY_MAX            0x0E2E
#define NIOS_MAILBOX_VOLTAGE_PVNN_MIN                 0x028A
#define NIOS_MAILBOX_VOLTAGE_PVNN_MAX                 0x04D8
#define NIOS_MAILBOX_VOLTAGE_PVPP_MIN                 0x08CA
#define NIOS_MAILBOX_VOLTAGE_PVPP_MAX                 0x0ABE
#define NIOS_MAILBOX_VOLTAGE_PVCCREF_MIN              0x045C
#define NIOS_MAILBOX_VOLTAGE_PVCCREF_MAX              0x0554
#define NIOS_MAILBOX_VOLTAGE_P1V8_STBY_MIN            0x0654
#define NIOS_MAILBOX_VOLTAGE_P1V8_STBY_MAX            0x07BC
#define NIOS_MAILBOX_VOLTAGE_PVCCSRAM_MIN             0x0384
#define NIOS_MAILBOX_VOLTAGE_PVCCSRAM_MAX             0x044C
#define NIOS_MAILBOX_VOLTAGE_PVCCP_MIN                0x0384
#define NIOS_MAILBOX_VOLTAGE_PVCCP_MAX                0x044C
#define NIOS_MAILBOX_VOLTAGE_P1V0_A7_FPGA_MIN         0x0384
#define NIOS_MAILBOX_VOLTAGE_P1V0_A7_FPGA_MAX         0x044C
#define NIOS_MAILBOX_VOLTAGE_P1V2_VDDQ_MIN            0x0438
#define NIOS_MAILBOX_VOLTAGE_P1V2_VDDQ_MAX            0x0528
#define NIOS_MAILBOX_VOLTAGE_P1V2_A7_MGTAVTT_MIN      0x0438
#define NIOS_MAILBOX_VOLTAGE_P1V2_A7_MGTAVTT_MAX      0x0528
#define NIOS_MAILBOX_VOLTAGE_P1V8_A7_FPGA_MIN         0x0654
#define NIOS_MAILBOX_VOLTAGE_P1V8_A7_FPGA_MAX         0x07BC
#define NIOS_MAILBOX_VOLTAGE_P1V05_MIN                0x03B1
#define NIOS_MAILBOX_VOLTAGE_P1V05_MAX                0x0483
#define NIOS_MAILBOX_VOLTAGE_P1V0_A7_MGTAVCC_FPGA_MIN 0x0384
#define NIOS_MAILBOX_VOLTAGE_P1V0_A7_MGTAVCC_FPGA_MAX 0x044C
#define NIOS_MAILBOX_VOLTAGE_PVTT_MIN                 0x021C
#define NIOS_MAILBOX_VOLTAGE_PVTT_MAX                 0x0294
#define NIOS_MAILBOX_VOLTAGE_DB_P1V2_MGTAVTT_MIN      0x0438
#define NIOS_MAILBOX_VOLTAGE_DB_P1V2_MGTAVTT_MAX      0x0528
#define NIOS_MAILBOX_VOLTAGE_DB_P1V0_MGTAVCC_MIN      0x0384
#define NIOS_MAILBOX_VOLTAGE_DB_P1V0_MGTAVCC_MAX      0x044C


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

typedef struct pwr_seq_v_nios_t_ {
    char *volt_p;               /* Voltage text pointer */
    ren_t over_v;               /* Over  Voltage ADC */
    ren_t under_v;              /* Under Voltage ADC */
    uint32_t rt_o;              /* Real time register offset */
    uint32_t max_o;             /* Maximum register offset */
    uint32_t min_o;             /* Minimum register offset */
} pwr_seq_v_nios_t;


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
#define PS_STAT_12V_SHORT_DET_MSK        0x0C00 /* 12V short detect during pwoer up */
#define PS_STAT_12V_SHORT_DET_SHIFT          10 /* 12v short detect during power up  Shift */
#define PS_STAT_VTG_FAULT_CNTR_MSK       0x0300 /* Voltage Fault Counter */
#define PS_STAT_VTG_FAULT_CNTR_SHIFT          8 /* Voltage Fault Counter Shift */
#define PS_STAT_VTG_FAULT_PU_MSK         0x0020 /* Neptune - Voltage Fault During Power Up */
#define PS_STAT_VTG_FAULT_OPT_MSK        0x0010    /* Neptune - Voltage Fault During Operation */
#define PS_STAT_SYS_PWR_DWN_MSK          0x0020    /* System Power Down */
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

/* Power Control Register - 0x48 */
#define PS_PC_POE_PWR_CTRL           0x0010 /* bit[4]: PoE Power Control Bit */
#define PS_PC_POE_PWR_CTRL_SHIFT          4 /* PoE Power Control Bit shift */
#define PS_PC_GSHUT_DWN_TIMER_MSK    0x000F /* bit[3:0]: Graceful Shutdown Timer */

/* The minimum number of bytes in a bootloader command. */
#define BASE_CMD_SIZE           0x07
/* Maximum number of bytes to allocate for a single command.  */
#define MAX_COMMAND_SIZE 512
/* The first byte of any boot loader command. */
#define CMD_START               0x01
/* The last byte of any boot loader command. */
#define CMD_STOP                0x17

#define PWR_EN_FW_UPGRADE_REG 0xC
#define PWR_EN_FW_UPGRADE_VAL 0xA5FA

#define PWR_EN_SW_RESET_REG 0x15
#define PWR_EN_SW_RESET_VAL 0xCA8E

/*
 * This enum defines the different types of checksums that can be
 * used by the bootloader for ensuring data integrety.
 */
typedef enum
{
    /* Checksum type is a basic inverted summation of all bytes */
    SUM_CHECKSUM = 0x00,
    /* 16-bit CRC checksum using the CCITT implementation */
    CRC_CHECKSUM = 0x01,
} CyBtldr_ChecksumType;

/* Command identifier for starting the boot loader.  All other commands ignored until this is sent. */
#define CMD_ENTER_BOOTLOADER    0x38
/* Command identifier for exiting the bootloader and restarting the target program. */
#define CMD_EXIT_BOOTLOADER     0x3B
/* Command identifier for getting the number of flash rows in the target device. */
#define CMD_GET_FLASH_SIZE      0x32
#define CMD_SEND_DATA           0x37
#define CMD_VERIFY_ROW          0x3A
#define CMD_PROGRAM_ROW         0x39
#define CMD_SET_ACTIVE_APP      0x36
#define CMD_GET_APP_STATUS      0x33
#define APP_IS_ACTIVE 0x1
#define APP_IS_VALID  0x0
#define MAX_BUFFER_SIZE 768
/* The bootloader reported an error */
#define CYRET_ERR_BTLDR_MASK    0x4000
#define NO_FLASH_ARRAY_DATA 0
#define MAX_FLASH_ARRAYS 4

/*
 * This struct defines all of the items necessary for the bootloader
 * host to communicate over an arbitrary communication protocol. The
 * caller must provide implementations of these items to use their
 * deisred communication protocol.
 */
typedef struct
{
    /* Function used to open the communications connection */
    int (*OpenConnection)(void);
    /* Function used to close the communications connection */
    int (*CloseConnection)(void);
    /* Function used to read data over the communications connection */
    int (*ReadData)(unsigned char*, int);
    /* Function used to write data over the communications connection */
    int (*WriteData)(unsigned char*, int);
    /* Value used to specify the maximum number of bytes that can be trasfered at a time */
    unsigned int MaxTransferSize;
} CyBtldr_CommunicationsData;

/******************************************************************************
 *    HOST ERROR CODES
 ******************************************************************************
 *
 * Different return codes from the bootloader host.  Functions are not
 * limited to these values, but are encuraged to use them when returning
 * standard error values.
 *
 * 0 is successful, all other values indicate a failure.
 *****************************************************************************/
/* Completed successfully */
#ifndef CYRET_SUCCESS
#define CYRET_SUCCESS           0x00
#endif
/* File is not accessable */
#define CYRET_ERR_FILE          0x01
/* Reached the end of the file */
#define CYRET_ERR_EOF           0x02
/* The amount of data available is outside the expected range */
#define CYRET_ERR_LENGTH        0x03
/* The data is not of the proper form */
#define CYRET_ERR_DATA          0x04
/* The command is not recognized */
#define CYRET_ERR_CMD           0x05
/* The expected device does not match the detected device */
#define CYRET_ERR_DEVICE        0x06
/* The bootloader version detected is not supported */
#define CYRET_ERR_VERSION       0x07
/* The checksum does not match the expected value */
#define CYRET_ERR_CHECKSUM      0x08
/* The flash array is not valid */
#define CYRET_ERR_ARRAY         0x09
/* The flash row is not valid */
#define CYRET_ERR_ROW           0x0A
/* The bootloader is not ready to process data */
#define CYRET_ERR_BTLDR         0x0B
/* The application is currently marked as active */
#define CYRET_ERR_ACTIVE        0x0C
/* An unknown error occured */
#define CYRET_ERR_UNK           0x0F

/* Functions prototype */
extern int init_pwr_seq(int err_log);
extern void build_pwr_tst_menu(int submenu);
extern void build_pwr_seq_menu(int);

#endif /* __PLATFORM_PWR_SEQ_H__ */

/*------------------------------------------------------------------
$Log: platform_pwr_seq.h,v $
Revision 1.2  2019/12/11 10:10:34  lucywang
Merged Nanook to main trunk


*/
