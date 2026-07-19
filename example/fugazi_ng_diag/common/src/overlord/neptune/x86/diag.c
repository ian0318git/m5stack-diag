/* $Id: diag.c,v 1.4 2019/08/06 06:56:15 alpeng Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/overlord/neptune/x86/diag.c,v $
 *------------------------------------------------------------------
 *
 * diag.c - Informers diagmon main menu and supporting wrappers.
 *
 * May 2008, Shih-Nan Huang adapted from Xformers.
 *
 * Copyright (c) 2009-2019 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
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
#include "platform_pci.h"
#include "plat_defs.h"   

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
extern menuinfo_t *fan_utilmenup;
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

extern boolean is_overlord(void);
//extern int get_platform_brd_type(unsigned int);
//extern int get_platform_fpga_ver(int verbose);
static int display_brd_info(int);
static int reset_dev(int);
static int power_ngio(int);
static int aux_sel(int);
static int common_pci_read(void);
static int set_uart_lpbk(int);
static int display_regs(int);
static int enable_uart(int);
static boolean support_ngio(unsigned int);
int reset_sys_by_watchdog(void);
static int program_x86_mac(int);
static int verify_mac_program_result(void);

extern struct menuinfo *pci_menup;
extern struct menuinfo *spi_prom_menup;
extern int invoke_bcm_shell(void);
extern void show_gesw_port_assign(void);
extern int ntpn_skip_ngio_slots; 
extern char *strcasestr(char* , char*);

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
     (type_t(*)())display_sata_mux_setting, &one, 0, (type_t(*)())is_overlord, 0},
    {"Program SATA MUX NGWIC Mode",           0, 0,
     (type_t(*)())program_sata_mux_ngwic_setting, &one, 0, (type_t(*)())is_overlord, 0},
    {"Program SATA MUX CPU Mode",           0, 0,
     (type_t(*)())program_sata_mux_cpu_direct_setting, &one, 0, (type_t(*)())is_overlord, 0},
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
    {"AUX to cavecreek1; Cavium to Uart7",         0, 0,
     (type_t(*)())aux_sel, &zero, 0, (type_t(*)())0, 0},
    {"AUX to Uart8; Cavium to Uart7",         0, 0,
     (type_t(*)())aux_sel, &one, 0, (type_t(*)())0, 0},
    {"AUX to to Cavium 0",         0, 0,
     (type_t(*)())aux_sel, &two, 0, (type_t(*)())0, 0},
    {"AUX: Uart loopback",         0, 0,
     (type_t(*)())set_uart_lpbk, &one, 0, (type_t(*)())0, 0},
    {"AUX: write to Uart ",         0, 0,
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
    {"program host CPU 10G port MAC",     0,	0,
     (type_t(*)())program_x86_mac,	  &zero,	0, (type_t(*)())0, 0},
    {"program host CPU Management port MAC", 0,	0,
     (type_t(*)())program_x86_mac,	  &one,0, (type_t(*)())0, 0},
    {"alter NGIOWIC cookie",    	  0,	0,
     (type_t(*)())alter_hwic_cookie,	  &one,	0, (type_t(*)())0, 0},
    {"alter NGIOWIC DAUGHTER cookie",    	  0,	0,
     (type_t(*)())alter_wic_dc_cookie,	  &one,	0, (type_t(*)())0, 0},
    {"alter NGIOSM cookie",	  0,	0,
     (type_t(*)())alter_sm_cookie,	  &one,	0, (type_t(*)())0, 0},
    {"alter NGIOSM DAUGHTER cookie",	  0,	0,
     (type_t(*)())alter_sm_dc_cookie,	  &one,	0, (type_t(*)())0, 0},
    {"alter PoE cookie",                0,0, 
     (type_t(*)())alter_poe_cookie, &zero, 0, (type_t(*)())0, 0},
    {"alter PoE PSU1 cookie",             0,              0, 
     (type_t(*)())poe_psu_cookie_utils,   &one,           0,
     (type_t(*)())has_poe_psu,            POE_PSU_ONE},
    {"alter PoE PSU2 cookie",             0,              0, 
     (type_t(*)())poe_psu_cookie_utils,   &two,           0,
     (type_t(*)())has_poe_psu,            POE_PSU_TWO},
    {"alter NGIOSM VM DAUGHTER cookie",   0,    0,
     (type_t(*)())alter_sm_vm_dc_cookie,  &one, 0, (type_t(*)())0, 0},
    {"Choose file to program cavecreek eeprom ",    	          0,	0,
     (type_t(*)())program_cavecreek_eeprom,	  &one,	0, (type_t(*)())0, 0},
    {"Check MAC programming result",     0,	0,
     (type_t(*)())verify_mac_program_result,	  &zero,	0, (type_t(*)())0, 0},
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
     (PFT)usb_utils_v2,	(type_t *)&one,           0,
     (type_t(*)())0,		0},
    {"RTC utilities",
     0,			0,
     (PFT)rtc_utility_main,	(type_t *)&zero,		0,
     (type_t(*)())0,		0},
    {"FAN utilities",
     0,                 0,
     (PFT)build_fan_menu, (type_t *)&zero,                0,
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
    {"Poll slots (Set Continuous flag to loop)", 0, 0, 
     (PFT)print_all_slots, (type_t *)&one, MF_CONTINUOUS, (type_t(*)())0, 0},
    {"Invoke BCM shell", 0, 0,
     (PFT)invoke_bcm_shell, (type_t *)&zero, 0, (type_t(*)())0,   0},
    {"Show GESW port assignment", 0, 0,
     (PFT)show_gesw_port_assign, (type_t *)&zero, 0, (type_t(*)())0,   0},
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
    {"test data plane",
     (PFT)octeon_tests,		TRUE,		MM_2,
     (type_t(*)())0, 0,		(PFT)octeon_tests,	FALSE},
    {"test NIM Slot 1",
     (PFT)wic_test,		FIRST_SLOT,		MM_2,
     (type_t(*)())0, 0,		(PFT)wic_test, FIRST_SLOT + MAX_WIC},
    {"test NIM Slot 2",
     (PFT)wic_test,		FIRST_SLOT+1,		MM_2,
     (type_t(*)())0, 0,		(PFT)wic_test, FIRST_SLOT+1 + MAX_WIC},
    {"test NIM Slot 3",
     (PFT)wic_test,             FIRST_SLOT+2,		MM_2,
     (type_t(*)())0, 0,		(PFT)wic_test, FIRST_SLOT+2 + MAX_WIC},
    {"test SM Slot 1",
     (PFT)sm_test,		FIRST_SLOT,		MM_2,
     (PFT)support_ngio, SKIP_SM1,	(PFT)sm_test, FIRST_SLOT + MAX_SM},
    {"test SM Slot 2",
     (PFT)sm_test,		FIRST_SLOT+1,		MM_2,
     (PFT)support_ngio, SKIP_SM2,	(PFT)sm_test, FIRST_SLOT+1 + MAX_SM},
    {"test SM Slot 3",
     (PFT)sm_test,		FIRST_SLOT+2,		MM_2,
     (PFT)support_ngio, SKIP_SM3,       (PFT)sm_test, FIRST_SLOT+2 + MAX_SM},
    {"test SM Slot 4",
     (PFT)sm_test,		FIRST_SLOT+3,		MM_2,
     (PFT)support_ngio, SKIP_SM4,       (PFT)sm_test, FIRST_SLOT+3 + MAX_SM},
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

/* This list contains the mother board test items skipped by user
 * using the skip_plugin.sh before invoking the diag
 */
