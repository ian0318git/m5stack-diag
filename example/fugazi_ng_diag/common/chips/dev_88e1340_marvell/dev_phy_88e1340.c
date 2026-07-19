/* $Id: dev_phy_88e1340.c,v 1.2 2013/10/08 08:48:25 tirawan Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/chips/dev_88e1340_marvell/dev_phy_88e1340.c,v $
 *------------------------------------------------------------------------------
 *
 * Filename:    dev_phy_88e1340.c
 *
 * Description: Marvell 88E1340 PHY device driver.
 *
 * James Lin - January 2010.
 *
 * Copyright (c) 2010-2013 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------------------
 *
 */

#include "endians.h"
#include "types.h"
#include "defs.h"
#include "common.h"
#include "common_utils.h"
#include "dev_print.h"
#include "dev_object.h"
#include "dev_phy_88e1340.h"
#include "free.h"
#include "proto.h"
#include "strings.h"
#include "nvmonvars.h"
#include <stdint.h>

#ifdef LINUX_APP
#include <assert.h>
#endif
/*===================================================================*
 *                    Function Prototypes                            *
 *===================================================================*/

/*===================================================================*
 *                    Global variables                               *
 *===================================================================*/
static char *buf_p;
static char err_msg[MRV88E1340_ERR_MSG_LEN];

dev_object_fvt_t              m88e1340_fvt;
dev_88e1340_callin_fvt_t      m88e1340_callin;
dev_88e1340_callout_fvt_t     m88e1340_callout;

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

static const mrvl_88e1340_phy_regs_t marvell_88e1340_phy_reg_tbl[] = {
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

#define NUM_PHY_PAGES (sizeof(marvell_88e1340_phy_reg_tbl) /      \
                             sizeof(struct mrvl_88e1340_phy_regs_t_))


/*******************************************************************************
 *
 * Name: dev_88e1340_check_phy_addr()
 *
 * Description: Check if phy addr is legal or not
 *
 * Input: dev_object_t pointer to the Marvell device.
 *        phy_addr for checking.
 *
 * Returns: PASSED/FAILED
 *
 *
 *******************************************************************************
 */
int dev_88e1340_check_phy_addr (dev_object_t *dev, uint phy_addr)
{
    dev_88e1340_object_t *phy = (dev_88e1340_object_t *)dev;
    uint start_addr;
    uint addr_seq = phy->addr_seq;
    
    /* init the phy per port */
    if (addr_seq == MRV88E1340_PHY_ADDR_INCR) {
        start_addr = phy->base_phyaddr;
    } else {
        start_addr = phy->base_phyaddr - MRV88E1340_PORTS + 1;
    }
    
    if ((phy_addr < start_addr) || 
        (phy_addr >= (start_addr + MRV88E1340_PORTS))) {
        assert(!"PHY address is invalid");
        return (FAILED);
    }
    return (PASSED);
}


/*******************************************************************************
 *
 * Function: dev_88e1340_reg_show()
 *
 * This function prints the specific 88e1340 PHY register values
 *
 * Input: dev_object_t pointer to the Marvell device
 *        A device print function vector
 *        phy addr 
 *
 * Output: PASSED/FAILED
 *
 *******************************************************************************
 */
