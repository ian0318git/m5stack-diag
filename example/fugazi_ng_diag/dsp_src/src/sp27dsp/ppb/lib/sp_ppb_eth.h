/* $Id: sp_ppb_eth.h,v 1.2 2012/05/10 22:48:11 srane Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/dsp_src/src/sp27dsp/ppb/lib/sp_ppb_eth.h,v $
 *------------------------------------------------------------------
 * sp_ppb_eth.h
 *
 * Mar 2012, Smita Rane
 *
 * Copyright (c) 2012 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#ifndef __SP_PPB_ETHERNET_H
#define __SP_PPB_ETHERNET_H

//#include "lsi_mg_std.h"

/*----------------------------------*/
/*   	CONSTANTS    				*/
/*----------------------------------*/

#define	SP_PPB_ETH_TYPE_IP			0x0800	// IP type in host byte order
#define	SP_PPB_ETH_TYPE_IP_LOW		0x08	// IP type in host byte order
#define	SP_PPB_ETH_TYPE_IP_HIGH		0x00	// IP type in host byte order
#define SP_PPB_ETH_TYPE_ARP			0x0806	// ARP type in host byte order
#define SP_PPB_ETH_TYPE_CESoETH		0x88D8
#define SP_PPB_ETH_TYPE_LINX		0x9999	// OSE LINX in host byte order
#define SP_PPB_ETH_TYPE_VLAN		0x8100	// VLAN type in the host byte order

#define SP_PPB_ETH_ADDR_LEN			6		// Octets in one ethernet addr
#define SP_PPB_ETH_ADDR_SIZE		SP_PPB_ETH_ADDR_LEN
#define SP_PPB_ETH_VLAN_TAG_SIZE	4		// Octets in one VLAN header
#define SP_PPB_ETH_DATA_LEN			1500	// Max. octets in payload
#define SP_PPB_ETH_MAX_FRM_SIZE		1518	// Max. octets in frame sans FCS
#define SP_PPB_ETH_MIN_FRM_SIZE		64
#define SP_PPB_ETH_CRC_LEN			4
#define SP_PPB_ETH_HDR_VLAN_SIZE	18
#define SP_PPB_ETH_HDR_SIZE			14

/*----------------------------------*/
/*   	TYPE DEFINITIONS    		*/
/*----------------------------------*/
/* Ethernet address */
typedef struct sp_eth_addr {
	uint8_t	byte[SP_PPB_ETH_ADDR_LEN];
} StarProPPB_EthAddr_t;

/* Ethernet header */
typedef struct sp_eth_header {
	uint8_t	dest_addr[6];					// destination ethernet address
	uint8_t	src_addr[6];					// source ethernet address
	uint8_t	proto[2];						// protocol identifier
//} StarProPPB_ETH_HDR_t;
} __attribute__((__packed__)) StarProPPB_ETH_HDR_t;

/* Ethernet header with VLAN */
typedef struct sp_eth_header_vlan {
	uint8_t	dest_addr[6];					// destination ethernet address
	uint8_t	src_addr[6];					// source ethernet address
	uint8_t	vlan[4];						// VLAN header
	uint8_t	proto[2];						// protocol identifier
} StarProPPB_ETH_VLAN_HDR_t;

/* "Extended" Ethernet header */
typedef struct sp_leth_header {
	uint8_t	id;								// identifies EMAC interface
	uint8_t	rsvd;							// reserved
	uint8_t	dest_addr[6];					// destination ethernet address
	uint8_t	src_addr[6];					// source ethernet address
	uint8_t	proto[2];						// protocol identifier
} StarProPPB_LETH_HDR_t;

/*----------------------------------*/
/*   	MACROS    					*/
/*----------------------------------*/

#define BUILD_MAC_ADDR(addr, top, bottom) { \
	addr[0] = bottom >> 24; \
	addr[1] = bottom >> 16; \
	addr[2] = bottom >> 8; \
	addr[3] = bottom >> 0; \
	addr[4] = top >> 8; \
	addr[5] = top >> 0; \
}

/*----------------------------------*/
/* 		FUNCTION DECLARATIONS 		*/
/*----------------------------------*/

#endif	/* __SP_PPB_ETHERNET_H */

/******** History ********
$Log: sp_ppb_eth.h,v $
Revision 1.2  2012/05/10 22:48:11  srane
clean up and modify files for exception handling and TDM tests.

Revision 1.1  2012/04/18 09:47:31  srane
Initial checkin


$Endlog$
*/

