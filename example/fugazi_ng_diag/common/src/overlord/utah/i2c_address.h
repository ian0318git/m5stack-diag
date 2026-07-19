/* $Id: i2c_address.h,v 1.13 2018/08/30 06:59:43 haohsu Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/overlord/utah/i2c_address.h,v $
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
/* CPU I2C Controller 1 Slave Device Addresses */
/* based on hardware spec address shift 1 bit to the right */
#define MB_I2C_ADDR_RTC		0x68	/* RTC - 0xD0 >> 1 */
#define MB_I2C_ADDR_CLK1	0x4C	/* CLK1 - 0x98 >> 1 */
#define MB_I2C_ADDR_CLK2	0x69	/* CLK2 - 0xD2 >> 1 */
#define MB_I2C_ADDR_NITROX	0x08	/* Nitrox - 0x10 >>1 */
#define MB_I2C_ADDR_MUX0	0x71	/* 1:4 Mux, connected to cpu - 0xE2 >> 1 */
#define MB_I2C_ADDR_MUX1	0x70	/* 1:4 Mux, connected to mux0 - 0xE0 >> 1 */
#define PSU_I2C_ADDR_COOKIE	0x52	/* Power supply Cookie - 0xA4 >> 1 */
#define PSU_I2C_ADDR_OBFL	0x53	/* Power Supply OBFL - 0xA6 >> 1 */
#define PSU_I2C_ADDR_TEMP	0x49	/* Power Supply Temp sensor - 0x92 >> 1 */
#define USB_I2C_ADDR		0x33	/* USB Console - 0x66 >> 1 */
#define USB_I2C_MUX		    0x1	    /* USB Console - 0x02 >> 1 */
#define USB_I2C_CTRL		0x0	    /* USB Console */
#define USB_R_I2C_ADDR		0x63	/* USB Console Read/Write - 0xC6 >> 1*/

#define MB_I2C_ADDR_EEPROM	0x57	/* 256 bytes EEPROM;  0xAE >> 1*/
#define MB_I2C_MUX_EEPROM       0
#define MB_I2C_CTRL_EEPROM      0


#define MP_I2C_ADDR_TEMP	0x4A	/* Mid Plane Temp sensor - 0x94 >> 1 */
#define MP_I2C_ADDR_COOKIE	0x51	/* Mid Plane Cookie - 0xA2 >> 1 */
#define MB_I2C_ADDR_SM_CLKBUF	0x6E	/* SM PCIE CLKBUF - 0xDC >> 1 */
#define MB_I2C_ADDR_PLX_BRIDGE 	0x38	/* PLX PCIE bridge - 0x70 >> 1 */
#define MB_I2C_ADDR_MCH		0x60	/* NorthBridge - datasheet table 106 */
#define MB_I2C_ADDR_ICH9	0x44	/* SouthBridge - user man 19.2.9 */

/* FPGA Bus 2 Address */
#define MB_I2C_MUX_PWR_SEQ         0
#define MB_I2C_CTRL_PWR_SEQ        2

#define MB_I2C_MUX_ENV_MCU         0
#define MB_I2C_CTRL_ENV_MCU        2

/* ACT2 Lite */
#define MB_I2C_ADDR_ACT2     (0x75)   
#define MB_I2C_MUX_ACT2            0
#define MB_I2C_CTRL_ACT2           0

/*
 * FPGA I2C devices Address
 */
/* FPGA I2C Controller 0 */
#define MB_I2C_ADDR_SYS_EEPROM   (0xAE >> 1)    /* 0x57 (after shifted) */

/* FPGA I2C Controller 1 */
#define MB_I2C_ADDR_USB_CONSOLE_FW_DL  (0x66 >> 1)    /* 0x33 (after shifted) */
#define MB_I2C_ADDR_USB_CONSOLE  (0xC6 >> 1)    /* 0x63 (after shifted) */

/* FPGA I2C Controller 2 */
#define MB_I2C_ADDR_PWR_SEQ      (0x88 >> 1)    /* 0x44 (after shifted) */
#define MB_I2C_ADDR_VTG_MNTR     (0x68 >> 1)    /* 0x34 (after shifted) */
#define MB_I2C_ADDR_ENV_MCU      (0x80 >> 1)    /* 0x40 (after shifted) */
#define MB_I2C_ADDR_SENSOR       (0xC0 >> 1)    /* 0x60 (after shifted) */
#ifndef UTAH
#define MB_I2C_ADDR_MB_TEMP      (0x30 >> 1)    /* 0x18 (after shifted) : MAX1617A */
#define MB_I2C_ADDR_MB_TEMP_ALRT (0x18 >> 1)    /* 0x0C (after shifted) : MAX1617A Alert*/
#else
#define MB_I2C_ADDR_MB_TEMP      (0x92 >> 1)    /* 0x49 (after shifted) : MAX1617A */
#define MB_I2C_ADDR_MB_TEMP_ALRT (0x93 >> 1)    /* 0x49 (after shifted) : MAX1617A Alert*/ 
#endif

