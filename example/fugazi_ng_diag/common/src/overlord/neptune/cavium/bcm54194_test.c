/* $Id: bcm54194_test.c,v 1.3 2018/10/03 09:53:57 meho Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/overlord/neptune/cavium/bcm54194_test.c,v $
*-----------------------------------------------------------------------------
* bcm82752_test.c - Diags Test for BCM GE PHY bcm54194.
*
* June 2016, Mecca Ho
*
* Copyright (c) 2016-2018 by Cisco Systems, Inc.
* All rights reserved.
*-----------------------------------------------------------------------------
*/
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <linux/types.h>
#include <linux/socket.h>
#include <sys/ioctl.h>
#include <linux/if_ether.h>
#include <linux/ip.h>
#include <linux/udp.h>
#include <net/if.h>
#include <netinet/in.h>
#include <linux/sockios.h>
#include <linux/mii.h>

#include "defs.h"
#include "types.h"
#include "proto.h"
#include "common.h"
#include "common_utils.h"
#include "monitor.h"
#include "menu.h"
#include "nvmonvars.h"
#include "error.h"
#include "router_if.h"
#include "sgmii_defs.h"
#include "cvmx.h"
#include "ethernet.h"
#include "linux_eth.h"
#include "platform_eth.h"
#include "cvmx-mdio.h"
#include "ethernet.h"  /* for SFPx port definition */
#include "dash_fpga.h" /* for get SFP ctrl reg */  
#include "queryflags.h" /* for query user functions */  
#include "platform_sfp_cookie.h"
#include "bcm54194_api.h"
#include "platform_ext_lpbk.h"
#include "platform_i2c.h"

#define MF_1    (MF_CONTINUOUS | MF_DOGRP)
#define MF_2    (MF_1 | MF_DOALL)
#define MF_3    (MF_2 | MF_SHOW_ERRCOUNT)
#define MF_4    (MF_1 | MF_SHOW_ERRCOUNT)

extern int is_item_available(void);
extern int ge_phy_ext_lpbk_test(int eth_num, int speed, int lpbkmode);

static int BCM541xx_utility (int);
static int macsec_test_main (int);
static int ge_phy_ptp1588_test_main (int);
static void neptune_stop_lnx_polling (void);
static void read_bcm541xx_ieee_reg (void);
static void write_bcm541xx_ieee_reg (void);
static void read_bcm541xx_rdb_reg (void);
static void write_bcm541xx_rdb_reg (void);
static void read_bcm541xx_i2c_reg(void);
static void write_bcm541xx_i2c_reg(void);
static int BCM541xx_register_test (void);
int neptune_cavium_sgmii_lpbk_test (void);
int BCM541xx_internal_loopback_test (void);
int BCM541xx_copper_ext_loopback_test (void);
int nep_sgmii_int_ext_loopback_test (void);
static int BCM541xx_sfp_ext_loopback_test (void);
static int neptune_macsec_test (void);
static int bcm54194_macsec_test (uint, uint);
static int neptune_ge_phy_ptp1588_test (void);
static void ge_phy_ieee_reg_dump (void);
static void ge_phy_reset(void); 
static void bcm541xx_test_mode_util(void);
static void read_bcm541xx_mdio45_reg(void);
static void write_bcm541xx_mdio45_reg(void);
static void bcm54194_sgmii_slave_mode_wrap(void);
static int bcm541xx_sfp_i2c_test_wrap(int);

static const reg_info_t bcm_54194_ieee_sgmii_reg[] = {
    {"SGMII Control",                          0x00, READ_WRITE, {2}, 0x55C0, 0x1140},
    {"SGMII Status",                           0x01, READ_ONLY,  {2}, 0x0000, 0x0149},
    {"SGMII AN Advertisement",                 0x04, READ_WRITE, {2}, 0x9E01, 0x0801},
    {"SGMII AN Link Partner Ability",          0x05, READ_ONLY,  {2}, 0x0000, 0x0001},
    {"end",                                    0x00, 0, {0}, 0, 0},
};

static const reg_info_t bcm_54194_ieee_copper_reg[] = {
    {"Copper MII Control",                     0x00, READ_WRITE, {2}, 0x55C0, 0x1140},
    {"Copper MII Status",                      0x01, READ_ONLY,  {2}, 0x0000, 0x79C9},
    {"PHY ID1 MSB",                            0x02, READ_ONLY,  {2}, 0x0000, 0xAE02},
    {"PHY ID2 LSB",                            0x03, READ_ONLY,  {2}, 0x0000, 0x5018},
    {"Copper AN Advertisement",                0x04, READ_WRITE, {2}, 0xBFFF, 0x01E1},
    {"Copper AN Link Partner Ability",         0x05, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"Copper AN Expansion",                    0x06, READ_ONLY,  {2}, 0x0000, 0x0064},
    {"Copper Next Page Transmit",              0x07, READ_WRITE, {2}, 0xB7FF, 0x2000},
    {"Copper Link Partner Received Next Page", 0x08, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"1000BT Control",                         0x09, READ_WRITE, {2}, 0xFF00, 0x0F00},
    {"1000BT Status",                          0x0A, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"IEEE Extended Status",                   0x0F, READ_ONLY,  {2}, 0x0000, 0x3000},
    {"end",                                    0x00, 0, {0}, 0, 0},
};

static const reg_info_t bcm_54194_ieee_fiber_reg[] = {
    {"Fiber Control",                          0x00, READ_WRITE, {2}, 0x55C0, 0x1140},
    {"1000BX Status",                          0x01, READ_ONLY,  {2}, 0x0000, 0x0140},
    {"1000BX AN Advertisement",                0x04, READ_WRITE, {2}, 0x0000, 0x0000},
    {"1000BX AN Link Partner Ability",         0x05, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"end",                                    0x00, 0, {0}, 0, 0},
};

static const bcm_phy_regs_t bcm54194_phy_ieee_reg_tbl[] = {
    {"Copper",    BCM54194_COPPER_INTF, bcm_54194_ieee_copper_reg},
    {"SGMII",     BCM54194_SGMII_INTF,  bcm_54194_ieee_sgmii_reg},
    {"Fiber",     BCM54194_FIBER_INTF,  bcm_54194_ieee_fiber_reg},
};


#define NUM_PHY_INTF (sizeof(bcm54194_phy_ieee_reg_tbl) /      \
                      sizeof(struct bcm_phy_regs_t_))

/******************************************************************************
 *  List of Menu used for SGMII BCM541xx
 *****************************************************************************/
static submenu_xtable_t BCM54194_tests_submenu_table[] = {
    {"GE PHY BCM541xx Utility", (type_t(*)())BCM541xx_utility,   FALSE,
       0, NULL, 0, (type_t(*)())BCM541xx_utility,   TRUE},
    {"GE PHY BCM541xx Register Test", (type_t(*)())BCM541xx_register_test,   0,
        MF_3, (type_t(*)())0, 0, (type_t(*)())0,   0},
    {"GE PHY Port0 SFP I2C Test", (type_t(*)())bcm541xx_sfp_i2c_test_wrap,   1,
        MF_3, (type_t(*)())0, 0, (type_t(*)())0,   0},
    {"GE PHY Port1 SFP I2C Test", (type_t(*)())bcm541xx_sfp_i2c_test_wrap,   2,
        MF_3, (type_t(*)())0, 0, (type_t(*)())0,   0},
    {"GE PHY BCM541xx Copper Ext/Internal Loopback Test", (type_t(*)())nep_sgmii_int_ext_loopback_test, 0,
        MF_3, (type_t(*)())0, 0, (type_t(*)())0,   0},
    {"GE PHY BCM541xx SFP External Loopback Test", (type_t(*)())BCM541xx_sfp_ext_loopback_test,   0,
        MF_3, (type_t(*)())0, 0, (type_t(*)())0,   0},
};

