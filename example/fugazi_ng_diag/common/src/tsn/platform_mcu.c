/* $Id: platform_mcu.c,v 1.4 2019/01/18 05:54:47 yungchen Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/tsn/platform_mcu.c,v $
 *-----------------------------------------------------------------------------
 * 
 * Filename   : platform_mcu.c
 * Description: TSN MCU Library.
 *
 * Copyright (c) 2017 - 2019 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *-----------------------------------------------------------------------------
 */
#include "error.h"
#include "common.h"
#include "types.h"
#include "menu.h"
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>
#include <fcntl.h>
#include <asm/ioctl.h>
#include <sys/stat.h>
#include <signal.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include "assert.h"
#include "byteswap.h"
#include "common_utils.h"
#include "nvmonvars.h"
#include "i2c_address.h"
#include "i2c_api.h"
#include "proto.h"
#include "platform_i2c.h"
#include "platform_mcu.h"
#include "platform_mcu_upgrade.h"
#include "defs.h"
#include "linux_api.h"
#include "free.h"
#include "platform_fpga.h"

/*******************************************************************************
 *                                Externs
 *******************************************************************************
 */

/*******************************************************************************
 *                          Function Declaration
 *******************************************************************************
 */
int        tsn_mcu_utils(int);
int        tsn_mcu_reg_rd(uint32_t, uint16_t *);
int        tsn_mcu_reg_wr(uint32_t, uint16_t);
static int mcu_reg_rd_util(int);
static int mcu_reg_wr_util(int);
/*static int mcu_reg_dump(void); */
static uint16_t pwr_seq_get_val(int16_t, uint);
static int pwr_seq_write_reg(int, uint16_t);
static int mcu_reg_dump(void);

