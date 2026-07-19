/* $Id: plug_lte_host_impl.c,v 1.2 2021/09/24 01:21:08 harrchan Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/elixir/plug_lte_host_impl.c,v $
 *------------------------------------------------------------------
 * 
 * plug_lte_host_impl.c
 *
 * Copyright (c) 2018 - 2019 by Cisco Systems, Inc.
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
#include "plug_common_host_impl.h"

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
 *  Output: PASSED/FAILED
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
 * Outputs    : PASSED or FAILED
 *******************************************************************************
 */
int plug_lte_host_get_modem_drv_path (char *drv_path)
{
    sprintf(drv_path, DIAG_FOLDER);

    return (PASSED);
}


/*******************************************************************************
 * Function   : plug_lte_telit_host_get_modem_drv_path
 * Description: To get the path of LTE telit modem driver
 * Inputs     : drv_path - pointer to store the path of LTE telit modem driver
 *              (e.g. /diag, /firmware)
 * Outputs    : PASSED or FAILED
 *******************************************************************************
 */
int plug_lte_telit_host_get_modem_drv_path (char *drv_path)
{
    plug_lte_host_get_modem_drv_path(drv_path);
    
    return (PASSED);
}
/*-------------------------------------------------
 * $Log: plug_lte_host_impl.c,v $
 * Revision 1.2  2021/09/24 01:21:08  harrchan
 * Collapse Elixir-branch to Main Trunk.
 *
 * Revision 1.1.2.1  2020/09/09 09:08:08  illiu
 * First version which has been ported with Dreamliner and Marvel CPSS
 *
 * Revision 1.3  2019/05/14 09:44:09  sherliu2
 * Support Hyerloop
 *
 * Revision 1.2.6.3  2019/02/25 06:19:25  sherliu2
 * Support Hyperloop on Betelgeuse
 *
 * Revision 1.2.6.2  2019/02/23 03:45:57  sherliu2
 * Sync up with main trunk
 *
 * Revision 1.2  2019/01/10 06:36:25  wilbhuan
 * The beginning of Betelgeuse application code.
 *
 *-------------------------------------------------
 */
