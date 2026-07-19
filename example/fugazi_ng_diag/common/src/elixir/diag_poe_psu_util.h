/* $Id: diag_poe_psu_util.h,v 1.2 2021/09/24 01:21:06 harrchan Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/elixir/diag_poe_psu_util.h,v $
 *------------------------------------------------------------------
 * 
 * diag_poe_psu_util.h
 *
 * Copyright (c) 2018 - 2019 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

extern int diag_poe_psu_util(int);

#define UTIL_DET_PORT_PWR   0
#define UTIL_REG_READ       1
#define UTIL_REG_WRITE      2
#define UTIL_REG_DUMP       3
#define UTIL_SHOW_PORT_STAT 4
#define CHK_VALID_UTIL(UTIL_ITEM) ((UTIL_ITEM == UTIL_DET_PORT_PWR) || \
                                   (UTIL_ITEM == UTIL_REG_READ) || \
                                   (UTIL_ITEM == UTIL_REG_WRITE) || \
                                   (UTIL_ITEM == UTIL_REG_DUMP) || \
                                   (UTIL_ITEM == UTIL_SHOW_PORT_STAT))

/*-------------------------------------------------
 * $Log: diag_poe_psu_util.h,v $
 * Revision 1.2  2021/09/24 01:21:06  harrchan
 * Collapse Elixir-branch to Main Trunk.
 *
 * Revision 1.1.2.1  2020/09/09 09:09:52  illiu
 * First version which has been ported with Dreamliner and Marvell CPSS
 *
 * Revision 1.2  2019/01/10 06:36:27  wilbhuan
 * The beginning of Betelgeuse application code.
 *
 *-------------------------------------------------
 */
