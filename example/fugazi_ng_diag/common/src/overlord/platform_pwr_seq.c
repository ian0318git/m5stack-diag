/* $Id: platform_pwr_seq.c,v 1.4 2013/11/26 08:40:36 hroni Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/overlord/platform_pwr_seq.c,v $
 *------------------------------------------------------------------
 * Filename:	platform_pwr_seq.c
 *
 * Description: Informers Power Sequencer I2C device.
 *
 *		According to EDCS-618748 (Xformers Power Sequencer HFS),
 *		"Diagnostics must consider a board to have failed if either
 *		the VOLTAGE_FAULT_DURING_POWER_UP or the
 *		VOLTAGE_FAULT_DURING_OPERATION bits are set in the Status
 *		Register. In additional to flagging the particular platform
 *		as "FAILED", the diagnostic code should dump all the Power
 *		Sequencer registers to aid in debug and isolation of the
 *		problem."
 *
 *		The Power Sequencer has a PWR_MCU_RST_L (power sequencer
 *		reset signal) pin, and a GFYM_HRESET_OUT_L ("This is the
 *		software reset signal") pin. Only GFYM_HRESET_OUT_L from
 *		Goofy can reset the Power Sequencer.
 *
 *    Notes: update register table from Operation-Overlord
 *    Power Sequencer Hardware Functional Specification
 *    (EDCS-1042044) 2011.07.14
 *
 * Copyright (c) 2013 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */
#include <stdio.h>
#include <string.h>
#include "endians.h"
#include "common.h"
#include "nvmonvars.h"
#include "plat_defs.h"
#include "platform_pwr_seq.h"
#include "defs.h"
#include "proto.h"
#include "error.h"
#include "mon_plat_defs.h"
#include "cross_platform.h"
#include "platform_i2c.h"
#include "menu.h"
#include "n2g_api_rc.h"
#include "byteswap.h"
#include "i2c_address.h"
#include "dash_fpga.h"
#include "goofy_i2c.h"


#define WD_BYPASS /* */

/* Function prototypes */
extern uint32_t n2g_i2c_write(n2g_i2c_if_t *i2c_p);
extern uint32_t n2g_i2c_read(n2g_i2c_if_t *i2c_p);
extern uint32 err_report (dev_object_t *dev, char *err_msg,
			  uint32 err_type); /* in hwic_spidey_ct3.c */
extern int do_all_menu_items(struct menuinfo *);

static int get_pwr_seq_i2c_struct(n2g_i2c_if_t *);
static int show_reg(void);
static int get_pwr_seq_fw_rev(int);
static int alter_reg(void);
static int pwr_read(n2g_i2c_if_t *, char *);
static int pwr_reg_test(int);
static int pwr_stat_test(int);
static int pwr_ctr_test(int);
static int pwr_vtg_check(int);
static int pwr_stat_check(char *);
static int get_reg(uint32_t, uint16_t *);
static int pwr_seq_read_reg(int, uint16_t *);
static int pwr_seq_write_reg(int, uint16_t);
static int pwr_write(n2g_i2c_if_t *);
static int get_pwr_seq_iadc(uint16_t *);
static int get_pwr_seq_vadc(uint16_t *);


