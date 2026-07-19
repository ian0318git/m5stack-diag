/* $Id: platform_i2c.h,v 1.2 2020/05/22 02:28:38 qingcwan Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/overlord/neptune/curie_2RU/platform_i2c.h,v $
 *------------------------------------------------------------------
 * Filename: platform_i2c.h
 *
 * Description: Platform specific I2C header file.
 *
 * Copyright (c) 2013-2018 by Cisco Systems, Inc.
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
    PLUG_FPGA,         /* I2C bus on the Pluggable FPGA */
    MOD_IOFPGA_I2C,     /* I2C controller on modules IO FPGA */
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
    MB_I2C_IR3570_VCORE = 0,/* IR3570 VCore */
    MB_I2C_IR3570_VCCSCSUS, /* IR3570 VCCSCSUS */
    MB_I2C_IR3570_VCCGBE,   /* IR3570 VCCGBE */
    MB_I2C_IR3570_1_2V,     /* IR3570 1.2V */
    MB_I2C_TPS536XX_VCORE_0P85_VCCSA = 0, /* TPS53659 Vcore/0.85_VCCSA */
    MB_I2C_TPS536XX_1P0V,                 /* TPS53622 1.0V */
    MB_I2C_TPS536XX_1P2V_0P9VNN,          /* TPS53622 1.2V/0.9VNN */
    MB_I2C_TPS536XX_1P2V_1P05,            /* TPS53622 1.2V/1.05 */
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
#define I2C_CTRL_SEVENTEEN  17
#define I2C_CTRL_EIGHTEEN   18
#define I2C_CTRL_NINETEEN   19
#define I2C_CTRL_TWENTY     20
#define I2C_CTRL_MAX        I2C_CTRL_TWENTY

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
 * Map chip specific mux port mask with generic macro
 */

#define I2C1_MUX_PORT0_MASK        MUX9545_PORT0_MASK
#define I2C1_MUX_PORT1_MASK        MUX9545_PORT1_MASK
#define I2C1_MUX_PORT2_MASK        MUX9545_PORT2_MASK
#define I2C1_MUX_PORT3_MASK        MUX9545_PORT3_MASK
#define I2C1_MUX_PORT_NULL_MASK    MUX9545_PORT_NULL_MASK
#define I2C1_MUX_PORT_ALL_MASK     MUX9545_PORT_ALL_MASK

/* I2C MUX */
typedef enum {
    OVLD_SFP_I2C_MUX = 0,
    OVLD_PSU_I2C_MUX,
    OVLD_INVALID_I2C_MUX,
} OVLD_I2C_MUX_ID;

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

/*	SM - Master Goofy Ports 2-5 */

#define SM1_I2C_CTRL	10
#define SM2_I2C_CTRL	11
#define SM3_I2C_CTRL	18
#define SM4_I2C_CTRL	19
#define WIC1_I2C_CTRL	12
#define WIC2_I2C_CTRL	13
#define WIC3_I2C_CTRL	14
#define VM_I2C_CTRL	15


//#define SM_I2C_ADDR_OBFL	0x50	/* OBFL */

#define NGIOSM_I2C_ADDR_ACT2	0x75	/* Quack */
#define NGIOSM_I2C_ADDR_OIR	0x4A	/* OIR */
#define NGIOSM_VM_DC_I2C_ADDR_ACT2      0x73    /* Quack */

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

/*
 *-----------------------------------------------------------------------------
$Log: platform_i2c.h,v $
Revision 1.2  2020/05/22 02:28:38  qingcwan
Merge switzer-carrier code into main chunk.

Revision 1.1  2020/01/09 01:02:02  jiajliu
Merge Curie 2RU to main trunk

$Endlog$
 *-----------------------------------------------------------------------------
 */
