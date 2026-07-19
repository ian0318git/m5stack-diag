/* $Id: i2c_address.h,v 1.2 2019/08/06 06:56:12 alpeng Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/overlord/neptune/curie_1RU/i2c_address.h,v $
 *------------------------------------------------------------------
 * by: mcharon
 * 5/2008
 *
 * Copyright (c) 2011-2019 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#ifndef __I2C_ADDRESS__
#define __I2C_ADDRESS__
/* CPU I2C Controller 1 Slave Device Addresses */
/* based on hardware spec address shift 1 bit to the right */
#define MB_I2C_ADDR_RTC		0x68	/* RTC */
#define MB_I2C_ADDR_CLK1	0x4C	/* CLK1 */
#define MB_I2C_ADDR_CLK2	0x69	/* CLK2 */
#define MB_I2C_ADDR_NITROX	0x08	/* Nitrox */
#define MB_I2C_ADDR_MUX0	0x71	/* 1:4 Mux, connected to cpu */
#define MB_I2C_ADDR_MUX1	0x70	/* 1:4 Mux, connected to mux0 */
#define PSU_I2C_ADDR_COOKIE	0x52	/* Power supply Cookie */
#define PSU_I2C_ADDR_OBFL	0x53	/* Power Supply OBFL */
#define PSU_I2C_ADDR_TEMP	0x49	/* Power Supply Temp sensor */
#define USB_I2C_ADDR		0x33	/* USB Console */
#define USB_I2C_MUX		0x1	/* USB Console */
#define USB_I2C_CTRL		0x0	/* USB Console */
#define USB_R_I2C_ADDR		0x63	/* USB Console Read/Write */

#define MB_I2C_ADDR_EEPROM	(0xAE >> 1) //0x57	/* 256 bytes EEPROM;  0xAE >> 1*/
#define MB_I2C_MUX_EEPROM       0
#define MB_I2C_CTRL_EEPROM      0
#define MB_I2C_ADDR_PSU1_EEPROM_SD 0    /* dummy for o2 */

#define MP_I2C_ADDR_TEMP	0x4A	/* Mid Plane Temp sensor */
#define MP_I2C_ADDR_COOKIE	0x51	/* Mid Plane Cookie */
#define MB_I2C_ADDR_SM_CLKBUF	0x6E	/* SM PCIE CLKBUF */
#define MB_I2C_ADDR_PLX_BRIDGE 	0x38	/* PLX PCIE bridge */
#define MB_I2C_ADDR_MCH		0x60	/* NorthBridge - datasheet table 106 */
#define MB_I2C_ADDR_ICH9	0x44	/* SouthBridge - user man 19.2.9 */

/* FPGA Bus 2 Address */
#define MB_I2C_MUX_PWR_SEQ         0
#define MB_I2C_CTRL_PWR_SEQ        2

#define MB_I2C_MUX_ENV_MCU         0
#define MB_I2C_CTRL_ENV_MCU        2

/* Pluggable I2C Device Address 
 * include below header file for PIM
 * */
#include "plug_testcard_host_impl.h"

/* ACT2 Lite */
#define MB_I2C_ADDR_ACT2     (0x75)   
#define MB_I2C_MUX_ACT2            0
#define MB_I2C_CTRL_ACT2           0

/* 0x77 AIKIDO chip */
#define MB_I2C_ADDR_AIKIDO_ACT2  0x77           /* 0x77 AIKIDO Chip */

/* Cavecreek PCH  MST_SMB Address */
#define MB_I2C_ADDR_SYS_CLK      (0xD2 >> 1)    /* SYS CLK */
#define MB_I2C_ADDR_PCIE_CLK 	 (0xD8 >> 1)	/* 0x6C after shifting 1 bitPCIE Clock */

/* CPU I2C Controller 1 Slave Device Addresses */
/* IR3570 connect to CPU for Curie 1RU */
#define MB_I2C_ADDR_3570_VCORE       (0xE4 >> 1)    /* 0x72 (after shifted )*/
#define MB_I2C_ADDR_3570_1P05V_SCSUS (0xE6 >> 1)    /* 0x73 (after shifted )*/
#define MB_I2C_ADDR_3570_1P05V_GBE   (0xE8 >> 1)    /* 0x74 (after shifted )*/
#define MB_I2C_ADDR_3570_1P2V        (0xEA >> 1)    /* 0x75 (after shifted )*/ 

