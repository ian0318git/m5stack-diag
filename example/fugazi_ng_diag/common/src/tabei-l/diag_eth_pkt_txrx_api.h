 /* $Id: diag_eth_pkt_txrx_api.h,v 1.3 2019/11/25 08:55:51 kehuang2 Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/tabei-l/diag_eth_pkt_txrx_api.h,v $
 *-----------------------------------------------------------------------------
 * File: diag_eth_pkt_txrx_api.h
 * Description:Application for packets tx/rx
 * Aug 2015, Alan Peng
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
extern int get_sgmii_port_num(uint, uint);

/* for future use */
#if 0
static int eth_is_linkup(int);
#endif 

#endif 
/*---------------------------------------------------------------
$Log: diag_eth_pkt_txrx_api.h,v $
Revision 1.3  2019/11/25 08:55:51  kehuang2
Collapse Tabei-L into main trunk

Revision 1.2  2019/10/17 02:16:21  kehuang2
Collapse Tabei-L into main trunk

Revision 1.1.2.1  2018/10/02 01:49:58  harrchan
Initial commit for Tabei-L P1A bring up.

$Endlog$
*/

