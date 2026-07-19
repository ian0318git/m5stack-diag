/* $Id: diag_mother_board_test.h,v 1.2 2019/01/10 06:36:27 wilbhuan Exp $ 
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/betelgeuse/diag_mother_board_test.h,v $
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
 * Revision 1.2  2019/01/10 06:36:27  wilbhuan
 * The beginning of Betelgeuse application code.
 *
 *-------------------------------------------------
 */