/* Global variables */
static reg_info_t pwr_seq_table[]=
{
    /* 
     * update register table for ovld (EDCS-1042044)
     * Register name, Offset, Type, Size, Mask, Reset Value 
     */
    
    {"Firmware Revision",                    PWR_SEQ_REV,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT},
	0x0000, 0x0000},
    {"Status - Sticky",                      PWR_SEQ_STA_S,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT},
	0x0000, 0x0000},
    {"Status - Clearable",                   PWR_SEQ_STA_C,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT},
	0x0000, 0x0000},
    {"Voltage Fault - Sticky",               PWR_SEQ_V_F_S,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT},
	0x0000, 0x0000},
    {"Voltage Fault - Clearable",            PWR_SEQ_V_F_C,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT},
	0x0000, 0x0000},
    {"Run Time Counter Word 2 (MS)",         PWR_SEQ_RTC_2,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT},
	0x0000, 0x0000},
    {"Run Time Counter Word 1",              PWR_SEQ_RTC_1,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT},
	0x0000, 0x0000},
    {"Run Time Counter Word 0 (LS)",         PWR_SEQ_RTC_0,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT},
	0x0000, 0x0000},
    {"12 Volt, Real Time",                   PWR_SEQ_12_RT,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT},
	0x0000, 0x0000},
    {"12 Volt, Maximum",                     PWR_SEQ_12_MAX,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT},
	0x0000, 0x0000},
    {"12 Volt, Minimum",                     PWR_SEQ_12_MIN,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT},
	0x0000, 0x0000},
    {"5.0 Volt, Real Time",                  PWR_SEQ_5_RT,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT},
	0x0000, 0x0000},
    {"5.0 Volt, Maximum",                    PWR_SEQ_5_MAX,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT},
	0x0000, 0x0000},
    {"5.0 Volt, Minimum",                    PWR_SEQ_5_MIN,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT},
	0x0000, 0x0000},
    {"3.3 Volt, Real Time",                  PWR_SEQ_3_3_RT,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT},
	0x0000, 0x0000},
    {"3.3 Volt, Maximum",                    PWR_SEQ_3_3_MAX,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT},
	0x0000, 0x0000},
    {"3.3 Volt, Minimum",                    PWR_SEQ_3_3_MIN,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT},
	0x0000, 0x0000},
    {"1.8V_PCH Volts, Real Time",            PWR_SEQ_PCHV_1_8_RT,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT},
	0x0000, 0x0000},
    {"1.8V_PCH Volts, Maximum",	             PWR_SEQ_PCHV_1_8_MAX, 
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT},
	0x0000, 0x0000},
    {"1.8V_PCH, Minimum",                    PWR_SEQ_PCH_1_8_MIN,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT},
	0x0000, 0x0000},
    {"1.5V_PCH, Real Time",                  PWR_SEQ_PCH_1_5_RT, 
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT},
	0x0000, 0x0000},
    {"1.5V_PCH, Maximum",                    PWR_SEQ_PCH_1_5_MAX,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT},
	0x0000, 0x0000},
    {"1.5V_PCH, Minimum",                    PWR_SEQ_PCH_1_5_MIN, 
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT},
	0x0000, 0x0000},
    {"12V Current, Real Time",               PWR_SEQ_12_C_RT,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT},
	0x0000, 0x0000},
    {"12V Current, Maximum",                 PWR_SEQ_12_C_MX,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT},
	0x0000, 0x0000},
    {"12V Current, Minimum",                 PWR_SEQ_12_C_MN,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT},
	0x0000, 0x0000},
    {"1.5V_CCPU, Real Time",                 PWR_SEQ_CC_1_5_RT,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT},
	0x0000, 0x0000},
    {"1.5V_CCPU, Maximum",                   PWR_SEQ_CC_1_5_MAX, 
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT},
	0x0000, 0x0000},
    {"1.5V_CCPU, Minimum",                   PWR_SEQ_CC_1_5_MIN,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT},
	0x0000, 0x0000},
    {"1.5V_ICPU , Real Time",                PWR_SEQ_IC_1_5_RT,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT},
	0x0000, 0x0000},
    {"1.5V_ICPU, Maximum",                   PWR_SEQ_IC_1_5_MAX,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT}, 
	0x0000, 0x0000},
    {"1.5V_ICPU, Minimum",                   PWR_SEQ_IC_1_5_MIN,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT},
	0x0000, 0x0000},
    {"1.05V, Real Time",                     PWR_SEQ_1_05_RT,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT},
	0x0000, 0x0000},
    {"1.05V, Maximum",                       PWR_SEQ_1_05_MAX,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT},
	0x0000, 0x0000},
    {"1.05V, Minimum",                       PWR_SEQ_1_05_MIN,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT},
	0x0000, 0x0000},
    {"1.0V_PCH, Real Time",                  PWR_SEQ_PCH_1_RT,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT},
	0x0000, 0x0000},
    {"1.0V_PCH, Maximum",                    PWR_SEQ_PCH_1_MAX,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT},
	0x0000, 0x0000},
    {"1.0V_PCH, Minimum",                    PWR_SEQ_PCH_1_MIN,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT},
	0x0000, 0x0000},
    {"Scratch Pad 0",                        PWR_SEQ_SCR0,
	READ_WRITE | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT},
	0xFFFF, 0x0000},
    {"Scratch Pad 1",                        PWR_SEQ_SCR1,
	READ_WRITE | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT},
	0xFFFF, 0x0000},
    {"Scratch Pad 2",                        PWR_SEQ_SCR2,
	READ_WRITE | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT},
	0xFFFF, 0x0000},
    {"Scratch Pad 3",                        PWR_SEQ_SCR3,
	READ_WRITE | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT},
	0xFFFF, 0x0000},
    {"Disable Voltage Fault Detection",      PWR_SEQ_F_DET,
	READ_WRITE | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT},
	0xFFFF, 0x0000},
    {"Enable Double Sampling",               PWR_SEQ_D_SAMP,
	READ_WRITE | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT},
	0xFFFF, 0x0000},
    {"Enable System Watchdog Timer",         PWR_SEQ_WDG_EN,
	READ_WRITE | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT},
	0x01FF, 0x00B7},
    {"Watchdog Refresh",                     PWR_SEQ_WDG_RF,
	READ_WRITE | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT},
	0xFFFF, 0x0000},
    {"Watchdog Timeout Value",               PWR_SEQ_WDG_TO,
	READ_WRITE | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT},
	0xFFFF, 0x0078},
    {"Power Down Enable Register",           PWR_SEQ_PD_EN,
	READ_WRITE | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT},
	0xFFFF, 0x003C},
    {"Power Down Time Register",             PWR_SEQ_PD_TIMER,
	READ_WRITE | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT},
	0xFFFF, 0x0000},
    {"12 Volt Ramp Time",                    PWR_SEQ_12_RPT,
	READ_WRITE | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT},
	0xFFFF, 0x0000},
    {"3.3 Volt Ramp Time",                   PWR_SEQ_3P3_RPT,
	READ_WRITE | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT},
	0xFFFF, 0x0900},
    {"5 Volts Ramp Time",                    PWR_SEQ_5_RPT,
	READ_WRITE | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT},
	0xFFFF, 0x0900},
    {"1.8V_PCH  Ramp Time",                  PWR_SEQ_PCH_1P8_RPT,
	READ_WRITE | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT},
	0xFFFF, 0x0900},
    {"1.5V_PCH Ramp Time",                   PWR_SEQ_PCH_1P5_RPT,
	READ_WRITE | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT},
	0xFFFF, 0x0900},
    {"1.5V_CCPU Ramp Time",                  PWR_SEQ_CC_1P5_RPT,
	READ_WRITE | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT},
	0xFFFF, 0x0900},
    {"1.5V_ICPU Ramp Time",                  PWR_SEQ_IC_1P5_RPT,
	READ_WRITE | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT},
	0xFFFF, 0x0900},
    {"1.05 Volts Ramp Time",                 PWR_SEQ_1P05_V_RPT,
	READ_WRITE | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT},
	0xFFFF, 0x0C00},
    {"1.0V_PCH Ramp Time",                   PWR_SEQ_PCH_1_V_RPT,
	READ_WRITE | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT},
	0xFFFF, 0x0900},
    {"Number of Power Up Loops",             PWR_SEQ_NUM_PWR_UP_LP,
	READ_WRITE | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT},
	0xFFFF, 0x0000},
    {"Power Up Flow",                        PWR_SEQ_PWR_UP_FLOW,
	READ_WRITE | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT},
	0xFFFF, 0x0000},
    {"Maximum 12V Power Up Current",         PWR_SEQ_MX_12V_PWR_CUR,
	READ_WRITE | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT},
	0xFFFF, 0x0000},
    {"Power Up Current Count",               PWR_SEQ_PWR_UP_CUR_CNT,
	READ_WRITE | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT},
	0xFFFF, 0x0000},
    {"Power Up First Voltage Level",         PWR_SEQ_PWR_UP_1ST_VTG_L,
	READ_WRITE | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT},
	0xFFFF, 0x0000},
    {"Power Up Voltage Count",               PWR_SEQ_PWR_UP_VTG_CNT,
	READ_WRITE | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT},
	0xFFFF, 0x0000},
    {"Number of Power Up Loops - Saved",     PWR_SEQ_NUM_PWR_UP_LP_S,
	READ_WRITE | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT},
	0xFFFF, 0x0000},
    {"Power Up Flow - Saved",                PWR_SEQ_PWR_UP_FLOW_S,
	READ_WRITE | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT},
	0xFFFF, 0x0000},
    {"Maximum 12V Power Up Current - Saved", PWR_SEQ_MX_12V_PWR_CUR_S,
	READ_WRITE | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT},
	0xFFFF, 0x0000},
    {"Power Up Current Count - Saved",       PWR_SEQ_PWR_UP_CUR_CNT_S,
	READ_WRITE | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT},
	0xFFFF, 0x0000},
    {"Power Up First Voltage Level - Saved", PWR_SEQ_PWR_UP_1ST_VTG_L_S,
	READ_WRITE | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT},
	0xFFFF, 0x0000},
    {"Power Up Voltage Count - Saved",       PWR_SEQ_PWR_UP_VTG_CNT_S,
	READ_WRITE | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT},
	0xFFFF, 0x0000},
    {"Power LED Control Register",           PWR_SEQ_PWR_LED_CTRL,   
	READ_WRITE | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT},
	0xFFFF, 0x2040},
    {"Power Cycle Request Count Register",   PWR_SEQ_PWR_CYC_REQ_CNT,     
	READ_WRITE | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT},
	0xFFFF, 0xAAAA},
    {"Power Control Register",               PWR_SEQ_PWR_CTRL,
	READ_WRITE | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT},
	0xFFFF, 0x0010},
    {"End", 0xFFFFFFFF, READ_ONLY, {0}, 0, 0},
};

/* change to ovld version */
static pwr_seq_v_t voltage_test_table[] =
{
    {"12 Volt", PWR_12V_OVER, PWR_12V_UNDER, PWR_SEQ_12_RT,
	PWR_SEQ_12_MAX, PWR_SEQ_12_MIN},
    {"5.0 Volt", PWR_5V_OVER, PWR_5V_UNDER, PWR_SEQ_5_RT,
	PWR_SEQ_5_MAX, PWR_SEQ_5_MIN},
    {"3.3 Volt", PWR_3P3V_OVER, PWR_3P3V_UNDER, PWR_SEQ_3_3_RT,
	PWR_SEQ_3_3_MAX, PWR_SEQ_3_3_MIN},
	  {"1.80 PCH", PWR_1P8V_PCH_OVER, PWR_1P8V_PCH_UNDER, PWR_SEQ_PCHV_1_8_RT,
	PWR_SEQ_PCHV_1_8_MAX, PWR_SEQ_PCH_1_8_MIN},
    {"1.50 PCH", PWR_1P5V_PCH_OVER, PWR_1P5V_PCH_UNDER, PWR_SEQ_PCH_1_5_RT,
	PWR_SEQ_PCH_1_5_MAX, PWR_SEQ_PCH_1_5_MIN},
    {"1.50 ICPU", PWR_1P5V_ICPU_OVER, PWR_1P5V_ICPU_UNDER, PWR_SEQ_IC_1_5_RT, 
	PWR_SEQ_IC_1_5_MAX, PWR_SEQ_IC_1_5_MIN},
    {"1.50 CCPU", PWR_1P5V_CCPU_OVER, PWR_1P5V_CCPU_UNDER, PWR_SEQ_CC_1_5_RT, 
	PWR_SEQ_CC_1_5_MAX, PWR_SEQ_CC_1_5_MIN},
    {"1.05", PWR_1P05V_OVER, PWR_1P05V_UNDER, PWR_SEQ_1_05_RT, 
	PWR_SEQ_1_05_MAX, PWR_SEQ_1_05_MIN},
    {"1.00 PCH", PWR_1P0V_PCH_OVER, PWR_1P0V_PCH_UNDER, PWR_SEQ_PCH_1_RT,
        PWR_SEQ_PCH_1_MAX, PWR_SEQ_PCH_1_MIN},
    {NULL, 0, 0, 0, 0, 0},
};


