/* $Id: diag_ge_phy.h,v 1.4 2019/01/24 01:07:22 letsai Exp $ 
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/tsn/diag_ge_phy.h,v $
 *-----------------------------------------------------------------------------
 *
 * Filename   : diag_ge_phy.h
 * Description: Head file of TSN GE PHY(Marvell 88E1112) Diag.
 *
 * Copyright (c) 2017 - 2019 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *-----------------------------------------------------------------------------
 */

#ifndef __DIAG_GE_PHY_H__
#define __DIAG_GE_PHY_H__ 

/* Common */
#define GEWAN_TXTYPE_A   1
#define GEWAN_TXTYPE_B   0

#define CPU_MAC_1000BASEX_MODE_REG   (0x02)

#define PHY_88E1112_GE0_SMIADDR  0x1D /* instead of PLAT_GE0_SMIADDR */
#define PHY_88E1112_GE1_SMIADDR  0x1C /* instead of PLAT_GE1_SMIADDR */

#define GE0 0
#define GE1 1

#define MRV88E111N_ENABLE_INTR  0xe1e
#define MRV88E111N_DISABLE_INTR 0x21e

#define COMPARE_AND     0 /* read data & pattern == 0       */
#define COMPARE_EQL     1 /* read data           == pattern */
#define COMPARE_AND_EQL 2 /* read data & pattern == pattern */

#define INTR_POLLING_PERIOD        5 /* 5ms */
#define INTR_POLLING_ROUND         1000

#define MAX_POLLING_ROUND          2000
#define POLLING_PERIOD             5 /* 5ms */
#define MRV88E1112_PHY_INTR_BUFFERING      3 /* 3 second */

/* Externs */
extern int  ge_phy_88E1112C_test(int);
extern int  ge_phy_88E1112C_loopback_test(int);
extern int  tsn_set_gephy_txtype(int, ushort);
extern int ge_phy_copper_ext_lpbk_test(int);
extern int ge_send_packet_util (int);
extern int gephy_set_test_mode(int);
extern int gephy_set_txtype_util(int);
extern int gephy_set_default(int);
extern int gephy_set_1000basex_mode (int);
extern int gephy_set_sgmii_mode (int);
extern int gephy_set_loopback_mode (int, int);
extern int gephy_get_loopback_mode (int);
extern int has_mrvl_88e6112(void);

#endif   /* __DIAG_GE_PHY_H__ */


/*-------------------------------------------------
 * $Log: diag_ge_phy.h,v $
 * Revision 1.4  2019/01/24 01:07:22  letsai
 * Add Supernova GE0/ESW Interrupt Test (CSCvo04335).
 *
 * Revision 1.3  2018/02/09 09:56:53  hondwang
 * Merge Star branch star-branch-c9xx to main trunk
 *
 * Revision 1.2.20.1  2018/01/20 06:27:23  hondwang
 * prepare merge star-branch-c9xx to main trunk
 *
 * Revision 1.2.4.4  2017/09/15 04:30:38  lucywang
 * added utility to enable/diable motherboard line loopback for Pluggable Serial GE port, not work yet
 *
 * Revision 1.2.4.3  2017/08/23 05:46:33  lucywang
 * enable/disable Receiver to Tansmitter in local PHY for pluggable serial module
 *
 * Revision 1.2.4.2  2017/08/22 03:29:59  lucywang
 * set 1000Base-X for pluggable serial and set sgmii for pluggable test card
 *
 * Revision 1.2.4.1  2017/08/15 14:18:38  hondwang
 * star branch c9xx initial check in
 *
 * Revision 1.2  2017/08/02 14:21:45  steja
 * Support TSN-H/M platform code
 *
 * Revision 1.1.8.2  2017/07/29 03:41:02  steja
 * tsn-branch5 synced with Maintrunk repositories
 *
 * Revision 1.1.6.2  2017/07/20 13:38:04  steja
 * tsn-branch4 merge with maintrunk
 *
 * Revision 1.1.4.3.6.1  2017/06/17 12:13:07  hondwang
 * Add test card phy testing function
 *
 * Revision 1.1.4.3  2016/07/17 10:52:56  palin2
 * 1. Added function and utility to set GE WAN PHY Transmitter Type.
 * 2. Clean up code.
 *
 * Revision 1.1.4.2  2016/06/30 06:22:47  steja
 * tsn-branch2 sync with main trunk
 *
 * Revision 1.1.2.1  2016/03/29 02:50:02  palin2
 * Added GE PHY Diag.
 *
 * $Endlog$
 *-------------------------------------------------
 */

