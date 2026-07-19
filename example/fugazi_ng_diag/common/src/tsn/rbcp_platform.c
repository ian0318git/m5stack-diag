/* $Id: rbcp_platform.c,v 1.1 2018/05/09 06:53:12 letsai Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/tsn/rbcp_platform.c,v $
 *------------------------------------------------------------------
 * Filename: rbcp_platform.c
 *
 * Description: Contains the platform dependent code for RBCP function
 * Author: Times Huang
 *
 * Copyright (c) 2018 by cisco Systems, Inc.
 * All rights reserved.
 *------------------------------------------------------------------
 */

#include "common.h"
#include "types.h"
#include "proto.h"
#include "strings.h"
#include "cpu.h"
#include "error.h"
#include "nvmonvars.h"
#include "nvsysvars.h"
#include "dev_object.h"
#include "rbcp_lib.h"
#include "rbcp_platform.h"
#include "router_if.h"
#include "sgmii_defs.h"
#include "platform_ext_lpbk.h"

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

int platform_rbcp_send(uint8_t *, int);
int platform_rbcp_recv(uint8_t *, int *);
int platform_rbcp_clear_recv(void);
int platform_setup_rbcp_ge_env(void);
int platform_cleanup_rbcp_ge_env(void);
int platform_wait_for_ge_packet(uchar *);

/***********************************************************************
 *  Externs
 ************************************************************************/


/***********************************************************************
 *  Global Variable
 ************************************************************************/


/***********************************************************************
 *  Functions
 ************************************************************************/

/***********************************************************************
 * Name: platform_rbcp_send
 *
 * Description:
 *      Send rbcp packet
 *
 * Input: buf : send context
 *        length : packet length
 *
 * Output: PASSED.
 *
 ***********************************************************************
 */

int platform_rbcp_send (uint8_t *buf, int length)
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
    
    if (!((NVRAM)->diagflag & D_VERBOSE)) {
        printf("\ntx_pkt_p->pkt_type = 0x%04x", tx_pkt_p->pkt_type);
        printf("\ntx_pkt_p->tx_status  = 0x%04x", tx_pkt_p->tx_status);
        printf("\ntx_pkt_p->payload_size = 0x%04x", tx_pkt_p->payload_size);
        dismem((uchar *)tx_pkt_p, 0x40, (ulong)tx_pkt_p, 1);
    }

    if (diagflag_xram & D_SET_OPTIONS) {
        printf("\nTx data in gshdsl_rbcp_send :\n");
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

/***********************************************************************
 * Name: platform_rbcp_recv
 *
 * Description:
 *      receive rbcp  packet.
 *
 * Input: buf : send context
 *        length : packet length
 *
 * Output: PASSED.
 *
 ***********************************************************************
 */

int platform_rbcp_recv (uint8_t *buf, int *length)
{
    int wait_count = RBCP_WAIT_COUNT;
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
            printf("\nG.SHDSL RBCP receive packet fail. status = %d.\n",status);
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
 * Name: platform_rbcp_clear_recv
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
int platform_rbcp_clear_recv (void)
{
    int wait_count = RBCP_WAIT_COUNT;
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
        if (( status != PASSED) || (loopcount > RBCP_PKT_RECV_CLEAN)) {
            recflag = DISABLE;
        } else if (diagflag_xram & D_SET_OPTIONS) {
            printf("\nclearing recv packet queue: loopcount=%d\n", loopcount);
            dismem((uchar *)(recv_buffer), 0x50, (ulong)(recv_buffer), 1);
        }
    }

    if (diagflag_xram & D_SET_OPTIONS) {
        printf("\nG.SHDSL RBCP clear receive packet. status = %d,"
               "loopcount = %d.\n",status, loopcount);
    }

    return (PASSED);
}

/***********************************************************************
 * Name: platform_setup_rbcp_ge_env
 *
 * Description:
 *      This test will set up GE operation environment.
 *
 * Input: None
 *
 * Output: PASSED or FAILED.
 *
 ***********************************************************************
 */
int platform_setup_rbcp_ge_env (void)
{
    int sgmii_port = 0;
    int status = PASSED;
    char if_name[10];

    sgmii_port = CPU_SGMII_PORT2;

    sprintf(if_name, "eth%d", sgmii_port);
    status = setup_eth_port(sgmii_port, &socket_gl);
    
    if (((NVRAM)->diagflag & D_VERBOSE)) {
        printf("\nsocket_gl = 0x%02x\n", socket_gl); 
    }
    
    if (status) {
        cterr('f', 0, "Setup: Failed, status = 0x%x", status);
        return (FAILED);
    }

    return (PASSED);
}

/***********************************************************************
 * Name: platform_cleanup_rbcp_ge_env
 *
 * Description:
 *      This test will clean up the GE operation environment.
 *
 * Input: iface - void
 *
 * Output: PASSED or FAILED.
 *
 ***********************************************************************
 */
int platform_cleanup_rbcp_ge_env (void)
{
    int sgmii_port = 0;
    int status = PASSED;
    char if_name[10];

    sgmii_port = CPU_SGMII_PORT2;

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
 *  Function: platform_wait_for_ge_packet
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
int platform_wait_for_ge_packet (uchar *buf)
{
    int wait_count = RBCP_WAIT_COUNT;
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



/*------------------------------------------------------------------
 * $Log: rbcp_platform.c,v $
 * Revision 1.1  2018/05/09 06:53:12  letsai
 * Add TSN GSHDSL portion
 *
 *
 *------------------------------------------------------------------
 * $Endlog$
 *
 *------------------------------------------------------------------
 */
