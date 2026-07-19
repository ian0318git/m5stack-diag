/* $Id: diag_sock.c,v 1.2 2016/04/20 08:41:35 benchen2 Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/tachi_bmc/fru/diag_sock.c,v $
 *
 *      File:   diag_serv.c
 *      Name:   Sudharshan Kadari
 *
 *      Description:
 *       Routines to access the Redwood RTL simulation 
 *
 *
 * Copyright (c) 1985-2016 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *
 *********************************************************************/

/*
 * Copyright (c) 2006 Nuova Systems, Inc.  All rights reserved.
 * $Id: diag_sock.c,v 1.2 2016/04/20 08:41:35 benchen2 Exp $
 *
 * Routines to interface with the network.
 */

#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/time.h>

#include <net/ethernet.h>
#include <net/if.h>
#include <netinet/in.h>
#include <netpacket/packet.h>

#include <arpa/inet.h>

#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <time.h>

#define ETH_CRC_SIZE 4
#include "diag_sock.h"
#include "ipmi_sprom.h"
#include "ipmi_sprom_ops.h"

#define SRV_PRINT printf
#define DBG_PRINT printf

// Functions
extern void exit(int);

#define terminate( why) _terminate( why)
#define warn( why) _warn( why)

uint32_t diag_get_peer_port(uint8_t slot)
{
	return (UDP_DIAG_PORT+slot);
}

void _terminate( char *why)
{
	printf("\n[diag_sock] %d\n\tFATAL: %s [errno: %d -- %s]\n",
		__LINE__, why, errno, strerror(errno));
	exit(-1);
}

void _warn( char *why)
{
	int slot = 0;
	printf("\n[diagsock] WARN: %s\n"
		"[diagsock] (slot::%d)\n", why, slot );
}

void set_sockaddr( uint32_t ip_addr, 
		   uint16_t udp_port,
                   struct sockaddr_in *sockaddr_in)
{
	sockaddr_in->sin_family      = AF_INET;
	sockaddr_in->sin_addr.s_addr = htonl(ip_addr);
	sockaddr_in->sin_port        = htons(udp_port);
}

void set_tlv( diag_tlv_tag_t tag, uint16_t slot, uint16_t subslot,
		uint32_t opcode, int len, diag_tlv_t *p_tlv )
{
	p_tlv->slot = htons(slot );
	p_tlv->subslot = htons(subslot );
	p_tlv->tag = htonl( tag );
	p_tlv->len = htonl( len - TLV_HDR_LEN );
	p_tlv->opcode = htonl(opcode);
}

/*****************************************************************************
* create_server_connection
*****************************************************************************/
int create_server_connection( unsigned short tcp_port, 
			      char *p_client_ip_addr )
{
    int    s, server_s;
    int    rc;
    struct sockaddr    tmp;
    struct sockaddr    *p_sockaddr    = (struct sockaddr *)&tmp;
    struct sockaddr_in *p_sockaddr_in = (struct sockaddr_in *)&tmp;
    socklen_t salen;
    uint32_t in_addr;

    s = socket( PF_INET, SOCK_STREAM, IPPROTO_TCP );
    if (s == -1) {
	SRV_PRINT(" Port=%d Cant create socket\n", tcp_port);
        return (s);
    }

    memset((char*)p_sockaddr, 0, sizeof(*p_sockaddr));
    set_sockaddr( INADDR_ANY, tcp_port, p_sockaddr_in );

    rc = bind( s, p_sockaddr, sizeof(*p_sockaddr));
    if (rc == -1) {
	SRV_PRINT(" Port=%d Cant create socket\n", tcp_port);
        return (rc);
    }


    SRV_PRINT("[socklib] Listen on 0.0.0.0::%d\n", tcp_port );

    listen(s, 5);

    memset((char*)p_sockaddr, 0, sizeof(*p_sockaddr));
    salen = sizeof(struct sockaddr); // salen is a value/result arg!
    server_s = accept( s, p_sockaddr, &salen);

    in_addr = ntohl( (unsigned long)(p_sockaddr_in->sin_addr.s_addr) );
    SRV_PRINT("[socklib] Accept connection from %d.%d.%d.%d::%d\n",
           ((in_addr >> 24) & 0xff),
           ((in_addr >> 16) & 0xff),
           ((in_addr >>  8) & 0xff),
           ((in_addr      ) & 0xff),
           tcp_port);

    /* return client IP address, if requested */
    if (p_client_ip_addr)
        *p_client_ip_addr = in_addr;

    return server_s;
}

