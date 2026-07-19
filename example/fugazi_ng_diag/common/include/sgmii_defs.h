/* $Id: sgmii_defs.h,v 1.5 2017/03/30 08:17:09 hondwang Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/include/sgmii_defs.h,v $
 *------------------------------------------------------------------
 *
 * sgmii_defs.h -- Definitions file for SGMII Interface
 *
 * Feb 2008, Art Wong
 *
 * Copyright (c) 2008-2017 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */
#ifndef __SGMII_INTF_H__
#define __SGMII_INTF_H__

#ifdef TACHI
#else
typedef struct {
    mac_addr_t  dest_addr;
    mac_addr_t  src_addr;
    ushort	pkt_type;
    ushort      tx_status;
    ushort      payload_size;
    uchar	*bufr_st_addr;
    uint	pkt_num;
    int         socket;
} eth_tx_pkt_t;

typedef struct {
    ushort      rx_status;
    ushort      pkt_size;
    uchar	*bufr_st_addr;
    ushort      rx_bufr_size;
    uint	pkt_num;
    uint	wait_time;
    int         socket;
    int         rx_chk;
} eth_rx_pkt_t;
#endif

/* ethernet transmit defines */
#define ETH_PKT_TX_OK		0x00000000
#define ETH_PKT_TX_ERR		0x00000001
#define ETH_TX_ERR		0x00000002
#define ETH_NO_PKT_TX		0x00000004
#define ETH_TX_BD_ERR		0x00000008

/* ethernet receive defines */
#define ETH_PKT_RX_OK		0x00000000
#define ETH_PKT_RX_ERR		0x00000001
#define ETH_RX_ERR		0x00000002
#define ETH_NO_PKT_RX		0x00000004
#define ETH_RX_BD_ERR		0x00000008
#define ETH_RX_BUFR_OVFL	0x00000010

/* define loopback mode */
enum
{
    SGMII_LPBK_NONE,		/* no loopback */
    SGMII_LPBK_MAC,		/* internal loopback at ppc etsec */
    SGMII_LPBK_PCS,		/* internal loopback at cavium pcs */
    SGMII_LPBK_QLM,		/* line loopback at cavium QLM */
    SGMII_SW_LPBK_INTERNAL,	/* internal loopback at marvell GE switch */
};

#define SGMII_LINK_UP		0x0100
#define SGMII_LINK_DOWN		0x0000
#define SGMII_FLOW_CTRL		0x0080
#define SGMII_NO_FLOW_CTRL	0x0000
#define SGMII_FULL_DUPLEX	0x0040
#define SGMII_HALF_DUPLEX	0x0000
#define SGMII_ETH_SPEED_MASK	0x0030
#define SGMII_SPEED_1000	0x0020	/* 1000Mbps */
#define SGMII_SPEED_100		0x0010	/* 100Mbps */
#define SGMII_SPEED_10		0x0000	/* 10Mbps */
#define SGMII_LOOP_MODE_MASK	0x000F	/* defined in above enum */

#define SGMII_PORT0		0
#define SGMII_PORT1		1
#define SGMII_PORT2		2
#define SGMII_PORT3		3

/* defines for debug_flag */
#define D_DEBUG                 0x00000001
#define D_TX_DEBUG              0x00000002
#define D_RX_DEBUG              0x00000004
#define D_INTERACT_DEBUG        0x00000008

