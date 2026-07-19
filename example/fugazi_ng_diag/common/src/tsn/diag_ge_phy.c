/* $Id: diag_ge_phy.c,v 1.11 2019/01/24 01:07:22 letsai Exp $ 
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/tsn/diag_ge_phy.c,v $
 *-----------------------------------------------------------------------------
 *
 * Filename   : diag_ge_phy.c
 * Description: TSN GE PHY(Marvell 88E1112) Diag tests and utilities.
 *
 * Copyright (c) 2016~2019 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *-----------------------------------------------------------------------------
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
#include "plat_defs.h"
#include "platform_cpu.h"
#include "platform_ge_phy.h"
#include "smi_api.h"
#include "dev_mrvl_ge.h"
#include "diag_ge_phy.h"
#include "tsn_comm.h"
#include "platform_ext_lpbk.h"
#include "platform_fru.h"
#include "platform_cookie.h"
#include "platform_fpga.h"
#include "platform_sensor.h"
#include "platform_smi.h"
#include "i2c_api.h"
#include "platform_sfp_cookie.h"
#include "platform_i2c.h"

extern boolean has_sfp(int);
extern int sfp_phy_ext_lpbk_test(int);
extern int tsn_ge_led_utils(int);
/*******************************************************************************
 *                          Function Declaration
 *******************************************************************************
 */
int ge_phy_copper_ext_lpbk_test(int);
static int ge_phy_sfp_ext_lpbk_test(int);
int ge_send_packet_util(int);
int        tsn_gephy_reg_test(int);
int        tsn_gephy0_utils(int);
int        tsn_gephy1_utils(int);
int gephy_set_test_mode(int);
int        tsn_set_gephy_txtype(int, ushort);
int gephy_set_txtype_util(int);
int gephy_set_default(int);
static int gephy_copper_mac_lpbk_test(int);
int        tsn_wan_show_phy_status(int);
int gephy_set_1000basex_mode(int);
int gephy_set_sgmii_mode(int);
static int diag_ge_phy_intr_test(int);
static uint32_t diag_ge_smi_read(int, smi_if_t *);
static uint32_t diag_ge_smi_write(int, smi_if_t *);
static int mrvl_88e1112_ge_polling_reg(int, int, smi_t, uchar, smi_t);
static int diag_ge_chk_intr_assert(int);
static int diag_ge_chk_intr_deassert(int);
static int mrvl_88e1112_ge_chk_intr_assert(int);
static int mrvl_88e1112_ge_intr_test(int);
static int mrvl_88e1112_ge_intr_disable(int);
static int mrvl_88e1112_ge_intr_enable(int);
static int mrvl_88e1112_ge_rd_wr_reg(int, uint, smi_t, uchar, smi_t *);
int has_mrvl_88e1112(void);

/*******************************************************************************
 *                                Globals
 *******************************************************************************
 */
#define ENHANCE_ERROR_MSG_RDY 1
uint sfp_type = SFP_DEFAULT;
uint sfp_encode = SFP_ENCODE_UNKNOWN;

/* GE PHY copper speed table */
static int gephy_copper_speed_tbl[] = {SPD_10MBPS, SPD_100MBPS, SPD_1000MBPS};

static submenu_xtable_t tsn_gephy0_diag_tbl[] = {
    {"GE PHY 0 Utilities",
     (type_t(*)())tsn_gephy0_utils,                              FALSE,
     0, 
     (type_t(*)())0,                                             0,
     (type_t(*)())0,                                             0},
    {"PHY Register Test",
     (type_t(*)())tsn_gephy_reg_test,                            TSN_GE0,
     (MF_CONTINUOUS | MF_DOGRP | MF_DOALL | MF_SHOW_ERRCOUNT),
     (type_t(*)())0,                                             0,
     (type_t(*)())0,                                             0},
    {"CPU to PHY MAC Loopback Test",
     (type_t(*)())gephy_copper_mac_lpbk_test,                    TSN_GE0,
     (MF_CONTINUOUS | MF_DOGRP | MF_DOALL | MF_SHOW_ERRCOUNT),
     (type_t(*)())0,                                             0,
     (type_t(*)())0,                                             0},
    {"Copper port External Loopback Test",
     (type_t(*)())ge_phy_copper_ext_lpbk_test,                   TSN_GE0,
     (MF_CONTINUOUS | MF_DOGRP | MF_DOALL | MF_SHOW_ERRCOUNT),
     (type_t(*)())0,                                             0,
     (type_t(*)())0,                                             0},
    {"SFP port External Loopback Test",
     (type_t(*)())ge_phy_sfp_ext_lpbk_test,                      TSN_GE0,
     (MF_CONTINUOUS | MF_DOGRP | MF_DOALL | MF_SHOW_ERRCOUNT),
     (type_t(*)())has_sfp,                0,
     (type_t(*)())0,                                             0},
    {"GE0 Interrupt Test",
     (type_t(*)())diag_ge_phy_intr_test,                         TSN_GE0,
     (MF_CONTINUOUS | MF_DOGRP | MF_DOALL | MF_SHOW_ERRCOUNT),
     (type_t(*)())has_mrvl_88e1112,                             0,
     (type_t(*)())0,                                             0},

};

#define TSN_GEPHY0_DIAG_TBL_SIZE (sizeof(tsn_gephy0_diag_tbl) / \
                                  sizeof(submenu_xtable_t))

/*
 * Primary & secondary submenu items (filled in from xtable)
 */
static mitem_t tsn_gephy0_diag_pri_items[TSN_GEPHY0_DIAG_TBL_SIZE + MAX_BASE_ITEMS];
static mitem_t tsn_gephy0_diag_sec_items[TSN_GEPHY0_DIAG_TBL_SIZE + MAX_BASE_ITEMS];

menuinfo_t tsn_gephy0_diag_menu = {
    "%s Subtest Menu",
    0,                                /* mtparam added by init_empty_menu */
    (PFT)show_endnote,                /* notes missing WICs in combos */
    0,                                /* use generic prompt */
    0,                                /* size (bumped by add_menu_item() */
    tsn_gephy0_diag_pri_items,
};
menuinfo_t *tsn_gephy0_diag_menup = &tsn_gephy0_diag_menu;

static submenu_xtable_t tsn_gephy1_diag_tbl[] = {
    {"GE PHY 1 Utilities",
     (type_t(*)())tsn_gephy1_utils,                              FALSE,
     0,
     (type_t(*)())0,                                             0,
     (type_t(*)())0,                                             0},
    {"PHY Register Test",
     (type_t(*)())tsn_gephy_reg_test,                            TSN_GE1,
     (MF_CONTINUOUS | MF_DOGRP | MF_DOALL | MF_SHOW_ERRCOUNT),
     (type_t(*)())0,                                             0,
     (type_t(*)())0,                                             0},
    {"CPU to PHY MAC Loopback Test",
     (type_t(*)())gephy_copper_mac_lpbk_test,                    TSN_GE1,
     (MF_CONTINUOUS | MF_DOGRP | MF_DOALL | MF_SHOW_ERRCOUNT),
     (type_t(*)())0,                                             0,
     (type_t(*)())0,                                             0},
    {"Copper port External Loopback Test",
     (type_t(*)())ge_phy_copper_ext_lpbk_test,                   TSN_GE1,
     (MF_CONTINUOUS | MF_DOGRP | MF_DOALL | MF_SHOW_ERRCOUNT),
     (type_t(*)())0,                                             0,
     (type_t(*)())0,                                             0},
    {"GE1 Interrupt Test",
     (type_t(*)())diag_ge_phy_intr_test,                         TSN_GE1,
     (MF_CONTINUOUS | MF_DOGRP | MF_DOALL | MF_SHOW_ERRCOUNT),
     (type_t(*)())has_mrvl_88e1112,                             0,
     (type_t(*)())0,                                             0},

};

#define TSN_GEPHY1_DIAG_TBL_SIZE (sizeof(tsn_gephy1_diag_tbl) / \
            sizeof(submenu_xtable_t))

/*
 * Primary & secondary submenu items (filled in from xtable)
 */
static mitem_t tsn_gephy1_diag_pri_items[TSN_GEPHY1_DIAG_TBL_SIZE + MAX_BASE_ITEMS];
static mitem_t tsn_gephy1_diag_sec_items[TSN_GEPHY1_DIAG_TBL_SIZE + MAX_BASE_ITEMS];

menuinfo_t tsn_gephy1_diag_menu = {
    "%s Subtest Menu",
    0,                                /* mtparam added by init_empty_menu */
    (PFT)show_endnote,                /* notes missing WICs in combos */
    0,                                /* use generic prompt */
    0,                                /* size (bumped by add_menu_item() */
    tsn_gephy1_diag_pri_items,
};
menuinfo_t *tsn_gephy1_diag_menup = &tsn_gephy1_diag_menu;

