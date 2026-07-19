/* $Id: dreamliner_phy.c,v 1.10 2020/01/09 01:02:10 jiajliu Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/dreamliner/dreamliner_phy.c,v $
 *------------------------------------------------------------------
 *
 * dreamliner_phy.c - This file contains functions to init and control
 *                    Marvell 88E1680 PHY.
 *
 * Christine Wen -- Nov. 2013
 *
 * Copyright (c) 2014-2019 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */
#include "common.h"
#include "types.h"
#include "nvmonvars.h"
#include "nvsysvars.h"
#include "defs.h"
#include "menu.h"
#include "error.h"
#include "common_utils.h"
#include "proto.h"
#include "strings.h"
#include "queryflags.h"
#include "plat_defs.h"
#include "dash_fpga.h"

#include "madApi.h"
#include "madHwCntl.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <stdint.h> 

#include "dreamliner.h"
#include "dreamliner_ge_switch.h"
#include "dreamliner_phy.h"

#ifdef TACHI_INTEL
#include "nim_test_defs.h"
#endif 

extern void testMADDisplayStatus(MAD_STATUS status);
extern int get_ctrl_plane_sgmii_port(void); 
extern int get_sgmii_port_num(uint, uint);

static int peek_phy_reg();
static int poke_phy_reg();
static int display_phy_reg();
static int reset_phy_port();
static int phy_disable_int(MAD_DEV *dev);
static int phy_set_test_mode();
static unsigned int phy_read_reg (MAD_DEV *dev, MAD_LPORT port_num, MAD_U16 page_num, MAD_U16 reg_addr, MAD_U32 *data);
static unsigned int phy_write_reg (MAD_DEV *dev,MAD_LPORT port_num, MAD_U16 page_num, MAD_U16 reg_addr, MAD_U16 data);

static int phy_init(int port);
static int phy_config(int port);
static void print_phy_counter();
static void clear_phy_counter();
static void dump_phy_reg();
static int reset_phy();

#ifdef TACHI_INTEL 
static void clr_bridge_nim_ge_port(void);
static void phy_cfg_dbg0();
static void phy_cfg_dbg1();
static int port_setup_bdg(int, int);
static void clr_cnt_sw_phy(void);
static void print_cnt_sw_phy(void);
static void clr_force_link_down_ge_port(void);
void phy_lpbk_ge_clr(int, int);
void phy_lpbk_ge_init(int, int);
static void test_ge_init(void);
static void test_ge_clr(void);
#endif 

/* submenu for PHY utilities */
submenu_xtable_t phy_util_submenu_table[] = {
    {"peek PHY register",  
     (PFT)peek_phy_reg,     0, 0, (type_t(*)())0, 0, 
     (type_t(*)())0, 0},
    {"poke PHY register",  
     (PFT)poke_phy_reg,     0, 0, (type_t(*)())0, 0, 
     (type_t(*)())0, 0},
    {"display PHY registers",  
     (PFT)display_phy_reg,  0, 0, (type_t(*)())0, 0, 
     (type_t(*)())0, 0},
    {"soft reset PHY port",  
     (PFT)reset_phy_port,   0, 0, (type_t(*)())0, 0, 
     (type_t(*)())0, 0},
    {"Set Marvell PHY Test Mode",   
     (PFT)phy_set_test_mode,   0, 0, (type_t(*)())0, 0, 
     (type_t(*)())0, 0}, 
#ifdef TACHI_INTEL
    {"Reset PHY", 
     (PFT)reset_phy,   0, 0, (type_t(*)())0, 0, 
    (type_t(*)())0, 0},
    {"Clear Force Link down GE ", 
     (PFT)clr_force_link_down_ge_port,   0, 0, (type_t(*)())0, 0, 
    (type_t(*)())0, 0},
    {"Clean up bridge NIM GE with Dreamliner PHY ports",
     (PFT)clr_bridge_nim_ge_port,             0, 0, (type_t(*)())0, 0,
    (type_t(*)())0, 0},
    {"Configure PHY and GE0 switch for ext. lpbk",  
     (PFT)phy_cfg_dbg0,   0, 0, (type_t(*)())0, 0, 
    (type_t(*)())0, 0},
    {"Configure PHY and GE1 switch for ext. lpbk",  
     (PFT)phy_cfg_dbg1,   0, 0, (type_t(*)())0, 0, 
    (type_t(*)())0, 0},
    {"print phy sw counter",
     (PFT)print_cnt_sw_phy,   0, 0, (type_t(*)())0, 0, 
    (type_t(*)())0, 0},
    {"clr phy sw counter",  
     (PFT)clr_cnt_sw_phy,   0, 0, (type_t(*)())0, 0, 
    (type_t(*)())0, 0},
    {"set lpbk", 
     (PFT)test_ge_init,   0, 0, (type_t(*)())0, 0, 
    (type_t(*)())0, 0},
    {"clr lpbk", 
     (PFT)test_ge_clr,   0, 0, (type_t(*)())0, 0, 
    (type_t(*)())0, 0},
#endif 
};

#define PHY_UTIL_SUBMENU_TABLE_SIZE (sizeof(phy_util_submenu_table) / \
                                       sizeof(submenu_xtable_t))

/* 
 * Primary & secondary submenu items (filled in from xtable)
 */
static mitem_t phy_util_primary_items[PHY_UTIL_SUBMENU_TABLE_SIZE +
						MAX_BASE_ITEMS];
static mitem_t phy_util_secondary_items[PHY_UTIL_SUBMENU_TABLE_SIZE +
						MAX_BASE_ITEMS];

menuinfo_t phy_util_menu = {
    "Marvell PHY Utility Menu",
    0,                                /* mtparam added by init_empty_menu */
    (PFT)show_endnote,                /* notes missing WICs in combos */
    0,                                /* use generic prompt */
    0,                                /* size (bumped by add_menu_item() */
    phy_util_primary_items,
};
menuinfo_t *phy_submenup = &phy_util_menu;


static const reg_info_t marvell_88e1680_reg_page0[] = {   /* Page 0*/
    {"Copper Control",      0x00, READ_ONLY,  {2}, 0x3140, 0x1940},
    {"Copper Status",       0x01, READ_ONLY,  {2}, 0x0000, 0x7949},
    {"PHY ID1",             0x02, READ_ONLY,  {2}, 0x0000, 0x0141},
    {"PHY ID2",             0x03, READ_ONLY,  {2}, 0x0000, 0x0eb0},
    {"Copper Auto-Neg",     0x04, READ_ONLY,  {2}, 0xA21F, 0x01e1},
    {"Copper Link-P Abil",  0x05, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"Copper Auto-Neg Exp", 0x06, READ_ONLY,  {2}, 0x0000, 0x0004},
    {"Copper Next Page",    0x07, READ_ONLY,  {2}, 0xB7FF, 0x2001},
    {"Copper Link Partner", 0x08, READ_ONLY,  {2}, 0x0000, 0x0000},    
    {"1000BT Control",      0x09, READ_ONLY,  {2}, 0xF200, 0x0f00},
    {"1000BT Status",       0x0A, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"MMD Control",         0x0D, READ_ONLY,  {2}, 0xC000, 0x0000},
    {"MMD Addr/Data",       0x0E, READ_WRITE, {2}, 0xFFFF, 0x0000},
    {"Extended Status",     0x0F, READ_ONLY,  {2}, 0x0000, 0x3000},
    {"Copper Spec Cntl1",   0x10, READ_ONLY,  {2}, 0x0000, 0x3060},
    {"Copper Spec Ststus",  0x11, READ_ONLY,  {2}, 0x0000, 0x8040},
    {"Copper Spec Intr Ena",0x12, READ_ONLY,  {2}, 0xFFFF, 0x0000},
    {"Copper Intr Status",  0x13, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"Copper Spec Cntl2",   0x14, READ_ONLY,  {2}, 0x0000, 0x0020},
    {"Copper Spec Rx Err",  0x15, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"Page Register",       0x16, READ_WRITE, {2}, 0x0000, 0x0000},
    {"Global Intr Status",  0x17, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"Copper Spec Cntl3",   0x1A, READ_ONLY,  {2}, 0x0000, 0x0040},
    {"end",                 0x00, 0, {0}, 0, 0},
};

static const reg_info_t marvell_88e1680_reg_page2[] = {   /* Page 2*/
    {"MAC Spec Cntl1",      0x10, READ_ONLY,  {2}, 0x0000, 0x4008},
    {"MAC Spec Intr Ena",   0x12, READ_ONLY,  {2}, 0x008C, 0x0000},
    {"MAC Intr Status",     0x13, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"MAC Spec Cntl2",      0x15, READ_ONLY,  {2}, 0x0000, 0x1046},
    {"end",                 0x00, 0, {0}, 0, 0},
};

static const reg_info_t marvell_88e1680_reg_page3[] = {   /* Page 3*/
    {"LED Func Cntl1",      0x10, READ_WRITE, {2}, 0x00FF, 0x111e},
    {"LED Polarity Cntl",   0x11, READ_WRITE, {2}, 0xFF0F, 0x8800},
    {"LED Timer Cntl",      0x12, READ_WRITE, {2}, 0xF70F, 0x4b05},
    {"LED Func Cntl&Polar", 0x13, READ_WRITE, {2}, 0x0000, 0x0073},
    {"end",                 0x00, 0, {0}, 0, 0},
};

static const reg_info_t marvell_88e1680_reg_page4[] = {   /* Page 4*/
    {"QSGMII Control",      0x00, READ_ONLY,  {2}, 0x4000, 0x1140},
    {"QSGMII Status",       0x01, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"QSGMII Spec Cntl1",   0x10, READ_ONLY,  {2}, 0xF0C0, 0x6244},
    {"QSGMII Spec Status",  0x11, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"QSGMII Spec Intr Ena",0x12, READ_ONLY,  {2}, 0x7F80, 0x0000},
    {"QSGMII Intr Status",  0x13, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"QSGMII RX_ER Byte",   0x14, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"QSGMII Rx Err Cnt",   0x15, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"Page Register",       0x16, READ_WRITE, {2}, 0x0000, 0x0000},
    {"PRBS Control",        0x17, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"PRBS Err Cnt LSB",    0x18, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"PRBS Err Cnt MSB",    0x19, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"QSGMII Global Cntr1", 0x1A, READ_ONLY,  {2}, 0x3204, 0xC000},
    {"QSGMII Global Cntr2", 0x1B, READ_ONLY,  {2}, 0x4103, 0x3f80},
    {"end",                 0x00, 0, {0}, 0, 0},
};

static const reg_info_t marvell_88e1680_reg_page5[] = {   /* Page 5*/
    {"Adv VCT TX MDI0",     0x10, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"Adv VCT TX MDI1",     0x11, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"Adv VCT TX MDI2",     0x12, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"Adv VCT TX MDI3",     0x13, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"1000BT Pair Skew",    0x14, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"1000BT Pair Swap",    0x15, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"Page Register",       0x16, READ_WRITE, {2}, 0x0000, 0x0000},
    {"Adv VCT Control",     0x17, READ_ONLY,  {2}, 0x3FFF, 0x0603},
    {"Adv VCT Sample point",0x18, READ_ONLY,  {2}, 0x03FF, 0x000},
    {"Adv VCT Cross Pair",  0x19, READ_ONLY,  {2}, 0x7F7F, 0x0104},
    {"Adv VCT Same Pair 01",0x1A, READ_ONLY,  {2}, 0x7F7F, 0x0F12},
    {"Adv VCT Same Pair 23",0x1B, READ_ONLY,  {2}, 0x7F7F, 0x0A0C},
    {"Adv VCT Same Pair 4", 0x1C, READ_ONLY,  {2}, 0x3FFF, 0x0006},
    {"end",                 0x00, 0, {0}, 0, 0},
};

