/* $Id: diag_usb_test.h,v 1.2 2019/10/17 02:16:23 kehuang2 Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/tabei-l/diag_usb_test.h,v $
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
Revision 1.2  2019/10/17 02:16:23  kehuang2
Collapse Tabei-L into main trunk

Revision 1.1.4.4  2019/01/18 02:30:16  olin2
Clean up code

Revision 1.1.4.3  2018/10/03 06:51:01  kodko
Initial bring up for P1A Tabei-L USB 2.0/3.0 read/write test.

Revision 1.1.4.2  2018/10/02 01:50:01  harrchan
Initial commit for Tabei-L P1A bring up.

$Endlog$
*/
