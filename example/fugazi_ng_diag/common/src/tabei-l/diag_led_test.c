 /* $Id: diag_led_test.c,v 1.5 2020/08/06 07:54:55 kehuang2 Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/tabei-l/diag_led_test.c,v $
 *------------------------------------------------------------------
 *
 * diag_led_test.c - This file is for LED test.
 *
 *
 * Copyright (c) 2008-2020 by Cisco Systems, Inc.
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
#include "dev_88e151x.h"
#include "dnv_eth_lib.h"
#include "diag_gephy_lib.h"
#include "diag_led_util.h"
#include "diag_led_test.h"
#include "diag_cpld_lib.h"
#include "sys/io.h"
#include "platform_stub.h"

/*
 * Global variables
 */


/* Local functions */
int build_led_util_menu(boolean);
int diag_led_test(boolean);
int diag_env_led_test(void);
int diag_status_led_test(void);
int diag_I350_RJ45_led_test(void);
int diag_I350_SFP_led_test(void);
int diag_hdd_led_test(void);
int diag_gephy0_led_test(void);
int diag_gephy1_led_test(void);
int diag_gephy1543_led_test(void);
int diag_esw6390_led_test(void);
int diag_port80_led_test(void);
int diag_all_led_test (void);

/*
 * Sub Menu used for "Main menu -> LED test"
 */
