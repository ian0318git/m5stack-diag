/* $Id: diag_fru_util.c,v 1.2 2016/04/20 11:25:31 benchen2 Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/tachi_bmc/diag_fru_util.c,v $
 * Copyright (c) 2016 by Cisco Systems, Inc.
 * All rights reserved.
 */
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ioctl.h> 
#include "common.h"
#include "types.h"
#include "error.h"
#include "queryflags.h"
#include "slot.h"
#include "menu.h"
#include "diag_fru_util.h"

int diag_fru_util(void);
static void diag_platform_fru_util(int);
extern int diag_ipmi_sprom_ui (int platform, int type, int slot, int cmd);


static submenu_xtable_t fru_util_submenu_table[] = {
    {"FRU Set", (type_t(*)())diag_platform_fru_util, FRU_SET,
	0, (type_t(*)())0, 0, (type_t(*)())0,   0},
    {"FRU Show", (type_t(*)())diag_platform_fru_util, FRU_SHOW,
	0, (type_t(*)())0, 0, (type_t(*)())0,   0},
    {"FRU Invalidate", (type_t(*)())diag_platform_fru_util, FRU_INVALIDTE,
	0, (type_t(*)())0, 0, (type_t(*)())0,   0},
    {"FRU Zero", (type_t(*)())diag_platform_fru_util, FRU_ZERO,
	0, (type_t(*)())0, 0, (type_t(*)())0,   0},    
};

#define FRU_UTIL_SUBMENU_TABLE_SIZE (sizeof(fru_util_submenu_table) / \
				       sizeof(submenu_xtable_t))

/*
 * Primary & secondary submenu items (filled in from xtable)
 */
static mitem_t fru_util_primary_items[FRU_UTIL_SUBMENU_TABLE_SIZE +
				       MAX_BASE_ITEMS];
static mitem_t fru_util_secondary_items[FRU_UTIL_SUBMENU_TABLE_SIZE +
					 MAX_BASE_ITEMS];

menuinfo_t fru_util_subtest_menu = {
    "%s Subtest Menu",
    0,                                /* mtparam added by init_empty_menu */
    (PFT)show_endnote,                /* notes missing WICs in combos */
    0,                                /* use generic prompt */
    0,                                /* size (bumped by add_menu_item() */
    fru_util_primary_items,
};
menuinfo_t *fru_util_submenup = &fru_util_subtest_menu;


int diag_fru_util (void)
{
    build_primary_submenu(fru_util_submenu_table,
			              FRU_UTIL_SUBMENU_TABLE_SIZE,
                          "FRU Utility", &fru_util_submenup);
    build_secondary_submenu(fru_util_submenu_table,
                            FRU_UTIL_SUBMENU_TABLE_SIZE,
                            fru_util_secondary_items);    
                            
    menu(fru_util_submenup, fru_util_secondary_items, '\0');
    return (PASSED);
}

static void diag_platform_fru_util(int arg)
{
	switch (arg) {
    case FRU_SET:
        /*sprom set bmc*/
        diag_ipmi_sprom_ui(FRU_PLATFROM, FRU_TYPE, FRU_SLOT, FRU_SET);
        break;
    case FRU_ZERO:
        /*sprom zero bmc*/
        diag_ipmi_sprom_ui(FRU_PLATFROM, FRU_TYPE, FRU_SLOT, FRU_ZERO);
        break;
    case FRU_INVALIDTE:
        /*sprom invalidate bmc*/
        diag_ipmi_sprom_ui(FRU_PLATFROM, FRU_TYPE, FRU_SLOT, FRU_INVALIDTE);
        break;
    case FRU_SHOW:
        /*sprom show bmc*/
        diag_ipmi_sprom_ui(FRU_PLATFROM, FRU_TYPE, FRU_SLOT, FRU_SHOW);
        break;
	default:
	    break;	
	}
}

