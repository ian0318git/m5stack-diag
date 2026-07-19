/* $Id: sm_patriot_utils.c,v 1.26 2018/05/18 09:24:48 alpeng Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/sm_patriot_utils.c,v $
 *******************************************************************************
 * File Name: sm_patriot_utils.c
 *
 * Description: Patriot utilities source file
 *
 *      
 * Author: Huan Ngo
 * Copyright (c)2013-2018 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *******************************************************************************
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <net/if.h>

#include "endians.h"
#include "types.h"
#include "defs.h"
#include "signals.h"
#include "error.h"
#include "nvmonvars.h"
#include "nvsysvars.h"
#include "pm_utils.h"
#include "pci.h"
#include "slot.h"
#include "mon_plat_defs.h"
#include "common.h"
#include "common_utils.h"
#include "cookie_4.h"
#include "proto.h"
#include "queryflags.h"
#include "router_if.h"
#include "sgmii_defs.h"
#include "strings.h"
#include "new_proto.h"
#include "sm_patriot.h"
#include "ethernet.h"
#include "ngio.h"
#include "i2c_api.h"
#include "pca.h"

extern uchar cmd_param[4];

static fe_packet_t recv_packet;
static fe_packet_t * recv_packet_p = &recv_packet;
static fe_packet_t cmd_packet;
static fe_packet_t *cmd_packet_p = &cmd_packet;
static fe_packet_t result_packet;
static fe_packet_t *result_packet_p = &result_packet;

/* From P1020RDB_BSP_User_Manual.pdf for MAC1 */
mac_addr_t dest_mac_addr = {0x00, 0x22, 0xbd, 0xe2, 0xd6, 0x15};

mac_addr_t patriot_sm1_mac;
mac_addr_t patriot_sm2_mac;

mac_addr_t host_sm1_mac;
mac_addr_t host_sm2_mac;
uchar  tx_packet_buffer[1600];

extern int get_host_mac_addr(uint, unsigned char *);
extern n2g_i2c_if_t pca_i2c[];

static int socket_gl;

static reg_info_t patriot_te3_fpga_regs[] = {
    {"LED Control                  ",
                                0x00, SAVE_RESTORE, {BW_8BITS}, 0x1F, 0x3F},
    {"Port Type Select             ",
                                0x01, READ_ONLY,  {BW_8BITS}, 0x79, 0x00},
    {"Framer GPIO                  ",
                                0x02, READ_ONLY,  {BW_8BITS}, 0xFF, 0x00},
    {"Framer GPIO OE               ",
                                0x03, READ_WRITE, {BW_8BITS}, 0xFF, 0x00},
    {"T3 Sub Mode Sel              ",
                                0x06, READ_ONLY,  {BW_8BITS}, 0x3F, 0x00},
    {"T3 Sub BandWidth Sel1        ",
                                0x07, READ_WRITE, {BW_8BITS}, 0xFF, 0x00},
    {"T3 Sub BandWidth Sel2        ",
                                0x08, READ_WRITE, {BW_8BITS}, 0xFF, 0x00},
    {"T3 Sub BandWidth Sel3        ",
                                0x09, READ_WRITE, {BW_8BITS}, 0x7F, 0x00},
    {"E3 Sub Mode Sel              ",
                                0x0a, READ_ONLY,  {BW_8BITS}, 0x17, 0x00},
    {"E3 Sub BandWidth Sel1        ",
                                0x0b, READ_WRITE, {BW_8BITS}, 0xFF, 0x00},
    {"E3 Sub BandWidth Sel2        ",
                                0x0c, READ_WRITE, {BW_8BITS}, 0xFF, 0x00},
    {"E3 Sub BandWidth Sel3        ",
                                0x0d, READ_WRITE, {BW_8BITS}, 0x7F, 0x00},
    {"TDM FPGA Rev. Reg            ",
                                0x0e, READ_ONLY,  {BW_8BITS}, 0xFF, 0x00},
    {"Scratch Pad Reg              ",
                                0x0f, READ_WRITE, {BW_8BITS}, 0xFF, 0x00},
    {"Interrupt cause reg          ",
                                0x10, READ_ONLY,  {BW_8BITS}, 0x03, 0x00},
    // FPGA multiboot registers
    {"FPGA Reconfig Ctrl Reg                                      ",
     0x20, READ_ONLY,  {BW_8BITS}, 0x00, 0x00},
    {"FPGA Reconfig Status Reg                                    ",
     0x21, READ_ONLY,  {BW_8BITS}, 0x00, 0x00},
    {"Cached FPGA Upgrade Img Header-Rev.ID.Byte 0(LS Byte)       ",
     0x24, READ_ONLY,  {BW_8BITS}, 0x00, 0x00},
    {"Cached FPGA Upgrade Img Header-Rev.ID.Byte 1                ",
     0x25, READ_ONLY,  {BW_8BITS}, 0x00, 0x00},
    {"Cached FPGA Upgrade Img Header-Rev.ID.Byte 2                ",
     0x26, READ_ONLY,  {BW_8BITS}, 0x00, 0x00},
    {"Cached FPGA Upgrade Img Header-Rev.ID.Byte 3(MS Byte)       ",
     0x27, READ_ONLY,  {BW_8BITS}, 0x00, 0x00},
    {"Cached FPGA Upgrade Img Header-Rev.Date.Byte 0(LS Byte)     ",
     0x28, READ_ONLY,  {BW_8BITS}, 0x00, 0x00},
    {"Cached FPGA Upgrade Img Header-Rev.Date.Byte 1              ",
     0x29, READ_ONLY,  {BW_8BITS}, 0x00, 0x00},
    {"Cached FPGA Upgrade Img Header-Rev.Date.Byte 2              ",
     0x2a, READ_ONLY,  {BW_8BITS}, 0x00, 0x00},
    {"Cached FPGA Upgrade Img Header-Rev.Date.Byte 3(MS Byte)     ",
     0x2b, READ_ONLY,  {BW_8BITS}, 0x00, 0x00},
    {"Cached FPGA Upgrade Img Header-Flag.Byte 0(LS Byte)         ",
     0x2c, READ_ONLY,  {BW_8BITS}, 0x00, 0x00},
    {"Cached FPGA Upgrade Img Header-Magic Number.Byte 0(LS Byte) ",
     0x30, READ_ONLY,  {BW_8BITS}, 0x00, 0x00},
    {"Cached FPGA Upgrade Img Header-Magic Number.Byte 1          ",
     0x31, READ_ONLY,  {BW_8BITS}, 0x00, 0x00},
    {"Cached FPGA Upgrade Img Header-Magic Number.Byte 2          ",
     0x32, READ_ONLY,  {BW_8BITS}, 0x00, 0x00},
    {"Cached FPGA Upgrade Img Header-Magic Number.Byte 3(MS Byte) ",
     0x33, READ_ONLY,  {BW_8BITS}, 0x00, 0x00},
    {"FPGA Multiboot State History Reg.Byte 0(LS Byte)            ",
     0x34, READ_ONLY,  {BW_8BITS}, 0x00, 0x00},
    {"FPGA Multiboot State History Reg.Byte 1                     ",
     0x35, READ_ONLY,  {BW_8BITS}, 0x00, 0x00},
    {"FPGA Multiboot State History Reg.Bytte 2                    ",
     0x36, READ_ONLY,  {BW_8BITS}, 0x00, 0x00},
    {"FPGA Multiboot State History Reg.Byte 3(MS Byte)            ",
     0x37, READ_ONLY,  {BW_8BITS}, 0x00, 0x00},
    {"FPGA Secure Boot State History Reg.Byte 0(LS Byte)          ",
     0x38, READ_ONLY,  {BW_8BITS}, 0x00, 0x00},
    {"FPGA Secure Boot State History Reg.Byte 1(MS Byte)          ",
     0x39, READ_ONLY,  {BW_8BITS}, 0x00, 0x00},
    {"FPGA Secure Boot Core Status.Byte 0(LS Byte)                ",
     0x3c, READ_ONLY,  {BW_8BITS}, 0x00, 0x00},
    {"FPGA Secure Boot Core Status.Byte 1                         ",
     0x3d, READ_ONLY,  {BW_8BITS}, 0x00, 0x00},
    {"FPGA Secure Boot Core Status.Byte 2                         ",
     0x3e, READ_ONLY,  {BW_8BITS}, 0x00, 0x00},
    {"FPGA Secure Boot Core Status.Byte 3(MS Byte)                ",
     0x3f, READ_ONLY,  {BW_8BITS}, 0x00, 0x00},
    {"FPGA Secure Boot Core Sign Status.Byte 0(LS Byte)           ",
     0x40, READ_ONLY,  {BW_8BITS}, 0x00, 0x00},
    {"FPGA Secure Boot Core Sign Status.Byte 1                    ",
     0x41, READ_ONLY,  {BW_8BITS}, 0x00, 0x00},
    {"FPGA Secure Boot Core Sign Status.Byte 2                    ",
     0x42, READ_ONLY,  {BW_8BITS}, 0x00, 0x00},
    {"FPGA Secure Boot Core Sign Status.Byte 3(MS Byte)           ",
     0x43, READ_ONLY,  {BW_8BITS}, 0x00, 0x00},
    {"end",                     0xff, 0, {0}, 0x0, 0x0},
};


