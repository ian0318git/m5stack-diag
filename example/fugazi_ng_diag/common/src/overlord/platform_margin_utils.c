/* $Id: platform_margin_utils.c,v 1.4 2018/05/18 09:24:51 alpeng Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/overlord/platform_margin_utils.c,v $
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
void build_freq_margin_submenu(int);

static int show_temp_wrapper(int);
static int show_freq_mrgn(void);
static int freq_mrgn(int);

/*
 *  Externs
 */
extern int show_temp(int, int);
extern int mb_fpga_ios_reload(void);
extern uint32 cterr_db_print (char *fmtptr, ...);

/*
 *  Globals  
 */
#define BUFF_SIZE   10

/*
 * Margin Utilities Menu.
 */
static submenu_xtable_t mrgn_items[] = {
    {"Display all margins",	(PFT)show_margins, 0, 0,
	(type_t(*)())0,	0, (PFT)show_margins, 0},
    {"Display temperatures",	(PFT)show_temp_wrapper, TRUE, 0,
	(type_t(*)())0,	0, (PFT)show_temp_wrapper, TRUE},
    {"Frequency Margin Utility",(PFT)build_freq_margin_submenu, TRUE, 0,
	(type_t(*)())0,	0, (PFT)build_freq_margin_submenu, TRUE},
    {"Set 1.8/1.5V to Normal", (PFT)vtg_mrgn, VTG_MRGN_SET_DDR_NORM, 0,
	(type_t(*)())0,	0, (PFT)vtg_mrgn, VTG_MRGN_SET_DDR_NORM},
    {"Set 1.8/1.5V Margin High",	(PFT)vtg_mrgn, VTG_MRGN_SET_DDR_HI, 0,
	(type_t(*)())0,	0, (PFT)vtg_mrgn, VTG_MRGN_SET_DDR_HI},
    {"Set 1.8/1.5V Margin Low",	(PFT)vtg_mrgn, VTG_MRGN_SET_DDR_LO, 0,
	(type_t(*)())0,	0, (PFT)vtg_mrgn, VTG_MRGN_SET_DDR_LO},
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


/*******************************************************************************
 *                    Frequency Margin Utilities Submenu
 *******************************************************************************
 */
static submenu_xtable_t freq_mrgn_table[] = {
    {"Display current Frequency Margin", (PFT)show_freq_mrgn, 0, 0,
	(type_t(*)())0,	0, (PFT)show_freq_mrgn, 0},
    {"Set  -2.47%% freq margin", (PFT)freq_mrgn, SQ420D_FREQ_SEL_247P_M, 0,
	(type_t(*)())0,	0, (PFT)freq_mrgn, SQ420D_FREQ_SEL_247P_M},
    {"Set  -1.17%% freq margin", (PFT)freq_mrgn, SQ420D_FREQ_SEL_117P_M, 0,
	(type_t(*)())0,	0, (PFT)freq_mrgn, SQ420D_FREQ_SEL_117P_M},
    {"Set  +0.00%% freq margin", (PFT)freq_mrgn, SQ420D_FREQ_SEL_NORMAL, 0,
	(type_t(*)())0,	0, (PFT)freq_mrgn, SQ420D_FREQ_SEL_NORMAL},
    {"Set  +1.30%% freq margin", (PFT)freq_mrgn, SQ420D_FREQ_SEL_130P_P, 0,
	(type_t(*)())0,	0, (PFT)freq_mrgn, SQ420D_FREQ_SEL_130P_P},
    {"Set  +2.47%% freq margin", (PFT)freq_mrgn, SQ420D_FREQ_SEL_247P_P, 0,
	(type_t(*)())0,	0, (PFT)freq_mrgn, SQ420D_FREQ_SEL_247P_P},
    {"Set -10.03%% freq margin (for future support)", (PFT)freq_mrgn, 
        SQ420D_FREQ_SEL_1003P_M, 0,              (type_t(*)())0,
        0,                       (PFT)freq_mrgn, SQ420D_FREQ_SEL_1003P_M},
    {"Set  -8.72%% freq margin (for future support)", (PFT)freq_mrgn,
        SQ420D_FREQ_SEL_872P_M,  0,              (type_t(*)())0,
        0,                       (PFT)freq_mrgn, SQ420D_FREQ_SEL_872P_M},
    {"Set  -7.42%% freq margin (for future support)", (PFT)freq_mrgn,
        SQ420D_FREQ_SEL_742P_M,  0,              (type_t(*)())0,
        0,                       (PFT)freq_mrgn, SQ420D_FREQ_SEL_742P_M},
    {"Set  -6.25%% freq margin (for future support)", (PFT)freq_mrgn,
        SQ420D_FREQ_SEL_625P_M,  0,              (type_t(*)())0,
        0,                       (PFT)freq_mrgn, SQ420D_FREQ_SEL_625P_M},
    {"Set  -4.95%% freq margin (for future support)", (PFT)freq_mrgn,
        SQ420D_FREQ_SEL_495P_M,  0,              (type_t(*)())0,
        0,                       (PFT)freq_mrgn, SQ420D_FREQ_SEL_495P_M},
    {"Set  -3.78%% freq margin (for future support)", (PFT)freq_mrgn,
        SQ420D_FREQ_SEL_378P_M,  0,              (type_t(*)())0,
        0,                       (PFT)freq_mrgn, SQ420D_FREQ_SEL_378P_M},
    {"Set  +3.78%% freq margin (for future support)", (PFT)freq_mrgn,
        SQ420D_FREQ_SEL_378P_P,  0,              (type_t(*)())0,
        0,                       (PFT)freq_mrgn, SQ420D_FREQ_SEL_378P_P},
    {"Set  +5.08%% freq margin (for future support)", (PFT)freq_mrgn,
        SQ420D_FREQ_SEL_508P_P,  0,              (type_t(*)())0,
        0,                       (PFT)freq_mrgn, SQ420D_FREQ_SEL_508P_P},
    {"Set  +6.25%% freq margin (for future support)", (PFT)freq_mrgn,
        SQ420D_FREQ_SEL_625P_P,  0,              (type_t(*)())0,
        0,                       (PFT)freq_mrgn, SQ420D_FREQ_SEL_625P_P},
    {"Set  +7.55%% freq margin (for future support)", (PFT)freq_mrgn,
        SQ420D_FREQ_SEL_755P_P,  0,              (type_t(*)())0,
        0,                       (PFT)freq_mrgn, SQ420D_FREQ_SEL_755P_P},
    {"Set +10.03%% freq margin (for future support)", (PFT)freq_mrgn,
        SQ420D_FREQ_SEL_1003P_P, 0,              (type_t(*)())0,
        0,                       (PFT)freq_mrgn, SQ420D_FREQ_SEL_1003P_P},
};

#define FREQ_MRGN_TABLE_SIZE (sizeof(freq_mrgn_table)/sizeof(submenu_xtable_t))

/*
 * Primary & secondary submenu items (filled in from xtable)
 */
static mitem_t freq_mrgn_primary_items[FREQ_MRGN_TABLE_SIZE + MAX_BASE_ITEMS];
static mitem_t freq_mrgn_secondary_items[FREQ_MRGN_TABLE_SIZE + MAX_BASE_ITEMS];

static struct menuinfo freq_mrgn_diag = {
  "Frequency Margin SubMenu",   /* title */
  0,                            /* title string added by init_empty_menu */
  0,                            /* shows major flags */
  0,                            /* generic prompt */
  0,                            /* size - bumped by add_menu_item */
  freq_mrgn_primary_items,
};

static struct menuinfo *freq_mrgn_menup = &freq_mrgn_diag;


/*******************************************************************************
 *
 * Function   : build_freq_margin_submenu
 * Description:	Build SubMenu for Frequency Margin utility.
 * Inputs     : option - Not used
 * Outputs    : None
 *
 *******************************************************************************
 */
void build_freq_margin_submenu (int option)
{
    build_primary_submenu(freq_mrgn_table, FREQ_MRGN_TABLE_SIZE,
			  "Frequency Margin SubMenu", &freq_mrgn_menup);
    build_secondary_submenu(freq_mrgn_table, FREQ_MRGN_TABLE_SIZE,
			    freq_mrgn_secondary_items);
    menu(&freq_mrgn_diag, freq_mrgn_secondary_items, 0);
}


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
 * Function:	show_temp_wrapper
 *
 * Description:	Display temperature wrapper for the submenu
 *
 * Inputs:	err_log - TRUE to cterr. FALSE to printf
 *
 * Outputs:	PASSED/FAILED.
 *
 **********************************************************************
 */
static int
show_temp_wrapper(int err_log)
{
    return(show_temp(err_log, DISPLAY_HCI));
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
    int rc, format;

    if (mode == MENU_MODE) {
        format = DISPLAY_HCI;     /* Format for Menu driven */
    }else
    if (mode == CLI_MODE) {
        format = DISPLAY_M2M;     /* Format for CLI driven */
    }else
    if (mode == CTERR_MODE) {
        format = DISPLAY_CTERR;     /* Format for CTERR */
    }else {
        prpass(testpass, "%s()",__func__);
        cterr('f',0,"Unknown mode in %s function,"
		    "\nSupport CLI MODE, MENU MODE, or CTERR MODE"
                    "\nbut received mode = %x ", __func__, mode); 
        return (FAILED); 
    }

    rc = vtg_mrgn(VTG_MRGN_GET_DDR, format);     /* display 1.8/1.5V DDR margin */
    rc |= vtg_mrgn(VTG_MRGN_GET_3_3V, format);   /* display 3.3/3.0V margin */

    rc |= show_freq_mrgn(); /* 0 for display */

    return(rc);
}


/*******************************************************************************
 *
 * Function   : show_freq_mrgn
 * Description:	To display current system frequency margin info.
 * Inputs     : opt
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
static int show_freq_mrgn (void)
{
    int rc = FAILED;
    uchar freq_sel_val = 0;
    char buff[BUFF_SIZE], freq_sel_reg = 0, out_en_reg = 0;

    /* Clean buffer */
    memset(&buff[0], 0, BUFF_SIZE);

    /* For Overlord, to get Freq. Margin related info from SQ420D, we need:
     * 1. Read Bit[0:3] of Freq. Select Reg.(Byte 6) to get
     *    the current freq. margin setting.
     */
    rc = sq420d_get_reg(SQ420D_CPU_FQ_SEL, &freq_sel_reg);
    if (rc != PASSED) {
        cterr('f', 0, "Failed to get value from SQ420D Freq. Seletct Reg"
                      "(off. = %d)", SQ420D_CPU_FQ_SEL);
        return (FAILED);
    }

    freq_sel_val = (freq_sel_reg & SQ420D_CPU_FS_MASK);

    switch (freq_sel_val) {
    case SQ420D_FREQ_SEL_1003P_M:
        sprintf(buff, "-10.03%%");
        break;
    case SQ420D_FREQ_SEL_872P_M:
        sprintf(buff, "-8.72%%");
        break;
    case SQ420D_FREQ_SEL_742P_M:
        sprintf(buff, "-7.42%%");
        break;
    case SQ420D_FREQ_SEL_625P_M:
        sprintf(buff, "-6.25%%");
        break;
    case SQ420D_FREQ_SEL_495P_M:
        sprintf(buff, "-4.95%%");
        break;
    case SQ420D_FREQ_SEL_378P_M:
        sprintf(buff, "-3.78%%");
        break;
    case SQ420D_FREQ_SEL_247P_M:
        sprintf(buff, "-2.47%%");
        break;
    case SQ420D_FREQ_SEL_117P_M:
        sprintf(buff, "-1.17%%");
        break;
    case SQ420D_FREQ_SEL_NORMAL:
        sprintf(buff, "Normal");
        break;
    case SQ420D_FREQ_SEL_130P_P:
        sprintf(buff, "+1.30%%");
        break;
    case SQ420D_FREQ_SEL_247P_P:
        sprintf(buff, "+2.47%%");
        break;
    case SQ420D_FREQ_SEL_378P_P:
        sprintf(buff, "+3.78%%");
        break;
    case SQ420D_FREQ_SEL_508P_P:
        sprintf(buff, "+5.08%%");
        break;
    case SQ420D_FREQ_SEL_625P_P:
        sprintf(buff, "+6.25%%");
        break;
    case SQ420D_FREQ_SEL_755P_P:
        sprintf(buff, "+7.55%%");
        break;
    case SQ420D_FREQ_SEL_1003P_P:
        sprintf(buff, "+10.03%%");
        break;
    default:
        sprintf(buff, "Unknown");
        break;
    }
    cterr_db_print("\n");
    cterr_db_print("Current Frequency Margin: %s(%#x)\n", buff, freq_sel_val);

    /* 2. Check Bit 0 of Output Enable Reg.(Byte 1) to see
     *    CPU Spread Spectrum is enabled or not.
     */
    rc = sq420d_get_reg(SQ420D_OUT_EN1, &out_en_reg);
    if (rc != PASSED) {
        cterr('f', 0, "Failed to get value from SQ420D Output Enable Reg"
                      "(off. = %d)", SQ420D_OUT_EN1);
        return (FAILED);
    }

    cterr_db_print("& Spread Spectrum is %s.\n",
           (out_en_reg & SQ420D_SS_EN) ? "Enabled" : "Disabled");

    return (rc);
}


