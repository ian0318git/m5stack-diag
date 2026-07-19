/* $Id: plug_serial_host_impl.h,v 1.2 2021/09/24 01:21:08 harrchan Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/elixir/plug_serial_host_impl.h,v $
 *------------------------------------------------------------------
 *
 * plug_serial_host_impl.h
 *                              
 * Copyright (c) 2019 by Cisco Systems, Inc.
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
 * Revision 1.2  2021/09/24 01:21:08  harrchan
 * Collapse Elixir-branch to Main Trunk.
 *
 * Revision 1.1.2.1  2020/09/09 09:09:54  illiu
 * First version which has been ported with Dreamliner and Marvell CPSS
 *
 * Revision 1.1  2019/06/24 07:21:38  wilbhuan
 * Supported Pluggable Serial Module.
 *
 *------------------------------------------------------------------
 * $Endlog$
 */