/******************************************************************************
 *  List of Utilities used for SGMII BCM541xx
 *****************************************************************************/
static submenu_xtable_t BCM54194_util_items[] = {
    {"Show BCM541xx PHY registers", (type_t(*)())ge_phy_ieee_reg_dump,   0, 0,
     (type_t(*)())0, 0, (type_t(*)())0,   0},
    {"STOP Linux Driver Polling link status", (type_t(*)())neptune_stop_lnx_polling, 0, 0,
     (type_t(*)())0, 0, (type_t(*)())0, 0},
    {"Read PHY BCM541xx IEEE Register", (type_t(*)())read_bcm541xx_ieee_reg, 0, 0,
     (type_t(*)())0, 0, (type_t(*)())0, 0},
    {"Write PHY BCM541xx IEEE Register", (type_t(*)())write_bcm541xx_ieee_reg, 0, 0,
     (type_t(*)())0, 0, (type_t(*)())0, 0},
    {"Read PHY BCM541xx RDB Register", (type_t(*)())read_bcm541xx_rdb_reg, 0, 0,
     (type_t(*)())0, 0, (type_t(*)())0, 0},
    {"Write PHY BCM541xx RDB Register", (type_t(*)())write_bcm541xx_rdb_reg, 0, 0,
     (type_t(*)())0, 0, (type_t(*)())0, 0},
    {"Read PHY BCM541xx I2C Slave Register", (type_t(*)())read_bcm541xx_i2c_reg, 0, 0,
     (type_t(*)())0, 0, (type_t(*)())0, 0},
    {"Write PHY BCM541xx I2C Slave Register", (type_t(*)())write_bcm541xx_i2c_reg, 0, 0,
     (type_t(*)())0, 0, (type_t(*)())0, 0},
    {"SGMII/SFP GE PHY loopback util", (type_t(*)()) neptune_phy_lpbk_util, 0, 0,
     (type_t(*)())0, 0, (type_t(*)())0,   0},
    {"GLC-GE-100FX SFP loopback test", (type_t(*)()) neptune_glc_ge_100fx_ext_lpbk, 0, 0,
     (type_t(*)())0, 0, (type_t(*)())0,   0},
    {"Cavium SGMII MAC loopback test", (type_t(*)()) neptune_cavium_sgmii_lpbk_test, 0, 0, 
     (type_t(*)())0, 0, (type_t(*)())0,   0},
    {"GE PHY BCM541xx Internal Loopback Test", (type_t(*)())BCM541xx_internal_loopback_test, 0, 0,
     (type_t(*)())0, 0, (type_t(*)())0,   0},
    {"GE PHY BCM541xx Copper External Loopback Test", (type_t(*)())BCM541xx_copper_ext_loopback_test, 0, 0,
     (type_t(*)())0, 0, (type_t(*)())0,   0},
    {"BCM541xx test mode util", (type_t(*)())bcm541xx_test_mode_util, 0, 0,
     (type_t(*)())0, 0, (type_t(*)())0, 0},
    {"GE PHY BCM541xx MACsec Test", (type_t(*)())macsec_test_main,   FALSE,
        0, 0, 0, (type_t(*)())macsec_test_main,   TRUE},
    {"GE PHY BCM541xx PTP1588 Test", (type_t(*)())ge_phy_ptp1588_test_main,   FALSE,
        0, (PFT)is_item_available, 0, (type_t(*)())ge_phy_ptp1588_test_main,   TRUE},
    {"PHY BCM541xx reset",           (type_t(*)())ge_phy_reset, 0, 0,
     (type_t(*)())0, 0, (type_t(*)())0, 0},
    {"Read PHY BCM541xx MDIO45 Register", (type_t(*)())read_bcm541xx_mdio45_reg, 0, 0,
     (type_t(*)())0, 0, (type_t(*)())0, 0},
    {"Write PHY BCM541xx MDIO45 Register", (type_t(*)())write_bcm541xx_mdio45_reg, 0, 0,
     (type_t(*)())0, 0, (type_t(*)())0, 0},
    {"Reset and Init BCM541XX", (type_t(*)())bcm54194_reset, 0, 0,
     (type_t(*)())0, 0, (type_t(*)())0, 0},
    {"Set BCM541XX SGMII-Slave Mode", (type_t(*)())bcm54194_sgmii_slave_mode_wrap, 0, 0,
     (type_t(*)())0, 0, (type_t(*)())0, 0},
};

/******************************************************************************
 *  Macro Definitions
 *****************************************************************************/
#define BCM54194_TESTS_SUBMENU_TABLE_SIZE (sizeof(BCM54194_tests_submenu_table) / \
                                           sizeof(submenu_xtable_t))

#define BCM54194_TESTS_UTIL_SIZE (sizeof(BCM54194_util_items) / \
                                  sizeof(submenu_xtable_t))

/******************************************************************************
 *  Global Variable
 *****************************************************************************/

/******************************************************************************
 * Primary & secondary submenu items (filled in from xtable)
 *****************************************************************************/
static mitem_t BCM54194_tests_primary_items[BCM54194_TESTS_SUBMENU_TABLE_SIZE +
                                            MAX_BASE_ITEMS];
static mitem_t BCM54194_tests_secondary_items[BCM54194_TESTS_SUBMENU_TABLE_SIZE +
                                              MAX_BASE_ITEMS];

/******************************************************************************
 * Primary & secondary utilities menu items (filled in from xtable)
 *****************************************************************************/
static mitem_t BCM54194_tests_primary_util_items[BCM54194_TESTS_UTIL_SIZE +
                                                 MAX_BASE_ITEMS];
static mitem_t BCM54194_tests_secondary_util_items[BCM54194_TESTS_UTIL_SIZE +
                                                   MAX_BASE_ITEMS];

/******************************************************************************
 * BCM541xx Utils submenu
 *****************************************************************************/
menuinfo_t BCM54194_util_menu = {
    "GE PHY BCM541xx Utility Menu",
    0,
    (PFT)show_endnote,
    0,
    0,
    BCM54194_tests_primary_util_items,
};
menuinfo_t *BCM54194_util_menup = &BCM54194_util_menu;

menuinfo_t BCM54194_subtest_menu = {
    "%s Subtest Menu",
    0,                                /* mtparam added by init_empty_menu */
    (PFT)show_endnote,                /* notes missing WICs in combos */
    0,                                /* use generic prompt */
    0,                                /* size (bumped by add_menu_item() */
    BCM54194_tests_primary_items,
};
menuinfo_t *BCM54194_submenup = &BCM54194_subtest_menu;

