/* $Id: platform_esw.h,v 1.6 2019/01/24 01:07:22 letsai Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/tsn/platform_esw.h,v $
 *------------------------------------------------------------------
 * 
 * Filename   : platform_esw.h
 * Description: Platform specific header file of TSN ethernet switch,
 *              Marvell 88E6390.
 *
 * Copyright (c) 2016 ~ 2019 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#ifndef __PLATFORM_ESW_H__
#define __PLATFORM_ESW_H__

/* Common */
#define TSN_ESW_PORTS   8

#define ESW_PORT0       0
#define ESW_PORT1       1
#define ESW_PORT2       2
#define ESW_PORT3       3
#define ESW_PORT4       4
#define ESW_PORT5       5
#define ESW_PORT6       6
#define ESW_PORT7       7
#define ESW_PORT8       8
#define ESW_PORT9       9
#define ESW_PORT10      10

#define ESW_VLAN1       1
#define ESW_VLAN2       2
#define ESW_VLAN3       3
#define ESW_VLAN4       4

#define MRVL88E6390_MCA_SMI_CMD_REG    0x0
#define MRVL88E6390_MCA_SMI_DATA_REG   0x1

#define TSN_ESW_SMI_CMD_REG    MRVL88E6390_MCA_SMI_CMD_REG
#define TSN_ESW_SMI_DATA_REG   MRVL88E6390_MCA_SMI_DATA_REG

#define MRVL88E6176_PORT_REG_BASE      0x10
#define TSN_M_ESW_PORT_REG_BASE        MRVL88E6176_PORT_REG_BASE
#define MRVL88E6176_SERDES_REG_ADDR    0xF
#define TSN_M_ESW_CPU_PORT_ADDR        MRVL88E6176_SERDES_REG_ADDR

#define ALL_ESW_LEDS         0xF
#define ESW_LED_F_ON         1
#define ESW_LED_F_OFF        0 

#define ESW_RESET_ONE_SEC    1000   /* 1sec = 1000ms */

#define MRVL88E6176_INTR_POLLING_PERIOD        50 /* 50ms */
#define MRVL88E6176_INTR_POLLING_ROUND         1000

/* Multi Chip Addressing mode */
/* SMI command register(0x0) */
#define SMI_CMD_SMIBUSY       (1 << 15)
#define SMI_CMD_SMIMODE_C22   (1 << 12)
#define SMI_CMD_SMIMODE_C45   (0 << 12)
#define SMI_CMD_SMIOP_RD      (1 << 11)   /* 11:10 0x2 Read Data Reg. */
#define SMI_CMD_SMIOP_WR      (1 << 10)   /* 11:10 0x1 Write Data Reg. */
#define SMIOP_C45_WR_ADDR     (0 << 10)   /* [11:10] C45 0x0 Write Addr. Reg. */
#define SMIOP_C45_WR_DATA     (1 << 10)   /* [11:10] C45 0x1 Write Data Reg. */
#define SMIOP_C45_RD_DATA_PI  (2 << 10)   /* [11:10] C45 0x2 Read Data Reg. with post increament on Addr. Reg. */
#define SMIOP_C45_RD_DATA     (3 << 10)   /* [11:10] C45 0x3 Read Data Reg. */

/* ESW SMI Device Register MAP */
/* Device Addr. */
#define ESW_SMIDEV_GLOB1      0x1B
#define ESW_SMIDEV_GLOB2      0x1C

#define ESW_PHYCTR_REG        0x1
#define ESW_PORTCTR_REG       0x4
#define ESW_PORT_VLAN_REG     0x6
#define ESW_PORTVLAN_ID_REG   0x7
#define ESW_PORTCTR2_REG      0x8

#define ESW_PCR_PSTAT_MSK     0x3
#define ESW_PCR_DISABLE       0x0
#define ESW_PCR_BLOCKING      0x1
#define ESW_PCR_LEARNING      0x2
#define ESW_PCR_FORWARD       0x3

