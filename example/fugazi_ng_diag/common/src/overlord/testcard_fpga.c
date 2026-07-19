/* $Id: testcard_fpga.c,v 1.11 2020/05/22 02:28:34 qingcwan Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/overlord/testcard_fpga.c,v $
 *------------------------------------------------------------------
 * Filename   : testcard_fpga.c
 *
 * Description: TestCard FPGA related diag tests and utilities.
 *
 * Copyright (c) 2013-2019 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "byteswap.h"
#include "common.h"
#include "common_utils.h"
#include "defs.h"
#include "error.h"
#include "menu.h"
#include "nvmonvars.h"
#include "i2c_api.h"
#include "i2c_address.h"
#include "slot.h"
#include "plat_defs.h"
#include "proto.h"
#include "ngio_testcard.h"
#include "testcard_fpga.h"
#include "platform_i2c.h"
#include "queryflags.h"
#include "goofy_i2c.h"
#include "cross_platform.h"
#include "platform_cookie.h"
#include "slot.h"

#ifdef OVERLORD
#include "dash_fpga.h"
#endif


/******************************************************************************* 
 *                            Function Prototypes                              *
 *******************************************************************************
 */
void build_tc_fpga_menu(int);
int  tc_fpga_reg_rd(uint32_t, uint16_t *);
int  tc_fpga_reg_wr(uint32_t, uint16_t);
int  tc_alter_fpga_reg_wrap(void);
int  tc_read_fpga_reg_wrap(void);
int  tc_fpga_rev_checker(uint16_t, char *, uint16_t);
int  tc_pll_locked_test(void);
int  tc_fpga_reg_test(void);
int  tc_sync_sig_test(void);
int  tc_minus_54v_detect_test(void);

static void build_tc_fpga_utils(int);
static int  fpga_dump_all_reg(void);


/*******************************************************************************
 *                                  Externs                                    *
 *******************************************************************************
 */
#ifdef OVERLORD
extern int dash_alt_mem(int argc);
#endif
extern int get_platform_ver(unsigned int, unsigned int *, unsigned int *,
                            unsigned int *, unsigned int *);
extern int slot_get_73_part_num(uchar *, uchar *);
extern int get_cookie_pid_wrap (int, int, uchar *, char *);
extern int do_all_menu_items(struct menuinfo *);

extern ngio_if  *tc_ngio_p;
extern uint32_t ovld_check_poe_psu_wrap (void);

/*******************************************************************************
 *                              Global Variables                               *
 *******************************************************************************
 */
#define TC_FPGA_V105                  0x0105
#define TC_FPGA_V107                  0x0107

#define TC_SIG_TEST_FPGA_THRES           0x0105
#define TC_SIG_TEST_HOST_FPGA_THRES_O2    0x020400
#define TC_SIG_TEST_HOST_FPGA_THRES_USD   0x000300
#define HOST_FPGA_REV_MSK                 0x7FFFFF

#define TC_SYNC_SIG_TEST_PATTERN1     0x01180118
#define TC_SYNC_SIG_TEST_PATTERN2     0x01FE01FE
#define TC_SYNC_SIG_TEST_PATTERN3     0x01FF01FF

/* PLL Lock test */
#define TC_PLL_TEST_FPGA_THRES        0x0105
#define TC_NGSM_PLL_LOCK_THRES        3
#define TC_NGWIC_PLL_LOCK_THRES       4

#define TC_MINUS_54V_TEST_FPGA_THRES  0x0107

static n2g_i2c_if_t tc_fpga_i2c_if;

static reg_info_t tc_fpga_reg_tbl_v104[]=
{
    {"FPGA ID",                   FPGA_ID_REG_OFFSET,
     READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT},
     0x0000, 0x0000},
    {"Individual Device Reset",   DEV_RESET_REG_OFFSET,
     READ_WRITE | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT},
     0x007F, 0x0000},
    {"SMI0 Speed & Status",       SMI0_STAT_REG_OFFSET,
     READ_WRITE | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT},
     0x3F80, 0x0480},
    {"SMI0 Control",              SMI0_CTRL_REG_OFFSET,
     READ_WRITE | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT},
     0xFFFC, 0x6200},
    {"SMI0 Data & Address",       SMI0_ADDR_REG_OFFSET,
     READ_WRITE | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT},
     0xFFFF, 0x0000},
    {"SMI1 Speed & Status",       SMI1_STAT_REG_OFFSET,
     READ_WRITE | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT},
     0x3F80, 0x0080},
    {"SMI1 Control",              SMI1_CTRL_REG_OFFSET,
     READ_WRITE | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT},
     0xFFFC, 0x0200},
    {"SMI1 Data & Address",       SMI1_ADDR_REG_OFFSET,
     READ_WRITE | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT},
     0xFFFF, 0x0000},
    {"UART Control",              UART_REG_OFFSET,
     READ_WRITE | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT},
     0x0003, 0x0000},
    {"PCIe Loopback",             PCIE_LPBK_REG_OFFSET,
     READ_WRITE | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT},
     0x0001, 0x0000},
    {"GPIO-Expander Status",      GPIO_STAT_REG_OFFSET,
     READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT},
     0x0000, 0x0000},
    {"External Module Control",   EXT_MOD_REG_OFFSET,
     READ_WRITE | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT},
     0x01FF, 0x0000},
    {"GPIO-Expander Interrupt",   GPIO_INTR_REG_OFFSET,
     READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT},
     0x0000, 0x0000},
    {"GE and XAUI Interrupt",     PHY_INTR_REG_OFFSET,
     READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT},
     0x0000, 0x0000},
    {"SyncE Loopback Control",    SYNC_LPBK_REG_OFFSET,
     READ_WRITE | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT},
     0x0003, 0x0000},
    {"Hot Swap Controller GPIO3", HOT_SWAP_REG_OFFSET,
     READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT},
     0x0000, 0x0000},
    {"End", 0xFFFFFFFF, READ_ONLY, {0}, 0x0000, 0x0000},
};


static reg_info_t tc_fpga_reg_tbl_v105[]=
{
    {"FPGA ID",                   FPGA_ID_REG_OFFSET,
     READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT},
     0x0000, 0x0000},
    {"Individual Device Reset",   DEV_RESET_REG_OFFSET,
     READ_WRITE | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT},
     0x007F, 0x0000},
    {"SMI0 Speed & Status",       SMI0_STAT_REG_OFFSET,
     READ_WRITE | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT},
     0x3F80, 0x0480},
    {"SMI0 Control",              SMI0_CTRL_REG_OFFSET,
     READ_WRITE | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT},
     0xFFFC, 0x6200},
    {"SMI0 Data & Address",       SMI0_ADDR_REG_OFFSET,
     READ_WRITE | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT},
     0xFFFF, 0x0000},
    {"SMI1 Speed & Status",       SMI1_STAT_REG_OFFSET,
     READ_WRITE | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT},
     0x3F80, 0x0080},
    {"SMI1 Control",              SMI1_CTRL_REG_OFFSET,
     READ_WRITE | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT},
     0xFFFC, 0x0200},
    {"SMI1 Data & Address",       SMI1_ADDR_REG_OFFSET,
     READ_WRITE | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT},
     0xFFFF, 0x0000},
    {"UART Control",              UART_REG_OFFSET,
     READ_WRITE | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT},
     0x0003, 0x0000},
    {"PCIe Loopback",             PCIE_LPBK_REG_OFFSET,
     READ_WRITE | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT},
     0x0001, 0x0000},
    {"GPIO-Expander Status",      GPIO_STAT_REG_OFFSET,
     READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT},
     0x0000, 0x0000},
    {"External Module Control",   EXT_MOD_REG_OFFSET,
     READ_WRITE | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT},
     0x01FF, 0x0000},
    {"GPIO-Expander Interrupt",   GPIO_INTR_REG_OFFSET,
     READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT},
     0x0000, 0x0000},
    {"GE and XAUI Interrupt",     PHY_INTR_REG_OFFSET,
     READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT},
     0x0000, 0x0000},
    {"SyncE Loopback Control",    SYNC_LPBK_REG_OFFSET,
     READ_WRITE | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT},
     0x0003, 0x0000},
    {"Hot Swap Controller GPIO3", HOT_SWAP_REG_OFFSET,
     READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT},
     0x0000, 0x0000},
    {"PLL Lock",                  PLL_LOCK_REG_OFFSET,
     READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT},
     0x0000, 0x0000},
    {"End", 0xFFFFFFFF, READ_ONLY, {0}, 0x0000, 0x0000},
};


