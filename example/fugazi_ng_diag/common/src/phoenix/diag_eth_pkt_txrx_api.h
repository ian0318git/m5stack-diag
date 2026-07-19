/* $Id: diag_eth_pkt_txrx_api.h,v 1.2 2021/04/15 00:52:24 achiu2 Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/phoenix/diag_eth_pkt_txrx_api.h,v $
 *------------------------------------------------------------------
 * File: diag_eth_pkt_txrx_api.h
 * Description:Application for packets tx/rx
 * Aug 2015, Alan Peng
 * Copyright (c) 2018-2019 by Cisco Systems, Inc.
 * All rights reserved.
 *-----------------------------------------------------------------------------
 */
#ifndef __ETH_PKT_TXRX_API_H__
#define __ETH_PKT_TXRX_API_H__

#define DSP_SLOT0_ETH_INTF     2
#define DSP_SLOT1_ETH_INTF     3
#define DSP_SLOT0_ETH_PORT     2
#define DSP_SLOT1_ETH_PORT     3

extern void system_mac_addr_get(char *, mac_addr_t *);
extern int set_promisc(char *, int);
extern int clear_promisc(char *, int);
extern int setup_eth_dev(char *, int *);
extern int cleanup_eth_dev(char *, int);
extern void gen_eth_pkt(mac_addr_t, mac_addr_t, unsigned short,
                 unsigned char, char, int, unsigned char *);
extern int tx_a_pkt(int, unsigned char *, int);
extern int rx_a_pkt(int, unsigned char *, int);
extern int sgmii_lpbk_util(int, int);
extern int get_host_mac_addr(uint, unsigned char *);
extern void get_local_mac_addr(int, mac_addr_t *);
extern int get_sgmii_port_num(uint, uint);
extern int macstr2macaddr(char *, mac_addr_t *);
extern void set_host_eth_bridge(void);

#endif 
