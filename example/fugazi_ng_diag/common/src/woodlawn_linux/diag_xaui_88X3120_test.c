/* $Id: diag_xaui_88X3120_test.c,v 1.2 2013/10/08 08:48:29 tirawan Exp $ 
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/woodlawn_linux/diag_xaui_88X3120_test.c,v $
 *-----------------------------------------------------------------------------
 * diag_xaui_88X3120_test.c - Menu for Woodlawn PHY 88X3120
 *
 * February 2012, Leslie Chen
 * Copyright (c) 2013 by Cisco Systems, Inc.
 * All rights reserved.
 *-----------------------------------------------------------------------------
 */
#include "error.h"
#include "common.h"
#include "types.h"
#include "menu.h"
#include "queryflags.h"
#include <stdio.h>
#include "defs.h"
#include "diag_xaui_88X3120_lib.h"
#include "diag_xaui_88X2222M_lib.h"
#include "platform_xaui.h"
#include "platform_smi.h"
#include "platform_smi_lib.h"
#include "common_utils.h"
#include "diag_fpga_lib.h"

#define MARVELL_PHY_RONLY    (READ_ONLY | REG_ACCESS)
#define MARVEL_PHY_RW        (READ_WRITE | REG_ACCESS)

/******************************************************************************
 *  Global Variable
 *****************************************************************************/
extern unsigned char phy_3120_fw_hdr[];
extern unsigned long phy_3120_fw_hdr_size;

extern unsigned char phy_3120_fw_bin[];
extern unsigned long phy_3120_fw_bin_size;

/*****************************************************************************
 *  Functions Declaration
 *****************************************************************************/

extern void msleep(unsigned long);

int xaui_88X3120_test(int);
static int xaui_88X3120_utility(int);
static int xaui_88X3120_register_test(int);
static int xaui_88X3120_internal_loopback_test(void);
static int xaui_88X3120_external_loopback_test(void);
static int dump_phy_88X3120_registers(void);
static int alter_phy_88X3120_register(void);
static int phy_88X3120_fw_download(void);
static int pma_test_modes(void);
static int read_ten_g_phy_reg_fn(ulong, int, ulong *, void *);
static int write_ten_g_phy_reg_fn(ulong, int, ulong, void *);

ten_g_phy_t ten_g_phy_dev1_port_0 = {MRV88X3120_REG_DEVICE_1, MRVL_88X3120_SMI2_PORT0_ADDR};
ten_g_phy_t ten_g_phy_dev3_port_0 = {MRV88X3120_REG_DEVICE_3, MRVL_88X3120_SMI2_PORT0_ADDR};
ten_g_phy_t ten_g_phy_dev4_port_0 = {MRV88X3120_REG_DEVICE_4, MRVL_88X3120_SMI2_PORT0_ADDR};
ten_g_phy_t ten_g_phy_dev7_port_0 = {MRV88X3120_REG_DEVICE_7, MRVL_88X3120_SMI2_PORT0_ADDR};
ten_g_phy_t ten_g_phy_dev29_port_0 = {MRV88X3120_REG_DEVICE_29, MRVL_88X3120_SMI2_PORT0_ADDR};
ten_g_phy_t ten_g_phy_dev30_port_0 = {MRV88X3120_REG_DEVICE_30, MRVL_88X3120_SMI2_PORT0_ADDR};
ten_g_phy_t ten_g_phy_dev31_port_0 = {MRV88X3120_REG_DEVICE_31, MRVL_88X3120_SMI2_PORT0_ADDR};

ten_g_phy_t ten_g_phy_dev1_port_1 = {MRV88X3120_REG_DEVICE_1, MRVL_88X3120_SMI2_PORT1_ADDR};
ten_g_phy_t ten_g_phy_dev3_port_1 = {MRV88X3120_REG_DEVICE_3, MRVL_88X3120_SMI2_PORT1_ADDR};
ten_g_phy_t ten_g_phy_dev4_port_1 = {MRV88X3120_REG_DEVICE_4, MRVL_88X3120_SMI2_PORT1_ADDR};
ten_g_phy_t ten_g_phy_dev7_port_1 = {MRV88X3120_REG_DEVICE_7, MRVL_88X3120_SMI2_PORT1_ADDR};
ten_g_phy_t ten_g_phy_dev29_port_1 = {MRV88X3120_REG_DEVICE_29, MRVL_88X3120_SMI2_PORT1_ADDR};
ten_g_phy_t ten_g_phy_dev30_port_1 = {MRV88X3120_REG_DEVICE_30, MRVL_88X3120_SMI2_PORT1_ADDR};
ten_g_phy_t ten_g_phy_dev31_port_1 = {MRV88X3120_REG_DEVICE_31, MRVL_88X3120_SMI2_PORT1_ADDR};

 /***********************************************************************
 *  88X3120 reg_info_t extension - for "PORT 0"
 ************************************************************************/
/* reg_info_t extension for "device 1" access */
static reg_info_t_ext phy_88X3120_port_0_ext1 = {MRVL_88X3120_PHY_REG_LEN,
                                                 read_ten_g_phy_reg_fn,
                                                 write_ten_g_phy_reg_fn,
                                                 &ten_g_phy_dev1_port_0};