/*
 * Power Sequencer Menu
 */
static submenu_xtable_t pwr_menu_table[] = {
    {"Show Power Sequencer FW version", (PFT)get_pwr_seq_fw_rev,   1,
        0,                              (type_t(*)())0, 0, (PFT)0, 0},
    {"Show Power Sequencer registers",  (PFT)show_reg,             0,
        0,                              (type_t(*)())0, 0, (PFT)0, 0},
    {"Alter Power Sequencer register",  (PFT)alter_reg,            0,
        0,                              (type_t(*)())0, 0, (PFT)0, 0},
    {"Initialize Power Sequencer",      (PFT)init_pwr_seq,      TRUE,
        0,                              (type_t(*)())0, 0, (PFT)0, 0},
    {"Voltage Check",                   (PFT)pwr_vtg_check,     TRUE,
        (MF_CONTINUOUS | MF_DOALL),     (type_t(*)())0, 0, (PFT)0, 0},
    {"Status Check",                    (PFT)pwr_stat_test,     TRUE, 
        (MF_CONTINUOUS | MF_DOALL),     (type_t(*)())0, 0, (PFT)0, 0},
    {"Registers Test",                  (PFT)pwr_reg_test,      TRUE,
        (MF_CONTINUOUS | MF_DOALL),     (type_t(*)())0, 0, (PFT)0, 0},
    {"Run Time Counter Test",           (PFT)pwr_ctr_test,      TRUE,
        (MF_CONTINUOUS | MF_DOALL),     (type_t(*)())0, 0, (PFT)0, 0},
};

#define PWR_MENU_TABLE_SIZE (sizeof(pwr_menu_table) / sizeof(submenu_xtable_t))

/*
 * Primary & secondary submenu items (filled in from xtable)
 */
static mitem_t pwr_menu_primary_items[PWR_MENU_TABLE_SIZE + MAX_BASE_ITEMS];
static mitem_t pwr_menu_secondary_items[PWR_MENU_TABLE_SIZE + MAX_BASE_ITEMS];

static struct menuinfo pwrdiag = {
    "Power Sequencer Utility Menu",	/* title */
    0,				/* title string added by init_empty_menu */
    (PFT)menu_show_dflags,	/* shows major flags */
    0,				/* generic prompt */
    0,				/* size -- bumped by add_menu_item() */
    pwr_menu_primary_items,
};

static struct menuinfo *pwrdiagp = &pwrdiag;


#define PWR_MRGN_LOOP 1
#define PWR_MRGN_TBL_SIZE (sizeof(pwr_mrgn_tbl) / sizeof(pwr_mrgin_t))


/*******************************************************************************
 *
 * Function   : get_pwr_seq_i2c_struct
 * Description: To get Power Sequencer I2C interface structure.
 * Inputs     : Pointer to save the gotten I2C interface structure
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
static int get_pwr_seq_i2c_struct (n2g_i2c_if_t *pwr_seq_i2c)
{
    n2g_i2c_if_t *tmp;

    /* init i2c_if for I2C */
    tmp = (n2g_i2c_if_t *)get_n2g_i2c_if(I2C_CTRL_TWO, I2C_MUX_ZERO,
                                         MB_I2C_ADDR_PWR_SEQ);
    if (tmp == NULL) {
        printf("%s: Failed to get Power Sequencer I2C interface structure.\n",
               __FUNCTION__);
        return (FAILED);
    }

    memcpy(pwr_seq_i2c, tmp, sizeof(n2g_i2c_if_t));

    return (PASSED);
}


/*******************************************************************************
 *
 * function   : build_pwr_seq_menu
 * Description: Build Power Sequencer menu.
 * Inputs     : None
 * Outputs    : None
 *
 *******************************************************************************
 */
void build_pwr_seq_menu (int menu_opt)
{
    build_primary_submenu(pwr_menu_table, PWR_MENU_TABLE_SIZE,
			  "Power Sequencer Utility Menu", &pwrdiagp);
    build_secondary_submenu(pwr_menu_table, PWR_MENU_TABLE_SIZE,
			    pwr_menu_secondary_items);

    if (menu_opt) {
        /* Entered with submenu */
        menu(&pwrdiag, pwr_menu_secondary_items, 0);
    } else {
        do_all_menu_items(pwrdiagp);
    }
}


/*******************************************************************************
 *
 * Function   : get_pwr_seq_fw_rev
 * Description:	Returns Power sequencer FW revision.
 * Inputs     : option - to determine use cterr or not
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
static int get_pwr_seq_fw_rev (int option)
{
    uint32_t rc = FAILED;
    uint16_t rev = 0;

    rc = get_reg(PWR_SEQ_REV, &rev);
    if (rc != PASSED) {
        if (option) {
	    cterr('f', 0, "%s:%d Failed to read Power Sequencer FW reversion.",
              __FUNCTION__, __LINE__);
        }
        return (rc);
    } 
    printf("Power Sequencer FW version is %d.%02d.\n",
           (rev & PS_REV_MAJOR_MSK) >> PS_REV_MAJOR_SHIFT,
           (rev & PS_REV_MINOR_MSK));
    
    return (rc);
}


/*******************************************************************************
 *
 * Function   : get_pwr_seq_iadc
 * Description:	Function to get 12V current from Power sequencer register.
 * Inputs     : Data pointer to save get back iadc value
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
static int get_pwr_seq_iadc (uint16_t *iadc)
{
    return (get_reg(PWR_SEQ_12_C_RT, iadc));
}


/*******************************************************************************
 *
 * Function   : get_pwr_seq_vadc
 * Description:	Function to get 12V voltage from Power sequencer register.
 * Inputs     : Data pointer to save get back vadc value
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
static int get_pwr_seq_vadc (uint16_t *vadc)
{
    return (get_reg(PWR_SEQ_12_RT, vadc));
}


/*******************************************************************************
 *
 * Function   : get_reg
 * Description:	Read requested Power sequencer register.
 * Inputs     : Register offset
 *              Pointer to register to be saved
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
static int get_reg (uint32_t offset, uint16_t *reg_data)
{
    n2g_i2c_if_t i2c_if;
    uint32_t     rc = FAILED;
    char         err_buf[OVLD_BUF_SIZE];

    /* Get Power Sequencer I2C interface structure */
    rc = get_pwr_seq_i2c_struct(&i2c_if);
    if (rc != PASSED) {
	printf("%s:%d Failed to get Power Sequencer I2C structure",
               __FUNCTION__, __LINE__);
        return (rc);
    }

    /* Setup the related info */
    i2c_if.offset = offset;
    i2c_if.buf = (char *)reg_data;

    /* Read out the value of Register */
    rc = pwr_read(&i2c_if, &err_buf[0]);
    if (rc != PASSED) {
	printf("%s:%d %s\n",__FUNCTION__, __LINE__, err_buf);
    }

    return (rc);
}


/**********************************************************************
 *
 * Function:	init_pwr_seq
 *
 * Description:	Initilize Power Sequencer
 *
 * Inputs:	err_log - cterr if TRUE. printf if FALSE.
 *
 * Outputs:	PASSED/FAILED.
 *
 **********************************************************************
 */