typedef struct
{
    union
    {
        uint32 u32;
        struct
        {
            uint32 dst_port_mgidl   : 8;  /**< When MCST=0, This is destination port. When MCST=1, This is LSBs of 		
											multicast group index. */
            uint32 dst_port_mgidh   : 8;  /**< When MCST=0, This is destination module ID. When MCST=1, This is MSBs of 		
											multicast group index. */
            uint32 tc               : 4;  /**< Traffic Class [3:0] indicates the distinctive Quality of Service (QoS)
                                            the switching fabric will provide when forwarding the packet
                                            through the fabric */
            uint32 mcst             : 1;  /**< Destination packet is multicast type
                                            - 0: Unicast
                                            - 1: Mulitcast */
            uint32 reserved_21_23   : 3;
            uint32 sop              : 8;  /**< The delimiter indicating the start of a packet transmission */
        } s;
    } b0; /* block 0 */
    union
    {
        uint32 u32;
        struct
        {
            uint32 ppd_type       : 3;  /**< Packet Processing Descriptor Type
                                            - 000: PPD Overlay1
                                            - 001: PPD Overlay2
                                            - 010~111: Reserved */
            uint32 reserved_3_5   : 3;
            uint32 dp             : 2;  /**< Drop Precedence indicates the traffic rate violation status of the
                                            packet measured by the ingress module.
                                            - 00: GREEN
                                            - 01: RED
                                            - 10: Reserved
                                            - 11: Yellow */
            uint32 lbid           : 8;  /**< Load Balancing ID indicates a packet flow hashing index
                                            computed by the ingress XGS module for statistical distribution of
                                            packet flows through a multipath fabric */
            uint32 src_pid        : 8;  /**< Source Port ID indicates a port associated with the module
                                            indicated by the SRC_MODID, through which the packet has
                                            entered the system */
            uint32 srcmod_id      : 8;  /**< Source Module ID indicates the source XGS module from which
                                            the packet is originated. (It can also be used for the fabric multicast
                                            load balancing purpose.) */
        } s;
    } b1; /* block 1 */
    union
    {
        uint32 u32;
        struct
        {
			uint32 dest_vp      : 16; /**< The destination VP index or multicast index for 
										egress chip packet modifications and encapsulation */
			uint32 vni_low      : 8; /**< The lower 8-bits of the VNI */
			uint32 vni_mid      : 2; /**< The middle 2-bits of the VNI */
            uint32 fwd_type     : 5; /**< Indicates the forwarding type of the HG packet */
            uint32 multipoint   : 1; /**< Indicates whether the packet is sent to a single point 
											or multipoint tree within the egress chip */
        } s;
    } b2; /* block 2 */
    union
    {
        uint32 u32;
        struct
        {
			uint32 source_type	    : 1; /**< Indicates whether the source is a physical port (set to 1) 
												or virtual port (set to 0) */
			uint32 dest_type	    : 1; /**< Indicates whether the destination is a physical port (set to 1) or virtual port (set to 0) */
			uint32 preserve_dot1p	: 1; /**< If set, don't touch the 802.1 priority / CFI of the outermost L2 header */
			uint32 preserve_dscp	: 1; /**< If set, don't touch the DSCP of the outermost IP header */
			uint32 vni_high		    : 2; /**< The upper 2-bits of the VNI */
			uint32 reserved_7_6	    : 2; /**< Reserved */
			uint32 opcode			: 3; /**< Unused in PPD2 header */
			uint32 reserved_11	    : 1; /**< Reserved */
			uint32 lag_failover	    : 1; /**< When set, it indicates that the packet is redirected by a LAG failover. 
												The packet must not redirected again by LAG failover. */
			uint32 do_not_learn	    : 1; /**< when set, do not perform MAC learning */
			uint32 do_not_modify	: 1; /**< When set, indicates the packet should be sent to the destination port unmodified*/
			uint32 mirror			: 1; /**< Indicates whether the packet is mirrored or switched */
			uint32 source_vp 		: 16; /**< The source VP from which the packet originally enters the system.
												Used for MAC learning and source knockout */
        } s;
    } b3; /* block 3 */
} higig2_header_t;


extern int  test_debug_flag;

extern int  display_etsec_regs(int);
extern int  display_etsec_test_params(int);
extern int  etsec_lpbk_test_util(int);
extern int  get_ge_sw_port_num(int, int);
extern int  get_num_ge_sw_ports(void);
extern int  get_first_ge_sw_port(void);
extern int  gmii_elpbk_test(int);
extern int  mvl_sw_cleanup(int, int);
extern int  sgmii_ext_lpbk_test(int, int, int, int, int);
extern int  sgmii_init(int sgmii_port, int speed);
extern int  etsec_init(int, int, int, boolean);
extern int  etsec_adjust_link(int, int, int, int);
extern void etsec_start(int, boolean);

/* SGMII APIs */
extern int setup_eth_dev(char *, int *socket);
extern int cleanup_eth_dev(char *, int socket);
#ifdef TACHI
#else
extern int eth_pkt_tx (eth_tx_pkt_t *);
extern int eth_pkt_rx (eth_rx_pkt_t *);
#endif
extern int get_sgmii_port_num (uint, uint);
extern int get_host_mac_addr(uint, unsigned char *);

/* Added these for Overlord GE API
 */
extern int ovld_get_ge_sw_port_num (int slot, int tgt_device, int local_port);

#endif /* __SGMII_INTF_H__ */

/******** History ******** 
$Log: sgmii_defs.h,v $
Revision 1.5  2017/03/30 08:17:09  hondwang
Tachi-L brach merge

Revision 1.4.66.1  2017/02/21 07:34:30  haohsu
Add NIM Dynamo to TACHI

Revision 1.4  2013/11/11 21:18:38  mcharon
pass string instead of number in first argum of host_send_packet ; add xaui support

Revision 1.3  2013/07/22 19:55:50  mcharon
add struct

Revision 1.2  2012/03/28 00:38:12  mcharon
remove forward slash from second line

Revision 1.1.1.1  2012/03/23 23:02:04  ptong
Initial archive of ng_diag module


$Endlog$
*/
