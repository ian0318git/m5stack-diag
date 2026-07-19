 /* $Id: diag_esw_test.c,v 1.3 2020/04/20 02:28:24 lucywang Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/nanook/diag_esw_test.c,v $
 *------------------------------------------------------------------
 *
 * diag_esw_test.c - This file is for ethernet switch test
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
#include "diag_esw_lib.h"
#include "diag_esw_util.h"
#include "dnv_eth_lib.h"
#include "dnv_gpio_lib.h"
#include "dnv_eth_lib.h"
#include "diag_eth_pkt_txrx.h"
#include "dev_88e1680.h"
#include "dev_98dxc323.h"
#include "madApi.h"
#include "madHwCntl.h"
#include "diag_common.h"
#include "nanook_comm.h"
#include "dash_fpga.h"


/*
 * Global variables
 */

extern int marvell_cpssPpInit_xcat3;
extern MAD_DEV phy_dev_88e1680[NANOOK_1680_GROUP_NUM];
extern int marvell_ac3_cpss_dev_num_nanook;

extern int fpga_reset_api(uint, uint, uint, uint);

/* Local functions */
static int diag_esw_check_phy_link_up (uint, uint *);
static int diag_esw_check_phy_speed (uint, uint);
static int diag_esw_phy_clear_phy_counter (void);
static int diag_esw_phy_print_phy_counter (void);
static int diag_esw_xcat3_clear_counter (void);
static int diag_esw_xcat3_print_counter (void);
static int diag_esw_phy_port_internal_lpbk_test (int, int, int);
static int diag_esw_snake_lpbk_test (int, int);
static int diag_esw_phy_port_external_lpbk_test (int, int, int);
int diag_esw_int_lpbk_test_sub (int);
int diag_esw_ext_lpbk_test_sub (int);
int diag_esw_snake_test (void);
int diag_esw_xcat3_all_register_test (void);
int diag_esw_phy_register_test (void);
int diag_esw_xcat3_intr_test (void);
int diag_esw_phy_intr_test (void);

/* Local build menu functions */
int build_esw_test_menu(boolean);
int build_esw_util_menu(boolean);
int diag_esw_internal_lpbk_test(int);
int diag_esw_external_lpbk_test (int);


/* ESW supported speed table */
static int     esw_speed_tbl[] = {SPD_10MBPS, SPD_100MBPS, SPD_1000MBPS};


static int     esw_snake_foward_map_tbl[] = 
	{GE_XCAT3_PORT, XCAT3_PORT_01, XCAT3_PORT_03, XCAT3_PORT_05, XCAT3_PORT_07, 
	XCAT3_PORT_09, XCAT3_PORT_11, XCAT3_PORT_13, XCAT3_PORT_15, XCAT3_PORT_17,
	XCAT3_PORT_19, XCAT3_PORT_21, XCAT3_PORT_23};

static int     esw_snake_foward_pair_map_tbl[] = 
	{XCAT3_PORT_00, XCAT3_PORT_02, XCAT3_PORT_04, XCAT3_PORT_06, XCAT3_PORT_08, 
	XCAT3_PORT_10, XCAT3_PORT_12, XCAT3_PORT_14, XCAT3_PORT_16, XCAT3_PORT_18,
	XCAT3_PORT_20, XCAT3_PORT_22, GE_XCAT3_PORT};

static int     esw_snake_backward_map_tbl[] = 
	{GE_XCAT3_PORT, XCAT3_PORT_22, XCAT3_PORT_20, XCAT3_PORT_18, XCAT3_PORT_16, 
	XCAT3_PORT_14, XCAT3_PORT_12, XCAT3_PORT_10, XCAT3_PORT_08, XCAT3_PORT_06,
	XCAT3_PORT_04, XCAT3_PORT_02, XCAT3_PORT_00};

static int     esw_snake_backward_pair_map_tbl[] = 
	{XCAT3_PORT_23, XCAT3_PORT_21, XCAT3_PORT_19, XCAT3_PORT_17, XCAT3_PORT_15, 
	XCAT3_PORT_13, XCAT3_PORT_11, XCAT3_PORT_09, XCAT3_PORT_07, XCAT3_PORT_05,
	XCAT3_PORT_03, XCAT3_PORT_01, GE_XCAT3_PORT};

static int     esw_snake_pair_map_tbl[] = 
	{ESW_PORT_00, ESW_PORT_02, ESW_PORT_04, ESW_PORT_06, ESW_PORT_08, 
	ESW_PORT_10, ESW_PORT_12, ESW_PORT_14, ESW_PORT_16, ESW_PORT_18,
	ESW_PORT_20, ESW_PORT_22};


/*
 * Sub Menu used for "Ethernet switch test -> Ethernet switch submenu test"
 */


