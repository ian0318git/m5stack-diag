/* $Id: sm_patriot.c,v 1.31 2018/05/18 09:24:48 alpeng Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/sm_patriot.c,v $
 *******************************************************************************
 * File Name: sm_patriot.c
 *
 * Description: Patriot main source file
 *
 *      
 * Author: Huan Ngo
 * Copyright (c)2011 - 2018 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *******************************************************************************
 */
#include "endians.h"
#include "types.h"
#include "defs.h"
#include "signals.h"
#include "error.h"
#include "nvmonvars.h"
#include "nvsysvars.h"
#include "pm_utils.h"
#include "pci.h"
#include "slot.h"
#include "mon_plat_defs.h"
#include "common.h"
#include "common_utils.h"
#include "cookie_4.h"
#include "proto.h"
#include "strings.h"
#include "sm_slot.h"
#include "menu.h"
#include "router_if.h"
#include "sgmii_defs.h"
#include "dev_ltc4215.h"
#include "oir_ltc4215_api.h"
#include "platform_slot.h"
#include "sm_patriot.h"
#include "cli_cmd.h"
#include "cross_platform.h"
#include "platform_i2c.h"
#include "dash_fpga.h" /* for externs...*/
#include "linux_ntwk.h"
#include "ngio.h"
#include "linux_api.h"
#include "pca.h"

#include <stdio.h>
#include <assert.h>


/*===================================================================*
 *                             Globals                               *
 *===================================================================*/
patriot_ds_t patriot_iface[MAX_SM+1];
static patriot_ds_t *patriot_iface_p;

extern mac_addr_t patriot_sm1_mac;
extern mac_addr_t patriot_sm2_mac;

extern mac_addr_t host_sm1_mac;
extern mac_addr_t host_sm2_mac;

extern int mvl_sw_cleanup(int, int);

static int patriot_utility_submenu (int show_menu);
static int patriot_ds3170_main_test(patriot_ds_t *iface);
static int patriot_ds3170_submenu (patriot_ds_t *iface);
static int patriot_loopback_main_test(patriot_ds_t *iface);
static int patriot_loopback_submenu (patriot_ds_t *iface);
static int patriot_intr_main_test(patriot_ds_t *iface);
static int patriot_intr_submenu (patriot_ds_t *iface);
int patriot_sm_cleanup(patriot_ds_t *iface);
static int ltc4215_reg_read(patriot_ds_t *iface);
static int ltc4215_reg_write(patriot_ds_t *iface);
static int ltc4215_register_test(patriot_ds_t *iface);
static int ltc4215_led_test(patriot_ds_t *iface);
static int patriot_power_off (patriot_ds_t *iface);
static int patriot_power_on (patriot_ds_t *iface);
static int patriot_power_cycle (patriot_ds_t *iface);


#define FLAGS MF_CONTINUOUS | MF_DOALL | MF_SHOW_ERRCOUNT

static mitem_t        ds3170_menu_items[MAX_SUBTEST_ITEMS];
static title_buf_t    ds3170_menu_title[MAX_SUBTEST_ITEMS];
static title_buf_t    ds3170_menu_header;

static mitem_t        loop_menu_items[MAX_SUBTEST_ITEMS];
static title_buf_t    loop_menu_title[MAX_SUBTEST_ITEMS];
static title_buf_t    loop_menu_header;

static mitem_t        intr_menu_items[MAX_SUBTEST_ITEMS];
static title_buf_t    intr_menu_title[MAX_SUBTEST_ITEMS];
static title_buf_t    intr_menu_header;

static struct menuinfo *maindiagp;

pci_reg_spec_t p1016_cfg_reg_tbl[] = {
    {"Command and Status",                          0x00000546, 0x004},
    {"Base Address 0",                              0xfff00000, 0x010},
    /* Depending on PEXIWAR1, BAR1 has a number of writable bits */
    {"Base Address 1",                              0xf0000000, 0x014},
    /* Base Address 2,3,4,5 are not used, always 0's */
};

#define P1016_CFG_REG_TBL_SIZE \
                   sizeof(p1016_cfg_reg_tbl)/sizeof(pci_reg_spec_t)

/*******************************************************************************
 *                                   Menus                                     *
 *******************************************************************************
 */

static struct mitem patriot_subdiag[] = {    
    {adiagfstr,           0, 0, (PFT)menu,       
     (type_t *)&menu_diagflagp, 0,(type_t(*)())0, 0},
    {"basic utilities",   0, 0, (PFT)menu,       
     (type_t *)&utilmenup, 0,(type_t(*)())0, 0},
    {doalldgstr,          0, 0, (PFT)do_menu_all_diags, 
     (type_t *)&maindiagp, FLAGS,(type_t(*)())0, 0},
    {dogrpdgstr,          0, 0, (PFT)do_menu_grp_diags, 
     (type_t *)&maindiagp, FLAGS,(type_t(*)())0, 0},
    {"Patriot Utilities",    0, 0, (PFT)patriot_utility_submenu,
     (type_t *)&patriot_iface_p, 0,(type_t(*)())0, 0},
    {"OIR LTC4215 Register Test", 0, 0, (PFT)0,
     (type_t *)&patriot_iface_p, MF_CONTINUOUS | MF_DOALL | MF_SHOW_ERRCOUNT,
     (type_t(*)())0, 0},
    {"OIR LTC4215 LED Test", 0, 0, (PFT)0,
     (type_t *)&patriot_iface_p, MF_CONTINUOUS | MF_DOALL | MF_SHOW_ERRCOUNT,
     (type_t(*)())0, 0},
    {"CPU Alive Test",    0, 0, (PFT)0,
    (type_t *)&patriot_iface_p, 
    MF_CONTINUOUS | MF_DOALL | MF_SHOW_ERRCOUNT,
    (type_t(*)())0, 0},
    {"Memory Test",    0, 0, (PFT)0,
    (type_t *)&patriot_iface_p, 
    MF_CONTINUOUS | MF_DOALL | MF_SHOW_ERRCOUNT,
    (type_t(*)())0, 0},
    {"SPI PROM Test",    0, 0, (PFT)0,
    (type_t *)&patriot_iface_p, 
    MF_CONTINUOUS | MF_DOALL | MF_SHOW_ERRCOUNT,
    (type_t(*)())0, 0},
    {"FPGA Register Test",    0, 0, (PFT)0,
    (type_t *)&patriot_iface_p, 
    MF_CONTINUOUS | MF_DOALL | MF_SHOW_ERRCOUNT,
    (type_t(*)())0, 0},
    {"DS3170 Test",    0, 0, (PFT)patriot_ds3170_submenu,
    (type_t *)&patriot_iface_p, 0, (type_t(*)())0, 0},
    {"Loopback Test",    0, 0, (PFT)patriot_loopback_submenu,
    (type_t *)&patriot_iface_p, 0, (type_t(*)())0, 0},
    {"Interrupt Test",    0, 0, (PFT)patriot_intr_submenu,
    (type_t *)&patriot_iface_p, 0, (type_t(*)())0, 0},
    {"Switch Console",    0, 0, (PFT)0,
    (type_t *)&patriot_iface_p, 
    MF_SHOW_ERRCOUNT,
    (type_t(*)())0, 0},
    {"FPGA GPIO Framer GPIO Test",    0, 0, (PFT)0,
    (type_t *)&patriot_iface_p,
    MF_CONTINUOUS | MF_DOALL | MF_SHOW_ERRCOUNT,
    (type_t(*)())0, 0},
    {"Host to Module GPIO Test",    0, 0, (PFT)0,
    (type_t *)&patriot_iface_p,
    MF_CONTINUOUS | MF_DOALL | MF_SHOW_ERRCOUNT,
    (type_t(*)())0, 0},
    {"UART Test",    0, 0, (PFT)patriot_uart_test,
    (type_t *)&patriot_iface_p,
    MF_CONTINUOUS | MF_DOALL | MF_SHOW_ERRCOUNT,
    (type_t(*)())0, 0},
    {"DDR1 single bit ECC test",     0, 0,(PFT)patriot_memory_ecc_test,
     (type_t *)&patriot_iface_p,    MF_CONTINUOUS | MF_DOALL | MF_SHOW_ERRCOUNT,
     (type_t(*)())0, 0},
};

