/* $Id: platform_pll.c,v 1.3 2018/05/18 09:24:51 alpeng Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/overlord/platform_pll.c,v $
 *------------------------------------------------------------------
 * Filename   : platform_pll.c
 *
 * Description: Overlord PLL related Diag tests and Uitilities.
 *
 * Copyright (c) 2013-2018 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */
#include <stdio.h>
#include <string.h>
#include "common.h"
#include "defs.h"
#include "error.h"
#include "types.h"
#include "i2c_api.h"
#include "linux_api.h"
#include "menu.h"
#include "nvmonvars.h"
#include "proto.h"
#include "dash_fpga.h"


/******************************************************************************* 
 *                            Function Prototypes                              *
 *******************************************************************************
 */
int build_ovld_pll_menu(int);

static void build_ovld_pll_utils(int);
static int  ovld_pll_tests_wrap(void);


/******************************************************************************* 
 *                                  Externs                                    *
 *******************************************************************************
 */
extern int dash_alt_mem(int);


/*******************************************************************************
 *                              Global Variables                               *
 *******************************************************************************
 */
/* Clock Type of Overlord PLL test */
#define OVLD_PLL_REF1   1   /* Ref1 clock: 25M in, 25M out, 111Hz BandWidth */
#define OVLD_PLL_REF2   2   /* Ref2 clock: 8K in, 25M out, 60Hz BandWidth */

#define OVLD_PLL_TEST_FPGA_THRES   0x020402
#define OVLD_PLL_TEST_HW_THRES     0x4
#define OVLD_PLL_TEST_REF_CLK      0x02180018
#define REF1_25M_25M_111HZ         0x000055F0
#define REF2_8K_25M_60HZ           0x00100730
#define REF1_LOL_SEL               0x021800FE
#define REF2_LOL_SEL               0x02FE0018

#define REF1_PLL_CHECK_MSK         0x00000300
#define REF2_PLL_CHECK_MSK         0x00000500

#define REF1_PLL_CLEAN_BIT         0x00000003
#define REF2_PLL_CLEAN_BIT         0x00000005

#define REF1_PLL_CHECK_MSK2        0x00000303
#define REF2_PLL_CHECK_MSK2        0x00000505

#define OVLD_PLL_MAX_POLLING_TIME  10000 /* (ms) */

/* Buffer to save change time of LOL, and LOS */
#define TIME_BUF_SIZE              2

uint32_t pll_change_time_reset[TIME_BUF_SIZE];
uint32_t pll_change_time_clean[TIME_BUF_SIZE];
uint32_t pll_change_time_loss[TIME_BUF_SIZE];


/*******************************************************************************
 *                                   Menus                                     *
 *******************************************************************************
 */
/*
 * PLL Tests and Utilities Main Menu
 */
static submenu_xtable_t ovld_pll_diag_tbl[] = {
    {"PLL Utilities",               (PFT)build_ovld_pll_utils,   TRUE,
     0,                             (PFT)0,                      0,
     (PFT)build_ovld_pll_utils,     TRUE},
    {"PLL test",                    (PFT)ovld_pll_tests_wrap,    0,
     (MF_CONTINUOUS | MF_DOALL),    (PFT)0,                      0,
     (PFT)0,                        0},
};

#define OVLD_PLL_DIAG_TBL_SIZE (sizeof(ovld_pll_diag_tbl) / sizeof(submenu_xtable_t))

/*
 * Primary & secondary submenu items (filled in from xtable)
 */
static mitem_t ovld_pll_diag_pri_items[OVLD_PLL_DIAG_TBL_SIZE + MAX_BASE_ITEMS];
static mitem_t ovld_pll_diag_sec_items[OVLD_PLL_DIAG_TBL_SIZE + MAX_BASE_ITEMS];

static struct menuinfo ovld_pll_diag = {
    "PLL test SubMenu",        /* title */
    0,                         /* title string added by init_empty_menu */
    (PFT)menu_show_dflags,     /* shows major flags */
    0,                         /* generic prompt */
    0,                         /* size -- bumped by add_menu_item() */
    ovld_pll_diag_pri_items,
};

