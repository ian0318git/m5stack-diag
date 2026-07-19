/* $Id: platform_vtg_mntr.c,v 1.3 2013/11/26 08:40:37 hroni Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/overlord/platform_vtg_mntr.c,v $
 *------------------------------------------------------------------
 * Filename:	platform_vtg_mntr.c
 * 
 * Description: Operation-Overlord Voltage Monitor I2C device.
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
 * Copyright (c) 2013 by Cisco Systems, Inc.
 * All rights reserved.
 * 
 *  Notes: this voltage monitor is porting from 
 *         Informer platform_pwr_seq.c 
 *         
 *
 *------------------------------------------------------------------
 */
#include <stdio.h>
#include <string.h>
#include "endians.h"
#include "common.h"
#include "nvmonvars.h"
#include "platform_vtg_mntr.h"
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
extern int do_all_menu_items(struct menuinfo *);
extern uint32 cterr_db_print(char *fmtptr, ...);

static void show_volt_margin(ren_t vmc, uint option, int format);
static int get_vtg_mntr_i2c_struct (n2g_i2c_if_t *pwr_seq_i2c);
static int show_reg(void);
static int alter_reg(void);
static int vtg_mntr_read_reg(int, uint16_t *);
static int vtg_mntr_write_reg(int, uint16_t);
static int vm_read(n2g_i2c_if_t *i2c_if, char *err_buf);
static int vm_write(n2g_i2c_if_t *i2c_if);
static int vtg_mntr_reg_test(int submenu);
static int vtg_mntr_stat_test(int submenu);
static int vtg_mntr_ctr_test(int submenu);
static int vtg_mntr_vtg_check(int submenu);
static int vtg_mntr_stat_check(char *err_buf);
static int vtg_mntr_mrgin_test(int menu);
/* static int get_vtg_mntr_rev(void); */
/* static int get_reg(ren_o offset, ren_t *reg); */
static int pwr_rd(ulong addr, int size, ulong *buf, void *param);
static int pwr_wr(ulong addr, int size, ulong data, void *param);


/* Global variables */
static reg_info_t_ext reg_ext = {
	sizeof(ren_t), pwr_rd, pwr_wr, 0};

static reg_info_t vtg_mntr_table[]=
{
/*  Register name,		Offset,		Type, Size,
 *		Mask, Reset Value
 */
    {"Firmware Revision",	VTG_MNTR_REV,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(ulong)&reg_ext},
	0xFFFF, 0x0001},
    {"Status - Sticky",		VTG_MNTR_STA_S,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(ulong)&reg_ext},
	0x03FF, 0},
    {"Status - Clearable",	VTG_MNTR_STA_C,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(ulong)&reg_ext},
	0x03FF, 0},
    {"Voltage Fault - Sticky",	VTG_MNTR_V_F_S,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(ulong)&reg_ext},
	0x0FFF, 0},
    {"Voltage Fault - Clearable", VTG_MNTR_V_F_C,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(ulong)&reg_ext},
	0x0FFF, 0},
    {"Run Time Counter Word 2 (MS)", VTG_MNTR_RTC_2,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(ulong)&reg_ext},
	0xFFFF, 0},
    {"Run Time Counter Word 1",	VTG_MNTR_RTC_1,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(ulong)&reg_ext},
	0xFFFF, 0},
    {"Run Time Counter Word 0 (LS)", VTG_MNTR_RTC_0,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(ulong)&reg_ext},
	0xFFFF, 0},
    {"3.0 Volts, Real Time",	VTG_MNTR_3_0_RT,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(ulong)&reg_ext},
	0x03FF, 0},
    {"3.0 Volts, Maximum",	VTG_MNTR_3_0_MAX,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(ulong)&reg_ext},
	0x03FF, 0},
    {"3.0 Volts, Minimum",	VTG_MNTR_3_0_MIN,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(ulong)&reg_ext},
	0x03FF, 0},
    {"2.5 Volts, Real Time",	VTG_MNTR_2_5_RT,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(ulong)&reg_ext},
	0x03FF, 0},
    {"2.5 Volts, Maximum",	VTG_MNTR_2_5_MAX,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(ulong)&reg_ext},
	0x03FF, 0},
    {"2.5 Volts, Minimum",	VTG_MNTR_2_5_MIN,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(ulong)&reg_ext},
	0x03FF, 0},
    {"1.8 Volts, Real Time",	VTG_MNTR_1_8_RT,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(ulong)&reg_ext},
	0x03FF, 0},
    {"1.8 Volts, Maximum",	VTG_MNTR_1_8_MAX,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(ulong)&reg_ext},
	0x03FF, 0},
    {"1.8 Volts, Minimum",	VTG_MNTR_1_8_MIN,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(ulong)&reg_ext},
	0x03FF, 0},
    {"1.5 TX QLM, Real Time",	VTG_MNTR_1_5_TX_RT,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(ulong)&reg_ext},
	0x03FF, 0},
    {"1.5 TX QLM, Maximum",	VTG_MNTR_1_5_TX_MAX,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(ulong)&reg_ext},
	0x03FF, 0},
    {"1.5 TX QLM, Minimum",	VTG_MNTR_1_5_TX_MIN,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(ulong)&reg_ext},
	0x03FF, 0},
    {"1.2 Volts, Real Time",    VTG_MNTR_1_2_RT,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(ulong)&reg_ext},
	0x03FF, 0},
    {"1.2 Volts, Maximum",	VTG_MNTR_1_2_MAX,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(ulong)&reg_ext},
	0x03FF, 0},
    {"1.2 Volts, Minimum",	VTG_MNTR_1_2_MIN,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(ulong)&reg_ext},
	0x03FF, 0},
    {"1.15 Volts, Real Time", VTG_MNTR_1_15_RT,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(ulong)&reg_ext},
	0x03FF, 0},
    {"1.15 Volts, Maximum",   VTG_MNTR_1_15_MAX,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(ulong)&reg_ext},
	0x03FF, 0},
    {"1.15 Volts, Minimum",   VTG_MNTR_1_15_MIN,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(ulong)&reg_ext},
	0x03FF, 0},
    {"1.1 Volts, Real Time",  VTG_MNTR_1_1_RT,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(ulong)&reg_ext},
	0x03FF, 0},
    {"1.1 Volts, Maximum",    VTG_MNTR_1_1_MAX,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(ulong)&reg_ext},
	0x03FF, 0},
    {"1.1 Volts, Minimum",    VTG_MNTR_1_1_MIN,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(ulong)&reg_ext},
	0x03FF, 0},
    {"1.xx VCORE ICPU, Real Time", VTG_MNTR_1_XX_RT,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(ulong)&reg_ext},
	0x03FF, 0},
    {"1.xx VCORE ICPU, Maximum", VTG_MNTR_1_XX_MAX,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(ulong)&reg_ext},
	0x03FF, 0},
    {"1.xx VCORE ICPU, Minimum", VTG_MNTR_1_XX_MIN,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(ulong)&reg_ext},
	0x03FF, 0},
    {"1.0 Volts, Real Time",     VTG_MNTR_1_0_RT,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(ulong)&reg_ext},
	0x03FF, 0},
    {"1.0 Volts, Maximum", VTG_MNTR_1_0_MAX,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(ulong)&reg_ext},
	0x03FF, 0},
    {"1.0 Volts, Minimum", VTG_MNTR_1_0_MIN,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(ulong)&reg_ext},
	0x03FF, 0},
    {"VTT SA ICPU, Real Time",     VTG_MNTR_SA_RT,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(ulong)&reg_ext},
	0x03FF, 0},
    {"VTT SA ICPU, Maximum", VTG_MNTR_SA_MAX,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(ulong)&reg_ext},
	0x03FF, 0},
    {"VTT SA ICPU, Minimum", VTG_MNTR_SA_MIN,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(ulong)&reg_ext},
	0x03FF, 0},
    {"0.75V VTT ICPU, Real Time",     VTG_MNTR_0_75_I_RT,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(ulong)&reg_ext},
	0x03FF, 0},
    {"0.75V VTT ICPU, Maximum", VTG_MNTR_0_75_I_MAX,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(ulong)&reg_ext},
	0x03FF, 0},
    {"0.75V VTT ICPU, Minimum", VTG_MNTR_0_75_I_MIN,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(ulong)&reg_ext},
	0x03FF, 0},
    {"0.75V VTT CCPU, Real Time",     VTG_MNTR_0_75_C_RT,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(ulong)&reg_ext},
	0x03FF, 0},
    {"0.75V VTT CCPU, Maximum", VTG_MNTR_0_75_C_MAX,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(ulong)&reg_ext},
	0x03FF, 0},
    {"0.75V VTT CCPU, Minimum", VTG_MNTR_0_75_C_MIN,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(ulong)&reg_ext},
	0x03FF, 0},
    {"3.0V Ramp Time", VTG_MNTR_3_0_RPT,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(ulong)&reg_ext},
	0xFFFF, 0},
    {"2.5V Ramp Time", VTG_MNTR_2_5_RPT,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(ulong)&reg_ext},
	0xFFFF, 0x0900},
    {"1.8V Ramp Time", VTG_MNTR_1_8_RPT,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(ulong)&reg_ext},
	0xFFFF, 0x0900},
    {"1.5V TX QLM Ramp Time", VTG_MNTR_1_5_TX_RPT,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(ulong)&reg_ext},
	0xFFFF, 0x0900},
    {"1.2V Ramp Time", VTG_MNTR_1_2_RPT,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(ulong)&reg_ext},
	0xFFFF, 0x0900},
    {"1.15V Ramp Time", VTG_MNTR_1_15_RPT,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(ulong)&reg_ext},
	0xFFFF, 0x0C00},
    {"1.1V Ramp Time", VTG_MNTR_1_1_RPT,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(ulong)&reg_ext},
	0xFFFF, 0x0900},
    {"1.xxV VCORE ICPU Ramp Time", VTG_MNTR_1_XX_RPT,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(ulong)&reg_ext},
	0xFFFF, 0x0900},
    {"0.75V VTT CCPU Ramp Time", VTG_MNTR_0_75_C_RPT,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(ulong)&reg_ext},
	0xFFFF, 0x0900},
    {"Enable Double Sampling",	VTG_MNTR_EN_DS,
	READ_WRITE | SAVE_RESTORE | REG_ACCESS, {(ulong)&reg_ext},
	0xFFDF, 0x0000},
    {"Voltage Margin Control",	VTG_MNTR_MRGN_CTRL,
	READ_WRITE | SAVE_RESTORE | REG_ACCESS, {(ulong)&reg_ext},
	0x000F, 0x0000},
    {"Scratch Pad 0",		VTG_MNTR_SCR0,
	READ_WRITE | SAVE_RESTORE | REG_ACCESS, {(ulong)&reg_ext},
	0xFFFF, 0x0000},
    {"Scratch Pad 1",		VTG_MNTR_SCR1,
	READ_WRITE | SAVE_RESTORE | REG_ACCESS, {(ulong)&reg_ext},
	0xFFFF, 0x0000},
    {"Scratch Pad 2",		VTG_MNTR_SCR2,
	READ_WRITE | SAVE_RESTORE | REG_ACCESS, {(ulong)&reg_ext},
	0xFFFF, 0x0000},
    {"Scratch Pad 3",		VTG_MNTR_SCR3,
	READ_WRITE | SAVE_RESTORE | REG_ACCESS, {(ulong)&reg_ext},
	0xFFFF, 0x0000},
    {"Scratch Pad 4",		VTG_MNTR_SCR4,
	READ_WRITE | SAVE_RESTORE | REG_ACCESS, {(ulong)&reg_ext},
	0xFFFF, 0x0000},
    {"Scratch Pad 5",		VTG_MNTR_SCR5,
	READ_WRITE | SAVE_RESTORE | REG_ACCESS, {(ulong)&reg_ext},
	0xFFFF, 0x0000},
    {"Reserved 0", VTG_MNTR_RSV0,
	READ_WRITE | SAVE_RESTORE | REG_ACCESS, {(ulong)&reg_ext},
	0x0000, 0},
    {"Reserved 1", VTG_MNTR_RSV1,
	READ_WRITE | SAVE_RESTORE | REG_ACCESS, {(ulong)&reg_ext},
	0x0000, 0},
    {"Reserved 2", VTG_MNTR_RSV2,
	READ_WRITE | SAVE_RESTORE | REG_ACCESS, {(ulong)&reg_ext},
	0x0000, 0},
    {"Reserved 3", VTG_MNTR_RSV3,
	READ_WRITE | SAVE_RESTORE | REG_ACCESS, {(ulong)&reg_ext},
	0x0000, 0},
    {"Reserved 4", VTG_MNTR_RSV4,
	READ_WRITE | SAVE_RESTORE | REG_ACCESS, {(ulong)&reg_ext},
	0x0000, 0},
    {"Reserved 5", VTG_MNTR_RSV5,
	READ_WRITE | SAVE_RESTORE | REG_ACCESS, {(ulong)&reg_ext},
	0x0000, 0},
    {"Reserved 6", VTG_MNTR_RSV6,
	READ_WRITE | SAVE_RESTORE | REG_ACCESS, {(ulong)&reg_ext},
	0x0000, 0},
    {"End", 0xFFFFFFFF, READ_ONLY, {0}, 0, 0},
};

