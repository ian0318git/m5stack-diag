/* $Id: diag.c,v 1.5 2021/03/31 10:20:49 xiaolaya Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/overlord/neptune/curie_1RU/diag.c,v $
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
#include "platform_eeprom_access.h"
#include "ethernet.h"
#include "platform_pci.h"
#include "plat_defs.h"   
#include "platform_slot.h"
#include <unistd.h>
#include "plug_host_fpga_lib.h"
#include "plug_slot.h"
#include "bcm57412_test.h"
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

extern struct menuinfo *pci_menup;
extern struct menuinfo *spi_prom_menup;
extern int curie_1ru_skip_ngio_slots; 
extern char *strcasestr(char* , char*);
extern int program_reggio_spi_prom(void);
extern void tam_aikido_reset_utilty(void); 
extern int check_bcm57412_driver(void);
extern int plug_lte_modem_pwr_down_seq(struct plug_intf_t *);

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
    {"Enable/Disable NGIO, PIM",           0, 0,
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
     (type_t(*)())0, 0,         (PFT)wic_test, FIRST_SLOT + MAX_WIC},
    {"test SM Slot 1",
     (PFT)sm_test,              FIRST_SLOT,             MM_2,
     (PFT)support_ngio, SKIP_SM1,     (PFT)sm_test, FIRST_SLOT + MAX_SM},
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
     (PFT)m2_testcard_test,     TRUE,       MM_2,
     (PFT)is_m2_testcard_in, 0, (PFT)m2_testcard_test,  FALSE},
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
    char *result_file = "/curie-1RU-diag/skip_test.txt";
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
    struct plug_intf_t *plug;

    b = getdec_answer("enter module type SM/NIM/PIM [1/2/3]", 1, 1, 3);
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
        plug = (struct plug_intf_t *)slot_get_plugslot(c);
        break;
    default:
        printf("you won't come here \n");
    }
    
    if (d) {
        if (b == 3) {
            plug->on(plug);
            if (plug->i2c_unreset(plug) < 0) {
                cterr('f', 0, "slot%d power_ok bit not set", plug->slot);
            }
            plug->uart_on(plug);
            plug->unreset(plug);
        } else {
            ngio->on(ngio);
            if (ngio->i2c_unreset(ngio) < 0) {
                cterr('f', 0, "slot%d power_ok bit not set", ngio->slot);
            }
            ngio->uart_on(ngio);
            ngio->unreset(ngio);
        }
    } else {
        if (b == 3) {
            plug->get_id((void *)plug, '\0');
            if ((plug->id == PLUGGABLE_LTE_EM) ||
                (plug->id == PLUGGABLE_LTE_WP7601) ||
                (plug->id == PLUGGABLE_LTE_WP7603) ||
                (plug->id == PLUGGABLE_LTE_WP7607) ||
                (plug->id == PLUGGABLE_LTE_WP7608) ||
                (plug->id == PLUGGABLE_LTE_WP7609)) {
                plug_lte_modem_pwr_down_seq(plug);
            }
            msleep(PLUG_PWR_OFF_DELAY);
            plug->off(plug);
            plug->reset(plug);
        } else {
            ngio->off(ngio);
            ngio->reset(ngio);
        }
    }
    
    return (PASSED);
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
                /* Switch to /curie-1RU-diag/ dir in order to execute script load.sh */
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
   /* curie_1ru_skip_ngio_slots init from linux_main.c */
   if (curie_1ru_skip_ngio_slots & skip_ngio) {
       return (FALSE);  /* hide */ 
   } else {
       return (TRUE);   /* display */
   }
}


