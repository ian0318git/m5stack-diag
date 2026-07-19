 /* $Id: diag_eth_pkt_txrx_utils.h,v 1.2 2018/08/06 02:31:50 harrchan Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/viper/diag_eth_pkt_txrx_utils.h,v $
 *-----------------------------------------------------------------------------
 * File: diag_eth_pkt_txrx_utils.h
 * Description: contain functions are related to utility 
 * Aug 2015, Alan Peng
 * Copyright (c) 2018 by Cisco Systems, Inc.
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
Revision 1.2  2018/08/06 02:31:50  harrchan
Merge viper E2E to the main trunk (CSCvk28469)

Revision 1.1.2.1  2018/02/27 08:06:41  harrchan
Initial viper application code base

$Endlog$
*/

