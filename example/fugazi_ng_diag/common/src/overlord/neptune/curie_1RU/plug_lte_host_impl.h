/* $Id: plug_lte_host_impl.h,v 1.2 2019/08/06 06:56:15 alpeng Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/overlord/neptune/curie_1RU/plug_lte_host_impl.h,v $
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

#define HOST_USB0_DEVINFO                   "3-1"
#define HOST_USB1_2P0_DEVINFO               "2-1"
#define HOST_USB1_3P0_DEVINFO               "4-6"
#define DIAG_FOLDER                         "/lib/modules/4.14.40+"


extern int plug_lte_host_get_plug_usb_devinfo(int, char *, char *, char *);
extern int plug_lte_telit_host_get_plug_usb_devinfo(int, char *, char *, char *);
extern int plug_lte_host_get_modem_drv_path(char *);
extern int plug_lte_telit_host_get_modem_drv_path(char *);

#endif

/*-------------------------------------------------
$Log: plug_lte_host_impl.h,v $
Revision 1.2  2019/08/06 06:56:15  alpeng
merge curie, switzer and nightwatch to trunk

Revision 1.1.2.5  2019/07/10 09:07:18  sherliu2
Supported Hyperloop-PIM

Revision 1.1.2.4  2018/10/16 09:05:40  meho
Pluggable re-structured



*/

