/* $Id: diag_usb_util.h,v 1.4 2019/07/11 12:31:30 alicehua Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/nutella/diag_usb_util.h,v $
 *------------------------------------------------------------------
 * Filename: diag_usb_util.h
 *
 * Description: Diag usb utility header file.
 *
 * Copyright (c) 2011-2019 by cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#ifndef __DIAG_USB_UTIL_H__
#define __DIAG_USB_UTIL_H__

#define TEST_J_STATE  0x10000000
#define TEST_K_STATE  0x20000000
#define TEST_SE0_NAK  0x30000000
#define TEST_PACKET   0x40000000

extern int usb_test_mode (int option);

#endif                          /* __DIAG_USB_UTIL_H__ */


/******** History ********
$Log: diag_usb_util.h,v $
Revision 1.4  2019/07/11 12:31:30  alicehua
Collapse Nutella codes into main trunk.

$Endlog$
*/
