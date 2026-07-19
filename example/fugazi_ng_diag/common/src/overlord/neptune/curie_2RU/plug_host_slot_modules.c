/* $Id: plug_host_slot_modules.c,v 1.2 2021/06/02 02:56:25 alpeng Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/overlord/neptune/curie_2RU/plug_host_slot_modules.c,v $
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
#include "plug_NR_5G_telit_test.h"

#ifndef PLUGGABLE_PCIE_TEST_CARD_OLD
#define PLUGGABLE_PCIE_TEST_CARD_OLD 0x1235
#endif

/* Supported Pluggable Module Lists */
struct plug_module_info plug_host_module_tbl[] = {
    {.name = "Pluggable Test Card",          .id = PLUGGABLE_TEST_CARD,
     .diag = (PFT)plug_testcard_main,       .intf_diag = (PFT)plug_testcard_main, 
     .mod_info_flags = 0},
    {.name = "Pluggable PCIe Test Card",     .id = PLUGGABLE_PCIE_TEST_CARD,
     .diag = (PFT)plug_testcard_main,       .intf_diag = (PFT)plug_testcard_main, 
     .mod_info_flags = 0},
    {.name = "Pluggable PCIe Test Card Old", .id = PLUGGABLE_PCIE_TEST_CARD_OLD,
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
    {.name = "Pluggable LTE WP7607",        .id = PLUGGABLE_LTE_WP7607,
     .diag = (PFT)plug_lte_main,            .intf_diag = (PFT)plug_lte_main, 
     .mod_info_flags = 0},
    {.name = "Pluggable LTE WP7608",        .id = PLUGGABLE_LTE_WP7608,
     .diag = (PFT)plug_lte_main,            .intf_diag = (PFT)plug_lte_main, 
     .mod_info_flags = 0},
    {.name = "Pluggable LTE WP7609",        .id = PLUGGABLE_LTE_WP7609,
     .diag = (PFT)plug_lte_main,            .intf_diag = (PFT)plug_lte_main, 
     .mod_info_flags = 0},
    {.name= "Pluggable LTE Telit LM9x0",    .id = PLUGGABLE_LTE_TELIT_LM9x0,
     .diag = (PFT)plug_lte_telit_main,      .intf_diag = (PFT)plug_lte_telit_main,
     .mod_info_flags = 0},
   {.name = "Pluggable NR_5G Telit FN980", .id = PLUGGABLE_NR_5G_TELIT_FN980,
     .diag = (PFT)plug_NR_5g_telit_main,    .intf_diag = (PFT)plug_NR_5g_telit_main,
     .mod_info_flags = 0},
};


int MAX_MOD_IDS = (sizeof(plug_host_module_tbl) /
                   sizeof(struct plug_module_info));

/*
 *-----------------------------------------------------------------------------
$Log: plug_host_slot_modules.c,v $
Revision 1.2  2021/06/02 02:56:25  alpeng
merge sears into trunk

Revision 1.1.18.1  2020/09/05 00:12:07  tshanmug
Adding Sear PIM support

Revision 1.1  2020/01/09 01:02:05  jiajliu
Merge Curie 2RU to main trunk

$Endlog$
 *-----------------------------------------------------------------------------
 */