static star_mcu_table mcu_reg_info[] = {
    { "Firmware Revision:", 0x00 },
    { "Debug Pin Code:", 0x01 },
    { "Status: Sticky:", 0x02},
    { "Status: Clearable:", 0x03},
    { "Fault0: Sticky:", 0x04},
    { "Fault1: Sticky:", 0x05},
    { "Fault0: Clearable:", 0x06},
    { "Fault1: Clearable:", 0x07},
    { "Scratchpad 0:", 0x08},
    { "Scratchpad 1:", 0x09},
    { "Scratchpad 2:", 0x0A},
    { "Scratchpad 3:", 0x0B},
    { "Spare:", 0x0C},
    { "RTC Year/Month:", 0x0D},
    { "RTC Date/Hour:", 0x0E},
    { "RTC Minute/Second:", 0x0F},
    { "Black Box Command:", 0x10},
    { "Black Box Data 0: RTC Year/Month:", 0x11 },
    { "Black Box Data 1: RTC Date/Hour:", 0x12 },
    { "Black Box Data 2: RTC Minute/Second:", 0x13 },
    { "Black Box Data 3: Fault Code:", 0x14 },
    { "Voltage:Margin (-5, 0, 5):", 0x17},
    { "System Watchdog Enable:", 0x1A},
    { "System Watchdog Refresh:", 0x1B},
    { "System Watchdog Timeout:", 0x1C},
    { "Power Down Enable:", 0x1D},
    { "Power Down Time:", 0x1E},
    { "Performance Monitor:", 0x1F},
    { "VP12P0 Voltage: Latest Reading:", 0x20},
    { "VP12P0 Voltage: Maximum Reading:", 0x21},
    { "VP12P0 Voltage: Minimum Reading:", 0x22},
    { "VP12P0 Voltage: Ramp Time Maximum and Reading:", 0x23},
    { "VP5P0 Voltage: Latest Reading:", 0x24},
    { "VP5P0 Voltage: Maximum Reading:", 0x25},
    { "VP5P0 Voltage: Minimum Reading:", 0x26},
    { "VP5P0 Voltage: Ramp Time Maximum and Reading:", 0x27},
    { "VP3P3 STBY Voltage: Latest Reading:", 0x28},
    { "VP3P3 STBY Voltage: Maximum Reading:", 0x29},
    { "VP3P3 STBY Voltage: Minimum Reading:", 0x2A},
    { "VP3P3 STBY Voltage: Ramp Time Maximum and Reading:", 0x2B},
    { "VP3P3 Voltage: Latest Reading:", 0x2C},
    { "VP3P3 Voltage: Maximum Reading:", 0x2D},
    { "VP3P3 Voltage: Minimum Reading:", 0x2E},
    { "VP3P3 Voltage: Ramp Time Maximum and Reading:", 0x2F},
    { "VP2P5 Voltage: Latest Reading:", 0x30},
    { "VP2P5 Voltage: Maximum Reading:", 0x31},
    { "VP2P5 Voltage: Minimum Reading:", 0x32},
    { "VP2P5 Voltage: Ramp Time Maximum and Reading:", 0x33},
    { "VP1P8 Voltage: Latest Reading:", 0x34},
    { "VP1P8 Voltage: Maximum Reading:", 0x35},
    { "VP1P8 Voltage: Minimum Reading:", 0x36},
    { "VP1P8 Voltage: Ramp Time Maximum and Reading:", 0x37},
    { "VP1P2 Voltage: Latest Reading:", 0x3C},
    { "VP1P2 Voltage: Maximum Reading:", 0x3D},
    { "VP1P2 Voltage: Minimum Reading:", 0x3E},
    { "VP1P2 Voltage: Ramp Time Maximum and Reading:", 0x3F},
    { "VP0P9 Voltage: Latest Reading:", 0x48},
    { "VP0P9 Voltage: Maximum Reading:", 0x49},
    { "VP0P9_DDR Voltage: Minimum Reading:", 0x4A},
    { "VP0P9_DDR Voltage: Ramp Time Maximum and Reading:", 0x4B},
    { "VP0P6 Voltage: Latest Reading:", 0x4C},
    { "VP0P6 Voltage: Maximum Reading:", 0x4D},
    { "VP0P6 Voltage: Minimum Reading:", 0x4E},
    { "VP0P6 Voltage: Ramp Time Maximum and Reading:", 0x4F},
    { "VP3P0 Voltage: Latest Reading:", 0x50},
    { "VP3P0 Voltage: Maximum Reading:", 0x51},
    { "VP3P0 Voltage: Minimum Reading:", 0x52},
    { "VP3P0 Voltage: Ramp Time Maximum and Reading:", 0x53},
    { "Main Board 12V Current: Latest Reading:", 0x58},
    { "Main Board 12V: Maximum Reading:", 0x59},
    { "Main Board 12V: Minimum Reading:", 0x5A},
    { "Main Board 12V: Ramp Time Maximum and Reading:", 0x5B},
    { "DEBUG: Latest Reading:", 0x78},
    { "DEBUG: Maximum Reading:", 0x79},
    { "DEBUG: Minimum Reading:", 0x7A},
    { "DEBUG: Ramp Time Maximum and Reading:", 0x7B},
    { "VREF Voltage: Latest Reading:", 0x80},
    { "VREF Voltage: Maximum Reading:", 0x81},
    { "VREF Voltage: Minimum Reading:", 0x82},
    { "VREF Voltage: Ramp Time Maximum and Reading:", 0x83},
    { "Temperature: Latest Reading:", 0x84},
    { "Temperature: Maximum Reading:", 0x85},
    { "Temperature: Minimum Reading:", 0x86},
    { "Temperature: Ramp Time Maximum and Reading:", 0x87},
    {NULL, 0x00}
};

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
#if 0
    {"VP3P3 Voltage: Margin (-5, 0, 5)",         PWR_SEQ_3_3_MAR,
	READ_WRITE | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT},
	0xFFFF, 0x0000},
    {"VP2P5 Voltage: Margin (-5, 0, 5)",        PWR_SEQ_2_5_MAR,
	READ_WRITE | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT},
	0xFFFF, 0x0000},
#endif
    {"Voltage: Margin (-5, 0, 5)",       PWR_SEQ_DDR_MAR,
	READ_WRITE | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT},
	0xFFFF, 0x0000},
#if 0
    {"VP1P2 Voltage: Margin (-3 to 3)",	       PWR_SEQ_1_2_MAR, 
	READ_WRITE | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT},
	0xFFFF, 0x0000},
    {"VP1P0 Voltage: Margin (-3 to 3)",       PWR_SEQ_1_0_MAR,
	READ_WRITE | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT},
	0xFFFF, 0x0000},