char plugin_skip_list[64] = "";

/*************************************************************
 * The menu driven diagnostics are called from either the low level
 * power-on routine or from the monitor.  Frommon will be non-zero if
 * called from the monitor.  Note that this value is actually the
 * argc parameter (a little slight of hand here), which is always
 * greater than zero.
 *************************************************************
 */

/*
 * Function: get_skip_plugin 
 *
 * Description : Get the mother board items skipped in the
 * skip_test.txt file and compse the plugin_skip_list global variable
 *
 * Inputs: none 
 *
 * Output: TRUE/FALSE
 */
int get_skip_plugin() 
{
    FILE *fp;
    char *result_file = "/nep-diag/skip_test.txt";
    char buf[16], *bptr;

    fp = fopen(result_file, "r");
    if (fp == NULL) {
        return (FALSE);
    }

    bptr = plugin_skip_list;
    while (!feof(fp)) {
        if (fgets(buf, sizeof(buf), fp) != NULL) {
	    bptr += sprintf(bptr, " %s", buf); 
	    bptr--; /* back out the newline char at end of buf */
        } 
    }
    fclose(fp);

    return(TRUE);
}

/*
 * Function: prt_skip_plugin
 *
 * Description : This function will be called when the mb_tests menu item
 * is displayed to add the skipped plugin list after the menu title.
 *
 * Inputs: str - the skipped plugin list
 *
 * Output: TRUE always
 */
