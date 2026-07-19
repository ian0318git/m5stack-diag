/* $Id: diag.c,v 1.4 2021/03/31 10:20:56 xiaolaya Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/overlord/neptune/curie_2RU/diag.c,v $
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
#include <stdio.h>
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
#include "linux_usb_test.h"
#include "platform_poe_psu.h"
#include "platform_eeprom_access.h"
#include "platform_pci.h"
#include "plat_defs.h"   
#include "platform_slot.h"
#include <unistd.h>
#include "plug_host_fpga_lib.h"
#include "plug_slot.h"
#include "curie2ru_test.h"
#include <linux_pci.h>
#include "bcm57412_test.h"
#include "ethernet.h"
#include "tam_aikido_upgrade.h"
#include "m2_testcard.h"
#include "m2_testcard_host_impl.h"

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
extern int  alter_sm_cookie(void);
extern int  alter_vm_cookie(void);
extern int  alter_nim_cookie(void);
extern int  alter_nim_dc_cookie(void);
extern int  alter_sm_dc_cookie(void);
extern int  alter_sm_dc_wic_cookie(void);
extern int  alter_sm_dc_wic_dc_vm_cookie(void);
extern int  alter_sm_vm_dc_cookie(void);
extern int alter_pim_cookie(void);
extern int  print_all_slots(int);
extern int act1_prog(unsigned int, unsigned char *choice);
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
extern int mb_tests(int flag);
extern int act2_prog(int);
extern menuinfo_t *fan_utilmenup;
extern int erase_config_header(int);
extern int display_sata_mux_setting(int);
extern int program_sata_mux_ngwic_setting(int);
extern int program_sata_mux_cpu_direct_setting(int);
extern int program_boot_upgrade_flag(int);
/* 12V PoE PSU related */
extern uint32_t  poe_psu_cookie_utils(uint32_t);
extern boolean has_poe_psu(uint32_t);
extern int platform_ser_irq_intr_test(int dummy);
extern int io_iface_tests(int);

extern boolean is_overlord(void);
static int display_brd_info(int);
static int reset_dev(int);
static int power_ngio(int);
static int common_pci_read(void);
static int set_uart_lpbk(int);
static int display_regs(int);
static int enable_uart(int);
static boolean support_ngio(unsigned int);
int reset_sys_by_watchdog(void);
static int program_eth_mac(int);
static int verify_mac_program_result(void);
static void lpc_power_cycle(void);

extern struct menuinfo *pci_menup;
extern struct menuinfo *spi_prom_menup;
extern int curie_skip_ngio_slots;
extern char *strcasestr(char* , char*);
extern int program_reggio_spi_prom(void);
extern void tam_aikido_reset_utilty(void); 
extern int check_bcm57412_driver(void);
extern int curie2ru_eth_traf_utility(void);

