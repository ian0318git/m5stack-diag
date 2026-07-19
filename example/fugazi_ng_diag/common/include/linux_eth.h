/* $Id: linux_eth.h,v 1.2 2012/03/28 00:38:11 mcharon Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/include/linux_eth.h,v $
 *------------------------------------------------------------------
 * Header file for linux base ethernet port tests
 * 
 * Sept 2010 ptong
 *
 * Copyright (c) 2010-2012 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#ifndef __LINUX_ETH_H__
#define __LINUX_ETH_H__

/* A random pick of L4 port number. The socket code for
 * IP packet TX requires the port number in the IP header
 */
#define L4_DEST_PORT 0x2222

/* Linux ethernet interface name */
#define NAME_ETH        "eth"
#define NAME_XAUI       "xaui"
#define NAME_MGMT       "mgmt"

/* Struct to specify the packet payload first byte
 * value and packet lengt
 */
typedef struct pktdata_info {
    char val;
    int len;
    uint hkpflags;
    int send_count; /* 0: do not send, 1: send 1 time, etc. */
} pktdata_info_t;

/* Eth port information such as IP source and destination addresses used to
 * set up the port under test.
 */
typedef struct eth_lpbk_info {
  uint32 eth_port;
  char   *eth_name;         // eth1, eth2
  char   *ip_src_str;       // number and dot form
  char   *ip_dst_str;
  char   *ip_netmask_str;
  uint32 ip_src;
  uint32 ip_dst;
  pktdata_info_t  *pkt_tb_p;
  uchar  pktb_sz;
} eth_lpbk_info_t;

/* Ethernet socket parameters needed to set up the socket
 * connection to an interface.
 */
typedef struct skinfo {
  int skid;
  int skfamily;
  int sktype;
  int skprotocol;
  struct ifreq ethreq;
} skinfo_t;

/* Ethernet port information such as interface name, 
 * ip addres, socket info needed to set up the socket
 * connection to the interface.
 * to use this struct, include <netinet/in.h> first
 */
typedef struct ethport_info {
  uint portid;
  char *ifname;
  char *ipstr;
  uint ipaddr;
  char *netmask;
  struct sockaddr_in skaddr_in;	 
  skinfo_t skt;
} ethport_info_t;


extern int eth_port_lpbk(eth_lpbk_info_t *ethp);
extern int skio_cfg(int sock, ushort flag, void *inet_param, struct ifreq *ethreq_p);
extern int init_ip_socket(ethport_info_t *ethport_info_p);

#endif /* __LINUX_ETH_H__ */
/* end of module */

/******** History ******** 
$Log: linux_eth.h,v $
Revision 1.2  2012/03/28 00:38:11  mcharon
remove forward slash from second line

Revision 1.1.1.1  2012/03/23 23:02:03  ptong
Initial archive of ng_diag module


$Endlog$
*/
