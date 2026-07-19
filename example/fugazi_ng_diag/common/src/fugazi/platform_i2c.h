/* $Id: platform_i2c.h,v 1.2 2021/06/02 08:22:35 iachang Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/fugazi/platform_i2c.h,v $
 *------------------------------------------------------------------
 * Filename: platform_i2c.h
 *
 * Description: Platform specific I2C header file.
 *
 * Copyright (c) 2013-2020 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#ifndef __I2C_ADDR_H__
#define __I2C_ADDR_H__

/* Fugazi Intel side I2C Bus number enumeration */
typedef enum {
    CPU_I2C0 = 0,
    CPU_I2C1,
    CPU_I2C2,
    IOFPGA_I2C,
    I2C_BUS_INVALID,/* Invalid I2C bus */
} I2C_BUS;

/* CPU North Bridge I2C Master (CPU_I2C0 bus) */
typedef enum {
    MB_I2C_DIMM0 = 0,   /* DIMM0 */
    MB_I2C_DIMM1,       /* DIMM1 */
    MB_I2C_0_INVALID,   /* Invalid I2C */
} MB_I2C0_DEVICE;

/* CPU South Bridge I2C (CPU_I2C1 bus) */
typedef enum {
    MB_I2C_TPS53659_VCORE = 0,  /* TPS536589 VCore */
    MB_I2C_TPS53622_1P05V,      /* TPS53622 1.05V */
    MB_I2C_TPS53622_0P9VNN,     /* TPS53622 0.9VNN */
    MB_I2C_TPS53622_1V,         /* TPS53622 1.0V */
    MB_I2C_TPS53659_3P3V,       /* TPS53659 3.3V */
    MB_I2C_RTC = 0,             /* RTC */
    MB_I2C_CLK1,                /* CLK1 */
    MB_I2C_CLK2,        /* CLK2 (CK505) */
    MB_I2C_PWR_SEQ,     /* Power Sequencer */
    MB_I2C_VTG_MNTR,    /* Voltage monitor */
    MB_I2C_NITROX,      /* Nitrox */
    MB_I2C_PS1_COOKIE,  /* PSU1 cookie */
    MB_I2C_PS1_OBFL,    /* PSU1 OBFL */
    MB_I2C_PS1_THERM,   /* PSU1 Termal Sensor */
    MB_I2C_PS2_COOKIE,  /* PSU2 cookie */
    MB_I2C_PS2_OBFL,    /* PSU2 OBFL */
    MB_I2C_PS2_THERM,   /* PSU2 Termal Sensor */
    MB_I2C_USB,         /* USB Console */
    MB_I2C_USB_R,       /* USB Console Read/Write */
    MB_I2C_EEPROM,      /* 256 bytes EEPROM */
    MB_I2C_MP_TEMP,     /* Mid Plane Temperature Sensor */
    MB_I2C_MP_COOKIE,   /* Mid Plane cookie */
    MB_I2C_GOOFY_M,     /* Goofy Master */
    MB_I2C_ENV_MCU,     /* Env MCU */
    MB_I2C_MB_TEMP,     /* Mother Board Temperature Sensor */
    MB_I2C_CPU_CLKBUF,  /* Clock buffer for CPU and MCH */
    MB_I2C_1_INVALID,   /* Invalid I2C */
} MB_I2C1_DEVICE;

/* IO FPGA I2C Master */
typedef enum {
    IOFPGA_I2C_QUACK = 0,   /* Mother Board Quack */
    IOFPGA_I2C_INVALID, /* Invalid I2C */
} IOFPGA_I2C_DEVICE;


#define I2CBUS0      "/dev/i2c-0"

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

/* Number of SFP/PSU definitions */
#define SFP_PSU_ZERO    0
#define SFP_PSU_ONE     1
#define SFP_PSU_TWO     2
#define SFP_PSU_THREE   3
#define SFP_PSU_INVALID (-1)
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
    FUGAZI_SFP_I2C_MUX = 0,
    FUGAZI_PSU_I2C_MUX,
    FUGAZI_INVALID_I2C_MUX,
} FUGAZI_I2C_MUX_ID;


/* PSU */
#define FUGAZI_PSU_OFF    1

#define FUGAZI_PSU1       1
#define FUGAZI_PSU2       2
#define FUGAZI_PSU1_TRUE       ((FUGAZI_PSU1 << FUGAZI_PSU_OFF) | TRUE)
#define FUGAZI_PSU1_FALSE      ((FUGAZI_PSU1 << FUGAZI_PSU_OFF) | FALSE)
#define FUGAZI_PSU2_TRUE       ((FUGAZI_PSU2 << FUGAZI_PSU_OFF) | TRUE)
#define FUGAZI_PSU2_FALSE      ((FUGAZI_PSU2 << FUGAZI_PSU_OFF) | FALSE)

#define HD_SIZE_2            2
#define HD_SIZE_1            1

/* CPU I2C Controller 0 Slave Device Addresses */
#define MB_I2C_ADDR_DIMM0      (0xA0 >> 1)     /* DIMM0 */
#define MB_I2C_ADDR_DIMM1      (0xA4 >> 1)     /* DIMM1 */

/* Include */
#include "i2c_api.h"
#include "i2c_dev.h"

/* Externs */
extern void *get_n2g_i2c_if(uint8_t, uint8_t, uint8_t);
extern void *platform_i2c_get_quack(uint8_t, uint8_t);
extern int fugazi_i2c_scan_test(int);
extern int read_i2c_reg(int);
extern int write_i2c_reg(int);
extern void build_i2c_menu(void);
void *platform_i2c_get_quack(uint8_t, uint8_t);
extern uint32_t init_tps536xx_i2c_struct (n2g_i2c_dev_t *i2c_dev, uint32_t);
#endif                          /* __I2C_ADDR_H__ */

/*-------------------------------------------------
 * $Log: platform_i2c.h,v $
 * Revision 1.2  2021/06/02 08:22:35  iachang
 * CSCvo59196-33 : Merge Fugazi from ASR1K-main-branch to ISR main trunk
 *
 * Revision 1.1.8.2  2020/08/26 02:37:51  iachang
 * Merge Fugazi code into main trunk
 *
 * Revision 1.1.6.4  2020/08/04 08:37:06  iachang
 * Update Copyright to 2020
 *
 * Revision 1.1.6.3  2019/04/06 01:36:14  letsai
 * 1. Remove unused functions and files.
 * 2. Fix BCM54194 SFP External loopback test.
 * 3. Fix BCM54194 Register test.
 * 4. Fix Voltage Margin Utility.
 * 5. Add function to show system information.
 *
 * Revision 1.1.6.2  2019/03/14 03:48:27  letsai
 * Initial check in.
 *
 *
 *
 * $Endlog$
 */
