/* $Id: diag_i2c_lib.h,v 1.2 2021/04/15 00:52:26 achiu2 Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/phoenix/diag_i2c_lib.h,v $
 *------------------------------------------------------------------
 * Filename: diag_i2c_lib.h
 *
 * Description: Diag I2C library header file.
 *
 * Copyright (c) 2013-2019 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#ifndef __DIAG_I2C_LIB_H__
#define __DIAG_I2C_LIB_H__

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
extern int phoenix_i2c_scan_test(int);
extern int build_i2c_scan_menu(boolean);
extern uint32_t n2g_i2c_init(n2g_i2c_if_t *);
extern uint32_t n2g_i2c_open(n2g_i2c_if_t *);
extern uint32_t n2g_i2c_close(n2g_i2c_if_t *);


#define SMBUS_PECI_PROXY_READ_ADDR 0x4B
#define SMBUS_PECI_PROXY_WRITE_ADDR 0x4C
#define SMBUS_PECI_MODE_CMD_CODE 0x62
#define SMBUS_READ_CMD_CODE 0x40
#define SMBUS_PECI_PROXY_GETTEMP_WSIZE 6
#define SMBUS_PECI_PROXY_GETTEMP_RSIZE 5
#define SMBUS_PECI_CMD_BUSY 0x1
#define SMBUS_PECI_CMD_ERR 0x2
#define PHOENIX_PROCESSOR_TJMAX 91

#define REF_TEMP_MASK 0xFF0000
#define REF_TEMP_SHIFT_BIT 16
#define REF_TEMP_TXT_FILE_PATH "/tmp/ref_temp.txt"
#define RDMSR_CPU_REF_TEMP_CMD "rdmsr 0x1A2 > /tmp/ref_temp.txt"

#endif                          /* __I2C_ADDR_H__ */

