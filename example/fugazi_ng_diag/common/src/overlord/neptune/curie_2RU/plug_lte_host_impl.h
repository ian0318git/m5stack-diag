/* $Id: plug_lte_host_impl.h,v 1.1 2020/01/09 01:02:06 jiajliu Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/overlord/neptune/curie_2RU/plug_lte_host_impl.h,v $
 *------------------------------------------------------------------
 *
 * plug_lte_host_impl.h - Header file for Pluggable LTE Host 
 *                        Implementation
 *
 * Copyright (c) 2017 - 2018 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */
 
#ifndef __PLUG_LTE_HOST_IMPL__
#define __PLUG_LTE_HOST_IMPL__

#define HOST_USB0_DEVINFO                   "1-1"
#define HOST_USB1_2P0_DEVINFO               "2-1"
#define HOST_USB1_3P0_DEVINFO               "4-6"
#define SIERRA_DRV_PATH                     "/lib/modules/4.14.40+"

#define C2RU_HOST_USB1_2P0_DEVINFO          "1-4"
#define C2RU_HOST_USB1_3P0_DEVINFO          "2-10"

extern int plug_lte_host_get_plug_usb_devinfo(int, char *, char *, char *);
extern int plug_lte_host_get_modem_drv_path(char *);
extern int plug_lte_telit_host_get_plug_usb_devinfo(int, char *, char *, char *);
extern int plug_lte_telit_host_get_modem_drv_path(char *);



#endif

/*
 *-----------------------------------------------------------------------------
$Log: plug_lte_host_impl.h,v $
Revision 1.1  2020/01/09 01:02:06  jiajliu
Merge Curie 2RU to main trunk

$Endlog$
 *-----------------------------------------------------------------------------
 */