/* List of GE PHY 0 Utilities */
static submenu_xtable_t gephy0_util_items[] = {
    {"GE PHY register read",        (type_t(*)())tsn_gephy_reg_rd_util,
     TSN_GE0_ETHNUM,                0,
     (type_t(*)())0,                0,
     (type_t(*)())0,                0},
    {"GE PHY register write",       (type_t(*)())tsn_gephy_reg_wr_util,
     TSN_GE0_ETHNUM,                0,
     (type_t(*)())0,                0,
     (type_t(*)())0,                0},
    {"CPU register Read",           (type_t(*)())tsn_cpureg_rd_util, 0, 0,
     (type_t(*)())0, 0, (type_t(*)())0, 0},
    {"CPU register Write",          (type_t(*)())tsn_cpureg_wr_util, 0, 0,
     (type_t(*)())0, 0, (type_t(*)())0, 0},
    {"Send packet to GE PHY 0",     (type_t(*)())ge_send_packet_util, TSN_GE0_ETHNUM, 0,
     (type_t(*)())0, 0, (type_t(*)())0, 0},
    {"Set PHY 1000BaseT Test mode", (type_t(*)())gephy_set_test_mode, TSN_GE0_ETHNUM, 0,
     (type_t(*)())0, 0, (type_t(*)())0, 0},
    {"Set PHY Transmitter Type",    (type_t(*)())gephy_set_txtype_util, TSN_GE0_ETHNUM, 0,
     (type_t(*)())0, 0, (type_t(*)())0, 0},
    {"VOD Adjustments",             (type_t(*)())gephy_vod_adj_util,
     TSN_GE0,                       0,
     (type_t(*)())0,                0,
     (type_t(*)())0,                0},
    {"GE0 stat LED utils", (type_t(*)())tsn_ge_led_utils, 0,
     0,
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
    {"GE PHY register read", (type_t(*)())tsn_gephy_reg_rd_util, TSN_GE1_ETHNUM, 0,
     (type_t(*)())0, 0, (type_t(*)())0, 0},
    {"GE PHY register write", (type_t(*)())tsn_gephy_reg_wr_util, TSN_GE1_ETHNUM, 0,
     (type_t(*)())0, 0, (type_t(*)())0, 0},
    {"CPU register Read", (type_t(*)())tsn_cpureg_rd_util, 0, 0,
     (type_t(*)())0, 0, (type_t(*)())0, 0},
    {"CPU register Write", (type_t(*)())tsn_cpureg_wr_util, 0, 0,
     (type_t(*)())0, 0, (type_t(*)())0, 0},
    {"Send packet to GE PHY 1", (type_t(*)())ge_send_packet_util, TSN_GE1_ETHNUM, 0,
     (type_t(*)())0, 0, (type_t(*)())0, 0},
    {"Set PHY 1000BaseT Test mode", (type_t(*)())gephy_set_test_mode, TSN_GE1_ETHNUM, 0,
     (type_t(*)())0, 0, (type_t(*)())0, 0},
    {"Set PHY Transmitter Type", (type_t(*)())gephy_set_txtype_util, TSN_GE1_ETHNUM, 0,
     (type_t(*)())0, 0, (type_t(*)())0, 0},
    {"VOD Adjustments",             (type_t(*)())gephy_vod_adj_util,
     TSN_GE1,                       0,
     (type_t(*)())0,                0,
     (type_t(*)())0,                0},
    {"GE1 stat LED utils", (type_t(*)())tsn_ge_led_utils, 1,
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
 *    
 * Function   : ge_phy_copper_ext_lpbk_test
 * Description: Function to do GE PHY copper port external loopback test.
 * Inputs     : ge_num - phy number
 * Outputs    : PASSED/FAILED
 *               
 *******************************************************************************
 */
int ge_phy_copper_ext_lpbk_test (int ge_num)
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
    //cterr_add_reg_dump((PFV)bcm63168_get_xdsl_profile);

    /* Segment 6: Platform Environment initialized here*/
    cterr_add_env_dump((PFV)tsn_display_temp_errormsg);

    /* Segment 7: Top 3 Debugging Steps */
    cterr_add_debug("Change another external loopback fixture.",
                    "If step a. is still failed, try the internal "
                    "loopback to check if internal loopback is OK.",
                    "If step b. is OK, we can know PHY has problems. "
                    "If step b is failed, please try step d.",
                    "Observe MDIO register status to "
                    "check if PHY configuration is normal.",
                    "If step d. is OK, execute the MAC loopback test.",
                    "If step e. is OK, we can assume the interface "
                    "between Host SoC and PHY has problems.");
#endif

    char tname[32];
    int  eth_num = 0;
    uint orig_media_mode = (uint)MSCR1_MOD_AUTO_COP_SGMII;

    memset(tname, 0, sizeof(tname));

    if (ge_num == TSN_GE0) {
        sprintf(tname, "GE0 Copper Ext. loopback");
        eth_num = TSN_GE0_ETHNUM;
    } else if (ge_num == TSN_GE1) {
        sprintf(tname, "GE1 Copper Ext. loopback");
        eth_num = TSN_GE1_ETHNUM;
    } else {
        cterr('f', 0, "%s(%d): TSN doesn't have GE%d.",
                      __func__, __LINE__, ge_num);
        return (FAILED);
    }

    testname(tname);

    if (gephy_set_default(ge_num) != PASSED) {
        cterr('f', 0, "Failed to set GE%d back to default ", ge_num);
        prcomplete(testpass, errcount, (char *)0);
        return (FAILED);
    }

    /* Check if Ext. Loopback Flag is ON */
    if (check_ext_lpbk_flag() != TRUE) {
        printf("Skip %s test beacuse Ext. Loopback Flag is OFF.\n", tname);
        return (PASSED);
    }

    prpass(testpass, "%s, ", tname);

    /* Read and store current GEWAN PHY media mode */
    if (tsn_gephy_get_media_mode(eth_num, &orig_media_mode) != PASSED) {
        cterr('f', 0, "Failed to get current GEWAN%d media mode ", ge_num);
        return (FAILED);
    }

    /* Set GEWAN PHY media to Copper mode ONLY for testing */
    if (tsn_gephy_set_media_mode(eth_num, (uint)MSCR1_MOD_COP_ONLY) != PASSED) {
        cterr('f', 0, "Failed to set GEWAN%d media to Copper ONLY ", ge_num);
        return (FAILED);
    }

    if (tsn_ge_lpbk_test(eth_num, SGMII_INT_EXT_LPBK) != PASSED) {
        cterr('f', 0, "Failed to do");
        return (FAILED);
    }

    /* Restore GEWAN PHY media back to default after test */
    if (tsn_gephy_set_media_mode(eth_num, orig_media_mode) != PASSED) {
        cterr('f', 0, "Failed to set GEWAN%d media back to default ", ge_num);
        return (FAILED);
    }

    return (PASSED);
}

/*******************************************************************************
 *    
 * Function   : ge_phy_sfp_ext_lpbk_test
 * Description: Function to do GE PHY SFP port external loopback test.
 * Inputs     : ge_num - phy number
 * Outputs    : PASSED/FAILED
 *               
 *******************************************************************************
 */
static int ge_phy_sfp_ext_lpbk_test (int ge_num)
{
    char tname[32];
    int  eth_num = 0;
    uint speed = 0;

    memset(tname, 0, sizeof(tname));
    uint orig_media_mode = (uint)MSCR1_MOD_AUTO_COP_SGMII;

    if (ge_num == TSN_GE0) {
        eth_num = TSN_GE0_ETHNUM;
    } else if (ge_num == TSN_GE1) {
        eth_num = TSN_GE1_ETHNUM;
    } else {
        cterr('f', 0, "%s(%d): TSN doesn't have GE%d.",
                      __func__, __LINE__, ge_num);
        return (FAILED);
    }

    sprintf(tname, "GE%d(eth%d) SFP Ext. loopback", ge_num, eth_num);
    testname(tname);

    /* Check if Ext. Loopback Flag is ON */
    if (check_ext_lpbk_flag() != TRUE) {
        printf("Skip %s test beacuse Ext. Loopback Flag is OFF.\n", tname);
        return (PASSED);
    }

    /* Read and store current GEWAN PHY media mode */
    if (tsn_gephy_get_media_mode(eth_num, &orig_media_mode) != PASSED) {
        cterr('f', 0, "Failed to get current GEWAN%d media mode ", ge_num);
        return (FAILED);
    }

    /* Set flag according to SFP encoding type 
     * 1. Get_sfp_encoding 
     *        sfp_cookie_read
     *    sfp_encode = 
     *        SFP_ENCODE_8B10B (set_sfp_glc_t_1000)
     *        SFP_ENCODE_SONET
     *        SFP_ENCODE_4B5B 
     *            sfp_ge_100fx or sfp_fe_100fx.
     *   */
    switch (sfp_encode = get_sfp_reg(SFP_COO_ENC)) {
        case SFP_ENCODE_8B10B:
            printf("8B10B\n");
            /* Configure the external loopback setting for SFP Copper Module 
             * GLC-TE */
            if (get_sfp_reg(SFP_ETH_COMP_CODES) == SFP_1000BASE_T) {
                printf("Found GLC-TE\n");
                sfp_type = SFP_GLC_TE;
            }
            printf("Speed = 1000BASEX\n");
            speed = (uint)MSCR1_MOD_1000BASEX_ONLY;
            break;
        case SFP_ENCODE_SONET:
            printf("SONET\n");
            printf("Speed = 1000BASEX\n");
            speed = (uint)MSCR1_MOD_1000BASEX_ONLY;
            break;
        case SFP_ENCODE_4B5B:
            printf("4B5B\n");
            if (read_sfp_ext_id(CPU_I2C1) == SFP_XID_GE_100FX) {
                printf("Found GLC-GE-100FX\n");
                printf("Speed = SGMII ONLY\n");
                sfp_type = SFP_GE_100FX;
                speed = (uint)MSCR1_MOD_SGMII_ONLY;
            } else 
            if (read_sfp_ext_id(CPU_I2C1) == SFP_XID_FE_100FX) {
                printf("Speed = 100FX\n");
                sfp_type = SFP_FE_100FX;
                speed = (uint)MSCR1_MOD_100BASE_FX;
            } else {
                printf("Unsupported SFP Ext. ID\n");
                goto restore_default;
            } 
            break;
        default:
            cterr('f', 0, "%s() Unsupported SFP encoding %#x",
                    __func__, sfp_encode);
            goto restore_default;
    };  

    /* Set GEWAN PHY media to 1000Base-X mode ONLY for testing */
    if (tsn_gephy_set_media_mode(eth_num, speed) != PASSED) {
        cterr('f', 0, "Failed to set GEWAN%d media to correct speed", ge_num);
        goto restore_default;
    }

    if (sfp_phy_ext_lpbk_test(eth_num) != PASSED) {
        cterr('f', 0, "Failed to do");
        goto restore_default;
    }

    /* Restore GEWAN PHY media back to default after test */
    if (tsn_gephy_set_media_mode(eth_num, orig_media_mode) != PASSED) {
        cterr('f', 0, "Failed to set GEWAN%d media back to default ", ge_num);
        goto restore_default;
    }

    return (PASSED);

restore_default:
    sfp_encode = SFP_ENCODE_UNKNOWN;
    sfp_type = SFP_DEFAULT;
    return (FAILED);
}

/******************************************************************************
 *
 * Function   : tsn_gephy0_diag
 * Description: Entrance of TSN GE PHY0(88E1112) Diag tests.
 * Inputs     : show_menu - menu option
 * Outputs    : PASSED / FAILED
 *
 *******************************************************************************/
int tsn_gephy0_diag (int show_menu)
{
    build_primary_submenu(tsn_gephy0_diag_tbl,
                          TSN_GEPHY0_DIAG_TBL_SIZE,
                          "GE PHY 0", &tsn_gephy0_diag_menup);
    build_secondary_submenu(tsn_gephy0_diag_tbl,
                            TSN_GEPHY0_DIAG_TBL_SIZE,
                            tsn_gephy0_diag_sec_items);

    if (show_menu) {
        menu(tsn_gephy0_diag_menup, tsn_gephy0_diag_sec_items, '\0' );

    } else {
        menu_exec_doall_diags(tsn_gephy0_diag_menup);
    }
    return (PASSED);
}

/*******************************************************************************
 *
 * Function   : tsn_gephy1_diag
 * Description: Entrance of TSN GE PHY1(88E1112) Diag tests.
 * Inputs     : show_menu - menu option
 * Outputs     : PASSED / FAILED
 *
 *******************************************************************************
 */
int tsn_gephy1_diag (int show_menu)
{
    build_primary_submenu(tsn_gephy1_diag_tbl,
                          TSN_GEPHY1_DIAG_TBL_SIZE,
                          "GE PHY 1", &tsn_gephy1_diag_menup);
    build_secondary_submenu(tsn_gephy1_diag_tbl,
                            TSN_GEPHY1_DIAG_TBL_SIZE,
                            tsn_gephy1_diag_sec_items);

    if (show_menu) {
        menu(tsn_gephy1_diag_menup, tsn_gephy1_diag_sec_items, '\0' );

    } else {
        menu_exec_doall_diags(tsn_gephy1_diag_menup);
    }
    return (PASSED);
}

/*******************************************************************************
 *  
 * Function   : tsn_gephy_reg_test
 * Description: Function performs TSN GE PHY(Marvell 88E1112) register test.
 * Inputs     : ge_num - GE port number(GE0/GE1)
 * Outputs    : PASSED / FAILED
 *         
 *******************************************************************************
 */
int tsn_gephy_reg_test (int ge_num)
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
    //cterr_add_reg_dump((PFV)bcm63168_get_xdsl_profile);

    /* Segment 6: Platform Environment initialized here*/
    cterr_add_env_dump((PFV)tsn_display_temp_errormsg);

    /* Segment 7: Top 3 Debugging Steps */
    cterr_add_debug("Check the SGMII bus status to see "
                    "if it is normal among each component.",
                    "If the status is OK, contact vendor "
                    "to verify if SGMII driver is workable.");
#endif

    ushort orig_val = 0, test_pattern = (ushort)REG_PAGE(22);
    ushort read_back = 0;
    char   tname[32]; 
    int    eth_num = 0;
    int    phy_smiaddr = 0;

    memset(tname, 0, sizeof(tname));

    if (ge_num == TSN_GE0) {
        eth_num = TSN_GE0_ETHNUM;
        phy_smiaddr = (int)TSN_GE0_SMIADDR;
    } else if (ge_num == TSN_GE1) {
        eth_num = TSN_GE1_ETHNUM;
        phy_smiaddr = (int)TSN_GE1_SMIADDR;
    } else {
        cterr('f', 0, "%s(%d): TSN doesn't have GE%d.",
                      __func__, __LINE__, ge_num);
        return (FAILED);
    }

    sprintf(tname, "GE%d(eth%d) PHY register", ge_num, eth_num);
    testname(tname);
    prpass(testpass, "%s, ", tname);

    if (tsn_smi_read(phy_smiaddr, (int)REG_PAGE(22), &orig_val) != PASSED) {
        cterr('f', 0, "%s(%d): Failed to read original value of page reg.",
                      __func__, __LINE__);
        return (FAILED);
    }
    
    if (tsn_smi_write(phy_smiaddr, (int)REG_PAGE(22), test_pattern) != PASSED) {
        cterr('f', 0, "%s(%d): Failed to write test pattern to page reg.",
                      __func__, __LINE__);
        return (FAILED);
    }

    if (tsn_smi_read(phy_smiaddr, (int)REG_PAGE(22), &read_back) != PASSED) {
        cterr('f', 0, "%s(%d): Failed to read page reg.",
                      __func__, __LINE__);
        return (FAILED);
    }
    
    if (read_back != test_pattern) {
        cterr('f', 0, "Data mismatched: test_pattern %#x; read_back %#x.\n"
                      "Failed to do page reg.",
                      test_pattern, read_back);
        return (FAILED);
    }
    
    if (tsn_smi_write(phy_smiaddr, (int)REG_PAGE(22), orig_val) != PASSED) {
        cterr('f', 0, "%s(%d): Failed to restore original value of page reg.",
                      __func__, __LINE__);
        return (FAILED);
    }
    return (PASSED);
}

/*******************************************************************************
 *
 * Function    : tsn_gephy0_utils
 * Description : Entry point of TSN GE PHY0 utilities.
 * Inputs      : opt - reserve for future use
 * Outputs     : PASSED / FAILED
 *
 *******************************************************************************
 */
int tsn_gephy0_utils (int opt)
{
    build_primary_submenu(gephy0_util_items, GEPHY0_UTIL_SIZE,
                          "GE PHY 0 Utilities", &gephy0_util_menup);
    build_secondary_submenu(gephy0_util_items, GEPHY0_UTIL_SIZE,
                            gephy0_util_sec_items);

    menu(gephy0_util_menup, gephy0_util_sec_items, '\0' );

    return (PASSED);
}

/*******************************************************************************
 *
 * Function    : tsn_gephy1_utils
 * Description : Entry point of TSN GE PHY1 utilities.
 * Inputs      : opt - reserve for future use
 * Outputs     : PASSED / FAILED
 *
 *******************************************************************************
 */
int tsn_gephy1_utils (int opt)
{
    build_primary_submenu(gephy1_util_items, GEPHY1_UTIL_SIZE,
                          "GE PHY 1 Utilities", &gephy1_util_menup);
    build_secondary_submenu(gephy1_util_items, GEPHY1_UTIL_SIZE,
                            gephy1_util_sec_items);

    menu(gephy1_util_menup, gephy1_util_sec_items, '\0' );

    return (PASSED);
}

/*******************************************************************************
 *
 * Function   : ge_send_packet_util
 * Description: Utility to send and check specific TSN ethernet port.
 * Inputs     : eth_num - ethernet number
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
int ge_send_packet_util (int eth_num)
{
    int s_opt = 0, s_speed = 0;

    s_opt = getdec_answer("Enter speed(1:10MBPS, 2:100MBPS, 3:1000MBPS): ",
                          3, 1, 3);

    switch (s_opt) {
    case 1:
        s_speed = SPD_10MBPS;
        break;
    case 2:
        s_speed = SPD_100MBPS;
        break;
    case 3:
        s_speed = SPD_1000MBPS;
        break;
    default:
        s_speed = SPD_1000MBPS;
        break;
    }

    if (tsn_sgmii_lpbk_test(eth_num, s_speed) != PASSED) {
        tsn_wan_show_phy_status(eth_num);
        printf("Failed to send eth%d %dMBPS packet.\n", eth_num, s_speed);
    }
    return (PASSED);
}

/**********************************************************************
 *
 * Function   : gephy_set_testmode_util
 * Description: Utility to set TSN GE WAN PHY 1000BaseT test mode.
 * Inputs     : opt - reserved for future use
 * Outputs    : PASSED/FAILED
 *
 **********************************************************************
 */
int gephy_set_test_mode (int eth_num)
{
    int    reg_page = 0, reg_addr = 0;
    ushort reg_val = 0, test_mode = 0;
    int    ctr = 0;

    /* Got the current mode. */
    reg_page = (int)PHY_PAGE(0);
    reg_addr = (int)GEPHY_1000T_CNTL_REG;
    if (tsn_gephy_reg_rd(eth_num, reg_page, reg_addr, &reg_val) != PASSED) {
        printf("%s: Failed to get test mode(Reg. %d_%d)\n",
               __FUNCTION__, reg_page, reg_addr);
	return(FAILED);
    }
    test_mode = (ushort)((reg_val & ONEK_CNTL_TESTMODE_MASK) >>
                         ONEK_CNTL_TESTMODE_SHIFT);

    printf("\nTest modes -\n");
    printf("    0 - Normal Mode\n");
    printf("    1 - Test Mode 1 - Transmit Waveform Test\n");
    printf("    2 - Test Mode 2 - Transmit Jitter Test (Master mode)\n");
    printf("    3 - Test Mode 3 - Transmit Jitter Test (Slave mode)\n");
    printf("    4 - Test Mode 4 - Transmit Distortion Test\n");
    test_mode = (ushort)gethex_answer("Enter the test mode: ", test_mode, 0, 4);

    /* Write the new data */
    reg_val &= (ushort)(~ONEK_CNTL_TESTMODE_MASK); /* clear the test mode */
    reg_val |= (ushort)(test_mode << ONEK_CNTL_TESTMODE_SHIFT);

    if (tsn_gephy_reg_wr(eth_num, reg_page, reg_addr, reg_val) != PASSED) {
	printf("%s: Failed to set test mode(%d).\n",
	       __FUNCTION__, test_mode);
	return(FAILED);
    }

    /* Recover the page */
    reg_val = (ushort)PHY_PAGE(0);
    reg_page = (int)REG_PAGE(0);
    reg_addr = (int)MRVL88E1112_PAGE_ADDR_REG;
    if (tsn_gephy_reg_wr(eth_num, reg_page, reg_addr, reg_val) != PASSED) {
	printf("%s: Failed to recover the page(%d).\n",
	       __FUNCTION__, reg_val);
	return(FAILED);
    }

    if (test_mode == OCR_TESTMODE_NORMAL) {
        reg_val = 0;
        reg_page = (int)PHY_PAGE(0);
        reg_addr = (int)MRVL1112_COP_CTRL_REG;
        if (tsn_gephy_reg_rd(eth_num, reg_page, reg_addr, &reg_val) != PASSED) {
            printf("%s: Failed to read Reg. %d_%d\n",
                   __FUNCTION__, reg_page, reg_addr);
	    return(FAILED);
        }

        reg_val |= (ushort)COP_CTRL_RESET;
        if (tsn_gephy_reg_wr(eth_num, reg_page, reg_addr, reg_val) != PASSED) {
	    printf("%s: Failed to apply a soft reset.\n", __FUNCTION__);
	    return(FAILED);
        }

        for (ctr = 0; ctr < GEPHY_MAX_RETRY; ctr++) {
            msleep(10);

            reg_val = 0;
            if (tsn_gephy_reg_rd(eth_num, reg_page, reg_addr,
                                 &reg_val) != PASSED) {
                printf("%s:%d Failed to read Reg. %d_%d\n",
                       __FUNCTION__, __LINE__, reg_page, reg_addr);
	        return(FAILED);
            }

            if ((reg_val & (ushort)COP_CTRL_RESET) == 0) {
                break;
            }

            if (ctr == (GEPHY_MAX_RETRY - 1)) {
                printf("%s: Time out but PHY still in soft reset process.\n",
                       __FUNCTION__);
	        return(FAILED);
            }
        }
    }
    return (PASSED);
}

/*******************************************************************************
 *
 * Function   : gephy_set_txtype_util
 * Description: Utility to set TSN GE WAN PHY Transmitter Type.
 * Inputs     : eth_num - ethernet number
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
int gephy_set_txtype_util (int eth_num)
{
    int    reg_page = (int)REG_PAGE(0);
    int    reg_addr = (int)MRVL88E1112_CSC_REG2;
    ushort reg_val = 0, tx_type = 0;

    /* To get the current Transmitter Type */
    reg_page = (int)REG_PAGE(0);
    reg_addr = (int)MRVL88E1112_CSC_REG2;
    if (tsn_gephy_reg_rd(eth_num, reg_page, reg_addr, &reg_val) != PASSED) {
        printf("%s: Failed to get current Transmitter Type(Reg. %d_%d)\n",
               __FUNCTION__, reg_addr, reg_page);
    }
    tx_type = (ushort)((reg_val & (ushort)CSCR2_TXTYPE_MASK) >>
                       CSCR2_TXTYPE_OFFSET);

    /* To get user wanted Transmitter Type */
    tx_type = (ushort)gethex_answer("Enter TX Type(0: Class B; 1 - Class A): ",
                                    tx_type, 0, 1);

    if (tsn_set_gephy_txtype(eth_num, tx_type) != PASSED) {
        printf("Failed to set GE WAN PHY(eth%d) Transmitter Type.\n", eth_num);
        return (FAILED);
    }
    return (PASSED);
}

