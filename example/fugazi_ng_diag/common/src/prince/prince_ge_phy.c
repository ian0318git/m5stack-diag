/* $Id: prince_ge_phy.c,v 1.3 2014/01/13 03:07:05 xiaoyizh Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/prince/prince_ge_phy.c,v $
 *------------------------------------------------------------------
 *
 * prince_ge_mac.c - Prince GE PHY function.
 *
 * Xiaoying Zhang -- Nov. 2012
 *
 * Copyright (c) 2013-2014 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */
 
#include "common.h"
#include "types.h"
#include "defs.h"
#include "error.h"
#include "proto.h"
#include "common_utils.h"
#include "pcmap.h"
#include "prince_reg.h"
#include "prince_def.h"
#include "nvmonvars.h"
#include "prince_ge_mac.h"
#include "dev_phy_88e1512.h"
#include "prince_eth_pkt.h"

static void phy_88e1512_err_report (uchar *err_msg, uint32 err_id);
extern int fd_prc;

typedef struct mrvl_88e1512_phy_regs_t_
{
    const char *pagename;
    uint32_t    pagenum;
    const reg_info_t *pageregs;
} mrvl_88e1512_phy_regs_t;

static const reg_info_t marvell_88e1512_reg_page0[] = {   /* Page 0 */
    {"Copper Control",      0x00, READ_ONLY,  {2}, 0x3940, 0x1940},
    {"Copper Status",       0x01, READ_ONLY,  {2}, 0x0000, 0x7949},
    {"PHY ID1",             0x02, READ_ONLY,  {2}, 0x0000, 0x0141},
    {"PHY ID2",             0x03, READ_ONLY,  {2}, 0x0000, 0x0dc0},
    {"Copper Auto-Neg",     0x04, READ_ONLY,  {2}, 0xBFFF, 0x01e1},
    {"Copper Link-P Abil",  0x05, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"Copper Auto-Neg Exp", 0x06, READ_ONLY,  {2}, 0x0000, 0x0004},
    {"Copper Next Page",    0x07, READ_WRITE, {2}, 0xB7FF, 0x2001},
    {"Copper Link Partner", 0x08, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"1000BT Control",      0x09, READ_ONLY,  {2}, 0xF2FF, 0x0f00},
    {"1000BT Status",       0x0A, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"Extended Status",     0x0F, READ_ONLY,  {2}, 0x0000, 0x3000},
    {"Copper Spec Cntl1",   0x10, READ_WRITE, {2}, 0x7C9F, 0x3060},
    {"Copper Spec Ststus",  0x11, READ_ONLY,  {2}, 0x0000, 0xC040},
    {"Copper Spec Intr Ena",0x12, READ_ONLY,  {2}, 0xFFFF, 0x0000},
    {"Copper Intr Status",  0x13, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"Copper Spec Cntl2",   0x14, READ_WRITE, {2}, 0x00FF, 0x0020},
    {"Copper Spec Rx Err",  0x15, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"Global Intr Status",  0x17, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"Copper Spec Cntl3",   0x1A, READ_WRITE, {2}, 0x63FF, 0x0040},
    {"end",                 0x00, 0, {0}, 0, 0},
};

static const reg_info_t marvell_88e1512_reg_page1[] = {  /* Page 1*/
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
    {"Fiber Intr Enable",   0x12, READ_ONLY,  {2}, 0x7F80, 0x0000},
    {"Fiber Intr Status",   0x13, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"Fiber Rx Err Cnt",    0x15, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"PRBS Control",        0x17, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"PRBS Err Cnt LSB",    0x18, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"PRBS Err Cnt MSB",    0x19, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"Fiber Spec Cntl2",    0x1a, READ_ONLY,  {2}, 0xC24F, 0x0042},
    {"end",                 0x00, 0, {0}, 0, 0},
};

static const reg_info_t marvell_88e1512_reg_page2[] = {   /* Page 2*/
    {"MAC Spec Cntl1",      0x10, READ_ONLY,  {2}, 0xDBC8, 0x4004},
    {"MAC Spec Intr Ena",   0x12, READ_ONLY,  {2}, 0x008C, 0x0000},
    {"MAC Intr Status",     0x13, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"MAC RX_ER Byte",      0x14, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"MAC Spec Cntl2",      0x15, READ_WRITE, {2}, 0x4008, 0x1046},
    {"RGMII Out Imp Cal",   0x18, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"RGMII Out Imp Tar",   0x19, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"end",                 0x00, 0, {0}, 0, 0},
};

static const reg_info_t marvell_88e1512_reg_page3[] = {   /* Page 3*/
    {"LED Func Cntl1",      0x10, READ_WRITE, {2}, 0xFFFF, 0x1777},
    {"LED Polarity Cntl",   0x11, READ_WRITE, {2}, 0xFFFF, 0x8800},
    {"LED Timer Cntl",      0x12, READ_WRITE, {2}, 0xF70F, 0x4905},
    {"PTP LED Func Cntl",   0x13, READ_WRITE, {2}, 0x2000, 0x0073},
    {"end",                 0x00, 0, {0}, 0, 0},
};

static const reg_info_t marvell_88e1512_reg_page4[] = {   /* Page 4*/
    {"RGMII RX_ER Cap",     0x14, READ_ONLY,  {2}, 0xB3FF, 0x0000},
    {"end",                 0x00, 0, {0}, 0, 0},
};