#ifdef NOT_USED_WARNING
/* reg_info_t extension for "device 3" access */
static reg_info_t_ext phy_88X3120_port_0_ext3 = {MRVL_88X3120_PHY_REG_LEN,
                                                 read_ten_g_phy_reg_fn,
                                                 write_ten_g_phy_reg_fn,
                                                 &ten_g_phy_dev3_port_0};
/* reg_info_t extension for "device 4" access */
static reg_info_t_ext phy_88X3120_port_0_ext4 = {MRVL_88X3120_PHY_REG_LEN,
                                                 read_ten_g_phy_reg_fn,
                                                 write_ten_g_phy_reg_fn,
                                                 &ten_g_phy_dev4_port_0};
/* reg_info_t extension for "device 7" access */
static reg_info_t_ext phy_88X3120_port_0_ext7 = {MRVL_88X3120_PHY_REG_LEN,
                                                 read_ten_g_phy_reg_fn,
                                                 write_ten_g_phy_reg_fn,
                                                 &ten_g_phy_dev7_port_0};
/* reg_info_t extension for "device 29" access */
static reg_info_t_ext phy_88X3120_port_0_ext29 = {MRVL_88X3120_PHY_REG_LEN,
                                                  read_ten_g_phy_reg_fn,
                                                  write_ten_g_phy_reg_fn,
                                                  &ten_g_phy_dev29_port_0};
/* reg_info_t extension for "device 30" access */
static reg_info_t_ext phy_88X3120_port_0_ext30 = {MRVL_88X3120_PHY_REG_LEN,
                                                  read_ten_g_phy_reg_fn,
                                                  write_ten_g_phy_reg_fn,
                                                  &ten_g_phy_dev30_port_0};
/* reg_info_t extension for "device 31" access */
static reg_info_t_ext phy_88X3120_port_0_ext31 = {MRVL_88X3120_PHY_REG_LEN,
                                                  read_ten_g_phy_reg_fn,
                                                  write_ten_g_phy_reg_fn,
                                                  &ten_g_phy_dev31_port_0};
#endif

 /***********************************************************************
 *  88X3120 reg_info_t extension - for "PORT 1"
 ************************************************************************/
#ifdef NOT_USED_WARNING
/* reg_info_t extension for "device 1" access */
static reg_info_t_ext phy_88X3120_port_1_ext1 = {MRVL_88X3120_PHY_REG_LEN,
                                                 read_ten_g_phy_reg_fn,
                                                 write_ten_g_phy_reg_fn,
                                                 &ten_g_phy_dev1_port_1};
/* reg_info_t extension for "device 3" access */
static reg_info_t_ext phy_88X3120_port_1_ext3 = {MRVL_88X3120_PHY_REG_LEN,
                                                 read_ten_g_phy_reg_fn,
                                                 write_ten_g_phy_reg_fn,
                                                 &ten_g_phy_dev3_port_1};
/* reg_info_t extension for "device 4" access */
static reg_info_t_ext phy_88X3120_port_1_ext4 = {MRVL_88X3120_PHY_REG_LEN,
                                                 read_ten_g_phy_reg_fn,
                                                 write_ten_g_phy_reg_fn,
                                                 &ten_g_phy_dev4_port_1};
/* reg_info_t extension for "device 7" access */
static reg_info_t_ext phy_88X3120_port_1_ext7 = {MRVL_88X3120_PHY_REG_LEN,
                                                 read_ten_g_phy_reg_fn,
                                                 write_ten_g_phy_reg_fn,
                                                 &ten_g_phy_dev7_port_1};
/* reg_info_t extension for "device 29" access */
static reg_info_t_ext phy_88X3120_port_1_ext29 = {MRVL_88X3120_PHY_REG_LEN,
                                                  read_ten_g_phy_reg_fn,
                                                  write_ten_g_phy_reg_fn,
                                                  &ten_g_phy_dev29_port_1};
/* reg_info_t extension for "device 30" access */
static reg_info_t_ext phy_88X3120_port_1_ext30 = {MRVL_88X3120_PHY_REG_LEN,
                                                  read_ten_g_phy_reg_fn,
                                                  write_ten_g_phy_reg_fn,
                                                  &ten_g_phy_dev30_port_1};
/* reg_info_t extension for "device 31" access */
static reg_info_t_ext phy_88X3120_port_1_ext31 = {MRVL_88X3120_PHY_REG_LEN,
                                                  read_ten_g_phy_reg_fn,
                                                  write_ten_g_phy_reg_fn,
                                                  &ten_g_phy_dev31_port_1};
#endif

 /***********************************************************************
 *  88X3120 table - for "PORT 0"
 ************************************************************************/
/* reg_table for "device 1" */
static reg_info_t marvell_88X3120_port_0_dev1[] = {   /* Device 1*/
    {"LED Control", 0xC007, MARVEL_PHY_RW,
     {(unsigned long)&phy_88X3120_port_0_ext1}, 0xFFF0, 0x0001},
    {"end", 0x00, 0, {0}, 0, 0},
};