/******************************************************************************
 * The menu driven diagnostics are called from either the low level
 * power-on routine or from the monitor.  Frommon will be non-zero if
 * called from the monitor.  Note that this value is actually the
 * argc parameter (a little slight of hand here), which is always
 * greater than zero.
 *
 * The main menu is now defined in an _xtable_.  Both the primary items
 * and the secondary (shadow) items are built with function calls that
 * operate on it and insert the appropriate base items into the menu.
 ******************************************************************************/
int ge_phy_bcm541xx_test (int show_menu)
{
    build_primary_submenu(BCM54194_tests_submenu_table,
                          BCM54194_TESTS_SUBMENU_TABLE_SIZE,
                          "GE PHY BCM541xx", &BCM54194_submenup);
    build_secondary_submenu(BCM54194_tests_submenu_table,
                            BCM54194_TESTS_SUBMENU_TABLE_SIZE,
                            BCM54194_tests_secondary_items);

    if (show_menu) {
        menu(BCM54194_submenup, BCM54194_tests_secondary_items, '\0' );
    } else {
        menu_exec_doall_diags(BCM54194_submenup);
    }
    return (PASSED);
}


/*******************************************************************************
 *
 * Function    : BCM541xx_utility
 * Description :
 * Inputs      : menu_option - display menu instead of running all XAUI 88X2222M
 *               tests.
 * Outputs     : PASSED / FAILED
 *
 ******************************************************************************/
static int BCM541xx_utility (int show_menu)
{

    build_primary_submenu(BCM54194_util_items, BCM54194_TESTS_UTIL_SIZE,
                          "GE PHY BCM541xx Utilities Menu", &BCM54194_util_menup);
    build_secondary_submenu(BCM54194_util_items, BCM54194_TESTS_UTIL_SIZE,
                            BCM54194_tests_secondary_util_items);

    menu(BCM54194_util_menup, BCM54194_tests_secondary_util_items, '\0' );

    return (PASSED);
}

static int macsec_sgmii_eth_port_list[] = {ETH3, ETH4, ETH5, ETH6};
static int macsec_sgmii_eth_speed_list[] = {SPD_1000MBPS};

#define F_GRP        (MF_CONTINUOUS | MF_DOGRP)
#define F_GRP_E      (F_GRP | MF_SHOW_ERRCOUNT)
#define F_ALL        (F_GRP | MF_DOALL)
#define F_ALL_E      (F_ALL | MF_SHOW_ERRCOUNT)

/* Sub Menu used for Ethernet port tests.
 */
static submenu_xtable_t macsec_tests_submenu_table[] = {
    {"MACsec test on BCM541xx PHY", (type_t(*)())neptune_macsec_test,   0,
        F_GRP_E, (type_t(*)())0, 0, (type_t(*)())0,   0},
    {"Enable MACSec", (type_t(*)())init_bcm54194_macsec,   0xF, 0,
     (type_t(*)())0, 0, (type_t(*)())0,   0},
    {"Disable MACSec", (type_t(*)())disable_bcm54194_macsec,   0xF, 0,
     (type_t(*)())0, 0, (type_t(*)())0,   0},
};

#define MACSEC_TESTS_SUBMENU_TABLE_SIZE (sizeof(macsec_tests_submenu_table) / \
                                         sizeof(submenu_xtable_t))

/*
 * Primary & secondary submenu items (filled in from xtable)
 */
static mitem_t macsec_tests_primary_items[MACSEC_TESTS_SUBMENU_TABLE_SIZE +
                                          MAX_BASE_ITEMS];
static mitem_t macsec_tests_secondary_items[MACSEC_TESTS_SUBMENU_TABLE_SIZE +
                                            MAX_BASE_ITEMS];

menuinfo_t macsec_subtest_menu = {
    "%s Subtest Menu",
    0,                                /* mtparam added by init_empty_menu */
    (PFT)show_endnote,                /* notes missing WICs in combos */
    0,                                /* use generic prompt */
    0,                                /* size (bumped by add_menu_item() */
    macsec_tests_primary_items,
};
menuinfo_t *macsec_submenup = &macsec_subtest_menu;

/*------------------------------------------------------------------
 *
 * Function: macsec_test_main
 *      This is the entry point for the macsec main test.
 *
 * Input:  dummy
 *
 * Output: PASSED/FAILED
 *
 *------------------------------------------------------------------
 */
static int macsec_test_main (int dummy)
{

    build_primary_submenu(macsec_tests_submenu_table,
                          MACSEC_TESTS_SUBMENU_TABLE_SIZE,
                          "MACsec", &macsec_submenup);
    build_secondary_submenu(macsec_tests_submenu_table,
                            MACSEC_TESTS_SUBMENU_TABLE_SIZE,
                            macsec_tests_secondary_items);

    menu(macsec_submenup, macsec_tests_secondary_items, '\0' );

    return(PASSED);
}

/*------------------------------------------------------------------
 *
 * Function: neptune_macsec_test
 *      a testing wrapper for macsec test
 *
 * Input: NONE
 *
 * Output: PASSED/FAILED
 *
 *------------------------------------------------------------------
 */
static int neptune_macsec_test (void)
{
    uint port, port_curr, rc = FAILED;
    uint speed, speed_curr;
    uint speed_cnt, port_cnt;
    uint try, retry_limit = 2;
    int *eth_speed_list = macsec_sgmii_eth_speed_list;
    int *eth_port_list = macsec_sgmii_eth_port_list;

    testname("BCM541xx MACsec external loopback");

    port_cnt = sizeof(macsec_sgmii_eth_port_list) / sizeof(int);
    speed_cnt = sizeof(macsec_sgmii_eth_speed_list) / sizeof(int);

    for (port_curr = 0; port_curr < port_cnt; port_curr++) {
        port = eth_port_list[port_curr];

        for (speed_curr = 0; speed_curr < speed_cnt; speed_curr++) {
           speed = eth_speed_list[speed_curr];

           prpass(testpass, "Test port-%d speed-%d, ", port, speed);
           for (try=0; try < retry_limit; try++) {
               rc = bcm54194_macsec_test(port,speed);
               if ((rc == PASSED) || (try == (retry_limit - 1))) {
                    break;
               } else {
                    printf("####### retry the test #########\n");
                    bcm54194_reset();
               }
           }

           if (rc != PASSED) {
               cterr('f',0,"MACsec test failed on port%d with spd%d\n", port, speed);
               bcm54194_reset();
               return(FAILED);
           }

        }  /* for speed */
    }  /* for port */

    /* Reset PHY to restore default setting. */
    bcm54194_reset();

    return(PASSED);
}

/*------------------------------------------------------------------
 *
 * Function: bcm54194_macsec_test
 *
 * Description: testing macsec on 88E1548P.
 *              steps: turn on macsec on PHY,
 *              disable drop_bad_tag, send packets and
 *              check the statistic bit on PHY.
 *
 * Input:  port - test port
 *         speed - test speed
 *
 * Output: PASSED/FAILED
 *
 *------------------------------------------------------------------
 */
