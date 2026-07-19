/* $Id: diag_fpga_i2c.h,v 1.2 2016/04/20 11:25:30 benchen2 Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/tachi_bmc/diag_fpga_i2c.h,v $
 *------------------------------------------------------------------
 *
 * diag_fpga_i2c.h - Header file for FPGA I2C Function
 *
 * July 2015, Times Huang
 *
 * Copyright (c) 2016 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */
 
#ifndef __DIAG_FPGA_I2C__
#define __DIAG_FPGA_I2C__

#define SCL_DRIVE_TIMES                         (100)

#define FPGA_I2C_IDLE_TIMEOUT                   (30)
#define FPGA_I2C_IDLE_RETRY                     (5)
#define FPGA_I2C_OP_TIMEOUT                     (500)

/* FPGA I2C Status Register Mask */
#define FPGA_I2C_STAT_NOT_ACTIVE                (0x00000001)
#define FPGA_I2C_STAT_BUS_ERR                   (0x00000002)
#define FPGA_I2C_STAT_NO_SLV                    (0x00000004)
#define FPGA_I2C_STAT_SUB_ADDR_NACK             (0x00000008)
#define FPGA_I2C_STAT_STD_DONE                  (0x00000010)
#define FPGA_I2C_STAT_DATA_NACK                 (0x00000020)
#define FPGA_I2C_STAT_FIFO_UNDER                (0x00000040)
#define FPGA_I2C_STAT_FIFO_OVER                 (0x00000080)

/*
 * Contrl register bit position left shift value
 */
#define L_SHFT_FPGA_I2C_CTRL_EN                 (0)
#define L_SHFT_FPGA_I2C_CTRL_CLK_SEL            (2)
#define L_SHFT_FPGA_I2C_CTRL_SLV_EXT_ADDR_MODE  (5)
#define L_SHFT_FPGA_I2C_CTRL_SPEED              (6)
#define L_SHFT_FPGA_I2C_CTRL_RW                 (7)
#define L_SHFT_FPGA_I2C_CTRL_BYTE_LEN           (8)
#define L_SHFT_FPGA_I2C_CTRL_SUB_ADDR_EN        (24)  //16
#define L_SHFT_FPGA_I2C_CTRL_SOFT_RESET         (26)  //18
#define L_SHFT_FPGA_I2C_CTRL_SLV_ACK_MSK        (27)  //19
#define L_SHFT_FPGA_I2C_CTRL_MUX                (29)

/* FPGA I2C Control */
#define FPGA_I2C_CTRL_DISABLE                   (0)
#define FPGA_I2C_CTRL_NORMAL                    (1)
#define FPGA_I2C_CTRL_DMA                       (2)
#define FPGA_I2C_CTRL_BITBANG                   (3)

#define FPGA_I2C_CTRL_CLK_25                    (0x00000000)
#define FPGA_I2C_CTRL_CLK_50                    (0x00000004)
#define FPGA_I2C_CTRL_SLV_ADDR_7                (0x00000000)
#define FPGA_I2C_CTRL_SLV_ADDR_10               (0x00000020)
#define FPGA_I2C_CTRL_SPEED_NORMAL_100          (0x00000000)
#define FPGA_I2C_CTRL_SPEED_NORMAL_400          (0x00000040)
#define FPGA_I2C_CTRL_SPEED_DMA_400             (0x00000000)
#define FPGA_I2C_CTRL_SPEED_DMA_HI              (0x00000040)
#define FPGA_I2C_CTRL_WR_MODE                   (0x00000000)
#define FPGA_I2C_CTRL_RD_MODE                   (0x00000080)
#define FPGA_I2C_CTRL_SUB_ADDR_DIS              (0x00000000)
#define FPGA_I2C_CTRL_SUB_ADDR_1BYTE            (0x01000000)
#define FPGA_I2C_CTRL_SUB_ADDR_2BYTE            (0x02000000)
#define FPGA_I2C_CTRL_SUB_ADDR_3BYTE            (0x03000000)
#define FPGA_I2C_CTRL_SOFT_RESET                (0x04000000)
#define FPGA_I2C_CTRL_CHK_SLV_ACK               (0x00000000)
#define FPGA_I2C_CTRL_IGNOR_SLV_ACK             (0x08000000)

#define FPGA_I2C_CTRL_BYTE_LENGTH_SHIFT         (8)
#define FPGA_I2C_CTRL_SUBSIZE_SHIFT             (24)

#define I2C_BITBANG_SCL_DRIVER                  (0x00000001)
#define I2C_BITBANG_SDA_DRIVER                  (0x00000002)
#define I2C_BITBANG_SCL_IN                      (0x00000004)
#define I2C_BITBANG_SDA_IN                      (0x00000008)

extern int diag_fpga_get_i2c_ctrl_addr(uint8);
extern int diag_fpga_i2c_write(int, int, int, int, uint, uint, uchar *);
extern int diag_fpga_i2c_read(int, int, int, int, uint, uint, uchar *);

#ifdef FOXCONN_FPGA
#define FPGA_I2C_CTRL_BYTE_LENGTH               (0x00000100)
#define FPGA_I2C_CTRL_ENABLE                    (0x00000001)
#define FPGA_I2C_DIS_MASTER                     (0x0)
#define FPGA_I2C_STANDARD_MODE                  (0x1)
#define FPGA_I2C_BIT_BANG                       (0x3)
extern int diag_fxn_fpga_i2c_write(int, int, int, int, uint, uint, uchar *);
extern int diag_fxn_fpga_i2c_read(int, int, int, int, uint, uint, uchar *);
#endif

#endif /* __DIAG_FPGA_I2C__ */

/*---------------------------------------------------------------
$Log: diag_fpga_i2c.h,v $
Revision 1.2  2016/04/20 11:25:30  benchen2
add tachi fru portion

Revision 1.1.2.3  2015/08/28 02:33:52  tirawan
To support ACT2 M/B cookie programming using Foxconn FPGA

Revision 1.1.2.2  2015/08/21 10:38:30  benchen2
Add foxconn FPGA I2C R/W Function

Revision 1.1.2.1  2015/07/24 03:39:36  tirawan
Add FPGA I2C read/write function, ACT2 cookie read/write function



$Endlog$
*/