static const reg_info_t marvell_88e1680_reg_page6[] = {   /* Page 6*/
    {"Packet Generation",   0x10, READ_ONLY,  {2}, 0xFF07, 0x0000},
    {"CRC Counters",        0x11, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"Checker Control",     0x12, READ_ONLY,  {2}, 0x0008, 0x0000},
    {"Copper Port IPG Cntl",0x13, READ_ONLY,  {2}, 0x00FF, 0x000B},
    {"General Control",     0x14, READ_ONLY,  {2}, 0x0000, 0x0200},
    {"Page Register",       0x16, READ_WRITE, {2}, 0x0000, 0x0000},
    {"Late Colli Cnt1&2",   0x17, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"Late Colli Cnt3&4",   0x18, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"Late Colli Window",   0x19, READ_WRITE, {2}, 0x1F00, 0x0000},
    {"Misc Test",           0x1A, READ_WRITE, {2}, 0x9F00, 0x1900},
    {"Temperature Sensor",  0x1B, READ_WRITE, {2}, 0x1F00, 0x0C00},
    {"end",                 0x00, 0, {0}, 0, 0},
};

static const reg_info_t marvell_88e1680_reg_page7[] = {   /* Page 7*/
    {"PHY Cable Diag 0",    0x10, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"PHY Cable Diag 1",    0x11, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"PHY Cable Diag 2",    0x12, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"PHY Cable Diag 3",    0x13, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"PHY Cable Diag Relt", 0x14, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"PHY Cable Diag Cntl", 0x15, READ_ONLY,  {2}, 0x0400, 0x4000},
    {"Page Register",       0x16, READ_WRITE, {2}, 0x0000, 0x0000},
    {"Adv VCT Cros Pair",   0x19, READ_WRITE, {2}, 0x7F7F, 0x0104},
    {"Adv VCT Same Pair 01",0x1A, READ_WRITE, {2}, 0x7F7F, 0x0F12},
    {"Adv VCT Same Pair 23",0x1B, READ_WRITE, {2}, 0x7F7F, 0x0A0C},
    {"Adv VCT Same Pair 4", 0x1C, READ_WRITE, {2}, 0x007F, 0x0006},
    {"end",                 0x00, 0, {0}, 0, 0},
};

