/* $Id: diag_mother_board_test.h,v 1.2 2021/09/24 01:21:06 harrchan Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/elixir/diag_mother_board_test.h,v $
 *------------------------------------------------------------------
 * 
 * diag_mother_board_test.h
 *
 * Copyright (c) 2018 - 2019 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

extern menuinfo_t mb_subtest_menu;
extern menuinfo_t *mb_submenup;
extern int diag_mother_board_test(int);
extern int linux_memory_tester(int);
extern int do_all_menu_items(struct menuinfo *);

/*-------------------------------------------------
 * $Log: diag_mother_board_test.h,v $
 * Revision 1.2  2021/09/24 01:21:06  harrchan
 * Collapse Elixir-branch to Main Trunk.
 *
 * Revision 1.1.2.1  2020/09/09 09:09:51  illiu
 * First version which has been ported with Dreamliner and Marvell CPSS
 *
 * Revision 1.2  2019/01/10 06:36:27  wilbhuan
 * The beginning of Betelgeuse application code.
 *
 *-------------------------------------------------
 */