static struct mitem patriot_maindiag[] = {
    {adiagfstr,           0, 0, (PFT)menu,       
     (type_t *)&menu_diagflagp, 0,(type_t(*)())0, 0},
    {"basic utilities",   0, 0, (PFT)menu,       
     (type_t *)&utilmenup, 0,(type_t(*)())0, 0},
    {doalldgstr,          0, 0, (PFT)do_menu_all_diags, 
     (type_t *)&maindiagp, MF_CONTINUOUS,(type_t(*)())0, 0},
    {dogrpdgstr,          0, 0, (PFT)do_menu_grp_diags, 
     (type_t *)&maindiagp, MF_CONTINUOUS,(type_t(*)())0, 0},
    {"Patriot Utilities",    0, 0, (PFT)patriot_utility_submenu,
     (type_t *)&patriot_iface_p, 0,(type_t(*)())0, 0},
    {"OIR LTC4215 Register Test",    0, 0, (PFT)ltc4215_register_test,
    (type_t *)&patriot_iface_p, 
    MF_CONTINUOUS | MF_DOALL | MF_SHOW_ERRCOUNT,
    (type_t(*)())0, 0},
    {"OIR LTC4215 LED Test",    0, 0, (PFT)ltc4215_led_test,
    (type_t *)&patriot_iface_p, 
    MF_CONTINUOUS | MF_DOALL | MF_SHOW_ERRCOUNT,
    (type_t(*)())0, 0},
    {"CPU Alive Test",    0, 0, (PFT)patriot_cpu_alive_test,
    (type_t *)&patriot_iface_p, 
    MF_CONTINUOUS | MF_DOALL | MF_SHOW_ERRCOUNT,
    (type_t(*)())0, 0},
    {"Memory Test",    0, 0, (PFT)patriot_memory_test,
    (type_t *)&patriot_iface_p, 
    MF_CONTINUOUS | MF_DOALL | MF_SHOW_ERRCOUNT,
    (type_t(*)())0, 0},
    {"SPI PROM Test",    0, 0, (PFT)patriot_spi_prom_test_wrap,
    (type_t *)&patriot_iface_p, 
    MF_CONTINUOUS | MF_DOALL | MF_SHOW_ERRCOUNT,
    (type_t(*)())0, 0},
    {"FPGA Register Test",    0, 0, (PFT)patriot_fpga_reg_test,
    (type_t *)&patriot_iface_p, 
    MF_CONTINUOUS | MF_DOALL | MF_SHOW_ERRCOUNT,
    (type_t(*)())0, 0},
    {"DS3170 Test",    0, 0, (PFT)patriot_ds3170_main_test,
    (type_t *)&patriot_iface_p, 
    MF_CONTINUOUS | MF_DOALL | MF_SHOW_ERRCOUNT,
    (type_t(*)())0, 0},
    {"Loopback Test",    0, 0, (PFT)patriot_loopback_main_test,
    (type_t *)&patriot_iface_p, 
    MF_CONTINUOUS | MF_DOALL | MF_SHOW_ERRCOUNT,
    (type_t(*)())0, 0},
    {"Interrupt Test",    0, 0, (PFT)patriot_intr_main_test,
    (type_t *)&patriot_iface_p, 
    MF_CONTINUOUS | MF_DOALL | MF_SHOW_ERRCOUNT,
    (type_t(*)())0, 0},
    {"Switch Console",    0, 0, (PFT)patriot_switch_console,
    (type_t *)&patriot_iface_p, 
    MF_SHOW_ERRCOUNT,
    (type_t(*)())0, 0},
    {"FPGA GPIO Framer GPIO Test",    0, 0, (PFT)patriot_test_fpga_gpio_framer,
    (type_t *)&patriot_iface_p,
    MF_CONTINUOUS | MF_DOALL | MF_SHOW_ERRCOUNT,
    (type_t(*)())0, 0},
    {"Host to Module GPIO Test",    0, 0, (PFT)patriot_host_to_module_gpio_test,
    (type_t *)&patriot_iface_p,
    MF_CONTINUOUS | MF_DOALL | MF_SHOW_ERRCOUNT,
    (type_t(*)())0, 0},
    {"UART Test",    0, 0, (PFT)patriot_uart_test,
    (type_t *)&patriot_iface_p,
    MF_CONTINUOUS | MF_DOALL | MF_SHOW_ERRCOUNT,
    (type_t(*)())0, 0},
    {"DDR1 single bit ECC test",     0, 0,(PFT)patriot_memory_ecc_test,
     (type_t *)&patriot_iface_p,     MF_CONTINUOUS | MF_DOALL | MF_SHOW_ERRCOUNT,
     (type_t(*)())0, 0},
};

static struct menuinfo patriotmaindiag = {
    "Patriot SM Submenu",                /* title */
    (int)0 ,                                     /* title param */
    (PFT)menu_show_dflags,                       /* show diag flags */
    0,                                           /* generic prompt */
    sizeof(patriot_maindiag)/sizeof(struct mitem),/* size of menu */
    patriot_maindiag,
};


/******************************
 * Patriot utility menu tables
 ******************************
 */
static struct mitem patriot_util_items[] = {
    {"CPU Firmware download", 0, 0,(PFT)patriot_cpu_fw_download_util,
     (type_t *)&patriot_iface_p,        0,    (type_t(*)())0, 0}, 
    {"FPGA download",         0, 0,(PFT)patriot_fpga_download,
     (type_t *)&patriot_iface_p,       0,    (type_t(*)())0, 0},
    {"Display Module LED",    0, 0, (PFT)patriot_diplay_led,
     (type_t *)&patriot_iface_p,       0,    (type_t(*)())0, 0},
    {"SM Reset",              0, 0, (PFT)patriot_sm_reset,
     (type_t *)&patriot_iface_p,       0,    (type_t(*)())0, 0},
    {"CPU Reset",              0, 0, (PFT)patriot_cpu_reset,
     (type_t *)&patriot_iface_p,       0,    (type_t(*)())0, 0},
    {"FPGA Reset",          0, 0, (PFT)patriot_fpga_reset,
     (type_t *)&patriot_iface_p,       0,    (type_t(*)())0, 0},
    {"DS3170 Reset",          0, 0, (PFT)patriot_ds3170_reset,
     (type_t *)&patriot_iface_p,       0,    (type_t(*)())0, 0},
    {"LTC4215 Register Read",  0, 0,   (PFT)ltc4215_reg_read,
     (type_t *)&patriot_iface_p, 0, (type_t(*)())0, 0 },
    {"LTC4215 Register Write", 0, 0,   (PFT)ltc4215_reg_write,
     (type_t *)&patriot_iface_p, 0, (type_t(*)())0, 0 },
    {"SM Patriot Power Off",   0, 0,   (PFT)patriot_power_off,
     (type_t *)&patriot_iface_p, 0, (type_t(*)())0, 0 },
    {"SM Patriot Power On",   0, 0,   (PFT)patriot_power_on,
     (type_t *)&patriot_iface_p, 0, (type_t(*)())0, 0 },
    {"SM Patriot Power Cycle",   0, 0,   (PFT)patriot_power_cycle,
     (type_t *)&patriot_iface_p, 0, (type_t(*)())0, 0 },
    {"I2C IO Port Register Read", 0, 0,   (PFT)patriot_i2c_port_reg_read,
     (type_t *)&patriot_iface_p, 0, (type_t(*)())0, 0 },
    {"I2C IO Port Register Write", 0, 0,   (PFT)patriot_i2c_port_reg_write,
     (type_t *)&patriot_iface_p, 0, (type_t(*)())0, 0 },
    {"Display FPGA Version",         0, 0,(PFT)patriot_display_fpga_version,
     (type_t *)&patriot_iface_p,       0,    (type_t(*)())0, 0},
    {"Display FPGA Registers & Multiboot Info",0,0,(PFT)patriot_display_fpga_info,
     (type_t *)&patriot_iface_p,       0,    (type_t(*)())0, 0},
    {"Configure Power Margin",         0, 0,(PFT)patriot_config_power_margin,
     (type_t *)&patriot_iface_p,       0,    (type_t(*)())0, 0},
    {"Upgrade FPGA Download to SPI PROM",   0, 0,
     (PFT)patriot_upgrade_fpga_download_spi_prom,
     (type_t *)&patriot_iface_p,       0,    (type_t(*)())0, 0},
    {"Golden FPGA Download to SPI PROM",   0, 0,
     (PFT)patriot_golden_fpga_download_spi_prom,
     (type_t *)&patriot_iface_p,       0,    (type_t(*)())0, 0},
};

