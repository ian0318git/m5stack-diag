/* $Id: usb_dongle_common_test.h,v 1.2 2019/06/14 09:59:33 steja Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/usb_dongle/usb_dongle_common/usb_dongle_common_test.h,v $
 *------------------------------------------------------------------
 *
 * usb_dongle_common_test.h - Header for USB dongle Common Test functions
 *
 *
 * Copyright (c) 2015-2019 by cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */
#ifndef __USB_DONGLE_COMMON_TEST_H__
#define __USB_DONGLE_COMMON_TEST_H__

#define USB_MOD_STR                      "USB_DONGLE"

#define FIRST_PORT                       (1)
#define MAX_USB_PORT_NUMBER              (2)
#define USB_DONGLE_MAX_ENUMERATE_TIME    (15.5)
#define DELAY_1_SEC                      (1)

/* USB Dongle Vendor ID */
#define SIERRA_WIRELESS_VID              (0x1199)

/* USB Dongle Product ID */
#define SIERRA_WIRELESS_LTE_PID          (0x68c0)

/* USB Dongle Product*/
#define USB_DONGLE_LTE_WP7601            "Sierra Wireless WP7601"
#define USB_DONGLE_LTE_WP7603            "Sierra Wireless WP7603"
#define USB_DONGLE_LTE_WP7605            "Sierra Wireless WP7605"
#define USB_DONGLE_LTE_WP7607            "Sierra Wireless WP7607"
#define USB_DONGLE_LTE_WP7608            "Sierra Wireless WP7608"
#define USB_DONGLE_LTE_WP7609            "Sierra Wireless WP7609"
#define USB_DONGLE_LTE_WP7610            "Sierra Wireless WP7610"



typedef enum {
    USB_DONGLE_PORT_1 = 1,
    USB_DONGLE_PORT_2
} usb_dongle_port_no;

extern int usb_dongle_test_entry(int);
extern int usb_dongle_intf_test(char *, int);

#endif       /* __USB_DONGLE_COMMON_TEST_H__ */
/*-------------------------------------------------
$Log: usb_dongle_common_test.h,v $
Revision 1.2  2019/06/14 09:59:33  steja
Supported Cooper usb dongle LTE



*/
