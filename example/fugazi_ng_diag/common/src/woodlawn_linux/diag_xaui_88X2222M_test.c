/* $Id: diag_xaui_88X2222M_test.c,v 1.6 2015/02/14 12:48:41 kodko Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/woodlawn_linux/diag_xaui_88X2222M_test.c,v $
 *-----------------------------------------------------------------------------
 * diag_xaui_88X2222M_test.c - Menu for Woodlawn PHY 88X2222M
 *
 * February 2012, Kody Ko
 * Copyright (c) 2015 by Cisco Systems, Inc.
 * All rights reserved.
 *-----------------------------------------------------------------------------
 */
#include "error.h"
#include "common.h"
#include "types.h"
#include "menu.h"
#include <stdio.h>
#include "defs.h"
#include "diag_common_lib.h"
#include "platform_xaui.h"
#include "diag_xaui_88X2222M_lib.h"
#include "platform_smi.h"
#include "platform_smi_lib.h"
#include "common_utils.h"
#include "diag_xaui_88X2222M_test.h"
#include "platform_eth.h"
#include "cvmx.h"
#include "platform_smi.h"
#include "platform_smi_lib.h"
#include "cvmx-mdio.h"
#include "diag_fpga_lib.h"
#include "diag_common_drv.h"

extern void msleep(unsigned long);
static int read_ten_g_phy_reg_fn(ulong, int, ulong *, void *);
static int write_ten_g_phy_reg_fn(ulong, int, ulong, void *);
extern int marvell_2222p_init(void);

#define MF_1    (MF_CONTINUOUS | MF_DOGRP)
#define MF_2    (MF_1 | MF_DOALL)
#define MF_3    (MF_2 | MF_SHOW_ERRCOUNT)
#define MF_4    (MF_1 | MF_SHOW_ERRCOUNT)

#define MARVELL_PHY_RONLY    (READ_ONLY | REG_ACCESS)
#define MARVEL_PHY_RW        (READ_WRITE | REG_ACCESS)

ten_g_phy_t phy_88X2222M_dev1_port_0  = {MRV88X2222M_REG_DEVICE_1,
                                         MRVL_88X2222M_SMI2_PORT0_ADDR};
ten_g_phy_t phy_88X2222M_dev2_port_0  = {MRV88X2222M_REG_DEVICE_2,
                                         MRVL_88X2222M_SMI2_PORT0_ADDR};
ten_g_phy_t phy_88X2222M_dev3_port_0  = {MRV88X2222M_REG_DEVICE_3,
                                         MRVL_88X2222M_SMI2_PORT0_ADDR};
ten_g_phy_t phy_88X2222M_dev4_port_0  = {MRV88X2222M_REG_DEVICE_4,
                                         MRVL_88X2222M_SMI2_PORT0_ADDR};
ten_g_phy_t phy_88X2222M_dev30_port_0 = {MRV88X2222M_REG_DEVICE_30,
                                         MRVL_88X2222M_SMI2_PORT0_ADDR};
ten_g_phy_t phy_88X2222M_dev31_port_0 = {MRV88X2222M_REG_DEVICE_31,
                                         MRVL_88X2222M_SMI2_PORT0_ADDR};

ten_g_phy_t phy_88X2222M_dev1_port_2  = {MRV88X2222M_REG_DEVICE_1,
                                         MRVL_88X2222M_SMI2_PORT2_ADDR};
ten_g_phy_t phy_88X2222M_dev2_port_2  = {MRV88X2222M_REG_DEVICE_2,
                                         MRVL_88X2222M_SMI2_PORT2_ADDR};
ten_g_phy_t phy_88X2222M_dev3_port_2  = {MRV88X2222M_REG_DEVICE_3,
                                         MRVL_88X2222M_SMI2_PORT2_ADDR};
ten_g_phy_t phy_88X2222M_dev4_port_2  = {MRV88X2222M_REG_DEVICE_4,
                                        MRVL_88X2222M_SMI2_PORT2_ADDR};
ten_g_phy_t phy_88X2222M_dev30_port_2 = {MRV88X2222M_REG_DEVICE_30,
                                         MRVL_88X2222M_SMI2_PORT2_ADDR};
ten_g_phy_t phy_88X2222M_dev31_port_2 = {MRV88X2222M_REG_DEVICE_31,
                                         MRVL_88X2222M_SMI2_PORT2_ADDR};

 /***********************************************************************
 *  88X2222M reg_info_t extension - for "PORT 0"
 ************************************************************************/
/* reg_info_t extension for "device 1" access */
static reg_info_t_ext phy_88X2222M_port_0_ext1 = {MRVL_88X2222M_PHY_REG_LEN,
                                                  read_ten_g_phy_reg_fn,
                                                  write_ten_g_phy_reg_fn,
                                                  &phy_88X2222M_dev1_port_0};
#ifdef NOT_USED_WARNING
/* reg_info_t extension for "device 2" access */
static reg_info_t_ext phy_88X2222M_port_0_ext2 = {MRVL_88X2222M_PHY_REG_LEN,
                                                  read_ten_g_phy_reg_fn,
                                                  write_ten_g_phy_reg_fn,
                                                  &phy_88X2222M_dev2_port_0};
#endif
/* reg_info_t extension for "device 3" access */
static reg_info_t_ext phy_88X2222M_port_0_ext3 = {MRVL_88X2222M_PHY_REG_LEN,
                                                  read_ten_g_phy_reg_fn,
                                                  write_ten_g_phy_reg_fn,
                                                  &phy_88X2222M_dev3_port_0};
#ifdef NOT_USED_WARNING
/* reg_info_t extension for "device 4" access */
static reg_info_t_ext phy_88X2222M_port_0_ext4 = {MRVL_88X2222M_PHY_REG_LEN,
                                                  read_ten_g_phy_reg_fn,
                                                  write_ten_g_phy_reg_fn,
                                                  &phy_88X2222M_dev4_port_0};
/* reg_info_t extension for "device 30" access */
static reg_info_t_ext phy_88X2222M_port_0_ext30 = {MRVL_88X2222M_PHY_REG_LEN,
                                                   read_ten_g_phy_reg_fn,
                                                   write_ten_g_phy_reg_fn,
                                                   &phy_88X2222M_dev30_port_0};
#endif
/* reg_info_t extension for "device 31" access */
static reg_info_t_ext phy_88X2222M_port_0_ext31 = {MRVL_88X2222M_PHY_REG_LEN,
                                                   read_ten_g_phy_reg_fn,
                                                   write_ten_g_phy_reg_fn,
                                                   &phy_88X2222M_dev31_port_0};

 /***********************************************************************
 *  88X2222M reg_info_t extension - for "PORT 2"
 ************************************************************************/
/* reg_info_t extension for "device 1" access */
static reg_info_t_ext phy_88X2222M_port_2_ext1 = {MRVL_88X2222M_PHY_REG_LEN,
                                                  read_ten_g_phy_reg_fn,
                                                  write_ten_g_phy_reg_fn,
                                                  &phy_88X2222M_dev1_port_2};
#ifdef NOT_USED_WARNING
/* reg_info_t extension for "device 2" access */
static reg_info_t_ext phy_88X2222M_port_2_ext2 = {MRVL_88X2222M_PHY_REG_LEN,
                                                  read_ten_g_phy_reg_fn,
                                                  write_ten_g_phy_reg_fn,
                                                  &phy_88X2222M_dev2_port_2};
#endif
/* reg_info_t extension for "device 3" access */
static reg_info_t_ext phy_88X2222M_port_2_ext3 = {MRVL_88X2222M_PHY_REG_LEN,
                                                  read_ten_g_phy_reg_fn,
                                                  write_ten_g_phy_reg_fn,
                                                  &phy_88X2222M_dev3_port_2};
#ifdef NOT_USED_WARNING
/* reg_info_t extension for "device 4" access */
static reg_info_t_ext phy_88X2222M_port_2_ext4 = {MRVL_88X2222M_PHY_REG_LEN,
                                                  read_ten_g_phy_reg_fn,
                                                  write_ten_g_phy_reg_fn,
                                                  &phy_88X2222M_dev4_port_2};
/* reg_info_t extension for "device 30" access */
static reg_info_t_ext phy_88X2222M_port_2_ext30 = {MRVL_88X2222M_PHY_REG_LEN,
                                                   read_ten_g_phy_reg_fn,
                                                   write_ten_g_phy_reg_fn,
                                                   &phy_88X2222M_dev30_port_2};
