/* $Id: diag_esw_test.c,v 1.2 2021/09/24 01:21:06 harrchan Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/elixir/diag_esw_test.c,v $
 *------------------------------------------------------------------
 * 
 * diag_esw_test.c
 *
 * Copyright (c) 2018 - 2019 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include "error.h"
#include "common.h"
#include "types.h"
#include "proto.h"
#include "queryflags.h"
#include "menu.h"
#include "ethernet.h"
#include "common_utils.h"
#include "nvmonvars.h"
#include "diag_smi_lib.h"
#include "diag_moka_fpga_lib.h"
#include "diag_cpu_lib.h"
#include "diag_cpu_util.h"
#include "platform_cookie.h"
#include "diag_temp_sensor_util.h"
#include "diag_esw_lib.h"
#include "diag_esw_util.h"
#include "diag_esw_test.h"
#include "diag_moka_fpga_util.h"
#include "diag_enhance_err_msg_lib.h"
#include "diag_ge_phy_util.h"
#include "diag_eth_pkt_txrx.h"
#include "linux_pciutils.h"

#include <unistd.h>
#include "madApi.h"
#include "madHwCntl.h"
#include "dev_88e1680.h"
#include "dev_98dxc25x.h"


/* Function Declaration */
int diag_ac5_test(int);
int diag_esw_xcat5_all_register_test(void);
int diag_esw_phy_register_test(void);
int diag_esw_internal_lpbk_test(int);
int diag_cpu_esw_mac_lpbk_test(int);
int diag_esw_int_lpbk_test_sub(int);
int diag_esw_external_lpbk_test(int);
int diag_esw_ext_lpbk_test_sub(int);
int diag_esw_xcat5_intr_test(void);
int diag_esw_phy_intr_test(void);
int diag_esw_internal_lpbk_test_1gbps(int);
int diag_esw_external_lpbk_test_1gbps(int);
int diag_esw_pcie_speed_width_check(void) ;

static int diag_ac5_utils(int);
static int diag_esw_phy_clear_phy_counter(void);
static int diag_esw_phy_port_internal_lpbk_test(int, int, int);
static int diag_do_cpu_esw_mac_lpbk_test(int);
static int diag_esw_phy_print_phy_counter(void);
static int diag_esw_phy_port_external_lpbk_test(int, int, int);
static int diag_esw_xcat5_print_mac_counter(void);
static int diag_esw_xcat5_config_port_pve(int, int, int);
static int diag_esw_xcat5_unconfig_port_pve(int, int, int);

/* ESW supported speed table */
static int esw_speed_tbl[] = {SPD_10MBPS, SPD_100MBPS, SPD_1000MBPS};


/* Including AC5 menu */
static submenu_xtable_t esw_submenu_tbl[] = {
    {"ESW Utilities",
     (type_t(*)())diag_ac5_utils, FALSE, 0,
     (type_t(*)())0, 0, (type_t(*)())diag_ac5_utils, TRUE},

    {"xCat5 All Register Test",
     (PFT) diag_esw_xcat5_all_register_test, FALSE,
     MF_CONTINUOUS | MF_DOGRP | MF_DOALL | MF_SHOW_ERRCOUNT,
     (type_t(*)())0, 0, (PFT) 0, 0},

    {"PHY Register Test",
     (PFT) diag_esw_phy_register_test, FALSE,
     MF_CONTINUOUS | MF_DOGRP | MF_DOALL | MF_SHOW_ERRCOUNT,
     (type_t(*)())0, 0, (PFT) 0, 0},

    {"xCat5 Interrupt Test",
     (PFT) diag_esw_xcat5_intr_test, FALSE,
     MF_CONTINUOUS | MF_DOGRP | MF_DOALL | MF_SHOW_ERRCOUNT,
     (type_t(*)())0, 0, (PFT) 0, 0},

    {"PHY Interrupt Test",
     (PFT) diag_esw_phy_intr_test, FALSE,
     MF_CONTINUOUS | MF_DOGRP | MF_DOALL | MF_SHOW_ERRCOUNT,
     (type_t(*)())0, 0, (PFT) 0, 0},

    {"CPU to AC5 MAC Loopback Test",
     (PFT) diag_cpu_esw_mac_lpbk_test, TRUE,
     MF_CONTINUOUS | MF_DOGRP | MF_DOALL | MF_SHOW_ERRCOUNT,
     (type_t(*)())0, 0, (PFT) 0, 0},

    {"PHY Internal Loopback Test",
     (PFT) diag_esw_internal_lpbk_test, TRUE,
     MF_CONTINUOUS | MF_DOGRP | MF_DOALL | MF_SHOW_ERRCOUNT,
     (type_t(*)())0, 0,  (type_t(*)())diag_esw_internal_lpbk_test, FALSE},

    {"PHY External Loopback Test",
     (PFT) diag_esw_external_lpbk_test, TRUE,
     MF_CONTINUOUS | MF_DOGRP | MF_DOALL | MF_SHOW_ERRCOUNT,
     (type_t(*)())0, 0,  (type_t(*)())diag_esw_external_lpbk_test, FALSE},

    {"PHY Internal Loopback Test (1GBPS)",
     (PFT) diag_esw_internal_lpbk_test_1gbps, TRUE,
     MF_CONTINUOUS | MF_DOGRP | MF_SHOW_ERRCOUNT,
     (type_t(*)())0, 0,  (type_t(*)())diag_esw_internal_lpbk_test_1gbps, FALSE},

    {"PHY External Loopback Test (1GBPS)",
     (PFT) diag_esw_external_lpbk_test_1gbps, TRUE,
     MF_CONTINUOUS | MF_DOGRP | MF_SHOW_ERRCOUNT,
     (type_t(*)())0, 0,  (type_t(*)())diag_esw_external_lpbk_test_1gbps, FALSE},

    {"xCat5 PCIE GEN3 speed and width check",
     (PFT) diag_esw_pcie_speed_width_check, TRUE,
     MF_CONTINUOUS | MF_DOGRP | MF_DOALL | MF_SHOW_ERRCOUNT,
     (type_t(*)())0, 0,  (type_t(*)())0, 0},

};

#define ESW_SUBMENU_TBL_SZ (sizeof(esw_submenu_tbl) / sizeof(submenu_xtable_t))

/*
 * Primary & secondary submenu items (filled in from xtable)
 */
static mitem_t esw_pri_items[ESW_SUBMENU_TBL_SZ + MAX_BASE_ITEMS];
static mitem_t esw_sec_items[ESW_SUBMENU_TBL_SZ + MAX_BASE_ITEMS];

menuinfo_t esw_subtest_menu = {
    "%s Subtest Menu",
    0,                                /* mtparam added by init_empty_menu */
    (PFT)show_endnote,                /* notes missing WICs in combos */
    0,                                /* use generic prompt */
    0,                                /* size (bumped by add_menu_item() */
    esw_pri_items,
};
menuinfo_t *esw_subtest_menup = &esw_subtest_menu;


/* List of AC5 Utilities */
static submenu_xtable_t ac5_util_items[] = {

    {"ESW PCI Config Read Utility",
     (PFT) diag_esw_pcie_config_rd_util, FALSE, 0,
     (type_t(*)())0, 0, (PFT) 0, 0},

    {"ESW PCI Config Write Utility",
     (PFT) diag_esw_pcie_config_wr_util, FALSE, 0,
     (type_t(*)())0, 0, (PFT) 0, 0},

    {"ESW xCat5 Internal Register Read Utility",            
     (PFT) diag_esw_xcat5_internal_reg_rd_util, FALSE, 0,
     (type_t(*)())0, 0, (PFT) 0, 0},

    {"ESW xCat5 Internal Register Write Utility",
     (PFT) diag_esw_xcat5_internal_reg_wr_util, FALSE, 0,
     (type_t(*)())0, 0, (PFT) 0, 0},

    {"ESW xCat5 PP Register Read Utility",
     (PFT) diag_esw_xcat5_reg_rd_util, FALSE, 0,
     (type_t(*)())0, 0, (PFT) 0, 0},

    {"ESW xCat5 PP Register Write Utility",
     (PFT) diag_esw_xcat5_reg_wr_util, FALSE, 0,
     (type_t(*)())0, 0, (PFT) 0, 0},

    {"ESW PHY Register Read Utility",
     (PFT) diag_esw_phy_reg_rd_util, FALSE, 0,
     (type_t(*)())0, 0, (PFT) 0, 0},

    {"ESW PHY Register Write Utility",
     (PFT) diag_esw_phy_reg_wr_util, FALSE, 0,
     (type_t(*)())0, 0, (PFT) 0, 0},

    {"Print All PHY Counter Utility",
     (PFT) diag_esw_phy_print_phy_counter, FALSE, 0,
     (type_t(*)())0, 0, (PFT) 0, 0},

    {"Clear All PHY Counter Utility",
     (PFT) diag_esw_phy_clear_phy_counter, FALSE, 0,
     (type_t(*)())0, 0, (PFT) 0, 0},

    {"Print xCat5 Mac Counter Utility",
     (PFT) diag_esw_xcat5_print_mac_counter, FALSE, 0,
     (type_t(*)())0, 0, (PFT) 0, 0},

    {"PHY Test Mode Utility",
     (PFT) diag_esw_phy_test_mode_util, FALSE, 0,
     (type_t(*)())0, 0, (PFT) 0, 0},

    {"ESW Reset Default Utility",
     (PFT) diag_reset_esw_to_default, FALSE, 0,
     (type_t(*)())0, 0, (PFT) 0, 0},

    {"Send packet to ESW",     
     (type_t(*)())diag_util_ge_send_packet_util, GEESW, 0,
     (type_t(*)())0, 0, (type_t(*)())0, 0},
};

#define AC5_UTIL_SIZE (sizeof(ac5_util_items) / sizeof(submenu_xtable_t))

