/* $Id: platform_eth.c,v 1.4 2018/12/21 00:56:34 haohsu Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/vg400/platform_eth.c,v $
 *------------------------------------------------------------------
 * Platform specific code for Linux base ethernet port loopback test
 * 
 * Sept 2010 ptong
 * Copyright (c) 2011-2018 by Cisco Systems, Inc.
 * All rights reserved.
 *------------------------------------------------------------------
 */
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <linux/types.h>
#include <linux/socket.h>
#include <sys/ioctl.h>
#include <linux/if_ether.h>
#include <linux/ip.h>
#include <linux/udp.h>
#include <net/if.h>
#include <netinet/in.h>
#include <linux/sockios.h>
#include <linux/mii.h>

#include "defs.h"
#include "types.h"
#include "proto.h"
#include "common.h"
#include "common_utils.h"
#include "monitor.h"
#include "menu.h"
#include "nvmonvars.h"
#include "error.h"
#include "router_if.h"
#include "sgmii_defs.h"
#include "ethernet.h"
#include "linux_eth.h"
#include "platform_eth.h"
#include "platform_ext_lpbk.h"
#include "ethernet.h"  /* for SFPx port definition */
#include "dash_fpga.h" /* for get SFP ctrl reg */  
#include "queryflags.h" /* for query user functions */  

static int build_eth_util_menu(void);
static void vg400_phy_reg_access(void);
static int vg400_phy_select(int);
static void dump_phy_reg(int, const mrvl_phy_regs_t *);
static void sgmii_phy_reg_dump(void); 
static int phy_1548_reg_test(void);
int phy_1548_interrupt_test(void);
int utah_phy_reg_rd(uint , ushort , ushort *);
int utah_phy_reg_wr(uint , ushort , ushort ); 
int vg400_phy_reg_rd(uint , ushort , ushort *); 
int vg400_phy_reg_wr(uint , ushort , ushort ); 
extern int phy_1548_register_tests(ulong , reg_info_t *);


static const reg_info_t marvell_88e1340_reg_page0[] = {   /* Page 0*/
    {"Copper Control",      0x00, READ_WRITE, {2}, 0x3940, 0x1940},
    {"Copper Status",       0x01, READ_ONLY,  {2}, 0x0000, 0x7949},
    {"PHY ID1",             0x02, READ_ONLY,  {2}, 0x0000, 0x0141},
    {"PHY ID2",             0x03, READ_ONLY,  {2}, 0x0000, 0x0dc0},
    {"Copper Auto-Neg",     0x04, READ_WRITE, {2}, 0xA21F, 0x01e1},
    {"Copper Link-P Abil",  0x05, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"Copper Auto-Neg Exp", 0x06, READ_ONLY,  {2}, 0x0000, 0x0004},
    {"Copper Next Page",    0x07, READ_WRITE, {2}, 0xB7FF, 0x2001},
    {"Copper Link Partner", 0x08, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"1000BT Control",      0x09, READ_WRITE, {2}, 0xF2FF, 0x0f00},
    {"1000BT Status",       0x0A, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"Extended Status",     0x0F, READ_ONLY,  {2}, 0x0000, 0x3000},
    {"Copper Spec Cntl1",   0x10, READ_WRITE, {2}, 0x7C9F, 0x3060},
    {"Copper Spec Ststus",  0x11, READ_ONLY,  {2}, 0x0000, 0xC040},
    {"Copper Spec Intr Ena",0x12, READ_WRITE, {2}, 0xFFFF, 0x0000},
    {"Copper Intr Status",  0x13, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"Copper Spec Cntl2",   0x14, READ_WRITE, {2}, 0xFFDF, 0x0020},
    {"Copper Spec Rx Err",  0x15, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"Page Register",       0x16, READ_WRITE, {2}, 0x0000, 0x0000},
    {"Global Intr Status",  0x17, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"Copper Spec Cntl3",   0x1A, READ_WRITE, {2}, 0xFEFF, 0x0040},
    {"end",                 0x00, 0, {0}, 0, 0},
};

static const reg_info_t marvell_88e1340_reg_page1[] = {  /* Page 1*/
    {"Fiber Control",       0x00, READ_WRITE, {2}, 0x1100, 0x1140},
    {"Fiber Status",        0x01, READ_ONLY,  {2}, 0x0000, 0x6149},
    {"PHY ID1",             0x02, READ_ONLY,  {2}, 0x0000, 0x0141},
    {"PHY ID2",             0x03, READ_ONLY,  {2}, 0x0000, 0x0dc0},
    {"Fiber Auto-Neg",      0x04, READ_ONLY,  {2}, 0x0000, 0x0001},
    {"Fiber Link-P Abil",   0x05, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"Fiber Auto-Neg Exp",  0x06, READ_ONLY,  {2}, 0x0000, 0x0004},
    {"Fiber Next Page",     0x07, READ_WRITE, {2}, 0xB7FF, 0x2001},
    {"Fiber Link-P Next",   0x08, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"Extended Status",     0x0F, READ_ONLY,  {2}, 0x0000, 0x3000},
    {"Fiber Spec Cntl1",    0x10, READ_WRITE, {2}, 0xFC8C, 0x8084},
    {"Fiber Spec Status",   0x11, READ_ONLY,  {2}, 0x0000, 0x8000},
    {"Fiber Intr Enable",   0x12, READ_WRITE, {2}, 0x7F80, 0x0000},
    {"Fiber Intr Status",   0x13, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"Fiber Rx Err Cnt",    0x15, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"Page Register",       0x16, READ_WRITE, {2}, 0x0000, 0x0000},
    {"PRBS Control",        0x17, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"PRBS Err Cnt LSB",    0x18, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"PRBS Err Cnt MSB",    0x19, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"Fiber Spec Cntl2",    0x1a, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"end",                 0x00, 0, {0}, 0, 0},
};

static const reg_info_t marvell_88e1340_reg_page2[] = {   /* Page 2*/
    {"MAC Spec Cntl1",      0x10, READ_WRITE, {2}, 0xDBC8, 0x4004},
    {"MAC Spec Intr Ena",   0x12, READ_WRITE, {2}, 0x008C, 0x0000},
    {"MAC Intr Status",     0x13, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"MAC RX_ER Byte",      0x14, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"MAC Spec Cntl2",      0x15, READ_WRITE, {2}, 0x4008, 0x1046},
    {"Page Register",       0x16, READ_WRITE, {2}, 0x0000, 0x0000},
    {"end",                 0x00, 0, {0}, 0, 0},
};

static const reg_info_t marvell_88e1340_reg_page3[] = {   /* Page 3*/
    {"LED Func Cntl1",      0x10, READ_WRITE, {2}, 0xFFFF, 0x1777},
    {"LED Polarity Cntl",   0x11, READ_WRITE, {2}, 0xFFFF, 0x8800},
    {"LED Timer Cntl",      0x12, READ_WRITE, {2}, 0xF70F, 0x4905},
    {"LED Func Cntl&Polar", 0x13, READ_WRITE, {2}, 0xEFFF, 0x0073},
    {"Page Register",       0x16, READ_WRITE, {2}, 0x0000, 0x0000},
    {"end",                 0x00, 0, {0}, 0, 0},
};

static const reg_info_t marvell_88e1340_reg_page4[] = {   /* Page 4*/
    {"QSGMII Control",      0x00, READ_WRITE, {2}, 0x5C00, 0x1140},
    {"QSGMII Status",       0x01, READ_ONLY,  {2}, 0x0000, 0x7949},
    {"QSGMII Auto-Neg",     0x04, READ_ONLY , {2}, 0x0000, 0x0001},
    {"QSGMII Link-P Abil",  0x05, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"QSGMII Auto-Neg Exp", 0x06, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"QSGMII Spec Cntl1",   0x10, READ_WRITE, {2}, 0xFFFF, 0x6204},
    {"QSGMII Spec Status",  0x11, READ_ONLY,  {2}, 0x0000, 0xC040},
    {"QSGMII Spec Intr Ena",0x12, READ_WRITE, {2}, 0xFFFF, 0x0000},
    {"QSGMII Intr Status",  0x13, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"QSGMII RX_ER Byte",   0x14, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"QSGMII Rx Err Cnt",   0x15, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"Page Register",       0x16, READ_WRITE, {2}, 0x0000, 0x0000},
    {"PRBS Control",        0x17, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"PRBS Err Cnt LSB",    0x18, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"PRBS Err Cnt MSB",    0x19, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"QSGMII Global Cntr1", 0x1A, READ_WRITE, {2}, 0x7A04, 0xC000},
    {"QSGMII Global Cntr2", 0x1B, READ_WRITE, {2}, 0x7E03, 0x3E00},
    {"end",                 0x00, 0, {0}, 0, 0},
};