/*
 * FPGA I2C devices Address
 */
/* FPGA I2C Controller 0 */
#define MB_I2C_ADDR_QUACK        (0xEA >> 1)    /* 0x75 (after shifted) */
//#define MB_I2C_ADDR_SYS_EEPROM   (0xAE >> 1)    /* 0x57 (after shifted) */

/* FPGA I2C Controller 1 */
#define MB_I2C_ADDR_USB_CONSOLE_FW_DL  (0x66 >> 1)    /* 0x33 (after shifted) */
#define MB_I2C_ADDR_USB_CONSOLE  (0xC6 >> 1)    /* 0x63 (after shifted) */
//#define MB_I2C_ADDR_GE_SWITCH    (0x88 >> 1)    /* 0x44 (after shifted) */

/* FPGA I2C Controller 2 */
#define MB_I2C_ADDR_PWR_SEQ      (0x88 >> 1)    /* 0x44 (after shifted) */
#define MB_I2C_ADDR_SENSOR       (0xB8 >> 1)    /* 0x5C (after shifted) */ /* barometer */
#define MB_I2C_ADDR_MB_TEMP      (0x30 >> 1)    /* 0x18 (after shifted) : MAX1617A */ /* used by i2c_api.c */
#define MB_I2C_ADDR_TEMP1_IN_1   (0x90 >> 1)    /* 0x48 (after shifted) */
#define MB_I2C_ADDR_TEMP2_IN_2   (0x92 >> 1)    /* 0x49 (after shifted) */
#define MB_I2C_ADDR_TEMP3_out_1  (0x94 >> 1)    /* 0x4A (after shifted) */
#define MB_I2C_ADDR_TEMP4_out_2  (0x96 >> 1)    /* 0x4B (after shifted) */
/* IR3570 chips are moved to CPU I2C bus */

/* FPGA I2C Controller 4 */
#define MB_I2C_ADDR_PSU1_EEPROM      (0xA6 >> 1)    /* 0x53 (after shifted) */  /* able to remove? 0717 */
#define MB_I2C_ADDR_PSU1_MCNTRL      (0xB6 >> 1)    /* 0x5B (after shifted) */
#define MB_I2C_ADDR_PSU2_EEPROM      (0xA6 >> 1)    /* 0x53 (after shifted) */
#define MB_I2C_ADDR_PSU2_MCNTRL      (0xB6 >> 1)    /* 0x5B (after shifted) */
#define MB_I2C_ADDR_POE_PSU1_EEPROM  (0xA0 >> 1)    /* 0x50 (after shifted) */
#define MB_I2C_ADDR_POE_PSU1_MCNTRL  (0x94 >> 1)    /* 0x4A (after shifted) */
#define MB_I2C_ADDR_POE_PSU2_EEPROM  (0xA0 >> 1)    /* 0x50 (after shifted) */
#define MB_I2C_ADDR_POE_PSU2_MCNTRL  (0x94 >> 1)    /* 0x4A (after shifted) */
#define MB_I2C_ADDR_PEM0_EEPROM      (0xA6 >> 1)    /* 0x53 (after shifted) */ /* curie should be this one */
#define MB_I2C_ADDR_PEM0_MCNTRL      (0xB6 >> 1)    /* 0x58 (after shifted) */
#define MB_I2C_ADDR_PEM1_EEPROM      (0xA6 >> 1)    /* 0x53 (after shifted) */
#define MB_I2C_ADDR_PEM1_MCNTRL      (0xB6 >> 1)    /* 0x58 (after shifted) */

/* Neptune FPGA I2C Controler 5 */
//#define MB_I2C_ADDR_CLK_GENERATOR    (0xD8 >> 1)    /* 0x6C (after shifted) */
#define MB_I2C_ADDR_CLK_BUFFER       (0xD6 >> 1)    /* 0x6E (after shifted) */