int
init_pwr_seq (int err_log)
{
    n2g_i2c_if_t i2c_if;
    uint32_t rc = FAILED;
    ren_t data;
    reg_info_t *reg_table_p;
    char err_buf[OVLD_BUF_SIZE *2];
    uint ia;
    char *reg_data;
    char reg_tmp[PWR_SEQ_BUF_SIZE];

    /* Get Power Sequencer I2C interface structure */
    rc = get_pwr_seq_i2c_struct(&i2c_if);
    if (rc != PASSED) {
        printf("%s: Failed to get Power Sequencer I2C structure.\n",
              __FUNCTION__);
        return (rc);
    }

    /* init the dev, i2c parameters and buf.*/
    reg_data = &reg_tmp[0];

    /* Read the Revision register */
    i2c_if.offset = PWR_SEQ_REV;
    i2c_if.buf = reg_data;
    
    rc = pwr_read(&i2c_if, &err_buf[0]);
    if (rc != PASSED) {
        if (err_log == FALSE) {
            printf("*** %s: Unable to read Pwr Seq Rev reg.\n", __FUNCTION__);
        } else {
            cterr('f', 0, "%s: Pwr Seq Rev reg read failed.", __FUNCTION__);
        } /* endof if err_log */
        return (FAILED);
    }
    
    data = ((reg_tmp[1] << 8) | reg_tmp[0]);
    printf("Power Sequencer version %d.%02d.\n",
           (data & PS_REV_MAJOR_MSK) >> PS_REV_MAJOR_SHIFT,
           data & PS_REV_MINOR_MSK);

    /* Xformers Power Sequencer requires the status register to be read. */
    rc = pwr_stat_check(&err_buf[0]);	/* Check the status register */
    if ((rc == FAILED) || (rc == PWR_SEQ_STAT_CHECK_FAIL)) {
        /* Registers read/write failure */
        if (err_log == FALSE) {
            printf("\n***** %s ******\n", err_buf);
        } 

        if (rc == PWR_SEQ_STAT_CHECK_FAIL) {
            /* Dump out the registers per spec */
            /* Do not use pwr_show_reg() since it creates and destroys the
             * device object.
             */
            for (ia = 0, reg_table_p = &pwr_seq_table[0];
                 ia < (sizeof(pwr_seq_table) / sizeof(reg_info_t));
                 ia++, reg_table_p++) {
    
                i2c_if.offset = reg_table_p->offset;
    
                /* the byteswap is activiated in pwr_read. */
                rc = pwr_read(&i2c_if, &err_buf[0]);   
                if (rc != PASSED) {
                    /* Unable to read data */
                    cterr('f', 0, "%s: read - %s", __FUNCTION__, err_buf);
                    return (FAILED);
                }

                /* if size of *reg_data is larger than size of char, 
                 * the reg_tmp[1] will contain data  
                 */
                printf("%s - %02x , data:0x%x \n", reg_table_p->name,
                       reg_table_p->offset, ((reg_tmp[1] << 8) | reg_tmp[0]));           
            }

#ifdef PWR_DEBUG
        } else {
            printf("\n Pwr Seq. Status register = %#x\n", status);
#endif /* PWR_DEGUB */
        } /* endof if status */
    }

    return (rc);
}


/*******************************************************************************
 *
 * Function   : show_reg
 * Description:	Display Power Sequencer Registers.
 * Inputs     : None
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
static int show_reg (void)
{
    int result = FAILED;
    uint16_t buffer = 0;
    uint32_t offset;
    reg_info_t *reg_ptr;
    
    reg_ptr = &pwr_seq_table[0];

    printf("\n");

    while (reg_ptr->offset != 0xFFFFFFFF) {
        /* Print all registers */ 
        offset = (reg_ptr->offset);
        /* the byteswap is activiated in env_read. */
        if ((result = pwr_seq_read_reg(offset, &buffer)) != RC_I2C_OP_OK) {
            cterr('f', 0, "%s: Unable to read Regiser %#x.",
                  __FUNCTION__, offset);
            return (FAILED);
        }
        printf("%-36s (0x%02X), data: 0x%04X.\n", reg_ptr->name,
               reg_ptr->offset, buffer);
        reg_ptr++;
    }
    return (result);
}

 
/*******************************************************************************
 *
 * Function   : pwr_seq_read_reg
 * Description:	Read Power Sequencer Register.
 * Inputs     : offset of the regiseter
 *              buffer to store the read out data
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
static int pwr_seq_read_reg (int offset, uint16_t *data)
{
    n2g_i2c_if_t i2c_if;
    char         err_buf[OVLD_BUF_SIZE];
    int          result = FAILED;

    /* Get Power Sequencer I2C interface structure */
    result = get_pwr_seq_i2c_struct(&i2c_if);
    if (result != PASSED) {
        printf("%s: Failed to get Power Sequencer I2C structure.\n",
               __FUNCTION__);
        return (result);
    }

    /* Setup I2C API parameter struct */
    i2c_if.buf = (char *)data;
    i2c_if.offset = offset;
    
    result = pwr_read(&i2c_if , &err_buf[0]);   
    if (result != PASSED) {
        /* Unable to read data */
        printf("%s: Unable to read(result = 0x%08x).\n", __FUNCTION__, result);
    }

    return (result);
}


/*******************************************************************************
 *
 * Function   : alter_reg
 * Description:	Alter Environment MCU Register.
 * Inputs     : None
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
static int alter_reg (void)
{
    uint32_t rc = FAILED, i, offset;
    reg_info_t *reg_table_p;
    uint16_t tmp_mask, data = 0;

    /* Setup I2C API parameter struct */
    printf("\nRegister number:\n");
    for (i = 0, reg_table_p = &pwr_seq_table[0];
         i < (sizeof(pwr_seq_table) / sizeof(reg_info_t));
         i++, reg_table_p++) {
        if (!(reg_table_p->type & READ_ONLY)) {
	    /* Read writeable */
	    printf("   %02x - %s\n", reg_table_p->offset,
				  reg_table_p->name);
	}
    }

    offset = gethex_answer("Enter the register number:", 0, 0,
                           (sizeof(pwr_seq_table) / sizeof(reg_info_t)) - 1);

    /* Got the register offset. Check if writeable */
    for (i = 0, reg_table_p = &pwr_seq_table[0];
         i < (sizeof(pwr_seq_table) / sizeof(reg_info_t));
         i++, reg_table_p++) {

        if (reg_table_p->offset != offset) {
	    continue;
	}

        /* Found the offset */
	if (reg_table_p->type & READ_ONLY) {
	    /* read only */
	    cterr('f', 0, "Read only register");
	    return (FAILED);
	}

	/* Valid offset and writeable */
	tmp_mask = reg_table_p->mask; /* get the mask for user to alter reg. */
	break;
    }

    if ((rc = pwr_seq_read_reg(offset,  &data)) != PASSED) {
	cterr('f', 0, "%s: Failed to read Power Sequencer Reg %#x",
              __FUNCTION__, offset);
        return (rc);
    }

    data = gethex_answer("Enter the 16-bit data:", data, 0, tmp_mask);
    if ((rc = pwr_seq_write_reg(offset, data)) != PASSED) {
	cterr('f', 0, "%s: Failed to wrote Power Sequencer Reg %#x.",
              __FUNCTION__, offset);
    }

    return (rc);
}


/*******************************************************************************
 *
 * Function   : pwr_seq_write_reg
 * Description:	Write Power Sequencer Register.
 * Inputs     : offset of Register that will be written in
 *              data that will be written in
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
static int pwr_seq_write_reg (int offset, uint16_t data)
{
    n2g_i2c_if_t i2c_if;
    uint32_t     result = FAILED;

    /* Get Power Sequencer I2C interface structure */
    result = get_pwr_seq_i2c_struct(&i2c_if);
    if (result != PASSED) {
        printf("%s: Failed to get Power Sequencer I2C structure.\n",
              __FUNCTION__);
        return (result);
    }

    i2c_if.buf = (char *)&data;
    i2c_if.offset = offset;
    
    result = pwr_write(&i2c_if);   
    if (result != PASSED) {
        /* Unable to write data */
        printf("%s: Failed to write to Reg(%#x).", __FUNCTION__, offset);
    }
         
    return (result);
}