#define NUM_UTILS_TESTS sizeof(patriot_util_items)/sizeof(struct mitem)

static struct menuinfo patriot_utilmenu = {
    "Patriot Utilities Menu",
    0,
    (PFT)menu_show_dflags,
    0,
    NUM_UTILS_TESTS,
    patriot_util_items,
};

/* Make static for compiling */
struct menuinfo *patriot_utilmenup = &patriot_utilmenu; 

static struct mitem patriot_ds3170_items[] = {
    {"DS3170 Register Test",         0,0,  
     (PFT)patriot_ds3170_reg_test, (type_t *)&patriot_iface_p,
     MF_CONTINUOUS | MF_DOALL | MF_SHOW_ERRCOUNT,(type_t(*)())0, 0},
    {"Clear E3 AIS Test",         0,0,  
     (PFT)patriot_clear_e3_ais_test, (type_t *)&patriot_iface_p,
     MF_CONTINUOUS | MF_DOALL | MF_SHOW_ERRCOUNT,(type_t(*)())0, 0},
    {"Clear T3 Bert Test",         0,0,  
     (PFT)patriot_clear_t3_bert_test, (type_t *)&patriot_iface_p,
     MF_CONTINUOUS | MF_DOALL | MF_SHOW_ERRCOUNT,(type_t(*)())0, 0},

};

#define DS3170_TESTS_NUM sizeof(patriot_ds3170_items)/sizeof(struct mitem)
static struct menuinfo patriot_ds3170_menu = {
    "Patriot DS3170 Test Menu",
    (int)0 ,                   /* title param */
    (PFT)menu_show_dflags,     /* show diag flags */
    0,
    0,
    ds3170_menu_items,
};
static struct menuinfo *patriot_ds3170_menup = &patriot_ds3170_menu;

static struct mitem patriot_loop_items[] = {
    {"Freescale GE 0 Interface Loopback test",     0, 0,
     (PFT)patriot_ge0_loopback_test,
     (type_t *)&patriot_iface_p,     MF_CONTINUOUS | MF_DOALL | MF_SHOW_ERRCOUNT,
     (type_t(*)())0, 0},
    {"Freescale GE 1 Interface Loopback Test",         0,0,  
     (PFT)patriot_fs_lpbk_test, (type_t *)&patriot_iface_p,
     MF_CONTINUOUS | MF_DOALL | MF_SHOW_ERRCOUNT,(type_t(*)())0, 0},
    {"Freescale UCC Internal Loopback Test",         0,0,  
     (PFT)patriot_fs_ucc_lpbk_test, (type_t *)&patriot_iface_p,
     MF_CONTINUOUS | MF_DOALL | MF_SHOW_ERRCOUNT,(type_t(*)())0, 0},
    {"FPGA Loopback Test",         0,0,  
     (PFT)patriot_fpga_lpbk_test, (type_t *)&patriot_iface_p,
     MF_CONTINUOUS | MF_DOALL | MF_SHOW_ERRCOUNT,(type_t(*)())0, 0},    
    {"Clear T3 Loopback Test",         0,0,  
     (PFT)patriot_clear_t3_lpbk_test, (type_t *)&patriot_iface_p,
     MF_CONTINUOUS | MF_DOALL | MF_SHOW_ERRCOUNT,(type_t(*)())0, 0},
    {"Subrate T3 Loopback Test",         0,0,
     (PFT)patriot_subrate_t3_lpbk_test, (type_t *)&patriot_iface_p,
     MF_CONTINUOUS | MF_DOALL | MF_SHOW_ERRCOUNT,(type_t(*)())0, 0},
    {"Subrate T3 Individual Loopback Test",         0,0,
     (PFT)patriot_subrate_t3_ind_lpbk_test, (type_t *)&patriot_iface_p,
     MF_SHOW_ERRCOUNT,(type_t(*)())0, 0},
    {"Clear E3 Loopback Test",         0,0,  
     (PFT)patriot_clear_e3_lpbk_test, (type_t *)&patriot_iface_p,
     MF_CONTINUOUS | MF_DOALL | MF_SHOW_ERRCOUNT,(type_t(*)())0, 0},
    {"Subrate E3 Loopback Test",         0,0,  
     (PFT)patriot_subrate_e3_lpbk_test, (type_t *)&patriot_iface_p,
     MF_CONTINUOUS | MF_DOALL | MF_SHOW_ERRCOUNT,(type_t(*)())0, 0},
    {"Subrate E3 Individual Loopback Test",         0,0,
     (PFT)patriot_subrate_e3_ind_lpbk_test, (type_t *)&patriot_iface_p,
     MF_SHOW_ERRCOUNT,(type_t(*)())0, 0},

};

#define LOOP_TESTS_NUM sizeof(patriot_loop_items)/sizeof(struct mitem)
static struct menuinfo patriot_loop_menu = {
    "Patriot Loopback Test Menu",
    (int)0 ,                   /* title param */
    (PFT)menu_show_dflags,     /* show diag flags */
    0,
    0,
    loop_menu_items,
};
static struct menuinfo *patriot_loop_menup = &patriot_loop_menu;

static struct mitem patriot_intr_items[] = {
    {"Patriot FPGA Interrupt Test",         0,0,  
     (PFT)patriot_fpga_intr_test, (type_t *)&patriot_iface_p,
     MF_CONTINUOUS | MF_DOALL | MF_SHOW_ERRCOUNT,(type_t(*)())0, 0},
    {"Patriot Framer Interrupt Test",         0,0,  
     (PFT)patriot_framer_intr_test, (type_t *)&patriot_iface_p,
     MF_CONTINUOUS | MF_DOALL | MF_SHOW_ERRCOUNT,(type_t(*)())0, 0},
    
};

