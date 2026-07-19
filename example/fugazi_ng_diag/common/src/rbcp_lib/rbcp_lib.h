/* $Id: rbcp_lib.h,v 1.4 2018/02/24 07:30:48 letsai Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/rbcp_lib/rbcp_lib.h,v $
 *------------------------------------------------------------------
 * Filename: rbcp_lib.h
 *
 * Description: The RBCP Library header file
 * Author: Times Huang
 *
 * Copyright (c) 2015-2018 by cisco Systems, Inc.
 * All rights reserved.
 *------------------------------------------------------------------
 */


#ifndef __RBCP_LIB_H__
#define __RBCP_LIB_H__

#define HTONS               htons
#define NTOHS               ntohs

#define RBCP_MSG_BUF_SIZE   1600

#define ETH_ALEN            6

#define MAC_HEADER_LEN      14
#define LLC_LEN_802         3
#define SNAP_LEN_802        5
#define HEADER_LEN_802      (MAC_HEADER_LEN + LLC_LEN_802 + SNAP_LEN_802)

#define RBCP_PKT_RECV_TIMEOUT     (5)
#define RBCP_PKT_RECV_CLEAN       (100)
#define RBCP_RETRIES             	(6)

#define PLAT_SCP_SLOT_ID	0xAB
/*
 * Ethernet 802.2 defines
 */
#define ETH_DSAP	0xAA
#define ETH_SSAP	0xAA
#define ETH_CTRL	0x03
#define ETH_ORG_COD0	0x00
#define ETH_ORG_COD1	0x00
#define ETH_ORG_COD2	0x0C
#define ETH_PROTO_TYPE	0x011D

/*
 * RBCP SCP header defines
 */
#define SCP_DSLOT_ID		0xFE
#define SCP_SSLOT_ID		0x0D
#define SCP_DST_SAP		0x00
#define SCP_SRC_SAP		0x01
#define SCP_LEN			0x0C
#define SCP_START_SEQ_NUM	0x01


#define RBCP_PKT_PADDING          22

/*
 * NM sends with this source address
 * when it doesn't know the slot it is in.
 */
#define CISCO_SCP_SLOT_UNKNOWN          0x0E
#define CISCO_SCP_SLOT_RTR              0x0F    /* Fixed */
#define CISCO_SCP_REGISTER_SAP          0x01

/* SCP header - Addl Info mask bits */
#define RBCP_FLAG_REQ_RESP_MASK         0x01
#define RBCP_FLAG_SUPPORT_MASK          0x02
#define RBCP_FLAG_ACK_REQ_MASK          0x04
#define RBCP_FLAG_ACK_MASK              0x08

#define RBCP_FLAG_REQ             	0x00
#define RBCP_FLAG_RESP             	0x01
#define RBCP_FLAG_SUPPORT             	0x02
#define RBCP_FLAG_ACK_REQD         0x04
#define RBCP_FLAG_ACK              0x08

#define CISCO_SCP_OP_PING               0x0011
#define CISCO_SCP_OP_HEART_BEAT         0x0012
#define CISCO_SCP_OP_REG                0x0013
#define CISCO_SCP_OP_REQ_REG            0x0014
#define CISCO_SCP_CE_SHUTDOWN           0x0051
#define CISCO_SCP_CE_SHUTDOWN_COMP      0x0052
#define CISCO_SCP_CE_RELOAD             0x0053
#define CISCO_SCP_CE_IP_ADDRESS         0x0054
#define CISCO_SCP_CE_TIMEINFO           0x0055
#define CISCO_SCP_CE_PORT_FEATURE       0x0056
#define CISCO_SCP_CE_PORT_SHUTDOWN      0x0057
#define CISCO_SCP_CE_BACKUP_CONFIG      0x0058
#define CISCO_SCP_CE_RESTORE_CONFIG     0x0060
#define CISCO_SCP_CE_DEFAULTGW          0x0059
#define CISCO_SCP_CE_PLATFORM           0x005a
#define CISCO_SCP_CE_TIME               0x005b
#define CISCO_SCP_CE_PASSWORD_RESET     0x005c
#define CISCO_SCP_CE_EXIT               0x005d
#define CISCO_SCP_INTEL_PWR_OFF         0x0092
#define CISCO_SCP_INTEL_PWR_ON          0x0091
#define CISCO_SCP_UART_CONSOLE_SW       0x0098

#define CISCO_SCP_OP_PC_BLADE_MSG       0x014a

/*
 *  SRE related  messages and opcodes
 */
