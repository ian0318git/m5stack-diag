/* $Id: dnv_eth_lib.h,v 1.2 2021/04/15 00:52:26 achiu2 Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/phoenix/dnv_eth_lib.h,v $
 *------------------------------------------------------------------
 * 
 * dnv_eth_lib.h
 *
 * Copyright (c) 2019 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */
#ifndef _DNV_ETH_LIB_H_
#define _DNV_ETH_LIB_H_

/* Phoenix Interface */
#define PHOENIX_ETH0_BP                             "eth0" 
#define PHOENIX_ETH1_BP                             "eth1" 

#define PHOENIX_ETH_NIM0_SLOT_UP          "ifconfig eth0 192.123.123.1 up > /dev/null"
#define PHOENIX_ETH_NIM1_SLOT_UP          "ifconfig eth1 192.123.124.1 up > /dev/null"
#define PHOENIX_ETH_DSP0_SLOT_UP          "ifconfig eth2 192.123.125.1 up > /dev/null"

#define PHOENIX_ETH_NIM0_SLOT_DOWN        "ifconfig eth0 down > /dev/null"
#define PHOENIX_ETH_NIM1_SLOT_DOWN        "ifconfig eth1 down > /dev/null"

#define PHOENIX_NIM0_SLOT                 1
#define PHOENIX_NIM1_SLOT                 2

#define DISPLAY_I350_PORT0_CAP         "ethtool i350_eth0"
#define DISPLAY_I350_PORT1_CAP         "ethtool i350_eth1"

#define WAIT_BK_LINK_UP      (1000)
#define PHOENIX_ETH_RETRY               3

#define PHOENIX_DHCPD           "dhcpd &> /dev/null"
#define PHOENIX_OPENTFTP        "opentftpd"
#define PHOENIX_KILL_DHCPD      "killall dhcpd"
#define PHOENIX_KILL_OPENTFTP   "killall opentftpd"

#endif /* DNV_ETH_LIB_H_ */