static const reg_info_t marvell_88e1680_reg_page8[] = {   /* Page 8*/
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

static const reg_info_t marvell_88e1680_reg_page9[] = {   /* Page 9*/
    {"PTP Dep Status",      0x00, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"PTP Dep Byte1&0",     0x01, READ_ONLY,  {2}, 0x0000, 0x0000},     
    {"PTP Dep Byte3&2",     0x02, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"PTP Dep Sequ ID",     0x03, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"PTP Dep Cnt",         0x05, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"Page Register",       0x16, READ_WRITE, {2}, 0x0000, 0x0000},
    {"end",                 0x00, 0, {0}, 0, 0},
};

static const reg_info_t marvell_88e1680_reg_page12[] = {   /* Page 12*/
    {"TAI Global Conf 0",   0x00, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"TAI Global Conf 1",   0x01, READ_ONLY,  {2}, 0x0000, 0x1F40},     
    {"TAI Global Conf 2",   0x02, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"TAI Global Conf 3",   0x03, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"TAI Global Conf 4",   0x04, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"TAI Global Conf 5",   0x05, READ_ONLY,  {2}, 0x0000, 0xF000},
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

static const reg_info_t marvell_88e1680_reg_page14[] = {   /* Page 14*/
    {"PTP Global Conf 0",   0x00, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"PTP Global Conf 1",   0x01, READ_ONLY,  {2}, 0x0000, 0x0000},     
    {"PTP Global Conf 2",   0x02, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"PTP Global Conf 3",   0x03, READ_ONLY,  {2}, 0x0000, 0x0001},
    {"PTP Global Status",   0x08, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"Page Register",       0x16, READ_WRITE, {2}, 0x0000, 0x0000},
    {"end",                 0x00, 0, {0}, 0, 0},
};

typedef struct mrvl_88e1680_phy_regs_t_
{
    const char *pagename;
    uint32_t    pagenum;
    const reg_info_t *pageregs;
} mrvl_88e1680_phy_regs_t;

static const mrvl_88e1680_phy_regs_t marvell_88e1680_phy_reg_tbl[] = {
    {"Page  0",     0, marvell_88e1680_reg_page0},
    {"Page  2",     2, marvell_88e1680_reg_page2},
    {"Page  3",     3, marvell_88e1680_reg_page3},
    {"Page  4",     4, marvell_88e1680_reg_page4},
    {"Page  5",     5, marvell_88e1680_reg_page5},
    {"Page  6",     6, marvell_88e1680_reg_page6},
    {"Page  7",     7, marvell_88e1680_reg_page7},
    {"Page  8",     8, marvell_88e1680_reg_page8},
    {"Page  9",     9, marvell_88e1680_reg_page9},
    {"Page 12",    12, marvell_88e1680_reg_page12},
    {"Page 14",    14, marvell_88e1680_reg_page14},
};

#define NUM_PHY_PAGES (sizeof(marvell_88e1680_phy_reg_tbl) /      \
                             sizeof(struct mrvl_88e1680_phy_regs_t_))

static MAD_DEV phy_dev[3];
boolean phy_intr_happened;

/*********************************************************************
 *
 * Function: phy_utils()
 *
 * Description: Build the primary & secondary submenus for the
 * PHY utility menu. 
 *
 * Inputs: none       
 * Outputs: PASSED
 *
 *********************************************************************
 */
int 
phy_utils ()
{
    build_primary_submenu(phy_util_submenu_table, 
			  PHY_UTIL_SUBMENU_TABLE_SIZE,
			  "Marvell PHY Utility", &phy_submenup);
    build_secondary_submenu(phy_util_submenu_table,
			    PHY_UTIL_SUBMENU_TABLE_SIZE,
			    phy_util_secondary_items);

    menu(&phy_util_menu, phy_util_secondary_items, '\0');
    
    return PASSED;
}


/******************************************************************************
 *
 * Function   :	phy_smi_read
 * Description:	read PHY registers through SMI interface.
 * Inputs     :	dev - point to MAD_DEV
 *              smi_addr - PHY smi address
 *              reg_addr - PHY register address
 *              rd_data - point to unsigned int which holds regiter value
 * Outputs    : PASSED/FAILED
 *
 ******************************************************************************
 */
static unsigned int 
phy_smi_read (MAD_DEV *dev, unsigned int smi_addr, unsigned int reg_addr, unsigned int *reg_data)
{
    unsigned int port_num;

    port_num = smi_addr - PHY_ADDR;
    if (smi0_read_reg(port_num, reg_addr, (unsigned short *)reg_data) == PASSED)
	return MAD_TRUE;
    else
	return MAD_FALSE;
}

/******************************************************************************
 *
 * Function   :	phy_smi_write
 * Description:	write PHY registers through SMI interface.
 * Inputs     :	dev - point to MAD_DEV
 *              smi_addr - PHY smi address
 *              reg_addr - PHY register address
 *              data - data to write to PHY regiter
 * Outputs    : PASSED/FAILED
 *
 ******************************************************************************
 */
static unsigned int 
phy_smi_write (MAD_DEV *dev, unsigned int smi_addr, unsigned int reg_addr, unsigned int data)
{
    unsigned int port_num;

    port_num = smi_addr - PHY_ADDR;

    if (smi0_write_reg(port_num, reg_addr, (unsigned short)data) == PASSED)
	return MAD_TRUE;
    else
	return MAD_FALSE;
}


/******************************************************************************
 *
 * Function   :	phy_get_dev_name
 * Description:	get PHY device name from device ID.
 * Inputs     :	device_id
 * Outputs    : device name
 *
 ******************************************************************************
 */
static char * phy_get_dev_name ( MAD_DEVICE_ID device_id)
{
    switch (device_id) {
    case MAD_88E10X0: return ("MAD_88E10X0 ");   	
    case MAD_88E10X0S: return ("MAD_88E10X0S ");   
    case MAD_88E1011: return ("MAD_88E1011 ");   
    case MAD_88E104X: return ("MAD_88E104X ");
    case MAD_88E1111: return ("MAD_88E1111/MAD_88E1115 ");
    case MAD_88E1112: return ("MAD_88E1112 ");
    case MAD_88E1116: return ("MAD_88E1116/MAD_88E1116R ");
    case MAD_88E114X: return ("MAD_88E114X ");
    case MAD_88E1149: return ("MAD_88E1149 ");
    case MAD_88E1149R: return ("MAD_88E1149R ");
    case MAD_SWG65G : return ("MAD_SWG65G ");
    case MAD_88E1181: return ("MAD_88E1181 ");
    case MAD_88E3016: return ("MAD_88E3015/MAD_88E3016/MAD_88E3018/MAD_88E3019");	
/*        case MAD_88E3019: return ("MAD_88E3019 "); */
    case MAD_88E1121: return ("MAD_88E1121/MAD_88E1121R ");
    case MAD_88E3082: return ("MAD_88E3082/MAD_88E3083 ");
    case MAD_88E1240: return ("MAD_88E1240 ");
    case MAD_88E1340S: return ("MAD_88E1340S ");
    case MAD_88E1340: return ("MAD_88E1340 ");
    case MAD_88E1340M: return ("MAD_88E1340M ");
    case MAD_88E1119R: return ("MAD_88E1119R ");
    case MAD_88E1310: return ("MAD_88E1310 ");
    case MAD_88E1510: return ("MAD_88E1510 ");
    case MAD_88E1540: return ("MAD_88E1540 ");
    case MAD_88E1548: return ("MAD_88E1548 ");
    case MAD_88E1680: return ("MAD_88E1680 ");
    case MAD_SW1680: return ("MAD_SW1680 ");
    case MAD_88E3183: return ("MAD_88E3183 ");
    default : return (" No-name ");
    }
} ;

static MAD_SEM 
madOsSemCreate (MAD_SEM_BEGIN_STATE state)
{
    return MAD_OK;
}

static MAD_STATUS
madOsSemDelete (MAD_SEM smid)
{
    return MAD_OK;
}

static MAD_STATUS
madOsSemWait (MAD_SEM smid, MAD_U32 timeout)
{
    return MAD_OK;
}

static MAD_STATUS
madOsSemSignal (MAD_SEM smid)
{
    return MAD_OK;
}

/******************************************************************************
 *
 * Function   :	phy_load_driver
 * Description:	load PHY MAD driver
 * Inputs     :	dev - point to MAD_DEV
 *              smi_addr - PHY smi address
 * Outputs    : PASSED(0)/FAILED(1)
 *
 ******************************************************************************
 */
static unsigned int
phy_load_driver (MAD_DEV *dev, int smi_addr)
{
    MAD_SYS_CONFIG cfg;
    MAD_STATUS status;

    /* clear structures */
    memset((char*)&cfg,0,sizeof(MAD_SYS_CONFIG));
    memset((char*)dev,0,sizeof(MAD_DEV));

    /*
     *  Register all the required functions to MAD driver.
     */
    cfg.BSPFunctions.readMii   = phy_smi_read;
    cfg.BSPFunctions.writeMii  = phy_smi_write;

    cfg.BSPFunctions.semCreate = madOsSemCreate;
    cfg.BSPFunctions.semDelete = madOsSemDelete;
    cfg.BSPFunctions.semTake   = madOsSemWait;
    cfg.BSPFunctions.semGive   = madOsSemSignal;

    cfg.smiBaseAddr = smi_addr;  /* Set SMI Address */

    if((status=mdLoadDriver(&cfg, dev)) != PASSED)
    {
        cterr('f',0,"madLoadDriver return Failed, status = %#x", status);
	return status;
    }

    if ((NVRAM)->diagflag & D_VERBOSE) {
	printf("Device Name   : %s\n", phy_get_dev_name(dev->deviceId));
	printf("Device ID     : 0x%x\n",dev->deviceId);
	printf("Revision      : 0x%x\n",dev->revision);
	printf("Base Reg Addr : 0x%x\n",dev->baseRegAddr);
	printf("No of Ports   : %d\n",dev->numOfPorts);
	printf("MAD has been started.\n");
    }
    return PASSED;
}


/******************************************************************************
 *
 * Function   :	phy_start_driver
 * Description:	This function will do the followings:
 *  1. Load MAD driver for the Marvell Phy mapped to the given SMI address.
 *  2. Enable all the ports in the Marvell Phy device.
 * Inputs     :	smi_addr - base address for PHY
 * Outputs    : PASSED/FAILED
 *
 ******************************************************************************
 */

int
phy_start_driver (int smi_addr)
{
    unsigned char port;
    MAD_DEV *dev = &phy_dev[get_slot_num()-1];

    assert(dev);
    cterr_add_component("Marvell PHY", 
			"SMI0 interface from xCat2 switch");
    cterr_add_debug("Check Marvell PHY",
		    "Check SMI0 interface from xCat2 switch");

    /* load MAD Driver */
    if (phy_load_driver(dev,smi_addr) != PASSED) {
        cterr('f',0,"Failed to load PHY MAD Driver");
        return FAILED;
    }

    /* disable all the interrupts */
    if (phy_disable_int(dev) == FAILED) {
	cterr('f',0,"Failed to disable PHY interrupts.");
	return FAILED;
    }

    /* int all 8 PHYs */
    for(port=0; port<dev->numOfPorts; port++) {
	if (phy_init(port) == FAILED)
	    return (FAILED);

	if (phy_config(port) == FAILED)
	    return (FAILED);
    }

    /* enable all 8 PHYs */
    for(port=0; port<dev->numOfPorts; port++) {
        if(mdSysSetPhyEnable(dev,port,MAD_TRUE) != PASSED) {
            cterr('f',0,"Failed to Enable Phy (port %i)\n",port);
            return FAILED;
        }
    }

    return PASSED;
}

/******************************************************************************
 *
 * Function   :	phy_unload_driver
 * Description:	disable device interrupt and clear MAD_DEV structure
 * Inputs     :	None
 * Outputs    : PASSED(0)/FAILED(1)
 *
 ******************************************************************************
 */
int 
phy_unload_driver ()
{
    MAD_DEV *dev = &phy_dev[get_slot_num()-1];

    assert(dev);

    return (mdUnloadDriver(dev));
}


/******************************************************************************
 *
 * Function   :	start_mac_lpbk
 * Description:	enable mac interface loopback
 * Inputs     :	dev - point to MAD_DEV
 *              port - port number
 *              speed - Speed can be MAD_SPEED_10M, MAD_SPEED_100M, 
 *                      MAD_SPEED_1000M
 * Outputs    : PASSED/FAILED
 *
 ******************************************************************************
 */
unsigned int
start_mac_lpbk (MAD_DEV *dev, int port, MAD_SPEED_MODE speed)
{
    MAD_U32 reg_d;
    int rc;

    if (dev == 0) {
	cterr('f',0,"MAD driver is not initialized.");
        return FAILED;
    }

    /* disable stub test */
    rc = phy_read_reg(dev, port, 6, 18, &reg_d);
    if (rc != PASSED) {
	return (FAILED);
    }
    reg_d &= ~PHY_ENA_STUB_TEST;
    rc = phy_write_reg(dev, port, 6, 18, reg_d);
    if (rc != PASSED) {
	return (FAILED);
    }

    /* set QSGMII speed (page 2, reg 21) */
    reg_d = 0x1046;
    rc = phy_write_reg(dev, port, 2, 21, reg_d);
    if (rc != PASSED) {
	return (FAILED);
    }

    /* force master (page 0, reg 9) */
    reg_d = 0x1f00;
    rc = phy_write_reg(dev, port, 0, 9, reg_d);
    if (rc != PASSED) {
	return (FAILED);
    }

    /* set copper speed (page 0, reg 0) */
    /* 1000Mbps, full-duplex, auto-nego enabled, copper soft reset */
    reg_d = PHY_COPPER_FULL_DUPLEX | PHY_SPD_SEL_1000M | PHY_AUTO_NEO_ENA | PHY_COOPER_RST;
    rc = phy_write_reg(dev, port, 0, 0, reg_d);
    if (rc != PASSED) {
	return (FAILED);
    }
    msleep(100);

    /* set page 0xfa */
    reg_d = 0x0418;
    rc = phy_write_reg(dev, port, 0xfa, 1, reg_d);
    if (rc != PASSED) {
	return (FAILED);
    }
    reg_d = 0x020c;
    rc = phy_write_reg(dev, port, 0xfa, 7, reg_d);
    if (rc != PASSED) {
	return (FAILED);
    }

    msleep(200);

    /* Check copper link speed (page 0, reg 17)*/
    rc = phy_read_reg(dev, port, 0, 17, &reg_d);
    if (rc != PASSED) {
	return (FAILED);
    }

    if ((NVRAM)->diagflag & D_VERBOSE) {
	if ((reg_d & PHY_COPPER_LINK) != PHY_COPPER_LINK) {
	   printf("\nCopper link is not up. register @ (page 0, reg 17) = %#x\n", reg_d);
	} else {
	   printf("\nCopper link is up. register @ (page 0, reg 17) = %#x\n", reg_d);	
	}
    }

    if ((reg_d & PHY_LINK_SPEED_MASK) != PHY_LINK_SPEED_1000) {
	cterr('f',0,"Copper side speed is not 1000Mbps.");
	return (FAILED);
    }  
    if ((reg_d & PHY_FULL_DUPLEX) != PHY_FULL_DUPLEX) {
	cterr('f',0,"Copper side is not full duplex.");
	return (FAILED);
    } 

    /* Check MAC Side Link up and speed (page 4, reg 17)*/
    rc = phy_read_reg(dev, port, 4, 17, &reg_d);
    if (rc != PASSED) {
	return (FAILED);
    }

    if ((NVRAM)->diagflag & D_VERBOSE) {
	if ((reg_d & PHY_COPPER_LINK) != PHY_COPPER_LINK) {
	    printf("\nQSGMII link is not up. register @ (page 4, reg 17) = %#x\n", reg_d);
	} else {
	    printf("\nQSGMII link is up. register @ (page 4, reg 17) = %#x\n", reg_d);	
	}
    }

    if ((reg_d & PHY_LINK_SPEED_MASK) != PHY_LINK_SPEED_1000) {
	cterr('f',0,"MAC side speed is not 1000Mbps.");
	return (FAILED);
    }
    if ((reg_d & PHY_FULL_DUPLEX) != PHY_FULL_DUPLEX) {
	cterr('f',0,"MAC side is not full duplex.");
	return (FAILED);
    } 

    /* set loopback */
    rc = phy_read_reg(dev, port, 0, 0, &reg_d);
    if (rc != PASSED) {
	return (FAILED);
    }
    reg_d |= PHY_LPBK_ENA;
    rc = phy_write_reg(dev, port, 0, 0, reg_d);
    if (rc != PASSED) {
	return (FAILED);
    }

    msleep(2000);  /* Wait for link up */

    /* set page 0xfa */
    reg_d = 0x0200;
    rc = phy_write_reg(dev, port, 0xfa, 7, reg_d);
    if (rc != PASSED) {
	return (FAILED);
    }
    reg_d = 0x0400;
    rc = phy_write_reg(dev, port, 0xfa, 1, reg_d);
    if (rc != PASSED) {
	return (FAILED);
    }

    msleep(200);

    /* Check MAC Side Link up and Sync (page 4, reg 17)*/
    rc = phy_read_reg(dev, port, 4, 17, &reg_d);
    if (rc != PASSED) {
	return (FAILED);
    }
    if (!(reg_d & PHY_SYNC)) {
	return (FAILED);
    }
    if ((reg_d & PHY_LINK_SPEED_MASK) != PHY_LINK_SPEED_1000) {
	cterr('f',0,"MAC side speed is not 1000Mbps.");
	return (FAILED);
    }
    if ((reg_d & PHY_FULL_DUPLEX) != PHY_FULL_DUPLEX) {
	cterr('f',0,"MAC side is not full duplex.");
	return (FAILED);
    } 

    return PASSED;
}



/******************************************************************************
 *
 * Function   :	start_ext_lpbk
 * Description:	start PHY external loopback
 *              Since External Loopback setup(mdDiagSetExternalLoopback API) 
 *              overwrites Copper Auto-Neg mode, mdDiagSetExternalLoopback API
 *              should be called after loopback test is stopped.
 * Inputs     :	dev - point to MAD_DEV
 *              port - port number
 *              speed - supported Speed modes are:
 *                      MAD_SPEED_10M,
 *                      MAD_SPEED_100M, or
 *                      MAD_SPEED_1000M
 * Outputs    : PASSED/FAILED
 *
 ******************************************************************************
 */
unsigned int
start_ext_lpbk (MAD_DEV *dev, int port, MAD_SPEED_MODE speed)
{
    MAD_U32 reg_d;
    int rc;

    if (dev == 0) {
        cterr('f',0,"MAD driver is not initialized");
        return FAILED;
    }

    /* enable stub test */
    rc = phy_read_reg(dev, port, 6, 18, &reg_d);
    if (rc != PASSED) {
	return (FAILED);
    }
    reg_d |= PHY_ENA_STUB_TEST;
    rc = phy_write_reg(dev, port, 6, 18, reg_d);
    if (rc != PASSED) {
	return (FAILED);
    }

    /* set speed (page 0, reg 9)*/
    rc = phy_read_reg(dev, port, 0, 9, &reg_d);
    if (rc != PASSED) {
	return (FAILED);
    }
    reg_d |= PHY_1000BT_ADV;
    rc = phy_write_reg(dev, port, 0, 9, reg_d);
    if (rc != PASSED) {
	return (FAILED);
    }

    /* PHY soft reset */
    reg_d = PHY_COPPER_FULL_DUPLEX | PHY_SPD_SEL_1000M | PHY_AUTO_NEO_ENA | PHY_COOPER_RST;
    rc = phy_write_reg(dev, port, 0, 0, reg_d);
    if (rc != PASSED) {
	return (FAILED);
    }
    msleep(100);

    /* disable loopback */
    rc = phy_read_reg(dev, port, 0, 0, &reg_d);
    if (rc != PASSED) {
	return (FAILED);
    }

    reg_d &= ~PHY_LPBK_ENA;
    rc = phy_write_reg(dev, port, 0, 0, reg_d);
    if (rc != PASSED) {
	return (FAILED);
    }
    msleep(2000);  /* Wait for link up */

    /* Check copper link speed (page 0, reg 17)*/
    rc = phy_read_reg(dev, port, 0, 17, &reg_d);
    if (rc != PASSED) {
	return (FAILED);
    }

    if ((NVRAM)->diagflag & D_VERBOSE) {
	if ((reg_d & PHY_COPPER_LINK) != PHY_COPPER_LINK) {
	    printf("\nCopper link is not up. register @ (page 0, reg 17) = %#x\n", reg_d);
	} else {
	    printf("\nCopper link is up. register @ (page 0, reg 17) = %#x\n", reg_d);	
	}
    }

    if ((reg_d & PHY_LINK_SPEED_MASK) != PHY_LINK_SPEED_1000) {
	cterr('f',0,"Copper side speed is not 1000Mbps.");
	return (FAILED);
    }   
    /* Check MAC Side Link up and Sync (page 4, reg 17)*/
    rc = phy_read_reg(dev, port, 4, 17, &reg_d);
    if (rc != PASSED) {
	return (FAILED);
    }
    if (!(reg_d & PHY_SYNC)) {
	return (FAILED);
    }
    if ((reg_d & PHY_LINK_SPEED_MASK) != PHY_LINK_SPEED_1000) {
	cterr('f',0,"MAC side speed is not 1000Mbps.");
	return (FAILED);
    }

    return PASSED;
}



/******************************************************************************
 *
 * Function   :	phy_port_internal_lpbk_test
 * Description:	perform PHY internal loopback test.
 *              host->GE->XCAT2->phy->XCAT2->->GE->host
 * Inputs     :	port_num
 *              bp_port - backplane GE port, can GE0_XCAT2_PORT or GE1_XCAT2_PORT
 * Outputs    : PASSED/FAILED
 *
 ******************************************************************************
 */
static int
phy_port_internal_lpbk_test (int port_num, int bp_port)
{
    int ctrl_plane_sgmii_port = 0;
    int num_pkt = DREAMLINER_GE_BP_PACKET_NO;
    int rc = PASSED;

    if (xcat2_config_port_pve(xcat2_dev_num[get_slot_num()-1], bp_port, port_num)) {
	cterr('f',0,"Failed to configure PVE for port %d", port_num + 1);
	return (FAILED);
    }

    if (start_mac_lpbk(&phy_dev[get_slot_num()-1], port_num, MAD_SPEED_1000M)) {
	cterr('f',0,"Failed to enable PHY loopback for port %d", port_num + 1);
	return (FAILED);
    }

    /* Do SGMII loopback test. */
    ctrl_plane_sgmii_port = get_sgmii_port_num(get_slot_num(), TYPE_SWITCH);
    if (sgmii_lpbk_util(ctrl_plane_sgmii_port, num_pkt)) {
	dump_phy_reg(port_num);
	rc = FAILED;
    }

    if (xcat2_unconfig_port_pve(xcat2_dev_num[get_slot_num()-1], bp_port, port_num)) {
	cterr('f',0,"Failed to unconfigure PVE for port %d", port_num + 1);
	rc = FAILED;
    }

    if (phy_config(port_num) == FAILED) {
	cterr('f',0,"Failed to init PHY for port %d", port_num + 1);
	return (FAILED);
    }

    return (rc);
}


/******************************************************************************
 *
 * Function   :	phy_internal_lpbk_test_ge1
 * Description:	perform PHY internal loopback test.
 *              host->GE1->XCAT2->phy->XCAT2->->GE1->host
 * Inputs     :	None
 * Outputs    : PASSED/FAILED
 *
 ******************************************************************************
 */
int
phy_internal_lpbk_test_ge1 (void)
{
    int port_num = get_port_num();
    int i;
    boolean value = PASSED;
    if (is_goldbeach() || is_curie_1ru() || is_curie_2ru()) {
        printf("\nGoldbeach and Curie 1RU/2RU didn't Support GE1\n");
        return (PASSED);
    }
#ifdef TABEIL
    printf("\nTabei-L didn't Support GE1\n");
    return (PASSED);
#endif
    cterr_setup();
    cterr_add_component("Marvell PHY", 
			"SMI0 interface from xCat2 switch",
			"Backplane GE1 interface from the router");
    cterr_add_debug("Check Marvell PHY",
		    "Check SMI0 interface from xCat2 switch",
		    "Check Backplane GE1 interface from the router");

    /* reset PHY before the test */
    if (reset_phy() != PASSED) {
        cterr('f',0,"Failed reset_phy()");
        return (FAILED);
    }

    clear_sw_counter();

    if (port_force_link_set(LINK_DOWN, GE0_XCAT2_PORT, TRUE) != OK) {
        cterr('f',0,"Failed port_force_link_set()");
        return (FAILED);
    }

    for (i = 0; i < port_num; i++) {
        prpass(testpass, "PHY port %d internal loopback test through GE1, ", i + 1);

	/* clear phy counters */
	clear_phy_counter();

	if (phy_port_internal_lpbk_test(i, GE1_XCAT2_PORT)) {
	    print_sw_counter();
	    print_phy_counter();
        /*CSCus79764 : Should setup GE0 link down until all ports loopback test completed. */
//	    port_force_link_set(LINK_DOWN, GE0_XCAT2_PORT, FALSE);
            cterr('f', 0, "Failed PHY internal loopback test for port %d", i + 1);
            value = FAILED;
	}
	if ((NVRAM)->diagflag & D_VERBOSE) {
	    print_sw_counter();
	    print_phy_counter();
	}
    }
    if (port_force_link_set(LINK_DOWN, GE0_XCAT2_PORT, FALSE) != OK) {
        cterr('f',0,"Failed port_force_link_set()");
        return (FAILED);
    }

    return (value);
}

/******************************************************************************
 *
 * Function   :	phy_internal_lpbk_test_ge0
 * Description:	perform PHY internal loopback test.
 *              host->GE0->XCAT2->phy->XCAT2->->GE0->host
 * Inputs     :	None
 * Outputs    : PASSED/FAILED
 *
 ******************************************************************************
 */
int
phy_internal_lpbk_test_ge0 (void)
{
    int port_num = get_port_num();
    int i;
    boolean value = PASSED;

    cterr_setup();
    cterr_add_component("Marvell PHY", 
			"SMI0 interface from xCat2 switch",
			"Backplane GE0 interface from the router");
    cterr_add_debug("Check Marvell PHY",
		    "Check SMI0 interface from xCat2 switch",
		    "Check Backplane GE0 interface from the router");

    /* reset PHY before the test */
    if (reset_phy() != PASSED) {
        cterr('f',0,"Failed reset_phy()");
        return (FAILED);
    }

    clear_sw_counter();

    if (port_force_link_set(LINK_DOWN, GE1_XCAT2_PORT, TRUE) != OK) {
        cterr('f',0,"Failed port_force_link_set()");
        return (FAILED);
    }

    for (i = 0; i < port_num; i++) {
        prpass(testpass, "PHY port %d internal loopback test through GE0, ", i + 1);

	/* clear phy counters */
	clear_phy_counter();

	if (phy_port_internal_lpbk_test(i, GE0_XCAT2_PORT)) {
	    print_sw_counter();
	    print_phy_counter();
        /*CSCus79764 : Should setup GE1 link down until all ports loopback test completed. */
//	    port_force_link_set(LINK_DOWN, GE1_XCAT2_PORT, FALSE);
            cterr('f', 0, "Failed PHY internal loopback test for port %d", i + 1);
            value = FAILED;
	}
	if ((NVRAM)->diagflag & D_VERBOSE) {
	    print_sw_counter();
	    print_phy_counter();
	}
    }
    if (port_force_link_set(LINK_DOWN, GE1_XCAT2_PORT, FALSE) != OK) {
        cterr('f',0,"Failed port_force_link_set()");
        return (FAILED);
    }

    return (value);
}

/******************************************************************************
 *
 * Function   :	port_external_lpbk_test
 * Description:	perform external loopback test.
 *              host->GE->XCAT2->phy->loopback connecter->phy->XCAT2->->GE->host
 * Inputs     :	port_num
 *              bp_port - backplane GE port, can GE0_XCAT2_PORT or GE1_XCAT2_PORT
 * Outputs    : PASSED/FAILED
 *
 ******************************************************************************
 */
static int
port_external_lpbk_test (int port_num, int bp_port)
{
    int ctrl_plane_sgmii_port = 0;
    int num_pkt = DREAMLINER_GE_BP_PACKET_NO;
    int rc = PASSED;

    if (xcat2_config_port_pve(xcat2_dev_num[get_slot_num()-1], bp_port, port_num)) {
	cterr('f',0,"Failed to configure PVE for port %d", port_num);
	return (FAILED);
    }

    if (start_ext_lpbk(&phy_dev[get_slot_num()-1], port_num, MAD_SPEED_1000M)) {
	cterr('f',0,"Failed to enable external loopback for port %d", port_num);
	return (FAILED);
    }

    /* Do SGMII loopback test. */
    ctrl_plane_sgmii_port = get_sgmii_port_num(get_slot_num(), TYPE_SWITCH);
    if (sgmii_lpbk_util(ctrl_plane_sgmii_port, num_pkt)) {
	dump_phy_reg(port_num);
	rc = FAILED;
    }

    if (xcat2_unconfig_port_pve(xcat2_dev_num[get_slot_num()-1], bp_port, port_num)) {
	cterr('f',0,"Failed to unconfigure PVE for port %d", port_num);
	rc = FAILED;
    }

    if (phy_config(port_num) == FAILED) {
	cterr('f',0,"Failed to init PHY for port %d", port_num);
	return (FAILED);
    }

    return (rc);
}


/******************************************************************************
 *
 * Function   :	external_lpbk_test
 * Description:	perform external loopback test.
 *              host->GE0->XCAT2->phy->loopback connecter->phy->XCAT2->->GE0->host
 * Inputs     :	None
 * Outputs    : PASSED/FAILED
 *
 ******************************************************************************
 */
int
external_lpbk_test (void)
{
    int port_num = get_port_num();
    int i, m;
    boolean value = PASSED;

    cterr_setup();
    /*
     * if D_EXT_LOOPBACK is OFF, then just return
     */
    if ((NVRAM)->diagflag & D_EXT_LOOPBACK) {
        return (PASSED);
    }

    cterr_add_component("Marvell PHY", 
			"External GE loopback connector",
			"Backplane GE0 interface from the router");
    cterr_add_debug("Check Marvell PHY",
		    "Check External GE loopback connector",
		    "Check Backplane GE0 interface from the router");

    /* reset PHY before the test */
    if (reset_phy() != PASSED) {
        cterr('f',0,"Failed reset_phy()");
        return (FAILED);
    }

    clear_sw_counter();

    if (port_force_link_set(LINK_DOWN, GE1_XCAT2_PORT, TRUE) != OK) {
        cterr('f',0,"Failed port_force_link_set()");
        return (FAILED);
    }

    /* we need to do port mapping here.
       0 - 2, 1 - 1, 2 - 4, 3 - 3, 4 - 6, 5 -5, 6 - 8, 7 - 7 
    */
    for (i = 1; i <= port_num; i++) {
        prpass(testpass, "Port %d external loopback test, ", i);

        if (i % 2) 
            m = i;
        else
            m = i - 2;

        /* clear PHY counter */
        clear_phy_counter();

        if (port_external_lpbk_test(m, GE0_XCAT2_PORT)) {
            print_sw_counter();
            print_phy_counter();
            /*CSCus79764 : Should setup GE1 link down until all ports loopback test completed. */
//            port_force_link_set(LINK_DOWN, GE1_XCAT2_PORT, FALSE);
            cterr('f', 0, "Failed external loopback test for port %d", i);
            value = FAILED;
        }
        if ((NVRAM)->diagflag & D_VERBOSE) {
            print_sw_counter();
            print_phy_counter();
        }
    }

    if (port_force_link_set(LINK_DOWN, GE1_XCAT2_PORT, FALSE) != OK) {
        cterr('f',0,"Failed port_force_link_set()");
        return (FAILED);
    }
    return (value);
}


/******************************************************************************
 *
 * Function   :	phy_read_reg
 * Description:	read a PHY register in paged mode.
 * Inputs     :	dev - point to MAD_DEV
 *              port_num - PHY port number to read the register for.
 *              page_num - Page number to be accessed.
 *              reg_addr - The register's address.
 *              data - point to unsigned int which holds regiter value
 * Outputs    : PASSED/FAILED
 *
 ******************************************************************************
 */
static unsigned int
phy_read_reg (MAD_DEV *dev, MAD_LPORT port_num, MAD_U16 page_num, MAD_U16 reg_addr, MAD_U32 *data)
{
    MAD_STATUS status;

    status=mdSysGetPagedPhyReg(dev, port_num, page_num, reg_addr, data);

    if (status==MAD_OK) {
      *data &= 0xffff; 
      return PASSED;
    } else {
        testMADDisplayStatus(status);
	return FAILED;
    }
}


/******************************************************************************
 *
 * Function   :	phy_write_reg
 * Description:	write to a PHY register in paged mode.
 * Inputs     :	dev - point to MAD_DEV
 *              port_num - PHY port number to read the register for.
 *              page_num - Page number to be accessed.
 *              reg_addr - The register's address.
 *              data - write data
 * Outputs    : PASSED/FAILED
 *
 ******************************************************************************
 */
static unsigned int 
phy_write_reg (MAD_DEV *dev,MAD_LPORT port_num, MAD_U16 page_num, MAD_U16 reg_addr, MAD_U16 data)
{
    MAD_STATUS status;

    status=mdSysSetPagedPhyReg(dev, port_num, page_num, reg_addr, data);

    if (status==MAD_OK) {
	return PASSED;
    } else {
	testMADDisplayStatus(status);
	return FAILED;
    }
}

/******************************************************************************
 *
 * Function   :	peek_phy_reg
 * Description:	utility to peek a PHY register.
 * Inputs     :	none
 * Outputs    : PASSED/FAILED
 *
 ******************************************************************************
 */
static int
peek_phy_reg ()
{
    MAD_DEV *dev = &phy_dev[get_slot_num()-1];
    MAD_LPORT port_num;
    MAD_U16 page_num;
    MAD_U16 reg_num;
    MAD_U32 data;
    int ret;

    assert(dev);
   
    port_num = getdec_answer("Enter PHY port number: ", 0, 0, get_port_num()-1);
    page_num = getdec_answer("Enter PHY page number: ", 0, 0, 18);
    reg_num = getdec_answer("Enter PHY register number: ", 0, 0, 31);
    ret = phy_read_reg(dev, port_num, page_num, reg_num, &data);

    if (ret == PASSED)
	printf("PHY register value @ offset %d = %#x\n", reg_num, data&0xffff);
    return (ret);
}

/******************************************************************************
 *
 * Function   :	poke_phy_reg
 * Description:	utility to poke a PHY register.
 * Inputs     :	none
 * Outputs    : PASSED/FAILED
 *
 ******************************************************************************
 */
static int
poke_phy_reg ()
{
    MAD_DEV *dev = &phy_dev[get_slot_num()-1];
    MAD_LPORT port_num;
    MAD_U16 page_num;
    MAD_U16 reg_num;
    MAD_U16 data;

    assert(dev);
   
    port_num = getdec_answer("Enter PHY port number: ", 0, 0, get_port_num()-1);
    page_num = getdec_answer("Enter PHY page number: ", 0, 0, 18);
    reg_num = getdec_answer("Enter PHY register number: ", 0, 0, 31);
    data = gethex_answer("Enter write data: ", 0, 0, 0xffff);
    return (phy_write_reg(dev, port_num, page_num, reg_num, data));
}



/******************************************************************************
 *
 * Function   :	phy_display_reg
 * Description:	diaplay a page of registers for a specific port.
 * Inputs     :	dev - point to MAD_DEV
 *              port_num - PHY port number to read the register for.
 *              page_num - Page number to be accessed.
 * Outputs    : PASSED/FAILED
 *
 ******************************************************************************
 */
static unsigned int  
phy_display_reg (MAD_DEV *dev, MAD_U8 port_num, MAD_U16 page_num)
{
    MAD_STATUS status;
    int i;
    MAD_U16 data;

    if (dev == 0) {
        printf("MAD driver is not initialized.\n");
        return FAILED;
    }

    printf("Read PHY port %d page %d : \n", (int)port_num, (int)page_num);

    for (i=0; i<32; i++) {
	if((status = madHwReadPagedPhyReg(dev,port_num,page_num, i, &data)) != MAD_OK) {
	    testMADDisplayStatus(status);
	    cterr('f',0,"Reading page %d  port %d register %d failed.", 
		  (int)page_num, (int)port_num, i);
	    return FAILED;
	}

	if ((i+1)%4)
	    printf("reg %02d: 0x%04x    ", i, (int)data);
	else
	    printf("reg %02d: 0x%04x\n", i, (int)data);
    }

    printf("\n");
    
    return PASSED;
}


/******************************************************************************
 *
 * Function   :	display_phy_reg
 * Description:	utility to display registers of all the pages 
 *              for a specific PHY port.
 * Inputs     :	none
 * Outputs    : PASSED/FAILED
 *
 ******************************************************************************
 */
static int
display_phy_reg ()
{
    MAD_DEV *dev = &phy_dev[get_slot_num()-1];
    MAD_LPORT port_num;
    int i;

    assert(dev);

    port_num = getdec_answer("Enter PHY port number: ", 0, 0, get_port_num()-1);

    if (phy_display_reg(dev, port_num, 0) == FAILED) {
        return FAILED;
    }

    for (i = 2; i <= 9; i++) {
	if (phy_display_reg(dev, port_num, i) == FAILED)
	    return FAILED;
    }

    if (phy_display_reg(dev, port_num, 12) == FAILED)
	return FAILED;

    if (phy_display_reg(dev, port_num, 14) == FAILED)
	return FAILED;
    
    return (phy_display_reg(dev, port_num, 18));
}

/******************************************************************************
 *
 * Function   :	phy_soft_reset
 * Description:	soft reset a specific PHY port.
 * Inputs     :	dev - point to MAD_DEV
 *              port_num - PHY port number 
 * Outputs    : PASSED/FAILED
 *
 ******************************************************************************
 */
static unsigned int
phy_soft_reset (MAD_DEV *dev, MAD_LPORT port_num) 
{
    MAD_STATUS status;

    status = mdSysSoftReset(dev, port_num);

    if (status==MAD_OK) {
	printf("Soft reset for port %d.\n", (int)port_num);
	return PASSED;
    } else {
        testMADDisplayStatus(status);
	return FAILED;
    }
}


/******************************************************************************
 *
 * Function   :	reset_phy_port
 * Description:	utility to soft reset a specific PHY port.
 * Inputs     :	none
 * Outputs    : PASSED/FAILED
 *
 ******************************************************************************
 */
static int
reset_phy_port ()
{
    MAD_DEV *dev = &phy_dev[get_slot_num()-1];
    MAD_LPORT port_num;

    assert(dev);
    port_num = getdec_answer("Enter PHY port number: ", 0, 0, get_port_num()-1);

    return (phy_soft_reset(dev, port_num));
}


/******************************************************************************
 *
 * Function   :	phy_intr_test
 * Description:	Verify PHY is able to generate interrupt to the host via PonCat2 swtich.
 * Inputs     :	none
 * Outputs    : PASSED/FAILED
 *
 ******************************************************************************
 */
int
phy_intr_test ()
{
    MAD_DEV *dev = &phy_dev[get_slot_num()-1];
    uint16_t data; 
    uint32_t rd_data;
    int test_port = 0;
    int status, i;
    unsigned long config_base = dl_get_pci_base_addr();

    cterr_setup();
    prpass(testpass, "PHY interrupt test, ");

    phy_intr_happened = FALSE;

    cterr_add_component("Marvell PHY", 
			"Marvell xCat2 switch");
    cterr_add_debug("Check Marvell xCat2 switch","Check Marvell PHY",
		    "Check Marvell xCat2 switch");

    /* before the test, make sure the GPP2 within PonCat2 is unasserted */
    status = xcat2_reg_pci_read(GPP_INPUT_REG_OFFSET, &rd_data);
    if ((NVRAM)->diagflag & D_VERBOSE) {
	printf("GPP_INPUT_REG_OFFSET @ %#x = %#x\n", GPP_INPUT_REG_OFFSET, rd_data);
    fflush(0);
    }
    if (status == PASSED) {
	if ((rd_data & GPP_PHY_INT) != GPP_PHY_INT) {
	    cterr('f',0,"Before the PHY interrupt test, GPP2 pin is asserted.");
	    return (FAILED);
	}
    } else {
	cterr('f',0,"Failed to read PHY intr status from PonCat2.");
	return (FAILED);
    }

    /* read PHY intr status register to clear all the pending bits */
    phy_read_reg(dev, test_port, 4, PHY_QSGMII_INTR_STATUS_REG, &rd_data);

    /* enable PHY QSGMII Link Status Changed interrupt (page 4, register 19, bit 10) */
    data = PHY_QSGMII_LINK_STATUS_CHANGED;
    status =  phy_write_reg(dev, test_port, 4, PHY_QSGMII_INTR_ENA_REG, data);
    if (status != PASSED) {
	cterr('f',0, "phy smi write failed. port = %#x"
	      "page = %#x, reg = %#x, status = %#x\n",
	      test_port, 4, PHY_QSGMII_INTR_ENA_REG, status);
        return (FAILED);
    }

    /* clear xcat2 GPIO interrupt status */
    *(unsigned int *)(config_base + GPIO_INTR_CAUSE_REG) = 0;

    /* enable xcat2 GPIO interrupts for PHY */
    *(unsigned int *)(config_base + GPIO_INTR_LEVEL_MASK_REG) = (1 << GPIO_PHY_INT);

    /* start and stop PHY MAC loopback will trigger QSGMII Link Status Changed intr */
    if (start_mac_lpbk(dev, test_port, MAD_SPEED_1000M)) {
	cterr('f',0,"Failed to enable PHY loopback for port %d", test_port);
	return (FAILED);
    }
    msleep(1000);

    if (phy_config(test_port) == FAILED){
        return (FAILED);
    }

    msleep(1000);

    /* read PHY global intr status register (page 0, reg 23) */
    phy_read_reg(dev, test_port, 0, PHY_GLOBAL_INTR_STATUS_REG, &rd_data);
    if ((rd_data & (1 << test_port)) != (1 << test_port)) {
	cterr('f',0,"PHY interrupt is not active on port %d, PHY_GLOBAL_INTR_STATUS = %#x", 
	      test_port, rd_data);
	status = FAILED;
    }

    if ((NVRAM)->diagflag & D_VERBOSE) {
	rd_data = *(unsigned int *)(config_base + 0x20210);
	printf("main_int_cause_hi_reg = %#x\n", rd_data);
	rd_data = *(unsigned int *)(config_base + 0x10110);
	printf("gpio_data_in_reg = %#x\n", rd_data);
	rd_data = *(unsigned int *)(config_base + 0x10114);
	printf("gpio_int_cause_reg = %#x\n", rd_data);
    fflush(0);
    }

    for (i = 0; i < PHY_INTR_DELAY; i++) {
	if (phy_intr_happened == TRUE) {
	    phy_intr_happened = FALSE;
	    break;
	}
	msleep(20);
    }

    if (i == PHY_INTR_DELAY) {
	cterr('f', 0, "Timeout waiting for PHY interrupt.");
	status = FAILED;
    }

    /* read status bit to clear it */
    phy_read_reg(dev, test_port, 4, PHY_QSGMII_INTR_STATUS_REG, &rd_data);
    if ((NVRAM)->diagflag & D_VERBOSE) {
	printf("QSGMII interrupt status register = %#x\n", rd_data&0xffff);
    fflush(0);
    }

    /* disable PHY GPIO interrupts */
    *(unsigned int *)(config_base + GPIO_INTR_LEVEL_MASK_REG) = 0;

    /* disable QSGMII interrupts */
    data = 0;
    if (phy_write_reg(dev, test_port, 4, PHY_QSGMII_INTR_ENA_REG, data) != PASSED) {
	cterr('f',0, "phy smi write failed. port = %#x"
	      "page = %#x, reg = %#x, status = %#x\n",
	      test_port, 4, PHY_QSGMII_INTR_ENA_REG, status);
	status = FAILED;
    }

    return (status);
}



/******************************************************************************
 *
 * Function   :	phy_disable_int
 * Description:	disable all the interrupt
 * Inputs     :	dev - point to MAD_DEV
 * Outputs    : PASSED/FAILED
 *
 ******************************************************************************
 */
static int
phy_disable_int (MAD_DEV *dev)
{
    MAD_STATUS status;
    MAD_LPORT port;
    MAD_INT_TYPE int_type;

    /* clear out all int causes */
    memset(&int_type, 0, sizeof(MAD_INT_TYPE));

    for(port=0; port<dev->numOfPorts; port++) {
        if((status = mdIntSetEnable(dev,port,&int_type)) != MAD_OK) {
            testMADDisplayStatus(status);
            printf("mdIntSetEnable returned fail.\n");
            return FAILED;
        }
    }

    return PASSED;
}


/*****************************************************************************
 *
 * Function:    phy_register_tests
 *
 * Description: For each register from reg_ptr, this function checks for 
 *              accessibility and does a ripple 1 and a ripple 0 test if 
 *              applicable (not all registers are W/R register).
 *
 * Inputs :     dev - point to MAD_DEV
 *              port_num - PHY port number to read the register for.
 *              page_num - Page number to be accessed.
 *              reg_ptr  - point to reg_info_t table.
 *
 * Output: PASSED/FAILED
 *
 ******************************************************************************
 */
static int 
phy_register_tests (MAD_DEV *dev, MAD_LPORT port_num, MAD_U16 page_num, const reg_info_t *reg_ptr )
{
    uint32_t ix;
    uint retval, ret_val, save_val, readval;
    uint data, temp, tst_offset;

    readval = 0;
    retval = PASSED;
    ret_val = PASSED;

    while (reg_ptr->size.size != 0) {
        retval = phy_read_reg(dev, port_num, page_num, reg_ptr->offset, &save_val);
        if (retval == FAILED) {
            cterr('f',0,"%s(): Error reading PHY port %d page %d register %d",
		  __FUNCTION__, port_num, page_num, reg_ptr->offset);
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
                retval = phy_write_reg(dev, port_num, page_num, tst_offset, temp);

                /* Read back */
                if (retval == PASSED) {
                    ret_val = phy_read_reg(dev, port_num, page_num, reg_ptr->offset, &readval);
                }
                if (((readval & reg_ptr->mask) != temp) ||
                    (retval == FAILED) || (ret_val == FAILED)) {
                    cterr('f',0,"%s(): Ripple one test failed when accessing "
			  "PHY port %d page %d register %d. "
			  " Expect %#x, Read %#x", __FUNCTION__,
			  port_num, page_num, tst_offset, temp, readval);
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
                retval = phy_write_reg(dev, port_num, page_num, tst_offset, temp);
                if (retval == PASSED) {
                    /* Read back */
                    ret_val = phy_read_reg(dev, port_num, page_num, reg_ptr->offset, &readval);
                }
                if (((readval & reg_ptr->mask) != temp) ||
                    (retval == FAILED) || (ret_val == FAILED)) {
                    cterr('f',0,"%s(): Ripple zero test failed when accessing "
			  "PHY port %d page %d register %d. "
			  " Expect %#x, Read %#x", __FUNCTION__,
			  port_num, page_num, tst_offset, temp, readval);
                    return (FAILED);
                }
            }

            /*
             * pattern test
             */
            data = PATTERN;
            for (ix = 0; ix < 2; ix++) {
                temp = data & reg_ptr->mask;
                /* Write to register under test */
                retval = phy_write_reg(dev, port_num, page_num, tst_offset, temp);
                if (retval == PASSED) {
                    /* Read back */
                    ret_val = phy_read_reg(dev, port_num, page_num, reg_ptr->offset, &readval);
                }
                if (((readval & reg_ptr->mask) != temp) ||
                    (retval == FAILED) || (ret_val == FAILED)) {
                    cterr('f',0,"%s(): Pattern test failed when accessing "
			  "PHY port %d page %d register %d. "
			  " Expect %#x, Read %#x", __FUNCTION__,
			  port_num, page_num, tst_offset, temp, readval);
                    return (FAILED);
                }
    
                data = ~PATTERN; /* complement data pattern */
            }

            /*
             * restore original value
             */	    
	    retval = phy_write_reg(dev, port_num, page_num, tst_offset, save_val);

            if (retval == FAILED) {
		cterr('f',0,"%s(): Error restoring PHY port %d page %d register %d. ",
		      __FUNCTION__, port_num, page_num, tst_offset);
		return (FAILED);
            }
        }
        reg_ptr++;
    }

    return (PASSED);
}


/*******************************************************************************
 *
 * Function: phy_reg_test_single.
 *
 * Description: This function implements the PHY registers test for 
 *              a specific port.
 *
 * Input:       dev - point to MAD_DEV
 *              port_num - PHY port number to read the register for.
 *
 * Output: PASSED/FAILED.
 *
 *******************************************************************************
 */
static int 
phy_reg_test_single (MAD_DEV *dev, MAD_LPORT port_num)
{
    int ret = PASSED;

    if (phy_register_tests(dev, port_num, 0, &marvell_88e1680_reg_page0[0]) == FAILED) {
        cterr('f',0, "PHY Reg Test Failed");
        return (FAILED);
    }

    if (phy_register_tests(dev, port_num, 2, &marvell_88e1680_reg_page2[0]) == FAILED) {
        cterr('f',0, "PHY Reg Test Failed");
        return (FAILED);
    }

    if (phy_register_tests(dev, port_num, 3, &marvell_88e1680_reg_page3[0]) == FAILED) {
        cterr('f',0, "PHY Reg Test Failed");
        return (FAILED);
    }

    if (phy_register_tests(dev, port_num, 4, &marvell_88e1680_reg_page4[0]) == FAILED) {
        cterr('f',0, "PHY Reg Test Failed");
        return (FAILED);
    }

    if (phy_register_tests(dev, port_num, 5, &marvell_88e1680_reg_page5[0]) == FAILED) {
        cterr('f',0, "PHY Reg Test Failed");
        return (FAILED);
    }       

    if (phy_register_tests(dev, port_num, 6, &marvell_88e1680_reg_page6[0]) == FAILED) {
        cterr('f',0, "PHY Reg Test Failed");
        return (FAILED);
    }

    if (phy_register_tests(dev, port_num, 7, &marvell_88e1680_reg_page7[0]) == FAILED) {
        cterr('f',0, "PHY Reg Test Failed");
        return (FAILED);
    }

    if (phy_register_tests(dev, port_num, 8, &marvell_88e1680_reg_page8[0]) == FAILED) {
        cterr('f',0, "PHY Reg Test Failed");
        return (FAILED);
    }   

    if (phy_register_tests(dev, port_num, 9, &marvell_88e1680_reg_page9[0]) == FAILED) {
        cterr('f',0, "PHY Reg Test Failed");
        return (FAILED);
    }   

    if (phy_register_tests(dev, port_num, 12, &marvell_88e1680_reg_page12[0]) == FAILED) {
        cterr('f',0, "PHY Reg Test Failed");
        return (FAILED);
    }       

    if (phy_register_tests(dev, port_num, 14, &marvell_88e1680_reg_page14[0]) == FAILED) {
        cterr('f',0, "PHY Reg Test Failed");
        return (FAILED);
    }   

    return (ret);
}


/*******************************************************************************
 *
 * Function: phy_reg_test().
 *
 * Description: This function implements the PHY registers test.
 *
 * Input:   None
 * 
 * Output: PASSED/FAILED.
 *
 *******************************************************************************
 */
int 
phy_reg_test ()
{
    MAD_DEV *dev = &phy_dev[get_slot_num()-1];
    int port;

    cterr_setup();
    prpass(testpass, "PHY register test, ");
    assert(dev);

    cterr_add_component("Marvell PHY", 
			"SMI0 interface from xCat2 switch");
    cterr_add_debug("Check Marvell PHY",
		    "Check SMI0 interface from xCat2 switch");

    for (port = 0; port < get_port_num(); port++) {
        if (phy_reg_test_single(dev, port) == FAILED) {
            return (FAILED);
        }
    }
    
    return (PASSED);
}


/**********************************************************************
 *
 * Function: phy_set_test_mode
 *
 * Description: This function provides PHY test mode for Marvell GE PHY.
 *
 * Input: None
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
static int 
phy_set_test_mode ()
{
    MAD_DEV *dev = &phy_dev[get_slot_num()-1];
    uint port, test_mode;
    MAD_STATUS status;

    assert(dev);
    cterr_add_component("Marvell PHY", 
			"SMI0 interface from xCat2 switch");
    cterr_add_debug("Check Marvell PHY",
		    "Check SMI0 interface from xCat2 switch");

    port = gethex_answer("Enter the port number: ", 0, 0, get_port_num()-1);

    printf("    0 - Normal Mode\n");
    printf("    1 - Test Mode 1 - Transmit Waveform Test\n");
    printf("    2 - Test Mode 2 - Transmit Jitter Test (Master mode)\n");
    printf("    3 - Test Mode 3 - Transmit Jitter Test (Slave mode)\n");
    printf("    4 - Test Mode 4 - Transmit Distortion Test\n");
    test_mode = gethex_answer("Enter the test mode: ", 0, 0, 4);

    if (test_mode > 0) {
	if((status = mdDiagSetIEEETest(dev, port, ENABLE, test_mode)) != MAD_OK) {
	    testMADDisplayStatus(status);
	    cterr('f',0,"mdDiagSetIEEETest returned fail.");
	    return FAILED;
	}
    } else {
	/* go back to normal mode */
	if((status = mdDiagSetIEEETest(dev, port, DISABLE, 1)) != MAD_OK) {
	    testMADDisplayStatus(status);
	    cterr('f',0,"mdDiagSetIEEETest returned fail.");
	    return FAILED;
	}
    }

    return PASSED;
}


