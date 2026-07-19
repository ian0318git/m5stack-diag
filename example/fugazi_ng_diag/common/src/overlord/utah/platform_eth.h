/* $Id: platform_eth.h,v 1.13 2018/12/21 00:58:06 haohsu Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/overlord/utah/platform_eth.h,v $
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

/* utah mdio bus addr 0-3 for media PHY; 4-7 for bridge PHY */
#define ADDR_MEDIA_PHY 0
#define ADDR_BRIDGE_PHY 4

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

#define MAP_SIZE 4096UL
#define MAP_MASK (MAP_SIZE - 1)
#define GOLDBEACH_MGMT  1

typedef struct mrvl_phy_regs_t_ {
    const char *pagename;
    uint32_t pagenum;
    const reg_info_t *pageregs;
} mrvl_phy_regs_t;


enum loopback_num {
  BRIDGE_PHY_INT_LPBK = 0,
  MEDIA_PHY_INT_LPBK,
  SGMII_EXT_LPBK,
  SFP_EXT_LPBK,
  SGMII_INT_EXT_LPBK,
};

enum dagger_goldbeach_loopback_num {
  DAGGER_PHY_INT_LPBK = 1,
  DAGGER_SGMII_EXT_LPBK,
  DAGGER_SFP_EXT_LPBK,
  DAGGER_SGMII_INT_EXT_LPBK,
  GOLDBEACH_ETH0_INT_EXT_LPBK,
  VG400_ETH0_INT_EXT_LPBK,
  VG400_ETH1_INT_EXT_LPBK,
};


/* platform_eth.c
 */
extern int ovld_phy_reg_rd(int, int);
extern int ovld_phy_reg_wr(int, int, int);
extern int check_ext_lpbk_flag(void);
extern int phy_reg_wr(int, struct ifreq *, ushort, ushort);
extern int phy_reg_rd(int, struct ifreq *, ushort, ushort *);
extern int utah_phy_reg_wr(uint, ushort, ushort);
extern int utah_phy_reg_rd(uint, ushort, ushort *);
extern unsigned int is_sfp_present(int);
extern int phy_reg_show(int, int, int, boolean);
extern int check_ext_lpbk_flag(void);
extern void utah_marvell_1340_init(void);
extern void utah_marvell_phy_eye_enlarge(void);
extern void dagger_sgmii_setting(void);
extern int phy_1548_interrupt_test(void);

/* platform_ext_lpbk.c  New SGMII test function prototypes 
 */
extern int ovld_bridge_phy_int_lpbk_test(void);
extern int ovld_media_phy_int_lpbk_test(void);
extern int ovld_phy_ext_lpbk_test(void);
extern int ovld_sgmii_int_ext_lpbk_test(void);
extern int ovld_phy_lpbk_util(void);
extern int show_status_info(int);
extern int ovld_media_phy_testmode_util(void);
extern int tmp_ovld_set_packet(void);
extern int dagger_phy_int_lpbk_test(void);
extern int dagger_phy_ext_lpbk_test(void);
extern int dagger_phy_int_ext_lpbk_test(void);
extern int dagger_phy_lpbk_util(void);
extern int vg400_ge_phy_int_ext_lpbk_test(uint32_t);

/* platform_sfp_ext_lpbk.c  Fiber test function prototype 
 */
extern int sfp_phy_ext_lpbk_test(void);
extern int dagger_sfp_phy_ext_lpbk_test(void);


/* SGMII test function prototypes
 */
extern void set_gmxeno(int , boolean);
extern void set_sgmii_int_lpbk(int , boolean);
extern int sgmii_port_cfg(int, int, int);
extern void display_sgmii_port_cfg(void);
extern void display_sgmii_port_stats(void);

/* MACsec test function protoyptes */
extern int macsec_test_main(int);


/* phy_reg_test.c VG400 define */
#define PORT_NUM     0
#define REG_READ_TIME    5

/* diag.c VG400 define */
#define WATCHDOG_PATH     "/dev/watchdog"
#define RESET             1
#define UNRESET           0
#define PORT_NUM0         0
#define PORT_NUM1         1
#define PORT_NUM2         2
#define PORT_NUM3         3
#define EN_UART           0x1FF

/* platform_eth.c VG400 define */
#define PHY_REST_TIME        100
#define PHY_PAGE0            0 
#define PHY_PAGE20           20 
#define PHY_PAGE22           22 
#define PHY_PAGE23           23 
#define PHY_PAGE24           24 
#define PHY_PAGE25           25 
#define PHY_PAGE26           26 
#define PHY_PAGE27           27 
#define PHY_PAGE29           29 
#define PHY_PAGE30           30 
#define BRIDGE_MASK1         0x7
#define BRIDGE_MASK2         0x5
#define SIOCRMIIREG          0x89F1      /* VG400 Read MII PHY register. */ 
#define SIOCWMIIREG          0x89F2      /* VG400 Write MII PHY register.*/
extern int vg400_phy_reg_wr(uint, ushort, ushort);
extern int vg400_phy_reg_rd(uint, ushort, ushort *);

#endif /* __PLATFORM_ETH_H__ */

/*-------------------------------------------------
$Log: platform_eth.h,v $
Revision 1.13  2018/12/21 00:58:06  haohsu
CSCvn27142-Fixed 1548 PHY Interrupt test fail

Revision 1.12  2018/08/30 06:59:43  haohsu
Collapse Vg400-branch to Main Trunk

Revision 1.11  2016/10/16 12:28:22  iachang
Supported Goldbeach Platform.

Revision 1.10  2014/01/14 08:54:59  alpeng
support SFP loopback test on dagger

Revision 1.9  2013/12/25 07:12:16  alpeng
support dagger eth external loopback test with speed 100Mbps

Revision 1.8  2013/12/21 01:38:05  ptong
Change typo in function name

Revision 1.7  2013/12/17 08:11:03  alpeng
per HW request, support special setting on dagger

Revision 1.6  2013/12/06 11:58:58  danchung
Fix SGMII/SFP PHY loopback util menu for USD

Revision 1.5  2013/12/03 08:22:08  alpeng
support 1548 eye enlarge. power on ports before setting speed

Revision 1.4  2013/11/07 07:25:24  alpeng
support 1340 init, eye enlarge, and reset_quad_phy

Revision 1.3  2013/11/01 06:04:49  alpeng
support accessing bridge PHY on PHY utilities

Revision 1.2  2013/06/14 10:22:23  alpeng
follow O2 menu structure

Revision 1.1  2013/05/31 11:03:41  alpeng
support front panel GE loopback test

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
