/* $Id: diag_ge_phy_test.h,v 1.2 2021/09/24 01:21:06 harrchan Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/elixir/diag_ge_phy_test.h,v $
 *------------------------------------------------------------------
 * 
 * diag_ge_phy_test.h
 *
 * Copyright (c) 2018 - 2019 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#define ENHANCE_ERROR_MSG_RDY 1

extern int diag_88e1112_ge0_test(int);
extern int diag_88e1112_ge1_test(int);
extern int current_sfp_select;
/*-------------------------------------------------
 * $Log: diag_ge_phy_test.h,v $
 * Revision 1.2  2021/09/24 01:21:06  harrchan
 * Collapse Elixir-branch to Main Trunk.
 *
 * Revision 1.1.2.2  2020/09/16 02:25:35  harrchan
 * Support GE1 SFP test
 *
 * Revision 1.1.2.1  2020/09/09 09:09:51  illiu
 * First version which has been ported with Dreamliner and Marvell CPSS
 *
 * Revision 1.2  2019/01/10 06:36:26  wilbhuan
 * The beginning of Betelgeuse application code.
 *
 *-------------------------------------------------
 */
