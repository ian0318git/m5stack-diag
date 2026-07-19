/* $Id: dev_88e1543.c,v 1.8 2020/09/30 09:46:09 alicehua Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/chips/dev_88e1543_marvell/dev_88e1543.c,v $
 *------------------------------------------------------------------------------
 *
 * Filename:    dev_phy_88e1543.c
 *
 * Description: Marvell 88E1543 PHY device driver.
 *
 * Copyright (c) 2012 - 2019 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------------------
 *
 */
#include <stdlib.h>
#include "endians.h"
#include "types.h"
#include "defs.h"
#include "common.h"
#include "common_utils.h"
#include "dev_print.h"
#include "dev_object.h"
#include "dev_88e1543.h"
#include "free.h"
#include "proto.h"
#include "strings.h"
#include "nvmonvars.h"

/*===================================================================*
 *                    Function Prototypes                            *
 *===================================================================*/

static int dev_88e1543_led_on(dev_object_t *, uint);
static int dev_88e1543_led_off(dev_object_t *, uint);
static int dev_88e1543_led_default(dev_object_t *, uint);
static int dev_88e1543_check_if_plugged_with_download_cable(dev_object_t *, uint);

void dev_88e1543_create(dev_object_t *, dev_error_report_t);
static void dev_88e1543_destroy (dev_object_t **);

/*===================================================================*
 *                    Global variables                               *
 *===================================================================*/
static char *buf_p;
static char err_msg[MRV88E1543_ERR_MSG_LEN];
static int dev_88e154x_macsec_reg_wr(dev_object_t *, int, ushort, ushort, uint);
static int dev_88e154x_macsec_reg_rd(dev_object_t *, int , ushort, ushort);
static int dev_88e1543_copper_softreset(dev_object_t *, uint);
static uint32_t dev_88e1543_get_fiber_link_speed(dev_88e1543_object_t *, int, int *, int *);
static uint32_t dev_88e1543_get_link_speed(dev_88e1543_object_t *, int, int *, int *);
static uint32_t dev_88e1543_toggle_auto_nego(dev_88e1543_object_t *, int, int);
static uint32_t dev_88e1543_toggle_loopback_stub(dev_88e1543_object_t *, int, int);
static uint32_t dev_88e1543_toggle_loopback(dev_88e1543_object_t *, int, int);
static uint32_t dev_88e1543_config_phy_speed(dev_88e1543_object_t *, int, int);
static uint32_t dev_88e1543_config_mac_speed(dev_88e1543_object_t *, int, int);
static uint32_t dev_88e1543_advertise_speed(dev_88e1543_object_t *, int, int);
static uint32_t dev_88e1543_enable_force_interrupt(dev_88e1543_object_t *, int, int);
static uint32_t dev_88e1543_enable_interrupt_fiber_inserted (dev_88e1543_object_t *, int , int);

static uint32_t dev_88e1543_qsgmii_toggle_loopback_and_speed(dev_88e1543_object_t *, int, int, int);
static uint32_t dev_88e1543_mode_software_reset (dev_88e1543_object_t * , int, int);
static uint32_t dev_88e1543_qsgmii_software_reset (dev_88e1543_object_t *, int);
static int dev_88e1543_led_on(dev_object_t *, uint);
static int dev_88e1543_led_off(dev_object_t *, uint);

dev_object_fvt_t              m88e1543_fvt;
dev_88e1543_callin_fvt_t      m88e1543_callin;
dev_88e1543_callout_fvt_t     m88e1543_callout;
uint all_port_link_status[MRV88E1543_PORTS] = {0};

static const reg_info_t marvell_88e1543_reg_page0[] = {   /* Page 0*/
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
    {"Global Intr Status",  0x17, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"Copper Spec Cntl3",   0x1A, READ_WRITE, {2}, 0xFEFF, 0x0040},
    {"end",                 0x00, 0, {0}, 0, 0},
};

static const reg_info_t marvell_88e1543_reg_page1[] = {  /* Page 1*/
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
    {"PRBS Control",        0x17, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"PRBS Err Cnt LSB",    0x18, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"PRBS Err Cnt MSB",    0x19, READ_ONLY,  {2}, 0x0000, 0x0000},    
    {"end",                 0x00, 0, {0}, 0, 0},
};

static const reg_info_t marvell_88e1543_reg_page2[] = {   /* Page 2*/
    {"MAC Spec Cntl1",      0x10, READ_WRITE, {2}, 0xDBC8, 0x4004},
    {"MAC Spec Intr Ena",   0x12, READ_WRITE, {2}, 0x008C, 0x0000},
    {"MAC Intr Status",     0x13, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"MAC RX_ER Byte",      0x14, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"MAC Spec Cntl2",      0x15, READ_WRITE, {2}, 0x4008, 0x1046},
    {"end",                 0x00, 0, {0}, 0, 0},
};

static const reg_info_t marvell_88e1543_reg_page3[] = {   /* Page 3*/
    {"LED Func Cntl1",      0x10, READ_WRITE, {2}, 0xFFFF, 0x1777},
    {"LED Polarity Cntl",   0x11, READ_WRITE, {2}, 0xFFFF, 0x8800},
    {"LED Func Cntl&Polar", 0x13, READ_WRITE, {2}, 0xEFFF, 0x0073},
    {"end",                 0x00, 0, {0}, 0, 0},
};

static const reg_info_t marvell_88e1543_reg_page4[] = {   /* Page 4*/
    {"QSGMII Control",      0x00, READ_WRITE, {2}, 0x5C00, 0x1140},
    {"QSGMII Status",       0x01, READ_ONLY,  {2}, 0x0000, 0x7949},     
    {"QSGMII Auto-Neg",     0x04, READ_ONLY , {2}, 0x0000, 0x0001},
    {"QSGMII Link-P Abil",  0x05, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"QSGMII Auto-Neg Exp", 0x06, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"QSGMII Spec Status",  0x11, READ_ONLY,  {2}, 0x0000, 0xC040},
    {"QSGMII Spec Intr Ena",0x12, READ_WRITE, {2}, 0xFFFF, 0x0000},
    {"QSGMII Intr Status",  0x13, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"QSGMII RX_ER Byte",   0x14, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"QSGMII Rx Err Cnt",   0x15, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"PRBS Control",        0x17, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"PRBS Err Cnt LSB",    0x18, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"PRBS Err Cnt MSB",    0x19, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"QSGMII Global Cntr1", 0x1A, READ_WRITE, {2}, 0x7A04, 0xC000},
    {"QSGMII Global Cntr2", 0x1B, READ_WRITE, {2}, 0x7E03, 0x3E00},
    {"end",                 0x00, 0, {0}, 0, 0},
};

static const reg_info_t marvell_88e1543_reg_page5[] = {   /* Page 5*/
    {"Adv VCT TX MDI0",     0x10, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"Adv VCT TX MDI1",     0x11, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"Adv VCT TX MDI2",     0x12, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"Adv VCT TX MDI3",     0x13, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"1000BT Pair Skew",    0x14, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"1000BT Pair Swap",    0x15, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"Adv VCT Control",     0x17, READ_WRITE, {2}, 0x3FFF, 0x0000},
    {"Adv VCT Cross Pair",  0x18, READ_WRITE, {2}, 0x01FF, 0x0000},
    {"Adv VCT Same Pair 01",0x19, READ_WRITE, {2}, 0x7F7F, 0x0104},
    {"Adv VCT Same Pair 23",0x1A, READ_WRITE, {2}, 0x7F7F, 0x0F12},
    {"Adv VCT Same Pair 4", 0x1B, READ_WRITE, {2}, 0x7F7F, 0x0A0C},
    {"end",                 0x00, 0, {0}, 0, 0},
};

static const reg_info_t marvell_88e1543_reg_page6[] = {   /* Page 6*/
    {"Packet Generation",   0x10, READ_ONLY,  {2}, 0xFF06, 0x0000},
    {"CRC Counters",        0x11, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"Checker Control",     0x12, READ_WRITE, {2}, 0x0007, 0x0000},
    {"General Control",     0x14, READ_WRITE, {2}, 0x0F1F, 0x0200},
    {"Late Colli Cnt1&2",   0x17, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"Late Colli Cnt3&4",   0x18, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"Late Colli Window",   0x19, READ_WRITE, {2}, 0x1F00, 0x0000},
    {"Misc Test",           0x1A, READ_WRITE, {2}, 0x9FA0, 0x1900},
    {"end",                 0x00, 0, {0}, 0, 0},
};

static const reg_info_t marvell_88e1543_reg_page7[] = {   /* Page 7*/
    {"PHY Cable Diag 0",    0x10, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"PHY Cable Diag 1",    0x11, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"PHY Cable Diag 2",    0x12, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"PHY Cable Diag 3",    0x13, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"PHY Cable Diag Relt", 0x14, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"PHY Cable Diag Cntl", 0x15, READ_WRITE, {2}, 0x6400, 0x4000},
    {"Adv VCT Cros Pair",   0x19, READ_WRITE, {2}, 0x7F7F, 0x0104},
    {"Adv VCT Same Pair 01",0x1A, READ_WRITE, {2}, 0x7F7F, 0x0F12},
    {"Adv VCT Same Pair 23",0x1B, READ_WRITE, {2}, 0x7F7F, 0x0A0C},
    {"Adv VCT Same Pair 4", 0x1C, READ_WRITE, {2}, 0x007F, 0x0006},
    {"end",                 0x00, 0, {0}, 0, 0},
};

static const reg_info_t marvell_88e1543_reg_page18[] = {   /* Page 18*/
    {"EEE Ctrl Reg 1",      0x00, READ_WRITE, {2}, 0x0001, 0x0600},
    {"EEE Ctrl Reg 2",      0x01, READ_WRITE, {2}, 0xFFFF, 0x111E},     
    {"EEE Ctrl Reg 3",      0x02, READ_WRITE, {2}, 0xFFFF, 0x111E},
    {"Packet Generation",   0x10, READ_ONLY,  {2}, 0xFF1F, 0x0000},
    {"CRC Counters",        0x11, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"end",                 0x00, 0, {0}, 0, 0},
};

static const mrvl_88e1543_phy_regs_t marvell_88e1543_phy_reg_tbl[] = {
    {"Page  0",     0, marvell_88e1543_reg_page0},
    {"Page  1",     1, marvell_88e1543_reg_page1},
    {"Page  2",     2, marvell_88e1543_reg_page2},
    {"Page  3",     3, marvell_88e1543_reg_page3},
    {"Page  4",     4, marvell_88e1543_reg_page4},
    {"Page  5",     5, marvell_88e1543_reg_page5},
    {"Page  6",     6, marvell_88e1543_reg_page6},
    {"Page  7",     7, marvell_88e1543_reg_page7},
    {"Page 18",    14, marvell_88e1543_reg_page18},
};

static const mrvl_88e1543_macsec_regs_info_t marvell_88e1543_macsec_reg_tbl[] = {
    {"MRV88E154X_IGR_HIT",       MRV88E154X_IGR_HIT       },
    {"MRV88E154X_IGR_OK",        MRV88E154X_IGR_OK        },
    {"MRV88E154X_IGR_UNCHK",     MRV88E154X_IGR_UNCHK     },
    {"MRV88E154X_IGR_DELAY",     MRV88E154X_IGR_DELAY     },
    {"MRV88E154X_IGR_LATE",      MRV88E154X_IGR_LATE      },
    {"MRV88E154X_IGR_INVLD",     MRV88E154X_IGR_INVLD     },
    {"MRV88E154X_IGR_NOTVLD",    MRV88E154X_IGR_NOTVLD    },
    {"MRV88E154X_EGR_PKT_PORT",  MRV88E154X_EGR_PKT_PORT  },
    {"MRV88E154X_EGR_PKT_ENC",   MRV88E154X_EGR_PKT_ENC   },
    {"MRV88E154X_EGR_HIT",       MRV88E154X_EGR_HIT       },
    {"MRV88E154X_IGR_OCT_VAL",   MRV88E154X_IGR_OCT_VAL   },
    {"MRV88E154X_IGR_OCT_DEC",   MRV88E154X_IGR_OCT_DEC   },
    {"MRV88E154X_IGR_UNTAG",     MRV88E154X_IGR_UNTAG     },
    {"MRV88E154X_IGR_NOTAG",     MRV88E154X_IGR_NOTAG     },
    {"MRV88E154X_IGR_BADTAG",    MRV88E154X_IGR_BADTAG    },
    {"MRV88E154X_IGR_UNKSCI",    MRV88E154X_IGR_UNKSCI    },
    {"MRV88E154X_IGR_NOSCI",     MRV88E154X_IGR_NOSCI     },
    {"MRV88E154X_IGR_UNUSSA",    MRV88E154X_IGR_UNUSSA    },
    {"MRV88E154X_IGR_NOUSSA",    MRV88E154X_IGR_NOUSSA    },
    {"MRV88E154X_IGR_OCT_TOT",   MRV88E154X_IGR_OCT_TOT   },
    {"MRV88E154X_EGR_OCT_PORT",  MRV88E154X_EGR_OCT_PORT  },
    {"MRV88E154X_EGR_OCT_ENC",   MRV88E154X_EGR_OCT_ENC   },
    {"MRV88E154X_EGR_OCT_TOT",   MRV88E154X_EGR_OCT_TOT   },
    {"MRV88E154X_IGR_MISS",      MRV88E154X_IGR_MISS      },
    {"MRV88E154X_EGR_MISS",      MRV88E154X_EGR_MISS      },
    {"MRV88E154X_IGR_REDIR",     MRV88E154X_IGR_REDIR     },
    {"end",                      0                        },
};

#define NUM_PHY_PAGES (sizeof(marvell_88e1543_phy_reg_tbl) /      \
                             sizeof(struct mrvl_88e1543_phy_regs_t_))

/* ======== 88E1543 Test Mode Setting ========== */

/* Based on Marvell FAE,
 * Steps to enter 1543 PHY to Test Mode 1, 2 or 4 are:
 * 1. Write Page 0, Reg  9 = 0x1F00 (Set PHY to Master mode)
 * 2. Write Page 0, Reg  0 = 0x9140 (Soft-reset)
 * 3. Write Page 4, Reg 27 = 0x3E80 (Disable Clock on the HSDACP/N by set bit8 to 0)
 * 4. Write Page 6, Reg 26 = 0x8000 (Enable TX_TCLK)
 */
static mrvl_88e1543_phy_setup_t phy_testmode124_steps[] = {
    {MRV88E1543_REG_PAGE_0, MRV88E1543_1000B_CNTL_REG,  0x1F00, 0xFFFF},
    {MRV88E1543_REG_PAGE_0, MRV88E1543_CONTROL_REG,  0x9140, 0x7B40},
    {MRV88E1543_REG_PAGE_4, MRV_88E154X_P4_R27_QSGMII_CNTL_REG_2, 0x3E80, 0xFFFF},
    {MRV88E1543_REG_PAGE_6, MRV88E1543_P6_MISC_TEST, 0x8000, 0xFFA0}
};

/* Based on Marvell FAE,
 * Steps to enter 1543 PHY to Test Mode 3 are:
 * 1. Write Page 0, Reg  9 = 0x1700 (Set PHY to Slave mode)
 * 2. Write Page 0, Reg  0 = 0x9140 (Soft-reset)
 * 3. Write Page 4, Reg 27 = 0x3E80 (Disable Clock on the HSDACP/N by set bit8 to 0)
 * 4. Write Page 6, Reg 26 = 0x8000 (Enable TX_TCLK)
 */
static mrvl_88e1543_phy_setup_t phy_testmode3_steps[] = {
    {MRV88E1543_REG_PAGE_0, MRV88E1543_1000B_CNTL_REG,  0x1700, 0xFFFF},
    {MRV88E1543_REG_PAGE_0, MRV88E1543_CONTROL_REG,  0x9140, 0x7B40},
    {MRV88E1543_REG_PAGE_4, MRV_88E154X_P4_R27_QSGMII_CNTL_REG_2, 0x3E80, 0xFFFF},
    {MRV88E1543_REG_PAGE_6, MRV88E1543_P6_MISC_TEST, 0x8000, 0xFFA0}
};

/* 
 * Steps to enter 1543 PHY to 10M Pseudo-Random Test are:
 * 1. Write Page 0, Reg  16 = 0x0400 (Disable Auto-MDIX & Force copper link up)
 * 2. Write Page 0, Reg  0 = 0x8100 (Disable Auto-Neg. & Force speed to 10M)
 * 3. Write Page 6, Reg 16 = 0x0008 (Enable Packet Generator)
 */
static mrvl_88e1543_phy_setup_t phy_testmode5_steps[] = {
    {MRV88E1543_REG_PAGE_0, MRV88E1543_SPECIFIC_CONTROL1_REG,  0x0400, 0xFFFF},
    {MRV88E1543_REG_PAGE_0, MRV88E1543_CONTROL_REG,  0x8100, 0x7B40},
    {MRV88E1543_REG_PAGE_6, MRV88E1543_P6_COPPER_PKT_GEN, 0x0008, 0xFFFF}
};

/* 
 * Steps to enter 1543 PHY to 10M data 0/1 Test Mode are:
 * 1. Write Page 0, Reg  16 = 0x0400 (Disable Auto-MDIX & Force copper link up)
 * 2. Write Page 0, Reg  0 = 0x8100 (Disable Auto-Neg. & Force speed to 10M)
 * 3. Write Page 12, Reg 16 = 0x5044 (Enable Loopback of MDI to MDI)
 */
static mrvl_88e1543_phy_setup_t phy_testmode6_steps[] = {
    {MRV88E1543_REG_PAGE_0, MRV88E1543_SPECIFIC_CONTROL1_REG,  0x0400, 0xFFFF},
    {MRV88E1543_REG_PAGE_0, MRV88E1543_CONTROL_REG,  0x8100, 0x7B40},
    {0xC, 0x10, 0x40, 0xFFFF}
};

