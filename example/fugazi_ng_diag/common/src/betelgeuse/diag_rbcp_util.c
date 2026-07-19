/* $Id: diag_rbcp_util.c,v 1.2 2019/01/10 06:36:23 wilbhuan Exp $ 
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/betelgeuse/diag_rbcp_util.c,v $
 *------------------------------------------------------------------
 * 
 * diag_rbcp_util.c
 *
 * Copyright (c) 2018 - 2019 by Cisco Systems, Inc.
 * All rights reserved.
 *
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
#include "diag_rbcp_lib.h"
#include "router_if.h"
#include "sgmii_defs.h"
#include "diag_rbcp_util.h"
#include "ethernet.h"
#include "diag_pkt_txrx_lib.h"

#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <sys/un.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <linux/types.h>
#include <linux/netlink.h>
#include <errno.h>
#include <unistd.h>

extern int get_host_mac_addr(uint, unsigned char *);

/***********************************************************************
 *  Macro Definitions
 ************************************************************************/

/***********************************************************************
 *  Static Functions Declaration
 ************************************************************************/
static int socket_gl;
static int      crc_table_inited;
static unsigned int crc_table[256];



/***********************************************************************
 *  Functions Declaration
 ************************************************************************/

int plat_rbcp_send(uint8_t *, int);
int plat_rbcp_recv(uint8_t *, int *);
int plat_rbcp_clear_recv(void);
int plat_setup_rbcp_ge_env(void);
int plat_cleanup_rbcp_ge_env(void);
int plat_wait_for_ge_packet(uchar *);
int rx_a_pkt(int, uchar *buf_p, int);
int tx_a_pkt(int, uchar *pkt, int);
int rbcp_eth_pkt_rx(eth_rx_pkt_t *rxpkt_p);
int eth_pkt_rx(eth_rx_pkt_t *);
unsigned int crc32(unsigned int, unsigned char *, int);



/* This MAC address array is for cavecreek sgmii 1-3
 * These values in the array will be replaced by the
 * real value read from the system
 */
mac_addr_t local_mac_addr[] = {
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, /* eth0 not used internally */
    {0x10, 0x10, 0x10, 0x10, 0x10, 0x10},
    {0x20, 0x20, 0x20, 0x20, 0x20, 0x20},
    {0x30, 0x30, 0x30, 0x30, 0x30, 0x30},
    {0x40, 0x40, 0x40, 0x40, 0x40, 0x40},
};

/***********************************************************************
 *  Functions
 ************************************************************************/

/***********************************************************************
 * Name: swap32
 *
 * Description: Swap bit
 *
 * Input: bit
 *
 * Output: swap result
 *
 ***********************************************************************
 */
unsigned int swap32(unsigned int ix)
{
    ix = (ix << 16) | (ix >> 16);

    return (ix & 0xff00ffff) >> 8 | (ix & 0xffff00ff) << 8;
}

/***********************************************************************
 * Name: crc32
 *
 * Description: Ethernet packet 4 byte CRC calculationg
 *
 * Input: unsigned int crc, unsigned char *data, int len
 *
 * Output: crc
 *
 ***********************************************************************
 */
unsigned int crc32(unsigned int crc, unsigned char *data, int len)
{
    int         ix;

    if (!crc_table_inited) {
    int     jx;
    unsigned int        accum;

    for (ix = 0; ix < 256; ix++) {
        accum = ix;

        for (jx = 0; jx < 8; jx++) {
        if (accum & 1) {
            accum = accum >> 1 ^ 0xedb88320UL;
        } else {
            accum = accum >> 1;
        }
        }

        crc_table[ix] = swap32(accum);
    }

    crc_table_inited = 1;
    }

    for (ix = 0; ix < len; ix++) {
    crc = crc << 8 ^ crc_table[crc >> 24 ^ data[ix]];
    }

    return crc;
}

/***********************************************************************
 * Name: plat_rbcp_send
 *
 * Description:
 *      Send rbcp packet
 *
 * Input: buf: send context
 *        length: packet length
 *
 * Output: PASSED.
 *
 ***********************************************************************
 */

