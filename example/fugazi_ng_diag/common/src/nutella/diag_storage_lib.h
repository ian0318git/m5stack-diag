/* $Id: diag_storage_lib.h,v 1.4 2019/07/11 12:31:29 alicehua Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/nutella/diag_storage_lib.h,v $
 *------------------------------------------------------------------
 * Filename: diag_storage_lib.h
 *
 * Description: Diag storage library header file.
 *
 * Copyright (c) 2011-2019 by cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#ifndef __DIAG_STORGE_LIB_H__
#define __DIAG_STORGE_LIB_H__

extern int access_device_test(char *);

#define DEV_OPEN_RETRY 10

/* For USB test */
#define USB_OFFSET      0
#define USB20_SPEED     480
#define USB30_SPEED     5000
#define USB2                       2 
#define USB3                       3

/* For bootflash test */
#define EMMC_TEST_LEN            0x200
#define BOOTFLASH_TEST_LEN       0xFFF8
#define SECTOR_OFFSET            0x586000

#endif                          /* __DIAG_STORGE_LIB_H__ */


/******** History ********
$Log: diag_storage_lib.h,v $
Revision 1.4  2019/07/11 12:31:29  alicehua
Collapse Nutella codes into main trunk.

$Endlog$
*/
