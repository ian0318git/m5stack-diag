/* $Id: diag_bootflash_test.h,v 1.2 2013/10/08 08:48:27 tirawan Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/woodlawn_linux/diag_bootflash_test.h,v $
 *------------------------------------------------------------------
 * diag_bootflash_test.h 
 * 
 * February 2012, Leslie Chen
 * Copyright (c) 2013 by Cisco Systems, Inc.
 * All rights reserved.
 *------------------------------------------------------------------
 */

#ifndef __DIAG_BOOTFLASH_TEST_H__
#define __DIAG_BOOTFLASH_TEST_H__

extern int bootflash_test(void);
extern int get_bootflash_info(void);
extern int bootflash_otp_test(void);

#endif

/*-------------------------------------------------
 * $Log: diag_bootflash_test.h,v $
 * Revision 1.2  2013/10/08 08:48:27  tirawan
 * Woodlawn collapsed to main trunk
 *
 * Revision 1.1.4.2  2013/08/20 10:58:50  tirawan
 * Branch into woodlawn-branch2 and port woodlawn code
 *
 * Revision 1.1.2.2  2013/07/02 07:17:07  leschen
 * Add extern declaration for OTP test
 *
 * Revision 1.1.2.1  2013/04/24 10:37:14  tirawan
 * Initial check-in for woodlawn linux code
 *
 * Revision 1.1  2013/03/13 06:42:49  kuangik
 * Add for the first time
 *
 * Revision 1.3  2012/08/03 10:16:55  evanli
 * Mapping to latest O2 source code on 20120726
 *
 * Revision 1.1  2012/02/10 06:26:18  leslie
 * Add Woodlwan bootflash test header file.
 *
 * $Endlog$
 *-------------------------------------------------
 */