#ifdef NOT_USED_WARNING
/* reg_table for "device 3" */
static reg_info_t marvell_88X3120_port_0_dev3[] = {   /* Device 3*/
    {"PCS Control 1", 0x0000, MARVEL_PHY_RW,
     {(unsigned long)&phy_88X3120_port_0_ext3}, 0x4000, 0x2040},
    {"end", 0x00, 0, {0}, 0, 0},
};

/* reg_table for "device 4" */
static reg_info_t marvell_88X3120_port_0_dev4[] = {   /* Device 4*/
    {"10G XGXS Control", 0x0019, MARVEL_PHY_RW,
     {(unsigned long)&phy_88X3120_port_0_ext4}, 0x0004, 0x0000},
    {"end", 0x00, 0, {0}, 0, 0},
};

/* reg_table for "device 7" */
static reg_info_t marvell_88X3120_port_0_dev7[] = {   /* Device 7*/
    {"AN Control", 0x0000, MARVEL_PHY_RW,
     {(unsigned long)&phy_88X3120_port_0_ext7}, 0x1200, 0x3000},
    {"end", 0x00, 0, {0}, 0, 0},
};

/* reg_table for "device 29" */
static reg_info_t marvell_88X3120_port_0_dev29[] = {   /* Device 29*/
    {"Collision Test ", 0xC000, MARVEL_PHY_RW,
     {(unsigned long)&phy_88X3120_port_0_ext29}, 0x0080, 0x0000},
    {"end", 0x00, 0, {0}, 0, 0},
};

/* reg_table for "device 30" */
static reg_info_t marvell_88X3120_port_0_dev30[] = {   /* Device 30*/
    {"XFI Control 1", 0x0000, MARVEL_PHY_RW,
     {(unsigned long)&phy_88X3120_port_0_ext30}, 0x0001, 0x4040},
    {"end", 0x00, 0, {0}, 0, 0},
};

/* reg_table for "device 31" */
static reg_info_t marvell_88X3120_port_0_dev31[] = {   /* Device 31*/
    {"XFI PCS Control 1", 0x0000, MARVEL_PHY_RW,
     {(unsigned long)&phy_88X3120_port_0_ext31}, 0x4000, 0x4040},
    {"end", 0x00, 0, {0}, 0, 0},
};
#endif

/***********************************************************************
 *  88X3120 table - for "PORT 1"
 ************************************************************************/
#ifdef NOT_USED_WARNING
/* reg_table for "device 1" */
static reg_info_t marvell_88X3120_port_1_dev1[] = {   /* Device 1*/
    {"LED Control", 0xC007, MARVEL_PHY_RW,
     {(unsigned long)&phy_88X3120_port_1_ext1}, 0xFFF0, 0x0000},
    {"end", 0x00, 0, {0}, 0, 0},
};

/* reg_table for "device 3" */
static reg_info_t marvell_88X3120_port_1_dev3[] = {   /* Device 3*/
    {"PCS Control 1", 0x0000, MARVEL_PHY_RW,
     {(unsigned long)&phy_88X3120_port_1_ext3}, 0x4000, 0x2040},
    {"end", 0x00, 0, {0}, 0, 0},
};

/* reg_table for "device 4" */
static reg_info_t marvell_88X3120_port_1_dev4[] = {   /* Device 4*/
    {"10G XGXS Control", 0x0019, MARVEL_PHY_RW,
     {(unsigned long)&phy_88X3120_port_1_ext4}, 0x0004, 0x0000},
    {"end", 0x00, 0, {0}, 0, 0},
};

/* reg_table for "device 7" */
static reg_info_t marvell_88X3120_port_1_dev7[] = {   /* Device 7*/
    {"AN Control", 0x0000, MARVEL_PHY_RW,
     {(unsigned long)&phy_88X3120_port_1_ext7}, 0x1200, 0x3000},
    {"end", 0x00, 0, {0}, 0, 0},
};

/* reg_table for "device 29" */
static reg_info_t marvell_88X3120_port_1_dev29[] = {   /* Device 29*/
    {"Collision Test ", 0xC000, MARVEL_PHY_RW,
     {(unsigned long)&phy_88X3120_port_1_ext29}, 0x0080, 0x0000},
    {"end", 0x00, 0, {0}, 0, 0},
};

/* reg_table for "device 30" */
static reg_info_t marvell_88X3120_port_1_dev30[] = {   /* Device 30*/
    {"XFI Control 1", 0x0000, MARVEL_PHY_RW,
     {(unsigned long)&phy_88X3120_port_1_ext30}, 0x0001, 0x4040},
    {"end", 0x00, 0, {0}, 0, 0},
};

/* reg_table for "device 31" */
static reg_info_t marvell_88X3120_port_1_dev31[] = {   /* Device 31*/
    {"XFI PCS Control 1", 0x0000, MARVEL_PHY_RW,
     {(unsigned long)&phy_88X3120_port_1_ext31}, 0x4000, 0x4040},
    {"end", 0x00, 0, {0}, 0, 0},
};
#endif

/******************************************************************************
 *  List of Menu used for XAUI 88X3120
 *****************************************************************************/
