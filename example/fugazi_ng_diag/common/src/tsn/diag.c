/* $Id: diag.c,v 1.10 2019/01/18 05:54:46 yungchen Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/tsn/diag.c,v $
 *------------------------------------------------------------------
 *
 * diag.c - tsn diagmon main menu and supporting wrappers.
 *
 * Feb 2018, Sofian teja adapted from Xformers.
 *
 * Copyright (c) 2016 ~ 2019 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include "common.h"
#include "error.h"
#include "proto.h"
#include "types.h"
#include "setjmps.h"
#include "pcmap.h"
#include "monitor.h"
#include "nvmonvars.h"
#include "menu.h"
#include "error.h"
#include "plat_defs.h"
#include "platform_fpga.h"
#include "platform_stub.h"
#include "dsl_tests.h"
#include "i2c_address.h"
#include "platform_cpu.h"
#include "platform_i2c.h"
#include "lte.h"
#include "mb_tests.h"
#include "plat_defs.h"
#include "wifi_tests.h"
#include "diag_poe_psu.h"
#include "platform_cookie.h"
#include "platform_cpu.h"
#include "platform_fpga.h"
#include "queryflags.h"
#include "tam_act2_api_drv_support.h"
#include "platform_mcu.h"
#include "platform_sensor.h"
#include "plug_slot.h"
#include "plug_host_fpga_lib.h"
#include "platform_emmc.h"
#include "full_load.h"
#include "platform_mcu_upgrade.h"
#include "cookie_4.h"
#include "plug_common_host_impl.h"
#include "plug_host_fpga_lib.h"
#include "module_gshdsl.h"


/*****************************************************/
/*
 * Declare local function
 */

#ifdef SUPPORT_DISCRETE_AIKIDO_ACT2
static int act2_chip_select(void);
#endif
static boolean has_lte(int); 
static int io_interface_tests(void);
int display_sys_info(int);
boolean has_aux(int);
boolean has_poe(int);
boolean has_sfp(int);
boolean has_usb_console(int);
boolean has_xdsl(int);
boolean has_wifi_temp(int);
boolean has_fpga_sku_check(int);
boolean has_power_ok_led(int);
boolean has_console_stat_led(int);
boolean show_wifi_cookie_item(int);
static int ecc_err_injection(void);

/*
 * Declare external function
 */
extern int alt_mem(), dis_mem(), fil_mem(), mov_mem(), cmp_mem();       /* memory utility */
extern int memtest(), memloop(), addrloop(), find_mem(), memdebug();    /* memory utility */
extern char *banner_string;
extern int mb_tests(int);
extern int usb_utils(int);
extern void build_i2c_menu(void);
extern int debug_board_tests(boolean);
extern int dsl_tests(int);
extern int wifi_tests(int);
extern void diag_poe_psu(int);
extern int lte_subsystem_test(int);
extern int smartchip(int);
extern int alter_mb_cookie(void);
extern int alter_plug_cookie(void);
extern int alter_wifi_cookie(void);
extern int alter_poe_cookie(void);
extern int getdec_answer(char *, uint, uint, uint);
extern int diagact2_lib_initialize(char *, int);
extern int diag_extend_feature(boolean);
extern int tsn_all_green_leds_on(int);
extern int tsn_all_yellow_leds_on(int);
extern int tsn_all_leds_off(int);
extern int tsn_cpu_ondie_temp(int);
extern int tsn_volts_margin_util(int);
extern int  tsn_fpga_utils(int);
extern int plug_slot_module_info(int, uint16_t);

extern boolean tsn_has_poe(int);
extern int tsn_display_temp(void);
extern int tsn_display_voltage(void);
extern ushort get_mb_id(void);
extern int read_sfp_cookie(int);
extern int this_is_tsn_dsl_annex_sku(void);
extern int diag_psu_reg_test(int);
extern boolean wlan_io_test(void);
extern void insert_test_module(boolean);
extern int lte_reset_init(void);
extern void fload_start(void);
extern int emmc_full_test(int);
extern int tsn_led_ctrl_utils(int);

/*
 *  Globals  
 */
