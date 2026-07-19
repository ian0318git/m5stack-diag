/* $Id: plug_lte_host_impl.h,v 1.4 2019/05/14 09:18:32 sherliu2 Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/tsn/plug_lte_host_impl.h,v $
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
$Log: plug_lte_host_impl.h,v $
Revision 1.4  2019/05/14 09:18:32  sherliu2
Support hyperloop

Revision 1.3.4.1  2018/12/13 19:19:27  shjung
Supported Hyperloop PIM on Star platform

Revision 1.3  2018/11/23 08:49:53  hondwang
Re-instruct pluggable common code with CDETs CSCvn17216

Revision 1.2.54.2  2018/10/15 07:43:09  shjung
Re-struct for pluggable-LTE common codes

Revision 1.2.54.1  2018/10/15 06:53:08  hondwang
pluggable common code re-instruct modify code

Revision 1.2  2018/02/09 09:56:57  hondwang
Merge Star branch star-branch-c9xx to main trunk

Revision 1.1.6.2  2018/01/20 06:11:18  hondwang
prepare merge star-branch-c9xx to main trunk

Revision 1.1.4.6  2017/12/14 00:38:16  hondwang
Fix plug LTE USB2.0 detection issue.

Revision 1.1.4.5  2017/12/08 02:31:20  hondwang
Fix LTE USB2.0 detection fail with add C1109-4P USB HUb reset

Revision 1.1.4.4  2017/09/09 00:47:48  hondwang
Add C949-4P support with MB,Wifi,LTE EM

Revision 1.1.4.3  2017/08/16 08:31:59  tirawan
Re-enumerate USB modem by reconnecting modem when enabling USB 2.0 or USB 3.0

Revision 1.1.4.2  2017/08/15 14:18:39  hondwang
star branch c9xx initial check in

Revision 1.1.2.2  2017/07/24 23:39:23  tirawan
Correct the USB information path

Revision 1.1.2.1  2017/07/20 17:22:15  tirawan
Add Pluggable Host implementation codes



*/

