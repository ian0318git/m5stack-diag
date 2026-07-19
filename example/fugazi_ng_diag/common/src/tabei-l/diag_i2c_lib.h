 /* $Id: diag_i2c_lib.h,v 1.3 2019/11/25 08:55:51 kehuang2 Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/tabei-l/diag_i2c_lib.h,v $
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

typedef enum {
    SFP0 = 0,
    SFP1
} I2C_WHICH_SFP_OPTION;


/* Externs */
extern void *get_n2g_i2c_if(uint8_t, uint8_t, uint8_t);
extern void *platform_i2c_get_quack(uint8_t, uint8_t);
extern uint32_t init_dimm_i2c_struct(n2g_i2c_dev_t *);
extern int read_i2c_reg(int);
extern int write_i2c_reg(int);
extern int read_i2c(int);
extern int write_i2c(int);
extern int switch_sfp_mux(int);
extern void build_i2c_util_menu(void);
void *platform_i2c_get_quack(uint8_t, uint8_t);
extern int tabei_i2c_scan_test(int);
extern int build_i2c_scan_menu(boolean);
extern int switch_sfp_mux_util(void);
extern uint32_t n2g_i2c_init(n2g_i2c_if_t *);
extern uint32_t n2g_i2c_open(n2g_i2c_if_t *);
extern uint32_t n2g_i2c_close(n2g_i2c_if_t *);

#endif                          /* __I2C_ADDR_H__ */

/*------------------------------------------------------------------
$Log: diag_i2c_lib.h,v $
Revision 1.3  2019/11/25 08:55:51  kehuang2
Collapse Tabei-L into main trunk

Revision 1.2  2019/10/17 02:16:22  kehuang2
Collapse Tabei-L into main trunk

Revision 1.1.2.6  2019/08/26 07:55:00  kehuang2
Clean up code by the comment of code review

Revision 1.1.2.5  2019/07/09 06:11:31  kehuang2
Update I2C bus change and enhence I2C scan coverage

Revision 1.1.2.4  2018/10/24 10:45:17  harrchan
Seperate DIMM test from other I2C device

Revision 1.1.2.3  2018/10/18 03:17:30  olin2
Clean up redefined MACRO

Revision 1.1.2.2  2018/10/09 09:22:04  olin2
Initial commit for NIM test

Revision 1.1.2.1  2018/10/02 01:49:59  harrchan
Initial commit for Tabei-L P1A bring up.

$Endlog$
*/