/*
 * AC5 utility items (filled in from xtable)
 */
static mitem_t ac5_util_pri_items[AC5_UTIL_SIZE + MAX_BASE_ITEMS];
static mitem_t ac5_util_sec_items[AC5_UTIL_SIZE + MAX_BASE_ITEMS];

/*
 * GE PHY Utility Submenu
 */
menuinfo_t ac5_util_menu = {
    "%s Menu",
    0,
    (PFT)show_endnote,
    0,
    0,
    ac5_util_pri_items,
};

menuinfo_t *ac5_util_menup = &ac5_util_menu;

/*
 * Sub Menu used for "Ethernet switch test -> Ethernet switch submenu test -> Internal Loopback Test"
 */

/* The port parameter was modified:  Map panel port number (Cisco defined) 
 * to actual port number (Foxconn HW defined) */
submenu_xtable_t esw_internal_lpbk_test_tbl[] = {

    {"Internal Loopback Test for ESW Port 0",
     (PFT) diag_esw_int_lpbk_test_sub, FRONT_PANEL_PORT_ZERO,
     MF_CONTINUOUS | MF_DOGRP | MF_DOALL,
     (type_t(*)())0, 0, (PFT) 0, 0},

    {"Internal Loopback Test for ESW Port 1",
     (PFT) diag_esw_int_lpbk_test_sub, FRONT_PANEL_PORT_ONE,
     MF_CONTINUOUS | MF_DOGRP | MF_DOALL,
     (type_t(*)())0, 0, (PFT) 0, 0},

    {"Internal Loopback Test for ESW Port 2",
     (PFT) diag_esw_int_lpbk_test_sub, FRONT_PANEL_PORT_TWO,
     MF_CONTINUOUS | MF_DOGRP | MF_DOALL,
     (type_t(*)())0, 0, (PFT) 0, 0},

    {"Internal Loopback Test for ESW Port 3",
     (PFT) diag_esw_int_lpbk_test_sub, FRONT_PANEL_PORT_THREE,
     MF_CONTINUOUS | MF_DOGRP | MF_DOALL,
     (type_t(*)())0, 0, (PFT) 0, 0},

    {"Internal Loopback Test for ESW Port 4",
     (PFT) diag_esw_int_lpbk_test_sub, FRONT_PANEL_PORT_FOUR,
     MF_CONTINUOUS | MF_DOGRP | MF_DOALL,
     (type_t(*)())0, 0, (PFT) 0, 0},

    {"Internal Loopback Test for ESW Port 5",
     (PFT) diag_esw_int_lpbk_test_sub, FRONT_PANEL_PORT_FIVE,
     MF_CONTINUOUS | MF_DOGRP | MF_DOALL,
     (type_t(*)())0, 0, (PFT) 0, 0},

    {"Internal Loopback Test for ESW Port 6",
     (PFT) diag_esw_int_lpbk_test_sub, FRONT_PANEL_PORT_SIX,
     MF_CONTINUOUS | MF_DOGRP | MF_DOALL,
     (type_t(*)())0, 0, (PFT) 0, 0},

    {"Internal Loopback Test for ESW Port 7",
     (PFT) diag_esw_int_lpbk_test_sub, FRONT_PANEL_PORT_SEVEN,
     MF_CONTINUOUS | MF_DOGRP | MF_DOALL,
     (type_t(*)())0, 0, (PFT) 0, 0},

};


#define ESW_INTERNAL_LPBK_TEST_TBL_SIZE (sizeof(esw_internal_lpbk_test_tbl) / sizeof(submenu_xtable_t))

static mitem_t esw_internal_lpbk_test_primary_items[ESW_INTERNAL_LPBK_TEST_TBL_SIZE + MAX_BASE_ITEMS];
static mitem_t esw_internal_lpbk_test_secondary_items[ESW_INTERNAL_LPBK_TEST_TBL_SIZE + MAX_BASE_ITEMS];

menuinfo_t esw_internal_lpbk_test_menu = {
    "%s Menu",
    0,
    (PFT)show_endnote,
    0,
    0,
    esw_internal_lpbk_test_primary_items,
};
menuinfo_t *esw_internal_lpbk_test_menup = &esw_internal_lpbk_test_menu;


/*
 * Sub Menu used for "Ethernet switch test -> Ethernet switch submenu test -> External Loopback Test"
 */

/* The port parameter was modified:  Map panel port number (Cisco defined) to actual port number (Foxconn HW defined) */
submenu_xtable_t esw_external_lpbk_test_tbl[] = {

    {"External Loopback Test for ESW Port 0",
     (PFT) diag_esw_ext_lpbk_test_sub, FRONT_PANEL_PORT_ZERO,
     MF_CONTINUOUS | MF_DOGRP | MF_DOALL,
     (type_t(*)())0, 0, (PFT) 0, 0},

    {"External Loopback Test for ESW Port 1",
     (PFT) diag_esw_ext_lpbk_test_sub, FRONT_PANEL_PORT_ONE,
     MF_CONTINUOUS | MF_DOGRP | MF_DOALL,
     (type_t(*)())0, 0, (PFT) 0, 0},

    {"External Loopback Test for ESW Port 2",
     (PFT) diag_esw_ext_lpbk_test_sub, FRONT_PANEL_PORT_TWO,
     MF_CONTINUOUS | MF_DOGRP | MF_DOALL,
     (type_t(*)())0, 0, (PFT) 0, 0},

    {"External Loopback Test for ESW Port 3",
     (PFT) diag_esw_ext_lpbk_test_sub, FRONT_PANEL_PORT_THREE,
     MF_CONTINUOUS | MF_DOGRP | MF_DOALL,
     (type_t(*)())0, 0, (PFT) 0, 0},

    {"External Loopback Test for ESW Port 4",
     (PFT) diag_esw_ext_lpbk_test_sub, FRONT_PANEL_PORT_FOUR,
     MF_CONTINUOUS | MF_DOGRP | MF_DOALL,
     (type_t(*)())0, 0, (PFT) 0, 0},

    {"External Loopback Test for ESW Port 5",
     (PFT) diag_esw_ext_lpbk_test_sub, FRONT_PANEL_PORT_FIVE,
     MF_CONTINUOUS | MF_DOGRP | MF_DOALL,
     (type_t(*)())0, 0, (PFT) 0, 0},

    {"External Loopback Test for ESW Port 6",
     (PFT) diag_esw_ext_lpbk_test_sub, FRONT_PANEL_PORT_SIX,
     MF_CONTINUOUS | MF_DOGRP | MF_DOALL,
     (type_t(*)())0, 0, (PFT) 0, 0},

    {"External Loopback Test for ESW Port 7",
     (PFT) diag_esw_ext_lpbk_test_sub, FRONT_PANEL_PORT_SEVEN,
     MF_CONTINUOUS | MF_DOGRP | MF_DOALL,
     (type_t(*)())0, 0, (PFT) 0, 0},

};

#define ESW_EXTERNAL_LPBK_TEST_TBL_SIZE (sizeof(esw_external_lpbk_test_tbl) / sizeof(submenu_xtable_t))

static mitem_t esw_external_lpbk_test_primary_items[ESW_EXTERNAL_LPBK_TEST_TBL_SIZE + MAX_BASE_ITEMS];
static mitem_t esw_external_lpbk_test_secondary_items[ESW_EXTERNAL_LPBK_TEST_TBL_SIZE + MAX_BASE_ITEMS];

menuinfo_t esw_external_lpbk_test_menu = {
    "%s Menu",
    0,
    (PFT)show_endnote,
    0,
    0,
    esw_external_lpbk_test_primary_items,
};
menuinfo_t *esw_external_lpbk_test_menup = &esw_external_lpbk_test_menu;

/*
 * Sub Menu used for "Ethernet switch test -> Ethernet switch submenu test -> Internal Loopback Test (1GBPS)"
 */

/* The port parameter was modified:  Map panel port number (Cisco defined) to actual port number (Foxconn HW defined) */
submenu_xtable_t esw_internal_lpbk_test_1gbps_tbl[] = {

    {"Internal Loopback Test(1GBPS) for ESW Port 0",
     (PFT) diag_esw_int_lpbk_test_sub, FRONT_PANEL_PORT_ZERO_1GBPS,
     MF_CONTINUOUS | MF_DOGRP | MF_DOALL,
     (type_t(*)())0, 0, (PFT) 0, 0},

    {"Internal Loopback Test(1GBPS) for ESW Port 1",
     (PFT) diag_esw_int_lpbk_test_sub, FRONT_PANEL_PORT_ONE_1GBPS,
     MF_CONTINUOUS | MF_DOGRP | MF_DOALL,
     (type_t(*)())0, 0, (PFT) 0, 0},

    {"Internal Loopback Test(1GBPS) for ESW Port 2",
     (PFT) diag_esw_int_lpbk_test_sub, FRONT_PANEL_PORT_TWO_1GBPS,
     MF_CONTINUOUS | MF_DOGRP | MF_DOALL,
     (type_t(*)())0, 0, (PFT) 0, 0},

    {"Internal Loopback Test(1GBPS) for ESW Port 3",
     (PFT) diag_esw_int_lpbk_test_sub, FRONT_PANEL_PORT_THREE_1GBPS,
     MF_CONTINUOUS | MF_DOGRP | MF_DOALL,
     (type_t(*)())0, 0, (PFT) 0, 0},

    {"Internal Loopback Test(1GBPS) for ESW Port 4",
     (PFT) diag_esw_int_lpbk_test_sub, FRONT_PANEL_PORT_FOUR_1GBPS,
     MF_CONTINUOUS | MF_DOGRP | MF_DOALL,
     (type_t(*)())0, 0, (PFT) 0, 0},

    {"Internal Loopback Test(1GBPS) for ESW Port 5",
     (PFT) diag_esw_int_lpbk_test_sub, FRONT_PANEL_PORT_FIVE_1GBPS,
     MF_CONTINUOUS | MF_DOGRP | MF_DOALL,
     (type_t(*)())0, 0, (PFT) 0, 0},

    {"Internal Loopback Test(1GBPS) for ESW Port 6",
     (PFT) diag_esw_int_lpbk_test_sub, FRONT_PANEL_PORT_SIX_1GBPS,
     MF_CONTINUOUS | MF_DOGRP | MF_DOALL,
     (type_t(*)())0, 0, (PFT) 0, 0},

    {"Internal Loopback Test(1GBPS) for ESW Port 7",
     (PFT) diag_esw_int_lpbk_test_sub, FRONT_PANEL_PORT_SEVEN_1GBPS,
     MF_CONTINUOUS | MF_DOGRP | MF_DOALL,
     (type_t(*)())0, 0, (PFT) 0, 0},

};