/*******************************************************************************
 *
 * Function   : tsn_set_gephy_txtype
 * Description: Function to set TSN GE WAN PHY Transmitter Type.
 * Inputs     : eth_num - ethernet number
 *              tx_type - transmitter type (0: Class B; 1: Class A)
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
int tsn_set_gephy_txtype (int eth_num, ushort tx_type)
{
    int    reg_page = (int)REG_PAGE(0);
    int    reg_addr = (int)MRVL88E1112_CSC_REG2;
    ushort reg_val = 0, set_type = 0;

    if (tx_type > GEWAN_TXTYPE_A) {
        printf("TSN GE WAN PHY(MRVL88E1112) NOT support this type(%d).\n",
               tx_type);
        return (FAILED);
    }

    set_type = (ushort)(tx_type << CSCR2_TXTYPE_OFFSET);

    /* To get the current Transmitter Type */
    reg_page = (int)REG_PAGE(0);
    reg_addr = (int)MRVL88E1112_CSC_REG2;
    if (tsn_gephy_reg_rd(eth_num, reg_page, reg_addr, &reg_val) != PASSED) {
        printf("%s: Failed to get current Transmitter Type(Reg. %d_%d)\n",
               __FUNCTION__, reg_addr, reg_page);
    }

    /* Confirm if Transmitter Type needs to be changed */
    if ((reg_val & set_type) == set_type) {
        return (PASSED);
    }

    reg_val &= (ushort)(~CSCR2_TXTYPE_MASK);
    reg_val |= set_type;

    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("%s: set_type = 0x%04X; reg_val = 0x%04X.\n",
               __FUNCTION__, set_type, reg_val);
    }

    if (tsn_gephy_reg_wr(eth_num, reg_page, reg_addr, reg_val) != PASSED) {
	printf("%s: Failed to set GE WAN PHY(eth%d) Transmitter Type.\n",
	       __FUNCTION__, eth_num);
	return(FAILED);
    }

    return (PASSED);
}