static submenu_xtable_t xaui_88X3120_tests_submenu_table[] = {
   {"PHY 88X3120 Utility", (type_t(*)())xaui_88X3120_utility,   FALSE,
       0, NULL, 0, (type_t(*)())xaui_88X3120_utility,   TRUE}, 
   {"PHY 88X3120 Register Test", (type_t(*)())xaui_88X3120_register_test,   0,
       MF_CONTINUOUS | MF_DOGRP | MF_DOALL, (type_t(*)())0, 0, (type_t(*)())0,   0},
   {"PHY 88X3120 Internal Loopback Test", (type_t(*)())xaui_88X3120_internal_loopback_test, 0,
       MF_CONTINUOUS | MF_DOGRP | MF_DOALL, (type_t(*)())0, 0, (type_t(*)())0,   0},
   {"PHY 88X3120 Copper External Loopback Test", (type_t(*)())xaui_88X3120_external_loopback_test, 0,
       MF_CONTINUOUS | MF_DOGRP | MF_DOALL, (type_t(*)())0, 0, (type_t(*)())0,   0},
};

#define XAUI_88X3120_TESTS_SUBMENU_TABLE_SIZE (sizeof(xaui_88X3120_tests_submenu_table) / \
                                    sizeof(submenu_xtable_t))

/******************************************************************************
 * Primary & secondary submenu items (filled in from xtable)
 *****************************************************************************/
static mitem_t xaui_88X3120_tests_primary_items[XAUI_88X3120_TESTS_SUBMENU_TABLE_SIZE +
                                     MAX_BASE_ITEMS];
static mitem_t xaui_88X3120_tests_secondary_items[XAUI_88X3120_TESTS_SUBMENU_TABLE_SIZE +
                                     MAX_BASE_ITEMS];

menuinfo_t xaui_88X3120_subtest_menu = {
    "%s Subtest Menu",
    0,                                /* mtparam added by init_empty_menu */
    (PFT)show_endnote,                /* notes missing WICs in combos */
    0,                                /* use generic prompt */
    0,                                /* size (bumped by add_menu_item() */
    xaui_88X3120_tests_primary_items,
};
menuinfo_t *xaui_88X3120_submenup = &xaui_88X3120_subtest_menu;

/* List of XAUI 88X3120 Utilities */
static submenu_xtable_t xaui_88X3120_util_items[] = {
    {"Dump PHY 88X3120 Registers", (type_t(*)())dump_phy_88X3120_registers, 0, 0,
     (type_t(*)())0, 0, (type_t(*)())0, 0},
    {"Alter PHY 88X3120 Register", (type_t(*)())alter_phy_88X3120_register, 0, 0,
     (type_t(*)())0, 0, (type_t(*)())0, 0},
    {"PHY 88X3120 FW Download", (type_t(*)())phy_88X3120_fw_download, 0, 0,
     (type_t(*)())0, 0, (type_t(*)())0, 0},
    {"Enter PMA Test Modes", (type_t(*)())pma_test_modes, 0, 0,
     (type_t(*)())0, 0, (type_t(*)())0, 0},
};
#define XAUI_88X3120_TESTS_UTIL_SIZE (sizeof(xaui_88X3120_util_items) / \
                                    sizeof(submenu_xtable_t))

/******************************************************************************
 * XAUI 88X3120 util items (filled in from xtable)
 *****************************************************************************/
static mitem_t xaui_88X3120_tests_primary_util_items[XAUI_88X3120_TESTS_UTIL_SIZE +
                                     MAX_BASE_ITEMS];
static mitem_t xaui_88X3120_tests_secondary_util_items[XAUI_88X3120_TESTS_UTIL_SIZE +
                                     MAX_BASE_ITEMS];

/******************************************************************************
 * XAUI 88X3120 Utils submenu
 *****************************************************************************/
menuinfo_t xaui_88X3120_util_menu = {
    "XAUI 88X3120 Utility Menu",
    0,
    (PFT)show_endnote,
    0,
    0,
    xaui_88X3120_tests_primary_util_items,
};
menuinfo_t *xaui_88X3120_util_menup = &xaui_88X3120_util_menu;

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
 *****************************************************************************/
int xaui_88X3120_test (int show_menu)
{
    build_primary_submenu(xaui_88X3120_tests_submenu_table,
                            XAUI_88X3120_TESTS_SUBMENU_TABLE_SIZE,
                            "XAUI 88X3120", &xaui_88X3120_submenup);
    build_secondary_submenu(xaui_88X3120_tests_submenu_table,
                            XAUI_88X3120_TESTS_SUBMENU_TABLE_SIZE,
                            xaui_88X3120_tests_secondary_items);

    if (show_menu) {
        menu(xaui_88X3120_submenup, xaui_88X3120_tests_secondary_items, '\0' );
    } else {
        menu_exec_doall_diags(xaui_88X3120_submenup);
        prcomplete(testpass, errcount, (char *)0);
    }
      
        return (PASSED);
}

/*******************************************************************************
 *
 * Function    : xaui_88X3120_utility
 * Description :
 * Inputs      : menu_option - display menu instead of running all XAUI 88X3120
 *               tests.
 * Outputs     : PASSED / FAILED
 *
 ******************************************************************************/
