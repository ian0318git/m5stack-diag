/* $Id: mb_tests.h,v 1.1 2013/05/09 05:52:59 alpeng Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/overlord/utah/mb_tests.h,v $
 *------------------------------------------------------------------
 *
 * mb_tests.h
 *
 * May 2008, Shih-Nan Huang adapted from Xformers
 *
 * Copyright (c) 2013 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#ifndef _MB_TESTS_H_
#define _MB_TESTS_H_

/*
 * Used in sub menu for mb tests.
 */


/*-----------------------------------------------------------------------
 *  Externs                                                             *
 *----------------------------------------------------------------------*/
extern menuinfo_t     mb_subtest_menu;
extern menuinfo_t    *mb_submenup;
extern title_buf_t    mb_subtest_header;
extern title_buf_t    mb_subtest_title[];

extern int map_mainmem_test(int);
extern int test_memory_ecc();
extern int mvl_ge_switch_main(int);
extern int ethernet_tests(int);
extern int build_ge_phy_menu(int);
extern int usb_test(void);

#endif /* MB_TESTS_H__ */

/******** History ******** 
$Log: mb_tests.h,v $
Revision 1.1  2013/05/09 05:52:59  alpeng
add utah tree

Revision 1.2  2012/03/28 00:38:21  mcharon
remove forward slash from second line

Revision 1.1.1.1  2012/03/23 23:02:08  ptong
Initial archive of ng_diag module


$Endlog$
*/
