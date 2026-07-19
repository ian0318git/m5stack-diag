 /* $Id: diag.c,v 1.6 2021/05/13 08:49:58 kodko Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/tabei-l/diag.c,v $
 *------------------------------------------------------------------
 *
 * diag.c - Tabei diagmon main menu and supporting wrappers.
 *
 *
 * Copyright (c) 2018-2021 by Cisco Systems, Inc.
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
#include "diag_i2c_lib.h"
#include "diag_usb_util.h"
#include "diag_cpu_util.h"
#include "tam_act2_api_drv_support.h"
#include "diag_fpga.h"
#include "diag_rtc_util.h"
#include "diag_led_test.h"
#include "diag_spi_flash_util.h"
#include "diag_emmc_util.h"
#include "slot.h"
#include "ngio.h"
#include "diag_temp_snsr_test.h"
#include "plug_slot.h"
#include "plug_host_fpga_lib.h"
#include "diag_fpga_upgrade.h"
#include "diag_hdd_test.h"
#include "diag_m2_test.h"
#include "diag_rtc_test.h"
#include "fpga_smartfan.h"
#include "m2_testcard.h"
#include "m2_testcard_host_impl.h"

/*
 * Declare external function
 */
extern int ExecuteCmdbyPopen(char *cmd, char *retBuf, int sizeOfBuf);
extern int show_emmc_info(void);
extern int emmc_pslc_fully_enable(int);
extern int emmc_full_test(int);
extern int tabei_show_fpga_ver(int);
extern char *banner_string;
extern int tabei_cpu_ondie_temp (int opt);
extern int diag_temp_sensor_show_temp(void);
extern int diag_full_load_util(void);
extern void build_fan_menu(void);
extern boolean menu_display(void);
extern int fan_speed_test(void);

/*
 * Declare local function
 */

static void diag_sys_info_util(int);
static int diag_ex_feature_util(int);

