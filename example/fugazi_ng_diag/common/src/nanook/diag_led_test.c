 /* $Id: diag_led_test.c,v 1.2 2019/12/11 10:10:30 lucywang Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/nanook/diag_led_test.c,v $
 *------------------------------------------------------------------
 *
 * diag_led_test.c - This file is for LED test.
 *
 * Copyright (c) 2019 by Cisco Systems, Inc.
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
#include "dash_fpga.h"
#include "platform_fru.h"
#include "platform_cookie.h"
#include "plat_defs.h"
#include "dnv_eth_lib.h"
#include "diag_led_util.h"
#include "diag_led_test.h"
#include "common_utils.h"
#include "diag_led_util.h"
#include "diag_cpld_lib.h"
#include "diag_esw_lib.h"
#include "diag_esw_util.h"


/*
 * Global variables
 */

extern int marvell_cpssPpInit_xcat3;
int cpssPpInit_xcat3_called_from_led_menu = FALSE;

/* Local functions */
int build_led_util_menu(boolean);
int diag_led_test(boolean);
int diag_system_led_test(void);
int diag_88e1543_RJ45_led_test (int);
int diag_88e1543_SFP_led_test (int);
int diag_async_fpga_led_test (int);
int diag_esw_phy_led_test (void);



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

    {"88E1543 RJ45 PORT0 LED Test",
    (PFT) diag_88e1543_RJ45_led_test, 0,
    MF_CONTINUOUS | MF_DOGRP | MF_DOALL,
    (type_t(*)())0, 0, 
    (PFT) diag_88e1543_RJ45_led_test, TRUE},

    {"88E1543 RJ45 PORT1 LED Test",
    (PFT) diag_88e1543_RJ45_led_test, 1,
    MF_CONTINUOUS | MF_DOGRP | MF_DOALL,
    (type_t(*)())0, 0, 
    (PFT) diag_88e1543_RJ45_led_test, TRUE},

    {"88E1543 SFP PORT0 LED Test",
    (PFT) diag_88e1543_SFP_led_test, 0,
    MF_CONTINUOUS | MF_DOGRP | MF_DOALL,
    (type_t(*)())0, 0, 
    (PFT) diag_88e1543_SFP_led_test, TRUE},

    {"88E1543 SFP PORT1 LED Test",
    (PFT) diag_88e1543_SFP_led_test, 1,
    MF_CONTINUOUS | MF_DOGRP | MF_DOALL,
    (type_t(*)())0, 0, 
    (PFT) diag_88e1543_SFP_led_test, TRUE},

    {"Crocus 16 FPGA LED 0 Test",
    (PFT) diag_async_fpga_led_test, 0,
    MF_CONTINUOUS | MF_DOGRP | MF_DOALL,
    (type_t(*)())0, 0, 
    (PFT) diag_async_fpga_led_test, TRUE},

    {"Crocus 16 FPGA LED 1 Test",
    (PFT) diag_async_fpga_led_test, 1,
    MF_CONTINUOUS | MF_DOGRP | MF_DOALL,
    (type_t(*)())0, 0, 
    (PFT) diag_async_fpga_led_test, TRUE},

    {"Crocus 32 FPGA LED 0 Test",
    (PFT) diag_async_fpga_led_test, 2,
    MF_CONTINUOUS | MF_DOGRP | MF_DOALL,
    (type_t(*)())0, 0, 
    (PFT) diag_async_fpga_led_test, TRUE},

    {"Crocus 32 FPGA LED 2 Test",
    (PFT) diag_async_fpga_led_test, 3,
    MF_CONTINUOUS | MF_DOGRP | MF_DOALL,
    (type_t(*)())0, 0, 
    (PFT) diag_async_fpga_led_test, TRUE},

    {"Crocus 32 FPGA LED 3 Test",
    (PFT) diag_async_fpga_led_test, 4,
    MF_CONTINUOUS | MF_DOGRP | MF_DOALL,
    (type_t(*)())0, 0, 
    (PFT) diag_async_fpga_led_test, TRUE},

    {"Crocus 32 FPGA LED 4 Test",
    (PFT) diag_async_fpga_led_test, 5,
    MF_CONTINUOUS | MF_DOGRP | MF_DOALL,
    (type_t(*)())0, 0, 
    (PFT) diag_async_fpga_led_test, TRUE},

    {"ESW PHY LED Test",
    (PFT) diag_esw_phy_led_test, FALSE,
    MF_CONTINUOUS | MF_DOGRP | MF_DOALL,
    (type_t(*)())is_nanook_plus, 0, 
    (PFT) diag_esw_phy_led_test, TRUE},

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

    {"Async FPGA LED Utility",
     (PFT) diag_async_led_util, FALSE, 0,
     (type_t(*)())0, 0, (PFT) 0, 0},

    {"88E1543 RJ45 LED Utility",
     (PFT) diag_88E1543_RJ45_led_util, FALSE, 0,
     (type_t(*)())0, 0, (PFT) 0, 0},

    {"88E1543 SFP LED Utility",
     (PFT) diag_88E1543_SFP_led_util, FALSE, 0,
     (type_t(*)())0, 0, (PFT) 0, 0},

    {"ESW PHY LED Utility",
     (PFT) diag_esw_phy_led_util, FALSE, 0,
     (type_t(*)())is_nanook_plus, 0, (PFT) 0, 0},

    { "turn all Green LED on",
	 (PFT)diag_all_green_leds_on,  0, 0,
     (type_t(*)())0, 0, (PFT) 0, 0},
     
    { "turn all Yellow LED on",
	 (PFT)diag_all_yellow_leds_on,  0, 0,
     (type_t(*)())0, 0, (PFT) 0, 0},
     
    { "turn all LED off",
	 (PFT)diag_all_leds_off,  0, 0,
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

    if (is_nanook_plus()) {
        if (marvell_cpssPpInit_xcat3 == FALSE) {
		
            system(ETH_INSMOD_AC3_NIM_DM_MODULE);

            /* Init Switch */
            if (diag_esw_init() != PASSED) {
                cterr('f', 0, "Failed to init Switch.");
            }
            cpssPpInit_xcat3_called_from_led_menu = TRUE;
        }
    }
    

    build_primary_submenu(led_util_submenu_table, LED_UTIL_SUBMENU_TABLE_SIZE,
                          "LED util SubMenu", &led_util_submenup);
    build_secondary_submenu(led_util_submenu_table, LED_UTIL_SUBMENU_TABLE_SIZE,
                            led_util_secondary_items);
    menu(&led_util_subtest_menu, led_util_secondary_items, 0);

    if (is_nanook_plus()) {
        if(cpssPpInit_xcat3_called_from_led_menu == TRUE) {
            diag_esw_exit();
            cpssPpInit_xcat3_called_from_led_menu = FALSE;
        }
    }

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
    char *tname ="LED Test";

    testname(tname);

    if (is_nanook_plus()) {
	
    system(ETH_INSMOD_AC3_NIM_DM_MODULE);
	
        /* Init Switch */
        if (diag_esw_init() != PASSED) {
            cterr('f', 0, "Failed to init Switch.");
        }
    }

    /* Turn off all LED before LED test. */
    if (diag_all_leds_off() != PASSED) {
        cterr('f', 0, "Failed to turn off all LED before testing.");
    }

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

    if (is_nanook_plus()) {
        diag_esw_exit();
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
	
 
    reg_offset = FPGA_LPC_LED_CTRL_REG;
        
    /* Turn on system led with green */
    reg_val &= SYS_LED_OFF;
    reg_val |= SYS_LED_GREEN; 
    if (cpld_write_reg(reg_offset, reg_val) != PASSED) {
        printf("Failed to write FPGA register 0x%04X.\n", reg_offset);
        return (FAILED);
    }

    msleep(DELAY_FOR_LED_TEST);

    if (cpld_read_reg(reg_offset, &reg_val) != PASSED) {
        printf("Failed to read FPGA register 0x%04X.\n", reg_offset);
        return (FAILED);
    }
            
    /* Turn on system led with amber */
    reg_val &= SYS_LED_OFF;
    reg_val |= SYS_LED_AMBER;

    if (cpld_write_reg(reg_offset, reg_val) != PASSED) {
        printf("Failed to write FPGA register 0x%04X.\n", reg_offset);
        return (FAILED);
    }

    msleep(DELAY_FOR_LED_TEST);

    if (cpld_read_reg(reg_offset, &reg_val) != PASSED) {
        printf("Failed to read FPGA register 0x%04X.\n", reg_offset);
        return (FAILED);
    }

    /* Turn on system led with amber blink*/
    reg_val &= SYS_LED_OFF;
    reg_val |= SYS_LED_AMBER_BLINK;

    if (cpld_write_reg(reg_offset, reg_val) != PASSED) {
        printf("Failed to write FPGA register 0x%04X.\n", reg_offset);
        return (FAILED);
    }
	 
    msleep(DELAY_FOR_LED_TEST);

    /* Turn off all led */
    reg_val &= LED_OFF;
    if (cpld_write_reg(reg_offset, reg_val) != PASSED) {
        printf("Failed to write FPGA register 0x%04X.\n", reg_offset);
        return (FAILED);
    }

    prcomplete(testpass, errcount, (char *)0);
    return (PASSED);
}


/******************************************************************************
 *
 * Function: diag_88e1543_RJ45_led_test
 *
 * Description: 88E1543 RJ45 LED test
 *
 * Inputs      : test_target: 0-PORT0 1-PORT1
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/
int diag_88e1543_RJ45_led_test (int test_target)
{
    char *tname;
    uint32_t reg_offset = 0, reg_val = 0;

    if (test_target == 0) {
	 tname = "88E1543 RJ45 PORT0 LED Test";
    } else {
        tname = "88E1543 RJ45 PORT1 LED Test";
    }

    testname(tname);
    prpass(testpass, "%s, ", tname);

    reg_offset = FPGA_LED_BLINK_EN_REG;
    /* Access FPGA Register for controll system LED */
    if (dash_fpga_reg_read(reg_offset, &reg_val) != PASSED) {
        printf("Failed to read FPGA register 0x%04X.\n", reg_offset);
        return (FAILED);
    } 
    /* Disable Blink Control for RJ45 port */
    reg_val &= ~(RJ45_PORT1_BLINK_CTRL | RJ45_PORT0_BLINK_CTRL);
    if (dash_fpga_reg_write(reg_offset, reg_val) != PASSED) {
        printf("Failed to write FPGA register 0x%04X.\n", reg_offset);
        return (FAILED);
    }

    reg_offset = FPGA_LED_RJ45_ONOFF_REG;
    /* Access FPGA Register for controll system LED */
    if (dash_fpga_reg_read(reg_offset, &reg_val) != PASSED) {
        printf("Failed to read FPGA register 0x%04X.\n", reg_offset);
        return (FAILED);
    }

    reg_val &= LED_OFF;
    /* Enable RJ45 speed and link LED */
    if (test_target == 0) {
        /* PORT 0 */
        reg_val |= (FPGA_RJ45_PORT0_GREEN_LINK_LED | FPGA_RJ45_PORT0_SPEED_LED);
    } else if (test_target == 1) {
        /* PORT 1 */
        reg_val |= (FPGA_RJ45_PORT1_GREEN_LINK_LED | FPGA_RJ45_PORT1_SPEED_LED);
    } else {
        /* PORT 0 & 1 */
        reg_val |= ( FPGA_RJ45_PORT1_GREEN_LINK_LED | FPGA_RJ45_PORT1_SPEED_LED |
            FPGA_RJ45_PORT0_GREEN_LINK_LED | FPGA_RJ45_PORT0_SPEED_LED);
    }
    if (dash_fpga_reg_write(reg_offset, reg_val) != PASSED) {
        printf("Failed to write FPGA register 0x%04X.\n", reg_offset);
        return (FAILED);
    }

    msleep(DELAY_FOR_LED_TEST);

    reg_offset = FPGA_LED_BLINK_EN_REG;
    /* Access FPGA Register for controll system LED */
    if (dash_fpga_reg_read(reg_offset, &reg_val) != PASSED) {
        printf("Failed to read FPGA register 0x%04X.\n", reg_offset);
        return (FAILED);
    } 
    /* Disable Blink Control for RJ45 port */
    reg_val &= ~(RJ45_PORT1_BLINK_CTRL | RJ45_PORT0_BLINK_CTRL);
    if (dash_fpga_reg_write(reg_offset, reg_val) != PASSED) {
        printf("Failed to write FPGA register 0x%04X.\n", reg_offset);
        return (FAILED);
    }

    reg_offset = FPGA_LED_RJ45_ONOFF_REG;
    /* Access FPGA Register for controll system LED */
    if (dash_fpga_reg_read(reg_offset, &reg_val) != PASSED) {
        printf("Failed to read FPGA register 0x%04X.\n", reg_offset);
        return (FAILED);
    }
    reg_val &= LED_OFF;
    /* Enable RJ45 amber link LED */
    if (test_target == 0) {
        /* PORT 0 */
        reg_val |= (FPGA_RJ45_PORT0_YELLOW_LINK_LED);
    } else if (test_target == 1) {
        /* PORT 1 */
        reg_val |= (FPGA_RJ45_PORT1_YELLOW_LINK_LED);
    } else {
        /* PORT 0 & 1 */
        reg_val |= (FPGA_RJ45_PORT1_YELLOW_LINK_LED | FPGA_RJ45_PORT0_YELLOW_LINK_LED);
    }
    if (dash_fpga_reg_write(reg_offset, reg_val) != PASSED) {
        printf("Failed to write FPGA register 0x%04X.\n", reg_offset);
        return (FAILED);
    }

    msleep(DELAY_FOR_LED_TEST);

    reg_offset = FPGA_LED_BLINK_EN_REG;
    /* Access FPGA Register for controll system LED */
    if (dash_fpga_reg_read(reg_offset, &reg_val) != PASSED) {
        printf("Failed to read FPGA register 0x%04X.\n", reg_offset);
        return (FAILED);
    } 
    /* Disable Blink Control for RJ45 port */
    reg_val &= ~(RJ45_PORT1_BLINK_CTRL | RJ45_PORT0_BLINK_CTRL);
    if (dash_fpga_reg_write(reg_offset, reg_val) != PASSED) {
        printf("Failed to write FPGA register 0x%04X.\n", reg_offset);
        return (FAILED);
    }

    reg_offset = FPGA_LED_RJ45_ONOFF_REG;
    /* Access FPGA Register for controll system LED */
    if (dash_fpga_reg_read(reg_offset, &reg_val) != PASSED) {
        printf("Failed to read FPGA register 0x%04X.\n", reg_offset);
        return (FAILED);
    }
    /* Turn off all led */
    reg_val &= LED_OFF;
    if (dash_fpga_reg_write(reg_offset, reg_val) != PASSED) {
        printf("Failed to write FPGA register 0x%04X.\n", reg_offset);
        return (FAILED);
    }
			
    prcomplete(testpass, errcount, (char *)0);
    return (PASSED);
}


