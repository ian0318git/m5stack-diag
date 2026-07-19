 /* $Id: diag_i2c_test.h,v 1.2 2019/10/17 02:16:22 kehuang2 Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/tabei-l/diag_i2c_test.h,v $
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

#define    MAX_RETRY             1
#define    SFP_DEVICES           2
#define    DELAY_I2C_SCAN_RETRY  30

/* For I2C scan test, DIMM item is seperate form others device */
typedef enum {
    DIMM0_TEST = 0,
    DIMM1_TEST,
    BARO_TEST,
    DEFAULT_TEST,
    FORTNITE_TEST,
    SFP_TEST
} I2C_SCAN_TEST_OPTION;

extern int tabei_i2c_scan_test(int);
extern int build_i2c_scan_menu(boolean);

#endif                          /* __DIAG_I2C_TEST_H__ */

/*------------------------------------------------------------------
$Log: diag_i2c_test.h,v $
Revision 1.2  2019/10/17 02:16:22  kehuang2
Collapse Tabei-L into main trunk

Revision 1.1.4.8  2019/07/15 11:28:47  kehuang2
Support Barometer test and utility

Revision 1.1.4.7  2019/07/09 06:11:32  kehuang2
Update I2C bus change and enhence I2C scan coverage

Revision 1.1.4.6  2019/05/29 12:01:20  kehuang2
Rename function by the platform name

Revision 1.1.4.5  2019/05/16 08:48:13  kehuang2
Clean up code by the comment of code review.

Revision 1.1.4.4  2019/03/19 09:26:26  kehuang2
Merge Sku1 and Sku2 into same image

Revision 1.1.4.3  2018/10/24 10:45:17  harrchan
Seperate DIMM test from other I2C device

Revision 1.1.4.2  2018/10/02 01:49:59  harrchan
Initial commit for Tabei-L P1A bring up.

$Endlog$
*/
