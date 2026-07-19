 /* $Id: dnv_eth_lib.h,v 1.3 2020/08/06 07:54:55 kehuang2 Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/tabei-l/dnv_eth_lib.h,v $
 *------------------------------------------------------------------
 * 
 * dnv_eth_lib.h
 *
 * Copyright (c) 2019 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */
#ifndef _DNV_ETH_LIB_H_
#define _DNV_ETH_LIB_H_

/* Tabei-L */
enum {
    TABEI_GE0_88E1514_PHY = 0,
    TABEI_GE1_88E1514_PHY,
    TABEI_I350_SFP_PORT2,
    TABEI_I350_SFP_PORT3,
};

/* Fortnite */
enum {
    TABEI_88E1543_P0_QSGMII_PHY = 4,
    TABEI_88E1543_P0_AUTO_DETECT_PHY,
    TABEI_88E1543_P1_QSGMII_PHY,
    TABEI_88E1543_P1_AUTO_DETECT_PHY,
};

enum {
    DNV_LAN0_PORT0 = 0,
    DNV_LAN0_PORT1,
    DNV_LAN1_PORT0,
    DNV_LAN1_PORT1,
};

/* Tabei-L Interface */
#define TABEI_88E1514_PHY1_IFACE_NAME                 "x553_eth0"
#define TABEI_88E1514_PHY2_IFACE_NAME                 "x553_eth1"

#define TABEI_I350_SFP_P2_IFACE_NAME                  "i350_eth2"
#define TABEI_I350_SFP_P3_IFACE_NAME                  "i350_eth3"

#define TABEI_ETH_BP                                  "x553_eth3"

#define ETH_PHY_1514_GE0_UP            "ifconfig x553_eth0 up > /dev/null"
#define ETH_PHY_1514_GE0_DOWN          "ifconfig x553_eth0 down > /dev/null"
#define ETH_PHY_1514_GE1_UP            "ifconfig x553_eth1 up > /dev/null"
#define ETH_PHY_1514_GE1_DOWN          "ifconfig x553_eth1 down > /dev/null"

#define TABEI_ETH_NIM_SLOT_UP          "ifconfig x553_eth3 up > /dev/null"
#define TABEI_ETH_NIM_SLOT_IP          "ifconfig x553_eth3 192.123.123.1"

#define TESTCARD_ETH_UP                "ifconfig x553_eth3 up"
#define TESTCARD_ETH_DOWN              "ifconfig x553_eth3 down"

#define DISPLAY_I350_PORT0_CAP         "ethtool i350_eth0"
#define DISPLAY_I350_PORT1_CAP         "ethtool i350_eth1"
#define DISPLAY_I350_PORT2_CAP         "ethtool i350_eth2"
#define DISPLAY_I350_PORT3_CAP         "ethtool i350_eth3"

#define I350_PORT2_UP                  "ifconfig i350_eth2 up"
#define I350_PORT3_UP                  "ifconfig i350_eth3 up"
#define I350_PORT2_DOWN                "ifconfig i350_eth2 down"
#define I350_PORT3_DOWN                "ifconfig i350_eth3 down"

#define TABEI_1514_GE0_PHY_ADDR        0
#define TABEI_1514_GE1_PHY_ADDR        1

#define ETH_RM_IXGBE_MODULE            "rmmod ixgbe.ko"
#define ETH_INS_IXGBE_MODULE           "insmod /lib/modules/4.14.3/ixgbe.ko"

/* End of Tabei-L Interface */

/* Fortnite Interface */
#define TABEI_88E1543_P0_PHY_IFACE_NAME                "eno1"
#define TABEI_88E1543_P1_PHY_IFACE_NAME                "eno2"
#define CPU_LAN1_P0_TO_ESW_88E6390_P9_IF_NAME          "eno3"
#define CPU_LAN1_P1_TO_ESW_88E6390_P10_IF_NAME         "eno4"

#define ETH_PHY_1543_P0_UP            "ifconfig eno1 up > /dev/null"
#define ETH_PHY_1543_P0_DOWN          "ifconfig eno1 down > /dev/null"
#define ETH_PHY_1543_P1_UP            "ifconfig eno2 up > /dev/null"
#define ETH_PHY_1543_P1_DOWN          "ifconfig eno2 down > /dev/null"
#define ETH_LAN1_P0_UP                "ifconfig eno3 up > /dev/null"
#define ETH_LAN1_P1_UP                "ifconfig eno4 up > /dev/null"
#define ETH_LAN1_P0_DOWN              "ifconfig eno3 down > /dev/null"
#define ETH_LAN1_P1_DOWN              "ifconfig eno4 down > /dev/null"

#define TABEI_1543_P0_QSGMII_PHY_ADDR                       0x4
#define TABEI_1543_P0_AUTO_DET_PHY_ADDR                     0x5
#define TABEI_1543_P1_QSGMII_PHY_ADDR                       0x6
#define TABEI_1543_P1_AUTO_DET_PHY_ADDR                     0x7

#define ESW_PHY_ADDR_88E6390                                0xF
/* End of Fortnite Interface */

#define LINK_RETRY_COUNTER   (60) 
#define LINK_DELAY_TIME_500  (500)
#define WAIT_BK_LINK_UP      (1000)

#define TABEI_LPBK_RETRY                                    5

#define TABEI_DHCPD           "/opt/tool/dhcpd &"
#define TABEI_OPENTFTP        "/opt/tool/opentftpd -i /etc/opentftp.ini &"
#define TABEI_KILL_DHCPD      "killall dhcpd"
#define TABEI_KILL_OPENTFTP   "killall opentftpd"

