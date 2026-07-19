/* $Id: mb_tests.h,v 1.4 2019/07/11 12:31:31 alicehua Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/nutella/mb_tests.h,v $
 *------------------------------------------------------------------
 *
 * mb_tests.h
 *
 * May 2016, Sofian Teja adapted from Xformers
 *
 * Copyright (c) 2008-2019 by cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#ifndef _MB_TESTS_H_
#define _MB_TESTS_H_

/*
 * Global extern functions
 */
extern int do_all_menu_items(struct menuinfo *);
extern int build_boot_flash_menu(boolean);
extern int build_emmc_test_menu(boolean);
extern int build_snsr_menu(boolean);
extern int build_fpga_test_menu(boolean);
extern int build_esw_test_menu(boolean);
extern int linux_memory_tester_with_ecc_check(int);

extern menuinfo_t mb_subtest_menu;
extern menuinfo_t *mb_submenup;
extern int mb_tests(int);
extern int do_all_menu_items(struct menuinfo *);
/*
 * Used in sub menu for mb tests.
 */
/* For USB test */
#define USB2                       2 
#define USB3                       3

#endif                          /* MB_TESTS_H__ */

/*-------------------------------------------------
$Log: mb_tests.h,v $
Revision 1.4  2019/07/11 12:31:31  alicehua
Collapse Nutella codes into main trunk.

$Endlog$
*/