/*******************************************************************************
 *    
 * Function   : gephy_copper_mac_lpbk_test
 * Description: Function to do GE PHY MAC internal loopback test.
 * Inputs     : ge_num - GE PHY number
 * Outputs    : PASSED/FAILED
 *               
 *******************************************************************************
 */
static int gephy_copper_mac_lpbk_test (int ge_num)
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
    //cterr_add_reg_dump((PFV)bcm63168_get_xdsl_profile);

    /* Segment 6: Platform Environment initialized here*/
    cterr_add_env_dump((PFV)tsn_display_temp_errormsg);

    /* Segment 7: Top 3 Debugging Steps */
    cterr_add_debug("Observe MDIO register status to check "
                    "if PHY configuration is normal.",
                    "If step a is OK, observe registers of 1548L PHY"
                    " to check if the configuration is normal.",
                    "If step b is OK, we can assume the interface "
                    "between Host SoC and PHY has problems.");
#endif

    int      eth_num = 0;
    int      speed_ctr = 0, speed_tbl_size = 0, test_speed = SPD_1000MBPS;
    int      r_page = 0, r_addr = 0;
    uint16_t w_data = 0;
    uint     cpu_reg_addr = 0, cpu_reg_val = 0;
    uint     fpga_reg_addr = 0, fpga_reg_val = 0;
    char     test_name[32]; 
    uint orig_media_mode = (uint)MSCR1_MOD_AUTO_COP_SGMII;
    int mac_num = 0;

    memset(test_name, 0, sizeof(test_name));
    speed_tbl_size = sizeof(gephy_copper_speed_tbl) / sizeof(int);

    /* Based on TSN GE PHY mapping,
     * GE0 PHY: eth2
     * GE1 PHY: eth0
     */
    if (ge_num == TSN_GE0) {
        eth_num = TSN_GE0_ETHNUM;
        cpu_reg_addr = (uint)CPU_PORT_AN_CONF_REG(0);
        mac_num = TSN_GE0_CPUMAC_NUM;
    } else if (ge_num == TSN_GE1) {
        eth_num = TSN_GE1_ETHNUM;
        cpu_reg_addr = (uint)CPU_PORT_AN_CONF_REG(3);
        mac_num = TSN_GE1_CPUMAC_NUM;
    } else {
        cterr('f', 0, "%s(%d): TSN doesn't have GE%d.",
                      __func__, __LINE__, ge_num);
        return (FAILED);
    }

    /* Set test name */
    testname(GEPHY_MAC_LPBK_TEST);

    sprintf(test_name, "GE%d(eth%d) %s", ge_num, eth_num, GEPHY_MAC_LPBK_TEST);

    /* Read and store current GEWAN PHY media mode */
    if (tsn_gephy_get_media_mode(eth_num, &orig_media_mode) != PASSED) {
        cterr('f', 0, "Failed to get current GEWAN%d media mode ", ge_num);
        return (FAILED);
    }

    /* Set GEWAN PHY media to Copper mode ONLY for testing */
    if (tsn_gephy_set_media_mode(eth_num, (uint)MSCR1_MOD_COP_ONLY) != PASSED) {
        cterr('f', 0, "Failed to set GEWAN%d media to Copper ONLY ", ge_num);
        return (FAILED);
    }

    /* Confirm that SFP module transmitter is disabled */
    fpga_reg_addr = (uint)FPGA_SFP_AND_CTRL_REG;
    if (fpga_read_32_reg(fpga_reg_addr, &fpga_reg_val) != PASSED) {
        cterr('f', 0, "Failed to read FPGA register 0x%04X", fpga_reg_addr);
        return (FAILED);
    }

    if ((fpga_reg_val & (uint)SFP_SC_TX_DIS) == (uint)SFP_SC_TX_DIS) {
        fpga_reg_val &= (uint)(~SFP_SC_TX_DIS);

        if (fpga_write_32_reg(fpga_reg_addr, fpga_reg_val) != PASSED) {
            cterr('f', 0, "Failed to write FPGA Reg0x%04X", fpga_reg_addr);
            return (FAILED);
        }

        msleep(50);

        fpga_reg_val = 0;
        if (fpga_read_32_reg(fpga_reg_addr, &fpga_reg_val) != PASSED) {
            cterr('f', 0, "Failed to read FPGA register 0x%04X", fpga_reg_addr);
            return (FAILED);
        }

        if ((fpga_reg_val & (uint)SFP_SC_TX_DIS) == (uint)SFP_SC_TX_DIS) {
            cterr('f', 0, "Failed to disable SFP transmitter by FPGA");
            return (FAILED);
        }

        msleep(1000);
    }

    for (speed_ctr = 0; speed_ctr < speed_tbl_size; speed_ctr++) {
        test_speed = gephy_copper_speed_tbl[speed_ctr];
        prpass(testpass, "Test GE%d at %dMbps ", ge_num, test_speed);

        if (diag_kernel_ver == (uint)LINUX_KERNEL_V4_4_52) {
            /* Based on the change of Marvell driver from SDK 16.05 to 17.10,
             * need to add Force Link Good here for internal loopback test.
             * Currently only Vulcan uses Marvell SDK 17.10 Kernel(v4.4.52),
             * will update/remove this if-judgement after all TSN/Star merged
             * to same Kernel version.
             */
            if (test_speed == SPD_1000MBPS) {
                continue;
            }

            /* 1. Configure GEWAN PHY side */
            /* Force CPU GEMAC port link down for configure. */
            if (tsn_mem_write32(cpu_reg_addr,
                                (uint)PANCR_FORCE_LINK_DOWN) != PASSED) {
                printf("%s(%d): Failed to write CPU GEMAC%d reg. 0x%08X.\n",
                       __func__, __LINE__, mac_num, cpu_reg_addr);
                return (FAILED);
            }

            /* Confirm CPU GEMAC is link down */
            if (tsn_cpu_mac_check_linkstat(mac_num, CPUMAC_LINKDOWN) != PASSED) {
                printf("%s: Failed to force CPU GEMAC%d Link down.\n",
                       __func__, mac_num);
                return (FAILED);
            }

            /* 1-1. Configure GEWAN MAC speed */
            if (test_speed == SPD_10MBPS) {
                w_data = COP_SPD_10Mbps;
            } else if (test_speed == SPD_100MBPS) {
                w_data = COP_SPD_100Mbps;
            } else if (test_speed == SPD_1000MBPS) {
                w_data = COP_SPD_1000Mbps;
            } else {
                cterr('f', 0, "Unsupported Testspeed(%d) ", test_speed);
                return (FAILED);
            }
        
            if ((NVRAM)->diagflag & D_VERBOSE) {
                printf("%s: Set GE%d PHY MAC control reg.(0_2) to 0x%04X.\n",
                       __func__, ge_num, w_data);
            }

            if (tsn_gephy_set_macspeed(eth_num, w_data) != PASSED) {
                cterr('f', 0, "Failed to set GE%d PHY MAC speed ", ge_num);
                return (FAILED);
            }

            /* 1-3. Enable MAC loopback */
            if (tsn_gephy_set_macloopback(eth_num, GEPHY_COP,
                                          GEPHY_EN) != PASSED) {
                cterr('f', 0, "Failed to enable GE%d MAC loopback ", ge_num);
                return (FAILED);
            }

            /* 2. Configure CPU GEMAC side */
            /* 2-1. Configure CPU GEMAC port */
            cpu_reg_val = (uint)(PANCR_RESERVED |
                                 PANCR_SET_FULL_DUPLEX |
                                 PANCR_FORCE_LINK_UP);

            if (test_speed == SPD_100MBPS) {
                cpu_reg_val |= (uint)PANCR_SET_MII_100;
            } else if (test_speed == SPD_1000MBPS) {
                cpu_reg_val |= (uint)PANCR_SET_SGMII_1000;
            } else if (test_speed != SPD_10MBPS) {
                cterr('f', 0, "Failed at GE%d: Got unsupported Testspeed(%d) ",
                              ge_num, test_speed);
                return (FAILED);
            }
        
            if ((NVRAM)->diagflag & D_VERBOSE) {
                printf("%s: set GE%d CPU GEMAC port(0x%08X) as 0x%08X.\n",
                       __func__, ge_num, cpu_reg_addr, cpu_reg_val);
            }

            if (tsn_cpu_mac_config(mac_num, cpu_reg_val) != PASSED) {
                cterr('f', 0, "Failed to config GE%d(CPU GEMAC%d) reg. 0x%08X",
                              ge_num, mac_num, cpu_reg_addr);
                return (FAILED);
            }

            /* 3. Confirm testing port link state is up */
            /* 3-1 Confirm CPU GEMAC is link up */
            if (tsn_cpu_mac_check_linkstat(mac_num, CPUMAC_LINKUP) != PASSED) {
                cterr('f', 0, "GE%d failed to link up with CPU MAC.\n", ge_num);
                return (FAILED);
            }

			} else {
        /* 1. Config. GE PHY MAC */
        r_page = (int)REG_PAGE(2);
        r_addr = (int)REG_ADDR(0);

        if (test_speed == SPD_10MBPS) {
            w_data = COP_SPD_10Mbps;
        } else if (test_speed == SPD_100MBPS) {
            w_data = COP_SPD_100Mbps;
        } else if (test_speed == SPD_1000MBPS) {
            w_data = COP_SPD_1000Mbps;
        } else {
            cterr('f', 0, "Failed at GE%d: Got unsupported Testspeed(%d) ",
                          ge_num, test_speed);
            return (FAILED);
        }
        
        if ((NVRAM)->diagflag & D_VERBOSE) {
            printf("%s: Write-in data to GE%d PHY reg. %d_%d: 0x%04X.\n",
                   __FUNCTION__, ge_num, r_addr, r_page, w_data);
        }

        if (tsn_gephy_config_w_rst(eth_num, r_page, r_addr, w_data) != PASSED) {
            cterr('f', 0, "Failed to config GE%d PHY MAC speed to %dMbps ",
                          ge_num, test_speed);
            return (FAILED);
        }

        /* 2. Config. CPU GE MAC */
        cpu_reg_val = (uint)PANCR_FORCE_LINK_DOWN; 
        if (tsn_mem_write32(cpu_reg_addr, cpu_reg_val) != PASSED) {
            cterr('f', 0, "Failed to config CPU GE%d reg. 0x%08X",
                          ge_num, cpu_reg_addr);
            return (FAILED);
        }

        cpu_reg_val = (uint)(PANCR_RESERVED |
                             PANCR_SET_FULL_DUPLEX |
                             PANCR_AN_FC_EN |
                             PANCR_INBAND_BYPASS_EN |
                             PANCR_INBAND_AN_EN |
                             PANCR_FORCE_LINK_UP);

        if (test_speed == SPD_100MBPS) {
            cpu_reg_val |= (uint)PANCR_SET_MII_100;
        } else if (test_speed == SPD_1000MBPS) {
            cpu_reg_val |= (uint)PANCR_SET_SGMII_1000;
        } else if (test_speed != SPD_10MBPS) {
            cterr('f', 0, "Failed at GE%d: Got unsupported Testspeed(%d) ",
                          ge_num, test_speed);
            return (FAILED);
        }
        
        if ((NVRAM)->diagflag & D_VERBOSE) {
            printf("%s: Write-in data to CPU GE%d reg. 0x%08X: 0x%08X.\n",
                   __FUNCTION__, ge_num, cpu_reg_addr, cpu_reg_val);
        }

        if (tsn_mem_write32(cpu_reg_addr, cpu_reg_val) != PASSED) {
            cterr('f', 0, "Failed to config CPU GE%d reg. 0x%08X",
                          ge_num, cpu_reg_addr);
            return (FAILED);
        }

        /* 3. Config. Enable GE PHY MAC Loopback */
        r_page = (int)REG_PAGE(0);
        r_addr = (int)REG_ADDR(0);
        w_data = 0;

        if (tsn_gephy_reg_rd(eth_num, r_page, r_addr, &w_data) != PASSED) {
            cterr('f', 0, "Failed to read GE%d PHY reg. %d_%d ",
                          ge_num, r_addr, r_page);
            return (FAILED);
        }

        w_data |= (uint16_t)COP_CTRL_LPBK;

        if (tsn_gephy_reg_wr(eth_num, r_page, r_addr, w_data) != PASSED) {
            cterr('f', 0, "Failed to enable GE%d PHY MAC Loopback ", ge_num);
            return (FAILED);
        }
        }

        /* 4. Run SGMII loopback test */
        if (tsn_sgmii_lpbk_test(eth_num, test_speed) != PASSED) {
            tsn_wan_show_phy_status(eth_num);
            cterr('f', 0, "Failed at GE%d ", ge_num);
            return (FAILED);
        }
    }

    /* Restore GEWAN PHY media back to default after test */
    if (tsn_gephy_set_media_mode(eth_num, orig_media_mode) != PASSED) {
        cterr('f', 0, "Failed to set GEWAN%d media back to default ", ge_num);
        return (FAILED);
    }

    if (gephy_set_default(ge_num) != PASSED) {
        cterr('f', 0, "Failed to set GE%d back to default ", ge_num);
        return (FAILED);
    }

    return (PASSED);
}

