/* $Id: dnv_eth_lib.h,v 1.5 2019/08/28 01:22:04 alicehua Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/nutella/dnv_eth_lib.h,v $
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

enum {
    NUTELLA_88E1543_P0_PHY = 0,
    NUTELLA_88E1543_P1_PHY,
    NUTELLA_88E1543_P2_PHY,
    NUTELLA_88E1543_P3_PHY,
    NUTELLA_I350_SFP_PORT1,
    NUTELLA_I350_SFP_PORT2,
};

enum {
    DNV_LAN0_PORT0 = 0,
    DNV_LAN0_PORT1,
    DNV_LAN1_PORT0,
    DNV_LAN1_PORT1,
    DNV_I350_PORT1,
    DNV_I350_PORT2,
};

enum e1000_media_type {
	e1000_media_type_unknown = 0,
	e1000_media_type_copper = 1,
	e1000_media_type_fiber = 2,
	e1000_media_type_internal_serdes = 3,
	e1000_num_media_types
};

#define ETH_PHY_1543_P0_UP            "ifconfig enp5s0f0 up > /dev/null"
#define ETH_PHY_1543_P0_DOWN          "ifconfig enp5s0f0 down > /dev/null"
#define ETH_PHY_1543_P1_UP            "ifconfig enp5s0f1 up > /dev/null"
#define ETH_PHY_1543_P1_DOWN          "ifconfig enp5s0f1 down > /dev/null"
#define ETH_PHY_1543_P2_UP            "ifconfig enp7s0f0 up > /dev/null"
#define ETH_PHY_1543_P2_DOWN          "ifconfig enp7s0f0 down > /dev/null"
#define ETH_PHY_1543_P3_UP            "ifconfig enp7s0f1 up > /dev/null"
#define ETH_PHY_1543_P3_DOWN          "ifconfig enp7s0f1 down > /dev/null"

#define ETH_100M_PHY_1543_P0_UP            "ifconfig enp4s0f0 up > /dev/null"
#define ETH_100M_PHY_1543_P0_DOWN          "ifconfig enp4s0f0 down > /dev/null"
#define ETH_100M_PHY_1543_P1_UP            "ifconfig enp4s0f1 up > /dev/null"
#define ETH_100M_PHY_1543_P1_DOWN          "ifconfig enp4s0f1 down > /dev/null"
#define ETH_100M_PHY_1543_P2_UP            "ifconfig enp6s0f0 up > /dev/null"
#define ETH_100M_PHY_1543_P2_DOWN          "ifconfig enp6s0f0 down > /dev/null"
#define ETH_100M_PHY_1543_P3_UP            "ifconfig enp6s0f1 up > /dev/null"
#define ETH_100M_PHY_1543_P3_DOWN          "ifconfig enp6s0f1 down > /dev/null"

#define ETH_RM_IXGBE_MODULE           "rmmod ixgbe.ko"
#define ETH_INS_IXGBE_MODULE          "insmod /lib/modules/4.14.3/ixgbe.ko"


#define NUTELLA_88E1543_P0_PHY_IFACE_NAME                "enp5s0f0"
#define NUTELLA_88E1543_P1_PHY_IFACE_NAME                "enp5s0f1"
#define NUTELLA_88E1543_P2_PHY_IFACE_NAME                "enp7s0f0"
#define NUTELLA_88E1543_P3_PHY_IFACE_NAME                "enp7s0f1"

#define NUTELLA_100M_88E1543_P0_PHY_IFACE_NAME                "enp4s0f0"
#define NUTELLA_100M_88E1543_P1_PHY_IFACE_NAME                "enp4s0f1"
#define NUTELLA_100M_88E1543_P2_PHY_IFACE_NAME                "enp6s0f0"
#define NUTELLA_100M_88E1543_P3_PHY_IFACE_NAME                "enp6s0f1"

#define NUTELLA_1543_P0_PHY_ADDR                     0
#define NUTELLA_1543_P1_PHY_ADDR                     1
#define NUTELLA_1543_P2_PHY_ADDR                     2
#define NUTELLA_1543_P3_PHY_ADDR                     3

#define NUTELLA_1543_PAGE_REG                     (22)
#define NUTELLA_1543_PAGE_REG_VAL_FFFF            0xFFFF
#define NUTELLA_1543_PAGE_0                       (0)
#define NUTELLA_1543_PHYID_REG                    (2)
#define NUTELLA_1543_PHYID                        0x141

#define NUTELLA_I350_SFP_P1_IFACE_NAME      "enp3s0f0"
#define NUTELLA_I350_SFP_P2_IFACE_NAME      "enp3s0f1"
#define LINK_RETRY_COUNTER    (60)
#define LINK_DELAY_TIME_500   (500)

#define NUTELLA_LPBK_RETRY                            3
#define AVOID_RACING_CONDITION_DELAY                  2000

extern int dnv_read_phy_reg(uint , uint, uint , uint *);
extern int dnv_write_phy_reg (uint , uint, uint , uint );
extern int dnv_eth_get_iface_name(int, char *);
extern int dnv_get_correct_iface_name(int, char *);
extern int dnv_eth_link_is_up(int);
extern int dnv_eth_force_link_set(int, int);
extern void dnv_phy_reg_access(void);
extern int chk_linux_eth_linkup(int, int);
extern void ifconfig_down_up_eth(char *);


#endif /* DNV_ETH_LIB_H_ */

/*-------------------------------------------------
$Log: dnv_eth_lib.h,v $
Revision 1.5  2019/08/28 01:22:04  alicehua
1.CSCvr03904: Add retry to workaround Denverton loopback issue.
2.CSCvr03919: Fix console will hang when testing Marvell 88e1543 Interrupt test.

Revision 1.4  2019/07/11 12:31:30  alicehua
Collapse Nutella codes into main trunk.

$Endlog$
*/
