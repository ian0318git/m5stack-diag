/* $Id: platform_eth.h,v 1.14 2013/08/19 01:54:12 alpeng Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/overlord/cavium/platform_eth.h,v $
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
#include "common_utils.h"


/* Overlord platform internal IP addresses used in the diag
 */
#define HOST_ETH1_IP_ADDR        "192.123.123.1"
#define HOST_ETH2_IP_ADDR        "192.123.123.2"
#define HOST_ETH3_IP_ADDR        "192.123.123.3"
#define OCTEON_XAUI0_IP_ADDR     "192.123.123.234"
#define OCTEON_DUMMY_IP_ADDR     "192.123.123.235"
#define OCTEON_NETMASK           "255.255.255.0"

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

#define DUMP_ALL_PAGE 0
#define DUMP_ONE_PAGE 1
#define PLAT_PAGE_NUM_MAX 18

#define PLAT_SGMII_NUM_MAX    SGMII3
#define CVMX_GMX0_INF_ID       0
#define CVMX_GMX1_INF_ID       1
#define CVMX_XAUI_INF_ID       CVMX_GMX1_INF_ID

typedef struct mrvl_phy_regs_t_ {
    const char *pagename;
    uint32_t pagenum;
    const reg_info_t *pageregs;
} mrvl_phy_regs_t;


enum loopback_num {
  CAVIUM_INT_LPBK = 0,
  BRIDGE_PHY_INT_LPBK,
  MEDIA_PHY_INT_LPBK,
  SGMII_EXT_LPBK,
  SFP_EXT_LPBK,
  SGMII_INT_EXT_LPBK,
};

/* platform_eth.c
 */
extern void ovld_phy_reg_access(void);
extern int ovld_phy_reg_rd(int, int);
extern int ovld_phy_reg_wr(int, int, int);
extern int check_ext_lpbk_flag(void);
extern int phy_reg_wr(int, struct ifreq *, ushort, ushort);
extern int phy_reg_rd(int, struct ifreq *, ushort, ushort *);
extern unsigned int get_sfp_config(int);
extern unsigned int is_sfp_present(int);
extern unsigned int is_sfp_tx_fault(int);
extern int phy_reg_show(int, int, int, boolean);


/* platform_ext_lpbk.c  New SGMII test function prototypes 
 */
extern int ovld_cavium_int_lpbk_test(void);
extern int ovld_bridge_phy_int_lpbk_test(void);
extern int ovld_media_phy_int_lpbk_test(void);
extern int ovld_phy_ext_lpbk_test(void);
extern int ovld_sgmii_int_ext_lpbk_test(void);
extern int ovld_phy_lpbk_util(void);
extern int show_status_info(int);
extern int ovld_media_phy_testmode_util(void);

/* platform_sfp_ext_lpbk.c  Fiber test function prototype 
 */
extern int sfp_phy_ext_lpbk_test(void);


/* SGMII test function prototypes
 */
extern void set_gmxeno(int , boolean);
extern void set_sgmii_int_lpbk(int , boolean);
extern int sgmii_port_cfg(int, int, int);
extern void display_sgmii_port_cfg(void);
extern void display_sgmii_port_stats(void);
extern void sgmii_phy_reg_dump(void);


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
extern void marvell_1340_init(void);

/* MACsec test function protoyptes */
extern int macsec_test_main(int);


#endif /* __PLATFORM_ETH_H__ */

/*-------------------------------------------------
$Log: platform_eth.h,v $
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