static int dev_88e1340_reg_show (dev_object_t *dev, print_fn_t dev_print, 
                                 uint phy_addr)
{
    uint val, ix;
    uint32_t page;
    const reg_info_t *reg_ptr;
    const mrvl_88e1340_phy_regs_t *page_reg_ptr = 
                                   &marvell_88e1340_phy_reg_tbl[0];
    dev_88e1340_object_t *phy = (dev_88e1340_object_t *)dev;
   
    buf_p = err_msg;
    
    if (dev_88e1340_check_phy_addr(dev, phy_addr) == FAILED) {
        buf_p += sprintf(buf_p, "%s(): invalid phy addr (%d) found\n",
                                 __FUNCTION__, phy_addr);
        buf_p = err_msg;
        DEV_ERROR_REPORT(dev, buf_p, MRVL_88E1340_SHOW);
        return (FAILED);
    }

    dev_print("\nMarvell 88E1340 Base Address: %#.8x  phy_addr %#x\n", 
               (intptr_t)phy->base_phyaddr, phy_addr);

    for (ix = 0; ix < NUM_PHY_PAGES; ix++) {

        /* Set Page */
        val = SMIWRITE(phy, phy_addr, ix, MRV88E1340_PAGE_ADDRESS_REG, ix);
        if (val == FAILED) {
            buf_p += sprintf(buf_p, "Switch page fail %x\n", ix);
            return (FAILED);
        }

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
 * Function: dev_88e1340_power_up
 *
 * This function power up/down Copper I/F on PHY device.
 * Both of the two power down bit(power down = 1, normal operation = 0) 
 * must be changed according to input parameter "enable".
 *  1. Page 0, reg 0, bit 11.
 *  2. Page 0, reg 16, bit 2.
 *
 * Input: iface
 *        phy address: phy address of phy port.
 *        enable: 0 - power down, 1 - power up.
 *
 * Output: PASSED/FAILED
 *
 *******************************************************************************
 */
static int dev_88e1340_power_up (dev_object_t *dev, uint phy_addr, uint enable)
{
    dev_88e1340_object_t *phy = (dev_88e1340_object_t *)dev;
    uint rc, reg_d;

    buf_p = err_msg;
    
    if (dev_88e1340_check_phy_addr(dev, phy_addr) == FAILED) {
        buf_p += sprintf(buf_p, "%s(): invalid phy addr (%d) found\n",
                                 __FUNCTION__, phy_addr);
        buf_p = err_msg;
        DEV_ERROR_REPORT(dev, buf_p, MRVL_88E1340_POWER_UP);
        return (FAILED);
    }

    /* 1. Set page 0, reg 0, bit 11. */
    rc = SMIREAD(phy, phy_addr, MRV88E1340_REG_PAGE_0, MRV88E1340_CONTROL_REG, 
                 &reg_d);
    if (rc != PASSED) {
        buf_p += sprintf(buf_p, "%s(): phy smi read failed. phy_addr = "
                                "#%x, page = %#x, reg = %#x, rc = %#x\n",
                                 __FUNCTION__, phy_addr, 
                                 MRV88E1340_REG_PAGE_0,
                                 MRV88E1340_CONTROL_REG, rc);
        buf_p = err_msg;
        DEV_ERROR_REPORT(dev, buf_p, MRVL_88E1340_POWER_UP);
        return (FAILED);
    }

    if (enable == ENABLE) {
        if (reg_d & MRV88E1340_PWR_DOWN) { /* in power down mode */
            reg_d &= ~MRV88E1340_PWR_DOWN;
            rc = SMIWRITE(phy, phy_addr, MRV88E1340_REG_PAGE_0, 
                          MRV88E1340_CONTROL_REG, reg_d);
            if (rc != PASSED) {
                buf_p += sprintf(buf_p, "%s(): phy smi write failed. "
                                        "phy_addr = #%x, page = %#x, "
                                        "reg = %#x, data = %#x, rc = %#x\n",
                                         __FUNCTION__, phy_addr, 
                                         MRV88E1340_REG_PAGE_0,
                                         MRV88E1340_CONTROL_REG, reg_d, rc);
                buf_p = err_msg;      
                DEV_ERROR_REPORT(dev, buf_p, MRVL_88E1340_POWER_UP);
                return (FAILED);
            }
        }
    } else {    /* DISABLE */
        if (!(reg_d & MRV88E1340_PWR_DOWN)) { /* not in power down mode */
            reg_d |= MRV88E1340_PWR_DOWN;
            rc = SMIWRITE(phy, phy_addr, MRV88E1340_REG_PAGE_0, 
                          MRV88E1340_CONTROL_REG, reg_d);
            if (rc != PASSED) {
                buf_p += sprintf(buf_p, "%s(): phy smi write failed. "
                                        "phy_addr = #%x, page = %#x, "
                                        "reg = %#x, data = %#x, rc = %#x\n",
                                         __FUNCTION__, phy_addr, 
                                         MRV88E1340_REG_PAGE_0,
                                         MRV88E1340_CONTROL_REG, reg_d, rc);
                buf_p = err_msg;
                DEV_ERROR_REPORT(dev, buf_p, MRVL_88E1340_POWER_UP);
                return (FAILED);
            }
        }        
    }

    /* 2. Set page 0, reg 16, bit 2. */
    rc = SMIREAD(phy, phy_addr, MRV88E1340_REG_PAGE_0, 
                 MRV88E1340_SPECIFIC_CONTROL1_REG, &reg_d);
    if (rc != PASSED) {
        buf_p += sprintf(buf_p, "%s(): phy smi read failed. phy_addr = "
                                "#%x, page = %#x, reg = %#x, rc = %#x\n",
                                 __FUNCTION__, phy_addr, 
                                 MRV88E1340_REG_PAGE_0,
                                 MRV88E1340_SPECIFIC_CONTROL1_REG, rc);
        buf_p = err_msg;    
        DEV_ERROR_REPORT(dev, buf_p, MRVL_88E1340_POWER_UP);
        return (FAILED);
    }

    if (enable == ENABLE) {
        if (reg_d & MRV88E1340_P0_R16_PWR_DOWN) { /* in power down mode */
            reg_d &= ~MRV88E1340_P0_R16_PWR_DOWN;
            rc = SMIWRITE(phy, phy_addr, MRV88E1340_REG_PAGE_0, 
                          MRV88E1340_P0_R16_PWR_DOWN, reg_d);
            if (rc != PASSED) {
                buf_p += sprintf(buf_p, "%s(): phy smi write failed. "
                                        "phy_addr = #%x, page = %#x, "
                                        "reg = %#x, data = %#x, rc = %#x\n",
                                         __FUNCTION__, phy_addr, 
                                         MRV88E1340_REG_PAGE_0,
                                         MRV88E1340_SPECIFIC_CONTROL1_REG, 
                                         reg_d, rc);
                buf_p = err_msg;        
                DEV_ERROR_REPORT(dev, buf_p, MRVL_88E1340_POWER_UP);
                return (FAILED);
            }
        }
    } else {    /* DISABLE */
        if (!(reg_d & MRV88E1340_P0_R16_PWR_DOWN)) {/* not in power down mode */
            reg_d |= MRV88E1340_P0_R16_PWR_DOWN;
            rc = SMIWRITE(phy, phy_addr, MRV88E1340_REG_PAGE_0, 
                          MRV88E1340_P0_R16_PWR_DOWN, reg_d);
            if (rc != PASSED) {
                buf_p += sprintf(buf_p, "%s(): phy smi write failed. "
                                        "phy_addr = #%x, page = %#x, "
                                        "reg = %#x, data = %#x, rc = %#x\n",
                                         __FUNCTION__, phy_addr, 
                                         MRV88E1340_REG_PAGE_0,
                                         MRV88E1340_SPECIFIC_CONTROL1_REG, 
                                         reg_d, rc);
                buf_p = err_msg;        
                DEV_ERROR_REPORT(dev, buf_p, MRVL_88E1340_POWER_UP);
                return (FAILED);
            }
        }
    }
    return (PASSED);
}


/*******************************************************************************
 *
 * Function: dev_88e1340_alter_reg
 *
 * This function is a utility to alter specific PHY register.
 *
 * Input: dev - Pointer to the Marvell GE device object
 *
 * Output: PASSED/FAILED
 *
 *******************************************************************************
 */
static int dev_88e1340_alter_reg (dev_object_t *dev)
{
    uint phy_addr, page, reg, data;
    int retval=FAILED;
    dev_88e1340_object_t *phy = (dev_88e1340_object_t *)dev;

    buf_p = err_msg;

    page = gethex_answer("Enter the page: ", MRV88E1340_REG_PAGE_0,
                                             MRV88E1340_REG_PAGE_0, 
                                             MRV88E1340_REG_PAGE_MAX);

    reg = gethex_answer("Enter the register offset: ", 0, 0, 0x1f);

    /* display original value */
    retval = SMIREAD(phy, phy_addr, page, reg, &data);
    if (retval == PASSED) {
        printf("Original value: phy_addr %d, page %d, reg %d, data %#.8x\n",
                phy_addr, page, reg, data);
    } else {
        buf_p += sprintf(buf_p, "%s(): Can not read back reg before "
                                "alternation.\n", __FUNCTION__);
        buf_p = err_msg;
        DEV_ERROR_REPORT(dev, buf_p, MRVL_88E1340_ALTER_REG);
        return (FAILED);
    }

    /* alter register */
    data = gethex_answer("Enter the new data: ", 0, 0, 0xFFFF);

    /* Set Page */
    retval = SMIWRITE(phy, phy_addr, page, MRV88E1340_PAGE_ADDRESS_REG, page);
    if (retval == FAILED) {
        buf_p += sprintf(buf_p, "Switch page fail %x\n", page);
        return (FAILED);
    }

    retval = SMIWRITE(phy, phy_addr, page, reg, data);
    if (retval != PASSED) {
        buf_p += sprintf(buf_p, "%s(): Alter PHY reg write smi failed.\n",
                                 __FUNCTION__);
        buf_p = err_msg;
        DEV_ERROR_REPORT(dev, buf_p, MRVL_88E1340_ALTER_REG);
        return (FAILED);
    }

    retval = SMIREAD(phy, phy_addr, page, reg, &data);
    if (retval == PASSED) {
        printf("Alter phy_addr %d, page %d, reg %d, data %#.8x\n",
                phy_addr, page, reg, data);
    } else {
        buf_p += sprintf(buf_p, "%s(): Can not read back reg after "
                                "alternation.\n", __FUNCTION__);
        buf_p = err_msg;
        DEV_ERROR_REPORT(dev, buf_p, MRVL_88E1340_ALTER_REG);
    }
    
    return (retval);
}


/*******************************************************************************
 *
 * Function: dev_88e1340_lockup_fix
 *
 * This function is a workaround for SGMII/Fiber Interface lockup issue
 * after hard-reset de-assert or SMGII/Fiber interface is powered down and up
 *
 * Input: iface
 *        phy address
 *
 * Output: PASSED/FAILED
 *
 * Refer to Release notes section 3.6
 * If phy is not sync up(page 1, reg 17, bit 5), do following fix:
 *   1) write page   0, reg  0, value 0x1940 (Power down the PHY)
 *   2) write page  29. reg 30, value 0xEF00 (Reset PCS digital logic -cooper)
 *   3) write page 254, reg 26, value 0x803F (Reset PCS digital logic -SGMII)
 *   4) write page 254, reg 24, value 0x9300 (Reset PLL and RX)
 *   5) write page 254, reg 24, value 0x9100 (Release PLL reset)
 *   6) write page 254, reg 24, value 0x8100 (Releae RX reset)
 *   7) write page 254, reg 24, value 0x0000 (Disable override)
 *   8) wait 5ms
 *   9) write page 254, reg 26, value 0x8000 (Deassert PCS reset -SGMII)
 *  10) write page 254, reg 26, value 0x0000
 *  11) write page  29, reg 30, value 0x0000 (Deassert PCS reset -cooper)
 *  12) write page   0, reg  0, value 0x9140 (Release PHY from power down)
 *  13) wiat 100ms
 *******************************************************************************
 */
static int dev_88e1340_lockup_fix (dev_object_t *dev, uint phy_addr)
{
    dev_88e1340_object_t *phy = (dev_88e1340_object_t *)dev;
    uint rc, reg_d;

    buf_p = err_msg;
    
    if (dev_88e1340_check_phy_addr(dev, phy_addr) == FAILED) {
        buf_p += sprintf(buf_p, "%s(): invalid phy addr (%d) found\n",
                                 __FUNCTION__, phy_addr);
        buf_p = err_msg;
        DEV_ERROR_REPORT(dev, buf_p, MRVL_88E1340_LOCKUP_FIX);
        return (FAILED);
    }

    
    rc = SMIREAD(phy, phy_addr, MRV88E1340_REG_PAGE_1, 
                 MRV88E1340_SPECIFIC_STATUS1_REG, &reg_d);

    if (rc != PASSED) {
        buf_p += sprintf(buf_p, "%s(): phy smi read failed. "
                                "phy_addr = #%x, page = %#x, "
                                "reg = %#x, rc = %#x\n", 
                                 __FUNCTION__, phy_addr, 
                                 MRV88E1340_REG_PAGE_1,
                                 MRV88E1340_SPECIFIC_STATUS1_REG, rc);
        buf_p = err_msg;
        DEV_ERROR_REPORT(dev, buf_p, MRVL_88E1340_LOCKUP_FIX);
        return (FAILED);
    }

    if (reg_d & MRV88E1340_P1_R17_SYNC) { /* Check if sync up */
        return (PASSED);
    } else {  /* Not sync up */
        /* 1) write page   0, reg  0, value 0x1940 */
        reg_d = 0x1940;
        rc = SMIWRITE(phy, phy_addr, MRV88E1340_REG_PAGE_0, 
                      MRV88E1340_CONTROL_REG, reg_d);
        if (rc != PASSED) {
            buf_p += sprintf(buf_p, "%s(): phy smi write failed. "
                                    "phy_addr = #%x, page = %#x, "
                                    "reg = %#x, data = %#x, rc = %#x\n",
                                     __FUNCTION__, phy_addr,
                                     MRV88E1340_REG_PAGE_0, 
                                     MRV88E1340_CONTROL_REG, reg_d, rc);
            buf_p = err_msg;
            DEV_ERROR_REPORT(dev, buf_p, MRVL_88E1340_LOCKUP_FIX);
            return (FAILED);
        }

        /* 2) write page  29. reg 30, value 0xEF00 */
        reg_d = 0xEF00;
        rc = SMIWRITE(phy, phy_addr, MRV88E1340_REG_PAGE_29, 
                      MRV88E1340_REG_30, reg_d);
        if (rc != PASSED) {
            buf_p += sprintf(buf_p, "%s(): phy smi write failed. "
                                    "phy_addr = #%x, page = %#x, "
                                    "reg = %#x, data = %#x, rc = %#x\n",
                                     __FUNCTION__, phy_addr,
                                     MRV88E1340_REG_PAGE_29, 
                                     MRV88E1340_REG_30, reg_d, rc);
            buf_p = err_msg;
            DEV_ERROR_REPORT(dev, buf_p, MRVL_88E1340_LOCKUP_FIX);
            return (FAILED);
        }

        /* 3) write page 254, reg 26, value 0x803F */
        reg_d = 0x803F;
        rc = SMIWRITE(phy, phy_addr, MRV88E1340_REG_PAGE_254, 
                      MRV88E1340_REG_26, reg_d);
        if (rc != PASSED) {
            buf_p += sprintf(buf_p, "%s(): phy smi write failed. "
                                    "phy_addr = #%x, page = %#x, "
                                    "reg = %#x, data = %#x, rc = %#x\n",
                                     __FUNCTION__, phy_addr,
                                     MRV88E1340_REG_PAGE_254, 
                                     MRV88E1340_REG_26, reg_d, rc);
            buf_p = err_msg;
            DEV_ERROR_REPORT(dev, buf_p, MRVL_88E1340_LOCKUP_FIX);
            return (FAILED);
        }

        /* 4) write page 254, reg 24, value 0x9300 */
        reg_d = 0x9300;
        rc = SMIWRITE(phy, phy_addr, MRV88E1340_REG_PAGE_254, 
                      MRV88E1340_REG_24, reg_d);
        if (rc != PASSED) {
            buf_p += sprintf(buf_p, "%s(): phy smi write failed. "
                                    "phy_addr = #%x, page = %#x, "
                                    "reg = %#x, data = %#x, rc = %#x\n",
                                     __FUNCTION__, phy_addr, 
                                     MRV88E1340_REG_PAGE_254, 
                                     MRV88E1340_REG_24, reg_d, rc);
            buf_p = err_msg;
            DEV_ERROR_REPORT(dev, buf_p, MRVL_88E1340_LOCKUP_FIX);
            return (FAILED);
        }

        /* 5) write page 254, reg 24, value 0x9100 */
        reg_d = 0x9100;
        rc = SMIWRITE(phy, phy_addr, MRV88E1340_REG_PAGE_254, 
                      MRV88E1340_REG_24, reg_d);
        if (rc != PASSED) {
            buf_p += sprintf(buf_p, "%s(): phy smi write failed. "
                                    "phy_addr = #%x, page = %#x, "
                                    "reg = %#x, data = %#x, rc = %#x\n",
                                     __FUNCTION__, phy_addr,
                                     MRV88E1340_REG_PAGE_254, 
                                     MRV88E1340_REG_24, reg_d, rc);
            buf_p = err_msg;
            DEV_ERROR_REPORT(dev, buf_p, MRVL_88E1340_LOCKUP_FIX);
            return (FAILED);
        }

        /* 6) write page 254, reg 24, value 0x8100 */
        reg_d = 0x8100;
        rc = SMIWRITE(phy, phy_addr, MRV88E1340_REG_PAGE_254, 
                      MRV88E1340_REG_24, reg_d);
        if (rc != PASSED) {
            buf_p += sprintf(buf_p, "%s(): phy smi write failed. "
                                    "phy_addr = #%x, page = %#x, "
                                    "reg = %#x, data = %#x, rc = %#x\n",
                                     __FUNCTION__, phy_addr, 
                                     MRV88E1340_REG_PAGE_254, 
                                     MRV88E1340_REG_24, reg_d, rc);
            buf_p = err_msg;
            DEV_ERROR_REPORT(dev, buf_p, MRVL_88E1340_LOCKUP_FIX);
            return (FAILED);
        }

        /* 7) write page 254, reg 24, value 0x0000 */
        reg_d = 0x0000;
        rc = SMIWRITE(phy, phy_addr, MRV88E1340_REG_PAGE_254, 
                      MRV88E1340_REG_24, reg_d);
        if (rc != PASSED) {
            buf_p += sprintf(buf_p, "%s(): phy smi write failed. "
                                    "phy_addr = #%x, page = %#x, "
                                    "reg = %#x, data = %#x, rc = %#x\n",
                                     __FUNCTION__, phy_addr, 
                                     MRV88E1340_REG_PAGE_254, 
                                     MRV88E1340_REG_24, reg_d, rc);
            buf_p = err_msg;
            DEV_ERROR_REPORT(dev, buf_p, MRVL_88E1340_LOCKUP_FIX);
            return (FAILED);
        }

        /* 8) wait 5ms */
        msleep(5);
 
        /* 9) write page 254, reg 26, value 0x8000 */
        reg_d = 0x8000;
        rc = SMIWRITE(phy, phy_addr, MRV88E1340_REG_PAGE_254, 
                      MRV88E1340_REG_26, reg_d);
        if (rc != PASSED) {
            buf_p += sprintf(buf_p, "%s(): phy smi write failed. "
                                    "phy_addr = #%x, page = %#x, "
                                    "reg = %#x, data = %#x, rc = %#x\n",
                                     __FUNCTION__, phy_addr, 
                                     MRV88E1340_REG_PAGE_254, 
                                     MRV88E1340_REG_26, reg_d, rc);
            buf_p = err_msg;
            DEV_ERROR_REPORT(dev, buf_p, MRVL_88E1340_LOCKUP_FIX);
            return (FAILED);
        }       

        /* 10) write page 254, reg 26, value 0x0000 */
        reg_d = 0x0000;
        rc = SMIWRITE(phy, phy_addr, MRV88E1340_REG_PAGE_254, 
                      MRV88E1340_REG_26, reg_d);
        if (rc != PASSED) {
            buf_p += sprintf(buf_p, "%s(): phy smi write failed. "
                                    "phy_addr = #%x, page = %#x, "
                                    "reg = %#x, data = %#x, rc = %#x\n",
                                     __FUNCTION__, phy_addr,
                                     MRV88E1340_REG_PAGE_254, 
                                     MRV88E1340_REG_26, reg_d, rc);
            buf_p = err_msg;
            DEV_ERROR_REPORT(dev, buf_p, MRVL_88E1340_LOCKUP_FIX);
            return (FAILED);
        }

        /* 11) write page  29, reg 30, value 0x0000 */
        reg_d = 0x0000;
        rc = SMIWRITE(phy, phy_addr, MRV88E1340_REG_PAGE_29, 
                      MRV88E1340_REG_30, reg_d);
        if (rc != PASSED) {
            buf_p += sprintf(buf_p, "%s(): phy smi write failed. "
                                    "phy_addr = #%x, page = %#x, "
                                    "reg = %#x, data = %#x, rc = %#x\n",
                                     __FUNCTION__, phy_addr, 
                                     MRV88E1340_REG_PAGE_29, 
                                     MRV88E1340_REG_30, reg_d, rc);
            buf_p = err_msg;
            DEV_ERROR_REPORT(dev, buf_p, MRVL_88E1340_LOCKUP_FIX);
            return (FAILED);
        }

        /* 12) write page   0, reg  0, value 0x9140 */
        reg_d = 0x9140;
        rc = SMIWRITE(phy, phy_addr, MRV88E1340_REG_PAGE_0, 
                      MRV88E1340_CONTROL_REG, reg_d);
        if (rc != PASSED) {
            buf_p += sprintf(buf_p, "%s(): phy smi write failed. "
                                    "phy_addr = #%x, page = %#x, "
                                    "reg = %#x, data = %#x, rc = %#x\n",
                                     __FUNCTION__, phy_addr, 
                                     MRV88E1340_REG_PAGE_0,
                                     MRV88E1340_CONTROL_REG, reg_d, rc);
            buf_p = err_msg;
            DEV_ERROR_REPORT(dev, buf_p, MRVL_88E1340_LOCKUP_FIX);
            return (FAILED);
        }

        /* 13) wait 100ms */
        msleep(100);

    }

    return (PASSED);
}


/*******************************************************************************
 *
 * Function: dev_88e1340_intr_clr
 *
 * This function clear PHY intr.
 *
 * Input: iface - Pointer to the Marvell GE device object
 *
 * Output: PASSED/FAILED
 *
 *******************************************************************************
 */
static int dev_88e1340_intr_clr (dev_object_t *dev, uint phy_addr)
{
    uint rc, reg_d;
    dev_88e1340_object_t *phy = (dev_88e1340_object_t *)dev;

    buf_p = err_msg;
    
    if (dev_88e1340_check_phy_addr(dev, phy_addr) == FAILED) {
        buf_p += sprintf(buf_p, "%s(): invalid phy addr (%d) found\n",
                                 __FUNCTION__, phy_addr);
        buf_p = err_msg;
        DEV_ERROR_REPORT(dev, buf_p, MRVL_88E1340_INTR_CLR);
        return (FAILED);
    }

    /* disable temperature sensor interrupt generation */
    rc = SMIREAD(phy, phy_addr, MRV88E1340_REG_PAGE_6, 
                 MRV88E1340_P6_MISC_TEST, &reg_d);    
    if (rc != PASSED) {
        buf_p += sprintf(buf_p, "%s(): phy smi read failed. phy_addr = "
                                "#%x, page = %#x, reg = %#x, rc = %#x\n",
                                 __FUNCTION__, phy_addr, 
                                 MRV88E1340_REG_PAGE_6,
                                 MRV88E1340_P6_MISC_TEST, rc);
        buf_p = err_msg;    
        DEV_ERROR_REPORT(dev, buf_p, MRVL_88E1340_INTR_CLR);
        return (FAILED);
    } else {
        reg_d &= ~MRV88E1340_P6_R26_TEMP_THRESHOLD_MASK;
        reg_d |= MRV88E1340_P6_R26_TEMP_100C;  /* Set threshold to 100C */
        reg_d &= ~MRV88E1340_P6_R26_ENA_TEMP_SENSOR_INTR;
    }    

    rc = SMIWRITE(phy, phy_addr, MRV88E1340_REG_PAGE_6, 
                  MRV88E1340_P6_MISC_TEST, reg_d);
    if (rc != PASSED) {
        buf_p += sprintf(buf_p, "%s(): phy smi write failed. "
                                "phy_addr = #%x, page = %#x, "
                                "reg = %#x, data = %#x, rc = %#x\n",
                                 __FUNCTION__, phy_addr, 
                                 MRV88E1340_REG_PAGE_6,
                                 MRV88E1340_P6_MISC_TEST, reg_d, rc);
        buf_p = err_msg;
        DEV_ERROR_REPORT(dev, buf_p, MRVL_88E1340_INTR_CLR);     
        return (FAILED);
    }

    return (PASSED);
}


/*******************************************************************************
 *
 * Function: dev_88e1340_intr_gen
 *
 * This function generate PHY intr to Host.
 *
 * Input: iface - Pointer to the Marvell GE device object
 *
 * Output: PASSED/FAILED
 *
 *******************************************************************************
 */
static int dev_88e1340_intr_gen (dev_object_t *dev, uint phy_addr)
{
    uint rc, reg_d;
    dev_88e1340_object_t *phy = (dev_88e1340_object_t *)dev;

    buf_p = err_msg;
    
    if (dev_88e1340_check_phy_addr(dev, phy_addr) == FAILED) {
        buf_p += sprintf(buf_p, "%s(): invalid phy addr (%d) found\n",
                                 __FUNCTION__, phy_addr);
        buf_p = err_msg;
        DEV_ERROR_REPORT(dev, buf_p, MRVL_88E1340_INTR_GEN);
        return (FAILED);
    }

    /* generate temperature sensor interrupt */
    rc = SMIREAD(phy, phy_addr, MRV88E1340_REG_PAGE_6, 
                 MRV88E1340_P6_MISC_TEST, &reg_d);   
    if (rc != PASSED) {
        buf_p += sprintf(buf_p, "%s(): phy smi read failed. phy_addr = "
                                "#%x, page = %#x, reg = %#x, rc = %#x\n",
                                 __FUNCTION__, phy_addr, 
                                 MRV88E1340_REG_PAGE_6,
                                 MRV88E1340_P6_MISC_TEST, rc);
        buf_p = err_msg;    
        DEV_ERROR_REPORT(dev, buf_p, MRVL_88E1340_INTR_GEN);
        return (FAILED);
    } else {
        reg_d &= ~MRV88E1340_P6_R26_TEMP_THRESHOLD_MASK;
        reg_d |= MRV88E1340_P6_R26_TEMP_NEG_25C;  /* Set threshold to -25C */
        reg_d |= MRV88E1340_P6_R26_ENA_TEMP_SENSOR_INTR;
    }
    rc = SMIWRITE(phy, phy_addr, MRV88E1340_REG_PAGE_6, 
                  MRV88E1340_P6_MISC_TEST, reg_d);
    if (rc != PASSED) {
        buf_p += sprintf(buf_p, "%s(): phy smi write failed. "
                                "phy_addr = #%x, page = %#x, "
                                "reg = %#x, data = %#x, rc = %#x\n",
                                 __FUNCTION__, phy_addr, 
                                 MRV88E1340_REG_PAGE_6,
                                 MRV88E1340_P6_MISC_TEST, reg_d, rc);
        buf_p = err_msg;
        DEV_ERROR_REPORT(dev, buf_p, MRVL_88E1340_INTR_GEN); 
        return (FAILED);
    }

    return (PASSED);
}


/*******************************************************************************
 *
 * Function: dev_88e1340_set_test_mode
 *
 * This function provides PHY test mode for Marvell GE PHYs.
 *
 * Input: iface - Pointer to the Marvell GE device object
 *
 * Output: PASSED/FAILED
 *
 *******************************************************************************
 */
static int dev_88e1340_set_test_mode (dev_object_t *dev, uint phy_addr)
{
    uint rc;
    uint retval, reg_d, test_mode;
    dev_88e1340_object_t *phy = (dev_88e1340_object_t *)dev;

    buf_p = err_msg;
    
    if (dev_88e1340_check_phy_addr(dev, phy_addr) == FAILED) {
        buf_p += sprintf(buf_p, "%s(): invalid phy addr (%d) found\n",
                                 __FUNCTION__, phy_addr);
        buf_p = err_msg;
        DEV_ERROR_REPORT(dev, buf_p, MRVL_88E1340_SET_TEST_MODE);
        return (FAILED);
    }

    retval = dev_88e1340_power_up(dev, phy_addr, ENABLE);
    if (retval != PASSED) {
        return (FAILED);
    }

    /* Got the current mode. */
    rc = SMIREAD(phy, phy_addr, MRV88E1340_REG_PAGE_0, 
                 MRV88E1340_1000B_CNTL_REG, &reg_d);

    if (rc != PASSED) {
        buf_p += sprintf(buf_p, "%s(): Unable to read test mode. "
                                "rc = %#x", __FUNCTION__, rc);
        buf_p = err_msg;
        DEV_ERROR_REPORT(dev, buf_p, MRVL_88E1340_SET_TEST_MODE);
        return (FAILED);
    }

    test_mode = (reg_d & PHY_GT_CTL_TEST_MASK) >> (PHY_GT_CTL_TEST_SHIFT);

    printf("\nTest modes - (Current test mode is %d.) \n", test_mode);
    printf("    0 - Normal Mode\n");
    printf("    1 - Test Mode 1 - Transmit Waveform Test\n");
    printf("    2 - Test Mode 2 - Transmit Jitter Test (Master mode)\n");
    printf("    3 - Test Mode 3 - Transmit Jitter Test (Slave mode)\n");
    printf("    4 - Test Mode 4 - Transmit Distortion Test\n");
    test_mode = gethex_answer("Enter the test mode: ", test_mode, 0, 
                               PHY_GT_CTL_TEST_MAX);

    /* Write the new data */
    reg_d &= (~PHY_GT_CTL_TEST_MASK); /* clear the test mode */
    reg_d |= (test_mode << PHY_GT_CTL_TEST_SHIFT);

    rc = SMIWRITE(phy, phy_addr, MRV88E1340_REG_PAGE_0, 
                  MRV88E1340_1000B_CNTL_REG, reg_d);

    if (rc != PASSED) {
        buf_p += sprintf(buf_p, "%s(): Unable to write test mode. "
                                "rc = %#x", __FUNCTION__, rc);
        buf_p = err_msg;
        DEV_ERROR_REPORT(dev, buf_p, MRVL_88E1340_SET_TEST_MODE);
        return (FAILED);
    }

    return (PASSED);
}


/*******************************************************************************
 *
 * Function: dev_88e1340_reset().
 *
 * This function resets the phy and waits for its completion before returning.
 *
 * Input:  dev_object_t pointer to the Marvell GE device.
 *         reset: TRUE/FALSE.
 *
 * Output: PASSED/FAILED.
 *
 *******************************************************************************
 */
static int dev_88e1340_reset (dev_object_t *dev, uint phy_addr)
{
    dev_88e1340_object_t *phy = (dev_88e1340_object_t *)dev;
    int retval = FAILED;
    uint data;

    /* do a soft reset and power up */
    retval = SMIREAD(phy, phy_addr, MRV88E1340_REG_PAGE_0, 
                     MRV88E1340_CONTROL_REG, &data);
    if (retval == PASSED) {
        data |= (MRV88E1340_COOPER_RST);
        retval = SMIWRITE(phy, phy_addr, MRV88E1340_REG_PAGE_0, 
                          MRV88E1340_CONTROL_REG, data);
        if (retval != PASSED) {
            buf_p += sprintf(buf_p, "%s(): phy reset failed (phy addr = %d)\n",
                                     __FUNCTION__, phy_addr);
            return (retval);
        }
    } else {
        buf_p += sprintf(buf_p, "%s(): read phy control reg failed (phy addr"
                                " %d)\n", __FUNCTION__, phy_addr);
        return (retval);
    }

    /* Check if phy reset complete */
    msleep(100);
    retval = SMIREAD(phy, phy_addr, MRV88E1340_REG_PAGE_0, 
                     MRV88E1340_CONTROL_REG, &data);
    if (retval == PASSED) {
        if (data & MRV88E1340_COOPER_RST) {
            buf_p += sprintf(buf_p, "%s(): failed to get out of reset (phy "
                                    "addr = %d).\n", __FUNCTION__, phy_addr);
        } else {
            if ((NVRAM)->diagflag & D_VERBOSE) {
                printf("%s(): PHY soft reset ok at phy addr %d\n", 
                        __FUNCTION__, phy_addr);
            }
        }
    } else {
        buf_p += sprintf(buf_p, "%s(): Failed to read back reset bit (phy "
                                "addr = %d)\n", __FUNCTION__, phy_addr);
    }

    msleep(100);
    return (retval);
}


/*******************************************************************************
 *
 * Function: dev_88e1340_phone_detect
 *
 * Description: Check if Marvell switch detects a Power Device (PD) at a
 *              given port.
 *
 * Input:    iface - Pointer to interface data structure.
 *           ext_port - extnal ports to check for the PD.
 *
 * Outputs:  PASSED - Found a PD.
 *           FAILED - PD not detected or phy access error.
 *
 * Assumptions:
 *
 *******************************************************************************
 */
static int dev_88e1340_phone_detect (dev_object_t *dev, uint phy_addr)
{
    dev_88e1340_object_t *phy = (dev_88e1340_object_t *)dev;
    uint ix, rc, reg_d, detected;

    buf_p = err_msg;
    
    if (dev_88e1340_check_phy_addr(dev, phy_addr) == FAILED) {
        buf_p += sprintf(buf_p, "%s(): invalid phy addr (%d) found\n",
                                 __FUNCTION__, phy_addr);
        buf_p = err_msg;
        DEV_ERROR_REPORT(dev, buf_p, MRVL_88E1340_PHONE_DETECT);
        return (FAILED);
    }
    
    rc = dev_88e1340_power_up(dev, phy_addr, ENABLE);
    if (rc != PASSED) {
        return (FAILED);
    }
    
    /* Disable Auto Negotiation */
    rc = SMIREAD(phy, phy_addr, MRV88E1340_REG_PAGE_0, 
                 MRV88E1340_CONTROL_REG, &reg_d);
    if (rc != PASSED) {
        buf_p += sprintf(buf_p, "%s(): phy smi read failed. phy_addr = "
                                "#%x, page = %#x, reg = %#x, rc = %#x\n",
                                 __FUNCTION__, phy_addr, 
                                 MRV88E1340_REG_PAGE_0,
                                 MRV88E1340_CONTROL_REG, rc);
        buf_p = err_msg;    
        DEV_ERROR_REPORT(dev, buf_p, MRVL_88E1340_PHONE_DETECT);
        return (FAILED);
    }

    if ((reg_d & MRV88E1340_AUTO_NEO_ENA) == MRV88E1340_AUTO_NEO_ENA) {
        reg_d &= ~MRV88E1340_AUTO_NEO_ENA;
        rc = SMIWRITE(phy, phy_addr, MRV88E1340_REG_PAGE_0, 
                      MRV88E1340_CONTROL_REG, reg_d);
        if (rc != PASSED) {
            buf_p += sprintf(buf_p, "%s(): phy smi write failed. "
                                    "phy_addr = #%x, page = %#x, "
                                    "reg = %#x, data = %#x, rc = %#x\n",
                                     __FUNCTION__, phy_addr, 
                                     MRV88E1340_REG_PAGE_0,
                                     MRV88E1340_CONTROL_REG, reg_d, rc);
            buf_p = err_msg;
            DEV_ERROR_REPORT(dev, buf_p, MRVL_88E1340_PHONE_DETECT);     
            return (FAILED);
        }
        
        rc = dev_88e1340_reset(dev, phy_addr);
        if (rc != PASSED) {
            buf_p += sprintf(buf_p, "%s(): phy addr %d reset failed. rc = %#x"
                                    "\n", __FUNCTION__, phy_addr, rc);
            buf_p = err_msg;
            DEV_ERROR_REPORT(dev, buf_p, MRVL_88E1340_PHONE_DETECT);
            return (FAILED);
        }
    }

    /* Disable power over Ethernet detection */
    rc = SMIREAD(phy, phy_addr, MRV88E1340_REG_PAGE_0, 
                 MRV88E1340_SPECIFIC_CONTROL3_REG, &reg_d);
    if (rc != PASSED) {
        return (FAILED);
    }
    
    reg_d &= ~MRV88E1340_P0_R26_DTE_DETECT;
    rc = SMIWRITE(phy, phy_addr, MRV88E1340_REG_PAGE_0, 
                  MRV88E1340_SPECIFIC_CONTROL3_REG, reg_d);
    if (rc != PASSED) {
        buf_p += sprintf(buf_p, "%s(): phy smi write failed. "
                                "phy_addr = #%x, page = %#x, "
                                "reg = %#x, data = %#x, rc = %#x\n",
                                 __FUNCTION__, phy_addr, 
                                 MRV88E1340_REG_PAGE_0,
                                 MRV88E1340_SPECIFIC_CONTROL3_REG, reg_d, rc);
        buf_p = err_msg;
        DEV_ERROR_REPORT(dev, buf_p, MRVL_88E1340_PHONE_DETECT);     
        return (FAILED);
    }

    /* Set DTE power status drop to 5 seconds */
    rc = SMIREAD(phy, phy_addr, MRV88E1340_REG_PAGE_0, 
                 MRV88E1340_SPECIFIC_CONTROL3_REG, &reg_d);
    if (rc != PASSED) {
        buf_p += sprintf(buf_p, "%s(): phy smi read failed. phy_addr = "
                                "#%x, page = %#x, reg = %#x, rc = %#x\n",
                                 __FUNCTION__, phy_addr, 
                                 MRV88E1340_REG_PAGE_0,
                                 MRV88E1340_SPECIFIC_CONTROL3_REG, rc);
        buf_p = err_msg;    
        DEV_ERROR_REPORT(dev, buf_p, MRVL_88E1340_PHONE_DETECT);
        return (FAILED);
    }
    
    reg_d &= ~MRV88E1340_P0_R26_DTE_STATUS_DROP_MSK;
    reg_d |= MRV88E1340_P0_R26_DTE_STATUS_DROP_5S;
    rc = SMIWRITE(phy, phy_addr, MRV88E1340_REG_PAGE_0, 
                  MRV88E1340_SPECIFIC_CONTROL3_REG, reg_d);
    if (rc != PASSED) {
        buf_p += sprintf(buf_p, "%s(): phy smi write failed. "
                                "phy_addr = #%x, page = %#x, "
                                "reg = %#x, data = %#x, rc = %#x\n",
                                 __FUNCTION__, phy_addr, 
                                 MRV88E1340_REG_PAGE_0,
                                 MRV88E1340_SPECIFIC_CONTROL3_REG, reg_d, rc);
        buf_p = err_msg;
        DEV_ERROR_REPORT(dev, buf_p, MRVL_88E1340_PHONE_DETECT);  
        return (FAILED);
    }

    /* Enable power over Ethernet detection bit */
    rc = SMIREAD(phy, phy_addr, MRV88E1340_REG_PAGE_0, 
                 MRV88E1340_SPECIFIC_CONTROL3_REG, &reg_d);
    if (rc != PASSED) {
        buf_p += sprintf(buf_p, "%s(): phy smi read failed. phy_addr = "
                                "#%x, page = %#x, reg = %#x, rc = %#x\n",
                                 __FUNCTION__, phy_addr, 
                                 MRV88E1340_REG_PAGE_0,
                                 MRV88E1340_SPECIFIC_CONTROL3_REG, rc);
        buf_p = err_msg;    
        DEV_ERROR_REPORT(dev, buf_p, MRVL_88E1340_PHONE_DETECT);
        return (FAILED);
    }
    
    reg_d |= MRV88E1340_P0_R26_DTE_DETECT;
    rc = SMIWRITE(phy, phy_addr, MRV88E1340_REG_PAGE_0, 
                  MRV88E1340_SPECIFIC_CONTROL3_REG, reg_d);
    if (rc != PASSED) {
        buf_p += sprintf(buf_p, "%s(): phy smi write failed. "
                                "phy_addr = #%x, page = %#x, "
                                "reg = %#x, data = %#x, rc = %#x\n",
                                 __FUNCTION__, phy_addr, 
                                 MRV88E1340_REG_PAGE_0,
                                 MRV88E1340_SPECIFIC_CONTROL3_REG, reg_d, rc);
        buf_p = err_msg;
        DEV_ERROR_REPORT(dev, buf_p, MRVL_88E1340_PHONE_DETECT);  
        return (FAILED);
    }

    /* enable Auto Negotiation and reset phy */
    rc = SMIREAD(phy, phy_addr, MRV88E1340_REG_PAGE_0, 
                 MRV88E1340_CONTROL_REG, &reg_d);
    if (rc != PASSED) {
        buf_p += sprintf(buf_p, "%s(): phy smi read failed. phy_addr = "
                                "#%x, page = %#x, reg = %#x, rc = %#x\n",
                                 __FUNCTION__, phy_addr, 
                                 MRV88E1340_REG_PAGE_0,
                                 MRV88E1340_CONTROL_REG, rc);
        buf_p = err_msg;    
        DEV_ERROR_REPORT(dev, buf_p, MRVL_88E1340_PHONE_DETECT);
        return (FAILED);
    }
    reg_d |= MRV88E1340_AUTO_NEO_ENA;
    rc = SMIWRITE(phy, phy_addr, MRV88E1340_REG_PAGE_0, 
                  MRV88E1340_CONTROL_REG, reg_d);
    if (rc != PASSED) {
        buf_p += sprintf(buf_p, "%s(): phy smi write failed. "
                                "phy_addr = #%x, page = %#x, "
                                "reg = %#x, data = %#x, rc = %#x\n",
                                 __FUNCTION__, phy_addr, 
                                 MRV88E1340_REG_PAGE_0,
                                 MRV88E1340_CONTROL_REG, reg_d, rc);
        buf_p = err_msg;
        DEV_ERROR_REPORT(dev, buf_p, MRVL_88E1340_PHONE_DETECT);  
        return (FAILED);
    }
    
    rc = dev_88e1340_reset(dev, phy_addr);
    if (rc != PASSED) {
        buf_p += sprintf(buf_p, "%s(): phy addr %d reset failed. rc = %#x"
                                "\n", __FUNCTION__, phy_addr, rc);
        buf_p = err_msg;
        DEV_ERROR_REPORT(dev, buf_p, MRVL_88E1340_PHONE_DETECT);
        return (FAILED);
    }
    
    for (ix = MRVL_PHONE_DETECT_TIME; ix; ix--) {
        printf("\r%d seconds left", ix);
        msleep(1000);
    }

    /* read detection status register */
    rc = SMIREAD(phy, phy_addr, MRV88E1340_REG_PAGE_0, 
                 MRV88E1340_SPECIFIC_STATUS1_REG, &detected);
    if (rc != PASSED) {
        buf_p += sprintf(buf_p, "%s(): phy smi read failed. phy_addr = "
                                "#%x, page = %#x, reg = %#x, rc = %#x\n",
                                 __FUNCTION__, phy_addr, 
                                 MRV88E1340_REG_PAGE_0,
                                 MRV88E1340_SPECIFIC_STATUS1_REG, rc);
        buf_p = err_msg;    
        DEV_ERROR_REPORT(dev, buf_p, MRVL_88E1340_PHONE_DETECT);
        return (FAILED);
    }

    /* Disable power over Ethernet detection after detection */
    rc = SMIREAD(phy, phy_addr, MRV88E1340_REG_PAGE_0, 
                 MRV88E1340_SPECIFIC_CONTROL3_REG, &reg_d);
    if (rc != PASSED) {
        buf_p += sprintf(buf_p, "%s(): phy smi read failed. phy_addr = "
                                "#%x, page = %#x, reg = %#x, rc = %#x\n",
                                 __FUNCTION__, phy_addr, 
                                 MRV88E1340_REG_PAGE_0,
                                 MRV88E1340_SPECIFIC_CONTROL3_REG, rc);
        buf_p = err_msg;    
        DEV_ERROR_REPORT(dev, buf_p, MRVL_88E1340_PHONE_DETECT);
        return (FAILED);
    }
    
    reg_d &= ~MRV88E1340_P0_R26_DTE_DETECT;
    rc = SMIWRITE(phy, phy_addr, MRV88E1340_REG_PAGE_0, 
                  MRV88E1340_SPECIFIC_CONTROL3_REG, reg_d);
    if (rc != PASSED) {
        buf_p += sprintf(buf_p, "%s(): phy smi write failed. "
                                "phy_addr = #%x, page = %#x, "
                                "reg = %#x, data = %#x, rc = %#x\n",
                                 __FUNCTION__, phy_addr, 
                                 MRV88E1340_REG_PAGE_0,
                                 MRV88E1340_SPECIFIC_CONTROL3_REG, reg_d, rc);
        buf_p = err_msg;
        DEV_ERROR_REPORT(dev, buf_p, MRVL_88E1340_PHONE_DETECT); 
        return (FAILED);
    }

    /* Check detection result */
    if (detected & MRV88E1340_P0_R17_DTE_NEED_POWER) {
        return (PASSED);    /* Found Cisco PD */
    } else {
        return (FAILED);    /* PD not detected */
    }

}


/*******************************************************************************
 *
 * Function: dev_88e1340_cleanup_lpbk
 *
 * This function disable loopbacks and sets Marvell GE PHY
 * back into normal operating mode.
 *
 * Input:  inport   - input port number
 *         outport  - output port number
 *
 * Output: PASSED/FAILED
 *
 *******************************************************************************
 */
static int dev_88e1340_cleanup_lpbk (dev_object_t *dev, uint phy_addr)
{
    int retval, data, phy_speed, mac_speed;
    dev_88e1340_object_t *phy = (dev_88e1340_object_t *)dev;

    buf_p = err_msg;
    
    if (dev_88e1340_check_phy_addr(dev, phy_addr) == FAILED) {
        buf_p += sprintf(buf_p, "%s(): invalid phy addr (%d) found\n",
                                 __FUNCTION__, phy_addr);
        buf_p = err_msg;
        DEV_ERROR_REPORT(dev, buf_p, MRVL_88E1340_CLN_LPBK);
        return (FAILED);
    }

    retval = PASSED;
    phy_speed = MRV88E1340_SPD_SEL_1000M;
    mac_speed = MRV88E1340_MAC_SPD_1000M;

    /* restore PHY to 1Gbps, full duplex, auto-neg */
    /* Enable 1Gbps advertise (page 0, reg 9)*/
    retval = SMIREAD(phy, phy_addr, MRV88E1340_REG_PAGE_0, 
                     MRV88E1340_1000B_CNTL_REG, &data);
    if (retval == PASSED) {
        data |= MRV88E1340_1000BT_ADV;
        retval = SMIWRITE(phy, phy_addr, MRV88E1340_REG_PAGE_0, 
                          MRV88E1340_1000B_CNTL_REG, data);
        if (retval != PASSED) {
            buf_p += sprintf(buf_p, "%s(): Enable phy copper 1Gbps "
                                    "advertisement failed.\n", __FUNCTION__);
            buf_p = err_msg;
            DEV_ERROR_REPORT(dev, buf_p, MRVL_88E1340_CLN_LPBK);
            return (retval);
        }
    } else {
        buf_p += sprintf(buf_p, "%s(): phy smi read failed. phy_addr = "
                                "#%x, page = %#x, reg = %#x, rc = %#x\n",
                                 __FUNCTION__, phy_addr, 
                                 MRV88E1340_REG_PAGE_0,
                                 MRV88E1340_1000B_CNTL_REG, retval);
        buf_p = err_msg;    
        DEV_ERROR_REPORT(dev, buf_p, MRVL_88E1340_CLN_LPBK);
        return (retval);
    }

    /* Enable 10 & 100 Mbps advertise (page 0, reg 4)*/
    retval = SMIREAD(phy, phy_addr, MRV88E1340_REG_PAGE_0, 
                     MRV88E1340_AUTONEG_ADVR_REG, &data);
    if (retval == PASSED) {
        data |= MRV88E1340_100BT_ADV;
        data |= MRV88E1340_10BT_ADV;
        retval = SMIWRITE(phy, phy_addr, MRV88E1340_REG_PAGE_0, 
                          MRV88E1340_AUTONEG_ADVR_REG, data);
        if (retval != PASSED) {
            buf_p += sprintf(buf_p, "%s(): Enable phy copper 10&100Mbps "
                                    "advertisement failed.\n", __FUNCTION__);
            buf_p = err_msg;
            DEV_ERROR_REPORT(dev, buf_p, MRVL_88E1340_CLN_LPBK);
            return (retval);
        }
    } else {
        buf_p += sprintf(buf_p, "%s(): phy smi read failed. phy_addr = "
                                "#%x, page = %#x, reg = %#x, rc = %#x\n",
                                 __FUNCTION__, phy_addr, 
                                 MRV88E1340_REG_PAGE_0,
                                 MRV88E1340_AUTONEG_ADVR_REG, retval);
        buf_p = err_msg;    
        DEV_ERROR_REPORT(dev, buf_p, MRVL_88E1340_CLN_LPBK);
            return (retval);
    }

    /* config phy speed 1Gpbs for SGMII (page 2, reg 21) */
    retval = SMIREAD(phy, phy_addr, MRV88E1340_REG_PAGE_2, 
                     MRV88E1340_MAC_CNTL_REG2, &data);
    if (retval == PASSED) {
        data &= ~MRV88E1340_MAC_SPD_MASK;
        data |= mac_speed;
        retval = SMIWRITE(phy, phy_addr, MRV88E1340_REG_PAGE_2, 
                          MRV88E1340_MAC_CNTL_REG2, data);
        if (retval != PASSED) {
            buf_p += sprintf(buf_p, "%s(): Restore phy mac speed 1Gbps "
                                    "failed.\n", __FUNCTION__);
            buf_p = err_msg;
            DEV_ERROR_REPORT(dev, buf_p, MRVL_88E1340_CLN_LPBK);
            return (retval);
        }
    } else {
        buf_p += sprintf(buf_p, "%s(): phy smi read failed. phy_addr = "
                                "#%x, page = %#x, reg = %#x, rc = %#x\n",
                                 __FUNCTION__, phy_addr, 
                                 MRV88E1340_REG_PAGE_2,
                                 MRV88E1340_MAC_CNTL_REG2, retval);
        buf_p = err_msg;    
        DEV_ERROR_REPORT(dev, buf_p, MRVL_88E1340_CLN_LPBK);
        return (retval);
    }

    /* set speed 1Gbps & auto-neg & full-duplex of page 0, reg 0*/
    retval = SMIREAD(phy, phy_addr, MRV88E1340_REG_PAGE_0, 
                     MRV88E1340_CONTROL_REG, &data);
    if (retval == PASSED) {
        data &= ~MRV88E1340_SPD_SEL_MASK;
        data |= phy_speed | MRV88E1340_AUTO_NEO_ENA | MRV88E1340_FULL_DUPLEX;
        retval = SMIWRITE(phy, phy_addr, MRV88E1340_REG_PAGE_0, 
                          MRV88E1340_CONTROL_REG, data);
        if (retval != PASSED) {
            buf_p += sprintf(buf_p, "%s(): set phy copper speed "
                                    "failed.\n", __FUNCTION__);
            buf_p = err_msg;
            DEV_ERROR_REPORT(dev, buf_p, MRVL_88E1340_CLN_LPBK);
            return (retval);
        }
    } else {
        buf_p += sprintf(buf_p, "%s(): phy smi read failed. phy_addr = "
                                "#%x, page = %#x, reg = %#x, rc = %#x\n",
                                 __FUNCTION__, phy_addr, 
                                 MRV88E1340_REG_PAGE_0,
                                 MRV88E1340_CONTROL_REG, retval);
        buf_p = err_msg;    
        DEV_ERROR_REPORT(dev, buf_p, MRVL_88E1340_CLN_LPBK);
        return (retval);
    }


    /* clear 1000BT PHY External loopback mode */
    retval = SMIREAD(phy, phy_addr, MRV88E1340_REG_PAGE_6, 
                     MRV88E1340_P6_CHECKER_CTRL, &data);
    if (retval == PASSED) {
        data &= ~MRV88E1340_P6_R18_ENA_STUB_TEST;
        retval = SMIWRITE(phy, phy_addr, MRV88E1340_REG_PAGE_6,
                          MRV88E1340_P6_CHECKER_CTRL, data);
        if (retval != PASSED) {
            buf_p += sprintf(buf_p, "%s(): Clear phy 1000BT external "
                                    "failed.\n", __FUNCTION__);
            buf_p = err_msg;
            DEV_ERROR_REPORT(dev, buf_p, MRVL_88E1340_CLN_LPBK);
            return (retval); 
        }
    } else {
        buf_p += sprintf(buf_p, "%s(): phy smi read failed. phy_addr = "
                                "#%x, page = %#x, reg = %#x, rc = %#x\n",
                                 __FUNCTION__, phy_addr, 
                                 MRV88E1340_REG_PAGE_6,
                                 MRV88E1340_P6_CHECKER_CTRL, retval);
        buf_p = err_msg;    
        DEV_ERROR_REPORT(dev, buf_p, MRVL_88E1340_CLN_LPBK);
        return (retval);
    }

    /* clear PHY loopback mode (bit 14, page 0, reg 0) */
    retval = SMIREAD(phy, phy_addr, MRV88E1340_REG_PAGE_0,
                     MRV88E1340_CONTROL_REG, &data);
    if (retval == PASSED) {
        data &= ~MRV88E1340_LPBK_ENA;
        retval = SMIWRITE(phy, phy_addr, MRV88E1340_REG_PAGE_0, 
                          MRV88E1340_CONTROL_REG, data);
        if (retval != PASSED) {
            buf_p += sprintf(buf_p, "%s(): clear phy loopback mode "
                                    "failed.\n", __FUNCTION__);
            buf_p = err_msg;
            DEV_ERROR_REPORT(dev, buf_p, MRVL_88E1340_CLN_LPBK);
            return (retval);
        }
    } else {
        buf_p += sprintf(buf_p, "%s(): phy smi read failed. phy_addr = "
                                "#%x, page = %#x, reg = %#x, rc = %#x\n",
                                 __FUNCTION__, phy_addr, 
                                 MRV88E1340_REG_PAGE_0,
                                 MRV88E1340_CONTROL_REG, retval);
        buf_p = err_msg;    
        DEV_ERROR_REPORT(dev, buf_p, MRVL_88E1340_CLN_LPBK);
        return (retval);
    }

    retval = dev_88e1340_reset(dev, phy_addr);
    if (retval != PASSED) {
        buf_p += sprintf(buf_p, "%s(): phy addr %d reset failed. rc = %#x"
                                "\n", __FUNCTION__, phy_addr, retval);
        buf_p = err_msg;
        DEV_ERROR_REPORT(dev, buf_p, MRVL_88E1340_CLN_LPBK);
        return (FAILED);
    }
    
    return (retval);
}

/*******************************************************************************
 *
 * Function: dev_88e1340_lpbk_mode
 *
 * This function set PHY loopback mode and speed.
 *
 * Input: dev - Pointer to the Marvell GE device object
 *        phy_addr 
 *        enable
 *        
 * Output: PASSED/FAILED
 *
 *******************************************************************************
 */
static int dev_88e1340_lpbk_mode (dev_object_t *dev, int phy_addr, int enable)
{
    int retval, data;
    dev_88e1340_object_t *phy = (dev_88e1340_object_t *)dev;

    buf_p = err_msg;
    retval = PASSED;
    
    if (dev_88e1340_check_phy_addr(dev, phy_addr) == FAILED) {
        buf_p += sprintf(buf_p, "%s(): invalid phy addr (%d) found\n",
                                 __FUNCTION__, phy_addr);
        buf_p = err_msg;
        DEV_ERROR_REPORT(dev, buf_p, MRVL_88E1340_SET_LPBK);
        return (FAILED);
    }

    /* config PHY loopback mode (bit 14, page 0, reg 0) */
    retval = SMIREAD(phy, phy_addr, MRV88E1340_REG_PAGE_0,
                     MRV88E1340_CONTROL_REG, &data);
    if (retval == PASSED) {
        if (enable != ENABLE) {
            data &= ~MRV88E1340_LPBK_ENA;
        } else {
            data |= MRV88E1340_LPBK_ENA;
        }

        retval = SMIWRITE(phy, phy_addr, MRV88E1340_REG_PAGE_0, 
                          MRV88E1340_CONTROL_REG, data);
        if (retval != PASSED) {
            buf_p += sprintf(buf_p, "%s(): set phy lpbk mode failed."
                                    "\n", __FUNCTION__);
            buf_p = err_msg;
            DEV_ERROR_REPORT(dev, buf_p, MRVL_88E1340_SET_LPBK);
            return (retval);
        }
    } else {
        buf_p += sprintf(buf_p, "%s(): phy smi read failed. phy_addr = "
                                "#%x, page = %#x, reg = %#x, rc = %#x\n",
                                 __FUNCTION__, phy_addr, 
                                 MRV88E1340_REG_PAGE_0,
                                 MRV88E1340_CONTROL_REG, retval);
        buf_p = err_msg;    
        DEV_ERROR_REPORT(dev, buf_p, MRVL_88E1340_SET_LPBK);
        return (retval);
    }

    return (retval);
}

/*******************************************************************************
 *
 * Function: dev_88e1340_set_lpbk
 *
 * This function set PHY loopback mode and speed.
 *
 * Input: dev - Pointer to the Marvell GE device object
 *        phy_addr 
 *        speed - 10/100/1G
 *        lpbk  - loopback mode
 *
 * Output: PASSED/FAILED
 *
 *******************************************************************************
 */
static int dev_88e1340_set_lpbk (dev_object_t *dev, int phy_addr, int speed, 
                                 int lpbk)
{
    int retval, data, phy_speed = -1, mac_speed = -1;
    dev_88e1340_object_t *phy = (dev_88e1340_object_t *)dev;

    buf_p = err_msg;
    
    if (dev_88e1340_check_phy_addr(dev, phy_addr) == FAILED) {
        buf_p += sprintf(buf_p, "%s(): invalid phy addr (%d) found\n",
                                 __FUNCTION__, phy_addr);
        buf_p = err_msg;
        DEV_ERROR_REPORT(dev, buf_p, MRVL_88E1340_SET_LPBK);
        return (FAILED);
    }

    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("setup phy device, phy_addr %d, speed %d, loopback %d\n",
                phy_addr, speed, lpbk);
    }

    retval = PASSED;

    switch (speed) {
    case ETH_MODE_GE:
        phy_speed = MRV88E1340_SPD_SEL_1000M;
        mac_speed = MRV88E1340_MAC_SPD_1000M;
        break;
    case ETH_MODE_FE100:
        phy_speed = MRV88E1340_SPD_SEL_100M;
        mac_speed = MRV88E1340_MAC_SPD_100M;
        break;
    case ETH_MODE_FE10:
        phy_speed = MRV88E1340_SPD_SEL_10M;
        mac_speed = MRV88E1340_MAC_SPD_10M;
        break;
    default:
        buf_p += sprintf(buf_p, "%s(): unknown speed\n", __FUNCTION__);
        buf_p = err_msg;
        DEV_ERROR_REPORT(dev, buf_p, MRVL_88E1340_SET_LPBK);
        return (FAILED);
    }

    /* config 1000BT PHY External loopback mode */
    if (lpbk == SGMII_LPBK_NONE && speed == ETH_MODE_GE) {
        retval = SMIREAD(phy, phy_addr, MRV88E1340_REG_PAGE_6, 
                         MRV88E1340_P6_CHECKER_CTRL, &data);

        if (retval == PASSED) {
            data |= MRV88E1340_P6_R18_ENA_STUB_TEST;
            retval = SMIWRITE(phy, phy_addr, MRV88E1340_REG_PAGE_6,
                              MRV88E1340_P6_CHECKER_CTRL, data);
            if (retval != PASSED) {
                buf_p += sprintf(buf_p, "%s(): Set phy 1000BT external "
                                        "failed.\n", __FUNCTION__);
                buf_p = err_msg;
                DEV_ERROR_REPORT(dev, buf_p, MRVL_88E1340_SET_LPBK);
                return (retval); 
            }
        } else {
            buf_p += sprintf(buf_p, "%s(): phy smi read failed. phy_addr = "
                                    "#%x, page = %#x, reg = %#x, rc = %#x\n",
                                     __FUNCTION__, phy_addr, 
                                     MRV88E1340_REG_PAGE_6,
                                     MRV88E1340_P6_CHECKER_CTRL, retval);
            buf_p = err_msg;    
            DEV_ERROR_REPORT(dev, buf_p, MRVL_88E1340_SET_LPBK);
            return (retval);
        }
    } else {
        retval = SMIREAD(phy, phy_addr, MRV88E1340_REG_PAGE_6, 
                         MRV88E1340_P6_CHECKER_CTRL, &data);
        if (retval == PASSED) {
            data &= ~MRV88E1340_P6_R18_ENA_STUB_TEST;
            retval = SMIWRITE(phy, phy_addr, MRV88E1340_REG_PAGE_6,
                              MRV88E1340_P6_CHECKER_CTRL, data);
            if (retval != PASSED) {
                buf_p += sprintf(buf_p, "%s(): Clear phy 1000BT external"
                                        " failed.\n", __FUNCTION__);
                buf_p = err_msg;
                DEV_ERROR_REPORT(dev, buf_p, MRVL_88E1340_SET_LPBK);
                return (retval); 
            }
        } else {
            buf_p += sprintf(buf_p, "%s(): phy smi read failed. phy_addr = "
                                    "#%x, page = %#x, reg = %#x, rc = %#x\n",
                                     __FUNCTION__, phy_addr, 
                                     MRV88E1340_REG_PAGE_6,
                                     MRV88E1340_P6_CHECKER_CTRL, retval);
            buf_p = err_msg;    
            DEV_ERROR_REPORT(dev, buf_p, MRVL_88E1340_SET_LPBK);
            return (retval);
        }
    }

    if (lpbk == SGMII_LPBK_NONE) { /* PHY External loopback test */
        /* config phy copper link speed for external loopback */
        switch (speed) {
        case ETH_MODE_GE:
            /* Enable 1Gbps advertise (page 0, reg 9)*/
            retval = SMIREAD(phy, phy_addr, MRV88E1340_REG_PAGE_0, 
                             MRV88E1340_1000B_CNTL_REG, &data);
            if (retval == PASSED) {
                data |= MRV88E1340_1000BT_ADV;
                retval = SMIWRITE(phy, phy_addr, MRV88E1340_REG_PAGE_0, 
                                  MRV88E1340_1000B_CNTL_REG, data);
                if (retval != PASSED) {
                    buf_p += sprintf(buf_p, "%s(): Enable phy copper "
                                            "speed 1Gbps failed.\n", 
                                             __FUNCTION__);
                    buf_p = err_msg;
                    DEV_ERROR_REPORT(dev, buf_p, MRVL_88E1340_SET_LPBK);
                    return (retval);
                }
            } else {
                buf_p += sprintf(buf_p, "%s(): phy smi read failed. phy_addr = "
                                        "#%x, page = %#x, reg = %#x, rc = %#x\n",
                                         __FUNCTION__, phy_addr, 
                                         MRV88E1340_REG_PAGE_0,
                                         MRV88E1340_1000B_CNTL_REG, retval);
                buf_p = err_msg;    
                DEV_ERROR_REPORT(dev, buf_p, MRVL_88E1340_SET_LPBK);
                return (retval);
            }
            break;
   
        case ETH_MODE_FE100:
            /* Disable 1Gbps advertise (page 0, reg 9)*/
            retval = SMIREAD(phy, phy_addr, MRV88E1340_REG_PAGE_0, 
                             MRV88E1340_1000B_CNTL_REG, &data);
            if (retval == PASSED) {
                data &= ~MRV88E1340_1000BT_ADV;
                retval = SMIWRITE(phy, phy_addr, MRV88E1340_REG_PAGE_0, 
                                  MRV88E1340_1000B_CNTL_REG, data);
                if (retval != PASSED) {
                    buf_p += sprintf(buf_p, "%s(): Disable phy copper "
                                            "speed 1Gbps failed.\n", 
                                             __FUNCTION__);
                    buf_p = err_msg;
                    DEV_ERROR_REPORT(dev, buf_p, MRVL_88E1340_SET_LPBK);
                    return (retval);
                }
            } else {
                buf_p += sprintf(buf_p, "%s(): phy smi read failed. phy_addr = "
                                        "#%x, page = %#x, reg = %#x, rc = %#x\n",
                                         __FUNCTION__, phy_addr, 
                                         MRV88E1340_REG_PAGE_0,
                                         MRV88E1340_1000B_CNTL_REG, retval);
                buf_p = err_msg;    
                DEV_ERROR_REPORT(dev, buf_p, MRVL_88E1340_SET_LPBK);
                return (retval);
            }
   
            /* Enable 100Mbps advertise (page 0, reg 4)*/
            retval = SMIREAD(phy, phy_addr, MRV88E1340_REG_PAGE_0, 
                             MRV88E1340_AUTONEG_ADVR_REG, &data);
            if (retval == PASSED) {
                data |= MRV88E1340_100BT_ADV;
                retval = SMIWRITE(phy, phy_addr, MRV88E1340_REG_PAGE_0, 
                                  MRV88E1340_AUTONEG_ADVR_REG, data);
                if (retval != PASSED) {
                    buf_p += sprintf(buf_p, "%s(): Enable phy copper "
                                            "speed 100Mbps failed.\n",
                                             __FUNCTION__);
                    buf_p = err_msg;
                    DEV_ERROR_REPORT(dev, buf_p, MRVL_88E1340_SET_LPBK);
                    return (retval);
                }
            } else {
                buf_p += sprintf(buf_p, "%s(): phy smi read failed. phy_addr = "
                                        "#%x, page = %#x, reg = %#x, rc = %#x\n",
                                         __FUNCTION__, phy_addr, 
                                         MRV88E1340_REG_PAGE_0,
                                         MRV88E1340_AUTONEG_ADVR_REG, retval);
                buf_p = err_msg;    
                DEV_ERROR_REPORT(dev, buf_p, MRVL_88E1340_SET_LPBK);
                return (retval);
            }
            break;
   
        case ETH_MODE_FE10:
            /* Disable 1Gbps advertise (page 0, reg 9)*/
            retval = SMIREAD(phy, phy_addr, MRV88E1340_REG_PAGE_0, 
                             MRV88E1340_1000B_CNTL_REG, &data);
            if (retval == PASSED) {
                data &= ~MRV88E1340_1000BT_ADV;
                retval = SMIWRITE(phy, phy_addr, MRV88E1340_REG_PAGE_0, 
                                  MRV88E1340_1000B_CNTL_REG, data);
                if (retval != PASSED) {
                    buf_p += sprintf(buf_p, "%s(): Disable phy copper speed "
                                            "1Gbps failed.\n", __FUNCTION__);
                    buf_p = err_msg;
                    DEV_ERROR_REPORT(dev, buf_p, MRVL_88E1340_SET_LPBK);
                    return (retval);
                }
            } else {
                buf_p += sprintf(buf_p, "%s(): phy smi read failed. phy_addr = "
                                        "#%x, page = %#x, reg = %#x, rc = %#x\n",
                                         __FUNCTION__, phy_addr, 
                                         MRV88E1340_REG_PAGE_0,
                                         MRV88E1340_1000B_CNTL_REG, retval);
                buf_p = err_msg;    
                DEV_ERROR_REPORT(dev, buf_p, MRVL_88E1340_SET_LPBK);
                return (retval);
            }
   
            /* Disable 100Mbps & Enable 10Mbps advertise (page 0, reg 4)*/
            retval = SMIREAD(phy, phy_addr, MRV88E1340_REG_PAGE_0, 
                             MRV88E1340_AUTONEG_ADVR_REG, &data);
            if (retval == PASSED) {
                data &= ~MRV88E1340_100BT_ADV;
                data |= MRV88E1340_10BT_ADV;
                retval = SMIWRITE(phy, phy_addr, MRV88E1340_REG_PAGE_0, 
                                  MRV88E1340_AUTONEG_ADVR_REG, data);
                if (retval != PASSED) {
                    buf_p += sprintf(buf_p, "%s(): set phy copper speed "
                                            "10Mbps failed.\n", __FUNCTION__);
                    buf_p = err_msg;
                    DEV_ERROR_REPORT(dev, buf_p, MRVL_88E1340_SET_LPBK);
                    return (retval);
                }
            } else {
                buf_p += sprintf(buf_p, "%s(): phy smi read failed. phy_addr = "
                                        "#%x, page = %#x, reg = %#x, rc = %#x\n",
                                         __FUNCTION__, phy_addr, 
                                         MRV88E1340_REG_PAGE_0,
                                         MRV88E1340_AUTONEG_ADVR_REG, retval);
                buf_p = err_msg;    
                DEV_ERROR_REPORT(dev, buf_p, MRVL_88E1340_SET_LPBK);
                return (retval);
            }
            break;
   
        default:
            buf_p += sprintf(buf_p, "%s(): unknown speed\n", __FUNCTION__);
            buf_p = err_msg;
            DEV_ERROR_REPORT(dev, buf_p, MRVL_88E1340_SET_LPBK);
            return (FAILED);
        }
    } else { /* PHY internal loopback test */
        /* config phy speed for SGMII (page 2, reg 21) */
        retval = SMIREAD(phy, phy_addr, MRV88E1340_REG_PAGE_2, 
                         MRV88E1340_MAC_CNTL_REG2, &data);
        if (retval == PASSED) {
            data &= ~MRV88E1340_MAC_SPD_MASK;
            data |= mac_speed;
            retval = SMIWRITE(phy, phy_addr, MRV88E1340_REG_PAGE_2, 
                              MRV88E1340_MAC_CNTL_REG2, data);
            if (retval != PASSED) {
                buf_p += sprintf(buf_p, "%s(): set phy mac speed failed.\n",
                                         __FUNCTION__);
                buf_p = err_msg;
                DEV_ERROR_REPORT(dev, buf_p, MRVL_88E1340_SET_LPBK);
                return (retval);
            }
        } else {
            buf_p += sprintf(buf_p, "%s(): phy smi read failed. phy_addr = "
                                    "#%x, page = %#x, reg = %#x, rc = %#x\n",
                                     __FUNCTION__, phy_addr, 
                                     MRV88E1340_REG_PAGE_2,
                                     MRV88E1340_MAC_CNTL_REG2, retval);
            buf_p = err_msg;    
            DEV_ERROR_REPORT(dev, buf_p, MRVL_88E1340_SET_LPBK);
            return (retval);
        }

        /* set speed of page 0, reg 0*/
        retval = SMIREAD(phy, phy_addr, MRV88E1340_REG_PAGE_0, 
                         MRV88E1340_CONTROL_REG, &data);
        if (retval == PASSED) {
            data &= ~MRV88E1340_SPD_SEL_MASK;
            data |= phy_speed;
            retval = SMIWRITE(phy, phy_addr, MRV88E1340_REG_PAGE_0, 
                              MRV88E1340_CONTROL_REG, data);
            if (retval != PASSED) {
               buf_p += sprintf(buf_p, "%s(): set phy copper speed "
                                       "failed.\n", __FUNCTION__);
               buf_p = err_msg;
               DEV_ERROR_REPORT(dev, buf_p, MRVL_88E1340_SET_LPBK);
               return (retval);
            }
        } else {
            buf_p += sprintf(buf_p, "%s(): phy smi read failed. phy_addr = "
                                    "#%x, page = %#x, reg = %#x, rc = %#x\n",
                                     __FUNCTION__, phy_addr, 
                                     MRV88E1340_REG_PAGE_0,
                                     MRV88E1340_CONTROL_REG, retval);
            buf_p = err_msg;    
            DEV_ERROR_REPORT(dev, buf_p, MRVL_88E1340_SET_LPBK);
            return (retval);
        }
    }
    
    retval = dev_88e1340_reset(dev, phy_addr);
    if (retval != PASSED) {
        buf_p += sprintf(buf_p, "%s(): phy addr %d reset failed. rc = %#x"
                                "\n", __FUNCTION__, phy_addr, retval);
        buf_p = err_msg;
        DEV_ERROR_REPORT(dev, buf_p, MRVL_88E1340_SET_LPBK);
        return (FAILED);
    }
        
    /* config PHY loopback mode (bit 14, page 0, reg 0) */
    retval = SMIREAD(phy, phy_addr, MRV88E1340_REG_PAGE_0,
                     MRV88E1340_CONTROL_REG, &data);
    if (retval == PASSED) {
        if (lpbk == SGMII_LPBK_NONE) {
            data &= ~MRV88E1340_LPBK_ENA;
        } else {
            data |= MRV88E1340_LPBK_ENA;
        }

        retval = SMIWRITE(phy, phy_addr, MRV88E1340_REG_PAGE_0, 
                          MRV88E1340_CONTROL_REG, data);
        if (retval != PASSED) {
            buf_p += sprintf(buf_p, "%s(): set phy loopback mode failed.\n",
                                     __FUNCTION__);
            buf_p = err_msg;
            DEV_ERROR_REPORT(dev, buf_p, MRVL_88E1340_SET_LPBK);
            return (retval);
        }
    } else {
        buf_p += sprintf(buf_p, "%s(): phy smi read failed. phy_addr = "
                                "#%x, page = %#x, reg = %#x, rc = %#x\n",
                                 __FUNCTION__, phy_addr, 
                                 MRV88E1340_REG_PAGE_0,
                                 MRV88E1340_CONTROL_REG, retval);
        buf_p = err_msg;    
        DEV_ERROR_REPORT(dev, buf_p, MRVL_88E1340_SET_LPBK);
        return (retval);
    }

    msleep(2000);  /* Wait for link up */

    if (lpbk == SGMII_LPBK_NONE) { 
        /* Check copper link speed (page 0, reg 17)*/
        retval = SMIREAD(phy, phy_addr, MRV88E1340_REG_PAGE_0,
                         MRV88E1340_SPECIFIC_STATUS1_REG, &data);    
        if (retval != PASSED) {
            buf_p += sprintf(buf_p, "%s(): phy smi read failed. phy_addr = "
                                    "#%x, page = %#x, reg = %#x, rc = %#x\n",
                                     __FUNCTION__, phy_addr, 
                                     MRV88E1340_REG_PAGE_0,
                                     MRV88E1340_SPECIFIC_STATUS1_REG, retval);
            buf_p = err_msg;    
            DEV_ERROR_REPORT(dev, buf_p, MRVL_88E1340_SET_LPBK);
            return (retval);
        }    
        
        if (!(data & MRV88E1340_LINK_UP)) {
            if ((NVRAM)->diagflag & D_VERBOSE) {
                printf("%s(): Copper is NOT link up.\n", __FUNCTION__);
            }
        }
    
        switch (speed) {
        case ETH_MODE_GE:
            if ((data & MRV88E1340_LINK_SPEED_MASK) != 
                MRV88E1340_LINK_SPEED_1000) {
                buf_p += sprintf(buf_p, "%s(): Copper link speed is NOT "
                                        "1Gbps.\n", __FUNCTION__);
                buf_p = err_msg;
                DEV_ERROR_REPORT(dev, buf_p, MRVL_88E1340_SET_LPBK);
                return (FAILED);
            }
            break;
        case ETH_MODE_FE100:
            if ((data & MRV88E1340_LINK_SPEED_MASK) != 
                MRV88E1340_LINK_SPEED_100) {
                buf_p += sprintf(buf_p, "%s(): Copper link speed is NOT "
                                        "100Mbps.\n", __FUNCTION__);
                buf_p = err_msg;
                DEV_ERROR_REPORT(dev, buf_p, MRVL_88E1340_SET_LPBK);
                return (FAILED);
            }
            break;
        case ETH_MODE_FE10:
            if ((data & MRV88E1340_LINK_SPEED_MASK) != 
                MRV88E1340_LINK_SPEED_10) {
                buf_p += sprintf(buf_p, "%s(): Copper link speed is NOT "
                                        "10Mbps.\n", __FUNCTION__);
                buf_p = err_msg;
                DEV_ERROR_REPORT(dev, buf_p, MRVL_88E1340_SET_LPBK);
                return (FAILED);
            }
            break;
        default:
            buf_p += sprintf(buf_p, "%s(): unknown link speed\n", __FUNCTION__);
            buf_p = err_msg;
            DEV_ERROR_REPORT(dev, buf_p, MRVL_88E1340_SET_LPBK);
            return (FAILED);
        }
    }

    /* Check MAC Side Link up and Sync (page 1, reg 17)*/
    retval = SMIREAD(phy, phy_addr, MRV88E1340_REG_PAGE_1,
                     MRV88E1340_SPECIFIC_STATUS1_REG, &data);
    if (retval != PASSED) {
        buf_p += sprintf(buf_p, "%s(): phy smi read failed. phy_addr = "
                                "#%x, page = %#x, reg = %#x, rc = %#x\n",
                                 __FUNCTION__, phy_addr, 
                                 MRV88E1340_REG_PAGE_1,
                                 MRV88E1340_SPECIFIC_STATUS1_REG, retval);
        buf_p = err_msg;    
        DEV_ERROR_REPORT(dev, buf_p, MRVL_88E1340_SET_LPBK);
        return (retval);
    }

    if ((NVRAM)->diagflag & D_VERBOSE) {
        if (data & MRV88E1340_LINK_UP) {
            printf("%s(): MAC is link up.\n", __FUNCTION__);
        } else {
            printf("%s(): MAC is NOT link up.\n", __FUNCTION__);
        }
    }
    if (data & MRV88E1340_SYNC) {
        if ((NVRAM)->diagflag & D_VERBOSE) {
            printf("MAC is Sync.\n");
        }
    } else {
        buf_p += sprintf(buf_p, "%s(): MAC is NOT Sync.\n", __FUNCTION__);
        buf_p = err_msg;
        DEV_ERROR_REPORT(dev, buf_p, MRVL_88E1340_SET_LPBK);
        return (FAILED);
    }

    switch (speed) {
    case ETH_MODE_GE:
        if ((data & MRV88E1340_LINK_SPEED_MASK) != MRV88E1340_LINK_SPEED_1000) {
            buf_p += sprintf(buf_p, "%s(): MAC link speed is NOT 1Gbps.\n",
                                     __FUNCTION__);
            buf_p = err_msg;
            DEV_ERROR_REPORT(dev, buf_p, MRVL_88E1340_SET_LPBK);
            return (FAILED);
        }
        break;
    case ETH_MODE_FE100:
        if ((data & MRV88E1340_LINK_SPEED_MASK) != MRV88E1340_LINK_SPEED_100) {
            buf_p += sprintf(buf_p, "%s(): MAC link speed is NOT 100Mbps.\n",
                                     __FUNCTION__);
            buf_p = err_msg;
            DEV_ERROR_REPORT(dev, buf_p, MRVL_88E1340_SET_LPBK);
            return (FAILED);
        }
        break;
    case ETH_MODE_FE10:
        if ((data & MRV88E1340_LINK_SPEED_MASK) != MRV88E1340_LINK_SPEED_10) {
            buf_p += sprintf(buf_p, "%s(): MAC link speed is NOT 10Mbps.\n",
                                     __FUNCTION__);
            buf_p = err_msg;
            DEV_ERROR_REPORT(dev, buf_p, MRVL_88E1340_SET_LPBK);
            return (FAILED);
        }
        break;
    default:
        buf_p += sprintf(buf_p, "%s(): unknown MAC link speed\n", __FUNCTION__);
        buf_p = err_msg;
        DEV_ERROR_REPORT(dev, buf_p, MRVL_88E1340_SET_LPBK);
        return (FAILED);
    }

    return (retval);
}


/*******************************************************************************
 *
 * Function: phy_register_tests
 *
 * For each register from reg_ptr, this function checks for accessibility
 * and does a ripple 1 and a ripple 0 test if applicable (not all registers
 * are W/R register).
 *
 * Input : interface structure pointer, info for all registers
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
    dev_88e1340_object_t *phy = (dev_88e1340_object_t *)dev;

    readval = 0;
    retval = PASSED;
    ret_val = PASSED;

    /* Set Page */
    retval = SMIWRITE(phy, phy_addr, page, MRV88E1340_PAGE_ADDRESS_REG, page);
    if (retval == FAILED) {
    	buf_p += sprintf(buf_p, "Switch page fail %x\n", page);
    	return (FAILED);
    }

    while (reg_ptr->size.size != 0) {
        retval = SMIREAD(phy, phy_addr, page, reg_ptr->offset, &save_val);
        if (retval == FAILED) {
            buf_p += sprintf(buf_p, "%s(): Error reading %s register "
                                    "offset %#x, base_addr %#x, "
                                    "phy_addr %d\n", __FUNCTION__, 
                                     reg_ptr->name, reg_ptr->offset, 
                                     phy->base_phyaddr, phy_addr);
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
                    ret_val = SMIREAD(phy, phy_addr, page, tst_offset, &readval);
                }
                if (((readval & reg_ptr->mask) != temp) ||
                    (retval == FAILED) || (ret_val == FAILED)) {
                    buf_p += sprintf(buf_p, "%s(): Ripple one test "
                                            "failed when accessing %s "
                                            "Register offset %#x, "
                                            "phy_addr %d, base_addr %#x,"
                                            " Expect %#x, Read %#x",
                                             __FUNCTION__, reg_ptr->name,
                                             tst_offset, phy_addr, 
                                             phy->base_phyaddr, temp, 
                                             readval);
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
                    buf_p += sprintf(buf_p, "%s(): Ripple one test "
                                            "failed when accessing %s "
                                            "Register offset %#x, "
                                            "phy_addr %d, "
                                            "base_addr %#x, "
                                            "Expect %#x, Read %#x",
                                             __FUNCTION__, reg_ptr->name,
                                             tst_offset, phy_addr, 
                                             phy->base_phyaddr, temp,
                                             readval);
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
                    buf_p += sprintf(buf_p, "%s(): Pattern test failed "
                                            "when accessing %s Register "
                                            "offset %#x phy_addr %d, "
                                            "base_addr %#x, Expect %#x, "
                                            "Read %#x", __FUNCTION__,
                                             reg_ptr->name, tst_offset, 
                                             phy_addr,
                                             phy->base_phyaddr, temp,
                                             readval);
                    return (retval);
                }
    
                data = ~PATTERN; /* complement data pattern */
            }

            /*
             * restore original value
             */

            retval = SMIWRITE(phy, phy_addr, page, tst_offset, save_val);

            if (retval == FAILED) {
                buf_p += sprintf(buf_p, "%s(): Error restoring %s register "
                                        "offset %#x, base_addr %#x, "
                                        "phy_addr %d\n", __FUNCTION__, 
                                         reg_ptr->name, reg_ptr->offset, 
                                         phy->base_phyaddr, phy_addr);
                return (FAILED);
            }
        }
        reg_ptr++;
    }

    return (PASSED);
}


