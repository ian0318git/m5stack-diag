/* $Id: diag_sideband_test.c,v 1.2 2021/06/02 08:22:34 iachang Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/fugazi/diag_sideband_test.c,v $
 *------------------------------------------------------------------
 *
 * diag_sideband_test.c - Fugazi Side Band GPIO test.
 *
 * Mar. 2020, Ian Chang <iachang@cisco.com>
 *
 * Copyright (c) 2020 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
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
#include <assert.h>

#include "defs.h"
#include "types.h"
#include "proto.h"
#include "common.h"
#include "common_utils.h"
#include "monitor.h"
#include "menu.h"
#include "setjmps.h"
#include "nvmonvars.h"
#include "error.h"
#include "router_if.h"
#include "sgmii_defs.h"
#include "ethernet.h"
#include "linux_eth.h"
#include "platform_eth.h"
#include "ethernet.h"  /* for SFPx port definition */
#include "dash_fpga.h" /* for get SFP ctrl reg */  
#include "queryflags.h" /* for query user functions */  
#include "diag_bcm54194_api.h"
#include "platform_ext_lpbk.h"
#include "platform_i2c.h"
#include "diag_bnxt.h"
#include "diag_bcm_lib.h"
#include "diag_miura_reg.h"
#include "diag_bcm57412_utils.h"
#include "diag_bcm82757_test.h"
#include "platform_ext_lpbk.h"
#include "diag_bcm57412_utils.h"
#include "diag_sideband_test.h"

#define F_GRP       (MF_CONTINUOUS | MF_DOGRP)
#define F_GRP_E     (F_GRP | MF_SHOW_ERRCOUNT)
#define F_ALL       (F_GRP | MF_DOALL)
#define F_ALL_E     (F_ALL | MF_SHOW_ERRCOUNT)


int fugazi_bcm82757_set_sfp_present(fugazi_lane_t);
int bcm82752_emphasis_setting(void);
static long bcm82757_side_band_dump(void);    
static int bcm54194_side_band_test(void);
static int bcm54194_side_band_test_f(int, int);
long bcm82757_side_band_test(void);
    
/******************************************************************************
 *  List of Utilities used for Side Band 
 *****************************************************************************/
static submenu_xtable_t sideband_util_items[] = {
    {"BCM82757 Register Read", bcm82757_reg_read, 0,
     0, NULL, 0, NULL, 0},
    {"BCM82757 Register Write", bcm82757_reg_write, 0,
     0, NULL, 0, NULL, 0},
    {"BCM82757 Side_band register dump", bcm82757_side_band_dump, 0,
     0, NULL, 0, NULL, 0},
    {"BCM57412 sideband tx_dis", (type_t(*)())bcm57412_sideband_tx_dis, 0, 0,
     (type_t(*)())0, 0, (type_t(*)())0, 0},
    {"BCM82757 initialization", bcm82757_init_f, 0,
     0, NULL, 0, NULL, 0},
};
#define SIDEBAND_TESTS_UTIL_SIZE (sizeof(sideband_util_items) / sizeof(submenu_xtable_t))
static mitem_t sideband_tests_primary_util_items[SIDEBAND_TESTS_UTIL_SIZE +
                                                 MAX_BASE_ITEMS];
static mitem_t sideband_tests_secondary_util_items[SIDEBAND_TESTS_UTIL_SIZE +
                                                   MAX_BASE_ITEMS];

menuinfo_t sideband_util_menu = {
    "Side Band Utility Menu",
    0,
    (PFT)show_endnote,
    0,
    0,
    sideband_tests_primary_util_items,
};
menuinfo_t *sideband_util_menup = &sideband_util_menu;

/*******************************************************************************
 *
 * Function    : sideband_utility
 * Description :
 * Inputs      : menu_option - display utility for lasi test
 *               tests.
 * Outputs     : PASSED / FAILED
 *
 ******************************************************************************/
