 /* $Id: diag_gephy_test.c,v 1.4 2019/01/24 09:36:07 harrchan Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/viper/diag_gephy_test.c,v $
 *------------------------------------------------------------------
 *
 * diag_gephy_test.c - This file is for Marvell 1514 gephy test 
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
#include "platform_fru.h"
#include "platform_cookie.h"
#include "plat_defs.h"
#include "dev_88e151x.h"
#include "diag_eth_pkt_txrx.h"
#include "diag_gephy_test.h"
#include "diag_gephy_lib.h"
#include "diag_gephy_util.h"
#include "dnv_gpio_lib.h"
#include "dnv_eth_lib.h"
#include "diag_nc_lib.h"
#include "diag_common.h"
#include "diag_fpga.h"



/*
 * Global variables
 */


/* Local functions */
int diag_copper_ext_lpbk_test(void);


/* Local build menu functions */
int build_gephy0_test_menu(boolean);
int build_gephy1_test_menu(boolean);

static int build_gephy_util_menu(boolean);
static int diag_phy_reg_test(int);
static int diag_phy_int_test(int);
static int diag_gephy_lpbk_test(int);
static int diag_gephy_int_lpbk_test(void);
static int diag_gephy_ext_lpbk_test(void);

/*
 * Sub Menu used for "GE PHY test -> GE PHY submenu test"
 */

submenu_xtable_t gephy_submenu_table[] = {
    {"GE PHY Utility",
     (PFT) build_gephy_util_menu, 0, 0,
     (type_t(*)())0, 0, (type_t(*)()) build_gephy_util_menu, 0},

    {"PHY Register Test",
     (PFT) diag_phy_reg_test, FALSE,
     MF_CONTINUOUS | MF_DOGRP | MF_DOALL,
     (type_t(*)())0, 0, (PFT) 0, 0},
    
    {"PHY Interrupt Test",
     (PFT) diag_phy_int_test, FALSE,
     MF_CONTINUOUS | MF_DOGRP | MF_DOALL,
     (type_t(*)())0, 0, (PFT) 0, 0},

    {"PHY Internal Loopback Test",
     (PFT) diag_gephy_int_lpbk_test, FALSE,
     MF_CONTINUOUS | MF_DOGRP | MF_DOALL,
     (type_t(*)())0, 0, (PFT) 0, 0},

    {"PHY External Loopback Test",
     (PFT) diag_gephy_ext_lpbk_test, FALSE,
     MF_CONTINUOUS | MF_DOGRP | MF_DOALL,
     (type_t(*)())0, 0, (PFT) 0, 0},
};

#define GEPHY_SUBMENU_TABLE_SIZE (sizeof(gephy_submenu_table) / \
                                  sizeof(submenu_xtable_t))

static mitem_t gephy_primary_items[GEPHY_SUBMENU_TABLE_SIZE + MAX_BASE_ITEMS];
static mitem_t gephy_secondary_items[GEPHY_SUBMENU_TABLE_SIZE + MAX_BASE_ITEMS];

menuinfo_t gephy_subtest_menu = {
    "%s Subtest Menu",
    0,                          /* mtparam added by init_empty_menu */
    (PFT) show_endnote,         /* notes missing WICs in combos */
    0,                          /* use generic prompt */
    0,                          /* size (bumped by add_menu_item() */
    gephy_primary_items,
};

menuinfo_t *gephy_submenup = &gephy_subtest_menu;


/*
 * Sub Menu used for "GE PHY test -> GE PHY submenu test -> GE PHY utility submenu"
 */

submenu_xtable_t gephy_util_submenu_table[] = {
    {"GE PHY Register Read Utility", (PFT) diag_gephy_read_reg_util, FALSE, 0,
     (type_t(*)())0, 0, (PFT) 0, 0},
    {"GE PHY Register Write Utility", (PFT) diag_gephy_alter_reg_util, FALSE, 0,
     (type_t(*)())0, 0, (PFT) 0, 0},
    {"GE PHY Register Display Utility", (PFT) diag_gephy_dump_reg_util, FALSE, 0,
     (type_t(*)())0, 0, (PFT) 0, 0},
    {"Send Packet to GE PHY",
     (PFT) diag_gephy_send_pkt_util, FALSE,
     MF_CONTINUOUS | MF_DOGRP | MF_DOALL,
     (type_t(*)())0, 0, (PFT) 0, 0},
    {"GE PHY Test Mode Utility", (PFT) diag_gephy_testmode_util, FALSE, 0,
     (type_t(*)())0, 0, (PFT) 0, 0},
    {"GE PHY EEE Utility", (PFT) diag_gephy_eee_util, FALSE, 0,
     (type_t(*)())0, 0, (PFT) 0, 0},
};

