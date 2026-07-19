/* $Id: diag.c,v 1.2 2021/09/24 01:21:05 harrchan Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/elixir/diag.c,v $
 *------------------------------------------------------------------
 * 
 * diag.c
 *
 * Copyright (c) 2018 - 2019 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
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
#include "diag_i2c_lib.h"
#include "platform_i2c.h"
#include "diag_mother_board_test.h"
#include "diag_wifi_test.h"
#include "platform_cookie.h"
#include "queryflags.h"
#include "tam_act2_api_drv_support.h"
#include "cookie_4.h"
#include "diag_temp_sensor_util.h"
#include "dev_mrvl_ge.h"
#include "diag_ge_phy_lib.h"
#include "diag_poe_psu_test.h"
#include "diag_poe_psu_lib.h"
#include "diag_led_util.h"
#include "diag_emmc_util.h"
#include "diag_emmc_test.h"
#include "diag_moka_fpga_lib.h"
#include "diag_moka_fpga_util.h"
#include "diag_cpu_util.h"
#include "linux_main.h"
#include "diag_i2c_util.h"
#include "plug_slot.h"
#include "diag_sirius_fpga_lib.h"
#include "plug_host_fpga_lib.h"
#include "diag_usb_lib.h"
#include "diag_mcu_lib.h"
#include "diag_mcu_util.h"
#include "dev_98dxc25x.h"
#include "diag_esw_lib.h"


/*****************************************************/
/*
 * Declare local function
 */
static int diag_io_interface_test(void);
static int diag_plug_test(plug_slot_no);
static int diag_display_sys_info_util(int);

/*
 * Declare external function
 */
extern int alt_mem(), dis_mem(), fil_mem(), mov_mem(), cmp_mem();       /* memory utility */
extern int memtest(), memloop(), addrloop(), find_mem(), memdebug();    /* memory utility */
extern char *banner_string;
extern int debug_board_tests(boolean);
extern int lte_subsystem_test(int);
extern int smartchip(int);
extern int alter_mb_cookie(void);
extern int alter_wifi_cookie(void);
extern int alter_poe_cookie(void);
extern int getdec_answer(char *, uint, uint, uint);
extern int diagact2_lib_initialize(char *, int);

extern int plat_display_voltage(void);
extern ushort get_mb_id(void);
extern int diag_psu_reg_test(int);
extern boolean wlan_io_test(void);
extern void insert_test_module(boolean);
extern int lte_reset_init(void);

/*
 *  Globals  
 */
