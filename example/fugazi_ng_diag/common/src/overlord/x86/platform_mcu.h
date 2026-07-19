/* $Id: platform_mcu.h,v 1.2 2012/03/28 00:38:23 mcharon Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/overlord/x86/platform_mcu.h,v $
 *-----------------------------------------------------------------------------
 * platform_mcu.h - Include file for Environmental MCU
 *
 * August, 2008,  Ling Lee
 *
 * Copyright (c) 2008-2012 by Cisco Systems, Inc.
 * All rights reserved.
 *-----------------------------------------------------------------------------
 */

#ifndef __PLATFORM_MCU_H__
#define __PLATFORM_MCU_H__

#include "types.h"

#define ENV_MCU_MAX_DATA_SIZE  256

/* ENV MCU command definitions */
#define ENV_MCU_BIT_RATE_9600    0xB0
#define ENV_MCU_BIT_RATE_115200	 0xB4
#define ENV_MCU_BIT_RATE_500000  0xB5
#define ENV_MCU_CLEAR_STATUS     0x50
#define ENV_MCU_VER_INFO         0xFB
#define ENV_MCU_PAGE_PROG        0x41
#define ENV_MCU_ERASE_ALL_BLOCK  0xA7
#define ENV_MCU_TX_CONFIRM       0xD0
#define ENV_MCU_READ_STATUS_REG  0x70
#define ENV_MCU_CLEAR_STATUS_REG 0x50
#define ENV_MCU_PAGE_READ        0xFF
#define ENV_MCU_ID_CHECK         0xF5

/* ENV MCU Download Control register bit definitions */
#define ENV_MCU_CTRL_RESET_EN	0x8000	/* Environmental Reset Enable */
#define ENV_MCU_CTRL_RESET	0x4000	/* Environmental Reset */
#define ENV_MCU_CTRL_FORCE_MODE_LOW 0x0800  /* MCU force Mode low */
#define ENV_MCU_CTRL_DISABLE_RX 0x1000  /* Disable Rx during Tx */
#define ENV_MCU_CTRL_BAUD_RATE_MASK 0x7ff /* Buad rate mask */

/* Buad rate value for Download Control register baud rate field */
/* Based on Overlord HFS(EDCS- 999274), The formula applies:
 * BaudRate = 50MHz / ((register+1)*4).
 * Ex: BaudRate 9600   = 1301,
 *     BaudRate 500000 =   24.
 */
#define ENV_MCU_CTRL_9600_BAUD   1301
#define ENV_MCU_CTRL_115200_BAUD 108
#define ENV_MCU_CTRL_125000_BAUD 99
#define ENV_MCU_CTRL_250000_BAUD 49
#define ENV_MCU_CTRL_500000_BAUD 24

/* ENV MCU Download Status register bit definitions */
#define ENV_MCU_NO_STOP_BIT_RX  0x4     /* No stop bit received */
#define ENV_MCU_RX_DATA         0x2     /* Received Data */
#define ENV_MCU_TX_DONE         0x1     /* Transmit Done */
#define ENV_MCU_TX_RX_DATA      0x00ff  /* Transmit/Receive Data */

/* ENV MCU Download Interrupt Enable register bit definitions */
#define ENV_MCU_RX_INTR_EN      0x02

/* ENV MCU status register bit definitions */
#define ENV_MCU_SRD_SEQUENCE_READY     0x80
#define ENV_MCU_SRD_ERASE_ERROR        0x20
#define ENV_MCU_SRD_PROGRAM_ERROR      0x10
#define ENV_MCU_SRD1_ID_CHECK_MATCH    0x0C
#define ENV_MCU_SRD1_ID_CHECK_MISMATCH 0x04

/* Address bits */
#define ENV_MCU_MID_ORDER_ADDR        0x00FF00
#define ENV_MCU_HIGH_ORDER_ADDR       0xFF0000
#define ENV_MCU_MID_ADDR_SHIFT        8
#define ENV_MCU_HIGH_ADDR_SHIFT       16

/* ENV MCU Internal Flash */
#define ENV_MCU_INTERNAL_FLASH_START_ADDR 0x0E000
#define ENV_MCU_FLASH_LENGTH              0x2000

#define ENV_MCU_RX_DATA_TIMEOUT            5000  /* msecs */
#define ENV_MCU_SEQUENCE_STATUS_TIMEOUT    9000  /* msecs */
#define ENV_MCU_DNLD_ERASE_TIMEOUT        30000  /* Erase time */