#endif 
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
    {"Performance Monitor",                 PWR_SEQ_PM,
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
    {"VP12P0 Voltage: Ramp Time Maximum and  Reading",  PWR_SEQ_12_0_RPT,
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
    {"VP3P3 STBY Voltage : Latest Reading",    PWR_SEQ_3_3_STBYLR,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT},
	0x0000, 0x0000},
    {"VP3P3 STBY Voltage: Maximum Reading",    PWR_SEQ_3_3_STBYMAX,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT},
	0x0000, 0x0000},
    {"VP3P3 STBY Voltage: Minimum Reading",    PWR_SEQ_3_3_STBYMIN, 
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT},
	0x0000, 0x0000},
    {"VP3P3 STBY Voltage: Ramp Time Maximum and  Reading",  PWR_SEQ_3_3_STBYRPT,
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
    {"VP0P9 Voltage : Latest Reading",    PWR_SEQ_0_9_LR,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT},
	0x0000, 0x0000},
    {"VP0P9 Voltage: Maximum Reading",    PWR_SEQ_0_9_MAX,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT},
	0x0000, 0x0000},
    {"VP0P9 Voltage: Minimum Reading",    PWR_SEQ_0_9_MIN, 
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT},
	0x0000, 0x0000},
    {"VP0P9 Voltage: Ramp Time Maximum and  Reading",  PWR_SEQ_0_9_RPT,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT},
	0x0000, 0x0000},
    {"VP0P6 Voltage : Latest Reading",    PWR_SEQ_0_6_LR,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT},
	0x0000, 0x0000},
    {"VP0P6 Voltage: Maximum Reading",    PWR_SEQ_0_6_MAX,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT},
	0x0000, 0x0000},
    {"VP0P6 Voltage: Minimum Reading",    PWR_SEQ_0_6_MIN, 
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT},
	0x0000, 0x0000},
    {"VP0P6 Voltage: Ramp Time Maximum and  Reading",  PWR_SEQ_0_6_RPT,
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
    {"Main Board 12V Current : Latest Reading",    PWR_SEQ_12_MB_LR,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT},
	0x0000, 0x0000},
    {"Main Board 12V Current : Maximum Reading",    PWR_SEQ_12_MB_MAX,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT},
	0x0000, 0x0000},
    {"Main Board 12V Current : Minimum Reading",    PWR_SEQ_12_MB_MIN, 
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT},
	0x0000, 0x0000},
    {"Main Board 12V Current : Ramp Time Maximum and  Reading",  PWR_SEQ_12_MB_RPT,
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
    {"DEBUG : Power On Min threshold",    PWR_SEQ_DEBUG_PON_MIN,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT},
	0x0000, 0x0000},
    {"DEBUG : Power Off Minimum threshold",    PWR_SEQ_DEBUG_POFF_MIN,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT},
	0x0000, 0x0000},
    {"DEBUG : Maximum Operational threshold",    PWR_SEQ_DEBUG_MAX_OP,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT},
	0x0000, 0x0000},
    {"DEBUG : Minimum Operational threshold",    PWR_SEQ_DEBUG_MIN_OP,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT},
	0x0000, 0x0000},
    {"VREF Voltage : Power On Min threshold",    PWR_SEQ_VREF_V_PON,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT},
	0x0000, 0x0000},
    {"VREF Voltage : Power Off Minimum threshold",    PWR_SEQ_VREF_V_POFF,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT},
	0x0000, 0x0000},
    {"VREF Voltage : Maximum Operational threshold",    PWR_SEQ_VREF_V_MAX_OP, 
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT},
	0x0000, 0x0000},
    {"VREF Voltage : Minimum Operational threshold",  PWR_SEQ_VREF_V_MIN_OP,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT},
	0x0000, 0x0000},
    {"Temperature : Power On Min threshold",    PWR_SEQ_TMP_PON,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT},
	0x0000, 0x0000},
    {"Temperature : Power Off Minimum threshold",    PWR_SEQ_TMP_POFF,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT},
	0x0000, 0x0000},
    {"Temperature : Maximum Operational threshold",    PWR_SEQ_TMP_MAX_OP, 
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT},
	0x0000, 0x0000},
    {"Temperature : Minimum Operational threshold",  PWR_SEQ_TMP_MIN_OP,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT},
	0x0000, 0x0000},
    {"End", 0xFFFFFFFF, READ_ONLY, {0}, 0, 0},
};
/*******************************************************************************
 *                                Globals
 *******************************************************************************
 */
#define TSN_MCU_ACCESS_TIME   100   /* 100ms */

/*
 * MCU Utilities
 */
