/* $Id: diag_gephy_1543_test.c,v 1.3 2020/04/20 02:28:24 lucywang Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/nanook/diag_gephy_1543_test.c,v $
 *------------------------------------------------------------------
 *
 * diag_gephy_test.c - This file is for Marvell 1543 gephy test 
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
#include "platform_fru.h"
#include "platform_cookie.h"
#include "plat_defs.h"
#include "diag_eth_pkt_txrx.h"
#include "diag_gephy_1543_test.h"
#include "diag_gephy_1543_lib.h"
#include "diag_gephy_1543_util.h"
#include "dnv_gpio_lib.h"
#include "dnv_eth_lib.h"
#include "diag_common.h"
#include "dash_fpga.h"
#include "dev_88e1543.h"
#include "i2c_api.h"
#include "i2c_dev.h"
#include "diag_i2c_addr.h"
#include "platform_i2c.h"


/*
 * Global variables
 */
static dev_88e1543_object_t    dev_mrvl_88e1543_phy0; 
static int mrvl88e1543_dev_init = FALSE;
static int speed_index[2] = {1000,100};
static int sfp_speed_index[2] = {1000, 100};
/* Local functions */
int diag_copper_ext_lpbk_test(void);


/* Local build menu functions */
int build_gephy_1543_test_menu(boolean);

static int build_gephy_util_menu(boolean);
static int diag_phy_reg_test(int);
static int diag_phy_int_test(int);
static int diag_gephy_int_lpbk_test(int);
static int diag_gephy_ext_lpbk_test(int);
static int diag_gephy_sfp_ext_lpbk_test(int);

static int mvl_88e1543_set_int_qsgmii_lpbk(int);
static int mvl_88e1543_set_ext_qsgmii_lpbk(int,int,int);
static int mvl_88e1543_clear_ext_qsgmii_lpbk(int,int,int);
dev_object_t *mrvl88e1543_get_object(void);

static int diag_sfp_i2c_test(int);
static int diag_gephy_sfp_status(int);
int check_sfp_speed_100(int);
static int read_sfp_i2c_reg(int,int,int,ushort *,int);

/*
 * Sub Menu used for "GE PHY test -> GE PHY submenu test"
 */

submenu_xtable_t gephy_1543_submenu_table[] = {

    {"GE PHY Utility",
     (PFT) build_gephy_util_menu, 0, 0,
     (type_t(*)())0, 0, (type_t(*)()) build_gephy_util_menu, 0},

    {"PHY Interrupt Test",
     (PFT) diag_phy_int_test, FALSE,
     MF_CONTINUOUS | MF_DOGRP | MF_DOALL,
     (type_t(*)())0, 0, (PFT) 0, 0},
    
    {"PHY Register Test",
     (PFT) diag_phy_reg_test, FALSE,
     MF_CONTINUOUS | MF_DOGRP | MF_DOALL,
     (type_t(*)())0, 0, (PFT) 0, 0},

    {"GE0 PHY Internal Loopback Test",
     (PFT) diag_gephy_int_lpbk_test, GE0,
     MF_CONTINUOUS | MF_DOGRP | MF_DOALL,
     (type_t(*)())0, 0, (PFT) 0, 0},
    
    {"GE1 PHY Internal Loopback Test",
     (PFT) diag_gephy_int_lpbk_test, GE1,
     MF_CONTINUOUS | MF_DOGRP | MF_DOALL,
     (type_t(*)())0, 0, (PFT) 0, 0},

    {"GE0 PHY External Loopback Test",
     (PFT) diag_gephy_ext_lpbk_test, GE0,
     MF_CONTINUOUS | MF_DOGRP | MF_DOALL,
     (type_t(*)())0, 0, (PFT) 0, 0},
    
    {"GE1 PHY External Loopback Test",
     (PFT) diag_gephy_ext_lpbk_test, GE1,
     MF_CONTINUOUS | MF_DOGRP | MF_DOALL,
     (type_t(*)())0, 0, (PFT) 0, 0},

    {"SFP0 External Loopback Test",
     (PFT) diag_gephy_sfp_ext_lpbk_test, SFP0,
     MF_CONTINUOUS | MF_DOGRP | MF_DOALL,
     (type_t(*)())0, 0, (PFT) 0, 0},
    
    {"SFP1 External Loopback Test",
     (PFT) diag_gephy_sfp_ext_lpbk_test, SFP1,
     MF_CONTINUOUS | MF_DOGRP | MF_DOALL,
     (type_t(*)())0, 0, (PFT) 0, 0},
     
    {"SFP0 I2C Test",
     (PFT) diag_sfp_i2c_test, 0,
     MF_CONTINUOUS | MF_DOGRP | MF_DOALL,
     (type_t(*)())0, 0, (PFT) 0, 0},
     
    {"SFP1 I2C Test",
     (PFT) diag_sfp_i2c_test, 1,
     MF_CONTINUOUS | MF_DOGRP | MF_DOALL,
     (type_t(*)())0, 0, (PFT) 0, 0},

};

#define GEPHY_1543_SUBMENU_TABLE_SIZE (sizeof(gephy_1543_submenu_table) / \
                                  sizeof(submenu_xtable_t))

static mitem_t gephy_primary_items[GEPHY_1543_SUBMENU_TABLE_SIZE + MAX_BASE_ITEMS];
static mitem_t gephy_secondary_items[GEPHY_1543_SUBMENU_TABLE_SIZE + MAX_BASE_ITEMS];

menuinfo_t gephy_1543_subtest_menu = {
    "%s Subtest Menu",
    0,                          /* mtparam added by init_empty_menu */
    (PFT) show_endnote,         /* notes missing WICs in combos */
    0,                          /* use generic prompt */
    0,                          /* size (bumped by add_menu_item() */
    gephy_primary_items,
};

