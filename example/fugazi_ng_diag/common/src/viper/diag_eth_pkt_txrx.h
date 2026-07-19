 /* $Id: diag_eth_pkt_txrx.h,v 1.2 2018/08/06 02:31:50 harrchan Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/viper/diag_eth_pkt_txrx.h,v $
 *-----------------------------------------------------------------------------
 * File: diag_eth_pkt_txrx.h
 * Description:
 *       Transfer packet to specific eth port.
 * Aug 2015, Alan Peng
 * Copyright (c) 2018 by Cisco Systems, Inc.
 * All rights reserved.
 *-----------------------------------------------------------------------------
 */
#ifndef __ETH_PKT_TXRX_H__
#define __ETH_PKT_TXRX_H__

#include "router_if.h"

#define ETH1                          "eth1"
#define ETH1_MAC0                     "eth1_mac0"
#define ETH1_MAC1                     "eth1_mac1"
#define ETH1_MAC2                     "eth1_mac2"

int pkt_deb_flag;

typedef struct {
    mac_addr_t  dest_addr;
    mac_addr_t  src_addr;
    unsigned short      pkt_type;
    unsigned short      tx_status;
    unsigned short      payload_size;
    unsigned char       *bufr_st_addr;
    unsigned int        pkt_num;
    int         socket;
} eth_tx_pkt_t;

typedef struct {
    unsigned short      rx_status;
    unsigned short      pkt_size;
    unsigned char       *bufr_st_addr;
    unsigned short      rx_bufr_size;
    unsigned int        pkt_num;
    unsigned int        wait_time;
    int         socket;
    int         rx_chk;
} eth_rx_pkt_t;

#define FALSE     0
#define TRUE      1

/* Ethernet packet macrocs */
#define PKT_BUF_LEN   10240 + 60  /* 60 extra bytes, based on xaui packet buf length*/
#define ETH_MIN_LEN	   64
#define ETH_PKT_BUF_LEN	   1600
#define ETH_HDR_LEN             14 // MAC address + type
#define ETH_PKT_CRC_LEN         4
#define ETH_PKT_LEN_MAX         1518
#define ETH_L2_PKT_LEN(payload_len) (ETH_HDR_LEN + (payload_len) + ETH_PKT_CRC_LEN);
#define PKT_PAYLOAD_SIZE   60

/* ethernet transmit defines */
#define ETH_PKT_TX_OK           0x00000000
#define ETH_PKT_TX_ERR          0x00000001
#define ETH_TX_ERR              0x00000002
#define ETH_NO_PKT_TX           0x00000004
#define ETH_TX_BD_ERR           0x00000008

/* ethernet receive defines */
#define ETH_PKT_RX_OK           0x00000000
#define ETH_PKT_RX_ERR          0x00000001
#define ETH_RX_ERR              0x00000002
#define ETH_NO_PKT_RX           0x00000004
#define ETH_RX_BD_ERR           0x00000008
#define ETH_RX_BUFR_OVFL        0x00000010
#define RX_PKT_WAIT_TIME        3000000  /* in usec */


extern int eth_pkt_txrx(char *, int, int);
extern int eth_pkt_rx(eth_rx_pkt_t *);
extern int eth_pkt_tx(eth_tx_pkt_t *);
extern void system_mac_addr_get(char *name, mac_addr_t *mac_buf);

#endif /* __ETH_PKT_TXRX_H__ */

/*---------------------------------------------------------------
$Log: diag_eth_pkt_txrx.h,v $
Revision 1.2  2018/08/06 02:31:50  harrchan
Merge viper E2E to the main trunk (CSCvk28469)

Revision 1.1.2.1  2018/02/27 08:06:40  harrchan
Initial viper application code base

$Endlog$
*/