static const reg_info_t marvell_88e1340_reg_page5[] = {   /* Page 5*/
    {"Adv VCT TX MDI0",     0x10, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"Adv VCT TX MDI1",     0x11, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"Adv VCT TX MDI2",     0x12, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"Adv VCT TX MDI3",     0x13, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"1000BT Pair Skew",    0x14, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"1000BT Pair Swap",    0x15, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"Page Register",       0x16, READ_WRITE, {2}, 0x0000, 0x0000},
    {"Adv VCT Control",     0x17, READ_WRITE, {2}, 0x3FFF, 0x0000},
    {"Adv VCT Cross Pair",  0x18, READ_WRITE, {2}, 0x01FF, 0x0000},
    {"Adv VCT Same Pair 01",0x19, READ_WRITE, {2}, 0x7F7F, 0x0104},
    {"Adv VCT Same Pair 23",0x1A, READ_WRITE, {2}, 0x7F7F, 0x0F12},
    {"Adv VCT Same Pair 4", 0x1B, READ_WRITE, {2}, 0x7F7F, 0x0A0C},
    {"end",                 0x00, 0, {0}, 0, 0},
};

static const reg_info_t marvell_88e1340_reg_page6[] = {   /* Page 6*/
    {"Packet Generation",   0x10, READ_WRITE, {2}, 0xFF07, 0x0000},
    {"CRC Counters",        0x11, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"Checker Control",     0x12, READ_WRITE, {2}, 0x0007, 0x0000},
    {"General Control",     0x14, READ_WRITE, {2}, 0x0F1F, 0x0200},
    {"Page Register",       0x16, READ_WRITE, {2}, 0x0000, 0x0000},
    {"Late Colli Cnt1&2",   0x17, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"Late Colli Cnt3&4",   0x18, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"Late Colli Window",   0x19, READ_WRITE, {2}, 0x1F00, 0x0000},
    {"Misc Test",           0x1A, READ_WRITE, {2}, 0x9FA0, 0x1900},
    {"end",                 0x00, 0, {0}, 0, 0},
};

static const reg_info_t marvell_88e1340_reg_page7[] = {   /* Page 7*/
    {"PHY Cable Diag 0",    0x10, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"PHY Cable Diag 1",    0x11, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"PHY Cable Diag 2",    0x12, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"PHY Cable Diag 3",    0x13, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"PHY Cable Diag Relt", 0x14, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"PHY Cable Diag Cntl", 0x15, READ_WRITE, {2}, 0x6400, 0x4000},
    {"Page Register",       0x16, READ_WRITE, {2}, 0x0000, 0x0000},
    {"Adv VCT Cros Pair",   0x19, READ_WRITE, {2}, 0x7F7F, 0x0104},
    {"Adv VCT Same Pair 01",0x1A, READ_WRITE, {2}, 0x7F7F, 0x0F12},
    {"Adv VCT Same Pair 23",0x1B, READ_WRITE, {2}, 0x7F7F, 0x0A0C},
    {"Adv VCT Same Pair 4", 0x1C, READ_WRITE, {2}, 0x007F, 0x0006},
    {"end",                 0x00, 0, {0}, 0, 0},
};

static const reg_info_t marvell_88e1340_reg_page8[] = {   /* Page 8*/
    {"PTP Port Cntl 0",     0x00, READ_ONLY,  {2}, 0x0000, 0x1000},
    {"PTP Port Cntl 1",     0x01, READ_ONLY,  {2}, 0x0000, 0x020C},
    {"PTP Port Cntl 2",     0x02, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"PTP Port Cntl 8",     0x08, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"PTP Arr0 Byte1&0",    0x09, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"PTP Arr0 Byte3&2",    0x0A, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"PTP Arr0 Sequ ID",    0x0B, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"PTP Arr1 Byte1&0",    0x0C, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"PTP Arr1 Byte3&2",    0x0D, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"PTP Arr1 Sequ ID",    0x0E, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"Page Register",       0x16, READ_WRITE, {2}, 0x0000, 0x0000},
    {"end",                 0x00, 0, {0}, 0, 0},
};

static const reg_info_t marvell_88e1340_reg_page9[] = {   /* Page 9*/
    {"PTP Dep Status",      0x00, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"PTP Dep Byte1&0",     0x01, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"PTP Dep Byte3&2",     0x02, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"PTP Dep Sequ ID",     0x03, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"PTP Dep Cnt",         0x05, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"Page Register",       0x16, READ_WRITE, {2}, 0x0000, 0x0000},
    {"end",                 0x00, 0, {0}, 0, 0},
};

static const reg_info_t marvell_88e1340_reg_page12[] = {   /* Page 12*/
    {"TAI Global Conf 0",   0x00, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"TAI Global Conf 1",   0x01, READ_ONLY,  {2}, 0x0000, 0x1F40},
    {"TAI Global Conf 2",   0x02, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"TAI Global Conf 3",   0x03, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"TAI Global Conf 4",   0x04, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"TAI Global Conf 5",   0x05, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"TAI Global Conf 8",   0x08, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"TAI Globle Conf 9",   0x09, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"Event Cap Byte1&0",   0x0A, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"Event Cap Byte3&2",   0x0B, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"TAI Global Conf 12",  0x0C, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"TAI Global Conf 13",  0x0D, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"PTP Global Time1&0",  0x0E, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"PTP Global Time3&2",  0x0F, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"Page Register",       0x16, READ_WRITE, {2}, 0x0000, 0x0000},
    {"end",                 0x00, 0, {0}, 0, 0},
};

static const reg_info_t marvell_88e1340_reg_page14[] = {   /* Page 14*/
    {"PTP Global Conf 0",   0x00, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"PTP Global Conf 1",   0x01, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"PTP Global Conf 2",   0x02, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"PTP Global Conf 3",   0x03, READ_ONLY,  {2}, 0x0000, 0x0001},
    {"PTP Global Status",   0x08, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"Page Register",       0x16, READ_WRITE, {2}, 0x0000, 0x0000},
    {"end",                 0x00, 0, {0}, 0, 0},
};

static const mrvl_phy_regs_t marvell_88e1340_phy_reg_tbl[] = {
    {"Page  0",     0, marvell_88e1340_reg_page0},
    {"Page  1",     1, marvell_88e1340_reg_page1},
    {"Page  2",     2, marvell_88e1340_reg_page2},
    {"Page  3",     3, marvell_88e1340_reg_page3},
    {"Page  4",     4, marvell_88e1340_reg_page4},
    {"Page  5",     5, marvell_88e1340_reg_page5},
    {"Page  6",     6, marvell_88e1340_reg_page6},
    {"Page  7",     7, marvell_88e1340_reg_page7},
    {"Page  8",     8, marvell_88e1340_reg_page8},
    {"Page  9",     9, marvell_88e1340_reg_page9},
    {"Page 12",    12, marvell_88e1340_reg_page12},
    {"Page 14",    14, marvell_88e1340_reg_page14},
};

#define NUM_BRIDGE_PHY_PAGES (sizeof(marvell_88e1340_phy_reg_tbl) /      \
                             sizeof(struct mrvl_phy_regs_t_))

/***** 1548 PHY reg definiton *****/
/***** reset value and mask need to check again *****/
static  reg_info_t marvell_88e1548_reg_page0[] = {   /* Page 0*/
    {"Copper Control",      0x00, READ_ONLY,  {2}, 0x3940, 0x1940},
    {"Copper Status",       0x01, READ_ONLY,  {2}, 0x0000, 0x7949},
    {"PHY ID1",             0x02, READ_ONLY,  {2}, 0x0000, 0x0141},
    {"PHY ID2",             0x03, READ_ONLY,  {2}, 0x0000, 0x0dc0},
    {"Copper Auto-Neg Adv", 0x04, READ_ONLY,  {2}, 0xA21F, 0x01e1},
    {"Copper Link-P Abil",  0x05, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"Copper Auto-Neg Exp", 0x06, READ_ONLY,  {2}, 0x0000, 0x0004},
    {"Copper Next Page",    0x07, READ_ONLY,  {2}, 0xB7FF, 0x2001},
    {"Copper Link Partner", 0x08, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"1000BT Control",      0x09, READ_ONLY,  {2}, 0xF2FF, 0x0f00},
    {"1000BT Status",       0x0A, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"MMD ACCESS Control",  0x0D, READ_ONLY,  {2}, 0xC000, 0x0f00},
    {"MMD ACCES ADDR/DATA", 0x0E, READ_WRITE, {2}, 0x0000, 0x0000},
    {"Extended Status",     0x0F, READ_ONLY,  {2}, 0x0000, 0x3000},
    {"Copper Spec Cntl1",   0x10, READ_WRITE, {2}, 0x749B, 0x3060},
    {"Copper Spec Ststus",  0x11, READ_ONLY,  {2}, 0x0000, 0xC040},
    {"Copper Spec Intr Ena",0x12, READ_ONLY,  {2}, 0xFFFF, 0x0000},
    {"Copper Intr Status",  0x13, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"Copper Spec Cntl2",   0x14, READ_ONLY,  {2}, 0xFFDF, 0x0020},
    {"Copper Spec Rx Err",  0x15, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"Page Register",       0x16, READ_WRITE, {2}, 0x0000, 0x0000},
    {"Global Intr Status",  0x17, READ_WRITE, {2}, 0x0000, 0x0000},
    {"Copper Spec Cntl3",   0x1A, READ_ONLY , {2}, 0xFEFF, 0x0040},
    {"end",                 0x00, 0, {0}, 0, 0},
};