menuinfo_t *gephy_1543_submenup = &gephy_1543_subtest_menu;


/*
 * Sub Menu used for "GE PHY test -> GE PHY submenu test -> GE PHY utility submenu"
 */

submenu_xtable_t gephy_1543_util_submenu_table[] = {
    {"GE PHY Register Read Utility", (PFT) diag_gephy_1543_read_reg_util, FALSE, 0,
     (type_t(*)())0, 0, (PFT) 0, 0},
    {"GE PHY Register Write Utility", (PFT) diag_gephy_1543_alter_reg_util, FALSE, 0,
     (type_t(*)())0, 0, (PFT) 0, 0},
    {"Send Packet to GE PHY",
     (PFT) diag_gephy_1543_send_pkt_util, FALSE,
     MF_CONTINUOUS | MF_DOGRP | MF_DOALL,
     (type_t(*)())0, 0, (PFT) 0, 0},
    {"GE PHY Test Mode Utility", (PFT) diag_gephy_1543_testmode_util, FALSE, 0,
     (type_t(*)())0, 0, (PFT) 0, 0},
    {"GE PHY QSGMII Copper Ping Config Utility", 
     (PFT) diag_gephy_1543_ping_config_util, COPPER, 0,
     (type_t(*)())0, 0, (PFT) 0, 0},
    {"GE PHY QSGMII SFP Ping Config Utility", 
     (PFT) diag_gephy_1543_ping_config_util, FIBER, 0,
     (type_t(*)())0, 0, (PFT) 0, 0},
    {"SFP0 Port Status", 
     (PFT) diag_gephy_sfp_status, SFP0, 0,
     (type_t(*)())0, 0, (PFT) 0, 0},
    {"SFP1 Port Status", 
     (PFT) diag_gephy_sfp_status, SFP1, 0,
     (type_t(*)())0, 0, (PFT) 0, 0},
    {"Force GE PHY Speed Utility", 
     (PFT) diag_gephy_1543_force_phy_speed_util, 0, 0,
     (type_t(*)())0, 0, (PFT) 0, 0},
    {"SFP send packets to another SFP",
     (PFT) diag_gephy_1543_sfp_send_pkt_util, FALSE,
     MF_CONTINUOUS | MF_DOGRP | MF_DOALL,
     (type_t(*)())0, 0, (PFT) 0, 0},
};

#define GEPHY_UTIL_SUBMENU_TABLE_SIZE (sizeof(gephy_1543_util_submenu_table) / \
                                       sizeof(submenu_xtable_t))

static mitem_t gephy_util_primary_items[GEPHY_UTIL_SUBMENU_TABLE_SIZE +
                                        MAX_BASE_ITEMS];
static mitem_t gephy_util_secondary_items[GEPHY_UTIL_SUBMENU_TABLE_SIZE +
                                          MAX_BASE_ITEMS];

menuinfo_t gephy_1543_util_subtest_menu = {
    "%s Subtest Menu",
    0,                          /* mtparam added by init_empty_menu */
    (PFT) show_endnote,         /* notes missing WICs in combos */
    0,                          /* use generic prompt */
    0,                          /* size (bumped by add_menu_item() */
    gephy_util_primary_items,
};

menuinfo_t *gephy_1543_util_submenup = &gephy_1543_util_subtest_menu;

int diag_ge_phy_no;

/*******************************************************************************
 *
 * Function   : build_gephy_1543_test_menu
 * Description: GE PHY Test Menu, 88E1543
 * Inputs     : Test/Menu 
 * Outputs    : None
 *
 *******************************************************************************
 */