static submenu_xtable_t mcu_utils_tbl[] = {
    {"MCU register Read",       (type_t(*)())mcu_reg_rd_util,    0,
     0,                             (type_t(*)())0,                  0,
     (type_t(*)())0,                0},
    {"MCU register Write",      (type_t(*)())mcu_reg_wr_util,    0,
     0,                             (type_t(*)())0,                  0,
     (type_t(*)())0,                0},
    {"MCU version",             (type_t(*)())mcu_fw_show,    0,
     0,                             (type_t(*)())0,                  0,
     (type_t(*)())0,                0},
    {"MCU register dump",       (type_t(*)())mcu_reg_dump,    0,
     0,                             (type_t(*)())0,                  0,
     (type_t(*)())0,                0},
    /* Foxconn & Cisco MCU Upgrade */
    {"MCU upgrade",             (type_t(*)())mcu_fw_upg,    0,
     0,                         (type_t(*)())0,                  0,
     (type_t(*)())0,                0},
    {"Write use update cmd reg to MCU eeprom",    (type_t(*)())pwr_seq_eeprom_wr,    0,
     0,                         (type_t(*)())this_is_star_c1109_4p,                  0,
     (type_t(*)())0,                0},
    {"Read use cmd reg from MCU eeprom",    (type_t(*)())pwr_seq_eeprom_rd,   0,
     0,                         (type_t(*)())this_is_star_c1109_4p,                  0,
     (type_t(*)())0,                0},
    {"Alter MCU register",      (type_t(*)())alter_reg,   0,
     0,                         (type_t(*)())this_is_star_c1109_4p,                  0,
     (type_t(*)())0,                0},
};

#define MCU_UTILS_TBL_SIZE (sizeof(mcu_utils_tbl) / sizeof(submenu_xtable_t))

/* MCU Utilities items (filled in from xtable) */
static mitem_t mcu_utils_pri_items[MCU_UTILS_TBL_SIZE + MAX_BASE_ITEMS];
static mitem_t mcu_utils_sec_items[MCU_UTILS_TBL_SIZE + MAX_BASE_ITEMS];

/* MCU Utils submenu */
menuinfo_t mcu_utils_menu = {
    "%s Menu",
    0,
    (PFT)show_endnote,
    0,
    0,
    mcu_utils_pri_items,
};
menuinfo_t *mcu_utils_menup = &mcu_utils_menu;


/*******************************************************************************
 *
 * Function    : tsn_mcu_utils
 * Description : Function to show TSN MCU utilities submenu.
 * Inputs      : opt - reserved for future use.
 * Outputs     : PASSED / FAILED
 *
 *******************************************************************************
 */
int tsn_mcu_utils (int opt)
{
    build_primary_submenu(mcu_utils_tbl, MCU_UTILS_TBL_SIZE,
                          "MCU Utilities", &mcu_utils_menup);
    build_secondary_submenu(mcu_utils_tbl, MCU_UTILS_TBL_SIZE,
                            mcu_utils_sec_items);

    menu(mcu_utils_menup, mcu_utils_sec_items, '\0');

    return (PASSED);
}

/*******************************************************************************
 *
 * Function    : mcu_reg_rd_util
 * Description : Utility to read TSN MCU register.
 * Inputs      : opt - reserved for future use
 * Outputs     : PASSED/FAILED
 *
 *******************************************************************************
 */
static int mcu_reg_rd_util (int opt)
{
    uint32_t reg_offset = 0;
    uint16_t reg_val = 0;
    
    reg_offset = (uint32_t)gethex_answer("Enter Reg. address: ",
                                         TSN_MCU_VERSION_REG, 0, 0xFF);

    if (tsn_mcu_reg_rd(reg_offset, &reg_val) != PASSED) {
        return (FAILED);
    }
    printf("\nMCU reg.(0x%02X) = 0x%04X\n", reg_offset, reg_val);

    return (PASSED);
}

/*******************************************************************************
 *
 * Function    : mcu_reg_wr_util
 * Description : Utility to write TSN MCU register.
 * Inputs      : opt - reserved for future use
 * Outputs     : PASSED/FAILED
 *
 *******************************************************************************
 */