static int bcm54194_macsec_test (uint eth_num, uint speed)
{
    int *eth_mapping_port_num = eth_mapping_ge_num, result = FAILED;
    uint phy_addr = eth_mapping_phy_addr[eth_num];

    prpass(testpass, "Test port-%d speed-%d, ", eth_mapping_port_num[eth_num], speed);

    /* init macsec setting and cleanup macsec counter(statistic) */
    init_bcm54194_macsec(phy_addr);

    /* using external loopback configuration so far */
    if (set_ge_phy_lpbk(SEL_PORT_ETH, eth_num, speed, BCM54194_COPPER_INTF, GE_PHY_EXT_LPBK, TRUE)) {
        printf("Port%d set external loopback failed with spd %d\n",
               eth_mapping_port_num[eth_num], speed);
        return (FAILED);
    }

    msleep(1000);

    /* send packets with BRCM pattern. */
    result = neptune_set_packet(SEL_PORT_ETH, eth_num, speed);   

    /* Disable MACsec and loopback before leaving test. */
    if (set_ge_phy_lpbk(SEL_PORT_ETH, eth_num, speed, BCM54194_COPPER_INTF, GE_PHY_EXT_LPBK, FALSE)) {
        printf("Port%d set external loopback failed with spd %d\n",
               eth_mapping_port_num[eth_num], speed);
        return (FAILED);
    }

    if (result != PASSED) {
        printf("sgmii_set_packet failed %s\n",__FUNCTION__);
        return (result);
    }
    
    return (result);
}

/* Sub Menu used for Ethernet port tests.
 */
static submenu_xtable_t ge_phy_ptp1588_tests_submenu_table[] = {
    {"PTP1588 test on BCM541xx PHY", (type_t(*)())neptune_ge_phy_ptp1588_test,   0,
     F_GRP_E, (type_t(*)())0, 0, (type_t(*)())0,   0},
    {"ibts gm t1 ts da cap", (type_t(*)())enable_bcm54194_ibts_gm_sync_t1_ts_da_cap, ETH3,
     F_GRP_E, (type_t(*)())0, 0, (type_t(*)())0,   0},
    {"ibts sc sync t2 ts da cap", (type_t(*)())enable_bcm54194_ibts_sc_sync_t2_ts_da_cap, ETH3,
     F_GRP_E, (type_t(*)())0, 0, (type_t(*)())0,   0},
    {"ibts sc dreq t3 assist", (type_t(*)())enable_bcm54194_ibts_sc_dreq_t3_assist, ETH3,
     F_GRP_E, (type_t(*)())0, 0, (type_t(*)())0,   0},
    {"ibts gm rx dreq t4", (type_t(*)())enable_bcm54194_ibts_gm_rx_dreq_t4, ETH3,
     F_GRP_E, (type_t(*)())0, 0, (type_t(*)())0,   0},
    {"read timestamp", (type_t(*)())dump_bcm54194_timestamp, 0,
     F_GRP_E, (type_t(*)())0, 0, (type_t(*)())0,   0},
};

#define GE_PHY_PTP1588_TESTS_SUBMENU_TABLE_SIZE (sizeof(ge_phy_ptp1588_tests_submenu_table) / \
                                                 sizeof(submenu_xtable_t))

/*
 * Primary & secondary submenu items (filled in from xtable)
 */
static mitem_t ge_phy_ptp1588_tests_primary_items[GE_PHY_PTP1588_TESTS_SUBMENU_TABLE_SIZE +
                                                  MAX_BASE_ITEMS];
static mitem_t ge_phy_ptp1588_tests_secondary_items[GE_PHY_PTP1588_TESTS_SUBMENU_TABLE_SIZE +
                                                    MAX_BASE_ITEMS];

menuinfo_t ge_phy_ptp1588_subtest_menu = {
    "%s Subtest Menu",
    0,                                /* mtparam added by init_empty_menu */
    (PFT)show_endnote,                /* notes missing WICs in combos */
    0,                                /* use generic prompt */
    0,                                /* size (bumped by add_menu_item() */
    ge_phy_ptp1588_tests_primary_items,
};
menuinfo_t *ge_phy_ptp1588_submenup = &ge_phy_ptp1588_subtest_menu;

/*------------------------------------------------------------------
 *
 * Function: ge_phy_ptp1588_test_main
 *      This is the entry point for the macsec main test.
 *
 * Input:  dummy
 *
 * Output: PASSED/FAILED
 *
 *------------------------------------------------------------------
 */
static int ge_phy_ptp1588_test_main (int dummy)
{

    build_primary_submenu(ge_phy_ptp1588_tests_submenu_table,
                          GE_PHY_PTP1588_TESTS_SUBMENU_TABLE_SIZE,
                          "PTP1588", &ge_phy_ptp1588_submenup);
    build_secondary_submenu(ge_phy_ptp1588_tests_submenu_table,
                            GE_PHY_PTP1588_TESTS_SUBMENU_TABLE_SIZE,
                            ge_phy_ptp1588_tests_secondary_items);

    menu(ge_phy_ptp1588_submenup, ge_phy_ptp1588_tests_secondary_items, '\0' );

    return(PASSED);
}

static void neptune_stop_lnx_polling(void)
{
    int eth_num, onoff;
    eth_num = gethex_answer("\nEnter eth number", 3, 3, 6);
    onoff = gethex_answer("0:Resume, 1:Suspend", 0, 0, 1);
    bcm54194_suspend_lnx_link_polling ("eth", eth_num, onoff);
}

static void bcm54194_sgmii_slave_mode_wrap(void)
{
    uint port, enable = FALSE, addr;
    port = getdec_answer("\nEnter port num ", 0, 0, 1);
    enable = getdec_answer("\n0: Disable, 1: Enable ", 0, 0, 1);
    addr = ge_port_mapping_phy_addr[port];
    bcm54194_sgmii_slave_mode(addr, enable);
}

static int bcm541xx_sfp_i2c_test_wrap(int sfp)
{
    int rc = FAILED, port = sfp - OVLD_CAVIUM_TWSI_SFP0;
    char *tname = "BCM541xx sfp i2c";
    testname("%s port %d", tname, port);

    if ((NVRAM)->diagflag & D_EXT_LOOPBACK) {
        printf("External loopback flag is off, skip the BCM PHY port %d sfp+ i2c test\n", port);
    }

    rc = bcm_sfp_i2c_test(sfp);
    if (rc != PASSED) {
        cterr('f', 0, "bcm_sfp_i2c_test failed on port", port);
        return (rc);
    } else {
        prpass(testpass, "port %d passed, ", port);
    }

    return (rc);
}

/*
 * Function: read_bcm541xx_mdio45_reg
 *
 * Description: Read BCM541xx IEEE standard registers utility
 *
 * Input: none
 *
 * Return: none
 */
static void read_bcm541xx_mdio45_reg(void)
{
    int dev_id, bus_id = SMI_BUS_0;
    ushort rdval;
    int phy_addr, regnum;

    phy_addr = gethex_answer("\nEnter phy addr (0x0 - 0xFF)", 0, 0, 0xFF);
    dev_id = gethex_answer("\nEnter dev id (1, 3, 7)", 1, 1, 7);
    regnum = gethex_answer("\nEnter PHY mdio45 reg number", 0, 0, 0xFFFF);

    rdval = cvmx_mdio_45_read(bus_id, phy_addr, dev_id, regnum);
    if (rdval < 0) {
        printf("Failed to read GE PHY, addr:%#.2x, %d.%#.4x\n", phy_addr, dev_id, regnum);
    } else {
        printf("phy addr:%#.2x, %d.%#.4x = %#.4x\n", phy_addr, dev_id, regnum, rdval);
    }
}

