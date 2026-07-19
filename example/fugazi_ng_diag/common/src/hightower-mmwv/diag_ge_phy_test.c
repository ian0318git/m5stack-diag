/* $Id: diag_ge_phy_test.c,v 1.3 2021/06/02 02:56:21 alpeng Exp $ 
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/hightower-mmwv/diag_ge_phy_test.c,v $
 *-----------------------------------------------------------------------------
 *
 * Filename   : diag_ge_phy.c
 * Description: Chrysler GE PHY(Marvell 88E1514) Diag tests and utilities.
 *
 * Copyright (c) 2016~2020 by Cisco Systems, Inc.
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
#include "diag_ge_phy_lib.h"
#include "diag_ge_phy_test.h"
#include "platform_fru.h"
#include "platform_cookie.h"
#include "platform_sensor.h"
#include "i2c_api.h"
#include "platform_i2c.h"
#include "dev_88e151x.h"
#include "diag_ge_phy_lib.h"
#include "diag_eth_pkt_txrx.h"

/*******************************************************************************
 *                          Extern 
 *******************************************************************************
 */
extern int plat_mem_read32(uint, uint *); 
extern int plat_mem_write32(uint, uint);
extern uint32 diag_gephy_smi_wr(uint32, ushort);

/*******************************************************************************
 *                          Function Declaration
 *******************************************************************************
 */
int ge_phy_copper_ext_lpbk_test(int);
int chrysler_gephy_reg_test(int);
int chrysler_gephy_utils(int);
static int gephy_copper_mac_lpbk_test(int);
int gephy_set_sgmii_mode(int);
int diag_ge_phy_no;
extern uint32 diag_gephy_smi_wr(uint32, ushort);
int ge_phy_intr_test(void); 

/*******************************************************************************
 *                                Globals
 *******************************************************************************
 */

static submenu_xtable_t chrysler_gephy_diag_tbl[] = {
    {"GE PHY Utilities",
     (type_t(*)())chrysler_gephy_utils,                              FALSE,
     0, 
     (type_t(*)())0,                                             0,
     (type_t(*)())0,                                             0},
    {"PHY Register Test",
     (type_t(*)())chrysler_gephy_reg_test,                            0,
     (MF_CONTINUOUS | MF_DOGRP | MF_DOALL | MF_SHOW_ERRCOUNT),
     (type_t(*)())0,                                             0,
     (type_t(*)())0,                                             0},
    {"CPU to PHY MAC Loopback Test",
     (type_t(*)())gephy_copper_mac_lpbk_test,                    0,
     (MF_CONTINUOUS | MF_DOGRP | MF_DOALL | MF_SHOW_ERRCOUNT),
     (type_t(*)())0,                                             0,
     (type_t(*)())0,                                             0},
    {"Copper port External Loopback Test",
     (type_t(*)())ge_phy_copper_ext_lpbk_test,                   0,
     (MF_CONTINUOUS | MF_DOGRP | MF_DOALL | MF_SHOW_ERRCOUNT),
     (type_t(*)())0,                                             0,
     (type_t(*)())0,                                             0},
    {"1G PHY interrupt Test",              
     (type_t(*)())ge_phy_intr_test,                              0,
     (MF_CONTINUOUS | MF_DOGRP | MF_DOALL | MF_SHOW_ERRCOUNT),
     (type_t(*)())0,                                             0,
     (type_t(*)())0,                                             0},
};

#define CHRYSLER_GEPHY_DIAG_TBL_SIZE (sizeof(chrysler_gephy_diag_tbl) / \
                                  sizeof(submenu_xtable_t))

/*
 * Primary & secondary submenu items (filled in from xtable)
 */
static mitem_t chrysler_gephy_diag_pri_items[CHRYSLER_GEPHY_DIAG_TBL_SIZE + MAX_BASE_ITEMS];
static mitem_t chrysler_gephy_diag_sec_items[CHRYSLER_GEPHY_DIAG_TBL_SIZE + MAX_BASE_ITEMS];

menuinfo_t chrysler_gephy_diag_menu = {
    "%s Subtest Menu",
    0,                                /* mtparam added by init_empty_menu */
    (PFT)show_endnote,                /* notes missing WICs in combos */
    0,                                /* use generic prompt */
    0,                                /* size (bumped by add_menu_item() */
    chrysler_gephy_diag_pri_items,
};
menuinfo_t *chrysler_gephy_diag_menup = &chrysler_gephy_diag_menu;