static reg_info_t tc_fpga_reg_tbl_v107[]=
{
    {"FPGA ID",                   FPGA_ID_REG_OFFSET,
     READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT},
     0x0000, 0x0000},
    {"Individual Device Reset",   DEV_RESET_REG_OFFSET,
     READ_WRITE | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT},
     0x007F, 0x0000},
    {"SMI0 Speed & Status",       SMI0_STAT_REG_OFFSET,
     READ_WRITE | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT},
     0x3F80, 0x0480},
    {"SMI0 Control",              SMI0_CTRL_REG_OFFSET,
     READ_WRITE | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT},
     0xFFFC, 0x6200},
    {"SMI0 Data & Address",       SMI0_ADDR_REG_OFFSET,
     READ_WRITE | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT},
     0xFFFF, 0x0000},
    {"SMI1 Speed & Status",       SMI1_STAT_REG_OFFSET,
     READ_WRITE | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT},
     0x3F80, 0x0080},
    {"SMI1 Control",              SMI1_CTRL_REG_OFFSET,
     READ_WRITE | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT},
     0xFFFC, 0x0200},
    {"SMI1 Data & Address",       SMI1_ADDR_REG_OFFSET,
     READ_WRITE | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT},
     0xFFFF, 0x0000},
    {"UART Control",              UART_REG_OFFSET,
     READ_WRITE | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT},
     0x0003, 0x0000},
    {"PCIe Loopback",             PCIE_LPBK_REG_OFFSET,
     READ_WRITE | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT},
     0x0001, 0x0000},
    {"GPIO-Expander Status",      GPIO_STAT_REG_OFFSET,
     READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT},
     0x0000, 0x0000},
    {"External Module Control",   EXT_MOD_REG_OFFSET,
     READ_WRITE | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT},
     0x01FF, 0x0000},
    {"GPIO-Expander Interrupt",   GPIO_INTR_REG_OFFSET,
     READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT},
     0x0000, 0x0000},
    {"GE and XAUI Interrupt",     PHY_INTR_REG_OFFSET,
     READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT},
     0x0000, 0x0000},
    {"SyncE Loopback Control",    SYNC_LPBK_REG_OFFSET,
     READ_WRITE | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT},
     0x0003, 0x0000},
    {"Hot Swap Controller GPIO3", HOT_SWAP_REG_OFFSET,
     READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT},
     0x0000, 0x0000},
    {"PLL Lock",                  PLL_LOCK_REG_OFFSET,
     READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT},
     0x0000, 0x0000},
    {"-54V Present",              MINUS_54V_REG_OFFSET,
     READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT},
     0x0000, 0x0000},
    {"End", 0xFFFFFFFF, READ_ONLY, {0}, 0x0000, 0x0000},
};

/*******************************************************************************
 *                                   Menus                                     *
 *******************************************************************************
 */
/*
 * FPGA Tests and Utilities Main Menu
 */
static submenu_xtable_t tc_fpga_diag_tbl[] = {
    {"FPGA Utilities",      (PFT)build_tc_fpga_utils, TRUE,
     0,                          (PFT)0, 0, (PFT)build_tc_fpga_utils, TRUE},
    {"FPGA registers test", (PFT)tc_fpga_reg_test,    FALSE,
     (MF_CONTINUOUS | MF_DOALL), (PFT)0, 0, (PFT)0, 0},
    {"POE -54V detection test", (PFT)tc_minus_54v_detect_test,    FALSE,
     (MF_CONTINUOUS | MF_DOALL), (PFT)0, 0, (PFT)0, 0},
};

#define TC_FPGA_DIAG_TBL_SIZE (sizeof(tc_fpga_diag_tbl) / sizeof(submenu_xtable_t))

/*
 * Primary & secondary submenu items (filled in from xtable)
 */
static mitem_t tc_fpga_diag_pri_items[TC_FPGA_DIAG_TBL_SIZE + MAX_BASE_ITEMS];
static mitem_t tc_fpga_diag_sec_items[TC_FPGA_DIAG_TBL_SIZE + MAX_BASE_ITEMS];

static struct menuinfo tc_fpga_diag = {
    "TestCard FPGA SubMenu",   /* title */
    0,                         /* title string added by init_empty_menu */
    (PFT)menu_show_dflags,     /* shows major flags */
    0,                         /* generic prompt */
    0,                         /* size -- bumped by add_menu_item() */
    tc_fpga_diag_pri_items,
};

static struct menuinfo *tc_fpga_diag_p = &tc_fpga_diag;


/*
 * FPGA Utilities SubMenu
 */
static submenu_xtable_t fpga_utils_tbl[] = {
#ifdef OVERLORD
    {"Alter Host(Overlord) FPGA", (PFT)dash_alt_mem,           TRUE,
     0, (PFT)0, 0, (PFT)0, 0},
#endif
    {"Dump all FPGA registers", (PFT)fpga_dump_all_reg,        FALSE,
     0, (PFT)0, 0, (PFT)0, 0},
    {"Alter FPGA register",     (PFT)tc_alter_fpga_reg_wrap,   FALSE,
     0, (PFT)0, 0, (PFT)0, 0},
    {"Read FPGA register",      (PFT)tc_read_fpga_reg_wrap,    FALSE,
     0, (PFT)0, 0, (PFT)0, 0},
};

#define FPGA_UTILS_TBL_SIZE (sizeof(fpga_utils_tbl) / sizeof(submenu_xtable_t))

/*
 * Primary & secondary submenu items (filled in from xtable)
 */
static mitem_t tc_fpga_utils_pri_items[FPGA_UTILS_TBL_SIZE + MAX_BASE_ITEMS];
static mitem_t tc_fpga_utils_sec_items[FPGA_UTILS_TBL_SIZE + MAX_BASE_ITEMS];

static struct menuinfo tc_fpga_utils = {
    "TestCard FPGA Utilities",   /* title */
    0,                           /* title string added by init_empty_menu */
    (PFT)menu_show_dflags,       /* shows major flags */
    0,                           /* generic prompt */
    0,                           /* size -- bumped by add_menu_item() */
    tc_fpga_utils_pri_items,
};

static struct menuinfo *tc_fpga_utils_p = &tc_fpga_utils;


/*
 * Sync signals Tests and Utilities Main Menu
 */
static submenu_xtable_t tc_sig_diag_tbl[] = {
    {"FPGA Utilities",      (PFT)build_tc_fpga_utils, TRUE,
     0,                          (PFT)0, 0, (PFT)build_tc_fpga_utils, TRUE},
    {"Sync Signals test",   (PFT)tc_sync_sig_test,    FALSE,
     (MF_CONTINUOUS | MF_DOALL), (PFT)0, 0, (PFT)0, 0},
};

#define TC_SIG_DIAG_TBL_SIZE (sizeof(tc_sig_diag_tbl) / sizeof(submenu_xtable_t))

/*
 * Primary & secondary submenu items (filled in from xtable)
 */
static mitem_t tc_sig_diag_pri_items[TC_SIG_DIAG_TBL_SIZE + MAX_BASE_ITEMS];
static mitem_t tc_sig_diag_sec_items[TC_SIG_DIAG_TBL_SIZE + MAX_BASE_ITEMS];

static struct menuinfo tc_sig_diag = {
    "TestCard Sync Signal SubMenu",   /* title */
    0,                                /* title string added by init_empty_menu */
    (PFT)menu_show_dflags,            /* shows major flags */
    0,                                /* generic prompt */
    0,                                /* size -- bumped by add_menu_item() */
    tc_sig_diag_pri_items,
};

static struct menuinfo *tc_sig_diag_p = &tc_sig_diag;

/*
 * PLL Lock Tests and Utilities Main Menu
 */
static submenu_xtable_t tc_pll_diag_tbl[] = {
    {"FPGA Utilities",           (PFT)build_tc_fpga_utils, TRUE,
     0,                          (PFT)0, 0, (PFT)build_tc_fpga_utils, TRUE},
    {"PLL Lock test(100MHz PCIe Ref. CLK)", (PFT)tc_pll_locked_test,  FALSE,
     (MF_CONTINUOUS | MF_DOALL), (PFT)0, 0, (PFT)0, 0},
};

#define TC_PLL_DIAG_TBL_SIZE (sizeof(tc_pll_diag_tbl) / sizeof(submenu_xtable_t))

/*
 * Primary & secondary submenu items (filled in from xtable)
 */
static mitem_t tc_pll_diag_pri_items[TC_PLL_DIAG_TBL_SIZE + MAX_BASE_ITEMS];
static mitem_t tc_pll_diag_sec_items[TC_PLL_DIAG_TBL_SIZE + MAX_BASE_ITEMS];

static struct menuinfo tc_pll_diag = {
    "TestCard PLL Lock SubMenu",      /* title */
    0,                                /* title string added by init_empty_menu */
    (PFT)menu_show_dflags,            /* shows major flags */
    0,                                /* generic prompt */
    0,                                /* size -- bumped by add_menu_item() */
    tc_pll_diag_pri_items,
};

static struct menuinfo *tc_pll_diag_p = &tc_pll_diag;

/*******************************************************************************
 *
 * Function   : build_tc_pll_menu
 * Description: Build TestCard PLL Lock tests and utilities submenu.
 * Inputs     : submenu - TRUE if invoked from submenu
 * Outputs    : None
 *
 *******************************************************************************
 */
