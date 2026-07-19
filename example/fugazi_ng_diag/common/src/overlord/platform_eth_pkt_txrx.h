/* $Id: platform_eth_pkt_txrx.h,v 1.7 2019/07/19 08:34:08 alpeng Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/overlord/platform_eth_pkt_txrx.h,v $
 *------------------------------------------------------------------
 *
 * platform_eth_pkt_txrx.h - Platform ethernet packet tx and rx api defines
 *
 * Oct 2011, Paul Tong
 *
 * Copyright (c) 2013-2019 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#ifndef _PLATFORM_ETH_PKT_TXRX_H_
#include "ethernet.h"
#include "router_if.h"
#include "sgmii_defs.h"
#define _PLATFORM_ETH_PKT_TXRX_H_

#define XAUI_PKT_BUF_LEN   10240 + 60  /* 60 extra bytes */
#define ETH_PKT_BUF_LEN	   1600
#define ETH_MIN_LEN	   64
#define ETH_MAX_LEN	   1518

extern uchar *ifcfg_up_str[];
extern char *ifcfg_str[];
extern uchar *ifcfg_dn_str[];

extern int tx_a_pkt(int, uchar *, int);
extern int rx_a_pkt(int, uchar *, int);
extern int eth_pkt_tx (eth_tx_pkt_t *);
extern int eth_pkt_rx (eth_rx_pkt_t *);
extern int rbcp_eth_pkt_rx (eth_rx_pkt_t *);
extern void gen_eth_pkt(mac_addr_t macda, 
			mac_addr_t macsa, 
			ushort pkt_type,
			uchar seed, 
			char inc_dec, 
			int payload_len,
			uchar *buf_p);
extern void cavecreek_sgmii_macsa_declare(void);
extern void ctrl_plane_sgmii_macsa_declare(void);
extern void local_mac_addrs_init(void);
extern int get_ctrl_plane_sgmii_port (void);
extern void get_local_mac_addr(int, mac_addr_t *);
extern void do_ifconfig(int, char *);
extern void utah_do_ifconfig(int, char *);
extern int get_sgmii_port_num(uint, uint);
extern int get_host_mac_addr(uint, unsigned char *);
extern void get_host_port_ip(char *ip_str_buf);
extern int chk_macaddr(uchar *, uchar *);
extern int host_send_packet(char *if_name, mac_addr_t,
                            mac_addr_t, ushort pkt_type,
                            int, uchar seed, char, unsigned char *);
extern void system_mac_addr_get(char *name, mac_addr_t *mac_buf);
extern int test_txrx(char *, char *, char *,
                     int pkt_len, int, char* );
extern boolean eth_is_linkup(int); /* from cavecreek_sgmii.c */


#endif  /* _PLATFORM_ETH_PKT_TXRX_H_ */


/******** History ******** 
$Log: platform_eth_pkt_txrx.h,v $
Revision 1.7  2019/07/19 08:34:08  alpeng
support sm testcard w/ bcm57412

Revision 1.6  2014/01/28 02:40:35  ptong
Host SGMII port to GE switch use a fix IP address of 192.123.123.1 to support NGIO module code

Revision 1.5  2013/11/26 08:40:36  hroni
fix compiler warning

Revision 1.4  2013/11/11 21:18:40  mcharon
pass string instead of number in first argum of host_send_packet ; add xaui support

Revision 1.3  2013/10/07 21:12:12  ptong
Add get_ctrl_plane_sgmii_port() to support platform specific control plane SGMII port connection to GESW

Revision 1.2  2013/09/06 22:56:19  ptong
Support Utah with ctrl_plane_sgmii_macsa_declare

Revision 1.1  2013/05/09 05:42:39  alpeng
moving overlord common code from x86

Revision 1.6  2012/11/07 10:58:16  alpeng
remove useless file and clean up code

Revision 1.5  2012/09/14 01:12:38  ptong
Code clean up and add comments

Revision 1.4  2012/06/28 21:41:48  srane
Declare the chk_mac_addr routine (compile warning).

Revision 1.3  2012/04/24 08:30:15  hondwang
Add RBCP for Canis

Revision 1.2  2012/03/28 00:38:23  mcharon
remove forward slash from second line

Revision 1.1.1.1  2012/03/23 23:02:09  ptong
Initial archive of ng_diag module


$Endlog$
*/
