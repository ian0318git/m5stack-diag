/* $Id: mb_tests.h,v 1.2 2013/10/08 08:48:30 tirawan Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/woodlawn_linux/mb_tests.h,v $
 *--------------------------------------------------------------------
 * mb_tests.h
 *
 * March 2011, Paul Tong *
 * Copyright (c) 2013 by Cisco Systems, Inc.
 * All rights reserved.
 *--------------------------------------------------------------------
 */
#ifndef __MB_TESTS_H__
#define __MB_TESTS_H__

/*
 * Used in sub menu for mb tests.
 */
typedef struct mb_tests_submenu_t_ {
  char  *name;             /* sub menu test name          */ 
  PFT    diag;             /* sub menu test to be called  */
  type_t    parm;             /* parameter to be passed      */
  type_t   flag;             /* flags that can be used      */
} mb_tests_submenu_t;

typedef struct title_buf_t_ {
   char title[80];
} title_buf_t;

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
extern int build_ge_phy_menu(int);
extern boolean is_pvdm_slot_valid (int);
extern boolean is_sm_slot_valid (int);
extern boolean is_hwic_slot_valid (int);
extern boolean is_ism_slot_valid (int);
extern boolean is_cf_slot_valid (int);
extern boolean is_ge_switch_valid (int);
extern boolean is_reset_butt_valid (int);
extern boolean is_eusb_flash_valid (int);
extern type_t ism_iface_test(void);

#endif /* MB_TESTS_H__ */

/*-------------------------------------------------
 * $Log: mb_tests.h,v $
 * Revision 1.2  2013/10/08 08:48:30  tirawan
 * Woodlawn collapsed to main trunk
 *
 * Revision 1.1.4.2  2013/08/20 10:59:10  tirawan
 * Branch into woodlawn-branch2 and port woodlawn code
 *
 * Revision 1.1.2.1  2013/04/24 10:37:23  tirawan
 * Initial check-in for woodlawn linux code
 *
 * Revision 1.1  2013/03/13 06:43:00  kuangik
 * Add for the first time
 *
 * Revision 1.3  2012/08/03 10:16:55  evanli
 * Mapping to latest O2 source code on 20120726
 *
 * Revision 1.1.1.1  2012/02/10 05:59:50  kody
 * Initial imports Woodlawn project code base.
 *
 * Revision 1.1.2.1  2011/04/05 19:59:37  ptong
 * Initial checkin
 *
 * $Endlog$
 *-------------------------------------------------
 */
