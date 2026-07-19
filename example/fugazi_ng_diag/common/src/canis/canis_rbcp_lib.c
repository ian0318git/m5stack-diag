/* $Id: canis_rbcp_lib.c,v 1.12 2014/01/17 00:57:57 ptong Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/canis/canis_rbcp_lib.c,v $
 *------------------------------------------------------------------
 * Filename: canis_rbcp_lib.c
 *
 * Description: The RBCP Library function for Canis project
 * Author: Times Huang
 *
 * Copyright (c) 2014 by cisco Systems, Inc.
 * All rights reserved.
 *------------------------------------------------------------------
 */

#include "common.h"
#include "types.h"
#include "proto.h"
#include "strings.h"
#include "nvsysvars.h"
#include "error.h"
#include "canis_rbcp_lib.h"
#include "canis_rbcp_platform.h"

#include <stdio.h>
#include <string.h> /* for strncpy */
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <net/if.h>
#include <unistd.h>

/***********************************************************************
 *  Macro Definitions
 ************************************************************************/

/***********************************************************************
 *  Static Functions Declaration
 ************************************************************************/

static void canis_rbcp_add_snap_hdr(char *);
static int  canis_rbcp_recv_msgs(char *, ushort, int *);
static int  canis_rbcp_send_msgs(char *, int, uchar, ushort, int);

/***********************************************************************
 *  Functions Declaration
 ************************************************************************/
int canis_rbcp_register(void);
int canis_rbcp_ping(void);
int canis_rbcp_intel_power_off(void);
int canis_rbcp_intel_power_on(void);
int canis_handle_ind_rbcp_msg(char *, int);
int canis_rbcp_send_ack_msg(struct cisco_scp_hdr *, int);
int canis_is_rbcp_msg(char *);
int canis_get_mac(uchar);
inline void canis_rbcp_ntoh_scp_hdr(cisco_scp_hdr_t *);

/***********************************************************************
 *  Externs
 ************************************************************************/
extern int canis_test_slot;
extern int get_host_mac_addr(uint, unsigned char *);

/***********************************************************************
 *  Global Variable
 ************************************************************************/

static uint16 seq_num = 0;
static char snap_id[] = { 0xaa, 0xaa, 0x03, 0x00, 0x00, 0x0c, 0x01, 0x1d };
static char dst_mac[MAX_NUM_CANIS_SLOTS][7] = {{ 0xff }};
static char src_mac[] = { 0x00, 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff };
static uchar plt_slot_id;
static uchar bmc_slot_id;
static uchar plt_sap_id;
static uchar bmc_sap_id;

static char buf[RBCP_MSG_BUF_SIZE];

/***********************************************************************
 *  Functions
 ************************************************************************/
int canis_rbcp_register (void)
{
    int i, j, rc=0;
    struct cisco_scp_hdr *hdr = (struct cisco_scp_hdr *)
                                ((uchar *)(buf + HEADER_LEN_802 ));
    int pkt_len=0;

    memset(buf, 0, RBCP_MSG_BUF_SIZE);
    memset(dst_mac[canis_test_slot], 0xff, ETH_ALEN);

    /* O2 ethernet hdr will setup in canis_rbcp_add_snap_hdr function */

    /* set up scp hdr */
    hdr->dst_slot_id = SCP_DSLOT_ID;
    hdr->src_slot_id = SCP_SSLOT_ID;
    hdr->dst_sap = SCP_DST_SAP;
    hdr->src_sap = SCP_SRC_SAP;
    hdr->length = NTOHS(SCP_LEN);
    hdr->seq_num = NTOHS(SCP_START_SEQ_NUM);

    /* Clear not necessary packet first */
    canis_rbcp_clear_recv();

    /* Send RBCP registration request (opcode = 0x14) */
    if (canis_rbcp_send_msgs(buf, FALSE, RBCP_FLAG_REQ, CISCO_SCP_OP_REQ_REG,
                             pkt_len) == FAILED) {
        printf ("\n%s(): 0x14 req send failure\n",__FUNCTION__);
        return (RBCP_SEND_FAILURE | 0x10);
    }

    /* See if we receive the registration request pkt from BMC */
    for (j=0; j < CANIS_RBCP_RETRIES; j++) {
        rc = canis_rbcp_recv_msgs(buf, CISCO_SCP_OP_REG, &pkt_len);
        if (!rc) break;
    }
    if (rc) {
        cterr('f', 0, "Did not receive any Register opcode from BMC\n");
        return(FAILED);
    }

    /* save all the IDs during registration */
    //plt_slot_id = hdr->dst_slot_id;
    plt_slot_id = PLAT_SCP_SLOT_ID;
    bmc_slot_id = hdr->src_slot_id;
    plt_sap_id = hdr->dst_sap;
    bmc_sap_id = hdr->src_sap;

    /* Add our own length and sequence number */
    hdr->length = SCP_LEN;
    hdr->seq_num = SCP_START_SEQ_NUM;
    hdr->length = NTOHS(hdr->length);
    hdr->seq_num = NTOHS(hdr->seq_num);

    /* copy Src MAC to dst_mac */
    for (i=0;i < 6;i++) {
    	dst_mac[canis_test_slot][i] = *(buf + ETH_ALEN + i);
    }
    /* Send RBCP message */
    if (canis_rbcp_send_msgs(buf, TRUE, RBCP_FLAG_RESP, CISCO_SCP_OP_REG, 
                             pkt_len) == FAILED) {
        return (FAILED);
    }

    return(PASSED);
}