int netflashbooted = 0; /* menu.c need this */
boolean aikido_act2_flag = FALSE;
extern unsigned int tsn_gfast_sku;
unsigned short plug_slot1_cookid = 0;
unsigned short plug_slot2_cookid = 0;

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
     (type_t(*)())alter_mb_cookie, &one, 0, (type_t(*)())0, 0},
    {"alter Wifi cookie", 0, 0,
     (type_t(*)())alter_wifi_cookie, &zero, 0,
     (type_t(*)())show_wifi_cookie_item, 0},
    {"alter POE cookie", 0, 0,
     (type_t(*)())alter_poe_cookie, &zero, 0, (type_t(*)())has_poe, 0},
    {"read SFP cookie", 0, 0,
     (type_t(*)())read_sfp_cookie, &zero, 0, (type_t(*)())has_sfp, 0},
    {"alter Plug slot cookie", 0, 0,
     (type_t(*)())alter_plug_cookie, &zero, 0, (type_t(*)())this_is_star_with_sirius_fpga, 0},
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
     (PFT) display_sys_info, (type_t *) &one, 0,
     (type_t(*)())0, 0},

    {"Memory debug utilities", 0, 0,
    (PFT) menu, (type_t *) &mem_debug_menup, 0,
    (type_t(*)())0, 0},

    {"I2C utilities",
    0, 0,
    (PFT) build_i2c_menu, (type_t *) &one, 0,
    (type_t(*)())0, 0},

    {"Cookie utility",
     0, 0,
     (PFT) menu, (type_t *) &cookie_menup, 0,
     (type_t(*)())0, 0},

    {"Enumerate all USB",
    0, 0,
    (PFT) usb_utils, (type_t *) &one, 0,
    (type_t(*)())0, 0},

    {"Extended feature",
    0, 0,
    (PFT) diag_extend_feature, (type_t *)&zero, 0,
    (type_t(*)())0, 0},
#ifdef SUPPORT_DISCRETE_AIKIDO_ACT2
    {"ACT2 Chip Select", 
    0, 0,
    (PFT)act2_chip_select, (type_t *)&zero, 0,
    (type_t(*)())0, 0},