/* 
 * Steps to enter 1543 PHY to 100M Test Mode are:
 * 1. Write Page 0, Reg 16 = 0x0000 (Disable Auto-MDIX )
 * 2. Write Page 0, Reg  0 = 0xA100 (Disable Auto-Neg. & Force speed to 100M)
 */
static mrvl_88e1543_phy_setup_t phy_testmode7_steps[] = {
    {MRV88E1543_REG_PAGE_0, MRV88E1543_SPECIFIC_CONTROL1_REG,  0x0000, 0xFFFF},
    {MRV88E1543_REG_PAGE_0, MRV88E1543_CONTROL_REG,  0xA100, 0x7B40},
};

/*******************************************************************************
 *
 * Name: dev_88e1543_check_phy_address()
 *
 * Description: Check if phy addr is legal or not
 *
 * Input: *dev - pointer to the Marvell device.
 *        phy_addr - PHY address
 *
 * Returns: PASSED/FAILED
 *
 *******************************************************************************
 */
int dev_88e1543_check_phy_address (dev_object_t *dev, uint phy_addr)
{
    dev_88e1543_object_t *phy = (dev_88e1543_object_t *)dev;
    uint start_addr;
    uint addr_seq = phy->addr_seq;
    
    /* init the phy per port */
    if (addr_seq == MRV88E1543_PHY_ADDR_INCR) {
        start_addr = phy->start_phy_addr;
    } else {
        start_addr = phy->start_phy_addr - MRV88E1543_PORTS + 1;
    }
    
    if ((phy_addr < start_addr) || 
        (phy_addr >= (start_addr + MRV88E1543_PORTS))) {
        assert(!"PHY address is invalid");
        return (FAILED);
    }
    return (PASSED);
}

#if FULL_REG_TEST
/**********************************************************************
 *
 * Function: has_eee_cntr_reg
 *
 * This function checks whether the PHY has EEE Control Register
 *
 * Input : PHY addr
 *
 * Output: TRUE/FALSE
 *
 **********************************************************************
 */
static boolean has_eee_cntr_reg (dev_object_t *dev, uint phy_addr)
{
    uint val, model_num;
    dev_88e1543_object_t *phy = (dev_88e1543_object_t *)dev;

    SMIREAD(phy, phy_addr, MRV88E1543_REG_PAGE_0, MRV88E1543_PHY_ID2, &val);
    /* Get PHY model number */
    model_num = (val >> MRV88E1543_PHY_MODEL_NUM_SHIFT) & MRV88E1543_PHY_MODEL_MASK;

    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf(" %s(): phy_addr %#x, model number %#x\n",
                __FUNCTION__, phy_addr, model_num);
    }

    if (model_num == MRV88E1548L_PHY_MODEL_NUM) {
        if ((NVRAM)->diagflag & D_VERBOSE) {
            printf("Register test skip Page 18\n");
        }
        return (FALSE);
    }

    return (TRUE);

}
#endif

/*******************************************************************************
 *
 * Function: dev_88e1543_reg_show()
 *
 * This function prints the specific 88e1543 PHY register values
 *
 * Input: dev_object_t - pointer to the Marvell device
 *        dev_print - A device print function vector
 *        phy addr - PHY address
 *
 * Output: PASSED/FAILED
 *
 *******************************************************************************
 */
static int dev_88e1543_reg_show (dev_object_t *dev, print_fn_t dev_print, 
                                 uint phy_addr)
{
    uint val, ix;
    uint32_t page;
    const reg_info_t *reg_ptr;
    const mrvl_88e1543_phy_regs_t *page_reg_ptr = &marvell_88e1543_phy_reg_tbl[0];
    dev_88e1543_object_t *phy = (dev_88e1543_object_t *)dev;
   
    if (dev_88e1543_check_phy_address(dev, phy_addr) == FAILED) {
        sprintf(err_msg, "%s(): invalid phy addr (%d) found\n",
                __FUNCTION__, phy_addr);
        DEV_ERROR_REPORT(dev, err_msg, MRVL_88E1543_SHOW);
        return (FAILED);
    }

    dev_print("\nMarvell 88E1543 Base Address: %#.8x  phy_addr %#x\n", 
              (unsigned int)(long)phy->base.dev_addr, phy_addr);

    for (ix = 0; ix < NUM_PHY_PAGES; ix++) {
        dev_print("\n%s\n", page_reg_ptr->pagename);
        page = page_reg_ptr->pagenum;
        reg_ptr = page_reg_ptr->pageregs;
        while (reg_ptr->size.size != 0) {
            SMIREAD(phy, phy_addr, page, reg_ptr->offset, &val);
            dev_print("%-32s reg %.2d = %#.8x\n", reg_ptr->name, 
                                                  reg_ptr->offset, 
                                                  val);
            reg_ptr++;
        }
        page_reg_ptr++;
    }
 
    return (PASSED);
}

/*******************************************************************************
 *
 * Function: dev_88e1543_power_up
 *
 * This function power up/down Copper I/F on PHY device.
 * Both of the two power down bit(power down = 1, normal operation = 0) 
 * must be changed according to input parameter "enable".
 *  1. Page 0, reg 0, bit 11.
 *  2. Page 0, reg 16, bit 2.
 *
 * Input:  dev_object_t pointer to the Marvell GE device
 *         phy_addr - phy address
 *         enable: 0 - power down, 1 - power up.
 *
 * Output: PASSED/FAILED
 *
 *******************************************************************************
 */
static int dev_88e1543_power_up (dev_object_t *dev, uint phy_addr, uint enable)
{
    dev_88e1543_object_t *phy = (dev_88e1543_object_t *)dev;
    uint rc, reg_d;

    if (dev_88e1543_check_phy_address(dev, phy_addr) == FAILED) {
        sprintf(err_msg, "%s(): invalid phy addr (%d) found\n",
                __FUNCTION__, phy_addr);
        DEV_ERROR_REPORT(dev, err_msg, MRVL_88E1543_POWER_UP);
        return (FAILED);
    }

    /* 1. Set page 0, reg 0, bit 11. */
    rc = SMIREAD(phy, phy_addr, MRV88E1543_REG_PAGE_0, MRV88E1543_CONTROL_REG, 
                 &reg_d);
    if (rc != PASSED) {
        sprintf(err_msg, "%s(): phy smi read failed. phy_addr = "
                "#%x, page = %#x, reg = %#x, rc = %#x\n",__FUNCTION__,
                phy_addr, MRV88E1543_REG_PAGE_0, MRV88E1543_CONTROL_REG, rc);
        DEV_ERROR_REPORT(dev, err_msg, MRVL_88E1543_POWER_UP);
        return (FAILED);
    }

    if (enable == ENABLE) {
        if (reg_d & MRV88E1543_PWR_DOWN) { /* in power down mode */
            reg_d &= ~MRV88E1543_PWR_DOWN;
            rc = SMIWRITE(phy, phy_addr, MRV88E1543_REG_PAGE_0, 
                          MRV88E1543_CONTROL_REG, reg_d);
            if (rc != PASSED) {
                sprintf(err_msg, "%s(): phy smi write failed. "
                        "phy_addr = #%x, page = %#x, "
                        "reg = %#x, data = %#x, rc = %#x\n",
                        __FUNCTION__, phy_addr, MRV88E1543_REG_PAGE_0,
                        MRV88E1543_CONTROL_REG, reg_d, rc);
                DEV_ERROR_REPORT(dev, err_msg, MRVL_88E1543_POWER_UP);
                return (FAILED);
            }
        }
    } else {    /* DISABLE */
        if (!(reg_d & MRV88E1543_PWR_DOWN)) { /* not in power down mode */
            reg_d |= MRV88E1543_PWR_DOWN;
            rc = SMIWRITE(phy, phy_addr, MRV88E1543_REG_PAGE_0, 
                          MRV88E1543_CONTROL_REG, reg_d);
            if (rc != PASSED) {
                sprintf(err_msg, "%s(): phy smi write failed. "
                        "phy_addr = #%x, page = %#x, "
                        "reg = %#x, data = %#x, rc = %#x\n",
                        __FUNCTION__, phy_addr, MRV88E1543_REG_PAGE_0,
                        MRV88E1543_CONTROL_REG, reg_d, rc);
                DEV_ERROR_REPORT(dev, err_msg, MRVL_88E1543_POWER_UP);
                return (FAILED);
            }
        }        
    }

    /* 2. Set page 0, reg 16, bit 2. */
    rc = SMIREAD(phy, phy_addr, MRV88E1543_REG_PAGE_0, 
                 MRV88E1543_SPECIFIC_CONTROL1_REG, &reg_d);
    if (rc != PASSED) {
        sprintf(err_msg, "%s(): phy smi read failed. phy_addr = "
                "#%x, page = %#x, reg = %#x, rc = %#x\n",
                __FUNCTION__, phy_addr, MRV88E1543_REG_PAGE_0,
                MRV88E1543_SPECIFIC_CONTROL1_REG, rc);
        DEV_ERROR_REPORT(dev, err_msg, MRVL_88E1543_POWER_UP);
        return (FAILED);
    }

    if (enable == ENABLE) {
        if (reg_d & MRV88E1543_P0_R16_PWR_DOWN) { /* in power down mode */
            reg_d &= ~MRV88E1543_P0_R16_PWR_DOWN;
            rc = SMIWRITE(phy, phy_addr, MRV88E1543_REG_PAGE_0, 
                          MRV88E1543_SPECIFIC_CONTROL1_REG, reg_d);
            if (rc != PASSED) {
                sprintf(err_msg, "%s(): phy smi write failed. "
                        "phy_addr = #%x, page = %#x, "
                        "reg = %#x, data = %#x, rc = %#x\n",
                        __FUNCTION__, phy_addr, MRV88E1543_REG_PAGE_0,
                        MRV88E1543_SPECIFIC_CONTROL1_REG, reg_d, rc);
                DEV_ERROR_REPORT(dev, err_msg, MRVL_88E1543_POWER_UP);
                return (FAILED);
            }
        }
    } else {    /* DISABLE */
        if (!(reg_d & MRV88E1543_P0_R16_PWR_DOWN)) {/* not in power down mode */
            reg_d |= MRV88E1543_P0_R16_PWR_DOWN;
            rc = SMIWRITE(phy, phy_addr, MRV88E1543_REG_PAGE_0, 
                          MRV88E1543_SPECIFIC_CONTROL1_REG, reg_d);
            if (rc != PASSED) {
                sprintf(err_msg, "%s(): phy smi write failed. "
                        "phy_addr = #%x, page = %#x, "
                        "reg = %#x, data = %#x, rc = %#x\n",
                        __FUNCTION__, phy_addr, MRV88E1543_REG_PAGE_0,
                        MRV88E1543_SPECIFIC_CONTROL1_REG, reg_d, rc);
                DEV_ERROR_REPORT(dev, err_msg, MRVL_88E1543_POWER_UP);
                return (FAILED);
            }
        }
    }
    return (PASSED);
}

/*******************************************************************************
 *
 * Function: dev_88e1543_display_reg
 *
 * This function is a utility to display specific PHY register.
 *
 * Input: dev - Pointer to the Marvell GE device object
 *
 * Output: PASSED/FAILED
 *
 *******************************************************************************
 */
static int dev_88e1543_display_reg (dev_object_t *dev)
{
    uint phy_addr, page, reg, data, start_addr, end_addr;
    int retval=FAILED;
    dev_88e1543_object_t *phy = (dev_88e1543_object_t *)dev;
    start_addr = (uint)phy->start_phy_addr;
    end_addr = start_addr + MRV88E1543_PORTS -1;

    phy_addr = gethex_answer("Enter the phy address: ", start_addr, start_addr, end_addr);
    page = gethex_answer("Enter the page: ", MRV88E1543_REG_PAGE_0,
                                             MRV88E1543_REG_PAGE_0, 
                                             MRV88E1543_REG_PAGE_MAX);

    reg = gethex_answer("Enter the register offset: ", 0, 0, 0x20 - 1);

    /* display original value */
    retval = SMIREAD(phy, phy_addr, page, reg, &data);
    if (retval == PASSED) {
        printf("Original value: phy_addr %d, page %d, reg %d, data %#.8x\n",
               phy_addr, page, reg, data);
    } else {
        sprintf(err_msg, "%s(): Can not read back reg before "
                "alternation.\n", __FUNCTION__);
        DEV_ERROR_REPORT(dev, err_msg, MRVL_88E1543_ALTER_REG);
        return (FAILED);
    }
 
    return (retval);
}

/*******************************************************************************
 *
 * Function: dev_88e1543_alter_reg
 *
 * This function is a utility to alter specific PHY register.
 *
 * Input: dev - Pointer to the Marvell GE device object
 *
 * Output: PASSED/FAILED
 *
 *******************************************************************************
 */
static int dev_88e1543_alter_reg (dev_object_t *dev)
{
    uint phy_addr, page, reg, data, start_addr, end_addr;
    int retval=FAILED;
    dev_88e1543_object_t *phy = (dev_88e1543_object_t *)dev;
    start_addr = (uint)phy->start_phy_addr;
    end_addr = start_addr + MRV88E1543_PORTS -1;

    phy_addr = gethex_answer("Enter the phy address: ", start_addr, start_addr, end_addr);
    page = gethex_answer("Enter the page: ", MRV88E1543_REG_PAGE_0,
                                             MRV88E1543_REG_PAGE_0, 
                                             MRV88E1543_REG_PAGE_MAX);

    reg = gethex_answer("Enter the register offset: ", 0, 0, 0x20 - 1);

    /* display original value */
    retval = SMIREAD(phy, phy_addr, page, reg, &data);
    if (retval == PASSED) {
        printf("Original value: phy_addr %d, page %d, reg %d, data %#.8x\n",
               phy_addr, page, reg, data);
    } else {
        sprintf(err_msg, "%s(): Can not read back reg before "
                "alternation.\n", __FUNCTION__);
        DEV_ERROR_REPORT(dev, err_msg, MRVL_88E1543_ALTER_REG);
        return (FAILED);
    }

    /* alter register */
    data = gethex_answer("Enter the new data: ", 0, 0, 0xFFFF);

    retval = SMIWRITE(phy, phy_addr, page, reg, data);
    if (retval != PASSED) {
        sprintf(err_msg, "%s(): Alter PHY reg write smi failed.\n",
                __FUNCTION__);
        DEV_ERROR_REPORT(dev, err_msg, MRVL_88E1543_ALTER_REG);
        return (FAILED);
    }

    retval = SMIREAD(phy, phy_addr, page, reg, &data);
    if (retval == PASSED) {
        printf("Alter phy_addr %d, page %d, reg %d, data %#.8x\n",
               phy_addr, page, reg, data);
    } else {
        sprintf(err_msg, "%s(): Can not read back reg after "
                "alternation.\n", __FUNCTION__);
        DEV_ERROR_REPORT(dev, err_msg, MRVL_88E1543_ALTER_REG);
    }
    
    return (retval);
}


/*******************************************************************************
 *
 * Function: dev_88e1543_intr_disable
 *
 * This function clear PHY intr.
 *
 * Input: *dev     - Pointer to the Marvell GE device object
 *        phy_addr - MRV88E1543 PHY address
 *
 * Output: PASSED/FAILED
 *
 *******************************************************************************
 */
static int dev_88e1543_intr_disable (dev_object_t *dev, uint phy_addr)
{
    int retval=FAILED;
    dev_88e1543_object_t *phy = (dev_88e1543_object_t *)dev;

    if (dev_88e1543_check_phy_address(dev, phy_addr) == FAILED) {
        sprintf(err_msg, "%s(): invalid phy addr (%d) found\n",
                __FUNCTION__, phy_addr);
        DEV_ERROR_REPORT(dev, err_msg, MRVL_88E1543_INTR_DISABLE);
        return (FAILED);
    }

    if (dev_88e1543_enable_force_interrupt(phy, phy_addr, 
        DEV_88E154X_DISABLE) == FAILED) {
        sprintf(err_msg, "%s: Disable force interrupt failed\n", __func__);
        DEV_ERROR_REPORT(dev, err_msg, MRVL_88E1543_INTR_DISABLE);
        return (FAILED);
    }

    /* Write reg 0 register 18 to disable all the interrupt */
    retval = SMIWRITE(phy, phy_addr, MRV88E1543_REG_PAGE_0, 
                      MRV88E1543_INT_ENABLE_REG, MRV_88E154X_P0_R18_DISABLE_ALL_INT);


    return (retval);
}

/*******************************************************************************
 *
 * Function: dev_88e1543_intr_clr_fiber
 *
 * This function clear PHY intr.
 *
 * Input: iface - Pointer to the Marvell GE device object
 *
 * Output: PASSED/FAILED
 *
 *******************************************************************************
 */
static int dev_88e1543_intr_clr_fiber (dev_object_t *dev, uint phy_addr)
{
    dev_88e1543_object_t *phy = (dev_88e1543_object_t *)dev;

    buf_p = err_msg;
    
    if (dev_88e1543_check_phy_address(dev, phy_addr) == FAILED) {
        buf_p += sprintf(buf_p, "%s(): invalid phy addr (%d) found\n",
                                 __FUNCTION__, phy_addr);
        buf_p = err_msg;
        DEV_ERROR_REPORT(dev, buf_p, MRVL_88E1543_INTR_CLR_FIBER);
        return (FAILED);
    }

    if (dev_88e1543_enable_interrupt_fiber_inserted(phy, phy_addr, 
        DEV_88E154X_DISABLE) == FAILED) {
        buf_p += sprintf(buf_p, "%s: Disable force interrupt failed\n",
                         __func__);
        DEV_ERROR_REPORT(dev, err_msg, MRVL_88E1543_INTR_CLR_FIBER);
        return (FAILED);
    }

    return (PASSED);
}


