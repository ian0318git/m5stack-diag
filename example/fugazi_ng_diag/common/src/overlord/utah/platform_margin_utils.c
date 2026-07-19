/* $Id: platform_margin_utils.c,v 1.18 2019/09/11 07:33:50 alpeng Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/overlord/utah/platform_margin_utils.c,v $
 *-----------------------------------------------------------------------------
 * platform_margin_utils.c - Overlord Motherboard Margin utilities.
 *
 * Ported from Informers. The original author is Simon Yen.
 *
 * Copyright (c) 2014-2019 by Cisco Systems, Inc.
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
#include "platform_pwr_seq.h"
#include "platform_margin_utils.h"
#include "dash_fpga.h"

/*
 * Functional prototype
 */
/* for utilities submenu. */
int  show_margins(int);
void build_freq_margin_submenu(int);
void build_volt_margin_submenu(int);

static int show_temp_wrapper(int);
static int show_freq_mrgn(void);
static int freq_mrgn(int);

/*
 *  Externs
 */
extern uint32 show_temperature_all(void);
extern int mb_fpga_ios_reload(void);
extern int rs4420b_get_ext_reg(ulong *);
extern void unreset_platform_in_dev(int);
extern void show_fan_info(void);
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
    {"Voltage Margin Utility",(PFT)build_volt_margin_submenu, TRUE, 0,
	(type_t(*)())0,	0, (PFT)build_volt_margin_submenu, TRUE},
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
 *                    Voltage Margin Utilities Submenu
 *******************************************************************************
 */
static submenu_xtable_t volt_mrgn_table[] = {
    {"Set 3.3V voltage margin HI", (PFT)vtg_mrgn, PWR_SEQ_3_3_MRGN_HI, 0,
	(type_t(*)())0,	0, (type_t(*)())0, 0},
    {"Set 3.3V voltage margin NORM", (PFT)vtg_mrgn, PWR_SEQ_3_3_MRGN_NORM, 0,
	(type_t(*)())0,	0, (type_t(*)())0, 0},
    {"Set 3.3V voltage margin LO", (PFT)vtg_mrgn, PWR_SEQ_3_3_MRGN_LO, 0,
	(type_t(*)())0,	0, (type_t(*)())0, 0},
    {"Set 2.5V voltage margin HI", (PFT)vtg_mrgn, PWR_SEQ_2_5_MRGN_HI, 0,
	(type_t(*)())0,	0, (type_t(*)())0, 0},
    {"Set 2.5V voltage margin NORM", (PFT)vtg_mrgn, PWR_SEQ_2_5_MRGN_NORM, 0,
	(type_t(*)())0,	0, (type_t(*)())0, 0},
    {"Set 2.5V voltage margin LO", (PFT)vtg_mrgn, PWR_SEQ_2_5_MRGN_LO, 0,
	(type_t(*)())0,	0, (type_t(*)())0, 0},
    {"Set DDR voltage margin HI", (PFT)vtg_mrgn, PWR_SEQ_DDR_MRGN_HI, 0,
	(type_t(*)())0,	0, (type_t(*)())0, 0},
    {"Set DDR voltage margin NORM", (PFT)vtg_mrgn, PWR_SEQ_DDR_MRGN_NORM, 0,
	(type_t(*)())0,	0, (type_t(*)())0, 0},
    {"Set DDR voltage margin LO", (PFT)vtg_mrgn, PWR_SEQ_DDR_MRGN_LO, 0,
	(type_t(*)())0,	0, (type_t(*)())0, 0},
    {"Set 1.2V voltage margin HI", (PFT)vtg_mrgn, PWR_SEQ_1_2_MRGN_HI, 0,
	(type_t(*)())0,	0, (type_t(*)())0, 0},
    {"Set 1.2V voltage margin NORM", (PFT)vtg_mrgn, PWR_SEQ_1_2_MRGN_NORM, 0,
	(type_t(*)())0,	0, (type_t(*)())0, 0},
    {"Set 1.2V voltage margin LO", (PFT)vtg_mrgn, PWR_SEQ_1_2_MRGN_LO, 0,
	(type_t(*)())0,	0, (type_t(*)())0, 0},
    {"Set 1.0V voltage margin HI", (PFT)vtg_mrgn, PWR_SEQ_1_0_MRGN_HI, 0,
	(type_t(*)())0,	0, (type_t(*)())0, 0},
    {"Set 1.0V voltage margin NORM", (PFT)vtg_mrgn, PWR_SEQ_1_0_MRGN_NORM, 0,
	(type_t(*)())0,	0, (type_t(*)())0, 0},
    {"Set 1.0V voltage margin LO", (PFT)vtg_mrgn, PWR_SEQ_1_0_MRGN_LO, 0,
	(type_t(*)())0,	0, (type_t(*)())0, 0},
};

