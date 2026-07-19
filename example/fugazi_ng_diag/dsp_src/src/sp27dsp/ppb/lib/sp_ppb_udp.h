/* $Id: sp_ppb_udp.h,v 1.2 2012/05/10 22:57:02 srane Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/dsp_src/src/sp27dsp/ppb/lib/sp_ppb_udp.h,v $
 *------------------------------------------------------------------
 * sp_ppb_udp.h
 *
 * Mar 2012, Smita Rane
 *
 * Copyright (c) 2012 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#ifndef __SP_PPB_UDP_H
#define __SP_PPB_UDP_H

/*----------------------------------*/
/*   	CONSTANTS    				*/
/*----------------------------------*/
#define SP_PPB_UDP_HEADER_SRC_PORT_OFST		0
#define SP_PPB_UDP_HEADER_DEST_PORT_OFST	2
#define SP_PPB_UDP_HEADER_LENGTH_OFST		4
#define SP_PPB_UDP_HEADER_CSUM_OFST			6

/*----------------------------------*/
/*   	TYPE DEFINITIONS    		*/
/*----------------------------------*/
typedef struct sp_udp_header {
	uint16_t	src_port;				// Source port
	uint16_t	dst_port;				// Destination port
	uint16_t	len;					// UDP length
	uint16_t	csum;					// UDP checksum
}  StarProPPB_UDP_HDR_t;
//}  __attribute__((__packed__)) StarProPPB_UDP_HDR_t;

/* Byte offsets for formatting */
#define	SP_PPB_UDP_HDR_SRC_PORT_RO	0
#define	SP_PPB_UDP_HDR_DEST_PORT_RO	2
#define	SP_PPB_UDP_HDR_LEN_RO		4
#define	SP_PPB_UDP_HDR_CSUM_RO		6

typedef struct sp_ip_udp_header {
	StarProPPB_IPv4_HDR_t	ip_hdr;		// IP header
	StarProPPB_UDP_HDR_t	udp_hdr;	// UDP header
}  StarProPPB_IPv4_UDP_HDR_t;
//}  __attribute__((__packed__)) StarProPPB_IPv4_UDP_HDR_t;

#endif

/******** History ********
$Log: sp_ppb_udp.h,v $
Revision 1.2  2012/05/10 22:57:02  srane
Add TDM support.

Revision 1.1  2012/04/18 09:47:31  srane
Initial checkin


$Endlog$
*/

