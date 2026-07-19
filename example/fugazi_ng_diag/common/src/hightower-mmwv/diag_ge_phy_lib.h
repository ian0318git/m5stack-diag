/* $Id: diag_ge_phy_lib.h,v 1.3 2021/06/02 02:56:21 alpeng Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/hightower-mmwv/diag_ge_phy_lib.h,v $
 *-----------------------------------------------------------------------------
 *
 * Filename   : platform_ge_phy.h
 * Description: Head file of Chrysler GE PHY(Marvell 88E1514) platform.
 *
 * Copyright (c) 2017~2020 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *-----------------------------------------------------------------------------
 */

#ifndef __PLATFORM_GE_PHY_H__
#define __PLATFORM_GE_PHY_H__

/* Externs */
extern int chrysler_gephy_reg_wr_util(int);
extern int chrysler_gephy_reg_rd_util(int);
extern int dnv_eth_link_is_up(int);
extern int check_ext_lpbk_flag(void);
extern int plat_mem_write32(uint, uint);

#define CN9130_SMI_REG         0xF212A200
#define CN9130_GE_PHY_INTR_REG 0xf2440154

#include "dev_88e151x.h"

#define SMIMR_REGAD_MSK      0x1F /* bit25:21 */
#define SMIMR_REGAD_OFFSET   (21) /* bit25:21 */
#define SMIMR_REGAD          (uint32_t)(SMIMR_REGAD_MSK << SMIMR_REGAD_OFFSET)

#define SMIMR_PHYAD_MSK      0x1F /* bit20:16 */
#define SMIMR_PHYAD_OFFSET   (16) /* bit20:16 */
#define SMIMR_PHYAD          (uint32_t)(SMIMR_PHYAD_MSK << SMIMR_PHYAD_OFFSET)

#define PLAT_SMI_RETRY_MAX   100

#define CN9130_SMI_BUSY               (1 << 28)
#define CN9130_SMI_READ_VALID         (1 << 27)
#define CN9130_SMI_OPCODE_RD          (1 << 26)

#define CHRYSLER_1514_GE_PHY_ADDR       0 

extern int diag_gephy_dev_create(int, dev_88e151x_object_t *);

#define ETH_PHY_3310_GE_UP           "ifconfig eth0 up > /dev/null"
#define ETH_PHY_1514_GE_UP           "ifconfig eth1 up > /dev/null"
#define ETH_PHY_1514_GE_DOWN           "ifconfig eth1 down > /dev/null"

#define LPBK_LINK_UP_TOUT               (500)
#define LPBKTEST_PKT_CNT                (3)
#define CHRYSLER_LPBK_RETRY              3
#define PANCR_SET_SGMII_1000      0x0040   /* 1 <<  6, 1000Mbps, Bit[6:5] = 10 */
#define PANCR_FORCE_LINK_UP       0x0002   /* 1 <<  1 */
#define PANCR_SET_FULL_DUPLEX     0x1000   /* 1 << 12 */
#define PANCR_RESERVED            0x8000   /* 1 << 15 */
#define PANCR_FORCE_LINK_DOWN     0x0001   /* 1 <<  0 */
#define CPU_PORT_AN_CONF_REG(m)   (uint)(0xF2130E0C + (m*0x1000))

#define CHRYSLER_88E1514_PHY 0
#define ETH1_DIS_AN  "ethtool -s eth1 duplex full autoneg off"
#define ETHTOOL_SPD_1000 "ethtool -s eth1 autoneg off speed 1000"
#define ETHTOOL_SPD_100 "ethtool -s eth1 autoneg off speed 100"
#define ETHTOOL_SPD_10 "ethtool -s eth1 autoneg off speed 10"

#define COPPER_PAGE_0 0
#define MAC_PAGE_2 2
#define SWITCH_PAGE 0x16

#define COPPER_CTRL_REG 0
#define COPPER_SPECIFIC_CTRL_REG 0x10
#define MAC_SPECIFIC_CTRL_REG 0x15

#define FORCE_LINK_UP_VAL 0x3460
#define DISABLE_FORCE_LINK_UP_VAL 0x3060
#define MAC_SPD_1000 0x1076
#define MAC_SPD_100 0x3036
#define MAC_SPD_10 0x1036
#define SOFT_RESET_SPD_100 0xa100
#define SOFT_RESET_SPD_10 0x8100
#define EN_LPBK_SPD_100 0x6100
#define EN_LPBK_SPD_10 0x4100
#define SPD_10 0
#define SPD_100 1
#define SOFT_RESET_VAL 0x8000

#define MAC_PAGE_3 3
#define LED_TIMRER_CTRL_REG 0x12 
#define GEPHY_CLEAR_INTR 0x4905
#define GEPHY_FORCE_INTR 0xC985
#define GEPHY_INTR_BIT   0x400   /* MPP42 */

#endif   /* __PLATFORM_GE_PHY_H__ */


/*-------------------------------------------------
 * $Log: diag_ge_phy_lib.h,v $
 * Revision 1.3  2021/06/02 02:56:21  alpeng
 * merge sears into trunk
 *
 * Revision 1.2  2021/01/25 09:21:46  markzha
 * Sync RDT issues fixing and optimize compiling for Highrise
 *
 * Revision 1.1  2020/08/19 09:50:04  markzha
 * *** empty log message ***
 *
 * $Endlog$
 *-------------------------------------------------
 */