#define ESW_INTERNAL_LPBK_TEST_1GBPS_TBL_SIZE (sizeof(esw_internal_lpbk_test_1gbps_tbl) / sizeof(submenu_xtable_t))

static mitem_t esw_internal_lpbk_test_1gbps_primary_items[ESW_INTERNAL_LPBK_TEST_1GBPS_TBL_SIZE + MAX_BASE_ITEMS];
static mitem_t esw_internal_lpbk_test_1gbps_secondary_items[ESW_INTERNAL_LPBK_TEST_1GBPS_TBL_SIZE + MAX_BASE_ITEMS];

menuinfo_t esw_internal_lpbk_test_1gbps_menu = {
    "%s Menu",
    0,
    (PFT)show_endnote,
    0,
    0,
    esw_internal_lpbk_test_1gbps_primary_items,
};
menuinfo_t *esw_internal_lpbk_test_1gbps_menup = &esw_internal_lpbk_test_1gbps_menu;


/*
 * Sub Menu used for "Ethernet switch test -> Ethernet switch submenu test -> External Loopback Test (1GBPS)"
 */

/* The port parameter was modified:  Map panel port number (Cisco defined) to actual port number (Foxconn HW defined) */
submenu_xtable_t esw_external_lpbk_test_1gbps_tbl[] = {

    {"External Loopback Test(1GBPS) for ESW Port 0",
     (PFT) diag_esw_ext_lpbk_test_sub, FRONT_PANEL_PORT_ZERO_1GBPS,
     MF_CONTINUOUS | MF_DOGRP | MF_DOALL,
     (type_t(*)())0, 0, (PFT) 0, 0},

    {"External Loopback Test(1GBPS) for ESW Port 1",
     (PFT) diag_esw_ext_lpbk_test_sub, FRONT_PANEL_PORT_ONE_1GBPS,
     MF_CONTINUOUS | MF_DOGRP | MF_DOALL,
     (type_t(*)())0, 0, (PFT) 0, 0},

    {"External Loopback Test(1GBPS) for ESW Port 2",
     (PFT) diag_esw_ext_lpbk_test_sub, FRONT_PANEL_PORT_TWO_1GBPS,
     MF_CONTINUOUS | MF_DOGRP | MF_DOALL,
     (type_t(*)())0, 0, (PFT) 0, 0},

    {"External Loopback Test(1GBPS) for ESW Port 3",
     (PFT) diag_esw_ext_lpbk_test_sub, FRONT_PANEL_PORT_THREE_1GBPS,
     MF_CONTINUOUS | MF_DOGRP | MF_DOALL,
     (type_t(*)())0, 0, (PFT) 0, 0},

    {"External Loopback Test(1GBPS) for ESW Port 4",
     (PFT) diag_esw_ext_lpbk_test_sub, FRONT_PANEL_PORT_FOUR_1GBPS,
     MF_CONTINUOUS | MF_DOGRP | MF_DOALL,
     (type_t(*)())0, 0, (PFT) 0, 0},

    {"External Loopback Test(1GBPS) for ESW Port 5",
     (PFT) diag_esw_ext_lpbk_test_sub, FRONT_PANEL_PORT_FIVE_1GBPS,
     MF_CONTINUOUS | MF_DOGRP | MF_DOALL,
     (type_t(*)())0, 0, (PFT) 0, 0},

    {"External Loopback Test(1GBPS) for ESW Port 6",
     (PFT) diag_esw_ext_lpbk_test_sub, FRONT_PANEL_PORT_SIX_1GBPS,
     MF_CONTINUOUS | MF_DOGRP | MF_DOALL,
     (type_t(*)())0, 0, (PFT) 0, 0},

    {"External Loopback Test(1GBPS) for ESW Port 7",
     (PFT) diag_esw_ext_lpbk_test_sub, FRONT_PANEL_PORT_SEVEN_1GBPS,
     MF_CONTINUOUS | MF_DOGRP | MF_DOALL,
     (type_t(*)())0, 0, (PFT) 0, 0},

};

#define ESW_EXTERNAL_LPBK_TEST_1GBPS_TBL_SIZE (sizeof(esw_external_lpbk_test_1gbps_tbl) / sizeof(submenu_xtable_t))

static mitem_t esw_external_lpbk_test_1gbps_primary_items[ESW_EXTERNAL_LPBK_TEST_1GBPS_TBL_SIZE + MAX_BASE_ITEMS];
static mitem_t esw_external_lpbk_test_1gbps_secondary_items[ESW_EXTERNAL_LPBK_TEST_1GBPS_TBL_SIZE + MAX_BASE_ITEMS];

menuinfo_t esw_external_lpbk_test_1gbps_menu = {
    "%s Menu",
    0,
    (PFT)show_endnote,
    0,
    0,
    esw_external_lpbk_test_1gbps_primary_items,
};
menuinfo_t *esw_external_lpbk_test_1gbps_menup = &esw_external_lpbk_test_1gbps_menu;



/******************************************************************************
 * Function: diag_ac5_test
 *
 * Description: Entrance of Ethernet Switch Diag menu.
 * Inputs      : show_menu - show menu or do all tests 
 * Outputs     : PASSED / FAILED
 ******************************************************************************/
int diag_ac5_test (int show_menu)
{

    char *tname = "Ethernet Switch Test";

    testname(tname);

    build_primary_submenu(esw_submenu_tbl,
                          ESW_SUBMENU_TBL_SZ,
                          "Ethernet Switch", &esw_subtest_menup);
    build_secondary_submenu(esw_submenu_tbl,
                            ESW_SUBMENU_TBL_SZ,
                            esw_sec_items);

    if (show_menu) {
        menu(esw_subtest_menup, esw_sec_items, '\0' );
    } else {
        menu_exec_doall_diags(esw_subtest_menup);
    }


    return (PASSED);
}

/*******************************************************************************
 * Function    : diag_ac5_utils
 *
 * Description : Entrance of Ethernet Switch Utility menu.
 * Inputs      : opt - reserve for future use
 * Outputs     : PASSED / FAILED
 *******************************************************************************/
static int diag_ac5_utils (int opt)
{
    build_primary_submenu(ac5_util_items, AC5_UTIL_SIZE,
                          "Ethernet Switch Utilities", &ac5_util_menup);
    build_secondary_submenu(ac5_util_items, AC5_UTIL_SIZE,
                            ac5_util_sec_items);

    menu(ac5_util_menup, ac5_util_sec_items, '\0' );
    return (PASSED);
}

/******************************************************************************
 *
 * Function   :	diag_esw_xcat5_all_register_test
 *
 * Description: perform xCAT5 register test.
 * Inputs     :	None
 * Outputs    : PASSED/FAILED
 *
 ******************************************************************************
 */
int diag_esw_xcat5_all_register_test (void)
{

    int rc = 0;

    dev_98dxc25x_object_t *esw_98dxc25x_obj_p = NULL;
    dev_object_t *dev;

    char tname[32]; 

    memset(tname, 0, sizeof(tname));
    snprintf(tname, sizeof(tname), "xCat5 All Register");
    testname(tname);
    prpass(testpass, "%s, ", tname);


    /* Create 98dxc25x device driver */
    esw_98dxc25x_obj_p = (dev_98dxc25x_object_t *)diag_get_esw_98dxc25x_obj(); 
    if (esw_98dxc25x_obj_p == NULL) {
        cterr('f', 0, "%s: Creating object failed", __func__);
        return (FAILED);
    }
    dev = (dev_object_t *)esw_98dxc25x_obj_p;

    /* Call 98dxc25x xcat5 all register test function */
    rc = esw_98dxc25x_obj_p->callin_fvt->esw_xcat5_all_reg_test(dev, esw_98dxc25x_obj_p->cpss_dev); 
    if (rc != PASSED) {
        cterr('f', 0, "Failed to test xCat5 all register test");
        goto _exit;
    }

    prcomplete(testpass, errcount, (char *)0);

    return (PASSED);

 _exit:

    prcomplete(testpass, errcount, (char *)0);

    return (FAILED);

}

/******************************************************************************
 *
 * Function   :	diag_esw_phy_register_test
 * Description: perform esw phy register test.
 *              
 * Inputs     :	None
 *              
 * Outputs    : PASSED/FAILED
 *
 ******************************************************************************
 */
int diag_esw_phy_register_test (void)
{

    int ix;
    MAD_DEV *mad_dev;

    dev_88e1680_object_t *phy_88e1680_obj_p = NULL;
    dev_object_t *dev;

    char tname[32]; 

    memset(tname, 0, sizeof(tname));
    snprintf(tname, sizeof(tname) ,"PHY Register");
    testname(tname);
    prpass(testpass, "%s, ", tname);

    /* Create 88e1680 device driver */
    phy_88e1680_obj_p = (dev_88e1680_object_t *)diag_get_phy_88e1680_obj(); 
    if (phy_88e1680_obj_p == NULL) {
        cterr('f', 0, "%s: Creating object failed", __func__);
        return (FAILED);
    }
    dev = (dev_object_t *)phy_88e1680_obj_p;

    for (ix = 0; ix < ELIXIR_ESW_PORT_NUM; ix++) {
        mad_dev = &phy_mad_88e1680;
        /* Call 88e1680 PHY register test function */
        if (phy_88e1680_obj_p->callin_fvt->phy_reg_test(dev, mad_dev, ix) != PASSED) {
            cterr('f',0,"Failed to test phy register for port %d", ix);
            goto _exit;
        }
    }

    prcomplete(testpass, errcount, (char *)0);

    return (PASSED);

 _exit:

    prcomplete(testpass, errcount, (char *)0);

    return (FAILED);
    
}

