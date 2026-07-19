/* $Id: dev_32021.c,v 1.5 2013/11/26 08:40:32 hroni Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/chips/dev_idt82v32021/dev_32021.c,v $
 *------------------------------------------------------------------------------
 *
 * Filename:	dev_32021.c
 *
 * Description:	IDT82V32021 EBU WAN PLL driver functions.
 *
 *		IDT82V32021 does generate interrupt; therefore,
 *		dev_attach, dev_detach, dev_reconfig_needed, dev_restart,
 *		dev_init, dev_oper_enable, dev_oper_disable,
 *		dev_intr_enable, dev_intr_disable, dev_isr, dev_show,
 *		dev_err_report, dev_collect_crashinfo, dev_destroy are
 *		implemented.
 *
 *		Bits definition of the pins of the chip are platform specific.
 *		The caller will pass the struct of the bits and registers
 *		test table.
 *
 *		IDT82V32021 access involves offset, and data.
 *		Refer to the vendor's datasheet for more info.
 *
 * Copyright (c) 2007-2013 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------------------
 *
 */

#include <stdlib.h>
#include <assert.h>
#include "endians.h"
#include "types.h"
#include "defs.h"
#include "common.h"
#include "common_utils.h"
#include "dev_print.h"
#include "dev_object.h"
#include "dev_32021.h"
#include "free.h"
#include "proto.h"

/*===================================================================*
 *                    Function Prototypes                            *
 *===================================================================*/
static uint32   idt32021_dev_attach(dev_object_t *);
static uint32	idt32021_dev_detach(dev_object_t *);
static uint32	idt32021_dev_reconfig(dev_object_t *, void *, boolean *);
static uint32	idt32021_dev_restart(dev_object_t *);
static uint32	idt32021_dev_init(dev_object_t *);
static uint32	idt32021_dev_oper_en(dev_object_t *);
static uint32	idt32021_dev_oper_dis(dev_object_t *);
static uint32	idt32021_dev_intr_en(dev_object_t *);
static uint32	idt32021_dev_intr_dis(dev_object_t *);
static uint32	idt32021_dev_isr(dev_object_t *);
static uint32	idt32021_dev_show(dev_object_t *, print_fn_t, dev_show_cmd);
static uint32	idt32021_crsh(dev_object_t *, print_fn_t, dev_show_cmd);
static void	idt32021_destroy(dev_object_t **);
static int	idt32021_i2c_rd(ulong, int, ulong *, void *);
static int	idt32021_i2c_wr(ulong, int, ulong, void *);
static int	idt32021_reg_test(dev_object_t *);
    
/* Register test table */
static reg_info_t_ext reg_ext = {
			sizeof(char),
                        idt32021_i2c_rd,
                        idt32021_i2c_wr,
                        0};