/*******************************************************************************
 *
 * Function   : pwr_write
 * Description:	Write Power Sequencer Register.
 * Inputs     : Pointer of the I2C interface structure
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
static int pwr_write (n2g_i2c_if_t *i2c_if)
{
    uint32_t rc = FAILED;
    uint16_t data = 0;

    if (!i2c_if->buf) {
        assert(!"env_write: buf is null");
    }

    data = DSWAP2(*(uint16_t *)i2c_if->buf);
    memcpy(i2c_if->buf, &data, sizeof(uint16_t));
    
    rc = n2g_i2c_write(i2c_if);
    if (rc != RC_I2C_OP_OK) {
	msleep(REN_I2C_PROC_TIME);	/* Env MCU I2C cycle time */
        return (FAILED);
    }

    msleep(REN_I2C_PROC_TIME);	/* Env MCU I2C cycle time */
    return (PASSED);
}


/*******************************************************************************
 *
 * Function   : pwr_read
 * Description: Local Read Power Sequencer Register. Power Sequencer I2C read
 *		has 2 I2C operations. The I2C write with the register offset.
 *		Then wait for the REN_I2C_PROC_TIME milliseconds to allow
 *		the Power Sequencer firmware to setup the data of the requested
 *		register. Then the I2C read will return the data.
 * Inputs     : i2c_if - pointer to the I2C API struct
 *		err_buf - Points to the error buffer
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
int pwr_read (n2g_i2c_if_t *i2c_if, char *err_buf)
{
    uint32_t rc = FAILED;
    uint16_t data;

    if (!i2c_if->buf) {
        assert(!"env_read: buf is null");
    }
    
    rc = n2g_i2c_read(i2c_if);
    if (rc != RC_I2C_OP_OK) {
	/* Unable to read data */
	sprintf(err_buf, "%s:%d Failed to read Reg. %#x (rc = %#x).",
                __FUNCTION__, __LINE__, i2c_if->offset, rc);
	return (FAILED);
    }

    data = DSWAP2(*(uint16_t *)i2c_if->buf);
    memcpy(i2c_if->buf, &data, sizeof(uint16_t));

    msleep(REN_I2C_PROC_TIME);	/* I2C cycle time */

    return (PASSED);
}


/*******************************************************************************
 *
 * Function   : pwr_reg_test
 * Description: Test read/writeable registers.
 * Inputs     : submenu - TRUE if invoked from submenu
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
static int pwr_reg_test (int submenu)
{
    n2g_i2c_if_t i2c_if;
    reg_info_t   *reg_ptr;
    char         err_buf[OVLD_BUF_SIZE *2];
    uint16_t     original_data = 0, reg_data, chk_data = 0;
    uint16_t     ia, ix, ii;
    uint16_t     temp = 0, data, mask; 
    uint32_t     rc = FAILED;

/* since the pwr seq register is only 16 bits, 
 * the test pattern is from common.h 
 * #define PATTERN   0x5ADBA56C
 */    
#define PWR_SER_PATTERN 0xA56C

    if (submenu == TRUE) {
	testname("Power Sequencer Registers ");
    } else {
	prpass(testpass, "Power Sequencer Registers Test ");
    }

    /* Get Power Sequencer I2C interface structure */
    rc = get_pwr_seq_i2c_struct(&i2c_if);
    if (rc != PASSED) {
        cterr('f', 0, "%s: Failed to get Power Sequencer I2C structure"
                      " (rc = %#x).", __FUNCTION__, rc);
        return (rc);
    }

    /* init i2c parameters */
    reg_ptr = &pwr_seq_table[0];
    
    for (ia = 0, reg_ptr = &pwr_seq_table[0];
        ia < (sizeof(pwr_seq_table) / sizeof(reg_info_t));
        ia++, reg_ptr++) {

        /* Based on Operation-Overlord Power Sequencer HFS(EDCS-1042044),
         * the description of Reg. "Power Cycle Request Count"(offset = 0x47),
         * SW should not write to this register.
         */
        if (reg_ptr->offset == PWR_SEQ_PWR_CYC_REQ_CNT) {
            prpass(testpass, "SKIP %s Reg.(offset = 0x%02x) based on HFS.",
                   reg_ptr->name, reg_ptr->offset);
            continue;
        }
		
        if ((reg_ptr->type & (READ_ONLY | WRITE_ONLY)) == READ_WRITE) {
            i2c_if.offset = reg_ptr->offset;       
            i2c_if.buf = (char *)&original_data;

            /* check result of read data */
            rc = pwr_read(&i2c_if, &err_buf[0]);  
            if (rc != PASSED) {
                sprintf(err_buf, "%s: FAILED to I2C read from %s (rc = %#x).\n",
                                 __FUNCTION__, reg_ptr->name, rc);
                break;
            }

            /*     
             * Ripple 1 test
             */
            for (ix = 0; ix < (sizeof(i2c_if.size) * 8); ix++) {
                temp = (1 << ix);
                if (!temp) {
                    continue;
                }

                prpass(testpass, "Ripple 1: %s Reg.(0x%02X), "
                                 "Testpatent = 0x%04x",
                                 reg_ptr->name, reg_ptr->offset, temp);

                reg_data = temp;
                i2c_if.buf = (char *)&reg_data;

                rc = pwr_write(&i2c_if);
                if (rc != PASSED) {
                    sprintf(err_buf, "%s: Ripple one FAILED on I2C write "
                                     "0x%04X to %s (rc = %#x).\n",
                                     __FUNCTION__, temp, reg_ptr->name, rc);
                    break;
                }

                msleep(500);
        
                i2c_if.buf = (char *)&chk_data;
                rc = pwr_read(&i2c_if, &err_buf[0]);     
                if (rc != PASSED) {
                    sprintf(err_buf, "%s: Ripple one FAILED on I2C read back %s"
                                     " (rc = %#x).\n",
                                     __FUNCTION__, reg_ptr->name, rc);
                    break;
                }
#if DEBUG
    printf(" *** %s Ripple one , value 0x%x and 0x%x.\n", reg_ptr->name,chk_data,temp );
#endif
                if (chk_data != temp) {
                    rc = FAILED;
                    sprintf(err_buf, "%s: %s Reg. Ripple one test FAILED, "
                                     "read back = 0x%04x and expected = 0x%04x.\n",
                                     __FUNCTION__, reg_ptr->name, chk_data, temp);
		    break;
                }
            }
          
            /* leave the for loop of reg_ptr */ 
            if (rc != PASSED) {
                break;
            }
          
            /* 
             * Ripple 0 test
             */
            for (ix = 0; ix < (sizeof(i2c_if.size) * 8); ix++) {
                temp = (1 << ix);
                if (!temp) {
                    continue;
                }

                temp = (~(1 << ix));

                prpass(testpass, "Ripple 0: %s Reg.(0x%02X), "
                                 "Testpatent = 0x%04x",
                                 reg_ptr->name, reg_ptr->offset, temp);

                reg_data = temp;
                i2c_if.buf = (char *)&reg_data;
          
                rc = pwr_write(&i2c_if);
                if (rc != PASSED) {
                    sprintf(err_buf, "%s: Ripple zero FAILED on I2C write"
                                     " 0x%04X to %s (rc = %#x).\n",
                                     __FUNCTION__, temp, reg_ptr->name, rc);
                    break;
                }

                msleep(500);

                i2c_if.buf = (char *)&chk_data;
                rc = pwr_read(&i2c_if, &err_buf[0]);
                if (rc != PASSED) {
                    sprintf(err_buf, "%s: Ripple zero test FAILED on I2C read back"
                                     " from %s (rc = %#x).\n",
                                     __FUNCTION__, reg_ptr->name, rc);          	
                    break;
                }
#if DEBUG
    printf(" *** %s Ripple zero , value 0x%x and 0x%x.\n", reg_ptr->name, chk_data, temp );
#endif
                if (chk_data != temp) {
                    rc = FAILED;  	
                    sprintf(err_buf, "%s: %s Reg. Ripple zero test FAILED, read back"
                                     " = 0x%04x, and expected = 0x%04x.\n",
                                     __FUNCTION__, reg_ptr->name, chk_data, temp);          	
		    break;
                }
            }
          
            /* leave the for loop of reg_ptr*/ 
            if (rc != PASSED) {
                break;
            }

            /*
             * Pattern test
             */
            data = (uint16_t)PWR_SER_PATTERN;

            for (ix = 0; ix < 2; ix++) {
                /* build mask of size for pattern */
	    	for (ii = 0; ii < (sizeof(i2c_if.size)*8); ii++) {
                    mask |= (1 << ii);
                }
                temp = data & mask;
                if (!temp) {
                    continue;
                }

                reg_data = temp;
                i2c_if.buf = (char *)&reg_data;

                prpass(testpass, "Pattern test: %s Reg.(0x%02X), "
                                 "Testpatent = 0x%04x",
                                 reg_ptr->name, reg_ptr->offset, temp);

                rc = pwr_write(&i2c_if);
                if (rc != PASSED) {
                    sprintf(err_buf, "%s: Pattern test FAILED on I2C write"
                                     " 0x%04X to %s (rc =%#x).\n",
                                     __FUNCTION__, temp, reg_ptr->name, rc);
                    break;
                }

                msleep(500);

                i2c_if.buf = (char *)&chk_data;
                rc = pwr_read(&i2c_if, &err_buf[0]);
                if (rc != PASSED) {
                    sprintf(err_buf, "%s: Pattern test FAILED to I2C read "
                                     "back %s (rc = %#x).\n",
                                     __FUNCTION__, reg_ptr->name, rc);  
                    break;
                }
#if DEBUG
    printf(" *** %s pattern , value 0x%x and 0x%x.\n", reg_ptr->name,chk_data,temp );
#endif

                if (chk_data != temp) {
                    rc = FAILED;
                    sprintf(err_buf, "%s: %s Reg. Pattern test FAILED, read back "
                                     "= 0x%04x, and expected = 0x%04x.\n",
                                     __FUNCTION__, reg_ptr->name, chk_data, temp);
                    break;
                }

                data = (uint16_t)(~PWR_SER_PATTERN); /* complemrent data pattern */
            } /* for (ix = 0; ix < 2; ix++) */

        /* leave the for loop of reg_ptr */ 
        if (rc != PASSED) {
            break;
        }
	    
        /*
         * Restore Reset value
         */
        i2c_if.buf = (char *)&original_data;
        
        rc = pwr_write(&i2c_if);
        if (rc != PASSED) {
            sprintf(err_buf, "%s: Failed to restore value back to %s.\n",
                             __FUNCTION__, reg_ptr->name);   
            break;
        }

    } /*if ((reg_table_p->type & (READ_ONLY | WRITE_ONLY))*/
    } /* for (i = 0, reg_ptr = &pwr_seq_table[0]; */

    if (rc != PASSED) {
        cterr('f', 0, err_buf);
    } /* endof if rc */

    if (submenu == TRUE) {
        prcomplete(testpass, errcount, (char *)0);
    }

    return (rc);
}


