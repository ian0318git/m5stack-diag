/* $Id: sm_csrs_top.h,v 1.1 2020/05/22 02:28:45 qingcwan Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/switzer/csrs/sm_csrs_top.h,v $
 *------------------------------------------------------------------
 *
 * sm_csrs_top.h - Switzer-carrier FPGA source.
 *
 * Mar. 2019, Shiyu Wu <shiywu@cisco.com>
 *
 * Copyright (c) 2019 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#ifndef __SM_CSRS_TOP_H__
#define __SM_CSRS_TOP_H__

#include "nim_te_csrs_top.h"

#define SWITZER_CARRIER_MAX_SLOT 3
#define SWITZER_CARRIER_SLOT_NUM (SWITZER_CARRIER_MAX_SLOT - 1)

struct switzer_ng_t {
#define SWITZER_NGIO_HDLS_MODE   0x40000
#define SWITZER_NGIO_PWR_FLT_OVR 0x20000
#define SWITZER_NGIO_PWR_OK      0x10000
#define SWITZER_NGIO_FLT_INTR    0x400
#define SWITZER_NGIO_INS_INTR    0x200
#define SWITZER_NGIO_RMV_INTR    0x100
#define SWITZER_NGIO_PRSNT       0x80
#define SWITZER_NGIO_I2C_OK      0x40
#define SWITZER_NGIO_UART_TX     0x20
#define SWITZER_NGIO_PWR_EN      0x10
#define SWITZER_NGIO_SRC_SEL     8
#define SWITZER_NGIO_PCI_RDY     4
#define SWITZER_NGIO_RESET       2
#define SWITZER_NGIO_I2C_RESET   1
    volatile unsigned int ctrl;

#define SWITZER_NGIO_FLT_INTR    0x400
#define SWITZER_NGIO_INS_INTR    0x200
#define SWITZER_NGIO_REM_INTR    0x100
    volatile unsigned int intr;

    volatile unsigned int debounce;
};

/*
 * Contrl register bit position left shift value
 */
#define SWITZER_DASH_I2C_L_SHFT_CTRL_EN                 0
#define SWITZER_DASH_I2C_L_SHFT_CTRL_CLK_SEL            2
#define SWITZER_DASH_I2C_L_SHFT_CTRL_SLV_EXT_ADDR_MODE  5
#define SWITZER_DASH_I2C_L_SHFT_CTRL_SPEED              6
#define SWITZER_DASH_I2C_L_SHFT_CTRL_RW                 7
#define SWITZER_DASH_I2C_L_SHFT_CTRL_BYTE_LEN           8
#define SWITZER_DASH_I2C_L_SHFT_CTRL_SUB_ADDR_EN        24  //16
#define SWITZER_DASH_I2C_L_SHFT_CTRL_SOFT_RESET         26  //18
#define SWITZER_DASH_I2C_L_SHFT_CTRL_SLV_ACK_MSK        27  //19
#define SWITZER_DASH_I2C_L_SHFT_CTRL_MUX                29

struct switzer_i2c_ctrl_t {
#define SWITZER_DASH_I2C_CTRL_DISABLE            0
#define SWITZER_DASH_I2C_CTRL_NORMAL             1
#define SWITZER_DASH_I2C_CTRL_DMA                2
#define SWITZER_DASH_I2C_CTRL_BITBANG            3

#define SWITZER_DASH_I2C_CTRL_CLK_25             0x00000000
#define SWITZER_DASH_I2C_CTRL_CLK_50             0x00000004
#define SWITZER_DASH_I2C_CTRL_SLV_ADDR_7         0x00000000
#define SWITZER_DASH_I2C_CTRL_SLV_ADDR_10        0x00000020
#define SWITZER_DASH_I2C_CTRL_SPEED_NORMAL_100   0x00000000
#define SWITZER_DASH_I2C_CTRL_SPEED_NORMAL_400   0x00000040
#define SWITZER_DASH_I2C_CTRL_SPEED_DMA_400      0x00000000
#define SWITZER_DASH_I2C_CTRL_SPEED_DMA_HI       0x00000040
#define SWITZER_DASH_I2C_CTRL_WR_MODE            0x00000000
#define SWITZER_DASH_I2C_CTRL_RD_MODE            0x00000080
#define SWITZER_DASH_I2C_CTRL_SUB_ADDR_DIS       0x00000000
#define SWITZER_DASH_I2C_CTRL_SUB_ADDR_1BYTE     0x01000000
#define SWITZER_DASH_I2C_CTRL_SUB_ADDR_2BYTE     0x02000000
#define SWITZER_DASH_I2C_CTRL_SUB_ADDR_3BYTE     0x03000000
#define SWITZER_DASH_I2C_CTRL_SOFT_RESET         0x04000000
#define SWITZER_DASH_I2C_CTRL_CHK_SLV_ACK        0x00000000
#define SWITZER_DASH_I2C_CTRL_IGNOR_SLV_ACK      0x08000000
    volatile unsigned int ctrl;
    volatile unsigned int pad0;

/* Status register bit mask */
#define SWITZER_DASH_I2C_STAT_NOT_ACTIVE      0x00000001
#define SWITZER_DASH_I2C_STAT_BUS_ERR         0x00000002
#define SWITZER_DASH_I2C_STAT_NO_SLV          0x00000004
#define SWITZER_DASH_I2C_STAT_SUB_ADDR_NACK   0x00000008
#define SWITZER_DASH_I2C_STAT_STD_DONE        0x00000010
#define SWITZER_DASH_I2C_STAT_DATA_NACK       0x00000020
#define SWITZER_DASH_I2C_STAT_FIFO_UNDER      0x00000040
#define SWITZER_DASH_I2C_STAT_FIFO_OVER       0x00000080
    volatile unsigned int stat;

