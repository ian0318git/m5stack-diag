 /* $Id: diag.c,v 1.3 2018/08/31 03:59:30 chieyang Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/viper/diag.c,v $
 *------------------------------------------------------------------
 *
 * diag.c - Viper diagmon main menu and supporting wrappers.
 *
 *
 * Copyright (c) 2018 by Cisco Systems, Inc.
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
#include "diag_xdsl_test.h"
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

/*
 * Declare external function
 */
extern int ExecuteCmdbyPopen(char *cmd, char *retBuf, int sizeOfBuf);
extern int show_emmc_info (void);
extern int emmc_pslc_fully_enable(int);
extern int emmc_full_test(int);
extern int viper_show_fpga_ver(int);
extern char *banner_string;
extern int viper_cpu_ondie_temp (int opt);
extern int diag_temp_sensor_show_temp(void);
extern int diag_full_load_util(void);
extern boolean menu_display(void);

/*
 * Declare local function
 */

static int diag_sys_info_util(int);
static int diag_ex_feature_util(int);

static int diag_vol_margin_util(int);
static int diag_mb_temper_util(int);
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

    {"Enumerate all USB", 0, 0,
     (PFT) usb_utils, (type_t *) &one, 0,
     (type_t(*)())0, 0},

    {"Extended Feature", 0, 0,
     (PFT) diag_ex_feature_util, (type_t *) &one, MF_HIDDEN_EXE,
     (type_t(*)())menu_display, 0},

    {"LED Utilities", 0, 0,
     (PFT) build_led_util_menu, (type_t *) &one, 0,
     (type_t(*)())0, 0},

    {"Voltage Margin (Foxconn FPGA only)", 0, 0,
     (PFT) diag_vol_margin_util, (type_t *) &one, 0,
     (type_t(*)())this_is_viper_foxconn, 0},

    {"FPGA Utilities", 0, 0,
     (PFT) viper_fpga_utils, (type_t *) &one, 0,
     (type_t(*)())0, 0},

    {"M/B Temperature Utilites", 0, 0,
     (PFT) diag_mb_temper_util, (type_t *) &one, 0,
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

    {"Enable eMMC pSLC mode with create 8MB GPP",
     0, 0,
     (PFT) emmc_pslc_fully_enable,   (type_t *) &zero, 0,
     (type_t(*)())0, 0},
    
    {"Show eMMC info",
     0, 0,
     (PFT) show_emmc_info,           (type_t *) &zero, 0,
     (type_t(*)())0, 0},
    
    {"eMMC full test",
     0, 0,
     (PFT) emmc_full_test,           (type_t *) &zero, 0,
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
    (PFT)viper_spi_flash_utils, (type_t *)&zero, 0,
    (type_t(*)())0, 0},
    
     
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

    {"xDSL Test",
    (PFT) diag_xdsl_tests, TRUE,
    MF_CONTINUOUS | MF_DOALL | MF_SHOW_ERRCOUNT,
    (type_t(*)())has_dsl_sku, 0, (PFT) diag_xdsl_tests, FALSE},

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
    viper_show_cpuinfo();
    viper_show_meminfo();
    printf("Thermal Sensor(NXP LM75BD) ");
    diag_temp_sensor_show_temp();
    viper_show_fpga_ver(0);

    system("/printver_gen.sh");
    printf("%s", banner_string);
    return (PASSED);
}

/**********************************************************************
 *
 * Function: display_viper_sku_info
 *
 * Description: display Viper SKU info
 *
 * Input : dummy
 *                     
 * Output: None
 *
 **********************************************************************
 */

int display_viper_sku_info (void)
{   
    uchar mb_get_pid[64] = {0};
    ushort controller_type = 0x0;


    /* Get controller type */
    controller_type = get_mb_id();

    /* Get PID */
    platform_get_pid((char *)mb_get_pid);

    printf("\n");
    printf("PID             : %s\n", mb_get_pid);
    printf("Controller Type : [0x%x]\n", controller_type);


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
 * Description: Utility to modify viper voltage margin
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
 * Function: diag_mb_temper_util
 *
 * Description: display mother board temperature info
 *
 * Input : dummy
 *                     
 * Output: None
 *
 **********************************************************************
 */

int diag_mb_temper_util (int dummy)
{
    viper_cpu_ondie_temp(0);
    printf("Thermal Sensor(NXP LM75BD) ");
    diag_temp_sensor_show_temp();
	
    return (PASSED);
}

/*-------------------------------------------------
 * $Log: diag.c,v $
 * Revision 1.3  2018/08/31 03:59:30  chieyang
 * Add SPI flash utility, show memory size and xdsl test modification. Merge from viper-branch2
 *
 * Revision 1.2  2018/08/06 02:31:49  harrchan
 * Merge viper E2E to the main trunk (CSCvk28469)
 *
 * Revision 1.1.2.21  2018/07/03 05:38:55  harrchan
 * Follow the coding rule to clean up code
 *
 * Revision 1.1.2.20  2018/06/27 07:38:51  lucywang
 * Hid "Extended Feature"
 *
 * Revision 1.1.2.19  2018/06/27 07:27:28  lucywang
 * Removed string ViperJ
 *
 * Revision 1.1.2.18  2018/06/27 06:27:48  harrchan
 * Follow the coding rule to clean up code
 *
 * Revision 1.1.2.17  2018/06/25 10:07:29  lucywang
 * Modified messages for full load utility
 *
 * Revision 1.1.2.16  2018/06/14 01:28:46  harrchan
 * Remove Aikido option and Aikido keyword in whole source code
 *
 * Revision 1.1.2.15  2018/06/13 01:02:06  olin2
 * Updated basic utility menu
 *
 * Revision 1.1.2.14  2018/06/06 11:38:06  lucywang
 * Modified the process to enable pSLC and create 8MB GPP on eMMC based on Cisco SW requirement
 *
 * Revision 1.1.2.13  2018/05/11 08:46:01  lucywang
 * Added full loading utility
 *
 * Revision 1.1.2.12  2018/05/10 09:35:32  lucywang
 * Added LTE Mux Switch Utility
 *
 * Revision 1.1.2.11  2018/05/10 09:04:37  lucywang
 * Added USB 2.0 test mode utility
 *
 * Revision 1.1.2.10  2018/05/10 05:51:21  olin2
 * Support voltage margin util for Viper-Intel
 *
 * Revision 1.1.2.9  2018/05/09 07:11:25  olin2
 * 1. Move GE and DSL init to the beginning. 2. Add has GE1. 3. Show cookie info
 *
 * Revision 1.1.2.8  2018/05/03 08:48:40  lucywang
 * Added Temperature to System Information
 *
 * Revision 1.1.2.7  2018/04/20 03:05:49  lucywang
 * Based on FPGA Board Type Register to show LTE/DLS test item
 *
 * Revision 1.1.2.6  2018/04/17 11:16:25  lucywang
 * Fixed LED issue for ViperJ
 *
 * Revision 1.1.2.5  2018/04/09 02:34:50  lucywang
 * Added System Intermation
 *
 * Revision 1.1.2.4  2018/03/29 01:11:19  lucywang
 * Added RTC test and utility
 *
 * Revision 1.1.2.3  2018/03/27 09:16:54  harrchan
 * Move led test to mother board test menu
 *
 * Revision 1.1.2.2  2018/03/27 03:01:40  lucywang
 * Added eMMC utilities for full test and pSLC mode
 *
 * Revision 1.1.2.1  2018/02/27 08:06:31  harrchan
 * Initial viper application code base
 *
 * $Endlog$
 *-------------------------------------------------
 */