#define GEPHY_UTIL_SUBMENU_TABLE_SIZE (sizeof(gephy_util_submenu_table) / \
                                       sizeof(submenu_xtable_t))

static mitem_t gephy_util_primary_items[GEPHY_UTIL_SUBMENU_TABLE_SIZE +
                                        MAX_BASE_ITEMS];
static mitem_t gephy_util_secondary_items[GEPHY_UTIL_SUBMENU_TABLE_SIZE +
                                          MAX_BASE_ITEMS];

menuinfo_t gephy_util_subtest_menu = {
    "%s Subtest Menu",
    0,                          /* mtparam added by init_empty_menu */
    (PFT) show_endnote,         /* notes missing WICs in combos */
    0,                          /* use generic prompt */
    0,                          /* size (bumped by add_menu_item() */
    gephy_util_primary_items,
};

menuinfo_t *gephy_util_submenup = &gephy_util_subtest_menu;

int diag_ge_phy_no;

/*******************************************************************************
 *
 * Function   : build_gephy0_test_menu
 * Description: GE PHY 0 Test Menu, 88E1514
 * Inputs     : Test/Menu 
 * Outputs    : None
 *
 *******************************************************************************
 */
int build_gephy0_test_menu (boolean mb_temp_test_items_executed)
{
    char *menu_title= "GE PHY 0 Test";

    diag_ge_phy_no = VIPER_88E1514_PHY;


    build_primary_submenu(gephy_submenu_table, GEPHY_SUBMENU_TABLE_SIZE,
                          menu_title, &gephy_submenup);
    build_secondary_submenu(gephy_submenu_table, GEPHY_SUBMENU_TABLE_SIZE,
                            gephy_secondary_items);
    if (mb_temp_test_items_executed) {
        menu(&gephy_subtest_menu, gephy_secondary_items, 0);
    } else {
        do_all_menu_items(gephy_submenup);
    }
    return (PASSED);
}


/*******************************************************************************
 *
 * Function   : build_gephy1_test_menu
 * Description: GE PHY 1 Test Menu, 88E1514
 * Inputs     : Test/Menu 
 * Outputs    : None
 *
 *******************************************************************************
 */
int build_gephy1_test_menu (boolean mb_temp_test_items_executed)
{
    char *menu_title= "GE PHY 1 Test";

    diag_ge_phy_no = VIPER_GE1_88E1514_PHY;


    build_primary_submenu(gephy_submenu_table, GEPHY_SUBMENU_TABLE_SIZE,
                          menu_title, &gephy_submenup);
    build_secondary_submenu(gephy_submenu_table, GEPHY_SUBMENU_TABLE_SIZE,
                            gephy_secondary_items);
    if (mb_temp_test_items_executed) {
        menu(&gephy_subtest_menu, gephy_secondary_items, 0);
    } else {
        do_all_menu_items(gephy_submenup);
    }
    return (PASSED);
}


/*******************************************************************************
 *
 * Function   : build_gephy_util_menu
 * Description: GE PHY Utility menu
 * Inputs     : Test/Menu 
 * Outputs    : None
 *
 *******************************************************************************
 */
int build_gephy_util_menu (boolean mb_temp_test_items_executed)
{
    char menu_title[64];

    sprintf(menu_title, "GE PHY %d Utility", diag_ge_phy_no);

    build_primary_submenu(gephy_util_submenu_table, GEPHY_UTIL_SUBMENU_TABLE_SIZE,
                          menu_title, &gephy_util_submenup);
    build_secondary_submenu(gephy_util_submenu_table, GEPHY_UTIL_SUBMENU_TABLE_SIZE,
                            gephy_util_secondary_items);
    menu(&gephy_util_subtest_menu, gephy_util_secondary_items, 0);

    return (PASSED);
}


