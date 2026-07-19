/* $Id: eth_pkt_utils.h,v 1.2 2021/06/02 08:22:35 iachang Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/fugazi/eth_pkt_utils.h,v $
 *------------------------------------------------------------------
 *
 * eth_pkt_utils.h - Common ethernet packet utils defines
 *
 * Jan 2012, Paul Tong
 *
 * Copyright (c) 2019-2020 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#ifndef _ETH_PKT_UTILS_H_
#define _ETH_PKT_UTILS_H_

#include "router_if.h"

extern unsigned int crc32(unsigned int crc, unsigned char *data, int len);
extern void display_pkt(unsigned char *b_ptr, int pktlen);
extern int macstr2macaddr(uchar *macstr, mac_addr_t *mac_buf);
extern void mac_addr_query(char *query_str, mac_addr_t *mac_addr);

#endif  /* _ETH_PKT_UTILS_H_ */


/*-------------------------------------------------
 * $Log: eth_pkt_utils.h,v $
 * Revision 1.2  2021/06/02 08:22:35  iachang
 * CSCvo59196-33 : Merge Fugazi from ASR1K-main-branch to ISR main trunk
 *
 * Revision 1.1.8.2  2020/08/26 02:37:49  iachang
 * Merge Fugazi code into main trunk
 *
 * Revision 1.1.6.3  2020/07/31 09:52:09  iachang
 * Code clean up.
 *
 * Revision 1.1.6.2  2019/03/14 03:48:26  letsai
 * Initial check in.
 *
 *
 *
 * $Endlog$
 * */
