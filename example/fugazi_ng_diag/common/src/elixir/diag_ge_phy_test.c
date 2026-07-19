/* $Id: diag_ge_phy_test.c,v 1.2 2021/09/24 01:21:06 harrchan Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/elixir/diag_ge_phy_test.c,v $
 *------------------------------------------------------------------
 * 
 * diag_ge_phy_test.c
 *
 * Copyright (c) 2018 - 2019 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#include <stdlib.h>
#include <string.h>
#include "error.h"
#include "common.h"
#include "types.h"
#include "proto.h"
#include "queryflags.h"
#include "menu.h"
#include "nvmonvars.h"
#include "ethernet.h"
#include "common_utils.h"
#include "diag_cpu_lib.h"
#include "diag_cpu_util.h"
#include "diag_enhance_err_msg_lib.h"
#include "platform_cookie.h"
#include "diag_moka_fpga_lib.h"
#include "diag_temp_sensor_util.h"
#include "diag_smi_lib.h"
#include "i2c_api.h"
#include "platform_i2c.h"
#include "dev_mrvl_ge.h"
#include "diag_ge_phy_test.h"
#include "diag_ge_phy_lib.h"
#include "diag_ge_phy_util.h"

#define ENHANCE_ERROR_MSG_RDY 1


int current_sfp_select;
/*******************************************************************************
 *                          Function Declaration
 *******************************************************************************/
static int diag_ge_phy_reg_test(int);
static int diag_ge_phy_copper_mac_lpbk_test(int);
static int diag_ge_phy_copper_ext_lpbk_test(int);
static int diag_ge_phy_sfp_ext_lpbk_test(int);
static int diag_ge_phy_intr_test(int);
static int diag_ge_phy_copper_mac_lpbk_test_1gbps(int);
static int diag_ge_phy_copper_ext_lpbk_test_1gbps(int);

/* utility menu */
int diag_util_menu_88e1112_ge0(int);
int diag_util_menu_88e1112_ge1(int);

/* Map panel port number (Cisco defined) to actual port number (Foxconn HW defined) */
static submenu_xtable_t plat_gephy0_diag_tbl[] = {
    {"GE0 Utilities",
     (type_t(*)())diag_util_menu_88e1112_ge0,                              FALSE,
     0, 
     (type_t(*)())0,                                             0,
     (type_t(*)())0,                                             0},
    {"GE0 Register Test",
     (type_t(*)())diag_ge_phy_reg_test,                           GE1,
     (MF_CONTINUOUS | MF_DOGRP | MF_DOALL | MF_SHOW_ERRCOUNT),
     (type_t(*)())0,                                             0,
     (type_t(*)())0,                                             0},
    {"GE0 MAC Loopback Test",
     (type_t(*)())diag_ge_phy_copper_mac_lpbk_test,               GE1,
     (MF_CONTINUOUS | MF_DOGRP | MF_DOALL | MF_SHOW_ERRCOUNT),
     (type_t(*)())0,                                             0,
     (type_t(*)())0,                                             0},
    {"GE0 External Loopback Test",
     (type_t(*)())diag_ge_phy_copper_ext_lpbk_test,              GE1,
     (MF_CONTINUOUS | MF_DOGRP | MF_DOALL | MF_SHOW_ERRCOUNT),
     (type_t(*)())0,                                             0,
     (type_t(*)())0,                                             0},
    {"GE0 SFP External Loopback Test",
     (type_t(*)())diag_ge_phy_sfp_ext_lpbk_test,                 GE1,
     (MF_CONTINUOUS | MF_DOGRP | MF_DOALL | MF_SHOW_ERRCOUNT),
     (type_t(*)())0,                                             0,
     (type_t(*)())0,                                             0},
    {"GE0 Interrupt Test",
     (type_t(*)())diag_ge_phy_intr_test,                 GE1,
     (MF_CONTINUOUS | MF_DOGRP | MF_DOALL | MF_SHOW_ERRCOUNT),
     (type_t(*)())0,                                             0,
     (type_t(*)())0,                                             0},
    {"GE0 MAC Loopback Test (1GBPS)",
     (type_t(*)())diag_ge_phy_copper_mac_lpbk_test_1gbps,        GE1,
     (MF_CONTINUOUS | MF_DOGRP | MF_SHOW_ERRCOUNT),
     (type_t(*)())0,                                             0,
     (type_t(*)())0,                                             0},
    {"GE0 External Loopback Test (1GBPS)",
     (type_t(*)())diag_ge_phy_copper_ext_lpbk_test_1gbps,        GE1,
     (MF_CONTINUOUS | MF_DOGRP | MF_SHOW_ERRCOUNT),
     (type_t(*)())0,                                             0,
     (type_t(*)())0,                                             0},
};

#define PLAT_GEPHY0_DIAG_TBL_SIZE (sizeof(plat_gephy0_diag_tbl) / \
                                  sizeof(submenu_xtable_t))

/*
 * Primary & secondary submenu items (filled in from xtable)
 */
static mitem_t plat_gephy0_diag_pri_items[PLAT_GEPHY0_DIAG_TBL_SIZE + MAX_BASE_ITEMS];
static mitem_t plat_gephy0_diag_sec_items[PLAT_GEPHY0_DIAG_TBL_SIZE + MAX_BASE_ITEMS];

