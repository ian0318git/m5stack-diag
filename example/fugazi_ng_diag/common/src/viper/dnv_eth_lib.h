 /* $Id: dnv_eth_lib.h,v 1.4 2018/12/10 09:57:05 harrchan Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/viper/dnv_eth_lib.h,v $
 *------------------------------------------------------------------
 * 
 * dnv_eth_lib.h
 *
 * Copyright (c) 2018 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */
#ifndef _DNV_ETH_LIB_H_
#define _DNV_ETH_LIB_H_

enum {
    VIPER_88E1514_PHY,
    VIPER_GE1_88E1514_PHY,
    VIPER_88E6176,
    VIPER_88E1512_PHY,
};

enum {
    DNV_LAN0_PORT0 = 0,
    DNV_LAN0_PORT1,
    DNV_LAN1_PORT0,
    DNV_LAN1_PORT1,
};


#define ETH_PHY_1514_GE0_UP           "ifconfig enp3s0f0 up > /dev/null"
#define ETH_PHY_1514_GE0_DOWN           "ifconfig enp3s0f0 down > /dev/null"
#define ETH_PHY_1514_GE1_UP           "ifconfig enp3s0f1 up > /dev/null"
#define ETH_PHY_1514_GE1_DOWN           "ifconfig enp3s0f1 down > /dev/null"

#define VIPERJ_ETH_PHY_1514_GE0_UP    "ifconfig enp3s0f0 up > /dev/null"
#define VIPERJ_ETH_PHY_1514_GE1_UP    "ifconfig enp3s0f1 up > /dev/null"
#define VIPERJ_ETH_ESW_88E6176_UP     "ifconfig enp5s0f0 up > /dev/null"

#define VIPER_ETH_DSL_UP              "ifconfig enp5s0f1 up > /dev/null"
#define VIPER_ETH_DSL_DOWN            "ifconfig enp5s0f1 down > /dev/null"
#define VIPER_ETH_DSL_SET_IP          "ifconfig enp5s0f1 192.168.2.100"


#define ETH_RM_IXGBE_MODULE           "rmmod ixgbe.ko"
#define ETH_INS_IXGBE_MODULE          "insmod /lib/modules/4.14.3/ixgbe.ko"


#define VIPER_88E1514_PHY_IFACE_NAME                "enp3s0f0"
#define SKY_88E1514_PHY_IFACE_NAME                  "enp3s0f1"
#define VIPER_88E6176_IFACE_NAME                    "enp5s0f0"
#define VIPER_88E1512_PHY_IFACE_NAME                "enp5s0f1"

#define VIPERJ_88E1514_PHY0_IFACE_NAME              "enp3s0f0"
#define VIPERJ_88E1514_PHY1_IFACE_NAME              "enp3s0f1"
#define VIPERJ_88E6176_IFACE_NAME                   "enp5s0f0"

#define VIPER_1512_PHY_ADDR                         1
#define VIPER_1514_GE0_PHY_ADDR                     0
#define VIPER_1514_GE1_PHY_ADDR                     1

#define VIPER_1514_PAGE_REG  (22)
#define VIPER_1514_PAGE_REG_VAL_FFFF  0xFFFF

#define VIPER_1514_PAGE_0  (0)
#define VIPER_1514_PHYID_REG  (2)
#define VIPER_1514_PHYID  0x141


#define VIPER_6176_PHY_ADDR                         0x1e
#define VIPER_6176_SMI_REG_VAL_FFFF                 0xFFFF
#define VIPER_LPBK_RETRY                            3

extern int dnv_read_phy_reg(uint , uchar, ushort , ushort *);
extern int dnv_write_phy_reg (uint , uchar, ushort , ushort );
extern int dnv_eth_get_iface_name(int, char *);
extern int dnv_eth_link_is_up(int);
extern int dnv_eth_force_link_set(int, int);
extern void dnv_phy_reg_access(void);



#endif /* DNV_ETH_LIB_H_ */

/*-------------------------------------------------
 * $Log: dnv_eth_lib.h,v $
 * Revision 1.4  2018/12/10 09:57:05  harrchan
 * Add workaround to PHY and Switch loopback test (CSCvn43011)
 *
 * Revision 1.3  2018/10/11 06:02:59  harrchan
 * Add FPGA function test (CSCvm72986)
 *
 * Revision 1.2  2018/08/06 02:31:51  harrchan
 * Merge viper E2E to the main trunk (CSCvk28469)
 *
 * Revision 1.1.2.8  2018/05/11 02:22:11  harrchan
 * Changed interface name by using Cisco BIOS
 *
 * Revision 1.1.2.7  2018/05/04 03:44:44  lucywang
 * Changed interface name by using Cisco BIOS c900-rommon.05022018.bin
 *
 * Revision 1.1.2.6  2018/04/16 08:41:44  olin2
 * Support DSL test
 *
 * Revision 1.1.2.5  2018/03/29 10:25:52  lucywang
 * Changed interface name by using Cisco BIOS
 *
 * Revision 1.1.2.4  2018/03/28 07:03:51  lucywang
 * Added API to check SKU ViperJ and changed interface name for ViperJ
 *
 * Revision 1.1.2.3  2018/03/23 06:36:17  olin2
 * Hide ifconfig message
 *
 * Revision 1.1.2.2  2018/03/14 06:59:37  olin2
 * Modify 1514 init sequence
 *
 * Revision 1.1.2.1  2018/02/27 08:06:49  harrchan
 * Initial viper application code base
 *
 *
 *
 *
 *
 *
 * $Endlog$
 *-------------------------------------------------
 */
