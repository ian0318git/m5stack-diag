/* $Id: diag_bcm54194_test.c,v 1.2 2021/06/02 08:22:34 iachang Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/fugazi/diag_bcm54194_test.c,v $
*-----------------------------------------------------------------------------
* bcm82752_test.c - Diags Test for BCM GE PHY bcm54194.
*
* June 2016, Mecca Ho
* Jan 2019, Letsai modified for Fugazi.
*
* Copyright (c) 2016-2020 by Cisco Systems, Inc.
* All rights reserved.
*-----------------------------------------------------------------------------
*/
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <linux/types.h>
#include <linux/socket.h>
#include <sys/ioctl.h>
#include <linux/if_ether.h>
#include <linux/ip.h>
#include <linux/udp.h>
#include <net/if.h>
#include <netinet/in.h>
#include <linux/sockios.h>
#include <linux/mii.h>
#include <assert.h>

#include "defs.h"
#include "types.h"
#include "proto.h"
#include "common.h"
#include "common_utils.h"
#include "monitor.h"
#include "menu.h"
#include "setjmps.h"
#include "nvmonvars.h"
#include "error.h"
#include "router_if.h"
#include "sgmii_defs.h"
#include "ethernet.h"
#include "linux_eth.h"
#include "platform_eth.h"
#include "ethernet.h"  /* for SFPx port definition */
#include "dash_fpga.h" /* for get SFP ctrl reg */
#include "queryflags.h" /* for query user functions */
#include "diag_bcm54194_api.h"
#include "platform_ext_lpbk.h"
#include "platform_i2c.h"
#include "diag_bnxt.h"
#include "diag_bcm_lib.h"
#include "diag_miura_reg.h"
#include "diag_bcm57412_utils.h"

#define MF_1    (MF_CONTINUOUS | MF_DOGRP)
#define MF_2    (MF_1 | MF_DOALL)
#define MF_3    (MF_2 | MF_SHOW_ERRCOUNT)
#define MF_4    (MF_1 | MF_SHOW_ERRCOUNT)

extern int is_item_available(void);
extern int ge_phy_ext_lpbk_test(int eth_num, int speed, int lpbkmode);

static int BCM54194_utility(int);
static void read_bcm54194_ieee_reg(void);
static void write_bcm54194_ieee_reg(void);
static void read_bcm54194_rdb_reg(void);
static void write_bcm54194_rdb_reg(void);
static int BCM54194_register_test(void);
static int BCM54194_sfp_ext_loopback_test(void);
static void ge_phy_ieee_reg_dump(void);
static void ge_phy_reset(void);
static void read_bcm54194_mdio45_reg(void);
static void write_bcm54194_mdio45_reg(void);
static int phy_register_tests(int, int, bcm54194_intf_t intf, const reg_info_t *reg_ptr );
int BCM54194_internal_loopback_test(void);
static int bcm54194_interrupt_test_f(void);

extern struct fugazi *fugazi_struct;


static const reg_info_t bcm_54194_ieee_sgmii_reg[] = {
    {"SGMII Control",                          0x00, READ_WRITE, {2}, 0x55C0, 0x1140},
    {"SGMII Status",                           0x01, READ_ONLY,  {2}, 0x0000, 0x0149},
    {"SGMII AN Advertisement",                 0x04, READ_WRITE, {2}, 0x9E01, 0x0801},
    {"SGMII AN Link Partner Ability",          0x05, READ_ONLY,  {2}, 0x0000, 0x0001},
    {"end",                                    0x00, 0, {0}, 0, 0},
};

static const reg_info_t bcm_54194_ieee_copper_reg[] = {
    {"Copper MII Control",                     0x00, READ_WRITE, {2}, 0x55C0, 0x1140},
    {"Copper MII Status",                      0x01, READ_ONLY,  {2}, 0x0000, 0x79C9},
    {"PHY ID1 MSB",                            0x02, READ_ONLY,  {2}, 0x0000, 0xAE02},
    {"PHY ID2 LSB",                            0x03, READ_ONLY,  {2}, 0x0000, 0x5018},
    {"Copper AN Advertisement",                0x04, READ_WRITE, {2}, 0xBFFF, 0x01E1},
    {"Copper AN Link Partner Ability",         0x05, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"Copper AN Expansion",                    0x06, READ_ONLY,  {2}, 0x0000, 0x0064},
    {"Copper Next Page Transmit",              0x07, READ_WRITE, {2}, 0xB7FF, 0x2000},
    {"Copper Link Partner Received Next Page", 0x08, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"1000BT Control",                         0x09, READ_WRITE, {2}, 0xFF00, 0x0F00},
    {"1000BT Status",                          0x0A, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"IEEE Extended Status",                   0x0F, READ_ONLY,  {2}, 0x0000, 0x3000},
    {"end",                                    0x00, 0, {0}, 0, 0},
};