int xaui_88X3120_utility (int show_menu)
{

    build_primary_submenu(xaui_88X3120_util_items, XAUI_88X3120_TESTS_UTIL_SIZE,
                          "XAUI 88X3120 Utilities Menu", &xaui_88X3120_util_menup);
    build_secondary_submenu(xaui_88X3120_util_items, XAUI_88X3120_TESTS_UTIL_SIZE,
                            xaui_88X3120_tests_secondary_util_items);

    if (show_menu) {
        menu(xaui_88X3120_util_menup, xaui_88X3120_tests_secondary_util_items, '\0' );
    } else {
        menu_exec_doall_diags(xaui_88X3120_util_menup);
        prcomplete(testpass, errcount, (char *)0);
    }

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
 * Function: xaui_88X3120_register_test
 *
 * Description: This function performs the XAUI 88X3210 register test.
 *
 * Inputs      : None
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/
static int xaui_88X3120_register_test (int port)
{
    testname("88X3120 PHY %d", port);

/***********************************************************************
 *  88X3120 register test - for "PORT 0"
************************************************************************/
    if (register_tests(0, marvell_88X3120_port_0_dev1) == FAILED) {
        return (FAILED);
    }
    
    prpass(testpass, "88X3120 Register Test Success");

    return (PASSED);
}

/******************************************************************************
 *
 * Function: xaui_88X3120_internal_loopback_test
 *
 * Description: This function perform the internal loopback test from Cavium
 *              through 88X2222M to 88X3120.
 *
 * Inputs      : None
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/
static int xaui_88X3120_internal_loopback_test (void)
{
    uint reg_val;
    int rc = PASSED;

    testname("Xaui %s loopback", "internal");
    prpass(testpass, "");

    ten_g_phy_t phy_88X3120 = {MRV88X3120_REG_DEVICE_30,
                               MRVL_88X3120_SMI2_PORT0_ADDR};

    /* Enable the external loopback setting of Phy 88X2222M */
    if (mrvl_88X2222M_sel_port2() == FAILED) {
        return (FAILED);
    }

    msleep(1000);

    /* Write SMI device id 30 port 0 register offset 0xc002 value 0x0005
     * Suggestion from Marvel to link up with 88X2222M
     * Decrease XFI transmit amplitude: write 1e.0xc002 = 0x0005
     */
    /* Write register value */
    if ((write_ten_g_phy_reg(0xc002, MRVL_88X3120_PHY_REG_LEN,
                             0x0005, &phy_88X3120)) == FAILED) {
        return (FAILED);
    }

    msleep(1000);

    phy_88X3120.device_id = MRV88X3120_REG_DEVICE_1;

    /*
     * Set up the PMA loopback mode in 88X3120 chip:
     * Cavium CPU -> Phy 88X2222M -> 88X3120
     */

    /* Get the current device address 0x1 register offset 0x0 value */
    if ((read_ten_g_phy_reg(PMA_PMD_CTRL_1_REG, MRVL_88X3120_PHY_REG_LEN,
                                      &reg_val, &phy_88X3120)) == FAILED) {
        return (FAILED);
    }

    /* Select PMA/PMD Control 1 (1.0) and set Bit 0 to 1 (Enable PMA loopback) */
    reg_val |= (EN_PMA_LPBK_MODE);
    /* Write register value */
    if ((write_ten_g_phy_reg(PMA_PMD_CTRL_1_REG, MRVL_88X3120_PHY_REG_LEN,
                             reg_val, &phy_88X3120)) == FAILED) {
        return (FAILED);
    }

    msleep(1000);

    /* Perform 88X3120 internal loopback test */
    if ((xaui_lpbk_test(LOOP_EXT)) == FAILED) {
        cterr('f', 0, "88X3120 internal loopback test failed");
        rc = FAILED;
    }

    /* Clear PMA loopback mode */
    reg_val &= (~EN_PMA_LPBK_MODE);
    /* Write register value */
    if ((write_ten_g_phy_reg(PMA_PMD_CTRL_1_REG, MRVL_88X3120_PHY_REG_LEN,
                             reg_val, &phy_88X3120)) == FAILED) {
        return (FAILED);
    }

    return (rc);
}