#endif
/* reg_info_t extension for "device 31" access */
static reg_info_t_ext phy_88X2222M_port_2_ext31 = {MRVL_88X2222M_PHY_REG_LEN,
                                                   read_ten_g_phy_reg_fn,
                                                   write_ten_g_phy_reg_fn,
                                                   &phy_88X2222M_dev31_port_2};

 /***********************************************************************
 *  88X2222M table - for "PORT 0"
 ************************************************************************/
/* reg_table for "device 1" */
static reg_info_t marvell_88X2222M_port_0_dev1[] = {   /* Device 1*/
    {"BASE-R FEC Ctrl", 0x00AB, MARVEL_PHY_RW,
     {(unsigned long)&phy_88X2222M_port_0_ext1}, 0x0002, 0x0000},
    {"end", 0x00, 0, {0}, 0, 0},
};

#ifdef NOT_USED_WARNING
/* reg_table for "device 2" */
static reg_info_t marvell_88X2222M_port_0_dev2[] = {   /* Device 2*/
    {"WIS Control 1", 0x0000, MARVEL_PHY_RW,
     {(unsigned long)&phy_88X2222M_port_0_ext2}, 0x4000, 0x2040},
    {"end", 0x00, 0, {0}, 0, 0},
};
#endif

/* reg_table for "device 3" */
static reg_info_t marvell_88X2222M_port_0_dev3[] = {   /* Device 3*/
    {"10GBASE-R PCS Ctrl 1", 0x0000, MARVEL_PHY_RW,
     {(unsigned long)&phy_88X2222M_port_0_ext3}, 0x4000, 0x2040},
    {"end", 0x00, 0, {0}, 0, 0},
};

#ifdef NOT_USED_WARNING
/* reg_table for "device 4" */
static reg_info_t marvell_88X2222M_port_0_dev4[] = {   /* Device 4*/
    {"SERDES Ctrl 1", 0xF003, MARVEL_PHY_RW,
     {(unsigned long)&phy_88X2222M_port_0_ext4}, 0x1080, 0x0000},
    {"end", 0x00, 0, {0}, 0, 0},
};
#endif

/* reg_table for "device 31" */
static reg_info_t marvell_88X2222M_port_0_dev31[] = {   /* Device 31*/
    {"LED0 Control", 0xF020, MARVEL_PHY_RW,
     {(unsigned long)&phy_88X2222M_port_0_ext31}, 0x0772, 0x0160},
    {"end", 0x00, 0, {0}, 0, 0},
};

 /***********************************************************************
 *  88X2222M table - for "PORT 2"
 ************************************************************************/
/* reg_table for "device 1" */
static reg_info_t marvell_88X2222M_port_2_dev1[] = {   /* Device 1*/
    {"BASE-R FEC Ctrl", 0x00AB, MARVEL_PHY_RW,
     {(unsigned long)&phy_88X2222M_port_2_ext1}, 0x0002, 0x0000},
    {"end", 0x00, 0, {0}, 0, 0},
};

#ifdef NOT_USED_WARNING
/* reg_table for "device 2" */
static reg_info_t marvell_88X2222M_port_2_dev2[] = {   /* Device 2*/
    {"WIS Control 1", 0x0000, MARVEL_PHY_RW,
     {(unsigned long)&phy_88X2222M_port_2_ext2}, 0x4000, 0x2040},
    {"end", 0x00, 0, {0}, 0, 0},
};
#endif

/* reg_table for "device 3" */
static reg_info_t marvell_88X2222M_port_2_dev3[] = {   /* Device 3*/
    {"10GBASE-R PCS Ctrl 1", 0x0000, MARVEL_PHY_RW,
     {(unsigned long)&phy_88X2222M_port_2_ext3}, 0x4000, 0x2040},
    {"end", 0x00, 0, {0}, 0, 0},
};

#ifdef NOT_USED_WARNING
/* reg_table for "device 4" */
static reg_info_t marvell_88X2222M_port_2_dev4[] = {   /* Device 4*/
    {"SERDES Ctrl 1", 0xF003, MARVEL_PHY_RW,
     {(unsigned long)&phy_88X2222M_port_2_ext4}, 0x1080, 0x0000},
    {"end", 0x00, 0, {0}, 0, 0},
};
#endif

/* reg_table for "device 31" */
static reg_info_t marvell_88X2222M_port_2_dev31[] = {   /* Device 31*/
    {"LED0 Control", 0xF020, MARVEL_PHY_RW,
     {(unsigned long)&phy_88X2222M_port_2_ext31}, 0x0772, 0x0160},
    {"end", 0x00, 0, {0}, 0, 0},
};
/*****************************************************************************
 *  Functions Declaration
 *****************************************************************************/
int xaui_88X2222M_test(int);
static int xaui_88X2222M_register_test(void);
static int xaui_88X2222M_internal_loopback_test(void);
static int xaui_88X2222M_external_loopback_test(void);
int dump_phy_88X2222M_registers(void);
int alter_phy_88X2222M_register(void);
int dump_phy_88X2222M_ptp_registers(void);
int alter_phy_88X2222M_ptp_register(void);
int verify_2222_clk_trig_in(void);
static int xaui_88X2222M_utility(int);
static int enter_prbs31_mode(void);
static int enter_prbs9_mode(void);
static int enter_prbs9_inverted_mode(void);
static int enter_square_wave_mode(void);
static int disable_prbs_test(void);


int xaui_88X222M_do_all_wrapper(void);

/******************************************************************************
 *  List of Menu used for XAUI 88X2222M
 *****************************************************************************/
static submenu_xtable_t xaui_88X2222M_tests_submenu_table[] = {
   {"PHY 88X2222M Utility", (type_t(*)())xaui_88X2222M_utility,   FALSE,
       0, NULL, 0, (type_t(*)())xaui_88X2222M_utility,   TRUE},
   {"PHY 88X2222M Register Test", (type_t(*)())xaui_88X2222M_register_test,   0,
       MF_3, (type_t(*)())0, 0, (type_t(*)())0,   0},
   {"PHY 88X2222M Internal Loopback Test", (type_t(*)())xaui_88X2222M_internal_loopback_test,   0,
       MF_3, (type_t(*)())0, 0, (type_t(*)())0,   0},
   {"PHY 88X2222M SFP+ External Loopback Test", (type_t(*)())xaui_88X2222M_external_loopback_test,   0,
       MF_3, (type_t(*)())0, 0, (type_t(*)())0,   0},
};

/******************************************************************************
 *  List of Utilities used for XAUI 88X2222M
 *****************************************************************************/
static submenu_xtable_t xaui_88X2222M_util_items[] = {
    {"Dump PHY 88X2222M Registers", (type_t(*)())dump_phy_88X2222M_registers, 0, 0, (type_t(*)())0,
     0, (type_t(*)())0, 0},
    {"Alter PHY 88X2222M Register", (type_t(*)())alter_phy_88X2222M_register, 0, 0,
     (type_t(*)())0, 0, (type_t(*)())0, 0},
    {"Switch SFP Plus Led_OFF", (type_t(*)())switch_sfp_plus_led, 0, 0,
     (type_t(*)())0, 0, (type_t(*)())0, 0},
    {"Switch SFP Plus Led_ON", (type_t(*)())switch_sfp_plus_led, 1, 0,
     (type_t(*)())0, 0, (type_t(*)())0, 0},
    {"Enter PRBS31 Mode", (type_t(*)())enter_prbs31_mode, 0, 0,
     (type_t(*)())0, 0, (type_t(*)())0, 0},
    {"Enter PRBS9 Mode", (type_t(*)())enter_prbs9_mode, 0, 0,
     (type_t(*)())0, 0, (type_t(*)())0, 0},
    {"Enter PRBS9 Inverted Mode", (type_t(*)())enter_prbs9_inverted_mode, 0, 0,
     (type_t(*)())0, 0, (type_t(*)())0, 0},
    {"Enter 8180(Square Wave) Mode", (type_t(*)())enter_square_wave_mode, 0, 0,
     (type_t(*)())0, 0, (type_t(*)())0, 0},
    {"Disable generator and checker", (type_t(*)())disable_prbs_test, 0, 0,
      (type_t(*)())0, 0, (type_t(*)())0, 0},
    {"Execute 88X2222M initializatioin script", (type_t(*)())marvell_2222m_init, 0, 0,
      (type_t(*)())0, 0, (type_t(*)())0, 0},
    {"Execute 88X2222M SFI compliance testing", (type_t(*)())marvell_2222m_sfi_compliance_testing, 0, 0,
      (type_t(*)())0, 0, (type_t(*)())0, 0},
    {"Dump 88X2222M PTP Registers", (type_t(*)())dump_phy_88X2222M_ptp_register, 0, 0, 
     (type_t(*)())0, 0, (type_t(*)())0, 0},
    {"Alter 88X2222M PTP Register", (type_t(*)())alter_phy_88X2222M_ptp_register, 0, 0,
     (type_t(*)())0, 0, (type_t(*)())0, 0},
    {"MRVL2222 CLK/TRIG In Verification", (type_t(*)())verify_2222_clk_trig_in, 0, 0,
     (type_t(*)())0, 0, (type_t(*)())0, 0},
    {"Config MRVL2222 CLK Generation", (type_t(*)())config_2222_gen_clk_out, 0, 0,
     (type_t(*)())0, 0, (type_t(*)())0, 0},
    {"Config MRVL2222 TRIG Generation", (type_t(*)())config_2222_gen_trig_out, 0, 0,
     (type_t(*)())0, 0, (type_t(*)())0, 0},
};

