/* $Id: diag_common_drv.h,v 1.2 2013/10/08 08:48:27 tirawan Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/woodlawn_linux/diag_common_drv.h,v $
 *------------------------------------------------------------------
 * Filename: diag_common_drv.h
 *
 * Description: Device Driver Common Header Files
 * Author: Times Huang
 *
 * Copyright (c) 2013 by cisco Systems, Inc.
 * All rights reserved.
 *------------------------------------------------------------------
 */

#include "dev_object.h"

#ifndef DIAG_COMMON_DRV_H_
#define DIAG_COMMON_DRV_H_

#define SMI_ACCESS_RETRY_TIME       (5)

extern dev_object_t *diag_get_88e1340_obj(int);
extern int woodlawn_phy_reg_rd(int, int, int, int *);
extern int woodlawn_phy_reg_wr(int, int, int, int);
extern int get_smi_bus_id(int);

#define PHY_88E1112C_ID (0x18);

#define SMI_BUS_0 (0x0);
#define SMI_BUS_1 (0x1);
#define SMI_BUS_2 (0x2);
#define SMI_BUS_3 (0x3);

#endif /* DIAG_COMMON_DRV_H_ */



/*------------------------------------------------------------------
 * $Log: diag_common_drv.h,v $
 * Revision 1.2  2013/10/08 08:48:27  tirawan
 * Woodlawn collapsed to main trunk
 *
 * Revision 1.1.4.2  2013/08/20 10:58:50  tirawan
 * Branch into woodlawn-branch2 and port woodlawn code
 *
 * Revision 1.1.2.1  2013/04/24 10:37:14  tirawan
 * Initial check-in for woodlawn linux code
 *
 * Revision 1.1  2013/03/13 06:42:49  kuangik
 * Add for the first time
 *
 * Revision 1.6  2013/03/12 10:42:48  kuangik
 * Implement mdio read/write function which retries to anticipate race condition
 *
 * Revision 1.5  2013/02/18 07:49:00  leslie
 * Add smi bus macro
 *
 * Revision 1.4  2012/08/03 10:16:55  evanli
 * Mapping to latest O2 source code on 20120726
 *
 * Revision 1.2  2012/07/03 03:51:13  leslie
 * Add Extern Functions Declaration
 *
 * Revision 1.1  2012/04/06 06:10:15  kuangik
 * Add Common Drive File for the first time
 *
 *
 *
 *------------------------------------------------------------------
 * $Endlog$
 *
 *------------------------------------------------------------------
 */
