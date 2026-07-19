/* $Id: diag.c,v 1.4 2019/07/11 12:31:26 alicehua Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/nutella/diag.c,v $
 *------------------------------------------------------------------
 *
 * diag.c - Nutella diag main menu and supporting wrappers.
 *
 *
 * Copyright (c) 2019 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h> 
#include <string.h> 
#include <sys/mman.h>
#include "common.h"
#include "error.h"
#include "proto.h"
#include "types.h"
#include "setjmps.h"
#include "monitor.h"
#include "nvmonvars.h"
#include "menu.h"
#include "error.h"
#include "plat_defs.h"
#include "platform_cookie.h"
#include "mb_tests.h"
#include "i2c_api.h"
#include "diag_lte_test.h"
#include "diag_lte_lib.h"
#include "diag_i2c_lib.h"
#include "diag_usb_util.h"
#include "diag_cpu_util.h"
#include "diag_storage_lib.h"
#include "tam_act2_api_drv_support.h"
#include "diag_fpga.h"
#include "diag_rtc_util.h"
#include "diag_led_test.h"
#include "diag_emmc_util.h"
#include "full_load_util.h"
#include "diag_spi_flash_util.h"
#include "diag_fpga_upgrade.h"

/*
 * Declare external function
 */
extern int ExecuteCmdbyPopen(char *cmd, char *retBuf, int sizeOfBuf);
extern int show_emmc_info (void);
extern int emmc_pslc_fully_enable(int);
extern int emmc_full_test(int);
extern int nutella_show_fpga_ver(int);
extern char *banner_string;
extern int nutella_cpu_ondie_temp (int opt);
extern int diag_temp_sensor_show_temp(void);
extern int diag_full_load_util(void);
extern boolean menu_display(void);

/*
 * Declare local function
 */

static int diag_sys_info_util(int);
static int diag_ex_feature_util(int);

static int diag_vol_margin_util(int);
static int diag_display_mb_temperature(int);
/*
 *  Globals  
 */
int netflashbooted = 0; /* menu.c need this */

/*
 * Main menu -> Basic utility -> Memory Utility
 */
static struct mitem mem_debug_items[] = {
    {"alter memory", 0, 0,
     (PFT) alt_mem, &one,
     MF_CONTINUOUS | MF_DOALL | MF_SHOW_ERRCOUNT,
     (type_t(*)())0, 0},
    {"compare memory block", 0, 0,
     (PFT) cmp_mem, &one,
     MF_CONTINUOUS | MF_DOALL | MF_SHOW_ERRCOUNT,
     (type_t(*)())0, 0},
    {"display memory", 0, 0,
     (PFT) dis_mem, &one,
     MF_CONTINUOUS | MF_DOALL | MF_SHOW_ERRCOUNT,
     (type_t(*)())0, 0},
    {"move memory block", 0, 0,
     (PFT) mov_mem, &one,
     MF_CONTINUOUS | MF_DOALL | MF_SHOW_ERRCOUNT,
     (type_t(*)())0, 0},
    {"fill memory", 0, 0,
     (PFT) fil_mem, &one,
     MF_CONTINUOUS | MF_DOALL | MF_SHOW_ERRCOUNT,
     (type_t(*)())0, 0},
    {"find memory", 0, 0,
     (PFT) memtest, &one,
     MF_CONTINUOUS | MF_DOALL | MF_SHOW_ERRCOUNT,
     (type_t(*)())0, 0},
    {"memory read or write loop", 0, 0,
     (PFT) memloop, &one,
     MF_CONTINUOUS | MF_DOALL | MF_SHOW_ERRCOUNT,
     (type_t(*)())0, 0},
    {"memory debug loop", 0, 0,
     (PFT) memdebug, &one,
     MF_CONTINUOUS | MF_DOALL | MF_SHOW_ERRCOUNT,
     (type_t(*)())0, 0},
    {"address loop", 0, 0,
     (PFT) addrloop, &one,
     MF_CONTINUOUS | MF_DOALL | MF_SHOW_ERRCOUNT,
     (type_t(*)())0, 0},
};

static struct menuinfo mem_debug_menu = {
    "Memory Utility Menu",
    0,
    0,
    0,
    sizeof(mem_debug_items) / sizeof(struct mitem),
    mem_debug_items,
};

