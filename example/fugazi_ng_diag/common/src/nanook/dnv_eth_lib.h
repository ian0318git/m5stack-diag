 /* $Id: dnv_eth_lib.h,v 1.3 2020/04/20 02:28:24 lucywang Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/nanook/dnv_eth_lib.h,v $
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
    NANOOK_88E1543_P0_QSGMII_PHY = 8,
    NANOOK_88E1543_P0_AUTO_DETECT_PHY,
    NANOOK_88E1543_P1_QSGMII_PHY,
    NANOOK_88E1543_P1_AUTO_DETECT_PHY,
};

enum {
    DNV_LAN0_PORT0 = 0,
    DNV_LAN0_PORT1,
    DNV_LAN1_PORT0,
    DNV_LAN1_PORT1,
};

extern char inface_lan0p0[32];
extern char inface_lan0p1[32];
extern char inface_lan1p0[32];
extern char inface_lan1p1[32];


#define ETH_PHY_1543_P2_UP            "ifconfig enp17s0f0 up > /dev/null"
#define ETH_PHY_1543_P2_DOWN          "ifconfig enp17s0f0 down > /dev/null"
#define ETH_PHY_1543_P3_UP            "ifconfig enp17s0f1 up > /dev/null"
#define ETH_PHY_1543_P3_DOWN          "ifconfig enp17s0f1 down > /dev/null"

#define DYNAMIC_ETH_PREFIX            "ifconfig -a | grep enp* | grep HWaddr | awk '{print $1}' | awk 'NR=="  

#define ETH_RM_IXGBE_MODULE           "rmmod ixgbe.ko"
#define ETH_INS_IXGBE_MODULE          "insmod /lib/modules/4.14.3/ixgbe.ko"

#define ETH_INS_IXGBE_MODULE_AC3      "insmod /lib/modules/4.14.3/ixgbe_LAN0Force10G.ko"
#define ETH_INS_IXGBE_MODULE_TESTCARD "insmod /lib/modules/4.14.3/ixgbe_LAN0AN.ko"


#define NANOOK_1543_P0_QSGMII_PHY_ADDR                       0x8
#define NANOOK_1543_P0_AUTO_DET_PHY_ADDR                     0x9
#define NANOOK_1543_P1_QSGMII_PHY_ADDR                       0xa
#define NANOOK_1543_P1_AUTO_DET_PHY_ADDR                     0xb

#define LINK_RETRY_COUNTER   (60) 
#define LINK_DELAY_TIME_500  (500)
#define WAIT_BK_LINK_UP      (1000)
#define AVOID_RACING_CONDITION_DELAY    (2000)

#define NANOOK_LPBK_RETRY                            3

#define NANOOK_ETH_BP_LAN0PORT0                   0

#define NANOOK_DHCPD           "/opt/tool/dhcpd &"
#define NANOOK_OPENTFTP        "/opt/tool/opentftpd -i /etc/opentftp.ini &"
#define NANOOK_KILL_DHCPD      "killall dhcpd"
#define NANOOK_KILL_OPENTFTP   "killall opentftpd"

#define LPBKTEST_PKT_CNT                (3)

extern int dnv_read_phy_reg(uint , uchar, ushort , ushort *);
extern int dnv_write_phy_reg (uint , uchar, ushort , ushort );
extern int dnv_eth_get_iface_name(int, char *);
extern int dnv_eth_link_is_up(int);
extern int dnv_eth_force_link_set(int, int);
extern void dnv_phy_reg_access(void);
extern int chk_linux_eth_linkup(int, int);
extern int dynamic_get_inface (int, char *);
extern void ifconfig_down_up_eth(char *);


#endif /* DNV_ETH_LIB_H_ */

 
/*-------------------------------------------------
 * $Log: dnv_eth_lib.h,v $
 * Revision 1.3  2020/04/20 02:28:24  lucywang
 *
 * 1. Fixed unplug/plug NIM module dynamically issue and added NIM cookie
 * 2. Added to support NIM Prince
 * 3. (CSCvn43011) add retry workaround for Deverton issue
 * 4. add debug message and set default value to seneors
 * 5. Reverted Register value of temp/press snsr after test
 * 6. Bumped up version to 1.0.2
 *
 * Revision 1.2  2019/12/11 10:10:32  lucywang
 * Merged Nanook to main trunk
 *
 *
 * $Endlog$
 *-------------------------------------------------
 */