/******************************************************************************
 *
 * Function: diag_phy_reg_test
 *
 * Description:  PHY Register Test
 *
 * Inputs      : dummy
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/
static int diag_phy_reg_test (int dummy)
{
    dev_88e151x_object_t gephy_obj;
    dev_88e151x_object_t *gephy_obj_p = &gephy_obj;
    int rc;
    uchar mb_get_loc[FRU_SIZE] = {0};
    uchar mb_get_pid[FRU_SIZE] = {0};
    /*
     * 1. Subtests of the test function will reuse all variables
     * 2. All variables will be cleared automatically when
     *    entering and leaving each menu item.
     */
    /* Segment 1: PID | Unique_string : slot_info */
    fru_table_offset = MB;
    /* fru_table_offset should be set, otherwise, it will not */
    /* go to enhanced error message format in cterr() */
    /* set fru_table_offset to get the predefine value */
    /* or change mb_pid & mb_loc below */
    platform_get_pid((char *)mb_get_pid);
    strcpy((char *)mb_get_loc, "MB");
    platform_fru_table[fru_table_offset].pid_string = mb_get_pid;
    platform_fru_table[fru_table_offset].location_string = mb_get_loc;

    /* Segment 2: Test step captured from prpass */
    /* Segment 3: Failure message captured from cterr */

    /* Segment 4: Components used */
    cterr_add_component("Intel Denverton SOC C3558", "SMI", "Marvell 88E1514 GE PHY");

    /* Segment 5: register and memory dump */

    /* Segment 6: Platform Environment initialized here*/

    /* Segment 7: Top 3 Debugging Steps */
    cterr_add_debug("Check the SMI bus status to see if it is normal among each component",
                    "If the status is OK, contact vendor to verify if SMI driver is workable");
    testname("GE PHY Register");

    rc = diag_gephy_dev_create(diag_ge_phy_no, gephy_obj_p);

    if (rc == FAILED) {
        cterr('f', 0, "%s: Creating object failed", __func__);
        return (FAILED);
    }

    prpass(testpass, "Register Test");

    rc = gephy_obj_p->callin_fvt->register_test((dev_object_t *)gephy_obj_p);

    gephy_obj_p->base.dev_object_fvt->dev_destroy((dev_object_t **)&gephy_obj_p);

    prcomplete(testpass, errcount, (char *)0);

    return (rc);
}


/******************************************************************************
 *
 * Function: diag_phy_int_test
 *
 * Description:  PHY Interrupt Test
 *
 * Inputs      : dummy
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/
static int diag_phy_int_test (int dummy)
{
    dev_88e151x_object_t gephy_obj;
    dev_88e151x_object_t *gephy_obj_p = &gephy_obj;
    int rc, gpio_pin, ix;
    uint int_val;
    uint data;
    uchar mb_get_loc[FRU_SIZE] = {0};
    uchar mb_get_pid[FRU_SIZE] = {0};
    /*
     * 1. Subtests of the test function will reuse all variables
     * 2. All variables will be cleared automatically when
     *    entering and leaving each menu item.
     */
    /* Segment 1: PID | Unique_string : slot_info */
    fru_table_offset = MB;
    /* fru_table_offset should be set, otherwise, it will not */
    /* go to enhanced error message format in cterr() */
    /* set fru_table_offset to get the predefine value */
    /* or change mb_pid & mb_loc below */
    platform_get_pid((char *)mb_get_pid);
    strcpy((char *)mb_get_loc, "MB");
    platform_fru_table[fru_table_offset].pid_string = mb_get_pid;
    platform_fru_table[fru_table_offset].location_string = mb_get_loc;

    /* Segment 2: Test step captured from prpass */
    /* Segment 3: Failure message captured from cterr */

    /* Segment 4: Components used */
    cterr_add_component("Intel Denverton SOC C3558", "GPIO","SMI",
                        "Marvell 88E1514 GE PHY");

    /* Segment 5: register and memory dump */

    /* Segment 6: Platform Environment initialized here*/

    /* Segment 7: Top 3 Debugging Steps */


    cterr_add_debug("Run PHY register test for check SMI bus is ok or not",
                    "Check whether the interface between Intel Denverton "
                    "GPIO and PHY is damaged or the soldering issue");
    testname("GE PHY Interrupt");

    rc = diag_gephy_dev_create(diag_ge_phy_no, gephy_obj_p);

    if (rc == FAILED) {
        cterr('f', 0, "%s: Creating object failed", __func__);
        return (FAILED);
    }

    /* Check the CPU interrupt first */
    if (diag_ge_phy_no == VIPER_GE1_88E1514_PHY) {
        gpio_pin = DNV_GPIO_8;
    } else {
        gpio_pin = DNV_GPIO_9;
    }

    prpass(testpass, "Check CPU Interrupt (Before the test)");

    /* Need to configure PADCFG RXDIS 0 and TXDIS 1 to enable GPIO RX */
    if (dnv_gpio_read(gpio_pin, &data) == FAILED) {
        printf("%s: Read GPIO (%d) Fails\n", __func__, gpio_pin);
        return (FAILED);
    }
    data &=(uint)~PADCFG0_GPIOTXRXDIS_MASK;
    data |= (uint)PADCFG0_GPIOTXRXDIS_VAL;
    dnv_gpio_write (gpio_pin, data);
	
    rc = dnv_gpio_read_rx_val(gpio_pin, &int_val);

    if (rc != PASSED) {
        cterr('f', 0, "Read GPIO Value Fails");
        goto __exit;
    }

    if (int_val == GPIO_LOW) {
        cterr('f', 0, "PHY Interrupt is already asserted");
        rc = FAILED;
        goto __exit;
    }

    /* Enable and Force Interrupt */
    prpass(testpass, "Enable and Force Interrupt");

    rc = gephy_obj_p->callin_fvt->enable_force_interrupt((dev_object_t *)gephy_obj_p);

    if (rc != PASSED) {
        cterr('f', 0, "Enable and Force Interrupt Fails");
        goto __exit;
    }

    /* Check if CPU senses the interrupt coming from the PHY */
    prpass(testpass, "Check CPU Interrupt (After the test)");
    for (ix = 0; ix < GEPHY_INT_TIMEOUT; ix++) {
        rc = dnv_gpio_read_rx_val(gpio_pin, &int_val);
        if (rc != PASSED) {
            cterr('f', 0, "Read GPIO Value Fails");
        }

        if (int_val == GPIO_LOW) {
            break;
        }
        msleep (SLEEP_100);
    }

    if (ix == GEPHY_INT_TIMEOUT) {
        cterr('f', 0, "Interrupt is not detected");
        rc = FAILED;
    }


    /* Disable Interrupt */
    prpass(testpass, "Disable Interrupt");
    if (gephy_obj_p->callin_fvt->disable_interrupt((dev_object_t *)gephy_obj_p) 
        != PASSED) {
        cterr('f', 0, "Disable Interrupt FailS");
        rc = FAILED;
    }