/* ESW Port Status Reg.(0x0) */
#define ESW_PSR_ADDR             0x0
#define ESW_PSR_LINKUP           (1 << 11)
#define ESW_PSR_LINK             (1 << 11)
#define ESW_PSR_FULLDPX          (1 << 10)
#define ESW_PSR_DPX              (1 << 10)
#define ESW_PSR_SPD_MSK          0x0300
#define ESW_PSR_10MBPS           0x0000
#define ESW_PSR_100MBPS          0x0100
#define ESW_PSR_1000MBPS         0x0200
#define ESW_PSR_10GBPS           0x0300

/* ESW Physical Control Reg.(0x1) */
#define ESW_PCR_ADDR             0x1
#define ESW_PCR_RGMII_RX_DELAY   (1 << 15)
#define ESW_PCR_RGMII_TX_DELAY   (1 << 14)
#define ESW_PCR_FORCE_SPEED      (1 << 13)
#define ESW_PCR_MII_MAC_MODE     (0 << 11)
#define ESW_PCR_MII_PHY_MODE     (1 << 11)
#define ESW_PCR_F_LINKUP         (1 << 5)
#define ESW_PCR_FORCE_LINK       (1 << 4)
#define ESW_PCR_F_FULLDPX        (1 << 3)
#define ESW_PCR_FORCE_DPX        (1 << 2)
#define ESW_PCR_10MBPS           0x0
#define ESW_PCR_100MBPS          0x1
#define ESW_PCR_1000MBPS         0x2
#define ESW_PCR_10GBPS           0x3
#define ESW_PCR_DEF_VAL          0x3

/* ESW Port Control Reg.(0x4) */
#define ESW_PCR_PS_MSK           0x3
#define ESW_PCR_PORT_DIS         0x0
#define ESW_PCR_PORT_FORWARD     0x3

/* ESW Port Based VLAN Map Reg.(0x6) */
#define ESW_PBVM_VLAN_TBL_MSK    0x7F
#define ESW_PBVM_VLAN_TBL(x)     (1 << x)

/* ESW Default Port VLAN ID & Priority */
#define ESW_PVID_FORCE_DVID      (1 << 12)
#define ESW_PVID_DVID_MSK        0xFFF

/* ESW Port Control 2 Reg.(0x08) */
#define ESW_PCR2_8021Q_MODE_MSK  (0x3 << 10)
#define ESW_PCR2_8021Q_SECURE    (0x3 << 10)

/* Global 2(0x1C) SMI register addr. */
#define ESW_GLOB2_PC          0x18   /* SMI PHY Command */
#define ESW_GLOB2_PD          0x19   /* SMI PHY Data */

/* ESW GE PHY Register Map */
#define ESW_GEPHY_PAGE_ADDR   22

/* ESW Copper Control register Map */
#define ESW_CCR_PWRDWN        (1 << 11)

/* ESW LED Control(0x16) register */
#define ESW_LED_CONTR_REG     0x16
#define ESW_LCR_UPDATE        (1 << 15)
#define ESW_LCR_LED1_F_ON     (0xF << 4)
#define ESW_LCR_LED0_F_ON     0xF
#define ESW_LCR_LED1_F_OFF    (0xE << 4)
#define ESW_LCR_LED0_F_OFF    0xE

/* ESW SGMII register Map */
#define ESW_SGMII_DEVNUM      4
#define ESW_SGMII_CONTR_REG   0x2000
#define ESW_SGMII_PWRDWN      (1 << 11)

/* ESW Global 1 Reg. Map */
#define ESW_G1_VTUFID_REG            0x2
#define ESW_G1_VTUOP_REG             0x5
#define ESW_G1_VTUVID_REG            0x6
#define ESW_G1_VTUDATA_0TO7_REG      0x7
#define ESW_G1_VTUDATA_8TO10_REG     0x8

#define ESW_G1_VID_ENTRY_VALID       (1 << 12)

#define ESW_G1_DATA_FRAME_UNTAGGED   0x1
#define ESW_G1_MEMBER_STATE_MSK      0x3

#define ESW_G1_OP_VTUBUSY            (1 << 15)
#define ESW_G1_OP_VTULOAD            (3 << 12)

/* ESW PHY */
#define ESW_ALL_PHY_PORTS            0xA

#define ESW_SET_PORT_10M             0x1
#define ESW_SET_PORT_100M            0x2
#define ESW_SET_PORT_1G              0x3

#define ESW_SET_PORT_HD              0x0
#define ESW_SET_PORT_FD              0x1

