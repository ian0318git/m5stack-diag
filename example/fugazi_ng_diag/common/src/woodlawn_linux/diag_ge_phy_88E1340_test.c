/* $Id: diag_ge_phy_88E1340_test.c,v 1.2 2013/10/08 08:48:28 tirawan Exp $ 
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/woodlawn_linux/diag_ge_phy_88E1340_test.c,v $
 *-----------------------------------------------------------------------------
 * diag_ge_phy_88E1340_test.c - Menu for Woodlawn PHY 88E1340
 *
 * February 2012, Leslie Chen
 * Copyright (c) 2013 by Cisco Systems, Inc.
 * All rights reserved.
 *-----------------------------------------------------------------------------
 */
#include "error.h"
#include "common.h"
#include "types.h"
#include "menu.h"
#include "queryflags.h"
#include "ethernet.h"
#include "common_utils.h"
#include "diag_ge_phy_88E1340_lib.h"
#include "diag_ge_phy_88E1548L_lib.h"
#include "platform_eth.h"
#include "dev_phy_88e1340.h"
#include "platform_ext_lpbk.h"
#include "diag_common_drv.h"
#include "diag_fpga_lib.h"

#define MF_1    (MF_CONTINUOUS | MF_DOGRP)
#define MF_2    (MF_1 | MF_DOALL)
#define MF_3    (MF_2 | MF_SHOW_ERRCOUNT)
#define MF_4    (MF_1 | MF_SHOW_ERRCOUNT)

static int ge_phy_88E1340_utility(int);
static int ge_phy_88E1340_register_test(int);
static int ge_phy_88E1340_internal_loopback_test(int);
int dump_phy_88E1340_registers(void);
int alter_phy_88E1340_register(void);
static int has_second_phy(void);
int ge_phy_88E1340_test(int);
int ge_88E1340_do_all_wrapper(void);

static int ge_mapping_phy_addr[] = {0x9, 0x8, 0xb, 0xa, 0xf, 0xe, 0xd, 0xc};

/* Sub Menu used for GE phy 88E1340 tests.*/
static submenu_xtable_t ge_phy_88E1340_tests_submenu_table[] = {
   {"PHY Utilities", (type_t(*)())ge_phy_88E1340_utility,   FALSE,
    0, NULL, 0, (type_t(*)())ge_phy_88E1340_utility,   TRUE}, 
   {"PHY 0 Register Test", (type_t(*)())ge_phy_88E1340_register_test,   MRVL_1340_PHY0,
    MF_3, (type_t(*)())0, 0, (type_t(*)())0,   0},
   {"PHY 1 Register Test", (type_t(*)())ge_phy_88E1340_register_test,   MRVL_1340_PHY1,
    MF_3, (type_t(*)())has_second_phy, 0, (type_t(*)())0,   0},
   {"PHY 0 Loopback Test", (type_t(*)())ge_phy_88E1340_internal_loopback_test,   MRVL_1340_PHY0,
    0, (type_t(*)())0, 0, (type_t(*)())0,   0},
    {"PHY 1 Loopback Test", (type_t(*)())ge_phy_88E1340_internal_loopback_test,   MRVL_1340_PHY1,
    0, (type_t(*)())has_second_phy, 0, (type_t(*)())0,   0},
};

#define GE_PHY_88E1340_TESTS_SUBMENU_TABLE_SIZE (sizeof(ge_phy_88E1340_tests_submenu_table) / \
            sizeof(submenu_xtable_t))

/*
 * Primary & secondary submenu items (filled in from xtable)
 */
static mitem_t gy_phy_88E1340_tests_primary_items[GE_PHY_88E1340_TESTS_SUBMENU_TABLE_SIZE +
                     MAX_BASE_ITEMS];
static mitem_t ge_phy_88E1340_tests_secondary_items[GE_PHY_88E1340_TESTS_SUBMENU_TABLE_SIZE +
                     MAX_BASE_ITEMS];

menuinfo_t ge_phy_88E1340_subtest_menu = {
    "%s Subtest Menu",
    0,                                /* mtparam added by init_empty_menu */
    (PFT)show_endnote,                /* notes missing WICs in combos */
    0,                                /* use generic prompt */
    0,                                /* size (bumped by add_menu_item() */
    gy_phy_88E1340_tests_primary_items,
};
menuinfo_t *ge_phy_88E1340_submenup = &ge_phy_88E1340_subtest_menu;