/******************************************************************************
 *
 * Function: xaui_88X3120_external_loopback_test
 *
 * Description: This function perform the external loopback test from Cavium to
 *              through 88X2222M to 88X3120 10G BASE-T MagJack.
 *
 * Inputs      : None
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/
static int xaui_88X3120_external_loopback_test (void)
{
    uint reg_val, ix;

    testname("Xaui %s loopback", "external");
    prpass(testpass, "");

    ten_g_phy_t phy_88X3120 = {MRV88X3120_REG_DEVICE_1,
                                MRVL_88X3120_SMI2_PORT0_ADDR};

    /* Enable the external loopback setting of Phy 88X2222M */
    if (mrvl_88X2222M_sel_port2() == FAILED) {
        return (FAILED);
    }

    /*
     * Set up the PMA loopback mode in 88X3120 chip:
     * Cavium CPU -> Phy 88X2222M -> 88X3120
     */

    /* Get the current device address 0x1 register offset 0x0 value */
    if ((read_ten_g_phy_reg(PMA_PMD_CTRL_1_REG, MRVL_88X3120_PHY_REG_LEN,
                            &reg_val, &phy_88X3120)) == FAILED) {
        return (FAILED);
    }
    /* Clear the Bit 0, disable the PMA loopback mode*/
    reg_val &= PMA_LPBK_BIT_MASK;
    /* Write register value */
    if ((write_ten_g_phy_reg(PMA_PMD_CTRL_1_REG, MRVL_88X3120_PHY_REG_LEN,
                             reg_val, &phy_88X3120)) == FAILED) {
        return (FAILED);
    }

    /* Get the current device address 0x1 register offset 0x0 value */
    if ((read_ten_g_phy_reg(PMA_PMD_CTRL_1_REG, MRVL_88X3120_PHY_REG_LEN,
                            &reg_val, &phy_88X3120)) == FAILED) {
        return (FAILED);
    }
    /* Perform a software reset */
    reg_val |= SOFTWARE_RESET;
    /* Write register value */
    if ((write_ten_g_phy_reg(PMA_PMD_CTRL_1_REG, MRVL_88X3120_PHY_REG_LEN,
                             reg_val, &phy_88X3120)) == FAILED) {
        return (FAILED);
    }

    /* Monitor if software reset can be completed within 3 seconds. */
    for (ix = 0; ix < 1000; ix++) {
        msleep(3);
        /* Get the current device address 0x1 register offset 0x0 value */
        if ((read_ten_g_phy_reg(PMA_PMD_CTRL_1_REG, MRVL_88X3120_PHY_REG_LEN,
                                &reg_val, &phy_88X3120)) == FAILED) {
            return (FAILED);
        }
        if ((reg_val & SOFTWARE_RESET) == 0) {
            break;
        }
    }

    if (reg_val & SOFTWARE_RESET) {
        cterr('f', 0, "88X3120 do software reset fails.");
        return (FAILED);
    }

    /* Perform 88X3120 internal loopback test */
    if ((xaui_lpbk_test(LOOP_EXT)) == FAILED) {
        cterr('f', 0, "88X3120 internal loopback test failed.");
        return (FAILED);
    }

    return (PASSED);
}

/******************************************************************************
 *
 * Function: dump_phy_88X3120_registers
 *
 * Description: This function dumps the XAUI 88X3120 registers
 *
 * Inputs      : None
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/
static int dump_phy_88X3120_registers (void)
{
    uint ix, port_num, reg_addr, dev_addr, bus_addr, phy_addr;
    uint reg_val;

    port_num = MRVL_88X3120_PORTS;
    phy_addr = MRVL_88X3120_PORT_0_ADDR;
    bus_addr = (MRVL_88X3120_SMI2_ADDR << 4);
    
    printf("Dump phy 88X3120 Register.\n");
    dev_addr = gethex_answer("Enter device address (0x1,0x3,0x4,0x7,0x1D,0x1E,0x1F): ", 0, 1, 0x1F);
    reg_addr = gethex_answer("Enter register address (0x0~0xFFFFF): ", 0, 0, 0xFFFFF);
    
    for (ix = 0; ix < port_num; ix++) {
        ten_g_phy_t dump_phy_88X3120 = {dev_addr, (bus_addr | (phy_addr + ix))};

        /* Get the current register vlaue */
        if ((read_ten_g_phy_reg(reg_addr, MRVL_88X3120_PHY_REG_LEN, 
                                        &reg_val, &dump_phy_88X3120)) == FAILED) {
            cterr('f', 0, "Can't read register value at port=%d, "
                  "dev_addr=%04x, reg_addr=%04x", ix, dev_addr, reg_addr);
            return (FAILED);
        }
        printf("Phy address %x Reg %#.4x, data %#.4x\n",
                phy_addr, reg_addr, reg_val);
    }
    return (PASSED);
}

/******************************************************************************
 *
 * Function: alter_phy_88X3120_register
 *
 * Description: This function will allow user to alter the XAUI 88X3120 registers
 *
 * Inputs      : None
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/
static int alter_phy_88X3120_register (void)
{
    uint ix, port_num, reg_addr, dev_addr, bus_addr, phy_addr;
    uint reg_val;

    port_num = MRVL_88X3120_PORTS;
    phy_addr = MRVL_88X3120_PORT_0_ADDR;
    bus_addr = (MRVL_88X3120_SMI2_ADDR << 4);
    
    printf("Alter phy 88X3120 Register.\n");
    dev_addr = gethex_answer("Enter device address (0x1,0x3,0x4,0x7,0x1D,0x1E,0x1F): ", 0, 1, 0x1F);
    reg_addr = gethex_answer("Enter register address (0x0~0xFFFFF): ", 0, 0, 0xFFFFF);
    
    ten_g_phy_t alter_phy_88X3120 = {dev_addr, (bus_addr | (phy_addr))};

    /* Get the current register vlaue */
    if ((read_ten_g_phy_reg(reg_addr, MRVL_88X3120_PHY_REG_LEN,
                                            &reg_val, &alter_phy_88X3120)) == FAILED) {
        cterr('f', 0, "Can't read register value at port=%d, "
              "dev_addr=%04x, reg_addr=%04x", ix, dev_addr, reg_addr);
        return (FAILED);
    }
    printf("Original value: reg %#.4x, data %#.4x\n", reg_addr, reg_val);

    /* Alter register with new value */
    reg_val = gethex_answer("Enter the new data (hex): ", reg_val, 0, 0xFFFF);

    /* Alter the current register with new vlaue */
    if ((write_ten_g_phy_reg(reg_addr, MRVL_88X3120_PHY_REG_LEN,
                             reg_val, &alter_phy_88X3120))== FAILED) {
        cterr('f', 0, "Can't alter register value at port=%d, "
              "dev_addr=%04x, reg_addr=%04x", ix, dev_addr, reg_addr);
        return (FAILED);
    }

    /* Get the new register vlaue */
    if ((read_ten_g_phy_reg(reg_addr, MRVL_88X3120_PHY_REG_LEN,
                            &reg_val, &alter_phy_88X3120)) == FAILED) {
        cterr('f', 0, "Can't read the newe register value at port=%d, "
              "dev_addr=%04x, reg_addr=%04x", ix, dev_addr, reg_addr);
        return (FAILED);
    }

    printf("After alter phy address %x register value, reg %#.4x, data %#.4x\n",
            phy_addr, reg_addr, reg_val);
     
    return (PASSED);
}

