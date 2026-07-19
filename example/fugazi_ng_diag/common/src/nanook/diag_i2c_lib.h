 /* $Id: diag_i2c_lib.h,v 1.2 2019/12/11 10:10:30 lucywang Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/nanook/diag_i2c_lib.h,v $
 *------------------------------------------------------------------
 * Filename: diag_i2c_lib.h
 *
 * Description: Diag I2C library header file.
 *
 * Copyright (c) 2019 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#ifndef __I2C_ADDR_H__
#define __I2C_ADDR_H__



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
extern int nanook_i2c_scan_test(int);
extern int build_i2c_scan_menu(boolean);
extern uint32_t n2g_i2c_init(n2g_i2c_if_t *);
extern uint32_t n2g_i2c_open(n2g_i2c_if_t *);
extern uint32_t n2g_i2c_close(n2g_i2c_if_t *);

#endif                          /* __I2C_ADDR_H__ */

/*------------------------------------------------------------------
$Log: diag_i2c_lib.h,v $
Revision 1.2  2019/12/11 10:10:30  lucywang
Merged Nanook to main trunk


$Endlog$
*/
