 /* $Id: diag_gephy_test.c,v 1.2 2019/10/17 02:16:21 kehuang2 Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/tabei-l/diag_gephy_test.c,v $
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
#include "diag_common.h"
#include "diag_fpga.h"
#include "nvmonvars.h"
#include "queryflags.h"

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
static int check_ext_lpbk_flag(void);
static int ge_phy_no_retry = FALSE;

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
    {"GE PHY Init Utility", (PFT) diag_gephy_init_util, FALSE, 0,
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
 * Outputs    : PASSED
 *
 *******************************************************************************
 */
int build_gephy0_test_menu (boolean mb_temp_test_items_executed)
{
    char *menu_title= "GE PHY 0 Test";

    diag_ge_phy_no = TABEI_GE0_88E1514_PHY;


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
 * Outputs    : PASSED
 *
 *******************************************************************************
 */
int build_gephy1_test_menu (boolean mb_temp_test_items_executed)
{
    char *menu_title= "GE PHY 1 Test";

    diag_ge_phy_no = TABEI_GE1_88E1514_PHY;


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
 * Outputs    : PASSED
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
    int rc, ix;
    ulong gpio_pin;
    uint data;
    testname("GE PHY Interrupt");

    rc = diag_gephy_dev_create(diag_ge_phy_no, gephy_obj_p);

    if (rc == FAILED) {
        cterr('f', 0, "%s: Creating object failed", __func__);
        return (FAILED);
    }

    /* Check the CPU interrupt first */
    if (diag_ge_phy_no == TABEI_GE1_88E1514_PHY) {
        gpio_pin = LAN0_PORT1_SPD0;
    } else {
        gpio_pin = LAN0_PORT0_SPD0;
    }

    prpass(testpass, "Check CPU Interrupt (Before the test)");

    /* Clear Interrupt pin before Interrupt Test */
    if (gephy_obj_p->callin_fvt->disable_interrupt((dev_object_t *)gephy_obj_p) 
        != PASSED) {
        cterr('f', 0, "Disable Interrupt Fails");
        rc = FAILED;
    }

    /* Check Interrupt pin is asserted or not */
    rc = dnv_ethernet_sdp_pin_read(gpio_pin, &data);
    if (rc != PASSED) {
        cterr('f', 0, "Read GPIO Value Fails");
        goto __exit;
    }
    if ((data & EXTENDED_SPD_CTRL_SPD0_MASK) == GPIO_HIGH) {
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
        rc = dnv_ethernet_sdp_pin_read(gpio_pin, &data);
        if (rc != PASSED) {
            cterr('f', 0, "Read GPIO Value Fails");
        }

        if ((data & EXTENDED_SPD_CTRL_SPD0_MASK) == GPIO_HIGH) {
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
        cterr('f', 0, "Disable Interrupt Fails");
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
    if (check_ext_lpbk_flag()) {
        return (diag_gephy_lpbk_test(DEV_88E151X_EXT_LPBK));
    } else {
        printf("External Loopback flag is off. Skip the test\n");
        return (PASSED);
    }
}
/*******************************************************************************
 *
 * Function   : check_ext_lpbk_flag
 * Description: Function to check if Ext. Loopback Flag is ON or not.
 * Inputs     : None
 * Outputs    : TRUE(ON) / FALSE(OFF)
 *
 *******************************************************************************
 */
static int check_ext_lpbk_flag (void)
{
    /* according to menu_show_dflags(), D_EXT_LPBK is inverse flag */ 
    if ((NVRAM)->diagflag & D_EXT_LOOPBACK) {
        return (FALSE);
    } else { 
        return (TRUE);
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
    int rc = FAILED, retry_count, nolink_retry_count = 0;
    int ix, jx, link_status;
    char *speed_str[] = {"10Mbps", "100Mbps", "1000Mbps"};
    char iface_name[16];

    if (dnv_eth_get_iface_name(diag_ge_phy_no, iface_name) == FAILED) {
        printf("%s: Get Iface name failed (%d)\n", __func__, diag_ge_phy_no);
        return (FAILED);
    }

    if (lpbk_mode == DEV_88E151X_INT_LPBK) {
        testname("PHY Internal Loopback");
    } else {
        testname("PHY External Loopback");
    }

    if (ge_phy_no_retry == TRUE) {
       if ((NVRAM)->diagflag & D_VERBOSE) {
            printf("No loopback workaround (No Retry)\n");
       }
    } else {
        printf("Loopback workaround turn on\n");
    }

    rc = diag_gephy_dev_create(diag_ge_phy_no, gephy_obj_p);

    if (rc == FAILED) {
        cterr('f', 0, "%s: Creating object failed", __func__);
        return (FAILED);
    }

    for (ix = DEV_88E151X_SPD_10; ix <= DEV_88E151X_SPD_1000; ix++) {
        if (ge_phy_no_retry == TRUE) {
            retry_count = TABEI_LPBK_RETRY;
        } else {
            retry_count = 0;
        }
retry:
        prpass(testpass, "Speed %s", speed_str[ix]);

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
            msleep(SLEEP_10);
        }

        if ((jx == LPBK_LINK_UP_TOUT || link_status == FALSE) 
             && (nolink_retry_count == TABEI_LPBK_RETRY)) {
            rc = FAILED;
            cterr('f', 0, "Link (%s) is not detected", speed_str[ix]);
            goto __exit;
        }

        /* (CSCvr12865)Base on Intel reply there isn't have solution so we add retry 
         * here to workaround Denverton loopback issue 
         */
        if (jx == LPBK_LINK_UP_TOUT) {
            nolink_retry_count++;
            if (lpbk_mode == DEV_88E151X_INT_LPBK) {
                printf("\nWarning this is internal no link retry %d time\n", retry_count);
            } else {
                printf("\nWarning this is external no link retry %d time\n", retry_count);
            }
            if (diag_ge_phy_no == TABEI_GE0_88E1514_PHY) {
                system(ETH_PHY_1514_GE0_DOWN);
                system(ETH_PHY_1514_GE0_UP);
            } else {
                system(ETH_PHY_1514_GE1_DOWN);
                system(ETH_PHY_1514_GE1_UP);
            }
           /* CSCvq58855: add 2s delay for driver finish accessing PHY, or will have 
            * race condition with Diag configure PHY                        *                           
            */
            msleep(WAITING_DRIVER_FINISH);
            goto retry;
        }

        rc = eth_pkt_txrx(iface_name, LPBKTEST_PKT_CNT, FALSE);

        if ((retry_count == TABEI_LPBK_RETRY) && (rc == FAILED)) {
            cterr('f', 0, "Loopback failed (%s)", speed_str[ix]);
            goto __exit;
        }

        /* (CSCvn43011)Base on Intel reply there isn't have solution so we add retry 
         * here to workaround Denverton loopback issue 
         */
        if (rc == FAILED) {
            retry_count++;
            if (lpbk_mode == DEV_88E151X_INT_LPBK) {
                printf("\nWarning this is internal lpbk retry %d time\n", retry_count);
            } else {
                printf("\nWarning this is external lpbk retry %d time\n", retry_count);
            }
            if (diag_ge_phy_no == TABEI_GE0_88E1514_PHY) {
                system(ETH_PHY_1514_GE0_DOWN);
                system(ETH_PHY_1514_GE0_UP);
            } else {
                system(ETH_PHY_1514_GE1_DOWN);
                system(ETH_PHY_1514_GE1_UP);
            }
           /* CSCvq58855: add 2s delay for driver finish accessing PHY, or will have 
            * race condition with Diag configure PHY                        *                           
            */
            msleep(WAITING_DRIVER_FINISH);
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
 * Revision 1.2  2019/10/17 02:16:21  kehuang2
 * Collapse Tabei-L into main trunk
 *
 * Revision 1.1.4.18  2019/09/11 09:26:25  kehuang2
 * CSCvr12865: add workaround for no link issue
 *
 * Revision 1.1.4.17  2019/08/29 03:49:27  kehuang2
 * Clean up code by the comment of code review
 *
 * Revision 1.1.4.16  2019/08/26 07:55:00  kehuang2
 * Clean up code by the comment of code review
 *
 * Revision 1.1.4.15  2019/08/21 07:14:11  kehuang2
 * Add workaround for viper issue
 *
 * Revision 1.1.4.14  2019/07/26 03:41:56  kehuang2
 * Add setting interrupt pin configuration into initial sequence
 *
 * Revision 1.1.4.13  2019/06/28 04:01:30  kehuang2
 * Change GEPHY Inperttupt pin from GPIO to Ethernet Controller SDP
 *
 * Revision 1.1.4.12  2019/06/04 07:49:15  olin2
 * Add verbose flag for debug info
 *
 * Revision 1.1.4.11  2019/05/29 03:16:17  kehuang2
 *
 * 1.Merge image according to official board type.
 * 2.Reform the structure of diag menu
 *
 * Revision 1.1.4.10  2019/05/24 09:56:11  kehuang2
 *
 * 1.Update Temp Interrupt test
 * 2.Clean up code
 *
 * Revision 1.1.4.9  2019/05/22 09:01:57  kehuang2
 * Clean up code
 *
 * Revision 1.1.4.8  2019/03/07 07:59:38  olin2
 * Set default no loopback workaround
 *
 * Revision 1.1.4.7  2019/01/25 02:31:53  olin2
 * Correct description
 *
 * Revision 1.1.4.6  2019/01/02 07:07:24  olin2
 * Add debug util
 *
 * Revision 1.1.4.5  2018/12/25 06:38:40  olin2
 * Support initial GE PHY util
 *
 * Revision 1.1.4.4  2018/12/24 08:04:43  olin2
 * Support GE PHY workaround for DNV ethernet controller bug
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
