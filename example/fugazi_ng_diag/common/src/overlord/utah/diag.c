/* $Id: diag.c,v 1.30 2019/08/06 06:56:15 alpeng Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/overlord/utah/diag.c,v $
 *------------------------------------------------------------------
 *
 * diag.c - Informers diagmon main menu and supporting wrappers.
 *
 * May 2008, Shih-Nan Huang adapted from Xformers.
 *
 * Copyright (c) 2013-2019 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include "endians.h"
#include "common.h"
#include "types.h"
#include "defs.h"
#include "setjmps.h"
#include "pcmap.h"
#include "monitor.h"
#include "mon_plat_defs.h"
#include "nvmonvars.h"
#include "menu.h"
#include "signals.h"
#include "uio_utils.h"
#include "pci.h"
#include "queryflags.h"
#include "error.h"
#include "slot.h"
#include "pm_utils.h"
#include "cross_platform.h"
#include "proto.h"
#include "strings.h"
#include "platform_led.h"
#include "platform_prom.h"
#include "dash_fpga.h"
#include "goofy_i2c.h"
#include "cookie_4.h"
#include "ngio.h"
#include "octeon_test.h"
#include "linux_usb_test.h"
#include "platform_poe_psu.h"
#include "platform_eeprom_access.h"
#include "plat_defs.h"
#include "platform_pci.h"
#include "platform_eth.h"

extern unsigned int use_mb_quack;
/* Function prototype */
extern int dash_set_map(int);
extern int dash_alt_mem(int argc);
extern int dash_dis_mem(int argc);
extern int dash_fil_mem(int argc);
extern int  alt_mem(), dis_mem(), fil_mem(), mov_mem(), cmp_mem();
extern int  memtest(), memloop(), addrloop(), find_mem();
extern int  build_i2c_menu(void);
extern int  build_margin_menu(void);
extern int  build_pwr_seq_menu(void);
extern int  alter_mb_cookie(void);
extern int  alter_poe_cookie(void);
extern int  alter_mp_cookie(void);
extern int  alter_ism_cookie(void);
extern int  alter_sm_cookie(void);
extern int  alter_vm_cookie(void);
extern int  alter_hwic_cookie(void);
extern int  alter_wic_dc_cookie(void);
extern int  alter_sm_dc_cookie(void);
extern int  alter_sm_vm_dc_cookie(void);
extern int  print_all_slots(int);
extern int  mb_led_bitmap_test(int);
extern int  mb_led_visual_test(int);
extern int  sys_all_leds(int);
extern int  sys_ok_led(int);
extern int  sys_vpn_led(int);
extern int  sys_ppp_led(int);
extern int  sys_ilp_led(int);
extern int  sys_cf_led(int);
extern int  alt_mb_cookie(int);
extern int act1_prog(unsigned int, unsigned char *choice);
extern char *strcasestr(char*, char*);
//extern void cache_enbl(void);
extern int  user_pci_write(void), user_pci_read(void);
extern int smartchip(int submenu_flag);
extern int  mb_board_type(void);
extern void show_sys_config(void);
extern int  user_pci_config_read(void);
extern int  user_pci_config_write(void);
extern int  dump_pci_regs(void);
extern int  alt_io_port(), dis_io_port();
extern void build_fan_menu(void);
extern int display_fpga_regs(int);
extern int rtc_utility_main(int);
/* MDIO utilities */
extern int vm_test(int);
extern int mb_tests(int flag);
extern int act2_prog(int);
//extern int smartchip(int);
extern int hwic_slot_test(int);
extern void read_date_time(void);
extern int platform_pwr_cycle(unsigned int);
extern int display_spi_prom_regs(int);
extern int dump_config_header(int);
extern int erase_config_header(int);
extern int program_config_header(int);
extern int display_sata_mux_setting(int);
extern int program_sata_mux_ngwic_setting(int);
extern int program_sata_mux_cpu_direct_setting(int);
extern int program_boot_upgrade_flag(int);
extern int program_cavecreek_eeprom(int dummy);
extern int platform_ser_irq_intr_test(int dummy);
/* 12V PoE PSU related */
extern uint32_t  poe_psu_cookie_utils(uint32_t);
extern boolean has_poe_psu(uint32_t);
extern int read_header_spi_prom_image(uchar type);
extern int platform_ser_irq_intr_test(int dummy);
extern int io_iface_tests(int);
extern int build_mux_menu(void);
extern int build_sfp_cookie_menu(int);

extern int hts_tests(int);
//extern int get_platform_brd_type(unsigned int);
//extern int get_platform_fpga_ver(int verbose);
static int display_brd_info(int);
static int reset_dev(int);
static int power_ngio(int);
static int set_uart_lpbk(int);
static int display_regs(int);
static int enable_uart(int);
static int program_mac(int);
static int common_pci_read(void);
int reset_sys_by_watchdog(void);
static int platform_shell(void);
static int shell_command(void);
static int verify_mac_program_result(void);
extern void aux_multiplex (int);
extern struct menuinfo *pci_menup;
extern struct menuinfo *spi_prom_menup;
extern int invoke_bcm_shell(void);

extern void pfix_kr_mode_debug(void); //pfix-debug
extern void show_gesw_port_assign(void);
extern int boot_flash_util(void);
void ngio_headless_mode(void);
void lpc_power_cycle(void);

