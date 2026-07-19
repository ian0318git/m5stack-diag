/* $Id: linux_main.h,v 1.2 2021/09/24 01:21:07 harrchan Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/elixir/linux_main.h,v $
 *------------------------------------------------------------------
 * 
 * linux_main.h
 *
 * Copyright (c) 2018 - 2019 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#define LINUX_KER_V4_4_52_STRING "4.4.52"
#define LINUX_KER_V4_4_8_STRING  "4.4.8"
#define LINUX_KERNEL_V4_4_52     4452
#define LINUX_KERNEL_V4_4_8      448
#define M7040_GENRATION_2_SET_REG(n)	(0xF21208F8 + (0x1000 * n))
#define G2_TX_SSC_AMP_OFFSET			9
#define G2_TX_SSC_AMP_MASK				0x00007E00

extern int ExecuteCmdbyPopen(char *, char *, int);
extern int quiet_launch;
extern int get_i2c_fd(int);
extern int plat_show_cpuinfo (void);

/*-------------------------------------------------
 * $Log: linux_main.h,v $
 * Revision 1.2  2021/09/24 01:21:07  harrchan
 * Collapse Elixir-branch to Main Trunk.
 *
 * Revision 1.1.2.1  2020/09/09 09:09:53  illiu
 * First version which has been ported with Dreamliner and Marvell CPSS
 *
 * Revision 1.2  2019/01/10 06:36:28  wilbhuan
 * The beginning of Betelgeuse application code.
 *
 *-------------------------------------------------
 */
