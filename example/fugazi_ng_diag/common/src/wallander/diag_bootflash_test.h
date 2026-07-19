/* $Id: diag_bootflash_test.h,v 1.1 2015/02/26 07:18:29 xiaoyizh Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/wallander/diag_bootflash_test.h,v $
 *-----------------------------------------------------------------------------
 * diag_bootflash_test.h 
 *
 * Xiaoying Zhang -- Feb. 2014
 *
 * Copyright (c) 2015 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#ifndef __DIAG_BOOTFLASH_TEST_H__
#define __DIAG_BOOTFLASH_TEST_H__

extern int bootflash_test(void);
extern int get_bootflash_info(void);
extern int get_bootflash_info_byte(void);
extern int get_bootflash_info_word(void);
extern int bootflash_golden_lock_test(void);
extern int bootflash_lock_golden(void);
extern int bootflash_all_ppb_earse(void);

#endif

/*-------------------------------------------------
 * $Log: diag_bootflash_test.h,v $
 * Revision 1.1  2015/02/26 07:18:29  xiaoyizh
 * Initial check in for Wallander.
 *
 *
 * $Endlog$
 *-------------------------------------------------
 */