/*
 * Function: write_bcm541xx_mdio45_reg
 *
 * Description: Write BCM541xx IEEE standard registers utility
 *
 * Input: none
 *
 * Return: none
 */
static void write_bcm541xx_mdio45_reg(void)
{
    int bus_id = SMI_BUS_0;
    ushort wrval;
    int rc, phy_addr, dev_id, regnum;

    phy_addr = gethex_answer("\nEnter phy addr (0x0 - 0xFF)", 0, 0, 0xFF);
    dev_id = gethex_answer("\nEnter dev id (1, 3, 7)", 1, 1, 7);
    regnum = gethex_answer("\nEnter PHY mdio45 reg number", 0, 0, 0xFFFF);
    wrval = gethex_answer("Enter value:", 0, 0, 0xFFFF);

    rc = cvmx_mdio_45_write(bus_id, phy_addr, dev_id, regnum, wrval);
    if (rc != PASSED) {
        printf("Failed to write GE PHY, phy addr:%#.2x, %d.%#.4x\n", phy_addr, dev_id, regnum);
    } else {
        printf("phy addr:%#.2x, %d.%#.4x <-- %#.4x, \n", phy_addr, dev_id, regnum, wrval);
    }
}

/*
 * Function: read_bcm541xx_ieee_reg
 *
 * Description: Read BCM541xx IEEE standard registers utility
 *
 * Input: none
 *
 * Return: none
 */
static void read_bcm541xx_ieee_reg(void)
{
    int port_max = 3;
    int port_min = 0;
    int bus_id = SMI_BUS_0, phy_intf = BCM54194_SGMII_INTF;
    ushort rdval;
    int phy_addr, portnum, regnum, regnum_max = 0xF;

    portnum = gethex_answer("\nEnter port num (0x0 - 0x3)", port_min, port_min, port_max);
    phy_intf = gethex_answer("\nEnter Interface (SGMII:0, Copper:1, Fiber:2)", 0, BCM54194_SGMII_INTF, BCM54194_FIBER_INTF);
    regnum = gethex_answer("\nEnter PHY IEEE reg number", 0, 0, regnum_max);

    phy_addr = ge_port_mapping_phy_addr[portnum];

    /* Switch to SGMII or Fiber register space */
    if (phy_intf == BCM54194_SGMII_INTF) {
        //bcm54194_switch_intf_access(phy_intf);
        phy_addr += 4;
    } else if (phy_intf == BCM54194_FIBER_INTF) {
    	bcm54194_reg_1000x_en(phy_addr, TRUE);
    }

    rdval = cvmx_mdio_read(bus_id, phy_addr, regnum);
    if (rdval < 0) {
        printf("Failed to read GE PHY, port%d, 0x%x\n", portnum, regnum);
    } else {
        printf("port%d, 0x%x = %#.4x, \n", portnum, regnum, rdval);
    }

#ifdef BCM54194_A0_SILICON
    /* Restore to Copper register space */
    if (phy_intf == BCM54194_SGMII_INTF) {
        bcm54194_switch_intf_access(BCM54194_COPPER_INTF);
    } else if (phy_intf == BCM54194_FIBER_INTF) {
    	bcm54194_reg_1000x_en(phy_addr, FALSE);
    }
#endif
}

/*
 * Function: write_bcm541xx_ieee_reg
 *
 * Description: Write BCM541xx IEEE standard registers utility
 *
 * Input: none
 *
 * Return: none
 */
static void write_bcm541xx_ieee_reg(void)
{
    int port_max = 3;
    int port_min = 0;
    int bus_id = SMI_BUS_0, phy_intf = BCM54194_SGMII_INTF;
    ushort wrval;
    int rc, phy_addr, portnum, regnum, regnum_max = 0xF;

    portnum = gethex_answer("\nEnter port num (0x0 - 0x3)", port_min, port_min, port_max);
    phy_intf = gethex_answer("\nEnter Interface (SGMII:0, Copper:1, Fiber:2)", 0, BCM54194_SGMII_INTF, BCM54194_FIBER_INTF);
    regnum = gethex_answer("\nEnter PHY IEEE reg number", 0, 0, regnum_max);
    wrval = gethex_answer("Enter value:", 0, 0, 0xFFFF);

    phy_addr = ge_port_mapping_phy_addr[portnum];

    /* Switch to SGMII or Fiber register space */
    if (phy_intf == BCM54194_SGMII_INTF) {
        //bcm54194_switch_intf_access(phy_intf);
        phy_addr += 4;
    } else if (phy_intf == BCM54194_FIBER_INTF) {
    	bcm54194_reg_1000x_en(phy_addr, TRUE);
    }

    rc = cvmx_mdio_write(bus_id, phy_addr, regnum, wrval);
    if (rc != PASSED) {
        printf("Failed to write GE PHY, port%d, 0x%x\n", portnum, regnum);
    } else {
        printf("port%d, 0x%x <-- %#.4x, \n", portnum, regnum, wrval);
    }
#ifdef BCM54194_A0_SILICON
    /* Restore to Copper register space */
    if (phy_intf == BCM54194_SGMII_INTF) {
        bcm54194_switch_intf_access(BCM54194_COPPER_INTF);
    } else if (phy_intf == BCM54194_FIBER_INTF) {
    	bcm54194_reg_1000x_en(phy_addr, FALSE);
    }
#endif
}

/*
 * Function: read_bcm541xx_rdb_reg
 *
 * Description: Read BCM541xx RDB registers utility
 *
 * Per-Port RDB Registers (RDB_reg. 0x00 to 0x2FF)
 * Global RDB Registers (RDB_reg. 0x800 to 0xAFF)
 *
 * Input: none
 *
 * Return: none
 */
static void read_bcm541xx_rdb_reg(void)
{
    int port_min = 0, port_max = 3;
    int bus_id = SMI_BUS_0;
    ushort rdval;
    int rc, phy_addr, portnum, regnum, regnum_max = 0xAFF;

    printf("\nPer-Port RDB Registers (RDB_reg. 0x00 to 0x2FF)\n");
    printf("\nGlobal RDB Registers (RDB_reg. 0x800 to 0xAFF)\n");
    portnum = gethex_answer("\nEnter port num (0x0 - 0x3)", port_min, port_min, port_max);
    regnum = gethex_answer("\nEnter PHY RDB reg number", 0, 0, regnum_max);

    phy_addr = ge_port_mapping_phy_addr[portnum];
    rc = bcm54194_rdb_read(bus_id, phy_addr, regnum, &rdval);
    if (rc < 0) {
        printf("Failed to read GE PHY, phy addr:%d, RDB offset:0x%x\n", phy_addr, regnum);
    } else {
        printf("port%d, RDB:0x%x = %#.4x, \n", portnum, regnum, rdval);
    }
}

/*
 * Function: write_bcm541xx_rdb_reg
 *
 * Description: Utility to do peek and poke to GE PHY registers
 *
 * Input: none
 *
 * Return: none
 */