static reg_info_t idt32021_default_reg_tbl[] = {
    {"ID[7:0] - Device ID 1",				IDT32021_ID1,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT}, 0xFF, 0x88},
    {"ID[15:8] - Device ID 2",				IDT32021_ID2,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT}, 0xFF, 0x11},
    {"Crystal Osc. Freq. Offset Calibration Config. 1",	IDT32021_NOM_FREQ_CNFG1,
	READ_WRITE | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT}, 0xFF, 0x00},
    {"Crystal Osc. Freq. Offset Calibration Config. 2",	IDT32021_NOM_FREQ_CNFG2,
	READ_WRITE | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT}, 0xFF, 0x00},
    {"Crystal Osc. Freq. Offset Calibration Config. 3",	IDT32021_NOM_FREQ_CNFG3,
	READ_WRITE | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT}, 0xFF, 0x00},
    {"Phase Lock Alarm Time-Out Configurationr",	IDT32021_PH_ALRM_TO_CNF,
	READ_WRITE | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT}, 0xFF, 0x32},
    {"Input Mode Configuration", 			IDT32021_INP_MODE_CNF,
	READ_WRITE | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT}, 0xFD, 0xA2},
    {"Master Clock Configuration",			IDT32021_OSCI_CNFG,
	READ_WRITE | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT}, 0x04, 0x89},
    {"Freq. Monitor, Input Clock Selection & PBO Ctrl",	IDT32021_MON_SW_PBO_CNF,
	READ_WRITE | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT}, 0xFD, 0x85},
    {"Register Protection Mode Configuration",		IDT32021_PROTECTION_CNF,
	READ_WRITE | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT}, 0xFF, 0x85},
    {"Interrupt Configuration",				IDT32021_INTERRUPT_CNFG,
	READ_WRITE | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT}, 0x03, 0x02},
    {"Interrupt Status 1",				IDT32021_INTERRUPT1_STS,
	READ_WRITE | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT}, 0x0C, 0x3C},
    {"Interrupt Status 2",				IDT32021_INTERRUPT2_STS,
	READ_WRITE | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT}, 0xC0, 0x81},
    {"Interrupt Status 3",				IDT32021_INTERRUPT3_STS,
	READ_WRITE | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT}, 0x80, 0xD0},
    {"Interrupt Control 1",				IDT32021_INTR1_EN_CNFG,
	READ_WRITE | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT}, 0x0C, 0x00},
    {"Interrupt Control 2",				IDT32021_INTR2_EN_CNFG,
	READ_WRITE | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT}, 0xC0, 0x00},
    {"Interrupt Control 3",				IDT32021_INTR3_EN_CNFG,
	READ_WRITE | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT}, 0x80, 0x00},
    {"CMOS Input Clock 1 Configuration",		IDT32021_IN1_CMOS_CNFG,
	READ_WRITE | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT}, 0xFF, 0x00},
    {"CMOS Input Clock 2 Configuration",		IDT32021_IN2_CMOS_CNFG,
	READ_WRITE | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT}, 0xFF, 0x00},
    {"DivN Divider Channel Selection",			IDT32021_PRE_DIV_C_CNFG,
	READ_WRITE | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT}, 0x0F, 0x00},
    {"DivN Divider Division Factor Configuration 1",	IDT32021_PRE_DIVN1_CNFG,
	READ_WRITE | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT}, 0xFF, 0x00},
    {"DivN Divider Division Factor Configuration 2",	IDT32021_PRE_DIVN2_CNFG,
	READ_WRITE | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT}, 0x7F, 0x00},
    {"CMOS Input Clock 1 & 2 Priority Configuration",	IDT32021_IN1_2_CMOS_SEL,
	READ_WRITE | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT}, 0xFF, 0x21},
    {"Factor of Frequency Monitor Configuration",	IDT32021_FREQ_MON_FACT,
	READ_WRITE | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT}, 0x0F, 0x0B},
    {"Frequency Monitor Threshold for All Input Clocks",IDT32021_ALL_FREQ_MON_T,
	READ_WRITE | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT}, 0x0F, 0x23},
    {"Upper Threshold for Leaky Bucket Configuration 0", IDT32021_UP_THR_0_CNFG,
	READ_WRITE | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT}, 0xFF, 0x06},
    {"Lower Threshold for Leaky Bucket Configuration 0", IDT32021_LO_THR_0_CNFG,
	READ_WRITE | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT}, 0xFF, 0x04},
    {"Bucket Size for Leaky Bucket Configuration 0",	IDT32021_BUCKET_SIZE_0,
	READ_WRITE | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT}, 0xFF, 0x08},
    {"Decay Rate for Leaky Bucket Configuration 0",	IDT32021_DECAY_RATE_0_C,
	READ_WRITE | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT}, 0x03, 0x01},
    {"Upper Threshold for Leaky Bucket Configuration 1", IDT32021_UP_THR_1_CNFG,
	READ_WRITE | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT}, 0xFF, 0x06},
    {"Lower Threshold for Leaky Bucket Configuration 1", IDT32021_LO_THR_1_CNFG,
	READ_WRITE | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT}, 0xFF, 0x04},
    {"Bucket Size for Leaky Bucket Configuration 1",	IDT32021_BUCKET_SIZE_1,
	READ_WRITE | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT}, 0xFF, 0x08},
    {"Decay Rate for Leaky Bucket Configuration 1",	IDT32021_DECAY_RATE_1_C,
	READ_WRITE | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT}, 0x03, 0x01},
    {"Upper Threshold for Leaky Bucket Configuration 2", IDT32021_UP_THR_2_CNFG,
	READ_WRITE | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT}, 0xFF, 0x06},
    {"Lower Threshold for Leaky Bucket Configuration 2", IDT32021_LO_THR_2_CNFG,
	READ_WRITE | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT}, 0xFF, 0x04},
    {"Bucket Size for Leaky Bucket Configuration 2",	IDT32021_BUCKET_SIZE_2,
	READ_WRITE | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT}, 0xFF, 0x08},
    {"Decay Rate for Leaky Bucket Configuration 2",	IDT32021_DECAY_RATE_2_C,
	READ_WRITE | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT}, 0x03, 0x01},
    {"Upper Threshold for Leaky Bucket Configuration 3", IDT32021_UP_THR_3_CNFG,
	READ_WRITE | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT}, 0xFF, 0x06},
    {"Lower Threshold for Leaky Bucket Configuration 3", IDT32021_LO_THR_3_CNFG,
	READ_WRITE | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT}, 0xFF, 0x04},
    {"Bucket Size for Leaky Bucket Configuration 3",	IDT32021_BUCKET_SIZE_3,
	READ_WRITE | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT}, 0xFF, 0x08},
    {"Decay Rate for Leaky Bucket Configuration 3",	IDT32021_DECAY_RATE_3_C,
	READ_WRITE | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT}, 0x03, 0x01},
    {"Input Clock Frequency Read Channel Selection",	IDT32021_IN_FREQ_R_CH_C,
	READ_WRITE | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT}, 0x0F, 0x00},
    {"Input Clock Frequency Read Value",		IDT32021_IN_FREQ_READ_S,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT}, 0xFF, 0x00},
    {"CMOS Input Clock 1 & 2 Status",			IDT32021_IN1_2_CMOS_STS,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT}, 0x77, 0x60},
    {"Input Clocks Validity 1",				IDT32021_INPUT_VALID1_S,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT}, 0x0C, 0x04},
    {"Priority Status 1",				IDT32021_PRIO_TBL1_STS,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT}, 0xFF, 0x33},
    {"Priority Status 2",				IDT32021_PRIO_TBL2_STS,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT}, 0x0F, 0x00},
    {"T0 Selected input Clock Configuration",		IDT32021_T0_INPUT_SEL_C,
	READ_WRITE | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT}, 0x0F, 0x00},
    {"T0 DPLL Operating Status",			IDT32021_OPERATING_STS,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT}, 0xAF, 0x8C},
    {"T0 DPLL Operating Mode Configuration",		IDT32021_T0_OP_MODE_CFG,
	READ_WRITE | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT}, 0x07, 0x00},
    {"T0 DPLL & APLL Path Configuration",		IDT32021_T0_D_APLL_PATH,
	READ_WRITE | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT}, 0xFF, 0x20},
    {"T0 DPLL Start Bandwidth & Damping Factor Config.", IDT32021_T0_DPLL_ST_BW,
	READ_WRITE | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT}, 0xFF, 0x6F},
    {"T0 DPLL Acquisition bandwidth & Damping Factor",	IDT32021_T0_DPLL_ACQ_BW,
	READ_WRITE | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT}, 0xFF, 0x6F},
    {"T0 DPLL Locked Bandwidth & Damping Factor Conf.", IDT32021_T0_DPLL_LCK_BW,
	READ_WRITE | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT}, 0xFF, 0x6F},
    {"T0 DPLL Bandwidth Overshoot Configuration",	IDT32021_T0_BW_OVERSHOT,
	READ_WRITE | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT}, 0x88, 0x88},
    {"Phase Loss Coarse Detector Limit Configuration",	IDT32021_PHASE_LOSS_CRS,
	READ_WRITE | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT}, 0xFF, 0x85},
    {"Phase Loss Fine Detector Limit Configuration",	IDT32021_PHASE_LOSS_FIN,
	READ_WRITE | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT}, 0xC7, 0xA2},
    {"T0 DPLL Holdover Mode Configuration",		IDT32021_T0_HOLD_MODE_C,
	READ_WRITE | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT}, 0xFC, 0x44},
    {"T0 DPLL Holdover Frequency Configuration 1",	IDT32021_T0_HOLD_FREQ_1,
	READ_WRITE | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT}, 0xFF, 0x00},
    {"T0 DPLL Holdover Frequency Configuration 2",	IDT32021_T0_HOLD_FREQ_2,
	READ_WRITE | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT}, 0xFF, 0x00},
    {"T0 DPLL Holdover Frequency Configuration 3",	IDT32021_T0_HOLD_FREQ_3,
	READ_WRITE | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT}, 0xFF, 0x00},
    {"DPLL Current Frequency Status 1",			IDT32021_CURR_FREQ_1_S,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT}, 0xFF, 0x29},
    {"DPLL Current Frequency Status 3",			IDT32021_CURR_FREQ_2_S,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT}, 0xFF, 0x25},
    {"DPLL Current Frequency Status 3",			IDT32021_CURR_FREQ_3_S,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT}, 0xFF, 0xFE},
    {"DPLL Soft Limit Configuration",			IDT32021_DPLL_FREQ_SOFT,
	READ_WRITE | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT}, 0xFF, 0x8C},
    {"DPLL Hard Limit Configuration 1",			IDT32021_DPLL_FREQ_HRD1,
	READ_WRITE | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT}, 0xFF, 0xAB},
    {"DPLL Hard Limit Configuration 2",			IDT32021_DPLL_FREQ_HRD2,
	READ_WRITE | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT}, 0xFF, 0x19},
    {"DPLL Current Phase Status 1",			IDT32021_CURR_DPLL_P1_S,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT}, 0xFF, 0x01},
    {"DPLL Current Phase Status 2",			IDT32021_CURR_DPLL_P2_S,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT}, 0xFF, 0x01},
    {"T0 APLL Bandwidth Configuration",			IDT32021_T0_APLL_BW_CFG,
	READ_WRITE | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT}, 0x30, 0x11},
    {"Output Clock 1 Frequency Configuration",		IDT32021_OUT1_FREQ_CNFG,
	READ_WRITE | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT}, 0xFF, 0x03},
    {"Output Clock 1 Invert Configuration",		IDT32021_OUT1_INV_CNFG,
	READ_WRITE | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT}, 0x04, 0x40},
    {"Frame Sync Output Configuration",			IDT32021_FR_SYNC_CNFG,
	READ_WRITE | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT}, 0xDC, 0x60},
    {"Phase Transient Monitor & PBO Configuration",	IDT32021_PH_MON_PBO_CFG,
	READ_WRITE | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT}, 0xBF, 0x06},
    {"Sync Monitor Configuration",			IDT32021_SYNC_MON_CNFG,
	READ_WRITE | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT}, 0xF0, 0x2B},
    {"Sync Phase Configuration",			IDT32021_SYNC_PHASE_CFG,
	READ_WRITE | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT}, 0x0F, 0x00},