menuinfo_t plat_gephy0_diag_menu = {
    "%s Subtest Menu",
    0,                                /* mtparam added by init_empty_menu */
    (PFT)show_endnote,                /* notes missing WICs in combos */
    0,                                /* use generic prompt */
    0,                                /* size (bumped by add_menu_item() */
    plat_gephy0_diag_pri_items,
};
menuinfo_t *plat_gephy0_diag_menup = &plat_gephy0_diag_menu;


/* Map panel port number (Cisco defined) to actual port number (Foxconn HW defined) */
static submenu_xtable_t plat_gephy1_diag_tbl[] = {
    {"GE1 Utilities",
     (type_t(*)())diag_util_menu_88e1112_ge1,                              FALSE,
     0,
     (type_t(*)())0,                                             0,
     (type_t(*)())0,                                             0},
    {"GE1 Register Test",
     (type_t(*)())diag_ge_phy_reg_test,                           GE0,
     (MF_CONTINUOUS | MF_DOGRP | MF_DOALL | MF_SHOW_ERRCOUNT),
     (type_t(*)())0,                                             0,
     (type_t(*)())0,                                             0},
    {"GE1 MAC Loopback Test",
     (type_t(*)())diag_ge_phy_copper_mac_lpbk_test,               GE0,
     (MF_CONTINUOUS | MF_DOGRP | MF_DOALL | MF_SHOW_ERRCOUNT),
     (type_t(*)())0,                                             0,
     (type_t(*)())0,                                             0},
    {"GE1 External Loopback Test",
     (type_t(*)())diag_ge_phy_copper_ext_lpbk_test,              GE0,
     (MF_CONTINUOUS | MF_DOGRP | MF_DOALL | MF_SHOW_ERRCOUNT),
     (type_t(*)())0,                                             0,
     (type_t(*)())0,                                             0},
    {"GE1 SFP External Loopback Test",
     (type_t(*)())diag_ge_phy_sfp_ext_lpbk_test,                 GE0,
     (MF_CONTINUOUS | MF_DOGRP | MF_DOALL | MF_SHOW_ERRCOUNT),
     (type_t(*)())0,                                             0,
     (type_t(*)())0,                                             0},
    {"GE1 Interrupt Test",
     (type_t(*)())diag_ge_phy_intr_test,                 GE0,
     (MF_CONTINUOUS | MF_DOGRP | MF_DOALL | MF_SHOW_ERRCOUNT),
     (type_t(*)())0,                                             0,
     (type_t(*)())0,                                             0},
    {"GE1 MAC Loopback Test (1GBPS)",
     (type_t(*)())diag_ge_phy_copper_mac_lpbk_test_1gbps,        GE0,
     (MF_CONTINUOUS | MF_DOGRP | MF_SHOW_ERRCOUNT),
     (type_t(*)())0,                                             0,
     (type_t(*)())0,                                             0},
    {"GE1 External Loopback Test (1GBPS)",
     (type_t(*)())diag_ge_phy_copper_ext_lpbk_test_1gbps,        GE0,
     (MF_CONTINUOUS | MF_DOGRP | MF_SHOW_ERRCOUNT),
     (type_t(*)())0,                                             0,
     (type_t(*)())0,                                             0},
};

#define PLAT_GEPHY1_DIAG_TBL_SIZE (sizeof(plat_gephy1_diag_tbl) / \
            sizeof(submenu_xtable_t))

/*
 * Primary & secondary submenu items (filled in from xtable)
 */
static mitem_t plat_gephy1_diag_pri_items[PLAT_GEPHY1_DIAG_TBL_SIZE + MAX_BASE_ITEMS];
static mitem_t plat_gephy1_diag_sec_items[PLAT_GEPHY1_DIAG_TBL_SIZE + MAX_BASE_ITEMS];

menuinfo_t plat_gephy1_diag_menu = {
    "%s Subtest Menu",
    0,                                /* mtparam added by init_empty_menu */
    (PFT)show_endnote,                /* notes missing WICs in combos */
    0,                                /* use generic prompt */
    0,                                /* size (bumped by add_menu_item() */
    plat_gephy1_diag_pri_items,
};
menuinfo_t *plat_gephy1_diag_menup = &plat_gephy1_diag_menu;

/* List of GE PHY 0 Utilities */
static submenu_xtable_t gephy0_util_items[] = {
    {"GE PHY register read",        (type_t(*)())diag_util_ge_rd_reg,
     GE1,                0, (type_t(*)())0, 0, (type_t(*)())0, 0},
    {"GE PHY register write",       (type_t(*)())diag_util_ge_wr_reg,
     GE1,                0, (type_t(*)())0, 0, (type_t(*)())0, 0},
    {"CPU register Read",           (type_t(*)())diag_cpu_reg_rd_util, 0, 0,
     (type_t(*)())0, 0, (type_t(*)())0, 0},
    {"CPU register Write",          (type_t(*)())diag_cpu_reg_wr_util, 0, 0,
     (type_t(*)())0, 0, (type_t(*)())0, 0},
    {"Send packet to GE PHY 0",     (type_t(*)())diag_util_ge_send_packet_util, GE1, 0,
     (type_t(*)())0, 0, (type_t(*)())0, 0},
    {"Set PHY 1000BaseT Test mode", (type_t(*)())diag_util_ge_set_test_mode, GE1, 0,
     (type_t(*)())0, 0, (type_t(*)())0, 0},
    {"Set PHY Transmitter Type",    (type_t(*)())diag_util_ge_set_tx_type, GE1, 0,
     (type_t(*)())0, 0, (type_t(*)())0, 0},
    {"VOD Adjustments",             (type_t(*)())diag_util_ge_set_vod,
     GE1,                       0,
     (type_t(*)())0,                0,
     (type_t(*)())0,                0},
    {"GE0 stat LED utils", (type_t(*)())diag_util_ge_led, GE1, 0,
      (type_t(*)())0, 0, (type_t(*)())0, 0},
};