/* FPGA I2C Controller 4 */
#define MB_I2C_ADDR_PSU1_EEPROM      (0xA4 >> 1)    /* 0x52 (after shifted) */
#define MB_I2C_ADDR_PSU1_MCNTRL      (0xA6 >> 1)    /* 0x53 (after shifted) */
#define MB_I2C_ADDR_PSU2_EEPROM      (0xA4 >> 1)    /* 0x52 (after shifted) */
#define MB_I2C_ADDR_PSU2_MCNTRL      (0xA6 >> 1)    /* 0x53 (after shifted) */
#define MB_I2C_ADDR_POE_PSU1_EEPROM  (0xA0 >> 1)    /* 0x50 (after shifted) */
#define MB_I2C_ADDR_POE_PSU1_MCNTRL  (0x94 >> 1)    /* 0x4A (after shifted) */
#define MB_I2C_ADDR_POE_PSU2_EEPROM  (0xA0 >> 1)    /* 0x50 (after shifted) */
#define MB_I2C_ADDR_POE_PSU2_MCNTRL  (0x94 >> 1)    /* 0x4A (after shifted) */
/* For Sword and Dagger */
#define MB_I2C_ADDR_PSU1_EEPROM_SD      (0xA8 >> 1) /* 0x54 (after shifted) */

/* FPGA I2C Controller 8 */
#define MB_I2C_ADDR_POE_30W_QUACK    (0xE0 >> 1)    /* 0x70 (after shifted) mux 0 */
#define MB_I2C_ADDR_POE_30W_CTRLER   (0x40 >> 1)    /* 0x20 (after shifted) mux 1 */

/* FPGA I2C Controller 16 */
#define MB_I2C_ADDR_PCIE_SWITCH  (0xEE >> 1)    /* 0x77 (after shifted) */
#define MB_I2C_ADDR_PLX_PCIE_SWITCH (0x7E >> 1) /* 0x3f (after shifted) */
#define MB_I2C_ADDR_GE_SWITCH    (0x88 >> 1)    /* 0x44 (after shifted) */
#define MB_I2C_ADDR_GH_GESW_CLK  (0xDC >> 1)    /* 0x6E (after shifted) */

/* For Juno only platform_psu.c */
#define MB_I2C_ADDR_PEM0_EEPROM      (0xA6 >> 1)    /* 0x53 (after shifted) */
#define MB_I2C_ADDR_PEM0_MCNTRL      (0xB6 >> 1)    /* 0x58 (after shifted) */
#define MB_I2C_ADDR_PEM1_EEPROM      (0xA6 >> 1)    /* 0x53 (after shifted) */
#define MB_I2C_ADDR_PEM1_MCNTRL      (0xB6 >> 1)    /* 0x58 (after shifted) */

/* UTAH defined address                     */
#define MB_I2C_ADDR_SMLINK       (0x96 >> 1)    /* 0x4b (after shifted) - Rangeley SMBUS2/PECI TBD */  

/* FGPA I2C Controller 17 */
#define MB_I2C_ADDR_SFP_MUX     (0 >> 1)
#define MB_I2C_ADDR_BEZEL_TEMP0  (0x94 >> 1)
#define MB_I2C_ADDR_BEZEL_TEMP1  (0x96 >> 1)
#define MB_I2C_ADDR_PSU_I2C_MUX  (0xE0 >> 1)
#define MB_I2C_ADDR_SFP_I2C_MUX  (0xE0 >> 1)
#define MB_I2C_ADDR_IO_TEMP0  (0x90 >> 1)
#define MB_I2C_ADDR_IO_TEMP1  (0x92 >> 1)
#define MB_I2C_ADDR_IO_VIRTUAL_NIM_GPIO_EXPANDER 0x1A 

#endif
/*-------------------------------------------------
$Log: i2c_address.h,v $
Revision 1.13  2018/08/30 06:59:43  haohsu
Collapse Vg400-branch to Main Trunk

Revision 1.12  2014/08/14 10:27:00  alpeng
support greyhound gesw clk gen on i2c scan test and its util

Revision 1.11  2014/02/13 19:03:12  mcharon
support act2 authentication on sword

Revision 1.10  2014/01/22 10:30:58  hroni
remove redundant macro MB_I2C_ADDR_POE_PSU1_EEPROM_SD

Revision 1.9  2014/01/07 05:55:59  hroni
support psu diag for sword

Revision 1.8  2013/12/18 02:39:26  hroni
Fix psu mux util and do some clean up

Revision 1.7  2013/12/12 00:37:40  alpeng
fixed compile issue

Revision 1.6  2013/12/11 10:12:40  alpeng
remove usb console i2c test due to rommon issue; 30w poe is not supported on sword and dagger; adding temp sensor i2c test

Revision 1.5  2013/11/19 11:27:59  danchung
Add PEM i2c address of Juno

Revision 1.4  2013/11/18 10:37:09  alpeng
support i2c scan test for PLX on sword/dagger

Revision 1.3  2013/09/09 06:35:02  hroni
1. use 5% margining on 1.2v and 1.0v, will recover to 9% after HW confirm it is safe
2. add show latest read voltage after and before doing margining
3. turn off byte swap in pwr_write()

Revision 1.2  2013/06/13 08:34:56  hroni
add support for SFP mux

Revision 1.1  2013/06/04 07:09:14  hroni
move platform specific files to the corresponding directory (i.e. x86 and utah)

Revision 1.3  2013/05/31 12:51:28  danchung
Add checking board type for Juno.

Revision 1.2  2012/03/28 00:38:17  mcharon
remove forward slash from second line

Revision 1.1.1.1  2012/03/23 23:02:06  ptong
Initial archive of ng_diag module


$Endlog$
*/
