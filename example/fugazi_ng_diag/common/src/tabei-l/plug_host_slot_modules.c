/* $Id: plug_host_slot_modules.c,v 1.2 2019/10/17 02:16:27 kehuang2 Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/tabei-l/plug_host_slot_modules.c,v $
 *------------------------------------------------------------------
 *
 * plug_host_slot_modules.c - Host Supported PLUGGABLE Slot Module
 *                            Informations
 *
 * Copyright (c) 2015 - 2019 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#include <stdio.h>
#include <string.h>
#include "types.h"
#include "cookie_4.h"
#include "plug_host_slot_modules.h"
#include "plug_testcard_test.h"
#include "plug_lte_test.h"
#include "plug_lte_telit_test.h"


/* Supported Pluggable Module Lists */
struct plug_module_info plug_host_module_tbl[] = {
    {.name = "Pluggable Test Card",          .id = PLUGGABLE_TEST_CARD,
     .diag = (PFT)plug_testcard_main,       .intf_diag = (PFT)plug_testcard_main, 
     .mod_info_flags = 0},
    {.name = "Pluggable PCIe Test Card",     .id = PLUGGABLE_PCIE_TEST_CARD,
     .diag = (PFT)plug_testcard_main,       .intf_diag = (PFT)plug_testcard_main,
     .mod_info_flags = 0},
    {.name = "Pluggable LTE EM",             .id = PLUGGABLE_LTE_EM,
     .diag = (PFT)plug_lte_main,            .intf_diag = (PFT)plug_lte_main, 
     .mod_info_flags = 0},
    {.name = "Pluggable LTE WP7601",        .id = PLUGGABLE_LTE_WP7601,
     .diag = (PFT)plug_lte_main,            .intf_diag = (PFT)plug_lte_main, 
     .mod_info_flags = 0},
    {.name = "Pluggable LTE WP7603",        .id = PLUGGABLE_LTE_WP7603,
     .diag = (PFT)plug_lte_main,            .intf_diag = (PFT)plug_lte_main, 
     .mod_info_flags = 0},
    {.name = "Pluggable LTE WP7605",        .id = PLUGGABLE_LTE_WP7605,
     .diag = (PFT)plug_lte_main,            .intf_diag = (PFT)plug_lte_main, 
     .mod_info_flags = 0},
    {.name = "Pluggable LTE WP7607",        .id = PLUGGABLE_LTE_WP7607,
     .diag = (PFT)plug_lte_main,            .intf_diag = (PFT)plug_lte_main, 
     .mod_info_flags = 0},
    {.name = "Pluggable LTE WP7608",        .id = PLUGGABLE_LTE_WP7608,
     .diag = (PFT)plug_lte_main,            .intf_diag = (PFT)plug_lte_main, 
     .mod_info_flags = 0},
    {.name = "Pluggable LTE WP7609",        .id = PLUGGABLE_LTE_WP7609,
     .diag = (PFT)plug_lte_main,            .intf_diag = (PFT)plug_lte_main, 
     .mod_info_flags = 0},
    {.name = "Pluggable LTE WP7610",        .id = PLUGGABLE_LTE_WP7610,
     .diag = (PFT)plug_lte_main,            .intf_diag = (PFT)plug_lte_main, 
     .mod_info_flags = 0},
    {.name = "Pluggable LTE Telit LM9x0",   .id = PLUGGABLE_LTE_TELIT_LM9x0,
     .diag = (PFT)plug_lte_telit_main,      .intf_diag = (PFT)plug_lte_telit_main,
     .mod_info_flags = 0},
};


int MAX_MOD_IDS = (sizeof(plug_host_module_tbl) /
                   sizeof(struct plug_module_info));

/*-------------------------------------------------
$Log: plug_host_slot_modules.c,v $
Revision 1.2  2019/10/17 02:16:27  kehuang2
Collapse Tabei-L into main trunk

Revision 1.1.2.4  2019/09/18 09:09:47  kehuang2
Sync PIM with main trunk

Revision 1.1.2.3  2019/07/12 09:33:11  sherliu2
Supported Hyperloop-PIM

Revision 1.1.2.2  2019/02/27 07:14:25  olin2
Support new PIM test card

Revision 1.1.2.1  2018/10/26 08:40:50  kodko
Add support for PIM LTE and test card modules.

$Endlog$
*/