static int sideband_utility (int show_menu)
{
    build_primary_submenu(sideband_util_items, SIDEBAND_TESTS_UTIL_SIZE,
                          "Side Band Utilities Menu", &sideband_util_menup);
    build_secondary_submenu(sideband_util_items, SIDEBAND_TESTS_UTIL_SIZE,
                            sideband_tests_secondary_util_items);

    menu(sideband_util_menup, sideband_tests_secondary_util_items, '\0' );

    return (PASSED);
}


/* Side Band submenu items */
static submenu_xtable_t sideband_submenu_table[] = {
    {"BCM82757 Side Band Test", bcm82757_side_band_test, FUGAZI_LANE_0,
     F_ALL_E, NULL, 0, NULL, 0},
    {"BCM54194 Side Band Test", (type_t(*)())bcm54194_side_band_test,   0,
     F_ALL_E, (type_t(*)())0, 0, (type_t(*)())0,   0},
    {"Side Band Utility", (type_t(*)())sideband_utility, FALSE,
     0, NULL, 0, (type_t(*)())sideband_utility, TRUE},
};

#define SIDEBAND_SUBMENU_TABLE_SZ (sizeof(sideband_submenu_table) / sizeof(submenu_xtable_t))

static mitem_t sideband_submenu_primary_items[SIDEBAND_SUBMENU_TABLE_SZ+MAX_BASE_ITEMS];
static mitem_t sideband_submenu_secondary_items[SIDEBAND_SUBMENU_TABLE_SZ+MAX_BASE_ITEMS];

static char sideband_submenu_title[] = "Fugazi Side Band Subtest Menu";

static menuinfo_t sideband_submenu = {
    sideband_submenu_title,
    0,                          /* mtparam added by init_empty_menu */
    (PFT)show_endnote,          /* notes missing WICs in combos */
    0,                          /* use generic prompt */
    0,                          /* size (bumped by add_menu_item() */
    sideband_submenu_primary_items,
};

static menuinfo_t *sideband_submenup = &sideband_submenu;

/*
 * Function: bcm82757_side_band_dump
 *
 * Description: BCM82757 side band register read
 *
 * Inputs      : NONE
 * Outputs     : PASSED / FAILED
 */
static long bcm82757_side_band_dump (void)
{
    int rc;
    uint32_t data, regaddr, devaddr = FUGAZI_MIURA_DEV_PMA_PMD;
    uint32_t rx_los_reg, tx_flt_reg, mod_abs;
    fugazi_lane_t lane;
    fugazi_if_side_t if_side;
    int ix;
    
    if_side = FUGAZI_IF_SIDE_LINE;
    devaddr = FUGAZI_MIURA_DEV_PMA_PMD; 
    /* rx_los status reg. = 0x8a5f */
    /* tx_fault status reg. = 0x8A67 */
    /* mod_abs status reg. = 0x8A6F (SFP present) */
    rx_los_reg = RX_LOS_STATUS_REG;
    tx_flt_reg = TX_FLT_STATUS_REG;
    mod_abs    = MOD_ABS_STATUS_REG; 
    for (ix = 0; ix < BCM82757_LANE_MAX ; ix++) {
        regaddr = rx_los_reg + ((ix % 2) * 2);
        lane = ix;
        rc = fugazi_bcm82757_read(fugazi_struct, lane, if_side, devaddr, 
                                  regaddr, &data);
        if (rc < 0) {
            cterr('f', 0, "BCM82757 port %d read error", ix);
            return (FAILED);
        }
        printf("Port %d rx_los   : %d.%#.4x --> %#.4x\n",ix, devaddr, regaddr, 
                data);
        regaddr = tx_flt_reg + ((ix % 2) * 2); 
        rc = fugazi_bcm82757_read(fugazi_struct, lane, if_side, devaddr, 
                                  regaddr, &data);
        if (rc < 0) {
            cterr('f', 0, "BCM82757 port %d read error", ix);
            return (FAILED);
        }
        printf("Port %d tx_fault : %d.%#.4x --> %#.4x\n",ix, devaddr, regaddr, 
                data);
        regaddr = mod_abs + ((ix % 2) * 2);
        rc = fugazi_bcm82757_read(fugazi_struct, lane, if_side, devaddr, 
                                  regaddr, &data);
        if (rc < 0) {
            cterr('f', 0, "BCM82757 port %d read error", ix);
            return (FAILED);
        }
        printf("Port %d mod_abs  : %d.%#.4x --> %#.4x\n\n",ix, devaddr, regaddr, 
                data);
    }
    return (PASSED);
}