submenu_xtable_t eth_submenu_table[] = {
    {"ESW Utilities",
     (PFT) build_esw_util_menu, FALSE, 0,
     (type_t(*)())0, 0, (PFT) build_esw_util_menu, TRUE},

    {"xCat3 All Register Test",
     (PFT) diag_esw_xcat3_all_register_test, FALSE,
     MF_CONTINUOUS | MF_DOGRP | MF_DOALL,
     (type_t(*)())0, 0, (PFT) 0, 0},

    {"PHY Register Test",
     (PFT) diag_esw_phy_register_test, FALSE,
     MF_CONTINUOUS | MF_DOGRP | MF_DOALL,
     (type_t(*)())0, 0, (PFT) 0, 0},

    {"xCat3 Interrupt Test",
     (PFT) diag_esw_xcat3_intr_test, FALSE,
     MF_CONTINUOUS | MF_DOGRP | MF_DOALL,
     (type_t(*)())0, 0, (PFT) 0, 0},

    {"PHY Interrupt Test",
     (PFT) diag_esw_phy_intr_test, FALSE,
     MF_CONTINUOUS | MF_DOGRP | MF_DOALL,
     (type_t(*)())0, 0, (PFT) 0, 0},

    {"Snake Test for CPU MAC and PHY",
     (PFT) diag_esw_snake_test, FALSE,
     MF_CONTINUOUS | MF_DOGRP | MF_DOALL | MF_DOALL,
     (type_t(*)())0, 0, (PFT)0, 0},

    {"Internal Loopback Test",
     (PFT) diag_esw_internal_lpbk_test, TRUE,
     MF_CONTINUOUS | MF_SHOW_ERRCOUNT,
     (type_t(*)())0, 0,  (type_t(*)())diag_esw_internal_lpbk_test, FALSE},

    {"External Loopback Test",
     (PFT) diag_esw_external_lpbk_test, TRUE,
     MF_CONTINUOUS | MF_SHOW_ERRCOUNT,
     (type_t(*)())0, 0,  (type_t(*)())diag_esw_external_lpbk_test, FALSE},

     
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

    {"ESW PCI Config Read Utility",
     (PFT) diag_esw_pcie_config_rd_util, FALSE, 0,
     (type_t(*)())0, 0, (PFT) 0, 0},

    {"ESW PCI Config Write Utility",
     (PFT) diag_esw_pcie_config_wr_util, FALSE, 0,
     (type_t(*)())0, 0, (PFT) 0, 0},

    {"ESW xCat3 Internal Register Read Utility",
     (PFT) diag_esw_xcat3_internal_reg_rd_util, FALSE, 0,
     (type_t(*)())0, 0, (PFT) 0, 0},

    {"ESW xCat3 Internal Register Write Utility",
     (PFT) diag_esw_xcat3_internal_reg_wr_util, FALSE, 0,
     (type_t(*)())0, 0, (PFT) 0, 0},

    {"ESW xCat3 PP Register Read Utility",
     (PFT) diag_esw_xcat3_reg_rd_util, FALSE, 0,
     (type_t(*)())0, 0, (PFT) 0, 0},

    {"ESW xCat3 PP Register Write Utility",
     (PFT) diag_esw_xcat3_reg_wr_util, FALSE, 0,
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

    {"Print xCat3 Counter Utility",
     (PFT) diag_esw_xcat3_print_counter, FALSE, 0,
     (type_t(*)())0, 0, (PFT) 0, 0},

    {"Clear xCat3 Counter Utility",
     (PFT) diag_esw_xcat3_clear_counter, FALSE, 0,
     (type_t(*)())0, 0, (PFT) 0, 0},

    {"PHY Test Mode Utility",
     (PFT) diag_esw_phy_test_mode_util, FALSE, 0,
     (type_t(*)())0, 0, (PFT) 0, 0},

    {"Set IXIA Snake Configuration Utility",
     (PFT) diag_esw_set_ixia_snake_config_util, FALSE, 0,
     (type_t(*)())0, 0, (PFT) 0, 0},

    {"ESW Reset Default Utility",
     (PFT) diag_reset_esw_to_default, FALSE, 0,
     (type_t(*)())0, 0, (PFT) 0, 0},

    {"ESW 10G KR Test Mode Utility",
     (PFT) diag_esw_xcat3_10g_kr_test_mode_util, FALSE, 0,
     (type_t(*)())0, 0, (PFT) 0, 0},

    {"ESW xCat3 Serdes Tx Config Read Utility",
     (PFT) diag_esw_xcat3_serdes_tx_config_read_util, FALSE, 0,
     (type_t(*)())0, 0, (PFT) 0, 0},

    {"ESW xCat3 Serdes Tx Config Write Utility",
     (PFT) diag_esw_xcat3_serdes_tx_config_write_util, FALSE, 0,
     (type_t(*)())0, 0, (PFT) 0, 0},

    {"ESW 88E1680 Tx Config Read Utility",
     (PFT) diag_esw_xcat3_phy_tx_config_read_util, FALSE, 0,
     (type_t(*)())0, 0, (PFT) 0, 0},

    {"ESW 88E1680 Tx Config Write Utility",
     (PFT) diag_esw_xcat3_phy_tx_config_write_util, FALSE, 0,
     (type_t(*)())0, 0, (PFT) 0, 0},

    {"Set for IXIA Speed 1000 Configuration Utility",
     (PFT) diag_esw_set_ixia_speed_config_util, 0, 0,
     (type_t(*)())0, 0, (PFT) 0, 0},

    {"Set for IXIA Speed 100 Configuration Utility",
     (PFT) diag_esw_set_ixia_speed_config_util, 1, 0,
     (type_t(*)())0, 0, (PFT) 0, 0},

    {"Set for IXIA Speed 10 Configuration Utility",
     (PFT) diag_esw_set_ixia_speed_config_util, 2, 0,
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


/*
 * Sub Menu used for "Ethernet switch test -> Ethernet switch submenu test -> Internal Loopback Test"
 */

submenu_xtable_t esw_internal_lpbk_test_tbl[] = {

    {"Internal Loopback Test for ESW Port 0",
     (PFT) diag_esw_int_lpbk_test_sub, 0,
     MF_CONTINUOUS | MF_DOGRP | MF_DOALL,
     (type_t(*)())0, 0, (PFT) 0, 0},

    {"Internal Loopback Test for ESW Port 1",
     (PFT) diag_esw_int_lpbk_test_sub, 1,
     MF_CONTINUOUS | MF_DOGRP | MF_DOALL,
     (type_t(*)())0, 0, (PFT) 0, 0},

    {"Internal Loopback Test for ESW Port 2",
     (PFT) diag_esw_int_lpbk_test_sub, 2,
     MF_CONTINUOUS | MF_DOGRP | MF_DOALL,
     (type_t(*)())0, 0, (PFT) 0, 0},

    {"Internal Loopback Test for ESW Port 3",
     (PFT) diag_esw_int_lpbk_test_sub, 3,
     MF_CONTINUOUS | MF_DOGRP | MF_DOALL,
     (type_t(*)())0, 0, (PFT) 0, 0},

    {"Internal Loopback Test for ESW Port 4",
     (PFT) diag_esw_int_lpbk_test_sub, 4,
     MF_CONTINUOUS | MF_DOGRP | MF_DOALL,
     (type_t(*)())0, 0, (PFT) 0, 0},

    {"Internal Loopback Test for ESW Port 5",
     (PFT) diag_esw_int_lpbk_test_sub, 5,
     MF_CONTINUOUS | MF_DOGRP | MF_DOALL,
     (type_t(*)())0, 0, (PFT) 0, 0},

    {"Internal Loopback Test for ESW Port 6",
     (PFT) diag_esw_int_lpbk_test_sub, 6,
     MF_CONTINUOUS | MF_DOGRP | MF_DOALL,
     (type_t(*)())0, 0, (PFT) 0, 0},

    {"Internal Loopback Test for ESW Port 7",
     (PFT) diag_esw_int_lpbk_test_sub, 7,
     MF_CONTINUOUS | MF_DOGRP | MF_DOALL,
     (type_t(*)())0, 0, (PFT) 0, 0},

    {"Internal Loopback Test for ESW Port 8",
     (PFT) diag_esw_int_lpbk_test_sub, 8,
     MF_CONTINUOUS | MF_DOGRP | MF_DOALL,
     (type_t(*)())0, 0, (PFT) 0, 0},

    {"Internal Loopback Test for ESW Port 9",
     (PFT) diag_esw_int_lpbk_test_sub, 9,
     MF_CONTINUOUS | MF_DOGRP | MF_DOALL,
     (type_t(*)())0, 0, (PFT) 0, 0},

    {"Internal Loopback Test for ESW Port 10",
     (PFT) diag_esw_int_lpbk_test_sub, 10,
     MF_CONTINUOUS | MF_DOGRP | MF_DOALL,
     (type_t(*)())0, 0, (PFT) 0, 0},

    {"Internal Loopback Test for ESW Port 11",
     (PFT) diag_esw_int_lpbk_test_sub, 11,
     MF_CONTINUOUS | MF_DOGRP | MF_DOALL,
     (type_t(*)())0, 0, (PFT) 0, 0},

    {"Internal Loopback Test for ESW Port 12",
     (PFT) diag_esw_int_lpbk_test_sub, 12,
     MF_CONTINUOUS | MF_DOGRP | MF_DOALL,
     (type_t(*)())0, 0, (PFT) 0, 0},

    {"Internal Loopback Test for ESW Port 13",
     (PFT) diag_esw_int_lpbk_test_sub, 13,
     MF_CONTINUOUS | MF_DOGRP | MF_DOALL,
     (type_t(*)())0, 0, (PFT) 0, 0},

    {"Internal Loopback Test for ESW Port 14",
     (PFT) diag_esw_int_lpbk_test_sub, 14,
     MF_CONTINUOUS | MF_DOGRP | MF_DOALL,
     (type_t(*)())0, 0, (PFT) 0, 0},

    {"Internal Loopback Test for ESW Port 15",
     (PFT) diag_esw_int_lpbk_test_sub, 15,
     MF_CONTINUOUS | MF_DOGRP | MF_DOALL,
     (type_t(*)())0, 0, (PFT) 0, 0},

    {"Internal Loopback Test for ESW Port 16",
     (PFT) diag_esw_int_lpbk_test_sub, 16,
     MF_CONTINUOUS | MF_DOGRP | MF_DOALL,
     (type_t(*)())0, 0, (PFT) 0, 0},

    {"Internal Loopback Test for ESW Port 17",
     (PFT) diag_esw_int_lpbk_test_sub, 17,
     MF_CONTINUOUS | MF_DOGRP | MF_DOALL,
     (type_t(*)())0, 0, (PFT) 0, 0},

    {"Internal Loopback Test for ESW Port 18",
     (PFT) diag_esw_int_lpbk_test_sub, 18,
     MF_CONTINUOUS | MF_DOGRP | MF_DOALL,
     (type_t(*)())0, 0, (PFT) 0, 0},

    {"Internal Loopback Test for ESW Port 19",
     (PFT) diag_esw_int_lpbk_test_sub, 19,
     MF_CONTINUOUS | MF_DOGRP | MF_DOALL,
     (type_t(*)())0, 0, (PFT) 0, 0},

    {"Internal Loopback Test for ESW Port 20",
     (PFT) diag_esw_int_lpbk_test_sub, 20,
     MF_CONTINUOUS | MF_DOGRP | MF_DOALL,
     (type_t(*)())0, 0, (PFT) 0, 0},

    {"Internal Loopback Test for ESW Port 21",
     (PFT) diag_esw_int_lpbk_test_sub, 21,
     MF_CONTINUOUS | MF_DOGRP | MF_DOALL,
     (type_t(*)())0, 0, (PFT) 0, 0},

    {"Internal Loopback Test for ESW Port 22",
     (PFT) diag_esw_int_lpbk_test_sub, 22,
     MF_CONTINUOUS | MF_DOGRP | MF_DOALL,
     (type_t(*)())0, 0, (PFT) 0, 0},

    {"Internal Loopback Test for ESW Port 23",
     (PFT) diag_esw_int_lpbk_test_sub, 23,
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

submenu_xtable_t esw_external_lpbk_test_tbl[] = {

    {"External Loopback Test for ESW Port 0",
     (PFT) diag_esw_ext_lpbk_test_sub, 0,
     MF_CONTINUOUS | MF_DOGRP | MF_DOALL,
     (type_t(*)())0, 0, (PFT) 0, 0},

    {"External Loopback Test for ESW Port 1",
     (PFT) diag_esw_ext_lpbk_test_sub, 1,
     MF_CONTINUOUS | MF_DOGRP | MF_DOALL,
     (type_t(*)())0, 0, (PFT) 0, 0},

    {"External Loopback Test for ESW Port 2",
     (PFT) diag_esw_ext_lpbk_test_sub, 2,
     MF_CONTINUOUS | MF_DOGRP | MF_DOALL,
     (type_t(*)())0, 0, (PFT) 0, 0},

    {"External Loopback Test for ESW Port 3",
     (PFT) diag_esw_ext_lpbk_test_sub, 3,
     MF_CONTINUOUS | MF_DOGRP | MF_DOALL,
     (type_t(*)())0, 0, (PFT) 0, 0},

    {"External Loopback Test for ESW Port 4",
     (PFT) diag_esw_ext_lpbk_test_sub, 4,
     MF_CONTINUOUS | MF_DOGRP | MF_DOALL,
     (type_t(*)())0, 0, (PFT) 0, 0},

    {"External Loopback Test for ESW Port 5",
     (PFT) diag_esw_ext_lpbk_test_sub, 5,
     MF_CONTINUOUS | MF_DOGRP | MF_DOALL,
     (type_t(*)())0, 0, (PFT) 0, 0},

    {"External Loopback Test for ESW Port 6",
     (PFT) diag_esw_ext_lpbk_test_sub, 6,
     MF_CONTINUOUS | MF_DOGRP | MF_DOALL,
     (type_t(*)())0, 0, (PFT) 0, 0},

    {"External Loopback Test for ESW Port 7",
     (PFT) diag_esw_ext_lpbk_test_sub, 7,
     MF_CONTINUOUS | MF_DOGRP | MF_DOALL,
     (type_t(*)())0, 0, (PFT) 0, 0},

    {"External Loopback Test for ESW Port 8",
     (PFT) diag_esw_ext_lpbk_test_sub, 8,
     MF_CONTINUOUS | MF_DOGRP | MF_DOALL,
     (type_t(*)())0, 0, (PFT) 0, 0},

    {"External Loopback Test for ESW Port 9",
     (PFT) diag_esw_ext_lpbk_test_sub, 9,
     MF_CONTINUOUS | MF_DOGRP | MF_DOALL,
     (type_t(*)())0, 0, (PFT) 0, 0},

    {"External Loopback Test for ESW Port 10",
     (PFT) diag_esw_ext_lpbk_test_sub, 10,
     MF_CONTINUOUS | MF_DOGRP | MF_DOALL,
     (type_t(*)())0, 0, (PFT) 0, 0},

    {"External Loopback Test for ESW Port 11",
     (PFT) diag_esw_ext_lpbk_test_sub, 11,
     MF_CONTINUOUS | MF_DOGRP | MF_DOALL,
     (type_t(*)())0, 0, (PFT) 0, 0},

    {"External Loopback Test for ESW Port 12",
     (PFT) diag_esw_ext_lpbk_test_sub, 12,
     MF_CONTINUOUS | MF_DOGRP | MF_DOALL,
     (type_t(*)())0, 0, (PFT) 0, 0},

    {"External Loopback Test for ESW Port 13",
     (PFT) diag_esw_ext_lpbk_test_sub, 13,
     MF_CONTINUOUS | MF_DOGRP | MF_DOALL,
     (type_t(*)())0, 0, (PFT) 0, 0},

    {"External Loopback Test for ESW Port 14",
     (PFT) diag_esw_ext_lpbk_test_sub, 14,
     MF_CONTINUOUS | MF_DOGRP | MF_DOALL,
     (type_t(*)())0, 0, (PFT) 0, 0},

    {"External Loopback Test for ESW Port 15",
     (PFT) diag_esw_ext_lpbk_test_sub, 15,
     MF_CONTINUOUS | MF_DOGRP | MF_DOALL,
     (type_t(*)())0, 0, (PFT) 0, 0},

    {"External Loopback Test for ESW Port 16",
     (PFT) diag_esw_ext_lpbk_test_sub, 16,
     MF_CONTINUOUS | MF_DOGRP | MF_DOALL,
     (type_t(*)())0, 0, (PFT) 0, 0},

    {"External Loopback Test for ESW Port 17",
     (PFT) diag_esw_ext_lpbk_test_sub, 17,
     MF_CONTINUOUS | MF_DOGRP | MF_DOALL,
     (type_t(*)())0, 0, (PFT) 0, 0},

    {"External Loopback Test for ESW Port 18",
     (PFT) diag_esw_ext_lpbk_test_sub, 18,
     MF_CONTINUOUS | MF_DOGRP | MF_DOALL,
     (type_t(*)())0, 0, (PFT) 0, 0},

    {"External Loopback Test for ESW Port 19",
     (PFT) diag_esw_ext_lpbk_test_sub, 19,
     MF_CONTINUOUS | MF_DOGRP | MF_DOALL,
     (type_t(*)())0, 0, (PFT) 0, 0},

    {"External Loopback Test for ESW Port 20",
     (PFT) diag_esw_ext_lpbk_test_sub, 20,
     MF_CONTINUOUS | MF_DOGRP | MF_DOALL,
     (type_t(*)())0, 0, (PFT) 0, 0},

    {"External Loopback Test for ESW Port 21",
     (PFT) diag_esw_ext_lpbk_test_sub, 21,
     MF_CONTINUOUS | MF_DOGRP | MF_DOALL,
     (type_t(*)())0, 0, (PFT) 0, 0},

    {"External Loopback Test for ESW Port 22",
     (PFT) diag_esw_ext_lpbk_test_sub, 22,
     MF_CONTINUOUS | MF_DOGRP | MF_DOALL,
     (type_t(*)())0, 0, (PFT) 0, 0},

    {"External Loopback Test for ESW Port 23",
     (PFT) diag_esw_ext_lpbk_test_sub, 23,
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
    char cmd[256];
    uint rc;
    testname(tname);

    rc = enable_ether_interface(ETHER_INTERFACE_AC3);
    if (rc != PASSED) {
        cterr('f', 0, "Failed to insmod ixgbe.");
        return (FAILED);
    }
	
    system(ETH_INSMOD_AC3_NIM_DM_MODULE);

    /* Init Switch */
    if (diag_esw_init() != PASSED) {
        cterr('f', 0, "Failed to init Switch.");
        return (FAILED);
    }

    memset(cmd, 0, sizeof(cmd));
    sprintf(cmd, "ifconfig %s up > /dev/null", inface_lan0p1);
    system(cmd);

    build_primary_submenu(eth_submenu_table, ETH_SUBMENU_TABLE_SIZE,
                          "Ethernet Switch test", &eth_submenup);
    build_secondary_submenu(eth_submenu_table, ETH_SUBMENU_TABLE_SIZE,
                            eth_secondary_items);
    if (mb_temp_test_items_executed) {
        menu(&eth_subtest_menu, eth_secondary_items, 0);
    } else {
        do_all_menu_items(eth_submenup);
    }

    diag_esw_exit();

    rc = enable_ether_interface(ETHER_INTERFACE_NIM);
    if (rc != PASSED) {
        cterr('f', 0, "Failed to insmod ixgbe.");
        return (FAILED);
    }
    msleep(SLEEP_1000);

    sprintf(cmd, "ifconfig %s up > /dev/null; ifconfig %s 192.123.123.1",
            inface_lan0p0, inface_lan0p0);
    system(cmd);

    msleep(WAIT_BK_LINK_UP);
    system(NANOOK_KILL_DHCPD);
    system(NANOOK_KILL_OPENTFTP);
    system(NANOOK_DHCPD);
    system(NANOOK_OPENTFTP);
    
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

    if (diag_reset_esw_to_default(0) == FAILED) {
        cterr('f', 0, "%s: Reset ESW to default failed.",__func__);
        return (FAILED);
    }

    if (opt) {
        do_all_menu_items(&esw_internal_lpbk_test_menu);
    } else {
        menu(esw_internal_lpbk_test_menup, esw_internal_lpbk_test_secondary_items, '\0');
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
    } else {
        menu(esw_external_lpbk_test_menup, esw_external_lpbk_test_secondary_items, '\0');
    }

    return (PASSED);
}


/******************************************************************************
 *
 * Function   :	diag_esw_check_phy_link_up
 * Description: perform check all phy link up.
 * Inputs     :	NULL
 * Outputs    : PASSED/FAILED
 *
 ******************************************************************************
 */
static int
diag_esw_check_phy_link_up (uint port_num, uint * link_status)
{  
    int rc = 0;
    uint port_group, port_group_phy_num;
    uint page_num, reg_num, data;
    MAD_DEV * mad_dev;
	
    dev_88e1680_object_t phy_88e1680_obj;
    dev_88e1680_object_t *phy_88e1680_obj_p  = &phy_88e1680_obj;

    /* Create 88e1680 device driver */
    rc = diag_phy_88e1680_dev_create(phy_88e1680_obj_p);
    if (rc == FAILED) {
        cterr('f', 0, "%s: Creating object failed",__func__);
        goto _exit;
    }

    port_group = (port_num / NANOOK_1680_NUM_PHY_IN_ONE_GRUOP);
    port_group_phy_num = (port_num % NANOOK_1680_NUM_PHY_IN_ONE_GRUOP);
    mad_dev = &phy_dev_88e1680[port_group];

    page_num = 0;
    reg_num = 17;

    /* Call 88e1680 PHY register read utility function */
    if (phy_88e1680_obj_p->callin_fvt->read_phy_reg_util((dev_object_t *)phy_88e1680_obj_p, mad_dev, port_group_phy_num, page_num, reg_num, &data)!= PASSED) {
        cterr('f',0,"Failed to read phy reg.", port_num);
        goto _exit;
    }

    //BIT 10: Copper Link
    //BIT 3: Global Link Status
    if((data & COOPER_AND_GLOBAL_LINK_UP) == COOPER_AND_GLOBAL_LINK_UP) {
        *link_status = TRUE;
    } else {
        *link_status = FALSE;
    }
  
    phy_88e1680_obj_p->base.dev_object_fvt->dev_destroy((dev_object_t **)&phy_88e1680_obj_p);
    return(PASSED);
	
 _exit:
    phy_88e1680_obj_p->base.dev_object_fvt->dev_destroy((dev_object_t **)&phy_88e1680_obj_p);
    return (FAILED);

}


/******************************************************************************
 *
 * Function   :	diag_esw_check_phy_speed
 * Description: perform check all phy link up.
 * Inputs     :	NULL
 * Outputs    : PASSED/FAILED
 *
 ******************************************************************************
 */
static int
diag_esw_check_phy_speed (uint port_num, uint port_speed)
{  
    int rc = 0;
    uint port_group, port_group_phy_num;
    uint page_num, reg_num, data;
    uint target_speed_pattern;
    MAD_DEV * mad_dev;
	
    dev_88e1680_object_t phy_88e1680_obj;
    dev_88e1680_object_t *phy_88e1680_obj_p  = &phy_88e1680_obj;

    /* Create 88e1680 device driver */
    rc = diag_phy_88e1680_dev_create(phy_88e1680_obj_p);
    if (rc == FAILED) {
        cterr('f', 0, "%s: Creating object failed",__func__);
        goto _exit;
    }

    port_group = (port_num / NANOOK_1680_NUM_PHY_IN_ONE_GRUOP);
    port_group_phy_num = (port_num % NANOOK_1680_NUM_PHY_IN_ONE_GRUOP);
    mad_dev = &phy_dev_88e1680[port_group];

    page_num = 0;
    reg_num = 17;

    /* Call 88e1680 PHY register read utility function */
    if (phy_88e1680_obj_p->callin_fvt->read_phy_reg_util((dev_object_t *)phy_88e1680_obj_p, mad_dev, port_group_phy_num, page_num, reg_num, &data)!= PASSED) {
        cterr('f',0,"Failed to read phy reg.", port_num);
        goto _exit;
    }

    if(port_speed == SPD_1000MBPS) {
        target_speed_pattern = MAD_SPEED_1000M;
    } else if (port_speed == SPD_100MBPS) {
        target_speed_pattern = MAD_SPEED_100M;
    } else if (port_speed == SPD_10MBPS) {
        target_speed_pattern = MAD_SPEED_10M;
    } else {
        cterr('f', 0, "%s: Unsupported speed.\n",__func__);
        goto _exit;
    }

    if (((data & SPEED_MASK) >> SPEED_SHIFT_BIT) != target_speed_pattern) {
	 cterr('f', 0, "%s: Port %d link up with unexpected speed %d, expected speed %d (0:10M, 1:100M, 2:1000M)\n",__func__, port_num,  ((data & SPEED_MASK) >> SPEED_SHIFT_BIT) , target_speed_pattern);
        goto _exit;
    } 

    if ((NVRAM)->diagflag & D_VERBOSE) {
        if (((data & SPEED_MASK) >> SPEED_SHIFT_BIT) == MAD_SPEED_1000M) {
            printf("Link up with 1000M for port %d\n", port_num);
        } else if (((data & SPEED_MASK) >> SPEED_SHIFT_BIT) == MAD_SPEED_100M) {
            printf("Link up with 100M for port %d\n", port_num);
        } else if (((data & SPEED_MASK) >> SPEED_SHIFT_BIT) == MAD_SPEED_10M) {
            printf("Link up with 10M for port %d\n", port_num);
        } else {
            cterr('f', 0, "%s: Unsupported speed.\n",__func__);
            goto _exit;
        } 
    }


    phy_88e1680_obj_p->base.dev_object_fvt->dev_destroy((dev_object_t **)&phy_88e1680_obj_p);
    return(PASSED);
	
 _exit:
    phy_88e1680_obj_p->base.dev_object_fvt->dev_destroy((dev_object_t **)&phy_88e1680_obj_p);
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
static int
diag_esw_phy_clear_phy_counter (void)
{
    int ix, rc = 0;
    uint port_group, port_group_phy_num;
    MAD_DEV * mad_dev;
	
    dev_88e1680_object_t phy_88e1680_obj;
    dev_88e1680_object_t *phy_88e1680_obj_p  = &phy_88e1680_obj;

    /* Create 88e1680 device driver */
    rc = diag_phy_88e1680_dev_create(phy_88e1680_obj_p);
    if (rc == FAILED) {
        cterr('f', 0, "%s: Creating object failed",__func__);
        goto _exit;
    }

    for (ix = 0; ix < NANOOK_ESW_PORT_NUM; ix++) {
	 port_group = (ix / NANOOK_1680_NUM_PHY_IN_ONE_GRUOP);
        port_group_phy_num = (ix % NANOOK_1680_NUM_PHY_IN_ONE_GRUOP);
        mad_dev = &phy_dev_88e1680[port_group];
       /* Call 88e1680 PHY clear counter function */
        if (phy_88e1680_obj_p->callin_fvt->clear_phy_counter((dev_object_t *)phy_88e1680_obj_p, mad_dev, port_group_phy_num)!= PASSED) {
            cterr('f',0,"Failed to clear phy counter for phy port %d", ix);
            goto _exit;
        }
    }

    phy_88e1680_obj_p->base.dev_object_fvt->dev_destroy((dev_object_t **)&phy_88e1680_obj_p);
    return(PASSED);
	
 _exit:
    phy_88e1680_obj_p->base.dev_object_fvt->dev_destroy((dev_object_t **)&phy_88e1680_obj_p);
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
static int
diag_esw_phy_print_phy_counter (void)
{
    int ix, rc = 0;
    uint port_group, port_group_phy_num;
    MAD_DEV * mad_dev;
	
    dev_88e1680_object_t phy_88e1680_obj;
    dev_88e1680_object_t *phy_88e1680_obj_p  = &phy_88e1680_obj;

    /* Create 88e1680 device driver */
    rc = diag_phy_88e1680_dev_create(phy_88e1680_obj_p);
    if (rc == FAILED) {
        cterr('f', 0, "%s: Creating object failed",__func__);
        goto _exit;
    }

    for (ix = 0; ix < NANOOK_ESW_PORT_NUM; ix++) {
	 port_group = (ix / NANOOK_1680_NUM_PHY_IN_ONE_GRUOP);
        port_group_phy_num = (ix % NANOOK_1680_NUM_PHY_IN_ONE_GRUOP);
        mad_dev = &phy_dev_88e1680[port_group];
       /* Call 88e1680 PHY print counter function */
        if (phy_88e1680_obj_p->callin_fvt->print_phy_counter((dev_object_t *)phy_88e1680_obj_p, mad_dev, port_group_phy_num)!= PASSED) {
            cterr('f',0,"Failed to clear phy counter for phy port %d", ix);
            goto _exit;
        }
    }

    phy_88e1680_obj_p->base.dev_object_fvt->dev_destroy((dev_object_t **)&phy_88e1680_obj_p);
    return(PASSED);
	
 _exit:
    phy_88e1680_obj_p->base.dev_object_fvt->dev_destroy((dev_object_t **)&phy_88e1680_obj_p);
    return (FAILED);

}

/******************************************************************************
 *
 * Function   :	diag_esw_xcat3_clear_counter
 * Description: perform xcat3 clear counter.
 * Inputs     :	NULL
 * Outputs    : PASSED/FAILED
 *
 ******************************************************************************
 */
static int
diag_esw_xcat3_clear_counter (void)
{
    int rc = PASSED;
    int cpss_dev = marvell_ac3_cpss_dev_num_nanook;
    dev_98dxc323_object_t esw_98dxc323_obj;
    dev_98dxc323_object_t *esw_98dxc323_obj_p  = &esw_98dxc323_obj;

    /* Create 98dxc323 device driver */
    rc = diag_esw_98dxc323_dev_create(esw_98dxc323_obj_p);
    if (rc == FAILED) {
        cterr('f', 0, "%s: Creating object failed",__func__);
        return (FAILED);
    }

    if (esw_98dxc323_obj_p->callin_fvt->esw_clear_sw_counter((dev_object_t *)esw_98dxc323_obj_p, cpss_dev, NANOOK_ESW_PORT_NUM) != PASSED) {
         cterr('f',0,"Failed esw_print_sw_counter()");
         goto _exit;
    }

    return (PASSED);

_exit:
    esw_98dxc323_obj_p->base.dev_object_fvt->dev_destroy((dev_object_t **)&esw_98dxc323_obj_p);
    return (FAILED);

}



/******************************************************************************
 *
 * Function   :	diag_esw_xcat3_print_counter
 * Description: perform xcat3 print counter.
 * Inputs     :	NULL
 * Outputs    : PASSED/FAILED
 *
 ******************************************************************************
 */
static int
diag_esw_xcat3_print_counter (void)
{
    int rc = PASSED;
    int cpss_dev = marvell_ac3_cpss_dev_num_nanook;
    dev_98dxc323_object_t esw_98dxc323_obj;
    dev_98dxc323_object_t *esw_98dxc323_obj_p  = &esw_98dxc323_obj;

    /* Create 98dxc323 device driver */
    rc = diag_esw_98dxc323_dev_create(esw_98dxc323_obj_p);
    if (rc == FAILED) {
        cterr('f', 0, "%s: Creating object failed",__func__);
        return (FAILED);
    }

    if (esw_98dxc323_obj_p->callin_fvt->esw_print_sw_counter((dev_object_t *)esw_98dxc323_obj_p, cpss_dev, NANOOK_ESW_PORT_NUM) != PASSED) {
         cterr('f',0,"Failed esw_print_sw_counter()");
         goto _exit;
    }

    return (PASSED);

_exit:
    esw_98dxc323_obj_p->base.dev_object_fvt->dev_destroy((dev_object_t **)&esw_98dxc323_obj_p);
    return (FAILED);

}


/******************************************************************************
 *
 * Function   :	diag_esw_phy_port_internal_lpbk_test
 * Description: perform PHY internal loopback test.
 *              host->GE->XCAT3->phy->XCAT3->->GE->host
 * Inputs     :	port_num
 *              bp_port - backplane GE port, can GE_XCAT3_PORT
 * Outputs    : PASSED/FAILED
 *
 ******************************************************************************
 */
static int
diag_esw_phy_port_internal_lpbk_test (int port_num, int bp_port, int speed)
{

    int rc = PASSED, retry_count = 0;
    int cpss_dev = marvell_ac3_cpss_dev_num_nanook;
    uint port_group, port_group_phy_num;
    MAD_DEV * mad_dev;
    MAD_SPEED_MODE target_speed = 0;
	
    dev_98dxc323_object_t esw_98dxc323_obj;
    dev_98dxc323_object_t *esw_98dxc323_obj_p  = &esw_98dxc323_obj;
	
    dev_88e1680_object_t phy_88e1680_obj;
    dev_88e1680_object_t *phy_88e1680_obj_p  = &phy_88e1680_obj;

    /* Create 98dxc323 device driver */
    rc = diag_esw_98dxc323_dev_create(esw_98dxc323_obj_p);
    if (rc == FAILED) {
        cterr('f', 0, "%s: Creating object failed",__func__);
        return (FAILED);
    }

    /* Create 88e1680 device driver */
    rc = diag_phy_88e1680_dev_create(phy_88e1680_obj_p);
    if (rc == FAILED) {
        cterr('f', 0, "%s: Creating object failed",__func__);
        goto _exit_98dxc323;
    }

    retry_count = 0;

retry:
    ifconfig_down_up_eth(inface_lan0p1);
    /* Config port speed */
    if (diag_config_port_speed(cpss_dev, port_num, speed)!= PASSED) {
        cterr('f',0,"Failed to config MAC port speed.");
        goto _exit;
    }

    /* Call 98dxc323 config port pve function */
    if (esw_98dxc323_obj_p->callin_fvt->esw_config_port_pve((dev_object_t *)esw_98dxc323_obj_p, cpss_dev, bp_port, port_num) != PASSED) {
        cterr('f',0,"Failed to configure PVE for port %d", port_num);
	 goto _exit;
    }

    if(speed == SPD_1000MBPS) {
        target_speed = MAD_SPEED_1000M;
    } else if (speed == SPD_100MBPS) {
        target_speed = MAD_SPEED_100M;
    } else if (speed == SPD_10MBPS) {
        target_speed = MAD_SPEED_10M;
    } else {
        cterr('f', 0, "%s: Unsupported speed.\n",__func__);
    }

    /* Call 88e1680 start mac loopback function */
    port_group = (port_num / NANOOK_1680_NUM_PHY_IN_ONE_GRUOP);
    port_group_phy_num = (port_num % NANOOK_1680_NUM_PHY_IN_ONE_GRUOP);
    mad_dev = &phy_dev_88e1680[port_group];
    if (phy_88e1680_obj_p->callin_fvt->start_mac_lpbk((dev_object_t *)phy_88e1680_obj_p, mad_dev, port_group_phy_num, target_speed)!= PASSED) {
        cterr('f',0,"Failed to enable PHY loopback for port %d", port_num);
        goto _exit;
    }


    rc = diag_esw_ext_lpbk_test();
    /* Do SGMII loopback test. */
    if((retry_count == NANOOK_LPBK_RETRY) && (rc == FAILED)) {
        phy_88e1680_obj_p->callin_fvt->dump_phy_reg((dev_object_t *)phy_88e1680_obj_p, mad_dev, port_num);
        goto _exit;
    }

    /* (CSCvn43011) Base on Intel reply there isn't have solution so we add retry here to workaround Denverton loopback issue.*/
    if (rc == FAILED) {
        retry_count++;
        printf("\nWarning this is internal lpbk retry %d time\n", retry_count);
        ifconfig_down_up_eth(inface_lan0p1);

        goto retry;
    }

    /* Call 98dxc323 unconfig port pve function */
    if (esw_98dxc323_obj_p->callin_fvt->esw_unconfig_port_pve((dev_object_t *)esw_98dxc323_obj_p, cpss_dev, bp_port, port_num) != PASSED) {
        cterr('f',0,"Failed to unconfigure PVE for port %d", port_num);
	 goto _exit;
    }

    /* Call 88e1680 phy config function */
    if (phy_88e1680_obj_p->callin_fvt->phy_config((dev_object_t *)phy_88e1680_obj_p, mad_dev, port_group_phy_num)!= PASSED) {
        cterr('f',0,"Failed to init PHY for port %d", port_num);
        goto _exit;
    }

    /* Destroy object */
    phy_88e1680_obj_p->base.dev_object_fvt->dev_destroy((dev_object_t **)&phy_88e1680_obj_p);
    esw_98dxc323_obj_p->base.dev_object_fvt->dev_destroy((dev_object_t **)&esw_98dxc323_obj_p);
    return (PASSED);

 _exit_98dxc323:
    esw_98dxc323_obj_p->base.dev_object_fvt->dev_destroy((dev_object_t **)&esw_98dxc323_obj_p);
    return (FAILED);
	
 _exit:
    phy_88e1680_obj_p->base.dev_object_fvt->dev_destroy((dev_object_t **)&phy_88e1680_obj_p);
    esw_98dxc323_obj_p->base.dev_object_fvt->dev_destroy((dev_object_t **)&esw_98dxc323_obj_p);
    return (FAILED);
}


/******************************************************************************
 *
 * Function   :	diag_esw_phy_port_external_lpbk_test
 * Description: perform external loopback test.
 *              host->GE->XCAT3->phy->loopback connecter->phy->XCAT3->->GE->host
 * Inputs     :	port_num
 *              bp_port - backplane GE port, can GE_XCAT3_PORT
 * Outputs    : PASSED/FAILED
 *
 ******************************************************************************
 */
static int
diag_esw_phy_port_external_lpbk_test (int port_num, int bp_port, int speed)
{
    int rc = PASSED, retry_count = 0;

    int cpss_dev = marvell_ac3_cpss_dev_num_nanook;
    uint port_group, port_group_phy_num;
    MAD_DEV * mad_dev;
    MAD_SPEED_MODE target_speed = 0;
	
    dev_98dxc323_object_t esw_98dxc323_obj;
    dev_98dxc323_object_t *esw_98dxc323_obj_p  = &esw_98dxc323_obj;
	
    dev_88e1680_object_t phy_88e1680_obj;
    dev_88e1680_object_t *phy_88e1680_obj_p  = &phy_88e1680_obj;

    /* Create 98dxc323 device driver */
    rc = diag_esw_98dxc323_dev_create(esw_98dxc323_obj_p);
    if (rc == FAILED) {
        cterr('f', 0, "%s: Creating object failed",__func__);
        return (FAILED);
    }

    /* Create 88e1680 device driver */
    rc = diag_phy_88e1680_dev_create(phy_88e1680_obj_p);
    if (rc == FAILED) {
        cterr('f', 0, "%s: Creating object failed",__func__);
        goto _exit_98dxc323;
    }

    retry_count = 0;

retry:
    /* Config port speed */
    if (diag_config_port_speed(cpss_dev, port_num, speed)!= PASSED) {
        cterr('f',0,"Failed to config MAC port speed.");
        goto _exit;
    }

    /* Call 98dxc323 xcat3 config port pve function */
    if (esw_98dxc323_obj_p->callin_fvt->esw_config_port_pve((dev_object_t *)esw_98dxc323_obj_p, cpss_dev, bp_port, port_num) != PASSED) {
        cterr('f',0,"Failed to configure PVE for port %d", port_num);
	 goto _exit;
    }

    if(speed == SPD_1000MBPS) {
        target_speed = MAD_SPEED_1000M;
    } else if (speed == SPD_100MBPS) {
        target_speed = MAD_SPEED_100M;
    } else if (speed == SPD_10MBPS) {
        target_speed = MAD_SPEED_10M;
    } else {
        cterr('f', 0, "%s: Unsupported speed.\n",__func__);
    }

    port_group = (port_num / NANOOK_1680_NUM_PHY_IN_ONE_GRUOP);
    port_group_phy_num = (port_num % NANOOK_1680_NUM_PHY_IN_ONE_GRUOP);
    mad_dev = &phy_dev_88e1680[port_group];

    /* Call 88e1680 start external loopback function */
    if (phy_88e1680_obj_p->callin_fvt->start_ext_lpbk((dev_object_t *)phy_88e1680_obj_p, mad_dev, port_group_phy_num, target_speed)!= PASSED) {
        cterr('f',0,"Failed to enable external loopback for port %d", port_num);
        goto _exit;
    }

    /* Do SGMII loopback test. */
    rc = diag_esw_ext_lpbk_test();
    if((retry_count == NANOOK_LPBK_RETRY) && (rc == FAILED)) {
        phy_88e1680_obj_p->callin_fvt->dump_phy_reg((dev_object_t *)phy_88e1680_obj_p, mad_dev, port_num);
        goto _exit;
    }
    
    /* (CSCvn43011) Base on Intel reply there isn't have solution so we add retry here to workaround Denverton loopback issue.*/
    if (rc == FAILED) {
        retry_count++;
        printf("\nWarning this is external lpbk retry %d time\n", retry_count);
        ifconfig_down_up_eth(inface_lan0p1);

        goto retry;
    }

    /* Call 98dxc323 xcat3 unconfig port pve function */
    if (esw_98dxc323_obj_p->callin_fvt->esw_unconfig_port_pve((dev_object_t *)esw_98dxc323_obj_p, cpss_dev, bp_port, port_num) != PASSED) {
        cterr('f',0,"Failed to unconfigure PVE for port %d", port_num);
	 goto _exit;
    }

    /* Call 88e1680 phy config function */
    if (phy_88e1680_obj_p->callin_fvt->phy_config((dev_object_t *)phy_88e1680_obj_p, mad_dev, port_group_phy_num)!= PASSED) {
        cterr('f',0,"Failed to init PHY for port %d", port_num);
        goto _exit;
    }

    /* Destroy object */
    phy_88e1680_obj_p->base.dev_object_fvt->dev_destroy((dev_object_t **)&phy_88e1680_obj_p);
    esw_98dxc323_obj_p->base.dev_object_fvt->dev_destroy((dev_object_t **)&esw_98dxc323_obj_p);
    return (PASSED);

 _exit_98dxc323:
    esw_98dxc323_obj_p->base.dev_object_fvt->dev_destroy((dev_object_t **)&esw_98dxc323_obj_p);
    return (FAILED);
	
 _exit:
    phy_88e1680_obj_p->base.dev_object_fvt->dev_destroy((dev_object_t **)&phy_88e1680_obj_p);
    esw_98dxc323_obj_p->base.dev_object_fvt->dev_destroy((dev_object_t **)&esw_98dxc323_obj_p);
    return (FAILED);
}


/******************************************************************************
 *
 * Function   :	diag_esw_internal_lpbk_test
 * Description: perform PHY internal loopback test.
 *              host->GE1->XCAT3->phy->XCAT3->->GE1->host
 * Inputs     :	None
 * Outputs    : PASSED/FAILED
 *
 ******************************************************************************
 */
int
diag_esw_int_lpbk_test_sub (int target_port)
{

    int port_num = NANOOK_ESW_PORT_NUM;	
    int ix, rc = 0;

    int cpss_dev = marvell_ac3_cpss_dev_num_nanook;
    int  total_spd = 0,  spd_ctr = 0,  test_spd = 0;

    dev_98dxc323_object_t esw_98dxc323_obj;
    dev_98dxc323_object_t *esw_98dxc323_obj_p  = &esw_98dxc323_obj;
	
    dev_88e1680_object_t phy_88e1680_obj;
    dev_88e1680_object_t *phy_88e1680_obj_p  = &phy_88e1680_obj;

    char tname[32]; 

    memset(tname, 0, sizeof(tname));
    sprintf(tname, "CPU to Switch PHY Internal loopback");
    testname(tname);
    prpass(testpass, "%s, ", tname);

    /* Create 98dxc323 device driver */
    rc = diag_esw_98dxc323_dev_create(esw_98dxc323_obj_p);
    if (rc == FAILED) {
        cterr('f', 0, "%s: Creating object failed",__func__);
        return (FAILED);
    }

    /* Create 88e1680 device driver */
    rc = diag_phy_88e1680_dev_create(phy_88e1680_obj_p);
    if (rc == FAILED) {
        cterr('f', 0, "%s: Creating object failed",__func__);
        goto _exit_98dxc323;
    }


    /* Call 98dxc323 clear software counter */
    if (esw_98dxc323_obj_p->callin_fvt->esw_clear_sw_counter((dev_object_t *)esw_98dxc323_obj_p, cpss_dev, port_num) != PASSED) {
        cterr('f',0,"Failed clear xcat3 sw counter");
        goto _exit;
    }

    total_spd = sizeof(esw_speed_tbl) / sizeof(int);
    ix = target_port;

        for (spd_ctr = 0; spd_ctr < total_spd; spd_ctr++) {
            test_spd = esw_speed_tbl[spd_ctr];
            prpass(testpass, "Testing switch port%d in %dmbps ",
                             ix, test_spd);

	     /* Call 88e1680 clear PHY counter */
            if (diag_esw_phy_clear_phy_counter() != PASSED) {
                cterr('f',0,"Failed diag_esw_phy_clear_phy_counter()");
                goto _exit;
            }

            if (diag_esw_phy_port_internal_lpbk_test(ix, GE_XCAT3_PORT, test_spd)) {

                cterr('f', 0, "Failed PHY internal loopback test for port %d", ix);
                /* Call 98dxc323 print software counter */ 
                if (esw_98dxc323_obj_p->callin_fvt->esw_print_sw_counter((dev_object_t *)esw_98dxc323_obj_p, cpss_dev, port_num) != PASSED) {
                    cterr('f',0,"Failed esw_print_sw_counter()");
                    goto _exit;
                }
		
                /* Call 88e1680 print PHY counter */
	         if (diag_esw_phy_print_phy_counter()!= PASSED) {
                    cterr('f',0,"Failed print_phy_counter()");
                    goto _exit;
                }

                goto _exit;
	     
            }
		
            if ((NVRAM)->diagflag & D_VERBOSE) {
                /* Call 98dxc323 print software counter */ 
                if (esw_98dxc323_obj_p->callin_fvt->esw_print_sw_counter((dev_object_t *)esw_98dxc323_obj_p, cpss_dev, port_num) != PASSED) {
                    cterr('f',0,"Failed esw_print_sw_counter()");
	             goto _exit;
                }
                /* Call 88e1680 print PHY counter */
	         if (diag_esw_phy_print_phy_counter()!= PASSED) {
                    cterr('f',0,"Failed print_phy_counter()");
                    goto _exit;
                }
           }
        }
#if 0 /* remove to avoid reset esw frequently */
    if (diag_reset_esw_to_default(TRUE) != PASSED) {
        cterr('f', 0, "Failed reset switch ");
        goto _exit;
    }
#endif
    phy_88e1680_obj_p->base.dev_object_fvt->dev_destroy((dev_object_t **)&phy_88e1680_obj_p);
    esw_98dxc323_obj_p->base.dev_object_fvt->dev_destroy((dev_object_t **)&esw_98dxc323_obj_p);
    prcomplete(testpass, errcount, (char *)0);
    return (PASSED);

 _exit_98dxc323:
    esw_98dxc323_obj_p->base.dev_object_fvt->dev_destroy((dev_object_t **)&esw_98dxc323_obj_p);
    prcomplete(testpass, errcount, (char *)0);
    return (FAILED);
	
 _exit:
    phy_88e1680_obj_p->base.dev_object_fvt->dev_destroy((dev_object_t **)&phy_88e1680_obj_p);
    esw_98dxc323_obj_p->base.dev_object_fvt->dev_destroy((dev_object_t **)&esw_98dxc323_obj_p);
    return (FAILED);
}


/******************************************************************************
 *
 * Function   :	diag_esw_external_lpbk_test
 * Description: perform external loopback test.
 *              host->GE1->XCAT3->phy->loopback connecter->phy->XCAT3->->GE1->host
 * Inputs     :	None
 * Outputs    : PASSED/FAILED
 *
 ******************************************************************************
 */
int
diag_esw_ext_lpbk_test_sub (int target_port)
{
    int port_num = NANOOK_ESW_PORT_NUM;
    int ix, rc = 0;
    int spd_mode;

    int cpss_dev = marvell_ac3_cpss_dev_num_nanook;
    int  total_spd = 0,  spd_ctr = 0,  test_spd = 0;
	
    dev_98dxc323_object_t esw_98dxc323_obj;
    dev_98dxc323_object_t *esw_98dxc323_obj_p  = &esw_98dxc323_obj;
	
    dev_88e1680_object_t phy_88e1680_obj;
    dev_88e1680_object_t *phy_88e1680_obj_p  = &phy_88e1680_obj;

    char tname[32]; 

    memset(tname, 0, sizeof(tname));
    sprintf(tname, "CPU to Switch PHY External loopback");
    testname(tname);
    prpass(testpass, "%s, ", tname);
	
    //boolean value = PASSED;

    /*
     * if D_EXT_LOOPBACK is OFF, then just return
     */
    if ((NVRAM)->diagflag & D_EXT_LOOPBACK) {
        return (PASSED);
    }

    if (!(diagflag_xram & D_MIN_TEST_TIME)) {
        spd_mode = DEV_ESW_SPD_1000;
    } else {
        spd_mode = DEV_ESW_SPD_10;
    }

    /* Create 98dxc323 device driver */
    rc = diag_esw_98dxc323_dev_create(esw_98dxc323_obj_p);
    if (rc == FAILED) {
        cterr('f', 0, "%s: Creating object failed",__func__);
        return (FAILED);
    }

    /* Create 88e1680 device driver */
    rc = diag_phy_88e1680_dev_create(phy_88e1680_obj_p);
    if (rc == FAILED) {
        cterr('f', 0, "%s: Creating object failed",__func__);
        goto _exit_98dxc323;
    }

#if 0
    /* reset PHY before the test */
    if (phy_88e1680_obj_p->callin_fvt->reset_phy((dev_object_t *)phy_88e1680_obj_p, mad_dev)!= PASSED) {
        cterr('f',0,"Failed reset_phy()");
        goto _exit;
    }
#endif

    /* esw_clear_sw_counter */
    if (esw_98dxc323_obj_p->callin_fvt->esw_clear_sw_counter((dev_object_t *)esw_98dxc323_obj_p, cpss_dev, port_num) != PASSED) {
        cterr('f',0,"Failed clear xcat3 sw counter");
	 goto _exit;
    }

    total_spd = sizeof(esw_speed_tbl) / sizeof(int);
    ix = target_port;

    //for (ix = 0; ix < port_num; ix++) {
        for (spd_ctr = spd_mode; spd_ctr < total_spd; spd_ctr++) {

	     test_spd = esw_speed_tbl[spd_ctr];
            prpass(testpass, "Testing switch port%d in %dmbps ",
                             ix, test_spd);

            /* clear phy counters */
            if (diag_esw_phy_clear_phy_counter() != PASSED) {
                cterr('f',0,"Failed diag_esw_phy_clear_phy_counter()");
                goto _exit;
            }

            if (diag_esw_phy_port_external_lpbk_test(ix, GE_XCAT3_PORT, test_spd)) {
                if (esw_98dxc323_obj_p->callin_fvt->esw_print_sw_counter((dev_object_t *)esw_98dxc323_obj_p, cpss_dev, port_num) != PASSED) {
                    cterr('f',0,"Failed esw_print_sw_counter()");
	             goto _exit;
                }
	         if (diag_esw_phy_print_phy_counter()!= PASSED) {
                    cterr('f',0,"Failed print_phy_counter()");
                    goto _exit;
                }
                cterr('f', 0, "Failed external loopback test for port %d", ix);
                goto _exit;
            }
            if ((NVRAM)->diagflag & D_VERBOSE) {
                if (esw_98dxc323_obj_p->callin_fvt->esw_print_sw_counter((dev_object_t *)esw_98dxc323_obj_p, cpss_dev, port_num) != PASSED) {
                    cterr('f',0,"Failed esw_print_sw_counter()");
	             goto _exit;
                }
	         if (diag_esw_phy_print_phy_counter()!= PASSED) {
                    cterr('f',0,"Failed print_phy_counter()");
                    goto _exit;
                }
            }
        }
    //}

    phy_88e1680_obj_p->base.dev_object_fvt->dev_destroy((dev_object_t **)&phy_88e1680_obj_p);
    esw_98dxc323_obj_p->base.dev_object_fvt->dev_destroy((dev_object_t **)&esw_98dxc323_obj_p);
    prcomplete(testpass, errcount, (char *)0);
    return (PASSED);
	
 _exit_98dxc323:
    esw_98dxc323_obj_p->base.dev_object_fvt->dev_destroy((dev_object_t **)&esw_98dxc323_obj_p);
    prcomplete(testpass, errcount, (char *)0);
    return (FAILED);
	
 _exit:
    phy_88e1680_obj_p->base.dev_object_fvt->dev_destroy((dev_object_t **)&phy_88e1680_obj_p);
    esw_98dxc323_obj_p->base.dev_object_fvt->dev_destroy((dev_object_t **)&esw_98dxc323_obj_p);
    prcomplete(testpass, errcount, (char *)0);
    return (FAILED);
}


/******************************************************************************
 *
 * Function   :	diag_esw_snake_lpbk_test
 * Description: perform snake loopback test.
 *              CPU MAC -> xCat3 PORT 24 -> SWITCH PORT Snake -> xCat3 PORT 24 -> CPU MAC
 * Inputs     :	
 *              direction - forward (CPU MAC -> 0 -> 1 ... -> 22 -> 23 -> CPU MAC) or 
 *                              backward (CPU MAC -> 23 -> 22 ... -> 1 -> 0 -> CPU MAC) test.
 * Outputs    : PASSED/FAILED
 *
 ******************************************************************************
 */
static int
diag_esw_snake_lpbk_test (int direction, int speed)
{
    int rc = PASSED;
    int ix, jx, src_port, dst_port;
    uint port_num, port_link_status, link_up_num, link_down_num;
    uint pair_link_record[NANOOK_SNAKE_PHY_PAIR_NUM];

    int cpss_dev = marvell_ac3_cpss_dev_num_nanook;
    uint port_group, port_group_phy_num;
    MAD_DEV * mad_dev;
    MAD_SPEED_MODE target_speed = 0;
	
    dev_98dxc323_object_t esw_98dxc323_obj;
    dev_98dxc323_object_t *esw_98dxc323_obj_p  = &esw_98dxc323_obj;
	
    dev_88e1680_object_t phy_88e1680_obj;
    dev_88e1680_object_t *phy_88e1680_obj_p  = &phy_88e1680_obj;

    /* Create 98dxc323 device driver */
    rc = diag_esw_98dxc323_dev_create(esw_98dxc323_obj_p);
    if (rc == FAILED) {
        cterr('f', 0, "%s: Creating object failed",__func__);
        return (FAILED);
    }

    /* Create 88e1680 device driver */
    rc = diag_phy_88e1680_dev_create(phy_88e1680_obj_p);
    if (rc == FAILED) {
        cterr('f', 0, "%s: Creating object failed",__func__);
        goto _exit_98dxc323;
    }

    for(ix = 0; ix < NANOOK_ESW_PORT_NUM; ix++)
    {
        /* Power off port */
        if (diag_port_power_control(cpss_dev, ix, ESW_PORT_PWR_DOWN)!= PASSED) {
            cterr('f',0,"Failed to power off port %d.", ix);
            goto _exit;
        }
    }

    if(speed == SPD_1000MBPS) {
        target_speed = MAD_SPEED_1000M;
    } else if (speed == SPD_100MBPS) {
        target_speed = MAD_SPEED_100M;
    } else if (speed == SPD_10MBPS) {
        target_speed = MAD_SPEED_10M;
    } else {
        cterr('f', 0, "%s: Unsupported speed.\n",__func__);
    }

    for(ix = 0; ix < NANOOK_ESW_PORT_NUM; ix++) {
        port_group = (ix / NANOOK_1680_NUM_PHY_IN_ONE_GRUOP);
        port_group_phy_num = (ix % NANOOK_1680_NUM_PHY_IN_ONE_GRUOP);
        mad_dev = &phy_dev_88e1680[port_group];

        /* Call 88e1680 start external loopback function */
        if (phy_88e1680_obj_p->callin_fvt->phy_force_speed((dev_object_t *)phy_88e1680_obj_p, mad_dev, port_group_phy_num, target_speed)!= PASSED) {
            cterr('f',0,"Failed to force speed for port %d", ix);
            goto _exit;
        }
    }

    for(ix = 0; ix < NANOOK_ESW_PORT_NUM; ix++)
    {
        /* Power up port */
        if (diag_port_power_control(cpss_dev, ix, ESW_PORT_PWR_UP)!= PASSED) {
            cterr('f',0,"Failed to power up port %d.", ix);
            goto _exit;
        }
    }
	
    for(ix = 0; ix < NANOOK_ESW_PORT_NUM; ix++)
    {
        /* Config port speed */
        if (diag_config_port_speed(cpss_dev, ix, speed)!= PASSED) {
            cterr('f',0,"Failed to config MAC port speed for port %d.", ix);
            goto _exit;
        }
    }

    if (direction == NANOOK_SNAKE_TEST_FORWARD) {

        for (ix = 0; ix < NANOOK_SNAKE_PAIR_NUM; ix++) {

            src_port = esw_snake_foward_map_tbl[ix];
            dst_port = esw_snake_foward_pair_map_tbl[ix];;

            /* Call 98dxc323 xcat3 config port pve function */
	     //printf("Config port pve for src_port:%d  dst_port:%d\n", src_port, dst_port);
            if (esw_98dxc323_obj_p->callin_fvt->esw_config_port_pve_single_direction((dev_object_t *)esw_98dxc323_obj_p, cpss_dev, src_port, dst_port) != PASSED) {
                cterr('f',0,"Failed to configure PVE for src_port:%d and dst_port:%d\n", src_port, dst_port);
	         goto _exit;
            }
	  
        }

    }else if (direction == NANOOK_SNAKE_TEST_BACKWARD) {

        for (ix = 0; ix < NANOOK_SNAKE_PAIR_NUM; ix++) {

            src_port = esw_snake_backward_map_tbl[ix];
            dst_port = esw_snake_backward_pair_map_tbl[ix];;

            /* Call 98dxc323 xcat3 config port pve function */
	     //printf("Config port pve for src_port:%d  dst_port:%d\n", src_port, dst_port);
            if (esw_98dxc323_obj_p->callin_fvt->esw_config_port_pve_single_direction((dev_object_t *)esw_98dxc323_obj_p, cpss_dev, src_port, dst_port) != PASSED) {
                cterr('f',0,"Failed to configure PVE for src_port:%d and dst_port:%d\n", src_port, dst_port);
	         goto _exit;
            }
	  
        }

    } else {
    
        cterr('f', 0, "%s: Unsupported Snake test mode.",__func__);
        goto _exit;
    }

    for (ix = 0; ix < NANOOK_LINK_UP_TOUT; ix ++) {
        link_up_num = 0;
	 link_down_num =0;
        for (jx = 0; jx < NANOOK_SNAKE_PHY_PAIR_NUM; jx ++ ) {
            port_num = esw_snake_pair_map_tbl[jx];
	     pair_link_record[jx] = FALSE;
	     diag_esw_check_phy_link_up(port_num, &port_link_status);
	     if (port_link_status == TRUE) {
                link_up_num ++;
		  pair_link_record[jx] = TRUE;
	     } else {
	         link_down_num ++;
	     }
        }

        if ((NVRAM)->diagflag & D_VERBOSE) {
	     for (jx = 0; jx < NANOOK_SNAKE_PHY_PAIR_NUM; jx ++ ) {
	         printf("%dms, pair%d:port%d and port%d, status:%d\n", (ix * ESW_WAIT_100MS), jx, esw_snake_pair_map_tbl[jx], esw_snake_pair_map_tbl[jx] + 1, pair_link_record[jx]);	 	
	     }
        }

        if (link_up_num == NANOOK_SNAKE_PHY_PAIR_NUM) {
             if ((NVRAM)->diagflag & D_VERBOSE) {
	         printf("DBG:All PHY pair link up:%d at time %d ms.\n", link_up_num, (ix * ESW_WAIT_100MS));
             }
	     break;
	 } else {
            if ((NVRAM)->diagflag & D_VERBOSE) {
	         printf("DBG:PHY pair link up:%d, and there is still num of PHY pair link down:%d at time %d ms.\n", link_up_num, link_down_num,  (ix * ESW_WAIT_100MS));    
	     }
	 }
        msleep(ESW_WAIT_100MS); 
    }

    if (ix == NANOOK_LINK_UP_TOUT) {
        cterr('f',0,"PHY pair link up timeout failed, there is still num of PHY pair link down:%d ...\n", link_down_num);
	 for (jx = 0; jx < NANOOK_SNAKE_PHY_PAIR_NUM; jx ++ ) {
	     if (pair_link_record[jx] == FALSE) {
                printf("port %d and port %d are not link up after 10000 ms.\n", esw_snake_pair_map_tbl[jx], esw_snake_pair_map_tbl[jx] + 1);
	     }
            	 	
	 }
	 goto _exit;
    }

    if (ix > 50) {
        printf("DBG: Over 5000 ms, all PHY pair link up:%d at time %d ms.\n", link_up_num, (ix * ESW_WAIT_100MS));
    }

    /*All port link up, then check if the expected speed.*/
    for (ix = 0; ix < NANOOK_ESW_PORT_NUM; ix ++ ) {
        if (diag_esw_check_phy_speed(ix, speed) != PASSED) {
	     cterr('f',0,"Wrong speed for port %d", ix);
            goto _exit;
	 }
    }

    diag_esw_phy_clear_phy_counter();

    /* Do SGMII loopback test. */
    if (diag_esw_ext_lpbk_test()) {
	diag_esw_phy_print_phy_counter();
       goto _exit;
    }

    diag_esw_phy_print_phy_counter();

    if (direction == NANOOK_SNAKE_TEST_FORWARD) {

        for (ix = 0; ix < NANOOK_SNAKE_PAIR_NUM; ix++) {

            src_port = esw_snake_foward_map_tbl[ix];
            dst_port = esw_snake_foward_pair_map_tbl[ix];;

             /* Call 98dxc323 xcat3 unconfig port pve function */
	      //printf("Unconfig port pve for src_port:%d  dst_port:%d\n", src_port, dst_port);
             if (esw_98dxc323_obj_p->callin_fvt->esw_unconfig_port_pve_single_direction((dev_object_t *)esw_98dxc323_obj_p, cpss_dev, src_port, dst_port) != PASSED) {
                 cterr('f',0,"Failed to unconfigure PVE for src_port:%d and dst_port:%d\n", src_port, dst_port);
	          goto _exit;
             }
	  
        }

    }else if (direction == NANOOK_SNAKE_TEST_BACKWARD) {

        for (ix = 0; ix < NANOOK_SNAKE_PAIR_NUM; ix++) {

            src_port = esw_snake_backward_map_tbl[ix];
            dst_port = esw_snake_backward_pair_map_tbl[ix];;

             /* Call 98dxc323 xcat3 unconfig port pve function */
	      //printf("Unconfig port pve for src_port:%d  dst_port:%d\n", src_port, dst_port);
             if (esw_98dxc323_obj_p->callin_fvt->esw_unconfig_port_pve_single_direction((dev_object_t *)esw_98dxc323_obj_p, cpss_dev, src_port, dst_port) != PASSED) {
                 cterr('f',0,"Failed to unconfigure PVE for src_port:%d and dst_port:%d\n", src_port, dst_port);
	          goto _exit;
             }
	  
        }

    } else {
    
        cterr('f', 0, "%s: Unsupported Snake test mode.",__func__);
        goto _exit;
    }
#if 0 /* remove to avoid reset esw frequently */
    if (diag_reset_esw_to_default(TRUE) != PASSED) {
        /* enhance error msg: error */
        cterr('f', 0, "%s:%d: Fail to reset ESW", 
                      __FUNCTION__, __LINE__);
        prcomplete(testpass, errcount, (char *)0);
        goto _exit;
    }
#endif 

    /* Destroy object */
    phy_88e1680_obj_p->base.dev_object_fvt->dev_destroy((dev_object_t **)&phy_88e1680_obj_p);
    esw_98dxc323_obj_p->base.dev_object_fvt->dev_destroy((dev_object_t **)&esw_98dxc323_obj_p);
    return (PASSED);

 _exit_98dxc323:
    esw_98dxc323_obj_p->base.dev_object_fvt->dev_destroy((dev_object_t **)&esw_98dxc323_obj_p);
    return (FAILED);
	
 _exit:
    phy_88e1680_obj_p->base.dev_object_fvt->dev_destroy((dev_object_t **)&phy_88e1680_obj_p);
    esw_98dxc323_obj_p->base.dev_object_fvt->dev_destroy((dev_object_t **)&esw_98dxc323_obj_p);
    return (FAILED);
}


/******************************************************************************
 *
 * Function   :	diag_esw_snake_test
 * Description: perform snake test.
 *              host->GE1->XCAT3->phy->loopback connecter->phy->XCAT3->->GE1->host
 * Inputs     :	None
 * Outputs    : PASSED/FAILED
 *
 ******************************************************************************
 */
int
diag_esw_snake_test (void)
{
    int port_num = NANOOK_ESW_PORT_NUM;
    int rc = 0;

    int cpss_dev = marvell_ac3_cpss_dev_num_nanook;
    int total_spd = 0, spd_ctr = 0, test_spd = 0, spd_mode;
	
    dev_98dxc323_object_t esw_98dxc323_obj;
    dev_98dxc323_object_t *esw_98dxc323_obj_p  = &esw_98dxc323_obj;
	
    dev_88e1680_object_t phy_88e1680_obj;
    dev_88e1680_object_t *phy_88e1680_obj_p  = &phy_88e1680_obj;

    char tname[32]; 

    memset(tname, 0, sizeof(tname));
    sprintf(tname, "Snake");
    testname(tname);
    prpass(testpass, "%s, ", tname);
	
    //boolean value = PASSED;

    /*
     * if D_EXT_LOOPBACK is OFF, then just return
     */
    if ((NVRAM)->diagflag & D_EXT_LOOPBACK) {
        return (PASSED);
    }

    if (!(diagflag_xram & D_MIN_TEST_TIME)) {
        spd_mode = DEV_ESW_SPD_1000;
    } else {
        spd_mode = DEV_ESW_SPD_10;
    }

    if (diag_reset_esw_to_default(0) == FAILED) {
        cterr('f', 0, "%s: Reset ESW to default failed.",__func__);
        return (FAILED);
    }

    /* Create 98dxc323 device driver */
    rc = diag_esw_98dxc323_dev_create(esw_98dxc323_obj_p);
    if (rc == FAILED) {
        cterr('f', 0, "%s: Creating object failed",__func__);
        return (FAILED);
    }

    /* Create 88e1680 device driver */
    rc = diag_phy_88e1680_dev_create(phy_88e1680_obj_p);
    if (rc == FAILED) {
        cterr('f', 0, "%s: Creating object failed",__func__);
        goto _exit_98dxc323;
    }

    /* esw_clear_sw_counter */
    if (esw_98dxc323_obj_p->callin_fvt->esw_clear_sw_counter((dev_object_t *)esw_98dxc323_obj_p, cpss_dev, port_num) != PASSED) {
        cterr('f',0,"Failed clear xcat3 sw counter");
	 goto _exit;
    }

    total_spd = sizeof(esw_speed_tbl) / sizeof(int);

    for (spd_ctr = spd_mode; spd_ctr < total_spd; spd_ctr++) {

	     test_spd = esw_speed_tbl[spd_ctr];

            /* clear phy counters */
            if (diag_esw_phy_clear_phy_counter() != PASSED) {
                cterr('f',0,"Failed diag_esw_phy_clear_phy_counter()");
                goto _exit;
            }

            prpass(testpass, "Testing Snake Foward in %dmbps ", test_spd);
            if (diag_esw_snake_lpbk_test(NANOOK_SNAKE_TEST_FORWARD, test_spd)) {
		  cterr('f',0,"Failed to Snake Forward Test in %dmbps", test_spd);
                if (esw_98dxc323_obj_p->callin_fvt->esw_print_sw_counter((dev_object_t *)esw_98dxc323_obj_p, cpss_dev, port_num) != PASSED) {
                    cterr('f',0,"Failed esw_print_sw_counter()");
	             goto _exit;
                }
	         if (diag_esw_phy_print_phy_counter()!= PASSED) {
                    cterr('f',0,"Failed print_phy_counter()");
                    goto _exit;
                }
                goto _exit;

            }

            prpass(testpass, "Testing Snake Backward in %dmbps ", test_spd);
            if (diag_esw_snake_lpbk_test(NANOOK_SNAKE_TEST_BACKWARD, test_spd)) {
                cterr('f',0,"Failed to Snake Backward Test in %dmbps", test_spd);
                if (esw_98dxc323_obj_p->callin_fvt->esw_print_sw_counter((dev_object_t *)esw_98dxc323_obj_p, cpss_dev, port_num) != PASSED) {
                    cterr('f',0,"Failed esw_print_sw_counter()");
	             goto _exit;
                }
	         if (diag_esw_phy_print_phy_counter()!= PASSED) {
                    cterr('f',0,"Failed print_phy_counter()");
                    goto _exit;
                }
		  goto _exit;

            }

            if ((NVRAM)->diagflag & D_VERBOSE) {
                if (esw_98dxc323_obj_p->callin_fvt->esw_print_sw_counter((dev_object_t *)esw_98dxc323_obj_p, cpss_dev, port_num) != PASSED) {
                    cterr('f',0,"Failed esw_print_sw_counter()");
	             goto _exit;
                }
	         if (diag_esw_phy_print_phy_counter()!= PASSED) {
                    cterr('f',0,"Failed print_phy_counter()");
                    goto _exit;
                }
            }
    }
	
    phy_88e1680_obj_p->base.dev_object_fvt->dev_destroy((dev_object_t **)&phy_88e1680_obj_p);
    esw_98dxc323_obj_p->base.dev_object_fvt->dev_destroy((dev_object_t **)&esw_98dxc323_obj_p);
    prcomplete(testpass, errcount, (char *)0);
    return (PASSED);
	
 _exit_98dxc323:
    esw_98dxc323_obj_p->base.dev_object_fvt->dev_destroy((dev_object_t **)&esw_98dxc323_obj_p);
    prcomplete(testpass, errcount, (char *)0);
    return (FAILED);
	
 _exit:
    phy_88e1680_obj_p->base.dev_object_fvt->dev_destroy((dev_object_t **)&phy_88e1680_obj_p);
    esw_98dxc323_obj_p->base.dev_object_fvt->dev_destroy((dev_object_t **)&esw_98dxc323_obj_p);
    prcomplete(testpass, errcount, (char *)0);
    return (FAILED);
}


/******************************************************************************
 *
 * Function   :	diag_esw_all_register_test
 * Description: perform xCAT3 register test.
 *              
 * Inputs     :	None
 *              
 * Outputs    : PASSED/FAILED
 *
 ******************************************************************************
 */
int
diag_esw_xcat3_all_register_test (void)
{

    int rc = PASSED;
    int cpss_dev = marvell_ac3_cpss_dev_num_nanook;
	
    dev_98dxc323_object_t esw_98dxc323_obj;
    dev_98dxc323_object_t *esw_98dxc323_obj_p  = &esw_98dxc323_obj;

    char tname[32]; 

    memset(tname, 0, sizeof(tname));
    sprintf(tname, "xCat3 All Register");
    testname(tname);
    prpass(testpass, "%s, ", tname);


    /* Create 98dxc323 device driver */
    rc = diag_esw_98dxc323_dev_create(esw_98dxc323_obj_p);
    if (rc == FAILED) {
        cterr('f', 0, "%s: Creating object failed",__func__);
        return (FAILED);
    }

    /* Call 98dxc323 xcat3 all register test function */
    if (esw_98dxc323_obj_p->callin_fvt->esw_xcat3_all_reg_test((dev_object_t *)esw_98dxc323_obj_p, cpss_dev) != PASSED) {
        cterr('f',0,"Failed to test xCat3 all register test");
	 goto _exit;
    }

    esw_98dxc323_obj_p->base.dev_object_fvt->dev_destroy((dev_object_t **)&esw_98dxc323_obj_p);
    prcomplete(testpass, errcount, (char *)0);
    return(PASSED);
	
 _exit:
    esw_98dxc323_obj_p->base.dev_object_fvt->dev_destroy((dev_object_t **)&esw_98dxc323_obj_p);
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
int
diag_esw_phy_register_test (void)
{

    int ix, rc = 0;
    uint port_group, port_group_phy_num;
    MAD_DEV * mad_dev;
	
    dev_88e1680_object_t phy_88e1680_obj;
    dev_88e1680_object_t *phy_88e1680_obj_p  = &phy_88e1680_obj;

    char tname[32]; 

    memset(tname, 0, sizeof(tname));
    sprintf(tname, "PHY Register");
    testname(tname);
    prpass(testpass, "%s, ", tname);

    /* Create 88e1680 device driver */
    rc = diag_phy_88e1680_dev_create(phy_88e1680_obj_p);
    if (rc == FAILED) {
        cterr('f', 0, "%s: Creating object failed",__func__);
        goto _exit;
    }

    for (ix = 0; ix < NANOOK_ESW_PORT_NUM; ix++) {
	 port_group = (ix / NANOOK_1680_NUM_PHY_IN_ONE_GRUOP);
        port_group_phy_num = (ix % NANOOK_1680_NUM_PHY_IN_ONE_GRUOP);
        mad_dev = &phy_dev_88e1680[port_group];
       /* Call 88e1680 PHY register test function */
        if (phy_88e1680_obj_p->callin_fvt->phy_reg_test((dev_object_t *)phy_88e1680_obj_p, mad_dev, port_group_phy_num)!= PASSED) {
            cterr('f',0,"Failed to test phy register for port %d", ix);
            goto _exit;
        }
    }

    phy_88e1680_obj_p->base.dev_object_fvt->dev_destroy((dev_object_t **)&phy_88e1680_obj_p);
    prcomplete(testpass, errcount, (char *)0);
    return(PASSED);
	
 _exit:
    phy_88e1680_obj_p->base.dev_object_fvt->dev_destroy((dev_object_t **)&phy_88e1680_obj_p);
    prcomplete(testpass, errcount, (char *)0);
    return (FAILED);
}


/******************************************************************************
 *
 * Function   :	diag_esw_xcat3_intr_test
 * Description: perform xCAT3 interrupt test.
 *              
 * Inputs     :	None
 *              
 * Outputs    : PASSED/FAILED
 *
 ******************************************************************************
 */
int
diag_esw_xcat3_intr_test (void)
{

    int ix, rc = PASSED;
    int cpss_dev = marvell_ac3_cpss_dev_num_nanook;
    uint reg_addr, intr_sts = 0;
    uint data;
	
    dev_98dxc323_object_t esw_98dxc323_obj;
    dev_98dxc323_object_t *esw_98dxc323_obj_p  = &esw_98dxc323_obj;

    char tname[32]; 

    memset(tname, 0, sizeof(tname));
    sprintf(tname, "xCat3 Interrupt");
    testname(tname);
    prpass(testpass, "%s, ", tname);

    /* Create 98dxc323 device driver */
    rc = diag_esw_98dxc323_dev_create(esw_98dxc323_obj_p);
    if (rc == FAILED) {
        cterr('f', 0, "%s: Creating object failed",__func__);
        return (FAILED);
    }

    /* Call 98dxc323 xcat3 generate interrupt function */
    if (esw_98dxc323_obj_p->callin_fvt->esw_xcat3_gen_int((dev_object_t *)esw_98dxc323_obj_p, cpss_dev) != PASSED) {
        cterr('f',0,"Failed to generate xCat3 interrupt");
	 goto _exit;
    }

    reg_addr = DASH_FPGA_MISCELLANEOUS_INT_STS_REG;
    /* Check if FPGA senses the interrupt coming from the xCAT3 */
    for (ix = 0; ix < GEPHY_INT_TIMEOUT; ix++) {
        rc = dash_fpga_reg_read(reg_addr, &data);
        if (rc != PASSED) {
            cterr('f', 0, "Read FPGA Value Fails");
	     goto _exit;
        }

        printf("FPGA data 0x%x\n", data);
 
        intr_sts = ((data & MISC_INT_AC3_INT_STS) >> MISC_INT_AC3_INT_BIT);   


        if (intr_sts == MISC_INT_AC3_INT_PENDING) {
            break;
        }
        msleep (SLEEP_100);
    }

    if (ix == GEPHY_INT_TIMEOUT) {
        cterr('f', 0, "Interrupt is not detected for 88E1680 Interrupt %d ", ix);
        goto _exit;
    }

    /* Call 98dxc323 xcat3 generate interrupt function */
    if (esw_98dxc323_obj_p->callin_fvt->esw_xcat3_clear_int((dev_object_t *)esw_98dxc323_obj_p, cpss_dev) != PASSED) {
        cterr('f',0,"Failed to clear xCat3 interrupt");
	 goto _exit;
    }

    esw_98dxc323_obj_p->base.dev_object_fvt->dev_destroy((dev_object_t **)&esw_98dxc323_obj_p);
    prcomplete(testpass, errcount, (char *)0);
    return(PASSED);
	
 _exit:
    esw_98dxc323_obj_p->base.dev_object_fvt->dev_destroy((dev_object_t **)&esw_98dxc323_obj_p);
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
int
diag_esw_phy_intr_test (void)
{

    int test_port[NANOOK_PHY_INTR_TEST_PORT_NUM] = 
		{NANOOK_PHY_INTR_TEST_PORT_0 , NANOOK_PHY_INTR_TEST_PORT_8, NANOOK_PHY_INTR_TEST_PORT_16};
    int port;
    int ix, jx, rc = 0;
    uint port_group, port_group_phy_num;
    MAD_DEV * mad_dev;
    uint reg_addr, intr_sts = 0;
    uint data;
	
    dev_88e1680_object_t phy_88e1680_obj;
    dev_88e1680_object_t *phy_88e1680_obj_p  = &phy_88e1680_obj;

    char tname[32]; 

    memset(tname, 0, sizeof(tname));
    sprintf(tname, "PHY Interrupt");
    testname(tname);
    prpass(testpass, "%s, ", tname);

    /* Create 88e1680 device driver */
    rc = diag_phy_88e1680_dev_create(phy_88e1680_obj_p);
    if (rc == FAILED) {
        cterr('f', 0, "%s: Creating object failed",__func__);
        goto _exit;
    }

    for (ix = 0; ix < NANOOK_PHY_INTR_TEST_PORT_NUM; ix++) {
        port = test_port[ix];
	 port_group = (port / NANOOK_1680_NUM_PHY_IN_ONE_GRUOP);
        port_group_phy_num = (port % NANOOK_1680_NUM_PHY_IN_ONE_GRUOP);
        mad_dev = &phy_dev_88e1680[port_group];
        /* Call 88e1680 PHY interrupt test function */
        if (phy_88e1680_obj_p->callin_fvt->gen_int((dev_object_t *)phy_88e1680_obj_p, mad_dev, port_group_phy_num)!= PASSED) {
            cterr('f',0,"Failed to generate PHY interrupt for 88E1680 Interrupt %d ", ix);
            goto _exit;
        } 

        reg_addr = DASH_FPGA_MISCELLANEOUS_INT_STS_REG;
        /* Check if CPU senses the interrupt coming from the PHY */
        for (jx = 0; jx < GEPHY_INT_TIMEOUT; jx++) {
            rc = dash_fpga_reg_read(reg_addr, &data);
            if (rc != PASSED) {
                cterr('f', 0, "Read FPGA Value Fails");
		  goto _exit;
            }
 
            if (ix == 0) {
	         intr_sts = ((data & MISC_INT_88E1680_PHY0_INT_STS) >> MISC_INT_88E1680_PHY0_INT_BIT);   
            } else if (ix == 1) {
                intr_sts = ((data & MISC_INT_88E1680_PHY1_INT_STS) >> MISC_INT_88E1680_PHY1_INT_BIT);   
            } else {
                intr_sts = ((data & MISC_INT_88E1680_PHY2_INT_STS) >> MISC_INT_88E1680_PHY2_INT_BIT);   
            }

            if (intr_sts == MISC_INT_88E1680_INT_PENDING) {
                break;
            }
            msleep (SLEEP_100);
        }

        if (jx == GEPHY_INT_TIMEOUT) {
            cterr('f', 0, "Interrupt is not detected for 88E1680 Interrupt %d ", ix);
            goto _exit;
        }

	 if (phy_88e1680_obj_p->callin_fvt->clear_int((dev_object_t *)phy_88e1680_obj_p, mad_dev, port_group_phy_num)!= PASSED) {
            cterr('f',0,"Failed to clear PHY interrupt for 88E1680 Interrupt %d ", ix);
            goto _exit;
        } 
    }

 
    phy_88e1680_obj_p->base.dev_object_fvt->dev_destroy((dev_object_t **)&phy_88e1680_obj_p);
    prcomplete(testpass, errcount, (char *)0);
    return(PASSED);
	
 _exit:
    phy_88e1680_obj_p->base.dev_object_fvt->dev_destroy((dev_object_t **)&phy_88e1680_obj_p);
    return (FAILED);
}
 
 
/*-------------------------------------------------
 * $Log: diag_esw_test.c,v $
 * Revision 1.3  2020/04/20 02:28:24  lucywang
 *
 * 1. Fixed unplug/plug NIM module dynamically issue and added NIM cookie
 * 2. Added to support NIM Prince
 * 3. (CSCvn43011) add retry workaround for Deverton issue
 * 4. add debug message and set default value to seneors
 * 5. Reverted Register value of temp/press snsr after test
 * 6. Bumped up version to 1.0.2
 *
 * Revision 1.2  2019/12/11 10:10:28  lucywang
 * Merged Nanook to main trunk
 *
 *
 * $Endlog$
 *-------------------------------------------------
 */
