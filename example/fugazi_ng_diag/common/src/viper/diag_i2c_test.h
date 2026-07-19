 /* $Id: diag_i2c_test.h,v 1.2 2018/08/06 02:31:50 harrchan Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/viper/diag_i2c_test.h,v $
 *------------------------------------------------------------------
 * Filename: diag_i2c_test.h
 *
 * Description: Platform specific I2C header file.
 *
 * Copyright (c) 2013-2018 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */
#ifndef __DIAG_I2C_TEST_H__
#define __DIAG_I2C_TEST_H__

#define    MAX_RETRY             1
#define    DELAY_I2C_SCAN_RETRY  30
extern int viper_i2c_scan_test(int);


#endif                          /* __DIAG_I2C_TEST_H__ */

/*------------------------------------------------------------------
$Log: diag_i2c_test.h,v $
Revision 1.2  2018/08/06 02:31:50  harrchan
Merge viper E2E to the main trunk (CSCvk28469)

Revision 1.1.2.2  2018/07/03 05:38:55  harrchan
Follow the coding rule to clean up code

Revision 1.1.2.1  2018/02/27 08:06:44  harrchan
Initial viper application code base


$Endlog$
*/
