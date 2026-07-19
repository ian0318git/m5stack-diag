/* $Id: diag.c,v 1.8 2015/03/31 06:25:20 leschen Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/woodlawn_linux/diag.c,v $
 *-----------------------------------------------------------------------------
 * diag.c - Menus for Woodlawn Cavium data plane
 *
 * January 2012, Kody Ko
 * Copyright (c) 2015 by Cisco Systems, Inc.
 * All rights reserved.
 *-----------------------------------------------------------------------------
 */
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include "common.h"
#include "types.h"
#include "menu.h"
#include "proto.h"
#include "error.h"
#include "nvsysvars.h"
#include "diag_fpga_lib.h"
#include "diag_xaui_88X3120_test.h"
#include "diag_xaui_88X2222M_test.h"
#include "diag_bootflash_test.h"
#include "diag_ge_phy_88E1340_test.h"
#include "diag_ge_phy_88E1548L_test.h"
#include "diag_temperature_sensor_test.h"
#include "diag_fpga_test.h"
#include "diag_pll_test.h"
#include "platform_xaui.h"
#include "diag_tlk10232_test.h"
#include "diag_backplane_xaui_test.h"
#include "diag_ge_phy_88E1548L_lib.h"
#include "dev_mrvl_ge.h"
#include "diag_ge_phy_88E1112C_test.h"
#include "platform_eth.h"
#include "diag_xaui_88X2222M_lib.h"
#include "sm_woodlawn_comm.h"
#include "diag_tlk10232_lib.h"

/* M/B test flag defines */
#define MF_1    (MF_CONTINUOUS | MF_DOGRP)
#define MF_2    (MF_1 | MF_DOALL)
#define MF_3    (MF_2 | MF_SHOW_ERRCOUNT)
#define MF_4    (MF_1 | MF_SHOW_ERRCOUNT)

#define DIAG_RTN_STS_TMP_FILE                    "/tmp/woodlawn.status"
#define BP_XAUI_IP_ADDR                          "192.123.123.1"
#define BP_PASS_STR                              "PASS"
#define BP_FAIL_STR                              "FAIL"
#define PING_HOST_TOUT                           (180)
#define PLATFORM_IP_ADDR_SUBNET                  "192.123.123"

static int xaui_line_loopback_test(void);
static int has_88E1112C(void);
static int has_88x2222(void);
static void diag_report_status_host(char *);
static void doall_print_head(char *);
static void doall_print_tail(char *);
static long diag_voltmarg_display_wrapper(char *);
static long diag_voltmarg_set_wrapper(char *);
static long diag_led_set_wrapper(char *);
static long diag_1340_reg_read_wrapper(char *);
static long diag_1340_reg_write_wrapper(char *);
static long diag_1548_reg_read_wrapper(char *);
static long diag_1548_reg_write_wrapper(char *);
static long diag_1548_ptp_reg_read_wrapper(char *);
static long diag_1548_ptp_reg_write_wrapper(char *);
static long diag_2222_reg_read_wrapper(char *);
static long diag_2222_reg_write_wrapper(char *);
static long diag_2222_ptp_reg_read_wrapper(char *);
static long diag_2222_ptp_reg_write_wrapper(char *);
static long diag_fpga_reg_read_wrapper(char *);
static long diag_fpga_reg_write_wrapper(char *);
static long diag_10232_reg_read_wrapper(char *);
static long diag_10232_reg_write_wrapper(char *);
static long diag_cavium_sgmii_status_wrapper(char *);
static long diag_cavium_sgmii_config_wrapper(char *);
static long diag_cavium_sgmii_reg_read_wrapper(char *);
static long diag_cavium_xaui_status_wrapper(char *);
static long diag_cavium_xaui_gmx_reg_read_wrapper(char *);
static long diag_cavium_xaui_pcs_reg_read_wrapper(char *);
static long diag_bootflash_get_info_wrapper(char *); 
static long diag_bootflash_otp_test_wrapper(char *);
static long diag_1548_drift_adjustment_wrapper(char *);
static long diag_enable_1548_ptp_engine_wrapper(char *);
static long diag_1548_clk_trig_in_wrapper(char *);
static long diag_2222_clk_trig_in_wrapper(char *);
static long diag_config_1548_gen_clk_wrapper(char *);
static long diag_config_2222_gen_clk_wrapper(char *);
static long diag_config_1548_gen_trig_wrapper(char *);
static long diag_config_2222_gen_trig_wrapper(char *);
static long diag_config_fpga_clk_mux_ge0_wrapper(char *);
static long diag_config_fpga_clk_mux_ge1_wrapper(char *);
static long diag_config_fpga_trig_mux_ge0_wrapper(char *);
static long diag_config_fpga_trig_mux_ge1_wrapper(char *);
static long diag_config_fpga_clk_mux_x2222p_wrapper(char *);
static long diag_config_fpga_trig_mux_x2222p_wrapper(char *);
static long diag_verify_fpga_sync_clk_out_wrapper(char *);
static long diag_verify_fpga_sync_trig_out_wrapper(char *);
int ping_host(void);

/*
 *  Externs
 */
int netflashbooted = 1;

/* for utilities submenu. */
extern int  memtest();
extern int  memloop();
extern int  memdebug();
extern int  map_mem_util();
extern int  addrloop();
extern int dump_sfp_eeprom(int);

/* for main menu */
extern int main_mem_test(void);
extern int eth_port_test_main(int);

extern int dump_phy_88E1340_registers(void);
extern int alter_phy_88E1340_register(void);
extern int dump_phy_88E1548L_registers(void);
extern int alter_phy_88E1548L_register(void);
extern int dump_phy_88E1548_ptp_reg(void);
extern int alter_phy_88E1548_ptp_reg(void);
extern int dump_phy_88X2222M_registers(void);
extern int alter_phy_88X2222M_register(void);
extern int dump_phy_88X2222M_ptp_register(void);
extern int alter_phy_88X2222M_ptp_register(void);
extern int display_fpga_regs(void);
extern int alter_fpga_regs(void);
extern int setup_1548_ptp_engine(void);
int dump_tlk_10232_registers(void);
int alter_tlk_10232_register(void);

/* for multicore utilities menu */
extern int release_core_num_reset(int);
extern int display_core_status(int);
extern int display_mbox_setx_regs (void);
extern int reset_core_num(int);
extern int core0_and_coreX_main_test(int);

/* for debug menu */
extern void print_smi_reg(void);
extern void print_cavium_intr_info(void);
extern void print_cavium_status_reg(void);
extern void print_cavium_cause_reg(void);
extern void print_cavium_desave_reg(void);
extern void print_cavium_prid_reg(void);
extern void write_cavium_cop0_reg(void);
extern void print_cavium_msi_regs(void);
extern int test_malloc_nm(void);
extern int test_malloc(void);
extern void cvmx_dump_npei(int);
extern void cvmx_dump_pesc(int);
extern void cvmx_dump_cfg(int);
extern void test_delay(void);
extern int set_cavium_gpio_pin5 (void);
extern int build_plat_dimm_util_menu(int);
extern int test_malloc_dev(void);
extern int pca9557_power_margin_ctrl(void);
extern int pca9557_power_margin_status(void);
/*
 *  Globals  
 */

