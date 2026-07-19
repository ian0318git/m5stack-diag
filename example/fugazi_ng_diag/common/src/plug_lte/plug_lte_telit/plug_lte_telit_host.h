/* $Id: plug_lte_telit_host.h,v 1.2 2019/05/14 08:48:37 sherliu2 Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/plug_lte/plug_lte_telit/plug_lte_telit_host.h,v $
 *------------------------------------------------------------------
 *
 * plug_lte_telit_host.h - Header File for Pluggable LTE Host
 *
 * Copyright (c) 2018 - 2019 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#ifndef __PLUG_LTE_TELIT_HOST_H__
#define __PLUG_LTE_TELIT_HOST_H__

extern int plug_lte_telit_host_get_plug_usb_devinfo(int, char *, char *,
                                                    char *);
extern int plug_lte_telit_host_get_modem_drv_path(char *);

#endif /* __PLUG_LTE_TELIT_HOST_H__ */

/*------------------------------------------------------------------
$Log: plug_lte_telit_host.h,v $
Revision 1.2  2019/05/14 08:48:37  sherliu2
Support hyperloop

Revision 1.1.2.1  2018/12/14 00:50:16  shjung
Initial check-in for Hyperloop



$Endlog$
*/