/*    {0, 0, 0, {0}, 0, 0}, */
};

/* 
 * Refer to EDCS-624333 Section 12.3.2 (External TDM Synchronizer Initialization
 * for more details.
 */
static dev_idt32021_reg_init_t reg_init[] = {
    {IDT32021_OSCI_CNFG,	0x89},	/* Reg0A */
    {IDT32021_MON_SW_PBO_CNF,	0x85},	/* Reg0B */
    {IDT32021_INTERRUPT1_STS,	0x3C},	/* Reg0D */
    {IDT32021_INTERRUPT2_STS,	0x81},	/* Reg0E */
    {IDT32021_INTERRUPT3_STS,	0xD0},	/* Reg0F */
    {IDT32021_IN1_2_CMOS_SEL,	0x21},	/* Reg27 */
    {IDT32021_ALL_FREQ_MON_T,	0x23},	/* Reg2F */
    {IDT32021_UP_THR_0_CNFG,	0x31},	/* Reg31 */
    {IDT32021_T0_D_APLL_PATH,	0x20},	/* Reg55 */
    {IDT32021_T0_DPLL_ST_BW,	0x72},	/* Reg56 */
    {IDT32021_T0_DPLL_ACQ_BW,	0x72},	/* Reg57 */
    {IDT32021_PHASE_LOSS_FIN,	0xA2},	/* Reg5B */
    {IDT32021_DPLL_FREQ_HRD1,	0xFF},	/* Reg66 */
    {IDT32021_DPLL_FREQ_HRD2,	0xFF},	/* Reg67 */
    {IDT32021_OUT1_FREQ_CNFG,	0x03},	/* Reg6D */
    {IDT32021_OUT1_INV_CNFG,	0x40},	/* Reg73 */
    {IDT32021_FR_SYNC_CNFG,	0x60},	/* Reg74 */
};

static dev_idt32021_reg_init_t intr_en_reg[] = {
    {IDT32021_INTR1_EN_CNFG, IDT32021_INTR_IN2_CMOS + IDT32021_INTR_IN1_CMOS},
    {IDT32021_INTR2_EN_CNFG, IDT32021_INTR_T0_OP_MOD + IDT32021_INTR_T0_MAIN_R},
    {IDT32021_INTR3_EN_CNFG, IDT32021_INTR_EX_SYNC_A},
};

static dev_idt32021_reg_init_t id_reg[] = {
    {IDT32021_ID1,	IDT32021_DEVID1},
    {IDT32021_ID2,	IDT32021_DEVID2},
};

/*****************************************************************
 *
 * Name: idt32021_dev_create()
 *
 * Description: Create object with various device function
 * point to "do nothing"
 *
 * Input: dev_object_t pointer to the IDT82V32021 device.
 *	  error reporting function pointer.
 *
 * Returns: none
 *
 *****************************************************************/