/*******************************************************************************
 *
 * Function: dev_88e1543_intr_gen
 *
 * This function generate PHY intr to Host.
 *
 * Input: *dev     - Pointer to the Marvell GE device object
 *        phy_addr - MRV88E1543 PHY address
 *
 * Output: PASSED/FAILED
 *
 *******************************************************************************
 */
static int dev_88e1543_intr_gen (dev_object_t *dev, uint phy_addr)
{
    dev_88e1543_object_t *phy = (dev_88e1543_object_t *)dev;

    if (dev_88e1543_check_phy_address(dev, phy_addr) == FAILED) {
        sprintf(err_msg, "%s(): invalid phy addr (%d) found\n",
                __FUNCTION__, phy_addr);
        DEV_ERROR_REPORT(dev, err_msg, MRVL_88E1543_INTR_GEN);
        return (FAILED);
    }

    /* Use speed change event to generate PHY interrupt */
    /* 1. Enable Force interrupt , P3_R18 */
    if (dev_88e1543_enable_force_interrupt(phy, phy_addr, 
        DEV_88E154X_ENABLE) == FAILED) {
        sprintf(err_msg, "%s: Enable force interrupt failed\n", __func__);
        DEV_ERROR_REPORT(dev, err_msg, MRVL_88E1543_INTR_GEN);
        return (FAILED);
    }

    return (PASSED);
}

/*******************************************************************************
 *
 * Function: dev_88e1543_intr_gen_fiber
 *
 * This function generate PHY intr to Host for SFP module inserted.
 *
 * Input: iface - Pointer to the Marvell GE device object
 *
 * Output: PASSED/FAILED
 *
 *******************************************************************************
 */
static int dev_88e1543_intr_gen_fiber (dev_object_t *dev, uint phy_addr)
{
    dev_88e1543_object_t *phy = (dev_88e1543_object_t *)dev;

    buf_p = err_msg;
    
    if (dev_88e1543_check_phy_address(dev, phy_addr) == FAILED) {
        buf_p += sprintf(buf_p, "%s(): invalid phy addr (%d) found\n",
                                 __FUNCTION__, phy_addr);
        buf_p = err_msg;
        DEV_ERROR_REPORT(dev, buf_p, MRVL_88E1543_INTR_GEN_FIBER);
        return (FAILED);
    }

    /* Use speed change event to generate PHY interrupt */
    /* 1. Enable Force interrupt , P1_R18 */
    if (dev_88e1543_enable_interrupt_fiber_inserted(phy, phy_addr, 
        DEV_88E154X_ENABLE) == FAILED) {
        buf_p += sprintf(buf_p, "%s: Enable force interrupt failed\n",
                         __func__);
        DEV_ERROR_REPORT(dev, err_msg, MRVL_88E1543_INTR_GEN_FIBER);
        return (FAILED);
    }

    return (PASSED);
}


/*******************************************************************************
 *
 * Function: dev_88e1543_set_test_mode
 *
 * This function provides PHY test mode for Marvell GE PHYs.
 *
 * Input:  *dev -  pointer to the Marvell GE device
 *         phy_addr - phy address
 *
 * Output: PASSED/FAILED
 *
 *******************************************************************************
 */
static int dev_88e1543_set_test_mode (dev_object_t *dev, uint phy_addr)
{
    int test_mode = 0;;
    int ctr = 0, total_steps = 0;
    uint16_t testmode_val = 0;
    dev_88e1543_object_t *phy = (dev_88e1543_object_t *)dev;
    mrvl_88e1543_phy_setup_t *step_ptr;

    printf("PHY(Marvell 151x) Supported TestMode:\n");
    printf("[0] Normal Mode.\n");
    printf("[1] Transmit Waveform Test.\n");
    printf("[2] Transmit Jitter Test (Master).\n");
    printf("[3] Transmit Jitter Test (Slave).\n");
    printf("[4] Transmit Distortion Test.\n");
    printf("[5] 10M Pseudo-Random Test.\n");
    printf("[6] 10M Data 0/1 Test.\n");
    printf("[7] 100M Waveform Test. .\n");
    test_mode = gethex_answer("Enter Test mode (0-7)", 0, 0, 7);
    
    if ((test_mode == PHY_TESTMODE_1) || (test_mode == PHY_TESTMODE_2) ||
        (test_mode == PHY_TESTMODE_4)) {
        step_ptr = &phy_testmode124_steps[0];
        total_steps = sizeof(phy_testmode124_steps) / sizeof(mrvl_88e1543_phy_setup_t);

        /* 1. Enable Test mode 1: 0x3F00
         * 2. Enable Test mode 2: 0x5F00
         * 3. Enable Test mode 4: 0x9F00
         */
        if (test_mode == PHY_TESTMODE_1) {
            testmode_val = PHY_TESTMODE_1_REG_VAL;
        } else if (test_mode == PHY_TESTMODE_2) {
            testmode_val = PHY_TESTMODE_2_REG_VAL;
        } else if (test_mode == PHY_TESTMODE_4) {
            testmode_val = PHY_TESTMODE_4_REG_VAL;
        }
    } else if (test_mode == PHY_TESTMODE_3) {
        step_ptr = &phy_testmode3_steps[0];
        total_steps = sizeof(phy_testmode3_steps) / sizeof(mrvl_88e1543_phy_setup_t);

        /* Enable Test mode 3: 0x7700 */
        testmode_val = PHY_TESTMODE_3_REG_VAL;
    } else if (test_mode == PHY_TESTMODE_5) {
        step_ptr = &phy_testmode5_steps[0];
        total_steps = sizeof(phy_testmode5_steps) / sizeof(mrvl_88e1543_phy_setup_t);
    } else if (test_mode == PHY_TESTMODE_6) {
        step_ptr = &phy_testmode6_steps[0];
        total_steps = sizeof(phy_testmode6_steps) / sizeof(mrvl_88e1543_phy_setup_t);
    }  else if (test_mode == PHY_TESTMODE_7) {
        step_ptr = &phy_testmode7_steps[0];
        total_steps = sizeof(phy_testmode7_steps) / sizeof(mrvl_88e1543_phy_setup_t);
    } else if (test_mode == PHY_TESTMODE_NORMAL) {
        SMIWRITE(phy, phy_addr, MRV88E1543_REG_PAGE_0, MRV88E1543_CONTROL_REG,
                0x9140);
        return (PASSED);
    } else {
        printf("%s: Not support TestMode%d.\n", __FUNCTION__, test_mode);
        return (FAILED);
    }

    for (ctr = 0; ctr < total_steps; ctr++, step_ptr++) {
        /* Set register */
        printf("Set TestMode%d: Set page%d Reg%.2d to 0x%04X\n",
               test_mode, step_ptr->reg_page, step_ptr->reg_off, step_ptr->val);
        if (SMIWRITE(phy, phy_addr, step_ptr->reg_page, step_ptr->reg_off,
                     step_ptr->val) != PASSED) {
            printf("\n%s: Failed to set PHY(1543) page%d Reg%.2d to 0x%04X.\n",
                   __FUNCTION__, step_ptr->reg_page,
                   step_ptr->reg_off, step_ptr->val);
            return (FAILED);
        }
    }

    /* Test mode 5, 6 and 7, donot need to set */
    if ((test_mode == PHY_TESTMODE_5) || (test_mode == PHY_TESTMODE_6) ||
        (test_mode == PHY_TESTMODE_7)) {

    } else {
        /* Set Test mode by write page0 Reg 9 */
        /* Set register */
        printf("Set TestMode%d: Set page%d Reg%.2d to 0x%04X\n",
               test_mode, MRV88E1543_REG_PAGE_0, MRV88E1543_1000B_CNTL_REG, testmode_val);
        if (SMIWRITE(phy, phy_addr, MRV88E1543_REG_PAGE_0, MRV88E1543_1000B_CNTL_REG,
                     testmode_val) != PASSED) {
            printf("\n%s: Failed to set PHY(1543) page%d Reg%.2d to 0x%04X.\n",
                   __FUNCTION__, MRV88E1543_REG_PAGE_0, MRV88E1543_1000B_CNTL_REG, 
                   testmode_val);
            return (FAILED);
        }
    }

    printf("\nNow PHY(1543) enter TestMode%d, and press \'q\' to exit: ",
           test_mode);

    while (1) {
        if(getchar() == 'q') {
            SMIWRITE(phy, phy_addr, MRV88E1543_REG_PAGE_0, MRV88E1543_CONTROL_REG,
                     0x9140);
            break;
        }
    }
    return (PASSED);
}


/*******************************************************************************
 *
 * Function: dev_88e1548x_set_test_mode
 *
 * This function provides PHY test mode for Marvell GE PHYs.
 *
 * Input: iface - Pointer to the Marvell GE device object
 *
 * Output: PASSED/FAILED
 *
 *******************************************************************************
 */
static int dev_88e1548x_set_test_mode (dev_object_t *dev, int phy_addr,
                                       print_fn_t dev_print)
{
    uint rc;
    uint retval, reg_d, test_mode, val;
    dev_88e1543_object_t *phy = (dev_88e1543_object_t *)dev;

    buf_p = err_msg;
    
    if (dev_88e1543_check_phy_address(dev, phy_addr) == FAILED) {
        buf_p += sprintf(buf_p, "%s(): invalid phy addr (%d) found\n",
                                 __FUNCTION__, phy_addr);
        buf_p = err_msg;
        DEV_ERROR_REPORT(dev, buf_p, MRVL_88E1543_SET_TEST_MODE);
        return (FAILED);
    }

    retval = dev_88e1543_power_up(dev, phy_addr, ENABLE);
    if (retval != PASSED) {
        return (FAILED);
    }

    /* Got the current mode. */
    rc = SMIREAD(phy, phy_addr, MRV88E1543_REG_PAGE_0, 
                 MRV88E1543_1000B_CNTL_REG, &reg_d);

    if (rc != PASSED) {
        buf_p += sprintf(buf_p, "%s(): Unable to read test mode. "
                                "rc = %#x", __FUNCTION__, rc);
        buf_p = err_msg;
        DEV_ERROR_REPORT(dev, buf_p, MRVL_88E1543_SET_TEST_MODE);
        return (FAILED);
    }

    test_mode = (reg_d & PHY_GT_CTL_TEST_MASK) >> (PHY_GT_CTL_TEST_SHIFT);

    dev_print("\nTest modes - (Current test mode is %d.) \n", test_mode);
    dev_print("    0 - Normal Mode\n");
    dev_print("    1 - Test Mode 1 - Transmit Waveform Test\n");
    dev_print("    2 - Test Mode 2 - Transmit Jitter Test (Master mode)\n");
    dev_print("    3 - Test Mode 3 - Transmit Jitter Test (Slave mode)\n");
    dev_print("    4 - Test Mode 4 - Transmit Distortion Test\n");
    test_mode = gethex_answer("Enter the test mode: ", test_mode, 0, 
                               PHY_GT_CTL_TEST_MAX);

    /* Common setting for all test mode */
    
    SMIWRITE(phy, phy_addr, MRV88E1543_REG_PAGE_4,
             MRV_88E154X_P4_R27_QSGMII_CNTL_REG_2, 
             MRV_88E154X_P4_R27_DISABLE_CLOCK);


    SMIWRITE(phy, phy_addr, MRV88E1543_REG_PAGE_6, MRV88E1543_P6_MISC_TEST, 
             MRV_88E154X_P6_R26_ENABLE_TX_TCLK  );
    

    if ((test_mode == TRANSMIT_WAVEFORM_TEST) || 
        (test_mode == TRANSMIT_JITTER_TEST_MASTER_MODE) || 
        (test_mode == TRANSMIT_DISTORTION_TEST)) {
        SMIWRITE(phy, phy_addr, MRV88E1543_REG_PAGE_0, MRV88E1543_1000B_CNTL_REG,
                 MRV_88E154X_SET_PHY_TO_MASTER_MODE);
        SMIWRITE(phy, phy_addr, MRV88E1543_REG_PAGE_0, MRV88E1543_CONTROL_REG, 
                 MRV_88E154X_PHY_SOFT_RESET);
    } else if (test_mode == TRANSMIT_JITTER_TEST_SLAVE_MODE) {
        SMIWRITE(phy, phy_addr, MRV88E1543_REG_PAGE_0, MRV88E1543_1000B_CNTL_REG,
                 MRV_88E154X_SET_PHY_TO_SLAVE_MODE);
        SMIWRITE(phy, phy_addr, MRV88E1543_REG_PAGE_0, MRV88E1543_CONTROL_REG,
                 MRV_88E154X_PHY_SOFT_RESET);
    }

    switch (test_mode) {
        case 0:
            printf("Normal Mode\n");
            val = MRV_88E154X_NORMAL_MODE;
            break;
        case 1:
            printf("Test Mode 1 - Transmit Waveform Test\n");
            val = MRV_88E154X_TEST_MODE_1;
            break;
        case 2:
            printf("Test Mode 2 - Transmit Jitter Test(Master Mode)\n");
            val = MRV_88E154X_TEST_MODE_2;
            break;
        case 3:
            printf("Test Mode 3 - Transmit Jitter Test(Slave Mode)\n");
            val = MRV_88E154X_TEST_MODE_3;
            break;
        case 4:
            printf("Test Mode 4 - Transmit Distortion Test\n");
            val = MRV_88E154X_TEST_MODE_4;
            break;
        default :
            printf("Not support this test mode\n");
            return (FAILED);
    }
    /* Write the new data */
    rc = SMIWRITE(phy, phy_addr, MRV88E1543_REG_PAGE_0, 
                  MRV88E1543_1000B_CNTL_REG, val);  

    if (rc != PASSED) {
        buf_p += sprintf(buf_p, "%s(): Unable to write test mode. "
                                "rc = %#x", __FUNCTION__, rc);
        buf_p = err_msg;
        DEV_ERROR_REPORT(dev, buf_p, MRVL_88E1543_SET_TEST_MODE);
        return (FAILED);
    }

    return (PASSED);
}



/*******************************************************************************
 *
 * Function: dev_88e1543_copper_softreset().
 *
 * This function resets the phy and waits for its completion before returning.
 *
 * Input:  *dev -  pointer to the Marvell GE device
 *         phy_addr - phy address
 *
 * Output: PASSED/FAILED.
 *
 *******************************************************************************
 */
static int dev_88e1543_copper_softreset (dev_object_t *dev, uint phy_addr)
{
    dev_88e1543_object_t *phy = (dev_88e1543_object_t *)dev;
    int retval = FAILED, ix;
    uint data;

    /* do a soft reset and power up */
    retval = SMIREAD(phy, phy_addr, MRV88E1543_REG_PAGE_0, 
                     MRV88E1543_CONTROL_REG, &data);
    if (retval == PASSED) {
        data |= (MRV88E1543_COOPER_RST);
        retval = SMIWRITE(phy, phy_addr, MRV88E1543_REG_PAGE_0, 
                          MRV88E1543_CONTROL_REG, data);
        if (retval != PASSED) {
            sprintf(err_msg, "%s(): phy reset failed (phy addr = %d)\n",
                    __FUNCTION__, phy_addr);
            return (retval);
        }
    } else {
        sprintf(err_msg, "%s(): read phy control reg failed (phy addr %d)\n",
                __FUNCTION__, phy_addr);
        return (retval);
    }

    /* Check if phy reset complete */
    for (ix = 0; ix < COOPER_RST_POLLING_COUNT; ix++) {
        retval = SMIREAD(phy, phy_addr, MRV88E1543_REG_PAGE_0,
                         MRV88E1543_CONTROL_REG, &data);
        if (retval == PASSED) {
            if (!(data & MRV88E1543_COOPER_RST)) {
                /* Software reset is done. */
                if ((NVRAM)->diagflag & D_VERBOSE) {
                    printf("%s(): PHY soft reset ok at phy addr %d\n", 
                           __FUNCTION__, phy_addr);
                }
                break;
            }
        } else {
            sprintf(err_msg, "%s(): Failed to read back reset bit (phy addr = %d)\n",
                    __FUNCTION__, phy_addr);
            break;
        }
        /* Wait 10 mini-second in the polling loop */
        msleep(10);
    }

    if (ix == COOPER_RST_POLLING_COUNT) {
        sprintf(err_msg, "%s(): failed to get out of reset (phy addr = %d).\n",
                __FUNCTION__, phy_addr);
    }

    msleep(100);

    /* Add fix for slow link w/ short calbe(less than 50m) 
       after every software reset or power on */
    retval = SMIWRITE(phy, phy_addr, 0xFA,  25, 0x0);
    if (retval != PASSED) {
        sprintf(err_msg, "%s(): write phy slow link fix failed (phy addr = %d)\n",
                __FUNCTION__, phy_addr);
        return (retval);
    }

    return (retval);
}

/*******************************************************************************
 *
 * Function: dev_88e1543_phone_detect
 *
 * Description: Check if Marvell switch detects a Power Device (PD) at a
 *              given port.
 *
 * Input: *dev - Pointer to interface data structure.
 *        phy_addr - PHY address
 *
 * Outputs:  PASSED - Found a PD.
 *           FAILED - PD not detected or phy access error.
 *
 * Assumptions:
 *
 *******************************************************************************
 */