static const reg_info_t marvell_88e1548_reg_page1[] = {  /* Page 1*/
    {"Fiber Control",       0x00, READ_WRITE, {2}, 0x1100, 0x1140},
    {"Fiber Status",        0x01, READ_ONLY,  {2}, 0x0000, 0x6149},
    {"PHY ID1",             0x02, READ_ONLY,  {2}, 0x0000, 0x0141},
    {"PHY ID2",             0x03, READ_ONLY,  {2}, 0x0000, 0x0dc0},
    {"Fiber Auto-Neg Adv",  0x04, READ_ONLY,  {2}, 0x0000, 0x0001},
    {"Fiber Link-P Abil",   0x05, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"Fiber Auto-Neg Exp",  0x06, READ_ONLY,  {2}, 0x0000, 0x0004},
    {"Fiber Next Page",     0x07, READ_WRITE, {2}, 0xB7FF, 0x2001},
    {"Fiber Link-P Next",   0x08, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"Extended Status",     0x0F, READ_ONLY,  {2}, 0x0000, 0x3000},
    {"Fiber Spec Cntl1",    0x10, READ_WRITE, {2}, 0xFC8C, 0x8084},
    {"Fiber Spec Status",   0x11, READ_ONLY,  {2}, 0x0000, 0x8000},
    {"Fiber Intr Enable",   0x12, READ_WRITE, {2}, 0x7F80, 0x0000},
    {"Fiber Intr Status",   0x13, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"Fiber Rx Err Cnt",    0x15, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"Page Register",       0x16, READ_WRITE, {2}, 0x0000, 0x0000},
    {"PRBS Control",        0x17, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"PRBS Err Cnt LSB",    0x18, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"PRBS Err Cnt MSB",    0x19, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"Fiber Spec Cntl2",    0x1a, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"end",                 0x00, 0, {0}, 0, 0},
};

static const reg_info_t marvell_88e1548_reg_page2[] = {   /* Page 2*/
    {"MAC Spec Cntl1",      0x10, READ_WRITE, {2}, 0xDBC8, 0x4004},
    {"MAC Spec Intr Ena",   0x12, READ_WRITE, {2}, 0x008C, 0x0000},
    {"MAC Spec Status",     0x13, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"MAC Spec Cntl2",      0x15, READ_WRITE, {2}, 0x4008, 0x1046},
    {"Page Register",       0x16, READ_WRITE, {2}, 0x0000, 0x0000},
    {"end",                 0x00, 0, {0}, 0, 0},
};

static const reg_info_t marvell_88e1548_reg_page3[] = {   /* Page 3*/
    {"LED Func Cntl1",      0x10, READ_WRITE, {2}, 0xFFFF, 0x1777},
    {"LED Polarity Cntl",   0x11, READ_WRITE, {2}, 0xFFFF, 0x8800},
    {"LED Timer Cntl",      0x12, READ_WRITE, {2}, 0xF70F, 0x4905},
    {"LED Func Cntl&Polar", 0x13, READ_WRITE, {2}, 0xEFFF, 0x0073},
    {"Page Register",       0x16, READ_WRITE, {2}, 0x0000, 0x0000},
    {"LED Rand Blik Cntl 1",0x1C, READ_WRITE, {2}, 0x0000, 0x0000},
    {"LED Rand Blik Cntl 2",0x1D, READ_WRITE, {2}, 0x0000, 0x0000},
    {"end",                 0x00, 0, {0}, 0, 0},
};

static const reg_info_t marvell_88e1548_reg_page4[] = {   /* Page 4*/
    {"QSGMII Control",      0x00, READ_WRITE, {2}, 0x5C00, 0x1140},
    {"QSGMII Status",       0x01, READ_ONLY,  {2}, 0x0000, 0x7949},
    {"QSGMII Auto-Neg",     0x04, READ_ONLY , {2}, 0x0000, 0x0001},
    {"QSGMII Link-P Abil",  0x05, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"QSGMII Auto-Neg Exp", 0x06, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"QSGMII Spec Ctrl 1",  0x10, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"QSGMII Spec Stat",    0x11, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"QSGMII Spec Intr Ena",0x12, READ_WRITE, {2}, 0xFFFF, 0x0000},
    {"QSGMII Intr Status",  0x13, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"QSGMII RX_ER Byte",   0x14, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"QSGMII Rx Err Cnt",   0x15, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"Page Register",       0x16, READ_WRITE, {2}, 0x0000, 0x0000},
    {"PRBS Control",        0x17, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"PRBS Err Cnt LSB",    0x18, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"PRBS Err Cnt MSB",    0x19, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"QSGMII Global Cntr1", 0x1A, READ_WRITE, {2}, 0x7A04, 0xC000},
    {"QSGMII Global Cntr2", 0x1B, READ_WRITE, {2}, 0x7E03, 0x3E00},
    {"end",                 0x00, 0, {0}, 0, 0},
};

static const reg_info_t marvell_88e1548_reg_page5[] = {   /* Page 5*/
    {"Adv VCT TX MDI0",     0x10, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"Adv VCT TX MDI1",     0x11, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"Adv VCT TX MDI2",     0x12, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"Adv VCT TX MDI3",     0x13, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"1000BT Pair Skew",    0x14, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"1000BT Pair Swap",    0x15, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"Page Register",       0x16, READ_WRITE, {2}, 0x0000, 0x0000},
    {"Adv VCT Control",     0x17, READ_WRITE, {2}, 0x3FFF, 0x0000},
    {"Adv VCT Sample Pt.",  0x18, READ_WRITE, {2}, 0x01FF, 0x0000},
    {"Adv VCT Cross Pair",  0x19, READ_WRITE, {2}, 0x01FF, 0x0000},
    {"Adv VCT Same Pair 01",0x1A, READ_WRITE, {2}, 0x7F7F, 0x0104},
    {"Adv VCT Same Pair 23",0x1B, READ_WRITE, {2}, 0x7F7F, 0x0F12},
    {"Adv VCT Same Pair 4", 0x1C, READ_WRITE, {2}, 0x7F7F, 0x0A0C},
    {"end",                 0x00, 0, {0}, 0, 0},
};

static const reg_info_t marvell_88e1548_reg_page6[] = {   /* Page 6*/
    {"Packet Generation",   0x10, READ_WRITE, {2}, 0xFF07, 0x0000},
    {"CRC Counters",        0x11, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"Checker Control",     0x12, READ_WRITE, {2}, 0x0007, 0x0000},
    {"Packet Gen IPG Ctrl", 0x13, READ_WRITE, {2}, 0x0000, 0x0000},
    {"General Control",     0x14, READ_WRITE, {2}, 0x0F1F, 0x0200},
    {"Page Register",       0x16, READ_WRITE, {2}, 0x0000, 0x0000},
    {"Late Colli Cnt1&2",   0x17, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"Late Colli Cnt3&4",   0x18, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"Late Colli Window",   0x19, READ_WRITE, {2}, 0x1F00, 0x0000},
    {"Misc Test",           0x1A, READ_WRITE, {2}, 0x9FA0, 0x1900},
    {"Temp Sensor",         0x1B, READ_WRITE, {2}, 0x0000, 0x0000},
    {"end",                 0x00, 0, {0}, 0, 0},
};

static const reg_info_t marvell_88e1548_reg_page7[] = {   /* Page 7*/
    {"PHY Cable Diag 0",    0x10, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"PHY Cable Diag 1",    0x11, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"PHY Cable Diag 2",    0x12, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"PHY Cable Diag 3",    0x13, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"PHY Cable Diag Relt", 0x14, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"PHY Cable Diag Cntl", 0x15, READ_WRITE, {2}, 0x6400, 0x4000},
    {"Page Register",       0x16, READ_WRITE, {2}, 0x0000, 0x0000},
    {"Adv VCT Cros Pair",   0x19, READ_WRITE, {2}, 0x7F7F, 0x0104},
    {"Adv VCT Same Pair 01",0x1A, READ_WRITE, {2}, 0x7F7F, 0x0F12},
    {"Adv VCT Same Pair 23",0x1B, READ_WRITE, {2}, 0x7F7F, 0x0A0C},
    {"Adv VCT Same Pair 4", 0x1C, READ_WRITE, {2}, 0x007F, 0x0006},
    {"end",                 0x00, 0, {0}, 0, 0},
};