/* List of GE phy 88E1340 Utilities */
static submenu_xtable_t ge_phy_88E1340_util_items[] = {
    {"Dump PHY Registers", (type_t(*)())dump_phy_88E1340_registers, 0, 0, (type_t(*)())0,
     0, (type_t(*)())0, 0},
    {"Alter PHY Registers", (type_t(*)())alter_phy_88E1340_register, 0, 0,
     (type_t(*)())0, 0, (type_t(*)())0, 0},
};

#define GE_PHY_88E1340_TESTS_UTIL_SIZE (sizeof(ge_phy_88E1340_util_items) / \
                                     sizeof(submenu_xtable_t))

/*
 * ge phy 88E1340 util items (filled in from xtable)
 */
static mitem_t ge_phy_88E1340_tests_primary_util_items[GE_PHY_88E1340_TESTS_UTIL_SIZE +
                                     MAX_BASE_ITEMS];
static mitem_t ge_phy_88E1340_tests_secondary_util_items[GE_PHY_88E1340_TESTS_UTIL_SIZE +
                                     MAX_BASE_ITEMS];

/*
 * GE phy 88E1340 Utils submenu
 */
menuinfo_t ge_phy_88E1340_util_menu = {
    "GE PHY 88E1340 Utility Menu",
    0,
    (PFT)show_endnote,
    0,
    0,
    ge_phy_88E1340_tests_primary_util_items,
};

menuinfo_t *ge_phy_88E1340_util_menup = &ge_phy_88E1340_util_menu;


/******************************************************************************
 *
 * Function: ge_phy_88E1340_test
 *
 * Description: Main entrance for 88E1340 menu
 *
 * Inputs      : port - port number
 * Outputs     : PASSED / FAILED
 *
 *******************************************************************************/
int ge_phy_88E1340_test (int show_menu)
{
    build_primary_submenu(ge_phy_88E1340_tests_submenu_table,
                        GE_PHY_88E1340_TESTS_SUBMENU_TABLE_SIZE,
                        "GE PHY 88E1340", &ge_phy_88E1340_submenup);
    build_secondary_submenu(ge_phy_88E1340_tests_submenu_table,
                        GE_PHY_88E1340_TESTS_SUBMENU_TABLE_SIZE,
                        ge_phy_88E1340_tests_secondary_items);

    if (show_menu) {
        menu(ge_phy_88E1340_submenup, ge_phy_88E1340_tests_secondary_items, '\0' );

    } else {
        menu_exec_doall_diags(ge_phy_88E1340_submenup);
    }
    return (PASSED);
}


/*******************************************************************************
 *
 * Function    : ge_88E1340_do_all_wrapper
 * Description : Wrapper for GE PHY 88E1340 do all test items
 * Inputs      : none
 * Outputs     : PASSED / FAILED
 *
 *******************************************************************************
 */
int ge_88E1340_do_all_wrapper (void)
{
    int rc = PASSED;

    if (ge_phy_88E1340_register_test(MRVL_1340_PHY0) == FAILED) {
        rc = FAILED;
    }

    if (has_second_phy()) {
        if (ge_phy_88E1340_register_test(MRVL_1340_PHY1) == FAILED) {
            rc = FAILED;
        }
    }

    return (rc);
}

/******************************************************************************
 *  
 * Function: ge_phy_88E1340_register_test
 *    
 * Description: This function performs the 88E1340 register test.
 *   
 * Inputs      : port - port number
 * Outputs     : PASSED / FAILED
 *         
 *******************************************************************************/