static const reg_info_t bcm_54194_ieee_fiber_reg[] = {
    {"Fiber Control",                          0x00, READ_WRITE, {2}, 0x55C0, 0x1140},
    {"1000BX Status",                          0x01, READ_ONLY,  {2}, 0x0000, 0x0140},
    {"1000BX AN Advertisement",                0x04, READ_WRITE, {2}, 0x0000, 0x0000},
    {"1000BX AN Link Partner Ability",         0x05, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"end",                                    0x00, 0, {0}, 0, 0},
};

static const bcm_phy_regs_t bcm54194_phy_ieee_reg_tbl[] = {
    {"Copper",    BCM54194_COPPER_INTF, bcm_54194_ieee_copper_reg},
    {"SGMII",     BCM54194_SGMII_INTF,  bcm_54194_ieee_sgmii_reg},
    {"Fiber",     BCM54194_FIBER_INTF,  bcm_54194_ieee_fiber_reg},
};


#define NUM_PHY_INTF (sizeof(bcm54194_phy_ieee_reg_tbl) /      \
                      sizeof(struct bcm_phy_regs_t_))

/******************************************************************************
 *  List of Menu used for SGMII BCM54194
 *****************************************************************************/
static submenu_xtable_t BCM54194_tests_submenu_table[] = {
    {"GE PHY BCM54194 Utility", (type_t(*)())BCM54194_utility,   FALSE,
       0, NULL, 0, (type_t(*)())BCM54194_utility,   TRUE},
    {"GE PHY BCM54194 Register Test", (type_t(*)())BCM54194_register_test,   0,
        MF_3, (type_t(*)())0, 0, (type_t(*)())0,   0},
    {"GE PHY BCM54194 Internal Loopback Test", (type_t(*)())BCM54194_internal_loopback_test, 0,
        MF_3, (type_t(*)())0, 0, (type_t(*)())0,   0},
    {"GE PHY BCM54194 SFP External Loopback Test", (type_t(*)())BCM54194_sfp_ext_loopback_test,   0,
        MF_3, (type_t(*)())0, 0, (type_t(*)())0,   0},
};
/******************************************************************************
 *  List of Utilities used for SGMII BCM54194
 *****************************************************************************/
static submenu_xtable_t BCM54194_util_items[] = {
    {"Show BCM54194 PHY registers", (type_t(*)())ge_phy_ieee_reg_dump,   0, 0,
     (type_t(*)())0, 0, (type_t(*)())0,   0},
    {"Read PHY BCM54194 IEEE Register", (type_t(*)())read_bcm54194_ieee_reg, 0, 0,
     (type_t(*)())0, 0, (type_t(*)())0, 0},
    {"Write PHY BCM54194 IEEE Register", (type_t(*)())write_bcm54194_ieee_reg, 0, 0,
     (type_t(*)())0, 0, (type_t(*)())0, 0},
    {"Read PHY BCM54194 RDB Register", (type_t(*)())read_bcm54194_rdb_reg, 0, 0,
     (type_t(*)())0, 0, (type_t(*)())0, 0},
    {"Write PHY BCM54194 RDB Register", (type_t(*)())write_bcm54194_rdb_reg, 0, 0,
     (type_t(*)())0, 0, (type_t(*)())0, 0},
    {"Read PHY BCM54194 MDIO45 Register", (type_t(*)())read_bcm54194_mdio45_reg, 0, 0,
     (type_t(*)())0, 0, (type_t(*)())0, 0},
    {"Write PHY BCM54194 MDIO45 Register", (type_t(*)())write_bcm54194_mdio45_reg, 0, 0,
     (type_t(*)())0, 0, (type_t(*)())0, 0},
    {"Reset and Init All BCM54194 PHY", (type_t(*)())bcm54194_reset, 1, 0,
     (type_t(*)())0, 0, (type_t(*)())0, 0},
    {"Config Loopback Mode", (type_t(*)())config_lpbk_mode, 0, 0,
     (type_t(*)())0, 0, (type_t(*)())0, 0},
    {"PHY BCM54194 reset",           (type_t(*)())ge_phy_reset, 0, 0,
     (type_t(*)())0, 0, (type_t(*)())0, 0},
    {"Check link status", (type_t(*)())check_link, 0, 0,
     (type_t(*)())0, 0, (type_t(*)())0, 0},
    {"PRBS Config/Check", (type_t(*)())bcm54194_config_prbs, 0, 0,
     (type_t(*)())0, 0, (type_t(*)())0, 0},
    {"Show/Enable Packet counter", (type_t(*)())packet_counter_util, 0, 0,
     (type_t(*)())0, 0, (type_t(*)())0, 0},
    {"Interrupt utility", (type_t(*)())bcm54194_interrupt_util, 0, 0,
     (type_t(*)())0, 0, (type_t(*)())0, 0},
    {"Interrupt test", (type_t(*)())bcm54194_interrupt_test_f, 0, 0,
     (type_t(*)())0, 0, (type_t(*)())0, 0},
    {"BCM57412 sideband tx_dis", (type_t(*)())bcm57412_sideband_tx_dis, 0, 0,
     (type_t(*)())0, 0, (type_t(*)())0, 0},
};

/******************************************************************************
 *  Macro Definitions
 *****************************************************************************/