#define VOLT_MRGN_TABLE_SIZE (sizeof(volt_mrgn_table)/sizeof(submenu_xtable_t))

/*
 * Primary & secondary submenu items (filled in from xtable)
 */
static mitem_t volt_mrgn_primary_items[VOLT_MRGN_TABLE_SIZE + MAX_BASE_ITEMS];
static mitem_t volt_mrgn_secondary_items[VOLT_MRGN_TABLE_SIZE + MAX_BASE_ITEMS];

static struct menuinfo volt_mrgn_diag = {
  "Voltage Margin SubMenu",   /* title */
  0,                            /* title string added by init_empty_menu */
  0,                            /* shows major flags */
  0,                            /* generic prompt */
  0,                            /* size - bumped by add_menu_item */
  volt_mrgn_primary_items,
};

static struct menuinfo *volt_mrgn_menup = &volt_mrgn_diag;

/*******************************************************************************
 *                    Frequency Margin Utilities Submenu
 *******************************************************************************
 */
static submenu_xtable_t freq_mrgn_table[] = {
    {"Display current Frequency Margin", (PFT)show_freq_mrgn, 0, 0,
	(type_t(*)())0,	0, (PFT)show_freq_mrgn, 0},
    {"Set  -3.0%% freq margin", (PFT)freq_mrgn, RS4420B_FREQ_SEL_M3P0, 0,
	(type_t(*)())0,	0, (type_t(*)())0, 0},
    {"Set  -2.5%% freq margin", (PFT)freq_mrgn, RS4420B_FREQ_SEL_M2P5, 0,
	(type_t(*)())0,	0, (type_t(*)())0, 0},
    {"Set  -2.0%% freq margin", (PFT)freq_mrgn, RS4420B_FREQ_SEL_M2P0, 0,
	(type_t(*)())0,	0, (type_t(*)())0, 0},
    {"Set  -1.5%% freq margin", (PFT)freq_mrgn, RS4420B_FREQ_SEL_M1P5, 0,
	(type_t(*)())0,	0, (type_t(*)())0, 0},
    {"Set  -1.0%% freq margin", (PFT)freq_mrgn, RS4420B_FREQ_SEL_M1P0, 0,
	(type_t(*)())0,	0, (type_t(*)())0, 0},
    {"Set  -0.5%% freq margin", (PFT)freq_mrgn, RS4420B_FREQ_SEL_M0P5, 0,
	(type_t(*)())0,	0, (type_t(*)())0, 0},
    {"Set  Normal freq margin", (PFT)freq_mrgn, RS4420B_FREQ_SEL_NORM, 0,
	(type_t(*)())0,	0, (type_t(*)())0, 0},
    {"Set  +0.5%% freq margin", (PFT)freq_mrgn, RS4420B_FREQ_SEL_P0P5, 0,
	(type_t(*)())0,	0, (type_t(*)())0, 0},
    {"Set  +1.0%% freq margin", (PFT)freq_mrgn, RS4420B_FREQ_SEL_P1P0, 0,
	(type_t(*)())0,	0, (type_t(*)())0, 0},
    {"Set  +1.5%% freq margin", (PFT)freq_mrgn, RS4420B_FREQ_SEL_P1P5, 0,
	(type_t(*)())0,	0, (type_t(*)())0, 0},
    {"Set  +2.0%% freq margin", (PFT)freq_mrgn, RS4420B_FREQ_SEL_P2P0, 0,
	(type_t(*)())0,	0, (type_t(*)())0, 0},
    {"Set  +2.5%% freq margin", (PFT)freq_mrgn, RS4420B_FREQ_SEL_P2P5, 0,
	(type_t(*)())0,	0, (type_t(*)())0, 0},
    {"Set  +3.0%% freq margin", (PFT)freq_mrgn, RS4420B_FREQ_SEL_P3P0, 0,
	(type_t(*)())0,	0, (type_t(*)())0, 0},
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
 * Function   : build_volt_margin_submenu
 * Description: Build SubMenu for Voltage margin utility.
 * Inputs     : option - Not used
 * Outputs    : None
 *
 *******************************************************************************
 */
void build_volt_margin_submenu (int option)
{
    build_primary_submenu(volt_mrgn_table, VOLT_MRGN_TABLE_SIZE,
                          "Voltage Margin SubMenu", &volt_mrgn_menup);
    build_secondary_submenu(volt_mrgn_table, VOLT_MRGN_TABLE_SIZE,
                            volt_mrgn_secondary_items);
    menu(&volt_mrgn_diag, volt_mrgn_secondary_items, 0);
}

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
    int ret = FAILED;
    
    ret = show_temperature_all();
    show_fan_info();

    return ret;
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
    int rc, format, option, ia;

    if (mode == MENU_MODE) {
        format = DISPLAY_HCI;     /* Format for Menu driven */
    }else
    if (mode == CLI_MODE) {
        format = DISPLAY_M2M;     /* Format for CLI driven */
    }else {
        prpass(testpass, "%s()",__func__);
        cterr('f',0,"Unknown mode in %s function,"
		    "\nSupport CLI MODE, MENU MODE, or CTERR MODE"
                    "\nbut received mode = %x ", __func__, mode); 
        return (FAILED); 
    }

    for (ia = PWR_SEQ_VP3P3; ia <= PWR_SEQ_VP1P0; ia++) {
        option = ((PWR_SEQ_SHOW_MRGN << PWR_SEQ_SPE_OP_OFF) | (ia << PWR_SEQ_DEVICE_OFF));
        rc |= vtg_mrgn(option, format);     /* display Voltage margin */
    }


    rc |= show_freq_mrgn(); /* 0 for display */

    /* for menu mode nios is disabled when entering menu */
    if (mode == CLI_MODE) {
    }

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
    ulong freq_sel_val = 0;
    char buff[BUFF_SIZE], check_mn_bit;

    /* Clean buffer */
    memset(&buff[0], 0, BUFF_SIZE);

    rc = rs4420b_get_reg(RS4420B_MN_PROG, &check_mn_bit);
    if (rc != PASSED) {
        cterr('f', 0, "Failed to get value from RS4420B Freq. \n");
        return (FAILED);
    }

    /* M/N bit is enable = margining 
     * M/N bit is disable = no margining */
    if (check_mn_bit & RS4420B_DIF_SRC_PCI_MN_EN) {
     
        rc = rs4420b_get_ext_reg(&freq_sel_val);
        if (rc != PASSED) {
            cterr('f', 0, "Failed to get value from RS4420B Freq. \n");
            return (FAILED);
        }

        /* Eliminate byte23 value of the freq. select regiaters 
         * for convenience to identifying the margin offset 
         * since the value of byte23 is always 0x81 for margining.
         */
        freq_sel_val &= ~RS4420B_FREQ_REG23_MSK;

        switch (freq_sel_val) {
        case RS4420B_FREQ_SEL_M3P0:
        case RS4420BD_FREQ_SEL_M3P0:
            sprintf(buff, "-3.0%%");
            break;
        case RS4420B_FREQ_SEL_M2P5:
        case RS4420BD_FREQ_SEL_M2P5:
            sprintf(buff, "-2.5%%");
            break;
        case RS4420B_FREQ_SEL_M2P0:
        case RS4420BD_FREQ_SEL_M2P0:
            sprintf(buff, "-2.0%%");
            break;
        case RS4420B_FREQ_SEL_M1P5:
        case RS4420BD_FREQ_SEL_M1P5:
            sprintf(buff, "-1.5%%");
            break;
        case RS4420B_FREQ_SEL_M1P0:
        case RS4420BD_FREQ_SEL_M1P0:
            sprintf(buff, "-1.0%%");
            break;
        case RS4420B_FREQ_SEL_M0P5:
        case RS4420BD_FREQ_SEL_M0P5:
            sprintf(buff, "-0.5%%");
            break;
        case RS4420B_FREQ_SEL_P0P5:
        case RS4420BD_FREQ_SEL_P0P5:
            sprintf(buff, "+0.5%%");
            break;
        case RS4420B_FREQ_SEL_P1P0:
        case RS4420BD_FREQ_SEL_P1P0:
            sprintf(buff, "+1.0%%");
            break;
        case RS4420B_FREQ_SEL_P1P5:
        case RS4420BD_FREQ_SEL_P1P5:
            sprintf(buff, "+1.5%%");
            break;
        case RS4420B_FREQ_SEL_P2P0:
        case RS4420BD_FREQ_SEL_P2P0:
            sprintf(buff, "+2.0%%");
            break;
        case RS4420B_FREQ_SEL_P2P5:
        case RS4420BD_FREQ_SEL_P2P5:
            sprintf(buff, "+2.5%%");
            break;
        case RS4420B_FREQ_SEL_P3P0:
        case RS4420BD_FREQ_SEL_P3P0:
            sprintf(buff, "+3.0%%");
            break;
        default:
            sprintf(buff, "Unknown %#lx", freq_sel_val);
            break;
        }
    } else {
        /* M/N programming disable, margining disable */
        printf("\nCurrent Frequency Margin: 0.0%%" );
        return (rc);
    }

    printf("\nCurrent Frequency Margin: %s(0x%x%lX)\n", buff,RS4420B_FREQ_MN_BYTE23 ,freq_sel_val);

#if 0
    /* 2. Check Bit 0 of Output Enable Reg.(Byte 1) to see
     *    CPU Spread Spectrum is enabled or not.
     */
    rc = sq420d_get_reg(SQ420D_OUT_EN1, &out_en_reg);
    if (rc != PASSED) {
        cterr('f', 0, "Failed to get value from SQ420D Output Enable Reg"
                      "(off. = %d)", SQ420D_OUT_EN1);
        return (FAILED);
    }

    printf("& Spread Spectrum is %s.\n",
           (out_en_reg & SQ420D_SS_EN) ? "Enabled" : "Disabled");
#endif 

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

    /* 2nd argument on rs4420b_set_freq is N/A on MENU_MODE */
    rc = rs4420b_set_freq(opt, dummy, MENU_MODE);
    if (rc != PASSED) {
        cterr('f', 0, "FAILED to set Frequency Margin.");
    }

    rc = show_freq_mrgn();
    if (rc != PASSED) {
        cterr('f', 0, "FAILED to show Frequency Margin.");
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
    int rc = FAILED; 
    int mrgn_list [][3] = {
        {PWR_SEQ_1_0_MRGN_LO, PWR_SEQ_1_0_MRGN_NORM, PWR_SEQ_1_0_MRGN_HI},
        {PWR_SEQ_1_2_MRGN_LO, PWR_SEQ_1_2_MRGN_NORM, PWR_SEQ_1_2_MRGN_HI},
        {PWR_SEQ_2_5_MRGN_LO, PWR_SEQ_2_5_MRGN_NORM, PWR_SEQ_2_5_MRGN_HI},
        {PWR_SEQ_3_3_MRGN_LO, PWR_SEQ_3_3_MRGN_NORM, PWR_SEQ_3_3_MRGN_HI},
        {PWR_SEQ_DDR_MRGN_LO, PWR_SEQ_DDR_MRGN_NORM, PWR_SEQ_DDR_MRGN_HI}
    };

    if (level == MRGN_LV2)
        printf("Margining to norm-5%%\n");
    else if (level == MRGN_LV3)
        printf("Margining to norm+0%%\n");
    else  /* LV4 */
        printf("Margining to norm+5%%\n");

    rc = vtg_mrgn(mrgn_list[type - MRGN_1ST][level - MRGN_LV2], DISPLAY_M2M);
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
    int rc = FAILED;
    ulong  option;

    if (type == MRGN_1ST) {
        switch(level) {
        case MRGN_LV1:
          option = RS4420B_FREQ_SEL_M1P0;
        break;
        case MRGN_LV2:
          option = RS4420B_FREQ_SEL_M0P5;
        break;
        case MRGN_LV3:
          option = RS4420B_FREQ_SEL_NORM;
        break;
        case MRGN_LV4:
          option = RS4420B_FREQ_SEL_P0P5;
        break;
        case MRGN_LV5:
          option = RS4420B_FREQ_SEL_P1P0;
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
        printf("Margining -1.0%%");
    else if (level == MRGN_LV2)
        printf("Margining -0.5%%");
    else if (level == MRGN_LV3)
        printf("Margining +0.0%%");
    else if (level == MRGN_LV4)
        printf("Margining +0.5%%");
    else  /* LV5 */
        printf("Margining +1.0%%");

#if 0 /* spread */
    if (spread)
        printf(" with spread ENABLE.\n");
    else
        printf(" with spread DISABLE.\n");
#endif 

    rc = rs4420b_set_freq(option, spread, CLI_MODE);
    if (rc != PASSED) {
        cterr('f', 0, "FAILED to set Frequency Margin.");
    }

    return (rc);
}

/******** History ******** 
*---------------------------------------------------
$Log: platform_margin_utils.c,v $
Revision 1.18  2019/09/11 07:33:50  alpeng
CSCvr18160 - adjust NIOS mode setup on Utah

Revision 1.17  2014/06/05 08:53:32  danchung
Support clock chip version D

Revision 1.16  2014/02/19 04:01:35  hroni
in CLI_MODE, explicitly disable nios in show_margins_x()

Revision 1.15  2014/02/11 09:52:36  hroni
enhance voltage margin support for USD

Revision 1.14  2014/01/21 01:36:05  hroni
disable NIOS in margin utility

Revision 1.13  2014/01/20 09:13:15  danchung
Fix frequency margin display issue on Utah

Revision 1.12  2014/01/14 02:44:20  hroni
support NIOS_DIAG_MODE. use NIOS_DIAG_MODE instead of NIOS_NORMAL_MODE

Revision 1.11  2014/01/08 07:56:09  hroni
use enable_nios() instead of reseting NIOS

Revision 1.10  2014/01/06 09:16:05  hroni
fix display temperatures for margin utility

Revision 1.9  2013/11/26 08:40:37  hroni
fix compiler warning

Revision 1.8  2013/11/08 10:09:29  danchung
Modify to display correct frequency margin

Revision 1.7  2013/10/14 12:15:34  danchung
Correct the frequency margin programming table to solve the hang issue

Revision 1.6  2013/10/08 11:14:27  erwu2
enhanced err msg first check-in

Revision 1.5  2013/09/09 06:35:02  hroni
1. use 5% margining on 1.2v and 1.0v, will recover to 9% after HW confirm it is safe
2. add show latest read voltage after and before doing margining
3. turn off byte swap in pwr_write()

Revision 1.4  2013/07/18 17:17:04  mcharon
add -Wal and clean up compile warnings

Revision 1.3  2013/06/18 03:38:30  alpeng
support margining normal

Revision 1.2  2013/06/17 11:14:50  alpeng
support chip 9VRS4420B and freq margin

Revision 1.1  2013/06/14 10:25:48  alpeng
support voltage margin

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