static struct mitem reggio_fpga_items[] = {
    {"Program SPI PROM image without header",  0, 0,
     (type_t(*)())program_reggio_spi_prom_old,  &zero, 0, (type_t(*)())0, 0},
    {"Program SPI PROM image with header",  0, 0,
     (type_t(*)())program_reggio_spi_prom_old,   &one, 0, (type_t(*)())0, 0},
    {"Erase/Program Image Upgrade Header",  0, 0,
     (type_t(*)())program_image_upgrade_header,   &one, 0, (type_t(*)())0, 0},
    {"Set FPGA update flag",  0, 0,
     (type_t(*)())program_image_update_type,   &one, 0, (type_t(*)())0, 0},
    {"Set FPGA revision and date",  0, 0,
     (type_t(*)())set_date_revision,   &one, 0, (type_t(*)())0, 0},
    {"Display FPGA MULTI BOOT registers",  0, 0,
     (type_t(*)())display_multiboot,   &one, 0, (type_t(*)())0, 0},
    {"Display a sector", 0, 0, 
     (type_t(*)())display_prom_sector,   &zero, 0, (type_t(*)())0, 0},
    {"Test NIOS SPI",  0, 0,
     (type_t(*)())nios_test_spi_prom,  &three, 0, (type_t(*)())0, 0},
    {"Show Board type/FPGA Version",           0, 0,
     (type_t(*)())display_brd_info, &one, 0, (type_t(*)())0, 0},
    {"Erase Config header Sector",           0, 0,
     (type_t(*)())erase_config_header, &one, 0, (type_t(*)())0, 0},
    {"Display SATA mux setting",           0, 0,
     (type_t(*)())display_sata_mux_setting, &one, 0, (type_t(*)())0, 0},
    {"Program SATA MUX NGWIC Mode",           0, 0,
     (type_t(*)())program_sata_mux_ngwic_setting, &one, 0, (type_t(*)())0, 0},
    {"Program SATA MUX CPU Mode",           0, 0,
     (type_t(*)())program_sata_mux_cpu_direct_setting, &one, 0, (type_t(*)())0, 0},
    {"Program Boot Upgrade Flags",           0, 0,
     (type_t(*)())program_boot_upgrade_flag, &one, 0, (type_t(*)())0, 0},
    {"Reset internal devices",           0, 0,
     (type_t(*)())reset_dev, &zero, 0, (type_t(*)())0, 0},
    {"Reset external devices",           0, 0,
     (type_t(*)())reset_dev, &one, 0, (type_t(*)())0, 0},
    {"Enable/Disable NGIO",           0, 0,
     (type_t(*)())power_ngio, &one, 0, (type_t(*)())0, 0},
    {"rd/wr test",           0, 0,
     (type_t(*)())dash_rd_wr_test, &one, 0, (type_t(*)())0, 0},
    {"FPGA intr test", 0, 0,
     (type_t(*)())platform_intr_test,   &one, MF_CONTINUOUS | MF_DOGRP, (type_t(*)())0, 0},
    {"serial IRQ intr test", 0, 0,
     (type_t(*)())platform_ser_irq_intr_test,   &one, MF_CONTINUOUS | MF_DOGRP, (type_t(*)())0, 0},
    {"Toggle to CPLD/FPGA (default FPGA)", 0, 0, 
     (type_t(*)())dash_set_map,   &one, 0, (type_t(*)())0, 0},
    {"FPGA read", 0, 0, 
     (type_t(*)())dash_dis_mem,   &one, 0, (type_t(*)())0, 0},
    {"FPGA fill", 0, 0, 
     (type_t(*)())dash_fil_mem,   &one, 0, (type_t(*)())0, 0},
    {"FPGA alter", 0, 0, 
     (type_t(*)())dash_alt_mem,   &one, 0, (type_t(*)())0, 0},
    {"NGIO Headless Mode", 0, 0, 
     (type_t(*)())ngio_headless_mode,   &zero, 0, (type_t(*)())0, 0},
    {"LPC Power Cycle", 0, 0, 
     (type_t(*)())lpc_power_cycle,   &zero, 0, (type_t(*)())0, 0},
};

static struct menuinfo reggio_fpga_menu = {
    "  FPGA utility Menu",
    0,
    0,
    0,
    sizeof(reggio_fpga_items)/sizeof(struct mitem),
    reggio_fpga_items,
};
static struct menuinfo *reggio_fpga_menup = &reggio_fpga_menu;
static struct mitem uart_items[] = {
    {"AUX to FPGA UART6; Rangeley UART0 to SMBUS/PECI",         0, 0,
     (type_t(*)())aux_multiplex, &zero, 0, (type_t(*)())0, 0},
    {"AUX to NIOS UART8; Rangeley UART0 to SMBUS/PECI",         0, 0,
     (type_t(*)())aux_multiplex, &one, 0, (type_t(*)())0, 0},
    {"AUX to Rangeley UART0",         0, 0,
     (type_t(*)())aux_multiplex, &two, 0, (type_t(*)())0, 0},
    {"UART loopback",         0, 0,
     (type_t(*)())set_uart_lpbk, &one, 0, (type_t(*)())0, 0},
    {"Write to Uart ",         0, 0,
     (type_t(*)())set_uart_lpbk, &zero, 0, (type_t(*)())0, 0},
    {"display UART regs",         0, 0,
     (type_t(*)())display_regs, &zero, 0, (type_t(*)())0, 0},
    {"enable UART intr",         0, 0,
     (type_t(*)())enable_uart, &zero, 0, (type_t(*)())0, 0},

};

static struct menuinfo uart_menu = {
    "  UART  utility Menu",
    0,
    0,
    0,
    sizeof(uart_items)/sizeof(struct mitem),
    uart_items,
};
static struct menuinfo *uart_menup = &uart_menu;

/*
 * Memory debug utility
 */
static struct mitem mem_debug_items[] = {
    {"alter memory",    	    0,		0,
     (PFT)alt_mem,   &one,	0, (type_t(*)())0, 0},
    {"compare memory block",   	    0,		0,
     (PFT)cmp_mem,		            &one,	0, (type_t(*)())0, 0},
    {"display memory",    	    0,		0,
     (PFT)dis_mem,		            &one,	0, (type_t(*)())0, 0},
    {"move memory block",    	    0,		0,
     (PFT)mov_mem,		            &one,	0, (type_t(*)())0, 0},
    {"fill memory",    	            0,		0,
     (PFT)fil_mem,		            &one,	0, (type_t(*)())0, 0},
    {"find memory",    	            0,		0,
     (PFT)memtest,		            &one,	0, (type_t(*)())0, 0},
    {"memory read or write loop",   0,		0,
     (PFT)memloop,		            &one,	0, (type_t(*)())0, 0},
    {"memory debug loop",    	    0,		0,
     (PFT)memdebug,		            &one,	0, (type_t(*)())0, 0},
    {"address loop",    	    0,		0,
     (PFT)addrloop,		            &one,	0, (type_t(*)())0, 0},
};

static struct menuinfo mem_debug_menu = {
    "Memory debug utility Menu",
    0,
    0,
    0,
    sizeof(mem_debug_items)/sizeof(struct mitem),
    mem_debug_items,
};
static struct menuinfo *mem_debug_menup = &mem_debug_menu;

/*
 * Cookie menu utility
 */
