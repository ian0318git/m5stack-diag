 /* $Id: diag_usb_util.h,v 1.2 2019/12/11 10:10:32 lucywang Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/nanook/diag_usb_util.h,v $
 *------------------------------------------------------------------
 * Filename: diag_usb_util.h
 *
 * Description: Diag usb utility header file.
 *
 * Copyright (c) 2019 by cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#ifndef __DIAG_USB_UTIL_H__
#define __DIAG_USB_UTIL_H__

#define USB_SLOT0                       0
#define USB_SLOT1                       1
#define USB_SLOT2                       2

extern int nanook_usb_utils(int);
extern int usb_test_mode (int option);

#endif                          /* __DIAG_USB_UTIL_H__ */


/******** History ********
$Log: diag_usb_util.h,v $
Revision 1.2  2019/12/11 10:10:32  lucywang
Merged Nanook to main trunk


$Endlog$
*/