/* NC Command Table */
static struct nc_command nc_cmd_items[] = {
    {DIAG_COMMAND_VOLTAGE_MARGIN_SET,       diag_voltmarg_set_wrapper},
    {DIAG_COMMAND_VOLTAGE_MARGIN_GET,       diag_voltmarg_display_wrapper},
    {DIAG_COMMAND_LED_SET,                  diag_led_set_wrapper},
    {DIAG_COMMAND_1340_REG_READ,            diag_1340_reg_read_wrapper},
    {DIAG_COMMAND_1340_REG_WRITE,           diag_1340_reg_write_wrapper},
    {DIAG_COMMAND_1548_REG_READ,            diag_1548_reg_read_wrapper},
    {DIAG_COMMAND_1548_REG_WRITE,           diag_1548_reg_write_wrapper},
    {DIAG_COMMAND_1548_PTP_REG_READ,        diag_1548_ptp_reg_read_wrapper},
    {DIAG_COMMAND_1548_PTP_REG_WRITE,       diag_1548_ptp_reg_write_wrapper},
    {DIAG_COMMAND_2222_REG_READ,            diag_2222_reg_read_wrapper},
    {DIAG_COMMAND_2222_REG_WRITE,           diag_2222_reg_write_wrapper},
    {DIAG_COMMAND_2222_PTP_REG_READ,        diag_2222_ptp_reg_read_wrapper},
    {DIAG_COMMAND_2222_PTP_REG_WRITE,       diag_2222_ptp_reg_write_wrapper},
    {DIAG_COMMAND_SM_FPGA_REG_READ,         diag_fpga_reg_read_wrapper},
    {DIAG_COMMAND_SM_FPGA_REG_WRITE,        diag_fpga_reg_write_wrapper},
    {DIAG_COMMAND_10232_REG_READ,           diag_10232_reg_read_wrapper},
    {DIAG_COMMAND_10232_REG_WRITE,          diag_10232_reg_write_wrapper},
    {DIAG_COMMAND_CAVIUM_SGMII_PORT_STATUS, diag_cavium_sgmii_status_wrapper}, 
    {DIAG_COMMAND_CAVIUM_SGMII_PORT_CONFIG, diag_cavium_sgmii_config_wrapper},
    {DIAG_COMMAND_CAVIUM_SGMII_REG_READ,    diag_cavium_sgmii_reg_read_wrapper},
    {DIAG_COMMAND_CAVIUM_XAUI_PORT_STATUS,  diag_cavium_xaui_status_wrapper},
    {DIAG_COMMAND_CAVIUM_XAUI_GMX_REG_READ, diag_cavium_xaui_gmx_reg_read_wrapper},
    {DIAG_COMMAND_CAVIUM_XAUI_PCS_REG_READ, diag_cavium_xaui_pcs_reg_read_wrapper},
    {DIAG_COMMAND_BOOTFLASH_GET_INFO,       diag_bootflash_get_info_wrapper},
    {DIAG_COMMAND_BOOTFLASH_OTP_TEST,       diag_bootflash_otp_test_wrapper},
    {DIAG_COMMAND_VERIFY_1548_PTP,          diag_1548_drift_adjustment_wrapper},
    {DIAG_COMMAND_ENABLE_1548_PTP_ENGINE,   diag_enable_1548_ptp_engine_wrapper},
    {DIAG_COMMAND_VERIFY_1548_CLK_TRIG_IN,  diag_1548_clk_trig_in_wrapper},
    {DIAG_COMMAND_VERIFY_2222_CLK_TRIG_IN,  diag_2222_clk_trig_in_wrapper},
    {DIAG_COMMAND_CONFIG_1548_GEN_CLK_OUT,  diag_config_1548_gen_clk_wrapper},
    {DIAG_COMMAND_CONFIG_2222_GEN_CLK_OUT,  diag_config_2222_gen_clk_wrapper},
    {DIAG_COMMAND_CONFIG_1548_GEN_TRIG_OUT, diag_config_1548_gen_trig_wrapper},
    {DIAG_COMMAND_CONFIG_2222_GEN_TRIG_OUT, diag_config_2222_gen_trig_wrapper},
    {DIAG_COMMAND_CONFIG_FPGA_CLK_MUX_GE0,  diag_config_fpga_clk_mux_ge0_wrapper},
    {DIAG_COMMAND_CONFIG_FPGA_CLK_MUX_GE1,  diag_config_fpga_clk_mux_ge1_wrapper},
    {DIAG_COMMAND_CONFIG_FPGA_TRIG_MUX_GE0, diag_config_fpga_trig_mux_ge0_wrapper},
    {DIAG_COMMAND_CONFIG_FPGA_TRIG_MUX_GE1, diag_config_fpga_trig_mux_ge1_wrapper},
    {DIAG_COMMAND_CONFIG_FPGA_CLK_MUX_X2222P,   diag_config_fpga_clk_mux_x2222p_wrapper},
    {DIAG_COMMAND_CONFIG_FPGA_TRIG_MUX_X2222P,  diag_config_fpga_trig_mux_x2222p_wrapper},
    {DIAG_COMMAND_VERIFY_FPGA_SYNC_CLK_OUT,      diag_verify_fpga_sync_clk_out_wrapper},
    {DIAG_COMMAND_VERIFY_FPGA_SYNC_TRIG_OUT,     diag_verify_fpga_sync_trig_out_wrapper},
};

#define NC_CMDS_SIZE        (sizeof(nc_cmd_items)/sizeof(struct nc_command))

#ifdef WOODLAWN_SUPPORT_THESE
/*
 * Debug menu utility
 */
static struct mitem debug_items[] = {
#ifdef LINUX_APP
    {"display SMI registers",   0,0, (type_t(*)())print_smi_reg,    (type_t *)&one,  0, (type_t(*)())0, 0},
    {"display CPU PRID register",   0,0, (type_t(*)())print_cavium_prid_reg,    (type_t *)&one,  0, (type_t(*)())0, 0},
    {"display CPU Status register",  0,0, (type_t(*)())print_cavium_status_reg,  (type_t *)&one,  0, (type_t(*)())0, 0},
    {"display CPU Cause register",   0,0, (type_t(*)())print_cavium_cause_reg,   (type_t *)&one,  0, (type_t(*)())0, 0},
    {"display CPU DESAVE register",  0,0, (type_t(*)())print_cavium_desave_reg,  (type_t *)&one,  0, (type_t(*)())0, 0},
    {"write CPU COP0 register",      0,0, (type_t(*)())write_cavium_cop0_reg,    (type_t *)&one,  0, (type_t(*)())0, 0},
#ifdef NOT_CAVIUM_CN68XX
    {"display CSR CIU registers",    0,0, (type_t(*)())print_cavium_intr_info,   (type_t *)&one,  0, (type_t(*)())0, 0},
    {"display npei",                 0,0, (type_t(*)())cvmx_dump_npei,           (type_t *)&zero, 0, (type_t(*)())0, 0},
#endif
    {"display PCIe MSI registers",   0,0, (type_t(*)())print_cavium_msi_regs,    (type_t *)&one,  0, (type_t(*)())0, 0},
    {"display octeon cfg reg pcie_port 0",   0,0, (type_t(*)())cvmx_dump_cfg,    (type_t *)&zero, 0, (type_t(*)())0, 0},
    {"display octeon cfg reg pcie_port 1",   0,0, (type_t(*)())cvmx_dump_cfg,    (type_t *)&one,  0, (type_t(*)())0, 0},
    {"display octeon pesc pcie_port 0",   0,0, (type_t(*)())cvmx_dump_pesc,      (type_t *)&zero, 0, (type_t(*)())0, 0},
    {"display octeon pesc pcie_port 1",   0,0, (type_t(*)())cvmx_dump_pesc,      (type_t *)&one,  0, (type_t(*)())0, 0},
    {"test malloc_dev",              0,0, (type_t(*)())test_malloc_dev,          (type_t *)&one,  0, (type_t(*)())0, 0},
    {"test malloc",                  0,0, (type_t(*)())test_malloc,              (type_t *)&one,  0, (type_t(*)())0, 0},
    {"set cavium gpio pin5",         0,0, (type_t(*)())set_cavium_gpio_pin5,     (type_t *)&one,  0, (type_t(*)())0, 0},
#endif /*linux_app */
};

static struct menuinfo debug_menu = {
    "Debug utility Menu",
    0,
    0,
    0,
    sizeof(debug_items)/sizeof(struct mitem),
    debug_items,
};
static struct menuinfo *debug_menup = &debug_menu;
#endif

#ifdef WOODLAWN_SUPPORT_THESE
/*
 * Multi Core menu utility
 */