/*******************************************************************************
 *
 * Function   : pwr_stat_test
 * Description:	Check Status and Voltage Fault registers.
 * Inputs     : submenu - TRUE if invoked from submenu
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
static int pwr_stat_test(int submenu)
{
    int  rc = FAILED;
    char err_buf[OVLD_BUF_SIZE *2];

    if (submenu == TRUE) {
	testname("Power Sequencer Status");
    } else {
	prpass(testpass, "Power Sequencer Status Test");
    }

    rc = pwr_stat_check(&err_buf[0]);
    if (rc != PASSED) {
	cterr('f', 0, "%s", err_buf);
	return (FAILED);
    }    
    
    if (submenu == TRUE) {
	prcomplete(testpass, errcount, (char *)0);
    }

    return (rc);
}


/*******************************************************************************
 *
 * Function   : pwr_ctr_test
 * Description:	Test Run Time Counter registers to make sure it increments.
 * Inputs     : submenu - TRUE if invoked from submenu
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
static int pwr_ctr_test (int submenu)
{
    n2g_i2c_if_t i2c_if;
    uint32_t     rc = FAILED, retry_count = 0;
    ren_t        rtc_1_ms, rtc_1, rtc_1_ls, rtc_2_ms, rtc_2, rtc_2_ls;
    char         err_buf[OVLD_BUF_SIZE *2];

    if (submenu == TRUE) {
	testname("Power Sequencer Run Time Counter");
    } else {
	prpass(testpass, "Power Sequencer Run Time Counter Test");
    }
 
    /* Get Power Sequencer I2C interface structure */
    rc = get_pwr_seq_i2c_struct(&i2c_if);
    if (rc != PASSED) {
        cterr('f', 0, "%s: Failed to get Power Sequencer I2C structure",
                      __FUNCTION__);
        return (rc);
    }
    
    while (retry_count < PWR_SEQ_RUNTIME_RETRY) {
        /* Read the Run time counter registers first */
        i2c_if.offset = PWR_SEQ_RTC_2; 
        i2c_if.buf = (char *)&rtc_1_ms;

        if ((rc = pwr_read(&i2c_if, &err_buf[0])) != PASSED) {
            sprintf(err_buf, "%s: Unable to read MS byte first time.\n",
                             __FUNCTION__);
        } else {
            i2c_if.offset = PWR_SEQ_RTC_1;
            i2c_if.buf = (char *)&rtc_1;

            if ((rc = pwr_read(&i2c_if, &err_buf[0])) != PASSED) {
                sprintf(err_buf, "%s: Unable to read middle byte first time.\n",
                                 __FUNCTION__);
            } else {
                i2c_if.offset = PWR_SEQ_RTC_0;
                i2c_if.buf = (char *)&rtc_1_ls;

                if ((rc = pwr_read(&i2c_if, &err_buf[0])) != PASSED) {
                    sprintf(err_buf, "%s: Unable to read LS byte first time.\n",
                                     __FUNCTION__);
                } /* endof if RTC_0 */
            } /* endof if RTC_1 */
        } /* endof if RTC_2 */

        if (rc != PASSED) {
            /* Read failed */
            rc = FAILED;
            break;  /* out of the while */
        }

        /* Wait for the Run Time counter to increment */
        msleep(PWR_SEQ_RTC_TEST_TIME);

        /* Read the Run time counter registers again */
        i2c_if.offset = PWR_SEQ_RTC_2;
        i2c_if.buf = (char *)&rtc_2_ms;

        if ((rc = pwr_read(&i2c_if, &err_buf[0])) != PASSED) {
            sprintf(err_buf, "%s: Unable to read MS byte again.\n",
                             __FUNCTION__);
        } else {
            i2c_if.offset = PWR_SEQ_RTC_1;
            i2c_if.buf = (char *)&rtc_2;

            if ((rc = pwr_read(&i2c_if, &err_buf[0])) != PASSED) {
                sprintf(err_buf, "%s: Unable to read middle again.\n",
                                 __FUNCTION__);
            } else {
                i2c_if.offset = PWR_SEQ_RTC_0;
                i2c_if.buf = (char *)&rtc_2_ls;

                if ((rc = pwr_read(&i2c_if, &err_buf[0])) != PASSED) {
                    sprintf(err_buf, "%s: Unable to read LS byte again.\n",
                                     __FUNCTION__);
                } /* endof if RTC_0 read */
            } /* endof if RTC_1 read */
        } /* endof if RTC_2 read */

        if (rc != PASSED) {
            /* Read failed */
            rc = FAILED;
            break;  /* out of the while */
        }

        /* Check the readings */
        /* Also covers the wrap around case */
        if (((rtc_2_ms < rtc_1_ms) && rtc_2_ms && rtc_2) ||
            ((rtc_2_ms == rtc_1_ms) && (rtc_2 < rtc_1)) ||
            ((rtc_2_ms == rtc_1_ms) && (rtc_2 == rtc_1) &&
            (rtc_2_ls <= rtc_1_ls))) {
            if (retry_count >=  PWR_SEQ_RUNTIME_RETRY) {
                sprintf(err_buf, "RunTime Counter test failed. "
                                 "Read %#x %#x %#x first time. "
                                 "Then read %#x, %#x, %#x.\n",
                        rtc_1_ms, rtc_1, rtc_1_ls,
                        rtc_2_ms, rtc_2, rtc_2_ls);
                rc = FAILED;
                break;
            } /* endof if retry_count */
        } else {
            /* Test passed */
            break;
        } /* endof if rtc */
        retry_count++;  /* update the retry count */
    } /* endof while */

    if (rc != PASSED) {
        cterr('f', 0, err_buf);
    } /* endof if rc */

    if (submenu == TRUE) {
        prcomplete(testpass, errcount, (char *)0);
    }

    return (rc);
}