int netflashbooted = 0; /* menu.c need this */
boolean aikido_act2_flag = FALSE;
extern unsigned int plat_gfast_sku;
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
    {"alter POE cookie", 0, 0,
     (type_t(*)())alter_poe_cookie, &zero, 0, (type_t(*)())platform_has_poe, 0},
    {"read SFP0 cookie", 0, 0,
     (type_t(*)())diag_read_sfp_cookie, &zero, 0, (type_t(*)())0, GE0},
    {"read SFP1 cookie", 0, 0,
     (type_t(*)())diag_read_sfp_cookie, &zero, 0, (type_t(*)())0, GE1},
    {"alter Plug slot cookie", 0, 0,
     (type_t(*)())alter_plug_cookie, &zero, 0, (type_t(*)())platform_has_pluggable, 0},
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
     (PFT) diag_display_sys_info_util, (type_t *) &one, 0,
     (type_t(*)())0, 0},

    {"CPU Utility",   0, 0,
    (PFT)diag_cpu_util,           (type_t *) &zero, 0,
    (type_t(*)())0, 0},
    
    {"Memory Debug Utility", 0, 0,
    (PFT) menu, (type_t *) &mem_debug_menup, 0,
    (type_t(*)())0, 0},

    {"I2C Utility",
    0, 0,
    (PFT) diag_i2c_util, (type_t *) &one, 0,
    (type_t(*)())0, 0},

    {"Cookie Utility",
     0, 0,
     (PFT) menu, (type_t *) &cookie_menup, 0,
     (type_t(*)())0, 0},

    {"Enumerate All USB",
    0, 0,
    (PFT) diag_usb_util, (type_t *) &one, 0,
    (type_t(*)())0, 0},

    {"Turn all Green LEDs ON",
    0, 0,
    (PFT)diag_led_all_green_on_util, (type_t *) &one, 0,
    (type_t(*)())0, 0},

    {"Turn all Yellow LEDs ON",
    0, 0,
    (PFT)diag_led_all_yellow_on_util, (type_t *) &one, 0,
    (type_t(*)())0, 0},

    {"Turn all LEDs OFF",
    0, 0,
    (PFT)diag_led_all_off_util, (type_t *) &one, 0,
    (type_t(*)())0, 0},

    {"CPU on-die Temperature",
    0, 0,
    (PFT)diag_cpu_ondie_temp_util, (type_t *)&zero, 0,
    (type_t(*)())0, 0},

    {"Voltage Margin",
    0, 0,
    (PFT)diag_volts_margin_util, (type_t *) &one, 0,
    (type_t(*)())0, 0},

    {"FPGA Utility",
    0, 0,
    (PFT)diag_moka_fpga_util,        (type_t *) &one, 0,
    (type_t(*)())0, 0},

    {"M/B Temperature utility",
    0, 0,
    (PFT)diag_temp_sensor_util,  (type_t *) &one, 0,
    (type_t(*)())0, 0},
    
    {"Power On Pluggable Module", 0, 0,
    (PFT) diag_plug_pwr_on_util,   (type_t *) &one, 0,
    (type_t(*)())platform_has_pluggable, 0},
    
    {"Power Off Pluggable Module", 0, 0,
    (PFT) diag_plug_pwr_off_util,   (type_t *) &one, 0,
    (type_t(*)())platform_has_pluggable, 0},
    
    {"Enable eMMC pSLC mode",
    0, 0,
    (PFT)diag_emmc_pslc_fully_en_util,   (type_t *) &zero, 0,
    (type_t(*)())0, 0},
    
    {"Show eMMC info",
    0, 0,
    (PFT) diag_show_emmc_info_util,           (type_t *) &zero, 0,
    (type_t(*)())0, 0},

    {"eMMC full test",
    0, 0,
    (PFT)diag_emmc_full_size_rw_test,           (type_t *) &zero, 0,
    (type_t(*)())0, 0},
    
    {"LED Control Utility",
    0, 0,
    (PFT)diag_led_ctrl_util,        (type_t *) &one, 0,
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
    (PFT) diag_io_interface_test, FALSE,
    MF_CONTINUOUS | MF_DOALL | MF_SHOW_ERRCOUNT,
    (type_t(*)())platform_has_sirius_fpga, 0, (PFT)0, FALSE}
    ,
    {"Motherboard tests",
    (PFT) diag_mother_board_test, TRUE,
    MF_CONTINUOUS | MF_DOALL | MF_SHOW_ERRCOUNT,
    (type_t(*)())0, 0, (PFT) diag_mother_board_test, FALSE}
    ,
    {"PoE PSU tests", 
    (PFT)diag_poe_psu_test, TRUE,
    MF_CONTINUOUS | MF_DOALL | MF_SHOW_ERRCOUNT,
    (type_t(*)())platform_has_poe, 0, (PFT)diag_poe_psu_test, FALSE}
    ,
    {"WiFi tests",
    (PFT)diag_wifi_test, TRUE,
    MF_CONTINUOUS | MF_DOALL | MF_SHOW_ERRCOUNT,
    (type_t(*)())0, 0,
    (PFT)diag_wifi_test, FALSE}
    ,
    {"Pluggable Slot 1 tests",
     (PFT) diag_plug_test, PLUG_SLOT_1 ,
     MF_CONTINUOUS | MF_DOALL | MF_SHOW_ERRCOUNT,
     (type_t(*)())platform_has_pluggable, 0,
     (PFT) diag_plug_test, PLUG_SLOT_1 + MAX_PLUG_SLOT_NUMBER}
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
 * Function: diag_display_sys_info_util
 *
 * Description: display system info, ex. current time, Diag ver
 *
 * Input : dummy
 *                     
 * Output: None
 *
 **********************************************************************
 */