int plat_rbcp_send (uint8_t *buf, int length)
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
        printf("\nTx data in plat_rbcp_send :\n");
        dismem((unsigned char *)(buf), 0x40, (unsigned long)(buf), 1);
    }

    ret_val = eth_pkt_tx(tx_pkt_p);

    if (ret_val != ETH_PKT_TX_OK ) {
        printf("%s:%d:Failed to send command : ret = 0x%x, status = 0x%x\n", 
               __FUNCTION__, __LINE__, ret_val, tx_pkt_p->tx_status);
        return (FAILED);
    }

    return (PASSED);
}

/***********************************************************************
 * Name: plat_rbcp_recv
 *
 * Description:
 *      receive rbcp  packet.
 *
 * Input: buf: send context
 *        length: packet length
 *
 * Output: PASSED.
 *
 ***********************************************************************
 */

int plat_rbcp_recv (uint8_t *buf, int *length)
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
            printf("\nRBCP receive packet fail. status = %d.\n",status);
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
 * Name: plat_rbcp_clear_recv
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
int plat_rbcp_clear_recv (void)
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
        if (( status != PASSED) || (loopcount > RBCP_PKT_RECV_CLEAN)) {
            recflag = DISABLE;
        } else if (diagflag_xram & D_SET_OPTIONS) {
            printf("\nclearing recv packet queue: loopcount=%d\n", loopcount);
            dismem((uchar *)(recv_buffer), 0x50, (ulong)(recv_buffer), 1);
        }
    }

    if (diagflag_xram & D_SET_OPTIONS) {
        printf("\nRBCP clear receive packet. status = %d,"
               "loopcount = %d.\n",status, loopcount);
    }

    return (PASSED);
}

/***********************************************************************
 * Name: plat_setup_rbcp_ge_env
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
int plat_setup_rbcp_ge_env (void)
{
    int sgmii_port = 0;
    int status = PASSED;

    sgmii_port = CPU_SGMII_PORT2;
    status = setup_eth_port(sgmii_port, &socket_gl);
    
    if (((NVRAM)->diagflag & D_VERBOSE)) {
         printf("\nsocket_gl = 0x%02x\n", socket_gl);
    }

    if (status) {
        printf("%s:%d:Failed to setup, status = 0x%x\n", 
               __FUNCTION__, __LINE__, status);
        return (FAILED);
    }

    return (PASSED);
}

/***********************************************************************
 * Name: plat_cleanup_rbcp_ge_env
 *
 * Description:
 *      This test will clean up the GE operation environment.
 *
 * Input: None
 *
 * Output: PASSED or FAILED.
 *
 ***********************************************************************
 */
int plat_cleanup_rbcp_ge_env (void)
{
    int sgmii_port = 0;
    int status = PASSED;
    char if_name[10];

    sgmii_port = CPU_SGMII_PORT2;

    sprintf(if_name, "eth%d", sgmii_port);
    status = cleanup_eth_dev(if_name, socket_gl);

    if (status) {
        printf("%s:%d:Failed to clean up, status = 0x%x\n", 
               __FUNCTION__, __LINE__, status);
        return (FAILED);
    }
    close(socket_gl);
    return (PASSED);
}

/*
 **********************************************************************
 *
 *  Function: plat_wait_for_ge_packet
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
int plat_wait_for_ge_packet (uchar *buf)
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
 *
 * Function:    rbcp_eth_pkt_rx
 *
 * Description: This function will check to see if an ethernet packet
 *              was received by the GEMAC, which is attached to the backplane
 *              GE switch.
 *              If a receive packets is received, then check for good
 *              receive status and, if it is good, copy the receive
 *              packet to the user supplied buffer.
 *              There is a check on the size of the user supplied
 *              buffer; if the size of the receive packet is larger
 *              than the size of the user supplied buffer, then a
 *              buffer overflow error will be flagged and the receive
 *              data will be truncated to fit the size of the user buffer.
 *              The receive status will be checked to see if there are
 *              any receive errors and is returned to the user in the
 *              eth_rx_pkt_t structure member, rx_status.
 *
 * Input:   eth_rx_pkt_t - pointer to structure holding rx packet info
 *          bufr_st_addr - buffer address to of where to put rx buffer
 *          rx_bufr_size - size of user rx buffer
 *          pkt_size     - 0
 *          pkt_num      - packet number, optional
 *          wait_time    - time to wait for rx packet, in usec
 *          socket  - Linux socket of the RX port
 *
 * Output:  PASSED  if a packet is received without errors
 *          FAILED if a packet is received with errors
 *          rx_pkt_p->pkt_size contains the size of the rx packet
 *          rx_pkt_p->rx_status contains the receive buffer
 *          descriptor status word
 *
 ************************************************************************
 */
