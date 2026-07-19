/* $Id: libeth.h,v 1.2 2017/07/28 07:58:48 harrchan Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/dsp_src/src/oakenshield_dsp/ppb/lib/libeth.h,v $
 *------------------------------------------------------------------
 * libeth.h
 *
 * Mar 2012, Smita Rane
 *
 * Copyright (c) 2017 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

/******************************************************************************
 *                             NOTIFICATION
 *
 * Copyright (c) 2010 LSI Inc.  All Rights Reserved
 *
 * This is unpublished proprietary information of LSI Inc.  This
 * copyright notice does not evidence publication.
 *
 * The use of the software, documentation, methodologies, and other information
 * contained herein is governed solely by the associated license agreements.
 * Any inconsistent use shall be deemed to be a misappropriation of the
 * intellectual property of LSI Inc. and treated accordingly.
 *----------------------------------------------------------------------------
 *
 * libeth.h - utilities for using SP2700 Ethernet MAC, PCE and TXD
 *
 *  Created on: Dec 8, 2009
 *      Author: dokim
 */

#ifndef LIBETH_H_
#define LIBETH_H_

#include "sp_ppb_eth.h"
#include "sp_ppb_icmp.h"
#include "sp_ppb_ipv4.h"
#include "sp_ppb_udp.h"
#include "emac27_hal.h"
#include "diag_ppb.h"

/*----------------------------------*/
/*   	CONSTANTS    				*/
/*----------------------------------*/

#define PKT_SUCCESS						0
#define PKT_ERR_INVALID_MACDEST			1
#define PKT_ERR_INVALID_MACSRC			2
#define PKT_ERR_INVALID_IPDEST			3
#define PKT_ERR_INVALID_IPSRC			4
#define PKT_ERR_INVALID_SIZE			5
#define PKT_ERR_NO_BUFFERS				6
#define PKT_ERR_INVALID_UDPDESTPORT		7
#define PKT_ERR_INVALID_UDPSRCPORT		8

/*----------------------------------*/
/*		DEFAULT VALUES				*/
/*----------------------------------*/

/* EMAC0 default configuration */
#define UDP_HEADER_LEN 8
#define IPv6_HEADER_LEN 40

/* PHY device addresses */
#define DEVADDR_AM3SDB_MAC0			0x2
#define DEVADDR_AM3SDB_MAC1			0xA
#define DEVADDR_RABBEARS_DSP0MAC0	0x10
#define DEVADDR_RABBEARS_DSP1MAC0	0x11

/* PHY OUI register addresses */
#define MDIO_REG_PHY_ID_UPPER		0x2
#define MDIO_REG_PHY_ID_LOWER		0x3

/*----------------------------------*/
/*   	MACROS    					*/
/*----------------------------------*/

#define SP_PPB_HTONS(n) ( (uint16_t)	\
						( (((uint16_t)(n) & 0x00ff) << 8)	\
						| (((uint16_t)(n) & 0xff00) >> 8) ) )
#define SP_PPB_HTONL(n) ( ((((uint32_t)(n) & 0x000000ff)) << 24) \
						| ((((uint32_t)(n) & 0x0000ff00)) << 8)	\
						| ((((uint32_t)(n) & 0x00ff0000)) >> 8)	\
						| ((((uint32_t)(n) & 0xff000000)) >> 24) )

#define SP_PPB_NTOHS	SP_PPB_HTONS
#define SP_PPB_NTOHL	SP_PPB_HTONL

/*----------------------------------*/
/*   	TYPE DEFINITIONS    		*/
/*----------------------------------*/

typedef long long MAC_ADDR;

typedef enum {
	EMAC0 = 0,
	EMAC1
} sp27xxMacID_t;

/* Srtucture used to pass information for header formatting
 * the LETH header ends aligned on a 32-bit boundary to avoid
 * structure alignment problems.
 */
