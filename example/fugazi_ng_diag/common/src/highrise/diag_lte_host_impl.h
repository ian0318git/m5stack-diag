 /*------------------------------------------------------------------
 * 
 * diag_lte_host_impl.h
 *
 * Copyright (c) 2018 - 2019 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */
 
#ifndef __LTE_HOST_IMPL__
#define __LTE_HOST_IMPL__

#define HOST_USB0_DEVINFO                   "1-1"
#define HOST_USB1_2P0_DEVINFO               "1-1"
#define HOST_USB1_3P0_DEVINFO               "2-1"
#define DIAG_FOLDER                         "/diag"

extern int diag_lte_host_get_usb_devinfo(char *, char *, char *);
extern int diag_lte_telit_host_get_usb_devinfo(char *, char *, char *);
extern int diag_lte_host_get_modem_drv_path(char *);
extern int diag_lte_telit_host_get_modem_drv_path(char *);

#endif

/*-------------------------------------------------
 * $Log: diag_lte_host_impl.h,v $
 * Revision 1.1  2020/08/19 09:49:34  markzha
 * *** empty log message ***
 *
 *
 *-------------------------------------------------
 */