/* Copper Control Reg. (0_0) */
#define ESWPHY_CCR_ADDR              0x0
#define ESWPHY_CCR_COP_RST           (1 << 15)
#define ESWPHY_CCR_LPBK              (1 << 14)
#define ESWPHY_CCR_AN_EN             (1 << 12)

/* Copper Auto-Negotitation Advertisement Reg. (4_0) */
#define ESWPHY_CANAR_ADDR            0x4
#define ESWPHY_COP_ANAR_100FD        (1 << 8)
#define ESWPHY_COP_ANAR_100HD        (1 << 7)
#define ESWPHY_COP_ANAR_10FD         (1 << 6)
#define ESWPHY_COP_ANAR_10HD         (1 << 5)

/* 1000BASE-T Control Reg. (9_0) */
#define ESWPHY_1000TCR_ADDR          0x9
#define ESWPHY_1G_CNTR_1000FD        (1 << 9)
#define ESWPHY_1G_CNTR_1000HD        (1 << 8)

/* Copper Specific Status Reg1 (17_0) */
#define ESWPHY_CSSR1_SPEED           (3 << 14)
#define ESWPHY_CSSR1_DUPLEX          (1 << 13)
#define ESWPHY_CSSR1_RESOLVED        (1 << 11)
#define ESWPHY_CSSR1_COP_LINK        (1 << 10)
#define ESWPHY_CSSR1_LINK_STAT       (1 << 3)

#define ESWPHY_CSSR1_SPD_1000MBPS    0x8000
#define ESWPHY_CSSR1_SPD_100MBPS     0x4000
#define ESWPHY_CSSR1_SPD_10MBPS      0x0000
#define ESWPHY_CSSR1_FULLDUP         0x2000
#define ESWPHY_CSSR1_RT_LINK_UP      0x0400
#define ESWPHY_CSSR1_COP_LINK_UP     0x0008

/* MAC Specific Control Reg2 (21_2) */
#define ESWPHY_MSCR2_MACSPD_MSK        0x7   /* 21_2.2:0 */
#define ESWPHY_MSCR2_MACSPD_10MBPS     0x4   /* 10 MBPS  (21_2.2:0 = 100) */
#define ESWPHY_MSCR2_MACSPD_100MBPS    0x5   /* 100 MBPS (21_2.2:0 = 101) */
#define ESWPHY_MSCR2_MACSPD_1000MBPS   0x6   /* 1000 MBPS(21_2.2:0 = 110) */

/* Checker Control Reg. (18_6) */
#define ESWPHY_CHKREG_STUB_EN_BIT      3
#define ESWPHY_CHKREG_STUB_EN          0x0008
#define ESWPHY_CHKREG_STUB_EN_MSK      0x0008

#define DATA_REG      (5)   /* Port 5 Reg 0x1A is the corresponding data reg */
#define CMD_REG       (4)   /* Port 4 Reg 0x1A is a command reg */
#define REG_RSVD      (0x1A) /* Reserved Register, based Marvell Errata  */
/* Port 5 Reg 0x1A (Block 0xF Index 0x0) */
#define ERRATA_REG_0X1C0 (0x01c0)   /* Must be 0x1C0 */
/* Port 4 Reg 0x1A Debug Control */
#define ERRATA_DBG_CTRL_0XFC00 (0xFC00)   /* Must be 0xFC00 */
#define ERRATA_DBG_CTRL_0XFC20 (0xFC20)   /* Must be 0xFC20 */
#define ERRATA_DBG_CTRL_0XFC40 (0xFC40)   /* Must be 0xFC40 */
#define ERRATA_DBG_CTRL_0XFC60 (0xFC60)   /* Must be 0xFC60 */
#define ERRATA_DBG_CTRL_0XFC80 (0xFC80)   /* Must be 0xFC80 */
#define ERRATA_DBG_CTRL_0XFCA0 (0xFCA0)   /* Must be 0xFCA0 */
#define ERRATA_DBG_CTRL_0XFCC0 (0xFCC0)   /* Must be 0xFCC0 */
#define ERRATA_DBG_CTRL_0XFCE0 (0xFCE0)   /* Must be 0xFCE0 */
#define ERRATA_DBG_CTRL_0XFD00 (0xFD00)   /* Must be 0xFD00 */
#define ERRATA_DBG_CTRL_0XFD20 (0xFD20)   /* Must be 0xFD20 */
#define ERRATA_DBG_CTRL_0XFD40 (0xFD40)   /* Must be 0xFD40 */

