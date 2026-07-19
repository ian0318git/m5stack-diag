/* $Id: skye_smi_lib.h,v 1.2 2015/05/25 03:59:11 steja Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/skye_linux/include/skye_smi_lib.h,v $
 *------------------------------------------------------------------
 * Header file for Service module SMI lib code
 * 
 * Jul 2014 - Paul Lin(palin2) ported from Shrinkray,
 *            original author is Sofian(steja).
 *
 * Copyright (c) 2013-2015 by Cisco Systems, Inc.
 * All rights reserved.
 *------------------------------------------------------------------
 */
 
#ifndef __SHRINKRAY_SMI_LIB_H__
#define __SHRINKRAY_SMI_LIB_H__

typedef struct ten_g_phy_t_ {
    unsigned int device_id;
    unsigned int port_id;
} ten_g_phy_t;

#endif


/*-------------------------------------------------
 * $Log: skye_smi_lib.h,v $
 * Revision 1.2  2015/05/25 03:59:11  steja
 * Add Support Skye SM
 *
 * Revision 1.1.4.2  2015/04/29 11:36:28  steja
 * Code check-in to skye-branch2 for ER code review
 *
 *
 * ------------------------------------------------------------------
 * Revision 1.1.2.1  2014/07/21 01:56:40  palin2
 * Initial check-in Skye module side Diag code.
 *
 *-------------------------------------------------
 * shrinkray_smi_lib.h:
 * Revision 1.2  2014/02/27 15:01:09  palin2
 * Initial check-in ShrinkRay SM side Diag code.
 *
 * Revision 1.1.4.2  2013/09/13 07:00:00  palin2
 * Initial check-in ShrinkRay SM side Diag code.
 *
 * Revision 1.1.2.1  2013/06/24 09:03:35  steja
 * Checkin :
 * - Support TLK10323 Loopback test & Utility
 * - Support MV1514 Loopback test
 *
 *-------------------------------------------------
 * $Endlog$
 */
