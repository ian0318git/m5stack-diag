/* $Id: plug_lte_host.c,v 1.4 2018/11/23 09:15:07 hondwang Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/plug_lte/plug_lte_host.c,v $
 *------------------------------------------------------------------
 *
 * plug_lte_host.c - PLUGGABLE LTE Host Function
 *                   (Needs to be implemented by host side)
 *
 * Copyright (c) 2015 - 2018 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <fcntl.h>
#include "common.h"
#include "types.h"
#include "error.h"
#include "plug_lte_host.h"

#define PLUG_WARNING_MSG(func)         printf("'%s' is not implemented !!!\n", func);

int plug_lte_host_get_plug_usb_devinfo(int, char *, char *, char *);
int plug_lte_host_get_modem_drv_path(char *);


/*******************************************************************************
 * Function   : plug_lte_host_get_plug_usb_devinfo
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
int plug_lte_host_get_plug_usb_devinfo (int plug_slot, char *plug_usb2p0_ptr,
                                        char *plug_usb3p0_ptr,
                                        char *plug_usb_dport_ptr)
__attribute__((weak, alias("__plug_lte_host_get_plug_usb_devinfo")));
int __plug_lte_host_get_plug_usb_devinfo (int plug_slot,
                                          char *plug_usb2p0_ptr,
                                          char *plug_usb3p0_ptr,
                                          char *plug_usb_dport_ptr)
{
    PLUG_WARNING_MSG(__func__)
    return (FAILED);
}


/*******************************************************************************
 * Function   : plug_lte_host_get_modem_drv_path
 * Description: To get the path of LTE driver
 * Inputs     : drv_path - pointer to store the path of LTE modem driver
 *              (e.g. /diag, /firmware)
 * Outputs    : PASSED or FAILED
 *******************************************************************************
 */
int plug_lte_host_get_modem_drv_path (char *drv_path)
__attribute__((weak, alias("__plug_lte_host_get_modem_drv_path")));
int __plug_lte_host_get_modem_drv_path (char *drv_path)
{
    PLUG_WARNING_MSG(__func__)
    return (FAILED);
}


/*-------------------------------------------------
$Log: plug_lte_host.c,v $
Revision 1.4  2018/11/23 09:15:07  hondwang
Re-instruct pluggable common code with CDETs CSCvn17216

Revision 1.3.54.2  2018/10/15 07:43:26  shjung
Re-struct for pluggable-LTE common codes

Revision 1.3.54.1  2018/10/15 06:51:05  hondwang
pluggable common code re-instruct modify code

Revision 1.3  2018/02/09 09:15:45  hondwang
Merge Star branch star-branch-c9xx to main trunk

Revision 1.2.2.2  2018/01/20 06:56:33  hondwang
prepare merge star-branch-c9xx to main trunk

Revision 1.2  2018/01/20 05:01:08  hondwang
prepare merge star-branch-c9xx to main trunk

Revision 1.1.4.3  2017/08/16 08:31:58  tirawan
Re-enumerate USB modem by reconnecting modem when enabling USB 2.0 or USB 3.0

Revision 1.1.4.2  2017/08/08 07:42:13  hondwang
add pluggable LTE for star-branch-c9xx

Revision 1.1.2.1  2017/07/20 17:22:50  tirawan
Add USB 2.0 test and Debug port, and host implementation function prototype


*/

