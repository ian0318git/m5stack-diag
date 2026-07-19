/* $Id: diag_usb_test.h,v 1.2 2019/12/11 10:10:31 lucywang Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/nanook/diag_usb_test.h,v $
 *------------------------------------------------------------------
 * Filename: diag_usb_test.h
 *
 * Description: Diag usb test header file.
 *
 * Copyright (c) 2019 by cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#ifndef __DIAG_USB_TEST_H__
#define __DIAG_USB_TEST_H__

typedef enum {
    BACK_USB_EXT_PORT_20 = 0,
    BACK_USB_EXT_PORT_30,
    FRONT_USB_EXT_PORT_20
} USB_HOST_PORT_NO;

extern int diag_ext_usb_test(int);

#endif                          /* __DIAG_USB_TEST_H__ */


/******** History ********
$Log: diag_usb_test.h,v $
Revision 1.2  2019/12/11 10:10:31  lucywang
Merged Nanook to main trunk


$Endlog$
*/