type_t prt_skip_plugin(char *str)
{
  printf("%s", str);
  return(TRUE);
}

/*
 * Function: add_mb_test_skip_msg
 *
 * Description : Using the mitem_t feature. This function adds the
 * prt_skip_plugin to the mlfunc field of mitem_t to allow the
 * skipped plugin list to be appended after the mb menu title.
 *
 * Inputs: 
 *
 * Output: TRUE always
 */
int add_mb_test_skip_msg(submenu_xtable_t *px, int size, menuinfo_t **addr_submenup)
{
    int i;
    mitem_t *pi;
    menuinfo_t *pm = *addr_submenup;
    char *bptr, skip_plugin_msg[128] = "";

    if (get_skip_plugin() == TRUE) {
        bptr = skip_plugin_msg;
	bptr += sprintf(bptr, "...(%s", plugin_skip_list);
	sprintf(--bptr, " ) skipped by user");

	pi = pm->miptr + base_submenu_item_total;  /* pi is at  next item after base items */
	for (i = 0; i < size; i++, px++, pi++) {
	    if ((void *)px->x_pfunc == (void *)&mb_tests) {
	        pi->mlfunc = &prt_skip_plugin;
		pi->mlparam = skip_plugin_msg;
		break;
	    }
	}
    }
    return (TRUE);
}

