/* $Id: sgmii_defs.h,v 1.2 2015/05/25 03:59:10 steja Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/skye_linux/include/sgmii_defs.h,v $
 *------------------------------------------------------------------
 *
 * sgmii_defs.h -- Definitions file for SGMII Interface
 *
 * Feb 2008, Art Wong
 *
 * Copyright (c) 2008-2015 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */
#ifndef __SGMII_INTF_H__
#define __SGMII_INTF_H__


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
} eth_rx_pkt_t;

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
extern int setup_eth_dev (int sgmii_port, int *socket);
extern int cleanup_eth_dev (int sgmii_port, int socket);
extern int eth_pkt_tx (eth_tx_pkt_t *);
extern int eth_pkt_rx (eth_rx_pkt_t *);
extern int get_sgmii_port_num (uint, uint);
extern int get_host_mac_addr(uint, unsigned char *);

/* Added these for Overlord GE API
 */
extern int ovld_get_ge_sw_port_num (int slot, int tgt_device, int local_port);

#endif /* __SGMII_INTF_H__ */

/******** History ******** 
$Log: sgmii_defs.h,v $
Revision 1.2  2015/05/25 03:59:10  steja
Add Support Skye SM

Revision 1.1.4.2  2015/04/29 11:36:27  steja
Code check-in to skye-branch2 for ER code review


------------------------------------------------------------------
Revision 1.1.2.1  2014/07/21 01:56:39  palin2
Initial check-in Skye module side Diag code.

$Endlog$
*/