static struct mitem cookie_items[] = {
    {"alter MB CPU cookie",    	          0,	0,
     (type_t(*)())alter_mb_cookie,	  &one,	0, (type_t(*)())0, 0},
    {"program host CPU GE0/1/2 MAC",    	          0,	0,
     (type_t(*)())program_mac,	  &zero,	0, (type_t(*)())0, 0},
    {"program host CPU Managment port MAC",    	          0,	0,
     (type_t(*)())program_mac,	  &one,	0, (type_t(*)())0, 0},
    {"alter NGIOWIC cookie",    	  0,	0,
     (type_t(*)())alter_hwic_cookie,	  &one,	0, (type_t(*)())0, 0},
    {"alter NGIOWIC DAUGHTER cookie",    	  0,	0,
     (type_t(*)())alter_wic_dc_cookie,	  &one,	0, (type_t(*)())0, 0},
    {"alter NGIOSM cookie",	  0,	0,
     (type_t(*)())alter_sm_cookie,	  &one,	0, (type_t(*)())0, 0},
    {"alter NGIOSM DAUGHTER cookie",	  0,	0,
     (type_t(*)())alter_sm_dc_cookie,	  &one,	0, (type_t(*)())0, 0},
    {"alter NGIOVM cookie",	  0,	0,
     (type_t(*)())alter_vm_cookie,	  &one,	0, (type_t(*)())0, 0},
    {"alter PoE cookie",                0,0,
     (type_t(*)())alter_poe_cookie, &zero, 0, (type_t(*)())is_utah, 0},
    {"alter PoE PSU1 cookie",             0,              0, 
     (type_t(*)())poe_psu_cookie_utils,   &one,           0,
     (type_t(*)())has_poe_psu,            POE_PSU_ONE},
    {"alter PoE PSU2 cookie",             0,              0, 
     (type_t(*)())poe_psu_cookie_utils,   &two,           0,
     (type_t(*)())has_poe_psu,            POE_PSU_TWO},
    {"alter NGIOSM VM DAUGHTER cookie",	  0,	0,
     (type_t(*)())alter_sm_vm_dc_cookie,  &one,	0, (type_t(*)())0, 0},
    {"Check MAC programming result",     0,     0,
     (type_t(*)())verify_mac_program_result,      &zero,        0, (type_t(*)())0, 0},
#if 0
    {"alter Back Plane cookie",	          0,	0,
     (type_t(*)())alter_mp_cookie,	  &one,	0, (type_t(*)())0, 0},
    {"alter MB PVDM cookie",    	  0,	0,
     (type_t(*)())alter_pvdm_cookie,	  &one,	0, (type_t(*)())0, 0},
    {"alter NM H/V/WIC cookie",	          0,	0,
     (type_t(*)())alt_vic_eeprom,	  &one,	0, (type_t(*)())0, 0},
#endif
    
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

/*
 * CPU Registers dump sub-menu utility
 */
#if 0
static struct mitem cpu_reg_items[] = {
    {"set platform debug level",        0,	0,
     (type_t(*)())user_set_platform_debug, &zero, 0, (type_t(*)())0, 0},
    {"show cpu info",    	        0,	0,
     (type_t(*)())show_cpu_info,	&zero,	0, (type_t(*)())0, 0},
    {"show cpu cr registers",    	0,	0,
     (type_t(*)())show_cpu_cr,     	&zero,	0, (type_t(*)())0, 0},
    {"show cpu msr registers",   	0,	0,
     (type_t(*)())show_cpu_msr,	        &zero,	0, (type_t(*)())0, 0},
    {"show console registers",    	0,	0,
     (type_t(*)())show_com_regs,	&zero,	0, (type_t(*)())0, 0},
    {"show aux registers",	        0,	0,
     (type_t(*)())show_com_regs,	&one,	0, (type_t(*)())0, 0},
    {"show lapic registers",    	0,	0,
     (type_t(*)())show_lapic_regs,      &zero,	0, (type_t(*)())0, 0},
    {"show ioapic registers",	        0,	0,
     (type_t(*)())show_ioapic_regs,	&zero,	0, (type_t(*)())0, 0},
    {"show hpet registers",    	        0,	0,
     (type_t(*)())show_hpet_regs,	&zero,	0, (type_t(*)())0, 0},
    {"test alarm",    	                0,	0,
     (type_t(*)())test_alarm,		&zero,	0, (type_t(*)())0, 0},
    {"test msleep",    	                0,	0,
     (type_t(*)())test_msleep,		&zero,	0, (type_t(*)())0, 0},
    {"test delay",    	                0,	0,
     (type_t(*)())test_delay,		&zero,	0, (type_t(*)())0, 0},
    {"alter i/o port",    	        0,	0,
     (type_t(*)())alt_io_port,		&one,	0, (type_t(*)())0, 0},
    {"display i/o port",   	        0,	0,
     (type_t(*)())dis_io_port,		&one,	0, (type_t(*)())0, 0},
    {"test int 3",    	                0,	0,
     (type_t(*)())gen_int_3,		&zero,	0, (type_t(*)())0, 0},
    {"show spirious watchdog interrupt counter",     0,      0,
     (type_t(*)())show_spirious_wdog_intr_cnt, &one, 0,	(type_t(*)())0,	0},
    {"test iofpga device reset bits one by one",     0,      0,
     (type_t(*)())mb_fpga_ind_dev_reset,&one,	0, (type_t(*)())0, 0},
    {"show and clear cpu machine check log",   	0,	0,
     show_cpu_errlog_report,		&one,	0, (type_t(*)())0, 0},
    {"show and clear mch machine check log",   	0,	0,
     show_mch_errlog_report,		&one,	0, (type_t(*)())0, 0},
    {"force memory ecc error",    	0,	0,
     gen_mem_ecc_err,		        &zero,	0, (type_t(*)())0, 0},
    {"iofpga warm reload",    	        0,	0,
     mb_fpga_ios_reload,		&zero,	0, (type_t(*)())0, 0},
    {"iofpga cold reload",    	        0,	0,
     mb_fpga_ios_reload,		&one,	0, (type_t(*)())0, 0},
    {"iofpga reset",    	        0,	0,
     mb_fpga_ios_reload,		&two,	0, (type_t(*)())0, 0},
};

static struct menuinfo cpu_reg_menu = {
    "CPU Registers Dump utility Menu",
    0,
    0,
    0,
    sizeof(cpu_reg_items)/sizeof(struct mitem),
    cpu_reg_items,
};
static struct menuinfo *cpu_reg_menup = &cpu_reg_menu;
#endif

/*
 * Cache utility
 */
#if 0
static struct mitem cpu_cache_items[] = {
};

static struct menuinfo cpu_cache_menu = {
    "CPU Cache utility Menu",
    0,
    0,
    0,
    sizeof(cpu_cache_items)/sizeof(struct mitem),
    cpu_cache_items,
};
static struct menuinfo *cpu_cache_menup = &cpu_cache_menu;
#endif

/* 
 * Basic utilities
 */
static struct mitem utilmenuitems[] = {