static const reg_info_t marvell_88e1548_reg_page8[] = {   /* Page 8*/
    {"PTP Port Conf 0",     0x00, READ_ONLY,  {2}, 0x0000, 0x1000},
    {"PTP Port Conf 1",     0x01, READ_ONLY,  {2}, 0x0000, 0x020C},
    {"PTP Port Conf 2",     0x02, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"PTP Arr0 Port Stat",  0x08, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"PTP Arr0 Byte1&0",    0x09, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"PTP Arr0 Byte3&2",    0x0A, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"PTP Arr0 Sequ ID",    0x0B, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"PTP Arr1 Port Stat",  0x0C, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"PTP Arr1 Byte1&0",    0x0D, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"PTP Arr1 Byte3&2",    0x0E, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"PTP Arr1 Sequ ID",    0x0F, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"Page Register",       0x16, READ_WRITE, {2}, 0x0000, 0x0000},
    {"end",                 0x00, 0, {0}, 0, 0},
};

static const reg_info_t marvell_88e1548_reg_page9[] = {   /* Page 9*/
    {"PTP Dep Status",      0x00, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"PTP Dep Byte1&0",     0x01, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"PTP Dep Byte3&2",     0x02, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"PTP Dep Sequ ID",     0x03, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"PTP Dep Cnt",         0x05, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"Page Register",       0x16, READ_WRITE, {2}, 0x0000, 0x0000},
    {"end",                 0x00, 0, {0}, 0, 0},
};

static const reg_info_t marvell_88e1548_reg_page12[] = {   /* Page 12*/
    {"TAI Global Conf 0",   0x00, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"TAI Global Conf 1",   0x01, READ_ONLY,  {2}, 0x0000, 0x1F40},
    {"TAI Global Conf 2",   0x02, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"TAI Global Conf 3",   0x03, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"TAI Global Conf 4",   0x04, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"TAI Global Conf 5",   0x05, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"TAI Global Conf 8",   0x08, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"TAI Globle Conf 9",   0x09, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"Event Cap Byte1&0",   0x0A, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"Event Cap Byte3&2",   0x0B, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"TAI Global Conf 12",  0x0C, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"TAI Global Conf 13",  0x0D, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"PTP Global Time1&0",  0x0E, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"PTP Global Time3&2",  0x0F, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"Page Register",       0x16, READ_WRITE, {2}, 0x0000, 0x0000},
    {"end",                 0x00, 0, {0}, 0, 0},
};

static const reg_info_t marvell_88e1548_reg_page14[] = {   /* Page 14*/
    {"PTP Global Conf 0",   0x00, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"PTP Global Conf 1",   0x01, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"PTP Global Conf 2",   0x02, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"PTP Global Conf 3",   0x03, READ_ONLY,  {2}, 0x0000, 0x0001},
    {"PTP Global Status",   0x08, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"ReadPlus Cmd Reg",    0x0E, READ_WRITE, {2}, 0x0000, 0x0000},
    {"ReadPlus Data Reg",   0x0F, READ_WRITE, {2}, 0x0000, 0x0000},
    {"Page Register",       0x16, READ_WRITE, {2}, 0x0000, 0x0000},
    {"end",                 0x00, 0, {0}, 0, 0},
};

static const reg_info_t marvell_88e1548_reg_page16[] = {   /* Page 16*/
    {"LinkCrypt RD/ADDR",   0x00, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"LinkCrypt WR/ADDR",   0x01, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"LinkCrypt DATALo",    0x02, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"LinkCrypt DATAHi",    0x03, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"end",                 0x00, 0, {0}, 0, 0},
};

static const reg_info_t marvell_88e1548_reg_page18[] = {   /* Page 18*/
    {"PTP Global Conf 0",   0x00, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"Page Generation",     0x10, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"CRC Counters",        0x11, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"Checker Ctrl",        0x12, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"PKT GEN IPG Ctrl",    0x13, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"General Ctrl Reg 1",  0x14, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"Page Register",       0x16, READ_WRITE, {2}, 0x0000, 0x0000},
    {"Link Disconnect Cnt", 0x19, READ_WRITE, {2}, 0x0000, 0x0000},
    {"Cop/SER RX_ER Byt Cap",0x1A, READ_WRITE, {2}, 0x0000, 0x0000},
    {"General Ctrl Reg 2",  0x1B, READ_WRITE, {2}, 0x0000, 0x0000},
    {"end",                 0x00, 0, {0}, 0, 0},
};

static const mrvl_phy_regs_t marvell_88e1548_phy_reg_tbl[] = {
    {"Page  0",     0, marvell_88e1548_reg_page0},
    {"Page  1",     1, marvell_88e1548_reg_page1},
    {"Page  2",     2, marvell_88e1548_reg_page2},
    {"Page  3",     3, marvell_88e1548_reg_page3},
    {"Page  4",     4, marvell_88e1548_reg_page4},
    {"Page  5",     5, marvell_88e1548_reg_page5},
    {"Page  6",     6, marvell_88e1548_reg_page6},
    {"Page  7",     7, marvell_88e1548_reg_page7},
    {"Page  8",     8, marvell_88e1548_reg_page8},
    {"Page  9",     9, marvell_88e1548_reg_page9},
    {"Page 12",    12, marvell_88e1548_reg_page12},
    {"Page 14",    14, marvell_88e1548_reg_page14},
    {"Page 16",    16, marvell_88e1548_reg_page16},
    {"Page 18",    18, marvell_88e1548_reg_page18},
};


#define NUM_MEDIA_PHY_PAGES (sizeof(marvell_88e1548_phy_reg_tbl) /      \
                             sizeof(struct mrvl_phy_regs_t_))


#define F_GRP	     (MF_CONTINUOUS | MF_DOGRP)
#define F_GRP_E	     (F_GRP | MF_SHOW_ERRCOUNT)
#define F_ALL	     (F_GRP | MF_DOALL)
#define F_ALL_E      (F_ALL | MF_SHOW_ERRCOUNT)

/* Ethernet port utility menu
 */
static submenu_xtable_t eth_util_submenu_table[] = {
    /* ---------- Below are utilities -----------*/
    {"Media PHY (1548) test mode util",
        (type_t(*)()) ovld_media_phy_testmode_util,   0,
        0,       (type_t(*)())0,         0, 
        (type_t(*)())0,   0},
    {"Send packet Util",
        (type_t(*)())tmp_ovld_set_packet,   0,
	0, (type_t(*)())0,               0, 
        (type_t(*)())0,   0},
    {"Access SGMII PHY registers",
        (type_t(*)())vg400_phy_reg_access,   0,
	0, (type_t(*)())0,               0,
        (type_t(*)())0,   0},
    {"Show SGMII PHY registers",
        (type_t(*)())sgmii_phy_reg_dump,    0,
	0, (type_t(*)())0,               0,
        (type_t(*)())0,   0},
    {"Reset 1548 PHY",
        (type_t(*)())reset_quad_phy,        0,
	F_GRP, (type_t(*)())0,           0,
        (type_t(*)())0,   0},
    {"1548 PHY eye enlarge", 
        (type_t(*)())utah_marvell_phy_eye_enlarge,   0,
	F_GRP, (type_t(*)())0,           0, 
        (type_t(*)())0,   0},

};

#define ETH_UTIL_SUBMENU_TABLE_SIZE (sizeof(eth_util_submenu_table) / \
					 sizeof(submenu_xtable_t))

/*
 * Primary & secondary submenu items (filled in from xtable)
 */
static mitem_t eth_util_primary_items[ETH_UTIL_SUBMENU_TABLE_SIZE +
					  MAX_BASE_ITEMS];
static mitem_t eth_util_secondary_items[ETH_UTIL_SUBMENU_TABLE_SIZE +
					    MAX_BASE_ITEMS];

static menuinfo_t eth_util_menu = {
    "Ethernet Port Utility Menu",
    0,                                /* mtparam added by init_empty_menu */
    (PFT)show_endnote,                /* notes missing WICs in combos */
    0,                                /* use generic prompt */
    0,                                /* size (bumped by add_menu_item() */
    eth_util_primary_items,
};
static menuinfo_t *eth_util_submenup = &eth_util_menu;

int build_eth_util_menu(void);

/* Sub Menu used for Ethernet port tests.
 */