static struct mitem multicore_items[] = {
#ifndef LINUX_APP
    {"Display Core info/status",       0,0,(PFT)display_core_status,         (type_t *)&one,    0,   (type_t(*)())0, 0},
    {"Display mbox setx regs",         0,0,(PFT)display_mbox_setx_regs,      (type_t *)&one,    0,   (type_t(*)())0, 0},
    {"Full (Core0 - Core1) test",      0,0,(PFT)core0_and_coreX_main_test,   (type_t *)&one,    0,   (type_t(*)())0, 0},
    {"Full (Core0 - Core2) test",      0,0,(PFT)core0_and_coreX_main_test,   (type_t *)&two,    0,   (type_t(*)())0, 0},
    {"Full (Core0 - Core3) test",      0,0,(PFT)core0_and_coreX_main_test,   (type_t *)&three,  0,   (type_t(*)())0, 0},
    {"Take 1st core out of reset",     0,0,(PFT)release_core_num_reset,      (type_t *)&one,    0,   (type_t(*)())0, 0},
    {"Take 2nd core out of reset",     0,0,(PFT)release_core_num_reset,      (type_t *)&two,    0,   (type_t(*)())0, 0},
    {"Take 3th core out of reset",     0,0,(PFT)release_core_num_reset,      (type_t *)&three,  0,   (type_t(*)())0, 0},
    {"Reset Core 1 (hold in reset)",   0,0,(PFT)reset_core_num,              (type_t *)&one,    0,   (type_t(*)())0, 0},
    {"Reset Core 2 (hold in reset)",   0,0,(PFT)reset_core_num,              (type_t *)&two,    0,   (type_t(*)())0, 0},
    {"Reset Core 3 (hold in reset)",   0,0,(PFT)reset_core_num,              (type_t *)&three,  0,   (type_t(*)())0, 0},
#endif /* linux_app */
};

static struct menuinfo multicore_menu = {
    "Multi Core utility Menu",
    0,
    0,
    0,
    sizeof(multicore_items)/sizeof(struct mitem),
    multicore_items,
};
static struct menuinfo *multicore_menup = &multicore_menu;

/*
 * Cookie menu utility
 */
static struct mitem cookie_items[] = {
  //    {"alter MB CPU cookie",                   0,0, (type_t(*)())alter_mb_cookie,        (type_t *)&one, 0,(type_t(*)())0, 0},
};

static struct menuinfo cookie_menu = {
    "Cookie utility Menu",
    0,
    0,
    0,
    sizeof(cookie_items)/sizeof(struct mitem),
    cookie_items,
};
static struct menuinfo *cookie_menup = &cookie_menu;
#endif
/*
 * Basic utilities menu.
 */
static struct mitem utilmenuitems[] = {
    {"alter memory",                0,0,(PFT)alt_mem,                   (type_t *)&one,            0,            (type_t(*)())0, 0},
    {"compare memory block",        0,0,(PFT)cmp_mem,                   (type_t *)&one,            0,            (type_t(*)())0, 0},
    {"move memory block",           0,0,(PFT)mov_mem,                   (type_t *)&one,            0,            (type_t(*)())0, 0},
    {"display memory",              0,0,(PFT)dis_mem,                   (type_t *)&one,            0,            (type_t(*)())0, 0},
    {"fill memory",                 0,0,(PFT)fil_mem,                   (type_t *)&one,            0,            (type_t(*)())0, 0},
#ifdef WOODLAWN_SUPPORT_THESE
    {"cookie utility",              0,0,(PFT)menu,                 (type_t *)&cookie_menup,   0,            (type_t(*)())0, 0},
    {"debug utility",               0,0,(PFT)menu,                 (type_t *)&debug_menup,    0,            (type_t(*)())0, 0},
    //{"Octeon CPU BIST check",       0,0,(PFT)check_octeon_bist_results, (type_t *)&one,        0,            (type_t(*)())0, 0},
    {"Multi Core utility",          0,0,(PFT)menu,                 (type_t *)&multicore_menup,0,            (type_t(*)())0, 0},
    //{"Reset Octeon",	            0,0,(PFT)reset_octeon,         (type_t *)&one,             0,            (type_t(*)())0, 0},
    {"DIMM utility",                0,0,(PFT)build_plat_dimm_util_menu, (type_t *)&one,        0,            (type_t(*)())0, 0},
    {"Mux utility",                 0,0,(PFT)build_mux_menu,            (type_t *)&zero,       0,            (type_t(*)())0, 0},
#endif
    {"Show SGMII port status",      0,0,(PFT)display_sgmii_port_stats,     (type_t *)&zero,        0,            (type_t(*)())0, 0},
    {"Show SGMII port config",      0,0,(PFT)display_sgmii_port_cfg,     (type_t *)&zero,        0,            (type_t(*)())0, 0},
    {"Show SGMII PHY registers",    0,0,(PFT)sgmii_phy_reg_dump,     (type_t *)&zero,        0,            (type_t(*)())0, 0},
    {"Show SMI regs",               0,0,(PFT)smi_ctl_reg_dump,     (type_t *)&zero,        0,            (type_t(*)())0, 0},
    {"Access SGMII PHY registers",  0,0,(PFT)woodlawn_phy_reg_access,     (type_t *)&zero,        0,            (type_t(*)())0, 0},
    {"Linux SMI Access SGMII PHY registers",0,0,(PFT)phy_reg_access,     (type_t *)&zero,        0,            (type_t(*)())0, 0},
    {"Change XAUI internal loopback",0,0,(PFT)xaui_int_lpbk_util,     (type_t *)&zero,        0,            (type_t(*)())0, 0},
    {"Show XAUI port status",       0,0,(PFT)display_xaui_port_status,     (type_t *)&zero,        0,            (type_t(*)())0, 0},
    {"Show XAUI GMX regs",          0,0,(PFT)dump_xaui_gmx_regs,     (type_t *)&zero,        0,            (type_t(*)())0, 0},
    {"Show XAUI PCS regs",          0,0,(PFT)dump_xaui_pcs_regs,     (type_t *)&zero,        0,            (type_t(*)())0, 0},
    {"SFP0 EEPROM Display",         0,0,(PFT)dump_sfp_eeprom,     (type_t *)&zero,        0,            (type_t(*)())0, 0},
    {"SFP1 EEPROM Display",         0,0,(PFT)dump_sfp_eeprom,     (type_t *)&one,        0,            (type_t(*)())0, 0},
    {"SFP2 EEPROM Display",         0,0,(PFT)dump_sfp_eeprom,     (type_t *)&two,        0,            (type_t(*)())0, 0},
    {"SFP3 EEPROM Display",         0,0,(PFT)dump_sfp_eeprom,     (type_t *)&three,      0,            (type_t(*)())0, 0},
    {"SFP4 EEPROM Display",         0,0,(PFT)dump_sfp_eeprom,     (type_t *)&four,       0,            (type_t(*)())0, 0},
    {"SFP5 EEPROM Display",         0,0,(PFT)dump_sfp_eeprom,     (type_t *)&five,       0,            (type_t(*)())0, 0},
    {"SFP+ EEPROM Display",         0,0,(PFT)dump_sfp_eeprom,     (type_t *)&six,       0,            (type_t(*)())0, 0},
    {"QLM2 XAUI loopback mode",     0,0,(PFT)xaui_line_loopback_test,     (type_t *)&zero,       0,    (type_t(*)())0, 0},
    {"PCA9557 Power Margin Control",     0,0,(PFT)pca9557_power_margin_ctrl,     (type_t *)&zero,       0,    (type_t(*)())0, 0},
    {"PCA9557 Power Margin Status",     0,0,(PFT)pca9557_power_margin_status,     (type_t *)&zero,       0,    (type_t(*)())0, 0},
    {"Bootflash Get Information",     0,0,(PFT)get_bootflash_info,     (type_t *)&zero,       0,    (type_t(*)())0, 0},
    {"Bootflash OTP Verification",     0,0,(PFT)bootflash_otp_test,     (type_t *)&zero,       0,    (type_t(*)())0, 0},
};

