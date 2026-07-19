/* $Id: platform_eth.h,v 1.1 2015/02/26 07:18:29 xiaoyizh Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/wallander/platform_eth.h,v $
 *------------------------------------------------------------------
 * Header file for platform ethernet code 
 * 
 * April 2014, Xiaoying Zhang
 * Copyright (c) 2015 by Cisco Systems, Inc.
 * All rights reserved.
 *------------------------------------------------------------------
 */

#ifndef __PLATFORM_ETH_H__
#define __PLATFORM_ETH_H__

#include <net/if.h>


/* Linux ethernet interface name */
#define NAME_ETH        "eth"
#define NAME_XAUI       "xaui"
#define NAME_MGMT       "mgmt"

#define TX_RX_SYNC_TIME       10

#define MAX_7_PORT      7
#define MAX_5_PORT      5
#define MAX_4_PORT      4

/* Overlord platform internal IP addresses used in the diag
 */
#define HOST_ETH1_IP_ADDR                        "192.123.123.1"
#define HOST_ETH2_IP_ADDR                        "192.123.123.2"
#define HOST_ETH3_IP_ADDR                        "192.123.123.3"
#define OCTEON_XAUI0_IP_ADDR                  "18.18.18.18"
#define OCTEON_XAUI0_DUMMY_IP_ADDR     "18.18.18.20"
#define OCTEON_XAUI1_IP_ADDR                   "19.19.19.19"
#define OCTEON_XAUI1_DUMMY_IP_ADDR     "19.19.19.20"
#define OCTEON_NETMASK                              "255.255.255.0"

#define SPD_10MBPS    10
#define SPD_100MBPS   100
#define SPD_1000MBPS   1000

#define ADDR_MEDIA_PHY 4
#define ADDR_BRIDGE_PHY 128

#define SEL_PORT_ETH "eth"

#define SIG_COPPER 0
#define SIG_FIBER 1

#define DISABLE_SIG   0
#define ENABLE_SIG    1

#define INT_LPBK 0
#define EXT_LPBK 1

#define AUTONEG_OFF  0
#define AUTONEG_ON   1

#define HALF_DUPLEX  0
#define FULL_DUPLEX  1

#define RX_READY   0x1
#define RX_FINISH  0x10


/* The cpu port connected to the GE switch */
#define CPU_GESW_PORT         SGMII3
#define PLAT_SGMII_NUM_MAX    SGMII3
#define PLAT_RGMII_NUM_MAX    RGMII1

#define CVMX_GMX0_INF_ID       0
#define CVMX_GMX1_INF_ID       1
#define CVMX_GMX2_INF_ID       2
#define CVMX_GMX3_INF_ID       3
#define CVMX_GMX4_INF_ID       4
/* Woodlawn use Cavium CN68XX which XAUI is attach to GMX3 */
#define CVMX_XAUI_INF_ID       CVMX_GMX3_INF_ID
#define CVMX_BP_XAUI_INF_ID       CVMX_GMX2_INF_ID

enum loopback_num {
    CAVIUM_INT_LPBK = 0,
    BRIDGE_PHY_INT_LPBK,
    MEDIA_PHY_INT_LPBK,
    SGMII_EXT_LPBK,
    SFP_EXT_LPBK,
    SGMII_INT_EXT_LPBK,
    PTP_SGMII_EXT_LPBK
};

enum eth_ge_port_num {
    GE_PORT0 = 0,
    GE_PORT1,
    GE_PORT2,
    GE_PORT3,
    GE_PORT4,
    GE_PORT5,
};

enum eth_sfp_port_num {
    SFP_PORT0 = 0,
    SFP_PORT1,
    SFP_PORT2,
    SFP_PORT3,
    SFP_PORT4,
    SFP_PORT5,
};

enum phy_port_num {
    PHY_PORT0 = 0,
    PHY_PORT1,
    PHY_PORT2,
    PHY_PORT3,
};