#define BCM54194_TESTS_SUBMENU_TABLE_SIZE (sizeof(BCM54194_tests_submenu_table) / \
                                           sizeof(submenu_xtable_t))

#define BCM54194_TESTS_UTIL_SIZE (sizeof(BCM54194_util_items) / \
                                  sizeof(submenu_xtable_t))

/******************************************************************************
 *  Global Variable
 *****************************************************************************/

/******************************************************************************
 * Primary & secondary submenu items (filled in from xtable)
 *****************************************************************************/
static mitem_t BCM54194_tests_primary_items[BCM54194_TESTS_SUBMENU_TABLE_SIZE +
                                            MAX_BASE_ITEMS];
static mitem_t BCM54194_tests_secondary_items[BCM54194_TESTS_SUBMENU_TABLE_SIZE +
                                              MAX_BASE_ITEMS];

/******************************************************************************
 * Primary & secondary utilities menu items (filled in from xtable)
 *****************************************************************************/
static mitem_t BCM54194_tests_primary_util_items[BCM54194_TESTS_UTIL_SIZE +
                                                 MAX_BASE_ITEMS];
static mitem_t BCM54194_tests_secondary_util_items[BCM54194_TESTS_UTIL_SIZE +
                                                   MAX_BASE_ITEMS];

/******************************************************************************
 * BCM54194 Utils submenu
 *****************************************************************************/
menuinfo_t BCM54194_util_menu = {
    "GE PHY BCM54194 Utility Menu",
    0,
    (PFT)show_endnote,
    0,
    0,
    BCM54194_tests_primary_util_items,
};
menuinfo_t *BCM54194_util_menup = &BCM54194_util_menu;

menuinfo_t BCM54194_subtest_menu = {
    "%s Subtest Menu",
    0,                                /* mtparam added by init_empty_menu */
    (PFT)show_endnote,                /* notes missing WICs in combos */
    0,                                /* use generic prompt */
    0,                                /* size (bumped by add_menu_item() */
    BCM54194_tests_primary_items,
};
menuinfo_t *BCM54194_submenup = &BCM54194_subtest_menu;

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
int ge_phy_bcm54194_test (int show_menu)
{
    build_primary_submenu(BCM54194_tests_submenu_table,
                          BCM54194_TESTS_SUBMENU_TABLE_SIZE,
                          "GE PHY BCM54194", &BCM54194_submenup);
    build_secondary_submenu(BCM54194_tests_submenu_table,
                            BCM54194_TESTS_SUBMENU_TABLE_SIZE,
                            BCM54194_tests_secondary_items);

    if (show_menu) {
        menu_exec_doall_diags(BCM54194_submenup);
    } else {
        menu(BCM54194_submenup, BCM54194_tests_secondary_items, '\0' );
    }
    return (PASSED);
}


/*******************************************************************************
 *
 * Function    : BCM54194_utility
 * Description :
 * Inputs      : menu_option - display menu instead of running all XAUI 88X2222M
 *               tests.
 * Outputs     : PASSED / FAILED
 *
 ******************************************************************************/
static int BCM54194_utility (int show_menu)
{

    build_primary_submenu(BCM54194_util_items, BCM54194_TESTS_UTIL_SIZE,
                          "GE PHY BCM54194 Utilities Menu", &BCM54194_util_menup);
    build_secondary_submenu(BCM54194_util_items, BCM54194_TESTS_UTIL_SIZE,
                            BCM54194_tests_secondary_util_items);

    menu(BCM54194_util_menup, BCM54194_tests_secondary_util_items, '\0' );

    return (PASSED);
}

/*
 * Function: read_bcm54194_mdio45_reg
 *
 * Description: Read BCM54194 IEEE standard registers utility
 *
 * Input: none
 *
 * Return: none
 */
static void read_bcm54194_mdio45_reg (void)
{
    uint16_t rdval;
    int phy_addr, regnum;
    int rc;
    int phy, phy_num;

    phy = gethex_answer("\nEnter phy num (0 ~ 3)", 0, 0, 3);
    phy_num = ge_phy_mapping_phy_num[phy];
    phy_addr = gethex_answer("\nEnter phy addr (0x0 - 0xFF)", 0, 0, 0xFF);
    regnum = gethex_answer("\nEnter PHY mdio45 reg number", 0, 0, 0xFFFF);

    rc = fugazi_bnxt_mdio_read(&fugazi_struct->bnxt[phy_num], phy_addr, FUGAZI_MIURA_DEV_1G_PHY, regnum, &rdval);
    if (rc < 0) {
        printf("Failed to read GE PHY, addr:%#.2x, %#.4x\n", phy_addr, regnum);
    } else {
        printf("phy addr:%#.2x, %#.4x = %#.4x\n", phy_addr, regnum, rdval);
    }
}

/*
 * Function: write_bcm54194_mdio45_reg
 *
 * Description: Write BCM54194 IEEE standard registers utility
 *
 * Input: none
 *
 * Return: none
 */
