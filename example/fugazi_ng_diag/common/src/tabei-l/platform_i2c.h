 /* $Id: platform_i2c.h,v 1.2 2019/10/17 02:16:26 kehuang2 Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/tabei-l/platform_i2c.h,v $
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

/* Tabei Intel side I2C Bus number enumeration */
typedef enum {
    CPU_I2C0 = 0,       /* Intel North Bridge I2C */
    CPU_I2C1,           /* Intel South Bridge I2C */
    PLUG_FPGA,         /* I2C bus on the Pluggable FPGA */
    IOFPGA_I2C,         /* I2C controller on the IO FPGA */
    I2C_BUS_INVALID,    /* Invalid I2C bus */
} I2C_BUS;

/* CPU I2C (CPU_I2C2 bus) */
typedef enum {
    MB_I2C_RTC = 0,             /* RTC DS1337S+ */
    MB_I2C_MCU,                 /* MCU */
    MB_I2C_2_INVALID,           /* Invalid I2C */
} MB_I2C2_DEVICE;

/* CPU I2C Master (CPU_I2C1 bus) */
typedef enum {
    MB_I2C_MB_TEMP = 0,         /* Mother Board Temperature Sensor */
    MB_I2C_1_INVALID,           /* Invalid I2C */
} MB_I2C1_DEVICE;

/* CPU I2C (CPU_I2C0 bus) */
typedef enum {
    MB_I2C_DIMM0 = 0,   /* EEPROM SPD 0*/
    MB_I2C_DIMM1,       /* EEPROM SPD 1*/
    MB_I2C_EEPROM,      /* EEPROM */
    MB_I2C_0_INVALID,   /* Invalid I2C */
} MB_I2C0_DEVICE;

/* Common definition for I2C controller */
#define I2C_CTRL_ZERO        0
#define I2C_CTRL_ONE         1
#define I2C_CTRL_TWO         2
#define I2C_CTRL_THREE       3
#define I2C_CTRL_FOUR        4
#define I2C_CTRL_FIVE        5

/* Common definition for MUX */
#define I2C_MUX_ZERO         0
#define I2C_MUX_ONE          1
#define I2C_MUX_TWO          2
#define I2C_MUX_THREE        3

#define I2CBUS0      "/dev/i2c-0"

#define    MAX_RETRY            1
#define    WIFI_ACT2_MAX_RETRY  300 /* 9 secs */ 

#define HD_SIZE_1            1
#define HD_SIZE_2            2


/*****  TBD ******/
#define NGWIC_I2C_ADDR_IO_PORT                      (0x1C)
#define NIM_I2C_ADDR_OIR                            (0x4B)   /* OIR after >> 1*/
#define NGWIC_I2C_ADDR_IO_PORT_16b      0x21  /* 0x42 >> 1 */
#define SM_I2C_ADDR_IO_PORT1            0x1A   /* Dummy */
#define VM_I2C_CTRL         15
#define NGIOWIC_I2C_ADDR_ACT2   0x74 /* Quack 0xE8 after >> 1 */
#define NGIOVM_I2C_ADDR_ACT2    0x73    /* 0xE6 after >> 1 */
#define NGIO_I2C_MUX_ACT2       0


#define WIC1_I2C_CTRL   12
#define WIC2_I2C_CTRL   13
#define WIC3_I2C_CTRL   14
#define VM_I2C_CTRL     15

/* Pluggable I2C Device Address */
#include "plug_testcard_host_impl.h"

/* Externs */
extern void *get_n2g_i2c_if(uint8_t, uint8_t, uint8_t);
extern void *platform_i2c_get_quack(uint8_t, uint8_t);
extern void build_i2c_menu(void);
extern boolean g_i2c_read_cterr;
extern void *platform_get_wic_oir(int);
extern uint8_t get_wic_i2c_ctrl(int);
extern uint8_t get_sm_i2c_ctrl(int);
extern void *platform_get_sm_oir (int);
extern void *platform_get_vm_oir(int);
extern void *platform_get_carrier_wic_oir(int);


#endif                          /* __I2C_ADDR_H__ */

/*-------------------------------------------------
 * $Log: platform_i2c.h,v $
 * Revision 1.2  2019/10/17 02:16:26  kehuang2
 * Collapse Tabei-L into main trunk
 *
 * Revision 1.1.4.11  2019/07/09 06:11:32  kehuang2
 * Update I2C bus change and enhence I2C scan coverage
 *
 * Revision 1.1.4.10  2019/05/21 03:18:00  kehuang2
 *
 * 1.SFP EN LED Support base on PreP2B respin
 * 2.Support SFP Mux access utility
 *
 * Revision 1.1.4.9  2019/02/25 07:11:50  meho
 * Support new PIM test-card (PCIe).
 *
 * Revision 1.1.4.8  2018/12/27 07:30:38  harrchan
 * Update I2C scan test
 *
 * Revision 1.1.4.7  2018/11/02 02:39:03  kodko
 * Support cookie read for NIM and PIM modules.
 *
 * Revision 1.1.4.6  2018/10/18 03:17:30  olin2
 * Clean up redefined MACRO
 *
 * Revision 1.1.4.5  2018/10/17 06:14:28  olin2
 * Support FPGA I2C scan
 *
 * Revision 1.1.4.4  2018/10/15 11:48:29  olin2
 * Update for using common slot.c
 *
 * Revision 1.1.4.3  2018/10/09 09:22:05  olin2
 * Initial commit for NIM test
 *
 * Revision 1.1.4.2  2018/10/02 01:50:03  harrchan
 * Initial commit for Tabei-L P1A bring up.
 *
 * $Endlog$
 *-------------------------------------------------
 */