static void write_bcm541xx_rdb_reg(void)
{
    int port_min = 0, port_max = 3;
    int bus_id = SMI_BUS_0;
    ushort wrval;
    int rc, phy_addr, portnum, regnum, regnum_max = 0xAFF;

    printf("\nPer-Port RDB Registers (RDB_reg. 0x00 to 0x2FF)\n");
    printf("\nGlobal RDB Registers (RDB_reg. 0x800 to 0xAFF)\n");
    portnum = gethex_answer("\nEnter port num (0x0 - 0x3)", port_min, port_min, port_max);
    regnum = gethex_answer("\nEnter PHY RDB reg number", 0, 0, regnum_max);
    wrval = gethex_answer("Enter value:", 0, 0, 0xFFFF);

    phy_addr = ge_port_mapping_phy_addr[portnum];

    rc = bcm54194_rdb_write(bus_id, phy_addr, regnum, wrval);
    if (rc < 0) {
    	printf("Failed to write GE PHY, phy addr:0x%x, RDB offset:0x%x, value=0x%#.4x\n", phy_addr, regnum, wrval);
    } else {
        printf("port%d, RDB:0x%x <-- %#.4x, \n", portnum, regnum, wrval);
    }
}

/*
 * Function: read_bcm541xx_i2c_reg
 *
 * Description: BCM541xx external I2C device register read utility
 *
 * Input: none
 *
 * Return: none
 */
static void read_bcm541xx_i2c_reg(void)
{
    ushort rdval;
    int rc, dev_addr, reg_addr;

    dev_addr = gethex_answer("\nEnter PHY I2C dev addr", 0, 0, 0xFF);
    reg_addr = gethex_answer("\nEnter PHY I2C reg addr", 0, 0, 0xFF);

    enable_bcm54194_i2c_access(TRUE);

    rc = bcm54194_i2c_slave_read(dev_addr, reg_addr, &rdval);
    if (rc < 0) {
        printf("Failed to read PHY I2C reg, dev addr:0x%x, reg addr:0x%x\n", dev_addr, reg_addr);
    } else {
        printf("dev addr:0x%x, reg addr:0x%x = 0x%#.4x, \n", dev_addr, reg_addr, rdval);
    }

    enable_bcm54194_i2c_access(FALSE);
}

/*
 * Function: write_bcm541xx_i2c_reg
 *
 * Description: BCM541xx external I2C device register write utility
 *
 * Input: none
 *
 * Return: none
 */
static void write_bcm541xx_i2c_reg(void)
{
    ushort wrval;
    int rc, dev_addr, reg_addr;

    dev_addr = gethex_answer("\nEnter PHY I2C dev addr", 0, 0, 0xFF);
    reg_addr = gethex_answer("\nEnter PHY I2C reg addr", 0, 0, 0xFF);
    wrval = gethex_answer("\nEnter the value", 0, 0, 0xFF);
    
    enable_bcm54194_i2c_access(TRUE);
    
    rc = bcm54194_i2c_slave_write(dev_addr, reg_addr, wrval);
    if (rc < 0) {
    	printf("Failed to write PHY I2C reg, dev addr:0x%x, reg addr:0x%x, value = 0x%#.4x\n", dev_addr, reg_addr, wrval);
    } else {
        printf("dev addr:0x%x, reg addr:0x%x <-- 0x%#.4x, \n", dev_addr, reg_addr, wrval);
    }

    enable_bcm54194_i2c_access(FALSE);
}

/*******************************************************************************
 *
 * Function: phy_register_tests
 *
 * For each register from reg_ptr, this function checks for accessibility
 * and does a ripple 1 and a ripple 0 test if applicable (not all registers
 * are W/R register).
 *
 * Input : interface structure pointer, info for all registers
 *
 * Output: PASS/FAIL
 *
 *******************************************************************************/
static int phy_register_tests (int phy_addr, bcm54194_intf_t intf, const reg_info_t *reg_ptr )
{
    uint32_t ix;
    uint16_t retval = PASSED;
    uint16_t data, temp, tst_offset, save_val, readval = 0x0;
    int bus_id = SMI_BUS_0;

    /* Switch to Copper/Fiber register space */
    if (intf == BCM54194_FIBER_INTF) {
    	bcm54194_reg_1000x_en(phy_addr, TRUE);
    } else {
        bcm54194_reg_1000x_en(phy_addr, FALSE);
    }

    while (reg_ptr->size.size != 0) {

        save_val = cvmx_mdio_read(bus_id, phy_addr, reg_ptr->offset);
        if (save_val < 0) {
            cterr('f', 0, "%s(): Error reading %s register offset %#x"
                  "phy_addr %d\n", __FUNCTION__,  reg_ptr->name,
                  reg_ptr->offset, phy_addr);
            return (FAILED);
        }

        if (reg_ptr->type == READ_WRITE) {

            tst_offset = reg_ptr->offset;

            /*
             * ripple 1 test
             */
            for (ix = 0; ix < (reg_ptr->size.size * 8); ix++) {

                temp = (1 << ix) & reg_ptr->mask;
                if (!temp) {
                	continue;
                }

                /* Write to register under test */

                retval = cvmx_mdio_write(bus_id, phy_addr, tst_offset, temp);

                /* Read back */
                if (retval == PASSED) {
                    readval = cvmx_mdio_read(bus_id, phy_addr, tst_offset);
                }

                if ( ((readval & reg_ptr->mask) != temp) || (retval == FAILED)) {

                    cterr('f', 0, "%s(): Ripple one test failed when accessing %s "
                          "Register offset %#x, phy_addr %d,Expect %#x, Read %#x",
                          __FUNCTION__, reg_ptr->name, tst_offset, phy_addr, temp,
                          readval);
                    return (FAILED);
                }
            }

            /*
             * ripple 0 test
             */
            for (ix = 0; ix < (reg_ptr->size.size * 8); ix++) {
                temp = (1 << ix) & reg_ptr->mask;
                if (!temp) {
                    continue;
                }

                temp = (~(1 << ix)) & reg_ptr->mask;
                /* Write to register under test */
                retval = cvmx_mdio_write(bus_id, phy_addr, tst_offset, temp);

                if (retval == PASSED) {
                    /* Read back */
                    readval = cvmx_mdio_read(bus_id, phy_addr, tst_offset);
                }

                if (((readval & reg_ptr->mask) != temp) || (retval == FAILED)) {
                    cterr('f', 0, "%s(): Ripple one test failed when accessing %s "
                          "Register offset %#x, phy_addr %d, Expect %#x, Read %#x",
                          __FUNCTION__, reg_ptr->name, tst_offset, phy_addr, temp,
                          readval);
                    return (retval);
                }
            }

            /*
             * pattern test
             */
            data = NEP_PATTERN;
            for (ix = 0; ix < 2; ix++) {
                temp = data & reg_ptr->mask;
                /* Write to register under test */
                retval = cvmx_mdio_write(bus_id, phy_addr, tst_offset, temp);

                if (retval == PASSED) {
                    /* Read back */
                    readval = cvmx_mdio_read(bus_id, phy_addr, tst_offset);
                }

                if (((readval & reg_ptr->mask) != temp) || (retval == FAILED)) {
                    cterr('f', 0, "%s(): Pattern test failed when accessing %s "
                          "Register offset %#x phy_addr %d, Expect %#x, "
                          "Read %#x", __FUNCTION__, reg_ptr->name, tst_offset,
                          phy_addr, temp, readval);
                    return (retval);
                }

                data = ~NEP_PATTERN; /* complement data pattern */
            }

            /*
             * restore original value
             */
            retval = cvmx_mdio_write(bus_id, phy_addr, tst_offset, save_val);
            if (retval == FAILED) {
                cterr('f', 0, "%s(): Error restoring %s register "
                      "offset %#x, phy_addr %d\n", __FUNCTION__,
                      reg_ptr->name, reg_ptr->offset, phy_addr);
                return (FAILED);
            }
        }
        reg_ptr++;
    }

    return (PASSED);
}

