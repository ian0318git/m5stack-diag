/* $Id: diag_mv1514_test.c,v 1.2 2015/05/25 03:59:15 steja Exp $ 
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/skye_linux/src/diag_mv1514_test.c,v $
 *-----------------------------------------------------------------------------
 * 
 * diag_mv1514_test.c - Menu for Skye TLk10232
 * 
 * Ported from Woodlawn Project
 * May 2013, Steja
 * Copyright (c) 2013 ~ 2015 by Cisco Systems, Inc.
 * All rights reserved.
 * 
 *-----------------------------------------------------------------------------
 */
  
#include "common.h"
#include "types.h"
#include "menu.h"
#include "defs.h"
#include "error.h"
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include "common_utils.h"
#include "diag_mv1514_test.h"
#include "skye_eth.h"
#include "skye_ext_lpbk.h"
#include "diag_common_drv.h"
#include "proto.h"
#include "nvmonvars.h"
#include "platform_fru.h"


/*****************************************************************************
 *  Functions Declaration
 *****************************************************************************/
int mv1514_utility (int);
int mv1514_led_test(void);
int led_function_on(int);
int led_function_off(int);
int led_blink_function_on(int);
int led_polarity_test (int mode, int polarity, int enable);
static int force_linkup(boolean);
static int mv1514_register_test(void);
static int mv1514_internal_lpbk_test(void);
static int mv1514_external_lpbk_test(void);
static int dump_phy_88E1514_registers(void);
static int alter_phy_88E1514_register(void);
static int dev_88E1514_power_up(uint);
static int auto_negotiation(boolean);
int dump_phy_88E1514_pg0_registers(void);

#define MF_1    (MF_CONTINUOUS | MF_DOGRP)
#define MF_2    (MF_1 | MF_DOALL)
#define MF_3    (MF_2 | MF_SHOW_ERRCOUNT)
#define MF_4    (MF_1 | MF_SHOW_ERRCOUNT)

/*****************************************************************************
 *  For Register Test Use
 *****************************************************************************/
#ifdef DEBUG
static reg_info_t mrvl_1512_cmn_reg_tbl[] = {
    {"PHY Identifier 1", MRV88E1512_PHYID_1_REG, READ_ONLY | SAVE_RESTORE |
            REG_ACCESS, {2}, 0xFFFF, MRV88E1512_PHY_ID_1_VALUE},
    {"PHY Identifier 2", MRV88E1512_PHYID_2_REG, READ_ONLY | SAVE_RESTORE |
            REG_ACCESS, {2}, 0xFFFF, MRV88E1512_PHY_ID_2_VALUE},
    {"Page Address", MRV88E1512_PAGE_ADDRESS_REG, READ_WRITE | SAVE_RESTORE |
            REG_ACCESS, {2}, 0x00FF, 0x0000},
    {"End of Common registers page", 0, 0, {0}, 0, 0},
};
#endif

/* Page 0 - Copper */
static reg_info_t mrvl_1512_p0_reg_tbl[] = {
    {"Control Register", MRV88E1512C_CONTROL_REG,
    READ_WRITE | SAVE_RESTORE | REG_ACCESS, {2},
    0x7540, 0x1140},
    {"Status Register", MRV88E1512C_STATUS_REG,
    READ_ONLY | SAVE_RESTORE | REG_ACCESS, {2},
    0x0000, 0x7949},
    {"Auto-Negotiation Advertisement Register", MRV88E1512C_AUTONEG_ADVR_REG,
    READ_WRITE | SAVE_RESTORE | REG_ACCESS, {2},
    0xBFFF, 0x01e1},
    {"Link Partner Ability Register - Base Page", MRV88E1512C_LINK_PART_AV_REG,
    READ_ONLY | SAVE_RESTORE | REG_ACCESS, {2},
    0x0000, 0x0000},
    {"Auto-Negotiation Expansion Register", MRV88E1512C_AUTONEG_EXPANSION_REG,
    READ_ONLY | SAVE_RESTORE | REG_ACCESS, {2},
    0x0000, 0x0004},
    {"Next Page Transmit Register", MRV88E1512C_NEXT_PAGE_REG,
    READ_WRITE | SAVE_RESTORE | REG_ACCESS, {2},
    0xB7FF, 0x2001},
    {"Link Partner Next Page Register", MRV88E1512C_LP_NEXT_PAGE_REG,
    READ_ONLY | SAVE_RESTORE | REG_ACCESS, {2},
    0x0000, 0x0000},
    {"1000BASE-T Control Register", MRV88E1512C_1000B_CNTL_REG,
    READ_WRITE | SAVE_RESTORE | REG_ACCESS, {2},
    0xFFFF, 0x0300},
    {"1000BASE-T Status Register", MRV88E1512C_1000B_STATUS_REG,
    READ_ONLY | SAVE_RESTORE | REG_ACCESS, {2},
    0x0000, 0x0000},
    {"Extended Status Register", MRV88E1512C_EXTENDED_STATUS_REG,
    READ_ONLY | SAVE_RESTORE | REG_ACCESS, {2},
    0x0000, 0x3000},
    {"Specific Control Register 1", MRV88E1512C_SPECIFIC_CONTROL1_REG,
    READ_WRITE | SAVE_RESTORE | REG_ACCESS, {2},
    0x7F7F, 0x3060},
    {"Specific Status Register 1", MRV88E1512C_SPECIFIC_STATUS1_REG,
    READ_ONLY | SAVE_RESTORE | REG_ACCESS, {2},
    0x0000, 0x8040},
    {"Interrupt Enable Register", MRV88E1512C_SPECIFIC_INT_ENABLE_REG,
    READ_WRITE | SAVE_RESTORE | REG_ACCESS, {2},
    0xFFFF, 0x0000},
    {"Specific Status Register 2", MRV88E1512C_INT_STATUS_REG,
    READ_ONLY | SAVE_RESTORE | REG_ACCESS, {2},
    0x0000, 0x0040},
    {"Specific Control Register 2", MRV88E1512C_SPECIFIC_CONTROL2_REG,
    READ_WRITE | SAVE_RESTORE | REG_ACCESS, {2},
    0xFFFF, 0x0020},
    {"Receive Error Counter Register", MRV88E1512C_REC_ERROR_COUNTER_REG,
    READ_ONLY | SAVE_RESTORE | REG_ACCESS, {2},
    0x0000, 0x0000},
    {"End of Page 0 registers", 0, 0, {0}, 0, 0},
};

/* Page 1 - Fiber */
    reg_info_t mrvl_1512_p1_reg_tbl[] = {
    {"Control Register", MRV88E1512F_CONTROL_REG,
    READ_WRITE | SAVE_RESTORE | REG_ACCESS, {2},
    0x5900, 0x0140},
    {"Status Register", MRV88E1512F_STATUS_REG,
    READ_ONLY | SAVE_RESTORE | REG_ACCESS, {2},
    0x0000, 0x0149},
    {"Auto-Negotiation Advertisement Register", MRV88E1512F_AUTONEG_ADVR_REG,
    READ_ONLY | SAVE_RESTORE | REG_ACCESS, {2},
    0xB1FF, 0x0060},
    {"Link Partner Ability Register - Base Page", MRV88E1512F_LINK_PART_AV_REG,
    READ_ONLY | SAVE_RESTORE | REG_ACCESS, {2},
    0x0000, 0x0000},
    {"Auto-Negotiation Expansion Register", MRV88E1512F_AUTONEG_EXPANSION_REG,
    READ_ONLY | SAVE_RESTORE | REG_ACCESS, {2},
    0x000F, 0x0004},
    {"Next Page Transmit Register", MRV88E1512F_NEXT_PAGE_REG,
    READ_WRITE | SAVE_RESTORE | REG_ACCESS, {2},
    0xB7FF, 0x2001},
    {"Link Partner Next Page Register", MRV88E1512F_LP_NEXT_PAGE_REG,
    READ_ONLY | SAVE_RESTORE | REG_ACCESS, {2},
    0x0000, 0x0000},
    {"Extended Status Register", MRV88E1512F_EXTENDED_STATUS_REG,
    READ_ONLY | SAVE_RESTORE | REG_ACCESS, {2},
    0x0000, 0xC000},
    {"Specific Control Register 1", MRV88E1512F_SPECIFIC_CONTROL1_REG,
    READ_WRITE | SAVE_RESTORE | REG_ACCESS, {2},
    0xFFFC, 0x420D},
    {"Specific Status Register 1", MRV88E1512F_SPECIFIC_STATUS_REG,
    READ_ONLY | SAVE_RESTORE | REG_ACCESS, {2},
    0x0000, 0x8010},
    {"Interrupt Enable Register", MRV88E1512F_INT_ENABLE_REG,
    READ_WRITE | SAVE_RESTORE | REG_ACCESS, {2},
    0x7FB0, 0x0000},
    {"Interrupt Status Register", MRV88E1512F_INT_STATUS_REG,
    READ_ONLY | SAVE_RESTORE | REG_ACCESS, {2},
    0x3F10, 0x0000},
    {"Receive Error Counter Register", MRV88E1512F_REC_ERROR_COUNTER_REG,
    READ_ONLY | SAVE_RESTORE | REG_ACCESS, {2},
    0x0000, 0x0000},
    {"PRBS Control", MRV88E1512F_PRBS_CONTROL_REG,
    READ_WRITE | SAVE_RESTORE | REG_ACCESS, {2},
    0xFF1F, 0x0000},
    {"PRBS Error Counter LSB", MRV88E1512F_PRBS_ERROR_CONNTER_LSB_REG,
    READ_ONLY | SAVE_RESTORE | REG_ACCESS, {2},
    0x0000, 0x0000},
    {"PRBS Error Counter MSB", MRV88E1512F_PRBS_ERROR_CONNTER_MSB_REG,
    READ_ONLY | SAVE_RESTORE | REG_ACCESS, {2},
    0x0000, 0x0000},
    {"PRBS Error Counter MSB", MRV88E1512F_SPECIFIC_CONTROL2_REG,
    READ_WRITE | SAVE_RESTORE | REG_ACCESS, {2},
    0xFFDF, 0x0042},
    {"End of Page 1 registers", 0, 0, {0}, 0, 0},
};

/* Page 2 - MAC */
    reg_info_t mrvl_1512_p2_reg_tbl[] = {
    {"Specific Control Register 1", MRV88E1512M_SPECIFIC_CONTROL1_REG,
    READ_WRITE | SAVE_RESTORE | REG_ACCESS, {2},
    0xFFFF, 0x4448},
    {"Specific Interrupt Enable Register", MRV88E1512M_SPECIFIC_INT_ENABLE_REG,
    READ_WRITE | SAVE_RESTORE | REG_ACCESS, {2},
    0xFFFF, 0x0000},
    {"Specific Status Register", MRV88E1512M_SPECIFIC_STATUS_REG,
    READ_ONLY | SAVE_RESTORE | REG_ACCESS, {2},
    0x0000, 0x0000},
    {"Copper RX_ER Byte Capture Register", MRV88E1512M_COPPER_RXER_BYTE_CAPTURE_REG,
    READ_WRITE | SAVE_RESTORE | REG_ACCESS, {2},
    0x0000, 0x0000},
    {"Specific Control Register 2", MRV88E1512M_SPECIFIC_CONTROL2_REG,
    READ_WRITE | SAVE_RESTORE | REG_ACCESS, {2},
    0xFFFF, 0x1076},
    {"RGMII Output Impedance Calibration Override", MRV88E1512M_RGMII_OUT_IMP_CALI_OV_REG,
    READ_WRITE | SAVE_RESTORE | REG_ACCESS, {2},
    0x2FCF, 0x6505},
    {"RGMII Output Impedance Target", MRV88E1512M_RGMII_OUT_IMP_TARGET_REG,
    READ_WRITE | SAVE_RESTORE | REG_ACCESS, {2},
    0xFFFF, 0x0013},
    {"End of Page 2 registers", 0, 0, {0}, 0, 0},
};
#ifdef DEBUG
/* Page 6 - Miscellaneous */
static reg_info_t mrvl_1512_p6_reg_tbl[] = {
    {"Packet Generation", MRV88E1512_PACKET_GEN_REG,
    READ_WRITE | SAVE_RESTORE | REG_ACCESS, {2},
    0xFFFF, 0x0000},
    {"CRC Counters", MRV88E1512_CRC_COUNTER_REG,
    READ_ONLY | SAVE_RESTORE | REG_ACCESS, {2},
    0x0000, 0x0000},
    {"Checker Control", MRV88E1512_CRC_CHKR_REG,
    READ_WRITE | SAVE_RESTORE | REG_ACCESS, {2},
    0xFFE7, 0x0000},
    {"General Control Register", MRV88E1512_GEN_CONTROL_REG,
    READ_WRITE | SAVE_RESTORE | REG_ACCESS, {2},
    0xFFFF, 0x0200},
    {"End of Page 6 registers", 0, 0, {0}, 0, 0},
};

/* Page 18 - Miscellaneous*/
static reg_info_t mrvl_1512_p18_reg_tbl[] = {
    {"EEE Buffer Control Register 1", MRV88E1512_EEE_Buffer_CONTROL1_REG,
    READ_WRITE | SAVE_RESTORE | REG_ACCESS, {2},
    0xFFFF, 0x0c00},
    {"EEE Buffer Control Register 2", MRV88E1512_EEE_Buffer_CONTROL2_REG,
    READ_ONLY | SAVE_RESTORE | REG_ACCESS, {2},
    0xFFFF, 0x111e},
    {"EEE Buffer Control Register 3", MRV88E1512_EEE_Buffer_CONTROL3_REG,
    READ_WRITE | SAVE_RESTORE | REG_ACCESS, {2},
    0xFFFF, 0x111e},
    {"Packet Generation Register", MRV88E1512_PACKET_GEN_REG,
    READ_WRITE | SAVE_RESTORE | REG_ACCESS, {2},
    0xFF1F, 0x0000},
    {"CRC Counters Register", MRV88E1512_CRC_COUNTER_REG,
    READ_ONLY | SAVE_RESTORE | REG_ACCESS, {2},
    0x0000, 0x0000},
    {"Checker Control Register", MRV88E1512_CHKR_CONTROL_REG,
    READ_WRITE | SAVE_RESTORE | REG_ACCESS, {2},
    0xFFBF, 0x0000},
    {"General Control Register 1", MRV88E1512_GEN_CONTROL1_REG,
    READ_WRITE | SAVE_RESTORE | REG_ACCESS, {2},
    0x7FFF, 0x0007},
    {"Link Disconnect Count Register", MRV88E1512_Link_DISCON_COUNT_REG,
    READ_WRITE | SAVE_RESTORE | REG_ACCESS, {2},
    0xFF00, 0x0000},
    {"SERDES RX_ER Byte Capture Register", MRV88E1512_SERDES_RX_ER_BYTE_CAP_REG,
    READ_ONLY | SAVE_RESTORE | REG_ACCESS, {2},
    0x0000, 0x0000},
    {"End of Page 18 registers", 0, 0, {0}, 0, 0},
};
#endif