/* List of GE PHY Utilities */
static submenu_xtable_t gephy_util_items[] = {
    {"GE PHY register read",        (type_t(*)())chrysler_gephy_reg_rd_util,
     0,                0,
     (type_t(*)())0,                0,
     (type_t(*)())0,                0},
    {"GE PHY register write",       (type_t(*)())chrysler_gephy_reg_wr_util,
     0,                0,
     (type_t(*)())0,                0,
     (type_t(*)())0,                0},
};

#define GEPHY_UTIL_SIZE (sizeof(gephy_util_items) / sizeof(submenu_xtable_t))

/*
 * GE PHY 0 utility items (filled in from xtable)
 */
static mitem_t gephy_util_pri_items[GEPHY_UTIL_SIZE + MAX_BASE_ITEMS];
static mitem_t gephy_util_sec_items[GEPHY_UTIL_SIZE + MAX_BASE_ITEMS];

/*
 * GE PHY Utility Submenu
 */
menuinfo_t gephy_util_menu = {
    "%s Menu",
    0,
    (PFT)show_endnote,
    0,
    0,
    gephy_util_pri_items,
};

menuinfo_t *gephy_util_menup = &gephy_util_menu;

/******************************************************************************
 *
 * Function: diag_gephy_lpbk_test
 *
 * Description: Function to perform internal/external loopback test
 *
 * Inputs      : lpbk_mode - Internal/External
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/
static int diag_gephy_lpbk_test (int lpbk_mode)
{
    dev_88e151x_object_t gephy_obj;
    dev_88e151x_object_t *gephy_obj_p = &gephy_obj;
    int rc = FAILED, retry_count = 0;
    int ix, jx, link_status;
    char *speed_str = {"1000Mbps"};
    char iface_name[16] = "eth1";

    testname("External Loopback");

    rc = diag_gephy_dev_create(diag_ge_phy_no, gephy_obj_p);

    if (rc == FAILED) {
        cterr('f', 0, "%s: Creating object failed", __func__);
        return (FAILED);
    }

    for (ix = DEV_88E151X_SPD_1000; ix <= DEV_88E151X_SPD_1000; ix++) {
        retry_count = 0;
        prpass(testpass, "Speed %s", speed_str);

retry:
        /* Set speed */
        rc = gephy_obj_p->callin_fvt->set_spd((dev_object_t *)gephy_obj_p, ix);
        if (rc == FAILED) {
            cterr('f', 0, "Set Speed to %s Failed", speed_str);
            goto __exit;
        }
        rc = gephy_obj_p->callin_fvt->set_lpbk((dev_object_t *)gephy_obj_p,
                                                lpbk_mode, ix);

        if (rc != PASSED) {
            cterr('f', 0, "Enable Loopback for %s fails", speed_str);
            goto __exit;
        }

        prpass(testpass, "Running Loopback ");

        /* Make sure that link status is detected before performing loopback */
        for (jx = 0; jx < LPBK_LINK_UP_TOUT; jx++) {
            link_status = dnv_eth_link_is_up(diag_ge_phy_no);

            if (link_status == TRUE) {
                break;
            }
            msleep(12);
        }

        if (jx == LPBK_LINK_UP_TOUT || link_status == FALSE) {
            if (retry_count == CHRYSLER_LPBK_RETRY) {
                rc = FAILED;
                cterr('f', 0, "Link (%s) is not detected", speed_str);
                goto __exit;
            } else {
                retry_count++;
                goto retry;
            }
        }

        rc = eth_pkt_txrx(iface_name, LPBKTEST_PKT_CNT, FALSE);
        if ((retry_count == CHRYSLER_LPBK_RETRY) && (rc == FAILED)) {
            cterr('f', 0, "Loopback failed (%s)", speed_str);
            goto __exit;
        }

        if (rc == FAILED) {
            retry_count++;
            printf("\nWarning this is external lpbk retry %d time\n", retry_count);
            goto retry;
        }
    }
__exit:
    /* Set speed to 1G */
    rc = gephy_obj_p->callin_fvt->set_spd((dev_object_t *)gephy_obj_p,
                                           DEV_88E151X_SPD_1000);

    if (rc != PASSED) {
        cterr('f', 0, "Set speed to 1G fails");
    }

    /* Disable Loopback */
    rc = gephy_obj_p->callin_fvt->set_lpbk((dev_object_t *)gephy_obj_p,
                                            DEV_88E151X_DIS_LPBK,
                                            DEV_88E151X_SPD_1000);

    if (rc != PASSED) {
        cterr('f', 0, "Disable loopback fails");
    }

    gephy_obj_p->base.dev_object_fvt->dev_destroy((dev_object_t **)&gephy_obj_p);

    prcomplete(testpass, errcount, (char *)0);

    return (rc);
}

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
    if (check_ext_lpbk_flag()) {
        return (diag_gephy_lpbk_test(DEV_88E151X_EXT_LPBK));
    } else {
        printf("External Loopback flag is off. Skip the test\n");
        return (PASSED);
    }

    return (PASSED);
}