static vtg_mntr_v_t voltage_test_table[] =
{
    {"3.0 Volts",        OVER_ADC_3_0V,       UNDER_ADC_3_0V,
     VTG_MNTR_3_0_RT,    VTG_MNTR_3_0_MAX,    VTG_MNTR_3_0_MIN},
    {"2.5 Volts",        OVER_ADC_2_5V,       UNDER_ADC_2_5V,
     VTG_MNTR_2_5_RT,    VTG_MNTR_2_5_MAX,    VTG_MNTR_2_5_MIN},
    {"1.8 Volts",        OVER_ADC_1_8V,       UNDER_ADC_1_8V,
     VTG_MNTR_1_8_RT,    VTG_MNTR_1_8_MAX,    VTG_MNTR_1_8_MIN},
    {"1.5 Volts TX QLM", OVER_ADC_1_5V,       UNDER_ADC_1_5V,
     VTG_MNTR_1_5_TX_RT, VTG_MNTR_1_5_TX_MAX, VTG_MNTR_1_5_TX_MIN},
    {"1.2 Volts",        OVER_ADC_1_2V,       UNDER_ADC_1_2V,
     VTG_MNTR_1_2_RT,    VTG_MNTR_1_2_MAX,    VTG_MNTR_1_2_MIN},
    {"1.15 Volts",       OVER_ADC_1_15V,      UNDER_ADC_1_15V,
     VTG_MNTR_1_15_RT,   VTG_MNTR_1_15_MAX,   VTG_MNTR_1_15_MIN},
    {"1.1 Volts",        OVER_ADC_1_1V,       UNDER_ADC_1_1V,
     VTG_MNTR_1_1_RT,    VTG_MNTR_1_1_MAX,    VTG_MNTR_1_1_MIN},
    {"VCORE ICPU",       OVER_ADC_VCORE,      UNDER_ADC_VCORE,
     VTG_MNTR_1_XX_RT,   VTG_MNTR_1_XX_MAX,   VTG_MNTR_1_XX_MIN},
    {"1.0 Volts",        OVER_ADC_1_0V,       UNDER_ADC_1_0V,
     VTG_MNTR_1_0_RT,    VTG_MNTR_1_0_MAX,    VTG_MNTR_1_0_MIN},
    {"VTT SA ICPU",      OVER_ADC_SA,         UNDER_ADC_SA,
     VTG_MNTR_SA_RT,     VTG_MNTR_SA_MAX,     VTG_MNTR_SA_MIN},
    {"0.75V VTT ICPU",   OVER_ADC_0_75V_I,    UNDER_ADC_0_75V_I,
     VTG_MNTR_0_75_I_RT, VTG_MNTR_0_75_I_MAX, VTG_MNTR_0_75_I_MIN},
    {"0.75V VTT CCPU",   OVER_ADC_0_75V_C,    UNDER_ADC_0_75V_C,
     VTG_MNTR_0_75_C_RT, VTG_MNTR_0_75_C_MAX, VTG_MNTR_0_75_C_MIN},
    {NULL, 0, 0, 0, 0, 0},
};


/*
 * Voltage Monitor Menu
 */
static submenu_xtable_t vtg_mntr_menu_table[] = {
    {"Show Voltage Monitor registers", (PFT)show_reg,               0,
        0,                             (type_t(*)())0, 0, (PFT)0, 0},
    {"Alter Voltage Monitor register", (PFT)alter_reg,              0,
        0,                             (type_t(*)())0, 0, (PFT)0, 0},
    {"Initialize voltage monitor",     (PFT)init_vtg_mntr,       TRUE,
        0,                             (type_t(*)())0, 0, (PFT)0, 0},
    {"Registers Test",                 (PFT)vtg_mntr_reg_test,   TRUE,
        (MF_CONTINUOUS | MF_DOALL),    (type_t(*)())0, 0, (PFT)0, 0},
    {"Status Check",                   (PFT)vtg_mntr_stat_test,  TRUE,
        (MF_CONTINUOUS | MF_DOALL),    (type_t(*)())0, 0, (PFT)0, 0},
    {"Run Time Counter Test",          (PFT)vtg_mntr_ctr_test,   TRUE,
        (MF_CONTINUOUS | MF_DOALL),    (type_t(*)())0, 0, (PFT)0, 0},
    {"Voltage Check",                  (PFT)vtg_mntr_vtg_check,  TRUE,
        (MF_CONTINUOUS | MF_DOALL),    (type_t(*)())0, 0, (PFT)0, 0},
    {"Voltage Margin I2C test",        (PFT)vtg_mntr_mrgin_test, TRUE,
        (MF_CONTINUOUS | MF_DOALL),    (type_t(*)())0, 0, (PFT)0, 0},
};

