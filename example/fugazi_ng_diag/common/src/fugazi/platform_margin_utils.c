/* $Id: platform_margin_utils.c,v 1.2 2021/06/02 08:22:35 iachang Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/fugazi/platform_margin_utils.c,v $
 *-----------------------------------------------------------------------------
 * platform_margin_utils.c - Overlord Motherboard Margin utilities.
 *
 * Ported from Informers. The original author is Simon Yen.
 *
 * Copyright (c) 2019 - 2020 by Cisco Systems, Inc.
 * All rights reserved.
 *-----------------------------------------------------------------------------
 */

#include <string.h>
#include <linux/types.h>
#include "common.h"
#include "types.h"
#include "defs.h"
#include "menu.h"
#include "signals.h"
#include "error.h"
#include "platform_vtg_mntr.h"
#include "dash_fpga.h" 
#include "platform_i2c.h"
#include "i2c_address.h"
#include "queryflags.h"
#include "platform_synce_pll_utils.h"


/*
 * Functional prototype
 */
/* for utilities submenu. */
void margin_init(int);
static int  idt8a3_freq_margin_f(void);
static int  idt8a3_show_freq_f(void);
void build_vtg_margin_menu(int);
void build_frq_margin_menu(int);

extern long bcm82757_reset(int);
extern void bcm54194_reset(void);


extern ulong typ_size;
extern ulong pppm_ppctg_size;
extern ulong nppm_npctg_size;

extern uchar typ[];
extern uchar pppm_ppctg[];
extern uchar nppm_npctg[];

/*
 *  Globals  
 */
#define BUFF_SIZE   10

/*
 * Margin Utilities Menu.
 */
static submenu_xtable_t mrgn_items[] = {
    {"Voltage margin utility",  (PFT)build_vtg_margin_menu, 0, 0,
        (type_t(*)())0,	0, (PFT)build_vtg_margin_menu, 0},
    {"Frequency margin utility",	(PFT)build_frq_margin_menu, 0, 0,
        (type_t(*)())0,	0, (PFT)build_frq_margin_menu, 0},
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
  0,                            /* title string added by init_empty_menu */
  0,                            /* shows major flags */
  0,                            /* generic prompt */
  0,                            /* size - bumped by add_menu_item */
  mrgn_menu_primary_items,
};
static struct menuinfo *margin_util_menup = &margindiag;

/*
 * Voltage Margin Utilities Menu.
 */
static submenu_xtable_t vtg_mrgn_items[] = {
    {"Margin Value Init",  (PFT)margin_init, 1, 0,
     (type_t(*)())0,	0, (PFT)margin_init, 1},
    {"Set 3.3V/3.0V to Normal",	(PFT)vtg_mrgn, VTG_MRGN_SET_3_3V_NORM, 0,
     (type_t(*)())0,	0, (PFT)vtg_mrgn, VTG_MRGN_SET_3_3V_NORM},
    {"Set 3.3V/3.0V Margin High", (PFT)vtg_mrgn, VTG_MRGN_SET_3_3V_HI, 0,
     (type_t(*)())0,	0, (PFT)vtg_mrgn, VTG_MRGN_SET_3_3V_HI},
    {"Set 3.3V/3.0V Margin Low", (PFT)vtg_mrgn, VTG_MRGN_SET_3_3V_LO, 0,
     (type_t(*)())0,	0, (PFT)vtg_mrgn, VTG_MRGN_SET_3_3V_LO},
    {"Show Current Voltage", (PFT)show_mrgn, 0, 0,
     (type_t(*)())0,	0, (PFT)show_mrgn, 0},
};
#define VTG_MARGIN_MENU_TABLE_SIZE (sizeof(vtg_mrgn_items) / sizeof(submenu_xtable_t))

/*
 * Primary & secondary submenu items (filled in from xtable)
 */
static mitem_t vtg_mrgn_menu_primary_items[VTG_MARGIN_MENU_TABLE_SIZE +
                                           MAX_BASE_ITEMS];
static mitem_t vtg_mrgn_menu_secondary_items[VTG_MARGIN_MENU_TABLE_SIZE +
                                           MAX_BASE_ITEMS];

static struct menuinfo vtgmargindiag = {
  "Voltage Margin Utility Menu",    /* title */
  0,                                /* title string added by init_empty_menu */
  0,                                /* shows major flags */
  0,                                /* generic prompt */
  0,                                /* size - bumped by add_menu_item */
  vtg_mrgn_menu_primary_items,
};
static struct menuinfo *vtg_margin_util_menup = &vtgmargindiag;

