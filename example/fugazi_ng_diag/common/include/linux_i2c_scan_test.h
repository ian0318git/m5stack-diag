/* $Id: linux_i2c_scan_test.h,v 1.2 2019/07/11 12:34:40 alicehua Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/include/linux_i2c_scan_test.h,v $
 *------------------------------------------------------------------
 *
 * linux_i2c_scan_test.h - Header file for Linux I2C scan Test
 *
 * May 2019
 *
 * Copyright (c) 2015-2019 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */
 
#ifndef __LINUX_I2C_SCAN_TEST_H__
#define __LINUX_I2C_SCAN_TEST_H__

#define    MAX_RETRY             1
#define    DELAY_I2C_SCAN_RETRY  30
extern int linux_i2c_scan_test(int, n2g_i2c_if_t *, int);

#endif /* __LINUX_BLOCK_TEST_H__ */

/*---------------------------------------------------------------
$Log: linux_i2c_scan_test.h,v $
Revision 1.2  2019/07/11 12:34:40  alicehua
Collapse Nutella codes into main trunk

$Endlog$
*/