void build_tc_pll_menu (int submenu)
{
    build_primary_submenu(tc_pll_diag_tbl, TC_PLL_DIAG_TBL_SIZE,
                          "TestCard PLL Lock SubMenu", &tc_pll_diag_p);
    build_secondary_submenu(tc_pll_diag_tbl, TC_PLL_DIAG_TBL_SIZE,
                            tc_pll_diag_sec_items);

    if (submenu) {
        /* Entered with submenu */
        menu(&tc_pll_diag, tc_pll_diag_sec_items, 0);
    } else {
        do_all_menu_items(tc_pll_diag_p);
    }
}

/*******************************************************************************
 *
 * Function   :	build_tc_sig_menu
 * Description:	Build TestCard Sync Signals tests and utilities submenu.
 * Inputs     :	submenu - TRUE if invoked from submenu
 * Outputs    : None
 *
 *******************************************************************************
 */
void build_tc_sig_menu (int submenu)
{
    build_primary_submenu(tc_sig_diag_tbl, TC_SIG_DIAG_TBL_SIZE,
                          "TestCard Sync Signal SubMenu", &tc_sig_diag_p);
    build_secondary_submenu(tc_sig_diag_tbl, TC_SIG_DIAG_TBL_SIZE,
                            tc_sig_diag_sec_items);

    if (submenu) {
        /* Entered with submenu */
        menu(&tc_sig_diag, tc_sig_diag_sec_items, 0);
    } else {
        do_all_menu_items(tc_sig_diag_p);
    }
}


/*******************************************************************************
 *
 * Function   :	build_tc_fpga_menu
 * Description:	Build TestCard FPGA tests and utilities submenu.
 * Inputs     :	submenu - TRUE if invoked from submenu
 * Outputs    : None
 *
 *******************************************************************************
 */
void build_tc_fpga_menu (int submenu)
{
    build_primary_submenu(tc_fpga_diag_tbl, TC_FPGA_DIAG_TBL_SIZE,
                          "TestCard FPGA SubMenu", &tc_fpga_diag_p);
    build_secondary_submenu(tc_fpga_diag_tbl, TC_FPGA_DIAG_TBL_SIZE,
                            tc_fpga_diag_sec_items);

    if (submenu) {
        /* Entered with submenu */
        menu(&tc_fpga_diag, tc_fpga_diag_sec_items, 0);
    } else {
        do_all_menu_items(tc_fpga_diag_p);
    }
}


/*******************************************************************************
 *
 * Function   : build_tc_fpga_utils
 * Description: Build TestCard FPGA related utilities submenu.
 * Inputs     : submenu - TRUE if invoked from submenu
 * Outputs    : None
 *
 *******************************************************************************
 */
static void build_tc_fpga_utils (int submenu)
{
    build_primary_submenu(fpga_utils_tbl, FPGA_UTILS_TBL_SIZE,
                          "TestCard Ethernet Utilities", &tc_fpga_utils_p);
    build_secondary_submenu(fpga_utils_tbl, FPGA_UTILS_TBL_SIZE,
                            tc_fpga_utils_sec_items);

    menu(&tc_fpga_utils, tc_fpga_utils_sec_items, 0);
}


/*******************************************************************************
 *
 * Function   :	tc_fpga_reg_test
 * Description:	TestCard FPGA registers test.
 * Inputs     :	None
 * Outputs    : None
 *
 *******************************************************************************
 */
int tc_fpga_reg_test (void)
{
    uint32_t   ctr = 0, test_ctr = 0, test_round = 0, test_size = 0;
    uint16_t   ori_val = 0, test_data = 0, check_data = 0, test_mask = 0;
    uint16_t   tc_fpga_rev = 0;
    reg_info_t *reg_p = 0;

    testname("TestCard FPGA register");
    prpass(testpass, "%s%d ", testcard_if_p->type_name, testcard_if_p->slot);

    if (tc_fpga_reg_rd(FPGA_ID_REG_OFFSET, &tc_fpga_rev) != PASSED) {
        cterr('f', 0, "%s: Failed to read FPGA reigster 0x%01X back.\n",
                      __FUNCTION__, FPGA_ID_REG_OFFSET);
        return (FAILED);
    }

    if ((tc_fpga_rev & TC_FPGA_REV_MSK) < TC_FPGA_V105) {
        reg_p = &tc_fpga_reg_tbl_v104[0];
        test_size = (sizeof(tc_fpga_reg_tbl_v104) / sizeof(reg_info_t));
    } else {
        reg_p = &tc_fpga_reg_tbl_v105[0];
        test_size = (sizeof(tc_fpga_reg_tbl_v105) / sizeof(reg_info_t));
    }

    for (ctr = 0; ctr < test_size; ctr++, reg_p++) {
        if (!strcmp(reg_p->name, "End")) {
            break;
        }

        /* Skip Device Reset Register */
        if (reg_p->offset == DEV_RESET_REG_OFFSET) {
            continue;
        }

        /* Based on TestCard FPGA reigster document,
         * these registers are updated on a read command ONLY.
         * So just test write-in functionality.
         */
        if ((reg_p->offset == SMI0_ADDR_REG_OFFSET) ||
            (reg_p->offset == SMI1_ADDR_REG_OFFSET)) {
            if (tc_fpga_reg_wr(reg_p->offset, (uint16_t)FPGA_REG_TEST_PATTERN) != PASSED) {
                cterr('f', 0, "%s: Failed to wrote 0x%04X "
                              "to Test Card FPGA Reg %#x.",
                              __FUNCTION__, FPGA_REG_TEST_PATTERN, reg_p->offset);
            }

            continue;
        }

        if (!(reg_p->type & READ_ONLY) || (reg_p->type & WRITE_ONLY)) {
            /* Backup Original value */
            if (tc_fpga_reg_rd(reg_p->offset, &ori_val) != PASSED) {
                cterr('f', 0, "%s: Failed to read Test Card FPGA Reg %#x",
                      __FUNCTION__, reg_p->offset);
                return (FAILED);
            }

            /*
             * Ripple 1 test
             */
            for (test_ctr = 0; test_ctr < (sizeof(tc_fpga_i2c_if.size) * 8);
                 test_ctr++) {
                test_data = ((1 << test_ctr) & reg_p->mask);
                if (!test_data) {
                    continue;
                }

                /* Write Test Data in */
                if (tc_fpga_reg_wr(reg_p->offset, test_data) != PASSED) {
                    cterr('f', 0, "%s: Failed to wrote 0x%04X "
                                  "to Test Card FPGA Reg %#x.",
                                  __FUNCTION__, test_data, reg_p->offset);
                }

                /* Read the register value back for double check */
                if (tc_fpga_reg_rd(reg_p->offset, &check_data) != PASSED) {
                    cterr('f', 0, "%s: Failed to read Test Card FPGA Reg %#x",
                          __FUNCTION__, reg_p->offset);
                    return (FAILED);
                }

                /* Data Comparation */
                if (check_data != test_data) {
                    cterr('f', 0, "%s Reg. Ripple one test FAILED, "
                                  "read back = 0x%04x and expected = 0x%04x.",
                                  reg_p->name, check_data, test_data);
                    return (FAILED);
                }
            }   /* End of Ripple 1 Test */

            /*
             * Ripple 0 test
             */
            for (test_ctr = 0; test_ctr < (sizeof(tc_fpga_i2c_if.size) * 8);
                 test_ctr++) {
                test_data = (1 << test_ctr);
                if (!test_data) {
                    continue;
                }

                test_data = ((~(1 << test_ctr)) & reg_p->mask);

                /* Write Test Data in */
                if (tc_fpga_reg_wr(reg_p->offset, test_data) != PASSED) {
                    cterr('f', 0, "%s: Failed to wrote 0x%04X "
                                  "to Test Card FPGA Reg %#x.",
                                  __FUNCTION__, test_data, reg_p->offset);
                }

                /* Read the register value back for double check */
                if (tc_fpga_reg_rd(reg_p->offset, &check_data) != PASSED) {
                    cterr('f', 0, "%s: Failed to read Test Card FPGA Reg %#x",
                          __FUNCTION__, reg_p->offset);
                    return (FAILED);
                }

                /* Data Comparation */
                if (check_data != test_data) {
                    cterr('f', 0, "%s Reg. Ripple zero test FAILED, "
                                  "read back = 0x%04x and expected = 0x%04x.",
                                  reg_p->name, check_data, test_data);
                    return (FAILED);
                }
            }   /* End of Ripple 0 Test */

            /*
             * Pattern Test
             */
           test_data = (uint16_t)FPGA_REG_TEST_PATTERN;

           for (test_round = 0; test_round < 2; test_round++) {
                /* build mask of size for pattern */
                for (test_ctr = 0; test_ctr < (sizeof(tc_fpga_i2c_if.size) * 8);
                     test_ctr++) {
                    test_mask |= (1 << test_ctr);
                }
                test_data &= test_mask;
                if (!test_data) {
                    continue;
                }

                test_data &= reg_p->mask;

                /* Write Test Data in */
                if (tc_fpga_reg_wr(reg_p->offset, test_data) != PASSED) {
                    cterr('f', 0, "%s: Failed to wrote 0x%04X "
                                  "to Test Card FPGA Reg %#x.",
                                  __FUNCTION__, test_data, reg_p->offset);
                }

                /* Read the register value back for double check */
                if (tc_fpga_reg_rd(reg_p->offset, &check_data) != PASSED) {
                    cterr('f', 0, "%s: Failed to read Test Card FPGA Reg %#x",
                          __FUNCTION__, reg_p->offset);
                    return (FAILED);
                }

                /* Data Comparation */
                if (check_data != test_data) {
                    cterr('f', 0, "%s Reg. Ripple zero test FAILED, "
                                  "read back = 0x%04x and expected = 0x%04x.",
                                  reg_p->name, check_data, test_data);
                    return (FAILED);
                }

                /* complemrent test pattern */
                test_data = (uint16_t)(~FPGA_REG_TEST_PATTERN);

           }   /* End of Pattern Test */

           /* Restore the value before test */
           if (tc_fpga_reg_wr(reg_p->offset, ori_val) != PASSED) {
               cterr('f', 0, "%s: Failed to wrote 0x%04X "
                             "to Test Card FPGA Reg %#x.",
                             __FUNCTION__, ori_val, reg_p->offset);
           }
        }
    }

    return (PASSED);
}