static int mcu_reg_wr_util (int opt)
{
    uint32_t reg_offset = 0;
    uint16_t orig_val = 0, wr_val = 0;
    
    reg_offset = (uint32_t)gethex_answer("Enter Reg. address: ",
                                         TSN_MCU_PS_MARGIN_REG, 0, 0xFF);

    if (tsn_mcu_reg_rd(reg_offset, &orig_val) != PASSED) {
        return (FAILED);
    }

    wr_val = (uint16_t)gethex_answer("Enter write-in data(hex) ",
                                     orig_val, 0, 0xffff);

    if (tsn_mcu_reg_wr(reg_offset, wr_val) != PASSED) {
        return (FAILED);
    }
    printf("\nDone writing 0x%04X to MCU reg.(0x%02X).\n", wr_val, reg_offset);

    return (PASSED);
}

/*******************************************************************************
 *
 * Function    : tsn_mcu_reg_rd
 * Description : Function to read TSN MCU register.
 * Inputs      : reg_off - register offset
 *               *rd_buf - read buffer
 * Outputs     : PASSED/FAILED
 *
 *******************************************************************************
 */
int tsn_mcu_reg_rd (uint32_t reg_off, uint16_t *rd_buf)
{
    n2g_i2c_if_t *i2c_if;
    int          ret_val = 0;
    uint16_t     reg_val = 0;

    /* Init i2c_if for MCU */
    i2c_if = (n2g_i2c_if_t *)get_n2g_i2c_if(I2C_CTRL_TWO, I2C_MUX_ZERO,
                                            MB_I2C2_MCU);
    if (i2c_if == NULL) {
        printf("%s: Failed to get MCU I2C info.\n", __FUNCTION__);
        return (FAILED);
    }
    i2c_if->offset = reg_off;
    i2c_if->buf = (char *)&reg_val;

    ret_val = n2g_i2c_read(i2c_if);
    if (ret_val != PASSED) {
        printf("%s: Failed to read MCU Reg. %#x.(ret_code = %#x)",
               __FUNCTION__, reg_off, ret_val);
        return (FAILED);
    }

    *rd_buf = (uint16_t)dswap2(reg_val); 

    return (PASSED);
}

/*******************************************************************************
 *
 * Function    : tsn_mcu_reg_wr
 * Description : Function to write TSN MCU register.
 * Inputs      : reg_off - register offset
 *               wr_data - write data
 * Outputs     : PASSED/FAILED
 *
 *******************************************************************************
 */
int tsn_mcu_reg_wr (uint32_t reg_off, uint16_t wr_data)
{
    n2g_i2c_if_t *i2c_if;
    int           ret_val = 0;

    /* Init i2c_if for MCU */
    i2c_if = (n2g_i2c_if_t *)get_n2g_i2c_if(I2C_CTRL_TWO, I2C_MUX_ZERO,
                                            MB_I2C2_MCU);
    if (i2c_if == NULL) {
        printf("%s: Failed to get MCU I2C info.\n", __FUNCTION__);
        return (FAILED);
    }
    i2c_if->offset = reg_off;

    i2c_if->buf = (char *)&wr_data;

    ret_val = n2g_i2c_write(i2c_if);
    if (ret_val != PASSED) {
        printf("%s: Failed to write MCU Reg. %#x.(ret_code = %#x)",
               __FUNCTION__, reg_off, ret_val);
        return (FAILED);
    }
    return (PASSED);
}

/*******************************************************************************
 *
 * Function    : tsn_volts_margin_util
 * Description : Utility to do TSN voltages(1.8/2.5/3.3V) margin.
 * Inputs      : opt - reserved for future use
 * Outputs     : PASSED/FAILED
 *
 *******************************************************************************
 */