/******************************************************************************
 *
 * Function: phy_88X3120_fw_download
 *
 * Description: This function will allow user to download the XAUI 88X3120
 *              firmware into flash.
 *
 * Inputs      : None
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/
static int phy_88X3120_fw_download (void)
{
    uchar reg_val;
    uint32 flashsize, slavesize, smi_addr;
    ushort rc;

    flashsize = phy_3120_fw_hdr_size;
    slavesize = phy_3120_fw_bin_size;
    smi_addr = MRVL_88X3120_SMI2_ADDR;

    printf("\nflashSize = %x, slaveSize = %x ...\n", flashsize, slavesize);

    printf("\nStarting to write flash on PHY 3120, PhyAddr = 0x%X\n",
           MRVL_88X3120_PORT_0_ADDR);

    /* Set FPGA_88X3120_FLASH_CFG_P0 in high status, so phy 3120
     * would download the firmware from Cavium mdio interface into flash */
    fpga_reg_read(FPGA_DEV_SETTING_REG, (char *)&reg_val);
    reg_val |= FLASH_CFG_P0_HIGH;
    fpga_reg_write(FPGA_DEV_SETTING_REG, reg_val);

    msleep(1000);

    /* Reset phy 3120 */
    fpga_reg_read(FPGA_RST_SIG_REG, (char *)&reg_val);
    reg_val &= ~FPGA_3120_RESET;
    fpga_reg_write(FPGA_RST_SIG_REG, reg_val);

    msleep(3000);

    rc = SFPhyDownLoadFlash(&smi_addr /* smi bus number */,
                            MRVL_88X3120_PORT_0_ADDR /* phy address*/,
                            phy_3120_fw_hdr/* flashAddr(char*) */,
                            phy_3120_fw_hdr_size /* flash file size */,
                            phy_3120_fw_bin/* slaveAddr(char*) */,
                            phy_3120_fw_bin_size /* slave file size */);

    if (rc != 1) {
        printf("Phy 3120 flash burn failed, return = 0x%X\n",rc);
        return (FAILED);
    }

    msleep(3000);

    SFPhyRemovePhyDownloadMode(&smi_addr /* smi bus number */,
                               MRVL_88X3120_PORT_0_ADDR /* phy address*/);

    msleep(3000);

    /* Set FPGA_88X3120_FLASH_CFG_P0 in low status, so that phy 3120
     * would download the firmware from flash */
    reg_val &= (~FLASH_CFG_P0_HIGH);
    fpga_reg_write(FPGA_DEV_SETTING_REG, reg_val);

    msleep(1000);

    /* Reset phy 3120 */
    fpga_reg_read(FPGA_RST_SIG_REG, (char *)&reg_val);
    reg_val &= ~FPGA_3120_RESET;
    fpga_reg_write(FPGA_RST_SIG_REG, reg_val);

    msleep(3000);

    return (PASSED);
}