/* Externs */
extern int tsn_esw_reg_rd(int, int, ushort *);
extern int tsn_esw_reg_wr(int, int, ushort);
extern int tsn_esw_force_led_onoff(int, boolean);
extern int tsn_reset_esw_to_default(int);
extern int tsn_esw_phy_reg_rd(int, int, int, ushort *);
extern int tsn_esw_phy_reg_wr(int, int, int, ushort);
extern int has_mrvl_88e6176(void);

#endif   /* __PLATFORM_ESW_H__ */

/*------------------------------------------------------------------
$Log: platform_esw.h,v $
Revision 1.6  2019/01/24 01:07:22  letsai
Add Supernova GE0/ESW Interrupt Test (CSCvo04335).

Revision 1.5  2018/05/21 09:00:13  steja
Fix these two issues.
1. CSCvj11429 - Found intermittent GE Switch External Loopback
2. CSCvj11436 - Found GE Switch Ext. loopback fail after use permutation test

Revision 1.4  2018/01/23 11:38:19  steja
Merge tsn-gfast-branch4 code to maintrunk for support TSN-G.Fast (CSCvh40981)

Revision 1.3.4.1  2018/01/23 09:56:53  palin2
Enhanced code readability.

Revision 1.3  2017/12/01 13:50:34  palin2
Fixed CSCvg97205: Added force Switch MAC and PHY speed function back to external loopback test.

Revision 1.2  2017/08/02 14:21:48  steja
Support TSN-H/M platform code

Revision 1.1.8.2  2017/07/29 03:41:19  steja
tsn-branch5 synced with Maintrunk repositories

Revision 1.1.6.2  2017/07/20 13:38:06  steja
tsn-branch4 merge with maintrunk

Revision 1.1.4.10.2.1  2017/07/17 13:54:44  palin2
Code cleanup.

Revision 1.1.4.10  2016/09/28 04:36:15  palin2
Added CPU to ESW PHY MAC loopback test.

Revision 1.1.4.9  2016/09/23 07:09:37  palin2
Added TSN-M Switch VLANs config. profile for compliance team.

Revision 1.1.4.8  2016/07/29 14:27:47  palin2
Added utility and function to config. Switch port to specific speed and mode.

Revision 1.1.4.7  2016/07/26 16:05:24  palin2
Fixed ESW VLANs setup function.

Revision 1.1.4.6  2016/07/10 10:29:34  steja
Add LED test

Revision 1.1.4.5  2016/07/05 14:26:51  palin2
Added utililty to force ON/OFF TSN Switch port LED(s).

Revision 1.1.4.4  2016/07/04 15:29:28  palin2
1. Updated TSN-M Switch part related code after bring up.
2. Added utility to change LAN PHY port VOD setup for HW.

Revision 1.1.4.3  2016/06/30 14:06:31  steja
Pick up the latest from tsn-branch1

Revision 1.1.4.2  2016/06/30 06:22:50  steja
tsn-branch2 sync with main trunk

Revision 1.1.2.6  2016/06/29 14:14:51  palin2
1. Updated code to support TSN-M.
2. Added utility to set LAN PHY 1000Base-T Test mode.

Revision 1.1.2.5  2016/06/15 14:46:16  palin2
Added utilities to config Switch VLAN and forwarding for extended feature.

Revision 1.1.2.4  2016/06/13 16:55:21  palin2
Added Diag extending feature, Switch forwarding and VLAN for 8 ports snake test.

Revision 1.1.2.3  2016/05/26 03:09:23  palin2
Added TSN Switch init function, and SMI C45 read/write utility.

Revision 1.1.2.2  2016/05/03 16:00:57  palin2
1. Added Switch register test.
2. Added Switch external loopback test to support 10 and 100Mbps speed.

Revision 1.1.2.1  2016/04/29 10:14:57  palin2
Updated code and added support ext. loopback test after bring up Switch.

$Endlog$
*/