/***********************************************************************
 * Name:	get_patriot_ip_addr
 *
 * Description:	get Patriot IP address
 *
 * Input:	sgmii port
 *
 * Output:	IP address
 *
 ***********************************************************************
 */
uint get_patriot_ip_addr (int sgmii_port)
{

    return PATRIOT_IP_ADDR;
}


/***********************************************************************
 * Name: patriot_setup_ge_env
 *
 * Description:
 *      This test will set up GE operation environment. 
 *
 * Input: iface - Patriot data structure info pointer
 *
 * Output: PASSED or FAILED.
 *
 ***********************************************************************
 */
int 
patriot_setup_ge_env(patriot_ds_t *iface)
{
    int sgmii_port = 0;
    int status = PASSED;
    char eth_name[IFNAMSIZ];
    
    sgmii_port = get_sgmii_port_num(0, TYPE_SWITCH);

    if (sgmii_port == -1) {
	cterr('f', 0, "Setup: Failed to get sgmii port number.");
	return (FAILED);
    }

    sprintf(eth_name, "eth%d", sgmii_port);

    status = setup_eth_dev(eth_name, &socket_gl);


    if (set_promisc(eth_name, socket_gl) == -1) {
        return(FAILED);
    }
#ifdef DEBUG
    printf("\nsocket_gl = 0x%02x\n", socket_gl);
#endif
    if (status) {
	cterr('f', 0, "Setup: Failed, status = 0x%x", status);
	return (FAILED);
    }
    
    return (PASSED);
}

/***********************************************************************
 * Name: patriot_cleanup_ge_env
 *
 * Description:
 *      This test will clean up the GE operation environment.
 *
 * Input: iface - Patriot data structure info pointer
 *
 * Output: PASSED or FAILED.
 *
 ***********************************************************************
 */
int 
patriot_cleanup_ge_env(patriot_ds_t *iface)
{
    int sgmii_port = 0;
    int status = PASSED;
    char if_name[10];

    sgmii_port = get_sgmii_port_num(0, TYPE_SWITCH);

    if (sgmii_port == -1) {
        cterr('f', 0, "cleanup: Failed to get sgmii port number.");
	return (FAILED);
    }
    sprintf(if_name, "eth%d", sgmii_port);
    status = cleanup_eth_dev(if_name, socket_gl);

    if (status) {
	cterr('f', 0, "cleanup: Failed, status = 0x%x", status);
	return (FAILED);
    }

    return (PASSED);
}

/***********************************************************************
 * Name: patriot_clear_rx_buf
 *
 * Description:
 *      Clear receiver buffer before testing.
 *
 * Input: NONE.
 *
 * Output: NONE.
 *
 ***********************************************************************
*/
void
patriot_clear_rx_buf(void)
{
    uchar *c_ptr = (uchar *)result_packet_p;
    int i;
    
    for (i = 0; i < sizeof(fe_packet_t); i++) {
        *c_ptr++ = 0;
    }
    return;
}


/*
 **********************************************************************
 *
 *  Function: patriot_wait_for_ge_packet
 *
 *  Description: Wait for Ethernet packets
 *
 *  Input: pak - received packet buffer
 *
 *  Returns: PASSED if successful; 
 *           FAILED, otherwise
 *
 **********************************************************************
 */
int
patriot_wait_for_ge_packet(uchar *pak)
{
    int wait_count = 10000;
    eth_rx_pkt_t rx_pkt;
    eth_rx_pkt_t * rx_pkt_p = &rx_pkt;
    int status;
    uchar recv_buffer[1600]; /* need to take largest packet (1518) */

    /* clear buffer before use */
    memset((uchar *)pak, 0, sizeof(fe_packet_t));
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
    /* copy received to user pak */
    memcpy ((char *)pak, (uchar *)recv_buffer, sizeof(fe_packet_t));
    
    if (diagflag_xram & D_SET_OPTIONS) {
        printf("\nRx data :\n");
        dismem((unsigned char *)(pak), 0x40,
            (unsigned long)(pak), 1);
    }
    
    return (PASSED);
}

/***********************************************************************
 * Name: build_eth_header
 *
 * Description: Build the header for an Ethernet frame
 *      An ethernet header is composed of the following:
 *      - 6 bytes of destination address
 *      - 6 bytes of source address
 *      - 2 bytes of type
 *
 * Input: data_ptr   - Points to the buffer
 *        dest_addr  - Points to destination address array
 *        src_addr   - Points to source address array
 *        type       - type
 *
 * Output: None.
 *
 ***********************************************************************
 */
void patriot_build_eth_header (fe_packet_t *framep, mac_addr_t dst_mac_addr,
			       mac_addr_t src_mac_addr, uint16 type)
{
    memcpy((char *)&(framep->eth_hdr.dest_addr), (char *)dst_mac_addr,
           MAC_ADDR_SIZE);
    memcpy((char *)&(framep->eth_hdr.src_addr), (char *)src_mac_addr,
           MAC_ADDR_SIZE);
    framep->eth_hdr.pkt_len = type;
}


/*
 **********************************************************************
 *
 *  Function: patriot_build_lpbk_frame
 *
 *  Description: build the test data frame for ge loopback
 *
 *  Input: frame buffer pointer; desitnation MAC, size, test base value
 *		  and  increment value;
 *
 *  Returns: PASSED if successful;
 *           FAILED, otherwise
 *
 **********************************************************************
 */
int patriot_build_data_frame(fe_packet_t *frame_ptr, mac_addr_t dst_mac_addr,
			     mac_addr_t src_mac_addr, uint16 frm_size,
			     char base_val, char inc_val, char lpbk_op)
{
    uint16 data_len, count;
    uchar data;
    uchar *datap;
    fe_packet_t *framep;

    /* build ethernet frame header */
    framep = (fe_packet_t *)frame_ptr;

    data_len = frm_size - sizeof(ether_hdr_t) - 4; /* size of real data */
    /* build ethernet frame payload */
    data = base_val;
    /* First 2 bytes for type and loopback option */
    framep->data[0] = PATRIOT_DATA;
    framep->data[1] = lpbk_op;
    framep->data[2] = (uchar)((data_len & 0xFF00) >> 8); /* MSB */
    framep->data[3] = (uchar)(data_len & 0xFF); /* LSB */
    datap = (uchar *)&framep->data[4];
    
    for (count = 0; count < data_len; count++) {
        *datap++ = data;
        data += inc_val;
    }

    return (PASSED);
}

/*
 **********************************************************************
 *
 *  Function: patriot_check_rx_frame
 *
 *  Description: Check the recevied frame test data 
 *
 *  Input: test frame pointer; recv frame pointer; packet number and size  
 *
 *  Returns: PASSED if successful;
 *           FAILED, otherwise
 *
 **********************************************************************
 */