static submenu_xtable_t eth_port_tests_submenu_table[] = {

   /* ------------ SGMII copper and SFP loopback tests -------------*/
   {"Vg400 GE PHY 1548 Register Test",
        (type_t(*)()) phy_1548_reg_test,   0,
        F_ALL_E, (type_t(*)())0, 0, (type_t(*)())0,   0},
   {"Vg400 GE PHY 1548 Interrupt Test",
        (type_t(*)()) phy_1548_interrupt_test,   0,
        F_ALL_E, (type_t(*)())0, 0, (type_t(*)())0,   0},
   {"Vg400 GE0 PHY Ext/Internal loopback test",
        (type_t(*)()) vg400_ge_phy_int_ext_lpbk_test,   0,
        F_ALL_E, (type_t(*)())0, 0, (type_t(*)())0,   0},
   {"Vg400 GE1 PHY Ext/Internal loopback test",
        (type_t(*)())vg400_ge_phy_int_ext_lpbk_test,   1,
        0, (type_t(*)())0, 0, (type_t(*)())0,   0},
   /* ---------- Below are utilities -----------*/
    {"Ethernet port utility menu",
         (PFT)build_eth_util_menu, 0,
         0, (type_t(*)())0, 0, (type_t(*)())0,   0},
};

#define ETH_PORT_TESTS_SUBMENU_TABLE_SIZE (sizeof(eth_port_tests_submenu_table) / \
					 sizeof(submenu_xtable_t))

/*
 * Primary & secondary submenu items (filled in from xtable)
 */
static mitem_t eth_port_tests_primary_items[ETH_PORT_TESTS_SUBMENU_TABLE_SIZE +
					  MAX_BASE_ITEMS];
static mitem_t eth_port_tests_secondary_items[ETH_PORT_TESTS_SUBMENU_TABLE_SIZE +
					    MAX_BASE_ITEMS];

menuinfo_t eth_port_subtest_menu = {
    "%s Subtest Menu",
    0,                                /* mtparam added by init_empty_menu */
    (PFT)show_endnote,                /* notes missing WICs in combos */
    0,                                /* use generic prompt */
    0,                                /* size (bumped by add_menu_item() */
    eth_port_tests_primary_items,
};
menuinfo_t *eth_port_submenup = &eth_port_subtest_menu;

/*------------------------------------------------------------------
 *
 * Function: eth_port_test_main
 *	This is the entry point for the ethernet port main
 *	test.
 *
 * Input:  exec_tests = 0 show submenu, !=0 perform all tests
 *
 * Output: PASSED/FAILED
 *
 *------------------------------------------------------------------
 */
int eth_port_test_main (int exec_tests)
{
	
    build_primary_submenu(eth_port_tests_submenu_table,
			  ETH_PORT_TESTS_SUBMENU_TABLE_SIZE,
                          "Ethernet port", &eth_port_submenup);
    build_secondary_submenu(eth_port_tests_submenu_table,
                            ETH_PORT_TESTS_SUBMENU_TABLE_SIZE,
                            eth_port_tests_secondary_items);

    /* Initialize the ethernet ports on the system.
     */
    system("ifconfig eth0 10.10.10.10 netmask 255.255.0.0 promisc -arp -allmulti -multicast");
    system("ifconfig eth1 11.11.11.11 netmask 255.255.0.0 promisc -arp -allmulti -multicast");

    dagger_sgmii_setting();

    if (exec_tests) {
        exec_doall_menu_items(eth_port_submenup);
    } else {
        menu(eth_port_submenup, eth_port_tests_secondary_items, '\0' );
    }
    return (PASSED);
}

/*------------------------------------------------------------------
 *
 * Function: build_eth_util_menu
 *      This is the entry point for the ethernet port utilities. 
 *
 * Input:  None.
 *
 * Output: PASSED/FAILED
 *
 *------------------------------------------------------------------
 */
static int build_eth_util_menu (void)
{
    build_primary_submenu(eth_util_submenu_table,
			  ETH_UTIL_SUBMENU_TABLE_SIZE,
                          "pfix", &eth_util_submenup);
    build_secondary_submenu(eth_util_submenu_table,
                            ETH_UTIL_SUBMENU_TABLE_SIZE,
                            eth_util_secondary_items);
    menu(eth_util_submenup, eth_util_secondary_items, 0 );
    return (PASS);
}

/*
 * Function: utah_phy_reg_wr
 *
 * Description:
 * Write marvell 88E1112C PHY register
 *
 * Input:
 * portnum - port number
 * regnum - register number
 * regval - val write to the reg
 *
 * Return: pass/fail
 */
int utah_phy_reg_wr (uint portnum, ushort regnum, ushort regval)
{
    struct mii_ioctl_data *miip;
    int sk;
    struct ifreq ethreq;

    /* Create socket for ioctl calls
     */
    sk = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_IP));
    if (-1 == sk) {
        printf("%s() Error Creating RX Socket", __FUNCTION__);
        return (FAILED);
    }

    sprintf(ethreq.ifr_name, "eth%d", portnum);

    miip = (struct mii_ioctl_data *)&ethreq.ifr_ifru;

    miip->reg_num = regnum;
    miip->val_in = regval;

    /* Note: This ioctl call will get to dev_ioctl() in linux/core/dev.c.
     * The cmd SIOCSMIIREG will eventually get to calling
     * cvm_oct_ioctl in deriver/net/octeon/ethernet-mdio.c and
     * the finally phy_mii_ioctl() in phy.c
     */
    ioctl(sk, SIOCSMIIREG, &ethreq);

    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("\nwrote PHY reg %d = %#.4x\n", miip->reg_num, miip->val_in);
    }

    close(sk);

    /* NOTE: (before P1A) 
     *we need it since we need to resolve the timing issue after utah is available */
    msleep(PHY_REST_TIME);

    return (PASS);
}

/*
 * Function: utah_phy_reg_rd
 *
 * Description:
 * Read marvell 88E1112C PHY register
 *
 * Input:
 * portnum - port number
 * regnum - register number
 * buf - pointer to the data buffer to hold the return value
 *
 * Return: pass/fail
 */
int utah_phy_reg_rd (uint portnum, ushort regnum, ushort *buf)
{
    struct mii_ioctl_data *miip;
    int sk;
    struct ifreq ethreq;

    /* Create socket for ioctl calls
     */
    sk = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_IP));
    if (-1 == sk) {
        printf("%s() Error Creating RX Socket", __FUNCTION__);
        return (FAILED);
    }

    sprintf(ethreq.ifr_name, "eth%d", portnum);

    miip = (struct mii_ioctl_data *)&ethreq.ifr_ifru;

    miip->reg_num = regnum;
    ioctl(sk, SIOCGMIIREG, &ethreq);
    *buf = miip->val_out;

    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("\nread PHY reg %d = %#.4x\n", miip->reg_num, miip->val_out);
    }

    close(sk);

    /* NOTE: (before P1A) 
     *we need it since we need to resolve the timing issue after utah is available */
    msleep(PHY_REST_TIME);

    return (PASSED);
}


/*
 * Function: vg400_phy_reg_rd
 *
 * Description:
 * Read VG400 PHY register
 *
 * Input:
 * portnum - port number
 * regnum - register number
 * buf - pointer to the data buffer to hold the return value
 *
 * Return: pass/fail
 */
int vg400_phy_reg_rd (uint portnum, ushort regnum, ushort *buf)
{
    struct mii_ioctl_data *miip;
    int sk, rc;
    struct ifreq ethreq;

    /* Create socket for ioctl calls
     */
    sk = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_IP));
    if (-1 == sk) {
        printf("%s() Error Creating RX Socket", __FUNCTION__);
        return (FAILED);
    }

    sprintf(ethreq.ifr_name, "eth%d", portnum);

    miip = (struct mii_ioctl_data *)&ethreq.ifr_ifru;

    miip->reg_num = regnum;
    rc = ioctl(sk, SIOCRMIIREG, &ethreq);
    if (rc == -1) {
        cterr('f',0,"Calling Ioctl function failed\n"); 
        close(sk);
        return (FAILED);
    }
    *buf = miip->val_out;

    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("\nread PHY reg %d = %#.4x\n", miip->reg_num, miip->val_out);
    }

    close(sk);

    return (PASSED);
}


/*
 * Function: vg400_phy_reg_wr
 *
 * Description:
 * Write VG400 PHY register
 *
 * Input:
 * portnum - port number
 * regnum - register number
 * regval - val write to the reg
 *
 * Return: pass/fail
 */
int vg400_phy_reg_wr (uint portnum, ushort regnum, ushort regval)
{
    struct mii_ioctl_data *miip;
    int sk, rc;
    struct ifreq ethreq;

    /* Create socket for ioctl calls
     */
    sk = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_IP));
    if (-1 == sk) {
        printf("%s() Error Creating RX Socket", __FUNCTION__);
        return (FAILED);
    }

    sprintf(ethreq.ifr_name, "eth%d", portnum);

    miip = (struct mii_ioctl_data *)&ethreq.ifr_ifru;

    miip->reg_num = regnum;
    miip->val_in = regval;

    /* Note: This ioctl call will get to dev_ioctl() in linux/core/dev.c.
     * The cmd SIOCSWIIREG will eventually get to calling
     * cvm_oct_ioctl in deriver/net/octeon/ethernet-mdio.c and
     * the finally phy_mii_ioctl() in phy.c
     */
    rc = ioctl(sk, SIOCWMIIREG, &ethreq);
    if (rc == -1) {
        cterr('f',0,"Calling Ioctl function failed\n"); 
        close(sk);
        return (FAILED);
    }

    if ((NVRAM)->diagflag & D_VERBOSE) {  
        printf("\nwrote PHY reg %d = %#.4x\n", miip->reg_num, miip->val_in);
    }

    close(sk);

    return (PASSED);
}


