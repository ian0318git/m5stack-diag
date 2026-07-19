/* $Id: usb_dongle_common_host.h,v 1.2 2019/06/14 09:59:33 steja Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/usb_dongle/usb_dongle_common/usb_dongle_common_host.h,v $
 *------------------------------------------------------------------
 *
 * usb_dongle_common_host.h - Header file for USB dongles common Host.
 *
 * Copyright (c) 2015 - 2019 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#ifndef __USB_DONGLE_COMMON_HOST_H__
#define __USB_DONGLE_COMMON_HOST_H__

extern int usb_dongle_host_get_max_usb_slot_no(int *);
extern int usb_dongle_host_get_usb_bus_no(int, int *, int *);
extern int usb_dongle_host_get_usb_lev_no(int, int *);
extern int usb_dongle_host_get_usb_devinfo(int, char *, char *);
extern int usb_storage_rd_wr_tests(int);


#endif       /* __USB_DONGLE_COMMON_HOST_H__ */

/*-------------------------------------------------
$Log: usb_dongle_common_host.h,v $
Revision 1.2  2019/06/14 09:59:33  steja
Supported Cooper usb dongle LTE



*/