int tsn_volts_margin_util (int opt)
{
    uint32_t reg_offset = (uint32_t)TSN_MCU_PS_MARGIN_REG;
    uint16_t m_mode = 0, c_mode = 3;
    
    if (tsn_mcu_reg_rd(reg_offset, &c_mode) != PASSED) {
        printf("Failed to get TSN/STAR/SUPERNOVA current voltage margin setup.\n");
        return (FAILED);
    }

    m_mode = (uint16_t)gethex_answer("Enter Voltage Margin mode"
                                     "(0-Normal, 1-Low, 2-High): ",
                                     c_mode, 0, 2);

    /* 1. Set Voltage Margin to Normal. */
    if ((c_mode & (uint16_t)TSN_MCU_R17_PS_MSK) != TSN_MCU_R17_PS_NORMAL) {

        if (tsn_mcu_reg_wr(reg_offset,
                           (uint16_t)TSN_MCU_R17_PS_NORMAL) != PASSED) {
            return (FAILED);
        }

        msleep(TSN_MCU_ACCESS_TIME);

        if (tsn_mcu_reg_rd(reg_offset, &c_mode) != PASSED) {
            return (FAILED);
        }

        if (c_mode != (uint16_t)TSN_MCU_R17_PS_NORMAL) {
            printf("\nFailed to set TSN/STAR/SUPERNOVA voltage margin to Normal.\n");
            return (FAILED);
        }
    }

    if (m_mode == (uint16_t)TSN_MCU_R17_PS_NORMAL) {
        printf("\nTSN/STAR/SUPERNOVA current Voltage Margin: Normal.\n");
        return (PASSED);
    }

    /* 2. Set Voltage Margin to High/Low based on user requeset. */
    if (tsn_mcu_reg_wr(reg_offset, m_mode) != PASSED) {
        return (FAILED);
    }

    msleep(TSN_MCU_ACCESS_TIME);

    if (tsn_mcu_reg_rd(reg_offset, &c_mode) != PASSED) {
        return (FAILED);
    }

    if ((c_mode & (uint16_t)TSN_MCU_R17_PS_MSK) !=
        (m_mode & (uint16_t)TSN_MCU_R17_PS_MSK)) {
            printf("\nFailed to set TSN/STAR/SUPERNOVA voltage margin to %s.\n",
               ((m_mode & (uint16_t)TSN_MCU_R17_PS_MSK) == TSN_MCU_R17_PS_ML) ?
               "Low" : "High");
        return (FAILED);
    }
        printf("\nTSN/STAR/SUPERNOVA current Voltage Margin: %s.\n",
           ((c_mode & (uint16_t)TSN_MCU_R17_PS_MSK) == TSN_MCU_R17_PS_ML) ?
           "Low" : "High");

    return (PASSED);
}

/*******************************************************************************
 *
 * Function    : tsn_display_voltage
 * Description : Display tsn voltage margin information.
 * Inputs      : None
 * Outputs     : PASSED/FAILED
 *
 *******************************************************************************
 */
int tsn_display_voltage (void)
{
    uint32_t reg_offset = (uint32_t)TSN_MCU_PS_MARGIN_REG;
    uint16_t c_mode = 3;
    
    if (tsn_mcu_reg_rd(reg_offset, &c_mode) != PASSED) {
        printf("Failed to get TSN/STAR/SUPERNOVA current voltage margin setup.\n");
        return (FAILED);
    }

        if (c_mode == 0) { 
            printf("TSN/STAR/SUPERNOVA current Voltage Margin Normal\n");
        } else if (c_mode == 1) {
            printf("TSN/STAR/SUPERNOVA current Voltage Margin Low\n");
        } else if (c_mode == 2) {
            printf("TSN/STAR/SUPERNOVA current Voltage Margin High\n");
        }
    return (PASSED);
}


/*******************************************************************************
 *
 * Function    : _mcu_reg_dump 
 * Description : Dump MCU register information.
 * Inputs      : None
 * Outputs     : PASSED/FAILED
 *
 *******************************************************************************
 */