/*********************************************************************
 *
 * Function:	pwr_vtg_check
 *
 * Description:	Check Voltage registers set.
 *
 * Inputs:	submenu - TRUE if invoked from submenu.
 *
 * Outputs:	PASSED/FAILED.
 *
 * Assumptions:
 *
 *********************************************************************
 */
static int pwr_vtg_check(int submenu)
{
    n2g_i2c_if_t i2c_if;
    pwr_seq_v_t  *vtg_p;
    uint32_t     rc = FAILED;
    uint16_t     rt, max, min;
    char         err_buf[OVLD_BUF_SIZE *2], rd_buf[OVLD_BUF_SIZE];

    if (submenu == TRUE) {
	testname("Power Sequencer Voltage Check");
    } else {
	prpass(testpass, "Power Sequencer Voltage Check");
    }

    /* Get Power Sequencer I2C interface structure */
    rc = get_pwr_seq_i2c_struct(&i2c_if);
    if (rc != PASSED) {
        cterr('f', 0, "%s: Failed to get Power Sequencer I2C structure.",
                      __FUNCTION__);
        return (rc);
    }
    
	 vtg_p = &voltage_test_table[0];

	while (vtg_p->volt_p) {
	    /* Read Real Time before Max and Min voltages, since Max and Min
	     * are updated after the Real Time. By following this order, we
	     * can minimize the race condition of these registers. CSCsr68713
	     */
	    /* Read Real Time voltage */
    
/*
    if ((rc = pwr_seq_read_reg(vtg_p->rt_o, &rt)) != RC_I2C_OP_OK) {
            cterr('f', 0, " %s RealTime read failed.", vtg_p->volt_p);
            return FAILED;
    }
    
    if ((rc = pwr_seq_read_reg(vtg_p->max_o, &max)) != RC_I2C_OP_OK) {
            cterr('f', 0, " %s Max read failed.", vtg_p->volt_p);
            return FAILED;
    }
    
    if ((rc = pwr_seq_read_reg(vtg_p->min_o, &min)) != RC_I2C_OP_OK) {
            cterr('f', 0, " %s Min read failed.", vtg_p->volt_p);
            return FAILED;
    }

*/
	    i2c_if.offset = vtg_p->rt_o;
	    i2c_if.buf = (char *)&rt;
	    rc = pwr_read(&i2c_if, &rd_buf[0]);

	    if (rc != PASSED) {
		sprintf(err_buf, "%s: %s RealTime I2C read failed "
                                 "(rc = %#x)(rd_buf = %s).\n",
				 __FUNCTION__, vtg_p->volt_p, rc, rd_buf);
		rc = FAILED;
		break;
	    }

	    /* Read Maximum voltage */
	    i2c_if.offset = vtg_p->max_o;
	    i2c_if.buf = (char *)&max;
	    rc = pwr_read(&i2c_if, &rd_buf[0]);
	    if (rc != PASSED) {
		sprintf(err_buf, "%s: %s Max I2C read failed "
                                 "(rc = %#x)(rd_buf = %s).\n",
				 __FUNCTION__, vtg_p->volt_p, rc, rd_buf);
		rc = FAILED;
		break;
	    }

	    /* Read Minimum voltage */
	    i2c_if.offset = vtg_p->min_o;
	    i2c_if.buf = (char *)&min;
	    rc = pwr_read(&i2c_if, &rd_buf[0]);
	    if (rc != PASSED) {
		sprintf(err_buf, "%s Min read failed. %s",
				  vtg_p->volt_p, rd_buf);
		rc = FAILED;
		break;
	    }

	    /* Got all voltages. Check Real Time is within Min and Max */
	    if ((rt > max) || (rt < min)) {
		sprintf(err_buf, "%s: %s registers not updated. Max = %#x "
				 "Min = %#x. RT = %#x.\n",
                                 __FUNCTION__,  vtg_p->volt_p, max, min, rt);
		rc = FAILED;
		break;
	    }

	    /* Check Max and Min within spec */
	    /* From the HFS (EDCS-618748 rev 4.0), section 3.8 third paragraph,
	     * "The diagnostics should note that typically the 12V minimum
	     * reading will be much lower if the system has been power cycled
	     * (which is not a failed condition) so the 12V minimum value
	     * should be ignored (no checking performed)."
	     */
	    if (max > vtg_p->over_v) {
		sprintf(err_buf, "%s Max = %#x is over spec (%#x). \n"
				 "Min = %#x. Max = %#x. Real Time = %#x.",
				 vtg_p->volt_p, max, vtg_p->over_v,
				 min, max, rt);
		rc = FAILED;
		break;
	    }

	    if ((min < vtg_p->under_v) && (vtg_p->rt_o != PWR_SEQ_12_RT)) {
		sprintf(err_buf, "%s: %s Min = %#x is under spec (%#x). \n"
				 "Min = %#x. Max = %#x. Real Time = %#x. \n",
				 __FUNCTION__, vtg_p->volt_p, min,
                                 vtg_p->under_v, min, max, rt);
		rc = FAILED;
		break;
	    }
		
	    vtg_p++;	/* Next voltage */
	} /* endof while */

    if (rc != PASSED) {
	cterr('f', 0, err_buf);
    }

    if (submenu == TRUE) {
	prcomplete(testpass, errcount, (char *)0);
    }

    return (rc);

}

/*********************************************************************
 *
 * Function:	pwr_stat_check
 *
 * Description:	Check the status and Voltage Fault registers.
 *
 * Inputs:	i2c_if_p - Points to the I2C interface struct.
 *		err_buf - Points to the error buffer.
 *
 * Outputs:	PASSED/FAILED.
 *
 * Assumptions:
 *
 *********************************************************************
 */
