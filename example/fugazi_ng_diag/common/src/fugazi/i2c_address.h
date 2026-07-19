/* $Id: i2c_address.h,v 1.2 2021/06/02 08:22:35 iachang Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/fugazi/i2c_address.h,v $
 *------------------------------------------------------------------
 * by: mcharon
 * 5/2008
 *
 * Copyright (c) 2011-2020 by Cisco Systems, Inc.
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
#define USB_I2C_MUX		    0x1	/* USB Console */
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
/* TPS536xx connect to CPU for Fugazi */
#define MB_I2C_ADDR_53659_VCORE       (0xC2 >> 1)    /* 0x61 (after shifted )*/
#define MB_I2C_ADDR_53622_1P05V       (0xC8 >> 1)    /* 0x64 (after shifted )*/
#define MB_I2C_ADDR_53622_0P9VNN      (0xC6 >> 1)    /* 0x63 (after shifted )*/
#define MB_I2C_ADDR_53622_1V          (0xC4 >> 1)    /* 0x62 (after shifted )*/ 
#define MB_I2C_ADDR_53659_3P3V        (0xCC >> 1)    

/*
 * FPGA I2C devices Address
 */
/* FPGA I2C Controller 0 */
#define MB_I2C_ADDR_QUACK        (0xEA >> 1)    /* 0x75 (after shifted) */

/* FPGA I2C Controller 1 */
#define MB_I2C_ADDR_USB_CONSOLE_FW_DL  (0x66 >> 1)    /* 0x33 (after shifted) */
#define MB_I2C_ADDR_USB_CONSOLE  (0xC6 >> 1)    /* 0x63 (after shifted) */

/* FPGA I2C Controller 2 */
#define MB_I2C_ADDR_PWR_SEQ      (0x88 >> 1)    /* 0x44 (after shifted) */
#define MB_I2C_ADDR_SENSOR       (0xB8 >> 1)    /* 0x5C (after shifted) */ /* barometer */
#define MB_I2C_ADDR_MB_TEMP      (0x30 >> 1)    /* 0x18 (after shifted) : MAX1617A */ /* used by i2c_api.c */
#define MB_I2C_ADDR_TEMP1_IN_1   (0x90 >> 1)    /* 0x48 (after shifted) */
#define MB_I2C_ADDR_TEMP2_IN_2   (0x92 >> 1)    /* 0x49 (after shifted) */
#define MB_I2C_ADDR_TEMP3_out_1  (0x94 >> 1)    /* 0x4A (after shifted) */
#define MB_I2C_ADDR_TEMP4_out_2  (0x96 >> 1)    /* 0x4B (after shifted) */
#define MB_I2C_ADDR_SMLINK_0     (0x2C >> 1)    /* 0x16 (after shifted) - Skylake SMLINK 0 Mux 2 */  
#define MB_I2C_ADDR_SMLINK_1     (0x90 >> 1)    /* 0x48 (after shifted) - Skylake SMLINK 1 Mux 1 */  


/* FPGA I2C Controller 4 */
#define MB_I2C_ADDR_PSU1_EEPROM      (0xA6 >> 1)    /* 0x53 (after shifted) */
#define MB_I2C_ADDR_PSU1_MCNTRL      (0xB6 >> 1)    /* 0x5B (after shifted) */
#define MB_I2C_ADDR_PSU2_EEPROM      (0xA6 >> 1)    /* 0x53 (after shifted) */
#define MB_I2C_ADDR_PSU2_MCNTRL      (0xB6 >> 1)    /* 0x5B (after shifted) */
#define MB_I2C_ADDR_PEM0_EEPROM      (0xA6 >> 1)    /* 0x53 (after shifted) */ /* curie should be this one */
#define MB_I2C_ADDR_PEM0_MCNTRL      (0xB6 >> 1)    /* 0x58 (after shifted) */
#define MB_I2C_ADDR_PEM1_EEPROM      (0xA6 >> 1)    /* 0x53 (after shifted) */
#define MB_I2C_ADDR_PEM1_MCNTRL      (0xB6 >> 1)    /* 0x58 (after shifted) */

/* FPGA I2C Controler 5 */
#define MB_I2C_ADDR_CLK_BUFFER       (0xD6 >> 1)    /* 0x6E (after shifted) */
#define MB_I2C_ADDR_SYNCE_REG        (0xB0 >> 1)    /* 0x58 (after shifted) */  /* to access idt8a35004 register */

/* FPGA I2C Controller 16 */
#define MB_I2C_ADDR_USB0_REDRIVER  (0xC0 >> 1)    /* 0x60 (after shifted) */
#define MB_I2C_ADDR_USB1_REDRIVER  (0xC2 >> 1)    /* 0x61 (after shifted) */
#define MB_I2C_ADDR_SYNCE_EEPROM   (0xA0 >> 1)    /* 0x50 (after shifted) */  /* to access idt8a35004 EEPROM */

/* FGPA I2C Controller 17 */
#define MB_I2C_ADDR_PSU_I2C_MUX  (0xE0 >> 1)
#define MB_I2C_ADDR_SFP_I2C_MUX  (0xE0 >> 1)

#endif



/*-------------------------------------------------
 * $Log: i2c_address.h,v $
 * Revision 1.2  2021/06/02 08:22:35  iachang
 * CSCvo59196-33 : Merge Fugazi from ASR1K-main-branch to ISR main trunk
 *
 * Revision 1.1.8.2  2020/08/26 02:37:49  iachang
 * Merge Fugazi code into main trunk
 *
 * Revision 1.1.6.8  2020/08/04 08:37:06  iachang
 * Update Copyright to 2020
 *
 * Revision 1.1.6.7  2019/04/06 01:36:14  letsai
 * 1. Remove unused functions and files.
 * 2. Fix BCM54194 SFP External loopback test.
 * 3. Fix BCM54194 Register test.
 * 4. Fix Voltage Margin Utility.
 * 5. Add function to show system information.
 *
 * Revision 1.1.6.6  2019/04/03 01:24:44  iachang
 * Add SMLink 0/1 into I2C scan.
 *
 * Revision 1.1.6.5  2019/03/28 19:00:34  letsai
 * 1. Modify FPGA interrupt test and utility.
 * 2. Modify I2C address of PSU2.
 * 3. Clean up code.
 * 4. Merge M.2 NVME and M.2 USB tests to combo test.
 *
 * Revision 1.1.6.4  2019/03/25 18:22:47  letsai
 * Modified I2C address of PSU
 *
 * Revision 1.1.6.3  2019/03/18 09:22:23  letsai
 * Fixed 1.Boot flash test 2.I2C scan test 3. FPGA interrupt test
 *
 * Revision 1.1.6.2  2019/03/14 03:48:26  letsai
 * Initial check in.
 *
 *
 *
 *
 * $Endlog$
 * */

