/* $Id: plug_host_slot_modules.c,v 1.6 2021/06/02 02:56:25 alpeng Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/tsn/plug_host_slot_modules.c,v $
 *------------------------------------------------------------------
 *
 * plug_host_slot_modules.c - Host Supported PLUGGABLE Slot Module
 *                            Informations
 *
 * Copyright (c) 2015 ~ 2019 by Cisco Systems, Inc.
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
#include "platform_plug_serial_test.h"
#include "plug_lte_telit_test.h"
#include "plug_NR_5G_telit_test.h"


/* Supported Pluggable Module Lists */
struct plug_module_info plug_host_module_tbl[] = {
    {.name = "Pluggable Test Card",        .id = PLUGGABLE_TEST_CARD,
     .diag = (PFT)plug_testcard_main,      .intf_diag = (PFT)plug_testcard_main, 
     .mod_info_flags = 0},
    {.name = "Pluggable LTE EM",           .id = PLUGGABLE_LTE_EM,
     .diag = (PFT)plug_lte_main,           .intf_diag = (PFT)plug_lte_main, 
     .mod_info_flags = 0},
    {.name = "Pluggable LTE WP7601",       .id = PLUGGABLE_LTE_WP7601,
     .diag = (PFT)plug_lte_main,           .intf_diag = (PFT)plug_lte_main, 
     .mod_info_flags = 0},
    {.name = "Pluggable LTE WP7603",       .id = PLUGGABLE_LTE_WP7603,
     .diag = (PFT)plug_lte_main,           .intf_diag = (PFT)plug_lte_main, 
     .mod_info_flags = 0},
    {.name = "Pluggable LTE WP7605",       .id = PLUGGABLE_LTE_WP7605,
     .diag = (PFT)plug_lte_main,           .intf_diag = (PFT)plug_lte_main, 
     .mod_info_flags = 0},
    {.name = "Pluggable LTE WP7607",       .id = PLUGGABLE_LTE_WP7607,
     .diag = (PFT)plug_lte_main,           .intf_diag = (PFT)plug_lte_main, 
     .mod_info_flags = 0},
    {.name = "Pluggable LTE WP7608",       .id = PLUGGABLE_LTE_WP7608,
     .diag = (PFT)plug_lte_main,           .intf_diag = (PFT)plug_lte_main, 
     .mod_info_flags = 0},
    {.name = "Pluggable LTE WP7609",       .id = PLUGGABLE_LTE_WP7609,
     .diag = (PFT)plug_lte_main,           .intf_diag = (PFT)plug_lte_main, 
     .mod_info_flags = 0},
    {.name = "Pluggable LTE WP7610",       .id = PLUGGABLE_LTE_WP7610,
     .diag = (PFT)plug_lte_main,           .intf_diag = (PFT)plug_lte_main, 
     .mod_info_flags = 0},
    {.name = "Pluggable Serial",           .id = PLUGGABLE_SERIAL,
     .diag = (PFT)plug_serial_main,        .intf_diag = (PFT)plug_serial_main,
     .mod_info_flags = 0},
    {.name = "Pluggable LTE Telit LM9x0",  .id = PLUGGABLE_LTE_TELIT_LM9x0,
     .diag = (PFT)plug_lte_telit_main,     .intf_diag = (PFT)plug_lte_telit_main,
     .mod_info_flags = 0},
    {.name = "Pluggable NR_5G Telit FN980", .id = PLUGGABLE_NR_5G_TELIT_FN980,
     .diag = (PFT)plug_NR_5g_telit_main,    .intf_diag = (PFT)plug_NR_5g_telit_main,
     .mod_info_flags = 0},
};


int MAX_MOD_IDS = (sizeof(plug_host_module_tbl) /
                   sizeof(struct plug_module_info));

/*-------------------------------------------------
$Log: plug_host_slot_modules.c,v $
Revision 1.6  2021/06/02 02:56:25  alpeng
merge sears into trunk

Revision 1.5.36.1  2021/04/26 23:02:10  tshanmug
Sear PIM support for TSN

Revision 1.5  2019/08/15 09:27:51  shjung
Supported WP7610 PIM

Revision 1.4  2019/06/14 05:48:11  shjung
Supported WP7605 modules

Revision 1.3  2019/05/14 09:18:32  sherliu2
Support hyperloop

Revision 1.2.4.1  2018/12/13 19:19:27  shjung
Supported Hyperloop PIM on Star platform

Revision 1.2  2018/11/23 08:49:53  hondwang
Re-instruct pluggable common code with CDETs CSCvn17216

Revision 1.1.2.2  2018/10/22 08:47:59  hondwang
Add IO interface testing for plug testcard

Revision 1.1.2.1  2018/10/15 06:47:17  hondwang
pluggable common code re-instruct add and remove files



*/
