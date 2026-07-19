 /* $Id: diag_led_test.c,v 1.2 2018/08/06 02:31:50 harrchan Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/viper/diag_led_test.c,v $
 *------------------------------------------------------------------
 *
 * diag_led_test.c - This file is for LED test.
 *
 *
 * Copyright (c) 2008-2018 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "endians.h"
#include "defs.h"
#include "types.h"
#include "nvsysvars.h"
#include "error.h"
#include "pcmap.h"
#include "strings.h"
#include "menu.h"
#include "common.h"
#include "cross_platform.h"
#include "mb_tests.h"
#include "plat_defs.h"
#include "setjmps.h"
#include "proto.h"
#include "diag_fpga.h"
#include "platform_fru.h"
#include "platform_cookie.h"
#include "plat_defs.h"
#include "dev_88e6176.h"
#include "diag_esw_lib.h"
#include "dev_88e151x.h"
#include "dnv_eth_lib.h"
#include "diag_gephy_lib.h"
#include "diag_led_util.h"
#include "diag_led_test.h"
#include "diag_dsl_util.h"


/*
 * Global variables
 */


/* Local functions */
int build_led_util_menu(boolean);
int diag_led_test(boolean);
int diag_system_led_test(void);
int diag_vpn_led_test(void);
int diag_lte_led_test(void);
int diag_esw_led_test(void);
int diag_gephy0_led_test(void);
int diag_gephy1_led_test(void);


/*
 * Sub Menu used for "Main menu -> LED test"
 */
submenu_xtable_t led_tests_submenu_table[] = {
    {"LED Utilities",
     (PFT) build_led_util_menu, FALSE, 0,
     (type_t(*)())0, 0, (PFT) build_led_util_menu, TRUE},

    {"System LED Test",
    (PFT) diag_system_led_test, FALSE,
    MF_CONTINUOUS | MF_DOGRP | MF_DOALL,
    (type_t(*)())0, 0, 
    (PFT) diag_system_led_test, TRUE},

    {"VPN LED Test",
    (PFT) diag_vpn_led_test, FALSE,
    MF_CONTINUOUS | MF_DOGRP | MF_DOALL,
    (type_t(*)())0, 0, 
    (PFT) diag_vpn_led_test, TRUE},

    {"LTE LED Test",
    (PFT) diag_lte_led_test, FALSE,
    MF_CONTINUOUS | MF_DOGRP | MF_DOALL,
    (type_t(*)())has_lte_sku, 0, 
    (PFT) diag_lte_led_test, TRUE},

    {"Ethernet Switch LED Test",
    (PFT) diag_esw_led_test, FALSE,
    MF_CONTINUOUS | MF_DOGRP | MF_DOALL,
    (type_t(*)())0, 0, 
    (PFT) diag_esw_led_test, TRUE},

    {"GEPHY0 LED Test",
    (PFT) diag_gephy0_led_test, FALSE,
    MF_CONTINUOUS | MF_DOGRP | MF_DOALL,
    (type_t(*)())0, 0, 
    (PFT) diag_gephy0_led_test, TRUE},

    {"GEPHY1 LED Test",
    (PFT) diag_gephy1_led_test, FALSE,
    MF_CONTINUOUS | MF_DOGRP | MF_DOALL,
    (type_t(*)())has_ge1_sku, 0, 
    (PFT) diag_gephy1_led_test, TRUE},

};

#define LED_TESTS_SUBMENU_TABLE_SIZE (sizeof(led_tests_submenu_table) / \
                                     sizeof(submenu_xtable_t))

/*
 * "Main menu -> led test" primary & secondary submenu items (filled in from xtable)
 */
static mitem_t led_tests_primary_items[LED_TESTS_SUBMENU_TABLE_SIZE +
                                      MAX_BASE_ITEMS];
static mitem_t led_tests_secondary_items[LED_TESTS_SUBMENU_TABLE_SIZE +
                                        MAX_BASE_ITEMS];