    {"PCIE read utility", 0, 0,
     (PFT)common_pci_read, (type_t *)&one, 0, (type_t(*)())0, 0},
    {"Memory debug utilities",        0, 0,
     (PFT)menu, (type_t *)&mem_debug_menup, 0,
     (type_t(*)())0,0},
    {"FPGA utilities",          0, 0,
     (PFT)menu, (type_t *)&reggio_fpga_menup, 0,
     (type_t(*)())0,0},
#if 0
    {"CPU registers dump utility",
     0,			0,
     (PFT)menu,		(type_t *)&cpu_reg_menup,	0,
     (type_t(*)())0,		0},
#endif
    {"I2C utilities",
     0,			0,
     (PFT)build_i2c_menu,	(type_t *)&one,		0,
     (type_t(*)())0,		0},
    {"Margin utilities",
     0,			0,
     (PFT)build_margin_menu,	(type_t *)&zero,		0,
     (type_t(*)())0,		0},
    {"Cookie utility",
     0,			0,
     (PFT)menu,		(type_t *)&cookie_menup,   0,
     (type_t(*)())0,		0},
    {"USB/Compact Flash utility",
     0,			0,
     (PFT)usb_utils,	(type_t *)&one,           0,
     (type_t(*)())0,		0},
#if 0
    {"Display iofpga regs",
     0,			0,
     (PFT)show_mb_fpga_regs,	(int*)&zero,		0,
     (type_t(*)())0,		0},
    {"FPGA global device reset test",
     0,			0,
     (PFT)mb_fpga_dev_reset,	(type_t *)&on_screen,	0,
     (type_t(*)())0,		0},
#endif
    {"RTC utilities",
     0,			0,
     (PFT)rtc_utility_main,	(type_t *)&zero,		0,
     (type_t(*)())0,		0},
    {"FAN utilities",
     0,                 0,
     (PFT)build_fan_menu,    (type_t *)&zero,                0,
     (type_t(*)())0,            0},
    {"Mother LED utility",
     0,                      0,
     (PFT)menu,              (type_t *)&led_menup,      0,
     (type_t(*)())0,         0},
    {"UART utility",          0, 0,
     (PFT)menu, (type_t *)&uart_menup, 0,
     (type_t(*)())0,0},
    {"Reset system by watchdog", 0, 0, 
     (PFT)reset_sys_by_watchdog, (type_t *)&one, 0, (type_t(*)())0, 0},
    {"Poll slots", 0, 0, 
     (PFT)print_all_slots, (type_t *)&one, 0, (type_t(*)())0, 0},
    {"Invoke BCM shell", 0, 0,
     (PFT)invoke_bcm_shell, (type_t *)&zero, 0, (type_t(*)())0,   0},
    {"Show GESW port assignment", 0, 0,
     (PFT)show_gesw_port_assign, (type_t *)&zero, 0, (type_t(*)())0,   0},
    { "Escape to Shell (debugging only)",  0, 0,   (PFT)platform_shell,
      (type_t *)&zero, 0,   (type_t(*)())0, 0 },
    { "Execute a Shell command (debugging only)",   0, 0,  (PFT)shell_command,
      (type_t *)&zero, 0,   (type_t(*)())0, 0 },
    {"Boot Flash utility", 0, 0,
     (PFT)boot_flash_util, &one,	    0, (type_t(*)())is_goldbeach,0},    
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
 * Main menu test flag defines
 */

#define MM_1	(MF_CONTINUOUS)
#define MM_2	(MM_1 | MF_DOALL)
#define MM_3	(MM_2 | MF_SHOW_ERRCOUNT)

/*=========================================
 * Main menu items
 *=========================================
 */
submenu_xtable_t main_menu_table[] = {
    {"i/o interface test",
     (PFT)io_iface_tests,       FALSE,          MM_1, 
     (type_t(*)())0, 0, (PFT)io_iface_tests, TRUE},
    {"ACT-2 utilities and programming",
     (PFT)smartchip,		FALSE,		MM_1,
     (type_t(*)())0, 0,		(PFT)smartchip,	TRUE},
    {"motherboard tests",
     (PFT)mb_tests,		TRUE,		MM_2,
     (type_t(*)())0, 0,		(PFT)mb_tests,	FALSE},
#ifdef OVERLORD_SPECIFIC /* hroni: might need modification */
    {"test data plane",
     (PFT)octeon_tests,		TRUE,		MM_2,
     (type_t(*)())0, 0,		(PFT)octeon_tests,	FALSE},
#else
    {"FPGA XAUI Interface",
     (PFT)hts_tests,		FALSE,		MM_2,
     (type_t(*)())is_usd_machines, 0,		(PFT)hts_tests,	TRUE},
#endif
    {"test WIC Slot 1",
     (PFT)wic_test,		FIRST_SLOT,		MM_2,
     (type_t(*)())0, 0,		(PFT)wic_test, FIRST_SLOT + MAX_WIC},
    {"test WIC Slot 2",
     (PFT)wic_test,		FIRST_SLOT+1,		MM_2,
     (type_t(*)())0, 0,		(PFT)wic_test, FIRST_SLOT+1 + MAX_WIC},
    {"test WIC Slot 3",
     (PFT)wic_test,             FIRST_SLOT+2,		MM_2,
     (type_t(*)())is_utah, 0,		(PFT)wic_test, FIRST_SLOT+2 + MAX_WIC},
    {"test SM Slot 1",
     (PFT)sm_test,		FIRST_SLOT,		MM_2,
     (type_t(*)())exist_sm_slot1, 0,		(PFT)sm_test, FIRST_SLOT + MAX_SM},
    {"test SM Slot 2",
     (PFT)sm_test,		FIRST_SLOT+1,		MM_2,
     (type_t(*)())is_utah, 0,		(PFT)sm_test, FIRST_SLOT+1 + MAX_SM},
    {"test VM",
     (PFT)vm_test,		FIRST_SLOT,		MM_2,
     (type_t(*)())is_usd_machines, 0,		(PFT)vm_test, FIRST_SLOT+ MAX_VM},

};

#define MAIN_MENU_TABLE_SIZE \
        (sizeof(main_menu_table) / sizeof(submenu_xtable_t))

/*
 * Primary & secondary submenu items (filled in from xtable)
 */
static mitem_t main_menu_primary_items[MAIN_MENU_TABLE_SIZE + MAX_BASE_ITEMS];
static mitem_t main_menu_secondary_items[MAIN_MENU_TABLE_SIZE + MAX_BASE_ITEMS];

static struct menuinfo maindiag = {
    "Main %s",			/* title */
    0,				/* title string added by init_empty_menu */
    (PFT)menu_show_dflags,	/* shows major flags */
    0,				/* generic prompt */
    0,				/* size -- bumped by add_menu_item() */
    main_menu_primary_items,
};
static struct menuinfo *maindiagp = &maindiag;

/*
 * SHIN_FIXME: pm_subtest_title is needed in nighthawk.c
 */
/*
 * Declarations for the Port Module subtest menu, which may be
 * selected by the user.  The struct subtest_items is a skeleton,
 * fleshed out by calls to add_menu_item(), after initialization by
 * a call to init_base_submenu.
 */
mitem_t subtest_items[MAX_SUBTEST_ITEMS];

menuinfo_t pm_subtest_menu = {
    "Port Module %s Subtest Menu",
    0,                                /* mtparam  added by init_empty_menu */
    (PFT)show_endnote,                /* notes missing WICs in combos */
    0,                                /* use generic prompt */
    0,                                /* size (bumped by add_menu_item() */
    subtest_items,
};
menuinfo_t *pm_submenup = &pm_subtest_menu;

title_buf_t    pm_subtest_header;
title_buf_t    pm_subtest_title[MAX_SUBTEST_ITEMS];

/*************************************************************
 * The menu driven diagnostics are called from either the low level
 * power-on routine or from the monitor.  Frommon will be non-zero if
 * called from the monitor.  Note that this value is actually the
 * argc parameter (a little slight of hand here), which is always
 * greater than zero.
 *************************************************************
 */
void
diag_menu(int argc, char *argv[]) 
{
    char arg;

    if (argc > 1)
    	arg = *argv[1];
    else
    	arg = 0;
    (NVRAM)->pollcon = 1;		/* poll the console */
    /*envflag = INDIAG;*/			/* set the environment flag */
    //    dobro_debug_flag = 0;
    build_primary_submenu(main_menu_table, MAIN_MENU_TABLE_SIZE, dgmenustr,
                          &maindiagp);
    build_secondary_submenu(main_menu_table, MAIN_MENU_TABLE_SIZE,
                            main_menu_secondary_items);

    menu(&maindiag, main_menu_secondary_items, arg);



}

/**********************************************************************
 *
 * Function: cli_main_menu_table_size()
 *
 * This routine is used for cli command to return the menu size
 *
 * Input : none
 *
 * Output: MAIN_MENU_TABLE_SIZE
 *
 **********************************************************************
 */
int
cli_main_menu_table_size(void)
{
    return (MAIN_MENU_TABLE_SIZE);
}

/**********************************************************************
 *
 * Function: display_brd_info
 *
 * Description: display board info, ie version number, revision number,
 *              etc...
 *
 * Input : val -- not used
 *
 * Output: PASSED
 *
 **********************************************************************
 */
static int
display_brd_info(int val)
{
    unsigned int fpga_ver, cpld_ver, fpga_brd, cpld_brd;
    get_platform_ver(1, &fpga_ver, &cpld_ver, &fpga_brd, &cpld_brd);
    //    get_platform_prom_boot1(1);
    //    get_platform_prom_sel1(1);
    if (is_goldbeach()) { 
        /* Goldbeach Display Secure JTAG status and register */
        get_secure_jtag_status (); 
    }
    return (PASSED);
}

static int
reset_dev(int val)
{
    unsigned int c;
    c = getdec_answer("enter '1' to reset; enter '0' to unreset", 1, 0, 1);

    if (val == 1) {
        dash_reset_ext(c);
    }
    if (val == 0) {
        dash_reset_int(c);
    }
    return PASSED;
}

/**********************************************************************
 *
 * Function: power_ngio
 *
 * Description: utility to turn on/off ngio power
 *
 * Input : val -- not used
 *
 * Output: PASSED
 *
 **********************************************************************
 */
static int
power_ngio(int val)
{
    unsigned int b, c, d;
    struct ngio_intf_t *ngio;

    b = getdec_answer("enter module type SM/WIC/VM [1/2/3]", 1, 1, 3);
    c = getdec_answer("enter slot number (first slot is 1)", 1, 1, 5);
    d = getdec_answer("power on or off [1/0]", 0, 0, 1);
    
    switch (b) {
    case 1:
        ngio = (struct ngio_intf_t *)slot_get_ngiosm(c);
        break;
    case 2:
        ngio = (struct ngio_intf_t *)slot_get_ngiowic(c);
        break;
    case 3:
        ngio = (struct ngio_intf_t *)slot_get_ngiovm(c);
        break;
    default:
        assert(!"wrong choice");
        
    }
    
    if (d) {
        ngio->on(ngio);
        if (ngio->i2c_unreset(ngio)<0) {
            cterr('f', 0, "slot%d power_ok bit not set", ngio->slot);
        }
        ngio->uart_on(ngio);
        ngio->unreset(ngio);
    } else {
        ngio->off(ngio);
        ngio->reset(ngio);
    }
    
    return PASSED;
}


/**********************************************************************
 *
 * Function: set_uart_lpbk
 *
 * Description: entry point to put FPGA uart in lpbk
 *
 * Input : val -- not used
 *
 * Output: PASSED
 *
 **********************************************************************
 */
static int
set_uart_lpbk (int val)
{
    uart_lpbk(val);   
    return PASSED;
}


/**********************************************************************
 *
 * Function: program_mac
 *
 * Description: entry point to program mac cavecreeek
 *
 * Input : management_port -- flag set if we are programming management port
 *                     
 * Output: PASSED
 *
 **********************************************************************
 */
static
int program_mac (int management_port)
{
    uchar buf[15] = { 0,0,0,0,0,0,0,0,0,0,0,0,0,0 }; /* has to be at least 6 */
    int32_t size = 0;
    unsigned int port = 0;
    char mac_str[13];
    char cmd[128];

    if (is_goldbeach()) { 
        size = get_mac_blk_size(); /* CSCuz11898 : MAC Address block size  : 144*/
    } else {
        size = get_mac_blk_size() - 1;
    }
    printf("size is %d\n", size);
    if (size < 4 ) {
        printf("data size = %d. it's too small.\n", size);
        printf("Please run 'alter mb cpu cookie' option at least once.\n");
        return(FAILED);
    }

    /* We are not support Utah P1A anymore */
    /* Utah P1B:
     * eth3 is control plane (mac base + blocksize - 2 ).
     * eth4 is management port (mac base + blocksize - 1). done outside for loop
     * xaui is data plane (mac base + block size - 5 ). this is done by xaui driver.
     * eth0/1/2 should be (mac base + 0/1/2).
     * Goldbeach:
     * eth0 is control plane (mac base + 0 ).
     * eth1 is management port (mac base + 1)
     * eth2 is NIM 1 (mac base + blocksize - 2 ).
     * eth3 is NIM 2 (mac base + blocksize - 3). 
     * mac base and blocksize is a stored cookie value.
     */
    if (!management_port) {
        for (port = 0; port <=3; port++) {
            if (is_goldbeach()) { 
                if (port >= 2) {
                    get_mac_from_block(size - port, buf);
                } else {
                    get_mac_from_block(port, buf);
                }
            } else {
                if (port == 3) {
                    get_mac_from_block(size-1, buf);
                } else {
                    get_mac_from_block(port, buf);
                }
            }
            if (!((*buf) || *(buf+1) || *(buf+2) || *(buf+3) || *(buf+4) ||
                  (*buf+5))) {
                printf("error getting mac base addr; did u run cookie util yet?\n");
                printf("Please run 'alter mb cpu cookie' and 'display cookie'"
                       " content at least once.\n");
                return(FAILED);
            }
            sprintf(cmd, "/utah-diag/intel-eeupdate-tool/nal");
            printf(cmd);
            system(cmd);

            sprintf(cmd, "/utah-diag/intel-eeupdate-tool/eeupdate64e /NIC=%d /MAC=%02x%02x%02x%02x%02x%02x\n",
                    port+1, buf[0], buf[1], buf[2], buf[3], buf[4], buf[5]);
            printf(cmd);
            system(cmd);

        } 
        return(PASSED);
    }/* !management_port */

    /*
     * Utah P1B also has eth4 implemented with i211 as the
     * management port which uses the last MAC address of the
     * MAC address block assigned to the system.
     * The MAC address progarmming of this port uses the Intel
     * eepudate utilisty. The utility is put under
     * utah-diag/intel-eeupdate-tool in the utah rootfs.
     * The parameter "port+1" is the NIC number to the
     * eeupdate util.
     * IMPORTANT: This user app must be run under /utah-diag directory.
     * otherwise the script will fail. (eeupdate64e /FILE option does
     * accept multiple directory path, so we run it in one directory
     * above where eeupdate64e is located.)
     */
    if (management_port) {
        if (is_goldbeach()) { 
            port = GOLDBEACH_MGMT; /* Goldbeach eth1 is management port  */
            get_mac_from_block(port, buf);
            sprintf(mac_str,"%02x%02x%02x%02x%02x%02x\n",
                    buf[0], buf[1], buf[2], buf[3], buf[4], buf[5]);
            sprintf(cmd, "/utah-diag/intel-eeupdate-tool/eeupdate64e /NIC=%d /MAC=%02x%02x%02x%02x%02x%02x\n",
                    port+1, buf[0], buf[1], buf[2], buf[3], buf[4], buf[5]);
         } else {
            get_mac_from_block(size, buf);
            sprintf(mac_str,"%02x%02x%02x%02x%02x%02x\n",
                    buf[0], buf[1], buf[2], buf[3], buf[4], buf[5]);
            sprintf(cmd, "/utah-diag/intel-eeupdate-tool/program_i211 %d %s\n",
                    CPU_SGMII_PORT4+1, mac_str);
        }    
        printf(cmd);
        system(cmd);
    }
    return PASSED;
}

/**********************************************************************
 *
 * Function: verify_mac_program_result 
 *
 * Description: Check MAC programming result, read out MAC from Linux  
 *              user space and check against the cookie MAC contents
 *
 * Input : NONE
 *                     
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
static int verify_mac_program_result (void)
{
    uchar buf[15] = { 0,0,0,0,0,0,0,0,0,0,0,0,0,0 }; /* has to be at least 6 */
    int32_t size = 0;
    unsigned int port = 0;
    char mac_str[128];
    char cmd[128], rd_mac[128];
    char *check_mac_file = "/utah-diag/check_mac.txt";
    int rc = FAILED;
    FILE *fp;