int rbcp_eth_pkt_rx (eth_rx_pkt_t *rxpkt_p)
{
    int rv = 0;
    struct timeval tv;
    uint wt_sec, wt_usec;

    memset(rxpkt_p->bufr_st_addr, 0, rxpkt_p->rx_bufr_size);

    /* Prepare for RX
     */
    rxpkt_p->pkt_size = 0;
    rxpkt_p->rx_status = 0;

    /* Get the second and microsecond portion of wait time
     * to set the socket time out.
     */
    wt_sec =  rxpkt_p->wait_time / 1000000;
    wt_usec = rxpkt_p->wait_time % 1000000;
    memset(&tv, 0, sizeof(tv));
    tv.tv_sec = wt_sec;   /* sec portion of wait time */
    tv.tv_usec = wt_usec;  /* microsec portion of wait time */
    if (setsockopt(rxpkt_p->socket, SOL_SOCKET, SO_RCVTIMEO,(struct timeval *)&tv,
                   sizeof(struct timeval))==-1) {
        perror("can't set receive time out");
    return (ETH_NO_PKT_RX);
    }

    /* rv containe -1 or the number of bytes received
     */
    rv = rx_a_pkt(rxpkt_p->socket,
             rxpkt_p->bufr_st_addr,
             rxpkt_p->rx_bufr_size);

    if (rv < 0) {
#if DEBUG
        printf("%s receive socket timeout.\n", __FUNCTION__);
    perror("eth_pkt_rx perror value:");
#endif
    return (ETH_NO_PKT_RX);
    }

    if (rv > ETH_MAX_LEN) {
#if DEBUG
        printf("%s rx packet size error.\n", __FUNCTION__);
    perror("eth_pkt_rx perror value:");
#endif
        return (ETH_PKT_RX_ERR);
    }

    /* The called of eth_pkt_rx should know about the CRC.
     * Subtract 4 byte from the size
     */
    rxpkt_p->pkt_size = rv - ETH_CRC_LEN;
    rxpkt_p->pkt_num += 1;

#if DEBUG
    printf("%s RX packet %d is:\n", __FUNCTION__, rxpkt_p->pkt_num);
    display_pkt(rxpkt_p->bufr_st_addr, rxpkt_p->pkt_size);
#endif

    return (ETH_PKT_RX_OK);
}



/*******************************************************************
 *
 * Function:    cleanup_eth_dev
 *
 * Description: Cleanup the Linux ethernet packet socket to prevent
 * either TX and RX.
 *
 * Input:   sgmii_port - sgmii port number to disable
 *              socket - The socket created in setup_eth_dev.
 *
 * Output:  PASS/FAIL
 *
 ************************************************************************
 */
int cleanup_eth_dev (char *if_name, int socket)
{

    if (close(socket) == -1) {
        if ((NVRAM)->diagflag & D_VERBOSE) {
            printf("%s:%d: Failed to clean up eth device on interface:%s\n", 
                   __FUNCTION__, __LINE__, if_name);
        }
        return (FAIL);
    }

    return (PASS);
}


/***********************************************************************
 * Function: tx_a_pkt
 *   Transmit a packet through the socket. The size is limited to 1514 bytes.
 *
 * Input:
 *   socket - socket ID
 *   pkt - the packet string
 *   pkt_len - The length of the packet
 *
 * Return: bytes being sent or -1 for error
 ***********************************************************************
 */
int tx_a_pkt (int socket, uchar *pkt, int pkt_len)
{
    int rv;

#if DEBUG
    printf("%s The tx packet is:\n", __FUNCTION__);
    display_pkt(pkt, pkt_len);
#endif

    /* Note: the socket buffer is limited to 1514 bytes
     */

    rv = write(socket, pkt, pkt_len);

    if (rv != pkt_len) {
        printf("\nsending %d bytes\n", pkt_len);
        perror("write command failed.");
    }
    return (rv);
}


