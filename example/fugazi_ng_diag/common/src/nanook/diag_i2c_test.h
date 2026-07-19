 /* $Id: diag_i2c_test.h,v 1.2 2019/12/11 10:10:30 lucywang Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/nanook/diag_i2c_test.h,v $
 *------------------------------------------------------------------
 * Filename: diag_i2c_test.h
 *
 * Description: Platform specific I2C header file.
 *
 * Copyright (c) 2019 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */
#ifndef __DIAG_I2C_TEST_H__
#define __DIAG_I2C_TEST_H__

#define    MAX_RETRY             1
#define    DELAY_I2C_SCAN_RETRY  30

/* For I2C scan test, DIMM item is seperate form others device */
typedef enum {
    DIMM0_TEST = 0,
    DIMM1_TEST,
    DEFAULT_TEST,
} I2C_SCAN_TEST_OPTION;

extern int nanook_i2c_scan_test(int);
extern int build_i2c_scan_menu(boolean);

#endif                          /* __DIAG_I2C_TEST_H__ */

/*------------------------------------------------------------------
$Log: diag_i2c_test.h,v $
Revision 1.2  2019/12/11 10:10:30  lucywang
Merged Nanook to main trunk


$Endlog$
*/