/*
 * Function: vg400_phy_select
 *
 * Description:
 *   Utah specific. using patched igb driver to select current mdio bus.
 *   for accessing port 0-3:
 *      bridge PHY (mdiobus 4-7)
 *       media PHY (mdiobus 0-3)
 * Input: mdio_bus_num - mdio bus number
 *
 * Return: PASS/FAIL
 */
static int vg400_phy_select (int mdio_bus_num)
{
    struct mii_ioctl_data *miip;
    int sk, port, rc;
    struct ifreq ethreq;

    /* media  PHY port0=bus0, port1=bus1, port2=bus2, port3=bus3 *
     * bridge PHY port0=bus4, port1=bus5, port2=bus6, port3=bus7 *
     */
    if (mdio_bus_num < 4) {
        port = mdio_bus_num; /* media phy */
    } else { 
        port = mdio_bus_num - 4; /* bridge phy */
    }

    /* Create socket for ioctl calls
     */
    sk = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_IP));
    if (-1 == sk) {
        printf("%s() Error Creating RX Socket", __FUNCTION__);
        return (FAILED);
    }

    sprintf(ethreq.ifr_name, "eth%d", port);

    miip = (struct mii_ioctl_data *)&ethreq.ifr_ifru;

    /* sent bus num to select phy */
    miip->phy_id = mdio_bus_num;
    miip->reg_num = 0;
    miip->val_in = 0;
    rc = ioctl(sk, SIOCGMIIPHY, &ethreq);
    if (rc == -1) {
        cterr('f',0,"Calling Ioctl function failed\n"); 
        close(sk);
        return (FAILED);
    }

    close(sk);

    return (PASSED);
}

/*
 * Function: vg400_phy_reg_access
 *
 * Description: Utility to do peek and poke to PHY registers
 *
 * Input: none
 *
 * Return: none
 */
static void vg400_phy_reg_access (void)
{
    char c;
    ushort rdval, wrval;
    int busnum, bus_max = 7;
    int portnum;
    int regnum, regnum_max = 32;

    printf(" Media PHY port[0-3] = mdiobus[0-3] \n");
    printf("Bridge PHY port[0-3] = mdiobus[4-7] \n");
    busnum = getdec_answer("\nEnter PHY bus number", 0, 0, bus_max);

    /* select bus number */
    vg400_phy_select(busnum);

    if (busnum < 4) {
        portnum = busnum;
    } else {
        portnum = busnum - 4;
    }

    do {
        regnum = getdec_answer("\nEnter PHY reg number", 0, 0, regnum_max);
        utah_phy_reg_rd(portnum, regnum, &rdval);
        printf("Current value of reg %d = (%d)%#.4x\n", regnum, rdval ,rdval);

        c = getc_answer("Do you want to change value?", "yn",'n');

        if (c == 'y') {
            wrval = gethex_answer("Enter value:", 0, 0, 0xffff);
            utah_phy_reg_wr(portnum, regnum, wrval);
            utah_phy_reg_rd(portnum, regnum, &rdval);
            printf("Read back reg %d = %#.4x\n", regnum, rdval);
        }
    } while(getc_answer("Continue?", "yn", 'y') == 'y');

    return;
}

/*
 * Function: utah_marvell_1340_init
 *
 * Description:
 *   Utah specific. Special init for marvell 1340 PHY
 *   Init the 1340 PHY according to Marvell's device release note
 *
 * Input: none
 *
 * Return: void
 */
void utah_marvell_1340_init (void)
{
    int port, phy_addr;
    ushort rdval;

    for (port = 0; port <= SGMII2; port++) {
        phy_addr = port + ADDR_BRIDGE_PHY;

        /* select mdio bus to bridge PHY */
        vg400_phy_select(phy_addr);

        /* Init sequence #1
         */
        utah_phy_reg_wr(port, OVLD_PHY_PAGE22, 0x00ff);
        utah_phy_reg_wr(port, PHY_REG(PHY_PAGE24), 0x2800);
        utah_phy_reg_wr(port, PHY_REG(PHY_PAGE23), 0x2001);
        utah_phy_reg_wr(port, OVLD_PHY_PAGE22, 0x0000);

        /* Check init sequence #1
         */
        utah_phy_reg_wr(port, OVLD_PHY_PAGE22, 0x00ff);
        utah_phy_reg_wr(port, PHY_REG(PHY_PAGE23), 0x1001);
        utah_phy_reg_rd(port, PHY_REG(PHY_PAGE25), &rdval);
        utah_phy_reg_wr(port, OVLD_PHY_PAGE22, 0x0000);

        /* Init sequence #2 for the BGA package
         */
        utah_phy_reg_wr(port, OVLD_PHY_PAGE22, 0x0000);
        utah_phy_reg_wr(port, PHY_REG(PHY_PAGE29), 0x0003);
        utah_phy_reg_wr(port, PHY_REG(PHY_PAGE30), 0x0002);
        utah_phy_reg_wr(port, PHY_REG(PHY_PAGE29), 0x0000);

        /* Check init sequence #2
         */
        utah_phy_reg_wr(port, PHY_REG(PHY_PAGE29), 0x0003);
        utah_phy_reg_rd(port, PHY_REG(PHY_PAGE30), &rdval);
        utah_phy_reg_wr(port, PHY_REG(PHY_PAGE29), 0x0000);

        /* Turn on the power
         * Set page 4 reg 0 (QSGMII control reg) power
         * down bit to normal
         */
        utah_phy_reg_wr(port, OVLD_PHY_PAGE22, 0x0004);
        utah_phy_reg_rd(port, PHY_REG(PHY_PAGE0), &rdval);
        rdval &= ~0x0800;
        utah_phy_reg_wr(port, PHY_REG(PHY_PAGE0), rdval);
        utah_phy_reg_rd(port, PHY_REG(PHY_PAGE0), &rdval);

        /* set register page 0 */
        utah_phy_reg_wr(port, OVLD_PHY_PAGE22, 0x0000);

        /* select mdio bus back to media PHY */
        vg400_phy_select(port);
    }

    return;
}

/*
 * Function: utah_marvell_phy_eye_enlarge
 *
 * Description:
 *   Enlarge the eye of the 1548 and 1340 phy according to HW request.
 *   dagger has only one PHY 1548. 
 * Input: none
 *
 * Return: void
 */
void utah_marvell_phy_eye_enlarge (void)
{
    int port, phy_addr;
    ushort rdval;

    for (port = 0; port <= SGMII3; port++) {
        phy_addr = port;

        /* select mdio bus to bridge or media PHY */
        vg400_phy_select(phy_addr);

        utah_phy_reg_wr(port, OVLD_PHY_PAGE22, 0x0001);
        utah_phy_reg_rd(port, PHY_REG(PHY_PAGE26), &rdval);
        rdval &= ~BRIDGE_MASK1;
        rdval |= BRIDGE_MASK2;
        utah_phy_reg_wr(port, PHY_REG(PHY_PAGE26), rdval);
        utah_phy_reg_wr(port, OVLD_PHY_PAGE22, 0x0000);

        /* select mdio bus back to media PHY */
        vg400_phy_select(port);
        
    }

    return; 
}

/*******************************************************************************
 *
 * Function   : dagger_sgmii_setting
 * Description: per HW request, we need to have special setting on 1548 PHY
 * Inputs     : none
 * Outputs    : none
 *
 *******************************************************************************
 */