static int ge_phy_88E1340_register_test (int port)
{
    dev_88e1340_object_t *mvl_1340_obj;
    uint ix, phy_addr, no_of_phy;
    int id;

    mvl_1340_obj = (dev_88e1340_object_t *)diag_get_88e1340_obj(port);

    if (mvl_1340_obj == NULL) {
        cterr('f', 0, "%s: Mrvl 88e1340 Null Object", __FUNCTION__);
        return (FAILED);
    }

    testname("88E1340 PHY %d", port);

    /* First 88E1340 has 4 PHYs, while the second one only has 2 */
    if (port == MRVL_1340_PHY0) {
        no_of_phy = 4;
        phy_addr = MRVL_88E1340_PHY0_SMI_ADDR;
    } else {
        /* New SKU just have 4 GE ports */
        id = get_sku_id();
        if (id == WOODLAWN_4GE_1XAUI) {
            return (PASSED);
        } else {
            no_of_phy = 2;
            phy_addr = MRVL_88E1340_PHY1_SMI_ADDR;
        }
    }

    for (ix = 0; ix < no_of_phy; ix++) {

        if (MVL_88E1340_REG_TEST_SINGLE(mvl_1340_obj, phy_addr) == FAILED) {
            cterr('f', 0, "Register Test on address %#x fails.", phy_addr);
            return (FAILED);
        }

        /* Address is decreasing */
        phy_addr--;
    }

    return (PASSED);
}

/***********************************************************************
 *  
 * Function: ge_phy_88E1340_internal_loopback_test
 *    
 * Description: Do GE PHY loopback test
 *    
 * Inputs: phy - phy number
 *        
 * Outputs: PASSED/FAILED
 *          
 *************************************************************************/
static int ge_phy_88E1340_internal_loopback_test (int phy)
{
    return (woodlawn_phy_lpbk_test(phy, BRIDGE_PHY_INT_LPBK));
}

/***********************************************************************
 *  
 * Function: dump_phy_88E1340_registers
 *    
 * Description: Display GE PHY registers
 *      
 * Inputs: None
 *        
 * Outputs: PASSED/FAILED
 *          
 ************************************************************************/
int dump_phy_88E1340_registers (void)
{
    dev_88e1340_object_t *mvl_1340_obj;
    int ix, port_num, port_map, phy;
    int sku_id, max_port, base_devaddr;

    sku_id = get_sku_id();

    if (sku_id == WOODLAWN_6GE_1XAUI) {
        max_port = MAX_7_PORT;
        /* Not official SKU */
        printf("Enter port number\n");
        printf("Enter 0 - GE0(PHY0)\n");
        printf("Enter 1 - GE1(PHY0)\n");
        printf("Enter 2 - GE2(PHY0)\n");
        printf("Enter 3 - GE3(PHY0)\n");
        printf("Enter 4 - GE4(PHY1)\n");
        printf("Enter 5 - GE5(PHY1)\n");
        printf("Enter 6 - PORT2(PHY1)\n");
        printf("Enter 7 - PORT3(PHY1)\n");
    } else {
        /* Official SKUs */
        if (sku_id == WOODLAWN_6GE) {
            max_port = MAX_5_PORT;
            printf("Enter port number\n");
            printf("Enter 0 - GE0(PHY1)\n");
            printf("Enter 1 - GE1(PHY1)\n");
            printf("Enter 2 - GE2(PHY0)\n");
            printf("Enter 3 - GE3(PHY0)\n");
            printf("Enter 4 - GE4(PHY0)\n");
            printf("Enter 5 - GE5(PHY0)\n");
        } else if (sku_id == WOODLAWN_4GE_1XAUI) {
            max_port = MAX_4_PORT;
            printf("Enter port number\n");
            printf("Enter 0 - GE0(PHY0)\n");
            printf("Enter 1 - GE1(PHY0)\n");
            printf("Enter 2 - GE2(PHY0)\n");
            printf("Enter 3 - GE3(PHY0)\n");
        }
    }
    
    port_num = getdec_answer("Select port", 0, 0, max_port);

    if (sku_id == WOODLAWN_6GE_1XAUI) {
        /* Mapping front panel GE port number to phy port number */
        port_map = old_ge_mapping_phy_port[port_num];

        if (port_num < 4) {
            phy = MRVL_1340_PHY0;
        } else {
            phy = MRVL_1340_PHY1;
        }
    } else {
        /* Mapping front panel GE port number to phy port number */
        if (sku_id == WOODLAWN_6GE) {
            port_map = two_phy_ge_mapping_phy_port[port_num];
            /* New SKU has GE port 2~5 in phy0, GE port 0~1 and 6~7 in phy 1*/
            if ((GE_PORT2 <= port_num) && (port_num <= GE_PORT5)) {
                phy = MRVL_1340_PHY0;
            } else {
                phy = MRVL_1340_PHY1;
            }
        } else if (sku_id == WOODLAWN_4GE_1XAUI) {
            port_map = one_phy_ge_mapping_phy_port[port_num];
            /* One new SKU just have 4 GE ports */
            phy = MRVL_1340_PHY0;
        }
    }

    mvl_1340_obj = (dev_88e1340_object_t *)diag_get_88e1340_obj(phy);

    if (mvl_1340_obj == NULL) {
        cterr('f', 0, "%s: Mrvl 88e1340 Null Object", __FUNCTION__);
        return (FAILED);
    }

    base_devaddr = (int)mvl_1340_obj->base_phyaddr;
    printf("Dump 88E1340 PHY %d Registers:\n", phy);
    if (MVL_88E1340_SHOW_REG(mvl_1340_obj, (print_fn_t)&printf,
        base_devaddr - port_map) == FAILED) {
        printf("Dump port %d fails\n", ix);
        return (FAILED);
    }

    return (PASSED);
}