/*******************************************************************************
 *
 * Function   :	tc_read_fpga_reg_wrap
 * Description:	Utility to read TestCard FPGA register.
 * Inputs     :	None
 * Outputs    : None
 *
 *******************************************************************************
 */
int tc_read_fpga_reg_wrap (void)
{
    uint32_t   choice = 0, end_num = 0, start_num, total_reg_num = 0, ctr = 0;
    uint16_t   reg_val = 0, tc_fpga_rev = 0;
    reg_info_t *reg_table_p = 0, *chosen_reg_p = 0;

    if (tc_fpga_reg_rd(FPGA_ID_REG_OFFSET, &tc_fpga_rev) != PASSED) {
        cterr('f', 0, "%s: Failed to read FPGA reigster 0x%01X back.\n",
                      __FUNCTION__, FPGA_ID_REG_OFFSET);
        return (FAILED);
    }

    if ((tc_fpga_rev & TC_FPGA_REV_MSK) < TC_FPGA_V105) {
        total_reg_num = (sizeof(tc_fpga_reg_tbl_v104) / sizeof(reg_info_t));
        reg_table_p = &tc_fpga_reg_tbl_v104[0];
    } else if ((tc_fpga_rev & TC_FPGA_REV_MSK) < TC_FPGA_V107) {
        total_reg_num = (sizeof(tc_fpga_reg_tbl_v105) / sizeof(reg_info_t));
        reg_table_p = &tc_fpga_reg_tbl_v105[0];
    } else {
        total_reg_num = (sizeof(tc_fpga_reg_tbl_v107) / sizeof(reg_info_t));
        reg_table_p = &tc_fpga_reg_tbl_v107[0];
    }

    printf("\nAll Readable Reg. list:\n");
    for (ctr = 0, start_num = total_reg_num, end_num = 0;
         ctr < total_reg_num;
         ctr++, reg_table_p++) {

        if (!strcmp(reg_table_p->name, "End")) {
            break;
        }

        if (!(reg_table_p->type & WRITE_ONLY)) {
            if (ctr <= start_num) {
                start_num = ctr;
            }

            if (ctr > end_num) {
                end_num = ctr;
            }

            /* Readable */
            printf("[%d] %-23s (off. = 0x%02X)\n",
                   ctr, reg_table_p->name, reg_table_p->offset);
        }
    }

    if (end_num < start_num) {
        printf("\nSorry !! There's no Read/Writable Reg. here.\n");
        return (PASSED);
    }

    choice = getdec_answer("Enter the number of Reg. you want to read:",
                           start_num, start_num, end_num);

    if ((tc_fpga_rev & TC_FPGA_REV_MSK) < TC_FPGA_V105) {
        chosen_reg_p = &tc_fpga_reg_tbl_v104[choice];
    } else if ((tc_fpga_rev & TC_FPGA_REV_MSK) < TC_FPGA_V107) {
        chosen_reg_p = &tc_fpga_reg_tbl_v105[choice];
    } else {
        chosen_reg_p = &tc_fpga_reg_tbl_v107[choice];
    }

    if (tc_fpga_reg_rd(chosen_reg_p->offset, &reg_val) != PASSED) {
        cterr('f', 0, "%s: Failed to read Test Card FPGA Reg %#x",
              __FUNCTION__, chosen_reg_p->offset);
        return (FAILED);
    }

    printf("\nThe value of TestCard FPGA %s(off. 0x%02X) is 0x%04X.\n",
           chosen_reg_p->name, chosen_reg_p->offset, reg_val);

    return (PASSED);
}


/*******************************************************************************
 *
 * Function   :	fpga_dump_all_reg
 * Description:	Utility to dump all TestCard FPGA registers.
 * Inputs     :	None
 * Outputs    : None
 *
 *******************************************************************************
 */
static int fpga_dump_all_reg (void)
{
    int result = FAILED;
    uint16_t buffer = 0, tc_fpga_rev = 0;
    uint32_t offset;
    reg_info_t *reg_ptr;

    if (tc_fpga_reg_rd(FPGA_ID_REG_OFFSET, &tc_fpga_rev) != PASSED) {
        cterr('f', 0, "%s: Failed to read FPGA reigster 0x%01X back.\n",
                      __FUNCTION__, FPGA_ID_REG_OFFSET);
        return (FAILED);
    }

    if ((tc_fpga_rev & TC_FPGA_REV_MSK) < TC_FPGA_V105) {
        reg_ptr = &tc_fpga_reg_tbl_v104[0];
    } else if ((tc_fpga_rev & TC_FPGA_REV_MSK) < TC_FPGA_V107) {
        reg_ptr = &tc_fpga_reg_tbl_v105[0];
    } else {
        reg_ptr = &tc_fpga_reg_tbl_v107[0];
    }

    printf("\n");

    while (reg_ptr->offset != 0xFFFFFFFF) {
        /* Print all registers */
        offset = (reg_ptr->offset);
        /* the byteswap is activiated in env_read. */
        if ((result = tc_fpga_reg_rd(offset, &buffer)) != RC_I2C_OP_OK) {
            cterr('f', 0, "%s: Unable to read Regiser %#x.",
                  __FUNCTION__, offset);
            return (FAILED);
        }
        printf("%-23s (off. = 0x%02X): 0x%04x\n", reg_ptr->name,
               reg_ptr->offset, buffer);
        reg_ptr++;
    }
    return (result);
}


/*******************************************************************************
 *
 * Function   :	tc_fpga_reg_rd
 * Description:	Utility to read TestCard FPGA register.
 * Inputs     :	None
 * Outputs    : None
 *
 *******************************************************************************
 */
int tc_fpga_reg_rd (uint32_t offset, uint16_t *data)
{
    int result = FAILED;

    /* Setup I2C API parameter struct */
    /* 1. To get TestCard common I2C structure */
    get_tc_i2c_struct(&tc_fpga_i2c_if);

    /* 2. To set TestCard FPGA read specific parameters */
    tc_fpga_i2c_if.i2c_dev = TESTCARD_FPGA_I2C_ADDR;
    tc_fpga_i2c_if.buf = (char *)data;
    tc_fpga_i2c_if.offset = offset;
    tc_fpga_i2c_if.size = sizeof(uint16_t);

    result = n2g_i2c_read(&tc_fpga_i2c_if);
    if (result != RC_I2C_OP_OK) {
        /* Unable to read data */
        cterr('f', 0, "%s: Unable to read. rc = 0x%08x", __FUNCTION__, result);
        return (FAILED);
    }

    *data = DSWAP2(*data);

    return (PASSED);
}


