/* $Id: platform_pwr_seq.c,v 1.26 2019/09/11 07:18:15 alpeng Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/overlord/utah/platform_pwr_seq.c,v $
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
 * Copyright (c) 2014-2019 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
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
#include "linux_api.h"

#define MAX_REGION 2
#define BIN_FW "pseq.bin"
#define SREC_DATA_SZ 32
#define FW_BIN_SZ 0x10000
#define WD_BYPASS /* */
#define PWR_SEQ_FW_DEFAULT_PATH "pseq.s19"

#define TO_EEPROM
/* Function prototypes */

extern int srec2bin_main(int, char *argv[], int*, int*);
extern void print_spining_wheel(int pass);
extern uint32_t n2g_i2c_write(n2g_i2c_if_t *i2c_p);
extern uint32_t n2g_i2c_read(n2g_i2c_if_t *i2c_p);
extern uint32 err_report (dev_object_t *dev, char *err_msg,
			  uint32 err_type); /* in hwic_spidey_ct3.c */
extern int do_all_menu_items(struct menuinfo *);
static int pwr_seq_eeprom_wr(void);
static int pwr_seq_eeprom_rd(void);
static int dump_pwr_seq_fw(void);
static int verify_pwr_seq_fw(void);
static int get_pwr_seq_i2c_struct(n2g_i2c_if_t *);
static int show_reg(void);
static int get_pwr_seq_fw_rev(int);
static int _pwr_seq_eeprom_rd (uint16_t to_addr, uint16_t *rd_data);
static int alter_reg(void);
static int alter_dump_blk_cmd_reg(void);
static int pwr_reg_test(int);
int pwr_read(n2g_i2c_if_t *, char *);  /* extern for i2c scan test */
static int pwr_stat_test(int);
static int pwr_ctr_test(int);
static int pwr_vtg_check(int);
static int pwr_stat_check(char *);
static int pwr_seq_read_reg(int, uint16_t *);
static int pwr_seq_write_reg(int, uint16_t);
static int pwr_write(n2g_i2c_if_t *);
static int get_pwr_seq_iadc(uint16_t *);
static int get_pwr_seq_vadc(uint16_t *);
static void show_volt_margin(uint, uint, int);
static int dump_pwr_seq_reg(void);
static int write_pwr_seq_fw(int);
static uint16_t pwr_seq_get_val(int16_t, uint);
static uint16_t start[MAX_REGION]   = {0x8000, 0x1000};
static uint16_t end[MAX_REGION]     = {0xFC00, 0x1400};

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
    {"Debug Pin Code",                      PWR_SEQ_PIN_CODE,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT},
	0x0000, 0x0000},
    {"Status: Sticky",                   PWR_SEQ_STA_S,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT},
	0x0000, 0x0000},
    {"Status: Clearable",               PWR_SEQ_STA_C,
	READ_WRITE | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT},
	0x0000, 0x0000},
    {"Fault0: Sticky",            PWR_SEQ_F0_S,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT},
	0x0000, 0x0000},
    {"Fault1: Sticky",         PWR_SEQ_F1_S,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT},
	0x0000, 0x0000},
    {"Fault0: Clearable",              PWR_SEQ_F0_C,
	READ_WRITE | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT},
	0x0000, 0x0000},
    {"Fault1: Clearable",         PWR_SEQ_F1_C,
	READ_WRITE | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT},
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
#if 0
    {"Spare",                   PWR_SEQ_SPARE,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT},
	0x0000, 0x0000},