/* FPGA I2C Controller 8 */
#define MB_I2C_ADDR_POE_30W_QUACK    (0xE0 >> 1)    /* 0x70 (after shifted) mux 0 */  /* used for platform_cookie.c */
//#define MB_I2C_ADDR_POE_30W_CTRLER   (0x40 >> 1)    /* 0x20 (after shifted) mux 1 */

/* FPGA I2C Controller 16 */
#define MB_I2C_ADDR_USB0_REDRIVER  (0xC0 >> 1)    /* 0x60 (after shifted) */
#define MB_I2C_ADDR_USB1_REDRIVER  (0xC2 >> 1)    /* 0x61 (after shifted) */

/* FGPA I2C Controller 17 */
#define MB_I2C_ADDR_PSU_I2C_MUX  (0xE0 >> 1)
#define MB_I2C_ADDR_SFP_I2C_MUX  (0xE0 >> 1)

#endif
/*-------------------------------------------------
$Log: i2c_address.h,v $
Revision 1.2  2019/08/06 06:56:12  alpeng
merge curie, switzer and nightwatch to trunk

Revision 1.1.2.10  2019/04/03 08:40:44  alpeng
add psu poe check back to curie per HW request.

Revision 1.1.2.9  2019/02/11 07:33:37  meho
Support new PIM test-card (PCIe)

Revision 1.1.2.8  2018/12/27 08:55:41  alpeng
clean up code

Revision 1.1.2.7  2018/09/27 09:46:24  alpeng
support tam lib and aikido for curie

Revision 1.1.2.6  2018/08/24 19:07:04  meho
Fixed plug i2c address

Revision 1.1.2.5  2018/08/15 23:00:17  alpeng
fixed ir3570 addr and test for curie

Revision 1.1.2.4  2018/08/15 19:37:27  alpeng
 update i2c scan test for PSU and USB redriver

Revision 1.1.2.3  2018/07/27 08:23:53  meho
Added pluggable LTE/Testcard test item.

Revision 1.1.2.2  2018/07/19 09:27:37  alpeng
1. Moving IR3570 chips to CPU I2C bus. 2. Removed related code of i2c devices which are not support on Curie; max1617, IDT8T49N287I(sys clk), poe psu, 30w poe

Revision 1.1.2.1  2018/06/22 08:05:18  alpeng
move curie diag to neptune/curie_1RU directory

Revision 1.1.2.1  2018/05/30 02:39:36  alpeng
porting neptune x86 to curie

Revision 1.2  2018/05/18 09:24:59  alpeng
 Neptune merge to trunk with tag <neptune-branch-0518>

Revision 1.1.2.9  2018/01/25 08:12:28  leschen
Adding PSU, POE_PSU and IR3570 into I2C lane scan test.

Revision 1.1.2.8  2017/09/27 06:01:46  leschen
Support usb re-driver for I2c scan test.

Revision 1.1.2.7  2017/04/05 08:27:54  leschen
Sync with <ng_diag-tag-032917>

Revision 1.1.2.6  2017/03/23 06:35:06  leschen
Support Barometer LPS25H

Revision 1.1.2.5  2016/10/26 07:06:14  leschen
Modify I2C scan test and POE daughter card support.

Revision 1.1.2.4  2016/10/06 20:25:16  leschen
Modify Neptune PCIe clk i2c address.

Revision 1.1.2.3  2016/08/30 07:19:43  leschen
Update Neptune I2C slave address.

Revision 1.1.2.2  2016/06/01 23:14:17  jskow
Update Makefile for Neptune, add mb_test structures for PCIe IF test and PCIe register check

Revision 1.3  2014/01/07 08:41:30  hroni
fix compile error

Revision 1.2  2013/11/01 07:04:44  alpeng
support i2c scan test on juno-plx

Revision 1.1  2013/06/04 07:09:15  hroni
move platform specific files to the corresponding directory (i.e. x86 and utah)

Revision 1.3  2013/05/31 12:51:28  danchung
Add checking board type for Juno.

Revision 1.2  2012/03/28 00:38:17  mcharon
remove forward slash from second line

Revision 1.1.1.1  2012/03/23 23:02:06  ptong
Initial archive of ng_diag module


$Endlog$
*/