/******************************************************************************
 *  Macro Definitions
 *****************************************************************************/
#define XAUI_88X2222M_TESTS_SUBMENU_TABLE_SIZE (sizeof(xaui_88X2222M_tests_submenu_table) / \
                                                sizeof(submenu_xtable_t))

#define XAUI_88X2222M_TESTS_UTIL_SIZE (sizeof(xaui_88X2222M_util_items) / \
                                     sizeof(submenu_xtable_t))

/******************************************************************************
 *  Global Variable
 *****************************************************************************/
/******************************************************************************
 * Primary & secondary submenu items (filled in from xtable)
 *****************************************************************************/
static mitem_t xaui_88X2222M_tests_primary_items[XAUI_88X2222M_TESTS_SUBMENU_TABLE_SIZE +
                                                 MAX_BASE_ITEMS];
static mitem_t xaui_88X2222M_tests_secondary_items[XAUI_88X2222M_TESTS_SUBMENU_TABLE_SIZE +
                                                   MAX_BASE_ITEMS];

/******************************************************************************
 * Primary & secondary utilities menu items (filled in from xtable)
 *****************************************************************************/
static mitem_t xaui_88X2222M_tests_primary_util_items[XAUI_88X2222M_TESTS_UTIL_SIZE +
                                                      MAX_BASE_ITEMS];
static mitem_t xaui_88X2222M_tests_secondary_util_items[XAUI_88X2222M_TESTS_UTIL_SIZE +
                                                        MAX_BASE_ITEMS];

/******************************************************************************
 * XAUI 88X2222M Utils submenu
 *****************************************************************************/
menuinfo_t xaui_88X2222M_util_menu = {
    "XAUI 88X2222M Utility Menu",
    0,
    (PFT)show_endnote,
    0,
    0,
    xaui_88X2222M_tests_primary_util_items,
};
menuinfo_t *xaui_88X2222M_util_menup = &xaui_88X2222M_util_menu;

menuinfo_t xaui_88X2222M_subtest_menu = {
    "%s Subtest Menu",
    0,                                /* mtparam added by init_empty_menu */
    (PFT)show_endnote,                /* notes missing WICs in combos */
    0,                                /* use generic prompt */
    0,                                /* size (bumped by add_menu_item() */
    xaui_88X2222M_tests_primary_items,
};
menuinfo_t *xaui_88X2222M_submenup = &xaui_88X2222M_subtest_menu;

/******************************************************************************
 * The menu driven diagnostics are called from either the low level
 * power-on routine or from the monitor.  Frommon will be non-zero if
 * called from the monitor.  Note that this value is actually the
 * argc parameter (a little slight of hand here), which is always
 * greater than zero.
 *
 * The main menu is now defined in an _xtable_.  Both the primary items
 * and the secondary (shadow) items are built with function calls that
 * operate on it and insert the appropriate base items into the menu.
 ******************************************************************************/

int xaui_88X2222M_test (int show_menu)
{
    build_primary_submenu(xaui_88X2222M_tests_submenu_table,
                            XAUI_88X2222M_TESTS_SUBMENU_TABLE_SIZE,
                            "XAUI 88X2222M", &xaui_88X2222M_submenup);
    build_secondary_submenu(xaui_88X2222M_tests_submenu_table,
                            XAUI_88X2222M_TESTS_SUBMENU_TABLE_SIZE,
                            xaui_88X2222M_tests_secondary_items);

    if (show_menu) {
        menu(xaui_88X2222M_submenup, xaui_88X2222M_tests_secondary_items, '\0' );
    } else {
        menu_exec_doall_diags(xaui_88X2222M_submenup);
    }
    return (PASSED);
}

/*******************************************************************************
 *
 * Function    : xaui_88X222M_do_all_wrapper
 * Description : Wrapper for XAUI PHY 88X2222M do all test items
 * Inputs      : none
 * Outputs     : PASSED / FAILED
 *
 *******************************************************************************
 */
int xaui_88X222M_do_all_wrapper (void)
{
    int rc = PASSED;

    if (xaui_88X2222M_register_test() == FAILED) {
        rc = FAILED;
    }

    if (xaui_88X2222M_internal_loopback_test() == FAILED) {
        rc = FAILED;
    }

    if (xaui_88X2222M_external_loopback_test() == FAILED) {
        rc = FAILED;
    }

    return (rc);
}

/*******************************************************************************
 *
 * Function    : xaui_88X2222M_utility
 * Description :
 * Inputs      : menu_option - display menu instead of running all XAUI 88X2222M
 *               tests.
 * Outputs     : PASSED / FAILED
 *
 ******************************************************************************/
int xaui_88X2222M_utility (int show_menu)
{

    build_primary_submenu(xaui_88X2222M_util_items, XAUI_88X2222M_TESTS_UTIL_SIZE,
                          "XAUI 88X2222M Utilities Menu", &xaui_88X2222M_util_menup);
    build_secondary_submenu(xaui_88X2222M_util_items, XAUI_88X2222M_TESTS_UTIL_SIZE,
                            xaui_88X2222M_tests_secondary_util_items);

    if (show_menu) {
       menu(xaui_88X2222M_util_menup, xaui_88X2222M_tests_secondary_util_items, '\0' );
    } else {
       menu_exec_doall_diags(xaui_88X2222M_util_menup);
       prcomplete(testpass, errcount, (char *)0);
    }

    return (PASSED);
}

/******************************************************************************
 *
 * Function: xaui_88X2222M_register_test
 *
 * Description: This function performs the XAUI 88X2222M register test.
 *
 * Inputs      : None
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/
static int xaui_88X2222M_register_test (void)
{
    testname("88X2222M PHY Register Test");

/***********************************************************************
 *  88X2222M register test - for "PORT 0"
************************************************************************/

    if (register_tests(0, marvell_88X2222M_port_0_dev1) == FAILED) {
        return (FAILED);
    }

    if (register_tests(0, marvell_88X2222M_port_0_dev3) == FAILED) {
        return (FAILED);
    }

    if (register_tests(0, marvell_88X2222M_port_0_dev31) == FAILED) {
        return (FAILED);
    }

/***********************************************************************
 *  88X2222M register test - for "PORT 1"
************************************************************************/
    if (register_tests(0, marvell_88X2222M_port_2_dev1) == FAILED) {
        return (FAILED);
    }

    if (register_tests(0, marvell_88X2222M_port_2_dev3) == FAILED) {
        return (FAILED);
    }

    if (register_tests(0, marvell_88X2222M_port_2_dev31) == FAILED) {
        return (FAILED);
    }

    prpass(testpass, "88X2222M Register Test Success");

    return (PASSED);
}


/*******************************************************************
 *
 * Function    : read_ten_g_phy_reg_fn
 * Description : SMI read funtion for ten_g_phy reg test.
 * Input       : addr  - register offset.
 *               size  - read data size
 *               buf   - read buffer
 *               param - parameter
 *               
 * Output: PASSED/FAILED
 *
 *******************************************************************
 */
static int read_ten_g_phy_reg_fn (ulong addr, int size, ulong *buff, void *addr_info)
{
    return (read_ten_g_phy_reg(addr, size, (uint *)buff, addr_info));
}