int build_gephy_1543_test_menu (boolean mb_temp_test_items_executed)
{
    char *menu_title= "GE PHY 1543 Test";
    int rc = FAILED;

    dev_88e1543_object_t *gephy_obj_p;

    gephy_obj_p = (dev_88e1543_object_t *)mrvl88e1543_get_object();

    if (gephy_obj_p == NULL) {
        cterr('f', 0, "%s: Null pointer", __FUNCTION__);
        return (FAILED);
    }
    
    rc = phy_88e1543_dev_init(gephy_obj_p, 
                              NANOOK_PHY0_START_ADDR,
                              MRV88E1543_PHY_ADDR_INCR);


    build_primary_submenu(gephy_1543_submenu_table, GEPHY_1543_SUBMENU_TABLE_SIZE,
                          menu_title, &gephy_1543_submenup);
    build_secondary_submenu(gephy_1543_submenu_table, GEPHY_1543_SUBMENU_TABLE_SIZE,
                            gephy_secondary_items);
    if (mb_temp_test_items_executed) {
        menu(&gephy_1543_subtest_menu, gephy_secondary_items, 0);
    } else {
        do_all_menu_items(gephy_1543_submenup);
    }

    return (rc);
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

    sprintf(menu_title, "GE PHY 1543 Utility");

    build_primary_submenu(gephy_1543_util_submenu_table, GEPHY_UTIL_SUBMENU_TABLE_SIZE,
                          menu_title, &gephy_1543_util_submenup);
    build_secondary_submenu(gephy_1543_util_submenu_table, GEPHY_UTIL_SUBMENU_TABLE_SIZE,
                            gephy_util_secondary_items);
    menu(&gephy_1543_util_subtest_menu, gephy_util_secondary_items, 0);

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

    testname("1543 PHY Register");
    prpass(testpass,"1543 PHY Register,");

    prpass(testpass, "Marvell 88E1543 PHY Register,");
    gephy_obj_p = (dev_88e1543_object_t *)mrvl88e1543_get_object();

    if (gephy_obj_p == NULL) {
        cterr('f', 0, "%s: Null pointer", __FUNCTION__);
        return (FAILED);
    }

    if (MVL_88E1543_REG_TEST(gephy_obj_p) != PASSED) {
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
    int rc = FAILED, intr_sts = 0, ix, link_status;
    uint reg_addr;
    uint32_t  reg_bk;

    testname("1543 PHY Interrupt");
    prpass(testpass,"1543 PHY Interrupt,");

    diag_gephy_1543_init();
    msleep(200);

    gephy_obj_p = (dev_88e1543_object_t *)mrvl88e1543_get_object();

    if (gephy_obj_p == NULL) {
        cterr('f', 0, "%s: Null pointer", __FUNCTION__);
        return (FAILED);
    }
    rc = phy_88e1543_dev_init(gephy_obj_p, 
                              NANOOK_PHY0_START_ADDR,
                              MRV88E1543_PHY_ADDR_INCR);
            if (rc != PASSED) {
                printf("%s(): PHY init", __FUNCTION__);
                return (rc);
            }

    /* Check the CPU interrupt first */
    prpass(testpass, "Check CPU Interrupt (Before the test)");

    reg_addr = DASH_FPGA_MISCELLANEOUS_INT_STS_REG;;
    dash_fpga_reg_read(reg_addr, &reg_bk);
    intr_sts = ((reg_bk & MISC_INT_88E1543_INT_STS) >> MISC_INT_88E1543_INT_BIT);   

    /* Check SFP port link status*/
    link_status = dnv_eth_link_is_up(NANOOK_PHY0_START_ADDR + 1);
    
    if (link_status != TRUE) {
        if (intr_sts == MISC_INT_88E1543_INT_PENDING) {
            cterr('f', 0, "PHY Interrupt is already asserted");
            return (FAILED);
        }

        /* Enable and Force Interrupt */
        /* Force 88E1543 PHY1 to generate interrupt to host */
        prpass(testpass, "Enable and Force Interrupt");
        if (MVL_88E1543_INTR_GEN(gephy_obj_p, NANOOK_PHY0_START_ADDR + 1) != PASSED) {
            return (FAILED);
        }
    } else {
        
        /* Enable and Force Interrupt */
        /* Force 88E1543 PHY1 to generate interrupt to host */
        prpass(testpass, "Enable and Force Interrupt*****");
        if (MVL_88E1543_INTR_GEN_FIBER(gephy_obj_p, NANOOK_PHY0_START_ADDR + 1) != PASSED) {
            return (FAILED);
        }
    }

    /* Check if CPU senses the interrupt coming from the PHY */
    prpass(testpass, "Check CPU Interrupt (After the test)");
    for (ix = 0; ix < GEPHY_INT_TIMEOUT; ix++) {
        rc = dash_fpga_reg_read(reg_addr, &reg_bk);
        if (rc != PASSED) {
            cterr('f', 0, "Read FPGA Value Fails");
        }
        printf("FPGA data 0x%x\n", reg_bk);
        intr_sts = ((reg_bk & MISC_INT_88E1543_INT_STS) >> MISC_INT_88E1543_INT_BIT);   
        if (intr_sts == MISC_INT_88E1543_INT_PENDING) {
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
    if (link_status != TRUE) {
        rc = MVL_88E1543_INTR_DISABLE(gephy_obj_p, NANOOK_PHY0_START_ADDR + 1);
    } else {
        rc = MVL_88E1543_INTR_CLR_FIBER(gephy_obj_p, NANOOK_PHY0_START_ADDR + 1);
    }

    if (rc != PASSED) {
        cterr('f', 0, "Disable Interrupt Fails");
        return (FAILED);
    }

    prcomplete(testpass, errcount, (char *)0);

    return (rc);
}


/******************************************************************************
 *
 * Function: diag_gephy_int_lpbk_test 
 *
 * Description: PHY Internal Loopback Test
 *
 * Inputs      : pn - port number
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/
static int diag_gephy_int_lpbk_test (int pn)
{
    int retval = FAILED, port = 0;
    int speed, link_status, jx, retry_count;
    char iface_name[16];
    dev_88e1543_object_t *gephy_obj_p;

    testname("1543 PHY Internal Loopback");
    prpass(testpass,"1543 PHY Internal Loopback,");

    gephy_obj_p = (dev_88e1543_object_t *)mrvl88e1543_get_object();

    if (gephy_obj_p == NULL) {
        cterr('f', 0, "%s: Null pointer", __FUNCTION__);
        return (FAILED);
    }
    
    if (pn == GE0) {
        port = NANOOK_PHY_QSGMII_PORT0;
    } else if (pn == GE1) {
        port = NANOOK_PHY_QSGMII_PORT1;
    } else {
        printf("%s: Failed Port number (%d)\n", __func__, port);
        return (FAILED);
    }

    for (speed = ETH_MODE_GE; speed <= ETH_MODE_FE100; speed ++) { 
        retry_count = 0;
retry:
        prpass(testpass, "PHY Internal Loopback at port %d "
                         "(speed %dMbps),", port, speed_index[speed]);
        /* Give the interface name */
        if (dnv_eth_get_iface_name(port, iface_name) == FAILED) {
            printf("%s: Get Iface name failed (%d)\n", __func__, port);
            return (FAILED);
        }

        /* SET LPBK */
        retval =  mvl_88e1543_set_int_qsgmii_lpbk(port);
        if (retval != PASSED) {
            printf("%s(): phy int loopback preset failed (phy addr %#x)\n",
                    __FUNCTION__, port);
            return (retval);
        }

        retval = MVL_88E1543_SET_INT_QSGMII_LPBK(gephy_obj_p, port, speed, 
                                      SGMII_PHY_LPBK_INTERNAL);
        if (retval != PASSED) {
            printf("%s(): phy int loopback setup failed (phy addr %#x)\n",
                    __FUNCTION__, port);
            return (retval);
        }

        /* Transmit data and verify data */
        /* Make sure that link status is detected before performing loopback */
        for (jx = 0; jx < LPBK_LINK_UP_TOUT; jx++) {
            link_status = dnv_eth_link_is_up(port);

            if (link_status == TRUE) {
                break;
            }
            msleep(CHECK_LINK_UP_DELAY); 
        }

        if (jx == LPBK_LINK_UP_TOUT || link_status == FALSE) {
            cterr('f', 0, "Link is not detected");
            return (FAILED);
        }

        retval = eth_pkt_txrx(iface_name, LPBKTEST_PKT_CNT, FALSE);

        /* Check whether the retry times is reach maximum */
        if ((retry_count == NANOOK_LPBK_RETRY) && (retval == FAILED)) {
            cterr('f', 0, "Loopback failed (%dMbps)", speed_index[speed]);
            goto __exit;
        }

        /* (CSCvn43011)Base on Intel reply there isn't have solution so we add retry
            here to workaround Denverton loopback issue */
        if (retval == FAILED) {
            retry_count++;
            printf("\nWarning this is internal lpbk retry %d time\n", retry_count);
            ifconfig_down_up_eth(iface_name);

            goto retry;
        }

__exit:

        /* phy clean up */
        retval = phy_88e1543_dev_init(gephy_obj_p, 
                          NANOOK_PHY0_START_ADDR,
                          MRV88E1543_PHY_ADDR_INCR);
        if (retval != PASSED) {
            printf("%s(): PHY loopback clean up failed", __FUNCTION__);
            return (retval);
        }
    } /* for speed */

    prcomplete(testpass, errcount, (char *)0);
    
    return (retval);
}

/******************************************************************************
 *
 * Function: diag_gephy_ext_lpbk_test
 *
 * Description: Copper External Loopback Test
 *
 * Inputs      : pn - port number
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/
static int diag_gephy_ext_lpbk_test (int pn)
{

    int retval = FAILED, port = 0;
    int speed, link_status, jx, retry_count;
    int maxspeed;
    ushort val, val1;
    char iface_name[16];
    dev_88e1543_object_t *gephy_obj_p;
    char *tname = "1543 PHY External Loopback";

    if (check_menu_flag(D_EXT_LOOPBACK)) {
        prpass(testpass, "\n External loopback flag is off, skip '%s' \
            external loopback test. ", tname);
        prcomplete(testpass, errcount, (char *)0);
        return (PASSED);
    }

    if (!(diagflag_xram & D_MIN_TEST_TIME)) {
        maxspeed = ETH_MODE_GE;
    } else {
        maxspeed = ETH_MODE_FE100;
    }

    testname(tname);
    prpass(testpass,"%s,", tname);

    gephy_obj_p = (dev_88e1543_object_t *)mrvl88e1543_get_object();

    if (gephy_obj_p == NULL) {
        cterr('f', 0, "%s: Null pointer", __FUNCTION__);
        return (FAILED);
    }
    
    if (pn == GE0) {
        port = NANOOK_PHY_AUTO_PORT0;
    } else if (pn == GE1) {
        port = NANOOK_PHY_AUTO_PORT1;
    } else {
        printf("%s: Failed Port number (%d)\n", __func__, port);
        return (FAILED);
    }

    read_sfp_i2c_reg (pn, 0x50, 0x0, &val, 1);
    read_sfp_i2c_reg (pn, 0x56, 0x0, &val1, 1);
    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("val=%x, val1=%x\n", val, val1);
    }
    if ((val == 0x3) || (val1 == 0x3f) || (val1 == 0x7f3f)) {
        if (diag_gephy_smi_wr(port, 0x1, 0x0, 0x1940) == FAILED ){
            return (FAILED);
        }
    }

    for (speed = ETH_MODE_GE; speed <= maxspeed; speed ++) { 
        retry_count = 0;
retry:
        prpass(testpass, "PHY External Loopback at port %d "
                         "(speed %dMbps),", port, speed_index[speed]);
        /* Give the interface name */
        if (dnv_eth_get_iface_name(port, iface_name) == FAILED) {
            printf("%s: Get Iface name failed (%d)\n", __func__, port);
            return (FAILED);
        }

        /* SET LPBK */
        retval = mvl_88e1543_set_ext_qsgmii_lpbk(port, speed,
                                                 COPPER);
        if (retval != PASSED) {
            printf("%s(): phy int loopback setup failed (phy addr %#x)\n",
                    __FUNCTION__, port);
            return (retval);
        }

        /* Transmit data and verify data */
        /* Make sure that link status is detected before performing loopback */
        for (jx = 0; jx < LPBK_LINK_UP_TOUT; jx++) {
            link_status = dnv_eth_link_is_up(port);

            if (link_status == TRUE) {
                break;
            }
            msleep(CHECK_LINK_UP_DELAY);
        }

        if (jx == LPBK_LINK_UP_TOUT || link_status == FALSE) {
            cterr('f', 0, "Link is not detected");
            return (FAILED);
        }

        msleep(SET_PHY_DELAY);
        retval = eth_pkt_txrx(iface_name, LPBKTEST_PKT_CNT, FALSE);

        /* Check whether the retry times is reach maximum */
        if ((retry_count == NANOOK_LPBK_RETRY) && (retval == FAILED)) {
            cterr('f', 0, "Loopback failed (%dMbps)", speed_index[speed]);
            goto __exit;
        }

        /* (CSCvn43011)Base on Intel reply there isn't have solution so we add retry
            here to workaround Denverton loopback issue */
        if (retval == FAILED) {
            retry_count++;
            printf("\nWarning this is external lpbk retry %d time\n", retry_count);
            ifconfig_down_up_eth(iface_name);

            goto retry;
        }
__exit:

        if (retval != PASSED) {
            printf("%s(): PHY loopback tx/rx failed", __FUNCTION__);
            return (retval);
        }
        
        if ((val == 0x3) || (val1 == 0x3f) || (val1 == 0x7f3f)) {
            if (diag_gephy_smi_wr(port, 0x1, 0x0, 0x1140) == FAILED ){
                return (FAILED);
            }
        }

        /* phy clean up */
        retval = mvl_88e1543_clear_ext_qsgmii_lpbk(port, speed,
                                                   COPPER);

        retval = phy_88e1543_dev_init(gephy_obj_p, 
                          NANOOK_PHY0_START_ADDR,
                          MRV88E1543_PHY_ADDR_INCR);
        if (retval != PASSED) {
            printf("%s(): PHY loopback clean up failed", __FUNCTION__);
            return (retval);
        }
    } /* for speed */

    prcomplete(testpass, errcount, (char *)0);
    
    return (retval);

}

