/* $Id: kalamata_uart.h,v 1.1 2018/03/30 03:24:09 letsai Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/kalamata/kalamata_uart.h,v $
 *******************************************************************************
 * File Name: kalamata_uart.h
 *
 * Description: UART parameter used for Kalamata 
 * 
 * Author: Kody Ko
 *
 * Copyright (c) 2018 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *******************************************************************************
 */

#ifndef KALAMATA_UART_H
#define KALAMATA_UART_H

#define KALAMATA_POWER_UP_DELAY     (1000)

#define KALAMATA_PING_TOUT          (10)  /* 10 secs */
#define KALAMATA_BOOT_TOUT          (30)  /* 30 secs */
#define KALAMATA_GO_TOUT            (30)  /* 30 secs */
#define KALAMATA_BL_PROMPT_TOUT     (600)  /* 600 secs */
#define KALAMATA_DEST_DIAG_IMG      "/firmware/kalamata.SSA"
#define KALAMATA_SRC_DIAG_IMG       "kalamata.SSA"
#define KALAMATA_BL_PROMPT          ">"
#define KALAMATA_CR_STRING          "\012"
#define KALAMATA_PING_ALIVE         "is alive"
#define KALAMATA_SAVE_ENV           "saveenv\012"


#define KALAMATA_SET_IPADDR         "setenv ipaddr 192.123.123.100\012"
#define KALAMATA_SET_NETMASK        "setenv netmask 255.255.255.0\012"
#define KALAMATA_SET_GETWAY         "setenv gateway 192.123.123.1\012"
#define KALAMATA_SET_SERVERIP       "setenv serverip 192.123.123.1\012"
#define KALAMATA_SET_ETH1           "setenv eth1addr 192.123.123.23\012"
#define KALAMATA_SET_ETH2           "setenv eth2addr 192.123.123.24\012"
#define KALAMATA_SET_ETH            "setenv ethaddr 192.123.123.25\012"
#define KALAMATA_SET_ETHACT         "setenv ethact eTSEC2\012"
#define KALAMATA_PING_SERVER        "ping 192.123.123.1\012"
#define KALAMATA_SET_FILENAME       "setenv bootfile /firmware/kalamata.SSA\012"
#define KALAMATA_BOOT_UP_CMD        "tftpboot\012"
#define KALAMATA_GO_ADDR            "go 0x1000000\012"
#define KALAMATA_RBCP_LOOP          "Non RBCP packet"
#define KALAMATA_BOOT_PASSED        "passed"

#endif /* KALAMATA_UART_H */

/******** History ********/
/*------------------------------------------------------------------------------
 * $Log: kalamata_uart.h,v $
 * Revision 1.1  2018/03/30 03:24:09  letsai
 * Change auto boot function to common code and can do all NIM tests automatically.
 *
 *
 *------------------------------------------------------------------------------
 * $Endlog$
 *------------------------------------------------------------------------------
 */

