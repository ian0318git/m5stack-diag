 /* $Id: diag_esw_test.c,v 1.3 2018/12/10 09:57:05 harrchan Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/viper/diag_esw_test.c,v $
 *------------------------------------------------------------------
 *
 * diag_esw_test.c - This file is for ethernet switch test
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
#include "error.h"
#include "types.h"
#include "queryflags.h"
#include "ethernet.h"
#include "nvsysvars.h"
#include "error.h"
#include "pcmap.h"
#include "strings.h"
#include "menu.h"
#include "common.h"
#include "cross_platform.h"
#include "plat_defs.h"
#include "setjmps.h"
#include "proto.h"
#include "platform_fru.h"
#include "platform_cookie.h"
#include "plat_defs.h"
#include "diag_esw_test.h"
#include "mb_tests.h"
#include "diag_gephy_test.h"
#include "diag_fpga.h"
#include "diag_esw_util.h"
#include "diag_esw_lib.h"
#include "dnv_eth_lib.h"
#include "dev_88e6176.h"
#include "dnv_gpio_lib.h"
#include "diag_nc_lib.h"
#include "dnv_eth_lib.h"
#include "diag_eth_pkt_txrx.h"
#include "diag_fpga.h"


/*
 * Global variables
 */
//#define ENHANCE_ERROR_MSG_RDY 0

/* Local functions */
int diag_smi_reg_test(void);
int diag_eth_ext_lpbk_test(void);
int esw_set_allports_forward(void);
int diag_esw_int_test(void);
static int diag_cpu_esw_mac_lpbk_test(int);

/* Local build menu functions */
int build_esw_test_menu(boolean);
int build_esw_util_menu(boolean);

/* ESW supported speed table */
static int     esw_speed_tbl[] = {SPD_10MBPS, SPD_100MBPS, SPD_1000MBPS};

/*
 * Sub Menu used for "Ethernet switch test -> Ethernet switch submenu test"
 */

submenu_xtable_t eth_submenu_table[] = {
    {"ESW Utilities",
     (PFT) build_esw_util_menu, FALSE, 0,
     (type_t(*)())0, 0, (PFT) build_esw_util_menu, TRUE},

    {"SMI Register Test",
     (PFT) diag_smi_reg_test, FALSE,
     MF_CONTINUOUS | MF_DOGRP | MF_DOALL,
     (type_t(*)())0, 0, (PFT) 0, 0},
    
    {"Ethernet Switch Interrupt test",
     (PFT) diag_esw_int_test, FALSE,
     MF_CONTINUOUS | MF_DOGRP | MF_DOALL,
     (type_t(*)())0, 0, (PFT) 0, 0},

    {"Internal Loopback Test",
     (PFT) diag_cpu_esw_mac_lpbk_test, ETH0,
     MF_CONTINUOUS | MF_DOGRP | MF_DOALL,
     (type_t(*)())0, 0, (PFT) 0, 0},

    {"External Loopback Test",
     (PFT) diag_eth_ext_lpbk_test, FALSE,
     MF_CONTINUOUS | MF_DOGRP | MF_DOALL,
     (type_t(*)())0, 0, (PFT) 0, 0},
};

#define ETH_SUBMENU_TABLE_SIZE (sizeof(eth_submenu_table) / \
                                     sizeof(submenu_xtable_t))

static mitem_t eth_primary_items[ETH_SUBMENU_TABLE_SIZE +
                                 MAX_BASE_ITEMS];
static mitem_t eth_secondary_items[ETH_SUBMENU_TABLE_SIZE +
                                   MAX_BASE_ITEMS];

menuinfo_t eth_subtest_menu = {
    "%s Subtest Menu",
    0,                          /* mtparam added by init_empty_menu */
    (PFT) show_endnote,         /* notes missing WICs in combos */
    0,                          /* use generic prompt */
    0,                          /* size (bumped by add_menu_item() */
    eth_primary_items,
};

menuinfo_t *eth_submenup = &eth_subtest_menu;


/*
 * Sub Menu used for "Ethernet switch test -> Ethernet switch submenu test -> ESW utility submenu"
 */