    size = get_mac_blk_size() - 1;

    if (size < 4 ) {
        printf("data size = %d. it's too small.\n", size);
        printf("Please run 'alter mb cpu cookie' option at least once.\n");
        return (rc);
    }

    for (port = 0; port <= 4; port++) {
        if (port == 3) {
            get_mac_from_block(size-1, buf);
        } else {
            get_mac_from_block(port, buf);
        }

        if (!((*buf) || *(buf+1) || *(buf+2) || *(buf+3) || *(buf+4) || (*buf+5))) {
            printf("error getting mac base addr; did u run cookie util yet?\n");
            printf("Please run 'alter mb cpu cookie' and 'display cookie'"
                   " content at least once.\n");
            return (rc);
        }

        sprintf(mac_str,"%.2x:%.2x:%.2x:%.2x:%.2x:%.2x\n",
            buf[0], buf[1], buf[2], buf[3], buf[4], buf[5]);

        sprintf(cmd, "ifconfig eth%d | grep HWaddr | sed -n 's/.*'HWaddr'//p' | sed s/[[:space:]]//g > %s", port, check_mac_file);
        system(cmd);

        fp = fopen(check_mac_file, "r");
        if (fp == NULL) {
            printf("Unable to open file - %s\n", check_mac_file);
            return (rc);
        }

        while (!feof(fp)) {
            fgets(rd_mac, sizeof(rd_mac), fp);
            if (strcasestr(rd_mac, mac_str) != NULL) {
                rc = PASSED;
            }
        }

        sprintf(cmd, "rm -f %s", check_mac_file);
        system(cmd);
        fclose(fp);

        if (rc == PASSED) {
            printf("eth%d MAC program successfully! - %s\n", port, rd_mac);
        } else {
            printf("eth%d MAC didn't program correctly! expected - %s, got - %s\n", port, mac_str, rd_mac);
            return(rc);
        }
    }

