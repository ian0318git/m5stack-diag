/* $Id: platform_pwr_seq.h,v 1.2 2019/10/17 02:16:26 kehuang2 Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/tabei-l/platform_pwr_seq.h,v $
 *------------------------------------------------------------------
 * Filename:	platform_pwr_seq.h
 *
 * Description: Informers Power Sequencer. This file is  based on EDCS-618748.
 *
 * Copyright (c) 2018-2019 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#ifndef __PLATFORM_PWR_SEQ_H__
#define __PLATFORM_PWR_SEQ_H__

#include "dev_csco_10698.h"


/* Common defines */
#define BUF_SIZE                 256
#define PWR_SER_PATTERN          0xA56C

#define PWR_SEQ_RTC_TEST_TIME	10	/* Run Time Counter wait time */
#define PWR_SEQ_STAT_CHECK_FAIL	(FAILED + 1)
#define PWR_SEQ_RUNTIME_RETRY	3	/* Wraparound retry count */
#define PWR_SEQ_BUF_SIZE 32

/* Utah, Sword, Dagger VREF: 2.702 */
#define PWR_SEQ_VOLT_PER_TICK   0.000659668  /* (VREF/4096) */
#define MARGIN_DELAY_1V_1_2V    14 /* in secs */
#define MARGIN_DELAY_DDR        2  /* in secs */
#define SREC_LINE_LEN           1024 /* in bytes */