void
idt32021_dev_create (dev_object_t *dev, dev_error_report_t error_report_fn)
{
    dev_object_fvt_t	*dev_fvt;
    dev_idt82v32021_object_t *idt32021 = (dev_idt82v32021_object_t *)dev;

    /* Allocate memory for the device object */
    if ((dev_fvt = (dev_object_fvt_t *)malloc(sizeof(dev_object_fvt_t))) ==
		NULL) {
	/* Unable to allocate memory */
	error_report_fn(dev, "malloc failure in idt32021_dev_create()",
			0);
	return;
    }

    /* Init the device object structure to default "do nothing" */
    idt32021->base.dev_state = DEV_STATE_CREATE;
    init_default_dev_object(dev, dev_fvt);

    idt32021->base.dev_object_fvt->dev_attach	= idt32021_dev_attach;
    idt32021->base.dev_object_fvt->dev_detach	= idt32021_dev_detach;
    idt32021->base.dev_object_fvt->dev_reconfig_needed = idt32021_dev_reconfig;
    idt32021->base.dev_object_fvt->dev_restart	= idt32021_dev_restart;
    idt32021->base.dev_object_fvt->dev_init	= idt32021_dev_init;
    idt32021->base.dev_object_fvt->dev_oper_enable  = idt32021_dev_oper_en;
    idt32021->base.dev_object_fvt->dev_oper_disable = idt32021_dev_oper_dis;
    idt32021->base.dev_object_fvt->dev_intr_enable = idt32021_dev_intr_en;
    idt32021->base.dev_object_fvt->dev_intr_disable = idt32021_dev_intr_dis;
    idt32021->base.dev_object_fvt->dev_isr	= idt32021_dev_isr;
    idt32021->base.dev_object_fvt->dev_show	= idt32021_dev_show;
    idt32021->base.dev_object_fvt->dev_error_report = error_report_fn;
    idt32021->base.dev_object_fvt->dev_collect_crashinfo = idt32021_crsh;
    idt32021->base.dev_object_fvt->dev_destroy	= idt32021_destroy;
    idt32021->base.dev_object_fvt->dev_name	= "IDT82V32021 - EBU WAN PLL";

    idt32021->callin_fvt = (idt32021_callin_fvt_t *)
				malloc(sizeof(idt32021_callin_fvt_t));
    idt32021->callout_fvt = (idt32021_callout_fvt_t *)
				malloc(sizeof(idt32021_callout_fvt_t));
}
/*****************************************************************
 *
 * Name: idt32021_dev_attach()
 *
 * Description: Attach the IDT82V32021 device for use. This
 *   function will initialize and setup all necessary pointers
 *   and bring the chip to operation.
 *
 * Input: Pointer to the IDT82V32021 device object
 *
 * Returns: PASSED/FAILED
 *
 *****************************************************************/
static uint32
idt32021_dev_attach (dev_object_t *dev)
{
    uint32 rc;
    dev_idt82v32021_object_t *idt32021 = (dev_idt82v32021_object_t *) dev;

    if (idt32021->callin_fvt == NULL) {
	DEV_ERROR_REPORT(dev, "idt32021_dev_attach() callin malloc", 
			 (FATAL | IDT32021_ATTACH));
	return(FAILED);
    }

    if (idt32021->callout_fvt == NULL) {
	DEV_ERROR_REPORT(dev, "idt32021_dev_attach() callout malloc", 
			 (FATAL | IDT32021_ATTACH));
	return(FAILED);
    }

    /* init the call in function */
    idt32021->callin_fvt->register_test = idt32021_reg_test;

    /* Lock the I2C device */
    if ((rc = idt32021->callout_fvt->open(idt32021->i2c_p)) != PASSED) {
        DEV_ERROR_REPORT(dev, "idt32021_dev_attach() I2C open",
			 (FATAL | rc));
        return(FAILED);
    }

    idt32021->base.dev_state = DEV_STATE_ATTACH;

    return (PASSED);
}

/*****************************************************************
 *
 * Name: idt32021_dev_detach()
 *
 * Description: detach the device specific functions from the caller.
 *		All of the device specific function are connected to the
 *		dev_do_nothing() function, except for the dev_attach()
 *		function. Also, the dev_state must be assigned the value
 *		of DEV_STATE_DETACH.
 *
 *		Since, some platforms may want to detach the device, but not
 *		release the memory resources (via a free () in the
 *		dev_destroy()), this function can be executed to accomplish
 *		this task. However, before a detached device can be used again,
 *		it must be re-attached (via the dev_attach()).
 *
 * Input: Pointer to the IDT82V32021 device object
 *
 * Returns: PASSED/FAILED
 *
 *****************************************************************/
static uint32
idt32021_dev_detach (dev_object_t *dev)
{
    uint32 rc;
    dev_idt82v32021_object_t *idt32021 = (dev_idt82v32021_object_t *) dev;

    /* Unlock the I2C device */
    if ((rc = idt32021->callout_fvt->close(idt32021->i2c_p)) != PASSED) {
	DEV_ERROR_REPORT(dev, "idt32021_dev_detach() I2C close",
			 (FATAL | rc));
	return(FAILED);
    }

    /* Init the device object structure to default "do nothing" */
    init_default_dev_object(dev, idt32021->base.dev_object_fvt);

    idt32021->base.dev_state = DEV_STATE_DETACH;

    return (PASSED);

}

/*****************************************************************
 * Name: idt32021_dev_reconfig_needed
 *
 * Description: To check whether device re-configuration is needed during
 *		(re)initialization. Based on the provided context information,
 *		the boolean return value, and possibly other factors external
 *		to the device object, the caller shall decide whether to invoke
 *		either dev_restart or dev_init, but not both. In general, the
 *		boolean return value alone is not sufficient to decide whether
 *		the device can safely be restarted or whether it must be fully
 *		initialized from scratch.
 *
 * Input: dev_object_t pointer to the IDT82V32021 device
 *	  void * - a device/platform specific context handle
 *	  boolean * - a pointer to a boolean
 *
 * Returns: PASSED/FAILED, context information and a boolean value.
 *	    The boolean value shall be set to TRUE if the device must be
 *	    reconfigured from scratch and it shall be set to FALSE otherwise.
 *
 * Assumptions: The dev_attach() function has been called and successfully
 *****************************************************************/
static uint32
idt32021_dev_reconfig(dev_object_t *dev, void *context_handle,
					 boolean *reconfig)
{
    *reconfig = FALSE;		/* No need to reconfig from scratch */
    return(PASSED);
}

/*****************************************************************
 * Name: idt32021_dev_restart
 *
 * Description:	To restart a previously initialized device without resetting
 *		the device or changing its configuration.
 *		For example, during a failover event.
 *
 *		Change the state of the device from its current state
 *		to an initial state. Also, dev_state must be assigned the
 *		value of DEV_STATE_INIT.
 *   
 * Input: dev_object_t pointer to the IDT82V32021 DEVICE
 *
 * Returns: PASSED/FAILED
 *
 * Assumptions: The dev_attach() and dev_reconfig_needed() functions has been
 *              called and successfully executed.
 *****************************************************************/
static uint32
idt32021_dev_restart(dev_object_t *dev)
{
    dev_idt82v32021_object_t *idt32021 = (dev_idt82v32021_object_t *) dev;

    idt32021->base.dev_state = DEV_STATE_INIT;
    return(PASSED);
}