/******************************************************************************
 *
 * Function   : chrysler_gephy_diag
 * Description: Entrance of Chrysler GE PHY (88E1514) Diag tests.
 * Inputs     : show_menu - menu option
 * Outputs    : PASSED / FAILED
 *
 *******************************************************************************/
int chrysler_gephy_diag (int show_menu)
{
    char *menu_title= "GE PHY 1514 Test";

    diag_ge_phy_no = CHRYSLER_1514_GE_PHY_ADDR;

    build_primary_submenu(chrysler_gephy_diag_tbl,
                          CHRYSLER_GEPHY_DIAG_TBL_SIZE,
                          menu_title, &chrysler_gephy_diag_menup);
    build_secondary_submenu(chrysler_gephy_diag_tbl,
                            CHRYSLER_GEPHY_DIAG_TBL_SIZE,
                            chrysler_gephy_diag_sec_items);

    if (show_menu) {
        do_all_menu_items(chrysler_gephy_diag_menup);

    } else {
        menu(chrysler_gephy_diag_menup, chrysler_gephy_diag_sec_items, 0);
    }
    return (PASSED);
}

/*******************************************************************************
 *  
 * Function   : chrysler_gephy_reg_test
 * Description: Function performs Chrysler GE PHY(Marvell 88E1514) register test.
 * Inputs     : GE port number(GE0/GE1)
 * Outputs    : PASSED / FAILED
 *         
 *******************************************************************************
 */
int chrysler_gephy_reg_test (int ge_num)
{
    dev_88e151x_object_t gephy_obj;
    dev_88e151x_object_t *gephy_obj_p = &gephy_obj;
    int rc;

     testname("GE PHY Register");

    rc = diag_gephy_dev_create(diag_ge_phy_no, gephy_obj_p);

    if (rc == FAILED) {
        cterr('f', 0, "%s: Creating object failed", __func__);
        return (FAILED);
    }

    prpass(testpass, "Register Test");

    rc = gephy_obj_p->callin_fvt->register_test((dev_object_t *)gephy_obj_p);

    gephy_obj_p->base.dev_object_fvt->dev_destroy((dev_object_t **)&gephy_obj_p);

    prcomplete(testpass, errcount, (char *)0);

    return (rc);
}

/*******************************************************************************
 *
 * Function    : chrysler_gephy_utils
 * Description : Entry of Chrysler GE PHY utilities.
 * Inputs      : opt - reserve for future use
 * Outputs     : PASSED / FAILED
 *
 *******************************************************************************
 */