#define PWR_SEQ_REV	   	    0x00
#define PWR_SEQ_PIN_CODE   	0x01
#define PWR_SEQ_STA_S	   	0x02
#define PWR_SEQ_STA_C	   	0x03
#define PWR_SEQ_F0_S	   	0x04
#define PWR_SEQ_F1_S	   	0x05
#define PWR_SEQ_F0_C	   	0x06
#define PWR_SEQ_F1_C	   	0x07
#define PWR_SEQ_SCR0	   	0x08
#define PWR_SEQ_SCR1	   	0x09
#define PWR_SEQ_SCR2	   	0x0A
#define PWR_SEQ_SCR3	   	0x0B
#define PWR_SEQ_FW_UP_ST 	0x0C
#define PWR_SEQ_RTC_YM	   	0x0D
#define PWR_SEQ_RTC_DH	   	0x0E
#define PWR_SEQ_RTC_MS		0x0F
#define PWR_SEQ_BB_C		0x10
#define PWR_SEQ_BB_D0		0x11
#define PWR_SEQ_BB_D1		0x12
#define PWR_SEQ_BB_D2		0x13
#define PWR_SEQ_BB_D3		0x14
#define PWR_SEQ_SW_RST      0x15
#define PWR_SEQ_WD_EN		0x1A
#define PWR_SEQ_WD_REF		0x1B
#define PWR_SEQ_WD_TO		0x1C
#define PWR_SEQ_PR_D_EN		0x1D
#define PWR_SEQ_PW_D_T		0x1E
#define PWR_SEQ_PM		    0x1F
#define PWR_SEQ_12_0_LR	    0x20
#define PWR_SEQ_12_0_MAX	0x21
#define PWR_SEQ_12_0_MIN	0x22
#define PWR_SEQ_12_0_RPT	0x23
#define PWR_SEQ_12_0_C_PSU1_LR	0x24
#define PWR_SEQ_12_0_C_PSU1_MAX	0x25
#define PWR_SEQ_12_0_C_PSU1_MIN	0x26
#define PWR_SEQ_12_0_C_PSU1_RPT	0x27
#define PWR_SEQ_5_0_LR		0x28
#define PWR_SEQ_5_0_MAX		0x29
#define PWR_SEQ_5_0_MIN		0x2A
#define PWR_SEQ_5_0_RPT		0x2B
#define PWR_SEQ_3_3_CP_CPU_LR	0x2C
#define PWR_SEQ_3_3_CP_CPU_MAX	0x2D
#define PWR_SEQ_3_3_CP_CPU_MIN	0x2E
#define PWR_SEQ_3_3_CP_CPU_RPT	0x2F
#define PWR_SEQ_3_3_LR		0x30
#define PWR_SEQ_3_3_MAX		0x31
#define PWR_SEQ_3_3_MIN		0x32
#define PWR_SEQ_3_3_RPT		0x33
#define PWR_SEQ_3_0_LR		0x34
#define PWR_SEQ_3_0_MAX		0x35
#define PWR_SEQ_3_0_MIN		0x36
#define PWR_SEQ_3_0_RPT		0x37
#define PWR_SEQ_2_5_LR		0x38
#define PWR_SEQ_2_5_MAX		0x39
#define PWR_SEQ_2_5_MIN		0x3A
#define PWR_SEQ_2_5_RPT		0x3B
#define PWR_SEQ_1_95_CPU_LR	0x3C 
#define PWR_SEQ_1_95_CPU_MAX	0x3D
#define PWR_SEQ_1_95_CPU_MIN	0x3E
#define PWR_SEQ_1_95_CPU_RPT	0x3F
#define PWR_SEQ_1_8_LR		0x40
#define PWR_SEQ_1_8_MAX		0x41
#define PWR_SEQ_1_8_MIN		0x42
#define PWR_SEQ_1_8_RPT		0x43
#define PWR_SEQ_1_7_LR		0x44
#define PWR_SEQ_1_7_MAX		0x45
#define PWR_SEQ_1_7_MIN		0x46
#define PWR_SEQ_1_7_RPT		0x47
#define PWR_SEQ_1_5_LR		0x48
#define PWR_SEQ_1_5_MAX		0x49
#define PWR_SEQ_1_5_MIN		0x4A
#define PWR_SEQ_1_5_RPT		0x4B
#define PWR_SEQ_1_3_LR		0x4C
#define PWR_SEQ_1_3_MAX		0x4D
#define PWR_SEQ_1_3_MIN		0x4E
#define PWR_SEQ_1_3_RPT		0x4F
#define PWR_SEQ_1_2_LR		0x50
#define PWR_SEQ_1_2_MAX		0x51
#define PWR_SEQ_1_2_MIN		0x52
#define PWR_SEQ_1_2_RPT		0x53
#define PWR_SEQ_12_0_C_PSU2_LR	0x54 
#define PWR_SEQ_12_0_C_PSU2_MAX	0x55
#define PWR_SEQ_12_0_C_PSU2_MIN	0x56
#define PWR_SEQ_12_0_C_PSU2_RPT	0x57
#define PWR_SEQ_1_05_LR		0x58
#define PWR_SEQ_1_05_MAX	0x59
#define PWR_SEQ_1_05_MIN	0x5A
#define PWR_SEQ_1_05_RPT	0x5B
#define PWR_SEQ_1_05_SUS_LR	0x5C
#define PWR_SEQ_1_05_SUS_MAX	0x5D
#define PWR_SEQ_1_05_SUS_MIN	0x5E
#define PWR_SEQ_1_05_SUS_RPT	0x5F
#define PWR_SEQ_1_0_LR		0x60
#define PWR_SEQ_1_0_MAX		0x61
#define PWR_SEQ_1_0_MIN		0x62
#define PWR_SEQ_1_0_RPT		0x63
#define PWR_SEQ_0_6_CP_LR	0x64
#define PWR_SEQ_0_6_CP_MAX	0x65
#define PWR_SEQ_0_6_CP_MIN	0x66
#define PWR_SEQ_0_6_CP_RPT	0x67
#define PWR_SEQ_NIM_CR_LR	0x68
#define PWR_SEQ_NIM_CR_MAX	0x69
#define PWR_SEQ_NIM_CR_MIN	0x6A
#define PWR_SEQ_NIM_CR_RPT	0x6B
#define PWR_SEQ_FAN_CR_LR	0x6C
#define PWR_SEQ_FAN_CR_MAX	0x6D
#define PWR_SEQ_FAN_CR_MIN	0x6E
#define PWR_SEQ_FAN_CR_RPT	0x6F
#define PWR_SEQ_3_3_STV_LR	0x70
#define PWR_SEQ_3_3_STV_MAX	0x71
#define PWR_SEQ_3_3_STV_MIN	0x72
#define PWR_SEQ_3_3_STV_RPT	0x73
#define PWR_SEQ_DEBUG_LR	0x74
#define PWR_SEQ_DEBUG_MAX	0x75
#define PWR_SEQ_DEBUG_MIN	0x76
#define PWR_SEQ_DEBUG_RPT	0x77

/*
 * Limit Register - OFF, ON, Over, Under
 */
