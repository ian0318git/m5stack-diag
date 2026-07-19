/* $Id: platform_plug_serial_host.h,v 1.2 2018/11/23 09:28:46 hondwang Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/plug_serial/platform_plug_serial_host.h,v $
 *------------------------------------------------------------------
 *
 * platform_plug_serial_host.h: Header file for Pluggable Serial Host
 *                              (Needs to be implemented by host side)
 * Sep 2018 - Ian Chang
 *
 * Copyright (c) 2018 by Cisco Systems, Inc.
 * All rights reserved.
 *
 */
 
#ifndef __PLATFORM_PLUG_SERIAL_HOST__
#define __PLATFORM_PLUG_SERIAL_HOST__

extern int plug_ser_host_set_1000basex_mode(int);
extern int plug_ser_host_set_loopback_mode(int, int); 
extern int plug_ser_host_get_loopback_mode(int); 
extern void plug_ser_host_get_uart_info(int, char *, char *);
extern void plug_ser_host_get_server_ip_ethnum(int, char *, char *);


#endif

/*
 *------------------------------------------------------------------
 * $Log: platform_plug_serial_host.h,v $
 * Revision 1.2  2018/11/23 09:28:46  hondwang
 * Re-instruct pluggable common code with CDETs CSCvn17216
 *
 * Revision 1.1.2.1  2018/10/15 06:44:31  hondwang
 * pluggable common code re-instruct add and remove files
 *
 *
 *------------------------------------------------------------------
 * $Endlog$
 */