#define VTG_MNTR_MENU_TABLE_SIZE (sizeof(vtg_mntr_menu_table) / sizeof(submenu_xtable_t))

/*
 * Primary & secondary submenu items (filled in from xtable)
 */
static mitem_t vtg_mntr_menu_primary_items[VTG_MNTR_MENU_TABLE_SIZE + MAX_BASE_ITEMS];
static mitem_t vtg_mntr_menu_secondary_items[VTG_MNTR_MENU_TABLE_SIZE + MAX_BASE_ITEMS];

static struct menuinfo pwrdiag = {
    "Voltage Monitor Utility Menu",	/* title */
    0,				/* title string added by init_empty_menu */
    (PFT)menu_show_dflags,	/* shows major flags */
    0,				/* generic prompt */
    0,				/* size -- bumped by add_menu_item() */
    vtg_mntr_menu_primary_items,
};

static struct menuinfo *pwrdiagp = &pwrdiag;

typedef struct pwr_mrgin_t_{
    char *name; /* Margin string */
    ren_t mask; /* test voltage margin register */
} pwr_mrgin_t;

static pwr_mrgin_t
pwr_mrgn_tbl[] = {
        {"1.8/1.5V No margin", 0},
        {"1.8/1.5V High",      VTG_MNTR_DDR_MRGN_EN | VTG_MNTR_DDR_MRGN},
        {"1.8/1.5V Low",       VTG_MNTR_DDR_MRGN_EN},
        {"3.0/3.0V No margin", 0},
        {"3.0/3.0V High",      VTG_MNTR_3_3V_MRGN_EN | VTG_MNTR_3_3V_MRGN},
        {"3.0/3.0V Low",       VTG_MNTR_3_3V_MRGN_EN},
    };

#define PWR_MRGN_LOOP 1
#define PWR_MRGN_TBL_SIZE (sizeof(pwr_mrgn_tbl) / sizeof(pwr_mrgin_t))


/*******************************************************************************
 *
 * Function   : get_vtg_mntr_i2c_struct
 * Description: To get Voltage Mointor I2C interface structure.
 * Inputs     : Pointer to save the gotten I2C interface structure
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
static int get_vtg_mntr_i2c_struct (n2g_i2c_if_t *pwr_seq_i2c)
{
    n2g_i2c_if_t *tmp;

    /* init i2c_if for I2C */
    tmp = (n2g_i2c_if_t *)get_n2g_i2c_if(I2C_CTRL_TWO, I2C_MUX_ZERO, 
                                         MB_I2C_ADDR_VTG_MNTR);

    if (tmp == NULL) {
        printf("%s: Failed to get Voltage Mointor I2C interface structure.\n",
               __FUNCTION__);
        return (FAILED);
    }

    memcpy(pwr_seq_i2c, tmp, sizeof(n2g_i2c_if_t));

    return (PASSED);
}

/**********************************************************************
 *
 * function:	build_vtg_mntr_menu
 *
 * Description:	Build Voltage Monitor menu.
 *
 * Input:	None.
 *
 * Output:	None.
 *
 **********************************************************************
 */
void build_vtg_mntr_menu (int menu_opt)
{

    build_primary_submenu(vtg_mntr_menu_table, VTG_MNTR_MENU_TABLE_SIZE,
			  "Voltage Monitor Utility Menu", &pwrdiagp);
    build_secondary_submenu(vtg_mntr_menu_table, VTG_MNTR_MENU_TABLE_SIZE,
			    vtg_mntr_menu_secondary_items);

    if (menu_opt) {
        /* Entered with submenu */
        menu(&pwrdiag, vtg_mntr_menu_secondary_items, 0);
    } else {
        do_all_menu_items(pwrdiagp);
    }
}


/*********************************************************************
 *
 * Function:	vtg_mrgn
 *
 * Description:	Margin read/write.
 *
 * Inputs:	option - Refer to voltage_margin_t enum.
 *              format - DISPLAY_HCI /DISPLAY_M2M
 *
 * Outputs:	PASSED/FAILED.
 *
 * Assumptions:
 *
 *********************************************************************
 */
int vtg_mrgn(int option, int format)
{
    uint32_t rc = FAILED;
    n2g_i2c_if_t i2c_if;
    uint32_t old_data, new_data; 
    char err_buf[ERR_BUF_SIZE];

    /* Get Voltage Monitor I2C interface structure */
    rc = get_vtg_mntr_i2c_struct(&i2c_if);
    if (rc != PASSED) {
        printf("%s: Failed to get Voltage Monitor I2C structure.\n",
              __FUNCTION__);
        return (rc);
    }
    
    i2c_if.buf = (char *)&old_data;
    i2c_if.offset = VTG_MNTR_MRGN_CTRL;    
  
    /* read to check the register on offset is available or not */
    rc = vm_read(&i2c_if, &err_buf[0]);   
    if (rc != PASSED) {
	/* Unable to read data */
	cterr('f', 0, "vtg_mrgn() VMC read - %s", err_buf);
	return(FAILED);
    }
  
#ifdef VM_DEBUG
  printf("read from reg:pwr_seq_ctrl:  old_data = %d \n", old_data);
#endif 

    switch(option) {
    case VTG_MRGN_GET_DDR:
	new_data = old_data;
	show_volt_margin(old_data, option, format);
	break;
    case VTG_MRGN_GET_3_3V:
	new_data = old_data;
	show_volt_margin(old_data, option, format);
	break;
    case VTG_MRGN_SET_DDR_NORM:
        new_data = old_data & (~VTG_MNTR_DDR_MRGN_EN);
	break;
    case VTG_MRGN_SET_DDR_HI:
	new_data = (old_data | VTG_MNTR_DDR_MRGN | VTG_MNTR_DDR_MRGN_EN);
	break;
    case VTG_MRGN_SET_DDR_LO:
	new_data = old_data & (~VTG_MNTR_DDR_MRGN);
	new_data |= VTG_MNTR_DDR_MRGN_EN;
	break;
    case VTG_MRGN_SET_3_3V_NORM:
	new_data = old_data & (~VTG_MNTR_3_3V_MRGN_EN);
	break;
    case VTG_MRGN_SET_3_3V_HI:
	new_data = (old_data | VTG_MNTR_3_3V_MRGN | VTG_MNTR_3_3V_MRGN_EN);
	break;
    case VTG_MRGN_SET_3_3V_LO:
	new_data = old_data & (~VTG_MNTR_3_3V_MRGN);
	new_data |= VTG_MNTR_3_3V_MRGN_EN;
	break;
    default:
	new_data = old_data;
	cterr('f', 0, "vtg_mrgn() Invalid option %#x", option);
	rc = FAILED;
	break;
    } /* endof switch */


    if (new_data != old_data) {  
  printf("write to reg:pwr_seq_ctrl: new_data = %d\n",  new_data);
  i2c_if.buf = (char *)&new_data;    
  rc = vm_write(&i2c_if);
    }

    if ((new_data != old_data) && (rc != PASSED)) {
	cterr('f', 0, "vtg_mrgn() Write VMC register failed. rc = %#x", rc);
    }
    
    return (rc);
}


/*********************************************************************
 *
 * Function:	show_volt_margin
 *
 * Description:	Display voltage margin.
 *
 * Inputs:	vmc - Voltage Margin Control register read.
 *		option - 1.8 v or 2.5/3.3v Use voltage_margin_t enum.
 *              format - DISPLAY_HCI /DISPLAY_M2M
 *
 * Outputs:	None
 *
 *********************************************************************
 */