int chrysler_gephy_utils (int opt)
{
    build_primary_submenu(gephy_util_items, GEPHY_UTIL_SIZE,
                          "GE PHY Utilities", &gephy_util_menup);
    build_secondary_submenu(gephy_util_items, GEPHY_UTIL_SIZE,
                            gephy_util_sec_items);

    menu(gephy_util_menup, gephy_util_sec_items, '\0' );

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
    char iface_name[16] = "eth1";
    int rc = FAILED, retry_count = 0;
    int ix, jx, link_status;
    char *speed_str = {"NULL"};

    testname("PHY 1514 Internal Loopback");
    for (ix = SPD_10; ix <= SPD_100; ix++) {
        if (ix == SPD_10) {
speed_10M_retry:
            speed_str = "10M";
            prpass(testpass, "Running 10M ");
            //printf("Running PHY 1514 10M internal Loopback\n");

            system(ETHTOOL_SPD_10);
            msleep(10);
            rc = diag_gephy_smi_wr(COPPER_SPECIFIC_CTRL_REG, FORCE_LINK_UP_VAL);
            if (rc == FAILED) {
                cterr('f', 0, "Failed to force link up. Line - %d", __LINE__);
            }
            rc = diag_gephy_smi_wr(SWITCH_PAGE, MAC_PAGE_2);
            if (rc == FAILED) {
                cterr('f', 0, "Failed to switch to page 2. Line - %d", __LINE__);
            }
            rc = diag_gephy_smi_wr(MAC_SPECIFIC_CTRL_REG, MAC_SPD_10);
            if (rc == FAILED) {
                cterr('f', 0, "Failed to alter MAC speed to 10M. Line - %d", __LINE__);
            }
            rc = diag_gephy_smi_wr(SWITCH_PAGE, COPPER_PAGE_0);
            if (rc == FAILED) {
                cterr('f', 0, "Failed to switch to page 0. Line - %d", __LINE__);
            }
            rc = diag_gephy_smi_wr(COPPER_CTRL_REG, SOFT_RESET_SPD_10);
            if (rc == FAILED) {
                cterr('f', 0, "Failed to do soft reset. Line - %d", __LINE__);
            }
            system(ETHTOOL_SPD_1000);
            msleep(10);
            system(ETHTOOL_SPD_10);
            msleep(10);
            rc = diag_gephy_smi_wr(COPPER_CTRL_REG, EN_LPBK_SPD_10);
            if (rc == FAILED) {
                cterr('f', 0, "Failed to enable 10M lpbk bit. Line - %d", __LINE__);
            }
        } else {
speed_100M_retry:
            prpass(testpass, "Running 100M ");
            //printf("Running PHY 1514 100M internal Loopback\n");
            speed_str = "100M";

            system(ETHTOOL_SPD_100);
            msleep(10);
            rc = diag_gephy_smi_wr(COPPER_SPECIFIC_CTRL_REG, FORCE_LINK_UP_VAL);
            if (rc == FAILED) {
                cterr('f', 0, "Failed to force link up. Line - %d", __LINE__);
            }
            rc = diag_gephy_smi_wr(SWITCH_PAGE, MAC_PAGE_2);
            if (rc == FAILED) {
                cterr('f', 0, "Failed to switch to page 2. Line - %d", __LINE__);
            }
            rc = diag_gephy_smi_wr(MAC_SPECIFIC_CTRL_REG, MAC_SPD_100);
            if (rc == FAILED) {
                cterr('f', 0, "Failed to alter MAC speed to 100M. Line - %d", __LINE__);
            }
            rc = diag_gephy_smi_wr(SWITCH_PAGE, COPPER_PAGE_0);
            if (rc == FAILED) {
                cterr('f', 0, "Failed to switch to page 0. Line - %d", __LINE__);
            }
            rc = diag_gephy_smi_wr(COPPER_CTRL_REG, SOFT_RESET_SPD_100);
            if (rc == FAILED) {
                cterr('f', 0, "Failed to do soft reset. Line - %d", __LINE__);
            }
            system(ETHTOOL_SPD_1000);
            msleep(10);
            system(ETHTOOL_SPD_100);
            msleep(10);
            rc = diag_gephy_smi_wr(COPPER_CTRL_REG, EN_LPBK_SPD_100);
            if (rc == FAILED) {
                cterr('f', 0, "Failed to enable 100M lpbk bit. Line - %d", __LINE__);
            }
        }

        /* Make sure that link status is detected before performing loopback */
        for (jx = 0; jx < LPBK_LINK_UP_TOUT; jx++) {
            link_status = dnv_eth_link_is_up(diag_ge_phy_no);

            if (link_status == TRUE) {
                printf("\n%s speed link is UP\n", speed_str);
                break;
            }
            msleep(12);
        }

        if (jx == LPBK_LINK_UP_TOUT || link_status == FALSE) {
            if (retry_count == CHRYSLER_LPBK_RETRY) {
                rc = FAILED;
                cterr('f', 0, "Link (%s) is not detected", speed_str);
                goto __internal_exit;
            } else {
                retry_count++;
                if (ix == SPD_10) {
                    goto speed_10M_retry;
                } else {
                    goto speed_100M_retry;
                }
            }
        }

        rc = eth_pkt_txrx(iface_name, LPBKTEST_PKT_CNT, FALSE);
        if ((retry_count == CHRYSLER_LPBK_RETRY) && (rc == FAILED)) {
            cterr('f', 0, "PHY 1514 internal Loopback failed (%s)", speed_str);
            goto __internal_exit;
        }

        if (rc == FAILED) {
            retry_count++;
            printf("\nWarning this is internal lpbk retry %d time\n", retry_count);
            if (ix == SPD_10) {
                goto speed_10M_retry;
            } else {
                goto speed_100M_retry;
            }
        } else {
            printf("\nPHY 1514 %s speed internal loopback test is passed\n", speed_str);
        }
    }

__internal_exit:
    rc = diag_gephy_smi_wr(COPPER_SPECIFIC_CTRL_REG, DISABLE_FORCE_LINK_UP_VAL);
    if (rc == FAILED) {
        cterr('f', 0, "Failed to disable force link up. Line - %d", __LINE__);
    }

    rc = diag_gephy_smi_wr(SWITCH_PAGE, MAC_PAGE_2);
    if (rc == FAILED) {
        cterr('f', 0, "Failed to switch to page 2. Line - %d", __LINE__);
    }

    rc = diag_gephy_smi_wr(MAC_SPECIFIC_CTRL_REG, MAC_SPD_1000);
    if (rc == FAILED) {
        cterr('f', 0, "Failed to alter MAC speed to 1G. Line - %d", __LINE__);
    }

    rc = diag_gephy_smi_wr(SWITCH_PAGE, COPPER_PAGE_0);
    if (rc == FAILED) {
        cterr('f', 0, "Failed to switch to page 0. Line - %d", __LINE__);
    }

    rc = diag_gephy_smi_wr(COPPER_CTRL_REG, SOFT_RESET_VAL);
    if (rc == FAILED) {
        cterr('f', 0, "Failed to do soft reset. Line - %d", __LINE__);
    }

    system(ETHTOOL_SPD_1000);
    msleep(10);

    prcomplete(testpass, errcount, (char *)0);

    return (rc);
}

