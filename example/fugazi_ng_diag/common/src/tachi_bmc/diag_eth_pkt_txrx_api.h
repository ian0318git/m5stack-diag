/* $Id: diag_eth_pkt_txrx_api.h,v 1.3 2019/09/10 01:03:39 haohsu Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/tachi_bmc/diag_eth_pkt_txrx_api.h,v $
 *-----------------------------------------------------------------------------
 * File: diag_eth_pkt_txrx_api.h
 * Description:Application for packets tx/rx
 * Aug 2015, Alan Peng
 * Copyright (c) 2019 by Cisco Systems, Inc.
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
                 unsigned char, char, int, unsigned char *, int );
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
Revision 1.3  2019/09/10 01:03:39  haohsu
[CSCvr07313]-Marvell 6320 to BMC eth1 frame error issue

Revision 1.2  2016/04/20 11:25:28  benchen2
add tachi fru portion

Revision 1.1.2.3  2015/10/14 07:21:06  alpeng
update get host mac addr for f35

Revision 1.1.2.2  2015/08/21 06:46:28  alpeng
support ge/xaui test for testcard; clean up repo;

Revision 1.1.2.1  2015/08/04 01:34:19  hondwang
Application for packets tx/rx

$Endlog$
*/