/*******************************************************************************
 *
 * Function: phy_detect_phone
 *
 * Description: Check if Marvell PHY detects a Power Device (PD) at a
 *              given port.
 *
 * Input:   port - PHY port number
 *
 * Outputs:  PASSED - Found a PD.
 *           FAILED - PD not detected or phy access error.
 *
 * Assumptions:
 *
 *******************************************************************************
 */
int 
phy_detect_phone (int port)
{
    MAD_DEV *dev = &phy_dev[get_slot_num()-1];
    uint ix, rc, detected;
    MAD_U32 reg_d;

    assert(dev);
    cterr_add_component("Marvell PHY", 
			"SMI0 interface from xCat2 switch");
    cterr_add_debug("Check Marvell PHY",
		    "Check SMI0 interface from xCat2 switch");

    /* power up copper interface on PHY device. */
    if(mdSysSetPhyEnable(dev,port,MAD_TRUE) != MAD_OK) {
	cterr('f',0,"Failed to Enable Phy (port %i)\n",port);
	return FAILED;
    }

    /* Disable Auto Negotiation */
    rc = phy_read_reg(dev, port, 0, PHY_CONTROL_REG, &reg_d);
    if (rc != PASSED) {
        cterr('f',0, "phy smi read failed. port = %#x"
	      "page = %#x, reg = %#x, rc = %#x\n",
	      port, 0, PHY_CONTROL_REG, rc);
        return (FAILED);
    }

    if ((reg_d & PHY_AUTO_NEO_ENA) == PHY_AUTO_NEO_ENA) {
        reg_d &= ~PHY_AUTO_NEO_ENA;
        rc = phy_write_reg(dev, port, 0, PHY_CONTROL_REG, reg_d);
        if (rc != PASSED) {
	    cterr('f',0, "phy smi write failed. port = %#x"
		  "page = %#x, reg = %#x, rc = %#x\n",
		  port, 0, PHY_CONTROL_REG, rc);
            return (FAILED);
        }
        
        if (madHwPagedReset(dev,port,0) != MAD_OK) {
	    if (rc != PASSED) {
		cterr('f',0,"PHY soft reset failed.");
		return (FAILED);
	    }
	}
    }

    /* Disable power over Ethernet detection */
    rc = phy_read_reg(dev, port, 0, PHY_SPECIFIC_CONTROL3_REG, &reg_d);
    if (rc != PASSED) {
	cterr('f',0, "phy smi read failed. port = %#x"
	      "page = %#x, reg = %#x, rc = %#x\n",
	      port, 0, PHY_SPECIFIC_CONTROL3_REG, rc);
        return (FAILED);
    }
    
    reg_d &= ~PHY_P0_R26_DTE_DETECT;
    rc = phy_write_reg(dev, port, 0, PHY_SPECIFIC_CONTROL3_REG, reg_d);
    if (rc != PASSED) {
	cterr('f',0, "phy smi write failed. port = %#x"
	      "page = %#x, reg = %#x, rc = %#x\n",
	      port, 0, PHY_SPECIFIC_CONTROL3_REG, rc);
        return (FAILED);
    }

    /* Set DTE power status drop to 5 seconds */
    rc = phy_read_reg(dev, port, 0, PHY_SPECIFIC_CONTROL3_REG, &reg_d);
    if (rc != PASSED) {
	cterr('f',0, "phy smi read failed. port = %#x"
	      "page = %#x, reg = %#x, rc = %#x\n",
	      port, 0, PHY_SPECIFIC_CONTROL3_REG, rc);
        return (FAILED);
    }

    reg_d &= ~PHY_P0_R26_DTE_STATUS_DROP_MSK;
    reg_d |= PHY_P0_R26_DTE_STATUS_DROP_5S;
    rc = phy_write_reg(dev, port, 0, PHY_SPECIFIC_CONTROL3_REG, reg_d);
    if (rc != PASSED) {
	cterr('f',0, "phy smi write failed. port = %#x"
	      "page = %#x, reg = %#x, rc = %#x\n",
	      port, 0, PHY_SPECIFIC_CONTROL3_REG, rc);
        return (FAILED);
    }

    /* Enable power over Ethernet detection bit */
    rc = phy_read_reg(dev, port, 0, PHY_SPECIFIC_CONTROL3_REG, &reg_d);
    if (rc != PASSED) {
	cterr('f',0, "phy smi read failed. port = %#x"
	      "page = %#x, reg = %#x, rc = %#x\n",
	      port, 0, PHY_SPECIFIC_CONTROL3_REG, rc);
        return (FAILED);
    }
    
    reg_d |= PHY_P0_R26_DTE_DETECT;
    rc = phy_write_reg(dev, port, 0, PHY_SPECIFIC_CONTROL3_REG, reg_d);
    if (rc != PASSED) {
	cterr('f',0, "phy smi write failed. port = %#x"
	      "page = %#x, reg = %#x, rc = %#x\n",
	      port, 0, PHY_SPECIFIC_CONTROL3_REG, rc);
        return (FAILED);
    }

    /* enable Auto Negotiation and reset phy */
    rc = phy_read_reg(dev, port, 0, PHY_CONTROL_REG, &reg_d);
    if (rc != PASSED) {
        cterr('f',0, "phy smi read failed. port = %#x"
	      "page = %#x, reg = %#x, rc = %#x\n",
	      port, 0, PHY_CONTROL_REG, rc);
        return (FAILED);
    }

    reg_d |= PHY_AUTO_NEO_ENA;
    rc = phy_write_reg(dev, port, 0, PHY_CONTROL_REG, reg_d);
    if (rc != PASSED) {
	cterr('f',0, "phy smi write failed. port = %#x"
	      "page = %#x, reg = %#x, rc = %#x\n",
	      port, 0, PHY_CONTROL_REG, rc);
	return (FAILED);
    }
        
    if (madHwPagedReset(dev,port,0) != MAD_OK) {
        if (rc != PASSED) {
	    cterr('f',0,"PHY soft reset failed.");
            return (FAILED);
        }
    }
    
    for (ix = MRVL_PHONE_DETECT_TIME; ix; ix--) {
        printf("\r%d seconds left", ix);
        msleep(1000);
    }

    /* read detection status register */
    rc = phy_read_reg(dev, port, 0, PHY_SPECIFIC_STATUS1_REG, &detected);
    if (rc != PASSED) {
        cterr('f',0, "phy smi read failed. port = %#x"
	      "page = %#x, reg = %#x, rc = %#x\n",
	      port, 0, PHY_CONTROL_REG, rc);
        return (FAILED);
    }

    /* Disable power over Ethernet detection after detection */
     rc = phy_read_reg(dev, port, 0, PHY_SPECIFIC_CONTROL3_REG, &reg_d);
    if (rc != PASSED) {
	cterr('f',0, "phy smi read failed. port = %#x"
	      "page = %#x, reg = %#x, rc = %#x\n",
	      port, 0, PHY_SPECIFIC_CONTROL3_REG, rc);
        return (FAILED);
    }
    
    reg_d &= ~PHY_P0_R26_DTE_DETECT;
    rc = phy_write_reg(dev, port, 0, PHY_SPECIFIC_CONTROL3_REG, reg_d);
    if (rc != PASSED) {
	cterr('f',0, "phy smi write failed. port = %#x"
	      "page = %#x, reg = %#x, rc = %#x\n",
	      port, 0, PHY_SPECIFIC_CONTROL3_REG, rc);
        return (FAILED);
    }
    
    /* Check detection result */
    if (detected & PHY_P0_R17_DTE_NEED_POWER) {
        return (PASSED);    /* Found Cisco PD */
    } else {
        return (FAILED);    /* PD not detected */
    }
}