menuinfo_t led_subtest_menu = {
    "%s Subtest Menu",
    0,                          /* mtparam added by init_empty_menu */
    (PFT) show_endnote,         /* notes missing WICs in combos */
    0,                          /* use generic prompt */
    0,                          /* size (bumped by add_menu_item() */
    led_tests_primary_items,
};

menuinfo_t *led_submenup = &led_subtest_menu;

/*
 * Sub Menu used for "LED test -> LED utility submenu"
 */

submenu_xtable_t led_util_submenu_table[] = {
    {"System LED Utility",
     (PFT) diag_sys_led_util, FALSE, 0,
     (type_t(*)())0, 0, (PFT) 0, 0},

    {"LTE Modem RSSI LED Utility",
     (PFT) diag_lte_rssi_led_util, FALSE, 0,
     (type_t(*)())has_lte_sku, 0, (PFT) 0, 0},

    {"LTE SIM LED Utility",
     (PFT) diag_lte_sim_led_util, FALSE, 0,
     (type_t(*)())has_lte_sku, 0, (PFT) 0, 0},

    {"VPN LED Utility",
     (PFT) diag_vpn_led_util, FALSE, 0,
     (type_t(*)())0, 0, (PFT) 0, 0},

    {"Ethernet Switch LED Utility",
     (PFT) diag_esw_led_util, FALSE, 0,
     (type_t(*)())0, 0, (PFT) 0, 0},

    {"GEPHY0 LED Utility",
     (PFT) diag_gephy_led_util, GEPHY0, 0,
     (type_t(*)())0, 0, (PFT) 0, 0},

    {"GEPHY1 LED Utility",
     (PFT) diag_gephy_led_util, GEPHY1, 0,
     (type_t(*)())has_ge1_sku, 0, (PFT) 0, 0},

    {"DSL LED Utility",
     (PFT) xdsl_bcm63168_led_utils, 0, 0,
     (type_t(*)())has_dsl_sku, 0, (PFT) 0, 0},
     
    { "turn all Green LED on",
	 (PFT)diag_all_green_leds_on,  0, 0,
     (type_t(*)())this_is_viper_j, 0, (PFT) 0, 0},
     
    { "turn all Yellow LED on",
	 (PFT)diag_all_yellow_leds_on,  0, 0,
     (type_t(*)())this_is_viper_j, 0, (PFT) 0, 0},
     
    { "turn all LED off",
	 (PFT)diag_all_leds_off,  0, 0,
     (type_t(*)())this_is_viper_j, 0, (PFT) 0, 0},
};

#define LED_UTIL_SUBMENU_TABLE_SIZE (sizeof(led_util_submenu_table) / \
                                     sizeof(submenu_xtable_t))

static mitem_t led_util_primary_items[LED_UTIL_SUBMENU_TABLE_SIZE +
                                      MAX_BASE_ITEMS];
static mitem_t led_util_secondary_items[LED_UTIL_SUBMENU_TABLE_SIZE +
                                        MAX_BASE_ITEMS];

menuinfo_t led_util_subtest_menu = {
    "%s Subtest Menu",
    0,                          /* mtparam added by init_empty_menu */
    (PFT) show_endnote,         /* notes missing WICs in combos */
    0,                          /* use generic prompt */
    0,                          /* size (bumped by add_menu_item() */
    led_util_primary_items,
};

menuinfo_t *led_util_submenup = &led_util_subtest_menu;
/*******************************************************************************
 *
 * Function   : build_led_util_menu
 * Description: build LED utility menu.
 * Inputs     : Test/Menu 
 * Outputs    : None
 *
 *******************************************************************************
 */
int build_led_util_menu (boolean mb_temp_test_items_executed)
{
    char *tname = "LED utility";
    testname(tname);

    build_primary_submenu(led_util_submenu_table, LED_UTIL_SUBMENU_TABLE_SIZE,
                          "LED util SubMenu", &led_util_submenup);
    build_secondary_submenu(led_util_submenu_table, LED_UTIL_SUBMENU_TABLE_SIZE,
                            led_util_secondary_items);
    menu(&led_util_subtest_menu, led_util_secondary_items, 0);
    return (PASSED);
}