/*****************************************************************
 *
 * Name: idt32021_dev_init()
 *
 * Description: Initializes the IDT82V32021 chip 
 *              
 *
 * Input: dev_object_t pointer to the IDT82V32021 device.
 *	  Caller has to setup the i2c_p parameters with init values.
 *
 * Returns: PASSED/FAILED
 *
 * Note: Make sure base.dev_addr has been initialized to chip_base_addr
 *       before calling this function.
 *
 *****************************************************************/
static uint32
idt32021_dev_init (dev_object_t *dev)
{
    uint32 rc;
    dev_idt82v32021_object_t *idt32021 = (dev_idt82v32021_object_t *)dev;
    idt32021_callout_fvt_t *callout_p = idt32021->callout_fvt;
    uint i;
    n2g_i2c_if_t new_i2c_if;
    reg_d reg;
    dev_idt32021_reg_init_t *init_p;
    char err_buf[80];

    /* Initialize the new I2C API interface struct */
    new_i2c_if.size = sizeof(reg);
    new_i2c_if.buf = (char *)&reg;
    new_i2c_if.i2c_bus_type = idt32021->i2c_p->i2c_bus_type;
    new_i2c_if.i2c_dev = idt32021->i2c_p->i2c_dev;

#ifdef TDM_PLL_DEBUG
    printf("reg_init %#x, struct %#x\n", sizeof(reg_init),
		sizeof(dev_idt32021_reg_init_t));
#endif /* TDM_PLL_DEBUG */

    for (i = 0, init_p = &reg_init[0]; i < (sizeof(reg_init) /
		sizeof(dev_idt32021_reg_init_t)); i++, init_p++) {
	reg = init_p->data;
	new_i2c_if.offset = init_p->offset;

#ifdef TDM_PLL_DEBUG
	printf("writing %#x to offset %#x\n", init_p->data, init_p->offset);
#endif /* TDM_PLL_DEBUG */

	/* Write to the IDT82V32021 with new init value */
	rc = callout_p->wr(&new_i2c_if);

	if (rc != PASSED) {
	    sprintf(err_buf, "idt32021_dev_init() write %#x @ %#x rc = %#x\n",
				init_p->data, init_p->offset, rc);
	    DEV_ERROR_REPORT(dev, err_buf, (FATAL | IDT32021_INIT));
	    return(FAILED);
	}
    }

    idt32021->base.dev_state = DEV_STATE_INIT;

    return(PASSED);

}

/*****************************************************************
 * Name: idt32021_dev_oper_en
 *
 * Description:	Enable device operation.
 *
 *		Change the state of the device from its current state to an
 *		enabled state (which implies that the device is in an
 *		operational state at the end of this function execution). Also,
 *		the dev_state must be assigned the value of DEV_STATE_ENABLE_OP
 *
 *		For devices such as port asic's and framers, this function
 *		be used to enable all or only part of the total device port's
 *		or channel's.
 *
 * Input: dev_object_t pointer to the IDT82V32021 device
 *
 * Returns: PASSED/FAILED
 *
 * Assumptions: The dev_init() function has been called and successfully
 *
 *****************************************************************/
static uint32
idt32021_dev_oper_en(dev_object_t *dev)
{
    uint32 rc;
    dev_idt82v32021_object_t *idt32021 = (dev_idt82v32021_object_t *)dev;
    idt32021_callout_fvt_t *callout_p = idt32021->callout_fvt;
    char err_buf[80];

    /* Take the IDT82V32021 out of reset */
    rc = (*callout_p->reset)(idt32021->i2c_p, ENABLE);

    if (rc != PASSED) {
	sprintf(err_buf, "idt32021_dev_oper_en() reset return code %#x.", rc);
	DEV_ERROR_REPORT(dev, err_buf, (FATAL | IDT32021_OPER_EN));
	return(FAILED);
    }

    msleep(IDT32021_DELAY_AFTER_RESET);	/* wait for device out of reset state */

    idt32021->base.dev_state = DEV_STATE_ENABLE_OP;

    return (PASSED);

}

/*****************************************************************
 * Name: idt32021_dev_oper_dis
 *
 * Description:	Disable device operation.
 *
 *		Change the state of the device from its current state to an
 *		disabled state (which implies that the device is in a
 *		non-operational state at the end of this function execution).
 *		Also, the dev_state must be assigned the value of
 *		DEV_STATE_DISABLE_OP
 *
 *		For devices such as port asic's and framers, this function
 *		be used to disable all the ports or channels of a specific
 *		device
 * 
 *		For IDT82V32021, one clock or multiple of clocks can be disabled
 *		at the same time, and it is passed in the parameter struct.
 *
 * Input: dev_object_t pointer to the IDT82V32021 device
 *
 * Returns: PASSED/FAILED
 *
 * Assumptions: The dev_init() function has been called and successfully
 *
 *****************************************************************/
static uint32
idt32021_dev_oper_dis(dev_object_t *dev)
{
    uint32 rc;
    dev_idt82v32021_object_t *idt32021 = (dev_idt82v32021_object_t *)dev;
    idt32021_callout_fvt_t *callout_p = idt32021->callout_fvt;
    char err_buf[80];

    /* Put the IDT82V32021 into reset state to disable */
    rc = (*callout_p->reset)(idt32021->i2c_p, DISABLE);

    if (rc != PASSED) {
	sprintf(err_buf, "idt32021_dev_oper_dis() reset return code %#x.", rc);
	DEV_ERROR_REPORT(dev, err_buf, (FATAL | IDT32021_OPER_DIS));
	return(FAILED);
    }

    wastetime(IDT32021_RESET_TIME);	/* Assert to meet min reset time */

    idt32021->base.dev_state = DEV_STATE_DISABLE_OP;

    return (PASSED);

}

/*****************************************************************
 * Name: idt32021_dev_intr_en
 *
 * Description: Enable the device interrupt(s).
 *		This function is expected to enable the interrupt(s) for a
 *		device with the expectation that all portions of the device
 *		are active, it is possible that some portions of a device are
 *		disabled.
 *
 * Input: dev_object_t pointer to the IDT82V32021 device
 *
 * Returns: PASSED/FAILED
 *
 * Assumptions: The dev_init() function has been called and successfully
 *
 *****************************************************************/