int
patriot_check_rx_frame (volatile fe_packet_t * test_frame_p,
			volatile uchar * recv_frame_p,
			int packet_num, uint32 frm_size, uchar lpbk_op)
{
    ushort rx_size, tx_size;
    int count, error = 0;
    uchar *rd_ptr, *wr_ptr;
    volatile fe_packet_t *rx_frame_p;

    rx_frame_p = (fe_packet_t *)recv_frame_p;
    /*
     * verify that we received the correct number of bytes
     * the byte count in tx_bd->length does not include 4 bytes of CRC
     * while the byte count in rx_bd->length does include it
     */
    if (rx_frame_p->data[0] != PATRIOT_DATA) {
	cterr('f', 0, "Wrong type, must be data type");
	return (FAILED);
    }

    if (rx_frame_p->data[1] != lpbk_op) {
	cterr('f', 0, "Wrong type of loopback option, expect %d, receive %d",
	      lpbk_op, rx_frame_p->data[1]);
	return (FAILED);
    }

    rx_size = ((rx_frame_p->data[2]) << 8) | rx_frame_p->data[3];
    tx_size = ((test_frame_p->data[2]) << 8) | test_frame_p->data[3];

    if (rx_size != tx_size) {
	cterr('f', 0, "Wrong rx data length, tx_size = 0x%04x, rx_size = 0x%04x",
	      rx_size, tx_size);
	return (FAILED);
    }

    rd_ptr = (uchar *)(&rx_frame_p->data[4]);
    wr_ptr = (uchar *)(&test_frame_p->data[4]);
    
#ifdef DEBUG
    printf("\nTx data: ");
    dismem((unsigned char *)(wr_ptr), frm_size,
            (unsigned)(wr_ptr), 4);

    printf("\nRx data: ");
    dismem((unsigned char *)(rd_ptr), frm_size,
            (unsigned)(rd_ptr), 4);
#endif
    
    for (count = 0; count < (frm_size - sizeof(ether_hdr_t) - 4); count++) {
	if (*rd_ptr != *wr_ptr) {
	    cterr('f', 0, "Packet%d data mismatch at offset %#x, "
		  "sent %#.8x, rcvd %#.8x\ntx bd @%#.8x, rx bd @%#.8x",
		  packet_num, count, *wr_ptr, *rd_ptr, test_frame_p,
		  recv_frame_p);
	    error = FAILED;
	    break;
	}
	rd_ptr++;
	wr_ptr++;
    }

    return (error);
}

/*
 **********************************************************************
 *
 *  Function: patriot_eth_frames_test
 *
 *  Description: This function sends a packet with data to
 *               Patriot SM
 *
 *  Input: iface - Patriot data structure info pointer
 *
 *  Returns: PASSED/FAILED 
 *
 **********************************************************************
 */
int
patriot_eth_frames_test (patriot_ds_t *iface, uint32 frame_num, uint32 frm_size,
			 mac_addr_t dst_mac_addr, int lpbk_op)
{

    char       base_val, inc_val;
    int        result = PASSED, wait_time = 0;
    fe_packet_t test_frame;
    fe_packet_t *test_frame_p = &test_frame;
    uchar recv_frame_p[2048];
    eth_tx_pkt_t tx_pkt;
    eth_tx_pkt_t *tx_pkt_p = &tx_pkt;
    
    memset((uchar *)test_frame_p, 0, sizeof(fe_packet_t));
    memset((uchar *)recv_frame_p, 0, sizeof(fe_packet_t));
    
    /* make ethernet frame, data pat used dependent on odd/even frame size */
    if (frm_size & 1) {
        base_val = 0;
        inc_val = 1;
    } else {
        base_val = 0xff;
        inc_val = -1;
    }

    /* make ethernet frame, data pat dependent on odd/even frame size */
    if (iface->slot == FIRST_SLOT) {
	if (patriot_build_data_frame(test_frame_p, patriot_sm1_mac, host_sm1_mac,
				     frm_size, base_val, inc_val, lpbk_op)
	    == FAILED) {
	    return (FAILED);
	}
    } else {
	if (patriot_build_data_frame(test_frame_p, patriot_sm2_mac, host_sm2_mac,
				     frm_size, base_val, inc_val, lpbk_op)
	    == FAILED) {
	    return (FAILED);
	}
    }
    memset((uchar *)tx_pkt_p, 0, sizeof(eth_tx_pkt_t));

    if (iface->slot == FIRST_SLOT) {
	memcpy((uchar *)(tx_pkt_p->dest_addr), (uchar *)patriot_sm1_mac,
	       sizeof(mac_addr_t));
	memcpy((uchar *)(tx_pkt_p->src_addr), (uchar *)host_sm1_mac,
	   sizeof(mac_addr_t));
    } else {
	memcpy((uchar *)(tx_pkt_p->dest_addr), (uchar *)patriot_sm2_mac,
	       sizeof(mac_addr_t));
	memcpy((uchar *)(tx_pkt_p->src_addr), (uchar *)host_sm2_mac,
	   sizeof(mac_addr_t));
    }
    
    tx_pkt_p->pkt_type = frm_size;
    
    tx_pkt_p->payload_size = frm_size - sizeof(ether_hdr_t); /* payload size */
    tx_pkt_p->bufr_st_addr = (uchar *)&(test_frame_p->data[0]); /* payload */
    tx_pkt_p->pkt_num = 0;
    tx_pkt_p->socket = socket_gl;

#ifdef DEBUG    
    printf("\ntx_pkt_p->pkt_type = 0x%04x", tx_pkt_p->pkt_type);
    dismem((uchar *)tx_pkt_p, 0x40, (ulong)tx_pkt_p, BW_32BITS);
#endif    
    
    result = eth_pkt_tx(tx_pkt_p);
    if (result != ETH_PKT_TX_OK ) {
	cterr('f', 0, "%s: Failed lpbk test tx : ret = 0x%x, status = 0x%x",
	      __FUNCTION__, result, tx_pkt_p->tx_status);
        return (FAILED);
    }

    wait_time = 600;

    while ((result = patriot_wait_for_ge_packet(recv_frame_p)) == FAILED) {
        if (--wait_time <= 0) {
	    cterr('f', 0, "Timeout on receiving frame%d \n", frame_num);
	    return (FAILED);
        }
        msleep(1);
    }
    
    if (result == PASSED) {
	result = patriot_check_rx_frame (test_frame_p, recv_frame_p, 
					 frame_num, frm_size, lpbk_op);
    }
    
    return (result);
    
}


/*
 **********************************************************************
 *
 *  Function: patriot_send_test_data_packets
 *
 *  Description: This function sends packets with test data to
 *               Patriot SM
 *
 *  Input: iface - Patriot data structure info pointer
 *
 *  Returns: PASSED/FAILED 
 *
 **********************************************************************
 */
int patriot_send_test_data_packets (patriot_ds_t *iface, int lpbk_op)
{

    int retval = PASSED;
    uint32 frame_num, frm_size;
    mac_addr_t mac_da;

    unsigned short pak_size[10] = {64, 108, 512, 256,
				   1490, 65, 1411, 128,
				   66, 719};
    if (iface->slot == FIRST_SLOT) {
	memcpy((uchar *)mac_da, patriot_sm1_mac, sizeof(mac_addr_t));
    } else {
	memcpy((uchar *)mac_da, patriot_sm2_mac, sizeof(mac_addr_t));	
    }

    for (frame_num = 0; frame_num < 10; frame_num++) {
        frm_size = pak_size[frame_num];
        prpass(testpass,"loopback test, frame%d, size %d",
               frame_num, frm_size);
        if (patriot_eth_frames_test(iface, frame_num, frm_size,
                		mac_da, lpbk_op)) {
            retval = FAILED;
            break;
        }
    }

    return(retval);
    
}

/*
 **********************************************************************
 *
 *  Function: patriot_send_data_packet
 *
 *  Description: This function sends a packet with data to
 *               Patriot SM used for download FPGA and FW
 *
 *  Input: iface - Patriot data structure info pointer
 *         data_ptr - pointer to data to send
 *         blk_size - size of data packeet
 *
 *  Returns: PASSED/FAILED 
 *
 **********************************************************************
 */