submenu_xtable_t led_tests_submenu_table[] = {
    {"LED Utilities",
     (PFT) build_led_util_menu, FALSE, 0,
     (type_t(*)())0, 0, (PFT) build_led_util_menu, TRUE},

    {"Status LED Test",
    (PFT) diag_status_led_test, FALSE,
    MF_CONTINUOUS | MF_DOGRP | MF_DOALL,
    (type_t(*)())0, 0, 
    (PFT) diag_status_led_test, TRUE},

    {"Environment LED Test",
    (PFT) diag_env_led_test, FALSE,
    MF_CONTINUOUS | MF_DOGRP | MF_DOALL,
    (type_t(*)())0, 0, 
    (PFT) diag_env_led_test, TRUE},

    {"GEPHY0/0/0 LED Test",
    (PFT) diag_gephy0_led_test, FALSE,
    MF_CONTINUOUS | MF_DOGRP | MF_DOALL,
    (type_t(*)())has_phy1514, 0, 
    (PFT) diag_gephy0_led_test, TRUE},

    {"GEPHY0/0/1 LED Test",
    (PFT) diag_gephy1_led_test, FALSE,
    MF_CONTINUOUS | MF_DOGRP | MF_DOALL,
    (type_t(*)())has_phy1514, 0, 
    (PFT) diag_gephy1_led_test, TRUE},

    {"GEPHY0/0/2 and GEPHY0/0/3 LED Test",
    (PFT) diag_I350_RJ45_led_test, FALSE,
    MF_CONTINUOUS | MF_DOGRP | MF_DOALL,
    (type_t(*)())is_tabeil, 0, 
    (PFT) diag_I350_RJ45_led_test, TRUE},

    {"GEPHY0/0/4 and GEPHY0/0/5 LED Test",
    (PFT) diag_I350_SFP_led_test, FALSE,
    MF_CONTINUOUS | MF_DOGRP | MF_DOALL,
    (type_t(*)())is_tabeil, 0, 
    (PFT) diag_I350_SFP_led_test, TRUE},

    {"GEPHY0/0/0 and GEPHY0/0/1 LED Test",
    (PFT) diag_I350_RJ45_led_test, FALSE,
    MF_CONTINUOUS | MF_DOGRP | MF_DOALL,
    (type_t(*)())is_promethium, 0, 
    (PFT) diag_I350_RJ45_led_test, TRUE},

    {"GEPHY0/0/2 and GEPHY0/0/3 LED Test",
    (PFT) diag_I350_SFP_led_test, FALSE,
    MF_CONTINUOUS | MF_DOGRP | MF_DOALL,
    (type_t(*)())is_promethium, 0, 
    (PFT) diag_I350_SFP_led_test, TRUE},

    {"GEPHY1543 LED Test",
    (PFT) diag_gephy1543_led_test, FALSE,
    MF_CONTINUOUS | MF_DOGRP | MF_DOALL,
    (type_t(*)())has_phy1543, 0, 
    (PFT) diag_gephy1543_led_test, TRUE},

    {"ESW6390 LED Test",
    (PFT) diag_esw6390_led_test, FALSE,
    MF_CONTINUOUS | MF_DOGRP | MF_DOALL,
    (type_t(*)())has_esw6390, 0, 
    (PFT) diag_esw6390_led_test, TRUE},

    {"Port80 LED Test",
    (PFT) diag_port80_led_test, FALSE,
    MF_CONTINUOUS | MF_DOGRP | MF_DOALL,
    (type_t(*)())0, 0, 
    (PFT) diag_port80_led_test, TRUE},

    {"HDD LED Test",
    (PFT) diag_hdd_led_test, FALSE,
    MF_CONTINUOUS | MF_DOGRP | MF_DOALL,
    (type_t(*)())has_hdd, 0, 
    (PFT) diag_hdd_led_test, TRUE},

    {"All LED Test",
    (PFT) diag_all_led_test, FALSE,
    MF_CONTINUOUS | MF_DOGRP | MF_DOALL,
    (type_t(*)())0, 0, 
    (PFT) diag_all_led_test, TRUE},
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
    {"Status LED Utility",
     (PFT) diag_status_led_util, FALSE, 0,
     (type_t(*)())0, 0, (PFT) 0, 0},

    {"Environment LED Utility",
     (PFT) diag_env_led_util, FALSE, 0,
     (type_t(*)())0, 0, (PFT) 0, 0},

    {"GEPHY0 LED Utility",
     (PFT) diag_gephy_led_util, GEPHY0, 0,
     (type_t(*)())has_phy1514, 0, (PFT) 0, 0},

    {"GEPHY1 LED Utility",
     (PFT) diag_gephy_led_util, GEPHY1, 0,
     (type_t(*)())has_phy1514, 0, (PFT) 0, 0},

    {"GEPHY2/3 LED Utility",
     (PFT) diag_I350_RJ45_led_util, FALSE, 0,
     (type_t(*)())is_tabeil, 0, (PFT) 0, 0},

    {"GEPHY4/5 LED Utility",
     (PFT) diag_I350_SFP_led_util, FALSE, 0,
     (type_t(*)())is_tabeil, 0, (PFT) 0, 0},

    {"GEPHY0/1 LED Utility",
     (PFT) diag_I350_RJ45_led_util, FALSE, 0,
     (type_t(*)())is_promethium, 0, (PFT) 0, 0},

    {"GEPHY2/3 LED Utility",
     (PFT) diag_I350_SFP_led_util, FALSE, 0,
     (type_t(*)())is_promethium, 0, (PFT) 0, 0},

    {"Port80 LED Utility",
     (PFT) diag_port80_led_util, FALSE, 0,
     (type_t(*)())0, 0, (PFT) 0, 0},

    {"HDD LED Utility",
     (PFT) diag_hdd_led_util, FALSE, 0,
     (type_t(*)())has_hdd, 0, (PFT) 0, 0},

    {"ALL LED Utility",
     (PFT) diag_all_led_util, FALSE, 0,
     (type_t(*)())0, 0, (PFT) 0, 0},
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

    build_primary_submenu(led_tests_submenu_table,
                          LED_TESTS_SUBMENU_TABLE_SIZE, "LED",
                          &led_submenup);

    build_secondary_submenu(led_tests_submenu_table,
                            LED_TESTS_SUBMENU_TABLE_SIZE,
                            led_tests_secondary_items);

    if (led_test_items_executed) {
        do_all_menu_items(&led_subtest_menu);
    } else {
        menu(&led_subtest_menu, led_tests_secondary_items, '\0');
    }

    return (PASSED);
}

