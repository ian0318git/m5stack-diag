 /* $Id: diag_eth_info.h,v 1.2 2019/12/11 10:10:29 lucywang Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/nanook/diag_eth_info.h,v $
 *-----------------------------------------------------------------------------
 * File: diag_eth_info.h
 * Description: a general eth related definition
 *
 * Copyright (c) 2019 by Cisco Systems, Inc.
 * All rights reserved.
 *-----------------------------------------------------------------------------
 */

#ifndef __DIAG_ETH_INFO_H__
#define __DIAG_ETH_INFO_H__

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


#endif /* __DIAG_ETH_INFO_H__ */

/*---------------------------------------------------------------
$Log: diag_eth_info.h,v $
Revision 1.2  2019/12/11 10:10:29  lucywang
Merged Nanook to main trunk


$Endlog$
*/

