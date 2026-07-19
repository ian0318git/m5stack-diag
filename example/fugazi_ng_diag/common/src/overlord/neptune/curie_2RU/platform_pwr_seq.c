/* $Id: platform_pwr_seq.c,v 1.1 2020/01/09 01:02:04 jiajliu Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/overlord/neptune/curie_2RU/platform_pwr_seq.c,v $
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
 *    Notes: update register table from ISR-NEPTUNE 
 *    Power Sequencer Hardware Functional Specification
 *    (EDCS-1573047) 2016.09.02
 *
 * Copyright (c) 2018 by Cisco Systems, Inc.
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
#include "platform_i2c.h"

#define MAX_REGION 2
#define BIN_FW "pseq.bin"
#define SREC_DATA_SZ 32
#define FW_BIN_SZ 0x10000
#define WD_BYPASS /* */
#define PWR_SEQ_FW_DEFAULT_PATH "pseq.s19"

#define TO_EEPROM

/* Variable used to store the currently selected packet checksum type */
CyBtldr_ChecksumType CyBtldr_Checksum = SUM_CHECKSUM;
unsigned long g_validRows[MAX_FLASH_ARRAYS];
CyBtldr_CommunicationsData comm1;
int CyBtldr_ParseEnterBootLoaderCmdResult(unsigned char*, unsigned long, unsigned long*, unsigned char*, unsigned long*, unsigned char*);
int CyBtldr_TransferData(unsigned char*, int, unsigned char*, int);
int get_app_status(int, unsigned char*, unsigned char*);
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
int get_pwr_seq_fw_rev(int);
static int mcu_pin_shorten_check(void);
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
static int dump_pwr_seq_reg(void);
static int write_pwr_seq_fw(int);
static uint16_t pwr_seq_get_val(int16_t, uint);
static uint16_t start[MAX_REGION]   = {0x8000, 0x1000};
static uint16_t end[MAX_REGION]     = {0xFC00, 0x1400};
int ovld_show_mb_watts(void);


/* Neptune - scratchpad register table */
static reg_info_t pwr_seq_scratchpad_table[]=
{
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
    {"End", 0xFFFFFFFF, READ_ONLY, {0}, 0, 0},
};

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
    {"FW update Starting",                   PWR_SEQ_FW_UP_ST, 
	WRITE_ONLY | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT},
	0xFFFF, 0x0000},
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
    {"Black Box Data 3: Fault Code",      PWR_SEQ_BB_D3,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT},
	0x0000, 0x0000},
    {"SW Reset",                      PWR_SEQ_SW_RST, 
	READ_WRITE | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT},
	0x0000, 0x0000},
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
    {"Performance Monitor",             PWR_SEQ_PM,    
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT},
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
    {"VP12P0 Voltage: Ramp Time Maximum and Reading",  PWR_SEQ_12_0_RPT,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT},
	0x0000, 0x0000},
    {"VP12P0 Voltage Current from PSU1: Latest Reading",    PWR_SEQ_12_0_C_PSU1_LR,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT},
	0x0000, 0x0000},
    {"VP12P0 Voltage Current from PSU1 : Maximum Reading",    PWR_SEQ_12_0_C_PSU1_MAX,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT},
	0x0000, 0x0000},
    {"VP12P0 Voltage Current from PSU1: Minimum Reading",    PWR_SEQ_12_0_C_PSU1_MIN, 
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT},
	0x0000, 0x0000},
    {"VP12P0 Voltage Current from PSU1: Ramp Time Maximum and Reading",  PWR_SEQ_12_0_C_PSU1_RPT,
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
    {"VP3P3 Voltage CPU: Latest Reading",    PWR_SEQ_3_3_CP_CPU_LR,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT},
	0x0000, 0x0000},
    {"VP3P3 Voltage CPU: Maximum Reading",    PWR_SEQ_3_3_CP_CPU_MAX,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT},
	0x0000, 0x0000},
    {"VP3P3 Voltage CPU: Minimum Reading",    PWR_SEQ_3_3_CP_CPU_MIN, 
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT},
	0x0000, 0x0000},
    {"VP3P3 Voltage CPU: Ramp Time Maximum and  Reading",  PWR_SEQ_3_3_CP_CPU_RPT,
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
    {"VP3P0 Voltage : Latest Reading",    PWR_SEQ_3_0_LR,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT},
	0x0000, 0x0000},
    {"VP3P0 Voltage: Maximum Reading",    PWR_SEQ_3_0_MAX,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT},
	0x0000, 0x0000},
    {"VP3P0 Voltage: Minimum Reading",    PWR_SEQ_3_0_MIN, 
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT},
	0x0000, 0x0000},
    {"VP3P0 Voltage: Ramp Time Maximum and Reading",  PWR_SEQ_3_0_RPT,
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
    {"VP2P5 Voltage: Ramp Time Maximum and Reading",  PWR_SEQ_2_5_RPT,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT},
	0x0000, 0x0000},
    {"VP1P82 Voltage : Latest Reading",    PWR_SEQ_1_82_LR,    
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT},
	0x0000, 0x0000},
    {"VP1P82 Voltage: Maximum Reading",    PWR_SEQ_1_82_MAX,    
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT},
	0x0000, 0x0000},
    {"VP1P82 Voltage: Minimum Reading",    PWR_SEQ_1_82_MIN,     
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT},
	0x0000, 0x0000},
    {"VP1P82 Voltage: Ramp Time Maximum and Reading",  PWR_SEQ_1_82_RPT,    
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
    {"VP1P7 Voltage : Latest Reading",    PWR_SEQ_1_7_LR,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT},
	0x0000, 0x0000},
    {"VP1P7 Voltage: Maximum Reading",    PWR_SEQ_1_7_MAX,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT},
	0x0000, 0x0000},
    {"VP1P7 Voltage: Minimum Reading",    PWR_SEQ_1_7_MIN, 
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT},
	0x0000, 0x0000},
    {"VP1P7 Voltage: Ramp Time Maximum and  Reading",  PWR_SEQ_1_7_RPT,
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
    {"VP1P3 Voltage : Latest Reading",    PWR_SEQ_1_3_LR,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT},
	0x0000, 0x0000},
    {"VP1P3 Voltage: Maximum Reading",    PWR_SEQ_1_3_MAX,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT},
	0x0000, 0x0000},
    {"VP1P3 Voltage: Minimum Reading",    PWR_SEQ_1_3_MIN, 
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT},
	0x0000, 0x0000},
    {"VP1P3 Voltage: Ramp Time Maximum and  Reading",  PWR_SEQ_1_3_RPT,
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
    {"VP1P2 Voltage CPU : Latest Reading",  PWR_SEQ_1_2_CPU_LR,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT},
	0x0000, 0x0000},
    {"VP1P2 Voltage CPU : Maximum Reading",    PWR_SEQ_1_2_CPU_MAX,    
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT},
	0x0000, 0x0000},
    {"VP1P2 Voltage CPU : Minimum Reading",    PWR_SEQ_1_2_CPU_MIN,     
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT},
	0x0000, 0x0000},
    {"VP1P2 Voltage CPU : Ramp Time Maximum and Reading",  PWR_SEQ_1_2_CPU_RPT,    
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT},
	0x0000, 0x0000},
    {"VP1P05 SUS Voltage : Latest Reading",    PWR_SEQ_1_05_SUS_LR,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT},
	0x0000, 0x0000},
    {"VP1P05 SUS Voltage: Maximum Reading",    PWR_SEQ_1_05_SUS_MAX,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT},
	0x0000, 0x0000},
    {"VP1P05 SUS Voltage: Minimum Reading",    PWR_SEQ_1_05_SUS_MIN, 
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT},
	0x0000, 0x0000},
    {"VP1P05 SUS Voltage: Ramp Time Maximum and  Reading",  PWR_SEQ_1_05_SUS_RPT,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT},
	0x0000, 0x0000},
    {"VP1P05 Voltage : Latest Reading",    PWR_SEQ_1_05_LR,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT},
	0x0000, 0x0000},
    {"VP1P05 Voltage: Maximum Reading",    PWR_SEQ_1_05_MAX,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT},
	0x0000, 0x0000},
    {"VP1P05 Voltage: Minimum Reading",    PWR_SEQ_1_05_MIN, 
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT},
	0x0000, 0x0000},
    {"VP1P05 Voltage: Ramp Time Maximum and  Reading",  PWR_SEQ_1_05_RPT,
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
    {"VP0P6 CP Voltage : Latest Reading",    PWR_SEQ_0_6_CP_LR,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT},
	0x0000, 0x0000},
    {"VP0P6 CP Voltage: Maximum Reading",    PWR_SEQ_0_6_CP_MAX,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT},
	0x0000, 0x0000},
    {"VP0P6 CP Voltage: Minimum Reading",    PWR_SEQ_0_6_CP_MIN, 
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT},
	0x0000, 0x0000},
    {"VP0P6 CP Voltage: Ramp Time Maximum and  Reading",  PWR_SEQ_0_6_CP_RPT,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT},
	0x0000, 0x0000},
    {"VP12P0 Voltage Current from PSU2: Latest Reading",    PWR_SEQ_12_0_C_PSU2_LR,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT},
	0x0000, 0x0000},
    {"VP12P0 Voltage Current from PSU2 : Maximum Reading",    PWR_SEQ_12_0_C_PSU2_MAX,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT},
	0x0000, 0x0000},
    {"VP12P0 Voltage Current from PSU2: Minimum Reading",    PWR_SEQ_12_0_C_PSU2_MIN, 
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT},
	0x0000, 0x0000},
    {"VP12P0 Voltage Current from PSU2: Ramp Time Maximum and Reading",  PWR_SEQ_12_0_C_PSU2_RPT,
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
    {"End", 0xFFFFFFFF, READ_ONLY, {0}, 0, 0},


};