/******************************************************************************
 *  List of Menu used for 88E1514
 *****************************************************************************/
static submenu_xtable_t mv1514_tests_submenu_table[] = {
   {"88E1514 Utility", (type_t(*)())mv1514_utility,   FALSE,
       0, NULL, 0, (type_t(*)())mv1514_utility,   TRUE},
   {"88E1514 Register Test", (type_t(*)())mv1514_register_test,   0,
       MF_3, (type_t(*)())0, 0, (type_t(*)())0,   0},
   {"88E1514 LED Test", (type_t(*)())mv1514_led_test,   0,
       0, (type_t(*)())0, 0, (type_t(*)())0,   0},
   {"88E1514 Internal Loopback Test", (type_t(*)())mv1514_internal_lpbk_test,   0,
       MF_3, (type_t(*)())0, 0, (type_t(*)())0,   0},
   {"88E1514 External Loopback Test", (type_t(*)())mv1514_external_lpbk_test,   0,
       MF_3, (type_t(*)())0, 0, (type_t(*)())0,   0},
};

/******************************************************************************
 *  List of Utilities used for TLK 10232
 *****************************************************************************/
static submenu_xtable_t mv1514_util_items[] = {
    {"Dump 88E1514 Register", (type_t(*)())dump_phy_88E1514_registers, 0, 0, (type_t(*)())0,
     0, (type_t(*)())0, 0},
    {"Alter 88E1514 Register", (type_t(*)())alter_phy_88E1514_register, 0, 0,
     (type_t(*)())0, 0, (type_t(*)())0, 0},
    {"Turn ON LED 0", (type_t(*)())led_function_on, LED0, 0,
     (type_t(*)())0, 0, (type_t(*)())0, 0},
    {"Turn ON LED 1", (type_t(*)())led_function_on, LED1, 0,
     (type_t(*)())0, 0, (type_t(*)())0, 0},
    {"Turn ON LED 2", (type_t(*)())led_function_on, LED2, 0,
     (type_t(*)())0, 0, (type_t(*)())0, 0},
    {"Turn OFF LED 0", (type_t(*)())led_function_off, LED0, 0,
     (type_t(*)())0, 0, (type_t(*)())0, 0},
    {"Turn OFF LED 1", (type_t(*)())led_function_off, LED1, 0,
     (type_t(*)())0, 0, (type_t(*)())0, 0},
    {"Turn OFF LED 2", (type_t(*)())led_function_off, LED2, 0,
     (type_t(*)())0, 0, (type_t(*)())0, 0},
    {"Turn LED 0 Blink", (type_t(*)())led_blink_function_on, LED0, 0,
    (type_t(*)())0, 0, (type_t(*)())0, 0},
    {"Turn LED 1 Blink", (type_t(*)())led_blink_function_on, LED1, 0,
    (type_t(*)())0, 0, (type_t(*)())0, 0},
    {"Turn LED 2 Blink", (type_t(*)())led_blink_function_on, LED2, 0,
    (type_t(*)())0, 0, (type_t(*)())0, 0},
    {"Turn OFF All LED", (type_t(*)())led_function_off, ALL_LED, 0,
    (type_t(*)())0, 0, (type_t(*)())0, 0},
};

/******************************************************************************
 *  Macro Definitions
 *****************************************************************************/
#define MV1514_TESTS_SUBMENU_TABLE_SIZE (sizeof(mv1514_tests_submenu_table) / \
                                                sizeof(submenu_xtable_t))
                                                
#define MV1514_TESTS_UTIL_SIZE (sizeof(mv1514_util_items) / \
                                     sizeof(submenu_xtable_t))
                                     
/******************************************************************************
 *  Global Variable
 *****************************************************************************/
/******************************************************************************
 * Primary & secondary submenu items (filled in from xtable)
 *****************************************************************************/
static mitem_t mv1514_tests_primary_items[MV1514_TESTS_SUBMENU_TABLE_SIZE +
                                                 MAX_BASE_ITEMS];
static mitem_t mv1514_tests_secondary_items[MV1514_TESTS_SUBMENU_TABLE_SIZE +
                                                   MAX_BASE_ITEMS];

/******************************************************************************
 * Primary & secondary utilities menu items (filled in from xtable)
 *****************************************************************************/
static mitem_t mv1514_tests_primary_util_items[MV1514_TESTS_UTIL_SIZE +
                                                      MAX_BASE_ITEMS];
static mitem_t mv1514_tests_secondary_util_items[MV1514_TESTS_UTIL_SIZE +
                                                        MAX_BASE_ITEMS];

/******************************************************************************
 * TLK 10232 Utils submenu
 *****************************************************************************/
menuinfo_t mv1514_util_menu = {
    "88E1514 Utility Menu",
    0,
    (PFT)show_endnote,
    0,
    0,
    mv1514_tests_primary_util_items,
};
menuinfo_t *mv1514_util_menup = &mv1514_util_menu;

menuinfo_t mv1514_subtest_menu = {
    "%s Subtest Menu",
    0,                                /* mtparam added by init_empty_menu */
    (PFT)show_endnote,                /* notes missing WICs in combos */
    0,                                /* use generic prompt */
    0,                                /* size (bumped by add_menu_item() */
    mv1514_tests_primary_items,
};
menuinfo_t *mv1514_submenup = &mv1514_subtest_menu;

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
int
mv1514_test (int show_menu)
{
    build_primary_submenu(mv1514_tests_submenu_table,
                            MV1514_TESTS_SUBMENU_TABLE_SIZE,
                            "88E1514", &mv1514_submenup);
    build_secondary_submenu(mv1514_tests_submenu_table,
                            MV1514_TESTS_SUBMENU_TABLE_SIZE,
                            mv1514_tests_secondary_items);
    if (show_menu) {
        menu(mv1514_submenup, mv1514_tests_secondary_items, '\0' );
    } else {
        menu_exec_doall_diags(mv1514_submenup);
    }
    return (PASSED);
}

/*******************************************************************************
 *
 * Function    : mv1514_utility
 * Description :
 * Inputs      : menu_option - display menu instead of running all 88E1514
 *               tests.
 * Outputs     : PASSED / FAILED
 *
 ******************************************************************************/