int patriot_send_data_packet (patriot_ds_t *iface, uchar *data_ptr,
			      int blk_size)
{
    
    ushort rx_size, tx_size;
    int        result = PASSED, wait_time = 0;
    fe_packet_t data_frame;
    fe_packet_t *data_frame_p = &data_frame;
    uchar recv_frame_p[2048];
    eth_tx_pkt_t tx_pkt;
    eth_tx_pkt_t *tx_pkt_p = &tx_pkt;
    volatile fe_packet_t *rx_frame_p;
    uchar *datap;
    
    memset((uchar *)data_frame_p, 0, sizeof(fe_packet_t));
    memset((uchar *)recv_frame_p, 0, sizeof(fe_packet_t));    

    if (iface->slot == FIRST_SLOT) {
	patriot_build_eth_header(data_frame_p, patriot_sm1_mac, host_sm1_mac,
				 blk_size);
    } else {
	patriot_build_eth_header(data_frame_p, patriot_sm2_mac, host_sm2_mac,
				 blk_size);
    }
    /* build ethernet frame payload */
    /* First 2 bytes for type and loopback option, second 2 bytes for size */
    data_frame_p->data[0] = PATRIOT_DATA;
    data_frame_p->data[1] = PATRIOT_CPU_PASS; 
    data_frame_p->data[2] = (uchar)((blk_size & 0xFF00) >> 8); /* MSB */
    data_frame_p->data[3] = (uchar)(blk_size & 0xFF); /* LSB */
    
    datap = (uchar *)&data_frame_p->data[4];

    memcpy((uchar *)(datap), (uchar *)data_ptr, blk_size);

    memset((uchar *)tx_pkt_p, 0, sizeof(eth_tx_pkt_t));

    if (iface->slot == FIRST_SLOT) {
	memcpy((uchar *)(tx_pkt_p->dest_addr), (uchar *)patriot_sm1_mac,
	       sizeof(mac_addr_t));
	memcpy((uchar *)(tx_pkt_p->src_addr), (uchar *)host_sm1_mac,
	   sizeof(mac_addr_t));
    } else {
	memcpy((uchar *)(tx_pkt_p->dest_addr), (uchar *)patriot_sm2_mac,
	       sizeof(mac_addr_t));
	memcpy((uchar *)(tx_pkt_p->src_addr), (uchar *)host_sm2_mac,
	   sizeof(mac_addr_t));
    }
    
    tx_pkt_p->pkt_type = blk_size + 4;  /* 4 bytes info */
    tx_pkt_p->payload_size = blk_size;  /* 4 bytes info */
    tx_pkt_p->bufr_st_addr = (uchar *)&(data_frame_p->data); /* payload */
    tx_pkt_p->pkt_num = 0;
    tx_pkt_p->socket = socket_gl;
    result = eth_pkt_tx(tx_pkt_p);
    
    if (result != ETH_PKT_TX_OK ) {
	cterr('f', 0, "%s: Failed lpbk test tx : ret = 0x%x, status = 0x%x",
	      __FUNCTION__, result, tx_pkt_p->tx_status);
        return (FAILED);
    }

    wait_time = 600;

    while ((result = patriot_wait_for_ge_packet(recv_frame_p)) == FAILED) {
        if (--wait_time <= 0) {
	    cterr('f', 0, "Timeout on receiving the response\n");
	    return (FAILED);
        }
        msleep(1);
    }

    rx_frame_p = (fe_packet_t *)recv_frame_p;
    /*
     * verify that we received the correct number of bytes
     * the byte count in tx_bd->length does not include 4 bytes of CRC
     * while the byte count in rx_bd->length does include it
     */
    if (rx_frame_p->data[0] != PATRIOT_DATA) {
	cterr('f', 0, "Wrong type, must be data type");
	return (FAILED);
    }

    if (rx_frame_p->data[1] != PATRIOT_CPU_PASS) {
	cterr('f', 0, "Wrong type of loopback option, expect %d, receive %d",
	      PATRIOT_CPU_PASS, rx_frame_p->data[1]);
	return (FAILED);
    }
    
    rx_size = ((rx_frame_p->data[2]) << 8) | rx_frame_p->data[3];
    tx_size = ((data_frame_p->data[2]) << 8) | data_frame_p->data[3];

    if (rx_size != tx_size) {
	cterr('f', 0, "Wrong rx data length, tx_size = 0x%04x, rx_size = 0x%04x",
	      rx_size, tx_size);
	return (FAILED);
    }

    return (PASSED);

}


/*
 **********************************************************************
 *
 *  Function: mac_compare
 *
 *  Description: This function compare 2 mac address, if they're
 *               the same, return TRUE, otherwise, return FALSE
 *
 *  Input: mac1, mac2
 *
 *  Returns: TRUE/FALSE 
 *
 **********************************************************************
 */
boolean
mac_check(mac_addr_t mac, int sm_slot)
{
    int i;
    if (sm_slot == FIRST_SLOT) {
	for (i = 0; i < sizeof(mac_addr_t); i++) {
	    if (mac[i] !=  patriot_sm1_mac[i]) {
		return (FALSE);
	    }
	}
    } else {
	for (i = 0; i < sizeof(mac_addr_t); i++) {
	    if (mac[i] !=  patriot_sm2_mac[i]) {
		return (FALSE);
	    }
	}
    }
    return TRUE;
}


/*
 **********************************************************************
 *
 *  Function: patriot_rcv_cmd_result_packet_for_ge0_lpbk
 *
 *  Description: This function wait for the result packet after
 *               the command has been sent for GE0 loopback test
 *
 *  Input: iface - Patriot data structure info pointer
 *         cmd - command
 *
 *  Returns: PASSED/FAILED 
 *
 **********************************************************************
 */
int
patriot_rcv_cmd_result_packet_for_ge0_lpbk(patriot_ds_t *iface, uchar cmd)
{
    uchar recv_frame_p[2048];   
    int ret_val = PASSED, wait_time = 0, retry;

    
    memset((uchar *)recv_frame_p, 0, sizeof(fe_packet_t));
        
    wait_time = 600;

    recv_packet_p = (fe_packet_t *)recv_frame_p;
    
    for (retry = 0; retry < 3; retry++) {
	while ((ret_val = patriot_wait_for_ge_packet(recv_frame_p)) == FAILED ||
	       (mac_check(recv_packet_p->eth_hdr.src_addr, iface->slot) == FALSE)){
	    if (--wait_time <= 0) {
		cterr('f', 0, "Timeout on receiving frame\n");
		return (FAILED);
	    }
	    msleep(100);
	}
	
#ifdef DEBUG    
	printf("\nReceive packet, recv_packet_p->data[1] = 0x%02x\n",
	       recv_packet_p->data[1]);
#endif    
	if (ret_val == PASSED) {
	    if (recv_packet_p->data[1] == cmd + TEST_OK) {
		break;
	    }
	}
    }
    if (retry == 3) {
	display_err_msg();
	cterr('f', 0,"Wrong response, expected = 0x%02x, received = 0x%02x",
	      cmd + TEST_OK, recv_packet_p->data[1]);
	return (FAILED);
    }
    return (PASSED);

}


/*
 **********************************************************************
 *
 *  Function: patriot_rcv_cmd_result_packet
 *
 *  Description: This function wait for the result packet after
 *               the command has been sent
 *
 *  Input: iface - Patriot data structure info pointer
 *         cmd - command
 *
 *  Returns: PASSED/FAILED 
 *
 **********************************************************************
 */
