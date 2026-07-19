 /* $Id: diag_usb_lib.h,v 1.2 2018/08/06 02:31:51 harrchan Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/viper/diag_usb_lib.h,v $
 *------------------------------------------------------------------
 * Filename: diag_usb_lib.h
 *
 * Description: Diag usb library header file.
 *
 * Copyright (c) 2011-2018 by cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#ifndef __DIAG_USB_LIB_H__
#define __DIAG_USB_LIB_H__

#include "linux_usb_test.h"

#define MAX_FILENAME_LENGTH     255
#define MAX_COMMAND_LENGTH      2048
#define USB_SLOT0       0
#define USB_SLOT1       1
#define USB_SLOT2       2
#define USB_SLOT3       3

extern struct usb_info_t usb[4];
extern int usb_get_info(void);
extern int usb_get_speed(int);
extern void usb_display(int);


#endif                          /* __DIAG_USB_TEST_H__ */


/******** History ********
$Log: diag_usb_lib.h,v $
Revision 1.2  2018/08/06 02:31:51  harrchan
Merge viper E2E to the main trunk (CSCvk28469)

Revision 1.1.2.2  2018/03/27 07:12:10  lucywang
Modified USB test for 2.0 and 3.0

Revision 1.1.2.1  2018/02/27 08:06:47  harrchan
Initial viper application code base



$Endlog$
*/
