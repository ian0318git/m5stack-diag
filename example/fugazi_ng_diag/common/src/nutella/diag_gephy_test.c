/* $Id: diag_gephy_test.c,v 1.9 2020/09/30 09:46:09 alicehua Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/nutella/diag_gephy_test.c,v $
 *------------------------------------------------------------------
 *
 * diag_gephy_test.c - This file is for Marvell 1543 gephy test 
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
#include "diag_eth_pkt_txrx.h"
#include "diag_gephy_test.h"
#include "diag_gephy_lib.h"
#include "diag_gephy_util.h"
#include "dnv_gpio_lib.h"
#include "dnv_eth_lib.h"
#include "diag_common.h"
#include "diag_fpga.h"



/*
 * Global variables
 */
static dev_88e1543_object_t    dev_mrvl_88e1543_phy0;
int mrvl88e1543_dev_init;


/* Local functions */
int diag_copper_ext_lpbk_test(void);


/* Local build menu functions */
int build_gephy_test_menu(boolean);
static int build_gephy_util_menu(boolean);
static int diag_phy_reg_test(int);
static int diag_phy_int_test(int);
static int diag_gephy_lpbk_test(int);
static int diag_gephy_lpbk_port_test(int);
dev_object_t *mrvl88e1543_get_object(void);

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
     (PFT) diag_gephy_lpbk_test, SGMII_PHY_LPBK_INTERNAL,
     MF_CONTINUOUS | MF_DOGRP | MF_DOALL,
     (type_t(*)())0, 0, (PFT) 0, 0},

    {"PHY External Loopback Test",
     (PFT) diag_gephy_lpbk_test, SGMII_PHY_LPBK_EXTERNAL,
     MF_CONTINUOUS | MF_DOGRP | MF_DOALL,
     (type_t(*)())0, 0, (PFT) 0, 0},

    {"PHY Internal Loopback Port 0 Test",
     (PFT) diag_gephy_lpbk_port_test, SGMII_PHY_LPBK_INTERNAL_PORT0,
     MF_CONTINUOUS | MF_DOGRP,
     (type_t(*)())0, 0, (PFT) 0, 0},

    {"PHY Internal Loopback Port 1 Test",
     (PFT) diag_gephy_lpbk_port_test, SGMII_PHY_LPBK_INTERNAL_PORT1,
     MF_CONTINUOUS | MF_DOGRP,
     (type_t(*)())0, 0, (PFT) 0, 0},

    {"PHY Internal Loopback Port 2 Test",
     (PFT) diag_gephy_lpbk_port_test, SGMII_PHY_LPBK_INTERNAL_PORT2,
     MF_CONTINUOUS | MF_DOGRP,
     (type_t(*)())0, 0, (PFT) 0, 0},

    {"PHY Internal Loopback Port 3 Test",
     (PFT) diag_gephy_lpbk_port_test, SGMII_PHY_LPBK_INTERNAL_PORT3,
     MF_CONTINUOUS | MF_DOGRP,
     (type_t(*)())0, 0, (PFT) 0, 0},

    {"PHY External Loopback Port 0 Test",
     (PFT) diag_gephy_lpbk_port_test, SGMII_PHY_LPBK_EXTERNAL_PORT0,
     MF_CONTINUOUS | MF_DOGRP,
     (type_t(*)())0, 0, (PFT) 0, 0},

    {"PHY External Loopback Port 1 Test",
     (PFT) diag_gephy_lpbk_port_test, SGMII_PHY_LPBK_EXTERNAL_PORT1,
     MF_CONTINUOUS | MF_DOGRP,
     (type_t(*)())0, 0, (PFT) 0, 0},

    {"PHY External Loopback Port 2 Test",
     (PFT) diag_gephy_lpbk_port_test, SGMII_PHY_LPBK_EXTERNAL_PORT2,
     MF_CONTINUOUS | MF_DOGRP,
     (type_t(*)())0, 0, (PFT) 0, 0},

    {"PHY External Loopback Port 3 Test",
     (PFT) diag_gephy_lpbk_port_test, SGMII_PHY_LPBK_EXTERNAL_PORT3,
     MF_CONTINUOUS | MF_DOGRP,
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
    {"Send Packet to GE PHY",
     (PFT) diag_gephy_send_pkt_util, FALSE,
     MF_CONTINUOUS | MF_DOGRP | MF_DOALL,
     (type_t(*)())0, 0, (PFT) 0, 0},
    {"GE PHY Test Mode Utility", (PFT) diag_gephy_testmode_util, FALSE, 0,
     (type_t(*)())0, 0, (PFT) 0, 0},
    {"GE PHY Read Page Utility", (PFT) diag_gephy_read_page_reg, FALSE, 0,
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
 * Function   : build_gephy_test_menu
 * Description: GE PHY Test Menu, 88E1543
 * Inputs     : Test/Menu 
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
int build_gephy_test_menu (boolean mb_temp_test_items_executed)
{
    char *menu_title= "GE PHY Test";
    int rc = FAILED;
    dev_88e1543_object_t *gephy_obj_p;

    gephy_obj_p = (dev_88e1543_object_t *)mrvl88e1543_get_object();

    if (gephy_obj_p == NULL) {
        cterr('f', 0, "%s: Null pointer", __FUNCTION__);
        return (FAILED);
    }
    
    rc = phy_88e1543_dev_init(gephy_obj_p, 
                              NUTELLA_PHY_START_ADDR,
                              MRV88E1543_PHY_ADDR_INCR);

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
    dev_88e1543_object_t *gephy_obj_p;
    testname("Marvell 88e1543 register test");
    uint port;

    gephy_obj_p = (dev_88e1543_object_t *)mrvl88e1543_get_object();

    if (gephy_obj_p == NULL) {
        cterr('f', 0, "%s: Null pointer", __FUNCTION__);
        return (FAILED);
    }
    
    /* Check if phy port is plugged with download cable,
     * skip test the port which is plugged download cable */
    for (port = NUTELLA_PHY_PORT0; port <= NUTELLA_PHY_PORT3; port++) {
        gephy_obj_p->callin_fvt->gephy_check_if_plugged_with_cable((dev_object_t *)gephy_obj_p, port);
    }

    if (MVL_88E1543_REG_TEST(gephy_obj_p) != PASSED) {
        cterr('f', 0, "MRV88E1543 Register test Failed");
        return (FAILED); 
    }
    
    prcomplete(testpass, errcount, (char *)0);

    return (PASSED);
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
    dev_88e1543_object_t *gephy_obj_p;
    int rc = FAILED, gpio_pin, ix, jx;
    uint phy_addr, port;
    uint all_link_stat = 0;
    uint int_val;
    testname( "Marvell 88e1543 interrupt test");

    gephy_obj_p = (dev_88e1543_object_t *)mrvl88e1543_get_object();

    if (gephy_obj_p == NULL) {
        cterr('f', 0, "%s: Null pointer", __FUNCTION__);
        return (FAILED);
    }

    /* Check if ethernet port is plugged with download cable,
     * use no link up port to generate interrupt. */
    for (port = NUTELLA_PHY_PORT0; port <= NUTELLA_PHY_PORT3; port++) {
        gephy_obj_p->callin_fvt->gephy_check_if_plugged_with_cable((dev_object_t *)gephy_obj_p, port);

        if (all_port_link_status[port] == FALSE) {
            phy_addr = port;
            printf("Use PHY port %d to generate interrupt\n", phy_addr);
            break;
        }
        
        all_link_stat = all_link_stat + all_port_link_status[port];
    }

    if (all_link_stat == MRV88E1543_PORTS) {
        cterr('f', 0, "All PHY ports are plugged with download cable, couldn't do PHY Interrupt Test\n");
        return (FAILED);
    }
    
    /* DNV_GPIO_9 is MRV1543 interrupt pin */
    gpio_pin = DNV_GPIO_9;

    prpass(testpass, "Check CPU Interrupt (Before the test)");

    /* Need to configure PADCFG RXDIS 0 and TXDIS 1 to enable GPIO RX */
    rc = set_dnv_gpio_direction(gpio_pin, GPIO_IN);
    if (rc != PASSED) {
        cterr('f', 0, "Set GPIO direction Fails");
        return (FAILED);
    }

    /* If interrupt is asserted disable it first*/
    for (ix = 0; ix < GEPHY_INT_TIMEOUT; ix++) {
        rc = dnv_gpio_read_rx_val(gpio_pin, &int_val);
        if (rc != PASSED) {
            cterr('f', 0, "Read GPIO Value Fails");
            return (FAILED);
        }

        if (int_val == GPIO_HIGH) {
            break;
        }

        for (jx = 0; jx <= NUTELLA_PHY_MAX_PORT_NUM; jx++) {
            rc = MVL_88E1543_INTR_DISABLE(gephy_obj_p, jx);
            if (rc != PASSED) {
                cterr('f', 0, "Disable Interrupt Fails on %x port", jx);
                return (FAILED);
            }
        }

        msleep (SLEEP_100);
    }

    if (ix == GEPHY_INT_TIMEOUT) {
        cterr('f', 0, "PHY Interrupt is already asserted and can't be cleared");
        return (FAILED);
    }

    /* Force 88E1543 PHY to generate interrupt to host */
    prpass(testpass, "Enable and Force Interrupt");
    if (MVL_88E1543_INTR_GEN(gephy_obj_p, phy_addr) != PASSED) {
        return (FAILED);
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
        return (FAILED);
    }

    /* Disable Interrupt */
    prpass(testpass, "Disable Interrupt");
    for (jx = 0; jx <= NUTELLA_PHY_MAX_PORT_NUM; jx++) {
        rc = MVL_88E1543_INTR_DISABLE(gephy_obj_p, jx);

        if (rc != PASSED) {
            cterr('f', 0, "Disable Interrupt Fails on %x port", jx);
            return (FAILED);
        }
    }

    prcomplete(testpass, errcount, (char *)0);

    return (rc);
}

/******************************************************************************
 *
 * Function: diag_gephy_lpbk_test
 *
 * Description: Copper External/Internal Loopback Test
 *
 * Inputs      : lpbk_mode - external lpbk or internal lpbk
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/
static int diag_gephy_lpbk_test (int lpbk_mode)
{
    int retval = FAILED, port, jx, retry_count, nolink_retry_count;
    int speed, link_status;
    char iface_name[16];
    int spd_idx[3] = {1000, 100, 10};
    dev_88e1543_object_t *gephy_obj_p;
    
    if (lpbk_mode == SGMII_PHY_LPBK_EXTERNAL) {
        testname("Nutella PHY External Loopback");
        prpass(testpass,"Nutella PHY External Loopback,");
    } else {
        testname("Nutella PHY Internal Loopback");
        prpass(testpass,"Nutella PHY Internal Loopback,");
    }

    /*
     * D_EXT_LOOPBACK = 0, enable ext. gephy lpbk
     * D_EXT_LOOPBACK = 1, disable ext. gephy lpbk
     */
    if (check_menu_flag(D_EXT_LOOPBACK) && lpbk_mode == SGMII_PHY_LPBK_EXTERNAL) {
        prpass(testpass, "\n External loopback flag is off, skip"
                         "external loopback test.");
        prcomplete(testpass, errcount, (char *)0);
        return (PASSED);
    }

    gephy_obj_p = (dev_88e1543_object_t *)mrvl88e1543_get_object();

    if (gephy_obj_p == NULL) {
        cterr('f', 0, "%s: Null pointer", __FUNCTION__);
        return (FAILED);
    }

    for (port = NUTELLA_PHY_PORT0; port <= NUTELLA_PHY_PORT3; port++) {
        for (speed = ETH_MODE_GE; speed <= ETH_MODE_FE10; speed ++) {
            retry_count = 0;
            nolink_retry_count = 0;
retry:
            prpass(testpass, "PHY Loopback at port %d "
                   "(speed %dMbps),\n", port, spd_idx[speed]);
            /* Give the interface name */
            if (dnv_eth_get_iface_name(port, iface_name) == FAILED) {
                cterr('f', 0, "%s: Get Iface name failed (%d)\n", __FUNCTION__, port);
                return (FAILED);
            }

            /* SET EXTERNAL/INTERNAL LPBK */
            retval = MVL_88E1543_SET_LPBK(gephy_obj_p, port, speed, 
                                          lpbk_mode);
            if (retval != PASSED) {
                if (lpbk_mode == SGMII_PHY_LPBK_EXTERNAL) {
                    cterr('f', 0, "%s(): phy external loopback setup failed (phy addr %#x)\n",
                           __FUNCTION__, port);
                } else {
                    cterr('f', 0, "%s(): phy internal loopback setup failed (phy addr %#x)\n",
                           __FUNCTION__, port);
                }
                return (retval);
            }

            /* Make sure that link status is detected before performing loopback */
            for (jx = 0; jx < LPBK_LINK_UP_TOUT; jx++) {
                link_status = dnv_eth_link_is_up(port);

                if (link_status == TRUE) {
                    break;
                }
                msleep(CHECK_LINK_UP_DELAY);
            }

            /* Check whether the no link retry times is reach maximum */
            if ((jx == LPBK_LINK_UP_TOUT) && (nolink_retry_count == NUTELLA_LPBK_RETRY)) {
                retval = FAILED;
                cterr('f', 0, "Link is not detected");
                goto __exit;
            }

            /* (CSCvr12865)Base on Intel reply there isn't have solution so we add retry
             * here to workaround Denverton no link issue*/
            if (jx == LPBK_LINK_UP_TOUT) {
                nolink_retry_count++;
                if (lpbk_mode == SGMII_PHY_LPBK_EXTERNAL) {
                    printf("\nWarning this is external no link retry %d time\n", nolink_retry_count);
                } else {
                    printf("\nWarning this is internal no link retry %d time\n", nolink_retry_count);
                }
                
                ifconfig_down_up_eth(iface_name);
                goto retry;
			}

			/* tx and rx packet*/
            retval = eth_pkt_txrx(iface_name, LPBKTEST_PKT_CNT, FALSE);
            
            /* Check whether the retry times is reach maximum */
            if ((retry_count == NUTELLA_LPBK_RETRY) && (retval == FAILED)) {
                cterr('f', 0, "Loopback failed (%dMbps)", spd_idx[speed]);
                goto __exit;
            }
            
            /* (CSCvn43011)Base on Intel reply there isn't have solution so we add retry
             * here to workaround Denverton loopback issue*/
            if (retval == FAILED) {
                retry_count++;
                if (lpbk_mode == SGMII_PHY_LPBK_EXTERNAL) {
                    printf("\nWarning this is external lpbk retry %d time\n", retry_count);
                } else {
                    printf("\nWarning this is internal lpbk retry %d time\n", retry_count);
                }
                
                ifconfig_down_up_eth(iface_name);
                
                goto retry;
            }
__exit:
            /* Check retval */
            if (retval == FAILED) {
                cterr('f', 0, "Fail at tx and rx packet");
                return (retval);
            }
            /* phy clean up */
            retval = MVL_88E1543_CLR_LPBK(gephy_obj_p, port);
            if (retval != PASSED) {
                cterr('f', 0, "%s(): PHY loopback clean up failed", __FUNCTION__);
                return (retval);
            }
        } /* for speed */
    } /* for port  */

    prcomplete(testpass, errcount, (char *)0);

    return (retval);
}

/******************************************************************************
 *
 * Function: diag_gephy_lpbk_port_test
 *
 * Description: Copper External/Internal Loopback Test by single port
 *
 * Inputs      : lpbk_mode_port - external lpbk or internal lpbk
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/
static int diag_gephy_lpbk_port_test (int lpbk_mode_port)
{
    int retval = FAILED, port, jx, retry_count, nolink_retry_count;
    int speed, link_status, lpbk_mode;
    char iface_name[16];
    int spd_idx[3] = {1000, 100, 10};
    dev_88e1543_object_t *gephy_obj_p;
    
    if (lpbk_mode_port >= SGMII_PHY_LPBK_EXTERNAL_PORT0) {
        testname("Nutella PHY External Loopback");
        prpass(testpass,"Nutella PHY External Loopback,");
        port = lpbk_mode_port - NUTELLA_PHY_PORT_NUM;
        lpbk_mode = SGMII_PHY_LPBK_EXTERNAL;
    } else {
        testname("Nutella PHY Internal Loopback");
        prpass(testpass,"Nutella PHY Internal Loopback,");
        port = lpbk_mode_port;
        lpbk_mode = SGMII_PHY_LPBK_INTERNAL;
    }

    /*
     * D_EXT_LOOPBACK = 0, enable ext. gephy lpbk
     * D_EXT_LOOPBACK = 1, disable ext. gephy lpbk
     */
    if (check_menu_flag(D_EXT_LOOPBACK) && lpbk_mode_port >= SGMII_PHY_LPBK_EXTERNAL_PORT0) {
        prpass(testpass, "\n External loopback flag is off, skip"
                         "external loopback test.");
        prcomplete(testpass, errcount, (char *)0);
        return (PASSED);
    }

    gephy_obj_p = (dev_88e1543_object_t *)mrvl88e1543_get_object();

    if (gephy_obj_p == NULL) {
        cterr('f', 0, "%s: Null pointer", __FUNCTION__);
        return (FAILED);
    }

    for (speed = ETH_MODE_GE; speed <= ETH_MODE_FE10; speed ++) {
        retry_count = 0;
        nolink_retry_count = 0;
retry:
        prpass(testpass, "PHY Loopback at port %d "
               "(speed %dMbps),\n", port, spd_idx[speed]);
        /* Give the interface name */
        if (dnv_eth_get_iface_name(port, iface_name) == FAILED) {
            cterr('f', 0, "%s: Get Iface name failed (%d)\n", __FUNCTION__, port);
            return (FAILED);
        }

        /* SET EXTERNAL/INTERNAL LPBK */
        retval = MVL_88E1543_SET_LPBK(gephy_obj_p, port, speed, 
                                      lpbk_mode);
        if (retval != PASSED) {
            if (lpbk_mode_port >= SGMII_PHY_LPBK_EXTERNAL_PORT0) {
                cterr('f', 0, "%s(): phy external loopback setup failed (phy addr %#x)\n",
                       __FUNCTION__, port);
            } else {
                cterr('f', 0, "%s(): phy internal loopback setup failed (phy addr %#x)\n",
                      __FUNCTION__, port);
            }
            return (retval);
        }

        /* Make sure that link status is detected before performing loopback */
        for (jx = 0; jx < LPBK_LINK_UP_TOUT; jx++) {
            link_status = dnv_eth_link_is_up(port);

            if (link_status == TRUE) {
                break;
            }
            msleep(CHECK_LINK_UP_DELAY);
        }

        /* Check whether the no link retry times is reach maximum */
        if ((jx == LPBK_LINK_UP_TOUT) && (nolink_retry_count == NUTELLA_LPBK_RETRY)) {
            retval = FAILED;
            cterr('f', 0, "Link is not detected");
            goto __exit;
        }

        /* (CSCvr12865)Base on Intel reply there isn't have solution so we add retry
         * here to workaround Denverton no link issue*/
        if (jx == LPBK_LINK_UP_TOUT) {
            nolink_retry_count++;
        if (lpbk_mode_port >= SGMII_PHY_LPBK_EXTERNAL_PORT0) {
            printf("\nWarning this is external no link retry %d time\n", nolink_retry_count);
        } else {
            printf("\nWarning this is internal no link retry %d time\n", nolink_retry_count);
        }
                
            ifconfig_down_up_eth(iface_name);
            goto retry;
		}

		/* tx and rx packet*/
        retval = eth_pkt_txrx(iface_name, LPBKTEST_PKT_CNT, FALSE);
            
        /* Check whether the retry times is reach maximum */
        if ((retry_count == NUTELLA_LPBK_RETRY) && (retval == FAILED)) {
            cterr('f', 0, "Loopback failed (%dMbps)", spd_idx[speed]);
            goto __exit;
        }
            
        /* (CSCvn43011)Base on Intel reply there isn't have solution so we add retry
         * here to workaround Denverton loopback issue*/
        if (retval == FAILED) {
                retry_count++;
        if (lpbk_mode_port >= SGMII_PHY_LPBK_EXTERNAL_PORT0) {
            printf("\nWarning this is external lpbk retry %d time\n", retry_count);
        } else {
            printf("\nWarning this is internal lpbk retry %d time\n", retry_count);
        }
                
            ifconfig_down_up_eth(iface_name);
                
            goto retry;
        }
__exit:
        /* Check retval */
        if (retval == FAILED) {
            cterr('f', 0, "Fail at tx and rx packet");
            return (retval);
        }
        /* phy clean up */
        retval = MVL_88E1543_CLR_LPBK(gephy_obj_p, port);
        if (retval != PASSED) {
            cterr('f', 0, "%s(): PHY loopback clean up failed", __FUNCTION__);
            return (retval);
        }
    } /* for speed */
    prcomplete(testpass, errcount, (char *)0);
    return (retval);
}