static int dev_88e1543_phone_detect (dev_object_t *dev, uint phy_addr)
{
    dev_88e1543_object_t *phy = (dev_88e1543_object_t *)dev;
    uint ix, rc, reg_d, detected;

    if (dev_88e1543_check_phy_address(dev, phy_addr) == FAILED) {
        sprintf(err_msg, "%s(): invalid phy addr (%d) found\n",
                __FUNCTION__, phy_addr);
        DEV_ERROR_REPORT(dev, err_msg, MRVL_88E1543_PHONE_DETECT);
        return (FAILED);
    }
    
    rc = dev_88e1543_power_up(dev, phy_addr, ENABLE);
    if (rc != PASSED) {
        return (FAILED);
    }
    
    /* Disable Auto Negotiation */
    rc = SMIREAD(phy, phy_addr, MRV88E1543_REG_PAGE_0, 
                 MRV88E1543_CONTROL_REG, &reg_d);
    if (rc != PASSED) {
        sprintf(err_msg, "%s(): phy smi read failed. phy_addr = "
                "#%x, page = %#x, reg = %#x, rc = %#x\n",
                __FUNCTION__, phy_addr, MRV88E1543_REG_PAGE_0,
                MRV88E1543_CONTROL_REG, rc);
        DEV_ERROR_REPORT(dev, err_msg, MRVL_88E1543_PHONE_DETECT);
        return (FAILED);
    }

    if ((reg_d & MRV88E1543_AUTO_NEO_ENA) == MRV88E1543_AUTO_NEO_ENA) {
        reg_d &= ~MRV88E1543_AUTO_NEO_ENA;
        rc = SMIWRITE(phy, phy_addr, MRV88E1543_REG_PAGE_0, 
                      MRV88E1543_CONTROL_REG, reg_d);
        if (rc != PASSED) {
            sprintf(err_msg, "%s(): phy smi write failed. "
                    "phy_addr = #%x, page = %#x, "
                    "reg = %#x, data = %#x, rc = %#x\n",
                    __FUNCTION__, phy_addr, MRV88E1543_REG_PAGE_0,
                    MRV88E1543_CONTROL_REG, reg_d, rc);
            DEV_ERROR_REPORT(dev, err_msg, MRVL_88E1543_PHONE_DETECT);     
            return (FAILED);
        }
        
        rc = dev_88e1543_copper_softreset(dev, phy_addr);
        if (rc != PASSED) {
            sprintf(err_msg, "%s(): phy addr %d reset failed. rc = %#x\n",
                    __FUNCTION__, phy_addr, rc);
            DEV_ERROR_REPORT(dev, err_msg, MRVL_88E1543_PHONE_DETECT);
            return (FAILED);
        }
    }

    /* Disable power over Ethernet detection */
    rc = SMIREAD(phy, phy_addr, MRV88E1543_REG_PAGE_0, 
                 MRV88E1543_SPECIFIC_CONTROL3_REG, &reg_d);
    if (rc != PASSED) {
        return (FAILED);
    }
    
    reg_d &= ~MRV88E1543_P0_R26_DTE_DETECT;
    rc = SMIWRITE(phy, phy_addr, MRV88E1543_REG_PAGE_0, 
                  MRV88E1543_SPECIFIC_CONTROL3_REG, reg_d);
    if (rc != PASSED) {
        sprintf(err_msg, "%s(): phy smi write failed. "
                "phy_addr = #%x, page = %#x, "
                "reg = %#x, data = %#x, rc = %#x\n",
                __FUNCTION__, phy_addr, MRV88E1543_REG_PAGE_0,
                MRV88E1543_SPECIFIC_CONTROL3_REG, reg_d, rc);
        DEV_ERROR_REPORT(dev, err_msg, MRVL_88E1543_PHONE_DETECT);     
        return (FAILED);
    }

    /* Set DTE power status drop to 5 seconds */
    rc = SMIREAD(phy, phy_addr, MRV88E1543_REG_PAGE_0, 
                 MRV88E1543_SPECIFIC_CONTROL3_REG, &reg_d);
    if (rc != PASSED) {
        sprintf(err_msg, "%s(): phy smi read failed. phy_addr = "
                "#%x, page = %#x, reg = %#x, rc = %#x\n",
                __FUNCTION__, phy_addr, MRV88E1543_REG_PAGE_0,
                MRV88E1543_SPECIFIC_CONTROL3_REG, rc);
        DEV_ERROR_REPORT(dev, err_msg, MRVL_88E1543_PHONE_DETECT);
        return (FAILED);
    }
    
    reg_d &= ~MRV88E1543_P0_R26_DTE_STATUS_DROP_MSK;
    reg_d |= MRV88E1543_P0_R26_DTE_STATUS_DROP_5S;
    rc = SMIWRITE(phy, phy_addr, MRV88E1543_REG_PAGE_0, 
                  MRV88E1543_SPECIFIC_CONTROL3_REG, reg_d);
    if (rc != PASSED) {
        sprintf(err_msg, "%s(): phy smi write failed. "
                "phy_addr = #%x, page = %#x, "
                "reg = %#x, data = %#x, rc = %#x\n",
                __FUNCTION__, phy_addr, MRV88E1543_REG_PAGE_0,
                MRV88E1543_SPECIFIC_CONTROL3_REG, reg_d, rc);
        DEV_ERROR_REPORT(dev, err_msg, MRVL_88E1543_PHONE_DETECT);  
        return (FAILED);
    }

    /* Enable power over Ethernet detection bit */
    rc = SMIREAD(phy, phy_addr, MRV88E1543_REG_PAGE_0, 
                 MRV88E1543_SPECIFIC_CONTROL3_REG, &reg_d);
    if (rc != PASSED) {
        sprintf(err_msg, "%s(): phy smi read failed. phy_addr = "
                "#%x, page = %#x, reg = %#x, rc = %#x\n",
                __FUNCTION__, phy_addr, MRV88E1543_REG_PAGE_0,
                MRV88E1543_SPECIFIC_CONTROL3_REG, rc);
        DEV_ERROR_REPORT(dev, err_msg, MRVL_88E1543_PHONE_DETECT);
        return (FAILED);
    }
    
    reg_d |= MRV88E1543_P0_R26_DTE_DETECT;
    rc = SMIWRITE(phy, phy_addr, MRV88E1543_REG_PAGE_0, 
                  MRV88E1543_SPECIFIC_CONTROL3_REG, reg_d);
    if (rc != PASSED) {
        sprintf(err_msg, "%s(): phy smi write failed. "
                "phy_addr = #%x, page = %#x, "
                "reg = %#x, data = %#x, rc = %#x\n",
                __FUNCTION__, phy_addr, MRV88E1543_REG_PAGE_0,
                MRV88E1543_SPECIFIC_CONTROL3_REG, reg_d, rc);
        DEV_ERROR_REPORT(dev, err_msg, MRVL_88E1543_PHONE_DETECT);  
        return (FAILED);
    }

    /* enable Auto Negotiation and reset phy */
    rc = SMIREAD(phy, phy_addr, MRV88E1543_REG_PAGE_0, 
                 MRV88E1543_CONTROL_REG, &reg_d);
    if (rc != PASSED) {
        sprintf(err_msg, "%s(): phy smi read failed. phy_addr = "
                "#%x, page = %#x, reg = %#x, rc = %#x\n",
                __FUNCTION__, phy_addr, MRV88E1543_REG_PAGE_0,
                MRV88E1543_CONTROL_REG, rc);
        DEV_ERROR_REPORT(dev, err_msg, MRVL_88E1543_PHONE_DETECT);
        return (FAILED);
    }
    reg_d |= MRV88E1543_AUTO_NEO_ENA;
    rc = SMIWRITE(phy, phy_addr, MRV88E1543_REG_PAGE_0, 
                  MRV88E1543_CONTROL_REG, reg_d);
    if (rc != PASSED) {
        sprintf(err_msg, "%s(): phy smi write failed. "
                "phy_addr = #%x, page = %#x, "
                "reg = %#x, data = %#x, rc = %#x\n",
                __FUNCTION__, phy_addr, MRV88E1543_REG_PAGE_0,
                MRV88E1543_CONTROL_REG, reg_d, rc);
        DEV_ERROR_REPORT(dev, err_msg, MRVL_88E1543_PHONE_DETECT);  
        return (FAILED);
    }
    
    rc = dev_88e1543_copper_softreset(dev, phy_addr);
    if (rc != PASSED) {
        sprintf(err_msg, "%s(): phy addr %d reset failed. rc = %#x\n",
                __FUNCTION__, phy_addr, rc);
        DEV_ERROR_REPORT(dev, err_msg, MRVL_88E1543_PHONE_DETECT);
        return (FAILED);
    }
    
    for (ix = MRVL_PHONE_DETECT_TIME; ix; ix--) {
        printf("\r%d seconds left", ix);
        msleep(1000);
    }

    /* read detection status register */
    rc = SMIREAD(phy, phy_addr, MRV88E1543_REG_PAGE_0, 
                 MRV88E1543_SPECIFIC_STATUS1_REG, &detected);
    if (rc != PASSED) {
        sprintf(err_msg, "%s(): phy smi read failed. phy_addr = "
                "#%x, page = %#x, reg = %#x, rc = %#x\n",
                __FUNCTION__, phy_addr, MRV88E1543_REG_PAGE_0,
                MRV88E1543_SPECIFIC_STATUS1_REG, rc);
        DEV_ERROR_REPORT(dev, err_msg, MRVL_88E1543_PHONE_DETECT);
        return (FAILED);
    }

    /* Disable power over Ethernet detection after detection */
    rc = SMIREAD(phy, phy_addr, MRV88E1543_REG_PAGE_0, 
                 MRV88E1543_SPECIFIC_CONTROL3_REG, &reg_d);
    if (rc != PASSED) {
        sprintf(err_msg, "%s(): phy smi read failed. phy_addr = "
                "#%x, page = %#x, reg = %#x, rc = %#x\n",
                __FUNCTION__, phy_addr, MRV88E1543_REG_PAGE_0,
                MRV88E1543_SPECIFIC_CONTROL3_REG, rc);
        DEV_ERROR_REPORT(dev, err_msg, MRVL_88E1543_PHONE_DETECT);
        return (FAILED);
    }
    
    reg_d &= ~MRV88E1543_P0_R26_DTE_DETECT;
    rc = SMIWRITE(phy, phy_addr, MRV88E1543_REG_PAGE_0, 
                  MRV88E1543_SPECIFIC_CONTROL3_REG, reg_d);
    if (rc != PASSED) {
        sprintf(err_msg, "%s(): phy smi write failed. "
                "phy_addr = #%x, page = %#x, "
                "reg = %#x, data = %#x, rc = %#x\n",
                __FUNCTION__, phy_addr, MRV88E1543_REG_PAGE_0,
                MRV88E1543_SPECIFIC_CONTROL3_REG, reg_d, rc);
        DEV_ERROR_REPORT(dev, err_msg, MRVL_88E1543_PHONE_DETECT); 
        return (FAILED);
    }

    /* Check detection result */
    if (detected & MRV88E1543_P0_R17_DTE_NEED_POWER) {
        return (PASSED);    /* Found Cisco PD */
    } else {
        return (FAILED);    /* PD not detected */
    }
}

/*******************************************************************************
 *
 * Function: dev_88e1543_cleanup_lpbk
 *
 * This function disable loopbacks and sets Marvell GE PHY
 * back into normal operating mode.
 *
 * Input:  dev - Pointer to the Marvell GE device object
 *         phy_addr  - PHY address
 *
 * Output: PASSED/FAILED
 *
 *******************************************************************************
 */
static int dev_88e1543_cleanup_lpbk (dev_object_t *dev, uint phy_addr)
{
    int retval, data, phy_speed, mac_speed;
    dev_88e1543_object_t *phy = (dev_88e1543_object_t *)dev;
    
    if (dev_88e1543_check_phy_address(dev, phy_addr) == FAILED) {
        sprintf(err_msg, "%s(): invalid phy addr (%d) found\n",
                __FUNCTION__, phy_addr);
        DEV_ERROR_REPORT(dev, err_msg, MRVL_88E1543_CLN_LPBK);
        return (FAILED);
    }

    /* errata restore page 250 reg 1 to 0x400 */ 
    data = 0x400;
    SMIWRITE(phy, phy_addr, MRV88E1543_REG_PAGE_250, 0x1, data);
    /* errata restore page 250 reg 7 to 0x200 */ 
    data = 0x200;
    SMIWRITE(phy, phy_addr, MRV88E1543_REG_PAGE_250, 0x7, data);
    /* errata force master page 0 reg 16 bit 10 to 1 */ 
    SMIREAD(phy, phy_addr, MRV88E1543_REG_PAGE_0,
            MRV88E1543_SPECIFIC_CONTROL1_REG, &data);
    data &= ~0x400;
    SMIWRITE(phy, phy_addr, MRV88E1543_REG_PAGE_0,
             MRV88E1543_SPECIFIC_CONTROL1_REG, data);

    phy_speed = MRV88E1543_SPD_SEL_1000M;
    mac_speed = MRV88E1543_MAC_SPD_1000M;

    /* restore PHY to 1Gbps, full duplex, auto-neg */
    /* Enable 1Gbps advertise (page 0, reg 9)*/
    retval = SMIREAD(phy, phy_addr, MRV88E1543_REG_PAGE_0, 
                     MRV88E1543_1000B_CNTL_REG, &data);
    if (retval == PASSED) {
        data |= MRV88E1543_1000BT_ADV;
        retval = SMIWRITE(phy, phy_addr, MRV88E1543_REG_PAGE_0, 
                          MRV88E1543_1000B_CNTL_REG, data);
        if (retval != PASSED) {
            sprintf(err_msg, "%s(): Enable phy copper 1Gbps "
                    "advertisement failed.\n", __FUNCTION__);
            DEV_ERROR_REPORT(dev, err_msg, MRVL_88E1543_CLN_LPBK);
            return (retval);
        }
    } else {
        sprintf(err_msg, "%s(): phy smi read failed. phy_addr = "
                "#%x, page = %#x, reg = %#x, rc = %#x\n",
                __FUNCTION__, phy_addr, MRV88E1543_REG_PAGE_0,
                MRV88E1543_1000B_CNTL_REG, retval);
        DEV_ERROR_REPORT(dev, err_msg, MRVL_88E1543_CLN_LPBK);
        return (retval);
    }

    /* Enable 10 & 100 Mbps advertise (page 0, reg 4)*/
    retval = SMIREAD(phy, phy_addr, MRV88E1543_REG_PAGE_0, 
                     MRV88E1543_AUTONEG_ADVR_REG, &data);
    if (retval == PASSED) {
        data |= MRV88E1543_100BT_ADV;
        data |= MRV88E1543_10BT_ADV;
        retval = SMIWRITE(phy, phy_addr, MRV88E1543_REG_PAGE_0, 
                          MRV88E1543_AUTONEG_ADVR_REG, data);
        if (retval != PASSED) {
            sprintf(err_msg, "%s(): Enable phy copper 10&100Mbps "
                    "advertisement failed.\n", __FUNCTION__);
            DEV_ERROR_REPORT(dev, err_msg, MRVL_88E1543_CLN_LPBK);
            return (retval);
        }
    } else {
        sprintf(err_msg, "%s(): phy smi read failed. phy_addr = "
                "#%x, page = %#x, reg = %#x, rc = %#x\n",
                __FUNCTION__, phy_addr, MRV88E1543_REG_PAGE_0,
                MRV88E1543_AUTONEG_ADVR_REG, retval);
        DEV_ERROR_REPORT(dev, err_msg, MRVL_88E1543_CLN_LPBK);
        return (retval);
    }

    /* config phy speed 1Gpbs for SGMII (page 2, reg 21) */
    retval = SMIREAD(phy, phy_addr, MRV88E1543_REG_PAGE_2, 
                     MRV88E1543_MAC_CNTL_REG2, &data);
    if (retval == PASSED) {
        data &= ~MRV88E1543_MAC_SPD_MASK;
        data |= mac_speed;
        retval = SMIWRITE(phy, phy_addr, MRV88E1543_REG_PAGE_2, 
                          MRV88E1543_MAC_CNTL_REG2, data);
        if (retval != PASSED) {
            sprintf(err_msg, "%s(): Restore phy mac speed 1Gbps "
                    "failed.\n", __FUNCTION__);
            DEV_ERROR_REPORT(dev, err_msg, MRVL_88E1543_CLN_LPBK);
            return (retval);
        }
    } else {
        sprintf(err_msg, "%s(): phy smi read failed. phy_addr = "
                "#%x, page = %#x, reg = %#x, rc = %#x\n",
                __FUNCTION__, phy_addr, MRV88E1543_REG_PAGE_2,
                MRV88E1543_MAC_CNTL_REG2, retval);
        DEV_ERROR_REPORT(dev, err_msg, MRVL_88E1543_CLN_LPBK);
        return (retval);
    }

    /* set speed 1Gbps & auto-neg & full-duplex of page 0, reg 0*/
    retval = SMIREAD(phy, phy_addr, MRV88E1543_REG_PAGE_0, 
                     MRV88E1543_CONTROL_REG, &data);
    if (retval == PASSED) {
        data &= ~MRV88E1543_SPD_SEL_MASK;
        data |= phy_speed | MRV88E1543_AUTO_NEO_ENA | MRV88E1543_FULL_DUPLEX;
        retval = SMIWRITE(phy, phy_addr, MRV88E1543_REG_PAGE_0, 
                          MRV88E1543_CONTROL_REG, data);
        if (retval != PASSED) {
            sprintf(err_msg, "%s(): set phy copper speed "
                    "failed.\n", __FUNCTION__);
            DEV_ERROR_REPORT(dev, err_msg, MRVL_88E1543_CLN_LPBK);
            return (retval);
        }
    } else {
        sprintf(err_msg, "%s(): phy smi read failed. phy_addr = "
                "#%x, page = %#x, reg = %#x, rc = %#x\n",
                __FUNCTION__, phy_addr, MRV88E1543_REG_PAGE_0,
                MRV88E1543_CONTROL_REG, retval);
        DEV_ERROR_REPORT(dev, err_msg, MRVL_88E1543_CLN_LPBK);
        return (retval);
    }

    /* clear 1000BT PHY External loopback mode */
    retval = SMIREAD(phy, phy_addr, MRV88E1543_REG_PAGE_6, 
                     MRV88E1543_P6_CHECKER_CTRL, &data);
    if (retval == PASSED) {
        data &= ~MRV88E1543_P6_R18_ENA_STUB_TEST;
        retval = SMIWRITE(phy, phy_addr, MRV88E1543_REG_PAGE_6,
                          MRV88E1543_P6_CHECKER_CTRL, data);
        if (retval != PASSED) {
            sprintf(err_msg, "%s(): Clear phy 1000BT external "
                    "failed.\n", __FUNCTION__);
            DEV_ERROR_REPORT(dev, err_msg, MRVL_88E1543_CLN_LPBK);
            return (retval); 
        }
    } else {
        sprintf(err_msg, "%s(): phy smi read failed. phy_addr = "
                "#%x, page = %#x, reg = %#x, rc = %#x\n",
                __FUNCTION__, phy_addr, MRV88E1543_REG_PAGE_6,
                MRV88E1543_P6_CHECKER_CTRL, retval);
        DEV_ERROR_REPORT(dev, err_msg, MRVL_88E1543_CLN_LPBK);
        return (retval);
    }

    /* clear PHY loopback mode (bit 14, page 0, reg 0) */
    retval = SMIREAD(phy, phy_addr, MRV88E1543_REG_PAGE_0,
                     MRV88E1543_CONTROL_REG, &data);
    if (retval == PASSED) {
        data &= ~MRV88E1543_LPBK_ENA;
        retval = SMIWRITE(phy, phy_addr, MRV88E1543_REG_PAGE_0, 
                          MRV88E1543_CONTROL_REG, data);
        if (retval != PASSED) {
            sprintf(err_msg, "%s(): clear phy loopback mode "
                                    "failed.\n", __FUNCTION__);
            DEV_ERROR_REPORT(dev, err_msg, MRVL_88E1543_CLN_LPBK);
            return (retval);
        }
    } else {
        sprintf(err_msg, "%s(): phy smi read failed. phy_addr = "
                "#%x, page = %#x, reg = %#x, rc = %#x\n",
                __FUNCTION__, phy_addr, MRV88E1543_REG_PAGE_0,
                MRV88E1543_CONTROL_REG, retval);
        DEV_ERROR_REPORT(dev, err_msg, MRVL_88E1543_CLN_LPBK);
        return (retval);
    }

    retval = dev_88e1543_copper_softreset(dev, phy_addr);
    if (retval != PASSED) {
        sprintf(err_msg, "%s(): phy addr %d reset failed. rc = %#x\n",
                __FUNCTION__, phy_addr, retval);
        DEV_ERROR_REPORT(dev, err_msg, MRVL_88E1543_CLN_LPBK);
        return (FAILED);
    }
    
    return (retval);
}

