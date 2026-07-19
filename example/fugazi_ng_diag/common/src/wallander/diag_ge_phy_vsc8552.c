/* $Id: diag_ge_phy_vsc8552.c,v 1.1 2015/02/26 07:18:29 xiaoyizh Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/wallander/diag_ge_phy_vsc8552.c,v $
 *------------------------------------------------------------------
 *
 * diag_ge_phy_vsc8552.c - Wallander PHY VSC8552 functions.
 *
 * Xiaoying Zhang -- Mar. 2014
 *
 * Copyright (c) 2015 by Cisco Systems, Inc.
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
// #include "pcmap.h"
#include "nvsysvars.h"
// #include "nvmonvars.h"
#include "diag_vtss_phy.h"

// static void phy_88e1512_err_report (uchar *err_msg, uint32 err_id);
// extern int fd_prc;

static reg_info_t vsc8552_reg_page0[] = {   /* Page 0 */
    /* IEEE 802.3 Registers */
    {"Mode Control",            0x00, READ_ONLY,  {2}, 0x7DC0, 0x1040},
    {"Mode Status",             0x01, READ_ONLY,  {2}, 0x0000, 0x79C9},
    {"PHY ID1",                 0x02, READ_ONLY,  {2}, 0x0000, 0x0007},
    {"PHY ID2",                 0x03, READ_ONLY,  {2}, 0x0000, 0x07c0},
    {"Auto-Neg Adv",            0x04, READ_ONLY,  {2}, 0xBFFF, 0x01e1},
    {"Auto-Neg Link-P Abil",    0x05, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"Auto-Neg Exp",            0x06, READ_ONLY,  {2}, 0x0000, 0x0004},
    {"Auto-Neg Next Page Tx",   0x07, READ_ONLY,  {2}, 0xB7FF, 0x2001},
    {"Auto-Neg Link-P Rx",      0x08, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"1000BT Control",          0x09, READ_ONLY,  {2}, 0xFF00, 0x0700},
    {"1000BT Status",           0x0A, READ_ONLY,  {2}, 0x0000, 0x4000},
    {"MMD Access Ctrl",         0x0D, READ_ONLY,  {2}, 0xC01F, 0x0000},
    {"MMD Addr or Data",        0x0E, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"1000BT Extended Status",  0x0F, READ_ONLY,  {2}, 0x0000, 0x3000},

    /* Main registers */
    {"100BT Status Ext",        0x10, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"100BT Status ext",        0x11, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"Bypass Control",          0x12, READ_WRITE, {2}, 0xFEFE, 0x0088},
    {"Err Counter 1",           0x13, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"Err Counter 2",           0x14, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"Err Counter 3",           0x15, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"Extended Ctrl & Status",  0x16, READ_WRITE, {2}, 0xFE01, 0x3200},
    {"Extended PHY Ctrl 1",     0x17, READ_ONLY,  {2}, 0x1FC8, 0x0000},
    {"Extended PHY Ctrl 2",     0x18, READ_ONLY,  {2}, 0xF031, 0x0000},
    {"Interrupt Mask",          0x19, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"Interrupt Status",        0x1A, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"Aux Ctrl & Status",       0x1C, READ_WRITE, {2}, 0x00C4, 0x0004},
    {"LED Mode Select",         0x1D, READ_WRITE, {2}, 0xFFFF, 0x8021},
    {"LED behavior",            0x1E, READ_WRITE, {2}, 0xDDEF, 0x0800},
    {"Extended page access",    0x1F, READ_ONLY,  {2}, 0xFFFF, 0x0000},
    {"end",                     0x00, 0, {0}, 0, 0},
};

static reg_info_t vsc8552_reg_page1[] = {  /* Extended Page 1 */
    {"SerDes Media Ctrl",       0x10, READ_ONLY,  {2}, 0xC300, 0x0300},
    {"Cu Media CRC good cnt",   0x12, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"Extended Mode Ctrl",      0x13, READ_ONLY,  {2}, 0xF81C, 0x0000},
    {"Extended PHY ctrl 3",     0x14, READ_ONLY,  {2}, 0xFE3C, 0x4004},
    {"Extended PHY ctrl 4",     0x17, READ_ONLY,  {2}, 0x0400, 0x0000},
    {"VeriPHY 1",               0x18, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"VeriPHY 2",               0x19, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"VeriPHY 3",               0x1a, READ_ONLY,  {2}, 0xC24F, 0x0042},
    {"Eth Pkt Gen 1",           0x1D, READ_WRITE, {2}, 0xFFFF, 0x0040},
    {"Eth Pkt Gen 2",           0x1E, READ_WRITE, {2}, 0xFFFF, 0x0000},
    {"end",                     0x00, 0, {0}, 0, 0},
};