/* change to curie version */
static pwr_seq_v_t voltage_test_table[] =
{
    {" VP12P0 Voltage  ",  PWR_SEQ_12_0_MAX_OP_THRE,  PWR_SEQ_12_0_MIN_OP_THRE,
    PWR_SEQ_12_0_LR,  PWR_SEQ_12_0_MAX,  PWR_SEQ_12_0_MIN},

    {" VP12P0 Current PSU1",  PWR_SEQ_12_0_C_PSU1_MAX_OP_THRE, PWR_SEQ_12_0_C_PSU1_MIN_OP_THRE,
    PWR_SEQ_12_0_C_PSU1_LR,  PWR_SEQ_12_0_C_PSU1_MAX,  PWR_SEQ_12_0_C_PSU1_MIN}, 

    {" VP5P0 Voltage ",  PWR_SEQ_5_0_MAX_OP_THRE, PWR_SEQ_5_0_MIN_OP_THRE,
    PWR_SEQ_5_0_LR,  PWR_SEQ_5_0_MAX,  PWR_SEQ_5_0_MIN}, 

    {" VP3P3_CPU Voltage ",  PWR_SEQ_3_3_CPU_MAX_OP_THRE, PWR_SEQ_3_3_CPU_MIN_OP_THRE,
    PWR_SEQ_3_3_CP_CPU_LR,  PWR_SEQ_3_3_CP_CPU_MAX,  PWR_SEQ_3_3_CP_CPU_MIN},

    {" VP3P3 Voltage ",  PWR_SEQ_3_3_MAX_OP_THRE, PWR_SEQ_3_3_MIN_OP_THRE,
    PWR_SEQ_3_3_LR,  PWR_SEQ_3_3_MAX,  PWR_SEQ_3_3_MIN}, 

    {" VP3P0 Voltage ",  PWR_SEQ_3_0_MAX_OP_THRE, PWR_SEQ_3_0_MIN_OP_THRE,
    PWR_SEQ_3_0_LR,  PWR_SEQ_3_0_MAX,  PWR_SEQ_3_0_MIN}, 

    {" VP2P5 Voltage ",  PWR_SEQ_2_5_MAX_OP_THRE, PWR_SEQ_2_5_MIN_OP_THRE,
    PWR_SEQ_2_5_LR,  PWR_SEQ_2_5_MAX,  PWR_SEQ_2_5_MIN}, 

    {" VP1P82 Voltage ",  PWR_SEQ_1_82_MAX_OP_THRE, PWR_SEQ_1_82_MIN_OP_THRE,    
    PWR_SEQ_1_82_LR,  PWR_SEQ_1_82_MAX,  PWR_SEQ_1_82_MIN},

    {" VP1P8 Voltage ",  PWR_SEQ_1_8_MAX_OP_THRE, PWR_SEQ_1_8_MIN_OP_THRE,
    PWR_SEQ_1_8_LR,  PWR_SEQ_1_8_MAX,  PWR_SEQ_1_8_MIN}, 

    {" VP1P7 Voltage ",  PWR_SEQ_1_7_MAX_OP_THRE, PWR_SEQ_1_7_MIN_OP_THRE,
    PWR_SEQ_1_7_LR,  PWR_SEQ_1_7_MAX,  PWR_SEQ_1_7_MIN}, 

    {" VP1P5 Voltage ",  PWR_SEQ_1_5_MAX_OP_THRE, PWR_SEQ_1_5_MIN_OP_THRE,
    PWR_SEQ_1_5_LR,  PWR_SEQ_1_5_MAX,  PWR_SEQ_1_5_MIN}, 

    {" VP1P3 Voltage ",  PWR_SEQ_1_3_MAX_OP_THRE, PWR_SEQ_1_3_MIN_OP_THRE,
    PWR_SEQ_1_3_LR,  PWR_SEQ_1_3_MAX,  PWR_SEQ_1_3_MIN}, 

    {" VP1P2 Voltage ",  PWR_SEQ_1_2_MAX_OP_THRE, PWR_SEQ_1_2_MIN_OP_THRE,
    PWR_SEQ_1_2_LR,  PWR_SEQ_1_2_MAX,  PWR_SEQ_1_2_MIN}, 

    {" VP1P2 CPU Voltage",  PWR_SEQ_1_2_CPU_MAX_OP_THRE, PWR_SEQ_1_2_CPU_MIN_OP_THRE,        
    PWR_SEQ_1_2_CPU_LR, PWR_SEQ_1_2_CPU_MAX, PWR_SEQ_1_2_CPU_MIN},

    {" VP1P05 SUS Voltage ",  PWR_SEQ_1_05_SUS_MAX_OP_THRE, PWR_SEQ_1_05_SUS_MIN_OP_THRE,    
    PWR_SEQ_1_05_SUS_LR,  PWR_SEQ_1_05_SUS_MAX,  PWR_SEQ_1_05_SUS_MIN},

    {" VP1P05 Voltage ",  PWR_SEQ_1_05_MAX_OP_THRE, PWR_SEQ_1_05_MIN_OP_THRE,
    PWR_SEQ_1_05_LR,  PWR_SEQ_1_05_MAX,  PWR_SEQ_1_05_MIN},

    {" VP1P0 Voltage ",  PWR_SEQ_1_0_MAX_OP_THRE, PWR_SEQ_1_0_MIN_OP_THRE,
    PWR_SEQ_1_0_LR,  PWR_SEQ_1_0_MAX,  PWR_SEQ_1_0_MIN}, 

    {" VP0P6 Voltage ",  PWR_SEQ_0_6_CPU_MAX_OP_THRE, PWR_SEQ_0_6_CPU_MIN_OP_THRE,
    PWR_SEQ_0_6_CP_LR,  PWR_SEQ_0_6_CP_MAX,  PWR_SEQ_0_6_CP_MIN},

    {" VP12P0 Current PSU2",  PWR_SEQ_12_0_C_PSU2_MAX_OP_THRE, PWR_SEQ_12_0_C_PSU2_MIN_OP_THRE,
    PWR_SEQ_12_0_C_PSU2_LR,  PWR_SEQ_12_0_C_PSU2_MAX,  PWR_SEQ_12_0_C_PSU2_MIN}, 

    {" NIM Current ",  PWR_SEQ_NIM_CR_MAX_OP_THRE, PWR_SEQ_NIM_CR_MIN_OP_THRE,
    PWR_SEQ_NIM_CR_LR,  PWR_SEQ_NIM_CR_MAX,  PWR_SEQ_NIM_CR_MIN},

    {" FAN Current ",  PWR_SEQ_FAN_CR_MAX_OP_THRE, PWR_SEQ_FAN_CR_MIN_OP_THRE,
    PWR_SEQ_FAN_CR_LR,  PWR_SEQ_FAN_CR_MAX,  PWR_SEQ_FAN_CR_MIN},

    {" VP3P3 Standby Voltage ",  PWR_SEQ_3_3_STV_MAX_OP_THRE, PWR_SEQ_3_3_STV_MIN_OP_THRE,
    PWR_SEQ_3_3_STV_LR,  PWR_SEQ_3_3_STV_MAX,  PWR_SEQ_3_3_STV_MIN}, 

    {" DEBUG ",  PWR_SEQ_DEBUG_MAX_OP_THRE, PWR_SEQ_DEBUG_MIN_OP_THRE,
    PWR_SEQ_DEBUG_LR,  PWR_SEQ_DEBUG_MAX,  PWR_SEQ_DEBUG_MIN},

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
    {"Dump Power Sequencer registers", (PFT)dump_pwr_seq_reg, 0,
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
    {"Show MB watts ",            (PFT)ovld_show_mb_watts,     0,
        0,     (type_t(*)())0, 0, (PFT)0, 0},
    {"MCU P2_PSQ Pin Shorten Check", (PFT)mcu_pin_shorten_check,   0,
        0,                              (type_t(*)())0, 0, (PFT)0, 0},
};

/*******************************************************************************
 *
 * Function   : mcu_pin_shorten_check 
 * Description:	If version is bigger than V1.03 and offset 0x80 value
 *              is lower than 0xa00, print message
 *              "Shorten P2_PSQ pin 1 and pin 2 before MCU FW upgrade".
 * Inputs     : NONE 
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
static int mcu_pin_shorten_check (void)
{
    uint32_t rc = FAILED;
    uint16_t val = 0;
    char ver[32];
    char v1p04[32] = "1.04";
    uint32_t shorten_value = 0xa00;

    rc = pwr_seq_read_reg(PWR_SEQ_REV, &val);
    if (rc != PASSED) {
        printf("Failed to read Power Sequencer FW revision.\n");
        return (rc);
    }

    sprintf(ver, "%d.%02d", ((val & PS_REV_MAJOR_MSK) >> PS_REV_MAJOR_SHIFT),
           (val & PS_REV_MINOR_MSK));

    rc = pwr_seq_read_reg(PWR_SEQ_DEBUG_LR, &val);
    if (rc != PASSED) {
        printf("Failed to read Power Sequencer Debug pin.\n");
        return (rc);
    }

    if ((atof(ver) >= atof(v1p04)) && (val < shorten_value)) {
        printf("please shorten P2_PSQ pin1 & pin2 before MCU image upgrade\n");
        rc = FAILED;
    } else {
        printf("Power Sequencer FW version is %s, debug pin value is %x\n", ver, val);
    }

    return (rc);
}
 
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
int get_pwr_seq_fw_rev (int option)
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
    /* VP12P0 Current from PSU1: Latest Reading */
    return (pwr_seq_read_reg(PWR_SEQ_12_0_C_PSU1_LR, iadc));
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
    if ((offset > PWR_SEQ_BB_D3) && (offset < PWR_SEQ_WD_EN)) {
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
        if ((offset >= PWR_SEQ_BB_C) && (offset <= PWR_SEQ_BB_D3)) {
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
    uint32_t rc = FAILED, offset;
    uint16_t data = 0;

    offset = gethex_answer("Enter the register number:", 0, 0, 0xff);

    if ((rc = pwr_seq_read_reg(offset,  &data)) != PASSED) {
        cterr('f', 0, "%s: Failed to read Power Sequencer Reg %#x",
              __FUNCTION__, offset);
        return (rc);
    } else {
        printf("Register 0x%x current val = 0x%x\n", offset, data);
    }

    data = gethex_answer("Enter the 16-bit data:", data, 0, 0xffff);
    if ((rc = pwr_seq_write_reg(offset, data)) != PASSED) {
        cterr('f', 0, "%s: Failed to write Power Sequencer Reg %#x.",
              __FUNCTION__, offset);
    } else {
        if ((rc = pwr_seq_read_reg(offset,  &data)) != PASSED) {
            cterr('f', 0, "%s: Failed to read Power Sequencer Reg %#x",
                  __FUNCTION__, offset);
        return (rc);
        } else {
            printf("Register 0x%x new val = 0x%x\n", offset, data);
        }
    }

    return (rc);
}

/****************************************************************************
 *
 * Function   : pwr_seq_get_val   
 * Description:	get value from user for write. 
 * Inputs     : offset - dummy
 *              data - original data 
 * Outputs    : data 
 *
 ****************************************************************************
 */
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

    if (!i2c_if->buf) {
        assert(!"env_write: buf is null");
    }

    rc = n2g_i2c_write(i2c_if);
    if (rc != RC_I2C_OP_OK) {
        return (FAILED);
    }

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

    /* based on Utah HW eng. to test scratched register is enough to verify 
     * I2C register accessing. We don't want to test the other registers which 
     * will be monitored by firmware */
    /* init i2c parameters */
    reg_ptr = &pwr_seq_scratchpad_table[0];
    
    for (ia = 0, reg_ptr = &pwr_seq_scratchpad_table[0];
        ia < (sizeof(pwr_seq_scratchpad_table) / sizeof(reg_info_t));
        ia++, reg_ptr++) {

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
        /* deubg field is used for hw, no need to check */
        if (vtg_p->over_v ==  PWR_SEQ_DEBUG_MAX_OP_THRE) {
            break;
        }

        /* Read Real Time before Max and Min voltages, since Max and Min
         * are updated after the Real Time. By following this order, we
         * can minimize the race condition of these registers. CSCsr68713
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
int dump_pwr_seq_reg(void)
{
    uint32_t rc = FAILED, offset;
    uint16_t data = 0;

    offset = gethex_answer("Enter the register number:", 0, 0, 0xff);

    if ((rc = pwr_seq_read_reg(offset,  &data)) != PASSED) {
        cterr('f', 0, "%s: Failed to read Power Sequencer Reg %#x",
              __FUNCTION__, offset);
    } else {
        printf("Register 0x%x val = 0x%x\n", offset, data);
    }

    return (rc);
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

unsigned short CyBtldr_ComputeChecksum(unsigned char* buf, unsigned long size)
{
    if (CyBtldr_Checksum == CRC_CHECKSUM)
    {
            unsigned short crc = 0xffff;
            unsigned short tmp;
            int i;

            if (size == 0)
                    return (~crc);

            do
            {
                    for (i = 0, tmp = 0x00ff & *buf++; i < 8; i++, tmp >>= 1)
                    {
                            if ((crc & 0x0001) ^ (tmp & 0x0001))
                                    crc = (crc >> 1) ^ 0x8408;
                            else
                                crc >>= 1;
                    }
            }
            while (--size);

            crc = ~crc;
        tmp = crc;
            crc = (crc << 8) | (tmp >> 8 & 0xFF);

            return crc;
    }
    else /* SUM_CHECKSUM */
    {
        unsigned short sum = 0;
            while (size-- > 0)
                    sum += *buf++;

            return (1 + ~sum);
    }
}

void create_end_bootload_cmd (unsigned char *cmdBuf, unsigned long* cmdSize, unsigned long* resSize)
{
    const unsigned char reset = 0x00;
    const unsigned long COMMAND_DATA_SIZE = 1;
    const unsigned int COMMAND_SIZE = BASE_CMD_SIZE + COMMAND_DATA_SIZE;
    unsigned short checksum;

    *resSize = BASE_CMD_SIZE;
    *cmdSize = COMMAND_SIZE;
    cmdBuf[0] = CMD_START;
    cmdBuf[1] = CMD_EXIT_BOOTLOADER;
    cmdBuf[2] = (unsigned char)COMMAND_DATA_SIZE;
    cmdBuf[3] = (unsigned char)(COMMAND_DATA_SIZE >> 8);
    cmdBuf[4] = reset;
    checksum = CyBtldr_ComputeChecksum(cmdBuf, COMMAND_SIZE - 3);
    cmdBuf[5] = (unsigned char)checksum;
    cmdBuf[6] = (unsigned char)(checksum >> 8);
    cmdBuf[7] = CMD_STOP;
}

unsigned char FromHex(char value)
{
    if ('0' <= value && value <= '9')
        return (unsigned char)(value - '0');
    if ('a' <= value && value <= 'f')
        return (unsigned char)(10 + value - 'a');
    if ('A' <= value && value <= 'F')
        return (unsigned char)(10 + value - 'A');
    return 0;
}

int FromAscii(unsigned int bufSize, unsigned char* buffer, unsigned short* rowSize, unsigned char* rowData)
{
    unsigned short i;
    int err = CYRET_SUCCESS;

    if (bufSize & 1) { // Make sure even number of bytes
        err = CYRET_ERR_LENGTH;
    } else {
        for (i = 0; i < bufSize / 2; i++)
        {
            rowData[i] = (FromHex(buffer[i * 2]) << 4) | FromHex(buffer[i * 2 + 1]);
        }
        *rowSize = i;
    }

    return err;
}

int ParseHeader(unsigned int bufSize, unsigned char* buffer, unsigned long* siliconId, unsigned char* siliconRev, unsigned char* chksum)
{
    const unsigned int LENGTH_ID = 5;            //4-silicon id, 1-silicon rev
    const unsigned int LENGTH_CHKSUM = LENGTH_ID + 1; //1-checksum type

    unsigned short rowSize;
    unsigned char rowData[MAX_BUFFER_SIZE];

    int err = FromAscii(bufSize, buffer, &rowSize, rowData);

    if (CYRET_SUCCESS == err) {
        if (rowSize >= LENGTH_CHKSUM) {
            *chksum = rowData[5];
        }

        if (rowSize >= LENGTH_ID) {
            *siliconId = (rowData[0] << 24) | (rowData[1] << 16) | (rowData[2] << 8) | (rowData[3]);
            *siliconRev = rowData[4];
        } else {
            err = CYRET_ERR_LENGTH;
        }
    }

    return err;
}

void SetCheckSumType(CyBtldr_ChecksumType chksumType)
{
    CyBtldr_Checksum = chksumType;
}

int ParseRowData(unsigned int bufSize, unsigned char* buffer, unsigned char* arrayId, unsigned short* rowNum, unsigned char* rowData, unsigned short* size, unsigned char* checksum)
{
    const unsigned short MIN_SIZE = 6; //1-array, 2-addr, 2-size, 1-checksum
    const int DATA_OFFSET = 5;

    unsigned int i;
    unsigned short hexSize;
    unsigned char hexData[MAX_BUFFER_SIZE];
    int err = CYRET_SUCCESS;

    if (bufSize <= MIN_SIZE) {
        err = CYRET_ERR_LENGTH;
    } else if (buffer[0] == ':') {
        err = FromAscii(bufSize - 1, &buffer[1], &hexSize, hexData);

        *arrayId = hexData[0];
        *rowNum = (hexData[1] << 8) | (hexData[2]);
        *size = (hexData[3] << 8) | (hexData[4]);
        *checksum = (hexData[hexSize - 1]);

        if ((*size + MIN_SIZE) == hexSize) {
            for (i = 0; i < *size; i++)
            {
                rowData[i] = (hexData[DATA_OFFSET + i]);
            }
        }
        else
            err = CYRET_ERR_DATA;
    } else {
        err = CYRET_ERR_CMD;
    }

    return err;
}

int CyBtldr_ParseGetFlashSizeCmdResult(unsigned char* cmdBuf, unsigned long cmdSize, unsigned short* startRow, unsigned short* endRow, unsigned char* status)
{
    const unsigned long RESULT_DATA_SIZE = 4;
    const unsigned long RESULT_SIZE = BASE_CMD_SIZE + RESULT_DATA_SIZE;
    int err = CYRET_SUCCESS;

    if (cmdSize != RESULT_SIZE) {
        err = CYRET_ERR_LENGTH;
    } else if (cmdBuf[1] != CYRET_SUCCESS) {
        err = CYRET_ERR_BTLDR_MASK | (*status = cmdBuf[1]);
    } else if (cmdBuf[0] != CMD_START || cmdBuf[2] != RESULT_DATA_SIZE || cmdBuf[3] != (RESULT_DATA_SIZE >> 8) || cmdBuf[RESULT_SIZE - 1] != CMD_STOP) {
        err = CYRET_ERR_DATA;
    } else {
        *startRow = (cmdBuf[5] << 8) | cmdBuf[4];
        *endRow = (cmdBuf[7] << 8) | cmdBuf[6];
        *status = cmdBuf[1];
    }

    return err;
}

int CyBtldr_CreateGetFlashSizeCmd(unsigned char arrayId, unsigned char* cmdBuf, unsigned long* cmdSize, unsigned long* resSize)
{
    const unsigned long RESULT_DATA_SIZE = 4;
    const unsigned long COMMAND_DATA_SIZE = 1;
    const unsigned int COMMAND_SIZE = BASE_CMD_SIZE + COMMAND_DATA_SIZE;
    unsigned short checksum;

    *resSize = BASE_CMD_SIZE + RESULT_DATA_SIZE;
    *cmdSize = COMMAND_SIZE;
    cmdBuf[0] = CMD_START;
    cmdBuf[1] = CMD_GET_FLASH_SIZE;
    cmdBuf[2] = (unsigned char)COMMAND_DATA_SIZE;
    cmdBuf[3] = (unsigned char)(COMMAND_DATA_SIZE >> 8);
    cmdBuf[4] = arrayId;
    checksum = CyBtldr_ComputeChecksum(cmdBuf, COMMAND_SIZE - 3);
    cmdBuf[5] = (unsigned char)checksum;
    cmdBuf[6] = (unsigned char)(checksum >> 8);
    cmdBuf[7] = CMD_STOP;

    return CYRET_SUCCESS;
}

int CyBtldr_ValidateRow(unsigned char arrayId, unsigned short rowNum)
{
    unsigned long inSize;
    unsigned long outSize;
    unsigned short minRow = 0;
    unsigned short maxRow = 0;
    unsigned char inBuf[MAX_COMMAND_SIZE];
    unsigned char outBuf[MAX_COMMAND_SIZE];
    unsigned char status = CYRET_SUCCESS;
    int err = CYRET_SUCCESS;

    if (arrayId < MAX_FLASH_ARRAYS) { //MAX_FLASH_ARRAYS is 4
        if (NO_FLASH_ARRAY_DATA == g_validRows[arrayId]) {
            err = CyBtldr_CreateGetFlashSizeCmd(arrayId, inBuf, &inSize, &outSize);

            if (CYRET_SUCCESS == err) {
                err = CyBtldr_TransferData(inBuf, inSize, outBuf, outSize);
            }

            if (CYRET_SUCCESS == err) {
                err = CyBtldr_ParseGetFlashSizeCmdResult(outBuf, outSize, &minRow, &maxRow, &status);
            }

            if (CYRET_SUCCESS != status) {
                err = status | CYRET_ERR_BTLDR_MASK;
            }

            if (CYRET_SUCCESS == err) {
                if (CYRET_SUCCESS == status) {
                    g_validRows[arrayId] = (minRow << 16) + maxRow;
                } else {
                    err = status | CYRET_ERR_BTLDR_MASK;
                }
            }
        }

        if (CYRET_SUCCESS == err) {
            minRow = (unsigned short)(g_validRows[arrayId] >> 16);
            maxRow = (unsigned short)g_validRows[arrayId];
            if (rowNum < minRow || rowNum > maxRow) {
                err = CYRET_ERR_ROW;
            }
        }
    } else {
        err = CYRET_ERR_ARRAY;
    }

    return err;
}

int CyBtldr_CreateSendDataCmd(unsigned char* buf, unsigned short size, unsigned char* cmdBuf, unsigned long* cmdSize, unsigned long* resSize)
{
    unsigned short checksum;
    unsigned long i;

    *resSize = BASE_CMD_SIZE;
    *cmdSize = size + BASE_CMD_SIZE;
    cmdBuf[0] = CMD_START;
    cmdBuf[1] = CMD_SEND_DATA;
    cmdBuf[2] = (unsigned char)size;
    cmdBuf[3] = (unsigned char)(size >> 8);
    for (i = 0; i < size; i++)
        cmdBuf[i + 4] = buf[i];
    checksum = CyBtldr_ComputeChecksum(cmdBuf, (*cmdSize) - 3);
    cmdBuf[(*cmdSize) - 3] = (unsigned char)checksum;
    cmdBuf[(*cmdSize) - 2] = (unsigned char)(checksum >> 8);
    cmdBuf[(*cmdSize) - 1] = CMD_STOP;

    return CYRET_SUCCESS;
}

int CyBtldr_ParseDefaultCmdResult(unsigned char* cmdBuf, unsigned long cmdSize, unsigned char* status)
{
    int err = CYRET_SUCCESS;

    if (cmdSize != BASE_CMD_SIZE) {
        err = CYRET_ERR_LENGTH;
    } else if (cmdBuf[1] != CYRET_SUCCESS) {
        err = CYRET_ERR_BTLDR_MASK | (*status = cmdBuf[1]);
    } else if (cmdBuf[0] != CMD_START || cmdBuf[2] != 0 || cmdBuf[3] != 0 || cmdBuf[6] != CMD_STOP) {
        err = CYRET_ERR_DATA;
    } else {
        *status = cmdBuf[1];
    }

    return err;
}

int CyBtldr_ParseSendDataCmdResult(unsigned char* cmdBuf, unsigned long cmdSize, unsigned char* status)
{
    return CyBtldr_ParseDefaultCmdResult(cmdBuf, cmdSize, status);
}

int CyBtldr_CreateProgramRowCmd(unsigned char arrayId, unsigned short rowNum, unsigned char* buf, unsigned short size, unsigned char* cmdBuf, unsigned long* cmdSize, unsigned long* resSize)
{
    const unsigned long COMMAND_DATA_SIZE = 3;
    unsigned int checksum;
    unsigned long i;

    *resSize = BASE_CMD_SIZE;
    *cmdSize = BASE_CMD_SIZE + COMMAND_DATA_SIZE + size;
    cmdBuf[0] = CMD_START;
    cmdBuf[1] = CMD_PROGRAM_ROW;
    cmdBuf[2] = (unsigned char)(size + COMMAND_DATA_SIZE);
    cmdBuf[3] = (unsigned char)((size + COMMAND_DATA_SIZE) >> 8);
    cmdBuf[4] = arrayId;
    cmdBuf[5] = (unsigned char)rowNum;
    cmdBuf[6] = (unsigned char)(rowNum >> 8);

    for (i = 0; i < size; i++) {
        cmdBuf[i + 7] = buf[i];
    }
    checksum = CyBtldr_ComputeChecksum(cmdBuf, (*cmdSize) - 3);
    cmdBuf[*cmdSize - 3] = (unsigned char)checksum;
    cmdBuf[*cmdSize - 2] = (unsigned char)(checksum >> 8);
    cmdBuf[*cmdSize - 1] = CMD_STOP;

    return CYRET_SUCCESS;
}

int CyBtldr_ParseProgramRowCmdResult(unsigned char* cmdBuf, unsigned long cmdSize, unsigned char* status)
{
    return CyBtldr_ParseDefaultCmdResult(cmdBuf, cmdSize, status);
}

int ProgramRow(unsigned char arrayID, unsigned short rowNum, unsigned char* buf, unsigned short size)
{
    const int TRANSFER_HEADER_SIZE = 11;

    unsigned char inBuf[MAX_COMMAND_SIZE];
    unsigned char outBuf[MAX_COMMAND_SIZE];
    unsigned long inSize;
    unsigned long outSize;
    unsigned long offset = 0;
    unsigned short subBufSize;
    unsigned char status = CYRET_SUCCESS;
    unsigned int MaxTransferSize = 64u;

    int err = CyBtldr_ValidateRow(arrayID, rowNum);

    /* Break row into pieces to ensure we don't send too much for the transfer protocol */
    while ((CYRET_SUCCESS == err) && ((size - offset + TRANSFER_HEADER_SIZE) > MaxTransferSize))
    {
        subBufSize = (unsigned short)(MaxTransferSize - TRANSFER_HEADER_SIZE);

        err = CyBtldr_CreateSendDataCmd(&buf[offset], subBufSize, inBuf, &inSize, &outSize);
        if (CYRET_SUCCESS == err) {
            err = CyBtldr_TransferData(inBuf, inSize, outBuf, outSize);
        }

        if (CYRET_SUCCESS == err) {
            err = CyBtldr_ParseSendDataCmdResult(outBuf, outSize, &status);
        }

        if (CYRET_SUCCESS != status) {
            err = status | CYRET_ERR_BTLDR_MASK;
        }

        offset += subBufSize;
    }

    if (CYRET_SUCCESS == err)
    {
        subBufSize = (unsigned short)(size - offset);

        err = CyBtldr_CreateProgramRowCmd(arrayID, rowNum, &buf[offset], subBufSize, inBuf, &inSize, &outSize);
        if (CYRET_SUCCESS == err) {
            err = CyBtldr_TransferData(inBuf, inSize, outBuf, outSize);
        }

        if (CYRET_SUCCESS == err) {
            err = CyBtldr_ParseProgramRowCmdResult(outBuf, outSize, &status);
        }

        if (CYRET_SUCCESS != status) {
            err = status | CYRET_ERR_BTLDR_MASK;
        }
    }

    return err;
}

int CyBtldr_CreateVerifyRowCmd(unsigned char arrayId, unsigned short rowNum, unsigned char* cmdBuf, unsigned long* cmdSize, unsigned long* resSize)
{
    const unsigned long RESULT_DATA_SIZE = 1;
    const unsigned long COMMAND_DATA_SIZE = 3;
    const unsigned int COMMAND_SIZE = BASE_CMD_SIZE + COMMAND_DATA_SIZE;
    unsigned short checksum;

    *resSize = BASE_CMD_SIZE + RESULT_DATA_SIZE;
    *cmdSize = COMMAND_SIZE;
    cmdBuf[0] = CMD_START;
    cmdBuf[1] = CMD_VERIFY_ROW;
    cmdBuf[2] = (unsigned char)COMMAND_DATA_SIZE;
    cmdBuf[3] = (unsigned char)(COMMAND_DATA_SIZE >> 8);
    cmdBuf[4] = arrayId;
    cmdBuf[5] = (unsigned char)rowNum;
    cmdBuf[6] = (unsigned char)(rowNum >> 8);
    checksum = CyBtldr_ComputeChecksum(cmdBuf, COMMAND_SIZE - 3);
    cmdBuf[7] = (unsigned char)checksum;
    cmdBuf[8] = (unsigned char)(checksum >> 8);
    cmdBuf[9] = CMD_STOP;

    return CYRET_SUCCESS;
}

int CyBtldr_ParseVerifyRowCmdResult(unsigned char* cmdBuf, unsigned long cmdSize, unsigned char* checksum, unsigned char* status)
{
    const unsigned long RESULT_DATA_SIZE = 1;
    const unsigned long RESULT_SIZE = BASE_CMD_SIZE + RESULT_DATA_SIZE;
    int err = CYRET_SUCCESS;

    if (cmdSize != RESULT_SIZE) {
        err = CYRET_ERR_LENGTH;
    } else if (cmdBuf[1] != CYRET_SUCCESS) {
        err = CYRET_ERR_BTLDR_MASK | (*status = cmdBuf[1]);
    } else if (cmdBuf[0] != CMD_START || cmdBuf[2] != RESULT_DATA_SIZE || cmdBuf[3] != (RESULT_DATA_SIZE >> 8) || cmdBuf[RESULT_SIZE - 1] != CMD_STOP) {
        err = CYRET_ERR_DATA;
    } else {
        *checksum = cmdBuf[4];
        *status = cmdBuf[1];
    }

    return err;
}

int CyBtldr_VerifyRow(unsigned char arrayID, unsigned short rowNum, unsigned char checksum)
{
    unsigned char inBuf[MAX_COMMAND_SIZE];
    unsigned char outBuf[MAX_COMMAND_SIZE];
    unsigned long inSize = 0;
    unsigned long outSize = 0;
    unsigned char rowChecksum = 0;
    unsigned char status = CYRET_SUCCESS;

    int err = CyBtldr_ValidateRow(arrayID, rowNum);
    if (CYRET_SUCCESS == err) {
        err = CyBtldr_CreateVerifyRowCmd(arrayID, rowNum, inBuf, &inSize, &outSize);
    }
    if (CYRET_SUCCESS == err) {
        err = CyBtldr_TransferData(inBuf, inSize, outBuf, outSize);
    }
    if (CYRET_SUCCESS == err) {
        err = CyBtldr_ParseVerifyRowCmdResult(outBuf, outSize, &rowChecksum, &status);
    }
    if (CYRET_SUCCESS != status) {
        err = status | CYRET_ERR_BTLDR_MASK;
    }
    if ((CYRET_SUCCESS == err) && (rowChecksum != checksum)) {
        err = CYRET_ERR_CHECKSUM;
    }

    return err;
}

int CyBtldr_CreateEnterBootLoaderCmd(unsigned char* cmdBuf, unsigned long* cmdSize, unsigned long* resSize)
{
    const unsigned long RESULT_DATA_SIZE = 8;
    unsigned short checksum;

    *resSize = BASE_CMD_SIZE + RESULT_DATA_SIZE;
    *cmdSize = BASE_CMD_SIZE;
    cmdBuf[0] = CMD_START;
    cmdBuf[1] = CMD_ENTER_BOOTLOADER;
    cmdBuf[2] = 0;
    cmdBuf[3] = 0;
    checksum = CyBtldr_ComputeChecksum(cmdBuf, BASE_CMD_SIZE - 3);
    cmdBuf[4] = (unsigned char)checksum;
    cmdBuf[5] = (unsigned char)(checksum >> 8);
    cmdBuf[6] = CMD_STOP;

    return CYRET_SUCCESS;
}

int CyBtldr_StartBootloadOperation(CyBtldr_CommunicationsData* comm, unsigned long expSiId, unsigned char expSiRev, unsigned long* blVer)
{
    unsigned long i;
    unsigned long inSize = 0;
    unsigned long outSize = 0;
    unsigned long siliconId = 0;
    unsigned char inBuf[MAX_COMMAND_SIZE];
    unsigned char outBuf[MAX_COMMAND_SIZE];
    unsigned char siliconRev = 0;
    unsigned char status = CYRET_SUCCESS;
    int err;

    for (i = 0; i < MAX_FLASH_ARRAYS; i++) {
        g_validRows[i] = NO_FLASH_ARRAY_DATA;
    }

    err = CyBtldr_CreateEnterBootLoaderCmd(inBuf, &inSize, &outSize);
    if (CYRET_SUCCESS == err) {
        err = CyBtldr_TransferData(inBuf, inSize, outBuf, outSize);
    }
    if (CYRET_SUCCESS == err) {
        err = CyBtldr_ParseEnterBootLoaderCmdResult(outBuf, outSize, &siliconId, &siliconRev, blVer, &status);
    }

    if (CYRET_SUCCESS == err)
    {
        if (CYRET_SUCCESS != status) {
            err = status | CYRET_ERR_BTLDR_MASK;
        }
        if (expSiId != siliconId || expSiRev != siliconRev) {
            err = CYRET_ERR_DEVICE;
        }
    }

    return err;
}

int CyBtldr_ParseEnterBootLoaderCmdResult(unsigned char* cmdBuf, unsigned long cmdSize, unsigned long* siliconId, unsigned char* siliconRev, unsigned long* blVersion, unsigned char* status)
{
    const unsigned long RESULT_DATA_SIZE = 8;
    const unsigned long RESULT_SIZE = BASE_CMD_SIZE + RESULT_DATA_SIZE;
    int err = CYRET_SUCCESS;

    if (cmdSize != RESULT_SIZE) {
        err = CYRET_ERR_LENGTH;
    } else if (cmdBuf[1] != CYRET_SUCCESS) {
        err = CYRET_ERR_BTLDR_MASK | (*status = cmdBuf[1]);
    } else if (cmdBuf[0] != CMD_START || cmdBuf[2] != RESULT_DATA_SIZE || cmdBuf[3] != (RESULT_DATA_SIZE >> 8) || cmdBuf[RESULT_SIZE - 1] != CMD_STOP) { 
        err = CYRET_ERR_DATA;
    } else {
        *siliconId = (cmdBuf[7] << 24) | (cmdBuf[6] << 16) | (cmdBuf[5] << 8) | cmdBuf[4];
        *siliconRev = cmdBuf[8];
        *blVersion = (cmdBuf[11] << 16) | (cmdBuf[10] << 8) | cmdBuf[9];
        *status = cmdBuf[1];
    }

    return err;
}

int CyBtldr_TransferData(unsigned char* inBuf, int inSize, unsigned char* outBuf, int outSize)
{
    n2g_i2c_if_t i2c_if;
    int rc;
    memset(&i2c_if, 0, sizeof(i2c_if));

    i2c_if.i2c_bus_type = IOFPGA_I2C;
    i2c_if.i2c_ctrl = 0x2;
    i2c_if.i2c_dev = 0x44;
    i2c_if.mux = 0x0;
    i2c_if.offset = -1;

    /* I2c Write */
    i2c_if.size = inSize;
    i2c_if.buf = (char *)&inBuf[0];

    if (n2g_i2c_write(&i2c_if) != RC_I2C_OP_OK) {
        printf("Unable to write i2c\n");
        return (FAILED);
    } 

    msleep(25);

    /* I2c Read */
    i2c_if.size = outSize;
    memset(outBuf, 0, outSize);
    i2c_if.buf = (char *)outBuf;

    if (n2g_i2c_read(&i2c_if) != RC_I2C_OP_OK) {
        printf("Unable to read i2c\n");
        return (FAILED);
    } else {
        rc = CYRET_SUCCESS;
        return rc;
    }
}

int CyBtldr_CreateSetActiveAppCmd(unsigned char appId, unsigned char* cmdBuf, unsigned long* cmdSize, unsigned long* resSize)
{
    const unsigned long COMMAND_DATA_SIZE = 1;
    const unsigned int COMMAND_SIZE = BASE_CMD_SIZE + COMMAND_DATA_SIZE;
    unsigned short checksum;

    *resSize = BASE_CMD_SIZE;
    *cmdSize = COMMAND_SIZE;
    cmdBuf[0] = CMD_START;
    cmdBuf[1] = CMD_SET_ACTIVE_APP;
    cmdBuf[2] = (unsigned char)COMMAND_DATA_SIZE;
    cmdBuf[3] = (unsigned char)(COMMAND_DATA_SIZE >> 8);
    cmdBuf[4] = appId;
    checksum = CyBtldr_ComputeChecksum(cmdBuf, COMMAND_SIZE - 3);
    cmdBuf[5] = (unsigned char)checksum;
    cmdBuf[6] = (unsigned char)(checksum >> 8);
    cmdBuf[7] = CMD_STOP;

    return CYRET_SUCCESS;
}

int CyBtldr_ParseSetActiveAppCmdResult(unsigned char* cmdBuf, unsigned long cmdSize, unsigned char* status)
{
    return CyBtldr_ParseDefaultCmdResult(cmdBuf, cmdSize, status);
}

int CyBtldr_CreateGetAppStatusCmd(int appId, unsigned char* cmdBuf, unsigned long* cmdSize, unsigned long* resSize)
{
    const unsigned long RESULT_DATA_SIZE = 2;
    const unsigned long COMMAND_DATA_SIZE = 1;
    const unsigned int COMMAND_SIZE = BASE_CMD_SIZE + COMMAND_DATA_SIZE;
    unsigned short checksum;

    *resSize = BASE_CMD_SIZE + RESULT_DATA_SIZE;
    *cmdSize = COMMAND_SIZE;
    cmdBuf[0] = CMD_START;
    cmdBuf[1] = CMD_GET_APP_STATUS;
    cmdBuf[2] = (unsigned char)COMMAND_DATA_SIZE;
    cmdBuf[3] = (unsigned char)(COMMAND_DATA_SIZE >> 8);
    cmdBuf[4] = appId;
    checksum = CyBtldr_ComputeChecksum(cmdBuf, COMMAND_SIZE - 3);
    cmdBuf[5] = (unsigned char)checksum;
    cmdBuf[6] = (unsigned char)(checksum >> 8);
    cmdBuf[7] = CMD_STOP;

    return CYRET_SUCCESS;
}

int CyBtldr_ParseGetAppStatusCmdResult(unsigned char* cmdBuf, unsigned long cmdSize, unsigned char* isValid, unsigned char* isActive, unsigned char* status)
{
    const unsigned long RESULT_DATA_SIZE = 2;
    const unsigned long RESULT_SIZE = BASE_CMD_SIZE + RESULT_DATA_SIZE;
    int err = CYRET_SUCCESS;

    if (cmdSize != RESULT_SIZE) {
        err = CYRET_ERR_LENGTH;
    } else if (cmdBuf[1] != CYRET_SUCCESS) {
        err = CYRET_ERR_BTLDR_MASK | (*status = cmdBuf[1]);
    } else if (cmdBuf[0] != CMD_START || cmdBuf[2] != RESULT_DATA_SIZE || cmdBuf[3] != (RESULT_DATA_SIZE >> 8) || cmdBuf[RESULT_SIZE - 1] != CMD_STOP) {
        err = CYRET_ERR_DATA;
    } else {
        *isValid = cmdBuf[4];
        *isActive = cmdBuf[5];
        *status = cmdBuf[1];
    }

    return err;
}

int get_app_status(int app_id, unsigned char* is_valid, unsigned char* is_active)
{
    unsigned long insize = 0;
    unsigned long outsize = 0;
    unsigned char inbuf[MAX_COMMAND_SIZE];
    unsigned char outbuf[MAX_COMMAND_SIZE];
    unsigned char status = CYRET_SUCCESS;
    int err;

    err = CyBtldr_CreateGetAppStatusCmd(app_id, inbuf, &insize, &outsize);
    if (CYRET_SUCCESS == err) {
        err = CyBtldr_TransferData(inbuf, insize, outbuf, outsize);
    } else {
        return (FAILED);
    }

    if (CYRET_SUCCESS == err) {
        err = CyBtldr_ParseGetAppStatusCmdResult(outbuf, outsize, is_valid, is_active, &status);
    } else {
        return (FAILED);
    }

    if (CYRET_SUCCESS == err) {
        if (CYRET_SUCCESS != status) {
            return (FAILED);
        } 
    }

    return (PASSED);
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
    int rc;
    uint16_t data;

    unsigned char cmdBuf[MAX_COMMAND_SIZE];
    unsigned char sw_reset_buf[2];
    unsigned char arrayId;
    unsigned short rowNum;
    unsigned char rowData[300];
    unsigned short rowSize;
    unsigned char err;
    unsigned int lineLen;
    /* Assign silicon ID, Revision and bootloader versoin here to get app status */
    unsigned long siliconID = 0x11111101;
    unsigned char siliconRev = 0x0;
    unsigned long blVer = 0x10132;
    unsigned char packetChkSumType;
    unsigned int lineCntr;

    unsigned char checksum;
    unsigned char checksum2;
    /* get application status */
    unsigned long inSize = 0;
    unsigned long outSize = 0;
    unsigned char inBuf[MAX_COMMAND_SIZE];
    unsigned char outBuf[MAX_COMMAND_SIZE];
    unsigned char status = CYRET_SUCCESS;
    int app_1 = 0;
    int app_2 = 1;
    unsigned char is_valid[1];
    unsigned char is_active[1];
    int active_app;

    int total_row;
    unsigned long exit_inSize = 0;
    unsigned long exit_outSize = 0;
    const char f_1[10] = "_1.h";
    const char f_2[10] = "_2.h";
    char *ret_str;

    /* save fw contents to array */
    char mcu_fw_file[50];
    char check_mcu_file[100]; 
    char ch;
    FILE *fp;
    int mcu_fw_size = 0;
    char *stringImage[234];
    int ix = 0;
    char opt;

    memset(&i2c_if, 0, sizeof(i2c_if));

    printf("Usage : Please copy the MCU image into 'firmware' directory\n");
    printf("        'q' - Not to upgrade and quit only\n");
    printf("        'c' - Continue to upgrade\n");
    printf("Do you want to quit or continue to upgrade : ");
    scanf("%c", &opt);

    if (opt == 'q') {
        return (PASSED);
    }

    /* Get Power Sequencer I2C struct */
    rc = get_pwr_seq_i2c_struct(&i2c_if);
    if (rc != PASSED) {
        printf("%s: Failed to get Power Sequencer I2C structure.",
                     __FUNCTION__);
        return (rc);
    }

    mcu_pin_shorten_check();

    /* set cmd to UPDATE mode */
    i2c_if.offset = PWR_EN_FW_UPGRADE_REG;
    data = PWR_EN_FW_UPGRADE_VAL;
    i2c_if.buf = (char *)&data;
    rc = pwr_write(&i2c_if);
    if (rc != PASSED) {
	printf("Set cmd to update mode failed (offset = %X, data = %x)\n",\
        i2c_if.offset, data);
        return (rc);
    }

    /* Needed - HW suggest to add delay here */
    msleep(300);

    /* Enter Bootload Command */
    err = CyBtldr_StartBootloadOperation(&comm1, siliconID, siliconRev, &blVer);

    /* get application status - to know using which fm to upgrade */
    get_app_status(app_1, is_valid, is_active);
    if ((*is_active == APP_IS_ACTIVE) && (*is_valid == APP_IS_VALID)) {
        printf("- Using Bootloadable application '2' firmware image\n");

        total_row = mcu_fw_size;
        active_app = app_2;
    } else {
        get_app_status(app_2, is_valid, is_active);
        if ((*is_active == APP_IS_ACTIVE) && (*is_valid == APP_IS_VALID)) {
            printf("- Using Bootloadable application '1' firmware image\n");

            total_row = mcu_fw_size;
            active_app = app_1;
        }
    }

    /* End Bootload Operation */
    create_end_bootload_cmd(cmdBuf, &exit_inSize, &exit_outSize);
    i2c_if.offset = -1;
    i2c_if.size = exit_inSize;
    i2c_if.buf = (char *)&cmdBuf[0];

    if (n2g_i2c_write(&i2c_if) != RC_I2C_OP_OK) {
        printf("Unable to write i2c\n");
        return (FAILED);
    } 
    /* get application status end*/

    printf("Enter MCU firmware file name : ");
    scanf("%s", mcu_fw_file);

    /* To prevent user using the wrong application firmware */
    if (active_app == app_1) {
        ret_str = strstr(mcu_fw_file, f_1);
        if (ret_str == NULL) {
            printf("warning - Please use correct application 1 firmware\n");
            return (FAILED);
        } else {
            printf("You're using firmware image %s\n", mcu_fw_file);
        }
    } else { 
        ret_str = strstr(mcu_fw_file, f_2);
        if (ret_str == NULL) {
            printf("warning - Please use correct application 2 firmware\n");
            return (FAILED);
        } else {
            printf("You're using firmware image %s\n", mcu_fw_file);
        }
    } 

    /* Pick up mcu firmware from firmware directory */
    sprintf(check_mcu_file, "/firmware/%s", mcu_fw_file);
    fp = fopen(check_mcu_file, "r"); 
    if (fp == NULL) {
        printf("Warning - File not found, please copy the file into 'firmware' directory");
        return (FAILED);
    }

    while ((ch = fgetc(fp)) != EOF) {
        if (ch == ',') {
            mcu_fw_size++;
        }
    }

    /* Need to plus 1 */
    mcu_fw_size += 1;

    fclose(fp);

    /* Read contents and store to array */
    fp = fopen(check_mcu_file, "r");
    if (fp == NULL) {
        printf("Warning - File not found, please copy the file into 'firmware' directory");
        return (FAILED);
    }

    while ((ch = fgetc(fp)) != EOF) {
        if (ch == '{') {
            break;
        }
    }

    char *pch;
    for (ix = 0; ix < mcu_fw_size; ix++) {
        stringImage[ix] = calloc(272, sizeof(char));
    }

    for (ix = 0; ix < mcu_fw_size; ix++) {
        while ((ch = fgetc(fp)) != EOF) {
            if (ch == '"') {
                break;
            }
        }
        fscanf(fp, "%s", stringImage[ix]);
        pch = strtok(stringImage[ix], " \",");
        while (pch != NULL) {
            pch = strtok(NULL, " \",");
        }
    }

    memset(&i2c_if, 0, sizeof(i2c_if));

    /* Get Power Sequencer I2C struct */
    rc = get_pwr_seq_i2c_struct(&i2c_if);
    if (rc != PASSED) {
        printf("%s: Failed to get Power Sequencer I2C structure.",
                     __FUNCTION__);
        return (rc);
    }

    /* set cmd to UPDATE mode */
    i2c_if.offset = PWR_EN_FW_UPGRADE_REG;
    data = PWR_EN_FW_UPGRADE_VAL;
    i2c_if.buf = (char *)&data;
    rc = pwr_write(&i2c_if);
    if (rc != PASSED) {
	printf("Set cmd to update mode failed (offset = %X, data = %x)\n",\
        i2c_if.offset, data);
        return (rc);
    }

    /* HW suggest to add delay here */
    msleep(300);

    /* Initialize line counter */
    lineCntr = 0u;

    /* Get length of the first line in cyacd_1 file */
    lineLen = strlen(stringImage[lineCntr]);

    /* Parsing */
    err = ParseHeader(lineLen, (unsigned char *)stringImage[lineCntr], &siliconID, &siliconRev, &packetChkSumType);
    if (err != CYRET_SUCCESS) {
        return (FAILED);
    }

    /* Set the packet checksum type */
    SetCheckSumType((CyBtldr_ChecksumType)packetChkSumType);

    /* Enter Bootload Command */
    err = CyBtldr_StartBootloadOperation(&comm1, siliconID, siliconRev, &blVer);

    lineCntr ++;
    total_row = mcu_fw_size;
    printf("Start to upgrade MCU firmware\n");
    while((err == CYRET_SUCCESS) && (lineCntr < total_row)) 
    {
        printf(".");
        fflush(stdout);

        /* Get the string length for the line */
        lineLen = strlen(stringImage[lineCntr]);

        /* Parse row data */
        err = ParseRowData((unsigned int)lineLen, (unsigned char *)stringImage[lineCntr], &arrayId, &rowNum, rowData, &rowSize, &checksum);

        if (err == CYRET_SUCCESS) {
            /* Program Row */
            err = ProgramRow(arrayId, rowNum, rowData, rowSize);

            if (err == CYRET_SUCCESS) {
                /* Verify Row */
                checksum2 = (unsigned char)(checksum + arrayId + rowNum + (rowNum >> 8) + rowSize + (rowSize >> 8));
                err = CyBtldr_VerifyRow(arrayId, rowNum, checksum2);
            }
        }
        /* Increment the lineCntr */
        lineCntr ++;
    }

    printf("\nSuccessfully sent 0x%x rows data to device, totally rows data should be sent is 0x%x\n", lineCntr, total_row);

    if (lineCntr == total_row) {
        /* set active image */
        err = CyBtldr_CreateSetActiveAppCmd(active_app, inBuf, &inSize, &outSize);
        if (CYRET_SUCCESS == err) {
            err = CyBtldr_TransferData(inBuf, inSize, outBuf, outSize);
        } else {
            return (FAILED);
        }

        if (CYRET_SUCCESS == err) {
            err = CyBtldr_ParseSetActiveAppCmdResult(outBuf, outSize, &status);
        } else {
            return (FAILED);
        }

        if (CYRET_SUCCESS == err)
        {
            if (CYRET_SUCCESS != status) {
                return (FAILED);
            }
        }
    }

    /* End Bootload Operation */
    create_end_bootload_cmd(cmdBuf, &exit_inSize, &exit_outSize);
    i2c_if.offset = -1;
    i2c_if.size = exit_inSize;
    i2c_if.buf = (char *)&cmdBuf[0];

    if (n2g_i2c_write(&i2c_if) != RC_I2C_OP_OK) {
        printf("Unable to write i2c\n");
        fclose(fp);
        return (FAILED);
    } else {
        printf("Bootload operation done, reset system...\n");
        fclose(fp);
        /* HW's suggestion - add 1s delay before writing 0x8eca to reg 0x15 */
        msleep(1000);
        for (ix = 0; ix < 3; ix++) {
            i2c_if.offset = 0x15;
            i2c_if.size = 0x2;
            sw_reset_buf[0] = 0x8e;
            sw_reset_buf[1] = 0xca;
            i2c_if.buf = (char *)&sw_reset_buf[0];
            if (n2g_i2c_write(&i2c_if) != RC_I2C_OP_OK) {
                printf("Unable to write i2c\n");
                return (FAILED);
            }
            msleep(1000);
        }
        return (PASSED);
    }
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

/*
 *-----------------------------------------------------------------------------
$Log: platform_pwr_seq.c,v $
Revision 1.1  2020/01/09 01:02:04  jiajliu
Merge Curie 2RU to main trunk

$Endlog$
 *-----------------------------------------------------------------------------
 */