/*******************************************************************************
 *
 * Function: dev_88e1340_reg_test_single.
 *
 * This function implements the PHY registers test of single phy_addr.
 *
 * Input:  dev_object_t pointer to the Marvell GE device.
 *
 * Output: PASSED/FAILED.
 *
 *******************************************************************************
 */
static int dev_88e1340_reg_test_single (dev_object_t *dev, uint phy_addr)
{
    int ret = PASSED;

    buf_p = err_msg;

    if (phy_register_tests(dev, phy_addr, MRV88E1340_REG_PAGE_0,
                           &marvell_88e1340_reg_page0[0]) == FAILED) {
        buf_p += sprintf(buf_p, "%s(): PHY Reg Test Failed: PHY addr %d "
                                "on Page 0.\n", __FUNCTION__, phy_addr);
        DEV_ERROR_REPORT(dev, err_msg, MRVL_88E1340_REG_TEST);
        return (FAILED);
    }

    if (phy_register_tests(dev, phy_addr, MRV88E1340_REG_PAGE_1,
                           &marvell_88e1340_reg_page1[0]) == FAILED) {
        buf_p += sprintf(buf_p, "%s(): PHY Reg Test Failed: PHY addr %d "
                                "on Page 1.\n", __FUNCTION__, phy_addr);
        DEV_ERROR_REPORT(dev, err_msg, MRVL_88E1340_REG_TEST);
        return (FAILED);
    }

    if (phy_register_tests(dev, phy_addr, MRV88E1340_REG_PAGE_2,
                           &marvell_88e1340_reg_page2[0]) == FAILED) {
        buf_p += sprintf(buf_p, "%s(): PHY Reg Test Failed: PHY addr %d "
                                "on Page 2.\n", __FUNCTION__, phy_addr);
        DEV_ERROR_REPORT(dev, err_msg, MRVL_88E1340_REG_TEST);
        return (FAILED);
    }

    if (phy_register_tests(dev, phy_addr, MRV88E1340_REG_PAGE_3,
                           &marvell_88e1340_reg_page3[0]) == FAILED) {
        buf_p += sprintf(buf_p, "%s(): PHY Reg Test Failed: PHY addr %d "
                                "on Page 3.\n", __FUNCTION__, phy_addr);
        DEV_ERROR_REPORT(dev, err_msg, MRVL_88E1340_REG_TEST);
        return (FAILED);
    }

    if (phy_register_tests(dev, phy_addr, MRV88E1340_REG_PAGE_4,
                           &marvell_88e1340_reg_page4[0]) == FAILED) {
        buf_p += sprintf(buf_p, "%s(): PHY Reg Test Failed: PHY addr %d "
                                "on Page 4.\n", __FUNCTION__, phy_addr);
        DEV_ERROR_REPORT(dev, err_msg, MRVL_88E1340_REG_TEST);
        return (FAILED);
    }

    if (phy_register_tests(dev, phy_addr, MRV88E1340_REG_PAGE_5,
                           &marvell_88e1340_reg_page5[0]) == FAILED) {
        buf_p += sprintf(buf_p, "%s(): PHY Reg Test Failed: PHY addr %d "
                                "on Page 5.\n", __FUNCTION__, phy_addr);
        DEV_ERROR_REPORT(dev, err_msg, MRVL_88E1340_REG_TEST);
        return (FAILED);
    }       

    if (phy_register_tests(dev, phy_addr, MRV88E1340_REG_PAGE_6,
                           &marvell_88e1340_reg_page6[0]) == FAILED) {
        buf_p += sprintf(buf_p, "%s(): PHY Reg Test Failed: PHY addr %d "
                                "on Page 6.\n", __FUNCTION__, phy_addr);
        DEV_ERROR_REPORT(dev, err_msg, MRVL_88E1340_REG_TEST);
        return (FAILED);
    }

    if (phy_register_tests(dev, phy_addr, MRV88E1340_REG_PAGE_7,
                           &marvell_88e1340_reg_page7[0]) == FAILED) {
        buf_p += sprintf(buf_p, "%s(): PHY Reg Test Failed: PHY addr %d "
                                "on Page 7.\n", __FUNCTION__, phy_addr);
        DEV_ERROR_REPORT(dev, err_msg, MRVL_88E1340_REG_TEST);
        return (FAILED);
    }

    if (phy_register_tests(dev, phy_addr, MRV88E1340_REG_PAGE_8,
                           &marvell_88e1340_reg_page8[0]) == FAILED) {
        buf_p += sprintf(buf_p, "%s(): PHY Reg Test Failed: PHY addr %d "
                                "on Page 8.\n", __FUNCTION__, phy_addr);
        DEV_ERROR_REPORT(dev, err_msg, MRVL_88E1340_REG_TEST);
        return (FAILED);
    }   

    if (phy_register_tests(dev, phy_addr, MRV88E1340_REG_PAGE_9,
                           &marvell_88e1340_reg_page9[0]) == FAILED) {
        buf_p += sprintf(buf_p, "%s(): PHY Reg Test Failed: PHY addr %d "
                                "on Page 9.\n", __FUNCTION__, phy_addr);
        DEV_ERROR_REPORT(dev, err_msg, MRVL_88E1340_REG_TEST);
        return (FAILED);
    }   

    if (phy_register_tests(dev, phy_addr, MRV88E1340_REG_PAGE_12,
                           &marvell_88e1340_reg_page12[0]) == FAILED) {
        buf_p += sprintf(buf_p, "%s(): PHY Reg Test Failed: PHY addr %d "
                                "on Page 12.\n", __FUNCTION__, phy_addr);
        DEV_ERROR_REPORT(dev, err_msg, MRVL_88E1340_REG_TEST);
        return (FAILED);
    }       

    if (phy_register_tests(dev, phy_addr, MRV88E1340_REG_PAGE_14,
                           &marvell_88e1340_reg_page14[0]) == FAILED) {
        buf_p += sprintf(buf_p, "%s(): PHY Reg Test Failed: PHY addr %d "
                                "on Page 14.\n", __FUNCTION__, phy_addr);
        DEV_ERROR_REPORT(dev, err_msg, MRVL_88E1340_REG_TEST);
        return (FAILED);
    }   

    return (ret);
}