/*******************************************************************
 *
 * Function    : write_ten_g_phy_reg_fn
 * Description : SMI write funtion for ten_g_phy reg test.
 * Input       : addr  - register offset.
 *               size  - read data size
 *               value - data to be written.
 *               param - parameter
 *               
 * Output: PASSED/FAILED
 *
 *******************************************************************
 */
static int write_ten_g_phy_reg_fn (ulong addr, int size, ulong val, void *addr_info)
{
    return (write_ten_g_phy_reg(addr, size, val, addr_info));
}


/******************************************************************************
 *
 * Function: xaui_88X2222M_internal_loopback_test
 *
 * Description: This function perform the internal loopback test
 *              (Shallow Host Loopback) from Cavium to 88X2222M.
 *
 * Inputs      : None
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/
static int xaui_88X2222M_internal_loopback_test (void)
{
    uint reg_val;
    int rc = PASSED;
    int ix;
    unsigned int mii_value;
    unsigned int smi2 = 0x2;
    unsigned int port0 = 0x4;

    testname("Xaui %s loopback", "internal");

    /* Init 2222 */
    marvell_2222m_init();
    msleep(200);

    ten_g_phy_t phy_88X2222M = {MRV88X2222M_REG_DEVICE_31,
                                MRVL_88X2222M_SMI2_PORT0_ADDR};

    /* identify the 88x2222 silicon revision */
    mii_value = cvmx_mdio_45_read(smi2, port0, 0x1, 0x0003);

    /* If rev is Z1, software reset 10G phy, will cause xaui link down : Device 4, Register 0x1000 */
    phy_88X2222M.device_id = MRV88X2222M_REG_DEVICE_4;
    if (mii_value == 0x0db1) {
        /* Write register with value 0xa040*/
        if (write_ten_g_phy_reg(XFI_PCS_CTRL1_REG, MRVL_88X2222M_PHY_REG_LEN,
                                 XFI_PCS_CTRL1_VALUE, &phy_88X2222M)== FAILED) {
            return (FAILED);
        }
    }

    /* Enable loopback Bit, let xaui link up : Device 4, Register 0xf003 */
    if (write_ten_g_phy_reg(SERDES_CTRL_REG, MRVL_88X2222M_PHY_REG_LEN,
                             SERDES_CTRL_VALUE, &phy_88X2222M)== FAILED) {
        return (FAILED);
    }

    /* Deley to make xaui have enough time to link up */
    msleep(3000);

    /* Check xaui link status */
    /* Get the current device 4 register 0x1001 value */
    for (ix = 0; ix < RETRY_STICKY_BIT; ix++) {
        if (read_ten_g_phy_reg(XFI_PCS_STATUS_1_REG, MRVL_88X2222M_PHY_REG_LEN,
                                &reg_val, &phy_88X2222M) == FAILED) {
            cterr('f', 0, "Read 88X2222M dev 4 reg 0x1001 fail");
            return (FAILED);
        } 

        if (((reg_val & XFI_PCS_LINK_STATUS) >> 2) == 1) {
                prpass(testpass, "XAUI is link up");
                break;
        } else {
            if (ix < RETRY_STICKY_BIT) {
                msleep(1000);
                continue;
            } else {
                cterr('f', 0, "XAUI is not link up");
                return (FAILED);
            } 
        }
    }
    
    /* Change to Device 31 */
    phy_88X2222M.device_id = MRV88X2222M_REG_DEVICE_31;
    
    /*
     * Set up the internal loopback mode in 88X2222M chip:
     * Shallow Host Loopback -> Traffic from Cavium to Phy 88X2222M
     */

    /* Get the current device address 31 register 0xF400 value */
    if (read_ten_g_phy_reg(TRANS_SOUR_N_REG, MRVL_88X2222M_PHY_REG_LEN,
                            &reg_val, &phy_88X2222M) == FAILED) {
        return (FAILED);
    }

    /* Clear the Bits 11:8 and 3:0 */
    reg_val &= 0xF0F0;
    /* Bits 11:8 SFI line port 2 are in output idles status */
    reg_val |= (LINE_PORT_IDLE << 8);
    /* Bits 3:0 Select XFI port 0 to attach to SFI line port 0 */
    reg_val |= (SELECT_XFI_PORT_0);
    /* Write register value */
    if (write_ten_g_phy_reg(TRANS_SOUR_N_REG, MRVL_88X2222M_PHY_REG_LEN,
                             reg_val, &phy_88X2222M) == FAILED) {
        return (FAILED);
    }

    /* Get the current device address 31 register 0xF401 value */
    if (read_ten_g_phy_reg(TRANS_SOUR_M_REG, MRVL_88X2222M_PHY_REG_LEN,
                            &reg_val, &phy_88X2222M) == FAILED) {
        return (FAILED);
    }

    /* Clear the Bits 3:0 */
    reg_val &= 0xFFF0;
    /* Bits 3:0 Select SFI port 0 to attach to XFI line port 0 */
    reg_val |= (SELECT_SFI_PORT_0);
    /* Write register value */
    if (write_ten_g_phy_reg(TRANS_SOUR_M_REG, MRVL_88X2222M_PHY_REG_LEN,
                             reg_val, &phy_88X2222M)== FAILED) {
        return (FAILED);
    }

    /* Get the current device address 4 register 0xF003 value */
    phy_88X2222M.device_id = MRV88X2222M_REG_DEVICE_4;
    if (read_ten_g_phy_reg(SERDES_CTRL_REG, MRVL_88X2222M_PHY_REG_LEN,
                            &reg_val, &phy_88X2222M) == FAILED) {
        return (FAILED);
    }
    /* Clear the Bit 12 */
    reg_val &= HOST_LPBK_MASK;
    /* SERDES Control Register 1 Bit 12 = 1 to enable loopback */
    reg_val |= (EN_HOST_LPBK);
    /* Write register value */
    if (write_ten_g_phy_reg(SERDES_CTRL_REG, MRVL_88X2222M_PHY_REG_LEN,
                             reg_val, &phy_88X2222M)== FAILED) {
        return (FAILED);
    }

    /* Perform 88X2222M internal loopback test */
    if (xaui_lpbk_test(LOOP_EXT) == FAILED) {
        cterr('f', 0, "88X2222M internal loopback test failed");
        rc = FAILED;
    }

    /* SERDES Control Register 1 Bit 12 = 1 to disable loopback */
    reg_val &= ~(EN_HOST_LPBK);
    /* Write register value */
    if (write_ten_g_phy_reg(SERDES_CTRL_REG, MRVL_88X2222M_PHY_REG_LEN,
                             reg_val, &phy_88X2222M)== FAILED) {
        return (FAILED);
    }

    return (rc);
}

/******************************************************************************
 *
 * Function: xaui_88X2222M_external_loopback_test
 *
 * Description: This function perform the external loopback test from Cavium to
 *              88X2222M SFP+.
 *
 * Inputs      : None
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/
static int xaui_88X2222M_external_loopback_test (void)
{
    testname("Xaui %s loopback", "external");
    prpass(testpass, "");
    int ix;
    uint reg_val;

    if (!check_ext_lpbk_flag()) {
        prpass(testpass, "Test skipped. Ext. loopback diag flag is off. ");
        return (PASSED); /* external loopback is not set, skipped. */
    }

    /* Init 2222 */
    marvell_2222m_init();
    msleep(200);

    ten_g_phy_t phy_88X2222M_dev4 = {MRV88X2222M_REG_DEVICE_4,
                                MRVL_88X2222M_SMI2_PORT0_ADDR};

    /* Enable the external loopback setting of Phy 88X2222M */
    if (mrvl_88X2222M_en_ext_lpbk() == FAILED) {
        return (FAILED);
    }

    /* read twice because it's sticky bit */ 
    for (ix = 0; ix < RETRY_STICKY_BIT; ix++) {
        /* Check host side PCS link status -  Device 4 Register 0x1001 bit 2 */
        if (read_ten_g_phy_reg(XFI_PCS_STATUS_1_REG, MRVL_88X2222M_PHY_REG_LEN,
                           &reg_val, &phy_88X2222M_dev4) == FAILED) {
            cterr('f', 0, "Read 88X2222M dev 4 reg 0x1001 fail");
            return (FAILED);
        }
    
        if (((reg_val & 0x4) >> 2) == 1) {
            prpass(testpass, "Host side PCS is link up\n");
            break;
        } else { 
            if (ix == RETRY_LIMIT) {
                cterr('f', 0, "Host side PCS is not link up");
                return (FAILED);
            } else {
                msleep(1000);
                continue;
            }
        }
    }

    ten_g_phy_t phy_88X2222M_dev3 = {MRV88X2222M_REG_DEVICE_3,
                                MRVL_88X2222M_SMI2_PORT0_ADDR};

    /* read twice because it's sticky bit */ 
    for (ix = 0; ix < RETRY_STICKY_BIT; ix++) {
        /* Check line side PCS link status - Device 3, Register 0x0001 */
        if (read_ten_g_phy_reg(SFI_LINE_SIDE_PCS_STATUS_REG, MRVL_88X2222M_PHY_REG_LEN,
                           &reg_val, &phy_88X2222M_dev3) == FAILED) {
            cterr('f', 0, "Read 88X2222M dev 3 reg 0x0001 fail");
            return (FAILED);
        }

        if (((reg_val & 0x4) >> 2) == 1) {
            prpass(testpass, "Line side PCS is link up");
            break;
        } else {
            if (ix == RETRY_LIMIT) {
                cterr('f', 0, "Line side PCS is not link up");
                return (FAILED);
            } else {
                msleep(1000);
                continue;
            } 
        }
    }

    /* Perform 88X2222M external loopback test */
    if ((xaui_lpbk_test(LOOP_EXT)) == FAILED) {
        cterr('f', 0, "88X2222M external loopback test failed");
        return (FAILED);
    }

    return (PASSED);
}

