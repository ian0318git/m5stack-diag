/* $Id: platform_margin_utils.c,v 1.1 2020/01/09 01:02:03 jiajliu Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/overlord/neptune/curie_2RU/platform_margin_utils.c,v $
 *-----------------------------------------------------------------------------
 * platform_margin_utils.c - Overlord Motherboard Margin utilities.
 *
 * Ported from Informers. The original author is Simon Yen.
 *
 * Copyright (c) 2018 by Cisco Systems, Inc.
 * All rights reserved.
 *-----------------------------------------------------------------------------
 */

#include <string.h>
#include "common.h"
#include "types.h"
#include "defs.h"
#include "menu.h"
#include "signals.h"
#include "error.h"
#include "cli_cmd.h"
#include "platform_sys_clk.h"
#include "platform_vtg_mntr.h"
#include "platform_margin_utils.h"
#include "dash_fpga.h" 

/*
 * Functional prototype
 */
/* for utilities submenu. */
int  show_margins(int);

extern ulong typ_size;
extern ulong pppm_ppctg_size;
extern ulong nppm_npctg_size;

extern uchar typ[];
extern uchar pppm_ppctg[];
extern uchar nppm_npctg[];

/*
 *  Externs
 */
extern int show_temp(int, int);

/*
 *  Globals  
 */
#define BUFF_SIZE   10

/*
 * Margin Utilities Menu.
 */
static submenu_xtable_t mrgn_items[] = {
    {"Display Voltage margins",	(PFT)show_margins, 0, 0,
	 (type_t(*)())0,	0, (PFT)show_margins, 0},
    {"Set 3.3V/3.0V to Normal",	(PFT)vtg_mrgn, VTG_MRGN_SET_3_3V_NORM, 0,
	 (type_t(*)())0,	0, (PFT)vtg_mrgn, VTG_MRGN_SET_3_3V_NORM},
    {"Set 3.3V/3.0V Margin High", (PFT)vtg_mrgn, VTG_MRGN_SET_3_3V_HI, 0,
	 (type_t(*)())0,	0, (PFT)vtg_mrgn, VTG_MRGN_SET_3_3V_HI},
    {"Set 3.3V/3.0V Margin Low", (PFT)vtg_mrgn, VTG_MRGN_SET_3_3V_LO, 0,
	 (type_t(*)())0,	0, (PFT)vtg_mrgn, VTG_MRGN_SET_3_3V_LO},
};
#define MARGIN_MENU_TABLE_SIZE (sizeof(mrgn_items) / sizeof(submenu_xtable_t))

/*
 * Primary & secondary submenu items (filled in from xtable)
 */
static mitem_t mrgn_menu_primary_items[MARGIN_MENU_TABLE_SIZE +
		MAX_BASE_ITEMS];
static mitem_t mrgn_menu_secondary_items[MARGIN_MENU_TABLE_SIZE +
		MAX_BASE_ITEMS];

static struct menuinfo margindiag = {
  "Margin Utility Menu",	    /* title */
  0,                                /* title string added by init_empty_menu */
  0,				    /* shows major flags */
  0,                                /* generic prompt */
  0,				    /* size - bumped by add_menu_item */
  mrgn_menu_primary_items,
};
static struct menuinfo *margin_util_menup = &margindiag;


/**********************************************************************
 *
 * function:	build_margin_menu
 *
 * Description:	Build menu for Margin utility.
 *
 * Input:	dummp - Not used.
 *
 * Output:	None.
 *
 **********************************************************************
 */
void
build_margin_menu(int dummy)
{

    testname("Margin");

    build_primary_submenu(mrgn_items, MARGIN_MENU_TABLE_SIZE,
	        "Margin Utility Menu", &margin_util_menup);
    build_secondary_submenu(mrgn_items,  MARGIN_MENU_TABLE_SIZE,
			mrgn_menu_secondary_items);
    menu(&margindiag, mrgn_menu_secondary_items, 0);

}

/**********************************************************************
 *
 * Function:	show_temp_cterr_wrapper
 *
 * Description:	wrapper for show_temp_cterr() to pass DISPLAY_CTERR
 *
 * Input:   None
 *
 * Outputs:	None
 *
 **********************************************************************
 */
void
show_temp_cterr_wrapper(void)
{
    show_temp(FALSE, DISPLAY_CTERR);
}

/**********************************************************************
 *
 * Function:	show_margins
 *
 * Description:	Display voltage and frequency margins, and temperatures.
 *
 * Input:	dummy - ignored.
 *
 * Output:	PASSED/FAILED
 *
 **********************************************************************
 */
int
show_margins(int dummy)
{
    /* calling for menu driven */
    return (show_margins_x(0, MENU_MODE));
}

/**********************************************************************
 *
 * Function:	show_margins_cterr_wrapper
 *
 * Description:	wrapper for show_margins_x() to pass CTERR_MODE
 *
 * Input:     None
 *
 * Output:    None
 *
 **********************************************************************
 */
void
show_margins_cterr_wrapper(void)
{
    show_margins_x(FALSE, CTERR_MODE);
}	

/**********************************************************************
 *
 * Function:	show_margins_x
 *
 * Description:	Display voltage and frequency margins, and temperatures.
 *
 * Input:	dummy - ignored.
 *              mode - MENU_MODE / CLI_MODE /CTERR_MODE
 *
 * Output:	PASSED/FAILED
 *
 **********************************************************************
 */
int
show_margins_x(int dummy, int mode)
{
    int rc;

    rc = vtg_mrgn(VTG_MRGN_GET_3_3V);   /* display 3.3V margin */

    return(rc);
}

/*******************************************************************************
 *
 * Function   : vtg_mrgn_x
 * Description: To set system voltage margin via CLI.
 * Inputs     : type - select type for margin, level - select margin level
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
int vtg_mrgn_x (int type, int level)
{
    int rc = FAILED, option;

    if (type == MRGN_1ST) {
        switch(level) {
        case MRGN_LV2:
          option = VTG_MRGN_SET_3_3V_LO;
        break;
        case MRGN_LV3:
          option = VTG_MRGN_SET_3_3V_NORM;
        break;
        case MRGN_LV4:
          option = VTG_MRGN_SET_3_3V_HI;
        break;
        default:
          printf("%s not applicable level%d type%d\n", __FUNCTION__,  level, type);
          return (FAILED);
        break;
        }
        printf("PSU ");
    } else { /* MRGN_2nd */
        switch(level) {
        /* mfix: no DDR margin */
        default:
          printf("%s not applicable level%d type%d\n", __FUNCTION__,  level, type);
          return (FAILED);
        break;
        }
        printf("DDR ");
     }

    if (level == MRGN_LV2)
        printf("Margining -3%%\n");
    else if (level == MRGN_LV3)
        printf("Margining +0%%\n");
    else  /* LV4 */
        printf("Margining +3%%\n");

    rc = vtg_mrgn(option);
    /* failed message already called on func. vtg_mrgn() */

    return (rc);
}

/*
 *-----------------------------------------------------------------------------
$Log: platform_margin_utils.c,v $
Revision 1.1  2020/01/09 01:02:03  jiajliu
Merge Curie 2RU to main trunk

$Endlog$
 *-----------------------------------------------------------------------------
 */
