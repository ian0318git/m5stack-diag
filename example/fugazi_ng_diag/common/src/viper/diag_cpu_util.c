 /* $Id: diag_cpu_util.c,v 1.2 2018/08/06 02:31:50 harrchan Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/viper/diag_cpu_util.c,v $
 *-----------------------------------------------------------------------------
 * diag_cpu_util.c - CPU relative function
 *
 *
 * Copyright (c) 2018 by Cisco Systems, Inc.
 * All rights reserved.
 *-----------------------------------------------------------------------------
 */
#include <stdio.h>
#include <string.h>
#include "proto.h"
#include "queryflags.h"
#include "common.h"
#include "menu.h"
#include "error.h"
#include "nvmonvars.h"
#include "plat_defs.h"
#include "diag_fpga.h"
#include "platform_fru.h"
#include "dnv_gpio_lib.h"
#include "dnv_eth_lib.h"




/*******************************************************************************
 *                             Functions Declaration                           *
 *******************************************************************************
 */


/*******************************************************************************
 *                               Global Variable                               *
 *******************************************************************************
 */


/*
 * DNV Interface Utilities Submenu
 */
static submenu_xtable_t dnv_interface_utils_table[] = {
    {"MDIO/MDC Access util",  (PFT)dnv_phy_reg_access, 0,
        0, (type_t(*)())0, 0, (PFT)dnv_phy_reg_access,  0},
    {"Read Intel GPIO util",  (PFT)dnv_gpio_read_util, 0,
        0, (type_t(*)())0, 0, (PFT)dnv_gpio_read_util,  0},
    {"Write Intel GPIO util", (PFT)dnv_gpio_write_util, 0,
        0, (type_t(*)())0, 0, (PFT)dnv_gpio_write_util,  0},
    {"Read SBReg util",       (PFT)dnv_p2sb_read_util, 0,
        0, (type_t(*)())0, 0, (PFT)dnv_p2sb_read_util,  0},
    {"Write SBReg util",      (PFT)dnv_p2sb_write_util, 0,
        0, (type_t(*)())0, 0, (PFT)dnv_p2sb_write_util,  0},
};

#define DNV_INT_UTILS_TABLE_SIZE (sizeof(dnv_interface_utils_table) / sizeof(submenu_xtable_t))

/*
 * Primary & secondary submenu items (filled in from xtable)
 */
static mitem_t dnv_int_utils_primary_items[DNV_INT_UTILS_TABLE_SIZE + MAX_BASE_ITEMS];
static mitem_t dnv_int_utils_secondary_items[DNV_INT_UTILS_TABLE_SIZE + MAX_BASE_ITEMS];

static struct menuinfo dnv_interface_utils_diag = {
    "Denverton Interface Utilities Submenu",    /* title */
    0,                          /* title string added by init_empty_menu */
    (PFT)menu_show_dflags,      /* shows major flags */
    0,                          /* generic prompt */
    0,                          /* size -- bumped by add_menu_item() */
    dnv_int_utils_primary_items,
};

static struct menuinfo *dnv_interface_utils_diagp = &dnv_interface_utils_diag;




/*******************************************************************************
 *                                    Functions                                *
 *******************************************************************************
 */


/*******************************************************************************
 *
 * Function   : build_dnv_utils_menu
 * Description: To build Denverton interface utilities submenu
 * Inputs     : None
 * Outputs    : None
 *
 *******************************************************************************
 */
void build_dnv_utils_menu (void) {

    build_primary_submenu(dnv_interface_utils_table, DNV_INT_UTILS_TABLE_SIZE,
                          "Denverton Interface Utilities Submenu", 
                          &dnv_interface_utils_diagp);
    build_secondary_submenu(dnv_interface_utils_table, DNV_INT_UTILS_TABLE_SIZE,
                            dnv_int_utils_secondary_items);
    menu(&dnv_interface_utils_diag, dnv_int_utils_secondary_items, 0);
}






/* end of file */


/******** History ********
*---------------------------------------------------
$Log: diag_cpu_util.c,v $
Revision 1.2  2018/08/06 02:31:50  harrchan
Merge viper E2E to the main trunk (CSCvk28469)

Revision 1.1.2.2  2018/06/27 06:18:17  harrchan
Modify menu name DNV to Denverton and remove menu item Display PCIE 00:1f.1

Revision 1.1.2.1  2018/02/27 08:06:32  harrchan
Initial viper application code base






$Endlog$
*/