#define INTR_TESTS_NUM sizeof(patriot_intr_items)/sizeof(struct mitem)
static struct menuinfo patriot_intr_menu = {
    "Patriot Interrupt Test Menu",
    (int)0 ,                   /* title param */
    (PFT)menu_show_dflags,     /* show diag flags */
    0,
    0,
    intr_menu_items,
};
static struct menuinfo *patriot_intr_menup = &patriot_intr_menu;

n2g_i2c_if_t pca_i2c[] = {
    {
        .i2c_dev = SM_I2C_ADDR_IO_PORT,
    },
    {
        .i2c_dev = SM_I2C_ADDR_IO_PORT1,
    },
};

static char pca_buff0[256];
static char pca_buff1[256];
static n2g_i2c_if_t *oir;

    
/**********************************************************************
 *
 * Function: patriot_utility_submenu
 *
 * This function builds utility menu
 *
 * Input : None
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
static int
patriot_utility_submenu (int show_menu)
{
    menu(patriot_utilmenup, patriot_util_items, '\0');
    return (PASSED);
}
    

/**********************************************************************
 *
 * Function: patriot_ds3170_main_test
 *
 * This function run ds3170 tests for Patriot
 *
 * Input : iface  - Patriot ds info
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
static int
patriot_ds3170_main_test(patriot_ds_t *iface)
{

    if (patriot_ds3170_reg_test(iface)) {
	return (FAILED);
    }

    if (patriot_clear_e3_ais_test(iface)) {
	return (FAILED);
    }    

    if (patriot_clear_t3_bert_test(iface)) {
	return (FAILED);
    }    
    
    return (FAILED);

}



/**********************************************************************
 *
 * Function: patriot_ds3170_submenu
 *
 * This function run submenu for ds3170 tests for Patriot
 *
 * Input : iface  - Patriot ds info
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
static int 
patriot_ds3170_submenu(patriot_ds_t *iface)
{
    int  i;
		
    init_empty_menu(patriot_ds3170_menup, (long)ds3170_menu_header.title);

    /* Build DS3170 Menu */
    for (i = 0; i < DS3170_TESTS_NUM; i++) {

        sprintf(ds3170_menu_title[i].title, "%s", patriot_ds3170_items[i].mline);
        add_menu_item(&patriot_ds3170_menu, ds3170_menu_title[i].title,
                      patriot_ds3170_items[i].mfunc,
                      patriot_ds3170_items[i].mfparam,
                      patriot_ds3170_items[i].mflag);
    }
    menu(&patriot_ds3170_menu, (mitem_t *)0, '\0');
    return (PASSED);
}

/**********************************************************************
 *
 * Function: patriot_loopback_main_test
 *
 * This function run loopback tests for Patriot
 *
 * Input : iface  - Patriot ds info
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
static int
patriot_loopback_main_test(patriot_ds_t *iface)
{

    if (patriot_ge0_loopback_test(iface)) {
	return (FAILED);
    }

    if (patriot_fs_lpbk_test(iface)) {
	return (FAILED);
    }

    if (patriot_fs_ucc_lpbk_test(iface)) {
	return (FAILED);
    }    

    if (patriot_fpga_lpbk_test(iface)) {
	return (FAILED);
    }
    
    if (patriot_clear_t3_lpbk_test(iface)) {
	return (FAILED);
    }    

    if (patriot_subrate_t3_lpbk_test(iface)) {
	return (FAILED);
    }    

    if (patriot_clear_e3_lpbk_test(iface)) {
	return (FAILED);
    }    

    if (patriot_subrate_e3_lpbk_test(iface)) {
	return (FAILED);
    }    

    return (PASSED);

}


/**********************************************************************
 *
 * Function: patriot_ds3170_submenu
 *
 * This function run submenu for loopback tests for Patriot
 *
 * Input : iface  - Patriot ds info
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
static int 
patriot_loopback_submenu(patriot_ds_t *iface)
{
    int  i;

		
    init_empty_menu(patriot_loop_menup, (long)loop_menu_header.title);

    /* Build loopback Menu */
    for (i = 0; i < LOOP_TESTS_NUM; i++) {

        sprintf(loop_menu_title[i].title, "%s", patriot_loop_items[i].mline);

        add_menu_item(&patriot_loop_menu, loop_menu_title[i].title,
                      patriot_loop_items[i].mfunc,
                      patriot_loop_items[i].mfparam,
                      patriot_loop_items[i].mflag);
    }
    menu(&patriot_loop_menu, (mitem_t *)0, '\0');
    return (PASSED);
}


/**********************************************************************
 *
 * Function: patriot_intr_main_test
 *
 * This function run interrupt tests for Patriot
 *
 * Input : iface  - Patriot ds info
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
static int
patriot_intr_main_test(patriot_ds_t *iface)
{

    if (patriot_fpga_intr_test(iface)) {
	return (FAILED);
    }

    if (patriot_framer_intr_test(iface)) {
	return (FAILED);
    }
    
    return (PASSED);

}


/**********************************************************************
 *
 * Function: patriot_intr_submenu
 *
 * This function run submenu for interrupt tests for Patriot
 *
 * Input : iface  - Patriot ds info
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
static int 
patriot_intr_submenu(patriot_ds_t *iface)
{
    int  i;

		
    init_empty_menu(patriot_intr_menup, (long)intr_menu_header.title);

    /* Build loopback Menu */
    for (i = 0; i < INTR_TESTS_NUM; i++) {

        sprintf(intr_menu_title[i].title, "%s", patriot_intr_items[i].mline);

        add_menu_item(&patriot_intr_menu, intr_menu_title[i].title,
                      patriot_intr_items[i].mfunc,
                      patriot_intr_items[i].mfparam,
                      patriot_intr_items[i].mflag);
    }
    menu(&patriot_intr_menu, (mitem_t *)0, '\0');
    return (PASSED);
}


/**********************************************************************
 *
 * Function: patriot_sm_cleanup
 *
 * Description: This function perform the cleanup task before exiting
 *              the test.
 *
 * Input : None
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
int patriot_sm_cleanup(patriot_ds_t *iface)
{
    struct ngio_intf_t *ngio;

    ngio = (struct ngio_intf_t *)slot_get_ngiosm(iface->slot);

    ngio->uart_off(ngio);
    ngio->reset(ngio);

    msleep(1000);
    
    return(PASSED);
}


/**********************************************************************
 *
 * Function: patriot_fs_pci_conf_setup
 *
 * This function setup the Freescale P1012 PCI configuration registers
 *
 * Input : iface  - Patriot ds info
 *
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
int patriot_fs_pci_conf_setup(patriot_ds_t *iface)
{
    int     pci_dev_num, slot;
    ulong   host_base_addr;

    slot           = iface->slot;
    pci_dev_num    = get_pci_dev_num(slot, 0);  /* Same as Apex Zeta */
    host_base_addr = get_pci_device_base((uint)slot, (uint)pci_dev_num);

    iface->host_pci_base_addr = host_base_addr;
    iface->nm_pci_base_addr   = 
           get_pci_device_base_offset((uint)iface->slot, (uint)pci_dev_num);
    
    return(PASSED);
}


