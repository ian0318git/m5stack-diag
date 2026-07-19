/* $Id: diag.c,v 1.2 2021/04/15 00:52:23 achiu2 Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/phoenix/diag.c,v $
 *------------------------------------------------------------------
 *
 * diag.c - Phoenix diagmon main menu and supporting wrappers.
 *
 *
 * Copyright (c) 2018-2019 by Cisco Systems, Inc.
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
#include "diag_fpga_upgrade.h"
#include "diag_hdd_test.h"
#include "diag_m2_test.h"
#include "diag_rtc_test.h"
#include "diag_eth_pkt_txrx.h"
#include "diag_eth_pkt_txrx_api.h"
#include "platform_psu.h"
#include "cookie_4.h"
#include "platform_pwr_seq.h"
#include "diag_i350_test.h"


/*
 * Declare external function
 */
extern int ExecuteCmdbyPopen(char *cmd, char *retBuf, int sizeOfBuf);
extern int show_emmc_info(void);
extern int emmc_pslc_fully_enable(int);
extern int emmc_full_test(int);
extern int phoenix_show_fpga_ver(int);
extern int phoenix_show_sku_dbx_info(void);
extern char *banner_string;
extern int phoenix_cpu_ondie_temp(int opt);
extern int diag_temp_sensor_show_temp(void);
extern int diag_full_load_util(void);
extern void build_fan_menu(void);
extern boolean menu_display(void);
extern int get_pwr_seq_fw_rev(int);
extern void phoenix_show_temp_info(void);
extern int show_barometer_info(void);
extern void show_fan_sts(void);
extern void phoenix_show_nios_ver(void);
extern int phoenix_show_aikido_fpga_ver(void);

/*
 * Declare local function
 */

static void diag_sys_info_util(int);
static int diag_ex_feature_util(int);