#define SFP_IDENTIFIER_SFP_3       3
#define SFP_IDENTIFIER_SFP_DWDM_b  0xb
#define SFP_VENDOR_NAME_20         20
#define SFP_VENDOR_NAME_35         35
#define EEPROM_DATA_ADDR_0         0
#define EEPROM_DATA_ADDR_64        64


extern int dnv_read_phy_reg(uint , uchar, ushort , ushort *);
extern int dnv_write_phy_reg (uint , uchar, ushort , ushort );
extern int dnv_eth_get_iface_name(int, char *);
extern int dnv_eth_link_is_up(int);
extern int dnv_eth_force_link_set(int, int);
extern void dnv_phy_reg_access(void);
extern int chk_linux_eth_linkup(int, int);
extern int igb_read_sfp_eeprom_util(void);
extern int igb_dump_sfp_eeprom_util(void);
extern int igb_read_sfp_vendor_name(int, char *);
extern int igb_read_sfp_phy_util(void);
extern int igb_write_sfp_phy_util(void);
extern int igb_read_sfp_phy(int, ushort, ushort *);
extern int igb_write_sfp_phy(int, ushort, ushort);


#endif /* DNV_ETH_LIB_H_ */

/*-------------------------------------------------
 * $Log: dnv_eth_lib.h,v $
 * Revision 1.3  2020/08/06 07:54:55  kehuang2
 * Collapse Promethium into main trunk
 *
 * Revision 1.2  2019/10/17 02:16:24  kehuang2
 * Collapse Tabei-L into main trunk
 *
 * Revision 1.1.2.29  2019/09/18 01:27:07  kehuang2
 * Suppout the PHY chip config on sfp for I350
 *
 * Revision 1.1.2.28  2019/09/10 06:10:33  olin2
 * Support read/write SFP PHY function
 *
 * Revision 1.1.2.27  2019/08/29 07:29:37  olin2
 * Support read spcific SFP PHY util
 *
 * Revision 1.1.2.26  2019/08/29 03:49:27  kehuang2
 * Clean up code by the comment of code review
 *
 * Revision 1.1.2.25  2019/08/26 08:13:04  olin2
 * Support read SFP EEPROM
 *
 * Revision 1.1.2.24  2019/08/21 07:14:11  kehuang2
 * Add workaround for viper issue
 *
 * Revision 1.1.2.23  2019/07/26 08:25:33  olin2
 * Code clean up
 *
 * Revision 1.1.2.22  2019/07/04 03:20:19  kehuang2
 * Update with udev modified
 *
 * Revision 1.1.2.21  2019/05/29 12:01:20  kehuang2
 * Rename function by the platform name
 *
 * Revision 1.1.2.20  2019/03/27 08:56:16  kehuang2
 * Clean up code for Promethium
 *
 * Revision 1.1.2.19  2019/03/14 08:18:20  kehuang2
 * Support Tabei-L Promethium sku
 *
 * Revision 1.1.2.18  2019/03/14 06:14:47  olin2
 * Clean up code
 *
 * Revision 1.1.2.17  2019/03/12 01:22:01  kehuang2
 * Support Test Mode Utility
 *
 * Revision 1.1.2.16  2019/03/08 02:00:20  kehuang2
 * Support 88e1543 QSGMII ping config utility
 *
 * Revision 1.1.2.15  2019/03/07 06:38:09  olin2
 * Support Arkentone on Tabei-L
 *
 * Revision 1.1.2.14  2019/02/26 08:10:42  olin2
 * Add opentftp and dhcpd app
 *
 * Revision 1.1.2.13  2019/02/25 09:47:34  harrchan
 * For different network config in Cisco Bios0.5
 *
 * Revision 1.1.2.12  2019/02/22 08:20:06  harrchan
 * Support 88E1543 utility
 *
 * Revision 1.1.2.11  2019/02/01 03:46:41  wilbhuan
 * Defined new macro for shutting down LAN1 P0/P1 interface.
 *
 * Revision 1.1.2.10  2019/01/31 01:44:43  harrchan
 * Support Register test and Interrupt test
 *
 * Revision 1.1.2.9  2019/01/25 08:48:26  harrchan
 * Merge sku1 and sku2 function
 *
 * Revision 1.1.2.8  2019/01/25 03:21:06  wilbhuan
 * 1. Added ESW(Ethernet Switch) test with 88E6390 PHY device.
 * 2. The scope of ESW test as following:
 *    (1) Register test
 *    (2) MAC loopback test
 *    (3) External loopback test
 *    (4) Interrupt test
 *
 * Revision 1.1.2.7  2019/01/19 02:37:56  harrchan
 * Update Phy 1543 Structure
 *
 * Revision 1.1.2.6  2019/01/18 02:31:46  harrchan
 * Update code after code review
 *
 * Revision 1.1.2.5  2019/01/16 04:03:45  harrchan
 * Init phy1543 test
 *
 * Revision 1.1.2.4  2018/12/24 08:04:44  olin2
 * Support GE PHY workaround for DNV ethernet controller bug
 *
 * Revision 1.1.2.3  2018/12/04 08:12:45  olin2
 * Update check link
 *
 * Revision 1.1.2.2  2018/10/24 02:47:27  harrchan
 * 88E1514 GEPHY test
 *
 * Revision 1.1.2.1  2018/10/02 01:50:02  harrchan
 * Initial commit for Tabei-L P1A bring up.
 *
 * $Endlog$
 *-------------------------------------------------
 */