/*******************************************************************************
 *
 * Function   :	tc_alter_fpga_reg_wrap
 * Description:	Utility to alter TestCard FPGA register.
 * Inputs     :	None
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
int tc_alter_fpga_reg_wrap (void)
{
    int        rc = FAILED;
    uint32_t   ctr, offset, reg_num = 0, start_num, end_num, max_num = 0;
    reg_info_t *reg_table_p;
    uint16_t   tmp_mask = 0, data = 0, write_in = 0, tc_fpga_rev = 0;

    if (tc_fpga_reg_rd(FPGA_ID_REG_OFFSET, &tc_fpga_rev) != PASSED) {
        cterr('f', 0, "%s: Failed to read FPGA reigster 0x%01X back.\n",
                      __FUNCTION__, FPGA_ID_REG_OFFSET);
        return (FAILED);
    }

    if ((tc_fpga_rev & TC_FPGA_REV_MSK) < TC_FPGA_V105) {
        max_num = (sizeof(tc_fpga_reg_tbl_v104) / sizeof(reg_info_t));
        reg_table_p = &tc_fpga_reg_tbl_v104[0];
    } else {
        max_num = (sizeof(tc_fpga_reg_tbl_v105) / sizeof(reg_info_t));
        reg_table_p = &tc_fpga_reg_tbl_v105[0];
    }

    printf("\nAll Read/Writable Reg. list:\n");
    for (ctr = 0, start_num = max_num, end_num = 0;
         ctr < max_num;
         ctr++, reg_table_p++) {
        if (!(reg_table_p->type & READ_ONLY)) {
            if (ctr <= start_num) {
                start_num = ctr;
            }

            if (ctr > end_num) {
                end_num = ctr;
            }

            /* Read writeable */
            printf("[%2d] %-23s (off. = 0x%02X)\n",
                   ctr, reg_table_p->name, reg_table_p->offset);
        }
    }

    if (end_num < start_num) {
        printf("\nSorry !! There's no Read/Writable Reg. here.\n");
        return (PASSED);
    }

    reg_num = getdec_answer("Enter the register number:",
                            start_num, start_num, end_num);

    /* Got the expected reg. num, check if it writable again */
    if ((tc_fpga_rev & TC_FPGA_REV_MSK) < TC_FPGA_V105) {
        reg_table_p = &tc_fpga_reg_tbl_v104[reg_num];
    } else {
        reg_table_p = &tc_fpga_reg_tbl_v105[reg_num];
    }

    if (reg_table_p->type & READ_ONLY) {
        /* read only */
        printf("Sorry !!! %s(off = 0x%02X) is a Read only Register.\n",
               reg_table_p->name, reg_table_p->offset);
        return (FAILED);
    }

    offset = reg_table_p->offset;
    tmp_mask = reg_table_p->mask; /* get the mask for user to alter reg. */

    if ((rc = tc_fpga_reg_rd(offset, &data)) != PASSED) {
        cterr('f', 0, "%s: Failed to read Test Card FPGA Reg %#x",
              __FUNCTION__, offset);
        return (rc);
    }

    data = gethex_answer("Enter the 16-bit data:", data, 0, 0xFFFF);
    write_in = (data & tmp_mask);
    printf("Mask of %s Reg. (off. = 0x%02X) = 0x%04X,\n",
           reg_table_p->name, reg_table_p->offset, tmp_mask);
    printf("so the write-in data = 0x%04X.\n", write_in);

    if ((rc = tc_fpga_reg_wr(offset, write_in)) != PASSED) {
        cterr('f', 0, "%s: Failed to wrote 0x%04X to Test Card FPGA Reg %#x.",
              __FUNCTION__, write_in, offset);
    }

    return (rc);
}


/*******************************************************************************
 *
 * Function   :	tc_fpga_reg_wr
 * Description:	Utility to write data into specific TestCard FPGA register.
 * Inputs     :	None
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
int tc_fpga_reg_wr (uint32_t offset, uint16_t data)
{
    int      result = FAILED;
    uint32_t data_in = 0;

    data_in = ((offset << 24) + (data << 8));
    data_in = DSWAP4(data_in);

    /* Setup I2C API parameter struct */
    /* 1. To get TestCard common I2C structure */
    get_tc_i2c_struct(&tc_fpga_i2c_if);

    /* 2. To set TestCard FPGA write specific parameters */
    tc_fpga_i2c_if.i2c_dev = TESTCARD_FPGA_I2C_ADDR;
    tc_fpga_i2c_if.buf = (char *)&data_in;
    tc_fpga_i2c_if.offset = -1;
    tc_fpga_i2c_if.size = 3;

    result = n2g_i2c_write(&tc_fpga_i2c_if);
    if (result != RC_I2C_OP_OK) {
        printf("%s: Failed to write to Reg(%#x).", __FUNCTION__, offset);
        return (FAILED);
    }

    return (PASSED);
}


/*******************************************************************************
 *
 * Function   :	tc_set_fpga_reg
 * Description:	Utility to set TestCard FPGA specific register.
 * Inputs     :	reg_off  - offset of the chosen FPGA register
 *              reg_data - data that user want to set/clean
 *              opt      - ENABLE/DISABLE
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
int tc_set_fpga_reg (uint32_t reg_off, uint16_t reg_data, uint8_t opt)
{
    uint16_t reg_val = 0;
    int      ret = FAILED;

    /* 1. Read the data of chosen register back. */
    if (tc_fpga_reg_rd(reg_off, &reg_val) != PASSED) {
        printf("%s: Failed to read FPGA register(off. 0x%02X).\n",
               __FUNCTION__, reg_off);
        return (FAILED);
    }

    /* 2. Prepare the write in data. */
    if (opt == ENABLE) {
        reg_val |= reg_data;
    } else {
        reg_val &= (uint16_t)(~reg_data);
    }

    /* 3. Write data into specific register. */
    if (tc_fpga_reg_wr(reg_off, reg_val) != PASSED) {
        printf("%s: Failed to write FPGA register(off. 0x%02X).\n",
               __FUNCTION__, reg_off);
        return (FAILED);
    }

    /* 4. Double confirm if option is correct. */
    reg_val = 0;

    if (tc_fpga_reg_rd(reg_off, &reg_val) != PASSED) {
        printf("%s: Failed to read FPGA register(off. 0x%02X).\n",
               __FUNCTION__, reg_off);
        return (FAILED);
    }

    if (opt == ENABLE) {
        if ((reg_val & reg_data) == reg_data) {
            ret = PASSED;
        }
    } else {
        if ((reg_val & reg_data) != reg_data) {
            ret = PASSED;
        }
    }

    return (ret);
}


/*******************************************************************************
 *
 * Function   :	tc_get_reset_dev_name
 * Description:	Function to get related TestCard device name from reset bit.
 * Inputs     :	dev      - related device reset bit
 *              dev_name - buffer to put related device name
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
static int tc_get_reset_dev_name (uint16_t dev, char *dev_name)
{
    switch (dev) {
    case GE_RESET:
        sprintf(dev_name, "GE PHY");
        break;
    case XAUI_RESET:
        sprintf(dev_name, "XAUI PHY");
        break;
    case PCIE_RESET:
        sprintf(dev_name, "PCIe slot");
        break;
    case SMI0_RESET:
        sprintf(dev_name, "SMI0");
        break;
    case SMI1_RESET:
        sprintf(dev_name, "SMI1");
        break;
    case PCIE_REDRV_RESET:
        sprintf(dev_name, "PCIe reDriver");
        break;
    case EXT_MOD_RESET:
        sprintf(dev_name, "External Module");
        break;
    default:
        printf("%s:%d Invalid Device Type: %d.\n",
               __FUNCTION__, __LINE__, dev);
        return (FAILED);
    }

    return (PASSED);
}


/*******************************************************************************
 *
 * Function   :	tc_fpga_reset_device
 * Description:	Function to put specific device into reset state by accessing
 *              TestCard FPGA Individual Device Reset register (0x02h).
 * Inputs     :	dev - device that want to put into Reset state
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
int tc_fpga_reset_device (uint16_t dev)
{
    char dev_name[TC_BUF_SIZE];

    if (tc_get_reset_dev_name(dev, &dev_name[0]) != PASSED) {
        return (FAILED);
    }

    if (tc_set_fpga_reg(DEV_RESET_REG_OFFSET, dev, ENABLE) != PASSED) {
        printf("%s: Failed to put %s in Reset state.\n",
               __FUNCTION__, dev_name);
        return (FAILED);
    }

    msleep(100);

    return (PASSED);
}


/*******************************************************************************
 *
 * Function   :	tc_fpga_unreset_device
 * Description:	Function to release specific device from reset state by accessing
 *              TestCard FPGA Individual Device Reset register (0x02h).
 * Inputs     :	dev - device that want to put into Reset state
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
int tc_fpga_unreset_device (uint16_t dev)
{
    char dev_name[TC_BUF_SIZE];

    if (tc_get_reset_dev_name(dev, &dev_name[0]) != PASSED) {
        return (FAILED);
    }

    if (tc_set_fpga_reg(DEV_RESET_REG_OFFSET, dev, DISABLE) != PASSED) {
        printf("%s: Failed to release %s from Reset state.\n",
               __FUNCTION__, dev_name);
        return (FAILED);
    }

    sleep(1);

    return (PASSED);
}


/*******************************************************************************
 *
 * Function   :	tc_fpga_set_sync_lpbk
 * Description:	Function to set related bits of TestCard FPGA
 *              SyncE Loopback Control Register(off: 0x1Ch) for testing the
 *              sync signals from host to module.
 * Inputs     :	None
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
int tc_fpga_set_sync_lpbk (boolean opt)
{
    uint16_t reg_val = 0;

    /* 1. Read value of SyncE Loopback Control register(0x1Ch) */
    if (tc_fpga_reg_rd(SYNC_LPBK_REG_OFFSET, &reg_val) != PASSED) {
        printf("%s: Failed to read FPGA reigster 0x%02X.\n",
               __FUNCTION__, SYNC_LPBK_REG_OFFSET);
        return (FAILED);
    }

    /* 2. For TestCard, there are totally two bits needed to set
     *    2.1 Set SYNC_TRIG_LPBK bit:
     *        To let host ngio_sync_trig_out to host ngio_sync_trig_in loopback.
     *    2.2 Set SYNC_LPBK bit:
     *        To let host ngio_sync_out to host ngio_sync_in loopback.
     */
    if (opt == ENABLE) {
        reg_val |= (SYNC_TRIG_LPBK | SYNC_LPBK);
    } else {
        reg_val &= (~(SYNC_TRIG_LPBK | SYNC_LPBK));
    }

    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("%s: The write-in data: 0x%02X.\n", __FUNCTION__, reg_val);
    }

    if (tc_fpga_reg_wr(SYNC_LPBK_REG_OFFSET, reg_val) != PASSED) {
        printf("%s: Failed to wrote 0x%04X to Test Card FPGA Reg 0x%02X.\n",
               __FUNCTION__, reg_val, SYNC_LPBK_REG_OFFSET);
        return (FAILED);
    }

    /* 3. Read the value of SyncE Loopback Control register(0x1Ch)
     *    back again to confirm the changes.
     */
    reg_val = 0;

    if (tc_fpga_reg_rd(SYNC_LPBK_REG_OFFSET, &reg_val) != PASSED) {
        printf("%s: Failed to read FPGA reigster 0x%02X back.\n",
               __FUNCTION__, SYNC_LPBK_REG_OFFSET);
        return (FAILED);
    }

    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("%s: The SyncE Loopback Control reg: 0x%02X.\n",
               __FUNCTION__, reg_val);
    }

    if (opt == ENABLE) {
        if (!((reg_val & SYNC_TRIG_LPBK) && (reg_val & SYNC_LPBK))) {
            printf("%s: Failed to set TestCard FPGA SyncE Loopback Control"
                   " reg(0x%02X).\n",
                   __FUNCTION__, SYNC_LPBK_REG_OFFSET);
            return (FAILED);
        }
    } else {
        if ((reg_val & SYNC_TRIG_LPBK) || (reg_val & SYNC_LPBK)) {
            printf("%s: Failed to unset TestCard FPGA SyncE Loopback Control"
                   " reg(0x%02X).\n",
                   __FUNCTION__, SYNC_LPBK_REG_OFFSET);
            return (FAILED);
        }
    }

    return (PASSED);
}


