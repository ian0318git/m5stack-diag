/* $Id: usb_dongle_common_host.c,v 1.2 2019/06/14 09:59:33 steja Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/usb_dongle/usb_dongle_common/usb_dongle_common_host.c,v $
 *------------------------------------------------------------------
 *
 * usb_dongle_common_host.c - USB dongles common Host Function.
 *                   (Needs to be implemented by host side)
 *
 * Copyright (c) 2015 - 2019 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include "common.h"
#include "types.h"
#include "error.h"
#include "usb_dongle_common_host.h"

#define UDONGLE_WARNING_MSG(func)         printf("'%s' is not implemented !!!\n", func);


int usb_dongle_host_get_max_usb_slot_no(int *);
int usb_dongle_host_get_usb_bus_no(int, int *, int *);
int usb_dongle_host_get_usb_lev_no(int, int *);
int usb_dongle_host_get_usb_devinfo(int, char *, char *);
int usb_storage_rd_wr_tests(int);

/*------------------------------------------------------------------------------
 * Function   : usb_dongle_host_get_max_usb_slot_no
 * Description: Function to get the maximum of USB slot number
 * Inputs     : max_slot_no - the maximum of USB slot number
 * Outputs    : PASSED/FAILED
 *------------------------------------------------------------------------------
 */
int usb_dongle_host_get_max_usb_slot_no (int *max_slot_no)
__attribute__((weak, alias("__usb_dongle_host_get_max_usb_slot_no")));
int __usb_dongle_host_get_max_usb_slot_no (int *max_slot_no)
{
    UDONGLE_WARNING_MSG(__func__);
    return (FAILED);
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
__attribute__((weak, alias("__usb_dongle_host_get_usb_bus_no")));
int __usb_dongle_host_get_usb_bus_no (int usb_slot, int *usb_2p0_bus_no,
                                      int *usb_3p0_bus_no)
{
    UDONGLE_WARNING_MSG(__func__);
    return (FAILED);
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
__attribute__((weak, alias("__usb_dongle_host_get_usb_lev_no")));
int __usb_dongle_host_get_usb_lev_no (int usb_slot, int *usb_lev_no)
{
    UDONGLE_WARNING_MSG(__func__);
    return (FAILED);
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
__attribute__((weak, alias("__usb_dongle_host_get_usb_devinfo")));
int __usb_dongle_host_get_usb_devinfo (int usb_slot, char *udongle_usb2p0_ptr, 
                                       char *udongle_usb3p0_ptr)
{
    UDONGLE_WARNING_MSG(__func__);
    return (FAILED);
}


/*------------------------------------------------------------------------------
 * Function   : usb_storage_rd_wr_tests
 * Description: Function to test USB interface which connects an external USB
 *              storage device
 * Inputs     : usb_slot - USB slot number
 * Outputs    : FAILED
 *------------------------------------------------------------------------------
 */
int usb_storage_rd_wr_tests(int slot)
__attribute__((weak, alias("__usb_storage_rd_wr_tests")));
int __usb_storage_rd_wr_tests(int slot)
{
    UDONGLE_WARNING_MSG(__func__);
    return (FAILED);
}
    
    
/*-------------------------------------------------
$Log: usb_dongle_common_host.c,v $
Revision 1.2  2019/06/14 09:59:33  steja
Supported Cooper usb dongle LTE



*/