/******************************************************************************
 *
 * Function: diag_88e1543_SFP_led_test
 *
 * Description: 88E1543 SFP LED test
 *
 * Inputs      : test_target: 0-PORT0 1-PORT1
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/
int diag_88e1543_SFP_led_test (int test_target)
{
    char *tname;
    uint32_t reg_offset = 0, reg_val = 0;

    if (test_target == 0) {
	 tname = "88E1543 SFP PORT0 LED Test";
    } else {
        tname = "88E1543 SFP PORT1 LED Test";
    }

    testname(tname);
    prpass(testpass, "%s, ", tname);
	
    reg_offset = FPGA_LED_BLINK_EN_REG;
    /* Access FPGA Register for controll system LED */
    if (dash_fpga_reg_read(reg_offset, &reg_val) != PASSED) {
        printf("Failed to read FPGA register 0x%04X.\n", reg_offset);
        return (FAILED);
    } 
    /* Disable Blink Control for RJ45 port */
    reg_val &= ~(SFP_PORT1_BLINK_CTRL | SFP_PORT0_BLINK_CTRL);
    if (dash_fpga_reg_write(reg_offset, reg_val) != PASSED) {
        printf("Failed to write FPGA register 0x%04X.\n", reg_offset);
        return (FAILED);
    }

    reg_offset = FPGA_LED_SFP_ONOFF_REG;
    /* Access FPGA Register for controll system LED */
    if (dash_fpga_reg_read(reg_offset, &reg_val) != PASSED) {
        printf("Failed to read FPGA register 0x%04X.\n", reg_offset);
        return (FAILED);
    }

    reg_val &= LED_OFF;
    /* Enable SFP speed and link LED */
    if (test_target == 0) {
        /* PORT 0 */
        reg_val |= (FPGA_SFP_PORT0_GREEN_LINK_LED | FPGA_SFP_PORT0_SPEED_LED);
    } else if (test_target == 1) {
        /* PORT 1 */
        reg_val |= (FPGA_SFP_PORT1_GREEN_LINK_LED | FPGA_SFP_PORT1_SPEED_LED);
    } else {
        /* PORT 0 & 1 */
        reg_val |= ( FPGA_SFP_PORT1_GREEN_LINK_LED | FPGA_SFP_PORT1_SPEED_LED |
        FPGA_SFP_PORT0_GREEN_LINK_LED | FPGA_SFP_PORT0_SPEED_LED);
    }
    if (dash_fpga_reg_write(reg_offset, reg_val) != PASSED) {
        printf("Failed to write FPGA register 0x%04X.\n", reg_offset);
        return (FAILED);
    }
 
    msleep(DELAY_FOR_LED_TEST);	

    reg_offset = FPGA_LED_BLINK_EN_REG;
    /* Access FPGA Register for controll system LED */
    if (dash_fpga_reg_read(reg_offset, &reg_val) != PASSED) {
        printf("Failed to read FPGA register 0x%04X.\n", reg_offset);
        return (FAILED);
    } 
    /* Disable Blink Control for RJ45 port */
    reg_val &= ~(SFP_PORT1_BLINK_CTRL | SFP_PORT0_BLINK_CTRL);
    if (dash_fpga_reg_write(reg_offset, reg_val) != PASSED) {
        printf("Failed to write FPGA register 0x%04X.\n", reg_offset);
        return (FAILED);
    }

    reg_offset = FPGA_LED_SFP_ONOFF_REG;
    /* Access FPGA Register for controll system LED */
    if (dash_fpga_reg_read(reg_offset, &reg_val) != PASSED) {
        printf("Failed to read FPGA register 0x%04X.\n", reg_offset);
        return (FAILED);
    }
    reg_val &= LED_OFF;
    /* Enable SFP amber link LED */
    if (test_target == 0) {
        /* PORT 0 */
        reg_val |= (FPGA_SFP_PORT0_YELLOW_LINK_LED);
    } else if (test_target == 1) {
        /* PORT 1 */
        reg_val |= (FPGA_SFP_PORT1_YELLOW_LINK_LED);
    } else {
        /* PORT 0 & 1 */
        reg_val |= (FPGA_SFP_PORT1_YELLOW_LINK_LED | FPGA_RJ45_PORT0_YELLOW_LINK_LED);
    }
    if (dash_fpga_reg_write(reg_offset, reg_val) != PASSED) {
        printf("Failed to write FPGA register 0x%04X.\n", reg_offset);
        return (FAILED);
    }

    msleep(DELAY_FOR_LED_TEST);

    reg_offset = FPGA_LED_BLINK_EN_REG;
    /* Access FPGA Register for controll system LED */
    if (dash_fpga_reg_read(reg_offset, &reg_val) != PASSED) {
        printf("Failed to read FPGA register 0x%04X.\n", reg_offset);
        return (FAILED);
    } 
    /* Disable Blink Control for RJ45 port */
    reg_val &= ~(SFP_PORT1_BLINK_CTRL | SFP_PORT0_BLINK_CTRL);
    if (dash_fpga_reg_write(reg_offset, reg_val) != PASSED) {
        printf("Failed to write FPGA register 0x%04X.\n", reg_offset);
        return (FAILED);
    }

    reg_offset = FPGA_LED_SFP_ONOFF_REG;
    /* Access FPGA Register for controll system LED */
    if (dash_fpga_reg_read(reg_offset, &reg_val) != PASSED) {
        printf("Failed to read FPGA register 0x%04X.\n", reg_offset);
        return (FAILED);
    }
    /* Turn off all led */
    reg_val &= LED_OFF;
    if (dash_fpga_reg_write(reg_offset, reg_val) != PASSED) {
        printf("Failed to write FPGA register 0x%04X.\n", reg_offset);
        return (FAILED);
    }
	
    prcomplete(testpass, errcount, (char *)0);
    return (PASSED);
}