/*******************************************************************************
 *
 * Function   :	set_tc_sig_test_parameters
 * Description:	Function to set TestCard Sync Signal test related parameters.
 * Inputs     :	sync_reg_base_addr - Base addr of related NGIO_SYNC_Control reg.
 *              sync_in_stat_mask  - SYNC_IN_STATUS mask
 *              sync_trig_in_stat_mask  - SYNC_TRIG_IN_STATUS mask
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
int set_tc_sig_test_parameters (uint32_t *sync_reg_base_addr, 
                                uint32_t *sync_in_stat_mask,
                                uint32_t *sync_trig_in_stat_mask)
{
    if (tc_ngio_p->mod_type == DAUGHTER_CARD) {
        switch (tc_ngio_p->pc->slot) {
        case NGSM1_SLOT:
            *sync_reg_base_addr = (NET_CLK_PTP_CONF_REG_OFF + 0x10);
            *sync_in_stat_mask = NGSM1_SYNC_IN_STAT;
            *sync_trig_in_stat_mask = NGSM1_SYNC_TRIG_IN_STAT;
            break;
        case NGSM2_SLOT:
            *sync_reg_base_addr = (NET_CLK_PTP_CONF_REG_OFF + 0x14);
            *sync_in_stat_mask = NGSM2_SYNC_IN_STAT;
            *sync_trig_in_stat_mask = NGSM2_SYNC_TRIG_IN_STAT;
            break;
        default:
            printf("%s: Invalid slot: %d.\n",
                   __FUNCTION__, tc_ngio_p->pc->slot);
            return (FAILED);
        }
        return(PASSED);
    }

    if (testcard_if_p->type == TC_NGSM) {
        switch (testcard_if_p->slot) {
        case NGSM1_SLOT:
            *sync_reg_base_addr = (NET_CLK_PTP_CONF_REG_OFF + 0x10);
            *sync_in_stat_mask = NGSM1_SYNC_IN_STAT;
            *sync_trig_in_stat_mask = NGSM1_SYNC_TRIG_IN_STAT;
            break;
        case NGSM2_SLOT:
            *sync_reg_base_addr = (NET_CLK_PTP_CONF_REG_OFF + 0x14);
            *sync_in_stat_mask = NGSM2_SYNC_IN_STAT;
            *sync_trig_in_stat_mask = NGSM2_SYNC_TRIG_IN_STAT;
            break;
        case NGSM3_SLOT:
            *sync_reg_base_addr = (NET_CLK_PTP_CONF_REG_OFF + 0x18);
            *sync_in_stat_mask = NGSM3_SYNC_IN_STAT;
            *sync_trig_in_stat_mask = NGSM3_SYNC_TRIG_IN_STAT;
            break;
        default:
            printf("%s: Invalid slot: %d.\n",
                   __FUNCTION__, testcard_if_p->slot);
            return (FAILED);
        }
    } else if (testcard_if_p->type == TC_NGWIC) {
        switch (testcard_if_p->slot) {
        case NGWIC1_SLOT:
            *sync_reg_base_addr = (NET_CLK_PTP_CONF_REG_OFF + 0x20);
            *sync_in_stat_mask = NGWIC1_SYNC_IN_STAT;
            *sync_trig_in_stat_mask = NGWIC1_SYNC_TRIG_IN_STAT;
            break;
        case NGWIC2_SLOT:
            *sync_reg_base_addr = (NET_CLK_PTP_CONF_REG_OFF + 0x24);
            *sync_in_stat_mask = NGWIC2_SYNC_IN_STAT;
            *sync_trig_in_stat_mask = NGWIC2_SYNC_TRIG_IN_STAT;
            break;
        case NGWIC3_SLOT:
            *sync_reg_base_addr = (NET_CLK_PTP_CONF_REG_OFF + 0x28);
            *sync_in_stat_mask = NGWIC3_SYNC_IN_STAT;
            *sync_trig_in_stat_mask = NGWIC3_SYNC_TRIG_IN_STAT;
            break;
        default:
            printf("%s: Invalid slot: %d.\n",
                   __FUNCTION__, testcard_if_p->slot);
            return (FAILED);
        }
    } else {
        printf("%s: Invalid interface type of TestCard (%#x).",
               __FUNCTION__, testcard_if_p->type);
        return (FAILED);
    }

    return (PASSED);
}


/*******************************************************************************
 *
 * Function   :	tc_sync_sig_test
 * Description:	Function to set related bits of TestCard FPGA
 *              SyncE Loopback Control Register(off: 0x1Ch) for testing the
 *              sync signals from host to module.
 * Inputs     :	None
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
int tc_sync_sig_test (void)
{
    int        ret_val = PASSED;
    uint16_t   tc_fpga_rev = 0;
    uint32_t   host_fpga_rev = 0, sync_trig_stat_mask = 0, sync_stat_mask = 0;
    uint32_t   sync_reg_off = 0, check_reg_val = 0, thres = 0;
    uint32_t   deb_rev = 0, minor_rev = 0, major_rev = 0;
    volatile uint32_t *ngio_sync_ctrl;
    volatile uint32_t *sync_debug;
    sys_lvl_t  *fpga  = (sys_lvl_t*)dash_fpga;

    testname("TestCard Sync Signal loopback");
    prpass(testpass, "%s%d ", testcard_if_p->type_name, testcard_if_p->slot);

    /* 1. Based on HW's info, to support this test:
     *    the TestCard FPGA has to be 1.05 or higher;
     *    and Host(Overlord) FPGA has to be 2.4.0 or higher.
     *    So for backward compatibility, we check FPGA revision first.
     */
    /* 1.1 Check TestCard FPGA revision */
    if (tc_fpga_reg_rd(FPGA_ID_REG_OFFSET, &tc_fpga_rev) != PASSED) {
        cterr('f', 0, "%s: Failed to read FPGA reigster 0x%01X back.\n",
                      __FUNCTION__, FPGA_ID_REG_OFFSET);
        return (FAILED);
    }

    if (tc_fpga_rev_checker(tc_fpga_rev, "TestCard Sync Signal",
                            TC_SIG_TEST_FPGA_THRES) != TRUE) {
        return (PASSED);
    }

    /* 1.2 Check Host(Overlord) FPGA revision */
    host_fpga_rev = (uint32_t)fpga->ver;
    if (is_overlord() || is_juno()) {
        thres = TC_SIG_TEST_HOST_FPGA_THRES_O2;
    } else {
        thres = TC_SIG_TEST_HOST_FPGA_THRES_USD;
    }
     
    if (is_curie_1ru() || is_curie_2ru()) {
        /* curie 1ru/2ru is already support sync signal test, bypass FPGA ver chk*/
    } else if ((host_fpga_rev & HOST_FPGA_REV_MSK) < thres) {
        /* based on HFS FPGA reg 0x84 to know the offset */
        deb_rev = host_fpga_rev & 0xFF;
        minor_rev = (host_fpga_rev & 0xFF00) >> 8;
        major_rev = (host_fpga_rev & 0x7F0000) >> 16;
        printf("SKIP because Host Master FPGA Revision register: 0x%04X.\n",
               host_fpga_rev);
        printf("FPGA Rev: Major 0x%02X, minor 0x%02X, debug 0x%02X\n",
               major_rev, minor_rev, deb_rev);

        deb_rev = thres & 0xFF;
        minor_rev = (thres & 0xFF00) >> 8;
        major_rev = (thres & 0x7F0000) >> 16;
        printf("(To support this TestCard Sync Signal test, you need to upgrade "
               "Host FPGA revision higher.)\n");
        printf("Higher FPGA Rev: Major 0x%02X, minor 0x%02X, debug 0x%02X\n",
               major_rev, minor_rev, deb_rev);
        return (PASSED);
    } else { 
        printf("%s : should not be here !! \n", __FUNCTION__); 
        /* left for future use */
    } 

    /* 2. Enable Sync signal loopback by setting TestCard FPGA
     *    SyncE Loopback Control Register(off: 0x1Ch).
     */
    if (tc_fpga_set_sync_lpbk(ENABLE) != PASSED) {
        cterr('f', 0, "\n%s: Failed to set TestCard Sync signals into loopback.",
                       __FUNCTION__);
        return (FAILED);
    }

    /* 3. Do Sync signals loopback test from Host side FPGA. */
    /* Take Overlord NGSM1 for example, we need to do:
     * 3.1 Set Host side FPGA register 0x0001_0110 to 0x01180118.
     * 3.2 Set Host side FPGA register 0x0001_0110 to 0x01FE01FE.
     * 3.3 Check Host side FPGA SYNC_DEBUG register 0001_01C0,
     *     bits 16 and 17 are set to 0.
     * 3.4 Set Host side FPGA register 0x0001_0110 to 0x01FF01FF.
     * 3.5 Check Host side FPGA SYNC_DEBUG register 0001_01C0,
     *     bits 16 and 17 are set to 1.
     */
    if (set_tc_sig_test_parameters(&sync_reg_off, &sync_stat_mask,
                                   &sync_trig_stat_mask) != PASSED) {
        cterr('f', 0, "\n%s: Failed to set TestCard Sync Signal test"
                      " related parameters.", __FUNCTION__);
        return (FAILED);
    }
    ngio_sync_ctrl = (volatile uint32_t *)(dash_fpga + sync_reg_off);
    sync_debug = (volatile uint32_t *)(dash_fpga + SYNC_DEBUG_REG_OFF);

    *ngio_sync_ctrl = TC_SYNC_SIG_TEST_PATTERN1;
    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("%s:%d Host(Overlord) FPGA SYNC_CONTROL register: 0x%04X.\n",
               __FUNCTION__, __LINE__, *ngio_sync_ctrl);
    }

    *ngio_sync_ctrl = TC_SYNC_SIG_TEST_PATTERN2;
    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("%s:%d Host(Overlord) FPGA SYNC_CONTROL register: 0x%04X.\n",
               __FUNCTION__, __LINE__, *ngio_sync_ctrl);
    }

    /* Switzer carrier FPGA provides 20ns delay during this loopback test. */
    /* So this test random fault in curie2ru SM2 slot with switzer carrier. */
    /* We add 1us delay as a workaround. */
    usleep(1);
    check_reg_val = *sync_debug;
    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("%s:%d Host(Overlord) FPGA SYNC_DEBUG register: 0x%04X.\n",
               __FUNCTION__, __LINE__, check_reg_val);
    }

    if ((check_reg_val & sync_stat_mask) || 
        (check_reg_val & sync_trig_stat_mask)) {
        cterr('f', 0, "\n%s:%d The read back value is not expected: 0x%04X.\n",
                      __FUNCTION__, __LINE__, check_reg_val);
        ret_val = FAILED;
    }

    *ngio_sync_ctrl = TC_SYNC_SIG_TEST_PATTERN3;
    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("%s:%d Host(Overlord) FPGA SYNC_CONTROL register: 0x%04X.\n",
               __FUNCTION__, __LINE__, *ngio_sync_ctrl);
    }

    /* Switzer carrier FPGA provides 20ns delay during this loopback test. */
    /* So this test random fault in curie2ru SM2 slot with switzer carrier. */
    /* We add 1us delay as a workaround. */
    usleep(1);
    check_reg_val = *sync_debug;
    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("%s:%d Host(Overlord) FPGA SYNC_DEBUG register: 0x%04X.\n",
               __FUNCTION__, __LINE__, check_reg_val);
    }

    if ((!(check_reg_val & sync_stat_mask)) ||
        (!(check_reg_val & sync_trig_stat_mask))) {
        cterr('f', 0, "\n%s:%d The read back value is not expected: 0x%04X.\n",
                      __FUNCTION__, __LINE__, check_reg_val);
        ret_val = FAILED;
    }

    /* 4. Disable Sync signal loopback after test by setting TestCard FPGA
     *    SyncE Loopback Control Register(off: 0x1Ch).
     */
    if (tc_fpga_set_sync_lpbk(DISABLE) != PASSED) {
        cterr('f', 0, "\n%s: Failed to release TestCard Sync signals"
                      " from loopback.", __FUNCTION__);
        return (FAILED);
    }

    return (ret_val);
}