#define PWR_SEQ_12_0_PW_ON_MIN_THRE  0x88 
#define PWR_SEQ_12_0_PW_OFF_MIN_THRE 0x89
#define PWR_SEQ_12_0_MAX_OP_THRE     0x8A
#define PWR_SEQ_12_0_MIN_OP_THRE     0x8B

#define PWR_SEQ_12_0_C_PSU1_PW_ON_MIN_THRE  0x8C 
#define PWR_SEQ_12_0_C_PSU1_PW_OFF_MIN_THRE 0x8D
#define PWR_SEQ_12_0_C_PSU1_MAX_OP_THRE     0x8E
#define PWR_SEQ_12_0_C_PSU1_MIN_OP_THRE     0x8F

#define PWR_SEQ_5_0_PW_ON_MIN_THRE  0x90 
#define PWR_SEQ_5_0_PW_OFF_MIN_THRE 0x91
#define PWR_SEQ_5_0_MAX_OP_THRE     0x92
#define PWR_SEQ_5_0_MIN_OP_THRE     0x93

#define PWR_SEQ_3_3_CPU_PW_ON_MIN_THRE  0x94 
#define PWR_SEQ_3_3_CPU_PW_OFF_MIN_THRE 0x95
#define PWR_SEQ_3_3_CPU_MAX_OP_THRE     0x96
#define PWR_SEQ_3_3_CPU_MIN_OP_THRE     0x97

#define PWR_SEQ_3_3_PW_ON_MIN_THRE  0x98 
#define PWR_SEQ_3_3_PW_OFF_MIN_THRE 0x99
#define PWR_SEQ_3_3_MAX_OP_THRE     0x9A
#define PWR_SEQ_3_3_MIN_OP_THRE     0x9B

#define PWR_SEQ_3_0_PW_ON_MIN_THRE  0x9C 
#define PWR_SEQ_3_0_PW_OFF_MIN_THRE 0x9D
#define PWR_SEQ_3_0_MAX_OP_THRE     0x9E
#define PWR_SEQ_3_0_MIN_OP_THRE     0x9F

#define PWR_SEQ_2_5_PW_ON_MIN_THRE  0xA0 
#define PWR_SEQ_2_5_PW_OFF_MIN_THRE 0xA1
#define PWR_SEQ_2_5_MAX_OP_THRE     0xA2
#define PWR_SEQ_2_5_MIN_OP_THRE     0xA3

#define PWR_SEQ_1_95_CPU_PW_ON_MIN_THRE  0xA4 
#define PWR_SEQ_1_95_CPU_PW_OFF_MIN_THRE 0xA5
#define PWR_SEQ_1_95_CPU_MAX_OP_THRE     0xA6
#define PWR_SEQ_1_95_CPU_MIN_OP_THRE     0xA7

#define PWR_SEQ_1_8_PW_ON_MIN_THRE  0xA8 
#define PWR_SEQ_1_8_PW_OFF_MIN_THRE 0xA9
#define PWR_SEQ_1_8_MAX_OP_THRE     0xAA
#define PWR_SEQ_1_8_MIN_OP_THRE     0xAB

#define PWR_SEQ_1_7_PW_ON_MIN_THRE  0xAC 
#define PWR_SEQ_1_7_PW_OFF_MIN_THRE 0xAD
#define PWR_SEQ_1_7_MAX_OP_THRE     0xAE
#define PWR_SEQ_1_7_MIN_OP_THRE     0xAF

#define PWR_SEQ_1_5_PW_ON_MIN_THRE  0xB0 
#define PWR_SEQ_1_5_PW_OFF_MIN_THRE 0xB1
#define PWR_SEQ_1_5_MAX_OP_THRE     0xB2
#define PWR_SEQ_1_5_MIN_OP_THRE     0xB3

#define PWR_SEQ_1_3_PW_ON_MIN_THRE  0xB4 
#define PWR_SEQ_1_3_PW_OFF_MIN_THRE 0xB5
#define PWR_SEQ_1_3_MAX_OP_THRE     0xB6
#define PWR_SEQ_1_3_MIN_OP_THRE     0xB7

#define PWR_SEQ_1_2_PW_ON_MIN_THRE  0xB8 
#define PWR_SEQ_1_2_PW_OFF_MIN_THRE 0xB9
#define PWR_SEQ_1_2_MAX_OP_THRE     0xBA
#define PWR_SEQ_1_2_MIN_OP_THRE     0xBB