submenu_xtable_t esw_util_submenu_table[] = {
    {"ESW Register Read Utility",
     (PFT) diag_esw_reg_rd_util, FALSE, 0,
     (type_t(*)())0, 0, (PFT) 0, 0},

    {"ESW Register Write Utility",
     (PFT) diag_esw_reg_wr_util, FALSE, 0,
     (type_t(*)())0, 0, (PFT) 0, 0},

    {"ESW PHY Register Read Utility",
     (PFT) diag_esw_phy_reg_rd_util, FALSE, 0,
     (type_t(*)())0, 0, (PFT) 0, 0},

    {"ESW PHY Register Write Utility",
     (PFT) diag_esw_phy_reg_wr_util, FALSE, 0,
     (type_t(*)())0, 0, (PFT) 0, 0},

    {"ESW Set All Ports Forwarding",
     (PFT) diag_esw_set_allport_forward_util, FALSE, 0,
     (type_t(*)())0, 0, (PFT) 0, 0},

    {"Set PHY 1000Base-T Test Mode",
     (PFT) esw_set_1k_testmode_util, FALSE, 0,
     (type_t(*)())0, 0, (PFT) 0, 0},

    {"ESW Configures VLAN",
     (PFT) diag_esw_config_vlan_profile, FALSE, 0,
     (type_t(*)())0, 0, (PFT) 0, 0},

    {"Adjust Port VOD",
     (PFT) diag_esw_adjust_port_vod_util, FALSE, 0,
     (type_t(*)())0, 0, (PFT) 0, 0},

    {"SMI C45 Read Utility",
     (PFT) diag_smi_c45_rd_util, FALSE, 0,
     (type_t(*)())0, 0, (PFT) 0, 0},

    {"SMI C45 Write Utility",
     (PFT) diag_smi_c45_wr_util, FALSE, 0,
     (type_t(*)())0, 0, (PFT) 0, 0},
};

#define ESW_UTIL_SUBMENU_TABLE_SIZE (sizeof(esw_util_submenu_table) / \
                                     sizeof(submenu_xtable_t))

static mitem_t esw_util_primary_items[ESW_UTIL_SUBMENU_TABLE_SIZE +
                                      MAX_BASE_ITEMS];
static mitem_t esw_util_secondary_items[ESW_UTIL_SUBMENU_TABLE_SIZE +
                                        MAX_BASE_ITEMS];

menuinfo_t esw_util_subtest_menu = {
    "%s Subtest Menu",
    0,                          /* mtparam added by init_empty_menu */
    (PFT) show_endnote,         /* notes missing WICs in combos */
    0,                          /* use generic prompt */
    0,                          /* size (bumped by add_menu_item() */
    esw_util_primary_items,
};

menuinfo_t *esw_util_submenup = &esw_util_subtest_menu;



/*******************************************************************************
 *
 * Function   : build_esw_test_menu
 * Description: build ethernet switch test sub menu. 
 * Inputs     : Test/Menu 
 * Outputs    : PASSED
 *
 *******************************************************************************
 */
int build_esw_test_menu (boolean mb_temp_test_items_executed)
{
    char *tname = "Ethernet Switch Test";
    testname(tname);

    /* Init Switch */
    if (diag_esw_init() != PASSED) {
        cterr('f', 0, "Failed to init Switch.");
    }

    if(this_is_viper_j()) {
        system(VIPERJ_GE_SWITCH_IFACE_UP);
    } else {
        system(GE_SWITCH_IFACE_UP);
    }
    build_primary_submenu(eth_submenu_table, ETH_SUBMENU_TABLE_SIZE,
                          "Ethernet Switch test", &eth_submenup);
    build_secondary_submenu(eth_submenu_table, ETH_SUBMENU_TABLE_SIZE,
                            eth_secondary_items);
    if (mb_temp_test_items_executed) {
        menu(&eth_subtest_menu, eth_secondary_items, 0);
    } else {
        do_all_menu_items(eth_submenup);
    }
    return (PASSED);
}

/*******************************************************************************
 *
 * Function   : build_esw_util_menu
 * Description: build ethernet switch utility menu.
 * Inputs     : Test/Menu 
 * Outputs    : None
 *
 *******************************************************************************
 */