/******************************************************************************
 *
 * Function: mvl_88e1543_set_int_qsgmii_lpbk
 *
 * Description:Set 88e1543 Internal Loopback Test
 *
 * Inputs      : None
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/
static int mvl_88e1543_set_int_qsgmii_lpbk(int phy_addr){

    char cmd[256];
    
    memset(cmd, 0, sizeof(cmd));
    if (phy_addr == NANOOK_PHY_QSGMII_PORT0) {
        sprintf(cmd, "ifconfig %s up > /dev/null", inface_lan1p0);
        system(cmd);
    }

    if (phy_addr == NANOOK_PHY_QSGMII_PORT1) {
        sprintf(cmd, "ifconfig %s up > /dev/null", inface_lan1p1);
        system(cmd);
    }

    /* Reset phy mode (P18_R20_B15) */
    
    /* Set SGMII to QSGMII mode and Reset Phy (P18_R20) */
    if (diag_gephy_smi_wr(phy_addr, 0x12, 0x14, 0x8005) == FAILED ){
        return (FAILED);
    }

    if (phy_addr == NANOOK_PHY_QSGMII_PORT0) {
        sprintf(cmd, "ifconfig %s down > /dev/null; ifconfig %s up > /dev/null",
                inface_lan1p0, inface_lan1p0);
        system(cmd);
    }

    if (phy_addr == NANOOK_PHY_QSGMII_PORT1) {
        sprintf(cmd, "ifconfig %s down > /dev/null; ifconfig %s up > /dev/null",
                inface_lan1p1, inface_lan1p1);
        system(cmd);
    }
    return (PASSED);
}