typedef struct{
	StarProPPB_ETH_HDR_t	eth_hdr;			// Ethernet header
	StarProPPB_IPv4_HDR_t	ip_hdr;				// IP header
	StarProPPB_UDP_HDR_t	udp_hdr;			// UDP header
}  StarProPPB_TxHDR_t;
//}  __attribute__((__packed__)) StarProPPB_TxHDR_t;

typedef struct {
	uint32_t 			ip_addr;
	MAC_ADDR 			mac_addr;
} IP2ETHERmap_t;

typedef enum {
	SSSMII = 0,
	SGMII,
}ethMode_t;

/* forward path option */
typedef enum {
	LOCAL_FWD_PATH = 0,
	EXTERNAL_FWD_PATH,
}fwdPath_t;

/* routing option */
typedef enum {
	NORMAL_ROUTING = 0,
	MAC_ADDR_ROUTING,
} routeOption;

typedef enum {
	PHYMODE_NO_PHY = 0,
	PHYMODE_WAIT_AUTONEG,
	PHYMODE_NO_WAIT_AUTONEG
} phyMode_t;

/* mode defines */
typedef enum {
	NO_FORWARD = 0,
	FORWARD_ALWAYS,
	FORWARD_IF_NOT_MATCH_LOCAL_RECV_THAT_NOT_MATCH_BOTH,
	FORWARD_IF_NOT_MATCH_LOCAL_RECV_BROADCAST,
	FORWARD_IF_NOT_MATCH_LOCAL_RECV_BROADCAST_WITH_REMOTE_AS_LOCAL
} fwdMode_t;

typedef enum {
	FAIL_Q = 0,
	PPB_Q,
	DSS0_Q,
	DSS1_Q,
	DSS2_Q,
	DSS3_Q,
	PCE_ALL_Q
}PCE_QUEUE;

typedef enum {
	TXD_Q0 = 0,
	TXD_Q1,
	TXD_Q2,
	TXD_Q3,
	TXD_Q4,
	TXD_Q5,
	TXD_Q6,
	TXD_Q7,
	TXD_Q8,
	TXD_Q_ALL
}TXD_QUEUE;

#define PCE_DLTA	0
#define PCE_DLTB	1

#define PCE_DLT_INIT_ON 1
#define PCE_DLT_INIT_OFF 0

#define PCE_USE_DLTB 1
#define PCE_NO_USE_DLTB 0

#define PCE_CLASSIFICATION_ON	1
#define PCE_CLASSIFICATION_OFF	0

/* IP protocol version */
#define IPV4	0
#define IPV6	1

/* Pkt type */
#define ETHERNETV2 	0
#define SNAP8023 	1

/* Number of buffer TX descriptors */
#define BuffTxElements  36
/* Number of buffer RX descriptors */
// SR prig #define BuffRxElements  24
#define BuffRxElements  90
/* Round frame size up to next buffer boundary */
#define XMIT_FRAME_SIZE (((SP_PPB_ETH_MAX_FRM_SIZE/XMIT_BUF_MAX_SIZE)+1) * XMIT_BUF_MAX_SIZE)

extern PCE_DEVICE_S pce0;
extern PCE_DEVICE_S pce1;

extern TXD_DEVICE_S txd0;
extern TXD_DEVICE_S txd1;

void
sp_EthSoftRst(			/* reset both EMAC interfaces: MAC, PCE & TXD - call after every loopback test */
	void);

int32_t
sp_EthSetupMAC(					/* set up MAC - call after PCE is set up to avoid losing packets */
		sp27xxMacID_t eth_port,	/* in: SP27XX eth port, EMAC0 or EMAC1 */
		ethMode_t eth_mode,		/* in: SSSMII or SGMII */
		phyMode_t phymode);		/* in: phy is there or not */

/* check LINKOK bit in mac_anstat register, returns 1 if LINK is on, 0 otherwise */
int32_t
sp_EthChkMACLinkOk(
		sp27xxMacID_t eth_port);	/* in: SP27XX eth port, EMAC0 or EMAC1 */

