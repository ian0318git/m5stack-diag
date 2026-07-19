/* $Id: diag_common_drv.h,v 1.2 2015/05/25 03:59:10 steja Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/skye_linux/include/diag_common_drv.h,v $
 *------------------------------------------------------------------
 * Filename: diag_common_drv.h
 *
 * Description: Device Driver Common Header Files
 * Author: Sofian Teja
 *
 * Copyright (c) 2013-2015 by cisco Systems, Inc.
 * All rights reserved.
 *------------------------------------------------------------------
 */


#ifndef DIAG_COMMON_DRV_H_
#define DIAG_COMMON_DRV_H_

#define SMI_ACCESS_RETRY_TIME       (5)
extern int skye_phy_reg_rd(int, int, int, int *);
extern int skye_phy_reg_wr(int, int, int, int);
extern int get_smi_bus_id(int);


#define SMI_BUS_0 (0x0);
#define SMI_BUS_1 (0x1);
#define SMI_BUS_2 (0x2);
#define SMI_BUS_3 (0x3);

#endif /* DIAG_COMMON_DRV_H_ */


/*------------------------------------------------------------------
 * $Log: diag_common_drv.h,v $
 * Revision 1.2  2015/05/25 03:59:10  steja
 * Add Support Skye SM
 *
 * Revision 1.1.4.2  2015/04/29 11:36:24  steja
 * Code check-in to skye-branch2 for ER code review
 *
 *
 * ------------------------------------------------------------------
 * Revision 1.1.2.1  2014/07/21 01:56:36  palin2
 * Initial check-in Skye module side Diag code.
 *
 * $Endlog$
 */
