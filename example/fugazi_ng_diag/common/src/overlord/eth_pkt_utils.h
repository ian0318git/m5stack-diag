/* $Id: eth_pkt_utils.h,v 1.3 2013/11/26 08:40:35 hroni Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/overlord/eth_pkt_utils.h,v $
 *------------------------------------------------------------------
 *
 * eth_pkt_utils.h - Common ethernet packet utils defines
 *
 * Jan 2012, Paul Tong
 *
 * Copyright (c) 2013 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#ifndef _ETH_PKT_UTILS_H_
#define _ETH_PKT_UTILS_H_

extern unsigned int crc32(unsigned int crc, unsigned char *data, int len);
extern void display_pkt(unsigned char *b_ptr, int pktlen);
extern int macstr2macaddr(uchar *macstr, mac_addr_t *mac_buf);
extern void mac_addr_query(char *query_str, mac_addr_t *mac_addr);

#endif  /* _ETH_PKT_UTILS_H_ */


/******** History ******** 
$Log: eth_pkt_utils.h,v $
Revision 1.3  2013/11/26 08:40:35  hroni
fix compiler warning

Revision 1.2  2012/03/28 00:38:17  mcharon
remove forward slash from second line

Revision 1.1.1.1  2012/03/23 23:02:06  ptong
Initial archive of ng_diag module


$Endlog$
*/