/*****************************************************************************
 * create_client_connection
 *
 * Creates a socket and establishes a TCP connection to the specified
 * IP address and port. The connection attempts will continue forever,
 * the assumption beng that the connection is failing due only to the
 * fact the server has not started "listening" yet.
 *****************************************************************************/
int create_client_connection( uint32_t ip_addr, unsigned short tcp_port )
{
        int    s;
        int    rc;
        struct sockaddr    tmp;
        struct sockaddr    *p_sockaddr    = (struct sockaddr *)&tmp;
        struct sockaddr_in *p_sockaddr_in = (struct sockaddr_in *)&tmp;


        SRV_PRINT("[socklib] Connecting to %d.%d.%d.%d::%d..\n",
               ((ip_addr >> 24) & 0xff),
               ((ip_addr >> 16) & 0xff),
               ((ip_addr >>  8) & 0xff),
               ((ip_addr      ) & 0xff),
               tcp_port);
                                                                                                                          
        /* Create the socket descriptor */
        s = socket( PF_INET, SOCK_STREAM, IPPROTO_TCP );
        if (s == -1) {
		SRV_PRINT(" ip=%d.%d.%d.%d Port=%d Cant create socket\n",
			(ip_addr >> 24)&0xFF,
			(ip_addr >> 16)&0xFF,
			(ip_addr >>  8)&0xFF,
			(ip_addr&0xFF), tcp_port);
		return (s);
	}

retry:
        memset((char*)p_sockaddr, 0, sizeof(*p_sockaddr));
        set_sockaddr( ip_addr, tcp_port, p_sockaddr_in );

        /* try to coonect. On failure, assume its because the peer isn't
         * listening yet and retry */
        rc = connect( s, p_sockaddr, sizeof(*p_sockaddr) );
        if (rc == -1) {
            SRV_PRINT("[socklib] connect failed: errno= %d (%s). Retrying..\n", 
			errno, strerror(errno));
            sleep(1);
            goto retry;
        }

        SRV_PRINT("[socklib] Connected to @%d.%d.%d.%d::%d\n",
               ((ip_addr >> 24) & 0xff),
               ((ip_addr >> 16) & 0xff),
               ((ip_addr >>  8) & 0xff),
               ((ip_addr      ) & 0xff),
               tcp_port);


        return s;
}

/*
 * Append a CRC-32 blob onto the end of the buffer.  We assume that there 
 * are 4 bytes of space free at the end of the buffer for this.
 */
#define CRCPOLY_LE 0xEDB88320
uint32_t compute_crc(uint8_t *data, size_t datalen)
{
	int i;
	uint32_t crc;

	crc = ~0;
	while (datalen--) {
		crc ^= *data++;
		for (i = 0; i < 8; i++)
			crc = (crc >> 1) ^ ((crc & 1) ? CRCPOLY_LE : 0);
	}
	return( ~(crc) );
}

static inline void
network_append_crc_32(uint8_t *data, size_t datalen)
{
	uint32_t *crc = (uint32_t *)(data + datalen);
	*crc = compute_crc(data, datalen);
}