/******************************************************************************
 *
 * Function: BCM541xx_register_test
 *
 * Description: This function performs the BCM541xx register test.
 * This test only performs BCM541xx IEEE register set.
 *
 * Inputs      : port - port number
 *
 * Outputs     : PASSED / FAILED
 *
 ********************************************************************************/
static int BCM541xx_register_test (void)
{
    int ix = 0, phy_addr = 0, port_cnt = 4;
    testname("BCM541xx PHY IEEE Register");

    for (ix = 0; ix < port_cnt; ix++) {
        phy_addr = ge_port_mapping_phy_addr[ix];

        prpass(testpass, "Fiber : ");
        if (phy_register_tests(phy_addr, BCM54194_FIBER_INTF,
                               &bcm_54194_ieee_fiber_reg[0]) == FAILED) {
            cterr('f', 0, "Register Test on address 0x%#x fails.", phy_addr);
            return (FAILED);
        }

        prpass(testpass, "Copper : ");
        if (phy_register_tests(phy_addr, BCM54194_COPPER_INTF,
                               &bcm_54194_ieee_copper_reg[0]) == FAILED) {
            cterr('f', 0, "Register Test on address 0x%#x fails.", phy_addr);
            return (FAILED);
        }
    }

    return (PASSED);
}

/*******************************************************************************
 *
 * Function: dump_phy_reg()
 *
 * This function prints the specific PHY register values
 *
 * Input: curr_port - current port (wit offset)
 *        page_reg_ptr - page table pointer.
 *
 * Output: PASSED/FAILED
 *
 *******************************************************************************
 */
static void dump_phy_reg(int phy_addr, const bcm_phy_regs_t *phy_reg_ptr)
{
      short rdval, bus_id = SMI_BUS_0;
      const reg_info_t *reg_ptr;

      /* Switch to SGMII or Fiber register space */
      if (phy_reg_ptr->phy_intf == BCM54194_SGMII_INTF) {
          //bcm54194_switch_intf_access(phy_reg_ptr->phy_intf);
          phy_addr += 4;
      } else if (phy_reg_ptr->phy_intf == BCM54194_FIBER_INTF) {
          bcm54194_reg_1000x_en(phy_addr, TRUE);
      }

      //printf("\n%s\n", phy_reg_ptr->intfname);
      reg_ptr = phy_reg_ptr->intfregs;

      while (reg_ptr->size.size != 0) {
          rdval = cvmx_mdio_read(bus_id, phy_addr, reg_ptr->offset);
          /* we don't check rdval is nagetive here,
           * some of registers will get '0xF' on MSB
           */

          printf("%s : %-32s reg %#.2x = %#.4x\n", phy_reg_ptr->intfname, reg_ptr->name,
                  reg_ptr->offset, rdval);
          reg_ptr++;
          msleep(10); /* wait for a while for next register. */
      }
#ifdef BCM54194_A0_SILICON
      /* Restore to Copper register space */
      if (phy_reg_ptr->phy_intf == BCM54194_SGMII_INTF) {
          bcm54194_switch_intf_access(BCM54194_COPPER_INTF);
      } else if (phy_reg_ptr->phy_intf == BCM54194_FIBER_INTF) {
          bcm54194_reg_1000x_en(phy_addr, FALSE);
      }
#endif
}

/*******************************************************************************
 *
 * Function: phy_reg_show()
 *
 * This function get PHY page/regs info and call dump_phy_reg() to prints
 * PHY registers.
 *
 * Input: port - current port (without offset)
 *        phy_sel - PHY offset value to get specifc PHY addr.
 *        page_sel - select PHY page.
 *        dump_type - dump on page or all pages.
 *
 * Output: PASSED/FAILED
 *
 *******************************************************************************
 */
static int phy_reg_show (int port)
{
    uint ix, intf_move;
    const bcm_phy_regs_t *phy_reg_ptr;

    phy_reg_ptr = &bcm54194_phy_ieee_reg_tbl[0];
    intf_move = NUM_PHY_INTF;

    /* dump all page */
    for (ix = 0; ix < intf_move; ix++) {
        dump_phy_reg(port, phy_reg_ptr);
        phy_reg_ptr++;
    }

    return (PASSED);
}

/*
 * Function: ge_phy_ieee_reg_dump
 *
 * This function displays the PHY setting of the requested SGMII port.
 * Using phy_reg_show() to dump PHY page.
 *
 * Input: none.
 *
 * Output: void
 */
static void ge_phy_ieee_reg_dump (void)
{
    int port_max = 3;
    int port_min = 0;
    int portnum, phy_addr;

    printf("\nGE PHY port num are 0x0 - 0x3)\n");
    portnum = gethex_answer("\nEnter port num ", port_min, port_min, port_max);
    phy_addr = ge_port_mapping_phy_addr[portnum];
    phy_reg_show(phy_addr);
}

/**********************************************************************
 *
 * Function: neptune_cavium_int_lpbk_test
 *
 * Description: Neptune Cavium internal loopback test
 *
 * Input: void
 *
 * Return: pass/fail
 */
int neptune_cavium_sgmii_lpbk_test(void)
{
    return(neptune_phy_lpbk_test(CAVIUM_INT_LPBK));
}

/***********************************************************************
 *
 * Function: BCM541xx_internal_loopback_test
 *
 * Description: Do GE PHY internal loopback test
 *
 * Inputs: phy - phy number
 *
 * Outputs: PASSED/FAILED
 *
 **************************************************************************/
int BCM541xx_internal_loopback_test (void)
{
    printf("\n!!! BCM PHY541xx internal loopback require no external plug connected !!!\n");
    return (neptune_phy_lpbk_test(GE_PHY_INT_LPBK));
}

/***********************************************************************
 *
 * Function: BCM541xx_copper_ext_loopback_test
 *
 * Description: Do GE PHY copper external loopback test
 *
 * Inputs: phy - phy number
 *
 * Outputs: PASSED/FAILED
 *
 ***************************************************************************/
int BCM541xx_copper_ext_loopback_test (void)
{
    return (neptune_phy_lpbk_test(GE_PHY_EXT_LPBK));
}

/***********************************************************************
 *
 * Function: nep_sgmii_int_ext_loopback_test
 *
 * Description: Do GE PHY copper external loopback test
 *
 * Inputs: phy - phy number
 *
 * Outputs: PASSED/FAILED
 *
 ***************************************************************************/