static struct menuinfo *ovld_pll_diag_p = &ovld_pll_diag;


/*
 * PLL Utilities SubMenu
 */
static submenu_xtable_t pll_utils_tbl[] = {
    {"Overlord FPGA Alter Utility",   (PFT)dash_alt_mem,   TRUE,
     0, (PFT)0, 0, (PFT)0, 0},
};

#define PLL_UTILS_TBL_SIZE (sizeof(pll_utils_tbl) / sizeof(submenu_xtable_t))

/*
 * Primary & secondary submenu items (filled in from xtable)
 */
static mitem_t ovld_pll_utils_pri_items[PLL_UTILS_TBL_SIZE + MAX_BASE_ITEMS];
static mitem_t ovld_pll_utils_sec_items[PLL_UTILS_TBL_SIZE + MAX_BASE_ITEMS];

static struct menuinfo ovld_pll_utils = {
    "PLL Utilities",             /* title */
    0,                           /* title string added by init_empty_menu */
    (PFT)menu_show_dflags,       /* shows major flags */
    0,                           /* generic prompt */
    0,                           /* size -- bumped by add_menu_item() */
    ovld_pll_utils_pri_items,
};

static struct menuinfo *ovld_pll_utils_p = &ovld_pll_utils;


/*******************************************************************************
 *
 * Function   : ovld_pll_doall_test
 * Description:	Function to run all tests in menu with DOALL flag.
 * Inputs     :	None
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
int ovld_pll_doall_test (void)
{
    if (ovld_pll_tests_wrap() != PASSED) {
        return (FAILED);
    }

    return (PASSED);
}


/*******************************************************************************
 *
 * Function   :	build_ovld_pll_menu
 * Description:	Build Overlord PLL tests and utilities submenu.
 * Inputs     :	submenu - TRUE if invoked from submenu
 * Outputs    : None
 *
 *******************************************************************************
 */
int build_ovld_pll_menu (int submenu)
{
    build_primary_submenu(ovld_pll_diag_tbl, OVLD_PLL_DIAG_TBL_SIZE,
                          "PLL tests SubMenu", &ovld_pll_diag_p);
    build_secondary_submenu(ovld_pll_diag_tbl, OVLD_PLL_DIAG_TBL_SIZE,
                            ovld_pll_diag_sec_items);

    if (submenu) {
        /* Entered with submenu */
        menu(&ovld_pll_diag, ovld_pll_diag_sec_items, 0);
    } else {
        return (ovld_pll_doall_test());
    }

    return (PASSED);
}


/*******************************************************************************
 *
 * Function   : build_ovld_pll_utils
 * Description: Build Overlord PLL related utilities submenu.
 * Inputs     : submenu - TRUE if invoked from submenu
 * Outputs    : None
 *
 *******************************************************************************
 */
static void build_ovld_pll_utils (int submenu)
{
    build_primary_submenu(pll_utils_tbl, PLL_UTILS_TBL_SIZE,
                          "PLL Utilities", &ovld_pll_utils_p);
    build_secondary_submenu(pll_utils_tbl, PLL_UTILS_TBL_SIZE,
                            ovld_pll_utils_sec_items);

    menu(&ovld_pll_utils, ovld_pll_utils_sec_items, 0);
}


/*******************************************************************************
 *
 * Function   : ovld_pll_test 
 * Description:	Function to do Overlord PLL test.
 * Inputs     : clk_type - Clock that be tested
 * Outputs    : Passed or Failed 
 *
 *******************************************************************************
 */