#define GEPHY0_UTIL_SIZE (sizeof(gephy0_util_items) / sizeof(submenu_xtable_t))

/*
 * GE PHY 0 utility items (filled in from xtable)
 */
static mitem_t gephy0_util_pri_items[GEPHY0_UTIL_SIZE + MAX_BASE_ITEMS];
static mitem_t gephy0_util_sec_items[GEPHY0_UTIL_SIZE + MAX_BASE_ITEMS];

/*
 * GE PHY 0 Utility Submenu
 */
menuinfo_t gephy0_util_menu = {
    "%s Menu",
    0,
    (PFT)show_endnote,
    0,
    0,
    gephy0_util_pri_items,
};

menuinfo_t *gephy0_util_menup = &gephy0_util_menu;

/* List of GE PHY 1 Utilities */
static submenu_xtable_t gephy1_util_items[] = {
    {"GE PHY register read", (type_t(*)())diag_util_ge_rd_reg, GE0, 0,
     (type_t(*)())0, 0, (type_t(*)())0, 0},
    {"GE PHY register write", (type_t(*)())diag_util_ge_wr_reg, GE0, 0,
     (type_t(*)())0, 0, (type_t(*)())0, 0},
    {"CPU register Read", (type_t(*)())diag_cpu_reg_rd_util, 0, 0,
     (type_t(*)())0, 0, (type_t(*)())0, 0},
    {"CPU register Write", (type_t(*)())diag_cpu_reg_wr_util, 0, 0,
     (type_t(*)())0, 0, (type_t(*)())0, 0},
    {"Send packet to GE PHY 1", (type_t(*)())diag_util_ge_send_packet_util, GE0, 0,
     (type_t(*)())0, 0, (type_t(*)())0, 0},
    {"Set PHY 1000BaseT Test mode", (type_t(*)())diag_util_ge_set_test_mode, GE0, 0,
     (type_t(*)())0, 0, (type_t(*)())0, 0},
    {"Set PHY Transmitter Type", (type_t(*)())diag_util_ge_set_tx_type, GE0, 0,
     (type_t(*)())0, 0, (type_t(*)())0, 0},
    {"VOD Adjustments",             (type_t(*)())diag_util_ge_set_vod,
     GE0,                       0,
     (type_t(*)())0,                0,
     (type_t(*)())0,                0},
    {"GE1 stat LED utils", (type_t(*)())diag_util_ge_led, GE0,
     0,
      (type_t(*)())0, 0, (type_t(*)())0, 0},
};

#define GEPHY1_UTIL_SIZE (sizeof(gephy1_util_items) / sizeof(submenu_xtable_t))

/*
 * GE PHY 1 utility items (filled in from xtable)
 */
static mitem_t gephy1_util_pri_items[GEPHY1_UTIL_SIZE + MAX_BASE_ITEMS];
static mitem_t gephy1_util_sec_items[GEPHY1_UTIL_SIZE + MAX_BASE_ITEMS];

/*
 * GE PHY 1 Utility Submenu
 */
menuinfo_t gephy1_util_menu = {
    "%s Menu",
    0,
    (PFT)show_endnote,
    0,
    0,
    gephy1_util_pri_items,
};

menuinfo_t *gephy1_util_menup = &gephy1_util_menu;

/*******************************************************************************
 * Function    : diag_util_menu_88e1112_ge0
 * Description : Entry point of GE PHY0 utilities.
 * Inputs      : opt - reserve for future use
 * Outputs     : PASSED / FAILED
 *******************************************************************************
 */
int diag_util_menu_88e1112_ge0 (int opt)
{
    build_primary_submenu(gephy0_util_items, GEPHY0_UTIL_SIZE,
                          "GE PHY 0 Utilities", &gephy0_util_menup);
    build_secondary_submenu(gephy0_util_items, GEPHY0_UTIL_SIZE,
                            gephy0_util_sec_items);

    menu(gephy0_util_menup, gephy0_util_sec_items, '\0' );

    return (PASSED);
}

/*******************************************************************************
 * Function    : diag_util_menu_88e1112_ge1
 * Description : Entry point of GE PHY1 utilities.
 * Inputs      : opt - reserve for future use
 * Outputs     : PASSED / FAILED
 *******************************************************************************
 */
