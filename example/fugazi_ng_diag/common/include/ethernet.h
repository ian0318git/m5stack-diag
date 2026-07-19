/* $Id: ethernet.h,v 1.6 2019/08/06 06:56:06 alpeng Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/include/ethernet.h,v $
 *------------------------------------------------------------------
 * Header file for IEEE 802.3 ethernet packet defines
 * 
 * Oct 2010 ptong
 *
 * Copyright (c) 2011 ~ 2019 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#ifndef __ETHERNET_H__
#define __ETHERNET_H__

/* Ethernet 802.3 maximum and minimum packet length
 */
#define ETH_FRAME_MAX_LEN       1518 //From MACDA to CRC
#define ETH_FRAME_MIN_LEN       64
#define ETH_PKT_CRC_LEN         4
#define ETH_PKT_MAX_LEN         (ETH_FRAME_MAX_LEN - ETH_PKT_CRC_LEN)
#define ETH_PKT_MIN_LEN         (ETH_FRAME_MIN_LEN - ETH_PKT_CRC_LEN)
#define ETH_HDR_LEN        14
#define ETH_CRC_LEN        4


/* Ethernet, IP, TCP, UDP packet header lengths (option field omitted)
 */
#define ETH_HDR_LEN             14 // MAC address + type
#define ETH_IP_HDR_LEN          20
#define ETH_UDP_HDR_LEN         8
#define ETH_TCP_HDR_LEN         20
#define ETH_IP_UDP_HDR_LEN      (ETH_HDR_LEN + ETH_IP_HDR_LEN + ETH_UDP_HDR_LEN)
#define ETH_IP_TCP_HDR_LEN      (ETH_HDR_LEN + ETH_IP_HDR_LEN + ETH_TCP_HDR_LEN)

/* Packet maximum lengths
 */
#define ETH_PKT_DATA_MAX_LEN    (ETH_PKT_MAX_LEN - ETH_HDR_LEN)
#define ETH_IP_PKT_MAX_LEN      ETH_PKT_DATA_MAX_LEN
#define ETH_IP_DATA_MAX_LEN     (ETH_IP_PKT_MAX_LEN - ETH_IP_HDR_LEN)
#define ETH_UDP_PKT_MAX_LEN     ETH_IP_DATA_MAX_LEN
#define ETH_UDP_DATA_MAX_LEN    (ETH_UDP_PKT_MAX_LEN - ETH_UDP_HDR_LEN)
#define ETH_TCP_PKT_MAX_LEN     ETH_IP_DATA_MAX_LEN
#define ETH_TCP_DATA_MAX_LEN    (ETH_TCP_PKT_MAX_LEN - ETH_TCP_HDR_LEN)

/* Packet minimum lengths
 */
#define ETH_PKT_DATA_MIN_LEN    (ETH_PKT_MIN_LEN - ETH_HDR_LEN)
#define ETH_IP_PKT_MIN_LEN      ETH_PKT_DATA_MIN_LEN
#define ETH_IP_DATA_MIN_LEN     (ETH_IP_PKT_MIN_LEN - ETH_IP_HDR_LEN)
#define ETH_UDP_PKT_MIN_LEN     ETH_IP_DATA_MIN_LEN
#define ETH_UDP_DATA_MIN_LEN    (ETH_UDP_PKT_MIN_LEN - ETH_UDP_HDR_LEN)
#define ETH_TCP_PKT_MIN_LEN     ETH_IP_DATA_MIN_LEN
#define ETH_TCP_DATA_MIN_LEN    (ETH_TCP_PKT_MIN_LEN - ETH_TCP_HDR_LEN)

#define IPV4_VER_HLEN            0x45

#define AN_EN               1
#define AN_DIS              0

/* Generic port numbering and speed
 */
enum eth_spd {
  ETH_10MBS = 0,
  ETH_100MBS,
  ETH_1GBS,
  ETH_10GBS,
};

enum eth_pnum {
  ETH0 = 0,
  ETH1,
  ETH2,
  ETH3,
  ETH4,
  ETH5,
  ETH6,
  ETH7,
  ETH8,
  ETH9,
  ETH10,
  ETH11,
  ETH12,
};

enum xaui_pnum {
  XAUI0 = 0,
  XAUI1,
  XAUI2,
  XAUI3,
};

enum rgmii_pnum {
  RGMII0 = 0,
  RGMII1,
  RGMII2,
  RGMII3,
};

enum phy_addr_num {
  PHY_ADDR0 = 0,
  PHY_ADDR1,
  PHY_ADDR2,
  PHY_ADDR3,
  PHY_ADDR4,
  PHY_ADDR5,
  PHY_ADDR6,
  PHY_ADDR7,
  PHY_ADDR8,
  PHY_ADDR9,
};

enum sgmii_pnum {
  SGMII0 = 0,
  SGMII1,
  SGMII2,
  SGMII3,
  SGMII4,
  SGMII5,
  SGMII6,
  SGMII7,
  SGMII8,
  SGMII9,
};

enum sfp_pnum {
  SFP0 = 0,
  SFP1,
  SFP2,
  SFP3,
};
#define MAC_ADDR_SIZE                6


#endif /* __ETHERNET_H__ */

/******** History ******** 
$Log: ethernet.h,v $
Revision 1.6  2019/08/06 06:56:06  alpeng
merge curie, switzer and nightwatch to trunk

Revision 1.5.122.1  2019/07/24 08:31:33  alpeng
merge trunk to branch

Revision 1.5  2013/11/11 21:18:38  mcharon
pass string instead of number in first argum of host_send_packet ; add xaui support

Revision 1.4  2013/10/08 08:48:25  tirawan
Woodlawn collapsed to main trunk

Revision 1.3  2012/11/07 00:57:15  mcharon
add define for MAC_ADDR_SIZE used in patriot; remove tcipi.h

Revision 1.2  2012/03/28 00:38:10  mcharon
remove forward slash from second line

Revision 1.1.1.1  2012/03/23 23:02:03  ptong
Initial archive of ng_diag module


$Endlog$
*/