void dagger_sgmii_setting (void) {

    ushort rdval, wrval, port, phy_addr;
 
    for (port = SGMII0; port <= SGMII3; port++) {
       phy_addr = port;

       /* make sure mdio bus is for media PHY */
       vg400_phy_select(phy_addr);

       /* 1st part: internal QSGMII loopback and QSGMII output crossover. */
       /* *** page4 reg27 is qsgmii global control register,
        * *** all the ports share the same setting. 
        * *** this is why we only do once on here. 
        */
       if (port == SGMII0) {
           /* page 4, reg 27, bit 14 = 1 */
           /* page 4, reg 27, bit 1 = 1 for port2 and port3 */
           /* page 4, reg 27, bit 0 = 1 for port0 and port1 */

           utah_phy_reg_wr(port, PHY_REG(PHY_PAGE22), 0x0004);
           utah_phy_reg_rd(port, PHY_REG(PHY_PAGE27), &rdval);
           rdval |= 0x4003;

           utah_phy_reg_wr(port, PHY_REG(PHY_PAGE27), rdval);
       }
       /* end of 1st part */

       /* 2nd part: mode selection */
       /* page 18, reg 20 port0 and port2 for SGMII to QSGMII media */
       utah_phy_reg_wr(port, PHY_REG(PHY_PAGE22), 0x0012);

       if (port == SGMII0 || port == SGMII2) {
           wrval = 0x0205;
       } else {
           wrval = 0x0207;
       }
       utah_phy_reg_wr(port, PHY_REG(PHY_PAGE20), wrval);

       if (port == SGMII0 || port == SGMII2) {
           wrval = 0x8205;
       } else {
           wrval = 0x8207;
       }
       utah_phy_reg_wr(port, PHY_REG(PHY_PAGE20), wrval);

       wrval = 0x1;
       utah_phy_reg_wr(port, PHY_REG(PHY_PAGE22), wrval);
       wrval = 0x1140;
       utah_phy_reg_wr(port, PHY_REG(PHY_PAGE0), wrval);
       wrval = 0x0;
       utah_phy_reg_wr(port, PHY_REG(PHY_PAGE22), wrval);
       /* end of 2nd part */
    }

    return;
}

/*******************************************************************************
 *
 * Function: dump_phy_reg()
 *
 * This function prints the specific PHY register values
 *
 * Input: curr_port - current port (wit offset)
 *        page_reg_ptr - page table pointer. 
 *
 * Output: PASSED/FAILED
 *
 *******************************************************************************
 */
static void dump_phy_reg (int curr_port, const mrvl_phy_regs_t *page_reg_ptr)
{
    ushort rdval;
    const reg_info_t *reg_ptr;

    utah_phy_reg_wr(curr_port, OVLD_PHY_PAGE22, page_reg_ptr->pagenum);

    printf("\n%s\n", page_reg_ptr->pagename);
    reg_ptr = page_reg_ptr->pageregs;

    while (reg_ptr->size.size != 0) {
        utah_phy_reg_rd(curr_port, reg_ptr->offset, &rdval);

        printf("%-32s reg %.2d = %#.8x\n", reg_ptr->name,
                                       reg_ptr->offset, rdval);
        reg_ptr++;
        msleep(10); /* wait for a while for next register. */
    }
}


/*******************************************************************************
 *
 * Function: phy_reg_show()
 *
 * This function get PHY page/regs info and call dump_phy_reg() to prints  
 * PHY registers. 
 *
 * Input: port - current port (without offset)
 *        phy_sel - PHY offset value to get specifc PHY addr.
 *        page_sel - select PHY page. 
 *        dump_type - dump on page or all pages. 
 *
 * Output: PASSED/FAILED
 *
 *******************************************************************************
 */
int phy_reg_show (int port, int phy_sel, int page_sel, boolean dump_type)
{
    uint ix, page_move, curr_bus;
    const mrvl_phy_regs_t *page_reg_ptr;

    if (phy_sel == ADDR_MEDIA_PHY) {
        page_reg_ptr = &marvell_88e1548_phy_reg_tbl[0];
        page_move = NUM_MEDIA_PHY_PAGES;
    } else if (phy_sel == ADDR_BRIDGE_PHY) {
        page_reg_ptr = &marvell_88e1340_phy_reg_tbl[0];
        page_move = NUM_BRIDGE_PHY_PAGES;
    } else {
        printf("\nUnknown PHY. \n");
        return (FAILED);
    }

    /* select the mdio bus for the PHY */
    curr_bus = port + phy_sel;

    /* select bus number */
    vg400_phy_select(curr_bus);

    /* start dump regs */
    if (dump_type == DUMP_ALL_PAGE) {
        
        /* dump all page */
        for (ix = 0; ix < page_move; ix++) {
            dump_phy_reg(port, page_reg_ptr);
            page_reg_ptr++;
        }

    } else { /* dump_type == DUMP_ONE_PAGE */

        /* skip to selected page */
        while (page_reg_ptr->pagenum != page_sel)
            page_reg_ptr++;
 
        dump_phy_reg(port, page_reg_ptr);
    }

    /* restore bus to media PHY anyway. */
    vg400_phy_select(port);

    return (PASSED);
}

/*
 * Function: sgmii_phy_reg_dump
 *
 * This function displays the PHY setting of the requested SGMII port.
 * Using phy_reg_show() to dump PHY page.
 *
 * Input: none.
 *
 * Output: void
 */
static void sgmii_phy_reg_dump (void)
{
    int port, phy_sel, dump_type, page_sel = 0;

    port = getdec_answer("\nEnter SGMII port number", 0, 0, PLAT_SGMII_NUM_MAX);
    phy_sel = getdec_answer("\nSelect 0:Ext. 1:Int. PHY", 0, 0, 1);

    if (phy_sel) {
        phy_sel = ADDR_BRIDGE_PHY;
    } else {
        phy_sel = ADDR_MEDIA_PHY;
    }
    if (getc_answer("Dump ALL Page?", "yn", 'y') == 'y') {
        dump_type = DUMP_ALL_PAGE;

    } else {
        /* select page. */
        dump_type = DUMP_ONE_PAGE;
        page_sel = getdec_answer("\nSelect PAGE", 0, 0, PLAT_PAGE_NUM_MAX);
    }

    phy_reg_show(port, phy_sel, page_sel, dump_type);
}
 

/*------------------------------------------------------------------
 * Function: check_ext_lpbk_flag
 *
 * Check if external loopback flag is set in diag
 *
 * Input:  NONE
 *
 * Output: TRUE/FALSE
 *
 *------------------------------------------------------------------
 */
int check_ext_lpbk_flag (void)
{
    /* according to menu_show_dflags(), D_EXT_LPBK is inverse flag */ 
    if ((NVRAM)->diagflag & D_EXT_LOOPBACK) {
        return (FALSE);
    } else { 
        return (TRUE);
    }
}

/************ Dummy function function **********/
int cvmx_mdio_write (int we, int are, int dummy, int para) {

   return (PASSED);

}

int cvmx_mdio_read (int worked, int on, int cavium) {

   return (PASSED);

}

/************ Dummy function function **********/


/*
 * Function: ovld_phy_reg_wr
 *
 */
int ovld_phy_reg_wr(int phy_id, int reg, int val)
{
    /* Vg400 doesn't have 88E1340 PHY */
    return (0);
}

/*
 * Function: ovld_phy_reg_rd
 *
 */
int ovld_phy_reg_rd(int phy_id, int reg)
{
    /* Vg400 doesn't have 88E1340 PHY */
    return (0);
}

/*
 * Function: phy_reg_wr
 *
 * Description:
 * Write marvell 88E1112C PHY register
 *
 * Input:
 * sk - socket id for ioctl()
 * ethreq_p - ptr to ifreq data structure for Linux MII reg access
 * regnum - register number
 * regval - val write to the reg
 *
 * Return: pass/fail
 */
int phy_reg_wr (int sk, struct ifreq *ethreq_p, ushort regnum, ushort regval)
{
    struct mii_ioctl_data *miip;

    miip = (struct mii_ioctl_data *)&ethreq_p->ifr_ifru;

    miip->reg_num = regnum;
    miip->val_in = regval;

    /* Note: This ioctl call will get to dev_ioctl() in linux/core/dev.c.
     * The cmd SIOCSMIIREG will eventually get to calling
     * cvm_oct_ioctl in deriver/net/octeon/ethernet-mdio.c and
     * the finally phy_mii_ioctl() in phy.c
     */
    ioctl(sk, SIOCSMIIREG, ethreq_p);

    printf("\nwrote PHY reg %d = %#.4x\n", miip->reg_num, miip->val_in);

    return (PASS);
}

/*
 * Function: phy_reg_rd
 *
 * Description:
 * Read marvell 88E1112C PHY register
 *
 * Input:
 * sk - socket id for ioctl()
 * ethreq_p - ptr to ifreq data structure for Linux MII reg access
 * regnum - register number
 * buf - pointer to the data buffer to hold the return value
 *
 * Return: pass/fail
 */
int phy_reg_rd (int sk, struct ifreq *ethreq_p, ushort regnum, ushort *buf)
{
    struct mii_ioctl_data *miip;

    miip = (struct mii_ioctl_data *)&ethreq_p->ifr_ifru;

    miip->reg_num = regnum;
    ioctl(sk, SIOCGMIIREG, ethreq_p);
    *buf = miip->val_out;

    printf("\nread PHY reg %d = %#.4x\n", miip->reg_num, miip->val_out);

    return (PASS);
}