static int diag_display_sys_info_util (int dummy)
{
    int rc = PASSED;
    
    printf("\n====================================\n");
    printf("========== Show CPU Info. ==========\n");
    printf("====================================\n");
    rc = plat_show_cpuinfo();
    if (rc != PASSED) {
        printf("Failed show CPU info.\n");
        return (FAILED);
    }

    printf("\n=========================================\n");
    printf("========== Show Platform Temp. ==========\n");
    printf("=========================================\n");
    rc = show_plat_curr_temps();
    if (rc != PASSED) {
        printf("Failed show temperature.\n");
        return (FAILED);
    } 

    printf("\n========================================\n");
    printf("========== Show Voltage Info. ==========\n");
    printf("========================================\n");
    rc = plat_display_voltage();
    if (rc != PASSED) {
        printf("Failed show voltage.\n");
        return (FAILED);
    }
    
    printf("\n=======================================\n");
    printf("========== Show Kernel Info. ==========\n");
    printf("=======================================\n");
    diag_cpu_system_show_devbus_info_util();
    system("/etc/print_version.sh");

    printf("\n======================================\n");
    printf("========== Show MCU Version ==========\n");
    printf("======================================\n");
    mcu_fw_verno();

    printf("\n================================================\n");
    printf("========== Show Platform FPGA Version ==========\n");
    printf("================================================\n");
    plat_show_fpga_ver(dummy);

    printf("\n=================================================\n");
    printf("============== Show FPGA Board Type =============\n");
    printf("=================================================\n");
    plat_show_fpga_board_type_reg(dummy);

    printf("\n=================================================\n");
    printf("========= Show FPGA SKU feature register ========\n");
    printf("=================================================\n");
    plat_show_fpga_sku_feature_reg(dummy);

    printf("\n=================================================\n");
    printf("========== Show Pluggable FPGA Version ==========\n");
    printf("=================================================\n");
    show_plug_fpga_ver(dummy);

    printf("%s", banner_string);
    
    return (rc);
}

/*-------------------------------------------------------------------
 * Function: diag_io_interface_test
 * Description : io interface tests.
 * Inputs: N/A
 * Output: PASSED/FAILED
 *------------------------------------------------------------------*/