static struct mitem reggio_fpga_items[] = {
    {"Platfrom FPGA Program SPI PROM image without header",  0, 0,
     (type_t(*)())program_reggio_spi_prom_old,  &zero, 0, (type_t(*)())0, 0},
    {"Platform FPGA Program SPI PROM image with header",  0, 0,
     (type_t(*)())program_reggio_spi_prom_old,   &one, 0, (type_t(*)())0, 0},
    {"Aikido Program FPGA SPI PROM image", 0, 0, 
     (type_t(*)())program_reggio_spi_prom,      &zero, 0, (type_t(*)())0, 0},
    {"Erase/Program Image Upgrade Header",  0, 0,
     (type_t(*)())program_image_upgrade_header,   &one, 0, (type_t(*)())0, 0},
#ifdef AIKIDO_DEV_KEY
    {"Program Aikido FPGA DEV keys (Development phase)", 0, 0,
     (type_t(*)())program_aikido_dev_key, &zero, 0, (type_t(*)())0, 0},
#endif
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
    {"AIKIDO SPI read", 0, 0, 
     (type_t(*)())aikido_spi_read_util, &zero, 0, (type_t(*)())0, 0},
    {"AIKIDO SPI write", 0, 0, 
     (type_t(*)())aikido_spi_write_util, &zero, 0, (type_t(*)())0, 0},
    {"AIKIDO reset and unreset", 0, 0, 
     (type_t(*)())tam_aikido_reset_utilty, &zero, 0, (type_t(*)())0, 0},
    {"toggle flag : aikido_mailbox_flag", 0, 0, 
     (type_t(*)())aikido_flag_mailbox, &zero, 0, (type_t(*)())0, 0},
    {"toggle flag: aikido_act2_flag", 0, 0, 
     (type_t(*)())aikido_flag_act2, &zero, 0, (type_t(*)())0, 0},
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
    {"program Intel internal 10G port MAC",     0,	0,
     (type_t(*)())program_eth_mac,	  &zero,	0, (type_t(*)())0, 0},
    {"program I350 management port MAC", 0,	0,
     (type_t(*)())program_eth_mac,	  &one,0, (type_t(*)())0, 0},
    {"program BCM57412 port MAC", 0,	0,
     (type_t(*)())program_eth_mac,	  &two,0, (type_t(*)())0, 0},
    {"alter NGIOWIC cookie",    	  0,	0,
     (type_t(*)())alter_nim_cookie,	  &one,	0, (type_t(*)())0, 0},
    {"alter NGIOWIC DAUGHTER cookie",    	  0,	0,
     (type_t(*)())alter_nim_dc_cookie,	  &one,	0, (type_t(*)())0, 0},
    {"alter NGIOSM cookie",	  0,	0,
     (type_t(*)())alter_sm_cookie,	  &one,	0, (type_t(*)())0, 0},
    {"alter NGIOSM DAUGHTER cookie",	  0,	0,
     (type_t(*)())alter_sm_dc_cookie,	  &one,	0, (type_t(*)())0, 0},
    {"alter NGIOSM VM DAUGHTER cookie",   0,    0,
     (type_t(*)())alter_sm_vm_dc_cookie,  &one, 0, (type_t(*)())0, 0},
    {"alter PoE cookie",                  0,    0,
     (type_t(*)())alter_poe_cookie, &zero, 0, (type_t(*)())0, 0},
    {"alter PoE PSU1 cookie",             0,              0,
     (type_t(*)())poe_psu_cookie_utils,   &one,           0,
     (type_t(*)())has_poe_psu,            POE_PSU_ONE},
    {"alter PoE PSU2 cookie",             0,              0,
     (type_t(*)())poe_psu_cookie_utils,   &two,           0,
     (type_t(*)())has_poe_psu,            POE_PSU_TWO},
    {"Check MAC programming result",     0,	0,
     (type_t(*)())verify_mac_program_result,	  &zero,	0, (type_t(*)())0, 0},
    {"alter PIM cookie",	  0,	0,
     (type_t(*)())alter_pim_cookie,	  &zero,	0, (type_t(*)())0, 0},