/**********************************************************************
 *
 * Function: patriot_main_test
 *
 * This function runs the main test for Patriot
 *
 * Input : iface  - Patriot ds info
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
int
patriot_main_test(patriot_ds_t *iface)
{
    
    if (ltc4215_register_test(iface)) {
	return (FAILED);
    }

    if (patriot_cpu_alive_test(iface)) {
	return (FAILED);
    }

    if (patriot_display_fpga_version(iface)) {
	return (FAILED);
    }

    if (patriot_host_to_module_gpio_test(iface)) {
	return (FAILED);
    }

    if (patriot_uart_test(iface)) {
 	return (FAILED);
    }

    if (patriot_memory_test(iface)) {
	return (FAILED);
    }

    if (patriot_memory_ecc_test(iface)) {
	return (FAILED);
    }

    if (patriot_spi_prom_test(iface, 0)) {
	return (FAILED);
    }    
    
    if (patriot_fpga_reg_test(iface)) {
	return (FAILED);
    }

    if (patriot_fpga_intr_test(iface)) {
	return (FAILED);
    }

    if (patriot_test_fpga_gpio_framer(iface)) {
    	return (FAILED);
    }

    if (patriot_ge0_loopback_test(iface)) {
	return (FAILED);
    }

    if (patriot_fs_lpbk_test(iface)) {
	return (FAILED);
    }    

    if (patriot_fs_ucc_lpbk_test(iface)) {
	return (FAILED);
    }

    if (patriot_fpga_lpbk_test(iface)) {
	return (FAILED);
    }

    if (patriot_ds3170_reg_test(iface)) {
	return (FAILED);
    }

    if (!((NVRAM)->diagflag & D_EXT_LOOPBACK)) {
	if (patriot_clear_e3_ais_test(iface)) {
	    return (FAILED);
	}
    }

    if (patriot_framer_intr_test(iface)) {
	return (FAILED);
    }    


    if (!((NVRAM)->diagflag & D_EXT_LOOPBACK)) {
	if (patriot_clear_t3_bert_test(iface)) {
	    return (FAILED);
	}
    }

    if (patriot_clear_t3_lpbk_test(iface)) {
	return (FAILED);
    }

    if (patriot_subrate_t3_lpbk_test(iface)) {
	return (FAILED);
    }    

    if (patriot_clear_e3_lpbk_test(iface)) {
	return (FAILED);
    }

    if (patriot_subrate_e3_lpbk_test(iface)) {
	return (FAILED);
    }    
    
    return (PASSED);    

}

/**********************************************************************
 *
 * Function: configure_ltc4215_and_io_port
 *
 * This function configure ltc4215 and IO port PCA9557
 *
 * Input : iface  - Patriot ds info
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
int
configure_ltc4215_and_io_port(patriot_ds_t *iface)
{
    uint8_t i, data = 0;
    uchar io_port_conf = 0;
    n2g_i2c_if_t *pca1;
    pca1 = &pca_i2c[1];
    struct ngio_intf_t *ngio;

    ngio = iface->patriot_sm_iface;

    if (ngio->is_present(ngio)) {
        ngio->reset(ngio);
    }
    
    /* Enable power for the SM via hot swap controller */
    if (patriot_power_on(iface)==FAILED) {
        return(FAILED);
    }

    /* Read the LTC215 Control Reg */
    if (oir_ltc4215_reg_read((void *)oir, LTC4215_CONTROL_REG,
			     &data)) {
        return(FAILED);
    }

    /* Set GPIO1 to General Purpose input */
    data |= LTC4215_GPIO1_GENRAL_PURPOSE_INPUT;
    if (oir_ltc4215_reg_write((void *)oir, LTC4215_CONTROL_REG,
			      &data)) {
        return(FAILED);
    }

    msleep(10);

    for (i = 0; i < 100; i++) {
	if (oir_ltc4215_reg_read((void *)oir, LTC4215_STATUS_REG,
				 &data)) {
	    return(FAILED);
	}
	if (data & LTC4215_GPIO1_INPUT_MASK) {
	    break;
	}
	wastetime(10);
    }
					    
    if (i == 100) {
	cterr('f', 0, "Timed out, power is not on");
	return (FAILED);
    }
    if (oir_ltc4215_reg_read((void *)oir, LTC4215_CONTROL_REG,
			     &data)) {
        return(FAILED);
    }

    /* Set GPIO1 to output */
    data &= ~LTC4215_GPIO1_GENRAL_PURPOSE_INPUT;
    data |= LTC4215_GPIO1_GENRAL_PURPOSE_OUTPUT;

    if (oir_ltc4215_reg_write((void *)oir, LTC4215_CONTROL_REG,
			      &data)) {
        return(FAILED);
    }

    if (io_port_8bit_i2c_read(pca1, CONFIGURATION_REG,
			      &io_port_conf, TRUE)) {
	return (FAILED);
    }
    /* Set IO 1, 2, 4, 7 to output.  Set IO 3 to input */
    io_port_conf |= BIT3;
    io_port_conf &= ~(BIT1 | BIT2 | BIT4 | BIT7);
#ifdef DEBUG	
    printf("\n%d, io_port_conf = 0x%02x\n", __LINE__, io_port_conf);
#endif	
    if (io_port_8bit_i2c_write(pca1, CONFIGURATION_REG,
			       &io_port_conf)) {
	return (FAILED);
    }
    if (io_port_8bit_i2c_read(pca1, CONFIGURATION_REG,
			      &io_port_conf, TRUE)) {
	return (FAILED);
    }
#ifdef DEBUG	
    printf("\n%d, io_port_conf = 0x%02x\n", __LINE__, io_port_conf);
#endif	
    /* Set IO 2 to 0x1, this will release the Patriot CPU from reset,  but it's
     DON'T CARE now */
    /* Clear IO 1 to 0x0, this will tell the uboot it's diagnostic */
    /* Clear IO 7 to 0x0, this will tell the uboot it's NGIO */
    /* Clear IO 4 to 0 for O2 for diagnostic */
    data = 0;

    if (io_port_8bit_i2c_write(pca1, OUTPUT_PORT_REG,
			       &data)) {
	return (FAILED);
    }

#ifdef DEBUG
    if (io_port_8bit_i2c_read(pca1, INPUT_PORT_REG,
			      &data, TRUE)) {
	return (FAILED);
    }
    printf("\nINPUT_PORT_REG = 0x%02x", data);
    if (io_port_8bit_i2c_read(pca1, OUTPUT_PORT_REG,
			      &data, TRUE)) {
	return (FAILED);
    }
    printf("\nOUTPUT_PORT_REG = 0x%02x", data);
    if (io_port_8bit_i2c_read(pca1, POLARITY_INVERSION_REG,
			      &data, TRUE)) {
	return (FAILED);
    }
    printf("\nPOLARITY_INVERSION_REG = 0x%02x", data);
    if (io_port_8bit_i2c_read(pca1, CONFIGURATION_REG,
			      &data, TRUE)) {
	return (FAILED);
    }
    printf("\nCONFIGURATION_REG = 0x%02x\n", data);	
#endif	
    /*
     * Release Patriot SM out of reset.
     */
    /* uart/i2c unreset should be done via function pointer passed into the
       entry point */
    assert(ngio);
    
    ngio->uart_on(ngio);
    ngio->unreset(ngio);

    msleep(1000);
    
    return (PASSED);

}