/*******************************************************************************
 *
 * Function: dev_88e1543_lpbk_mode
 *
 * This function set PHY loopback mode and speed.
 *
 * Input : *dev     - point to the Marvell GE device 
 *         phy_addr - MRV88E1543 PHY addr
 *         enable   - enable loopback mode 
 *        
 * Output: PASSED/FAILED
 *
 *******************************************************************************
 */
static int dev_88e1543_lpbk_mode (dev_object_t *dev, int phy_addr, int enable)
{
    int retval = PASSED, data;
    dev_88e1543_object_t *phy = (dev_88e1543_object_t *)dev;
    
    if (dev_88e1543_check_phy_address(dev, phy_addr) == FAILED) {
        sprintf(err_msg, "%s(): invalid phy addr (%d) found\n",
                __FUNCTION__, phy_addr);
        DEV_ERROR_REPORT(dev, err_msg, MRVL_88E1543_SET_LPBK);
        return (FAILED);
    }

    /* config PHY loopback mode (bit 14, page 0, reg 0) */
    retval = SMIREAD(phy, phy_addr, MRV88E1543_REG_PAGE_0,
                     MRV88E1543_CONTROL_REG, &data);
    if (retval == PASSED) {
        if (enable != ENABLE) {
            data &= ~MRV88E1543_LPBK_ENA;
        } else {
            data |= MRV88E1543_LPBK_ENA;
        }

        retval = SMIWRITE(phy, phy_addr, MRV88E1543_REG_PAGE_0, 
                          MRV88E1543_CONTROL_REG, data);
        if (retval != PASSED) {
            sprintf(err_msg, "%s(): set phy lpbk mode failed.\n",
                    __FUNCTION__);
            DEV_ERROR_REPORT(dev, err_msg, MRVL_88E1543_SET_LPBK);
            return (retval);
        }
    } else {
        sprintf(err_msg, "%s(): phy smi read failed. phy_addr = "
                "#%x, page = %#x, reg = %#x, rc = %#x\n",
                __FUNCTION__, phy_addr, MRV88E1543_REG_PAGE_0,
                MRV88E1543_CONTROL_REG, retval);
        DEV_ERROR_REPORT(dev, err_msg, MRVL_88E1543_SET_LPBK);
        return (retval);
    }

    return (retval);
}

/*******************************************************************************
 *
 * Function: dev_88e1543_set_lpbk
 *
 * This function set PHY loopback mode and speed.
 *
 * Input : *dev     - point to the Marvell GE device 
 *         phy_addr - MRV88E1543 PHY addr
 *         speed - 10/100/1G
 *         lpbk  - loopback mode
 *
 * Output: PASSED/FAILED
 *
 *******************************************************************************
 */
static int dev_88e1543_set_lpbk (dev_object_t *dev, int phy_addr, int speed, 
                                 int lpbk)
{
    int retval, link;
    dev_88e1543_object_t *phy = (dev_88e1543_object_t *)dev;
    int link_timeout, current_speed;
    int data;
    
    if (dev_88e1543_check_phy_address(dev, phy_addr) == FAILED) {
        sprintf(err_msg, "%s(): invalid phy addr (%d) found\n",
                __FUNCTION__, phy_addr);
        DEV_ERROR_REPORT(dev, err_msg, MRVL_88E1543_SET_LPBK);
        return (FAILED);
    }

    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("setup phy device, phy_addr %d, speed %d, loopback %d\n",
               phy_addr, speed, lpbk);
    }

    /* 
     * if loopback == external then
     *     advertise speed (10,100,1G)
     * else // Internal loopback // 
     *     configure PHY speed (P2_R21)
     *     configure speed (P0_R0)
     *
     * Reset PHY (P0_R0_B15)
     *
     * if loopback == external then
     *     disable loopback bit (P0_R0_B14)
     *     if speed == 1G then
     *         enable loopback stub
     *     Poll PHY link up and speed
     *     
     * else // Internal loopback //
     *     if speed == 1G then
     *         enable loopback stub
     *     enable loopback bit (P0_R0_B14)
     *
     * Check MAC Link up and speed
     */

    if (lpbk == SGMII_PHY_LPBK_EXTERNAL) {
        /* Advertise speed */
        if (dev_88e1543_advertise_speed(phy, phy_addr, speed) == FAILED) {
            sprintf(err_msg, "%s: Advertise speed failed\n", __func__);
            DEV_ERROR_REPORT(dev, err_msg, MRVL_88E1543_SET_LPBK);
            return (FAILED);
        }
    } else { /* Internal Loopback */
        /* Configure MAC speed (P2_R21) */
        if (dev_88e1543_config_mac_speed(phy, phy_addr, speed) == FAILED) {
            sprintf(err_msg, "%s: Configure MAC speed failed\n", __func__);
            DEV_ERROR_REPORT(dev, err_msg, MRVL_88E1543_SET_LPBK);
            return (FAILED);
        }
        
        /* Disable Auto-Negotiation */
        if (dev_88e1543_toggle_auto_nego(phy, phy_addr,
                                         DEV_88E154X_DISABLE) == FAILED) { 
            return (FAILED);
        }
        if (speed == ETH_MODE_GE) {
            /* errata force master page 0 reg 9 to 0x1f00*/ 
            SMIWRITE(phy, phy_addr, MRV88E1543_REG_PAGE_0,
                     MRV88E1543_1000B_CNTL_REG, 0x1f00);
        }

        /* Configure PHY speed (P0_R0) */
        if (dev_88e1543_config_phy_speed(phy, phy_addr, speed) == FAILED) {
            sprintf(err_msg,  "%s: Configure PHY speed failed\n", __func__);
            DEV_ERROR_REPORT(dev, err_msg, MRVL_88E1543_SET_LPBK);
            return (FAILED);
        }
    }

    /* Reset PHY */
    retval = dev_88e1543_copper_softreset(dev, phy_addr);
    if (retval != PASSED) {
        sprintf(err_msg, "%s(): phy addr %d reset failed. rc = %#x\n",
                __FUNCTION__, phy_addr, retval);
        DEV_ERROR_REPORT(dev, err_msg, MRVL_88E1543_SET_LPBK);
        return (FAILED);
    }

    if (lpbk == SGMII_PHY_LPBK_EXTERNAL) {
        /* Disable Loopback bit (P0_R0_B14) */
        if (dev_88e1543_toggle_loopback(phy, phy_addr,
                                        DEV_88E154X_DISABLE) == FAILED) {
            sprintf(err_msg, "%s: Disable PHY Loopback failed\n", __func__);
            DEV_ERROR_REPORT(dev, err_msg, MRVL_88E1543_SET_LPBK);
            return (FAILED);
        }

        if (speed == ETH_MODE_GE) {
            /* Enable loopback stub */
            if (dev_88e1543_toggle_loopback_stub(phy, phy_addr,
                                                 DEV_88E154X_ENABLE) == FAILED) {
                sprintf(err_msg, "%s: Enable PHY Loopback failed\n", __func__);
                DEV_ERROR_REPORT(dev, err_msg, MRVL_88E1543_SET_LPBK);
                return (FAILED);
            }
        }
        /* Poll PHY link up and speed */
        link_timeout = LINK_UP_TOUT;
        do {
            if (dev_88e1543_get_link_speed(phy, phy_addr, &link,
                                           &current_speed) == FAILED) {
                sprintf(err_msg, "%s: Get Link/Speed failed\n", __func__);
                DEV_ERROR_REPORT(dev, err_msg, MRVL_88E1543_SET_LPBK);
                return (FAILED);
            }

            if (link == LINK_UP) {
                break;
            }
            msleep(10);
        } while (link_timeout--);

        if (link == LINK_DOWN) {
            sprintf(err_msg, "%s: Link up timeout\n", __func__);
            DEV_ERROR_REPORT(dev, err_msg, MRVL_88E1543_SET_LPBK);
            return (FAILED);
        }

        /* Check whether current speed matches with tested speed */
        if (current_speed != speed) {
            sprintf(err_msg, "%s: Speed mismatch. Current speed (%d), "
                    "Expected speed (%d) \n", __func__, current_speed, speed);
            DEV_ERROR_REPORT(dev, err_msg, MRVL_88E1543_SET_LPBK);
            return (FAILED);
        }
    } else { /* Internal Loopback */
        if (speed == ETH_MODE_GE) {
            /* errata force master page 250 reg 1 to 0x0418 */ 
            SMIWRITE(phy, phy_addr, MRV88E1543_REG_PAGE_250, 1, 0x0418);
            /* errata force master page 250 reg 7 to 0x020C */ 
            SMIWRITE(phy, phy_addr, MRV88E1543_REG_PAGE_250, 7, 0x020C);
            /* Enable loopback stub */
            if (dev_88e1543_toggle_loopback_stub(phy, phy_addr,
                                                 DEV_88E154X_ENABLE) == FAILED) {
                sprintf(err_msg, "%s: Disable PHY Loopback failed\n", __func__);
                DEV_ERROR_REPORT(dev, err_msg, MRVL_88E1543_SET_LPBK);
                return (FAILED);
            }
        }

        /* errata force master page 0 reg 16 bit 10 to 1 */ 
        SMIREAD(phy, phy_addr, MRV88E1543_REG_PAGE_0,
                MRV88E1543_SPECIFIC_CONTROL1_REG, &data);
        data |= MRV88E1543_CNTL_FORCE_LINK;
        SMIWRITE(phy, phy_addr, MRV88E1543_REG_PAGE_0,
                 MRV88E1543_SPECIFIC_CONTROL1_REG, data);
        /* Enable loopback bit (P0_R0_B14) */
        if (dev_88e1543_toggle_loopback(phy, phy_addr, DEV_88E154X_ENABLE) == FAILED) {
            sprintf(err_msg, "%s: Enable PHY Loopback failed\n", __func__);
            DEV_ERROR_REPORT(dev, err_msg, MRVL_88E1543_SET_LPBK);
            return (FAILED);
        }
        /* errata wait 100ms*/
        msleep(100);
        
        /* Poll MAC link up and speed */
        link_timeout = LINK_UP_TOUT;
        do {
            if (dev_88e1543_get_fiber_link_speed(phy, phy_addr,
                                                 &link, &current_speed) == FAILED) {
                sprintf(err_msg, "%s: Get Link/Speed failed\n", __func__);
                DEV_ERROR_REPORT(dev, err_msg, MRVL_88E1543_SET_LPBK);
                return (FAILED);
            }
            
            if (link == LINK_UP) {
                break;
            }
            msleep(10);
        } while (link_timeout--);
        
        if (link == LINK_DOWN) {
            sprintf(err_msg, "%s: Link up timeout\n", __func__);
            DEV_ERROR_REPORT(dev, err_msg, MRVL_88E1543_SET_LPBK);
            return (FAILED);
        }
        
        /* Check whether current speed matches with tested speed */
        if (current_speed != speed) {
            sprintf(err_msg, "%s: Speed mismatch. Current speed (%d), "
                    "Expected speed (%d) \n", __FUNCTION__, current_speed, speed);
            DEV_ERROR_REPORT(dev, err_msg, MRVL_88E1543_SET_LPBK);
            return (FAILED);
        }
    }

    return (PASSED);
}


/*******************************************************************************
 *
 * Function: dev_88e1543_set_qsgmii_lpbk
 *
 * This function set PHY loopback mode and speed.
 *
 * Input: dev - Pointer to the Marvell GE device object
 *        phy_addr 
 *        speed - 100/1G
 *        lpbk  - loopback mode
 *
 * Output: PASSED/FAILED
 *
 *******************************************************************************
 */
static int dev_88e1543_set_qsgmii_int_lpbk (dev_object_t *dev, int phy_addr, int speed, 
                                 int lpbk)
{
    dev_88e1543_object_t *phy = (dev_88e1543_object_t *)dev;

    buf_p = err_msg;

    /* Reset phy mode (P18_R20_B15) */
    if (dev_88e1543_mode_software_reset(phy, phy_addr, SGMII_TO_QSGMII) == FAILED) {
        buf_p += sprintf(buf_p, "%s: Reset phy mode failed\n", __func__);
        DEV_ERROR_REPORT(dev, err_msg, MRVL_88E1543_SET_LPBK);
       return (FAILED);
    }

   /* Reset qsgmii_phy (P4_R0_B15) */
   if (dev_88e1543_qsgmii_software_reset(phy, phy_addr) == FAILED) {
       buf_p += sprintf(buf_p, "%s: Reset qsgmii phy failed\n", __func__);
       DEV_ERROR_REPORT(dev, err_msg, MRVL_88E1543_SET_LPBK);
       return (FAILED);
   }
   /* Enable loopback bit (P4_R0_B14) */
   if (dev_88e1543_qsgmii_toggle_loopback_and_speed(phy, phy_addr, DEV_88E154X_ENABLE, speed) == FAILED) {
       buf_p += sprintf(buf_p, "%s: Enable PHY Loopback failed\n", __func__);
       DEV_ERROR_REPORT(dev, err_msg, MRVL_88E1543_SET_LPBK);
       return (FAILED);
   }

   return(PASSED);


}


/*******************************************************************************
 *
 * Function: phy_register_tests
 *
 * For each register from reg_ptr, this function checks for accessibility
 * and does a ripple 1 and a ripple 0 test if applicable (not all registers
 * are W/R register).
 *
 * Input : *dev     - point to the Marvell GE device 
 *         phy_addr - MRV88E1543 PHY addr
 *         page     - MRV88E1543 register page
 *         *reg_ptr - MRV88E1543 register 
 *
 * Output: PASS/FAIL
 *
 *******************************************************************************
 */