int
patriot_rcv_cmd_result_packet(patriot_ds_t *iface, uchar cmd)
{
    uchar recv_frame_p[2048];   
    int ret_val = PASSED, wait_time = 0;

    
    memset((uchar *)recv_frame_p, 0, sizeof(fe_packet_t));
        
    if ((cmd == FROM_HOST_MEMORY_TEST) || (cmd == FROM_HOST_SPI_PROM_TEST)) {
	wait_time = 1200;
    } else {
	wait_time = 600;
    }

    recv_packet_p = (fe_packet_t *)recv_frame_p;

    while ((ret_val = patriot_wait_for_ge_packet(recv_frame_p)) == FAILED ||
	   (mac_check(recv_packet_p->eth_hdr.src_addr, iface->slot) == FALSE)){
        if (--wait_time <= 0) {
	    cterr('f', 0, "Timeout on receiving frame\n");
	    return (FAILED);
        }
        msleep(100);
    }
    
#ifdef DEBUG    
    printf("\nReceive packet, recv_packet_p->data[1] = 0x%02x\n",
	   recv_packet_p->data[1]);
#endif    
    if (ret_val == PASSED) {
	if (recv_packet_p->data[1] != cmd + TEST_OK) {
	    display_err_msg();
	    cterr('f', 0,"Wrong response, expected = 0x%02x, received = 0x%02x",
		  cmd + TEST_OK, recv_packet_p->data[1]);
	    return (FAILED);
	}
    }
    return (PASSED);

}


/*
 **********************************************************************
 *
 *  Function: patriot_send_command_packet
 *
 *  Description: This function sends a packet with the command to
 *               Patriot SM
 *
 *  Input: iface - Patriot data structure info pointer
 *         cmd - command
 *
 *  Returns: PASSED/FAILED 
 *
 **********************************************************************
 */
int patriot_send_command_packet (patriot_ds_t *iface, uchar cmd, int param)
{

    eth_tx_pkt_t tx_pkt;
    eth_tx_pkt_t *tx_pkt_p = &tx_pkt;
    uchar recv_frame_p[2048] = {0};
    int ret_val = PASSED,wait_time = 0;
    ushort pkt_type = 0x0800;
    
    memset((uchar *)recv_frame_p, 0, sizeof(fe_packet_t));
    memset((uchar *)cmd_packet_p, 0, sizeof(fe_packet_t));
    switch (cmd) {
    	case FROM_HOST_SUB_T3_IND_LPBK_TEST :
    	case FROM_HOST_SUB_T3_IND_EX_LPBK_TEST:
    	case FROM_HOST_SUB_E3_IND_LPBK_TEST :
    	case FROM_HOST_SUB_E3_IND_EX_LPBK_TEST:
	    cmd_packet_p->data[0] = PATRIOT_CMD;
	    cmd_packet_p->data[1] = cmd_param[1];
	    cmd_packet_p->data[2] = cmd_param[2];
	    cmd_packet_p->data[3] = cmd_param[3];
	    break;
    	case FROM_HOST_WRITE_MAC_ADDR :
	    /* Do Nothing in here for MAC addr since cmd_packed_p array already
	     * fill it in the patriot_write_mac_addr() */
	    break;
    default:
	cmd_packet_p->data[0] = PATRIOT_CMD;
	cmd_packet_p->data[1] = cmd; /* Juset set the command to [1] for now */
	cmd_packet_p->data[2] = (uchar)((param & 0xFF000000) >> 24);
	cmd_packet_p->data[3] = (uchar)((param & 0x00FF0000) >> 16);
	cmd_packet_p->data[4] = (uchar)((param & 0x0000FF00) >> 8);
	cmd_packet_p->data[5] = (uchar)(param & 0x000000FF);
	break;
    }
#ifdef DEBUG
    printf("\ncmd_packet_p->data[0] = 0x%02x\n", cmd_packet_p->data[0]);
    printf("\ncmd_packet_p->data[1] = 0x%02x\n", cmd_packet_p->data[1]);
    printf("\ncmd_packet_p->data[2] = 0x%02x\n", cmd_packet_p->data[2]);
    printf("\ncmd_packet_p->data[3] = 0x%02x\n", cmd_packet_p->data[3]);
#endif
    memset((uchar *)tx_pkt_p, 0, sizeof(eth_tx_pkt_t));

    if (iface->slot == FIRST_SLOT) {
	memcpy((uchar *)(tx_pkt_p->dest_addr), (uchar *)patriot_sm1_mac,
	       sizeof(mac_addr_t));
	memcpy((uchar *)(tx_pkt_p->src_addr), (uchar *)host_sm1_mac,
	   sizeof(mac_addr_t));
    } else {
	memcpy((uchar *)(tx_pkt_p->dest_addr), (uchar *)patriot_sm2_mac,
	       sizeof(mac_addr_t));
	memcpy((uchar *)(tx_pkt_p->src_addr), (uchar *)host_sm2_mac,
	   sizeof(mac_addr_t));
    }
    
    tx_pkt_p->pkt_type = pkt_type;
    tx_pkt_p->payload_size = sizeof(fe_packet_t) - sizeof(ether_hdr_t) - 4;
    tx_pkt_p->bufr_st_addr = (uchar *)&cmd_packet_p->data[0];
    tx_pkt_p->pkt_num = 0;
    tx_pkt_p->socket = socket_gl;
#ifdef DEBUG
    printf("\ntx_pkt_p->pkt_type = 0x%04x", tx_pkt_p->pkt_type);
    printf("\ntx_pkt_p->tx_status  = 0x%04x", tx_pkt_p->tx_status);
    printf("\ntx_pkt_p->payload_size = 0x%04x", tx_pkt_p->payload_size);
    printf("\ntx_pkt_p->bufr_st_addr = 0x%08x", tx_pkt_p->bufr_st_addr);
    dismem((uchar *)tx_pkt_p, 0x40, (ulong)tx_pkt_p, BW_32BITS);
#endif
    ret_val = eth_pkt_tx(tx_pkt_p);
    
    if (ret_val != ETH_PKT_TX_OK ) {
	cterr('f', 0, "%s: Failed send command : ret = 0x%x, status = 0x%x",
	      __FUNCTION__, ret_val, tx_pkt_p->tx_status);
        return (FAILED);
    }
    
    if (cmd == FROM_HOST_SWITCH_CONSOLE) {
	return (PASSED);
    }

    wait_time = 600;

    recv_packet_p = (fe_packet_t *)recv_frame_p;

    while ((ret_val = patriot_wait_for_ge_packet(recv_frame_p)) == FAILED ||
	   (mac_check(recv_packet_p->eth_hdr.src_addr, iface->slot) == FALSE)){
        if (--wait_time <= 0) {
	    cterr('f', 0, "Timeout on receiving frame, wait_time = %d\n",
		  wait_time);
	    return (FAILED);
        }
        msleep(10);
    }

    if (ret_val == PASSED) {
	if (recv_packet_p->data[1] != cmd_packet_p->data[1] + TEST_ACK) {
	    cterr('f', 0,"Wrong ACK, expected = 0x%02x, received = 0x%02x",
		  cmd_packet_p->data[1] + TEST_ACK, recv_packet_p->data[1]);
	    return (FAILED);
	}
    }

    return (PASSED);
}


/**********************************************************************
 *
 * Function: patriot_send_cmd
 *
 * This function sends commands to the Patriot SM
 *
 * Input : iface - Patriot data structure info pointer
 *         cmd - command
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
int
patriot_send_cmd(patriot_ds_t *iface, uchar cmd, int param)
{
    int retval = PASSED;

    
    if (patriot_send_command_packet(iface, cmd, param)) {
	retval = FAILED;
    }
    
    return retval;
}



/**********************************************************************
 *
 * Function: patriot_cpu_fw_download_util
 *
 * This function download the Linux kernel and boot up Freescale CPU
 *
 * Input : iface - Patriot data structure info pointer
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
int
patriot_cpu_fw_download_util(patriot_ds_t *iface)
{
    int i;
    
    if (configure_ltc4215_and_io_port(iface)) {
	return (FAILED);
    }

    printf("\nPlease wait for the kernel to boot up and download FPGA ");
    for (i = 0; i < BOOTUP_TIME; i++) {
	printf(" .");
	msleep(1000);
	
    }

    
    return (PASSED);

}


/**********************************************************************
 *
 * Function: patriot_fpga_download_to_fpga
 * This function download FPGA fw from module DDR to FPGA chip
 *
 * Input : iface - Patriot data structure info pointer
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
int
patriot_fpga_download_to_fpga(patriot_ds_t *iface)
{
    int retval = PASSED;
    
    if (patriot_setup_ge_env(iface) == FAILED) {
        return (FAILED);
    }

    patriot_clear_rx_buf();

    if (patriot_send_cmd(iface, FROM_HOST_FPGA_DOWNLOAD_TO_FPGA, 0)) {
	retval = FAILED;
    }

    if (retval == PASSED) {
	if (patriot_rcv_cmd_result_packet(iface,
					  FROM_HOST_FPGA_DOWNLOAD_TO_FPGA)) {
	    retval = FAILED;
	}
    }

    if (patriot_cleanup_ge_env(iface) == FAILED) {
	retval = FAILED;
    }
    
    return(retval);
    
}



/**********************************************************************
 *
 * Function: patriot_fpga_download
 *
 * This function downloads FPGA
 *
 * Input : iface - Patriot data structure info pointer
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
int
patriot_fpga_download(patriot_ds_t *iface)
{

    /* Then download FPGA from module DDR mem to FPGA chip */
    if (patriot_fpga_download_to_fpga(iface)) {
	return (FAILED);
	
    }

    iface->fpga_downloaded[iface->slot] = TRUE;
    return (PASSED);
}