int build_esw_util_menu (boolean mb_temp_test_items_executed)
{
    char *tname = "ESW utility";
    testname(tname);

    build_primary_submenu(esw_util_submenu_table, ESW_UTIL_SUBMENU_TABLE_SIZE,
                          "ESW util SubMenu", &esw_util_submenup);
    build_secondary_submenu(esw_util_submenu_table, ESW_UTIL_SUBMENU_TABLE_SIZE,
                            esw_util_secondary_items);
    menu(&esw_util_subtest_menu, esw_util_secondary_items, 0);
    return (PASSED);
}

/******************************************************************************
 *
 * Function: diag_smi_reg_test
 *
 * Description: Function to access Ethernet Switch register  
 *
 * Inputs      : None
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/
int diag_smi_reg_test (void)
{
    int rc = FAILED;
    dev_88e6176_object_t esw_obj;
    dev_88e6176_object_t *esw_obj_p  = &esw_obj;
    char *tname = "Switch register test";
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
    cterr_add_component("Intel Denverton SOC C3558", "SMI", 
                        "Marvell 88E6176 Ethernet Switch");

    /* Segment 5: register and memory dump */

    /* Segment 6: Platform Environment initialized here*/

    /* Segment 7: Top 3 Debugging Steps */
    cterr_add_debug("Check the SMI to see whether it's ok or not.",
                    "If the SMI is OK, contact vendor to verify"
                    "if SMI driver is workable.");
    
    testname(tname);

    rc = diag_esw_dev_create(esw_obj_p);

    if (rc == FAILED) {
        cterr('f', 0, "%s: Creating object failed",__func__);
        return (FAILED);
    }

    rc = esw_obj_p->callin_fvt->register_test((dev_object_t *)esw_obj_p);

    esw_obj_p->base.dev_object_fvt->dev_destroy((dev_object_t **)&esw_obj_p);

    prcomplete(testpass, errcount, (char*)0);

    return (rc);
}

/******************************************************************************
 *
 * Function: diag_eth_ext_lpbk_test
 * Description: Function to do Ethernet Switch ports external loopback test.
 * Inputs      : None
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/
int diag_eth_ext_lpbk_test (void)
{
    dev_88e6176_object_t esw_obj;
    dev_88e6176_object_t *esw_obj_p  = &esw_obj;
    int rc = FAILED, retry = 0;
    int start_port = 0, end_port = 0;
    char *tname = "Switch Ext. loopback";
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
    cterr_add_component("Intel Denverton SOC C3558", "SGMII", "Marvell 88E6176 Ethernet Switch",
                        "SMI");

    /* Segment 5: register and memory dump */

    /* Segment 6: Platform Environment initialized here*/

    /* Segment 7: Top 3 Debugging Steps */
    cterr_add_debug("Change another external loopback cable.",
                    "If step a. is still failed, try the internal "
                    "loopback to check if internal loopback is OK.",
                    "If step b. is OK, we can know PHY has problems. "
                    "If step b is failed, please try step d.",
                    "Observe PCIe register status to "
                    "check if switch configuration is normal.",
                    "If step d. is OK, we can assume the interface "
                    "between Host SoC and switch has problems.");

    testname(tname);

    /* Check if Ext. Loopback Flag is ON */
    if (check_ext_lpbk_flag() != TRUE) {
        printf("Skip %s test because Ext. Loopback Flag is OFF.\n", tname);
        return (PASSED);
    }

    rc = diag_esw_dev_create(esw_obj_p);

    if (rc == FAILED) {
        cterr('f', 0, "%s: Creating object failed",__func__);
        return (FAILED);
    }

    prpass(testpass, "Start to do external loopback, ");
    start_port = ESW_PORT0;
    end_port = ESW_PORT3;
    retry = 0;

    /* (CSCvn43011)Base on Intel reply there isn't have solution so we add retry here to 
     * workaround Denverton loopback issue*/
    if (esw_obj_p->callin_fvt->ext_lpbk_test((dev_object_t *)esw_obj_p,
                                                start_port, end_port)!= PASSED) {
        while (retry <= VIPER_LPBK_RETRY) {
            retry++;
            printf("\nWarning this is external lpbk retry %d time\n", retry);
            if (this_is_viper_j()) {
                system(VIPERJ_GE_SWITCH_IFACE_DOWN);
                system(VIPERJ_GE_SWITCH_IFACE_UP);
            } else {
                system(GE_SWITCH_IFACE_DOWN);
                system(GE_SWITCH_IFACE_UP);
            }

            rc = esw_obj_p->callin_fvt->ext_lpbk_test((dev_object_t *)esw_obj_p,
                                                      start_port, end_port);
            if (rc == PASSED) {
                break;
            } else {
                if (retry == VIPER_LPBK_RETRY) {
                    cterr('f', 0, "Failed at ESW external loopback");
                    goto _exit;
                }
            }
        }
    }