/*******************************************************************************
 *
 * Function    : diag_esw_internal_lpbk_test
 * Description : Function to show ESW internal loopback test submenu.
 * Inputs      : opt - reserved for future use.
 * Outputs     : PASSED / FAILED
 *
 *******************************************************************************
 */
int diag_esw_internal_lpbk_test (int opt)
{
    build_primary_submenu(esw_internal_lpbk_test_tbl, ESW_INTERNAL_LPBK_TEST_TBL_SIZE,
                          "ESW Internal Loopback test", &esw_internal_lpbk_test_menup);
    build_secondary_submenu(esw_internal_lpbk_test_tbl, ESW_INTERNAL_LPBK_TEST_TBL_SIZE,
                            esw_internal_lpbk_test_secondary_items);

    if (opt) {
        do_all_menu_items(&esw_internal_lpbk_test_menu);
    } else {
        menu(esw_internal_lpbk_test_menup, esw_internal_lpbk_test_secondary_items, '\0');
    }

    return (PASSED);
}

/******************************************************************************
 *
 * Function   :	diag_cpu_esw_mac_lpbk_test
 * Description: perform mac internal loopback test.
 *              host -> XCAT5 -> host
 * Inputs     :	None
 * Outputs    : PASSED/FAILED
 *
 ******************************************************************************
 */
int diag_cpu_esw_mac_lpbk_test (int reserve)
{

    int port_num = XCAT5_TO_CPU_PORT;

    dev_98dxc25x_object_t *esw_98dxc25x_obj_p = NULL;
    dev_object_t *dev;

    char tname[100]; 

    memset(tname, 0, sizeof(tname));
    snprintf(tname, sizeof(tname), "CPU to Switch Port Internal loopback");
    testname(tname);
    prpass(testpass, "%s, ", tname);

    /* Create 98dxc25x device driver */
    esw_98dxc25x_obj_p = (dev_98dxc25x_object_t *)diag_get_esw_98dxc25x_obj();
    if (esw_98dxc25x_obj_p == NULL) {
        cterr('f', 0, "%s: Creating object failed", __func__);
        return (FAILED);
    }
    dev = (dev_object_t *)esw_98dxc25x_obj_p;

    prpass(testpass, "Testing switch port%d", GE_XCAT5_PORT);


    /* Elixir's GE_XCAT5_PORT value is 26 (MAC <--> Switch)*/
    if (diag_do_cpu_esw_mac_lpbk_test(GE_XCAT5_PORT) != PASSED) {
        cterr('f', 0, "Failed MAC internal loopback test for port");
        /* Call 98dxc25x print software counter */ 
        if (esw_98dxc25x_obj_p->callin_fvt->esw_print_mac_counter(dev, esw_98dxc25x_obj_p->cpss_dev,
                                                                 port_num) != PASSED) {
            cterr('f',0,"Failed esw_print_mac_counter()");
            goto _exit;
        }
        goto _exit;
    }

    if ((NVRAM)->diagflag & D_VERBOSE) {
        /* Call 98dxc25x print software counter */ 
        if (esw_98dxc25x_obj_p->callin_fvt->esw_print_mac_counter(dev, esw_98dxc25x_obj_p->cpss_dev,
                                                                 port_num) != PASSED) {
            cterr('f',0,"Failed esw_print_mac_counter()");
	        goto _exit;
        }
    }

    prcomplete(testpass, errcount, (char *)0);

    return (PASSED);

 _exit:

    prcomplete(testpass, errcount, (char *)0);

    return (FAILED);
    
}

/******************************************************************************
 *
 * Function   :	diag_esw_int_lpbk_test_sub
 * Description: perform PHY internal loopback test.
 *              host->GE1->XCAT5->phy->XCAT5->->GE1->host
 * Inputs     :	target_port - which phy port
 * Outputs    : PASSED/FAILED
 *
 ******************************************************************************
 */
int diag_esw_int_lpbk_test_sub (int target_port)
{

    int port_num = ELIXIR_ESW_PORT_NUM;
    int spd_mode;
    int total_spd = 0,  spd_ctr = 0,  test_spd = 0;

    dev_98dxc25x_object_t *esw_98dxc25x_obj_p = NULL;
    dev_object_t *dev;

    char tname[100]; 

    memset(tname, 0, sizeof(tname));
    snprintf(tname, sizeof(tname), "CPU to Switch PHY Internal loopback");
    testname(tname);
    prpass(testpass, "%s, ", tname);

    /* Create 98dxc25x device driver */
    esw_98dxc25x_obj_p = (dev_98dxc25x_object_t *)diag_get_esw_98dxc25x_obj();
    if (esw_98dxc25x_obj_p == NULL) {
        cterr('f', 0, "%s: Creating object failed", __func__);
        return (FAILED);
    }
    dev = (dev_object_t *)esw_98dxc25x_obj_p;

    total_spd = sizeof(esw_speed_tbl) / sizeof(int);
    spd_mode = ESW_SPD_10; 

    /* For Fixed speed 1GBPS test item */
    if (target_port >= SPEED_1GBPS_TAG) {
        target_port = target_port - SPEED_1GBPS_TAG;
        spd_mode = ESW_SPD_1000;
    }

    for (spd_ctr = spd_mode; spd_ctr < total_spd; spd_ctr++) {
        test_spd = esw_speed_tbl[spd_ctr];
        prpass(testpass, "Testing switch port%d in %dmbps ",
               target_port, test_spd);

        /* Call 88e1680 clear PHY counter */
        if (diag_esw_phy_clear_phy_counter() != PASSED) {
            cterr('f',0,"Failed diag_esw_phy_clear_phy_counter()");
            goto _exit;
        }

        /* Elixir's GE_XCAT5_PORT value is 26 (MAC <--> Switch)*/
        if (diag_esw_phy_port_internal_lpbk_test(target_port, GE_XCAT5_PORT, test_spd) != PASSED) {
            cterr('f', 0, "Failed PHY internal loopback test for port %d", target_port);

            /* Call 98dxc25x print software counter */ 
            if (esw_98dxc25x_obj_p->callin_fvt->esw_print_mac_counter(dev, esw_98dxc25x_obj_p->cpss_dev,
                                                                     port_num) != PASSED) {
                cterr('f',0,"Failed esw_print_mac_counter()");
                goto _exit;
            }

            /* Call 88e1680 print PHY counter */
            if (diag_esw_phy_print_phy_counter() != PASSED) {
                cterr('f',0,"Failed print_phy_counter()");
                goto _exit;
            }
            goto _exit;
        }
    
        if ((NVRAM)->diagflag & D_VERBOSE) {
            /* Call 98dxc25x print software counter */ 
            if (esw_98dxc25x_obj_p->callin_fvt->esw_print_mac_counter(dev, esw_98dxc25x_obj_p->cpss_dev,
                                                                     port_num) != PASSED) {
                cterr('f',0,"Failed esw_print_mac_counter()");
                goto _exit;
            }

            /* Call 88e1680 print PHY counter */
            if (diag_esw_phy_print_phy_counter()!= PASSED) {
                cterr('f',0,"Failed print_phy_counter()");
                goto _exit;
            }
       }
    }

    prcomplete(testpass, errcount, (char *)0);

    return (PASSED);

 _exit:

    prcomplete(testpass, errcount, (char *)0);

    return (FAILED);

}

/******************************************************************************
 *
 * Function   :	diag_esw_phy_clear_phy_counter
 * Description: perform PHY clear counter.
 * Inputs     :	NULL
 * Outputs    : PASSED/FAILED
 *
 ******************************************************************************
 */
static int diag_esw_phy_clear_phy_counter (void)
{

    int ix;
    MAD_DEV * mad_dev;

    dev_88e1680_object_t *phy_88e1680_obj_p = NULL;
    dev_object_t *dev;

    /* Create 88e1680 device driver */
    phy_88e1680_obj_p = (dev_88e1680_object_t *)diag_get_phy_88e1680_obj();
    if (phy_88e1680_obj_p == NULL) {
        cterr('f', 0, "%s: Creating object failed", __func__);
        return (FAILED);
    }
    dev = (dev_object_t *)phy_88e1680_obj_p;

    for (ix = 0; ix < ELIXIR_ESW_PORT_NUM; ix++) {
        mad_dev = &phy_mad_88e1680;
        /* Call 88e1680 PHY clear counter function */
        phy_88e1680_obj_p->callin_fvt->clear_phy_counter(dev, mad_dev, ix);
    }

    return (PASSED);

}

/******************************************************************************
 *
 * Function   :	diag_esw_phy_port_internal_lpbk_test
 * Description: perform PHY internal loopback test.
 *              host->GE->XCAT5->phy->XCAT5->->GE->host
 * Inputs     :	port_num
 *              bp_port - backplane GE port, can GE_XCAT5_PORT
 * Outputs    : PASSED/FAILED
 *
 ******************************************************************************
 */