static int diag_vol_margin_util(int);
static int diag_mb_temper_util(int);
static int io_interface_tests(void);
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
    
    {"alter PIM cookie", 0, 0,
     (type_t(*)()) alter_pim_cookie, &one, 0, (type_t(*)())has_pim, 0},

    {"alter BIOS EEPROM", 0, 0,
     (type_t(*)()) alter_bios_eeprom, &one, 0, (type_t(*)())has_bios_eeprom, 0},
    
    {"alter NIM cookie", 0, 0,
     (type_t(*)()) alter_nim_cookie, &one, 0, (type_t(*)())has_nim, 0},

    {"alter NGIOWIC DAUGHTER cookie",             0,    0,
     (type_t(*)())alter_nim_dc_cookie,    &one, 0, (type_t(*)())0, 0},

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

    {"Memory Debug Utilities", 0, 0,
     (PFT) menu, (type_t *) &mem_debug_menup, 0,
     (type_t(*)())0, 0},

    {"I2C Utility", 0, 0,
     (PFT) build_i2c_util_menu, (type_t *) &one, 0,
     (type_t(*)())0, 0},

    {"Cookie Utility", 0, 0,
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
     (PFT) tabei_fpga_utils, (type_t *) &one, 0,
     (type_t(*)())0, 0},

    {"M/B Temperature Utilites", 0, 0,
     (PFT) diag_mb_temper_util, (type_t *) &one, 0,
     (type_t(*)())0, 0},

    {"Full Load Utility", 0, 0,
     (PFT) diag_full_load_util, (type_t *) &one, 0,
     (type_t(*)())0, 0},

    {"Intel Denverton Interface Utility", 0, 0,
     (PFT) build_dnv_utils_menu, (type_t *) &one, 0,
     (type_t(*)())0, 0},

    {"Enable eMMC pSLC Mode", 0, 0,
     (PFT) emmc_pslc_fully_enable, (type_t *) &zero, 0,
     (type_t(*)())has_emmc, 0},

    {"Show eMMC Info", 0, 0,
     (PFT) show_emmc_info, (type_t *) &zero, 0,
     (type_t(*)())has_emmc, 0},

    {"eMMC full test", 0, 0,
     (PFT) emmc_full_test, (type_t *) &zero, 0,
     (type_t(*)())has_emmc, 0},

    {"RTC Utility", 0, 0,
     (PFT) build_rtc_utils_menu, (type_t *) &one, 0,
     (type_t(*)())0, 0},
     
    {"USB 2.0 Test Mode Utility",
     0, 0,
     (PFT)usb_test_mode, (type_t *)&zero, 0,
     (type_t(*)())0, 0},

    {"SPI Flash Utility",
    0, 0,
    (PFT)tabei_spi_flash_utils, (type_t *)&zero, 0,
    (type_t(*)())0, 0},

    {"FAN Utility",
    0,                 0,
    (PFT)build_fan_menu, (type_t *)&zero,                0,
    (type_t(*)())0,            0},

    {"Check M2 Device Utility",
    0,                 0,
    (PFT)check_m2_device_utility, (type_t *)&zero,                0,
    (type_t(*)())0,            0},

    {"AIKIDO SPI read", 0, 0,
     (type_t(*)())aikido_spi_read_util, &zero, 0, (type_t(*)())0, 0},

    {"AIKIDO SPI write", 0, 0,
     (type_t(*)())aikido_spi_write_util, &zero, 0, (type_t(*)())0, 0},

    {"toggle flag : aikido_mailbox_flag", 0, 0,
     (type_t(*)())aikido_flag_mailbox, &zero, 0, (type_t(*)())0, 0},

    {"toggle flag: aikido_act2_flag", 0, 0,
     (type_t(*)())aikido_flag_act2, &zero, 0, (type_t(*)())0, 0},

    {"POE -54V Detect Utility", 0, 0,
     (type_t(*)())fpga_poe_detect_util, &zero, 0, (type_t(*)())0, 0},

    {"Display smartfan info", 0, 0,
     (type_t(*)())display_smartfan_info_workaround, &zero, 0, (type_t(*)())0, 0},

    {"Check Fan speed", 0, 0,
     (type_t(*)())fan_speed_test, &zero, 0, (type_t(*)())0, 0},

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

    {"I/O Interface tests",
    (PFT) io_interface_tests, FALSE,
    MF_CONTINUOUS | MF_DOALL | MF_SHOW_ERRCOUNT,
    (type_t(*)())0, 0, (PFT)0, FALSE},

    {"Motherboard tests",
    (PFT) mb_tests, TRUE,
    MF_CONTINUOUS | MF_DOALL | MF_SHOW_ERRCOUNT,
    (type_t(*)())0, 0, (PFT) mb_tests, FALSE},

    {"test NIM Slot",
    (PFT)wic_test,             FIRST_SLOT,
    MF_CONTINUOUS | MF_DOALL | MF_SHOW_ERRCOUNT,
    (type_t(*)())has_nim, 0,         (PFT)wic_test,  FIRST_SLOT + MAX_WIC},

    {"test PIM Slot",
     (PFT) plug_test, PLUG_SLOT_1 ,
     MF_CONTINUOUS | MF_DOALL | MF_SHOW_ERRCOUNT,
     (type_t(*)())has_pim, PLUG_SLOT_1,
     (PFT) plug_test, PLUG_SLOT_1 + MAX_PLUG_SLOT_NUMBER},

    /* M.2 Test Card: if present, show this test; not present, not show*/
    {"M.2 testcard test",
     (PFT) m2_testcard_test, TRUE,
     MF_CONTINUOUS | MF_DOALL | MF_SHOW_ERRCOUNT,
     (type_t(*)())is_m2_testcard_in, 0,
     (PFT) m2_testcard_test, FALSE},
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

void diag_sys_info_util (int dummy)
{
    tabei_show_cpuinfo();
    tabei_show_meminfo();
    printf("Thermal Sensor\n");
    show_temperature_all();
    tabei_show_fpga_ver(0);

    system("/printver_gen.sh");
    printf("%s", banner_string);
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
 * Description: Utility to modify voltage margin
 *
 * Input : dummy
 *                     
 * Output: PASSED
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
    tabei_cpu_ondie_temp(0);
    printf("Thermal Sensor\n ");
    show_temperature_all();	
    return (PASSED);
}

/*-------------------------------------------------------------------
 *
 * Function: io_tests()
 *
 * Description : io interface tests.
 *
 * Inputs: N/A
 *
 * Output: PASSED/FAILED
 *
 *-------------------------------------------------------------------
 */
static int io_interface_tests (void)
{
    int pim_rc = FAILED, nim_rc = FAILED;
    char *tname = "I/O interface";
    
    testname(tname);
    prpass(testpass, "%s, ", tname);

    prpass(testpass, " NIM I/O interface");
    if (wic_iface_test() == PASSED) {
        nim_rc = PASSED;
    }

    prpass(testpass, " PIM I/O interface");
    if (plug_intf_test((PLUG_SLOT_1)) == PASSED) {
        pim_rc = PASSED;
        prpass(testpass, "%s PLUG %d detected.", tname, PLUG_SLOT_1);
    }

    prcomplete(testpass, errcount, (char *)0);
    
    return (pim_rc | nim_rc); 
}