_exit:
    esw_obj_p->base.dev_object_fvt->dev_destroy((dev_object_t **)&esw_obj_p);

    prcomplete(testpass, errcount, (char*)0);

    return (rc);
}

/*******************************************************************************
 *
 * Function   : diag_esw_int_test
 * Description: ethernet switch interrupt test 
 * Inputs     : None
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
int diag_esw_int_test (void)
{
    dev_88e6176_object_t esw_obj;
    dev_88e6176_object_t *esw_obj_p  = &esw_obj;
    int port, ix;
    uint gpio_value;
    ushort reg_val;
    unsigned char marv_6176_gpio_pin = DNV_GPIO_7;
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
    cterr_add_component("Intel Denverton SOC C3558", "GPIO", "SMI", 
                        "Marvell 88E6176 Ethernet Switch");

    /* Segment 5: register and memory dump */

    /* Segment 6: Platform Environment initialized here*/

    /* Segment 7: Top 3 Debugging Steps */
    cterr_add_debug("Excute SMI Register Test to make sure PHY "
                    "is accessible.",
                    "Measure GPIO of CPU to make sure HW signal"
                    " is good.");

    /* Create 88e6176 device driver */
    if (diag_esw_dev_create(esw_obj_p) == FAILED) {
        cterr('f', 0, "%s: Creating object failed",__func__);
        return (FAILED);
    }

    /* Read global reg (1B_0) first for clear EEINT and GPIO will go high*/ 
    if (esw_obj_p->callin_fvt->esw_reg_read((dev_object_t *)esw_obj_p
                        , ESW_GLOBAL1_REG, REG_ADDR(0), &reg_val) == FAILED) {
        goto _exit;
    }

    if (dnv_gpio_read_rx_val(marv_6176_gpio_pin, &gpio_value) != PASSED) {
        printf("\n Can not read CPU GPIO \n");
        cterr('f', 0, "Can not read CPU GPIO");
        goto _exit;
    }
    /* Check if interrupt is already asserted before the test */
    if (gpio_value == GPIO_LOW) {
        /* Before the test interrupt is occur need to clear it*/ 
        reg_val = DISABLE_ALL_INT_PHY_REG;
        for (port = ESW_PORT0; port <= ESW_PORT3; port ++) {
            if (esw_obj_p->callin_fvt->esw_phy_reg_write((dev_object_t *)esw_obj_p
                                , port, PHY_PAGE(0), PHY_REG(18), reg_val) != PASSED) {
                printf("%s:%d Failed to set interrupt on  port%d.",
                   __FUNCTION__, __LINE__, port);
                goto _exit;
            }
        }

        if (dnv_gpio_read_rx_val(marv_6176_gpio_pin, &gpio_value) != PASSED) {
            printf("\n Can not read CPU GPIO \n");
            cterr('f', 0, "Can not read CPU GPIO");
            goto _exit;
        }
        /* check gpio after disable interrupt  */
        if (gpio_value == GPIO_LOW) {
            printf("After disable ESW GPIO interrupt pin still assert");
            goto _exit;
        }
    }

    /* 
     * reset and out of reset 88e6176 This will cause ESW send interrupt
     * to CPU
     */
    if (fpga_reset_api(FPGA_EXTER_DEV_RST_REG, EXT_ESW_RESET, TRUE,
                          WAITTIME_20_MS) != PASSED) {
        printf("%s: Failed to reset switch.\n", __FUNCTION__);
        goto _exit;
    }
    
    /* Release ESW from reset */
    if (fpga_reset_api(FPGA_EXTER_DEV_RST_REG, EXT_ESW_RESET, FALSE,
                          WAITTIME_20_MS) != PASSED) {
        printf("%s: Failed to release switch from Reset.\n", __FUNCTION__);
        return (FAILED);
    }

    /* After release esw from reset need to init esw for rest test */
    if (diag_esw_init() != PASSED) {
        cterr('f', 0, "Failed to init Switch.");
    }
    
    /* Polling status od GPIO   */
    for (ix = MAX_RETRY; ix > 0 ; ix--) {
        if (dnv_gpio_read_rx_val(marv_6176_gpio_pin, &gpio_value) != PASSED) {
            printf("\n Can not read CPU GPIO \n");
            cterr('f', 0, "Can not read CPU GPIO");
            goto _exit;
        }

        if (gpio_value == GPIO_HIGH) {
            printf("\n.\n");
        } else { 
            printf("\n receive interrupt from ESW\n");
            break;
        }
        msleep(POLL_DELAY);
    }

    if (ix == 0) {
        cterr('f', 0, "Did not receive interrupt");
        goto _exit;
    }

    /* Read global reg (1B_0) first for clear EEINT and GPIO will go high*/ 
    if (esw_obj_p->callin_fvt->esw_reg_read((dev_object_t *)esw_obj_p
                        , ESW_GLOBAL1_REG, REG_ADDR(0), &reg_val) == FAILED) {
        goto _exit;
    }

    /* Check whether interrupt is asserted or not. */
    for (ix = MAX_RETRY; ix > 0 ; ix--) {
        if (dnv_gpio_read_rx_val(marv_6176_gpio_pin, &gpio_value) != PASSED) {
            printf("\n Can not read CPU GPIO \n");
            return (FAILED);
        } 
        
        if (gpio_value == GPIO_HIGH) {
            break;
        }
        msleep(POLL_DELAY);
    }

    if (ix == 0) {
        cterr('f', 0, "\n interrupt did not clear\n");
        goto _exit;
    }
    esw_obj_p->base.dev_object_fvt->dev_destroy((dev_object_t **)&esw_obj_p);
    prcomplete(testpass, errcount, (char *)0);
    return (PASSED);