int canis_rbcp_ping (void)
{
    int rc=FAILED;
    int pkt_len=0;
    int counter=0;

    /* Clear not necessary packet first */
    canis_rbcp_clear_recv();

    /* Send message */
ping_retry:
    counter++;
    if (canis_rbcp_send_msgs(buf, TRUE, RBCP_FLAG_REQ, CISCO_SCP_OP_HEART_BEAT,
                             pkt_len) == FAILED) {
        return (FAILED);
    }


    /* Check for the Ack from BMC */

    memset(buf, 0, RBCP_MSG_BUF_SIZE);

    /* See if we receive the heartbeat response pkt from BMC */
    if ((rc=canis_rbcp_recv_msgs(buf, CISCO_SCP_OP_HEART_BEAT, &pkt_len))) {
    /* Try again */
        if (counter < CANIS_RBCP_RETRIES) {
            goto ping_retry;
        }
        cterr('f', 0, "Did not receive Heartbeat opcode from BMC after %d tries\n", counter);
    }

    return (rc);
}

int canis_rbcp_con_sw_bmc (void)
{
    int rc=FAILED;
    int pkt_len=0,msglen;
    ucse_uart_msg_t *msg;

    msglen = sizeof(struct ucse_uart_msg);
    msg = (struct ucse_uart_msg *)((uchar *)(buf + HEADER_LEN_802 ));

    /* Clean payload data */
    memset(msg, 0, msglen);

    msg->payload.ucse_uart_type = htonl(UCSE_UART_BMC);

    pkt_len = msglen + LLC_LEN_802 + SNAP_LEN_802 + CANIS_RBCP_PKT_PADDING;

    /* Clear not necessary packet first */
    canis_rbcp_clear_recv();

    /* Send message */
    if (canis_rbcp_send_msgs(buf, TRUE, RBCP_FLAG_REQ, CISCO_SCP_UART_CONSOLE_SW,
                             pkt_len) == FAILED) {
        return (FAILED);
    }

    /* Check for the Ack from BMC */
    memset(buf, 0, RBCP_MSG_BUF_SIZE);

    /* See if we receive the Intel pwr off response pkt from BMC */
    rc = canis_rbcp_recv_msgs(buf, CISCO_SCP_UART_CONSOLE_SW, &pkt_len);

    return (rc);
}

int canis_rbcp_con_sw_intel (void)
{
    int rc=FAILED;
    int pkt_len=0,msglen;
    ucse_uart_msg_t *msg;

    msglen = sizeof(struct ucse_uart_msg);
    msg = (struct ucse_uart_msg *)((uchar *)(buf + HEADER_LEN_802 ));

    /* Clean payload data */
    memset(msg, 0, msglen);

    msg->payload.ucse_uart_type = htonl(UCSE_UART_HOST);

    pkt_len = msglen + LLC_LEN_802 + SNAP_LEN_802 + CANIS_RBCP_PKT_PADDING;

    /* Clear not necessary packet first */
    canis_rbcp_clear_recv();

    /* Send message */
    if (canis_rbcp_send_msgs(buf, TRUE, RBCP_FLAG_REQ, CISCO_SCP_UART_CONSOLE_SW,
                             pkt_len) == FAILED) {
	    return (FAILED);
    }

    /* Check for the Ack from BMC */
    memset(buf, 0, RBCP_MSG_BUF_SIZE);

    /* See if we receive the Intel pwr off response pkt from BMC */
    rc=canis_rbcp_recv_msgs(buf, CISCO_SCP_UART_CONSOLE_SW, &pkt_len);

    return (rc);
}