static int 
phy_1680_init(int port)
{
    MAD_DEV *dev = &phy_dev[get_slot_num()-1];
    int rc;
    MAD_U32 reg_d;

    assert(dev);

    /* workaround implement */
    rc = phy_read_reg(dev, port, 4, 27, &reg_d);
    if (rc != PASSED) {
	return (FAILED);
    }

    /* write 27.5 = 1 */
    reg_d |= 1 << 5;
    rc = phy_write_reg(dev, port, 4, 27, reg_d);
    if (rc != PASSED) {
	return (FAILED);
    }

    /* page 253, for QSGMII TX amplitude change */
    rc = phy_write_reg(dev, port, 0x00FD, 8, 0x0B53);
    if (rc != PASSED) {
	return (FAILED);
    }

    rc = phy_write_reg(dev, port, 0x00FD, 7, 0x200D);
    if (rc != PASSED) {
	return (FAILED);
    }

    /* page 255, for EEE initialization */
    rc = phy_write_reg(dev, port, 0x00FF, 17, 0xB030);
    if (rc != PASSED) {
	return (FAILED);
    }

    rc = phy_write_reg(dev, port, 0x00FF, 16, 0x215C);
    if (rc != PASSED) {
	return (FAILED);
    }

    return (PASSED);
}

static int 
phy_init (int port)
{
    MAD_DEV *dev = &phy_dev[get_slot_num()-1];
    int rc;
    MAD_U32 reg_d;

    cterr_add_component("Marvell PHY", 
			"SMI0 interface from xCat2 switch");
    cterr_add_debug("Check Marvell PHY",
		    "Check SMI0 interface from xCat2 switch");

    /* workaroud implement */
    /* 1680L revision A0 */
    rc = phy_1680_init(port);
    if (rc != PASSED) {
	return (rc);
    }

    /* power down/ power up QSMGII */
    rc= phy_read_reg(dev, port, 4, 0, &reg_d);
    if (rc != PASSED) {
	return (FAILED);
    }
    reg_d |= 1 << 11;
    rc= phy_write_reg(dev, port, 4, 0, reg_d);
    if (rc != PASSED) {
	return (FAILED);
    }
    
    rc= phy_read_reg(dev, port, 4, 0, &reg_d);
    if (rc != PASSED) {
	return (FAILED);
    }
    reg_d &= ~(1 << 11);
    rc= phy_write_reg(dev, port, 4, 0, reg_d);
    if (rc != PASSED) {
	return (FAILED);
    }

    /* disable MacSec */
    rc = phy_read_reg(dev, port, 18, 27, &reg_d);
    if (rc != PASSED) {
	return (FAILED);
    }

    /* write 27_18.13 = 0 */
    reg_d &= ~(1 << 13);
    rc = phy_write_reg(dev, port, 18, 27, reg_d);
    if (rc != PASSED) {
	return (FAILED);
    }

    /* enable QSGMII counter */
    rc = phy_write_reg(dev, port, 18, 18, 0x0006);
    if (rc != PASSED) {
	return (FAILED);
    }
    
    /* enable copper counter */
    rc = phy_write_reg(dev, port, 6, 16, 0x0010);
    if (rc != PASSED) {
	return (FAILED);
    }
    
    return (PASSED);
}


