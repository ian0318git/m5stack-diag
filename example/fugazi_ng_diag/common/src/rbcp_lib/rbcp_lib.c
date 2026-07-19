/* $Id: rbcp_lib.c,v 1.5 2018/02/24 07:30:48 letsai Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/rbcp_lib/rbcp_lib.c,v $
 *------------------------------------------------------------------
 * Filename: rbcp_lib.c
 *
 * Description: The RBCP Library function
 * Author: Times Huang
 *
 * Copyright (c) 2015-2018 by cisco Systems, Inc.
 * All rights reserved.
 *------------------------------------------------------------------
 */

#include "common.h"
#include "types.h"
#include "proto.h"
#include "strings.h"
#include "nvsysvars.h"
#include "error.h"
#include "rbcp_platform.h"
#include "rbcp_lib.h"

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

static void rbcp_add_snap_hdr(char *);
int  rbcp_recv_msgs(char *, ushort, int *);
int  rbcp_send_msgs(char *, int, uchar, ushort, int);

/***********************************************************************
 *  Functions Declaration
 ************************************************************************/
int rbcp_register(void);
int rbcp_ping(void);
int rbcp_intel_power_off(void);
int rbcp_intel_power_on(void);
int handle_ind_rbcp_msg(char *, int);
int rbcp_send_ack_msg(struct cisco_scp_hdr *, int);
int is_rbcp_msg(char *);
int rbcp_con_sw_bmc(void);
int rbcp_con_sw_intel(void);
int rbcp_get_mac(uchar);
inline void rbcp_ntoh_scp_hdr(cisco_scp_hdr_t *);

/***********************************************************************
 *  Externs
 ************************************************************************/
extern int shedir_test_slot;
extern int get_host_mac_addr(uint, unsigned char *);
/***********************************************************************
 *  Global Variable
 ************************************************************************/

static uint16 seq_num = 0;
static char snap_id[] = { 0xaa, 0xaa, 0x03, 0x00, 0x00, 0x0c, 0x01, 0x1d };
static char dst_mac[MAX_NUM_SHEDIR_SLOTS][7];
static char src_mac[] = { 0x00, 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff };
static uchar plt_slot_id;
static uchar bmc_slot_id;
static uchar plt_sap_id;
static uchar bmc_sap_id;

static char buf[RBCP_MSG_BUF_SIZE];

/***********************************************************************
 *  Functions
 ************************************************************************/


/**********************************************************************
 *
 *Function:rbcp_register
 *
 *Description: Registration function between platform and module
 *
 *Inpunts: void 
 *
 *Outputs:PASS/FAILED
 **********************************************************************
 */

int rbcp_register (void)
{
    int i, j, rc=0;
    struct cisco_scp_hdr *hdr = (struct cisco_scp_hdr *)
                                ((uchar *)(buf + HEADER_LEN_802 ));
    int pkt_len=0;

    memset(buf, 0, RBCP_MSG_BUF_SIZE);
    memset(dst_mac[shedir_test_slot], 0xff, ETH_ALEN);

    /* O2 ethernet hdr will setup in rbcp_add_snap_hdr function */

    /* set up scp hdr */
    hdr->dst_slot_id = SCP_DSLOT_ID;
    hdr->src_slot_id = SCP_SSLOT_ID;
    hdr->dst_sap = SCP_DST_SAP;
    hdr->src_sap = SCP_SRC_SAP;
    hdr->length = NTOHS(SCP_LEN);
    hdr->seq_num = NTOHS(SCP_START_SEQ_NUM);

    /* Clear not necessary packet first */
    platform_rbcp_clear_recv();

    /* Send RBCP registration request (opcode = 0x14) */
    if (rbcp_send_msgs(buf, FALSE, RBCP_FLAG_REQ, CISCO_SCP_OP_REQ_REG,
                             pkt_len) == FAILED) {
        printf ("\n%s(): 0x14 req send failure\n",__FUNCTION__);
        return (RBCP_SEND_FAILURE | 0x10);
    }

    /* See if we receive the registration request pkt from BMC */
    for (j=0; j < RBCP_RETRIES; j++) {
        rc = rbcp_recv_msgs(buf, CISCO_SCP_OP_REG, &pkt_len);
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
    	dst_mac[shedir_test_slot][i] = *(buf + ETH_ALEN + i);
    }
    /* Send RBCP message */
    if (rbcp_send_msgs(buf, TRUE, RBCP_FLAG_RESP, CISCO_SCP_OP_REG, 
                             pkt_len) == FAILED) {
        return (FAILED);
    }

    return(PASSED);
}