/******************************************************************************
 *
 * Function: diag_led_test
 *
 * Description: Show led test submenu 
 *
 * Inputs      : None
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/
int diag_led_test (boolean led_test_items_executed)
{
    int rc = FAILED;

    build_primary_submenu(led_tests_submenu_table,
                          LED_TESTS_SUBMENU_TABLE_SIZE, "Motherboard",
                          &led_submenup);

    build_secondary_submenu(led_tests_submenu_table,
                            LED_TESTS_SUBMENU_TABLE_SIZE,
                            led_tests_secondary_items);

    if (led_test_items_executed) {
        do_all_menu_items(&led_subtest_menu);
    } else {
        menu(&led_subtest_menu, led_tests_secondary_items, '\0');
    }

    return (rc);
}

/******************************************************************************
 *
 * Function: diag_system_led_test
 *
 * Description: System LED test
 *
 * Inputs      : None
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/
int diag_system_led_test (void)
{
    char *tname ="System LED Test";
    uint32_t reg_offset = 0, reg_val = 0;

    testname(tname);
    prpass(testpass, "%s, ", tname);

    /* Access FPGA Register for controll system LED */
    if (fpga_read_reg(FPGA_LED_REG, &reg_val) != PASSED) {
        printf("Failed to read FPGA register 0x%04X.\n", reg_offset);
        return (FAILED);
    }
    
    /* Turn on system led with green */
    reg_val &= LED_OFF;
    if (this_is_viper_j()) {
        /* Enable JDM system LED test mode first*/
        if (fpga_write_reg(FPGA_MANUFAC_TEST_MODE_REG, DBG_TEST_MODE_ON) != PASSED) {
             printf("Failed to enable JDM Debug Test Mode\n");
             return (FAILED);
        }
        reg_val |= SYS_OK_LED_GREEN_J; 
    } else {
        reg_val |= SYS_OK_LED_GREEN; 
    }
    if (fpga_write_reg(FPGA_LED_REG, reg_val) != PASSED) {
        printf("Failed to write FPGA register 0x%04X.\n", reg_offset);
        return (FAILED);
    }

    msleep(DELAY_FOR_LED_TEST);

    if (fpga_read_reg(FPGA_LED_REG, &reg_val) != PASSED) {
        printf("Failed to read FPGA register 0x%04X.\n", reg_offset);
        return (FAILED);
    }
    
    /* Turn on system led with amber */
    reg_val &= LED_OFF;
    if (this_is_viper_j()) {
        reg_val |= SYS_OK_LED_AMBER_J; 
    } else {
        reg_val |= SYS_OK_LED_AMBER;
    }
    if (fpga_write_reg(FPGA_LED_REG, reg_val) != PASSED) {
        printf("Failed to write FPGA register 0x%04X.\n", reg_offset);
        return (FAILED);
    }

    msleep(DELAY_FOR_LED_TEST);

    if (this_is_viper_j()) {
         /* Disable JDM system LED test mode*/
        if (fpga_write_reg(FPGA_MANUFAC_TEST_MODE_REG, DBG_TEST_MODE_OFF) != PASSED) {
             printf("Failed to disable JDM Debug Test Mode\n");
             return (FAILED);
        }
    } 

    prcomplete(testpass, errcount, (char *)0);
    return (PASSED);
}