typedef struct {     
   char name[10];  /* name of eth*/
     int speed;   /* test speed */
     int pkt_num; /* packet number */
     int pkt_len; /* packet length */
     boolean signal;  /* test signal */
     ushort type; /* to avoid set env everytime */
     int socket;
} diag_info_pthread_t;


extern int phy_soft_reset(char *, int);
extern int set_phy_int_lpbk(char *ifname, int eth_num);
extern int set_phy_ext_lpbk(char *ifname, int eth_num);
extern boolean is_eth_phy_linkup (char *ifname, int portnum);
extern void phy_reg_dump(char *ifname, int portnum);
extern void phy_reg_access(void);
extern int check_ext_lpbk_flag(void);
extern int cfg_phy(char *ifname, int portnum, int speed, int duplex, int autoneg);

extern int send_packet_util(void);
extern int show_status_info(int);

/* platform_sfp_ext_lpbk.c  Fiber test function prototype 
 */
extern int sfp_phy_ext_lpbk_test(int);
extern int sfp_88E1548l_line_lpbk_test(void);
extern int check_sfp_link(char *);

/* SGMII test function prototypes
 */
extern void set_gmxeno(int , boolean);
extern boolean is_sgmii_linkup (int eth_num);
extern void set_sgmii_int_lpbk (int eth_num, boolean onoff);
extern int sgmii_port_cfg (int port_num, int speed, int an_en);
extern int cpu_gesw_port_cfg (int port_num);
extern void display_sgmii_port_cfg(void);
extern void display_sgmii_port_stats(void);
extern void sgmii_phy_reg_dump(void);
extern int sgmii_phy_int_lpbk_test(void);
extern int sgmii_phy_ext_lpbk_test(void);
extern int sgmii_phy_lpbk_util(void);
extern void config_ethx(void);
extern void sgmii_get_link_status(int, int *, int *, int*);
extern void smi_ctl_reg_dump(void);


/* XAUI test function prototypes
 */
extern int xaui_internal_lpbk_test(void);
extern int xaui_external_lpbk_test(void);
extern int xaui_ping_test(void);
extern void display_xaui_port_status(void);
extern void dump_xaui_gmx_regs(void);
extern void dump_xaui_pcs_regs(void);
extern void xaui_int_lpbk_util(void);
extern void config_xaui0(void);

extern int reset_quad_phy(void);

extern int old_eth_mapping_ge_num[];
extern int old_eth_mapping_sfp_num[];
extern int old_ge_mapping_eth_num[];
extern int old_ge_mapping_phy_port[];
extern int old_sfp_mapping_eth_num[];
extern int old_sfp_mapping_phy_port[];
extern int old_eth_fiber_mapping[];
extern int two_phy_eth_mapping_ge_num[];
extern int one_phy_eth_mapping_ge_num[];
extern int two_phy_eth_mapping_sfp_num[];
extern int one_phy_eth_mapping_sfp_num[];
extern int two_phy_ge_mapping_eth_num[];
extern int one_phy_ge_mapping_eth_num[];
extern int two_phy_ge_mapping_phy_port[];
extern int one_phy_ge_mapping_phy_port[];
extern int two_phy_sfp_mapping_eth_num[];
extern int one_phy_sfp_mapping_eth_num[];
extern int two_phy_sfp_mapping_phy_port[];
extern int one_phy_sfp_mapping_phy_port[];
extern int two_phy_eth_fiber_mapping[];
extern int one_phy_eth_fiber_mapping[];
extern int eth_qlm0_list[];
extern int eth_qlm4_list[];
extern int qlm_0_4_1340_phy_addr[];
extern int qlm_0_4_1548_phy_addr[];

#endif /* __PLATFORM_ETH_H__ */

/*-------------------------------------------------
 * $Log: platform_eth.h,v $
 * Revision 1.1  2015/02/26 07:18:29  xiaoyizh
 * Initial check in for Wallander.
 *
 *
 * $Endlog$
 *-------------------------------------------------
 */