int nep_sgmii_int_ext_loopback_test (void)
{
    if (check_ge_int_lpbk_flag() || !check_ext_lpbk_flag()) {
        printf("\n!!! BCM PHY541xx internal loopback require no external plug connected !!!\n");
    }
    return (neptune_phy_lpbk_test(SGMII_INT_EXT_LPBK));
}

/***********************************************************************
 *
 * Function: sgmii_BCM54194_sfp_ext_loopback_test
 *
 * Description: Do GE PHY fiber external loopback test
 *
 * Inputs: phy - phy number
 *
 * Outputs: PASSED/FAILED
 *
 ***************************************************************************/
static int BCM541xx_sfp_ext_loopback_test (void)
{
    return (sfp_phy_ext_lpbk_test());
}

/***********************************************************************
 *
 * Function: sgmii_BCM54194_copper_ext_loopback_test
 *
 * Description: Do GE PHY copper external loopback test
 * The function has NOT been verified yet.
 *
 * Inputs: phy - phy number
 *
 * Outputs: PASSED/FAILED
 *
 ***************************************************************************/
static int neptune_ge_phy_ptp1588_test (void)
{
    return (neptune_phy_lpbk_test(PTP_SGMII_EXT_LPBK));
}

/***********************************************************************
 *
 * Function: ge_phy_reset
 *
 * Description: Do GE PHY reset/unreset 
 *
 * Inputs: none
 *
 * Outputs: none
 *
 ***************************************************************************/
static void ge_phy_reset (void) 
{
    int reset; 
    sys_lvl_t *sys = (sys_lvl_t *)dash_fpga;

    assert(dash_fpga);

    reset = gethex_answer("\nPHY reset 1; unreset 0", 0, 0, 1);

    if (reset) {
        printf("Reset GE PHY...\n");
        sys->ext_rst |= FPGA_EXT_GE_QUAD_RST; 
    } else {
        printf("Unreset GE PHY...\n");
        sys->ext_rst &= ~FPGA_EXT_GE_QUAD_RST; 
    }
}

/*
 * Function: bcm541xx_test_mode_util
 *
 * Description: Write BCM541xx IEEE standard registers utility
 *
 * Input: none
 *
 * Return: none
 */
static void bcm541xx_test_mode_util(void)
{
    int port_max = 3;
    int port_min = 0;
    int bus_id = SMI_BUS_0;
    int phy_addr, portnum, test_mode;

    portnum = gethex_answer("\nEnter port num (0x0 - 0x3)", port_min, port_min, port_max);
    printf("Normal Operation : 0\n");
    printf("Transmit Waveform Test : 1\n");
    printf("Master Transmit Jitter Test : 2\n");
    printf("Slave Transmit Jitter Test : 3\n");
    printf("Transmit Distortion Test : 4\n");
    test_mode = gethex_answer("\nEnter PHY 802.3 test mode", 0, 0, 4);

    phy_addr = ge_port_mapping_phy_addr[portnum];

    bcm54194_transmit_test_pattern(bus_id, phy_addr, test_mode);
}

/*-------------------------------------------------
$Log: bcm54194_test.c,v $
Revision 1.3  2018/10/03 09:53:57  meho
Added test coverage between BCM PHY and SFP/SFP+ via I2C interface.

Revision 1.2  2018/05/18 09:24:53  alpeng
 Neptune merge to trunk with tag <neptune-branch-0518>

Revision 1.1.2.32  2018/05/11 07:32:28  meho
Removed Fiber SGMII AN ADV register from bcm54194 reg test.

Revision 1.1.2.31  2018/04/16 08:47:58  meho
Corrected GLC-GE-100FX utility name.

Revision 1.1.2.30  2018/04/16 08:42:19  meho
Added GLC-GE-100FX SFP loopback utility.

Revision 1.1.2.29  2017/12/29 06:28:11  meho
Workaround for BCM54194 B0 silicon MDIO address issue.

Revision 1.1.2.28  2017/10/31 02:47:51  meho
Added comment on BCM54194 PTP scripts.

Revision 1.1.2.27  2017/10/30 08:52:57  meho
Added 1588 config script for BCM54194.

Revision 1.1.2.26  2017/10/18 09:18:20  meho
Added BCM54194 reset by FPGA.

Revision 1.1.2.25  2017/10/17 09:58:46  meho
Added bcm54194 MACsec test.

Revision 1.1.2.24  2017/06/09 08:05:03  meho
Added warning message for GE PHY internal loopback when turn off Ext flag.

Revision 1.1.2.23  2017/04/10 05:27:24  meho
Integrated BCM82752/82757 API.

Revision 1.1.2.22  2017/04/05 05:47:24  meho
Added GE PHY series number in loopback warning message.

Revision 1.1.2.21  2017/03/17 11:01:09  meho
Increased GE PHY register test coverage.

Revision 1.1.2.20  2017/01/11 03:40:08  meho
Added GE PHY Test Mode Util.

Revision 1.1.2.19  2017/01/09 09:56:04  alpeng
support 10g and ge phy reset and unreset utilities

Revision 1.1.2.18  2016/12/28 09:07:22  meho
Changed the test name of loopback test.

Revision 1.1.2.17  2016/12/27 08:55:48  meho
Fixed show error count bug.

Revision 1.1.2.16  2016/12/27 08:22:42  meho
Corrected the print Pass location.

Revision 1.1.2.15  2016/12/27 06:33:37  meho
Added warning message for GE intternal loopback test.

Revision 1.1.2.14  2016/12/15 02:00:18  meho
Added check external flag for GE loopback test.

Revision 1.1.2.13  2016/11/29 06:27:52  meho
Changed submenu name and code clean up.

Revision 1.1.2.12  2016/11/28 03:43:55  meho
1. Fixed GE phy Mac/Int/Ext loopback test bugs.
2. Added 10G FW download.

Revision 1.1.2.11  2016/09/14 02:44:28  meho
Added BCM54194 I2C r/w utilities.

Revision 1.1.2.10  2016/08/12 10:12:18  meho
Clean up code.

Revision 1.1.2.9  2016/07/26 10:09:43  meho
Added 10G PHY PTP1588 loopback test skeleton.

Revision 1.1.2.8  2016/07/26 07:54:26  meho
Added GE PHY PTP1588 loopback test skeleton.

Revision 1.1.2.7  2016/07/25 11:28:29  meho
Added register dump utility for BCM82752.

Revision 1.1.2.6  2016/07/25 09:05:41  meho
Added register dump utility for BCM54194.

Revision 1.1.2.5  2016/07/22 03:48:57  meho
Added BCM82757 MACsec skeleton.

Revision 1.1.2.4  2016/07/21 09:43:12  meho
Added GE PHY MACsec skeleton.

Revision 1.1.2.3  2016/07/20 01:44:59  meho
Added GE PHY loopback debug utilities.

Revision 1.1.2.2  2016/07/12 08:40:58  meho
1. Added BCM54194/BCM82752 register tests.
2. Added BCM54194 internal/external-copper loopback configuration.

Revision 1.1.2.1  2016/07/07 09:04:29  meho
1. Added BCM54194 RDB register r/w utility.
2. Added GE PHY internal/external loopback skeleton.
3. Added 10GE PHY internal/external loopback skeleton.



$Endlog$
*/
