/* $Id: plat_defs.h,v 1.8 2020/07/10 11:36:50 steja Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/tsn/plat_defs.h,v $
 *------------------------------------------------------------------
 *
 * plat_defs.h - TSN platform defines.
 *
 * March, 2016, Sofian Teja
 *
 * Copyright (c) 2016 ~ 2018 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */


#ifndef _PLAT_DEFS_H_
#define _PLAT_DEFS_H_

#include "ethernet.h"
#include "types.h"

/*
 * Common
 */
#define LINUX_KER_V4_4_52_STRING "4.4.52"
#define LINUX_KER_V4_4_8_STRING  "4.4.8"
#define LINUX_KERNEL_V4_4_52     4452
#define LINUX_KERNEL_V4_4_8      448

/*
 * Main menu test flag defines
 */
#define MM_1    (MF_CONTINUOUS | MF_DOGRP)
#define MM_2    (MM_1 | MF_DOALL)
#define MM_3    (MM_2 | MF_SHOW_ERRCOUNT)
#define ENHANCE_ERROR_MSG_RDY 1

/* Common definitions */
#define TSN_OCT17_PILOT_ROMMON_IMGNAME "/C1100-rel-dedi-prog-166-1r-SPA.bin"
#define TSN_OCT17_PILOT_ROMMON_BANNER  "16.6(1r)"
#define TSN_BF_BUSNUM   0

#define USB_SLOT0       0
#define USB_SLOT1       1
#define USB_SLOT2       2
int tsn_show_cpuinfo (void);

#define TSN_GE_PHY_SMI_BUS   (3)
#define TSN_GE_PHY_ID        (0x18)

/* TSN board SKU definitions */
#define TSN_H_MB   0
#define TSN_M_MB   1

/* GPIO  definition */
#define CP_MPP8_GPIO8         0x00000100

/* FPGA  definition */
#define FPGA_CPU_INT0_L       CP_MPP8_GPIO8

#define TSN_FPGA_REG_WIDTH    4

/* Ethernet definition */
#define TSN_GE0               0
#define TSN_GE1               1

#define TSN_GE0_ETHNUM        ETH0
#define TSN_GE1_ETHNUM        ETH2
#define TSN_ESW_ETHNUM        ETH1


#define TSN_GE0_CPUMAC_NUM    (int)0
#define TSN_GE1_CPUMAC_NUM    (int)3

#define TSN_ESW_CPU_MACNUM    2

#define TSN_GE0_SMIADDR       0x1D
#define TSN_H_ESW_SMIADDR     0x0F
#define TSN_M_ESW_SMIADDR     0x1E
#define TSN_GE1_SMIADDR       0x1C

#define TSN_H_ESW_CPU_PORT    9
#define TSN_H_ESW_GEPORTS     8

#define TSN_M_ESW_CPU_PORT    5
#define TSN_M_ESW_GEPORTS     4


/* PoE PSU controller(TI, TPS2386B) */
#define TSN_PSU_REG_WIDTH     1
#define TSN_POE_PORTS         4
#define TSN_H_POE_PORTS       4
#define TSN_M_POE_PORTS       2

/* define sleep seconds */
#define wait1sec              1 
#define wait5sec              5 
#define wait8sec              8
#define wait20sec             20 
#define wait25sec             25
#define wait100sec            100 

/* define sleep seconds */
#define LENGTH100             100
#define LENGTH1000            1000
#define LENGTH1024            1024

#define LENGTH32               32
#define LENGTH64               64 
#define COUNT30                30

#define SFP_EEPROM_16_LENGTH   (16+1)
#define SFP_EEPROM_8_LENGTH    (8+1)

#define SFP_BUFFER_256         256

#define EMMC_TEST_BUFFER_SIZE	512
#define EMMC_TEST_PATTERN_SIZE	128
#define EMMC_BLK	"/dev/mmcblk0"

/* Extern */
extern int quiet_launch;
extern int usb_dump_x(int);
extern int usb_debugport_test(int);
extern int get_i2c_fd(int);
extern int usb_utils(int);
extern boolean tsn_has_poe(int);
extern int diag_extend_feature(boolean);
extern int tsn_display_temp(void);
extern int read_sfp_cookie(int);
extern uint diag_kernel_ver;
extern int get_kernel_ver(void); 