#endif
    {"RTC Year/Month",                     PWR_SEQ_RTC_YM,
	READ_WRITE | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT},
	0x0000, 0x0000},
    {"RTC Date/Hour",                     PWR_SEQ_RTC_DH,
	READ_WRITE | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT},
	0x0000, 0x0000},
    {"RTC Min/Sec",                       PWR_SEQ_RTC_MS,
	READ_WRITE | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT},
	0x0000, 0x0000},
    {"Black Box  Command",                     PWR_SEQ_BB_C,
	READ_WRITE | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT},
	0xFFFF, 0x0000},
    {"Black Box Data 0 : RTC Year/Month",       PWR_SEQ_BB_D0,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT},
	0x0000, 0x0000},
    {"Black Box Data 1 : RTC Date/Hour",        PWR_SEQ_BB_D1,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT},
	0x0000, 0x0000},
    {"Black Box Data 2 :  RTC Minute/Second",     PWR_SEQ_BB_D2,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT},
	0x0000, 0x0000},
    {"Black Box Data 3: Fault Code",      PWR_SEQ_DD_D3,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT},
	0x0000, 0x0000},
    {"VP3P3 Voltage: Margin (-5, 0, 5)",         PWR_SEQ_3_3_MAR,
	READ_WRITE | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT},
	0xFFFF, 0x0000},
    {"VP2P5 Voltage: Margin (-5, 0, 5)",        PWR_SEQ_2_5_MAR,
	READ_WRITE | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT},
	0xFFFF, 0x0000},
    {"DDR Voltage: Margin (-5, 0, 5)",       PWR_SEQ_DDR_MAR,
	READ_WRITE | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT},
	0xFFFF, 0x0000},
    {"VP1P2 Voltage: Margin (-3 to 3)",	       PWR_SEQ_1_2_MAR, 
	READ_WRITE | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT},
	0xFFFF, 0x0000},
    {"VP1P0 Voltage: Margin (-3 to 3)",       PWR_SEQ_1_0_MAR,
	READ_WRITE | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT},
	0xFFFF, 0x0000},
    {"System Watchdog Enable",        PWR_SEQ_WD_EN, 
	READ_WRITE | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT},
	0x0000, 0x0000},
    {"System Watchdog Refresh",      PWR_SEQ_WD_REF,
	READ_WRITE | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT},
	0x0000, 0x0000},
    {"System Watchdog Timeout",      PWR_SEQ_WD_TO, 
	READ_WRITE | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT},
	0x0000, 0x0000},
    {"Power Down Enable",               PWR_SEQ_PR_D_EN,
	READ_WRITE | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT},
	0x0000, 0x0000},
    {"Power Down Time",                 PWR_SEQ_PW_D_T,
	READ_WRITE | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT},
	0x0000, 0x0000},
    {"VP12P0 Voltage : Latest Reading",    PWR_SEQ_12_0_LR,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT},
	0x0000, 0x0000},
    {"VP12P0 Voltage: Maximum Reading",    PWR_SEQ_12_0_MAX,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT},
	0x0000, 0x0000},
    {"VP12P0 Voltage: Minimum Reading",    PWR_SEQ_12_0_MIN, 
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT},
	0x0000, 0x0000},
    {"VP12P0 Voltage: Ramp Time Maximum and  Reading",  PWR_SEQ_12_0_RPT,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT},
	0x0000, 0x0000},
    {"VP12P0 Voltage Current : Latest Reading",    PWR_SEQ_12_0_C_LR,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT},
	0x0000, 0x0000},
    {"VP12P0 Voltage Current : Maximum Reading",    PWR_SEQ_12_0_C_MAX,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT},
	0x0000, 0x0000},
    {"VP12P0 Voltage Current : Minimum Reading",    PWR_SEQ_12_0_C_MIN, 
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT},
	0x0000, 0x0000},
    {"VP12P0 Voltage Current : Ramp Time Maximum and  Reading",  PWR_SEQ_12_0_C_RPT,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT},
	0x0000, 0x0000},
    {"VP5P0 Voltage : Latest Reading",    PWR_SEQ_5_0_LR,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT},
	0x0000, 0x0000},
    {"VP5P0 Voltage: Maximum Reading",    PWR_SEQ_5_0_MAX,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT},
	0x0000, 0x0000},
    {"VP5P0 Voltage: Minimum Reading",    PWR_SEQ_5_0_MIN, 
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT},
	0x0000, 0x0000},
    {"VP5P0 Voltage: Ramp Time Maximum and  Reading",  PWR_SEQ_5_0_RPT,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT},
	0x0000, 0x0000},
    {"VP3P3 Voltage : Latest Reading",    PWR_SEQ_3_3_LR,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT},
	0x0000, 0x0000},
    {"VP3P3 Voltage: Maximum Reading",    PWR_SEQ_3_3_MAX,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT},
	0x0000, 0x0000},
    {"VP3P3 Voltage: Minimum Reading",    PWR_SEQ_3_3_MIN, 
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT},
	0x0000, 0x0000},
    {"VP3P3 Voltage: Ramp Time Maximum and  Reading",  PWR_SEQ_3_3_RPT,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT},
	0x0000, 0x0000},
    {"VP2P5 Voltage : Latest Reading",    PWR_SEQ_2_5_LR,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT},
	0x0000, 0x0000},
    {"VP2P5 Voltage: Maximum Reading",    PWR_SEQ_2_5_MAX,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT},
	0x0000, 0x0000},
    {"VP2P5 Voltage: Minimum Reading",    PWR_SEQ_2_5_MIN, 
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT},
	0x0000, 0x0000},
    {"VP2P5 Voltage: Ramp Time Maximum and  Reading",  PWR_SEQ_2_5_RPT,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT},
	0x0000, 0x0000},
    {"VP1P8 Voltage : Latest Reading",    PWR_SEQ_1_8_LR,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT},
	0x0000, 0x0000},
    {"VP1P8 Voltage: Maximum Reading",    PWR_SEQ_1_8_MAX,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT},
	0x0000, 0x0000},
    {"VP1P8 Voltage: Minimum Reading",    PWR_SEQ_1_8_MIN, 
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT},
	0x0000, 0x0000},
    {"VP1P8 Voltage: Ramp Time Maximum and  Reading",  PWR_SEQ_1_8_RPT,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT},
	0x0000, 0x0000},	
    {"VP1P5 Voltage : Latest Reading",    PWR_SEQ_1_5_LR,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT},
	0x0000, 0x0000},
    {"VP1P5 Voltage: Maximum Reading",    PWR_SEQ_1_5_MAX,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT},
	0x0000, 0x0000},
    {"VP1P5 Voltage: Minimum Reading",    PWR_SEQ_1_5_MIN, 
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT},
	0x0000, 0x0000},
    {"VP1P5 Voltage: Ramp Time Maximum and  Reading",  PWR_SEQ_1_5_RPT,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT},
	0x0000, 0x0000},
    {"VP1P2 Voltage : Latest Reading",    PWR_SEQ_1_2_LR,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT},
	0x0000, 0x0000},
    {"VP1P2 Voltage: Maximum Reading",    PWR_SEQ_1_2_MAX,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT},
	0x0000, 0x0000},
    {"VP1P2 Voltage: Minimum Reading",    PWR_SEQ_1_2_MIN, 
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT},
	0x0000, 0x0000},
    {"VP1P2 Voltage: Ramp Time Maximum and  Reading",  PWR_SEQ_1_2_RPT,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT},
	0x0000, 0x0000},
    {"VP1P0 Voltage : Latest Reading",    PWR_SEQ_1_0_LR,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT},
	0x0000, 0x0000},
    {"VP1P0 Voltage: Maximum Reading",    PWR_SEQ_1_0_MAX,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT},
	0x0000, 0x0000},
    {"VP1P0 Voltage: Minimum Reading",    PWR_SEQ_1_0_MIN, 
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT},
	0x0000, 0x0000},
    {"VP1P0 Voltage: Ramp Time Maximum and  Reading",  PWR_SEQ_1_0_RPT,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT},
	0x0000, 0x0000},
    {"VP0P75_DDR_VTT Voltage : Latest Reading",    PWR_SEQ_0_75_DDR_LR,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT},
	0x0000, 0x0000},
    {"VP0P75_DDR_VTT Voltage: Maximum Reading",    PWR_SEQ_0_75_DDR_MAX,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT},
	0x0000, 0x0000},
    {"VP0P75_DDR_VTT Voltage: Minimum Reading",    PWR_SEQ_0_75_DDR_MIN, 
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT},
	0x0000, 0x0000},
    {"VP0P75_DDR_VTT Voltage: Ramp Time Maximum and  Reading",  PWR_SEQ_0_75_DDR_RPT,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT},
	0x0000, 0x0000},
    {"VP1P5_DDR Voltage : Latest Reading",    PWR_SEQ_1_5_DDR_LR,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT},
	0x0000, 0x0000},
    {"VP1P5_DDR Voltage: Maximum Reading",    PWR_SEQ_1_5_DDR_MAX,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT},
	0x0000, 0x0000},
    {"VP1P5_DDR Voltage: Minimum Reading",    PWR_SEQ_1_5_DDR_MIN, 
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT},
	0x0000, 0x0000},
    {"VP1P5_DDR Voltage: Ramp Time Maximum and  Reading",  PWR_SEQ_1_5_DDR_RPT,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT},
	0x0000, 0x0000},
    {"VP1P35_CPU  Voltage : Latest Reading",    PWR_SEQ_1_35_CPU_LR,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT},
	0x0000, 0x0000},
    {"VP1P35_CPU  Voltage: Maximum Reading",    PWR_SEQ_1_35_CPU_MAX,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT},
	0x0000, 0x0000},
    {"VP1P35_CPU  Voltage: Minimum Reading",    PWR_SEQ_1_35_CPU_MIN, 
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT},
	0x0000, 0x0000},
    {"VP1P35_CPU  Voltage: Ramp Time Maximum and  Reading",  PWR_SEQ_1_35_CPU_RPT,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT},
	0x0000, 0x0000},
    {"VP1P0_VCC Voltage : Latest Reading",    PWR_SEQ_1_0_VCC_LR,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT},
	0x0000, 0x0000},
    {"VP1P0_VCC Voltage: Maximum Reading",    PWR_SEQ_1_0_VCC_MAX,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT},
	0x0000, 0x0000},
    {"VP1P0_VCC Voltage: Minimum Reading",    PWR_SEQ_1_0_VCC_MIN, 
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT},
	0x0000, 0x0000},
    {"VP1P0_VCC Voltage: Ramp Time Maximum and  Reading",  PWR_SEQ_1_0_VCC_RPT,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT},
	0x0000, 0x0000},
    {"VP1P0_VNN Voltage : Latest Reading",    PWR_SEQ_1_0_VNN_LR,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT},
	0x0000, 0x0000},
    {"VP1P0_VNN Voltage: Maximum Reading",    PWR_SEQ_1_0_VNN_MAX,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT},
	0x0000, 0x0000},
    {"VP1P0_VNN Voltage: Minimum Reading",    PWR_SEQ_1_0_VNN_MIN, 
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT},
	0x0000, 0x0000},
    {"VP1P0_VNN Voltage: Ramp Time Maximum and  Reading",  PWR_SEQ_1_0_VNN_RPT,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT},
	0x0000, 0x0000},
    {"VP1P8_CPU Voltage : Latest Reading",    PWR_SEQ_1_8_CPU_LR,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT},
	0x0000, 0x0000},
    {"VP1P8_CPU Voltage: Maximum Reading",    PWR_SEQ_1_8_CPU_MAX,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT},
	0x0000, 0x0000},
    {"VP1P8_CPU Voltage: Minimum Reading",    PWR_SEQ_1_8_CPU_MIN, 
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT},
	0x0000, 0x0000},
    {"VP1P8_CPU Voltage: Ramp Time Maximum and  Reading",  PWR_SEQ_1_8_CPU_RPT,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT},
	0x0000, 0x0000},
    {"VP1P1_CPU Voltage : Latest Reading",    PWR_SEQ_1_1_CPU_LR,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT},
	0x0000, 0x0000},
    {"VP1P1_CPU Voltage: Maximum Reading",    PWR_SEQ_1_1_CPU_MAX,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT},
	0x0000, 0x0000},
    {"VP1P1_CPU Voltage: Minimum Reading",    PWR_SEQ_1_1_CPU_MIN, 
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT},
	0x0000, 0x0000},
    {"VP1P1_CPU Voltage: Ramp Time Maximum and  Reading",  PWR_SEQ_1_1_CPU_RPT,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT},
	0x0000, 0x0000},
    {"VP1P0_CPU Voltage : Latest Reading",    PWR_SEQ_1_0_CPU_LR,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT},
	0x0000, 0x0000},
    {"VP1P0_CPU Voltage: Maximum Reading",    PWR_SEQ_1_0_CPU_MAX,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT},
	0x0000, 0x0000},
    {"VP1P0_CPU Voltage: Minimum Reading",    PWR_SEQ_1_0_CPU_MIN, 
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT},
	0x0000, 0x0000},
    {"VP1P0_CPU Voltage: Ramp Time Maximum and  Reading",  PWR_SEQ_1_0_CPU_RPT,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT},
	0x0000, 0x0000},
    {"NIM Current : Latest Reading",    PWR_SEQ_NIM_CR_LR,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT},
	0x0000, 0x0000},
    {"NIM Current : Maximum Reading",    PWR_SEQ_NIM_CR_MAX,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT},
	0x0000, 0x0000},
    {"NIM Current : Minimum Reading",    PWR_SEQ_NIM_CR_MIN, 
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT},
	0x0000, 0x0000},
    {"NIM Current : Ramp Time Maximum and  Reading",  PWR_SEQ_NIM_CR_RPT,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT},
	0x0000, 0x0000},	
    {"FAN Current : Latest Reading",    PWR_SEQ_FAN_CR_LR,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT},
	0x0000, 0x0000},
    {"FAN Current : Maximum Reading",    PWR_SEQ_FAN_CR_MAX,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT},
	0x0000, 0x0000},
    {"FAN Current : Minimum Reading",    PWR_SEQ_FAN_CR_MIN, 
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT},
	0x0000, 0x0000},
    {"FAN Current : Ramp Time Maximum and  Reading",  PWR_SEQ_FAN_CR_RPT,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT},
	0x0000, 0x0000},
    {"VP3P3 Standby Voltage : Latest Reading",    PWR_SEQ_3_3_STV_LR,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT},
	0x0000, 0x0000},
    {"VP3P3 Standby Voltage : Maximum Reading",    PWR_SEQ_3_3_STV_MAX,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT},
	0x0000, 0x0000},
    {"VP3P3 Standby Voltage : Minimum Reading",    PWR_SEQ_3_3_STV_MIN, 
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT},
	0x0000, 0x0000},
    {"VP3P3 Standby Voltage : Ramp Time Maximum and  Reading",  PWR_SEQ_3_3_STV_RPT,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT},
	0x0000, 0x0000},
    {"VP3P0 Voltage : Latest Reading",    PWR_SEQ_3_0_LR,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT},
	0x0000, 0x0000},
    {"VP3P0 Voltage : Maximum Reading",    PWR_SEQ_3_0_MAX,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT},
	0x0000, 0x0000},
    {"VP3P0 Voltage : Minimum Reading",    PWR_SEQ_3_0_MIN, 
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT},
	0x0000, 0x0000},
    {"VP3P0 Voltage : Ramp Time Maximum and  Reading",  PWR_SEQ_3_0_RPT,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT},
	0x0000, 0x0000},
    {"MON21 Spare : Latest Reading",    PWR_SEQ_MON21_LR,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT},
	0x0000, 0x0000},
    {"MON21 Spare : Maximum Reading",    PWR_SEQ_MON21_MAX,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT},
	0x0000, 0x0000},
    {"MON21 Spare : Minimum Reading",    PWR_SEQ_MON21_MIN, 
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT},
	0x0000, 0x0000},
    {"MON21 Spare : Ramp Time Maximum and  Reading",  PWR_SEQ_MON21_RPT,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT},
	0x0000, 0x0000},
    {"DEBUG : Latest Reading",    PWR_SEQ_DEBUG_LR,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT},
	0x0000, 0x0000},
    {"DEBUG : Maximum Reading",    PWR_SEQ_DEBUG_MAX,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT},
	0x0000, 0x0000},
    {"DEBUG : Minimum Reading",    PWR_SEQ_DEBUG_MIN, 
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT},
	0x0000, 0x0000},
    {"DEBUG : Ramp Time Maximum and  Reading",  PWR_SEQ_DEBUG_RPT,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT},
	0x0000, 0x0000},
    {"MON23 Spare : Latest Reading",    PWR_SEQ_MON23_LR,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT},
	0x0000, 0x0000},
    {"MON23 Spare : Maximum Reading",    PWR_SEQ_MON23_MAX,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT},
	0x0000, 0x0000},
    {"MON23 Spare : Minimum Reading",    PWR_SEQ_MON23_MIN, 
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT},
	0x0000, 0x0000},
    {"MON23 Spare : Ramp Time Maximum and  Reading",  PWR_SEQ_MON23_RPT,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT},
	0x0000, 0x0000},
    {"VREF Voltage : Latest Reading",    PWR_SEQ_VREF_V_LR,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT},
	0x0000, 0x0000},
    {"VREF Voltage : Maximum Reading",    PWR_SEQ_VREF_V_MAX,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT},
	0x0000, 0x0000},
    {"VREF Voltage : Minimum Reading",    PWR_SEQ_VREF_V_MIN, 
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT},
	0x0000, 0x0000},
    {"VREF Voltage : Ramp Time Maximum and  Reading",  PWR_SEQ_VREF_V_RPT,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT},
	0x0000, 0x0000},
    {"Temperature  : Latest Reading",    PWR_SEQ_TMP_LR,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT},
	0x0000, 0x0000},
    {"Temperature : Maximum Reading",    PWR_SEQ_TMP_MAX,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT},
	0x0000, 0x0000},
    {"Temperature : Minimum Reading",    PWR_SEQ_TMP_MIN, 
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT},
	0x0000, 0x0000},
    {"Temperature : Ramp Time Maximum and  Reading",  PWR_SEQ_TMP_RPT,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT},
	0x0000, 0x0000},
    {"End", 0xFFFFFFFF, READ_ONLY, {0}, 0, 0},
};