/******************************************************************************
 *
 * Function: read_phy_88X2222M_ptp_reg
 *
 * Description: This function read the XAUI 88X2222M PTP registers
 *
 * Inputs      : None
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/
int read_phy_88X2222M_ptp_reg (int reg_addr, uint *data_lo, uint *data_hi)
{
    uint reg_val;
    int rd_timeout = 5000;

    ten_g_phy_t read_phy_88X2222M = {MRV88X2222M_REG_DEVICE_31,
                                     MRVL_88X2222M_SMI2_PORT0_ADDR};

    /* Enable MACSec before accessing MACSec/PTP registers */
    if (enable_mrvl2222m_macsec_power() == FAILED) {
        printf("Enable MACsec power failed\n");
        fflush(stdout);
        return (FAILED);
    }

    /* Get the current register vlaue */
    if ((read_ten_g_phy_reg(reg_addr, MRVL_88X2222M_PHY_REG_LEN, 
                            &reg_val, &read_phy_88X2222M)) == FAILED) {
        printf("Can't read register value at Reg_addr=0x%x", reg_addr);
        fflush(stdout);
        return (FAILED);
    }

    do {
        if ((read_ten_g_phy_reg(MRV2222P_INDIRECT_RD_ADDR, MRVL_88X2222M_PHY_REG_LEN, 
                                &reg_val, &read_phy_88X2222M)) == FAILED) {
            printf("Can't read register value at Reg_addr=0x%x", reg_addr);
            fflush(stdout);
            return (FAILED);
        }
        if (reg_val == reg_addr) {
            break;
        }
        msleep(1);
    } while (rd_timeout--);
    
    if (rd_timeout == 0) {
        printf("Timeout on reading Address field\n");
        fflush(stdout);
        return (FAILED);
    }

    if ((read_ten_g_phy_reg(MRV2222P_INDIRECT_RD_DATA_LO, MRVL_88X2222M_PHY_REG_LEN, 
                            data_lo, &read_phy_88X2222M)) == FAILED) {
        printf("Can't read register value at Reg_addr=0x%x", reg_addr);
        fflush(stdout);
        return (FAILED);
    }

    if ((read_ten_g_phy_reg(MRV2222P_INDIRECT_RD_DATA_HI, MRVL_88X2222M_PHY_REG_LEN, 
                            data_hi, &read_phy_88X2222M)) == FAILED) {
        printf("Can't read register value at Reg_addr=0x%x", reg_addr);
        fflush(stdout);
        return (FAILED);
    }
    
    return (PASSED);
}

/******************************************************************************
 *
 * Function: dump_phy_88X2222M_ptp_register
 *
 * Description: This function dumps the XAUI 88X2222M PTP registers
 *
 * Inputs      : None
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/
int dump_phy_88X2222M_ptp_register (void)
{
    int reg_addr;
    uint data_lo, data_hi;

    printf("Dump phy 88X2222M PTP Register.\n");

    reg_addr = gethex_answer("Enter register address (0x0~0xFFFF): ", 0, 0, 0xFFFF);

    /* Get the current register vlaue */
    if (read_phy_88X2222M_ptp_reg(reg_addr, &data_lo, &data_hi) == FAILED) {
        printf("Can't read register value at Reg_addr=0x%x", reg_addr);
        fflush(stdout);
        return (FAILED);
    }

    printf("register addr %#.2x, data low: %#.8x, "
           "data high: %#.8x\n", reg_addr, data_lo, data_hi);

    return (PASSED);
}

/******************************************************************************
 *
 * Function: dump_phy_88X2222M_registers
 *
 * Description: This function dumps the XAUI 88X2222M registers
 *
 * Inputs      : None
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/
int dump_phy_88X2222M_registers (void)
{
    int ix, port_num, reg_addr, dev_addr, bus_addr, phy_addr;
    uint reg_val;

    port_num = MRVL_88X2222M_PORTS;
    phy_addr = MRVL_88X2222M_PORT_0_ADDR;
    bus_addr = (MRVL_88X2222M_SMI2_ADDR << 4);

    printf("Dump phy 88X2222M Register.\n");
    dev_addr = gethex_answer("Enter device address (0x1,0x2,0x3,0x4,0x1E,0x1F): ", 0, 0, 0x1F);
    reg_addr = gethex_answer("Enter register address (0x0~0xFFFF): ", 0, 0, 0xFFFF);
    
    for (ix = 0; ix <= port_num; ix += 2) {
        ten_g_phy_t dump_phy_88X2222M = {dev_addr, (bus_addr | (phy_addr + ix))};

        /* Get the current register vlaue */
        if ((read_ten_g_phy_reg(reg_addr, MRVL_88X2222M_PHY_REG_LEN, 
                                &reg_val, &dump_phy_88X2222M)) == FAILED) {
            cterr('f', 0, "Can't read register value at port=%d, "
                  "dev_addr=%04x, reg_addr=%04x", ix, dev_addr, reg_addr);
            return (FAILED);
        }
        printf("Phy %#.2x reg %#.8x, data %#.8x\n", phy_addr + ix, reg_addr, reg_val);
    }

    return (PASSED);
}

/******************************************************************************
 *
 * Function: write_phy_88X2222M_ptp_reg
 *
 * Description: This function read the XAUI 88X2222M PTP registers
 *
 * Inputs      : None
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/
int write_phy_88X2222M_ptp_reg (int reg_addr, uint data_lo, uint data_hi)
{  
    ten_g_phy_t write_phy_88X2222M = {MRV88X2222M_REG_DEVICE_31,
                                      MRVL_88X2222M_SMI2_PORT0_ADDR};

    /* Enable MACSec before accessing MACSec/PTP registers */
    if (enable_mrvl2222m_macsec_power() == FAILED) {
        printf("Enable MACsec power failed\n");
        fflush(stdout);
        return (FAILED);
    }

    /* Alter the lower register with new vlaue */
    if ((write_ten_g_phy_reg(reg_addr, MRVL_88X2222M_PHY_REG_LEN,
                             data_lo, &write_phy_88X2222M))== FAILED) {
        printf("Can't alter register value at reg_addr=%04x", reg_addr);
        fflush(stdout);
        return (FAILED);
    }

    /* Alter the upper register with new vlaue */
    if ((write_ten_g_phy_reg(reg_addr + 1, MRVL_88X2222M_PHY_REG_LEN,
                             data_hi, &write_phy_88X2222M))== FAILED) {
        printf("Can't alter register value at reg_addr=%04x", reg_addr);
        fflush(stdout);
        return (FAILED);
    }

    return (PASSED);
}

