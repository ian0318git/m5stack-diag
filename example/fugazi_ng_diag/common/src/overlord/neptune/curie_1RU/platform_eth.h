/* $Id: platform_eth.h,v 1.2 2019/08/06 06:56:12 alpeng Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/overlord/neptune/curie_1RU/platform_eth.h,v $
 *------------------------------------------------------------------
 * Header file for platform ethernet code 
 * 
 * Feb 2019 Leschen
 * Copyright (c) 2011-2019 by Cisco Systems, Inc.
 * All rights reserved.
 *------------------------------------------------------------------
 */

#ifndef __PLATFORM_ETH_H__
#define __PLATFORM_ETH_H__

#include <net/if.h>
#include "common_utils.h"

#define NEP_PATTERN   0x5ADB

/* Neptune platform internal IP addresses used in the diag
 */
#define HOST_ETH1_IP_ADDR        "192.123.123.1"
#define HOST_ETH2_IP_ADDR        "192.123.123.2"
#define HOST_ETH3_IP_ADDR        "192.123.123.3"
#define OCTEON_XFI0_IP_ADDR       "192.123.123.234"
#define OCTEON_XFI1_IP_ADDR       "19.19.19.19"
#define OCTEON_DUMMY_IP_ADDR     "192.123.123.235"
#define OCTEON_XFI0_DUMMY_IP_ADDR "18.18.18.20"
#define OCTEON_XFI1_DUMMY_IP_ADDR "19.19.19.20"
#define OCTEON_NETMASK           "255.255.255.0"

#define SPD_10MBPS    10
#define SPD_100MBPS   100
#define SPD_1000MBPS   1000
#define SPD_10000MBPS   10000

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

#define DUMP_ALL_PAGE 0
#define DUMP_ONE_PAGE 1
#define PLAT_PAGE_NUM_MAX 18

#define PLAT_SGMII_NUM_MAX    SGMII3
#define PLAT_XFI_NUM_MAX      1

#define CVMX_GMX0_INF_ID       0
#define CVMX_GMX1_INF_ID       1
#define CVMX_GMX2_INF_ID       2
#define CVMX_GMX3_INF_ID       3
#define CVMX_GMX4_INF_ID       4
#define CVMX_GMX5_INF_ID       5
#define CVMX_GMX6_INF_ID       6
#define CVMX_XAUI_INF_ID       CVMX_GMX1_INF_ID
#define CVMX_BP_XAUI_INF_ID    CVMX_GMX2_INF_ID

#define CVMX_XFI_INF_ID        CVMX_GMX3_INF_ID
#define CVMX_BP_XFI_INF_ID     CVMX_GMX2_INF_ID

#define CVMX_BGX0_INF_ID       0
#define CVMX_BGX1_INF_ID       1
#define CVMX_BGX2_INF_ID       2

typedef struct bcm_phy_regs_t_ {
    const char *intfname;
    int phy_intf;
    const reg_info_t *intfregs;
} bcm_phy_regs_t;

typedef enum nep_bcm_phy_t_ {
    BCM_GE_PHY,
    BCM_TEN_GE_PHY,
} nep_bcm_phy_t;

enum loopback_num {
  CAVIUM_INT_LPBK = 0,
  GE_PHY_INT_LPBK,
  GE_PHY_EXT_LPBK,
  GE_PHY_SFP_EXT_LPBK,
  GE_PHY_SGMII_LPBK,
  SGMII_INT_EXT_LPBK,
  TEN_GE_PHY_INT_LPBK,
  TEN_GE_PHY_SFP_EXT_LPBK,
  XFI_INT_EXT_LPBK,
  PTP_SGMII_EXT_LPBK,
  PTP_XFI_SFP_EXT_LPBK,
};

enum eth_ge_port_num {
    GE_PORT0 = 0,
    GE_PORT1,
    GE_PORT2,
    GE_PORT3,
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

enum smi_bus_num {
  SMI_BUS_0 = 0,
  SMI_BUS_1,
};

extern int eth_mapping_phy_addr[];
extern int ge_port_mapping_phy_addr[];
extern int te_port_mapping_phy_addr[];
extern int sgmii_mapping_qlm_num[];
extern int eth_mapping_cvmx_bgx_num[];
extern int eth_mapping_sgmii_num[];
extern int eth_mapping_xfi_num[];
//extern int eth_mapping_cvmx_ipd_port[];

extern int show_status_info(int);

/* platform_sfp_ext_lpbk.c  Fiber test function prototype
 */
extern int sfp_phy_ext_lpbk_test(void);
//extern int sfp_88E1548l_line_lpbk_test(void);
extern int check_sfp_link(char *);

/* SGMII test function prototypes
 */
extern int sgmii_port_cfg (int port_num, int speed, int an_en);
extern void set_gmxeno (int port_num, boolean onoff);
extern void set_sgmii_int_lpbk (int eth_num, boolean onoff);

/* platform_eth.c
 */
extern unsigned int get_sfp_config(int);
extern unsigned int is_sfp_present(int);
extern unsigned int is_sfp_tx_fault(int);
extern int check_ext_lpbk_flag(void);
extern int check_ge_int_lpbk_flag(void);


#endif /* __PLATFORM_ETH_H__ */

/*-------------------------------------------------
$Log: platform_eth.h,v $
Revision 1.2  2019/08/06 06:56:12  alpeng
merge curie, switzer and nightwatch to trunk

Revision 1.1.2.1  2019/03/12 07:41:52  leschen
Initial check in to support BCM82752


$Endlog$
*/