static int diag_vol_margin_util(int);
static int diag_mb_temper_util(int);
static int io_interface_tests(void);
static int program_mac(int);
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
    
    {"alter NIM 0 cookie", 0, 0,
     (type_t(*)()) alter_nim_cookie, &one, 0, (type_t(*)())0, 0},

    {"alter NIM 1 cookie", 0, 0,
     (type_t(*)()) alter_nim_cookie, &two, 0, (type_t(*)())0, 0},

    {"program host CPU GE MAC",    	          0,	0,
     (type_t(*)())program_mac,	  &zero,	0, (type_t(*)())0, 0},
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

    {"Logic FPGA Utilities", 0, 0,
     (PFT) phoenix_fpga_utils, (type_t *) &one, 0,
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
     (type_t(*)())0, 0},

    {"Show eMMC Info", 0, 0,
     (PFT) show_emmc_info, (type_t *) &zero, 0,
     (type_t(*)())0, 0},

    {"eMMC full test", 0, 0,
     (PFT) emmc_full_test, (type_t *) &zero, 0,
     (type_t(*)())0, 0},

    {"RTC Utility", 0, 0,
     (PFT) build_rtc_utils_menu, (type_t *) &one, 0,
     (type_t(*)())0, 0},
     
    {"SPI Flash Utility", 0, 0,
     (PFT)phoenix_spi_flash_utils, (type_t *)&zero, 0,
     (type_t(*)())0, 0},

    {"FAN Utility", 0, 0,
     (PFT)build_fan_menu, (type_t *)&zero, 0,
     (type_t(*)())0, 0},

    {"Check M2 Device Utility", 0, 0,
     (PFT)check_m2_device_utility, (type_t *)&zero, 0,
     (type_t(*)())has_m2_device, 0},

    {"PSU Utility", 0, 0,
     (PFT)build_psu_util_menu, (type_t *) &zero, 0,
     (type_t(*)())0, 0},

    {"Aikido Utilities", 0, 0,
     (PFT) build_aikido_utils, (type_t *) &zero, 0,
     (type_t(*)())0, 0},

    {"MCU Utilities", 0, 0,
     (PFT) build_pwr_seq_menu, (type_t *) &one, 0,
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

    {"I/O Interface tests",
    (PFT) io_interface_tests, FALSE,
    MF_CONTINUOUS | MF_DOALL | MF_SHOW_ERRCOUNT,
    (type_t(*)())0, 0, (PFT)0, FALSE},

    {"Motherboard tests",
    (PFT) mb_tests, TRUE,
    MF_CONTINUOUS | MF_DOALL | MF_SHOW_ERRCOUNT,
    (type_t(*)())0, 0, (PFT) mb_tests, FALSE},

    {"NIM 0 Test",
    (PFT)wic_test,             FIRST_SLOT,
    MF_CONTINUOUS | MF_DOALL | MF_SHOW_ERRCOUNT,
    (type_t(*)())0, 0, (PFT)wic_test,  FIRST_SLOT + MAX_WIC},

    {"NIM 1 Test",
    (PFT)wic_test,             SECOND_SLOT,
    MF_CONTINUOUS | MF_DOALL | MF_SHOW_ERRCOUNT,
    (type_t(*)())0, 0, (PFT)wic_test,  SECOND_SLOT + MAX_WIC},

    {"Virtual SM 0 Test",                                                                             
    (PFT)sm_test,     FIRST_SLOT,
    MF_CONTINUOUS | MF_DOALL | MF_SHOW_ERRCOUNT,
    (type_t(*)())0, 0,     (PFT)sm_test, FIRST_SLOT + MAX_SM},
    /* Mark for P1B will not include DSP1
    {"Virtual SM 1 Test",                                                                             
    (PFT)sm_test,     SECOND_SLOT,
    MF_CONTINUOUS | MF_DOALL | MF_SHOW_ERRCOUNT,
    (type_t(*)())0, 0,     (PFT)sm_test, SECOND_SLOT + MAX_SM},
    */
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
 * Function: diag_sys_show_fw_ver
 *
 * Description: display all firmware version.
 *
 * Input : None
 *
 * Output: None
 *
 **********************************************************************
 */
void diag_sys_show_fw_ver(void)
{
    phoenix_show_fpga_ver(0);
    printf("\n");

    phoenix_show_aikido_fpga_ver();
    printf("\n");

    phoenix_show_nios_ver();
    printf("\n");

    phoenix_show_i350_ver();
    printf("\n");

    get_pwr_seq_fw_rev(0);
    printf("\n");
}


/**********************************************************************
 *
 * Function: diag_sys_show_env
 *
 * Description: display all environment status.
 *
 * Input : None
 *
 * Output: None
 *
 **********************************************************************
 */
void diag_sys_show_env(void)
{
    phoenix_show_temp_info();
    show_barometer_info();
    printf("\n");
    show_fan_sts();
    printf("\n");
    psu_show_env_info();
    printf("\n");
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
    phoenix_show_cpuinfo();

    phoenix_show_meminfo();

    diag_sys_show_env();

    diag_sys_show_fw_ver();

    phoenix_show_sku_dbx_info();

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
    phoenix_cpu_ondie_temp(0);
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

    prcomplete(testpass, errcount, (char *)0);
    
    return (pim_rc | nim_rc); 
}

/**********************************************************************
 *
 * Function: program_mac
 *
 * Description: entry point to program mac 
 *
 * Inputs: N/A
 *                     
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
static int program_mac (int dummy)
{
    uchar buf[15] = { 0,0,0,0,0,0,0,0,0,0,0,0,0,0 }; /* has to be at least 6 */
    int32_t size = 0;
    unsigned int port = 0;
    char mac_str[13];
    char cmd[128];

    size = get_mac_blk_size(); 
    printf("size is %d\n", size);
    if (size < 4 ) {
        printf("data size = %d. it's too small.\n", size);
        printf("Please run 'alter mb cpu cookie' option at least once.\n");
        return (FAILED);
    }

    /* Phoenix:
     * i350_eth0 is I350 port 0 (mac base + 0).
     * i350_eth1 is I350 port 1 (mac base + 1).
     * eth0 is NIM 0 (mac base + 0x8E).
     * eth1 is NIM 1 (mac base + 0x8D).
     * eth2 is DSP 0 (mac base + 0x8C).
     * eth3 is DSP 1 (mac base + 0x8A).
     * mac base and blocksize is a stored cookie value.
     * (blocksize = 144 = 0x90)
     */
    for (port = 0 ; port <= 5 ; port++) {

        switch (port) {
            case 0:
            case 1:
                get_mac_from_block(port, buf);
                break;
            case 2:
            case 3:
            case 4:
                get_mac_from_block(size-port, buf);
                break;
            case 5:
                get_mac_from_block(size-port-1, buf);
                break;
            default:
                printf("Error: unsupported port number\n");
                return (FAILED);
        }

        if (!((*buf) || *(buf + 1) || *(buf + 2) || *(buf + 3) || *(buf + 4) ||
                (*buf+5))) {
            printf("Error getting mac base addr.\n");
            printf("Please run 'alter mb cpu cookie' and 'display cookie'"
                    " content at least once.\n");
            return (FAILED);
        }
        sprintf(mac_str,"%02x%02x%02x%02x%02x%02x",
                buf[0], buf[1], buf[2], buf[3], buf[4], buf[5]);

        sprintf(cmd, "/opt/tool/intel/eeupdate64e /nic=%d /mac=%s\n",
                port+1, mac_str);
        printf(cmd);
        system(cmd);
        msleep(10);
    } 
    return (PASSED);
}

