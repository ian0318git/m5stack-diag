/* $Id: diag_led_test.c,v 1.7 2020/02/04 08:49:42 alicehua Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/nutella/diag_led_test.c,v $
 *------------------------------------------------------------------
 *
 * diag_led_test.c - This file is for LED test.
 *
 *
 * Copyright (c) 2008-2019 by Cisco Systems, Inc.
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
#include "diag_fpga_lib.h"
#include "platform_fru.h"
#include "platform_cookie.h"
#include "plat_defs.h"
#include "dnv_eth_lib.h"
#include "diag_gephy_lib.h"
#include "diag_led_util.h"
#include "diag_led_test.h"
#include "sys/io.h"

/*
 * Global variables
 */


/* Local functions */
int build_led_util_menu(boolean);
int diag_led_test(boolean);
int diag_system_led_test(void);
int diag_vpn_led_test(void);
int diag_lte_led_test(void);
int diag_gephy_led_test(void);
int diag_I350_SFP_led_test(void);
int diag_port80_led_test(void);

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

    {"MRV88E1543 GE PHY LED Test",
    (PFT) diag_gephy_led_test, FALSE,
    MF_CONTINUOUS | MF_DOGRP | MF_DOALL,
    (type_t(*)())0, 0, 
    (PFT) diag_gephy_led_test, TRUE},
    
    {"I350 SFP LED Test",
    (PFT) diag_I350_SFP_led_test, FALSE,
    MF_CONTINUOUS | MF_DOGRP | MF_DOALL,
    (type_t(*)())has_sfp_sku, 0, 
    (PFT) diag_I350_SFP_led_test, TRUE},
    
    {"Port80 LED Test",
    (PFT) diag_port80_led_test, FALSE,
    MF_CONTINUOUS | MF_DOGRP | MF_DOALL,
    (type_t(*)())0, 0, 
    (PFT) diag_port80_led_test, TRUE},
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

    {"MRV88E1543 GE PHY LED Utility",
     (PFT) diag_gephy_led_util, FALSE, 0,
     (type_t(*)())0, 0, (PFT) 0, 0},
    
    {"I350 SFP LED Utility",
     (PFT) diag_i350_led_util, FALSE, 0,
     (type_t(*)())has_sfp_sku, 0, (PFT) 0, 0},
    
    {"Port80 LED Utility",
     (PFT) diag_port80_led_util, FALSE, 0,
     (type_t(*)())0, 0, (PFT) 0, 0},

    {"Turn All Led Green On",
     (PFT) diag_all_led_on_util, GREEN, 0,
     (type_t(*)())0, 0, (PFT) 0, 0},

    {"Turn All LED Yellow On",
     (PFT) diag_all_led_on_util, YELLOW, 0,
     (type_t(*)())0, 0, (PFT) 0, 0},

    {"Turn All Led Off",
     (PFT) diag_all_led_off_util, 0, 0,
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
    int rc = FAILED, rc2 = FAILED, rc3 = FAILED;
    int on = 1, off = 0;
    
    testname(tname);
    prpass(testpass, "%s, ", tname);

    if (fpga_version_is_more_than_2p0() == FALSE) {
        /* Turn on system led with green */
        rc = fpga_register_operation(FPGA_LED_REG, SYS_OK_LED_GREEN, on);
    
        /* Turn on system led with amber */
        rc2 = fpga_register_operation(FPGA_LED_REG, SYS_OK_LED_AMBER, on);
    
        /* Turn off all led */
        rc3 = fpga_register_operation(FPGA_LED_REG, LED_OFF, off);
    } else {
        /* Turn on system led with green */
        rc = fpga_register_operation(CEDGE_LPC_STATUS_LED_REG,
                                     CEDGE_SYS_OK_LED_GREEN, on);
    
        /* Turn on system led with amber */
        rc2 = fpga_register_operation(CEDGE_LPC_STATUS_LED_REG,
                                      CEDGE_SYS_OK_LED_AMBER, on);
    
        /* Turn off all led */
        rc3 = fpga_register_operation(CEDGE_LPC_STATUS_LED_REG, LED_OFF, off);

    }

    prcomplete(testpass, errcount, (char *)0);
    return (rc | rc2 | rc3);
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
    int rc = FAILED, rc2 = FAILED, rc3 = FAILED;
    int on = 1, off = 0;

    testname(tname);
    prpass(testpass, "%s, ", tname);

    if (fpga_version_is_more_than_2p0() == FALSE) {
        /* Turn on vpn led with green*/
        rc = fpga_register_operation(FPGA_LED_REG, VPN_OK_LED_GREEN, on);
         
        /* Turn on vpn led with amber*/
        rc2 = fpga_register_operation(FPGA_LED_REG, VPN_OK_LED_YELLOW, on);
         
        /* Turn off all led*/
        rc3 = fpga_register_operation(FPGA_LED_REG, LED_OFF, off); 
    } else {
        /* Turn on vpn led with green*/
        rc = fpga_register_operation(CEDGE_LPC_STATUS_LED_REG, VPN_OK_LED_GREEN, on);
        
        /* Turn on vpn led with amber*/
        rc2 = fpga_register_operation(CEDGE_LPC_STATUS_LED_REG, VPN_OK_LED_YELLOW, on);
        
        /* Turn off all led*/
        rc3 = fpga_register_operation(CEDGE_LPC_STATUS_LED_REG, LED_OFF, off); 
    
    }
    
    prcomplete(testpass, errcount, (char *)0);
    return (rc | rc2 | rc3);
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
    int rc = FAILED, rc2 = FAILED, rc3 = FAILED, rc4 = FAILED,
        rc5 = FAILED,rc6 = FAILED, rc7 = FAILED;
    int on = 1, off = 0;

    testname(tname);
    prpass(testpass, "%s, ", tname);

    /* Turn on LTE Modem RSSI with high RSSI */
    rc = fpga_register_operation(FPGA_LTE_RSSI_LED, LTE_MOD_HIGH_RSSI, on);
    
    /* Turn on LTE Modem RSSI with medium RSSI */
    rc2 = fpga_register_operation(FPGA_LTE_RSSI_LED, LTE_MOD_MEDIUM_RSSI, on);
    
    /* Turn on LTE Modem RSSI with low RSSI */
    rc3 = fpga_register_operation(FPGA_LTE_RSSI_LED, LTE_MOD_LOW_RSSI, on);

    /* Turn on LTE Modem RSSI with weak RSSI */
    rc4 = fpga_register_operation(FPGA_LTE_RSSI_LED, LTE_MOD_RSSI, on);

    /* Turn on LTE Modem RSSI with no RSSI */
    rc5 = fpga_register_operation(FPGA_LTE_RSSI_LED, LTE_MOD_NO_RSSI, on);
    
    /* Turn on LTE SIM card LED*/
    rc6 = fpga_register_operation(FPGA_LED_REG, LTE_SIM_ACT_LED, on);

    /* Turn off all led */
    rc7 = fpga_register_operation(FPGA_LED_REG, LED_OFF, off);

    prcomplete(testpass, errcount, (char *)0);
    return (rc | rc2 | rc3 | rc4 | rc5 | rc6 | rc7);
}
/******************************************************************************
 *
 * Function: diag_gephy_led_test
 *
 * Description: GE PHY(Marvell 88E1543) LED test 
 *
 * Inputs      : None
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/
int diag_gephy_led_test (void)
{
    dev_88e1543_object_t gephy_obj;
    dev_88e1543_object_t *gephy_obj_p = &gephy_obj;
    char *tname ="gephy LED Test";
    int rc = FAILED;
    int gephy_num;

    testname(tname);
    prpass(testpass, "%s, ", tname);
    
    /* Create device for MRV1543 GE PHY */
    rc = diag_gephy_dev_create(gephy_obj_p);
    if (rc == FAILED) {
        cterr('f', 0, "%s: Creating object failed", __FUNCTION__);
        return (FAILED);
    }

    for (gephy_num = NUTELLA_88E1543_P0_PHY;
         gephy_num <= NUTELLA_88E1543_P3_PHY; gephy_num++){
        /* Turn on 1543 LED */
        rc = gephy_obj_p->callin_fvt->
             gephy_set_led_on((dev_object_t *)gephy_obj_p, gephy_num);
        if ( rc == FAILED) {
            goto _exit;
        }
        
        msleep(DELAY_FOR_LED_TEST);

        /* Turn off 1543 LED */
        rc = gephy_obj_p->callin_fvt->
             gephy_set_led_off((dev_object_t *)gephy_obj_p, gephy_num);
        if (rc == FAILED) {
            goto _exit;
        }
        
        msleep(DELAY_FOR_LED_TEST);

        /* Let 1543 LED to be default value*/
        rc = gephy_obj_p->callin_fvt->
             gephy_set_led_default((dev_object_t *)gephy_obj_p, gephy_num);
        if (rc == FAILED) {
            goto _exit;
        }
        
        msleep(DELAY_FOR_LED_TEST);
    }