static int phy_register_tests (dev_object_t *dev, int phy_addr, uint page, 
                               const reg_info_t *reg_ptr )
{
    uint32_t ix;
    uint retval, ret_val, save_val, readval;
    uint data, temp, tst_offset;
    dev_88e1543_object_t *phy = (dev_88e1543_object_t *)dev;

    readval = 0;
    retval = PASSED;
    ret_val = PASSED;
    
    while (reg_ptr->size.size != 0) {
        retval = SMIREAD(phy, phy_addr, page, reg_ptr->offset, &save_val);
        if (retval == FAILED) {
            sprintf(err_msg, "%s(): Error reading %s register "
                    "offset %#x, base_addr %#x, "
                    "phy_addr %d\n", __FUNCTION__, reg_ptr->name, reg_ptr->offset, 
                    (unsigned int)(long)phy->base.dev_addr, phy_addr);
            return (FAILED);
        }

        if (reg_ptr->type == READ_WRITE) {
            tst_offset = reg_ptr->offset;

            /* 
             * ripple 1 test
             */
            for (ix = 0; ix < (reg_ptr->size.size * 8); ix++) {
                temp = (1 << ix) & reg_ptr->mask;
                if (!temp) {
                    continue;
                }
    
                /* Write to register under test */
                retval = SMIWRITE(phy, phy_addr, page, tst_offset, temp);
                /* Read back */
                if (retval == PASSED) {
                    ret_val = SMIREAD(phy, phy_addr, page, tst_offset, 
                                      &readval);
                }
                if (((readval & reg_ptr->mask) != temp) ||
                    (retval == FAILED) || (ret_val == FAILED)) {
                    sprintf(err_msg, "%s(): Ripple one test "
                            "failed when accessing %s Register offset %#x, "
                            "phy_addr %d, base_addr %#x, Expect %#x, Read %#x",
                            __FUNCTION__, reg_ptr->name, tst_offset, phy_addr, 
                            (unsigned int)(long)phy->base.dev_addr, temp, readval);
                    return (FAILED);
                }
            }
    
            /* 
             * ripple 0 test
             */
            for (ix = 0; ix < (reg_ptr->size.size * 8); ix++) {
                temp = (1 << ix) & reg_ptr->mask;
                if (!temp) {
                    continue;
                }
                temp = (~(1 << ix)) & reg_ptr->mask;
                /* Write to register under test */
                retval = SMIWRITE(phy, phy_addr, page, tst_offset, temp);
                if (retval == PASSED) {
                    /* Read back */
                    ret_val = SMIREAD(phy, phy_addr, page, tst_offset, 
                                      &readval);
                }
                if (((readval & reg_ptr->mask) != temp) ||
                    (retval == FAILED) || (ret_val == FAILED)) {
                    sprintf(err_msg, "%s(): Ripple one test "
                            "failed when accessing %s Register offset %#x, "
                            "phy_addr %d, base_addr %#x, Expect %#x, Read %#x",
                            __FUNCTION__, reg_ptr->name, tst_offset, phy_addr, 
                            (unsigned int)(long)phy->base.dev_addr, temp, readval);
                    return (retval);
                }
            }
    
            /*
             * pattern test
             */
            data = PATTERN;
            for (ix = 0; ix < 2; ix++) {
                temp = data &reg_ptr->mask;
                /* Write to register under test */
                retval = SMIWRITE(phy, phy_addr, page, tst_offset, temp);
                if (retval == PASSED) {
                    /* Read back */
                    ret_val = SMIREAD(phy, phy_addr, page, tst_offset, 
                                      &readval);
                }
                if (((readval & reg_ptr->mask) != temp) ||
                    (retval == FAILED) || (ret_val == FAILED)) {
                    sprintf(err_msg, "%s(): Pattern test failed "
                            "when accessing %s Register offset %#x "
                            "phy_addr %d, base_addr %#x, Expect %#x, Read %#x",
                            __FUNCTION__, reg_ptr->name, tst_offset, phy_addr,
                            (unsigned int)(long)phy->base.dev_addr, temp, readval);
                    return (retval);
                }
    
                data = ~PATTERN; /* complement data pattern */
            }
            
            /*
             * restore original value
             */
            retval = SMIWRITE(phy, phy_addr, page, tst_offset, save_val);
            if (retval == FAILED) {
                sprintf(err_msg, "%s(): Error restoring %s register "
                        "offset %#x, base_addr %#x, phy_addr %d\n",
                        __FUNCTION__, reg_ptr->name, reg_ptr->offset, 
                        (unsigned int)(long)phy->base.dev_addr, phy_addr);
                return (FAILED);
            }
        }
        reg_ptr++;
    }

    return (PASSED);
}

/*******************************************************************************
 *
 * Function: dev_88e1543_reg_test_single.
 *
 * This function implements the PHY registers test of single phy_addr.
 *
 * Input:  dev_object_t  - pointer to the Marvell GE device.
 *         phy_addr      - MRV88E1543 PHY address.
 *
 * Output: PASSED/FAILED.
 *
 *******************************************************************************
 */
static int dev_88e1543_reg_test_single (dev_object_t *dev, uint phy_addr)
{
    int ret = PASSED;
    
    if (phy_register_tests(dev, phy_addr, MRV88E1543_REG_PAGE_3,
                           &marvell_88e1543_reg_page3[0]) == FAILED) {
        return (FAILED);
    }

    return (ret);
}

/*******************************************************************************
 *
 * Function: dev_88e1543_reg_test().
 *
 * This function implements the PHY registers test.
 *
 * Input:  dev_object_t pointer to the Marvell GE device.
 *         start_addr: 88e1543 phy port 0 addr
 *         order: 0(phy_addr incremental) , 1(phy_addr decremental)
 *                example: order 0, start_addr = 4; 4 phy_addr (4, 5, 6, 7)
 *                         order 1, start_addr = 4; 4 phy_addr (4, 3, 2, 1)
 * 
 * Output: PASSED/FAILED.
 *
 *******************************************************************************
 */
static int dev_88e1543_reg_test (dev_object_t *dev)
{
    uint phy_addr, phy_start;
    dev_88e1543_object_t *phy = (dev_88e1543_object_t *)dev;
    uint addr_seq = (uint)phy->addr_seq;
    uint all_link_stat = 0;
    
    if (addr_seq == MRV88E1543_PHY_ADDR_INCR) {
        phy_start = (uint)phy->start_phy_addr;
    } else {
        phy_start = (unsigned int)(long)phy->start_phy_addr - MRV88E1543_PORTS + 1;
    }

    for (phy_addr = phy_start; phy_addr < (phy_start + MRV88E1543_PORTS);
         phy_addr++) {
        if (all_port_link_status[phy_addr] == FALSE) {
            printf("PHY Port %d Register Test\n", phy_addr);
            if (dev_88e1543_reg_test_single(dev, phy_addr) == FAILED) {
                DEV_ERROR_REPORT(dev, err_msg, MRVL_88E1543_REG_TEST);
                return (FAILED);
            }
        } else {
            printf("Skip PHY Port %d Register Test\n", phy_addr);
        }

        all_link_stat = all_link_stat + all_port_link_status[phy_addr];
    }

    if (all_link_stat == MRV88E1543_PORTS) {
        printf("All PHY ports are plugged with download cable, skip all PHY Register Test\n");
        return (FAILED);
    }

    return (PASSED);
}

/*******************************************************************************
 * Name: dev_88e1543_show
 *
 * Description: Provide platforms with a mechanism to display some common
 *      device information via the device print function argument.
 *
 * Input: *dev - pointer to the Marvell GE device
 *        dev_print - A device print function vector
 *        cmd - A dev_show_cmd_e command
 *
 * Returns: PASSED/FAILED
 *
 * Assumptions: The device printf function vector has been provided by the host
 *              platform which implements the print logging functionality. The
 *              dev_attach() function has been called and successfully executed
 *
 *******************************************************************************
 */
static uint32_t dev_88e1543_show (dev_object_t *dev, print_fn_t dev_print, 
                                  dev_show_cmd cmd)
{
    dev_88e1543_object_t *phy = (dev_88e1543_object_t *)dev;
    uint phy_addr = (unsigned int)(long)phy->base.dev_addr;
    uint addr_seq = (uint)phy->addr_seq;
    uint phy_start;

    switch (cmd) {
    case DEV_SHOW_REGISTERS:
        if (addr_seq == MRV88E1543_PHY_ADDR_INCR) {
            phy_start = (uint)phy->start_phy_addr;
        } else {
            phy_start = (unsigned int)(long)phy->start_phy_addr - MRV88E1543_PORTS + 1;
        }
        for (phy_addr = phy_start; phy_addr < (phy_start + MRV88E1543_PORTS);
             phy_addr++) {
            dev_88e1543_reg_show(dev, dev_print, phy_addr);
        }
        break;
    default:
        assert(!"dev_88e1543_show");
    }
    return (PASSED);
}

/*******************************************************************************
 *
 * Name: dev_88e1543_init()
 *
 * Description: Initializes the Marvell 88E1543 (for each port)
 *              Refere to release notes 3.1 PHY initialization
 *
 * Input: dev_object_t pointer to the Marvell GE device.
 *
 * Returns: PASSED/FAILED
 *
 * Note:  PHY Initialization process
 *          Write page 0 Reg 4 = 0x01e1
 *          Write page 18 Reg 27 = 0x0000
 *          Write page 253 Reg 11 = 0x1d70
 *          Write page 0 Reg 16 = 0x3060
 *
 *******************************************************************************
 */
static uint32_t dev_88e1543_init (dev_object_t *dev)
{
    dev_88e1543_object_t *phy = (dev_88e1543_object_t *)dev;
    uint phy_addr, phy_start;
    uint addr_seq = (uint)phy->addr_seq;

    if (addr_seq == MRV88E1543_PHY_ADDR_INCR) {
        phy_start = (uint)phy->start_phy_addr;
    } else {
        phy_start = (unsigned int)(long)phy->start_phy_addr - MRV88E1543_PORTS + 1;
    }
    for (phy_addr = phy_start; phy_addr < (phy_start + MRV88E1543_PORTS); 
         phy_addr++) {
        SMIWRITE(phy, phy_addr, 0x0, 0x4, 0x01e1);
        SMIWRITE(phy, phy_addr, 0x12, 0x1b, 0x0000);
        SMIWRITE(phy, phy_addr, 0xfd, 0xb, 0x1d70);
        SMIWRITE(phy, phy_addr, 0x0, 0x10, 0x3060);

        /* Marvell recommends to do a software reset 
           at the end of init process */
        dev_88e1543_copper_softreset(dev, phy_addr);
    }

    for (phy_addr = phy_start; phy_addr < (phy_start + MRV88E1543_PORTS); 
         phy_addr++) {
        /* power up phy */
        if (MVL_88E1543_PWR_UP(phy, phy_addr, ENABLE) == FAILED) {
            cterr('f', 0, "Failed to power up PHY, phy_addr 0x%2x", phy_addr);
            return (FAILED);
        }
    }
    phy->base.dev_state = DEV_STATE_INIT;

    return (PASSED);
}

/*******************************************************************************
 *
 * Name: dev_88e1548l_init()
 *
 * Description: Initializes the Marvell 88E1548L (for each port)
 *              Refere to release notes  PHY initialization
 *
 * Input: dev_object_t pointer to the Marvell GE device.
 *
 * Returns: PASSED/FAILED
 *
 * Note:  PHY Initialization process
 *          Write Reg 17 = 0x2148
 *          Write Reg 16 = 0x2144
 *          Write Reg 17 = 0xDC0C
 *          Write Reg 16 = 0x2159
 *           
 *
 *******************************************************************************
 */
static uint32_t dev_88e1548l_init (dev_object_t *dev)
{
    dev_88e1543_object_t *phy = (dev_88e1543_object_t *)dev;
    uint phy_addr, phy_start;
    uint addr_seq = (uint)phy->addr_seq;

    if (addr_seq == MRV88E1543_PHY_ADDR_INCR) {
        phy_start = (uint)phy->start_phy_addr;
    } else {
        phy_start = (unsigned int)(long)phy->start_phy_addr - MRV88E1543_PORTS + 1;
    }
    for (phy_addr = phy_start; phy_addr < (phy_start + MRV88E1543_PORTS); 
         phy_addr++) {

        SMIWRITE(phy, phy_addr, 0xFF, 17, 0x2148);
        SMIWRITE(phy, phy_addr, 0xFF, 16, 0x2144);
        SMIWRITE(phy, phy_addr, 0xFF, 17, 0xDC0C);
        SMIWRITE(phy, phy_addr, 0xFF, 16, 0x2159);        

        /* Marvell recommends to do a software reset 
           at the end of init process */
        dev_88e1543_copper_softreset(dev, phy_addr);
    }

    phy->base.dev_state = DEV_STATE_INIT;

    return (PASSED);
}

/*******************************************************************************
 *
 * Function: dev_88e154x_macsec_reg_wr
 *
 * Description: macsec register write function which is based on 
 *              88E1548 register read/write
 * Input:  dev_object_t pointer to the Marvell GE device
 *         phy_addr - phy address
 *         addr - register addr
 *         offset - register offset
 *         data - the data we write into macsec register. 
 *
 * Output: NONE
 *
 *******************************************************************************
 */
static int dev_88e154x_macsec_reg_wr (dev_object_t *dev, int phy_addr, 
                                      ushort addr, ushort offset, uint data)
{
    ushort datahi, datalo, reg_addr;
    uint dummy;    

    datahi = ((data >> 16) & 0xFFFF);
    datalo = (data & 0xFFFF);
    reg_addr = addr + offset;

    SMIWRITE(dev, phy_addr, MRV88E154x_REG_PAGE_16, 
             MRV88E154X_P16_R1_LINK_CRYPT_WRITE_ADDR, reg_addr);
    SMIWRITE(dev, phy_addr, MRV88E154x_REG_PAGE_16, 
             MRV88E154X_P16_R2_LINK_CRYPT_DATA_LO, datalo);
    SMIWRITE(dev, phy_addr, MRV88E154x_REG_PAGE_16,
             MRV88E154X_P16_R3_LINK_CRYPT_DATA_HI, datahi);

    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("MACsec addr:wrval = %#.4x:%#.8x ,rd back\n", reg_addr, data);
    }

    dummy = dev_88e154x_macsec_reg_rd(dev, phy_addr, addr, offset);
    return (PASSED);
}

/********************************************************************************
 *
 * Function: dev_88e154x_macsec_reg_rd
 *
 * Description: macsec register read function which is based on 
 *              88E1548 register read/write
 *
 * Input:  dev_object_t pointer to the Marvell GE device
 *         phy_addr - phy address
 *         addr - register addr
 *         offset - register offset
 *
 * Output: rdval - read value of macsec reg
 ********************************************************************************
 */
static int dev_88e154x_macsec_reg_rd (dev_object_t *dev, int phy_addr, 
                                      ushort addr, ushort offset)
{
    uint datahi, datalo;
    ushort reg_addr;
    uint rdval;    

    reg_addr = addr + offset;
    SMIWRITE(dev, phy_addr, MRV88E154x_REG_PAGE_16,
             MRV88E154X_P16_R0_LINK_CRYPT_READ_ADDR, reg_addr);
    SMIREAD(dev, phy_addr, MRV88E154x_REG_PAGE_16, 
            MRV88E154X_P16_R2_LINK_CRYPT_DATA_LO, &datalo);
    SMIREAD(dev, phy_addr, MRV88E154x_REG_PAGE_16,
            MRV88E154X_P16_R3_LINK_CRYPT_DATA_HI, &datahi);
    
    rdval = datalo;
    rdval |= (datahi << 16);    
    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("MACsec addr:rdval = %#.4x:%#.8x\n", reg_addr, rdval);
    }

    return (rdval);
}

/********************************************************************************
 *
 * Function:  dev_88e154x_clr_macsec_cnt
 *
 * Description: cleanup macsec counter(statistic) via reading them.
 *
 * Input:  dev_object_t pointer to the Marvell GE device
 *         phy_addr - phy address
 *         port - setup port
 *         
 * Output: NONE
 *
 ********************************************************************************
 */
static int dev_88e154x_clr_macsec_cnt (dev_object_t *dev, int phy_addr, int port) 
{
    uint offset, ia, dummy;

    offset = (MRV88E154X_MACSEC_CNT_OFFSET * port);
    for (ia = 0; ia < MRV88E154X_MACSEC_CNT_OFFSET; ia++) {
        dummy = dev_88e154x_macsec_reg_rd(dev, phy_addr, 
                                         (ia + MRV88E154X_IGR_HIT), offset);       
    }

    return (PASSED);
}

/********************************************************************************
 *
 * Function: dev_88e154x_set_macsec
 *   from Marvell Eng: setup internal loopback need to disable macsec 
 *
 * Input: dev_object_t pointer to the Marvell GE device
 *        phy_addr - phy address
 *        onoff - turn on/off
 *
 * Output: PASSED/FAILED
 *
 ********************************************************************************
 */
static int dev_88e154x_set_macsec(dev_object_t *dev, int phy_addr, boolean onoff)
{
    uint rdval, wrval;      

    SMIREAD(dev, phy_addr, MRV88E1543_REG_PAGE_18, 
            MRV88E154X_P18_R27_GEN_CON_REG2, &rdval);

    if (onoff) {
        wrval = rdval | MRV88E154X_P18_R27_ENABLE_MACSEC;  /*restore*/
    } else {
        wrval = rdval & ~MRV88E154X_P18_R27_ENABLE_MACSEC;  /*disable*/
    }
    SMIWRITE(dev, phy_addr, MRV88E1543_REG_PAGE_18, 
             MRV88E154X_P18_R27_GEN_CON_REG2, wrval);

    return PASSED;
}

/*******************************************************************************
 *
 * Function: dev_88e154x_init_macsec
 *
 * Description: we configure macsec related setting on its mem/reg.
 *
 * Input:  dev_object_t pointer to the Marvell GE device
 *         phy_addr - phy address
 *         port - phy port
 *
 * Output: NONE
 *
 * NOTE: the data we write into macsec registers are based on 
 * Marvell FAE suggestion. 
 * 0x0000 - 0x07FF Port0 registers/memory 
 * 0x0800 - 0x0FFF Port1 registers/memory 
 * 0x1000 - 0x17FF Port2 registers/memory 
 * 0x1800 - 0x1FFF Port3 registers/memory 
 *******************************************************************************
 */