/**********************************************************************
 *
 *Function:rbcp_ping
 *
 *Description: heartbeat test between platform and module
 *
 *Inpunts: void 
 *
 *Outputs:PASS/FAILED
 **********************************************************************
 */

int rbcp_ping (void)
{
    int rc=FAILED;
    int pkt_len=0;
    int counter=0;
    struct cisco_scp_hdr *hdr = (struct cisco_scp_hdr *)
                                ((uchar *)(buf + HEADER_LEN_802 ));

    hdr->length = NTOHS(SCP_LEN);

    /* Clear not necessary packet first */
    platform_rbcp_clear_recv();

    /* Send message */
ping_retry:
    counter++;
    if (rbcp_send_msgs(buf, TRUE, RBCP_FLAG_REQ, CISCO_SCP_OP_HEART_BEAT,
                             pkt_len) == FAILED) {
        return (FAILED);
    }


    /* Check for the Ack from BMC */

    memset(buf, 0, RBCP_MSG_BUF_SIZE);

    /* See if we receive the heartbeat response pkt from BMC */
    if ((rc=rbcp_recv_msgs(buf, CISCO_SCP_OP_HEART_BEAT, &pkt_len))) {
    /* Try again */
        if (counter < RBCP_RETRIES) {
            goto ping_retry;
        }
        cterr('f', 0, "Did not receive Heartbeat opcode from BMC after %d tries\n", counter);
    }

    return (rc);
}

/**********************************************************************
 *
 *Function:rbcp_con_sw_lsi
 *
 *Description: console switch to LSI (opcode 0x98)
 *
 *Inpunts: void 
 *
 *Outputs:PASS/FAILED
 **********************************************************************
 */

int rbcp_con_sw_lsi (void)
{
    int rc=FAILED;
    int pkt_len = 0, msglen;
    ucse_uart_msg_t *msg;

    msglen = sizeof(struct ucse_uart_msg);
    msg = (struct ucse_uart_msg *)((uchar *)(buf + HEADER_LEN_802 ));

    /* Clean payload data */
    memset(msg, 0, msglen);

    msg->payload.ucse_uart_type = htonl(UCSE_UART_LSI);

    pkt_len = msglen + LLC_LEN_802 + SNAP_LEN_802 + RBCP_PKT_PADDING;

    /* Clear not necessary packet first */
    platform_rbcp_clear_recv();

    /* Send message */
    if (rbcp_send_msgs(buf, TRUE, RBCP_FLAG_REQ, CISCO_SCP_UART_CONSOLE_SW,
                             pkt_len) == FAILED) {
        return (FAILED);
    }

    /* Check for the Ack from BMC */
    memset(buf, 0, RBCP_MSG_BUF_SIZE);

    /* See if we receive the Intel pwr off response pkt from BMC */
    rc = rbcp_recv_msgs(buf, CISCO_SCP_UART_CONSOLE_SW, &pkt_len);

    return (rc);
}

/**********************************************************************
 *
 *Function:rbcp_con_sw_bmc
 *
 *Description: console switch to BMC (opcode 0x98)
 *
 *Inpunts: void 
 *
 *Outputs:PASS/FAILED
 **********************************************************************
 */

int rbcp_con_sw_bmc (void)
{
    int rc=FAILED;
    int pkt_len=0,msglen;
    ucse_uart_msg_t *msg;

    msglen = sizeof(struct ucse_uart_msg);
    msg = (struct ucse_uart_msg *)((uchar *)(buf + HEADER_LEN_802 ));

    /* Clean payload data */
    memset(msg, 0, msglen);

    msg->payload.ucse_uart_type = htonl(UCSE_UART_BMC);

    pkt_len = msglen + LLC_LEN_802 + SNAP_LEN_802 + RBCP_PKT_PADDING;

    /* Clear not necessary packet first */
    platform_rbcp_clear_recv();

    /* Send message */
    if (rbcp_send_msgs(buf, TRUE, RBCP_FLAG_REQ, CISCO_SCP_UART_CONSOLE_SW,
                             pkt_len) == FAILED) {
        return (FAILED);
    }

    /* Check for the Ack from BMC */
    memset(buf, 0, RBCP_MSG_BUF_SIZE);

    /* See if we receive the Intel pwr off response pkt from BMC */
    rc = rbcp_recv_msgs(buf, CISCO_SCP_UART_CONSOLE_SW, &pkt_len);

    return (rc);
}

