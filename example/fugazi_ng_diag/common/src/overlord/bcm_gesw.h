/* $Id: bcm_gesw.h,v 1.6 2017/02/06 08:36:15 alpeng Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/overlord/bcm_gesw.h,v $
 *------------------------------------------------------------------
 *
 * bcm_gesw.h - Data structure used by the bcm functions
 *
 * Oct 2011, Paul Tong
 *
 * Copyright (c) 2013-2017 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */
#ifndef __BCM_GESW_H__
#define __BCM_GESW_H__

#include "bcm_gesw_defs.h"

typedef unsigned char mac_addr_t[6];

/* Overlord GESW global environment
 */

/* Connection info for forwarding a packet from one port to another */
typedef struct gesw_port_conn_s {
    bcm_port_t  this_port;
    bcm_port_t  to_port;
    bcm_vlan_t  added_vlan;
    sal_mac_addr_t dst_mac;
    sal_mac_addr_t src_mac;
    int         dst_modid;
    int         src_modid;
} gesw_port_conn_t;

/* Notes: Port from sgmii_defs.h.
 * eth_tx_pkt_t and eth_rx_pkt_t data structures were used in the
 * xformers GE switch API for supporting IO modules.
 * Port over here to used in gesw internal code to support
 * the packet generated for the broadcom cmic port traffic.
 */

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

/* GESW API defines
 */
#define PASS      0
#define FAIL      1

/* Ethernet packet macrocs
 */
#define ETH_HDR_LEN             14 // MAC address + type
#define ETH_PKT_CRC_LEN         4
#define ETH_PKT_LEN_MAX         1518
#define ETH_L2_PKT_LEN(payload_len) (ETH_HDR_LEN + (payload_len) + ETH_PKT_CRC_LEN);

/* GESW advertisement setting */
#define BCM_GESW_10G_ADV        0x220
#define BCM_GESW_DEFAULT_ADV    0xe20

/* end note */

extern int invoke_bcm_shell(void);
extern void host_init_bcm_shell(void);

extern int bcm_gesw_xaui_lpbk_set(int unit, int port, int lb);
extern int bcm_gesw_xaui_lpbk_get(int unit, int port, int lb, int *state);
extern int bcm_gesw_ge_lpbk_set(int unit, int port, int lb);
extern int bcm_gesw_ge_lpbk_get(int unit, int port, int lb, int *state);
extern int get_bcm_shell_test_result(int testnum, int *runcnt, int *passcnt);
extern int exec_bcm_shell_cmd (int unit, char *test_cmd, int print_cmd);
extern int ovld_gesw_init(void);

#endif /* __BCM_GESW_H__ */

/******** History ******** 
$Log: bcm_gesw.h,v $
Revision 1.6  2017/02/06 08:36:15  alpeng
fixed NTC 10G-KR link up unstable issue

Revision 1.5  2014/06/19 22:00:51  ptong
Incorporated BCM sdk-6.4.1-EA2 for 10G-KR interface. New code works on O2, June, and non Greyhound USD regression

Revision 1.4  2014/03/13 18:32:31  ptong
Remove gesw_info_init and unused code

Revision 1.3  2013/11/26 08:40:35  hroni
fix compiler warning

Revision 1.2  2013/11/11 21:18:40  mcharon
pass string instead of number in first argum of host_send_packet ; add xaui support

Revision 1.1  2013/05/09 05:42:38  alpeng
moving overlord common code from x86

Revision 1.5  2012/09/07 22:50:00  ptong
Code clean-up

Revision 1.4  2012/06/05 11:44:36  palin2
Clean up compiler warnings.

Revision 1.3  2012/04/03 01:46:17  ptong
Replace loading rc.soc with ovld_gesw_init() to avoid re-init the bcm drivers

Revision 1.2  2012/03/28 00:38:20  mcharon
remove forward slash from second line

Revision 1.1.1.1  2012/03/23 23:02:08  ptong
Initial archive of ng_diag module


$Endlog$
*/
