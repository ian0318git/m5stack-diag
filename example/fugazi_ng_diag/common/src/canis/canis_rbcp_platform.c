/* $Id: canis_rbcp_platform.c,v 1.8 2013/11/11 21:18:39 mcharon Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/canis/canis_rbcp_platform.c,v $
 *------------------------------------------------------------------
 * Filename: canis_rbcp_platform.c
 *
 * Description: Contains the platform dependent code
 * Author: Times Huang
 *
 * Copyright (c) 2013 by cisco Systems, Inc.
 * All rights reserved.
 *------------------------------------------------------------------
 */

#include "common.h"
#include "types.h"
#include "proto.h"
#include "strings.h"
#include "cpu.h"
#include "error.h"
#include "nvsysvars.h"
#include "dev_object.h"
#include "canis_rbcp_lib.h"
#include "canis_rbcp_platform.h"
#include "router_if.h"
#include "sgmii_defs.h"

#include <string.h>
#include <unistd.h>

extern int get_host_mac_addr(uint, unsigned char *);
extern int rbcp_eth_pkt_rx(eth_rx_pkt_t *);

/***********************************************************************
 *  Macro Definitions
 ************************************************************************/

/***********************************************************************
 *  Static Functions Declaration
 ************************************************************************/
static int socket_gl;

/***********************************************************************
 *  Functions Declaration
 ************************************************************************/

int canis_rbcp_send(uint8_t *, int);
int canis_rbcp_recv(uint8_t *, int *);
int canis_rbcp_clear_recv (void);

/***********************************************************************
 *  Externs
 ************************************************************************/

//extern dev_pse2_object_t dev_canis_pse2_object;

/***********************************************************************
 *  Global Variable
 ************************************************************************/


/***********************************************************************
 *  Functions
 ************************************************************************/

int canis_rbcp_send (uint8_t *buf, int length)
{

    eth_tx_pkt_t tx_pkt;
    eth_tx_pkt_t *tx_pkt_p = &tx_pkt;
    int ret_val = PASSED;

    memset((uchar *)tx_pkt_p, 0, sizeof(eth_tx_pkt_t));
    memcpy((uchar *)(tx_pkt_p->dest_addr), /*dest_mac_addr*/buf,
            sizeof(mac_addr_t));
    memcpy((uchar *)(tx_pkt_p->src_addr), /*source_mac_addr*/buf + ETH_ALEN,
            sizeof(mac_addr_t));

    tx_pkt_p->pkt_type = *(uint16 *)(buf + (2 * ETH_ALEN));
    tx_pkt_p->payload_size = length;
    tx_pkt_p->bufr_st_addr = buf + MAC_HEADER_LEN;
    tx_pkt_p->pkt_num = 0;
    tx_pkt_p->socket = socket_gl;
#ifdef DEBUG
    printf("\ntx_pkt_p->pkt_type = 0x%04x", tx_pkt_p->pkt_type);
    printf("\ntx_pkt_p->tx_status  = 0x%04x", tx_pkt_p->tx_status);
    printf("\ntx_pkt_p->payload_size = 0x%04x", tx_pkt_p->payload_size);
    printf("\ntx_pkt_p->bufr_st_addr = 0x%08x", tx_pkt_p->bufr_st_addr);
    dismem((uchar *)tx_pkt_p, 0x40, (ulong)tx_pkt_p, 1);
#endif

    if (diagflag_xram & D_SET_OPTIONS) {
        printf("\nTx data in canis_rbcp_send :\n");
        dismem((unsigned char *)(buf), 0x40, (unsigned long)(buf), 1);
    }

    ret_val = eth_pkt_tx(tx_pkt_p);

    if (ret_val != ETH_PKT_TX_OK ) {
        cterr('f', 0, "%s: Failed send command : ret = 0x%x, status = 0x%x",
              __FUNCTION__, ret_val, tx_pkt_p->tx_status);
        return (FAILED);
    }

    return (PASSED);
}


