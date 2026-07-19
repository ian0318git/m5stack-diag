/* $Id: usb_dongle_host_slot_modules.c,v 1.2 2019/06/14 09:59:29 steja Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/tsn/usb_dongle_host_slot_modules.c,v $
 *------------------------------------------------------------------
 *
 * usb_dongle_host_slot_modules.c 
 *
 * Copyright (c) 2015-2019 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */
#include <string.h>
#include "types.h"
#include "usb_dongle_host_slot_modules.h"
#include "usb_dongle_common_test.h"
#include "usb_dongle_lte_swi_test.h"


/* USB Dongle Module Lists */
struct udongle_module_info udongle_module_tb1[] = {
    {.name = "USB Dongle LTE WP7601",  .product = USB_DONGLE_LTE_WP7601,
     .vid  = SIERRA_WIRELESS_VID,      .pid = SIERRA_WIRELESS_LTE_PID,
     .diag = (PFT)usb_dongle_lte_swi_main, 
     .intf_diag = (PFT)usb_dongle_lte_swi_main},
    {.name = "USB Dongle LTE WP7603",  .product = USB_DONGLE_LTE_WP7603,
     .vid  = SIERRA_WIRELESS_VID,      .pid = SIERRA_WIRELESS_LTE_PID,
     .diag = (PFT)usb_dongle_lte_swi_main, 
     .intf_diag = (PFT)usb_dongle_lte_swi_main},
    {.name = "USB Dongle LTE WP7605",  .product = USB_DONGLE_LTE_WP7605,
     .vid  = SIERRA_WIRELESS_VID,      .pid = SIERRA_WIRELESS_LTE_PID,
     .diag = (PFT)usb_dongle_lte_swi_main, 
     .intf_diag = (PFT)usb_dongle_lte_swi_main},
    {.name = "USB Dongle LTE WP7607",  .product = USB_DONGLE_LTE_WP7607,
     .vid  = SIERRA_WIRELESS_VID,      .pid = SIERRA_WIRELESS_LTE_PID,
     .diag = (PFT)usb_dongle_lte_swi_main, 
     .intf_diag = (PFT)usb_dongle_lte_swi_main},
    {.name = "USB Dongle LTE WP7608",  .product = USB_DONGLE_LTE_WP7608,
     .vid  = SIERRA_WIRELESS_VID,      .pid = SIERRA_WIRELESS_LTE_PID,
     .diag = (PFT)usb_dongle_lte_swi_main, 
     .intf_diag = (PFT)usb_dongle_lte_swi_main},
    {.name = "USB Dongle LTE WP7609",  .product = USB_DONGLE_LTE_WP7609,
     .vid  = SIERRA_WIRELESS_VID,      .pid = SIERRA_WIRELESS_LTE_PID,
     .diag = (PFT)usb_dongle_lte_swi_main, 
     .intf_diag = (PFT)usb_dongle_lte_swi_main},
    {.name = "USB Dongle LTE WP7610",  .product = USB_DONGLE_LTE_WP7610,
     .vid  = SIERRA_WIRELESS_VID,      .pid = SIERRA_WIRELESS_LTE_PID,
     .diag = (PFT)usb_dongle_lte_swi_main, 
     .intf_diag = (PFT)usb_dongle_lte_swi_main},
};

int MAX_DONGLE_IDS = (sizeof(udongle_module_tb1) /
                   sizeof(struct udongle_module_info));


/*-------------------------------------------------
$Log: usb_dongle_host_slot_modules.c,v $
Revision 1.2  2019/06/14 09:59:29  steja
Supported Cooper usb dongle LTE



*/
