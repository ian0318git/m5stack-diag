/* $Id: i2c_address.h,v 1.1 2020/01/09 01:01:59 jiajliu Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/overlord/neptune/curie_2RU/i2c_address.h,v $
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

/* CPU I2C Controller 1 Slave Device Addresses */
/* TPS536XX connect to CPU for Curie 2RU */
#define MB_I2C_ADDR_TPS536XX_VCORE_0P85_VCCSA   (0xC2 >> 1) /* 0x61 (after shifted )*/
#define MB_I2C_ADDR_TPS536XX_1P0V               (0xC4 >> 1) /* 0x62 (after shifted )*/
#define MB_I2C_ADDR_TPS536XX_1P2V_0P9VNN        (0xC6 >> 1) /* 0x63 (after shifted )*/
#define MB_I2C_ADDR_TPS536XX_1P2V_1P05          (0xC8 >> 1) /* 0x64 (after shifted )*/

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
#define MB_I2C_ADDR_PSU1_EEPROM      (0xA4 >> 1)    /* 0x52 (after shifted) */
#define MB_I2C_ADDR_PSU1_MCNTRL      (0xA6 >> 1)    /* 0x53 (after shifted) */
#define MB_I2C_ADDR_PSU2_EEPROM      (0xA4 >> 1)    /* 0x52 (after shifted) */
#define MB_I2C_ADDR_PSU2_MCNTRL      (0xA6 >> 1)    /* 0x53 (after shifted) */
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

/*
 *-----------------------------------------------------------------------------
$Log: i2c_address.h,v $
Revision 1.1  2020/01/09 01:01:59  jiajliu
Merge Curie 2RU to main trunk

$Endlog$
 *-----------------------------------------------------------------------------
 */
