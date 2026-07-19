 /* $Id: diag_eth_pkt_txrx_api.h,v 1.2 2021/09/24 01:21:06 harrchan Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/elixir/diag_eth_pkt_txrx_api.h,v $
 *-----------------------------------------------------------------------------
 * File: diag_eth_pkt_txrx_api.h
 * Description:Application for packets tx/rx
 * 
 * Copyright (c) 2018-2019 by Cisco Systems, Inc.
 * All rights reserved.
 *-----------------------------------------------------------------------------
 */
#ifndef __ETH_PKT_TXRX_API_H__
#define __ETH_PKT_TXRX_API_H__

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
extern int get_ctrl_plane_sgmii_port(void);
extern int get_sgmii_port_num(uint, uint);

/* for future use */
#if 0
static int eth_is_linkup(int);
#endif 

#endif 
/*---------------------------------------------------------------
$Log: diag_eth_pkt_txrx_api.h,v $
Revision 1.2  2021/09/24 01:21:06  harrchan
Collapse Elixir-branch to Main Trunk.

Revision 1.1.2.1  2020/11/05 06:34:55  harrchan
1.Base on P1A bring up result to Modify the AC5 MAC/internal/external loopback test
2.Remove some debug message on AC5 init process

Revision 1.2  2019/12/11 10:10:29  lucywang
Merged Nanook to main trunk


$Endlog$
*/