/******************************************************************************
 *
 * Function: alter_phy_88X2222M_ptp_register
 *
 * Description: This function alters the XAUI 88X2222M PTP registers
 *
 * Inputs      : None
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/
int alter_phy_88X2222M_ptp_register (void)
{
    int reg_addr;
    uint data_lo, data_hi;

    printf("Alter phy 88X2222M PTP Register.\n");
    reg_addr = gethex_answer("Enter register address (0x0~0xFFFF): ", 0, 0, 0xFFFF);

    /* Get new current register vlaue */
    if (read_phy_88X2222M_ptp_reg(reg_addr, &data_lo, &data_hi) == FAILED) {
        printf("Can't read register value at reg_addr=%04x", reg_addr);
        fflush(stdout);
        return (FAILED);
    }

    printf("register addr %#.2x, data low: %#.8x, "
           "data high: %#.8x\n", reg_addr, data_lo, data_hi);
    fflush(stdout);

    /* Alter register with new value */
    data_lo = gethex_answer("Enter the new data low (hex): ", 0, 0, 0xFFFF);
    data_hi = gethex_answer("Enter the new data high (hex): ", 0, 0, 0xFFFF);

    /* Alter the PTP register with new vlaue */
    if (write_phy_88X2222M_ptp_reg (reg_addr, data_lo, data_hi) == FAILED) {
        printf("Can't alter register value at reg_addr=%04x", reg_addr);
        fflush(stdout);
        return (FAILED);
    }

    /* Get new current register vlaue */
    if (read_phy_88X2222M_ptp_reg(reg_addr, &data_lo, &data_hi) == FAILED) {
        printf("Can't alter register value at reg_addr=%04x", reg_addr);
        fflush(stdout);
        return (FAILED);
    }

    printf("After alter Register, reg %#.2x, data low: %#.8x, "
           "data high: %#.8x\n", reg_addr, data_lo, data_hi);
    fflush(stdout);

    return (PASSED);
}

/******************************************************************************
 *
 * Function: alter_phy_88X2222M_register
 *
 * Description: This function alters the XAUI 88X2222M registers
 *
 * Inputs      : None
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/
int alter_phy_88X2222M_register (void)
{
    int port_num, reg_addr, dev_addr, bus_addr, phy_addr;
    uint reg_val;

    port_num = MRVL_88X2222M_PORTS;
    bus_addr = (MRVL_88X2222M_SMI2_ADDR << 4);
    
    printf("Alter phy 88X2222M Register.\n");
    dev_addr = gethex_answer("Enter device address (0x1,0x2,0x3,0x4,0x1E,0x1F): ", 0, 1, 0x1F);
    reg_addr = gethex_answer("Enter register address (0x0~0xFFFF): ", 0, 0, 0xFFFF);
    phy_addr = gethex_answer("Enter phy address (0x4 or 0x6): ", 0x4, 0x4, 0x6);
    
    ten_g_phy_t alter_phy_88X2222M = {dev_addr, (bus_addr | (phy_addr))};

    /* Get the current register vlaue */
    if ((read_ten_g_phy_reg(reg_addr, MRVL_88X2222M_PHY_REG_LEN,
                            &reg_val, &alter_phy_88X2222M)) == FAILED) {
        cterr('f', 0, "Can't read register value at phy_addr=%d, "
              "dev_addr=%04x, reg_addr=%04x", phy_addr, dev_addr, reg_addr);
        return (FAILED);
    }
    
    printf("Original value: phy address %x reg %#.8x, data %#.8x\n",
            phy_addr, reg_addr, reg_val);

    /* Alter register with new value */
    reg_val = gethex_answer("Enter the new data (hex): ", 0, 0, 0xFFFF);

    /* Alter the current register with new vlaue */
    if ((write_ten_g_phy_reg(reg_addr, MRVL_88X2222M_PHY_REG_LEN,
                             reg_val, &alter_phy_88X2222M))== FAILED) {
        cterr('f', 0, "Can't alter register v alue at phy_addr=%d, "
                "dev_addr=%04x, reg_addr=%04x", phy_addr, dev_addr, reg_addr);
        return (FAILED);
    }

    /* Get new current register vlaue */
    if ((read_ten_g_phy_reg(reg_addr, MRVL_88X2222M_PHY_REG_LEN,
                            &reg_val, &alter_phy_88X2222M)) == FAILED) {
        cterr('f', 0, "Can't read the new register value at phy_addr=%d, "
                      "dev_addr=%04x, reg_addr=%04x",
                      phy_addr, dev_addr, reg_addr);
        return (FAILED);
    }

    printf("After alter Phy address %x Register value, reg %#.2x, data %#.8x\n",
            phy_addr, reg_addr, reg_val);

    return (PASSED);
}

/******************************************************************************
 *
 * Function: enter_prbs31_mode
 *
 * Description: This function force XAUI 88X2222M to enter PRBS31 mode.
 *
 * Inputs      : None
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/
static int enter_prbs31_mode (void)
{
    uint reg_val;

    printf("Let 88X2222M enter PRBS31 mode.\n");

    /* Assing 88X2222M SMI bus 2 Port 0 address device 3 register 0xf030 */
    ten_g_phy_t alter_phy_88X2222M = {MRV88X2222M_REG_DEVICE_3,
                                      MRVL_88X2222M_SMI2_PORT0_ADDR};

    /* Get the current PRBS Control Lane 0 register vlaue */
    if ((read_ten_g_phy_reg(PRBS_CONTROL_LANE_0, MRVL_88X2222M_PHY_REG_LEN,
                            &reg_val, &alter_phy_88X2222M)) == FAILED) {
        return (FAILED);
    }

    /* Clear PRBS Mode bit 3:0 register value */
    reg_val &= ~(PRBS_MODE_MASK);
    /* Set PRBS31 mode and enable checker/generator mode */
    reg_val |= (PRBS31_PATTERN_MODE | ENABLE_CHECKER | ENABLE_GENERATOR);

    /* Enter the PRBS 31 Mode */
    if ((write_ten_g_phy_reg(PRBS_CONTROL_LANE_0, MRVL_88X2222M_PHY_REG_LEN,
                             reg_val, &alter_phy_88X2222M))== FAILED) {
        return (FAILED);
    }

    return (PASSED);
}

/******************************************************************************
 *
 * Function: enter_prbs9_mode
 *
 * Description: This function force XAUI 88X2222M to enter PRBS9 mode.
 *
 * Inputs      : None
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/
static int enter_prbs9_mode (void)
{
    uint reg_val;

    printf("Let 88X2222M enter PRBS9 mode.\n");

    /* Assing 88X2222M SMI bus 2 Port 0 address device 3 register 0xf030 */
    ten_g_phy_t alter_phy_88X2222M = {MRV88X2222M_REG_DEVICE_3,
                                      MRVL_88X2222M_SMI2_PORT0_ADDR};

    /* Get the current PRBS Control Lane 0 register vlaue */
    if ((read_ten_g_phy_reg(PRBS_CONTROL_LANE_0, MRVL_88X2222M_PHY_REG_LEN,
                            &reg_val, &alter_phy_88X2222M)) == FAILED) {
        return (FAILED);
    }

    /* Clear PRBS Mode bit 3:0 register value */
    reg_val &= ~(PRBS_MODE_MASK);
    /* Set PRBS9 mode and enable checker/generator mode */
    reg_val |= (PRBS9_PATTERN_MODE | ENABLE_CHECKER | ENABLE_GENERATOR);

    /* Enter the PRBS 9 Mode */
    if ((write_ten_g_phy_reg(PRBS_CONTROL_LANE_0, MRVL_88X2222M_PHY_REG_LEN,
                             reg_val, &alter_phy_88X2222M))== FAILED) {
        return (FAILED);
    }

    return (PASSED);
}

static int enter_prbs9_inverted_mode (void)
{
    uint reg_val;

    printf("Let 88X2222M enter PRBS9 Inverted mode.\n");

    /* Assing 88X2222M SMI bus 2 Port 0 address device 3 register 0xf030 */
    ten_g_phy_t alter_phy_88X2222M = {MRV88X2222M_REG_DEVICE_3,
                                      MRVL_88X2222M_SMI2_PORT0_ADDR};

    /* Get the current PRBS Control Lane 0 register vlaue */
    if ((read_ten_g_phy_reg(PRBS_CONTROL_LANE_0, MRVL_88X2222M_PHY_REG_LEN,
                            &reg_val, &alter_phy_88X2222M)) == FAILED) {
        return (FAILED);
    }

    /* Clear PRBS Mode bit 3:0 register value */
    reg_val &= ~(PRBS_MODE_MASK);
    /* Set PRBS9 mode and enable checker/generator mode */
    reg_val |= (PRBS9_PATTERN_MODE | ENABLE_CHECKER | ENABLE_GENERATOR | 0x4);

    /* Enter the PRBS 9 Inverted Mode */
    if ((write_ten_g_phy_reg(PRBS_CONTROL_LANE_0, MRVL_88X2222M_PHY_REG_LEN,
                             reg_val, &alter_phy_88X2222M))== FAILED) {
        return (FAILED);
    }

    return (PASSED);
}

