/* $Id: diag_esw_test.c,v 1.2 2019/01/10 06:36:22 wilbhuan Exp $ 
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/betelgeuse/diag_esw_test.c,v $
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
#include "diag_pkt_txrx_lib.h"
#include "diag_cpu_lib.h"
#include "diag_cpu_util.h"
#include "platform_cookie.h"
#include "diag_temp_sensor_util.h"
#include "dev_88e6176.h"
#include "dev_88e6390.h"
#include "diag_esw_lib.h"
#include "diag_esw_util.h"
#include "diag_esw_test.h"
#include "diag_moka_fpga_util.h"
#include "diag_enhance_err_msg_lib.h"

/* Function Declaration */
static int diag_esw_reg_test(void);
static int diag_esw_mac_lpbk_test(int);
static int diag_esw_ext_lpbk_test (void);
static int diag_esw_intr_test (void);
static int diag_esw_utils(int);

/* EWS global device object */
static dev_88e6390_object_t dev_88e6390_obj;
static dev_88e6176_object_t dev_88e6176_obj;

/* ESW supported speed table */
static int esw_speed_tbl[] = {SPD_10MBPS, SPD_100MBPS, SPD_1000MBPS};

/* including 88E6390 & 88E6176 menu */
static submenu_xtable_t esw_submenu_tbl[] = {
    {"ESW Utilities",
     (type_t(*)())diag_esw_utils,                                 FALSE,
     0,
     (type_t(*)())0,                                             0,
     (type_t(*)())0,                                             0},
    {"ESW Register Test",
     (type_t(*)())diag_esw_reg_test,                             0,
     (MF_CONTINUOUS | MF_DOGRP | MF_DOALL | MF_SHOW_ERRCOUNT),
     (type_t(*)())0,                                             0,
     (type_t(*)())0,                                             0},
    {"ESW MAC Loopback Test",
     (type_t(*)())diag_esw_mac_lpbk_test,                    0,
     (MF_CONTINUOUS | MF_DOGRP | MF_DOALL | MF_SHOW_ERRCOUNT),
     (type_t(*)())0,                                             0,
     (type_t(*)())0,                                             0},
    {"ESW External Loopback Test",
     (type_t(*)())diag_esw_ext_lpbk_test,                        0,
     (MF_CONTINUOUS | MF_DOGRP | MF_DOALL | MF_SHOW_ERRCOUNT),
     (type_t(*)())0,                                             0,
     (type_t(*)())0,                                             0},
    {"ESW Interrupt Test",
     (type_t(*)())diag_esw_intr_test,                        ETH1,
     (MF_CONTINUOUS | MF_DOGRP | MF_DOALL | MF_SHOW_ERRCOUNT),
     (type_t(*)())0,                                             0,
     (type_t(*)())0,                                             0},
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
/* List of ESW Utilities */
static submenu_xtable_t esw_util_items[] = {
    {"spd1000 ext. lpbk util",       
        (type_t(*)())esw_send_packet_util,              
        SPD_1000MBPS, 0, (type_t(*)())0, 0, (type_t(*)())0, 0}, 
    {"spd100 ext. lpbk util",        
        (type_t(*)())esw_send_packet_util,              
        SPD_100MBPS,  0, (type_t(*)())0, 0, (type_t(*)())0, 0},
    {"spd10 ext. lpbk util",         
        (type_t(*)())esw_send_packet_util,              
        SPD_10MBPS,   0, (type_t(*)())0, 0, (type_t(*)())0, 0},
    {"FPGA register Read",           
        (type_t(*)())fpga_reg_rd_util,
        0,            0, (type_t(*)())0, 0, (type_t(*)())0, 0},
    {"FPGA register Write",          
        (type_t(*)())fpga_reg_wr_util,
        0,            0, (type_t(*)())0, 0, (type_t(*)())0, 0},
    {"CPU register Read",            
        (type_t(*)())diag_cpu_reg_rd_util,
        0,            0, (type_t(*)())0, 0, (type_t(*)())0, 0},
    {"CPU register Write",           
        (type_t(*)())diag_cpu_reg_wr_util,
        0,            0, (type_t(*)())0, 0, (type_t(*)())0, 0},
    {"ESW set all ports forwarding", 
        (type_t(*)())diag_esw_set_allports_forward_util,
        0,            0, (type_t(*)())0, 0, (type_t(*)())0, 0},
    {"ESW Register Read",            
        (type_t(*)())diag_esw_reg_rd_util,
        0,            0, (type_t(*)())0, 0, (type_t(*)())0, 0},
    {"ESW Register Write",           
        (type_t(*)())diag_esw_reg_wr_util,
        0,            0, (type_t(*)())0, 0, (type_t(*)())0, 0},
    {"ESW PHY Register Read",        
        (type_t(*)())diag_esw_phy_reg_rd_util,
        0,            0, (type_t(*)())0, 0, (type_t(*)())0, 0},
    {"ESW PHY Register Write",       
        (type_t(*)())diag_esw_phy_reg_wr_util,
        0,            0, (type_t(*)())0, 0, (type_t(*)())0, 0},
    {"SMI C45 read",                 
        (type_t(*)())diag_esw_smi_c45_rd_util,
        0,            0, (type_t(*)())0, 0, (type_t(*)())0, 0},
    {"SMI C45 write",                
        (type_t(*)())diag_esw_smi_c45_wr_util,
        0,            0, (type_t(*)())0, 0, (type_t(*)())0, 0},
    {"Set PHY 1000Base-T Test mode", 
        (type_t(*)())diag_esw_set_1k_testmode_util,
        0,            0, (type_t(*)())0, 0, (type_t(*)())0, 0},
    {"Reset and re-init ESW",        
        (type_t(*)())diag_reset_esw_to_default,
        FALSE,        0, (type_t(*)())0, 0, (type_t(*)())0, 0},
    {"Adjust port VOD",              
        (type_t(*)())diag_esw_port_vod_adjust_util,
        0,            0, (type_t(*)())0, 0, (type_t(*)())0, 0},
    {"Turn port LED ON/OFF",         
        (type_t(*)())diag_esw_force_led_onoff_util,
        0,            0, (type_t(*)())0, 0, (type_t(*)())0, 0},
};

#define ESW_UTIL_SIZE (sizeof(esw_util_items) / sizeof(submenu_xtable_t))

/*
 * ESW utility items (filled in from xtable)
 */
static mitem_t esw_util_pri_items[ESW_UTIL_SIZE + MAX_BASE_ITEMS];
static mitem_t esw_util_sec_items[ESW_UTIL_SIZE + MAX_BASE_ITEMS];

/*
 * GE PHY Utility Submenu
 */
menuinfo_t esw_util_menu = {
    "%s Menu",
    0,
    (PFT)show_endnote,
    0,
    0,
    esw_util_pri_items,
};

menuinfo_t *esw_util_menup = &esw_util_menu;

/******************************************************************************
 * Function: diag_esw_reg_test
 *
 * Description: Function to access Ethernet Switch register  
 * Inputs      : None
 * Outputs     : PASSED / FAILED
 *****************************************************************************/
int diag_esw_reg_test (void)
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
    cterr_add_env_dump((PFV)show_plat_curr_temps);

    /* Segment 7: Top 3 Debugging Steps */
    cterr_add_debug("Check the SGMII bus status to see "
                    "if it is normal among each component.",
                    "If the status is OK, contact vendor "
                    "to verify if SGMII driver is workable.");
#endif

    int rc = FAILED;
    void *esw_obj_p = NULL;

    /* enhance error msg: setting test name */
    char test_name[32] = "ESW Register test";
    testname(test_name);
    prpass(testpass, "%s, ", test_name);
    printf("\n");

    if (platform_esw_type() == ESW_MRVL88E6390) {
        esw_obj_p = &dev_88e6390_obj;
    } else {
        esw_obj_p = &dev_88e6176_obj;
    }

    /* create ESW device object */
    rc = diag_esw_dev_create(esw_obj_p);
    if (rc == FAILED) {
        /* enhance error msg: error */
        cterr('f', 0, "%s:%d: Fail to create Marvell ESW Object", 
              __FUNCTION__, __LINE__);
        prcomplete(testpass, errcount, (char *)0);
        return (FAILED);
    }

    /* 88E6390 or 88E6176 Register test*/
    if (platform_esw_type() == ESW_MRVL88E6390) {
        rc =((dev_88e6390_object_t *)esw_obj_p)->callin_fvt->
            register_test((dev_object_t *)esw_obj_p);

        if (rc != PASSED) {
            /* enhance error msg: error */
            cterr('f', 0, "%s:%d: Register test failed on 88E6390\n", 
                  __FUNCTION__, __LINE__);
            prcomplete(testpass, errcount, (char *)0);
        }

        ((dev_88e6390_object_t *)esw_obj_p)->base.dev_object_fvt->
        dev_destroy((dev_object_t **)&esw_obj_p);

    } else {
        rc =((dev_88e6176_object_t *)esw_obj_p)->callin_fvt->
        register_test((dev_object_t *)esw_obj_p);

        if (rc != PASSED) {
            /* enhance error msg: error */
            cterr('f', 0, "%s:%d: Register test failed on 88E6176\n", 
                  __FUNCTION__, __LINE__);
            prcomplete(testpass, errcount, (char *)0);
        }

        ((dev_88e6176_object_t *)esw_obj_p)->base.dev_object_fvt->
        dev_destroy((dev_object_t **)&esw_obj_p);

    }
    
    return (rc);
}