#define CISCO_SCP_SRE_INSTL_INFO        0x0091
#define CISCO_SCP_SRE_INSTL_INFO_RESP   0x0092
#define CISCO_SCP_SRE_INSTL_STATUS      0x0093
#define RBCP_SCP_SRE_GET_RSRC           0x0094
#define RBCP_SCP_SRE_GET_RSRC_RESP      0x0095
#define CISCO_SCP_SRE_INSTALL_CMD       0x0096
#define CISCO_SCP_SRE_INSTALL_CMD_RESP  0x0097
#define CISCO_SCP_SRE_LOCAL_OP_MSG      0x009A
#define CISCO_SCP_SRE_LOCAL_OP_MSG_RESP 0x009B

/*
 * RBCP VLAN configuration message
 */
#define SERVICE_MODULE_VLAN_CFG         0x0098

/*
 * bryce: serdes switch
 */
#define CISCO_SCP_SERDES_SWITCH         0x0065

/*
 * RBCP message to set the default gateway for the MGF interface
 */
#define SERVICE_MODULE_SRE_DEFAULT_GWY  0x99

/*
 * bryce: for the new message for daughter card cookie
 */
#define CISCO_SCP_DAUGHTER_CARD_COOKIE  0x005f

/*
 * RBCP Failure user defined codes
 */
#define RBCP_SEND_FAILURE		0x01
#define RBCP_RECV_FAILURE		0x02

#ifndef PACKED
#define STRUCT_PACKED(item) __attribute__ ((packed)) item
#define PACKED(item) item
#endif

typedef struct cisco_scp_hdr {
    u_char   PACKED(src_slot_id);
    u_char   PACKED(dst_slot_id);
    u_short  PACKED(length);
    u_char   PACKED(dst_sap);
    u_char   PACKED(src_sap);
    u_short  PACKED(opcode);
    u_short  PACKED(seq_num);
    u_char   PACKED(flags);
    u_char   PACKED(version);
    u_short  PACKED(fv_length);
    u_char   PACKED(reserved[2]);
} STRUCT_PACKED(cisco_scp_hdr_t);

typedef uchar rbcp_mac_addr_t[6];

typedef struct eth_pak_ {
    rbcp_mac_addr_t  PACKED(dest_addr);
    rbcp_mac_addr_t  PACKED(src_addr);
    ushort      PACKED(pkt_len);
    uchar       PACKED(dest_sap);
    uchar       PACKED(src_sap);
    uchar       PACKED(cntl);
    uchar       PACKED(org_code[3]);
    ushort      PACKED(protocol_type);
} STRUCT_PACKED(eth_pak_t);

typedef enum ucse_uart_type {
     UCSE_UART_HOST,
     UCSE_UART_BMC,
     UCSE_UART_LSI,
} ucse_uart_type_t;

typedef struct ucse_uart_msg {
    struct cisco_scp_hdr	PACKED(hdr);
    struct {
        u_char                PACKED(rsp);
        u_char                PACKED(pad[3]);
        ucse_uart_type_t      PACKED(ucse_uart_type);
    } payload;
} STRUCT_PACKED(ucse_uart_msg_t);

/***********************************************************************
 *  Extern Functions Declaration
 ************************************************************************/

extern int handle_ind_rbcp_msg(char *, int);
extern int rbcp_send_ack_msg(struct cisco_scp_hdr *, int);
extern int is_rbcp_msg(char *);
extern inline void rbcp_ntoh_scp_hdr(cisco_scp_hdr_t *);
extern int rbcp_clear_recv (void);
extern int rbcp_register(void);
extern int rbcp_ping(void);
extern int rbcp_intel_power_off(void);
extern int rbcp_intel_power_on(void);
extern int handle_ind_rbcp_msg(char *, int);
extern int rbcp_send_ack_msg(struct cisco_scp_hdr *, int);
extern int rbcp_get_mac(uchar);
extern int rbcp_con_sw_bmc(void);
extern int rbcp_con_sw_bmc(void);
extern int rbcp_con_sw_intel(void);
extern int rbcp_con_sw_lsi(void);
extern int rbcp_send_msgs(char *, int, uchar, ushort, int);
extern int rbcp_recv_msgs(char *, ushort, int *);

#endif /* __RBCP_LIB_H__ */

/*------------------------------------------------------------------
 * $Log: rbcp_lib.h,v $
 * Revision 1.4  2018/02/24 07:30:48  letsai
 * Collapse Kalamata-branch to Main Trunk.
 *
 * Revision 1.3  2017/03/21 08:41:57  olin2
 * Collapse Aquila-branch to Main Trunk.
 *
 * Revision 1.2.12.1  2016/10/27 08:06:53  olin2
 * Support LSI console switch utility
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
 *------------------------------------------------------------------
 * $Endlog$
 *
 *------------------------------------------------------------------
 */
