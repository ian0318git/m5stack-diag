/* $Id: diag_sock.h,v 1.2 2016/04/20 08:41:36 benchen2 Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/tachi_bmc/fru/diag_sock.h,v $
 *
 *      File:   rdw_sim_lib.h
 *      Name:   Sudharshan Kadari
 *
 *      Description:
 *
 *
 * Copyright (c) 1985-2016 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *
 *********************************************************************/


#ifndef _DIAG_SOCK_H
#define _DIAG_SOCK_H

#define UDP_DIAG_PORT      9886

/**********************************************************************
* diag_tlv_tag_t
*
* TLV tag for requests between diag USD and simluators
* (MODEL and COSIM).
*
* Note: tag_error is used to indicate some error occurred. It replaces
*       the requestors tag (which is normally echoed).
**********************************************************************/
typedef enum {

        TAG_NONE = 0,  // 0
        TAG_OPCODE,    // 1
        TAG_PACKET,    // 2, MODEL -> SIMD
        TAG_ERROR = 0xEE

} diag_tlv_tag_t;

typedef enum {
	DIAG_OPC_SPROM_DUMP,
	DIAG_OPC_SPROM_GET,
	DIAG_OPC_SPROM_SET,
	DIAG_OPC_MOD_INFO_GET,
	DIAG_OPC_MOD_INFO_DUMP,
	DIAG_OPC_CLI_COMMAND,

	DIAG_OPC_END
} diag_opcode_t;
/**********************************************************************
* diag_tlv_t
**********************************************************************/
typedef struct diag_tlv_t {

  diag_tlv_tag_t    tag;
  unsigned int      len;
  unsigned short    slot; 
  unsigned short    subslot; 
  unsigned int 	    opcode;
} diag_tlv_t;

/* obvious */
#define TLV_HDR_LEN (sizeof(diag_tlv_t))

/*********************************************************************
* tlv->len does NOT include TLV header size. So, to get the proper 
* length to transmit on a wire you must add back the TLV header length.
**********************************************************************/
#define TLV_TOTAL_LEN( tlv ) ( ntohl(tlv->len) + TLV_HDR_LEN)

/**********************************************************************
* diag_serv_req_t
*
* Register READ/WRITE request structure. "reg_data" is returned for
* tag_read.
**********************************************************************/
typedef struct  {

  /* tlv header */
  diag_tlv_t    tlv;

  /* register read/write operation specific fields */
  unsigned int data;

} diag_opc_req_t;

/**********************************************************************
 * diag_pkt_req_t
 **********************************************************************/
typedef struct {

  /* tlv header */
  diag_tlv_t     tlv;

  /* packet read/write operation specific fields */
  unsigned char data[1024];

} diag_pkt_req_t;

/*********************************************************************
* Gives address of first byte of a data packet
*********************************************************************/
#define PKT_DATA_FLD( pkt_req ) (&pkt_req->data[0])

extern uint32_t diag_recv_send(int my_socket);
extern uint32_t diag_send_recv(int my_socket, uint16_t slot, 
				uint16_t subslot, uint32_t opcode, 
				uint32_t data, char *buf);
extern int create_server_connection( unsigned short tcp_port,
                              char *p_client_ip_addr );
extern int create_client_connection( uint32_t ip_addr, unsigned short tcp_port);
extern uint32_t diag_get_peer_port(uint8_t slot);
extern int diag_server_proc (int blade_slot);
extern int diag_client_proc (int blade_slot);
extern void diag_fix_blade_ip_addr(int cmc_slot);

#endif // _DIAG_SOCK_H
