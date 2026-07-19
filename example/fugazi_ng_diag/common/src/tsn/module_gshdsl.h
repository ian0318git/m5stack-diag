/* $Id: module_gshdsl.h,v 1.1 2018/05/09 06:53:12 letsai Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/tsn/module_gshdsl.h,v $
 *******************************************************************************
 * File Name: module_gshdsl.h
 *
 * Description: GSHDSL  main header file
 *
 * Author: Kody Ko
 *
 * Copyright (c) 2018 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *******************************************************************************
 */

#ifndef MODULE_GSHDSL_H
#define MODULE_GSHDSL_H

#include "ngio.h"


#define GSHDSL_POWER_UP_DELAY     (1000)

#define GSHDSL_PING_TOUT          (10)  /* 10 secs */
#define GSHDSL_BOOT_TOUT          (30)  /* 30 secs */
#define GSHDSL_GO_TOUT            (30)  /* 30 secs */
#define GSHDSL_BL_PROMPT_TOUT     (600)  /* 600 secs */
#define GSHDSL_SET_UART            "/dev/ttyS1"

#define GSHDSL_DEST_DIAG_IMG      "kalamata.SSA"

#define GSHDSL_BL_PROMPT          ">"
#define GSHDSL_CR_STRING          "\012"
#define GSHDSL_PING_ALIVE         "is alive"
#define GSHDSL_SAVE_ENV           "saveenv\012"

#define GSHDSL_SET_GATEWAY        "setenv gateway 192.123.123.1\012"
#define GSHDSL_SET_IPADDR         "setenv ipaddr 192.168.2.101\012"
#define GSHDSL_SET_NETMASK        "setenv netmask 255.255.255.0\012"
#define GSHDSL_SET_GETWAY         "setenv getway 192.168.2.100\012"
#define GSHDSL_SET_SERVERIP       "setenv serverip 192.168.2.100\012"
#define GSHDSL_SET_ETH1           "setenv eth1addr 192.168.2.102\012"
#define GSHDSL_SET_ETH2           "setenv eth2addr 192.168.2.103\012"
#define GSHDSL_SET_ETH            "setenv ethaddr 192.168.2.104\012"
#define GSHDSL_SET_ETHACT         "setenv ethact eTSEC2\012"
#define GSHDSL_PING_SERVER        "ping 192.168.2.100\012"
#define GSHDSL_SET_FILENAME       "setenv bootfile kalamata.SSA\012"
#define GSHDSL_BOOT_UP_CMD        "tftpboot\012"
#define GSHDSL_GO_ADDR            "go 0x1000000\012"
#define GSHDSL_RBCP_LOOP          "Non RBCP packet"
#define GSHDSL_BOOT_PASSED        "passed"
#define GSHDSL_BOOT_DONE          "done"

extern int gshdsl_tests(int);

#endif /* MODULE_GSHDSL_H */

/******** History ********/
/*------------------------------------------------------------------------------
 * $Log: module_gshdsl.h,v $
 * Revision 1.1  2018/05/09 06:53:12  letsai
 * Add TSN GSHDSL portion
 *
 *
 *------------------------------------------------------------------------------
 * $Endlog$
 *------------------------------------------------------------------------------
 */