/*---------------------------------------------------------------
$Log: diag.c,v $
Revision 1.5  2021/03/31 10:20:49  xiaolaya
Add M2 Testcard support

Revision 1.4  2021/02/24 03:46:34  xiaolaya
Fix bug for Switzer-Carrier SM Daughter NIM Daughter VM cookie

Revision 1.3  2020/12/29 03:09:03  leschen
Remove bnxt_en operations.

Revision 1.2  2019/08/06 06:56:12  alpeng
merge curie, switzer and nightwatch to trunk

Revision 1.1.2.38  2019/07/30 02:17:42  alpeng
add mfg key re-prog

Revision 1.1.2.37  2019/07/24 08:32:38  alpeng
merge trunk to branch

Revision 1.1.2.36  2019/04/08 03:25:07  meho
Fixed bug of power on PIM utility.

Revision 1.1.2.35  2019/04/08 03:14:19  meho
Added toggle the power of PIM in FPGA basic util.

Revision 1.1.2.34  2019/01/17 08:50:22  leschen
Remove/insert bnxt_en driver when launching/exit diag to support bcm57412 sm.

Revision 1.1.2.33  2019/01/15 09:43:19  leschen
Fix the problem of checking mac status.

Revision 1.1.2.32  2019/01/11 15:33:26  alpeng
update aikido get chip info; and util menu naming

Revision 1.1.2.31  2018/12/20 07:17:50  alpeng
a. skip aikido reset on platfrom code, instead of common code; b. add aikido soft reset utility

Revision 1.1.2.30  2018/12/12 03:43:18  meho
Added alter PIM cookie utility

Revision 1.1.2.29  2018/11/27 06:52:03  alpeng
remove cavecreek cookie program

Revision 1.1.2.28  2018/11/13 09:11:38  alpeng
move the aikido fpga upgrade item to 3rd, in case the order is not match the documentation

Revision 1.1.2.27  2018/11/09 01:00:29  alpeng
update fpga utility for adding aikido upgrade item

Revision 1.1.2.26  2018/10/29 10:25:21  alpeng
remove PoE cookie util

Revision 1.1.2.25  2018/10/16 09:05:39  meho
Pluggable re-structured

Revision 1.1.2.24  2018/10/15 10:47:05  alpeng
add new func set_ngio_now_test() for platform to assign correct host eth port number for ngio

Revision 1.1.2.23  2018/09/27 09:46:24  alpeng
support tam lib and aikido for curie

Revision 1.1.2.22  2018/09/18 22:36:33  ptong
Add i2c address in print statement. Use PIM for pluggable in test menu

Revision 1.1.2.21  2018/09/07 01:43:58  alpeng
add spi read/write util for aikido; change tam lib on Makefile

Revision 1.1.2.20  2018/09/07 01:34:44  leschen
Support Curie MAC checking

Revision 1.1.2.19  2018/09/07 00:09:36  ptong
Make skip plugin work on Cuire-1RU. Remove AUX port test

Revision 1.1.2.18  2018/09/04 22:44:09  ptong
Move pluggable test to end of menu

Revision 1.1.2.17  2018/08/27 20:31:39  alpeng
using bridge (set on rcS) to resolve sharing ip 192.123.123.1 problem

Revision 1.1.2.16  2018/08/24 19:07:04  meho
Fixed plug i2c address

Revision 1.1.2.15  2018/08/23 22:53:03  leschen
Support Curie MAC programming for Intel 10G/I350/BCM57412

Revision 1.1.2.14  2018/08/20 18:09:38  alpeng
revert change since it breaks NGIO test

Revision 1.1.2.13  2018/08/16 23:07:52  meho
Fixed compile error

Revision 1.1.2.12  2018/08/16 21:19:52  meho
Rename test pluggable slot in main menu

Revision 1.1.2.11  2018/08/16 18:22:38  alpeng
remove useless info on i2c_drv; fixed get_sgmii_port on platform_eth_pkt_txrx.c for curie; add wrapper for wic_test and sm_test for prepare eth info on platform_slot.c; support ge1 for SM on testcard;

Revision 1.1.2.10  2018/08/10 08:15:53  alpeng
update eth num for sm1; support sync signal test for curie; skip plx pcie check (wait for rommon); skip ge1 on SM for both tlk10232 and xaui loopback test (wait for rommon); remove NIM slot2 test; fixed pcie lane scan test

Revision 1.1.2.9  2018/08/08 20:13:47  meho
Temp skip pluggable test card init

Revision 1.1.2.8  2018/07/31 08:56:53  alpeng
 remove Cavium related uart items and clean up useless extern functions

Revision 1.1.2.7  2018/07/30 08:36:54  meho
Added Pluggable LTE test item in menu

Revision 1.1.2.6  2018/07/30 08:15:39  alpeng
remove nim3, sm2,3,4 entry; update pcie scan test, except nvme (vid/did/ need to verify with HW

Revision 1.1.2.5  2018/07/19 09:27:37  alpeng
1. Moving IR3570 chips to CPU I2C bus. 2. Removed related code of i2c devices which are not support on Curie; max1617, IDT8T49N287I(sys clk), poe psu, 30w poe

Revision 1.1.2.4  2018/07/10 07:27:23  alpeng
add is_radium() and is_polonium(); remove pcie switch files from Makefile

Revision 1.1.2.3  2018/06/28 10:19:19  alpeng
remove bcm gesw files from Makefile and put its functions into platform_stub.c for NGIO reference; will follow GB method on NGIO GE SW portion

Revision 1.1.2.2  2018/06/28 07:38:26  alpeng
remove data plane and timing card portions for curie 1RU;

Revision 1.1.2.1  2018/06/22 08:05:17  alpeng
move curie diag to neptune/curie_1RU directory

Revision 1.1.2.1  2018/05/30 02:39:36  alpeng
porting neptune x86 to curie

$Endlog$
*/
