/* $Id: platform_plug_serial_host.c,v 1.2 2018/11/23 09:28:46 hondwang Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/plug_serial/platform_plug_serial_host.c,v $
 *------------------------------------------------------------------
 *
 * platform_plug_serial_host.c: Pluggable Serial Host Function
 *                              (Needs to be implemented by host side)
 * Sep 2018 - Ian Chang
 *
 * Copyright (c) 2018 by Cisco Systems, Inc.
 * All rights reserved.
 *
 */

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <fcntl.h>
#include "common.h"
#include "types.h"
#include "error.h"
#include "platform_plug_serial_host.h"

#define PLUG_WARNING_MSG(func)         printf("'%s' is not implemented !!!\n", func);

int plug_ser_host_set_1000basex_mode(int);
int plug_ser_host_set_loopback_mode(int, int); 
int plug_ser_host_get_loopback_mode(int); 
void plug_ser_host_get_uart_info(int, char *, char *);
void plug_ser_host_get_server_ip_ethnum(int, char *, char *);

/*******************************************************************************
 *    
 * Function   : plug_ser_host_set_1000basex_mode
 * Description: Function to set CPU GE PHY to 1000Base-X mode.
 * Inputs     : slot - pluggable slot number
 * Outputs    : PASSED/FAILED
 *               
 *******************************************************************************
 */
int plug_ser_host_set_1000basex_mode (int slot)
__attribute__((weak, alias("__plug_ser_host_set_1000basex_mode")));
int __plug_ser_host_set_1000basex_mode (int slot)
{
    PLUG_WARNING_MSG(__func__)
    return (FAILED);
}
/*******************************************************************************
 *    
 * Function   : plug_ser_host_set_loopback_mode
 * Description: Function to set CPU PHY loopback mode for pluggable module.
 * Inputs     : slot - pluggable slot number
 *            : enable - enalbe / disable loopback
 * Outputs    : PASSED/FAILED
 *               
 *******************************************************************************
 */
int plug_ser_host_set_loopback_mode (int slot, int enable)
__attribute__((weak, alias("__plug_ser_host_set_loopback_mode")));
int __plug_ser_host_set_loopback_mode (int slot, int enable)
{
    PLUG_WARNING_MSG(__func__)
    return (FAILED);
}
/*******************************************************************************
 *    
 * Function   : plug_ser_host_get_loopback_mode
 * Description: Function to get CPU PHY loopback mode for pluggable module.
 * Inputs     : slot - pluggable slot number
 * Outputs    : PASSED/FAILED
 *               
 *******************************************************************************
 */
int plug_ser_host_get_loopback_mode (int slot)
__attribute__((weak, alias("__plug_ser_host_get_loopback_mode")));
int __plug_ser_host_get_loopback_mode (int slot)
{
    PLUG_WARNING_MSG(__func__)
    return (FAILED);
}
/*******************************************************************************
 *    
 * Function   : plug_ser_host_get_uart_info
 * Description: Function to get UART device name.
 * Inputs     : slot     - Pluggable slot no.
 *            : dev_name - UART device name (ie. ttySIRIUS)
 *            : drv_path - UART driver full path (ie. /diag/sirius_uart.ko)
 * Outputs    : None
 *               
 *******************************************************************************
 */
void plug_ser_host_get_uart_info (int slot, char *dev_name, char *drv_path)
__attribute__((weak, alias("__plug_ser_host_get_uart_info")));
int __plug_ser_host_get_uart_info (int slot, char *dev_name, char *drv_path)
{
    PLUG_WARNING_MSG(__func__)
    return (FAILED);
}

/*******************************************************************************
 *    
 * Function   : plug_ser_host_get_server_ip_ethnum
 * Description: Function to get Platform TFTP Server IP.
 * Inputs     : slot      - Pluggable slot no.
 *            : eth_num   - Platform GE name (ie. eth2)
 *            : server_ip - Platform TFTP Server IP (ie. 192.168.2.100)
 * Outputs    : None
 *               
 *******************************************************************************
 */
void plug_ser_host_get_server_ip_ethnum (int slot, char *eth_num, char *server_ip)
__attribute__((weak, alias("__plug_ser_host_get_server_ip_ethnum")));
int __plug_ser_host_get_server_ip_ethnum (int slot, char *eth_num, char *server_ip)
{
    PLUG_WARNING_MSG(__func__)
    return (FAILED);
}

/*
 *------------------------------------------------------------------
 * $Log: platform_plug_serial_host.c,v $
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