/*******************************************************************************
 *
 * Function   : freq_mrgn
 * Description:	To set system frequency margin.
 * Inputs     : opt - the desired frequency value
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
static int freq_mrgn (int opt)
{
    int rc = FAILED, dummy = 0;

    /* 2nd argument on sq420d_set_freq is N/A on MENU_MODE */
    rc = sq420d_set_freq(opt, dummy, MENU_MODE);
    if (rc != PASSED) {
        cterr('f', 0, "FAILED to set Frequency Margin.");
    }

    return (rc);
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
        case MRGN_LV2:
          option = VTG_MRGN_SET_DDR_LO;
        break;
        case MRGN_LV3:
          option = VTG_MRGN_SET_DDR_NORM;
        break;
        case MRGN_LV4:
          option = VTG_MRGN_SET_DDR_HI;
        break;
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

    rc = vtg_mrgn(option, DISPLAY_M2M);
    /* failed message already called on func. vtg_mrgn() */

    return (rc);
}

/*******************************************************************************
 *
 * Function   : freq_mrgn_x
 * Description: To set system frequency margin via CLI.
 * Inputs     : type - select type for margin, level - select margin level
 *              spread - enable spread or not 
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
int freq_mrgn_x (int type, int level, boolean spread)
{
    int rc = FAILED, option;

    if (type == MRGN_1ST) {
        switch(level) {
        case MRGN_LV1:
          option = SQ420D_FREQ_SEL_247P_M;
        break;
        case MRGN_LV2:
          option = SQ420D_FREQ_SEL_117P_M;
        break;
        case MRGN_LV3:
          option = SQ420D_FREQ_SEL_NORMAL;
        break;
        case MRGN_LV4:
          option = SQ420D_FREQ_SEL_130P_P;
        break;
        case MRGN_LV5:
          option = SQ420D_FREQ_SEL_247P_P;
        break;
        default:
          printf("not applicable level%d type%d\n", level, type);
          return (FAILED);
        break;
        }
        printf("CLK ");
    } else { 
        printf("not applicable level%d type%d\n", level, type);
        return (FAILED);
    }

    if (level == MRGN_LV1)
        printf("Margining -2.47%%");
    else if (level == MRGN_LV2)
        printf("Margining -1.17%%");
    else if (level == MRGN_LV3)
        printf("Margining +0.00%%");
    else if (level == MRGN_LV4)
        printf("Margining +1.30%%");
    else  /* LV5 */
        printf("Margining +2.47%%");

    if (spread)
        printf(" with spread ENABLE.\n");
    else
        printf(" with spread DISABLE.\n");

    rc = sq420d_set_freq(option, spread, CLI_MODE);
    if (rc != PASSED) {
        cterr('f', 0, "FAILED to set Frequency Margin.");
    }

    return (rc);
}