static const reg_info_t marvell_88e1512_reg_page5[] = {   /* Page 5*/
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

static const reg_info_t marvell_88e1512_reg_page6[] = {   /* Page 6*/
    {"Packet Generation",   0x10, READ_ONLY,  {2}, 0xFF07, 0x0000},
    {"CRC Counters",        0x11, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"Checker Control",     0x12, READ_WRITE, {2}, 0x0007, 0x0000},
    {"General Control",     0x14, READ_WRITE, {2}, 0x03c0, 0x0200},
    {"Late Colli Cnt1&2",   0x17, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"Late Colli Cnt3&4",   0x18, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"Late Colli Window",   0x19, READ_WRITE, {2}, 0x1F00, 0x0000},
    {"Misc Test",           0x1A, READ_WRITE, {2}, 0x9FA0, 0x1900},
    {"Misc Test Temp Sensor", 0x1B, READ_WRITE, {2}, 0x1F00, 0x0C00},
    {"end",                 0x00, 0, {0}, 0, 0},
};

static const reg_info_t marvell_88e1512_reg_page7[] = {   /* Page 7*/
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

static const reg_info_t marvell_88e1512_reg_page8[] = {   /* Page 8*/
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
    {"end",                 0x00, 0, {0}, 0, 0},
};

static const reg_info_t marvell_88e1512_reg_page9[] = {   /* Page 9*/
    {"PTP Dep Status",      0x00, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"PTP Dep Byte1&0",     0x01, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"PTP Dep Byte3&2",     0x02, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"PTP Dep Sequ ID",     0x03, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"PTP Dep Cnt",         0x05, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"end",                 0x00, 0, {0}, 0, 0},
};

static const reg_info_t marvell_88e1512_reg_page12[] = {   /* Page 12*/
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
    {"end",                 0x00, 0, {0}, 0, 0},
};

static const reg_info_t marvell_88e1512_reg_page14[] = {   /* Page 14*/
    {"PTP Global Conf 0",   0x00, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"PTP Global Conf 1",   0x01, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"PTP Global Conf 2",   0x02, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"PTP Global Conf 3",   0x03, READ_ONLY,  {2}, 0x0000, 0x0001},
    {"PTP Global Status",   0x08, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"end",                 0x00, 0, {0}, 0, 0},
};

static const reg_info_t marvell_88e1512_reg_page17[] = {   /* Page 17*/
    {"WOL Control",         0x10, READ_WRITE, {2}, 0xe1ff, 0x0000},
    {"WOL Status",          0x11, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"SRAM Pkt 7/6 Length", 0x12, READ_WRITE, {2}, 0x03ff, 0x0cff},
    {"SRAM Pkt 5/4 Length", 0x13, READ_WRITE, {2}, 0x03ff, 0x0cff},
    {"SRAM Pkt 3/2 Length", 0x14, READ_WRITE, {2}, 0x03ff, 0x0cff},
    {"SRAM Pkt 1/0 Length", 0x15, READ_WRITE, {2}, 0x03ff, 0x0cff},
    {"Magic Pkt Des Addr2", 0x17, READ_ONLY,  {2}, 0xffff, 0x0000},
    {"Magic Pkt Des Addr1", 0x18, READ_ONLY,  {2}, 0xffff, 0x0000},
    {"Magic Pkt Des Addr0", 0x19, READ_ONLY,  {2}, 0xffff, 0x0000},
    {"SRAM Byte Addr Ctrl", 0x1a, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"SRAM Byte Data Ctrl", 0x1b, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"SRAM Read Ctrl",      0x1c, READ_ONLY,  {2}, 0x01ff, 0x0000},
    {"end",                 0x00, 0, {0}, 0, 0},
};

static const reg_info_t marvell_88e1512_reg_page18[] = {   /* Page 18*/
    {"EEE Buffer Ctrl1",    0x00, READ_ONLY, {2}, 0xffff, 0x0000},
    {"EEE Buffer Ctrl2",    0x01, READ_ONLY, {2}, 0xffff, 0x0000},
    {"EEE Buffer Ctrl3",    0x02, READ_ONLY, {2}, 0xffff, 0x0000},
    {"Packet Generation",   0x10, READ_ONLY, {2}, 0xffff, 0x0000},
    {"CRC Counters",        0x11, READ_ONLY, {2}, 0xffff, 0x0000},
    {"Checker Control",     0x12, READ_ONLY, {2}, 0xffff, 0x0000},
    {"Packet Generation",   0x13, READ_ONLY, {2}, 0xffff, 0x0000},
    {"General Control 1",   0x14, READ_ONLY, {2}, 0xffff, 0x0000},
    {"Link Disconnect cnt", 0x19, READ_ONLY, {2}, 0xffff, 0x0000},
    {"SERDES RX_ER Cap",    0x1a, READ_ONLY, {2}, 0xffff, 0x0000},
    {"end",                 0x00, 0, {0}, 0, 0},
};

static const mrvl_88e1512_phy_regs_t marvell_88e1512_phy_reg_tbl[] = {
    {"Page  0",     0, marvell_88e1512_reg_page0},
    {"Page  1",     1, marvell_88e1512_reg_page1},
    {"Page  2",     2, marvell_88e1512_reg_page2},
    {"Page  3",     3, marvell_88e1512_reg_page3},
    {"Page  4",     4, marvell_88e1512_reg_page4},
    {"Page  5",     5, marvell_88e1512_reg_page5},
    {"Page  6",     6, marvell_88e1512_reg_page6},
    {"Page  7",     7, marvell_88e1512_reg_page7},
    {"Page  8",     8, marvell_88e1512_reg_page8},
    {"Page  9",     9, marvell_88e1512_reg_page9},
    {"Page 12",    12, marvell_88e1512_reg_page12},
    {"Page 14",    14, marvell_88e1512_reg_page14},
    {"Page 17",    17, marvell_88e1512_reg_page17},
    {"Page 18",    18, marvell_88e1512_reg_page18},
};

#define NUM_PHY_PAGES (sizeof(marvell_88e1512_phy_reg_tbl) /      \
                             sizeof(mrvl_88e1512_phy_regs_t))

static uchar *buf_p;
static uchar err_msg[MRV88E1512_ERR_MSG_LEN];

int dev_phy_read_reg(ushort page, ushort offset, ushort *data)
{
    int retval;

    /* Set page */
    if (ge_mac_mdio_write(MRV88E1512_PAGE_ADDRESS_REG, &page)) {
        printf("Failed to set page %d.", page);  
        return (FAILED);
    }

    retval = ge_mac_mdio_read(offset, data);
    if (retval != PASSED) {
        printf("phy smi read failed. phy_addr = %d, "
               "page = %d, reg = %#x\n",
               PRINCE_PHY_ADDR, page, offset);
    }
    return (retval);
}

int dev_phy_write_reg(ushort page, ushort offset, ushort *data)
{
    int retval;

    /* Set page */
    if (ge_mac_mdio_write(MRV88E1512_PAGE_ADDRESS_REG, &page)) {
        printf("Failed to set page %d.", page);
        return (FAILED);
    }

    retval = ge_mac_mdio_write(offset, data);
    if (retval != PASSED) {
        printf("phy smi write failed. phy_addr = %d,"
               "page = %d, reg = %#x\n",
               PRINCE_PHY_ADDR, page, offset);
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
static int phy_register_tests (ushort page, 
                               const reg_info_t *reg_ptr )
{
    uint32_t i;
    int retval = PASSED;
    int retval2 =  PASSED;
    ushort save_val, readval;
    ushort data, temp;
    ushort tst_offset;

    readval = 0;

    while (reg_ptr->size.size != 0) {
        retval = dev_phy_read_reg(page, reg_ptr->offset, &save_val);
        if (retval == FAILED) {
            buf_p += sprintf(buf_p, "%s(): Error reading %s register "
                                    "offset %#x", __FUNCTION__, 
                                     reg_ptr->name, reg_ptr->offset);
            return (FAILED);
        }

        if (reg_ptr->type == READ_WRITE) {
            tst_offset = reg_ptr->offset;

            /* 
             * ripple 1 test
             */
            for (i = 0; i < (reg_ptr->size.size * 8); i++) {
                temp = (1 << i) & reg_ptr->mask;
                if (!temp) {
                    continue;
                }

                /* Write to register under test */
                retval = dev_phy_write_reg(page, tst_offset, &temp);
                /* Read back */
                readval = 0;
                if (retval == PASSED) {
                    retval2 = dev_phy_read_reg(page, tst_offset, &readval);
                }
                if (((readval & reg_ptr->mask) != temp) ||
                    (retval == FAILED) || (retval2 == FAILED)) {
                    buf_p += sprintf(buf_p, "%s(): Ripple one test "
                                            "failed when accessing %s "
                                            "Register offset %#x, "
                                            " Expect %#x, Read %#x",
                                             __FUNCTION__, reg_ptr->name,
                                             tst_offset, temp, 
                                             readval);
                    return (FAILED);
                }
            }

            /* 
             * ripple 0 test
             */
            for (i = 0; i < (reg_ptr->size.size * 8); i++) {
                temp = (1 << i) & reg_ptr->mask;
                if (!temp) {
                    continue;
                }
                temp = (~(1 << i)) & reg_ptr->mask;
                /* Write to register under test */
                readval = 0xFFFF;
                retval = dev_phy_write_reg(page, tst_offset, &temp);
                if (retval == PASSED) {
                    /* Read back */
                    retval2 = dev_phy_read_reg(page, tst_offset, &readval);
                }
                if (((readval & reg_ptr->mask) != temp) ||
                    (retval == FAILED) || (retval2 == FAILED)) {
                    buf_p += sprintf(buf_p, "%s(): Ripple one test "
                                            "failed when accessing %s "
                                            "Register offset %#x, "
                                            "Expect %#x, Read %#x",
                                             __FUNCTION__, reg_ptr->name,
                                             tst_offset, temp,
                                             readval);
                    return (retval);
                }
            }

            /*
             * pattern test
             */
            data = (ushort)PATTERN;
            for (i = 0; i < 2; i++) {
                temp = data &reg_ptr->mask;
                /* Write to register under test */
                retval = dev_phy_write_reg(page, tst_offset, &temp);
                if (retval == PASSED) {
                    /* Read back */
                    retval2 = dev_phy_read_reg(page, tst_offset, &readval);
                }
                if (((readval & reg_ptr->mask) != temp) ||
                    (retval == FAILED) || (retval2 == FAILED)) {
                    buf_p += sprintf(buf_p, "%s(): Pattern test failed "
                                            "when accessing %s Register "
                                            "offset %#x Expect %#x, "
                                            "Read %#x", __FUNCTION__,
                                             reg_ptr->name, tst_offset, 
                                             temp, readval);
                    return (retval);
                }

                data = (ushort)~PATTERN; /* complement data pattern */
            }

            /*
             * restore original value
             */
            retval = dev_phy_write_reg(page, tst_offset, &save_val);
            if (retval == FAILED) {
                buf_p += sprintf(buf_p, "%s(): Error restoring %s register "
                                        "offset %#x\n", __FUNCTION__, 
                                         reg_ptr->name, reg_ptr->offset);
                return (FAILED);
            }
        }
        reg_ptr++;
    }

    return (PASSED);
}

/*******************************************************************************
 *
 * Function: phy_id_check
 *
 * Read the Phy ID registers to check with the desired value.
 *
 * Output: PASSED/FAILED
 *
 *******************************************************************************
 */
int phy_id_check()
{
    ushort data;
    int retval;

    retval = dev_phy_read_reg(MRV88E1512_REG_PAGE_0, 
                     MRV88E1512_PHY_ID1, &data);
    if (retval != PASSED) {
        buf_p += sprintf(buf_p, "%s(): phy smi read failed\n",
                                 __FUNCTION__);
        return (FAILED);
    }
    if (data != 0x0141) {
        buf_p += sprintf(buf_p, "%s(): phy id1 check failed, Expected %#x, Read %#x\n",
                                 __FUNCTION__, 0x0141, data);
        return (FAILED);
    }

    retval = dev_phy_read_reg(MRV88E1512_REG_PAGE_0, 
                     MRV88E1512_PHY_ID2, &data);
    if (retval != PASSED) {
        buf_p += sprintf(buf_p, "%s(): phy smi read failed\n",
                                 __FUNCTION__);
        return (FAILED);
    }
    if ((data & 0xfff0) != 0x0dd0) {
        buf_p += sprintf(buf_p, "%s(): phy id2 check failed, Expected %#x, Read %#x\n",
                                 __FUNCTION__, 0x0dd0, data);
        return (FAILED);
    }
    return (PASSED);
}

int phy_reg_test_all_pages()
{
    int i;
    ushort page = 0;
    const reg_info_t *reg_ptr;
    const mrvl_88e1512_phy_regs_t *page_reg_ptr = 
                             &marvell_88e1512_phy_reg_tbl[0];

    buf_p = err_msg;

    for (i = 0; i < NUM_PHY_PAGES; i++) {
        page = page_reg_ptr->pagenum;
        reg_ptr = page_reg_ptr->pageregs;

        prpass(testpass, "PHY register test on page %d", page);
        if (phy_register_tests(page, reg_ptr) != PASSED) {
            buf_p = err_msg;
            phy_88e1512_err_report(buf_p, MRVL_88E1512_REG_TEST | FATAL);
            cterr('f', 0, "PHY register test failed on page %d.", page);
            return (FAILED);
        }
        page_reg_ptr++;
    }

    return (PASSED);
}

int phy_reg_test()
{
    testname("PHY Register");

    buf_p = err_msg;

    if (phy_id_check()) {
        buf_p = err_msg;
        phy_88e1512_err_report(buf_p, MRVL_88E1512_REG_TEST | FATAL);
        cterr('f', 0, "PHY register test failed on id check.");
        return (FAILED);
    }

    prpass(testpass, "PHY register test passed");

    return (PASSED);
}

/*********************************************************************
 *
 * Function: phy_88e1512_err_report()
 *
 * Description: This function reports error or warning
 *              depends on the error ID flag
 *
 * Inputs:  err_msg  - Error message to be reported
 *          err_id   - Error reporting type identifier
 *
 * Outputs: void
 *
 *********************************************************************
 */
static void phy_88e1512_err_report (uchar *err_msg, uint32 err_id)
{
    if (err_id & ~FATAL) {
          printf("\%d :%s\n", err_id, err_msg);
    } else {
        switch (err_id & FATAL) {
        case WARNING:
            cterr('w', 0, "%s", err_msg);
            break;
        case RETRY:
            printf("\nRetry: %s\n", err_msg);
            break;
        default:
            cterr('f', 0, "%s", err_msg);
            break;
        }
    }
}

/*******************************************************************************
 *
 * Function: retval2dev_88e1512_soft_reset().
 *
 * This function resets the phy and waits for its completion before returning.
 *
 * Input:  phy_addr
 *
 * Output: PASSED/FAILED.
 *
 *******************************************************************************
 */
static int retval2dev_88e1512_soft_reset (uint phy_addr)
{
    int retval = FAILED;
    ushort data;
    buf_p = err_msg;

    /* Do a soft reset and power up */
    retval = dev_phy_read_reg(MRV88E1512_REG_PAGE_1, 
                     MRV88E1512_CONTROL_REG, &data);
    if (retval != PASSED) {
        buf_p += sprintf(buf_p, "%s(): read phy control reg failed (phy addr"
                                " %d)\n", __FUNCTION__, phy_addr);
        return (retval);
    }
    data |= (MRV88E1512_FIBER_RST);
    retval = dev_phy_write_reg(MRV88E1512_REG_PAGE_1, 
                      MRV88E1512_CONTROL_REG, &data);
    if (retval != PASSED) {
        buf_p += sprintf(buf_p, "%s(): phy reset failed (phy addr = %d)\n",
                                 __FUNCTION__, phy_addr);
        return (retval);
    }

    /* Check if phy reset complete */
    msleep(100);
    retval = dev_phy_read_reg(MRV88E1512_REG_PAGE_1, 
                     MRV88E1512_CONTROL_REG, &data);
    if (retval != PASSED) {
        buf_p += sprintf(buf_p, "%s(): Failed to read back reset bit (phy "
                                "addr = %d)\n", __FUNCTION__, phy_addr);
        return (retval);
    }
    
    if (data & MRV88E1512_FIBER_RST) {
        buf_p += sprintf(buf_p, "%s(): failed to get out of reset (phy "
                                "addr = %d).\n", __FUNCTION__, phy_addr);
    } else {
        if ((NVRAM)->diagflag & D_VERBOSE) {
            printf("%s(): PHY soft reset ok at phy addr %d\n", 
                    __FUNCTION__, phy_addr);
        }
    }

    msleep(100);
    return (retval);
}

/*******************************************************************************
 *
 * Function: retval2dev_88e1512_mode_soft_reset().
 *
 * Input:  phy_addr
 *
 * Output: PASSED/FAILED.
 *
 *******************************************************************************
 */
static int retval2dev_88e1512_mode_soft_reset (uint phy_addr)
{
    int retval = FAILED;
    ushort data;

    /* Do a soft reset and power up */
    retval = dev_phy_read_reg(MRV88E1512_REG_PAGE_18, 
                     MRV88E1512_GEN_CTRL1, &data);
    if (retval != PASSED) {
        buf_p += sprintf(buf_p, "%s(): read general control reg failed (phy addr"
                                " %d)\n", __FUNCTION__, phy_addr);
        return (retval);
    }
    data |= (MRV88E1512_P18_R20_RST);
    retval = dev_phy_write_reg(MRV88E1512_REG_PAGE_18, 
                      MRV88E1512_GEN_CTRL1, &data);
    if (retval != PASSED) {
        buf_p += sprintf(buf_p, "%s(): phy mode reset failed (phy addr = %d)\n",
                                 __FUNCTION__, phy_addr);
        return (retval);
    }

    /* Check if phy reset complete */
    msleep(100);
    retval = dev_phy_read_reg(MRV88E1512_REG_PAGE_18, 
                     MRV88E1512_GEN_CTRL1, &data);
    if (retval != PASSED) {
        buf_p += sprintf(buf_p, "%s(): Failed to read back reset bit (phy "
                                "addr = %d)\n", __FUNCTION__, phy_addr);
        return (retval);
    }
    
    if (data & MRV88E1512_P18_R20_RST) {
        buf_p += sprintf(buf_p, "%s(): failed to get out of reset (phy "
                                "addr = %d).\n", __FUNCTION__, phy_addr);
    } else {
        if ((NVRAM)->diagflag & D_VERBOSE) {
            printf("%s(): PHY mode soft reset ok at phy addr %d\n", 
                    __FUNCTION__, phy_addr);
        }
    }

    msleep(100);
    return (retval);
}

int phy_intr_test()
{
    int retval = FAILED;
    ushort data;
    int mask = 0;
    int i;

    testname("PHY Interrupt");

    /* Enable interrupts on LED2 pin */
    retval = dev_phy_read_reg(MRV88E1512_REG_PAGE_3, 
                     MRV88E1512_LED_TIMER_CTRL, &data);
    if (retval != PASSED) {
        buf_p += sprintf(buf_p, "%s(): phy smi read failed\n",
                                 __FUNCTION__);
        buf_p = err_msg;
        phy_88e1512_err_report(buf_p, MRVL_88E1512_INIT);
        return (retval);
    }

    data |= MRV88E1512_P3_R18_INT_EN;
    retval = dev_phy_write_reg(MRV88E1512_REG_PAGE_3, 
                      MRV88E1512_LED_TIMER_CTRL, &data);
    if (retval != PASSED) {
        buf_p += sprintf(buf_p, "%s(): Enable LED Timer Interrupt "
                                "failed.\n", __FUNCTION__);
        buf_p = err_msg;
        phy_88e1512_err_report(buf_p, MRVL_88E1512_INIT);
        return (retval);
    }

    if (ioctl(fd_prc, ENABLE_IRQ, MVL_PHY_INTR_ID)) {
        buf_p += sprintf(buf_p, "%s(): Enable PHY interrupt "
                                "failed.\n", __FUNCTION__);
        return (FAILED);
    }

    /* Force INTn to assert */
    retval = dev_phy_read_reg(MRV88E1512_REG_PAGE_1, 
                     MRV88E1512_SPECIFIC_CONTROL2_REG, &data);
    if (retval != PASSED) {
        buf_p += sprintf(buf_p, "%s(): phy smi read failed\n",
                                 __FUNCTION__);
        buf_p = err_msg;    
        phy_88e1512_err_report(buf_p, MRVL_88E1512_INIT);
        return (retval);
    }

    data |= MRV88E1512_FIBER_FORCE_INT;
    retval = dev_phy_write_reg(MRV88E1512_REG_PAGE_1, 
                      MRV88E1512_SPECIFIC_CONTROL2_REG, &data);
    if (retval != PASSED) {
        buf_p += sprintf(buf_p, "%s(): Force Interrupt "
                                "failed.\n", __FUNCTION__);
        buf_p = err_msg;
        phy_88e1512_err_report(buf_p, MRVL_88E1512_INIT);
        return (retval);
    }

    /* Wait 1 seconds here to let interrupt to be serviced and cleared. */
    for (i = 0; i < 5000; i++) {
        retval = dev_phy_read_reg(MRV88E1512_REG_PAGE_1, 
                        MRV88E1512_SPECIFIC_CONTROL2_REG, &data);
        if (retval != PASSED) {
            buf_p += sprintf(buf_p, "%s(): phy smi read failed\n",
                                    __FUNCTION__);
            buf_p = err_msg;    
            phy_88e1512_err_report(buf_p, MRVL_88E1512_INIT);
            return (retval);
        }

        if ((data & MRV88E1512_FIBER_FORCE_INT) == 0) {
            break;
        } else {
            usleep(100);
        }
    }

    get_gic_spi_status1();

    if (ioctl(fd_prc, DISABLE_IRQ, MVL_PHY_INTR_ID)) {
        perror("Failed to Disable PHY interrupt");
        return (FAILED);
    }


    if (i == 5000) {
        cterr('f', 0, 
              "Timeout waiting for interrupt to be cleared.",
              "intr status %#x ",
               get_gic_spi_status1());
        return (FAILED);
    }

    return (PASSED);
}

/*******************************************************************************
 *
 * Function: dev_88e1512_set_lpbk
 *
 * This function set PHY loopback mode and speed.
 *
 * Input: phy_addr 
 *        speed - 10/100/1G
 *        lpbk  - loopback mode
 *
 * Output: PASSED/FAILED
 *
 *******************************************************************************
 */
static int dev_88e1512_set_lpbk (int phy_addr, 
                                 int speed, 
                                 int lpbk)
{
    int retval = PASSED;
    int phy_speed = -1, mac_speed = -1;
    ushort data;

    buf_p = err_msg;
    
    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("setup phy device, phy_addr %d, speed %d, loopback %d\n",
                phy_addr, speed, lpbk);
    }

    switch (speed) {
    case ETH_MODE_GE:
        phy_speed = MRV88E1512_SPD_SEL_1000M;
        mac_speed = MRV88E1512_MAC_SPD_1000M;
        break;
    default:
        buf_p += sprintf(buf_p, "%s(): unknown speed\n", __FUNCTION__);
        buf_p = err_msg;
        phy_88e1512_err_report(buf_p, MRVL_88E1512_SET_LPBK);
        return (FAILED);
    }

    /* Config phy speed for SGMII (page 2, reg 21) */
    retval = dev_phy_read_reg(MRV88E1512_REG_PAGE_2, 
                        MRV88E1512_MAC_CNTL_REG2, &data);
    if (retval != PASSED) {
        buf_p += sprintf(buf_p, "%s(): phy smi read failed\n",
                                 __FUNCTION__);
        buf_p = err_msg;    
        phy_88e1512_err_report(buf_p, MRVL_88E1512_SET_LPBK);
        return (retval);
    }

    data &= ~MRV88E1512_MAC_SPD_MASK;
    data |= mac_speed;
    retval = dev_phy_write_reg(MRV88E1512_REG_PAGE_2, 
                        MRV88E1512_MAC_CNTL_REG2, &data);
    if (retval != PASSED) {
        buf_p += sprintf(buf_p, "%s(): set phy mac speed failed.\n",
                                    __FUNCTION__);
        buf_p = err_msg;
        phy_88e1512_err_report(buf_p, MRVL_88E1512_SET_LPBK);
        return (retval);
    }

    /* Disable Auto-Negotiation */
    retval = dev_phy_read_reg(MRV88E1512_REG_PAGE_1, 
                     MRV88E1512_CONTROL_REG, &data);
    if (retval != PASSED) {
        buf_p += sprintf(buf_p, "%s(): phy smi read failed\n",
                                 __FUNCTION__);
        buf_p = err_msg;
        phy_88e1512_err_report(buf_p, MRVL_88E1512_CLN_LPBK);
        return (retval);
    }

    data &= ~MRV88E1512_AUTO_NEO_ENA;
    retval = dev_phy_write_reg(MRV88E1512_REG_PAGE_1, 
                      MRV88E1512_CONTROL_REG, &data);
    if (retval != PASSED) {
        buf_p += sprintf(buf_p, "%s(): disable auto-negotiation "
                                "failed.\n", __FUNCTION__);
        buf_p = err_msg;
        phy_88e1512_err_report(buf_p, MRVL_88E1512_CLN_LPBK);
        return (retval);
    }

    /* Set speed of page 1, reg 0 */
    retval = dev_phy_read_reg(MRV88E1512_REG_PAGE_1, 
                        MRV88E1512_CONTROL_REG, &data);
    if (retval != PASSED) {
        buf_p += sprintf(buf_p, "%s(): phy smi read failed\n",
                                 __FUNCTION__);
        buf_p = err_msg;    
        phy_88e1512_err_report(buf_p, MRVL_88E1512_SET_LPBK);
        return (retval);
    }

    data &= ~MRV88E1512_SPD_SEL_MASK;
    data |= phy_speed;
    retval = dev_phy_write_reg(MRV88E1512_REG_PAGE_1, 
                        MRV88E1512_CONTROL_REG, &data);
    if (retval != PASSED) {
        buf_p += sprintf(buf_p, "%s(): set phy copper speed "
                                "failed.\n", __FUNCTION__);
        buf_p = err_msg;
        phy_88e1512_err_report(buf_p, MRVL_88E1512_SET_LPBK);
        return (retval);
    }

    /* Do soft reset to make the speed config take effect */
    retval = retval2dev_88e1512_soft_reset(phy_addr);
    if (retval != PASSED) {
        buf_p += sprintf(buf_p, "%s(): phy addr %d reset failed. rc = %#x"
                                "\n", __FUNCTION__, phy_addr, retval);
        buf_p = err_msg;
        phy_88e1512_err_report(buf_p, MRVL_88E1512_SET_LPBK);
        return (FAILED);
    }

    /* Config PHY loopback mode (bit 14, page 1, reg 0) */
    retval = dev_phy_read_reg(MRV88E1512_REG_PAGE_1,
                     MRV88E1512_CONTROL_REG, &data);
    if (retval != PASSED) {
        buf_p += sprintf(buf_p, "%s(): phy smi read failed\n",
                                 __FUNCTION__);
        buf_p = err_msg;    
        phy_88e1512_err_report(buf_p, MRVL_88E1512_SET_LPBK);
        return (retval);
    }

    if (lpbk == SGMII_LPBK_NONE) {
        data &= ~MRV88E1512_LPBK_ENA;
    } else {
        data |= MRV88E1512_LPBK_ENA;
    }

    retval = dev_phy_write_reg(MRV88E1512_REG_PAGE_1, 
                      MRV88E1512_CONTROL_REG, &data);
    if (retval != PASSED) {
        buf_p += sprintf(buf_p, "%s(): set phy loopback mode failed.\n",
                                 __FUNCTION__);
        buf_p = err_msg;
        phy_88e1512_err_report(buf_p, MRVL_88E1512_SET_LPBK);
        return (retval);
    }

    msleep(2000);  /* Wait for link up */

    /* Check fiber link speed (page 1, reg 17)*/
    retval = dev_phy_read_reg(MRV88E1512_REG_PAGE_1,
                        MRV88E1512_SPECIFIC_STATUS1_REG, &data);    
    if (retval != PASSED) {
        buf_p += sprintf(buf_p, "%s(): phy smi read failed\n",
                                 __FUNCTION__);
        buf_p = err_msg;    
        phy_88e1512_err_report(buf_p, MRVL_88E1512_SET_LPBK);
        return (retval);
    }    
    
    if (!(data & MRV88E1512_LINK_UP)) {
        if ((NVRAM)->diagflag & D_VERBOSE) {
            printf("%s(): Copper is NOT link up.\n", __FUNCTION__);
        }
    }

    switch (speed) {
    case ETH_MODE_GE:
        if ((data & MRV88E1512_LINK_SPEED_MASK) != 
            MRV88E1512_LINK_SPEED_1000) {
            buf_p += sprintf(buf_p, "%s(): Copper link speed is NOT "
                                    "1Gbps.\n", __FUNCTION__);
            buf_p = err_msg;
            phy_88e1512_err_report(buf_p, MRVL_88E1512_SET_LPBK);
            return (FAILED);
        }
        break;
    default:
        buf_p += sprintf(buf_p, "%s(): unknown link speed\n", __FUNCTION__);
        buf_p = err_msg;
        phy_88e1512_err_report(buf_p, MRVL_88E1512_SET_LPBK);
        return (FAILED);
    }

    return (retval);
}

/*******************************************************************************
 *
 * Function: dev_88e1512_cleanup_lpbk
 *
 * This function disable loopbacks and sets Marvell GE PHY
 * back into normal operating mode.
 *
 * Input:  phy address
 *
 * Output: PASSED/FAILED
 *
 *******************************************************************************
 */
static int dev_88e1512_cleanup_lpbk (uint phy_addr)
{
    int retval = PASSED;
    int phy_speed = MRV88E1512_SPD_SEL_1000M;
    int mac_speed = MRV88E1512_MAC_SPD_1000M;
    ushort data;

    buf_p = err_msg;

    /* clear PHY loopback mode (bit 14, page 1, reg 0) */
    retval = dev_phy_read_reg(MRV88E1512_REG_PAGE_1,
                     MRV88E1512_CONTROL_REG, &data);
    if (retval != PASSED) {
        buf_p += sprintf(buf_p, "%s(): phy smi read failed\n",
                                 __FUNCTION__);
        buf_p = err_msg;    
        phy_88e1512_err_report(buf_p, MRVL_88E1512_CLN_LPBK);
        return (retval);
    }

    data &= ~MRV88E1512_LPBK_ENA;
    retval = dev_phy_write_reg(MRV88E1512_REG_PAGE_1, 
                      MRV88E1512_CONTROL_REG, &data);
    if (retval != PASSED) {
        buf_p += sprintf(buf_p, "%s(): clear phy loopback mode "
                                "failed.\n", __FUNCTION__);
        buf_p = err_msg;
        phy_88e1512_err_report(buf_p, MRVL_88E1512_CLN_LPBK);
        return (retval);
    }

    return (retval);
}

/*******************************************************************************
 *
 * Function: dev_88e1512_set_out_amp
 *
 * This function sets the output amptitule for Marvell GE PHY
 *
 * Input:  phy address
 *
 * Output: PASSED/FAILED
 *
 *******************************************************************************
 */
static int dev_88e1512_set_out_amp (uint phy_addr)
{
    int retval = PASSED;
    ushort data;

    retval = dev_phy_read_reg(MRV88E1512_REG_PAGE_1, 
                     MRV88E1512_SPECIFIC_CONTROL2_REG, &data);
    if (retval != PASSED) {
        buf_p += sprintf(buf_p, "%s(): phy smi read failed\n",
                                 __FUNCTION__);
        buf_p = err_msg;    
        phy_88e1512_err_report(buf_p, MRVL_88E1512_ALTER_REG);
        return (retval);
    }

    data &= ~0x7;
    data |= 0x5;
    retval = dev_phy_write_reg(MRV88E1512_REG_PAGE_1, 
                      MRV88E1512_SPECIFIC_CONTROL2_REG, &data);
    if (retval != PASSED) {
        buf_p += sprintf(buf_p, "%s(): Set SGMII Output Amplitude "
                                "failed.\n", __FUNCTION__);
        return (retval);
    }

    return (retval);
}

/*******************************************************************************
 *
 * Function: ge_phy_init
 *
 * This function initialize the PHY.
 *
 * Input:  phy address
 *
 * Output: PASSED/FAILED
 *
 *******************************************************************************
 */
int ge_phy_init(uint phy_addr)
{
    int retval = PASSED;
    ushort data;

    /* Enable PHY Rx delay, disable Tx Delay */
    if (diagflag_xram & D_TRACE) {
        printf("Enable PHY Rx delay, disable Tx Delay\n");
    }
    retval = dev_phy_read_reg(MRV88E1512_REG_PAGE_2, 
                     MRV88E1512_MAC_CNTL_REG2, &data);
    if (retval != PASSED) {
        buf_p += sprintf(buf_p, "%s(): phy smi read failed\n",
                                 __FUNCTION__);
        return (retval);
    }

    data |= MRV88E1512_P2_R21_RX_CTRL;
    data &= ~MRV88E1512_P2_R21_TX_CTRL;
    retval = dev_phy_write_reg(MRV88E1512_REG_PAGE_2, 
                      MRV88E1512_MAC_CNTL_REG2, &data);
    if (retval != PASSED) {
        buf_p += sprintf(buf_p, "%s(): Failed to set MAC Control2.\n", 
                    __FUNCTION__);
        return (retval);
    }

    /* Mode: RGMII(system side) and SGMII(media side) */
    if (diagflag_xram & D_TRACE) {
        printf("RGMII(system side) and SGMII(media side)\n");
    }
    retval = dev_phy_read_reg(MRV88E1512_REG_PAGE_18, 
                     MRV88E1512_GEN_CTRL1, &data);
    if (retval != PASSED) {
        buf_p += sprintf(buf_p, "%s(): phy smi read failed\n",
                                 __FUNCTION__);
        return (retval);
    }

    /* Write 4 to Mode field. Defines RGMII (system side) and SGMII (media side) */
    data &= ~MRV88E1512_P18_R20_MODE;
    data |= 4;
    retval = dev_phy_write_reg(MRV88E1512_REG_PAGE_18, 
                      MRV88E1512_GEN_CTRL1, &data);
    if (retval != PASSED) {
        buf_p += sprintf(buf_p, "%s(): Set General Control 1 "
                                "failed.\n", __FUNCTION__);
        return (retval);
    }

    /* Phy mode software reset */
    if (diagflag_xram & D_TRACE) {
        printf("Phy mode software reset\n");
    }
    retval = retval2dev_88e1512_mode_soft_reset(phy_addr);
    if (retval != PASSED) {
        return (retval);
    }

    /* Enable interrupts on LED2 pin */
    if (diagflag_xram & D_TRACE) {
        printf("Enable interrupts on LED2 pin\n");
    }
    retval = dev_phy_read_reg(MRV88E1512_REG_PAGE_3, 
                     MRV88E1512_LED_TIMER_CTRL, &data);
    if (retval != PASSED) {
        buf_p += sprintf(buf_p, "%s(): phy smi read failed\n",
                                 __FUNCTION__);
        return (retval);
    }

    data |= MRV88E1512_P3_R18_INT_EN;
    retval = dev_phy_write_reg(MRV88E1512_REG_PAGE_3, 
                      MRV88E1512_LED_TIMER_CTRL, &data);
    if (retval != PASSED) {
        buf_p += sprintf(buf_p, "%s(): Enable LED Timer Interrupt "
                                "failed.\n", __FUNCTION__);
        return (retval);
    }

    /* Enable MAC interrupts */
    if (diagflag_xram & D_TRACE) {
        printf("Enable MAC interrupts\n");
    }
    retval = dev_phy_read_reg(MRV88E1512_REG_PAGE_2, 
                     MRV88E1512_MAC_SPEC_INT_EN, &data);
    if (retval != PASSED) {
        buf_p += sprintf(buf_p, "%s(): phy smi read failed\n",
                                 __FUNCTION__);
        return (retval);
    }

    data |= MRV88E1512_P2_R18_OVER_EN | 
            MRV88E1512_P2_R18_IDLE_INS_EN |
            MRV88E1512_P2_R18_IDLE_DEL_EN;
    retval = dev_phy_write_reg(MRV88E1512_REG_PAGE_2, 
                      MRV88E1512_MAC_SPEC_INT_EN, &data);
    if (retval != PASSED) {
        buf_p += sprintf(buf_p, "%s(): Enable MAC Specific Interrupt "
                                "failed.\n", __FUNCTION__);
        return (retval);
    }

    /* Set SGMII Output Amplitude */
    if (diagflag_xram & D_TRACE) {
        printf("Set SGMII Output Amplitude\n");
    }
    retval = dev_phy_read_reg(MRV88E1512_REG_PAGE_1, 
                     MRV88E1512_SPECIFIC_CONTROL2_REG, &data);
    if (retval != PASSED) {
        buf_p += sprintf(buf_p, "%s(): phy smi read failed\n",
                                 __FUNCTION__);
        return (retval);
    }

    data &= ~0x7;
    data |= 0x6;
    retval = dev_phy_write_reg(MRV88E1512_REG_PAGE_1, 
                      MRV88E1512_SPECIFIC_CONTROL2_REG, &data);
    if (retval != PASSED) {
        buf_p += sprintf(buf_p, "%s(): Set SGMII Output Amplitude "
                                "failed.\n", __FUNCTION__);
        return (retval);
    }

    return (retval);
}

int phy_int_lpbk_test()
{
    int rc = PASSED;

    testname("PHY Loopback");

    /* Diable MAC Internal Loopback */
    set_mac_loopback(GE_LPBK_MAC_EXT);

    /* Initialize GE PHY */
    printf("\nInitialize PHY.\n");
    if (ge_phy_init(PRINCE_PHY_ADDR)) {
        cterr('f', 0, "PHY Initialize Failed.");
        buf_p = err_msg;
        phy_88e1512_err_report(buf_p, MRVL_88E1512_INIT);
        return (FAILED);
    }

    /* Set loopback mode and speed */
    dev_88e1512_set_lpbk(PRINCE_PHY_ADDR, ETH_MODE_GE, SGMII_PHY_LPBK_INTERNAL);

    /* Start loopback test */
    rc = prince_pkt_lpbk_test();

    if (rc != PASSED) {
        cterr('f', 0, "PHY Internal Loopback Failed.");
        phy_reg_dp();
        return rc;
    }

    /* Clear internal loopback mode if applied */
    dev_88e1512_cleanup_lpbk(PRINCE_PHY_ADDR);

    return (rc);
}

int phy_int_lpbk_test_raw_skt()
{
    int rc = PASSED;

    testname("PHY Loopback");

    /* Diable MAC Internal Loopback */
    set_mac_loopback(GE_LPBK_MAC_EXT);

    /* Set loopback mode and speed */
    printf("Enable PHY internal loopback.\n");
    dev_88e1512_set_lpbk(PRINCE_PHY_ADDR, ETH_MODE_GE, SGMII_PHY_LPBK_INTERNAL);

    /* Set output amplitude per HW request */
    printf("Set output amplitude.\n");
    dev_88e1512_set_out_amp(PRINCE_PHY_ADDR);

    /* Start loopback test */
    printf("Start Packet Loopback Test...\n");
    rc = prince_set_packet(0, SPD_1000MBPS);

    /* Clear internal loopback mode if applied */
    dev_88e1512_cleanup_lpbk(PRINCE_PHY_ADDR);

    if (rc != PASSED) {
        cterr('f', 0, "PHY Internal Loopback Failed.");
        phy_reg_dp();
        return rc;
    }

    return (rc);
}

int phy_ext_lpbk_test()
{
    int rc = PASSED;

    testname("PHY External Loopback");

    /* Diable MAC Internal Loopback */
    set_mac_loopback(GE_LPBK_MAC_EXT);

    /* Initialize GE PHY */
    printf("\nInitialize PHY.\n");
    if (ge_phy_init(PRINCE_PHY_ADDR)) {
        cterr('f', 0, "PHY Initialize Failed.");
        buf_p = err_msg;
        phy_88e1512_err_report(buf_p, MRVL_88E1512_INIT);
        return (FAILED);
    }

    /* Set loopback mode and speed */
    printf("Start PHY External Loopback. "
        "Please enable Backplane line loopback first.\n");
    dev_88e1512_set_lpbk(PRINCE_PHY_ADDR, ETH_MODE_GE, SGMII_LPBK_NONE);

    /* Start loopback test */
    rc = prince_pkt_lpbk_test();

    if (rc != PASSED) {
        cterr('f', 0, "PHY External Loopback Failed.");
        phy_reg_dp();
    }

    return (rc);
}

int phy_ext_lpbk_test_raw_skt()
{
    int rc = PASSED;

    testname("PHY External Loopback");

    /* Diable MAC Internal Loopback */
    set_mac_loopback(GE_LPBK_MAC_EXT);

    /* Set loopback mode and speed */
    printf("Start PHY External Loopback. "
        "Please enable Backplane line loopback first.\n");
    dev_88e1512_set_lpbk(PRINCE_PHY_ADDR, ETH_MODE_GE, SGMII_LPBK_NONE);

    /* Set output amplitude per HW request */
    printf("Set output amplitude.\n");
    dev_88e1512_set_out_amp(PRINCE_PHY_ADDR);

    /* Start loopback test */
    printf("Start Packet Loopback Test...\n");
    rc = prince_set_packet(0, SPD_1000MBPS);

    if (rc != PASSED) {
        cterr('f', 0, "PHY External Loopback Failed.");
        phy_reg_dp();
    }

    return (rc);
}

int phy_reg_rd()
{
    ushort offset;
    ushort page;
    ushort reg_data;
    ulong *reg_p;

    page = getdec_answer("\nEnter register page[0 to 18]:",
               0, 0, 18);
    offset = getdec_answer("\nEnter register offset[0 to 31]:",
               0, 0, 31);

    if (dev_phy_read_reg(page, offset, &reg_data)) {
        perror("Failed to read PHY reg in Page %d Offset %d.", page, offset);
        return (FAILED);
    } else {
        printf("\n Register value %#x ", reg_data);
        return (PASSED);
    }
}

int phy_reg_wr()
{
    ushort page;
    ushort offset;
    ushort reg_data;

    page = getdec_answer("\nEnter register page[0 to 18]:",
           0, 0, 18);
    offset = getdec_answer("\nEnter register offset[0 to 31]:",
             0, 0, 31);
    reg_data = gethex_answer("\nEnter write value[0x0 to 0xFFFF]:",
               0, 0, 0xffff);
   
    if (dev_phy_write_reg(page, offset, &reg_data)) {
        perror("Failed to write PHY reg in Page %d Offset %d.", page, offset);
        return (FAILED);
    } else {
        printf("\n Write Register value %#x to page %d Offset %d", 
        reg_data, page, offset);
        return (PASSED);
    }
}

int phy_reg_dp()
{
    int i;
    ushort data;
    ushort page;
    const reg_info_t *reg_ptr;
    const mrvl_88e1512_phy_regs_t *page_reg_ptr = 
                                   &marvell_88e1512_phy_reg_tbl[0];

    printf("PHY Register Dump");

    for (i = 0; i < NUM_PHY_PAGES; i++) {
        printf("\n%s\n", page_reg_ptr->pagename);
        page = page_reg_ptr->pagenum;
        reg_ptr = page_reg_ptr->pageregs;
        while (reg_ptr->size.size != 0) {
            if (dev_phy_read_reg(page, reg_ptr->offset, &data)) {
                cterr('f', 0, "Failed to read PHY reg in Page %d Offset %d.", 
                    page, reg_ptr->offset);
                return (FAILED);
            }
            printf("%-32s reg %.2d = %#.4x\n", reg_ptr->name, 
                                               reg_ptr->offset, 
                                               data);
            reg_ptr++;
        }
        page_reg_ptr++;
    }
 
    return (PASSED);
}

/******** History ********
$Log: prince_ge_phy.c,v $
Revision 1.3  2014/01/13 03:07:05  xiaoyizh
Per HW's request, modified the SGMII output amplitude.
Add new routine for PHY external loopback test using raw socket.

Revision 1.2  2013/08/02 09:08:47  xiaoyizh
Modify some registers' attribute.
Add new PHY loopback test routine using raw socket.

Revision 1.1  2013/04/19 07:17:51  xiaoyizh
Initial check in for Prince NIM.

$Endlog$
*/