/*******************************************************************************
 *
 * Function: dev_88e1340_reg_test().
 *
 * This function implements the PHY registers test.
 *
 * Input:  dev_object_t pointer to the Marvell GE device.
 *         start_addr: 88e1340 phy port 0 addr
 *         order: 0(phy_addr incremental) , 1(phy_addr decremental)
 *                example: order 0, start_addr = 4; 4 phy_addr (4, 5, 6, 7)
 *                         order 1, start_addr = 4; 4 phy_addr (4, 3, 2, 1)
 * 
 * Output: PASSED/FAILED.
 *
 *******************************************************************************
 */
static int dev_88e1340_reg_test (dev_object_t *dev)
{
    uint phy_addr, phy_start;
    dev_88e1340_object_t *phy = (dev_88e1340_object_t *)dev;
    uint addr_seq = (uint)phy->addr_seq;
    buf_p = err_msg;
    
    if (addr_seq == MRV88E1340_PHY_ADDR_INCR) {
        phy_start = (intptr_t)phy->base_phyaddr;
    } else {
        phy_start = (intptr_t)phy->base_phyaddr - MRV88E1340_PORTS + 1;
    }
    
    for (phy_addr = phy_start; phy_addr < (phy_start + MRV88E1340_PORTS);
         phy_addr++) {
        if (dev_88e1340_reg_test_single(dev, phy_addr) == FAILED) {
            buf_p = err_msg;
            DEV_ERROR_REPORT(dev, buf_p, MRVL_88E1340_REG_TEST);
            return (FAILED);
        }
    }
    
    return (PASSED);
}