/*******************************************************************************
 *    
 * Function   : gephy_set_1000basex_mode
 * Description: Function to set GE PHY to 1000Base-X mode.
 * Inputs     : ge_num - GE PHY number
 * Outputs    : PASSED/FAILED
 *               
 *******************************************************************************
 */
int gephy_set_1000basex_mode (int ge_num)
{
    int      eth_num = 0;
    uint     cpu_reg_addr = 0, cpu_reg_val = 0;

    if (ge_num == TSN_GE0) {
        eth_num = TSN_GE0_ETHNUM;
        cpu_reg_addr = (uint)CPU_PORT_MAC_CTRL_REG(0);
    } else if (ge_num == TSN_GE1) {
        eth_num = TSN_GE1_ETHNUM;
     cpu_reg_addr = (uint)CPU_PORT_MAC_CTRL_REG(3);
    } else {
        printf("%s: Unknown GE number(%d)\n", __FUNCTION__, ge_num);
        return (FAILED);
    }

    /* Set CPU MAC 1000BASE-X Mode */
    if (tsn_mem_read32(cpu_reg_addr, &cpu_reg_val) != PASSED) {
        cterr('f', 0, "%s: Failed to read CPU register 0x%08X.\n",
         __FUNCTION__, cpu_reg_addr);
        return (FAILED);
    }
    cpu_reg_val |= CPU_MAC_1000BASEX_MODE_REG; 
    if (tsn_mem_write32(cpu_reg_addr, cpu_reg_val) != PASSED) {
        cterr('f', 0, "Failed to config CPU GE%d reg. 0x%08X",
                          ge_num, cpu_reg_addr);
        return (FAILED);
    }

    return (PASSED);

}

/*******************************************************************************
 *    
 * Function   : gephy_set_sgmii_mode
 * Description: Function to set GE PHY SGMII mode.
 * Inputs     : ge_num - GE PHY number
 * Outputs    : PASSED/FAILED
 *               
 *******************************************************************************
 */
int gephy_set_sgmii_mode (int ge_num)
{
    int      eth_num = 0;
    uint     cpu_reg_addr = 0, cpu_reg_val = 0;

    if (ge_num == TSN_GE0) {
        eth_num = TSN_GE0_ETHNUM;
        cpu_reg_addr = (uint)CPU_PORT_MAC_CTRL_REG(0);
    } else if (ge_num == TSN_GE1) {
        eth_num = TSN_GE1_ETHNUM;
     cpu_reg_addr = (uint)CPU_PORT_MAC_CTRL_REG(3);
    } else {
        printf("%s: Unknown GE number(%d)\n", 
            __FUNCTION__, ge_num);
        return (FAILED);
    }

    /* Set CPU MAC SGMII Mode */
    if (tsn_mem_read32(cpu_reg_addr, &cpu_reg_val) != PASSED) {
      cterr('f', 0, "%s: Failed to read CPU register 0x%08X.\n",
         __FUNCTION__, cpu_reg_addr);
        return (FAILED);
    }
    cpu_reg_val &=~CPU_MAC_1000BASEX_MODE_REG; 
    if (tsn_mem_write32(cpu_reg_addr, cpu_reg_val) != PASSED) {
         cterr('f', 0, "Failed to config CPU GE%d reg. 0x%08X",
                          ge_num, cpu_reg_addr);
         return (FAILED);
    }

    return (PASSED);

}

/*******************************************************************************
 *    
 * Function   : gephy_set_loopback_mode
 * Description: Function to set loopback mode 
 * Inputs     : ge_num - GE PHY number
 * Outputs    : PASSED/FAILED
 *               
 *******************************************************************************
 */
int gephy_set_loopback_mode (int ge_num, int enable)
{
    uint cpu_phy0_test_control_reg_addr = 0;
    uint cpu_digital_loopback_enable_reg_addr = 0, cpu_digital_loopback_enable_reg_val = 0;

    if (ge_num == TSN_GE1) {
        cpu_phy0_test_control_reg_addr = (uint)CPU_PHY_0_CONTORL_REG(1);
        cpu_digital_loopback_enable_reg_addr = (uint)CPU_DIGITAL_LOOPBACK_ENABLE_REG(1);
    } else {
        printf("%s: Unknown GE number(%d) for loopback mode\n", 
            __FUNCTION__, ge_num);
        return (FAILED);
    }

    /* Set loopback mode */
    if (enable) {
        if (tsn_mem_write32(cpu_phy0_test_control_reg_addr, CPU_PHY_0_CONTORL_TEST_ENABLE_1) != PASSED) {
            cterr('f', 0, "Failed to config CPU GE%d reg. 0x%08X", ge_num, cpu_phy0_test_control_reg_addr);
            return (FAILED);
        }
		
		if (tsn_mem_read32(cpu_digital_loopback_enable_reg_addr, &cpu_digital_loopback_enable_reg_val) != PASSED) {
            cterr('f', 0, "%s: Failed to read CPU register 0x%08X.\n", __FUNCTION__, cpu_digital_loopback_enable_reg_addr);
            return (FAILED);
        }
		
		cpu_digital_loopback_enable_reg_val |= CPU_LOCAL_DIG_RX2TX_LPBK_EN;
		
		if (tsn_mem_write32(cpu_digital_loopback_enable_reg_addr, cpu_digital_loopback_enable_reg_val) != PASSED) {
            cterr('f', 0, "Failed to config CPU GE%d reg. 0x%08X", ge_num, cpu_digital_loopback_enable_reg_addr);
            return (FAILED);
        }
		
		if (tsn_mem_write32(cpu_phy0_test_control_reg_addr, CPU_PHY_0_CONTORL_TEST_ENABLE_2) != PASSED) {
            cterr('f', 0, "Failed to config CPU GE%d reg. 0x%08X", ge_num, cpu_phy0_test_control_reg_addr);
            return (FAILED);
        }
    } else {
		if (tsn_mem_write32(cpu_phy0_test_control_reg_addr, CPU_PHY_0_CONTORL_TEST_ENABLE_0) != PASSED) {
            cterr('f', 0, "Failed to config CPU GE%d reg. 0x%08X", ge_num, cpu_phy0_test_control_reg_addr);
            return (FAILED);
        }
		
		if (tsn_mem_read32(cpu_digital_loopback_enable_reg_addr, &cpu_digital_loopback_enable_reg_val) != PASSED) {
            cterr('f', 0, "%s: Failed to read CPU register 0x%08X.\n", __FUNCTION__, cpu_digital_loopback_enable_reg_addr);
            return (FAILED);
        }
		
		cpu_digital_loopback_enable_reg_val &= ~CPU_LOCAL_DIG_RX2TX_LPBK_EN;
		
		if (tsn_mem_write32(cpu_digital_loopback_enable_reg_addr, cpu_digital_loopback_enable_reg_val) != PASSED) {
            cterr('f', 0, "Failed to config CPU GE%d reg. 0x%08X", ge_num, cpu_digital_loopback_enable_reg_addr);
            return (FAILED);
        }
    }

    return (PASSED);

}

/*******************************************************************************
 *    
 * Function   : gephy_get_loopback_mode
 * Description: Function to get loopback mode 
 * Inputs     : ge_num - GE PHY number
 * Outputs    : 0 - disabled, 1 - enabled
 *               
 *******************************************************************************
 */
int gephy_get_loopback_mode (int ge_num)
{
    uint cpu_digital_loopback_enable_reg_addr = 0, cpu_digital_loopback_enable_reg_val = 0;

    if (ge_num == TSN_GE1) {
        cpu_digital_loopback_enable_reg_addr = (uint)CPU_DIGITAL_LOOPBACK_ENABLE_REG(1);
    } else {
        printf("%s: Unknown GE number(%d) for loopback mode\n", 
            __FUNCTION__, ge_num);
        return (0);
    }

    /* Get loopback mode */
    if (tsn_mem_read32(cpu_digital_loopback_enable_reg_addr, &cpu_digital_loopback_enable_reg_val) != PASSED) {
      cterr('f', 0, "%s: Failed to read CPU register 0x%08X.\n",
         __FUNCTION__, cpu_digital_loopback_enable_reg_addr);
        return (0);
    }
    
    if(cpu_digital_loopback_enable_reg_val & CPU_LOCAL_DIG_RX2TX_LPBK_EN)
        return 1;
    
    return (PASSED);
}

