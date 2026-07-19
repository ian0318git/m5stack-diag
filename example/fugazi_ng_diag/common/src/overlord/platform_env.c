/* $Id: platform_env.c,v 1.5 2017/07/10 02:51:58 leschen Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/overlord/platform_env.c,v $
 *------------------------------------------------------------------
 * Filename:	platform_env.c
 *
 * Description: Informers Environment MCU I2C device.
 *
 *		Refer to EDCS-534569 for more info.
 *		Environment MCU does not handle Restart. A write of the
 *		register offset must be performed for read.
 *
 * Copyright (c) 2013-2017 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "endians.h"
#include "byteswap.h"
#include "common.h"
#include "platform_env.h"
#include "defs.h"
#include "proto.h"
#include "error.h"
#include "mon_plat_defs.h"
#include "cross_platform.h"
#include "platform_i2c.h"
#include "menu.h"
#include "n2g_api_rc.h"
#include "queryflags.h"
#include "platform_psu.h"
#include "nvsysvars.h"
#include "i2c_address.h"
#include "dash_fpga.h"
#include "goofy_i2c.h"

extern unsigned long dash_fpga;
#define ENV_MCU_INIT_TIME 2000


/* Function prototypes */
static int get_env_mcu_i2c_struct(n2g_i2c_if_t *);
static int show_reg(void);
static int write_reg(int, uint16_t);
static int read_reg(int, uint16_t *);
static int write_env_reg(int);
static int slow_fan_speed(int);
static int read_env_reg(int);
static int alter_reg(void);
static int pwm_sweep(void);
static int fan_ctl(void);
static int env_read(n2g_i2c_if_t *);
static int env_write(n2g_i2c_if_t *);
static int get_no_of_fans(void);
static int show_fan_speed(n2g_i2c_if_t *, int, int); 
static int env_reg_test(int);
static int env_alert_temp_test(n2g_i2c_if_t *, int, char *);
static int env_temp_alert_test(int);
static int env_ext_alert_test(int);
static int env_temp_alert_enable_test(int);
static int env_hyst_test(int);
static int env_fan_en_test(int);
static int env_wdg_test(int);
static int env_hys_test(int);
static int env_time_hys_test(int);
static int hyst_stat_check(n2g_i2c_if_t *, env_hyst_test_t *, char *);
static int reg_wr_rd_test(n2g_i2c_if_t *);
static int env_get_status(n2g_i2c_if_t *, ren_t *, ren_t *,
			  char *, char *);
static int env_clear_stat2(n2g_i2c_if_t *, ren_t *,
			   char *, char *);
static int env_regs_access(n2g_i2c_if_t *, int,
			   dev_env_mcu_t *, char *);
static int env_force_fan_test(int);
static int env_temp_offset_test(int);
static int  show_all_fans_speed(int);

/* in hwic_spidey_ct3.c */
extern uint32 err_report(dev_object_t *, char *, uint32);
extern void build_fan_utils_menu(void);
extern int get_env_mcu_i2c_struct(n2g_i2c_if_t *);
extern int ovld_show_mb_watts(void);
extern uint32 cterr_db_print(char *fmtptr, ...);

/* Global variables */
static ren_t env_ver = 0;
static reg_info_t env_mcu_table[]=
{
/*  Register name,		Offset,		Type, Size,
 *		Mask, Reset Value
 */
    {"Firmware Revision",	ENV_MCU_REV,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT},
	0xFFFF, 0},
    {"Status 1",		ENV_MCU_STA1,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT},
	0xFFBF, 0x0001},
    {"Status 2",		ENV_MCU_STA2,
	READ_WRITE | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT},
	0x0000, 0x0000},
    {"Inlet 1 Temperature in Celsius",	ENV_MCU_I1T,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS | REG_DEV, {(uint)REG_EXT},
	0xFFFF, 0},
    {"Inlet 2 Temperature in Celsius",	ENV_MCU_I2T,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS | REG_DEV, {(uint)REG_EXT},
	0xFFFF, 0},
    {"Outlet 1 Temperature in Celsius",	ENV_MCU_O1T,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS | REG_DEV, {(uint)REG_EXT},
	0xFFFF, 0},
    {"Outlet 2 Temperature in Celsius",	ENV_MCU_O2T,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS | REG_DEV, {(uint)REG_EXT},
	0xFFFF, 0},
    {"Fan 1 Tachometer RPM",	ENV_MCU_F1T,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS | REG_DEV, {(uint)REG_EXT},
	0xFFFF, 0},
    {"Fan 2 Tachometer RPM",	ENV_MCU_F2T,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS | REG_DEV, {(uint)REG_EXT},
	0xFFFF, 0},
    {"Fan 3 Tachometer RPM",	ENV_MCU_F3T,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS | REG_DEV, {(uint)REG_EXT},
	0xFFFF, 0},
    {"Fan 4 Tachometer RPM",	ENV_MCU_F4T,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS | REG_DEV, {(uint)REG_EXT},
	0xFFFF, 0},
    {"Fan 5 Tachometer RPM",	ENV_MCU_F5T,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS | REG_DEV, {(uint)REG_EXT},
	0xFFFF, 0},
    {"+12V Current Measurement", ENV_MCU_CUR,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT},
	0x00FF, 0},
    {"+12V Maximum Current Measurement", ENV_MCU_MCM,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT},
	0x00FF, 0},
    {"Control 1",		ENV_MCU_CTL1,
	READ_WRITE | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT},
	0xDFBF, 0x81BF},
    {"Control 2",		ENV_MCU_CTL2,
	READ_WRITE | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT},
	0xEC00, 0xE000},
    {"Temperature 1 L2 to L1 in Celsius", ENV_MCU_T1,
	READ_WRITE | SAVE_RESTORE | REG_ACCESS | REG_DEV, {(uint)REG_EXT},
	0xFFFF, 23},
    {"Temperature 2 L1 to L2 in Celsius", ENV_MCU_T2,
	READ_WRITE | SAVE_RESTORE | REG_ACCESS | REG_DEV, {(uint)REG_EXT},
	0xFFFF, 27},
    {"Temperature 3 L3 to L2 in Celsius", ENV_MCU_T3,
	READ_WRITE | SAVE_RESTORE | REG_ACCESS | REG_DEV, {(uint)REG_EXT},
	0xFFFF, 28},
    {"Temperature 4 L2 to L3 in Celsius", ENV_MCU_T4,
	READ_WRITE | SAVE_RESTORE | REG_ACCESS | REG_DEV, {(uint)REG_EXT},
	0xFFFF, 32},
    {"Fan 1 Level 1 Speed",	ENV_MCU_F1L,
	READ_WRITE | SAVE_RESTORE | REG_ACCESS | REG_DEV, {(uint)REG_EXT},
	0xFFFF, 40},
    {"Fan 1 Level 2 Speed",	ENV_MCU_F1M,
	READ_WRITE | SAVE_RESTORE | REG_ACCESS | REG_DEV, {(uint)REG_EXT},
	0xFFFF, 45},
    {"Fan 1 Level 3 Speed",	ENV_MCU_F1H,
	READ_WRITE | SAVE_RESTORE | REG_ACCESS | REG_DEV, {(uint)REG_EXT},
	0xFFFF, 63},
    {"Fan 2 Level 1 Speed",	ENV_MCU_F2L,
	READ_WRITE | SAVE_RESTORE | REG_ACCESS | REG_DEV, {(uint)REG_EXT},
	0xFFFF, 40},
    {"Fan 2 Level 2 Speed",	ENV_MCU_F2M,
	READ_WRITE | SAVE_RESTORE | REG_ACCESS | REG_DEV, {(uint)REG_EXT},
	0xFFFF, 45},
    {"Fan 2 Level 3 Speed",	ENV_MCU_F2H,
	READ_WRITE | SAVE_RESTORE | REG_ACCESS | REG_DEV, {(uint)REG_EXT},
	0xFFFF, 63},
    {"Fan 3 Level 1 Speed",	ENV_MCU_F3L,
	READ_WRITE | SAVE_RESTORE | REG_ACCESS | REG_DEV, {(uint)REG_EXT},
	0xFFFF, 40},
    {"Fan 3 Level 2 Speed",	ENV_MCU_F3M,
	READ_WRITE | SAVE_RESTORE | REG_ACCESS | REG_DEV, {(uint)REG_EXT},
	0xFFFF, 45},
    {"Fan 3 Level 3 Speed",	ENV_MCU_F3H,
	READ_WRITE | SAVE_RESTORE | REG_ACCESS | REG_DEV, {(uint)REG_EXT},
	0xFFFF, 63},
    {"Fan 4 Level 1 Speed",	ENV_MCU_F4L,
	READ_WRITE | SAVE_RESTORE | REG_ACCESS | REG_DEV, {(uint)REG_EXT},
	0xFFFF, 40},
    {"Fan 4 Level 2 Speed",	ENV_MCU_F4M,
	READ_WRITE | SAVE_RESTORE | REG_ACCESS | REG_DEV, {(uint)REG_EXT},
	0xFFFF, 45},
    {"Fan 4 Level 3 Speed",	ENV_MCU_F4H,
	READ_WRITE | SAVE_RESTORE | REG_ACCESS | REG_DEV, {(uint)REG_EXT},
	0xFFFF, 63},
    {"Fan 5 Level 1 Speed",	ENV_MCU_F5L,
	READ_WRITE | SAVE_RESTORE | REG_ACCESS | REG_DEV, {(uint)REG_EXT},
	0xFFFF, 40},
    {"Fan 5 Level 2 Speed",	ENV_MCU_F5M,
	READ_WRITE | SAVE_RESTORE | REG_ACCESS | REG_DEV, {(uint)REG_EXT},
	0xFFFF, 45},
    {"Fan 5 Level 3 Speed",	ENV_MCU_F5H,
	READ_WRITE | SAVE_RESTORE | REG_ACCESS | REG_DEV, {(uint)REG_EXT},
	0xFFFF, 63},
    {"Fan Control Loop Proportional Constant",	ENV_MCU_PROP,
	READ_WRITE | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT},
	0xFFFF, 0x0100},
    {"Fan PWM Slope",		ENV_MCU_PWS,
	READ_WRITE | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT},
	0xFFFF, 2},
    {"Fan Hysteresis Timeout",	ENV_MCU_HTO,
	READ_WRITE | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT},
	0xFFFF, 30},
    {"Fan Tray Removal Timeout", ENV_MCU_TRT,
	READ_WRITE | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT},
	0xFFFF, 180},
    {"Inlet 1 Alert Temperature in Celsius", ENV_MCU_I1A,
	READ_WRITE | SAVE_RESTORE | REG_ACCESS | REG_DEV, {(uint)REG_EXT},
	0xFFFF, 55},
    {"Inlet 2 Alert Temperature in Celsius", ENV_MCU_I2A,
	READ_WRITE | SAVE_RESTORE | REG_ACCESS | REG_DEV, {(uint)REG_EXT},
	0xFFFF, 55},
    {"Outlet 1 Alert Temperature in Celsius", ENV_MCU_O1A,
	READ_WRITE | SAVE_RESTORE | REG_ACCESS | REG_DEV, {(uint)REG_EXT},
	0xFFFF, 72},
    {"Outlet 2 Alert Temperature in Celsius", ENV_MCU_O2A,
	READ_WRITE | SAVE_RESTORE | REG_ACCESS | REG_DEV, {(uint)REG_EXT},
	0xFFFF, 72},
    {"Alert Shutdown Timer",	ENV_MCU_AST,
	READ_WRITE | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT},
	0x00FF, 60},
    {"Power Sequencer Reset",	ENV_MCU_PSR,
	READ_WRITE | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT},
	0xFFFF, 0},
    {"Power Sequencer Reset Timeout", ENV_MCU_PRT,
	READ_WRITE | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT},
	0xFFFF, 15},
    {"Watchdog Reset Counter",	ENV_MCU_WRT,
	READ_WRITE | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT},
	0xFFFF, 0x0000},
    {"Scratchpad",		ENV_MCU_SCR,
	READ_WRITE | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT},
	0xFFFF, 0},
    {"Fan Dynamic Proportional RPM Threshold",	ENV_MCU_DPT,
	READ_WRITE | SAVE_RESTORE | REG_ACCESS | REG_DEV, {(uint)REG_EXT},
	0xFFFF, 1250},
    {"Fan Dynamic Proportional Upper Value",	ENV_MCU_DPH,
	READ_WRITE | SAVE_RESTORE | REG_ACCESS | REG_DEV, {(uint)REG_EXT},
	0xFFFF, 400},
    {"Fan Dynamic Proportional Lower Value",	ENV_MCU_DPL,
	READ_WRITE | SAVE_RESTORE | REG_ACCESS | REG_DEV, {(uint)REG_EXT},
	0xFFFF, 100},
    {"Fan Dynamic Proportional Step Value",	ENV_MCU_DPS,
	READ_WRITE | SAVE_RESTORE | REG_ACCESS | REG_DEV, {(uint)REG_EXT},
	0xFFFF, 50},
    {"Temperature 5 L4 to L3",	ENV_MCU_T5,
	READ_WRITE | SAVE_RESTORE | REG_ACCESS | REG_DEV, {(uint)REG_EXT},
	0xFFFF, 33},
    {"Temperature 6 L3 to L4",	ENV_MCU_T6,
	READ_WRITE | SAVE_RESTORE | REG_ACCESS | REG_DEV, {(uint)REG_EXT},
	0xFFFF, 37},
    {"Temperature 7 L5 to L4",	ENV_MCU_T7,
	READ_WRITE | SAVE_RESTORE | REG_ACCESS | REG_DEV, {(uint)REG_EXT},
	0xFFFF, 38},
    {"Temperature 8 L4 to L5",	ENV_MCU_T8,
	READ_WRITE | SAVE_RESTORE | REG_ACCESS | REG_DEV, {(uint)REG_EXT},
	0xFFFF, 42},
    {"Fan 1 Level 4 Speed",	ENV_MCU_F1L4,
	READ_WRITE | SAVE_RESTORE | REG_ACCESS | REG_DEV, {(uint)REG_EXT},
	0xFFFF, 82},
    {"Fan 1 Level 5 Speed",	ENV_MCU_F1L5,
	READ_WRITE | SAVE_RESTORE | REG_ACCESS | REG_DEV, {(uint)REG_EXT},
	0xFFFF, 98},
    {"Fan 2 Level 4 Speed",	ENV_MCU_F2L4,
	READ_WRITE | SAVE_RESTORE | REG_ACCESS | REG_DEV, {(uint)REG_EXT},
	0xFFFF, 82},
    {"Fan 2 Level 5 Speed",	ENV_MCU_F2L5,
	READ_WRITE | SAVE_RESTORE | REG_ACCESS | REG_DEV, {(uint)REG_EXT},
	0xFFFF, 98},
    {"Fan 3 Level 4 Speed",	ENV_MCU_F3L4,
	READ_WRITE | SAVE_RESTORE | REG_ACCESS | REG_DEV, {(uint)REG_EXT},
	0xFFFF, 82},
    {"Fan 3 Level 5 Speed",	ENV_MCU_F3L5,
	READ_WRITE | SAVE_RESTORE | REG_ACCESS | REG_DEV, {(uint)REG_EXT},
	0xFFFF, 98},
    {"Fan 4 Level 4 Speed",	ENV_MCU_F4L4,
	READ_WRITE | SAVE_RESTORE | REG_ACCESS | REG_DEV, {(uint)REG_EXT},
	0xFFFF, 82},
    {"Fan 4 Level 5 Speed",	ENV_MCU_F4L5,
	READ_WRITE | SAVE_RESTORE | REG_ACCESS | REG_DEV, {(uint)REG_EXT},
	0xFFFF, 98},
    {"Fan 5 Level 4 Speed",	ENV_MCU_F5L4,
	READ_WRITE | SAVE_RESTORE | REG_ACCESS | REG_DEV, {(uint)REG_EXT},
	0xFFFF, 82},
    {"Fan 5 Level 5 Speed",	ENV_MCU_F5L5,
	READ_WRITE | SAVE_RESTORE | REG_ACCESS | REG_DEV, {(uint)REG_EXT},
	0xFFFF, 98},
    {"Soft Reset/Watchdog Test", ENV_MCU_SFT_WDG,
	READ_WRITE | SAVE_RESTORE | REG_ACCESS | REG_DEV, {(uint)REG_EXT},
	0xFFFF, 0},
    {"Maximum Inlet 1 Temperature", ENV_MCU_MI1T,
	READ_WRITE | SAVE_RESTORE | REG_ACCESS | REG_DEV, {(uint)REG_EXT},
	0xFFFF, 0},
    {"Maximum Inlet 2 Temperature", ENV_MCU_MI2T,
	READ_WRITE | SAVE_RESTORE | REG_ACCESS | REG_DEV, {(uint)REG_EXT},
	0xFFFF, 0},
    {"Maximum Outlet 1 Temperature", ENV_MCU_MO1T,
	READ_WRITE | SAVE_RESTORE | REG_ACCESS | REG_DEV, {(uint)REG_EXT},
	0xFFFF, 0},
    {"Maximum Outlet 2 Temperature", ENV_MCU_MO2T,
	READ_WRITE | SAVE_RESTORE | REG_ACCESS | REG_DEV, {(uint)REG_EXT},
	0xFFFF, 0},
    {"Working Temperature",	ENV_MCU_WRKT,
	READ_ONLY | SAVE_RESTORE | REG_ACCESS | REG_DEV, {(uint)REG_EXT},
	0xFFFF, 0},
    {"Inlet 1 Temperature Offset", ENV_MCU_I1TO,
	READ_WRITE | SAVE_RESTORE | REG_ACCESS | REG_DEV, {(uint)REG_EXT},
	0x00FF, 0},
    {"Inlet 2 Temperature Offset", ENV_MCU_I2TO,
	READ_WRITE | SAVE_RESTORE | REG_ACCESS | REG_DEV, {(uint)REG_EXT},
	0x00FF, 0},
    {"Outlet 1 Temperature Offset", ENV_MCU_O1TO,
	READ_WRITE | SAVE_RESTORE | REG_ACCESS | REG_DEV, {(uint)REG_EXT},
	0x00FF, 0},
    {"Outlet 2 Temperature Offset", ENV_MCU_O2TO,
	READ_WRITE | SAVE_RESTORE | REG_ACCESS | REG_DEV, {(uint)REG_EXT},
	0x00FF, 0},
    {"End", 0xFFFFFFFF, READ_ONLY, {0}, 0, 0},
};