/*-------------------------------------------------
 * $Log: diag.c,v $
 * Revision 1.6  2021/05/13 08:49:58  kodko
 * Support M.2 testcard.
 *
 * Revision 1.5  2020/08/17 07:22:40  kehuang2
 * CSCvv34796: Support fan speed test
 *
 * Revision 1.4  2020/08/06 07:54:55  kehuang2
 * Collapse Promethium into main trunk
 *
 * Revision 1.3  2019/12/30 06:03:45  kehuang2
 * CSCvs55860: Support Alter Quack cookie
 *
 * Revision 1.2  2019/10/17 02:16:19  kehuang2
 * Collapse Tabei-L into main trunk
 *
 * Revision 1.1.4.31  2019/10/01 03:00:49  kehuang2
 * Clean up code
 *
 * Revision 1.1.4.30  2019/08/20 10:30:12  kehuang2
 * Support POE detect Utility
 *
 * Revision 1.1.4.29  2019/08/06 07:20:27  kehuang2
 * Update present function base on the comment of code review
 *
 * Revision 1.1.4.28  2019/07/16 07:48:31  olin2
 * Remove unused function
 *
 * Revision 1.1.4.27  2019/07/15 09:40:02  kehuang2
 * Update Voltage Margin utility
 *
 * Revision 1.1.4.26  2019/07/04 03:23:35  kehuang2
 * Combine Tabei-L sereies image together(Fortnite, Tabei-L, Promethium)
 *
 * Revision 1.1.4.25  2019/06/20 06:21:13  kehuang2
 *
 * 1. Support linux_block_test function
 * 2. Update Diag menu item base on currently project information
 *
 * Revision 1.1.4.24  2019/05/29 03:16:17  kehuang2
 *
 * 1.Merge image according to official board type.
 * 2.Reform the structure of diag menu
 *
 * Revision 1.1.4.23  2019/04/30 05:57:32  kodko
 * Remove needless enumerate USB utility
 *
 * Revision 1.1.4.22  2019/04/10 03:34:17  kodko
 * Support altering NIM module cookie content.
 *
 * Revision 1.1.4.21  2019/01/25 07:42:24  harrchan
 * Add SKU1 in Makefile for seperature sku in future
 *
 * Revision 1.1.4.20  2019/01/19 03:49:03  kodko
 * Support alter PIM cookie.
 *
 * Revision 1.1.4.19  2019/01/18 02:30:15  olin2
 * Clean up code
 *
 * Revision 1.1.4.18  2018/12/21 07:09:47  olin2
 * Update M.2 device menu
 *
 * Revision 1.1.4.17  2018/12/13 02:49:43  olin2
 * Support Emmc enable pSLC util
 *
 * Revision 1.1.4.16  2018/12/07 01:41:22  olin2
 * Clean up menu
 *
 * Revision 1.1.4.15  2018/12/07 01:33:59  olin2
 * Support Check M.2 device util
 *
 * Revision 1.1.4.14  2018/12/05 06:50:34  olin2
 * initial commit for Aikido
 *
 * Revision 1.1.4.13  2018/11/16 05:42:09  olin2
 * Clean up code
 *
 * Revision 1.1.4.12  2018/11/15 06:56:06  olin2
 * initial commit for Fan utils
 *
 * Revision 1.1.4.11  2018/11/05 12:17:31  kodko
 * Support 64Kbits BIOS EEPROM program.
 *
 * Revision 1.1.4.10  2018/11/02 07:42:40  harrchan
 * EEPROM init utility
 *
 * Revision 1.1.4.9  2018/11/01 01:07:40  harrchan
 * EEPROM read/write utility
 *
 * Revision 1.1.4.8  2018/10/29 01:29:55  olin2
 * Remove unused function
 *
 * Revision 1.1.4.7  2018/10/26 08:40:50  kodko
 * Add support for PIM LTE and test card modules.
 *
 * Revision 1.1.4.6  2018/10/25 02:37:57  harrchan
 * eMMC Test
 *
 * Revision 1.1.4.5  2018/10/22 11:29:32  harrchan
 * Temperature sensor
 *
 * Revision 1.1.4.4  2018/10/15 11:48:28  olin2
 * Update for using common slot.c
 *
 * Revision 1.1.4.3  2018/10/09 09:22:04  olin2
 * Initial commit for NIM test
 *
 * Revision 1.1.4.2  2018/10/02 01:49:57  harrchan
 * Initial commit for Tabei-L P1A bring up.
 *
 * $Endlog$
 *-------------------------------------------------
 */
