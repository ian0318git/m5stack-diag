/* $Id: diag_ge_phy_88E1340_lib.h,v 1.2 2013/10/08 08:48:28 tirawan Exp $ 
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/woodlawn_linux/diag_ge_phy_88E1340_lib.h,v $
 *-----------------------------------------------------------------------------
 * diag_ge_phy_88E1340_lib.h
 *
 * February 2012, Leslie Chen
 * Copyright (c) 2013 by Cisco Systems, Inc.
 * All rights reserved.
 *-----------------------------------------------------------------------------
 */

#ifndef __DIAG_GE_PHY_88E1340_LIB_H__
#define __DIAG_GE_PHY_88E1340_LIB_H__

typedef enum mrvl_88e1340_phy_t_ {
    MRVL_1340_PHY0,
    MRVL_1340_PHY1,
} mrvl_88e1340_phy_t;

#define MRVL_88E1340_PHY0_SMI_ADDR          (0xB)
#define MRVL_88E1340_PHY1_SMI_ADDR          (0xF)

extern int diag_88e1340_smi_phy_wr(uint, int, int);
extern int diag_88e1340_smi_phy_rd(uint, int, int *);
extern int diag_88e1340_init(void);

#endif
/*-------------------------------------------------
 * $Log: diag_ge_phy_88E1340_lib.h,v $
 * Revision 1.2  2013/10/08 08:48:28  tirawan
 * Woodlawn collapsed to main trunk
 *
 * Revision 1.1.4.2  2013/08/20 10:58:52  tirawan
 * Branch into woodlawn-branch2 and port woodlawn code
 *
 * Revision 1.1.2.1  2013/04/24 10:37:16  tirawan
 * Initial check-in for woodlawn linux code
 *
 * Revision 1.3  2013/03/27 04:49:35  kuangik
 * Code cleanup after adding -Wall
 *
 * Revision 1.6  2012/08/27 06:43:21  evanli
 * Modify PHY Addr
 *
 * Revision 1.5  2012/08/03 10:16:55  evanli
 * Mapping to latest O2 source code on 20120726
 *
 * Revision 1.3  2012/06/12 02:30:55  leslie
 * Modify PHY0 and PHY1 address
 *
 * Revision 1.2  2012/04/06 06:06:30  kuangik
 * Update for 88E1340 Test item
 *
 * Revision 1.1  2012/02/10 06:53:17  leslie
 * Add Woodlawn phy 88E1340 lib header file.
 *
 *
 * $Endlog$
 *-------------------------------------------------
 */