/*******************************************************************************
 * Name: dev_88e1340_show
 *
 * Description: Provide platforms with a mechanism to display some common
 *      device information via the device print function argument.
 *
 * Input: dev_object_t pointer to the Marvell GE device
 *        A device print function vector
 *        A dev_show_cmd_e command
 *
 * Returns: PASSED/FAILED
 *
 * Assumptions: The device printf function vector has been provided by the host
 *              platform which implements the print logging functionality. The
 *              dev_attach() function has been called and successfully executed
 *
 *******************************************************************************
 */
static uint32_t dev_88e1340_show (dev_object_t *dev, print_fn_t dev_print, 
                                  dev_show_cmd cmd)
{
    dev_88e1340_object_t *phy = (dev_88e1340_object_t *)dev;
    uint phy_addr = (intptr_t)phy->base_phyaddr;
    uint addr_seq = (uint)phy->addr_seq;
    uint phy_start;

    switch (cmd) {
    case DEV_SHOW_REGISTERS:
        if (addr_seq == MRV88E1340_PHY_ADDR_INCR) {
            phy_start = (intptr_t)phy->base_phyaddr;
        } else {
            phy_start = (intptr_t)phy->base_phyaddr - MRV88E1340_PORTS + 1;
        }
        for (phy_addr = phy_start; phy_addr < (phy_start + MRV88E1340_PORTS);
             phy_addr++) {
            dev_88e1340_reg_show(dev, dev_print, phy_addr);
        }
        break;
    default:
        assert(!"dev_88e1340_show");
    }
    return (PASSED);
}


