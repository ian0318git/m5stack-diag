/* $Id: plug_lte_host_impl.c,v 1.4 2019/05/14 09:18:32 sherliu2 Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/tsn/plug_lte_host_impl.c,v $
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
#include "platform_fpga.h"
#include "plug_host_fpga_lib.h"

int plug_lte_host_get_plug_usb_devinfo(int, char *, char *, char *);
int plug_lte_telit_host_get_plug_usb_devinfo(int, char *, char *, char *);
int plug_lte_host_get_modem_drv_path(char *);
int plug_lte_telit_host_get_modem_drv_path(char *);


/*******************************************************************************
 *  Function: plug_lte_host_get_plug_usb_devinfo
 *  Description: Function to get the corresponding USB device info for 
 *               pluggable module
 *  Inputs: plug_slot - which pluggable slot
 *          plug_usb2p0_ptr - pointer to the corresponding USB2.0 device info
 *          plug_usb3p0_ptr - pointer to the corresponding USB3.0 device info
 *          plug_dport_ptr  - pointer to the corresponding debug port device 
 *                            info
 *  Output: PASSED/FAILED
 *******************************************************************************
 */
int plug_lte_host_get_plug_usb_devinfo (int plug_slot, char *plug_usb2p0_ptr,
                                        char *plug_usb3p0_ptr, 
                                        char *plug_usb_dport_ptr)
{
    sprintf(plug_usb_dport_ptr, "%s", HOST_USB0_DEVINFO);

    if (this_is_star_c1109_4p() == TRUE) {
        sprintf(plug_usb3p0_ptr, "%s.%d", HOST_USB1_3P0_DEVINFO, plug_slot);
        sprintf(plug_usb2p0_ptr, "%s.%d", HOST_USB1_2P0_DEVINFO, plug_slot);
    } else {
        sprintf(plug_usb3p0_ptr, "%s", HOST_USB1_3P0_DEVINFO);
        sprintf(plug_usb2p0_ptr, "%s", HOST_USB1_2P0_DEVINFO);
    }   
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
$Log: plug_lte_host_impl.c,v $
Revision 1.4  2019/05/14 09:18:32  sherliu2
Support hyperloop

Revision 1.3.4.2  2019/02/27 02:48:02  sherliu2
Modified Hyperloop PIM code

Revision 1.3.4.1  2018/12/13 19:19:27  shjung
Supported Hyperloop PIM on Star platform

Revision 1.3  2018/11/23 08:49:53  hondwang
Re-instruct pluggable common code with CDETs CSCvn17216

Revision 1.2.54.2  2018/10/15 07:43:09  shjung
Re-struct for pluggable-LTE common codes

Revision 1.2.54.1  2018/10/15 06:53:07  hondwang
pluggable common code re-instruct modify code

Revision 1.2  2018/02/09 09:56:57  hondwang
Merge Star branch star-branch-c9xx to main trunk

Revision 1.1.6.2  2018/01/20 06:11:18  hondwang
prepare merge star-branch-c9xx to main trunk

Revision 1.1.4.8  2017/12/14 00:38:16  hondwang
Fix plug LTE USB2.0 detection issue.

Revision 1.1.4.7  2017/12/08 02:31:20  hondwang
Fix LTE USB2.0 detection fail with add C1109-4P USB HUb reset

Revision 1.1.4.6  2017/11/20 07:54:32  lucywang
Changed PID to C1101/C1109-2P/C1109-4P

Revision 1.1.4.5  2017/09/09 00:47:48  hondwang
Add C949-4P support with MB,Wifi,LTE EM

Revision 1.1.4.4  2017/08/16 08:31:59  tirawan
Re-enumerate USB modem by reconnecting modem when enabling USB 2.0 or USB 3.0

Revision 1.1.4.3  2017/08/16 07:24:28  shjung
Reread cookie id while entering pluggable test menu and fix usb mode switching issue

Revision 1.1.4.2  2017/08/15 14:18:39  hondwang
star branch c9xx initial check in

Revision 1.1.2.1  2017/07/20 17:22:15  tirawan
Add Pluggable Host implementation codes


*/