void
diag_menu(int argc, char *argv[]) 
{
    char arg;

    if (argc > 1) {
        arg = *argv[1];
    } else {
        arg = 0;
    }
    (NVRAM)->pollcon = 1;		/* poll the console */
    /*envflag = INDIAG;*/			/* set the environment flag */
    //    dobro_debug_flag = 0;
    build_primary_submenu(main_menu_table, MAIN_MENU_TABLE_SIZE, dgmenustr,
                          &maindiagp);
    build_secondary_submenu(main_menu_table, MAIN_MENU_TABLE_SIZE,
                            main_menu_secondary_items);

    /* Add the mother board skipped plugin list after the menu title
     */
    add_mb_test_skip_msg(main_menu_table, MAIN_MENU_TABLE_SIZE, &maindiagp);
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

    b = getdec_answer("enter module type SM/WIC [1/2]", 1, 1, 2);
    c = getdec_answer("enter slot number (first slot is 1)", 1, 1, 5);
    d = getdec_answer("power on or off [1/0]", 0, 0, 1);
    
    switch (b) {
    case 1:
        ngio = (struct ngio_intf_t *)slot_get_ngiosm(c);
        break;
    case 2:
        ngio = (struct ngio_intf_t *)slot_get_ngiowic(c);
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
 * Function: aux_sel
 *
 * Description: entry point to modify aux selection 
 *
 * Input : val -- not used
 *
 * Output: PASSED
 *
 **********************************************************************
 */
static int
aux_sel (int val)
{
    aux_multiplex(val);   
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
 * Function: program_x86_mac
 *
 * Description: entry point to execute diag_util script program_x86_mac
 *
 * Input : user -- flag set if user wants to specify mac address;
 *                 otherwise, get mac from cookie
 *                     
 * Output: PASSED
 *
 **********************************************************************
 */
static int program_x86_mac(int management_port)
{
    uchar buf[15] = { 0,0,0,0,0,0,0,0,0,0,0,0,0,0 }; /* has to be at least 6 */
    int32_t size = 0;
    unsigned int port = 0;
    char cmd[128];
    int nic_num = 0;

    size = get_mac_blk_size()-1;
    printf("size is %d\n", size);
    if (size < 10 ) {
        printf("data size = %d. it's too small.\n", size);
        printf("Please run 'alter mb cpu cookie' option at least once.\n");
        return(FAILED);
    }

    /* Neptune:
     * x86 CPU has 3 GE ports. The managemenet port is implemented
     * with i211. The other 2 ports are the built-in 10G ports on the CPU.
     * In the kernel, these ports are eth0-2 as shown below.
     * eth0 is management port (i211, mac base + blocksize - 1).
     * eth1 is CPU 10G port 0 (10G to GESW, mngt port mac - 1)
     * eth2 is CPU 10G port 1 (one-G to GESW, mngt port mac - 2)
     *
     * Cisco convention assigns to the management port the last MAC address of the
     * MAC address block assigned to the system.
     * Chasis mac base and blocksize are values stored in MB cookie.
     *
     * The MAC address progarmming of this ports uses the Intel
     * eepudate utilisty. The utility is put under
     * utah-diag/intel-eeupdate-tool in the kernel rootfs.
     */
    
    /* This for-loop is just for code sharing purpose. Port does not
     * really mean port number.
     * port=0 is for i211. port=1,2 are for the 10G ports.
     * Notes that the NIC numbers are different between them.
     */
    for (port = 0; port <=2; port++) {
        if ((management_port && (port > 0)) ||
	    (!management_port && (port == 0))) {
	    continue;
	}

	/* The nic_num is determined by the CPU PCIe hierachy.
	 * They are hard coded here by observing the output of eeupdate64e on the
	 * Neptune platform.
	 */
	nic_num = port;
	if (port == 0) {
	    nic_num = 3; /* the i211 has nic = 3 */
	}

	/* get_mac_from_block(size) give you the last MAC address of
	 * block.
	 */
        get_mac_from_block((size - port), buf);
	if (!((*buf) || *(buf+1) || *(buf+2) || *(buf+3) || *(buf+4) ||
	      (*buf+5))) {
	    printf("error getting mac base addr; did u run cookie util yet?\n");
	    printf("Please run 'alter mb cpu cookie' and 'display cookie'"
		   " content at least once.\n");
	    return(FAILED);
	}
	printf("eth%d: 0x%02x:0x%02x:0x%02x:0x%02x:0x%02x:0x%02x\n",
	       port, buf[0], buf[1], buf[2], buf[3], buf[4], buf[5]);


	sprintf(cmd, "%s/%s %d %.2x%.2x%.2x%.2x%.2x%.2x", INTEL_EEUPDATE_DIR, PROGRAM_X86_MAC,
		nic_num, buf[0], buf[1], buf[2], buf[3], buf[4], buf[5]);
	printf("%s\n", cmd);
	system(cmd);
    }
    return(PASSED);
}

/**********************************************************************
 *
 * Function: verify_mac_program_result 
 *
 * Description: Check MAC programming result, read out MAC from Linux user 
 *              space and check against the cookie MAC contents
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
    char cmd[128];
    FILE *fp;
    char *check_mac_file = "/nep-diag/check_mac.txt";
    char rd_mac[128];
    int rc = FAILED;
    char cookie_mac[128];

    size = get_mac_blk_size()-1;
    if (size < 10 ) {
        printf("data size = %d. it's too small.\n", size);
        printf("Please run 'alter mb cpu cookie' option at least once.\n");
        return (rc);
    }

    for (port = 0; port <=2; port++) {
	/* get_mac_from_block(size) give you the last MAC address of
	 * block.
	 */
        get_mac_from_block((size - port), buf);
	if (!((*buf) || *(buf+1) || *(buf+2) || *(buf+3) || *(buf+4) ||
	      (*buf+5))) {
	    printf("error getting mac base addr; did u run cookie util yet?\n");
	    printf("Please run 'alter mb cpu cookie' and 'display cookie'"
		   " content at least once.\n");
	    return(rc);
	}

	sprintf(cookie_mac, "%.2x:%.2x:%.2x:%.2x:%.2x:%.2x",
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
            if (strcasestr(rd_mac, cookie_mac) != NULL) {
                rc = PASSED;
            } 
        }

	sprintf(cmd, "rm -f %s", check_mac_file);
	system(cmd);
        fclose(fp);

        if (rc == PASSED) {
            printf("eth%d MAC program successfully! - %s\n", port, rd_mac);
        } else {
            printf("eth%d MAC didn't program correctly! expected - %s, got - %s\n", port, cookie_mac, rd_mac);
            return(rc);
        }
    }

    return(rc);
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

/**********************************************************************
 *
 * Function: common_pci_read
 *
 * Description: PCI read util for configuration space read 
 *
 * Input : None
 *
 * Output: PASSED
 *
 **********************************************************************
 */
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
 * Function: support_ngio
 *
 * Description: neptune, triton, proteus and neso have different 
 *              NGIO slot 
 *
 * Input : skip_ngio - ngio type
 *
 * Output: True/False
 *
 **********************************************************************
 */
static boolean support_ngio (unsigned int skip_ngio)
{
   /* ntpn_skip_ngio_slots init from linux_main.c */
   if (ntpn_skip_ngio_slots & skip_ngio) {
       return FALSE;  /* hide */ 
   } else {
       return TRUE;   /* display */
   }
}


/*---------------------------------------------------------------
$Log: diag.c,v $
Revision 1.4  2019/08/06 06:56:15  alpeng
merge curie, switzer and nightwatch to trunk

Revision 1.3.2.1  2018/10/15 10:40:37  alpeng
fixed neptune compile error and using ifdef AIKIDO_ACT2 since neptune act2 driver is already define mbx_read/write

Revision 1.3  2018/05/22 02:31:12  alpeng
fixed compiler warning, CSCvj57934

Revision 1.2  2018/05/18 09:24:59  alpeng
 Neptune merge to trunk with tag <neptune-branch-0518>

Revision 1.1.2.26  2018/05/11 06:14:10  alpeng
remove vm from diag.c which is display on main menu

Revision 1.1.2.25  2017/11/06 01:29:26  leschen
Modify the codes for MAC checking utility to make it more concisely.

Revision 1.1.2.24  2017/10/24 08:05:52  leschen
Add new utility to validate 1G and 10G Ethernet ports MAC programming result.

Revision 1.1.2.23  2017/09/19 10:18:51  alpeng
support oakenshield; fix oakenshield andf2w uart issue

Revision 1.1.2.22  2017/08/11 03:42:57  leschen
Support Neptune SM4 slot test

Revision 1.1.2.21  2017/07/05 03:13:22  alpeng
redirect show temp func, add d0 setup on i2c scan

Revision 1.1.2.20  2017/02/28 23:32:49  ptong
Fix program_x86_mac to setup management and 10G port MAC addresses

Revision 1.1.2.19  2017/01/25 03:46:56  meho
Changed program x86 mac script.

Revision 1.1.2.18  2017/01/24 09:33:53  alpeng
fix bug

Revision 1.1.2.17  2017/01/23 10:36:52  alpeng
update ngio slot info for triton, proteus and neso

Revision 1.1.2.16  2017/01/20 08:05:40  meho
Updated program GE/10G MAC menu.

Revision 1.1.2.15  2017/01/18 23:31:02  ptong
Fail diag when GESW failed to set 10GKR on NGIO

Revision 1.1.2.14  2017/01/18 03:09:43  alpeng
add MF_CONTINUOUS to poll slot for MDVT

Revision 1.1.2.13  2017/01/10 05:54:25  alpeng
remove sm4

Revision 1.1.2.12  2017/01/09 00:49:49  ptong
Print the skipped plugin list after mb_tests menu title to warn user on Neptune diag

Revision 1.1.2.11  2017/01/03 03:25:01  alpeng
hide vm for neptune, display it for triton, neso and proteus

Revision 1.1.2.10  2016/12/28 09:47:04  alpeng
update usb util, it is obsolete on new kernel

Revision 1.1.2.9  2016/12/27 09:49:22  leschen
Clean up codes and add display mem info utility.

Revision 1.1.2.8  2016/11/24 02:56:37  leschen
Not allow user to execute SM4 slot test for Neptune.

Revision 1.1.2.7  2016/11/09 08:00:40  leschen
Print message to remind user SM4 only support double wide module.

Revision 1.1.2.6  2016/10/21 18:19:39  alpeng
update testcard for sm4

Revision 1.1.2.5  2016/10/17 23:03:24  ptong
Fixed menu item display for SM-1 and 2

Revision 1.1.2.4  2016/10/16 23:26:57  ptong
Add invoke bcm shell in util menu

Revision 1.1.2.3  2016/06/21 21:39:07  jskow
Add SM4 skeleton code, add eUSB/emmc check, add msata test

Revision 1.1.2.2  2016/06/06 09:38:05  leschen
Support Neptune fan utility.

Revision 1.1.2.1  2016/06/02 22:04:01  jskow
Move Overlord/x86 specific files to Neptune/x86.

Revision 1.45  2014/02/18 09:11:11  alpeng
CSCul88171-3: remove useless files: mon_boot.h, c82576_ethmap.h, eth_frames.c, stack.h, sys_regs.h

Revision 1.44  2013/11/13 11:47:29  hroni
use one, two, three, and four from menu.h

Revision 1.43  2013/05/31 12:51:04  danchung
Add checking board type for Juno.

Revision 1.42  2013/05/09 19:25:21  mcharon
remove unused header files. fixed dependancy compile problem

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