/******************************************************************************
 *
 * Function: mvl_88e1543_set_ext_qsgmii_lpbk
 *
 * Description:Set 88e1543 External Loopback Test
 *
 * Inputs      : None
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/
static int mvl_88e1543_set_ext_qsgmii_lpbk(int phy_addr, int speed, int lpbk_mode){
    
    char cmd[256];
    
    memset(cmd, 0, sizeof(cmd));
    if (phy_addr == NANOOK_PHY_AUTO_PORT0) {
        sprintf(cmd, "ifconfig %s up > /dev/null", inface_lan1p0);
        system(cmd);
    }

    if (phy_addr == NANOOK_PHY_AUTO_PORT1) {
        sprintf(cmd, "ifconfig %s up > /dev/null", inface_lan1p1);
        system(cmd);
    }

    /* Reset phy mode (P18_R20_B15) */
    
    if ((lpbk_mode == FIBER) && (speed == ETH_MODE_FE100)) {
        if (diag_gephy_smi_wr(phy_addr - 1, 0x12, 0x14, 0x8005) == FAILED ){
            return (FAILED);
        }
        msleep(SLEEP_1MS);
        if (diag_gephy_smi_wr(phy_addr, 0x12, 0x14, 0x8006) == FAILED ){
            return (FAILED);
        }
    } else {
        /* Set QSGMII to Auto Detect and Reset Phy (P18_R20) */
        if (diag_gephy_smi_wr(phy_addr, 0x12, 0x14, 0x8007) == FAILED ){
            return (FAILED);
        }
    }

    if (phy_addr == NANOOK_PHY_AUTO_PORT0) {
        sprintf(cmd, "ifconfig %s down > /dev/null; ifconfig %s up > /dev/null",
                inface_lan1p0, inface_lan1p0);
        system(cmd);
    }

    if (phy_addr == NANOOK_PHY_AUTO_PORT1) {
        sprintf(cmd, "ifconfig %s down > /dev/null; ifconfig %s up > /dev/null",
                inface_lan1p1, inface_lan1p1);
        system(cmd);
    }
    /* Turn off default copper (P0_R0_B11) */ 
    if (diag_gephy_smi_wr(phy_addr, 0x0, 0x0, 0x1940) == FAILED ){
        return (FAILED);
    }
    /* SFP External loopback */
    if (lpbk_mode == FIBER) {
        /* SFP External loopback */
        /* Enable Stub Test (P6_R18_B3) */
        if (diag_gephy_smi_wr(phy_addr, 0x6, 0x12, 0x0008) == FAILED ){
            return (FAILED);
        }
        /* Power On fiber (P1_R0_B11) */
        if (diag_gephy_smi_wr(phy_addr, 0x1, 0x0, 0x1140) == FAILED ){
            return (FAILED);
        }

    /* Copper External loopback */
    } else if (lpbk_mode == COPPER){
        /* Copper Extenal loopback for 1G */

        if (speed == ETH_MODE_GE){
            /* Enable Stub Test (P6_R18_B3) */
            if (diag_gephy_smi_wr(phy_addr, 0x6, 0x12, 0x0008) == FAILED ){
                return (FAILED);
            }
            /* Power On copper (P0_R0_B11) */
            if (diag_gephy_smi_wr(phy_addr, 0x0, 0x0, 0x1140) == FAILED ){
                return (FAILED);
            }

        /* Copper External loopback for 100M */
        } else if (speed == ETH_MODE_FE100){
            /* Set No Advertise (P0_R9_B8,9) */
            if (diag_gephy_smi_wr(phy_addr, 0x0, 0x9, 0x0000) == FAILED ){
                return (FAILED);
            }
            /* Enable MAC Loopback (P2_R21_B14)*/
            if (diag_gephy_smi_wr(phy_addr, 0x2, 0x15, 0x5046) == FAILED ){
                return (FAILED);
            }
            /* Reset Copper, Set Speed, Disable Auto-Negotiation (P0_R0)*/
            if (diag_gephy_smi_wr(phy_addr, 0x0, 0x0, 0xa100) == FAILED ){
                return (FAILED);
            }
        } else {
            printf("%s: Unsupport speed\n", __func__);
            return (FAILED);
    
        }

    }else{
       printf("%s: Unknown loopback mode\n", __func__);
       return (FAILED);
    }
    return(PASSED);


}
/******************************************************************************
 *
 * Function: mvl_88e1543_clear_ext_qsgmii_lpbk
 *
 * Description: Clear 88e1543 External Loopback Test
 *
 * Inputs      : None
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/
static int mvl_88e1543_clear_ext_qsgmii_lpbk(int phy_addr, int speed, int lpbk_mode){

    if ((lpbk_mode == FIBER) && (speed == ETH_MODE_FE100)) {
        if (diag_gephy_smi_wr(phy_addr - 1, 0x12, 0x14, 0x8005) == FAILED ){
            return (FAILED);
        }
    }
    /* Set QSGMII to Auto Detect and Reset Phy (P18_R20) */
    if (diag_gephy_smi_wr(phy_addr, 0x12, 0x14, 0x8007) == FAILED ){
        return (FAILED);
    }

    /* SFP External loopback for 1G */
    if (lpbk_mode == FIBER) {
        /* Disable Stub Test (P6_R18_B3) */
        if (diag_gephy_smi_wr(phy_addr, 0x6, 0x12, 0x0000) == FAILED ){
            return (FAILED);
        }
        /* Power off fiber (P1_R0_B11) */
        if (diag_gephy_smi_wr(phy_addr, 0x1, 0x0, 0x1940) == FAILED ){
            return (FAILED);
        }

    /* Copper External loopback */
    } else if (lpbk_mode == COPPER){
        /* Copper Extenal loopback for 1G */

        if (speed == ETH_MODE_GE){
        /* Disable Stub Test (P6_R18_B3) */
            if (diag_gephy_smi_wr(phy_addr, 0x6, 0x12, 0x0000) == FAILED ){
                return (FAILED);
            }
            
        /* Copper External loopback for 100M */
        } else if (speed == ETH_MODE_FE100){
            /* Set Advertise (P0_R9_B8,9) */
            if (diag_gephy_smi_wr(phy_addr, 0x0, 0x9, 0x0300) == FAILED ){
                return (FAILED);
            }
            /* Disable MAC Loopback (P2_R21_B14)*/
            if (diag_gephy_smi_wr(phy_addr, 0x2, 0x15, 0x1046) == FAILED ){
                return (FAILED);
            }
        } else {
            printf("%s: Unsupport speed\n", __func__);
            return (FAILED);
    
        }

    } else {
       printf("%s: Unknown loopback mode\n", __func__);
       return (FAILED);
    }
    return(PASSED);
}

