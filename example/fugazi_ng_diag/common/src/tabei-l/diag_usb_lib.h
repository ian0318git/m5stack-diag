 /* $Id: diag_usb_lib.h,v 1.2 2019/10/17 02:16:23 kehuang2 Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/tabei-l/diag_usb_lib.h,v $
 *------------------------------------------------------------------
 * Filename: diag_usb_lib.h
 *
 * Description: Diag usb library header file.
 *
 * Copyright (c) 2011-2019 by cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#ifndef __DIAG_USB_LIB_H__
#define __DIAG_USB_LIB_H__

#include "linux_usb_test.h"

#define USB_DEVICE_MAX_NUM          (16)

#define MAX_FILENAME_LENGTH         255
#define MAX_COMMAND_LENGTH          2048

#define USB_HOST20_SPEED            (480)
#define USB_HOST30_SPEED            (5000)

#define USB_DEVICE_TYPE_STORAGE     "Driver=usb-storage"
#define USB_DEVICE_TYPE_HUB         "Driver=hub"

typedef enum {
    FRONT_USB = 0,
    PIM_USB,
} USB_IO_TYPE;

typedef enum {
    USB_BUS_0 = 0,
    USB_BUS_1,
    USB_BUS_2,
    USB_BUS_3,
    USB_BUS_END
} USB_HOST_BUS_NO;

typedef enum {
    USB_PORT_0 = 0,
    USB_PORT_1,
    USB_PORT_2,
    USB_PORT_3,
    USB_PORT_END
} USB_HOST_PORT_NUMBER;

typedef enum {
    USB_DEV_0 = 0,
    USB_DEV_1,
    USB_DEV_2,
    USB_DEV_3,
    USB_DEV_END
} USB_HOST_DEV_NO;

typedef enum {
    USB_LEV_0 = 0,
    USB_LEV_1,
    USB_LEV_2,
    USB_LEV_3,
    USB_LEV_END
} USB_HOST_LEV_NO;

extern int usb_parse_info(void);
extern int usb_get_speed(int);
extern void usb_display(int);
extern int ExecuteCmdbyPopen(char *cmd, char *retBuf, int sizeOfBuf);
extern int usb_mass_stor_present_by_bus_lev(int, int, int *);
extern int get_usb_storage_info(int, int, struct usb_info_t *);
extern int find_front_usb_storage_index(int, int *);

#endif                          /* __DIAG_USB_TEST_H__ */


/******** History ********
$Log: diag_usb_lib.h,v $
Revision 1.2  2019/10/17 02:16:23  kehuang2
Collapse Tabei-L into main trunk

Revision 1.1.4.6  2019/06/20 03:41:29  kodko
Fix pim usb3.0 flash can not be found issue when front usb is plugged with usb2.0 flash only by adding check usb device type.

Revision 1.1.4.5  2019/02/12 12:57:42  kodko
Support standalone USB3.0 and USB2.0 USB test and special USB hub test.

Revision 1.1.4.4  2018/11/16 13:42:30  kodko
Support front USB hub and PIM USB hub connect with USB3.0 and USB2.0 storage read/write test.

Revision 1.1.4.3  2018/10/03 06:51:01  kodko
Initial bring up for P1A Tabei-L USB 2.0/3.0 read/write test.

Revision 1.1.4.2  2018/10/02 01:50:01  harrchan
Initial commit for Tabei-L P1A bring up.

$Endlog$
*/