/*******************************************************************************
 *    
 * Function   : gephy_set_default
 * Description: Function to set GE PHY back to default.
 * Inputs     : ge_num - GE PHY number
 * Outputs    : PASSED/FAILED
 *               
 *******************************************************************************
 */
int gephy_set_default (int ge_num)
{
    int      r_page = 0, r_addr = 0;
    uint16_t w_data = 0;
    uint     cpu_reg_addr = 0, cpu_reg_val = 0;
    uint     mac_num = 0;
    int      eth_num = 0;

    if (ge_num == TSN_GE0) {
        eth_num = (int)TSN_GE0_ETHNUM;
        cpu_reg_addr = (uint)CPU_PORT_AN_CONF_REG(0);
        mac_num = TSN_GE0_CPUMAC_NUM;
    } else if (ge_num == TSN_GE1) {
        eth_num = (int)TSN_GE1_ETHNUM;
        cpu_reg_addr = (uint)CPU_PORT_AN_CONF_REG(3);
        mac_num = TSN_GE1_CPUMAC_NUM;
    } else {
        cterr('f', 0, "%s(%d): TSN doesn't have GE%d.",
                      __func__, __LINE__, ge_num);
        return (FAILED);
    }

    if (diag_kernel_ver == (uint)LINUX_KERNEL_V4_4_52) {
        /* Based on the change of Marvell driver from SDK 16.05 to 17.10,
         * need to add Force Link Good here for internal loopback test.
         * Currently only Vulcan uses Marvell SDK 17.10 Kernel(v4.4.52),
         * will update/remove this if-judgement after all TSN/Star merged
         * to same Kernel version.
         */

        /* Force CPU GEMAC port link down for configure. */
        if (tsn_mem_write32(cpu_reg_addr,
                            (uint)PANCR_FORCE_LINK_DOWN) != PASSED) {
            printf("%s(%d): Failed to write CPU GEMAC%d reg. 0x%08X.\n",
                   __func__, __LINE__, mac_num, cpu_reg_addr);
            return (FAILED);
        }

        /* 1-1. Disable Force Copper link good */
        if (tsn_gephy_force_linkgood(eth_num, GEPHY_COP,
                                     GEPHY_LINKDOWN) != PASSED) {
            cterr('f', 0, "Failed to disable GE%d Copper force Link Good ",
                          ge_num);
            return (FAILED);
        }

        /* 1-2. Configure PHY MAC back to default */
        w_data = (MCR_MAC_AN_EN | MCR_SPD_1000Mbps);
        if (tsn_gephy_set_macspeed(eth_num, w_data) != PASSED) {
            cterr('f', 0, "Failed to set GE%d PHY MAC speed ", ge_num);
            return (FAILED);
        }

        /* 1-3. Config PHY Copper as default */
        w_data = (uint16_t)(COP_SPD_1000Mbps | COP_CTRL_AUTONEG);

        if (tsn_gephy_config_media(eth_num, GEPHY_COP,
                                   w_data, GEPHY_EN) != PASSED) {
            cterr('f', 0, "Failed to configure GE%d PHY Copper ", ge_num);
            return (FAILED);
        }

        /* 2-1. Configure CPU GEMAC port */
        cpu_reg_val = (uint)(PANCR_RESERVED |
                             PANCR_AN_DUPLEX_EN |
                             PANCR_AN_FC_EN |
                             PANCR_AN_SPEED_EN |
                             PANCR_INBAND_BYPASS_EN |
                             PANCR_INBAND_AN_EN);

        if (tsn_cpu_mac_config(mac_num, cpu_reg_val) != PASSED) {
            cterr('f', 0, "Failed to config GE%d(CPU GEMAC%d) reg. 0x%08X",
                          ge_num, mac_num, cpu_reg_addr);
            return (FAILED);
        }
	} else {
    /* 1. Disable GE PHY MAC loopback */
    r_page = (int)REG_PAGE(0);
    r_addr = (int)REG_ADDR(0);

    w_data = (uint16_t)(COP_CTRL_AUTONEG |
                        COP_CTRL_DUPLEX_FULL |
                        COP_SPD_1000Mbps);
    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("%s: Write-in data to GE%d PHY reg. %d_%d: 0x%04X.\n",
               __FUNCTION__, ge_num, r_addr, r_page, w_data);
    }

    if (tsn_gephy_config_w_rst(eth_num, r_page, r_addr, w_data) != PASSED) {
        printf("%s: Failed to set GE%d PHY MAC back to default.\n",
               __FUNCTION__, ge_num);
        return (FAILED);
    }

    /* 2. Set GE PHY MAC back to default */
    r_page = (int)REG_PAGE(2);
    r_addr = (int)REG_ADDR(0);

    w_data = (uint16_t)(MCR_MAC_AN_EN | MCR_SPD_1000Mbps);
    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("%s: Write-in data to GE%d PHY reg. %d_%d: 0x%04X.\n",
               __FUNCTION__, ge_num, r_addr, r_page, w_data);
    }

    if (tsn_gephy_config_w_rst(eth_num, r_page, r_addr, w_data) != PASSED) {
        printf("%s: Failed to set GE%d PHY MAC back to default.\n",
               __FUNCTION__, ge_num);
        return (FAILED);
    }

    /* 3. Set CPU GE MAC back to default */
    cpu_reg_val = (uint)PANCR_FORCE_LINK_DOWN; 
    if (tsn_mem_write32(cpu_reg_addr, cpu_reg_val) != PASSED) {
        printf("%s: Failed to config CPU GE%d reg. 0x%08X",
               __FUNCTION__, ge_num, cpu_reg_addr);
        return (FAILED);
    }

    cpu_reg_val = (uint)(PANCR_RESERVED |
                         PANCR_AN_DUPLEX_EN |
                         PANCR_AN_FC_EN |
                         PANCR_AN_SPEED_EN |
                         PANCR_INBAND_BYPASS_EN |
                         PANCR_INBAND_AN_EN);
 
    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("%s: Write-in data to CPU GE%d reg. 0x%08X: 0x%08X.\n",
               __FUNCTION__, ge_num, cpu_reg_addr, cpu_reg_val);
    }

    if (tsn_mem_write32(cpu_reg_addr, cpu_reg_val) != PASSED) {
        printf("%s: Failed to set CPU GE%d registers back to default.\n",
               __FUNCTION__, ge_num);
        return (FAILED);
    }
    }
    return (PASSED);
}

/*******************************************************************************
 *
 * Function   : tsn_wan_show_phy_status
 * Description: Function to show GE WAN PHY status per port.
 * Inputs     : eth_num - number of GE PHY
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
int tsn_wan_show_phy_status (int eth_num)
{
    ushort result = 0;
    uint   speed = 0;
    int    reg_addr = 0;
    ushort reg_val = 0, wr_data = 0;
    int    phy_smiaddr = 0;

    if (eth_num == TSN_GE0_ETHNUM) {
        phy_smiaddr = (int)TSN_GE0_SMIADDR;
    } else if (eth_num == TSN_GE1_ETHNUM) {
        phy_smiaddr = (int)TSN_GE1_SMIADDR;
    } else {
        printf("%s(%d): TSN doesn't have eth%d.", __func__, __LINE__, eth_num);
        return (FAILED);
    }

    /* Get value of Copper Auto-nego Adv. register(4_0) */
    reg_addr = (int)REG_PAGE(22);
    wr_data = (ushort)REG_PAGE(0);
    if (tsn_smi_write(phy_smiaddr, reg_addr, wr_data) != PASSED) {
        printf("%s(%d): Failed to write 0x%04X to Reg. %d.\n",
               __func__, __LINE__, wr_data, reg_addr);
        return (FAILED);
    }

    reg_addr = (int)COP_AUTONEG_ADV_REG4;
    if (tsn_smi_read(phy_smiaddr, reg_addr, &reg_val) != PASSED) {
        printf("%s(%d): Failed to read Reg. %d.\n",
               __func__, __LINE__, reg_addr);
        return (FAILED);
    }
    printf("eth%d PHY Copper Auto-Nego Adv Reg(%d_%d) = 0x%04X.\n",
           eth_num, reg_addr, wr_data, reg_val);

    /* Get value of Copper Specific Status register 1(17_0) */
    reg_addr = (int)REG_PAGE(22);
    wr_data = (ushort)REG_PAGE(0);
    if (tsn_smi_write(phy_smiaddr, reg_addr, wr_data) != PASSED) {
        printf("%s(%d): Failed to write 0x%04X to Reg. %d.\n",
               __func__, __LINE__, wr_data, reg_addr);
        return (FAILED);
    }

    reg_addr = (int)COP_STATUS_REG17;
    reg_val = 0;
    if (tsn_smi_read(phy_smiaddr, reg_addr, &reg_val) != PASSED) {
        printf("%s(%d): Failed to read Reg. %d.\n",
               __func__, __LINE__, reg_addr);
        return (FAILED);
    }

    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("eth%d PHY Copper Auto-Nego Adv Reg(%d_%d) = 0x%04X.\n",
               eth_num, reg_addr, wr_data, reg_val);
    }

    result = ((reg_val & (ushort)COP_P0R17_SPEED) >> COP_P0R17_SPEED_OFFSET);

    switch (result) {
    case COP_P0R17_SPEED_1000:
        speed = SPD_1000MBPS;
    break;
    case COP_P0R17_SPEED_100:
        speed = SPD_100MBPS;
    break;
    case COP_P0R17_SPEED_10:
        speed = SPD_10MBPS;
    break;
    default:
        printf("Unknown speed value: %d.\n", result);
    break;
    }   
    prpass(testpass, "PHY Speed is %d Mbps", speed);

    if (reg_val & (ushort)COP_P0R17_DUPLEX_FULL) {
        prpass(testpass, "PHY is Full Duplex");
    } else {
        prpass(testpass, "PHY is Half Duplex");
    }
   
    if (reg_val & (ushort)COP_P0R17_COP_LINK_UP) {
        prpass(testpass, "Copper Link Up");
    } else { 
        prpass(testpass, "Copper Link Down");
    }
   
    if (reg_val & (ushort)COP_P0R17_GLOBAL_LINK_UP) {
        prpass(testpass, "Global Link Status is Up");
    } else { 
        prpass(testpass, "Global Link Status is Down");
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
    cterr_add_component("Marvell Armada 7040", "Marvell 88E1112 GE WAN Phy", "Moka FPGA");

    /* Segment 5: register and memory dump */
    //cterr_add_reg_dump((PFV)bcm63168_get_xdsl_profile);

    /* Segment 6: Platform Environment initialized here*/
     cterr_add_env_dump((PFV)tsn_display_temp_errormsg);

    /* Segment 7: Top 3 Debugging Steps */
    cterr_add_debug("Run PHY register test for check SMI bus is ok or not.",
                    "Check whether the interface between Intel Denverton GPIO" 
                    "and PHY is damaged or the soldering issue.");
#endif
    int rc;
    /* enhance error msg: setting test name */
    char test_name[32];
    memset(test_name, 0, sizeof(test_name));
    sprintf(test_name, "GE%d %s", ge_num, "Interrupt test");
    testname(test_name);
    prpass(testpass, "%s, ", test_name);
    printf("\n");
    
    /* run test */
    rc = mrvl_88e1112_ge_intr_test(ge_num); 

    if (rc != PASSED) {
        /* enhance error msg: error */
        cterr('f', 0, "%s:%d:GE%d: Interrupt test fail", 
              __FUNCTION__, __LINE__, ge_num);
        prcomplete(testpass, errcount, (char *)0);
        return (rc);
    }

    return (PASSED);
}

