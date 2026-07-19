 /* $Id: diag_storage_lib.h,v 1.3 2020/08/06 07:54:55 kehuang2 Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/tabei-l/diag_storage_lib.h,v $
 *------------------------------------------------------------------
 * Filename: diag_storage_lib.h
 *
 * Description: Diag storage library header file.
 *
 * Copyright (c) 2011-2020 by cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#ifndef __DIAG_STORGE_LIB_H__
#define __DIAG_STORGE_LIB_H__

extern int access_device_test(char *);
extern int access_device_random_offset_test(char *, int);

#define DEV_OPEN_RETRY 10

/* For USB test */
#define USB_OFFSET      0
#define USB20_SPEED     480
#define USB30_SPEED     5000
#define USB2                       2 
#define USB3                       3

/* For bootflash test */
#define BOOTFLASH_TEST_LEN       0x10000
#define TABEI_SECTOR_OFFSET      0x596000   /* For Tabei-L */
#define PMTM_SECTOR_OFFSET       0x7E0000   /* For Promethium */

#endif                          /* __DIAG_STORGE_LIB_H__ */


/******** History ********
$Log: diag_storage_lib.h,v $
Revision 1.3  2020/08/06 07:54:55  kehuang2
Collapse Promethium into main trunk

Revision 1.2  2019/10/17 02:16:23  kehuang2
Collapse Tabei-L into main trunk

Revision 1.1.4.5  2019/06/06 00:59:29  kehuang2
Update offset address for bootflash test based on BIOS V0.9

Revision 1.1.4.4  2019/05/08 03:09:21  kodko
Support USB device random offset read/write test.

Revision 1.1.4.3  2019/01/18 02:30:15  olin2
Clean up code

Revision 1.1.4.2  2018/10/02 01:50:00  harrchan
Initial commit for Tabei-L P1A bring up.

$Endlog$
*/
