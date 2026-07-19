/* $Id: diag_ge_phy_88E1548L_test.c,v 1.3 2015/02/14 12:48:41 kodko Exp $ 
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/woodlawn_linux/diag_ge_phy_88E1548L_test.c,v $
 *-----------------------------------------------------------------------------
 * diag_ge_phy_88E1548L_test.c - Menu for Woodlawn PHY 88E1548L
 *
 * February 2012, Leslie Chen
 * Copyright (c) 2013 - 2015 by Cisco Systems, Inc.
 * All rights reserved.
 *-----------------------------------------------------------------------------
 */

#include "error.h"
#include "common.h"
#include "types.h"
#include "menu.h"
#include "diag_ge_phy_88E1548L_lib.h"
#include "diag_ge_phy_88E1548L_test.h"
#include "platform_eth.h"
#include "diag_common_drv.h"
#include "common_utils.h"
#include "diag_fpga_lib.h"
#include "platform_ext_lpbk.h"

#define MF_1    (MF_CONTINUOUS | MF_DOGRP)
#define MF_2    (MF_1 | MF_DOALL)
#define MF_3    (MF_2 | MF_SHOW_ERRCOUNT)
#define MF_4    (MF_1 | MF_SHOW_ERRCOUNT)

int ge_phy_88E1548L_test(int);
int ge_88E1548_do_all_wrapper(void);
int diag_ge_led_toggle(int, int, int);

static int phy_register_tests(int, uint, const reg_info_t *);
static int ge_phy_88E1548L_utility(int);
static int ge_phy_88E1548L_register_test(int);
static int ge_phy_88E1548L_internal_loopback_test(int);
static int ge_phy_88E1548L_copper_ext_loopback_test(int);
static int ge_phy_88E1548L_sfp_ext_loopback_test(int);
static int ge_phy_ptp_test(int);
int dump_phy_88E1548L_registers(void);
int alter_phy_88E1548L_register(void);
static int has_second_phy(void);
int switch_led_mode(void);
static int setup_1548_test_mode(void);
static int setup_1548_pseudorandom_packet(void);
int force_led_on(void);
int force_led_off(void);
int force_led_blink(void);
int led_speed_blink(void);
static void print_input_msg(int *, int *, int *, int);
int enable_88e1548_ptp_engine(int);
int en_88e1548_ptp_per_port(int, int);
int verify_1548_drift_adjustment_mode(void);
int verify_1548_clk_trig_in(void);
int config_1548_gen_clk_out(void);
int config_1548_gen_trig_out(void);
int dump_phy_88E1548_ptp_reg(void);
int alter_phy_88E1548_ptp_reg(void);
extern void msleep(unsigned long);

static char *sku_4ge_1xaui_cop_lpbk_str = "Copper GE0~GE3 External Loopback Test";
static char *sku_4ge_1xaui_sfp_lpbk_str = "SFP0~SFP3 External Loopback Test";
static char *sku_6ge_cop_lpbk_phy0_str  = "Copper GE2~GE5 External Loopback Test";
static char *sku_6ge_cop_lpbk_phy1_str  = "Copper GE0~GE1 External Loopback Test";
static char *sku_6ge_sfp_lpbk_phy0_str  = "SFP2~SFP5 External Loopback Test";
static char *sku_6ge_sfp_lpbk_phy1_str  = "SFP0~SFP1 External Loopback Test";

/* Sub Menu used for GE phy 88E1548L tests.*/
static submenu_xtable_t ge_phy_88E1548L_tests_submenu_table[] = {
    {"PHY Utilities", (type_t(*)())ge_phy_88E1548L_utility,   FALSE,
       0, NULL, 0, (type_t(*)())ge_phy_88E1548L_utility,   TRUE}, 
    {"PHY 0 Register Test", (type_t(*)())ge_phy_88E1548L_register_test,   MRVL_1548_PHY0,
       MF_3, (type_t(*)())0, 0, (type_t(*)())0,   0},
    {"PHY 1 Register Test", (type_t(*)())ge_phy_88E1548L_register_test,   MRVL_1548_PHY1,
       MF_3, (type_t(*)())has_second_phy, 0, (type_t(*)())0,   0},
    {"PHY 0 Internal Loopback Test", (type_t(*)())ge_phy_88E1548L_internal_loopback_test,
       MRVL_1548_PHY0,
       0, (type_t(*)())0, 0, (type_t(*)())0,   0},
    {"PHY 1 Internal Loopback Test", (type_t(*)())ge_phy_88E1548L_internal_loopback_test,
       MRVL_1548_PHY1,
       0, (type_t(*)())has_second_phy, 0, (type_t(*)())0,   0},
    {"Copper Port GE0~GE3 External Loopback Test", (type_t(*)())ge_phy_88E1548L_copper_ext_loopback_test,
       MRVL_1548_PHY0,
       MF_3, (type_t(*)())0, 0, (type_t(*)())0,   0},
    {"Copper Port GE4~GE5 External Loopback Test", (type_t(*)())ge_phy_88E1548L_copper_ext_loopback_test,
       MRVL_1548_PHY1,
       MF_3, (type_t(*)())has_second_phy, 0, (type_t(*)())0,   0},
    {"SFP0~SFP3 External Loopback Test", (type_t(*)())ge_phy_88E1548L_sfp_ext_loopback_test,
       MRVL_1548_PHY0,
       MF_3, (type_t(*)())0, 0, (type_t(*)())0,   0},
    {"SFP4~SFP5 External Loopback Test", (type_t(*)())ge_phy_88E1548L_sfp_ext_loopback_test,
       MRVL_1548_PHY1,
       MF_3, (type_t(*)())has_second_phy, 0, (type_t(*)())0,   0},
    {"Copper Port GE0~GE3 PTP Verification Test", (type_t(*)())ge_phy_ptp_test,
       MRVL_1548_PHY0,
       MF_3, (type_t(*)())0, 0, (type_t(*)())0,   0},
    {"Copper Port GE4~GE5 PTP Verification Test", (type_t(*)())ge_phy_ptp_test,
       MRVL_1548_PHY1,
       MF_3, (type_t(*)())has_second_phy, 0, (type_t(*)())0,   0},
};

#define GE_PHY_88E1548L_TESTS_SUBMENU_TABLE_SIZE (sizeof(ge_phy_88E1548L_tests_submenu_table) / \
                                    sizeof(submenu_xtable_t))

/*
 * Primary & secondary submenu items (filled in from xtable)
 */
static mitem_t ge_phy_88E1548L_tests_primary_items[GE_PHY_88E1548L_TESTS_SUBMENU_TABLE_SIZE +
                                     MAX_BASE_ITEMS];
static mitem_t ge_phy_88E1548L_tests_secondary_items[GE_PHY_88E1548L_TESTS_SUBMENU_TABLE_SIZE +
                                     MAX_BASE_ITEMS];

menuinfo_t ge_phy_88E1548L_subtest_menu = {
    "%s Subtest Menu",
    0,                                /* mtparam added by init_empty_menu */
    (PFT)show_endnote,                /* notes missing WICs in combos */
    0,                                /* use generic prompt */
    0,                                /* size (bumped by add_menu_item() */
    ge_phy_88E1548L_tests_primary_items,
};
menuinfo_t *ge_phy_88E1548L_submenup = &ge_phy_88E1548L_subtest_menu;

/*-------------------------------------------------------------------------------------*/

/* List of GE phy 88E1548L Utilities */
static submenu_xtable_t ge_phy_88E1548L_util_items[] = {
    {"Dump PHY Registers", (type_t(*)())dump_phy_88E1548L_registers, 0, 0, (type_t(*)())0,
     0, (type_t(*)())0, 0},
    {"Alter PHY Register", (type_t(*)())alter_phy_88E1548L_register, 0, 0,
     (type_t(*)())0, 0, (type_t(*)())0, 0},
    {"10M Pseudorandom Packet", (type_t(*)())setup_1548_pseudorandom_packet, 0, 0,                                                             (type_t(*)())0, 0, (type_t(*)())0, 0},
    {"1000M PHY Test Mode", (type_t(*)())setup_1548_test_mode, 0, 0,
     (type_t(*)())0, 0, (type_t(*)())0, 0},
    {"Switch PHY LED Mode", (type_t(*)())switch_led_mode, 0, 0,
     (type_t(*)())0, 0, (type_t(*)())0, 0},
    {"PHY LED speed blink", (type_t(*)())led_speed_blink, 0, 0,
     (type_t(*)())0, 0, (type_t(*)())0, 0},
    {"Force turn on PHY LED", (type_t(*)())force_led_on, 0, 0,
     (type_t(*)())0, 0, (type_t(*)())0, 0},
    {"Force turn off PHY LED", (type_t(*)())force_led_off, 0, 0,
     (type_t(*)())0, 0, (type_t(*)())0, 0},
    {"Force PHY LED blink", (type_t(*)())force_led_blink, 0, 0,
     (type_t(*)())0, 0, (type_t(*)())0, 0},
    {"PTP CLK/TRIG Verification", (type_t(*)())verify_1548_drift_adjustment_mode, 0, 0,
     (type_t(*)())0, 0, (type_t(*)())0, 0},
    {"Dump PTP Register", (type_t(*)())dump_phy_88E1548_ptp_reg, 0, 0,
     (type_t(*)())0, 0, (type_t(*)())0, 0},
    {"Alter PTP Register", (type_t(*)())alter_phy_88E1548_ptp_reg, 0, 0,
     (type_t(*)())0, 0, (type_t(*)())0, 0},
    {"MRVL1548 CLK/TRIG In Verification", (type_t(*)())verify_1548_clk_trig_in, 0, 0,
     (type_t(*)())0, 0, (type_t(*)())0, 0},
    {"Config MRVL1548 CLK Generation", (type_t(*)())config_1548_gen_clk_out, 0, 0,
     (type_t(*)())0, 0, (type_t(*)())0, 0},
    {"Config MRVL1548 TRIG Generation", (type_t(*)())config_1548_gen_trig_out, 0, 0,
     (type_t(*)())0, 0, (type_t(*)())0, 0}, 
};

#define GE_PHY_88E1548L_TESTS_UTIL_SIZE (sizeof(ge_phy_88E1548L_util_items) / \
                                    sizeof(submenu_xtable_t))

/*
 * ge phy 88E1548L util items (filled in from xtable)
 */
static mitem_t ge_phy_88E1548L_tests_primary_util_items[GE_PHY_88E1548L_TESTS_UTIL_SIZE +
                                     MAX_BASE_ITEMS];
static mitem_t ge_phy_88E1548L_tests_secondary_util_items[GE_PHY_88E1548L_TESTS_UTIL_SIZE +
                                     MAX_BASE_ITEMS];

/*
 * GE phy 88E1548L Utils submenu
 */
menuinfo_t ge_phy_88E1548L_util_menu = {
    "GE PHY 88E1548L Utility Menu",
    0,
    (PFT)show_endnote,
    0,
    0,
    ge_phy_88E1548L_tests_primary_util_items,
};

menuinfo_t *ge_phy_88E1548L_util_menup = &ge_phy_88E1548L_util_menu;


/******************************************************************************
 *
 * Function: ge_phy_88E1548L_test
 *
 * Description: Main entrance for 88E1548 menu
 *
 * Inputs      : port - port number
 * Outputs     : PASSED / FAILED
 *
 *******************************************************************************/
int ge_phy_88E1548L_test (int show_menu)
{
    int sku_id;

    /* Change the title based on SKU ID */
    sku_id = get_sku_id();

    if (sku_id == WOODLAWN_4GE_1XAUI) {
        ge_phy_88E1548L_tests_submenu_table[5].x_title = sku_4ge_1xaui_cop_lpbk_str;
        ge_phy_88E1548L_tests_submenu_table[7].x_title = sku_4ge_1xaui_sfp_lpbk_str;
    } else if (sku_id == WOODLAWN_6GE) {
        ge_phy_88E1548L_tests_submenu_table[5].x_title = sku_6ge_cop_lpbk_phy0_str;
        ge_phy_88E1548L_tests_submenu_table[6].x_title = sku_6ge_cop_lpbk_phy1_str;
        ge_phy_88E1548L_tests_submenu_table[7].x_title = sku_6ge_sfp_lpbk_phy0_str;
        ge_phy_88E1548L_tests_submenu_table[8].x_title = sku_6ge_sfp_lpbk_phy1_str;
    }

    build_primary_submenu(ge_phy_88E1548L_tests_submenu_table,
                          GE_PHY_88E1548L_TESTS_SUBMENU_TABLE_SIZE,
                          "GE PHY 88E1548L", &ge_phy_88E1548L_submenup);
    build_secondary_submenu(ge_phy_88E1548L_tests_submenu_table,
                           GE_PHY_88E1548L_TESTS_SUBMENU_TABLE_SIZE,
                           ge_phy_88E1548L_tests_secondary_items);

    if (show_menu) {
        menu(ge_phy_88E1548L_submenup, ge_phy_88E1548L_tests_secondary_items, '\0' );
    } else {
        menu_exec_doall_diags(ge_phy_88E1548L_submenup);
    }
    return (PASSED);
}


/*******************************************************************************
 *
 * Function    : ge_88E1548_do_all_wrapper
 * Description : Wrapper for GE PHY 88E1548 do all test items
 * Inputs      : none
 * Outputs     : PASSED / FAILED
 *
 *******************************************************************************
 */
int ge_88E1548_do_all_wrapper (void)
{
    int rc = PASSED;

    if (ge_phy_88E1548L_register_test(MRVL_1548_PHY0) == FAILED) {
        rc = FAILED;
    }

    if (has_second_phy()) {
        if (ge_phy_88E1548L_register_test(MRVL_1548_PHY1) == FAILED) {
            rc = FAILED;
        }
    }

    if (ge_phy_88E1548L_copper_ext_loopback_test(MRVL_1548_PHY0) == FAILED) {
        rc = FAILED;
    }

    if (has_second_phy()) {
        if (ge_phy_88E1548L_copper_ext_loopback_test(MRVL_1548_PHY1) == FAILED) {
            rc = FAILED;
        }
    }

    if (ge_phy_88E1548L_sfp_ext_loopback_test(MRVL_1548_PHY0) == FAILED) {
        rc = FAILED;
    }

    if (has_second_phy()) {
        if (ge_phy_88E1548L_sfp_ext_loopback_test(MRVL_1548_PHY1) == FAILED) {
            rc = FAILED;
        }
    }

    return (rc);
}


/***********************************************************************
 *
 * Function: diag_ge_led_toggle
 *
 * Description: This function toggles LED on GE coppper RJ-45 ports
 *
 * Inputs: port - GE port number
 *         LED  - which LED (Speed[Left]/Link[Right])
 *         on   - TRUE to turn on, FALSE to turn off
 *
 * Outputs: PASSED/FAILED
 *
 *************************************************************************/
int diag_ge_led_toggle (int port, int led, int on)
{
    int phy_addr, page_no, port_num, reg_val, port_map, sku_id, bus_id;

    sku_id = get_sku_id();

    port_num = port;
    page_no  = MRVL_88E1548_LED_CTRL_PAGE;

    /* New SKU for 4 ports and 6 ports GE*/
    if (sku_id == WOODLAWN_6GE) {
        port_map = two_phy_ge_mapping_phy_port[port_num];
        if ((MRVL_1548_GE2 <= port_num) && (port_num <= MRVL_1548_GE5)) {
            phy_addr = MRVL_88E1548_PHY0_SMI_ADDR + port_map;
        } else {
            /* One new SKU just have 4 GE ports */
            phy_addr = MRVL_88E1548_PHY1_SMI_ADDR + port_map;
        }
    } else if (sku_id == WOODLAWN_4GE_1XAUI) {
        port_map = one_phy_ge_mapping_phy_port[port_num];
        phy_addr = MRVL_88E1548_PHY0_SMI_ADDR + port_map;
    } else {
        printf("\nUnrecognized SKU: %#x\n", sku_id);
        return (FAILED);
    }


    bus_id = get_88e1548_bus_id(phy_addr);

    /* Need to write page register for set page */
    if (woodlawn_phy_reg_wr(bus_id, phy_addr, MRV88E1548L_PAGE_ADDRESS_REG, page_no)
        == FAILED) {
        printf("Set Register Page %d Fail !\n", page_no);
        return (FAILED);
    }

    /* Configures LED polarity */
    if (woodlawn_phy_reg_wr(bus_id, phy_addr, MRVL_88E1548_LED_POLARITY_REG,
        MRVL_88E1548_LED_POLARITY) == FAILED) {
        printf("Alter port %d polarity fails\n", port_num);
        return (FAILED);
    }

    /* read original register value*/
    if (woodlawn_phy_reg_rd(bus_id, phy_addr, MRVL_88E1548_LED_FUNCTION_REG,
                            &reg_val) == FAILED) {
        printf("Dump port %d fails\n", port_num);
        return (FAILED);
    }

    /* Force on LED_1 */
    reg_val |= MRVL_88E1548_LED1_ON;

    if (led == DIAG_PHY_LED_SPEED) { /* LED 0 */
        reg_val &= ~(0x000F);
        if (on == TRUE) {
            reg_val |= MRVL_88E1548_LED0_ON;
        } else {
            reg_val |= MRVL_88E1548_LED0_OFF;
        }
    } else { /* LED 2 */
        reg_val &= ~(0x0F00);
        if (on == TRUE) {
            reg_val |= MRVL_88E1548_LED2_ON;
        } else {
            reg_val |= MRVL_88E1548_LED2_OFF;
        }
    }

    /* Alter the current register with new vlaue */
    if (woodlawn_phy_reg_wr(bus_id, phy_addr, MRVL_88E1548_LED_FUNCTION_REG,
                            reg_val) == FAILED) {
        printf("Alter port %d fails\n", port_num);
        return (FAILED);
    }

    return (PASSED);
}


/*******************************************************************************
 *
 * Function    : ge_phy_88E1548L_utility
 * Description :
 * Inputs      : menu_option - display menu instead of running all temp. sensor tests.
 * Outputs     : PASSED / FAILED
 *
 *******************************************************************************
 */

static int ge_phy_88E1548L_utility (int show_menu)
{
    build_primary_submenu(ge_phy_88E1548L_util_items, GE_PHY_88E1548L_TESTS_UTIL_SIZE,
                           "GE PHY 88E1548L Utilities Menu", &ge_phy_88E1548L_util_menup);
    build_secondary_submenu(ge_phy_88E1548L_util_items, GE_PHY_88E1548L_TESTS_UTIL_SIZE,
                            ge_phy_88E1548L_tests_secondary_util_items);

    if (show_menu) {
        menu(ge_phy_88E1548L_util_menup, ge_phy_88E1548L_tests_secondary_util_items, '\0' );
    } else {
        menu_exec_doall_diags(ge_phy_88E1548L_util_menup);
        prcomplete(testpass, errcount, (char *)0);
    }

    return (PASSED);
}

static const reg_info_t marvell_88e1548L_reg_page0[] = {   /* Page 0*/
    {"Copper Control",          0x00, READ_WRITE, {2}, 0x0000, 0x1940},
    {"Copper Status",           0x01, READ_ONLY,  {2}, 0x0000, 0x7949},
    {"PHY ID1",                 0x02, READ_ONLY,  {2}, 0x0000, 0x0141},
    {"PHY ID2",                 0x03, READ_ONLY,  {2}, 0x0000, 0x0EBF},
    {"Copper Auto-Neg",         0x04, READ_WRITE, {2}, 0x0000, 0x01E1},
    {"Copper Link-P Abil",      0x05, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"Copper Auto-Neg Exp",     0x06, READ_ONLY,  {2}, 0x0000, 0x0004},
    {"Copper Next Page",        0x07, READ_WRITE, {2}, 0xB7FF, 0x2001},
    {"Copper Link Partner",     0x08, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"1000BT Control",          0x09, READ_WRITE, {2}, 0xF2FF, 0x0F00},
    {"1000BT Status",           0x0A, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"MMD Access Control",      0x0D, READ_WRITE, {2}, 0xC000, 0x0000},
    {"MMd Access Address/Data", 0x0E, READ_WRITE, {2}, 0xFFFF, 0x0000},
    {"Extended Status",         0x0F, READ_ONLY,  {2}, 0x0000, 0x3000},
    {"Copper Spec Cntl1",       0x10, READ_WRITE, {2}, 0x0000, 0x3360},
    {"Copper Spec Ststus",      0x11, READ_ONLY,  {2}, 0x0000, 0xC040},
    {"Copper Spec Intr Ena",    0x12, READ_WRITE, {2}, 0xFFFF, 0x0000},
    {"Copper Intr Status",      0x13, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"Copper Spec Cntl2",       0x14, READ_WRITE, {2}, 0xFFDF, 0x0020},
    {"Copper Spec Rx Err",      0x15, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"Page Register",           0x16, READ_WRITE, {2}, 0x0000, 0x0000},
    {"Global Intr Status",      0x17, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"Copper Spec Cntl3",       0x1A, READ_WRITE, {2}, 0xFEFF, 0x0040},
    {"end",                     0x00, 0, {0}, 0, 0},
};

static const reg_info_t marvell_88e1548L_reg_page1[] = {   /* Page 1*/
    {"Fiber Control",           0x00, READ_WRITE, {2}, 0x1100, 0x1140},
    {"Fiber Status",            0x01, READ_ONLY,  {2}, 0x0000, 0x6149},
    {"PHY ID1",                 0x02, READ_ONLY,  {2}, 0x0000, 0x0141},
    {"PHY ID2",                 0x03, READ_ONLY,  {2}, 0x0000, 0x0EB0},
    {"Fiber Auto-Neg",          0x04, READ_WRITE, {2}, 0x0000, 0x0001},
    {"Fiber Link-P Abil",       0x05, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"Fiber Auto-Neg Exp",      0x06, READ_ONLY,  {2}, 0x0000, 0x0004},
    {"Fiber Next Page",         0x07, READ_WRITE, {2}, 0xB7FF, 0x2001},
    {"Fiber Link Partner",      0x08, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"Extended Status",         0x0F, READ_ONLY,  {2}, 0x0000, 0x3000},
    {"Fiber Spec Cntl1",        0x10, READ_WRITE, {2}, 0xFC8C, 0x8084},
    {"Fiber Spec Ststus",       0x11, READ_ONLY,  {2}, 0x0000, 0x8000},
    {"Fiber Spec Intr Ena",     0x12, READ_WRITE, {2}, 0x7F80, 0x0000},
    {"Fiber Intr Status",       0x13, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"Fiber Spec Rx Err",       0x15, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"Page Register",           0x16, READ_WRITE, {2}, 0x0000, 0x0000},
    {"PRBS Control",            0x17, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"PRBS Err Counter LSB",    0x18, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"PRBS Err Counter MSB",    0x19, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"Fiber Spec Cntl2",        0x1A, READ_WRITE, {2}, 0x0000, 0x0000},
    {"end",                     0x00, 0, {0}, 0, 0},
};

static const reg_info_t marvell_88e1548L_reg_page2[] = {   /* Page 2*/
    {"Mac Specific Control 1",      0x10, READ_WRITE, {2}, 0xDBC8, 0x4004},
    {"Mac Specific Interrupt En",   0x12, READ_WRITE,  {2}, 0x008C, 0x0000},
    {"Mac Specific Status",         0x13, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"Mac Specific Control 2",      0x15, READ_WRITE,  {2}, 0x4008, 0x1046},
    {"Page Register",               0x16, READ_WRITE, {2}, 0x0000, 0x0000},
    {"end",                         0x00, 0, {0}, 0, 0},
};

static const reg_info_t marvell_88e1548L_reg_page3[] = {   /* Page 3*/
    {"LED Func Control",              0x10, READ_WRITE, {2}, 0xFFFF, 0x1777},
    {"LED Polarity Control",          0x11, READ_WRITE,  {2}, 0xFFFF, 0x8800},
    {"LED Timer Control",             0x12, READ_WRITE,  {2}, 0xF70F, 0x4905},
    {"LED Func Control and Polarity", 0x13, READ_WRITE,  {2}, 0xEFFF, 0x0073},
    {"Page Register",                 0x16, READ_WRITE, {2}, 0x0000, 0x0000},
    {"end",                           0x00, 0, {0}, 0, 0},
};

static const reg_info_t marvell_88e1548L_reg_page4[] = {   /* Page 4*/
    {"QSGMII Control",                        0x00, READ_WRITE, {2}, 0x0000, 0x1140},
    {"QSGMII Status",                         0x01, READ_ONLY,  {2}, 0x0000, 0x7949},
    {"QSGMII Auto Negotiation Ad",            0x04, READ_ONLY,  {2}, 0x0000, 0x0001},
    {"QSGMII Link Partner Ability",           0x05, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"QSGMII Auto Negotiation Expansion",     0x06, READ_ONLY , {2}, 0x0000, 0x0000},
    {"QSGMII Specific Control",               0x10, READ_WRITE, {2}, 0x0000, 0xC4FD},
    {"QSGMII Specific Status",                0x11, READ_ONLY,  {2}, 0x0000, 0xC040},
    {"QSGMII Specific Interrupt Enable",      0x12, READ_WRITE, {2}, 0xFFFF, 0x0000},
    {"QSGMII Interrupt Status",               0x13,  READ_ONLY, {2}, 0x0000, 0x0000},
    {"QSGMII RX_ER Byte Capture",             0x14, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"QSGMII Specific Receive Error Counter", 0x15, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"Page Register",                         0x16, READ_WRITE, {2}, 0x0000, 0x0000},
    {"PRBS Control",                          0x17, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"PRBS Err Counter LSB",                  0x18, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"PRBS Err Counter MSB",                  0x19,  READ_ONLY,  {2}, 0x0000, 0x0000},
    {"QSGMII Global Control 1",               0x1A, READ_WRITE, {2}, 0x0000, 0xC000},
    {"QSGMII Global Control 2",               0x1B, READ_WRITE, {2}, 0x0000, 0x3E00},
    {"end",                                   0x00, 0, {0}, 0, 0},
};

static const reg_info_t marvell_88e1548L_reg_page5[] = {   /* Page 5*/
    {"Adv VCT Tx to MDI[0] Rx Coupling",               0x10,  READ_ONLY,  {2}, 0x0000, 0x0000},
    {"Adv VCT Tx to MDI[1] Rx Coupling",               0x11,  READ_ONLY,  {2}, 0x0000, 0x0000},
    {"Adv VCT Tx to MDI[2] Rx Coupling",               0x12,  READ_ONLY,  {2}, 0x0000, 0x0000},
    {"Adv VCT Tx to MDI[3] Rx Coupling",               0x13,  READ_ONLY,  {2}, 0x0000, 0x0000},
    {"1000BASE-T Pair Skew",                           0x14,  READ_ONLY,  {2}, 0x0000, 0x0000},
    {"1000BASE-T Swap and Polarity",                   0x15,  READ_ONLY,  {2}, 0x0000, 0x0000},
    {"Page Register",                                  0x16, READ_WRITE,  {2}, 0x0000, 0x0000},
    {"Adv VCT Control",                                0x17,  READ_WRITE, {2}, 0x3FFF, 0x0000},
    {"Adv VCT Sample Point Dist",                      0x18, READ_ONLY,   {2}, 0x0000, 0x0000},
    {"Adv VCT Cross Pair Positive Threshold",          0x19,  READ_WRITE, {2}, 0x0000, 0x0000},
    {"Adv VCT Same Pair 0 and 1",                      0x1A, READ_WRITE,  {2}, 0x7F7F, 0x0104},
    {"Adv VCT Same Pair 2 and 3",                      0x1B, READ_WRITE,  {2}, 0x7F7F, 0x0F12},
    {"Adv VCT Same Pair 4 and Transmit Pulse Control", 0x1C, READ_WRITE,  {2}, 0x3F7F, 0x0A0C},
    {"end",                                            0x00, 0, {0}, 0, 0},
};

static const reg_info_t marvell_88e1548L_reg_page6[] = {   /* Page 6*/
    {"Packet Generation",             0x10, READ_WRITE,  {2}, 0x0000, 0x0000},
    {"CRC Counters",                  0x11, READ_ONLY,   {2}, 0x0000, 0x0000},
    {"Checker Control",               0x12, READ_WRITE,  {2}, 0x0007, 0x0000},
    {"General Control",               0x14, READ_WRITE,  {2}, 0x0000, 0x0200},
    {"Page Register",                 0x16, READ_WRITE,  {2}, 0x0000, 0x0000},
    {"Late Collision Counters 1 & 2", 0x17,  READ_ONLY,  {2}, 0x0000, 0x0000},
    {"Late Collision Counters 3 & 4", 0x18,  READ_ONLY,  {2}, 0x0000, 0x0000},
    {"Late Collision Window Adjust",  0x19, READ_WRITE,  {2}, 0x1F00, 0x0000},
    {"Misc Test",                     0x1A,  READ_WRITE, {2}, 0x9FA0, 0x1900},
    {"Temperature Sensor",            0x1B, READ_ONLY,   {2}, 0x0000, 0x0000},
    {"end",                           0x00, 0, {0}, 0, 0},
};

static const reg_info_t marvell_88e1548L_reg_page7[] = {   /* Page 7*/
    {"PHY Cable Diag Pair 0 Length",          0x10, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"PHY Cable Diag Pair 1 Length",          0x11, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"PHY Cable Diag Pair 2 Length",          0x12, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"PHY Cable Diag Pair 3 Length",          0x13, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"PHY Cable Diag Results",                0x14, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"PHY Cable Diag Control",                0x15, READ_WRITE, {2}, 0x6400, 0x4000},
    {"Page Register",                         0x16, READ_WRITE, {2}, 0x0000, 0x0000},
    {"Adv VCT Cross Pair Negative Threshold", 0x19,  READ_WRITE, {2}, 0x7F7F, 0x0104},
    {"Adv VCT Same Pair 0 and 1",             0x1A, READ_WRITE, {2}, 0x7F7F, 0x0F12},
    {"Adv VCT Same Pair 2 and 3",             0x1B,  READ_WRITE, {2}, 0x7F7F, 0x0A0C},
    {"Adv VCT Same Pair 4",                   0x1C, READ_WRITE, {2}, 0x007F, 0x0006},
    {"end",                                   0x00, 0, {0}, 0, 0},
};

static const reg_info_t marvell_88e1548L_reg_page8[] = {   /* Page 8*/
    {"PTP Port Configuration 0",                0x00,  READ_ONLY,  {2}, 0x0000, 0x1000},
    {"PTP Port Configuration 1",                0x01, READ_ONLY,  {2}, 0x0000, 0x020C},
    {"PTP Port Configuration 2",                0x02, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"PTP Arrival 0 Time Port Status",          0x08, READ_ONLY,  {2}, 0x0000, 0x0dc0},
    {"PTP Arrival 0 Timer Register Byte 1 & 0", 0x09, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"PTP Arrival 0 Timer Register Byte 3 & 2", 0x0A,  READ_ONLY,  {2}, 0x0000, 0x0000},
    {"PTP Arrival 0 Sequence Identifier",       0x0B, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"PTP Arrival 1 Time Port Status",          0x0C, READ_ONLY, {2}, 0x0000, 0x0000},
    {"PTP Arrival 1 Timer Register Byte 1 & 0", 0x0D, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"PTP Arrival 1 Timer Register Byte 3 & 2", 0x0E, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"PTP Arrival 1 Sequence Identifier",       0x0F, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"Page Register",                           0x16, READ_WRITE, {2}, 0x0000, 0x0000},
    {"end",                                     0x00, 0, {0}, 0, 0},
};

static const reg_info_t marvell_88e1548L_reg_page9[] = {   /* Page 9*/
    {"PTP Depature Time Port Status",            0x00, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"PTP Departure Timer Reg Bytes 1 & 0",      0x01, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"PTP Departure Timer Reg Bytes 3 & 2",      0x02,  READ_ONLY,  {2}, 0x0000, 0x0000},
    {"PTP Departure Sequence Identifier Status", 0x03, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"PTP Port Discard Counter",                 0x05,  READ_ONLY,  {2}, 0x0000, 0x0000},
    {"Page Register",                            0x16, READ_WRITE, {2}, 0x0000, 0x0000},
    {"end",                                      0x00, 0, {0}, 0, 0},
};

static const reg_info_t marvell_88e1548L_reg_page12[] = {   /* Page 12*/
    {"TAI Global Config 0",            0x00, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"TAI Global Config 1",            0x01, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"TAI Global Config 2",            0x02, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"TAI Global Config 3",            0x03, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"TAI Global Config 4",            0x04, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"TAI Global Config 5",            0x05, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"TAI Global Config 8",            0x08, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"TAI Global Config 9",            0x09, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"Event Capture Reg Byte 1 & 0",   0x0A, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"Event Capture Reg Byte 3 & 2",   0x0B, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"TAI Global Config 12",           0x0C, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"TAI Global Config 13",           0x0D, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"PTP Global Time Reg Byte 1 & 0", 0x0E, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"PTP Global Time Reg Byte 3 & 2", 0x0F, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"Page Register",                  0x16, READ_WRITE, {2}, 0x0000, 0x0000},
    {"end",                            0x00, 0, {0}, 0, 0},
};

static const reg_info_t marvell_88e1548L_reg_page14[] = {   /* Page 14*/
    {"PTP Global Config 0",     0x00, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"PTP  Global Config 1",    0x01, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"PTP  Global Config 2",    0x02, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"PTP  Global Config 3",    0x03, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"PTP  Global Status",      0x08, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"Read Plus Command",       0x0E, READ_ONLY, {2} ,0x0000, 0x0000},
    {"Read Plus Data",          0x0F, READ_ONLY, {2}, 0x0000, 0x0000},
    {"Page Register",           0x16, READ_WRITE, {2}, 0x0000, 0x0000},
    {"end",                     0x00, 0, {0}, 0, 0},
};

static const reg_info_t marvell_88e1548L_reg_page16[] = {   /* Page 16*/
    {"Link Cypt Read Address",      0x00, READ_WRITE, {2}, 0x0000, 0x0000},
    {"Link Cypt Write Address",     0x01, READ_ONLY,  {2}, 0x0000, 0x7949},
    {"Link Cypt Data Lo",           0x02, READ_ONLY,  {2}, 0x0000, 0x0141},
    {"Link Cypt Data Hi",           0x03, READ_ONLY,  {2}, 0x0000, 0x0dc0},
    {"Page Register",               0x16, READ_WRITE, {2}, 0x0000, 0x0000},
    {"end",                         0x00, 0, {0}, 0, 0},
};

static const reg_info_t marvell_88e1548L_reg_page18[] = {   /* Page 18*/
    {"Packet Generation",             0x10, READ_WRITE, {2}, 0x0000, 0x1940},
    {"CRC Counters",                  0x11, READ_ONLY,  {2}, 0x0000, 0x7949},
    {"Packet Generation IPG Control", 0x13, READ_ONLY,  {2}, 0x0000, 0x0141},
    {"General Control Reg 1",         0x14, READ_ONLY,  {2}, 0x0000, 0x0dc0},
    {"Page Register",                 0x16, READ_WRITE, {2}, 0x0000, 0x0000},
    {"Link Disconnect Count",         0x19, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"RX_ER Byte Capture",            0x1A, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"General Control Reg 2",         0x1B, READ_ONLY,  {2}, 0x0000, 0x2000},
    {"end",                           0x00, 0, {0}, 0, 0},
};

static const reg_info_t marvell_88e1548L_reg_page20[] = {   /* Page 20*/
    {"Eye Monitor Control",                  0x10, READ_WRITE, {2}, 0x3940, 0x1940},
    {"Eye Monitor Measure Config",           0x14, READ_ONLY,  {2}, 0x0000, 0x7949},
    {"Page Register",                        0x16, READ_WRITE, {2}, 0x0000, 0x0000},
    {"Eye Monitor Sample Config",            0x17, READ_ONLY,  {2}, 0x0000, 0x0141},
    {"Eye Monitor Compare Err Counter(LSB)", 0x18, READ_ONLY,  {2}, 0x0000, 0x0dc0},
    {"Eye Monitor Compare Err Counter(MSB)", 0x19, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"end",                 0x00, 0, {0}, 0, 0},
};

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
 *******************************************************************************/
 
static int phy_register_tests (int phy_addr, uint page, const reg_info_t *reg_ptr )
{
    uint32_t ix;
    uint retval, ret_val, save_val, readval;
    uint data, temp, tst_offset;
    int bus_id;
    
    readval = 0;
    retval = PASSED;
    ret_val = PASSED;

    if (phy_addr & 0x4) {
        bus_id = SMI_BUS_1;
    } else {
        bus_id = SMI_BUS_0;
    }
    
    /* Need to write page register for set page */
    retval = woodlawn_phy_reg_wr(bus_id, phy_addr, MRV88E1548L_PAGE_ADDRESS_REG, page);
    if (retval == FAILED) {
        cterr('f', 0, "%s(): Set Register Page %d Fail !\n", __FUNCTION__, page);
        return (FAILED);
    }

    while (reg_ptr->size.size != 0) {

        retval = woodlawn_phy_reg_rd(bus_id, phy_addr, reg_ptr->offset, (int *)&save_val);
        if (retval == FAILED) {
            cterr('f', 0, "%s(): Error reading %s register offset %#x"
                  "phy_addr %d\n", __FUNCTION__,  reg_ptr->name,
                  reg_ptr->offset, phy_addr);
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

                retval = woodlawn_phy_reg_wr(bus_id, phy_addr, tst_offset, temp);

                /* Read back */
                if (retval == PASSED) {
                    ret_val = woodlawn_phy_reg_rd(bus_id, phy_addr, tst_offset,
                              (int *)&readval);
                }

                if ( ((readval & reg_ptr->mask) != temp) ||
                    (retval == FAILED) || (ret_val == FAILED) ) {

                    cterr('f', 0, "%s(): Ripple one test failed when accessing %s "
                          "Register offset %#x, phy_addr %d,Expect %#x, Read %#x",
                          __FUNCTION__, reg_ptr->name, tst_offset, phy_addr, temp,
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
                retval = woodlawn_phy_reg_wr(bus_id, phy_addr, tst_offset, temp);

                if (retval == PASSED) {
                    /* Read back */
                    ret_val = woodlawn_phy_reg_rd(bus_id, phy_addr, tst_offset,
                                                  (int *)&readval);
                }

                if (((readval & reg_ptr->mask) != temp) ||
                    (retval == FAILED) || (ret_val == FAILED)) {
                    cterr('f', 0, "%s(): Ripple one test failed when accessing %s "
                          "Register offset %#x, phy_addr %d, Expect %#x, Read %#x",
                          __FUNCTION__, reg_ptr->name, tst_offset, phy_addr, temp,
                          readval);
                    return (retval);
                }
            }

            /*
             * pattern test
             */
            data = PATTERN;
            for (ix = 0; ix < 2; ix++) {
                temp = data & reg_ptr->mask;
                /* Write to register under test */
                retval = woodlawn_phy_reg_wr(bus_id, phy_addr, tst_offset, temp);

                if (retval == PASSED) {
                    /* Read back */
                    ret_val = woodlawn_phy_reg_rd(bus_id, phy_addr, tst_offset,
                                                  (int *)&readval);
                }

                if (((readval & reg_ptr->mask) != temp) ||
                    (retval == FAILED) || (ret_val == FAILED)) {
                    cterr('f', 0, "%s(): Pattern test failed when accessing %s "
                          "Register offset %#x phy_addr %d, Expect %#x, "
                          "Read %#x", __FUNCTION__, reg_ptr->name, tst_offset,
                          phy_addr, temp, readval);
                    return (retval);
                }

                data = ~PATTERN; /* complement data pattern */
            }

            /*
             * restore original value
             */
            retval = woodlawn_phy_reg_wr(bus_id, phy_addr, tst_offset, save_val);
            if (retval == FAILED) {
                cterr('f', 0, "%s(): Error restoring %s register "
                      "offset %#x, phy_addr %d\n", __FUNCTION__,
                      reg_ptr->name, reg_ptr->offset, phy_addr);
                return (FAILED);
            }
        }
        reg_ptr++;
    }

    return (PASSED);
}

/******************************************************************************
 *  
 * Function: ge_phy_88E1548L_register_test
 *    
 * Description: This function performs the 88E1548L register test.
 *      
 * Inputs      : port - port number
 *
 * Outputs     : PASSED / FAILED
 *         
 ********************************************************************************/
static int ge_phy_88E1548L_register_test (int port)
{
    uint ix, phy_addr, no_of_phy;
    int id;
    testname("88E1548L PHY %d", port);

    /* First 88E1548L has 4 PHYs, while the second one only has 2 */
    if (port == MRVL_1548_PHY0) {
        no_of_phy = 4;
        phy_addr = MRVL_88E1548_PHY0_SMI_ADDR;
    } else {
        /* New SKU just have 4 GE ports */
        id = get_sku_id();
        if (id == WOODLAWN_4GE_1XAUI) {
            return (PASSED);
        } else {
            no_of_phy = 2;
            phy_addr = MRVL_88E1548_PHY1_SMI_ADDR;
        }
    }

    for (ix = 0; ix < no_of_phy; ix++) {
        if (phy_register_tests(phy_addr, MRV88E1548L_REG_PAGE_0, 
                        &marvell_88e1548L_reg_page0[0]) == FAILED) {
             cterr('f', 0, "Register Test on address %#x fails.", phy_addr);
            return (FAILED);
        }

        if (phy_register_tests(phy_addr, MRV88E1548L_REG_PAGE_8, 
                        &marvell_88e1548L_reg_page8[0]) == FAILED) {
             cterr('f', 0, "Register Test on address %#x fails.", phy_addr);
            return (FAILED);
        }

        phy_addr++;
    }
    
    return (PASSED);
}

/***********************************************************************
 *   
 * Function: ge_phy_ptp_test
 *       
 * Description: Do GE PHY 1588 loopback test
 *          
 * Inputs: phy - phy number
 *         
 * Outputs: PASSED/FAILED
 *              
 ***************************************************************************/
static int ge_phy_ptp_test (int phy)
{
    return (woodlawn_phy_lpbk_test(phy, PTP_SGMII_EXT_LPBK));
}

/***********************************************************************
 *  
 * Function: ge_phy_88E1548L_internal_loopback_test
 *    
 * Description: Do GE PHY internal loopback test
 *      
 * Inputs: phy - phy number
 *        
 * Outputs: PASSED/FAILED
 *          
 **************************************************************************/
static int ge_phy_88E1548L_internal_loopback_test (int phy)
{
    return (woodlawn_phy_lpbk_test(phy, MEDIA_PHY_INT_LPBK));
}

/***********************************************************************
 *    
 * Function: ge_phy_88E1548L_copper_ext_loopback_test
 *        
 * Description: Do GE PHY copper external loopback test
 *            
 * Inputs: phy - phy number
 *         
 * Outputs: PASSED/FAILED
 *               
 ***************************************************************************/
static int ge_phy_88E1548L_copper_ext_loopback_test (int phy)
{
    return (woodlawn_phy_lpbk_test(phy, SGMII_INT_EXT_LPBK));
}

/***********************************************************************
 *   
 * Function: ge_phy_88E1548L_sfp_ext_loopback_test
 *       
 * Description: Do GE PHY fiber external loopback test
 *          
 * Inputs: phy - phy number
 *         
 * Outputs: PASSED/FAILED
 *              
 ***************************************************************************/
static int ge_phy_88E1548L_sfp_ext_loopback_test (int phy)
{
    return (sfp_phy_ext_lpbk_test(phy));
}


/***********************************************************************
 *  
 *  Function: dump_phy_88E1548L_registers
 *    
 * Description: Display GE PHY registers
 *      
 * Inputs: None
 *       
 * Outputs: PASSED/FAILED
 *          
 *************************************************************************/
int dump_phy_88E1548L_registers (void)
{
    int reg_addr,phy_addr, page_no, port_num, reg_val, port_map;
    int sku_id, bus_id;
    
    sku_id = get_sku_id();

    /* Get the SKUs related port information */
    print_input_msg(&port_num, &reg_addr, &page_no, sku_id);

    printf("Dump Marvell 88E1548L GE%d Register.\n", port_num);

    if (sku_id == WOODLAWN_6GE_1XAUI) {
        /* Not official SKU */
        port_map = old_ge_mapping_phy_port[port_num];
        if (port_num < MRVL_1548_GE4) {
            phy_addr = MRVL_88E1548_PHY0_SMI_ADDR + port_map;
        } else {
            phy_addr = MRVL_88E1548_PHY1_SMI_ADDR + port_map;
        }
    } else {
        /* New SKU for 4 ports and 6 ports GE*/
        if (sku_id == WOODLAWN_6GE) {
            port_map = two_phy_ge_mapping_phy_port[port_num];
            if ((MRVL_1548_GE2 <= port_num) && (port_num <= MRVL_1548_GE5)) {
                phy_addr = MRVL_88E1548_PHY0_SMI_ADDR + port_map;
            } else {
                phy_addr = MRVL_88E1548_PHY1_SMI_ADDR + port_map;
            }
        } else if (sku_id == WOODLAWN_4GE_1XAUI) {
            port_map = one_phy_ge_mapping_phy_port[port_num];
            phy_addr = MRVL_88E1548_PHY0_SMI_ADDR + port_map;
        }
    }

    bus_id = get_88e1548_bus_id(phy_addr);
    
    /* Need to write page register for set page */
    if (woodlawn_phy_reg_wr(bus_id, phy_addr, MRV88E1548L_PAGE_ADDRESS_REG, page_no)
        == FAILED) {
        printf("Set Register Page %d Fail !\n", page_no);
        return (FAILED);
    }

    if (woodlawn_phy_reg_rd(bus_id, phy_addr, reg_addr, &reg_val) == FAILED) {
        printf("Dump port %d fails\n", port_num);
        return (FAILED);
    } else {
        printf("value: reg %#.4x, data %#.4x\n", reg_addr, reg_val);
    }
    return (PASSED);
}

/***********************************************************************
 *  
 * Function: alter_phy_88E1548L_register
 *    
 * Description: Alter GE PHY registers
 *      
 * Inputs: None
 *        
 * Outputs: PASSED/FAILED
 *          
 *************************************************************************/
int alter_phy_88E1548L_register (void)
{
    int reg_addr,phy_addr, page_no, port_num, reg_val, port_map, sku_id, bus_id;
    
    sku_id = get_sku_id();

    /* Get the SKUs related port information */
    print_input_msg(&port_num, &reg_addr, &page_no, sku_id);

    printf("Alter Marvell 88E1548L GE%d Register.\n", port_num);
        
    if (sku_id == WOODLAWN_6GE_1XAUI) {
        /* Not official SKU */
        port_map = old_ge_mapping_phy_port[port_num];
        if (port_num < MRVL_1548_GE4) {
            phy_addr = MRVL_88E1548_PHY0_SMI_ADDR + port_map;
        } else {
            phy_addr = MRVL_88E1548_PHY1_SMI_ADDR + port_map;
        }
    } else {
        /* New SKU for 4 ports and 6 ports GE*/
        if (sku_id == WOODLAWN_6GE) {
            port_map = two_phy_ge_mapping_phy_port[port_num];
            if ((MRVL_1548_GE2 <= port_num) && (port_num <= MRVL_1548_GE5)) {
                phy_addr = MRVL_88E1548_PHY0_SMI_ADDR + port_map;
            } else {
                /* One new SKU just have 4 GE ports */
                phy_addr = MRVL_88E1548_PHY1_SMI_ADDR + port_map;
            }
        } else if (sku_id == WOODLAWN_4GE_1XAUI) {
            port_map = one_phy_ge_mapping_phy_port[port_num];
            phy_addr = MRVL_88E1548_PHY0_SMI_ADDR + port_map;
        }
    }

    bus_id = get_88e1548_bus_id(phy_addr);
    
    /* Need to write page register for set page */
    if (woodlawn_phy_reg_wr(bus_id, phy_addr, MRV88E1548L_PAGE_ADDRESS_REG, page_no)
        == FAILED) {
        printf("Set Register Page %d Fail !\n", page_no);
        return (FAILED);
    }

    /* read original register value*/
    if (woodlawn_phy_reg_rd(bus_id, phy_addr, reg_addr, &reg_val) == FAILED) {
        printf("Dump port %d fails\n", port_num);
        return (FAILED);
    } else {
        printf("value: reg %#.4x, data %#.4x\n", reg_addr, reg_val);
    }
    
    /* Alter register with new value */
    reg_val = gethex_answer("Enter the new data (hex): ", reg_val, 0, 0xFFFF);

    /* Alter the current register with new vlaue */
    if (woodlawn_phy_reg_wr(bus_id, phy_addr, reg_addr, reg_val) == FAILED) {
        printf("Alter port %d fails\n", port_num);
        return (FAILED);
    }

    /* Read the new register value */
    if (woodlawn_phy_reg_rd(bus_id, phy_addr, reg_addr, &reg_val) == FAILED) {
        printf("Dump port %d fails\n", port_num);
        return (FAILED);
    } else {
        printf("value: reg %#.4x, data %#.4x\n", reg_addr, reg_val);
    }
    
    return (PASSED);
}

int dump_phy_88E1548_ptp_reg (void)
{
    int phy_addr;
    int bus_id, data_lo, data_hi;
    int sum;
    int reg_addr;

    phy_addr = gethex_answer("Enter phy address (0x0 ~ 0x7): ", 0, 0, 0xff);
    bus_id = get_smi_bus_id(phy_addr);

    woodlawn_phy_reg_wr(bus_id, phy_addr, 22, 0x0010);

    reg_addr = gethex_answer("Enter register address (0x0 ~ 0xFFFF): ", 0, 0, 0xffff);

    woodlawn_phy_reg_wr(bus_id, phy_addr, 0, reg_addr);
    woodlawn_phy_reg_rd(bus_id, phy_addr, 2, &data_lo);
    woodlawn_phy_reg_rd(bus_id, phy_addr, 3, &data_hi);
    sum = (data_hi << 16) | data_lo;

    printf("Reg - %#.4x => data_lo - %#.8x\n", reg_addr, data_lo);
    fflush(stdout);
    printf("Reg - %#.4x => data_hi - %#.8x\n", reg_addr, data_hi);
    fflush(stdout);
    printf("Reg - %#.4x = %#.8x\n", reg_addr, sum);
    fflush(stdout);

    return (PASSED);
}

int alter_phy_88E1548_ptp_reg (void)
{
    int phy_addr;
    int bus_id;
    int reg_addr, data_lo, data_hi;

    phy_addr = gethex_answer("Enter phy address (0x0 ~ 0x7): ", 0, 0, 0xff);
    bus_id = get_smi_bus_id(phy_addr);

    woodlawn_phy_reg_wr(bus_id, phy_addr, 22, 0x0010);
    reg_addr = gethex_answer("Enter register address (0x0 ~ 0xFFFF): ", 0, 0, 0xffff);
    data_hi = gethex_answer("Enter data hi val (0x0 ~ 0xFFFF): ", 0, 0, 0xffff);
    data_lo = gethex_answer("Enter data lo val (0x0 ~ 0xFFFF): ", 0, 0, 0xffff);

    woodlawn_phy_reg_wr(bus_id, phy_addr, 1, reg_addr);
    woodlawn_phy_reg_wr(bus_id, phy_addr, 2, data_lo);
    woodlawn_phy_reg_wr(bus_id, phy_addr, 3, data_hi);

    return (PASSED);
}

/***********************************************************************
 *  
 * Function: verify_1548_drift_adjustment_mode
 *    
 * Description: Timing card clock and trigger verification 
 *      
 * Inputs: None
 *        
 * Outputs: PASSED/FAILED
 *          
 *************************************************************************/
int verify_1548_drift_adjustment_mode (void)
{
    int ix;
    int phy_addr = 0x0;
    int bus_id, data_lo, data_hi;
    int trigger_count = 0, trigger_count_cmp = 0;
    char fpga_val, sync_out_clk, sync_out_valid;
    int idt_clk;

    bus_id = get_smi_bus_id(phy_addr);

    /* RW U1 P0-3 R22 H0010 */
    woodlawn_phy_reg_wr(bus_id, phy_addr, 22, 0x0010);

    for (ix = 0; ix < WAIT_PHY_READY; ix++) {
        if (fpga_reg_read(FPGA_RESET_SIGNAL_REG, &fpga_val) == FAILED) {
            printf("Read FPGA register %#.8x failed\n", FPGA_RESET_SIGNAL_REG);
            return (FAILED);
        }

        /* If bit 0 is "1" - Finish reset and PHY is ready */
        if ((fpga_val & GE_PHY_READY_MASK) == GE_PHY_READY) {
            printf("GE PHY is ready\n");
            break;
        }
        msleep(100);
    }

    /* Pulse-in counter use bit 7:0 and max value is 0xff.
     * Need to clear or re-init pulse-in counter when full(0xff).
     */
    woodlawn_phy_reg_wr(bus_id, phy_addr, 0, TRIG_IN_CNT_REG);
    woodlawn_phy_reg_rd(bus_id, phy_addr, 2, &data_lo);
    woodlawn_phy_reg_rd(bus_id, phy_addr, 3, &data_hi);

    if (data_lo == PTP_PULSE_IN_CNT_FULL) {
        data_lo &= (~PTP_INIT_PULSE_IN_CNT);
        msleep(100);
        woodlawn_phy_reg_wr(bus_id, phy_addr, 1, TRIG_IN_CNT_REG);
        woodlawn_phy_reg_wr(bus_id, phy_addr, 2, data_lo);
        woodlawn_phy_reg_wr(bus_id, phy_addr, 3, data_hi);
    }

    msleep(100);
    /* Register 0x2322 will keep count of the number of incoming pulse-triggers 
        when it sees a low to high transition */
    woodlawn_phy_reg_wr(bus_id, phy_addr, 0, TRIG_IN_CNT_REG);
    woodlawn_phy_reg_rd(bus_id, phy_addr, 2, &data_lo);
    woodlawn_phy_reg_rd(bus_id, phy_addr, 3, &data_hi);
    trigger_count = (data_hi << 16) | data_lo;

    for (ix = 0; ix < PTP_VERIFY_NUM; ix++) {
        /* Delay to make sure pulse in counter increase */
        msleep(1000);

        /* Read again pulse in counter */
        woodlawn_phy_reg_wr(bus_id, phy_addr, 0, TRIG_IN_CNT_REG);
        woodlawn_phy_reg_rd(bus_id, phy_addr, 2, &data_lo);
        woodlawn_phy_reg_rd(bus_id, phy_addr, 3, &data_hi);
        trigger_count_cmp = (data_hi << 16) | data_lo;

        if (trigger_count != trigger_count_cmp) {
            trigger_count = trigger_count_cmp;
        } else {
            printf("trigger_count = %#.8x, trigger_count_cmp = %#.8x\n", trigger_count, trigger_count_cmp);
            printf("1pps trigger is not fed into PHY successfully\n");
            return (FAILED);
        }
    }

    printf("1pps trigger is fed into successfully\n");

    /* Read FPGA reg 0x0e to verify that the sync_out clock is valid and the frequency is right */
    if (fpga_reg_read(DEV_STATUS_REG, &fpga_val) == FAILED) {
        printf("Read FPGA register %#.8x failed\n", DEV_STATUS_REG);
        return (FAILED);
    } 

    sync_out_valid = (fpga_val & FPGA_SYNC_OUT_VALID_MASK) >> FPGA_SYNC_OUT_VALID_SHIFT;
    if (sync_out_valid == SYNC_OUT_CLK_VALID) {
        printf("Valid sync_out_clock\n");
    } else {
        printf("Invalid sync_out_clock\n");
        return (FAILED);
    }

    sync_out_clk = fpga_val >> FPGA_SYNC_OUT_SHIFT;
    if (sync_out_clk == SYNC_OUT_CLK_25M) {
        printf("25MHz sync_out_clock is fed into FPGA successfully\n");
        idt_clk = CLK_CTRL_25M;
    } else {
        printf("8KHz sync_out_clock is fed into FPGA successfully\n");
        idt_clk = CLK_CTRL_8K;
    } 

    /* Lock 813N252 clock */
    if (fpga_reg_write(IDT_CTRL_REG, idt_clk) == FAILED) {
        printf("Write data %#.8x to register %#.8x failed\n", idt_clk, IDT_CTRL_REG);
        return (FAILED);
    }

    for (ix = 0; ix < WAIT_PHY_READY; ix++) {
        if (fpga_reg_read(IDT_CTRL_REG, &fpga_val) == FAILED) {
            printf("Read FPGA register %#.8x failed\n", IDT_CTRL_REG);
            return (FAILED);
        }

        /* If bit 0 is "0" - Finish reset and PHY is ready
         * If bit 2:1 is "10" - IDT use SYNC_OUT 25MHz clock from BP
         * If bit 2:1 is "00" - IDT use SYNC_OUT 8KHz clock from BP
         * If bit 2:1 is "11" - IDT use free run mode
         */
        if (((fpga_val & SYNC_OUT_CLK_MASK) == IDT_CLK0_25M) && ((fpga_val & IDT_READY_MASK) == IDT_PHY_READY)) {
            printf("813N252 use sync_out 25MHz\n");
            break;
        } else if (((fpga_val & SYNC_OUT_CLK_MASK) == IDT_CLK0_8K) && ((fpga_val & IDT_READY_MASK) == IDT_PHY_READY)) {
            printf("813N252 use sync_out 8KHz\n");
            break;
        } else {
            if (ix == (WAIT_PHY_READY -1)) {
                printf("813N252 use free run mode\n");
                return (FAILED);
            }
        }
        msleep(100);
    }

    return (PASSED);
}

/***********************************************************************
 *  
 * Function: verify_1548_clk_trig_in
 *    
 * Description: 88E1548 clock and trigger in verification 
 *      
 * Inputs: None
 *        
 * Outputs: PASSED/FAILED
 *          
 *************************************************************************/
int verify_1548_clk_trig_in (void)
{
    int ix;
    int phy_addr = 0x0;
    int bus_id, data_lo, data_hi;
    int sku_id;
    int trig_cnt, clk_cnt, trigger_count = 0, trigger_count_cmp = 0;
    int clock_count = 0, clock_count_cmp = 0;
    char fpga_val;
    int port_cnt, port_curr;
    int port_phy_addr[] = {0x0, 0x4};

    /* Get the SKU id */
    sku_id = get_sku_id();

    /* Not official SKU has different eth number copper port mapping compared
     * with new official SKU
     */
    if (sku_id == WOODLAWN_6GE) {
        port_cnt = 2;
    } else if (sku_id == WOODLAWN_4GE_1XAUI) {
        port_cnt = 1;
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
    for (port_curr = 0; port_curr < port_cnt; port_curr++) {
        phy_addr = port_phy_addr[port_curr];
        bus_id = get_smi_bus_id(phy_addr);
        trig_cnt = 0;
        /* RW U1 P0-3 R22 H0010 */
        woodlawn_phy_reg_wr(bus_id, phy_addr, 22, MRV88E1548L_REG_PAGE_16);

        /* Pulse-in counter use bit 7:0 and max value is 0xff.
         * Need to clear pulse-in counter.
         */
        data_lo &= (~PTP_INIT_PULSE_IN_CNT);
        data_hi &= (~PTP_INIT_PULSE_IN_CNT);
        msleep(100);
        woodlawn_phy_reg_wr(bus_id, phy_addr, 1, TRIG_IN_CNT_REG);
        woodlawn_phy_reg_wr(bus_id, phy_addr, 2, data_lo);
        woodlawn_phy_reg_wr(bus_id, phy_addr, 3, data_hi);

        msleep(PTP_CNT_DELAY);

        /* Register 0x2322 will keep count of the number of incoming pulse-triggers 
           when it sees a low to high transition */
        woodlawn_phy_reg_wr(bus_id, phy_addr, 0, TRIG_IN_CNT_REG);
        woodlawn_phy_reg_rd(bus_id, phy_addr, 2, &data_lo);
        woodlawn_phy_reg_rd(bus_id, phy_addr, 3, &data_hi);
        trigger_count = (data_hi << 16) | data_lo;

        printf("Verifying Trigger In on PHY-%d\n", port_curr);
        fflush(stdout);

        for (ix = 0; ix < TRIG_VERIFY_TIME; ix++) {
            /* Read again pulse in counter */
            woodlawn_phy_reg_wr(bus_id, phy_addr, 0, TRIG_IN_CNT_REG);
            woodlawn_phy_reg_rd(bus_id, phy_addr, 2, &data_lo);
            woodlawn_phy_reg_rd(bus_id, phy_addr, 3, &data_hi);
            trigger_count_cmp = (data_hi << 16) | data_lo;

            if (trigger_count < trigger_count_cmp) {
                trigger_count = trigger_count_cmp;
                trig_cnt++;
            }

            if (trig_cnt == TRIG_VERIFY_NUM) {
                break;
            }

            msleep(PTP_READ_DELAY);
        }

        if (trig_cnt < TRIG_VERIFY_NUM) {
            printf("1pps trigger is not fed into PHY-%d successfully\n", port_curr);
            fflush(stdout);
            return (FAILED);
        }
    }

    /* Check clock_in counter */
    for (port_curr = 0; port_curr < port_cnt; port_curr++) {
        phy_addr = port_phy_addr[port_curr];
        bus_id = get_smi_bus_id(phy_addr);
        clk_cnt = 0;

        /* RW U1 P0-3 R22 H0010 */
        woodlawn_phy_reg_wr(bus_id, phy_addr, 22, MRV88E1548L_REG_PAGE_16);

        /* Clock-in counter use bit 31:0 to count the incoming clock.
         * Each time an incoming clock(rising edge) is received, this counter is incremented. 
         * Need to clear clock-in counter.
         */
        data_lo &= (~PTP_INIT_CLOCK_IN_CNT);
        data_hi &= (~PTP_INIT_CLOCK_IN_CNT);
        msleep(100);
        woodlawn_phy_reg_wr(bus_id, phy_addr, 1, CLOCK_IN_CNT_REG);
        woodlawn_phy_reg_wr(bus_id, phy_addr, 2, data_lo);
        woodlawn_phy_reg_wr(bus_id, phy_addr, 3, data_hi);

        msleep(PTP_CNT_DELAY);

        /* Register 0x232e will keep count of the number of incoming clock 
           when it sees a low to high transition */
        woodlawn_phy_reg_wr(bus_id, phy_addr, 0, CLOCK_IN_CNT_REG);
        woodlawn_phy_reg_rd(bus_id, phy_addr, 2, &data_lo);
        woodlawn_phy_reg_rd(bus_id, phy_addr, 3, &data_hi);
        clock_count = (data_hi << 16) | data_lo;

        printf("Verifying Clock In on PHY-%d\n", port_curr);
        fflush(stdout);

        for (ix = 0; ix < CLK_VERIFY_TIME; ix++) {
            /* Read again pulse in counter */
            woodlawn_phy_reg_wr(bus_id, phy_addr, 0, CLOCK_IN_CNT_REG);
            woodlawn_phy_reg_rd(bus_id, phy_addr, 2, &data_lo);
            woodlawn_phy_reg_rd(bus_id, phy_addr, 3, &data_hi);
            clock_count_cmp = (data_hi << 16) | data_lo;

            if (clock_count < clock_count_cmp) {
                clock_count = clock_count_cmp;
                clk_cnt++;
            }
            
            if (clk_cnt == CLK_VERIFY_NUM) {
                break;
            }
            
            msleep(PTP_READ_DELAY);
        }

        if (clk_cnt < CLK_VERIFY_NUM) {
            printf("Clock is not fed into PHY-%d successfully\n", port_curr);
            fflush(stdout);
            return (FAILED);
       	}
    }

    return (PASSED);
}

/***********************************************************************
 *  
 * Function: config_1548_gen_trig_out
 *    
 * Description: Configure 88E1548P to generate 1PPS trigger
 *      
 * Inputs: None
 *        
 * Outputs: PASSED/FAILED
 *          
 *************************************************************************/
int config_1548_gen_trig_out (void)
{
    int phy_addr = 0x0;
    int sku_id;
    int ge_cnt, ge_curr;
    int port_phy_addr[] = {0x3, 0x7};
    int bus_id;

    /* Get the SKU id */
    sku_id = get_sku_id();

    /* Not official SKU has different eth number copper port mapping compared
     * with new official SKU
     */
    if (sku_id == WOODLAWN_6GE) {
        ge_cnt = 2;
    } else if (sku_id == WOODLAWN_4GE_1XAUI) {
        ge_cnt = 1;
    }

    /* Generate clock out from 88E1548 */
    for (ge_curr = 0; ge_curr < ge_cnt; ge_curr++) {
        /* Get SMI bus id */
        phy_addr = port_phy_addr[ge_curr];
        bus_id = get_88e1548_bus_id(phy_addr);

        /* Setup port3 LED[3:0] function control register */
        if (woodlawn_phy_reg_wr(bus_id, phy_addr, MRV88E1548L_PAGE_ADDRESS_REG, 
                                MRV88E1548L_REG_PAGE_3) == FAILED) {
            printf("Set Register Page %d Fail !\n", MRV88E1548L_REG_PAGE_3);
            fflush(stdout);
            return (FAILED);
        }

        /* Set LED[3] PTP generated pulse/clock
         * Set LED[2] on - 10Mbps link
         * Set LED[1] on - 100Mbps link
         * Set LED[0] on - 1000Mbps link
         */
        if (woodlawn_phy_reg_wr(bus_id, phy_addr, MRV88E1548L_LED_CTRL_REG, 
                                0x5777) == FAILED) {
            printf("Alter PHY Address 0x%x fails\n", phy_addr);
            fflush(stdout);
            return (FAILED);
        }

        /* Setup LED[3:0] Polarity Control Register 
         * On - drive LED[3] high, Off - drive LED[3] low
         */
        if (woodlawn_phy_reg_wr(bus_id, phy_addr, MRV88E1548L_LED_POLARITY_CTRL_REG, 
                                0x0040) == FAILED) {
            printf("Alter PHY Address 0x%x fails\n", phy_addr);
            fflush(stdout);
            return (FAILED);
        }

        /* Enable MACsec PTP (27_18.13) 
         * Select PTP TAI PulseGen/ClockGen (27_18.3)
         */
        if (woodlawn_phy_reg_wr(bus_id, phy_addr, MRV88E1548L_PAGE_ADDRESS_REG, 
                                MRV88E1548L_REG_PAGE_18) == FAILED) {
            printf("Set Register Page %d Fail !\n", MRV88E1548L_REG_PAGE_18);
            fflush(stdout);
            return (FAILED);
        }

        if (woodlawn_phy_reg_wr(bus_id, phy_addr, MRV88E1548L_GEN_CTRL_REG2, 
                                0x200A) == FAILED) {
            printf("Alter PHY Address 0x%x fails\n", phy_addr);
            fflush(stdout);
            return (FAILED);
        }

        /* Setup PTP registers */
        if (woodlawn_phy_reg_wr(bus_id, phy_addr, MRV88E1548L_PAGE_ADDRESS_REG, 
                                MRV88E1548L_REG_PAGE_16) == FAILED) {
            printf("Set Register Page %d Fail !\n", MRV88E1548L_REG_PAGE_16);
            fflush(stdout);
            return (FAILED);
        }

        /* Enable TriGen and set pulse width (Global TAI register)
         * Set the timer operation to capture mode to prevent 88E1548
         * receive unexpected external trigger and do the unexpected action.
         * through tod_func_cfg address(0x2323)
         */
        woodlawn_phy_reg_wr(bus_id, phy_addr, MRV88E1548L_PTP_WRITE_ADDR, 
                            MRV88E1548L_TOD_FUNC_CFG);
        woodlawn_phy_reg_wr(bus_id, phy_addr, MRV88E1548L_PTP_DATA_LO, 0x0000);
        woodlawn_phy_reg_wr(bus_id, phy_addr, MRV88E1548L_PTP_DATA_HI, 0xe100);

        /* Unmask TriGen TOD Mask (Global TAI register) */
        woodlawn_phy_reg_wr(bus_id, phy_addr, MRV88E1548L_PTP_WRITE_ADDR, 
                            MRV88E1548L_TRIG_GEN_MASK2);
        woodlawn_phy_reg_wr(bus_id, phy_addr, MRV88E1548L_PTP_DATA_LO, 0x0);
        woodlawn_phy_reg_wr(bus_id, phy_addr, MRV88E1548L_PTP_DATA_HI, 0x0);

        /* Program TrigGen TOD (whenever TOD = TrigGen TOD, a pulse will be sent out)
         * (Global TAI register)
         */
        woodlawn_phy_reg_wr(bus_id, phy_addr, MRV88E1548L_PTP_WRITE_ADDR, 
                            MRV88E1548L_TRIG_GEN_TOD0);
        woodlawn_phy_reg_wr(bus_id, phy_addr, MRV88E1548L_PTP_DATA_LO, 0x0);
        woodlawn_phy_reg_wr(bus_id, phy_addr, MRV88E1548L_PTP_DATA_HI, 0x0);

        woodlawn_phy_reg_wr(bus_id, phy_addr, MRV88E1548L_PTP_WRITE_ADDR, 
                            MRV88E1548L_TRIG_GEN_TOD1);
        woodlawn_phy_reg_wr(bus_id, phy_addr, MRV88E1548L_PTP_DATA_LO, 0x0);
        woodlawn_phy_reg_wr(bus_id, phy_addr, MRV88E1548L_PTP_DATA_HI, 0x0);

        woodlawn_phy_reg_wr(bus_id, phy_addr, MRV88E1548L_PTP_WRITE_ADDR, 
                            MRV88E1548L_TRIG_GEN_TOD2);
        woodlawn_phy_reg_wr(bus_id, phy_addr, MRV88E1548L_PTP_DATA_LO, 0x1);
        woodlawn_phy_reg_wr(bus_id, phy_addr, MRV88E1548L_PTP_DATA_HI, 0x0);

        woodlawn_phy_reg_wr(bus_id, phy_addr, MRV88E1548L_PTP_WRITE_ADDR, 
                            MRV88E1548L_TRIG_GEN_TOD3);
        woodlawn_phy_reg_wr(bus_id, phy_addr, MRV88E1548L_PTP_DATA_LO, 0x0);
        woodlawn_phy_reg_wr(bus_id, phy_addr, MRV88E1548L_PTP_DATA_HI, 0x0);
    }

    msleep(PTP_CONFIG_DELAY);

    return (PASSED);
}

/***********************************************************************
 *  
 * Function: config_1548_gen_clk_out
 *    
 * Description: Configure 88E1548P to generate 8kHz clock
 *      
 * Inputs: None
 *        
 * Outputs: PASSED/FAILED
 *          
 *************************************************************************/
int config_1548_gen_clk_out (void)
{
    int phy_addr = 0x0;
    int sku_id;
    int ge_cnt, ge_curr;
    int port_phy_addr[] = {0x2, 0x6};
    int bus_id, page_no;

    /* Get the SKU id */
    sku_id = get_sku_id();

    /* Not official SKU has different eth number copper port mapping compared
     * with new official SKU
     */
    if (sku_id == WOODLAWN_6GE) {
        ge_cnt = 2;
    } else if (sku_id == WOODLAWN_4GE_1XAUI) {
        ge_cnt = 1;
    }

    /* Generate clock out from 88E1548 */
    for (ge_curr = 0; ge_curr < ge_cnt; ge_curr++) {
        /* Get SMI bus id */
        phy_addr = port_phy_addr[ge_curr];
        bus_id = get_88e1548_bus_id(phy_addr);

        /* Setup port2 LED[3:0] function control register */
        if (woodlawn_phy_reg_wr(bus_id, phy_addr, MRV88E1548L_PAGE_ADDRESS_REG, 
                                MRV88E1548L_REG_PAGE_3) == FAILED) {
            printf("Set Register Page %d Fail !\n", page_no);
            fflush(stdout);
            return (FAILED);
        }

        /* Set LED[3] PTP generated pulse/clock
         * Set LED[2] on - 10Mbps link
         * Set LED[1] on - 100Mbps link
         * Set LED[0] on - 1000Mbps link
         */
        if (woodlawn_phy_reg_wr(bus_id, phy_addr, MRV88E1548L_LED_CTRL_REG, 
                                0x5777) == FAILED) {
            printf("Alter PHY Address 0x%x fails\n", phy_addr);
            fflush(stdout);
            return (FAILED);
        }

        /* Setup General Control Register 2 */
        if (woodlawn_phy_reg_wr(bus_id, phy_addr, MRV88E1548L_PAGE_ADDRESS_REG, 
                                MRV88E1548L_REG_PAGE_18) == FAILED) {
            printf("Set Register Page %d Fail !\n", page_no);
            fflush(stdout);
            return (FAILED);
        }

        /* Enable MACsec PTP (27_18.13) 
         * Send PTP TAI PulseGen/ClockGen (27_18.3)
         */
        if (woodlawn_phy_reg_wr(bus_id, phy_addr, MRV88E1548L_GEN_CTRL_REG2, 
                                0x200A) == FAILED) {
            printf("Alter PHY Address 0x%x fails\n", phy_addr);
            fflush(stdout);
            return (FAILED);
        }

        /* Setup PTP registers */
        if (woodlawn_phy_reg_wr(bus_id, phy_addr, MRV88E1548L_PAGE_ADDRESS_REG, 
                                MRV88E1548L_REG_PAGE_16) == FAILED) {
            printf("Set Register Page %d Fail !\n", page_no);
            fflush(stdout);
            return (FAILED);
        }

        /* Specified the nanosecond part of the clock cycle for clock generation
         * through clock_cyc address(0x232C)
         */
        woodlawn_phy_reg_wr(bus_id, phy_addr, MRV88E1548L_PTP_WRITE_ADDR, 
                            MRV88E1548L_CLOCK_CYC);
        woodlawn_phy_reg_wr(bus_id, phy_addr, MRV88E1548L_PTP_DATA_LO, 0x9000);
        woodlawn_phy_reg_wr(bus_id, phy_addr, MRV88E1548L_PTP_DATA_HI, 0x7);

        /* Controls which config pin to take TOD alignment clock from config[2].
         * Enable internal clock generator generates a slow multi-device TOD alignment
         * clock signal with a clock cycle defined by <Clock Cycle>.
         * Generate an external multi-device TOD alignment clock signal.
         * through tod_cfg_gen address(0x230B)
         */
        woodlawn_phy_reg_wr(bus_id, phy_addr, MRV88E1548L_PTP_WRITE_ADDR, 
		            MRV88E1548L_TOD_CFG_GEN);
        woodlawn_phy_reg_wr(bus_id, phy_addr, MRV88E1548L_PTP_DATA_LO, 0x205);
        woodlawn_phy_reg_wr(bus_id, phy_addr, MRV88E1548L_PTP_DATA_HI, 0x0);
    }

    msleep(PTP_CONFIG_DELAY);

    return (PASSED);
}

/***********************************************************************
 *  
 * Function: enable_88e1548_ptp_engine
 *    
 * Description: Enable PHY 1548 PTP engine
 *      
 * Inputs: None
 *        
 * Outputs: PASSED/FAILED
 *          
 *************************************************************************/
int enable_88e1548_ptp_engine (int eth_port)
{
    int start_phy_addr[] = {0x0, 0x4};
    int four_ge_eth_port_map[] = {0x0, 0x0, 0x0, 0x0, 0x3, 0x2, 0x1, 0x0};
    int six_ge_eth_port_map[] = {0x1, 0x0, 0x0, 0x0, 0x3, 0x2, 0x1, 0x0};
    int phy_addr, bus_id;
    int sku_id;
    int phy_num, port_num;

    sku_id = get_sku_id();

    if (sku_id == WOODLAWN_4GE_1XAUI) {
        phy_num = 1;
        port_num = four_ge_eth_port_map[eth_port];
        phy_addr = start_phy_addr[0];
    } else {
        port_num = six_ge_eth_port_map[eth_port];
        if (eth_port < 0x4) {
            phy_addr = start_phy_addr[1];
        } else {
            phy_addr = start_phy_addr[0];
        }
    }
    
    bus_id = get_smi_bus_id(phy_addr);

    /* Switch page to 16 to access PTP/MACsec Registers */
    /* RW U1 p0 R22 h0010 */
    woodlawn_phy_reg_wr(bus_id, phy_addr, 22, 0x0010);

    /* Init sequence */

    /* Release Note Errata 6.1 */
    /* RW U1 p0 R1 h2111 */
    woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x2111 + (0x800 * port_num));
    /* RW U1 p0 R2 hF00F */
    woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xf00f);
    /* RW U1 p0 R3 h0000 */
    woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0000);
    /* RW U1 p0 R1 h2111 */
    woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x2111 + (0x800 * port_num));
    /* RW U1 p0 R2 h000F */
    woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x000f);
    /* RW U1 p0 R3 h0000 */
    woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0000);

    /* For 2-step PTP */
    woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x210a + (0x800 * port_num));
    woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x9031);
    woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0000);
    woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x210d + (0x800 * port_num));
    woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x007f);
    woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0000);
    woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x2330 + (0x800 * port_num));
    woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x000f);
    woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0000);

    /* Release Note Errata 6.1 */
    /* RW U1 p0 R1 h000A */
    woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x000a + (0x800 * port_num));
    /* RW U1 p0 R2 h003A */
     woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x003a);
    /* RW U1 p0 R3 h0000 */
    woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0000);

    /* Release Note Errata 7.9 */
    /* Disable Wire MAC PortEn (Table 311) */
    /* RW U1 p0 R1 h0040 */
    woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x0040 + (0x800 * port_num));
    /* RW U1 p0 R2 h1848 */
    woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x1848);
    /* RW U1 p0 R3 h0001 */
    woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0001);
    /* Disable System MAC PortEn (Table 320) */
    /* RW U1 p0 R1 h0050 */
    woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x0050 + (0x800 * port_num));
    /* RW U1 p0 R2 h17C8 */
    woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x17c8);
    /* RW U1 p0 R3 h0000 */
    woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0000);

    /* Disable PTP Core */
    /* Disable PTP Core on Ingress (Table 432) */
    /* RW U1 p0 R1 h3480 */
    woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3480 + (0x800 * port_num));
    /* RW U1 p0 R2 h0008 */
    woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x0008);
    /* RW U1 p0 R3 h0000 */
    woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0000);
    /* Disable PTP Core on Egress (Table 432) */
    /* RW U1 p0 R1 h3080 */
    woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3080 + (0x800 * port_num));
    /* RW U1 p0 R2 h0008 */
    woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x0008);
    /* RW U1 p0 R3 h0000 */
    woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0000);

    /* Init for PTP Update PAM Range of Ingress/Egress (Table 429 & 430) */
    /* RW U1 p0 R1 h3059 */
    woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3059 + (0x800 * port_num));
    /* RW U1 p0 R2 hFFFF */
    woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xffff);
    /* RW U1 p0 R3 h0000 */
    woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0000);
    /* RW U1 p0 R1 h3459 */
    woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3459 + (0x800 * port_num));
    /* RW U1 p0 R2 hFFFF */
    woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xffff);
    /* RW U1 p0 R3 h0000 */
    woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0000);
    /* RW U1 p0 R1 h305E */
    woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x305e + (0x800 * port_num));
    /* RW U1 p0 R2 hCA00 */
    woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xca00);
    /* RW U1 p0 R3 h0000 */
    woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0000);
    /* RW U1 p0 R1 h345E */
    woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x345e + (0x800 * port_num)); 
    /* RW U1 p0 R2 hCA00 */
    woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xca00);
    /* RW U1 p0 R3 h0000 */
    woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0000);
    /* RW U1 p0 R1 h305F */
    woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x305f + (0x800 * port_num));
    /* RW U1 p0 R2 h3B9A */
    woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x3b9a);
    /* RW U1 p0 R3 h0000 */
    woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0000);
    /* RW U1 p0 R1 h345F */
    woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x345f + (0x800 * port_num));
    /* RW U1 p0 R2 h3B9A */
    woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x3b9a);
    /* RW U1 p0 R3 h0000 */
    woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0000);
    /* RW U1 p0 R1 h305A */
    woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x305a + (0x800 * port_num));
    /* RW U1 p0 R2 h0001 */
    woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x0001);
    /* RW U1 p0 R3 h0000 */
    woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0000);
    /* RW U1 p0 R1 h305D */
    woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x305d + (0x800 * port_num));
    /* RW U1 p0 R2 h0000 */
    woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x0000);
    /* RW U1 p0 R3 h0000 */
    woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0000);
    /* RW U1 p0 R1 h345C */
    woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x345c + (0x800 * port_num));
    /* RW U1 p0 R2 h88B5 */
    woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x88b5);
    /* RW U1 p0 R3 h0000 */
    woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0000);
    /* RW U1 p0 R1 h345D */
    woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x345d + (0x800 * port_num));
    /* RW U1 p0 R2 h88B6 */
    woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x88b6);
    /* RW U1 p0 R3 h0000 */
    woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0000);

    /* Init for PTP Parser PAM Range of Ingress/Egress (Table 427 & 428) */
    /* RW U1 p0 R1 h3019 */
    woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3019 + (0x800 * port_num));
    /* RW U1 p0 R2 hCA00 */
    woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xca00);
    /* RW U1 p0 R3 h0000 */
    woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0000);
    /* RW U1 p0 R1 h301A */
    woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x301a + (0x800 * port_num));
    /* RW U1 p0 R2 h3B9A */
    woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x3b9a);
    /* RW U1 p0 R3 h0000 */
    woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0000);
    /* RW U1 p0 R1 h301B */
    woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x301b + (0x800 * port_num));
    /* RW U1 p0 R2 h8007 */
    woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x8007);
    /* RW U1 p0 R3 h0000 */
    woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0000);
    /* RW U1 p0 R1 h301C */
    woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x301c + (0x800 * port_num));
    /* RW U1 p0 R2 h88B5 */
    woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x88b5);
    /* RW U1 p0 R3 h0000 */
    woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0000);
    /* RW U1 p0 R1 h301D */
    woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x301d + (0x800 * port_num));
    /* RW U1 p0 R2 h88B6 */
    woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x88b6);
   /* RW U1 p0 R3 h0000 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0000);
   /* RW U1 p0 R1 h3010 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3010 + (0x800 * port_num));
   /* RW U1 p0 R2 h0F00 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x0f00);
   /* RW U1 p0 R3 h0000 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0000);
   /* RW U1 p0 R1 h3410 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3410 + (0x800 * port_num));
   /* RW U1 p0 R2 h0F00 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x0f00);
   /* RW U1 p0 R3 h0000 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0000);
   /* RW U1 p0 R1 h3012 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3012 + (0x800 * port_num));
   /* RW U1 p0 R2 h05DC */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x05dc); 
   /* RW U1 p0 R3 h0000 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0000);
   /* RW U1 p0 R1 h3412 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3412 + (0x800 * port_num));
   /* RW U1 p0 R2 h05DC */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x05dc); 
   /* RW U1 p0 R3 h0000 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0000);
   /* RW U1 p0 R1 h3013 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3013 + (0x800 * port_num));
   /* RW U1 p0 R2 h0000 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x0000);
   /* RW U1 p0 R3 h0000 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0000);
   /* RW U1 p0 R1 h3413 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3413 + (0x800 * port_num));
   /* RW U1 p0 R2 h0000 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x0000);
   /* RW U1 p0 R3 h0000 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0000);
   /* RW U1 p0 R1 h3014 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3014 + (0x800 * port_num));
   /* RW U1 p0 R2 h3000 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x3000);
   /* RW U1 p0 R3 h0000 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0000);
   /* RW U1 p0 R1 h3414 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3414 + (0x800 * port_num));
   /* RW U1 p0 R2 h3000 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x3000);
   /* RW U1 p0 R3 h0000 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0000);
   /* RW U1 p0 R1 h3015 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3015 + (0x800 * port_num));
   /* RW U1 p0 R2 hf000 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xf000);
   /* RW U1 p0 R3 h0000 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0000);
   /* RW U1 p0 R1 h3415 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3415 + (0x800 * port_num));
   /* RW U1 p0 R2 hf000 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xf000);
   /* RW U1 p0 R3 h0000 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0000);
   /* RW U1 p0 R1 h3016 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3016 + (0x800 * port_num));
   /* RW U1 p0 R2 haaaa */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xaaaa);
   /* RW U1 p0 R3 h0000 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0000);
   /* RW U1 p0 R1 h3416 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3416 + (0x800 * port_num));
   /* RW U1 p0 R2 haaaa */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xaaaa);
   /* RW U1 p0 R3 h0000 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0000);
   /* RW U1 p0 R1 h3017 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3017 + (0x800 * port_num));
   /* RW U1 p0 R2 h0300 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x0300);
   /* RW U1 p0 R3 h0000 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0000);
   /* RW U1 p0 R1 h3417 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3417 + (0x800 * port_num));
   /* RW U1 p0 R2 h0300 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x0300);
   /* RW U1 p0 R3 h0000 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0000);
   /* RW U1 p0 R1 h3018 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3018 + (0x800 * port_num));
   /* RW U1 p0 R2 hff00 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xff00);
   /* RW U1 p0 R3 h0000 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0000);
   /* RW U1 p0 R1 h3418 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3418 + (0x800 * port_num));
   /* RW U1 p0 R2 hff00 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xff00);
   /* RW U1 p0 R3 h0000 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0000);
   /* RW U1 p0 R1 h301e */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x301e + (0x800 * port_num));
   /* RW U1 p0 R2 h8902 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x8902);
   /* RW U1 p0 R3 h0000 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0000);
   /* RW U1 p0 R1 h341e */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x341e + (0x800 * port_num));
   /* RW U1 p0 R2 h8902 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x8902);
   /* RW U1 p0 R3 h0000 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0000);
   /* RW U1 p0 R1 h301f */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x301f + (0x800 * port_num));
   /* RW U1 p0 R2 h0140 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x0140);
   /* RW U1 p0 R3 h0000 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0000);
   /* RW U1 p0 R1 h341f */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x341f + (0x800 * port_num));
   /* RW U1 p0 R2 h0140 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x0140);
   /* RW U1 p0 R3 h0000 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0000);

   /* Init for PTP Core (Internal Factory Registers) */
   /* RW U1 p0 R1 h3200 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3200 + (0x800 * port_num));
   /* RW U1 p0 R2 h00fe */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x00fe);
   /* RW U1 p0 R3 h0002 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0002);
   /* RW U1 p0 R1 h3201 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3201 + (0x800 * port_num));
   /* RW U1 p0 R2 hd055 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xd055);
   /* RW U1 p0 R3 h0000 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0000);
   /* RW U1 p0 R1 h3202 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3202 + (0x800 * port_num));
   /* RW U1 p0 R2 h2401 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x2401);
   /* RW U1 p0 R3 h0004 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0004);
   /* RW U1 p0 R1 h3203 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3203 + (0x800 * port_num));
   /* RW U1 p0 R2 h6801 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x6801);
   /* RW U1 p0 R3 h0004 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0004);
   /* RW U1 p0 R1 h3204 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3204 + (0x800 * port_num));
   /* RW U1 p0 R2 hd1c0 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xd1c0);
   /* RW U1 p0 R3 h0000 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0000);
   /* RW U1 p0 R1 h3205 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3205 + (0x800 * port_num));
   /* RW U1 p0 R2 h31cf */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x31cf);
   /* RW U1 p0 R3 h0003 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0003);
   /* RW U1 p0 R1 h3206 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3206 + (0x800 * port_num));
   /* RW U1 p0 R2 hd0c2 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xd0c2);
   /* RW U1 p0 R3 h0000 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0000);
   /* RW U1 p0 R1 h3207 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3207 + (0x800 * port_num));
   /* RW U1 p0 R2 hd14d */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xd14d);
   /* RW U1 p0 R3 h0000 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0000);
   /* RW U1 p0 R1 h3208 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3208 + (0x800 * port_num));
   /* RW U1 p0 R2 hdb41 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xdb41);
   /* RW U1 p0 R3 h0001 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0001);
   /* RW U1 p0 R1 h3209 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3209 + (0x800 * port_num));
   /* RW U1 p0 R2 hdd41 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xdd41);
   /* RW U1 p0 R3 h0001 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0001);
   /* RW U1 p0 R1 h320a */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x320a + (0x800 * port_num));
   /* RW U1 p0 R2 hd1fc */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xd1fc);
   /* RW U1 p0 R3 h0007 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0007);
   /* RW U1 p0 R1 h320b */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x320b + (0x800 * port_num));
   /* RW U1 p0 R2 hc081 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xc081);
   /* RW U1 p0 R3 h0000 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0000);
   /* RW U1 p0 R1 h320c */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x320c + (0x800 * port_num));
   /* RW U1 p0 R2 h308f */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x308f);
   /* RW U1 p0 R3 h0003 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0003);
   /* RW U1 p0 R1 h320d */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x320d + (0x800 * port_num));
   /* RW U1 p0 R2 hde8d */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xde8d);
   /* RW U1 p0 R3 h0001 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0001);
   /* RW U1 p0 R1 h320e */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x320e + (0x800 * port_num));
   /* RW U1 p0 R2 h101e */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x101e);
   /* RW U1 p0 R3 h0002 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0002);
   /* RW U1 p0 R1 h320f */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x320f + (0x800 * port_num));
   /* RW U1 p0 R2 h608a */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x608a);
   /* RW U1 p0 R3 h0004 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0004);
   /* RW U1 p0 R1 h3210 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3210 + (0x800 * port_num));
   /* RW U1 p0 R2 h6082 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x6082);
   /* RW U1 p0 R3 h0004 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0004);
   /* RW U1 p0 R1 h3211 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3211 + (0x800 * port_num));
   /* RW U1 p0 R2 h60a2 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x60a2);
   /* RW U1 p0 R3 h0004 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0004);
   /* RW U1 p0 R1 h3212 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3212 + (0x800 * port_num));
   /* RW U1 p0 R2 h609a */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x609a);
   /* RW U1 p0 R3 h0004 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0004);
   /* RW U1 p0 R1 h3213 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3213 + (0x800 * port_num));
   /* RW U1 p0 R2 h6092 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x6092);
   /* RW U1 p0 R3 h0004 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0004);
   /* RW U1 p0 R1 h3214 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3214 + (0x800 * port_num));
   /* RW U1 p0 R2 hc7d4 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xc7d4);
   /* RW U1 p0 R3 h0007 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0007);
   /* RW U1 p0 R1 h3215 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3215 + (0x800 * port_num));
   /* RW U1 p0 R2 hb7d7 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xb7d7);
   /* RW U1 p0 R3 h0007 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0007);
   /* RW U1 p0 R1 h3216 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3216 + (0x800 * port_num));
   /* RW U1 p0 R2 h6072 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x6072);
   /* RW U1 p0 R3 h0004 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0004);
   /* RW U1 p0 R1 h3217 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3217 + (0x800 * port_num));
   /* RW U1 p0 R2 hd101 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xd101);
   /* RW U1 p0 R3 h0000 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0000);
   /* RW U1 p0 R1 h3218 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3218 + (0x800 * port_num));
   /* RW U1 p0 R2 hdf08 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xdf08);
   /* RW U1 p0 R3 h0001 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0001);
   /* RW U1 p0 R1 h3219 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3219 + (0x800 * port_num));
   /* RW U1 p0 R2 hd184 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xd184);
   /* RW U1 p0 R3 h0000 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0000);
   /* RW U1 p0 R1 h321a */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x321a + (0x800 * port_num));
   /* RW U1 p0 R2 hdf88 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xdf88);
   /* RW U1 p0 R3 h0001 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0001);
   /* RW U1 p0 R1 h321b */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x321b + (0x800 * port_num));
   /* RW U1 p0 R2 hdd88 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xdd88);
   /* RW U1 p0 R3 h0001 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0001);
   /* RW U1 p0 R1 h321c */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x321c + (0x800 * port_num));
   /* RW U1 p0 R2 hc326 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xc326);
   /* RW U1 p0 R3 h0001 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0001);
   /* RW U1 p0 R1 h321d */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x321d + (0x800 * port_num));
   /* RW U1 p0 R2 h2102 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x2102);
   /* RW U1 p0 R3 h0004 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0004);
   /* RW U1 p0 R1 h321e */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x321e + (0x800 * port_num));
   /* RW U1 p0 R2 h4929 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x4929);
   /* RW U1 p0 R3 h0007 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0007);
   /* RW U1 p0 R1 h321f */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x321f + (0x800 * port_num));
   /* RW U1 p0 R2 h2729 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x2729);
   /* RW U1 p0 R3 h0007 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0007);
   /* RW U1 p0 R1 h3220 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3220 + (0x800 * port_num));
   /* RW U1 p0 R2 hc103 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xc103);
   /* RW U1 p0 R3 h0000 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0000);
   /* RW U1 p0 R1 h3221 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3221 + (0x800 * port_num));
   /* RW U1 p0 R2 h3108 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x3108);
   /* RW U1 p0 R3 h0003 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0003);
   /* RW U1 p0 R1 h3222 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3222 + (0x800 * port_num));
   /* RW U1 p0 R2 h1926 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x1926);
   /* RW U1 p0 R3 h0007 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0007);
   /* RW U1 p0 R1 h3223 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3223 + (0x800 * port_num));
   /* RW U1 p0 R2 hdf08 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xdf08);
   /* RW U1 p0 R3 h0001 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0001);
   /* RW U1 p0 R1 h3224 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3224 + (0x800 * port_num));
   /* RW U1 p0 R2 h8d08 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x8d08);
   /* RW U1 p0 R3 h0004 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0004);
   /* RW U1 p0 R1 h3225 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3225 + (0x800 * port_num));
   /* RW U1 p0 R2 h0029 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x0029);
   /* RW U1 p0 R3 h0002 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0002);
   /* RW U1 p0 R1 h3226 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3226 + (0x800 * port_num));
   /* RW U1 p0 R2 hfcd0 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xfcd0);
   /* RW U1 p0 R3 h0007 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0007);
   /* RW U1 p0 R1 h3227 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3227 + (0x800 * port_num));
   /* RW U1 p0 R2 hdf08 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xdf08);
   /* RW U1 p0 R3 h0001 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0001);
   /* RW U1 p0 R1 h3228 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3228 + (0x800 * port_num));
   /* RW U1 p0 R2 h8d08 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x8d08);
   /* RW U1 p0 R3 h0004 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0004);
   /* RW U1 p0 R1 h3229 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3229 + (0x800 * port_num));
   /* RW U1 p0 R2 hd08c */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xd08c);
   /* RW U1 p0 R3 h0000 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0000);
   /* RW U1 p0 R1 h322a */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x322a + (0x800 * port_num));
   /* RW U1 p0 R2 h5980 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x5980);
   /* RW U1 p0 R3 h0007 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0007);
   /* RW U1 p0 R1 h322b */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x322b + (0x800 * port_num));
   /* RW U1 p0 R2 hd112 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xd112);
   /* RW U1 p0 R3 h0000 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0000);
   /* RW U1 p0 R1 h322c */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x322c + (0x800 * port_num));
   /* RW U1 p0 R2 h042f */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x042f);
   /* RW U1 p0 R3 h0007 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0007);
   /* RW U1 p0 R1 h322d */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x322d + (0x800 * port_num));
   /* RW U1 p0 R2 h43d0 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x43d0);
   /* RW U1 p0 R3 h0005 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0005);
   /* RW U1 p0 R1 h322e */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x322e + (0x800 * port_num));
   /* RW U1 p0 R2 hc7f1 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xc7f1);
   /* RW U1 p0 R3 h0005 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0005);
   /* RW U1 p0 R1 h322f */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x322f + (0x800 * port_num));
   /* RW U1 p0 R2 hd111 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xd111);
   /* RW U1 p0 R3 h0000 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0000);
   /* RW U1 p0 R1 h3230 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3230 + (0x800 * port_num));
   /* RW U1 p0 R2 h5506 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x5506);
   /* RW U1 p0 R3 h0003 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0003);
   /* RW U1 p0 R1 h3231 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3231 + (0x800 * port_num));
   /* RW U1 p0 R2 h203a */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x203a);
   /* RW U1 p0 R3 h0002 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0002);
   /* RW U1 p0 R1 h3232 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3232 + (0x800 * port_num));
   /* RW U1 p0 R2 h3036 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x3036);
   /* RW U1 p0 R3 h0002 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0002);
   /* RW U1 p0 R1 h3233 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3233 + (0x800 * port_num));
   /* RW U1 p0 R2 hd110 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xd110);
   /* RW U1 p0 R3 h0000 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0000);
   /* RW U1 p0 R1 h3234 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3234 + (0x800 * port_num));
   /* RW U1 p0 R2 h5507 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x5507);
   /* RW U1 p0 R3 h0003 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0003);
   /* RW U1 p0 R1 h3235 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3235 + (0x800 * port_num));
   /* RW U1 p0 R2 h203a */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x203a);
   /* RW U1 p0 R3 h0002 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0002);
   /* RW U1 p0 R1 h3236 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3236 + (0x800 * port_num));
   /* RW U1 p0 R2 h43d0 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x43d0);
   /* RW U1 p0 R3 h0005 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0005);
   /* RW U1 p0 R1 h3237 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3237 + (0x800 * port_num));
   /* RW U1 p0 R2 hc7f1 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xc7f1);
   /* RW U1 p0 R3 h0005 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0005);
   /* RW U1 p0 R1 h3238 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3238 + (0x800 * port_num));
   /* RW U1 p0 R2 h43d0 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x43d0);
   /* RW U1 p0 R3 h0005 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0005);
   /* RW U1 p0 R1 h3239 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3239 + (0x800 * port_num));
   /* RW U1 p0 R2 hc7f1 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xc7f1);
   /* RW U1 p0 R3 h0005 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0005);
   /* RW U1 p0 R1 h323a */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x323a + (0x800 * port_num));
   /* RW U1 p0 R2 he090 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xe090);
   /* RW U1 p0 R3 h0000 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0000);
   /* RW U1 p0 R1 h323b */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x323b + (0x800 * port_num));
   /* RW U1 p0 R2 h4370 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x4370);
   /* RW U1 p0 R3 h0005 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0005);
   /* RW U1 p0 R1 h323c */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x323c + (0x800 * port_num));
   /* RW U1 p0 R2 hc791 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xc791);
   /* RW U1 p0 R3 h0005 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0005);
   /* RW U1 p0 R1 h323d */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x323d + (0x800 * port_num));
   /* RW U1 p0 R2 hcbb2 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xcbb2);
   /* RW U1 p0 R3 h0005 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0005);
   /* RW U1 p0 R1 h323e */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x323e + (0x800 * port_num));
   /* RW U1 p0 R2 h3148 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x3148);
   /* RW U1 p0 R3 h0007 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0007);
   /* RW U1 p0 R1 h323f */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x323f + (0x800 * port_num));
   /* RW U1 p0 R2 hd118 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xd118);
   /* RW U1 p0 R3 h0000 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0000);
   /* RW U1 p0 R1 h3240 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3240 + (0x800 * port_num));
   /* RW U1 p0 R2 hfc45 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xfc45);
   /* RW U1 p0 R3 h0007 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0007);
   /* RW U1 p0 R1 h3241 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3241 + (0x800 * port_num));
   /* RW U1 p0 R2 h42f0 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x42f0);
   /* RW U1 p0 R3 h0005 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0005);
   /* RW U1 p0 R1 h3242 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3242 + (0x800 * port_num));
   /* RW U1 p0 R2 hc711 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xc711);
   /* RW U1 p0 R3 h0005 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0005);
   /* RW U1 p0 R1 h3243 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3243 + (0x800 * port_num));
   /* RW U1 p0 R2 hcbb2 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xcbb2);
   /* RW U1 p0 R3 h0005 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0005);
   /* RW U1 p0 R1 h3244 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3244 + (0x800 * port_num));
   /* RW U1 p0 R2 h0048 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x0048);
   /* RW U1 p0 R3 h0002 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0002);
   /* RW U1 p0 R1 h3245 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3245 + (0x800 * port_num));
   /* RW U1 p0 R2 h42f0 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x42f0);
   /* RW U1 p0 R3 h0005 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0005);
   /* RW U1 p0 R1 h3246 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3246 + (0x800 * port_num));
   /* RW U1 p0 R2 hc711 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xc711);
   /* RW U1 p0 R3 h0005 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0005);
   /* RW U1 p0 R1 h3247 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3247 + (0x800 * port_num));
   /* RW U1 p0 R2 hcb32 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xcb32);
   /* RW U1 p0 R3 h0005 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0005);
   /* RW U1 p0 R1 h3248 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3248 + (0x800 * port_num));
   /* RW U1 p0 R2 h40f0 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x40f0);
   /* RW U1 p0 R3 h0006 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0006);
   /* RW U1 p0 R1 h3249 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3249 + (0x800 * port_num));
   /* RW U1 p0 R2 hc4d1 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xc4d1);
   /* RW U1 p0 R3 h0006 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0006);
   /* RW U1 p0 R1 h324a */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x324a + (0x800 * port_num));
   /* RW U1 p0 R2 hcbb2 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xcbb2);
   /* RW U1 p0 R3 h0006 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0006);
   /* RW U1 p0 R1 h324b */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x324b + (0x800 * port_num));
   /* RW U1 p0 R2 hd188 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xd188);
   /* RW U1 p0 R3 h0000 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0000);
   /* RW U1 p0 R1 h324c */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x324c + (0x800 * port_num));
   /* RW U1 p0 R2 h4150 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x4150);
   /* RW U1 p0 R3 h0005 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0005);
   /* RW U1 p0 R1 h324d */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x324d + (0x800 * port_num));
   /* RW U1 p0 R2 hc531 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xc531);
   /* RW U1 p0 R3 h0005 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0005);
   /* RW U1 p0 R1 h324e */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x324e + (0x800 * port_num));
   /* RW U1 p0 R2 hc912 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xc912);
   /* RW U1 p0 R3 h0005 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0005);
   /* RW U1 p0 R1 h324f */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x324f + (0x800 * port_num));
   /* RW U1 p0 R2 h50f1 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x50f1);
   /* RW U1 p0 R3 h0002 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0002);
   /* RW U1 p0 R1 h3250 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3250 + (0x800 * port_num));
   /* RW U1 p0 R2 h435f */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x435f);
   /* RW U1 p0 R3 h0007 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0007);
   /* RW U1 p0 R1 h3251 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3251 + (0x800 * port_num));
   /* RW U1 p0 R2 h2292 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x2292);
   /* RW U1 p0 R3 h0003 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0003);
   /* RW U1 p0 R1 h3252 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3252 + (0x800 * port_num));
   /* RW U1 p0 R2 h2291 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x2291);
   /* RW U1 p0 R3 h0003 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0003);
   /* RW U1 p0 R1 h3253 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3253 + (0x800 * port_num));
   /* RW U1 p0 R2 h2290 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x2290);
   /* RW U1 p0 R3 h0003 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0003);
   /* RW U1 p0 R1 h3254 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3254 + (0x800 * port_num));
   /* RW U1 p0 R2 h2688 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x2688);
   /* RW U1 p0 R3 h0003 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0003);
   /* RW U1 p0 R1 h3255 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3255 + (0x800 * port_num));
   /* RW U1 p0 R2 h2689 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x2689);
   /* RW U1 p0 R3 h0003 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0003);
   /* RW U1 p0 R1 h3256 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3256 + (0x800 * port_num));
   /* RW U1 p0 R2 h268a */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x268a);
   /* RW U1 p0 R3 h0003 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0003);
   /* RW U1 p0 R1 h3257 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3257 + (0x800 * port_num));
   /* RW U1 p0 R2 h435f */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x435f);
   /* RW U1 p0 R3 h0007 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0007);
   /* RW U1 p0 R1 h3258 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3258 + (0x800 * port_num));
   /* RW U1 p0 R2 h07fe */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x07fe);
   /* RW U1 p0 R3 h0007 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0007);
   /* RW U1 p0 R1 h3259 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3259 + (0x800 * port_num));
   /* RW U1 p0 R2 h617e */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x617e);
   /* RW U1 p0 R3 h0007 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0007);
   /* RW U1 p0 R1 h325a */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x325a + (0x800 * port_num));
   /* RW U1 p0 R2 h5499 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x5499);
   /* RW U1 p0 R3 h0003 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0003);
   /* RW U1 p0 R1 h325b */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x325b + (0x800 * port_num));
   /* RW U1 p0 R2 h405d */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x405d);
   /* RW U1 p0 R3 h0002 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0002);
   /* RW U1 p0 R1 h325c */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x325c + (0x800 * port_num));
   /* RW U1 p0 R2 hc682 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xc682);
   /* RW U1 p0 R3 h0001 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0001);
   /* RW U1 p0 R1 h325d */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x325d + (0x800 * port_num));
   /* RW U1 p0 R2 h8942 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x8942);
   /* RW U1 p0 R3 h0004 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0004);
   /* RW U1 p0 R1 h325e */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x325e + (0x800 * port_num));
   /* RW U1 p0 R2 h9082 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x9082);
   /* RW U1 p0 R3 h0004 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0004);
   /* RW U1 p0 R1 h325f */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x325f + (0x800 * port_num));
   /* RW U1 p0 R2 hfa02 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xfa02);
   /* RW U1 p0 R3 h0000 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0000);
   /* RW U1 p0 R1 h3260 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3260 + (0x800 * port_num));
   /* RW U1 p0 R2 h87b3 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x87b3);
   /* RW U1 p0 R3 h0007 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0007);
   /* RW U1 p0 R1 h3261 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3261 + (0x800 * port_num));
   /* RW U1 p0 R2 h00fe */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x00fe);
   /* RW U1 p0 R3 h0002 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0002);
   /* RW U1 p0 R1 h3262 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3262 + (0x800 * port_num));
   /* RW U1 p0 R2 h41fe */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x41fe);
   /* RW U1 p0 R3 h0007 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0007);
   /* RW U1 p0 R1 h3263 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3263 + (0x800 * port_num));
   /* RW U1 p0 R2 hfa02 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xfa02);
   /* RW U1 p0 R3 h0000 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0000);
   /* RW U1 p0 R1 h3264 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3264 + (0x800 * port_num));
   /* RW U1 p0 R2 hd14d */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xd14d);
   /* RW U1 p0 R3 h0000 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0000);
   /* RW U1 p0 R1 h3265 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3265 + (0x800 * port_num));
   /* RW U1 p0 R2 hdd41 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xdd41);
   /* RW U1 p0 R3 h0001 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0001);
   /* RW U1 p0 R1 h3266 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3266 + (0x800 * port_num));
   /* RW U1 p0 R2 h4370 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x4370);
   /* RW U1 p0 R3 h0005 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0005);
   /* RW U1 p0 R1 h3267 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3267 + (0x800 * port_num));
   /* RW U1 p0 R2 hc791 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xc791);
   /* RW U1 p0 R3 h0005 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0005);
   /* RW U1 p0 R1 h3268 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3268 + (0x800 * port_num));
   /* RW U1 p0 R2 hcbb2 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xcbb2);
   /* RW U1 p0 R3 h0005 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0005);
   /* RW U1 p0 R1 h3269 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3269 + (0x800 * port_num));
   /* RW U1 p0 R2 hcf83 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xcf83);
   /* RW U1 p0 R3 h0005 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0005);
   /* RW U1 p0 R1 h326a */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x326a + (0x800 * port_num));
   /* RW U1 p0 R2 hd1c1 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xd1c1);
   /* RW U1 p0 R3 h0000 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0000);
   /* RW U1 p0 R1 h326b */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x326b + (0x800 * port_num));
   /* RW U1 p0 R2 hfe2e */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xfe2e);
   /* RW U1 p0 R3 h0000 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0000);
   /* RW U1 p0 R1 h326c */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x326c + (0x800 * port_num));
   /* RW U1 p0 R2 h51c6 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x51c6);
   /* RW U1 p0 R3 h0003 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0003);
   /* RW U1 p0 R1 h326d */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x326d + (0x800 * port_num));
   /* RW U1 p0 R2 h4075 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x4075);
   /* RW U1 p0 R3 h0002 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0002);
   /* RW U1 p0 R1 h326e */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x326e + (0x800 * port_num));
   /* RW U1 p0 R2 hfe2d */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xfe2d);
   /* RW U1 p0 R3 h0000 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0000);
   /* RW U1 p0 R1 h326f */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x326f + (0x800 * port_num));
   /* RW U1 p0 R2 h51c6 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x51c6);
   /* RW U1 p0 R3 h0003 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0003);
   /* RW U1 p0 R1 h3270 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3270 + (0x800 * port_num));
   /* RW U1 p0 R2 h4077 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x4077);
   /* RW U1 p0 R3 h0002 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0002);
   /* RW U1 p0 R1 h3271 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3271 + (0x800 * port_num));
   /* RW U1 p0 R2 hfe2f */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xfe2f);
   /* RW U1 p0 R3 h0000 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0000);
   /* RW U1 p0 R1 h3272 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3272 + (0x800 * port_num));
   /* RW U1 p0 R2 h51c6 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x51c6);
   /* RW U1 p0 R3 h0003 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0003);
   /* RW U1 p0 R1 h3273 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3273 + (0x800 * port_num));
   /* RW U1 p0 R2 h4077 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x4077);
   /* RW U1 p0 R3 h0002 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0002);
   /* RW U1 p0 R1 h3274 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3274 + (0x800 * port_num));
   /* RW U1 p0 R2 h00fe */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x00fe);
   /* RW U1 p0 R3 h0002 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0002);
   /* RW U1 p0 R1 h3275 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3275 + (0x800 * port_num));
   /* RW U1 p0 R2 hd954 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xd954);
   /* RW U1 p0 R3 h0001 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0001);
   /* RW U1 p0 R1 h3276 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3276 + (0x800 * port_num));
   /* RW U1 p0 R2 h0078 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x0078);
   /* RW U1 p0 R3 h0002 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0002);
   /* RW U1 p0 R1 h3277 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3277 + (0x800 * port_num));
   /* RW U1 p0 R2 hd944 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xd944);
   /* RW U1 p0 R3 h0001 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0001);
   /* RW U1 p0 R1 h3278 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3278 + (0x800 * port_num));
   /* RW U1 p0 R2 h8948 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x8948);
   /* RW U1 p0 R3 h0004 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0004);
   /* RW U1 p0 R1 h3279 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3279 + (0x800 * port_num));
   /* RW U1 p0 R2 h509a */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x509a);
   /* RW U1 p0 R3 h0004 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0004);
   /* RW U1 p0 R1 h327a */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x327a + (0x800 * port_num));
   /* RW U1 p0 R2 h5092 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x5092);
   /* RW U1 p0 R3 h0004 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0004);
   /* RW U1 p0 R1 h327b */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x327b + (0x800 * port_num));
   /* RW U1 p0 R2 h508a */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x508a);
   /* RW U1 p0 R3 h0004 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0004);
   /* RW U1 p0 R1 h327c */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x327c + (0x800 * port_num));
   /* RW U1 p0 R2 h5082 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x5082);
   /* RW U1 p0 R3 h0004 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0004);
   /* RW U1 p0 R1 h327d */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x327d + (0x800 * port_num));
   /* RW U1 p0 R2 h00fe */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x00fe);
   /* RW U1 p0 R3 h0002 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0002);
   /* RW U1 p0 R1 h327e */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x327e + (0x800 * port_num));
   /* RW U1 p0 R2 h2285 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x2285);
   /* RW U1 p0 R3 h0003 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0003);
   /* RW U1 p0 R1 h327f */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x327f + (0x800 * port_num));
   /* RW U1 p0 R2 h005a */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x005a);
   /* RW U1 p0 R3 h0002 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0002);
   /* RW U1 p0 R1 h3280 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3280 + (0x800 * port_num));
   /* RW U1 p0 R2 h3193 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x3193);
   /* RW U1 p0 R3 h0007 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0007);
   /* RW U1 p0 R1 h3281 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3281 + (0x800 * port_num));
   /* RW U1 p0 R2 h2688 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x2688);
   /* RW U1 p0 R3 h0003 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0003);
   /* RW U1 p0 R1 h3282 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3282 + (0x800 * port_num));
   /* RW U1 p0 R2 h2689 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x2689);
   /* RW U1 p0 R3 h0003 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0003);
   /* RW U1 p0 R1 h3283 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3283 + (0x800 * port_num));
   /* RW U1 p0 R2 h268a */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x268a);
   /* RW U1 p0 R3 h0003 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0003);
   /* RW U1 p0 R1 h3284 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3284 + (0x800 * port_num));
   /* RW U1 p0 R2 hd118 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xd118);
   /* RW U1 p0 R3 h0000 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0000);
   /* RW U1 p0 R1 h3285 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3285 + (0x800 * port_num));
   /* RW U1 p0 R2 hfc8c */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xfc8c);
   /* RW U1 p0 R3 h0007 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0007);
   /* RW U1 p0 R1 h3286 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3286 + (0x800 * port_num));
   /* RW U1 p0 R2 hd188 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xd188);
   /* RW U1 p0 R3 h0000 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0000);
   /* RW U1 p0 R1 h3287 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3287 + (0x800 * port_num));
   /* RW U1 p0 R2 h2aea */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x2aea);
   /* RW U1 p0 R3 h0005 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0005);
   /* RW U1 p0 R1 h3288 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3288 + (0x800 * port_num));
   /* RW U1 p0 R2 ha709 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xa709);
   /* RW U1 p0 R3 h0005 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0005);
   /* RW U1 p0 R1 h3289 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3289 + (0x800 * port_num));
   /* RW U1 p0 R2 ha3a8 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xa3a8);
   /* RW U1 p0 R3 h0005 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0005);
   /* RW U1 p0 R1 h238a */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x238a + (0x800 * port_num));
   /* RW U1 p0 R2 h50e6 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x50e6);
   /* RW U1 p0 R3 h0002 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0002);
   /* RW U1 p0 R1 h328b */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x328b + (0x800 * port_num));
   /* RW U1 p0 R2 h008f */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x008f);
   /* RW U1 p0 R3 h0002 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0002);
   /* RW U1 p0 R1 h328c */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x328c + (0x800 * port_num));
   /* RW U1 p0 R2 h2aea */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x2aea);
   /* RW U1 p0 R3 h0005 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0005);
   /* RW U1 p0 R1 h328d */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x328d + (0x800 * port_num));
   /* RW U1 p0 R2 ha709 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xa709);
   /* RW U1 p0 R3 h0005 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0005);
   /* RW U1 p0 R1 h328e */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x328e + (0x800 * port_num));
   /* RW U1 p0 R2 ha328 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xa328);
   /* RW U1 p0 R3 h0005 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0005);
   /* RW U1 p0 R1 h328f */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x328f + (0x800 * port_num));
   /* RW U1 p0 R2 h4393 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x4393);
   /* RW U1 p0 R3 h0007 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0007);
   /* RW U1 p0 R1 h3290 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3290 + (0x800 * port_num));
   /* RW U1 p0 R2 h228a */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x228a);
   /* RW U1 p0 R3 h0003 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0003);
   /* RW U1 p0 R1 h3291 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3291 + (0x800 * port_num));
   /* RW U1 p0 R2 h2289 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x2289);
   /* RW U1 p0 R3 h0003 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0003);
   /* RW U1 p0 R1 h3292 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3292 + (0x800 * port_num));
   /* RW U1 p0 R2 h2288 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x2288);
   /* RW U1 p0 R3 h0003 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0003);
   /* RW U1 p0 R1 h3293 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3293 + (0x800 * port_num));
   /* RW U1 p0 R2 h4157 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x4157);
   /* RW U1 p0 R3 h0007 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0007);
   /* RW U1 p0 R1 h3294 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3294 + (0x800 * port_num));
   /* RW U1 p0 R2 hb762 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xb762);
   /* RW U1 p0 R3 h0007 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0007);
   /* RW U1 p0 R1 h3295 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3295 + (0x800 * port_num));
   /* RW U1 p0 R2 h4370 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x4370);
   /* RW U1 p0 R3 h0005 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0005);
   /* RW U1 p0 R1 h3296 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3296 + (0x800 * port_num));
   /* RW U1 p0 R2 hc791 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xc791);
   /* RW U1 p0 R3 h0005 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0005);

   /* RW U1 p0 R1 h3297 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3297 + (0x800 * port_num));
   /* RW U1 p0 R2 hd191 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xd191);
   /* RW U1 p0 R3 h0000 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0000);
   /* RW U1 p0 R1 h3298 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3298 + (0x800 * port_num));
   /* RW U1 p0 R2 h559f */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x559f);
   /* RW U1 p0 R3 h0003 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0003);
   /* RW U1 p0 R1 h3299 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3299 + (0x800 * port_num));
   /* RW U1 p0 R2 h30ab */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x30ab);
   /* RW U1 p0 R3 h0002 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0002);
   /* RW U1 p0 R1 h329a */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x329a + (0x800 * port_num));
   /* RW U1 p0 R2 h40a2 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x40a2);
   /* RW U1 p0 R3 h0002 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0002);
   /* RW U1 p0 R1 h329b */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x329b + (0x800 * port_num));
   /* RW U1 p0 R2 h4b52 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x4b52);
   /* RW U1 p0 R3 h0005 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0005);
   /* RW U1 p0 R1 h329c */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x329c + (0x800 * port_num));
   /* RW U1 p0 R2 hcfb3 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xcfb3);
   /* RW U1 p0 R3 h0005 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0005);
   /* RW U1 p0 R1 h329d */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x329d + (0x800 * port_num));
   /* RW U1 p0 R2 hd3b4 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xd3b4);
   /* RW U1 p0 R3 h0005 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0005);
   /* RW U1 p0 R1 h329e */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x329e + (0x800 * port_num));
   /* RW U1 p0 R2 h43d0 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x43d0);
   /* RW U1 p0 R3 h0006 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0006);
   /* RW U1 p0 R1 h329f */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x329f + (0x800 * port_num));
   /* RW U1 p0 R2 hc7b1 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xc7b1);
   /* RW U1 p0 R3 h0006 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0006);
   /* RW U1 p0 R1 h32a0 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x32a0 + (0x800 * port_num));
   /* RW U1 p0 R2 h47f1 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x47f1);
   /* RW U1 p0 R3 h0006 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0006);
   /* RW U1 p0 R1 h32a1 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x32a1 + (0x800 * port_num));
   /* RW U1 p0 R2 h00ab */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x00ab);
   /* RW U1 p0 R3 h0002 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0002);
   /* RW U1 p0 R1 h32a2 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x32a2 + (0x800 * port_num));
   /* RW U1 p0 R2 hd190 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xd190);
   /* RW U1 p0 R3 h0000 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0000);
   /* RW U1 p0 R1 h32a3 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x32a3 + (0x800 * port_num));
   /* RW U1 p0 R2 h559e */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x559e);
   /* RW U1 p0 R3 h0003 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0003);
   /* RW U1 p0 R1 h32a4 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x32a4 + (0x800 * port_num));
   /* RW U1 p0 R2 h30ab */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x30ab);
   /* RW U1 p0 R3 h0002 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0002);
   /* RW U1 p0 R1 h32a5 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x32a5 + (0x800 * port_num));
   /* RW U1 p0 R2 h4b52 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x4b52);
   /* RW U1 p0 R3 h0005 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0005);
   /* RW U1 p0 R1 h32a6 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x32a6 + (0x800 * port_num));
   /* RW U1 p0 R2 hcfb3 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xcfb3);
   /* RW U1 p0 R3 h0005 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0005);
   /* RW U1 p0 R1 h32a7 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x32a7 + (0x800 * port_num));
   /* RW U1 p0 R2 hd3b4 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xd3b4);
   /* RW U1 p0 R3 h0005 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0005);
   /* RW U1 p0 R1 h32a8 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x32a8 + (0x800 * port_num));
   /* RW U1 p0 R2 h43d0 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xd3d0);
   /* RW U1 p0 R3 h0006 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0006);
   /* RW U1 p0 R1 h32a9 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x32a9 + (0x800 * port_num));
   /* RW U1 p0 R2 hc7b1 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xc7b1);
   /* RW U1 p0 R3 h0006 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0006);
   /* RW U1 p0 R1 h32aa */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x32aa + (0x800 * port_num));
   /* RW U1 p0 R2 h47f1 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x47f1);
   /* RW U1 p0 R3 h0006 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0006);
   /* RW U1 p0 R1 h32ab */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x32ab + (0x800 * port_num));
   /* RW U1 p0 R2 h435f */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x435f);
   /* RW U1 p0 R3 h0007 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0007);
   /* RW U1 p0 R1 h32ac */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x32ac + (0x800 * port_num));
   /* RW U1 p0 R2 h2290 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x2290);
   /* RW U1 p0 R3 h0003 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0003);
   /* RW U1 p0 R1 h32ad */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x32ad + (0x800 * port_num));
   /* RW U1 p0 R2 h2291 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x2291);
   /* RW U1 p0 R3 h0003 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0003);
   /* RW U1 p0 R1 h32ae */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x32ae + (0x800 * port_num));
   /* RW U1 p0 R2 h2292 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x2292);
   /* RW U1 p0 R3 h0003 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0003);
   /* RW U1 p0 R1 h32af */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x32af + (0x800 * port_num));
   /* RW U1 p0 R2 h2293 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x2293);
   /* RW U1 p0 R3 h0003 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0003);
    /* RW U1 p0 R1 h32b0 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x32b0 + (0x800 * port_num));
   /* RW U1 p0 R2 h2294 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x2294);
   /* RW U1 p0 R3 h0003 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0003);
   /* RW U1 p0 R1 h32b1 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x32b1 + (0x800 * port_num));
   /* RW U1 p0 R2 h268f */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x268f);
   /* RW U1 p0 R3 h0003 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0003);
   /* RW U1 p0 R1 h32b2 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x32b2 + (0x800 * port_num));
   /* RW U1 p0 R2 h0057 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x0057);
   /* RW U1 p0 R3 h0002 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0002);
   /* RW U1 p0 R1 h32b3 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x32b3 + (0x800 * port_num));
   /* RW U1 p0 R2 hd94a */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xd94a);
   /* RW U1 p0 R3 h0001 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0001);
   /* RW U1 p0 R1 h32b4 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x32b4 + (0x800 * port_num));
   /* RW U1 p0 R2 h59c9 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x59c9);
   /* RW U1 p0 R3 h0007 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0007);
   /* RW U1 p0 R1 h32b5 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x32b5 + (0x800 * port_num));
   /* RW U1 p0 R2 hffe0 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xffe0);
   /* RW U1 p0 R3 h0007 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0007);
   /* RW U1 p0 R1 h32b6 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x32b6 + (0x800 * port_num));
   /* RW U1 p0 R2 h8946 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x8946);
   /* RW U1 p0 R3 h0004 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0004);
   /* RW U1 p0 R1 h32b7 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x32b7 + (0x800 * port_num));
   /* RW U1 p0 R2 h5092 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x5092);
   /* RW U1 p0 R3 h0004 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0004);
   /* RW U1 p0 R1 h32b8 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x32b8 + (0x800 * port_num));
   /* RW U1 p0 R2 h508a */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x508a);
   /* RW U1 p0 R3 h0004 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0004);
   /* RW U1 p0 R1 h32b9 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x32b9 + (0x800 * port_num));
   /* RW U1 p0 R2 h5082 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x5082);
   /* RW U1 p0 R3 h0004 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0004);
   /* RW U1 p0 R1 h32ba */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x32ba + (0x800 * port_num));
   /* RW U1 p0 R2 hd948 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xd948);
   /* RW U1 p0 R3 h0001 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0001);
   /* RW U1 p0 R1 h32bb */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x32bb + (0x800 * port_num));
   /* RW U1 p0 R2 h61bf */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x61bf);
   /* RW U1 p0 R3 h0007 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0007);
   /* RW U1 p0 R1 h32bc */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x32bc + (0x800 * port_num));
   /* RW U1 p0 R2 h8944 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x8944);
   /* RW U1 p0 R3 h0004 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0004);
   /* RW U1 p0 R1 h32bd */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x32bd + (0x800 * port_num));
   /* RW U1 p0 R2 h1002 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x1002);
   /* RW U1 p0 R3 h0004 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0004);
   /* RW U1 p0 R1 h32be */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x32be + (0x800 * port_num));
   /* RW U1 p0 R2 h1002 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x1002);
   /* RW U1 p0 R3 h0004 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0004);
   /* RW U1 p0 R1 h32bf */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x32bf + (0x800 * port_num));
   /* RW U1 p0 R2 h41c7 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x41c7);
   /* RW U1 p0 R3 h0007 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0007);
   /* RW U1 p0 R1 h32c0 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x32c0 + (0x800 * port_num));
   /* RW U1 p0 R2 hd952 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xd952);
   /* RW U1 p0 R3 h001 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0001);
   /* RW U1 p0 R1 h32c1 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x32c1 + (0x800 * port_num));
   /* RW U1 p0 R2 h894a */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x894a);
   /* RW U1 p0 R3 h0004 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0004);
   /* RW U1 p0 R1 h32c2 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x32c2 + (0x800 * port_num));
   /* RW U1 p0 R2 h50a2 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x50a2);
   /* RW U1 p0 R3 h0004 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0004);
   /* RW U1 p0 R1 h32c3 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x32c3 + (0x800 * port_num));
   /* RW U1 p0 R2 h509a */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x509a);
   /* RW U1 p0 R3 h0004 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0004);
   /* RW U1 p0 R1 h32c4 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x32c4 + (0x800 * port_num));
   /* RW U1 p0 R2 h5092 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x5092);
   /* RW U1 p0 R3 h0004 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0004);
   /* RW U1 p0 R1 h32c5 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x32c5 + (0x800 * port_num));
   /* RW U1 p0 R2 h508a */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x508a);
   /* RW U1 p0 R3 h0004 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0004);
   /* RW U1 p0 R1 h32c6 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x32c6 + (0x800 * port_num));
   /* RW U1 p0 R2 h5082 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x5082);
   /* RW U1 p0 R3 h0004 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0004);
   /* RW U1 p0 R1 h32c7 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x32c7 + (0x800 * port_num));
   /* RW U1 p0 R2 h0000 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x0000);
   /* RW U1 p0 R3 h0004 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0004);
   /* RW U1 p0 R1 h32c8 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x32c8 + (0x800 * port_num));
   /* RW U1 p0 R2 h0000 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x0000);
   /* RW U1 p0 R3 h0003 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0003);
   /* RW U1 p0 R1 h32c9 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x32c9 + (0x800 * port_num));
   /* RW U1 p0 R2 h31ba */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x31ba);
   /* RW U1 p0 R3 h0007 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0007);
   /* RW U1 p0 R1 h32ca */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x32ca + (0x800 * port_num));
   /* RW U1 p0 R2 hffda */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xffda);
   /* RW U1 p0 R3 h0007 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0007);
   /* RW U1 p0 R1 h32cb */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x32cb + (0x800 * port_num));
   /* RW U1 p0 R2 h8946 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x8946);
   /* RW U1 p0 R3 h0004 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0004);
   /* RW U1 p0 R1 h32cc */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x32cc + (0x800 * port_num));
   /* RW U1 p0 R2 h5042 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x5042);
   /* RW U1 p0 R3 h0004 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0004);
   /* RW U1 p0 R1 h32cd */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x32cd + (0x800 * port_num));
   /* RW U1 p0 R2 h504a */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x504a);
   /* RW U1 p0 R3 h0004 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0004);
   /* RW U1 p0 R1 h32ce */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x32ce + (0x800 * port_num));
   /* RW U1 p0 R2 h5052 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x5052);
   /* RW U1 p0 R3 h0004 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0004);
   /* RW U1 p0 R1 h32cf */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x32cf + (0x800 * port_num));
   /* RW U1 p0 R2 h00ba */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x00ba);
   /* RW U1 p0 R3 h0002 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0002);
   /* RW U1 p0 R1 h32d0 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x32d0 + (0x800 * port_num));
   /* RW U1 p0 R2 h310f */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x310f);
   /* RW U1 p0 R3 h0003 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0003);
   /* RW U1 p0 R1 h32d1 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x32d1 + (0x800 * port_num));
   /* RW U1 p0 R2 hdf08 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xdf08);
   /* RW U1 p0 R3 h0001 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0001);
   /* RW U1 p0 R1 h32d2 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x32d2 + (0x800 * port_num));
   /* RW U1 p0 R2 h8d10 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x8d10);
   /* RW U1 p0 R3 h0004 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0004);
   /* RW U1 p0 R1 h32d3 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x32d3 + (0x800 * port_num));
   /* RW U1 p0 R2 h0029 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x0029);
   /* RW U1 p0 R3 h0002 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0002);
   /* RW U1 p0 R1 h32d4 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x32d4 + (0x800 * port_num));
   /* RW U1 p0 R2 h6072 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x6072);
   /* RW U1 p0 R3 h0004 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0004);
   /* RW U1 p0 R1 h32d5 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x32d5 + (0x800 * port_num));
   /* RW U1 p0 R2 h600a */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x600a);
   /* RW U1 p0 R3 h0004 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0004);
   /* RW U1 p0 R1 h32d6 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x32d6 + (0x800 * port_num));
   /* RW U1 p0 R2 h001e */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x001e);
   /* RW U1 p0 R3 h0002 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0002);
   /* RW U1 p0 R1 h32d7 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x32d7 + (0x800 * port_num));
   /* RW U1 p0 R2 h600a */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x600a);
   /* RW U1 p0 R3 h0004 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0004);
   /* RW U1 p0 R1 h32d8 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x32d8 + (0x800 * port_num));
   /* RW U1 p0 R2 h2002 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x2002);
   /* RW U1 p0 R3 h0004 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0004);
   /* RW U1 p0 R1 h32d9 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x32d9 + (0x800 * port_num));
   /* RW U1 p0 R2 h001e */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x001e);
   /* RW U1 p0 R3 h0002 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0002);
   /* RW U1 p0 R1 h32da */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x32da + (0x800 * port_num));
   /* RW U1 p0 R2 h8948 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x8948);
   /* RW U1 p0 R3 h0004 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0004);
   /* RW U1 p0 R1 h32db */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x32db + (0x800 * port_num));
   /* RW U1 p0 R2 h5042 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x5042);
   /* RW U1 p0 R3 h0004 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0004);
   /* RW U1 p0 R1 h32dc */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x32dc + (0x800 * port_num));
   /* RW U1 p0 R2 h504a */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x504a);
   /* RW U1 p0 R3 h0004 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0004);
   /* RW U1 p0 R1 h32dd */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x32dd + (0x800 * port_num));
   /* RW U1 p0 R2 h5052 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x5052);
   /* RW U1 p0 R3 h0004 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0004);
   /* RW U1 p0 R1 h32de */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x32de + (0x800 * port_num));
   /* RW U1 p0 R2 h505a */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x505a);
   /* RW U1 p0 R3 h0004 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0004);
   /* RW U1 p0 R1 h32df */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x32df + (0x800 * port_num));
   /* RW U1 p0 R2 h00ba */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x00ba);
   /* RW U1 p0 R3 h0002 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0002);
   /* RW U1 p0 R1 h32e0 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x32e0 + (0x800 * port_num));
   /* RW U1 p0 R2 h8948 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x8948);
   /* RW U1 p0 R3 h0004 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0004);
   /* RW U1 p0 R1 h32e1 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x32e1 + (0x800 * port_num));
   /* RW U1 p0 R2 h5092 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x5092);
   /* RW U1 p0 R3 h0004 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0004);
   /* RW U1 p0 R1 h32e2 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x32e2 + (0x800 * port_num));
   /* RW U1 p0 R2 h508a */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x508a);
   /* RW U1 p0 R3 h0004 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0004);
   /* RW U1 p0 R1 h32e3 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x32e3 + (0x800 * port_num));
   /* RW U1 p0 R2 h5082 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x5082);
   /* RW U1 p0 R3 h0004 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0004);
   /* RW U1 p0 R1 h32e4 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x32e4 + (0x800 * port_num));
   /* RW U1 p0 R2 h509a */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x509a);
   /* RW U1 p0 R3 h0004 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0004);
   /* RW U1 p0 R1 h32e5 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x32e5 + (0x800 * port_num));
   /* RW U1 p0 R2 h00ba */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x00ba);
   /* RW U1 p0 R3 h0002 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0002);
   /* RW U1 p0 R1 h32e6 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x32e6 + (0x800 * port_num));
   /* RW U1 p0 R2 hfe8f */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xfe8f);
   /* RW U1 p0 R3 h0007 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0007);
   /* RW U1 p0 R1 h32e7 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x32e7 + (0x800 * port_num));
   /* RW U1 p0 R2 h268b */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x268b);
   /* RW U1 p0 R3 h0003 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0003);
   /* RW U1 p0 R1 h32e8 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x32e8 + (0x800 * port_num));
   /* RW U1 p0 R2 hd119 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xd119);
   /* RW U1 p0 R3 h0000 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0000);
   /* RW U1 p0 R1 h32e9 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x32e9 + (0x800 * port_num));
   /* RW U1 p0 R2 he05c */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xe05c);
   /* RW U1 p0 R3 h0000 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0000);
   /* RW U1 p0 R1 h32ea */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x32ea + (0x800 * port_num));
   /* RW U1 p0 R2 he054 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xe054);
   /* RW U1 p0 R3 h0000 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0000);
   /* RW U1 p0 R1 h32eb */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x32eb + (0x800 * port_num));
   /* RW U1 p0 R2 he04c */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xe04c);
   /* RW U1 p0 R3 h0000 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0000);
   /* RW U1 p0 R1 h32ec */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x32ec + (0x800 * port_num));
   /* RW U1 p0 R2 h310f */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x310f);
   /* RW U1 p0 R3 h0003 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0003);
   /* RW U1 p0 R1 h32ed */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x32ed + (0x800 * port_num));
   /* RW U1 p0 R2 he044 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xe044);
   /* RW U1 p0 R3 h0000 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0000);
   /* RW U1 p0 R1 h32ee */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x32ee + (0x800 * port_num));
   /* RW U1 p0 R2 h228b */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x228b);
   /* RW U1 p0 R3 h0003 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0003);
   /* RW U1 p0 R1 h32ef */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x32ef + (0x800 * port_num));
   /* RW U1 p0 R2 h31ef */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x31ef);
   /* RW U1 p0 R3 h0003 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0003);
   /* RW U1 p0 R1 h32f0 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x32f0 + (0x800 * port_num));
   /* RW U1 p0 R2 h008f */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x008f);
   /* RW U1 p0 R3 h0002 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0002);
   /* RW U1 p0 R1 h32f1 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x32f1 + (0x800 * port_num));
   /* RW U1 p0 R2 hfe50 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xfe50);
   /* RW U1 p0 R3 h0007 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0007);
   /* RW U1 p0 R1 h32f2 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x32f2 + (0x800 * port_num));
   /* RW U1 p0 R2 h268b */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x268b);
   /* RW U1 p0 R3 h0003 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0003);
   /* RW U1 p0 R1 h32f3 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x32f3 + (0x800 * port_num));
   /* RW U1 p0 R2 hd119 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xd119);
   /* RW U1 p0 R3 h0000 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0000);
   /* RW U1 p0 R1 h32f4 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x32f4 + (0x800 * port_num));
   /* RW U1 p0 R2 he09c */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xe09c);
   /* RW U1 p0 R3 h0000 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0000);
   /* RW U1 p0 R1 h32f5 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x32f5 + (0x800 * port_num));
   /* RW U1 p0 R2 he084 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xe084);
   /* RW U1 p0 R3 h0000 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0000);
   /* RW U1 p0 R1 h32f6 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x32f6 + (0x800 * port_num));
   /* RW U1 p0 R2 he08c */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xe08c);
   /* RW U1 p0 R3 h0000 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0000);
   /* RW U1 p0 R1 h32f7 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x32f7 + (0x800 * port_num));
   /* RW U1 p0 R2 h310f */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x310f);
   /* RW U1 p0 R3 h0003 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0003);
   /* RW U1 p0 R1 h32f8 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x32f8 + (0x800 * port_num));
   /* RW U1 p0 R2 he094 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xe094);
   /* RW U1 p0 R3 h0000 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0000);
   /* RW U1 p0 R1 h32f9 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x32f9 + (0x800 * port_num));
   /* RW U1 p0 R2 h2293 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x2293);
   /* RW U1 p0 R3 h0003 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0003);
   /* RW U1 p0 R1 h32fa */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x32fa + (0x800 * port_num));
   /* RW U1 p0 R2 h31ef */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x31ef);
   /* RW U1 p0 R3 h0003 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0003);
   /* RW U1 p0 R1 h32fb */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x32fb + (0x800 * port_num));
   /* RW U1 p0 R2 h0050 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x0050);
   /* RW U1 p0 R3 h0002 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0002);
   /* RW U1 p0 R1 h32fc */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x32fc + (0x800 * port_num));
   /* RW U1 p0 R2 h1400 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x1400);
   /* RW U1 p0 R3 h0004 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0004);
   /* RW U1 p0 R1 h32fd */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x32fd + (0x800 * port_num));
   /* RW U1 p0 R2 h0000 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x0000);
   /* RW U1 p0 R3 h0003 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0003);
   /* RW U1 p0 R1 h32fe */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x32fe + (0x800 * port_num));
   /* RW U1 p0 R2 h0000 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x0000);
   /* RW U1 p0 R3 h0004 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0004);
   /* RW U1 p0 R1 h32ff */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x32ff + (0x800 * port_num));
   /* RW U1 p0 R2 h0000 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x0000);
   /* RW U1 p0 R3 h0003 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0003);

   /* RW U1 p0 R1 h3600 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3600 + (0x800 * port_num));
   /* RW U1 p0 R2 h00d3 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x00d3);
   /* RW U1 p0 R3 h0002 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0002);
   /* RW U1 p0 R1 h3601 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3601 + (0x800 * port_num));
   /* RW U1 p0 R2 he0b0 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xe0b0);
   /* RW U1 p0 R3 h0000 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0000);
   /* RW U1 p0 R1 h3602 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3602 + (0x800 * port_num));
   /* RW U1 p0 R2 hd055 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xd055);
   /* RW U1 p0 R3 h0000 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0000);
   /* RW U1 p0 R1 h3603 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3603 + (0x800 * port_num));
   /* RW U1 p0 R2 h6801 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x6801);
   /* RW U1 p0 R3 h0004 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0004);
   /* RW U1 p0 R1 h3604 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3604 + (0x800 * port_num));
   /* RW U1 p0 R2 hd1c0 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xd1c0);
   /* RW U1 p0 R3 h0000 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0000);
   /* RW U1 p0 R1 h3605 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3605 + (0x800 * port_num));
   /* RW U1 p0 R2 h31cf */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x31cf);
   /* RW U1 p0 R3 h0003 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0003);
   /* RW U1 p0 R1 h3606 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3606 + (0x800 * port_num));
   /* RW U1 p0 R2 hd1d1 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xd1d1);
   /* RW U1 p0 R3 h0007 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0007);
   /* RW U1 p0 R1 h3607 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3607 + (0x800 * port_num));
   /* RW U1 p0 R2 hd110 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xd110);
   /* RW U1 p0 R3 h0000 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0000);
   /* RW U1 p0 R1 h3608 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3608 + (0x800 * port_num));
   /* RW U1 p0 R2 hd0d1 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xd0d1);
   /* RW U1 p0 R3 h0000 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0000);
   /* RW U1 p0 R1 h3609 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3609 + (0x800 * port_num));
   /* RW U1 p0 R2 hd192 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xd192);
   /* RW U1 p0 R3 h0000 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0000);
   /* RW U1 p0 R1 h360a */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x360a + (0x800 * port_num));
   /* RW U1 p0 R2 h060e */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x060e);
   /* RW U1 p0 R3 h0007 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0007);
   /* RW U1 p0 R1 h360b */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x360b + (0x800 * port_num));
   /* RW U1 p0 R2 hb70e */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xb70e);
   /* RW U1 p0 R3 h0007 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0007);
   /* RW U1 p0 R1 h360c */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x360c + (0x800 * port_num));
   /* RW U1 p0 R2 h43d0 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x43d0);
   /* RW U1 p0 R3 h0005 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0005);
   /* RW U1 p0 R1 h360d */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x360d + (0x800 * port_num));
   /* RW U1 p0 R2 hc7f1 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xc7f1);
   /* RW U1 p0 R3 h0005 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0005);
   /* RW U1 p0 R1 h360e */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x360e + (0x800 * port_num));
   /* RW U1 p0 R2 hc995 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xc995);
   /* RW U1 p0 R3 h0007 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0007);
   /* RW U1 p0 R1 h360f */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x360f + (0x800 * port_num));
   /* RW U1 p0 R2 hc141 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xc141);
   /* RW U1 p0 R3 h0000 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0000);
   /* RW U1 p0 R1 h3610 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3610 + (0x800 * port_num));
   /* RW U1 p0 R2 h314f */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x314f);
   /* RW U1 p0 R3 h0003 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0003);
   /* RW U1 p0 R1 h3611 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3611 + (0x800 * port_num));
   /* RW U1 p0 R2 hdf4d */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xdf4d);
   /* RW U1 p0 R3 h0001 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0001);
   /* RW U1 p0 R1 h3612 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3612 + (0x800 * port_num));
   /* RW U1 p0 R2 h102c */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x102c);
   /* RW U1 p0 R3 h0002 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0002);
   /* RW U1 p0 R1 h3613 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3613 + (0x800 * port_num));
   /* RW U1 p0 R2 h20c5 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x20c5);
   /* RW U1 p0 R3 h0004 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0004);
   /* RW U1 p0 R1 h3614 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3614 + (0x800 * port_num));
   /* RW U1 p0 R2 h2105 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x2105);
   /* RW U1 p0 R3 h0004 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0004);
   /* RW U1 p0 R1 h3615 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3615 + (0x800 * port_num));
   /* RW U1 p0 R2 h60a5 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x60a5);
   /* RW U1 p0 R3 h0004 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0004);
   /* RW U1 p0 R1 h3616 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3616 + (0x800 * port_num));
   /* RW U1 p0 R2 h609d */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x609d);
   /* RW U1 p0 R3 h0004 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0004);
   /* RW U1 p0 R1 h3617 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3617 + (0x800 * port_num));
   /* RW U1 p0 R2 h6095 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x6095);
   /* RW U1 p0 R3 h0004 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0004);
   /* RW U1 p0 R1 h3618 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3618 + (0x800 * port_num));
   /* RW U1 p0 R2 h8724 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x8724);
   /* RW U1 p0 R3 h0007 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0007);
   /* RW U1 p0 R1 h3619 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3619 + (0x800 * port_num));
   /* RW U1 p0 R2 hc721 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xc721);
   /* RW U1 p0 R3 h0007 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0007);
   /* RW U1 p0 R1 h361a */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x361a + (0x800 * port_num));
   /* RW U1 p0 R2 hb71e */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xb71e);
   /* RW U1 p0 R3 h0007 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0007);
   /* RW U1 p0 R1 h361b */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x361b + (0x800 * port_num));
   /* RW U1 p0 R2 h2005 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x2005);
   /* RW U1 p0 R3 h0004 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0004);
   /* RW U1 p0 R1 h361c */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x361c + (0x800 * port_num));
   /* RW U1 p0 R2 h2005 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x2005);
   /* RW U1 p0 R3 h0004 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0004);
   /* RW U1 p0 R1 h361d */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x361d + (0x800 * port_num));
   /* RW U1 p0 R2 h002c */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x002c);
   /* RW U1 p0 R3 h0002 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0002);
   /* RW U1 p0 R1 h361e */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x361e + (0x800 * port_num));
   /* RW U1 p0 R2 h600d */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x600d);
   /* RW U1 p0 R3 h0004 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0004);
   /* RW U1 p0 R1 h361f */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x361f + (0x800 * port_num));
   /* RW U1 p0 R2 h2005 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x2005);
   /* RW U1 p0 R3 h0004 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0004);
   /* RW U1 p0 R1 h3620 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3620 + (0x800 * port_num));
   /* RW U1 p0 R2 h002c */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x002c);
   /* RW U1 p0 R3 h0002 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0002);
   /* RW U1 p0 R1 h3621 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3621 + (0x800 * port_num));
   /* RW U1 p0 R2 h6075 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x6075);
   /* RW U1 p0 R3 h0004 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0004);
   /* RW U1 p0 R1 h3622 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3622 + (0x800 * port_num));
   /* RW U1 p0 R2 h600d */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x600d);
   /* RW U1 p0 R3 h0004 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0004);
   /* RW U1 p0 R1 h3623 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3623 + (0x800 * port_num));
   /* RW U1 p0 R2 h002c */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x002c);
   /* RW U1 p0 R3 h0002 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0002);
   /* RW U1 p0 R1 h3624 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3624 + (0x800 * port_num));
   /* RW U1 p0 R2 h6075 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x6075);
   /* RW U1 p0 R3 h0004 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0004);
   /* RW U1 p0 R1 h3625 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3625 + (0x800 * port_num));
   /* RW U1 p0 R2 hd181 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xd181);
   /* RW U1 p0 R3 h0000 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0000);
   /* RW U1 p0 R1 h3626 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3626 + (0x800 * port_num));
   /* RW U1 p0 R2 hdf88 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xdf88);
   /* RW U1 p0 R3 h0001 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0001);
   /* RW U1 p0 R1 h3627 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3627 + (0x800 * port_num));
   /* RW U1 p0 R2 hd084 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xd084);
   /* RW U1 p0 R3 h0000 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0000);
   /* RW U1 p0 R1 h3628 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3628 + (0x800 * port_num));
   /* RW U1 p0 R2 hde88 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xde88);
   /* RW U1 p0 R3 h0001 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0001);
   /* RW U1 p0 R1 h3629 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3629 + (0x800 * port_num));
   /* RW U1 p0 R2 hdc88 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xdc88);
   /* RW U1 p0 R3 h0001 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0001);
   /* RW U1 p0 R1 h362a */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x362a + (0x800 * port_num));
   /* RW U1 p0 R2 hc3b2 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xc3b2);
   /* RW U1 p0 R3 h0001 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0001);
   /* RW U1 p0 R1 h362b */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x362b + (0x800 * port_num));
   /* RW U1 p0 R2 h2185 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x2185);
   /* RW U1 p0 R3 h0004 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0004);
   /* RW U1 p0 R1 h362c */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x362c + (0x800 * port_num));
   /* RW U1 p0 R2 hd08c */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xd08c);
   /* RW U1 p0 R3 h0000 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0000);
   /* RW U1 p0 R1 h362d */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x362d + (0x800 * port_num));
   /* RW U1 p0 R2 hc682 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xc682);
   /* RW U1 p0 R3 h0001 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0001);
   /* RW U1 p0 R1 h362e */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x362e + (0x800 * port_num));
   /* RW U1 p0 R2 h268a */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x268a);
   /* RW U1 p0 R3 h0003 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0003);
   /* RW U1 p0 R1 h362f */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x362f + (0x800 * port_num));
   /* RW U1 p0 R2 h2689 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x2689);
   /* RW U1 p0 R3 h0003 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0003);
   /* RW U1 p0 R1 h3630 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3630 + (0x800 * port_num));
   /* RW U1 p0 R2 h2688 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x2688);
   /* RW U1 p0 R3 h0003 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0003);
   /* RW U1 p0 R1 h3631 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3631 + (0x800 * port_num));
   /* RW U1 p0 R2 h2937 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x2937);
   /* RW U1 p0 R3 h0007 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0007);
   /* RW U1 p0 R1 h3632 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3632 + (0x800 * port_num));
   /* RW U1 p0 R2 hd188 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xd188);
   /* RW U1 p0 R3 h0000 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0000);
   /* RW U1 p0 R1 h3633 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3633 + (0x800 * port_num));
   /* RW U1 p0 R2 h2b4a */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x2b4a);
   /* RW U1 p0 R3 h0005 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0005);
   /* RW U1 p0 R1 h3634 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3634 + (0x800 * port_num));
   /* RW U1 p0 R2 ha769 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xa769);
   /* RW U1 p0 R3 h0005 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0005);
   /* RW U1 p0 R1 h3635 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3635 + (0x800 * port_num));
   /* RW U1 p0 R2 ha2c8 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xa2c8);
   /* RW U1 p0 R3 h0005 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0005);
   /* RW U1 p0 R1 h3636 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3636 + (0x800 * port_num));
   /* RW U1 p0 R2 h50b5 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x50b5);
   /* RW U1 p0 R3 h0002 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0002);
   /* RW U1 p0 R1 h3637 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3637 + (0x800 * port_num));
   /* RW U1 p0 R2 h3142 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x3142);
   /* RW U1 p0 R3 h0007 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0007);
   /* RW U1 p0 R1 h3638 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3638 + (0x800 * port_num));
   /* RW U1 p0 R2 hd158 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xd158);
   /* RW U1 p0 R3 h0000 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0000);
   /* RW U1 p0 R1 h3639 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3639 + (0x800 * port_num));
   /* RW U1 p0 R2 hfd3f */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xfd3f);
   /* RW U1 p0 R3 h0007 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0007);
   /* RW U1 p0 R1 h363a */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x363a + (0x800 * port_num));
   /* RW U1 p0 R2 h2aea */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x2aea);
   /* RW U1 p0 R3 h0005 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0005);
   /* RW U1 p0 R1 h363b */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x363b + (0x800 * port_num));
   /* RW U1 p0 R2 ha709 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xa709);
   /* RW U1 p0 R3 h0005 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0005);
   /* RW U1 p0 R1 h363c */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x363c + (0x800 * port_num));
   /* RW U1 p0 R2 ha2c8 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xa2c8);
   /* RW U1 p0 R3 h0005 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0005);
   /* RW U1 p0 R1 h363d */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x363d + (0x800 * port_num));
   /* RW U1 p0 R2 h50c0 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x50c0);
   /* RW U1 p0 R3 h0002 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0002);
   /* RW U1 p0 R1 h363e */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x363e + (0x800 * port_num));
   /* RW U1 p0 R2 h0042 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x0042);
   /* RW U1 p0 R3 h0002 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0002);
   /* RW U1 p0 R1 h363f */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x363f + (0x800 * port_num));
   /* RW U1 p0 R2 h2aea */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x2aea);
   /* RW U1 p0 R3 h0005 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0005);
   /* RW U1 p0 R1 h3640 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3640 + (0x800 * port_num));
   /* RW U1 p0 R2 ha709 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xa709);
   /* RW U1 p0 R3 h0005 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0005);
   /* RW U1 p0 R1 h3641 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3641 + (0x800 * port_num));
   /* RW U1 p0 R2 ha328 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xa328);
   /* RW U1 p0 R3 h0005 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0005);
   /* RW U1 p0 R1 h3642 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3642 + (0x800 * port_num));
   /* RW U1 p0 R2 h228a */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x228a);
   /* RW U1 p0 R3 h0003 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0003);
   /* RW U1 p0 R1 h3643 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3643 + (0x800 * port_num));
   /* RW U1 p0 R2 h2289 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x2289);
   /* RW U1 p0 R3 h0003 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0003);
   /* RW U1 p0 R1 h3644 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3644 + (0x800 * port_num));
   /* RW U1 p0 R2 h2288 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x2288);
   /* RW U1 p0 R3 h0003 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0003);
   /* RW U1 p0 R1 h3645 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3645 + (0x800 * port_num));
   /* RW U1 p0 R2 h614e */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x614e);
   /* RW U1 p0 R3 h0007 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0007);
   /* RW U1 p0 R1 h3646 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3646 + (0x800 * port_num));
   /* RW U1 p0 R2 h194a */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x194a);
   /* RW U1 p0 R3 h0007 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0007);
   /* RW U1 p0 R1 h3647 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3647 + (0x800 * port_num));
   /* RW U1 p0 R2 h2093 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x2093);
   /* RW U1 p0 R3 h0003 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0003);
   /* RW U1 p0 R1 h3648 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3648 + (0x800 * port_num));
   /* RW U1 p0 R2 h2094 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x2094);
   /* RW U1 p0 R3 h0003 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0003);
   /* RW U1 p0 R1 h3649 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3649 + (0x800 * port_num));
   /* RW U1 p0 R2 h004c */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x004c);
   /* RW U1 p0 R3 h0002 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0002);
   /* RW U1 p0 R1 h364a */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x364a + (0x800 * port_num));
   /* RW U1 p0 R2 h2291 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x2291);
   /* RW U1 p0 R3 h0003 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0003);
   /* RW U1 p0 R1 h364b */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x364b + (0x800 * port_num));
   /* RW U1 p0 R2 h2290 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x2290);
   /* RW U1 p0 R3 h0003 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0003);
   /* RW U1 p0 R1 h364c */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x364c + (0x800 * port_num));
   /* RW U1 p0 R2 h2686 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x2686);
   /* RW U1 p0 R3 h0003 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0003);
   /* RW U1 p0 R1 h364d */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x364d + (0x800 * port_num));
   /* RW U1 p0 R2 h2687 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x2687);
   /* RW U1 p0 R3 h0003 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0003);
   /* RW U1 p0 R1 h364e */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x364e + (0x800 * port_num));
   /* RW U1 p0 R2 hd14d */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xd14d);
   /* RW U1 p0 R3 h0000 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0000);
   /* RW U1 p0 R1 h364f */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x364f + (0x800 * port_num));
   /* RW U1 p0 R2 hdb41 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xdb41);
   /* RW U1 p0 R3 h0001 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0001);
   /* RW U1 p0 R1 h3650 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3650 + (0x800 * port_num));
   /* RW U1 p0 R2 hdd41 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xdd41);
   /* RW U1 p0 R3 h0001 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0001);
   /* RW U1 p0 R1 h3651 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3651 + (0x800 * port_num));
   /* RW U1 p0 R2 hd182 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xd182);
   /* RW U1 p0 R3 h0000 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0000);
   /* RW U1 p0 R1 h3652 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3652 + (0x800 * port_num));
   /* RW U1 p0 R2 h465f */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x465f);
   /* RW U1 p0 R3 h0007 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0007);
   /* RW U1 p0 R1 h3653 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3653 + (0x800 * port_num));
   /* RW U1 p0 R2 he15a */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xe15a);
   /* RW U1 p0 R3 h0007 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0007);
   /* RW U1 p0 R1 h3654 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3654 + (0x800 * port_num));
   /* RW U1 p0 R2 hb15a */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xb15a);
   /* RW U1 p0 R3 h0007 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0007);
   /* RW U1 p0 R1 h3655 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3655 + (0x800 * port_num));
   /* RW U1 p0 R2 ha95a */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xa95a);
   /* RW U1 p0 R3 h0007 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0007);
   /* RW U1 p0 R1 h3656 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3656 + (0x800 * port_num));
   /* RW U1 p0 R2 hfa02 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xfa02);
   /* RW U1 p0 R3 h0000 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0000);
   /* RW U1 p0 R1 h3657 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3657 + (0x800 * port_num));
   /* RW U1 p0 R2 h8764 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x8764);
   /* RW U1 p0 R3 h0007 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0007);
   /* RW U1 p0 R1 h3658 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3658 + (0x800 * port_num));
   /* RW U1 p0 R2 h0000 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x0000);
   /* RW U1 p0 R3 h0004 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0004);
   /* RW U1 p0 R1 h3659 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3659 + (0x800 * port_num));
   /* RW U1 p0 R2 h0000 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x0000);
   /* RW U1 p0 R3 h0003 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0003);
   /* RW U1 p0 R1 h365a */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x365a + (0x800 * port_num));
   /* RW U1 p0 R2 h5499 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x5499);
   /* RW U1 p0 R3 h0003 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0003);
   /* RW U1 p0 R1 h365b */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x365b + (0x800 * port_num));
   /* RW U1 p0 R2 h405d */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x405d);
   /* RW U1 p0 R3 h0002 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0002);
   /* RW U1 p0 R1 h365c */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x365c + (0x800 * port_num));
   /* RW U1 p0 R2 hc682 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xc682);
   /* RW U1 p0 R3 h0001 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0001);
   /* RW U1 p0 R1 h365d */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x365d + (0x800 * port_num));
   /* RW U1 p0 R2 h8942 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x8942);
   /* RW U1 p0 R3 h0004 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0004);
   /* RW U1 p0 R1 h365e */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x365e + (0x800 * port_num));
   /* RW U1 p0 R2 h9082 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x9082);
   /* RW U1 p0 R3 h0004 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0004);
   /* RW U1 p0 R1 h365f */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x365f + (0x800 * port_num));
   /* RW U1 p0 R2 hfa02 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xfa02);
   /* RW U1 p0 R3 h0000 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0000);
   /* RW U1 p0 R1 h3660 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3660 + (0x800 * port_num));
   /* RW U1 p0 R2 h8764 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x8764);
   /* RW U1 p0 R3 h0007 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0007);
   /* RW U1 p0 R1 h3661 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3661 + (0x800 * port_num));
   /* RW U1 p0 R2 hb76c */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xb76c);
   /* RW U1 p0 R3 h0007 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0007);
   /* RW U1 p0 R1 h3662 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3662 + (0x800 * port_num));
   /* RW U1 p0 R2 h0000 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x0000);
   /* RW U1 p0 R3 h0004 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0004);
   /* RW U1 p0 R1 h3663 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3663 + (0x800 * port_num));
   /* RW U1 p0 R2 h0000 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x0000);
   /* RW U1 p0 R3 h0003 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0003);
   /* RW U1 p0 R1 h3664 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3664 + (0x800 * port_num));
   /* RW U1 p0 R2 hd94a */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xd94a);
   /* RW U1 p0 R3 h0001 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0001);
   /* RW U1 p0 R1 h3665 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3665 + (0x800 * port_num));
   /* RW U1 p0 R2 h3183 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x3183);
   /* RW U1 p0 R3 h0007 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0007);
   /* RW U1 p0 R1 h3666 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3666 + (0x800 * port_num));
   /* RW U1 p0 R2 h0084 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x0084);
   /* RW U1 p0 R3 h0002 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0002);
   /* RW U1 p0 R1 h3667 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3667 + (0x800 * port_num));
   /* RW U1 p0 R2 h0000 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x0000);
   /* RW U1 p0 R3 h0003 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0003);
   /* RW U1 p0 R1 h3668 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3668 + (0x800 * port_num));
   /* RW U1 p0 R2 h508a */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x508a);
   /* RW U1 p0 R3 h0004 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0004);
   /* RW U1 p0 R1 h3669 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3669 + (0x800 * port_num));
   /* RW U1 p0 R2 h5082 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x5082);
   /* RW U1 p0 R3 h0004 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0004);
   /* RW U1 p0 R1 h366a */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x366a + (0x800 * port_num));
   /* RW U1 p0 R2 h0000 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x0000);
   /* RW U1 p0 R3 h0004 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0004);
   /* RW U1 p0 R1 h366b */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x366b + (0x800 * port_num));
   /* RW U1 p0 R2 h0000 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x0000);
   /* RW U1 p0 R3 h0003 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0003);
   /* RW U1 p0 R1 h366c */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x366c + (0x800 * port_num));
   /* RW U1 p0 R2 h61d3 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x61d3);
   /* RW U1 p0 R3 h0007 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0007);
   /* RW U1 p0 R1 h366d */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x366d + (0x800 * port_num));
   /* RW U1 p0 R2 hd14d */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xd14d);
   /* RW U1 p0 R3 h0000 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0000);
   /* RW U1 p0 R1 h366e */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x366e + (0x800 * port_num));
   /* RW U1 p0 R2 hdd41 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xdd41);
   /* RW U1 p0 R3 h0001 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0001);
   /* RW U1 p0 R1 h366f */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x366f + (0x800 * port_num));
   /* RW U1 p0 R2 hd1c1 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xd1c1);
   /* RW U1 p0 R3 h0000 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0000);
   /* RW U1 p0 R1 h3670 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3670 + (0x800 * port_num));
   /* RW U1 p0 R2 hfe2e */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xfe2e);
   /* RW U1 p0 R3 h0000 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0000);
   /* RW U1 p0 R1 h3671 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3671 + (0x800 * port_num));
   /* RW U1 p0 R2 h51c6 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x51c6);
   /* RW U1 p0 R3 h0003 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0003);
   /* RW U1 p0 R1 h3672 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3672 + (0x800 * port_num));
   /* RW U1 p0 R2 h407a */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x407a);
   /* RW U1 p0 R3 h0002 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0002);
   /* RW U1 p0 R1 h3673 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3673 + (0x800 * port_num));
   /* RW U1 p0 R2 hfe2d */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xfe2d);
   /* RW U1 p0 R3 h0000 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0000);
   /* RW U1 p0 R1 h3674 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3674 + (0x800 * port_num));
   /* RW U1 p0 R2 h51c6 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x51c6);
   /* RW U1 p0 R3 h0003 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0003);
   /* RW U1 p0 R1 h3675 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3675 + (0x800 * port_num));
   /* RW U1 p0 R2 h407c */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x407c);
   /* RW U1 p0 R3 h0002 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0002);
   /* RW U1 p0 R1 h3676 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3676 + (0x800 * port_num));
   /* RW U1 p0 R2 hfe2f */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xfe2f);
   /* RW U1 p0 R3 h0000 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0000);
   /* RW U1 p0 R1 h3677 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3677 + (0x800 * port_num));
   /* RW U1 p0 R2 h51c6 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x51c6);
   /* RW U1 p0 R3 h0003 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0003);
   /* RW U1 p0 R1 h3678 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3678 + (0x800 * port_num));
   /* RW U1 p0 R2 h407c */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x407c);
   /* RW U1 p0 R3 h0002 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0002);
   /* RW U1 p0 R1 h3679 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3679 + (0x800 * port_num));
   /* RW U1 p0 R2 h00d3 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x00d3);
   /* RW U1 p0 R3 h0002 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0002);
   /* RW U1 p0 R1 h367a */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x367a + (0x800 * port_num));
   /* RW U1 p0 R2 hd95c */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xd95c);
   /* RW U1 p0 R3 h0001 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0001);
   /* RW U1 p0 R1 h367b */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x367b + (0x800 * port_num));
   /* RW U1 p0 R2 h007d */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x007d);
   /* RW U1 p0 R3 h0002 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0002);
   /* RW U1 p0 R1 h367c */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x367c + (0x800 * port_num));
   /* RW U1 p0 R2 hd94c */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xd94c);
   /* RW U1 p0 R3 h0001 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0001);
   /* RW U1 p0 R1 h367d */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x367d + (0x800 * port_num));
   /* RW U1 p0 R2 h8948 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x8948);
   /* RW U1 p0 R3 h0004 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0004);
   /* RW U1 p0 R1 h367e */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x367e + (0x800 * port_num));
   /* RW U1 p0 R2 h509a */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x509a);
   /* RW U1 p0 R3 h0004 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0004);
   /* RW U1 p0 R1 h367f */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x367f + (0x800 * port_num));
   /* RW U1 p0 R2 h5092 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x5092);
   /* RW U1 p0 R3 h0004 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0004);
   /* RW U1 p0 R1 h3680 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3680 + (0x800 * port_num));
   /* RW U1 p0 R2 h508a */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x508a);
   /* RW U1 p0 R3 h0004 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0004);
   /* RW U1 p0 R1 h3681 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3681 + (0x800 * port_num));
   /* RW U1 p0 R2 h5082 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x5082);
   /* RW U1 p0 R3 h0004 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0004);
   /* RW U1 p0 R1 h3682 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3682 + (0x800 * port_num));
   /* RW U1 p0 R2 h00d3 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x00d3);
   /* RW U1 p0 R3 h0002 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0002);
   /* RW U1 p0 R1 h3683 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3683 + (0x800 * port_num));
   /* RW U1 p0 R2 h2989 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x2989);
   /* RW U1 p0 R3 h0007 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0007);
   /* RW U1 p0 R1 h3684 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3684 + (0x800 * port_num));
   /* RW U1 p0 R2 hffcb */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xffcb);
   /* RW U1 p0 R3 h0007 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0007);
   /* RW U1 p0 R1 h3685 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3685 + (0x800 * port_num));
   /* RW U1 p0 R2 h8946 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x8946);
   /* RW U1 p0 R3 h0004 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0004);
   /* RW U1 p0 R1 h3686 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3686 + (0x800 * port_num));
   /* RW U1 p0 R2 h5042 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x5042);
   /* RW U1 p0 R3 h0004 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0004);
   /* RW U1 p0 R1 h3687 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3687 + (0x800 * port_num));
   /* RW U1 p0 R2 h504a */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x504a);
   /* RW U1 p0 R3 h0004 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0004);
   /* RW U1 p0 R1 h3688 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3688 + (0x800 * port_num));
   /* RW U1 p0 R2 h5052 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x5052);
   /* RW U1 p0 R3 h0004 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0004);
   /* RW U1 p0 R1 h3689 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3689 + (0x800 * port_num));
   /* RW U1 p0 R2 h6193 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x6193);
   /* RW U1 p0 R3 h0007 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0007);
   /* RW U1 p0 R1 h368a */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x368a + (0x800 * port_num));
   /* RW U1 p0 R2 hd948 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xd948);
   /* RW U1 p0 R3 h0001 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0001);
   /* RW U1 p0 R1 h368b */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x368b + (0x800 * port_num));
   /* RW U1 p0 R2 h8944 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x8944);
   /* RW U1 p0 R3 h0004 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0004);
   /* RW U1 p0 R1 h368c */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x368c + (0x800 * port_num));
   /* RW U1 p0 R2 h1991 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x1991);
   /* RW U1 p0 R3 h0007 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0007);
   /* RW U1 p0 R1 h368d */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x368d + (0x800 * port_num));
   /* RW U1 p0 R2 h10c2 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x10c2);
   /* RW U1 p0 R3 h0004 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0004);
   /* RW U1 p0 R1 h368e */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x368e + (0x800 * port_num));
   /* RW U1 p0 R2 h1102 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x1102);
   /* RW U1 p0 R3 h0004 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0004);
   /* RW U1 p0 R1 h368f */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x368f + (0x800 * port_num));
   /* RW U1 p0 R2 h0000 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x0000);
   /* RW U1 p0 R3 h0004 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0004);
   /* RW U1 p0 R1 h3690 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3690 + (0x800 * port_num));
   /* RW U1 p0 R2 h0000 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x0000);
   /* RW U1 p0 R3 h0003 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0003);
   /* RW U1 p0 R1 h3691 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3691 + (0x800 * port_num));
   /* RW U1 p0 R2 h508a */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x508a);
   /* RW U1 p0 R3 h0004 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0004);
   /* RW U1 p0 R1 h3692 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3692 + (0x800 * port_num));
   /* RW U1 p0 R2 h5082 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x5082);
   /* RW U1 p0 R3 h0004 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0004);
   /* RW U1 p0 R1 h3693 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3693 + (0x800 * port_num));
   /* RW U1 p0 R2 h0000 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x0000);
   /* RW U1 p0 R3 h0004 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0004);
   /* RW U1 p0 R1 h3694 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3694 + (0x800 * port_num));
   /* RW U1 p0 R2 h0000 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x0000);
   /* RW U1 p0 R3 h0003 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0003);
   /* RW U1 p0 R1 h3695 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3695 + (0x800 * port_num));
   /* RW U1 p0 R2 hfa02 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xfa02);
   /* RW U1 p0 R3 h0000 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0000);
   /* RW U1 p0 R1 h3696 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3696 + (0x800 * port_num));
   /* RW U1 p0 R2 hfe08 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xfe08);
   /* RW U1 p0 R3 h0000 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0000);
   /* RW U1 p0 R1 h3697 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3697 + (0x800 * port_num));
   /* RW U1 p0 R2 hd14f */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xd14f);
   /* RW U1 p0 R3 h0000 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0000);
   /* RW U1 p0 R1 h3698 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3698 + (0x800 * port_num));
   /* RW U1 p0 R2 hdd41 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xdd41);
   /* RW U1 p0 R3 h0001 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0001);
   /* RW U1 p0 R1 h3699 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3699 + (0x800 * port_num));
   /* RW U1 p0 R2 h19a0 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x19a0);
   /* RW U1 p0 R3 h0007 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0007);
   /* RW U1 p0 R1 h369a */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x369a + (0x800 * port_num));
   /* RW U1 p0 R2 h0546 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x0546);
   /* RW U1 p0 R3 h0004 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0004);
   /* RW U1 p0 R1 h369b */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x369b + (0x800 * port_num));
   /* RW U1 p0 R2 h50ea */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x50ea);
   /* RW U1 p0 R3 h0004 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0004);
   /* RW U1 p0 R1 h369c */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x369c + (0x800 * port_num));
   /* RW U1 p0 R2 h50a2 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x50a2);
   /* RW U1 p0 R3 h0004 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0004);
   /* RW U1 p0 R1 h369d */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x369d + (0x800 * port_num));
   /* RW U1 p0 R2 h509a */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x509a);
   /* RW U1 p0 R3 h0004 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0004);
   /* RW U1 p0 R1 h369e */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x369e + (0x800 * port_num));
   /* RW U1 p0 R2 h5092 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x5092);
   /* RW U1 p0 R3 h0004 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0004);
   /* RW U1 p0 R1 h369f */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x369f + (0x800 * port_num));
   /* RW U1 p0 R2 h000f */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x000f);
   /* RW U1 p0 R3 h0002 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0002);
   /* RW U1 p0 R1 h36a0 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x36a0 + (0x800 * port_num));
   /* RW U1 p0 R2 h79a2 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x79a2);
   /* RW U1 p0 R3 h0007 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0007);
   /* RW U1 p0 R1 h36a1 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x36a1 + (0x800 * port_num));
   /* RW U1 p0 R2 hd988 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xd988);
   /* RW U1 p0 R3 h0001 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0001);
   /* RW U1 p0 R1 h36a2 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x36a2 + (0x800 * port_num));
   /* RW U1 p0 R2 h0546 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x0546);
   /* RW U1 p0 R3 h0004 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0004);
   /* RW U1 p0 R1 h36a3 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x36a3 + (0x800 * port_num));
   /* RW U1 p0 R2 h50e2 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x50e2);
   /* RW U1 p0 R3 h0004 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0004);
   /* RW U1 p0 R1 h36a4 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x36a4 + (0x800 * port_num));
   /* RW U1 p0 R2 hc141 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xc141);
   /* RW U1 p0 R3 h0000 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0000);
   /* RW U1 p0 R1 h36a5 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x36a5 + (0x800 * port_num));
   /* RW U1 p0 R2 hdd4d */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xdd4d);
   /* RW U1 p0 R3 h0001 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0001);
   /* RW U1 p0 R1 h36a6 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x36a6 + (0x800 * port_num));
   /* RW U1 p0 R2 hdf4d */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xdf4d);
   /* RW U1 p0 R3 h0001 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0001);
   /* RW U1 p0 R1 h36a7 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x36a7 + (0x800 * port_num));
   /* RW U1 p0 R2 hf9ac */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xf9ac);
   /* RW U1 p0 R3 h0007 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0007);
   /* RW U1 p0 R1 h36a8 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x36a8 + (0x800 * port_num));
   /* RW U1 p0 R2 h1142 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x1142);
   /* RW U1 p0 R3 h0004 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0004);
   /* RW U1 p0 R1 h36a9 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x36a9 + (0x800 * port_num));
   /* RW U1 p0 R2 h508a */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x508a);
   /* RW U1 p0 R3 h0004 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0004);
   /* RW U1 p0 R1 h36aa */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x36aa + (0x800 * port_num));
   /* RW U1 p0 R2 h5082 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x5082);
   /* RW U1 p0 R3 h0004 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0004);
   /* RW U1 p0 R1 h36ab */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x36ab + (0x800 * port_num));
   /* RW U1 p0 R2 h000f */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x000f);
   /* RW U1 p0 R3 h0002 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0002);
   /* RW U1 p0 R1 h36ac */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x36ac + (0x800 * port_num));
   /* RW U1 p0 R2 h316f */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x316f);
   /* RW U1 p0 R3 h0003 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0003);
   /* RW U1 p0 R1 h36ad */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x36ad + (0x800 * port_num));
   /* RW U1 p0 R2 h1142 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x1142);
   /* RW U1 p0 R3 h0004 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0004);
   /* RW U1 p0 R1 h36ae */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x36ae + (0x800 * port_num));
   /* RW U1 p0 R2 h10c2 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x10c2);
   /* RW U1 p0 R3 h0004 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0004);
   /* RW U1 p0 R1 h36af */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x36af + (0x800 * port_num));
   /* RW U1 p0 R2 h1102 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x1102);
   /* RW U1 p0 R3 h0004 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0004);
   /* RW U1 p0 R1 h36b0 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x36b0 + (0x800 * port_num));
   /* RW U1 p0 R2 h50a2 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x50a2);
   /* RW U1 p0 R3 h0004 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0004);
   /* RW U1 p0 R1 h36b1 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x36b1 + (0x800 * port_num));
   /* RW U1 p0 R2 h509a */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x509a);
   /* RW U1 p0 R3 h0004 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0004);
   /* RW U1 p0 R1 h36b2 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x36b2 + (0x800 * port_num));
   /* RW U1 p0 R2 h5092 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x5092);
   /* RW U1 p0 R3 h0004 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0004);
   /* RW U1 p0 R1 h36b3 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x36b3 + (0x800 * port_num));
   /* RW U1 p0 R2 h9002 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x9002);
   /* RW U1 p0 R3 h0004 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0004);
   /* RW U1 p0 R1 h36b4 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x36b4 + (0x800 * port_num));
   /* RW U1 p0 R2 h000f */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x000f);
   /* RW U1 p0 R3 h0002 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0002);
   /* RW U1 p0 R1 h36b5 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x36b5 + (0x800 * port_num));
   /* RW U1 p0 R2 hfeff */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xfeff);
   /* RW U1 p0 R3 h0007 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0007);
   /* RW U1 p0 R1 h36b6 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x36b6 + (0x800 * port_num));
   /* RW U1 p0 R2 h268b */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x268b);
   /* RW U1 p0 R3 h0003 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0003);
   /* RW U1 p0 R1 h36b7 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x36b7 + (0x800 * port_num));
   /* RW U1 p0 R2 hd159 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xd159);
   /* RW U1 p0 R3 h0000 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0000);
   /* RW U1 p0 R1 h36b8 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x36b8 + (0x800 * port_num));
   /* RW U1 p0 R2 he05d */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xe05d);
   /* RW U1 p0 R3 h0000 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0000);
   /* RW U1 p0 R1 h36b9 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x36b9 + (0x800 * port_num));
   /* RW U1 p0 R2 he055 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xe055);
   /* RW U1 p0 R3 h0000 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0000);
   /* RW U1 p0 R1 h36ba */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x36ba + (0x800 * port_num));
   /* RW U1 p0 R2 he04d */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xe04d);
   /* RW U1 p0 R3 h0000 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0000);
   /* RW U1 p0 R1 h36bb */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x36bb + (0x800 * port_num));
   /* RW U1 p0 R2 h314f */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x314f);
   /* RW U1 p0 R3 h0003 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0003);
   /* RW U1 p0 R1 h36bc */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x36bc + (0x800 * port_num));
   /* RW U1 p0 R2 he045 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xe045);
   /* RW U1 p0 R3 h0000 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0000);
   /* RW U1 p0 R1 h36bd */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x36bd + (0x800 * port_num));
   /* RW U1 p0 R2 h228b */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x228b);
   /* RW U1 p0 R3 h0003 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0003);
   /* RW U1 p0 R1 h36be */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x36be + (0x800 * port_num));
   /* RW U1 p0 R2 h31ef */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x31ef);
   /* RW U1 p0 R3 h0003 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0003);
   /* RW U1 p0 R1 h36bf */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x36bf + (0x800 * port_num));
   /* RW U1 p0 R2 h0042 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x0042);
   /* RW U1 p0 R3 h0002 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0002);
   /* RW U1 p0 R1 h36c0 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x36c0 + (0x800 * port_num));
   /* RW U1 p0 R2 hfe42 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xfe42);
   /* RW U1 p0 R3 h0007 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0007);
   /* RW U1 p0 R1 h36c1 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x36c1 + (0x800 * port_num));
   /* RW U1 p0 R2 h268b */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x268b);
   /* RW U1 p0 R3 h0003 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0003);
   /* RW U1 p0 R1 h36c2 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x36c2 + (0x800 * port_num));
   /* RW U1 p0 R2 hd159 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xd159);
   /* RW U1 p0 R3 h0000 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0000);
   /* RW U1 p0 R1 h36c3 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x36c3 + (0x800 * port_num));
   /* RW U1 p0 R2 he05d */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xe05d);
   /* RW U1 p0 R3 h0000 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0000);
   /* RW U1 p0 R1 h36c4 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x36c4 + (0x800 * port_num));
   /* RW U1 p0 R2 he055 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xe055);
   /* RW U1 p0 R3 h0000 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0000);
   /* RW U1 p0 R1 h36c5 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x36c5 + (0x800 * port_num));
   /* RW U1 p0 R2 he04d */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xe04d);
   /* RW U1 p0 R3 h0000 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0000);
   /* RW U1 p0 R1 h36c6 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x36c6 + (0x800 * port_num));
   /* RW U1 p0 R2 h314f */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x314f);
   /* RW U1 p0 R3 h0003 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0003);
   /* RW U1 p0 R1 h36c7 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x36c7 + (0x800 * port_num));
   /* RW U1 p0 R2 he045 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xe045);
   /* RW U1 p0 R3 h0000 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0000);
   /* RW U1 p0 R1 h36c8 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x36c8 + (0x800 * port_num));
   /* RW U1 p0 R2 h228b */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x228b);
   /* RW U1 p0 R3 h0003 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0003);
   /* RW U1 p0 R1 h36c9 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x36c9 + (0x800 * port_num));
   /* RW U1 p0 R2 h31ef */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x31ef);
   /* RW U1 p0 R3 h0003 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0003);
   /* RW U1 p0 R1 h36ca */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x36ca + (0x800 * port_num));
   /* RW U1 p0 R2 h0042 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x0042);
   /* RW U1 p0 R3 h0002 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0002);
   /* RW U1 p0 R1 h36cb */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x36cb + (0x800 * port_num));
   /* RW U1 p0 R2 h8948 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x8948);
   /* RW U1 p0 R3 h0004 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0004);
   /* RW U1 p0 R1 h36cc */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x36cc + (0x800 * port_num));
   /* RW U1 p0 R2 h5042 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x5042);
   /* RW U1 p0 R3 h0004 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0004);
   /* RW U1 p0 R1 h36cd */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x36cd + (0x800 * port_num));
   /* RW U1 p0 R2 h504a */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x504a);
   /* RW U1 p0 R3 h0004 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0004);
   /* RW U1 p0 R1 h36ce */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x36ce + (0x800 * port_num));
   /* RW U1 p0 R2 h5052 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x5052);
   /* RW U1 p0 R3 h0004 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0004);
   /* RW U1 p0 R1 h36cf */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x36cf + (0x800 * port_num));
   /* RW U1 p0 R2 h505a */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x505a);
   /* RW U1 p0 R3 h0004 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0004);
   /* RW U1 p0 R1 h36d0 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x36d0 + (0x800 * port_num));
   /* RW U1 p0 R2 h0089 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x0089);
   /* RW U1 p0 R3 h0002 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0002);
   /* RW U1 p0 R1 h36d1 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x36d1 + (0x800 * port_num));
   /* RW U1 p0 R2 h1400 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x1400);
   /* RW U1 p0 R3 h0004 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0004);
   /* RW U1 p0 R1 h36d2 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x36d2 + (0x800 * port_num));
   /* RW U1 p0 R2 h0000 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x0000);
   /* RW U1 p0 R3 h0003 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0003);
   /* RW U1 p0 R1 h36d3 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x36d3 + (0x800 * port_num));
   /* RW U1 p0 R2 h0000 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x0000);
   /* RW U1 p0 R3 h0004 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0004);
   /* RW U1 p0 R1 h36d4 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x36d4 + (0x800 * port_num));
   /* RW U1 p0 R2 h0000 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x0000);
   /* RW U1 p0 R3 h0003 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0003);

   /* RW U1 p0 R1 h3100 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3100 + (0x800 * port_num));
   /* RW U1 p0 R2 h00e8 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x00e8);
   /* RW U1 p0 R3 h0002 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0002);
   /* RW U1 p0 R1 h3101 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3101 + (0x800 * port_num));
   /* RW U1 p0 R2 hd151 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xd151);
   /* RW U1 p0 R3 h0000 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0000);
   /* RW U1 p0 R1 h3102 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3102 + (0x800 * port_num));
   /* RW U1 p0 R2 hdf48 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xdf48);
   /* RW U1 p0 R3 h0001 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0001);
   /* RW U1 p0 R1 h3103 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3103 + (0x800 * port_num));
   /* RW U1 p0 R2 hc005 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xc005);
   /* RW U1 p0 R3 h0000 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0000);
   /* RW U1 p0 R1 h3104 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3104 + (0x800 * port_num));
   /* RW U1 p0 R2 hd946 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xd946);
   /* RW U1 p0 R3 h0001 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0001);
   /* RW U1 p0 R1 h3105 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3105 + (0x800 * port_num));
   /* RW U1 p0 R2 hd806 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xd806);
   /* RW U1 p0 R3 h0001 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0001);
   /* RW U1 p0 R1 h3106 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3106 + (0x800 * port_num));
   /* RW U1 p0 R2 h8431 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x8431);
   /* RW U1 p0 R3 h0002 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0002);
   /* RW U1 p0 R1 h3107 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3107 + (0x800 * port_num));
   /* RW U1 p0 R2 hcc2e */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xcc2e);
   /* RW U1 p0 R3 h0002 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0002);
   /* RW U1 p0 R1 h3108 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3108 + (0x800 * port_num));
   /* RW U1 p0 R2 hac2b */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xac2b);
   /* RW U1 p0 R3 h0002 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0002);
   /* RW U1 p0 R1 h3109 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3109 + (0x800 * port_num));
   /* RW U1 p0 R2 h9c3f */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x9c3f);
   /* RW U1 p0 R3 h0002 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0002);
   /* RW U1 p0 R1 h310a */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x310a + (0x800 * port_num));
   /* RW U1 p0 R2 haa3f */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xaa3f);
   /* RW U1 p0 R3 h0002 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0002);
   /* RW U1 p0 R1 h310b */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x310b + (0x800 * port_num));
   /* RW U1 p0 R2 hc41d */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xc41d);
   /* RW U1 p0 R3 h0002 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0002);
   /* RW U1 p0 R1 h310c */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x310c + (0x800 * port_num));
   /* RW U1 p0 R2 he47a */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xe47a);
   /* RW U1 p0 R3 h0002 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0002);
   /* RW U1 p0 R1 h310d */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x310d + (0x800 * port_num));
   /* RW U1 p0 R2 hec8a */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xec8a);
   /* RW U1 p0 R3 h0002 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0002);
   /* RW U1 p0 R1 h310e */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x310e + (0x800 * port_num));
   /* RW U1 p0 R2 h8c35 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x8c35);
   /* RW U1 p0 R3 h0002 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0002);
   /* RW U1 p0 R1 h310f */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x310f + (0x800 * port_num));
   /* RW U1 p0 R2 hb463 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xb463);
   /* RW U1 p0 R3 h0002 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0002);
   /* RW U1 p0 R1 h3110 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3110 + (0x800 * port_num));
   /* RW U1 p0 R2 hbc6e */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xbc6e);
   /* RW U1 p0 R3 h0002 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0002);
   /* RW U1 p0 R1 h3111 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3111 + (0x800 * port_num));
   /* RW U1 p0 R2 h94a4 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x94a4);
   /* RW U1 p0 R3 h0002 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0002);
   /* RW U1 p0 R1 h3112 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3112 + (0x800 * port_num));
   /* RW U1 p0 R2 hf418 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xf418);
   /* RW U1 p0 R3 h0002 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0002);
   /* RW U1 p0 R1 h3113 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3113 + (0x800 * port_num));
   /* RW U1 p0 R2 hd840 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xd840);
   /* RW U1 p0 R3 h0000 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0000);
   /* RW U1 p0 R1 h3114 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3114 + (0x800 * port_num));
   /* RW U1 p0 R2 h5452 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x5452);
   /* RW U1 p0 R3 h0003 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0003);
   /* RW U1 p0 R1 h3115 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3115 + (0x800 * port_num));
   /* RW U1 p0 R2 h3834 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x3834);
   /* RW U1 p0 R3 h0002 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0002);
   /* RW U1 p0 R1 h3116 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3116 + (0x800 * port_num));
   /* RW U1 p0 R2 he004 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xe004);
   /* RW U1 p0 R3 h0000 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0000);
   /* RW U1 p0 R1 h3117 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3117 + (0x800 * port_num));
   /* RW U1 p0 R2 h0000 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x0000);
   /* RW U1 p0 R3 h0003 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0003);
   /* RW U1 p0 R1 h3118 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3118 + (0x800 * port_num));
   /* RW U1 p0 R2 h3126 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x3126);
   /* RW U1 p0 R3 h0003 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0003);
   /* RW U1 p0 R1 h3119 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3119 + (0x800 * port_num));
   /* RW U1 p0 R2 he068 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xe068);
   /* RW U1 p0 R3 h0000 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0000);
   /* RW U1 p0 R1 h311a */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x311a + (0x800 * port_num));
   /* RW U1 p0 R2 h7008 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x7008);
   /* RW U1 p0 R3 h0000 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0000);
   /* RW U1 p0 R1 h311b */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x311b + (0x800 * port_num));
   /* RW U1 p0 R2 he004 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xe004);
   /* RW U1 p0 R3 h0000 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0000);
   /* RW U1 p0 R1 h311c */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x311c + (0x800 * port_num));
   /* RW U1 p0 R2 h0000 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x0000);
   /* RW U1 p0 R3 h0003 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0003);
   /* RW U1 p0 R1 h311d */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x311d + (0x800 * port_num));
   /* RW U1 p0 R2 hcc2a */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xcc2a);
   /* RW U1 p0 R3 h0007 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0007);
   /* RW U1 p0 R1 h311e */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x311e + (0x800 * port_num));
   /* RW U1 p0 R2 hd0d1 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xd0d1);
   /* RW U1 p0 R3 h0000 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0000);
   /* RW U1 p0 R1 h311f */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x311f + (0x800 * port_num));
   /* RW U1 p0 R2 h8329 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x8329);
   /* RW U1 p0 R3 h0007 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0007);
   /* RW U1 p0 R1 h3120 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3120 + (0x800 * port_num));
   /* RW U1 p0 R2 hf429 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xf429);
   /* RW U1 p0 R3 h0007 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0007);
   /* RW U1 p0 R1 h3121 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3121 + (0x800 * port_num));
   /* RW U1 p0 R2 h312e */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x312e);
   /* RW U1 p0 R3 h0003 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0003);
   /* RW U1 p0 R1 h3122 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3122 + (0x800 * port_num));
   /* RW U1 p0 R2 hd8e0 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x8e0);
   /* RW U1 p0 R3 h0000 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0000);
   /* RW U1 p0 R1 h3123 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3123 + (0x800 * port_num));
   /* RW U1 p0 R2 hd802 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xd802);
   /* RW U1 p0 R3 h0001 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0001);
   /* RW U1 p0 R1 h3124 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3124 + (0x800 * port_num));
   /* RW U1 p0 R2 hc140 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xc140);
   /* RW U1 p0 R3 h0000 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0000);
   /* RW U1 p0 R1 h3125 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3125 + (0x800 * port_num));
   /* RW U1 p0 R2 h6b06 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x6b06);
   /* RW U1 p0 R3 h0007 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0007);
   /* RW U1 p0 R1 h3126 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3126 + (0x800 * port_num));
   /* RW U1 p0 R2 hd804 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xd804);
   /* RW U1 p0 R3 h0001 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0001);
   /* RW U1 p0 R1 h3127 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3127 + (0x800 * port_num));
   /* RW U1 p0 R2 hc140 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xc140);
   /* RW U1 p0 R3 h0000 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0000);
   /* RW U1 p0 R1 h3128 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3128 + (0x800 * port_num));
   /* RW U1 p0 R2 h0006 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x0006);
   /* RW U1 p0 R3 h0002 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0002);
   /* RW U1 p0 R1 h3129 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3129 + (0x800 * port_num));
   /* RW U1 p0 R2 h312e */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x312e);
   /* RW U1 p0 R3 h0003 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0003);
   /* RW U1 p0 R1 h312a */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x312a + (0x800 * port_num));
   /* RW U1 p0 R2 h00e6 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x00e6);
   /* RW U1 p0 R3 h0002 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0002);
   /* RW U1 p0 R1 h312b */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x312b + (0x800 * port_num));
   /* RW U1 p0 R2 h312b */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x312b);
   /* RW U1 p0 R3 h0003 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0003);
   /* RW U1 p0 R1 h312c */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x312c + (0x800 * port_num));
   /* RW U1 p0 R2 hd802 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xd802);
   /* RW U1 p0 R3 h0001 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0001);
   /* RW U1 p0 R1 h312d */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x312d + (0x800 * port_num));
   /* RW U1 p0 R2 h0005 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x0005);
   /* RW U1 p0 R3 h0002 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0002);
   /* RW U1 p0 R1 h312e */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x312e + (0x800 * port_num));
   /* RW U1 p0 R2 h312a */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x312a);
   /* RW U1 p0 R3 h0003 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0003);
   /* RW U1 p0 R1 h312f */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x312f + (0x800 * port_num));
   /* RW U1 p0 R2 hd801 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xd801);
   /* RW U1 p0 R3 h0001 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0001);
   /* RW U1 p0 R1 h3130 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3130 + (0x800 * port_num));
   /* RW U1 p0 R2 h0006 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x0006);
   /* RW U1 p0 R3 h0002 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0002);
   /* RW U1 p0 R1 h3131 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3131 + (0x800 * port_num));
   /* RW U1 p0 R2 h3129 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x3129);
   /* RW U1 p0 R3 h0003 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0003);
   /* RW U1 p0 R1 h3132 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3132 + (0x800 * port_num));
   /* RW U1 p0 R2 hd801 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xd801);
   /* RW U1 p0 R3 h0001 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0001);
   /* RW U1 p0 R1 h3133 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3133 + (0x800 * port_num));
   /* RW U1 p0 R2 h0006 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x0006);
   /* RW U1 p0 R3 h0002 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0002);
   /* RW U1 p0 R1 h3134 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3134 + (0x800 * port_num));
   /* RW U1 p0 R2 hd801 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xd801);
   /* RW U1 p0 R3 h0001 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0001);
   /* RW U1 p0 R1 h3135 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3135 + (0x800 * port_num));
   /* RW U1 p0 R2 hd8a0 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xd8a0);
   /* RW U1 p0 R3 h0000 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0000);
   /* RW U1 p0 R1 h3136 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3136 + (0x800 * port_num));
   /* RW U1 p0 R2 h5496 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x5496);
   /* RW U1 p0 R3 h0003 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0003);
   /* RW U1 p0 R1 h3137 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3137 + (0x800 * port_num));
   /* RW U1 p0 R2 h48e6 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x48e6);
   /* RW U1 p0 R3 h0002 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0002);
   /* RW U1 p0 R1 h3138 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3138 + (0x800 * port_num));
   /* RW U1 p0 R2 hd8a0 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xd8a0);
   /* RW U1 p0 R3 h0000 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0000);
   /* RW U1 p0 R1 h3139 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3139 + (0x800 * port_num));
   /* RW U1 p0 R2 he098 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xe098);
   /* RW U1 p0 R3 h0001 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0001);
   /* RW U1 p0 R1 h313a */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x313a + (0x800 * port_num));
   /* RW U1 p0 R2 h5497 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x5497);
   /* RW U1 p0 R3 h0003 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0003);
   /* RW U1 p0 R1 h313b */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x313b + (0x800 * port_num));
   /* RW U1 p0 R2 h48e6 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x48e6);
   /* RW U1 p0 R3 h0002 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0002);
   /* RW U1 p0 R1 h313c */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x313c + (0x800 * port_num));
   /* RW U1 p0 R2 h312c */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x312c);
   /* RW U1 p0 R3 h0003 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0003);
   /* RW U1 p0 R1 h313d */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x313d + (0x800 * port_num));
   /* RW U1 p0 R2 hd8a0 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xd8a0);
   /* RW U1 p0 R3 h0000 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0000);
   /* RW U1 p0 R1 h313e */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x313e + (0x800 * port_num));
   /* RW U1 p0 R2 h0006 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x0006);
   /* RW U1 p0 R3 h0002 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0002);
   /* RW U1 p0 R1 h313f */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x313f + (0x800 * port_num));
   /* RW U1 p0 R2 h312d */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x312d);
   /* RW U1 p0 R3 h0003 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0003);
   /* RW U1 p0 R1 h3140 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3140 + (0x800 * port_num));
   /* RW U1 p0 R2 hd860 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xd860);
   /* RW U1 p0 R3 h0000 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0000);
   /* RW U1 p0 R1 h3141 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3141 + (0x800 * port_num));
   /* RW U1 p0 R2 hd8a0 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xd8a0);
   /* RW U1 p0 R3 h0000 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0000);
   /* RW U1 p0 R1 h3142 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3142 + (0x800 * port_num));
   /* RW U1 p0 R2 hc251 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xc251);
   /* RW U1 p0 R3 h0007 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0007);
   /* RW U1 p0 R1 h3143 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3143 + (0x800 * port_num));
   /* RW U1 p0 R2 hd8e0 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xd8e0);
   /* RW U1 p0 R3 h0000 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0000);
   /* RW U1 p0 R1 h3144 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3144 + (0x800 * port_num));
   /* RW U1 p0 R2 hd8e0 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xd8e0);
   /* RW U1 p0 R3 h0000 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0000);
   /* RW U1 p0 R1 h3145 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3145 + (0x800 * port_num));
   /* RW U1 p0 R2 hc351 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xc351);
   /* RW U1 p0 R3 h0007 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0007);
   /* RW U1 p0 R1 h3146 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3146 + (0x800 * port_num));
   /* RW U1 p0 R2 hd8e0 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xd8e0);
   /* RW U1 p0 R3 h0000 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0000);
   /* RW U1 p0 R1 h3147 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3147 + (0x800 * port_num));
   /* RW U1 p0 R2 hd8e0 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xd8e0);
   /* RW U1 p0 R3 h0000 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0000);
   /* RW U1 p0 R1 h3148 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3148 + (0x800 * port_num));
   /* RW U1 p0 R2 hc351 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xc351);
   /* RW U1 p0 R3 h0007 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0007);
   /* RW U1 p0 R1 h3149 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3149 + (0x800 * port_num));
   /* RW U1 p0 R2 hd8e0 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xd8e0);
   /* RW U1 p0 R3 h0000 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0000);
   /* RW U1 p0 R1 h314a */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x314a + (0x800 * port_num));
   /* RW U1 p0 R2 hd8e0 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xd8e0);
   /* RW U1 p0 R3 h0000 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0000);
   /* RW U1 p0 R1 h314b */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x314b + (0x800 * port_num));
   /* RW U1 p0 R2 hc351 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xc351);
   /* RW U1 p0 R3 h0007 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0007);
   /* RW U1 p0 R1 h314c */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x314c + (0x800 * port_num));
   /* RW U1 p0 R2 hd8e0 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xd8e0);
   /* RW U1 p0 R3 h0000 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0000);
   /* RW U1 p0 R1 h314d */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x314d + (0x800 * port_num));
   /* RW U1 p0 R2 hd8e0 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xd8e0);
   /* RW U1 p0 R3 h0000 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0000);
   /* RW U1 p0 R1 h314e */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x314e + (0x800 * port_num));
   /* RW U1 p0 R2 hc351 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xc351);
   /* RW U1 p0 R3 h0007 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0007);
   /* RW U1 p0 R1 h314f */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x314f + (0x800 * port_num));
   /* RW U1 p0 R2 he004 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xe004);
   /* RW U1 p0 R3 h0000 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0000);
   /* RW U1 p0 R1 h3150 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3150 + (0x800 * port_num));
   /* RW U1 p0 R2 h0000 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x0000);
   /* RW U1 p0 R3 h0003 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0003);
   /* RW U1 p0 R1 h3151 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3151 + (0x800 * port_num));
   /* RW U1 p0 R2 he453 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xe453);
   /* RW U1 p0 R3 h0001 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0001);
   /* RW U1 p0 R1 h3152 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3152 + (0x800 * port_num));
   /* RW U1 p0 R2 h105d */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x105d);
   /* RW U1 p0 R3 h0002 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0002);
   /* RW U1 p0 R1 h3153 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3153 + (0x800 * port_num));
   /* RW U1 p0 R2 hd840 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xd840);
   /* RW U1 p0 R3 h0000 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0000);
   /* RW U1 p0 R1 h3154 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3154 + (0x800 * port_num));
   /* RW U1 p0 R2 hde4c */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xde4c);
   /* RW U1 p0 R3 h0001 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0001);
   /* RW U1 p0 R1 h3155 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3155 + (0x800 * port_num));
   /* RW U1 p0 R2 h5844 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x5844);
   /* RW U1 p0 R3 h0003 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0003);
   /* RW U1 p0 R1 h3156 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3156 + (0x800 * port_num));
   /* RW U1 p0 R2 h4063 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x4063);
   /* RW U1 p0 R3 h0002 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0002);
   /* RW U1 p0 R1 h3157 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3157 + (0x800 * port_num));
   /* RW U1 p0 R2 h5846 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x5846);
   /* RW U1 p0 R3 h0003 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0003);
   /* RW U1 p0 R1 h3158 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3158 + (0x800 * port_num));
   /* RW U1 p0 R2 h406e */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x406e);
   /* RW U1 p0 R3 h0002 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0002);
   /* RW U1 p0 R1 h3159 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3159 + (0x800 * port_num));
   /* RW U1 p0 R2 h5840 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x5840);
   /* RW U1 p0 R3 h0003 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0003);
   /* RW U1 p0 R1 h315a */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x315a + (0x800 * port_num));
   /* RW U1 p0 R2 h4061 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x4061);
   /* RW U1 p0 R3 h0002 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0002);
   /* RW U1 p0 R1 h315b */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x315b + (0x800 * port_num));
   /* RW U1 p0 R2 he004 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xe004);
   /* RW U1 p0 R3 h0000 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0000);
   /* RW U1 p0 R1 h315c */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x315c + (0x800 * port_num));
   /* RW U1 p0 R2 h0000 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x0000);
   /* RW U1 p0 R3 h0003 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0003);
   /* RW U1 p0 R1 h315d */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x315d + (0x800 * port_num));
   /* RW U1 p0 R2 he095 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xe095);
   /* RW U1 p0 R3 h0001 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0001);
   /* RW U1 p0 R1 h315e */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x315e + (0x800 * port_num));
   /* RW U1 p0 R2 he494 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xe494);
   /* RW U1 p0 R3 h0001 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0001);
   /* RW U1 p0 R1 h315f */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x315f + (0x800 * port_num));
   /* RW U1 p0 R2 h1005 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x1005);
   /* RW U1 p0 R3 h0002 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0002);
   /* RW U1 p0 R1 h3160 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3160 + (0x800 * port_num));
   /* RW U1 p0 R2 h0053 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x0053);
   /* RW U1 p0 R3 h0002 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0002);
   /* RW U1 p0 R1 h3161 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3161 + (0x800 * port_num));
   /* RW U1 p0 R2 hd802 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xd802);
   /* RW U1 p0 R3 h0001 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0001);
   /* RW U1 p0 R1 h3162 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3162 + (0x800 * port_num));
   /* RW U1 p0 R2 h0005 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x0005);
   /* RW U1 p0 R3 h0002 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0002);
   /* RW U1 p0 R1 h3163 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3163 + (0x800 * port_num));
   /* RW U1 p0 R2 h3122 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x3122);
   /* RW U1 p0 R3 h0003 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0003);
   /* RW U1 p0 R1 h3164 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3164 + (0x800 * port_num));
   /* RW U1 p0 R2 hd840 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xd840);
   /* RW U1 p0 R3 h0000 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0000);
   /* RW U1 p0 R1 h3165 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3165 + (0x800 * port_num));
   /* RW U1 p0 R2 hd804 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xd804);
   /* RW U1 p0 R3 h0001 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0001);
   /* RW U1 p0 R1 h3166 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3166 + (0x800 * port_num));
   /* RW U1 p0 R2 he050 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xe050);
   /* RW U1 p0 R3 h0001 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0001);
   /* RW U1 p0 R1 h3167 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3167 + (0x800 * port_num));
   /* RW U1 p0 R2 hde47 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xde47);
   /* RW U1 p0 R3 h0001 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0001);
   /* RW U1 p0 R1 h3168 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3168 + (0x800 * port_num));
   /* RW U1 p0 R2 h584a */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x584a);
   /* RW U1 p0 R3 h0003 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0003);
   /* RW U1 p0 R1 h3169 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3169 + (0x800 * port_num));
   /* RW U1 p0 R2 h30e6 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x30e6);
   /* RW U1 p0 R3 h0002 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0002);
   /* RW U1 p0 R1 h316a */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x316a + (0x800 * port_num));
   /* RW U1 p0 R2 hda44 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xda44);
   /* RW U1 p0 R3 h0001 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0001);
   /* RW U1 p0 R1 h316b */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x316b + (0x800 * port_num));
   /* RW U1 p0 R2 h59c0 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x59c0);
   /* RW U1 p0 R3 h0000 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0000);
   /* RW U1 p0 R1 h316c */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x316c + (0x800 * port_num));
   /* RW U1 p0 R2 hc801 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xc801);
   /* RW U1 p0 R3 h0001 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0001);
   /* RW U1 p0 R1 h316d */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x316d + (0x800 * port_num));
   /* RW U1 p0 R2 h0091 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x0091);
   /* RW U1 p0 R3 h0002 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0002);
   /* RW U1 p0 R1 h316e */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x316e + (0x800 * port_num));
   /* RW U1 p0 R2 h3123 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x3123);
   /* RW U1 p0 R3 h0003 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0003);
   /* RW U1 p0 R1 h316f */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x316f + (0x800 * port_num));
   /* RW U1 p0 R2 hd803 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xd803);
   /* RW U1 p0 R3 h0001 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0001);
   /* RW U1 p0 R1 h3170 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3170 + (0x800 * port_num));
   /* RW U1 p0 R2 hd9c0 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xd9c0);
   /* RW U1 p0 R3 h0000 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0000);
   /* RW U1 p0 R1 h3171 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3171 + (0x800 * port_num));
   /* RW U1 p0 R2 hd811 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xd811);
   /* RW U1 p0 R3 h0001 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0001);
   /* RW U1 p0 R1 h3172 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3172 + (0x800 * port_num));
   /* RW U1 p0 R2 hdfc8 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xdfc8);
   /* RW U1 p0 R3 h0001 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0001);
   /* RW U1 p0 R1 h3173 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3173 + (0x800 * port_num));
   /* RW U1 p0 R2 h0091 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x0091);
   /* RW U1 p0 R3 h0002 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0002);
   /* RW U1 p0 R1 h3174 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3174 + (0x800 * port_num));
   /* RW U1 p0 R2 h3125 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x3125);
   /* RW U1 p0 R3 h0003 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0003);
   /* RW U1 p0 R1 h3175 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3175 + (0x800 * port_num));
   /* RW U1 p0 R2 hd860 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xd860);
   /* RW U1 p0 R3 h0000 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0000);
   /* RW U1 p0 R1 h3176 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3176 + (0x800 * port_num));
   /* RW U1 p0 R2 hde4a */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xde4a);
   /* RW U1 p0 R3 h0001 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0001);
   /* RW U1 p0 R1 h3177 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3177 + (0x800 * port_num));
   /* RW U1 p0 R2 h18e6 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x18e6);
   /* RW U1 p0 R3 h0002 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0002);
   /* RW U1 p0 R1 h3178 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3178 + (0x800 * port_num));
   /* RW U1 p0 R2 hd405 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xd405);
   /* RW U1 p0 R3 h0002 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0002);
   /* RW U1 p0 R1 h3179 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3179 + (0x800 * port_num));
   /* RW U1 p0 R2 h0006 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x0006);
   /* RW U1 p0 R3 h0002 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0002);
   /* RW U1 p0 R1 h317a */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x317a + (0x800 * port_num));
   /* RW U1 p0 R2 h3124 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x3124);
   /* RW U1 p0 R3 h0003 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0003);
   /* RW U1 p0 R1 h317b */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x317b + (0x800 * port_num));
   /* RW U1 p0 R2 hdd49 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xdd49);
   /* RW U1 p0 R3 h0001 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0001);
   /* RW U1 p0 R1 h317c */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x317c + (0x800 * port_num));
   /* RW U1 p0 R2 hd860 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xd860);
   /* RW U1 p0 R3 h0000 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0000);
   /* RW U1 p0 R1 h317d */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x317d + (0x800 * port_num));
   /* RW U1 p0 R2 he05b */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xe05b);
   /* RW U1 p0 R3 h0001 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0001);
   /* RW U1 p0 R1 h317e */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x317e + (0x800 * port_num));
   /* RW U1 p0 R2 hc24d */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xc24d);
   /* RW U1 p0 R3 h0001 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0001);
   /* RW U1 p0 R1 h317f */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x317f + (0x800 * port_num));
   /* RW U1 p0 R2 he011 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xe011);
   /* RW U1 p0 R3 h0000 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0000);
   /* RW U1 p0 R1 h3180 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3180 + (0x800 * port_num));
   /* RW U1 p0 R2 hf034 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xf034);
   /* RW U1 p0 R3 h0000 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0000);
   /* RW U1 p0 R1 h3181 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3181 + (0x800 * port_num));
   /* RW U1 p0 R2 hf03c */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xf03c);
   /* RW U1 p0 R3 h0000 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0000);
   /* RW U1 p0 R1 h3182 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3182 + (0x800 * port_num));
   /* RW U1 p0 R2 h7906 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x7906);
   /* RW U1 p0 R3 h0007 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0007);
   /* RW U1 p0 R1 h3183 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3183 + (0x800 * port_num));
   /* RW U1 p0 R2 hd802 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xd802);
   /* RW U1 p0 R3 h0001 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0001);
   /* RW U1 p0 R1 h3184 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3184 + (0x800 * port_num));
   /* RW U1 p0 R2 hd8a0 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xd8a0);
   /* RW U1 p0 R3 h0000 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0000);
   /* RW U1 p0 R1 h3185 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3185 + (0x800 * port_num));
   /* RW U1 p0 R2 h0288 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x0288);
   /* RW U1 p0 R3 h0007 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0007);
   /* RW U1 p0 R1 h3186 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3186 + (0x800 * port_num));
   /* RW U1 p0 R2 h1f27 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x1f27);
   /* RW U1 p0 R3 h0005 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0005);
   /* RW U1 p0 R1 h3187 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3187 + (0x800 * port_num));
   /* RW U1 p0 R2 h9b46 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x9b46);
   /* RW U1 p0 R3 h0005 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0005);
   /* RW U1 p0 R1 h3188 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3188 + (0x800 * port_num));
   /* RW U1 p0 R2 hd801 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xd801);
   /* RW U1 p0 R3 h0001 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0001);
   /* RW U1 p0 R1 h3189 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3189 + (0x800 * port_num));
   /* RW U1 p0 R2 h0006 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x0006);
   /* RW U1 p0 R3 h0002 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0002);
   /* RW U1 p0 R1 h318a */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x318a + (0x800 * port_num));
   /* RW U1 p0 R2 h3124 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x3124);
   /* RW U1 p0 R3 h0003 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0003);
   /* RW U1 p0 R1 h318b */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x318b + (0x800 * port_num));
   /* RW U1 p0 R2 hd802 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xd802);
   /* RW U1 p0 R3 h0001 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0001);
   /* RW U1 p0 R1 h318c */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x318c + (0x800 * port_num));
   /* RW U1 p0 R2 hdd49 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xdd49);
   /* RW U1 p0 R3 h0001 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0001);
   /* RW U1 p0 R1 h318d */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x318d + (0x800 * port_num));
   /* RW U1 p0 R2 he015 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xe015);
   /* RW U1 p0 R3 h0000 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0000);
   /* RW U1 p0 R1 h318e */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x318e + (0x800 * port_num));
   /* RW U1 p0 R2 hf03c */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xf03c);
   /* RW U1 p0 R3 h0000 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0000);
   /* RW U1 p0 R1 h318f */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x318f + (0x800 * port_num));
   /* RW U1 p0 R2 h316f */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x316f);
   /* RW U1 p0 R3 h0003 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0003);
   /* RW U1 p0 R1 h3190 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3190 + (0x800 * port_num));
   /* RW U1 p0 R2 h0006 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x0006);
   /* RW U1 p0 R3 h0002 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0002);
   /* RW U1 p0 R1 h3191 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3191 + (0x800 * port_num));
   /* RW U1 p0 R2 h59d1 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x59d1);
   /* RW U1 p0 R3 h0003 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0003);
   /* RW U1 p0 R1 h3192 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3192 + (0x800 * port_num));
   /* RW U1 p0 R2 h409b */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x409b);
   /* RW U1 p0 R3 h0002 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0002);
   /* RW U1 p0 R1 h3193 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3193 + (0x800 * port_num));
   /* RW U1 p0 R2 h59ef */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x59ef);
   /* RW U1 p0 R3 h0003 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0003);
   /* RW U1 p0 R1 h3194 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3194 + (0x800 * port_num));
   /* RW U1 p0 R2 h4074 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x4074);
   /* RW U1 p0 R3 h0002 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0002);
   /* RW U1 p0 R1 h3195 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3195 + (0x800 * port_num));
   /* RW U1 p0 R2 h59c4 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x59c4);
   /* RW U1 p0 R3 h0003 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0003);
   /* RW U1 p0 R1 h3196 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3196 + (0x800 * port_num));
   /* RW U1 p0 R2 h4063 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x4063);
   /* RW U1 p0 R3 h0002 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0002);
   /* RW U1 p0 R1 h3197 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3197 + (0x800 * port_num));
   /* RW U1 p0 R2 h59e9 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x59e9);
   /* RW U1 p0 R3 h0003 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0003);
   /* RW U1 p0 R1 h3198 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3198 + (0x800 * port_num));
   /* RW U1 p0 R2 h406e */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x406e);
   /* RW U1 p0 R3 h0002 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0002);
   /* RW U1 p0 R1 h3199 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3199 + (0x800 * port_num));
   /* RW U1 p0 R2 he004 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xe004);
   /* RW U1 p0 R3 h0000 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0000);
   /* RW U1 p0 R1 h319a */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x319a + (0x800 * port_num));
   /* RW U1 p0 R2 h0000 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x0000);
   /* RW U1 p0 R3 h0003 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0003);
   /* RW U1 p0 R1 h319b */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x319b + (0x800 * port_num));
   /* RW U1 p0 R2 h3121 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x3121);
   /* RW U1 p0 R3 h0003 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0003);
   /* RW U1 p0 R1 h319c */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x319c + (0x800 * port_num));
   /* RW U1 p0 R2 hd8e0 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xd8e0);
   /* RW U1 p0 R3 h0000 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0000);
   /* RW U1 p0 R1 h319d */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x319d + (0x800 * port_num));
   /* RW U1 p0 R2 hdca1 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xdca1);
   /* RW U1 p0 R3 h0002 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0002);
   /* RW U1 p0 R1 h319e */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x319e + (0x800 * port_num));
   /* RW U1 p0 R2 hfca1 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xfca1);
   /* RW U1 p0 R3 h0002 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0002);
   /* RW U1 p0 R1 h319f */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x319f + (0x800 * port_num));
   /* RW U1 p0 R2 he004 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xe004);
   /* RW U1 p0 R3 h0000 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0000);
   /* RW U1 p0 R1 h31a0 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x31a0 + (0x800 * port_num));
   /* RW U1 p0 R2 h0000 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x0000);
   /* RW U1 p0 R3 h0003 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0003);
   /* RW U1 p0 R1 h31a1 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x31a1 + (0x800 * port_num));
   /* RW U1 p0 R2 hd8e0 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xd8e0);
   /* RW U1 p0 R3 h0000 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0000);
   /* RW U1 p0 R1 h31a2 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x31a2 + (0x800 * port_num));
   /* RW U1 p0 R2 hd8e0 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xd8e0);
   /* RW U1 p0 R3 h0000 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0000);
   /* RW U1 p0 R1 h31a3 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x31a3 + (0x800 * port_num));
   /* RW U1 p0 R2 he063 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xe063);
   /* RW U1 p0 R3 h0000 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0000);
   /* RW U1 p0 R1 h31a4 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x31a4 + (0x800 * port_num));
   /* RW U1 p0 R2 he068 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xe068);
   /* RW U1 p0 R3 h0000 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0000);
   /* RW U1 p0 R1 h31a5 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x31a5 + (0x800 * port_num));
   /* RW U1 p0 R2 hd840 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xd840);
   /* RW U1 p0 R3 h0000 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0000);
   /* RW U1 p0 R1 h31a6 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x31a6 + (0x800 * port_num));
   /* RW U1 p0 R2 h09d8 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x09d8);
   /* RW U1 p0 R3 h0007 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0007);
   /* RW U1 p0 R1 h31a7 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x31a7 + (0x800 * port_num));
   /* RW U1 p0 R2 h3120 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x3120);
   /* RW U1 p0 R3 h0003 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0003);
   /* RW U1 p0 R1 h31a8 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x31a8 + (0x800 * port_num));
   /* RW U1 p0 R2 hf00c */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xf00c);
   /* RW U1 p0 R3 h0000 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0000);
   /* RW U1 p0 R1 h31a9 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x31a9 + (0x800 * port_num));
   /* RW U1 p0 R2 hd801 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xd801);
   /* RW U1 p0 R3 h0001 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0001);
   /* RW U1 p0 R1 h31aa */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x31aa + (0x800 * port_num));
   /* RW U1 p0 R2 hf024 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xf024);
   /* RW U1 p0 R3 h0000 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0000);
   /* RW U1 p0 R1 h31ab */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x31ab + (0x800 * port_num));
   /* RW U1 p0 R2 hf01c */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xf01c);
   /* RW U1 p0 R3 h0000 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0000);
   /* RW U1 p0 R1 h31ac */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x31ac + (0x800 * port_num));
   /* RW U1 p0 R2 hf044 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xf044);
   /* RW U1 p0 R3 h0000 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0000);
   /* RW U1 p0 R1 h31ad */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x31ad + (0x800 * port_num));
   /* RW U1 p0 R2 hf04c */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xf04c);
   /* RW U1 p0 R3 h0000 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0000);
   /* RW U1 p0 R1 h31ae */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x31ae + (0x800 * port_num));
   /* RW U1 p0 R2 hf054 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xf054);
   /* RW U1 p0 R3 h0000 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0000);
   /* RW U1 p0 R1 h31af */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x31af + (0x800 * port_num));
   /* RW U1 p0 R2 hf05c */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xf05c);
   /* RW U1 p0 R3 h0000 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0000);
   /* RW U1 p0 R1 h31b0 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x31b0 + (0x800 * port_num));
   /* RW U1 p0 R2 hd9e0 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xd9e0);
   /* RW U1 p0 R3 h0000 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0000);
   /* RW U1 p0 R1 h31b1 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x31b1 + (0x800 * port_num));
   /* RW U1 p0 R2 hd9a0 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xd9a0);
   /* RW U1 p0 R3 h0000 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0000);
   /* RW U1 p0 R1 h31b2 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x31b2 + (0x800 * port_num));
   /* RW U1 p0 R2 hd805 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xd805);
   /* RW U1 p0 R3 h0001 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0001);
   /* RW U1 p0 R1 h31b3 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x31b3 + (0x800 * port_num));
   /* RW U1 p0 R2 hf074 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xf074);
   /* RW U1 p0 R3 h0000 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0000);
   /* RW U1 p0 R1 h31b4 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x31b4 + (0x800 * port_num));
   /* RW U1 p0 R2 ha4b8 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xa4b8);
   /* RW U1 p0 R3 h0007 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0007);
   /* RW U1 p0 R1 h31b5 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x31b5 + (0x800 * port_num));
   /* RW U1 p0 R2 he037 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xe037);
   /* RW U1 p0 R3 h0000 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0000);
   /* RW U1 p0 R1 h31b6 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x31b6 + (0x800 * port_num));
   /* RW U1 p0 R2 he03e */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xe03e);
   /* RW U1 p0 R3 h0000 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0000);
   /* RW U1 p0 R1 h31b7 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x31b7 + (0x800 * port_num));
   /* RW U1 p0 R2 h00bf */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x00bf);
   /* RW U1 p0 R3 h0002 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0002);
   /* RW U1 p0 R1 h31b8 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x31b8 + (0x800 * port_num));
   /* RW U1 p0 R2 h7dbf */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x7dbf);
   /* RW U1 p0 R3 h0007 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0007);
   /* RW U1 p0 R1 h31b9 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x31b9 + (0x800 * port_num));
   /* RW U1 p0 R2 hd087 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xd087);
   /* RW U1 p0 R3 h0000 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0000);
   /* RW U1 p0 R1 h31ba */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x31ba + (0x800 * port_num));
   /* RW U1 p0 R2 he037 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xe037);
   /* RW U1 p0 R3 h0000 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0000);
   /* RW U1 p0 R1 h31bb */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x31bb + (0x800 * port_num));
   /* RW U1 p0 R2 he03e */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xe03e);
   /* RW U1 p0 R3 h0000 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0000);
   /* RW U1 p0 R1 h31bc */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x31bc + (0x800 * port_num));
   /* RW U1 p0 R2 h02bf */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x02bf);
   /* RW U1 p0 R3 h0007 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0007);
   /* RW U1 p0 R1 h31bd */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x31bd + (0x800 * port_num));
   /* RW U1 p0 R2 h1f27 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x1f27);
   /* RW U1 p0 R3 h0005 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0005);
   /* RW U1 p0 R1 h31be */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x31be + (0x800 * port_num));
   /* RW U1 p0 R2 h9b46 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x9b46);
   /* RW U1 p0 R3 h0005 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0005);
   /* RW U1 p0 R1 h31bf */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x31bf + (0x800 * port_num));
   /* RW U1 p0 R2 h0cd6 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x0cd6);
   /* RW U1 p0 R3 h0007 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0007);
   /* RW U1 p0 R1 h31c0 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x31c0 + (0x800 * port_num));
   /* RW U1 p0 R2 h58c0 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x58c0);
   /* RW U1 p0 R3 h0003 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0003);
   /* RW U1 p0 R1 h31c1 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x31c1 + (0x800 * port_num));
   /* RW U1 p0 R2 h40d6 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x40d6);
   /* RW U1 p0 R3 h0002 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0002);
   /* RW U1 p0 R1 h31c2 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x31c2 + (0x800 * port_num));
   /* RW U1 p0 R2 h2077 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x2077);
   /* RW U1 p0 R3 h0003 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0003);
   /* RW U1 p0 R1 h31c3 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x31c3 + (0x800 * port_num));
   /* RW U1 p0 R2 he029 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xe029);
   /* RW U1 p0 R3 h0000 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0000);
   /* RW U1 p0 R1 h31c4 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x31c4 + (0x800 * port_num));
   /* RW U1 p0 R2 hd042 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xd042);
   /* RW U1 p0 R3 h0000 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0000);
   /* RW U1 p0 R1 h31c5 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x31c5 + (0x800 * port_num));
   /* RW U1 p0 R2 h3068 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x3068);
   /* RW U1 p0 R3 h0003 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0003);
   /* RW U1 p0 R1 h31c6 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x31c6 + (0x800 * port_num));
   /* RW U1 p0 R2 he011 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xe011);
   /* RW U1 p0 R3 h0000 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0000);
   /* RW U1 p0 R1 h31c7 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x31c7 + (0x800 * port_num));
   /* RW U1 p0 R2 hc6c3 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xc6c3);
   /* RW U1 p0 R3 h0001 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0001);
   /* RW U1 p0 R1 h31c8 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x31c8 + (0x800 * port_num));
   /* RW U1 p0 R2 h24df */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x24df);
   /* RW U1 p0 R3 h0003 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0003);
   /* RW U1 p0 R1 h31c9 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x31c9 + (0x800 * port_num));
   /* RW U1 p0 R2 h24de */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x24de);
   /* RW U1 p0 R3 h0003 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0003);
   /* RW U1 p0 R1 h31ca */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x31ca + (0x800 * port_num));
   /* RW U1 p0 R2 he063 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xe063);
   /* RW U1 p0 R3 h0000 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0000);
   /* RW U1 p0 R1 h31cb */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x31cb + (0x800 * port_num));
   /* RW U1 p0 R2 hd801 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xd801);
   /* RW U1 p0 R3 h0001 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0001);
   /* RW U1 p0 R1 h31cc */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x31cc + (0x800 * port_num));
   /* RW U1 p0 R2 hd8a0 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xd8a0);
   /* RW U1 p0 R3 h0000 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0000);
   /* RW U1 p0 R1 h31cd */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x31cd + (0x800 * port_num));
   /* RW U1 p0 R2 hf07c */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xf07c);
   /* RW U1 p0 R3 h0000 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0000);
   /* RW U1 p0 R1 h31ce */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x31ce + (0x800 * port_num));
   /* RW U1 p0 R2 h228f */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x228f);
   /* RW U1 p0 R3 h0003 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0003);
   /* RW U1 p0 R1 h31cf */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x31cf + (0x800 * port_num));
   /* RW U1 p0 R2 hf07c */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xf07c);
   /* RW U1 p0 R3 h0000 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0000);
   /* RW U1 p0 R1 h31d0 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x31d0 + (0x800 * port_num));
   /* RW U1 p0 R2 h228f */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x228f);
   /* RW U1 p0 R3 h0003 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0003);
   /* RW U1 p0 R1 h31d1 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x31d1 + (0x800 * port_num));
   /* RW U1 p0 R2 hf07c */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xf07c);
   /* RW U1 p0 R3 h0000 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0000);
   /* RW U1 p0 R1 h31d2 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x31d2 + (0x800 * port_num));
   /* RW U1 p0 R2 h228f */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x228f);
   /* RW U1 p0 R3 h0003 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0003);
   /* RW U1 p0 R1 h31d3 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x31d3 + (0x800 * port_num));
   /* RW U1 p0 R2 hf078 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xf078);
   /* RW U1 p0 R3 h0000 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0000);
   /* RW U1 p0 R1 h31d4 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x31d4 + (0x800 * port_num));
   /* RW U1 p0 R2 h228f */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x228f);
   /* RW U1 p0 R3 h0003 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0003);
   /* RW U1 p0 R1 h31d5 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x31d5 + (0x800 * port_num));
   /* RW U1 p0 R2 he07a */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xe07a);
   /* RW U1 p0 R3 h0000 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0000);
   /* RW U1 p0 R1 h31d6 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x31d6 + (0x800 * port_num));
   /* RW U1 p0 R2 he004 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xe004);
   /* RW U1 p0 R3 h0000 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0000);
   /* RW U1 p0 R1 h31d7 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x31d7 + (0x800 * port_num));
   /* RW U1 p0 R2 h0000 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x0000);
   /* RW U1 p0 R3 h0003 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0003);
   /* RW U1 p0 R1 h31d8 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x31d8 + (0x800 * port_num));
   /* RW U1 p0 R2 h3128 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x3128);
   /* RW U1 p0 R3 h0003 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0003);
   /* RW U1 p0 R1 h31d9 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x31d9 + (0x800 * port_num));
   /* RW U1 p0 R2 hf02c */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xf02c);
   /* RW U1 p0 R3 h0000 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0000);
   /* RW U1 p0 R1 h31da */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x31da + (0x800 * port_num));
   /* RW U1 p0 R2 hf044 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xf044);
   /* RW U1 p0 R3 h0000 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0000);
   /* RW U1 p0 R1 h31db */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x31db + (0x800 * port_num));
   /* RW U1 p0 R2 hd808 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xd808);
   /* RW U1 p0 R3 h0001 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0001);
   /* RW U1 p0 R1 h31dc */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x31dc + (0x800 * port_num));
   /* RW U1 p0 R2 hf00c */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xf00c);
   /* RW U1 p0 R3 h0000 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0000);
   /* RW U1 p0 R1 h31dd */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x31dd + (0x800 * port_num));
   /* RW U1 p0 R2 hf04c */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xf04c);
   /* RW U1 p0 R3 h0000 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0000);
   /* RW U1 p0 R1 h31de */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x31de + (0x800 * port_num));
   /* RW U1 p0 R2 hf054 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xf054);
   /* RW U1 p0 R3 h0000 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0000);
   /* RW U1 p0 R1 h31df */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x31df + (0x800 * port_num));
   /* RW U1 p0 R2 hf05c */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xf05c);
   /* RW U1 p0 R3 h0000 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0000);
   /* RW U1 p0 R1 h31e0 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x31e0 + (0x800 * port_num));
   /* RW U1 p0 R2 hf024 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xf024);
   /* RW U1 p0 R3 h0000 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0000);
   /* RW U1 p0 R1 h31e1 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x31e1 + (0x800 * port_num));
   /* RW U1 p0 R2 hf074 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xf074);
   /* RW U1 p0 R3 h0000 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0000);
   /* RW U1 p0 R1 h31e2 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x31e2 + (0x800 * port_num));
   /* RW U1 p0 R2 hd802 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xd802);
   /* RW U1 p0 R3 h0001 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0001);
   /* RW U1 p0 R1 h31e3 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x31e3 + (0x800 * port_num));
   /* RW U1 p0 R2 hf018 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xf018);
   /* RW U1 p0 R3 h0000 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0000);
   /* RW U1 p0 R1 h31e4 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x31e4 + (0x800 * port_num));
   /* RW U1 p0 R2 he004 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xe004);
   /* RW U1 p0 R3 h0000 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0000);
   /* RW U1 p0 R1 h31e5 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x31e5 + (0x800 * port_num));
   /* RW U1 p0 R2 h0000 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x0000);
   /* RW U1 p0 R3 h0003 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0003);
   /* RW U1 p0 R1 h31e6 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x31e6 + (0x800 * port_num));
   /* RW U1 p0 R2 he004 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xe004);
   /* RW U1 p0 R3 h0000 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0000);
   /* RW U1 p0 R1 h31e7 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x31e7 + (0x800 * port_num));
   /* RW U1 p0 R2 h0000 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x0000);
   /* RW U1 p0 R3 h0003 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0003);
   /* RW U1 p0 R1 h31e8 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x31e8 + (0x800 * port_num));
   /* RW U1 p0 R2 h312f */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x312f);
   /* RW U1 p0 R3 h0003 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0003);
   /* RW U1 p0 R1 h31e9 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x31e9 + (0x800 * port_num));
   /* RW U1 p0 R2 he004 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xe004);
   /* RW U1 p0 R3 h0000 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0000);
   /* RW U1 p0 R1 h31ea */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x31ea + (0x800 * port_num));
   /* RW U1 p0 R2 h0000 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x0000);
   /* RW U1 p0 R3 h0003 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0003);
   /* RW U1 p0 R1 h31eb */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x31eb + (0x800 * port_num));
   /* RW U1 p0 R2 h0000 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x0000);
   /* RW U1 p0 R3 h0000 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0000);
   /* RW U1 p0 R1 h31ec */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x31ec + (0x800 * port_num));
   /* RW U1 p0 R2 h0000 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x0000);
   /* RW U1 p0 R3 h0000 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0000);
   /* RW U1 p0 R1 h31ed */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x31ed + (0x800 * port_num));
   /* RW U1 p0 R2 h0000 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x0000);
   /* RW U1 p0 R3 h0000 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0000);
   /* RW U1 p0 R1 h31ee */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x31ee + (0x800 * port_num));
   /* RW U1 p0 R2 h0000 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x0000);
   /* RW U1 p0 R3 h0000 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0000);
   /* RW U1 p0 R1 h31ef */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x31ef + (0x800 * port_num));
   /* RW U1 p0 R2 h0000 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x0000);
   /* RW U1 p0 R3 h0000 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0000);
   /* RW U1 p0 R1 h31f0 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x31f0 + (0x800 * port_num));
   /* RW U1 p0 R2 h0000 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x0000);
   /* RW U1 p0 R3 h0000 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0000);
   /* RW U1 p0 R1 h31f1 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x31f1 + (0x800 * port_num));
   /* RW U1 p0 R2 h0000 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x0000);
   /* RW U1 p0 R3 h0000 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0000);
   /* RW U1 p0 R1 h31f2 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x31f2 + (0x800 * port_num));
   /* RW U1 p0 R2 h0000 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x0000);
   /* RW U1 p0 R3 h0000 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0000);
   /* RW U1 p0 R1 h31f3 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x31f3 + (0x800 * port_num));
   /* RW U1 p0 R2 h0000 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x0000);
   /* RW U1 p0 R3 h0000 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0000);
   /* RW U1 p0 R1 h31f4 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x31f4 + (0x800 * port_num));
   /* RW U1 p0 R2 h0000 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x0000);
   /* RW U1 p0 R3 h0000 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0000);
   /* RW U1 p0 R1 h31f5 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x31f5 + (0x800 * port_num));
   /* RW U1 p0 R2 h0000 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x0000);
   /* RW U1 p0 R3 h0000 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0000);
   /* RW U1 p0 R1 h31f6 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x31f6 + (0x800 * port_num));
   /* RW U1 p0 R2 h0000 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x0000);
   /* RW U1 p0 R3 h0000 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0000);
   /* RW U1 p0 R1 h31f7 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x31f7 + (0x800 * port_num));
   /* RW U1 p0 R2 h0000 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x0000);
   /* RW U1 p0 R3 h0000 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0000);
   /* RW U1 p0 R1 h31f8 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x31f8 + (0x800 * port_num));
   /* RW U1 p0 R2 h0000 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x0000);
   /* RW U1 p0 R3 h0000 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0000);
   /* RW U1 p0 R1 h31f9 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x31f9 + (0x800 * port_num));
   /* RW U1 p0 R2 h0000 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x0000);
   /* RW U1 p0 R3 h0000 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0000);
   /* RW U1 p0 R1 h31fa */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x31fa + (0x800 * port_num));
   /* RW U1 p0 R2 h0000 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x0000);
   /* RW U1 p0 R3 h0000 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0000);
   /* RW U1 p0 R1 h31fb */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x31fb + (0x800 * port_num));
   /* RW U1 p0 R2 h0000 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x0000);
   /* RW U1 p0 R3 h0000 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0000);
   /* RW U1 p0 R1 h31fc */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x31fc + (0x800 * port_num));
   /* RW U1 p0 R2 h0000 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x0000);
   /* RW U1 p0 R3 h0000 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0000);
   /* RW U1 p0 R1 h31fd */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x31fd + (0x800 * port_num));
   /* RW U1 p0 R2 h0000 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x0000);
   /* RW U1 p0 R3 h0000 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0000);
   /* RW U1 p0 R1 h31fe */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x31fe + (0x800 * port_num));
   /* RW U1 p0 R2 h0000 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x0000);
   /* RW U1 p0 R3 h0000 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0000);
   /* RW U1 p0 R1 h31ff */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x31ff + (0x800 * port_num));
   /* RW U1 p0 R2 h0002 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x0002);
   /* RW U1 p0 R3 h0000 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0000);

   /* RW U1 p0 R1 h3500 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3500 + (0x800 * port_num));
   /* RW U1 p0 R2 h00e9 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x00e9);
   /* RW U1 p0 R3 h0002 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0002);
   /* RW U1 p0 R1 h3501 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3501 + (0x800 * port_num));
   /* RW U1 p0 R2 hd051 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xd051);
   /* RW U1 p0 R3 h0000 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0000);
   /* RW U1 p0 R1 h3502 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3502 + (0x800 * port_num));
   /* RW U1 p0 R2 hde48 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xde48);
   /* RW U1 p0 R3 h0001 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0001);
   /* RW U1 p0 R1 h3503 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3503 + (0x800 * port_num));
   /* RW U1 p0 R2 hc001 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xc001);
   /* RW U1 p0 R3 h0000 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0000);
   /* RW U1 p0 R1 h3504 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3504 + (0x800 * port_num));
   /* RW U1 p0 R2 hd846 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xd846);
   /* RW U1 p0 R3 h0001 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0001);
   /* RW U1 p0 R1 h3505 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3505 + (0x800 * port_num));
   /* RW U1 p0 R2 he079 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xe079);
   /* RW U1 p0 R3 h0000 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0000);
   /* RW U1 p0 R1 h3506 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3506 + (0x800 * port_num));
   /* RW U1 p0 R2 hd806 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xd806);
   /* RW U1 p0 R3 h0001 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0001);
   /* RW U1 p0 R1 h3507 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3507 + (0x800 * port_num));
   /* RW U1 p0 R2 h843f */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x843f);
   /* RW U1 p0 R3 h0002 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0002);
   /* RW U1 p0 R1 h3508 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3508 + (0x800 * port_num));
   /* RW U1 p0 R2 hcc3c */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xcc3c);
   /* RW U1 p0 R3 h0002 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0002);
   /* RW U1 p0 R1 h3509 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3509 + (0x800 * port_num));
   /* RW U1 p0 R2 hac2a */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xac2a);
   /* RW U1 p0 R3 h0002 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0002);
   /* RW U1 p0 R1 h350a */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x350a + (0x800 * port_num));
   /* RW U1 p0 R2 h9c65 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x9c65);
   /* RW U1 p0 R3 h0002 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0002);
   /* RW U1 p0 R1 h350b */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x350b + (0x800 * port_num));
   /* RW U1 p0 R2 ha465 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xa465);
   /* RW U1 p0 R3 h0002 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0002);
   /* RW U1 p0 R1 h350c */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x350c + (0x800 * port_num));
   /* RW U1 p0 R2 hc41c */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xc41c);
   /* RW U1 p0 R3 h0002 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0002);
   /* RW U1 p0 R1 h350d */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x350d + (0x800 * port_num));
   /* RW U1 p0 R2 hb489 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xb489);
   /* RW U1 p0 R3 h0002 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0002);
   /* RW U1 p0 R1 h350e */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x350e + (0x800 * port_num));
   /* RW U1 p0 R2 hbc94 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xbc94);
   /* RW U1 p0 R3 h0002 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0002);
   /* RW U1 p0 R1 h350f */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x350f + (0x800 * port_num));
   /* RW U1 p0 R2 h94c1 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x94c1);
   /* RW U1 p0 R3 h0002 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0002);
   /* RW U1 p0 R1 h3510 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3510 + (0x800 * port_num));
   /* RW U1 p0 R2 h8c51 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x8c51);
   /* RW U1 p0 R3 h0002 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0002);
   /* RW U1 p0 R1 h3511 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3511 + (0x800 * port_num));
   /* RW U1 p0 R2 hf417 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xf417);
   /* RW U1 p0 R3 h0002 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0002);
   /* RW U1 p0 R1 h3512 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3512 + (0x800 * port_num));
   /* RW U1 p0 R2 hd840 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xd840);
   /* RW U1 p0 R3 h0000 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0000);
   /* RW U1 p0 R1 h3513 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3513 + (0x800 * port_num));
   /* RW U1 p0 R2 h5452 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x5452);
   /* RW U1 p0 R3 h0003 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0003);
   /* RW U1 p0 R1 h3514 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3514 + (0x800 * port_num));
   /* RW U1 p0 R2 h3850 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x3850);
   /* RW U1 p0 R3 h0002 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0002);
   /* RW U1 p0 R1 h3515 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3515 + (0x800 * port_num));
   /* RW U1 p0 R2 he004 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xe004);
   /* RW U1 p0 R3 h0000 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0000);
   /* RW U1 p0 R1 h3516 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3516 + (0x800 * port_num));
   /* RW U1 p0 R2 h0000 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x0000);
   /* RW U1 p0 R3 h0003 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0003);
   /* RW U1 p0 R1 h3517 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3517 + (0x800 * port_num));
   /* RW U1 p0 R2 h3126 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x3126);
   /* RW U1 p0 R3 h0003 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0003);
   /* RW U1 p0 R1 h3518 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3518 + (0x800 * port_num));
   /* RW U1 p0 R2 he068 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xe068);
   /* RW U1 p0 R3 h0000 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0000);
   /* RW U1 p0 R1 h3519 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3519 + (0x800 * port_num));
   /* RW U1 p0 R2 h7008 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x7008);
   /* RW U1 p0 R3 h0000 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0000);
   /* RW U1 p0 R1 h351a */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x351a + (0x800 * port_num));
   /* RW U1 p0 R2 he004 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xe004);
   /* RW U1 p0 R3 h0000 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0000);
   /* RW U1 p0 R1 h351b */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x351b + (0x800 * port_num));
   /* RW U1 p0 R2 h0000 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x0000);
   /* RW U1 p0 R3 h0003 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0003);
   /* RW U1 p0 R1 h351c */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x351c + (0x800 * port_num));
   /* RW U1 p0 R2 hcc29 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xcc29);
   /* RW U1 p0 R3 h0007 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0007);
   /* RW U1 p0 R1 h351d */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x351d + (0x800 * port_num));
   /* RW U1 p0 R2 hd0d1 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xd0d1);
   /* RW U1 p0 R3 h0000 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0000);
   /* RW U1 p0 R1 h351e */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x351e + (0x800 * port_num));
   /* RW U1 p0 R2 h8328 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x8328);
   /* RW U1 p0 R3 h0007 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0007);
   /* RW U1 p0 R1 h351f */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x351f + (0x800 * port_num));
   /* RW U1 p0 R2 hf428 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xf428);
   /* RW U1 p0 R3 h0007 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0007);
   /* RW U1 p0 R1 h3520 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3520 + (0x800 * port_num));
   /* RW U1 p0 R2 h312e */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x312e);
   /* RW U1 p0 R3 h0003 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0003);
   /* RW U1 p0 R1 h3521 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3521 + (0x800 * port_num));
   /* RW U1 p0 R2 hd8e0 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xd8e0);
   /* RW U1 p0 R3 h0000 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0000);
   /* RW U1 p0 R1 h3522 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3522 + (0x800 * port_num));
   /* RW U1 p0 R2 hd802 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xd802);
   /* RW U1 p0 R3 h0001 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0001);
   /* RW U1 p0 R1 h3523 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3523 + (0x800 * port_num));
   /* RW U1 p0 R2 he078 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xe078);
   /* RW U1 p0 R3 h0000 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0000);
   /* RW U1 p0 R1 h3524 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3524 + (0x800 * port_num));
   /* RW U1 p0 R2 h6b07 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x6b07);
   /* RW U1 p0 R3 h0007 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x00027);
   /* RW U1 p0 R1 h3525 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3525 + (0x800 * port_num));
   /* RW U1 p0 R2 hd804 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xd804);
   /* RW U1 p0 R3 h0001 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0001);
   /* RW U1 p0 R1 h3526 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3526 + (0x800 * port_num));
   /* RW U1 p0 R2 he078 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xe078);
   /* RW U1 p0 R3 h0000 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0000);
   /* RW U1 p0 R1 h3527 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3527 + (0x800 * port_num));
   /* RW U1 p0 R2 h0007 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x0007);
   /* RW U1 p0 R3 h0002 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0002);
   /* RW U1 p0 R1 h3528 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3528 + (0x800 * port_num));
   /* RW U1 p0 R2 h312e */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x312e);
   /* RW U1 p0 R3 h0003 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0003);
   /* RW U1 p0 R1 h3529 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3529 + (0x800 * port_num));
   /* RW U1 p0 R2 h00e7 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x00e7);
   /* RW U1 p0 R3 h0002 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0002);
   /* RW U1 p0 R1 h352a */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x352a + (0x800 * port_num));
   /* RW U1 p0 R2 h312b */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x312b);
   /* RW U1 p0 R3 h0003 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0003);
   /* RW U1 p0 R1 h352b */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x352b + (0x800 * port_num));
   /* RW U1 p0 R2 hd802 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xd802);
   /* RW U1 p0 R3 h0001 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0001);
   /* RW U1 p0 R1 h352c */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x352c + (0x800 * port_num));
   /* RW U1 p0 R2 hd806 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xd806);
   /* RW U1 p0 R3 h0001 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0001);
   /* RW U1 p0 R1 h352d */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x352d + (0x800 * port_num));
   /* RW U1 p0 R2 h843f */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x843f);
   /* RW U1 p0 R3 h0002 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0002);
   /* RW U1 p0 R1 h352e */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x352e + (0x800 * port_num));
   /* RW U1 p0 R2 hcc3c */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xcc3c);
   /* RW U1 p0 R3 h0002 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0002);
   /* RW U1 p0 R1 h352f */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x352f + (0x800 * port_num));
   /* RW U1 p0 R2 hac2a */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xac2a);
   /* RW U1 p0 R3 h0002 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0002);
   /* RW U1 p0 R1 h3530 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3530 + (0x800 * port_num));
   /* RW U1 p0 R2 h9c65 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x9c65);
   /* RW U1 p0 R3 h0002 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0002);
   /* RW U1 p0 R1 h3531 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3531 + (0x800 * port_num));
   /* RW U1 p0 R2 ha465 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xa465);
   /* RW U1 p0 R3 h0002 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0002);
   /* RW U1 p0 R1 h3532 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3532 + (0x800 * port_num));
   /* RW U1 p0 R2 hc41c */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xc41c);
   /* RW U1 p0 R3 h0002 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0002);
   /* RW U1 p0 R1 h3533 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3533 + (0x800 * port_num));
   /* RW U1 p0 R2 hb489 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xb489);
   /* RW U1 p0 R3 h0002 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0002);
   /* RW U1 p0 R1 h3534 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3534 + (0x800 * port_num));
   /* RW U1 p0 R2 hbc94 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xbc94);
   /* RW U1 p0 R3 h0002 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0002);
   /* RW U1 p0 R1 h3535 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3535 + (0x800 * port_num));
   /* RW U1 p0 R2 h94c1 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x94c1);
   /* RW U1 p0 R3 h0002 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0002);
   /* RW U1 p0 R1 h3556 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3556 + (0x800 * port_num));
   /* RW U1 p0 R2 h8c51 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x8c51);
   /* RW U1 p0 R3 h0002 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0002);
   /* RW U1 p0 R1 h3537 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3537 + (0x800 * port_num));
   /* RW U1 p0 R2 hd840 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xd840);
   /* RW U1 p0 R3 h0000 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0000);
   /* RW U1 p0 R1 h3538 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3538 + (0x800 * port_num));
   /* RW U1 p0 R2 h5452 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x5452);
   /* RW U1 p0 R3 h0003 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0003);
   /* RW U1 p0 R1 h3539 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3539 + (0x800 * port_num));
   /* RW U1 p0 R2 h3850 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x3850);
   /* RW U1 p0 R3 h0002 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0002);
   /* RW U1 p0 R1 h353a */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x353a + (0x800 * port_num));
   /* RW U1 p0 R2 he004 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xe004);
   /* RW U1 p0 R3 h0000 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0000);
   /* RW U1 p0 R1 h353b */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x353b + (0x800 * port_num));
   /* RW U1 p0 R2 h0000 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x0000);
   /* RW U1 p0 R3 h0003 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0003);
   /* RW U1 p0 R1 h353c */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x353c + (0x800 * port_num));
   /* RW U1 p0 R2 h312a */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x312a);
   /* RW U1 p0 R3 h0003 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0003);
   /* RW U1 p0 R1 h353d */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x353d + (0x800 * port_num));
   /* RW U1 p0 R2 hd801 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xd801);
   /* RW U1 p0 R3 h0001 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0001);
   /* RW U1 p0 R1 h353e */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x353e + (0x800 * port_num));
   /* RW U1 p0 R2 h0007 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x0007);
   /* RW U1 p0 R3 h0002 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0002);
   /* RW U1 p0 R1 h353f */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x353f + (0x800 * port_num));
   /* RW U1 p0 R2 h3129 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x3129);
   /* RW U1 p0 R3 h0003 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0003);
   /* RW U1 p0 R1 h3540 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3540 + (0x800 * port_num));
   /* RW U1 p0 R2 hd801 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xd801);
   /* RW U1 p0 R3 h0001 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0001);
   /* RW U1 p0 R1 h3541 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3541 + (0x800 * port_num));
   /* RW U1 p0 R2 h843f */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x843f);
   /* RW U1 p0 R3 h0002 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0002);
   /* RW U1 p0 R1 h3542 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3542 + (0x800 * port_num));
   /* RW U1 p0 R2 hcc3c */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xcc3c);
   /* RW U1 p0 R3 h0002 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0002);
   /* RW U1 p0 R1 h3543 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3543 + (0x800 * port_num));
   /* RW U1 p0 R2 hac2a */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xac2a);
   /* RW U1 p0 R3 h0002 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0002);
   /* RW U1 p0 R1 h3544 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3544 + (0x800 * port_num));
   /* RW U1 p0 R2 h9c65 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x9c65);
   /* RW U1 p0 R3 h0002 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0002);
   /* RW U1 p0 R1 h3545 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3545 + (0x800 * port_num));
   /* RW U1 p0 R2 ha465 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xa465);
   /* RW U1 p0 R3 h0002 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0002);
   /* RW U1 p0 R1 h3546 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3546 + (0x800 * port_num));
   /* RW U1 p0 R2 hc41c */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xc41c);
   /* RW U1 p0 R3 h0002 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0002);
   /* RW U1 p0 R1 h3547 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3547 + (0x800 * port_num));
   /* RW U1 p0 R2 hb489 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xb489);
   /* RW U1 p0 R3 h0002 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0002);
   /* RW U1 p0 R1 h3548 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3548 + (0x800 * port_num));
   /* RW U1 p0 R2 hbc94 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xbc94);
   /* RW U1 p0 R3 h0002 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0002);
   /* RW U1 p0 R1 h3549 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3549 + (0x800 * port_num));
   /* RW U1 p0 R2 h94c1 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x94c1);
   /* RW U1 p0 R3 h0002 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0002);
   /* RW U1 p0 R1 h354a */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x354a + (0x800 * port_num));
   /* RW U1 p0 R2 h8c51 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x8c51);
   /* RW U1 p0 R3 h0002 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0002);
   /* RW U1 p0 R1 h354b */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x354b + (0x800 * port_num));
   /* RW U1 p0 R2 hd840 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xd840);
   /* RW U1 p0 R3 h0000 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0000);
   /* RW U1 p0 R1 h354c */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x354c + (0x800 * port_num));
   /* RW U1 p0 R2 h5452 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x5452);
   /* RW U1 p0 R3 h0003 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0003);
   /* RW U1 p0 R1 h354d */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x354d + (0x800 * port_num));
   /* RW U1 p0 R2 h3850 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x3850);
   /* RW U1 p0 R3 h0002 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0002);
   /* RW U1 p0 R1 h354e */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x354e + (0x800 * port_num));
   /* RW U1 p0 R2 he004 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xe004);
   /* RW U1 p0 R3 h0000 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0000);
   /* RW U1 p0 R1 h354f */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x354f + (0x800 * port_num));
   /* RW U1 p0 R2 h0000 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x0000);
   /* RW U1 p0 R3 h0003 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0003);
   /* RW U1 p0 R1 h3550 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3550 + (0x800 * port_num));
   /* RW U1 p0 R2 hd801 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xd801);
   /* RW U1 p0 R3 h0001 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0001);
   /* RW U1 p0 R1 h3551 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3551 + (0x800 * port_num));
   /* RW U1 p0 R2 hd8a0 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xd8a0);
   /* RW U1 p0 R3 h0000 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0000);
   /* RW U1 p0 R1 h3552 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3552 + (0x800 * port_num));
   /* RW U1 p0 R2 h5496 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x5496);
   /* RW U1 p0 R3 h0003 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0003);
   /* RW U1 p0 R1 h3553 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3553 + (0x800 * port_num));
   /* RW U1 p0 R2 h48e7 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x48e7);
   /* RW U1 p0 R3 h0002 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0002);
   /* RW U1 p0 R1 h3554 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3554 + (0x800 * port_num));
   /* RW U1 p0 R2 hd8a0 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xd8a0);
   /* RW U1 p0 R3 h0000 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0000);
   /* RW U1 p0 R1 h3555 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3555 + (0x800 * port_num));
   /* RW U1 p0 R2 he098 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xe098);
   /* RW U1 p0 R3 h0001 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0001);
   /* RW U1 p0 R1 h3556 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3556 + (0x800 * port_num));
   /* RW U1 p0 R2 h5497 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x5497);
   /* RW U1 p0 R3 h0003 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0003);
   /* RW U1 p0 R1 h3557 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3557 + (0x800 * port_num));
   /* RW U1 p0 R2 h48e7 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x48e7);
   /* RW U1 p0 R3 h0002 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0002);
   /* RW U1 p0 R1 h3558 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3558 + (0x800 * port_num));
   /* RW U1 p0 R2 h312c */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x312c);
   /* RW U1 p0 R3 h0003 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0003);
   /* RW U1 p0 R1 h3559 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3559 + (0x800 * port_num));
   /* RW U1 p0 R2 hd8a0 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xd8a0);
   /* RW U1 p0 R3 h0000 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0000);
   /* RW U1 p0 R1 h355a */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x355a + (0x800 * port_num));
   /* RW U1 p0 R2 h843f */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x843f);
   /* RW U1 p0 R3 h0002 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0002);
   /* RW U1 p0 R1 h355b */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x355b + (0x800 * port_num));
   /* RW U1 p0 R2 hcc3c */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xcc3c);
   /* RW U1 p0 R3 h0002 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0002);
   /* RW U1 p0 R1 h355c */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x355c + (0x800 * port_num));
   /* RW U1 p0 R2 hac2a */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xac2a);
   /* RW U1 p0 R3 h0002 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0002);
   /* RW U1 p0 R1 h355d */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x355d + (0x800 * port_num));
   /* RW U1 p0 R2 h9c65 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x9c65);
   /* RW U1 p0 R3 h0002 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0002);
   /* RW U1 p0 R1 h355e */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x355e + (0x800 * port_num));
   /* RW U1 p0 R2 ha465 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xa465);
   /* RW U1 p0 R3 h0002 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0002);
   /* RW U1 p0 R1 h355f */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x355f + (0x800 * port_num));
   /* RW U1 p0 R2 hc41c */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xc41c);
   /* RW U1 p0 R3 h0002 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0002);
   /* RW U1 p0 R1 h3560 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3560 + (0x800 * port_num));
   /* RW U1 p0 R2 hb489 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xb489);
   /* RW U1 p0 R3 h0002 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0002);
   /* RW U1 p0 R1 h3561 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3561 + (0x800 * port_num));
   /* RW U1 p0 R2 hbc94 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xbc94);
   /* RW U1 p0 R3 h0002 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0002);
   /* RW U1 p0 R1 h3562 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3562 + (0x800 * port_num));
   /* RW U1 p0 R2 h94c1 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x94c1);
   /* RW U1 p0 R3 h0002 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0002);
   /* RW U1 p0 R1 h3563 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3563 + (0x800 * port_num));
   /* RW U1 p0 R2 he004 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xe004);
   /* RW U1 p0 R3 h0000 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0000);
   /* RW U1 p0 R1 h3564 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3564 + (0x800 * port_num));
   /* RW U1 p0 R2 h0000 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x0000);
   /* RW U1 p0 R3 h0003 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0003);
   /* RW U1 p0 R1 h3565 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3565 + (0x800 * port_num));
   /* RW U1 p0 R2 h312d */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x312d);
   /* RW U1 p0 R3 h0003 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0003);
   /* RW U1 p0 R1 h3566 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3566 + (0x800 * port_num));
   /* RW U1 p0 R2 hd860 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xd860);
   /* RW U1 p0 R3 h0000 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0000);
   /* RW U1 p0 R1 h3567 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3567 + (0x800 * port_num));
   /* RW U1 p0 R2 hd8a0 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xd8a0);
   /* RW U1 p0 R3 h0000 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0000);
   /* RW U1 p0 R1 h3568 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3568 + (0x800 * port_num));
   /* RW U1 p0 R2 hc277 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xc277);
   /* RW U1 p0 R3 h0007 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0007);
   /* RW U1 p0 R1 h3569 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3569 + (0x800 * port_num));
   /* RW U1 p0 R2 hd8e0 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xd8e0);
   /* RW U1 p0 R3 h0000 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0000);
   /* RW U1 p0 R1 h356a */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x356a + (0x800 * port_num));
   /* RW U1 p0 R2 hd8e0 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xd8e0);
   /* RW U1 p0 R3 h0000 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0000);
   /* RW U1 p0 R1 h356b */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x356b + (0x800 * port_num));
   /* RW U1 p0 R2 hc377 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xc377);
   /* RW U1 p0 R3 h0007 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0007);
   /* RW U1 p0 R1 h356c */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x356c + (0x800 * port_num));
   /* RW U1 p0 R2 hd8e0 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xd8e0);
   /* RW U1 p0 R3 h0000 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0000);
   /* RW U1 p0 R1 h356d */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x356d + (0x800 * port_num));
   /* RW U1 p0 R2 hd8e0 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xd8e0);
   /* RW U1 p0 R3 h0000 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0000);
   /* RW U1 p0 R1 h356e */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x356e + (0x800 * port_num));
   /* RW U1 p0 R2 hc377 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xc377);
   /* RW U1 p0 R3 h0007 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0007);
   /* RW U1 p0 R1 h356f */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x356f + (0x800 * port_num));
   /* RW U1 p0 R2 hd8e0 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xd8e0);
   /* RW U1 p0 R3 h0000 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0000);
   /* RW U1 p0 R1 h3570 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3570 + (0x800 * port_num));
   /* RW U1 p0 R2 hd8e0 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xd8e0);
   /* RW U1 p0 R3 h0000 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0000);
   /* RW U1 p0 R1 h3571 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3571 + (0x800 * port_num));
   /* RW U1 p0 R2 hc377 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xc377);
   /* RW U1 p0 R3 h0007 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0007);
   /* RW U1 p0 R1 h3572 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3572 + (0x800 * port_num));
   /* RW U1 p0 R2 hd8e0 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xd8e0);
   /* RW U1 p0 R3 h0000 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0000);
   /* RW U1 p0 R1 h3573 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3573 + (0x800 * port_num));
   /* RW U1 p0 R2 hd8e0 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xd8e0);
   /* RW U1 p0 R3 h0000 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0000);
   /* RW U1 p0 R1 h3574 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3574 + (0x800 * port_num));
   /* RW U1 p0 R2 hc377 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xc377);
   /* RW U1 p0 R3 h0007 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0007);
   /* RW U1 p0 R1 h3575 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3575 + (0x800 * port_num));
   /* RW U1 p0 R2 he004 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xe004);
   /* RW U1 p0 R3 h0000 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0000);
   /* RW U1 p0 R1 h3576 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3576 + (0x800 * port_num));
   /* RW U1 p0 R2 h0000 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x0000);
   /* RW U1 p0 R3 h0003 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0003);
   /* RW U1 p0 R1 h3577 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3577 + (0x800 * port_num));
   /* RW U1 p0 R2 he453 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xe453);
   /* RW U1 p0 R3 h0001 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0001);
   /* RW U1 p0 R1 h3578 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3578 + (0x800 * port_num));
   /* RW U1 p0 R2 h1083 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x1083);
   /* RW U1 p0 R3 h0002 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0002);
   /* RW U1 p0 R1 h3579 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3579 + (0x800 * port_num));
   /* RW U1 p0 R2 hd840 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xd840);
   /* RW U1 p0 R3 h0000 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0000);
   /* RW U1 p0 R1 h357a */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x357a + (0x800 * port_num));
   /* RW U1 p0 R2 hde4c */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xde4c);
   /* RW U1 p0 R3 h0001 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0001);
   /* RW U1 p0 R1 h357b */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x357b + (0x800 * port_num));
   /* RW U1 p0 R2 h5844 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x5844);
   /* RW U1 p0 R3 h0003 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0003);
   /* RW U1 p0 R1 h357c */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x357c + (0x800 * port_num));
   /* RW U1 p0 R2 h4089 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x4089);
   /* RW U1 p0 R3 h0002 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0002);
   /* RW U1 p0 R1 h357d */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x357d + (0x800 * port_num));
   /* RW U1 p0 R2 h5846 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x5846);
   /* RW U1 p0 R3 h0003 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0003);
   /* RW U1 p0 R1 h357e */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x357e + (0x800 * port_num));
   /* RW U1 p0 R2 h4094 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x4094);
   /* RW U1 p0 R3 h0002 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0002);
   /* RW U1 p0 R1 h357f */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x357f + (0x800 * port_num));
   /* RW U1 p0 R2 h5840 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x5840);
   /* RW U1 p0 R3 h0003 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0003);
   /* RW U1 p0 R1 h3580 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3580 + (0x800 * port_num));
   /* RW U1 p0 R2 h4087 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x4087);
   /* RW U1 p0 R3 h0002 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0002);
   /* RW U1 p0 R1 h3581 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3581 + (0x800 * port_num));
   /* RW U1 p0 R2 he004 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xe004);
   /* RW U1 p0 R3 h0000 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0000);
   /* RW U1 p0 R1 h3582 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3582 + (0x800 * port_num));
   /* RW U1 p0 R2 h0000 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x0000);
   /* RW U1 p0 R3 h0003 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0003);
   /* RW U1 p0 R1 h3583 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3583 + (0x800 * port_num));
   /* RW U1 p0 R2 he095 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xe095);
   /* RW U1 p0 R3 h0001 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0001);
   /* RW U1 p0 R1 h3584 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3584 + (0x800 * port_num));
   /* RW U1 p0 R2 he494 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xe494);
   /* RW U1 p0 R3 h0001 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0001);
   /* RW U1 p0 R1 h3585 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3585 + (0x800 * port_num));
   /* RW U1 p0 R2 h1006 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x1006);
   /* RW U1 p0 R3 h0002 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0002);
   /* RW U1 p0 R1 h3586 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3586 + (0x800 * port_num));
   /* RW U1 p0 R2 h0079 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x0079);
   /* RW U1 p0 R3 h0002 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0002);
   /* RW U1 p0 R1 h3587 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3587 + (0x800 * port_num));
   /* RW U1 p0 R2 hd802 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xd802);
   /* RW U1 p0 R3 h0001 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0001);
   /* RW U1 p0 R1 h3588 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3588 + (0x800 * port_num));
   /* RW U1 p0 R2 h0006 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x0006);
   /* RW U1 p0 R3 h0002 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0002);
   /* RW U1 p0 R1 h3589 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3589 + (0x800 * port_num));
   /* RW U1 p0 R2 h3122 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x3122);
   /* RW U1 p0 R3 h0003 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0003);
   /* RW U1 p0 R1 h358a */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x358a + (0x800 * port_num));
   /* RW U1 p0 R2 hd840 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xd840);
   /* RW U1 p0 R3 h0000 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0000);
   /* RW U1 p0 R1 h358b */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x358b + (0x800 * port_num));
   /* RW U1 p0 R2 hd804 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xd804);
   /* RW U1 p0 R3 h0001 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0001);
   /* RW U1 p0 R1 h358c */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x358c + (0x800 * port_num));
   /* RW U1 p0 R2 he050 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xe050);
   /* RW U1 p0 R3 h0001 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0001);
   /* RW U1 p0 R1 h358d */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x358d + (0x800 * port_num));
   /* RW U1 p0 R2 hde47 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xde47);
   /* RW U1 p0 R3 h0001 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0001);
   /* RW U1 p0 R1 h358e */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x358e + (0x800 * port_num));
   /* RW U1 p0 R2 h584a */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x584a);
   /* RW U1 p0 R3 h0003 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0003);
   /* RW U1 p0 R1 h358f */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x358f + (0x800 * port_num));
   /* RW U1 p0 R2 h30e7 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x30e7);
   /* RW U1 p0 R3 h0002 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0002);
   /* RW U1 p0 R1 h3590 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3590 + (0x800 * port_num));
   /* RW U1 p0 R2 hda44 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xda44);
   /* RW U1 p0 R3 h0001 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0001);
   /* RW U1 p0 R1 h3591 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3591 + (0x800 * port_num));
   /* RW U1 p0 R2 h59c0 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x59c0);
   /* RW U1 p0 R3 h0000 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0000);
   /* RW U1 p0 R1 h3592 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3592 + (0x800 * port_num));
   /* RW U1 p0 R2 hc801 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xc801);
   /* RW U1 p0 R3 h0001 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0001);
   /* RW U1 p0 R1 h3593 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3593 + (0x800 * port_num));
   /* RW U1 p0 R2 h00ae */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x00ae);
   /* RW U1 p0 R3 h0002 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0002);
   /* RW U1 p0 R1 h3594 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3594 + (0x800 * port_num));
   /* RW U1 p0 R2 h3123 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x3123);
   /* RW U1 p0 R3 h0003 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0003);
   /* RW U1 p0 R1 h3595 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3595 + (0x800 * port_num));
   /* RW U1 p0 R2 hd803 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xd803);
   /* RW U1 p0 R3 h0001 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0001);
   /* RW U1 p0 R1 h3596 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3596 + (0x800 * port_num));
   /* RW U1 p0 R2 hd9c0 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xd9c0);
   /* RW U1 p0 R3 h0000 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0000);
   /* RW U1 p0 R1 h3597 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3597 + (0x800 * port_num));
   /* RW U1 p0 R2 hd811 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xd811);
   /* RW U1 p0 R3 h0001 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0001);
   /* RW U1 p0 R1 h3598 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3598 + (0x800 * port_num));
   /* RW U1 p0 R2 hdfc8 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xdfc8);
   /* RW U1 p0 R3 h0001 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0001);
   /* RW U1 p0 R1 h3599 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3599 + (0x800 * port_num));
   /* RW U1 p0 R2 h00ae */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x00ae);
   /* RW U1 p0 R3 h0002 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0002);
   /* RW U1 p0 R1 h359a */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x359a + (0x800 * port_num));
   /* RW U1 p0 R2 h3125 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x3125);
   /* RW U1 p0 R3 h0003 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0003);
   /* RW U1 p0 R1 h359b */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x359b + (0x800 * port_num));
   /* RW U1 p0 R2 hd860 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xd860);
   /* RW U1 p0 R3 h0000 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0000);
   /* RW U1 p0 R1 h359c */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x359c + (0x800 * port_num));
   /* RW U1 p0 R2 hde4a */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xde4a);
   /* RW U1 p0 R3 h0001 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0001);
   /* RW U1 p0 R1 h359d */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x359d + (0x800 * port_num));
   /* RW U1 p0 R2 h18e7 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x18e7);
   /* RW U1 p0 R3 h0002 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0002);
   /* RW U1 p0 R1 h359e */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x359e + (0x800 * port_num));
   /* RW U1 p0 R2 hd406 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xd406);
   /* RW U1 p0 R3 h0002 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0002);
   /* RW U1 p0 R1 h359f */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x359f + (0x800 * port_num));
   /* RW U1 p0 R2 h843f */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x843f);
   /* RW U1 p0 R3 h0002 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0002);
   /* RW U1 p0 R1 h35a0 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x35a0 + (0x800 * port_num));
   /* RW U1 p0 R2 hcc3c */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xcc3c);
   /* RW U1 p0 R3 h0002 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0002);
   /* RW U1 p0 R1 h35a1 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x35a1 + (0x800 * port_num));
   /* RW U1 p0 R2 hac2a */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xac2a);
   /* RW U1 p0 R3 h0002 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0002);
   /* RW U1 p0 R1 h35a2 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x35a2 + (0x800 * port_num));
   /* RW U1 p0 R2 h9c65 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x9c65);
   /* RW U1 p0 R3 h0002 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0002);
   /* RW U1 p0 R1 h35a3 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x35a3 + (0x800 * port_num));
   /* RW U1 p0 R2 ha465 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xa465);
   /* RW U1 p0 R3 h0002 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0002);
   /* RW U1 p0 R1 h35a4 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x35a4 + (0x800 * port_num));
   /* RW U1 p0 R2 hc41c */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xc41c);
   /* RW U1 p0 R3 h0002 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0002);
   /* RW U1 p0 R1 h35a5 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x35a5 + (0x800 * port_num));
   /* RW U1 p0 R2 hb489 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xb489);
   /* RW U1 p0 R3 h0002 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0002);
   /* RW U1 p0 R1 h35a6 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x35a6 + (0x800 * port_num));
   /* RW U1 p0 R2 hbc94 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xbc94);
   /* RW U1 p0 R3 h0002 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0002);
   /* RW U1 p0 R1 h35a7 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x35a7 + (0x800 * port_num));
   /* RW U1 p0 R2 h94c1 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x94c1);
   /* RW U1 p0 R3 h0002 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0002);
   /* RW U1 p0 R1 h35a8 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x35a8 + (0x800 * port_num));
   /* RW U1 p0 R2 h8c51 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x8c51);
   /* RW U1 p0 R3 h0002 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0002);
   /* RW U1 p0 R1 h35a9 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x35a9 + (0x800 * port_num));
   /* RW U1 p0 R2 hd840 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xd840);
   /* RW U1 p0 R3 h0000 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0000);
   /* RW U1 p0 R1 h35aa */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x35aa + (0x800 * port_num));
   /* RW U1 p0 R2 h5452 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x5452);
   /* RW U1 p0 R3 h0003 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0003);
   /* RW U1 p0 R1 h35ab */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x35ab + (0x800 * port_num));
   /* RW U1 p0 R2 h3850 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x3850);
   /* RW U1 p0 R3 h0002 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0002);
   /* RW U1 p0 R1 h35ac */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x35ac + (0x800 * port_num));
   /* RW U1 p0 R2 he004 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xe004);
   /* RW U1 p0 R3 h0000 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0000);
   /* RW U1 p0 R1 h35ad */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x35ad + (0x800 * port_num));
   /* RW U1 p0 R2 h0000 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x0000);
   /* RW U1 p0 R3 h0003 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0003);
   /* RW U1 p0 R1 h35ae */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x35ae + (0x800 * port_num));
   /* RW U1 p0 R2 h59d1 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x59d1);
   /* RW U1 p0 R3 h0003 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0003);
   /* RW U1 p0 R1 h35af */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x35af + (0x800 * port_num));
   /* RW U1 p0 R2 h40b8 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x40b8);
   /* RW U1 p0 R3 h0002 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0002);
   /* RW U1 p0 R1 h35b0 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x35b0 + (0x800 * port_num));
   /* RW U1 p0 R2 h59ef */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x59ef);
   /* RW U1 p0 R3 h0003 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0003);
   /* RW U1 p0 R1 h35b1 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x35b1 + (0x800 * port_num));
   /* RW U1 p0 R2 h409a */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x409a);
   /* RW U1 p0 R3 h0002 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0002);
   /* RW U1 p0 R1 h35b2 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x35b2 + (0x800 * port_num));
   /* RW U1 p0 R2 h59c4 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x59c4);
   /* RW U1 p0 R3 h0003 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0003);
   /* RW U1 p0 R1 h35b3 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x35b3 + (0x800 * port_num));
   /* RW U1 p0 R2 h4089 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x4089);
   /* RW U1 p0 R3 h0002 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0002);
   /* RW U1 p0 R1 h35b4 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x35b4 + (0x800 * port_num));
   /* RW U1 p0 R2 h59e9 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x59e9);
   /* RW U1 p0 R3 h0003 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0003);
   /* RW U1 p0 R1 h35b5 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x35b5 + (0x800 * port_num));
   /* RW U1 p0 R2 h4094 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x4094);
   /* RW U1 p0 R3 h0002 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0002);
   /* RW U1 p0 R1 h35b6 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x35b6 + (0x800 * port_num));
   /* RW U1 p0 R2 he004 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xe004);
   /* RW U1 p0 R3 h0000 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0000);
   /* RW U1 p0 R1 h35b7 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x35b7 + (0x800 * port_num));
   /* RW U1 p0 R2 h0000 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x0000);
   /* RW U1 p0 R3 h0003 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0003);
   /* RW U1 p0 R1 h35b8 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x35b8 + (0x800 * port_num));
   /* RW U1 p0 R2 h3121 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x3121);
   /* RW U1 p0 R3 h0003 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0003);
   /* RW U1 p0 R1 h35b9 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x35b9 + (0x800 * port_num));
   /* RW U1 p0 R2 hd8e0 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xd8e0);
   /* RW U1 p0 R3 h0000 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0000);
   /* RW U1 p0 R1 h35ba */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x35ba + (0x800 * port_num));
   /* RW U1 p0 R2 hdcbe */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xdcbe);
   /* RW U1 p0 R3 h0002 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0002);
   /* RW U1 p0 R1 h35bb */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x35bb + (0x800 * port_num));
   /* RW U1 p0 R2 hfcbe */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xfcbe);
   /* RW U1 p0 R3 h0002 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0002);
   /* RW U1 p0 R1 h35bc */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x35bc + (0x800 * port_num));
   /* RW U1 p0 R2 he004 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xe004);
   /* RW U1 p0 R3 h0000 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0000);
   /* RW U1 p0 R1 h35bd */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x35bd + (0x800 * port_num));
   /* RW U1 p0 R2 h0000 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x0000);
   /* RW U1 p0 R3 h0003 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0003);
   /* RW U1 p0 R1 h35be */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x35be + (0x800 * port_num));
   /* RW U1 p0 R2 hd8e0 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xd8e0);
   /* RW U1 p0 R3 h0000 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0000);
   /* RW U1 p0 R1 h35bf */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x35bf + (0x800 * port_num));
   /* RW U1 p0 R2 hd8e0 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xd8e0);
   /* RW U1 p0 R3 h0000 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0000);
   /* RW U1 p0 R1 h35c0 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x35c0 + (0x800 * port_num));
   /* RW U1 p0 R2 he063 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xe063);
   /* RW U1 p0 R3 h0000 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0000);
   /* RW U1 p0 R1 h35c1 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x35c1 + (0x800 * port_num));
   /* RW U1 p0 R2 h0cc7 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x0cc7);
   /* RW U1 p0 R3 h0007 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0007);
   /* RW U1 p0 R1 h35c2 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x35c2 + (0x800 * port_num));
   /* RW U1 p0 R2 h58c0 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x58c0);
   /* RW U1 p0 R3 h0003 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0003);
   /* RW U1 p0 R1 h35c3 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x35c3 + (0x800 * port_num));
   /* RW U1 p0 R2 h40c7 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x40c7);
   /* RW U1 p0 R3 h0002 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0002);
   /* RW U1 p0 R1 h35c4 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x35c4 + (0x800 * port_num));
   /* RW U1 p0 R2 hd042 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xd042);
   /* RW U1 p0 R3 h0000 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0000);
   /* RW U1 p0 R1 h35c5 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x35c5 + (0x800 * port_num));
   /* RW U1 p0 R2 h3068 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x3068);
   /* RW U1 p0 R3 h0003 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0003);
   /* RW U1 p0 R1 h35c6 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x35c6 + (0x800 * port_num));
   /* RW U1 p0 R2 he011 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xe011);
   /* RW U1 p0 R3 h0000 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0000);
   /* RW U1 p0 R1 h35c7 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x35c7 + (0x800 * port_num));
   /* RW U1 p0 R2 he068 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xe068);
   /* RW U1 p0 R3 h0000 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0000);
   /* RW U1 p0 R1 h35c8 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x35c8 + (0x800 * port_num));
   /* RW U1 p0 R2 hd840 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xd840);
   /* RW U1 p0 R3 h0000 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0000);
   /* RW U1 p0 R1 h35c9 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x35c9 + (0x800 * port_num));
   /* RW U1 p0 R2 h09d9 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x09d9);
   /* RW U1 p0 R3 h0007 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0007);
   /* RW U1 p0 R1 h35ca */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x35ca + (0x800 * port_num));
   /* RW U1 p0 R2 h3120 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x3120);
   /* RW U1 p0 R3 h0003 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0003);
   /* RW U1 p0 R1 h35cb */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x35cb + (0x800 * port_num));
   /* RW U1 p0 R2 hf00c */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xf00c);
   /* RW U1 p0 R3 h0000 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0000);
   /* RW U1 p0 R1 h35cc */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x35cc + (0x800 * port_num));
   /* RW U1 p0 R2 hf02c */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xf02c);
   /* RW U1 p0 R3 h0000 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0000);
   /* RW U1 p0 R1 h35cd */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x35cd + (0x800 * port_num));
   /* RW U1 p0 R2 hf024 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xf024);
   /* RW U1 p0 R3 h0000 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0000);
   /* RW U1 p0 R1 h35ce */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x35ce + (0x800 * port_num));
   /* RW U1 p0 R2 hf01c */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xf01c);
   /* RW U1 p0 R3 h0000 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0000);
   /* RW U1 p0 R1 h35cf */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x35cf + (0x800 * port_num));
   /* RW U1 p0 R2 hf044 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xf044);
   /* RW U1 p0 R3 h0000 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0000);
   /* RW U1 p0 R1 h35d0 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x35d0 + (0x800 * port_num));
   /* RW U1 p0 R2 hf04c */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xf04c);
   /* RW U1 p0 R3 h0000 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0000);
   /* RW U1 p0 R1 h35d1 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x35d1 + (0x800 * port_num));
   /* RW U1 p0 R2 hf054 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xf054);
   /* RW U1 p0 R3 h0000 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0000);
   /* RW U1 p0 R1 h35d2 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x35d2 + (0x800 * port_num));
   /* RW U1 p0 R2 hf05c */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xf05c);
   /* RW U1 p0 R3 h0000 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0000);
   /* RW U1 p0 R1 h35d3 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x35d3 + (0x800 * port_num));
   /* RW U1 p0 R2 hf034 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xf034);
   /* RW U1 p0 R3 h0000 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0000);
   /* RW U1 p0 R1 h35d4 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x35d4 + (0x800 * port_num));
   /* RW U1 p0 R2 hf03c */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xf03c);
   /* RW U1 p0 R3 h0000 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0000);
   /* RW U1 p0 R1 h35d5 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x35d5 + (0x800 * port_num));
   /* RW U1 p0 R2 hd805 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xd805);
   /* RW U1 p0 R3 h0001 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0001);
   /* RW U1 p0 R1 h35d6 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x35d6 + (0x800 * port_num));
   /* RW U1 p0 R2 hf070 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xf070);
   /* RW U1 p0 R3 h0000 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0000);
   /* RW U1 p0 R1 h35d7 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x35d7 + (0x800 * port_num));
   /* RW U1 p0 R2 he004 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xe004);
   /* RW U1 p0 R3 h0000 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0000);
   /* RW U1 p0 R1 h35d8 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x35d8 + (0x800 * port_num));
   /* RW U1 p0 R2 h0000 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x0000);
   /* RW U1 p0 R3 h0003 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0003);
   /* RW U1 p0 R1 h35d9 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x35d9 + (0x800 * port_num));
   /* RW U1 p0 R2 h3128 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x3128);
   /* RW U1 p0 R3 h0003 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0003);
   /* RW U1 p0 R1 h35da */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x35da + (0x800 * port_num));
   /* RW U1 p0 R2 hf02c */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xf02c);
   /* RW U1 p0 R3 h0000 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0000);
   /* RW U1 p0 R1 h35db */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x35db + (0x800 * port_num));
   /* RW U1 p0 R2 hf044 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xf044);
   /* RW U1 p0 R3 h0000 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0000);
   /* RW U1 p0 R1 h35dc */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x35dc + (0x800 * port_num));
   /* RW U1 p0 R2 hd808 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xd808);
   /* RW U1 p0 R3 h0001 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0001);
   /* RW U1 p0 R1 h35dd */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x35dd + (0x800 * port_num));
   /* RW U1 p0 R2 hf00c */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xf00c);
   /* RW U1 p0 R3 h0000 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0000);
   /* RW U1 p0 R1 h35de */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x35de + (0x800 * port_num));
   /* RW U1 p0 R2 hf04c */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xf04c);
   /* RW U1 p0 R3 h0000 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0000);
   /* RW U1 p0 R1 h35df */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x35df + (0x800 * port_num));
   /* RW U1 p0 R2 hf054 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xf054);
   /* RW U1 p0 R3 h0000 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0000);
   /* RW U1 p0 R1 h35e0 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x35e0 + (0x800 * port_num));
   /* RW U1 p0 R2 hf05c */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xf05c);
   /* RW U1 p0 R3 h0000 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0000);
   /* RW U1 p0 R1 h35e1 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x35e1 + (0x800 * port_num));
   /* RW U1 p0 R2 hf024 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xf024);
   /* RW U1 p0 R3 h0000 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0000);
   /* RW U1 p0 R1 h35e2 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x35e2 + (0x800 * port_num));
   /* RW U1 p0 R2 hf074 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xf074);
   /* RW U1 p0 R3 h0000 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0000);
   /* RW U1 p0 R1 h35e3 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x35e3 + (0x800 * port_num));
   /* RW U1 p0 R2 hd802 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xd802);
   /* RW U1 p0 R3 h0001 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0001);
   /* RW U1 p0 R1 h35e4 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x35e4 + (0x800 * port_num));
   /* RW U1 p0 R2 hf018 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xf018);
   /* RW U1 p0 R3 h0000 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0000);
   /* RW U1 p0 R1 h35e5 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x35e5 + (0x800 * port_num));
   /* RW U1 p0 R2 h0004 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x0004);
   /* RW U1 p0 R3 h0000 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0000);
   /* RW U1 p0 R1 h35e6 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x35e6 + (0x800 * port_num));
   /* RW U1 p0 R2 h0000 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x0000);
   /* RW U1 p0 R3 h0003 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0003);
   /* RW U1 p0 R1 h35e7 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x35e7 + (0x800 * port_num));
   /* RW U1 p0 R2 he004 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xe004);
   /* RW U1 p0 R3 h0000 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0000);
   /* RW U1 p0 R1 h35e8 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x35e8 + (0x800 * port_num));
   /* RW U1 p0 R2 h0000 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x0000);
   /* RW U1 p0 R3 h0003 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0003);
   /* RW U1 p0 R1 h35e9 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x35e9 + (0x800 * port_num));
   /* RW U1 p0 R2 h312f */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x312f);
   /* RW U1 p0 R3 h0003 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0003);
   /* RW U1 p0 R1 h35ea */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x35ea + (0x800 * port_num));
   /* RW U1 p0 R2 he004 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xe004);
   /* RW U1 p0 R3 h0000 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0000);
   /* RW U1 p0 R1 h35eb */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x35eb + (0x800 * port_num));
   /* RW U1 p0 R2 h0000 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x0000);
   /* RW U1 p0 R3 h0003 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0003);
   /* RW U1 p0 R1 h35ec */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x35ec + (0x800 * port_num));
   /* RW U1 p0 R2 h0000 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x0000);
   /* RW U1 p0 R3 h0000 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0000);
   /* RW U1 p0 R1 h35ed */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x35ed + (0x800 * port_num));
   /* RW U1 p0 R2 h0000 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x0000);
   /* RW U1 p0 R3 h0000 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0000);
   /* RW U1 p0 R1 h35ee */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x35ee + (0x800 * port_num));
   /* RW U1 p0 R2 h0000 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x0000);
   /* RW U1 p0 R3 h0000 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0000);
   /* RW U1 p0 R1 h35ef */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x35ef + (0x800 * port_num));
   /* RW U1 p0 R2 h0000 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x0000);
   /* RW U1 p0 R3 h0000 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0000);
   /* RW U1 p0 R1 h35f0 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x35f0 + (0x800 * port_num));
   /* RW U1 p0 R2 h0000 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x0000);
   /* RW U1 p0 R3 h0000 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0000);
   /* RW U1 p0 R1 h35f1 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x35f1 + (0x800 * port_num));
   /* RW U1 p0 R2 h0000 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x0000);
   /* RW U1 p0 R3 h0000 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0000);
   /* RW U1 p0 R1 h35f2 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x35f2 + (0x800 * port_num));
   /* RW U1 p0 R2 h0000 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x0000);
   /* RW U1 p0 R3 h0000 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0000);
   /* RW U1 p0 R1 h35f3 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x35f3 + (0x800 * port_num));
   /* RW U1 p0 R2 h0000 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x0000);
   /* RW U1 p0 R3 h0000 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0000);
   /* RW U1 p0 R1 h35f4 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x35f4 + (0x800 * port_num));
   /* RW U1 p0 R2 h0000 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x0000);
   /* RW U1 p0 R3 h0000 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0000);
   /* RW U1 p0 R1 h35f5 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x35f5 + (0x800 * port_num));
   /* RW U1 p0 R2 h0000 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x0000);
   /* RW U1 p0 R3 h0000 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0000);
   /* RW U1 p0 R1 h35f6 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x35f6 + (0x800 * port_num));
   /* RW U1 p0 R2 h0000 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x0000);
   /* RW U1 p0 R3 h0000 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0000);
   /* RW U1 p0 R1 h35f7 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x35f7 + (0x800 * port_num));
   /* RW U1 p0 R2 h0000 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x0000);
   /* RW U1 p0 R3 h0000 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0000);
   /* RW U1 p0 R1 h35f8 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x35f8 + (0x800 * port_num));
   /* RW U1 p0 R2 h0000 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x0000);
   /* RW U1 p0 R3 h0000 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0000);
   /* RW U1 p0 R1 h35f9 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x35f9 + (0x800 * port_num));
   /* RW U1 p0 R2 h0000 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x0000);
   /* RW U1 p0 R3 h0000 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0000);
   /* RW U1 p0 R1 h35fa */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x35fa + (0x800 * port_num));
   /* RW U1 p0 R2 h0000 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x0000);
   /* RW U1 p0 R3 h0000 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0000);
   /* RW U1 p0 R1 h35fb */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x35fb + (0x800 * port_num));
   /* RW U1 p0 R2 h0000 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x0000);
   /* RW U1 p0 R3 h0000 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0000);
   /* RW U1 p0 R1 h35fc */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x35fc + (0x800 * port_num));
   /* RW U1 p0 R2 h0000 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x0000);
   /* RW U1 p0 R3 h0000 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0000);
   /* RW U1 p0 R1 h35fd */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x35fd + (0x800 * port_num));
   /* RW U1 p0 R2 h0000 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x0000);
   /* RW U1 p0 R3 h0000 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0000);
   /* RW U1 p0 R1 h35fe */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x35fe + (0x800 * port_num));
   /* RW U1 p0 R2 h0000 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x0000);
   /* RW U1 p0 R3 h0000 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0000);
   /* RW U1 p0 R1 h35ff */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x35ff + (0x800 * port_num));
   /* RW U1 p0 R2 h0002 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x0002);
   /* RW U1 p0 R3 h0000 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0000);

    return (PASSED);
}

int en_88e1548_ptp_per_port (int eth_port, int speed)
{
    int start_phy_addr[] = {0x0, 0x4};
    int four_ge_eth_port_map[] = {0x0, 0x0, 0x0, 0x0, 0x3, 0x2, 0x1, 0x0};
    int six_ge_eth_port_map[] = {0x1, 0x0, 0x0, 0x0, 0x3, 0x2, 0x1, 0x0};
    int phy_addr, bus_id;
    int sku_id;
    int phy_num, port_num;

    sku_id = get_sku_id();

    if (sku_id == WOODLAWN_4GE_1XAUI) {
        phy_num = 1;
        port_num = four_ge_eth_port_map[eth_port];
        phy_addr = start_phy_addr[0];
    } else {
        port_num = six_ge_eth_port_map[eth_port];
        if (eth_port < 0x4) {
            phy_addr = start_phy_addr[1];
        } else {
            phy_addr = start_phy_addr[0];
        }
    }
    
    bus_id = get_smi_bus_id(phy_addr);

    /* Switch page to 16 to access PTP/MACsec Registers */
    /* RW U1 p0 R22 h0010 */
    woodlawn_phy_reg_wr(bus_id, phy_addr, 22, 0x0010);

   /* message type 0 ingress */ 
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3780 + (0x800 * port_num));
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x0001);
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0000);
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3781 + (0x800 * port_num));
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x0000);
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0000);
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3782 + (0x800 * port_num));
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x0000);
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0000);
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3784 + (0x800 * port_num));
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x0001);
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0f00);
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3785 + (0x800 * port_num));
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x0000);
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0000);
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3786 + (0x800 * port_num));
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x0000);
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0000);
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3700 + (0x800 * port_num));
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x0000);
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0000);
   
   /* message type 0 egress */ 
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3380 + (0x800 * port_num));
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x0001);
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0000);
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3381 + (0x800 * port_num));
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x0000);
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0000);
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3382 + (0x800 * port_num));
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x0000);
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0000);
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3384 + (0x800 * port_num));
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x0001);
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0f00);
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3385 + (0x800 * port_num));
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x0000);
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0000);
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3386 + (0x800 * port_num));
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x0000);
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0000);
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3300 + (0x800 * port_num));
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0xa140);
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0000);

    /* Set Latency Adjustment on Egress 
    (In 2-Step, CPU needs to include it in Residence Time Calculation) */
    /* RW U1 p0 R1 h305B */
    woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x305b + (0x800 * port_num));
    /* RW U1 p0 R2 h1878 */
    if (speed == SPD_1000MBPS) {
        woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x1878);
    } else if (speed == SPD_100MBPS) {
        woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x5a74);
    } else {
        woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x4f1c);
    }
    /* RW U1 p0 R3 h0000 */
    woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0000);
    /* RW U1 p0 R1 h305C */
    woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x305c + (0x800 * port_num));
    /* RW U1 p0 R2 h0000 */
    woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x0000);
    /* RW U1 p0 R3 h0000 */
    woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0000);
    /* RW U1 p0 R1 h3085 */
    woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3085 + (0x800 * port_num));
    /* RW U1 p0 R2 h0300 */
    if (speed == SPD_1000MBPS) {
        woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x0300);
    } else if (speed == SPD_100MBPS) {
        woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x0ac8);
    } else {
        woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x64c8);
    }
    /* RW U1 p0 R3 h0000 */
    woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0000);
    /* RW U1 p0 R1 h3086 */
    woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3086 + (0x800 * port_num));
    /* RW U1 p0 R2 h0300 */
    if (speed == SPD_1000MBPS) {
        woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x0300);
    } else if (speed == SPD_100MBPS) {
        woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x0ac8);
    } else {
        woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x64c8);
    }
    /* RW U1 p0 R3 h0004 */
    woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0004);

   /* Timeout Control */
   /* RW U1 p0 R1 h3084 */
   /* Egress timeout_ctl (Table 436) */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3084 + (0x800 * port_num));
   /* RW U1 p0 R2 h0000 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x0000);
   /* RW U1 p0 R3 h0000 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0000);
   /* RW U1 p0 R1 h3484 */
   /* Ingress timeout_ctl (Table 436) */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3484 + (0x800 * port_num));
   /* RW U1 p0 R2 h0000 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x0000);
   /* RW U1 p0 R3 h0000 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0000);

   /* Setting ts_bound */
   /* Ingress ts_queue (Table 441) */
   /* RW U1 p0 R1 h3489 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3489 + (0x800 * port_num));
   /* RW U1 p0 R2 h0100 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x0100);
   /* RW U1 p0 R3 h0000 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0000);
   /* RW U1 p0 R1 h3089 */
   /* Egress ts_queue (Table 441) */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3089 + (0x800 * port_num));
   /* RW U1 p0 R2 h0100 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x0100);
   /* RW U1 p0 R3 h0000 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0000);

  /* Other Errata from Release Note */
  /* Errata 7.2 (set wire mac minimum IPG = 11 bytes), Table 312 */
  /* RW U1 p0 R1 h0041 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x0041 + (0x800 * port_num));
   /* RW U1 p0 R2 h00b1 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x00b1);
   /* RW U1 p0 R3 h0002 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0002);
   /* Errata 7.4 (disable drop_bad_tag), Table 265 */
   /* RW U1 p0 R1 h000b */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x000b + (0x800 * port_num));
   /* RW U1 p0 R2 h0000 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x0000);
   /* RW U1 p0 R3 h0fb4 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0fb4);

    /* 2-step ptp */
    woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x001d + (0x800 * port_num));
    woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x003f);
    woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0000);
    woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x0046 + (0x800 * port_num));
    woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x000f);
    woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0000);
    woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x0056 + (0x800 * port_num));
    woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x000f);
    woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0000);
    woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3483 + (0x800 * port_num));
    woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x6ff0);
    woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0000);
    woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3083 + (0x800 * port_num));
    woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x6ff0);
    woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0000);

   /* Set Cut-Through Mode in MacSec */
   /* RW U1 p0 R1 h0070 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x0070 + (0x800 * port_num));
   /* RW U1 p0 R2 h0074 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x0074);
   /* RW U1 p0 R3 h0000 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0000);

    /* Enable PTP/MACsec (Table 217) */
    /* RW U1 p0 R22 h0012 */
    woodlawn_phy_reg_wr(bus_id, phy_addr + port_num, 22, 0x0012);
    /* RW U1 p0 R27 h2000 */
    woodlawn_phy_reg_wr(bus_id, phy_addr + port_num, 27, 0x2000);
    /* RW U1 p0 R22 h0010 */
    woodlawn_phy_reg_wr(bus_id, phy_addr + port_num, 22, 0x0010);

   /* Enable PTP Core */
   /* RW U1 p0 R1 h3480 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3480 + (0x800 * port_num));
   /* RW U1 p0 R2 h0009 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x0009);
   /* RW U1 p0 R3 h0000 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0000);
   /* RW U1 p0 R1 h3080 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x3080 + (0x800 * port_num));
   /* RW U1 p0 R2 h0009 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x0009);
   /* RW U1 p0 R3 h0000 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0000);

  /* Release Note Errata 7.9 */
  /* RW U1 p0 R1 h0040 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x0040 + (0x800 * port_num));
   /* RW U1 p0 R2 h1849 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x1849);
   /* RW U1 p0 R3 h0001 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0001);
   /* RW U1 p0 R1 h0050 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 1, 0x0050 + (0x800 * port_num));
   /* RW U1 p0 R2 h17c9 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 2, 0x17c9);
   /* RW U1 p0 R3 h0000 */
   woodlawn_phy_reg_wr(bus_id, phy_addr, 3, 0x0000);

    /* RW U1 p0 R22 h0000 */
    woodlawn_phy_reg_wr(bus_id, phy_addr, 22, 0x0000);

    return(PASSED);
}

/***********************************************************************
 *  
 * Function: switch_led_mode
 *    
 * Description: Switch led mode
 *      
 * Inputs: None
 *        
 * Outputs: PASSED/FAILED
 *          
 *************************************************************************/
int setup_1548_ptp_engine (void)
{
    int id, ix, eth_port;
    int *sku_eth_mapping_ge_num;
    int four_ge_eth_port[] = {0x4, 0x5, 0x6, 0x7};
    int six_ge_eth_port[] = {0x0, 0x1, 0x4, 0x5, 0x6, 0x7};

    /* Get the SKU id */
    id = get_sku_id();

    /* Not official SKU has different eth number copper port mapping compared
     * with new official SKU
     */
    if (id == WOODLAWN_6GE) {
        sku_eth_mapping_ge_num = two_phy_eth_mapping_ge_num;
    } else if (id == WOODLAWN_4GE_1XAUI) {
        sku_eth_mapping_ge_num = one_phy_eth_mapping_ge_num;
    }

    if (id == WOODLAWN_6GE) {
        for (ix = 0; ix <= 5; ix++) {
            enable_88e1548_ptp_engine(six_ge_eth_port[ix]);
        }
    } else {
        for (ix = 0; ix <= 3; ix++) {
            enable_88e1548_ptp_engine(four_ge_eth_port[ix]);
        }
    }

    eth_port = gethex_answer("Enter eth (0 ~ 7): ", 0, 0, 0x7);
    en_88e1548_ptp_per_port (eth_port, SPD_1000MBPS);

    return (PASSED);
}

/***********************************************************************
 *  
 * Function: switch_led_mode
 *    
 * Description: Switch led mode
 *      
 * Inputs: None
 *        
 * Outputs: PASSED/FAILED
 *          
 *************************************************************************/
int switch_led_mode (void)
{
    printf("Show phy LED link status.\n");

    int phy_addr, port_num, function_reg_val, led_mode;
    int port_map, sku_id, bus_id;

    sku_id = get_sku_id();
    
    led_mode = gethex_answer("Choose LED mode(0-copper, 1-fiber)", 0, 0, 0x1);

    /* Alter register with new value */
    if (led_mode == 0) {
        function_reg_val = MRVL_88E1548_GE_ON_SFP_OFF;
        port_num = gethex_answer("Enter GE port number(GE0~GE5)", 0, 0, 0x5);
        port_map = get_phy_port(sku_id, port_num);
    } else {
        function_reg_val = MRVL_88E1548_GE_OFF_SFP_ON;
        port_num = gethex_answer("Enter SFP port number(SFP0~SFP5)", 0, 0, 0x5);
        port_map = get_phy_port(sku_id, port_num);
    }

    phy_addr = get_88e1548_phy_addr(port_num, port_map);

    bus_id = get_88e1548_bus_id(phy_addr);
    
    /* Need to write page register for set page */
    if (woodlawn_phy_reg_wr(bus_id, phy_addr, MRV88E1548L_PAGE_ADDRESS_REG, 
        MRV88E1548L_REG_PAGE_3) == FAILED) {
        printf("Set Register Page %d Fail !\n", MRV88E1548L_REG_PAGE_3);
        return (FAILED);
    }

    /* Alter the current register with new vlaue */
    if (woodlawn_phy_reg_wr(bus_id, phy_addr, MRVL_88E1548_LED_FUNCTION_REG, 
        function_reg_val) == FAILED) {
        printf("Alter port %d fails\n", port_num);
        return (FAILED);
    }
    
    if (woodlawn_phy_reg_wr(bus_id, phy_addr, MRVL_88E1548_LED_POLARITY_REG, 
        MRVL_88E1548_LED_POLARITY) == FAILED) {
        printf("Alter port %d fails\n", port_num);
        return (FAILED);
    }
    return PASSED;
}

static int setup_1548_pseudorandom_packet (void)
{
    int port_num, phy_addr, port_map;
    int sku_id, bus_id;

    sku_id = get_sku_id();

    if (sku_id == WOODLAWN_6GE) {
        port_num = gethex_answer("Enter GE port number(GE0~GE5)", 0, 0, 0x5);
    } else if (sku_id == WOODLAWN_4GE_1XAUI) {
        port_num = gethex_answer("Enter GE port number(GE0~GE3)", 0, 0, 0x3);
    } else {
        port_num = gethex_answer("Enter GE port number(GE0~GE5)", 0, 0, 0x5);
    }

    port_map = get_phy_port(sku_id, port_num);

    if (sku_id == WOODLAWN_6GE_1XAUI) {
        /* Not official SKU */
        phy_addr = get_88e1548_phy_addr(port_num, port_map);
    } else {
        /* Official SKU */
        if (sku_id == WOODLAWN_6GE) {
            phy_addr = get_88e1548_phy_addr(port_num, port_map);
        } else if (sku_id == WOODLAWN_4GE_1XAUI) {
            phy_addr = get_88e1548_4ge_phy_addr(port_num, port_map);
        }
    }

    bus_id = get_88e1548_bus_id(phy_addr);

    /* Need to write page register for set page */
    if (woodlawn_phy_reg_wr(bus_id, phy_addr, MRV88E1548L_PAGE_ADDRESS_REG, 
        0x0) == FAILED) {
        printf("Set Register Page %d Fail !\n", 0x0);
        return (FAILED);
    }

    woodlawn_phy_reg_wr(bus_id, phy_addr, 0x0, 0x1100); 

    /* Need to write page register for set page */
    if (woodlawn_phy_reg_wr(bus_id, phy_addr, MRV88E1548L_PAGE_ADDRESS_REG, 
        0x1) == FAILED) {
        printf("Set Register Page %d Fail !\n", 0x1);
        return (FAILED);
    }
        
    woodlawn_phy_reg_wr(bus_id, phy_addr, 0x16, 0x00fc);

    /* Need to write page register for set page */
    if (woodlawn_phy_reg_wr(bus_id, phy_addr, MRV88E1548L_PAGE_ADDRESS_REG, 
        0xfc) == FAILED) {
        printf("Set Register Page %d Fail !\n", 0xfc);
        return (FAILED);
    }
        
    woodlawn_phy_reg_wr(bus_id, phy_addr, 0x11, 0xaa11);
    woodlawn_phy_reg_wr(bus_id, phy_addr, 0x16, 0x0000);

    /* Need to write page register for set page */
    if (woodlawn_phy_reg_wr(bus_id, phy_addr, MRV88E1548L_PAGE_ADDRESS_REG, 
        0x6) == FAILED) {
        printf("Set Register Page %d Fail !\n", 0x6);
        return (FAILED);
    }
        
    woodlawn_phy_reg_wr(bus_id, phy_addr, 0x10, 0x0018);

    return (PASSED);
}

/*******************************************************************************
 *  
 * Function    : setup_1548_test_mode
 *    
 * Description : Set up 88E1548L test mode
 *      
 * Inputs      : None
 *        
 * Outputs     : PASSED / FAILED
 *          
 **********************************************************************************/
static int setup_1548_test_mode (void)
{
    int port_num, phy_addr, mode, val, port_map;
    int sku_id, bus_id;
   
    printf("Enter phy 88E1548L test modes.\n");
    printf("Enter 0 - Normal Mode\n");
    printf("Enter 1 - Transmit Waveform Test\n");
    printf("Enter 2 - Transmit Jitter Test Master Mode\n");
    printf("Enter 3 - Transmit Jitter Test Slave Mode\n");
    printf("Enter 4 - Transmit Distortion Test\n");

    sku_id = get_sku_id();

    if (sku_id == WOODLAWN_6GE) {
        port_num = gethex_answer("Enter GE port number(GE0~GE5)", 0, 0, 0x5);
    } else if (sku_id == WOODLAWN_4GE_1XAUI) {                                                                                      
        port_num = gethex_answer("Enter GE port number(GE0~GE3)", 0, 0, 0x3);
    } else {
        port_num = gethex_answer("Enter GE port number(GE0~GE5)", 0, 0, 0x5);
    }

    port_map = get_phy_port(sku_id, port_num);

    mode = gethex_answer("Select Test Modes: ", 0, 0, 0x4);

    if (sku_id == WOODLAWN_6GE_1XAUI) {
        /* Not official SKU */
        phy_addr = get_88e1548_phy_addr(port_num, port_map);
    } else {
        /* Official SKU */
        if (sku_id == WOODLAWN_6GE) {
            phy_addr = get_88e1548_phy_addr(port_num, port_map);
        } else if (sku_id == WOODLAWN_4GE_1XAUI) {
            phy_addr = get_88e1548_4ge_phy_addr(port_num, port_map);
        }
    }

    bus_id = get_88e1548_bus_id(phy_addr);
    
    /* Common setting for all test mode */
    if (woodlawn_phy_reg_wr(bus_id, phy_addr, MRV88E1548L_PAGE_ADDRESS_REG, 
                                          MRV88E1548L_REG_PAGE_4) == FAILED) {
        printf("Set Register Page %d Fail !\n", MRV88E1548L_REG_PAGE_4);
        return (FAILED);
    }
    if (woodlawn_phy_reg_wr(bus_id, phy_addr, MRV88E1548L_REG_PAGE_27, 
                                          DISABLE_CLOCK) == FAILED) {
        printf("Writer Register %#.8x Fail !\n", DISABLE_CLOCK);
        return (FAILED);
    }
    if (woodlawn_phy_reg_wr(bus_id, phy_addr, MRV88E1548L_PAGE_ADDRESS_REG, 
                                          MRV88E1548L_REG_PAGE_6) == FAILED) {
        printf("Set Register Page %d Fail !\n", MRV88E1548L_REG_PAGE_6);
        return (FAILED);
    }
    if (woodlawn_phy_reg_wr(bus_id, phy_addr, MRV88E1548L_REG_PAGE_26, 
                                          ENABLE_TX_TCLK) == FAILED) {
        printf("Writer Register %#.8x Fail !\n", ENABLE_TX_TCLK);
        return (FAILED);
    }

    /* Need to write page register for set page */
    if (woodlawn_phy_reg_wr(bus_id, phy_addr, MRV88E1548L_PAGE_ADDRESS_REG, 
        MRVL_88E1548_TEST_MODE_PAGE) == FAILED) {
        printf("Set Register Page %d Fail !\n", MRVL_88E1548_TEST_MODE_PAGE);
        return (FAILED);
    }
    
    if ((mode == TRANSMIT_WAVEFORM_TEST) || 
        (mode == TRANSMIT_JITTER_TEST_MASTER_MODE) || 
        (mode == TRANSMIT_DISTORTION_TEST)) {
        woodlawn_phy_reg_wr(bus_id, phy_addr, MRVL_88E1548_TEST_MODE_REG, 
        SET_PHY_TO_MASTER_MODE);
        woodlawn_phy_reg_wr(bus_id, phy_addr, MRV88E1548L_REG_PAGE_0, PHY_SOFT_RESET);
    } else if (mode == TRANSMIT_JITTER_TEST_SLAVE_MODE) {
        woodlawn_phy_reg_wr(bus_id, phy_addr, MRVL_88E1548_TEST_MODE_REG, 
        SET_PHY_TO_SLAVE_MODE);
        woodlawn_phy_reg_wr(bus_id, phy_addr, MRV88E1548L_REG_PAGE_0, PHY_SOFT_RESET);
    }
    
    switch (mode) {
        case 0:
            printf("Normal Mode\n");
            val = MRVL_88E1548_NORMAL_MODE;
            break;
        case 1:
            printf("Test Mode 1 - Transmit Waveform Test\n");
            val = MRVL_88E1548_TEST_MODE_1;
            break;
        case 2:
            printf("Test Mode 2 - Transmit Jitter Test(Master Mode)\n");
            val = MRVL_88E1548_TEST_MODE_2;
            break;
        case 3:
            printf("Test Mode 3 - Transmit Jitter Test(Slave Mode)\n");
            val = MRVL_88E1548_TEST_MODE_3;
            break;
        case 4:
            printf("Test Mode 4 - Transmit Distortion Test\n");
            val = MRVL_88E1548_TEST_MODE_4;
            break;
        default :
            printf("Not support this test mode\n");
            return (FAILED);
    }

    /* Switch Mode */
    if (woodlawn_phy_reg_wr(bus_id, phy_addr, MRVL_88E1548_TEST_MODE_REG, val) == FAILED) {
        printf("Alter port %d fails\n", port_num);
        return (FAILED);
    }

    return (PASSED);
}

/***********************************************************************************
 * Function    : force_led_on
 *    
 * Description : Force turn on led
 *      
 * Inputs      : None
 *        
 * Outputs     : PASSED / FAILED
 *          
 **********************************************************************************/
int force_led_on (void)
{
    printf("Force turn on LED.\n");

    int phy_addr, port_num, led_mode, function_reg_val, port_map;
    int sku_id, bus_id;
    
    sku_id = get_sku_id();

    led_mode = gethex_answer("Choose LED mode(0-copper, 1-fiber)", 0, 0, 0x1);

    /* Alter register with new value */
    if (led_mode == 0) {
        function_reg_val = MRVL_88E1548_FORCE_GE_LED_ON;
        port_num = gethex_answer("Enter GE port number(GE0~GE5)", 0, 0, 0x5);
        port_map = get_phy_port(sku_id, port_num);
    } else {
        function_reg_val = MRVL_88E1548_FORCE_SFP_LED_ON;
        port_num = gethex_answer("Enter SFP port number(SFP0~SFP5)", 0, 0, 0x5);
        port_map = get_phy_port(sku_id, port_num);
    }

    phy_addr = get_88e1548_phy_addr(port_num, port_map);

    bus_id = get_88e1548_bus_id(phy_addr);
    
    /* Need to write page register for set page */
    if (woodlawn_phy_reg_wr(bus_id, phy_addr, MRV88E1548L_PAGE_ADDRESS_REG, 
        MRVL_88E1548_LED_CTRL_PAGE)
        == FAILED) {
        printf("Set Register Page %d Fail !\n", MRVL_88E1548_LED_CTRL_PAGE);
        return (FAILED);
    }

    /* Force LED on */
    if (woodlawn_phy_reg_wr(bus_id, phy_addr, MRVL_88E1548_LED_POLARITY_REG, 
        MRVL_88E1548_LED_POLARITY) == FAILED) {
        printf("Alter port %d polarity fails\n", port_num);
        return (FAILED);
    }

    if (woodlawn_phy_reg_wr(bus_id, phy_addr, MRVL_88E1548_LED_FUNCTION_REG, 
        function_reg_val) == FAILED) {
        printf("Alter port %d function fails\n", port_num);
        return (FAILED);
    }
    
    return (PASSED);
}

/*******************************************************************************
 *  
 * Function    : force_led_off
 *    
 * Description : Force turn off led
 *      
 * Inputs      : None
 *        
 * Outputs     : PASSED / FAILED
 *          
 **********************************************************************************/
int force_led_off (void)
{
    int sku_id;
    printf("Force turn off LED.\n");

    sku_id = get_sku_id();

    int phy_addr, port_num, led_mode, function_reg_val, port_map;
    int bus_id;

    led_mode = gethex_answer("Choose LED mode(0-copper, 1-fiber)", 0, 0, 0x1);

    /* Alter register with new value */
    if (led_mode == 0) {
        function_reg_val = MRVL_88E1548_FORCE_GE_LED_OFF;
        port_num = gethex_answer("Enter GE port number(GE0~GE5)", 0, 0, 0x5);
        port_map = get_phy_port(sku_id, port_num);
    } else {
        function_reg_val = MRVL_88E1548_FORCE_SFP_LED_OFF;
        port_num = gethex_answer("Enter SFP port number(SFP0~SFP5)", 0, 0, 0x5);
        port_map = get_phy_port(sku_id, port_num);
    }
    
    phy_addr = get_88e1548_phy_addr(port_num, port_map);

    bus_id = get_88e1548_bus_id(phy_addr);
    
    /* Need to write page register for set page */
    if (woodlawn_phy_reg_wr(bus_id, phy_addr, MRV88E1548L_PAGE_ADDRESS_REG, 
        MRVL_88E1548_LED_CTRL_PAGE)
        == FAILED) {
        printf("Set Register Page %d Fail !\n", MRVL_88E1548_LED_CTRL_PAGE);
        return (FAILED);
    }

    /* Force LED on */
    if (woodlawn_phy_reg_wr(bus_id, phy_addr, MRVL_88E1548_LED_POLARITY_REG, 
        MRVL_88E1548_LED_POLARITY) == FAILED) {
        printf("Alter port %d polarity fails\n", port_num);
        return (FAILED);
    }

    if (woodlawn_phy_reg_wr(bus_id, phy_addr, MRVL_88E1548_LED_FUNCTION_REG, 
        function_reg_val) == FAILED) {
        printf("Alter port %d function fails\n", port_num);
        return (FAILED);
    }
    
    return (PASSED);
}

/*******************************************************************************
 *  
 * Function    : force_led_blink
 *    
 * Description : Force led blink
 *      
 * Inputs      : None
 *        
 * Outputs     : PASSED / FAILED
 *         
 **********************************************************************************/
int force_led_blink (void)
{
    int sku_id;
    printf("Force LED blink.\n");

    sku_id = get_sku_id();

    int phy_addr, port_num, led_mode, function_reg_val, port_map;
    int bus_id;

    led_mode = gethex_answer("Choose LED mode(0-copper, 1-fiber)", 0, 0, 0x1);

    /* Alter register with new value */
    if (led_mode == 0) {
        function_reg_val = MRVL_88E1548_FORCE_GE_LED_BLINK;
        port_num = gethex_answer("Enter GE port number(GE0~GE5)", 0, 0, 0x5);
        port_map = get_phy_port(sku_id, port_num);
    } else {
        function_reg_val = MRVL_88E1548_FORCE_SFP_LED_BLINK;
        port_num = gethex_answer("Enter SFP port number(SFP0~SFP5)", 0, 0, 0x5);
        port_map = get_phy_port(sku_id, port_num);
    }
    
    phy_addr = get_88e1548_phy_addr(port_num, port_map);

    bus_id = get_88e1548_bus_id(phy_addr);
    
    /* Need to write page register for set page */
    if (woodlawn_phy_reg_wr(bus_id, phy_addr, MRV88E1548L_PAGE_ADDRESS_REG, 
        MRVL_88E1548_LED_CTRL_PAGE)
        == FAILED) {
        printf("Set Register Page %d Fail !\n", MRVL_88E1548_LED_CTRL_PAGE);
        return (FAILED);
    }

    /* Force LED blink */
    if (woodlawn_phy_reg_wr(bus_id, phy_addr, MRVL_88E1548_LED_POLARITY_REG, 
        MRVL_88E1548_LED_POLARITY) == FAILED) {
        printf("Alter port %d polarity fails\n", port_num);
        return (FAILED);
    }

    if (woodlawn_phy_reg_wr(bus_id, phy_addr, MRVL_88E1548_LED_FUNCTION_REG, 
        function_reg_val) == FAILED) {
        printf("Alter port %d function fails\n", port_num);
        return (FAILED);
    }

    return (PASSED);
}

/*******************************************************************************
 *  
 * Function    : led_speed_blink
 *    
 * Description : Make LED blink according to the test speed
 *      
 * Inputs      : None
 *        
 * Outputs     : PASSED / FAILED
 *          
 *********************************************************************************/
int led_speed_blink (void)
{
    int sku_id;
    printf("LED speed blink.\n");

    sku_id = get_sku_id();

    int phy_addr, port_num, led_mode, port_map;
    int bus_id;
    
    led_mode = gethex_answer("Choose LED mode(0-copper, 1-fiber)", 0, 0, 0x1);

    /* Alter register with new value */
    if (led_mode == 0) {
        port_num = gethex_answer("Enter GE port number(GE0~GE5)", 0, 0, 0x5);
        port_map = get_phy_port(sku_id, port_num);
    } else {
        port_num = gethex_answer("Enter SFP port number(SFP0~SFP5)", 0, 0, 0x5);
        port_map = get_phy_port(sku_id, port_num);
    }
    
    phy_addr = get_88e1548_phy_addr(port_num, port_map);

    bus_id = get_88e1548_bus_id(phy_addr);
    
    /* Need to write page register for set page */
    if (woodlawn_phy_reg_wr(bus_id, phy_addr, MRV88E1548L_PAGE_ADDRESS_REG, 
        MRVL_88E1548_LED_CTRL_PAGE)
        == FAILED) {
        printf("Set Register Page %d Fail !\n", MRVL_88E1548_LED_CTRL_PAGE);
        return (FAILED);
    }

    /* LED speed blink */
    if (woodlawn_phy_reg_wr(bus_id, phy_addr, MRVL_88E1548_LED_POLARITY_REG, 
        MRVL_88E1548_LED_POLARITY) == FAILED) {
        printf("Alter port %d polarity fails\n", port_num);
        return (FAILED);
    }

    if (woodlawn_phy_reg_wr(bus_id, phy_addr, MRVL_88E1548_LED_FUNCTION_REG, 
        MRVL_88E1548_LED_SPEED_BLINK) == FAILED) {
        printf("Alter port %d function fails\n", port_num);
        return (FAILED);
    }
    return (PASSED);
}

/*******************************************************************************
 *  
 * Function    : has_second_phy
 *    
 * Description : Judge whether have second phy
 *      
 * Inputs      : None
 *        
 * Outputs     : TRUE / FALSE
 *          
 *********************************************************************************/
static int has_second_phy (void)
{
    int id;

    id = get_sku_id();

    if (id == WOODLAWN_4GE_1XAUI) {
        return (FALSE);
    } else {
        return (TRUE);
    }
}

/*------------------------------------------------------------------
 *
 * Function: print_input_msg
 * Description: Print out the input port information.
 *
 * Input:  *port_num - the port number
 *         *reg_addr - register address
 *         *page_no - page number
 *         sku_id - sku number
 *
 * Output: PASSED/FAILED
 *
 *------------------------------------------------------------------
 */
static void print_input_msg (int *port_num, int *reg_addr, int *page_no,
                             int sku_id)
{
    if (sku_id == WOODLAWN_6GE_1XAUI) {
        printf("Enter GE/SFP port number\n");
        printf("Enter 0 - GE0/SFP3\n");
        printf("Enter 1 - GE1/SFP2\n");
        printf("Enter 2 - GE2/SFP1\n");
        printf("Enter 3 - GE3/SFP0\n");
        printf("Enter 4 - GE4/SFP5\n");
        printf("Enter 5 - GE5/SFP4\n");
        printf("Enter 6 - PORT2(PHY1)\n");
        printf("Enter 7 - PORT3(PHY1)\n");
    } else {
        /* Official SKUs */
        if (sku_id == WOODLAWN_6GE) {
            printf("Enter GE/SFP port number\n");
            printf("Enter 0 - GE0/SFP0(PHY1)\n");
            printf("Enter 1 - GE1/SFP1(PHY1)\n");
            printf("Enter 2 - GE2/SFP2\n");
            printf("Enter 3 - GE3/SFP3\n");
            printf("Enter 4 - GE4/SFP4\n");
            printf("Enter 5 - GE5/SFP5\n");
        } else if (sku_id == WOODLAWN_4GE_1XAUI) {
            printf("Enter GE/SFP port number\n");
            printf("Enter 0 - GE0/SFP0\n");
            printf("Enter 1 - GE1/SFP1\n");
            printf("Enter 2 - GE2/SFP2\n");
            printf("Enter 3 - GE3/SFP3\n");
        }

    }

    *port_num = gethex_answer("Select port", 0, 0, 0x7);
    *reg_addr = gethex_answer("Enter reg offset", 0, 0, 0xff);
    *page_no = gethex_answer("Enter page number", 0, 0, 0xff);
}

/*-------------------------------------------------
 * $Log: diag_ge_phy_88E1548L_test.c,v $
 * Revision 1.3  2015/02/14 12:48:41  kodko
 * Collapse timing card branch code into main trunk.
 *
 * Revision 1.2.8.4  2014/05/13 02:23:37  kodko
 * Change TOD trigger mode from update to capture.
 *
 * Revision 1.2.8.3  2014/05/05 07:27:34  kodko
 * Add comment for config gen clk/trig function.
 *
 * Revision 1.2.8.2  2014/04/30 13:47:22  kodko
 * Support 1548P/2222P clock/trigger in/out verification.
 *
 * Revision 1.2.8.1  2014/03/11 02:25:55  leschen
 * Add 1588 clk/trig verification and r/w 1588 register utility.
 *
 * Revision 1.2  2013/10/08 08:48:28  tirawan
 * Woodlawn collapsed to main trunk
 *
 * Revision 1.1.4.4  2013/09/06 01:23:30  leschen
 * Remove PTP lpbk test from do all warpper.
 *
 * Revision 1.1.4.3  2013/09/05 06:15:13  leschen
 * Add 10M pseudorandom packet utility
 *
 * Revision 1.1.4.2  2013/08/20 10:58:52  tirawan
 * Branch into woodlawn-branch2 and port woodlawn code
 *
 * Revision 1.1.2.5  2013/08/06 09:42:09  leschen
 * Add PTP test into 88e1548 do all wraper
 *
 * Revision 1.1.2.4  2013/08/06 09:33:11  leschen
 * Add PTP engine init function.
 *
 * Revision 1.1.2.3  2013/06/17 11:05:19  leschen
 * Remove static declaration from alter and dump utility
 *
 * Revision 1.1.2.2  2013/06/13 11:42:44  tirawan
 * Implement LED nc dispatch command for host side to be able to control SM LED
 *
 * Revision 1.1.2.1  2013/04/24 10:37:17  tirawan
 * Initial check-in for woodlawn linux code
 *
 * Revision 1.7  2013/04/10 09:48:48  kuangik
 * Implement do all wrapper for host run sm test
 *
 * Revision 1.6  2013/03/29 09:45:28  leslie
 * Add comment to each function
 *
 * Revision 1.5  2013/03/27 04:49:35  kuangik
 * Code cleanup after adding -Wall
 *
 * Revision 1.25  2013/03/12 11:24:03  leslie
 * Fix submenu flag for show error number message
 *
 * Revision 1.24  2013/03/08 09:34:07  kuangik
 * Clear all warning
 *
 * Revision 1.22  2013/02/19 08:38:22  leslie
 * Useing lib to replace redundant code
 *
 * Revision 1.21  2013/02/18 07:51:32  leslie
 * Pass argument bus id to woodlawn_phy_reg_wr and woodlawn_phy_reg_rd lib
 *
 * Revision 1.20  2013/02/18 06:47:11  kody
 * Modify for the port mapping changed according to the new SKUs.
 *
 * Revision 1.19  2013/01/16 01:18:05  leslie
 * Add function to judge whether have second PHY.
 *
 * Revision 1.17  2012/12/11 01:56:15  leslie
 * Fix for fiber 88E1548L platform loopback test.
 *
 * Revision 1.16  2012/11/19 02:33:34  leslie
 * Fix phy led speed blink utility.
 *
 * Revision 1.15  2012/10/24 10:36:09  leslie
 * Combination of the phy 0 and phy 1 to run same test item and clean up code.
 *
 * Revision 1.14  2012/10/18 12:55:02  kody
 * Add 88E1548L fiber line loopback between platform side.
 *
 * Revision 1.13  2012/10/08 09:53:07  leslie
 * Add LED utility and fix the input port to GE port.
 *
 * Revision 1.12  2012/09/21 10:55:15  leslie
 * Fix the dump/alter register and add test mode lib.
 *
 * Revision 1.11  2012/09/05 22:53:59  kody
 * Fix the dump register.
 *
 * Revision 1.10  2012/08/30 13:24:49  leslie
 * Keep page 0 and 8 to do register test.
 *
 * Revision 1.9  2012/08/30 01:29:05  kody
 * Fix register test semgmentation fault and register mask error issues.
 *
 * Revision 1.8  2012/08/27 06:48:33  evanli
 * Writing page address before R/W page
 *
 * Revision 1.7  2012/08/03 10:16:55  evanli
 * Mapping to latest O2 source code on 20120726
 *
 * Revision 1.5  2012/06/12 03:16:49  leslie
 * Add 88E1548 tables and Update the functions
 *
 * Revision 1.4  2012/05/18 10:19:40  kody
 * Fix the type warning during compile.
 *
 * Revision 1.3  2012/04/06 06:07:18  kuangik
 * Update for 88E1548 Test Item
 *
 * Revision 1.2  2012/02/13 03:31:58  leslie
 * Add function prototype.
 *
 * Revision 1.1  2012/02/10 06:59:52  leslie
 * Add Woodlawn phy 88E1548L test.
 *
 *
 * $Endlog$
 *-------------------------------------------------
 */