#endif
    {"Turn all Green LEDs ON",
    0, 0,
    (PFT)tsn_all_green_leds_on, (type_t *) &one, 0,
    (type_t(*)())0, 0},

    {"Turn all Yellow LEDs ON",
    0, 0,
    (PFT)tsn_all_yellow_leds_on, (type_t *) &one, 0,
    (type_t(*)())0, 0},

    {"Turn all LEDs OFF",
    0, 0,
    (PFT)tsn_all_leds_off, (type_t *) &one, 0,
    (type_t(*)())0, 0},

    {"CPU on-die Temperature",
    0, 0,
    (PFT)tsn_cpu_ondie_temp, (type_t *)&zero, 0,
    (type_t(*)())0, 0},

    {"Voltage Margin",
    0, 0,
    (PFT)tsn_volts_margin_util, (type_t *) &one, 0,
    (type_t(*)())0, 0},

    {"FPGA Utilities",
    0, 0,
    (PFT)tsn_fpga_utils,        (type_t *) &one, 0,
    (type_t(*)())0, 0},

    {"M/B Temperature utilities",
    0, 0,
    (PFT) build_snsr_menu,   (type_t *) &one, 0,
    (type_t(*)())0, 0},
    
    {"Wifi Temperature utilities",
    0, 0,
    (PFT) build_wifi_snsr_menu,   (type_t *) &one, 0,
    (type_t(*)())0, 0},
    
    {"CPU Switch mux LTE external micro USB",
    0, 0,
    (PFT) usb_lte_utility,   (type_t *) &one, 0,
    (type_t(*)())has_lte, 0},
    
    {"LTE external micro USB Switch mux CPU",
    0, 0,
    (PFT) usb_lte_utility,   (type_t *) &zero, 0,
    (type_t(*)())has_lte, 0},

    {"Power On Pluggable Module", 0, 0,
    (PFT) plug_pwr_on_util,   (type_t *) &one, 0,
    (type_t(*)())has_plug_slot, 0},
    
    {"Power Off Pluggable Module", 0, 0,
    (PFT) plug_pwr_off_util,   (type_t *) &one, 0,
    (type_t(*)())has_plug_slot, 0},
    
    {"Enable eMMC pSLC mode",
    0, 0,
    (PFT) emmc_pslc_fully_enable,   (type_t *) &zero, 0,
    (type_t(*)())0, 0},
    
    {"Show eMMC info",
    0, 0,
    (PFT) show_emmc_info,           (type_t *) &zero, 0,
    (type_t(*)())0, 0},

    {"Full Load Utility", 0, 0,
     (PFT) fload_start, (type_t *) &zero, 0,
     (type_t(*)())0, 0},

    {"eMMC full test",
    0, 0,
    (PFT) emmc_full_test,           (type_t *) &zero, 0,
    (type_t(*)())0, 0},
    
    {"USB console to UART", 0, 0,
    (PFT) usb_console_to_uart,   (type_t *) &one, 0,
    (type_t(*)())has_usb_console, 0},
    
    {"ECC Error Injection",
    0, 0,
    (PFT) ecc_err_injection,           (type_t *) &zero, 0,
    (type_t(*)())0, 0},
    
    {"LED Control Utilities",
    0, 0,
    (PFT)tsn_led_ctrl_utils,        (type_t *) &one, 0,
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


submenu_xtable_t main_menu_table[] = {
    {"ACT-2 utilities and programming",
     (PFT) smartchip, FALSE,
     MF_CONTINUOUS,
     (type_t(*)())0, 0, (PFT) smartchip, TRUE}
    ,
    {"I/O Interface tests",
    (PFT) io_interface_tests, FALSE,
    MF_CONTINUOUS | MF_DOALL | MF_SHOW_ERRCOUNT,
    (type_t(*)())star_io_interface_show, 0, (PFT)0, FALSE}
    ,
    {"Motherboard tests",
    (PFT) mb_tests, TRUE,
    MF_CONTINUOUS | MF_DOALL | MF_SHOW_ERRCOUNT,
    (type_t(*)())0, 0, (PFT) mb_tests, FALSE}
    ,
    {"ADSL2+VDSL2 tests", 
    (PFT)dsl_tests, TRUE,
    MF_CONTINUOUS | MF_DOALL | MF_SHOW_ERRCOUNT,
    (type_t(*)())has_xdsl, 0, (PFT) dsl_tests, FALSE}
    ,
    {"PoE PSU tests", 
    (PFT)diag_poe_psu, TRUE,
    MF_CONTINUOUS | MF_DOALL | MF_SHOW_ERRCOUNT,
    (type_t(*)())tsn_has_poe, 0, (PFT)diag_poe_psu, FALSE}
    ,
    {"LTE test",
    (PFT) lte_subsystem_test, FALSE,
    MF_CONTINUOUS | MF_DOALL | MF_SHOW_ERRCOUNT,
    (type_t(*) ())tsn_fpga_check_dev_present, FPGA_CPP_LTE0_PRESENT,
    (PFT) lte_subsystem_test, TRUE}
    ,
    {"WiFi tests",
    (PFT) wifi_tests, TRUE,
    MF_CONTINUOUS | MF_DOALL | MF_SHOW_ERRCOUNT,
    (type_t(*)())tsn_fpga_check_dev_present, FPGA_CPP_WLAN_PRESENT,
    (PFT) wifi_tests, FALSE}
    ,
    {"Pluggable Slot 1 tests",
     (PFT) plug_test, PLUG_SLOT_1 ,
     MF_CONTINUOUS | MF_DOALL | MF_SHOW_ERRCOUNT,
     (type_t(*)())has_plug_slot, PLUG_SLOT_1,
     (PFT) plug_test, PLUG_SLOT_1 + MAX_PLUG_SLOT_NUMBER}
    ,
    {"Pluggable Slot 2 tests",
     (PFT) plug_test, PLUG_SLOT_2 ,
     MF_CONTINUOUS | MF_DOALL | MF_SHOW_ERRCOUNT,
     (type_t(*)())this_is_star_c1109_4p, PLUG_SLOT_2,
     (PFT) plug_test, PLUG_SLOT_2 + MAX_PLUG_SLOT_NUMBER}
    ,
    {"GSHDSL tests", 
    (PFT)gshdsl_tests, TRUE,
    MF_CONTINUOUS | MF_DOALL | MF_SHOW_ERRCOUNT,
    (type_t(*)())this_is_tsn_gshdsl_sku, 0, (PFT) gshdsl_tests, FALSE}
    ,
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
 * Function   : has_aux
 * Description: Function to check if support AUX feature.
 * Inputs     : opt - reserve for future use
 * Outputs    : TRUE(yes)/FALSE(no)
 *
 **********************************************************************
 */
boolean has_aux (int opt)
{
    if (this_is_star()) {
        return (FALSE);
    }
    if (this_is_supernova()) {
        return (FALSE);
    }
    return (TRUE);
}

/**********************************************************************
 *
 * Function   : has_poe
 * Description: Function to check if support POE feature.
 * Inputs     : opt - reserve for future use
 * Outputs    : TRUE(yes)/FALSE(no)
 *
 **********************************************************************
 */
boolean has_poe (int opt)
{
    if (this_is_star()) {
        return (FALSE);
    }
    if (this_is_supernova()) {
        return (FALSE);
    }
    return (TRUE);
}

/**********************************************************************
 *
 * Function   : has_sfp
 * Description: Function to check if support SFP feature.
 * Inputs     : opt - reserve for future use
 * Outputs    : TRUE(yes)/FALSE(no)
 *
 **********************************************************************
 */
boolean has_sfp (int opt)
{
    if (this_is_star()) {
        return (FALSE);
    }
    if (this_is_supernova()) {
        return (FALSE);
    }
    return (TRUE);
}

/**********************************************************************
 *
 * Function   : has_usb_console
 * Description: Function to check if support USB console.
 * Inputs     : opt - reserve for future use
 * Outputs    : TRUE(yes)/FALSE(no)
 *
 **********************************************************************
 */
boolean has_usb_console (int opt)
{
    if (this_is_star()) {
        return (TRUE);
    }
    if (this_is_supernova()) {
        return (TRUE);
    }
    return (FALSE);
}

/**********************************************************************
 *
 * Function   : has_lte
 * Description: Function to check if this board has LTE feature.
 * Inputs     : opt - reserve for future use
 * Outputs    : TRUE(yes)/FALSE(no)
 *
 **********************************************************************
 */
static boolean has_lte (int opt)
{
    uint b_type = 0;

    if (tsn_get_boardtype(&b_type) != PASSED) {
        printf("%s: Failed to get Board Type.\n", __FUNCTION__);
        return (FALSE);
    }

    if ((b_type & TSN_W_LTE) == TSN_W_LTE) {
        return (TRUE);
    }
    return (FALSE);
}

/**********************************************************************
 *
 * Function   : has_xdsl
 * Description: Function to check if this board has xdsl feature.
 * Inputs     : opt - reserve for future use
 * Outputs    : TRUE(yes)/FALSE(no)
 *
 **********************************************************************
 */
boolean has_xdsl (int opt)
{
    uint b_type = 0;

    /* TSH GSHDSL doesn't have xdsl */
    if (this_is_tsn_gshdsl_sku() == TRUE) {
        return (FALSE);
    }

    if (tsn_get_boardtype(&b_type) != PASSED) {
        printf("%s: Failed to get Board Type.\n", __FUNCTION__);
        return (FALSE);
    }

    /* Based on TSN-H HW design, GE0 ethernet and xDSL SKU feature
     * shared the same SERDES Lane1.
     */
    if ((b_type & TSN_W_GE1) == TSN_W_GE1) {
        return (FALSE);
    }
    if (this_is_star()) {
        return (FALSE);
    }
    if (this_is_supernova()) {
        return (FALSE);
    }

    return (TRUE);
}

/**********************************************************************
 *
 * Function   : has_wifi
 * Description: Function to check if this board has wifi feature.
 * Inputs     : opt - reserve for future use
 * Outputs    : TRUE(yes)/FALSE(no)
 *
 **********************************************************************
 */
boolean has_wifi (int opt)
{
    uint b_type = 0;

    if (tsn_get_boardtype(&b_type) != PASSED) {
        printf("%s: Failed to get Board Type.\n", __FUNCTION__);
        return (FALSE);
    }

    if ((b_type & TSN_W_WIFI) == TSN_W_WIFI) {
        return (TRUE);
    }
    return (FALSE);
}

/**********************************************************************
 *
 * Function   : has_wifi_temp
 * Description: Function to check if has WIFI temperature.
 * Inputs     : opt - reserve for future use
 * Outputs    : TRUE(yes)/FALSE(no)
 *
 **********************************************************************
 */
boolean has_wifi_temp (int opt)
{
    if (this_is_star()) {
        return (FALSE);
    }
    if (this_is_supernova()) {
        return (FALSE);
    }
    return (TRUE);
}

/**********************************************************************
 *
 * Function   : has_fpga_sku_check
 * Description: has FGPA provide SKU with platform cookie.
 * Inputs     : opt - reserve for future use
 * Outputs    : TRUE(yes)/FALSE(no)
 *
 **********************************************************************
 */
boolean has_fpga_sku_check (int opt)
{
    if (this_is_star()) {
        return (TRUE);
    }
    if (this_is_supernova()) {
        return (TRUE);
    }
    return (FALSE);
}

/**********************************************************************
 *
 * Function   : has_power_ok_led
 * Description: has power OK led.
 * Inputs     : opt - reserve for future use
 * Outputs    : TRUE(yes)/FALSE(no)
 *
 **********************************************************************
 */
boolean has_power_ok_led (int opt)
{
    if (this_is_star()) {
        return (FALSE);
    }
    if (this_is_supernova()) {
        return (FALSE);
    }
    return (TRUE);
}

/**********************************************************************
 *
 * Function   : has_console_stat_led
 * Description: has console state led.
 * Inputs     : opt - reserve for future use
 * Outputs    : TRUE(yes)/FALSE(no)
 *
 **********************************************************************
 */
boolean has_console_stat_led (int opt)
{
    if (this_is_star()) {
        return (FALSE);
    }
    if (this_is_supernova()) {
        return (FALSE);
    }
    return (TRUE);
}

/**********************************************************************
 *
 * Function: display_sys_info
 *
 * Description: display system info, ex. current time, Diag ver
 *
 * Input : dummy
 *                     
 * Output: None
 *
 **********************************************************************
 */

int display_sys_info (int dummy)
{
    int rc = PASSED;
    
    rc = tsn_show_cpuinfo();
    if (rc != PASSED) {
        printf("Failed show CPU info.\n");
        return (FAILED);
    }

    rc = tsn_display_temp();
    if (rc != PASSED) {
        printf("Failed show temperature.\n");
        return (FAILED);
    } 

    rc = tsn_display_voltage();
    if (rc != PASSED) {
        printf("Failed show voltage.\n");
        return (FAILED);
    }
    
    tsn_show_devbus_info();
    system("/etc/print_version.sh");
    mcu_fw_verno();
    if (this_is_star_c1109_4p()) {
        mcu_volcur_check(); 
    }

    tsn_show_fpga_ver(dummy);

    if (this_is_star_c1101p() || this_is_star_c1109_4p()) {
        show_plug_fpga_ver(dummy);
        plug_slot1_cookid = plug_cookie_get(PLUG_SLOT_1);
        plug_module_power_off(PLUG_SLOT_1);
        plug_slot_module_info(PLUG_SLOT_1, plug_slot1_cookid);
        
        if (this_is_star_c1109_4p()) {
            plug_slot2_cookid = plug_cookie_get(PLUG_SLOT_2);
            plug_module_power_off(PLUG_SLOT_2);
            plug_slot_module_info(PLUG_SLOT_2, plug_slot2_cookid);
        }
    }

    get_mb_id();
    if (this_is_star() == TRUE) {
       printf("This is Star.\n");  
    } else if (this_is_supernova() == TRUE) {
       printf("This is Supernova.\n");
    } else if (this_is_tsn_h_sku() == TRUE) {
       printf("This is TSN-H.\n");
    } else {
       printf("This is TSN-M.\n");
    }

    if ((this_is_star() == TRUE) || (this_is_supernova() == TRUE)) {
        /* Bypass xDSL info */
    /* check if dsl SKU */
    } else if (has_xdsl(0) == TRUE) {
        if ((this_is_tsn_dsl_annex_sku() == (DSL138_SKU_GFAST | DSL138_SKU_ANNEX_A)) && (tsn_gfast_sku == TRUE)) {
            printf("G.Fast Annex A\n");
            rc = PASSED;
        } else
        if ((this_is_tsn_dsl_annex_sku() == (DSL138_SKU_GFAST | DSL138_SKU_ANNEX_B)) && (tsn_gfast_sku == TRUE)) {
            printf("G.Fast Annex B/J\n");
            rc = PASSED;
        } else
        if ((this_is_tsn_dsl_annex_sku() == (DSL138_SKU_GFAST | DSL138_SKU_ANNEX_M)) && (tsn_gfast_sku == TRUE)) {
            printf("G.Fast Annex M\n");
            rc = PASSED;
        } else
        if ((this_is_tsn_dsl_annex_sku() == DSL_SKU_ANNEX_A) && (tsn_gfast_sku == FALSE)) {
            printf("xDSL Annex A\n");
            rc = PASSED;
        } else
        if ((this_is_tsn_dsl_annex_sku() == DSL_SKU_ANNEX_B) && (tsn_gfast_sku == FALSE)) {
            printf("xDSL Annex B/J\n");
            rc = PASSED;
        } else
        if ((this_is_tsn_dsl_annex_sku() == DSL_SKU_ANNEX_M) && (tsn_gfast_sku == FALSE)) {
            printf("xDSL Annex M\n");
            rc = PASSED;
        } else {
            printf("Unknown xDSL Annex??\n");
            rc = FAILED;
        }
    } else {
        printf("E-to-E Sku\n");
        rc = PASSED;
    }        
    printf("%s", banner_string);
    
    return (rc);
}


#ifdef SUPPORT_DISCRETE_AIKIDO_ACT2
/*------------------------------------------------------------------------------
 *
 * Function: act2_chip_select().
 *
 * This function select the Discrete-ACT2 or Aikido-ACT2
 *
 * Input : None
 *
 * Output: PASSED/FAILED.
 *
 *------------------------------------------------------------------------------
 */
static int act2_chip_select (void)
{
    int  act2_chip;
    char i2c_adapter[] = "/dev/i2c-0";

    printf("\n\nSelect Act2 Chip:");
    act2_chip = getdec_answer("\n(0-Discrete-ACT2, 1-Aikido-ACT2):", 0, 0, 1);
    if (act2_chip == 1) {
        aikido_act2_flag = TRUE;
        printf("\nSelect AIKIDO ACT2\n");
    } else {
        aikido_act2_flag = FALSE;
        printf("\nSelect Discrete ACT2\n");
    }
    if (diagact2_lib_initialize(i2c_adapter, MB_I2C_ADDR_ACT2) != PASSED) {
        return (FAILED);
    }
    return (PASSED);
}
#endif

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
    int rc = FAILED;
    int plug_slot;
    int stat = FAILED;
    char *tname = "";

    tname = "I/O interface";
    testname(tname);
    prpass(testpass, "%s, ", tname);
   
    /* TSN I/O interface test */
    if ((this_is_star() != TRUE) && (this_is_supernova() != TRUE)) {
    /*POE*/
    if (tsn_fpga_check_dev_present(FPGA_CPP_POE_PRESENT) == TRUE) {
        if (diag_psu_reg_test(0) == PASSED) {
            rc = PASSED;
            prpass(testpass, "%s POE detected.", tname);
        } else {
            cterr('w', 0, "POE is not detected.");
            return (FAILED);
        }
    } else {
        rc = PASSED;
        prpass(testpass, "%s POE not present.", tname);
    }
    fflush(0);    
    /*WiFi*/
    if (has_wifi(0) == TRUE) {
        if (tsn_fpga_check_dev_present(FPGA_CPP_WLAN_PRESENT) == TRUE) {
            if (wlan_io_test() == PASSED) {
                rc = PASSED;
                prpass(testpass, "%s WiFi detected.", tname);
            } else {
                cterr('w', 0, "WiFi is not detected.");
                return (FAILED);
            }
        } else {
            cterr('w', 0, "Wifi SKU, but WiFi DC is not present.");
            return (FAILED);
        }
    } else {
        rc = PASSED;
        prpass(testpass, "%s None Wifi SKU.", tname);
    }
    fflush(0);    

    /*LTE*/
    if (has_lte(0) == TRUE) {
        if (tsn_fpga_check_dev_present(FPGA_CPP_LTE0_PRESENT) == TRUE) {
            insert_test_module(TRUE);
            /* Do the LTE reset initialization sequence. */
            if (lte_reset_init() == PASSED) {
                insert_test_module(FALSE); 
                rc = PASSED;
                prpass(testpass, "%s LTE detected\n", tname);
            } else {
                insert_test_module(FALSE);
                cterr('w', 0, "LTE module is not detected.");
                return (FAILED);
            }
        } else {
            cterr('w', 0, "LTE module is not present.");
            return (FAILED);
        }
    } else {
        rc = PASSED;
        prpass(testpass, "%s None LTE SKU.", tname);
    }
    fflush(0);    
    } else {
        /*WiFi*/
        if (has_wifi(0) == TRUE) {
            if (tsn_fpga_check_dev_present(FPGA_CPP_WLAN_PRESENT) == TRUE) {
                if (wlan_io_test() == PASSED) {
                    rc = PASSED;
                    prpass(testpass, "%s WiFi detected.", tname);
                } else {
                    cterr('w', 0, "WiFi is not detected.");
                    return (FAILED);
                }
            } else {
                cterr('w', 0, "Wifi SKU, but WiFi DC is not present.");
                return (FAILED);
            } 
        } else {
            rc = PASSED;
            prpass(testpass, "%s None Wifi SKU.", tname);
        }
        /* Check if C1101 or C1109-4P */
        if (has_plug_slot(PLUG_SLOT_1) == TRUE) {
            for (plug_slot = PLUG_SLOT_1; plug_slot < (PLUG_SLOT_1 + MAX_PLUG_SLOT_NUMBER);
                plug_slot ++) {
                if ((star_plug_is_present(plug_slot)) == TRUE) {
                    stat = plug_intf_test(plug_slot);
                } else {
                    stat = FAILED;
                    printf("%s: Pluggable slot %d is not present.\n", __func__, plug_slot);
                }
                /* Break the loop since C1109P only has one slot */
                if ((stat !=PASSED) || (this_is_star_c1101p() == TRUE)) {
                    break;
                }
            }
            if (stat == PASSED) {
                rc = PASSED;
                prpass(testpass, "%s Pluggable slot I/O interface test passed.\n", tname);
            } else {
                cterr('f', 0, "Pluggable slot I/O interface test failed.");
                return (FAILED);
            }
        } else {
            rc = PASSED;
            prpass(testpass, "%s No Pluggable slot.", tname);
        } 
        fflush(0);   
    }
    
    prcomplete(testpass, errcount, (char *)0);
    return (rc);
}

/*******************************************************************************
 *
 * Function   : show_wifi_cookie_item
 * Description: Function to check if WiFi cookie item is needed to show.
 * Inputs     : opt - reserve for future use
 * Outputs    : TRUE(yes)/FALSE(no)
 *
 *******************************************************************************
 */
boolean show_wifi_cookie_item (int opt)
{
    /* Based on current Star/Supernova WiFi design, there's no EEPROM for cookie as usual
     * and they are stored in flash of WiFi module.
     * Host side can't read WiFi cookie directly, so temporarily masked out
     * "alter WiFi cookie" item from menu.
     */
    if ((this_is_star() == TRUE) || (this_is_supernova() == TRUE)) {
        return (FALSE);
    }
    return (TRUE);
}

/*******************************************************************************
 *
 * Function   : ecc_err_injection
 * Description: Function to inject ECC errors
 * Inputs     : N/A
 * Outputs    : N/A
 *
 *******************************************************************************
 */
static int ecc_err_injection(void)
{
	printf("If you want to stop ECC error injection, you need to power cycle.\n");
    if(getc_answer("Are you sure to inject ECC errors?", "yn", 'n') == 'n') {
        return (PASSED);
	}
    
	system(ECC_ERR_LOG_CONFIG);
	system(ECC_1BIT_ERR_COUNTER);
	system(ECC_ERR_INFO_0);
	system(ECC_ERR_INFO_1);
	system(INTERRUPT_STATUS_REG);
	system(INTERRUPT_ENABLE_REG);
	system(PHY_REG_FILE_ACCESS_0);
	system(PHY_REG_FILE_ACCESS_1);
	
	return (PASSED);
}

/*-------------------------------------------------
$Log: diag.c,v $
Revision 1.10  2019/01/18 05:54:46  yungchen
Merge Supernova branch to the main trunk (CSCvn79871)

Revision 1.9  2018/11/23 08:49:51  hondwang
Re-instruct pluggable common code with CDETs CSCvn17216

Revision 1.8.36.2  2018/10/22 09:38:07  hondwang
move plug_slot_module_info to common codeplug_common/plug_common_lib.c

Revision 1.8.36.1  2018/10/15 06:53:07  hondwang
pluggable common code re-instruct modify code

Revision 1.8  2018/05/15 09:37:32  steja
CSCvj38863: Enhanced LED single test utility

Revision 1.7  2018/05/09 06:53:12  letsai
Add TSN GSHDSL portion

Revision 1.6  2018/02/09 09:56:53  hondwang
Merge Star branch star-branch-c9xx to main trunk

Revision 1.5  2018/01/23 11:38:18  steja
Merge tsn-gfast-branch4 code to maintrunk for support TSN-G.Fast (CSCvh40981)

Revision 1.4.16.4  2018/02/07 10:23:01  lucywang
Followed coding rule

Revision 1.4.16.3  2018/01/22 01:05:42  lucywang
Added utility to inject ECC error

Revision 1.4.16.2  2018/01/20 07:21:47  hondwang
Fix some merge branch issue

Revision 1.4.16.1  2018/01/20 06:27:23  hondwang
prepare merge star-branch-c9xx to main trunk

Revision 1.4.14.2  2018/01/15 08:59:50  steja
Updade code based on code review comment (CSCvh40981)

Revision 1.4.14.1  2018/01/11 08:02:44  steja
gfast branch4 sync with maintrunk

Revision 1.4  2017/09/06 12:14:20  steja
1.Fix TSN WIFI ACT2 i2c scan test failed at first time after power on (CSCvf83218)
2. Remove Discrete ACT2 utility and I2C Scan for Discrete ACT2 only for Development phase(CSCvf81035)

Revision 1.3  2017/09/04 16:09:41  palin2
Added utilities to enable fully eMMC pSLC mode and show eMMC info.(CSCvf82437)

Revision 1.2.4.17  2017/12/08 11:20:27  hondwang
Add console link with USB or RJ45 utility

Revision 1.2.4.16  2017/11/23 09:19:00  lucywang
Display utility p & q for on-board LTE only

Revision 1.2.4.15  2017/11/23 03:20:44  hondwang
Show plug module name with system info function

Revision 1.2.4.14  2017/11/22 09:45:46  hondwang
Fix demo SKU and menu show

Revision 1.2.4.13  2017/11/20 07:54:31  lucywang
Changed PID to C1101/C1109-2P/C1109-4P

Revision 1.2.4.12  2017/11/13 09:05:49  hondwang
Add pluggable slot cookie info with Diag login

Revision 1.2.4.11  2017/10/24 11:52:45  palin2
Temporarily masked out "alter WiFi cookie" item from cookie utiliy on Star
because of HW design difference on Star WiFi module.

Revision 1.2.4.10  2017/10/20 07:55:26  lucywang
Added utility to test full eMMC

Revision 1.2.4.9  2017/10/20 04:41:53  hondwang
Add C949-4P MCU voltage and currently display by HW request.

Revision 1.2.4.8  2017/10/03 21:56:26  hondwang
Add MCU and Kernel version dump with system utility

Revision 1.2.4.7  2017/09/29 23:01:22  hondwang
Show FPGA version with sys_info function

Revision 1.2.4.6  2017/09/21 00:22:01  lucywang
fixed build failed issue

Revision 1.2.4.5  2017/09/20 08:18:09  lucywang
added full load utility

Revision 1.2.4.4  2017/09/14 23:36:11  hondwang
Add Wifi with IO interface testing

Revision 1.2.4.3  2017/09/04 15:13:15  shjung
Add utility to enable eMMC pSLC

Revision 1.2.4.2  2017/08/28 07:52:50  shjung
Added pluggable I/O interface test

Revision 1.2.4.1  2017/08/15 14:18:38  hondwang
star branch c9xx initial check in

Revision 1.2  2017/08/02 14:21:44  steja
Support TSN-H/M platform code

Revision 1.1.8.2  2017/07/29 03:41:01  steja
tsn-branch5 synced with Maintrunk repositories

Revision 1.1.6.4  2017/07/24 14:14:10  palin2
1. To improve code readability.
2. All changes are verified before check-in.

Revision 1.1.6.3  2017/07/21 10:46:03  steja
Update based on code review comment

Revision 1.1.6.2  2017/07/20 13:38:03  steja
tsn-branch4 merge with maintrunk

Revision 1.1.4.15.6.10  2017/07/26 14:04:53  hondwang
Add USB console check function

Revision 1.1.4.15.6.9  2017/07/25 23:48:09  shjung
Add power on/off pluggable module utility

Revision 1.1.4.15.6.8  2017/07/25 23:42:49  shjung
Fix pluggable slot test item show up issue

Revision 1.1.4.15.6.7  2017/07/18 05:44:55  hondwang
Fix build issue

Revision 1.1.4.15.6.5  2017/07/10 09:53:30  hondwang
Star not support xdsl, fix show xdsl item issue

Revision 1.1.4.15.6.4  2017/06/16 13:03:33  tirawan
I2C driver modification to support ACT2 cookie programming

Revision 1.1.4.15.6.3  2017/06/16 06:52:37  tirawan
Foxconn Pluggable FPGA I2C Read/Write function correction during the bring up

Revision 1.1.4.15.6.2  2017/06/14 14:03:02  hondwang
Add plug program in menu

Revision 1.1.4.15.6.1  2017/06/13 09:35:56  tirawan
Add Pluggable Discovery function for Star C941, and add pluggable temperature sensor, GPIO Expander test functions

Revision 1.1.4.15.2.5  2017/07/18 14:23:37  steja
Code Cleanup

Revision 1.1.4.15.2.4  2017/07/18 03:53:00  steja
Code cleanup

Revision 1.1.4.15.2.3  2017/07/13 12:47:15  steja
code clean up io interface

Revision 1.1.4.15.2.2  2017/07/11 10:13:16  steja
1. Remove Debugcard test
2. Add LTE micro usb utility to basic utilities
3. Code clean up

Revision 1.1.4.15.2.1  2017/05/17 01:17:52  palin2
Updated GE WAN mapping number with team's decision.
(GE0: GE WAN with SFP; GE1: 2nd GE WAN)

Revision 1.1.4.15  2016/12/30 09:12:44  steja
Enhanced io interface test

Revision 1.1.4.14  2016/12/23 11:00:15  steja
Add IO interface test

Revision 1.1.4.13  2016/11/29 02:54:39  palin2
Dynamically getting device bus window base from CPU register.

Revision 1.1.4.12  2016/10/11 13:09:01  steja
Add show current voltage in initial diag

Revision 1.1.4.11  2016/10/07 13:07:55  steja
1. Add Check xDSL sku type
2. Support Annex B

Revision 1.1.4.10  2016/09/13 14:35:47  steja
Commit Aikido / TAM Mailbox code

Revision 1.1.4.9  2016/09/08 13:45:19  steja
Add Utilitiy to read SFP eeprom

Revision 1.1.4.8  2016/07/22 14:39:24  steja
Add Read MB cookie id

Revision 1.1.4.7  2016/07/22 13:04:37  palin2
Added function to check DC present.

Revision 1.1.4.6  2016/07/21 14:26:32  steja
1. Update Temperature sensor function
2. Add Wifi Temperature sensor to basic utilities
3. Add LTE Extended Feature Test
4. Display Chasis Temperature When Boot up
5. Update I2C scan function
6. Update POE cookie check card present

Revision 1.1.4.5  2016/07/18 13:14:29  steja
1. Move M/B Temperature sensor register test to run as default test.
2. Move M/B Temperature utilities under basic utilities.

Revision 1.1.4.4  2016/07/17 10:36:58  palin2
Added FPGA utilities entry in basic utility submenu.

Revision 1.1.4.3  2016/07/10 10:29:32  steja
Add LED test

Revision 1.1.4.2  2016/06/30 06:22:47  steja
tsn-branch2 sync with main trunk

Revision 1.1.2.21  2016/06/21 04:36:33  palin2
Added voltage margin utility and MCU register R/W utilities.

Revision 1.1.2.20  2016/06/16 07:51:11  palin2
Updated PoE related utilities and code.

Revision 1.1.2.19  2016/06/03 01:00:45  palin2
Added function to show CPU on die temperature.

Revision 1.1.2.18  2016/05/26 11:53:03  palin2
Added utilities to turn TSN all Green/Yellow LEDs ON.

Revision 1.1.2.17  2016/05/23 06:21:37  leschen
Support wifi portion.

Revision 1.1.2.16  2016/05/16 13:12:37  steja
Fix code compiled error

Revision 1.1.2.15  2016/05/16 12:47:36  steja
Move Debug card and Extended Feature to Utility items

Revision 1.1.2.14  2016/05/16 09:13:39  steja
Add Extened feature for Bridge software

Revision 1.1.2.13  2016/05/16 06:44:55  palin2
Add function to get TSN board type, and config Diag test items in menu for
different SKUs based on its board type info.

Revision 1.1.2.12  2016/05/06 16:10:18  steja
Bring up I2C-2 for RTC

Revision 1.1.2.11  2016/04/22 11:33:59  steja
check-in for first release

Revision 1.1.2.10  2016/03/27 14:17:33  steja
update based on code review comment 3/25/2016

Revision 1.1.2.9  2016/03/24 10:35:04  steja
Add Cookie and Act2 programming

Revision 1.1.2.8  2016/03/24 03:58:26  steja
Add LTE test

Revision 1.1.2.7  2016/03/22 22:19:12  palin2
Added PoE PSU Diag.

Revision 1.1.2.6  2016/03/22 09:28:20  leschen
Add 63168 test items.

Revision 1.1.2.5  2016/03/21 02:56:06  steja
Add debug card test items

Revision 1.1.2.4  2016/03/20 12:24:01  steja
Add I2C utilities

Revision 1.1.2.3  2016/03/16 10:06:15  steja
add usb utility

Revision 1.1.2.2  2016/03/14 14:32:03  steja
Add memory test

Revision 1.1.2.1  2016/03/08 09:55:10  steja
Initial Check-in


*/