/**********************************************************************
 * Function: diag_ge_smi_read
 * Description: Function to read GE Register through SMI 
 * Inputs     : ge_num          - GE PHY number 
 *              pointer of type - smi_if_t
 * Outputs    : PASSED/FAILED
 **********************************************************************
 */
static uint32_t diag_ge_smi_read (int ge_num, smi_if_t *smi_p) 
{
    if (ge_num == TSN_GE0) {
        return (tsn_smi_read(PHY_88E1112_GE0_SMIADDR,  /* GE0 PHY SMI ADDRESS */ 
                             smi_p->offset,            /* target register of 88E1112 */
                             smi_p->buf));             /* read/write buffer pointer */
    } else {
        return (tsn_smi_read(PHY_88E1112_GE1_SMIADDR,  /* GE1 PHY SMI ADDRESS */ 
                             smi_p->offset,            /* target register of 88E1112 */
                             smi_p->buf));             /* read/write buffer pointer */
    }
}

/**********************************************************************
 * Function: diag_ge_smi_write
 * Description: Function to write GE Register through SMI 
 * Inputs     : ge_num          - GE PHY number 
 *              pointer of type - smi_if_t
 * Outputs    : PASSED/FAILED
 **********************************************************************
 */
static uint32_t diag_ge_smi_write (int ge_num, smi_if_t *smi_p) 
{
    if (ge_num == TSN_GE0) {
        return (tsn_smi_write(PHY_88E1112_GE0_SMIADDR,  /* GE0 PHY SMI ADDRESS */ 
                              smi_p->offset,            /* target register of 88E1112 */
                              (ushort)*(smi_p->buf)));  /* read/write buffer pointer */
    } else {
         return (tsn_smi_write(PHY_88E1112_GE1_SMIADDR,  /* GE1 PHY SMI ADDRESS */ 
                               smi_p->offset,            /* target register of 88E1112 */
                               (ushort)*(smi_p->buf)));  /* read/write buffer pointer */
    }
}

/*******************************************************************
 * Function:    mrvl_88e1112_ge_polling_reg
 *
 * Description: Polling a specific register.
 * Input:	ge_num           - GE PHY number
 *          compare_op       - COMPARE_AND/COMPARE_EQL
 *          page             - Page number.
 *          offset           - register offset.
 *          pattern          - a test pattern for data checking
 * Returns:     PASSED/FAILED
 *******************************************************************/
static int mrvl_88e1112_ge_polling_reg (int ge_num, int compare_op, smi_t page, uchar offset, smi_t pattern)
{
    int rc = FAILED, polling_rc = FAILED, ix;
    smi_t rd_buf;
    
    for (ix = 0; ix < MAX_POLLING_ROUND; ix++) {
        rc = mrvl_88e1112_ge_rd_wr_reg(ge_num, PHY_READ, page, offset, &rd_buf);
        if (rc != PASSED) {
            printf("%s:%d: Polling: PHY PAGE:%d REG:%d  fail\n", 
                   __FUNCTION__, __LINE__, page, offset);
            return (FAILED);
        }

        /* compare data with pattern */
        if (compare_op == COMPARE_AND) {
            if ((rd_buf & pattern) == 0) {
                polling_rc = PASSED;
                break;
            }
        } else if(compare_op == COMPARE_EQL) {
            if (rd_buf == pattern) {
                polling_rc = PASSED;
                break;
            }
        } else {
            if ((rd_buf & pattern) == pattern) {
                polling_rc = PASSED;
                break;
            }
        }

        if ((NVRAM)->diagflag & D_VERBOSE) {
            printf("DBG[%s:%d] The inner PHY SMI bus is busy\n", __FUNCTION__, __LINE__);
        }

        /* put a delay for hardware preparation */
        msleep(POLLING_PERIOD);
    }
    
    /* checking polling result */ 
    if (polling_rc != PASSED) {
            printf("%s:%d: Polling: PHY PAGE:%d REG:%d  fail, pattern:0x%04x, read data:0x%04x\n", 
                   __FUNCTION__, __LINE__, page, offset, pattern, rd_buf);
            return (FAILED);
    }

    return (polling_rc);
}

/**********************************************************************
 * Function: diag_ge_chk_intr_assert
 * Description: checking the GE interrupt is asserted
 * Inputs     : rsv - reserved field
 * Outputs    : PASSED/FAILED
 **********************************************************************
 */
static int diag_ge_chk_intr_assert (int ge_num)
{
    if (ge_num == TSN_GE0) {
        return (diag_check_ge_ext_intr_pending(PENDING_BIT_GE0));
    } else {
        return (diag_check_ge_ext_intr_pending(PENDING_BIT_GE1));
    }
}

/**********************************************************************
 * Function: diag_ge_chk_intr_deassert
 * Description: checking the GE interrupt is de-asserted
 * Inputs     : ge_num - GE PHY number
 * Outputs    : PASSED/FAILED
 **********************************************************************
 */
static int diag_ge_chk_intr_deassert (int ge_num)
{
    if (ge_num == TSN_GE0){
        return (diag_check_ge_ext_intr_no_pending(PENDING_BIT_GE0));
    } else {
        return (diag_check_ge_ext_intr_no_pending(PENDING_BIT_GE1));
    }
}

/*****************************************************
 * Function:    mrvl_88e1112_ge_chk_intr_assert
 *
 * Description: it will call a callout funciton to 
 *              check interrupt is asserted
 * Input :      ge_num - GE PHY number
 * Returns:     PASSED/FAILED
 *****************************************************/
static int mrvl_88e1112_ge_chk_intr_assert (int ge_num)
{
    int rc = FAILED, ix;

    /* As Hardware Team's suggestion, while platform running in high/low temp.(EDVT), 
     * the electric reaction of Interrupt Pin might not instantaneous,
     * hence, checking the status of Interrupt Pin by polling. */
    for (ix = 0; ix < INTR_POLLING_ROUND; ix++) 
    {
        rc = diag_ge_chk_intr_assert(ge_num);
        if (rc == PASSED) {
            break;
        }
        msleep(INTR_POLLING_PERIOD);
    }

    return (rc);
}

/*****************************************************
 * Function:    mvl_ge_chk_intr_deassert
 *
 * Description: it will call a callout funciton to 
 *              check interrupt is de-asserted
 * Input :      ge_num - GE PHY number 
 * Returns:     PASSED/FAILED
 *****************************************************/
static int mvl_ge_chk_intr_deassert (int ge_num)
{
    int rc = FAILED, ix;

    /* As Hardware Team's suggestion, while platform running in high/low temp.(EDVT), 
     * the electric reaction of Interrupt Pin might not instantaneous,
     * hence, checking the status of Interrupt Pin by polling. */
    for (ix = 0; ix < INTR_POLLING_ROUND; ix++) 
    {
        rc = diag_ge_chk_intr_deassert(ge_num);
        if (rc == PASSED) {
            break;
        }
        msleep(INTR_POLLING_PERIOD);
    }

    return (rc);
}

/************************************************************
 * Function:    mrvl_88e1112_ge_rd_wr_reg
 *
 * Description: reads or writes a register of specified page.
 * Input:	ge_num - GE PHY number
 *          op     - PHY_READ or PHY_WRITE
 *          page   - Page number.
 *          offset - register offset.
 *          data   - Points to data of read/write
 * Returns:     PASSED/FAILED
*************************************************************/
static int mrvl_88e1112_ge_rd_wr_reg (int ge_num, uint op, smi_t page, uchar offset, smi_t *data)
{
    uint32_t rc;
    smi_if_t smi_if;
    smi_t reg_d;
    char err_buf[ERR_BUF_SIZE];

    reg_d = page;
    smi_if.buf = &reg_d;
    smi_if.offset = MRV88E111N_PAGE_ADDRESS_REG;
    /* 
     * Before reading or writing a specific register, there're two steps as below:
     * (1) Writing the page number into "Reg. 22, Page Address Register" by callout function first.
     * (2) Then read/write data by callout function.
     */
    rc = diag_ge_smi_write(ge_num, &smi_if);

    if (rc != PASSED) {
        sprintf(err_buf, "mvl_ge_rd_wr_reg() set page %#x failed. rc = %#x",
                page, rc);
        cterr('f', 0, "%s", err_buf);
        return (FAILED);
    }

    smi_if.buf = data;
    smi_if.offset = offset;

    if (op == PHY_READ) {
        rc = diag_ge_smi_read(ge_num, &smi_if);
    } else {
        rc = diag_ge_smi_write(ge_num, &smi_if);
    }

    if (rc != PASSED) {
        sprintf(err_buf, "mvl_ge_rd_wr_reg() op = %#x reg @ %#x failed. "
                " rc = %#x", op, offset, rc);
        cterr('f', 0, "%s", err_buf);
    }

    return (rc);
}

/********************************************************************
 * Function:    mrvl_88e1112_ge_intr_disable
 *
 * Description: disable the interrupt GE PHY
 * Returns:     PASSED/FAILED
 ********************************************************************/
static int mrvl_88e1112_ge_intr_disable (int ge_num)
{
    uint32_t rc;
    smi_t wr_buf;
    smi_t page   = MRV88E111N_REG_PAGE_3; 
    uchar offset = MRV88E111L_FUNC_CONTROL_REG; 
    
    /* write data */
    wr_buf = MRV88E111N_DISABLE_INTR; 
    rc = mrvl_88e1112_ge_rd_wr_reg(ge_num, PHY_WRITE, page, offset, &wr_buf);
    if (rc != PASSED) {
        printf("Failed\n");
        printf("%s:%d: Fail to write data:0x%04x to  Page:%d Reg:%d\n", 
                __FUNCTION__, __LINE__, wr_buf, page, offset);
        return (FAILED);
    }
    return (PASSED);
}

/********************************************************************
 * Function:    mrvl_88e1112_ge_intr_enable
 *
 * Description: disable the interrupt GE PHY
 * Returns:     PASSED/FAILED
 ********************************************************************/
static int mrvl_88e1112_ge_intr_enable (int ge_num)
{
    uint32_t rc;
    smi_t wr_buf;
    smi_t page   = MRV88E111N_REG_PAGE_3; 
    uchar offset = MRV88E111L_FUNC_CONTROL_REG; 

    /* write data */
    wr_buf = MRV88E111N_ENABLE_INTR; 
    rc = mrvl_88e1112_ge_rd_wr_reg(ge_num, PHY_WRITE, page, offset, &wr_buf);
    if (rc != PASSED) {
        printf("Failed\n");
        printf("%s:%d: Fail to write data:0x%04x to  Page:%d Reg:%d\n", 
                __FUNCTION__, __LINE__, wr_buf, page, offset);
        return (FAILED);
    }
    return (PASSED);
}

/********************************************************************
 * Function:    mrvl_88e1112_ge_intr_test
 *
 * Description: testing the interrupt function of GE PHY
 * Input:       ge_num - GE PHY number
 * Returns:     PASSED/FAILED
 ********************************************************************/