static struct menuinfo utilmenu = {
    "Diagnostic Utilities Menu",
    0,
    0,
    0,
    sizeof(utilmenuitems)/sizeof(struct mitem),
    utilmenuitems,
};
struct menuinfo *utilmenup = &utilmenu;

/*
 * Main Menu.
 */
submenu_xtable_t main_menu_table[] = {
    {"Memory test",          (PFT)main_mem_test,   0,
       MF_3, (type_t(*)())0, 0, (type_t(*)())0,   0},
    {"Bootflash test",    (PFT)bootflash_test,    0,
       MF_3,    (type_t(*)())0, 0, (type_t(*)())0,   0},
    {"Temperature sensor test",     (PFT)temperature_sensor_test,    FALSE,
       MF_2,    (type_t(*)())0, 0, (type_t(*)())temperature_sensor_test,   TRUE},
    {"GE PHY 88E1340 test",         (PFT)ge_phy_88E1340_test,    FALSE,
       MF_2,    (type_t(*)())0, 0, (type_t(*)())ge_phy_88E1340_test,   TRUE},
    {"GE PHY 88E1548L test",        (PFT)ge_phy_88E1548L_test,    FALSE,
       MF_2,    (type_t(*)())0, 0, (type_t(*)())ge_phy_88E1548L_test,   TRUE},
    {"XAUI 88X2222M test",         (PFT)xaui_88X2222M_test,    FALSE,
       MF_2,    (type_t(*)())has_88x2222, 0, (type_t(*)())xaui_88X2222M_test,   TRUE},
    {"GE PHY 88E1112C test",        (PFT)ge_phy_88E1112C_test,    FALSE,
       MF_2,    (type_t(*)())has_88E1112C, 0, (type_t(*)())ge_phy_88E1112C_test,   TRUE},
    {"TLK 10232 test",         (PFT)tlk_10232_test,    FALSE,
       MF_2,    (type_t(*)())0, 0, (type_t(*)())tlk_10232_test,   TRUE},
    {"GE Backplane Loopback Test", (type_t(*)())ge_phy_88E1112C_loopback_test,   LOOPBACK_SFP,
       MF_3, (type_t(*)())has_88E1112C, 0, (type_t(*)())0,   0},
    {"XAUI backplane loopback test",          (PFT)xaui_backplane_loopback_test,     2,
       MF_3,   (type_t(*)())0, 0, (type_t(*)())0,   0},
    {"FPGA test",     (PFT)fpga_test,    FALSE,
       MF_2, (type_t(*)())0, 0, (type_t(*)())fpga_test, TRUE},
};

#define MAIN_MENU_TABLE_SIZE \
        (sizeof(main_menu_table) / sizeof(submenu_xtable_t))
        
/* 
 * Primary & secondary submenu items (filled in from xtable)
 */
static mitem_t main_menu_primary_items[MAIN_MENU_TABLE_SIZE + MAX_BASE_ITEMS];
static mitem_t main_menu_secondary_items[MAIN_MENU_TABLE_SIZE + MAX_BASE_ITEMS];

static struct menuinfo maindiag = {
  "Woodlawn %s",                        /* title */
  0,                                /* title string added by init_empty_menu */
  (PFT)menu_show_dflags,            /* shows major flags */
  0,                                /* generic prompt */
  0,                                /* size -- bumped by add_menu_item() */
  main_menu_primary_items,
};
static struct menuinfo *maindiagp = &maindiag;

/*
 * The menu driven diagnostics are called from either the low level
 * power-on routine or from the monitor.  Frommon will be non-zero if
 * called from the monitor.  Note that this value is actually the
 * argc parameter (a little slight of hand here), which is always
 * greater than zero.
 *
 * The main menu is now defined in an _xtable_.  Both the primary items
 * and the secondary (shadow) items are built with function calls that
 * operate on it and insert the appropriate base items into the menu.
 */
void
diag_menu (int argc, char *argv[])
{
    char arg;

    if(argc > 1) arg = *argv[1];
    else arg = 0;
    build_primary_submenu(main_menu_table, MAIN_MENU_TABLE_SIZE, dgmenustr,
                          &maindiagp);
    build_secondary_submenu(main_menu_table, MAIN_MENU_TABLE_SIZE,
                            main_menu_secondary_items);
    menu(&maindiag, main_menu_secondary_items, arg);
}


/******************************************************************************
 *
 * Function: diag_nc_dispatch_comm
 *
 * Description: This function reads the command transferred via nc command
 *              and execute the function accordingly.
 * Format: opcode,option
 * (comma as delimiter)
 * Inputs      : None
 * Outputs     : None
 *
 *****************************************************************************/
void diag_nc_dispatch_comm (void)
{
    FILE *fp;
    char buff[256];
    char *token;
    char cmd[32];
    int ix;

    /* Delete command temp file */
    sprintf(cmd, "rm -f %s", DIAG_COMMAND_DISPATCH_FILE);
    system(cmd);

    /* Retrieve the command from host side */
    sprintf(cmd, "nc %s %d > %s", BP_XAUI_IP_ADDR,
            DIAG_EXECUTE_COMMAND_TRANSFER_PORT_BASE, DIAG_COMMAND_DISPATCH_FILE);
    system(cmd);

    fp = fopen(DIAG_COMMAND_DISPATCH_FILE, "r");
    if (fp == NULL) {
        printf("%s: Open %s fails\n", __FUNCTION__, DIAG_COMMAND_DISPATCH_FILE);
        return;
    }

    if (fgets(buff, sizeof(buff), fp) == NULL) {
        printf("Nothing in buffer\n");
        goto __exit;
    }

    /* Get the NC command */
    token = strtok(buff, ",");

    for (ix = 0; ix < NC_CMDS_SIZE; ix++) {
        if (!strcmp(nc_cmd_items[ix].cmd_str, token)) {
            token = strtok(NULL, ",");
            nc_cmd_items[ix].func(token);
        }
    }

__exit:
    fclose(fp);
}


/******************************************************************************
 *
 * Function: diag_do_all
 *
 * Description: This function performs all tests
 *
 * Inputs      : None
 * Outputs     : PASSED/FAILED
 *
 *****************************************************************************/
int diag_do_all (void)
{
    doall_print_head("Memory");
    if (main_mem_test() == FAILED) {
        diag_report_status_host(BP_FAIL_STR);
        cterr('f', 0, "Main Memory Test Fails");
        return (FAILED);
    }
    doall_print_tail("Memory");

    doall_print_head("Boot Flash");
    if (bootflash_test() == FAILED) {
        diag_report_status_host(BP_FAIL_STR);
        cterr('f', 0, "Boot Flash Test Fails");
        return (FAILED);
    }
    doall_print_tail("Boot Flash");

    doall_print_head("FPGA");
    if (fpga_do_all_wrapper() == FAILED) {
        diag_report_status_host(BP_FAIL_STR);
        cterr('f', 0, "FPGA Test Fails");
        return (FAILED);
    }
    doall_print_tail("FPGA");

    doall_print_head("Temperature Sensor");
    if (temp_sensor_do_all_wrapper() == FAILED) {
        diag_report_status_host(BP_FAIL_STR);
        cterr('f', 0, "Temperature Sensor Test Fails");
        return (FAILED);
    }
    doall_print_tail("Temperature Sensor");


    doall_print_head("88E1340");
    if (ge_88E1340_do_all_wrapper() == FAILED) {
        diag_report_status_host(BP_FAIL_STR);
        cterr('f', 0, "GE PHY 88E1340 Test Fails");
        return (FAILED);
    }
    doall_print_tail("88E1340");

    doall_print_head("88E1548");
    if (ge_88E1548_do_all_wrapper() == FAILED) {
        diag_report_status_host(BP_FAIL_STR);
        cterr('f', 0, "GE PHY 88E1548 Test Fails");
        return (FAILED);
    }
    doall_print_tail("88E1548");

    if (has_88x2222()) {
        doall_print_head("88X2222M");
        if (xaui_88X222M_do_all_wrapper() == FAILED) {
            diag_report_status_host(BP_FAIL_STR);
            cterr('f', 0, "XAUI 88X222M Test Fails");
            return (FAILED);
        }
        doall_print_tail("88X2222M");
    }

    doall_print_head("88E1112C");
    if (ge_88E1112C_do_all_wrapper() == FAILED) {
        diag_report_status_host(BP_FAIL_STR);
        cterr('f', 0, "GE PHY 88E1112 Test Fails");
        return (FAILED);
    }
    doall_print_tail("88E1112C");

    doall_print_head("TLK10232");
    if (tlk10232_do_all_wrapper() == FAILED) {
        diag_report_status_host(BP_FAIL_STR);
        cterr('f', 0, "TLK10232 Test Fails");
        return (FAILED);
    }
    doall_print_tail("TLK10232");

    if (is_10gkr_capable() != TRUE) {
        /* Non Greyhound Switch */
        doall_print_head("GE Backplane Loopback");
        if (ge_phy_88E1112C_loopback_test(0) == FAILED) {
            diag_report_status_host(BP_FAIL_STR);
            cterr('f', 0, "GE Backplane Loopback Test Fails");
            return (FAILED);
        }
        doall_print_tail("GE Backplane Loopback");
    }

    diag_report_status_host(BP_PASS_STR);

    return (PASSED);
}


