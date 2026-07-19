 /* $Id: diag_storage_lib.h,v 1.2 2018/08/06 02:31:51 harrchan Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/viper/diag_storage_lib.h,v $
 *------------------------------------------------------------------
 * Filename: diag_storage_lib.h
 *
 * Description: Diag storage library header file.
 *
 * Copyright (c) 2011-2018 by cisco Systems, Inc.
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
#define BOOTFLASH_TEST_LEN       0x10000
#define SECTOR_OFFSET            0x586000

#endif                          /* __DIAG_STORGE_LIB_H__ */


/******** History ********
$Log: diag_storage_lib.h,v $
Revision 1.2  2018/08/06 02:31:51  harrchan
Merge viper E2E to the main trunk (CSCvk28469)

Revision 1.1.2.4  2018/07/03 05:38:55  harrchan
Follow the coding rule to clean up code

Revision 1.1.2.3  2018/05/08 11:24:14  lucywang
Modified Boot flash test address base on Cisco BIOS team suggestion

Revision 1.1.2.2  2018/03/26 09:21:22  harrchan
Support usb debug port detection test

Revision 1.1.2.1  2018/02/27 08:06:46  harrchan
Initial viper application code base



$Endlog$
*/
