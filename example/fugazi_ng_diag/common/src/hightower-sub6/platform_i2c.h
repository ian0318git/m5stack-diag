/* $Id: platform_i2c.h,v 1.2 2021/06/02 02:56:24 alpeng Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/hightower-sub6/platform_i2c.h,v $
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

/* Slave address defination */
/* I2C 0 Device Addresses */

/* I2C 0 Device Addresses */
#define MB_I2C_ADDR_CPLD       0x22 
#define MB_I2C_BUS_CPLD        CPU_I2C0
#define MB_I2C_CTRL_CPLD       CPU_I2C0
#define MB_I2C_MUX_CPLD        I2C_MUX_ZERO


/* ACT2 Lite */
#define MB_I2C_ADDR_ACT2       0x74 /*(0xE8 >> 1) */
#define MB_I2C_BUS_ACT2        CPU_I2C2
#define MB_I2C_CTRL_ACT2       CPU_I2C2
#define MB_I2C_MUX_ACT2        I2C_MUX_ZERO

/* RTC */
#define MB_I2C_ADDR_RTC        0x68 /*(0x68 >> 1) */
#define MB_I2C_BUS_RTC         CPU_I2C2
#define MB_I2C_CTRL_RTC        CPU_I2C2
#define MB_I2C_MUX_RTC         I2C_MUX_ZERO

/* TMP75 */
#define MB_I2C_ADDR_TMP75      0x48 /*(0x48 >> 1) */
#define MB_I2C_BUS_TMP75       CPU_I2C2
#define MB_I2C_CTRL_TMP75      CPU_I2C2
#define MB_I2C_MUX_TMP75       I2C_MUX_ZERO


/* I2C Bus number enumeration */
typedef enum {
    CPU_I2C0 = 0,
    CPU_I2C1,
    CPU_I2C2,
    I2C_BUS_INVALID,            /* Invalid I2C bus */
} I2C_BUS;

/* CPU I2C (CPU_I2C0 bus) */
typedef enum {
    MB_I2C_CPLD,
    MB_I2C_0_INVALID,   /* Invalid I2C */
} MB_I2C0_DEVICE;

/* CPU I2C (CPU_I2C1 bus) */
typedef enum {
    MB_I2C_1_INVALID,         /* Invalid I2C */
} MB_I2C1_DEVICE;

/* CPU I2C (CPU_I2C2 bus) */
typedef enum {
    MB_I2C_ACT2,         /* ACT2 */
    MB_I2C_RTC,      /* RTC */
    MB_I2C_TMP75,        /* TMP76 */
    MB_I2C_2_INVALID,           /* Invalid I2C */
} MB_I2C2_DEVICE;

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

#define HD_SIZE_2            2
/* Externs */
extern int highrise_i2c_scan_test(int);
extern int highrise_i2c_reg_rw_test(int);
extern void *get_n2g_i2c_if(uint8_t, uint8_t, uint8_t);
extern void *platform_i2c_get_quack(uint8_t, uint8_t);
extern void build_i2c_menu(void);
extern boolean g_i2c_read_cterr;
#endif                          /* __I2C_ADDR_H__ */

/*********************************************************************
 * $Log: platform_i2c.h,v $
 * Revision 1.2  2021/06/02 02:56:24  alpeng
 * merge sears into trunk
 *
 * Revision 1.1.4.2  2020/12/09 06:35:02  alpeng
 * add cvs log field
 *
 *
 */