static void show_volt_margin(ren_t vmc, uint option, int format)
{
    switch(option) {
    case VTG_MRGN_GET_DDR:
        if (format == DISPLAY_HCI) {
            /* show in human computer interface */
	    printf("1.8/1.5V DDR ");
        }else 
        if (format == DISPLAY_M2M) {
            /* show in machine to machine */
            printf("1.8_1.5_DDR:");
        } else if (format == DISPLAY_CTERR) {
            /* show cterr */
            cterr_db_print("1.8_1.5_DDR:");
        
        }else {
            prpass(testpass, "%s()",__func__);
            cterr('f',0,"Unknown format in %s function,"
                "\nSupport DISPLAY_HCI or DISPLAY_M2M,"
                "\nbut received format = %x ", __func__, format); 
            return;
        }
	if (vmc & VTG_MNTR_DDR_MRGN_EN) {
        switch(format){
            case DISPLAY_CTERR:
                /* show in CTERR */
                cterr_db_print("margined ");
                if (vmc & VTG_MNTR_DDR_MRGN) {
                    cterr_db_print("+3%%\n");
                } else {
                    cterr_db_print("-3%%\n");
                }
                break;
            case DISPLAY_HCI:
                /* show in human computer interface */
                printf("margined ");
            default:
                if (vmc & VTG_MNTR_DDR_MRGN) {
                    printf("+3%%\n");
                } else {
                    printf("-3%%\n");
                }
                break;
        }
	} else {
            if (format == DISPLAY_HCI) {
                /* show in human computer interface */
	        /* Normal */
	        printf("not margined\n");
            }else 
            if (format == DISPLAY_M2M) {
                /* show in machine to machine */
                printf("MARGIN NO\n");
            }else 
            if (format == DISPLAY_CTERR) {
                /* show in CTERR */
                cterr_db_print("MARGIN NO\n");
            }else {
                prpass(testpass, "%s()",__func__);
                cterr('f',0,"Unknown format in %s function,"
                    "\nSupport DISPLAY_HCI or DISPLAY_M2M,"
                    "\nbut received format = %x ", __func__, format); 
                return;
            }
	}
	break;
   case VTG_MRGN_GET_3_3V:
        if (format == DISPLAY_HCI) {
            /* show in human computer interface */
            printf("3.3V/3.0V ");
        }else 
        if (format == DISPLAY_M2M) {
            /* show in machine to machine */
            printf("VOLT_3.3_3.0:");
        }else 
        if (format == DISPLAY_CTERR) {
            /* show in CTERR */
            cterr_db_print("VOLT_3.3_3.0:");
        }else {
            prpass(testpass, "%s()",__func__);
            cterr('f',0,"Unknown format in %s function,"
                "\nSupport DISPLAY_HCI or DISPLAY_M2M,"
                "\nbut received format = %x ", __func__, format); 
            return;
        }
	if (vmc & VTG_MNTR_3_3V_MRGN_EN) {
        switch(format){
            case DISPLAY_CTERR:
                /* show in CTERR */
                cterr_db_print("margined ");
                if (vmc & VTG_MNTR_3_3V_MRGN) {
                    cterr_db_print("+3%%\n");
                } else {
                    cterr_db_print("-3%%\n");
                }
                break;
            case DISPLAY_HCI:
                /* show in human computer interface */
                printf("margined ");
            default:
                if (vmc & VTG_MNTR_3_3V_MRGN) {
                    printf("+3%%\n");
                } else {
                    printf("-3%%\n");
                }
                break;
        }
	} else {
            if (format == DISPLAY_HCI) {
                /* show in human computer interface */
	        /* Normal */
	        printf("not margined\n");
            } else 
            if (format == DISPLAY_M2M) {
                /* show in machine to machine */
                printf("MARGIN NO\n");
            } else 
            if (format == DISPLAY_CTERR) {
                /* show in cterr */
                cterr_db_print("MARGIN NO\n");
            }else {
                prpass(testpass, "%s()",__func__);
                cterr('f',0,"Unknown format in %s function,"
                    "\nSupport DISPLAY_HCI or DISPLAY_M2M,"
                    "\nbut received format = %x ", __func__, format); 
                return;
            }
	}
	break;
    default:
	break;
    }
}

/**********************************************************************
 *
 * Function:	init_vtg_mntr
 *
 * Description:	Initilize Voltage Monitor
 *
 * Inputs:	err_log - cterr if TRUE. printf if FALSE.
 *
 * Outputs:	PASSED/FAILED.
 *
 **********************************************************************
 */
int init_vtg_mntr(int err_log)
{
    n2g_i2c_if_t i2c_if;
    uint32_t rc = FAILED;
    ren_t data;
    reg_info_t *reg_table_p;
    char err_buf[ERR_BUF_SIZE];
    uint ia;
    char *reg_data;
    char reg_tmp[VTG_MNTR_BUF_SIZE];

    /* Get Voltage Monitor I2C interface structure */
    rc = get_vtg_mntr_i2c_struct(&i2c_if);
    if (rc != PASSED) {
        printf("%s: Failed to get Voltage Monitor I2C structure.\n",
              __FUNCTION__);
        return (rc);
    }
    
    /* init the dev, i2c parameters and buf.*/
    reg_data = &reg_tmp[0];

    /* Read the Revision register */
    i2c_if.offset = VTG_MNTR_REV;
    i2c_if.buf = reg_data;
    
    rc = vm_read(&i2c_if, &err_buf[0]);
    if (rc != PASSED) {
  if (err_log == FALSE) {
      printf("\n*** init_vtg_mntr() Unable to read Rev reg\n");
  } else {
      cterr('f', 0, "init_vtg_mntr() Rev reg read failed");
  } /* endof if err_log */
  return(FAILED);
    }
    
    
    data = ((reg_tmp[1] << 8) | reg_tmp[0]);
    printf("\n Voltage Monitor version %d.%02d\n",
				(data & VTG_MNTR_REV_MAJOR) >> 8,
				data & VTG_MNTR_REV_MINOR);
				

    /* Display the voltage margins */
    i2c_if.offset = VTG_MNTR_MRGN_CTRL;
    rc = vm_read(&i2c_if, &err_buf[0]);
    if (rc != PASSED) {
  if (err_log == FALSE) {
      printf("\n*** init_vtg_mntr() Unable to read reg. rc = %#x\n",
              rc);
  } else {
      cterr('f', 0, "init_vtg_mntr() Unable to read reg. rc = %#x",
              rc);
  } /* endof if err_log */
  return(FAILED);
    }


    /* Xformers Power Sequencer requires the status register to be read. */
    rc = vtg_mntr_stat_check(&err_buf[0]);	/* Check the status register */

    if ((rc == FAILED) || (rc == VTG_MNTR_STAT_CHECK_FAIL)) {
  /* Registers read/write failure */
  if (err_log == FALSE) {
      printf("\n***** %s ******\n", err_buf);
  } 

  if (rc == VTG_MNTR_STAT_CHECK_FAIL) {
      /* Dump out the registers per spec */
      /* Do not use pwr_show_reg() since it creates and destroys the
       * device object
       */
    for (ia = 0, reg_table_p = &vtg_mntr_table[0];
  ia < (sizeof(vtg_mntr_table) / sizeof(reg_info_t));
  ia++, reg_table_p++) {
    
    i2c_if.offset = reg_table_p->offset;
    
    /* the byteswap is activiated in vm_read. */
    rc = vm_read(&i2c_if, &err_buf[0]);   
    if (rc != PASSED) {
	/* Unable to read data */
	cterr('f', 0, "init_vtg_mntr() read - %s", err_buf);
	return(FAILED);
    }

           
    /* if size of *reg_data is larger than size of char, 
     * the reg_tmp[1] will contain data  
     */
    printf("%s - %02x , data:0x%04x \n", reg_table_p->name,
           reg_table_p->offset, ((reg_tmp[1] << 8) | reg_tmp[0]));
           
  }

#ifdef VM_DEBUG
  } else {
      printf("\n Voltage monitor status register = %#x\n", status);
#endif /* VM_DEBUG */
  } /* endof if status */
    }

    return(rc);
}

/*********************************************************************
 *
 * Function:	show_reg
 *
 * Description:	Display Voltage Monitor Registers.
 *
 * Inputs:	None
 *
 * Outputs:	PASSED - No errors encounterd.
 *		FAILED - Errors encounterd.
 *
 * Assumptions:
 *
 *********************************************************************
 */
static int show_reg(void)
{
    uint32_t result = FAILED, offset;
    uint16_t buffer = 0;
    reg_info_t *reg_ptr;
    
    reg_ptr = &vtg_mntr_table[0];
    
    printf("\n");

    while (reg_ptr->offset != 0xFFFFFFFF) {
        /* Print all registers */ 
        offset = (reg_ptr->offset);
        if ((result = vtg_mntr_read_reg(offset, &buffer)) != RC_I2C_OP_OK) {
            cterr('f', 0, "show reg: unable to read register");
            return FAILED;
        }

        printf("%-28s (0x%02X), data: 0x%.04x\n", reg_ptr->name,
               reg_ptr->offset, buffer);      
        reg_ptr++;
    } 
    return(result);
} 


/*********************************************************************
 *
 * Function:	alter_reg
 *
 * Description:	Alter Voltage Monitor Register.
 *
 * Inputs:	None.
 *
 * Outputs:	PASSED/FAILED.
 *
 * Assumptions:
 *
 *********************************************************************
 */
static int alter_reg(void)
{
    uint32_t rc = FAILED, i, offset;
    reg_info_t *reg_table_p;
    uint16_t data = 0, tmp_mask;

    /* Setup I2C API parameter struct */
    printf("\nRegister number:\n");
    for (i = 0, reg_table_p = &vtg_mntr_table[0];
		i < (sizeof(vtg_mntr_table) / sizeof(reg_info_t));
		i++, reg_table_p++) {
	if (!(reg_table_p->type & READ_ONLY)) {
	    /* Read writeable */
	    printf("   %02x - %s\n", reg_table_p->offset,
				  reg_table_p->name);
	}
    }

    offset = gethex_answer("Enter the register number:", 0, 0,
			(sizeof(vtg_mntr_table) / sizeof(reg_info_t)) - 1);

    /* Got the register offset. Check if writeable */
    for (i = 0, reg_table_p = &vtg_mntr_table[0];
		i < (sizeof(vtg_mntr_table) / sizeof(reg_info_t));
		i++, reg_table_p++) {
	if (reg_table_p->offset != offset) {
	    continue;
	}
	/* Found the offset */
	if (reg_table_p->type & READ_ONLY) {
	    /* read only */
	    cterr('f', 0, "Read only register");
	    return(FAILED);
	}
	/* Valid offset and writeable */
	tmp_mask = reg_table_p->mask; /* get the mask for user to alter reg. */
	break;
    }

    if ((rc = vtg_mntr_read_reg(offset,  &data)) != RC_I2C_OP_OK) {
	cterr('f', 0, "Voltage Monitor read failed with return code 0x%08x", rc);
        return FAILED;
    }
    data = gethex_answer("Enter the 16-bit data:", data, 0, tmp_mask);
    if ((rc = vtg_mntr_write_reg(offset, data)) != RC_I2C_OP_OK) {
	cterr('f', 0, "Voltage Monitor write failed with return code 0x%08x", rc);
        return FAILED;
    }

    return(PASSED);
}