    return (rc);
}

/**********************************************************************
 *
 * Function: reset_sys_by_watchdog
 *
 * Description: reboot system by enableing watchdog timer.
 *              this test will reboot system!!!
 *
 * Input : NONE
 *                     
 * Output: PASSED
 *
 **********************************************************************
 */
int reset_sys_by_watchdog (void)
{
    int fd_wd;

    /* Reset system by watchdog */
    fd_wd = open("/dev/watchdog", O_RDWR);
    if (fd_wd == -1) {
        cterr('f',0,"open /dev/watchdog failed. \n");
        return (FAILED);
    } else {
        printf("System will be reboot by watchdog after 1 mins.\n");
    }
    return (PASSED);
}

/**********************************************************************
 *
 * Function: display_regs
 *
 * Description: display uart reg
 *
 * Input : NONE
 *                     
 * Output: PASSED
 *
 **********************************************************************
 */
static
int display_regs(int d)
{
    display_uart_regs(d);
    return PASSED;
}

/**********************************************************************
 *
 * Function: enable_uart
 *
 * Description: enable uart
 *
 * Input : d -- not used
 *                     
 * Output: PASSED
 *
 **********************************************************************
 */
static
int enable_uart(int d)
{
    enable_platform_uart_intr(0x1FF);
    uio_enable_intr();

    return (PASSED);
}

int pfix_empty_test (void)
{
    printf("Test not supported.\n");
    return(PASSED);
}

int common_pci_read (void) {

   int bus, dev, func, reg; 
    do {
        bus = gethex_answer("BUS", 0, 0, 0xF); 
        dev = gethex_answer("DEV", 0, 0, 0xF); 
        func = gethex_answer("FUN", 0, 0, 0xF); 
        reg = gethex_answer("REG", 0, 0, 0xFF); 


        reg = pci_config_read(bus, dev, func, reg);

        printf("reg === 0x%x \n", reg);


    } while(getc_answer("Continue?", "yn", 'y') == 'y');

   return 0;

}

/**********************************************************************
 *
 * Function: platform_shell
 *
 * This function to escaping to shell bash.
 *
 * Input : None
 *
 * Output: PASSED
 *
 **********************************************************************
 */
static int platform_shell (void)
{
    printf("\nEscaping to Shell from Main Menu,\n");
    printf("To back to Menu, please type exit from Shell.\n\n");

    system("/bin/bash");
    return(PASSED);
}


/**********************************************************************
 *
 * Function: shell_command
 *
 * This function enter shell command
 *
 * Input : void
 *
 * Output: PASSED
 *
 **********************************************************************
 */
static int shell_command (void)
{
    const int maxlen = 128;
    char cmd[maxlen];

    printf("\nPlease enter command: ");
    fgets(cmd, maxlen-1, stdin);
    system(cmd);

    return(PASSED);
}
/**********************************************************************
 *
 * Function: ngio_headless_mode
 *
 * When set, prevents host system reset (not power cycle) from affecting 
 * all the bits in this control register, and the corresponding slot¡¦s 
 * SYNCE control registers.
 *
 * Input : void
 *
 * Output: PASSED
 *
 **********************************************************************
 */
void ngio_headless_mode (void)
{
    int sts;
    ushort slot_val;
    
    ngio_t *ngio = (ngio_t *)(dash_fpga +  NGIO_BASE);
    slot_val = gethex_answer("Enter NIM Slot No:(1/2)", 1, 1, 2);
    sts = ngio->wic[slot_val - FIRST_SLOT].ctrl;
    sts |= HEADLESS_MODE;
    ngio->wic[slot_val - FIRST_SLOT].ctrl = sts;
    return;
}
/**********************************************************************
 *
 * Function: lpc_power_cycle
 *
 * To force a power cycle of the system, write to this register twice to initiate the power down.  
 * The two writes must contain the 1st and 2nd key in the upper word.
 *
 * Input : void
 *
 * Output: PASSED
 *
 **********************************************************************
 */
void lpc_power_cycle (void)
{
    assert(dash_cpld);

    rst_cpld_t *cpld = (rst_cpld_t*)dash_cpld;
    cpld->pwr = LPC_POWER_CYCLE_KEY1;
    sleep(1);
    cpld->pwr = LPC_POWER_CYCLE_KEY2;
    return;
}

/*---------------------------------------------------------------
$Log: diag.c,v $
Revision 1.30  2019/08/06 06:56:15  alpeng
merge curie, switzer and nightwatch to trunk

Revision 1.29.28.1  2018/10/15 10:45:10  alpeng
fixed compile error; add PLUG_FPGA define for i2c, naming confliction for tam lib and act2 lib

Revision 1.29  2017/11/16 07:56:25  leschen
Added cookie utility verify_mac_program_result for checking MAC programming result for Utah.

Revision 1.28  2017/07/28 07:49:43  harrchan
Collapse Oakenshield-branch to Main Trunk.

Revision 1.27  2016/10/17 11:23:03  iachang
Supported Goldbeach Platform.

Revision 1.26  2016/10/17 01:13:54  iachang
Hiding Goldbeach non-supported item.

Revision 1.25  2016/10/16 12:28:22  iachang
Supported Goldbeach Platform.

Revision 1.24  2014/08/06 21:29:30  ptong
Use BRCM SDK-LGA20140718 for Greyhound. Support autoneg on KR ports

Revision 1.23  2014/06/19 22:00:51  ptong
Incorporated BCM sdk-6.4.1-EA2 for 10G-KR interface. New code works on O2, June, and non Greyhound USD regression

Revision 1.22  2014/05/08 18:13:38  ptong
Put the Utility menu order per MFG request

Revision 1.21  2014/04/24 07:58:51  hroni
rename pcie read utility

Revision 1.20  2014/04/22 06:18:21  alpeng
not support utah P1A anymore; remove is_utah_p1a()

Revision 1.19  2014/03/11 08:08:55  alpeng
supprot 30w poe for utah only

Revision 1.18  2014/02/26 10:25:33  alpeng
USD doesn't support 30w poe anymore; still keep the code for platform_cookie.c

Revision 1.17  2014/02/18 09:11:11  alpeng
CSCul88171-3: remove useless files: mon_boot.h, c82576_ethmap.h, eth_frames.c, stack.h, sys_regs.h

Revision 1.16  2013/11/15 10:16:16  danchung
1.Remove "WIC Slot 3" and "SM Slot 2" test items for Sword and Dagger
2.Remove "SM Slot 1" test item for Dagger

Revision 1.15  2013/11/13 11:47:22  hroni
use one, two, three, and four from menu.h

Revision 1.14  2013/09/20 18:10:27  mcharon
update comments to explain about mac programming

Revision 1.13  2013/09/20 17:56:08  mcharon
use eepromupdate64e utility instead of ethtoot to program mac; ethtool was corrupting checksum. split program of mnagement port and ge0/1/2 intto different menu items

Revision 1.12  2013/09/11 21:41:08  ptong
Bump version to 1.0.0 and support i211 eeprom MAC programming for P1B

Revision 1.11  2013/08/22 06:40:49  alpeng
support fan utility on Utah

Revision 1.10  2013/08/14 17:44:59  hroni
add menu to select AUX port connection to FPGA UART 6, Rangeley 0, or NIOS uart 8

Revision 1.9  2013/07/24 01:19:53  hroni
Modify uart menu name
Enable DTR and RTS for Utah UART

Revision 1.8  2013/07/22 20:40:32  ptong
Changed program MAC address programming for HW P1A

Revision 1.7  2013/07/18 17:17:03  mcharon
add -Wal and clean up compile warnings

Revision 1.6  2013/07/17 23:28:46  mcharon
call hts_tests for xaui tests

Revision 1.5  2013/07/16 01:33:38  ptong
Minor change on funcition names

Revision 1.4  2013/07/03 23:50:19  ptong
Modify for proper init and menu setup

Revision 1.3  2013/06/13 08:34:56  hroni
add support for SFP mux

Revision 1.2  2013/05/14 03:09:39  hroni
fix compile error

Revision 1.1  2013/05/09 05:52:59  alpeng
add utah tree

Revision 1.41  2013/02/21 07:32:04  alpeng
fixed NGWIC3 menu flag from MM_3 to MM_2

Revision 1.40  2012/12/07 02:04:09  mcharon
handle error when mac size < 0 so program won't seg fault

Revision 1.39  2012/12/05 01:02:49  mcharon
call ngio->reset when turning off vm

Revision 1.38  2012/11/28 19:07:59  palin2
Move FPGA related debug utilities into item "FPGA utilities" Submenu.

Revision 1.37  2012/11/28 18:19:09  palin2
To make I2C utilities SubMenu more intuitive.

Revision 1.36  2012/11/07 18:21:17  mcharon
cleanup

Revision 1.35  2012/11/06 20:39:50  mcharon
add headers/cleanup/remove unneeded functions/files

Revision 1.34  2012/10/11 08:24:42  alpeng
unsupported SATA on wic slot 3

Revision 1.33  2012/09/25 23:02:28  mcharon
support fpga erase header option

Revision 1.32  2012/09/25 21:00:09  mcharon
support multiboot fpga programming

Revision 1.31  2012/09/20 00:13:01  mcharon
support oir

Revision 1.30  2012/09/18 19:27:58  mcharon
fix compile issue with extern print_all_slots

Revision 1.29  2012/09/18 19:19:55  mcharon
support poll slot-fix io intr test; support fpga upgrade; support serial irq intr tst

Revision 1.28  2012/09/15 01:22:46  ptong
Set diag menu items with MF_SHOW_ERRCOUNT flag

Revision 1.27  2012/09/14 17:15:06  mcharon
add submenu item for serial irq test

Revision 1.26  2012/09/14 00:28:19  mcharon
add i/o interface

Revision 1.25  2012/09/12 10:26:07  palin2
Add NGWIC TestCard support from Host side(Overlord) DiagMenu.

Revision 1.24  2012/09/12 09:21:13  alpeng
remove SATA test from mbtest and integrate SATA test into ngwic3 test

Revision 1.23  2012/09/11 04:33:16  srane
Correct vm test bug - menu parameter forces menu_display mode.

Revision 1.22  2012/08/20 13:22:58  palin2
Add NGSM TestCard support from Host side(Overlord) DiagMenu.

Revision 1.21  2012/08/14 11:30:56  palin2
Removed "ovld_" from TestCard related filename because TestCard is not Overlord's unique.

Revision 1.20  2012/08/09 22:58:20  mcharon
remove printf in diag_menu

Revision 1.19  2012/08/07 17:48:20  mcharon
allow user to specify fm file to program eeprom

Revision 1.18  2012/07/25 20:36:05  mcharon
add use_mb_quack flag

Revision 1.17  2012/07/23 17:33:54  palin2
Initial check-in for Overlord Test Card diag tests.

Revision 1.16  2012/07/17 23:59:11  mcharon
fix i2c utility...missing first byte && add i2c debug flag

Revision 1.15  2012/06/25 23:33:46  mcharon
support programming cavecreek eeprom

Revision 1.14  2012/06/07 02:11:21  palin2
Clean up compiler warnings.

Revision 1.13  2012/06/05 11:44:36  palin2
Clean up compiler warnings.

Revision 1.12  2012/05/11 21:47:33  mcharon
check return status of i2c_unreset

Revision 1.11  2012/05/11 18:47:45  ptong
Fixed power_ngio numbering

Revision 1.10  2012/05/09 08:28:14  alpeng
moving FPGA I2C scan test to MB test menu

Revision 1.9  2012/05/05 04:02:21  mcharon
support alter daughter board cookie for wic

Revision 1.8  2012/05/04 20:01:46  mcharon
use void* instead of int as argument to to func ptrs in ngio_intf

Revision 1.7  2012/05/02 02:05:29  mcharon
add config header support

Revision 1.6  2012/04/18 20:23:57  mcharon
change size to (size-1) when programmic mac

Revision 1.5  2012/04/18 19:11:26  mcharon
support mac program of eth1 to eth3 CSCtz29803

Revision 1.4  2012/04/17 14:14:06  palin2
Add 12V PoE PSU cookie utility support.

Revision 1.3  2012/04/11 21:27:16  ptong
Setup cavium named block for mailbox area, and use nc server on cavium to take command from host

Revision 1.2  2012/03/28 00:38:20  mcharon
remove forward slash from second line

$Endlog$
*/
