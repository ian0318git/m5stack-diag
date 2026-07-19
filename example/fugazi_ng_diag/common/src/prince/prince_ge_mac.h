/* $Id: prince_ge_mac.h,v 1.1 2013/04/19 07:17:51 xiaoyizh Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/prince/prince_ge_mac.h,v $
 *------------------------------------------------------------------
 * prince_ge_mac.h 
 *      Prince GE MAC definitions.
 *
 * Xiaoying Zhang -- Dec. 2012
 *
 * Copyright (c) 2012-2013 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */
 
#ifndef _GE_MAC_H_
#define _GE_MAC_H_

/* Define for system MAC control register */
#define SYS_MAC_RX_MTU          0xffff0000
#define SYS_MAC_GE_IFG          0x0000ff00
#define SYS_MAC_TX_FIFO_RST     0x00000010
#define SYS_MAC_RX_FIFO_RST     0x00000008
#define SYS_MAC_GE_LPBK         0x00000004
#define SYS_MAC_GE_PHY_RST      0x00000002
#define SYS_MAC_GE_RST          0x00000001

#define SYS_MAC_RX_MTU_SHIFT            16
#define SYS_MAC_GE_IFG_SHIFT            8
#define SYS_MAC_TX_FIFO_RST_SHIFT       4
#define SYS_MAC_RX_FIFO_RST_SHIFT       3
#define SYS_MAC_GE_LPBK_SHIFT           2
#define SYS_MAC_GE_PHY_RST_SHIFT        1
#define SYS_MAC_GE_RST_SHIFT            0

#define GE_RX_MTU               0x2800
#define GE_IFG_MIN              0x000c
#define GE_LPBK_MAC_INT                 1
#define GE_LPBK_MAC_EXT                 0

/* Defines for RX Configuration Word 0 */
#define MAC_RCVR_CFG0           0x0400
#define PF_MAC_ADDR0            0xffffffff

/* Defines for RX Configuration Word 1 */
#define MAC_RCVR_CFG1           0x0404
#define PF_MAC_ADDR1            0x0000ffff
#define CF_LEN_CHR_DIS          0x01000000
#define LT_ERR_CHK_DIS          0x02000000
#define RX_HALF_DPEX            0x04000000
#define RX_VLAN_EN              0x08000000
#define RX_EN                   0x10000000
#define RX_IB_FCS_EN            0x20000000
#define RX_JUMBO_FRAM_EN        0x40000000
#define RX_RST                  0x80000000

#define PF_MAC_ADDR1_SHIFT      0
#define CF_LEN_CHR_DIS_SHIFT    24
#define LT_ERR_CHK_DIS_SHIFT    25
#define RX_HALF_DPEX_SHIFT      26
#define RX_VLAN_EN_SHIFT        27
#define RX_EN_SHIFT             28
#define RX_IB_FCS_EN_SHIFT      29
#define RX_JUMBO_FRAM_EN_SHIFT  30
#define RX_RST_SHIFT            31

/* Defines for TX Configuration */
#define MAC_TX_CFG              0x0408
#define IFG_ADJ_EN              0x02000000
#define TX_HALF_DPEX            0x04000000
#define TX_VLAN_EN              0x08000000
#define TX_EN                   0x10000000
#define TX_IB_FCS_EN            0x20000000
#define TX_JUMBO_FRAM_EN        0x40000000
#define TX_RST                  0x80000000

#define IFG_ADJ_EN_SHIFT        25
#define TX_HALF_DPEX_SHIFT      26
#define TX_VLAN_EN_SHIFT        27
#define TX_EN_SHIFT             28
#define TX_IB_FCS_EN_SHIFT      29
#define TX_JUMBO_FRAM_EN_SHIFT  30
#define TX_RST_SHIFT            31

/* Defines for MAC Flow control reg */
#define MAC_FLOW_CTRL           0x040c
#define FLOW_CTRL_RX_EN         0x20000000
#define FLOW_CTRL_TX_EN         0x40000000

#define FLOW_CTRL_RX_EN_SHIFT   29
#define FLOW_CTRL_TX_EN_SHIFT   30

#define MAC_SPEED_CFG           0x0410

/* Defines for MAC RX max fram reg */
#define MAC_RX_MAX_FRAME        0x0414
#define MAC_RX_MF_LEN           0x000007ff
#define MAC_RX_MF_EN            0x00010000

#define MAC_RX_MF_LEN_SHIFT     0
#define MAC_RX_MF_EN_SHIFT      16

/* Defines for MAC TX max fram reg */
#define MAC_TX_MAX_FRAME        0x0418
#define MAC_TX_MF_LEN           0x000007ff
#define MAC_TX_MF_EN            0x00010000

#define MAC_TX_MF_LEN_SHIFT     0
#define MAC_TX_MF_EN_SHIFT      16


#define MAC_ID                  0x04f8
#define MAC_ABILITY             0x04fc

#define MAC_MDIO_CFG0           0x0500
#define MAC_MDIO_CLK_DIV        0x0000002f
#define MAC_MDIO_EN             0x00000040

/* Definition for MAC MDIO Registers */
#define MAC_MDIO_CFG0           0x0500
#define MAC_MDIO_CLK_DEV        0x0000003f
#define MAC_MDIO_EN             0x00000040
#define MAC_MDIO_CLK_DEV_SHIFT  0
#define MAC_MDIO_EN_SHIFT       6

#define MAC_MDIO_CFG1           0x0504
#define MAC_MDIO_READY          0x00000080
#define MAC_MDIO_INIT           0x00000800
#define MAC_MDIO_TX_OP          0x0000c000
#define MAC_MDIO_TX_REGAD       0x001f0000
#define MAC_MDIO_TX_PHYAD       0x1f000000

#define MDIO_READY_SHIFT        7
#define MDIO_INIT_SHIFT         11
#define MDIO_TX_OP_SHIFT        14
#define MDIO_TX_REGAD_SHIFT     16
#define MDIO_TX_PHYAD_SHIFT     24

#define MAC_MDIO_TX_DATA        0x0508
#define MAC_MDIO_WR_DATA        0x0000ffff
#define MDIO_WR_DATA_SHIFT      0

#define MAC_MDIO_RX_DATA        0x050c
#define MAC_MDIO_RD_DATA        0x0000ffff
#define MDIO_RD_DATA_SHIFT      0
#define MAC_MDIO_READY_CP       0x00010000

#define MAC_MDIO_INTR_MASK      0x00000001

#define MAC_INTR_STATUS         0x0600
#define MAC_INTR_PENDING        0x0610
#define MAC_INTR_ENABLE         0x0620
#define MAC_INTR_CLEAR          0x0630

#define PRINCE_PHY_ADDR         0x0
#define MDIO_WAIT_LOOP          2000
#define MDIO_DELAY              20

extern int ge_mac_init(void);
extern void set_mac_loopback(int);

extern int ge_mac_mdio_read(ushort, ushort *);
extern int ge_mac_mdio_write(ushort, ushort *);

#endif //_GE_MAC_H_

/******** History ********
$Log: prince_ge_mac.h,v $
Revision 1.1  2013/04/19 07:17:51  xiaoyizh


Initial check in for Prince NIM.


$Endlog$
*/