/*******************************************************************************
 * Function   : diag_esw_mac_lpbk_test
 *
 * Description: Function to do CPU to switch PHY MAC loopback test.
 * Inputs     : opt - reserve for future use
 * Outputs    : PASSED/FAILED
 *******************************************************************************/
int diag_esw_mac_lpbk_test (int opt) 
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
    cterr_add_env_dump((PFV)show_plat_curr_temps);

    /* Segment 7: Top 3 Debugging Steps */
    cterr_add_debug("Check the SGMII bus status to see "
                    "if it is normal among each component.",
                    "If the status is OK, contact vendor "
                    "to verify if SGMII driver is workable.");
#endif

    int p_ctr = 0, spd_ctr = 0, test_spd = 0;
    int start_port = 0, end_port = 0;
    int total_spd = 0;
    int result = FAILED;

    /* enhance error msg: setting test name */
    char test_name[32] = "ESW MAC loopback test";
    testname(test_name);
    prpass(testpass, "%s, ", test_name);
    printf("\n");

    void *esw_obj_p = NULL;
    if (platform_esw_type() == ESW_MRVL88E6390) {
        esw_obj_p = &dev_88e6390_obj;
        start_port = (int)ESW_PORT1;
        end_port = (int)ESW_PORT8;
    } else {
        esw_obj_p = &dev_88e6176_obj;
        start_port = (int)ESW_PORT0;
        end_port = (int)ESW_PORT3;
        total_spd = sizeof(esw_speed_tbl) / sizeof(int);
    }

    /* create ESW device object */
    if (diag_esw_dev_create(esw_obj_p) !=PASSED) {
        /* enhance error msg: error */
        cterr('f', 0, "%s:%d: Fail to create Marvell ESW Object", 
              __FUNCTION__, __LINE__);
        prcomplete(testpass, errcount, (char *)0);
        return (FAILED);
    }

    /* 88E6390 or 88E6176 MAC loopback test */
    if (platform_esw_type() == ESW_MRVL88E6390) {
        if (diag_esw_force_cpu_linkup() != PASSED) {
            /* enhance error msg: error */
            cterr('f', 0, "%s:%d: Fail to force link up CPU side MAC", 
                  __FUNCTION__, __LINE__);
            prcomplete(testpass, errcount, (char *)0);
            goto _exit;
        }
        if (((dev_88e6390_object_t *)esw_obj_p)->callin_fvt->
            esw_phy_mac_lpbk_test((dev_object_t *)esw_obj_p, 
            start_port, end_port) != PASSED) {
            /* enhance error msg: error */
            cterr('f', 0, "%s:%d: Fail to run MAC loopback test", 
                  __FUNCTION__, __LINE__);
            prcomplete(testpass, errcount, (char *)0);
            goto _exit;
        }
        result = PASSED;;
    } else {
        for (p_ctr = start_port; p_ctr <= end_port; p_ctr++) {
            if (diag_esw_force_cpu_linkup() != PASSED) {
                /* enhance error msg: error */
                cterr('f', 0, "%s:%d: Fail to force link up CPU side MAC", 
                      __FUNCTION__, __LINE__);
                prcomplete(testpass, errcount, (char *)0);
                goto _exit;
            }

            for (spd_ctr = 0; spd_ctr < total_spd; spd_ctr++) {
                test_spd = esw_speed_tbl[spd_ctr];
     
                if (((dev_88e6176_object_t *)esw_obj_p)->callin_fvt->
                    esw_phy_mac_lpbk_test((dev_object_t *)esw_obj_p,
                    p_ctr, test_spd) != PASSED) {
                    /* enhance error msg: error */
                    cterr('f', 0, "%s:%d: Fail to run MAC loopback test at Port:%d in Speed:%dMbpc", 
                          __FUNCTION__, __LINE__, p_ctr, test_spd);
                    prcomplete(testpass, errcount, (char *)0);
                    goto _exit;
                }
            }
            if (diag_reset_esw_to_default(TRUE) != PASSED) {
                /* enhance error msg: error */
                cterr('f', 0, "%s:%d: Fail to reset ESW", 
                      __FUNCTION__, __LINE__);
                prcomplete(testpass, errcount, (char *)0);
                goto _exit;
            }
        }
        result = PASSED;
    }

    /* use to destroy ESW device object */
