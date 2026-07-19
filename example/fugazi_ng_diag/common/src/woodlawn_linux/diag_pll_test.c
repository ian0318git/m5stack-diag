/* $Id: diag_pll_test.c,v 1.2 2013/10/08 08:48:29 tirawan Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/woodlawn_linux/diag_pll_test.c,v $
 *-----------------------------------------------------------------------------
 * diag_pll_test.c - pll test function for Woodlawn
 *
 * February 2012, Leslie Chen
 * Copyright (c) 2013 by Cisco Systems, Inc.
 * All rights reserved.
 *-----------------------------------------------------------------------------
 */


#include "proto.h"
#include "menu.h"
#include "types.h"
#include "common.h"

#include <stdio.h>
#include <stdlib.h>
#include "nvsysvars.h"
#include "error.h"
#include "i2c_api.h"
#include "platform_i2c.h"
#include "common_utils.h"
#include "diag_pll_lib.h"
#include "defs.h"

#define PLL_RONLY    (READ_ONLY | REG_ACCESS)
#define PLL_RW       (READ_WRITE | REG_ACCESS)

int pll_test(int);
int pll_register_test(void);
static int pll_utility(int);
static int display_pll_regs(void);
static int alter_pll_regs(void);
static int diag_pll_read_fn(ulong, int, ulong *, void *);
static int diag_pll_write_fn(ulong, int, ulong, void *);

static reg_info_t_ext pll_reg_ext = {1, diag_pll_read_fn,
                                      diag_pll_write_fn, 0};

static reg_info_t pll_test_regs[] = {
    {"PLL 0x1F",       0X1F, PLL_RW,   {(unsigned long)&pll_reg_ext},   0xFF, 0x0},
    {"PLL 0x55",       0X55, PLL_RW,   {(unsigned long)&pll_reg_ext},   0xFF, 0x0},
    {"PLL 0x6C",          0X6C, PLL_RW,{(unsigned long)&pll_reg_ext},   0xFF, 0x0},
    {"PLL 0x6D",     0X6D, PLL_RW,     {(unsigned long)&pll_reg_ext},   0xFF, 0x0},
    {"PLL 0x6E",         0X6E, PLL_RW,  {(unsigned long)&pll_reg_ext},   0xFF, 0x0},
    {"END",                       0x00,  0,           {0},   0x0,  0x0},
};

/* Sub Menu used for FPGA tests.*/
static submenu_xtable_t pll_tests_submenu_table[] = {
   {"PLL Utility", (type_t(*)())pll_utility,   FALSE,
       0, NULL, 0, (type_t(*)())pll_utility,   TRUE}, 
   {"PLL Register Test", (type_t(*)()) pll_register_test,   0,
       MF_CONTINUOUS | MF_DOGRP | MF_DOALL, (type_t(*)())0, 0, (type_t(*)())0,   0},
};

#define PLL_TESTS_SUBMENU_TABLE_SIZE (sizeof(pll_tests_submenu_table) / \
                                     sizeof(submenu_xtable_t))


/*
 * Primary & secondary submenu items (filled in from xtable)
 */
static mitem_t pll_tests_primary_items[PLL_TESTS_SUBMENU_TABLE_SIZE +
                                       MAX_BASE_ITEMS];
static mitem_t pll_tests_secondary_items[PLL_TESTS_SUBMENU_TABLE_SIZE +
                                       MAX_BASE_ITEMS];

menuinfo_t pll_subtest_menu = {
    "%s Subtest Menu",
    0,                                /* mtparam added by init_empty_menu */
    (PFT)show_endnote,                /* notes missing WICs in combos */
    0,                                /* use generic prompt */
    0,                                /* size (bumped by add_menu_item() */
    pll_tests_primary_items,
};
menuinfo_t *pll_submenup = &pll_subtest_menu;

/* List of PLL Utilities */
static submenu_xtable_t pll_util_items[] = {
    {"Display PLL Register", (type_t(*)())display_pll_regs, 0, 0,
     (type_t(*)())0, 0, (type_t(*)())0, 0},
    {"Alter PLL Register", (type_t(*)())alter_pll_regs, 0, 0,
     (type_t(*)())0, 0, (type_t(*)())0, 0},
};

