/* $Id: usb_dongle_common_host_impl.c,v 1.2 2019/06/14 09:59:29 steja Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/tsn/usb_dongle_common_host_impl.c,v $
 *------------------------------------------------------------------
 *
 * usb_dongle_common_host_impl.c - USB Dongle Common Host Function 
 *                                 Implementation
 *
 *
 * Copyright (c) 2015-2019 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "common.h"
#include "usb_dongle_common_host_impl.h"


int usb_dongle_host_get_max_usb_slot_no(int *);
int usb_dongle_host_get_usb_bus_no(int, int *, int *);
int usb_dongle_host_get_usb_lev_no(int, int *);
int usb_dongle_host_get_usb_devinfo(int, char *, char *);


/* Mapping between USB Slot and bus number */
static usb_slot_to_bus_t usb_slot_to_bus[USB_LAST_PORT] = {
    {USB_BUS_1, USB_BUS_2}, /* USB Slot 0 */
};

static int usb_slot_to_lev[USB_LAST_PORT] = {
    USB_LEV_1, /* USB Slot 0 */
};

static usb_slot_to_devinfo_t usb_slot_to_devinfo[USB_LAST_PORT] = {
    {SLOT_0_USB2P0_DEVINFO, SLOT_0_USB3P0_DEVINFO}, /* USB Slot 0 */
};


/*------------------------------------------------------------------------------
 * Function   : usb_dongle_host_get_max_usb_slot_no
 * Description: Function to get the maximum of USB slot number
 * Inputs     : max_slot_no - the maximum of USB slot number
 * Outputs    : PASSED/FAILED
 *------------------------------------------------------------------------------
 */
int usb_dongle_host_get_max_usb_slot_no (int *max_slot_no)
{
    *max_slot_no = USB_MAX_SLOT_NO;

    return (PASSED);
}


/*------------------------------------------------------------------------------
 * Function   : usb_dongle_host_get_usb_bus_no
 * Description: Function to get the USB bus number based on USB slot
 * Inputs     : usb_slot - USB slot number
 *              usb_2p0_bus_no - pointer to store USB 2.0 bus number
 *              usb_3p0_bus_no - pointer to store USB 3.0 bus number
 * Outputs    : PASSED/FAILED
 *------------------------------------------------------------------------------
 */
int usb_dongle_host_get_usb_bus_no (int usb_slot, int *usb_2p0_bus_no,
                                    int *usb_3p0_bus_no)
{
    *usb_2p0_bus_no = usb_slot_to_bus[usb_slot].usb_2p0_bus;
    *usb_3p0_bus_no = usb_slot_to_bus[usb_slot].usb_3p0_bus;

    return (PASSED);
}


/*------------------------------------------------------------------------------
 * Function   : usb_dongle_host_get_usb_lev_no
 * Description: Function to get the USB level number based on USB slot
 * Inputs     : usb_slot - USB slot number
 *              usb_lev_no - pointer to store USB level number
 * Outputs    : PASSED/FAILED
 *------------------------------------------------------------------------------
 */
int usb_dongle_host_get_usb_lev_no (int usb_slot, int *usb_lev_no)
{
    *usb_lev_no = usb_slot_to_lev[usb_slot];

    return (PASSED);
}


/*------------------------------------------------------------------------------
 * Function   : usb_dongle_host_get_usb_devinfo
 * Description: Function to get the USB2.0/3.0 device info based on USB slot 
 *              (USB device info: e.g. 1-1, 2-1, 3-1, etc.)
 * Inputs     : usb_slot - USB slot number
 *              udongle_usb2p0_ptr - pointer to the corresponding USB2.0 device
 *                                   info
 *              udongle_usb3p0_ptr - pointer to the corresponding USB3.0 device
 *                                   info
 * Outputs    : PASSED/FAILED
 *------------------------------------------------------------------------------
 */
int usb_dongle_host_get_usb_devinfo (int usb_slot, char *udongle_usb2p0_ptr,
                                     char *udongle_usb3p0_ptr)
{
    sprintf(udongle_usb2p0_ptr, "%s", 
            usb_slot_to_devinfo[usb_slot].usb_2p0_devinfo);
    sprintf(udongle_usb3p0_ptr, "%s",
            usb_slot_to_devinfo[usb_slot].usb_3p0_devinfo);

    return (PASSED);
}


/*-------------------------------------------------
$Log: usb_dongle_common_host_impl.c,v $
Revision 1.2  2019/06/14 09:59:29  steja
Supported Cooper usb dongle LTE



*/
