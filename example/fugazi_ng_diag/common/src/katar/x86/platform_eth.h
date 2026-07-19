/* $Id: platform_eth.h,v 1.2 2019/06/14 05:24:50 mikech2 Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/katar/x86/platform_eth.h,v $
 *------------------------------------------------------------------
 * Header file for platform ethernet code 
 * 
 * Oct 2010 ptong
 * Copyright (c) 2017-2019 by Cisco Systems, Inc.
 * All rights reserved.
 *------------------------------------------------------------------
 */

#ifndef __PLATFORM_ETH_H__
#define __PLATFORM_ETH_H__

#include <net/if.h>
#include "common_utils.h"

typedef enum { IFSTATUS_DOWN, IFSTATUS_UP, IFSTATUS_ERR } interface_status_t;

#define SPD_10MBPS    10
#define SPD_100MBPS   100
#define SPD_1000MBPS   1000
#define SPD_2500MBPS   2500
#define SPD_5000MBPS   5000
#define SPD_10000MBPS   10000

#define ADDR_MEDIA_PHY 4
#define ADDR_BRIDGE_PHY 128

#define SEL_PORT_ETH "eth"

#define SIG_COPPER 0
#define SIG_FIBER 1

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

enum loopback_num {
  GE_PHY_INT_LPBK = 0,
  GE_PHY_EXT_LPBK,
  GE2P5_PHY_INT_LPBK,
  GE2P5_PHY_EXT_LPBK,
  GE10_PHY_INT_LPBK,
  GE10_PHY_EXT_LPBK,
  GE10_PHY_SFP_INT_LPBK,
  GE10_PHY_SFP_EXT_LPBK,
};

enum eth_port_num {
    ETH_GE_PORT0 = 0,
    ETH_GE_PORT1,
    ETH_10GE_PORT0,
    ETH_10GE_PORT1,
    ETH_2P5GE_PORT0,
    ETH_2P5GE_PORT1,
    ETH_2P5GE_PORT2,
    ETH_2P5GE_PORT3,
	ETH_PORT_MAX,
};

enum eth_ge_port_num {
    GE_PORT0 = 0,
    GE_PORT1,
};

enum eth_2p5ge_port_num {
    GE2P5_PORT0 = 0,
    GE2P5_PORT1,
    GE2P5_PORT2,
    GE2P5_PORT3,
};

enum eth_10ge_port_num {
    GE10_PORT0 = 0,
    GE10_PORT1,
};

enum eth_10gsfp_port_num {
    SFP_PORT0 = 0,
    SFP_PORT1,
};

enum phy_port_num {
    PHY_PORT0 = 0,
    PHY_PORT1,
    PHY_PORT2,
    PHY_PORT3,
};

#endif /* __PLATFORM_ETH_H__ */
/*
 *------------------------------------------------------------------
 * $Log: platform_eth.h,v $
 * Revision 1.2  2019/06/14 05:24:50  mikech2
 * Collapse katar-branch00 to Main Trunk
 *
 * Revision 1.1.2.2  2019/05/29 05:59:17  mikech2
 * Code cleanup
 *
 * Revision 1.1.2.1  2019/02/12 08:06:29  mikech2
 * rename katar_*.h files
 *
 * Revision 1.1.2.2  2018/11/28 01:35:42  benlu
 * AQR412c config restore after corss test, link down retry, modify message
 *
 * Revision 1.1.2.1  2018/10/22 08:02:27  mikech2
 * Move project folder to common/src/katar/x86
 *
 * Revision 1.1.2.3  2018/10/08 03:36:17  mikech2
 * Modify pcie scan for different AQC100 FW
 *
 * Revision 1.1.2.2  2018/09/21 08:52:12  mikech2
 * Add cross-port & internal lpbk test util
 *
 * Revision 1.1.2.1  2018/07/10 09:43:37  benlu
 * phy internal/external loopback
 *
 *
 *
 *------------------------------------------------------------------
 * $Endlog$
 */