/******************************************************************************
 *
 * Function: enter_square_wave_mode
 *
 * Description: This function force XAUI 88X2222M to enter 8180 (Square Wave)
 *              mode.
 *
 * Inputs      : None
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/
static int enter_square_wave_mode (void)
{
    uint reg_val;

    printf("Let 88X2222M enter 8180(Square Wave) mode.\n");

    /* Assing 88X2222M SMI bus 2 Port 0 address device 3 register 0xf030 */
    ten_g_phy_t alter_phy_88X2222M = {MRV88X2222M_REG_DEVICE_3,
                                      MRVL_88X2222M_SMI2_PORT0_ADDR};

    /* Get the current PRBS Control Lane 0 register vlaue */
    if ((read_ten_g_phy_reg(PRBS_CONTROL_LANE_0, MRVL_88X2222M_PHY_REG_LEN,
                            &reg_val, &alter_phy_88X2222M)) == FAILED) {
        return (FAILED);
    }

    /* Clear PRBS Mode bit 3:0 register value */
    reg_val &= ~(PRBS_MODE_MASK);
    /* Set 8180(Wave Square) mode and enable checker/generator mode */
    reg_val |= (SQUARE_WAVE_PATTERN_MODE | ENABLE_CHECKER | ENABLE_GENERATOR);

    /* Enter the 8180 (Square Wave) Mode */
    if ((write_ten_g_phy_reg(PRBS_CONTROL_LANE_0, MRVL_88X2222M_PHY_REG_LEN,
                             reg_val, &alter_phy_88X2222M))== FAILED) {
        return (FAILED);
    }

    return (PASSED);
}

/******************************************************************************
 *
 * Function: disable_prbs_test
 *
 * Description: This function disable XAUI 88X2222M generator and checker bits.
 *              mode.
 *
 * Inputs      : None
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/
static int disable_prbs_test (void)
{
    uint reg_val;

    printf("Disable 88X2222M generator and checker bits.\n");

    /* Assing 88X2222M SMI bus 2 Port 0 address device 3 register 0xf030 */
    ten_g_phy_t alter_phy_88X2222M = {MRV88X2222M_REG_DEVICE_3,
                                      MRVL_88X2222M_SMI2_PORT0_ADDR};

    /* Get the current PRBS Control Lane 0 register vlaue */
    if ((read_ten_g_phy_reg(PRBS_CONTROL_LANE_0, MRVL_88X2222M_PHY_REG_LEN,
                            &reg_val, &alter_phy_88X2222M)) == FAILED) {
        return (FAILED);
    }

    /* Clear generator and checker bits value */
    reg_val &= ~(ENABLE_CHECKER | ENABLE_GENERATOR);

    /* Clear generator and checker bits value */
    if ((write_ten_g_phy_reg(PRBS_CONTROL_LANE_0, MRVL_88X2222M_PHY_REG_LEN,
                             reg_val, &alter_phy_88X2222M))== FAILED) {
        return (FAILED);
    }

    return (PASSED);
}

/***********************************************************************
 *  
 * Function: config_2222_gen_clk_out
 *    
 * Description: Configure 88X2222P to generate 8kHz clock
 *      
 * Inputs: None
 *        
 * Outputs: PASSED/FAILED
 *          
 *************************************************************************/
int config_2222_gen_clk_out (void)
{
    int phy_addr = 0x0;
    int sku_id;

    /* Get the SKU id */
    sku_id = get_sku_id();

    /* Not official SKU has different eth number copper port mapping compared
     * with new official SKU
     */
    if (sku_id == WOODLAWN_6GE) {
        printf("6GE port does not has SFP\n");
        fflush(stdout);
        return (PASSED);
    } else if (sku_id == WOODLAWN_4GE_1XAUI) {
        phy_addr = 0x4;
    }

    /* Generate clock out from 88X2222P */
    printf("Config phy_addr = 0x%x clock out\n", phy_addr);
    fflush(stdout);

    /* Specified the nanosecond part of the clock cycle for clock generation
     *  through clock_cyc address(0xBC58)
     */
    write_phy_88X2222M_ptp_reg (MRV2222P_CLOCK_CYC, 0x9000, 0X7);

    /* Enable internal clock generator generates a slow multi-device TOD alignment
     * clock signal with a clock cycle defined by <Clock Cycle>.
     * Generate an external multi-device TOD alignment clock signal.
     * through tod_cfg_gen address(0xBC16)
     */
    write_phy_88X2222M_ptp_reg (MRV2222P_TOD_CFG_GEN, 0x5, 0X0);

    msleep(PTP_CONFIG_DELAY);

    return (PASSED);
}


/***********************************************************************
 *  
 * Function: verify_2222_clk_trig_in
 *    
 * Description: Timing card clock and trigger verification 
 *      
 * Inputs: None
 *        
 * Outputs: PASSED/FAILED
 *          
 *************************************************************************/
int verify_2222_clk_trig_in (void)
{
    int ix;
    int phy_addr = 0x0;
    int bus_id;
    int sku_id;
    int trig_cnt = 0, clk_cnt = 0, trigger_count = 0, trigger_count_cmp = 0;
    int clock_count = 0, clock_count_cmp = 0;
    char fpga_val;
    uint data_lo, data_hi;

    /* Get the SKU id */
    sku_id = get_sku_id();

    /* Not official SKU has different eth number copper port mapping compared
     * with new official SKU
     */
    if (sku_id == WOODLAWN_6GE) {
        printf("6GE port does not has SFP\n");
        fflush(stdout);
        return (PASSED);
    } else if (sku_id == WOODLAWN_4GE_1XAUI) {
        phy_addr = 0x4;
    }

    for (ix = 0; ix < WAIT_PHY_READY; ix++) {
        if (fpga_reg_read(FPGA_RESET_SIGNAL_REG, &fpga_val) == FAILED) {
            printf("Read FPGA register %#.8x failed\n", FPGA_RESET_SIGNAL_REG);
            fflush(stdout);
            return (FAILED);
        }

        /* If bit 0 is "1" - Finish reset and PHY is ready */
        if ((fpga_val & GE_PHY_READY_MASK) == GE_PHY_READY) {
            break;
        }
        msleep(100);
    }

    if (ix == WAIT_PHY_READY) {
        printf("PHY is not ready\n");
        fflush(stdout);
        return (FAILED);
    }

    /* Check pulse_in counter */
    bus_id = MRVL_88X2222M_SMI2_ADDR;

    /* Pulse-in counter use bit 7:0 and max value is 0xff.
     * Need to clear pulse-in counter.
     */
    data_lo &= (~PTP_INIT_PULSE_IN_CNT);
    data_hi &= (~PTP_INIT_PULSE_IN_CNT);
    msleep(100);
    if ((write_phy_88X2222M_ptp_reg(MRV2222P_PULSE_IN_COUNT_REG,
                                    data_lo, data_hi)) == FAILED) {
        return (FAILED);
    }

    msleep(PTP_CNT_DELAY);

     /* Register 0xBC44 will keep count of the number of incoming pulse-triggers 
        when it sees a low to high transition */
    if ((read_phy_88X2222M_ptp_reg(MRV2222P_PULSE_IN_COUNT_REG,
                                   &data_lo, &data_hi)) == FAILED) {
        return (FAILED);
    }

    trigger_count = (data_hi << 16) | data_lo;

    for (ix = 0; ix < MRVL2222P_TRIG_VERIFY_TIME; ix++) {
        /* Read again pulse in counter */
        if ((read_phy_88X2222M_ptp_reg(MRV2222P_PULSE_IN_COUNT_REG,
                                       &data_lo, &data_hi)) == FAILED) {
            return (FAILED);
        }

        trigger_count_cmp = (data_hi << 16) | data_lo;

        if (trigger_count < trigger_count_cmp) {
            trigger_count = trigger_count_cmp;
            trig_cnt++;
        }

        if (trig_cnt == MRVL2222P_TRIG_VERIFY_NUM) {
            break;
        }

        msleep(MRVL2222P_PTP_READ_DELAY);
    }

    if (trig_cnt < MRVL2222P_TRIG_VERIFY_NUM) {
        printf("1pps trigger is not fed into PHY-0 successfully\n");
        fflush(stdout);
        return (FAILED);
    }

    /* Check clock_in counter */
    /* Clock-in counter use bit 31:0 to count the incoming clock.
     * Each time an incoming clock(rising edge) is received, this counter is incremented. 
     * Need to clear clock-in counter.
     */
    data_lo &= (~PTP_INIT_CLOCK_IN_CNT);
    data_hi &= (~PTP_INIT_CLOCK_IN_CNT);
    msleep(100);
    if ((write_phy_88X2222M_ptp_reg(MRV2222P_CLOCK_IN_COUNT_REG, 
                                    data_lo, data_hi)) == FAILED) {
        return (FAILED);
    }

    msleep(PTP_CNT_DELAY);

    /* Register 0xBC5C will keep count of the number of incoming clock 
        when it sees a low to high transition */
    if ((read_phy_88X2222M_ptp_reg(MRV2222P_CLOCK_IN_COUNT_REG, 
                                   &data_lo, &data_hi)) == FAILED) {
        return (FAILED);
    }

    clock_count = (data_hi << 16) | data_lo;

    for (ix = 0; ix < MRVL2222P_CLK_VERIFY_TIME; ix++) {
        /* Read again pulse in counter */
        if ((read_phy_88X2222M_ptp_reg(MRV2222P_CLOCK_IN_COUNT_REG, 
                                       &data_lo, &data_hi)) == FAILED) {
            return (FAILED);
        }

        clock_count_cmp = (data_hi << 16) | data_lo;

        if (clock_count < clock_count_cmp) {
            clock_count = clock_count_cmp;
            clk_cnt++;
        }
           
        if (clk_cnt == MRVL2222P_CLK_VERIFY_NUM) {
            break;
        }
            
        msleep(MRVL2222P_PTP_READ_DELAY);
    }

    if (clk_cnt < MRVL2222P_CLK_VERIFY_NUM) {
        printf("Clock is not fed into 88X222P successfully\n");
        fflush(stdout);
        return (FAILED);
    }

    return (PASSED);
}