/*
 * Frequency Margin Utilities Menu.
 */
static submenu_xtable_t frq_mrgn_items[] = {
    {"Synce PLL Frequency Margin",	(PFT)idt8a3_freq_margin_f,	FALSE, 0,
     (type_t(*)())0,	0, (PFT)0, 	0},
    {"Show current Synce PLL Frequency", (PFT)idt8a3_show_freq_f, FALSE, 0,
     (type_t(*)())0,	0, (PFT)0, 	0},
};
#define FRQ_MARGIN_MENU_TABLE_SIZE (sizeof(frq_mrgn_items) / sizeof(submenu_xtable_t))

/*
 * Primary & secondary submenu items (filled in from xtable)
 */
static mitem_t frq_mrgn_menu_primary_items[FRQ_MARGIN_MENU_TABLE_SIZE +
                                           MAX_BASE_ITEMS];
static mitem_t frq_mrgn_menu_secondary_items[FRQ_MARGIN_MENU_TABLE_SIZE +
                                           MAX_BASE_ITEMS];

static struct menuinfo frqmargindiag = {
  "Frequency Margin Utility Menu",    /* title */
  0,                                  /* title string added by init_empty_menu */
  0,                                  /* shows major flags */
  0,                                  /* generic prompt */
  0,                                  /* size - bumped by add_menu_item */
  frq_mrgn_menu_primary_items,
};
static struct menuinfo *frq_margin_util_menup = &frqmargindiag;

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
void build_margin_menu(int dummy)
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
 * function:	build_vtg_margin_menu
 *
 * Description:	Build menu for Voltage Margin utility.
 *
 * Input:	dummp - Not used.
 *
 * Output:	None.
 *
 **********************************************************************
 */
void build_vtg_margin_menu(int dummy)
{
    margin_init(0);
    build_primary_submenu(vtg_mrgn_items, VTG_MARGIN_MENU_TABLE_SIZE,
        "Voltage Margin Utility Menu", &vtg_margin_util_menup);
    build_secondary_submenu(vtg_mrgn_items,  VTG_MARGIN_MENU_TABLE_SIZE,
        vtg_mrgn_menu_secondary_items);
    menu(&vtgmargindiag, vtg_mrgn_menu_secondary_items, 0);

}

/**********************************************************************
 *
 * function:	build_freq_margin_menu
 *
 * Description:	Build menu for Frequency Margin utility.
 *
 * Input:	dummp - Not used.
 *
 * Output:	None.
 *
 **********************************************************************
 */
void build_frq_margin_menu(int dummy)
{
    build_primary_submenu(frq_mrgn_items, FRQ_MARGIN_MENU_TABLE_SIZE,
        "Frequency Margin Utility Menu", &frq_margin_util_menup);
    build_secondary_submenu(frq_mrgn_items,  FRQ_MARGIN_MENU_TABLE_SIZE,
         frq_mrgn_menu_secondary_items);
    menu(&frqmargindiag, frq_mrgn_menu_secondary_items, 0);

}

void margin_init(int print_msg)
{
    n2g_i2c_dev_t i2c_dev;

    /* Init device structure */
    init_tps536xx_i2c_struct(&i2c_dev, MB_I2C_TPS53659_3P3V);

    /* Write to the PAGE command to select the desired channel (E.g. PAGE = 00h for channel A) */
    i2c_smbus_write_word_data(i2c_dev.fp, CMD_PAGE, PAGE_A);

    /* Write MFR_SPECIFIC_02(D2h) to 01h to ensure that the PMBus interface has control of the output voltage. */
    i2c_smbus_write_word_data(i2c_dev.fp, CMD_MFR_SPECIFIC_02, SEL_PMBUS);

    /* Write Margin High(25h): 0x7C => 4*(245 + 5*0x7C) = 3460mV */
    i2c_smbus_write_word_data(i2c_dev.fp, CMD_VOUT_MARGIN_HIGH, MARGIN_3460MV);

    /* Write Margin Low: 0x6C(26h) => 4*(245 + 5*0x6C) = 3140mV */
    i2c_smbus_write_word_data(i2c_dev.fp, CMD_VOUT_MARGIN_LOW, MARGIN_3140MV);
    if (print_msg) {
        printf("Process Completed.");
    }
}

