/* $Id: plug_lte_host.h,v 1.4 2018/11/23 09:15:07 hondwang Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/plug_lte/plug_lte_host.h,v $
 *------------------------------------------------------------------
 *
 * plug_lte_host.h - Header file for Pluggable LTE Host 
 *
 * Copyright (c) 2018 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */
 
#ifndef __PLUG_LTE_HOST__
#define __PLUG_LTE_HOST__

extern int plug_lte_host_get_plug_usb_devinfo(int, char *, char *, char *);
extern int plug_lte_host_get_modem_drv_path(char *);

#endif

/*-------------------------------------------------
$Log: plug_lte_host.h,v $
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

