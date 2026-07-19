/* $Id: plug_lte_host_impl.c,v 1.2 2019/10/17 02:16:27 kehuang2 Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/tabei-l/plug_lte_host_impl.c,v $
 *------------------------------------------------------------------
 *
 * plug_lte_host_impl.c - PLUGGABLE LTE Host Function Implementation
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
#include <unistd.h>
#include <fcntl.h>
#include "common.h"
#include "types.h"
#include "nvmonvars.h"
#include "error.h"
#include "proto.h"
#include "plug_lte_host_impl.h"
#include "plug_host_fpga_lib.h"

int plug_lte_host_get_plug_usb_devinfo(int, char *, char *, char *);
int plug_lte_telit_host_get_plug_usb_devinfo(int, char *, char *, char *);
int plug_lte_host_get_modem_drv_path(char *);
int plug_lte_telit_host_get_modem_drv_path(char *);


/* ----------------------------------------------------------------------------
 *  Function: plug_lte_host_get_plug_usb_devinfo
 *  Description: Function to get the corresponding USB device info for 
 *               pluggable module
 *  Inputs: plug_slot - which pluggable slot
 *          plug_usb2p0_ptr - pointer to the corresponding USB2.0 device info
 *          plug_usb3p0_ptr - pointer to the corresponding USB3.0 device info
 *          plug_dport_ptr  - pointer to the corresponding debug port device 
 *                            info
 *  Output: PASSED
 * ----------------------------------------------------------------------------
 */
int plug_lte_host_get_plug_usb_devinfo (int plug_slot, char *plug_usb2p0_ptr,
                                        char *plug_usb3p0_ptr, 
                                        char *plug_usb_dport_ptr)
{
    sprintf(plug_usb_dport_ptr, "%s", HOST_USB0_DEVINFO);

    sprintf(plug_usb3p0_ptr, "%s", HOST_USB1_3P0_DEVINFO);
    sprintf(plug_usb2p0_ptr, "%s", HOST_USB1_2P0_DEVINFO);

    return (PASSED);
}    


/*******************************************************************************
 *  Function: plug_lte_telit_host_get_plug_usb_devinfo
 *  Description: Function to get the corresponding USB device info for 
 *               Telit pluggable module
 *  Inputs: plug_slot - which pluggable slot
 *          plug_usb2p0_ptr - pointer to the corresponding USB2.0 device info
 *          plug_usb3p0_ptr - pointer to the corresponding USB3.0 device info
 *          plug_dport_ptr  - pointer to the corresponding debug port device 
 *                            info
 *  Output: PASSED/FAILED
 *******************************************************************************
 */
int plug_lte_telit_host_get_plug_usb_devinfo (int plug_slot,
                                              char *plug_usb2p0_ptr,
                                              char *plug_usb3p0_ptr,
                                              char *plug_usb_dport_ptr)
{
    return (plug_lte_host_get_plug_usb_devinfo(plug_slot, plug_usb2p0_ptr,
                                               plug_usb3p0_ptr,
                                               plug_usb_dport_ptr));
}


/*******************************************************************************
 * Function   : plug_lte_host_get_modem_drv_path
 * Description: To get the path of LTE driver
 * Inputs     : drv_path - pointer to store the path of LTE modem driver
 *              (e.g. /diag, /firmware)
 * Outputs    : PASSED
 *******************************************************************************
 */
int plug_lte_host_get_modem_drv_path (char *drv_path)
{
    sprintf(drv_path, DRIVER_FOLDER);

    return (PASSED);
}


/*******************************************************************************
 * Function   : plug_lte_telit_host_get_modem_drv_path
 * Description: To get the path of LTE telit modem driver
 * Inputs     : drv_path - pointer to store the path of LTE telit modem driver
 *              (e.g. /diag, /firmware)
 * Outputs    : PASSED
 *******************************************************************************
 */
int plug_lte_telit_host_get_modem_drv_path (char *drv_path)
{
    plug_lte_host_get_modem_drv_path(drv_path);

    return (PASSED);
}


/*-------------------------------------------------
$Log: plug_lte_host_impl.c,v $
Revision 1.2  2019/10/17 02:16:27  kehuang2
Collapse Tabei-L into main trunk

Revision 1.1.2.3  2019/07/29 06:13:52  kodko
Clean up code based on off-line code review

Revision 1.1.2.2  2019/07/12 09:33:11  sherliu2
Supported Hyperloop-PIM

Revision 1.1.2.1  2018/10/26 08:40:50  kodko
Add support for PIM LTE and test card modules.

$Endlog$
*/