/***********************************************************************
 *  
 * Function: alter_phy_88E1340_register
 *    
 * Description: Alter GE PHY registers
 *      
 * Inputs: None
 *        
 * Outputs: PASSED/FAILED
 *          
 *************************************************************************/
int alter_phy_88E1340_register (void)
{
    dev_88e1340_object_t *mvl_1340_obj;
    int phy_num, phy;
    int sku_id, port_num;
    uint phy_addr;

    sku_id = get_sku_id();

    if (sku_id == WOODLAWN_6GE_1XAUI) {
        /* Not official SKU */
        printf("Enter PHY number(PHY0, PHY1)\n");
        printf("Enter 0 - PHY0(GE0, GE1, GE2 ,GE3)\n");
        printf("Enter 1 - PHY1(GE4, GE5, PORT2, PORT3)\n");
    } else {
        /* Official SKUs */
        if (sku_id == WOODLAWN_6GE) {
            printf("Enter PHY number(PHY0, PHY1)\n");
            printf("Enter 0 - PHY0(GE2, GE3, GE4, GE5)\n");
            printf("Enter 1 - PHY1(GE0, GE1)\n");
        } else if (sku_id == WOODLAWN_4GE_1XAUI) {
            phy_num = 0;
        }
    }

    if (sku_id != WOODLAWN_4GE_1XAUI) {
        phy_num = getdec_answer("Select phy", 0, 0, 1);
    }

    if (sku_id == WOODLAWN_6GE_1XAUI) {
        /* Not official SKU */
        if (phy_num == 0) {
            phy = MRVL_1340_PHY0;
        } else {
            phy = MRVL_1340_PHY1;
        }
    } else {
        if (phy_num == 0) {
            /* New SKU has GE port 2~5 in phy0, GE port 0~1 and 6~7 in phy 1*/
            phy = MRVL_1340_PHY0;
        } else {
            /* One new SKU just have 4 GE ports */
            if (sku_id == WOODLAWN_4GE_1XAUI) {
                return (PASSED);
            } else {
                phy = MRVL_1340_PHY1;
            }
        }
    }
    
    mvl_1340_obj = (dev_88e1340_object_t *)diag_get_88e1340_obj(phy);

    printf("Enter port number\n");
    printf("Enter 0 - GE0(PHY0)\n");
    printf("Enter 1 - GE1(PHY0)\n");
    printf("Enter 2 - GE2(PHY0)\n");
    printf("Enter 3 - GE3(PHY0)\n");
    printf("Enter 4 - GE4(PHY1)\n");
    printf("Enter 5 - GE5(PHY1)\n");
    printf("Enter 6 - PORT2(PHY1)\n");
    printf("Enter 7 - PORT3(PHY1)\n");
    port_num = gethex_answer("Select port", 0, 0, 0x7);
    /* Mapping front panel GE port number to phy address */
    phy_addr = ge_mapping_phy_addr[port_num];

    mvl_1340_obj->base_phyaddr = phy_addr;
    if (mvl_1340_obj == NULL) {
        cterr('f', 0, "%s: Mrvl 88e1340 Null Object", __FUNCTION__);
        return (FAILED);
    }

    return (MVL_88E1340_ALTER_REG(mvl_1340_obj));
}