static int ovld_pll_test (uint32_t clk_type)
{
    uint32_t          check_time = 0;
    uint32_t          pll_test_conf = 0, pll_lol_sel = 0;
    uint32_t          pll_test_status = 0xFFFFFFFF, pll_check_mask = 0;
    uint32_t          pll_clean_bit = 0, pll_check_mask2 = 0;
    volatile uint32_t *clk_pll_sel, *clk_pll_conf, *clk_pll_stat, *ext_dev_rst;

    /* Related parameters set-up. */
    clk_pll_sel = (volatile uint32_t *)(dash_fpga + EXT_PLL_REF_SEL_REG_OFF);
    clk_pll_conf = (volatile uint32_t *)(dash_fpga + EXT_SYNC_PLL_CONF_REG_OFF);
    clk_pll_stat = (volatile uint32_t *)(dash_fpga + EXT_SYNC_PLL_STAT_REG_OFF);
    ext_dev_rst = (volatile uint32_t *)(dash_fpga + EXT_DEV_RESET_REG_OFF);

    switch (clk_type) {
    case OVLD_PLL_REF1:
        pll_test_conf = REF1_25M_25M_111HZ; 
        pll_lol_sel = REF1_LOL_SEL;
        pll_check_mask = REF1_PLL_CHECK_MSK;
        pll_clean_bit = REF1_PLL_CLEAN_BIT;
        pll_check_mask2 = REF1_PLL_CHECK_MSK2;
        break;
    case OVLD_PLL_REF2:
        pll_test_conf = REF2_8K_25M_60HZ;
        pll_lol_sel = REF2_LOL_SEL;
        pll_check_mask = REF2_PLL_CHECK_MSK;
        pll_clean_bit = REF2_PLL_CLEAN_BIT;
        pll_check_mask2 = REF2_PLL_CHECK_MSK2;
        break;
    default:
        printf("\n%s: Invalid PLL reference clock (0x%02X).\n",
               __FUNCTION__, clk_type);
        return (FAILED);
    }

    /* 1. PLL Ref. CLK Lock Test. */
    /* 1.1 Set Ext. PLL Ref. Select Register (0x10108) */
    *clk_pll_sel = OVLD_PLL_TEST_REF_CLK;
    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("\n%s:%d Overlord FPGA EXT_PLL_REF_SELECT register"
               "(%p): 0x%04X.\n",
               __FUNCTION__, __LINE__, clk_pll_sel, *clk_pll_sel);
    }

    /* 1.2 Set Ext. Sync CLK PLL Config Register (0x10200) */
    *clk_pll_conf = pll_test_conf;
    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("\n%s:%d Overlord FPGA EXT_SYNC_PLL_CONFIG register"
               "(%p): 0x%04X.\n",
               __FUNCTION__, __LINE__, clk_pll_conf, *clk_pll_conf);
    }

    /* 1.3 Reset PLL */
    *ext_dev_rst |= FPGA_EXT_CLK_RST;
    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("\n%s:%d Overlord FPGA EXT_DEVICE_RESET register"
               "(%p): 0x%04X.\n",
               __FUNCTION__, __LINE__, ext_dev_rst, *ext_dev_rst);
    }

    usleep(2);   /* wait 2us based on HW's instruction */
    
    *ext_dev_rst &= (~FPGA_EXT_CLK_RST);
    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("\n%s:%d Overlord FPGA EXT_DEVICE_RESET register"
               "(%p): 0x%04X.\n",
               __FUNCTION__, __LINE__, ext_dev_rst, *ext_dev_rst);
    }

    /* 1.4 Check Ext. Sync CLK PLL status Register (0x10204) */
    check_time = 0;
    while (check_time <= OVLD_PLL_MAX_POLLING_TIME) {

        pll_test_status = *clk_pll_stat;
        if ((NVRAM)->diagflag & D_VERBOSE) {
            printf("\n%s:%d Overlord FPGA EXT_SYNC_PLL_STATUS register"
                   "(%p): 0x%04X.\n",
                   __FUNCTION__, __LINE__, clk_pll_stat, *clk_pll_stat);
        }

        if (!(pll_test_status & pll_check_mask)) {
            break;
        }
        msleep(100);   /* Based on HW's instruction */
        check_time += 100;
    }

    pll_change_time_reset[(clk_type - 1)] = check_time;

    if ((check_time >= OVLD_PLL_MAX_POLLING_TIME) &&
        (pll_test_status & pll_check_mask)) {
        printf("\n%s:%d Time Out !! LOL and LOS bits are not 0 (0x%08X).\n",
               __FUNCTION__, __LINE__, pll_test_status);
        return (FAILED); 
    }

    /* Try to clean bit */
    *clk_pll_stat = pll_clean_bit;

    check_time = 0;
    while (check_time <= OVLD_PLL_MAX_POLLING_TIME) {

        pll_test_status = *clk_pll_stat;

        if ((NVRAM)->diagflag & D_VERBOSE) {
            printf("\n%s:%d Overlord FPGA EXT_SYNC_PLL_STATUS register"
                   "(%p): 0x%04X.\n",
                   __FUNCTION__, __LINE__, clk_pll_stat, *clk_pll_stat);
        }

        if (!(pll_test_status & pll_check_mask2)) {
            break;
        }
        msleep(100);
        check_time += 100;
    }

    pll_change_time_clean[(clk_type - 1)] = check_time;

    if ((check_time >= OVLD_PLL_MAX_POLLING_TIME) &&
        (pll_test_status & pll_check_mask2)) {
        printf("\n%s:%d Time Out !! LOL, LOS status and interrupt bits"
               " are not 0 (0x%08X).\n",
               __FUNCTION__, __LINE__, pll_test_status);
        return (FAILED); 
    }

    /* 2. PLL Ref. CLK Loss of Lock Test. */
    /* 2.1 Set Ext. PLL Ref. Select Register (0x10108) */
    *clk_pll_sel = pll_lol_sel;
    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("\n%s:%d Overlord FPGA EXT_PLL_REF_SELECT register"
               "(%p): 0x%04X.\n",
               __FUNCTION__, __LINE__, clk_pll_sel, *clk_pll_sel);
    }

    /* 2.2 Check Ext. Sync CLK PLL status Register (0x10204) */
    pll_test_status = 0xFFFFFFFF;

    check_time = 0;
    while (check_time <= OVLD_PLL_MAX_POLLING_TIME) {

        pll_test_status = *clk_pll_stat;
        if ((NVRAM)->diagflag & D_VERBOSE) {
            printf("\n%s:%d Overlord FPGA EXT_SYNC_PLL_STATUS register"
                   "(%p): 0x%04X.\n",
                   __FUNCTION__, __LINE__, clk_pll_stat, *clk_pll_stat);
        }

        if ((pll_test_status & pll_check_mask) == pll_check_mask) {
            break;
        }
        msleep(100);   /* Based on HW's instruction */
        check_time += 100;
    }

    pll_change_time_loss[(clk_type - 1)] = check_time;

    if ((check_time >= OVLD_PLL_MAX_POLLING_TIME) &&
        ((pll_test_status & pll_check_mask) != pll_check_mask)) {
        printf("\n%s:%d Time Out !! LOL, LOS status and interrupt bits"
               " are set (0x%08X).\n",
               __FUNCTION__, __LINE__, pll_test_status);
        return (FAILED); 
    }
    return (PASSED);
}