_exit:
    esw_obj_p->base.dev_object_fvt->dev_destroy((dev_object_t **)&esw_obj_p);
    prcomplete(testpass, errcount, (char *)0);
    return (FAILED);
}

/*******************************************************************************
 *
 * Function   : diag_cpu_esw_mac_lpbk_test
 * Description: Function to do Viper CPU to switch PHY MAC loopback test.
 * Inputs     : opt - reserve for future use
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
static int diag_cpu_esw_mac_lpbk_test (int opt) {
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
    cterr_add_component("Intel Denverton SOC C3558", "SMI", "SGMII", 
                        "Marvell 88E6176 Ethernet Switch");

    /* Segment 5: register and memory dump */

    /* Segment 6: Platform Environment initialized here*/

    /* Segment 7: Top 3 Debugging Steps */
    cterr_add_debug("Observe SMI register status to check if switch"
                    " configuration is normal.",
                    "If step a. is OK, we can assume the interface "
                    "between Host SoC and switch has problems.");

    dev_88e6176_object_t esw_obj;
    dev_88e6176_object_t *esw_obj_p  = &esw_obj;
    char tname[32]; 
    char iface_name[16];
    int  rc = FAILED, retry = 0, p_ctr = 0, spd_ctr = 0, test_spd = 0;
    int  start_port = 0, end_port = 0;
    int  total_spd = 0;

    memset(tname, 0, sizeof(tname));
    sprintf(tname, "CPU to switch PHY MAC loopback");
    testname(tname);
    prpass(testpass, "%s, ", tname);

    /* Create device driver */
    if (diag_esw_dev_create(esw_obj_p) !=PASSED) {
        cterr('f', 0, "%s: Creating object failed",__func__);
        return (FAILED);
    }

    if (dnv_eth_get_iface_name(VIPER_88E6176, iface_name) == FAILED) {
        printf("%s: Get Iface name failed\n", __func__);
        goto _exit;
    }
    
    total_spd = sizeof(esw_speed_tbl) / sizeof(int);

    start_port = (int)ESW_PORT0;
    end_port = (int)ESW_PORT3;

    retry = 0;
    for (p_ctr = start_port; p_ctr <= end_port; p_ctr++) {
        for (spd_ctr = 0; spd_ctr < total_spd; spd_ctr++) {
            test_spd = esw_speed_tbl[spd_ctr];
            prpass(testpass, "Testing switch port%d in %dmbps ",
                             p_ctr, test_spd);

           /* (CSCvn43011)Base on Intel reply there isn't have solution so we add retry
            * here to workaround Denverton loopback issue*/
            if (esw_obj_p->callin_fvt->esw_phy_mac_lpbk_test((dev_object_t *)esw_obj_p,
                                            p_ctr, test_spd) != PASSED) {
                while (retry <= VIPER_LPBK_RETRY) {
                    retry++;
                    printf("\nWarning this is internal lpbk retry %d time\n", retry);
                    if(this_is_viper_j()) {
                        system(VIPERJ_GE_SWITCH_IFACE_DOWN);
                        system(VIPERJ_GE_SWITCH_IFACE_UP);
                    } else {
                        system(GE_SWITCH_IFACE_DOWN);
                        system(GE_SWITCH_IFACE_UP);
                    }

                    rc = esw_obj_p->callin_fvt->esw_phy_mac_lpbk_test(
                            (dev_object_t *)esw_obj_p, p_ctr, test_spd);

                    if (rc == PASSED) {
                        break;
                    } else {
                        if (retry == VIPER_LPBK_RETRY) {
                            cterr('f', 0, "Failed at ESW internal lpbk  port%d"
                                  "in %dmbps",p_ctr, test_spd);
                            goto _exit;
                        }
                    }
                }
            }
        }
        if (diag_reset_esw_to_default(TRUE) != PASSED) {
            cterr('f', 0, "Failed reset switch ");
            goto _exit;
        }
    }
    esw_obj_p->base.dev_object_fvt->dev_destroy((dev_object_t **)&esw_obj_p);
    prcomplete(testpass, errcount, (char *)0);
    return (PASSED);