/*******************************************************************************
 *
 * Name: dev_88e1340_init()
 *
 * Description: Initializes the Marvell GE chip (for single port)
 *              
 *
 * Input: dev_object_t pointer to the Marvell GE device.
 *        phy_addr of device.
 *
 * Returns: PASSED/FAILED
 *
 * Note:  PHY Initialization process
 *        DxPhyWrite 0x0 0x16 0xff      (set to page 0xff)
 *        DxPhyWrite 0x0 0x18 0x2800
 *        DxPhyWrite 0x0 0x17 0x2001
 *        DxPhyWrite 0x0 0x16 0x0       (set to page 0x0)
 *        
 *        DxPhyWrite 0x0 0x16 0xff      (set to page 0xff)
 *        DxPhyWrite 0x0 0x17 0x1001
 *        DxPhyRead  0x0 0x19           (equal to 0x2800)
 *        DxPhyWrite 0x0 0x16 0x0       (set to page 0x0)
 *
 *******************************************************************************
 */
static uint32_t dev_88e1340_init (dev_object_t *dev)
{
    dev_88e1340_object_t *phy = (dev_88e1340_object_t *)dev;
    uint phy_addr, phy_start;
    uint addr_seq = (uint)phy->addr_seq;
    uint data;

    buf_p = err_msg;

    if (addr_seq == MRV88E1340_PHY_ADDR_INCR) {
        phy_start = (intptr_t)phy->base_phyaddr;
    } else {
        phy_start = (intptr_t)phy->base_phyaddr - MRV88E1340_PORTS + 1;
    }
    for (phy_addr = phy_start; phy_addr < (phy_start + MRV88E1340_PORTS); 
         phy_addr++) {
        SMIWRITE(phy, phy_addr, 0xff, 0x18, 0x2800);
        SMIWRITE(phy, phy_addr, 0xff, 0x17, 0x2001);
        SMIWRITE(phy, phy_addr, 0xff, 0x17, 0x1001);

        if (SMIREAD(phy, phy_addr, 0xff, 0x19, &data) == FAILED) {
            buf_p += sprintf(buf_p, "%s(): phy smi read failed. phy_addr = "
                                    "#%x, page = %#x, reg = %#x \n",
                                     __FUNCTION__, phy_addr, 
                                     0xff, 0x19);
            buf_p = err_msg;    
            DEV_ERROR_REPORT(dev, buf_p, MRVL_88E1340_SET_LPBK);
            return (FAILED);
        } else {
            if (data != 0x2800) {
                buf_p += sprintf(buf_p, "%s(): PHY addr 0x%2x init failed, "
                                        "reg25:  %#.8x\n", 
                                         __FUNCTION__, phy_addr, data);
                buf_p = err_msg;
                DEV_ERROR_REPORT(dev, buf_p, MRVL_88E1340_INIT);
                return (FAILED);
            }
        }
    }

    phy->base.dev_state = DEV_STATE_INIT;

    return (PASSED);
}