/**********************************************************************
 *
 * Function: patriot_diplay_led
 *
 * This function display LED's
 *
 * Input : iface - Patriot data structure info pointer
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
int
patriot_diplay_led(patriot_ds_t *iface)
{

    int retval = PASSED;
    prpass(testpass, "Display LED");

    if (iface->fpga_downloaded[iface->slot] != TRUE) {
	if (patriot_fpga_download(iface)) {
	    return (FAILED);
	}
    }
    
    if (patriot_setup_ge_env(iface) == FAILED) {
        return (FAILED);
    }

    patriot_clear_rx_buf();

    if (patriot_send_cmd(iface, FROM_HOST_LED_DISPLAY, 0)) {
	retval = FAILED;
    }

    if (retval == PASSED) {
	if (patriot_rcv_cmd_result_packet(iface,
					  FROM_HOST_LED_DISPLAY)) {
	    retval = FAILED;
	}
    }

    if (patriot_cleanup_ge_env(iface) == FAILED) {
	retval = FAILED;
    }
    
    return(retval);
    
}


/**********************************************************************
 *
 * Function: patriot_sm_reset
 *
 * This function reset and unrerset the Patriot SM
 *
 * Input : iface - Patriot data structure info pointer
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
int
patriot_sm_reset(patriot_ds_t *iface)
{
    struct ngio_intf_t *ngio;
    
    prpass(testpass, "Patriot SM reset");

    ngio = (struct ngio_intf_t *)slot_get_ngiosm(iface->slot);
    
    /* Pull reset then release the reset*/
    assert(ngio);
    assert(ngio->slot);
    ngio->reset(ngio);
    //    ngiosm_reset(iface->slot);
    msleep(1000);

    if ((ngio->on(ngio)) < 0) {
        cterr('f', 0, "Unable to power module");
        return FAILED;
    }
    if ((ngio->i2c_unreset(ngio)) < 0) {
        cterr('f', 0, "Unable to unreset i2c");
        return FAILED;
    }
    ngio->unreset(ngio);
    msleep(1000);
    
    return(PASSED);
}


/**********************************************************************
 *
 * Function: patriot_cpu_reset
 *
 * This function reset the Patriot CPU
 *
 * Input : iface - Patriot data structure info pointer
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
int
patriot_cpu_reset(patriot_ds_t *iface)
{

    patriot_sm_reset(iface);

    return(PASSED);
}

/**********************************************************************
 *
 * Function: patriot_ds3170_reset
 *
 * This function reset the Patriot Maxim DS3170
 *
 * Input : iface - Patriot data structure info pointer
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
int
patriot_ds3170_reset(patriot_ds_t *iface)
{
    int retval = PASSED;

    if (patriot_setup_ge_env(iface) == FAILED) {
        return (FAILED);
    }    

    patriot_clear_rx_buf();
    
    if (patriot_send_cmd(iface, FROM_HOST_DS3170_RESET, 0)) {
	retval = FAILED;
    }

    if (retval == PASSED) {
	if (patriot_rcv_cmd_result_packet(iface, FROM_HOST_DS3170_RESET)) {
	    retval = FAILED;
	}    
    }

    if (patriot_cleanup_ge_env(iface) == FAILED) {
	retval = FAILED;
    }
    
    
    return retval;
}


/**********************************************************************
 *
 * Function: patriot_fpga_reset
 *
 * This function reset the Patriot FPGA
 *
 * Input : iface - Patriot data structure info pointer
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
int
patriot_fpga_reset(patriot_ds_t *iface)
{
    int retval = PASSED;

    if (patriot_setup_ge_env(iface) == FAILED) {
        return (FAILED);
    }    

    patriot_clear_rx_buf();
    
    if (patriot_send_cmd(iface, FROM_HOST_FPGA_RESET, 0)) {
	retval = FAILED;
    }

    if (retval == PASSED) {
	if (patriot_rcv_cmd_result_packet(iface, FROM_HOST_FPGA_RESET)) {
	    retval = FAILED;
	}    
    }

    if (patriot_cleanup_ge_env(iface) == FAILED) {
	retval = FAILED;
    }
        
    return retval;    

}



/**********************************************************************
 *
 * Function: patriot_switch_console
 *
 * This function switches the console to the module side
 *
 * Input : iface - Patriot data structure info pointer
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
int
patriot_switch_console(patriot_ds_t *iface)
{
    const int maxlen = 128;
    char cmd[maxlen];

    prpass(testpass, "Switch console");

    /* Send the command to break the infinite loop
       in Patriot module side */
       
    if (patriot_setup_ge_env(iface) == FAILED) {
        return (FAILED);
    }    

    patriot_clear_rx_buf();
    
    if (patriot_send_cmd(iface, FROM_HOST_SWITCH_CONSOLE, 0)) {
	return (FAILED);
    }

    if (patriot_cleanup_ge_env(iface) == FAILED) {
	return (FAILED);
    }
 
    snprintf(cmd, maxlen-1, "picocom -b9600 -d8 -pn -fn /dev/ttyDASH%d",
             iface->uart); 
    system(cmd); 
    
    return(PASSED);
}