int canis_rbcp_intel_power_off (void)
{
    int rc=FAILED;
    int pkt_len=0;
    int counter=0;

    /* Clear not necessary packet first */
    canis_rbcp_clear_recv();

    /* Send message */
power_off_retry:
    counter++;
    if (canis_rbcp_send_msgs(buf, TRUE, RBCP_FLAG_REQ, CISCO_SCP_INTEL_PWR_OFF,
                             pkt_len) == FAILED) {
        return (FAILED);
    }


    /* Check for the Ack from BMC */

    memset(buf, 0, RBCP_MSG_BUF_SIZE);

    /* See if we receive the Intel pwr off response pkt from BMC */
    if ((rc=canis_rbcp_recv_msgs(buf, CISCO_SCP_INTEL_PWR_OFF, &pkt_len))) {
        /* Try again */
        if (counter < CANIS_RBCP_RETRIES) goto power_off_retry;
    }
    return (rc);
}

int canis_rbcp_intel_power_on (void)
{
    int rc=FAILED;
    int pkt_len=0;
    int counter=0;

    /* Clear not necessary packet first */
    canis_rbcp_clear_recv();

    /* Send message */
power_on_retry:
        counter++;
    if (canis_rbcp_send_msgs(buf, TRUE, RBCP_FLAG_REQ, CISCO_SCP_INTEL_PWR_ON, 
                             pkt_len) == FAILED) {
        return (FAILED);
    }

    /* Check for the Ack from BMC */

    memset(buf, 0, RBCP_MSG_BUF_SIZE);

    /* See if we receive the Intel pwr on response pkt from BMC */
    if ((rc=canis_rbcp_recv_msgs(buf, CISCO_SCP_INTEL_PWR_ON, &pkt_len))) {
        /* Try again */
        if (counter < CANIS_RBCP_RETRIES) {
            goto power_on_retry;
        }
    }
    return (rc);
}


/***********************************************************************
 *  Static Functions
 ************************************************************************/

static int canis_rbcp_recv_msgs (char *buf, ushort opcode, int *len)
{
    struct cisco_scp_hdr *hdr;
    int ix=0;
    int msg_size;

    for (ix = 0; ix < CANIS_RBCP_PKT_RECV_TIMEOUT; ix++) {
        msg_size=0;
        /* See if we get any message */
        if (diagflag_xram & D_SET_OPTIONS) {
            printf("\n%s(): loop# %d\n",__FUNCTION__, ix);
        }
        if (canis_rbcp_recv((uchar *)buf, &msg_size) == PASSED) {
            if (diagflag_xram & D_SET_OPTIONS) {
                printf("\n\n%s(): Received a pkt\n", __FUNCTION__);
            }
            if (canis_is_rbcp_msg(buf) == TRUE) {
                if (diagflag_xram & D_SET_OPTIONS) {
                    printf("\n%s(): Received a RBCP pkt\n", __FUNCTION__);
                }
                msg_size = NTOHS(*(ushort *)(buf + (ETH_ALEN * 2)));
                hdr = (struct cisco_scp_hdr *)((uchar *)(buf +
                      HEADER_LEN_802 ));

                canis_rbcp_ntoh_scp_hdr(hdr);

                if (diagflag_xram & D_SET_OPTIONS) {
                    printf ("\n%s(): msg_size = %#4x",__FUNCTION__, msg_size);
                    printf ("\n%s(): hdr->opcode = %x",__FUNCTION__,
                            hdr->opcode);
                    printf ("\n%s(): hdr->flags = %x\n",__FUNCTION__,
                            hdr->flags);
                }
                if (hdr->opcode == opcode) {
                    *len = msg_size;
                    return(PASSED);
                }
            }
        }

        msleep(100);
    }

    return(FAILED);
}