/******************************************************************************
 *
 * Function: diag_gephy_sfp_ext_lpbk_test
 *
 * Description: Fiber External Loopback Test
 *
 * Inputs      : pn - port number
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/
static int diag_gephy_sfp_ext_lpbk_test (int pn)
{

    int retval = FAILED, port = 0;
    int speed, link_status, jx, retry_count = 0;
    char iface_name[16];
    dev_88e1543_object_t *gephy_obj_p;
    char *tname = "1543 SFP External Loopback";

    if (check_menu_flag(D_EXT_LOOPBACK)) {
        prpass(testpass, "\n External loopback flag is off, skip '%s' \
            external loopback test. ", tname);
        prcomplete(testpass, errcount, (char *)0);
        return (PASSED);
    }

    testname(tname);
    prpass(testpass,"%s,", tname);

    gephy_obj_p = (dev_88e1543_object_t *)mrvl88e1543_get_object();

    if (gephy_obj_p == NULL) {
        cterr('f', 0, "%s: Null pointer", __FUNCTION__);
        return (FAILED);
    }
    
    if (pn == SFP0) {
        port = NANOOK_PHY_AUTO_PORT0;
    } else if (pn == SFP1) {
        port = NANOOK_PHY_AUTO_PORT1;
    } else {
        printf("%s: Failed Port number (%d)\n", __func__, port);
        return (FAILED);
    }

    /* Give the interface name */
    if (dnv_eth_get_iface_name(port, iface_name) == FAILED) {
        printf("%s: Get Iface name failed (%d)\n", __func__, port);
        return (FAILED);
    }

    if (check_sfp_speed_100(port)) {
        speed = ETH_MODE_FE100;
    } else {
        speed = ETH_MODE_GE;
    }
