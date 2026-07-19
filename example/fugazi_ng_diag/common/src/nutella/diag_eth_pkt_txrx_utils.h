/* $Id: diag_eth_pkt_txrx_utils.h,v 1.4 2019/07/11 12:31:27 alicehua Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/nutella/diag_eth_pkt_txrx_utils.h,v $
 *-----------------------------------------------------------------------------
 * File: diag_eth_pkt_txrx_utils.h
 * Description: contain functions are related to utility 
 * Aug 2015, Alan Peng
 * Copyright (c) 2019 by Cisco Systems, Inc.
 * All rights reserved.
 *-----------------------------------------------------------------------------
 */
#ifndef __ETH_PKT_TXRX_UTILS_H__
#define __ETH_PKT_TXRX_UTILS_H__

extern unsigned int swap32(unsigned int);
extern unsigned int crc32(unsigned int, unsigned char *, int);
extern void display_pkt(unsigned char *, int);
extern int chk_macaddr(unsigned char *, unsigned char *);
extern void eth_pkt_txrx_usage(void);
extern void print_debug_msg (const char * format, ...);

#endif 
/*---------------------------------------------------------------
$Log: diag_eth_pkt_txrx_utils.h,v $
Revision 1.4  2019/07/11 12:31:27  alicehua
Collapse Nutella codes into main trunk.

$Endlog$
*/