/******************************************************************************
 *
 * Function: diag_env_led_test
 *
 * Description: System LED test
 *
 * Inputs      : None
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/
int diag_env_led_test (void)
{
    char *tname ="Environment LED Test";
    uint32_t origin_val = 0;

    testname(tname);
    prpass(testpass, "%s, ", tname);

    /* Back up Data */
    if (fpga_read_reg(FPGA_ENV_LED, &origin_val) != PASSED) {
        cterr('t', 0, "%d:Failed to write FPGA register.\n", __LINE__ );
        return (FAILED);
    }

    /* Environment LED Turn Green */
    if (fpga_register_operation(FPGA_ENV_LED, TABEI_LED_OFF, 
                                ENV_LED_GREEN) != PASSED) {
        cterr('t', 0, "%d:Failed to write FPGA register.\n", __LINE__ );
        return (FAILED);
    }

    /* Environment LED Turn Yellow */
    if (fpga_register_operation(FPGA_ENV_LED, TABEI_LED_OFF, 
                                ENV_LED_YELLOW) != PASSED) {
        cterr('t', 0, "%d:Failed to write FPGA register.\n", __LINE__ );
        return (FAILED);
    }

    /* Environment LED Turn OFF */
    if (fpga_register_operation(FPGA_ENV_LED, TABEI_LED_OFF, 
                                TABEI_LED_OFF) != PASSED) {
        cterr('t', 0, "%d:Failed to write FPGA register.\n", __LINE__ );
        return (FAILED);
    }

    /* Write Data Back */
    if (fpga_write_reg(FPGA_ENV_LED, origin_val) != PASSED) {
        cterr('t', 0, "%d:Failed to write FPGA register.\n", __LINE__ );
        return (FAILED);
    }

    prcomplete(testpass, errcount, (char *)0);
    return (PASSED);
}
/******************************************************************************
 *
 * Function: diag_status_led_test
 *
 * Description: Power Supply LED test
 *
 * Inputs      : None
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/
int diag_status_led_test (void)
{
    char *tname ="Status LED Test";
    uint32_t origin_val = 0;

    testname(tname);
    prpass(testpass, "%s, ", tname);
    
    /* Back up data*/
    if (cpld_read_reg(CPLD_STATUS_LED_REG, &origin_val) != PASSED) {
        cterr('t', 0, "%d:Failed to write CPLD register.\n", __LINE__ );
        return (FAILED);
    }

    /* Status LED Turn Green */
    if (cpld_register_operation(CPLD_STATUS_LED_REG, TABEI_LED_OFF, 
                                STATUS_LED_GREEN) != PASSED) {
        cterr('t', 0, "%d:Failed to write CPLD register.\n", __LINE__ );
        return (FAILED);
    }

    /* Status LED Turn Yellow */
    if (cpld_register_operation(CPLD_STATUS_LED_REG, TABEI_LED_OFF, 
                                STATUS_LED_YELLOW) != PASSED) {
        cterr('t', 0, "%d:Failed to write CPLD register.\n", __LINE__ );
        return (FAILED);
    }

    /* Status LED Turn OFF */
    if (cpld_register_operation(CPLD_STATUS_LED_REG, TABEI_LED_OFF, 
                                TABEI_LED_OFF) != PASSED) {
        cterr('t', 0, "%d:Failed to write CPLD register.\n", __LINE__ );
        return (FAILED);
    }

    /* Restore Data */
    if (cpld_write_reg(CPLD_STATUS_LED_REG, origin_val) != PASSED) {
        cterr('t', 0, "%d:Failed to write CPLD register.\n", __LINE__ );
        return (FAILED);
    }

    prcomplete(testpass, errcount, (char *)0);
    return (PASSED);
}