static struct menuinfo *mem_debug_menup = &mem_debug_menu;

/* 
 * Cookie menu utility
 */

static struct mitem cookie_items[] = {
    {"alter MB CPU cookie", 0, 0,
     (type_t(*)()) alter_mb_cookie, &one, 0, (type_t(*)())0, 0},
    
    {"alter DMI EEPROM", 0, 0,
     (type_t(*)()) alter_dmi_eeprom, &one, 0, (type_t(*)())0, 0},
};

static struct menuinfo cookie_menu = {
    "Cookie utility Menu",
    0,
    0,
    0,
    sizeof(cookie_items) / sizeof(struct mitem),
    cookie_items,
};

static struct menuinfo *cookie_menup = &cookie_menu;

/*
 * Main menu -> Basic utilities
 */

static struct mitem utilmenuitems[] = {
    {"System Information", 0, 0,
     (PFT) diag_sys_info_util, (type_t *) &one, 0,
     (type_t(*)())0, 0},

    {"Memory debug utilities", 0, 0,
     (PFT) menu, (type_t *) &mem_debug_menup, 0,
     (type_t(*)())0, 0},

    {"I2C utility", 0, 0,
     (PFT) build_i2c_util_menu, (type_t *) &one, 0,
     (type_t(*)())0, 0},

    {"Cookie utility", 0, 0,
     (PFT) menu, (type_t *) &cookie_menup, 0,
     (type_t(*)())0, 0},

    {"Extended Feature", 0, 0,
     (PFT) diag_ex_feature_util, (type_t *) &one, MF_HIDDEN_EXE,
     (type_t(*)())menu_display, 0},

    {"LED Utilities", 0, 0,
     (PFT) build_led_util_menu, (type_t *) &one, 0,
     (type_t(*)())0, 0},

    {"Voltage Margin", 0, 0,
     (PFT) diag_vol_margin_util, (type_t *) &one, 0,
     (type_t(*)())0, 0},

    {"FPGA Utilities", 0, 0,
     (PFT) nutella_fpga_utils, (type_t *) &one, 0,
     (type_t(*)())0, 0},

    {"M/B Temperature Utilites", 0, 0,
     (PFT) diag_display_mb_temperature, (type_t *) &one, 0,
     (type_t(*)())0, 0},

    {"CPU Switch mux LTE external micro USB",
     0, 0,
     (PFT) diag_lte_mux_switch_util,   (type_t *) &one, 0,
     (type_t(*)())has_lte_sku, 0},
    
    {"LTE external micro USB Switch mux CPU",
     0, 0,
     (PFT) diag_lte_mux_switch_util,   (type_t *) &zero, 0,
     (type_t(*)())has_lte_sku, 0},

    {"Full Load Utility", 0, 0,
     (PFT) diag_full_load_util, (type_t *) &one, 0,
     (type_t(*)())0, 0},

    {"Intel Denverton Interface Utility", 0, 0,
     (PFT) build_dnv_utils_menu, (type_t *) &one, 0,
     (type_t(*)())0, 0},

    {"RTC Utility", 0, 0,
     (PFT) build_rtc_utils_menu, (type_t *) &one, 0,
     (type_t(*)())0, 0},
     
    {"USB 2.0 Test Mode Utility",
     0, 0,
     (PFT)usb_test_mode, (type_t *)&zero, 0,
     (type_t(*)())0, 0},

    {"SPI Flash Utility",
    0, 0,
    (PFT)nutella_spi_flash_utils, (type_t *)&zero, 0,
    (type_t(*)())0, 0},

    {"AIKIDO SPI read", 0, 0,
     (type_t(*)())aikido_spi_read_util, &zero, 0, (type_t(*)())0, 0},

    {"AIKIDO SPI write", 0, 0,
     (type_t(*)())aikido_spi_write_util, &zero, 0, (type_t(*)())0, 0},

    {"toggle flag : aikido_mailbox_flag", 0, 0,
     (type_t(*)())aikido_flag_mailbox, &zero, 0, (type_t(*)())0, 0},

    {"toggle flag: aikido_act2_flag", 0, 0,
     (type_t(*)())aikido_flag_act2, &zero, 0, (type_t(*)())0, 0},
};

static struct menuinfo utilmenu = {
    "Diagnostic Utilities Menu",
    0,
    0,
    0,
    sizeof(utilmenuitems) / sizeof(struct mitem),
    utilmenuitems,
};