/******************************************************************************
 *
 * Function: diag_async_fpga_led_test
 *
 * Description: Async FPGA LED test
 *
 * Inputs      : test_target: 0-PORT0 1-PORT1
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/
int diag_async_fpga_led_test (int test_target)
{
    char *tname;
    uint32_t reg_offset = 0, reg_val = 0;

    if (test_target == 0) {
	 tname = "Crocus 16 FPGA LED 0 Test";
    } else if (test_target == 1){
        tname = "Crocus 16 FPGA LED 1 Test";
    } else if (test_target == 2){
        tname = "Crocus 32 FPGA LED 0 Test";
    } else if (test_target == 3){
        tname = "Crocus 32 FPGA LED 1 Test";
    } else if (test_target == 4){
        tname = "Crocus 32 FPGA LED 2 Test";
    } else {
        tname = "Crocus 32 FPGA LED 3 Test";
    }

    testname(tname);
    prpass(testpass, "%s, ", tname);

    /* Turn on system led with green */
    reg_offset = FPGA_LED_CROCUS_FPGA_CTRL_REG;

    if (test_target == 0) {
        reg_val |= CROCUS_16_FPGA_LED0;
    } else if (test_target == 1) {
        reg_val |= CROCUS_16_FPGA_LED1;
    } else if (test_target == 2) {
        reg_val |= CROCUS_32_FPGA_LED0;
    } else if (test_target == 3) {
        reg_val |= CROCUS_32_FPGA_LED1;
    } else if (test_target == 4) {
        reg_val |= CROCUS_32_FPGA_LED2;
    } else if (test_target == 5) {
        reg_val |= CROCUS_32_FPGA_LED3;
    }else {
        reg_val |= FPGA_LED_CROCUS_FPGA_LED_GREEN;
    }

    if (dash_fpga_reg_write(reg_offset, reg_val) != PASSED) {
        printf("Failed to write FPGA register 0x%04X.\n", reg_offset);
        return (FAILED);
    }

    msleep(DELAY_FOR_LED_TEST);

    /* Turn off all led */
    reg_offset = FPGA_LED_CROCUS_FPGA_CTRL_REG;
    /* Access FPGA Register for controll async LED */
    if (dash_fpga_reg_read(reg_offset, &reg_val) != PASSED) {
        printf("Failed to read FPGA register 0x%04X.\n", reg_offset);
        return (FAILED);
    }
    if (test_target == 0) {
        reg_val &= ~(CROCUS_16_FPGA_LED0);
    } else if (test_target == 1) {
        reg_val &= ~(CROCUS_16_FPGA_LED1);
    } else if (test_target == 2) {
        reg_val &= ~(CROCUS_32_FPGA_LED0);
    } else if (test_target == 3) {
        reg_val &= ~(CROCUS_32_FPGA_LED1);
    } else if (test_target == 4) {
        reg_val &= ~(CROCUS_32_FPGA_LED2);
    } else if (test_target == 5) {
        reg_val &= ~(CROCUS_32_FPGA_LED3);
    }else {
        reg_val = FPGA_LED_CROCUS_FPGA_LED_OFF;
    }
    if (dash_fpga_reg_write(reg_offset, reg_val) != PASSED) {
        printf("Failed to write FPGA register 0x%04X.\n", reg_offset);
        return (FAILED);
    }
	
    prcomplete(testpass, errcount, (char *)0);
    return (PASSED);
}


/******************************************************************************
 *
 * Function: diag_esw_phy_led_test
 *
 * Description: ESW PHY LED test
 *
 * Inputs      : None
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/
int diag_esw_phy_led_test (void)
{
    char *tname ="ESW PHY LED Test";

    testname(tname);
    prpass(testpass, "%s, ", tname);

    diag_esw_all_phy_led_on();

    msleep(DELAY_FOR_LED_TEST);

    diag_esw_all_phy_led_off();

    msleep(DELAY_FOR_LED_TEST);

    prcomplete(testpass, errcount, (char *)0);
    return (PASSED);
}


/*-------------------------------------------------
 * $Log: diag_led_test.c,v $
 * Revision 1.2  2019/12/11 10:10:30  lucywang
 * Merged Nanook to main trunk
 *
 *
 * $Endlog$
 *-------------------------------------------------
 */