/******************************************************************************
* diag_send_tlv
*
* Utility to do send
*
* send socket is created and destroyed inside this func
* Destination port must be provided
* The size fo the recv buffer, for returned data, must be provided
* Receiver address is determined by diag_socket (which must've been "bind"d earlier)
*
******************************************************************************/
static int diag_send_tlv( int s, void *p_data, int datalen,
                        char *send_err_str)
{
	int n;

	SRV_PRINT("s=%d sending %d bytes\n", s, datalen);
	n = send( s, p_data, datalen, 0 );
	if (n == -1) {
		SRV_PRINT( send_err_str );
		return (n);
	} else {
		DBG_PRINT("Sent %d bytes\n", n);
	}
	return (n);
}

/******************************************************************************
 * diag_recv_tlv
 *
 * Utility to read a TLV header and, from that, determine how many following
 * bytes to read (the TLV contents, which are possibly null).
 ******************************************************************************/
int diag_recv_tlv(int socket, unsigned char *p_buf, int max_len)
{
        int n, tlv_len;
        diag_tlv_t *p_tlv = (diag_tlv_t *)p_buf;

        /* receive the next TLV header */
        n = recv( socket, p_buf, sizeof(diag_tlv_t), 0 );

        if (n <= 0) {
		if (errno == EAGAIN) {	/* no nofitications... */
			return n;
		}
	}

        if (n != sizeof(diag_tlv_t)) {
		SRV_PRINT("Incomplete TLV?");
		return (-1);
	}

        DBG_PRINT("RECV'D %d bytes\n", n);
	max_len -= n;

	/* read the TLV data (if any) */
        tlv_len = ntohl(p_tlv->len);
	if (tlv_len > max_len) {
		printf(" ERROR: Insufficient size (tlv_len=%d, max_len=%d)\n",
			tlv_len, max_len);
		tlv_len = max_len;
	}
	DBG_PRINT("tlv_len = %d\n", tlv_len);
        if (tlv_len > 0)
        {
		n += recv( socket, p_buf + n, tlv_len, 0);
		if (n != (tlv_len + (int)sizeof(diag_tlv_t)) ) {
			SRV_PRINT("Incomplete TLV(2)?");
			return (-1);
		}
        }
        return n;
}

uint32_t diag_send_recv(int my_socket, uint16_t slot, uint16_t subslot,
		uint32_t opcode, uint32_t data, char *buf)
{
	int n;
	diag_opc_req_t req, *p_req = &req;
	diag_pkt_req_t pkt, *p_pkt = &pkt;

	switch (opcode) {

		case	DIAG_OPC_SPROM_SET:
			set_tlv(TAG_OPCODE, slot, subslot, opcode, 
				sizeof(pkt), &p_pkt->tlv );
			memcpy(p_pkt->data, buf, (data > 1) ? 
				sizeof(sprom_ipmi_mezz_t) : 
				sizeof(sprom_ipmi_ibmc_t));
			n = diag_send_tlv( my_socket, p_pkt, sizeof(pkt), 
					"Sending Request");
			if (n < 0) {
				return (n);
			}
			break;

		default:
			set_tlv(TAG_OPCODE, slot, subslot, opcode, 
				sizeof(req), &p_req->tlv );
			p_req->data = htonl(data);
			n = diag_send_tlv( my_socket, p_req, sizeof(req), 
					"Sending Request");
			if (n < 0) {
				return (n);
			}
			break;
	}

	n = diag_recv_tlv(my_socket, (unsigned char*)p_pkt, sizeof(pkt));
	if (n <= 0) {
		/* Evidently can happen sometimes for non-blocking sockets*/
		if (errno == EAGAIN) {
			return 0;
		}
		DBG_PRINT("Receiving response");
		return (n);
	}

	switch (ntohl(pkt.tlv.opcode)) {
		case	DIAG_OPC_SPROM_GET:
			memcpy(buf, p_pkt->data, (data > 1) ? 
				sizeof(sprom_ipmi_mezz_t) : 
				sizeof(sprom_ipmi_ibmc_t));
			break;	

		default:
			break;
	}
	return( 0 );
}