__exit:

    gephy_obj_p->base.dev_object_fvt->dev_destroy((dev_object_t **)&gephy_obj_p);

    prcomplete(testpass, errcount, (char *)0);

    return (rc);
}


/******************************************************************************
 *
 * Function: diag_gephy_int_lpbk_test 
 *
 * Description: PHY Internal Loopback Test
 *
 * Inputs      : None
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/
static int diag_gephy_int_lpbk_test (void)
{
    uchar mb_get_loc[FRU_SIZE] = {0};
    uchar mb_get_pid[FRU_SIZE] = {0};
    /*
     * 1. Subtests of the test function will reuse all variables
     * 2. All variables will be cleared automatically when
     *    entering and leaving each menu item.
     */
    /* Segment 1: PID | Unique_string : slot_info */
    fru_table_offset = MB;
    /* fru_table_offset should be set, otherwise, it will not */
    /* go to enhanced error message format in cterr() */
    /* set fru_table_offset to get the predefine value */
    /* or change mb_pid & mb_loc below */
    platform_get_pid((char *)mb_get_pid);
    strcpy((char *)mb_get_loc, "MB");
    platform_fru_table[fru_table_offset].pid_string = mb_get_pid;
    platform_fru_table[fru_table_offset].location_string = mb_get_loc;

    /* Segment 2: Test step captured from prpass */
    /* Segment 3: Failure message captured from cterr */

    /* Segment 4: Components used */
    cterr_add_component("Intel Denverton SOC C3558", "SGMII","SMI",
                        "Marvell 88E1514 GE PHY");

    /* Segment 5: register and memory dump */

    /* Segment 6: Platform Environment initialized here*/

    /* Segment 7: Top 3 Debugging Steps */


    cterr_add_debug("Check PHY register test is ok",
                    "If step a. is OK, check the packet status,"
                    " received or not? Received the unexpected packet?",
                    "If step b. is OK, consult with CPU vendor and PHY vendor");

    return (diag_gephy_lpbk_test(DEV_88E151X_INT_LPBK));
}