int diag_util_menu_88e1112_ge1 (int opt)
{
    build_primary_submenu(gephy1_util_items, GEPHY1_UTIL_SIZE,
                          "GE PHY 1 Utilities", &gephy1_util_menup);
    build_secondary_submenu(gephy1_util_items, GEPHY1_UTIL_SIZE,
                            gephy1_util_sec_items);

    menu(gephy1_util_menup, gephy1_util_sec_items, '\0' );

    return (PASSED);
}

/******************************************************************************
 * Function   : diag_88e1112_ge0_test
 * Description: Entrance of GE PHY0(88E1112) Diag tests.
 * Inputs     : show_menu - menu option
 * Outputs    : PASSED / FAILED
 *******************************************************************************/
int diag_88e1112_ge0_test (int show_menu)
{
    build_primary_submenu(plat_gephy0_diag_tbl,
                          PLAT_GEPHY0_DIAG_TBL_SIZE,
                          "GE PHY 0", &plat_gephy0_diag_menup);
    build_secondary_submenu(plat_gephy0_diag_tbl,
                            PLAT_GEPHY0_DIAG_TBL_SIZE,
                            plat_gephy0_diag_sec_items);

    if (show_menu) {
        menu(plat_gephy0_diag_menup, plat_gephy0_diag_sec_items, '\0' );

    } else {
        menu_exec_doall_diags(plat_gephy0_diag_menup);
    }
    return (PASSED);
}

/*******************************************************************************
 * Function   : diag_88e1112_ge1_test
 * Description: Entrance of GE PHY1(88E1112) Diag tests.
 * Inputs     : show_menu - menu option
 * Outputs    : PASSED / FAILED
 *******************************************************************************
 */
int diag_88e1112_ge1_test (int show_menu)
{
    build_primary_submenu(plat_gephy1_diag_tbl,
                          PLAT_GEPHY1_DIAG_TBL_SIZE,
                          "GE PHY 1", &plat_gephy1_diag_menup);
    build_secondary_submenu(plat_gephy1_diag_tbl,
                            PLAT_GEPHY1_DIAG_TBL_SIZE,
                            plat_gephy1_diag_sec_items);

    if (show_menu) {
        menu(plat_gephy1_diag_menup, plat_gephy1_diag_sec_items, '\0' );

    } else {
        menu_exec_doall_diags(plat_gephy1_diag_menup);
    }
    return (PASSED);
}

/*******************************************************************************
 * Function   : diag_ge_phy_reg_test
 * Description: Function performs GE PHY(Marvell 88E1112) register test.
 * Inputs     : ge_num - GE port number(GE0/GE1)
 * Outputs    : PASSED / FAILED
 *******************************************************************************
 */
static int diag_ge_phy_reg_test (int ge_num) {
    uchar mb_get_loc[FRU_SIZE] = {0};
    uchar mb_get_pid[FRU_SIZE] = {0};
#ifdef ENHANCE_ERROR_MSG_RDY
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
    cterr_add_component("Marvell Armada 7040", "SGMII", "Marvell 88E1112 GE WAN Phy", "Cu RJ45");

    /* Segment 5: register and memory dump */

    /* Segment 6: Platform Environment initialized here*/
    cterr_add_env_dump((PFV)show_plat_curr_temps);

    /* Segment 7: Top 3 Debugging Steps */
    cterr_add_debug("Check the SGMII bus status to see "
                    "if it is normal among each component.",
                    "If the status is OK, contact vendor "
                    "to verify if SGMII driver is workable.");
#endif

    dev_mrvl_ge_object_t *mrvl_obj; 
    dev_object_t *dev;
    int rc;
    /* enhance error msg: setting test name */
    char test_name[32];
    memset(test_name, 0, sizeof(test_name));
    snprintf(test_name, sizeof(test_name), "GE%d %s", ge_num, "Register Test");
    testname(test_name);
    prpass(testpass, "%s, ", test_name);
    printf("\n");

    /* create 88E1112 device */
    mrvl_obj = (dev_mrvl_ge_object_t *)diag_get_88e11112_obj(ge_num); 
    if (mrvl_obj == NULL) {
        /* enhance error msg: error */
        cterr('f', 0, "%s:%d:GE%d: Fail to create Marvell 88E1112 Object", 
              __FUNCTION__, __LINE__, ge_num);
        prcomplete(testpass, errcount, (char *)0);
        return (FAILED);
    }

    dev = (dev_object_t *)mrvl_obj;

    /* run test */
    rc = mrvl_obj->callin_fvt->ge_register_test(dev); 

    if (rc != PASSED) {
        /* enhance error msg: error */
        cterr('f', 0, "%s:%d:GE%d: Register test fail", 
              __FUNCTION__, __LINE__, ge_num);
        prcomplete(testpass, errcount, (char *)0);
        return (rc);
    }

    /* detach device */
    rc = mrvl_obj->base.dev_object_fvt->dev_detach(dev); /* detach dev obj */
    if (rc != PASSED) {
        /* enhance error msg: error */
        cterr('f', 0, "%s:%d:GE%d: Fail to detach Marvell 88E1112 Object", 
              __FUNCTION__, __LINE__, ge_num);
        prcomplete(testpass, errcount, (char *)0);
        return (rc);
    }
    
    return (PASSED);
}