_exit:
    gephy_obj_p->base.dev_object_fvt->dev_destroy((dev_object_t **)&gephy_obj_p);
    prcomplete(testpass, errcount, (char *)0);
    return (rc);
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
    printf("!!!Notice!!!\n");
    printf("Please plug in SFP loopback cable.\n");
    int rc, test_port;

    char *tname ="I350 SFP LED Test"; 
    testname(tname);
    prpass(testpass, "%s, ", tname);

    for (test_port = DNV_I350_PORT1; test_port <= DNV_I350_PORT2; test_port++){
        /*Turn on led by link up SFP*/
        rc = diag_i350_led_on_off(test_port, TRUE);
        if (rc == FAILED){
            printf("Failed to link up SFP Port %d.\n", test_port);
            return(FAILED);
        }
    
        msleep(DELAY_FOR_LED_TEST);
        
        /* Turn off led by link down SFP*/
        rc = diag_i350_led_on_off(test_port, FALSE);
        if (rc == FAILED){
            printf("Failed to link up SFP Port %d.\n", test_port);
            return(FAILED);
        }
        
        msleep(DELAY_FOR_LED_TEST);
    }

    prcomplete(testpass, errcount, (char *)0);
    return (rc);
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
    uint32_t reg_val = 0, default_val = 0;
    enum port80_led_bit port80_bit;

    testname(tname);
    prpass(testpass, "%s, ", tname);
    
    /* Set read/write permission start from Port80 on, only one port*/
    if (ioperm(PORT80_ADDR, ACCESS_PORT_NUM, PORT80_ACCESS_ON) < 0) {
        printf("Failed to set access of Port80 on.");
        return (FAILED);
    }

    default_val = inb(PORT80_ADDR);

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
    
    outb(default_val, PORT80_ADDR);
    
    /* Set read/write permission start from Port80 off, only one port*/
    if (ioperm(PORT80_ADDR, ACCESS_PORT_NUM, PORT80_ACCESS_OFF) < 0) {
        printf("Failed to set access of Port80 off");
        return (FAILED);
    }

    prcomplete(testpass, errcount, (char *)0);
    return (PASSED);
}

/*-------------------------------------------------
$Log: diag_led_test.c,v $
Revision 1.7  2020/02/04 08:49:42  alicehua
CSCvs68364: Add and modify codes for FPGA Phase2.

Revision 1.6  2019/10/16 23:50:47  alicehua
CSCvr68092: Add LED utility (turn on/off all LED).

Revision 1.5  2019/07/12 09:13:42  alicehua
Modified codes based on code PRRQs.

Revision 1.4  2019/07/11 12:31:28  alicehua
Collapse Nutella codes into main trunk.

$Endlog$
*/