static uint32
idt32021_dev_intr_en (dev_object_t *dev)
{
    uint32 rc;
    uint i;
    char x_data;	/* Data bytes for I2C */
    n2g_i2c_if_t new_i2c_if;
    dev_idt32021_reg_init_t *reg_p;
    dev_idt82v32021_object_t *idt32021 = (dev_idt82v32021_object_t *)dev;
    idt32021_callout_fvt_t *callout_p = idt32021->callout_fvt;
    char err_buf[80];

    /* Setup I2C API interface struct */
    new_i2c_if.buf = &x_data;
    new_i2c_if.size = sizeof(x_data);
    new_i2c_if.i2c_bus_type = idt32021->i2c_p->i2c_bus_type;
    new_i2c_if.i2c_dev = idt32021->i2c_p->i2c_dev;

    /* Enable Interrupt Control interupts */
    for (i = 0, reg_p = &intr_en_reg[0];
	 i < (sizeof(intr_en_reg) / sizeof(dev_idt32021_reg_init_t));
	 i++, reg_p++) {
	x_data = reg_p->data;
	new_i2c_if.offset = reg_p->offset;

	/* Write to the IDT82V32021 with interrupts enabled */
	rc = callout_p->wr(&new_i2c_if);

	if (rc != PASSED) {
	    sprintf(err_buf, "idt32021_dev_intr_en() write %#x @ %#x rc %#x\n",
				reg_p->data, reg_p->offset, rc);
	    DEV_ERROR_REPORT(dev, err_buf, (FATAL | IDT32021_INTR_EN));
	    return(FAILED);
	}
    }

    return (PASSED);
}

/*****************************************************************
 * Name: idt32021_dev_intr_dis
 *
 * Description: Disable the device interrupt(s).
 *		This function is expected to disable ALL of the device
 *		interrupt(s).
 *   
 * Input: dev_object_t pointer to the IDT82V32021 device
 *
 * Returns: PASSED/FAILED
 *
 * Assumptions: The dev_init() function has been called and successfully
 *
 *****************************************************************/
static uint32
idt32021_dev_intr_dis (dev_object_t *dev)
{
    uint32 rc;
    uint i;
    char x_data;	/* Data bytes for I2C */
    n2g_i2c_if_t new_i2c_if;
    dev_idt32021_reg_init_t *reg_p;
    dev_idt82v32021_object_t *idt32021 = (dev_idt82v32021_object_t *)dev;
    idt32021_callout_fvt_t *callout_p = idt32021->callout_fvt;
    char err_buf[80];

    /* Setup I2C API interface struct */
    x_data = 0;		/* Disable all interrupts */
    new_i2c_if.buf = &x_data;
    new_i2c_if.size = sizeof(x_data);
    new_i2c_if.i2c_bus_type = idt32021->i2c_p->i2c_bus_type;
    new_i2c_if.i2c_dev = idt32021->i2c_p->i2c_dev;

    /* Disable Interrupt Control interrupts */
    for (i = 0, reg_p = &intr_en_reg[0];
	 i < (sizeof(intr_en_reg) / sizeof(dev_idt32021_reg_init_t));
	 i++, reg_p++) {
	new_i2c_if.offset = reg_p->offset;

	/* Write to the IDT82V32021 with interrupts disabled */
	rc = callout_p->wr(&new_i2c_if);

	if (rc != PASSED) {
	    sprintf(err_buf, "idt32021_dev_intr_dis() write 0 to %#x rc %#x\n",
				 reg_p->offset, rc);
	    DEV_ERROR_REPORT(dev, err_buf, (FATAL | IDT32021_INTR_DIS));
	    return(FAILED);
	}
    }

    return (PASSED);
}

/*****************************************************************
 * Name: idt32021_dev_isr
 *
 * Description: Process and handle the interrupt(s) on this device.
 *   
 * Input: dev_object_t pointer to the IDT82V32021 device
 *
 * Returns: PASSED/FAILED
 *
 * Assumptions: The dev_init() function has been called and successfully
 *
 *****************************************************************/
static uint32
idt32021_dev_isr (dev_object_t *dev)
{
    uint32 rc;
    uint i;
    char intr[IDT32021_NUM_INTRS]; /* Interrupt Status registers */
    n2g_i2c_if_t new_i2c_if;
    dev_idt82v32021_object_t *idt32021 = (dev_idt82v32021_object_t *)dev;
    idt32021_callout_fvt_t *callout_p = idt32021->callout_fvt;
    char err_buf[80];

    /* Setup I2C API interface struct */
    new_i2c_if.size = sizeof(char);
    new_i2c_if.i2c_bus_type = idt32021->i2c_p->i2c_bus_type;
    new_i2c_if.i2c_dev = idt32021->i2c_p->i2c_dev;

    /*
     * Read the interrupt status registers.
     * *** Need mechanism to pass status back to caller. print them out for now
     */
    for (i = 0, new_i2c_if.offset = IDT32021_INTERRUPT1_STS;
	 i < IDT32021_NUM_INTRS; i++, new_i2c_if.offset++) {
	new_i2c_if.buf = &intr[i];

	/* Read the interrupt status from the IDT82V32021 */
	rc = callout_p->rd(&new_i2c_if);

	if (rc != PASSED) {
	    sprintf(err_buf, "idt32021_dev_isr() read from %#x rc %#x\n",
				 new_i2c_if.offset, rc);
	    DEV_ERROR_REPORT(dev, err_buf, (FATAL | IDT32021_ISR));
	    return(FAILED);
	}
	printf("\nIDT82V32021 Interrupt Status %d = %p\n", intr[i], 
		(void *)&intr[i]);

	/* Write the same value back to clear */
	rc = callout_p->wr(&new_i2c_if);

	if (rc != PASSED) {
	    sprintf(err_buf, "idt32021_dev_isr() write to %#x rc %#x\n",
				 new_i2c_if.offset, rc);
	    DEV_ERROR_REPORT(dev, err_buf, (FATAL | IDT32021_ISR));
	    return(FAILED);
	}

    }

    /* Mask off the interrupts to prevent solid interrupts */
    return(idt32021_dev_intr_dis(dev));
}

/*****************************************************************
 * Name: idt32021_dev_show
 *
 * Description:	Provide platforms with a mechanism to display some common
 *		device information via the device print function argument.
 *
 * Input: dev_object_t pointer to the IDT82V32021 device
 *	  A device print function vector
 *	  A dev_show_cmd_e command
 *
 * Returns: PASSED/FAILED
 *
 * Assumptions: The device printf function vector has been provided by the host
 *              platform which implements the print logging functionality. The
 *              dev_attach() function has been called and successfully executed
 *
 *****************************************************************/
