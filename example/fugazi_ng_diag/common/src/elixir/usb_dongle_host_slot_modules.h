/* $Id: usb_dongle_host_slot_modules.h,v 1.2 2021/09/24 01:21:08 harrchan Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/elixir/usb_dongle_host_slot_modules.h,v $
 *------------------------------------------------------------------
 *
 * usb_dongle_host_slot_modules.h
 *
 * Copyright (c) 2015 - 2019 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#ifndef __USB_DONGLE_COMMON_HOST_GET_USB_DEVINFO_H__
#define __USB_DONGLE_COMMON_HOST_GET_USB_DEVINFO_H__

struct udongle_module_info {
    char   *name;         /* module name */
    char   *product;      /* product name */
    uint16_t   vid;
    uint16_t   pid;
    PFT    diag;          /* the diagnosric for this module */
    PFT    intf_diag;     /* the pci test for this module */
    uint   mod_info_flags;/* these flags provide more info of the module*/
};

extern struct udongle_module_info udongle_module_tb1[];
extern int MAX_DONGLE_IDS;

#endif

/*-------------------------------------------------
$Log: usb_dongle_host_slot_modules.h,v $
Revision 1.2  2021/09/24 01:21:08  harrchan
Collapse Elixir-branch to Main Trunk.

Revision 1.1.2.1  2020/11/27 03:39:10  illiu
Add usb dongle feature into test item(External USB test)

Revision 1.2  2019/06/14 09:59:29  steja
Supported Cooper usb dongle LTE



*/