/**********************************************************************
 *
 * Function:	idt8a3_freq_margin_f
 *
 * Description:	utility entry point to set clock frequency output of idt8a3xxxx.
 *
 * Input:	n/a.
 *
 * Output:	PASSED/FAILED
 *
 **********************************************************************
 */
static int idt8a3_freq_margin_f( void)
{
    int rc = PASSED;

    /* calling for menu driven */
    uint32_t freq_margin_ppm_param = 1;
    uint32_t freq_margin_high_param = 1;
    uint32_t freq_margin_N = 0x0001;

    freq_margin_ppm_param  = getdec_answer("frequency margin in ppm (0: normal; 100)", 100, 0, 100);
    /* only allow 100ppm frequency margin */
    if ( (freq_margin_ppm_param != 0) && (freq_margin_ppm_param != 100) ) {
        printf(" ERROR: incorrect margin %dppm input!!. Only allow 0 or 100ppm\n",
                 freq_margin_ppm_param);
        return (FAILED);
    }

    freq_margin_high_param = getdec_answer("frequency margin high or low (1: high; 0: low)", 1, 0, 1);

    rc |= idt8a3_set_freq_margin(freq_margin_N, freq_margin_ppm_param, freq_margin_high_param );
    usleep(10*1000);  // 10ms

    /* need to reset PHY after frequency changed */
    bcm82757_reset( 1 );
    bcm54194_reset();

    return (rc);
}


/**********************************************************************
 *
 * Function:	idt8a3_show_freq_f
 *
 * Description:	utility entry point to display current clock frequency
 *              output of idt8a3xxxx.
 *
 * Input:	n/a.
 *
 * Output:	PASSED/FAILED
 *
 **********************************************************************
 */
static int idt8a3_show_freq_f( void)
{
    /* calling for menu driven */
    return idt8a3_show_freq();
}



/*-------------------------------------------------
 * $Log: platform_margin_utils.c,v $
 * Revision 1.2  2021/06/02 08:22:35  iachang
 * CSCvo59196-33 : Merge Fugazi from ASR1K-main-branch to ISR main trunk
 *
 * Revision 1.1.8.2  2020/08/26 02:37:51  iachang
 * Merge Fugazi code into main trunk
 *
 * Revision 1.1.6.14  2020/07/24 23:31:15  pdoong
 * Only allow 0 or 100ppm margin input.
 *
 * Revision 1.1.6.13  2019/08/20 21:55:04  pdoong
 * frequency margin only allow +-100ppm
 *
 * Revision 1.1.6.12  2019/08/09 19:46:58  pdoong
 * Workaround for CSCvq86209: SyncE Recovered clock test failed on frequency margin high&low
 *
 * Revision 1.1.6.11  2019/06/19 01:20:44  letsai
 * Hide initial message when get into menu(Voltage margin utility ).
 *
 * Revision 1.1.6.10  2019/06/15 00:00:37  pdoong
 * Change frequency margin parameter from % to ppm
 *
 * Revision 1.1.6.9  2019/05/14 01:25:18  letsai
 * Set voltage margin values before build menu.
 *
 * Revision 1.1.6.8  2019/05/13 08:22:47  letsai
 * Created Voltage and Frequency utilities submenu in Margin utility menu
 *
 * Revision 1.1.6.7  2019/05/13 07:35:28  letsai
 * Add utility to show current margin.
 *
 * Revision 1.1.6.6  2019/05/10 23:31:56  pdoong
 * added display current PLL clock frequency utility
 *
 * Revision 1.1.6.5  2019/05/03 23:27:53  pdoong
 * added pll frequency margin utility
 *
 * Revision 1.1.6.4  2019/04/25 01:19:40  letsai
 * 1. Remove UART utility
 * 2. Modify SyncE PLL Interrupt test
 * 3. Modify Margin utility
 *
 * Revision 1.1.6.3  2019/04/06 01:36:14  letsai
 * 1. Remove unused functions and files.
 * 2. Fix BCM54194 SFP External loopback test.
 * 3. Fix BCM54194 Register test.
 * 4. Fix Voltage Margin Utility.
 * 5. Add function to show system information.
 *
 * Revision 1.1.6.2  2019/03/14 03:48:37  letsai
 * Initial check in.
 *
 *
 *
 *
 * $Endlog$
 */