static uint32
idt32021_dev_show(dev_object_t *dev, print_fn_t dev_print, dev_show_cmd cmd)
{
    uint32 rc;
    uint i;
    char data;	/* data byte from 32021 */
    reg_info_t *reg_p;
    n2g_i2c_if_t new_i2c_if;
    dev_idt82v32021_object_t *idt32021 = (dev_idt82v32021_object_t *)dev;
    idt32021_callout_fvt_t *callout_p = idt32021->callout_fvt;
    char err_buf[80];

    /* Setup I2C API interface struct */
    new_i2c_if.size = sizeof(char);
    new_i2c_if.buf = &data;
    new_i2c_if.i2c_bus_type = idt32021->i2c_p->i2c_bus_type;
    new_i2c_if.i2c_dev = idt32021->i2c_p->i2c_dev;

    dev_print("\n	IDT82V32021 EBU WAN PLL Registers:\n");

    for (i = 0, reg_p = &idt32021_default_reg_tbl[0];
	 i < (sizeof(idt32021_default_reg_tbl) / sizeof(reg_info_t));
	 i++, reg_p++) {
	/* Read the data bytes of IDT82V32021 */
	new_i2c_if.offset = reg_p->offset;
	rc = (*callout_p->rd)(&new_i2c_if);

	if (rc != PASSED) {
	    sprintf(err_buf, "idt32021_dev_show() read rc = 0x%#08x", rc);
	    DEV_ERROR_REPORT(dev, err_buf, (FATAL | IDT32021_SHOW));
	    return(FAILED);
	}

#ifdef I2C_DEBUG
	printf("data = %#x @ %#x\n", data, &data);
#endif /* I2C_DEBUG */

	switch (cmd) {
	case DEV_SHOW_ALL:
	case DEV_SHOW_CONFIG:
	case DEV_SHOW_REGISTERS:
	    /* Print the read register */
	    dev_print("%s @ offset %#x = 0x%02X\n",
		       reg_p->name, reg_p->offset, data);
	    break;
	case DEV_SHOW_BRIEF:
	    if (i % IDT32021_SHOW_LINE_BREAK == 0) {
		dev_print("\n");	/* Newline */
	    }
	    dev_print("*(0x%02x) = 0x%02X ", reg_p->offset, data);
	    break;
	default:
	    assert(!"idt320232021_show");
	    break;
	} /* endof switch */
    } /* endof for */

    return(PASSED);
}

/*****************************************************************
 * Name: idt32021_crsh
 *
 * Description:	Allow platforms to collect data from a device during a crash.
 *		Print data to the crash log (via the provide print error) using
 *		the appropriate verbisity level requested by the host
 *
 * Input: dev_object_t pointer to the IDT82V32021 device
 *        A crash print function vector.
 *        A verbosity level.
 *
 * Returns: PASSED/FAILED
 *
 * Assumptions: A device print function vector has been provided by the host
 *		platform which implements the crash logging functionality. It
 *		could be the mechanism to log info to the Compact Flash before
 *		the device crash and now retrieve them. The dev_attch()
 *		function has been called and successfully executed.
 *
 *****************************************************************/
static uint32
idt32021_crsh(dev_object_t *dev, print_fn_t dev_print, dev_show_cmd verbosity)
{

    /* more development in this section */
    dev_print("idt32021_crsh(): No Crash info available for IDT82V32021\n");
    return(PASSED);
}

/*****************************************************************
 * Name: idt32021_destroy
 *
 * Description:	Destroy the dev_object structure and free all the resources.
 *
 * Input: dev_object_t pointer to the IDT82V32021 device
 *
 * Returns: none
 *
 * Assumptions: The dev_attch() function has been called and successfully
 *
 *****************************************************************/
static void
idt32021_destroy(dev_object_t **dev)
{
    uint32_t rc;
    dev_idt82v32021_object_t *idt32021;
    char err_buf[80];

    if (dev == NULL) {
	return;
    }

    if (*dev == NULL) {
	return;
    }

    idt32021 = (dev_idt82v32021_object_t *)*dev;

    /* Unlock the I2C device */
    if ((rc = idt32021->callout_fvt->close(idt32021->i2c_p)) != PASSED) {
	sprintf(err_buf, "idt32021_dev_destroy() I2C close ret. code %#x", rc);
	DEV_ERROR_REPORT(idt32021, err_buf, (FATAL | IDT32021_I2C_DESTROY));
	return;
    }

    if (idt32021->callout_fvt) {
	free(idt32021->callout_fvt);	/* Free callout struct */
    }

    if (idt32021->callin_fvt) {
	free(idt32021->callin_fvt);	/* Free callin struct */
    }

    free(idt32021->base.dev_object_fvt);	/* Free dev_object_t */
}

/**********************************************************************
 *
 * Function: idt32021_reg_test
 *
 * This function: tests IDT82V32021 registers.
 *		  Also check the ID of the chip.
 *
 * Input : dev - Pointer to the IDT82V32021 device object
 *
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
static int
idt32021_reg_test(dev_object_t *dev)
{
    char data;
    uint32 rc;
    uint i;
    reg_info_t *reg_ptr, *reg_p;
    dev_idt82v32021_object_t *idt32021 = (dev_idt82v32021_object_t *)dev;
    idt32021_callout_fvt_t *callout_p = idt32021->callout_fvt;
    char err_buf[80];
    n2g_i2c_if_t new_i2c_if;
    dev_idt32021_reg_init_t *id_reg_p;

    /* Setup I2C API interface struct */
    new_i2c_if.size = sizeof(data);
    new_i2c_if.buf = &data;
    new_i2c_if.i2c_bus_type = idt32021->i2c_p->i2c_bus_type;
    new_i2c_if.i2c_dev = idt32021->i2c_p->i2c_dev;

    if (idt32021->reg_table_p) {
	/* User provided register table */
	reg_ptr = idt32021->reg_table_p;
    } else {
	/* Use default register table */
	reg_ptr = &idt32021_default_reg_tbl[0];
    }

    /* Read the ID data byte of IDT82V32021 */
    for (i = 0, id_reg_p = &id_reg[0]; i < (sizeof(id_reg) /
		sizeof(dev_idt32021_reg_init_t)); i++, id_reg_p++) {
	new_i2c_if.offset = id_reg_p->offset;
	rc = (*callout_p->rd)(&new_i2c_if);

	if (rc != PASSED) {
	    sprintf(err_buf, "idt32021_reg_test() read return code %#x @ %#x",
						rc, id_reg_p->offset);
	    DEV_ERROR_REPORT(dev, err_buf, (FATAL | IDT32021_REG_TEST));
	    return(FAILED);
	}

	if (data != id_reg_p->data) {
	    /* ID does not match to the chip type */
	    sprintf(err_buf, "idt32021_reg_test() ID %#.2x. Expect %#.2x.",
						data, id_reg_p->data);
	    DEV_ERROR_REPORT(dev, err_buf, (FATAL | IDT32021_REG_TEST));
	    return(FAILED);
	}
    }

    /* Setup ext struct for register_tests() */
    reg_p = reg_ptr;
    while(reg_p->name) {
	/* Register name provided */
	if (reg_p->size.ext == 0) {
	    /* Need to setup registers test */
	    reg_p->size.ext = &reg_ext;
	}
	reg_p->size.ext->param = (void *)dev;
	reg_p++;
    }

    /* registers_test() will call idt32021_i2c_rd/wr() through reg_info_t_ext
     * struct's rd_ptr and wr_ptr.
     */
    return (register_tests(0, reg_ptr));

}

