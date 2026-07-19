/* $Id: plug_lte_host_impl.h,v 1.2 2019/10/17 02:16:27 kehuang2 Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/tabei-l/plug_lte_host_impl.h,v $
 *------------------------------------------------------------------
 *
 * plug_lte_host_impl.h - Header file for Pluggable LTE Host 
 *                        Implementation
 *
 * Copyright (c) 2017 - 2019 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */
 
#ifndef __PLUG_LTE_HOST_IMPL__
#define __PLUG_LTE_HOST_IMPL__

#define HOST_USB0_DEVINFO                   "1-4"
#define HOST_USB1_2P0_DEVINFO               "1-2"
#define HOST_USB1_3P0_DEVINFO               "2-2"
#define DRIVER_FOLDER                       "/lib/modules/4.14.3"


extern int plug_lte_host_get_plug_usb_devinfo(int, char *, char *, char *);
extern int plug_lte_telit_host_get_plug_usb_devinfo(int, char *, char *, char *);
extern int plug_lte_host_get_modem_drv_path(char *);
extern int plug_lte_telit_host_get_modem_drv_path(char *);

#endif

/*-------------------------------------------------
$Log: plug_lte_host_impl.h,v $
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

