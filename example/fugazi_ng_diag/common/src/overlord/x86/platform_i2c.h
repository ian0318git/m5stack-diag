/* $Id: platform_i2c.h,v 1.17 2020/05/22 02:28:44 qingcwan Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/overlord/x86/platform_i2c.h,v $
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
    PLUG_FPGA,          /* I2C bus on the Pluggable FPGA */
    IOFPGA_I2C,         /* I2C controller on the IO FPGA */
    MOD_IOFPGA_I2C,     /* I2C controller on modules IO FPGA */
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

#define OVLD_MENU_OPT_MSK    0x1

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

/* Common definition for MUX */
#define I2C_MUX_ZERO         0
#define I2C_MUX_ONE          1
#define I2C_MUX_TWO          2
#define I2C_MUX_THREE        3

/* 30W POE QUACK */
#define I2C_CTRL_POE_30W_QUACK  I2C_CTRL_NIGHT
#define I2C_MUX_POE_30W_QUACK   I2C_MUX_ZERO

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
 * Map chip specific mux port mask with generic macro
 */

#define I2C1_MUX_PORT0_MASK        MUX9545_PORT0_MASK
#define I2C1_MUX_PORT1_MASK        MUX9545_PORT1_MASK
#define I2C1_MUX_PORT2_MASK        MUX9545_PORT2_MASK
#define I2C1_MUX_PORT3_MASK        MUX9545_PORT3_MASK
#define I2C1_MUX_PORT_NULL_MASK    MUX9545_PORT_NULL_MASK
#define I2C1_MUX_PORT_ALL_MASK     MUX9545_PORT_ALL_MASK

#define MIDPLANE_I2C_COOKIE	MB_I2C_MP_COOKIE
#define MIDPLANE_I2C_TEMP	MB_I2C_MP_TEMP
#define PS1_I2C_COOKIE		MB_I2C_PS1_COOKIE
#define PS1_I2C_OBFL		MB_I2C_PS1_OBFL
#define PS1_I2C_THERM		MB_I2C_PS1_THERM
#define PS2_I2C_COOKIE		MB_I2C_PS2_COOKIE
#define PS2_I2C_OBFL		MB_I2C_PS2_OBFL
#define PS2_I2C_THERM		MB_I2C_PS2_THERM

/*
 * I2C Device address defines
 * 
 * Notes: These are the 7-bit address without the R/W bit
 */
/* Cavecreek PCH  MST_SMB Address */
#define MB_I2C_ADDR_CPU_CLKBUF (0xD2 >> 1)	/* CPU and MCH CLKBUF */
#define MB_I2C_ADDR_PCIE_CLK 	 (0xD8 >> 1)	/* PCIE Clock */
#define GB_MB_I2C_ADDR_PCIE_CLK  (0xDA >> 1)	/* PCIE Clock with GOLDBEACH */

/* FPGA Bus 0 Address */

/* FPGA Bus 1 Address */


/* CPU I2C Controller 0 Slave Device Addresses */
#define MB_I2C_ADDR_DIMM0      (0xA0 >> 1)     /* DIMM0 */
#define MB_I2C_ADDR_DIMM1      (0xA4 >> 1)     /* DIMM1 */

/* CPU I2C Controller 1 Slave Device Addresses */
/* IOFPGA Slave Device Addresses */


/*	Master Goofy Port 1 */
#define TDM_I2C_ADDR		0x54	/* TDM/PLL */

/*	SM - Master Goofy Ports 2-5 */

#define SM1_I2C_CTRL	10
#define SM2_I2C_CTRL	11
#define WIC1_I2C_CTRL	12
#define WIC2_I2C_CTRL	13
#define WIC3_I2C_CTRL	14
#define VM_I2C_CTRL	15


//#define SM_I2C_ADDR_OBFL	0x50	/* OBFL */

#define NGIOSM_I2C_ADDR_ACT2	0x75	/* Quack */
#define NGIOSM_I2C_ADDR_OIR	0x4A	/* OIR */
#define NGIOSM_VM_DC_I2C_ADDR_ACT2	0x73    /* Quack */
#define NGIOWIC_I2C_ADDR_ACT2	0x74	/* Quack after >> 1*/
#define NGIOWIC_I2C_ADDR_OIR	0x4B	/* OIR after >> 1*/

#define NGIOVM_I2C_ADDR_ACT2    0x73    /* 0xE6 after >> 1 */
#define NGIOVM_I2C_ADDR_OIR	0x4B	/* OIR after >> 1*/

#define NGIO_I2C_MUX_ACT2	0
#define DC_QUACK	0

/* Common defines */
#define ONE			1
#define I2C_BUF_MAX		512	/* overlord support up to 512
					 */

/* PSU */
#define OVLD_PSU_OFF    1