static int
pwr_stat_check(char *err_buf)
{
    n2g_i2c_if_t i2c_if;
    uint32_t rc = FAILED;
    ren_t status, wr_data, fault = 0;

    /* Get Power Sequencer I2C interface structure */
    rc = get_pwr_seq_i2c_struct(&i2c_if);
    if (rc != PASSED) {
        sprintf(err_buf, "%s: Failed to get Power Sequencer"
                         " I2C structure.\n", __FUNCTION__);
        return (rc);
    }

    /* Read the Clearable Status register */
    i2c_if.offset = PWR_SEQ_STA_C;
    i2c_if.buf = (char *)&status;
    rc = pwr_read(&i2c_if, err_buf);	/* Read the status register */

    if (rc == PASSED) {
	/* Got the status register. Check for the voltage fault status bits */
	if (status & PS_STAT_VTG_FAULT_PU_MSK) {
	    /* Voltage Fault during Power up */
	    sprintf(err_buf, "%s: Voltage Fault During "
                             "Power Up(status = %#x).\n"
                             "To eliminate this issue, you can manually power down the system for\n"
                             "at least 30 seconds and check the status again.\n",
                             __FUNCTION__, status);
	    rc = PWR_SEQ_STAT_CHECK_FAIL;
	} else {
	    /* Check for 12V fault during operation */
	    if (status & PS_STAT_VTG_FAULT_OPT_MSK) {
		/* Voltage Fault during Operation */
		/* Read  the Clearable Voltage Fault register */
		i2c_if.offset = PWR_SEQ_V_F_C;
		i2c_if.buf = (char *)&fault;

		rc = pwr_read(&i2c_if, err_buf); /* Read the voltage fault
						  * register */

		if (rc != PASSED) {
		    sprintf(err_buf, "%s: Failed to read Voltage Fault register"
				     "via I2C (rc = %#x).\n", __FUNCTION__, rc);
		    rc = FAILED;
		} else {
		    if (fault == PS_VF_12V_FAULT_MSK) {
			/* Only 12V fault is not real fault */
			/* Clear the status to avoid the register dump */
			status &= (~PS_STAT_VTG_FAULT_OPT_MSK);
		    } else {
		        /* True fault */
			sprintf(err_buf, "%s: Voltage Fault During Operation"
                                         "(status = %#x).\n",
                                         __FUNCTION__, status);
                        rc = PWR_SEQ_STAT_CHECK_FAIL;
		    } /* endof if fault */
		} /* endof if rc of pwr_read */
	    } /* endof if 12v check */
	} /* endof if voltage fault check */

	/* If voltage fault, clear both status and fault registers */
	if (fault && (rc == PASSED)) {
	    /* Voltage fault */
	    i2c_if.offset = PWR_SEQ_STA_C;
	    wr_data = DSWAP2(status);
	    i2c_if.buf = (char *)&wr_data;

	    rc = pwr_write(&i2c_if);

	    if (rc == PASSED) {
		i2c_if.offset = PWR_SEQ_V_F_C;
		wr_data = DSWAP2(fault);
		msleep(REN_I2C_PROC_TIME);

    rc = pwr_write(&i2c_if);
	    } else {
		sprintf(err_buf, "%s: Unable to clear Pwr Seq Status register"
				  "(rc = %#x).\n", __FUNCTION__, rc);
		rc = FAILED;
	    } /* endof if rc */
	} /* endof if fault */

#ifndef WD_BYPASS
	if (status & PWR_SEQ_STAT_WDOG) {
	    sprintf(err_buf, "%s: Power Sequencer WD status = %#x.\n",
                             __FUNCTION__, status);
	    rc = PWR_SEQ_STAT_CHECK_FAIL;
	}
#endif /* WD_BYPASS */
    } else {
	sprintf(err_buf, "%s: Failed to read Power sequencer Status register"
                         " via I2C (rc = %#x).\n", __FUNCTION__, rc);
    }

    return(rc);

}


/*******************************************************************************
 *
 * Function   : ovld_poe_psu_pwr_control
 * Description:	Function to control Overlord PoE PSU power by set/unset
 *              bit 4 of power sequencer power control register, 0x48.
 *              (1: Enable, 0: Disable)
 * Inputs     : option - ENABLE/DISABLE
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
uint32_t ovld_poe_psu_pwr_control (uint32_t option)
{
    uint16_t data = 0;
    uint32_t rc = FAILED;

    /* Read the original data out */
    if ((rc = pwr_seq_read_reg(PWR_SEQ_PWR_CTRL,  &data)) != PASSED) {
        cterr('f', 0, "%s: Failed to read POE Contorl Reg.(0x%.02x)",
              __FUNCTION__, PWR_SEQ_PWR_CTRL);
        return (rc);
    }

    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("%s:%d Original value of POE Control Reg."
               "(0x%.02x) is 0x%.04x.\n",
               __FUNCTION__, __LINE__, PWR_SEQ_PWR_CTRL, data);
    }

    /* Enable/Disable PoE PSU power based on the option */
    switch (option) {
    case ENABLE:
        data |= PS_PC_POE_PWR_CTRL;
        break;
    case DISABLE:
        data &= (uint16_t)(~(PS_PC_POE_PWR_CTRL));
        break;
    default:
        cterr('f', 0, "%s:%d Got Unknown option(%d).",
                      __FUNCTION__, __LINE__, option);
        return (FAILED);
    } 

    if ((NVRAM)->diagflag & D_VERBOSE) {
        if (option) {
            printf("%s:%d option is Enable.\n", __FUNCTION__, __LINE__);
        } else {
            printf("%s:%d option is Disable.\n", __FUNCTION__, __LINE__);
        }
        printf("%s:%d The expected write in value to POE Control Reg."
               "(0x%.02x) is 0x%.04x.\n",
               __FUNCTION__, __LINE__, PWR_SEQ_PWR_CTRL, data);
    }

    if ((rc = pwr_seq_write_reg(PWR_SEQ_PWR_CTRL, data)) != PASSED) {
	cterr('f', 0, "%s: Failed to wrote POE Control Reg.(0x%.02x)",
              __FUNCTION__, PWR_SEQ_PWR_CTRL);
    }
    return (rc);
}


/*******************************************************************************
 *
 * Function   : ovld_show_mb_watts
 * Description:	Function to show Overlord motherboard power consumption.
 * Inputs     : option - ENABLE/DISABLE
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
int ovld_show_mb_watts (void)
{
    int      rc = FAILED;
    uint16_t iadc = 0, vadc = 0;

    /* Get Power Sequencer reversion */
    rc = get_pwr_seq_fw_rev(0);
    if (rc != PASSED) {
        cterr('f', 0, "%s:%d Failed to get Power Sequencer FW revision",
              __FUNCTION__, __LINE__);
        return (FAILED);
    }

    rc = get_pwr_seq_iadc(&iadc);
    if (rc != PASSED) {
        cterr('f', 0, "%s:%d Unable to read iadc from Power Sequencer",
              __FUNCTION__, __LINE__);
        return (FAILED);
    }

    rc = get_pwr_seq_vadc(&vadc);
    if (rc != PASSED) {
        cterr('f', 0, "%s:%d Unable to read vadc from Power Sequencer",
              __FUNCTION__, __LINE__);
        return (FAILED);
    }

    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("CONST_P = %d. iadc = %d, vadc = %d.\n",
               OVLD_PWR_CONSUMP_CONST, iadc, vadc);
    }

    printf("\nMotherboard, NGWIC, NGVM, and FanTray Power consumption = %d mW\n",
           OVLD_PWR_CONSUMP_CONST * vadc * iadc / 1000);

    return (PASSED);
}


/*------------------------------------------------------------------
$Log: platform_pwr_seq.c,v $
Revision 1.4  2013/11/26 08:40:36  hroni
fix compiler warning

Revision 1.3  2013/07/03 10:25:10  danchung
Add description in power sequencer status check.

Revision 1.2  2013/05/23 01:09:26  palin2
Improved error print-out of Overlord I2C device related tests.

Revision 1.1  2013/05/09 05:42:37  alpeng
moving overlord common code from x86

Revision 1.11  2012/11/28 18:19:10  palin2
To make I2C utilities SubMenu more intuitive.

Revision 1.10  2012/11/21 19:47:22  palin2
Use function "do_all_menu_items" to replace "menu_exec_doall_diags".

Revision 1.9  2012/09/26 18:02:15  palin2
Uniformed the print out format of I2C devices defult tests.

Revision 1.8  2012/07/19 07:02:14  palin2
Update Register 0x48, Power Control Register.

Revision 1.7  2012/06/29 03:56:47  palin2
Update output message of function "ovld_show_mb_watts" based on HW team's request.

Revision 1.6  2012/06/26 12:18:42  palin2
Support to show the Power consumption of Overlord motherboard.

Revision 1.5  2012/06/26 03:59:45  palin2
Update registers map.

Revision 1.4  2012/05/30 16:45:03  palin2
Clean up compile warnings.

Revision 1.3  2012/04/16 15:29:26  palin2
Update 12V PoE PSU tests and utilities based on HW team's request:
1) Add "Registers test" support.
2) Add "PoE PSU" info into bootlog message.
3) Add utility to verified FPGA related PoE PSU detect function.

Revision 1.2  2012/03/28 00:38:24  mcharon
remove forward slash from second line

Revision 1.1.1.1  2012/03/23 23:02:10  ptong
Initial archive of ng_diag module


$Endlog$
*/
