/* $Id: skye_xaui.h,v 1.2 2015/05/25 03:59:11 steja Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/skye_linux/include/skye_xaui.h,v $
 *------------------------------------------------------------------
 * Filename: skye_xaui.h
 *
 * Description: XAUI interface Common Header Files
 *
 * Jul 2014 - Paul Lin(palin2) ported from Shrinkray,
 *            original author is Sofian(steja).
 *
 * Copyright (c) 2013-2015 by cisco Systems, Inc.
 * All rights reserved.
 *------------------------------------------------------------------
 */

#ifndef SHRINKRAY_XAUI_H_
#define SHRINKRAY_XAUI_H_

#define SHRINKRAY_DIAG_IP_ADDR_SUBNET "192.123.123"
#define SHRINKRAY_DIAG_IP_ADDR_BASE  (100)

#define INFO_LEN                (10000)
#define EN_XAUI_EXT_LPBK        (1)
#define DIS_XAUI_EXT_LPBK       (0)

#define SPD_10000MBPS   10000
#define SEL_PORT_XAUI   "xgbe"

#define TLK10G    0x01
#define NONTLK10G 0x0F

extern void config_bp_xaui(void);
extern void config_xaui(void);
extern int xaui_lpbk_test(int);
extern int set_xaui_ext_lpbk(boolean);
extern int enable_xaui1_interface(void);

#endif /* SHRINKRAY_XAUI_H_ */

/*
 *------------------------------------------------
 * $Log: skye_xaui.h,v $
 * Revision 1.2  2015/05/25 03:59:11  steja
 * Add Support Skye SM
 *
 * Revision 1.1.4.2  2015/04/29 11:36:29  steja
 * Code check-in to skye-branch2 for ER code review
 *
 *
 * ------------------------------------------------------------------
 * Revision 1.1.2.2  2014/08/12 12:33:14  steja
 * Update 10GKR loopback test code
 *
 * Revision 1.1.2.1  2014/07/21 01:56:40  palin2
 * Initial check-in Skye module side Diag code.
 *
 *------------------------------------------------
 * shinkray_xaui.h: 
 * Revision 1.2  2014/02/27 15:01:10  palin2
 * Initial check-in ShrinkRay SM side Diag code.
 *
 * Revision 1.1.4.4  2013/10/05 06:20:23  steja
 * Update for debug
 *
 * Revision 1.1.4.3  2013/09/27 07:25:13  steja
 * update code for bringup
 *
 * Revision 1.1.4.2  2013/09/13 07:00:01  palin2
 * Initial check-in ShrinkRay SM side Diag code.
 *
 * Revision 1.1.2.3  2013/08/30 09:05:46  steja
 * Fix the typo define
 *
 * Revision 1.1.2.2  2013/07/18 13:09:45  steja
 * define gbe for use in Evaluation board
 *
 * Revision 1.1.2.1  2013/06/24 09:03:35  steja
 * Checkin :
 * - Support TLK10323 Loopback test & Utility
 * - Support MV1514 Loopback test
 *
 *------------------------------------------------
 * $Endlog$
 */
