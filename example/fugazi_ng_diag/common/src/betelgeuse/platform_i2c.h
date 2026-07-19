/* $Id: platform_i2c.h,v 1.2 2019/01/10 06:36:28 wilbhuan Exp $ 
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/betelgeuse/platform_i2c.h,v $
 *------------------------------------------------------------------
 * 
 * platform_i2c.h
 *
 * Copyright (c) 2018 - 2019 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#ifndef __I2C_ADDR_H__
#define __I2C_ADDR_H__

/* I2C Bus number enumeration */
typedef enum {
    CPU_I2C0 = 0,
    CPU_I2C1,
    CPU_I2C2,
    CPU_I2C3,
    PLUG_FPGA,         /* I2C bus on the Pluggable FPGA */
    IOFPGA_I2C = PLUG_FPGA,
    I2C_BUS_INVALID,            /* Invalid I2C bus */
} I2C_BUS;

/* CPU I2C (CPU_I2C2 bus) */
typedef enum {
    MB_I2C_RTC = 0,             /* RTC DS1337S+ */
    MB_I2C_POE_CTRL,            /* PoE TPS23861PW */
    MB_I2C_POE_EEPROM,          /* PoE EEPROM */
    MB_I2C_MCU,                 /* MCU */
    MB_I2C_MCU_BOOTLOADER,      /* MCU bootloader */
    MB_I2C_WIFI_ACT2,           /* Wifi ACT2 */
    MB_I2C_WIFI_TEMP,           /* Wifi Temp */
    MB_I2C_WIFI_PLAT_TEMP,      /* Wifi Temp */
    MB_I2C_2_INVALID,           /* Invalid I2C */
} MB_I2C2_DEVICE;

/* CPU I2C Master (CPU_I2C1 bus) */
typedef enum {
    MB_I2C_MB_TEMP = 0,         /* Mother Board Temperature Sensor */
    MB_I2C_SFP,                 /* SFP port */
    MB_I2C_SFP_INT_REG,         /* SFP Internal Reg port */
    MB_I2C_1_INVALID,           /* Invalid I2C */
} MB_I2C1_DEVICE;

/* CPU I2C (CPU_I2C0 bus) */
typedef enum {
    MB_I2C_EEPROM = 0,      /* EEPROM SPD */
    MB_I2C_ACT2,            /* Secure Chip */
    MB_I2C_AIKIDO_ACT2,     /* Aikido Secure Chip */
    MB_I2C_0_INVALID,   /* Invalid I2C */
} MB_I2C0_DEVICE;

/* PLUG FPGA I2C Master */
typedef enum {
    PLUG_FPGA_I2C_0_TEMP = 0,   /* Pluggable Temperature Sensor */
    PLUG_FPGA_I2C_0_ACT2,   /* Pluggable ACT2 */
    PLUG_FPGA_I2C_0_GPIO_EXP,   /* Pluggable GPIO Expander */
    PLUG_FPGA_INVALID, /* Invalid I2C */
} PLUG_FPGA_I2C0_DEVICE;

/* Common definition for I2C controller */
#define I2C_CTRL_ZERO        0
#define I2C_CTRL_ONE         1
#define I2C_CTRL_TWO         2
#define I2C_CTRL_THREE       3

/* Common definition for MUX */
#define I2C_MUX_ZERO         0
#define I2C_MUX_ONE          1
#define I2C_MUX_TWO          2
#define I2C_MUX_THREE        3

#define I2CBUS0      "/dev/i2c-0"
#define I2CBUS1      "/dev/i2c-1"
#define I2CBUS2      "/dev/i2c-2"

#define    MAX_RETRY            1
#define    WIFI_ACT2_MAX_RETRY  300 /* 9 secs */ 

#define HD_SIZE_2            2
/* Externs */
extern int plat_x64_i2c_scan_test(int);
extern void *get_n2g_i2c_if(uint8_t, uint8_t, uint8_t);
extern void *platform_i2c_get_quack(uint8_t, uint8_t);
#endif                          /* __I2C_ADDR_H__ */

/*-------------------------------------------------
 * $Log: platform_i2c.h,v $
 * Revision 1.2  2019/01/10 06:36:28  wilbhuan
 * The beginning of Betelgeuse application code.
 *
 *-------------------------------------------------
 */
