/* $Id: diag_usb_test.h,v 1.4 2019/07/11 12:31:30 alicehua Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/nutella/diag_usb_test.h,v $
 *------------------------------------------------------------------
 * Filename: diag_usb_test.h
 *
 * Description: Diag usb test header file.
 *
 * Copyright (c) 2011-2019 by cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#ifndef __DIAG_USB_TEST_H__
#define __DIAG_USB_TEST_H__

typedef enum {
    USB_EXT_PORT_0 = 0,
    USB_EXT_PORT_1,
    USB_LAST_PORT
} USB_HOST_PORT_NO;

extern int diag_ext_usb_test(int);

#endif                          /* __DIAG_USB_TEST_H__ */


/******** History ********
$Log: diag_usb_test.h,v $
Revision 1.4  2019/07/11 12:31:30  alicehua
Collapse Nutella codes into main trunk.

$Endlog$
*/