/**********************************************************************
 *
 * Function: patriot_i2c_port_reg_read
 *
 * This function reads a register on I2C IO port chip
 *
 * Input : iface - Patriot data structure info pointer
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
int
patriot_i2c_port_reg_read(patriot_ds_t *iface)
{

    uchar offset = 0, value = 0;
    n2g_i2c_if_t *pca1;
    pca1 = &pca_i2c[1];
    
    offset = getdec_answer("Read I2C Port Register Offset: ", 0, 0, 3);

    if (io_port_8bit_i2c_read(pca1, offset, &value, FALSE)) {
	return (FAILED);
    }

    printf("\nI2C Port Register Value: 0x%02x", value);

    return (PASSED);

}

/**********************************************************************
 *
 * Function: patriot_i2c_port_reg_write
 *
 * This function writes a value to a register on I2C IO port chip
 *
 * Input : iface - Patriot data structure info pointer
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
int
patriot_i2c_port_reg_write(patriot_ds_t *iface)
{

    uchar offset = 0, value = 0;
    n2g_i2c_if_t *pca1;
    pca1 = &pca_i2c[1];
    
    offset = getdec_answer("Write I2C Port Register Offset: ", 0, 0, 3);

    value = gethex_answer("Write I2C Port Register Offset value: ", 0, 0, 0xFF);

    if (io_port_8bit_i2c_write(pca1, offset, &value)) {
	return (FAILED);
    }

    return (PASSED);

}


/**********************************************************************
 *
 * Function: patriot_write_mac_addr
 *
 * This function writes the MAC address from cookie to the Patriot
 * SM SPI EEPROM
 *
 * Input : iface - Patriot data structure info pointer
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
int
patriot_write_mac_addr(patriot_ds_t *iface)
{
    int retval = PASSED;
    uchar mac_addr[6] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00};


    if (get_sm_mac_addr (iface->slot, &mac_addr[0])) {
	cterr('f', 0, "Failed to get board MAC address");
	return (FAILED);
    }

    printf("\nMAC addr: %02x %02x %02x %02x %02x %02x", mac_addr[0],
	   mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);

    cmd_packet_p->data[0] = PATRIOT_CMD;
    cmd_packet_p->data[1] = FROM_HOST_WRITE_MAC_ADDR;
    cmd_packet_p->data[2] = mac_addr[0];
    cmd_packet_p->data[3] = mac_addr[1];
    cmd_packet_p->data[4] = mac_addr[2];
    cmd_packet_p->data[5] = mac_addr[3];
    cmd_packet_p->data[6] = mac_addr[4];
    cmd_packet_p->data[7] = mac_addr[5];

    if (patriot_setup_ge_env(iface) == FAILED) {
        return (FAILED);
    }

    patriot_clear_rx_buf();
    
    if (patriot_send_cmd(iface, FROM_HOST_WRITE_MAC_ADDR, 0)) {
	retval = FAILED;
    }

    if (retval == PASSED) {
	if (patriot_rcv_cmd_result_packet(iface, FROM_HOST_WRITE_MAC_ADDR)) {
	    retval = FAILED;
	}
    }

    if (patriot_cleanup_ge_env(iface) == FAILED) {
	retval = FAILED;
    }
    
    return(retval);

}


/**********************************************************************
 *
 * Function: patriot_display_fpga_version
 * This function displays the FPGA version
 *
 * Input : iface - Patriot data structure info pointer
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
int
patriot_display_fpga_version(patriot_ds_t *iface)
{
    int retval = PASSED;
    uchar fpga_ver;
    
    prpass(testpass, "Display FPGA Version test");

    if (iface->fpga_downloaded[iface->slot] != TRUE) {
	if (patriot_fpga_download(iface)) {
	    return (FAILED);
	}
    }
    
    if (patriot_setup_ge_env(iface) == FAILED) {
        return (FAILED);
    }

    patriot_clear_rx_buf();
    
    if (patriot_send_cmd(iface, FROM_HOST_READ_FPGA_VERSION, 0)) {
	retval = FAILED;
    }

    if (retval == PASSED) {
	if (patriot_rcv_cmd_result_packet(iface, FROM_HOST_READ_FPGA_VERSION)) {
	    retval = FAILED;
	}
    }
    fpga_ver = recv_packet_p->data[2];
    if ((fpga_ver == 0) || (fpga_ver == 0xff)) {
	cterr('f', 0, "FPGA version = 0x%02x, suspect FPGA download failure",
	      fpga_ver);
	retval = FAILED;
    } else {
	printf("\nFPGA Version : 0x%02x\n", fpga_ver);
    }
    
    if (patriot_cleanup_ge_env(iface) == FAILED) {
	retval = FAILED;
    }
    
    return(retval);
    
}

/**********************************************************************
 *
 * Function: patriot_config_power_margin
 * This function alter power margin
 *
 * Input : iface - Patriot data structure info pointer
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
int
patriot_config_power_margin(patriot_ds_t *iface)
{
    int retval = PASSED;
    uchar i = 0, cmd = 0x0;

    prpass(testpass, "Configure Power Margin");

    if (iface->fpga_downloaded[iface->slot] != TRUE) {
	if (patriot_fpga_download(iface)) {
	    return (FAILED);
	}
    }

    /* select no margin, low margin, high margin */
    printf("\n");
    i = gethex_answer("Set Power: 0-No Margin, 1-Low Margin, 2-High Margin,"
                       , 0, 0, 0x2);
    switch (i) {
       case 0x0:
    	   cmd = FROM_HOST_POWER_ALTER_NO_MARGIN;
    	   printf("\n No Power Margin \n");
    	   break;
       case 0x1:
    	   cmd = FROM_HOST_POWER_ALTER_LOW_MARGIN;
    	   printf("\n Low Power Margin \n");
    	   break;
       case 0x2:
    	   cmd = FROM_HOST_POWER_ALTER_HIGH_MARGIN;
    	   printf("\n High Power Margin \n");
    	   break;
       default:
    	   break;
    }

    if (patriot_setup_ge_env(iface) == FAILED) {
        return (FAILED);
    }

    patriot_clear_rx_buf();

    if (patriot_send_cmd(iface, cmd, 0)) {
	retval = FAILED;
    }

    if (retval == PASSED) {
	if (patriot_rcv_cmd_result_packet(iface, cmd)) {
	    retval = FAILED;
	}
    }

    if (patriot_cleanup_ge_env(iface) == FAILED) {
	retval = FAILED;
    }

    return(retval);

}

/**********************************************************************
 *
 * Function: patriot_upgrade_fpga_download_spi_prom
 *
 * This function download the Patriot upgrade FPGA to the FPGA SPI PROM
 *
 * Input : iface - Patriot data structure info pointer
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
int
patriot_upgrade_fpga_download_spi_prom(patriot_ds_t *iface)
{
    int retval = PASSED, i;


    if (patriot_setup_ge_env(iface) == FAILED) {
        return (FAILED);
    }    

    patriot_clear_rx_buf();
    
    if (patriot_send_cmd(iface, FROM_HOST_UPGRADE_FPGA_DOWNLOAD_TO_SPI_PROM, 0)) {
	retval = FAILED;
    }

    /* Add 3 minutes of delay here because the FPGA download takes long */
    printf("\nPlease wait for the FPGA download ");
    for (i = 0; i < 180; i++) {
	printf(". ");fflush(0);
	msleep(1000);
    }
	
    if (retval == PASSED) {
	if (patriot_rcv_cmd_result_packet(iface,
			           FROM_HOST_UPGRADE_FPGA_DOWNLOAD_TO_SPI_PROM)) {
		retval = FAILED;
	}    
    }

    if (patriot_cleanup_ge_env(iface) == FAILED) {
	retval = FAILED;
    }
        
    return retval;    

}

/**********************************************************************
 *
 * Function: patriot_golden_fpga_download_spi_prom
 *
 * This function download the golden Patriot FPGA to the FPGA SPI PROM
 *
 * Input : iface - Patriot data structure info pointer
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
int
patriot_golden_fpga_download_spi_prom(patriot_ds_t *iface)
{
    int retval = PASSED, i;
    char ch;


    printf("\nThis utility will replace the golden FPGA");
    printf("\nContinue? (y/n) [n]: ");
    ch = getchar();
    printf("%c\n", ch);
    if ((ch == 'y') || (ch == 'Y')) {
	;
    } else {
	return (FAILED);
    }
    
    if (patriot_setup_ge_env(iface) == FAILED) {
        return (FAILED);
    }    

    patriot_clear_rx_buf();
    
    if (patriot_send_cmd(iface, FROM_HOST_GOLDEN_FPGA_DOWNLOAD_TO_SPI_PROM, 0)) {
	retval = FAILED;
    }

    /* Add 3 minutes of delay here because the FPGA download takes long */
    printf("\nPlease wait for the FPGA download ");
    for (i = 0; i < 180; i++) {
	printf(". ");fflush(0);
	msleep(1000);
    }

    if (retval == PASSED) {
	if (patriot_rcv_cmd_result_packet(iface,
				    FROM_HOST_GOLDEN_FPGA_DOWNLOAD_TO_SPI_PROM)) {
	    retval = FAILED;
	}    
    }

    if (patriot_cleanup_ge_env(iface) == FAILED) {
	retval = FAILED;
    }
        
    return retval;    

}


/**********************************************************************
 *
 * Function: patriot_display_fpga_info
 * This function displays FPGA registers and multiboot info table
 *
 * Input : iface - Patriot data structure info pointer
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
int
patriot_display_fpga_info(patriot_ds_t *iface)
{
    int i, retval = PASSED;
    uchar buf[4 * BLOCK_256];
    reg_info_t *reg_ptr = patriot_te3_fpga_regs;
    
    prpass(testpass, "Display FPGA Info");

    if (iface->fpga_downloaded[iface->slot] != TRUE) {
	if (patriot_fpga_download(iface)) {
	    return (FAILED);
	}
    }
    
    if (patriot_setup_ge_env(iface) == FAILED) {
        return (FAILED);
    }

    patriot_clear_rx_buf();
    
    if (patriot_send_cmd(iface, FROM_HOST_FPGA_READ_INFO, 0)) {
	retval = FAILED;
    }

    if (retval == PASSED) {
	if (patriot_rcv_cmd_result_packet(iface, FROM_HOST_FPGA_READ_INFO)) {
	    retval = FAILED;
	}
    }

    /* First 12 bytes are for result and parameters */
    memcpy(&buf[0], &recv_packet_p->data[12], 4 * BLOCK_256);
    i = 0;