/*
 * Function: bcm82757_side_band_test
 *
 * Description: BCM82757 side band test
 *              Test BCM82757 side-band, four signls on SFP, tx_dis, sfp_abs, tx_fault, rx_los. 
 *              In this test. After control tx_dis from BCM57412 MAC (input to SFP); 
 *              read sfp_abs, tx_fault, rx_los from PHY (SFP output to PHY) :
 *              with SFP and ext loopback:
 *              tx_dis             sfp_abs  tx_fault  rx_los
 *              0 (disable)        0        0         0
 *              1 (enable)         0        0         1
 *
 *              without SFP:
 *              tx_dis             sfp_abs  tx_fault  rx_los
 *              0 (disable)        1        1         1
 *              1 (enable)         1        1         1 
 * Inputs      : NONE
 * Outputs     : PASSED / FAILED
 */
long bcm82757_side_band_test (void)
{
    int rc;
    uint32_t data, regaddr, devaddr = FUGAZI_MIURA_DEV_PMA_PMD;
    uint32_t rx_los_reg, tx_flt_reg, mod_abs;
    fugazi_lane_t lane;
    fugazi_if_side_t if_side;
    int ix, jx, result = PASSED;
    
    testname("BCM82757 Side Band");

    if_side = FUGAZI_IF_SIDE_LINE;
    devaddr = FUGAZI_MIURA_DEV_PMA_PMD; 
    /* rx_los status reg. = 0x8a5f */
    /* tx_fault status reg. = 0x8A67 */
    /* mod_abs status reg. = 0x8A6F (SFP present) */
    rx_los_reg = RX_LOS_STATUS_REG;
    tx_flt_reg = TX_FLT_STATUS_REG;
    mod_abs    = MOD_ABS_STATUS_REG; 
    if ((NVRAM)->diagflag & D_EXT_LOOPBACK) {
        printf("\nPleaes remove the SFP and loopback\n");
        /* Not plug-in SFP, rx_los,tx_flt,mod_abs were high */
        for (ix = 0; ix < BCM82757_LANE_MAX ; ix++) {
            prpass(testpass, "Port %x Side Band Test, ",ix);
            /* RX_LOS Test */
            regaddr = rx_los_reg + ((ix % 2) * 2);
            lane = ix;
            rc = fugazi_bcm82757_read(fugazi_struct, lane, if_side, devaddr, 
                                      regaddr, &data);
            if (rc < 0) {
                cterr('f', 0, "BCM82757 port %d read error", ix);
                result = FAILED; 
            }
            if (!(data & PIN_ACERT_VALUE)) {
                cterr('f', 0, "BCM82757 port %d rx_los (0x%x) error", ix, data);
                result = FAILED; 
            }
            
            /* TX_FAULT Test */
            regaddr = tx_flt_reg + ((ix % 2) * 2);
            rc = fugazi_bcm82757_read(fugazi_struct, lane, if_side, devaddr, 
                                      regaddr, &data);
            if (rc < 0) {
                cterr('f', 0, "BCM82757 read error");
                result = FAILED; 
            }
            if (!(data & PIN_ACERT_VALUE)) {
                cterr('f', 0, "BCM82757 port %d tx_fault (0x%x) error", ix, data);
                result = FAILED; 
            }
            
            /* SFP_PRESENT Test */
            regaddr = mod_abs + ((ix % 2) * 2);
            rc = fugazi_bcm82757_read(fugazi_struct, lane, if_side, devaddr, 
                                      regaddr, &data);
            if (rc < 0) {
                cterr('f', 0, "BCM82757 port %d read error", ix);
                result = FAILED; 
            }
            if (!(data & PIN_ACERT_VALUE)) {
                cterr('f', 0, "BCM82757 port %d sfp_present (0x%x) error", ix, data);
                result = FAILED; 
            }
            if (result == PASSED) {
                printf("\nPort %d Side Band Pass, ",ix);
            }
        }
    } else {
        printf("\nPleaes plug-in the SFP and loopback\n");
        /* Test1: tx_dis enable, rx_los=high, tx_flt,sfp_present=low */
        for (ix = 0; ix < BCM82757_LANE_MAX ; ix++) {
            prpass(testpass, "Port %x TX_DIS Enable, ",ix);
            result = bcm57412_sideband_tx_dis_setup(ix, ENABLE);
            msleep(SIDEBAND_ASSERT_TIME);
            /* RX_LOS Test */
            regaddr = rx_los_reg + ((ix % 2) * 2);
            lane = ix;
            for (jx = 0; jx < SIDEBAND_TIMEOUT ; jx++) {
                rc = fugazi_bcm82757_read(fugazi_struct, lane, if_side, devaddr, 
                                          regaddr, &data);
                if (rc < 0) {
                    cterr('f', 0, "BCM82757 port %d read error", ix);
                    result = FAILED; 
                }
                if (!(data & PIN_ACERT_VALUE)) {
                    result = FAILED;
                    msleep(SIDEBAND_ASSERT_TIME);
                    continue; 
                }
                break; 
            }            
            if ((jx == SIDEBAND_TIMEOUT)) {
                cterr('f', 0, "BCM82757 port %d rx_los (0x%x) error", ix, data);
                result = FAILED; 
            }
            
            /* TX_FAULT Test */
            regaddr = tx_flt_reg + ((ix % 2) * 2);
            rc = fugazi_bcm82757_read(fugazi_struct, lane, if_side, devaddr, 
                                      regaddr, &data);
            if (rc < 0) {
                cterr('f', 0, "BCM82757 port %d read error", ix);
                result = FAILED; 
            }
            if ((data & PIN_ACERT_VALUE)) {
                cterr('f', 0, "BCM82757 port %d tx_fault (0x%x) error", ix, data);
                result = FAILED; 
            }
            
            /* SFP_PRESENT Test */
            regaddr = mod_abs + ((ix % 2) * 2);
            rc = fugazi_bcm82757_read(fugazi_struct, lane, if_side, devaddr, 
                                      regaddr, &data);
            if (rc < 0) {
                cterr('f', 0, "BCM82757 port %d read error", ix);
                result = FAILED; 
            }
            if ((data & PIN_ACERT_VALUE)) {
                cterr('f', 0, "BCM82757 port %d sfp_present (0x%x) error", ix, 
                       data);
                result = FAILED; 
            }
            if (result == PASSED) {
                printf("\nPort %d Side Band Pass, ",ix);
            }
        }
        /* Test2: tx_dis disable,  rx_los=low, tx_flt, sfp_present=low */
        for (ix = 0; ix < 4 ; ix++) {
            prpass(testpass, "Port %x TX_DIS Disable, ",ix);
            result = bcm57412_sideband_tx_dis_setup(ix, DISABLE);
            msleep(SIDEBAND_ASSERT_TIME);
            /* RX_LOS Test */
            regaddr = rx_los_reg + ((ix % 2) * 2);
            lane = ix;
            for (jx = 0; jx < SIDEBAND_TIMEOUT ; jx++) {
                rc = fugazi_bcm82757_read(fugazi_struct, lane, if_side, devaddr, 
                                          regaddr, &data);
                if (rc < 0) {
                    cterr('f', 0, "BCM82757 port %d read error", ix);
                    result = FAILED; 
                }
                if ((data & PIN_ACERT_VALUE)) {
                    result = FAILED;
                    msleep(SIDEBAND_ASSERT_TIME);
                    continue; 
                }
                break; 
            }            
            if ((jx == SIDEBAND_TIMEOUT)) {
                cterr('f', 0, "BCM82757 port %d rx_los (0x%x) error", ix, data);
                result = FAILED; 
            }

            /* TX_FAULT Test */
            regaddr = tx_flt_reg + ((ix % 2) * 2);
            rc = fugazi_bcm82757_read(fugazi_struct, lane, if_side, devaddr, 
                                      regaddr, &data);
            if (rc < 0) {
                cterr('f', 0, "BCM82757 port %d read error", ix);
                result = FAILED; 
            }
            if ((data & PIN_ACERT_VALUE)) {
                cterr('f', 0, "BCM82757 port %d tx_fault (0x%x) error", ix, data);
                result = FAILED; 
            }
            
            /* SFP_PRESENT Test */
            regaddr = mod_abs + ((ix % 2) * 2);
            rc = fugazi_bcm82757_read(fugazi_struct, lane, if_side, devaddr, 
                                      regaddr, &data);
            if (rc < 0) {
                cterr('f', 0, "BCM82757 port %d read error", ix);
                result = FAILED; 
            }
            if ((data & PIN_ACERT_VALUE)) {
                cterr('f', 0, "BCM82757 port %d sfp_present (0x%x) error", ix, 
                       data);
                result = FAILED; 
            }
            if (result == PASSED) {
                printf("\nPort %d Side Band Pass, ",ix);
            }
        }
    }
    return (result);
}
/*
 * Function: bcm54194_side_band_test_f
 *
 * Description: Test BCM54194 side-band, four signls on SFP, tx_dis, sfp_abs, tx_fault, rx_los. 
 *              In this test. After control tx_dis from BCM57412 MAC (input to SFP); 
 *              read sfp_abs, tx_fault, rx_los from PHY (SFP output to PHY) 
 *     rx_los : RDB 0x811, bit[14]=1 (config LED for RX_LOS)
 *              RDB 0x814, bit[0] =1 (LED mode disable)
 *              RDB 0x237, bit[5] =1 (Enable signal detect mode per PHY port)
 *   tx_fault : RDB 0x811, bit[14]=1 (config LED for TX_FAULT)
 *
 * Input: phy_num  - BCM54194 port number
 *        phy_addr - BCM54194 PHY address
 *
 * Outputs: PASSED/FAILED
 */
