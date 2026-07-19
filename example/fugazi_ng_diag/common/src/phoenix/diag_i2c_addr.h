/* $Id: diag_i2c_addr.h,v 1.2 2021/04/15 00:52:26 achiu2 Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/phoenix/diag_i2c_addr.h,v $
 *------------------------------------------------------------------
 * 
 * 
 *
 * Copyright (c) 2018-2019 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#ifndef __DIAG_I2C_ADDR_H__
#define __DIAG_I2C_ADDR_H__



/* CPU I2C Controller 1 Device Addresses */
/* based on hardware spec address shift 1 bit to the right */

/* I2C 0 Device Addresses */
#define MB_I2C_ADDR_SPD             (0xA0 >> 1)    /* 0x50 */
#define MB_I2C_ADDR_EEPROM          (0xA8 >> 1)    /* 0x54 */

/* FPGA I2C Device Addresses */
#define MB_I2C_ADDR_PWR_SEQ         (0x88 >> 1)     /* 0x44 Power sequence MCU */
#define MB_I2C_ADDR_MB_TEMP1_IN_1   (0x90 >> 1)     /* 0x48 Temp sensor */
#define MB_I2C_ADDR_MB_TEMP2_IN_2   (0x92 >> 1)     /* 0x49 Temp sensor */
#define MB_I2C_ADDR_MB_TEMP3_OUT_1  (0x94 >> 1)    /* 0x4A Temp sensor */
#define MB_I2C_ADDR_MB_TEMP4_OUT_2  (0x96 >> 1)    /* 0x4B Temp sensor */
#define MB_I2C_ADDR_BAROMETER       (0xB8 >> 1)    /* 0x5C Baro sensor */
#define MB_I2C_ADDR_ACT2             0x75          /* 0x75 Discrete ACT2 */
#define MB_I2C_ADDR_AIKIDO           0x77          /* 0x77 Aikido ACT2 */
#define MB_I2C_MUX_ACT2              0
#define MB_I2C_CTRL_ACT2             0
#define MB_I2C_ADDR_PSU_MCCTLR     (0xA6 >> 1)    /* 0x53 PSU Microcontroller */
#define MB_I2C_ADDR_PSU_EEPROM     (0xA4 >> 1)    /* 0x52 PSU EEPROM */
#define MB_I2C_ADDR_USB_UART_CTRL_1 (0xC6 >> 1)    /* 0x63 USB to UART controller(register access) */
#define MB_I2C_ADDR_USB_UART_CTRL_2 (0x66 >> 1)    /* 0x33 USB to UART controller(firmware download) */

#define MB_I2C_ADDR_PECI_WR         0x4C          /* SMB PECI Write */
#define MB_I2C_ADDR_PECI_RD         0x4B          /* SMB PECI Read */

/* EEPROM */
#define MB_I2C_ADDR_SYS_EEPROM0 	(0xA4 >> 1) /* 0x52 EEPROM 512kbit */
#define MB_I2C_ADDR_SYS_EEPROM1 	(0xA6 >> 1) /* 0x53 EEPROM 512kbit */
#define MB_I2C_MUX_EEPROM            0
#define MB_I2C_CTRL_EEPROM           0


/* Externs */
extern void *get_n2g_i2c_if(uint8_t, uint8_t, uint8_t);
extern void *platform_i2c_get_quack(uint8_t, uint8_t);
extern void build_i2c_menu(void);
extern boolean g_i2c_read_cterr;


#endif