/* Simple registers read/write test table */
static env_reg_test_t simple_reg_test_tbl[] =
{
    {ENV_REG_TEST_OP1, ENV_MCU_REV},
    {ENV_REG_TEST_OP2, ENV_MCU_T1},
    {ENV_REG_TEST_OP2, ENV_MCU_T2},
    {ENV_REG_TEST_OP2, ENV_MCU_T3},
    {ENV_REG_TEST_OP2, ENV_MCU_T4},
    {ENV_REG_TEST_OP2, ENV_MCU_I1A},
    {ENV_REG_TEST_OP2, ENV_MCU_I2A},
    {ENV_REG_TEST_OP2, ENV_MCU_O1A},
    {ENV_REG_TEST_OP2, ENV_MCU_O2A},
    {ENV_REG_TEST_OP3, ENV_MCU_F5L},
    {ENV_REG_TEST_OP3, ENV_MCU_F5M},
    {ENV_REG_TEST_OP3, ENV_MCU_F5H},
    {ENV_REG_TEST_OP3, ENV_MCU_HTO},
    {ENV_REG_TEST_OP3, ENV_MCU_TRT},
    {ENV_REG_TEST_OP3, ENV_MCU_DPT},
    {ENV_REG_TEST_OP3, ENV_MCU_DPH},
    {ENV_REG_TEST_OP3, ENV_MCU_DPL},
    {ENV_REG_TEST_OP3, ENV_MCU_DPS},
#ifndef ENV_V203
    {ENV_REG_TEST_OP3, ENV_MCU_AST},
#endif /* ENV_V203 */
    {ENV_REG_TEST_OP3, ENV_MCU_PSR},
    {ENV_REG_TEST_OP3, ENV_MCU_PRT},
    {ENV_REG_TEST_OP3, ENV_MCU_WRT},
    {ENV_REG_TEST_OP3, ENV_MCU_SCR},
#ifndef ENV_V203
    {ENV_REG_TEST_OP4, ENV_MCU_PROP},
#endif /* ENV_V203 */
    {ENV_REG_TEST_OP4, ENV_MCU_PWS},
    {ENV_REG_TEST_INVALID, 0},
};

/* Control/Status Register Tests */
/*   Temperature Alert Test */
static env_temp_alert_test_t env_temp_alert_test_tbl[] =
{
    {ENV_MCU_I1T, ENV_MCU_I1A, ENV_MCU_STAT1_IN1_ALT},  /* */
    {ENV_MCU_I2T, ENV_MCU_I2A, ENV_MCU_STAT1_IN2_ALT},  /* */
    {ENV_MCU_O1T, ENV_MCU_O1A, ENV_MCU_STAT1_OUT1_ALT}, /* */
    {ENV_MCU_O2T, ENV_MCU_O2A, ENV_MCU_STAT1_OUT2_ALT},
    {0, 0, 0},
};

#if 0
/*
 * Environmental Control Unit Test Menu
 */
static submenu_xtable_t env_test_menu_table[] = {
    {"Simple Registers Test", (PFT)env_reg_test, TRUE,
	MF_CONTINUOUS | MF_DOALL, (type_t(*)())0, 0, (PFT)env_reg_test, TRUE},
    {"Temperature Alert Test", (PFT)env_temp_alert_test, TRUE,
	MF_CONTINUOUS | MF_DOALL, (type_t(*)())0, 0, (PFT)env_temp_alert_test, TRUE},
    {"External Temperature Alert Test", (PFT)env_ext_alert_test, TRUE,
	MF_CONTINUOUS | MF_DOALL, (type_t(*)())0, 0, (PFT)env_ext_alert_test, TRUE},
    {"Temperature Alert enable test", (PFT)env_temp_alert_enable_test, TRUE,
	MF_CONTINUOUS | MF_DOALL,(type_t(*)())0,0, (PFT)env_temp_alert_enable_test,TRUE},
    {"Hysteresis Temperature out-of-Order Test", (PFT)env_hyst_test, TRUE,
	MF_CONTINUOUS | MF_DOALL, (type_t(*)())0, 0, (PFT)env_hyst_test, TRUE},
    {"No Fan Enable Test", (PFT)env_fan_en_test, TRUE,
	MF_CONTINUOUS | MF_DOALL, (type_t(*)())0, 0, (PFT)env_fan_en_test, TRUE},
    {"Watchdog Test", (PFT)env_wdg_test, TRUE,
	MF_CONTINUOUS | MF_DOALL, (type_t(*)())0, 0, (PFT)env_wdg_test, TRUE},
    {"Hysteresis Test", (PFT)env_hys_test, TRUE,
	MF_CONTINUOUS | MF_DOALL, (type_t(*)())0, 0, (PFT)env_hys_test, TRUE},
    {"Timed Hysteresis Test", (PFT)env_time_hys_test, TRUE,
	MF_CONTINUOUS | MF_DOALL, (type_t(*)())0, 0, (PFT)env_time_hys_test, TRUE},
    {"Force Fan Alert Test", (PFT)env_force_fan_test, TRUE,
	MF_CONTINUOUS | MF_DOALL, (type_t(*)())0, 0, (PFT)env_force_fan_test, TRUE},
    {"Temperature Offset Test", (PFT)env_temp_offset_test, TRUE,
	MF_CONTINUOUS | MF_DOALL, (type_t(*)())0, 0, (PFT)env_temp_offset_test, TRUE},
};

#define ENV_TEST_MENU_TAB_SIZE (sizeof(env_test_menu_table) / \
				sizeof(submenu_xtable_t))

/*
 * Primary & secondary submenu items (filled in from xtable)
 */
static mitem_t env_test_menu_pri_items[ENV_TEST_MENU_TAB_SIZE + MAX_BASE_ITEMS];
static mitem_t env_test_menu_sec_items[ENV_TEST_MENU_TAB_SIZE + MAX_BASE_ITEMS];

static struct menuinfo env_test_diag = {
    "Environmental Control Unit Test Menu", /* title */
    0,		 		/* title string added by init_empty_menu */
    (PFT)menu_show_dflags,	/* shows major flags */
    0,				/* generic prompt */
    0,				/* size -- bumped by add_menu_item() */
    env_test_menu_pri_items,
};

static struct menuinfo *env_test_diagp = &env_test_diag;
#endif

/*
 * Environmental Control Unit Menu
 */
static submenu_xtable_t env_menu_table[] = {
    {"Show Environmental Control Unit registers", (PFT)show_reg,                      0,
	0,                                        (type_t(*)())0, 0, (PFT)0, 0},
    {"Read reg x",                                (PFT)read_env_reg,                  0,
	0,                                        (type_t(*)())0, 0, (PFT)0, 0},
    {"Write reg ",                                (PFT)write_env_reg,                 0,
	0,                                        (type_t(*)())0, 0, (PFT)0, 0},
    {"Slow fan speed ",                           (PFT)slow_fan_speed,                0,
	0,                                        (type_t(*)())0, 0, (PFT)0, 0},
    {"Alter Environmental Control Unit register", (PFT)alter_reg,                     0,
	0,                                        (type_t(*)())0, 0, (PFT)0, 0},
    {"Initialize Environmental Control Unit",     (PFT)init_env,                      0,
	0,                                        (type_t(*)())0, 0, (PFT)0, 0},
    {"PWM Sweep",		                  (PFT)pwm_sweep,                     0,
	0,                                        (type_t(*)())0, 0, (PFT)0, 0},
#if 0
    {"Environmental Control Unit tests",          (PFT)build_env_test_menu, FALSE,
	MF_CONTINUOUS | MF_DOALL, (type_t(*)())0, 0,(PFT)build_env_test_menu,TRUE},
#endif
    {"Fan Control utility",                       (PFT)fan_ctl,                       0,
	0,                                        (type_t(*)())0, 0, (PFT)0, 0},
    {"Show Motherboard Wattage",                  (PFT)ovld_show_mb_watts,            0,
	0,                                        (type_t(*)())0, 0, (PFT)0, 0},
    {"Simple Registers Test",                     (PFT)env_reg_test,               TRUE,
	(MF_CONTINUOUS | MF_DOALL),               (type_t(*)())0, 0, (PFT)0, 0},
    {"Temperature Alert Test",                    (PFT)env_temp_alert_test,        TRUE,
	(MF_CONTINUOUS | MF_DOALL),               (type_t(*)())0, 0, (PFT)0, 0},
    {"External Temperature Alert Test",           (PFT)env_ext_alert_test,         TRUE,
	(MF_CONTINUOUS | MF_DOALL),               (type_t(*)())0, 0, (PFT)0, 0},
    {"Temperature Alert enable test",             (PFT)env_temp_alert_enable_test, TRUE,
	(MF_CONTINUOUS | MF_DOALL),               (type_t(*)())0, 0, (PFT)0, 0},
    {"Hysteresis Temperature out-of-Order Test",  (PFT)env_hyst_test,              TRUE,
	(MF_CONTINUOUS | MF_DOALL),               (type_t(*)())0, 0, (PFT)0, 0},
    {"No Fan Enable Test",                        (PFT)env_fan_en_test,            TRUE,
	(MF_CONTINUOUS | MF_DOALL),               (type_t(*)())0, 0, (PFT)0, 0},
    {"Watchdog Test",                             (PFT)env_wdg_test,               TRUE,
	(MF_CONTINUOUS | MF_DOALL),               (type_t(*)())0, 0, (PFT)0, 0},
    {"Hysteresis Test",                           (PFT)env_hys_test,               TRUE,
	(MF_CONTINUOUS | MF_DOALL),               (type_t(*)())0, 0, (PFT)0, 0},
    {"Timed Hysteresis Test",                     (PFT)env_time_hys_test,          TRUE,
	(MF_CONTINUOUS | MF_DOALL),               (type_t(*)())0, 0, (PFT)0, 0},
    {"Force Fan Alert Test",                      (PFT)env_force_fan_test,         TRUE,
	(MF_CONTINUOUS | MF_DOALL),               (type_t(*)())0, 0, (PFT)0, 0},
    {"Temperature Offset Test",                   (PFT)env_temp_offset_test,       TRUE,
	(MF_CONTINUOUS | MF_DOALL),               (type_t(*)())0, 0, (PFT)0, 0},
};

#define ENV_MENU_TABLE_SIZE (sizeof(env_menu_table) / sizeof(submenu_xtable_t))

/*
 * Primary & secondary submenu items (filled in from xtable)
 */
static mitem_t env_menu_primary_items[ENV_MENU_TABLE_SIZE + MAX_BASE_ITEMS];
static mitem_t env_menu_secondary_items[ENV_MENU_TABLE_SIZE + MAX_BASE_ITEMS];

static struct menuinfo envdiag = {
    "Environmental Control Unit Utility Menu",	/* title */
    0,				/* title string added by init_empty_menu */
    (PFT)menu_show_dflags,	/* shows major flags */
    0,				/* generic prompt */
    0,				/* size -- bumped by add_menu_item() */
    env_menu_primary_items,
};

static struct menuinfo *envdiagp = &envdiag;

/*
 * FAN Utilities Submenu
 */
static submenu_xtable_t fan_utils_table[] = {
    {"Show Environmental Control Unit registers", (PFT)show_reg, 0,
	0, (type_t(*)())0, 0, (PFT)show_reg,  0},
    {"Show all FANs' speed", (PFT)show_all_fans_speed, 0,
	0, (type_t(*)())0, 0, (PFT)show_all_fans_speed,  0},
    {"Fan Control utility", (PFT)fan_ctl, 0,
	0, (type_t(*)())0, 0, (PFT)fan_ctl, 0},
};

#define FAN_UTILS_TABLE_SIZE (sizeof(fan_utils_table) / sizeof(submenu_xtable_t))

/*
 * Primary & secondary submenu items (filled in from xtable)
 */
static mitem_t fan_utils_primary_items[FAN_UTILS_TABLE_SIZE + MAX_BASE_ITEMS];
static mitem_t fan_utils_secondary_items[FAN_UTILS_TABLE_SIZE + MAX_BASE_ITEMS];

static struct menuinfo fan_utils_diag = {
    "Fan Utilities Submenu",    /* title */
    0,                          /* title string added by init_empty_menu */
    (PFT)menu_show_dflags,      /* shows major flags */
    0,                          /* generic prompt */
    0,                          /* size -- bumped by add_menu_item() */
    fan_utils_primary_items,
};

static struct menuinfo *fan_utils_diagp = &fan_utils_diag;


/*******************************************************************************
 *
 * Function   : build_fan_utils_menu
 * Description:	To build FAN utilities submenu.
 * Inputs     : None
 * Outputs    : None
 *
 *******************************************************************************
 */
void build_fan_utils_menu (void) {
    build_primary_submenu(fan_utils_table, FAN_UTILS_TABLE_SIZE,
			  "FAN Utilities Submenu", &fan_utils_diagp);
    build_secondary_submenu(fan_utils_table, FAN_UTILS_TABLE_SIZE,
			    fan_utils_secondary_items);
    menu(&fan_utils_diag, fan_utils_secondary_items, 0);
}


/**********************************************************************
 *
 * function:	build_env_menu
 *
 * Description:	Build Environmental Control Unit menu.
 *
 * Input:	None.
 *
 * Output:	None.
 *
 **********************************************************************
 */
void
build_env_menu (void)
{
    build_primary_submenu(env_menu_table, ENV_MENU_TABLE_SIZE,
			  "Environmental Control Unit Utility Menu", &envdiagp);
    build_secondary_submenu(env_menu_table, ENV_MENU_TABLE_SIZE,
			    env_menu_secondary_items);
    menu(&envdiag, env_menu_secondary_items, 0);
}

#if 0
/**********************************************************************
 *
 * function:	build_env_test_menu
 *
 * Description:	Build Environmental Control Unit test menu.
 *
 * Input:	submenu - TRUE if invoked from submenu.
 *
 * Output:	None.
 *
 **********************************************************************
 */
void
build_env_test_menu (int submenu)
{
    int rc;

    build_primary_submenu(env_test_menu_table, ENV_TEST_MENU_TAB_SIZE,
                          "Environmental Control Unit Test Menu",
			  &env_test_diagp);
    build_secondary_submenu(env_test_menu_table, ENV_TEST_MENU_TAB_SIZE,
			    env_test_menu_sec_items);
    if (submenu) {
	menu(&env_test_diag, env_test_menu_sec_items, 0);
    } else {
	/* Invoked the tests */
	testname("Environmental Control Unit");

	if (env_ver < ENV_TEST_VER) {
	    cterr('w', 0, "Version %d.%d in the system. Needs 3.0 to fully "
			  "test all functions", env_ver >> ENV_VER_MAJOR_SHIFTS,
			  env_ver & ENV_MCU_REV_MINOR);
	}

	if ((rc = env_reg_test(FALSE)) == PASSED) {
	    if ((rc = env_temp_alert_test(FALSE)) == PASSED) {
		if ((rc = env_ext_alert_test(FALSE)) == PASSED) {
		    rc = env_temp_alert_enable_test(FALSE);
		} /* endof if env_ext_alert_test */
	    } /* endof if env_temp_alert_test */
	} /* endof if env_reg_test */

	if (rc == PASSED) {
	    if ((rc = env_hyst_test(FALSE)) == PASSED) {
		if ((rc = env_fan_en_test(FALSE)) == PASSED) {
			rc = env_wdg_test(FALSE);
		} /* endof if env_fan_en_test */
	    } /* endof if env_hyst_test */
	} /* endof if rc */

	if (rc == PASSED) {
	    if ((rc = env_hys_test(FALSE)) == PASSED) {
		if ((rc = env_time_hys_test(FALSE)) == PASSED) {
		    if ((rc = env_force_fan_test(FALSE)) == PASSED) {
			rc = env_temp_offset_test(FALSE);
		    } /* endof if env_force_fan_test */
		} /* endof if env_time_hys_test */
	    } /* endof if env_hys_test */
	} /* endof if rc */

	prcomplete(testpass, errcount, (char *)0);
    }
}
#endif

/*******************************************************************************
 *
 * Function   : get_env_mcu_i2c_struct
 * Description: To get ENV MCU I2C interface structure.
 * Inputs     : Pointer to save the gotten I2C interface structure
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
static int
get_env_mcu_i2c_struct (n2g_i2c_if_t *env_mcu_i2c)
{
    n2g_i2c_if_t *tmp;

    /* init i2c_if for I2C */
    tmp = (n2g_i2c_if_t *)get_n2g_i2c_if(I2C_CTRL_TWO, I2C_MUX_ZERO,
                                         MB_I2C_ADDR_ENV_MCU);
    if (tmp == NULL) {
        printf("%s: Failed to get Power Sequencer I2C interface structure.\n",
               __FUNCTION__);
        return (FAILED);
    }

    memcpy(env_mcu_i2c, tmp, sizeof(n2g_i2c_if_t));

    return (PASSED);
}


/**********************************************************************
 *
 * Function:	init_env
 *
 * Description:	Initilize Environmental Control Unit
 *
 * Inputs:	None
 *
 * Outputs:	PASSED/FAILED.
 *
 **********************************************************************
 */
int
init_env (void)
{
    n2g_i2c_if_t i2c_if;
    int rc;
    ren_t data;
    char err_buf[ERR_BUF_SIZE];
    
    /* Get ENV MCU I2C interface structure */
    rc = get_env_mcu_i2c_struct(&i2c_if);
    if (rc != PASSED) {
        printf("%s: Failed to get Power Sequencer I2C structure.\n",
              __FUNCTION__);
        return (rc);
    }

    /* Read the Revision register */
    i2c_if.offset = ENV_MCU_REV;
    i2c_if.buf = (char *)&data;
    rc = env_read(&i2c_if);
    
    if (rc == PASSED) {
	      /* Got the version */
	      printf("\n Environmental MCU version %d.%02d.",
		        ((data & ENV_MCU_REV_MAJOR) >> ENV_VER_MAJOR_SHIFTS),
		        data & ENV_MCU_REV_MINOR);
	      
	      if (data & ENV_MCU_REV_DEBUG) {
	          printf(" (Debug)");
	      }
	      
	      if (data & ENV_MCU_REV_MEG) {
	          printf(" (Megatron)");
	      }
	      
	      printf("\n");
	      env_ver = data;

	      /* Enable Fan 5 */
	      /* Read control 1 register first */
	      i2c_if.offset = ENV_MCU_CTL1;
	      rc = env_read(&i2c_if);
	
	      if (rc == PASSED) {
	          data |= ENV_MCU_CTRL1_FAN5_EN; /* Enable fan 5 */

	          /* Write control 1 to enable fan 5 */
	          rc = env_write(&i2c_if);

	              if (rc != PASSED) {
		                sprintf(err_buf, "init_env() Env MCU Control 1 register "
				            "write failed. rc = %#x", rc);
		                rc = FAILED;
                } /* endof if rc control 1 write */
                
	     } else {
	         sprintf(err_buf, "init_env() Env MCU Control 1 register "
			     "read failed. rc = %#x", rc);
	         rc = FAILED;
	     } /* endof if rc control 1 read */
	     
    } else {
	      sprintf(err_buf, "init_env() Env MCU Rev reg read failed. rc = %#x", rc);
	      rc = FAILED;
    } /* endof if rc version read */

    return(rc);

}

/*********************************************************************
 *
 * Function:	show_env_temp
 *
 * Description:	Display environmental control unit temperaturs.
 *
 * Inputs:	err_log - cterr if TRUE. printf if FALSE.
 *		format - display format defined in common.h display_format_t.
 *
 * Output:	PASSED/FAILED
 *
 *********************************************************************
 */