_exit:
    if (platform_esw_type() == ESW_MRVL88E6390) {
        ((dev_88e6390_object_t *)esw_obj_p)->base.dev_object_fvt->
         dev_destroy((dev_object_t **)&esw_obj_p);
    } else {
        ((dev_88e6176_object_t *)esw_obj_p)->base.dev_object_fvt->
         dev_destroy((dev_object_t **)&esw_obj_p);
    }

    return (result);
}

/******************************************************************************
 * Function: diag_esw_ext_lpbk_test
 *
 * Description: Function to do Ethernet Switch ports external loopback test.
 * Inputs      : None
 * Outputs     : PASSED / FAILED
 *****************************************************************************/
int diag_esw_ext_lpbk_test (void) 
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
    cterr_add_env_dump((PFV)show_plat_curr_temps);

    /* Segment 7: Top 3 Debugging Steps */
    cterr_add_debug("Check the SGMII bus status to see "
                    "if it is normal among each component.",
                    "If the status is OK, contact vendor "
                    "to verify if SGMII driver is workable.");
#endif

    int  start_port = 0, end_port = 0;
    int result = FAILED;
    void *esw_obj_p = NULL;
    /* enhance error msg: setting test name */
    char test_name[32] = "ESW External loopback test";
    testname(test_name);
    prpass(testpass, "%s, ", test_name);
    printf("\n");

    if (platform_esw_type() == ESW_MRVL88E6390) {
        esw_obj_p = &dev_88e6390_obj;
        start_port = (int)ESW_PORT1;
        end_port = (int)ESW_PORT8;
    } else {
        esw_obj_p = &dev_88e6176_obj;
        start_port = (int)ESW_PORT0;
        end_port = (int)ESW_PORT3;
    }

    /* create ESW device object */
    if (diag_esw_dev_create(esw_obj_p) !=PASSED) {
        /* enhance error msg: error */
        cterr('f', 0, "%s:%d: Fail to create Marvell ESW Object", 
              __FUNCTION__, __LINE__);
        prcomplete(testpass, errcount, (char *)0);
        return (FAILED);
    }

    /* 88E6390 or 88E6176 External loopback test */
    if (platform_esw_type() == ESW_MRVL88E6390) {
        if (((dev_88e6390_object_t *)esw_obj_p)->callin_fvt->
            ext_lpbk_test((dev_object_t *)esw_obj_p, 
            start_port, end_port) != PASSED) {
            /* enhance error msg: error */
            cterr('f', 0, "%s:%d: Fail to run External loopback test", 
                  __FUNCTION__, __LINE__);
            prcomplete(testpass, errcount, (char *)0);
            goto _exit;
        }
        result = PASSED;
    } else {
        if (((dev_88e6176_object_t *)esw_obj_p)->callin_fvt->
            ext_lpbk_test((dev_object_t *)esw_obj_p, 
            start_port, end_port) != PASSED) {
            /* enhance error msg: error */
            cterr('f', 0, "%s:%d: Fail to run External loopback test", 
                  __FUNCTION__, __LINE__);
            prcomplete(testpass, errcount, (char *)0);
            goto _exit;
        }
        result = PASSED;
    }

    /* use to destroy ESW device object */
