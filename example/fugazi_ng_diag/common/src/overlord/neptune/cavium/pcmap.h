/* $Id: pcmap.h,v 1.2 2018/05/18 09:24:56 alpeng Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/overlord/neptune/cavium/pcmap.h,v $
 *------------------------------------------------------------------
 * pcmap.h - Overlord cavium cpu address space definition
 *
 * March 2011, Paul Tong
 * Copyright (c) 2011-2018 by Cisco Systems, Inc.
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
Revision 1.2  2018/05/18 09:24:56  alpeng
 Neptune merge to trunk with tag <neptune-branch-0518>

Revision 1.1.2.1  2016/06/06 05:58:51  xiaoyizh
Initial Check-in for Neptune Data Plane diags.

Revision 1.2  2012/03/28 00:38:18  mcharon
remove forward slash from second line

Revision 1.1.1.1  2012/03/23 23:02:07  ptong
Initial archive of ng_diag module


$Endlog$
*/