static int diag_esw_phy_port_internal_lpbk_test (int port_num, int bp_port, int speed)
{

    int rc = 0;
    MAD_DEV * mad_dev;
    MAD_SPEED_MODE target_speed = 0;

    dev_88e1680_object_t *phy_88e1680_obj_p = NULL;
    dev_object_t *phy_dev;

    dev_98dxc25x_object_t *esw_98dxc25x_obj_p = NULL;
    dev_object_t *esw_dev;

    /* Create 88e1680 device driver */
    phy_88e1680_obj_p = (dev_88e1680_object_t *)diag_get_phy_88e1680_obj(); 
    if (phy_88e1680_obj_p == NULL) {
        cterr('f', 0, "%s: Creating object failed", __func__);
        return (FAILED);
    }
    phy_dev = (dev_object_t *)phy_88e1680_obj_p;

    /* Create 98dxc25x device driver */
    esw_98dxc25x_obj_p = (dev_98dxc25x_object_t *)diag_get_esw_98dxc25x_obj();
    if (esw_98dxc25x_obj_p == NULL) {
        cterr('f', 0, "%s: Creating object failed", __func__);
        return (FAILED);
    }
    esw_dev = (dev_object_t *)esw_98dxc25x_obj_p;


    /* Config port speed */
    if (diag_config_port_speed(esw_98dxc25x_obj_p->cpss_dev, port_num, speed) != PASSED) {
        cterr('f',0,"Failed to config MAC port speed.");
        goto _exit;
    }

    /* Call 98dxc25x config port pve function */
    if (diag_esw_xcat5_config_port_pve(esw_98dxc25x_obj_p->cpss_dev, bp_port, port_num) != PASSED) {
        cterr('f',0,"Failed diag_esw_xcat5_config_port_pve()");
        goto _exit;
    }

    if (speed == SPD_1000MBPS) {
        target_speed = MAD_SPEED_1000M;
    } else if (speed == SPD_100MBPS) {
        target_speed = MAD_SPEED_100M;
    } else if (speed == SPD_10MBPS) {
        target_speed = MAD_SPEED_10M;
    }

    /* Call 88e1680 start mac loopback function */
    mad_dev = &phy_mad_88e1680;
    if (phy_88e1680_obj_p->callin_fvt->start_mac_lpbk(phy_dev, mad_dev, 
                                                      port_num, target_speed)!= PASSED) {
        cterr('f',0,"Failed to enable PHY loopback for port %d", port_num);
        goto _exit;
    }

    /* Do SGMII loopback test */
    rc = eth_pkt_txrx(LPBK_ETH0, LPBK_PKG, FALSE);
    if (rc == FAILED) {
        phy_88e1680_obj_p->callin_fvt->dump_phy_reg(phy_dev, mad_dev, port_num);
        goto _exit;
    }

   /* Call 98dxc25x unconfig port pve function */
    if (diag_esw_xcat5_unconfig_port_pve(esw_98dxc25x_obj_p->cpss_dev, bp_port, port_num) != PASSED) {
        cterr('f',0,"Failed diag_esw_xcat5_unconfig_port_pve()");
        goto _exit;
    }


    /* Call 88e1680 phy restore function */
    if (phy_88e1680_obj_p->callin_fvt->phy_config(phy_dev, mad_dev, port_num)!= PASSED) {
        cterr('f',0,"Failed to restore PHY for port %d", port_num);
        goto _exit;
    }

    return (PASSED);
	
 _exit:

    return (FAILED);

}

/******************************************************************************
 *
 * Function   :	diag_do_cpu_esw_mac_lpbk_test
 * Description: perform PHY internal loopback test.
 *              host->GE->XCAT5->->GE->host
 * Inputs     :	port_num
 *              port - GE port, can GE_XCAT5_PORT
 * Outputs    : PASSED/FAILED
 *
 ******************************************************************************
 */
static int diag_do_cpu_esw_mac_lpbk_test (int port)
{

    int rc = PASSED;

    dev_98dxc25x_object_t *esw_98dxc25x_obj_p = NULL;
    dev_object_t *dev;

    /* Create 98dxc25x device driver */
    esw_98dxc25x_obj_p = (dev_98dxc25x_object_t *)diag_get_esw_98dxc25x_obj();
    if (esw_98dxc25x_obj_p == NULL) {
        cterr('f', 0, "%s: Creating object failed", __func__);
        return (FAILED);
    }
    dev = (dev_object_t *)esw_98dxc25x_obj_p;


    /* Call 98dxc25x config port pve function */
    if (diag_esw_xcat5_config_port_pve(esw_98dxc25x_obj_p->cpss_dev, port, port) != PASSED) {
        cterr('f',0,"Failed diag_esw_xcat5_config_port_pve()");
        return (FAILED);
    }

    rc = eth_pkt_txrx(LPBK_ETH0, LPBK_PKG, FALSE);
    if (rc == FAILED) {
        cterr('f',0,"Failed MAC loopback test");
        goto _exit;
    }

   /* Call 98dxc25x unconfig port pve function */
    if (diag_esw_xcat5_unconfig_port_pve(esw_98dxc25x_obj_p->cpss_dev, port, port) != PASSED) {
        cterr('f',0,"Failed diag_esw_xcat5_unconfig_port_pve()");
        return (FAILED);
    }

    return (PASSED);

 _exit:

   /* Call 98dxc25x unconfig port pve function */
    if (diag_esw_xcat5_unconfig_port_pve(esw_98dxc25x_obj_p->cpss_dev, port, port) != PASSED) {
        cterr('f',0,"Failed diag_esw_xcat5_unconfig_port_pve()");
        return (FAILED);
    }

    return (FAILED);

}

/******************************************************************************
 *
 * Function   :	diag_esw_phy_print_phy_counter
 * Description: perform PHY print counter.
 * Inputs     :	NULL
 * Outputs    : PASSED/FAILED
 *
 ******************************************************************************
 */
static int diag_esw_phy_print_phy_counter (void)
{

    int ix;
    MAD_DEV * mad_dev;

    dev_88e1680_object_t *phy_88e1680_obj_p = NULL;
    dev_object_t *dev;

    /* Create 88e1680 device driver */
    phy_88e1680_obj_p = (dev_88e1680_object_t *)diag_get_phy_88e1680_obj();
    if (phy_88e1680_obj_p == NULL) {
        cterr('f', 0, "%s: Creating object failed", __func__);
        return (FAILED);
    }
    dev = (dev_object_t *)phy_88e1680_obj_p;

    for (ix = 0; ix < ELIXIR_ESW_PORT_NUM; ix++) {
        mad_dev = &phy_mad_88e1680;
        /* Call 88e1680 PHY print counter function */
        phy_88e1680_obj_p->callin_fvt->print_phy_counter(dev, mad_dev, ix);
    }

    return (PASSED);
    
}

/*******************************************************************************
 *
 * Function    : diag_esw_external_lpbk_test
 * Description : Function to show ESW external loopback test submenu.
 * Inputs      : opt - reserved for future use.
 * Outputs     : PASSED / FAILED
 *
 *******************************************************************************
 */
int diag_esw_external_lpbk_test (int opt)
{
    char *tname = "ESW External Loopback test";

    if (check_menu_flag(D_EXT_LOOPBACK)) {
        prpass(testpass, "\n External loopback flag is off, skip '%s' \
            external loopback test. ", tname);
        prcomplete(testpass, errcount, (char *)0);
        return (PASSED);
    }

    build_primary_submenu(esw_external_lpbk_test_tbl, ESW_EXTERNAL_LPBK_TEST_TBL_SIZE,
                          "ESW External Loopback test", &esw_external_lpbk_test_menup);
    build_secondary_submenu(esw_external_lpbk_test_tbl, ESW_EXTERNAL_LPBK_TEST_TBL_SIZE,
                            esw_external_lpbk_test_secondary_items);

    if (opt) {
        do_all_menu_items(&esw_external_lpbk_test_menu);
    }
    else {
        menu(esw_external_lpbk_test_menup, esw_external_lpbk_test_secondary_items, '\0');
    }

    return (PASSED);
}

/******************************************************************************
 *
 * Function   :	diag_esw_ext_lpbk_test_sub
 * Description: perform external loopback test.
 *              host->GE1->XCAT5->phy->loopback connecter->phy->XCAT5->->GE1->host
 * Inputs     :	None
 * Outputs    : PASSED/FAILED
 *
 ******************************************************************************
 */
int diag_esw_ext_lpbk_test_sub (int target_port)
{

    int port_num = ELIXIR_ESW_PORT_NUM;
    int spd_mode;
    int total_spd = 0,  spd_ctr = 0,  test_spd = 0;

    dev_98dxc25x_object_t *esw_98dxc25x_obj_p = NULL;
    dev_object_t *dev;

    char tname[100]; 

    memset(tname, 0, sizeof(tname));
    snprintf(tname, sizeof(tname), "CPU to Switch PHY External loopback");
    testname(tname);
    prpass(testpass, "%s, ", tname);

    if (!(diagflag_xram & D_MIN_TEST_TIME)) {
        spd_mode = ESW_SPD_1000;
    } else {
        spd_mode = ESW_SPD_10;
    }

    /* Create 98dxc25x device driver */
    esw_98dxc25x_obj_p = (dev_98dxc25x_object_t *)diag_get_esw_98dxc25x_obj();
    if (esw_98dxc25x_obj_p == NULL) {
        cterr('f', 0, "%s: Creating object failed", __func__);
        return (FAILED);
    }
    dev = (dev_object_t *)esw_98dxc25x_obj_p;

    total_spd = sizeof(esw_speed_tbl) / sizeof(int);

    /* For Fixed speed 1GBPS test item */
    if (target_port >= SPEED_1GBPS_TAG) {
        target_port = target_port - SPEED_1GBPS_TAG;
        spd_mode = ESW_SPD_1000;
    }

    for (spd_ctr = spd_mode; spd_ctr < total_spd; spd_ctr++) {
        test_spd = esw_speed_tbl[spd_ctr];
        prpass(testpass, "Testing switch port%d in %dmbps ",
                         target_port, test_spd);

        /* clear phy counters */
        if (diag_esw_phy_clear_phy_counter() != PASSED) {
            cterr('f',0,"Failed diag_esw_phy_clear_phy_counter()");
            goto _exit;
        }

        if (diag_esw_phy_port_external_lpbk_test(target_port, GE_XCAT5_PORT, test_spd) != PASSED) {
            if (esw_98dxc25x_obj_p->callin_fvt->esw_print_mac_counter(dev, esw_98dxc25x_obj_p->cpss_dev,
                                                                     port_num) != PASSED) {
                cterr('f',0,"Failed esw_print_mac_counter()");
                goto _exit;
            }
            if (diag_esw_phy_print_phy_counter() != PASSED) {
                cterr('f',0,"Failed print_phy_counter()");
                goto _exit;
            }
            cterr('f', 0, "Failed external loopback test for port %d", target_port);
            goto _exit;
        }

        if ((NVRAM)->diagflag & D_VERBOSE) {
            if (esw_98dxc25x_obj_p->callin_fvt->esw_print_mac_counter(dev, esw_98dxc25x_obj_p->cpss_dev,
                                                                     port_num) != PASSED) {
                cterr('f',0,"Failed esw_print_mac_counter()");
                goto _exit;
            }
            if (diag_esw_phy_print_phy_counter() != PASSED) {
                cterr('f',0,"Failed print_phy_counter()");
                goto _exit;
            }
        }
    }

    prcomplete(testpass, errcount, (char *)0);

    return (PASSED);

 _exit:

    prcomplete(testpass, errcount, (char *)0);

    return (FAILED);
    
}