_exit:
    if (platform_esw_type() == ESW_MRVL88E6390) {
        ((dev_88e6390_object_t *)esw_obj_p)->base.dev_object_fvt->
         dev_destroy((dev_object_t **)&esw_obj_p);
    } else {
        ((dev_88e6176_object_t *)esw_obj_p)->base.dev_object_fvt->
         dev_destroy((dev_object_t **)&esw_obj_p);
    }

    return (result);
}

/******************************************************************************
 * Function: diag_esw_test
 *
 * Description: Entrance of Ethernet Switch Diag menu.
 *
 * Inputs      : GE0/GE1
 * Outputs     : PASSED / FAILED
 ******************************************************************************/
int diag_esw_test (int show_menu)
{
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
 * Function    : diag_esw_utils
 *
 * Description :
 * Inputs      : opt - reserve for future use
 * Outputs     : PASSED / FAILED
 *******************************************************************************/
static int diag_esw_utils (int opt)
{
    build_primary_submenu(esw_util_items, ESW_UTIL_SIZE,
                          "Ethernet Switch Utilities", &esw_util_menup);
    build_secondary_submenu(esw_util_items, ESW_UTIL_SIZE,
                            esw_util_sec_items);

    menu(esw_util_menup, esw_util_sec_items, '\0' );
    return (PASSED);
}

/******************************************************************************
 * Function: diag_esw_intr_test
 *
 * Description: Function to do Ethernet Switch interrupt test.
 * Inputs      : None
 * Outputs     : PASSED / FAILED
 *****************************************************************************/
int diag_esw_intr_test (void)
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
    cterr_add_env_dump((PFV)show_plat_curr_temps);

    /* Segment 7: Top 3 Debugging Steps */
    cterr_add_debug("Check the SGMII bus status to see "
                    "if it is normal among each component.",
                    "If the status is OK, contact vendor "
                    "to verify if SGMII driver is workable.");
#endif

    int rc = FAILED;
    void *esw_obj_p = NULL;

    /* enhance error msg: setting test name */
    char test_name[32] = "ESW Interrupt test";
    testname(test_name);
    prpass(testpass, "%s, ", test_name);
    printf("\n");

    if (platform_esw_type() == ESW_MRVL88E6390) {
        esw_obj_p = &dev_88e6390_obj;
    } else {
        esw_obj_p = &dev_88e6176_obj;
    }

    /* create ESW device object */
    rc = diag_esw_dev_create(esw_obj_p);
    if (rc == FAILED) {
        /* enhance error msg: error */
        cterr('f', 0, "%s:%d: Fail to create Marvell ESW Object", 
              __FUNCTION__, __LINE__);
        prcomplete(testpass, errcount, (char *)0);
        return (FAILED);
    }

    /* 88E6390 or 88E6176 Register test*/
    if (platform_esw_type() == ESW_MRVL88E6390) {
        rc =((dev_88e6390_object_t *)esw_obj_p)->callin_fvt->
            intr_test((dev_object_t *)esw_obj_p);

        if (rc != PASSED) {
            /* enhance error msg: error */
            cterr('f', 0, "%s:%d: Register test failed on 88E6390\n", 
                  __FUNCTION__, __LINE__);
            prcomplete(testpass, errcount, (char *)0);
        }

        ((dev_88e6390_object_t *)esw_obj_p)->base.dev_object_fvt->
        dev_destroy((dev_object_t **)&esw_obj_p);

    } else {
        rc =((dev_88e6176_object_t *)esw_obj_p)->callin_fvt->
        intr_test((dev_object_t *)esw_obj_p);

        if (rc != PASSED) {
            /* enhance error msg: error */
            cterr('f', 0, "%s:%d: Register test failed on 88E6176\n", 
                  __FUNCTION__, __LINE__);
            prcomplete(testpass, errcount, (char *)0);
        }

        ((dev_88e6176_object_t *)esw_obj_p)->base.dev_object_fvt->
        dev_destroy((dev_object_t **)&esw_obj_p);

    }
    
    /* Re-init ESW. */
    rc = diag_esw_init();
    if (rc == FAILED) {
        /* enhance error msg: error */
        cterr('f', 0, "%s:%d:Fail to re-init ESW device\n", __FUNCTION__, __LINE__);
        prcomplete(testpass, errcount, (char *)0);
        return (FAILED);
    }

    return (rc);
}

/*-------------------------------------------------
 * $Log: diag_esw_test.c,v $
 * Revision 1.2  2019/01/10 06:36:22  wilbhuan
 * The beginning of Betelgeuse application code.
 *
 *-------------------------------------------------
 */