/******************************************************************************
 *
 * Function: diag_vpn_led_test
 *
 * Description: VPN LED test
 *
 * Inputs      : None
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/
int diag_vpn_led_test (void)
{
    char *tname ="VPN LED Test";
    uint32_t reg_offset = 0, reg_val = 0;

    testname(tname);
    prpass(testpass, "%s, ", tname);

    if (fpga_read_reg(FPGA_LED_REG, &reg_val) != PASSED) {
        printf("Failed to read FPGA register 0x%04X.\n", reg_offset);
        return (FAILED);
    }

    /* Access FPGA Register for controll VPN LED */
    /* Turn on VPN led with green */
    reg_val &= LED_OFF;
    reg_val |= VPN_OK_LED_GREEN; 
    if (fpga_write_reg(FPGA_LED_REG, reg_val) != PASSED) {
        printf("Failed to write FPGA register 0x%04X.\n", reg_offset);
        return (FAILED);
    }

    msleep(DELAY_FOR_LED_TEST);
    prcomplete(testpass, errcount, (char *)0);
    return (PASSED);
}

/******************************************************************************
 *
 * Function: diag_lte_led_test
 *
 * Description:  LTE LED test
 *
 * Inputs      : None
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/
int diag_lte_led_test (void)
{
    char *tname ="LTE LED Test";
    uint32_t reg_offset = 0, reg_val = 0;

    testname(tname);
    prpass(testpass, "%s, ", tname);

    if (fpga_read_reg(FPGA_LTE_RSSI_LED, &reg_val) != PASSED) {
        printf("Failed to read FPGA register 0x%04X.\n", reg_offset);
        return (FAILED);
    }
    
    /* Turn on LTE Modem RSSI with high RSSI */
    reg_val &= LED_OFF;
    reg_val |= LTE_MOD_HIGH_RSSI; 
    if (fpga_write_reg(FPGA_LTE_RSSI_LED, reg_val) != PASSED) {
        printf("Failed to write FPGA register 0x%04X.\n", reg_offset);
        return (FAILED);
    }

    msleep(DELAY_FOR_LED_TEST);

    /* Turn on LTE Modem RSSI with medium RSSI */
    reg_val &= LED_OFF;
    reg_val |= LTE_MOD_MEDIUM_RSSI; 
    if (fpga_write_reg(FPGA_LTE_RSSI_LED, reg_val) != PASSED) {
        printf("Failed to write FPGA register 0x%04X.\n", reg_offset);
        return (FAILED);
    }

    msleep(DELAY_FOR_LED_TEST);

    /* Turn on LTE Modem RSSI with low RSSI */
    reg_val &= LED_OFF;
    reg_val |= LTE_MOD_LOW_RSSI; 
    if (fpga_write_reg(FPGA_LTE_RSSI_LED, reg_val) != PASSED) {
        printf("Failed to write FPGA register 0x%04X.\n", reg_offset);
        return (FAILED);
    }

    msleep(DELAY_FOR_LED_TEST);

    /* Turn on LTE Modem RSSI with weak RSSI */
    reg_val &= LED_OFF;
    reg_val |= LTE_MOD_RSSI; 
    if (fpga_write_reg(FPGA_LTE_RSSI_LED, reg_val) != PASSED) {
        printf("Failed to write FPGA register 0x%04X.\n", reg_offset);
        return (FAILED);
    }

    msleep(DELAY_FOR_LED_TEST);

    /* Turn on LTE Modem RSSI with no RSSI */
    reg_val &= LED_OFF;
    reg_val |= LTE_MOD_NO_RSSI; 
    if (fpga_write_reg(FPGA_LTE_RSSI_LED, reg_val) != PASSED) {
        printf("Failed to write FPGA register 0x%04X.\n", reg_offset);
        return (FAILED);
    }

    msleep(DELAY_FOR_LED_TEST);

    if (fpga_read_reg(FPGA_LED_REG, &reg_val) != PASSED) {
        printf("Failed to read FPGA register 0x%04X.\n", reg_offset);
        return (FAILED);
    }
    
    /* Turn on LTE SIM card LED*/

    if (this_is_viper_j()) {
        /* Enable JDM system LED test mode first, for disable blinking.*/
        if (fpga_write_reg(FPGA_MANUFAC_TEST_MODE_REG, DBG_TEST_MODE_ON) != PASSED) {
             printf("Failed to enable JDM Debug Test Mode\n");
             return (FAILED);
        }
    }
    reg_val &= LED_OFF;
    reg_val |= LTE_SIM_ACT_LED; 
    if (fpga_write_reg(FPGA_LED_REG, reg_val) != PASSED) {
        printf("Failed to write FPGA register 0x%04X.\n", reg_offset);
        return (FAILED);
    }

    msleep(DELAY_FOR_LED_TEST);

    /* Turn off all led */
    reg_val &= LED_OFF;
    if (fpga_write_reg(FPGA_LED_REG, reg_val) != PASSED) {
        printf("Failed to write FPGA register 0x%04X.\n", reg_offset);
        return (FAILED);
    }

    if (this_is_viper_j()) {
         /* Disable JDM system LED test mode*/
        if (fpga_write_reg(FPGA_MANUFAC_TEST_MODE_REG, DBG_TEST_MODE_OFF) != PASSED) {
             printf("Failed to disable JDM Debug Test Mode\n");
             return (FAILED);
        }
    } 

    prcomplete(testpass, errcount, (char *)0);
    return (PASSED);
}
/******************************************************************************
 *
 * Function: diag_esw_led_test
 *
 * Description: Ethernet switch (Marvell 88e6176) LED test
 *
 * Inputs      : None
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/
int diag_esw_led_test (void)
{
    dev_88e6176_object_t esw_obj;
    dev_88e6176_object_t *esw_obj_p  = &esw_obj;
    char *tname ="Ethernet Switch LED Test";
    int port_num, rc = FAILED;

    testname(tname);
    prpass(testpass, "%s, ", tname);

    /* Create 88e6176 device driver */
    rc = diag_esw_dev_create(esw_obj_p);

    if (rc == FAILED) {
        cterr('f', 0, "%s: Creating object failed",__func__);
        return (FAILED);
    }

    /* Write 88e6176 LED Register to turn on light */
    for (port_num = ESW_PORT0; port_num < ESW_PORT4; port_num++) {
        rc = esw_obj_p->callin_fvt->esw_set_led_on((dev_object_t *)esw_obj_p,
                        port_num);
        if ( rc == FAILED) {
            goto _exit;
        } 
    }
    msleep(DELAY_FOR_LED_TEST);
    /* Write 88e6176 LED Register to default status */
    for (port_num = ESW_PORT0; port_num < ESW_PORT4; port_num++) { 
        rc = esw_obj_p->callin_fvt->esw_set_led_off((dev_object_t *)esw_obj_p,
                        port_num);
        if (rc == FAILED) {
            goto _exit;
        } 
    }

    msleep(DELAY_FOR_LED_TEST);