static void write_bcm54194_mdio45_reg (void)
{
    ushort wrval;
    int rc, phy_addr, regnum;
    int phy, phy_num;

    phy = gethex_answer("\nEnter phy num (0 ~ 3)", 0, 0, 3);
    phy_num = ge_phy_mapping_phy_num[phy];
    phy_addr = gethex_answer("\nEnter phy addr (0x0 - 0xFF)", 0, 0, 0xFF);
    regnum = gethex_answer("\nEnter PHY mdio45 reg number", 0, 0, 0xFFFF);
    wrval = gethex_answer("Enter value:", 0, 0, 0xFFFF);

    rc = fugazi_bnxt_mdio_write(&fugazi_struct->bnxt[phy_num], phy_addr, FUGAZI_MIURA_DEV_1G_PHY, regnum, wrval);

    if (rc != PASSED) {
        printf("Failed to write GE PHY %d, phy addr:%#.2x, %#.4x\n", phy_num, phy_addr, regnum);
    } else {
        printf("PHY %d, phy addr:%#.2x, %#.4x <-- %#.4x, \n", phy_num, phy_addr, regnum, wrval);
    }
}

/*
 * Function: read_bcm54194_ieee_reg
 *
 * Description: Read BCM54194 IEEE standard registers utility
 *
 * Input: none
 *
 * Return: none
 */
static void read_bcm54194_ieee_reg (void)
{
    int port_max = 3;
    int port_min = 0;
    int phy_intf = BCM54194_SGMII_INTF;
    uint16_t rdval;
    int phy_addr, rc, portnum, regnum, regnum_max = 0xF;
    int phy, phy_num;

    phy = gethex_answer("\nEnter phy num (0 ~ 3)", 0, 0, 3);
    phy_num = ge_phy_mapping_phy_num[phy];
    portnum = gethex_answer("\nEnter port num (0x0 - 0x3)", port_min, port_min, port_max);
    phy_intf = gethex_answer("\nEnter Interface (SGMII:0, Copper:1, Fiber:2)", 0, BCM54194_SGMII_INTF, BCM54194_FIBER_INTF);
    regnum = gethex_answer("\nEnter PHY IEEE reg number", 0, 0, regnum_max);

    phy_addr = ge_port_mapping_phy_addr_down[portnum];

    /* Switch to SGMII or Fiber register space */
    if (phy_intf == BCM54194_SGMII_INTF) {
        phy_addr += 4;
    } else if (phy_intf == BCM54194_FIBER_INTF) {
    	bcm54194_reg_1000x_en(phy_num, phy_addr, TRUE);
    }

    rc = fugazi_bnxt_mdio_read(&fugazi_struct->bnxt[phy_num], phy_addr, FUGAZI_MIURA_DEV_1G_PHY, regnum, &rdval);

    if (rc < 0) {
        printf("Failed to read GE PHY %d, port%d, 0x%x\n", phy_num, portnum, regnum);
    } else {
        printf("PHY %d port%d, 0x%x = %#.4x, \n", phy_num, portnum, regnum, rdval);
    }

}

/*
 * Function: write_bcm54194_ieee_reg
 *
 * Description: Write BCM54194 IEEE standard registers utility
 *
 * Input: none
 *
 * Return: none
 */
static void write_bcm54194_ieee_reg (void)
{
    int port_max = 1;
    int port_min = 0;
    int phy_intf = BCM54194_SGMII_INTF;
    ushort wrval;
    int rc, phy_addr, portnum, regnum, regnum_max = 0xF;
    int phy, phy_num;

    phy = gethex_answer("\nEnter phy num (0 ~ 3)", 0, 0, 3);
    phy_num = ge_phy_mapping_phy_num[phy];
    portnum = gethex_answer("\nEnter port num (0x0 - 0x1)", port_min, port_min, port_max);
    phy_intf = gethex_answer("\nEnter Interface (SGMII:0, Copper:1, Fiber:2)", 0, BCM54194_SGMII_INTF, BCM54194_FIBER_INTF);
    regnum = gethex_answer("\nEnter PHY IEEE reg number", 0, 0, regnum_max);
    wrval = gethex_answer("Enter value:", 0, 0, 0xFFFF);

    phy_addr = ge_port_mapping_phy_addr_down[(phy_num*2) + portnum];

    /* Switch to SGMII or Fiber register space */
    if (phy_intf == BCM54194_SGMII_INTF) {
        phy_addr += 4;
    } else if (phy_intf == BCM54194_FIBER_INTF) {
        bcm54194_reg_1000x_en(phy_num, phy_addr, TRUE);
    }

    rc = fugazi_bnxt_mdio_write(&fugazi_struct->bnxt[phy_num], phy_addr, FUGAZI_MIURA_DEV_1G_PHY, regnum, wrval);
    if (rc != PASSED) {
        printf("Failed to write GE PHY %d , port%d, 0x%x\n", phy_num, portnum, regnum);
    } else {
        printf("PHY %d, port%d, 0x%x <-- %#.4x, \n", phy_num, portnum, regnum, wrval);
    }
}

/*
 * Function: read_bcm54194_rdb_reg
 *
 * Description: Read BCM54194 RDB registers utility
 *
 * Per-Port RDB Registers (RDB_reg. 0x00 to 0x2FF)
 * Global RDB Registers (RDB_reg. 0x800 to 0xAFF)
 *
 * Input: none
 *
 * Return: none
 */