/******************************************************************************
 *
 * Function: diag_I350_RJ45_led_test
 *
 * Description: I350 RJ45 LED test
 *
 * Inputs      : None
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/
int diag_I350_RJ45_led_test (void)
{
    char *tname ="I350 RJ45 LED Test";

    testname(tname);
    prpass(testpass, "%s, ", tname);

    /* I350 RJ45 LED Turn Green */
    if (fpga_register_operation(FPGA_I350_RJ45_LED, TABEI_LED_OFF, 
                                RJ45_LED_ON) != PASSED) {
        cterr('t', 0, "%d:Failed to write FPGA register.\n", __LINE__ );
        return (FAILED);
    }

    /* I350 SFP LED Turn OFF */
    if (fpga_register_operation(FPGA_I350_RJ45_LED, TABEI_LED_OFF, 
                                TABEI_LED_OFF) != PASSED) {
        cterr('t', 0, "%d:Failed to write FPGA register.\n", __LINE__ );
        return (FAILED);
    }

    prcomplete(testpass, errcount, (char *)0);
    return (PASSED);
}
/******************************************************************************
 *
 * Function: diag_I350_SFP_led_test
 *
 * Description: I350 SFP LED test
 *
 * Inputs      : None
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/
int diag_I350_SFP_led_test (void)
{
    char *tname ="I350 SFP LED Test";

    testname(tname);
    prpass(testpass, "%s, ", tname);

    /* I350 SFP LED Turn Green */
    if (fpga_register_operation(FPGA_I350_SFP_LED, TABEI_LED_OFF, 
                                SFP_LED_GREEN) != PASSED) {
        cterr('t', 0, "%d:Failed to write FPGA register.\n", __LINE__ );
        return (FAILED);
    }

    /* I350 SFP LED Turn Yellow */
    if (fpga_register_operation(FPGA_I350_SFP_LED, TABEI_LED_OFF, 
                                SFP_LED_YELLOW) != PASSED) {
        cterr('t', 0, "%d:Failed to write FPGA register.\n", __LINE__ );
        return (FAILED);
    }

    /* I350 SFP LED Turn OFF */
    if (fpga_register_operation(FPGA_I350_SFP_LED, TABEI_LED_OFF, 
                                TABEI_LED_OFF) != PASSED) {
        cterr('t', 0, "%d:Failed to write FPGA register.\n", __LINE__ );
        return (FAILED);
    }

    prcomplete(testpass, errcount, (char *)0);
    return (PASSED);
}

/******************************************************************************
 *
 * Function: diag_hdd_led_test
 *
 * Description: HDD LED test
 *
 * Inputs      : None
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/
int diag_hdd_led_test (void)
{
    char *tname ="HDD LED Test";

    testname(tname);
    prpass(testpass, "%s, ", tname);

    /* HDD LED Turn Green */
    if (fpga_register_operation(FPGA_HDD_LED, TABEI_HDD_LED_MASK, 
                                HDD_LED_GREEN) != PASSED) {
        cterr('t', 0, "%d:Failed to turn it GREEN.\n", __LINE__ );
        return (FAILED);
    }

    /* HDD LED Turn Yellow */
    if (fpga_register_operation(FPGA_HDD_LED, TABEI_HDD_LED_MASK, 
                                HDD_LED_YELLOW) != PASSED) {
        cterr('t', 0, "%d:Failed to turn it YELLOW.\n", __LINE__ );
        return (FAILED);
    }

    /* HDD LED Turn OFF */
    if (fpga_register_operation(FPGA_HDD_LED, TABEI_HDD_LED_MASK, 
                                TABEI_LED_OFF) != PASSED) {
        cterr('t', 0, "%d:Failed to turn it OFF.\n", __LINE__ );
        return (FAILED);
    }

    prcomplete(testpass, errcount, (char *)0);
    return (PASSED);
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
    char *tname ="gephy0 LED Test";
    testname(tname);
    prpass(testpass, "%s, ", tname);

    if (access_gephy_led(TABEI_GE0_88E1514_PHY, GEPHY_ACCESS_ON) != PASSED) {
        cterr('t', 0, "%d:Failed to turn it ON.\n", __LINE__ );
        return (FAILED);
    }

    msleep(DELAY_FOR_LED_TEST);

    if (access_gephy_led(TABEI_GE0_88E1514_PHY, GEPHY_ACCESS_OFF) != PASSED) {
        cterr('t', 0, "%d:Failed to turn it OFF.\n", __LINE__ );
        return (FAILED);
    }
    prcomplete(testpass, errcount, (char *)0);
    return (PASSED);

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
    char *tname ="gephy1 LED Test";
    testname(tname);
    prpass(testpass, "%s, ", tname);

    if (access_gephy_led(TABEI_GE1_88E1514_PHY, GEPHY_ACCESS_ON) != PASSED) {
        cterr('t', 0, "%d:Failed to turn it ON.\n", __LINE__ );
        return (FAILED);
    }

    msleep(DELAY_FOR_LED_TEST);

    if (access_gephy_led(TABEI_GE1_88E1514_PHY, GEPHY_ACCESS_OFF) != PASSED) {
        cterr('t', 0, "%d:Failed to turn it OFF.\n", __LINE__ );
        return (FAILED);
    }

    prcomplete(testpass, errcount, (char *)0);
    return (PASSED);
}

