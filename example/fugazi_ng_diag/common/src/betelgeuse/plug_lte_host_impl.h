/* $Id: plug_lte_host_impl.h,v 1.3 2019/05/14 09:44:09 sherliu2 Exp $ 
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/betelgeuse/plug_lte_host_impl.h,v $
 *------------------------------------------------------------------
 * 
 * plug_lte_host_impl.h
 *
 * Copyright (c) 2018 - 2019 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */
 
#ifndef __PLUG_LTE_HOST_IMPL__
#define __PLUG_LTE_HOST_IMPL__

#define HOST_USB0_DEVINFO                   "1-1"
#define HOST_USB1_2P0_DEVINFO               "3-1"
#define HOST_USB1_3P0_DEVINFO               "4-1"
#define DIAG_FOLDER                         "/diag"

extern int plug_lte_host_get_plug_usb_devinfo(int, char *, char *, char *);
extern int plug_lte_telit_host_get_plug_usb_devinfo(int, char *, char *, char *);
extern int plug_lte_host_get_modem_drv_path(char *);
extern int plug_lte_telit_host_get_modem_drv_path(char *);

#endif

/*-------------------------------------------------
 * $Log: plug_lte_host_impl.h,v $
 * Revision 1.3  2019/05/14 09:44:09  sherliu2
 * Support Hyerloop
 *
 * Revision 1.2.6.3  2019/02/25 06:19:25  sherliu2
 * Support Hyperloop on Betelgeuse
 *
 * Revision 1.2.6.2  2019/02/23 03:45:57  sherliu2
 * Sync up with main trunk
 *
 * Revision 1.2  2019/01/10 06:36:29  wilbhuan
 * The beginning of Betelgeuse application code.
 *
 *-------------------------------------------------
 */