static void read_bcm54194_rdb_reg (void)
{
    int port_min = 0, port_max = 1;
    ushort rdval;
    int rc, phy_addr, portnum, regnum, regnum_max = 0xAFF;
    int phy, phy_num;

    printf("\nPer-Port RDB Registers (RDB_reg. 0x00 to 0x2FF)\n");
    printf("\nGlobal RDB Registers (RDB_reg. 0x800 to 0xAFF)\n");
    phy = gethex_answer("\nEnter phy num (0 - 3)", 0, 0, 3);
    portnum = gethex_answer("\nEnter port num (0x0 - 0x1)", port_min, port_min, port_max);
    regnum = gethex_answer("\nEnter PHY RDB reg number", 0, 0, regnum_max);

    phy_num = ge_phy_mapping_phy_num[phy];
    phy_addr = ge_port_mapping_phy_addr_down[(phy_num*2) + portnum];

    rc = bcm54194_rdb_read(phy_num, phy_addr, regnum, &rdval);
    if (rc < 0) {
        printf("Failed to read GE PHY %d, phy addr:%d, RDB offset:0x%x\n", phy_num-2, phy_addr, regnum);
    } else {
        printf("port%d, RDB:0x%x = %#.4x, \n", portnum, regnum, rdval);
    }
}

/*
 * Function: write_bcm54194_rdb_reg
 *
 * Description: Utility to do peek and poke to GE PHY registers
 *
 * Input: none
 *
 * Return: none
 */
static void write_bcm54194_rdb_reg (void)
{
    int port_min = 0, port_max = 3;
    ushort wrval;
    int rc, phy_addr, portnum, regnum, regnum_max = 0xAFF;
    int phy, phy_num;

    printf("\nPer-Port RDB Registers (RDB_reg. 0x00 to 0x2FF)\n");
    printf("\nGlobal RDB Registers (RDB_reg. 0x800 to 0xAFF)\n");
    phy = gethex_answer("\nEnter phy num (0 - 3)", 0, 0, 3);
    portnum = gethex_answer("\nEnter port num (0x0 - 0x3)", port_min, port_min, port_max);
    regnum = gethex_answer("\nEnter PHY RDB reg number", 0, 0, regnum_max);
    wrval = gethex_answer("Enter value:", 0, 0, 0xFFFF);

    phy_num = ge_phy_mapping_phy_num[phy];
    phy_addr = ge_port_mapping_phy_addr_down[(phy_num*2) + portnum];

    rc = bcm54194_rdb_write(phy_num, phy_addr, regnum, wrval);
    if (rc < 0) {
        printf("Failed to write GE PHY, phy addr:0x%x, RDB offset:0x%x, value=0x%#.4x\n", phy_addr, regnum, wrval);
    } else {
        printf("port%d, RDB:0x%x <-- %#.4x, \n", portnum, regnum, wrval);
    }
}

/*******************************************************************************
 *
 * Function: phy_register_tests
 *
 * Description: For each register from reg_ptr, this function checks for accessibility
 * and does a ripple 1 and a ripple 0 test if applicable (not all registers
 * are W/R register).
 *
 * Input: phy_num - PHY number
 *        phy_addr - PHY mdio address
 *        intf - what side of interface to test (0: SMII; 1: Copper; 2: Fiber
 *        reg_ptr - info for all registers to be tested
 *
 * Output: PASS/FAIL
 *
 *******************************************************************************/