_exit:
    esw_obj_p->base.dev_object_fvt->dev_destroy((dev_object_t **)&esw_obj_p);
    prcomplete(testpass, errcount, (char *)0);
    return (rc);

}
/******************************************************************************
 *
 * Function: diag_gephy0_led_test
 *
 * Description: GE0 (Marvell 88E1514) LED test 
 *
 * Inputs      : None
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/
int diag_gephy0_led_test (void)
{
    dev_88e151x_object_t gephy_obj;
    dev_88e151x_object_t *gephy_obj_p = &gephy_obj;
    char *tname ="gephy0 LED Test";
    int rc = FAILED;

    testname(tname);
    prpass(testpass, "%s, ", tname);

    /* Create device for GE0 */
    rc = diag_gephy_dev_create(VIPER_88E1514_PHY, gephy_obj_p);
    if (rc == FAILED) {
        cterr('f', 0, "%s: Creating object failed", __func__);
        return (FAILED);
    }

    /* Write 88e151x GE0 LED Register to turn on light */
    rc = gephy_obj_p->callin_fvt->gephy_set_led_on((dev_object_t *)gephy_obj_p);
 
    if (rc == FAILED) {
        goto _exit;
    
    } 
    msleep(DELAY_FOR_LED_TEST);

    /* Write 88e151x GE0 LED Register to default status */
    rc = gephy_obj_p->callin_fvt->gephy_set_led_off((dev_object_t *)gephy_obj_p); 
    if (rc == FAILED) {
        goto _exit;
    } 
    
    msleep(DELAY_FOR_LED_TEST);
