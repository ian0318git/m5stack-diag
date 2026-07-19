/* $Id: platform_i2c.h,v 1.2 2019/06/14 05:24:51 mikech2 Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/katar/x86/platform_i2c.h,v $
 *------------------------------------------------------------------
 * Filename: platform_i2c.h
 *
 * Description: Platform specific I2C header file.
 *
 * Copyright (c) 2013-2019 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#ifndef __I2C_ADDR_H__
#define __I2C_ADDR_H__

/* Overlord Intel side I2C Bus number enumeration */
typedef enum {
    CPU_I2C0 = 0,	/* Intel North Bridge I2C */
    CPU_I2C1,		/* Intel South Bridge I2C */
    IOFPGA_I2C,         /* I2C controller on the IO FPGA */
    I2C_BUS_INVALID,	/* Invalid I2C bus */
} I2C_BUS;

/* CPU North Bridge I2C Master (CPU_I2C0 bus) */
typedef enum {
    MB_I2C_DIMM0 = 0,	/* DIMM0 */
    MB_I2C_DIMM1,	/* DIMM1 */
    MB_I2C_0_INVALID,	/* Invalid I2C */
} MB_I2C0_DEVICE;

/* CPU South Bridge I2C (CPU_I2C1 bus) */
typedef enum {
    MB_I2C_RTC = 0,	/* RTC */
    MB_I2C_CLK1,	/* CLK1 */
    MB_I2C_CLK2,	/* CLK2 (CK505) */
    //MB_I2C_PWR_SEQ,	/* Power Sequencer */
    MB_I2C_VTG_MNTR,	/* Voltage monitor */
    MB_I2C_NITROX,	/* Nitrox */
    MB_I2C_PS1_COOKIE,	/* PSU1 cookie */
    MB_I2C_PS1_OBFL,	/* PSU1 OBFL */
    MB_I2C_PS1_THERM,	/* PSU1 Termal Sensor */
    MB_I2C_PS2_COOKIE,	/* PSU2 cookie */
    MB_I2C_PS2_OBFL,	/* PSU2 OBFL */
    MB_I2C_PS2_THERM,	/* PSU2 Termal Sensor */
    MB_I2C_USB,		/* USB Console */
    MB_I2C_USB_R,	/* USB Console Read/Write */
    MB_I2C_EEPROM,	/* 256 bytes EEPROM */
    MB_I2C_MP_TEMP,	/* Mid Plane Temperature Sensor */
    MB_I2C_MP_COOKIE,	/* Mid Plane cookie */
    MB_I2C_GOOFY_M,	/* Goofy Master */
    //    MB_I2C_ENV_MCU,	/* Env MCU */
    MB_I2C_MB_TEMP,	/* Mother Board Temperature Sensor */
    MB_I2C_CPU_CLKBUF,  /* Clock buffer for CPU and MCH */
    MB_I2C_SM_CLKBUF,   /* Clock buffer for SM PCIe */
    MB_I2C_PLX_BRIDGE,	/* PLX PCIe bridge */
    MB_I2C_DIODE,	/* Diode Sensor */ 
    MB_I2C_POE_30W,	/* POE 30W */
    MB_I2C_54V_POE1,	/* First -54v POE */ 
    MB_I2C_54V_POE2,	/* Second -54v POE */ 
    MB_I2C_1_INVALID,	/* Invalid I2C */
} MB_I2C1_DEVICE;

/* IO FPGA I2C Master */
typedef enum {
    IOFPGA_I2C_QUACK = 0,	/* Mother Board Quack */
    IOFPGA_I2C_INVALID,	/* Invalid I2C */
} IOFPGA_I2C_DEVICE;

#define MB_I2C_QUACK   IOFPGA_I2C_QUACK

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
#define I2C_CTRL_MAX 19

/* Common definition for MUX */
#define I2C_MUX_ZERO         0
#define I2C_MUX_ONE          1
#define I2C_MUX_TWO          2
#define I2C_MUX_THREE        3
#define I2C_MUX_MAX I2C_MUX_THREE

/* Neptune - 30W POE QUACK */
#define I2C_CTRL_POE_30W_QUACK  I2C_CTRL_EIGHT
#define I2C_MUX_POE_30W_QUACK   I2C_MUX_ONE

/* Number of SFP/PSU definitions */
#define SFP_PSU_ZERO    0
#define SFP_PSU_ONE     1
#define SFP_PSU_TWO     2
#define SFP_PSU_THREE   3
#define SFP_PSU_INVALID   (-1)

/*
 * Informers has 2 mux connected to CPU I2C bus I2C1.
 * This structure is used in i2c_api.c to map the
 * mux port connection of all the devices on bus I2C1.
 */
#define I2C1_MUX_NUM    2

#define MB_I2C_MUX0     I2C_MUX_ZERO
#define MB_I2C_MUX1     I2C_MUX_ONE

#define I2C1_MUX_0      0
#define I2C1_MUX_1      1
typedef struct i2c1_mux_mask_t_ {
    uint8_t mux0_mask;
    uint8_t mux1_mask;
} i2c1_mux_mask_t;

/*
 * I2C Device address defines
 * 
 * Notes: These are the 7-bit address without the R/W bit
 */
/* Cavecreek PCH  MST_SMB Address */
#define MB_I2C_ADDR_CPU_CLKBUF (0xD2 >> 1)	/* CPU and MCH CLKBUF */
#define MB_I2C_ADDR_PCIE_CLK 	 (0xD8 >> 1)	/* PCIE Clock */
#define GB_MB_I2C_ADDR_PCIE_CLK  (0xDA >> 1)    /* PCIE Clock with GOLDBEACH */

/* FPGA Bus 0 Address */

/* FPGA Bus 1 Address */


/* CPU I2C Controller 0 Slave Device Addresses */
#define MB_I2C_ADDR_DIMM0      (0xA0 >> 1)     /* DIMM0 */
#define MB_I2C_ADDR_DIMM1      (0xA4 >> 1)     /* DIMM1 */

/* CPU I2C Controller 1 Slave Device Addresses */
/* IOFPGA Slave Device Addresses */


/*	Master Goofy Port 1 */
#define TDM_I2C_ADDR		0x54	/* TDM/PLL */

/* Common defines */
#define ONE			1
#define I2C_BUF_MAX		512

/* Function prototypes */
extern void *platform_i2c_get_quack(uint8_t, uint8_t);

#endif /* __I2C_ADDR_H__ */

/*
 *------------------------------------------------------------------
 * $Log: platform_i2c.h,v $
 * Revision 1.2  2019/06/14 05:24:51  mikech2
 * Collapse katar-branch00 to Main Trunk
 *
 * Revision 1.1.2.2  2019/04/30 06:06:59  mikech2
 * Code cleanup
 *
 * Revision 1.1.2.1  2018/10/22 08:02:33  mikech2
 * Move project folder to common/src/katar/x86
 *
 * Revision 1.1.2.1  2018/06/07 01:19:23  peteteng
 * add project katar - based on neptune
 *
 *
 *
 *------------------------------------------------------------------
 * $Endlog$
 */