#if 0    
    for (i = 0; i < 50; i++) {
	printf("\noffset 0x%02x = 0x%02x", i, buf[i]);
    }
#endif
    while (reg_ptr->size.size != 0) {
	/*
	 * display current value
	 */
	printf("\n %25s",reg_ptr->name);
	printf(" , 0x%08x = 0x%.08x, (mask:0x%.08x)",
	       reg_ptr->offset, buf[reg_ptr->offset], reg_ptr->mask);
	
	reg_ptr++;
    }
    
    /* Runtime Calculated Bitstream HMAC */
    printf("\n\nRuntime Calculated Bitstream HMAC: \n");
    for (i = 0; i < RUNTIME_HMAC_SIZE; i++) {
	if (i % 16 == 0) {
	    printf("\n 0x%02x: ", i);
	}
	printf(" 0x%02x", buf[RUNTIME_HMAC_OFFSET + i]);
    }

    /* Rollover Key */
    printf("\n\nRollover Key: \n");
    for (i = 0; i < ROLLOVER_KEY_SIZE; i++) {
	if (i % 16 == 0) {
	    printf("\n 0x%03x: ", i);
	}
	printf(" 0x%02x", buf[ROLLOVER_KEY_OFFSET + i]);
    }
    
    /* SB Core Version String Size */
    printf("\n\nSB Core Version String Size: ");
    printf("0x%02x", buf[SB_CORE_VER_STR_SIZE_OFFSET]);

    /* SB Core Version String */
    printf("\n\nSB Core Version String : \n");
    for (i = 0; i < SB_CORE_VER_STR_SIZE; i++) {
	printf("%c", buf[i + SB_CORE_VER_STR_OFFSET]);    
    }
    
    /* Microloader Version String Size */
    printf("\n\nMicroloader Version String Size: ");
    printf("0x%02x", buf[MICRO_VER_STR_SIZE_OFFSET]);

    /* Microloader Version String */
    printf("\n\nMicroloader Version String : \n");
    for (i = 0; i < MICRO_VER_STR_SIZE; i++) {
	printf("%c", buf[i + MICRO_VER_STR_OFFSET]);    
    }

    /* FIPS-GUID Pointer */
    printf("\n\nFIPS-GUID Pointer : ");
    for (i = 0; i < FIPS_GUID_PTR_SIZE; i++) {
	printf(" 0x%02x", buf[i + FIPS_GUID_PTR_OFFSET]);    
    }

    /* Rollover Key Pointer */
    printf("\n\nRollover Key Pointer : ");
    for (i = 0; i < ROLLOVER_KEY_PTR_SIZE; i++) {
	printf(" 0x%02x", buf[i + ROLLOVER_KEY_PTR_OFFSET]);    
    }    
        
    if (patriot_cleanup_ge_env(iface) == FAILED) {
	retval = FAILED;
    }
    printf("\n");
    return(retval);
    
}

/**********************************************************************
 *
 * Function: get_mac_addr_from_cookie
 * This function extract the MAC address in cookie
 *
 * Input : cookie_contents - pointer to the cookie contents
 *         mac_addr - pointer to the MAC address structure
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
int
get_mac_addr_from_cookie(uchar *cookie_contents, mac_addr_t *mac_addr)
{

     uchar bytes, *data_ptr;
     
     if ((data_ptr = (uchar *)search_type_ret_addr_of_first_data
	 ((uchar *)cookie_contents, BOARD_MAC_ADDR, 
	  &bytes, FALSE)) == NULL) {
	 cterr('f', 0, "MAC address is not programmed in cookie");
	 return (FAILED);
     }

     memcpy((uchar *)mac_addr, data_ptr, bytes);
     return (PASSED);
}

/**********************************************************************
 *
 * Function: display_err_msg
 * This function utility for get the error message buffer from the module
 *
 * Input : None
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
void
display_err_msg(void)
{
    uchar buf[4 * BLOCK_256];

    if (recv_packet_p->data[12] != 0) { /* There is a message */
	/* first 12 bytes are for result and parameters */
	memcpy(&buf[0], &recv_packet_p->data[12], 4 * BLOCK_256);
	printf("\n******ERROR MESSAGE FROM MODULE:******\n%s", &buf[0]);fflush(0);
    }
}


/******** History ********/
/*------------------------------------------------------------------------------
$Log: sm_patriot_utils.c,v $
Revision 1.26  2018/05/18 09:24:48  alpeng
 Neptune merge to trunk with tag <neptune-branch-0518>

Revision 1.25  2017/07/14 02:51:38  alpeng
fixed compiler warning, due to cross-compiler version was updated.

Revision 1.24.40.2  2018/05/17 10:50:20  alpeng
 sync with trunk <trunk-051618>

Revision 1.24.40.1  2016/12/05 06:36:59  alpeng
fixed the uart ctrl num for ngio; change is approved on prrq

Revision 1.24  2014/06/12 19:16:57  huanngo
Fix the GE0 loopback failure on the new switch from Utah

Revision 1.23  2014/06/10 23:41:29  mcharon
remove redundant call to close()

Revision 1.22  2014/05/03 14:52:48  mcharon
use IFNAMSIZE; cache uio dir name in uio_utils

Revision 1.21  2014/02/18 09:11:12  alpeng
CSCul88171-3: remove useless files: mon_boot.h, c82576_ethmap.h, eth_frames.c, stack.h, sys_regs.h

Revision 1.20  2013/11/26 08:40:34  hroni
fix compiler warning

Revision 1.19  2013/11/11 21:18:39  mcharon
pass string instead of number in first argum of host_send_packet ; add xaui support

Revision 1.18  2013/05/09 19:25:18  mcharon
remove unused header files. fixed dependancy compile problem

Revision 1.17  2012/12/03 12:43:04  steja
Add error message utility

Revision 1.16  2012/11/07 02:42:58  steja
Add Subrate individual test

Revision 1.15  2012/11/07 00:57:15  mcharon
add define for MAC_ADDR_SIZE used in patriot; remove tcipi.h

Revision 1.14  2012/10/15 21:25:59  huanngo
Adding the code to program correct MAC address and filtering out the packets not from the testing Patriot SM

Revision 1.13  2012/10/04 00:13:48  huanngo
Fix an error message in patriot_send_command_packet()

Revision 1.12  2012/09/19 18:34:38  huanngo
Support new utility for secure boot and interface test

Revision 1.11  2012/07/19 17:40:12  huanngo
Support FPGA programming to SPI PROM

Revision 1.10  2012/06/27 06:17:03  steja
Add Power Margin Utilities

Revision 1.9  2012/06/08 21:27:46  huanngo
Remove patriot_memory_ecc_test

Revision 1.8  2012/06/08 19:05:43  huanngo
Move the patriot_memory_ecc_test from sm_patriot_utils.c to sm_patriot_test.c

Revision 1.7  2012/06/07 21:20:17  huanngo
Adding new tests

Revision 1.6  2012/05/04 20:01:46  mcharon
use void* instead of int as argument to to func ptrs in ngio_intf

Revision 1.5  2012/05/02 17:55:59  huanngo
Clean up and dowonload FPGA when necessary, not right after boot up Linux

Revision 1.4  2012/03/28 23:35:08  huanngo
Support new tests and utilities on Patriot

Revision 1.3  2012/03/28 00:38:15  mcharon
remove forward slash from second line

Revision 1.2  2012/03/25 03:58:22  palin2
Clean up Overlord Intel side (x86) compile warnings.

Revision 1.1.1.1  2012/03/23 23:02:05  ptong
Initial archive of ng_diag module


$Endlog$
*/
