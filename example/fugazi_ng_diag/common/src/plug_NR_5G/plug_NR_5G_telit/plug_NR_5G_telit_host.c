/* $Id: plug_NR_5G_telit_host.c,v 1.2 2021/06/02 02:56:19 alpeng Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/plug_NR_5G/plug_NR_5G_telit/plug_NR_5G_telit_host.c,v $
 *------------------------------------------------------------------
 *
 * plug_NR_5G_telit_host.c - Pluggable Telit Host Functions
 *                         (Needs to be implemented by host side)
 *
 * Copyright (c) 2020 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#include <stdlib.h>
#include <stdio.h>
#include "common.h"
#include "plug_lte_host_impl.h"

#define PLUG_WARNING_MSG(func)         printf("'%s' is not implemented !!!\n", func);

int plug_lte_telit_host_get_plug_usb_devinfo(int, char *, char *, char *);
int plug_lte_telit_host_get_modem_drv_path(char *);


/*******************************************************************************
 * Function   : plug_lte_telit_host_get_plug_usb_devinfo
 * Description: Function to get the corresponding USB device info for 
 *              pluggable module
 *              USB device info:      e.g. "3-1.1", "4-1.1", "4-1.2"
 *              USB device numbering: bus-port.port.port:configuration.interface
 *                                    i.e. "3-1.1:1.8" indicats a USB device connected 
 *                                    on bus 3, port 1, port 1, configuration 1, 
 *                                    and interface 2.
 * Inputs     : plug_slot - which pluggable slot
 *              plug_usb2p0_ptr - pointer to the corresponding USB2.0 device info
 *              plug_usb3p0_ptr - pointer to the corresponding USB3.0 device info
 * Outputs    : PASSED or FAILED
 *******************************************************************************
 */
int plug_lte_telit_host_get_plug_usb_devinfo (int plug_slot,
                                              char *plug_usb2p0_ptr,
                                              char *plug_usb3p0_ptr,
                                              char *plug_usb_dport_ptr)
__attribute__((weak, alias("__plug_NR_5g_telit_host_get_plug_usb_devinfo")));
int __plug_NR_5g_telit_host_get_plug_usb_devinfo (int plug_slot,
                                                char *plug_usb2p0_ptr,
                                                char *plug_usb3p0_ptr,
                                                char *plug_usb_dport_ptr)
{
    PLUG_WARNING_MSG(__func__)
    return (FAILED);
}


/*******************************************************************************
 * Function   : plug_lte_telit_host_get_modem_drv_path
 * Description: To get the path of LTE driver
 * Inputs     : drv_path - pointer to store the path of LTE modem driver
 *              (e.g. /diag, /firmware)
 * Outputs    : PASSED or FAILED
 *******************************************************************************
 */
int plug_lte_telit_host_get_modem_drv_path (char *drv_path)
__attribute__((weak, alias("__plug_NR_5g_telit_host_get_modem_drv_path")));
int __plug_NR_5g_telit_host_get_modem_drv_path (char *drv_path)
{
    PLUG_WARNING_MSG(__func__)
    return (FAILED);
}
/*********************************************************************
 * $Log: plug_NR_5G_telit_host.c,v $
 * Revision 1.2  2021/06/02 02:56:19  alpeng
 * merge sears into trunk
 *
 * Revision 1.1.2.2  2020/12/02 03:57:22  tshanmug
 * Sears Antenna test updated
 *
 *
 * $Endlog$
 */