#define PLL_TESTS_UTIL_SIZE (sizeof(pll_util_items) / \
                                     sizeof(submenu_xtable_t))

/*
 * pll util items (filled in from xtable)
 */
static mitem_t pll_tests_primary_util_items[PLL_TESTS_UTIL_SIZE +
                                     MAX_BASE_ITEMS];
static mitem_t pll_tests_secondary_util_items[PLL_TESTS_UTIL_SIZE +
                                     MAX_BASE_ITEMS];

/*
 * PLL Utils submenu
 */
menuinfo_t pll_util_menu = {
    "PLL Utility Menu",
    0,
    (PFT)show_endnote,
    0,
    0,
    pll_tests_primary_util_items,
};

menuinfo_t *pll_util_menup = &pll_util_menu;

/*
 * The menu driven diagnostics are called from either the low level
 * power-on routine or from the monitor.  Frommon will be non-zero if
 * called from the monitor.  Note that this value is actually the
 * argc parameter (a little slight of hand here), which is always
 * greater than zero.
 *
 * The main menu is now defined in an _xtable_.  Both the primary items
 * and the secondary (shadow) items are built with function calls that
 * operate on it and insert the appropriate base items into the menu.
 */

int pll_test (int show_menu)
{
    build_primary_submenu(pll_tests_submenu_table,
                            PLL_TESTS_SUBMENU_TABLE_SIZE,
                            "FPGA", &pll_submenup);
    build_secondary_submenu(pll_tests_submenu_table,
                            PLL_TESTS_SUBMENU_TABLE_SIZE,
                            pll_tests_secondary_items);

    if (show_menu) {
        menu(pll_submenup, pll_tests_secondary_items, '\0' );
    } else {
        menu_exec_doall_diags(pll_submenup);
        prcomplete(testpass, errcount, (char *)0);
    }
    return (PASSED);
}

/*******************************************************************************
 *
 * Function    : pll_utility
 * Description :
 * Inputs      : menu_option - display menu instead of running all temp. sensor tests.
 * Outputs     : PASSED / FAILED
 *
 *******************************************************************************
 */

static int pll_utility (int show_menu)
{
    build_primary_submenu(pll_util_items, PLL_TESTS_UTIL_SIZE,
                          "PLL Utilities Menu", &pll_util_menup);
    build_secondary_submenu(pll_util_items, PLL_TESTS_UTIL_SIZE,
                            pll_tests_secondary_util_items);

    if (show_menu) {
        menu(pll_util_menup, pll_tests_secondary_util_items, '\0' );
    } else {
        menu_exec_doall_diags(pll_util_menup);
        prcomplete(testpass, errcount, (char *)0);
    }

    return (PASSED);
}

/*******************************************************************************
 *
 * Function    : pll_register_test
 * Description : Do wan pll 3399 register r/w test.
 * Inputs      : menu_option - menu option passed from menu selection
 * Outputs    : PASSED - 0 - everything went well 
 *                  FAILED - 1 - there were some problem
 *
 ******************************************************************************/

 int pll_register_test (void)
 {
    testname("PLL Register");

    if (register_tests(0, pll_test_regs) == FAILED) {
        cterr('f', 0, "PLL Register Test Failed");
        return (FAILED);
    }

    prpass(testpass, "PLL Register Test Success");
    return (PASSED);
 }