/******************************************************************************
 *
 * Function: doall_print_head 
 *
 * Description: This function prints out testname at the beginning of test 
 *
 * Inputs      : teststr - Test String 
 * Outputs     : None
 *
 *****************************************************************************/
static void doall_print_head (char *teststr)
{
    printf("\n--- Running %s Test ---\n", teststr);
    fflush(stdout);
}


/******************************************************************************
 *
 * Function: doall_print_tail
 *
 * Description: This function prints out testname at the end of test 
 *
 * Inputs      : teststr - Test String 
 * Outputs     : None
 *
 *****************************************************************************/
static void doall_print_tail (char *teststr)
{
    printf("\n--- %s Test PASS ---\n", teststr);
    fflush(stdout);
}


/******************************************************************************
 *
 * Function: xaui_line_loopback_test
 *
 * Description: This function perform the Xaui line loopback test
 *              from Cavium QLM2 Xaui to platform GE Switch xaui port.
 *
 * Inputs      : None
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/
static int xaui_line_loopback_test (void)
{
    int onoff;

    if (getc_answer("Enable XAUI ext lpbk?(y/n)", "yn",'y') == 'y') {
        printf("Enable the XAUI ext lpbk mode.\n");
        onoff = EN_XAUI_EXT_LPBK;
    } else {
        printf("Disable the XAUI ext lpbk mode.\n");
        onoff = DIS_XAUI_EXT_LPBK;
    }

    if (set_xaui_ext_lpbk(onoff) == FAILED) {
        cterr('f', 0, "Set xaui ext on/off %d loopback fail.\n", onoff);
        return (FAILED);
    }

    return (PASSED);
}

static int has_88x2222 (void)
{
    int id;

    id = get_sku_id();

    if (id == WOODLAWN_6GE) {
        return (FALSE);
    } else {
        return (TRUE);
    }
}

static int has_88E1112C (void)
{
    int id;

    id = get_sku_id();

    if (id == WOODLAWN_6GE_1XAUI) {
        return (FALSE);
    } else {
        return (TRUE);
    }
}


/******************************************************************************
 *
 * Function: diag_voltmarg_display_wrapper
 *
 * Description: This wrapper function displays voltage margin status
 *
 * Inputs      : opt
 * Outputs     : PASSED/FAILED
 *
 *****************************************************************************/
static long diag_voltmarg_display_wrapper (char *opt)
{
    pca9557_power_margin_status();
    fflush(stdout);
    return (PASSED);
}


/******************************************************************************
 *
 * Function: diag_voltmarg_set_wrapper
 *
 * Description: This wrapper function configures voltage margin
 *
 * Inputs      : opt
 * Outputs     : PASSED/FAILED
 *
 *****************************************************************************/
static long diag_voltmarg_set_wrapper (char *opt)
{
    pca9557_power_margin_ctrl();
    return (PASSED);
}


/******************************************************************************
 *
 * Function: diag_led_set_wrapper
 *
 * Description: This wrapper function sets LED according to input
 *
 * Inputs      : opt
 * Outputs     : PASSED/FAILED
 *
 *****************************************************************************/
static long diag_led_set_wrapper (char *opt)
{
    char *token;
    char led_cmd_str[32];
    char led_op_str[8];
    int led_cmd, led_op;

    token = strtok(opt, ":");
    strcpy(led_cmd_str, token);
    token = strtok(NULL, ":");
    strcpy(led_op_str, token);

    led_cmd = atoi(led_cmd_str);
    led_op  = atoi(led_op_str);

    switch (led_cmd) {
    case DIAG_LED_SET_GE0_SPD:
    case DIAG_LED_SET_GE1_SPD:
    case DIAG_LED_SET_GE2_SPD:
    case DIAG_LED_SET_GE3_SPD:
    case DIAG_LED_SET_GE4_SPD:
    case DIAG_LED_SET_GE5_SPD:
        diag_ge_led_toggle(led_cmd / 4, DIAG_PHY_LED_SPEED, led_op);
        break;
    case DIAG_LED_SET_GE0_LINK:
    case DIAG_LED_SET_GE1_LINK:
    case DIAG_LED_SET_GE2_LINK:
    case DIAG_LED_SET_GE3_LINK:
    case DIAG_LED_SET_GE4_LINK:
    case DIAG_LED_SET_GE5_LINK:
        diag_ge_led_toggle(led_cmd / 4, DIAG_PHY_LED_LINK, led_op);
        break;
    case DIAG_LED_SET_SFP0_EN:
    case DIAG_LED_SET_SFP1_EN:
    case DIAG_LED_SET_SFP2_EN:
    case DIAG_LED_SET_SFP3_EN:
    case DIAG_LED_SET_SFP4_EN:
    case DIAG_LED_SET_SFP5_EN:
        fpga_toggle_sfp_led(led_cmd / 4, FPGA_SFP_LED_EN, led_op);
        break;
    case DIAG_LED_SET_SFP0_S:
    case DIAG_LED_SET_SFP1_S:
    case DIAG_LED_SET_SFP2_S:
    case DIAG_LED_SET_SFP3_S:
    case DIAG_LED_SET_SFP4_S:
    case DIAG_LED_SET_SFP5_S:
        fpga_toggle_sfp_led(led_cmd / 4, FPGA_SFP_LED_SPEED, led_op);
        break;
    case DIAG_LED_SET_SFP_PLUS_EN:
        fpga_toggle_sfp_led(6, FPGA_SFP_LED_EN, led_op);
        break;
    case DIAG_LED_SET_SFP_PLUS_SPD:
        if (switch_sfp_plus_led(led_op) == FAILED) {
            return (FAILED);
        }    
        break;
    default:
        printf("Unknown LED command: %d\n", led_cmd);
        break;
    }
    return (PASSED);
}

/******************************************************************************
 *
 * Function: diag_1340_reg_read_wrapper 
 *
 * Description: This wrapper function read 1340 register
 *
 * Inputs      : opt
 * Outputs     : PASSED/FAILED
 *
 *****************************************************************************/
static long diag_1340_reg_read_wrapper (char *opt)
{
    dump_phy_88E1340_registers();
    fflush(stdout);
    return (PASSED);
}

/******************************************************************************
 *
 * Function: diag_1340_reg_write_wrapper
 *
 * Description: This wrapper function alter 1340 register
 *
 * Inputs      : opt
 * Outputs     : PASSED/FAILED
 *
 *****************************************************************************/
static long diag_1340_reg_write_wrapper (char *opt)
{
    alter_phy_88E1340_register();
    fflush(stdout);
    return (PASSED);
}

