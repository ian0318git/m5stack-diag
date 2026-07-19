/* $Id: diag_eth_pkt_txrx_utils.h,v 1.2 2021/04/15 00:52:24 achiu2 Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/phoenix/diag_eth_pkt_txrx_utils.h,v $
 *------------------------------------------------------------------
 * File: diag_eth_pkt_txrx_utils.h
 * Description: contain functions are related to utility 
 * Copyright (c) 2018-2019 by Cisco Systems, Inc.
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
