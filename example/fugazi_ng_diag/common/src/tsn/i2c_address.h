/* $Id: i2c_address.h,v 1.4 2018/05/24 09:47:10 steja Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/tsn/i2c_address.h,v $
 *------------------------------------------------------------------
 * by: mcharon
 * 5/2008
 *
 * Copyright (c) 2011-2018 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#ifndef __I2C_ADDRESS__
#define __I2C_ADDRESS__
/* CPU I2C Controller 1 Device Addresses */
/* based on hardware spec address shift 1 bit to the right */
/* I2C 0 Device Addresses */
#define MB_I2C_ADDR_EEPROM	(0xAA >> 1)     /* 0x55 EEPROM 2kbit */

/* I2C 0 Device Addresses */
#define MB_I2C_ADDR_MB_TEMP	(0x38 >> 1)     /* 0x1C Temp sensor MAX31730AUB+ */

/* I2C 2 Device Addresses */
#define MB_I2C2_POE_CONTR       0x30        /* I2C2 0x30 PoE(TI, TPS2386B) controller */
#define MB_I2C_ADDR_POE_30W_CTRLER (0x60 >> 1) /* 0x30 (after shifted) mux 1 (TI TPS2386PW) */
#define MB_I2C_ADDR_POE_EEPROM (0xA4 >> 1)  /* 0x52 EEPROM POE */
#define MB_I2C2_MCU            (0x80 >> 1)  /* 0x40 TSN MCU */
#define MB_I2C2_MCU_BOOTLOADER   (0xA2 >> 1)  /* 0x51 TSN MCU bootloader mode */
#define	WIFI_I2C_STAR_ADDR_TEMP     (0x48)       /* STAR WiFi temp. sensor I2C addr. */
#define	WIFI_I2C_ADDR_TEMP     (0x38 >> 1)     /* 0x1C Wifi Temp sensor MAX31730AUB+ */


/* Pluggable I2C Device Address */
#define PLUG_I2C_ADDR_TEMP             (0x9C >> 1)    
#define PLUG_I2C_ADDR_ACT2             (0xE6 >> 1)
#define PLUG_TC_I2C_ADDR_GPIO_EXP      (0x38 >> 1)   /* Pluggable Test Card GPIO Expander */
#define PLUG_TC_I2C_ADDR_PHY           (0xB8 >> 1)   /* Pluggable Test Card 88E1112 PHY */
#define PLUG_MAN_I2C_ADDR_GPIO_EXP     (0x4E >> 1)   /* Pluggable LTE Mandatory GPIO Expander */
#define PLUG_OPT_I2C_ADDR_GPIO_EXP     (0x4C >> 1)   /* Pluggable LTE Optional GPIO Expander */

/* ACT2 Lite */
#define MB_I2C_ADDR_ACT2     (0xE0 >> 1)        /* 0x70 Secure Chip */
#define MB_I2C_ADDR_AIKIDO_ACT2  0x77           /* 0x77 AIKIDO Chip */
#define MB_I2C_MUX_ACT2            0
#define MB_I2C_CTRL_ACT2           0
#define WIFI_I2C_ADDR_ACT2   (0xE0 >> 1)      /* TBD */
#define WIFI_I2C_MUX_ACT2          0          /* TBD */
#define WIFI_I2C_CTRL_ACT2         0          /* TBD */
#define POE_I2C_ADDR_ACT2    (0xFE >> 1)      /* TBD */
#define POE_I2C_MUX_ACT2           0          /* TBD */
#define POE_I2C_CTRL_ACT2          0          /* TBD */

/* EEPROM */
#define MB_I2C_ADDR_SYS_EEPROM0	(0xA4 >> 1)     /* 0x52 EEPROM 512kbit */
#define MB_I2C_ADDR_SYS_EEPROM1	(0xA6 >> 1)     /* 0x53 EEPROM 512kbit */
#define MB_I2C_MUX_EEPROM       0
#define MB_I2C_CTRL_EEPROM      0

/* DIMM */
#define MB_I2C_ADDR_DIMM0	(0xA0 >> 1)     /* 0x50 */
#define MB_I2C_ADDR_DIMM1	(0xA2 >> 1)     /* 0x51 */

/* RTC */
#define MB_I2C_ADDR_RTC		(0xD0 >> 1)    /* 0x68 RTC DS1337S+ */

/* USB CONSOLE */
#define MB_I2C_ADDR_USB_CONSOLE_FW_DL  (0x66 >> 1)    /* 0x33 (after shifted) */
#define MB_I2C_ADDR_USB_CONSOLE  (0xC6 >> 1)    /* 0x63 (after shifted) */

/* SFP */
#define MB_I2C_ADDR_SFP0    (0xA0 >> 1)    /* 0x50 88E1112 SFP*/
#define MB_I2C_ADDR_SFP0_INT_REG    (0xAC >> 1)    /* 0x56 88E1112 SFP Internal Register */

#endif
/*-------------------------------------------------
$Log: i2c_address.h,v $
Revision 1.4  2018/05/24 09:47:10  steja
CSCvj57981-Enhance SFP GLC-GE-100FX Support

Revision 1.3  2018/02/09 09:56:54  hondwang
Merge Star branch star-branch-c9xx to main trunk

Revision 1.2.20.1  2018/01/20 06:27:23  hondwang
prepare merge star-branch-c9xx to main trunk

Revision 1.2.4.1  2017/08/15 14:18:38  hondwang
star branch c9xx initial check in

Revision 1.2  2017/08/02 14:21:45  steja
Support TSN-H/M platform code

Revision 1.1.8.2  2017/07/29 03:41:02  steja
tsn-branch5 synced with Maintrunk repositories

Revision 1.1.6.2  2017/07/20 13:38:04  steja
tsn-branch4 merge with maintrunk

Revision 1.1.4.4.6.5  2017/07/10 06:56:45  hondwang
add mcu upgrade function

Revision 1.1.4.4.6.4  2017/07/04 15:08:39  palin2
Added Star wifi temperature sensor diag tests.

Revision 1.1.4.4.6.3  2017/06/17 12:45:37  hondwang
Add test card phy testing function

Revision 1.1.4.4.6.2  2017/06/14 12:34:40  shjung
Create Pluggable LTE tests and utilities menu

Revision 1.1.4.4.6.1  2017/06/13 06:54:14  shjung
Add pluggable FPGA I2C read/write function

Revision 1.1.4.4  2016/09/13 14:35:47  steja
Commit Aikido / TAM Mailbox code

Revision 1.1.4.3  2016/07/21 14:26:32  steja
1. Update Temperature sensor function
2. Add Wifi Temperature sensor to basic utilities
3. Add LTE Extended Feature Test
4. Display Chasis Temperature When Boot up
5. Update I2C scan function
6. Update POE cookie check card present

Revision 1.1.4.2  2016/06/30 06:22:48  steja
tsn-branch2 sync with main trunk

Revision 1.1.2.6  2016/06/21 04:36:33  palin2
Added voltage margin utility and MCU register R/W utilities.

Revision 1.1.2.5  2016/05/10 06:17:33  palin2
Updated PoE PSE related diag code after bring up.

Revision 1.1.2.4  2016/05/09 08:06:55  steja
Fixed POE i2c address R/W

Revision 1.1.2.3  2016/05/06 16:10:18  steja
Bring up I2C-2 for RTC

Revision 1.1.2.2  2016/04/11 14:12:27  steja
Update code i2c utility for bringup

Revision 1.1.2.1  2016/03/20 05:32:40  steja
1. Add i2c scan and i2c read write utility
2. Add sensor temperature and eeprom utility



$Endlog$
*/