/**********************************************************************
 *
 * Function: patriot_sm_test
 *
 * This function is the main entrance for Patriot NM testing.
 * We want to download firmware to the FPGA only once after the diag
 * image boot up. Thus, a variable fpga_dnld is defined as static for this
 * purpose. Static variable gets initialized only once. It retains its old
 * value when enters patriot_nm_test after the first time.
 *
 * Input : slot       - slot number
 *         baseaddr   - base address
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
int patriot_sm_test(void *sm)
{
    int ret_val, real_slot, slot;
    int i;
    n2g_i2c_if_t *pca1;        
    struct ngio_intf_t *patriot_sm_iface = (struct ngio_intf_t *)sm;
    ushort board_id = 0;
    uchar data;

    maindiagp = &patriotmaindiag;
    assert(patriot_sm_iface);

    pca_init_i2c((void *)&pca_i2c[0]);
    pca_i2c[0].i2c_ctrl = patriot_sm_iface->i2c_ctrl;

    pca_i2c[0].i2c_dev = SM_I2C_ADDR_IO_PORT;
    pca_i2c[0].buf   = pca_buff0;

    pca_init_i2c((void *)&pca_i2c[1]);

    pca_i2c[1].i2c_ctrl = patriot_sm_iface->i2c_ctrl;
    pca_i2c[1].i2c_dev = SM_I2C_ADDR_IO_PORT1;
    pca_i2c[1].buf   = pca_buff1;
    pca1 = &pca_i2c[1];
    
    oir = (n2g_i2c_if_t *)patriot_sm_iface->oir;
    
    board_id = patriot_sm_iface->id;
    real_slot = slot = patriot_sm_iface->slot;
#ifdef DEBUG
    printf("\nspatriot_nm_test, board_id %#x, slot %d, real slot %d\n",
	   board_id, slot, real_slot);
#endif
    /*
     * Initialize an instance of Sidewinder data structure
     */
    patriot_iface_p = (patriot_ds_t *) &patriot_iface[real_slot];
    patriot_iface_p->board_id            = board_id;
    patriot_iface_p->slot                = real_slot;
    patriot_iface_p->fw_downloaded[real_slot] = FALSE;
    patriot_iface_p->fpga_downloaded[real_slot] = FALSE;
    patriot_iface_p->uart                = patriot_sm_iface->uart_ctrl;
    patriot_iface_p->patriot_sm_iface = (struct ngio_intf_t *)sm;

#ifdef DEBUG    
    printf("\nIN main patriot_iface_p = 0x%08x", patriot_iface_p);
#endif    
    patriot_iface_p->patriot_ds_addr = (ulong)&patriot_iface_p;

    switch (board_id) {
    case SM_1T3E3:
        sprintf(patriot_iface_p->b_name, "Slot%d SM-X-1T3/E3", real_slot);
        break;
    default:
        assert(!" invalid cookie id for patraio");
        return(FAILED);
    }

    testname(patriot_iface_p->b_name);
    if (patriot_iface_p->slot == FIRST_SLOT) {
	if(get_mac_addr_from_cookie((uchar *)&patriot_iface_p->patriot_sm_iface->cookie[0],
				    &patriot_sm1_mac)) {
	    return (FAILED);
	}
#ifdef DEBUG	
	printf("\nPatriot MAC address SM %d : %02x:%02x:%02x:%02x:%02x:%02x\n",
	       patriot_iface_p->slot, 
	       (uchar)patriot_sm1_mac[0],
	       (uchar)patriot_sm1_mac[1],
	       (uchar)patriot_sm1_mac[2],
	       (uchar)patriot_sm1_mac[3],
	       (uchar)patriot_sm1_mac[4],
	       (uchar)patriot_sm1_mac[5]);
#endif	
	if (get_host_mac_addr(0, (uchar *)&host_sm1_mac[0])) {
	    return (FAILED);
	}
#ifdef DEBUG	
	printf("\nHost MAC address SM %d : %02x:%02x:%02x:%02x:%02x:%02x\n",
	       patriot_iface_p->slot,
	       (uchar)host_sm1_mac[0],
	       (uchar)host_sm1_mac[1],
	       (uchar)host_sm1_mac[2],
	       (uchar)host_sm1_mac[3],
	       (uchar)host_sm1_mac[4],
	       (uchar)host_sm1_mac[5]);
#endif	
    } else {
	if(get_mac_addr_from_cookie(&patriot_iface_p->patriot_sm_iface->cookie[0],
				    &patriot_sm2_mac)) {
	    return (FAILED);
	}
#ifdef DEBUG	
	printf("\nPatriot MAC address SM %d : %02x:%02x:%02x:%02x:%02x:%02x\n",
	       patriot_iface_p->slot, 
	       (uchar)patriot_sm2_mac[0],
	       (uchar)patriot_sm2_mac[1],
	       (uchar)patriot_sm2_mac[2],
	       (uchar)patriot_sm2_mac[3],
	       (uchar)patriot_sm2_mac[4],
	       (uchar)patriot_sm2_mac[5]);
#endif	
	if (get_host_mac_addr(1, (uchar *)&host_sm2_mac[0])) {
	    return (FAILED);
	}
#ifdef DEBUG	
	printf("\nHost MAC address SM %d : %02x:%02x:%02x:%02x:%02x:%02x\n",
	       patriot_iface_p->slot,
	       (uchar)host_sm2_mac[0],
	       (uchar)host_sm2_mac[1],
	       (uchar)host_sm2_mac[2],
	       (uchar)host_sm2_mac[3],
	       (uchar)host_sm2_mac[4],
	       (uchar)host_sm2_mac[5]);
#endif	
    }

    patriot_sm_iface->uart_on(patriot_sm_iface);

    if (tftp_get(0,
		 "sm_1t3e3_fw.img",
		 0, "/firmware/sm_1t3e3_fw.img", 1) < 0) {

	cterr('f', 0, "Failed to tftp download firmware to local host");
	return (FAILED);
    }

    if (configure_ltc4215_and_io_port(patriot_iface_p)) {
        cterr('f', 0, "unable to configure ltc");
	return (FAILED);
    }

    printf("\nPlease wait for the kernel to boot up ");
    for (i = 0; i < BOOTUP_TIME; i++) {
	printf(" .");fflush(stdout);
	msleep(1000);
	
    }

    /* Check if the interface is up */
    if (io_port_8bit_i2c_read(pca1, INPUT_PORT_REG,
			      &data, TRUE)) {
	return (FAILED);
    }

    if ((data & BIT3) == 0) {
	cterr('f', 0, "Primary Interface Not Ready");
	return (FAILED);
    }
    
    if (patriot_sm_iface->menu_display) {
	if (diagflag_xram & D_DEBUG_OPTIONS) {
            patriot_sm_iface->reset(sm);
	    msleep(1000);
            patriot_sm_iface->unreset(sm);
	    msleep(1000);
	}
        menu(&patriotmaindiag, patriot_subdiag, '\0');
        patriot_sm_cleanup(patriot_iface_p);
        return (PASSED);
    } else {	
        ret_val = patriot_main_test(patriot_iface_p);
    }

    patriot_sm_cleanup(patriot_iface_p);

    
    prcomplete(testpass, errcount, 0);
    return(ret_val);
}


/**********************************************************************
 *
 * Function: patriot_iface_test
 *
 * This function runs the interface test for Patriot
 *
 * Input : iface  - Patriot ds info
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
int
patriot_iface_test(patriot_ds_t *iface)
{
    /* Testing the I2C interface */
    if (ltc4215_register_test(iface)) {
	return (FAILED);
    }

    if (patriot_uart_test(iface)) {
 	return (FAILED);
    }

    /* Testing the GE interface */
    if (patriot_cpu_alive_test(iface)) {
	return (FAILED);
    }

    if (patriot_ge0_loopback_test(iface)) {
	return (FAILED);
    }

    
    return (PASSED);    

}