/*******************************************************************************
 *
 * Name: dev_88e1340_detach()
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
 * Input: Pointer to the Marvell GE device object
 *
 * Returns: PASSED/FAILED
 *
 *******************************************************************************
 */
static uint32_t dev_88e1340_detach (dev_object_t *dev)
{
    dev_88e1340_object_t *phy = (dev_88e1340_object_t *) dev;

    /* Init the device object structure to default "do nothing" */
    init_default_dev_object(dev, &m88e1340_fvt);

    phy->base.dev_state = DEV_STATE_DETACH;

    return (PASSED);
}


/*******************************************************************************
 *
 * Name: dev_88e1340_attach()
 *
 * Description: Attach the Marvell GE device for use. This
 *              function will initialize and setup all necessary pointers
 *              and bring the chip to operation.
 *
 * Input: Pointer to the Marvell GE device object
 *
 * Returns: PASSED/FAILED
 *
 *******************************************************************************
 */
static uint32_t dev_88e1340_attach (dev_object_t *dev)
{
    dev_88e1340_object_t *phy = (dev_88e1340_object_t *)dev;

    buf_p = err_msg;

    /* init the call in function */
    phy->callin_fvt = &m88e1340_callin;
    phy->callin_fvt->register_test = dev_88e1340_reg_test;
    phy->callin_fvt->register_test_single = dev_88e1340_reg_test_single;
    phy->callin_fvt->set_loopback = dev_88e1340_set_lpbk;
    phy->callin_fvt->lpbk_mode = dev_88e1340_lpbk_mode;
    phy->callin_fvt->cleanup_loopback = dev_88e1340_cleanup_lpbk;
    phy->callin_fvt->power_up = dev_88e1340_power_up;
    phy->callin_fvt->intr_gen = dev_88e1340_intr_gen;
    phy->callin_fvt->intr_clr = dev_88e1340_intr_clr;
    phy->callin_fvt->lockup_fix = dev_88e1340_lockup_fix;

    /* PHY utilities */
    phy->callin_fvt->show_reg = dev_88e1340_reg_show;
    phy->callin_fvt->alter_reg = dev_88e1340_alter_reg;
    phy->callin_fvt->phone_detect = dev_88e1340_phone_detect;
    phy->callin_fvt->set_test_mode = dev_88e1340_set_test_mode;

    /* init the call out function */
    phy->callout_fvt = &m88e1340_callout;

    phy->base.dev_state = DEV_STATE_ATTACH;

    return (PASSED);
}