#define PWR_SEQ_12_0_C_PSU2_PW_ON_MIN_THRE  0xBC 
#define PWR_SEQ_12_0_C_PSU2_PW_OFF_MIN_THRE 0xBD
#define PWR_SEQ_12_0_C_PSU2_MAX_OP_THRE     0xBE
#define PWR_SEQ_12_0_C_PSU2_MIN_OP_THRE     0xBF

#define PWR_SEQ_1_05_PW_ON_MIN_THRE  0xC0 
#define PWR_SEQ_1_05_PW_OFF_MIN_THRE 0xC1
#define PWR_SEQ_1_05_MAX_OP_THRE     0xC2
#define PWR_SEQ_1_05_MIN_OP_THRE     0xC3

#define PWR_SEQ_1_05_SUS_PW_ON_MIN_THRE  0xC4 
#define PWR_SEQ_1_05_SUS_PW_OFF_MIN_THRE 0xC5
#define PWR_SEQ_1_05_SUS_MAX_OP_THRE     0xC6
#define PWR_SEQ_1_05_SUS_MIN_OP_THRE     0xC7

#define PWR_SEQ_1_0_PW_ON_MIN_THRE  0xC8 
#define PWR_SEQ_1_0_PW_OFF_MIN_THRE 0xC9
#define PWR_SEQ_1_0_MAX_OP_THRE     0xCA
#define PWR_SEQ_1_0_MIN_OP_THRE     0xCB

#define PWR_SEQ_0_6_CPU_PW_ON_MIN_THRE  0xCC 
#define PWR_SEQ_0_6_CPU_PW_OFF_MIN_THRE 0xCD
#define PWR_SEQ_0_6_CPU_MAX_OP_THRE     0xCE
#define PWR_SEQ_0_6_CPU_MIN_OP_THRE     0xCF

#define PWR_SEQ_NIM_CR_PW_ON_MIN_THRE  0xD0
#define PWR_SEQ_NIM_CR_PW_OFF_MIN_THRE 0xD1
#define PWR_SEQ_NIM_CR_MAX_OP_THRE     0xD2
#define PWR_SEQ_NIM_CR_MIN_OP_THRE     0xD3

#define PWR_SEQ_FAN_CR_PW_ON_MIN_THRE  0xD4
#define PWR_SEQ_FAN_CR_PW_OFF_MIN_THRE 0xD5
#define PWR_SEQ_FAN_CR_MAX_OP_THRE     0xD6
#define PWR_SEQ_FAN_CR_MIN_OP_THRE     0xD7

#define PWR_SEQ_3_3_STV_PW_ON_MIN_THRE  0xD8
#define PWR_SEQ_3_3_STV_PW_OFF_MIN_THRE 0xD9
#define PWR_SEQ_3_3_STV_MAX_OP_THRE     0xDA
#define PWR_SEQ_3_3_STV_MIN_OP_THRE     0xDB

#define PWR_SEQ_DEBUG_PW_ON_MIN_THRE  0xDC
#define PWR_SEQ_DEBUG PW_OFF_MIN_THRE 0xDD
#define PWR_SEQ_DEBUG_MAX_OP_THRE     0xDE
#define PWR_SEQ_DEBUG_MIN_OP_THRE     0xDF
/* 0xE0 - 0xFF reserved */


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
#define PS_STAT_12V_SHORT_DET_MSK        0x0C00 /* 12V short detect during pwoer up */
#define PS_STAT_12V_SHORT_DET_SHIFT          10 /* 12v short detect during power up  Shift */
#define PS_STAT_VTG_FAULT_CNTR_MSK       0x0300 /* Voltage Fault Counter */
#define PS_STAT_VTG_FAULT_CNTR_SHIFT          8 /* Voltage Fault Counter Shift */
#define PS_STAT_VTG_FAULT_PU_MSK         0x0020 /* Neptune - Voltage Fault During Power Up */
#define PS_STAT_VTG_FAULT_OPT_MSK        0x0010	/* Neptune - Voltage Fault During Operation */
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
Revision 1.2  2019/10/17 02:16:26  kehuang2
Collapse Tabei-L into main trunk

Revision 1.1.2.3  2019/09/27 07:57:24  kehuang2
Clean up code

Revision 1.1.2.2  2018/12/22 07:20:12  olin2
Clean up code

Revision 1.1.2.1  2018/10/25 09:55:24  harrchan
Add MCU utility in I2C utility

*/