int32_t
sp_EthSetupPCE(
		sp27xxMacID_t eth_port,	/* in: SP27XX eth port, EMAC0 or EMAC1 */
		fwdMode_t fwd_mode,		/* in: setting forward mode one of members in fwdMode_t */
		MAC_ADDR* mac_addr,		/* in: ptr to mac addresses used by system */
		uint32_t dlt_init,		/* in: dlt init will be done during initialization if this is 1 */
		uint32_t dltb_en,		/* in: if '1', dlt_b is enabled */
		routeOption mac_route,	/* in: mac address route on */
		uint32_t classification,/* in: classification on/off */
		void* qstart,			/* in: ptr to the starting address of rx buffer descriptors */
		uint32_t n_elements, 	/* in: number of "total" number of rx buffer descriptors of "all" ques */
		void* rxbuf);			/* in: ptr to the starting address of rx buffer */

/* set the range of dlt entry number in which all entries will be routed to dest que */
int32_t
sp_EthSetupDLTEntry(
		sp27xxMacID_t eth_port,	/* in: SP27XX eth port, EMAC0 or EMAC1 */
		uint32_t dest_rxq,		/* in: destination que */
		uint32_t dlt_entry_from,/* in: dlt entry number range from */
		uint32_t dlt_entry_to); /* in: dlt entry number range to */

/* set the max and min entry number (i.e udp port number) for DLTa or DLTb */
int32_t
sp_EthSetupDLTEntryMaxMin(
		sp27xxMacID_t eth_port,
		uint32_t dlta_dltb, 		/* in: '0' if dlta, '1' if dltb */
		uint16_t max,				/* in: max entry offset */
		uint16_t min);				/* in: min entry offset */

int32_t
sp_EthSetupTXD(
		sp27xxMacID_t eth_port,	/* in: SP27XX eth port, EMAC0 or EMAC1 */
		fwdPath_t fwd_path, 	/* in: local or remote, when local, the forward path will be made locally i.e) PCE0-TXD0 amd PCE1-TXD1 */
		void* qstart, 			/* in: ptr to the starting address of tx buffer descriptors */
		uint32_t n_elements);	/* in: number of "total" number of tx buffer descriptors of "all" ques */

/* register a raw packet into a txq */
int32_t				 	 	/* ret: num of transmitted or ERROR */
sp_EthRegRawPktToTxq(
	sp27xxMacID_t eth_port,		/* in: SP27XX eth port, EMAC0 or EMAC1 */
	uint32_t txq,				/* in: the queue to use for transmission... 0,1,2,3,4,5... */
	uint8_t *tx_buf,  			/* in: pointer to data buffer with UDP packet */
	uint32_t pkt_size);			/* in: size of UDP pay-load only */

/* trigger transmit (this function will check if tx scheduler is ready for handling packets in desired que first.) */
int32_t
sp_EthTxStart(
	sp27xxMacID_t eth_port, 	/* in: SP27XX eth port, EMAC0 or EMAC1 */
	uint32_t txq);				/* in: the queue to use for transmission... 0,1,2,3,4,5... */

/* move received (reception of packets is done by PCE(H/W state machine) automatically) packet to rx_buf */
int32_t							/* ret: num of bytes received or ERROR */
sp_EthRxRawPkt(
	sp27xxMacID_t eth_port,		/* in: SP27XX eth port, EMAC0 or EMAC1 */
	uint32_t  rxq,				/* in: DLT_DEST_DSS0, DLT_DEST_DSS1 ... DLT_DEST_PPB */
	uint32_t* msg_sz,			/* out: pkt size */
	uint8_t *rx_buf); 			/* out: buffer to copy packet into */

/****** Buidling a packet ********/