int
mv1514_utility (int show_menu)
{
    build_primary_submenu(mv1514_util_items, MV1514_TESTS_UTIL_SIZE,
                          "88E1514 Utilities Menu", &mv1514_util_menup);
    build_secondary_submenu(mv1514_util_items, MV1514_TESTS_UTIL_SIZE,
    		mv1514_tests_secondary_util_items);

    if (show_menu) {
       menu(mv1514_util_menup, mv1514_tests_secondary_util_items, '\0' );
    } else {
       menu_exec_doall_diags(mv1514_util_menup);
       prcomplete(testpass, errcount, (char *)0);
    }

    return (PASSED);
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
 *******************************************************************************/

static int
phy_register_tests (int phy_addr, uint page, const reg_info_t *reg_ptr )
{
    uint32_t ix;
    uint retval, ret_val, save_val, readval;
    uint data, temp, tst_offset;
    int port=0;

    readval = 0;
    retval = PASSED;
    ret_val = PASSED;

    if (is_cpu0() == TRUE) {
        /* Front Panel GE gbe5 */
        port = GE_FP_CPU0_PORT;
    } else {
        /* Front Panel GE gbe2 */
        port = GE_FP_CPU1_PORT;
    }

    /* Need to write page register for set page */
    if (select_phy_page_reg(page)
        == FAILED) {
        printf("Set Register Page %d Fail !\n", page);
        return (FAILED);
    }

    while (reg_ptr->size.size != 0) {

        retval = skye_phy_reg_rd(port, phy_addr, reg_ptr->offset, (int *)&save_val);
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

                retval = skye_phy_reg_wr(port, phy_addr, tst_offset, temp);

                /* Read back */
                if (retval == PASSED) {
                    ret_val = skye_phy_reg_rd(port, phy_addr, tst_offset,
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
                retval = skye_phy_reg_wr(port, phy_addr, tst_offset, temp);

                if (retval == PASSED) {
                    /* Read back */
                    ret_val = skye_phy_reg_rd(port, phy_addr, tst_offset,
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
                retval = skye_phy_reg_wr(port, phy_addr, tst_offset, temp);

                if (retval == PASSED) {
                    /* Read back */
                    ret_val = skye_phy_reg_rd(port, phy_addr, tst_offset,
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
            retval = skye_phy_reg_wr(port, phy_addr, tst_offset, save_val);
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
 * Function: mv1514_register_test
 *
 * Description: This function performs the 88E1514 register test.
 *
 * Inputs      : None
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/
static int
mv1514_register_test (void)
{
    int rc = PASSED;
    uint phy_addr;

    testname("88E1514 Register");
    prpass(testpass, "88E1514 Register Test");
#ifdef SKYE_ENHANCED_ERR_MSG
    /*
     * 1. Subtests of the test function will reuse all variables
     * 2. All variables will be cleared automatically when
     *    entering and leaving each menu item.
     */
    /* Segment 1: PID | Unique_string : slot_info */
    fru_table_offset = SKYE_881514;

    /* Segment 2: Test step captured from prpass */
    /* Segment 3: Failure message captured from cterr */

    /* Segment 4: Components used */
    cterr_add_component("Tilera CPU", "88E1514 Phy Chips");

    /* Segment 5: register and memory dump */
    cterr_add_reg_dump((PFV)dump_phy_88E1514_pg0_registers);

    /* Segment 6: Platform Environment initialized here*/
    cterr_add_env_dump((PFV)skye_dump_volt_margins,
                       (PFV)skye_dump_temps);

    /* Segment 7: Top 3 Debugging Steps */
    cterr_add_debug("Check 88E1514 register dump on segment no.5.",
                    "Check 88E1514 Data specs which bit are not write able.",
                    "Check 88E1514 by manual register read & write.",
                    "Check 88E1514 H/W connection to CPU is ok.");
#endif   /* SKYE_ENHANCED_ERR_MSG */
    phy_addr = PHY_ID_88E1514;

#ifdef DEBUG /* NO USE FOR THIS MOMENT */
        /* Common Reg */
    if (phy_register_tests(phy_addr, MRV88E1512_REG_PAGE_0,
        &mrvl_1512_cmn_reg_tbl[0]) == FAILED) {
        cterr('f', 0, "Register Test on address %#x fails.", phy_addr);
        return (rc);
    }
#endif

    if (phy_register_tests(phy_addr, MRV88E1512_REG_PAGE_0,
        &mrvl_1512_p0_reg_tbl[0]) == FAILED) {
        cterr('f', 0, "Register Test on address %#x fails.", phy_addr);
        return (rc);
    }
    prpass(testpass, "Register Test MRV88E1512_REG_PAGE_0 Pass");

    if (phy_register_tests(phy_addr, MRV88E1512_REG_PAGE_1,
        &mrvl_1512_p1_reg_tbl[0]) == FAILED) {
        cterr('f', 0, "Register Test on address %#x fails.", phy_addr);
        return (rc);
    }
    prpass(testpass, "Register Test MRV88E1512_REG_PAGE_1 Pass");

    if (phy_register_tests(phy_addr, MRV88E1512_REG_PAGE_2,
        &mrvl_1512_p2_reg_tbl[0]) == FAILED) {
        cterr('f', 0, "Register Test on address %#x fails.", phy_addr);
        return (rc);
    }
    prpass(testpass, "Register Test MRV88E1512_REG_PAGE_2 Pass");
#ifdef DEBUG /* NO USE FOR THIS MOMENT */
    if (phy_register_tests(phy_addr, MRV88E1512_REG_PAGE_6,
        &mrvl_1512_p6_reg_tbl[0]) == FAILED) {
        cterr('f', 0, "Register Test on address %#x fails.", phy_addr);
        return (rc);
    }
    prpass(testpass, "Register Test MRV88E1512_REG_PAGE_6 Pass");
    if (phy_register_tests(phy_addr, MRV88E1512_REG_PAGE_18,
        &mrvl_1512_p18_reg_tbl[0]) == FAILED) {
        cterr('f', 0, "Register Test on address %#x fails.", phy_addr);
        return (rc);
    }
    prpass(testpass, "Register Test MRV88E1512_REG_PAGE_18 Pass");
#endif
    prpass(testpass, "88E1514 Register Test All Pass");
    prcomplete(testpass, errcount, (char *)0);
    return (rc);
}


/******************************************************************************
 *
 * Function: mv1514_internal_lpbk_test
 *
 * Description: This function performs the 88E1514 Internal loopback test.
 *
 * Inputs      : None
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/
static int
mv1514_internal_lpbk_test (void)
{
    int port = 0;

    testname("88E1514 Internal Looopback");
    prpass(testpass, "88E1514 Internal Looopback Test");
#ifdef SKYE_ENHANCED_ERR_MSG
    /*
     * 1. Subtests of the test function will reuse all variables
     * 2. All variables will be cleared automatically when
     *    entering and leaving each menu item.
     */
    /* Segment 1: PID | Unique_string : slot_info */
    fru_table_offset = SKYE_881514;

    /* Segment 2: Test step captured from prpass */
    /* Segment 3: Failure message captured from cterr */

    /* Segment 4: Components used */
    cterr_add_component("Tilera CPU", "88E1514 Phy Chips");

    /* Segment 5: register and memory dump */
    cterr_add_reg_dump((PFV)dump_phy_88E1514_pg0_registers);

    /* Segment 6: Platform Environment initialized here*/
    cterr_add_env_dump((PFV)skye_dump_volt_margins,
                       (PFV)skye_dump_temps);

    /* Segment 7: Top 3 Debugging Steps */
    cterr_add_debug("Check 88E1514 register dump on segment no.5.",
                    "Check 88E1514 Data specs which bit are not write able.",
                    "Check 88E1514 by manual register read & write.",
                    "Check 88E1514 H/W connection to CPU is ok.");
#endif   /* SKYE_ENHANCED_ERR_MSG */

    printf("For Internal loopback, skip the 1000Mbps speed test.\n\n");

    if (is_cpu0() == TRUE) {
        /* Front Panel GE gbe5 */
        port = GE_FP_CPU0_PORT;
    } else {
        /* Front Panel GE gbe2 */
        port = GE_FP_CPU1_PORT;
    }

    if (phy_88E1514_initial() == FAILED) {
        cterr('f', 0, "Initial Fail.");
        return (FAILED);
    }

    if (skye_phy_lpbk_test(port, INT_LPBK) == FAILED) {
        cterr('f', 0, "Internal Loopback Fail.");
        return (FAILED);
    }
    prcomplete(testpass, errcount, (char *)0);

    return (PASSED);
}


/******************************************************************************
 *
 * Function: mv1514_external_lpbk_test
 *
 * Description: This function performs the 88E1514 External Loopback test.
 *
 * Inputs      : None
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/
static int
mv1514_external_lpbk_test (void)
{
    int port = 0;

    testname("88E1514 External Looopback");
    prpass(testpass, "88E1514 External Looopback Test");
#ifdef SKYE_ENHANCED_ERR_MSG
    /*
     * 1. Subtests of the test function will reuse all variables
     * 2. All variables will be cleared automatically when
     *    entering and leaving each menu item.
     */
    /* Segment 1: PID | Unique_string : slot_info */
    fru_table_offset = SKYE_881514;

    /* Segment 2: Test step captured from prpass */
    /* Segment 3: Failure message captured from cterr */

    /* Segment 4: Components used */
    cterr_add_component("Tilera CPU", "88E1514 Phy Chips");

    /* Segment 5: register and memory dump */
    cterr_add_reg_dump((PFV)dump_phy_88E1514_pg0_registers);

    /* Segment 6: Platform Environment initialized here*/
    cterr_add_env_dump((PFV)skye_dump_volt_margins,
                       (PFV)skye_dump_temps);

    /* Segment 7: Top 3 Debugging Steps */
    cterr_add_debug("Check 88E1514 register dump on segment no.5.",
                    "Check 88E1514 Data specs which bit are not write able.",
                    "Check 88E1514 by manual register read & write."
                    "Check External loopback cable was plugged in.");
#endif   /* SKYE_ENHANCED_ERR_MSG */

    printf("Warning !\nThis test is need external loopback stub plugged in the front panel GE!\n");

    /*
     * if D_EXT_LOOPBACK is OFF, then just return
     */
    if ((NVRAM)->diagflag & D_EXT_LOOPBACK) {
        printf("Skip External loopback, if don't plug with the Stub!");
        return (PASSED);
    }

    if (is_cpu0() == TRUE) {
        /* Front Panel GE gbe5 */
        port = GE_FP_CPU0_PORT;
    } else {
        /* Front Panel GE gbe2 */
        port = GE_FP_CPU1_PORT;
    }
    /* Initial */
    if (phy_88E1514_initial() == FAILED) {
        cterr('f', 0, "Initial Fail.");
        return (FAILED);
    }

    if (skye_phy_lpbk_test(port, EXT_LPBK) == FAILED) {
        cterr('f', 0, "External Loopback Fail.");
        return (FAILED);
    }
    prcomplete(testpass, errcount, (char *)0);

    return (PASSED);
}


/***********************************************************************
 *
 *  Function: dump_phy_88E1514_registers
 *
 * Description: Display GE PHY registers
 *
 * Inputs: None
 *
 * Outputs: PASSED/FAILED
 *
 *************************************************************************/
static int
dump_phy_88E1514_registers (void)
{
    int reg_addr = 0, phy_addr, page_no, reg_val;
    int ports;

    phy_addr = PHY_ID_88E1514;

    reg_addr = gethex_answer("Enter reg offset", 0, 0, 0xff);
    page_no = gethex_answer("Enter page number", 0, 0, 0xff);

    if (is_cpu0() == TRUE) {
        /* Front Panel GE gbe5 */
        ports = GE_FP_CPU0_PORT;
    } else {
        /* Front Panel GE gbe2 */
        ports = GE_FP_CPU1_PORT;
    }
    /* Front Panel GE gbe5 */

    /* Need to write page register for set page */
    if (select_phy_page_reg(page_no)
        == FAILED) {
        printf("Set Register Page %d Fail !\n", page_no);
        return (FAILED);
    }

    if (skye_phy_reg_rd(ports, phy_addr, reg_addr, &reg_val) == FAILED) {
        printf("Dump port fails\n");
        return (FAILED);
    } else {
        printf("value: reg %#.4x, data %#.4x\n", reg_addr, reg_val);
    }

    return (PASSED);
}

/***********************************************************************
 *
 * Function: alter_phy_88E1514_register
 *
 * Description: Alter GE PHY registers
 *
 * Inputs: None
 *
 * Outputs: PASSED/FAILED
 *
 *************************************************************************/
static int
alter_phy_88E1514_register (void)
{
    int reg_addr = 0,phy_addr, page_no, reg_val, port;

    phy_addr = PHY_ID_88E1514;

    reg_addr = gethex_answer("Enter reg offset", 0, 0, 0xff);
    page_no = gethex_answer("Enter page number", 0, 0, 0xff);

    if (is_cpu0() == TRUE) {
        /* Front Panel GE gbe5 */
        port = GE_FP_CPU0_PORT;
    } else {
        /* Front Panel GE gbe2 */
        port = GE_FP_CPU1_PORT;
    }
    /* Front Panel GE gbe5 */

    /* Need to write page register for set page */
    if (select_phy_page_reg(page_no)
        == FAILED) {
        printf("Set Register Page %d Fail !\n", page_no);
        return (FAILED);
    }

    /* read original register value*/
    if (skye_phy_reg_rd(port, phy_addr, reg_addr, &reg_val) == FAILED) {
        printf("Dump port fails\n");
        return (FAILED);
    } else {
        printf("value: reg %#.4x, data %#.4x\n", reg_addr, reg_val);
    }

    /* Alter register with new value */
    reg_val = gethex_answer("Enter the new data (hex): ", reg_val, 0, 0xFFFF);

    /* Alter the current register with new vlaue */
    if (skye_phy_reg_wr(port, phy_addr, reg_addr, reg_val) == FAILED) {
        printf("Alter port fails\n");
        return (FAILED);
    }

    /* Read the new register value */
    if (skye_phy_reg_rd(port, phy_addr, reg_addr, &reg_val) == FAILED) {
        printf("Dump port fails\n");
        return (FAILED);
    } else {
        printf("value: reg %#.4x, data %#.4x\n", reg_addr, reg_val);
    }

    return (PASSED);
}


/***********************************************************************
 *
 * Function: select_phy_page_reg
 *
 * Description: select page register
 *
 * Inputs: page_no - Page number register
 *
 * Outputs: PASSED/FAILED
 *
 *************************************************************************/
int
select_phy_page_reg (int page_no)
{
    int phy_addr, port;
    /* Check CPU0 or 1 to decide which port GE to test */
    if (is_cpu0() == TRUE) {
        /* Front Panel GE gbe5 */
        port = GE_FP_CPU0_PORT;
    } else {
        /* Front Panel GE gbe2 */
        port = GE_FP_CPU1_PORT;
    }
    phy_addr = PHY_ID_88E1514;
    /* Need to write page register for set page */
    if (skye_phy_reg_wr(port, phy_addr, MRV88E1512_PAGE_ADDRESS_REG, page_no)
        == FAILED) {
        cterr_db_print("Set Register Page %d Fail !\n", page_no);
        return (FAILED);
    }
    return (PASSED);
}


/***********************************************************************
 *
 * Function: reset_phy_88E1514_register
 *
 * Description: Reset GE PHY
 *
 * Inputs: None
 *
 * Outputs: PASSED/FAILED
 *
 *************************************************************************/
int
reset_phy_88E1514_register (void)
{
    int reg_addr,phy_addr, page_no, reg_val, port;
    phy_addr = PHY_ID_88E1514;
    /* Check CPU0 or 1 to decide which port GE to test */
    if (is_cpu0() == TRUE) {
        /* Front Panel GE gbe5 */
        port = GE_FP_CPU0_PORT;
    } else {
        /* Front Panel GE gbe2 */
        port = GE_FP_CPU1_PORT;
    }
    /* Front Panel GE gbe5 */
    page_no = 0; /* page 0 */

    /* Need to write page register for set page */
    if (select_phy_page_reg(page_no)
        == FAILED) {
        printf("Set Register Page %d Fail !\n", page_no);
        return (FAILED);
    }
    /* Copper Control Register Page 0, Register 0 */
    reg_addr = MRV88E1512C_CONTROL_REG;

    /* read original register value*/
    if (skye_phy_reg_rd(port, phy_addr, reg_addr, &reg_val) == FAILED) {
        printf("Dump port fails\n");
        return (FAILED);
    } else {
        if ((NVRAM)->diagflag & D_VERBOSE) { /* Turns debug on */
            printf("value: reg %#.4x, data %#.4x\n", reg_addr, reg_val);
        }
    }
    /* PHY Reset bit 15 */
    reg_val |= MRV88E1512_CONTROL_PHY_RESET;

    /* Need to write page register for set page */
    if (skye_phy_reg_wr(port, phy_addr, reg_addr, reg_val)
        == FAILED) {
        printf("Set Register Page %d Fail !\n", page_no);
        return (FAILED);
    }

    /* Check if phy reset complete */
    msleep(100);

    /* read original register value*/
    if (skye_phy_reg_rd(port, phy_addr, reg_addr, &reg_val) == FAILED) {
        printf("Dump port fails\n");
        return (FAILED);
    } else {
        if ((NVRAM)->diagflag & D_VERBOSE) { /* Turns debug on */
            printf("value: reg %#.4x, data %#.4x\n", reg_addr, reg_val);
        }
    }

    if (reg_val & MRV88E1512_CONTROL_PHY_RESET) {
        if ((NVRAM)->diagflag & D_VERBOSE) { /* Turns debug on */
            printf("value: reg %#.4x, data %#.4x\n", reg_addr, reg_val);
        }
    }

    return (PASSED);
}
/***********************************************************************
 *
 * Function: phy_88E1514_initial
 *
 * Description: 88E1514 needs the following initialization registers after every 
 * HW reset tooperate in SGMII-to-Copper mode.
 * 1. Write Reg 22 = 0x0012
 * 2. Set Reg 20.2:0 = 3'b001
 * 3. Set Reg 20.15 = 1'b1
 * 4. Write Reg 22 = 0x0000
 *
 * Inputs: None
 *
 * Outputs: PASSED/FAILED
 *
 *************************************************************************/
int
phy_88E1514_initial (void)
{
    int reg_addr = 0,phy_addr, page_no = 0, reg_val = 0, port;
    phy_addr = PHY_ID_88E1514;

    /* Check CPU0 or 1 to decide which port GE to test */
    if (is_cpu0() == TRUE) {
        /* Front Panel GE gbe5 */
        port = GE_FP_CPU0_PORT;
    } else {
        /* Front Panel GE gbe2 */
        port = GE_FP_CPU1_PORT;
    }

    /* Poser Down PHY */
    if (dev_88E1514_power_up(DISABLE) == FAILED) {
        printf("Set Register Page %d Fail !\n", page_no);
        return (FAILED);
    }

    msleep(1000);

    /* Poser Enable PHY */
    if (dev_88E1514_power_up(ENABLE) == FAILED) {
        printf("Set Register Page %d Fail !\n", page_no);
        return (FAILED);
    }

    msleep(1000);

    /* Setup 1 */
    /* Need to write page register for set page */
    page_no = PAGE_18; /* page 18 */
    if (select_phy_page_reg(page_no) == FAILED) {
        printf("Set Register Page %d Fail !\n", page_no);
        return (FAILED);
    }
    /* Setup 2 */
    /* read original register value*/
    reg_addr = MRV88E1512_GEN_CONTROL1_REG;/* Reg 20 */
    if (skye_phy_reg_rd(port, phy_addr, reg_addr, &reg_val) == FAILED) {
        printf("Dump port fails\n");
        return (FAILED);
    } else {
        if ((NVRAM)->diagflag & D_VERBOSE) { /* Turns debug on */
            printf("value: reg %#.4x, data %#.4x\n", reg_addr, reg_val);
        }
    }
    reg_val |= BIT_0;
    reg_val &= ~(BIT_1 | BIT_2);

    /* Need to write page register for set page */
    if (skye_phy_reg_wr(port, phy_addr, reg_addr, reg_val) == FAILED) {
        printf("Set Register Page %d Fail !\n", page_no);
        return (FAILED);
    }

    /* Setup 3 */
    reg_val |= BIT_15;

    /* Need to write page register for set page */
    if (skye_phy_reg_wr(port, phy_addr, reg_addr, reg_val) == FAILED) {
        printf("Set Register Page %d Fail !\n", page_no);
        return (FAILED);
    }
    /* Setup 4 */
    page_no = PAGE_0; /* page 0 */
    if (select_phy_page_reg(page_no) == FAILED) {
        printf("Set Register Page %d Fail !\n", page_no);
        return (FAILED);
    }

    return (PASSED);
}

/*******************************************************************************
 *
 * Function: dev_88E1514_power_up
 *
 * This function power up/down Copper I/F on PHY device.
 * Both of the two power down bit(power down = 1, normal operation = 0) 
 * must be changed according to input parameter "enable".
 *  1. Page 0, reg 0, bit 11.
 *  2. Page 0, reg 16, bit 2.
 *
 * Input: enable: 0 - power down, 1 - power up.
 *
 * Output: PASSED/FAILED
 *
 *******************************************************************************
 */
static int dev_88E1514_power_up (uint enable)
{
    int reg_addr = 0,phy_addr, page_no = 0, reg_val = 0, port = 0;
    phy_addr = PHY_ID_88E1514;
    /* Check CPU0 or 1 to decide which port GE to test */
    if (is_cpu0() == TRUE) {
        /* Front Panel GE gbe5 */
        port = GE_FP_CPU0_PORT;
    } else {
        /* Front Panel GE gbe2 */
        port = GE_FP_CPU1_PORT;
    }
    /* 1. Set page 0, reg 0, bit 11. */
    /* Need to write page register for set page */
    page_no = PAGE_0; /* page 0 */
    if (select_phy_page_reg(page_no)
        == FAILED) {
        printf("Set Register Page %d Fail !\n", page_no);
        return (FAILED);
    }
    /* read original register value*/
    reg_addr = MRV88E1512C_CONTROL_REG;/* Reg 0 */
    if (skye_phy_reg_rd(port, phy_addr, reg_addr, &reg_val) == FAILED) {
        printf("Dump port fails\n");
        return (FAILED);
    } else {
        if ((NVRAM)->diagflag & D_VERBOSE) { /* Turns debug on */
            printf("value: reg %#.4x, data %#.4x\n", reg_addr, reg_val);
        }
    }

    if (enable == ENABLE) {
        if (reg_val & MRV88E1512_CONTROL_POWER_DOWN) { /* in power down mode */
            reg_val &= ~MRV88E1512_CONTROL_POWER_DOWN;
            /* Need to write page register for set page */
            if (skye_phy_reg_wr(port, phy_addr, reg_addr, reg_val)
                == FAILED) {
                printf("Set Register Page %d Fail !\n", page_no);
                return (FAILED);
            }
        }
    } else {    /* DISABLE */
       if (!(reg_val & MRV88E1512_CONTROL_POWER_DOWN)) { /* not in power down mode */
            reg_val |= MRV88E1512_CONTROL_POWER_DOWN;
            /* Need to write page register for set page */
            if (skye_phy_reg_wr(port, phy_addr, reg_addr, reg_val)
                == FAILED) {
                printf("Set Register Page %d Fail !\n", page_no);
                return (FAILED);
            }
        }        
    }

    /* 2. Set page 0, reg 16, bit 2. */
    /* read original register value*/
    reg_addr = MRV88E1512C_SPECIFIC_CONTROL1_REG;/* Reg 16 */
    if (skye_phy_reg_rd(port, phy_addr, reg_addr, &reg_val) == FAILED) {
        printf("Dump port fails\n");
        return (FAILED);
    } else {
        if ((NVRAM)->diagflag & D_VERBOSE) { /* Turns debug on */
            printf("value: reg %#.4x, data %#.4x\n", reg_addr, reg_val);
        }
    }

    if (enable == ENABLE) {
        if (reg_val & MRV88E1512_CONTROL1_POWER_DOWN) { /* in power down mode */
            reg_val &= ~MRV88E1512_CONTROL1_POWER_DOWN;
            /* Need to write page register for set page */
            if (skye_phy_reg_wr(port, phy_addr, reg_addr, reg_val)
                == FAILED) {
                printf("Set Register Page %d Fail !\n", page_no);
                return (FAILED);
            }
        }
    } else {    /* DISABLE */
        if (!(reg_val & MRV88E1512_CONTROL1_POWER_DOWN)) { /* not in power down mode */
            reg_val |= MRV88E1512_CONTROL1_POWER_DOWN;
            /* Need to write page register for set page */
            if (skye_phy_reg_wr(port, phy_addr, reg_addr, reg_val)
                == FAILED) {
                printf("Set Register Page %d Fail !\n", page_no);
                return (FAILED);
            }
        }        
    }
    return (PASSED);
}

/***********************************************************************
 *
 * Function: enable_phy_lpbk
 *
 * Description: Enable GE PHY Loopback
 *
 * Inputs: enable - ENABLE / DISABLE
 *
 * Outputs: PASSED/FAILED
 *
 *************************************************************************/
int enable_phy_lpbk (int enable)
{
    int reg_addr,phy_addr, page_no, reg_val, port;
    phy_addr = PHY_ID_88E1514;
    /* Check CPU0 or 1 to decide which port GE to test */
    if (is_cpu0() == TRUE) {
        /* Front Panel GE gbe5 */
        port = GE_FP_CPU0_PORT;
    } else {
        /* Front Panel GE gbe2 */
        port = GE_FP_CPU1_PORT;
    }
    page_no = PAGE_0; /* page 0 */

    /* Need to write page register for set page */
    if (select_phy_page_reg(page_no)
        == FAILED) {
        printf("Set Register Page %d Fail !\n", page_no);
        return (FAILED);
    }
    /* Copper Control Register Page 0, Register 0 */
    reg_addr = MRV88E1512C_CONTROL_REG;

    /* read original register value*/
    if (skye_phy_reg_rd(port, phy_addr, reg_addr, &reg_val) == FAILED) {
        printf("Dump port fails\n");
        return (FAILED);
    } else {
        if ((NVRAM)->diagflag & D_VERBOSE) { /* Turns debug on */
            printf("value: reg %#.4x, data %#.4x\n", reg_addr, reg_val);
        }
    }
    /* Loopback enable bit 14 */
    if (enable)
        reg_val |= MRV88E1512_CONTROL_LOOPBACK;
    else
        reg_val &= (~MRV88E1512_CONTROL_LOOPBACK);

    /* Need to write page register for set page */
    if (skye_phy_reg_wr(port, phy_addr, reg_addr, reg_val)
        == FAILED) {
        printf("Set Register Page %d Fail !\n", page_no);
        return (FAILED);
    }

    /* read original register value*/
    if (skye_phy_reg_rd(port, phy_addr, reg_addr, &reg_val) == FAILED) {
        printf("Dump port fails\n");
        return (FAILED);
    } else {
        if ((NVRAM)->diagflag & D_VERBOSE) { /* Turns debug on */
            printf("value: reg %#.4x, data %#.4x\n", reg_addr, reg_val);
        }
    }

    return (PASSED);
}


/***********************************************************************
 *
 * Function: enable_phy_ext_lpbk
 *
 * Description: Enable GE PHY External Loopback
 *
 * Inputs: enable - ENABLE / DISABLE
 *
 * Outputs: PASSED/FAILED
 *
 *************************************************************************/
int enable_phy_ext_lpbk (int enable)
{
    int reg_addr,phy_addr, page_no, reg_val, port;
    phy_addr = PHY_ID_88E1514;
    /* Check CPU0 or 1 to decide which port GE to test */
    if (is_cpu0() == TRUE) {
        /* Front Panel GE gbe5 */
        port = GE_FP_CPU0_PORT;
    } else {
        /* Front Panel GE gbe2 */
        port = GE_FP_CPU1_PORT;
    }
    page_no = PAGE_6; /* page 6 */

    /* Need to write page register for set page */
    if (select_phy_page_reg(page_no)
        == FAILED) {
        printf("Set Register Page %d Fail !\n", page_no);
        return (FAILED);
    }
    /* Copper Control Register Page 6, Register 18 */
    reg_addr = MRV88E1512_CRC_CHKR_REG;

    /* read original register value*/
    if (skye_phy_reg_rd(port, phy_addr, reg_addr, &reg_val) == FAILED) {
        printf("Dump port fails\n");
        return (FAILED);
    } else {
        if ((NVRAM)->diagflag & D_VERBOSE) { /* Turns debug on */
            printf("value: reg %#.4x, data %#.4x\n", reg_addr, reg_val);
        }
    }
    /* Stub enable bit 3 */
    if (enable)
        reg_val |= MRV88E1512_ENABLE_STUB_TEST;
    else
        reg_val &= (~MRV88E1512_ENABLE_STUB_TEST);

    /* Need to write page register for set page */
    if (skye_phy_reg_wr(port, phy_addr, reg_addr, reg_val)
        == FAILED) {
        printf("Set Register Page %d Fail !\n", page_no);
        return (FAILED);
    }

    return (PASSED);
}


/***********************************************************************
 *
 * Function: phy_lpbk_speed
 *
 * Description: change mode GE PHY Loopback speed
 *
 * Inputs: mode - 10MBPS / 100MBPS / 1000MBPS
 *
 * Outputs: PASSED/FAILED
 *
 *************************************************************************/
int phy_lpbk_speed (int mode)
{
    int reg_addr,phy_addr, page_no, reg_val, port;
    phy_addr = PHY_ID_88E1514;
    /* Check CPU0 or 1 to decide which port GE to test */
    if (is_cpu0() == TRUE) {
        /* Front Panel GE gbe5 */
        port = GE_FP_CPU0_PORT;
    } else {
        /* Front Panel GE gbe2 */
        port = GE_FP_CPU1_PORT;
    }
    page_no = PAGE_2; /* page 2 */

    /* Need to write page register for set page */
    if (select_phy_page_reg(page_no)
        == FAILED) {
        printf("Set Register Page %d Fail !\n", page_no);
        return (FAILED);
    }
    /* Copper Control Register Page 2, Register 21 */
    reg_addr = MRV88E1512M_SPECIFIC_CONTROL2_REG;

    /* read original register value*/
    if (skye_phy_reg_rd(port, phy_addr, reg_addr, &reg_val) == FAILED) {
        printf("Dump port fails\n");
        return (FAILED);
    } else {
        if ((NVRAM)->diagflag & D_VERBOSE) { /* Turns debug on */
            printf("value: reg %#.4x, data %#.4x\n", reg_addr, reg_val);
        }
    }
    /* change speed mode 6,13 */
    switch (mode) {
    case MRV88E1512_SPEED_10BT:
        reg_val |= MRV88E1512_SPEED_10;
        break;
    case MRV88E1512_SPEED_100BT:
        reg_val |= MRV88E1512_SPEED_100;
        break;
    case MRV88E1512_SPEED_1000BT:
        reg_val |= MRV88E1512_SPEED_1000;
        break;
    default:
        printf("Speed mode %d not support\n", mode);
        break;
    }

    /* Need to write page register for set page */
    if (skye_phy_reg_wr(port, phy_addr, reg_addr, reg_val)
        == FAILED) {
        printf("Set Register Page %d Fail !\n", page_no);
        return (FAILED);
    }

    /* PHY reset to take changes effect */
    reset_phy_88E1514_register();

    return (PASSED);
}


/***********************************************************************
 *
 * Function: led_function_test
 *
 * Description: LED GE PHY function test
 *
 * Inputs: mode - LED0 / LED1 / LED2
 *         enable - ENABLE / DISABLE
 *
 * Outputs: PASSED/FAILED
 *
 *************************************************************************/
int led_function_test (int mode, int enable)
{
    int reg_addr,phy_addr, page_no, reg_val, port;
    phy_addr = PHY_ID_88E1514;
    /* Check CPU0 or 1 to decide which port GE to test */
    if (is_cpu0() == TRUE) {
        /* Front Panel GE gbe5 */
        port = GE_FP_CPU0_PORT;
    } else {
        /* Front Panel GE gbe2 */
        port = GE_FP_CPU1_PORT;
    }
    page_no = PAGE_3; /* page 3 */

    /* Need to write page register for set page */
    if (select_phy_page_reg(page_no)
        == FAILED) {
        printf("Set Register Page %d Fail !\n", page_no);
        return (FAILED);
    }

    /* Copper Control Register Page 3, Register 16 */
    reg_addr = MRV88E1512M_LED_FUNCTION_CONTROL_REG;

    /* read original register value*/
    if (skye_phy_reg_rd(port, phy_addr, reg_addr, &reg_val) == FAILED) {
        printf("Dump port fails\n");
        return (FAILED);
    } else {
        if ((NVRAM)->diagflag & D_VERBOSE) { /* Turns debug on */
            printf("#%s:value: reg %#.4x, data %#.4x\n", __FUNCTION__, reg_addr, reg_val);
        }
    }

    /* change led mode page 3, reg 16 */
    switch (mode) {
    case LED0:
        if (enable == SOLID) {
            reg_val &= (~MRV88E1512M_LED_CONFIG_MASK);
            reg_val |= (MRV88E1512M_LED1_FORCE_OFF | MRV88E1512M_LED0_FORCE_ON);
        } else if (enable == BLINK) {
            reg_val &= (~MRV88E1512M_LED_CONFIG_MASK);
            reg_val |= (MRV88E1512M_LED1_FORCE_OFF | MRV88E1512M_LED0_FORCE_BLINK);
        } else {
            reg_val &= (~MRV88E1512M_LED_CONFIG_MASK);
            reg_val |= MRV88E1512M_LED0_FORCE_OFF;
        }
        break;
    case LED1:
        if (enable == SOLID) {
            reg_val &= (~MRV88E1512M_LED_CONFIG_MASK);
            reg_val |= (MRV88E1512M_LED1_FORCE_ON | MRV88E1512M_LED0_FORCE_OFF);
        } else if (enable == BLINK) {
            reg_val &= (~MRV88E1512M_LED_CONFIG_MASK);
            reg_val |= (MRV88E1512M_LED1_FORCE_BLINK | MRV88E1512M_LED0_FORCE_OFF);
        } else {
            reg_val &= (~MRV88E1512M_LED_CONFIG_MASK);
            reg_val |= MRV88E1512M_LED1_FORCE_OFF;
        }
        break;
    case LED2:
        if (enable == SOLID) {
            reg_val &= (~MRV88E1512M_LED_CONFIG_MASK);
            reg_val |= MRV88E1512M_LED2_FORCE_ON;
        } else if (enable == BLINK) {
            reg_val &= (~MRV88E1512M_LED_CONFIG_MASK);
            reg_val |= MRV88E1512M_LED2_FORCE_BLINK;
        } else {
            reg_val &= (~MRV88E1512M_LED_CONFIG_MASK);
            reg_val |= MRV88E1512M_LED2_FORCE_OFF;
        }
        break;
    case ALL_GREEN:
        if (enable == SOLID) {
            reg_val &= (~MRV88E1512M_LED_CONFIG_MASK);
            reg_val |= (MRV88E1512M_LED1_FORCE_ON | MRV88E1512M_LED0_FORCE_OFF);
            reg_val |= MRV88E1512M_LED2_FORCE_ON;
        } else if (enable == BLINK) {
            reg_val &= (~MRV88E1512M_LED_CONFIG_MASK);
            reg_val |= (MRV88E1512M_LED1_FORCE_BLINK | MRV88E1512M_LED0_FORCE_OFF);
            reg_val |= MRV88E1512M_LED2_FORCE_BLINK;
        } else {
            reg_val &= (~MRV88E1512M_LED_CONFIG_MASK);
            reg_val |= MRV88E1512M_LED2_FORCE_OFF | MRV88E1512M_LED1_FORCE_OFF;
        }
        break;
    case ALL_LED:
        if (enable == RST_VAL) {
            reg_val &= (~MRV88E1512M_LED_CONFIG_MASK);
            reg_val |= MRV88E1512M_LED_RST_VALUE;
        }
        break;
    default:
        printf("LED mode %d not support\n", mode);
        break;
    }

    /* Need to write page register for set page */
    if (skye_phy_reg_wr(port, phy_addr, reg_addr, reg_val)
        == FAILED) {
        printf("Set Register Page %d Fail !\n", page_no);
        return (FAILED);
    }

    return (PASSED);
}


/***********************************************************************
 *
 * Function: led_function_on
 *
 * Description: LED GE Turn on
 *
 * Inputs: mode - LED0 / LED1 / LED2
 *
 * Outputs: PASSED/FAILED
 *
 *************************************************************************/
int
led_function_on (int led)
{
    /* Set LED polarity LOW HIGH */
    if (led_polarity_test(led, LOW_HIGH, TRUE)) {
        printf("Set LED polarity failed\n");
        return (FAILED);
    }

    /* LED ON */
    if (led_function_test(led, SOLID)) {
        printf("Set LED failed\n");
        return (FAILED);
    }
    return (PASSED);
}


/***********************************************************************
 *
 * Function: led_function_off
 *
 * Description: LED GE Turn off
 *
 * Inputs: mode - LED0 / LED1 / LED2
 *
 * Outputs: PASSED/FAILED
 *
 *************************************************************************/
int
led_function_off (int led)
{
    /* Set LED polarity LOW HIGH */
    if (led_polarity_test(led, LOW_HIGH, TRUE)) {
        printf("Set LED polarity failed\n");
        return (FAILED);
    }
    /* LED OFF */
    if (led == ALL_LED) {
        if (led_function_test(LED0, FORCE_LED_OFF)) {
            return (FAILED);
        }
        if (led_function_test(LED1, FORCE_LED_OFF)) {
            return (FAILED);
        }
        if (led_function_test(LED2, FORCE_LED_OFF)) {
            return (FAILED);
        }
        if (led_function_test(led, RST_VAL)) {
            return (FAILED);
        }
    } else {
        if (led_function_test(led, FORCE_LED_OFF)) {
            return (FAILED);
        }
    }
    return (PASSED);
}

/***********************************************************************
 *
 * Function: led_blink_function_on
 *
 * Description: LED GE Turn on blink
 *
 * Inputs: mode - LED0 / LED1 / LED2
 *
 * Outputs: PASSED/FAILED
 *
 *************************************************************************/
int
led_blink_function_on (int led)
{
    /* Set LED polarity LOW HIGH */
    if (led_polarity_test(led, LOW_HIGH, TRUE)) {
        printf("Set LED polarity failed\n");
        return (FAILED);
    }
    /* LED ON */
    if (led_function_test(led, BLINK)) {
        return (FAILED);
    }
    return (PASSED);
}

/***********************************************************************
 *
 * Function: mv1514_led_test
 *
 * Description: run mv1514 ge led test
 *
 * Inputs: mode - LED0 / LED1 / LED2
 *
 * Outputs: PASSED/FAILED
 *
 *************************************************************************/
int
mv1514_led_test (void)
{
    printf("\n\nExercise each LED in the following sequence:\n"
           "Off, Green On, Green Blink, Amber On, Amber Blink, Normal.\n");
    printf("\n!!! This is a visual test. User needs to decide whether "
           " the LED works correctly or not !!!\n\n");
    /* LED 0 ON */
    printf("LED 0 (SPEED STATUS 1000Mbps) ON...\n");
    led_function_on(LED0);
    msleep(1000);
    /* Turn LED 0 off */
    printf("LED 0 (SPEED STATUS 1000Mbps) OFF...\n");
    led_function_off (LED0);
    /* LED 1 ON */
    printf("LED 1 (SPEED STATUS 100Mbps) ON...\n");
    led_function_on(LED1);
    msleep(1000);
    /* Turn LED 1 off */
    printf("LED 1 (SPEED STATUS 100Mbps) OFF...\n");
    led_function_off (LED1);
    /* LED 2 ON */
    printf("LED 2 (LINK UP STATUS) ON...\n");
    led_function_on(LED2);
    msleep(1000);
    /* Turn LED 2 off */
    printf("LED 2 (LINK UP STATUS) OFF...\n");
    led_function_off (LED2);
    /* LED ALL GREEN ON */
    printf("LED ALL GREEN ON...\n");
    led_function_on(ALL_GREEN);
    msleep(1000);
    /* Turn LED ALL GREEN OFF */
    printf("LED ALL GREEN OFF...\n");
    led_function_off (ALL_GREEN);
    /* Blink */
    printf("All LED GREEN Blink ON...\n");
    led_blink_function_on(LED1);
    led_blink_function_on(LED2);
    led_blink_function_on(ALL_GREEN);
    msleep(1000);
    /* Turn all LED off */
    printf("All LED GREEN Blink OFF...\n");
    led_function_off (ALL_GREEN);
    msleep(1000);
    /* Turn LED 0 AMBER ON */
    printf("LED 0 AMBER Blink ON...\n");
    led_blink_function_on(LED0);
    msleep(1000);
    /* Turn LED 0 AMBER OFF */
    printf("LED 0 AMBER Blink OFF...\n");
    led_function_off(LED0);
    /* Turn all LED on */
    msleep(1000);
    printf("All LED turn on...\n");
    led_function_on(LED0);
    led_function_on(LED2);
    msleep(1000);
    /* Turn all LED off */
    printf("All LED turn off...\n");
    led_function_off (ALL_LED);
    printf("LED test done...\n");

    return (PASSED);
}

/***********************************************************************
 *
 * Function: led_polarity_test
 *
 * Description: LED GE PHY polarity test
 *
 * Inputs: mode - LED0 / LED1 / LED2
 *         polarity - polarity mode
 *         enable - ENABLE / DISABLE
 *
 * Outputs: PASSED/FAILED
 *
 *************************************************************************/
int
led_polarity_test (int mode, int polarity, int enable)
{
    int reg_addr,phy_addr, page_no, reg_val, port;
    phy_addr = PHY_ID_88E1514;

    if (is_cpu0() == TRUE) {
        /* Front Panel GE gbe5 */
        port = GE_FP_CPU0_PORT;
    } else {
        /* Front Panel GE gbe2 */
        port = GE_FP_CPU1_PORT;
    }

    page_no = 3; /* page 3 */

    /* Need to write page register for set page */
    if (select_phy_page_reg(page_no)
        == FAILED) {
        printf("Set Register Page %d Fail !\n", page_no);
        return (FAILED);
    }
    /* Copper Control Register Page 3, Register 17 */
    reg_addr = MRV88E1512M_LED_POLARITY_CONTROL_REG;

    /* read original register value*/
    if (skye_phy_reg_rd(port, phy_addr, reg_addr, &reg_val) == FAILED) {
        printf("Dump port fails\n");
        return (FAILED);
    } else {
        if ((NVRAM)->diagflag & D_VERBOSE) { /* Turns debug on */
            printf("value: reg %#.4x, data %#.4x\n", reg_addr, reg_val);
        }
    }

    /* change speed mode page 3, reg 17 */
    switch (mode) {
    case LED0:
        if (polarity == HIGH_LOW) {
            if (enable) {
                reg_val &= MRV88E1512M_LED_POLARITY_MASK;
                reg_val |= MRV88E1512M_LED0_POLARITY_HIGH_LOW;
            } else {
                reg_val &= (~MRV88E1512M_LED0_POLARITY_HIGH_LOW);
            }
        } else if (polarity == LOW_TRISTATE) {
            if (enable) {
                reg_val &= MRV88E1512M_LED_POLARITY_MASK;
                reg_val |= MRV88E1512M_LED0_POLARITY_LOW_TRISTATE;
            } else {
                reg_val &= (~MRV88E1512M_LED0_POLARITY_LOW_TRISTATE);
            }
        } else if (polarity == HIGH_TRISTATE) {
            if (enable) {
                reg_val &= MRV88E1512M_LED_POLARITY_MASK;
                reg_val |= MRV88E1512M_LED0_POLARITY_HIGH_TRISTATE;
            } else {
                reg_val &= (~MRV88E1512M_LED0_POLARITY_HIGH_TRISTATE);
            }
        } else {
            if (enable) {
                reg_val &= MRV88E1512M_LED_POLARITY_MASK;
                reg_val |= MRV88E1512M_LED0_POLARITY_LOW_HIGH;
            } else {
                reg_val &= (~MRV88E1512M_LED0_POLARITY_LOW_HIGH);
            }
        }
        break;
    case LED1:
        if (polarity == HIGH_LOW) {
            if (enable) {
                reg_val &= MRV88E1512M_LED_POLARITY_MASK;
                reg_val |= MRV88E1512M_LED1_POLARITY_HIGH_LOW;
            } else {
                reg_val &= (~MRV88E1512M_LED1_POLARITY_HIGH_LOW);
            }
        } else if (polarity == LOW_TRISTATE) {
            if (enable) {
                reg_val &= MRV88E1512M_LED_POLARITY_MASK;
                reg_val |= MRV88E1512M_LED1_POLARITY_LOW_TRISTATE;
            } else {
                reg_val &= (~MRV88E1512M_LED1_POLARITY_LOW_TRISTATE);
            }
        } else if (polarity == HIGH_TRISTATE) {
            if (enable) {
                reg_val &= MRV88E1512M_LED_POLARITY_MASK;
                reg_val |= MRV88E1512M_LED1_POLARITY_HIGH_TRISTATE;
            } else {
                reg_val &= (~MRV88E1512M_LED1_POLARITY_HIGH_TRISTATE);
            }
        } else {
            if (enable) {
                reg_val &= MRV88E1512M_LED_POLARITY_MASK;
                reg_val |= MRV88E1512M_LED1_POLARITY_LOW_HIGH;
            } else {
                reg_val &= (~MRV88E1512M_LED1_POLARITY_LOW_HIGH);
            }
        }
        break;
    case LED2:
        if (polarity == HIGH_LOW) {
            if (enable) {
                reg_val &= MRV88E1512M_LED_POLARITY_MASK;
                reg_val |= MRV88E1512M_LED2_POLARITY_HIGH_LOW;
            } else {
                reg_val &= (~MRV88E1512M_LED2_POLARITY_HIGH_LOW);
            }
        } else if (polarity == LOW_TRISTATE) {
            if (enable) {
                reg_val &= MRV88E1512M_LED_POLARITY_MASK;
                reg_val |= MRV88E1512M_LED2_POLARITY_LOW_TRISTATE;
            } else {
                reg_val &= (~MRV88E1512M_LED2_POLARITY_LOW_TRISTATE);
            }
        } else if (polarity == HIGH_TRISTATE) {
            if (enable) {
                reg_val &= MRV88E1512M_LED_POLARITY_MASK;
                reg_val |= MRV88E1512M_LED2_POLARITY_HIGH_TRISTATE;
            } else {
                reg_val &= (~MRV88E1512M_LED2_POLARITY_HIGH_TRISTATE);
            }
        } else {
            if (enable) {
                reg_val &= MRV88E1512M_LED_POLARITY_MASK;
                reg_val |= MRV88E1512M_LED2_POLARITY_LOW_HIGH;
            } else {
                reg_val &= (~MRV88E1512M_LED2_POLARITY_LOW_HIGH);
            }
        }
        break;
    case ALL_GREEN:
    case ALL_LED:
        break;
    default:
        printf("Speed mode %d not support\n", mode);
        break;
    }


    /* Need to write page register for set page */
    if (skye_phy_reg_wr(port, phy_addr, reg_addr, reg_val)
        == FAILED) {
        printf("Set Register Page %d Fail !\n", page_no);
        return (FAILED);
    }

    return (PASSED);
}

/*------------------------------------------------------------------
 *
 * Function: force_linkup
 *   for internal loopback force PHY link up to
 *   prevent there is no external stub connect to the port
 *   then the linux will let port link down.
 *   all of the internal lpbk test will need to turn
 *   on this function.
 *
 *
 * Input:  onoff - turn on/off
 *         phy_id - phy addr for setup port
 *
 * Output: PASSED/FAILED
 *
 *------------------------------------------------------------------
 */
int force_linkup (boolean onoff)
{
    int reg_addr,phy_addr, page_no, reg_val, port;
    phy_addr = PHY_ID_88E1514;
    /* Check CPU0 or 1 to decide which port GE to test */
    if (is_cpu0() == TRUE) {
        /* Front Panel GE gbe5 */
        port = GE_FP_CPU0_PORT;
    } else {
        /* Front Panel GE gbe2 */
        port = GE_FP_CPU1_PORT;
    }
    reg_val = 1;

    /* go to page 0 */
    page_no = MRV88E1512_REG_PAGE_0;
    /* Need to write page register for set page */
    if (select_phy_page_reg(page_no)
        == FAILED) {
        printf("Set Register Page %d Fail !\n", page_no);
        return (FAILED);
    }

    reg_addr = MRV88E1512C_SPECIFIC_CONTROL1_REG;

    /* read original register value*/
    if (skye_phy_reg_rd(port, phy_addr, reg_addr, &reg_val) == FAILED) {
        printf("Dump port fails\n");
        return (FAILED);
    } else {
        if ((NVRAM)->diagflag & D_VERBOSE) { /* Turns debug on */
            printf("value: reg %#.4x, data %#.4x\n", reg_addr, reg_val);
        }
    }

    /*bit 10 for force link up*/
    if (onoff) {
        /* enable force link up*/
    	reg_val |= MRV88E1512_COP_SPEC_CTRL_REG1_FORCE_LINK;
    } else {
        /* restore force link up*/
    	reg_val &= ~MRV88E1512_COP_SPEC_CTRL_REG1_FORCE_LINK;
    }

    /* Need to write page register for set page */
    if (skye_phy_reg_wr(port, phy_addr, reg_addr, reg_val)
        == FAILED) {
        printf("Set Register Page %d Fail !\n", page_no);
        return (FAILED);
    }

    sleep(1);

    /* read back register value*/
    if (skye_phy_reg_rd(port, phy_addr, reg_addr, &reg_val) == FAILED) {
        printf("Dump port fails\n");
        return (FAILED);
    } else {
        if ((NVRAM)->diagflag & D_VERBOSE) { /* Turns debug on */
            printf("value: reg %#.4x, data %#.4x\n", reg_addr, reg_val);
        }
    }

    if (reg_val < 0) {
        return (FAILED);
    } else {
        return (PASSED);
    }

}


/*------------------------------------------------------------------
 *
 * Function: auto_negotiation
 *           for link up the interface need to disable auto negotiation *
 *
 * Input:  onoff - turn on/off
 *         phy_id - phy addr for setup port
 *
 * Output: PASSED/FAILED
 *
 *------------------------------------------------------------------
 */
static int auto_negotiation (boolean onoff)
{
    int reg_addr,phy_addr, page_no, reg_val, port;
    phy_addr = PHY_ID_88E1514;
    /* Check CPU0 or 1 to decide which port GE to test */
    if (is_cpu0() == TRUE) {
        /* Front Panel GE gbe5 */
        port = GE_FP_CPU0_PORT;
    } else {
        /* Front Panel GE gbe2 */
        port = GE_FP_CPU1_PORT;
    }
    reg_val = 1;

    /* go to page 0 */
    page_no = MRV88E1512_REG_PAGE_0;
    /* Need to write page register for set page */
    if (select_phy_page_reg(page_no)
        == FAILED) {
        printf("Set Register Page %d Fail !\n", page_no);
        return (FAILED);
    }

    reg_addr = MRV88E1512C_CONTROL_REG;

    /* read original register value*/
    if (skye_phy_reg_rd(port, phy_addr, reg_addr, &reg_val) == FAILED) {
        printf("Dump port fails\n");
        return (FAILED);
    } else {
        if ((NVRAM)->diagflag & D_VERBOSE) { /* Turns debug on */
            printf("value: reg %#.4x, data %#.4x\n", reg_addr, reg_val);
        }
    }

    /*bit 12 for auto negotiation */
    if (onoff) {
        /* enable auto negotiation */
        reg_val |= MRV88E1512_CONTROL_AUTONEG_ENABLE;
    } else {
        /* disable auto negotation */
        reg_val &= ~MRV88E1512_CONTROL_AUTONEG_ENABLE;
    }

    /* Need to write page register for set page */
    if (skye_phy_reg_wr(port, phy_addr, reg_addr, reg_val)
        == FAILED) {
        printf("Set Register Page %d Fail !\n", page_no);
        return (FAILED);
    }

    sleep(1);

    /* read back register value*/
    if (skye_phy_reg_rd(port, phy_addr, reg_addr, &reg_val) == FAILED) {
        printf("Dump port fails\n");
        return (FAILED);
    } else {
        if ((NVRAM)->diagflag & D_VERBOSE) { /* Turns debug on */
            printf("value: reg %#.4x, data %#.4x\n", reg_addr, reg_val);
        }
    }

    if (reg_val < 0) {
        return (FAILED);
    } else {
        return (PASSED);
    }

}


/*------------------------------------------------------------------
 *
 * Function: enable_cooper_1g_speed
 *           for Enable cooper 1000 Mbps Speed
 *
 * Input:  enable - Enable / Disable speed
 *
 * Output: PASSED/FAILED
 *
 *------------------------------------------------------------------
 */
int enable_cooper_1g_speed (int enable)
{
    int reg_addr,phy_addr, page_no, reg_val, reg_val2, port;
    phy_addr = PHY_ID_88E1514;
    /* Check CPU0 or 1 to decide which port GE to test */
    if (is_cpu0() == TRUE) {
        /* Front Panel GE gbe5 */
        port = GE_FP_CPU0_PORT;
    } else {
        /* Front Panel GE gbe2 */
        port = GE_FP_CPU1_PORT;
    }
    page_no = PAGE_0; /* page 0 */

    /* Need to write page register for set page */
    if (select_phy_page_reg(page_no)
        == FAILED) {
        printf("Set Register Page %d Fail !\n", page_no);
        return (FAILED);
    }
    /* Copper Control Register Page 0, Register 9 */
    reg_addr = MRV88E1512C_1000B_CNTL_REG;

    /* read original register value*/
    if (skye_phy_reg_rd(port, phy_addr, reg_addr, &reg_val) == FAILED) {
        printf("Dump port fails\n");
        return (FAILED);
    } else {
        if ((NVRAM)->diagflag & D_VERBOSE) { /* Turns debug on */
            printf("value: reg %#.4x, data %#.4x\n", reg_addr, reg_val);
        }
    }
    /* 1G Page 0, Reg 9 , bit 8~9 */
    if (enable)
        reg_val |= MRV88E1512_1000BT_ADV;
    else
        reg_val &= (~MRV88E1512_1000BT_ADV);

    /* Need to write page register for set page */
    if (skye_phy_reg_wr(port, phy_addr, reg_addr, reg_val)
        == FAILED) {
        printf("Set Register Page %d Fail !\n", page_no);
        return (FAILED);
    }

    /* Read back the speed config to check if the speed was write correctly */
    if (skye_phy_reg_rd(port, phy_addr, reg_addr, &reg_val2) == FAILED) {
        printf("Dump port fails\n");
        return (FAILED);
    } else {
        if ((NVRAM)->diagflag & D_VERBOSE) { /* Turns debug on */
            printf("value: reg %#.4x, data %#.4x\n", reg_addr, reg_val2);
        }
    }

    if (reg_val2 != reg_val)
        printf("%s: write fail, value: reg %#.4x, data %#.4x\n", __FUNCTION__, reg_addr, reg_val2);

    return (PASSED);
}


/*------------------------------------------------------------------
 *
 * Function: enable_cooper_100_speed
 *           for Enable cooper 100 Mbps Speed
 *
 * Input:  enable - Enable / Disable speed
 *
 * Output: PASSED/FAILED
 *
 *------------------------------------------------------------------
 */
int enable_cooper_100_speed (int enable)
{
    int reg_addr,phy_addr, page_no, reg_val, reg_val2, port;
    phy_addr = PHY_ID_88E1514;
    /* Check CPU0 or 1 to decide which port GE to test */
    if (is_cpu0() == TRUE) {
        /* Front Panel GE gbe5 */
        port = GE_FP_CPU0_PORT;
    } else {
        /* Front Panel GE gbe2 */
        port = GE_FP_CPU1_PORT;
    }
    page_no = PAGE_0; /* page 0 */

    /* Need to write page register for set page */
    if (select_phy_page_reg(page_no)
        == FAILED) {
        printf("Set Register Page %d Fail !\n", page_no);
        return (FAILED);
    }
    /* Copper Control Register Page 0, Register 4 */
    reg_addr = MRV88E1512C_AUTONEG_ADVR_REG;

    /* read original register value*/
    if (skye_phy_reg_rd(port, phy_addr, reg_addr, &reg_val) == FAILED) {
        printf("Dump port fails\n");
        return (FAILED);
    } else {
        if ((NVRAM)->diagflag & D_VERBOSE) { /* Turns debug on */
            printf("value: reg %#.4x, data %#.4x\n", reg_addr, reg_val);
        }
    }
    /* 100 Mbps Page 0, Reg 4 , bit 7~8 */
    if (enable)
        reg_val |= MRV88E1512_100BT_ADV;
    else
        reg_val &= (~MRV88E1512_100BT_ADV);

    /* Need to write page register for set page */
    if (skye_phy_reg_wr(port, phy_addr, reg_addr, reg_val)
        == FAILED) {
        printf("Set Register Page %d Fail !\n", page_no);
        return (FAILED);
    }

    /* Read back the speed config to check if the speed was write correctly */
    if (skye_phy_reg_rd(port, phy_addr, reg_addr, &reg_val2) == FAILED) {
        printf("Dump port fails\n");
        return (FAILED);
    } else {
        if ((NVRAM)->diagflag & D_VERBOSE) { /* Turns debug on */
            printf("value: reg %#.4x, data %#.4x\n", reg_addr, reg_val2);
        }
    }

    if (reg_val2 != reg_val)
        printf("%s: write fail, value: reg %#.4x, data %#.4x\n", __FUNCTION__, reg_addr, reg_val2);

    return (PASSED);
}


/*------------------------------------------------------------------
 *
 * Function: enable_cooper_10_speed
 *           for Enable cooper 10 Mbps Speed
 *
 * Input:  enable - Enable / Disable speed
 *
 * Output: PASSED/FAILED
 *
 *------------------------------------------------------------------
 */
int enable_cooper_10_speed (int enable)
{
    int reg_addr,phy_addr, page_no, reg_val, reg_val2, port;
    phy_addr = PHY_ID_88E1514;
    /* Check CPU0 or 1 to decide which port GE to test */
    if (is_cpu0() == TRUE) {
        /* Front Panel GE gbe5 */
        port = GE_FP_CPU0_PORT;
    } else {
        /* Front Panel GE gbe2 */
        port = GE_FP_CPU1_PORT;
    }
    page_no = PAGE_0; /* page 0 */

    /* Need to write page register for set page */
    if (select_phy_page_reg(page_no)
        == FAILED) {
        printf("Set Register Page %d Fail !\n", page_no);
        return (FAILED);
    }
    /* Copper Control Register Page 0, Register 4 */
    reg_addr = MRV88E1512C_AUTONEG_ADVR_REG;

    /* read original register value*/
    if (skye_phy_reg_rd(port, phy_addr, reg_addr, &reg_val) == FAILED) {
        printf("Dump port fails\n");
        return (FAILED);
    } else {
        if ((NVRAM)->diagflag & D_VERBOSE) { /* Turns debug on */
            printf("value: reg %#.4x, data %#.4x\n", reg_addr, reg_val);
        }
    }
    /* 10 Mbps Page 0, Reg 4 , bit 5~6 */
    if (enable)
        reg_val |= MRV88E1512_10BT_ADV;
    else
        reg_val &= (~MRV88E1512_10BT_ADV);

    /* Need to write page register for set page */
    if (skye_phy_reg_wr(port, phy_addr, reg_addr, reg_val)
        == FAILED) {
        printf("Set Register Page %d Fail !\n", page_no);
        return (FAILED);
    }

    /* Read back the speed config to check if the speed was write correctly */
    if (skye_phy_reg_rd(port, phy_addr, reg_addr, &reg_val2) == FAILED) {
        printf("Dump port fails\n");
        return (FAILED);
    } else {
        if ((NVRAM)->diagflag & D_VERBOSE) { /* Turns debug on */
            printf("value: reg %#.4x, data %#.4x\n", reg_addr, reg_val2);
        }
    }

    if (reg_val2 != reg_val)
        printf("%s: write fail, value: reg %#.4x, data %#.4x\n", __FUNCTION__, reg_addr, reg_val2);

    return (PASSED);
}


/*------------------------------------------------------------------
 *
 * Function: config_mac_speed
 *           for configuration Mac Speed
 *
 * Input:  speedmode - speed mode selection
 *
 * Output: PASSED/FAILED
 *
 *------------------------------------------------------------------
 */
int config_mac_speed (int speedmode)
{
    int reg_addr,phy_addr, page_no, reg_val, reg_val2, port;
    phy_addr = PHY_ID_88E1514;
    /* Check CPU0 or 1 to decide which port GE to test */
    if (is_cpu0() == TRUE) {
        /* Front Panel GE gbe5 */
        port = GE_FP_CPU0_PORT;
    } else {
        /* Front Panel GE gbe2 */
        port = GE_FP_CPU1_PORT;
    }
    page_no = PAGE_2; /* page 2 */

    /* Need to write page register for set page */
    if (select_phy_page_reg(page_no)
        == FAILED) {
        printf("Set Register Page %d Fail !\n", page_no);
        return (FAILED);
    }
    /* Copper Control Register Page 2, Register 21 */
    reg_addr = MRV88E1512M_SPECIFIC_CONTROL2_REG;

    /* read original register value*/
    if (skye_phy_reg_rd(port, phy_addr, reg_addr, &reg_val) == FAILED) {
        printf("Dump port fails\n");
        return (FAILED);
    } else {
        if ((NVRAM)->diagflag & D_VERBOSE) { /* Turns debug on */
            printf("value: reg %#.4x, data %#.4x\n", reg_addr, reg_val);
        }
    }
    /* change speed mode bit 6,13 */
    switch (speedmode) {
    case MRV88E1512_MAC_SPD_10M:
        reg_val &= ~MRV88E1512_MAC_SPD_MASK;
        reg_val |= MRV88E1512_MAC_SPD_10M;
        break;
    case MRV88E1512_MAC_SPD_100M:
        reg_val &= ~MRV88E1512_MAC_SPD_MASK;
        reg_val |= MRV88E1512_MAC_SPD_100M;
        break;
    case MRV88E1512_MAC_SPD_1000M:
        reg_val &= ~MRV88E1512_MAC_SPD_MASK;
        reg_val |= MRV88E1512_MAC_SPD_1000M;
        break;
    default:
        printf("Speed mode %d not support\n", speedmode);
        break;
    }

    /* Need to write page register for set page */
    if (skye_phy_reg_wr(port, phy_addr, reg_addr, reg_val)
        == FAILED) {
        printf("Set Register Page %d Fail !\n", page_no);
        return (FAILED);
    }

    /* Read back the speed config to check if the speed was write correctly */
    if (skye_phy_reg_rd(port, phy_addr, reg_addr, &reg_val2) == FAILED) {
        printf("Dump port fails\n");
        return (FAILED);
    } else {
        if ((NVRAM)->diagflag & D_VERBOSE) { /* Turns debug on */
            printf("value: reg %#.4x, data %#.4x\n", reg_addr, reg_val2);
        }
    }

    if (reg_val2 != reg_val)
        printf("%s: write fail, value: reg %#.4x, data %#.4x\n", __FUNCTION__, reg_addr, reg_val2);

    return (PASSED);
}


/*------------------------------------------------------------------
 *
 * Function: config_phy_speed
 *           for configuration PHY Speed
 *
 * Input:  speedmode - speed mode selection
 *
 * Output: PASSED/FAILED
 *
 *------------------------------------------------------------------
 */
int config_phy_speed (int speedmode)
{
    int reg_addr,phy_addr, page_no, reg_val, reg_val2, port;
    phy_addr = PHY_ID_88E1514;
    /* Check CPU0 or 1 to decide which port GE to test */
    if (is_cpu0() == TRUE) {
        /* Front Panel GE gbe5 */
        port = GE_FP_CPU0_PORT;
    } else {
        /* Front Panel GE gbe2 */
        port = GE_FP_CPU1_PORT;
    }
    page_no = PAGE_0; /* page 0 */

    /* Need to write page register for set page */
    if (select_phy_page_reg(page_no)
        == FAILED) {
        printf("Set Register Page %d Fail !\n", page_no);
        return (FAILED);
    }
    /* Copper Control Register Page 0, Register 0 */
    reg_addr = MRV88E1512C_CONTROL_REG;

    /* read original register value*/
    if (skye_phy_reg_rd(port, phy_addr, reg_addr, &reg_val) == FAILED) {
        printf("Dump port fails\n");
        return (FAILED);
    } else {
        if ((NVRAM)->diagflag & D_VERBOSE) { /* Turns debug on */
            printf("value: reg %#.4x, data %#.4x\n", reg_addr, reg_val);
        }
    }
    /* change speed mode bit 6,13 */
    switch (speedmode) {
    case MRV88E1512_SPD_SEL_10M:
        reg_val &= ~MRV88E1512_SPD_SEL_MASK;
        reg_val |= MRV88E1512_SPD_SEL_10M;
        break;
    case MRV88E1512_SPD_SEL_100M:
        reg_val &= ~MRV88E1512_SPD_SEL_MASK;
        reg_val |= MRV88E1512_SPD_SEL_100M;
        break;
    case MRV88E1512_SPD_SEL_1000M:
        reg_val &= ~MRV88E1512_SPD_SEL_MASK;
        reg_val |= MRV88E1512_SPD_SEL_1000M;
        break;
    default:
        printf("Speed mode %d not support\n", speedmode);
        break;
    }

    /* Need to write page register for set page */
    if (skye_phy_reg_wr(port, phy_addr, reg_addr, reg_val)
        == FAILED) {
        printf("Set Register Page %d Fail !\n", page_no);
        return (FAILED);
    }

    /* Read back the speed config to check if the speed was write correctly */
    if (skye_phy_reg_rd(port, phy_addr, reg_addr, &reg_val2) == FAILED) {
        printf("Dump port fails\n");
        return (FAILED);
    } else {
        if ((NVRAM)->diagflag & D_VERBOSE) { /* Turns debug on */
            printf("value: reg %#.4x, data %#.4x\n", reg_addr, reg_val2);
        }
    }

    if (reg_val2 != reg_val)
        printf("%s: write fail, value: reg %#.4x, data %#.4x\n", __FUNCTION__, reg_addr, reg_val2);

    return (PASSED);
}


/*------------------------------------------------------------------
 *
 * Function: check_copper_link_speed
 *           for check cooper link speed
 *
 * Input:  ret_val - return value of speed
 *
 * Output: PASSED/FAILED
 *
 *------------------------------------------------------------------
 */
int check_copper_link_speed (int *ret_val)
{
    int reg_addr,phy_addr, page_no, reg_val, port;
    phy_addr = PHY_ID_88E1514;
    /* Check CPU0 or 1 to decide which port GE to test */
    if (is_cpu0() == TRUE) {
        /* Front Panel GE gbe5 */
        port = GE_FP_CPU0_PORT;
    } else {
        /* Front Panel GE gbe2 */
        port = GE_FP_CPU1_PORT;
    }
    page_no = PAGE_0; /* page 0 */

    /* Need to write page register for set page */
    if (select_phy_page_reg(page_no)
        == FAILED) {
        printf("Set Register Page %d Fail !\n", page_no);
        return (FAILED);
    }
    /* Copper Control Register Page 0, Register 17 */
    reg_addr = MRV88E1512C_SPECIFIC_STATUS1_REG;

    /* read original register value*/
    if (skye_phy_reg_rd(port, phy_addr, reg_addr, &reg_val) == FAILED) {
        printf("Dump port fails\n");
        return (FAILED);
    } else {
        if ((NVRAM)->diagflag & D_VERBOSE) { /* Turns debug on */
            printf("value: reg %#.4x, data %#.4x\n", reg_addr, reg_val);
        }
    }

    *ret_val = reg_val;

    return (PASSED);
}


/*------------------------------------------------------------------
 *
 * Function: check_mac_side_link_speed
 *           for check MAC link speed
 *
 * Input:  ret_val - return value of speed
 *
 * Output: PASSED/FAILED
 *
 *------------------------------------------------------------------
 */
int check_mac_side_link_speed (int *ret_val)
{
    int reg_addr,phy_addr, page_no, reg_val, port;
    phy_addr = PHY_ID_88E1514;
    /* Check CPU0 or 1 to decide which port GE to test */
    if (is_cpu0() == TRUE) {
        /* Front Panel GE gbe5 */
        port = GE_FP_CPU0_PORT;
    } else {
        /* Front Panel GE gbe2 */
        port = GE_FP_CPU1_PORT;
    }
    page_no = PAGE_1; /* page 1 */

    /* Need to write page register for set page */
    if (select_phy_page_reg(page_no)
        == FAILED) {
        printf("Set Register Page %d Fail !\n", page_no);
        return (FAILED);
    }
    /* Copper Control Register Page 0, Register 17 */
    reg_addr = MRV88E1512C_SPECIFIC_STATUS1_REG;

    /* read original register value*/
    if (skye_phy_reg_rd(port, phy_addr, reg_addr, &reg_val) == FAILED) {
        printf("Dump port fails\n");
        return (FAILED);
    } else {
        if ((NVRAM)->diagflag & D_VERBOSE) { /* Turns debug on */
            printf("value: reg %#.4x, data %#.4x\n", reg_addr, reg_val);
        }
    }

    *ret_val = reg_val;

    return (PASSED);
}


/*------------------------------------------------------------------
 *
 * Function: mv1514_reg_read
 *           API Read register
 *
 * Input:  page_no  - register page number
 *         reg_addr - register address
 *         ret_val  - return read value
 *
 * Output: PASSED/FAILED
 *
 *------------------------------------------------------------------
 */
int mv1514_reg_read (int page_no, int reg_addr, int *ret_val)
{
    int phy_addr, reg_val, port;
    phy_addr = PHY_ID_88E1514;
    /* Check CPU0 or 1 to decide which port GE to test */
    if (is_cpu0() == TRUE) {
        /* Front Panel GE gbe5 */
        port = GE_FP_CPU0_PORT;
    } else {
        /* Front Panel GE gbe2 */
        port = GE_FP_CPU1_PORT;
    }

    /* Need to write page register for set page */
    if (select_phy_page_reg(page_no)
        == FAILED) {
        printf("Set Register Page %d Fail !\n", page_no);
        return (FAILED);
    }

    /* read original register value*/
    if (skye_phy_reg_rd(port, phy_addr, reg_addr, &reg_val) == FAILED) {
        printf("Dump port fails\n");
        return (FAILED);
    } else {
        if ((NVRAM)->diagflag & D_VERBOSE) { /* Turns debug on */
            printf("rd_value: reg %#.4x, data %#.4x\n", reg_addr, reg_val);
        }
    }

    *ret_val = reg_val;

    return (PASSED);
}


/*------------------------------------------------------------------
 *
 * Function: mv1514_reg_write
 *           API Write register
 *
 * Input:  page_no  - register page number
 *         reg_addr - register address
 *         data     - write value
 *
 * Output: PASSED/FAILED
 *
 *------------------------------------------------------------------
 */
int mv1514_reg_write (int page_no, int reg_addr, int data)
{
    int phy_addr, port;
    phy_addr = PHY_ID_88E1514;
    /* Check CPU0 or 1 to decide which port GE to test */
    if (is_cpu0() == TRUE) {
        /* Front Panel GE gbe5 */
        port = GE_FP_CPU0_PORT;
    } else {
        /* Front Panel GE gbe2 */
        port = GE_FP_CPU1_PORT;
    }

    /* Need to write page register for set page */
    if (select_phy_page_reg(page_no)
        == FAILED) {
        printf("Set Register Page %d Fail !\n", page_no);
        return (FAILED);
    }

    /* Need to write page register for set page */
    if (skye_phy_reg_wr(port, phy_addr, reg_addr, data)
        == FAILED) {
        printf("Set Register Page %d Fail !\n", page_no);
        return (FAILED);
    } else {
        if ((NVRAM)->diagflag & D_VERBOSE) { /* Turns debug on */
            printf("wr_value: reg %#.4x, data %#.4x\n", reg_addr, data);
        }
    }

    return (PASSED);
}


/*******************************************************************************
 *
 * Function: dev_88e1514_cleanup_lpbk
 *
 * This function disable loopbacks and sets Marvell GE PHY
 * back into normal operating mode.
 *
 * Input:  inport   - input port number
 *         outport  - output port number
 *
 * Output: PASSED/FAILED
 *
 *******************************************************************************
 */
int dev_88e1514_cleanup_lpbk (void)
{
    int phy_addr, retval, data, phy_speed, mac_speed;

    phy_addr = PHY_ID_88E1514;
    retval = PASSED;
    phy_speed = MRV88E1512_SPD_SEL_1000M;
    mac_speed = MRV88E1512_MAC_SPD_1000M;

    /* config phy speed 1Gpbs for SGMII (page 2, reg 21) */
    retval = mv1514_reg_read(MRV88E1512_REG_PAGE_2, MRV88E1512M_SPECIFIC_CONTROL2_REG, &data);
    if (retval == PASSED) {
        data &= ~MRV88E1512_MAC_SPD_MASK;
        data |= mac_speed;
        retval = mv1514_reg_write(MRV88E1512_REG_PAGE_2, MRV88E1512M_SPECIFIC_CONTROL2_REG, data);
        if (retval != PASSED) {
            printf("%s(): Restore phy mac speed 1Gbps "
                                                "failed.\n", __FUNCTION__);
            return (retval);
        }
    } else {
        printf("%s(): phy smi read failed. phy_addr = "
                        "#%x, page = %#x, reg = %#x, rc = %#x\n",
                         __FUNCTION__, phy_addr,
                         MRV88E1512_REG_PAGE_2,
                         MRV88E1512M_SPECIFIC_CONTROL2_REG, retval);
        return (retval);
    }

    /* set speed 1Gbps & auto-neg & full-duplex of page 0, reg 0*/
    retval = mv1514_reg_read(MRV88E1512_REG_PAGE_0, MRV88E1512C_CONTROL_REG, &data);
    if (retval == PASSED) {
        data &= ~MRV88E1512_SPD_SEL_MASK;
        data |= phy_speed | MRV88E1512_CONTROL_AUTONEG_ENABLE | MRV88E1512_CONTROL_FULL_DUPLEX;
        retval = mv1514_reg_write(MRV88E1512_REG_PAGE_0, MRV88E1512C_CONTROL_REG, data);
        if (retval != PASSED) {
            printf("%s(): set phy copper speed "
                                                "failed.\n", __FUNCTION__);
            return (retval);
        }
    } else {
        printf("%s(): phy smi read failed. phy_addr = "
                        "#%x, page = %#x, reg = %#x, rc = %#x\n",
                         __FUNCTION__, phy_addr,
                         MRV88E1512_REG_PAGE_0,
                         MRV88E1512C_CONTROL_REG, retval);
        return (retval);
    }

    /* clear 1000BT PHY External loopback mode */
    retval = mv1514_reg_read(MRV88E1512_REG_PAGE_6, MRV88E1512_CRC_CHKR_REG, &data);
    if (retval == PASSED) {
        data &= ~MRV88E1512_ENABLE_STUB_TEST;
        retval = mv1514_reg_write(MRV88E1512_REG_PAGE_6, MRV88E1512_CRC_CHKR_REG, data);
        if (retval != PASSED) {
            printf("%s(): Clear phy 1000BT external "
                                                "failed.\n", __FUNCTION__);
            return (retval);
        }
    } else {
        printf("%s(): phy smi read failed. phy_addr = "
                        "#%x, page = %#x, reg = %#x, rc = %#x\n",
                         __FUNCTION__, phy_addr,
                         MRV88E1512_REG_PAGE_6,
                         MRV88E1512_CRC_CHKR_REG, retval);
        return (retval);
    }

    /* clear PHY loopback mode (bit 14, page 0, reg 0) */
    retval = mv1514_reg_read(MRV88E1512_REG_PAGE_0, MRV88E1512C_CONTROL_REG, &data);
    if (retval == PASSED) {
        data &= ~MRV88E1512_CONTROL_LOOPBACK;
        retval = mv1514_reg_write(MRV88E1512_REG_PAGE_0, MRV88E1512C_CONTROL_REG, data);
        if (retval != PASSED) {
            printf("%s(): Clear phy loopback mode "
                                                "failed.\n", __FUNCTION__);
            return (retval);
        }
    } else {
        printf("%s(): phy smi read failed. phy_addr = "
                        "#%x, page = %#x, reg = %#x, rc = %#x\n",
                         __FUNCTION__, phy_addr,
                         MRV88E1512_REG_PAGE_6,
                         MRV88E1512_CRC_CHKR_REG, retval);
        return (retval);
    }

    retval = reset_phy_88E1514_register();
    if (retval != PASSED) {
        printf("%s(): phy addr %d reset failed. rc = %#x"
                                "\n", __FUNCTION__, phy_addr, retval);
        return (FAILED);
    }

    return (retval);
}


/*******************************************************************************
 *
 * Function: dev_88e1514_set_lpbk
 *
 * This function set PHY loopback mode and speed.
 *
 * Input: speed - 10/100/1G
 *        lpbk  - loopback mode
 *
 * Output: PASSED/FAILED
 *
 *******************************************************************************
 */
int dev_88e1514_set_lpbk (int speed, int lpbk)
{
    int retval, data, phy_speed = -1, mac_speed = -1;
    int phy_addr, reg_val, port;
    phy_addr = PHY_ID_88E1514;
    /* Check CPU0 or 1 to decide which port GE to test */
    if (is_cpu0() == TRUE) {
        /* Front Panel GE gbe5 */
        port = GE_FP_CPU0_PORT;
    } else {
        /* Front Panel GE gbe2 */
        port = GE_FP_CPU1_PORT;
    }
    reg_val = 1;

    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("setup phy device, phy_addr %d, speed %d, loopback %d\n",
                phy_addr, speed, lpbk);
    }

    retval = PASSED;

    switch (speed) {
    case ETH_MODE_GE:
        phy_speed = MRV88E1512_SPD_SEL_1000M;
        mac_speed = MRV88E1512_MAC_SPD_1000M;
        break;
    case ETH_MODE_FE100:
        phy_speed = MRV88E1512_SPD_SEL_100M;
        mac_speed = MRV88E1512_MAC_SPD_100M;
        break;
    case ETH_MODE_FE10:
        phy_speed = MRV88E1512_SPD_SEL_10M;
        mac_speed = MRV88E1512_MAC_SPD_10M;
        break;
    default:
        printf("%s(): unknown speed\n", __FUNCTION__);
        return (FAILED);
    }

    /* config 1000BT PHY External loopback mode */
    if (lpbk == SGMII_LPBK_NONE && speed == ETH_MODE_GE) {
        enable_phy_ext_lpbk(ENABLE);
    } else {
        enable_phy_ext_lpbk(DISABLE);
    }

    /* config phy speed for SGMII (page 2, reg 21) */
    config_mac_speed(mac_speed);

    /* set speed of page 0, reg 0*/
    config_phy_speed(phy_speed);

    /* config Auto negotiation bit 12, Page 0, Register 0*/
    auto_negotiation(DISABLE);

    /* Force linkup for Internal loopback without stub */
    if (lpbk != SGMII_LPBK_NONE) {
        force_linkup(ENABLE);
    }

    /* config PHY reset bit 15, Page 0, Register 0*/
    reset_phy_88E1514_register();

    /* config PHY loopback mode (bit 14, page 0, reg 0) */
    if (lpbk == SGMII_LPBK_NONE) {
        enable_phy_lpbk(DISABLE);
    } else {
        enable_phy_lpbk(ENABLE);
    }
    /* Wait for link up */
    msleep(4000);  //DEBUG : Intermittent issue Link are not up

    if (lpbk == SGMII_LPBK_NONE) {
        /* Check copper link speed (page 0, reg 17)*/
        retval = check_copper_link_speed(&data);
        if (retval != PASSED) {
            printf("%s(): phy smi read failed. phy_addr = "
                    "#%x, page = %#x, reg = %#x, rc = %#x\n",
                     __FUNCTION__, phy_addr,
                     MRV88E1512_REG_PAGE_0,
                     MRV88E1512C_SPECIFIC_STATUS1_REG, retval);
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
                printf("%s(): Copper link speed is NOT "
                                        "1Gbps.\n", __FUNCTION__);
                return (FAILED);
            }
            break;
        case ETH_MODE_FE100:
            if ((data & MRV88E1512_LINK_SPEED_MASK) !=
                MRV88E1512_LINK_SPEED_100) {
                printf("%s(): Copper link speed is NOT "
                                        "100Mbps.\n", __FUNCTION__);
                return (FAILED);
            }
            break;
        case ETH_MODE_FE10:
            if ((data & MRV88E1512_LINK_SPEED_MASK) !=
                MRV88E1512_LINK_SPEED_10) {
                printf("%s(): Copper link speed is NOT "
                                        "10Mbps.\n", __FUNCTION__);
                return (FAILED);
            }
            break;
        default:
            printf("%s(): unknown link speed\n", __FUNCTION__);
            return (FAILED);
        }
    }

    /* Check MAC Side Link up and Sync (page 1, reg 17)*/
    retval = check_mac_side_link_speed(&data);
    if (retval != PASSED) {
        printf("%s(): phy smi read failed. phy_addr = "
                    "#%x, page = %#x, reg = %#x, rc = %#x\n",
                     __FUNCTION__, phy_addr,
                     MRV88E1512_REG_PAGE_1,
                     MRV88E1512C_SPECIFIC_STATUS1_REG, retval);
    }

    if ((NVRAM)->diagflag & D_VERBOSE) {
        if (data & MRV88E1512_LINK_UP) {
            printf("%s(): MAC is link up.\n", __FUNCTION__);
        } else {
            printf("%s(): MAC is NOT link up.\n", __FUNCTION__);
        }
    }
    if (data & MRV88E1512_SYNC) {
        if ((NVRAM)->diagflag & D_VERBOSE) {
            printf("MAC is Sync.\n");
        }
    } else {
        printf("%s(): MAC is NOT Sync.\n", __FUNCTION__);
        return (FAILED);
    }

    switch (speed) {
    case ETH_MODE_GE:
        if ((data & MRV88E1512_LINK_SPEED_MASK) != MRV88E1512_LINK_SPEED_1000) {
            printf("%s(): MAC link speed is NOT 1Gbps.\n",
                                     __FUNCTION__);
            return (FAILED);
        }
        break;
    case ETH_MODE_FE100:
        if ((data & MRV88E1512_LINK_SPEED_MASK) != MRV88E1512_LINK_SPEED_100) {
            printf("%s(): MAC link speed is NOT 100Mbps.\n",
                                     __FUNCTION__);
            return (FAILED);
        }
        break;
    case ETH_MODE_FE10:
        if ((data & MRV88E1512_LINK_SPEED_MASK) != MRV88E1512_LINK_SPEED_10) {
            printf("%s(): MAC link speed is NOT 10Mbps.\n",
                                     __FUNCTION__);
            return (FAILED);
        }
        break;
    default:
        printf("%s(): unknown MAC link speed\n", __FUNCTION__);
        return (FAILED);
    }

    return (retval);
}


/*******************************************************************************
 *
 * Function    : phy_88E1514_do_all_wrapper
 * Description : Wrapper for 88E1514 do all test items
 * Inputs      : none
 * Outputs     : PASSED / FAILED
 *
 *******************************************************************************
 */
int
phy_88E1514_do_all_wrapper (void)
{
    int rc = PASSED;

    if (mv1514_register_test() == FAILED) {
        return (FAILED);
    }

    if (mv1514_internal_lpbk_test() == FAILED) {
        return (FAILED);
    }

    if (mv1514_external_lpbk_test() == FAILED) {
        return (FAILED);
    }

    return (rc);
}


/***********************************************************************
 *
 *  Function: dump_phy_88E1514_pg0_registers
 *
 * Description: Display GE PHY page 0 registers
 *
 * Inputs: None
 *
 * Outputs: PASSED/FAILED
 *
 *************************************************************************/
int
dump_phy_88E1514_pg0_registers (void)
{
    int reg_addr = 0, phy_addr, reg_val;
    int ports;
    int i = 0;
    int page_no = PAGE_0;

    phy_addr = PHY_ID_88E1514;
    cterr_db_print("Dump Page %d Registers : \n", page_no);
    while (i <= 28) {
    reg_addr = i;

    if (is_cpu0() == TRUE) {
        /* Front Panel GE gbe5 */
        ports = GE_FP_CPU0_PORT;
    } else {
        /* Front Panel GE gbe2 */
        ports = GE_FP_CPU1_PORT;
    }

    /* Need to write page register for set page */
    if (select_phy_page_reg(page_no)
        == FAILED) {
        cterr_db_print("Set Register Page %d Fail !\n", page_no);
        return (FAILED);
    }

    if (skye_phy_reg_rd(ports, phy_addr, reg_addr, &reg_val) == FAILED) {
        cterr_db_print("Dump port fails\n");
        return (FAILED);
    } else {
        cterr_db_print("value: reg %#.4x, data %#.4x\n", reg_addr, reg_val);
    }
    i++;
    }
    printf ("\n");

    return (PASSED);
}

/*-------------------------------------------------
 * $Log: diag_mv1514_test.c,v $
 * Revision 1.2  2015/05/25 03:59:15  steja
 * Add Support Skye SM
 *
 * Revision 1.1.4.3  2015/04/30 08:33:52  steja
 * Clean up code
 *
 * Revision 1.1.4.2  2015/04/29 11:36:32  steja
 * Code check-in to skye-branch2 for ER code review
 *
 *-------------------------------------------------
 * Revision 1.1.2.11  2014/10/14 06:31:11  steja
 * Fix appropriate return failed for do all test.
 *
 * Revision 1.1.2.10  2014/09/24 08:04:58  steja
 * Minor fix remove "***" and arrange the TLK to the last test item for do all
 *
 * Revision 1.1.2.9  2014/09/18 07:18:43  steja
 * 1.Update NC command codei
 * 2.Update enhanced error message
 *
 * Revision 1.1.2.8  2014/09/17 11:13:15  palin2
 * Removed unused code.
 *
 * Revision 1.1.2.7  2014/09/15 07:58:58  steja
 * Code Clean up
 *
 * Revision 1.1.2.6  2014/09/03 02:42:14  steja
 * Increase delay to wait link up before send data.
 *
 * Revision 1.1.2.5  2014/09/02 13:09:49  steja
 * Update Enhance error code for 88E1514
 *
 * Revision 1.1.2.4  2014/08/28 02:54:26  steja
 * Support Do all test for NC command
 *
 * Revision 1.1.2.3  2014/08/25 11:55:48  steja
 * Update Code for BST Testing
 *
 * Revision 1.1.2.2  2014/08/14 12:24:25  steja
 * Remove debug message and add printf info for internal loopback
 *
 * Revision 1.1.2.1  2014/07/21 01:56:52  palin2
 * Initial check-in Skye module side Diag code.
 *
 *-------------------------------------------------
 * Revision 1.2.8.5  2014/06/27 09:16:20  steja
 * Revert to use frontpanel GE to download image.
 *
 * Revision 1.2.8.4  2014/06/25 13:10:20  steja
 * Add External loopback to test 10/100/1000 Mbps , Internal loopback 1000 Mbps still debugging
 *
 * Revision 1.2.8.3  2014/06/06 11:54:20  steja
 * Add Shrinkray LED Test
 *
 * Revision 1.2.8.2  2014/05/15 08:48:18  steja
 * Add more print info for led test
 *
 * Revision 1.2.8.1  2014/05/09 10:34:26  palin2
 * Loopback tests have intermittently fails, still debugging so temporarily
 * remove from default tests.
 *
 * Revision 1.2  2014/02/27 15:01:46  palin2
 * Initial check-in ShrinkRay SM side Diag code.
 *
 * Revision 1.1.4.11  2014/02/25 09:20:48  steja
 * debug frontpanel ge phy internal loopback intermittent fail.
 *
 * Revision 1.1.4.10  2014/01/29 11:44:14  steja
 * Add LED polarity function
 *
 * Revision 1.1.4.9  2014/01/28 13:49:45  steja
 * Update GE Frontpanel test code
 *
 * Revision 1.1.4.8  2014/01/28 07:44:22  steja
 * Update LED GE test
 *
 * Revision 1.1.4.7  2014/01/14 07:45:43  steja
 * Add Verbose flag and update Marvel Test loopback
 *
 * Revision 1.1.4.6  2013/09/29 05:23:16  steja
 * Add force to linkup the gbe
 *
 * Revision 1.1.4.5  2013/09/29 04:03:32  iachang
 * CPU0 GE Backplane RX Debug utility
 * Support 88E1514 initial function
 * Support 88E1514 Power Enable/Disable function
 *
 * Revision 1.1.4.4  2013/09/27 09:45:50  steja
 * Fix the 88E1514 Register Read and Write
 *
 * Revision 1.1.4.3  2013/09/27 07:25:13  steja
 * update code for bringup
 *
 * Revision 1.1.4.2  2013/09/13 07:00:07  palin2
 * Initial check-in ShrinkRay SM side Diag code.
 *
 * Revision 1.1.2.2  2013/07/02 08:24:11  steja
 * Update 88E1514 Register test function
 *
 * Revision 1.1.2.1  2013/06/24 09:03:34  steja
 * Checkin :
 * - Support TLK10323 Loopback test & Utility
 * - Support MV1514 Loopback test
 *
 *-------------------------------------------------
 * $Endlog$
 */