/******************************************************************************
 *
 * Function: diag_1548_reg_read_wrapper
 *
 * Description: This wrapper function read 1548 register
 *
 * Inputs      : opt
 * Outputs     : PASSED/FAILED
 *
 *****************************************************************************/
static long diag_1548_reg_read_wrapper (char *opt)
{
    dump_phy_88E1548L_registers(); 
    fflush(stdout);
    return (PASSED);
}

/******************************************************************************
 *
 * Function: diag_1548_reg_write_wrapper
 *
 * Description: This wrapper function alter 1548 register
 * 
 * Inputs      : opt
 * Outputs     : PASSED/FAILED
 *
 *****************************************************************************/
static long diag_1548_reg_write_wrapper (char *opt)
{
    alter_phy_88E1548L_register();
    fflush(stdout);
    return (PASSED);
}

/******************************************************************************
 *
 * Function: diag_1548_ptp_reg_read_wrapper
 *
 * Description: This wrapper function read 1548 ptp register
 *
 * Inputs      : opt
 * Outputs     : PASSED/FAILED
 *
 *****************************************************************************/
static long diag_1548_ptp_reg_read_wrapper (char *opt)
{
    dump_phy_88E1548_ptp_reg(); 
    fflush(stdout);
    return (PASSED);
}

/******************************************************************************
 *
 * Function: diag_1548_ptp_reg_write_wrapper
 *
 * Description: This wrapper function alter 1548 ptp register
 * 
 * Inputs      : opt
 * Outputs     : PASSED/FAILED
 *
 *****************************************************************************/
static long diag_1548_ptp_reg_write_wrapper (char *opt)
{
    alter_phy_88E1548_ptp_reg();
    fflush(stdout);
    return (PASSED);
}

/******************************************************************************
 *
 * Function: diag_2222_reg_read_wrapper
 *
 * Description: This wrapper function read 2222 register
 *
 * Inputs      : opt
 * Outputs     : PASSED/FAILED
 *
 *****************************************************************************/
static long diag_2222_reg_read_wrapper (char *opt)
{
    dump_phy_88X2222M_registers();
    fflush(stdout);
    return (PASSED);
}

/******************************************************************************
 *
 * Function: diag_2222_reg_write_wrapper
 *
 * Description: This wrapper function alter 2222 register
 *
 * Inputs      : opt
 * Outputs     : PASSED/FAILED
 *
 *****************************************************************************/
static long diag_2222_reg_write_wrapper (char *opt)
{
    alter_phy_88X2222M_register();
    fflush(stdout);
    return (PASSED);
}

/******************************************************************************
 *
 * Function: diag_2222_ptp_reg_read_wrapper
 *
 * Description: This wrapper function read 2222 ptp register
 *
 * Inputs      : opt
 * Outputs     : PASSED/FAILED
 *
 *****************************************************************************/
static long diag_2222_ptp_reg_read_wrapper (char *opt)
{
    dump_phy_88X2222M_ptp_register();
    fflush(stdout);
    return (PASSED);
}

/******************************************************************************
 *
 * Function: diag_2222_ptp_reg_write_wrapper
 *
 * Description: This wrapper function alter 2222 ptp register
 *
 * Inputs      : opt
 * Outputs     : PASSED/FAILED
 *
 *****************************************************************************/
static long diag_2222_ptp_reg_write_wrapper (char *opt)
{
    alter_phy_88X2222M_ptp_register();
    fflush(stdout);
    return (PASSED);
}

/******************************************************************************
 *
 * Function: diag_fpga_reg_read_wrapper
 *
 * Description: This wrapper function read fpga register
 * 
 * Inputs      : opt
 * Outputs     : PASSED/FAILED
 *
 *****************************************************************************/
static long diag_fpga_reg_read_wrapper (char *opt)
{
    display_fpga_regs();
    fflush(stdout);
    return (PASSED);
}

/******************************************************************************
 *
 * Function: diag_fpga_reg_write_wrapper
 *
 * Description: This wrapper function alter fpga register
 *
 * Inputs      : opt
 * Outputs     : PASSED/FAILED
 *
 *****************************************************************************/
static long diag_fpga_reg_write_wrapper (char *opt)
{
    alter_fpga_regs();
    fflush(stdout);
    return (PASSED);
}

/******************************************************************************
 *
 * Function: diag_10232_reg_read_wrapper
 *
 * Description: This wrapper function read tlk10232 register
 *
 * Inputs      : opt
 * Outputs     : PASSED/FAILED
 *
 *****************************************************************************/
static long diag_10232_reg_read_wrapper (char *opt)
{
    dump_tlk_10232_registers();
    fflush(stdout);
    return (PASSED);
}

/******************************************************************************
 *
 * Function: diag_10232_reg_write_wrapper
 *
 * Description: This wrapper function alter tlk10232 register
 *
 * Inputs      : opt
 * Outputs     : PASSED/FAILED
 *
 *****************************************************************************/
static long diag_10232_reg_write_wrapper (char *opt)
{
    alter_tlk_10232_register();
    fflush(stdout);
    return (PASSED);
}  


/******************************************************************************
 *
 * Function: diag_cavium_sgmii_status_wrapper
 *
 * Description: This wrapper function show cavium sgmii status 
 *
 * Inputs      : opt
 * Outputs     : PASSED/FAILED
 *
 *****************************************************************************/
static long diag_cavium_sgmii_status_wrapper (char *opt)
{
    display_sgmii_port_stats();
    fflush(stdout);
    return (PASSED);
}

/******************************************************************************
 *
 * Function: diag_cavium_sgmii_config_wrapper 
 *
 * Description: This wrapper function config cavium sgmii 
 *
 * Inputs      : opt
 * Outputs     : PASSED/FAILED
 *
 *****************************************************************************/
static long diag_cavium_sgmii_config_wrapper (char *opt)
{
    display_sgmii_port_cfg();
    fflush(stdout);
    return (PASSED);
}

/******************************************************************************
 *
 * Function: diag_cavium_sgmii_reg_read_wrapper 
 *
 * Description: This wrapper function read cavium sgmii reg
 *
 * Inputs      : opt
 * Outputs     : PASSED/FAILED
 *
 *****************************************************************************/
static long diag_cavium_sgmii_reg_read_wrapper (char *opt)
{
    sgmii_phy_reg_dump();
    fflush(stdout);
    return (PASSED);
}   

/******************************************************************************
 *
 * Function: diag_cavium_xaui_status_wrapper
 *
 * Description: This wrapper function show cavium xaui status
 *
 * Inputs      : opt
 * Outputs     : PASSED/FAILED
 *
 *****************************************************************************/
static long diag_cavium_xaui_status_wrapper (char *opt)
{
    display_xaui_port_status();
    fflush(stdout);
    return (PASSED);
}

/******************************************************************************
 *
 * Function: diag_cavium_xaui_gmx_reg_read_wrapper 
 *
 * Description: This wrapper function read cavium xaui gmx register 
 *
 * Inputs      : opt
 * Outputs     : PASSED/FAILED
 *
 *****************************************************************************/
static long diag_cavium_xaui_gmx_reg_read_wrapper (char *opt)
{
    dump_xaui_gmx_regs();
    fflush(stdout);
    return (PASSED);
}   

/******************************************************************************
 *
 * Function: diag_cavium_xaui_pcs_reg_read_wrapper
 *
 * Description: This wrapper function read cavium xaui pcs register
 *
 * Inputs      : opt
 * Outputs     : PASSED/FAILED
 *
 *****************************************************************************/
static long diag_cavium_xaui_pcs_reg_read_wrapper (char *opt)
{
    dump_xaui_pcs_regs();
    fflush(stdout);
    return (PASSED);
}   

/******************************************************************************
 *
 * Function: diag_bootflash_get_info_wrapper 
 *
 * Description: This wrapper function read bootflash info 
 *
 * Inputs      : opt
 * Outputs     : PASSED/FAILED
 *
 *****************************************************************************/
static long diag_bootflash_get_info_wrapper (char *opt)
{
    get_bootflash_info();
    fflush(stdout);
    return (PASSED);
} 