/******************************************************************************
 *
 * Function   :	diag_esw_phy_port_external_lpbk_test
 * Description: perform external loopback test.
 *              host->GE->XCAT5->phy->loopback connecter->phy->XCAT5->->GE->host
 * Inputs     :	port_num
 *              bp_port - backplane GE port, can GE_XCAT5_PORT
 * Outputs    : PASSED/FAILED
 *
 ******************************************************************************
 */
static int diag_esw_phy_port_external_lpbk_test (int port_num, int bp_port, int speed)
{

    int rc = 0;

    MAD_DEV * mad_dev;
    MAD_SPEED_MODE target_speed = 0;

    dev_88e1680_object_t *phy_88e1680_obj_p = NULL;
    dev_object_t *phy_dev;

    dev_98dxc25x_object_t *esw_98dxc25x_obj_p = NULL;
    dev_object_t *esw_dev;

    /* Create 88e1680 device driver */
    phy_88e1680_obj_p = (dev_88e1680_object_t *)diag_get_phy_88e1680_obj();
    if (phy_88e1680_obj_p == NULL) {
        cterr('f', 0, "%s: Creating object failed", __func__);
        return (FAILED);
    }
    phy_dev = (dev_object_t *)phy_88e1680_obj_p;

    /* Create 98dxc25x device driver */
    esw_98dxc25x_obj_p = (dev_98dxc25x_object_t *)diag_get_esw_98dxc25x_obj();
    if (esw_98dxc25x_obj_p == NULL) {
        cterr('f', 0, "%s: Creating object failed", __func__);
        return (FAILED);
    }
    esw_dev = (dev_object_t *)esw_98dxc25x_obj_p;

    /* Config port speed */
    if (diag_config_port_speed(esw_98dxc25x_obj_p->cpss_dev, port_num, speed) != PASSED) {
        cterr('f',0,"Failed to config MAC port speed.");
        goto _exit;
    }

    /* Call 98dxc25x config port pve function */
    if (diag_esw_xcat5_config_port_pve(esw_98dxc25x_obj_p->cpss_dev, bp_port, port_num) != PASSED) {
        cterr('f',0,"Failed diag_esw_xcat5_config_port_pve()");
        goto _exit;
    }

    if (speed == SPD_1000MBPS) {
        target_speed = MAD_SPEED_1000M;
    } else if (speed == SPD_100MBPS) {
        target_speed = MAD_SPEED_100M;
    } else if (speed == SPD_10MBPS) {
        target_speed = MAD_SPEED_10M;
    }

    /* Call 88e1680 start external loopback function */
    mad_dev = &phy_mad_88e1680;
    if (phy_88e1680_obj_p->callin_fvt->start_ext_lpbk(phy_dev, mad_dev,
                                                      port_num, target_speed)!= PASSED) {
        cterr('f',0,"Failed to enable external loopback for port %d", port_num);
        goto _exit;
    }

    /* Do SGMII loopback test. */
    rc = eth_pkt_txrx(LPBK_ETH0, LPBK_PKG, FALSE);
    if (rc == FAILED) {
        phy_88e1680_obj_p->callin_fvt->dump_phy_reg(phy_dev, mad_dev, port_num);
        goto _exit;
    }

   /* Call 98dxc25x unconfig port pve function */
    if (diag_esw_xcat5_unconfig_port_pve(esw_98dxc25x_obj_p->cpss_dev, bp_port, port_num) != PASSED) {
        cterr('f',0,"Failed diag_esw_xcat5_unconfig_port_pve()");
        goto _exit;
    }

    /* Call 88e1680 phy restore function */
    if (phy_88e1680_obj_p->callin_fvt->phy_config(phy_dev, mad_dev, port_num)!= PASSED) {
        cterr('f',0,"Failed to restore PHY for port %d", port_num);
        goto _exit;
    }

    return (PASSED);
	
 _exit:

    return (FAILED);
    
}

/******************************************************************************
 *
 * Function   :	diag_esw_xcat5_intr_test
 * Description: perform xCAT5 interrupt test.
 *              
 * Inputs     :	None
 *              
 * Outputs    : PASSED/FAILED
 *
 ******************************************************************************
 */
int diag_esw_xcat5_intr_test (void)
{

    int ix;
    int check_intr = FALSE;

    dev_98dxc25x_object_t *esw_98dxc25x_obj_p = NULL;
    dev_object_t *dev;

    char tname[32]; 

    memset(tname, 0, sizeof(tname));
    snprintf(tname, sizeof(tname), "xCat5 Interrupt");
    testname(tname);
    prpass(testpass, "%s, ", tname);


    /* Create 98dxc25x device driver */
    esw_98dxc25x_obj_p = (dev_98dxc25x_object_t *)diag_get_esw_98dxc25x_obj();
    if (esw_98dxc25x_obj_p == NULL) {
        cterr('f', 0, "%s: Creating object failed", __func__);
        return (FAILED);
    }
    dev = (dev_object_t *)esw_98dxc25x_obj_p;

    /* Call 98dxc25x xcat5 generate interrupt function */
    if (esw_98dxc25x_obj_p->callin_fvt->esw_xcat5_gen_int(dev, esw_98dxc25x_obj_p->cpss_dev) != PASSED) {
        cterr('f',0,"Failed to generate xCat5 interrupt");
        goto _exit;
    }

    /* Check the interrupt pin is asserted
     * As Hardware Team's suggestion, while platform running in high/low temp.(EDVT), 
     * the electric reaction of Interrupt Pin might not instantaneous,
     * hence, checking the status of Interrupt Pin by polling. */
    for (ix = 0; ix < AC5_INTR_POLLING_ROUND; ix++) {
        check_intr = diag_moka_fpga_get_ext_intr_pending(PENDING_BIT_ESW);

        if (check_intr == TRUE) {
            break;
        }
        msleep(AC5_INTR_POLLING_PERIOD);
    }

    if (check_intr != TRUE) {
        cterr('f', 0, "Interrupt is not detected for xCat5 Interrupt %d ", ix);
        goto _exit_clear;
    }

    /* Call 98dxc25x xcat5 clear interrupt function */
    if (esw_98dxc25x_obj_p->callin_fvt->esw_xcat5_clear_int(dev, esw_98dxc25x_obj_p->cpss_dev) != PASSED) {
        cterr('f',0,"Failed to clear xCat5 interrupt");
        goto _exit;
    }

    prcomplete(testpass, errcount, (char *)0);

    return (PASSED);

 _exit_clear:

    /* Call 98dxc25x xcat5 clear interrupt function */
    if (esw_98dxc25x_obj_p->callin_fvt->esw_xcat5_clear_int(dev, esw_98dxc25x_obj_p->cpss_dev) != PASSED) {
        cterr('f',0,"Failed to clear xCat5 interrupt");
    }

 _exit:

    prcomplete(testpass, errcount, (char *)0);

    return (FAILED);
    
}

/******************************************************************************
 *
 * Function   :	diag_esw_phy_intr_test
 * Description: perform esw phy interrupt test.
 *              
 * Inputs     :	None
 *              
 * Outputs    : PASSED/FAILED
 *
 ******************************************************************************
 */