/******************************************************************************
 *
 * Function: diag_gephy_ext_lpbk_test
 *
 * Description: Copper External Loopback Test
 *
 * Inputs      : None
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/
static int diag_gephy_ext_lpbk_test (void)
{
    uchar mb_get_loc[FRU_SIZE] = {0};
    uchar mb_get_pid[FRU_SIZE] = {0};
    /*
     * 1. Subtests of the test function will reuse all variables
     * 2. All variables will be cleared automatically when
     *    entering and leaving each menu item.
     */
    /* Segment 1: PID | Unique_string : slot_info */
    fru_table_offset = MB;
    /* fru_table_offset should be set, otherwise, it will not */
    /* go to enhanced error message format in cterr() */
    /* set fru_table_offset to get the predefine value */
    /* or change mb_pid & mb_loc below */
    platform_get_pid((char *)mb_get_pid);
    strcpy((char *)mb_get_loc, "MB");
    platform_fru_table[fru_table_offset].pid_string = mb_get_pid;
    platform_fru_table[fru_table_offset].location_string = mb_get_loc;

    /* Segment 2: Test step captured from prpass */
    /* Segment 3: Failure message captured from cterr */

    /* Segment 4: Components used */
    cterr_add_component("Intel Denverton SOC C3558", "SGMII","SMI",
                        "Marvell 88E1514 GE PHY");

    /* Segment 5: register and memory dump */

    /* Segment 6: Platform Environment initialized here*/

    /* Segment 7: Top 3 Debugging Steps */
    cterr_add_debug("Change another external loopback cable.",
                    "If step a. is still failed, try the internal" 
                    "loopback to check if internal loopback is OK.",
                    "If step b. is OK, we can know PHY has problems."
                    " If step b. is failed, please try step d.",
                    "Observe MDIO register status to check if PHY "
                    "configuration is normal.",
                    "If step d. is OK, we can assume the interface"
                    " between Host SoC and PHY has problems");
    if (check_ext_lpbk_flag()) {
        return (diag_gephy_lpbk_test(DEV_88E151X_EXT_LPBK));
    } else {
        printf("External Loopback flag is off. Skip the test\n");
        return (PASSED);
    }
}

/******************************************************************************
 *
 * Function: diag_gephy_lpbk_test
 *
 * Description: Function to perform internal/external loopback test
 *
 * Inputs      : lpbk_mode - Internal/External
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/
static int diag_gephy_lpbk_test (int lpbk_mode)
{
    dev_88e151x_object_t gephy_obj;
    dev_88e151x_object_t *gephy_obj_p = &gephy_obj;
    int rc = FAILED, retry_count = 0;
    int ix, jx, link_status;
    char *speed_str[] = {"10Mbps", "100Mbps", "1000Mbps"};
    char iface_name[16];

    if (dnv_eth_get_iface_name(diag_ge_phy_no, iface_name) == FAILED) {
        printf("%s: Get Iface name failed (%d)\n", __func__, diag_ge_phy_no);
        return (FAILED);
    }

    if (lpbk_mode == DEV_88E151X_INT_LPBK) {
        testname("PHY Loopback");
    } else {
        testname("External Loopback");
    }

    rc = diag_gephy_dev_create(diag_ge_phy_no, gephy_obj_p);

    if (rc == FAILED) {
        cterr('f', 0, "%s: Creating object failed", __func__);
        return (FAILED);
    }

    for (ix = DEV_88E151X_SPD_10; ix <= DEV_88E151X_SPD_1000; ix++) {
        retry_count = 0;
retry:
        prpass(testpass, "Speed %s", speed_str[ix]);
        if (diag_ge_phy_no == VIPER_88E1514_PHY) {
            system(ETH_PHY_1514_GE0_DOWN);
            system(ETH_PHY_1514_GE0_UP);
        } else {
            system(ETH_PHY_1514_GE1_DOWN);
            system(ETH_PHY_1514_GE1_UP);
        }      

        /* Set speed */
        rc = gephy_obj_p->callin_fvt->set_spd((dev_object_t *)gephy_obj_p, ix);
        if (rc == FAILED) {
            cterr('f', 0, "Set Speed to %s Failed", speed_str[ix]);
            goto __exit;
        }
        rc = gephy_obj_p->callin_fvt->set_lpbk((dev_object_t *)gephy_obj_p, 
                                                lpbk_mode, ix);

        if (rc != PASSED) {
            cterr('f', 0, "Enable Loopback for %s fails", speed_str[ix]);
            goto __exit;
        }

        prpass(testpass, "Running Loopback ");

        /* Make sure that link status is detected before performing loopback */
        for (jx = 0; jx < LPBK_LINK_UP_TOUT; jx++) {
            link_status = dnv_eth_link_is_up(diag_ge_phy_no);

            if (link_status == TRUE) {
                break;
            }
            msleep(10);
        }

        if (jx == LPBK_LINK_UP_TOUT || link_status == FALSE) {
            if (retry_count == VIPER_LPBK_RETRY) {
                rc = FAILED;
                cterr('f', 0, "Link (%s) is not detected", speed_str[ix]);
                goto __exit;
            } else {
                retry_count++;
                printf("\nWarning this is link up  retry %d time\n", retry_count);
                goto retry;
            }
        }

        rc = eth_pkt_txrx(iface_name, LPBKTEST_PKT_CNT, FALSE);
        if ((retry_count == VIPER_LPBK_RETRY) && (rc == FAILED)) {
            cterr('f', 0, "Loopback failed (%s)", speed_str[ix]);
            goto __exit;
        }        
        /* (CSCvn43011)Base on Intel reply there isn't have solution so we add retry 
         * here to workaround Denverton loopback issue*/
        if (rc == FAILED) {        
            retry_count++;
            if (lpbk_mode == DEV_88E151X_INT_LPBK) {
                printf("\nWarning this is internal lpbk retry %d time\n", retry_count);
            } else {
                printf("\nWarning this is external lpbk retry %d time\n", retry_count);
            }
            goto retry;
        }
    }