/**********************************************************************
 *
 * Function: idt32021_i2c_rd
 *
 * This function: reads IDT82V32021 registers
 *
 * Input : addr - offset of register to be read.
 *	   size - number of bytes to be read.
 *	   buf  - points to the data buffer to be read.
 *	   param - Pointer to the IDT82V32021 device object
 *
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
static int
idt32021_i2c_rd(ulong addr, int size, ulong *buf, void *param)
{
    uint32 rc;
    int i;
    uchar *data_p;
    char err_buf[80];
    n2g_i2c_if_t i2c_if;
    dev_idt82v32021_object_t *idt32021 = (dev_idt82v32021_object_t *)param;
    idt32021_callout_fvt_t *callout_p = idt32021->callout_fvt;

    /* Size cannot exceed the buffer size */
    if (size >= (int)sizeof(ulong) - (int)sizeof(uchar)) {
	sprintf(err_buf, "idt32021_i2c_rd() invalid size %#x", size);
	DEV_ERROR_REPORT((dev_object_t *)param, err_buf, (FATAL | IDT32021_I2C_READ));
	return(FAILED);
    }

    /* Setup the interface struct for I2C API read */
    i2c_if.offset = addr;
    i2c_if.buf = (char *)buf;
    i2c_if.size = size + sizeof(uchar);
    i2c_if.i2c_bus_type = idt32021->i2c_p->i2c_bus_type;
    i2c_if.i2c_dev = idt32021->i2c_p->i2c_dev;

    /* Call the I2C Read API */
    rc = callout_p->rd(&i2c_if);
    if (rc != PASSED) {
	/* Read failed */
	sprintf(err_buf, "idt32021_i2c_rd() read return code %#x", rc);
	DEV_ERROR_REPORT((dev_object_t *)param, err_buf, (FATAL | IDT32021_I2C_READ));
	return(FAILED);
    } else {
	/* Got the data */
	/* First byte is the byte count of the read. We can ignore it, and
	 * move the data bytes up a byte.
	 */
	for (i = 0, data_p = (uchar *)buf; i < size; i++, data_p++) {
	    *data_p = (uchar)(*buf >> (8 * (sizeof(ulong) - 1 - i)));
	}
	return(PASSED);
    }
}

/**********************************************************************
 *
 * Function: idt32021_i2c_wr
 *
 * This function: writes IDT82V32021 registers
 *
 * Input : addr - offset of register to be written.
 *         size - number of bytes to write.
 *         data  - write data.
 *         param - Pointer to the IDT82V32021 device object
 *
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
static int
idt32021_i2c_wr(ulong addr, int size, ulong data, void *param)
{
    uint32 rc;
    n2g_i2c_if_t i2c_if;
    dev_idt82v32021_object_t *idt32021 = (dev_idt82v32021_object_t *)param;
    idt32021_callout_fvt_t *callout_p = idt32021->callout_fvt;
    ulong data_buf;
    char err_buf[80];

    /* Size cannot exceed the buffer size */
    if (size >= (int)sizeof(ulong) - (int)sizeof(uchar)) {
	sprintf(err_buf, "idt32021_i2c_wr() invalid size %#x", size);
	DEV_ERROR_REPORT((dev_object_t *)param, err_buf, (FATAL | IDT32021_I2C_WRITE));
	return(FAILED);
    }

    /* IDT82V32021 protocol has data byte count as part of the data transfer.
     * Setup the new data buffer for the write.
     */
    data_buf = (size << (8 * (sizeof(unsigned int) - sizeof(uchar))));
    data_buf |= (data >> 8);

    /* Setup the interface struct for I2C API write */
    i2c_if.offset = addr;
    i2c_if.buf = (char *)&data_buf;
    i2c_if.size = size + sizeof(uchar);
    i2c_if.i2c_bus_type = idt32021->i2c_p->i2c_bus_type;
    i2c_if.i2c_dev = idt32021->i2c_p->i2c_dev;

    /* Call the I2C Write API */
    rc = callout_p->wr(&i2c_if);
    if (rc != PASSED) {
	/* Read failed */
	sprintf(err_buf, "idt32021_i2c_wr() write return code %#x", rc);
	DEV_ERROR_REPORT((dev_object_t *)param, err_buf, (FATAL | IDT32021_I2C_WRITE));
	return(FAILED);
    } else {
	/* Data written */
	return(PASSED);
    }
}


/******** History ******** 
$Log: dev_32021.c,v $
Revision 1.5  2013/11/26 08:40:32  hroni
fix compiler warning

Revision 1.4  2012/06/04 10:06:01  aarwang
- Clean up compiler warnings.

Revision 1.3  2012/03/28 00:38:07  mcharon
remove forward slash from second line

Revision 1.2  2012/03/25 03:57:59  palin2
Clean up Overlord Intel side (x86) compile warnings.

Revision 1.1.1.1  2012/03/23 23:02:01  ptong
Initial archive of ng_diag module


$Endlog$
*/