/*******************************************************************************
 *
 * Function    : ge_phy_88E1340_utility
 * Description :
 * Inputs      : menu_option - display menu instead of running all temp. sensor tests.
 * Outputs     : PASSED / FAILED
 *
 *******************************************************************************
 */

static int ge_phy_88E1340_utility (int show_menu)
{
    build_primary_submenu(ge_phy_88E1340_util_items, GE_PHY_88E1340_TESTS_UTIL_SIZE,
                          "GE PHY 88E1340 Utilities Menu", &ge_phy_88E1340_util_menup);
    build_secondary_submenu(ge_phy_88E1340_util_items, GE_PHY_88E1340_TESTS_UTIL_SIZE,
                            ge_phy_88E1340_tests_secondary_util_items);

    if (show_menu) {
        menu(ge_phy_88E1340_util_menup, ge_phy_88E1340_tests_secondary_util_items, '\0' );
    } else {
        menu_exec_doall_diags(ge_phy_88E1340_util_menup);
    }

    return (PASSED);
}

/*******************************************************************************
 *  
 * Function    : has_second_phy
 *
 * Description : Judge whether have second phy
 *
 * Inputs      : None
 *
 * Outputs     : TRUE / FALSE
 *       
 ********************************************************************************/
static int has_second_phy (void)
{
    int id;

    id = get_sku_id();

    if (id == WOODLAWN_4GE_1XAUI) {
        return (FALSE);
    } else {
        return (TRUE);
    }
}

/*-------------------------------------------------
 * $Log: diag_ge_phy_88E1340_test.c,v $
 * Revision 1.2  2013/10/08 08:48:28  tirawan
 * Woodlawn collapsed to main trunk
 *
 * Revision 1.1.4.2  2013/08/20 10:58:52  tirawan
 * Branch into woodlawn-branch2 and port woodlawn code
 *
 * Revision 1.1.2.2  2013/06/17 10:52:58  leschen
 * Modify 88e1340 alter reg utility
 *
 * Revision 1.1.2.1  2013/04/24 10:37:16  tirawan
 * Initial check-in for woodlawn linux code
 *
 * Revision 1.5  2013/04/10 09:48:48  kuangik
 * Implement do all wrapper for host run sm test
 *
 * Revision 1.4  2013/03/29 09:06:05  leslie
 * Add comment to each function
 *
 * Revision 1.3  2013/03/27 04:49:35  kuangik
 * Code cleanup after adding -Wall
 *
 * Revision 1.17  2013/03/12 11:23:53  leslie
 * Fix submenu flag for show error number message
 *
 * Revision 1.16  2013/03/08 09:34:07  kuangik
 * Clear all warning
 *
 * Revision 1.13  2013/02/18 06:47:11  kody
 * Modify for the port mapping changed according to the new SKUs.
 *
 * Revision 1.12  2013/01/16 00:59:45  leslie
 * Add function to judge whether have second PHY.
 *
 * Revision 1.11  2012/10/24 10:30:36  leslie
 * Combination of the phy 0 and phy 1 to run same test item and add some port mapping message.
 *
 * Revision 1.10  2012/10/08 09:57:04  leslie
 * Fix the eth port to ge port.
 *
 * Revision 1.9  2012/09/05 22:50:51  kody
 * Fix the dump register address parameter.
 *
 * Revision 1.8  2012/08/30 01:27:38  kody
 * Remove the test pass message in utility.
 *
 * Revision 1.7  2012/08/27 06:44:16  evanli
 * phy_addr is decreasing
 *
 * Revision 1.6  2012/08/03 10:16:55  evanli
 * Mapping to latest O2 source code on 20120726
 *
 * Revision 1.4  2012/06/12 02:36:27  leslie
 * Modify key in error:MRVL_88E1340_PHY1_SMI_ADDR
 *
 * Revision 1.3  2012/04/06 06:06:30  kuangik
 * Update for 88E1340 Test item
 *
 * Revision 1.2  2012/02/13 03:31:39  leslie
 * Add function prototype.
 *
 * Revision 1.1  2012/02/10 06:55:00  leslie
 * Add Woodlawn phy 88E1340 test.
 *
 *
 * $Endlog$
 *-------------------------------------------------
 */