int
show_env_temp (int err_log, int format)
{
    n2g_i2c_if_t i2c_if;
    int rc;
    ren_t data;
    int16_t temp;

    /* Get ENV MCU I2C interface structure */
    rc = get_env_mcu_i2c_struct(&i2c_if);
    if (rc != PASSED) {
        printf("%s: Failed to get Power Sequencer I2C structure.\n",
              __FUNCTION__);
        return (rc);
    }

    i2c_if.buf = (char *)&data;

    /* Read the Inlet 1 Temperature register */
    i2c_if.offset = ENV_MCU_I1T;

    rc = env_read(&i2c_if);
    if (rc == PASSED) {
	temp = (int16_t)data;
	switch(format) {
	case DISPLAY_M2M:
	    printf("IN1TEMP:%dC\n", temp);
	    break;
        case DISPLAY_CTERR:
            cterr_db_print("IN1TEMP:%dC\n", temp);
            break;
	case DISPLAY_HCI:
	default:
	    printf("Inlet 1  Temperature is %d degrees Celsius\n", temp);
	    break;
	} /* endof switch */
	/* Read the Inlet 2 Temperature register */
	i2c_if.offset = ENV_MCU_I2T;
	rc = env_read(&i2c_if);
    }

    if (rc == PASSED) {
	temp = (int16_t)data;
	switch(format) {
	case DISPLAY_M2M:
	    printf("IN2TEMP:%dC\n", temp);
	    break;
        case DISPLAY_CTERR:
            cterr_db_print("IN2TEMP:%dC\n", temp);
            break;
	case DISPLAY_HCI:
	default:
	    printf("Inlet 2  Temperature is %d degrees Celsius\n", temp);
	    break;
	} /* endof switch */
	/* Read the Outlet 1 Temperature register */
	i2c_if.offset = ENV_MCU_O1T;
	rc = env_read(&i2c_if);
    }

    if (rc == PASSED) {
	temp = (int16_t)data;
	switch(format) {
	case DISPLAY_M2M:
	    printf("OUT1TEMP:%dC\n", temp);
	    break;
        case DISPLAY_CTERR:
            cterr_db_print("OUT1TEMP:%dC\n", temp);
            break;	
	case DISPLAY_HCI:
	default:
	    printf("Outlet 1 Temperature is %d degrees Celsius\n", temp);
	    break;
	} /* endof switch */
	/* Read the Outlet 2 Temperature register */
	i2c_if.offset = ENV_MCU_O2T;
	rc = env_read(&i2c_if);
    }

    if (rc == PASSED) {
	temp = (int16_t)data;
	switch(format) {
	case DISPLAY_M2M:
	    printf("OUT2TEMP:%dC\n", temp);
	    break;
        case DISPLAY_CTERR:
            cterr_db_print("OUT2TEMP:%dC\n", temp);
            break;
	case DISPLAY_HCI:
	default:
	    printf("Outlet 2 Temperature is %d degrees Celsius\n", temp);
	    break;
	} /* endof switch */
	/* Display Fans speeds */
	rc = show_fan_speed(&i2c_if, err_log, format);
    }

    if (rc != PASSED) {
	/* Unable to read the regsiter */
	if (err_log == TRUE) {
	    cterr('f', 0, "show_env_temp() Unable to read register at offset "
			  "%#x. rc = %#x", i2c_if.offset, rc);
	} else {
	    printf("\n***** show_env_temp() Unable to read register at offset "
		   "%#x. rc = %#x\n", i2c_if.offset, rc);
	}
    }

    return(rc);
}


/*********************************************************************
 *
 * Function:	show_reg
 *
 * Description:	Display Environment MCU Registers.
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
static int
show_reg (void)
{
    int result = FAILED;
    uint32_t offset;
    uint16_t d16 = 0;
    reg_info_t *reg_ptr;
    
    reg_ptr = &env_mcu_table[0];
    
    printf("\n");

    while (reg_ptr->offset != 0xFFFFFFFF) {
        /* Print all registers */ 
        offset = (reg_ptr->offset);
        /* the byteswap is activiated in env_read. */
        if ((result = read_reg(offset, &d16)) != RC_I2C_OP_OK) {
            cterr('f', 0, "show reg: unable to read regiser");
            return FAILED;
        }

        printf("%-38s (0x%02X), data: 0x%04x\n", reg_ptr->name,
               reg_ptr->offset, d16);
         
        reg_ptr++;

    } 
    
    return(result);

}

/*********************************************************************
 *
 * Function:	alter_reg
 *
 * Description:	Alter Environment MCU Register.
 *
 * Inputs:	None.
 *
 * Outputs:	PASSED/FAILED.
 *
 * Assumptions:
 *
 *********************************************************************
 */
