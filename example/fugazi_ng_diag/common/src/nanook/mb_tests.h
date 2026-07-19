 /* $Id: mb_tests.h,v 1.2 2019/12/11 10:10:32 lucywang Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/nanook/mb_tests.h,v $
 *------------------------------------------------------------------
 *
 * mb_tests.h
 *
 * May 2016, Sofian Teja adapted from Xformers
 *
 * Copyright (c) 2019 by cisco Systems, Inc.
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
extern int build_ps_menu(boolean);
extern int build_fpga_test_menu(boolean);
extern int build_esw_test_menu(boolean);
extern int build_i2c_scan_menu(boolean);
extern int linux_memory_tester(int);

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
 * $Log: mb_tests.h,v $
 * Revision 1.2  2019/12/11 10:10:32  lucywang
 * Merged Nanook to main trunk
 *
 *
 * $Endlog$
 *-------------------------------------------------
 */