static int 
phy_config (port)
{
    MAD_DEV *dev = &phy_dev[get_slot_num()-1];
    int rc;
    MAD_U32 reg_d;

    /* Enable 1Gbps advertise (page 0, reg 9)*/
    rc = phy_read_reg(dev, port, 0, 9, &reg_d);
    if (rc != PASSED) {
	return (FAILED);
    }
    reg_d |= PHY_1000BT_ADV;
    rc = phy_write_reg(dev, port, 0, 9, reg_d);
    if (rc != PASSED) {
	return (FAILED);
    }

    /* Enable 10 & 100 Mbps advertise (page 0, reg 4)*/
    rc = phy_read_reg(dev, port, 0, 4, &reg_d);
    if (rc != PASSED) {
	return (FAILED);
    }
    reg_d |= PHY_100BT_ADV;
    reg_d |= PHY_10BT_ADV;
    rc = phy_write_reg(dev, port, 0, 4, reg_d);
    if (rc != PASSED) {
	return (FAILED);
    }

    /* config phy speed 1Gpbs for SGMII (page 2, reg 21) */
    rc = phy_read_reg(dev, port, 2, 21, &reg_d);
    if (rc != PASSED) {
	return (FAILED);
    }
    reg_d &= ~PHY_MAC_SPD_MASK;
    reg_d |= PHY_MAC_SPD_1000M;
    rc = phy_write_reg(dev, port, 2, 21, reg_d);
    if (rc != PASSED) {
	return (FAILED);
    }

    /* clear 1000BT PHY External loopback mode */
    rc = phy_read_reg(dev, port, 6, 18, &reg_d);
    if (rc != PASSED) {
	return (FAILED);
    }
    reg_d &= ~PHY_ENA_STUB_TEST;
    rc = phy_write_reg(dev, port, 6, 18, reg_d);
    if (rc != PASSED) {
	return (FAILED);
    }

    /* set 1Gbps & auto-neg & full-duplex, clear loopback, soft reset (page 0, reg 0) */
    reg_d = PHY_COPPER_FULL_DUPLEX | PHY_SPD_SEL_1000M | PHY_AUTO_NEO_ENA | PHY_COOPER_RST;
    rc = phy_write_reg(dev, port, 0, 0, reg_d);
    if (rc != PASSED) {
	return (FAILED);
    }

    return (PASSED);
}