#define OVLD_PSU1       1
#define OVLD_PSU2       2
#define OVLD_PSU1_TRUE       ((OVLD_PSU1 << OVLD_PSU_OFF) | TRUE)
#define OVLD_PSU1_FALSE      ((OVLD_PSU1 << OVLD_PSU_OFF) | FALSE)
#define OVLD_PSU2_TRUE       ((OVLD_PSU2 << OVLD_PSU_OFF) | TRUE)
#define OVLD_PSU2_FALSE      ((OVLD_PSU2 << OVLD_PSU_OFF) | FALSE)

/* 12V PoE PSU */
#define OVLD_POE_PSU_OFF   1

#define OVLD_POE_PSU1   1
#define OVLD_POE_PSU2   2
#define OVLD_POE_PSU1_TRUE    ((OVLD_POE_PSU1 << OVLD_POE_PSU_OFF) | TRUE)
#define OVLD_POE_PSU1_FALSE   ((OVLD_POE_PSU1 << OVLD_POE_PSU_OFF) | FALSE)
#define OVLD_POE_PSU2_TRUE    ((OVLD_POE_PSU2 << OVLD_POE_PSU_OFF) | TRUE)
#define OVLD_POE_PSU2_FALSE   ((OVLD_POE_PSU2 << OVLD_POE_PSU_OFF) | FALSE)

/* SM */
#define SM_I2C_ADDR_IO_PORT             0x20
#define SM_I2C_ADDR_IO_PORT1            0x1A

#define NGWIC_I2C_ADDR_IO_PORT          0x1c
#define NGWIC_I2C_ADDR_IO_PORT_16b      0x21  /* 0x42 >> 1 */
#define NGDC_I2C_ADDR_IO_PORT           0x1F  /* 3E >> 1 */
#define NGDC_I2C_ADDR_IO_PORT1          0x1E  /* 3C >> 1 */

/* Function prototypes */
extern int platform_i2c_init(void);
extern int init_mux(int, int);
extern int init_muxes(int);
extern void reset_muxes(void);
extern uint8_t get_wic_i2c_ctrl(int);
extern uint8_t get_sm_i2c_ctrl(int);
extern void *get_n2g_i2c_if(uint8_t, uint8_t, uint8_t);
extern void *platform_get_sm_oir(int);
extern void *platform_get_wic_oir(int);
extern void *platform_get_sm_act2(int);
extern void *platform_get_wic_act2(int);
extern void *platform_get_vm_oir(int);
extern void *platform_i2c_get_quack(uint8_t, uint8_t);
extern void *platform_get_dc_oir(int);
extern void *platform_get_carrier_wic_oir(int);

#endif /* __I2C_ADDR_H__ */

/*------------------------------------------------------------------
$Log: platform_i2c.h,v $
Revision 1.17  2020/05/22 02:28:44  qingcwan
Merge switzer-carrier code into main chunk.

Revision 1.16  2019/08/06 06:56:16  alpeng
merge curie, switzer and nightwatch to trunk

Revision 1.15.36.1  2018/10/15 10:45:10  alpeng
fixed compile error; add PLUG_FPGA define for i2c, naming confliction for tam lib and act2 lib

Revision 1.15  2017/08/10 10:12:45  iachang
CSCvf44161: Merge Goldbeach into USD platform as one image

Revision 1.14  2017/07/28 07:49:44  harrchan
Collapse Oakenshield-branch to Main Trunk.

Revision 1.13  2014/07/01 08:58:51  bowang3
Add functions to support NGSM carrier card Thule

Revision 1.12  2014/05/23 09:57:03  danchung
support 16-bit GPIO expander for NGWIC

Revision 1.11  2013/09/14 02:36:25  alpeng
support ACT2 on modules

Revision 1.10  2013/07/18 02:52:21  hroni
Fix Alter PoE cookie: remove the hard coded mux and ctrl ids, define the appropriate mux and ctrl ids in platform_i2c.h

Revision 1.9  2013/06/04 07:09:15  hroni
move platform specific files to the corresponding directory (i.e. x86 and utah)

Revision 1.1  2013/05/09 05:42:39  alpeng
moving overlord common code from x86

Revision 1.7  2012/09/26 18:02:14  palin2
Uniformed the print out format of I2C devices defult tests.

Revision 1.6  2012/05/16 07:29:24  srane
Daughter card support.

Revision 1.5  2012/05/04 20:01:46  mcharon
use void* instead of int as argument to to func ptrs in ngio_intf

Revision 1.4  2012/04/16 15:29:26  palin2
Update 12V PoE PSU tests and utilities based on HW team's request:
1) Add "Registers test" support.
2) Add "PoE PSU" info into bootlog message.
3) Add utility to verified FPGA related PoE PSU detect function.

Revision 1.3  2012/03/28 00:38:23  mcharon
remove forward slash from second line

Revision 1.2  2012/03/25 03:58:33  palin2
Clean up Overlord Intel side (x86) compile warnings.

Revision 1.1.1.1  2012/03/23 23:02:09  ptong
Initial archive of ng_diag module


$Endlog$
*/