/*******************************************************************************
 *
 * Function   :	tc_fpga_rev_checker
 * Description:	Function to check TestCard FPGA revision. This is because
 *              some TestCard FPGA related test need specific FPGA revision
 *              or higher.
 * Inputs     :	fpga_rev - FPGA revision
 *              test_name - Test name
 *              fpga_rev_threshold - Threshold of the FPGA revision
 * Outputs    : TRUE/FALSE
 *
 *******************************************************************************
 */
int tc_fpga_rev_checker (uint16_t fpga_rev, char *test_name,
                         uint16_t rev_threshold)
{
    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("%s: TestCard FPGA ID register(0x%01X): 0x%02X.\n",
               __FUNCTION__, FPGA_ID_REG_OFFSET, fpga_rev);
        printf("FPGA Rev. Threshold is 0x%02X.\n", rev_threshold);
    }

    if ((fpga_rev & TC_FPGA_REV_MSK) < rev_threshold) {
        printf("SKIP because FPGA rev. of your TestCard is %d.%2d\n",
               ((fpga_rev & FPGA_MAJ_REV_MSK) >> FPGA_MAJ_REV_SHIFT),
               (fpga_rev & FPGA_MIN_REG_MSK));
        printf("(To support this %s test, you need to upgrade "
               "TestCard FPGA revision to %d.%2d or higher.)\n",
               test_name,
               ((rev_threshold & FPGA_MAJ_REV_MSK) >> FPGA_MAJ_REV_SHIFT),
               (rev_threshold & FPGA_MIN_REG_MSK));

        return (FALSE);
    }
    return (TRUE);
}

/*******************************************************************************
 *
 * Function   : tc_pll_locked_test
 * Description: Function to check if the PLL has locked onto the 100MHz PCIe
 *              reference clock from the host.
 *              This test is by checking bit 0, PLL Locked bit, of TestCard
 *              FPGA PLL Lock Reg(0x20h).
 * Inputs     : None
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
int tc_pll_locked_test (void)
{
    uchar    part_num[4];
    uint8_t  tc_hw_rev = 0, hw_rev_threshold = 0xFF;
    uint16_t tc_fpga_rev = 0, tc_pll_lock_reg = 0;

    testname("TestCard PLL Lock");
    prpass(testpass, "%s%d ", testcard_if_p->type_name, testcard_if_p->slot);

    /* Based on HW's info, to support PLL test, we need:
     * 1. NGSM HW rev. is 3 or higher; NGWIC HW rev. is 4 or higher.
     * 2. TestCard FPGA rev. is 1.05 or higher.
     */
    /* 1.1 Check TestCard HW revision */
    memset(part_num, 0, sizeof(part_num));

    if (slot_get_73_part_num(tc_ngio_p->cookie, part_num) != PASSED) {
        cterr('f', 0, "%s: Failed to get cookie 73 part number back.\n",
                      __FUNCTION__);
        return (FAILED);
    }

    tc_hw_rev = part_num[3];

    if (testcard_if_p->type == TC_NGSM) {
        hw_rev_threshold = TC_NGSM_PLL_LOCK_THRES;
    } else {
        hw_rev_threshold = TC_NGWIC_PLL_LOCK_THRES;
    }

    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("\n%s: %s%d TestCard HW rev. is %d.\n",
               __FUNCTION__, testcard_if_p->type_name,
               testcard_if_p->slot, tc_hw_rev);
    }

    if (tc_hw_rev < hw_rev_threshold) {
        printf("SKIP because TestCard in %s%d is HW rev. %d.\n",
               testcard_if_p->type_name, testcard_if_p->slot, tc_hw_rev);
        printf("(To support this test, please use %s TestCard"
               " rev. %d or higher.)\n",
               testcard_if_p->type_name, hw_rev_threshold);
        return (PASSED);
    }

    /* 1.2 Check TestCard FPGA revision */
    if (tc_fpga_reg_rd(FPGA_ID_REG_OFFSET, &tc_fpga_rev) != PASSED) {
        cterr('f', 0, "%s: Failed to read FPGA reigster 0x%01X back.\n",
                      __FUNCTION__, FPGA_ID_REG_OFFSET);
        return (FAILED);
    }

    if (tc_fpga_rev_checker(tc_fpga_rev, "TestCard PLL Lock",
                            TC_PLL_TEST_FPGA_THRES) != TRUE) {
        return (PASSED);
    }

    /* PLL test */
    if (tc_fpga_reg_rd(FPGA_ID_REG_OFFSET, &tc_fpga_rev) != PASSED) {
        cterr('f', 0, "%s: Failed to read FPGA reigster 0x%02X back.\n",
                      __FUNCTION__, FPGA_ID_REG_OFFSET);
        return (FAILED);
    }

    if (tc_fpga_rev_checker(tc_fpga_rev, "TestCard PLL Locked",
                            TC_PLL_TEST_FPGA_THRES) != TRUE) {
        return (PASSED);
    }

    if (tc_fpga_reg_rd(PLL_LOCK_REG_OFFSET, &tc_pll_lock_reg) != PASSED) {
        cterr('f', 0, "%s: Failed to read FPGA reigster 0x%02X back.\n",
                      __FUNCTION__, PLL_LOCK_REG_OFFSET);
        return (FAILED);
    }

    if (!(tc_pll_lock_reg & PLL_LOCK_PCIE_REF)) {
        cterr('f', 0, "%s: PLL hasn't locked onto 100MHz PCIe "
                      "reference clock from the host.", __FUNCTION__);
        return (FAILED);
    }

    return (PASSED);
}

