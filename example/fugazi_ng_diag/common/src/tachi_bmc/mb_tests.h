/* $Id: mb_tests.h,v 1.3 2017/03/30 08:30:54 hondwang Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/tachi_bmc/mb_tests.h,v $
 *------------------------------------------------------------------
 *
 * mb_tests.h
 *
 * Jan 2015, Hsuan-Ming Yang adapted from Overload.
 *
 * Copyright (c) 2017 by cisco Systems, Inc.
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
extern int ethernet_tests(int);
extern int build_ge_phy_menu(int);
extern int usb_test(void);
extern uchar sku_id[32];
extern void display_env(void);

#endif /* MB_TESTS_H__ */

/******** History ******** 
$Log: mb_tests.h,v $
Revision 1.3  2017/03/30 08:30:54  hondwang
Tachi-L brach merge

Revision 1.2.14.1  2016/11/04 19:08:54  benchen2
Modify Enhanced error message

Revision 1.2  2016/04/20 11:25:28  benchen2
add tachi fru portion

Revision 1.1.2.1  2015/06/11 02:01:10  tirawan
Add files for Tachi BMC project

Revision 1.2  2012/03/28 00:38:21  mcharon
remove forward slash from second line

Revision 1.1.1.1  2012/03/23 23:02:08  ptong
Initial archive of ng_diag module


$Endlog$
*/