/* change to utah version */
static pwr_seq_v_t voltage_test_table[] =
{
    {"12 Volt",            PWR_SEQ_12_0_OVER,   PWR_SEQ_12_0_UNDER,
      PWR_SEQ_12_0_LR,     PWR_SEQ_12_0_MAX,    PWR_SEQ_12_0_MIN},
    {"5.0 Volt",           PWR_SEQ_5_0_OVER,    PWR_SEQ_5_0_UNDER, 
      PWR_SEQ_5_0_LR,      PWR_SEQ_5_0_MAX,     PWR_SEQ_5_0_MIN},
    {"3.3 Volt",           PWR_SEQ_3_3_OVER,    PWR_SEQ_3_3_UNDER,
      PWR_SEQ_3_3_LR,      PWR_SEQ_3_3_MAX,     PWR_SEQ_3_3_MIN},
    {"2.5 Volt",           PWR_SEQ_2_5_OVER,    PWR_SEQ_2_5_UNDER,
      PWR_SEQ_2_5_LR,      PWR_SEQ_2_5_MAX,     PWR_SEQ_2_5_MIN},
    {"1.8 Volt",           PWR_SEQ_1_8_OVER,    PWR_SEQ_1_8_UNDER, 
      PWR_SEQ_1_8_LR,      PWR_SEQ_1_8_MAX,     PWR_SEQ_1_8_MIN},
    {"1.5 Volt",           PWR_SEQ_1_5_OVER,    PWR_SEQ_1_5_UNDER,
      PWR_SEQ_1_5_LR,      PWR_SEQ_1_5_MAX,     PWR_SEQ_1_5_MIN},
    {"1.2 Volt",           PWR_SEQ_1_2_OVER,    PWR_SEQ_1_2_UNDER,
      PWR_SEQ_1_2_LR,      PWR_SEQ_1_2_MAX,     PWR_SEQ_1_2_MIN},
    {"1.0 Volt",           PWR_SEQ_1_0_OVER,    PWR_SEQ_1_0_UNDER,
      PWR_SEQ_1_0_LR,      PWR_SEQ_1_0_MAX,     PWR_SEQ_1_0_MIN},
    {"0.75 DDR Volt",      PWR_SEQ_0_75_DDR_OVER,   PWR_SEQ_0_75_DDR_UNDER, 
      PWR_SEQ_0_75_DDR_LR, PWR_SEQ_0_75_DDR_MAX,    PWR_SEQ_0_75_DDR_MIN},
    {"1.5 DDR Volt",       PWR_SEQ_1_5_DDR_OVER,    PWR_SEQ_1_5_DDR_UNDER,
      PWR_SEQ_1_5_DDR_LR,  PWR_SEQ_1_5_DDR_MAX,     PWR_SEQ_1_5_DDR_MIN},
    {"1.35 CPU Volt",      PWR_SEQ_1_35_CPU_OVER,   PWR_SEQ_1_35_CPU_UNDER,
      PWR_SEQ_1_35_CPU_LR, PWR_SEQ_1_35_CPU_MAX,    PWR_SEQ_1_35_CPU_MIN},
    {"1.0 VCC Volt",       PWR_SEQ_1_0_VCC_OVER,    PWR_SEQ_1_0_VCC_UNDER,
      PWR_SEQ_1_0_VCC_LR,  PWR_SEQ_1_0_VCC_MAX,     PWR_SEQ_1_0_VCC_MIN},
    {"1.0 VNN Volt",       PWR_SEQ_1_0_VNN_OVER,    PWR_SEQ_1_0_VNN_UNDER,
      PWR_SEQ_1_0_VNN_LR,  PWR_SEQ_1_0_VNN_MAX,     PWR_SEQ_1_0_VNN_MIN},
    {"1.8 CPU Volt",       PWR_SEQ_1_8_CPU_OVER,    PWR_SEQ_1_8_CPU_UNDER,
      PWR_SEQ_1_8_CPU_LR,  PWR_SEQ_1_8_CPU_MAX,     PWR_SEQ_1_8_CPU_MIN},
    {"1.1 CPU Volt",       PWR_SEQ_1_1_CPU_OVER,    PWR_SEQ_1_1_CPU_UNDER,
      PWR_SEQ_1_1_CPU_LR,  PWR_SEQ_1_1_CPU_MAX,     PWR_SEQ_1_1_CPU_MIN},
    {"1.0 CPU Volt",       PWR_SEQ_1_0_CPU_OVER,    PWR_SEQ_1_0_CPU_UNDER,
      PWR_SEQ_1_0_CPU_LR,  PWR_SEQ_1_0_CPU_MAX,     PWR_SEQ_1_0_CPU_MIN},
    {"NIM Current",        PWR_SEQ_NIM_CR_OVER, PWR_SEQ_NIM_CR_UNDER,
     PWR_SEQ_NIM_CR_LR,   PWR_SEQ_NIM_CR_MAX,   PWR_SEQ_NIM_CR_MIN},
    {"FAN Current",         PWR_SEQ_FAN_CR_OVER, PWR_SEQ_FAN_CR_UNDER,        
      PWR_SEQ_FAN_CR_LR,   PWR_SEQ_FAN_CR_MAX,  PWR_SEQ_FAN_CR_MIN},
    {"3.3 Standby Volt",   PWR_SEQ_3_3_STV_OVER, PWR_SEQ_3_3_STV_UNDER,
      PWR_SEQ_3_3_STV_LR,  PWR_SEQ_3_3_STV_MAX, PWR_SEQ_3_3_STV_MIN},
    {"3.0 Volt",          PWR_SEQ_3_0_OVER, PWR_SEQ_3_0_UNDER,
      PWR_SEQ_3_0_LR,      PWR_SEQ_3_0_MAX,     PWR_SEQ_3_0_MIN},
#if CHECK_SPARE_REGISTER 
    /* voltage check does not need to check these spare registers */
    {"MON21 Spare",        PWR_SEQ_MON21_OVER,  PWR_SEQ_MON21_UNDER,
      PWR_SEQ_MON21_LR,    PWR_SEQ_MON21_MAX,   PWR_SEQ_MON21_MIN},
    {"Debug",              PWR_SEQ_DEBUG_OVER,  PWR_SEQ_DEBUG_UNDER, 
      PWR_SEQ_DEBUG_LR,    PWR_SEQ_DEBUG_MAX,   PWR_SEQ_DEBUG_MIN},
    {"MON23 Spare",        PWR_SEQ_MON23_OVER,  PWR_SEQ_MON23_UNDER, 
      PWR_SEQ_MON23_LR,    PWR_SEQ_MON23_MAX,   PWR_SEQ_MON23_MIN},
#endif
    {"VREF Volt",          PWR_SEQ_VREF_V_OVER, PWR_SEQ_VREF_V_UNDER, 
      PWR_SEQ_VREF_V_LR,   PWR_SEQ_VREF_V_MAX,  PWR_SEQ_VREF_V_MIN},
    {"Temperature Volt",   PWR_SEQ_TMP_OVER,    PWR_SEQ_TMP_UNDER, 
      PWR_SEQ_TMP_LR,      PWR_SEQ_TMP_MAX,     PWR_SEQ_TMP_MIN},
    {NULL, 0, 0, 0, 0, 0},
};