/**********************************************************************
 *
 *Function:rbcp_con_sw_intel
 *
 *Description: Console switch to intel (opcode =0x98)
 *
 *Inpunts: void 
 *
 *Outputs:PASS/FAILED
 **********************************************************************
 */

int rbcp_con_sw_intel (void)
{
    int rc=FAILED;
    int pkt_len=0,msglen;
    ucse_uart_msg_t *msg;

    msglen = sizeof(struct ucse_uart_msg);
    msg = (struct ucse_uart_msg *)((uchar *)(buf + HEADER_LEN_802 ));

    /* Clean payload data */
    memset(msg, 0, msglen);

    msg->payload.ucse_uart_type = htonl(UCSE_UART_HOST);

    pkt_len = msglen + LLC_LEN_802 + SNAP_LEN_802 + RBCP_PKT_PADDING;

    /* Clear not necessary packet first */
    platform_rbcp_clear_recv();

    /* Send message */
    if (rbcp_send_msgs(buf, TRUE, RBCP_FLAG_REQ, CISCO_SCP_UART_CONSOLE_SW,
                             pkt_len) == FAILED) {
	    return (FAILED);
    }

    /* Check for the Ack from BMC */
    memset(buf, 0, RBCP_MSG_BUF_SIZE);

    /* See if we receive the Intel pwr off response pkt from BMC */
    rc=rbcp_recv_msgs(buf, CISCO_SCP_UART_CONSOLE_SW, &pkt_len);

    return (rc);
}

/**********************************************************************
 *
 *Function:rbcp_intel_power_off
 *
 *Description: send opcode (0x92) to power down intel
 *
 *Inpunts: void 
 *
 *Outputs:PASS/FAILED
 **********************************************************************
 */

int rbcp_intel_power_off (void)
{
    int rc=FAILED;
    int pkt_len=0;
    int counter=0;

    /* Clear not necessary packet first */
    platform_rbcp_clear_recv();

    /* Send message */
power_off_retry:
    counter++;
    if (rbcp_send_msgs(buf, TRUE, RBCP_FLAG_REQ, CISCO_SCP_INTEL_PWR_OFF,
                             pkt_len) == FAILED) {
        return (FAILED);
    }


    /* Check for the Ack from BMC */

    memset(buf, 0, RBCP_MSG_BUF_SIZE);

    /* See if we receive the Intel pwr off response pkt from BMC */
    if ((rc=rbcp_recv_msgs(buf, CISCO_SCP_INTEL_PWR_OFF, &pkt_len))) {
        /* Try again */
        if (counter < RBCP_RETRIES) goto power_off_retry;
    }
    return (rc);
}
/**********************************************************************
 *
 *Function:rbcp_intel_power_on
 *
 *Description: send opcode (0x92) to power on intel
 *
 *Inpunts: void 
 *
 *Outputs:PASS/FAILED
 **********************************************************************
 */

int rbcp_intel_power_on (void)
{
    int rc=FAILED;
    int pkt_len=0;
    int counter=0;

    /* Clear not necessary packet first */
    platform_rbcp_clear_recv();

    /* Send message */
power_on_retry:
        counter++;
    if (rbcp_send_msgs(buf, TRUE, RBCP_FLAG_REQ, CISCO_SCP_INTEL_PWR_ON, 
                             pkt_len) == FAILED) {
        return (FAILED);
    }

    /* Check for the Ack from BMC */

    memset(buf, 0, RBCP_MSG_BUF_SIZE);

    /* See if we receive the Intel pwr on response pkt from BMC */
    if ((rc=rbcp_recv_msgs(buf, CISCO_SCP_INTEL_PWR_ON, &pkt_len))) {
        /* Try again */
        if (counter < RBCP_RETRIES) {
            goto power_on_retry;
        }
    }
    return (rc);
}


/***********************************************************************
 *  Static Functions
 ************************************************************************/

/**********************************************************************
 *
 *Function:rbcp_recv_msgs
 *
 *Description: Recevie opcode from BMC
 *
 *Inpunts: buf,opcode,len
 *
 *Outputs:PASS/FAILED
 **********************************************************************
 */

