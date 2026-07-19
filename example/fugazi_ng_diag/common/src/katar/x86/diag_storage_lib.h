/* $Id: diag_storage_lib.h,v 1.2 2019/06/14 05:24:48 mikech2 Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/katar/x86/diag_storage_lib.h,v $
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

/* For USB test */
#define USB_OFFSET      0
#define USB20_SPEED     480
#define USB30_SPEED     5000
#define USB2                       2 
#define USB3                       3

/* For bootflash test */
#define BOOTFLASH_TEST_LEN       0x10000
#define BOOTFLASH_SWITCH_TEST_LEN  4
#define SECTOR_OFFSET            0x800000

#endif                          /* __DIAG_STORGE_LIB_H__ */


/******** History ********
$Log: diag_storage_lib.h,v $
Revision 1.2  2019/06/14 05:24:48  mikech2
Collapse katar-branch00 to Main Trunk

Revision 1.1.2.3  2019/04/23 07:10:20  peteteng
Add bootflash switch test

Revision 1.1.2.2  2019/03/27 03:21:03  peteteng
Fix bootflash test issue

Revision 1.1.2.1  2018/10/22 08:02:21  mikech2
Move project folder to common/src/katar/x86

Revision 1.1.2.1  2018/06/11 07:05:54  peteteng
add bootflash test from viper

Revision 1.1.2.3  2018/05/08 11:24:14  lucywang
Modified Boot flash test address base on Cisco BIOS team suggestion

Revision 1.1.2.2  2018/03/26 09:21:22  harrchan
Support usb debug port detection test

Revision 1.1.2.1  2018/02/27 08:06:46  harrchan
Initial viper application code base



$Endlog$
*/