/***********************************************************************
 * Function: rx_a_pkt
 *   Receive a packet through the socket.
 *
 * Input:
 *   socket - socket -ID
 *   buf_p - receive buffer
 *   buf_size - receive buffer size
 *
 * Return: number of bytes being read or -1 for error
 ***********************************************************************
 */
int rx_a_pkt (int socket, uchar *buf_p, int buf_size)
{
    int rv;

    rv = read(socket, buf_p, buf_size);
#if DEBUG
    if (rv < 0) {
        perror("read command failed.");
    }
#endif
    return (rv);
}

/***********************************************************************
 *
 * Function:    eth_pkt_tx
 *
 * Description: This function will setup and transmit an ethernet packet
 *              from CPU which is attached to the backplane GE switch.
 *              Then check for no transmission errors .
 *
 * Input:   eth_tx_pkt_t - pointer to structure holding tx packet info
 *          dest_addr    - destination MAC address
 *          src_addr     - source MAC address.  If the contents
 *                         equal zero, then use the host MAC address.
 *          pkt_type     - ethernet packet type field
 *          bufr_st_addr - buffer address to user tx buffer
 *          payload_size - size of tx data
 *          socket  - Linux socket ID of the TX port
 *
 * Output:  0    if the packet is transmitted without errors
 *          != 0 if the packet is transmitted with errors
 *
 ************************************************************************
 */
int eth_pkt_tx (eth_tx_pkt_t *tx_pkt_p)
{
    uchar *cptr, *buf_p;
    uchar pkt[XAUI_PKT_BUF_LEN];
    int pkt_len;
    uint crc;

    memset(pkt, 0, XAUI_PKT_BUF_LEN);
    cptr = pkt;
    buf_p = tx_pkt_p->bufr_st_addr;

    /* Build the packet
     */
    memcpy(cptr, tx_pkt_p->dest_addr, 6);
    cptr += 6;
    memcpy(cptr, tx_pkt_p->src_addr, 6);
    cptr += 6;
    *cptr++ = (tx_pkt_p->pkt_type >> 8) & 0xff;
    *cptr++ = tx_pkt_p->pkt_type & 0xff;


    /* Add the payload after the header
     */
    memcpy(cptr, buf_p, tx_pkt_p->payload_size);
    cptr += tx_pkt_p->payload_size;
    pkt_len = ETH_HDR_LEN + tx_pkt_p->payload_size;

    /* Add crc after the payload
     */
    crc = ~crc32(~0, pkt, pkt_len);
    *cptr++ = (crc >> 24) & 0xff;
    *cptr++ = (crc >> 16) & 0xff;
    *cptr++ = (crc >> 8) & 0xff;
    *cptr++ = crc & 0xff;
    pkt_len += ETH_CRC_LEN;

    if (tx_a_pkt(tx_pkt_p->socket, pkt, pkt_len) != pkt_len) {
        printf("%s Error sending packet on socket %d; length = %d",
               __FUNCTION__, tx_pkt_p->socket, pkt_len);
    return (ETH_PKT_TX_ERR);
    }

    return (ETH_PKT_TX_OK);
}


/***********************************************************************
 *
 * Function:    eth_pkt_rx
 *
 * Description: This function will check to see if an ethernet packet
 *              was received by the GEMAC, which is attached to the backplane
 *              GE switch.
 *              If a receive packets is received, then check for good
 *              receive status and, if it is good, copy the receive
 *              packet to the user supplied buffer.
 *              There is a check on the size of the user supplied
 *              buffer; if the size of the receive packet is larger
 *              than the size of the user supplied buffer, then a
 *              buffer overflow error will be flagged and the receive
 *              data will be truncated to fit the size of the user buffer.
 *              The receive status will be checked to see if there are
 *              any receive errors and is returned to the user in the
 *              eth_rx_pkt_t structure member, rx_status.
 *
 * Input:   eth_rx_pkt_t - pointer to structure holding rx packet info
 *          bufr_st_addr - buffer address to of where to put rx buffer
 *          rx_bufr_size - size of user rx buffer
 *          pkt_size     - 0
 *          pkt_num      - packet number, optional
 *          wait_time    - time to wait for rx packet, in usec
 *          socket  - Linux socket of the RX port
 *
 * Output:  PASSED  if a packet is received without errors
 *          FAILED if a packet is received with errors
 *          rx_pkt_p->pkt_size contains the size of the rx packet
 *          rx_pkt_p->rx_status contains the receive buffer
 *          descriptor status word
 *
 ************************************************************************
 */
