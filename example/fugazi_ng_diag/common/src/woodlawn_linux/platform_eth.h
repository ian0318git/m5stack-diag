/* $Id: platform_eth.h,v 1.2 2013/10/08 08:48:30 tirawan Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/woodlawn_linux/platform_eth.h,v $
 *------------------------------------------------------------------
 * Header file for platform ethernet code 
 * 
 * Oct 2010 ptong
 * Copyright (c) 2011-2013 by Cisco Systems, Inc.
 * All rights reserved.
 *------------------------------------------------------------------
 */

#ifndef __PLATFORM_ETH_H__
#define __PLATFORM_ETH_H__

#include <net/if.h>

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

extern int phy_soft_reset(char *, int);
extern int set_phy_int_lpbk(char *ifname, int eth_num);
extern int set_phy_ext_lpbk(char *ifname, int eth_num);
extern boolean is_eth_phy_linkup (char *ifname, int portnum);
extern void phy_reg_dump(char *ifname, int portnum);
extern void phy_reg_access(void);
extern void woodlawn_phy_reg_access(void);
extern int check_ext_lpbk_flag(void);
extern int phy_reg_wr(int, struct ifreq *, ushort, ushort);
extern int phy_reg_rd(int, struct ifreq *, ushort, ushort *);
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
 * Revision 1.2  2013/10/08 08:48:30  tirawan
 * Woodlawn collapsed to main trunk
 *
 * Revision 1.1.4.2  2013/08/20 10:59:10  tirawan
 * Branch into woodlawn-branch2 and port woodlawn code
 *
 * Revision 1.1.2.2  2013/08/06 09:27:21  leschen
 * Add PTP SGMII external lpbk num.
 *
 * Revision 1.1.2.1  2013/04/24 10:37:24  tirawan
 * Initial check-in for woodlawn linux code
 *
 * Revision 1.4  2013/03/27 08:45:05  kuangik
 * Code cleanup
 *
 * Revision 1.2  2013/03/19 09:51:24  kuangik
 * Add retry mechanism (ported from O2) and reset quad phy if the test fails
 *
 * Revision 1.10  2013/02/18 06:47:11  kody
 * Modify for the port mapping changed according to the new SKUs.
 *
 * Revision 1.9  2012/12/11 01:05:37  leslie
 * Backplane xaui0 loopback test use cavium QLM2.
 *
 * Revision 1.8  2012/10/18 12:55:02  kody
 * Add 88E1548L fiber line loopback between platform side.
 *
 * Revision 1.7  2012/09/21 11:51:42  kody
 * Fix for the xaui lpbk test.
 *
 * Revision 1.6  2012/09/05 22:55:15  kody
 * Add enable eth2 ~ 7 network interfaces.
 *
 * Revision 1.5  2012/08/03 10:16:56  evanli
 * Mapping to latest O2 source code on 20120726
 *
 * Revision 1.3  2012/05/18 10:24:37  kody
 * Woodlawn use Cavium CN68XX which XAUI is attach to GMX3
 *
 * Revision 1.2  2012/04/06 06:07:45  kuangik
 * Update for GE PHY Test
 *
 * Revision 1.1.1.1  2012/02/10 05:59:50  kody
 * Initial imports Woodlawn project code base.
 *
 * Revision 1.1.2.6  2011/12/07 11:54:31  alpeng
 * using semaphore to handle pthreads, replacing cmpbyte to memcmp.
 * update menu item.
 *
 * Revision 1.1.2.5  2011/11/10 08:03:18  alpeng
 * support SFP loopback utils
 *
 * Revision 1.1.2.4  2011/11/02 00:55:24  alpeng
 * update loopback test, add util. packet number and length should be increased
 *
 * Revision 1.1.2.3  2011/10/31 18:46:07  alpeng
 * support internal/external loopback on Overlord.
 *
 * Revision 1.1.2.2  2011/09/09 22:22:12  ptong
 * Add phy_reg_access and cfg_phy functions
 *
 * Revision 1.1.2.1  2011/04/05 19:59:38  ptong
 * Initial checkin
 *
 * $Endlog$
 *-------------------------------------------------
 */