/*******************************************************************************
 *    
 * Function   : diag_ge_phy_copper_mac_lpbk_test
 * Description: Function to do GE PHY MAC internal loopback test.
 * Inputs     : ge_num - GE PHY number
 * Outputs    : PASSED/FAILED
 *               
 *******************************************************************************
 */
static int diag_ge_phy_copper_mac_lpbk_test (int ge_num)
{
    uchar mb_get_loc[FRU_SIZE] = {0};
    uchar mb_get_pid[FRU_SIZE] = {0};
#ifdef ENHANCE_ERROR_MSG_RDY
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
    cterr_add_component("Marvell Armada 7040", "SGMII", "Marvell 88E1112 GE WAN Phy", "Cu RJ45");

    /* Segment 5: register and memory dump */

    /* Segment 6: Platform Environment initialized here*/
    cterr_add_env_dump((PFV)show_plat_curr_temps);

    /* Segment 7: Top 3 Debugging Steps */
    cterr_add_debug("Observe MDIO register status to check "
                    "if PHY configuration is normal.",
                    "If step a is OK, observe registers of 1112 PHY"
                    " to check if the configuration is normal.",
                    "If step b is OK, we can assume the interface "
                    "between Host SoC and PHY has problems.");
#endif

    dev_mrvl_ge_object_t *mrvl_obj; 
    dev_object_t *dev;
    int rc;
    /* enhance error msg: setting test name */
    char test_name[32];
    memset(test_name, 0, sizeof(test_name));
    snprintf(test_name, sizeof(test_name), "GE%d %s", ge_num, "MAC loopback test");
    testname(test_name);
    prpass(testpass, "%s, ", test_name);
    printf("\n");

    /* create 88E1112 device */
    mrvl_obj = (dev_mrvl_ge_object_t *)diag_get_88e11112_obj(ge_num); 
    if (mrvl_obj == NULL) {
        /* enhance error msg: error */
        cterr('f', 0, "%s:%d:GE%d: Fail to create Marvell 88E1112 Object", 
              __FUNCTION__, __LINE__, ge_num);
        prcomplete(testpass, errcount, (char *)0);
        return (FAILED);
    }

    dev = (dev_object_t *)mrvl_obj;

    /* run test */
    rc = mrvl_obj->callin_fvt->ge_phy_mac_lpbk_test(dev); 

    if (rc != PASSED) {
        /* enhance error msg: error */
        cterr('f', 0, "%s:%d:GE%d: MAC loopback test fail", 
              __FUNCTION__, __LINE__, ge_num);
        prcomplete(testpass, errcount, (char *)0);
        return (rc);
    }

    /* detach device */
    rc = mrvl_obj->base.dev_object_fvt->dev_detach(dev); /* detach dev obj */
    if (rc != PASSED) {
        /* enhance error msg: error */
        cterr('f', 0, "%s:%d:GE%d: Fail to detach Marvell 88E1112 Object", 
              __FUNCTION__, __LINE__, ge_num);
        prcomplete(testpass, errcount, (char *)0);
        return (rc);
    }
    
    return (PASSED);
}

/*******************************************************************************
 *    
 * Function   : diag_ge_phy_copper_ext_lpbk_test
 * Description: Function to do GE PHY external loopback test.
 * Inputs     : ge_num - GE PHY number
 * Outputs    : PASSED/FAILED
 *               
 *******************************************************************************
 */
static int diag_ge_phy_copper_ext_lpbk_test (int ge_num)
{
    uchar mb_get_loc[FRU_SIZE] = {0};
    uchar mb_get_pid[FRU_SIZE] = {0};
#ifdef ENHANCE_ERROR_MSG_RDY
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
    cterr_add_component("Marvell Armada 7040", "SGMII", "Marvell 88E1112 GE WAN Phy", "Cu RJ45");

    /* Segment 5: register and memory dump */

    /* Segment 6: Platform Environment initialized here*/
    cterr_add_env_dump((PFV)show_plat_curr_temps);

    /* Segment 7: Top 3 Debugging Steps */
    cterr_add_debug("Observe MDIO register status to check "
                    "if PHY configuration is normal.",
                    "If step a is OK, observe registers of 1112 PHY"
                    " to check if the configuration is normal.",
                    "If step b is OK, we can assume the interface "
                    "between Host SoC and PHY has problems.");
#endif

    dev_mrvl_ge_object_t *mrvl_obj; 
    dev_object_t *dev;
    int rc;
    /* enhance error msg: setting test name */
    char test_name[32];
    memset(test_name, 0, sizeof(test_name));
    snprintf(test_name, sizeof(test_name), "GE%d %s", ge_num, "External loopback test");
    testname(test_name);
    prpass(testpass, "%s, ", test_name);
    printf("\n");

    /* create 88E1112 device */
    mrvl_obj = (dev_mrvl_ge_object_t *)diag_get_88e11112_obj(ge_num); 
    if (mrvl_obj == NULL) {
        /* enhance error msg: error */
        cterr('f', 0, "%s:%d:GE%d: Fail to create Marvell 88E1112 Object", 
              __FUNCTION__, __LINE__, ge_num);
        prcomplete(testpass, errcount, (char *)0);
        return (FAILED);
    }

    dev = (dev_object_t *)mrvl_obj;

    /* run test */
    rc = mrvl_obj->callin_fvt->ge_ext_lpbk_test(dev); 

    if (rc != PASSED) {
        /* enhance error msg: error */
        cterr('f', 0, "%s:%d:GE%d: External loopback test fail", 
              __FUNCTION__, __LINE__, ge_num);
        prcomplete(testpass, errcount, (char *)0);
        return (rc);
    }

    /* detach device */
    rc = mrvl_obj->base.dev_object_fvt->dev_detach(dev); /* detach dev obj */
    if (rc != PASSED) {
        /* enhance error msg: error */
        cterr('f', 0, "%s:%d:GE%d: Fail to detach Marvell 88E1112 Object", 
              __FUNCTION__, __LINE__, ge_num);
        prcomplete(testpass, errcount, (char *)0);
        return (rc);
    }
    
    return (PASSED);
}

