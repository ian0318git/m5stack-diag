 /* $Id: diag_i2c_addr.h,v 1.2 2019/10/17 02:16:22 kehuang2 Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/tabei-l/diag_i2c_addr.h,v $
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
/* DIMM */
#define MB_I2C_ADDR_DIMM0           (0xA0 >> 1)    /* 0x50 */
#define MB_I2C_ADDR_DIMM1           (0xA4 >> 1)    /* 0x52 */
#define MB_I2C_ADDR_EEPROM          (0xA8 >> 1)    /* 0x54 */

/* FPGA I2C Device Addresses */
#define MB_I2C_ADDR_PWR_SEQ         (0x88 >> 1)     /* 0x44 Power sequence MCU */
#define MB_I2C_ADDR_MB_TEMP1_IN_1   (0x90 >> 1)     /* 0x48 Temp sensor */
#define MB_I2C_ADDR_MB_TEMP2_IN_2   (0x92 >> 1)     /* 0x49 Temp sensor */
#define MB_I2C_ADDR_MB_TEMP3_OUT_1  (0x94 >> 1)    /* 0x4A Temp sensor */
#define MB_I2C_ADDR_MB_TEMP4_OUT_2  (0x96 >> 1)    /* 0x4B Temp sensor */
#define MB_I2C_ADDR_BAROMETER       (0xB8 >> 1)    /* 0x5C Baro sensor */
#define MB_I2C_ADDR_ACT2             0x75        /* 0x70 Secure Chip */
#define MB_I2C_MUX_ACT2              0
#define MB_I2C_CTRL_ACT2             0

#define MB_I2C_MUX_SFP               0x71
#define MB_I2C_SFP_DEV               0x50
#define MUX_9543_EN_CHANNEL0         0x01
#define MUX_9543_EN_CHANNEL1         0x02
#define MUX_9543_COMMAND_SIZE        1

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
/*-------------------------------------------------
$Log: diag_i2c_addr.h,v $
Revision 1.2  2019/10/17 02:16:22  kehuang2
Collapse Tabei-L into main trunk

Revision 1.1.2.9  2019/07/15 11:28:47  kehuang2
Support Barometer test and utility

Revision 1.1.2.8  2019/07/09 06:11:31  kehuang2
Update I2C bus change and enhence I2C scan coverage

Revision 1.1.2.7  2019/05/21 09:18:51  kehuang2
Support Port80 LED

Revision 1.1.2.6  2019/05/21 03:18:00  kehuang2

1.SFP EN LED Support base on PreP2B respin
2.Support SFP Mux access utility

Revision 1.1.2.5  2018/12/27 07:30:38  harrchan
Update I2C scan test

Revision 1.1.2.4  2018/10/25 09:55:24  harrchan
Add MCU utility in I2C utility

Revision 1.1.2.3  2018/10/19 01:44:19  harrchan
I2C scan test

Revision 1.1.2.2  2018/10/16 02:26:02  kodko
Support Tabei-L ACT2 & Cookie programming.

Revision 1.1.2.1  2018/10/02 01:49:59  harrchan
Initial commit for Tabei-L P1A bring up.

$Endlog$
*/