#endif                          /* _PLAT_DEFS_H_ */
/*************************************************************
$Log: plat_defs.h,v $
Revision 1.8  2020/07/10 11:36:50  steja
Enhanced TSN LTE Series
1.CSCvu76591 [TSN-H/TSN-GFAST] Modify SIM_DETECT pin Test item
2.CSCvu72092: [TSN-H/TSN-GFAST] Enhance the LTE USB port number change dynamically
3.CSCvu72089: [TSN-H/TSN-GFAST] Adjust LTE Power Sequence

Revision 1.7.56.1  2020/06/19 08:24:57  steja
1. Adjust LTE Power Sequence based SWI Guideline
2. Dynamically ttyUSB port for AT command
3. Remove LTE init on SIM Detection Test

Revision 1.7  2018/11/23 08:49:51  hondwang
Re-instruct pluggable common code with CDETs CSCvn17216

Revision 1.6.32.1  2018/10/15 06:53:07  hondwang
pluggable common code re-instruct modify code

Revision 1.6  2018/06/05 09:54:08  lucywang
Merge Star branch star-branch-c110x to main trunk

Revision 1.5  2018/04/15 22:03:30  palin2
Merged Vulcan back to maintrunk.

Revision 1.4  2018/02/09 09:56:54  hondwang
Merge Star branch star-branch-c9xx to main trunk

Revision 1.3.10.2  2018/02/07 10:23:01  lucywang
Followed coding rule

Revision 1.3.10.1  2018/01/20 06:27:23  hondwang
prepare merge star-branch-c9xx to main trunk

Revision 1.3  2017/10/19 14:04:28  palin2
Added support to upgrade ROMMON to TSN Oct-2017 Pilot version, 16.6(1r).

Revision 1.2.4.1  2017/09/09 00:47:48  hondwang
Add C949-4P support with MB,Wifi,LTE EM

Revision 1.2  2017/08/02 14:21:47  steja
Support TSN-H/M platform code

Revision 1.1.8.2  2017/07/29 03:41:19  steja
tsn-branch5 synced with Maintrunk repositories

Revision 1.1.6.4  2017/07/24 14:14:10  palin2
1. To improve code readability.
2. All changes are verified before check-in.

Revision 1.1.6.3  2017/07/21 10:46:03  steja
Update based on code review comment

Revision 1.1.6.2  2017/07/20 13:38:06  steja
tsn-branch4 merge with maintrunk

Revision 1.1.4.8.2.9  2017/07/20 11:29:02  steja
Code cleanup

Revision 1.1.4.8.2.8  2017/07/18 06:10:36  steja
Code cleanup

Revision 1.1.4.8.2.7  2017/07/17 14:41:00  steja
code cleanup

Revision 1.1.4.8.2.6  2017/07/17 13:54:44  palin2
Code cleanup.

Revision 1.1.4.8.2.5  2017/07/11 10:13:16  steja
1. Remove Debugcard test
2. Add LTE micro usb utility to basic utilities
3. Code clean up

Revision 1.1.4.8.2.4  2017/07/08 07:27:26  steja
Code Clean up

Revision 1.1.4.8.2.3  2017/06/12 11:23:20  steja
Enhanced LTE mini-usb test

Revision 1.1.4.8.2.2  2017/05/17 02:19:41  palin2
Updated function of triggering PoE PSE side interrupt.

Revision 1.1.4.8.2.1  2017/05/17 01:17:53  palin2
Updated GE WAN mapping number with team's decision.
(GE0: GE WAN with SFP; GE1: 2nd GE WAN)

Revision 1.1.4.8  2016/11/29 02:54:39  palin2
Dynamically getting device bus window base from CPU register.

Revision 1.1.4.7  2016/09/28 04:36:15  palin2
Added CPU to ESW PHY MAC loopback test.

Revision 1.1.4.6  2016/09/13 08:14:23  palin2
Added CPU to GE PHY MAC loopback test.

Revision 1.1.4.5  2016/07/13 11:09:39  iachang
Provided Aikido register r/w utilities.

Revision 1.1.4.4  2016/07/05 14:26:51  palin2
Added utililty to force ON/OFF TSN Switch port LED(s).

Revision 1.1.4.3  2016/07/04 15:29:28  palin2
1. Updated TSN-M Switch part related code after bring up.
2. Added utility to change LAN PHY port VOD setup for HW.

Revision 1.1.4.2  2016/06/30 06:22:49  steja
tsn-branch2 sync with main trunk

Revision 1.1.2.10  2016/05/26 11:53:03  palin2
Added utilities to turn TSN all Green/Yellow LEDs ON.

Revision 1.1.2.9  2016/05/26 03:09:22  palin2
Added TSN Switch init function, and SMI C45 read/write utility.

Revision 1.1.2.8  2016/05/24 01:20:11  palin2
Updated GE Switch and PHY utilities.

Revision 1.1.2.7  2016/05/10 06:17:33  palin2
Updated PoE PSE related diag code after bring up.

Revision 1.1.2.6  2016/04/29 10:24:13  palin2
Updated Switch SMI address(0x1F to 0xF) based on HW change.
This is to change Switch to Multi Chip Address mode.

Revision 1.1.2.5  2016/04/22 12:28:36  palin2
Updated code after bring up GE PHY external loopback test.

Revision 1.1.2.4  2016/04/14 06:09:49  palin2
1. Removed cpld.c and cpld.h because TSN don't have CPLD.
2. Linked related function to correct FPGA one.

Revision 1.1.2.3  2016/03/29 02:50:03  palin2
Added GE PHY Diag.

Revision 1.1.2.2  2016/03/27 14:17:34  steja
update based on code review comment 3/25/2016

Revision 1.1.2.1  2016/03/08 09:55:11  steja
Initial Check-in


$Endlog$
*/
