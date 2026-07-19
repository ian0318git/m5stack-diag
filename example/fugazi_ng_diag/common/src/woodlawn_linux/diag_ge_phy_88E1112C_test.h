/* $Id: diag_ge_phy_88E1112C_test.h,v 1.2 2013/10/08 08:48:28 tirawan Exp $ 
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/woodlawn_linux/diag_ge_phy_88E1112C_test.h,v $
 *-----------------------------------------------------------------------------
 * diag_ge_phy_88E1112C_test.h
 *
 * January 2013, Leslie Chen
 * Copyright (c) 2013 by Cisco Systems, Inc.
 * All rights reserved.
 *-----------------------------------------------------------------------------
 */

#ifndef __DIAG_GE_PHY_88E1112C_TEST_H__
#define __DIAG_GE_PHY_88E1112C_TEST_H__ 

extern int ge_phy_88E1112C_test(int);
extern int ge_phy_88E1112C_loopback_test(int);
extern void dev_88e1112c_create(dev_mrvl_ge_object_t *, smi_if_t *);
extern int ge_88E1112C_do_all_wrapper(void);

#endif
/*-------------------------------------------------
 * $Log: diag_ge_phy_88E1112C_test.h,v $
 * Revision 1.2  2013/10/08 08:48:28  tirawan
 * Woodlawn collapsed to main trunk
 *
 * Revision 1.1.4.2  2013/08/20 10:58:51  tirawan
 * Branch into woodlawn-branch2 and port woodlawn code
 *
 * Revision 1.1.2.1  2013/04/24 10:37:16  tirawan
 * Initial check-in for woodlawn linux code
 *
 * Revision 1.3  2013/04/10 09:48:48  kuangik
 * Implement do all wrapper for host run sm test
 *
 * Revision 1.2  2013/02/19 00:48:16  leslie
 * Declare extern function
 *
 * Revision 1.1  2013/01/16 02:34:17  leslie
 * Add Woodlawn PHY 88E1112C test header file.
 *
 * $Endlog$
 *-------------------------------------------------
 */