/*******************************************************************************
 *
 * Function   : tc_minus_54v_detect_test
 * Description: Function to check if the POE -54V is present
 *              This test is by checking bits 0 & 1 of TestCard
 *              FPGA -54V Present Reg(0x22h).
 * Inputs     : None
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
int tc_minus_54v_detect_test (void)
{
    uint16_t tc_minus_54v_reg = 0, tc_fpga_rev = 0;
    char pid[128];
    struct ngio_intf_t dc, *dc_p;

    dc_p = &dc;

    testname("TestCard POE -54V Detection");
    prpass(testpass, "%s%d ", testcard_if_p->type_name, testcard_if_p->slot);

    /* Check TestCard FPGA revision 
     * to support -54 detection, testcard FPGA rev. should be 1.07 or higher
     */
    if (tc_fpga_reg_rd(FPGA_ID_REG_OFFSET, &tc_fpga_rev) != PASSED) {
        cterr('f', 0, "%s: Failed to read FPGA reigster 0x%01X back.\n",
                      __FUNCTION__, FPGA_ID_REG_OFFSET);
        return (FAILED);
    }

    if (tc_fpga_rev_checker(tc_fpga_rev, "TestCard POE -54V Detection",
                            TC_MINUS_54V_TEST_FPGA_THRES) != TRUE) {
        return (PASSED);
    }

    if (ovld_check_poe_psu_wrap() == TRUE) {
        if (tc_fpga_reg_rd(MINUS_54V_REG_OFFSET, &tc_minus_54v_reg) != PASSED) {
            cterr('f', 0, "%s: Failed to read FPGA reigster 0x%01X back.\n",
                          __FUNCTION__, MINUS_54V_REG_OFFSET);
            return (FAILED);
        }
        printf("-54V present register (0x22) = 0x%x\n", tc_minus_54v_reg);

        /* a. check tc_minus_54v_reg before read cookie 
         * b. not sure why original code use SM_DAUGHTER_CARD 
         * we use is_neptune and is_vg450 to divide old one */
        if (is_neptune() || is_vg450() || is_curie_1ru() || is_curie_2ru()) {
             if ((tc_minus_54v_reg & MINUS_54V_MAIN) || (tc_minus_54v_reg & MINUS_54V_AUX)) {
                 get_cookie_pid(tc_ngio_p->slot, tc_ngio_p->mod_type, tc_ngio_p->cookie, pid);
             } else {
                 cterr('f', 0, "Both -54V main and AUX are not present on testcard ");
                 return (FAILED);
             }
        } else {
             if (get_cookie_pid_wrap(tc_ngio_p->slot, SM_DAUGHTER_CARD, dc_p->cookie, pid) != PASSED) {
                 cterr('f', 0, "%s: Failed to get cookie product ID back.\n",
                          __FUNCTION__);
                 return (FAILED);
             }
        }

        if (!(tc_minus_54v_reg & MINUS_54V_MAIN)) {
            cterr('f', 0, "%s: POE -54V is not present on the test card "
                          "main connector.", __FUNCTION__);
            return (FAILED);
        } else {
            printf("\nMain -54V is detected");
        }

        if (testcard_if_p->is_10gkr == TC_10GKR_SM_ON_SM) {
            return (PASSED); /* SM TC don't need to test POE on AUX portion */
        }

        /* check the NIM test card is Double-wide or Single-wide by its PID
         * PID of SW : NIM-10GKR-TEST-SW
         * PID of DW : NIM-10GKR-TEST-DW     
         * The HEX code of the 15th character of DW PID 'D' is 0x44
         * Only Double Wide test card supports auxiliary connector
         */
        if (pid[15] == 0x44 ) {
            if (!(tc_minus_54v_reg & MINUS_54V_AUX)) {
                cterr('f', 0, "%s: POE -54V is not present on the test card "
                              "auxiliary connector.", __FUNCTION__);
                return (FAILED);
            } else {
                printf("\nAUX -54V is detected");
            }
        }

    } 

    return (PASSED);
}


/*------------------------------------------------------------------
$Log: testcard_fpga.c,v $
Revision 1.11  2020/05/22 02:28:34  qingcwan
Merge switzer-carrier code into main chunk.

Revision 1.10  2020/01/09 01:02:20  jiajliu
Merge Curie 2RU to main trunk

Revision 1.9  2019/08/06 06:56:10  alpeng
merge curie, switzer and nightwatch to trunk

Revision 1.8.2.2  2019/04/03 08:40:44  alpeng
add psu poe check back to curie per HW request.

Revision 1.8.2.1  2018/08/10 08:15:51  alpeng
update eth num for sm1; support sync signal test for curie; skip plx pcie check (wait for rommon); skip ge1 on SM for both tlk10232 and xaui loopback test (wait for rommon); remove NIM slot2 test; fixed pcie lane scan test

Revision 1.8  2018/05/18 09:24:51  alpeng
 Neptune merge to trunk with tag <neptune-branch-0518>

Revision 1.7.30.3  2018/02/05 09:48:26  alpeng
add check link speed before sending packet; skip aux -54v detection check

Revision 1.7.30.2  2018/01/30 09:03:40  alpeng
check regsiter before compare with cookie; fixed diag entry for sm testcard

Revision 1.7.30.1  2017/03/16 08:19:18  alpeng
update SM3 for signal tests

Revision 1.7  2014/09/02 09:46:58  danchung
add -54V present register in test card fpga dump and read utilities for
test card fpga Version 1.07

Revision 1.6  2014/08/29 10:32:08  danchung
support NIM test card POE -54V detection test when plugged in Thule

Revision 1.5  2014/08/05 12:06:19  danchung
Support NIM test card POE -54V detection test

Revision 1.4  2014/07/22 09:40:13  alpeng
support 10g-kr on ge0 and 1g-kx on ge1 for new testcard

Revision 1.3  2013/12/05 02:59:46  alpeng
support lower FPGA version cheching on USD platforms

Revision 1.2  2013/11/26 08:40:37  hroni
fix compiler warning

Revision 1.1  2013/05/09 05:42:38  alpeng
moving overlord common code from x86

Revision 1.9  2013/02/15 10:34:36  palin2
Update the display of the TestCard Diag tests based on code review's comment.

Revision 1.8  2012/12/21 01:27:48  palin2
Update description message of version check.

Revision 1.7  2012/11/28 18:36:31  palin2
Update the description of function TestCard FPGA revision checker.

Revision 1.6  2012/11/21 19:50:08  palin2
1. Add TestCard PLL Lock test support.
2. Update FPGA registers map to v1.05 with backward compatible.

Revision 1.5  2012/11/14 19:15:48  palin2
To support Sync Signals loopback test on TestCards.

Revision 1.4  2012/09/24 17:37:43  palin2
1. Use "Internal loopback test" as default test for TestCard.
2. Unify all tests print out format for TestCard.

Revision 1.3  2012/09/12 08:37:32  palin2
Improve the test time of TestCard FPGA register test.

Revision 1.2  2012/08/22 16:39:55  palin2
Put XAUI into Reset state when exits XAUI related tests to avoid
affecting other interface.

Revision 1.1  2012/08/14 11:30:55  palin2
Removed "ovld_" from TestCard related filename because TestCard is not Overlord's unique.

Revision 1.4  2012/08/08 22:19:41  palin2
1. Move TestCard UART external loopback test to "testcard_uart.c".
2. Add support TestCard UART internal loopback test and related utilities.

Revision 1.3  2012/08/03 08:34:49  palin2
Update TestCard FPGA registers map to rev 1.02.

Revision 1.2  2012/07/31 17:08:20  palin2
Initial check-in for TestCard PCIe tests.

Revision 1.1  2012/07/23 17:33:54  palin2
Initial check-in for Overlord Test Card diag tests.


$Endlog$
*/