/*******************************************************************************
 *    
 * Function   : diag_ge_phy_sfp_ext_lpbk_test
 * Description: Function to do GE PHY SFP port external loopback test.
 * Inputs     : ge_num - GE PHY number
 * Outputs    : PASSED/FAILED
 *               
 *******************************************************************************
 */
static int diag_ge_phy_sfp_ext_lpbk_test (int ge_num)
{
    uchar mb_get_loc[FRU_SIZE] = {0};
    uchar mb_get_pid[FRU_SIZE] = {0};
#ifdef ENHANCE_ERROR_MSG_RDY
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
    cterr_add_component("Marvell Armada 7040", "SGMII", "Marvell 88E1112 GE WAN Phy", "Cu RJ45");

    /* Segment 5: register and memory dump */

    /* Segment 6: Platform Environment initialized here*/
    cterr_add_env_dump((PFV)show_plat_curr_temps);

    /* Segment 7: Top 3 Debugging Steps */
    cterr_add_debug("Observe MDIO register status to check "
                    "if PHY configuration is normal.",
                    "If step a is OK, observe registers of 1112 PHY"
                    " to check if the configuration is normal.",
                    "If step b is OK, we can assume the interface "
                    "between Host SoC and PHY has problems.");
#endif

    dev_mrvl_ge_object_t *mrvl_obj; 
    dev_object_t *dev;
    int rc;
    /* enhance error msg: setting test name */
    char test_name[32];
    current_sfp_select = ge_num;
    memset(test_name, 0, sizeof(test_name));
    snprintf(test_name, sizeof(test_name), "GE%d %s", ge_num, "SFP External loopback test");
    testname(test_name);
    prpass(testpass, "%s, ", test_name);
    printf("\n");


    /* Config GE0/GE1 PHY fiber */
    if (diag_ge_phy_config_gephy_fiber(ge_num) != PASSED) {
        cterr('f', 0, "Failed to config GE PHY Fiber.");
    }

    /* create 88E1112 device */
    mrvl_obj = (dev_mrvl_ge_object_t *)diag_get_88e11112_obj(ge_num); 
    if (mrvl_obj == NULL) {
        /* enhance error msg: error */
        cterr('f', 0, "%s:%d:GE%d: Fail to create Marvell 88E1112 Object", 
              __FUNCTION__, __LINE__, ge_num);
        prcomplete(testpass, errcount, (char *)0);
        return (FAILED);
    }

    dev = (dev_object_t *)mrvl_obj;

    /* run test */
    rc = mrvl_obj->callin_fvt->ge_phy_sfp_ext_lpbk_test(dev); 

    if (rc != PASSED) {
        /* enhance error msg: error */
        cterr('f', 0, "%s:%d:GE%d: SFP External loopback test fail", 
              __FUNCTION__, __LINE__, ge_num);
        prcomplete(testpass, errcount, (char *)0);
        return (rc);
    }

    /* detach device */
    rc = mrvl_obj->base.dev_object_fvt->dev_detach(dev); /* detach dev obj */
    if (rc != PASSED) {
        /* enhance error msg: error */
        cterr('f', 0, "%s:%d:GE%d: Fail to detach Marvell 88E1112 Object", 
              __FUNCTION__, __LINE__, ge_num);
        prcomplete(testpass, errcount, (char *)0);
        return (rc);
    }
    
    return (PASSED);
}

/*******************************************************************************
 * Function   : diag_ge_phy_intr_test
 * Description: Function to do GE PHY interrupt test.
 * Inputs     : ge_num - GE PHY number
 * Outputs    : PASSED/FAILED
 * Comment    : EIPR - External Interrupt Pending Register(0x1128)              
 *******************************************************************************
 */