    /* The SM(like Switzer-Carrier) will be separated to two NIM which is different from 'NGIOSM DAUGHTER cookie' */
    {"alter SM DAUGHTER WIC cookie",	  0,	0,     /* for SM Daughter WIC card cookie */
     (type_t(*)())alter_sm_dc_wic_cookie,	  &one,	0, (type_t(*)())0, 0},
    {"alter SM DAUGHTER WIC DC VM cookie",	  0,	0, /* for SM Daughter WIC Daughter VM card cookie */
     (type_t(*)())alter_sm_dc_wic_dc_vm_cookie,	  &one,	0, (type_t(*)())0, 0},
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
    {"Power Cycle", 0, 0,
     (PFT)lpc_power_cycle, (type_t *)&zero, 0, (type_t(*)())0, 0},
    {"ETH Traf", 0, 0,
     (PFT)curie2ru_eth_traf_utility, (type_t *)&zero, 0, (type_t(*)())0, 0},

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
    /* afix - need to deal with io interface test for eth port problem */
    {"i/o interface test",
     (PFT)io_iface_tests,       FALSE,          MM_1, 
     (type_t(*)())0, 0, (PFT)io_iface_tests, TRUE},
    {"ACT-2 utilities and programming",
     (PFT)smartchip,		FALSE,		MM_1,
     (type_t(*)())0, 0,		(PFT)smartchip,	TRUE},
    {"motherboard tests",
     (PFT)mb_tests,		TRUE,		MM_2,
     (type_t(*)())0, 0,		(PFT)mb_tests,	FALSE},
    {"test NIM Slot 1",
     (PFT)wic_test,             FIRST_SLOT,             MM_2,
     (PFT)support_ngio, SKIP_NIM1, (PFT)wic_test, FIRST_SLOT + MAX_WIC},
    {"test NIM Slot 2",
     (PFT)wic_test,             FIRST_SLOT+1,           MM_2,
     (PFT)support_ngio, SKIP_NIM2, (PFT)wic_test, FIRST_SLOT+1 + MAX_WIC},
    {"test SM Slot 1",
     (PFT)sm_test,              FIRST_SLOT,             MM_2,
     (PFT)support_ngio, SKIP_SM1,  (PFT)sm_test, FIRST_SLOT + MAX_SM},
    {"test SM Slot 2",
     (PFT)sm_test,              FIRST_SLOT+1,           MM_2,
     (PFT)support_ngio, SKIP_SM2,  (PFT)sm_test, FIRST_SLOT+1 + MAX_SM},
    /* we need to use sm_test_wrapper and wic_test_wrapper here, 
     * otherwise we cannot deal with ngio using different host eth port number 
     * easily. */
    {"test PIM Slot 1",
     (PFT) plug_test, PLUG_SLOT_1 ,
     MF_CONTINUOUS | MF_DOALL | MF_SHOW_ERRCOUNT,
     (type_t(*)())0, PLUG_SLOT_1,
     (PFT) plug_test, PLUG_SLOT_1 + MAX_PLUG_SLOT_NUMBER},
    /* M.2 Test Card: if present, show this test; not present, not show*/
    {"M.2 testcard test",
     (PFT)m2_testcard_test,		TRUE,		MM_2,
     (PFT)is_m2_testcard_in, 0, (PFT)m2_testcard_test,	FALSE},
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
    char *result_file = "/curie-2RU-diag/skip_test.txt";
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

static int diag_platform_init(int argc, char *argv[])
{
    if (platform_led_info_init()) {
        return -1;
    }
    if (is_curie_2ru()) {
        if (curie2ru_diag_init(argc, argv) < 0)
            return -1;
    }
    return 0;
}

static void diag_platform_exit(void)
{
    if (is_curie_2ru()) {
        curie2ru_diag_exit();
    }
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
    if (diag_platform_init(argc, argv))
        cterr('f', 0, "diag platform init failed");

    build_primary_submenu(main_menu_table, MAIN_MENU_TABLE_SIZE, dgmenustr,
                          &maindiagp);
    build_secondary_submenu(main_menu_table, MAIN_MENU_TABLE_SIZE,
                            main_menu_secondary_items);

    /* Add the mother board skipped plugin list after the menu title
     */
    add_mb_test_skip_msg(main_menu_table, MAIN_MENU_TABLE_SIZE, &maindiagp);

    menu(&maindiag, main_menu_secondary_items, arg);
    diag_platform_exit();

    system(MODPROBE_BCM57412_DRIVER);
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
        printf("you won't come here \n"); 
        
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

#define MAX_NR_MAC_ADDR 4

struct mac_addr_map {
    const char *name;
    int count;
    int index[MAX_NR_MAC_ADDR];     /* ethX */
    int offset[MAX_NR_MAC_ADDR];    /* mac address offset */
    unsigned long priv[MAX_NR_MAC_ADDR];

    int (*prog_mac)(struct mac_addr_map *map, uint8_t *mac_base,
                    size_t block_size);
};

#define CURIE2RU_MAC_MAP_SIZE   \
    (sizeof(curie2ru_mac_map) / sizeof(curie2ru_mac_map[0]))

static void get_mac_addrs(struct mac_addr_map *map, uint8_t *mac_base,
                          size_t block_size, uint8_t (*mac_addr)[6])
{
    int i;

    for (i = 0; i < map->count; i++) {
        int offset;

        offset = map->offset[i];
        if (offset < 0)
            offset += block_size;

        get_mac_from_block(offset, mac_addr[i]);
    }
}

#define BNXTMT_BCM57412_HOST_MAC    "./load.sh -dev 1-2 -m -none"
#define BNXTMT_BCM57412_NIM_MAC     "./load.sh -dev 3-4 -m -none"

static int
curie2ru_prog_bcm57412_mac(struct mac_addr_map *map, uint8_t *mac_base,
                           size_t block_size)
{
    int i;
    uint8_t mac_addr[MAX_NR_MAC_ADDR][6];
    char mac_str[MAX_NR_MAC_ADDR][32];
    char cmd[256];

    get_mac_addrs(map, mac_base, block_size, mac_addr);

    for (i = 0; i < map->count; i++) {
        uint8_t *p = mac_addr[i];
        sprintf(mac_str[i], "%.2x%.2x%.2x%.2x%.2x%.2x", p[0], p[1], p[2], p[3], p[4], p[5]);
    }

    system(RM_BCM57412_DRIVER);
    /* Switch to /curie-2RU-diag/ dir in order to execute script load.sh */
    chdir(BCM57412_DIR_2RU);
    sprintf(cmd, "%s <<< $'%s\n%s\nexit\n'", BNXTMT_BCM57412_HOST_MAC, mac_str[0], mac_str[1]);
    system(cmd);
    sprintf(cmd, "%s <<< $'%s\n%s\nexit\n'", BNXTMT_BCM57412_NIM_MAC, mac_str[2], mac_str[3]);
    system(cmd);
    msleep(10);
    system(INSERT_BCM57412_DRIVER);

    return PASSED;
}

static int
curie2ru_prog_intel_mac(struct mac_addr_map *map, uint8_t *mac_base,
                        size_t block_size)
{
    int i;
    char cmd[128];
    uint8_t mac_addr[MAX_NR_MAC_ADDR][6];

    get_mac_addrs(map, mac_base, block_size, mac_addr);

    for (i = 0; i < map->count; i++) {
        uint8_t *buf = mac_addr[i];
        int nic_num = map->priv[i];
        sprintf(cmd, "%s/%s %d %.2x%.2x%.2x%.2x%.2x%.2x",
                INTEL_EEUPDATE_DIR, PROGRAM_X86_MAC, nic_num,
                buf[0], buf[1], buf[2], buf[3], buf[4], buf[5]);
        system(cmd);
    }

    return PASSED;
}

/**
 * I350             eth0-eth3
 * BCM57412(host)   eth4-eth5
 * BCM57412(nim)    eth6-eth7
 * X722             eth8-eth11
 *
 * NIM1             eth7     142         -2
 * NIM2             eth6     141         -3
 * SM1 GE0          eth11    140         -4
 * SM1 GE1          eth9     139         -5
 * SM2 GE0          eth10    138         -6
 * SM2 GE1          eth8     137         -7
 */

static struct mac_addr_map curie2ru_mac_map[3] = {
    { "10G-KR",   4, { 8, 9, 10, 11 }, { -7, -5, -6, -4 }, { 5, 6, 7, 8 },
      curie2ru_prog_intel_mac },
    { "Quad1G",   4, { 0, 1,  2,  3 }, { 0, 1, 2, 3 },     { 1, 2, 3, 4 },
      curie2ru_prog_intel_mac },
    { "BCM75412", 4, { 4, 5,  6,  7 }, { 4, 5, -3, -2 },   { 0, },
      curie2ru_prog_bcm57412_mac },
};

/* on success, it returns a positive offset and the mac address.
 * on error, it returns -1 */
static int
get_assigned_mac_addr(int index, uint8_t *mac_base, size_t block_size,
                      uint8_t *mac_buf)
{
    int i, j, offset;
    struct mac_addr_map *map, *ptr;
    size_t map_size;

    if (is_curie_2ru()) {
        map = curie2ru_mac_map;
        map_size = CURIE2RU_MAC_MAP_SIZE;
    } else
        return -1;

    for (i = 0; i < map_size; i++)  {
        ptr = &map[i];
        for (j = 0; j < ptr->count; j++) {
            if (ptr->index[j] == index) {
                offset = ptr->offset[j];
                if (offset < 0)
                    offset += block_size;
                if (mac_buf)
                    get_mac_from_block(offset, mac_buf);
                return offset;
            }
        }
    }

    return -1;
}

static int mac_base_check(uint8 *mac_base)
{
    uint8_t mac_addr[6], mac_zero[6] = { 0 };

    if (!mac_base)
        get_mac_from_block(0, mac_addr);
    else
        memcpy(mac_addr, mac_base, 6);

    if (memcmp(mac_addr, mac_zero, 6) == 0) {
        printf("error getting mac base addr; did u run cookie util yet?\n");
        printf("Please run 'alter mb cpu cookie' and 'display cookie'"
               " content at least once.\n");
        return FAILED;
    }

    return PASSED;
}

/* mac_base and block_size are from cookie */
static int
curie2ru_prog_mac_addr(int port_type, uint8_t *mac_base, size_t block_size)
{
    struct mac_addr_map *map;

    if (port_type >= CURIE2RU_MAC_MAP_SIZE || port_type < 0)
        return FAILED;

    if (mac_base_check(mac_base) != PASSED)
        return FAILED;

    map = &curie2ru_mac_map[port_type];

    return map->prog_mac(map, mac_base, block_size);

    return PASSED;
}

/**********************************************************************
 *
 * Function: program_eth_mac
 *
 * Description: entry point to execute diag_util script program_x86_mac
 *              and bcm57412 script load.sh
 *
 * Input : port_type - 0 : Intel internal 10G ports 
 *                     1 : Intel I350 1G ports
 *                     2 : BCM57412 10G ports
 *                     
 * Output: PASSED
 *
 **********************************************************************
 */
static int program_eth_mac(int port_type)
{
    uchar buf[15] = { 0,0,0,0,0,0,0,0,0,0,0,0,0,0 }; /* has to be at least 6 */
    uchar bcm_buf[15] = { 0,0,0,0,0,0,0,0,0,0,0,0,0,0 }; /* has to be at least 6 */
    int32_t size = 0;
    unsigned int port = 0, max_eth_num;
    char cmd[128];
    int nic_num = 0;

    /* Curie size is 143 */
    size = get_mac_blk_size() - 1;
    printf("size is %d\n", size);
    if (size < 10) {
        printf("data size = %d. it's too small.\n", size);
        printf("Please run 'alter mb cpu cookie' option at least once.\n");
        return(FAILED);
    }

    if (is_curie_2ru())
        return curie2ru_prog_mac_addr(port_type, NULL, size + 1);

    if (is_radium() || is_thallium()) {
        if (port_type == BCM57412_PORTS) {
            if (check_bcm57412_driver() == FAILED) {
                printf("Unable to check bcm57412 driver status\n");
            }
        }
    }

    /* Curie:
     * x86 CPU has 4 1G ports and 4 10G ports. The managemenet port is implemented
     * with i350. The other 4 10G ports are the built-in 10G ports on the CPU.
     * In the kernel, these ports are eth0-3 and eth6-9 as shown below.
     * eth0-3 is management port (i350, mac base + blocksize - 1).
     * eth6-9 is CPU 10G port
     *
     * Cisco convention assigns to the management port the last MAC address of the
     * MAC address block assigned to the system.
     * Chasis mac base and blocksize are values stored in MB cookie.
     *
     * The MAC address progarmming of this ports uses the Intel
     * eepudate utility. The utility is put under
     * /diag_utils/intel-eeupdate-tool in the kernel rootfs.
     */
    
    /* This for-loop is just for code sharing purpose. 
     * Port means really ETH number.
     * ETH 0-3 are for the I350.
     * ETH 4-5 are for the BCM57412 NIC
     * ETH 6-9 are for the Intel internal 10G ports. 
     * Note - By typing "celo64e /deviecs" can know the NIC number for each I350 and
     * Intel internal 10G ports.
     */

    if (port_type == I350_PORTS) {
        /* I350 1G ports */
        port = ETH1;
        max_eth_num = ETH3;
    } else if (port_type == BCM57412_PORTS) {
        /* BCM57412 10G ports */
        port = ETH5;
        max_eth_num = ETH5;
    } else {
        /* Intel internal 10G ports */
        port = ETH7;
        max_eth_num = ETH9;
    }

    /* 
     * port is linux eth number
     * this loop is used to get NIC for
     * I350 and Intel internal 10G ports
     *
     * MAC address assignment
     * I350 linux eth number 0-3: MAC address 0-3
     * BCM linux eth number 4-5: MAC address 4-5
     * Intel internal 10G port1 linux eth6: MAC address 140
     * Intel internal 10G port2 linux eth7: MAC address 142
     * Intel internal 10G port3 linux eth8: MAC address 139
     * Intel internal 10G port4 linux eth9: MAC address 141
     */
    for (port = (port - 1); port <= max_eth_num; port++) {
        switch (port) {
        case ETH0:
            nic_num = NIC5;
            /* get_mac_from_block(size) give you the last MAC address of block */
            get_mac_from_block(port, buf);
            break;
        case ETH1:
            nic_num = NIC6;
            get_mac_from_block(port, buf);
            break;
        case ETH2:
            nic_num = NIC7;
            get_mac_from_block(port, buf);
            break;
        case ETH3:
            nic_num = NIC8;
            get_mac_from_block(port, buf);
            break;
        case ETH4:
            get_mac_from_block(port, buf);
            break;
        case ETH5:
            get_mac_from_block(port, bcm_buf);
            break;
        case ETH6:
            nic_num = NIC1;
            get_mac_from_block(size - 3, buf);
            break;
        case ETH7:
            nic_num = NIC2;
            get_mac_from_block(size - 1, buf);
            break;
        case ETH8:
            nic_num = NIC3;
            get_mac_from_block(size - 4, buf);
            break;
        case ETH9:
            nic_num = NIC4;
            get_mac_from_block(size - 2, buf);
            break;
        }

        if (port == ETH5) {
            /* Here is using bcm_buf to store BCM57412 port2(ETH5) MAC */
	    if (!((*bcm_buf) || *(bcm_buf+1) || *(bcm_buf+2) || *(bcm_buf+3) || *(bcm_buf+4) ||
	          (*bcm_buf+5))) {
	        printf("error getting mac base addr; did u run cookie util yet?\n");
	        printf("Please run 'alter mb cpu cookie' and 'display cookie'"
		       " content at least once.\n");
	        return(FAILED);
	    }

	    printf("eth%d: 0x%02x:0x%02x:0x%02x:0x%02x:0x%02x:0x%02x\n",
	           port, bcm_buf[0], bcm_buf[1], bcm_buf[2], bcm_buf[3], bcm_buf[4], bcm_buf[5]);
        } else {
	    if (!((*buf) || *(buf+1) || *(buf+2) || *(buf+3) || *(buf+4) ||
	          (*buf+5))) {
	        printf("error getting mac base addr; did u run cookie util yet?\n");
	        printf("Please run 'alter mb cpu cookie' and 'display cookie'"
		       " content at least once.\n");
	        return(FAILED);
	    }

	    printf("eth%d: 0x%02x:0x%02x:0x%02x:0x%02x:0x%02x:0x%02x\n",
	           port, buf[0], buf[1], buf[2], buf[3], buf[4], buf[5]);
        }

        /* 
         * Do nothing when port is 4 because BCM57412 needs to program
         * port1(ETH4) and port2(ETH5) MAC at the same time by using CDiag tool
         * and type exit to leave CDiag tool prompt
         */
        if (port == ETH4 || port == ETH5) {
            if (port == ETH5) {
                system(RM_BCM57412_DRIVER);
                /* Switch to /curie-2RU-diag/ dir in order to execute script load.sh */
                chdir(BCM57412_DIR);
	        sprintf(cmd, "%s <<< $'%.2x%.2x%.2x%.2x%.2x%.2x\n%.2x%.2x%.2x%.2x%.2x%.2x\nexit\n'", BCM57412_SCRIPT,
	            buf[0], buf[1], buf[2], buf[3], buf[4], buf[5], 
                    bcm_buf[0], bcm_buf[1], bcm_buf[2], bcm_buf[3], bcm_buf[4], bcm_buf[5]);
	        printf("%s\n", cmd);
	        system(cmd);
                msleep(10);
                system(INSERT_BCM57412_DRIVER);
            }
        } else {
	    sprintf(cmd, "%s/%s %d %.2x%.2x%.2x%.2x%.2x%.2x", INTEL_EEUPDATE_DIR, PROGRAM_X86_MAC,
		nic_num, buf[0], buf[1], buf[2], buf[3], buf[4], buf[5]);
	    printf("%s\n", cmd);
	    system(cmd);
        }
    }

    return (PASSED);
}

static int verify_single_port(int port, uint8_t *mac_addr)
{
    char cmd[128];
    char *check_mac_file = "/curie-2RU-diag/check_mac.txt";
    char rd_mac[128];
    char cookie_mac[128];
    int rc;
    uint8_t *buf = mac_addr;
    FILE *fp;

    sprintf(cookie_mac, "%.2x:%.2x:%.2x:%.2x:%.2x:%.2x",
            buf[0], buf[1], buf[2], buf[3], buf[4], buf[5]);

    sprintf(cmd, "ifconfig eth%d | grep HWaddr | sed -n 's/.*'HWaddr'//p' | sed s/[[:space:]]//g > %s",
            port, check_mac_file);
    system(cmd);

    fp = fopen(check_mac_file, "r");
    if (fp == NULL) {
        printf("Unable to open file - %s\n", check_mac_file);
        return FAILED;
    }

    rc = FAILED;

    rd_mac[0] = 0;
    fgets(rd_mac, sizeof(rd_mac), fp);
    if (rd_mac[0])
        rd_mac[strlen(rd_mac) - 1] = 0;
    if (strcasestr(rd_mac, cookie_mac) != NULL) {
        rc = PASSED;
    }

    sprintf(cmd, "rm -f %s", check_mac_file);
    system(cmd);
    fclose(fp);

    if (rc == PASSED) {
        printf("eth%d MAC program successfully! - %s\n", port, rd_mac);
    } else {
        printf("eth%d MAC didn't program correctly! expected - %s, got - %s\n",
               port, cookie_mac, rd_mac);
    }

    return rc;
}

static int
curie2ru_verify_mac_prog_result(uint8_t *mac_base, size_t block_size)
{
    int i, j, err = 0;
    struct mac_addr_map *map = curie2ru_mac_map;
    uint8_t mac_addr[6];

    if (mac_base_check(mac_base) != PASSED)
        return FAILED;

    for (i = 0; i < CURIE2RU_MAC_MAP_SIZE; i++) {
        printf("Verifying %s..\n", map[i].name);
        for (j = 0; j < map[i].count; j++) {
            int port = map[i].index[j];

            get_assigned_mac_addr(port, mac_base, block_size, mac_addr);
            if (verify_single_port(port, mac_addr) != PASSED) {
                err++;
            }
        }
    }

    return err ? FAILED : PASSED;
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
    char *check_mac_file = "/curie-1RU-diag/check_mac.txt";
    char rd_mac[128];
    int rc;
    char cookie_mac[128];

    size = get_mac_blk_size() - 1;
    if (size < 10 ) {
        printf("data size = %d. it's too small.\n", size);
        printf("Please run 'alter mb cpu cookie' option at least once.\n");
        return (FAILED);
    }

    if (is_radium() || is_thallium()) {
        if (check_bcm57412_driver() == FAILED) {
            printf("Unable to check bcm57412 driver status\n");
        }
    }

    if (is_curie_2ru())
        return curie2ru_verify_mac_prog_result(NULL, size + 1);

    for (port = ETH0; port <= ETH9; port++) {
        rc = FAILED;
        switch (port) {
        case ETH0:
            get_mac_from_block(port, buf);
            break;
        case ETH1:
            get_mac_from_block(port, buf);
            break;
        case ETH2:
            get_mac_from_block(port, buf);
            break;
        case ETH3:
            get_mac_from_block(port, buf);
            break;
        case ETH4:
            get_mac_from_block(port, buf);
            break;
        case ETH5:
            get_mac_from_block(port, buf);
            break;
        case ETH6:
            get_mac_from_block(size - 3, buf);
            break;
        case ETH7:
            get_mac_from_block(size - 1, buf);
            break;
        case ETH8:
            get_mac_from_block(size - 4, buf);
            break;
        case ETH9:
            get_mac_from_block(size - 2, buf);
            break;
        }

	if (!((*buf) || *(buf+1) || *(buf+2) || *(buf+3) || *(buf+4) ||
	      (*buf+5))) {
	    printf("error getting mac base addr; did u run cookie util yet?\n");
	    printf("Please run 'alter mb cpu cookie' and 'display cookie'"
		   " content at least once.\n");
	    return (rc);
	}

	sprintf(cookie_mac, "%.2x:%.2x:%.2x:%.2x:%.2x:%.2x",
		buf[0], buf[1], buf[2], buf[3], buf[4], buf[5]);

        sprintf(cmd, "ifconfig eth%d | grep HWaddr | sed -n 's/.*'HWaddr'//p' | sed s/[[:space:]]//g | tr '[:upper:]' '[:lower:]'> %s", port, check_mac_file);
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
            printf("eth%d MAC program successfully! expected - %s, got - %s\n", port, cookie_mac, rd_mac);
        } else {
            printf("Error - eth%d MAC didn't program correctly! expected - %s, got - %s\n", port, cookie_mac, rd_mac);
            return (rc);
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
   /* curie_skip_ngio_slots init from linux_main.c */
   if (curie_skip_ngio_slots & skip_ngio) {
       return (FALSE);  /* hide */ 
   } else {
       return (TRUE);   /* display */
   }
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
static void lpc_power_cycle (void)
{
    assert(dash_cpld);

    rst_cpld_t *cpld = (rst_cpld_t*)dash_cpld;
    cpld->pwr = LPC_POWER_CYCLE_KEY1;
    sleep(1);
    cpld->pwr = LPC_POWER_CYCLE_KEY2;
    return;
}

/*
 *-----------------------------------------------------------------------------
$Log: diag.c,v $
Revision 1.4  2021/03/31 10:20:56  xiaolaya
Add M2 Testcard support

Revision 1.3  2021/02/24 03:46:38  xiaolaya
Fix bug for Switzer-Carrier SM Daughter NIM Daughter VM cookie

Revision 1.2  2020/03/11 17:46:59  jiajliu
Refine code for bcm utlity and test

Revision 1.1  2020/01/09 01:01:59  jiajliu
Merge Curie 2RU to main trunk

$Endlog$
 *-----------------------------------------------------------------------------
 */