/*******************************************************************************
 *
 * Name: dev_88e1340_create()
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
int dev_88e1340_create (dev_object_t *dev, dev_error_report_t error_report_fn)
{
    dev_88e1340_object_t *phy = (dev_88e1340_object_t *)dev;

    /* Init the device object structure to default "do nothing" */
    init_default_dev_object(dev, &m88e1340_fvt);

    phy->base.dev_object_fvt->dev_attach = dev_88e1340_attach;
    phy->base.dev_object_fvt->dev_detach = dev_88e1340_detach;
    phy->base.dev_object_fvt->dev_init = dev_88e1340_init;
    phy->base.dev_object_fvt->dev_show = dev_88e1340_show;
    phy->base.dev_object_fvt->dev_error_report = error_report_fn;
    phy->base.dev_object_fvt->dev_name = "Marvell GE PHY 88E1340";

    phy->base.dev_state = DEV_STATE_CREATE;

    return (PASSED);
}


/******** History ******** 
$Log: dev_phy_88e1340.c,v $
Revision 1.2  2013/10/08 08:48:25  tirawan
Woodlawn collapsed to main trunk

Revision 1.1.4.2  2013/08/20 10:58:48  tirawan
Branch into woodlawn-branch2 and port woodlawn code

Revision 1.1.2.2  2013/06/17 10:58:18  leschen
Remove platform code

Revision 1.1.2.1  2013/04/24 10:58:01  tirawan
First Woodlawn linux integration

Revision 1.3  2013/03/27 04:49:44  kuangik
Code cleanup after -Wall

Revision 1.2  2013/03/19 03:21:32  kuangik
Correct base address when displaying register

Revision 1.1  2013/03/13 06:42:07  kuangik
Add for the first time

Revision 1.12  2013/03/08 07:29:04  kuangik
Call DEV_PRINT_ERROR when register test fails

Revision 1.11  2012/10/24 10:48:37  leslie
Fix and clean up code.

Revision 1.10  2012/10/08 10:00:45  leslie
Fix the alter reg utility.

Revision 1.9  2012/09/05 22:48:51  kody
Add Fiber Spec Cntl2 in register table.

Revision 1.8  2012/08/30 01:26:36  kody
Add set page code before dump and alter any registers.

Revision 1.7  2012/08/28 08:22:46  leslie
*** empty log message ***

Revision 1.6  2012/08/27 06:35:24  leslie
add write page function to set page to R/W

Revision 1.5  2012/08/03 10:16:50  leslie
Mapping to latest O2 source code on 20120726

Revision 1.3  2012/05/18 10:10:10  kody
dev_phy_88e1340.c

Revision 1.2  2012/04/06 06:15:21  kuangik
Add Single Register Test

Revision 1.1.1.1  2012/02/10 05:59:50  kody
Initial imports Woodlawn project code base.

Revision 1.1.16.1  2011/03/11 21:38:41  mcharon
initial support informers linux

Revision 1.1  2010/05/26 07:58:56  jamlin
Initial check-in for Firebee

$Endlog$
*/
