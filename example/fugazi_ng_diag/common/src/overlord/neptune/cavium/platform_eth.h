/* $Id: platform_eth.h,v 1.2 2018/05/18 09:24:57 alpeng Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/overlord/neptune/cavium/platform_eth.h,v $
 *------------------------------------------------------------------
 * Header file for platform ethernet code 
 * 
 * Oct 2010 ptong
 * Copyright (c) 2011-2018 by Cisco Systems, Inc.
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
Revision 1.2  2018/05/18 09:24:57  alpeng
 Neptune merge to trunk with tag <neptune-branch-0518>

Revision 1.1.2.18  2016/12/27 02:01:42  meho
Added ge-Int loopback flag to control Cavium GE int/ext loopback test.

Revision 1.1.2.17  2016/11/28 03:43:55  meho
1. Fixed GE phy Mac/Int/Ext loopback test bugs.
2. Added 10G FW download.

Revision 1.1.2.16  2016/08/12 10:12:19  meho
Clean up code.

Revision 1.1.2.15  2016/07/26 07:54:26  meho
Added GE PHY PTP1588 loopback test skeleton.

Revision 1.1.2.14  2016/07/25 11:28:30  meho
Added register dump utility for BCM82752.

Revision 1.1.2.13  2016/07/25 09:05:41  meho
Added register dump utility for BCM54194.

Revision 1.1.2.12  2016/07/20 08:09:49  meho
1. Updated BCM82752 firmware array.
2. Added 10G PHY loopback debug utilities.

Revision 1.1.2.11  2016/07/20 01:44:59  meho
Added GE PHY loopback debug utilities.

Revision 1.1.2.10  2016/07/14 09:17:41  meho
Added internal/SFP-external loopback for BCM82752.

Revision 1.1.2.9  2016/07/13 08:28:09  meho
1. Added Cavium PCS internal loopback.
2. Added check link up function for bcm54194.

Revision 1.1.2.8  2016/07/12 08:40:58  meho
1. Added BCM54194/BCM82752 register tests.
2. Added BCM54194 internal/external-copper loopback configuration.

Revision 1.1.2.7  2016/07/07 09:04:30  meho
1. Added BCM54194 RDB register r/w utility.
2. Added GE PHY internal/external loopback skeleton.
3. Added 10GE PHY internal/external loopback skeleton.

Revision 1.1.2.6  2016/06/23 12:44:54  meho
Added bcm54194 soft-reset function.

Revision 1.1.2.5  2016/06/23 07:10:53  meho
Update previous comment:
Added switch to SGMII/Copper/Fiber register space steps in BCM54194 r/w utility.

Revision 1.1.2.4  2016/06/23 06:28:49  meho
Added switch to SGMII/Copper/Fiber register space steps in BCM54191 r/w utility.

Revision 1.1.2.3  2016/06/22 10:40:27  meho
Added GE/10GE PHY r/w utilities.

Revision 1.1.2.2  2016/06/12 10:31:07  bowang3
Add bcm82752 10G PHY code framework

Revision 1.1.2.1  2016/06/06 05:58:51  xiaoyizh
Initial Check-in for Neptune Data Plane diags.

Revision 1.14  2013/08/19 01:54:12  alpeng
checking sfp tx fault right after spf loopback test

Revision 1.13  2013/01/30 23:50:15  palin2
Add utility to set Cavium side GE PHY, Marvell 1548, into Test mode.

Revision 1.12  2013/01/25 10:47:02  alpeng
support macsec util

Revision 1.11  2012/10/18 05:19:54  ptong
Add marvell_1340_init and PHY reset util

Revision 1.10  2012/10/05 09:12:10  alpeng
support media/bridge PHY register dump

Revision 1.9  2012/09/17 15:55:31  alpeng
1. add is_linkup for sfp
2. combine soft reset on set_automedia and bridge_phy_mode for speed up
3. fixed definition order of SGMII_INT_EXT_LPBK for util.
4. clean up code.

Revision 1.8  2012/08/22 10:04:23  alpeng
Using Ext. loopback flag to decide loopback test is internal or external loopback test. Moving media PHY diag item into debug utility menu

Revision 1.7  2012/06/05 06:29:45  alpeng
clean up compiler warnings

Revision 1.6  2012/06/05 06:21:03  alpeng
clean up compiler warnings.

Revision 1.5  2012/04/29 04:38:30  ptong
Fix diag flag problem

Revision 1.4  2012/04/27 10:42:42  alpeng
fixed minor bugs and support set external loopback flag for controlling test flow

Revision 1.3  2012/03/28 00:38:19  mcharon
remove forward slash from second line

Revision 1.2  2012/03/27 16:18:21  alpeng
cavium side code clean up

Revision 1.1.1.1  2012/03/23 23:02:07  ptong
Initial archive of ng_diag module


$Endlog$
*/