int canis_rbcp_recv (uint8_t *buf, int *length)
{
    int wait_count = 1000000;
    eth_rx_pkt_t rx_pkt;
    eth_rx_pkt_t * rx_pkt_p = &rx_pkt;
    int status;
    uchar recv_buffer[1600]; /* need to take largest packet (1518) */

    /* clear buffer before use */
    memset((uchar *)buf, 0, RBCP_MSG_BUF_SIZE);
    memset((uchar *)rx_pkt_p, 0, sizeof(eth_rx_pkt_t));

    /* setup rx stucture for receiving */
    rx_pkt_p->bufr_st_addr = recv_buffer;
    rx_pkt_p->rx_bufr_size = sizeof(recv_buffer);
    rx_pkt_p->pkt_num = 0;
    rx_pkt_p->wait_time = wait_count * 2;
    rx_pkt_p->socket = socket_gl;

    /* now wait for */
    status = rbcp_eth_pkt_rx(rx_pkt_p);

    if (status) {
        if (diagflag_xram & D_SET_OPTIONS) {
            printf("\nCanis RBCP receive packet fail. status = %d.\n",status);
        }
        return (FAILED); /* retry is provided by caller */
    };
    /* copy received to user buf */
    memcpy ((char *)buf, (uchar *)recv_buffer, sizeof(fe_packet_t));

    *length = sizeof(fe_packet_t);

    if (diagflag_xram & D_SET_OPTIONS) {
        printf("\nRx data :\n");
        dismem((unsigned char *)(buf), 0x60, (unsigned long)(buf), 1);
    }

    return (PASSED);
}
/***********************************************************************
 * Name: canis_rbcp_clear_recv
 *
 * Description:
 *      This test will clear all receive packet.
 *
 * Input: None
 *
 * Output: PASSED.
 *
 ***********************************************************************
 */
int canis_rbcp_clear_recv (void)
{
    int wait_count = 1000000;
    eth_rx_pkt_t rx_pkt;
    eth_rx_pkt_t * rx_pkt_p = &rx_pkt;
    int status,recflag,loopcount;
    uchar recv_buffer[1600]; /* need to take largest packet (1518) */

    recflag = ENABLE;
    loopcount = 0;

    while (recflag) {
        /* clear buffer before use */
        memset((uchar *)recv_buffer, 0, RBCP_MSG_BUF_SIZE);
        memset((uchar *)rx_pkt_p, 0, sizeof(eth_rx_pkt_t));

        /* setup rx stucture for receiving */
        rx_pkt_p->bufr_st_addr = recv_buffer;
        rx_pkt_p->rx_bufr_size = sizeof(recv_buffer);
        rx_pkt_p->pkt_num = 0;
        rx_pkt_p->wait_time = wait_count;
        rx_pkt_p->socket = socket_gl;

        /* now check receive packet */
        status = rbcp_eth_pkt_rx(rx_pkt_p);
        loopcount++;
        if (( status != PASSED) || (loopcount > CANIS_RBCP_PKT_RECV_CLEAN)) {
            recflag = DISABLE;
        } else if (diagflag_xram & D_SET_OPTIONS) {
            printf("\nclearing recv packet queue: loopcount=%d\n", loopcount);
            dismem((uchar *)(recv_buffer), 0x50, (ulong)(recv_buffer), 1);
        }
    }

    if (diagflag_xram & D_SET_OPTIONS) {
        printf("\nCanis RBCP clear receive packet. status = %d,"
               "loopcount = %d.\n",status, loopcount);
    }

    return (PASSED);
}

/***********************************************************************
 * Name: canis_setup_rbcp_ge_env
 *
 * Description:
 *      This test will set up GE operation environment.
 *
 * Input: iface - Patriot data structure info pointer
 *
 * Output: PASSED or FAILED.
 *
 ***********************************************************************
 */
int canis_setup_rbcp_ge_env(void)
{
    int sgmii_port = 0;
    int status = PASSED;
    char if_name[10];

    sgmii_port = get_sgmii_port_num(0, TYPE_SWITCH);

    if (sgmii_port == -1) {
        cterr('f', 0, "Setup: Failed to get sgmii port number.");
        return (FAILED);
    }
    sprintf(if_name, "eth%d", sgmii_port);
    status = setup_eth_dev(if_name, &socket_gl);
//#ifdef DEBUG
    printf("\nsocket_gl = 0x%02x\n", socket_gl);
//#endif
    if (status) {
        cterr('f', 0, "Setup: Failed, status = 0x%x", status);
        return (FAILED);
    }

    return (PASSED);
}

