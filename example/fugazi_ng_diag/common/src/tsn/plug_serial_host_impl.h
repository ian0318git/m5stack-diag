/* $Id: plug_serial_host_impl.h,v 1.2 2018/11/23 08:49:53 hondwang Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/tsn/plug_serial_host_impl.h,v $
 *------------------------------------------------------------------
 *
 * plug_serial_host_impl.h: Header file for Pluggable Serial Host Function
 *                              
 * Sep 2018 - Ian Chang
 *
 * Copyright (c) 2018 by Cisco Systems, Inc.
 * All rights reserved.
 *
 */ 
#ifndef __PLUG_SERIAL_HOST_IMPL__
#define __PLUG_SERIAL_HOST_IMPL__

#define PLUG_HOST_UART_DEVICE_NAME      "ttySIRIUS"
#define PLUG_HOST_UART_DRIVER_PATH      "/diag/sirius_uart.ko"

extern int plug_ser_host_set_1000basex_mode(int);
extern int plug_ser_host_set_loopback_mode(int, int); 
extern int plug_ser_host_get_loopback_mode(int); 
extern void plug_ser_host_get_uart_info(int, char *, char *);
extern void plug_ser_host_get_server_ip_ethnum(int, char *, char *);


#endif

/*
 *------------------------------------------------------------------
 * $Log: plug_serial_host_impl.h,v $
 * Revision 1.2  2018/11/23 08:49:53  hondwang
 * Re-instruct pluggable common code with CDETs CSCvn17216
 *
 * Revision 1.1.2.1  2018/10/15 06:44:32  hondwang
 * pluggable common code re-instruct add and remove files
 *
 *
 *------------------------------------------------------------------
 * $Endlog$
 */