retry:
    prpass(testpass, "SFP External Loopback at port %d "
                     "(speed %dMbps),", port, sfp_speed_index[speed]);
    /* SET LPBK */
    retval = mvl_88e1543_set_ext_qsgmii_lpbk(port, speed, 
                                             FIBER);
    if (retval != PASSED) {
        printf("%s(): sfp ext loopback setup failed (phy addr %#x)\n",
                __FUNCTION__, port);
        return (retval);
    }

    /* Transmit data and verify data */
    /* Make sure that link status is detected before performing loopback */
    for (jx = 0; jx < LPBK_LINK_UP_TOUT; jx++) {
        link_status = dnv_eth_link_is_up(port);

        if (link_status == TRUE) {
            break;
        }
        msleep(CHECK_LINK_UP_DELAY); 
    }

    if (jx == LPBK_LINK_UP_TOUT || link_status == FALSE) {
        cterr('f', 0, "Link is not detected");
        return (FAILED);
    }

    msleep(SET_PHY_DELAY);
    retval = eth_pkt_txrx(iface_name, LPBKTEST_PKT_CNT, FALSE);

    /* Check whether the retry times is reach maximum */
    if ((retry_count == NANOOK_LPBK_RETRY) && (retval == FAILED)) {
        cterr('f', 0, "Loopback failed (%dMbps)", speed_index[speed]);
        goto __exit;
    }

    /* (CSCvn43011)Base on Intel reply there isn't have solution so we add retry
        here to workaround Denverton loopback issue */
    if (retval == FAILED) {
        retry_count++;
        printf("\nWarning this is external SFP lpbk retry %d time\n", retry_count);
        ifconfig_down_up_eth(iface_name);

        goto retry;
    }
__exit:

    if (retval != PASSED) {
        printf("%s(): SFP loopback tx/rx failed", __FUNCTION__);
        return (retval);
    }

    /* phy clean up */
    retval = mvl_88e1543_clear_ext_qsgmii_lpbk(port, speed, 
                                               FIBER);

    retval = phy_88e1543_dev_init(gephy_obj_p, 
                      NANOOK_PHY0_START_ADDR,
                      MRV88E1543_PHY_ADDR_INCR);
    if (retval != PASSED) {
        printf("%s(): SFP loopback clean up failed", __FUNCTION__);
        return (retval);
    }

    prcomplete(testpass, errcount, (char *)0);
    
    return (retval);

}

/***********************************************************************
 *
 * Function: diag_gephy_get_obj
 *
 * Description:
 *      This function create 88e1543 device object.
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

        /* init all device specific function vectors */
        if (DEV_ATTACH((dev_object_t *)gephy_obj_p)) {
            cterr('f', 0, "Failed to attach 88E1543 PHY device");
        }

    }

    mrvl88e1543_dev_init = TRUE;
    return ((dev_object_t *)gephy_obj_p);
}

/*****************************************************************************
 *
 * Function   : set_i2c_if_struct
 * Description: fill n2g_i2c_if_t struct based on different mux_id.
 * Inputs     : i2c_if_p: pointer to n2g_i2c_if_t struct
 *                 offset: i2c device offset value
 *              buf_p: pointer to i2c tx/rx buffer
 *                 size: size of beffer
 * Outputs    : PASSED/FAILED
 *
 *****************************************************************************/
static int 
set_i2c_if_struct (uint32_t sfp_id, uint32_t addr, n2g_i2c_if_t* i2c_if_p, int offset, char* buf_p, int size)
{
    memset(i2c_if_p, 0, sizeof(n2g_i2c_if_t));
    i2c_if_p->i2c_bus_type = IOFPGA_I2C;
	
    i2c_if_p->mux = I2C_MUX_ZERO;
    i2c_if_p->offset = offset;
    i2c_if_p->size = size;
    i2c_if_p->buf = buf_p;
    i2c_if_p->i2c_dev = addr;	
    switch (sfp_id) {
        case 0: 
            i2c_if_p->i2c_ctrl = 22;             
             break;
        case 1:
            i2c_if_p->i2c_ctrl = 23;
            break;
        default:
            return (FAILED);
    }       
    return (PASSED);
}   


/******************************************************************************
 *
 * function   : read_sfp_i2c_reg
 * Description: Wrapper to read temperature sensor's register.
 * Inputs     : addr_ptr_id - pointer register ID.
 *                 data_buf_p - pointer to data buffer
 *              size - number of bytes to be read.
 * Outputs    : PASSED/FAILED.
 *
 ******************************************************************************/
static int 
read_sfp_i2c_reg (int sfp_id, int i2c_addr, int addr_ptr_id, ushort * data_buf_p, int size)
{
    uint32_t rc = FAILED;
    n2g_i2c_if_t i2c_if;


    /* Setup I2C API interface struct */
    set_i2c_if_struct(sfp_id, i2c_addr , &i2c_if, addr_ptr_id, (char*)data_buf_p, size);
    rc = n2g_i2c_read(&i2c_if);
    if (rc != PASSED) {
        //cterr('f', 0, "%s:%d Failed to read offset %#x, size = %d(rc = %#x)",
	 //                     __FUNCTION__, __LINE__, addr_ptr_id, size, rc);
        return FAILED;
    }

    return (rc);
}

#if 0
/******************************************************************************
 *
 * function   : write_sfp_i2c_reg
 * Description: Wrapper to write temperature sensor's register.
 * Inputs     : addr_ptr_id - pointer register ID.
 *                 data_buf_p - pointer to data buffer
 *              size - number of bytes to be read.
 * Outputs    : PASSED/FAILED.
 *
 ******************************************************************************/
static int 
write_sfp_i2c_reg (int sfp_id, int addr_ptr_id, ushort * data_buf_p, int size)
{
    uint32_t rc = FAILED;
    n2g_i2c_if_t i2c_if;

    /* Setup I2C API interface struct */
    set_i2c_if_struct(sfp_id, &i2c_if, addr_ptr_id, (char*)data_buf_p, size);
    rc = n2g_i2c_write(&i2c_if);
    if (rc != PASSED) {
        cterr('f', 0, "%s:%d Failed to write offset %#x, size = %d(rc = %#x)",
                      __FUNCTION__, __LINE__, addr_ptr_id, size, rc);
        return (FAILED);
    }
    usleep(100000); /* sleep 100 ms after writing */
    return(rc);
}
#endif