/***********************************************************************
 * Name: canis_cleanup_rbcp_ge_env
 *
 * Description:
 *      This test will clean up the GE operation environment.
 *
 * Input: iface - Patriot data structure info pointer
 *
 * Output: PASSED or FAILED.
 *
 ***********************************************************************
 */
int canis_cleanup_rbcp_ge_env(void)
{
    int sgmii_port = 0;
    int status = PASSED;
    char if_name[10];

    sgmii_port = get_sgmii_port_num(0, TYPE_SWITCH);

    if (sgmii_port == -1) {
        cterr('f', 0, "cleanup: Failed to get sgmii port number.");
        return (FAILED);
    }
    sprintf(if_name, "eth%d", sgmii_port);
    status = cleanup_eth_dev(if_name, socket_gl);

    if (status) {
        cterr('f', 0, "cleanup: Failed, status = 0x%x", status);
        return (FAILED);
    }
    close(socket_gl);
    return (PASSED);
}

/*
 **********************************************************************
 *
 *  Function: canis_wait_for_ge_packet
 *
 *  Description: Wait for Ethernet packets
 *
 *  Input: buf - received packet buffer
 *
 *  Returns: PASSED if successful;
 *           FAILED, otherwise
 *
 **********************************************************************
 */
int canis_wait_for_ge_packet(uchar *buf)
{
    int wait_count = 10000;
    eth_rx_pkt_t rx_pkt;
    eth_rx_pkt_t * rx_pkt_p = &rx_pkt;
    int status;
    uchar recv_buffer[1600]; /* need to take largest packet (1518) */

    /* clear buffer before use */
    memset((uchar *)buf, 0, sizeof(fe_packet_t));
    memset((uchar *)rx_pkt_p, 0, sizeof(eth_rx_pkt_t));

    /* setup rx stucture for receiving */
    rx_pkt_p->bufr_st_addr = recv_buffer;
    rx_pkt_p->rx_bufr_size = sizeof(recv_buffer);
    rx_pkt_p->pkt_num = 0;
    rx_pkt_p->wait_time = wait_count;
    rx_pkt_p->socket = socket_gl;
    rx_pkt_p->rx_chk = 1;
    /* now wait for */
    status = eth_pkt_rx(rx_pkt_p);

    if (status) {
        return (FAILED); /* retry is provided by caller */
    };
    /* copy received to user buf */
    memcpy ((char *)buf, (uchar *)recv_buffer, sizeof(fe_packet_t));

    if (diagflag_xram & D_SET_OPTIONS) {
        printf("\nRx data :\n");
        dismem((unsigned char *)(buf), 0x40,(unsigned long)(buf), 1);
    }

    return (PASSED);
}

/***********************************************************************
 *  Static Functions
 ************************************************************************/


/*------------------------------------------------------------------
 * $Log: canis_rbcp_platform.c,v $
 * Revision 1.8  2013/11/11 21:18:39  mcharon
 * pass string instead of number in first argum of host_send_packet ; add xaui support
 *
 * Revision 1.7  2012/12/20 06:24:16  hondwang
 * Fill matrix valuse. Print debug info and increase retry to six
 *
 * Revision 1.6  2012/06/27 09:39:46  hondwang
 * revise canis source code for proper indent
 *
 * Revision 1.5  2012/06/08 06:45:05  hondwang
 * Fix canis complier warning on O2 x86
 *
 * Revision 1.4  2012/05/25 03:17:55  hondwang
 * fix canis building warning
 *
 * Revision 1.3  2012/05/17 08:01:42  hondwang
 * Add RBCP receive packet clean function
 *
 * Revision 1.2  2012/04/24 08:30:56  hondwang
 * Add RBCP for Canis
 *
 * Revision 1.1  2012/03/29 18:46:42  ksabzwar
 * Initial check in into ng_diag
 *
 * Revision 1.1.4.2  2012/03/10 01:18:28  ksabzwar
 * First check-in for Canis user menu for Overloard platform
 *
 *
 *
 *------------------------------------------------------------------
 * $Endlog$
 *
 *------------------------------------------------------------------
 */