static reg_info_t vsc8552_reg_page2[] = {   /* Extended Page 2 */
    {"Cu PMD Tx Ctrl",          0x10, READ_WRITE, {2}, 0xFFFF, 0x02BE},
    {"EEE Ctrl",                0x11, READ_WRITE, {2}, 0xFC3F, 0x0000},
    {"Extended Chip ID",        0x12, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"Extended Intr Mask",      0x1C, READ_ONLY,  {2}, 0x07FF, 0x0000},
    {"Extended Intr Status",    0x1D, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"Ring Resiliency Ctrl",    0x1E, READ_ONLY,  {2}, 0xB000, 0x0000},
    {"end",                     0x00, 0, {0}, 0, 0},
};

static reg_info_t vsc8552_reg_page3[] = {   /* Extended Page 3 */
    {"MAC SerDes PCS Ctrl",     0x10, READ_ONLY,  {2}, 0xFFFC, 0x0004},
    {"MAC SerDes PCS Status",   0x11, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"MAC SerDes C37 Adv",      0x12, READ_ONLY,  {2}, 0xFFFF, 0x0000},
    {"MAC SerDes C37 Link-P",   0x13, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"MAC SerDes Status",       0x14, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"Med/MAC SerDes Tx-good Cnt",  0x15, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"Med/MAC SerDes CRC-err Cnt",  0x16, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"Media SerDes PCS Ctrl",   0x17, READ_ONLY,  {2}, 0x2877, 0x0010},
    {"Media SerDes PCS Status", 0x18, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"Media SerDes C37 Adv",    0x19, READ_ONLY,  {2}, 0xFFFF, 0x0000},
    {"Media SerDes C37 Link-P", 0x1A, READ_ONLY,  {2}, 0xC24F, 0x0042},
    {"Med/MAC SerDes Status",   0x1B, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"Med/MAC SerDes Rx CRC Cnt",   0x1C, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"Media CRC Error Cnt",     0x1D, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"Freq-Offset Measure Ctrl",0x1E, READ_ONLY,  {2}, 0x030C, 0x000C},
    {"end",                     0x00, 0, {0}, 0, 0},
};

static reg_info_t vsc8552_reg_page16[] = {   /* Page 16*/
    /* General Purpose Registers */
    {"LED/SIGDET/GPIO Ctrl",    0x0D, READ_ONLY,    {2}, 0xFFFF, 0x0000},
    {"GPIO Ctrl 2",             0x0E, READ_ONLY,    {2}, 0xFCFF, 0x2600},
    {"GPIO Input",              0x0F, READ_ONLY,    {2}, 0x3FFF, 0x0000},
    {"GPIO Output",             0x10, READ_ONLY,    {2}, 0x3FFF, 0x0000},
    {"GPIO Output Enable",      0x11, READ_ONLY,    {2}, 0x3FFF, 0x0000},
    {"Micro Command",           0x12, READ_ONLY,    {2}, 0x0000, 0x0000},
    {"MAC Mode & Fast Link Config", 0x13, READ_ONLY, {2}, 0xC00F, 0x000F},
    {"Two-Wire Serial MUX Ctrl1",   0x14, READ_ONLY, {2}, 0xFE3F, 0xA010},
    {"Two-Wire Serial MUX Ctrl2",   0x15, READ_ONLY, {2}, 0x0FFF, 0x0100},
    {"Two-Wire Serial MUX R/W", 0x16, READ_ONLY,    {2}, 0x00FF, 0x0000},
    {"Recovered Clk1 Ctrl",     0x17, READ_ONLY,    {2}, 0xFF37, 0x0000},
    {"Recovered Clk2 Ctrl",     0x18, READ_ONLY,    {2}, 0xFF37, 0x0000},
    {"Enhanced LED Ctrl",       0x19, READ_ONLY,    {2}, 0xFFFF, 0x0000},
    {"Global Intr Status",      0x1d, READ_ONLY,    {2}, 0x0000, 0x0000},
    {"end",                     0x00, 0, {0}, 0, 0},
};

vsc_phy_regs_t vsc8552_phy_reg_tbl[] = {
    {"Page  0",     0, vsc8552_reg_page0},
    {"Page  1",     1, vsc8552_reg_page1},
    {"Page  2",     2, vsc8552_reg_page2},
    {"Page  3",     3, vsc8552_reg_page3},
//    {"Page  4",     4, vsc8552_reg_page4},
    {"Page 16",    16, vsc8552_reg_page16},
};

/******** History ********
$Log: diag_ge_phy_vsc8552.c,v $
Revision 1.1  2015/02/26 07:18:29  xiaoyizh
Initial check in for Wallander.


$Endlog$
*/