/******** History ******** 
*---------------------------------------------------
$Log: platform_margin_utils.c,v $
Revision 1.4  2018/05/18 09:24:51  alpeng
 Neptune merge to trunk with tag <neptune-branch-0518>

Revision 1.3.54.4  2017/03/29 02:06:09  leschen
Remove unnecessary debug messages.

Revision 1.3.54.3  2017/03/13 07:49:18  leschen
Support Triton system.

Revision 1.3.54.2  2017/01/12 02:26:47  leschen
Temporary skip display margin info for extended error report.

Revision 1.3.54.1  2016/12/28 08:57:18  leschen
Temporarily skip margin utility, will ask hw for latest FPGA spec.

Revision 1.3  2013/11/26 08:40:36  hroni
fix compiler warning

Revision 1.2  2013/10/08 11:14:27  erwu2
enhanced err msg first check-in

Revision 1.1  2013/05/09 05:42:37  alpeng
moving overlord common code from x86

Revision 1.8  2013/01/31 10:48:46  alpeng
supported CLI cmds for voltage and freq margin

Revision 1.7  2012/11/07 14:01:01  alpeng
fixed compiler warning

Revision 1.6  2012/11/07 10:58:16  alpeng
remove useless file and clean up code

Revision 1.5  2012/05/30 16:45:03  palin2
Clean up compile warnings.

Revision 1.4  2012/04/10 01:55:04  palin2
Update Freq. margin option iterms' order and display.

Revision 1.3  2012/03/28 00:38:23  mcharon
remove forward slash from second line

Revision 1.2  2012/03/25 03:58:33  palin2
Clean up Overlord Intel side (x86) compile warnings.

Revision 1.1.1.1  2012/03/23 23:02:09  ptong
Initial archive of ng_diag module


$Endlog$
*/