/******************************************************************************
 *
 * Function: alter_pll_regs
 *
 * Description: This function supports to alter the PLL register.
 *
 * Inputs      : None
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/
static int alter_pll_regs (void)
{
    uint32 rv;
    uint reg_addr;
    uchar reg_val, i2c_slave_addr, pll_buf;
    n2g_i2c_dev_t i2c_dev;

    i2c_slave_addr = CAVIUM_PLL;
    printf("Alter PLL Register.\n");
    reg_addr = gethex_answer("Enter register address (0x0 ~ 0xFF): ", 0, 0, 0xff); 

    /* Display original value */
    if (open_i2c(&i2c_dev, i2c_slave_addr, CPU_I2C0) == FAILED) {
        printf("Fail to open the i2c interface\n");
        return (FAILED);
    }
   
    rv = read_i2c_reg(&i2c_dev, &pll_buf, reg_addr, sizeof(pll_p));
    if (rv != PASSED) {
        printf("Read reg fail at offset %#.2x\n", reg_addr);
        return (FAILED);
    } else {
        printf("Original PLL val at offset %#.2x = %#.2x\n", reg_addr, pll_buf);
    }
 
    /* Alter register with new value */
    reg_val = gethex_answer("Enter the new data (hex): ", 0, 0, 0xFF);

    if (write_i2c_reg(&i2c_dev, &reg_val, reg_addr, sizeof(pll_p)) == FAILED) {
        printf("Fail to write new val.");
        return (FAILED);
    }

    /* Display the value again */
    rv = read_i2c_reg(&i2c_dev, &pll_buf, reg_addr, sizeof(pll_p));
    if (rv != PASSED) {
        printf("Read reg fail at offset %#.2x\n", reg_addr);
        return (FAILED);
    } else {
        printf("New PLL val at offset %#.2x=%.8x\n", reg_addr, pll_buf);
    }

    return (PASSED);
}

/******************************************************************************
 *
 * Function: display_pll_regs
 *
 * Description: This function supports to display the PLL registers.
 *
 * Inputs      : None
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/
static int display_pll_regs (void)
{
    uint32 rv;
    uint reg_addr;
    uchar i2c_slave_addr, pll_buf;
    n2g_i2c_dev_t i2c_dev;

    i2c_slave_addr = CAVIUM_PLL;
    printf("Dump PLL Register.\n");
    reg_addr = gethex_answer("Enter register address (0x0 ~ 0xFF): ", 0, 0, 0xff); 

    /* Display register value */
    if (open_i2c(&i2c_dev, i2c_slave_addr, CPU_I2C0) == FAILED) {
        printf("Fail to open the i2c interface\n");
        return (FAILED);
    }
   
    rv = read_i2c_reg(&i2c_dev, &pll_buf, reg_addr, sizeof(pll_p));
    if (rv != PASSED) {
        printf("Read reg fail at offset %#.8x\n", reg_addr);
        return (FAILED);
    } else {
        printf("PLL val at offset %#.8x = %#.2x\n", reg_addr, pll_buf);
    }

    return (PASSED);
}

/**********************************************************************
 *
 * Function: diag_pll_read_fn
 *
 * Description: PLL register read called by register_display
 *
 * Inputs      : addr - PLL register offset
 *               size - PLL register size
 *               *buf - pointer to the data buf
 *               *param - pointer to param
 *
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/
static int diag_pll_read_fn (ulong addr, int size, ulong *buf, void *param)
{
    char temp;

    pll_reg_read(addr, &temp);

    *buf = temp;

    return (PASSED);
}

/******************************************************************************
 *
 * Function: diag_pll_write_fn
 *
 * Description: handoff PLL register write called by register_display
 *
 * Inputs      : addr - PLL register offset
 *               size - PLL register size
 *               data - the write data value
 *               *param - pointer to param
 *
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/
static int diag_pll_write_fn (ulong addr, int size, ulong data, void *param)
{
    pll_reg_write((int)addr, (char)data);
    return (PASSED);
}

/*-------------------------------------------------
 * $Log: diag_pll_test.c,v $
 * Revision 1.2  2013/10/08 08:48:29  tirawan
 * Woodlawn collapsed to main trunk
 *
 * Revision 1.1.4.2  2013/08/20 10:58:54  tirawan
 * Branch into woodlawn-branch2 and port woodlawn code
 *
 * Revision 1.1.2.1  2013/04/24 10:37:18  tirawan
 * Initial check-in for woodlawn linux code
 *
 * Revision 1.1  2013/03/13 06:42:52  kuangik
 * Add for the first time
 *
 * Revision 1.3  2013/01/18 06:29:12  leslie
 * Fix and clean up code.
 *
 * Revision 1.2  2012/10/24 10:38:00  leslie
 * Fix and clean up code.
 *
 * Revision 1.1  2012/09/05 22:19:59  leslie
 * Add Woodlawn PLL test item.
 *
 * $Endlog$
 *-------------------------------------------------
 */