extern int blade_sprom_rd_hw(uint8_t *buf, int index);
extern int blade_sprom_wr_hw(uint8_t *buf, int index);

uint32_t diag_recv_send(int my_socket)
{
	int n, resp_len = 0;
	diag_opc_req_t req, *p_req = &req;
	diag_pkt_req_t pkt, *p_pkt = &pkt;
	unsigned char *presp = NULL;
	
	n = diag_recv_tlv (my_socket, 
			(unsigned char*)p_pkt, sizeof(pkt));
	if (n<=0) {
		SRV_PRINT("n=%d, errno = %d %s \n", 
			n, errno, strerror(errno));
	} else {
		SRV_PRINT("received n=%d bytes \n", n);
	}

	memcpy((char*)p_req, (const char*)p_pkt, sizeof(req));


	switch (ntohl(req.tlv.opcode)) {
		case	DIAG_OPC_SPROM_DUMP:
			set_tlv(TAG_OPCODE, ntohs(p_req->tlv.slot), 
				ntohs(p_req->tlv.subslot), ntohl(req.tlv.opcode), 
				sizeof(req), &p_req->tlv );
			presp = (unsigned char *)p_req;
			resp_len = sizeof(*p_req);
			break;	

		case	DIAG_OPC_SPROM_GET:
			{
			blade_sprom_rd_hw(p_pkt->data, ntohs(p_req->tlv.subslot));
			set_tlv(TAG_PACKET, ntohs(p_req->tlv.slot),
                                ntohs(p_req->tlv.subslot), ntohl(req.tlv.opcode), 
				sizeof(pkt), &p_pkt->tlv );
			presp = (unsigned char *)p_pkt;
			resp_len = sizeof(*p_pkt);
			}
			break;	

		case	DIAG_OPC_SPROM_SET:
			blade_sprom_wr_hw(p_pkt->data, ntohs(p_req->tlv.subslot));
			set_tlv(TAG_OPCODE, ntohs(p_req->tlv.slot),
                                ntohs(p_req->tlv.subslot), ntohl(req.tlv.opcode), 
				sizeof(req), &p_req->tlv );
			presp = (unsigned char *)p_req;
			resp_len = sizeof(*p_req);
			break;	

		case	DIAG_OPC_MOD_INFO_GET:
			set_tlv(TAG_PACKET, ntohs(p_req->tlv.slot),
                                ntohs(p_req->tlv.subslot), ntohl(req.tlv.opcode), 
				sizeof(pkt), &p_pkt->tlv );
			presp = (unsigned char *)p_pkt;
			resp_len = sizeof(*p_pkt);
			break;	

		case	DIAG_OPC_MOD_INFO_DUMP:
			set_tlv(TAG_OPCODE, ntohs(p_req->tlv.slot),
                                ntohs(p_req->tlv.subslot), req.tlv.opcode, 
				sizeof(req), &p_req->tlv );
			presp = (unsigned char *)p_req;
			resp_len = sizeof(*p_req);
			break;	

		case	DIAG_OPC_CLI_COMMAND:
			set_tlv(TAG_OPCODE, ntohs(p_req->tlv.slot),
                                ntohs(p_req->tlv.subslot), ntohl(req.tlv.opcode), 
				sizeof(req), &p_req->tlv );
			presp = (unsigned char *)p_req;
			resp_len = sizeof(*p_req);
			break;	

		default:
			set_tlv(TAG_OPCODE, ntohs(p_req->tlv.slot),
                                ntohs(p_req->tlv.subslot), ntohl(req.tlv.opcode), 
				sizeof(req), &p_req->tlv );
			presp = (unsigned char *)p_req;
			resp_len = sizeof(*p_req);
			break;
	}

	/* Send the response */ 
	diag_send_tlv( my_socket, presp, resp_len, "Sending Response");
	return (0);
}