static void
print_phy_counter()
{
    MAD_DEV *dev = &phy_dev[get_slot_num()-1];
    MAD_U32 reg_d1, reg_d2;
    int port_num = get_port_num();
    int i;

    printf("\n");
    for (i = 0; i < port_num; i++) {
	phy_read_reg(dev, i, 18, 17, &reg_d1);
	phy_read_reg(dev, i, 6, 17, &reg_d2);
	printf("counters for PHY port %d: QSGMII = 0x%x, Copper = 0x%x\n", i + 1, reg_d1, reg_d2);
    }
}

static void 
clear_phy_counter ()
{
    MAD_DEV *dev = &phy_dev[get_slot_num()-1];
    int port_num = get_port_num();
    int i;

    for (i = 0; i < port_num; i++) {
	phy_write_reg(dev, i, 6, 18, 0x0010);
	phy_write_reg(dev, i, 18, 18, 0x0016);
    }
}

static void
dump_phy_reg (int port)
{
    MAD_DEV *dev = &phy_dev[get_slot_num()-1];

    phy_display_reg(dev, port, 0);
    phy_display_reg(dev, port, 2);
    phy_display_reg(dev, port, 4);
    phy_display_reg(dev, port, 18);
}

static int 
reset_phy()
{
    unsigned char port;
    MAD_DEV *dev = &phy_dev[get_slot_num()-1];
    int rc;

    /* put PHY in reset */
    rc = phy_in_reset();
    if (rc != PASSED) {
	return (FAILED);
    }
    msleep(500);

    /* take PHY out of reset */
    rc = phy_out_of_reset();
    if (rc != PASSED) {
	return (FAILED);
    }
    msleep(500);

    /* disable all the interrupts */
    if (phy_disable_int(dev) == FAILED) {
	cterr('f',0,"Failed to disable PHY interrupts.");
	return FAILED;
    }

    /* int all 8 PHYs */
    for(port=0; port<dev->numOfPorts; port++) {
	if (phy_init(port) == FAILED)
	    return (FAILED);

	if (phy_config(port) == FAILED)
	    return (FAILED);
    }

    /* enable all 8 PHYs */
    for(port=0; port<dev->numOfPorts; port++) {
        if(mdSysSetPhyEnable(dev,port,MAD_TRUE) != PASSED) {
            cterr('f',0,"Failed to Enable Phy (port %i)\n",port);
            return FAILED;
        }
    }

    return PASSED;
}


#ifdef TACHI_INTEL
static int
port_setup_bdg (int port_num, int bp_port)
{
    int rc = PASSED;

    if (xcat2_config_port_pve(xcat2_dev_num[get_slot_num()-1], bp_port, port_num)) {
        cterr('f',0,"Failed to configure PVE for port %d", port_num);
        return (FAILED);
    }

    if (start_ext_lpbk(&phy_dev[get_slot_num()-1], port_num, MAD_SPEED_1000M)) {
        cterr('f',0,"Failed to enable external loopback for port %d", port_num);
        return (FAILED);
    }

    return (rc);
}