static int bcm54194_side_band_test_f (int phy_num, int phy_addr)
{
    ushort rdval = 0, wrval = 0;
    int rc = FAILED, regnum;
    /* RDB 0x811, bit[14]=1 (config LED for RX_LOS) */
    regnum = BCM54194_TOP_LEVEL_PIN_CTRL_REG;
    rc = bcm54194_rdb_read(phy_num, phy_addr, regnum, &rdval);
    if (rc < 0) {
        printf("Failed to read GE port %d, phy addr:%d, RDB offset:0x%x\n", 
                phy_num - 2, phy_addr, regnum);
    }
    wrval =  rdval | BCM54194_SFP_TXFLT_RXLOS_EN;
    rc = bcm54194_rdb_write(phy_num, phy_addr, regnum, wrval);
    if (rc < 0) {
        printf("Failed to write GE port, phy addr:0x%x, RDB offset:0x%x, "
               "value=0x%#.4x\n", phy_addr, regnum, wrval);
    }
    if (diagflag_xram & D_DEBUG_OPTIONS) {
        rc = bcm54194_rdb_read(phy_num, phy_addr, regnum, &rdval);
        printf("port%d, RDB:0x%x = %#.4x, \n", phy_addr, regnum, rdval);
    } 
    
    /* RDB 0x814, bit[0] =1 (LED mode disable) */
    regnum = 0x814;
    rc = bcm54194_rdb_read(phy_num, phy_addr, regnum, &rdval);
    if (rc < 0) {
        printf("Failed to read GE port %d, phy addr:%d, RDB offset:0x%x\n", 
                phy_num - 2, phy_addr, regnum);
    }
    wrval =  rdval | SET_PHY_BIT0;
    rc = bcm54194_rdb_write(phy_num, phy_addr, regnum, wrval);
    if (rc < 0) {
        printf("Failed to write GE port, phy addr:0x%x, RDB offset:0x%x, "
               "value=0x%#.4x\n", phy_addr, regnum, wrval);
    }
    if (diagflag_xram & D_DEBUG_OPTIONS) {
        rc = bcm54194_rdb_read(phy_num, phy_addr, regnum, &rdval);
        printf("port%d, RDB:0x%x = %#.4x, \n", phy_addr, regnum, rdval);
    } 
    
    /* RDB 0x237, bit[5] =1 (Enable signal detect mode per PHY port) */
    regnum = BCM54194_MISC_1000X_CTRL_2_REG;
    rc = bcm54194_rdb_read(phy_num, phy_addr, regnum, &rdval);
    if (rc < 0) {
        printf("Failed to read GE port %d, phy addr:%d, RDB offset:0x%x\n", 
                phy_num - 2, phy_addr, regnum);
    }
    wrval =  rdval | BCM54194_SIG_DET_EN;
    rc = bcm54194_rdb_write(phy_num, phy_addr, regnum, wrval);
    if (rc < 0) {
        printf("Failed to write GE port, phy addr:0x%x, RDB offset:0x%x, "
               "value=0x%#.4x\n", phy_addr, regnum, wrval);
    }
    if (diagflag_xram & D_DEBUG_OPTIONS) {
        rc = bcm54194_rdb_read(phy_num, phy_addr, regnum, &rdval);
        printf("port%d, RDB:0x%x = %#.4x, \n", phy_addr, regnum, rdval);
    } 
    regnum = BCM54194_MISC_1000X_CTRL_2_REG;
    rc = bcm54194_rdb_read(phy_num, phy_addr + 1, regnum, &rdval);
    if (rc < 0) {
        printf("Failed to read GE port %d, phy addr:%d, RDB offset:0x%x\n", 
                phy_num - 2, phy_addr, regnum);
    }
    wrval =  rdval | BCM54194_SIG_DET_EN;
    rc = bcm54194_rdb_write(phy_num, phy_addr + 1, regnum, wrval);
    if (rc < 0) {
        printf("Failed to write GE port, phy addr:0x%x, RDB offset:0x%x, "
               "value=0x%#.4x\n", phy_addr, regnum, wrval);
    }
    if (diagflag_xram & D_DEBUG_OPTIONS) {
        rc = bcm54194_rdb_read(phy_num, phy_addr + 1, regnum, &rdval);
        printf("port%d, RDB:0x%x = %#.4x, \n", phy_addr, regnum, rdval);
    } 

    /* read RDB 0x890 SFP pin change status */ 
    regnum = BCM54194_TOP_MISC_SFP_STS0_REG;
    rc = bcm54194_rdb_read(phy_num, phy_addr, regnum, &rdval);
    if (rc < 0) {
        printf("Failed to read GE port %d, phy addr:%d, RDB offset:0x%x\n", 
                phy_num - 2, phy_addr, regnum);
    }
    if (diagflag_xram & D_DEBUG_OPTIONS) {
        printf("port%d, RDB:0x%x = %#.4x, \n", phy_addr, regnum, rdval);
    } 
    
    if ((NVRAM)->diagflag & D_EXT_LOOPBACK) {
        if (!((rdval == 0x0) || (rdval == 0x0FFF))) {
            cterr('f', 0, "BCM54194 port %d side band expected (0x0) and "
                  "read (0x%x) ", phy_addr, rdval);
            rc = FAILED; 
        }
    } else {
        /* Not test tx_fault due to it is not changed on 1G PHY SFP */
        /* 0x3024 bit define 
         * bit 13 :  SFP port 1 connected
         * bit 12 :  SFP port 0 connected
         * bit  5 :  SFP port 1 RX loss state change
         * bit  2 :  SFP port 0 RX loss state change */
        if (!(rdval & BCM54194_SFP_P0_P1_SIDE_BAND_CHG)) { /* 0x3024 */
            cterr('f', 0, "BCM54194 port %d side band expected (0x3024) and "
                  "read (0x%x) ", phy_addr, rdval);
            rc = FAILED; 
        }
    }
    if (diagflag_xram & D_DEBUG_OPTIONS) {
        regnum = BCM54194_TOP_MISC_SFP_STS0_REG;
        rc = bcm54194_rdb_read(phy_num, phy_addr, regnum, &rdval);
        if (rc < 0) {
            printf("Failed to read GE port %d, phy addr:%d, RDB offset:0x%x\n", 
                    phy_num-2, phy_addr, regnum);
        }
        if (diagflag_xram & D_DEBUG_OPTIONS) {
            printf("port%d, RDB:0x%x = %#.4x, \n", phy_addr, regnum, rdval);
        } 
    } 
    return (rc);
}