static int alter_reg (void)
{
    uint32_t rc, i, offset;
    reg_info_t *reg_table_p;
    uint16_t tmp_mask, d16 = 0;

    /* Setup I2C API parameter struct */
    printf("\nRegister number:\n");
    for (i = 0, reg_table_p = &env_mcu_table[0];
		i < (sizeof(env_mcu_table) / sizeof(reg_info_t));
		i++, reg_table_p++) {
	if (!(reg_table_p->type & READ_ONLY)) {
	    /* Read writeable */
	    printf("   %02x - %s\n", reg_table_p->offset,
				  reg_table_p->name);
	}
    }

    offset = gethex_answer("Enter the register number:", 0, 0,
			(sizeof(env_mcu_table) / sizeof(reg_info_t)) - 1);

    /* Got the register offset. Check if writeable */
    for (i = 0, reg_table_p = &env_mcu_table[0];
		i < (sizeof(env_mcu_table) / sizeof(reg_info_t));
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

    if ((rc = read_reg(offset,  &d16)) != RC_I2C_OP_OK) {
	cterr('f', 0, "Env MCU read failed with return code 0x%08x", rc);
        return FAILED;
    }
    d16 = gethex_answer("Enter the 16-bit data:", d16, 0, tmp_mask);
    if ((rc = write_reg(offset, d16)) != RC_I2C_OP_OK) {
	cterr('f', 0, "Env MCU write failed with return code 0x%08x", rc);
        return FAILED;
    }

    return(PASSED);
}

/*********************************************************************
 *
 * Function:	pwm_sweep
 *
 * Description:	Log the min and max RPM seen at a certain PWM along with
 *		the current readings.
 *
 * Inputs:	None.
 *
 * Outputs:	PASSE/FAILED.
 *
 *********************************************************************
 */
static int
pwm_sweep (void)
{
    ren_t data;
    ren_t min_pwm, max_pwm, step_pwm, new_pwm_slope;
    int transition_time, log_time;
    ren_t high_speed[NO_OF_FANS], ctrl1, ctrl2, pwm_slope;
    ren_t current_pwm;
    ren_t log_min[NO_OF_FANS], log_max[NO_OF_FANS], log_cur[NO_OF_FANS];
    uint8_t fan_mask;
    n2g_i2c_if_t i2c_if;
    uint32_t rc;
    int i;
    int log_loop, fan_count;
    int fans;

    /* Setup I2C API parameter struct */
    /* Get ENV MCU I2C interface structure */
    rc = get_env_mcu_i2c_struct(&i2c_if);
    if (rc != PASSED) {
        printf("%s: Failed to get Power Sequencer I2C structure.\n",
              __FUNCTION__);
        return (rc);
    }

    fans = get_no_of_fans();

    /* Get the fan(s) number */
    fan_mask = gethex_answer("\nEnter Fan mask (01 - Fan 1, ... F - all) - ",
					FAN_MASK, 0, FAN_MASK);

    /* Get the starting PWM */
    min_pwm = getdec_answer("Enter minimum PWM - ", MIN_PWM, MIN_PWM,
                                                    MAX_PWM - 1);

    /* User entries */
    max_pwm = getdec_answer("Enter maximum PWM - ", MAX_PWM, min_pwm,
						    MAX_PWM);
    step_pwm = getdec_answer("Enter stepping PWM - ", 1, 1,
						      max_pwm - min_pwm);
    transition_time = getdec_answer("Enter transition time in seconds - ", 5,
						1, 10);
    log_time = getdec_answer("Enter number of samples per step - ",
						15, 1, 10000);

    new_pwm_slope = getdec_answer("Enter the new Fan PWM slope - ", 100,
						1, 100);

    current_pwm = min_pwm;

    /* Save registers that will be modified */
    i2c_if.buf = (char *)&ctrl1;
    i2c_if.offset = ENV_MCU_CTL1;     
    if ((rc = env_read(&i2c_if)) != PASSED) {
	/* Unable to read control 1 register */
	cterr('f', 0, "pwm_msleep() Unable to read control 1 register.");
	return(FAILED);
    }


    i2c_if.buf = (char *)&ctrl2;
    i2c_if.offset = ENV_MCU_CTL2;  
    if ((rc = env_read(&i2c_if)) != PASSED) {
	/* Unable to read control 2 register */
	cterr('f', 0, "pwm_msleep() Unable to read control 2 register.");
	return(FAILED);
    }

    i2c_if.buf = (char *)&pwm_slope;
    i2c_if.offset = ENV_MCU_PWS;           
    if ((rc = env_read(&i2c_if)) != PASSED) {
	/* Unable to read fan PWM slope register */
	cterr('f', 0, "pwm_msleep() Unable to read PWM slope register.");
	return(FAILED);
    }

    /* Setup of control1 to be used later. Also set the Fan high speed */
    data = (ctrl1 | ENV_MCU_CTRL1_PWM | ENV_MCU_CTRL1_HI_FAN) & (~
		 (ENV_MCU_CTRL1_FAN5_EN | ENV_MCU_CTRL1_FAN4_EN |
		  ENV_MCU_CTRL1_FAN3_EN | ENV_MCU_CTRL1_FAN2_EN |
		  ENV_MCU_CTRL1_FAN1_EN));

    for (i = 0, i2c_if.offset = (unsigned int)ENV_MCU_F1H, fan_count = 0; 
    i < fans;	i++, i2c_if.offset += (ENV_MCU_F2H - ENV_MCU_F1H)) {
	if (fan_mask & (0x1 << i)) {
	    /* Monitored fan */
	    fan_count++;
	    i2c_if.buf = (char *)&high_speed[i];
	    if ((rc = env_read(&i2c_if)) != PASSED) {
		/* Unable to read high speed */
		cterr('f', 0, "pwm_msleep() Unable to read Fan %d high speed.", i);
		return(FAILED);
	    } /* endof if i2c_read */

	    data |= (ENV_MCU_CTRL1_FAN1_EN << i);	/* Enable the fan */
	} /* endof if fan_mask */
    } /* endof for */

    /* Setup new control 1 register with fans disabled, and using PWM */
    i2c_if.buf = (char *)&data;
    i2c_if.offset = ENV_MCU_CTL1;
    if ((rc = env_write(&i2c_if)) != PASSED) {
	/* Unable to write control 1 register */
	cterr('f', 0, "pwm_msleep() Unable to write control 1 reg. rc = %#x",
			rc);
	return(FAILED);
    }

    /* Set new fan PWM slope register */
    if (new_pwm_slope != pwm_slope) {
	i2c_if.buf = (char *)&new_pwm_slope;
	i2c_if.offset = ENV_MCU_PWS;
	if ((rc = env_write(&i2c_if)) != PASSED) {
	    /* Unable to write Fan PWM slope register */
	    cterr('f', 0, "pwm_msleep() Unable to write pwm slope reg. rc = %#x",
				rc);
	    return(FAILED);
	}
    }

    while (current_pwm <= max_pwm) {
	/* Set the new high speed */
	for (i = 0, i2c_if.offset = ENV_MCU_F1H; i < fans;
                i++, i2c_if.offset += (ENV_MCU_F2H - ENV_MCU_F1H)) {
	    if (fan_mask & (0x1 << i)) {
		/* Monitored fan */
		i2c_if.buf = (char *)&current_pwm;

		if ((rc = env_write(&i2c_if)) != PASSED) {
		    /* Unable to write High speed register */
		    cterr('f', 0, "pwm_msleep() Unable to write Fan %d "
				  "high speed register. rc = 0x%08x", i, rc);
		    return(FAILED);
		} /* endof if i2c_write */

	    } /* endof if fan_mask */
	} /* endof for */

	/* Wait for the transition time */
	for (i = 0; i < transition_time; i++) {
	    msleep(MS_PER_SECOND);	/* Wait for one second */
	} /* endof for */

	if (current_pwm == min_pwm) {
	    /* First ramp, may require extra time */
	    if (getc_answer("High speed all set. Ready to log?", "yn", 'y') ==
				'n') {
		/* out */
		break;
	    }
	    printf("\n PWM ");
	    for (i = 0, fan_count = 0; i < fans; i++) {
		if (fan_mask & (0x1 << i)) {
		    /* Monitored fan */
		    printf("         Fan %d         ", i + 1);
		    fan_count++;
		} /* endof if fan_mask */
	    } /* endof for */

	    printf("\n     | ");
	    for (i = 0; i < fan_count; i++) {
		printf("  cur.  min.  max.   | ");
	    }
	    printf("\n");
	} /* endof if current_pwm */

	/* Log the RPMs */
	for (i = 0; i < fans; i++) {
	    if (fan_mask & (0x1 << i)) {
		/* Monitored fan */
		log_min[i] = MAX_RPM;
		log_max[i] = 0;
	    } /* endof if fan_mask */
	} /* endof for i */

	i2c_if.buf = (char *)&data;	/* Setup the buffer for read */
	for (log_loop = 0; log_loop < log_time; log_loop++) {
	    for (i = 0, i2c_if.offset = ENV_MCU_F1T; i < fans;
			i++, i2c_if.offset++) {
		if ((0x01 << i) & fan_mask) {
		    /* Monitored fan */
		    /* Read RPM */
		    if ((rc = env_read(&i2c_if)) != PASSED) {
			/* Unable to read tach rpm */
			cterr('f', 0, "pwm_msleep() Unable to read Fan %d RPM.", i);
			return(FAILED);
		    } /* endof if env_read */

		    /* Log the min and max RPM */
		    if (data > log_max[i]) {
			log_max[i] = data;
		    }
		    if (data < log_min[i]) {
			log_min[i] = data;
		    }
		    log_cur[i] = data;
		} /* endof if fan_mask */
	    } /* endof for fans */
	} /* endof for log_loop */

	/* End of the logging. Print the result */
	printf("%4d | ", current_pwm);
	for (i = 0; i < fans; i++) {
	    if (fan_mask & (0x01 << i)) {
		/* Monitored fan */
		printf("%6d %6d %6d | ", log_cur[i], log_min[i], log_max[i]);
	    }
	}
	printf("\n");
	current_pwm += step_pwm;
    } /* endof while */

    /* Restoring registers */
    i2c_if.buf = (char *)&ctrl1;
    i2c_if.offset = ENV_MCU_CTL1;

    if ((rc = env_write(&i2c_if)) != PASSED) {
	/* Unable to write control 1 register */
	cterr('f', 0, "pwm_msleep() Unable to restore control 1 reg. rc = %#x", rc);
	return(FAILED);
    }

    for (i = 0, i2c_if.offset = ENV_MCU_F1H, fan_count = 0; i < fans;
		i++, i2c_if.offset += (ENV_MCU_F2H - ENV_MCU_F1H)) {
	if (fan_mask & (0x1 << i)) {
	    /* Monitored fan */
	    i2c_if.buf = (char *)&high_speed[i];
	    if ((rc = env_write(&i2c_if)) != PASSED) {
		/* Unable to write high speed */
		cterr('f', 0, "pwm_msleep() Unable to restore Fan %d high speed."
			     "rc = 0x%08x", i, rc);
		return(FAILED);
	    } /* endof if i2c_write */
	} /* endof if fan_mask */
    } /* endof for */

    i2c_if.buf = (char *)&pwm_slope;
    i2c_if.offset = ENV_MCU_PWS;
    rc = env_write(&i2c_if);
    if (rc != PASSED) {
	/* Unable to write Fan PWM slope register */
	cterr('f', 0, "pwm_msleep() Unable to restore PWM slope. rc = 0x%08x",
					rc);
    }

    return(rc);
}

/*********************************************************************
 *
 * Function:	fan_ctl
 *
 * Description:	Fan control utility. Mainly used for mechanical team.
 *
 * Inputs:	None.
 *
 * Outputs:	PASSE/FAILED.
 *
 *********************************************************************
 */
static int
fan_ctl (void)
{
    n2g_i2c_if_t i2c_if;
    int i, fans, rc = PASSED;
    ren_t data, ctrl1, ctrl2, speed;
    uint8_t fan_mask, speed_mode;
    char err_buf[ERR_BUF_SIZE];

    /* Get number of fans */
    fans = get_no_of_fans();

    /* Get the fan(s) number */
    fan_mask = gethex_answer("\nEnter Fan mask (01 - Fan 1, ... F - all) - ",
			      FAN_MASK, 0, FAN_MASK);
    speed_mode = gethex_answer("Enter Speed Mode (0 - PWM, 1 - RPM) - ",
				0, 0, 1);
    switch(speed_mode) {
    case 0:
	/* PWM */
	speed = getdec_answer("Enter Speed in PWM (0 - 100) - ",
			       50, MIN_PWM, MAX_PWM);
	break;
    default:
	/* RPM */
	speed = getdec_answer("Enter Speed RPM (0 - 15000) - ",
			       3500, 0, 15000);
	break;
    }

    /* Get ENV MCU I2C interface structure */
    rc = get_env_mcu_i2c_struct(&i2c_if);
    if (rc != PASSED) {
        printf("%s: Failed to get Power Sequencer I2C structure.\n",
              __FUNCTION__);
        return (rc);
    }
    
    if (rc == PASSED) {
	/* Read Control 1 register */
	i2c_if.offset = ENV_MCU_CTL1;
	i2c_if.buf = (char *)&ctrl1;
	rc = env_read(&i2c_if);
	if (rc == PASSED) {
	    if (speed_mode == 1) {
		/* Read Control 2 register for RPM Mode */
		i2c_if.offset = ENV_MCU_CTL2;
		i2c_if.buf = (char *)&ctrl2;
		rc = env_read(&i2c_if);
		if (rc != PASSED) {
		    sprintf(err_buf, "fan_ctl() Unable to read Control 2 "
				     "register. rc = %#x", rc);
		    rc = FAILED;
		} /* endof if rc */
	    } /* endof if speed_mode */
	} else {
	    sprintf(err_buf, "fan_ctl() Unable to read Control 1 register "
			     "rc = %#x", rc);
	    rc = FAILED;
	} /* endof if rc */
    } else {
	sprintf(err_buf, "fan_ctl() Device attach failed. rc = %#x", rc);
	rc = FAILED;
    }

    if (rc == PASSED) {
	/* Got both control registers. Ready to write */
	i2c_if.buf = (char *)&data;
	/* Set manual control and PWM */ 
	data = ctrl1 | (ENV_MCU_CTRL1_PWM | ENV_MCU_CTRL1_HI_FAN);
	if (speed_mode == 1) {
	    /* RPM mode */
	    data &= (~ENV_MCU_CTRL1_PWM);
	}
	i2c_if.offset = ENV_MCU_CTL1;
	rc = env_write(&i2c_if);
	if (rc == PASSED) {
	    if (speed_mode == 1) {
		/* RPM also needs to update Control 2 register */
		i2c_if.offset = ENV_MCU_CTL2;
		data = ctrl2 | ENV_MCU_CTRL2_PWM_D;
		rc = env_write(&i2c_if);
		if (rc != PASSED) {
		    sprintf(err_buf, "fan_ctl() Unable to write Control 2 "
				     "register. rc = %#x", rc);
		    rc = FAILED;
		}
	    }
	} else {
	    sprintf(err_buf, "fan_ctl() Unable to write Control 1 register "
			     "rc = %#x", rc);
	    rc = FAILED;
	}
    }

    if (rc == PASSED) {
	/* Ready to set the speed */
	i2c_if.buf = (char *)&speed;
	for (i = 0, i2c_if.offset = ENV_MCU_F1H; i < fans;
	     i++, i2c_if.offset += (ENV_MCU_F2H - ENV_MCU_F1H)) {
	    if ((0x1 << i) & fan_mask) {
		rc = env_write(&i2c_if);
		if (rc != PASSED) {
		    /* Unable to write the speed */
		    sprintf(err_buf, "fan_ctl() Unable to write fan %d speed. "
				     "rc = %#x", i + 1, rc);
		    rc = FAILED;
		    break;
		} /* endof if rc */
	    } /* endof if fan_mask */
	} /* endof for */
    }

    /* Display Fans speeds */
    rc = show_fan_speed(&i2c_if, FALSE, 2);

    return(rc);
}


/*********************************************************************
 *
 * Function:	env_read
 *
 * Description:	Local Read Environment MCU Register.
 *		Env MCU I2C read has 2 I2C operations. The I2C write with
 *		the register offset. Then wait for the REN_I2C_PROC_TIME
 *		milliseconds to allow the Env MCU firmware to setup
 *		the data of the requested register. Then the I2C read will
 *		return the data.
 *
 * Inputs:	i2c_if - pointer to the I2C API struct.
 *
 * Outputs:	PASSED/FAILED.
 *
 * Assumptions:
 *
 *********************************************************************
 */
static int
env_read (n2g_i2c_if_t *i2c_if)
{
    uint32_t rc;
    uint16_t data = 0;

    if (!i2c_if->buf) {
        assert(!"env_read: buf is null");
    }
    
    rc = n2g_i2c_read(i2c_if);
    if (rc != RC_I2C_OP_OK) {
	/* Unable to read data */
	cterr('f', 0, "%s: Unable to read. rc = 0x%08x", __FUNCTION__, rc);
	return(FAILED);
    }

    data = DSWAP2(*(uint16_t *)i2c_if->buf);
    memcpy(i2c_if->buf, &data, sizeof(uint16_t));

    msleep(REN_I2C_PROC_TIME);	/* Env MCU I2C cycle time */

    return(PASSED);
}

/*********************************************************************
 *
 * Function:	env_write
 *
 * Description:	Write Environment MCU Register.
 *		Wait REN_I2C_PROC_TIME milliseconds after the successful
 *		write operations.
 *
 * Inputs:	i2c_if - pointer to the I2C API struct.
 *
 * Outputs:	n2g_i2c_write() return code.
 *
 * Assumptions:
 *
 *********************************************************************
 */
static int
env_write (n2g_i2c_if_t *i2c_if)
{
    uint32_t rc;
    uint16_t data = 0;
    n2g_i2c_if_t new_i2c_if;
    
    if (!i2c_if->buf) {
        assert(!"env_write: buf is null");
    }

    memcpy(&new_i2c_if, i2c_if, sizeof(n2g_i2c_if_t));

    data = DSWAP2(*(uint16_t *)i2c_if->buf);

    new_i2c_if.buf = (char *)&data;
    
    rc = n2g_i2c_write(&new_i2c_if);
    
    if (rc != RC_I2C_OP_OK) {
	msleep(REN_I2C_PROC_TIME);	/* Env MCU I2C cycle time */
        return (FAILED);
    }

    msleep(REN_I2C_PROC_TIME);	/* Env MCU I2C cycle time */

    return (PASSED);
}

/*********************************************************************
 *
 * Function:	get_no_of_fans
 *
 * Description:	Returns number of fans.
 *
 * Inputs:	None
 *
 * Outputs:	Number of fans.
 *
 * Assumptions:
 *
 *********************************************************************
 */
static int
get_no_of_fans (void)
{
    /* Overlord: 4 Fans; Juno: 3 Fans */
    if (is_juno()) {
        return(3);
    } else {
        return(4);
    }
}

/*********************************************************************
 *
 * Function:	show_fan_speed
 *
 * Description:	Displan fans tachometer registers .
 *
 * Inputs:	i2c_if - points to the I2C interface struct for read.
 *		err_log - TRUE to cterr() FALSE to printf()
 *		format - display format defined in common.h display_format_t
 *
 * Outputs:	PASSED/FAILED
 *
 * Assumptions:
 *
 *********************************************************************
 */
static int
show_fan_speed (n2g_i2c_if_t *i2c_if, int err_log, int format)
{
    n2g_i2c_if_t new_i2c_if;
    int i, fans, rc = PASSED;
    ren_t data, ctrl, fan_en_mask;

    fans = get_no_of_fans();

    memcpy(&new_i2c_if, i2c_if, sizeof(n2g_i2c_if_t));
    
    new_i2c_if.buf = (char *)&ctrl;

    /* Read the control 1 register for Fan enable */
    new_i2c_if.offset = ENV_MCU_CTL1;
    rc = env_read(&new_i2c_if);
    if (rc != PASSED) {
	if (err_log == TRUE) {
	    cterr('f', 0, "show_fan_speed() Cannot read Control 1 register. "
			  "rc = %#x", rc);
	} else {
	    printf("\n ***** show_fan_speed() Cannot read Control 1 register. "
		   "rc = %#x\n", rc);
	}
	return(FAILED);
    }	   

    /* Ready to read Fan Tachometer */
    new_i2c_if.buf = (char *)&data;

    for (i = 0, new_i2c_if.offset = ENV_MCU_F1T,
		fan_en_mask = ENV_MCU_CTRL1_FAN1_EN;
		i < fans;
		i++, new_i2c_if.offset++, fan_en_mask <<= 1) {
	/* Read Tach RPM */
	if ((rc = env_read(&new_i2c_if)) != PASSED) {
	    if (err_log == TRUE) {
		cterr('f', 0, "show_fan_speed()Unable to read Fan %d "
			      " Tachometer. rc = %#x", i + 1, rc);
	    } else {
		printf("\n ***** show_fan_speed() Unable to read Fan %d "
		       "Tachometer. rc = %#x", i + 1, rc);
	    }
	    return(FAILED);
	}

	switch(format) {
    case DISPLAY_CTERR:
	    cterr_db_print("FAN%dSTAT:%s\n", i + 1, 
                      (ctrl & fan_en_mask) ? "Enabled" :  "Disabled");
	    cterr_db_print("FAN%dSPD:%d\n", i + 1, data);
	    break;
        
	case DISPLAY_M2M:
	    printf("FAN%dSTAT:%s\n", i + 1, (ctrl & fan_en_mask) ? "Enabled" :
								   "Disabled");
	    printf("FAN%dSPD:%d\n", i + 1, data);
	    break;
	case DISPLAY_HCI:
	default:
	    if (ctrl & fan_en_mask) {
		printf("Fan %d is enabled. Running at %d RPM\n", i + 1, data);
	    } else {
		if (err_log == TRUE) {
		    cterr('f', 0, "Fan %d is disabled", i + 1);
		} else {
		    printf("   *** Fan %d is disabled ***\n", i + 1);
		} /* endof if err_log */
	    } /* endof if ctrl */
	    break;
	} /* endof switch */
    } /* endof for */

    printf("\n");

    return(rc);
}

/*********************************************************************
 *
 * Function:	env_reg_test
 *
 * Description:	Simple Register Read/Write test. Refer to "Diagnostics
 *		Tests" in EDCS-534569, Rev 6 and later.
 *
 * Inputs:	submenu - TRUE if submenu invoked.
 *
 * Outputs:	PASSED/FAILED
 *
 * Assumptions:
 *
 *********************************************************************
 */
static int
env_reg_test (int submenu)
{
    n2g_i2c_if_t i2c_if;
    env_reg_test_t *reg_tbl_p;
    uint32_t rc = FAILED;
    ren_t original_data, test_data, pat_1, pat_2, exp_pat_1, exp_pat_2;
    char err_buf[ERR_BUF_SIZE];

    if (submenu == TRUE) {
	testname("Simple Register Read/Write");
    } else {
	prpass(testpass, "Simple Register Read/Write Test");
    }

    /* Setup I2C API parameter struct */
    /* Get ENV MCU I2C interface structure */
    rc = get_env_mcu_i2c_struct(&i2c_if);
    if (rc != PASSED) {
        printf("%s: Failed to get Power Sequencer I2C structure.\n",
              __FUNCTION__);
        return (rc);
    }
    
    /* Ready to test registers */
    reg_tbl_p = &simple_reg_test_tbl[0];

    while ((reg_tbl_p->option != ENV_REG_TEST_INVALID) && (rc == PASSED)) {
	/* Check for version */
	if (env_ver < ENV_TEST_VER) {
	    /* Older version needs to bypass some registers tests */
	    if (((reg_tbl_p->option == ENV_REG_TEST_OP3) &&
		(reg_tbl_p->offset == ENV_MCU_AST)) ||
		(reg_tbl_p->option == ENV_REG_TEST_OP4)) {
		reg_tbl_p++;
		continue;
	    }
	}

	i2c_if.offset = reg_tbl_p->offset;
	prpass(testpass, "Offset %#x", reg_tbl_p->offset);
	/* Save the original value */
	i2c_if.buf = (char *)&original_data;
	rc = env_read(&i2c_if);
	if (rc != PASSED) {
	    sprintf(err_buf, "env_reg_test() Unable to read original register "
			      "@ %#x. rc = %#x", i2c_if.offset, rc);
	    rc = FAILED;
	    break;
	}

	i2c_if.buf = (char *)&test_data;
	switch (reg_tbl_p->option) {
	case ENV_REG_TEST_OP1:
	    /* Write 0 */
	    test_data = ENV_REG_TEST_OPTION1_PATTERN;

	    exp_pat_1 = pat_1 = exp_pat_2 = pat_2 = test_data;

	    rc = reg_wr_rd_test(&i2c_if);
	    if (rc != ENV_WR_RD_PASSED) {
		if (rc == ENV_WR_FAILED) {
		    /* Write failed */
		    sprintf(err_buf, "env_reg_test() Unable to write @ %#x. "
				     "rc = %#x", i2c_if.offset, i2c_if.err_no);
		} else {
		    /* Read ie rc == ENV_RD_FAILED */
		    sprintf(err_buf, "env_reg_test() Unable to read %#x. "
				     "rc = %#x", i2c_if.offset, i2c_if.err_no);
		}
		rc = FAILED;
	    }

	    if (original_data != test_data) {
		sprintf(err_buf, "env_reg_test() miscompare @ %#x. Expect %#x. "
				 "Read %#x", reg_tbl_p->offset,
				 original_data, test_data);
		rc = FAILED;
	    }

	    break;
	case ENV_REG_TEST_OP2:
	    exp_pat_1 = pat_1 = ENV_REG_TEST_OPTION2_PATTERN1;
	    exp_pat_2 = pat_2 = ENV_REG_TEST_OPTION2_PATTERN2;
	    break;
	case ENV_REG_TEST_OP3:
	    exp_pat_1 = pat_1 = ENV_REG_TEST_OPTION3_PATTERN1;
	    pat_2 = ENV_REG_TEST_OPTION3_PATTERN2;
	    if (reg_tbl_p->offset == ENV_MCU_AST) {
		exp_pat_2 = pat_2 & 0xFF;
	    } else {
		exp_pat_2 = pat_2;
	    }
	    break;
	case ENV_REG_TEST_OP4:
	    pat_1 = ENV_REG_TEST_OPTION4_PATTERN1;
	    exp_pat_1 = ENV_REG_TEST_OPTION4_PATTERN1_EXP;
	    exp_pat_2 = pat_2 = ENV_REG_TEST_OPTION4_PATTERN2;
	    break;
	default:
	    assert(!"env_reg_test() Invalid option");
	    exp_pat_1 = pat_1 = exp_pat_2 = pat_2 = 0;
	    rc = FAILED;
	    break;
	} /* endof switch */

	if (rc != PASSED) {
	    break;
	}

	switch (reg_tbl_p->option) {
	case ENV_REG_TEST_OP2:
	case ENV_REG_TEST_OP3:
	case ENV_REG_TEST_OP4:
	    /* Test pattern 1 */
	    test_data = pat_1;
	    rc = reg_wr_rd_test(&i2c_if);
	    if (rc != ENV_WR_RD_PASSED) {
		if (rc == ENV_WR_FAILED) {
		    /* Write failed */
		    sprintf(err_buf, "env_reg_test() Unable to write %#x @ "
				     "%#x. rc = %#x",
				     pat_1, i2c_if.offset, i2c_if.err_no);
		} else {
		    /* Read ie rc == ENV_RD_FAILED */
		    sprintf(err_buf, "env_reg_test() Unable to read @ %#x. "
				     "rc = %#x", i2c_if.offset, i2c_if.err_no);
		}
		rc = FAILED;
	    }

	    /* Check the test result */
	    if ((rc == PASSED) && (test_data != exp_pat_1)) {
#ifndef ENV_REG_DEBUG
		sprintf(err_buf, "env_reg_test() pattern 1 miscompare @ %#x. "
				 "Expect %#x. Read %#x",  reg_tbl_p->offset,
				 exp_pat_1, test_data);
#else /* ENV_REG_DEBUG */
		printf("\n****env_reg_test() pattern 1 miscompare @ %#x. "
		       "Expect %#x. Read %#x\n", reg_tbl_p->offset,
		       exp_pat_1, test_data);
		printf("Re-read the register\n");
/*		msleep(ENV_REG_TEST_DELAY);   * delay 510 ms */
		rc = env_read(&i2c_if);

		if (rc != PASSED) {
		    printf("re-read failed @ %#x size %d\n", i2c_if.offset,
		            i2c_if.size);
		} else {
		    printf("read read data = %#x @ %#x size %d. ", test_data,
			   i2c_if.offset, i2c_if.size);
		    printf("buf @ %#x, test_data @ %#x i2c_if @ %#x\n",
			   i2c_if.buf, &test_data, &i2c_if);
		}

#endif /* ENV_REG_DEBUG */
		rc = FAILED;
	    }

	    if (rc != PASSED) {
		break;
	    }

	    /* Test pattern 2 */
	    test_data = pat_2;
	    rc = reg_wr_rd_test(&i2c_if);
	    if (rc != ENV_WR_RD_PASSED) {
		if (rc == ENV_WR_FAILED) {
		    /* Write failed */
		    sprintf(err_buf, "env_reg_test() Unable to write %#x @ "
				     "%#x. rc = %#x",
				     pat_2, i2c_if.offset, i2c_if.err_no);
		} else {
		    /* Read ie rc == ENV_RD_FAILED */
		    sprintf(err_buf, "env_reg_test() Unable to read @ %#x. "
				     "rc = %#x", i2c_if.offset, i2c_if.err_no);
		}
		rc = FAILED;
	    }

	    /* Check the test result */
	    if ((rc == PASSED) && (test_data != exp_pat_2)) {
		sprintf(err_buf, "env_reg_test() pattern 2 miscompare @ %#x. "
				 "Expect %#x. Read %#x",  reg_tbl_p->offset,
				 exp_pat_2, test_data);
		rc = FAILED;
	    }

	    if (rc != PASSED) {
		break;
	    }

	    /* Restore the orginal value */
	    i2c_if.buf = (char *)&original_data;
	    rc = env_write(&i2c_if);

	    if (rc != PASSED) {
		sprintf(err_buf, "env_reg_test() Unable to restore the "
				 "original value @ %#x. rc = %#x",
				 i2c_if.offset, rc);
		rc = FAILED;
	    }
	    msleep(REN_I2C_PROC_TIME);
	    break;
	default:
	    break;
	} /* endof switch */

	reg_tbl_p++;
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
 * Function:	reg_wr_rd_test
 *
 * Description:	Write to a given register then read it back.
 *		Do not use cterr(). The caller will provide more info
 *		for the cterr() and cleanup.
 *
 * Inputs:	i2c_if - Points to the I2C interface struct with register
 *			 size etc all setup.
 *
 * Outputs:	PASSED or error code
 *
 * Assumptions:
 *
 *********************************************************************
 */
static int
reg_wr_rd_test (n2g_i2c_if_t *i2c_if)
{

    i2c_if->err_no = env_write(i2c_if);

    if (i2c_if->err_no != PASSED) {
	return(ENV_WR_FAILED);
    }

    /* Read it back */
    i2c_if->err_no = env_read(i2c_if);

    msleep(REN_I2C_PROC_TIME); /* Env MCU I2C cycle time */

    if (i2c_if->err_no != PASSED) {
	return(ENV_RD_FAILED);
    } else {
	return(ENV_WR_RD_PASSED);
    }
}

/*********************************************************************
 *
 * Function:	env_alert_temp_test
 *
 * Description:	Temperature Alert test with alert enabled or disabled.
 *		Refer to "Diagnostics  Tests" in EDCS-534569, Rev 6 and later.
 *
 * Inputs:	c_i2c_if - Points to caller's I2C interface struct.
 *		enable - ENABLE or DISABLE alert.
 *		err_buf - Points to the error buffer.
 *
 * Outputs:	PASSED/FAILED
 *
 * Assumptions: The err_buf is large enough for both the pre-text and added
 *		new text.
 *
 *********************************************************************
 */
static int
env_alert_temp_test (n2g_i2c_if_t *c_i2c_if, int enable, char *err_buf)
{
    n2g_i2c_if_t i2c_if;
    env_temp_alert_test_t *test_tbl_p;
    int rc;
    ren_t cur_temp, alert_temp, test_alert, max_alert, stat1, stat2;

    if (env_ver < ENV_TEST_VER) {
	cterr('w', 0, "Test not supported in Version %d.%d",
		      env_ver >> ENV_VER_MAJOR_SHIFTS,
		      env_ver & ENV_MCU_REV_MINOR);
	return(PASSED);
    }

    /* Copy the I2C interface struct */
    i2c_if = *c_i2c_if;

    /* Ready for the temperature alert tests */
    test_tbl_p = &env_temp_alert_test_tbl[0];

    max_alert = ENV_ALERT_MAX_TEMP;	/* The maximum Alert temperature */

    while (test_tbl_p->current_t) {
	/* Read current temperature */
	i2c_if.offset = test_tbl_p->current_t;
	i2c_if.buf = (char *)&cur_temp;
	rc = env_read(&i2c_if);
	if (rc != PASSED) {
	    sprintf(err_buf, "Read Current temperature @ %#x failed with rc = "
			     "%#x", i2c_if.offset, rc);
	    return(FAILED);
	}

	test_alert = cur_temp - ENV_ALERT_TEST_DELTA;

	/* Read and save Alert temperature */
	i2c_if.offset = test_tbl_p->alert_t;
	i2c_if.buf = (char *)&alert_temp;
	rc = env_read(&i2c_if);
	if (rc != PASSED) {
	    sprintf(err_buf, "Read Alert temperature @ %#x failed with rc = "
			     "%#x", i2c_if.offset, rc);
	    return(FAILED);
	}

	/* Set maximum alert temperature so that the alert bit should not
	 * be set when we test in the EDVT chamber
	 */
	i2c_if.offset = test_tbl_p->alert_t;
	i2c_if.buf = (char *)&max_alert;
	rc = env_write(&i2c_if);
	if (rc != PASSED) {
	    sprintf(err_buf, "env_alert_temp_test() Write max alert failed "
			     "rc = %#x", rc);
	    return(FAILED);
	}

	rc = env_clear_stat2(&i2c_if, &stat2, err_buf, NULL);
	if (rc != PASSED) {
	    /* Unable to clear status 2 register */
	    return(FAILED);
	}

	msleep(REN_I2C_PROC_TIME);	/* Wait for MCU to clear it out */

	/* Get Status registers */
	rc = env_get_status(&i2c_if, &stat1, &stat2, err_buf, NULL);
	if (rc != PASSED) {
	    return(FAILED);
	}

	/* Check only TEMPERATURE_ALERT bit. aggregate and fan alert may be
	 * on due to other alert condition.
	 */
	if ((stat1 & test_tbl_p->stat1_mask)) {
	    sprintf(err_buf, "env_alert_temp_test() Stat 1 = %#x. Expect mask "
			     " = %#x off", stat1, test_tbl_p->stat1_mask);
	    return(FAILED);
	}

	/* Set Alert temperature to lower */
	i2c_if.offset = test_tbl_p->alert_t;
	i2c_if.buf = (char *)&test_alert;
	rc = env_write(&i2c_if);
	if (rc != PASSED) {
	    sprintf(err_buf, "Write to Alert temperature @ %#x with %#x "
			     "failed with rc = %#x",
			     i2c_if.offset, test_alert, rc);
	    return(FAILED);
	}

	/* Check status */
	rc = env_get_status(&i2c_if, &stat1, &stat2, err_buf, NULL);
	if (rc != PASSED) {
	    return(FAILED);
	}

	/* Check the mask bit */
	if ((stat1 & test_tbl_p->stat1_mask) != test_tbl_p->stat1_mask) {
	    sprintf(err_buf, "Status 1 = %#x. Expect mask = %#x @ %#x",
			     stat1, test_tbl_p->stat1_mask,
			     test_tbl_p->alert_t);
	    return(FAILED);
	}

	if (enable == ENABLE) {
	    /* Check the aggregate bit */
	    if ((stat1 & ENV_MCU_STAT1_AG_TEMP) == 0) {
		sprintf(err_buf, "Status 1 = %#x. Aggregate temperature alert "
				 "not set @ %#x", stat1, test_tbl_p->alert_t);
		return(FAILED);
	    } /* endof if stat1 */
	    /* Check status 2 */
	    if ((stat2 & ENV_MCU_STAT2_FAN_ST_M) != ENV_MCU_STAT2_FAN_FULL) {
		sprintf(err_buf, "Status 2 = %#x. Fans in alert state not set "
				 "@ %#x", stat2, test_tbl_p->alert_t);
		return(FAILED);
	    } /* endof stat2 */
	} else { /* DISABLE */
	    if (stat1 & ENV_MCU_STAT1_AG_TEMP) {
		sprintf(err_buf, "Status 1 = %#x. Aggregate temperature alert "
				 "set unexpected @ %#x", stat1,
				 test_tbl_p->alert_t);
		return(FAILED);
	    } /* endof if stat1 */
	    /* Check status 2 */
	    if ((stat2 & ENV_MCU_STAT2_FAN_ST_M) == ENV_MCU_STAT2_FAN_FULL) {
		sprintf(err_buf, "Status 2 = %#x. Fans in alert state @ %#x",
				 stat2, test_tbl_p->alert_t);
		return(FAILED);
	    } /* endof if stat2 */
	} /* endof if enable */

	/* Restore Alert temperature */
	i2c_if.offset = test_tbl_p->alert_t;
	i2c_if.buf = (char *)&alert_temp;
	rc = env_write(&i2c_if);
	if (rc != PASSED) {
	    sprintf(err_buf, "Unable to restore Alert temperature @ %#x with "
			     "%#x. rc = %#x", i2c_if.offset, alert_temp, rc);
	    return(FAILED);
	}

	/* Clear Status 2 */
	rc = env_clear_stat2(&i2c_if, &stat2, &err_buf[0], NULL);

	if (rc != PASSED) {
	    return(FAILED);
	}

	test_tbl_p++;	/* Update to the next Alert temperature test */
    } /* endof while */

    return(PASSED);
}

/*********************************************************************
 *
 * Function: ext_alert_test
 *
 * Description:	External Temperature Alert test. Refer to "Diagnostics
 *		Tests" in EDCS-534569, Rev 6 and later.
 *
 * Inputs:	submenu - TRUE if submenu invoked.
 *
 * Outputs:	PASSED/FAILED
 *
 * Assumptions:
 *
 *********************************************************************
 */
static int
env_ext_alert_test (int submenu)
{
    printf("\n\n**plaform_env.c not supported line %d**\n\n", __LINE__);
    return(FAILED);
#if 0
    dev_ren_object_t mcu;
    n2g_i2c_if_t i2c_if;
    dev_object_t *dev = (dev_object_t *)&mcu;
    uint32_t rc = FAILED;
    ren_t stat1, stat2;
    char err_buf[ERR_BUF_SIZE];

    if (submenu == TRUE) {
	testname("External Temperature Alert");
    } else {
	prpass(testpass, "External Temperature Alert Test");
    }

    if (env_ver < ENV_TEST_VER) {
	cterr('w', 0, "External Temperature Alert Test not supported in "
		      "Version %d.%d", env_ver >> ENV_VER_MAJOR_SHIFTS,
		      env_ver & ENV_MCU_REV_MINOR);
	return(PASSED);
    }
    
    /* Setup I2C API parameter struct */
    /* Get ENV MCU I2C interface structure */
    rc = get_env_mcu_i2c_struct(&i2c_if);
    if (rc != PASSED) {
        printf("%s: Failed to get Power Sequencer I2C structure.\n",
              __FUNCTION__);
        return (rc);
    }
    
    /* Ready for the external temperature alert tests */
    /* Check Status */
    rc = env_get_status(&i2c_if, &stat1, &stat2, &err_buf[0], NULL);

    if (rc == PASSED) {
	if (stat1 & ENV_MCU_STAT1_1617_ALT) {
	    /* External alert is already set */
	    sprintf(err_buf, "env_ext_alert_test() External alert already set. "
			     "stat1 = %#x", stat1);
	    rc = FAILED;
	} else {
	    if ((stat1 & ENV_MCU_STAT1_AG_TEMP) ||
		((stat2 & ENV_MCU_STAT2_FAN_ST_M) == ENV_MCU_STAT2_FAN_FULL)) {
		/* Other condition causes aggregate bits set */
		sprintf(err_buf, "env_ext_alert_test() Unexpected aggregate "
				 "alerts. stat1 = %#x. stat2 = %#x",
				 stat1, stat2);
		rc = FAILED;
	    } else {
		/* Read the internal temperature of the external sensor */
		/* Also set the new alert threshold for the external sensor */
		rc = set_mb_temp_alert(ENV_ALERT_TEST_DELTA_LM75, &cur_alert,
				       &cur_cfg, &err_buf[0]);
	    } /* endof if stat1 & stat2 */
	} /* endof if stat1 */
    } /* endof if env_get_status */

    if (rc != PASSED) {
	cterr('f', 0, err_buf);
	return(FAILED);
    }

    msleep(SNSR_CONV_TIME);	/* Wait for the status to be set */

    /* Check status again */
    rc = env_get_status(&i2c_if, &stat1, &stat2, &err_buf[0], NULL);
    if (rc == PASSED) {
	if ((stat1 & (ENV_MCU_STAT1_1617_ALT | ENV_MCU_STAT1_AG_TEMP)) !=
		     (ENV_MCU_STAT1_1617_ALT | ENV_MCU_STAT1_AG_TEMP)) {
	    /* One or both of the expected status 1 not set */
	    sprintf(err_buf, "env_ext_alert_test() status 1 = %#x. Expect mask "
			     "of %#x", stat1,
			     ENV_MCU_STAT1_1617_ALT | ENV_MCU_STAT1_AG_TEMP);
	    rc = FAILED;
	} else {
	    if ((stat2 & ENV_MCU_STAT2_FAN_ST_M) != ENV_MCU_STAT2_FAN_FULL) {
		/* Fan not in full speed */
		sprintf(err_buf, "env_ext_alert_test() status 2 = %#x. Expect "
				 "mask of %#x", stat2, ENV_MCU_STAT2_FAN_FULL);
		rc = FAILED;
	    } else {
		/* Restore the alert threshold of the external sensor */
		rc = restore_mb_temp_alert(cur_alert, cur_cfg, &err_buf[0]);
		msleep(SNSR_CONV_TIME);
		if (rc == PASSED) {
		    /* Clear Status 2 */
		    rc = env_clear_stat2(&i2c_if, &stat2, &err_buf[0], NULL);
		    if (rc == PASSED) {
			/* Check if the status cleared */
			rc = env_get_status(&i2c_if, &stat1, &stat2,
					    &err_buf[0], NULL);
			if (rc == PASSED) {
			    if ((stat1 & (ENV_MCU_STAT1_1617_ALT |
					  ENV_MCU_STAT1_AG_TEMP)) ||
				((stat2 & ENV_MCU_STAT2_FAN_ST_M) ==
					  ENV_MCU_STAT2_FAN_FULL)) {
				sprintf(err_buf, "env_ext_alert_test() "
						 "Unexpected status. stat1 = "
						 "%#x. stat2 = %#x",
						 stat1, stat2);
				rc = FAILED;
			    } /* endof if stat1 */
			} /* endof if env_get_status */
		    } /* endof if env_clear_stat2 */
 		} /* endof if restore_1617_alert */
	    } /* endof if stat2 */
	} /* endof if stat1 */
    } /* endof if env_get_status */

    if (rc != PASSED) {
	cterr('f', 0, err_buf);
    }

    if (submenu == TRUE) {
	prcomplete(testpass, errcount, (char *)0);
    }
#endif 
    return(PASSED);

}

/*********************************************************************
 *
 * Function:	env_temp_alert_test
 *
 * Description:	Temperature Alert test. Refer to "Diagnostics
 *		Tests" in EDCS-534569, Rev 6 and later.
 *
 * Inputs:	submenu - TRUE if submenu invoked.
 *
 * Outputs:	PASSED/FAILED
 *
 * Assumptions: Assume Alerts are enabled.
 *
 *********************************************************************
 */
static int
env_temp_alert_test (int submenu)
{
    n2g_i2c_if_t i2c_if;
    uint32_t rc = FAILED;
    ren_t ctrl1, new_ctl;
    char err_buf[ERR_BUF_SIZE];

    if (submenu == TRUE) {
	testname("Temperature Alert");
    } else {
	prpass(testpass, "Temperature Alert Test");
    }

    /* Setup I2C API parameter struct */
    /* Get ENV MCU I2C interface structure */
    rc = get_env_mcu_i2c_struct(&i2c_if);
    if (rc != PASSED) {
        printf("%s: Failed to get Power Sequencer I2C structure.\n",
              __FUNCTION__);
        return (rc);
    }
    
    /* Save the original Control 1 */
    i2c_if.offset = ENV_MCU_CTL1;
    i2c_if.buf = (char *)&ctrl1;

    rc = env_read(&i2c_if);
    if (rc != PASSED) {
	sprintf(err_buf, "env_temp_alert_test() Unable to read control 1. "
			 "rc = %#x", rc);
	rc = FAILED;
    } else {
	/* Enable Inlet and Outlet Alert temperatures */
	new_ctl = ctrl1 | ENV_MCU_CTRL1_I1_AL_EN | ENV_MCU_CTRL1_I2_AL_EN |
		  ENV_MCU_CTRL1_O1_AL_EN | ENV_MCU_CTRL1_O2_AL_EN;
	i2c_if.buf = (char *)&new_ctl;
	rc = env_write(&i2c_if);
	if (rc != PASSED) {
	    sprintf(err_buf, "env_temp_alert_test() Unable to enable alerts. "
			     "rc = %#x", rc);
	    rc = FAILED;
	} else {
	    /* Ready to perform Alert Temperature tests */
	    rc = env_alert_temp_test(&i2c_if, ENABLE, &err_buf[0]);
	    if (rc == PASSED) {
		/* Restore Control 1 register */
		i2c_if.offset = ENV_MCU_CTL1;
		i2c_if.buf = (char *)&ctrl1;
		rc = env_write(&i2c_if);
		if (rc != PASSED) {
		    sprintf(err_buf, "env_temp_alert_test() Unable to restore "
				     "Control 1 register. rc = %#x", rc);
		    rc = FAILED;
		} /* endof if env_write restore */
	    } /* endof if env_alert_temp_test */
	} /* endof if rc env_write */
    } /* endof if env_read */

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
 * Function:	env_temp_alert_enable_test
 *
 * Description:	Temperature Alert Enable test. Refer to "Diagnostics
 *		Tests" in EDCS-534569, Rev 6 and later.
 *
 * Inputs:	submenu - TRUE if submenu invoked.
 *
 * Outputs:	PASSED/FAILED
 *
 * Assumptions:
 *
 *********************************************************************
 */
static int
env_temp_alert_enable_test (int submenu)
{
    n2g_i2c_if_t i2c_if;
    uint32_t rc = FAILED;
    ren_t ctrl1, new_ctl;
    char err_buf[ERR_BUF_SIZE];

    if (submenu == TRUE) {
	testname("Temperature Alert Enable");
    } else {
	prpass(testpass, "Temperature Alert Enable Test");
    }

    /* Setup I2C API parameter struct */
    /* Get ENV MCU I2C interface structure */
    rc = get_env_mcu_i2c_struct(&i2c_if);
    if (rc != PASSED) {
        printf("%s: Failed to get Power Sequencer I2C structure.\n",
              __FUNCTION__);
        return (rc);
    }
    
    /* Save the original Control 1 */
    i2c_if.offset = ENV_MCU_CTL1;
    i2c_if.buf = (char *)&ctrl1;

    rc = env_read(&i2c_if);
    if (rc != PASSED) {
	sprintf(err_buf, "env_temp_alert_enable_test() Unable to read control "
			 "1. rc = %#x", rc);
	rc = FAILED;
    } else {
	/* Enable Inlet and Outlet Alert temperatures */
	new_ctl = ctrl1 | ENV_MCU_CTRL1_I1_AL_EN | ENV_MCU_CTRL1_I2_AL_EN |
		  ENV_MCU_CTRL1_O1_AL_EN | ENV_MCU_CTRL1_O2_AL_EN;
	i2c_if.buf = (char *)&new_ctl;
	rc = env_write(&i2c_if);
	if (rc != PASSED) {
	    sprintf(err_buf, "env_temp_alert_enable_test() Unable to enable "
			     "alerts. rc = %#x", rc);
	    rc = FAILED;
	} else {
	    /* Ready to perform Alert Temperature tests */
	    rc = env_alert_temp_test(&i2c_if, ENABLE, &err_buf[0]);

	    if (rc == PASSED) {
		/* Disable Alert temperatures */
		new_ctl = ctrl1 & (~(ENV_MCU_CTRL1_I1_AL_EN |
				     ENV_MCU_CTRL1_I2_AL_EN |
				     ENV_MCU_CTRL1_O1_AL_EN |
				     ENV_MCU_CTRL1_O2_AL_EN));
		i2c_if.buf = (char *)&new_ctl;
		rc = env_write(&i2c_if);
		if (rc != PASSED) {
		    sprintf(err_buf, "env_temp_alert_enable_test() Unable to "
				     "disable alert. rc = %#x", rc);
		    rc = FAILED;
		} else {
		    /* Ready to perform Alert Temperature tests */
		    rc = env_alert_temp_test(&i2c_if, DISABLE, &err_buf[0]);

		    if (rc == PASSED) {
			/* Restore Control 1 register */
			i2c_if.buf = (char *)&ctrl1;
			rc = env_write(&i2c_if);
			if (rc != PASSED) {
			    sprintf(err_buf, "env_temp_alsert_enable_test(). "
					     "Unable to restore Control 1 "
					     "register. rc = %#x", rc);
			    rc = FAILED;
			} /* endof if env_write restore */
		    } /* endof if env_alert_temp_test disable */
		} /* endof if env_write of new_ctl disable */
	    } /* endof if env_alert_temp_test enable */
	} /* endof if env_write of new_ctl enable */
    } /* endof if read */

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
 * Function:    env_hyst_test
 *
 * Description:	Hysteresis Temperature Out-of-Order test. Refer to "Diagnostics
 *		Tests" in EDCS-534569, Rev 6 and later.
 *
 * Inputs:	submenu - TRUE if submenu invoked.
 *
 * Outputs:	PASSED/FAILED
 *
 * Assumptions:
 *
 *********************************************************************
 */
static int
env_hyst_test (int submenu)
{
    n2g_i2c_if_t i2c_if;
    uint32_t rc = FAILED;
    ren_t temp1, temp4, stat1, stat2;
    char err_buf[ERR_BUF_SIZE];

    if (submenu == TRUE) {
	testname("Hysteresis Temperature Out-of_order");
    } else {
	prpass(testpass, "Hysteresis Temperature Out-of_order Test");
    }

    if (env_ver < ENV_TEST_VER) {
	cterr('w', 0, "Hysteresis Temperature Test not supported in "
		      "Version %d.%d", env_ver >> ENV_VER_MAJOR_SHIFTS,
		      env_ver & ENV_MCU_REV_MINOR);
	return(PASSED);
    }

    /* Setup I2C API parameter struct */
    /* Get ENV MCU I2C interface structure */
    rc = get_env_mcu_i2c_struct(&i2c_if);
    if (rc != PASSED) {
        printf("%s: Failed to get Power Sequencer I2C structure.\n",
              __FUNCTION__);
        return (rc);
    }
    
    /* Clear Status 2 */
    rc = env_clear_stat2(&i2c_if, &stat2, &err_buf[0], NULL);

    if (rc == PASSED) {
	/* Check status */
	rc = env_get_status(&i2c_if, &stat1, &stat2, &err_buf[0], NULL);
	if (rc == PASSED) {
	    /* Ignore Current Fan Status and Reverse Airflow */
	    if (stat2 & (~(ENV_MCU_STAT2_FAN_ST_M | ENV_MCU_STAT2_REV_AIR))) {
		/* Status 2 bits not cleared */
		sprintf(err_buf, "env_hyst_test() Status 2 bits not cleared "
				 "initially. stat2 = %#x", stat2);
		rc = FAILED;
	    } /* endof if stat2 */
	} /* endof if rc env_get_status */
    } /* endof if rc env_clear_stat2 */

    if (rc != PASSED) {
	cterr('f', 0, err_buf);
	return(FAILED);
    }

    /* Get Temperature 1 Mid to Low register, and Temperature 4 Mid to High
     * register.
     */
    i2c_if.offset = ENV_MCU_T4;
    i2c_if.buf = (char *)&temp4;
    rc = env_read(&i2c_if);
    if (rc != PASSED) {
	sprintf(err_buf, "env_hyst_test(). Read Temperature 4 register failed. "
			 "rc = %#x", rc);
    } else {
	/* Ready to read Temperature 1 */
	i2c_if.offset = ENV_MCU_T1;
	i2c_if.buf = (char *)&temp1;
	rc = env_read(&i2c_if);
	if (rc != PASSED) {
	    sprintf(err_buf, "env_hyst_test(). Read Temperature 1 register "
			     "failed. rc = %#x", rc);
	} else {
	    /* Got temperture 1 and 4 registers. Set temp 1 to temp 4 */
	    i2c_if.buf = (char *)&temp4;
	    rc = env_write(&i2c_if);
	    if (rc != PASSED) {
		sprintf(err_buf, "env_hyst_test(). Write Temperature 1 "
				 "register failed. rc = %#x", rc);
	    }

#ifdef ENV_HYST_DEBUG
	    printf("\nwrote %#x to %#x\n", temp1, i2c_if.offset);
#endif /* ENV_HYST_DEBUG */
	}
    }

    if (rc != PASSED) {
	cterr('f', 0, err_buf);
	return(FAILED);
    }

    /* Check Status bits */
    rc = env_get_status(&i2c_if, &stat1, &stat2, &err_buf[0], NULL);

    if (rc == PASSED) {
	if (((stat1 & ENV_MCU_STAT1_AG_SW_ER) == 0) ||
	    ((stat2 & (ENV_MCU_STAT2_HYS_TEMP | ENV_MCU_STAT2_FAN_ST_M)) !=
	     (ENV_MCU_STAT2_HYS_TEMP | ENV_MCU_STAT2_FAN_FULL))) {
	    sprintf(err_buf, "env_hyst_test(). Expect stat1 %#x, got %#x. "
			     "Expect stat2 %#x, got %#x",
			     ENV_MCU_STAT1_AG_SW_ER, stat1,
			     ENV_MCU_STAT2_HYS_TEMP | ENV_MCU_STAT2_FAN_FULL,
			     stat2);
	    rc = FAILED;
	} else {
	    /* Got expected status. Restore temp 1 */
	    i2c_if.buf = (char *)&temp1;
	    i2c_if.offset = ENV_MCU_T1;
	    rc = env_write(&i2c_if);
	    if (rc != PASSED) {
		sprintf(err_buf, "env_hyst_test(). Unable to restore "
				 "Temperature 1 register. rc = %#x", rc);
	    } else {
		/* Clear status 2 bits */
		rc = env_clear_stat2(&i2c_if, &stat2, &err_buf[0], NULL);
		if (rc == PASSED) {
		    /* Check if Status 2 bits are cleared */
		    rc = env_get_status(&i2c_if, &stat1, &stat2, &err_buf[0],
					NULL);
		    if (rc == PASSED) {
			if (stat2 & ENV_MCU_STAT2_HYS_TEMP) {
			    sprintf(err_buf, "env_hyst_test(). Unexpected "
					     "Hysteresis bit been set. "
					     "stat2 = %#x", stat2);
			    rc = FAILED;
			} /* endof if stat2 */
		    } /* endof if rc env_get_status */
		} /* endof if rc env_clear_stat2 */
	    } /* endof if rc env_write */
	} /* endof if stat1 */
    } /* endof if rc env_get_status */

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
 * Function:    env_fan_en_test
 *
 * Description:	No Fan Enable test. Refer to "Diagnostics
 *		Tests" in EDCS-534569, Rev 6 and later.
 *
 * Inputs:	submenu - TRUE if submenu invoked.
 *
 * Outputs:	PASSED/FAILED
 *
 * Assumptions:
 *
 *********************************************************************
 */
static int
env_fan_en_test (int submenu)
{
    n2g_i2c_if_t i2c_if;
    uint32_t rc = FAILED;
    ren_t ctrl1, new_ctrl, stat1, stat2;
    char err_buf[ERR_BUF_SIZE];

    if (submenu == TRUE) {
	testname("No Fan Enable");
    } else {
	prpass(testpass, "No Fan Enable Test");
    }

    /* Setup I2C API parameter struct */
    /* Get ENV MCU I2C interface structure */
    rc = get_env_mcu_i2c_struct(&i2c_if);
    if (rc != PASSED) {
        printf("%s: Failed to get Power Sequencer I2C structure.\n",
              __FUNCTION__);
        return (rc);
    }
    
    /* Get Control 1 register */
    i2c_if.offset = ENV_MCU_CTL1;
    i2c_if.buf = (char *)&ctrl1;
    rc = env_read(&i2c_if);
    if (rc != PASSED) {
	sprintf(err_buf, "env_fan_en_test() Read Control 1 register failed. "
			 "rc = %#x", rc);
    } else {
	/* Got control 1 register. Disable all fans */
	new_ctrl = ctrl1 & (~(ENV_MCU_CTRL1_FAN5_EN | ENV_MCU_CTRL1_FAN4_EN |
			      ENV_MCU_CTRL1_FAN3_EN | ENV_MCU_CTRL1_FAN2_EN |
			      ENV_MCU_CTRL1_FAN1_EN));
	i2c_if.buf = (char *)&new_ctrl;
	rc = env_write(&i2c_if);
	if (rc != PASSED) {
	    sprintf(err_buf, "env_fan_en_test() Unable to write Control 1 "
			     "register. rc = %#x", rc);
	}
    }

    if (rc != PASSED) {
	cterr('f', 0, err_buf);
	return(FAILED);
    }

    /* Check Status bits */
    rc = env_get_status(&i2c_if, &stat1, &stat2, &err_buf[0], NULL);

    if (rc == PASSED) {
	if (((stat1 & ENV_MCU_STAT1_AG_SW_ER) == 0) ||
	    ((stat2 & ENV_MCU_STAT2_NO_FAN) == 0)) {
	    sprintf(err_buf, "env_fan_en_test() Status not set first time. "
			     "stat1 = %#x, stat2 = %#x", stat1, stat2);
	    rc = FAILED;
	} else {
	    /* Clear status 2 */
	    rc = env_clear_stat2(&i2c_if, &stat2, &err_buf[0], NULL);
	    if (rc == PASSED) {
		/* Check status bits again */
		rc = env_get_status(&i2c_if, &stat1, &stat2, &err_buf[0], NULL);
		if (rc == PASSED) {
		    if (((stat1 & ENV_MCU_STAT1_AG_SW_ER) == 0) ||
			((stat2 & ENV_MCU_STAT2_NO_FAN) == 0)) {
			sprintf(err_buf, "env_fan_en_test() Status not set "
					 "second time. stat1 = %#x. stat2 = "
					 "%#x.", stat1, stat2);
			rc = FAILED;
		    } else {
			/* Status checked. Restore Control 1 register */
			i2c_if.offset = ENV_MCU_CTL1;
			i2c_if.buf = (char *)&ctrl1;
			rc = env_write(&i2c_if);
			if (rc != PASSED) {
			    sprintf(err_buf, "env_fan_en_test() Unable to "
					     "restore Control 1 register. rc = "
					     "%#x", rc);
			} else {
			    /* Control 1 register restored.
			     *Clear status 2 again
			     */
			    rc = env_clear_stat2(&i2c_if, &stat2, &err_buf[0],
						 NULL);
			    if (rc == PASSED) {
				/* Check status bits again */
				rc = env_get_status(&i2c_if, &stat1, &stat2,
						    &err_buf[0], NULL);
				if (rc == PASSED) {
				    if (stat2 & ENV_MCU_STAT2_NO_FAN) {
					sprintf(err_buf, "env_fan_en_test() "
							 "Unable to clear No "
							 "Fan enabled. stat2 "
							 "= %#x", stat2);
					rc = FAILED;
				    } /* endof if stat2 */
				} /* endof if rc env_get_status */
			    } /* endof if rc */
			} /* endof if rc */
			msleep(REN_I2C_PROC_TIME);
		    } /* endof if stat1 */
		} /* endif if rc env_get_status */
	    } /* endof if rc env_clear_stat2 */
	} /* endof if first stat1 */
    }

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
 * Function:    env_wdg_test
 *
 * Description:	Watchdog Test. Refer to "Diagnostics Tests" in EDCS-534569,
 *		Rev 6 and later.
 *
 * Inputs:	submenu - TRUE for submenu invoked.
 *
 * Outputs:	PASSED/FAILED
 *
 * Assumptions:
 *
 *********************************************************************
 */
static int
env_wdg_test (int submenu)
{
    dev_env_mcu_t reg_log;
    n2g_i2c_if_t i2c_if;
    uint32_t rc = FAILED;
    ren_t stat1, stat2;
#ifndef FPGA_RESET_MCU
    ren_t wdg_tst, wdg_ctr;
#endif /* FPGA_RESET_MCU */
    char err_buf[ERR_BUF_SIZE];

    if (submenu == TRUE) {
	testname("Watchdog");
    } else {
	prpass(testpass, "Watchdog Test");
    }

    /* Setup I2C API parameter struct */
    /* Get ENV MCU I2C interface structure */
    rc = get_env_mcu_i2c_struct(&i2c_if);
    if (rc != PASSED) {
        printf("%s: Failed to get Power Sequencer I2C structure.\n",
              __FUNCTION__);
        return (rc);
    }
    
    /* Log the registers of all writeable registers */
    rc = env_regs_access(&i2c_if, N2G_I2C_READ, &reg_log, &err_buf[0]);
    if (rc == PASSED) {
	/* Write to Status 2 to clear status bits */
	rc = env_clear_stat2(&i2c_if, &stat2, &err_buf[0], NULL);
	if (rc == PASSED) {
	    /* Check the watchdog reset flag bit */
	    rc = env_get_status(&i2c_if, &stat1, &stat2, &err_buf[0], NULL);
	    if (rc == PASSED) {
		if (stat2 & ENV_MCU_STAT2_WD_RST) {
		    /* Watchdog Reset bit is set */
		    sprintf(err_buf, "env_wdg_test() Unexpected watchdog reset "
				     "bit is set. stat2 = %#x", stat2);
		    rc = FAILED;
		} else {
#ifdef FPGA_RESET_MCU
		    /* Reset the MCU */
		    IOFPGA_MAIN_REGS->env_mcu |= MCU_CTRL_RESET_EN;
		    flush_io_wb();

		    IOFPGA_MAIN_REGS->env_mcu |= MCU_CTRL_RESET;
		    flush_io_wb();
		    msleep(ENV_MCU_RESET_PULSE); /* 10 clock pulses of slower
						 * clock. or 80 us. Make it 1 ms
						 */
		    /* Release reset */
		    IOFPGA_MAIN_REGS->env_mcu &= (~(MCU_CTRL_RESET_EN |
						    MCU_CTRL_RESET));
		    msleep(ENV_MCU_INIT_TIME);	/* 2 seconds */
		    /*Check the watchdog reset flag bit again */
		    rc = env_get_status(&i2c_if, &stat1, &stat2, &err_buf[0],
					NULL);
#else /* Use Soft reset watchdog test */
		    /* Write the watchdog sequence into the Soft Reset /
		     * Watchdog Test register */
		    i2c_if.buf = (char *)&wdg_tst;
		    i2c_if.offset = ENV_MCU_SFT_WDG;
		    wdg_tst = ENV_MCU_WDG_TST1;
		    rc = env_write(&i2c_if);
		    if (rc == PASSED) {
			/* Wrote first data pattern. Ready to write second */
			wdg_tst = ENV_MCU_WDG_TST2;
			rc =  env_write(&i2c_if);
		    } /* endof if tst 1 write */

		    /* Check if write successful */
		    if (rc == PASSED) {
			msleep(ENV_MCU_INIT_TIME);	/* 2 seconds */
			/*Check the watchdog reset flag bit again */
			rc = env_get_status(&i2c_if, &stat1, &stat2,
					    &err_buf[0], NULL);
		    } else {
			sprintf(err_buf, "env_wdg_test() Unable to write %#x "
					 "to Soft Reset register. rc = %#x",
					 wdg_tst, rc);
			rc = FAILED;
		    } /* endof if tst1 and tst2 write */

#endif /* FPGA_RESET_MCU */
		    if (rc == PASSED) {
			if ((stat2 & ENV_MCU_STAT2_WD_RST) == 0) {
			    sprintf(err_buf, "env_wdg_test() Watchdog reset is "
					     "not set after reset. stat2 = %#x",
					     stat2);
			    rc = FAILED;
			} else {
			    /* Clear status 2 register */
			    rc = env_clear_stat2(&i2c_if, &stat2, &err_buf[0],
						 NULL);
			    if (rc == PASSED) {
#ifdef FPGA_RESET_MCU
				/* Restore writeable registers */
				rc = env_regs_access(&i2c_if, N2G_I2C_WRITE,
						     &reg_log, &err_buf[0]);
#else /* Soft reset/Watchdog test */
				/* Read Watchdog Reset Counter */
				i2c_if.buf = (char *)&wdg_ctr;
				i2c_if.offset = ENV_MCU_WRT;
				rc = env_read(&i2c_if);
				if (rc == PASSED) {
				    if (wdg_ctr != (reg_log.wd_rst_to + 1)) {
					sprintf(err_buf, "env_wdg_test() Got "
						"%#x. Expect %#x in Watchdog "
						"Reset Counter", wdg_ctr,
						reg_log.wd_rst_to + 1);	
					rc = FAILED;
				    } else {
					/* Restore writeable registers */
					rc = env_regs_access(&i2c_if,
							     N2G_I2C_WRITE,
							     &reg_log,
							     &err_buf[0]);
				    } /* endof if wdg_ctr */
				} else {
				    sprintf(err_buf, "env_wdg_test() Read "
						     "Watchdog Reset Counter "
						     "failed. rc = %#x", rc);
				    rc = FAILED;
				} /* endof if watchdog reset counter */
#endif /* FPGA_RESET_MCU */
			    } /* endof if rc */
			} /* endof if stat2 */
		    } /* endof if rc */
		} /* endof if stat2 */
	    } /* endof if rc */
	} /* endof if rc */
    } /* endof if rc */

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
 * Function:	env_hys_test
 *
 * Description:	Hysteresis test. Refer to "Diagnostics Tests" in EDCS-534569,
 *		Rev 7.04 and later.
 *
 * Inputs:	submenu - TRUE if submenu invoked.
 *
 * Outputs:	PASSED/FAILED
 *
 * Assumptions:
 *
 *********************************************************************
 */
static int
env_hys_test (int submenu)
{
    n2g_i2c_if_t i2c_if;
    env_hyst_test_t hyst_table[] = {
			 {ENV_HYST_TIME, ENV_MCU_STAT2_FAN_LO, 0, ENV_MCU_T1},
			 {ENV_HYST_TIME, ENV_MCU_STAT2_FAN_L2, 0, ENV_MCU_T2},
			 {ENV_HYST_TIME, ENV_MCU_STAT2_FAN_MID, 0, ENV_MCU_T4},
			 {ENV_HYST_TIME, ENV_MCU_STAT2_FAN_L4, 0, ENV_MCU_T6},
			 {ENV_HYST_TIME, ENV_MCU_STAT2_FAN_HI, 0, ENV_MCU_T8},
			 {ENV_HYST_TIME, ENV_MCU_STAT2_FAN_L4, 0, ENV_MCU_T7},
			 {ENV_HYST_TIME, ENV_MCU_STAT2_FAN_MID, 0, ENV_MCU_T5},
			 {ENV_HYST_TIME, ENV_MCU_STAT2_FAN_L2, 0, ENV_MCU_T3},
			 {ENV_HYST_TIME, ENV_MCU_STAT2_FAN_LO, 0, ENV_MCU_T1},
			 {0, 0, 0, 0},
			};
    env_hyst_test_t *hyst_ptr;
    uint32_t rc = FAILED;
    ren_t ctrl2, scr;
    char err_buf[ERR_BUF_SIZE];

    if (submenu == TRUE) {
	testname("Hysteresis");
    } else {
	prpass(testpass, "Hysteresis Test");
    }

    if (env_ver < ENV_TEST_VER) {
	cterr('w', 0, "Hysteresis Test not supported in "
		      "Version %d.%d", env_ver >> ENV_VER_MAJOR_SHIFTS,
		      env_ver & ENV_MCU_REV_MINOR);
	return(PASSED);
    }

    /* Setup I2C API parameter struct */
    /* Get ENV MCU I2C interface structure */
    rc = get_env_mcu_i2c_struct(&i2c_if);
    if (rc != PASSED) {
        printf("%s: Failed to get Power Sequencer I2C structure.\n",
              __FUNCTION__);
        return (rc);
    }
    
    /* Copy Temperature 1 to ScratchPad */
    i2c_if.offset = ENV_MCU_T1;
    i2c_if.buf = (char *)&scr;

    /* Read Temperature 1 first */
    rc = env_read(&i2c_if);
    if (rc != PASSED) {
	/* Temperature 1 Register read failed */
	sprintf(err_buf, "env_hys_test() Temperature 1 Register read failed. "
			 "rc = %#x", rc);
	rc = FAILED;
    } else {
	/* Write to Scratchpad register */
	i2c_if.offset = ENV_MCU_SCR;

	rc = env_write(&i2c_if);
	if (rc != PASSED) {
	    /* Scratchpad register write failed */
	    sprintf(err_buf, "env_hys_test() Scratchpad Register write failed. "
			     "rc = %#x", rc);
	    rc = FAILED;
	} else {
	    /* Save the original Control 2 register */
	    i2c_if.offset = ENV_MCU_CTL2;
	    i2c_if.buf = (char *)&ctrl2;

	    rc = env_read(&i2c_if);
	    if (rc != PASSED) {
		/* Control 2 Register read failed */
		sprintf(err_buf, "env_hys_test() Control 2 Register read "
				 "failed. rc = %#x", rc);
		rc = FAILED;
	    } else {
		/* Set to use ScratchPad register value */
		scr = ctrl2 & (~ENV_MCU_CTRL2_FAN_IN_M);
		scr |= ENV_MCU_CTRL2_FCTL_SC;

		/* Clear timed hysteresis */
		scr &= (~ENV_MCU_CTRL2_HYS_M);

		i2c_if.buf = (char *)&scr;

		rc = env_write(&i2c_if);
		if (rc != PASSED) {
		    /* Control 2 register write failed */
		    sprintf(err_buf, "env_hys_test() Control 2 Register write "
				     "failed. rc = %#x", rc);
		    rc = FAILED;
		} /* endof if rc write control 2 */
	    } /* endof if rc read control 2 */
	} /* endof if rc write scratchpad */
    } /* endof rc read of temp 1 */

    if (rc == PASSED) {
	/* Ready for the test */
	hyst_ptr = &hyst_table[0];
	while (hyst_ptr->offset) {
	    if ((rc = hyst_stat_check(&i2c_if, hyst_ptr, &err_buf[0])) !=
					   PASSED) {
		break;
	    } else {
		hyst_ptr++;
	    } /* endof if rc */
	} /* endof while */

	if (rc == PASSED) {
	    /* Restore Control 2 */
	    i2c_if.offset = ENV_MCU_CTL2;
	    i2c_if.buf = (char *)&ctrl2;

	    rc = env_write(&i2c_if);

	    if (rc != PASSED) {
		sprintf(err_buf, "env_hys_test() Unable to restore Control 2 "
				 " register. rc = %#x", rc);
		rc = FAILED;
	    } /* endof if rc control 2 */
	} else {
	    rc = FAILED;
	} /* endof if rc */
    } /* endof if rc from while */

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
 * Function:	env_time_hys_test
 *
 * Description:	Timed Hysteresis test. Refer to "Diagnostics Tests" in
 *		EDCS-534569, Rev 7.04 and later.
 *
 * Inputs:	submenu - TRUE if submenu invoked.
 *
 * Outputs:	PASSED/FAILED
 *
 * Assumptions:
 *
 *********************************************************************
 */
static int
env_time_hys_test (int submenu)
{
    n2g_i2c_if_t i2c_if;
    env_hyst_test_t hyst_table[] = {
			 {ENV_HYST_TIME, ENV_MCU_STAT2_FAN_LO, 0, ENV_MCU_T1},
			 {ENV_HYST_TIME, ENV_MCU_STAT2_FAN_L2, 0, ENV_MCU_T2},
			 {ENV_HYST_TIME, ENV_MCU_STAT2_FAN_MID, 0, ENV_MCU_T4},
			 {ENV_HYST_TIME, ENV_MCU_STAT2_FAN_L4, 0, ENV_MCU_T6},
			 {ENV_HYST_TIME, ENV_MCU_STAT2_FAN_HI, 0, ENV_MCU_T8},
			 {ENV_HYST_TIM2, ENV_MCU_STAT2_FAN_L4, 1, ENV_MCU_T8},
			 {ENV_HYST_TIM2, ENV_MCU_STAT2_FAN_MID, 1, ENV_MCU_T6},
			 {ENV_HYST_TIM2, ENV_MCU_STAT2_FAN_L2, 1, ENV_MCU_T4},
			 {ENV_HYST_TIM2, ENV_MCU_STAT2_FAN_LO, 1, ENV_MCU_T2},
			 {0, 0, 0, 0},
			};
    env_hyst_test_t *hyst_ptr;
    uint32_t rc = FAILED;
    ren_t ctrl2, scr;
    char err_buf[ERR_BUF_SIZE];

    if (submenu == TRUE) {
	testname("Timed Hysteresis");
    } else {
	prpass(testpass, "Timed Hysteresis Test");
    }

    if (env_ver < ENV_TEST_VER) {
	cterr('w', 0, "Timed Hysteresis Test not supported in "
		      "Version %d.%d", env_ver >> ENV_VER_MAJOR_SHIFTS,
		      env_ver & ENV_MCU_REV_MINOR);
	return(PASSED);
    }

    /* Setup I2C API parameter struct */
    /* Get ENV MCU I2C interface structure */
    rc = get_env_mcu_i2c_struct(&i2c_if);
    if (rc != PASSED) {
        printf("%s: Failed to get Power Sequencer I2C structure.\n",
              __FUNCTION__);
        return (rc);
    }
    
    /* Copy Temperature 1 to ScratchPad */
    i2c_if.offset = ENV_MCU_T1;
    i2c_if.buf = (char *)&scr;

    /* Read Temperature 1 first */
    rc = env_read(&i2c_if);
    if (rc != PASSED) {
	/* Temperature 1 Register read failed */
	sprintf(err_buf, "env_time_hys_test() Temperature 1 Register read "
			 "failed. rc = %#x", rc);
	rc = FAILED;
   } else {
	/* Write to Scratchpad register */
	i2c_if.offset = ENV_MCU_SCR;

	rc = env_write(&i2c_if);
	if (rc != PASSED) {
	    /* Scratchpad register write failed */
	    sprintf(err_buf, "env_time_hys_test() Scratchpad Register write "
			     "failed. rc = %#x", rc);
	    rc = FAILED;
	} else {
	    /* Set the Fan Hysteresis Timeout */
	    i2c_if.offset = ENV_MCU_HTO;
	    scr = HYST_FAN_TIMEOUT;

	    rc = env_write(&i2c_if);
	    if (rc != PASSED) {
		sprintf(err_buf, "env_time_hys_test() Fan Hysteresis Timeout "
				 "write failed. rc = %#x", rc);
		rc = FAILED;
	    } else {
		/* Save the original Control 2 register */
		i2c_if.offset = ENV_MCU_CTL2;
		i2c_if.buf = (char *)&ctrl2;

		rc = env_read(&i2c_if);
		if (rc != PASSED) {
		    /* Control 2 Register read failed */
		    sprintf(err_buf, "env_time_hys_test() Control 2 Register "
				     "read failed. rc = %#x", rc);
		    rc = FAILED;
		} else {
		    /* Set to use ScratchPad register value */
		    scr = ctrl2 & (~ENV_MCU_CTRL2_FAN_IN_M);
		    scr |= ENV_MCU_CTRL2_FCTL_SC;

		    /* Also set to normal hysteresis to avoid 60 seconds
		     * high level to low level delay */
		    scr &= (~ENV_MCU_CTRL2_HYS_M);

		    i2c_if.buf = (char *)&scr;

		    rc = env_write(&i2c_if);
		    if (rc != PASSED) {
			/* Control 2 register write failed */
			sprintf(err_buf, "%s() Control 2 Register write "
					 "failed. rc = %#x",
					 __FUNCTION__, rc);
			rc = FAILED;
		    } else {
			/* Wait 3 seconds then set timed hysteresis */
			msleep(3 * MS_PER_SECOND);

			scr |= ENV_MCU_CTRL2_HYS_2;

			rc = env_write(&i2c_if);
			if (rc != PASSED) {
			    /* Unable to set timed hysteresis */
			    sprintf(err_buf, "%s() Unable to set timed "
					     "hysteresis bit. rc = %#x",
					     __FUNCTION__, rc);
			    rc = FAILED;
			} else {
			    prpass(testpass, "Wait %d seconds for fans to "
					     "settle", ENV_HYST_TIME / 1000);
			} /* endof if hyst bit */
		    } /* endof if rc write control 2 */
		} /* endof if rc read control 2 */
	    } /* endof if rc write fan hysteresis timeout */
	} /* endof if rc write scratchpad */
    } /* endof rc read of temp 1 */

    if (rc == PASSED) {
	/* Ready for the test */
	hyst_ptr = &hyst_table[0];
	while (hyst_ptr->offset) {
	    if ((rc = hyst_stat_check(&i2c_if, hyst_ptr, &err_buf[0])) !=
					   PASSED) {
		break;
	    } else {
		hyst_ptr++;
	    } /* endof if rc */
	} /* endof while */

	if (rc == PASSED) {
	    /* Restore Control 2 */
	    i2c_if.offset = ENV_MCU_CTL2;
	    i2c_if.buf = (char *)&ctrl2;

	    rc = env_write(&i2c_if);

	    if (rc != PASSED) {
		sprintf(err_buf, "env_time_hys_test() Unable to restore "
				 "Control 2 register. rc = %#x", rc);
		rc = FAILED;
	    } /* endof if rc Control 2 */
	} else {
	    rc = FAILED;
	} /* endof if from while */
    } /* endof if rc */

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
 * Function:	env_force_fan_test
 *
 * Description:	Force Fan Alert test. Refer to "Diagnostics Tests" in
 *		EDCS-534569, Rev 9 and later.
 *
 * Inputs:	submenu - TRUE if submenu invoked.
 *
 * Outputs:	PASSED/FAILED
 *
 * Assumptions:
 *
 *********************************************************************
 */
static int
env_force_fan_test (int submenu)
{
    n2g_i2c_if_t i2c_if;
    uint32_t rc = FAILED;
    ren_t ctrl2, test_ctrl2, stat2;
    char err_buf[ERR_BUF_SIZE];

    if (submenu == TRUE) {
	testname("Force Fan Alert");
    } else {
	prpass(testpass, "Force Fan Alert Test");
    }

    if (env_ver < ENV_TEST_VER4P4) {
	cterr('w', 0, "Force Fan Alert Test not supported in "
		      "Version %d.%d", env_ver >> ENV_VER_MAJOR_SHIFTS,
		      env_ver & ENV_MCU_REV_MINOR);
	return(PASSED);
    }

    /* Setup I2C API parameter struct */
    /* Get ENV MCU I2C interface structure */
    rc = get_env_mcu_i2c_struct(&i2c_if);
    if (rc != PASSED) {
        printf("%s: Failed to get Power Sequencer I2C structure.\n",
              __FUNCTION__);
        return (rc);
    }
    
    /* Store original value of Control 2 to be written back
     * at the end of the test.
     */
    i2c_if.offset = ENV_MCU_CTL2;
    i2c_if.buf = (char *)&ctrl2;
    rc = env_read(&i2c_if);
    if (rc != PASSED) {
	/* Control 2 Register read failed */
	sprintf(err_buf, "%s() Control 2 Register read failed. rc = %#x",
			 __FUNCTION__, rc);
	rc = FAILED;
    } else {
	/* Set the FORCE_FANS_ALERT bit in Control 2 */
	test_ctrl2 = ctrl2 | ENV_MCU_CTRL2_FR_FN_AL;
	i2c_if.buf = (char *)&test_ctrl2;
	rc = env_write(&i2c_if);
	if (rc != PASSED) {
	    /* Write failed */
	    sprintf(err_buf, "%st() Unable to force fan alert. rc = %#x",
			     __FUNCTION__, rc);
	    rc = FAILED;
	} else {
	    /* Wait 3 seconds */
	    msleep(ENV_FORCE_FAN_AL_TIME);

	    /* Check that CURRENT_FAN_STATUS is '111' (alert status) in
	     * Status 2 */
	    i2c_if.offset = ENV_MCU_STA2;
	    i2c_if.buf = (char *)&stat2;
	    rc = env_read(&i2c_if);
	    if (rc != PASSED) {
		/* Status 2 Register read failed */
		sprintf(err_buf, "%s() Status 2 Register read failed. rc = %#x",
				 __FUNCTION__, rc);
		rc = FAILED;
	    } else {
		/* Check for alert status */
		if ((stat2 & ENV_MCU_STAT2_FAN_ST_M) !=
		    ENV_MCU_STAT2_FAN_FULL) {
		    /* Not alert */
		    printf(err_buf, "%s() Alert status not set. stat 2 = %#x",
				    __FUNCTION__, stat2);
		    rc = FAILED;
		} /* endof if stat2 */
	    } /* endof if stat2 read */

	    /* Write back the original value to Control 2 */
	    i2c_if.offset = ENV_MCU_CTL2;
	    i2c_if.buf = (char *)&ctrl2;
	    rc = env_write(&i2c_if);
	    if (rc != PASSED) {
		/* Unable to recover Control 2 */
		sprintf(err_buf, "%s() Unable to recover Control 2 register. "
				 "rc = %#x", __FUNCTION__, rc);
		rc = FAILED;
	    } /* endof if ctrl2 recover write */
	} /* endof if force fan alert write */
    } /* endof if control 2 read */

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
 * Function:	env_temp_offset_test
 *
 * Description:	Temperature Offset test. Refer to "Diagnostics Tests" in
 *		EDCS-534569, Rev 9 and later.
 *
 * Inputs:	submenu - TRUE if submenu invoked.
 *
 * Outputs:	PASSED/FAILED
 *
 * Assumptions:
 *
 *********************************************************************
 */
static int
env_temp_offset_test (int submenu)
{
    n2g_i2c_if_t i2c_if;
    env_tmp_off_test_t tmp_test_tbl[] = {
	{"Inlet 1", ENV_MCU_I1T, ENV_MCU_I1TO},
	{"Inlet 2", ENV_MCU_I2T, ENV_MCU_I2TO},
	{"Outlet 1", ENV_MCU_O1T, ENV_MCU_O1TO},
	{"Outlet 2", ENV_MCU_O2T, ENV_MCU_O2TO},
	{NULL, 0, 0},
	};
    env_tmp_off_test_t *tmp_reg_p = &tmp_test_tbl[0];
    uint32_t rc = FAILED;
    short original_temp, new_temp, max_temp, min_temp;
    ren_t tmp_off, temp, new_tmp_off, new_temp_rd;
    char err_buf[ERR_BUF_SIZE * 2], char_temp;
    boolean restore_flag = FALSE;

    if (submenu == TRUE) {
	testname("Temperature Offset");
    } else {
	prpass(testpass, "Temperature Offset Test");
    }

    if (env_ver < ENV_TEST_VER4P4) {
	cterr('w', 0, "Temperature Offset Test not supported in "
		      "Version %d.%d", env_ver >> ENV_VER_MAJOR_SHIFTS,
		      env_ver & ENV_MCU_REV_MINOR);
	return(PASSED);
    }

    /* Setup I2C API parameter struct */
    /* Get ENV MCU I2C interface structure */
    rc = get_env_mcu_i2c_struct(&i2c_if);
    if (rc != PASSED) {
        printf("%s: Failed to get Power Sequencer I2C structure.\n",
              __FUNCTION__);
        return (rc);
    }
    
    while (tmp_reg_p->reg_name) {
	/* Store original value of the Temperature Offset Register */
	original_temp = 0;
	restore_flag = FALSE;	/* Nothing to restore */
	i2c_if.offset = tmp_reg_p->off;
	i2c_if.buf = (char *)&tmp_off;
	rc = env_read(&i2c_if);
	if (rc != PASSED) {
	    /* Temperature Offset Register read failed */
	    sprintf(err_buf, "%s() %s Offset Register read failed. rc = %#x",
			     __FUNCTION__, tmp_reg_p->reg_name, rc);
	    rc = FAILED;
	} else {
	    /* Got the offset register and will be able to restore */
	    restore_flag = TRUE;
	    /* Read the value of the Temperature Register */
	    i2c_if.offset = tmp_reg_p->temp;
	    i2c_if.buf = (char *)&temp;
	    rc = env_read(&i2c_if);
	    if (rc != PASSED) {
		/* Temperature Register read failed */
		sprintf(err_buf, "%s() %s Temperature Register read failed.  "
				 "rc = %#x", __FUNCTION__, tmp_reg_p->reg_name,
				 rc);
		rc = FAILED;
	    } else {
		/* Add 10 to the Temperature offset register and write back
		 * to the MCU
		 */
		original_temp = (short)tmp_off;
		new_temp = original_temp + ENV_ALERT_TEST_DELTA;
		new_tmp_off = (ren_t)new_temp;
		new_temp = (short)temp;
		max_temp = new_temp - ENV_ALERT_TEST_DELTA + ENV_TEMP_OFF_TOLR;
		min_temp = new_temp - ENV_ALERT_TEST_DELTA - ENV_TEMP_OFF_TOLR;
		i2c_if.offset = tmp_reg_p->off;
		i2c_if.buf = (char *)&new_tmp_off;
		rc = env_write(&i2c_if);
		if (rc != PASSED) {
		    /* Unable to write to offset register */
		    sprintf(err_buf, "%s() %s Offset Register write + %d C "
				     "failed. rc = %#x", __FUNCTION__,
				     tmp_reg_p->reg_name, ENV_ALERT_TEST_DELTA,
				     rc);
		    rc = FAILED;
		} else {
		    /* Wait 3 seconds */
		    msleep(ENV_FORCE_FAN_AL_TIME);

		    /* Read the value of the Temperature Register */
		    i2c_if.offset = tmp_reg_p->temp;
		    i2c_if.buf = (char *)&new_temp_rd;
		    rc = env_read(&i2c_if);
		    if (rc != PASSED) {
			/* New Temperature register read failed */
			sprintf(err_buf, "%s() %s Temperature Register read "
					 "again failed. rc = %#x", __FUNCTION__,
					 tmp_reg_p->reg_name, rc);
			rc = FAILED;
		    } else {
			/* The value should be 10C less than the original
			 * reading. Allow for +/- 3C uncertainty.
			 */
			char_temp = (char)new_temp_rd;
			new_temp = (short)char_temp;
			if ((new_temp > max_temp) || (new_temp < min_temp)) {
			    /* Out of the high temp test range */
			    sprintf(err_buf, "%s() %s + %d C test failed. Read "
					     "%#x. Expect between %#x and %#x",
					     __FUNCTION__, tmp_reg_p->reg_name,
					     ENV_ALERT_TEST_DELTA, new_temp,
					     min_temp, max_temp);
			    rc = FAILED;
			} /* endof if new_temp */
		    } /* endof if new temp read */
		} /* endof if high write */
	    } /* endof if temperature read */
	} /* endof if offset read */

	if (rc == PASSED) {
	    /* Subtract 10 from the original Temperature Offset Register
	     * and write back to the MCU
	     */
	    new_temp = original_temp - ENV_ALERT_TEST_DELTA;
	    new_tmp_off = (ren_t)new_temp;
	    new_temp = (short)temp;
	    max_temp = new_temp + ENV_ALERT_TEST_DELTA + ENV_TEMP_OFF_TOLR;
	    min_temp = new_temp + ENV_ALERT_TEST_DELTA - ENV_TEMP_OFF_TOLR;
	    i2c_if.offset = tmp_reg_p->off;
	    i2c_if.buf = (char *)&new_tmp_off;
	    rc = env_write(&i2c_if);
	    if (rc != PASSED) {
		/* Unable to write to offset register */
		sprintf(err_buf, "%s() %s Offset Register write - %d C failed. "
				 "rc = %#x", __FUNCTION__, tmp_reg_p->reg_name,
				 ENV_ALERT_TEST_DELTA, rc);
		rc = FAILED;
	    } else {
		/* Wait 3 seconds */
		msleep(ENV_FORCE_FAN_AL_TIME);

		/* Read the value of the Temperature Register */
		i2c_if.offset = tmp_reg_p->temp;
		i2c_if.buf = (char *)&new_temp_rd;
		rc = env_read(&i2c_if);
		if (rc != PASSED) {
		    /* New Temperature register read failed */
		    sprintf(err_buf, "%s() %s Temperature Register last read "
				     "failed. rc = %#x", __FUNCTION__,
				     tmp_reg_p->reg_name, rc);
		    rc = FAILED;
		} else {
		    /* The value should be 10C higher than the original
		     * reading. Allow for +/- 3C uncertainty.
		     */
		    char_temp = (char)new_temp_rd;
		    new_temp = (short)char_temp;
		    if ((new_temp > max_temp) || (new_temp < min_temp)) {
			/* Out of the lowh temp test range */
#ifdef ENV_TEMP_OFF_DEBUG
			printf("\n new_temp = %d, max_temp = %d, min_temp = %d,"
			       " original_temp = %d\n",
			       new_temp, max_temp, min_temp, original_temp);
#endif /* ENV_TEMP_OFF_DEBUG */
			sprintf(err_buf, "%s() %s - %d C test failed. Read %#x "
					 "Expect between %#x and %#x",
					 __FUNCTION__, tmp_reg_p->reg_name,
					 ENV_ALERT_TEST_DELTA, new_temp,
					 min_temp, max_temp);
			rc = FAILED;
		    } else {
			/* Passed the test. Write back the original value for
			 * the Temperature Offset Register
			 */
			i2c_if.offset = tmp_reg_p->off;
			i2c_if.buf = (char *)&tmp_off;
			rc = env_write(&i2c_if);
			if (rc != PASSED) {
			    /* Unable to restore offset register */
			    sprintf(err_buf, "%s() Unable to restore %s Offset "
					     "Register. rc = %#x", __FUNCTION__,
					     tmp_reg_p->reg_name, rc);
			    rc = FAILED;
			} else {
			    /* Restore done */
			    restore_flag = FALSE;
			} /* endof if restore */
		    } /* endof if new_temp */
		} /* endof if new temp read */
	    } /* endof if low write */
	} /* endof if rc */

	if (rc != PASSED) {
	    /* Test failed. Out of the loop */
	    break;
	}
	tmp_reg_p++;
    } /* endof while */

    if (rc != PASSED) {
	cterr('f', 0, err_buf);
    }

    if (restore_flag == TRUE) {
	i2c_if.offset = tmp_reg_p->off;
	i2c_if.buf = (char *)&tmp_off;
	rc = env_write(&i2c_if);
	if (rc != PASSED) {
	    /* Unable to restore offset register */
	    sprintf(err_buf, "%s Unable to restore %s offset register. rc = "
			     "%#x", __FUNCTION__, tmp_reg_p->reg_name, rc);
	    cterr('f', 0, err_buf);
	}
    }
    if (submenu == TRUE) {
	prcomplete(testpass, errcount, (char *)0);
    }

    return(rc);
}

/*********************************************************************
 *
 * Function:	hyst_stat_check
 *
 * Description:	Copy the new level temperature to the scratchpad register.
 *		Wait for a given amount of time. Then check status for
 *		an expected fan speed level.
 *
 * Inputs:	i2c_if - I2C interface struct pointer.
 *		hyst_ptr - Points to Hysteresis test struct,
 *		err_buf - Points to the failing text buffer.
 *
 * Outputs:	PASSED/FAILED
 *
 *********************************************************************
 */
static int
hyst_stat_check (n2g_i2c_if_t *i2c_if, env_hyst_test_t *hyst_ptr, char *err_buf)
{
    n2g_i2c_if_t new_i2c_if;
    uint32_t rc;
    int wait_count;
    ren_t stat2, scratchpad;

    /* Copy the I2C struct */
    memcpy(&new_i2c_if, i2c_if, sizeof(n2g_i2c_if_t));
    /* Write the value of new temperature level to the Scratchpad register */
    new_i2c_if.offset = hyst_ptr->offset;
    new_i2c_if.buf = (char *)&scratchpad;

    /* Read the register first */
    rc = env_read(&new_i2c_if);
    if (rc != PASSED) {
	/* Register read failed */
	sprintf(err_buf, "hyst_stat_check() Register %#x read failed. "
			 "rc = %#x", hyst_ptr->offset, rc);
	return(FAILED);
    }

    /* Copy the read register to the Scratchpad register */
    new_i2c_if.offset = ENV_MCU_SCR;
    scratchpad -= hyst_ptr->diff;

    rc = env_write(&new_i2c_if);
    if (rc != PASSED) {
	/* Scratchpad register write failed */
	sprintf(err_buf, "hyst_stat_check() Scratchpad Register write failed. "
			 "copied from %#x. rc = %#x", hyst_ptr->offset, rc);
	return(FAILED);
    }

    /* Wait given amount of time in milliseconds */
    prpass(testpass, "Wait up to %d seconds for register at offset %#x to be "
		     "active",
		     hyst_ptr->wait / MS_PER_SECOND, hyst_ptr->offset);
    for (wait_count = hyst_ptr->wait / MS_PER_SECOND; wait_count;
						      wait_count--) {
	msleep(MS_PER_SECOND);

	/* Check that CURRENT_FAN_STATUS bits in Status 2 to expected level */
	new_i2c_if.offset = ENV_MCU_STA2;
	new_i2c_if.buf = (char *)&stat2;

	rc = env_read(&new_i2c_if);
	if (rc != PASSED) {
	    /* Status 2 register read failed */
	    sprintf(err_buf, "%s() Status 2 read failed. Expect %#x. rc = %#x",
			     __FUNCTION__, hyst_ptr->level, rc);
	    return(FAILED);
	}

	/* Check if the status matches */
	if ((stat2 & ENV_MCU_STAT2_FAN_ST_M) == hyst_ptr->level) {
	    /* Status match */
	    if ((NVRAM)->diagflag & D_VERBOSE) {
		printf("%s Level %#x status match in %d seconds\n",
		       __FUNCTION__, hyst_ptr->level,
		       (hyst_ptr->wait / MS_PER_SECOND) - wait_count + 1);
	    }
	    return(PASSED);
	}
    } /* endof for */

    /* Cannot match the status */
    sprintf(err_buf, "%s(). Expect level %#x. Read level %#x.", __FUNCTION__,
		     hyst_ptr->level, stat2 & ENV_MCU_STAT2_FAN_ST_M);
    return(FAILED);
}

/*********************************************************************
 *
 * Function:	env_get_status
 *
 * Description:	Read Status 1 and Status 2 registers. Also provide failing
 *		text.
 *
 * Inputs:	i2c_if - I2C interface struct pointer.
 *		stat1 - Pointer to the status 1 buffer.
 *		stat2 - Pointer to the status 2 buffer.
 *		err_buf - Points to the resulting failing text.
 *		pre_text_p - Points to the pre-text buffer. NULL if none.
 *
 * Outputs:	PASSED/FAILED
 *
 * Assumptions:	The err_buf is large enough for both the pre-text and added
 *		new text.
 *
 *********************************************************************
 */
static int
env_get_status (n2g_i2c_if_t *i2c_if, ren_t *stat1, ren_t *stat2, char *err_buf,
	       char *pre_text_p)
{
    n2g_i2c_if_t new_i2c_if;
    uint32_t rc;

    /* Copy the I2C struct */
    memcpy(&new_i2c_if, i2c_if, sizeof(n2g_i2c_if_t));
    /* Read status 1 */
    new_i2c_if.offset = ENV_MCU_STA1;
    new_i2c_if.buf = (char *)stat1;

    rc = env_read(&new_i2c_if);
    if (rc != PASSED) {
	/* Status 1 register read failed */
	if (pre_text_p) {
	    /* User provided pretext */
	    sprintf(err_buf, "%s Status 1 register read failed. rc = %#x",
			      pre_text_p, rc);
	} else {
	    /* Pre-text not provided */
	    sprintf(err_buf, "env_get_status() Status 1 register read failed. "
			     "rc = %#x", rc);
	}
	return(FAILED);
    }

    /* Read status 2 */
    new_i2c_if.offset = ENV_MCU_STA2;
    new_i2c_if.buf = (char *)stat2;

    rc = env_read(&new_i2c_if);
    if (rc != PASSED) {
	/* Status 2 register read failed */
	if (pre_text_p) {
	    /* User provided pretext */
	    sprintf(err_buf, "%s Status 2 register read failed. rc = %#x",
			      pre_text_p, rc);
	} else {
	    /* Pre-text not provided */
	    sprintf(err_buf, "env_get_status() Status 2 register read failed. "
			     "rc = %#x", rc);
	}
	return(FAILED);
    } else {
	return(PASSED);
    }
}

/*********************************************************************
 *
 * Function:	env_clear_stat2
 *
 * Description:	Clear Status 2 registers. Also provide failing text.
 *		Status 2 register is any write to clear all. Just in case
 *		it is changed to write 1 to clear. This implementation is
 *		ready.
 *
 * Inputs:	i2c_if - I2C interface struct pointer.
 *		stat2 - Pointer to the status 2 buffer.
 *		err_buf - Points to the resulting failing text.
 *		pre_text_p - Points to the pre-text buffer. NULL if none.
 *
 * Outputs:	PASSED/FAILED
 *
 * Assumptions:	The err_buf is large enough for both the pre-text and added
 *		new text.
 *
 *********************************************************************
 */
static int
env_clear_stat2 (n2g_i2c_if_t *i2c_if, ren_t *stat2, char *err_buf,
		char *pre_text_p)
{
    n2g_i2c_if_t new_i2c_if;
    uint32_t rc;

    /* Copy the I2C struct */
    memcpy(&new_i2c_if, i2c_if, sizeof(n2g_i2c_if_t));
    /* Write to Status 2 register */
    new_i2c_if.offset = ENV_MCU_STA2;
    new_i2c_if.buf = (char *)stat2;

    rc = env_read(&new_i2c_if);
    if (rc == PASSED) {
	rc = env_write(&new_i2c_if);
	if (rc != PASSED) {
	    /* Status 2 register write failed */
	    if (pre_text_p) {
		/* User provided pretext */
		sprintf(err_buf, "%s Status 2 register clear failed. rc = %#x",
				  pre_text_p, rc);
	    } else {
		/* Pre-text not provided */
		sprintf(err_buf, "env_clear_stat2() Status 2 register clear "
				 "failed. rc = %#x", rc);
	    } /* endof if pre_text_p */
	    return(FAILED);
	} else {
	    return(PASSED);
	} /* endof if rc */
    } else {
	/* Read failed */
	if (pre_text_p) {
	    /* User provided pretext */
	    sprintf(err_buf, "%s clear Status 2 register read failed. rc = %#x",
			      pre_text_p, rc);
	} else {
	    /* Pre-text not provided */
	    sprintf(err_buf, "env_clear_stat2() Status 2 register read "
			     "failed. rc = %#x", rc);
	} /* end of if pre_text_p */
	return(FAILED);
    } /* endof if rc */

}

/*********************************************************************
 *
 * Function:	env_regs_access
 *
 * Description:	Save or Restore all registers to a given struct.
 *
 * Inputs:	i2c_if - I2C interface struct pointer.
 *		option - N2G_I2C_READ for save. N2G_I2C_WRITE for restore.
 *		reg_log - Points to Env MCU registers struct.
 *		err_buf - Points to the resulting failing text.
 *
 * Outputs:	PASSED/FAILED
 *
 * Assumptions:	The err_buf is large enough for both the pre-text and added
 *		new text.
 *
 *********************************************************************
 */
static int
env_regs_access (n2g_i2c_if_t *i2c_if, int option, dev_env_mcu_t *reg_log,
		char *err_buf)
{
    n2g_i2c_if_t new_i2c_if;
    reg_info_t *reg_table_p;
    ulong reg_log_addr, i;
    uint32_t rc;

    /* Copy the I2C struct */
    memcpy(&new_i2c_if, i2c_if, sizeof(n2g_i2c_if_t));
    reg_log_addr = (ulong)reg_log;	/* Treat reg_log as register array */

    /* Traverse through MCU register info table to find Read/Write registers */
    for (i = 0, reg_table_p = &env_mcu_table[0];
		i < (sizeof(env_mcu_table) / sizeof(reg_info_t));
		i++, reg_table_p++) {
	if ((reg_table_p->type & (READ_ONLY + WRITE_ONLY)) == READ_WRITE) {
	    /* Read/Write register found */
	    new_i2c_if.offset = reg_table_p->offset;
	    new_i2c_if.buf = (char *)(reg_log_addr +
				      (sizeof(ren_t) * reg_table_p->offset));
#ifdef ENV_WDG_DEBUG
	    printf("log addr = %#x\n", new_i2c_if.buf);
#endif /* ENV_WDG_DEBUG */
	    if (option == N2G_I2C_READ) {
		rc = env_read(&new_i2c_if);
		if (rc != PASSED) {
		    sprintf(err_buf, "env_regs_access() Read %s @ %#x failed. "
				     "rc = %#x", reg_table_p->name,
				     reg_table_p->offset, rc);
		    return(FAILED);
		}
	    } else { /* N2G_I2C_WRITE */
		rc = env_write(&new_i2c_if);
		if (rc != PASSED) {
		    sprintf(err_buf, "env_regs_access() Write %s @%#x "
				     "failed. rc = %#x", reg_table_p->name,
				     reg_table_p->offset, rc);
		    return(FAILED);
		} /* endof if rc */
	    } /* endof if option */
	} /* endof if type */
    } /* endof for */
    return(PASSED);
}

/*******************************************************************************
 *
 * Function   : read_reg
 * Description: write env mcu register.
 * Inputs     : offset - register offset 
 *              d16 - 16 bit value for writing.
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
static int read_reg (int offset, uint16_t *d16)
{
    n2g_i2c_if_t i2c_if;
    uint32_t     result = FAILED;

    /* Setup I2C API parameter struct */
    /* Get ENV MCU I2C interface structure */
    result = get_env_mcu_i2c_struct(&i2c_if);
    if (result != PASSED) {
        printf("%s: Failed to get Power Sequencer I2C structure.\n",
              __FUNCTION__);
        return (result);
    }
    
    i2c_if.buf = (char *)d16;
    i2c_if.size = sizeof(uint16_t);
    i2c_if.offset = offset;
    
    result = env_read(&i2c_if);
    if (result != PASSED) {
        /* Unable to read data */
        cterr('f', 0, "show_reg() Unable to read. rc = 0x%08x", result);
        return (FAILED);
    }

    return (PASSED);
}

/*******************************************************************************
 *
 * Function   : write_reg
 * Description: write env mcu register.
 * Inputs     : offset - register offset 
 *              d16 - 16 bit value for writing. 
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
static int write_reg (int offset, uint16_t d16)
{
    n2g_i2c_if_t i2c_if;
    uint32_t     result = FAILED;

    /* Setup I2C API parameter struct */
    /* Get ENV MCU I2C interface structure */
    result = get_env_mcu_i2c_struct(&i2c_if);
    if (result != PASSED) {
        printf("%s: Failed to get Power Sequencer I2C structure.\n",
              __FUNCTION__);
        return (result);
    }
    
    i2c_if.buf = (char *)&d16;
    i2c_if.size = sizeof(uint16_t);
    i2c_if.offset = offset;
    
    result = env_write(&i2c_if);   

    if (result != PASSED) {
        /* Unable to read data */
        cterr('f', 0, "%s: Unable to read. rc = 0x%08x", __FUNCTION__, result);
        return (FAILED);
    }
         
    return (PASSED);
}

/*******************************************************************************
 *
 * Function   : read_env_reg
 * Description: read env mcu register which is selected by user.
 * Inputs     : dummy - useless.
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
static int read_env_reg (int dummy)
{
    int result = FAILED;
    uint32_t offset;
    uint16_t d16;

    offset = gethex_answer("enter offset ", 0x2f, 0, 0xFF);
    result = read_reg(offset, &d16);
    if (result == FAILED) {
        printf("%s: error reading reg\n", __FUNCTION__);
    }

    printf("@%#x = 0x%04x\n", offset, d16);

    return result;
}

/*******************************************************************************
 *
 * Function   : write_env_reg
 * Description: query user and write env mcu register. 
 * Inputs     : dummy - useless.
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
static int write_env_reg (int dummy)
{
    uint32_t offset, d16;
    int32_t result;
    
    offset = gethex_answer("enter offset ", 0x2f, 0, 0xFF);
    d16 = gethex_answer("enter 16-bit data (ie, 0x1234)", 0x1234, 0, 0xFFFF);

    result = write_reg(offset, d16);
    if (result == FAILED) {
        printf("%s: error reading reg\n", __FUNCTION__);
    }
    return result;
}

/*******************************************************************************
 *
 * Function   : slow_fan_speed
 * Description: to slow down fan speed.
 * Inputs     : dummy - useless.
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
static int slow_fan_speed (int dummy)
{
    uint16_t d16 = 0x8fbd;
    if (write_reg(0xe, d16) == FAILED) {
        return FAILED;
    }

    return PASSED;
}


/*******************************************************************************
 *
 * Function   : show_all_fans_speed
 * Description:	To show all FANs' speed.
 * Inputs     : None
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
int show_all_fans_speed (int opt) {
    n2g_i2c_if_t i2c_if;
    uint rc = FAILED;

    /* Get ENV MCU I2C interface structure */
    rc = get_env_mcu_i2c_struct(&i2c_if);
    if (rc != PASSED) {
        printf("*** %s:%d Failed to get ENV MCU I2C structure.\n",
               __FUNCTION__, __LINE__);
        return (rc);
    }
    
    /* Display Fans speeds */
    rc = show_fan_speed(&i2c_if, FALSE, 2);
    if (rc == FAILED) {
        printf("*** %s:%d Failed to show FANs' speed.\n",
               __FUNCTION__, __LINE__);
    }
    return (rc);
}

/*******************************************************************************
 *
 * Function   : env_get_version
 * Description: read env mcu version 
 * Inputs     : version
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
int env_get_version (uint16_t *version)
{
    if ((read_reg(0, version)) != RC_I2C_OP_OK) {
        return FAILED;
    }
    return PASSED;
}

/*------------------------------------------------------------------
$Log: platform_env.c,v $
Revision 1.5  2017/07/10 02:51:58  leschen
Remove unused variable

Revision 1.4  2014/04/03 01:17:07  alpeng
O2: 4 fans; Juno: 3 fans

Revision 1.3  2013/11/26 08:40:36  hroni
fix compiler warning

Revision 1.2  2013/10/08 11:14:27  erwu2
enhanced err msg first check-in

Revision 1.1  2013/05/09 05:42:37  alpeng
moving overlord common code from x86

Revision 1.10  2012/11/28 18:19:09  palin2
To make I2C utilities SubMenu more intuitive.

Revision 1.9  2012/11/16 00:31:14  mcharon
in env_write, dont't change content of i2c_if->buf

Revision 1.8  2012/11/07 10:58:16  alpeng
remove useless file and clean up code

Revision 1.7  2012/08/22 02:28:41  palin2
Add Cavium CPU temperature display in Overlord Diag boot-up message.

Revision 1.6  2012/07/24 18:48:22  mcharon
fix read_write_reg function

Revision 1.5  2012/06/28 06:11:38  palin2
Change register dump display format.

Revision 1.4  2012/06/26 12:18:42  palin2
Support to show the Power consumption of Overlord motherboard.

Revision 1.3  2012/05/30 16:45:03  palin2
Clean up compile warnings.

Revision 1.2  2012/03/28 00:38:23  mcharon
remove forward slash from second line

Revision 1.1.1.1  2012/03/23 23:02:09  ptong
Initial archive of ng_diag module


$Endlog$
*/