/******************************************************************************
 *
 * Function: diag_bootflash_otp_test_wrapper
 *
 * Description: This wrapper function do bootflash otp test 
 *
 * Inputs      : opt
 * Outputs     : PASSED/FAILED
 *
 *****************************************************************************/
static long diag_bootflash_otp_test_wrapper (char *opt)
{
    bootflash_otp_test();
    fflush(stdout);
    return (PASSED);
}

/******************************************************************************
 *
 * Function: diag_1548_drift_adjustment_wrapper
 *
 * Description: This wrapper function do 1548 drift adjustment script 
 *
 * Inputs      : opt
 * Outputs     : PASSED/FAILED
 *
 *****************************************************************************/
static long diag_1548_drift_adjustment_wrapper (char *opt)
{
    verify_1548_drift_adjustment_mode();
    fflush(stdout);
    return (PASSED);
}

/******************************************************************************
 *
 * Function: diag_enable_1548_ptp_engine_wrapper
 *
 * Description: This wrapper function enable 88E1548 PTP engine 
 *
 * Inputs      : opt
 * Outputs     : PASSED/FAILED
 *
 *****************************************************************************/
static long diag_enable_1548_ptp_engine_wrapper (char *opt)
{
    setup_1548_ptp_engine();
    fflush(stdout);
    return (PASSED);
}

/******************************************************************************
 *
 * Function: diag_1548_clk_trig_in_wrapper
 *
 * Description: This wrapper function do 1548 drift adjustment script 
 *
 * Inputs      : opt
 * Outputs     : PASSED/FAILED
 *
 *****************************************************************************/
static long diag_1548_clk_trig_in_wrapper (char *opt)
{
    printf("Running 88E1548P Clock/Trigger In Verification...\n");
    fflush(stdout);

    if (verify_1548_clk_trig_in() != PASSED) {
        printf("\n*** TEST FAIL\n");
    } else {
        printf("TEST PASS\n");
    }
    fflush(stdout);
    return (PASSED);
}

/******************************************************************************
 *
 * Function: diag_2222_clk_trig_in_wrapper
 *
 * Description: This wrapper function do 1548 drift adjustment script 
 *
 * Inputs      : opt
 * Outputs     : PASSED/FAILED
 *
 *****************************************************************************/
static long diag_2222_clk_trig_in_wrapper (char *opt)
{
    printf("Running 88X2222P Clock/Trigger In Verification...\n");
    fflush(stdout);

    if (verify_2222_clk_trig_in() != PASSED) {
        printf("\n*** TEST FAIL\n");
    } else {
        printf("TEST PASS\n");
    }
    fflush(stdout);
    return (PASSED);
}

/******************************************************************************
 *
 * Function: diag_config_1548_gen_clk_wrapper
 *
 * Description: This wrapper function do 1548 drift adjustment script 
 *
 * Inputs      : opt
 * Outputs     : PASSED/FAILED
 *
 *****************************************************************************/
static long diag_config_1548_gen_clk_wrapper (char *opt)
{
    config_1548_gen_clk_out();
    fflush(stdout);
    return (PASSED);
}

/******************************************************************************
 *
 * Function: diag_config_1548_gen_trig_wrapper
 *
 * Description: 
 *
 * Inputs      : opt
 * Outputs     : PASSED/FAILED
 *
 *****************************************************************************/
static long diag_config_1548_gen_trig_wrapper (char *opt)
{
    config_1548_gen_trig_out();
    fflush(stdout);
    return (PASSED);
}

/******************************************************************************
 *
 * Function: diag_config_2222_gen_trig_wrapper
 *
 * Description:
 *
 * Inputs      : opt
 * Outputs     : PASSED/FAILED
 *
 *****************************************************************************/
static long diag_config_2222_gen_trig_wrapper (char *opt)
{
    config_2222_gen_trig_out();
    fflush(stdout);
    return (PASSED);
}

/******************************************************************************
 *
 * Function: diag_verify_fpga_sync_clk_out_wrapper
 *
 * Description: This wrapper function do FPGA SYNC_CLK_OUT verification
 *
 * Inputs      : opt
 * Outputs     : PASSED/FAILED
 *
 *****************************************************************************/
static long diag_verify_fpga_sync_clk_out_wrapper (char *opt)
{
    printf("Running FPGA SYNC_CLK_Out Verification...\n");
    fflush(stdout);

    if (verify_fpga_sync_clk_out() != PASSED) {
        printf("\n*** TEST FAIL\n");
    } else {
        printf("TEST PASS\n");
    }
    fflush(stdout);
    return (PASSED);
}

/******************************************************************************
 *
 * Function: diag_verify_fpga_sync_trig_out_wrapper
 *
 * Description: This wrapper function do FPGA SYNC_TRIG_OUT verification
 *
 * Inputs      : opt
 * Outputs     : PASSED/FAILED
 *
 *****************************************************************************/
static long diag_verify_fpga_sync_trig_out_wrapper (char *opt)
{
    printf("Running FPGA SYNC_TRIG_OUT Verification...\n");
    fflush(stdout);

    if (verify_fpga_sync_trig_out() != PASSED) {
        printf("\n*** TEST FAIL\n");
    } else {
        printf("TEST PASS\n");
    }
    fflush(stdout);
    return (PASSED);
}

/******************************************************************************
 *
 * Function: diag_config_2222_gen_clk_wrapper
 *
 * Description: This wrapper function do 1548 drift adjustment script 
 *
 * Inputs      : opt
 * Outputs     : PASSED/FAILED
 *
 *****************************************************************************/
static long diag_config_2222_gen_clk_wrapper (char *opt)
{
    config_2222_gen_clk_out();
    fflush(stdout);
    return (PASSED);
}

/******************************************************************************
 *
 * Function: diag_config_fpga_clk_mux_ge0_wrapper
 *
 * Description: config FPGA clock mux selection
 *
 * Inputs      : opt
 * Outputs     : PASSED/FAILED
 *
 *****************************************************************************/
static long diag_config_fpga_clk_mux_ge0_wrapper (char *opt)
{
    config_fpga_clk_mux_sel(FPGA_CLK_MUX_GE0);
    fflush(stdout);
    return (PASSED);
}

/******************************************************************************
 *
 * Function: diag_config_fpga_trig_mux_ge0_wrapper
 *
 * Description: config FPGA trigger mux selection
 *
 * Inputs      : opt
 * Outputs     : PASSED/FAILED
 *
 *****************************************************************************/
static long diag_config_fpga_trig_mux_ge0_wrapper (char *opt)
{
    config_fpga_trig_mux_sel(FPGA_TRIG_MUX_GE0);
    fflush(stdout);
    return (PASSED);
}

/******************************************************************************
 *
 * Function: diag_config_fpga_clk_mux_ge1_wrapper
 *
 * Description: config FPGA clock mux selection
 *
 * Inputs      : opt
 * Outputs     : PASSED/FAILED
 *
 *****************************************************************************/
static long diag_config_fpga_clk_mux_ge1_wrapper (char *opt)
{
    config_fpga_clk_mux_sel(FPGA_CLK_MUX_GE1);
    fflush(stdout);
    return (PASSED);
}

/******************************************************************************
 *
 * Function: diag_config_fpga_trig_mux_ge1_wrapper
 *
 * Description: config FPGA trigger mux selection
 *
 * Inputs      : opt
 * Outputs     : PASSED/FAILED
 *
 *****************************************************************************/
static long diag_config_fpga_trig_mux_ge1_wrapper (char *opt)
{
    config_fpga_trig_mux_sel(FPGA_TRIG_MUX_GE1);
    fflush(stdout);
    return (PASSED);
}

/******************************************************************************
 *
 * Function: diag_config_fpga_clk_mux_x2222p_wrapper
 *
 * Description: config FPGA clock mux selection
 *
 * Inputs      : opt
 * Outputs     : PASSED/FAILED
 *
 *****************************************************************************/