/*
 * Function: bcm54194_side_band_test
 *
 * Description: Test BCM54194 side-band, four signls on SFP, tx_dis, sfp_abs, tx_fault, rx_los. 
 *              In this test. After control tx_dis from BCM57412 MAC (input to SFP); 
 *              read sfp_abs, tx_fault, rx_los from PHY (SFP output to PHY) 
 *     rx_los : RDB 0x811, bit[14]=1 (config LED for RX_LOS)
 *              RDB 0x814, bit[0] =1 (LED mode disable)
 *              RDB 0x237, bit[5] =1 (Enable signal detect mode per PHY port)
 *   tx_fault : RDB 0x811, bit[14]=1 (config LED for TX_FAULT)
 *
 * Input: none
 *
 * Outputs: PASSED/FAILED
 */
static int bcm54194_side_band_test (void)
{
    int ix;
    ushort rdval = 0;
    int rc = FAILED, phy_addr, regnum = 0;
    int phy_num = 0;

    testname("BCM54194 Side Band test");
    /* Case1: tx_dis Enable */
    for (ix = 0; ix < 8; ix += 2) {
        /* read to clear the SFP change status*/
        regnum = BCM54194_TOP_MISC_SFP_STS0_REG;
        rc = bcm54194_rdb_read(phy_num, phy_addr, regnum, &rdval);
        if (rc < 0) {
            printf("Failed to read GE port %d, phy addr:%d, RDB offset:0x%x\n", 
                    phy_num-2, phy_addr, regnum);
        }
        /* enable tx_dis */
        rc = bcm57412_sideband_tx_dis_setup(ix + 4, ENABLE);
        rc = bcm57412_sideband_tx_dis_setup(ix + 5, ENABLE);
        msleep(100);
        phy_num = ge_phy_mapping_phy_num[ix / 2];
        phy_addr = ge_port_mapping_phy_addr_down[ix + 4];
        if (diagflag_xram & D_DEBUG_OPTIONS) {
            printf("\nDBG : ix = %d,phy_num = %d , phy_addr = %d\n", ix, 
                    phy_num, phy_addr);
        }
        rc = bcm54194_side_band_test_f(phy_num, phy_addr);
        if (rc == FAILED) {
            cterr('f', 0, "BCM54194 port %d Side Band Failed", ix);
        } else {
            printf("\nPort %d, %d Side Band Pass, ",ix + 4, ix + 5);
        }
    }
    /* Case2: tx_dis Disable */
    for (ix = 0; ix < 8; ix += 2) {
        rc = bcm57412_sideband_tx_dis_setup(ix + 4, DISABLE);
        rc = bcm57412_sideband_tx_dis_setup(ix + 5, DISABLE);
        msleep(100);
        phy_num = ge_phy_mapping_phy_num[ix / 2];
        phy_addr = ge_port_mapping_phy_addr_down[ix + 4];
        if (diagflag_xram & D_DEBUG_OPTIONS) {
            printf("\nDBG : ix = %d,phy_num = %d , phy_addr = %d\n", ix, 
                    phy_num, phy_addr);
        }
        rc = bcm54194_side_band_test_f(phy_num, phy_addr);
        if (rc == FAILED) {
            cterr('f', 0, "BCM54194 port %d Side Band Failed", ix);
        } else {
            printf("\nPort %d, %d Side Band Pass, ",ix + 4, ix + 5);
        }
    }
    return (rc);
}
/*
 * Function: fugazi_sideband_test
 *
 * Description: Build the Side Band main menu
 *
 * Inputs      : show_menu - display menu instead of running all Side Band tests.
 * Outputs     : PASSED / FAILED
 */
