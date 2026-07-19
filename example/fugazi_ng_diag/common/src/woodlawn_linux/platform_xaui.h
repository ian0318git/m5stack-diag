/* $Id: platform_xaui.h,v 1.2 2013/10/08 08:48:32 tirawan Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/woodlawn_linux/platform_xaui.h,v $
 *------------------------------------------------------------------
 * Filename: platform_xaui.h
 *
 * Description: XAUI interface Common Header Files
 * Author: Kody Ko
 *
 * Copyright (c) 2013 by cisco Systems, Inc.
 * All rights reserved.
 *------------------------------------------------------------------
 */

#ifndef PLATFORM_XAUI_LIB_H_
#define PLATFORM_XAUI_LIB_H_

#define WOODLAWN_DIAG_IP_ADDR_SUBNET "192.123.123"
#define WOODLAWN_DIAG_IP_ADDR_BASE  (100)

#define INFO_LEN                (10000)
#define EN_XAUI_EXT_LPBK        (1)
#define DIS_XAUI_EXT_LPBK       (0)

#define SPD_10000MBPS   10000
#define SEL_PORT_XAUI "xaui"

extern void config_bp_xaui(void);
extern void config_xaui(void);
extern int xaui_lpbk_test(int);
extern int set_xaui_ext_lpbk(boolean);
extern int enable_xaui1_interface(void);

#endif /* PLATFORM_XAUI_LIB_H_ */

/*
 * $Log: platform_xaui.h,v $
 * Revision 1.2  2013/10/08 08:48:32  tirawan
 * Woodlawn collapsed to main trunk
 *
 * Revision 1.1.4.2  2013/08/20 10:59:11  tirawan
 * Branch into woodlawn-branch2 and port woodlawn code
 *
 * Revision 1.1.2.1  2013/04/24 10:37:25  tirawan
 * Initial check-in for woodlawn linux code
 *
 * Revision 1.3  2013/04/01 04:09:55  kuangik
 * Assign XAUI BP IP address based on slot number
 *
 * Revision 1.6  2013/03/07 12:36:57  leslie
 * Modify for 10G loopback test.
 *
 * Revision 1.5  2012/11/08 02:50:45  kody
 * Add enable QLM2 XAUI ext-loopback for O2 backplane XAUI loopback test.
 *
 * Revision 1.4  2012/09/21 11:54:46  kody
 * Woodlawn use xaui1 to do the lpbk test.
 *
 * Revision 1.3  2012/08/03 10:16:56  evanli
 * Mapping to latest O2 source code on 20120726
 *
 * Revision 1.1  2012/05/18 10:08:44  kody
 * Add platform header code for XAUI interface.
 *
 * $Endlog$
 */
