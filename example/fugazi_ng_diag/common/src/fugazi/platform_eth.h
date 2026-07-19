/* $Id: platform_eth.h,v 1.2 2021/06/02 08:22:35 iachang Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/fugazi/platform_eth.h,v $
 *------------------------------------------------------------------
 * Header file for platform ethernet code 
 * 
 * Oct 2010 ptong
 * Jan 2019, Letsai modified for Fugazi.
 *
 * Copyright (c) 2011-2021 by Cisco Systems, Inc.
 * All rights reserved.
 *------------------------------------------------------------------
 */

#ifndef __PLATFORM_ETH_H__
#define __PLATFORM_ETH_H__

#include <net/if.h>
#include "common_utils.h"

#define NEP_PATTERN   0x5ADB

/* Fugazi platform internal IP addresses used in the diag
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
#define SOURCE_SCRIPT            "source /diag_utils/fugazi/scripts/function.sh"
#define FUGAZI_UPDATE_MAC        "fugazi_update_all_mac_address"
#define FUGAZI_UPDATE_I211_MAC   "i211_update_mac_address"
#define FUGAZI_UPDATE_BCM57412_MAC  "fugazi_update_bcm57412_mac_address"
#define CHECK_MAC_FILE           "/fugazi-diag/check_mac.txt"


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

extern int ge_port_mapping_phy_addr_down[];
extern int ge_port_mapping_phy_addr_up[];
extern int ge_port_mapping_phy_addr[];
extern int ge_phy_mapping_phy_num[];
extern int te_port_mapping_phy_addr[];
extern int sgmii_mapping_qlm_num[];
extern int eth_mapping_sgmii_num[];
extern int eth_mapping_xfi_num[];
extern int eth_qlm5_sfp_list[];


/* platform_sfp_ext_lpbk.c  Fiber test function prototype
 */
extern int sfp_phy_ext_lpbk_test(void);
extern int check_sfp_link(char *);

/* SGMII test function prototypes
 */
extern int sgmii_port_cfg (int port_num, int speed, int an_en);
extern void set_gmxeno (int port_num, boolean onoff);
extern void set_sgmii_int_lpbk (int eth_num, boolean onoff);

/* platform_eth.c
 */
extern int check_ext_lpbk_flag(void);
extern int check_ge_int_lpbk_flag(void);


#endif /* __PLATFORM_ETH_H__ */

/*-------------------------------------------------
$Log: platform_eth.h,v $
Revision 1.2  2021/06/02 08:22:35  iachang
CSCvo59196-33 : Merge Fugazi from ASR1K-main-branch to ISR main trunk

Revision 1.1.8.3  2021/04/29 01:43:40  pdoong
Add checking if PHY Network side link is up in 'SyncE Recovered Clock Test'

Revision 1.1.8.2  2020/08/26 02:37:51  iachang
Merge Fugazi code into main trunk

Revision 1.1.6.11  2020/08/24 00:02:56  pdoong
Clean code for ER.

Revision 1.1.6.10  2020/08/06 04:35:28  pdoong
clean code for BCM54194 1G PHY

Revision 1.1.6.9  2020/08/04 08:37:06  iachang
Update Copyright to 2020

Revision 1.1.6.8  2020/06/08 06:54:39  iachang
Program Aikido FPGA DEV key utility
Program I211 and BCM57412 MAC address utility.

Revision 1.1.6.7  2019/04/18 01:21:30  letsai
1. Clean up code
2. Modify 1G phy address mapping
3. Modify print message of MCU FW opgrade

Revision 1.1.6.6  2019/04/11 22:32:29  letsai
1. Replace the sign "*" to "-" when doing FPGA interrupt test
2. Fix M.2 combo test when slot is empty.
3. Make "check link utility" easy to use.
4. When USB console detected, check the corresponding FPGA register bit.

Revision 1.1.6.5  2019/04/09 16:10:40  letsai
1. Support all BCM54194 PHY (0~3) Register Test.
2. Let utilities can dump each phy registers.
3. Check link status for each phy and each port(upstream and downstream).

Revision 1.1.6.4  2019/04/06 01:36:14  letsai
1. Remove unused functions and files.
2. Fix BCM54194 SFP External loopback test.
3. Fix BCM54194 Register test.
4. Fix Voltage Margin Utility.
5. Add function to show system information.

Revision 1.1.6.3  2019/04/01 21:02:05  iachang
Support MAC address program and verify utility.

Revision 1.1.6.2  2019/03/14 03:48:27  letsai
Initial check in.



$Endlog$
*/