_exit:
    gephy_obj_p->base.dev_object_fvt->dev_destroy((dev_object_t **)&gephy_obj_p);
    prcomplete(testpass, errcount, (char *)0);
    return (rc);
}

/******************************************************************************
 *
 * Function: diag_gephy1_led_test
 *
 * Description: GE1 (Marvell 88E1514) LED test 
 *
 * Inputs      : None
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/
int diag_gephy1_led_test (void)
{
    dev_88e151x_object_t gephy_obj;
    dev_88e151x_object_t *gephy_obj_p = &gephy_obj;
    char *tname ="gephy1 LED Test";
    int rc = FAILED;

    testname(tname);
    prpass(testpass, "%s, ", tname);


    /* Create device for GE1 */
    rc = diag_gephy_dev_create(VIPER_GE1_88E1514_PHY, gephy_obj_p);
    if (rc == FAILED) {
        cterr('f', 0, "%s: Creating object failed", __func__);
        return (FAILED);
    }
    /* Write 88e151x GE1 LED Register to turn on light */
    rc = gephy_obj_p->callin_fvt->gephy_set_led_on((dev_object_t *)gephy_obj_p);
    if (rc == FAILED) {
        goto _exit;
    } 

    msleep(DELAY_FOR_LED_TEST);

    /* Write 88e151x GE1 LED Register to default status */
    rc = gephy_obj_p->callin_fvt->gephy_set_led_off((dev_object_t *)gephy_obj_p); 
    if (rc == FAILED) {
            goto _exit;
    } 
_exit:
    gephy_obj_p->base.dev_object_fvt->dev_destroy((dev_object_t **)&gephy_obj_p);
    prcomplete(testpass, errcount, (char *)0);
    return (rc);
}

/*-------------------------------------------------
 * $Log: diag_led_test.c,v $
 * Revision 1.2  2018/08/06 02:31:50  harrchan
 * Merge viper E2E to the main trunk (CSCvk28469)
 *
 * Revision 1.1.2.12  2018/07/03 05:38:55  harrchan
 * Follow the coding rule to clean up code
 *
 * Revision 1.1.2.11  2018/06/27 06:27:49  harrchan
 * Follow the coding rule to clean up code
 *
 * Revision 1.1.2.10  2018/05/23 07:02:39  harrchan
 * Add has_lte_sku() and has_dsl_sku() in LED utility menu
 *
 * Revision 1.1.2.9  2018/05/21 08:42:36  olin2
 * Support DSL LED on/off utility
 *
 * Revision 1.1.2.8  2018/05/15 05:36:37  lucywang
 * Modified for ViperJ based on Cisco FPGA
 *
 * Revision 1.1.2.7  2018/05/09 07:11:26  olin2
 * 1. Move GE and DSL init to the beginning. 2. Add has GE1. 3. Show cookie info
 *
 * Revision 1.1.2.6  2018/04/13 11:19:12  lucywang
 * Modified to use Cisco FPGA : 1) Upgrade 2) LED 3) FPGA register 4) FPGA I2C reset
 *
 * Revision 1.1.2.5  2018/04/10 06:19:53  harrchan
 * Modify FPGA register address
 *
 * Revision 1.1.2.4  2018/03/29 12:56:06  lucywang
 * Added LED utilities to turn on/off all green/amber LEDs
 *
 * Revision 1.1.2.3  2018/03/26 09:21:03  harrchan
 * Add led utility
 *
 * Revision 1.1.2.2  2018/03/15 08:30:04  harrchan
 * Update led test
 *
 * Revision 1.1.2.1  2018/02/27 08:06:44  harrchan
 * Initial viper application code base
 *
 *
 * $Endlog$
 *-------------------------------------------------
 */