static int canis_rbcp_send_msgs (char *buf, int id_flag, uchar req_resp, 
                                 ushort opcode, int len)
{
    struct cisco_scp_hdr *hdr;
    int hdrlen;
    uint16 *etherlen;
    int total_size;

    if (diagflag_xram & D_SET_OPTIONS) {
        printf("\n\n%s(): Now Send opcode=%04x to BMC\n", 
                __FUNCTION__, opcode);
    }

    hdrlen = sizeof(struct cisco_scp_hdr);
    hdr = (struct cisco_scp_hdr *)((uchar *)(buf + HEADER_LEN_802 ));

    /* Set Response flag bit */
    if (req_resp==RBCP_FLAG_RESP) {
        hdr->flags |= RBCP_FLAG_RESP;
    } else {
        hdr->flags &= ~RBCP_FLAG_RESP;
    }
    hdr->opcode = HTONS(opcode);

    if (id_flag) {
        hdr->src_slot_id = plt_slot_id;
        hdr->dst_slot_id = bmc_slot_id;
        hdr->src_sap = plt_sap_id;
        hdr->dst_sap = bmc_sap_id;
    }

    /* Assign sequence number if required */
    if (hdr->seq_num == 0) {
        hdr->seq_num = HTONS(seq_num++);
    }


    /* Append Snap Header */
    canis_rbcp_add_snap_hdr(buf + MAC_HEADER_LEN);

    /* Append MAC Header */
    memcpy(buf, dst_mac[canis_test_slot], ETH_ALEN);
    memcpy(buf + ETH_ALEN, src_mac, ETH_ALEN);

    /* Ethernet Len = LLC + SNAP + SCP */
    etherlen = (uint16 *)(buf + (2 * ETH_ALEN));
    if (len==0) {
        *etherlen = hdrlen + LLC_LEN_802 + SNAP_LEN_802 ;
    } else {
        *etherlen = len;
    }
    /* Totol size is all frame size, must big than 60 Bytes (not include CRC) */
    if (len==0) {
        total_size = LLC_LEN_802 + SNAP_LEN_802
                   + hdrlen + CANIS_RBCP_PKT_PADDING;
    } else {
        total_size = len;
    }
    if (diagflag_xram & D_SET_OPTIONS)
        printf ("\n%s(): total packet length = %d \n",__FUNCTION__,total_size);

    /* Send the message, to be implemented */
    return (canis_rbcp_send((uint8_t *)buf, total_size));
}


int canis_is_rbcp_msg (char *buf)
{
    char *ptr;
    int ix, snap_hdr_len;

    /* Sanity check */
    if (buf == NULL) {
        return (FALSE);
    }

    ptr = buf + MAC_HEADER_LEN;

    snap_hdr_len = sizeof(snap_id);

    for (ix = 0; ix < snap_hdr_len; ix++) {
        if (ptr[ix] != snap_id[ix]) {
            return (FALSE);
        }
    }

    return (TRUE);
}

inline void canis_rbcp_ntoh_scp_hdr (cisco_scp_hdr_t *hdr)
{
    hdr->length     = NTOHS(hdr->length);
    hdr->opcode     = NTOHS(hdr->opcode);
    hdr->seq_num    = NTOHS(hdr->seq_num);
    hdr->fv_length  = NTOHS(hdr->fv_length);
}

static void canis_rbcp_add_snap_hdr (char *buf_snap)
{
    /* Append Snap Header */
    memcpy(buf_snap, snap_id, SNAP_LEN_802 + LLC_LEN_802);
}

int canis_get_mac(uchar slot) {
    if (get_host_mac_addr(0, (uchar *)&src_mac[0])) {
        return(FAILED);
    }
    return 0;
}

/*------------------------------------------------------------------
 * $Log: canis_rbcp_lib.c,v $
 * Revision 1.12  2014/01/17 00:57:57  ptong
 * Use get_host_mac_addr() to get the host to GE switch SGMII port MAC address to avoid hard coded to eth1
 *
 * Revision 1.11  2013/11/26 08:40:34  hroni
 * fix compiler warning
 *
 * Revision 1.10  2012/12/20 06:24:16  hondwang
 * Fill matrix valuse. Print debug info and increase retry to six
 *
 * Revision 1.9  2012/10/11 07:28:20  hondwang
 * porting multi card insert issue fix from G2. CSCua22608
 *
 * Revision 1.8  2012/06/27 09:39:46  hondwang
 * revise canis source code for proper indent
 *
 * Revision 1.7  2012/06/22 09:09:58  hondwang
 * fix issue in canis_rbcp_register function
 *
 * Revision 1.6  2012/06/20 13:09:57  hondwang
 * remove duplicate cterr report
 *
 * Revision 1.5  2012/06/08 06:45:05  hondwang
 * Fix canis complier warning on O2 x86
 *
 * Revision 1.4  2012/05/17 14:31:16  hondwang
 * Add RBCP registration request function
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


