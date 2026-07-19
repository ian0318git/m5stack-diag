/* 
 * $Id: plug_NR_5G_telit_host.h,v 1.2 2021/06/02 02:56:19 alpeng Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/plug_NR_5G/plug_NR_5G_telit/plug_NR_5G_telit_host.h,v $
 *
 *------------------------------------------------------------------
 *
 * plug_NR_5G_telit_host.h - Header File for Pluggable LTE Host
 *
 * Copyright (c) 2020 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#ifndef __PLUG_NR_5G_TELIT_HOST_H__
#define __PLUG_NR_5G_TELIT_HOST_H__

extern int plug_lte_telit_host_get_plug_usb_devinfo(int, char *, char *,
                                                    char *);
extern int plug_lte_telit_host_get_modem_drv_path(char *);

#endif /* __PLUG_NR_5G_TELIT_HOST_H__ */
/*********************************************************************
 * $Log: plug_NR_5G_telit_host.h,v $
 * Revision 1.2  2021/06/02 02:56:19  alpeng
 * merge sears into trunk
 *
 * Revision 1.1.2.2  2020/12/02 03:57:22  tshanmug
 * Sears Antenna test updated
 *
 *
 * $Endlog$
 */