static long diag_config_fpga_clk_mux_x2222p_wrapper (char *opt)
{
    config_fpga_clk_mux_sel(FPGA_CLK_MUX_X2222P);
    fflush(stdout);
    return (PASSED);
}

/******************************************************************************
 *
 * Function: diag_config_fpga_trig_mux_x2222p_wrapper
 *
 * Description: config FPGA trigger mux selection
 *
 * Inputs      : opt
 * Outputs     : PASSED/FAILED
 *
 *****************************************************************************/
static long diag_config_fpga_trig_mux_x2222p_wrapper (char *opt)
{
    config_fpga_trig_mux_sel(FPGA_TRIG_MUX_X2222P);
    fflush(stdout);
    return (PASSED);
}

/**********************************************************************
 *
 * Function: diag_report_status_host
 *
 * This function reports the pass/fail status to host through nc
 *
 * Input : str - status string
 *
 * Output: none
 *
 **********************************************************************
 */
static void diag_report_status_host (char *str)
{
    char cmd[64];
    int ix;
    int rc;

    /* Sanity check */
    if (str == NULL) {
        printf("%s: Null pointer\n", __FUNCTION__);
        return;
    }

    /* ping platform */
    rc = FAILED;
    for (ix = 0; ix < PING_HOST_TOUT; ix++) {
        if (ping_host() == TRUE) {
            printf("Ping pass, KR path is good - %d\n", ix);
            fflush(stdout);
            rc = PASSED;
            break;
        } else {
            tlk10232_path_reset();
        }
        msleep(1000);
    }

    if (rc != PASSED) {
        cterr('f', 0, "\nUnable to ping host from Woodlawn!");
        fflush(stdout);
    }

    sprintf(cmd, "echo %s | nc %s %d", str, BP_XAUI_IP_ADDR,
            DIAG_RTN_STS_OUT_PORT_BASE);
    system(cmd);
}

int ping_host (void) {
    char *result_file = "/tmp/woodlawn_ping_host_result";
    char cmdbuf[128], buf[128], dum_char[32];
    uint  pktcnt, deadline;
    FILE *fp;
    int tx_cnt, rx_cnt;
    char sm_ip[16];

    pktcnt = 2;
    deadline = 5;

    /* Remove old result file */
    fp = fopen(result_file, "r");
    if (fp != NULL) {
        fclose(fp);
        sprintf(cmdbuf, "rm %s", result_file);
        system(cmdbuf);
    }

    /* Host side ip address - 192.123.123.1 */
    sprintf(sm_ip, "%s.%d", PLATFORM_IP_ADDR_SUBNET, 1);

    sprintf(cmdbuf, "ping -c %d -w %d -I xaui0 %s > %s",
            pktcnt, deadline, sm_ip, result_file);
    system(cmdbuf);

    /* Open the new result file */
    fp = fopen(result_file, "r");
    if (fp == NULL) {
        return (FALSE);
    }

    /* Check the result */
    while (!feof(fp)) {
        fgets(buf, sizeof(buf), fp);

        if (strstr(buf, "received") != NULL) {
            break;
        }
    }
    fclose(fp);
    sprintf(cmdbuf, "rm %s", result_file);
    system(cmdbuf);

    /* Read the string */
    sscanf(buf, "%d %s %s %d", &tx_cnt, dum_char, dum_char, &rx_cnt);

    if (rx_cnt < pktcnt) {
        return (FALSE);
    } else {
        return (TRUE);
    }
}
/*-------------------------------------------------
 * $Log: diag.c,v $
 * Revision 1.8  2015/03/31 06:25:20  leschen
 * Ping host before return test result to host through nc.
 *
 * Revision 1.7  2015/02/14 12:48:41  kodko
 * Collapse timing card branch code into main trunk.
 *
 * Revision 1.6  2015/02/04 07:21:36  leschen
 * Fix for sfp+ speed led control.
 *
 * Revision 1.5  2014/11/12 06:32:59  leschen
 * Support Greyhound switch a
 *
 * Revision 1.4  2014/02/20 10:36:11  leschen
 * Include nvsysvars.h header file.
 *
 * Revision 1.3.4.2  2014/04/30 13:47:20  kodko
 * Support 1548P/2222P clock/trigger in/out verification.
 *
 * Revision 1.3.4.1  2014/03/11 02:30:39  leschen
 * Add 1588 clk/trig verification item.
 *
 * Revision 1.3  2013/11/26 08:40:39  hroni
 * fix compiler warning
 *
 * Revision 1.2  2013/10/08 08:48:27  tirawan
 * Woodlawn collapsed to main trunk
 *
 * Revision 1.1.4.2  2013/08/20 10:58:50  tirawan
 * Branch into woodlawn-branch2 and port woodlawn code
 *
 * Revision 1.1.2.5  2013/07/02 07:37:13  leschen
 * Add bootflash get info and OTP test utility
 *
 * Revision 1.1.2.4  2013/06/17 10:49:47  leschen
 * Implement utility nc dispatch command for host side to be able to control SM LED
 *
 * Revision 1.1.2.3  2013/06/13 11:42:44  tirawan
 * Implement LED nc dispatch command for host side to be able to control SM LED
 *
 * Revision 1.1.2.2  2013/05/29 08:41:03  leschen
 * Move synchronize O2 flag to main.c
 *
 * Revision 1.1.2.1  2013/04/24 10:37:13  tirawan
 * Initial check-in for woodlawn linux code
 *
 * Revision 1.13  2013/04/17 02:39:49  kuangik
 * Change Report Status mechanism to run echo, instead of transferring file
 *
 * Revision 1.12  2013/04/10 11:14:01  leslie
 * Use FPGA lib to dump sfp+ contents
 *
 * Revision 1.11  2013/04/10 09:48:48  kuangik
 * Implement do all wrapper for host run sm test
 *
 * Revision 1.10  2013/04/08 08:55:38  leslie
 * Add PCA9557 items in basic utility menu
 *
 * Revision 1.9  2013/04/03 03:47:52  kuangik
 * Return status to host to indicate pass/fail
 *
 * Revision 1.8  2013/04/02 13:44:34  kuangik
 * Correct declaration
 *
 * Revision 1.4  2013/03/20 10:33:48  kuangik
 * Update SFP+ EEPROM display
 *
 * Revision 1.21  2013/03/12 11:18:20  leslie
 * Fix main menu flag
 *
 * Revision 1.20  2013/03/12 09:34:28  kuangik
 * Add SMI register display utility
 *
 * Revision 1.14  2013/02/19 00:57:37  leslie
 * Remove 88E1548L GE backplane loopback test from main test menu
 *
 * Revision 1.13  2013/01/16 00:59:45  leslie
 * Add 88E1112C test into main menu table.
 *
 * Revision 1.11  2012/12/11 02:33:23  leslie
 * Add XAUI backplane loopback test into main menu table.
 *
 * Revision 1.10  2012/11/08 05:34:16  kody
 * Move the QLM2 XAUI ext lpbk mode to utility.
 *
 * Revision 1.9  2012/11/08 02:50:44  kody
 * Add enable QLM2 XAUI ext-loopback for O2 backplane XAUI loopback test.
 *
 * Revision 1.8  2012/08/30 06:29:12  leslie
 * Add PLL test and utility.
 *
 * Revision 1.7  2012/08/03 10:16:55  evanli
 * Mapping to latest O2 source code on 20120726
 *
 * Revision 1.5  2012/07/19 06:56:12  leslie
 * Add dump_sfp_eeprom item into basic utility menu.
 *
 * Revision 1.4  2012/07/16 02:43:47  leslie
 * Add print SMI register item to debug menu utility.
 *
 * Revision 1.3  2012/04/16 02:40:53  kody
 * Add Marvell XAUI 88X2222M test and remove 88X2120L test.
 *
 * Revision 1.2  2012/02/10 06:14:43  leslie
 * Add Woodlawn according to DFS.
 *
 * Revision 1.1.1.1  2012/02/10 05:59:50  kody
 * Initial imports Woodlawn project code base.
 *
 * $Endlog $
 *-------------------------------------------------
 */