int diag_esw_phy_intr_test (void)
{

    int rc = 0;
    int port = ELIXIR_PHY_INTR_TEST_PORT_0;
    MAD_DEV *mad_dev;
    uint data;

    dev_98dxc25x_object_t *esw_98dxc25x_obj_p = NULL;
    dev_object_t *esw_dev;

    dev_88e1680_object_t *phy_88e1680_obj_p = NULL;
    dev_object_t *dev;

    char tname[32]; 

    memset(tname, 0, sizeof(tname));
    snprintf(tname, sizeof(tname) ,"PHY Interrupt");
    testname(tname);
    prpass(testpass, "%s, ", tname);


    /* Create 98dxc25x device driver */
    esw_98dxc25x_obj_p = (dev_98dxc25x_object_t *)diag_get_esw_98dxc25x_obj();
    if (esw_98dxc25x_obj_p == NULL) {
        cterr('f', 0, "%s: Creating object failed", __func__);
        return (FAILED);
    }
    esw_dev = (dev_object_t *)esw_98dxc25x_obj_p;

    /* Create 88e1680 device driver */
    phy_88e1680_obj_p = (dev_88e1680_object_t *)diag_get_phy_88e1680_obj();
    if (phy_88e1680_obj_p == NULL) {
        cterr('f', 0, "%s: Creating object failed", __func__);
        return (FAILED);
    }
    dev = (dev_object_t *)phy_88e1680_obj_p;

    mad_dev = &phy_mad_88e1680;

    /* Call 88e1680 PHY generate interrupt function */
    if (phy_88e1680_obj_p->callin_fvt->gen_int(dev, mad_dev, port)!= PASSED) {
        cterr('f',0,"Failed to generate PHY interrupt for 88E1680 Interrupt");
        goto _exit;
    }

    /* Check AC5 interrupt pin(MPP41) is asserted */
    rc = xcat5_reg_pci_read(esw_98dxc25x_obj_p->cpss_dev, DATAIN_REG, &data);
    if (rc == FAILED) {
        cterr('f', 0, "%s: xcat5 pcie register read failed",__func__);
        goto _exit_clear;
    }

    data &= AC5_MPP_BIT_9; /* Do mask to get MPP41(bit 9 of the register) */
    if (data != 0) {
        cterr('f', 0, "Interrupt is not detected for 88E1680 Interrupt");
        goto _exit_clear;
    }

    /* Call 88e1680 PHY clear interrupt function */
	if (phy_88e1680_obj_p->callin_fvt->clear_int(dev, mad_dev, port)!= PASSED) {
        cterr('f',0,"Failed to clear PHY interrupt for 88E1680 Interrupt");
        goto _exit;
    }

    prcomplete(testpass, errcount, (char *)0);

    return (PASSED);

 _exit_clear:

    /* Call 88e1680 PHY clear interrupt function */
	if (phy_88e1680_obj_p->callin_fvt->clear_int(dev, mad_dev, port)!= PASSED) {
        cterr('f',0,"Failed to clear PHY interrupt for 88E1680 Interrupt");
    }

 _exit:

    prcomplete(testpass, errcount, (char *)0);

    return (FAILED);

}
 
/******************************************************************************
 *
 * Function   :	diag_esw_xcat5_print_mac_counter
 * Description: perform xcat5 print mac counter.
 * Inputs     :	NULL
 * Outputs    : PASSED/FAILED
 *
 ******************************************************************************
 */
static int diag_esw_xcat5_print_mac_counter (void)
{
    
    int ix = 0;

    dev_98dxc25x_object_t *esw_98dxc25x_obj_p = NULL;
    dev_object_t *dev;

    /* Create 98dxc25x device driver */
    esw_98dxc25x_obj_p = (dev_98dxc25x_object_t *)diag_get_esw_98dxc25x_obj(); 
    if (esw_98dxc25x_obj_p == NULL) {
        cterr('f', 0, "%s: Creating object failed", __func__);
        return (FAILED);
    }
    dev = (dev_object_t *)esw_98dxc25x_obj_p;

    /* print elixir ac5 port 0~7 counters */
    for (ix = 0; ix < ELIXIR_XCAT5_USED_PORT; ix++) {
        if (esw_98dxc25x_obj_p->callin_fvt->esw_print_mac_counter(dev, esw_98dxc25x_obj_p->cpss_dev,
                                                                  ix) != PASSED) {
            cterr('f',0,"Failed esw_print_mac_counter()");
            goto _exit;
        }
    }

    /* print elixir ac5 port 24 counters (WIFI6 port)*/
    if (esw_98dxc25x_obj_p->callin_fvt->esw_print_mac_counter(dev, esw_98dxc25x_obj_p->cpss_dev,
                                                              XCAT5_TO_WIFI_PORT) != PASSED) {
        cterr('f',0,"Failed esw_print_mac_counter()");
        goto _exit;
    }
    
    /* print elixir ac5 port 26 counters (CPU port)*/
    if (esw_98dxc25x_obj_p->callin_fvt->esw_print_mac_counter(dev, esw_98dxc25x_obj_p->cpss_dev,
                                                              XCAT5_TO_CPU_PORT) != PASSED) {
        cterr('f',0,"Failed esw_print_mac_counter()");
        goto _exit;
    }

    return (PASSED);

_exit:

    return (FAILED);

}

/*******************************************************************************
 *
 * Function    : diag_esw_internal_lpbk_test_1gbps
 * Description : Function to show ESW internal loopback test submenu (For fixed speed 1GBPS).
 * Inputs      : opt - reserved for future use.
 * Outputs     : PASSED / FAILED
 *
 *******************************************************************************
 */
int diag_esw_internal_lpbk_test_1gbps (int opt)
{
    build_primary_submenu(esw_internal_lpbk_test_1gbps_tbl, ESW_INTERNAL_LPBK_TEST_1GBPS_TBL_SIZE,
                          "ESW Internal Loopback test (1GBPS)", &esw_internal_lpbk_test_1gbps_menup);
    build_secondary_submenu(esw_internal_lpbk_test_1gbps_tbl, ESW_INTERNAL_LPBK_TEST_1GBPS_TBL_SIZE,
                            esw_internal_lpbk_test_1gbps_secondary_items);


    if (opt) {
        do_all_menu_items(&esw_internal_lpbk_test_1gbps_menu);
    }
    else {
        menu(esw_internal_lpbk_test_1gbps_menup, esw_internal_lpbk_test_1gbps_secondary_items, '\0');
    }

    return (PASSED);
}

/*******************************************************************************
 *
 * Function    : diag_esw_external_lpbk_test_1gbps
 * Description : Function to show ESW external loopback test submenu.(For fixed speed 1GBP)
 * Inputs      : opt - reserved for future use.
 * Outputs     : PASSED / FAILED
 *
 *******************************************************************************
 */
int diag_esw_external_lpbk_test_1gbps (int opt)
{
    char *tname = "ESW External Loopback test (1GBPS)";

    if (check_menu_flag(D_EXT_LOOPBACK)) {
        prpass(testpass, "\n External loopback flag is off, skip '%s' \
            external loopback test. ", tname);
        prcomplete(testpass, errcount, (char *)0);
        return (PASSED);
    }

    build_primary_submenu(esw_external_lpbk_test_1gbps_tbl, ESW_EXTERNAL_LPBK_TEST_1GBPS_TBL_SIZE,
                          "ESW External Loopback test (1GBPS)", &esw_external_lpbk_test_1gbps_menup);
    build_secondary_submenu(esw_external_lpbk_test_1gbps_tbl, ESW_EXTERNAL_LPBK_TEST_1GBPS_TBL_SIZE,
                            esw_external_lpbk_test_1gbps_secondary_items);

    if (opt) {
        do_all_menu_items(&esw_external_lpbk_test_1gbps_menu);
    }
    else {
        menu(esw_external_lpbk_test_1gbps_menup, esw_external_lpbk_test_1gbps_secondary_items, '\0');
    }

    return (PASSED);
}

/******************************************************************************
 *
 * Function   :	diag_esw_pcie_speed_width_check
 * Description: This function is to check xcat5 PCIE speed and width
 * Inputs     :	None
 * Outputs    : PASSED/FAILED
 *
 ******************************************************************************
 */
int diag_esw_pcie_speed_width_check (void) 
{
    uint32_t bus, domain, reg_val, cap_val, sta_val;
    uint32_t cap_s, sta_s, sta_w; /*cap_w*/


    bus = get_pcie_bus_num(AC5_DEV_VID, AC5_DEV_PID);
    domain = 0;

    reg_val = get_pcie_cap_struct_ptr_with_domain(domain, bus, PCI_DEV_0, PCI_FUN_0, PCI_CAP_PTR_OFFSET);
    if (reg_val == FAILED) {
        cterr('f',0, "Can't get PCI cap pointer");
        return (FAILED);
    }

    cap_val = get_pcie_link_cap_with_domain(domain, bus, PCI_DEV_0, PCI_FUN_0, reg_val);
    sta_val = get_pcie_link_status_with_domain(domain, bus, PCI_DEV_0, PCI_FUN_0, reg_val);

    /* Speed - bit 0~3 */
    cap_s = cap_val & PCI_EXP_LINK_STA_SPD_MASK;
    sta_s = sta_val & PCI_EXP_LINK_STA_SPD_MASK;
    /* Width - bit 4~9 */
    sta_w = (sta_val & PCI_EXP_LINK_STA_WID_MASK) >> PCI_EXP_LINK_WID_SHIFT;

    printf("Capbility Speed: %x, Link Speed: %x, Link Width: %x\n", cap_s, sta_s, sta_w);

    if (sta_s != PCI_EXP_LINK_STA_SPD_8GT) {
        cterr('f',0,"It's not PCIE GEN3 speed between AC5 and CPU\n");
        return (FAILED);
    }

    if (sta_w != PCI_EXP_LINK_STA_WID_1) {
        cterr('f',0,"The width between AC5 and CPU is not x1\n");
        return (FAILED);
    }

    return (PASSED);
}


/******************************************************************************
 *
 * Function   :	diag_esw_xcat5_config_port_pve
 *
 * Description: configure port pve
 *
 * Inputs     :	cpss_dev  - cpss device number
 *              bp_port   - source port
 *              port_num  - destination port
 *
 * Outputs    : PASSED/FAILED
 *
 ******************************************************************************
 */
static int diag_esw_xcat5_config_port_pve (int cpss_dev, int bp_port, int port_num)
{

    dev_98dxc25x_object_t *esw_98dxc25x_obj_p = NULL;
    dev_object_t *dev;

    /* Create 98dxc25x device driver */
    esw_98dxc25x_obj_p = (dev_98dxc25x_object_t *)diag_get_esw_98dxc25x_obj(); 
    if (esw_98dxc25x_obj_p == NULL) {
        cterr('f', 0, "%s: Creating object failed", __func__);
        return (FAILED);
    }
    dev = (dev_object_t *)esw_98dxc25x_obj_p;

    /* Call 98dxc25x config port pve function */
    if (esw_98dxc25x_obj_p->callin_fvt->esw_config_port_pve(dev, cpss_dev, 
                                                            bp_port, port_num) != PASSED) { 
        cterr('f',0,"Failed to configure PVE for port %d", port_num);
	    goto _exit;
    }

    return (PASSED);

 _exit:

    return (FAILED);

}