static int _mcu_reg_dump (void)
{
    uint32_t reg_offset = 0;
    uint16_t reg_val = 0;
    star_mcu_table *mcup;

    printf("MCU Register Display:\n");

    for (reg_offset = 0; reg_offset < 0xFF; reg_offset++) {
        if (reg_offset % 16 == 0) {
            printf("\n");
            printf("0x%02x: ", reg_offset);
        }
        tsn_mcu_reg_rd(reg_offset, &reg_val);

        printf("0x%04x ", reg_val);
    }
    printf("\n");

    mcup = mcu_reg_info;

    while (mcup->p_regs != NULL) {
        tsn_mcu_reg_rd(mcup->reg_addr, &reg_val);
        printf("%s    (0x%04x),data:0x%04x.\n", mcup->p_regs,mcup->reg_addr, reg_val);
        mcup++;
    }

    return (PASSED);
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
int show_reg (void)
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
        if ((result = tsn_mcu_reg_rd(offset, &buffer)) != RC_I2C_OP_OK) {
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
    n2g_i2c_if_t *i2c_if;
    uint32_t     result = FAILED;

    /* Init i2c_if for MCU */
    i2c_if = (n2g_i2c_if_t *)get_n2g_i2c_if(I2C_CTRL_TWO, I2C_MUX_ZERO,
                                            MB_I2C2_MCU);
    if (i2c_if == NULL) {
        printf("%s: Failed to get MCU I2C info.\n", __FUNCTION__);
        return (FAILED);
    }

    i2c_if->buf = (char *)&data;
    i2c_if->offset = offset;
    result = n2g_i2c_write(i2c_if);
    if (result != PASSED) {
        /* Unable to write data */
        printf("%s: Failed to write to Reg(%#x).", __FUNCTION__, offset);
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
int alter_reg (void)
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

    if ((rc = tsn_mcu_reg_rd(offset,  &data)) != PASSED) {
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

/*******************************************************************************
 *
 * Function   : pwr_seq_get_val
 * Description:	Get Power Sequencer Value.
 * Inputs     : data, offset 
 * Outputs    : data
 *
 *******************************************************************************
 */
static uint16_t pwr_seq_get_val (int16_t data, uint offset) 
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
 * Function    : mcu_reg_dump 
 * Description : Dump MCU register information.
 * Inputs      : None
 * Outputs     : PASSED/FAILED
 *
 *******************************************************************************
 */
static int mcu_reg_dump (void)
{
    boolean rc = PASSED;
    if (this_is_star_c1109_4p()){
        printf("Cisco MCU Reg dump:\n");
        rc = show_reg();
    } else {
        printf("Reg dump:\n");
        rc = _mcu_reg_dump();
    }
    return (rc);
}

/*-------------------------------------------------
 * $Log: platform_mcu.c,v $
 * Revision 1.4  2019/01/18 05:54:47  yungchen
 * Merge Supernova branch to the main trunk (CSCvn79871)
 *
 * Revision 1.3  2018/02/09 09:56:55  hondwang
 * Merge Star branch star-branch-c9xx to main trunk
 *
 * Revision 1.2.20.1  2018/01/20 06:27:24  hondwang
 * prepare merge star-branch-c9xx to main trunk
 *
 * Revision 1.2.4.10  2017/12/13 21:10:13  steja
 * Update MCU to support both CISCO MCU and others MCU
 *
 * Revision 1.2.4.9  2017/12/09 04:14:18  steja
 * Support CISCO MCU upgrade
 *
 * Revision 1.2.4.8  2017/11/22 09:45:46  hondwang
 * Fix demo SKU and menu show
 *
 * Revision 1.2.4.7  2017/10/11 21:48:25  hondwang
 * Base C949-4P HW request update mcu reg format
 *
 * Revision 1.2.4.6  2017/10/09 22:13:53  hondwang
 * Add MCU register offset by HW request
 *
 * Revision 1.2.4.5  2017/10/07 02:12:40  hondwang
 * Add FPGA SKU check function to double confirm FPGA info
 *
 * Revision 1.2.4.4  2017/09/30 01:40:58  hondwang
 * Add MCU dump function by Hardware request
 *
 * Revision 1.2.4.3  2017/09/12 03:03:19  hondwang
 * Change margin pin same with TSN by HW request
 *
 * Revision 1.2.4.2  2017/09/12 00:51:08  hondwang
 * Fix Star margin pin high/low hardware swap with TSN
 *
 * Revision 1.2.4.1  2017/08/15 14:18:39  hondwang
 * star branch c9xx initial check in
 *
 * Revision 1.2  2017/08/02 14:21:48  steja
 * Support TSN-H/M platform code
 *
 * Revision 1.1.8.2  2017/07/29 03:41:20  steja
 * tsn-branch5 synced with Maintrunk repositories
 *
 * Revision 1.1.6.3  2017/07/21 10:46:03  steja
 * Update based on code review comment
 *
 * Revision 1.1.6.2  2017/07/20 13:38:07  steja
 * tsn-branch4 merge with maintrunk
 *
 * Revision 1.1.4.3.6.1  2017/07/10 06:56:45  hondwang
 * add mcu upgrade function
 *
 * Revision 1.1.4.3  2016/10/11 13:09:01  steja
 * Add show current voltage in initial diag
 *
 * Revision 1.1.4.2  2016/06/30 06:22:50  steja
 * tsn-branch2 sync with main trunk
 *
 * Revision 1.1.2.1  2016/06/21 04:36:33  palin2
 * Added voltage margin utility and MCU register R/W utilities.
 *
 * $Endlog$
 *-------------------------------------------------
 */