static int phy_register_tests (int phy_num, int phy_addr, bcm54194_intf_t intf, const reg_info_t *reg_ptr )
{
    uint32_t ix;
    uint16_t retval = PASSED;
    uint16_t data, temp, tst_offset, save_val, readval = 0x0;
    int rc;

    /* Switch to Copper/Fiber register space */
    if (intf == BCM54194_FIBER_INTF) {
    	bcm54194_reg_1000x_en(phy_num, phy_addr, TRUE);
    } else {
        bcm54194_reg_1000x_en(phy_num, phy_addr, FALSE);
    }

    while (reg_ptr->size.size != 0) {

        rc = fugazi_bnxt_mdio_read(&fugazi_struct->bnxt[phy_num], phy_addr, FUGAZI_MIURA_DEV_1G_PHY, reg_ptr->offset, &save_val);
        if (rc < 0) {
            cterr('f', 0, "%s(): Error reading %s register offset %#x"
                  "phy_addr %d, GE PHY %d\n", __FUNCTION__,  reg_ptr->name,
                  reg_ptr->offset, phy_addr, phy_num-2);
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

                retval = fugazi_bnxt_mdio_write(&fugazi_struct->bnxt[phy_num], phy_addr, FUGAZI_MIURA_DEV_1G_PHY, tst_offset, temp);

                /* Read back */
                if (retval == PASSED) {
                    fugazi_bnxt_mdio_read(&fugazi_struct->bnxt[phy_num], phy_addr, FUGAZI_MIURA_DEV_1G_PHY, tst_offset, &readval);
                }

                if ( ((readval & reg_ptr->mask) != temp) || (retval == FAILED)) {

                    cterr('f', 0, "%s(): Ripple one test failed when accessing %s "
                          "Register offset %#x, phy_addr %d, GE PHY %d, Expect %#x, Read %#x",
                          __FUNCTION__, reg_ptr->name, tst_offset, phy_addr, phy_num-2, temp,
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
                retval  = fugazi_bnxt_mdio_write(&fugazi_struct->bnxt[phy_num], phy_addr, FUGAZI_MIURA_DEV_1G_PHY, tst_offset, temp);

                if (retval == PASSED) {
                    /* Read back */
                    fugazi_bnxt_mdio_read(&fugazi_struct->bnxt[phy_num], phy_addr, FUGAZI_MIURA_DEV_1G_PHY, tst_offset, &readval);
                }

                if (((readval & reg_ptr->mask) != temp) || (retval == FAILED)) {
                    cterr('f', 0, "%s(): Ripple one test failed when accessing %s "
                          "Register offset %#x, phy_addr %d, GE PHY %d, Expect %#x, Read %#x",
                          __FUNCTION__, reg_ptr->name, tst_offset, phy_addr, phy_num-2, temp,
                          readval);
                    return (retval);
                }
            }

            /*
             * pattern test
             */
            data = NEP_PATTERN;
            for (ix = 0; ix < 2; ix++) {
                temp = data & reg_ptr->mask;
                /* Write to register under test */
                retval  = fugazi_bnxt_mdio_write(&fugazi_struct->bnxt[phy_num], phy_addr, FUGAZI_MIURA_DEV_1G_PHY, tst_offset, temp);

                if (retval == PASSED) {
                    /* Read back */
                    fugazi_bnxt_mdio_read(&fugazi_struct->bnxt[phy_num], phy_addr, FUGAZI_MIURA_DEV_1G_PHY, tst_offset, &readval);
                }

                if (((readval & reg_ptr->mask) != temp) || (retval == FAILED)) {
                    cterr('f', 0, "%s(): Pattern test failed when accessing %s "
                          "Register offset %#x phy_addr %d, GE PHY %d, Expect %#x, "
                          "Read %#x", __FUNCTION__, reg_ptr->name, tst_offset,
                          phy_addr, phy_num-2, temp, readval);
                    return (retval);
                }

                data = ~NEP_PATTERN; /* complement data pattern */
            }

            /*
             * restore original value
             */
            retval  = fugazi_bnxt_mdio_write(&fugazi_struct->bnxt[phy_num], phy_addr, FUGAZI_MIURA_DEV_1G_PHY, tst_offset, save_val);
            if (retval == FAILED) {
                cterr('f', 0, "%s(): Error restoring %s register "
                      "offset %#x, phy_addr %d, GE PHY %d\n", __FUNCTION__,
                      reg_ptr->name, reg_ptr->offset, phy_addr, phy_num-2);
                return (FAILED);
            }
        }
        reg_ptr++;
    }

    return (PASSED);
}

/******************************************************************************
 *
 * Function: BCM54194_register_test
 *
 * Description: This function performs the BCM54194 register test.
 * This test only performs BCM54194 IEEE register set.
 *
 * Inputs      : None
 *
 * Outputs     : PASSED / FAILED
 *
 ********************************************************************************/
static int BCM54194_register_test (void)
{
    int ix = 0, phy_addr = 0, port_cnt = 2;
    int phy_num, result = PASSED;

    testname("BCM54194 PHY IEEE Register");

    for (phy_num = FUGAZI_MAC_1G_PHY_0; phy_num < (FUGAZI_MAC_1G_PHY_3+1); phy_num++)
    {
        /* 2 PHY port per 1G PHY */
        for (ix = 0; ix < port_cnt; ix++) {
            phy_addr = ge_port_mapping_phy_addr_down[(phy_num*2) + ix];

            prpass(testpass, "PHY %d Fiber addr %x: ", phy_num-2, phy_addr);
            if (phy_register_tests(phy_num, phy_addr, BCM54194_FIBER_INTF,
                                   &bcm_54194_ieee_fiber_reg[0]) == FAILED) {
                cterr('f', 0, "Register Test on address 0x%#x fails.", phy_addr);
                result = FAILED;
            }

        }

    }
    return (result);
}

/*******************************************************************************
 *
 * Function: dump_phy_reg()
 *
 * This function prints the specific PHY register values
 *
 * Input: phy_num - PHY number
 *        phy_addr - PHY mdio address
 *        phy_reg_ptr - info for all registers to be dump.
 *
 * Output: PASSED/FAILED
 *
 *******************************************************************************
 */
static void dump_phy_reg (int phy_num, int phy_addr, const bcm_phy_regs_t *phy_reg_ptr)
{
      const reg_info_t *reg_ptr;
      uint16_t rdval;

      /* Switch to SGMII or Fiber register space */
      if (phy_reg_ptr->phy_intf == BCM54194_SGMII_INTF) {
          bcm54194_switch_intf_access(phy_num, phy_reg_ptr->phy_intf);
          phy_addr += 4;
      } else if (phy_reg_ptr->phy_intf == BCM54194_FIBER_INTF) {
          bcm54194_reg_1000x_en(phy_num, phy_addr, TRUE);
      }

      reg_ptr = phy_reg_ptr->intfregs;

      while (reg_ptr->size.size != 0) {
          fugazi_bnxt_mdio_read(&fugazi_struct->bnxt[phy_num], phy_addr, FUGAZI_MIURA_DEV_1G_PHY, reg_ptr->offset, &rdval);

          /* we don't check rdval is nagetive here,
           * some of registers will get '0xF' on MSB
           */

          printf("%s : %-32s reg %#.2x = %#.4x\n", phy_reg_ptr->intfname, reg_ptr->name,
                  reg_ptr->offset, rdval);
          reg_ptr++;
          msleep(10); /* wait for a while for next register. */
      }
}

/*******************************************************************************
 *
 * Function: phy_reg_show()
 *
 * This function get PHY page/regs info and call dump_phy_reg() to prints
 * PHY registers.
 *
 * Input: phy_num - PHY number
 *        port - PHY mdio address
 *
 * Output: PASSED/FAILED
 *
 *******************************************************************************
 */
static int phy_reg_show (int phy_num, int port)
{
    uint ix, intf_move;
    const bcm_phy_regs_t *phy_reg_ptr;

    phy_reg_ptr = &bcm54194_phy_ieee_reg_tbl[0];
    intf_move = NUM_PHY_INTF;

    /* dump all page */
    for (ix = 0; ix < intf_move; ix++) {
        dump_phy_reg(phy_num, port, phy_reg_ptr);
        phy_reg_ptr++;
    }

    return (PASSED);
}

/*
 * Function: ge_phy_ieee_reg_dump
 *
 * This function displays the PHY setting of the requested SGMII port.
 * Using phy_reg_show() to dump PHY page.
 *
 * Input: none.
 *
 * Output: void
 */
static void ge_phy_ieee_reg_dump (void)
{
    int port_max = 1;
    int port_min = 0;
    int portnum, phy_addr, phy, phy_num;

    phy = gethex_answer("\nEnter phy num 0~3", 0, 0, 3);
    phy_num = ge_phy_mapping_phy_num[phy];

    printf("\nGE PHY port num are 0x0 - 0x1)\n");
    portnum = gethex_answer("\nEnter port num ", port_min, port_min, port_max);
    phy_addr = ge_port_mapping_phy_addr_down[(phy_num*2) + portnum];

    phy_reg_show(phy_num, phy_addr);
}
/***********************************************************************
 *
 * Function: BCM54194_internal_loopback_test
 *
 * Description: Do GE PHY internal loopback test
 *
 * Inputs: phy - phy number
 *
 * Outputs: PASSED/FAILED
 *
 **************************************************************************/
int BCM54194_internal_loopback_test (void)
{
    bcm54194_reset(0);
    return (fugazi_phy_lpbk_test(GE_PHY_INT_LPBK));
}

/***********************************************************************
 *
 * Function: sgmii_BCM54194_sfp_ext_loopback_test
 *
 * Description: Do GE PHY fiber external loopback test
 *
 * Inputs: None
 *
 * Outputs: PASSED/FAILED
 *
 ***************************************************************************/
static int BCM54194_sfp_ext_loopback_test (void)
{
    bcm54194_reset(0);
    return (sfp_phy_ext_lpbk_test());
}

/***********************************************************************
 *
 * Function: ge_phy_reset
 *
 * Description: Utility to do GE PHY reset/unreset from FPGA.
 *
 * Inputs: none
 *
 * Outputs: none
 *
 ***************************************************************************/
static void ge_phy_reset (void)
{
    int reset;
    sys_lvl_t *sys = (sys_lvl_t *)dash_fpga;

    assert(dash_fpga);

    reset = gethex_answer("\nPHY reset 1; unreset 0", 0, 0, 1);

    if (reset) {
        printf("Reset GE PHY...\n");
        sys->ext_rst |= FPGA_EXT_GE_RST_1G;
    } else {
        printf("Unreset GE PHY...\n");
        sys->ext_rst &= ~FPGA_EXT_GE_RST_1G;
    }
}

/*
 * Function: bcm54194_interrupt_test
 *
 * Description:
 * Utility to Enable/Disable BCM54194 LASI Interrupt .
 *
 * Input: none
 *
 * Return: None
 */
static int bcm54194_interrupt_test_f(void)
{
    int rc = PASSED;
    int ethnum, ethnum_index, eth_start, eth_end;
    uint16_t int_status=0;


    printf("\nPort number 0 - 7 = ETH4 - ETH11");
    ethnum = gethex_answer("\nEnter eth num (0x4 - 0xB; 0xff-all ports)", 0xff, 4, 0xff);

    if ( ethnum == 0xff) {
        eth_start = FUGAZI_1G_eth_4;
        eth_end = MAX_FUGAZI_1G_ETH;
    }
    else {
        eth_start = ethnum;
        eth_end = ethnum + 1;
    }

    for (ethnum_index=eth_start; ethnum_index<eth_end; ethnum_index++) {
        int_status = 0;
        if ( bcm54194_interrupt_test(ethnum_index, &int_status) ) {
            rc |= FAILED;
            printf("\nBCM54194 PHY eth%d interrupt test FAILED! (int_status=0x%04X)\n",
                        ethnum_index, int_status);
        }
        else {
            printf("\nBCM54194 PHY eth%d interrupt test PASSED\n", ethnum_index);
        }
    }

    return (rc);
}

/*-------------------------------------------------
$Log: diag_bcm54194_test.c,v $
Revision 1.2  2021/06/02 08:22:34  iachang
CSCvo59196-33 : Merge Fugazi from ASR1K-main-branch to ISR main trunk

Revision 1.1.4.2  2020/08/26 02:37:47  iachang
Merge Fugazi code into main trunk

Revision 1.1.2.7  2020/08/24 00:07:53  pdoong
Clean code for ER.

Revision 1.1.2.6  2020/08/06 04:25:01  pdoong
clean code for BCM54194 1G PHY

Revision 1.1.2.5  2020/03/19 06:31:40  iachang
Support Fugazi Side Band test

Revision 1.1.2.4  2020/03/13 09:06:50  iachang
Add BCM54194 1G PHY side band test

Revision 1.1.2.3  2020/02/25 02:48:35  pdoong
add utility to enable/generate 1G PHY interrupt to BCM57412 MAC LASI

Revision 1.1.2.2  2020/02/11 09:23:26  iachang
BCM54194 1G PHY test : HW would like test completed on all ports, not stop on failure port.

Revision 1.1.2.1  2019/10/16 06:12:31  letsai
Modify file name

Revision 1.1.6.22  2019/09/23 07:38:25  letsai
Add packet counter utility of BCM54194 phy

Revision 1.1.6.21  2019/08/30 06:44:56  letsai
1. Use HW reset to replace SW reset before BCM 54194 phy internal/external test. 2. Add delay time for Fiber link up.

Revision 1.1.6.20  2019/08/21 06:38:57  letsai
Add BCM54194 1G PHY PRBS utility

Revision 1.1.6.19  2019/07/19 07:35:29  letsai
1. Support LED control.
2. Support smart fan.
3. Change BCM 54194 phy reset bit.

Revision 1.1.6.18  2019/07/15 08:22:12  letsai
Add check link status function on network side for BCM 54194 phy

Revision 1.1.6.17  2019/06/15 03:48:42  letsai
1.Fix Rx mismatch error messgage showed in loopback test. 2.Removed Copper registers in BCM54194 phy register test. 3.Add print messgge when reset 1G phy.

Revision 1.1.6.16  2019/06/06 02:19:35  letsai
Modify test message. Reset and init 54194 phy before internal test.

Revision 1.1.6.15  2019/05/13 03:55:01  letsai
Add BCM54194 init function.

Revision 1.1.6.14  2019/05/13 03:37:40  letsai
Don't reset phy before external loopback setting.

Revision 1.1.6.13  2019/04/25 23:25:27  letsai
1. Remove eUSB test.
2. Fixed bnxt_mdio r/w function to support both 1G and 10G phy.

Revision 1.1.6.12  2019/04/18 23:11:58  letsai
Add loopback mode config uyility and clean up code.

Revision 1.1.6.11  2019/04/18 01:21:30  letsai
1. Clean up code
2. Modify 1G phy address mapping
3. Modify print message of MCU FW opgrade

Revision 1.1.6.10  2019/04/12 23:03:25  letsai
Add utility to enable 1000BASE-X Line-Side Loopback

Revision 1.1.6.9  2019/04/10 21:26:58  letsai
1. Support BCM54194 PHY SGMII Internal Loopback test.
2. Return FAILED when M.2 module not present.
3. Clean up code.

Revision 1.1.6.8  2019/04/10 16:29:30  letsai
1. Fix ethernet mapping.
2. Support all BCM54194 phy in utilities.
3. Remove unused functions.

Revision 1.1.6.7  2019/04/09 16:10:40  letsai
1. Support all BCM54194 PHY (0~3) Register Test.
2. Let utilities can dump each phy registers.
3. Check link status for each phy and each port(upstream and downstream).

Revision 1.1.6.6  2019/04/06 01:36:14  letsai
1. Remove unused functions and files.
2. Fix BCM54194 SFP External loopback test.
3. Fix BCM54194 Register test.
4. Fix Voltage Margin Utility.
5. Add function to show system information.

Revision 1.1.6.5  2019/04/03 18:30:36  letsai
Add utility to check link status

Revision 1.1.6.4  2019/03/30 00:56:02  letsai
1. Add USB console detect utility.
2. Modify FAN utility.
3. Remove unused items.
4. Fix BCM54194 phy register test.

Revision 1.1.6.3  2019/03/25 18:37:36  letsai
Modified eth and port number

Revision 1.1.6.2  2019/03/14 03:48:35  letsai
Initial check in.



$Endlog$
*/
