/* $Id: plug_serial_host_impl.c,v 1.2 2021/09/24 01:21:08 harrchan Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/elixir/plug_serial_host_impl.c,v $
 *------------------------------------------------------------------
 *
 * plug_serial_host_impl.c
 *
 * Copyright (c) 2019 by Cisco Systems, Inc.
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
#include <ifaddrs.h>
#include <netinet/in.h> 
#include <arpa/inet.h>
#include "common.h"
#include "types.h"
#include "error.h"
#include "plat_defs.h"
#include "diag_ge_phy_lib.h"
#include "plug_serial_host_impl.h"
#include "plug_host_fpga_lib.h"
#include "diag_cpu_lib.h"

int plug_ser_host_set_1000basex_mode(int);
int plug_ser_host_set_loopback_mode(int, int); 
int plug_ser_host_get_loopback_mode(int); 
void plug_ser_host_get_uart_info(int, char *, char *);
void plug_ser_host_get_tftp_server_ip(int, char *, char *);

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
{
    int  ge_num = slot;

    if (slot == PLUG_SLOT_1) {
        /* Set Platform PHY with 1000Base-X mode. */
        if(gephy_set_1000basex_mode() == FAILED){
            cterr('f', 0, "Set PHY 1000Base-X Mode Failed");
            return (FAILED);
        }
    } else {
        printf("%s: Unknown GE number(%d)\n", __FUNCTION__, ge_num);
        return (FAILED);
    }
    return (PASSED);
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
{
    int  ge_num = slot;

    if (slot == PLUG_SLOT_1) {
        /* clear bit[15] of Digitable Loopback Enable Register */
	gephy_set_loopback_mode(enable);	
    } else {
        printf("%s: Unknown GE number(%d) for loopback mode\n", 
               __FUNCTION__, ge_num);
        return (FAILED);
    }
    return (PASSED);
}

/*******************************************************************************
 *    
 * Function   : plug_ser_host_get_loopback_mode
 * Description: Function to get CPU PHY loopback mode for pluggable module.
 * Inputs     : slot - pluggable slot number
 * Outputs    : 0 - disabled, 1 - enabled  TRUE/FALSE
 *               
 *******************************************************************************
 */
int plug_ser_host_get_loopback_mode (int slot)
{
    int  ge_num = slot;

    if (slot == PLUG_SLOT_1) {
        /* enable bit[15] of Digitable Loopback Enable Register, 
         * pluggable interface at Lane1/SGMII2, offset comphy num1: 0xF212188C */
        return gephy_get_loopback_mode();
    } else {
        printf("%s: Unknown GE number(%d) for loopback mode\n", 
                __FUNCTION__, ge_num);
        return (FAILED);
    }
    return (PASSED);
}

/*-------------------------------------------------------------------
 * Function : plug_ser_host_get_uart_info
 * Description: Get Uart information with pluggable serial
 * Inputs     : slot     - Pluggable slot no.
 *            : dev_name - UART device name (ie. ttySIRIUS)
 *            : drv_path - UART driver full path (ie. /diag/sirius_uart.ko)
 * OUTPUT: None
 * -------------------------------------------------------------------
*/
void plug_ser_host_get_uart_info (int slot, char *dev_name, char *drv_path)
{
    sprintf(dev_name, "%s%d", PLUG_HOST_UART_DEVICE_NAME, slot - 1);
    sprintf(drv_path, "%s", PLUG_HOST_UART_DRIVER_PATH);
}

/*******************************************************************************
 *    
 * Function   : plug_ser_host_get_server_ip_ethnum
 * Description: Function to get Platform TFTP Server IP, and eth name.
 * Inputs     : slot      - Pluggable slot no.
 *            : eth_num   - Platform GE name (ie. eth2)
 *            : server_ip - Platform TFTP Server IP (ie. 192.168.2.100)
 * Outputs    : None
 *               
 *******************************************************************************
 */
void plug_ser_host_get_server_ip_ethnum (int slot, char *eth_num, char *server_ip)
{
    struct ifaddrs *ifaddrstruct = NULL;
    char addressbuffer[INET_ADDRSTRLEN];
    void *tmpAddrPtr = NULL;
    uint cpu_reg_addr = (uint)CPU_PORT_AN_CONF_REG(3);
    uint cpu_reg_val = 0x0;

    /* A configuration to set CPU's GE link up to pluggable serial module */
    cpu_reg_val = PANCR_RESERVED | 
                  PANCR_SET_FULL_DUPLEX | 
                  PANCR_SET_SGMII_1000 | 
                  PANCR_INBAND_BYPASS_EN | 
                  PANCR_INBAND_AN_EN;
    if (plat_mem_write32(cpu_reg_addr, cpu_reg_val) != PASSED) {
        printf("%s:%d:Failed to config CPU GE%d reg. 0x%08X\n", __FUNCTION__, __LINE__, GE1, cpu_reg_addr);
    }

    if (slot == PLUG_SLOT_1) {
        sprintf(eth_num, "eth%d", ETH2);
    }
    getifaddrs(&ifaddrstruct);
    while (ifaddrstruct!=NULL) {
        if (ifaddrstruct->ifa_addr->sa_family == AF_INET) { /* check IPV4 */
            tmpAddrPtr=&((struct sockaddr_in *)ifaddrstruct->ifa_addr)->sin_addr;
            inet_ntop(AF_INET, tmpAddrPtr, addressbuffer, INET_ADDRSTRLEN);
            if (strstr(ifaddrstruct->ifa_name, eth_num) != NULL) { 
                printf("TFTP Server IP %s\n", addressbuffer);
                sprintf(server_ip, "%s", addressbuffer);
                fflush(stdout);
            }
        }
        ifaddrstruct=ifaddrstruct->ifa_next;
    }
}

/*
 *------------------------------------------------------------------
 * $Log: plug_serial_host_impl.c,v $
 * Revision 1.2  2021/09/24 01:21:08  harrchan
 * Collapse Elixir-branch to Main Trunk.
 *
 * Revision 1.1.2.1  2020/09/09 09:08:08  illiu
 * First version which has been ported with Dreamliner and Marvel CPSS
 *
 * Revision 1.1  2019/06/24 07:21:37  wilbhuan
 * Supported Pluggable Serial Module.
 *
 *------------------------------------------------------------------
 * $Endlog$
 */
