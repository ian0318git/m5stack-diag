/* $Id: diag_usb_test.h,v 1.2 2021/04/15 00:52:26 achiu2 Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/phoenix/diag_usb_test.h,v $
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


