/* $Id: diag_mcu_util.h,v 1.2 2016/04/20 11:25:28 benchen2 Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/tachi_bmc/diag_mcu_util.h,v $
 *------------------------------------------------------------------
 *
 * diag_mcu_util.h - Header file for MCU Utility
 *
 * June 2015, Times Huang
 *
 * Copyright (c) 2016 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */
 
#ifndef __DIAG_MCU_UTIL__
#define __DIAG_MCU_UTIL__

extern int diag_mcu_util(void);
extern int diag_mcu_show_ver(void);

/* Page program packet structure */
typedef struct env_mcu_page_prog_t_ {
    volatile unsigned char cmd;          /* Command */
    volatile unsigned char mid_addr;     /* Middle-order address */
    volatile unsigned char high_addr;    /* High-order address */
    volatile unsigned char data[256];    /* 256 bytes data */
} env_mcu_page_prog_t;

/* Read Status Register packet structure */
typedef struct env_mcu_read_status_reg_t_ {
    volatile unsigned char cmd;          /* Command */
    volatile unsigned char data[2];      /* Status Register Data */
} env_mcu_read_status_reg_t;

/* ID Check */
typedef struct env_mcu_id_check_t_ {
    volatile unsigned char cmd;          /* Command */
    volatile unsigned char low_addr;     /* Low-order address */
    volatile unsigned char mid_addr;     /* Mid-order address */
    volatile unsigned char high_addr;    /* High-order address */
    volatile unsigned char id_size;      /* ID size */
    volatile unsigned char id[7];        /* ID */
} env_mcu_id_check_t;



/* Page read packet structure */
typedef struct env_mcu_page_read_t_ {
    volatile unsigned char cmd;          /* Command */
    volatile unsigned char mid_addr;     /* Middle-order address */
    volatile unsigned char high_addr;    /* High-order address */
    volatile unsigned char data[256];    /* 256 bytes data */
} env_mcu_page_read_t;

/* Clear Status Register packet structure */
typedef struct env_mcu_clear_status_reg_t_ {
    volatile unsigned char cmd;          /* Command */
} env_mcu_clear_status_reg_t;


typedef struct mb_iofpga_mcu_regs_t_ {
    volatile uint32_t mcu_dnld_ctrl;          /* 0x0  MCU Dnld Control Reg */
    volatile uint32_t mcu_dnld_status;        /* 0x4  MCU Dnld Status Reg  */
    volatile uint32_t mcu_dnld_intr_enable;   /* 0x8  MCU Intr Enable Reg  */
    volatile uint32_t mcu_dnld_data;          /* 0xC  MCU Data Reg         */
} mb_iofpga_mcu_regs_t;

/* Erase all unlocked blocks packet structure */
typedef struct env_mcu_erase_all_blk_t_ {
    volatile unsigned char cmd;          /* Command */
    volatile unsigned char confirm_cmd;  /* Confirmation command */
} env_mcu_erase_all_blk_t;

#define ENV_MCU_CTRL_RESET_EN	0x8000	/* Environmental Reset Enable */
#define ENV_MCU_CTRL_DISABLE_RX 0x1000  /* Disable Rx during Tx */
#define ENV_MCU_CTRL_RESET	0x4000	/* Environmental Reset */
#define DEFAULT_IRQ INTR_ENV_MCU
#define INTR_ENV_MCU 37
#define INTR_VM_MCU 38
#define ENV_MCU_MAX_DATA_SIZE  256
#define ENV_MCU_CTRL_FORCE_MODE_LOW 0x0800  /* MCU force Mode low */
#define ENV_MCU_CTRL_BAUD_RATE_MASK 0x7ff /* Buad rate mask */
#define ENV_MCU_CTRL_9600_BAUD   1301
#define ENV_MCU_NO_STOP_BIT_RX  0x4     /* No stop bit received */
/* ENV MCU Download Interrupt Enable register bit definitions */
#define ENV_MCU_RX_INTR_EN      0x02
#define MCU_BAUDRATE 9600
#define ENV_MCU_ID_CHECK         0xF5
#define ENV_MCU_RX_DATA_TIMEOUT            5000  /* msecs */
#define ENV_MCU_SRD1_ID_CHECK_MATCH    0x0C
#define ENV_MCU_DNLD_ERASE_TIMEOUT        30000  /* Erase time */
#define ENV_MCU_SRD_ERASE_ERROR        0x20
#define ENV_MCU_SRD_SEQUENCE_READY     0x80
#define ENV_MCU_ERASE_ALL_BLOCK  0xA7
#define ENV_MCU_TX_CONFIRM       0xD0
#define ENV_MCU_PAGE_PROG        0x41
#define ENV_MCU_MID_ORDER_ADDR        0x00FF00
#define ENV_MCU_MID_ADDR_SHIFT        8
#define ENV_MCU_HIGH_ADDR_SHIFT       16
#define ENV_MCU_HIGH_ORDER_ADDR       0xFF0000
#define FLASH_RD_WR_DELAY 50
#define ENV_MCU_SRD_PROGRAM_ERROR      0x10
#define ENV_MCU_CLEAR_STATUS_REG 0x50
#define ENV_MCU_READ_STATUS_REG  0x70
#define ENV_MCU_FLASH_LENGTH              0x2000
#define ENV_MCU_PAGE_READ        0xFF

#endif /* __DIAG_MCU_UTIL__ */

/*---------------------------------------------------------------
$Log: diag_mcu_util.h,v $
Revision 1.2  2016/04/20 11:25:28  benchen2
add tachi fru portion

Revision 1.1.2.3  2015/09/25 02:18:24  tirawan
Correct MCU reg read/write (not to byte swap) and display MCU version

Revision 1.1.2.2  2015/07/31 07:39:50  hondwang
mcu r,w, upgrade

Revision 1.1.2.1  2015/06/11 02:01:09  tirawan
Add files for Tachi BMC project


$Endlog$
*/