int fugazi_sideband_test (int show_menu)
{
    fugazi_lane_t lane;
    int lane_start, lane_end, rc;
    testname("Side Band");

    lane_start = 0;
    lane_end = MAX_NR_FUGAZI_LANE;
    /* Initialize bcm82757 10G PHY */
    if ((fugazi_bcm82757_init(fugazi_struct))) {
        cterr('f', 0, "fugazi_bcm82757_init failed");
    }
    for (lane = lane_start; lane < lane_end; lane++) {
    	printf("\n\rPHY port %d initialization ... ", lane);
		rc |= bcm82757_PHY_init(lane, FUGAZI_PORT_SPEED_10G);
	    if (rc < 0) {
	    	printf("failed.");
	    } else {
	    	printf("successful.");
	    }
	}

    /* Reset and initial CM54194_1G PHY */
    bcm54194_reset(0);
    msleep(BCM82757_LASI_WAIT_TIME); /* Waite for PHY initial */

    build_primary_submenu(sideband_submenu_table, SIDEBAND_SUBMENU_TABLE_SZ,
                          sideband_submenu_title, &sideband_submenup);
    build_secondary_submenu(sideband_submenu_table, SIDEBAND_SUBMENU_TABLE_SZ,
                            sideband_submenu_secondary_items);

    /* Emphasis compliance setting */
    bcm82752_emphasis_setting();

    if (show_menu) {
        menu_exec_doall_diags(sideband_submenup);
    } else {
        menu(sideband_submenup, sideband_submenu_secondary_items, '\0');
    }
    return (PASSED);
}