static int dev_88e154x_init_macsec (dev_object_t *dev, int phy_addr, int port) 
{
    uint offset;
    
    offset = (MRV88E154X_GENERAL_PORT_OFFSET * port);
    
    /* disable drop_bad_tag */
    dev_88e154x_macsec_reg_wr(dev, phy_addr, MRV88E154X_CRYPT_IGR_GEN,
                              offset, 0xFB40000);
  
    /* set VFL to only check no drop */
    dev_88e154x_macsec_reg_wr(dev, phy_addr, MRV88E154X_CRYPT_ISC_GEN,
                              offset, 0x2);

    /* egress default match, encrypt+auth */
    dev_88e154x_macsec_reg_wr(dev, phy_addr, MRV88E154X_ELU_TBL_OFF4,
                              offset, 0x40060000); 
    
    /* ingress decrypt+auth and VFL setting*/  
    dev_88e154x_macsec_reg_wr(dev, phy_addr, MRV88E154X_ILU_TBL_OFF6, 
                              offset, 0x220000); 
 
    /* igr default match */  
    dev_88e154x_macsec_reg_wr(dev, phy_addr, MRV88E154X_ILU_TBL_OFF7, 
                              offset, 0x4000);   
   
    /* set sci[31:0] and sci [63:32] */
    dev_88e154x_macsec_reg_wr(dev, phy_addr, MRV88E154X_EGR_CTXT_OFF0, 
                              offset, 0xF00DBEEF);  
    dev_88e154x_macsec_reg_wr(dev, phy_addr, MRV88E154X_EGR_CTXT_OFF1,
                              offset, 0xCAFEFEED);  
    
    /* set tci[7:0] */
    dev_88e154x_macsec_reg_wr(dev, phy_addr, MRV88E154X_EGR_CTXT_OFF3, 
                              offset, 0x2C);  
   
    /* set encrypt key */
    dev_88e154x_macsec_reg_wr(dev, phy_addr, MRV88E154X_ENC_KEY_OFF0,
                              offset, 0x102);   
    dev_88e154x_macsec_reg_wr(dev, phy_addr, MRV88E154X_ENC_KEY_OFF1, 
                              offset, 0x3040500);
    dev_88e154x_macsec_reg_wr(dev, phy_addr, MRV88E154X_ENC_KEY_OFF2, 
                              offset, 0x0);
    dev_88e154x_macsec_reg_wr(dev, phy_addr, MRV88E154X_ENC_KEY_OFF3,
                              offset, 0x0);


    /* set egres hash key */
    dev_88e154x_macsec_reg_wr(dev, phy_addr, MRV88E154X_EGR_HKEY_OFF0, 
                              offset, 0x102);  
    dev_88e154x_macsec_reg_wr(dev, phy_addr, MRV88E154X_EGR_HKEY_OFF1, 
                              offset, 0x3040500);
    dev_88e154x_macsec_reg_wr(dev, phy_addr, MRV88E154X_EGR_HKEY_OFF2, 
                              offset, 0x0);
    dev_88e154x_macsec_reg_wr(dev, phy_addr, MRV88E154X_EGR_HKEY_OFF3,
                              offset, 0x0);
    
    /*  set decrypt key */
    dev_88e154x_macsec_reg_wr(dev, phy_addr, MRV88E154X_DEC_KEY_OFF0,
                              offset, 0x102);
    dev_88e154x_macsec_reg_wr(dev, phy_addr, MRV88E154X_DEC_KEY_OFF1, 
                              offset, 0x3040500);
    dev_88e154x_macsec_reg_wr(dev, phy_addr, MRV88E154X_DEC_KEY_OFF2,
                              offset, 0x0);
    dev_88e154x_macsec_reg_wr(dev, phy_addr, MRV88E154X_DEC_KEY_OFF3,
                              offset, 0x0);    
    
    /*  set ingress hash key */
    dev_88e154x_macsec_reg_wr(dev, phy_addr, MRV88E154X_IGR_HKEY_OFF0,
                              offset, 0x102);  
    dev_88e154x_macsec_reg_wr(dev, phy_addr, MRV88E154X_IGR_HKEY_OFF1, 
                              offset, 0x3040500);
    dev_88e154x_macsec_reg_wr(dev, phy_addr, MRV88E154X_IGR_HKEY_OFF2, 
                              offset, 0x0);
    dev_88e154x_macsec_reg_wr(dev, phy_addr, MRV88E154X_IGR_HKEY_OFF3,
                              offset, 0x0);
 
    return (PASSED);
}

/********************************************************************************
 *
 * Function: dev_88e154x_check_status
 *
 * Description: read macsec counter to ensure packets are encrypted, 
 *              decrypted and authenticated.
 *
 * Input: dev_object_t pointer to the Marvell GE device
 *        phy_addr - phy address
 *        port - phy port
 *        dev_print - A device print function vector
 *
 * Output: PASS/FAILED
 *
 ********************************************************************************
 */
static int dev_88e154x_check_status (dev_object_t *dev, int phy_addr, 
                                     int port, print_fn_t dev_print) 
{
    uint offset, result_igrok, result_igrmiss;

    offset = (MRV88E154X_MACSEC_CNT_OFFSET * port);

    /* check IGR_OK and IGR_MISS */
    result_igrok =   dev_88e154x_macsec_reg_rd(dev, phy_addr, 
                                               MRV88E154X_IGR_OK, offset);
    result_igrmiss = dev_88e154x_macsec_reg_rd(dev ,phy_addr,
                                               MRV88E154X_IGR_MISS, offset);

    /* IGR is not OK, packet is not decrypted or authenticated */
    if ((!result_igrok) || (result_igrmiss)) {
        dev_print("Packets decrypted/authenticated OK num0x%x\n", result_igrok);
        dev_print("Packets igress missed num0x%x\n", result_igrmiss);
        dev_print("Packets validation failed. Detailed refer to Dump statistic Utility\n");  
        return (FAILED);
    } else {
        return (PASSED);
    }
}

/********************************************************************************
 *
 * Function: dev_88e154x_dump_statistic
 *
 * Description: dumping macsec statictic
 *
 * Input: dev_object_t pointer to the Marvell GE device
 *        phy_addr - phy address
 *        port - phy port
 *        dev_print - A device print function vector
 *
 * Output: PASS/FAILED
 *
 *******************************************************************************
 */
static void dev_88e154x_dump_statistic (dev_object_t *dev, int phy_addr, 
                                        int port, print_fn_t dev_print) 
{
    ushort offset; 
    uint retval;  
    const mrvl_88e1543_macsec_regs_info_t *reg_ptr;
    
    offset = (MRV88E154X_MACSEC_CNT_OFFSET * port);
    dev_print("##Dumpping MACsec counter..\n");
    dev_print("##IGR_OK and IGR_MISS will be read/clean during testing\n");
    
    reg_ptr = &marvell_88e1543_macsec_reg_tbl[0];

    while (reg_ptr->offset != 0) {
        retval = dev_88e154x_macsec_reg_rd(dev, phy_addr,
                                           reg_ptr->offset, offset);
        dev_print("%s(0x%x) = %d\n", reg_ptr->name, reg_ptr->offset, retval);
        reg_ptr++;
    }

}

/*******************************************************************************
 *
 * Name: dev_88e1543_enable_force_interrupt()
 *
 * Description: Enable/Disable Force Interrupt
 *
 * Input: phy - Pointer to the Marvell GE device object
 *        phy_addr - PHY Address
 *        which_int - Which Interrupt
 *        enable - Enable/Disable
 *
 * Returns: PASSED/FAILED
 *
 *******************************************************************************
 */
static uint32_t dev_88e1543_enable_force_interrupt (dev_88e1543_object_t *phy, 
                                                    int phy_addr, int enable)
{
    int data, bit_mask;
    if (SMIREAD(phy, phy_addr, MRV88E1543_REG_PAGE_3,
                MRV88E1543_TMR_CONTROL_REG, &data) == FAILED) { 
        return (FAILED);
    }

    bit_mask = PHY_TIMER_CNTRL_FORCE_INT;

    if (enable == DEV_88E154X_ENABLE) {
        data |= bit_mask;
    } else {
        data &= ~bit_mask;
    }

    return (SMIWRITE(phy, phy_addr, MRV88E1543_REG_PAGE_3, 
                     MRV88E1543_TMR_CONTROL_REG, data));
}

/*******************************************************************************
 *
 * Name: dev_88e1543_enable_interrupt_fiber_inserted()
 *
 * Description: Enable/Disable Force Interrupt for SFP module inserted
 *
 * Input: phy - Pointer to the Marvell GE device object
 *        phy_addr - PHY Address
 *        which_int - Which Interrupt
 *        enable - Enable/Disable
 *
 * Returns: PASSED/FAILED
 *
 *******************************************************************************
 */
static uint32_t dev_88e1543_enable_interrupt_fiber_inserted (dev_88e1543_object_t *phy, 
                                                             int phy_addr,
                                                             int enable)
{
    int data, bit_mask;
    if (SMIREAD(phy, phy_addr, MRV88E1543_REG_PAGE_1, MRV88E1543_P1_INTR_EN_REG,
        &data) == FAILED) { 
        return (FAILED);
    }

    bit_mask = MRV88E1543_P1_AUTO_NEG_INTR_EN;

    if (enable == DEV_88E154X_ENABLE) {
        data |= bit_mask;
    } else {
        data &= ~bit_mask;
    }

    return (SMIWRITE(phy, phy_addr, MRV88E1543_REG_PAGE_1, 
                     MRV88E1543_P1_INTR_EN_REG, data));
}

/*******************************************************************************
 *
 * Name: dev_88e1543_get_fiber_link_speed()
 *
 * Description: This function returns Fiber link status and current speed
 *
 * Input: phy - Pointer to the Marvell GE device object
 *        phy_addr - PHY Address
 *        link - Pointer to the link status buffer
 *        speed - Pointer to the current speed buffer
 *
 * Returns: PASSED/FAILED
 *
 *******************************************************************************
 */
static uint32_t dev_88e1543_get_fiber_link_speed (dev_88e1543_object_t *phy, 
                                                 int phy_addr, int *link, int *speed)
{   
    int data;

    if (SMIREAD(phy, phy_addr, MRV88E1543_REG_PAGE_1,
                MRV88E1543_SPECIFIC_STATUS1_REG, &data) == FAILED) { 
        return (FAILED);
    }

    if (data & MRV88E1543_LINK_UP) {
        *link = LINK_UP;
    } else {
        *link = LINK_DOWN;
    }

    if ((data & MRV88E1543_LINK_SPEED_MASK) == MRV88E1543_LINK_SPEED_1000) {
        *speed = ETH_MODE_GE;
    } else if ((data & MRV88E1543_LINK_SPEED_MASK) == MRV88E1543_LINK_SPEED_100) {
        *speed = ETH_MODE_FE100;
    } else {
        *speed = ETH_MODE_FE10;
    }

    return (PASSED);
}

/*******************************************************************************
 *
 * Name: dev_88e1543_get_link_speed()
 *
 * Description: This function returns the link status and current speed
 *
 * Input: phy - Pointer to the Marvell GE device object
 *        phy_addr - PHY Address
 *        link - Pointer to the link status buffer
 *        speed - Pointer to the current speed buffer
 *
 * Returns: PASSED/FAILED
 *
 *******************************************************************************
 */
static uint32_t dev_88e1543_get_link_speed (dev_88e1543_object_t *phy, 
                                            int phy_addr, int *link, int *speed)
{   
    int data;

    if (SMIREAD(phy, phy_addr, MRV88E1543_REG_PAGE_0,
                MRV88E1543_SPECIFIC_STATUS1_REG, &data) == FAILED) { 
        return (FAILED);
    }

    if (data & MRV88E1543_LINK_UP) {
        *link = LINK_UP;
    } else {
        *link = LINK_DOWN;
    }

    if ((data & MRV88E1543_LINK_SPEED_MASK) == MRV88E1543_LINK_SPEED_1000) {
        *speed = ETH_MODE_GE;
    } else if ((data & MRV88E1543_LINK_SPEED_MASK) == MRV88E1543_LINK_SPEED_100) {
        *speed = ETH_MODE_FE100;
    } else {
        *speed = ETH_MODE_FE10;
    }

    return (PASSED);
}

/*******************************************************************************
 *
 * Name: dev_88e1543_toggle_auto_nego()
 *
 * Description: Toggle Auto-Negotiation bit for loopback test
 *
 * Input: phy - Pointer to the Marvell GE device object
 *        phy_addr - PHY Address
 *        enable - DEV_88E154X_ENABLE/DEV_88E154X_DISABLE
 *
 * Returns: PASSED/FAILED
 *
 *******************************************************************************
 */
static uint32_t dev_88e1543_toggle_auto_nego (dev_88e1543_object_t *phy, 
                                              int phy_addr, int enable)
{
    int data;

    if (SMIREAD(phy, phy_addr, MRV88E1543_REG_PAGE_0,
                MRV88E1543_CONTROL_REG, &data) == FAILED) { 
        return (FAILED);
    }

    if (enable == DEV_88E154X_ENABLE) {
        data |= MRV88E1543_AUTO_NEO_ENA;
    } else {
        data &= ~MRV88E1543_AUTO_NEO_ENA;
    }

    return (SMIWRITE(phy, phy_addr, MRV88E1543_REG_PAGE_0, 
                     MRV88E1543_CONTROL_REG, data));
}

/*******************************************************************************
 *
 * Name: dev_88e1543_toggle_loopback_stub()
 *
 * Description: Toggle loopback stub bit for 1G External loopback test
 *
 * Input: phy - Pointer to the Marvell GE device object
 *        phy_addr - PHY Address
 *        enable - DEV_88E154X_ENABLE/DEV_88E154X_DISABLE
 *
 * Returns: PASSED/FAILED
 *
 *******************************************************************************
 */
static uint32_t dev_88e1543_toggle_loopback_stub (dev_88e1543_object_t *phy, 
                                                  int phy_addr, int enable)
{
    int data;

    if (SMIREAD(phy, phy_addr, MRV88E1543_REG_PAGE_6,
                MRV88E1543_P6_CHECKER_CTRL, &data) == FAILED) { 
        return (FAILED);
    }

    if (enable == DEV_88E154X_ENABLE) {
        data |= MRV88E1543_P6_R18_ENA_STUB_TEST;
    } else {
        data &= ~MRV88E1543_P6_R18_ENA_STUB_TEST;
    }

    return (SMIWRITE(phy, phy_addr, MRV88E1543_REG_PAGE_6, 
                     MRV88E1543_P6_CHECKER_CTRL, data));
}


/*******************************************************************************
 *
 * Name: dev_88e1543_toggle_loopback()
 *
 * Description: Toggle loopback bit in P0_R0_Bit14 for internal/external
 *              loopback
 *
 * Input: phy - Pointer to the Marvell GE device object
 *        phy_addr - PHY Address
 *        enable - DEV_88E154X_ENABLE/DEV_88E154X_DISABLE
 *
 * Returns: PASSED/FAILED
 *
 *******************************************************************************
 */
static uint32_t dev_88e1543_toggle_loopback (dev_88e1543_object_t *phy, 
                                             int phy_addr, int enable)
{
    int data;

    if (SMIREAD(phy, phy_addr, MRV88E1543_REG_PAGE_0,
                MRV88E1543_CONTROL_REG, &data) == FAILED) { 
        return (FAILED);
    }

    if (enable == DEV_88E154X_ENABLE) {
        data |= MRV88E1543_LPBK_ENA;
    } else {
        data &= ~MRV88E1543_LPBK_ENA;
    }

    return (SMIWRITE(phy, phy_addr, MRV88E1543_REG_PAGE_0, 
                     MRV88E1543_CONTROL_REG, data));
}

/*******************************************************************************
 *
 * Name: dev_88e1543_qsgmii_toggle_loopback()
 *
 * Description: Toggle loopback bit in P4_R0_Bit14 for internal/external
 *              loopback
 *
 * Input: phy - Pointer to the Marvell GE device object
 *        phy_addr - PHY Address
 *        enable - DEV_88E154X_ENABLE/DEV_88E154X_DISABLE
 *
 * Returns: PASSED/FAILED
 *
 *******************************************************************************
 */
static uint32_t dev_88e1543_qsgmii_toggle_loopback_and_speed (dev_88e1543_object_t *phy, 
                                                    int phy_addr, int enable, int speed)
{
    int data;

    if (SMIREAD(phy, phy_addr, MRV88E1543_REG_PAGE_4, MRV88E1543_CONTROL_REG, 
                &data) == FAILED) { 
        return (FAILED);
    }

    if (enable == DEV_88E154X_ENABLE) {
        if (speed == ETH_MODE_GE)
            data = MRV88E1543_LPBK_ENA | MRV88E1543_FULL_DUPLEX 
                    | MRV88E1543_SPD_SEL_1000M;
        if (speed == ETH_MODE_FE100)
            data = MRV88E1543_LPBK_ENA | MRV88E1543_FULL_DUPLEX
                    | MRV88E1543_SPD_SEL_100M;
    } else {
        data &= ~MRV88E1543_LPBK_ENA;
    }

    return (SMIWRITE(phy, phy_addr, MRV88E1543_REG_PAGE_4, 
                     MRV88E1543_CONTROL_REG, data));
}
/*******************************************************************************
 *
 * Name: dev_88e1543_config_phy_speed()
 *
 * Description: Configure PHY speed 
 *
 * Input: phy - Pointer to the Marvell GE device object
 *        phy_addr - PHY Address
 *        speed - 10/100/1G
 *
 * Returns: PASSED/FAILED
 *
 *******************************************************************************
 */
static uint32_t dev_88e1543_config_phy_speed (dev_88e1543_object_t *phy, 
                                              int phy_addr, int speed)
{
    int data;

    if (SMIREAD(phy, phy_addr, MRV88E1543_REG_PAGE_0,
                MRV88E1543_CONTROL_REG, &data) == FAILED) { 
        return (FAILED);
    }

    data &= ~MRV88E1543_SPD_SEL_MASK;
    
    switch (speed) {
    case ETH_MODE_GE:
        data |= MRV88E1543_SPD_SEL_1000M;
        break;
    case ETH_MODE_FE100:
        data |= MRV88E1543_SPD_SEL_100M;
        break;
    case ETH_MODE_FE10:
        data |= MRV88E1543_SPD_SEL_10M;
        break;
    default:
        return (FAILED);
    }

    return (SMIWRITE(phy, phy_addr, MRV88E1543_REG_PAGE_0, 
                     MRV88E1543_CONTROL_REG, data));
}


/*******************************************************************************
 *
 * Name: dev_88e1543_config_mac_speed()
 *
 * Description: Configure MAC speed 
 *
 * Input: phy - Pointer to the Marvell GE device object
 *        phy_addr - PHY Address
 *        speed - 10/100/1G
 *
 * Returns: PASSED/FAILED
 *
 *******************************************************************************
 */
