/* $Id: diag_usb_lib.h,v 1.4 2019/07/11 12:31:30 alicehua Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/nutella/diag_usb_lib.h,v $
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

#define USB_DEVICE_MAX_NUM      (16)

#define MAX_FILENAME_LENGTH     255
#define MAX_COMMAND_LENGTH      2048

#define USB_20       0
#define USB_30       1

#define USB_HOST20_SPEED        (480)
#define USB_HOST30_SPEED        (5000)

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
extern int ExecuteCmdbyPopen(char *cmd, char *retBuf, int sizeOfBuf);
extern int usb_mass_stor_present_by_bus_lev(int, int, int *);
extern int get_usb_storage_info(int, struct usb_info_t *);
extern int find_usb_storage_index(int, int *);

#endif                          /* __DIAG_USB_TEST_H__ */


/******** History ********
$Log: diag_usb_lib.h,v $
Revision 1.4  2019/07/11 12:31:30  alicehua
Collapse Nutella codes into main trunk.

$Endlog$
*/