/*********************************************************************
 *
 * Function:	vtg_mntr_read_reg
 *
 * Description:	For show/alter register using.
 *
 * Inputs:	Offset - offset for accessing the register
 *          d32 - pass the data with 32 bits.
 *
 * Outputs:	PASSED/FAILED.
 *
 * Assumptions:
 *
 *********************************************************************
 */
static int vtg_mntr_read_reg(int offset, uint16_t *data)
{
    n2g_i2c_if_t i2c_if;
    char err_buf[ERR_BUF_SIZE];
    int result = FAILED;

    /* Get Voltage Monitor I2C interface structure */
    result = get_vtg_mntr_i2c_struct(&i2c_if);
    if (result != PASSED) {
        printf("%s: Failed to get Voltage Monitor I2C structure.\n",
               __FUNCTION__);
        return (result);
    }
    
    i2c_if.buf = (char *)data;
    i2c_if.offset = offset;
    
    result = vm_read(&i2c_if , &err_buf[0]);   

    if (result != PASSED) {
        /* Unable to read data */
        cterr('f', 0, "show_reg() Unable to read. rc = 0x%08x", result);
        return(FAILED);
    }

    return(PASSED);

}

/*********************************************************************
 *
 * Function:	vtg_mntr_write_reg
 *
 * Description:	For alter register using.
 *
 * Inputs:	Offset - offset for accessing the register
 *          d32 - pass the data with 32 bits.
 *
 * Outputs:	PASSED/FAILED.
 *
 * Assumptions:
 *
 *********************************************************************
 */
static int vtg_mntr_write_reg(int offset, uint16_t data)
{
    n2g_i2c_if_t i2c_if;
    uint32_t result = FAILED;

    /* Get Voltage Monitor I2C interface structure */
    result = get_vtg_mntr_i2c_struct(&i2c_if);
    if (result != PASSED) {
        printf("%s: Failed to get Voltage Monitor I2C structure.\n",
              __FUNCTION__);
        return (result);
    }
    
    i2c_if.buf = (char *)&data;
    i2c_if.offset = offset;
    
    result = vm_write(&i2c_if);   

    if (result != PASSED) {
        /* Unable to read data */
        cterr('f', 0, "show_reg() Unable to read. rc = 0x%08x", result);
        return(FAILED);
    }
         
    return(PASSED);


}

/*********************************************************************
 *
 * Function:	vm_read
 *
 * Description:	Local Read Voltage Monitor Register. Voltage Monitor I2C read
 *		has 2 I2C operations. The I2C write with the register offset.
 *		Then wait for the REN_I2C_PROC_TIME milliseconds to allow
 *		the Voltage Monitor firmware to setup the data of the requested
 *		register. Then the I2C read will return the data.
 *
 * Inputs:	i2c_if - pointer to the I2C API struct.
 *		err_buf - Points to the error buffer.
 *
 * Outputs:	PASSED/FAILED.
 *
 * Assumptions:
 *
 *********************************************************************
 */
static int vm_read(n2g_i2c_if_t *i2c_if, char *err_buf)
{
    uint32_t rc = FAILED;
    uint16_t reg_val = 0;

    if (!i2c_if->buf) {
        assert(!"voltage monitor read: buf is null");
    }
    
    rc = n2g_i2c_read(i2c_if);
    if (rc != RC_I2C_OP_OK) {
	/* Unable to read data */
	cterr('f', 0, "%s:%d Unable to read (rc = 0x%08x).", 
              __FUNCTION__, __LINE__, rc);
	return (FAILED);
    }

    /* Swap the read-out data */
    reg_val = DSWAP2(*(uint16_t *)i2c_if->buf);
    memcpy(i2c_if->buf, &reg_val, sizeof(uint16_t));

    msleep(REN_I2C_PROC_TIME);	/* I2C cycle time */

    return (PASSED);
}

/*********************************************************************
 *
 * Function:	vm_write
 *
 * Description:	Local Write Voltage Monitor Register. Voltage Monitor I2C write
 *		has 2 I2C operations. The I2C write with the register offset.
 *		Then wait for the REN_I2C_PROC_TIME milliseconds to allow
 *		the Voltage Monitor firmware to setup the data of the requested
 *		register. 
 *
 * Inputs:	i2c_if - pointer to the I2C API struct.
 *
 * Outputs:	PASSED/FAILED.
 *
 * Assumptions:
 *
 *********************************************************************
 */
static int vm_write(n2g_i2c_if_t *i2c_if)
{
    uint32_t rc = FAILED;
    uint16_t write_data = 0;

    if (!i2c_if->buf) {
        assert(!"vm_write: buf is null");
    }

    /* Swap data */
    write_data = DSWAP2(*(uint16_t *)i2c_if->buf);
    memcpy(i2c_if->buf, &write_data, sizeof(uint16_t));
    
    rc = n2g_i2c_write(i2c_if);
    if (rc != RC_I2C_OP_OK) {
	msleep(REN_I2C_PROC_TIME);	/* Env MCU I2C cycle time */
        return (FAILED);
    }
    msleep(REN_I2C_PROC_TIME);	/* Env MCU I2C cycle time */
    return (PASSED);
}

/*********************************************************************
 *
 * Function:	vtg_mntr_reg_test
 *
 * Description:	Test read/writeable registers.
 *
 * Inputs:	submenu - TRUE if invoked from submenu.
 *
 * Outputs:	PASSED/FAILED.
 *
 * Assumptions:
 *
 *********************************************************************
 */