/******************************************************************************
 *
 * Function: diag_gephy1543_led_test
 *
 * Description: GE PHY(Marvell 88E1543) LED test 
 *
 * Inputs      : None
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/
int diag_gephy1543_led_test (void)
{
    fortnite_is_not_support();
    return (PASSED);
}

/******************************************************************************
 *
 * Function: diag_esw6390_led_test
 *
 * Description: GE SWITCH(Marvell 88E6390) LED test 
 *
 * Inputs      : None
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/
int diag_esw6390_led_test (void)
{
    fortnite_is_not_support();
    return (PASSED);
}

/******************************************************************************
 *
 * Function: diag_port80_led_test
 *
 * Description: Port80 LED test
 *
 * Inputs      : None
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/
int diag_port80_led_test (void)
{
    char *tname ="Port80 LED Test";
    uint32_t reg_val = 0;
    int port80_bit;

    testname(tname);
    prpass(testpass, "%s, ", tname);
    
    /* Set read/write permission start from Port80 on, only one port*/
    if (ioperm(PORT80_ADDR, ACCESS_PORT_NUM, PORT80_ACCESS_ON) < 0) {
        cterr('f', 0, "Failed to set access of Port80 on.");
        return (FAILED);
    }

    for (port80_bit = PORT80_BIT0; port80_bit <= PORT80_BIT7; port80_bit++) {
        /* Turn on Port80 led with green*/
        reg_val = inb(PORT80_ADDR);
        outb(reg_val | (PORT80_LED_ON << port80_bit), PORT80_ADDR);
        msleep(DELAY_FOR_LED_TEST);
        
        /* Turn off Port80 led */
        reg_val = inb(PORT80_ADDR);
        outb(reg_val & PORT80_LED_ALL_OFF, PORT80_ADDR);
        msleep(DELAY_FOR_LED_TEST);
    }
    
    /* Set read/write permission start from Port80 off, only one port*/
    if (ioperm(PORT80_ADDR, ACCESS_PORT_NUM, PORT80_ACCESS_OFF) < 0) {
        cterr('f', 0, "Failed to set access of Port80 off");
        return (FAILED);
    }

    prcomplete(testpass, errcount, (char *)0);
    return (PASSED);
}