/***********************************************************************
 *  
 * Function: config_2222_gen_trig_out
 *    
 * Description: Configure 88X2222P to generate 1PPS trigger
 *      
 * Inputs: None
 *        
 * Outputs: PASSED/FAILED
 *          
 *************************************************************************/
int config_2222_gen_trig_out (void)
{
    int phy_addr = 0x0;
    int sku_id;

    /* Get the SKU id */
    sku_id = get_sku_id();

    /* Not official SKU has different eth number copper port mapping compared
     * with new official SKU
     */
    if (sku_id == WOODLAWN_6GE) {
        printf("6GE port does not has SFP\n");
        fflush(stdout);
        return (PASSED);
    } else if (sku_id == WOODLAWN_4GE_1XAUI) {
        phy_addr = 0x4;
    }

    /* Generate clock out from 88X2222P */
    printf("Config phy_addr = 0x%x trigger out\n", phy_addr);
    fflush(stdout);

    /* Enable TriGen and set pulse width (Global TAI register)
     * Set the timer operation to capture mode to prevent 88X2222P
     * receive unexpected external trigger and do the unexpected action.
     * through tod_func_cfg address(0xBC46)
     */
    write_phy_88X2222M_ptp_reg (MRV2222P_TOD_FUNC_CFG, 0X0000, 0Xe100);

    /* Unmask TriGen TOD Mask (Global TAI register) */
    write_phy_88X2222M_ptp_reg (MRV2222P_TRIG_GEN_MASK2, 0X0, 0X0);

    /* Program TrigGen TOD (whenever TOD = TrigGen TOD, a pulse will be sent out)
     * (Global TAI register)
     */
    write_phy_88X2222M_ptp_reg (MRV2222P_TRIG_GEN_TOD0, 0X0, 0X0);
    write_phy_88X2222M_ptp_reg (MRV2222P_TRIG_GEN_TOD1, 0X0, 0X0);
    write_phy_88X2222M_ptp_reg (MRV2222P_TRIG_GEN_TOD2, 0X1, 0X0);
    write_phy_88X2222M_ptp_reg (MRV2222P_TRIG_GEN_TOD3, 0X0, 0X0);

    msleep(PTP_CONFIG_DELAY);

    return (PASSED);
}


/*-------------------------------------------------
 * $Log: diag_xaui_88X2222M_test.c,v $
 * Revision 1.6  2015/02/14 12:48:41  kodko
 * Collapse timing card branch code into main trunk.
 *
 * Revision 1.5  2015/02/04 07:23:12  leschen
 * Fix for sfp+ speed led control.
 *
 * Revision 1.4  2014/02/21 04:00:29  leschen
 * Add check external lpbk flag function into XAUI loopback code.
 *
 * Revision 1.3.2.3  2014/05/13 02:23:37  kodko
 * Change TOD trigger mode from update to capture.
 *
 * Revision 1.3.2.2  2014/05/05 07:27:34  kodko
 * Add comment for config gen clk/trig function.
 *
 * Revision 1.3.2.1  2014/04/30 13:47:22  kodko
 * Support 1548P/2222P clock/trigger in/out verification.
 *
 * Revision 1.3  2013/12/12 09:13:39  leschen
 * CSCul71044:Run Marvell 2222 init script before loopback test
 *
 * Revision 1.2  2013/10/08 08:48:29  tirawan
 * Woodlawn collapsed to main trunk
 *
 * Revision 1.1.4.3  2013/09/04 02:51:52  leschen
 * Add PRBS9 inverted mode utility
 *
 * Revision 1.1.4.2  2013/08/20 10:58:54  tirawan
 * Branch into woodlawn-branch2 and port woodlawn code
 *
 * Revision 1.1.2.5  2013/06/18 02:00:37  leschen
 * Show check link status when do internal lpbk
 *
 * Revision 1.1.2.4  2013/06/17 10:40:56  leschen
 * According phy rev to do soft reset for internal loopback
 *
 * Revision 1.1.2.3  2013/06/07 09:59:54  leschen
 * Check 2222 host side and line side link status
 *
 * Revision 1.1.2.2  2013/05/17 05:55:03  leschen
 * Fix external loopback fail issue when unplug/plug loopback cable
 *
 * Revision 1.1.2.1  2013/04/24 10:37:19  tirawan
 * Initial check-in for woodlawn linux code
 *
 * Revision 1.3  2013/04/10 09:48:48  kuangik
 * Implement do all wrapper for host run sm test
 *
 * Revision 1.2  2013/03/27 04:49:36  kuangik
 * Code cleanup after adding -Wall
 *
 * Revision 1.17  2013/03/12 11:24:32  leslie
 * Fix submenu flag for show error number message
 *
 * Revision 1.16  2013/03/08 07:56:04  kuangik
 * Remove prpass
 *
 * Revision 1.15  2012/12/11 01:00:23  leslie
 * Fix internal unstable issue and add check link mechanism.
 *
 * Revision 1.14  2012/11/19 02:39:26  leslie
 * Add function 2222m sfi compliance testing to utility.
 *
 * Revision 1.13  2012/10/24 10:40:29  leslie
 * Fix and clean up code.
 *
 * Revision 1.12  2012/10/03 06:04:33  kody
 * Disable the X2222 internal loopback bift after test.
 *
 * Revision 1.11  2012/09/21 11:47:56  kody
 * Fix 88X2222M register test.
 *
 * Revision 1.10  2012/09/05 22:43:37  leslie
 * Update for xaui 88X2222M test.
 *
 * Revision 1.9  2012/08/03 10:16:55  evanli
 * Mapping to latest O2 source code on 20120726
 *
 * Revision 1.7  2012/07/19 03:33:41  leslie
 * Modify err to cterr.
 *
 * Revision 1.5  2012/07/09 08:49:32  kody
 * Add Phy 2222M PRBS test mode in utilities.
 *
 * Revision 1.4  2012/05/18 10:22:27  kody
 * Add 88X2222M internal and external loopback test.
 *
 * Revision 1.3  2012/05/16 03:29:16  leslie
 * Update code
 *
 * Revision 1.2  2012/05/15 01:19:14  leslie
 * Add Marvel XAUI 88X2222M register and utility test
 *
 * Revision 1.1  2012/04/16 02:39:42  kody
 * Add Marvell XAUI 88X2222M test.
 *
 * $Endlog$
 *-------------------------------------------------
 */

