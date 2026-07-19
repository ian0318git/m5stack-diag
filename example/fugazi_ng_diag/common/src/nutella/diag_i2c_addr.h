/* $Id: diag_i2c_addr.h,v 1.4 2019/07/11 12:31:28 alicehua Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/nutella/diag_i2c_addr.h,v $
 *------------------------------------------------------------------
 * 
 * 
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
#define MB_I2C_ADDR_FRU_EEPROM    0x54     /* 0x54 */
#define MB_I2C_ADDR_DIMM0         0x50     /* 0x50 */
#define MB_I2C_ADDR_MB_TEMP_LM75  0x4A     /* 0x4A Temp sensor LM75BD */

/* FPGA I2C Device Addresses */
#define MB_I2C_ADDR_ACT2          0x70        /* 0x70 Secure Chip */
#define MB_I2C_ADDR_AIKIDO_ACT2   0x77        /* 0x77 Secure Chip */
#define MB_I2C_MUX_ACT2           0
#define MB_I2C_CTRL_ACT2          0

/* EEPROM */
#define MB_I2C_ADDR_SYS_EEPROM0	(0xA4 >> 1)     /* 0x52 EEPROM 512kbit */
#define MB_I2C_ADDR_SYS_EEPROM1	(0xA6 >> 1)     /* 0x53 EEPROM 512kbit */
#define MB_I2C_MUX_EEPROM       0
#define MB_I2C_CTRL_EEPROM      0


/* Externs */
extern void *get_n2g_i2c_if(uint8_t, uint8_t, uint8_t);
extern void *platform_i2c_get_quack(uint8_t, uint8_t);
extern void build_i2c_menu(void);
extern boolean g_i2c_read_cterr;


#endif
/*-------------------------------------------------
$Log: diag_i2c_addr.h,v $
Revision 1.4  2019/07/11 12:31:28  alicehua
Collapse Nutella codes into main trunk.

$Endlog$
*/