/*
 * Power Sequencer Menu
 */
static submenu_xtable_t pwr_menu_table[] = {
    {"Update Power Sequencer FW", (PFT)write_pwr_seq_fw,   0,
        0,                              (type_t(*)())0, 0, (PFT)0, 0},
    {"Show Power Sequencer FW version", (PFT)get_pwr_seq_fw_rev,   1,
        0,                              (type_t(*)())0, 0, (PFT)0, 0},
    {"Show Power Sequencer registers",  (PFT)show_reg,             0,
        0,                              (type_t(*)())0, 0, (PFT)0, 0},
    {"Dump Power Sequencer registers (Hex Only)", (PFT)dump_pwr_seq_reg, 0,
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
    {"Dump Fw in eeprom to file pseq_dump.txt",   (PFT)dump_pwr_seq_fw,      TRUE,
        0,     (type_t(*)())0, 0, (PFT)0, 0},
    {"Compare files pseq_dump.txt and pseq_bin",   (PFT)verify_pwr_seq_fw,      TRUE,
        0,     (type_t(*)())0, 0, (PFT)0, 0},
    {"Write value to eeprom",   (PFT)pwr_seq_eeprom_wr,      TRUE,
        0,     (type_t(*)())0, 0, (PFT)0, 0},
    {"Read value from eeprom",   (PFT)pwr_seq_eeprom_rd,      TRUE,
        0,     (type_t(*)())0, 0, (PFT)0, 0},
    {"Alter & Show Black Box Cmd register",  (PFT)alter_dump_blk_cmd_reg, 0,
        0,     (type_t(*)())0, 0, (PFT)0, 0},
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

    rc = pwr_seq_read_reg(PWR_SEQ_REV, &rev);
    if (rc != PASSED) {
        if (option) {
	    cterr('f', 0, "%s:%d Failed to read Power Sequencer FW revision.",
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
    return (pwr_seq_read_reg(PWR_SEQ_12_0_C_LR, iadc));
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
    return (pwr_seq_read_reg(PWR_SEQ_12_0_LR, vadc));
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
    char err_buf[OVLD_BUF_SIZE *2];
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

            show_reg();
#if 0 /* call show_reg() directly */

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
#endif 

#ifdef PWR_DEBUG
        } else {
            printf("\n Pwr Seq. Status register = %#x\n", status);
#endif /* PWR_DEGUB */
        } /* endof if status */
    }

    return (rc);
}

/*********************************************************************
 *
 * Function:    vtg_mrgn
 *
 * Description: Margin read/write.
 *
 * Inputs:      option - Refer to voltage_margin_t enum.
 *              format - DISPLAY_HCI /DISPLAY_M2M
 *
 * Outputs:     PASSED/FAILED.
 *
 * Assumptions:
 *
 *********************************************************************
 */
int vtg_mrgn(int option, int format)
{
    uint32_t rc = FAILED;
    uint16_t val_offset, margin_val, read_val; 
    double pre_mrgn, aft_mrgn;
    uint margin, device, spe_op, offset;
    uint offset_lr;
    uint delay = 1; /* default delay time in secs */
    float normal_val;
    int i = 0;
    int margin_target;
    int pct_3_offset = 0; /* +/-3 percent offset */

    margin = option & 0xF;
    device = (option & 0xF0) >> 4;
    spe_op = (option & 0xF00) >> 8;

    switch(device) {
      case PWR_SEQ_VP3P3:  /* 0 (enum) */
          offset = PWR_SEQ_3_3_MAR; /* 0x15 */
          offset_lr = PWR_SEQ_3_3_LR; 
          normal_val = 3.3;
      break; 
      case PWR_SEQ_VP2P5:  
          offset = PWR_SEQ_2_5_MAR; /* 0x16 */
          offset_lr = PWR_SEQ_2_5_LR; 
          normal_val = 2.5;
      break; 
      case PWR_SEQ_DDR: 
          offset = PWR_SEQ_DDR_MAR; /* 0x17 */
          offset_lr = PWR_SEQ_1_5_DDR_LR;
          normal_val = 1.5;
      break; 
      /* 
       * Note: 
       * 1.0V and 1.2V expect 10-12secs to take effect,
       * now fix the delay time to 1s as HW request
       */
      case PWR_SEQ_VP1P2:  
          offset = PWR_SEQ_1_2_MAR; /* 0x18 */
          offset_lr = PWR_SEQ_1_2_LR; 
          normal_val = 1.2;
      break; 
      case PWR_SEQ_VP1P0:  
          offset = PWR_SEQ_1_0_MAR; /* 0x19 */
          offset_lr = PWR_SEQ_1_0_LR; 
          normal_val = 1.0;
      break; 
      default:
          cterr('f', 0, "%s: Failed to parse margined device on Power Seq.\n", 
              __FUNCTION__);
          return(FAILED);
      break; 
    }

    /* for show voltage margin */
    if (spe_op) {
        if ((rc = pwr_seq_read_reg(offset, &read_val)) != RC_I2C_OP_OK) {
           cterr('f', 0, "%s: Unable to read Regiser %#x.",
                  __FUNCTION__, offset);
            return (FAILED);
        }

        show_volt_margin(read_val, device, format);
        return(rc);
    }
    /* latest read voltage before margining */
    if ((rc = pwr_seq_read_reg(offset_lr, &read_val)) != RC_I2C_OP_OK) {
        cterr('f', 0, "%s: Unable to read Regiser %#x.",
            __FUNCTION__, offset);
        return (FAILED);
    }
    cterr_db_print("Before margining: %.4f volt, Hex: %#X\n", \
            read_val * PWR_SEQ_VOLT_PER_TICK, read_val);

    /* store previous margin value before adjusting */
    pre_mrgn = read_val * PWR_SEQ_VOLT_PER_TICK;
    margin_val = val_offset = 0;
    /* setting voltage margin */
    /* NOTE: margin range on PWR_SEQ_1_2_MAR and  PWR_SEQ_1_0_MAR are -9, 0 and 9 */
    if (offset > PWR_SEQ_DDR_MAR) {
        /* 
         * set val_offset = 4 to get 9% margin, 
         *     val_offset = 0 to get 5% margin,
         * 1V and 1.2V are able to do 9% margining, but it will be dangerous.
         * so Diag uses 3% margining for now
         */
        val_offset = 0;
    }
    if ( (offset == PWR_SEQ_1_2_MAR)||(offset == PWR_SEQ_1_0_MAR) ) {
        switch(margin) {
            pct_3_offset = 0;
            case MARGIN_HI:
                margin_val = (3 + pct_3_offset);
                margin_target = 103;/* high: +3% margin */
            break;
            case MARGIN_LO:
                margin_val = -(3 + pct_3_offset); 
                margin_target = 97;/* low: -3% margin */
            break;
            case MARGIN_NORM:
                margin_val = 0;
                margin_target = 100; /* norm: 0% margin */
            break;
            default:
                cterr('f',0,"%s: Failed to margin +/-3% dev on 1.2v Pwr Seq.\n",
                    __FUNCTION__);
                return(FAILED);
            break;
        }
    } else {
        switch(margin) { 
            case MARGIN_HI:
                margin_val = (5 + val_offset);
                margin_target = 105;/* high: +5% margin */
            break;
            case MARGIN_LO:
                margin_val = -(5 + val_offset); 
                margin_target = 95;/* low: -5% margin */
            break;
            case MARGIN_NORM:
                margin_val = 0;
                margin_target = 100; /* norm: 0% margin */
            break;
            default:
                cterr('f',0,"%s: Failed to margin range device on Power Seq.\n", 
                    __FUNCTION__);
                return(FAILED);
            break;
        }
    }

    rc = pwr_seq_write_reg(offset, margin_val);
    if (rc != PASSED) {
        cterr('f', 0, "%s: Failed to wrote Power Sequencer Reg %#x.",
              __FUNCTION__, offset);
        return(FAILED);
    } else {
        printf("Margin register is set.\n");
    }
    /* fix delay time to 1s for margining */
    sleep(delay);
    while(i < 3) {
        /* latest read voltage after margining */
        if ((rc = pwr_seq_read_reg(offset_lr, &read_val)) != RC_I2C_OP_OK) {
            cterr('f', 0, "%s: Unable to read Register %#x.",
                __FUNCTION__, offset);
            return (FAILED);
        }
        
        if(
         /* for N and H margining */
         ((margin_target >= 100) && (read_val * PWR_SEQ_VOLT_PER_TICK >=\
             (margin_target*0.99/100)*normal_val)) || \
         /* for L margining */
         ((margin_target < 100) && (read_val * PWR_SEQ_VOLT_PER_TICK <=\
             (margin_target*1.01/100)*normal_val))) {
            break;
        } else {
            //printf("waiting for target margin (count: %d)\n", i);
            /* target margining is not achieved yet, try to wait */
            /* it will spend several ms to run loop 3 times */
            i++;
        }
    }
    cterr_db_print("After margining: %.4f volt, Hex: %#X\n", \
            read_val * PWR_SEQ_VOLT_PER_TICK, read_val);
    /* store changed margin value */
    aft_mrgn = read_val * PWR_SEQ_VOLT_PER_TICK;
    cterr_db_print("Margining changed: %.4f volt\n",(aft_mrgn - pre_mrgn));
    return(rc); 
}

/*********************************************************************
 *
 * Function:    show_volt_margin
 *
 * Description: Display voltage margin.
 *
 * Inputs:      vmc - Voltage Margin Control register read.
 *              option - 1.8 v or 2.5/3.3v Use voltage_margin_t enum.
 *              format - DISPLAY_HCI /DISPLAY_M2M
 *
 * Outputs:     None
 *
 *********************************************************************
 */
static void show_volt_margin(uint vmc, uint option, int format)
{
    switch(option) {
    case PWR_SEQ_VP3P3:
    case PWR_SEQ_VP2P5:
    case PWR_SEQ_DDR:
        if (format == DISPLAY_HCI) {
            /* show in human computer interface */
            if (option == PWR_SEQ_VP3P3) {
                cterr_db_print("VP3P3:");
            } else if (option == PWR_SEQ_VP2P5) {
                cterr_db_print("VP2P5:");
            } else {
                cterr_db_print("DDR  :");
            }
        }else
        if (format == DISPLAY_M2M) {
            /* show in machine to machine */
            if (option == PWR_SEQ_VP3P3) {
                cterr_db_print("VP3P3:");
            } else if (option == PWR_SEQ_VP2P5) {
                cterr_db_print("VP2P5:");
            } else {
                cterr_db_print("DDR  :");
            }
        }else {
            prpass(testpass, "%s()",__func__);
            cterr('f',0,"Unknown format in %s function,"
                "\nSupport DISPLAY_HCI or DISPLAY_M2M,"
                "\nbut received format = %x ", __func__, format);
            return;
        }
        if (vmc != 0) {
            if (format == DISPLAY_HCI) {

                /* show in human computer interface */
                cterr_db_print("margined ");
            }

            if (vmc == 5) {
                cterr_db_print("+5%%\n");
            } else 
            if ((vmc & 0xFFFF) == 0xFFFB) {
                cterr_db_print("-5%%\n");
            } else {
                cterr_db_print("unknown value is 0x%x, %.4f volt\n", vmc, vmc*(2.702/4096));
            }
        } 
        break;
    case PWR_SEQ_VP1P2:
    case PWR_SEQ_VP1P0:
        if (format == DISPLAY_HCI) {
            /* show in human computer interface */
            if (option == PWR_SEQ_VP1P2) {
                cterr_db_print("VP1P2:");
            } else {
                cterr_db_print("VP1P0:");
            }
        }else
        if (format == DISPLAY_M2M) {
            /* show in machine to machine */
            if (option == PWR_SEQ_VP1P2) {
                cterr_db_print("VP1P2:");
            } else {
                cterr_db_print("VP1P0:");
            }
        }else {
            prpass(testpass, "%s()",__func__);
            cterr('f',0,"Unknown format in %s function,"
                "\nSupport DISPLAY_HCI or DISPLAY_M2M,"
                "\nbut received format = %x ", __func__, format);
            return;
        }
        if (vmc != 0) {
            if (format == DISPLAY_HCI) {

                /* show in human computer interface */
                cterr_db_print("margined ");
            }
            if (vmc == 5) {
                cterr_db_print("+5%%\n");
            } else 
            if ((vmc & 0xFFFF) == 0xFFFB) {
                cterr_db_print("-5%%\n");
            } else 
            if (vmc == 3){
                cterr_db_print("+3%%\n");
            } else 
            if ((vmc & 0xFFFF) == 0xFFFD) {
                cterr_db_print("-3%%\n");
            } else {
                cterr_db_print("unknown value is 0x%x\n", vmc);
            }
        }
        break;
    default:
        break;
    }

    if (vmc == 0) {
        if (format == DISPLAY_HCI) {
            /* show in human computer interface */
            /* Normal */
            cterr_db_print("not margined\n");
        }else
        if (format == DISPLAY_M2M) {
            /* show in machine to machine */
            cterr_db_print("MARGIN NO\n");
         }else {
            prpass(testpass, "%s()",__func__);
            cterr('f',0,"Unknown format in %s function,"
            "\nSupport DISPLAY_HCI or DISPLAY_M2M,"
                "\nbut received format = %x ", __func__, format);
            return;
        }

    }
    return;
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
    i2c_if.buf = (char *) data;
    i2c_if.offset = offset;
    i2c_if.size = 2;
    result = pwr_read(&i2c_if , &err_buf[0]);   
    if (result != PASSED) {
        /* Unable to read data */
        printf("%s: Unable to read(result = 0x%08x).\n", __FUNCTION__, result);
    }

    return (result);
}


/*******************************************************************************
 *
 * Function   : alter_dump_blk_cmd_reg
 * Description:	Alter Environment MCU Black Box Cmd Register and Display
 *              Related Result.
 * Inputs     : None
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
static int alter_dump_blk_cmd_reg (void)
{
    uint32_t rc = FAILED, i, offset;
    reg_info_t *reg_table_p;
    reg_info_t *reg_ptr;
    uint16_t tmp_mask, data = 0;
    uint16_t buffer = 0;

	
    printf("\nAlter register number [0x10]black box cmd\n");
    offset = PWR_SEQ_BB_C;
	
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

    /* these interval are margining registers, 
     * we need to provide negative write for user */
    if ((offset > PWR_SEQ_DD_D3) && (offset < PWR_SEQ_WD_EN)) {
        printf("It is margining register, supporting negative number write. \n");
        data = pwr_seq_get_val(data, offset);
    } else { 
        data = gethex_answer("Enter the 16-bit data:", data, 0, tmp_mask);
    }

    if ((rc = pwr_seq_write_reg(offset, data)) != PASSED) {
        cterr('f', 0, "%s: Failed to wrote Power Sequencer Reg %#x.",
              __FUNCTION__, offset);
    }
	
    reg_ptr = &pwr_seq_table[0];

    printf("\n");

    while (reg_ptr->offset != 0xFFFFFFFF) {
        /* Print black box cmd related registers 0x10~0x14 */ 
        offset = (reg_ptr->offset);
        /* the byteswap is activiated in env_read. */
        if (pwr_seq_read_reg(offset, &buffer) != RC_I2C_OP_OK) {
            cterr('f', 0, "%s: Unable to read Regiser %#x.",
                  __FUNCTION__, offset);
            return (FAILED);
        }
        if ((offset >= PWR_SEQ_BB_C) && (offset <= PWR_SEQ_DD_D3)) {
            printf("%-36s (0x%02X), data: 0x%04X.\n", reg_ptr->name,
               reg_ptr->offset, buffer);
        }
        reg_ptr++;
    }
	
    return (rc);
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

    /* these interval are margining registers, 
     * we need to provide negative write for user */
    if ((offset > PWR_SEQ_DD_D3) && (offset < PWR_SEQ_WD_EN)) {
        printf("It is margining register, supporting negative number write. \n");
        data = pwr_seq_get_val(data, offset);
    } else { 
        data = gethex_answer("Enter the 16-bit data:", data, 0, tmp_mask);
    }

    if ((rc = pwr_seq_write_reg(offset, data)) != PASSED) {
	cterr('f', 0, "%s: Failed to wrote Power Sequencer Reg %#x.",
              __FUNCTION__, offset);
    }

    return (rc);
}

static uint16_t
pwr_seq_get_val (int16_t data, uint offset) 
{
    char buf_a[32], buf_b[32];
    short tmp; 

    sprintf(buf_a, "%d", data);

    printf("Enter the data [%s]:  ", buf_a);
    fflush(stdout);
    get_line(buf_b,sizeof(buf_b));

    /* nothing changed, using old val */
    if(buf_b[0] == '\0' || buf_b[0] == '\r' || buf_b[0] == '\n') {
        data = strtol(buf_a, NULL, 8); 
        return(data);
    }

    tmp = strtol(buf_b, NULL, 8);
    return((uint16_t)tmp);
}



static uint16_t
cnt_record(char *fw_path, uint16_t *line)
{
    FILE *fp;
    uint16_t ret = PASSED;
    char buf[SREC_DATA_SZ* 2 + 32]; /*data + header, etc... */

    if ((fp = fopen(fw_path, "rb")) == NULL) {
        printf("unable to open srec file %s.\n", fw_path);
        perror("");
        return(FAILED);
    }
    *line = 0;
    while ( fgets(buf, sizeof(buf), fp) ) {
        *line = *line+1;
        if (*line == 0xFFFF) {
            printf("too many records in srec file\n");
            ret = FAILED;
            break;
        }
    }
    fclose(fp);
    if (*line <= 1) {
        printf("Too few records in srec file\n");
        ret = FAILED;
    }
    printf("%d records in srec file\n", *line);
    return(ret);
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
    //uint16_t data = 0;

    if (!i2c_if->buf) {
        assert(!"env_write: buf is null");
    }

    //data = DSWAP2(*(uint16_t *)i2c_if->buf); 
    //memcpy(i2c_if->buf, &data, sizeof(uint16_t));
    
    rc = n2g_i2c_write(i2c_if);
    if (rc != RC_I2C_OP_OK) {
	//sleep(REN_I2C_PROC_TIME);	/* Env MCU I2C cycle time */
        return (FAILED);
    }

    //    msleep(REN_I2C_PROC_TIME);	/* Env MCU I2C cycle time */
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
    uint16_t data = 0;
    uint16_t* ptr;
    char buf[2];
    if (!i2c_if->buf) {
        assert(!"env_read: buf is null");
    }
    ptr = (uint16_t*)i2c_if->buf;
    i2c_if->buf = &buf[0];
    rc = n2g_i2c_read(i2c_if);
    if (rc != RC_I2C_OP_OK) {
	/* Unable to read data */
	sprintf(err_buf, "%s:%d Failed to read Reg. %#x (rc = %#x).",
                __FUNCTION__, __LINE__, i2c_if->offset, rc);
	return (FAILED);
    }
    data = (buf[0] & 0xFF) + ((buf[1] & 0xFF) << 8);
    //printf("buf[0] %x, buf[1] %x data %x\n", buf[0], buf[1], data);
    i2c_if->buf = (char*) ptr;
    memcpy(i2c_if->buf, &data, sizeof(uint16_t));
    //    msleep(REN_I2C_PROC_TIME);	/* I2C cycle time */
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

       /* based on Utah HW eng. to test scratched register is enough to verify 
        * I2C register accessing. We don't want to test the other registers which 
        * will be monitored by firmware */
  
        if ((reg_ptr->offset < PWR_SEQ_SCR0) || (reg_ptr->offset > PWR_SEQ_SCR3)) {
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
        i2c_if.offset = PWR_SEQ_RTC_YM; 
        i2c_if.buf = (char *)&rtc_1_ms;

        if ((rc = pwr_read(&i2c_if, &err_buf[0])) != PASSED) {
            sprintf(err_buf, "%s: Unable to read MS byte first time.\n",
                             __FUNCTION__);
        } else {
            i2c_if.offset = PWR_SEQ_RTC_DH;
            i2c_if.buf = (char *)&rtc_1;

            if ((rc = pwr_read(&i2c_if, &err_buf[0])) != PASSED) {
                sprintf(err_buf, "%s: Unable to read middle byte first time.\n",
                                 __FUNCTION__);
            } else {
                i2c_if.offset = PWR_SEQ_RTC_MS;
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
        i2c_if.offset = PWR_SEQ_RTC_YM;
        i2c_if.buf = (char *)&rtc_2_ms;

        if ((rc = pwr_read(&i2c_if, &err_buf[0])) != PASSED) {
            sprintf(err_buf, "%s: Unable to read MS byte again.\n",
                             __FUNCTION__);
        } else {
            i2c_if.offset = PWR_SEQ_RTC_DH;
            i2c_if.buf = (char *)&rtc_2;

            if ((rc = pwr_read(&i2c_if, &err_buf[0])) != PASSED) {
                sprintf(err_buf, "%s: Unable to read middle again.\n",
                                 __FUNCTION__);
            } else {
                i2c_if.offset = PWR_SEQ_RTC_MS;
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
    uint16_t     rt, max, min, over, under;
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
#if 0 
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

#endif
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
	    
        /* Read over voltage */
	    i2c_if.offset = vtg_p->over_v;
	    i2c_if.buf = (char *)&over;
	    rc = pwr_read(&i2c_if, &rd_buf[0]);
	    if (rc != PASSED) {
		sprintf(err_buf, "%s Over read failed. %s",
				  vtg_p->volt_p, rd_buf);
		rc = FAILED;
		break;
	    }

	    /* Read under voltage */
	    i2c_if.offset = vtg_p->under_v;
	    i2c_if.buf = (char *)&under;
	    rc = pwr_read(&i2c_if, &rd_buf[0]);
	    if (rc != PASSED) {
		sprintf(err_buf, "%s Under read failed. %s",
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
	    if (max > over) {
		sprintf(err_buf, "%s Max = %#x is over spec (%#x). \n"
				 "Min = %#x. Max = %#x. Real Time = %#x.",
				 vtg_p->volt_p, max, over,
				 min, max, rt);
		rc = FAILED;
		break;
	    }

	    if ((min < under) && (vtg_p->rt_o != PWR_SEQ_12_0_LR)) {
		sprintf(err_buf, "%s: %s Min = %#x is under spec (%#x). \n"
				 "Min = %#x. Max = %#x. Real Time = %#x. \n",
				 __FUNCTION__, vtg_p->volt_p, min,
                                 under, min, max, rt);
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
    i2c_if.offset = PWR_SEQ_STA_S;
    i2c_if.buf = (char *)&status;
    rc = pwr_read(&i2c_if, err_buf);	/* Read the status register */

    if (rc == PASSED) {
	/* Got the status register. Check for the voltage fault status bits */
	if (status & PS_STAT_VTG_FAULT_PU_MSK) {
	    /* Voltage Fault during Power up */
	    sprintf(err_buf, "%s: Voltage Fault During "
                             "Power Up(status = %#x).\n",
                             __FUNCTION__, status);
	    rc = PWR_SEQ_STAT_CHECK_FAIL;
	} else {
	    /* Check for 12V fault during operation */
	    if (status & PS_STAT_VTG_FAULT_OPT_MSK) {
		/* Voltage Fault during Operation */
		/* Read the Clearable Voltage Fault register */
		//i2c_if.offset = PWR_SEQ_V_F_C; /* 02 */
		i2c_if.offset = PWR_SEQ_F1_C;
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
		i2c_if.offset = PWR_SEQ_F0_C;
		wr_data = DSWAP2(fault);
		msleep(REN_I2C_PROC_TIME);

                rc = pwr_write(&i2c_if);
                 
                if (rc == PASSED) {

		    i2c_if.offset = PWR_SEQ_F1_C;
                    wr_data = DSWAP2(fault);
                    msleep(REN_I2C_PROC_TIME);

                    rc = pwr_write(&i2c_if);
                } else { 
		    sprintf(err_buf, "%s: Unable to clear Pwr Seq Fault register"
				  "(rc = %#x).\n", __FUNCTION__, rc);
		    rc = FAILED;
                }
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

    printf("warning: POE PSU disable is not supported yet, wait for HFS update\n");

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
 * Function   : dump_pwr_seq_reg
 * Description:	Function to display power seq register's value
 * Inputs     : -
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
int 
dump_pwr_seq_reg(void)
{
    n2g_i2c_if_t i2c_if;
    uint32_t rc = FAILED;
    int offset;
    char rd_buf[OVLD_BUF_SIZE];
    uint16_t val[PWR_SEQ_TMP_UNDER - PWR_SEQ_12_0_LR + 1] = {0};

    /* Get Power Sequencer I2C interface structure */
    rc = get_pwr_seq_i2c_struct(&i2c_if);
    if (rc != PASSED) {
        cterr('f', 0, "%s: Failed to get Power Sequencer I2C structure.",
                      __FUNCTION__);
        return (rc);
    }
    for (offset = PWR_SEQ_12_0_LR; offset <= PWR_SEQ_TMP_UNDER; offset++) {
        if (((offset - PWR_SEQ_12_0_LR) % 8) == 0) {
            printf("%08X: ", (offset));
        }
        i2c_if.offset = offset;
	    i2c_if.buf = (char *)&val[offset];
	    rc = pwr_read(&i2c_if, &rd_buf[0]);
	    if (rc != PASSED) {
	        printf("Dump register failed (offset = %X)\n", offset);
            rc = FAILED;
	        break;
        }
        printf("%04X ", val[offset]);
        if (((offset - PWR_SEQ_12_0_LR + 1) % 8) == 0) {
            printf("\n");
        }
    }
    return rc;
}

static int
verify_pwr_seq_fw (void)
{
    uint16_t wr_data, to_addr, end_addr;
    char err[80];
    unsigned char *hex;
    int id, region;
    int rc = FAILED;
    int mismatch = 0;
    n2g_i2c_if_t i2c_if;

    if ((hex = (unsigned char *)malloc(FW_BIN_SZ)) == NULL) {
        printf("system is out of memory.\n");
        return -1;
    }
    if ((readfile(BIN_FW, hex, FW_BIN_SZ) < 0)) {
        printf("unable to read binary firmware file.\n");
    }

    rc = get_pwr_seq_i2c_struct(&i2c_if);
    if (rc != PASSED) {
        printf("%s: Failed to get Power Sequencer I2C structure.",
                     __FUNCTION__);
        goto fun_ret;
    }

    /* set write address to corresponding addr of  the begining block */
    for (region = 0; region < MAX_REGION; region++) {
        to_addr = start[region];
        end_addr = end[region];

        i2c_if.offset = PWR_SEQ_FW_CMD_REG;
        i2c_if.buf = (char *)&to_addr; /* two bytes address */
        rc = pwr_write(&i2c_if);
        if (rc != PASSED) {
            printf("Reseting CMD register failed (offset = %X, data = %x)\n", \
                   i2c_if.offset, wr_data);
            goto fun_ret;
        }

        printf("verifying [0x%04x-0x%04x]...\n",  to_addr, end_addr);
    
        /* firwmare has 64K byte */
        for (mismatch = 0, id = to_addr; id < end_addr; id += 2) {
            i2c_if.offset = PWR_SEQ_FW_DATA_REG;
            i2c_if.buf = (char *)&wr_data;
            rc = pwr_read(&i2c_if, err);
            if (rc != PASSED) {
                printf("FW data register write failed (offset = %X, data = %x)\n", \
                       i2c_if.offset, wr_data);
                goto fun_ret;
            }
            if ((id%256)==0) {
                print_spining_wheel(-1);
            }

            if  ((((wr_data >> 8) & 0xFF) != hex[id])
                 && (((wr_data & 0xFF) != hex[id+1]))) {
                rc = FAILED;
                printf("data mismatch @0x%06x ", id);
                printf("expect [0x%02x%02x] : found [0x%02x%02x]\n", hex[id],
                       hex[id+1], (wr_data >> 8) & 0xFF, (wr_data & 0xFF));
                
                if (mismatch++ > 10) {
                    printf("too many mistmatches...aborting comparison\n");
                    break;
                }
            }
        }
    }
 fun_ret:
    if (hex)
        free(hex);
    return rc;
}

static int
dump_pwr_seq_fw(void)
{
    FILE *fp;
    uint16_t wr_data, to_addr, end_addr;
    char err[80];
    int id;
    int rc = FAILED;
    n2g_i2c_if_t i2c_if;

    rc = get_pwr_seq_i2c_struct(&i2c_if);
    if (rc != PASSED) {
        printf("%s: Failed to get Power Sequencer I2C structure.",
                     __FUNCTION__);
        goto fun_ret;
    }
    
    /* set write address to corresponding addr of  the begining block */
    to_addr = gethex_answer("Enter start address", 0x8000, 0, 0xFC00);
    end_addr = gethex_answer("Enter end address", 0xFC00, 0, 0xFC00);
    
    i2c_if.offset = PWR_SEQ_FW_CMD_REG;
    i2c_if.buf = (char *)&to_addr; /* two bytes address */
    rc = pwr_write(&i2c_if);
    if (rc != PASSED) {
        printf("Reseting CMD register failed (offset = %X, data = %x)\n",\
        i2c_if.offset, wr_data);
        goto fun_ret;
    }

    if ((fp = fopen("pseq_dump.txt", "w")) == NULL) {
        printf("can't open file to store fw");
        goto fun_ret;
    }

    /* firwmare has 64K byte */
    for (id = to_addr; id < end_addr; id += 2) /* two bytes per write */
    {
        i2c_if.offset = PWR_SEQ_FW_DATA_REG;
        i2c_if.buf = (char *)&wr_data;
        rc = pwr_read(&i2c_if, err);
        if (rc != PASSED) {
    	    printf("FW data register write failed (offset = %X, data = %x)\n",\
                    i2c_if.offset, wr_data);
            goto fun_ret;
        }
        if ((id%8)==0 && (id>0)) { 
            fprintf(fp, "\n");
        }
        fprintf(fp, "%02x%02x", (wr_data >> 8) & 0xFF, (wr_data & 0xFF));
        fflush(fp);
    }
    rc = PASSED;
fun_ret:
    fclose(fp);
    return rc;
}

static int
_pwr_seq_eeprom_update ()
{
    int rc = PASSED;
    n2g_i2c_if_t i2c_if;
    uint16_t data;
    
    rc = get_pwr_seq_i2c_struct(&i2c_if);
    
    if (rc != PASSED) {
        printf("%s: Failed to get Power Sequencer I2C structure.",
                     __FUNCTION__);
        goto fun_ret;
    }
    /* set cmd to UPDATE mode */
    i2c_if.offset = PWR_SEQ_FW_CMD_REG;
    data = PWR_SEQ_CMD_UPDATE;
    i2c_if.buf = (char *)&data;
    rc = pwr_write(&i2c_if);
    if (rc != PASSED) {
        printf("Set cmd to update mode failed (offset = %X, data = %x)\n", \
               i2c_if.offset, data);
        goto fun_ret;
    }
    
fun_ret:

    return rc;
}


static int
_pwr_seq_eeprom_rd (uint16_t to_addr, uint16_t *rd_data)
{
    char err[80];
    int rc = FAILED;
    n2g_i2c_if_t i2c_if;
    
    rc = get_pwr_seq_i2c_struct(&i2c_if);
    if (rc != PASSED) {
        printf("%s: Failed to get Power Sequencer I2C structure.",
                     __FUNCTION__);
        goto fun_ret;
    }

    /* set write address to corresponding addr of  the begining block */
    i2c_if.offset = PWR_SEQ_FW_CMD_REG;
    i2c_if.buf = (char *)&to_addr; /* two bytes address */
    *rd_data = 0;
    rc = pwr_write(&i2c_if);
    if (rc != PASSED) {
        printf("Reseting CMD register failed (offset = %X\n",\
               i2c_if.offset);
        goto fun_ret;
    }
    
    i2c_if.offset = PWR_SEQ_FW_DATA_REG;
    i2c_if.buf = (char *)rd_data;

    rc = pwr_read(&i2c_if, err);
    if (rc != PASSED) {
        printf("FW data register write failed (offset = %X", \
               i2c_if.offset);
        goto fun_ret;
    }
    rc = PASSED;

fun_ret:

    return rc;
}

static int
pwr_seq_eeprom_rd (void)
{
    uint16_t to_addr, val;
    int rc = FAILED;

    to_addr = gethex_answer("Enter address to read from", 0x8000, 0, 0xFC00);
    rc = _pwr_seq_eeprom_rd(to_addr, &val);
    printf("value: %x\n", val);
    return(rc);
}

static int
pwr_seq_eeprom_wr (void)
{
    uint16_t wr_data, to_addr, tmp;
    int rc = FAILED;
    n2g_i2c_if_t i2c_if;

    rc = get_pwr_seq_i2c_struct(&i2c_if);
    if (rc != PASSED) {
        printf("%s: Failed to get Power Sequencer I2C structure.",
                     __FUNCTION__);
        goto fun_ret;
    }
    tmp = gethex_answer("Enter value to write", 0, 0, 0xFFFF);
    to_addr = gethex_answer("Enter address to write", 0x8000, 0, 0xFC00);

    /* put device into update mode */
    i2c_if.offset = PWR_SEQ_FW_CMD_REG;
    wr_data = PWR_SEQ_CMD_UPDATE;
    i2c_if.buf = (char *)&wr_data;
    rc = pwr_write(&i2c_if);
    if (rc != PASSED) {
        printf("Set cmd to update mode failed (offset = %X, data = %x)\n", \
               i2c_if.offset, wr_data);
        goto fun_ret;
    }

    /* set write address */
    i2c_if.offset = PWR_SEQ_FW_CMD_REG;
    i2c_if.buf = (char *)&to_addr; /* two bytes address */
    rc = pwr_write(&i2c_if);
    if (rc != PASSED) {
        printf("Reseting CMD register failed (offset = %X, data = %x)\n",\
        i2c_if.offset, wr_data);
        goto fun_ret;
    }
    
    /* begin write data */
    i2c_if.offset = PWR_SEQ_FW_DATA_REG;
    wr_data = tmp;
    i2c_if.buf = (char *)&wr_data;

    rc = pwr_write(&i2c_if);
    if (rc != PASSED) {
        printf("FW data register write failed (offset = %X, data = %x)\n", \
               i2c_if.offset, wr_data);
        goto fun_ret;
    }
    rc = PASSED;

fun_ret:
    return rc;
}

/*******************************************************************************
 *
 * Function   : _write_pwr_seq_fw
 * Description: Write a section of power sequencer firmware through I2C
 * Inputs     : fw_buf: pointer to  a buffer to hold the binary data
 *              to_address: the start address (target addr) of the binary data
 *              byte_len: length of the binary data
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
static int
_write_pwr_seq_fw(unsigned char *fw_buf, uint16_t to_address,
                  uint16_t end_address)
{
    uint16_t wr_data;
    int id, idx;
    int rc = FAILED;
    n2g_i2c_if_t i2c_if;

    rc = get_pwr_seq_i2c_struct(&i2c_if);
    if (rc != PASSED) {
        printf("%s: Failed to get Power Sequencer I2C structure.",
                     __FUNCTION__);
        goto fun_ret;
    }
#ifdef TO_EEPROM
    /* set write address to corresponding addr of  the begining block */
    i2c_if.offset = PWR_SEQ_FW_CMD_REG;
    i2c_if.buf = (char *)&to_address; /* two bytes address */
    rc = pwr_write(&i2c_if);
    if (rc != PASSED) {
        printf("Reseting CMD register failed (offset = %X, data = %x)\n",\
        i2c_if.offset, wr_data);
        goto fun_ret;
    }
#endif
    fflush(NULL);

    for (idx = 0, id = to_address; id < end_address; id += 2,
             idx+=2 /*2 bytes per wr */) 
    {
        wr_data = ((fw_buf[idx] & 0xFF) << 8) +
            (fw_buf[idx+1] & 0xFF);
        if ((idx%ONE_K)==0) {
            print_spining_wheel(-1);
        }
        
        /* begins write data */
        i2c_if.offset = PWR_SEQ_FW_DATA_REG;
        i2c_if.buf = (char *)&wr_data;

#ifdef TO_EEPROM
        rc = pwr_write(&i2c_if);
        if (rc != PASSED) {
    	    printf("FW data register write failed (offset = %X, data = %x)\n",\
                    i2c_if.offset, wr_data);
            goto fun_ret;
        }
#endif
    }
    rc = PASSED;

fun_ret:
    return rc;
}

/******************************************************************************
 *
 * Function   : write_pwr_seq_fw
 * Description:	Function to update power sequencer's firmware
 * srec2bin will be called which takes srec file and produces bin file called
 * pseq.bin
 * to debug: we can convert pseq.bin to hex file using command:
 * 'xxd -ps -c 8 pseq.bin > pseq_fw.txt'
 * then use menu item to read out content of eeprom which will be stored in a
 * file called 'pseq_dump.txt'.
 * then compare the 2 files pseq.txt and pseq_dump.txt
 * 
 * Inputs     : NONE
 * Outputs    : PASSED/FAILED
 *
 ******************************************************************************
 */
int 
write_pwr_seq_fw(int dummy)
{
    n2g_i2c_if_t i2c_if;
    time_t start_t, stop_t;
    uint32_t rc = FAILED;
    uint16_t data;
    int byte_len, idx, ret_val;
    char fw_path[256] = {0};
    char *tk = NULL;
    char *argv[10];
    char cmd[80];
    unsigned char *fw_buf = NULL; //[SREC_DATA_SZ] = {0}; /* 32 bytes data per line (block)*/
    FILE *fp_bin = NULL;
    uint16_t to_address, blk_size, size_opt;
    int *addr_list, *byte_cnt_list;
    uint16_t line = 0;

    byte_cnt_list = addr_list = NULL;
    
    rc = get_pwr_seq_i2c_struct(&i2c_if);
    if (rc != PASSED) {
        printf("%s: Failed to get Power Sequencer I2C structure.",
                     __FUNCTION__);
        return (rc);
    }


    /* query fw path and filename */
    printf("Please enter srec file [%s] (Enter q to quit): ", \
           PWR_SEQ_FW_DEFAULT_PATH);
    fflush(stdout);
    get_line(fw_path, sizeof(fw_path));
    if (strcmp(fw_path, "q") == 0)
    {   /* quit */
        return PASSED;
    }
    if (strlen(fw_path) <= 0)
    {
        /* use default path */
        memcpy(fw_path, PWR_SEQ_FW_DEFAULT_PATH, \
                        strlen(PWR_SEQ_FW_DEFAULT_PATH));
    }
    if (cnt_record(fw_path, &line)==FAILED)
        return(FAILED);
    addr_list = (int *)malloc(sizeof(int *) * (line+1));
    byte_cnt_list = (int *)malloc(sizeof(int *) * (line+1));

    //usage: srec2bin output_file -k file_size -v 0 -d 0 -s pseq.s19
    //-v : verbose
    //-d : padd with 0
    //-s : input file..has to be the last option
    size_opt = FW_BIN_SZ / ONE_K;
    sprintf(cmd, "./srec2bin %s -K %d -v 0 -d 0 -s %s", BIN_FW, size_opt, fw_path);
    tk = strtok(cmd, " ");
    idx = 0;
    argv[idx++] = tk;
    while (tk != NULL) {
        tk = strtok(NULL, " ");
        argv[idx++] = tk;
        if (tk)
            printf("%s ", tk);
    }
    idx--;
    printf("\nsrec file is used to create %s [%d bytes].\n", BIN_FW,
           size_opt * ONE_K);
    srec2bin_main(idx, argv, addr_list, byte_cnt_list);
    if ((fp_bin = fopen(BIN_FW, "rb")) == NULL) {
        printf("unable to open binary file for programming.\n");
        return(FAILED);
    }

    /* check size of binary pseq_fw.bin (should be 64K) */
    fseek(fp_bin, 0, SEEK_END);
    byte_len = ftell(fp_bin);
    if (byte_len != FW_BIN_SZ) {
        printf("%s has incorrect size of %d bytes. expect %d bytes.\n", BIN_FW,
               byte_len, FW_BIN_SZ);
        goto fun_ret;
    }
    
    fw_buf = (unsigned char *)malloc(byte_len);
    memset(fw_buf, 0, sizeof(byte_len));

    /* set cmd to UPDATE mode */
    i2c_if.offset = PWR_SEQ_FW_CMD_REG;
    data = PWR_SEQ_CMD_UPDATE;
    i2c_if.buf = (char *)&data;
    rc = pwr_write(&i2c_if);
    if (rc != PASSED) {
	    printf("Set cmd to update mode failed (offset = %X, data = %x)\n",\
            i2c_if.offset, data);
        goto fun_ret;
    }

    printf("Using %s file to update.\n", BIN_FW);
    printf("Regions to be updated: [0x%4x-%4x] and [0x%4x-%4x]\n\n",
           start[0], end[0]-1, start[1], end[1]-1);
    time(&start_t);
    _pwr_seq_eeprom_update();
    
    for (idx=0;idx<line;idx++) {
        if ( ((addr_list[idx] >= start[0]) && (addr_list[idx] < end[0])) ||
             ((addr_list[idx] >= start[1]) && (addr_list[idx] < end[1])) ) {

            to_address = addr_list[idx];
            blk_size = byte_cnt_list[idx];

            /* seek and read data from file and store to fw_buf */
            fseek(fp_bin, to_address, SEEK_SET);
            byte_len = fread(fw_buf, 1, blk_size,  fp_bin);
            if (byte_len != blk_size) {
                printf("at record %d @0x%4x, found %d bytes; expect %d bytes\n",
                       idx, blk_size, byte_len, to_address);
                goto fun_ret;
            }
            /* handle special case when a record has odd number of data */
            if (blk_size%2) {
                /*read 2 bytes from eeprom; store only upper nibble to the
                 end of buffer to make the size of data even. */
                _pwr_seq_eeprom_rd(to_address+blk_size, &data);
                fw_buf[blk_size] = (data >> 8) & 0xFF;
                /*round up to even number */
                blk_size++;
            }
            _write_pwr_seq_fw(fw_buf, to_address, to_address+blk_size);
        } 
    }

    time(&stop_t);
    printf("Update done. Took %.0f secs.\n", difftime(stop_t, start_t));
    ret_val = verify_pwr_seq_fw();
    time(&stop_t);
    printf("total time: %.0f secs.\n", difftime(stop_t, start_t));
    
    if (ret_val < 0) {
        printf("\n\n****WARNING****\n\n");
        printf("\nFirmware verification failed. Please re-program firmware.\n");
        printf("Do not reboot system until firmware upgrade is successful.\n");
    } else
        printf("To load the updated firmware, please reboot system.\n");

    
    if (getc_answer("Reboot system? (y/n)", "yn", 'n') == 'y') {
        /* reboot */
        i2c_if.offset = PWR_SEQ_FW_CMD_REG;
        data = PWR_SEQ_CMD_REBOOT;
        i2c_if.buf = (char *)&data;
        rc = pwr_write(&i2c_if);
        if (rc != PASSED) {
	        printf("reboot power sequencer failed (offset = %X, data = %x)\n",\
                i2c_if.offset, data);
            goto fun_ret;
        }
        sleep(5); /* power sequencer is rebooting */
        exit(0);
    }
    rc = PASSED;
    
fun_ret:
    if (fp_bin!=NULL)
        fclose(fp_bin);
    if (fw_buf!=NULL)
        free(fw_buf);
    if (addr_list)
        free(addr_list);
    if (byte_cnt_list)
        free(byte_cnt_list);

    return rc;
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
Revision 1.26  2019/09/11 07:18:15  alpeng
CSCvr18160 - adjust NIOS mode setup on Utah

Revision 1.25  2014/08/21 07:17:17  erwu2
support pwr seq utility to dump black box cmd related reg just after writing

Revision 1.24  2014/08/15 03:47:29  alpeng
support negative val on margin register

Revision 1.23  2014/07/09 18:19:38  mcharon
add firmware verification after firmware upgrade

Revision 1.22  2014/06/03 19:08:14  mcharon
support odd size pwer seq s record

Revision 1.21  2014/05/29 19:22:46  mcharon
add pwr seq fw filename path erro checking

Revision 1.20  2014/05/29 00:41:28  mcharon
support programming of pwr seq fw using srec file

Revision 1.19  2014/05/23 03:00:48  erwu2
change the 1.0V margin from +/-5% to +/-3% per HW

Revision 1.18  2014/05/19 23:31:34  mcharon
cvs update

Revision 1.17  2014/05/10 07:11:30  erwu2
fine tune 1.2v margin from +/-5% to +/-3% per HW

Revision 1.16  2014/05/07 02:33:00  erwu2
fix delay time to 1s for adjusting margin voltage

Revision 1.15  2014/03/04 03:07:23  hroni
power sequencer update utility supports srec file

Revision 1.14  2014/02/27 07:56:30  hroni
add update power sequencer utility

Revision 1.13  2014/02/19 04:03:15  hroni
in vtg_mrgn(), replace printf() with cterr_db_print() to support enchanced error msg

Revision 1.12  2014/02/11 09:33:11  hroni
after margining, wait until the target margin range is achieved

Revision 1.11  2014/02/10 09:20:47  hroni
set default volt margining delay to 1 sec

Revision 1.10  2014/02/10 09:09:27  hroni
add 1 sec delay for ddr margining

Revision 1.9  2014/01/29 08:10:14  alpeng
update pwr seq i2c scan test

Revision 1.8  2014/01/25 03:28:17  hroni
1. remove spare registers check during voltage test. 2. remove the byte swap for read comparison

Revision 1.7  2014/01/23 08:03:08  hroni
enable ovld_poe_psu_pwr_control() to enable/disable poe psu

Revision 1.6  2013/11/26 08:40:37  hroni
fix compiler warning

Revision 1.5  2013/10/03 01:39:34  hroni
add transitional delay 2secs for margining 1.0V and 1.2V.

Revision 1.4  2013/09/09 06:35:02  hroni
1. use 5% margining on 1.2v and 1.0v, will recover to 9% after HW confirm it is safe
2. add show latest read voltage after and before doing margining
3. turn off byte swap in pwr_write()

Revision 1.3  2013/08/07 22:52:57  hroni
1. reset NIOS during diag init. 2. fix power sequencer utility

Revision 1.2  2013/07/18 17:17:04  mcharon
add -Wal and clean up compile warnings

Revision 1.1  2013/06/14 10:25:48  alpeng
support voltage margin

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