int eth_pkt_rx (eth_rx_pkt_t *rxpkt_p)
{
    int rv = 0;
    struct timeval tv;
    uint wt_sec, wt_usec;

    memset(rxpkt_p->bufr_st_addr, 0, rxpkt_p->rx_bufr_size);

    /* Prepare for RX
     */
    rxpkt_p->pkt_size = 0;
    rxpkt_p->rx_status = 0;

    /* Get the second and microsecond portion of wait time
     * to set the socket time out.
     */
    wt_sec =  rxpkt_p->wait_time / 1000000;
    wt_usec = rxpkt_p->wait_time % 1000000;
    memset(&tv, 0, sizeof(tv));
    tv.tv_sec = wt_sec;   /* sec portion of wait time */
    tv.tv_usec = wt_usec;  /* microsec portion of wait time */
    if (setsockopt(rxpkt_p->socket, SOL_SOCKET, SO_RCVTIMEO,(struct timeval *)&tv,
                   sizeof(struct timeval))==-1) {
        perror("can't set receive time out");
    return (ETH_NO_PKT_RX);
    }

    /* rv containe -1 or the number of bytes received
     */
    rv = rx_a_pkt(rxpkt_p->socket,
             rxpkt_p->bufr_st_addr,
             rxpkt_p->rx_bufr_size);

    if (rv < 0) {
#if DEBUG
        printf("%s receive socket timeout.\n", __FUNCTION__);
    perror("eth_pkt_rx perror value:");
#endif
    return (ETH_NO_PKT_RX);
    }

    if (rxpkt_p->rx_chk) {
        /* The called of eth_pkt_rx shoule know about the CRC.
         * Subtract 4 byte from the size
         */
        rxpkt_p->pkt_size = rv - ETH_CRC_LEN;

        if ((rv <= (ETH_HDR_LEN + ETH_CRC_LEN)) || (rv > ETH_MAX_LEN)) {
            printf("%s received under size packet of %d bytes\n", __FUNCTION__, rv);
            perror("eth_pkt_rx perror value:");
            return (ETH_PKT_RX_ERR);
        }
    } else {
        rxpkt_p->pkt_size = rv;
    }


    rxpkt_p->pkt_num += 1;

#if DEBUG
    printf("%s RX packet %d is:\n", __FUNCTION__, rxpkt_p->pkt_num);
    display_pkt(rxpkt_p->bufr_st_addr, rxpkt_p->pkt_size);
#endif

    return (ETH_PKT_RX_OK);
}

/***********************************************************************
 * Function: get_local_mac_addr
 *   Return the eth port mac addrss stored in local_mac_addr to the caller
 *
 * Input:
 *   port - sgmii port numner
 *   mac_buf - pointer to the buffer for output value
 *
 * Return: void
 ************************************************************************
 */
void get_local_mac_addr (int port, mac_addr_t *mac_buf)
{
    memcpy(mac_buf, &local_mac_addr[port], sizeof(mac_addr_t));
}

/***********************************************************************
 * Name:    get_host_mac_addr
 *
 * Description: get host mac addr
 *
 * Input:   port number, user array to store the address
 *
 * Output:  failed to get the number -1;
 *      OK : 0.
 *
 ***********************************************************************
 */
int get_host_mac_addr (uint port, unsigned char *mac)
{
    int sgmii_port = 0;
    sgmii_port = CPU_SGMII_PORT2;
    get_local_mac_addr(sgmii_port, (mac_addr_t *)mac);
    return (0);
}

/*-------------------------------------------------
 * $Log: diag_rbcp_util.c,v $
 * Revision 1.2  2019/01/10 06:36:23  wilbhuan
 * The beginning of Betelgeuse application code.
 *
 *-------------------------------------------------
 */
