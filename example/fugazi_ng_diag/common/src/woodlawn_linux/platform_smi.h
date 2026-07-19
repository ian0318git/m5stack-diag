/* $Id: platform_smi.h,v 1.2 2013/10/08 08:48:31 tirawan Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/woodlawn_linux/platform_smi.h,v $
 *------------------------------------------------------------------
 * Header file for platform SMI code 
 * 
 * May 2012 Leslie Chen
 * Copyright (c) 2013 by Cisco Systems, Inc.
 * All rights reserved.
 *------------------------------------------------------------------
 */

 #include "platform_smi_lib.h"
 
#ifndef __PLATFORM_SMI_H__
#define __PLATFORM_SMI_H__

extern int read_ten_g_phy_reg(ulong, int, uint *, void *);
extern int write_ten_g_phy_reg(ulong, int, uint, void *);

#endif
/*-------------------------------------------------
 * $Log: platform_smi.h,v $
 * Revision 1.2  2013/10/08 08:48:31  tirawan
 * Woodlawn collapsed to main trunk
 *
 * Revision 1.1.4.2  2013/08/20 10:59:10  tirawan
 * Branch into woodlawn-branch2 and port woodlawn code
 *
 * Revision 1.1.2.1  2013/04/24 10:37:25  tirawan
 * Initial check-in for woodlawn linux code
 *
 * Revision 1.1  2013/03/13 06:43:02  kuangik
 * Add for the first time
 *
 * Revision 1.5  2012/10/24 10:46:30  leslie
 * Fix and clean up code.
 *
 * Revision 1.4  2012/08/03 10:16:56  evanli
 * Mapping to latest O2 source code on 20120726
 *
 * Revision 1.2  2012/05/16 02:32:57  leslie
 * Modify variable  type
 *
 * Revision 1.1  2012/05/11 05:46:28  leslie
 * Add SMI platform code header file
 *
 * $Endlog$
 *-------------------------------------------------
 */
