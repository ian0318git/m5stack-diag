/* $Id: testcard_eth.h,v 1.3 2017/08/10 10:10:41 iachang Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/overlord/testcard_eth.h,v $
 *--------------------------------------------------------------------
 * Filename   : testcard_eth.h
 *
 * Description: Head file for TestCard ethernet related definition.
 *
 * Copyright (c) 2013-2017 by Cisco Systems, Inc.
 * All rights reserved.
 *--------------------------------------------------------------------
 */

#ifndef __TESTCARD_ETH_H__
#define __TESTCARD_ETH_H__

/* Common */
#define ETH_MAX_RETRY      10
#define GB_TC_MAX_ETH_PORT  1
#define TC_MAX_ETH_PORT     2

/* Speed Type */
#define TC_ETH_10BASE_T     0
#define TC_ETH_100BASE_T    1
#define TC_ETH_1000BASE_T   2

/* Loopback Tyep */
#define TC_ETH_INT_LPBK     1
#define TC_ETH_EXT_LPBK     2

/* SMI PHY Address */
#define TC_PHY_ADDR          0x4


/* Definition of Registers */
/*
 * Page 0
 */
#define PHY_PAGE_ADDR       22

/* Copper Control Register (Reg. 0) */
#define PHY_CCR_PAGE     0
#define PHY_CCR_ADDR     0
#define CCR_COPPER_RST   0x8000
#define CCR_LPBK         0x4000
#define CCR_AUTO_NEG     0x1000
#define CCR_PWR_DOWN     0x0800
#define CCR_COP_DUP      0x0100

#define CCR_10MBPS       0x0000
#define CCR_100MBPS      0x2000
#define CCR_1000MBPS     0x0040


/* 
 * Page 6
 */
#define PHY_CHK_CTRL_PAGE    6
#define PHY_CHK_CTRL        18

#define CHK_CTRL_REG_STUB_EN   0x0008

#define PHY_PAGE_NUM_MSK    0xFF


/* Externs */
extern int set_port_ext_lpbk_stub(uint, uint);


#endif /* __TESTCARD_ETH_H__ */

/* ------- End of file ------- */

/******** History ******** 
$Log: testcard_eth.h,v $
Revision 1.3  2017/08/10 10:10:41  iachang
CSCvf44161: Merge Goldbeach into USD platform as one image

Revision 1.2  2016/10/16 12:28:18  iachang
Supported Goldbeach Platform.

Revision 1.1  2013/05/09 05:42:40  alpeng
moving overlord common code from x86

Revision 1.3  2012/09/24 01:55:32  palin2
Add TestCard GE Internal loopback test support.

Revision 1.2  2012/08/22 16:39:55  palin2
Put XAUI into Reset state when exits XAUI related tests to avoid
affecting other interface.

Revision 1.1  2012/08/14 11:30:55  palin2
Removed "ovld_" from TestCard related filename because TestCard is not Overlord's unique.

Revision 1.2  2012/07/24 16:05:09  palin2
Add TestCard SGMII external loopback test support.

Revision 1.1  2012/07/23 17:33:55  palin2
Initial check-in for Overlord Test Card diag tests.


$Endlog$
*/