static int vtg_mntr_reg_test(int submenu)
{
    n2g_i2c_if_t i2c_if;
    reg_info_t *reg_ptr;
    char err_buf[ERR_BUF_SIZE];
    uint16_t original_data, reg_data, chk_data = 0;
    uint16_t  ia, ix, ii;
    uint16_t temp = 0; 
    uint16_t data, mask, rc = FAILED;

/* since the pwr seq register is only 16 bits, 
 * the test pattern is from common.h 
 * #define PATTERN   0x5ADBA56C
 */    
#define PWR_SER_PATTERN 0xA56C

    if (submenu == TRUE) {
	testname("Voltage Monitor Registers ");
    } else {
	prpass(testpass, "Voltage Monitor Registers Test ");
    }

    /* Get Voltage Monitor I2C interface structure */
    rc = get_vtg_mntr_i2c_struct(&i2c_if);
    if (rc != PASSED) {
        printf("%s: Failed to get Voltage Monitor I2C structure.\n",
              __FUNCTION__);
        return (rc);
    }
    
    reg_ptr = &vtg_mntr_table[0];
    
    printf("\n");

    for (ia = 0, reg_ptr = &vtg_mntr_table[0];
         ia < (sizeof(vtg_mntr_table) / sizeof(reg_info_t));
         ia++, reg_ptr++) {
		
        if ((reg_ptr->type & (READ_ONLY | WRITE_ONLY)) == READ_WRITE) {
            /* clear original_data */
            original_data = 0;

            i2c_if.offset = reg_ptr->offset;       
            i2c_if.buf = (char *)&original_data;

            rc = vm_read(&i2c_if, &err_buf[0]);  
            if (rc != PASSED) {
                sprintf(err_buf, "Ripple one FAILED to read from %s.",
                        reg_ptr->name);
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

                prpass(testpass, "Ripple 1 Test: %s Reg.(0x%02x), Testpattern = 0x%04x",
                       reg_ptr->name, reg_ptr->offset, temp);

                reg_data = temp;
                i2c_if.buf = (char *)&reg_data;

                rc = vm_write(&i2c_if);
                if (rc != PASSED) {
                    sprintf(err_buf, "Ripple one FAILED on write to %s.",
                            reg_ptr->name);
                    break;
                }

                if ((NVRAM)->diagflag & D_VERBOSE) {
                    printf("temp  = 0x%x   reg_data = 0x%x \n",temp, reg_data);
                }
     
                msleep(500);
        
                /* clear chk_data */
                chk_data = 0;
                i2c_if.buf = (char *)&chk_data;

                rc = vm_read(&i2c_if, &err_buf[0]);     
                if (rc != PASSED) {
                    sprintf(err_buf, "Ripple one FAILED on read back %s.",
                            reg_ptr->name);
                    break;
                }
          
                if ((NVRAM)->diagflag & D_VERBOSE) {
                    printf("temp  = 0x%x   chk_data = 0x%x \n",temp, chk_data);
                }
#if DEBUG
    printf(" *** %s Ripple one , value 0x%x and 0x%x.\n", reg_ptr->name,chk_data,temp );
#endif

                if (chk_data != temp) {
		    rc = FAILED;
                    sprintf(err_buf, "%s Ripple one FAILED, not the same value "
                                     "0x%x and 0x%x.", reg_ptr->name, chk_data,temp);
		    break;
                }
            }
          
            /* leave the for loop of reg_ptr*/ 
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

                prpass(testpass, "Ripple 0 Test: %s Reg.(0x%02x), Testpattern = 0x%04x",
                       reg_ptr->name, reg_ptr->offset, temp);

                reg_data = temp;
                i2c_if.buf = (char *)&reg_data;
          
                rc = vm_write(&i2c_if);
                if (rc != PASSED) {
                    sprintf(err_buf, "Ripple zero FAILED on write to %s.",
                            reg_ptr->name);
                    break;
                }

                msleep(500);
       
                i2c_if.buf = (char *)&chk_data;

                rc = vm_read(&i2c_if, &err_buf[0]);
                if (rc != PASSED) {
                    sprintf(err_buf, "Ripple zero FAILED to read back %s.",
                            reg_ptr->name);          	
                    break;
                }
#if DEBUG
    printf(" *** %s Ripple zero , value 0x%x and 0x%x.\n", reg_ptr->name, chk_data, temp);
#endif

                if (chk_data != temp) {
                    rc = FAILED;  	
                    sprintf(err_buf, "%s Ripple zero FAILED, not the same value"
                                     " 0x%x and 0x%x.", reg_ptr->name, chk_data,temp);
                    break;
                }
            }
          
            /* leave the for loop of reg_ptr*/ 
            if (rc != PASSED) {
                break;
            }

            /*
             * pattern test
             */
            data = (uint16_t)PWR_SER_PATTERN;

            for (ix = 0; ix < 2; ix++){
                /* build mask of size for pattern */
                for (ii = 0; ii < (sizeof(i2c_if.size)*8); ii++) {
                    mask |= (1 << ii);
                }
                temp = (data & mask);

                prpass(testpass, "Pattern Test: %s Reg.(0x%02x), Testpattern = 0x%04x",
                       reg_ptr->name, reg_ptr->offset, temp);

                reg_data = temp;
                i2c_if.buf = (char *)&reg_data;
                if (!temp) {
                    continue;
                }

                rc = vm_write(&i2c_if);
                if (rc != PASSED) {
                    sprintf(err_buf, "Pattern test FAILED on write to %s.",
                            reg_ptr->name);
                    break;
                }

                msleep(500);

                i2c_if.buf = (char *)&chk_data;
                rc = vm_read(&i2c_if, &err_buf[0]);
                if (rc != PASSED) {
                    sprintf(err_buf, "Pattern test FAILED to read back %s.",
                            reg_ptr->name);  
                    break;
                }
#if DEBUG
    printf(" *** %s pattern , value 0x%x and 0x%x.\n", reg_ptr->name,chk_data,temp );
#endif

                if (chk_data != temp) {
                    rc = FAILED;
                    sprintf(err_buf, "%s Pattern test FAILED, not the same value"
                                     " 0x%x and 0x%x.", reg_ptr->name, chk_data,temp);
                    break;
                }

                /* complement data pattern */
                data = (uint16_t)(~PWR_SER_PATTERN);
            } /* for (ix = 0; ix < 2; ix++) */

            /* leave the for loop of reg_ptr*/ 
            if (rc != PASSED) {
                break;
            }
	    
            /*
             * restore reset value
             */
            i2c_if.buf = (char *)&original_data;

            rc = vm_write(&i2c_if);
            if (rc != PASSED) {
                sprintf(err_buf, "Restore value FAILED %s.", reg_ptr->name);
                break;
            }
        } /*if ((reg_table_p->type & (READ_ONLY | WRITE_ONLY))*/
    } /* for (i = 0, reg_ptr = &vtg_mntr_table[0]; */

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
 * Function:	vtg_mntr_stat_test
 *
 * Description:	Check Status and Voltage Fault registers.
 *
 * Inputs:	submenu - TRUE if invoked from submenu.
 *
 * Outputs:	PASSED/FAILED.
 *
 * Assumptions:
 *
 *********************************************************************
 */
static int vtg_mntr_stat_test(int submenu)
{
    int rc;
    char err_buf[ERR_BUF_SIZE];

    if (submenu == TRUE) {
	testname("Voltage Monitor Status");
    } else {
	prpass(testpass, "Voltage Monitor Status Test");
    }

    rc = vtg_mntr_stat_check(&err_buf[0]);
    if (rc != PASSED) {
	cterr('f', 0, "%s", err_buf);
	return(FAILED);
    }    
    
    if (submenu == TRUE) {
	prcomplete(testpass, errcount, (char *)0);
    }

    return(rc);

}

/*********************************************************************
 *
 * Function:	vtg_mntr_ctr_test
 *
 * Description:	Test Run Time Counter registers to make sure it increments.
 *
 * Inputs:	submenu - TRUE if invoked from submenu.
 *
 * Outputs:	PASSED/FAILED.
 *
 * Assumptions:
 *
 *********************************************************************
 */