static int diag_io_interface_test (void)
{
    int rc = FAILED;
    char *tname = "I/O interface";

    dev_98dxc25x_object_t *esw_98dxc25x_obj_p = NULL;
    dev_object_t *dev;

    testname(tname);
    prpass(testpass, "%s, ", tname);
   
    /*POE*/
    printf("\n============================================\n");
    printf("========== Checking POE interface ==========\n");
    printf("============================================\n");
    if (plat_fpga_check_dev_present(FPGA_CPP_POE_PRESENT) == TRUE) {
        if (diag_psu_reg_test(0) == PASSED) {
            rc = PASSED;
            printf("\nPOE interface test PASSED\n");
            prpass(testpass, "%s POE detected.", tname);
        } else {
            cterr('w', 0, "POE is not detected.");
            return (FAILED);
        }
    } else {
        rc = PASSED;
        prpass(testpass, "%s POE not present.", tname);
    }

    /*WiFi*/
    printf("\n\n=============================================\n");
    printf("========== Checking WiFi interface ==========\n");
    printf("=============================================\n");
    /* Create 98dxc25x device driver */
    esw_98dxc25x_obj_p = (dev_98dxc25x_object_t *)diag_get_esw_98dxc25x_obj(); 
    if (esw_98dxc25x_obj_p == NULL) {
        cterr('f', 0, "%s: Creating object failed", __func__);
        return (FAILED);
    }
    dev = (dev_object_t *)esw_98dxc25x_obj_p;

    /* Call 98dxc25x config port pve function 
     * AC5 CPU port is 26
     * AC5 Wifi port is 24*/
    if (esw_98dxc25x_obj_p->callin_fvt->esw_config_port_pve(dev, esw_98dxc25x_obj_p->cpss_dev,
                                                            GE_XCAT5_PORT, XCAT5_TO_WIFI_PORT) != PASSED) {
        cterr('f',0,"Failed to configure PVE for port %d", XCAT5_TO_WIFI_PORT);
	    return (FAILED);
    }

    if (diag_wifi_module_bootup_test() == PASSED) {
        rc = PASSED;
        printf("\nWiFi interface test PASSED\n");
        prpass(testpass, "%s WiFi detected.", tname);
    } else {
        cterr('w', 0, "WiFi is not detected.");
        return (FAILED);
    }

    if (esw_98dxc25x_obj_p->callin_fvt->esw_unconfig_port_pve(dev, esw_98dxc25x_obj_p->cpss_dev,
                                                              GE_XCAT5_PORT, XCAT5_TO_WIFI_PORT) != PASSED) {
        cterr('f',0,"Failed to unconfigure PVE for port %d", XCAT5_TO_WIFI_PORT);
	    return (FAILED);
    }

    /*Pluggable*/
    printf("\n\n==================================================\n");
    printf("========== Checking Pluggable interface ==========\n");
    printf("==================================================\n");
    if (platform_has_pluggable()) {
        rc = plug_intf_test(PLUG_SLOT_1);
        if (rc == PASSED){
            printf("\nPluggable interface test PASSED\n");
            prpass(testpass, "%s Pluggable interface detected.", tname);
        } else {
            cterr('w', 0, "Pluggable interface is not detected.");
            return (FAILED);
        }
    } else {
        rc = PASSED;
        prpass(testpass, "%s Not pluggable SKU.", tname);
    } 
    fflush(0);   
    
    prcomplete(testpass, errcount, (char *)0);
    return (rc);
}

/*-------------------------------------------------------------------
 * Function: diag_plug_test
 * Description : used for pluggable module test 
 * Inputs: PLUG_SLOT_1/PLUG_SLOT_2/... 
 * Output: PASSED/FAILED
 *------------------------------------------------------------------*/
static int diag_plug_test(plug_slot_no slot)
{
    return (plug_test(slot));
}

/*-------------------------------------------------
 * $Log: diag.c,v $
 * Revision 1.2  2021/09/24 01:21:05  harrchan
 * Collapse Elixir-branch to Main Trunk.
 *
 * Revision 1.1.2.8  2021/05/26 04:05:05  harrchan
 * Display FPGA board type and FPGA sku feature in system information utility
 *
 * Revision 1.1.2.7  2021/04/23 02:36:06  illiu
 * Use variable cpss_dev which is a member of 98dxc25x object, instead of using local variable
 *
 * Revision 1.1.2.6  2021/04/12 08:43:30  illiu
 * Replace object-create method as object-get method (Device driver object)
 *
 * Revision 1.1.2.5  2020/12/22 09:33:45  illiu
 * Fix I/O Interface tests item
 *
 * Revision 1.1.2.4  2020/10/26 07:08:28  harrchan
 * 1.Changed PID table in platform_i2c.c
 * 2.Modify menu item to match up Elixir hardware design.
 *
 * Revision 1.1.2.3  2020/09/16 02:25:35  harrchan
 * Support GE1 SFP test
 *
 * Revision 1.1.2.2  2020/09/14 05:49:44  harrchan
 * Remove DSL and GSHDSL relevant part
 *
 * Revision 1.1.2.1  2020/09/09 09:08:05  illiu
 * First version which has been ported with Dreamliner and Marvel CPSS
 *
 * Revision 1.2  2019/01/10 06:36:21  wilbhuan
 * The beginning of Betelgeuse application code.
 *
 *-------------------------------------------------
 */