_exit:
    esw_obj_p->base.dev_object_fvt->dev_destroy((dev_object_t **)&esw_obj_p);
    prcomplete(testpass, errcount, (char *)0);
    return (FAILED);
}

/*-------------------------------------------------
 * $Log: diag_esw_test.c,v $
 * Revision 1.3  2018/12/10 09:57:05  harrchan
 * Add workaround to PHY and Switch loopback test (CSCvn43011)
 *
 * Revision 1.2  2018/08/06 02:31:50  harrchan
 * Merge viper E2E to the main trunk (CSCvk28469)
 *
 * Revision 1.1.2.9  2018/07/06 02:54:08  harrchan
 * Add enhance error message
 *
 * Revision 1.1.2.8  2018/07/03 05:38:55  harrchan
 * Follow the coding rule to clean up code
 *
 * Revision 1.1.2.7  2018/06/27 06:27:49  harrchan
 * Follow the coding rule to clean up code
 *
 * Revision 1.1.2.6  2018/03/29 08:09:18  harrchan
 * Fixed bug of esw interrupt test
 *
 * Revision 1.1.2.5  2018/03/28 07:03:51  lucywang
 * Added API to check SKU ViperJ and changed interface name for ViperJ
 *
 * Revision 1.1.2.4  2018/03/28 06:43:20  harrchan
 * Modify esw interrupt test to make sure esw send interrupt
 *
 * Revision 1.1.2.3  2018/03/26 09:20:52  harrchan
 * Delete init process on internal loopback test when test change speed
 *
 * Revision 1.1.2.2  2018/03/05 08:54:20  harrchan
 * Initial hydra application code base
 *
 * Revision 1.1.2.1  2018/02/27 08:06:39  harrchan
 * Initial viper application code base
 *
 *
 * $Endlog$
 *-------------------------------------------------
 */