/******************************************************************************
 *
 * Function: pma_test_modes
 *
 * Description: This function will allow user to enter XAUI 88X3120 PMA test
 *              Mode.
 *
 * Inputs      : None
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/
static int pma_test_modes (void)
{
    int test_modes;
    uint reg_val;

    printf("Enter phy 88X3120 PMA test modes.\n");
    printf("Normal Mode :        Enter 0");
    printf("\tTest Mode 1 :        Enter 1\n");
    printf("\tTest Mode 2 :        Enter 2\n");
    printf("\tTest Mode 3 :        Enter 3\n");
    printf("\tTest Mode 4 tone-1 : Enter 4\n");
    printf("\tTest Mode 4 tone-2 : Enter 5\n");
    printf("\tTest Mode 4 tone-3 : Enter 6\n");
    printf("\tTest Mode 4 tone-4 : Enter 7\n");
    printf("\tTest Mode 4 tone-5 : Enter 8\n");
    printf("\tTest Mode 5 :        Enter 9\n");
    printf("\tTest Mode 6 :        Enter 10\n");
    test_modes = getdec_answer("Select Test Modes: \n", 0, 1, 10);

    ten_g_phy_t alter_phy_88X3120 = {MRV88X3120_REG_DEVICE_1,
                                     MRVL_88X3120_SMI2_PORT0_ADDR};

    /* Get the current register vlaue */
    if ((read_ten_g_phy_reg(PMA_TEST_MODE_REG, MRVL_88X3120_PHY_REG_LEN,
                            &reg_val, &alter_phy_88X3120)) == FAILED) {
        return (FAILED);
    }

    /* Clear PMA test Mode bit 15:10 register value */
    reg_val &= ~(PMA_TEST_MODE_MASK);

    switch (test_modes) {
        case NORMAL_MODE:
            /* Set PMA normal mode */
            reg_val |= PMA_NORMAL_MODE;
            break;
        case TEST_MODE_1:
            /* Set PMA test mode 1 */
            reg_val |= PMA_TEST_MODE_1;
            break;
        case TEST_MODE_2:
            /* Set PMA test mode 2 */
            reg_val |= PMA_TEST_MODE_2;
            break;
        case TEST_MODE_3:
            /* Set PMA test mode 3 */
            reg_val |= PMA_TEST_MODE_3;
            break;
        case TEST_MODE_4_TONE_1:
            /* Set PMA test mode 4 tone 1 */
            reg_val |= PMA_TEST_MODE_4_TONE_1;
            break;
        case TEST_MODE_4_TONE_2:
            /* Set PMA test mode 4 tone 2 */
            reg_val |= PMA_TEST_MODE_4_TONE_2;
            break;
        case TEST_MODE_4_TONE_3:
            /* Set PMA test mode 4 tone 3 */
            reg_val |= PMA_TEST_MODE_4_TONE_3;
            break;
        case TEST_MODE_4_TONE_4:
            /* Set PMA test mode 4 tone 4 */
            reg_val |= PMA_TEST_MODE_4_TONE_4;
            break;
        case TEST_MODE_4_TONE_5:
            /* Set PMA test mode 4 tone 5 */
            reg_val |= PMA_TEST_MODE_4_TONE_5;
            break;
        case TEST_MODE_5:
            /* Set PMA test mode 5 */
            reg_val |= PMA_TEST_MODE_5;
            break;
        case TEST_MODE_6:
            /* Set PMA test mode 6 */
            reg_val |= PMA_TEST_MODE_6;
            break;
        default:
            printf("Input incorrect test mode number. ");
            break;
    }

    /* Alter the current register with new vlaue */
    if ((write_ten_g_phy_reg(PMA_TEST_MODE_REG, MRVL_88X3120_PHY_REG_LEN,
                             reg_val, &alter_phy_88X3120)) == FAILED) {
        return (FAILED);
    }

    return (PASSED);
}

/*-------------------------------------------------
 * $Log: diag_xaui_88X3120_test.c,v $
 * Revision 1.2  2013/10/08 08:48:29  tirawan
 * Woodlawn collapsed to main trunk
 *
 * Revision 1.1.4.2  2013/08/20 10:58:54  tirawan
 * Branch into woodlawn-branch2 and port woodlawn code
 *
 * Revision 1.1.2.1  2013/04/24 10:37:20  tirawan
 * Initial check-in for woodlawn linux code
 *
 * Revision 1.2  2013/03/27 04:49:37  kuangik
 * Code cleanup after adding -Wall
 *
 * Revision 1.17  2012/10/24 10:41:34  leslie
 * Fix and clean up code.
 *
 * Revision 1.16  2012/10/11 00:53:55  kody
 * Add print new line for X3120 test mode.
 *
 * Revision 1.15  2012/10/03 06:06:22  kody
 * Fix the X3120 internal loopback register setting.
 *
 * Revision 1.14  2012/09/21 11:49:43  kody
 * Fix the 88X3120 FW download issue.
 *
 * Revision 1.13  2012/08/03 10:16:55  evanli
 * Mapping to latest O2 source code on 20120726
 *
 * Revision 1.11  2012/07/19 06:30:51  leslie
 * Use cterr instead of use err
 *
 * Revision 1.10  2012/07/16 02:03:32  leslie
 * Modify dump/alter register function
 *
 * Revision 1.9  2012/07/11 08:10:19  kody
 * Modify the 88X3120 test mode setting.
 *
 * Revision 1.8  2012/07/09 08:51:11  kody
 * Add Phy 3120 test mode in utilities.
 *
 * Revision 1.7  2012/07/05 02:07:27  kody
 * Add Phy 3120 FW download utility.
 *
 * Revision 1.6  2012/05/18 10:23:42  kody
 * Add 88X3120 internal and external loopback test.
 *
 * Revision 1.5  2012/05/16 03:03:25  leslie
 * Update code
 *
 * Revision 1.4  2012/05/15 01:20:44  leslie
 * Add Marvel XAUI 88X3120 register and utility test
 *
 * Revision 1.3  2012/04/16 02:41:39  kody
 * Clean up the 88X3120 test code.
 *
 * Revision 1.2  2012/02/13 03:33:11  leslie
 * Add function prototype.
 *
 * Revision 1.1  2012/02/10 07:12:05  leslie
 * Add Woodlawn phy 88X3120 test.
 *
 *
 * $Endlog$
 *-------------------------------------------------
 */