/* Renesas Bit Rate Setting command (B5) defines  */
#define ENV_MCU_BR_9615		0x33	/* 9615   */
#define ENV_MCU_BR_19230	0x19	/* 19230  */
#define ENV_MCU_BR_38461	0x0C	/* 38461  */
#define ENV_MCU_BR_55555	0x08	/* 55555  */
#define ENV_MCU_BR_125000	0x03	/* 125000 */
#define ENV_MCU_BR_250000	0x01	/* 250000 */
#define ENV_MCU_BR_500000	0x00	/* 500000 */

#ifndef LINUX_KLM
/* 
 * ENV MCU command structure 
 */
/* Version Information packet structure */
typedef struct env_mcu_ver_info_t_ {
    volatile unsigned char cmd;          /* Command */
    volatile unsigned char buf[8];
} env_mcu_ver_info_t;

/* Page read packet structure */
typedef struct env_mcu_page_read_t_ {
    volatile unsigned char cmd;          /* Command */
    volatile unsigned char mid_addr;     /* Middle-order address */
    volatile unsigned char high_addr;    /* High-order address */
    volatile unsigned char data[256];    /* 256 bytes data */
} env_mcu_page_read_t;

/* Page program packet structure */
typedef struct env_mcu_page_prog_t_ {
    volatile unsigned char cmd;          /* Command */
    volatile unsigned char mid_addr;     /* Middle-order address */
    volatile unsigned char high_addr;    /* High-order address */
    volatile unsigned char data[256];    /* 256 bytes data */
} env_mcu_page_prog_t;

/* Erase all unlocked blocks packet structure */
typedef struct env_mcu_erase_all_blk_t_ {
    volatile unsigned char cmd;          /* Command */
    volatile unsigned char confirm_cmd;  /* Confirmation command */
} env_mcu_erase_all_blk_t;

/* Read Status Register packet structure */
typedef struct env_mcu_read_status_reg_t_ {
    volatile unsigned char cmd;          /* Command */
    volatile unsigned char data[2];      /* Status Register Data */
} env_mcu_read_status_reg_t;

/* Clear Status Register packet structure */
typedef struct env_mcu_clear_status_reg_t_ {
    volatile unsigned char cmd;          /* Command */
} env_mcu_clear_status_reg_t;

/* ID Check */
typedef struct env_mcu_id_check_t_ {
    volatile unsigned char cmd;          /* Command */
    volatile unsigned char low_addr;     /* Low-order address */
    volatile unsigned char mid_addr;     /* Mid-order address */
    volatile unsigned char high_addr;    /* High-order address */
    volatile unsigned char id_size;      /* ID size */
    volatile unsigned char id[7];        /* ID */
} env_mcu_id_check_t;


#endif /* LINUX_KLM */

/*
 * Env MCU Download Control Register (offset 0x00_0094)
 */
#define FPGA_MCU_RESET_ENA	0x8000	/* Environmental Reset Enable */
#define FPGA_MCU_RESET		0x4000	/* Environmental Reset Bit */
#define FPGA_MCU_LPBK		0x2000	/* Enable Loop Back */
#define FPGA_MCU_DIS_RX_ON_TX	0x1000	/* Disable Rx During Tx */
#define FPGA_MCU_FORCE_MODE_LOW	0x0800	/* Force MCU Mode Pin Low */
#define FPGA_MCU_BAUD_RATE_MASK	0x07FF	/* Baud Rate */

/*
 * Env MCU Download Status Register (offset 0x00_0096)
 * Env MCU Download Int Enable Register (offset 0x00_0098)
 */
#define FPGA_MCU_RX_NO_STOP_BIT	0x0004	/* W1C: No Stop Bit Received */
#define FPGA_MCU_RX_DATA	0x0002	/* W1C: Received Data */
#define FPGA_MCU_TX_DONE	0x0001	/* W1C: Transmit Done */

/*
 * Env MCU Download Data Register (offset 0x00_009A)
 */
#define FPGA_MCU_DATA_MASK	0x00FF	/* Tx/Rx Data */

typedef struct mb_iofpga_mcu_regs_t_ {
    volatile uint32_t mcu_dnld_ctrl;          /* 0x0  MCU Dnld Control Reg */
    volatile uint32_t mcu_dnld_status;        /* 0x4  MCU Dnld Status Reg  */
    volatile uint32_t mcu_dnld_intr_enable;   /* 0x8  MCU Intr Enable Reg  */
    volatile uint32_t mcu_dnld_data;          /* 0xC  MCU Data Reg         */
} mb_iofpga_mcu_regs_t;


#endif   /* __PLATFORM_MCU_H__ */

/* ------ End of Module ------ */

/******** History ******** 
$Log: platform_mcu.h,v $
Revision 1.2  2012/03/28 00:38:23  mcharon
remove forward slash from second line

Revision 1.1.1.1  2012/03/23 23:02:09  ptong
Initial archive of ng_diag module


$Endlog$
*/
