 /* $Id: diag_i2c_addr.h,v 1.2 2019/12/11 10:10:30 lucywang Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/nanook/diag_i2c_addr.h,v $
 *------------------------------------------------------------------
 * 
 * Filename: diag_i2c_addr.h
 *
 * Copyright (c) 2019 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#ifndef __DIAG_I2C_ADDR_H__
#define __DIAG_I2C_ADDR_H__



/* CPU I2C Controller 1 Device Addresses */
/* based on hardware spec address shift 1 bit to the right */

/* I2C 0 Device Addresses */
/* DIMM */
#define MB_I2C_ADDR_DIMM0         (0xA0 >> 1)    /* 0x50 */
#define MB_I2C_ADDR_DIMM1         (0xA4 >> 1)    /* 0x52 */
#define MB_I2C_ADDR_EEPROM        (0xA8 >> 1)    /* 0x54 */

/* FPGA I2C Device Addresses */
#define MB_I2C_ADDR_PWR_SEQ        (0x88 >> 1)     /* 0x44 Power sequence MCU */
#define MB_I2C_ADDR_MB_TEMP1       (0x94 >> 1)    /* 0x4A Temp sensor */
#define MB_I2C_ADDR_MB_TEMP2       (0x96 >> 1)    /* 0x4B Temp sensor */
#define MB_I2C_ADDR_MB_SFP         (0xA0 >> 1)    /* 0x50 SFP */
#define MB_I2C_ADDR_PRESSURE       (0xBA >> 1)    /* 0x5D Pressure sensor */
#define MB_I2C_ADDR_ACT2          0x75        /* 0x75 Secure Chip */
#define MB_I2C_MUX_ACT2           0
#define MB_I2C_CTRL_ACT2          0

/* EEPROM */
#define MB_I2C_ADDR_SYS_EEPROM0	(0xA4 >> 1)     /* 0x52 EEPROM 512kbit */
#define MB_I2C_ADDR_SYS_EEPROM1	(0xA6 >> 1)     /* 0x53 EEPROM 512kbit */
#define MB_I2C_MUX_EEPROM       0
#define MB_I2C_CTRL_EEPROM      0

/* 0x77 AIKIDO chip */
#define MB_I2C_ADDR_AIKIDO_ACT2  0x77           /* 0x77 AIKIDO Chip */

#define NGIOSM_I2C_ADDR_OIR     0x4A    /* OIR */
#define NGIOSM_VM_DC_I2C_ADDR_ACT2      0x73    /* Quack */
#define NGIOSM_I2C_ADDR_ACT2    0x75    /* Quack */

/* FPGA I2C Controller 8 */
#define MB_I2C_ADDR_POE_30W_QUACK    (0xE0 >> 1)    /* 0x70 (after shifted) mux 0 */  /* used for platform_cookie.c */
//#define MB_I2C_ADDR_POE_30W_CTRLER   (0x40 >> 1)    /* 0x20 (after shifted) mux 1 */
/* Neptune - 30W POE QUACK */
#define I2C_CTRL_POE_30W_QUACK  I2C_CTRL_EIGHT
#define I2C_MUX_POE_30W_QUACK   I2C_MUX_ONE

/* Common definition for I2C controller */
#define I2C_CTRL_ZERO        0
#define I2C_CTRL_ONE         1
#define I2C_CTRL_TWO         2
#define I2C_CTRL_FOUR        4
#define I2C_CTRL_FIVE        5
#define I2C_CTRL_SIX         6
#define I2C_CTRL_SEVEN       7
#define I2C_CTRL_EIGHT       8
#define I2C_CTRL_NIGHT       9
#define I2C_CTRL_TEN        10
#define I2C_CTRL_ELEVEN     11
#define I2C_CTRL_TWELVE     12
#define I2C_CTRL_THIRTEEN   13
#define I2C_CTRL_FOURTEEN   14
#define I2C_CTRL_FIFTEEN    15
#define I2C_CTRL_SIXTEEN    16
#define I2C_CTRL_SEVENTEEN  17
#define I2C_CTRL_EIGHTEEN   18
#define I2C_CTRL_NINETEEN   19
#define I2C_CTRL_TWENTY     20
#define I2C_CTRL_MAX        I2C_CTRL_TWENTY
/* Externs */
extern void *get_n2g_i2c_if(uint8_t, uint8_t, uint8_t);
extern void *platform_i2c_get_quack(uint8_t, uint8_t);
extern void build_i2c_menu(void);
extern boolean g_i2c_read_cterr;


#endif
/*-------------------------------------------------
$Log: diag_i2c_addr.h,v $
Revision 1.2  2019/12/11 10:10:30  lucywang
Merged Nanook to main trunk


$Endlog$
*/
