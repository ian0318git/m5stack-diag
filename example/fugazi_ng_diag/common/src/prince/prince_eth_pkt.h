/* $Id: prince_eth_pkt.h,v 1.1 2013/07/31 09:20:49 xiaoyizh Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/prince/prince_eth_pkt.h,v $
 *------------------------------------------------------------------
 * Header file for linux base ethernet port tests
 * 
 * Xiaoying Zhang -- Jul. 2013
 *
 * Copyright (c) 2012-2013 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#ifndef __PRINCE_ETH_PKT_H__
#define __PRINCE_ETH_PKT_H__

#define SPD_10MBPS          10
#define SPD_100MBPS         100
#define SPD_1000MBPS        1000

#define SEL_PORT_ETH        "eth"

#define TX_RX_SYNC_TIME     20

typedef struct {
    char name[10];          /* name of eth*/
    int speed;              /* test speed */
    int pkt_num;            /* packet number */
    int pkt_len;            /* packet length */
    boolean signal;         /* test signal */
    ushort type;            /* to avoid set env everytime */
    int priority;           /* tx dring priority */
    int socket;
} diag_info_pthread_t;

typedef struct {
    int  reg_page;          /* page of register */
    int  reg_off;           /* offset of register */
    uint16_t  val;          /* value to set */
    uint16_t  mask;         /* mask of register r/w capability */
} mrvl_phy_setup_t;

/* Priorities for different Drings */
#define PRIORITY_TX0        7
#define PRIORITY_TX1        0
#define PRIORITY_TX2        9

#endif /* __PRINCE_ETH_PKT_H__ */

/*
$Log: prince_eth_pkt.h,v $
Revision 1.1  2013/07/31 09:20:49  xiaoyizh
Initial check in for ethernet packet functions using raw socket.


$Endlog$
*/