/***********************************************************************
 *
 * Function: diag_sfp_i2c_test
 *
 * Description:
 *      This function verify FPGA i2c SFP test.
 *
 * Input : id - SFP0/SFP1
 *
 * Output: PASSED/FAILED
 *
 ***********************************************************************
 */
static int diag_sfp_i2c_test(int id)
{
    ushort val;
    
    testname("1543 SFP FPGA I2C");
    prpass(testpass,"1543 SFP FPGA I2C,");

    // Original SFP Module Addr 0x50 Reg 0x0 Val 0x3
    read_sfp_i2c_reg (id, 0x50, 0x0, &val, 1);
    if (val == 0x3) {
        prcomplete(testpass, errcount, (char *)0);
        return (PASSED);    
    }

    //Try next...
    //SFP Module from PT Addr 0x56 Reg 0x0 Val 0x3f
    read_sfp_i2c_reg (id, 0x56, 0x0, &val, 1);
    if (val == 0x3f) {
        prcomplete(testpass, errcount, (char *)0);
        return (PASSED);    
    }
    
    cterr('f', 0, "%s:%d Compare SFP FPGA reg failed",
                      __FUNCTION__, __LINE__);
    prcomplete(testpass, errcount, (char *)0);
    return (FAILED);
}

/******************************************************************************
 *
 * Function: diag_gephy_sfp_status
 *
 * Description:  Get SFP module status which inserted or not
 *
 * Inputs      : id - SFP0/SFP1
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/
static int diag_gephy_sfp_status(int id)
{
    uint reg_addr;
    uint32_t  reg_val;
    
    if (id == SFP0) {
        reg_addr = SFP0_CONF_OFFSET;
    } else if (id == SFP1) {
        reg_addr = SFP1_CONF_OFFSET;
    }
    
    dash_fpga_reg_read(reg_addr, &reg_val);
    
    if ((reg_val & SFP_PRESENT_BIT) && (!(reg_val & SFP_TX_FAULT_BIT)) 
                                    && (!(reg_val & SFP_LOSS_SIG_BIT))) {
        printf("SFP%d module is inserted\n", id);
    } else if ((!(reg_val & SFP_PRESENT_BIT)) && (reg_val & SFP_TX_FAULT_BIT) 
                                           && (reg_val & SFP_LOSS_SIG_BIT)) {
        printf("SFP%d module is not insert\n", id);
    } else {
        printf("SFP%d Unknown Status, %x", id, reg_val);
    }
    
    return (PASSED);
}

/***********************************************************************
 *
 * Function: check_sfp_speed_100
 *
 * Description:
 *      This function check SFP speed.
 *
 * Input : port NANOOK_PHY_AUTO_PORT0/NANOOK_PHY_AUTO_PORT1
 *
 * Output: 1 - SFP-100
 *         0 - SFP-1000
 *
 ***********************************************************************
 */
int check_sfp_speed_100(int port)
{
    ushort val = 0;
    int sfp_id;
    int sfp_i2c_addr;
    
    if (port == NANOOK_PHY_AUTO_PORT0) {
        sfp_id = 0;
    } else if (port == NANOOK_PHY_AUTO_PORT1) {
        sfp_id = 1;
    } else {
        return 0;
    }

    //Check if Factory SFP module, if yes, the SFP I2C address should be 0x56, otherwise 0x50.
    read_sfp_i2c_reg (sfp_id, 0x56, 0x0, &val, 1);
    if (val == 0x3f) {
        sfp_i2c_addr = 0x56;
    } else {
        sfp_i2c_addr = 0x50;
    }
    
    read_sfp_i2c_reg (sfp_id, sfp_i2c_addr, 12, &val, 1);
    if (val == 0x1) {
        return 1;
    }
    
    return (0);
}

/******************************************************************************
 *
 * Function: diag_gephy_1543_sfp_force_100 
 *
 * Description: Force to manually config 100M speed
 *
 * Inputs      : None
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/
int diag_gephy_1543_sfp_force_100 (void)
{
    int port = 0;
    
    for (port = NANOOK_PHY_QSGMII_PORT0; port <= NANOOK_PHY_QSGMII_PORT1; port+=2) {
        if (diag_gephy_smi_wr(port, 0x12, 0x14, 0x8005) == FAILED ){
            return (FAILED);
        }
        if (diag_gephy_smi_wr(port, 0x4, 0x0, 0x9140) == FAILED ){
            return (FAILED);
        }
        if (diag_gephy_smi_wr(port, 0x4, 0x1b, 0x7f83) == FAILED ){
            return (FAILED);
        }
    }
    
    for (port = NANOOK_PHY_AUTO_PORT0; port <= NANOOK_PHY_AUTO_PORT1; port+=2) {
        if (diag_gephy_smi_wr(port, 0x12, 0x14, 0x8006) == FAILED ){
            return (FAILED);
        }  
        if (diag_gephy_smi_wr(port, 0x4, 0x0, 0x9140) == FAILED ){
            return (FAILED);
        }
        if (diag_gephy_smi_wr(port, 0x4, 0x1b, 0x7f83) == FAILED ){
            return (FAILED);
        }
    }
    printf("SFP Force speed 100M\n");

    return (PASSED);
}


/*-------------------------------------------------
$Log: diag_gephy_1543_test.c,v $
Revision 1.3  2020/04/20 02:28:24  lucywang

1. Fixed unplug/plug NIM module dynamically issue and added NIM cookie
2. Added to support NIM Prince
3. (CSCvn43011) add retry workaround for Deverton issue
4. add debug message and set default value to seneors
5. Reverted Register value of temp/press snsr after test
6. Bumped up version to 1.0.2

Revision 1.2  2019/12/11 10:10:30  lucywang
Merged Nanook to main trunk


$Endlog$
*/
