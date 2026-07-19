 /* $Id: mb_tests.h,v 1.2 2019/10/17 02:16:25 kehuang2 Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/tabei-l/mb_tests.h,v $
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
extern int build_tabeil_fpga_test_menu(boolean);
extern int build_esw_test_menu(boolean);
extern int build_i2c_scan_menu(boolean);
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
 * $Log: mb_tests.h,v $
 * Revision 1.2  2019/10/17 02:16:25  kehuang2
 * Collapse Tabei-L into main trunk
 *
 * Revision 1.1.4.6  2019/06/05 07:44:36  meho
 * code clean up
 *
 * Revision 1.1.4.5  2019/03/26 06:09:16  olin2
 * Support Dreamliner on Tabei-L
 *
 * Revision 1.1.4.4  2019/03/22 08:20:24  meho
 * Added pcie speed/width detection in NVMe test.
 *
 * Revision 1.1.4.3  2018/10/24 10:45:17  harrchan
 * Seperate DIMM test from other I2C device
 *
 * Revision 1.1.4.2  2018/10/02 01:50:02  harrchan
 * Initial commit for Tabei-L P1A bring up.
 *
 * $Endlog$
 *-------------------------------------------------
 */