static int vtg_mntr_ctr_test(int submenu)
{
    n2g_i2c_if_t i2c_if;
    uint32_t     rc = FAILED, retry_count = 0;
    ren_t        rtc_1_ms, rtc_1, rtc_1_ls, rtc_2_ms, rtc_2, rtc_2_ls;
    char         err_buf[ERR_BUF_SIZE];

    if (submenu == TRUE) {
	testname("Voltage Monitor Run Time Counter");
    } else {
	prpass(testpass, "Voltage Monitor Run Time Counter Test");
    }
    
    /* Get Voltage Monitor I2C interface structure */
    rc = get_vtg_mntr_i2c_struct(&i2c_if);
    if (rc != PASSED) {
        printf("%s: Failed to get Voltage Monitor I2C structure", __FUNCTION__);
        return (rc);
    }
    
  while (retry_count < VTG_MNTR_RUNTIME_RETRY) {
      /* Read the Run time counter registers first */
      i2c_if.offset = VTG_MNTR_RTC_2; 
      i2c_if.buf = (char *)&rtc_1_ms;
      if ((rc = vm_read(&i2c_if, &err_buf[0])) != PASSED) {
    sprintf(err_buf, "vtg_mntr_ctr_test() Unable to read MS byte first "
         "time");
    } else {
    i2c_if.offset = VTG_MNTR_RTC_1;
    i2c_if.buf = (char *)&rtc_1;
    if ((rc = vm_read(&i2c_if, &err_buf[0])) != PASSED) {
        sprintf(err_buf, "vtg_mntr_ctr_test() Unable to read middle "
             "byte first time");
    } else {
        i2c_if.offset = VTG_MNTR_RTC_0;
        i2c_if.buf = (char *)&rtc_1_ls;
        if ((rc = vm_read(&i2c_if, &err_buf[0])) != PASSED) {
      sprintf(err_buf, "vtg_mntr_ctr_test() Unable to read LS "
           "byte first time");
        } /* endof if RTC_0 */
    } /* endof if RTC_1 */
      } /* endof if RTC_2 */

      if (rc != PASSED) {
    /* Read failed */
    rc = FAILED;
    break;  /* out of the while */
      }

      /* Wait for the Run Time counter to increment */
      msleep(VTG_MNTR_RTC_TEST_TIME);

      /* Read the Run time counter registers again */
      i2c_if.offset = VTG_MNTR_RTC_2;
      i2c_if.buf = (char *)&rtc_2_ms;
      if ((rc = vm_read(&i2c_if, &err_buf[0])) != PASSED) {
    sprintf(err_buf, "vtg_mntr_ctr_test() Unable to read MS byte "
         "again");
      } else {
    i2c_if.offset = VTG_MNTR_RTC_1;
    i2c_if.buf = (char *)&rtc_2;
    if ((rc = vm_read(&i2c_if, &err_buf[0])) != PASSED) {
        sprintf(err_buf, "vtg_mntr_ctr_test() Unable to read middle "
             "again");
    } else {
        i2c_if.offset = VTG_MNTR_RTC_0;
        i2c_if.buf = (char *)&rtc_2_ls;
        if ((rc = vm_read(&i2c_if, &err_buf[0])) != PASSED) {
      sprintf(err_buf, "vtg_mntr_ctr_test() Unable to read LS "
           "byte again");
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
    if (retry_count >=  VTG_MNTR_RUNTIME_RETRY) {
        sprintf(err_buf, "RunTime Counter test failed. "
             "Read %#x %#x %#x first time. "
             "Then read %#x, %#x, %#x",
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

    return(rc);

}

/*********************************************************************
 *
 * Function:	vtg_mntr_vtg_check
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
static int vtg_mntr_vtg_check(int submenu)
{
    n2g_i2c_if_t i2c_if;
    vtg_mntr_v_t *vtg_p;
    uint32_t rc = FAILED;
    uint16_t rt, max, min;
    char err_buf[ERR_BUF_SIZE * 2], rd_buf[ERR_BUF_SIZE];

    if (submenu == TRUE) {
	testname("Voltage Monitor Voltage Check");
    } else {
	prpass(testpass, "Voltage Monitor Voltage Check");
    }

    /* Get Voltage Monitor I2C interface structure */
    rc = get_vtg_mntr_i2c_struct(&i2c_if);
    if (rc != PASSED) {
        printf("%s: Failed to get Voltage Monitor I2C structure.\n",
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
	    i2c_if.offset = vtg_p->rt_o;
	    i2c_if.buf = (char *)&rt;
	    rc = vm_read(&i2c_if, &rd_buf[0]);

	    if (rc != PASSED) {
		sprintf(err_buf, "%s RealTime read failed. %s",
				  vtg_p->volt_p, rd_buf);
		rc = FAILED;
		break;
	    }

	    /* Read Maximum voltage */
	    i2c_if.offset = vtg_p->max_o;
	    i2c_if.buf = (char *)&max;
	    rc = vm_read(&i2c_if, &rd_buf[0]);
	    if (rc != PASSED) {
		sprintf(err_buf, "%s Max read failed. %s",
				  vtg_p->volt_p, rd_buf);
		rc = FAILED;
		break;
	    }

	    /* Read Minimum voltage */
	    i2c_if.offset = vtg_p->min_o;
	    i2c_if.buf = (char *)&min;
	    rc = vm_read(&i2c_if, &rd_buf[0]);
	    if (rc != PASSED) {
		sprintf(err_buf, "%s Min read failed. %s",
				  vtg_p->volt_p, rd_buf);
		rc = FAILED;
		break;
	    }

	    /* Got all voltages. Check Real Time is within Min and Max */
	    if ((rt > max) || (rt < min)) {
		sprintf(err_buf, "%s registers not updated. Max = %#x "
				 "Min = %#x. RT = %#x", vtg_p->volt_p,
				 max, min, rt);
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

	    if (min < vtg_p->under_v) {
		sprintf(err_buf, "%s Min = %#x is under spec (%#x). \n"
				 "Min = %#x. Max = %#x. Real Time = %#x.",
				 vtg_p->volt_p, min, vtg_p->under_v,
				 min, max, rt);
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

    return(rc);

}

/*********************************************************************
 *
 * Function:	vtg_mntr_stat_test
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
static int vtg_mntr_stat_check(char *err_buf)
{
    n2g_i2c_if_t i2c_if;
    uint32_t rc = FAILED;
    uint16_t status, wr_data, fault = 0;

    /* Get Voltage Monitor I2C interface structure */
    rc = get_vtg_mntr_i2c_struct(&i2c_if);
    if (rc != PASSED) {
        printf("%s: Failed to get Voltage Monitor I2C structure.\n",
              __FUNCTION__);
        return (rc);
    }

    /* Read the Clearable Status register */
    i2c_if.offset = VTG_MNTR_STA_C;
    i2c_if.buf = (char *)&status;
    rc = vm_read(&i2c_if, err_buf);	/* Read the status register */
    if (rc == PASSED) {
	/* Got the status register. Check for the voltage fault status bits */
	if (status & VTG_MNTR_STAT_V_F_PU) {
	    /* Voltage Fault during Power up */
	    sprintf(err_buf, "Voltage Fault During Power Up");
	    rc = VTG_MNTR_STAT_CHECK_FAIL;
	} else {
	    /* Check for 12V fault during operation */
	    if (status & VTG_MNTR_STAT_V_F_OP) {
		/* Voltage Fault during Operation */
		/* Read  the Clearable Voltage Fault register */
		i2c_if.offset = VTG_MNTR_V_F_C;
		i2c_if.buf = (char *)&fault;

		rc = vm_read(&i2c_if, err_buf); /* Read the voltage fault
						  * register */

		if (rc != PASSED) {
		    sprintf(err_buf, "vtg_mntr_stat_check() Voltage fault register "
				     "read failed. rc = %#x", rc);
		    rc = FAILED;
		} else {
		    /* True fault */
                    sprintf(err_buf, "Voltage Fault During Operation");
                    rc = VTG_MNTR_STAT_CHECK_FAIL;
		} /* endof if rc of vm_read */
	    } /* endof if 12v check */
	} /* endof if voltage fault check */

	/* If voltage fault, clear both status and fault registers */
	if (fault && (rc == PASSED)) {
	    /* Voltage fault */
	    i2c_if.offset = VTG_MNTR_STA_C;
	    wr_data = DSWAP2(status);
	    i2c_if.buf = (char *)&wr_data;

	    rc = vm_write(&i2c_if);

	    if (rc == PASSED) {
		i2c_if.offset = VTG_MNTR_V_F_C;
		wr_data = DSWAP2(fault);
		msleep(REN_I2C_PROC_TIME);

    rc = vm_write(&i2c_if);

	    } else {
		sprintf(err_buf, "Unable to clear Pwr Seq Status register. "
				  "rc = %#x", rc);
		rc = FAILED;
	    } /* endof if rc */
	} /* endof if fault */

#ifndef WD_BYPASS
	if (status & VTG_MNTR_STAT_WDOG) {
	    sprintf(err_buf, "Voltage Monitor WD status = %#x", status);
	    rc = VTG_MNTR_STAT_CHECK_FAIL;
	}
#endif /* WD_BYPASS */
    } else {
	sprintf(err_buf, "vtg_mntr_stat_check() Status register read failed. rc = "
			 "%#x", rc);
    }

    return (rc);
}

/*********************************************************************
 *
 * Function:	vtg_mntr_mrgin_test
 *
 * Description:	Voltage margins I2C test.
 *
 * Inputs:	submenu - called in from submenu or main menu.
 *
 * Outputs:	PASSED/FAILED.
 *
 * Assumptions:
 *
 *********************************************************************
 */
static int vtg_mntr_mrgin_test(int submenu)
{

    uint32_t rc = FAILED;
    n2g_i2c_if_t i2c_if;
    pwr_mrgin_t *pattern_p;
    uint pattern_loop, test_loop;
    uint16_t test_data, seq_ctl, masked_ctl, read_data, write_data;
    char err_buf[ERR_BUF_SIZE];


    /* Get Voltage Monitor I2C interface structure */
    rc = get_vtg_mntr_i2c_struct(&i2c_if);
    if (rc != PASSED) {
        printf("%s: Failed to get Voltage Monitor I2C structure.\n",
              __FUNCTION__);
        return (rc);
    }
    
    if (submenu == TRUE) {
	testname("Voltage Margin");
    } else {
	prpass(testpass, "Voltage Margin Test");
    }

    i2c_if.offset = VTG_MNTR_MRGN_CTRL;
    i2c_if.buf = (char *)&seq_ctl;

    /* Save the Power Margin Control register */
    rc = vm_read(&i2c_if, &err_buf[0]);

    if (rc == PASSED) {
	/* Got the register, clear all bits for test later */
	masked_ctl = seq_ctl & (~(VTG_MNTR_DDR_MRGN_EN | VTG_MNTR_DDR_MRGN |
                                  VTG_MNTR_3_3V_MRGN_EN | VTG_MNTR_3_3V_MRGN));
        if ((NVRAM)->diagflag & D_VERBOSE) {
            printf("%s:%d masked_ctl = 0x%04x, seq_ctl = 0x%04x.\n",
                   __FUNCTION__, __LINE__, masked_ctl, seq_ctl);
        }
    } else {
	/* Unable to read data */
	/* detach reset dev_destroy to dev_destroy_default */
	cterr('f', 0, "%s() Ctrl read failed. %s", __FUNCTION__, err_buf);
	return(FAILED);
    }

    /* Test margins */
    for (test_loop = 0; test_loop < PWR_MRGN_LOOP; test_loop++) {
	for (pattern_loop = 0, pattern_p = &pwr_mrgn_tbl[0];
	     pattern_loop < PWR_MRGN_TBL_SIZE; pattern_loop++, pattern_p++) {
	    test_data = masked_ctl | pattern_p->mask;

            if ((NVRAM)->diagflag & D_VERBOSE) {
                printf("%s:%d pattern_p->mask = 0x%04x, test_data = 0x%04x.\n",
                       __FUNCTION__, __LINE__, pattern_p->mask, test_data);
            }

	    prpass(testpass, pattern_p->name);

	    /* Write test data */
            write_data = test_data;
	    i2c_if.buf = (char *)&write_data;
	    
            rc = vm_write(&i2c_if);
	    if (rc != PASSED) {
		sprintf(&err_buf[0], "%s() Write %#x failed. rc = %#x",
			      __FUNCTION__, test_data, rc);
		break;
	    }
	    msleep(REN_I2C_PROC_TIME);

	    /* Read test data */
	    i2c_if.buf = (char *)&read_data;
	    rc = vm_read(&i2c_if, &err_buf[0]);

	    if (rc != PASSED) {
		sprintf(&err_buf[0], "%s() Read %#x failed. %s",
				     __FUNCTION__, test_data, err_buf);
		break;
	    }

            if ((NVRAM)->diagflag & D_VERBOSE) {
                printf("%s:%d test_data = 0x%04x, read_data = 0x%04x.\n",
                       __FUNCTION__, __LINE__, test_data, read_data);
            }

	    /* Compare data */
	    if (test_data != read_data) {
		rc = FAILED;
		sprintf(&err_buf[0], "%s expect %#x. read %#x",
				     __FUNCTION__, test_data, read_data);
		break;
	    } /* endof if test_data */
	} /* endof for pattern_loop */
    } /* endof for test_loop */

    /* Restore the Margin Control register */
    if (rc == PASSED) {
	i2c_if.buf = (char *)&seq_ctl;
  rc = vm_write(&i2c_if);

	sprintf(&err_buf[0], "%s() Ctrl restore failed. rc = %#x",
		__FUNCTION__, rc);
	msleep(REN_I2C_PROC_TIME);

    }

    if (rc != PASSED) {
	cterr('f', 0, &err_buf[0]);
    }


    return(rc);
}

#if 0
/*********************************************************************
 *
 * Function:	get_vtg_mntr_rev
 *
 * Description:	Returns Voltage Monitor revision.
 *
 * Inputs:	Pointer to revision to be saved.
 *
 * Outputs:	PASSED/FAILED.
 *
 * Assumptions:
 *
 *********************************************************************
 */
static int get_vtg_mntr_rev(void)
{
    uint32_t rc = FAILED;
    ren_t    rev;

    rc = get_reg(VTG_MNTR_REV, &rev);
    if (rc != PASSED) {
	cterr('f', 0, "%s: Voltage Monitor FW Rev. reg. read failed(rc = %#x).",
              __FUNCTION__, rc);
    } 
    printf("Voltage Monitor FW version is %d.%02d.\n",
           (rev & VTG_MNTR_REV_MAJOR) >> 8,
           (rev & VTG_MNTR_REV_MINOR));
           
    return(rc);

}


/*********************************************************************
 *
 * Function:	get_reg
 *
 * Description:	Read requested Voltage Monitor register.
 *
 * Inputs:	Register offset.
 *		Pointer to register to be saved.
 *
 * Outputs:	PASSED/FAILED.
 *
 * Assumptions:
 *
 *********************************************************************
 */
static int get_reg(ren_o offset, ren_t *reg)
{
    n2g_i2c_if_t i2c_if;
    uint32_t rc = FAILED;
    char err_buf[ERR_BUF_SIZE];

    /* Get Voltage Monitor I2C interface structure */
    rc = get_vtg_mntr_i2c_struct(&i2c_if);
    if (rc != PASSED) {
	cterr('f', 0, "%s: Failed to get Voltage Monitor I2C structure",
              __FUNCTION__);
        return (rc);
    }
    
    /* Setup the related info */
    i2c_if.offset = offset;
    i2c_if.buf = (char *)reg;
    
    /* Read out the value of Register */
    rc = vm_read(&i2c_if, &err_buf[0]);

    if (rc != PASSED) {
	cterr('f', 0, "%s() Voltage Monitor register read @ %#x failed. %s ",
		      __FUNCTION__, offset, err_buf);
    }
    
    return(rc);
}
#endif


/*********************************************************************
 *
 * Function:	pwr_rd
 *
 * Description:	Read Voltage Monitor Register. Voltage Monitor I2C Read has
 *		2 I2C operations. The I2C write with the register offset.
 *		Then wait for the REN_I2C_PROC_TIME milliseconds to allow
 *		the Voltage Monitor firmware to setup the data of the requested
 *		register. Then the I2C read will return the data.
 *
 * Inputs:	addr - offset of register to be read.
 *		size - Number of bytes to be read. Pwr Seq registers are 2 bytes
 *		buf  - points to the data buffer to be read.
 *		param - Pointer to the Pwr Seq.CU device object.
 *
 * Outputs:     PASSED/FAILED.
 *
 * Assumptions:
 *
 *********************************************************************
 */
static int
pwr_rd(ulong addr, int size, ulong *buf, void *param)
{
    uint32_t rc = FAILED;
    n2g_i2c_if_t i2c_if;
    ren_t data;

    /* Get Voltage Monitor I2C interface structure */
    rc = get_vtg_mntr_i2c_struct(&i2c_if);
    if (rc != PASSED) {
        printf("%s: Failed to get Voltage Monitor I2C structure.\n",
              __FUNCTION__);
        return (rc);
    }
    
    /* Setup the interface struct for I2C API read */
    i2c_if.offset = (uint8_t) addr;
    i2c_if.buf = (char *)&data;

    /* Need to perform the write of the addr first */
    /* The write will use the offset, but not the read */
    i2c_if.size = 0;
    rc = vm_write(&i2c_if);

    if (rc != PASSED) {
  /* Unable to write offset */
  cterr('f', 0, "pwd_rd() Unable to write offset. rc = 0x%08x", rc);
  return(FAILED);
    }

    msleep(REN_I2C_PROC_TIME);

    /* Ready for the read */
    i2c_if.size = size;
    rc = n2g_i2c_read(&i2c_if);

    if (rc != PASSED) {
  /* Unable to read data */
  cterr('f', 0, "pwr_rd() Unable to read. rc = 0x%08x", rc);
  return(FAILED);
    } else {
  /* Got the data */
  *buf = (ulong)data;
  msleep(REN_I2C_PROC_TIME); 
#ifdef VM_DEBUG
  printf("pwr_rd() read %#x %d bytes @ %#x\n", data, size, addr);
#endif /* VM_DEBUG */
    }
    
    return(PASSED);

}

/**********************************************************************
 *
 * Function:	pwr_wr
 *
 * Description:	Write Voltage Monitor register. After the write, wait
 *		REN_I2C_PROC_TIME milliseconds to have the operation to be
 *		valid.
 *
 * Inputs:	addr - offset of register to be written.
 *		size - Number of bytes to write. Pwr Seq registers are 2 bytes.
 *		data - Write data.
 *		param - Pointer to the Voltage Monitor device object. Ignored.
 *
 * Output:	PASSED/FAILED.
 *
 **********************************************************************
 */
static int
pwr_wr(ulong addr, int size, ulong data, void *param)
{
    uint32_t rc = FAILED;
    n2g_i2c_if_t i2c_if;
    ren_t pwr_data;

    /* Get Voltage Monitor I2C interface structure */
    rc = get_vtg_mntr_i2c_struct(&i2c_if);
    if (rc != PASSED) {
        printf("%s: Failed to get Voltage Monitor I2C structure.\n",
              __FUNCTION__);
        return (rc);
    }
    
    /* Setup the interface struct for I2C API read */
    i2c_if.offset = (uint8_t) addr;
    i2c_if.size = size;
    i2c_if.buf = (char *)&pwr_data;
    pwr_data = DSWAP2((ren_t)data);

#ifdef VM_DEBUG /* */
    printf("pwr_wr() write %#x %d bytes to %#x\n", pwr_data, size, addr);
#endif /* VM_DEBUG */

    rc = vm_write(&i2c_if);

    if (rc != PASSED) {
	/* Write failed */
	cterr('f', 0, "pwr_wr() write failed to write %#x @ %#x. rc = %#x",
		      data, addr, rc);
	return(FAILED);
    } else {
	/* Data written */
	msleep(REN_I2C_PROC_TIME);

	return(PASSED);
    }
}

int vtg_get_version (uint16_t *version)
{
    if ((vtg_mntr_read_reg(0, version)) != RC_I2C_OP_OK) {
        return FAILED;
    }

    return PASSED;
} 

/*------------------------------------------------------------------
$Log: platform_vtg_mntr.c,v $
Revision 1.3  2013/11/26 08:40:37  hroni
fix compiler warning

Revision 1.2  2013/10/08 11:14:27  erwu2
enhanced err msg first check-in

Revision 1.1  2013/05/09 05:42:38  alpeng
moving overlord common code from x86

Revision 1.8  2012/11/28 18:19:10  palin2
To make I2C utilities SubMenu more intuitive.

Revision 1.7  2012/11/21 19:47:22  palin2
Use function "do_all_menu_items" to replace "menu_exec_doall_diags".

Revision 1.6  2012/09/26 18:02:15  palin2
Uniformed the print out format of I2C devices defult tests.

Revision 1.5  2012/08/13 08:02:46  alpeng
clean up useless message

Revision 1.4  2012/06/28 06:11:38  palin2
Change register dump display format.

Revision 1.3  2012/05/30 16:45:03  palin2
Clean up compile warnings.

Revision 1.2  2012/03/28 00:38:25  mcharon
remove forward slash from second line

Revision 1.1.1.1  2012/03/23 23:02:10  ptong
Initial archive of ng_diag module


$Endlog$
*/