static uint32_t dev_88e1543_config_mac_speed (dev_88e1543_object_t *phy, 
                                             int phy_addr, int speed)
{
    int data;

    if (SMIREAD(phy, phy_addr, MRV88E1543_REG_PAGE_2,
                MRV88E1543_MAC_CNTL_REG2, &data) == FAILED) { 
        return (FAILED);
    }

    data &= ~MRV88E1543_MAC_SPD_MASK;
    
    switch (speed) {
    case ETH_MODE_GE:
        data |= MRV88E1543_MAC_SPD_1000M;
        break;
    case ETH_MODE_FE100:
        data |= MRV88E1543_MAC_SPD_100M;
        break;
    case ETH_MODE_FE10:
        data |= MRV88E1543_MAC_SPD_10M;
        break;
    default:
        return (FAILED);
    }

    return (SMIWRITE(phy, phy_addr, MRV88E1543_REG_PAGE_2, 
                     MRV88E1543_MAC_CNTL_REG2, data));
}


/*******************************************************************************
 *
 * Name: dev_88e1543_advertise_speed()
 *
 * Description: Advertise speed to link partner
 *
 * Input: phy - Pointer to the Marvell GE device object
 *        phy_addr - PHY Address
 *        speed - 10/100/1G
 *
 * Returns: PASSED/FAILED
 *
 *******************************************************************************
 */
static uint32_t dev_88e1543_advertise_speed (dev_88e1543_object_t *phy, 
                                             int phy_addr, int speed)
{
    int retval;
    int data_p0r9, data_p0r4;

    if (SMIREAD(phy, phy_addr, MRV88E1543_REG_PAGE_0,
                MRV88E1543_1000B_CNTL_REG, &data_p0r9) == FAILED ||
        SMIREAD(phy, phy_addr, MRV88E1543_REG_PAGE_0,
                MRV88E1543_AUTONEG_ADVR_REG, &data_p0r4) == FAILED) {
        return (FAILED);
    }

    switch (speed) {
    case ETH_MODE_GE:
        data_p0r9 |= MRV88E1543_1000BT_ADV;
        return (SMIWRITE(phy, phy_addr, MRV88E1543_REG_PAGE_0,
                         MRV88E1543_1000B_CNTL_REG, data_p0r9));
        break;
    case ETH_MODE_FE100:
    case ETH_MODE_FE10:
        /* Disable 1Gbps advertise */
        data_p0r9 &= ~MRV88E1543_1000BT_ADV;

        if (speed == ETH_MODE_FE100) {
            data_p0r4 |= MRV88E1543_100BT_ADV;
        } else {
            data_p0r4 &= ~MRV88E1543_100BT_ADV;
            data_p0r4 |= MRV88E1543_10BT_ADV;
        }

        retval = SMIWRITE(phy, phy_addr, MRV88E1543_REG_PAGE_0,
                          MRV88E1543_1000B_CNTL_REG, data_p0r9);

        retval |= SMIWRITE(phy, phy_addr, MRV88E1543_REG_PAGE_0, 
                           MRV88E1543_AUTONEG_ADVR_REG, data_p0r4);
        break;
    default:
        retval = FAILED;
    }
    return (retval);
}


/*******************************************************************************
 *
 * Name: dev_88e1543_detach()
 *
 * Description: detach the device specific functions from the caller.
 *      All of the device specific function are connected to the
 *      dev_do_nothing() function, except for the dev_attach()
 *      function. Also, the dev_state must be assigned the value
 *      of DEV_STATE_DETACH.
 *
 *      Since, some platforms may want to detach the device, but not
 *      release the memory resources (via a free () in the
 *      dev_destroy()), this function can be executed to accomplish
 *      this task. However, before a detached device can be used again,
 *      it must be re-attached (via the dev_attach()).
 *
 * Input: *dev - Pointer to the Marvell GE device object
 *
 * Returns: PASSED/FAILED
 *
 *******************************************************************************
 */
static uint32_t dev_88e1543_detach (dev_object_t *dev)
{
    dev_88e1543_object_t *phy = (dev_88e1543_object_t *) dev;

    /* Init the device object structure to default "do nothing" */
    init_default_dev_object(dev, phy->base.dev_object_fvt);

    phy->base.dev_state = DEV_STATE_DETACH;

    return (PASSED);
}

/*------------------------------------------------------------------
 *
 * Function: dev_88e1543_mode_software_reset
 *   Reset QSGMII by Software reset (P4_R0_B15)
 *
 * Input:  dev_object_t pointer to the Marvell device
 *         phy_addr - phy address
 *         mode     - SGMII to QSGMII
 *                    QSGMII to AUTO Detect
 *
 *
 * Output: PASSED/FAILED
 *
 *------------------------------------------------------------------
 */

static uint32_t dev_88e1543_mode_software_reset (dev_88e1543_object_t *phy, 
                                                 int phy_addr, int mode)
{
    int data;

    if (SMIREAD(phy, phy_addr, MRV88E1543_REG_PAGE_18, 20 , 
                &data) == FAILED) { 
        return (FAILED);
    }
    
    data &= ~MRV88E1543_MODE_QSGMII_MASK;    
    
    if (mode == SGMII_TO_QSGMII) {
        data |= MRV88E1543_MODE_SGMII_TO_QSGMII;
    }

    if (mode == QSGMII_TO_AUTO_DETECT) {
        data |= MRV88E1543_MODE_QSGMII_TO_AUTO_DETECT;
    }
    
    data |= MRV88E1543_MODE_RESET;

    return (SMIWRITE(phy, phy_addr, MRV88E1543_REG_PAGE_18, 20, data));
}
/*------------------------------------------------------------------
 *
 * Function: dev_88e1543_qsgmii_software_reset
 *   Reset QSGMII by Software reset (P4_R0_B15)
 *
 * Input:  dev_object_t pointer to the Marvell device
 *         phy_addr - phy address
 *
 * Output: PASSED/FAILED
 *
 *------------------------------------------------------------------
 */

static uint32_t dev_88e1543_qsgmii_software_reset (dev_88e1543_object_t *phy, 
                                                 int phy_addr)
{
    int data;

    if (SMIREAD(phy, phy_addr, MRV88E1543_REG_PAGE_4, MRV88E1543_REG_PAGE_0, 
                &data) == FAILED) { 
        return (FAILED);
    }

    data |= MRV88E1543_MODE_RESET;

    return (SMIWRITE(phy, phy_addr, MRV88E1543_REG_PAGE_4, MRV88E1543_REG_PAGE_0, data));
}

/*******************************************************************************
 *
 * Name: dev_88e1543_attach()
 *
 * Description: Attach the Marvell GE device for use. This
 *              function will initialize and setup all necessary pointers
 *              and bring the chip to operation.
 *
 * Input: *dev - Pointer to the Marvell GE device object
 *
 * Returns: PASSED/FAILED
 *
 *******************************************************************************
 */
static uint32_t dev_88e1543_attach (dev_object_t *dev)
{
    dev_88e1543_object_t *phy = (dev_88e1543_object_t *)dev;

    if (phy->callin_fvt == NULL) {
        DEV_ERROR_REPORT(dev, "dev_88e1543_attach() callin malloc", 
                         MRVL_88E1543_ATTACH);
        return (FAILED);
    }

    if (phy->callout_fvt == NULL) {
        DEV_ERROR_REPORT(dev, "dev_88e1543_attach() callout malloc", 
                         MRVL_88E1543_ATTACH);
        return (FAILED);
    }

    /* init the call in function */
    phy->callin_fvt->register_test = dev_88e1543_reg_test;
    phy->callin_fvt->set_loopback = dev_88e1543_set_lpbk;
    phy->callin_fvt->set_qsgmii_int_loopback = dev_88e1543_set_qsgmii_int_lpbk;
    phy->callin_fvt->lpbk_mode = dev_88e1543_lpbk_mode;
    phy->callin_fvt->cleanup_loopback = dev_88e1543_cleanup_lpbk;
    phy->callin_fvt->power_up = dev_88e1543_power_up;
    phy->callin_fvt->intr_gen = dev_88e1543_intr_gen;
    phy->callin_fvt->intr_gen_fiber = dev_88e1543_intr_gen_fiber;
    phy->callin_fvt->intr_disable = dev_88e1543_intr_disable;
    phy->callin_fvt->intr_clr_fiber = dev_88e1543_intr_clr_fiber;
    phy->callin_fvt->phy_88e1548l_init = dev_88e1548l_init; 

    phy->callin_fvt->gephy_set_led_on = dev_88e1543_led_on; 
    phy->callin_fvt->gephy_set_led_off = dev_88e1543_led_off; 
    phy->callin_fvt->gephy_set_led_default = dev_88e1543_led_default;
    phy->callin_fvt->gephy_check_if_plugged_with_cable = dev_88e1543_check_if_plugged_with_download_cable;

    /* PHY utilities */
    phy->callin_fvt->show_reg = dev_88e1543_reg_show;
    phy->callin_fvt->alter_reg = dev_88e1543_alter_reg;
    phy->callin_fvt->phone_detect = dev_88e1543_phone_detect;
    phy->callin_fvt->set_test_mode = dev_88e1543_set_test_mode;
    phy->callin_fvt->display_reg = dev_88e1543_display_reg;
    phy->callin_fvt->set_1548x_test_mode = dev_88e1548x_set_test_mode;
    phy->callin_fvt->clr_macsec_cnt = dev_88e154x_clr_macsec_cnt;
    phy->callin_fvt->set_macsec = dev_88e154x_set_macsec;
    phy->callin_fvt->init_macsec = dev_88e154x_init_macsec;
    phy->callin_fvt->check_status = dev_88e154x_check_status;
    phy->callin_fvt->dump_statistic = dev_88e154x_dump_statistic;

    phy->base.dev_state = DEV_STATE_ATTACH;

    return (PASSED);
}

/******************************************************************************
 * Name: dev_88e1543_destroy
 *
 * Description:	Destroy the dev_object structure and free all the resources.
 *
 * Input: *dev_object_t pointer to the 88E1543device
 *
 * Returns:	none
 *
 * Assumptions:	The dev_attch() function has been called and successfully
 *
 *****************************************************************************/
static void dev_88e1543_destroy (dev_object_t **dev)
{
    dev_88e1543_object_t *obj_88e1543;

    if (dev == NULL) {
        return;
    }

    if (*dev == NULL) {
        return;
    }

    obj_88e1543 = (dev_88e1543_object_t *)*dev;

    if (obj_88e1543->callout_fvt) {
        free(obj_88e1543->callout_fvt);	/* Free callout struct */
    }

    if (obj_88e1543->callin_fvt) {
        free(obj_88e1543->callin_fvt);		/* Free callin struct */
    }

    free(obj_88e1543->base.dev_object_fvt);	/* Free dev_object_t */
}

/*******************************************************************************
 *
 * Name: dev_88e1543_create()
 *
 * Description: Create object with various device function
 * point to "do nothing"
 *
 * Input: dev_object_t pointer to the Marvell device.
 *        error reporting function pointer.
 *
 * Returns: PASSED/FAILED
 *
 *
 *******************************************************************************
 */
void dev_88e1543_create (dev_object_t *dev, dev_error_report_t error_report_fn)
{
    dev_object_fvt_t	*m88e1543_fvt;
    dev_88e1543_object_t *phy = (dev_88e1543_object_t *)dev;
    
    /* Allocate memory for the device object */
    if ((m88e1543_fvt = (dev_object_fvt_t *)malloc(sizeof(dev_object_fvt_t))) == NULL) {
        /* Unable to allocate memory */
        error_report_fn(dev, "malloc failure in 88e1543_dev_create()", 0);
        printf("%s: NULL\n", __func__);
	    return;
    }

    /* Init the device object structure to default "do nothing" */
    init_default_dev_object(dev, m88e1543_fvt);

    phy->base.dev_object_fvt->dev_attach = dev_88e1543_attach;
    phy->base.dev_object_fvt->dev_detach = dev_88e1543_detach;
    phy->base.dev_object_fvt->dev_init = dev_88e1543_init;
    phy->base.dev_object_fvt->dev_show = dev_88e1543_show;
    phy->base.dev_object_fvt->dev_error_report = error_report_fn;
    phy->base.dev_object_fvt->dev_destroy = dev_88e1543_destroy;
    phy->base.dev_object_fvt->dev_name = "Marvell GE PHY 88E1543";

    phy->callin_fvt = (dev_88e1543_callin_fvt_t *)malloc(sizeof(dev_88e1543_callin_fvt_t));
    phy->callout_fvt = (dev_88e1543_callout_fvt_t *)malloc(sizeof(dev_88e1543_callout_fvt_t));

    phy->base.dev_state = DEV_STATE_CREATE;

}

/**********************************************************************
 *
 * Function: dev_88e1543_led_on
 *
 * This function: Turn on 88e1543 led.
 *
 * Inputs: *dev_- pointer to the 88E1543 device
 *         portnum - port number
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
static int dev_88e1543_led_on (dev_object_t *dev, uint portnum)
{
    dev_88e1543_object_t *obj_88e1543  = (dev_88e1543_object_t *)dev;
    dev_88e1543_callout_fvt_t *callout_p = obj_88e1543->callout_fvt;
    
    int rc;
    /* change to page 3, write (16_3) to turn on led */
    rc = (*callout_p->smi_write)(portnum, MRV88E1543_REG_PAGE_3,
            MRV88E1543_FUNC_CONTROL_REG, MRV88E1543_FORCE_ON_LED);
    if (rc != PASSED) {
        sprintf(err_msg, "%s: Write data to %#x Failed",
                __func__, 0x16);
        DEV_ERROR_REPORT(dev, err_msg, MRVL_88E1543_LED_ON);
        return (FAILED);
    }
    return (PASSED);
}

/**********************************************************************
 *
 * Function: dev_88e1543_led_off
 *
 * This function: Turn off 88e1543 led.
 *
 * Inputs: *dev -  pointer to the 88E1543 device
 *         portnum - port number
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
static int dev_88e1543_led_off (dev_object_t *dev, uint portnum)
{
    dev_88e1543_object_t *obj_88e1543  = (dev_88e1543_object_t *)dev;
    dev_88e1543_callout_fvt_t *callout_p = obj_88e1543->callout_fvt;

    int rc;
    /* change to page 3, write (16_3) to turn off led */
    rc = (*callout_p->smi_write)(portnum, MRV88E1543_REG_PAGE_3,
            MRV88E1543_FUNC_CONTROL_REG, MRV88E1543_FORCE_OFF_LED);
    if (rc != PASSED) {
        sprintf(err_msg, "%s: Write data to %#x Failed", 
                __func__, 0x16);
        DEV_ERROR_REPORT(dev, err_msg, MRVL_88E1543_LED_OFF);
        return (FAILED);
    }
    return (PASSED);
}

/**********************************************************************
 *
 * Function: dev_88e1543_led_default
 *
 * This function: Default 88e1543 led.
 *
 * Inputs: *dev - pointer to the 88E1543 device
 *         portnum - port number
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
static int dev_88e1543_led_default (dev_object_t *dev, uint portnum)
{
    dev_88e1543_object_t *obj_88e1543  = (dev_88e1543_object_t *)dev;
    dev_88e1543_callout_fvt_t *callout_p = obj_88e1543->callout_fvt;

    int rc;
    /* change to page 3, write (16_3) to default led */
    rc = (*callout_p->smi_write)(portnum, MRV88E1543_REG_PAGE_3,
            MRV88E1543_FUNC_CONTROL_REG, MRV88E1543_DEFAULT_FUNC_CONTROL);
    if (rc != PASSED) {
        sprintf(err_msg, "%s: Write data to %#x Failed", 
                __func__, 0x16);
        DEV_ERROR_REPORT(dev, err_msg, MRVL_88E1543_LED_OFF);
        return (FAILED);
    }
    return (PASSED);
}

/**********************************************************************
 *
 * Function: dev_88e1543_check_if_plugged_with_download_cable
 *
 * This function: Polling 5 seconds to check whether 88e1543 is plugged with download cable.
 *
 * Inputs: *dev - pointer to the 88E1543 device
 *         portnum - port number
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
static int dev_88e1543_check_if_plugged_with_download_cable (dev_object_t *dev, uint portnum)
{
    dev_88e1543_object_t *obj_88e1543  = (dev_88e1543_object_t *)dev;
    dev_88e1543_callout_fvt_t *callout_p = obj_88e1543->callout_fvt;

    int rc;
    uint port_link_status = 0, ix = 0;
    
    /* Polling 5 secs to get ethernet port link status */
    while (ix != MAX_POLLING_LINKUP_TIMES) {
        rc = (*callout_p->get_linkup_status)(portnum, &port_link_status);
        
        if (rc != FALSE) {
            printf("PHY Port %d is link up\n", portnum);
            break;
        } else if (ix == MAX_POLLING_LINKUP_TIMES) {
            printf("Get Port %d link status is timeout.\n", portnum);
            break;
        } else {
            ix = ix + 1;
            msleep(WAIT_FOR_LINKUP);
        }
    }
    all_port_link_status[portnum] = port_link_status;

    return (PASSED);
}

/******** History ******** 
$Log: dev_88e1543.c,v $
Revision 1.8  2020/09/30 09:46:09  alicehua
CSCvv85097: Marvell 88e1543 Register test failed when port is plugged with cable

Revision 1.7  2020/05/13 06:28:43  harrchan
Modify MRV88e1543 phy initialization process to fix interrupt test issue when port0 connect network(CSCvt94067)

Revision 1.6  2019/12/11 10:10:21  lucywang
Merged Nanook to main trunk

Revision 1.5  2019/10/16 23:44:10  alicehua
CSCvr66521: Modify error message in PHY register test(print correct message).

Revision 1.4  2019/07/11 12:34:41  alicehua
Collapse Nutella codes into main trunk

$Endlog$
*/
