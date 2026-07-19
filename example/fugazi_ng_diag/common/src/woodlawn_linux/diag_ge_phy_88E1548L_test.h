/* $Id: diag_ge_phy_88E1548L_test.h,v 1.2 2013/10/08 08:48:28 tirawan Exp $ 
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/woodlawn_linux/diag_ge_phy_88E1548L_test.h,v $
 *-----------------------------------------------------------------------------
 * diag_ge_phy_88E1548L_test.h
 *
 * February 2012, Leslie Chen
 * Copyright (c) 2013 by Cisco Systems, Inc.
 * All rights reserved.
 *-----------------------------------------------------------------------------
 */
 
#ifndef __DIAG_GE_PHY_88E1548L_TEST_H__
#define __DIAG_GE_PHY_88E1548L_TEST_H__

#define DIAG_PHY_LED_SPEED              (0x01)
#define DIAG_PHY_LED_LINK               (0x02)

extern int diag_ge_led_toggle(int, int, int);
extern int ge_phy_88E1548L_test(int);
extern int ge_88E1548_do_all_wrapper(void);

#endif
/*-------------------------------------------------
 * $Log: diag_ge_phy_88E1548L_test.h,v $
 * Revision 1.2  2013/10/08 08:48:28  tirawan
 * Woodlawn collapsed to main trunk
 *
 * Revision 1.1.4.2  2013/08/20 10:58:52  tirawan
 * Branch into woodlawn-branch2 and port woodlawn code
 *
 * Revision 1.1.2.2  2013/06/13 11:42:44  tirawan
 * Implement LED nc dispatch command for host side to be able to control SM LED
 *
 * Revision 1.1.2.1  2013/04/24 10:37:17  tirawan
 * Initial check-in for woodlawn linux code
 *
 * Revision 1.2  2013/04/10 09:48:48  kuangik
 * Implement do all wrapper for host run sm test
 *
 * Revision 1.5  2013/01/16 01:19:30  leslie
 * Add extern function declaration.
 *
 * Revision 1.3  2012/08/03 10:16:55  evanli
 * Mapping to latest O2 source code on 20120726
 *
 * Revision 1.1  2012/02/10 07:00:39  leslie
 * Add Woodlawn phy 88E1548L test header file.
 * 
 *
 * $Endlog$
 *-------------------------------------------------
 */