static int mrvl_88e1112_ge_intr_test (int ge_num)
{
    uint32_t rc;
    smi_t test_pattern;
    smi_t page = 0;
    uchar offset = 0;

    page   = MRV88E111N_REG_PAGE_3; 
    offset = MRV88E111L_FUNC_CONTROL_REG;  

    /*=======================================================================*/
    /*== [Config]setting GE PHY R16_P3 with data:0x021e, disable interrupt ==*/
    /*=======================================================================*/
    printf("Setting GE PHY INT Pin(Pin.61) as default ...");
    if (mrvl_88e1112_ge_intr_disable(ge_num) != PASSED) {
        printf("%s:%d: Failed to disable interrupt!!\n", __func__, __LINE__);
        return (FAILED);       
    }


    /* polling data */
    test_pattern = MRV88E111N_DISABLE_INTR;
    rc = mrvl_88e1112_ge_polling_reg(ge_num, COMPARE_EQL, page, offset, test_pattern);
    if (rc != PASSED) {
        printf("Failed\n");
        printf("%s:%d: Polling: PHY PAGE:%d REG:%d, Checking Function Control Register fail\n", 
               __FUNCTION__, __LINE__, page, offset);
        return (FAILED);
    }

    /*===================================================*/
    /*== [Config]checking the interrupt is de-asserted ==*/
    /*===================================================*/
    rc = mvl_ge_chk_intr_deassert(ge_num);
    if (rc != TRUE) {
        printf("Failed\n");
        printf("%s:%d: The interrupt is not de-asserted\n", 
               __FUNCTION__, __LINE__);
        return (FAILED);
    }
    printf("Done\n");

    /*======================================================================*/
    /*== [Config]setting GE PHY R16_P3 with data:0x0e1e, enable interrupt ==*/
    /*======================================================================*/
    printf("Catching GE PHY interrupt signal ...");
    if (mrvl_88e1112_ge_intr_enable(ge_num) != PASSED) {
        printf("%s:%d: Failed to disable interrupt!!\n", __func__, __LINE__);
        return (FAILED);       
    }
    /* polling data */
    test_pattern = MRV88E111N_ENABLE_INTR;
    rc = mrvl_88e1112_ge_polling_reg(ge_num, COMPARE_EQL, page, offset, test_pattern);
    if (rc != PASSED) {
        printf("Failed\n");
        printf("%s:%d: Polling: PHY PAGE:%d REG:%d, Checking Function Control Register fail\n", 
               __FUNCTION__, __LINE__, page, offset);
        return (FAILED);
    }

    /*================================================*/
    /*== [Config]checking the interrupt is asserted ==*/
    /*================================================*/
    rc = mrvl_88e1112_ge_chk_intr_assert(ge_num);
    if (rc != TRUE) {
        printf("Failed\n");
        printf("%s:%d: The interrupt is not asserted\n", 
               __FUNCTION__, __LINE__);
        return (FAILED);
    }
    printf("Hit\n");
    
    /*=======================================================================*/
    /*== [Config]setting GE PHY R16_P3 with data:0x021e, disable interrupt ==*/
    /*=======================================================================*/
    printf("Setting GE PHY INT Pin(Pin.61) as default ...");
    if (mrvl_88e1112_ge_intr_disable(ge_num) != PASSED) {
        printf("%s:%d: Failed to disable interrupt!!\n", __func__, __LINE__);
        return (FAILED);       
    }

    /* polling data */
    test_pattern = MRV88E111N_DISABLE_INTR;
    rc = mrvl_88e1112_ge_polling_reg(ge_num, COMPARE_EQL, page, offset, test_pattern);
    if (rc != PASSED) {
        printf("Failed\n");
        printf("%s:%d: Polling: PHY PAGE:%d REG:%d, Checking Function Control Register fail\n", 
               __FUNCTION__, __LINE__, page, offset);
        return (FAILED);
    }

    /*===================================================*/
    /*== [Config]checking the interrupt is de-asserted ==*/
    /*===================================================*/
    rc = mvl_ge_chk_intr_deassert(ge_num);
    if (rc != TRUE) {
        printf("Failed\n");
        printf("%s:%d: The interrupt is not de-asserted\n",
               __FUNCTION__, __LINE__);
        return (FAILED);
    }
    printf("Done\n");

    return (PASSED);
}

/*******************************************************************************
 * Function   : has_mrvl_88e6112
 * Description: Function to check whether this platform has mevl 88e6176 or not
 * Outputs    : TRUE / FALSE
 *
 *******************************************************************************
 */
int has_mrvl_88e1112 (void)
{
    if (this_is_star() || this_is_supernova()) {
        return (TRUE);
    }
    return (FALSE);
}
/*-------------------------------------------------
 * $Log: diag_ge_phy.c,v $
 * Revision 1.11  2019/01/24 01:07:22  letsai
 * Add Supernova GE0/ESW Interrupt Test (CSCvo04335).
 *
 * Revision 1.10  2019/01/18 05:54:46  yungchen
 * Merge Supernova branch to the main trunk (CSCvn79871)
 *
 * Revision 1.9  2018/11/23 08:49:51  hondwang
 * Re-instruct pluggable common code with CDETs CSCvn17216
 *
 * Revision 1.8.34.2  2018/11/13 07:10:29  hondwang
 * Base on PRRQ comment add gephy_set_test_mode back
 *
 * Revision 1.8.34.1  2018/10/15 06:53:07  hondwang
 * pluggable common code re-instruct modify code
 *
 * Revision 1.8  2018/05/30 10:06:57  steja
 * <CSCvj57981>Enhance SFP read Ext.ID functionality to be reuse.
 *
 * Revision 1.7  2018/05/24 09:47:10  steja
 * CSCvj57981-Enhance SFP GLC-GE-100FX Support
 *
 * Revision 1.6  2018/05/15 09:37:32  steja
 * CSCvj38863: Enhanced LED single test utility
 *
 * Revision 1.5  2018/04/15 22:03:30  palin2
 * Merged Vulcan back to maintrunk.
 *
 * Revision 1.4  2018/04/13 08:52:58  palin2
 * To fix CSCvi96469: Potential issue on GEWAN0(Copper + SFP) loopback test.
 *
 * Revision 1.3.10.1  2018/04/09 20:56:59  palin2
 * Enhanced GEWAN PHY Diag tests by config testing media accordingly.
 *
 * Revision 1.3  2018/02/09 09:56:53  hondwang
 * Merge Star branch star-branch-c9xx to main trunk
 *
 * Revision 1.2.20.1  2018/01/20 06:27:23  hondwang
 * prepare merge star-branch-c9xx to main trunk
 *
 * Revision 1.2.4.8  2017/10/17 08:57:46  lucywang
 * Corrected MAC port mapping for GE1
 *
 * Revision 1.2.4.7  2017/09/22 03:22:05  lucywang
 * Set CPU registers to enable motherboard line loopback for Pluggable Serial GE port
 *
 * Revision 1.2.4.6  2017/09/20 07:09:11  lucywang
 * set GE1 to 1000Base-X for pluggable serial module
 *
 * Revision 1.2.4.5  2017/09/15 04:30:38  lucywang
 * added utility to enable/diable motherboard line loopback for Pluggable Serial GE port, not work yet
 *
 * Revision 1.2.4.4  2017/09/14 22:25:51  hondwang
 * Fix pluggable GE phy testing fail with submenu in first time run
 *
 * Revision 1.2.4.3  2017/08/23 05:46:33  lucywang
 * enable/disable Receiver to Tansmitter in local PHY for pluggable serial module
 *
 * Revision 1.2.4.2  2017/08/22 03:29:59  lucywang
 * set 1000Base-X for pluggable serial and set sgmii for pluggable test card
 *
 * Revision 1.2.4.1  2017/08/15 14:18:38  hondwang
 * star branch c9xx initial check in
 *
 * Revision 1.2  2017/08/02 14:21:45  steja
 * Support TSN-H/M platform code
 *
 * Revision 1.1.8.3  2017/07/31 16:33:17  palin2
 * Added utiltiy to support GE WAN PHY VOD adjustments.
 *
 * Revision 1.1.8.2  2017/07/29 03:41:02  steja
 * tsn-branch5 synced with Maintrunk repositories
 *
 * Revision 1.1.6.4  2017/07/24 14:14:10  palin2
 * 1. To improve code readability.
 * 2. All changes are verified before check-in.
 *
 * Revision 1.1.6.3  2017/07/21 06:06:23  palin2
 * Code clean up.
 *
 * Revision 1.1.6.2  2017/07/20 13:38:03  steja
 * tsn-branch4 merge with maintrunk
 *
 * Revision 1.1.4.8.2.4  2017/07/17 13:54:44  palin2
 * Code cleanup.
 *
 * Revision 1.1.4.8.2.3  2017/07/12 12:23:08  palin2
 * Added Ext. Loopback Flag support.
 *
 * Revision 1.1.4.8.2.2  2017/05/17 01:17:52  palin2
 * Updated GE WAN mapping number with team's decision.
 * (GE0: GE WAN with SFP; GE1: 2nd GE WAN)
 * CV: ----------------------------------------------------------------------
 *
 * Revision 1.1.4.8.2.1.4.2  2017/07/03 13:16:39  hondwang
 * fix E2E LED, I2C and GE phy testing fail
 *
 * Revision 1.1.4.8.2.1.4.1  2017/06/17 12:13:07  hondwang
 * Add test card phy testing function
 *
 * Revision 1.1.4.8.2.1  2017/04/13 13:14:40  palin2
 * Added to confirm SFP transmitter is disabled before test GE1 Copper.
 *
 * Revision 1.1.4.8  2016/11/01 07:29:19  petteng
 * Add enhanced error message
 *
 * Revision 1.1.4.7  2016/10/04 06:39:08  petteng
 * Add enhanced error message
 *
 * Revision 1.1.4.6  2016/09/13 08:14:23  palin2
 * Added CPU to GE PHY MAC loopback test.
 *
 * Revision 1.1.4.5  2016/08/16 03:08:17  palin2
 * Unified test pass print outs.
 *
 * Revision 1.1.4.4  2016/07/17 10:52:56  palin2
 * 1. Added function and utility to set GE WAN PHY Transmitter Type.
 * 2. Clean up code.
 *
 * Revision 1.1.4.3  2016/06/30 14:06:31  steja
 * Pick up the latest from tsn-branch1
 *
 * Revision 1.1.4.2  2016/06/30 06:22:47  steja
 * tsn-branch2 sync with main trunk
 *
 * Revision 1.1.2.7  2016/06/29 12:08:37  palin2
 * Added utility to set GE WAN PHY 1000Base-T Test mode.
 *
 * Revision 1.1.2.6  2016/05/24 01:20:11  palin2
 * Updated GE Switch and PHY utilities.
 *
 * Revision 1.1.2.5  2016/05/03 16:01:58  palin2
 * Added GE PHY register test.
 *
 * Revision 1.1.2.4  2016/04/29 10:20:48  palin2
 * Added support GE PHY0 diag tests.
 *
 * Revision 1.1.2.3  2016/04/26 20:48:49  palin2
 * Updated code after bring up SFP external loopback test.
 *
 * Revision 1.1.2.2  2016/04/22 12:28:36  palin2
 * Updated code after bring up GE PHY external loopback test.
 *
 * Revision 1.1.2.1  2016/03/29 02:50:02  palin2
 * Added GE PHY Diag.
 *
 * $Endlog$
 *-------------------------------------------------
 */