/*******************************************************************************
 *
 * Function   : ge_phy_intr_test
 * Description: Function to do GE PHY MAC interrupt test.
 * Inputs     : None
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
int ge_phy_intr_test (void) 
{
    uint   reg_val = 0;
    uint   smi_regaddr = (uint)CN9130_GE_PHY_INTR_REG; 
    int    rc = FAILED;

    /* Note: GE PHY interrupt is active LOW, 
     * we check interrupt after disable interrut */
    testname("Interrupt "); 

    /* clean up intr, 0xf2440154 = 0*/
    if (plat_mem_write32(smi_regaddr, reg_val) != PASSED) {
        cterr('f', 0, "%s:%d Failed to write CPU GPIO Intr Reg.(0x%08X).\n",
               __FUNCTION__, __LINE__, smi_regaddr);
        return (FAILED);
    } else { 
        prpass(testpass, "Clean up CPU interrupt status reg"); 
    } 

    /* enable PHY interrupt and force interrupt */
    rc = diag_gephy_smi_wr(SWITCH_PAGE, MAC_PAGE_3);
    if (rc == FAILED) {
        cterr('f', 0, "Failed to switch to page 3. Line - %d", __LINE__);
    }

    rc = diag_gephy_smi_wr(LED_TIMRER_CTRL_REG, GEPHY_FORCE_INTR); 
    if (rc == FAILED) {
        cterr('f', 0, "Failed to force intr. Line - %d", __LINE__);
    } else { 
        prpass(testpass, "Force GE PHY interrupt"); 
    }
 
    /* disable PHY interrupt and force interrupt */
    rc = diag_gephy_smi_wr(LED_TIMRER_CTRL_REG, GEPHY_CLEAR_INTR); 
    if (rc == FAILED) {
        cterr('f', 0, "Failed to disable force intr. Line - %d", __LINE__);
    } else { 
        prpass(testpass, "Disable force GE PHY interrupt"); 
    }

    /* important! delay is needed for interrupt pull up. */
    msleep(100);

    /* check interrupt */
    if (plat_mem_read32(smi_regaddr, &reg_val) != PASSED) {
        cterr('f', 0, "%s:%d Failed to read CPU GPIO Intr Reg.(0x%08X).\n",
               __FUNCTION__, __LINE__, smi_regaddr);
        return (FAILED);
    } else {
        if (reg_val & GEPHY_INTR_BIT) { 
            prpass(testpass, "CPU detects PHY interrupt"); 
        } else { 
            cterr('f', 0, "%s:%d Failed to detect CPU GPIO Intr Reg.(0x%08X)=0x%x.\n",
                   __FUNCTION__, __LINE__, smi_regaddr, reg_val);
            return (FAILED);
       }     
    } 

    /* clean up intr, 0xf2440154 = 0*/
    if (plat_mem_write32(smi_regaddr, reg_val) != PASSED) {
        cterr('f', 0, "%s:%d Failed to write CPU GPIO Intr Reg.(0x%08X).\n",
               __FUNCTION__, __LINE__, smi_regaddr);
        return (FAILED);
    }

    return (PASSED); 

}


/*-------------------------------------------------
 * $Log: diag_ge_phy_test.c,v $
 * Revision 1.3  2021/06/02 02:56:21  alpeng
 * merge sears into trunk
 *
 * Revision 1.2  2021/01/25 09:21:49  markzha
 * Sync RDT issues fixing and optimize compiling for Highrise
 *
 * Revision 1.1  2020/08/19 09:50:04  markzha
 * *** empty log message ***
 *
 * $Endlog$
 *-------------------------------------------------
 */
