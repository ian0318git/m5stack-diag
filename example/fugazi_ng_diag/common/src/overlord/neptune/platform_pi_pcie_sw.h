/* $Id: platform_pi_pcie_sw.h,v 1.2 2018/05/18 09:24:52 alpeng Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/overlord/neptune/platform_pi_pcie_sw.h,v $
 *------------------------------------------------------------------
 *
 * platform_pi_pcie_sw.h - Neptune PCIe switch, Pericom PI7C9X2G1616PR,
 *                          related definitions.
 *
 * Copyright (c) 2016-2018 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#ifndef _PLATFORM_PLX_PCIE_SW_H_
#define _PLATFORM_PLX_PCIE_SW_H_

/* PLX8618 registers address */
#define UTP_BYTE0_3               0x210
#define UTP_BYTE4_7               0x214
#define UTP_BYTE8_11              0x218
#define UTP_BYTE12_15             0x21C
#define PHYSICAL_LAYER_CMD_EVEN_P 0x220
#define PHYSICAL_LAYER_CMD_ODD_P  0x224
#define SERDES_QUAD1_DIAG_DATA    0x248
#define SERDES_QUAD2_DIAG_DATA    0x244
#define SERDES_QUAD3_DIAG_DATA    0x24C
#define UTP_EN_SERDES0_7          0x258
#define UTP_EN_SERDES8_15         0x25C
#define PLX_LNK_STA_CTRL          0x98

/* Define the register values for loopback test setting */
#define PLX_PORT1_PHY_LPBK_MASTER_EN 0x1
#define PLX_PORT2_PHY_LPBK_MASTER_EN 0x10
#define PLX_PORT3_PHY_LPBK_MASTER_EN 0x10
#define PLX_PORT5_PHY_LPBK_MASTER_EN 0x100
#define PLX_PORT11_PHY_LPBK_MASTER_EN 0x100000
#define PLX_PORT13_PHY_LPBK_MASTER_EN 0x1000000

#define PLX_PORT1_PHY_LPBK_MASTER_READY 0x8
#define PLX_PORT2_PHY_LPBK_MASTER_READY 0x80
#define PLX_PORT3_PHY_LPBK_MASTER_READY 0x80
#define PLX_PORT5_PHY_LPBK_MASTER_READY 0x800
#define PLX_PORT11_PHY_LPBK_MASTER_READY 0x800000
#define PLX_PORT13_PHY_LPBK_MASTER_READY 0x8000000

#define PLX_SERDES4_UTP_EN 0x100000
#define PLX_SERDES5_UTP_EN 0x200000
#define PLX_SERDES8_UTP_EN 0x10000
#define PLX_SERDES9_UTP_EN 0x20000
#define PLX_SERDES12_UTP_EN 0x100000
#define PLX_SERDES13_UTP_EN 0x200000
#define PLX_SERDES14_UTP_EN 0x400000

#define PLX_SERDES4_8_12_DIAG_DATA_SELECT 0x0
#define PLX_SERDES5_9_13_DIAG_DATA_SELECT 0x1000000
#define PLX_SERDES14_DIAG_DATA_SELECT 0x2000000

#define UTP_ERR_COUNT_MSK       0x00FF0000
#define UTP_ERR_COUNT_OFS       16

/* Define the register values of PXL 8604 for Dagger */
#define PLX8604_P1_LPBK_EN 0x1
#define PLX8604_P5_LPBK_EN 0x100

#define PLX8604_P1_LPBK_READY 0x8
#define PLX8604_P5_LPBK_READY 0x800

#define PLX8604_ODDP_UTP_EN 0x10000

#define UTP_EN_ODDP_PLX8604 0x25C

#endif  /* _PLATFORM_PLX_PCIE_SW_H_ */


/*
 *------------------------------------------------------------------
 * $Log: platform_pi_pcie_sw.h,v $
 * Revision 1.2  2018/05/18 09:24:52  alpeng
 *  Neptune merge to trunk with tag <neptune-branch-0518>
 *
 * Revision 1.1.2.1  2016/06/01 23:15:06  jskow
 * Add PCIe switch Neptune files
 *
 * Revision 1.4  2014/05/28 14:08:24  danchung
 * Add more register definition of plx pcie switch
 *
 * Revision 1.3  2014/01/02 09:52:14  danchung
 * Fix testcard pcie loopback test fail on Dagger
 *
 * Revision 1.2  2013/11/19 13:40:52  danchung
 * Support testcard pcie lpbk test for Sword
 *
 * Revision 1.1  2013/10/22 14:32:34  danchung
 * Add support for PLX PCIe switch
 *
 *
 *------------------------------------------------------------------
 * $Endlog$
 */