/* building up IPv4 udp packet */
int32_t
sp_EthBuildUdpPktIPv4(				/* NOTE: p_buf area > (payload_len + 48) bytes */
	MAC_ADDR	dst_mac_addr,		/* in: MACADDR to send to */
	MAC_ADDR	src_mac_addr,		/* in: MACADDR to send from */
	uint32_t 	dst_ip_addr,		/* in: IPADDR to send to  */
	uint32_t 	src_ip_addr,		/* in: IPADDR for src  */
	uint16_t 	dst_udp_port,		/* in: UDP port to send to */
	uint16_t 	src_udp_port,		/* in: UDP port for src */
	const uint8_t 	*p_payload,		/* in: UDP payload buffer */
	uint16_t 	udp_payload_sz,		/* in: UDP payload size in bytes (w/o any header) */
	uint8_t		*p_buf);			/* in: area where pkt will be built */

/* building up IPv6 udp packet */
int32_t
sp_EthBuildUdpPktIPv6(
	MAC_ADDR	dst_mac_addr,		/* in: MACADDR to send to */
	MAC_ADDR	src_mac_addr,		/* in: MACADDR to send from */
	uint32_t 	dst_ipv6_addr[4],	/* in: IPADDR to send to  */
	uint32_t 	src_ipv6_addr[4],	/* in: IPADDR for src  */
	uint16_t dst_udp_port, 			/* destination udp port */
	uint16_t src_udp_port, 			/* source udp port */
	const uint8_t *frame, 				/* actual data */
	uint16_t data_len, 				/* size of the payload in bytes without header */
	uint8_t *buf,					/* area where the packet is built */
	uint8_t packet_type);  			/* packet type: ETHERNET_V2, ETHERNET_V2_with_VLAN, SNAP, SNAP_with_VLAN */

/* calculate and add checksum into the udp packet */
void
sp_EthAddUdpChksum(				/* checksum on pre-formatted packet from BuildUdpPkt() */
	uint8_t *tx_buf, 		 	/* in: pointer to data buffer with UDP packet */
	uint16_t udp_payload_sz);	/* in: UDP payload size in bytes (w/o any header) */

void
sp_EthAddVlanTag(
		uint8_t *p_buf, 		/* in: pointer to data buffer with UDP packet */
		uint16_t vlan_id,		/* in: Vlan ID */
		uint32_t payload_sz);	/* in: UDP payload size in bytes (w/o any header) */

void
sp_EthConvertPktLLCSnapType(
		uint8_t *p_buf, 		/* in: pointer to data buffer with UDP packet */
		uint32_t payload_sz);	/* in: UDP payload size in bytes (w/o any header) */


/*********** PHY Control through MDIO PORT *****************/

/* setup mdio with defaults */
void
sp_EthPhyMDIOPortSetupDflt(
		void);

/* read data from a register in PHY */
uint16_t
sp_EthPhyMDIOPortRd(
		uint8_t dev_addr,
		uint8_t reg_addr);

/* write data into a register in PHY */
void
sp_EthPhyMDIOPortWr(
		uint8_t dev_addr,
		uint8_t reg_addr,
		uint32_t wr_data);

/* poll SFP to make sure that auto-negotiation is complete and that the sgmii link is up */
int32_t
emac_wait_link_PHY(
	uint8_t dev_addr); /* in: MDIO device address of the PHY that should get polled for link status */

#endif /* LIBETH_H_ */



/******** History ********
$Log: libeth.h,v $
Revision 1.2  2017/07/28 07:58:48  harrchan
Collapse Oakenshield-branch to Main Trunk.

Revision 1.1.2.1  2017/06/29 08:14:35  harrchan
Initial commit code for Oakenshield

Revision 1.2.86.1  2017/03/30 10:25:50  harrchan
Add fpga upgrade utility

Revision 1.2  2012/05/10 22:48:11  srane
clean up and modify files for exception handling and TDM tests.

Revision 1.1  2012/04/18 09:47:31  srane
Initial checkin


$Endlog$
*/