static int diag_ge_phy_intr_test (int ge_num)
{
    uchar mb_get_loc[FRU_SIZE] = {0};
    uchar mb_get_pid[FRU_SIZE] = {0};
#ifdef ENHANCE_ERROR_MSG_RDY
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
    cterr_add_component("Marvell Armada 7040", "SGMII", "Marvell 88E1112 GE WAN Phy", "Cu RJ45");

    /* Segment 5: register and memory dump */

    /* Segment 6: Platform Environment initialized here*/
    cterr_add_env_dump((PFV)show_plat_curr_temps);

    /* Segment 7: Top 3 Debugging Steps */
    cterr_add_debug("Observe MDIO register status to check "
                    "if PHY configuration is normal.",
                    "If step a is OK, observe registers of 1112 PHY"
                    " to check if the configuration is normal.",
                    "If step b is OK, we can assume the interface "
                    "between Host SoC and PHY has problems.");
#endif
    dev_mrvl_ge_object_t *mrvl_obj; 
    dev_object_t *dev;
    int rc;
    /* enhance error msg: setting test name */
    char test_name[32];
    memset(test_name, 0, sizeof(test_name));
    snprintf(test_name, sizeof(test_name), "GE%d %s", ge_num, "Interrupt test");
    testname(test_name);
    prpass(testpass, "%s, ", test_name);
    printf("\n");

    /* create 88E1112 device */
    mrvl_obj = (dev_mrvl_ge_object_t *)diag_get_88e11112_obj(ge_num); 
    if (mrvl_obj == NULL) {
        /* enhance error msg: error */
        cterr('f', 0, "%s:%d:GE%d: Fail to create Marvell 88E1112 Object", 
              __FUNCTION__, __LINE__, ge_num);
        prcomplete(testpass, errcount, (char *)0);
        return (FAILED);
    }

    dev = (dev_object_t *)mrvl_obj;

    /* run test */
    rc = mrvl_obj->callin_fvt->ge_phy_intr_test(dev); 

    if (rc != PASSED) {
        /* enhance error msg: error */
        cterr('f', 0, "%s:%d:GE%d: Interrupt test fail", 
              __FUNCTION__, __LINE__, ge_num);
        prcomplete(testpass, errcount, (char *)0);
        return (rc);
    }

    /* detach device */
    rc = mrvl_obj->base.dev_object_fvt->dev_detach(dev); /* detach dev obj */
    if (rc != PASSED) {
        /* enhance error msg: error */
        cterr('f', 0, "%s:%d:GE%d: Fail to detach Marvell 88E1112 Object", 
              __FUNCTION__, __LINE__, ge_num);
        prcomplete(testpass, errcount, (char *)0);
        return (rc);
    }
    
    return (PASSED);
}

/*******************************************************************************
 *    
 * Function   : diag_ge_phy_copper_ext_lpbk_test_1gbps
 * Description: Function to do GE PHY external loopback test. (For fixed speed 1GBPS)
 * Inputs     : ge_num - GE PHY number
 * Outputs    : PASSED/FAILED
 *               
 *******************************************************************************
 */
static int diag_ge_phy_copper_ext_lpbk_test_1gbps (int ge_num)
{
    uchar mb_get_loc[FRU_SIZE] = {0};
    uchar mb_get_pid[FRU_SIZE] = {0};
#ifdef ENHANCE_ERROR_MSG_RDY
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
    cterr_add_component("Marvell Armada 7040", "SGMII", "Marvell 88E1112 GE WAN Phy", "Cu RJ45");

    /* Segment 5: register and memory dump */

    /* Segment 6: Platform Environment initialized here*/
    cterr_add_env_dump((PFV)show_plat_curr_temps);

    /* Segment 7: Top 3 Debugging Steps */
    cterr_add_debug("Observe MDIO register status to check "
                    "if PHY configuration is normal.",
                    "If step a is OK, observe registers of 1112 PHY"
                    " to check if the configuration is normal.",
                    "If step b is OK, we can assume the interface "
                    "between Host SoC and PHY has problems.");
#endif

    dev_mrvl_ge_object_t *mrvl_obj; 
    dev_object_t *dev;
    int rc;
    /* enhance error msg: setting test name */
    char test_name[32];
    memset(test_name, 0, sizeof(test_name));
    snprintf(test_name, sizeof(test_name), "GE%d %s", ge_num, "External loopback test (1GBPS)");
    testname(test_name);
    prpass(testpass, "%s, ", test_name);
    printf("\n");

    /* create 88E1112 device */
    mrvl_obj = (dev_mrvl_ge_object_t *)diag_get_88e11112_obj(ge_num); 
    if (mrvl_obj == NULL) {
        /* enhance error msg: error */
        cterr('f', 0, "%s:%d:GE%d: Fail to create Marvell 88E1112 Object", 
              __FUNCTION__, __LINE__, ge_num);
        prcomplete(testpass, errcount, (char *)0);
        return (FAILED);
    }

    dev = (dev_object_t *)mrvl_obj;

    /* run test */
    rc = mrvl_obj->callin_fvt->ge_ext_lpbk_test_1gbps(dev); 

    if (rc != PASSED) {
        /* enhance error msg: error */
        cterr('f', 0, "%s:%d:GE%d: External loopback test fail", 
              __FUNCTION__, __LINE__, ge_num);
        prcomplete(testpass, errcount, (char *)0);
        return (rc);
    }

    /* detach device */
    rc = mrvl_obj->base.dev_object_fvt->dev_detach(dev); /* detach dev obj */
    if (rc != PASSED) {
        /* enhance error msg: error */
        cterr('f', 0, "%s:%d:GE%d: Fail to detach Marvell 88E1112 Object", 
              __FUNCTION__, __LINE__, ge_num);
        prcomplete(testpass, errcount, (char *)0);
        return (rc);
    }
    
    return (PASSED);
}

