/* $Id: sp_ppb_ipv4.h,v 1.2 2012/05/10 22:48:11 srane Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/dsp_src/src/sp27dsp/ppb/lib/sp_ppb_ipv4.h,v $
 *------------------------------------------------------------------
 * sp_ppb_ipv4.h
 *
 * Mar 2012, Smita Rane
 *
 * Copyright (c) 2012 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#ifndef __SP_PPB_IPV4_H
#define __SP_PPB_IPV4_H

//#include "lsi_mg_std.h"

/*----------------------------------*/
/*   	CONSTANTS    				*/
/*----------------------------------*/
/* Protocols	*/
#define SP_PPB_IPV4_PROTO_ICMP 		1
#define SP_PPB_IPV4_PROTO_UDP 		17
#define SP_PPB_IPV4_PROTO_UDPLITE 	170
#define SP_PPB_IPV4_PROTO_TCP 		6
#define SP_PPB_IPV4_PROTO_OFST 		9
#define SP_PPB_IPV4_HDR_SIZE 		20

/* Address size	*/
#define	SP_PPB_IPV4_ADDR_SIZE		1
#define	SP_PPB_IP_ADDR_SIZE_BYTE	4
#define	SP_PPB_IPV6_ADDR_SIZE		4

/*----------------------------------*/
/*   	TYPE DEFINITIONS    		*/
/*----------------------------------*/
/* IPv4 address type */
typedef struct sp_ipv4_addr{
	uint32_t	word[SP_PPB_IPV4_ADDR_SIZE];
} StarProPPB_IPv4_ADDR_t;

typedef struct sp_ipv6_addr{
	uint32_t	word[SP_PPB_IPV6_ADDR_SIZE];
} StarProPPB_IPv6_ADDR_t;

/* IPv4 header type */
#define	SP_PPB_IPv4_HDR_WORD0_LEN_OFST		0
#define	SP_PPB_IPv4_HDR_WORD0_LEN_MASK		0x0000FFFF
#define	SP_PPB_IPv4_HDR_WORD0_LEN_DFLT		0
#define SP_PPB_IPv4_HDR_WORD0_LEN_GET(word32)	( (uint16_t) \
	SP_PPB_REG32_GET_FIELD(word32,\
	SP_PPB_IPv4_HDR_WORD0_LEN_MASK, \
	SP_PPB_IPv4_HDR_WORD0_LEN_OFST) )

#define	SP_PPB_IPv4_HDR_WORD0_V_OFST		28
#define	SP_PPB_IPv4_HDR_WORD0_V_MASK		0xF0000000
#define	SP_PPB_IPv4_HDR_WORD0_V_DFLT		0x4
#define	SP_PPB_IPv4_HDR_WORD0_HL_OFST		24
#define	SP_PPB_IPv4_HDR_WORD0_HL_MASK		0x0F000000
#define	SP_PPB_IPv4_HDR_WORD0_HL_DFLT		0x5
#define	SP_PPB_IPv4_HDR_WORD0_TOS_OFST		16
#define	SP_PPB_IPv4_HDR_WORD0_TOS_MASK		0x00FF0000
#define	SP_PPB_IPv4_HDR_WORD0_TOS_DFLT		0x00

#define	SP_PPB_IPv4_HDR_WORD0_DFLT		\
	( (SP_PPB_IPv4_HDR_WORD0_V_DFLT << SP_PPB_IPv4_HDR_WORD0_V_OFST)	\
	| (SP_PPB_IPv4_HDR_WORD0_HL_DFLT << SP_PPB_IPv4_HDR_WORD0_HL_OFST) )

#define	SP_PPB_IPv4_HDR_WORD1_DFLT			0x00000000

#define	SP_PPB_IPv4_HDR_WORD2_TTL_OFST		24
#define	SP_PPB_IPv4_HDR_WORD2_TTL_MASK		0xFF000000
#define	SP_PPB_IPv4_HDR_WORD2_TTL_DFLT		(uint8_t) 0xFF
#define	SP_PPB_IPv4_HDR_WORD2_PROTO_OFST	16
#define	SP_PPB_IPv4_HDR_WORD2_PROTO_MASK	0x00FF0000
#define	SP_PPB_IPv4_HDR_WORD2_PROTO_DFLT	SP_PPB_IPV4_PROTO_UDP
#define	SP_PPB_IPv4_HDR_WORD2_DFLT			0xFF110000

/* Byte offsets for formatting */
#define	SP_PPB_IPv4_HDR_WORD0_RO	0
#define	SP_PPB_IPv4_HDR_WORD1_RO	4
#define	SP_PPB_IPv4_HDR_WORD2_RO	8
#define	SP_PPB_IPv4_HDR_WORD3_RO	12
#define	SP_PPB_IPv4_HDR_WORD4_RO	16

typedef struct sp_ipv4_header {
  	union{
  		uint32_t reg;
  		struct {
  			uint32_t hl			:4;			// Header length
  			uint32_t v			:4;			// Version
  			uint32_t tos		:8;			// Type Of Service
  			uint32_t len		:16;		// Payload length
  		} fields;
  	} word0;

  	union{
  		uint32_t reg;
  		struct {
  			uint32_t id			:16;		// Identification
  			uint32_t offset		:16;		// fragment offset field
  		} fields;
  	} word1;

 	union{
  		uint32_t reg;
  		struct {
  			uint32_t ttl		:8;			// time to live
  			uint32_t protocol	:8;			// protocol
  			uint32_t csum		:16; 		// checksum
  		} fields;
  	} word2;
   	StarProPPB_IPv4_ADDR_t src_addr;			// source IP address
  	StarProPPB_IPv4_ADDR_t dest_addr; 			// destination IP address
} StarProPPB_IPv4_HDR_t;
//} __attribute__((__packed__)) StarProPPB_IPv4_HDR_t;

/* IPv4 header configuration type */
typedef struct
{
	uint8_t		tos;						// Type Of Service
	uint8_t		ttl;						// Time To Live
	uint16_t	rsvd;						// reserved
} StarProPPB_IPV4_HDR_CFG_t;
//}__attribute__((__packed__)) StarProPPB_IPV4_HDR_CFG_t;

/*----------------------------------*/
/*   	MACROS    					*/
/*----------------------------------*/
#define SP_PPB_IPv4_ADDR(A,B,C,D)		((uint32_t) (	((uint32_t)A<<24) | \
														((uint32_t)B<<16) | \
														((uint32_t)C<<8)  | \
														((uint32_t)D<<0)) )
/*----------------------------------*/
/* 		FUNCTION DECLARATIONS 		*/
/*----------------------------------*/

#endif	/* __SP_PPB_IPV4_H */

/******** History ********
$Log: sp_ppb_ipv4.h,v $
Revision 1.2  2012/05/10 22:48:11  srane
clean up and modify files for exception handling and TDM tests.

Revision 1.1  2012/04/18 09:47:31  srane
Initial checkin


$Endlog$
*/