/******************************************************************************
 *
 * Function: diag_all_led_test
 *
 * Description: All LED test
 *
 * Inputs      : None
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/
int diag_all_led_test (void)
{
    char *tname ="All LED Test";

    testname(tname);
    prpass(testpass, "%s, ", tname);

    /* All LED Turn Green */
    if (fpga_register_operation(FPGA_HDD_LED, TABEI_HDD_LED_MASK, 
                                HDD_LED_GREEN) != PASSED) {
        cterr('t', 0, "%d:Failed to turn all LED GREEN.\n", __LINE__ );
        return (FAILED);
    }

    if (is_tabeil() == TRUE) {
        /* Turn on GE0 LED */
        if (access_gephy_led(TABEI_GE0_88E1514_PHY, GEPHY_ACCESS_ON) != PASSED) {
            cterr('t', 0, "%d:Failed to turn GE0 LED ON.\n", __LINE__ );
            return (FAILED);
        }
    
        /* Turn on GE1 LED */
        if (access_gephy_led(TABEI_GE1_88E1514_PHY, GEPHY_ACCESS_ON) != PASSED) {
            cterr('t', 0, "%d:Failed to turn GE0 LED ON.\n", __LINE__ );
            return (FAILED);
        }
    }

    /* Turn on Port80 led */
    if (access_port80_led(PORT80_ALL, PORT80_ACCESS_ON) != PASSED) {
        cterr('t', 0, "%d:Failed to turn Port80 LED ON.\n", __LINE__ );
        return (FAILED);
    }

    /* Promethium */
if (is_promethium() == TRUE) {
    /* Promethium for status LED */
    if (cpld_register_operation(CPLD_STATUS_LED_REG, TABEI_LED_MASK, 
                                STATUS_LED_GREEN) != PASSED) {
        cterr('t', 0, "%d:Failed to turn Status LED GREEN.\n", __LINE__ );
        return (FAILED);
    }

    /* Promethium for environment LED */
    if (fpga_register_operation(FPGA_ENV_LED, TABEI_LED_MASK, 
                                ENV_LED_GREEN) != PASSED) {
        cterr('t', 0, "%d:Failed to turn Environment LED GREEN.\n", __LINE__ );
        return (FAILED);
    }

}


    /* All LED Turn Yellow */
    if (fpga_register_operation(FPGA_HDD_LED, TABEI_HDD_LED_MASK, 
                                HDD_LED_YELLOW) != PASSED) {
        cterr('t', 0, "%d:Failed to turn all LED YELLOW.\n", __LINE__ );
        return (FAILED);
    }

    /* Promethium */
    if (is_promethium() == TRUE) {
        /* Promethium for status LED */
        if (cpld_register_operation(CPLD_STATUS_LED_REG, TABEI_LED_MASK, 
                                    STATUS_LED_YELLOW) != PASSED) {
            cterr('t', 0, "%d:Failed to turn Status LED YELLOW.\n", __LINE__ );
            return (FAILED);
        }
 
        /* Promethium for environment LED */
        if (fpga_register_operation(FPGA_ENV_LED, TABEI_LED_MASK, 
                                    ENV_LED_YELLOW) != PASSED) {
            cterr('t', 0, "%d:Failed to turn Environment LED YELLOW.\n", __LINE__ );
            return (FAILED);
        }
 
    }

    /* All LED Turn OFF */
    if (fpga_register_operation(FPGA_HDD_LED, TABEI_HDD_LED_MASK, 
                                TABEI_LED_OFF) != PASSED) {
        cterr('t', 0, "%d:Failed to turn all LED OFF.\n", __LINE__ );
        return (FAILED);
    }

    if (is_tabeil() == TRUE) {
        /* Turn off GE0 LED */
        if (access_gephy_led(TABEI_GE0_88E1514_PHY, GEPHY_ACCESS_OFF) != PASSED) {
            cterr('t', 0, "%d:Failed to turn GE0 LED OFF.\n", __LINE__ );
            return (FAILED);
        }
    
        /* Turn off GE1 LED */
        if (access_gephy_led(TABEI_GE1_88E1514_PHY, GEPHY_ACCESS_OFF) != PASSED) {
            cterr('t', 0, "%d:Failed to turn GE1 LED OFF.\n", __LINE__ );
            return (FAILED);
        }
    }

    /* Turn off Port80 led */
    if (access_port80_led(PORT80_ALL, PORT80_ACCESS_OFF) != PASSED) {
        cterr('t', 0, "%d:Failed to turn Port80 LED OFF.\n", __LINE__ );
        return (FAILED);
    }

    /* Promethium */
    if (is_promethium() == TRUE) {
        /* Promethium for status LED */
        if (cpld_register_operation(CPLD_STATUS_LED_REG, TABEI_LED_MASK, 
                                    TABEI_LED_OFF) != PASSED) {
            cterr('t', 0, "%d:Failed to turn Status LED OFF.\n", __LINE__ );
            return (FAILED);
        }
        /* Promethium for environment LED */
        if (fpga_register_operation(FPGA_ENV_LED, TABEI_LED_MASK, 
                                    TABEI_LED_OFF) != PASSED) {
            cterr('t', 0, "%d:Failed to turn Environment LED OFF.\n", __LINE__ );
            return (FAILED);
        }

    }


    prcomplete(testpass, errcount, (char *)0);
    return (PASSED);
}