    volatile unsigned int stat_mask;
    volatile unsigned int sla_addr;
    volatile unsigned int sla_sub_addr;

#define SWITZER_DASH_I2C_BITBANG_SCL_DRIVER   0x00000001
#define SWITZER_DASH_I2C_BITBANG_SDA_DRIVER   0x00000002
#define SWITZER_DASH_I2C_BITBANG_SCL_IN       0x00000004
#define SWITZER_DASH_I2C_BITBANG_SDA_IN       0x00000008
    volatile unsigned int bit_bang;      /* 0x18 */

    volatile unsigned int byte_count;
    volatile unsigned int pad1[8];       /* skipped 0x20 to 0x3C */
    volatile unsigned int data_fifo;     /* 0x40 */
    volatile unsigned int data_fifo_rw_ptr;  /* 0x44 */
    volatile char pad__0[0x38];          /* 0x48 - 0x80 */
};

struct switzer_uart_ctrl_t {
    volatile unsigned int ctrl;
    volatile unsigned int led;
    volatile unsigned int misc;
};

#define SWITZER_CARRIER_FPGA_FREQ 62500000

struct switzer_uart_t {
    volatile unsigned int dll;  /* 0 */
    volatile unsigned int dlm;  /* 4 ier*/

#define SWITZER_UART_FCR_R_TRIG_00      0x00
#define SWITZER_UART_FCR_R_TRIG_01      0x40
#define SWITZER_UART_FCR_R_TRIG_10      0x80
#define SWITZER_UART_FCR_R_TRIG_11      0xc0
#define SWITZER_UART_FCR_ENABLE_FIFO    0x01 /* Enable the FIFO */
#define SWITZER_UART_FCR_CLEAR_RCVR     0x02 /* Clear the RCVR FIFO */
#define SWITZER_UART_FCR_CLEAR_XMIT     0x04 /* Clear the XMIT FIFO */
#define SWITZER_UART_FCR_DMA_SELECT     0x08 /* For DMA applications */
    volatile unsigned int fcr; /* 8 */

#define SWITZER_UART_LCR_DLAB           0x80 /* Divisor latch access bit */
#define SWITZER_UART_LCR_SBC            0x40 /* Set break control */
#define SWITZER_UART_LCR_SPAR           0x20 /* Stick parity (?) */
#define SWITZER_UART_LCR_EPAR           0x10 /* Even parity select */
#define SWITZER_UART_LCR_PARITY         0x08 /* Parity Enable */
#define SWITZER_UART_LCR_STOP           0x04 /* Stop bits: 0=1 bit, 1=2 bits */
#define SWITZER_UART_LCR_WLEN5          0x00 /* Wordlength: 5 bits */
#define SWITZER_UART_LCR_WLEN6          0x01 /* Wordlength: 6 bits */
#define SWITZER_UART_LCR_WLEN7          0x02 /* Wordlength: 7 bits */
#define SWITZER_UART_LCR_WLEN8          0x03 /* Wordlength: 8 bits */
    volatile unsigned int lcr; /* c */

#define SWITZER_UART_MCR_LOOP           0x10 /* Enable loopback test mode */
    volatile unsigned int mcr;/* 10 */

    volatile unsigned int lsr; /* 14 */
    volatile unsigned int msr; /* 18 */
    volatile unsigned int scr; /* 1C */
    volatile char pad__0[0xe0];          /* 0x20 - 0x100 */
};

struct sm_csrs_top {
    struct general_csrs general;                                 /*        0x0 - 0x48       */
    volatile char pad__0[0xb8];                                  /*       0x48 - 0x100      */
    struct switzer_ng_t wic[SWITZER_CARRIER_SLOT_NUM];           /*      0x100 - 0x118      */
    volatile char pad__1[0xe8];                                  /*      0x118 - 0x200      */
    struct switzer_uart_ctrl_t uart_ctrl;                        /*      0x200 - 0x20C      */
    volatile char pad__2[0xf4];                                  /*      0x20C - 0x300      */
    struct switzer_i2c_ctrl_t wic_i2c[SWITZER_CARRIER_SLOT_NUM]; /*      0x300 - 0x400      */
    struct switzer_i2c_ctrl_t pm_i2c;                            /*      0x400 - 0x480      */
    struct switzer_i2c_ctrl_t clk_i2c;                           /*      0x480 - 0x500      */
    struct switzer_uart_t uart[SWITZER_CARRIER_SLOT_NUM];        /*      0x500 - 0x700      */
    struct mb_csrs mb_ctrl;                                      /*      0x700 - 0x734      */
};

#endif