/*
 *------------------------------------------------------------------
 * $Log: diag_sideband_test.c,v $
 * Revision 1.2  2021/06/02 08:22:34  iachang
 * CSCvo59196-33 : Merge Fugazi from ASR1K-main-branch to ISR main trunk
 *
 * Revision 1.1.4.3  2020/12/03 11:13:20  iachang
 * Add PHY initial before side-band test.
 *
 * Revision 1.1.4.2  2020/08/26 02:37:49  iachang
 * Merge Fugazi code into main trunk
 *
 * Revision 1.1.2.8  2020/08/25 04:11:44  pdoong
 * Correct comment for side-band test.
 *
 * Revision 1.1.2.7  2020/08/24 00:00:26  pdoong
 * Change to use BCM54194 register defintion macro.
 *
 * Revision 1.1.2.6  2020/08/20 11:29:49  iachang
 * PRRQ CSCvo59196-8 : LASI and Side Band code review
 *
 * Revision 1.1.2.5  2020/08/06 02:43:30  iachang
 * Code clean up.
 *
 * Revision 1.1.2.4  2020/08/03 09:25:40  iachang
 * Code clean up.
 *
 * Revision 1.1.2.3  2020/07/24 03:44:58  iachang
 * tx_dis chang, add time out for rx_los checking
 *
 * Revision 1.1.2.2  2020/06/18 03:43:18  iachang
 * Enhance side band test error log.
 *
 * Revision 1.1.2.1  2020/03/19 06:31:40  iachang
 * Support Fugazi Side Band test
 *
 *
 *------------------------------------------------------------------
 * $Endlog$
 */
