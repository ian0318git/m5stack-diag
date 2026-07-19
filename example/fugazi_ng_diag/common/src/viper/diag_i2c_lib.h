 /* $Id: diag_i2c_lib.h,v 1.2 2018/08/06 02:31:50 harrchan Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/viper/diag_i2c_lib.h,v $
 *------------------------------------------------------------------
 * Filename: diag_i2c_lib.h
 *
 * Description: Diag I2C library header file.
 *
 * Copyright (c) 2013-2018 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#ifndef __I2C_ADDR_H__
#define __I2C_ADDR_H__

/* Intel side I2C Bus number enumeration */
typedef enum {
    CPU_I2C0 = 0,
    CPU_I2C1, 
    CPU_I2C2,
    IOFPGA_I2C,
    I2C_BUS_INVALID,            /* Invalid I2C bus */
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
    MB_I2C_EEPROM = 0,      /* EEPROM SPD */
    MB_I2C_ACT2,            /* Secure Chip */
    MB_I2C_0_INVALID,   /* Invalid I2C */
} MB_I2C0_DEVICE;

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


#define HD_SIZE_2            2
#define HD_SIZE_1            1
/* Externs */
extern void *get_n2g_i2c_if(uint8_t, uint8_t, uint8_t);
extern void *platform_i2c_get_quack(uint8_t, uint8_t);
extern uint32_t init_dimm_i2c_struct(n2g_i2c_dev_t *);
extern int read_i2c_reg(int);
extern int write_i2c_reg(int);
extern int read_i2c(int);
extern int write_i2c(int);
extern void build_i2c_util_menu(void);
void *platform_i2c_get_quack(uint8_t, uint8_t);
extern int viper_i2c_scan_test(int);
#endif                          /* __I2C_ADDR_H__ */

/*------------------------------------------------------------------
$Log: diag_i2c_lib.h,v $
Revision 1.2  2018/08/06 02:31:50  harrchan
Merge viper E2E to the main trunk (CSCvk28469)

Revision 1.1.2.2  2018/06/14 01:28:46  harrchan
Remove Aikido option and Aikido keyword in whole source code

Revision 1.1.2.1  2018/02/27 08:06:44  harrchan
Initial viper application code base


$Endlog$
*/