/***********************************************************************
 *
 * Function: diag_gephy_get_obj
 *
 * Description: This function create 88e1543 device object.
 *
 * Input : None
 *
 * Output: PASSED/FAILED
 *
 ***********************************************************************
 */
dev_object_t *mrvl88e1543_get_object(void)
{
    dev_88e1543_object_t *gephy_obj_p = &dev_mrvl_88e1543_phy0;

    if (mrvl88e1543_dev_init == FALSE) {
        /* Create the device object */
        dev_88e1543_create((dev_object_t *)gephy_obj_p, 
                           (dev_error_report_t)phy_88e1543_err_report);

        /*
         * Initialize the call-out function vectors:
         */
        gephy_obj_p->callout_fvt->smi_read = diag_gephy_smi_rd;
        gephy_obj_p->callout_fvt->smi_write = diag_gephy_smi_wr;
        gephy_obj_p->callout_fvt->get_linkup_status = diag_gephy_get_linkup_status;

        /* init all device specific function vectors */
        if (DEV_ATTACH((dev_object_t *)gephy_obj_p)) {
            cterr('f', 0, "Failed to attach 88E1543 PHY device");
        }
    }

    mrvl88e1543_dev_init = TRUE;
    return ((dev_object_t *)gephy_obj_p);
}
/*-------------------------------------------------
$Log: diag_gephy_test.c,v $
Revision 1.9  2020/09/30 09:46:09  alicehua
CSCvv85097: Marvell 88e1543 Register test failed when port is plugged with cable

Revision 1.8  2019/12/19 07:27:17  harrchan
Add single port test in GEPHY menu(CSCvs46809)

Revision 1.7  2019/10/16 23:47:51  alicehua
CSCvr66530: Add utility to read PHY page directly.

Revision 1.6  2019/09/11 09:09:31  harrchan
Add retry mechanism for no link up issue on gephy loopback test(CSCvr12865)

Revision 1.5  2019/08/28 01:22:04  alicehua
1.CSCvr03904: Add retry to workaround Denverton loopback issue.
2.CSCvr03919: Fix console will hang when testing Marvell 88e1543 Interrupt test.

Revision 1.4  2019/07/11 12:31:28  alicehua
Collapse Nutella codes into main trunk.

$Endlog$
*/
