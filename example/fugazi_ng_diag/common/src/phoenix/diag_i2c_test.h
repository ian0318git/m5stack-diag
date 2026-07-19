/* $Id: diag_i2c_test.h,v 1.2 2021/04/15 00:52:26 achiu2 Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/phoenix/diag_i2c_test.h,v $
 *------------------------------------------------------------------
 * Filename: diag_i2c_test.h
 *
 * Description: Platform specific I2C header file.
 *
 * Copyright (c) 2013-2019 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */
#ifndef __DIAG_I2C_TEST_H__
#define __DIAG_I2C_TEST_H__

#define MAX_RETRY_I2C_SCAN 1
#define DELAY_I2C_SCAN_RETRY 30

extern int diag_i2c_scan_test(int);
extern int build_i2c_scan_menu(boolean);

#endif                          /* __DIAG_I2C_TEST_H__ */