int rbcp_recv_msgs (char *buf, ushort opcode, int *len)
{
    struct cisco_scp_hdr *hdr;
    int ix=0;
    int msg_size;

    for (ix = 0; ix < RBCP_PKT_RECV_TIMEOUT; ix++) {
        msg_size=0;
        /* See if we get any message */
        if (diagflag_xram & D_SET_OPTIONS) {
            printf("\n%s(): loop# %d\n",__FUNCTION__, ix);
        }
        if (platform_rbcp_recv((uchar *)buf, &msg_size) == PASSED) {
            if (diagflag_xram & D_SET_OPTIONS) {
                printf("\n\n%s(): Received a pkt\n", __FUNCTION__);
            }
            if (is_rbcp_msg(buf) == TRUE) {
                if (diagflag_xram & D_SET_OPTIONS) {
                    printf("\n%s(): Received a RBCP pkt\n", __FUNCTION__);
                }
                msg_size = NTOHS(*(ushort *)(buf + (ETH_ALEN * 2)));
                hdr = (struct cisco_scp_hdr *)((uchar *)(buf +
                      HEADER_LEN_802 ));

                rbcp_ntoh_scp_hdr(hdr);

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

/**********************************************************************
 *
 *Function:rbcp_send_msgs
 *
 *Description: Send rbcp opcode to BMC
 *
 *Inpunts: buf,
 *         id_flag:Determine whether it have done registration test or not
 *         req_resp: Determine whether it is Request or Response
 *         opcode
 *         len
 *
 *Outputs:rbcp_send(buf,total_size)
 **********************************************************************
 */

int rbcp_send_msgs (char *buf, int id_flag, uchar req_resp, 
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
    rbcp_add_snap_hdr(buf + MAC_HEADER_LEN);

    /* Append MAC Header */
    memcpy(buf, dst_mac[shedir_test_slot], ETH_ALEN);
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
                   + hdrlen + RBCP_PKT_PADDING;
    } else {
        total_size = len;
    }
    if (diagflag_xram & D_SET_OPTIONS)
        printf ("\n%s(): total packet length = %d \n",__FUNCTION__,total_size);

    /* Send the message, to be implemented */
    return (platform_rbcp_send((uint8_t *)buf, total_size));
}

/**********************************************************************
 *
 *Function:shedir_is_rbcp_msg
 *
 *Description: According to header to determine whether this packet is rbcp packet or not
 *
 *Inpunts: buf
 *
 *Outputs:TRUE/FALSE
 **********************************************************************
 */

int is_rbcp_msg (char *buf)
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

inline void rbcp_ntoh_scp_hdr (cisco_scp_hdr_t *hdr)
{
    hdr->length     = NTOHS(hdr->length);
    hdr->opcode     = NTOHS(hdr->opcode);
    hdr->seq_num    = NTOHS(hdr->seq_num);
    hdr->fv_length  = NTOHS(hdr->fv_length);
}
/**********************************************************************
 *
 *Function:rbcp_add_snap_hdr
 *
 *Description: Add rbcp snap header
 *
 *Inpunts: buf_snap
 *
 *Outputs:void
 **********************************************************************
 */

static void rbcp_add_snap_hdr (char *buf_snap)
{
    /* Append Snap Header */
    memcpy(buf_snap, snap_id, SNAP_LEN_802 + LLC_LEN_802);
}

/**********************************************************************
 *
 *Function:rbcp_get_mac
 *
 *Description: According to platform(overlord/USD) to get responding MAC address
 *
 *Inpunts: slot
 *
 *Outputs:PASS/FAILED
 **********************************************************************
 */

int rbcp_get_mac(uchar slot) {
    if (get_host_mac_addr(0, (uchar *)&src_mac[0])) {
        return(FAILED);
    }
    return PASSED;
}

/*------------------------------------------------------------------
 * $Log: rbcp_lib.c,v $
 * Revision 1.5  2018/02/24 07:30:48  letsai
 * Collapse Kalamata-branch to Main Trunk.
 *
 * Revision 1.4  2017/03/21 08:41:57  olin2
 * Collapse Aquila-branch to Main Trunk.
 *
 * Revision 1.3  2016/01/21 01:50:04  olin2
 * Collapse Draco-branch to Main Trunk.
 *
 * Revision 1.2  2015/05/25 03:58:00  steja
 * Fix merge conflict issue
 *
 * Revision 1.1.2.2  2015/05/22 15:42:27  steja
 * Sync skye-branch2 with Maintrunk
 *
 * Revision 1.1  2015/05/14 03:29:31  hondwang
 * add rbcp_lib
 *
 *
 *
 *
 *
 *------------------------------------------------------------------
 * $Endlog$
 *
 *------------------------------------------------------------------
 */