static void clr_bridge_nim_ge_port (void) {

    unsigned int ans, bp_port, phyport;          

    ans = getdec_answer("Dreamliner front panel ports: ", 1, 1, 8);

    if (ans % 2) {
        phyport = ans;
    } else {
        phyport = ans - 2;
    }

    clear_sw_counter();
    bp_port = GE0_XCAT2_PORT;
    if (xcat2_unconfig_port_pve(xcat2_dev_num[get_slot_num()-1], bp_port, phyport)) {
        printf("GE0: Failed to unconfigure PVE for port %d", phyport);
    }

    clear_sw_counter();
    bp_port = GE1_XCAT2_PORT;
    if (xcat2_unconfig_port_pve(xcat2_dev_num[get_slot_num()-1], bp_port, phyport)) {
        printf("GE1: Failed to unconfigure PVE for port %d", phyport);
    }
    return; 
}

static void clr_force_link_down_ge_port (void) {

    unsigned int bp_port; 

    bp_port = GE0_XCAT2_PORT; 
    clear_sw_counter();
    if (port_force_link_set(LINK_DOWN, bp_port, FALSE) != OK) {
        printf("Failed port_force_link_set()");
    }

    bp_port = GE1_XCAT2_PORT; 
    clear_sw_counter(); 

    if (port_force_link_set(LINK_DOWN, bp_port, FALSE) != OK) {
        printf("Failed port_force_link_set()");
    }
    return ;
}

static void print_cnt_sw_phy (void) {

    printf("\n------------SW------------ \n");
    print_sw_counter();

    printf("\n------------PHY------------ \n");
    print_phy_counter();
    return; 
}

static void clr_cnt_sw_phy (void) {

    clear_phy_counter();
    clear_sw_counter();
    return; 
}

void phy_cfg_dbg1 (void)
{
    int ans = 0, i, m;

    printf("%s\n", __FUNCTION__);
    ans = getdec_answer("reset PHY YES-1  NO-0: ", 0, 0, 1);

    if (ans == 1) {
        reset_phy();
    }

    clear_sw_counter();

    if (port_force_link_set(LINK_DOWN, GE0_XCAT2_PORT, TRUE) != OK) {
        printf("Failed port_force_link_set()");
    }

    i = getdec_answer("PHY port (1-8): ", 1, 1, 8);

    if (i % 2)
        m = i;
    else
        m = i - 2;

    /* clear PHY counter */
    clear_phy_counter();

    if (port_setup_bdg(m, GE1_XCAT2_PORT)) {
        printf("Failed port_external_lpbk_test_dbg()");
    }

    clear_sw_counter();
    clear_phy_counter();

    return;
}

void phy_cfg_dbg0 (void)
{
    int ans = 0, i, m;

    printf("%s\n", __FUNCTION__);
    ans = getdec_answer("reset PHY (1/0): ", 0, 0, 1);

    if (ans == 1) {
        reset_phy();
    }

    clear_sw_counter();

    if (port_force_link_set(LINK_DOWN, GE1_XCAT2_PORT, TRUE) != OK) {
        printf("Failed port_force_link_set()");
    }

    i = getdec_answer("PHY port (1-8): ", 1, 1, 8);

    if (i % 2)
        m = i;
    else
        m = i - 2;

    /* clear PHY counter */
    clear_phy_counter();

    if (port_setup_bdg(m, GE0_XCAT2_PORT)) {
        printf("Failed port_external_lpbk_test_dbg()");
    }

    clear_sw_counter();
    clear_phy_counter();

    return;
}

void phy_lpbk_ge_init (int type, int phy_port)
{
    int ge_port, dis_port, m;
    int rc = 0;
    int dev_num = 0;

    /* type  = 0 for ge0 internal - GE0_INTER_LPBK
       type  = 1 for ge1 internal - GE1_INTER_LPBK
       type  = 2 for ge0 external - GE0_EXTER_LPBK
       there is no ge1 external originally 
     */
#ifdef TACHI_INTEL
    if (type == GE1_INTER_LPBK) {  
        ge_port = GE1_XCAT2_PORT;       
        dis_port = GE0_XCAT2_PORT;       
    } else {  
        ge_port = GE0_XCAT2_PORT;       
        dis_port = GE1_XCAT2_PORT;       
    }
#else 
    if (type == GE0_INTER_LPBK) {  
        ge_port = GE1_XCAT2_PORT;       
        dis_port = GE0_XCAT2_PORT;       
    } else {  
        ge_port = GE0_XCAT2_PORT;       
        dis_port = GE1_XCAT2_PORT;       
    }
#endif
    reset_phy();
    clear_sw_counter();

    if (port_force_link_set(LINK_DOWN, dis_port, TRUE) != OK) {
        printf("Failed port_force_link_set()\n");
    }

#ifndef TACHI_INTEL
    if (phy_port % 2)
        m = phy_port;
    else
        m = phy_port - 2;
#endif
    /* clear PHY counter */
    clear_phy_counter();

#ifdef TACHI_INTEL
    phy_port --;
    m = phy_port;
    /* Set VLAN setting, using VLAN 5 for loopback testing */
    rc = xcat2_vlan_port_add(dev_num, VLAN_5, ge_port);
    if(rc != OK) {
        cterr('f',0,"Failed to add port: %d to Vlan 5.", ge_port);
        return; 
    }
    /* The actual port mapping is 0 - 7 */
    rc = xcat2_vlan_port_add(dev_num, VLAN_5, phy_port );
    if(rc != OK) {
        cterr('f',0,"Failed to add port: %d to Vlan 5.", phy_port);
        return; 
    }
#else
    if (xcat2_config_port_pve(xcat2_dev_num[get_slot_num()-1], ge_port, m)) {
        printf("Failed to configure PVE for port %d", m);
        return; 
    }
#endif

    if (type == GE0_EXTER_LPBK) {
        if (start_ext_lpbk(&phy_dev[get_slot_num()-1], m, MAD_SPEED_1000M)) {
            printf("Failed to enable external loopback for port %d", m);
            return;
        }
    } else {
        if (start_mac_lpbk(&phy_dev[get_slot_num()-1], m, MAD_SPEED_1000M)) {
	     printf("Failed to enable PHY loopback for port %d", m);
	     return;
        }
    }

    return;
}

/* ge_port: 0 - ge0, 1 - ge1 ;
 * phy_port: 1-8 or 1-4, depends on sku 
 */
void phy_lpbk_ge_clr (int ge_port, int phy_port)
{
    int dis_port, m;
    int rc = 0;
    int dev_num = 0;

    if (ge_port == 1) {
        ge_port = GE1_XCAT2_PORT;
        dis_port = GE0_XCAT2_PORT;
    } else {
        ge_port = GE0_XCAT2_PORT;
        dis_port = GE1_XCAT2_PORT;
    }

#ifdef TACHI_INTEL
    phy_port --;
    m = phy_port;
    
    /* Unset the VLAN */
    rc = xcat2_vlan_port_del(dev_num, VLAN_5, ge_port); 
    if(rc != OK) {
        cterr('f',0,"Failed to del port: %d to Vlan 5.", ge_port);
        return;
    } 

    rc = xcat2_vlan_port_del(dev_num, VLAN_5, m); 
    if(rc != OK) {
        cterr('f',0,"Failed to del port: %d to Vlan 5.", m);
        return;
    }

#else
    if (phy_port % 2)
        m = phy_port;
    else
        m = phy_port - 2;

    if (xcat2_unconfig_port_pve(xcat2_dev_num[get_slot_num()-1], ge_port, m)) {
        printf("Failed to unconfigure PVE for port %d\n", phy_port);
    }
#endif

    if (phy_config(m) == FAILED) {
        printf("Failed to init PHY for port %d\n", m);
    }
#ifdef TACHI_INTEL
    if (port_force_link_set(LINK_DOWN, dis_port, FALSE) != OK) {
#else
    if (port_force_link_set(LINK_DOWN, dis_port, TRUE) != OK) {
#endif        
        printf("Failed port_force_link_set()\n");
    }

    return;
}

void test_ge_init (void) {

    int type, port;

    /* type  = 0 for ge0 internal - GE0_INTER_LPBK
       type  = 1 for ge1 internal - GE1_INTER_LPBK
       type  = 2 for ge0 external - GE0_EXTER_LPBK
       there is no ge1 external originally 
     */
    type = getdec_answer("Enter type: ", 0, 0, 2); 
    port = getdec_answer("Enter port: ", 1, 1, 8); 
    phy_lpbk_ge_init(type,  port);

    return; 
}


void test_ge_clr (void) {

    int type, port;

    /* type  = 0 for ge0 internal - GE0_INTER_LPBK
       type  = 1 for ge1 internal - GE1_INTER_LPBK
       type  = 2 for ge0 external - GE0_EXTER_LPBK
       there is no ge1 external originally 
     */
    type = getdec_answer("Enter type: ", 0, 0, 2);
    port = getdec_answer("Enter port: ", 1, 1, 8);
    phy_lpbk_ge_clr(type,  port);

    return;
}


#endif 
/*
 *------------------------------------------------------------------
 * $Log: dreamliner_phy.c,v $
 * Revision 1.10  2020/01/09 01:02:10  jiajliu
 * Merge Curie 2RU to main trunk
 *
 * Revision 1.9  2019/10/17 02:16:15  kehuang2
 * Collapse Tabei-L into main trunk
 *
 * Revision 1.8  2019/08/06 06:56:07  alpeng
 * merge curie, switzer and nightwatch to trunk
 *
 * Revision 1.7.36.1  2018/07/16 09:28:04  alpeng
 * skip ge switch portions for prince, reva, arkenstone and dreamliner
 *
 * Revision 1.7  2017/08/10 10:10:37  iachang
 * CSCvf44161: Merge Goldbeach into USD platform as one image
 *
 * Revision 1.6  2017/03/30 08:23:23  hondwang
 * Tachi-L brach merge
 *
 * Revision 1.5.2.1  2016/12/21 12:46:13  hondwang
 * Fix dreamliner loopback issue
 *
 * Revision 1.5  2016/10/19 01:42:50  iachang
 * Fixed TACHI compile error. Tachi intel didn't support get_sgmii_port_num() funciton.
 *
 * Revision 1.4  2016/10/16 12:28:15  iachang
 * Supported Goldbeach Platform.
 *
 * Revision 1.3  2016/04/20 07:06:40  benchen2
 * merge tachi branch into main trunk
 *
 * Revision 1.2.4.7  2016/04/18 08:57:39  alpeng
 * fix for prrq
 *
 * Revision 1.2.4.6  2016/01/20 01:26:07  alpeng
 * update include file for tachi-l
 *
 * Revision 1.2.4.5  2015/12/29 12:27:07  alpeng
 *  update dreamlienr utilities for cross functional team to setup vlan
 *
 * Revision 1.2.4.4  2015/12/09 10:35:57  alpeng
 * update code to support lpbk test on bmc for dreamliner
 *
 * Revision 1.2.4.3  2015/11/03 09:43:51  alpeng
 * update dreamliner utility to support sw bridge
 *
 * Revision 1.2.4.2  2015/10/05 10:21:39  alpeng
 * support single test, update loopback test
 *
 * Revision 1.2.4.1  2015/08/17 02:33:03  alpeng
 * first check in for tachi-intel test; fix smart_cookie.c and free.h
 *
 * Revision 1.2  2015/02/27 10:02:20  iachang
 *
 * Add support dreamliner NIM
 *
 * Revision 1.1.6.2  2015/02/14 07:13:53  iachang
 * Dreamliner Diag sync with main trunk.
 *
 * Revision 1.1.4.4  2015/02/06 12:06:18  iachang
 * Port number start from 1
 * Loopback test all ports, and report the fail port information.
 * 
 * Revision 1.1.4.3  2015/02/06 10:39:24  iachang
 * External loopback tested all ports if didn't plug in loopback
 * 
 * Revision 1.1.4.2  2015/01/28 22:59:21  iachang
 * Dreamliner-branch2 initial check-in.
 * 
 * Revision 1.1.2.1  2014/12/02 08:04:10  iachang
 * Dreamliner Diag initial check-in.
 *------------------------------------------------------------------
 * $Endlog$
 */