/*-------------------------------------------------
 * $Log: diag_led_test.c,v $
 * Revision 1.5  2020/08/06 07:54:55  kehuang2
 * Collapse Promethium into main trunk
 *
 * Revision 1.4  2019/12/30 06:02:00  kehuang2
 * CSCvs55860: Support All LED ON/OFF
 *
 * Revision 1.3  2019/11/25 08:55:52  kehuang2
 * Collapse Tabei-L into main trunk
 *
 * Revision 1.2  2019/10/17 02:16:22  kehuang2
 * Collapse Tabei-L into main trunk
 *
 * Revision 1.1.4.20  2019/09/27 07:57:23  kehuang2
 * Clean up code
 *
 * Revision 1.1.4.19  2019/09/20 07:00:34  kehuang2
 * Clean up code base on review comment
 *
 * Revision 1.1.4.18  2019/09/02 08:42:56  kehuang2
 * Clean up code by the comment of code review
 *
 * Revision 1.1.4.17  2019/08/29 03:49:27  kehuang2
 * Clean up code by the comment of code review
 *
 * Revision 1.1.4.16  2019/08/06 07:20:28  kehuang2
 * Update present function base on the comment of code review
 *
 * Revision 1.1.4.15  2019/07/08 01:49:20  kehuang2
 * Rename variable to avoid redefined with PIM module
 *
 * Revision 1.1.4.14  2019/07/04 03:23:36  kehuang2
 * Combine Tabei-L sereies image together(Fortnite, Tabei-L, Promethium)
 *
 * Revision 1.1.4.13  2019/05/31 08:19:00  kehuang2
 * Update LED Test for Fortnite
 *
 * Revision 1.1.4.12  2019/05/21 09:18:51  kehuang2
 * Support Port80 LED
 *
 * Revision 1.1.4.11  2019/05/21 03:18:00  kehuang2
 *
 * 1.SFP EN LED Support base on PreP2B respin
 * 2.Support SFP Mux access utility
 *
 * Revision 1.1.4.10  2019/05/16 08:48:14  kehuang2
 * Clean up code by the comment of code review.
 *
 * Revision 1.1.4.9  2019/04/29 08:14:26  kehuang2
 * Clean up code
 *
 * Revision 1.1.4.8  2019/04/24 07:59:21  kehuang2
 * Update CPLD access
 *
 * Revision 1.1.4.7  2019/04/19 03:15:29  kehuang2
 * 1.Support CPLD access 2.Support new FPGA 3.Clean up code
 *
 * Revision 1.1.4.6  2019/03/26 09:58:45  kehuang2
 * Support LED Test
 *
 * Revision 1.1.4.5  2018/12/26 03:48:33  harrchan
 * LED Test
 *
 * Revision 1.1.4.4  2018/11/16 05:42:11  olin2
 * Clean up code
 *
 * Revision 1.1.4.3  2018/10/24 02:47:27  harrchan
 * 88E1514 GEPHY test
 *
 * Revision 1.1.4.2  2018/10/02 01:49:59  harrchan
 * Initial commit for Tabei-L P1A bring up.
 *
 * $Endlog$
 *-------------------------------------------------
 */