/*******************************************************************************
 *    
 * Function   : diag_ge_phy_copper_mac_lpbk_test_1gbps
 * Description: Function to do GE PHY MAC internal loopback test.(For fixed speed 1GBPS)
 * Inputs     : ge_num - GE PHY number
 * Outputs    : PASSED/FAILED
 *               
 *******************************************************************************
 */
static int diag_ge_phy_copper_mac_lpbk_test_1gbps (int ge_num)
{
    uchar mb_get_loc[FRU_SIZE] = {0};
    uchar mb_get_pid[FRU_SIZE] = {0};
#ifdef ENHANCE_ERROR_MSG_RDY
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
    cterr_add_component("Marvell Armada 7040", "SGMII", "Marvell 88E1112 GE WAN Phy", "Cu RJ45");

    /* Segment 5: register and memory dump */

    /* Segment 6: Platform Environment initialized here*/
    cterr_add_env_dump((PFV)show_plat_curr_temps);

    /* Segment 7: Top 3 Debugging Steps */
    cterr_add_debug("Observe MDIO register status to check "
                    "if PHY configuration is normal.",
                    "If step a is OK, observe registers of 1112 PHY"
                    " to check if the configuration is normal.",
                    "If step b is OK, we can assume the interface "
                    "between Host SoC and PHY has problems.");
#endif

    dev_mrvl_ge_object_t *mrvl_obj; 
    dev_object_t *dev;
    int rc;
    /* enhance error msg: setting test name */
    char test_name[32];
    memset(test_name, 0, sizeof(test_name));
    snprintf(test_name, sizeof(test_name), "GE%d %s", ge_num, "MAC loopback test (1GBPS)");
    testname(test_name);
    prpass(testpass, "%s, ", test_name);
    printf("\n");

    /* create 88E1112 device */
    mrvl_obj = (dev_mrvl_ge_object_t *)diag_get_88e11112_obj(ge_num); 
    if (mrvl_obj == NULL) {
        /* enhance error msg: error */
        cterr('f', 0, "%s:%d:GE%d: Fail to create Marvell 88E1112 Object", 
              __FUNCTION__, __LINE__, ge_num);
        prcomplete(testpass, errcount, (char *)0);
        return (FAILED);
    }

    dev = (dev_object_t *)mrvl_obj;

    /* run test */
    rc = mrvl_obj->callin_fvt->ge_phy_mac_lpbk_test_1gbps(dev); 

    if (rc != PASSED) {
        /* enhance error msg: error */
        cterr('f', 0, "%s:%d:GE%d: MAC loopback test fail", 
              __FUNCTION__, __LINE__, ge_num);
        prcomplete(testpass, errcount, (char *)0);
        return (rc);
    }

    /* detach device */
    rc = mrvl_obj->base.dev_object_fvt->dev_detach(dev); /* detach dev obj */
    if (rc != PASSED) {
        /* enhance error msg: error */
        cterr('f', 0, "%s:%d:GE%d: Fail to detach Marvell 88E1112 Object", 
              __FUNCTION__, __LINE__, ge_num);
        prcomplete(testpass, errcount, (char *)0);
        return (rc);
    }
    
    return (PASSED);
}


/*-------------------------------------------------
 * $Log: diag_ge_phy_test.c,v $
 * Revision 1.2  2021/09/24 01:21:06  harrchan
 * Collapse Elixir-branch to Main Trunk.
 *
 * Revision 1.1.2.10  2021/05/31 10:48:11  illiu
 * Modify the parameter value of item: Send packet to GE PHY 0/1
 *
 * Revision 1.1.2.9  2021/05/05 07:13:09  illiu
 * Add port mapping comments for GE PHY 0/1 test
 *
 * Revision 1.1.2.8  2021/04/23 02:38:04  illiu
 * Replace sprintf with snprintf
 *
 * Revision 1.1.2.7  2021/01/07 06:23:20  illiu
 * Add GE MAC Loopback Test (1GBPS) item which is for fixed speed 1GBPS
 *
 * Revision 1.1.2.6  2020/12/21 09:12:20  illiu
 * 1. Add GE PHY External Loopback Test (1GBPS) item which is for fixed speed 1GBPS
 * 2. Add 1680 PHY Internal/External Loopback Tests (1GBPS) item which is for fixed speed 1GBPS
 *
 * Revision 1.1.2.5  2020/12/08 01:50:13  harrchan
 * Correct the gephy test port because Foxconn GEPHY definition was reverse with Cisco I/O
 *
 * Revision 1.1.2.4  2020/11/05 06:40:37  harrchan
 * 1.According elixir fpga spec to change the definition of SFP present and SFP transmitter disable bit.
 * 2.Add GE PHY1 SFP external loopback test
 *
 * Revision 1.1.2.3  2020/09/16 02:25:35  harrchan
 * Support GE1 SFP test
 *
 * Revision 1.1.2.2  2020/09/14 05:49:44  harrchan
 * Remove DSL and GSHDSL relevant part
 *
 * Revision 1.1.2.1  2020/09/09 09:08:06  illiu
 * First version which has been ported with Dreamliner and Marvel CPSS
 *
 * Revision 1.2  2019/01/10 06:36:22  wilbhuan
 * The beginning of Betelgeuse application code.
 *
 *-------------------------------------------------
 */