struct menuinfo *utilmenup = &utilmenu;

/*
 * Main menu
 */

submenu_xtable_t main_menu_table[] = {
    {"ACT-2 utilities and programming",
     (PFT) smartchip, FALSE,
     MF_CONTINUOUS,
     (type_t(*)())0, 0, (PFT) smartchip, TRUE},

    {"Motherboard tests",
    (PFT) mb_tests, TRUE,
    MF_CONTINUOUS | MF_DOALL | MF_SHOW_ERRCOUNT,
    (type_t(*)())0, 0, (PFT) mb_tests, FALSE},

    {"LTE Test",
    (PFT) diag_lte_test, TRUE,
    MF_CONTINUOUS | MF_DOALL | MF_SHOW_ERRCOUNT,
    (type_t(*)())has_lte_sku, 0, (PFT) diag_lte_test , FALSE},
};

#define MAIN_MENU_TABLE_SIZE \
        (sizeof(main_menu_table) / sizeof(submenu_xtable_t))

/*
 * Main menu primary & secondary submenu items
 */
static mitem_t main_menu_primary_items[MAIN_MENU_TABLE_SIZE +
                                       MAX_BASE_ITEMS];
static mitem_t main_menu_secondary_items[MAIN_MENU_TABLE_SIZE +
                                       MAX_BASE_ITEMS];

static struct menuinfo maindiag = {
    "Main %s",                  /* title */
    0,                          /* title string added by init_empty_menu */
    (PFT) menu_show_dflags,     /* shows major flags */
    0,                          /* generic prompt */
    0,                          /* size -- bumped by add_menu_item() */
    main_menu_primary_items,
};

static struct menuinfo *maindiagp = &maindiag;

void diag_menu (int argc, char *argv[])
{
    char arg = 0;

    build_primary_submenu(main_menu_table, MAIN_MENU_TABLE_SIZE, dgmenustr,
                          &maindiagp);
    build_secondary_submenu(main_menu_table, MAIN_MENU_TABLE_SIZE,
                            main_menu_secondary_items);
    menu(&maindiag, main_menu_secondary_items, arg);
}


/**********************************************************************
 *
 * Function: diag_sys_info_util
 *
 * Description: display system info, ex. current time, Diag ver
 *
 * Input : dummy
 *                     
 * Output: None
 *
 **********************************************************************
 */

int diag_sys_info_util (int dummy)
{
    nutella_show_cpuinfo();
    nutella_show_meminfo();
    printf("Thermal Sensor(NXP LM75BD) ");
    diag_temp_sensor_show_temp();
    nutella_show_fpga_ver(0);

    system("/printver_gen.sh");
    printf("%s", banner_string);
    return (PASSED);
}

/**********************************************************************
 *
 * Function: diag_ex_feature_util
 *
 * Description: display system info, ex. current time, Diag ver
 *
 * Input : dummy
 *                     
 * Output: None
 *
 **********************************************************************
 */

int diag_ex_feature_util (int dummy)
{
    return (PASSED);
}


/**********************************************************************
 *
 * Function: diag_vol_margin_util
 *
 * Description: Utility to modify nutella voltage margin
 *
 * Input : dummy
 *                     
 * Output: None
 *
 **********************************************************************
 */

int diag_vol_margin_util (int dummy)
{
    uint8_t v_status = 0;


    v_status = (uint8_t)gethex_answer("Enter Voltage Margin mode"
                                      "(0-Normal, 1-Low, 2-High): ",
                                      0, 0, 2);

    fpga_vol_margin(v_status);


    return (PASSED);
}


/**********************************************************************
 *
 * Function: diag_display_mb_temperature
 *
 * Description: display mother board temperature info
 *
 * Input : dummy
 *                     
 * Output: None
 *
 **********************************************************************
 */

int diag_display_mb_temperature (int dummy)
{
    nutella_cpu_ondie_temp(0);
    printf("Thermal Sensor(NXP LM75BD) ");
    diag_temp_sensor_show_temp();
	
    return (PASSED);
}

/*-------------------------------------------------
$Log: diag.c,v $
Revision 1.4  2019/07/11 12:31:26  alicehua
Collapse Nutella codes into main trunk.

$Endlog$
*/