/*******************************************************************************
 *
 * Function   : ovld_pll_tests_wrap 
 * Description:	Wrap function of Overlord PLL test.
 * Inputs     : None
 * Outputs    : Passed or Failed 
 *
 *******************************************************************************
 */
static int ovld_pll_tests_wrap (void)
{
    int        ref1_test_result = 0, ref2_test_result = 0;
    uint32_t   ovld_fpga_rev = 0, ctr = 0;
    sys_lvl_t  *fpga  = (sys_lvl_t*)dash_fpga;

    testname("Overlord PLL");

    /* Reset Time Buffer */
    memset(pll_change_time_reset, 0, sizeof(pll_change_time_reset));
    memset(pll_change_time_clean, 0, sizeof(pll_change_time_reset));
    memset(pll_change_time_loss, 0, sizeof(pll_change_time_reset));

    /* 1. Check FPGA revision:
     *    Based on HW's info, to support PLL test,
     *    Overlord FPGA revision must be 2.4.2 or higher.
     */
    ovld_fpga_rev = (uint32_t)fpga->ver;
 
    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("\n%s: Overlord Master FPGA Revision register: 0x%08X.\n",
               __FUNCTION__, ovld_fpga_rev);
    }

    if ((ovld_fpga_rev & DASH_FPGA_REV_MSK) < OVLD_PLL_TEST_FPGA_THRES) {
        printf("\nSKIP PLL test since the DASH FPGA revision of this Machine"
               " is %d.%d.%d\n",
               ((ovld_fpga_rev & DASH_FPGA_REV_MAJOR) >> DASH_FPGA_REV_MAJOR_OFF),
               ((ovld_fpga_rev & DASH_FPGA_REV_MINOR) >> DASH_FPGA_REV_MINOR_OFF),
               (ovld_fpga_rev & DASH_FPGA_REV_DEBUG));
        printf("(To support PLL test, please upgrade your DASH FPGA to revision"
               " 2.4.2 or higher)\n");
        return (PASSED);
    }

    /* 2. Check HW Board revision:
     *    Based on HW's suggestion, @84h, bit[26:24] > 4
     *    Skipped HW rev 6 and above
     */
    if (((ovld_fpga_rev & DASH_FPGA_HW_BRD_REV) >> DASH_FPGA_HW_BRD_OFF) > OVLD_PLL_TEST_HW_THRES) {
        printf("\nSKIP PLL test since the DASH HW revision is 6 and above\n");
        return (PASSED);
    }

    /* Do PLL test */
    prpass(testpass, "REF%d CLK ", OVLD_PLL_REF1);
    ref1_test_result = ovld_pll_test(OVLD_PLL_REF1);
    if (ref1_test_result != PASSED) {
        cterr('f', 0, "%s:%d Overlord PLL test with REF1 clock FAILED",
                      __FUNCTION__, __LINE__);
    } else {
        if (!((NVRAM)->diagflag & D_CONTINUOUS)) {
            printf("passed.\n");
        }
    }

    prpass(testpass, "REF%d CLK ", OVLD_PLL_REF2);
    ref2_test_result = ovld_pll_test(OVLD_PLL_REF2);
    if (ref2_test_result != PASSED) {
        cterr('f', 0, "%s:%d Overlord PLL test with REF2 clock FAILED",
                      __FUNCTION__, __LINE__);
    } else {
        if (!((NVRAM)->diagflag & D_CONTINUOUS)) {
            printf("passed.\n");
        }
    }

    /* Display PLL CLK change time for each sub-test based on HW's request. */
    for (ctr = 0; ctr < TIME_BUF_SIZE; ctr++) {
        printf("\nREF CLK %d:\n", (ctr + 1));
        printf("LOL and LOS change time for Lock test is around %d(ms).\n",
               pll_change_time_reset[ctr]);
        printf("LOL and LOS change time for Loss of Lock test is around %d(ms).\n",
               pll_change_time_loss[ctr]);
    }

    if ((ref1_test_result != PASSED) || (ref2_test_result != PASSED)) {
        return (FAILED);
    }

    return (PASSED);
}


/*------------------------------------------------------------------
$Log: platform_pll.c,v $
Revision 1.3  2018/05/18 09:24:51  alpeng
 Neptune merge to trunk with tag <neptune-branch-0518>

Revision 1.2  2017/08/15 01:52:26  alpeng
skip pll test on o2 if hw revision is 6 and above

Revision 1.1.66.1  2018/05/17 10:50:22  alpeng
 sync with trunk <trunk-051618>

Revision 1.2  2017/08/15 01:52:26  alpeng
skip pll test on o2 if hw revision is 6 and above

Revision 1.1  2013/05/09 05:42:37  alpeng
moving overlord common code from x86

Revision 1.1  2012/11/17 02:25:24  palin2
Add PLL test support.

$Endlog$
*/