/**********************************************************************
 *
 * Function: patriot_sm_iface_test
 *
 * This function tests the SM interface for Patriot SM
 *
 *
 * Input : slot       - slot number
 *         baseaddr   - base address
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
int patriot_sm_iface_test(void *sm)
{
    int ret_val, real_slot, slot;
    int i;
    n2g_i2c_if_t *pca1;        
    struct ngio_intf_t *patriot_sm_iface = (struct ngio_intf_t *)sm;
    ushort board_id = 0;
    uchar data;

    maindiagp = &patriotmaindiag;
    assert(patriot_sm_iface);

    pca_init_i2c((void *)&pca_i2c[0]);
    pca_i2c[0].i2c_ctrl = patriot_sm_iface->i2c_ctrl;

    pca_i2c[0].i2c_dev = SM_I2C_ADDR_IO_PORT;
    pca_i2c[0].buf   = pca_buff0;

    pca_init_i2c((void *)&pca_i2c[1]);

    pca_i2c[1].i2c_ctrl = patriot_sm_iface->i2c_ctrl;
    pca_i2c[1].i2c_dev = SM_I2C_ADDR_IO_PORT1;
    pca_i2c[1].buf   = pca_buff1;
    pca1 = &pca_i2c[1];
    
    oir = (n2g_i2c_if_t *)patriot_sm_iface->oir;


    board_id = patriot_sm_iface->id;
    real_slot = slot = patriot_sm_iface->slot;
#ifdef DEBUG
    printf("\nspatriot_nm_test, board_id %#x, slot %d, real slot %d\n",
	   board_id, slot, real_slot);
#endif
    /*
     * Initialize an instance of Sidewinder data structure
     */
    patriot_iface_p = (patriot_ds_t *) &patriot_iface[real_slot];
    patriot_iface_p->board_id            = board_id;
    patriot_iface_p->slot                = real_slot;
    patriot_iface_p->fw_downloaded[real_slot] = FALSE;
    patriot_iface_p->fpga_downloaded[real_slot] = FALSE;
    patriot_iface_p->uart                = patriot_sm_iface->uart_ctrl;
    patriot_iface_p->patriot_sm_iface = (struct ngio_intf_t *)sm;

#ifdef DEBUG    
    printf("\nIN main patriot_iface_p = 0x%08x", patriot_iface_p);
#endif    
    patriot_iface_p->patriot_ds_addr = (ulong)&patriot_iface_p;

    switch (board_id) {
    case SM_1T3E3:
        sprintf(patriot_iface_p->b_name, "Slot%d SM-1T3/E3", real_slot);
        break;
    default:
        assert(!" invalid cookie id for patraio");
        return(FAILED);
    }

    testname(patriot_iface_p->b_name);

    if (patriot_iface_p->slot == FIRST_SLOT) {
	if(get_mac_addr_from_cookie(&patriot_iface_p->patriot_sm_iface->cookie[0],
				    &patriot_sm1_mac)) {
	    return (FAILED);
	}
#ifdef DEBUG	
	printf("\nPatriot MAC address SM %d : %02x:%02x:%02x:%02x:%02x:%02x\n",
	       patriot_iface_p->slot, 
	       (uchar)patriot_sm1_mac[0],
	       (uchar)patriot_sm1_mac[1],
	       (uchar)patriot_sm1_mac[2],
	       (uchar)patriot_sm1_mac[3],
	       (uchar)patriot_sm1_mac[4],
	       (uchar)patriot_sm1_mac[5]);
#endif	
	if (get_host_mac_addr(0, (uchar *)&host_sm1_mac[0])) {
	    return (FAILED);
	}
#ifdef DEBUG	
	printf("\nHost MAC address SM %d : %02x:%02x:%02x:%02x:%02x:%02x\n",
	       patriot_iface_p->slot,
	       (uchar)host_sm1_mac[0],
	       (uchar)host_sm1_mac[1],
	       (uchar)host_sm1_mac[2],
	       (uchar)host_sm1_mac[3],
	       (uchar)host_sm1_mac[4],
	       (uchar)host_sm1_mac[5]);
#endif	
    } else {
	if(get_mac_addr_from_cookie(&patriot_iface_p->patriot_sm_iface->cookie[0],
				    &patriot_sm2_mac)) {
	    return (FAILED);
	}
#ifdef DEBUG	
	printf("\nPatriot MAC address SM %d : %02x:%02x:%02x:%02x:%02x:%02x\n",
	       patriot_iface_p->slot, 
	       (uchar)patriot_sm2_mac[0],
	       (uchar)patriot_sm2_mac[1],
	       (uchar)patriot_sm2_mac[2],
	       (uchar)patriot_sm2_mac[3],
	       (uchar)patriot_sm2_mac[4],
	       (uchar)patriot_sm2_mac[5]);
#endif	
	if (get_host_mac_addr(1, (uchar *)&host_sm2_mac[0])) {
	    return (FAILED);
	}
#ifdef DEBUG	
	printf("\nHost MAC address SM %d : %02x:%02x:%02x:%02x:%02x:%02x\n",
	       patriot_iface_p->slot,
	       (uchar)host_sm2_mac[0],
	       (uchar)host_sm2_mac[1],
	       (uchar)host_sm2_mac[2],
	       (uchar)host_sm2_mac[3],
	       (uchar)host_sm2_mac[4],
	       (uchar)host_sm2_mac[5]);
#endif	
    }    

    patriot_sm_iface->uart_on(patriot_sm_iface);

    if (tftp_get(0,
		 "sm_1t3e3_fw.img",
		 0, "/firmware/sm_1t3e3_fw.img", 1) < 0) {

	cterr('f', 0, "Failed to tftp download firmware to local host");
	return (FAILED);
    }

    if (configure_ltc4215_and_io_port(patriot_iface_p)) {
        cterr('f', 0, "unable to configure ltc");
	return (FAILED);
    }

    printf("\nPlease wait for the kernel to boot up ");
    for (i = 0; i < BOOTUP_TIME; i++) {
	printf(" .");fflush(stdout);
	msleep(1000);
	
    }

    /* Check if the interface is up */
    if (io_port_8bit_i2c_read(pca1, INPUT_PORT_REG,
			      &data, TRUE)) {
	return (FAILED);
    }

    if ((data & BIT3) == 0) {
	cterr('f', 0, "Primary Interface Not Ready");
	return (FAILED);
    }    

    ret_val = patriot_iface_test(patriot_iface_p);
    
    patriot_sm_cleanup(patriot_iface_p);

    
    prcomplete(testpass, errcount, 0);
    return(ret_val);
}


/**********************************************************************
 *
 * Function: patriot_power_off
 *
 * Description: This function power off Patriot SM.
 *
 * Input :  None
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
static int
patriot_power_off (patriot_ds_t *iface)
{
    uint8_t data = 0;

    prpass(testpass, "Power Off the Patriot SM");

    if (util_oir_ltc4215_led(oir, OIR_LED_OFF)) {
        return (FAILED);
    }

    if (oir_ltc4215_reg_read(oir, LTC4215_CONTROL_REG, &data)) {
        return(FAILED);
    }

    /* power off sm module */
    data &= ~LTC4215_FET_ON_CONTROL;
    if (oir_ltc4215_reg_write(oir, LTC4215_CONTROL_REG, &data)) {
        return(FAILED);
    }
    msleep(200);

    /* make sure the power is turned off */
    if (oir_ltc4215_reg_read(oir, LTC4215_STATUS_REG, &data)) {
        return(FAILED);
    }
    if (data & LTC4215_FET_ON_STATUS) {
        printf("FET CANNOT be Turned Off.\n");
        return(FAILED);
    }
    if (data & LTC4215_FET_SHORT_PRESENT) {
        printf("FET Shortage Detected.\n");
        return(FAILED);
    }
    if (!(data & LTC4215_POWER_BAD_STATUS)) {
        printf("Power CANNOT be Turned Off.\n");
        return(FAILED);
    }

    return (PASSED);
}

