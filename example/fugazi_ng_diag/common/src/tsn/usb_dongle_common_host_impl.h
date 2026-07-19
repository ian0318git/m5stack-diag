/* $Id: usb_dongle_common_host_impl.h,v 1.2 2019/06/14 09:59:29 steja Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/tsn/usb_dongle_common_host_impl.h,v $
 *------------------------------------------------------------------
 *
 * usb_dongle_common_host_impl.h - Header file for USB dongles common Host 
 *                                 Implementation.
 *
 * Copyright (c) 2015 - 2019 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#ifndef __USB_DONGLE_COMMON_HOST_IMPL_H__
#define __USB_DONGLE_COMMON_HOST_IMPL_H__

extern int usb_dongle_host_get_max_usb_slot_no(int *);
extern int usb_dongle_host_get_usb_bus_no(int, int *, int *);
extern int usb_dongle_host_get_usb_lev_no(int, int *);
extern int usb_dongle_host_get_usb_devinfo(int, char *, char *);

#define SLOT_0_USB2P0_DEVINFO   "1-1"
#define SLOT_0_USB3P0_DEVINFO   "2-1"
#define SLOT_1_USB2P0_DEVINFO   "3-1"
#define SLOT_1_USB3P0_DEVINFO   "4-1"
#define USB_MAX_SLOT_NO         (3)


typedef enum {
    USB_BUS_0 = 0,
    USB_BUS_1,
    USB_BUS_2,
    USB_BUS_3,
    USB_BUS_4,
    USB_BUS_END
} USB_HOST_BUS_NO;

typedef enum {
    USB_LEV_0 = 0,
    USB_LEV_1,
    USB_LEV_2,
    USB_LEV_3,
    USB_LEV_END
} USB_HOST_LEV_NO;

typedef enum {
    USB_EXT_PORT_0 = 0,
    USB_EXT_PORT_1,
    USB_LAST_PORT
} USB_HOST_PORT_NO;

typedef struct usb_bus_to_slot_t {
    int usb_2p0_bus;
    int usb_3p0_bus;
} usb_slot_to_bus_t;

typedef struct usb_bus_to_devinfo_t {
    char usb_2p0_devinfo[32];
    char usb_3p0_devinfo[32];
} usb_slot_to_devinfo_t;


#endif       /* __USB_DONGLE_COMMON_HOST_IMPL_H__ */

/*-------------------------------------------------
$Log: usb_dongle_common_host_impl.h,v $
Revision 1.2  2019/06/14 09:59:29  steja
Supported Cooper usb dongle LTE



*/

