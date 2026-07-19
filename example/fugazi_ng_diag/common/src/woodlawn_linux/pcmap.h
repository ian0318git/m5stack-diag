/* $Id: pcmap.h,v 1.2 2013/10/08 08:48:30 tirawan Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/woodlawn_linux/pcmap.h,v $
 *------------------------------------------------------------------
 * pcmap.h - Overlord cavium cpu address space definition
 *
 * March 2011, Paul Tong
 * Copyright (c) 2011-2013 by Cisco Systems, Inc.
 * All rights reserved.
 *------------------------------------------------------------------
 */
#ifndef __PCMAP_H__
#define __PCMAP_H__

/*------------------------------------------------------------*/
/*----    Overlord Cavium related defies    ------------------*/

#define CVMX_BOOTBUS_PHY_BASE_ADDR    0x8001000000000000ULL
#define DASH_FPGA_PHY_BASE_ADDR       CVMX_BOOTBUS_PHY_BASE_ADDR

/*------------------------------------------------------------*/

#endif /* !__PCMAP_H__ */

/*-------------------------------------------------
 * $Log: pcmap.h,v $
 * Revision 1.2  2013/10/08 08:48:30  tirawan
 * Woodlawn collapsed to main trunk
 *
 * Revision 1.1.4.2  2013/08/20 10:59:10  tirawan
 * Branch into woodlawn-branch2 and port woodlawn code
 *
 * Revision 1.1.2.1  2013/04/24 10:37:23  tirawan
 * Initial check-in for woodlawn linux code
 *
 * Revision 1.1  2013/03/13 06:43:00  kuangik
 * Add for the first time
 *
 * Revision 1.5  2012/08/03 10:16:56  evanli
 * Mapping to latest O2 source code on 20120726
 *
 * Revision 1.3  2012/07/25 01:34:59  leslie
 * Recover to previous revision 1.1.1.1
 *
 * Revision 1.1.1.1  2012/02/10 05:59:50  kody
 * Initial imports Woodlawn project code base.
 *
 * Revision 1.1.2.2  2011/12/21 23:33:50  ptong
 * Add FPGA register test
 *
 * Revision 1.1.2.1  2011/04/05 19:59:38  ptong
 * Initial checkin
 *
 * $Endlog$
 *-------------------------------------------------
 */