__exit:
    /* Set speed to 1G */
    rc = gephy_obj_p->callin_fvt->set_spd((dev_object_t *)gephy_obj_p, 
                                           DEV_88E151X_SPD_1000);

    if (rc != PASSED) {
        cterr('f', 0, "Set speed to 1G fails");
    } 

    /* Disable Loopback */
    rc = gephy_obj_p->callin_fvt->set_lpbk((dev_object_t *)gephy_obj_p, 
                                            DEV_88E151X_DIS_LPBK,
                                            DEV_88E151X_SPD_1000);

    if (rc != PASSED) {
        cterr('f', 0, "Disable loopback fails");
    } 


    gephy_obj_p->base.dev_object_fvt->dev_destroy((dev_object_t **)&gephy_obj_p);

    prcomplete(testpass, errcount, (char *)0);


    return (rc);
}

/*-------------------------------------------------
 * $Log: diag_gephy_test.c,v $
 * Revision 1.4  2019/01/24 09:36:07  harrchan
 * Remove the keyword fail and error when retry on ethernet switch loopback test(2)(CSCvn94470)
 *
 * Revision 1.3  2018/12/10 09:57:05  harrchan
 * Add workaround to PHY and Switch loopback test (CSCvn43011)
 *
 * Revision 1.2  2018/08/06 02:31:50  harrchan
 * Merge viper E2E to the main trunk (CSCvk28469)
 *
 * Revision 1.1.2.10  2018/07/06 02:54:08  harrchan
 * Add enhance error message
 *
 * Revision 1.1.2.9  2018/07/03 05:38:55  harrchan
 * Follow the coding rule to clean up code
 *
 * Revision 1.1.2.8  2018/06/27 06:27:49  harrchan
 * Follow the coding rule to clean up code
 *
 * Revision 1.1.2.7  2018/05/09 07:11:26  olin2
 * 1. Move GE and DSL init to the beginning. 2. Add has GE1. 3. Show cookie info
 *
 * Revision 1.1.2.6  2018/04/26 08:14:15  lucywang
 * Added utility to set 88E1514 EEE
 *
 * Revision 1.1.2.5  2018/04/10 06:33:16  lucywang
 * Modified GPIO setting before interrupt test for ViperJ
 *
 * Revision 1.1.2.4  2018/03/28 06:50:50  lucywang
 * Changed interrupt pin for GE1
 *
 * Revision 1.1.2.3  2018/03/16 01:59:55  olin2
 * Support GE PHY testmode util
 *
 * Revision 1.1.2.2  2018/03/14 06:59:37  olin2
 * Modify 1514 init sequence
 *
 * Revision 1.1.2.1  2018/02/27 08:06:43  harrchan
 * Initial viper application code base
 *
 *
 * $Endlog$
 *-------------------------------------------------
 */
