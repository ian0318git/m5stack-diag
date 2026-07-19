/* $Id: pcmap.h,v 1.2 2012/03/28 00:38:18 mcharon Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/overlord/cavium/pcmap.h,v $
 *------------------------------------------------------------------
 * pcmap.h - Overlord cavium cpu address space definition
 *
 * March 2011, Paul Tong
 * Copyright (c) 2011-2012 by Cisco Systems, Inc.
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
$Log: pcmap.h,v $
Revision 1.2  2012/03/28 00:38:18  mcharon
remove forward slash from second line

Revision 1.1.1.1  2012/03/23 23:02:07  ptong
Initial archive of ng_diag module


$Endlog$
*/