/**********************************************************************
 *
 * Function: patriot_power_on
 *
 * Description: This function power on Apex-Zeta SM.
 *
 * Input :	None
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
static int
patriot_power_on (patriot_ds_t *iface)
{

    uint8_t  data = 0;

    prpass(testpass, "Power On the Patriot SM");
    
    /* Read the LTC215 Control Reg */

    if (util_oir_ltc4215_led(oir, OIR_LED_AMBER_ONLY)) {
        return (FAILED);
    }

    if (oir_ltc4215_reg_read(oir, LTC4215_CONTROL_REG, &data)) {
        return(FAILED);
    }

    /* power on sm module */
    data |= LTC4215_FET_ON_CONTROL;
    if (oir_ltc4215_reg_write(oir, LTC4215_CONTROL_REG, &data)) {
        return(FAILED);
    }
    msleep(200);
    data = 0;
    /* make sure the power is output good */
    if (oir_ltc4215_reg_read(oir, LTC4215_STATUS_REG, &data)) {
        return(FAILED);
    }
    if (!(data & LTC4215_FET_ON_STATUS)) {
        printf("FET CANNOT be Turned On.\n");
        return(FAILED);
    }
    if (data & LTC4215_POWER_BAD_STATUS) {
        printf("Power CANNOT be Turned On.\n");
        return(FAILED);
    }

    printf("Waiting for Patriot SM to Power-Up.\n");
    msleep(2000);

    /* turn on the green light status if PSE2 re-init successfully */
    if (util_oir_ltc4215_led(oir, OIR_LED_GREEN_ONLY)) {
        return (FAILED);
    }

    return (PASSED);
}


/**********************************************************************
 *
 * Function: patriot_power_cycle
 *
 * Description: A wrapper function for LTC4215 Power Cycle test.
 *
 * Input : None 
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
static int
patriot_power_cycle (patriot_ds_t *iface)
{
    uint8_t i;

    prpass(testpass, "Power Cycle the Patriot SM");

    if (patriot_power_off(iface)) {
        cterr('f', 0, "Failed on Power Off the Patriot SM");
        return(FAILED);
    }

    /* msleep for 10 seconds. */
    for (i = 0; i < 10; i++) {
        printf(".");
        msleep(1000);
    }
    printf("\n");

    if (patriot_power_on(iface)) {
        cterr('f', 0, "Failed on Power On the Patriot SM");
        return(FAILED);
    }

    return(PASSED);
}


/**********************************************************************
 *
 * Function: ltc4215_register_test
 *
 * Description: A wrapper function for LTC4215 register test.
 *
 * Input : iface  - Patriot ds info
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
static int
ltc4215_register_test (patriot_ds_t *iface)
{
    prpass(testpass, "LTC4215 OIR Register test");
    return(oir_ltc4215_register_test(oir));
}

/**********************************************************************
 *
 * Function: ltc4215_led_test
 *
 * Description: A wrapper function for LTC4215 register test.
 *
 * Input : iface  - Patriot ds info 
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
static int
ltc4215_led_test (patriot_ds_t *iface)
{
    prpass(testpass, "LTC4215 OIR LED test");
    return(oir_ltc4215_leds_test(oir));
}

/**********************************************************************
 *
 * Function: ltc4215_reg_write
 *
 * Description: LTC4215 Register Write utility.
 *
 * Input : iface  - Patriot ds info
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
static int
ltc4215_reg_write(patriot_ds_t *iface)
{
    return(util_oir_ltc4215_reg_write(oir));
}

/**********************************************************************
 *
 * Function: ltc4215_reg_read
 *
 * Description: LTC4215 Register Read utility.
 *
 * Input : iface  - Patriot ds info
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
static int
ltc4215_reg_read(patriot_ds_t *iface)
{
    return(util_oir_ltc4215_reg_read((void *)oir));
}


/******** History ********/
/*------------------------------------------------------------------------------
$Log: sm_patriot.c,v $
Revision 1.31  2018/05/18 09:24:48  alpeng
 Neptune merge to trunk with tag <neptune-branch-0518>

Revision 1.30  2017/07/14 02:51:38  alpeng
fixed compiler warning, due to cross-compiler version was updated.

Revision 1.29.48.1  2018/05/17 10:50:20  alpeng
 sync with trunk <trunk-051618>

Revision 1.30  2017/07/14 02:51:38  alpeng
fixed compiler warning, due to cross-compiler version was updated.

Revision 1.29  2014/02/18 09:11:12  alpeng
CSCul88171-3: remove useless files: mon_boot.h, c82576_ethmap.h, eth_frames.c, stack.h, sys_regs.h

Revision 1.28  2013/12/18 06:32:46  hroni
use toolchain in router/bin to do make with TOOLS_VER=c4.5.3-p1, TOOLS_ARCH=x86_64

Revision 1.27  2013/11/26 08:40:34  hroni
fix compiler warning

Revision 1.26  2013/05/15 20:41:56  huanngo
If the firmware is at /firmware do not download it

Revision 1.25  2013/05/09 19:25:18  mcharon
remove unused header files. fixed dependancy compile problem

Revision 1.24  2013/03/26 19:25:46  huanngo
Do not download FW again if /firmware/sm_1t3e3_fw.img is existing

Revision 1.23  2013/01/28 01:52:47  steja
<CSCue19620> Diag Improvement for I/O Expander testing.

Revision 1.22  2013/01/15 20:34:50  huanngo
Change Patriot Product ID name to SM-X-1T3/E3

Revision 1.21  2012/12/05 02:15:48  huanngo
Cosmetic changes and remove unused utility

Revision 1.20  2012/11/07 02:42:58  steja
Add Subrate individual test

Revision 1.19  2012/10/25 08:23:54  steja
Remove "Clear" from the subrate test

Revision 1.18  2012/10/25 00:13:37  huanngo
Adding the code to read the MAC address in Patriot interface test - fix CSCuc86479

Revision 1.17  2012/10/15 21:23:17  huanngo
Removing PCIE test and adding the code to read MAC address from cookie

Revision 1.16  2012/10/08 19:39:21  huanngo
Adding testing GE0 interface to I/O interface test

Revision 1.15  2012/09/19 18:34:38  huanngo
Support new utility for secure boot and interface test

Revision 1.14  2012/09/05 23:58:17  huanngo
Modify the IO pin configure for the new uboot

Revision 1.13  2012/07/31 00:24:57  huanngo
Adding the ECC memory test to the main test

Revision 1.12  2012/07/25 00:10:55  huanngo
Add the uart test back

Revision 1.11  2012/07/19 17:40:12  huanngo
Support FPGA programming to SPI PROM

Revision 1.10  2012/06/30 00:15:28  huanngo
Adding UART test to the main test

Revision 1.9  2012/06/27 06:17:03  steja
Add Power Margin Utilities

Revision 1.8  2012/06/07 21:20:17  huanngo
Adding new tests

Revision 1.7  2012/05/22 22:29:10  huanngo
Adding a paremeter to tftp_get function call

Revision 1.6  2012/05/04 20:01:46  mcharon
use void* instead of int as argument to to func ptrs in ngio_intf

Revision 1.5  2012/05/02 22:12:40  mcharon
remove tftp_get definition

Revision 1.4  2012/05/02 17:55:58  huanngo
Clean up and dowonload FPGA when necessary, not right after boot up Linux

Revision 1.3  2012/03/28 23:35:08  huanngo
Support new tests and utilities on Patriot

Revision 1.2  2012/03/28 00:38:15  mcharon
remove forward slash from second line

Revision 1.1.1.1  2012/03/23 23:02:05  ptong
Initial archive of ng_diag module


$Endlog$
*/