/*
 * Function: marvell_1340_init
 *
 * Description:
 *   Special init for marvell 1340 PHY
 *   Init the 1340 PHY according to Marvell's device release note
 *
 * Input: none
 *
 * Return: void
 */
void marvell_1340_init (void)
{
    int port, phy_addr;
    ushort rdval;

    for (port = 0; port <= SGMII3; port++) {
        phy_addr = port + ADDR_BRIDGE_PHY;

	    /* Init sequence #1
	     */
	    ovld_phy_reg_wr(phy_addr, OVLD_PHY_PAGE22, 0x00ff);
	    ovld_phy_reg_wr(phy_addr, PHY_REG(24), 0x2800);
	    ovld_phy_reg_wr(phy_addr, PHY_REG(23), 0x2001);
	    ovld_phy_reg_wr(phy_addr, OVLD_PHY_PAGE22, 0x0000);

	    /* Check init sequence #1
	     */
	    ovld_phy_reg_wr(phy_addr, OVLD_PHY_PAGE22, 0x00ff);
	    ovld_phy_reg_wr(phy_addr, PHY_REG(23), 0x1001);
	    rdval = ovld_phy_reg_rd(phy_addr, PHY_REG(25));
	    ovld_phy_reg_wr(phy_addr, OVLD_PHY_PAGE22, 0x0000);

	    /* Init sequence #2 for the BGA package
	     */
	    ovld_phy_reg_wr(phy_addr, OVLD_PHY_PAGE22, 0x0000);
      	ovld_phy_reg_wr(phy_addr, PHY_REG(29), 0x0003);
      	ovld_phy_reg_wr(phy_addr, PHY_REG(30), 0x0002);
      	ovld_phy_reg_wr(phy_addr, PHY_REG(29), 0x0000);

	    /* Check init sequence #2
	    */
      	ovld_phy_reg_wr(phy_addr, PHY_REG(29), 0x0003);
	    rdval = ovld_phy_reg_rd(phy_addr, PHY_REG(30));
      	ovld_phy_reg_wr(phy_addr, PHY_REG(29), 0x0000);

	    /* Turn on the power
	     * Set page 4 reg 0 (QSGMII control reg) power
	     * down bit to normal
	     */
	    ovld_phy_reg_wr(phy_addr, OVLD_PHY_PAGE22, 0x0004);
	    rdval = ovld_phy_reg_rd(phy_addr, PHY_REG(0));
	    rdval &= ~0x0800;
      	ovld_phy_reg_wr(phy_addr, PHY_REG(0), rdval);
	    rdval = ovld_phy_reg_rd(phy_addr, PHY_REG(0));

	    /* set register page 0 */
	    ovld_phy_reg_wr(phy_addr, OVLD_PHY_PAGE22, 0x0000);
    }
}

unsigned int is_sfp_present (int sfp_num)
{
    /* No SFP fo VG400 */
    return (PASSED);
}

/*
 * Function: phy_1548_reg_test
 *
 * Description:
 *   Register test for PHY 1548
 *
 * Input: none
 *
 * Return: void
 */

int phy_1548_reg_test (void) {

    char *tname = "PHY 1548 Register";
    
    testname("%s", tname);
    prpass(testpass, "%s, ", tname); 

    if (phy_1548_register_tests(0,marvell_88e1548_reg_page0) == FAILED) {
        cterr('f', 0, "phy 1548 reg test fail\n");
        return (FAILED);
    }
    return (PASSED);
}


/*-------------------------------------------------
$Log: platform_eth.c,v $
Revision 1.4  2018/12/21 00:56:34  haohsu
CSCvn39819-Fix 1548 PHY Regsiter test fail

Revision 1.3  2018/09/27 06:10:53  haohsu
Fixed ACT2 can't get Chassis Serial Number

Revision 1.2  2018/08/30 06:47:15  haohsu
Collapse Vg400-branch to Main Trunk

Revision 1.19.38.3  2018/05/18 18:03:33  haohsu
Add Broad type Vg400

Revision 1.19.38.2  2018/03/15 00:33:12  haohsu
Code change for VG400

Revision 1.19.38.1  2018/01/26 09:14:31  haohsu
check in Vg400 branch code

Revision 1.19  2016/10/16 12:28:22  iachang
Supported Goldbeach Platform.

Revision 1.18  2014/06/20 08:12:58  alpeng
support GE testmode on dagger

Revision 1.17  2014/05/02 18:24:16  mcharon
replace strcpy with sprint when copying interface name to struct ifr

Revision 1.16  2014/02/19 09:11:34  alpeng
suport enhanced error code on loobpack tests

Revision 1.15  2014/01/14 08:54:59  alpeng
support SFP loopback test on dagger

Revision 1.14  2013/12/25 07:12:16  alpeng
support dagger eth external loopback test with speed 100Mbps

Revision 1.13  2013/12/21 01:38:05  ptong
Change typo in function name

Revision 1.12  2013/12/19 10:51:10  alpeng
support special phy setting for dagger per HW request

Revision 1.11  2013/12/17 08:11:03  alpeng
per HW request, support special setting on dagger

Revision 1.10  2013/12/06 11:58:58  danchung
Fix SGMII/SFP PHY loopback util menu for USD

Revision 1.9  2013/12/03 08:22:08  alpeng
support 1548 eye enlarge. power on ports before setting speed

Revision 1.8  2013/11/26 08:40:37  hroni
fix compiler warning

Revision 1.7  2013/11/22 00:14:45  alpeng
support eth ports testmode on utah/sword/dagger

Revision 1.6  2013/11/07 07:25:24  alpeng
support 1340 init, eye enlarge, and reset_quad_phy

Revision 1.5  2013/11/01 06:04:49  alpeng
support accessing bridge PHY on PHY utilities

Revision 1.4  2013/07/04 03:04:03  alpeng
clean up code, modify the menu structure and rearrange test ports.

Revision 1.3  2013/06/28 04:02:28  alpeng
for P1A check in, add media internal loopback test into menu

Revision 1.2  2013/06/14 10:22:23  alpeng
follow O2 menu structure

Revision 1.1  2013/05/31 11:03:40  alpeng
support front panel GE loopback test

Revision 1.26  2013/01/30 23:50:15  palin2
Add utility to set Cavium side GE PHY, Marvell 1548, into Test mode.

Revision 1.25  2013/01/25 10:47:02  alpeng
support macsec util

Revision 1.24  2012/11/03 01:28:36  ptong
Document and clean up

Revision 1.23  2012/10/18 05:19:09  ptong
Add marvell_1340_init and PHY reset util

Revision 1.22  2012/10/11 18:35:48  ptong
Add a missing register in the table

Revision 1.21  2012/10/05 09:12:10  alpeng
support media/bridge PHY register dump

Revision 1.20  2012/08/24 14:27:10  alpeng
fixed menu item name and display msg.

Revision 1.19  2012/08/22 10:04:23  alpeng
Using Ext. loopback flag to decide loopback test is internal or external loopback test. Moving media PHY diag item into debug utility menu

Revision 1.18  2012/08/11 00:00:18  ptong
Remove complile flag RELEASE_CVMX_DIAG

Revision 1.17  2012/08/10 20:18:54  ptong
Move xaui internal loopback test to the utility menu

Revision 1.16  2012/08/01 14:26:33  alpeng
adding check link up status for SFP and internal loopback

Revision 1.15  2012/07/18 22:59:29  ptong
Fix a problem so that (NVRAM)->diagflag is used correctly on Cavium data plane menu

Revision 1.14  2012/06/26 22:45:37  ptong
Fix typo

Revision 1.13  2012/06/25 07:02:14  alpeng
revert method for storing diag flag

Revision 1.12  2012/06/15 01:38:37  ptong
Added Ethernet debug util menu

Revision 1.11  2012/06/05 06:21:03  alpeng
clean up compiler warnings.

Revision 1.10  2012/05/23 09:20:08  alpeng
support is_sfp_present function for SFP test

Revision 1.9  2012/05/11 23:34:41  ptong
Use macro for diagflag.log

Revision 1.8  2012/05/08 00:05:15  ptong
Improve test printing

Revision 1.7  2012/04/29 04:38:30  ptong
Fix diag flag problem

Revision 1.6  2012/04/27 10:42:42  alpeng
fixed minor bugs and support set external loopback flag for controlling test flow

Revision 1.5  2012/04/27 01:02:15  ptong
Set SFP ext loopback test with the MF_DOALL flag

Revision 1.4  2012/04/11 21:27:16  ptong
Setup cavium named block for mailbox area, and use nc server on cavium to take command from host

Revision 1.3  2012/03/28 00:38:18  mcharon
remove forward slash from second line

Revision 1.2  2012/03/27 16:18:21  alpeng
cavium side code clean up

Revision 1.1.1.1  2012/03/23 23:02:07  ptong
Initial archive of ng_diag module


$Endlog$
*/