/******************************************************************************
 *
 * Function   :	diag_esw_xcat5_unconfig_port_pve
 *
 * Description: unconfigure port pve
 *
 * Inputs     :	cpss_dev  - cpss device number
 *              bp_port   - source port
 *              port_num  - destination port
 *
 * Outputs    : PASSED/FAILED
 *
 ******************************************************************************
 */
static int diag_esw_xcat5_unconfig_port_pve (int cpss_dev, int bp_port, int port_num)
{

    dev_98dxc25x_object_t *esw_98dxc25x_obj_p = NULL;
    dev_object_t *dev;

    /* Create 98dxc25x device driver */
    esw_98dxc25x_obj_p = (dev_98dxc25x_object_t *)diag_get_esw_98dxc25x_obj(); 
    if (esw_98dxc25x_obj_p == NULL) {
        cterr('f', 0, "%s: Creating object failed", __func__);
        return (FAILED);
    }
    dev = (dev_object_t *)esw_98dxc25x_obj_p;

    /* Call 98dxc25x unconfig port pve function */
    if (esw_98dxc25x_obj_p->callin_fvt->esw_unconfig_port_pve(dev, cpss_dev, 
                                                              bp_port, port_num) != PASSED) {
        cterr('f',0,"Failed to unconfigure PVE for port %d", port_num);
	    goto _exit;
    }

    return (PASSED);

 _exit:

    return (FAILED);

}


/*-------------------------------------------------
 * $Log: diag_esw_test.c,v $
 * Revision 1.2  2021/09/24 01:21:06  harrchan
 * Collapse Elixir-branch to Main Trunk.
 *
 * Revision 1.1.2.35  2021/06/07 01:19:04  harrchan
 * Add PCIE speed & width scane test into AC5 menu
 *
 * Revision 1.1.2.34  2021/05/31 10:42:59  illiu
 * Remove item Clear xCat5 Counter Utility
 * Remove item ESW 88E1680 Tx Config Read/Write Utility
 * Rename item Print xCat5 Counter Utility to Print xCat5 Mac Counter Utility
 * Rename function esw_print_sw_counter to esw_print_mac_counter
 * Rename function diag_esw_xcat5_print_counter to diag_esw_xcat5_print_mac_counter
 *
 * Revision 1.1.2.33  2021/04/23 02:36:49  illiu
 * Use variable cpss_dev which is a member of 98dxc25x object, instead of using local variable
 *
 * Revision 1.1.2.32  2021/04/12 08:48:11  illiu
 * 1. Replace object-create method as object-get method (Device driver object)
 * 2. Replace sprintf with snprintf
 *
 * Revision 1.1.2.31  2021/03/22 06:28:57  harrchan
 * Clean up code
 *
 * Revision 1.1.2.30  2021/03/22 06:26:47  harrchan
 * Clean up code for AC5 PCIE gen3 speed check
 *
 * Revision 1.1.2.29  2021/03/22 03:24:16  harrchan
 * Add PCIE speed check test in ESW menu
 *
 * Revision 1.1.2.28  2021/03/18 07:59:15  illiu
 * Replace variable phy_dev_88e1680 with phy_mad_88e1680
 *
 * Revision 1.1.2.27  2021/03/15 10:06:25  illiu
 * Rename function: diag_esw_mac_internal_lpbk_test(int) to diag_cpu_esw_mac_lpbk_test(int)
 * Rename function: diag_esw_mac_port_internal_lpbk_test(int) to diag_do_cpu_esw_mac_lpbk_test(int)
 * Rename test item name: (MAC Internal Loopback Test) to (CPU to AC5 MAC Loopback Test)
 * Use macro string to replace magic number
 *
 * Revision 1.1.2.26  2021/02/04 08:51:57  illiu
 * Modify loopback test port mapping and clean up code.
 *
 * Revision 1.1.2.25  2021/02/03 02:50:29  illiu
 * Clean up code
 *
 * Revision 1.1.2.24  2021/01/29 09:20:34  illiu
 * Map panel port number (Foxconn defined) to actual port number (Marvell defined). Related test item is loopback test
 *
 * Revision 1.1.2.23  2020/12/22 09:37:48  illiu
 * Modify tname variable declaration size
 *
 * Revision 1.1.2.22  2020/12/21 09:12:07  illiu
 * 1. Add GE PHY External Loopback Test (1GBPS) item which is for fixed speed 1GBPS
 * 2. Add 1680 PHY Internal/External Loopback Tests (1GBPS) item which is for fixed speed 1GBPS
 *
 * Revision 1.1.2.21  2020/12/04 08:36:55  illiu
 * Add 1680 PHY Test Mode Utility
 *
 * Revision 1.1.2.20  2020/11/19 06:54:49  harrchan
 * Modify MAC internal loopback test
 *
 * Revision 1.1.2.19  2020/11/10 06:44:42  harrchan
 * Add internal/external loopback test into default test
 *
 * Revision 1.1.2.18  2020/11/05 06:34:55  harrchan
 * 1.Base on P1A bring up result to Modify the AC5 MAC/internal/external loopback test
 * 2.Remove some debug message on AC5 init process
 *
 * Revision 1.1.2.17  2020/11/05 03:01:54  illiu
 * Add test item: xCat5 Interrupt Test, PHY Interrupt Test
 *
 * Revision 1.1.2.16  2020/10/15 12:04:49  illiu
 * 1. Move AC5 switch init and exit process to linux_main.c(It means do init once diag application is actived and do exit once diag application is exit)
 * 2. Add port configuration process for wifi6 module(XCAT5_TO_WIFI_PORT=26) which is connected to AC5 switch
 * 3. Add nim_dm driver polling, to check if driver is ready
 * 4. Add nim_dm driver polling, to check if driver exist before doing insmod or rmmod commend
 * 5. Modify the accessed path of pcie device in diag_esw_remove_pcie_device function
 * 6. Modify marvell_cpssPpInit_xcat5 and phy_dev_88e1680_group_start_addr to be static type variable
 * 7. Move array: phy_dev_88e1680 to header file
 * 8. Remove marvell_ac5_cpss_dev_num_elixir variable, and use ELIXIR_AC5_CPSS_DEV macro directly
 * 9. Modify AC5 switch test item name: External Loopback Test ==> PHY External Loopback Test
 * 10.Remove unneeded variable: port_group, port_group_phy_num
 * 11.Modify code alignment
 *
 * Revision 1.1.2.15  2020/10/07 11:20:45  illiu
 * Clean up code
 *
 * Revision 1.1.2.14  2020/10/07 09:19:33  illiu
 * Clean up code
 *
 * Revision 1.1.2.13  2020/10/06 02:06:13  illiu
 * Transform calling objects from AC3 file/function to AC5 file/finction (dev_98dxc323.c -> dev_98dxc25x.c)
 *
 * Revision 1.1.2.12  2020/09/28 10:32:26  illiu
 * Add below utility items:
 * 1. ESW PHY Register Read Utility
 * 2. ESW PHY Register Write Utility
 * 3. ESW 88E1680 Tx Config Read Utility
 * 4. ESW 88E1680 Tx Config Write Utility
 *
 * Revision 1.1.2.11  2020/09/26 03:32:47  illiu
 * Add below Utilities items:
 *     ESW PCI Config Read Utility
 *     ESW PCI Config Write Utility
 *     ESW xCat3 Internal Register Write Utility
 *     ESW xCat3 PP Register Read Utility
 *     ESW xCat3 PP Register Write Utility
 *     Print All PHY Counter Utility
 *     Clear All PHY Counter Utility
 *     Print xCat3 Counter Utility
 *     Clear xCat3 Counter Utility
 *     ESW Reset Default Utility
 *
 * Revision 1.1.2.10  2020/09/25 10:05:42  harrchan
 * Add test item Mac internal loopback test
 *
 * Revision 1.1.2.9  2020/09/25 07:03:50  illiu
 * 1. Modify the <xCat3 Interrupt Test> to call betelguse's FPGA function
 * 2. Remove the retry process which is for Intel Denverton loopback workaround in the <Internal/External Loopback Test>
 *
 * Revision 1.1.2.8  2020/09/24 09:42:58  illiu
 * Add test item(xCat3 Interrupt Test, PHY Interrupt Test)
 *
 * Revision 1.1.2.7  2020/09/23 09:51:53  illiu
 * Replace diag_esw_ext_lpbk_test() with plat_sgmii_lpbk_test() to do loopback test
 *
 * Revision 1.1.2.6  2020/09/22 03:30:56  illiu
 * Add test item(External Loopback Test) and its relative function
 *
 * Revision 1.1.2.5  2020/09/21 09:33:51  illiu
 * Add test item(Internal Loopback Test) and its relative function
 *
 * Revision 1.1.2.4  2020/09/17 09:54:41  illiu
 * Add test item(PHY Register Test)
 *
 * Revision 1.1.2.3  2020/09/15 09:33:35  illiu
 * Fix AC3 switch init and add test item(diag_esw_xcat3_all_register_test)
 *
 * Revision 1.1.2.2  2020/09/10 09:52:19  illiu
 * Delete 88E6390/88E6176 Switch related code
 *
 * Revision 1.1.2.1  2020/09/09 09:08:06  illiu
 * First version which has been ported with Dreamliner and Marvel CPSS
 *
 * Revision 1.2  2019/01/10 06:36:22  wilbhuan
 * The beginning of Betelgeuse application code.
 *
 *-------------------------------------------------
 */
