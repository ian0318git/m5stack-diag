/* $Id: skye_main.c,v 1.2 2015/05/25 03:59:16 steja Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/skye_linux/src/skye_main.c,v $
 *------------------------------------------------------------------
 * 
 * skye_main.c: Skye SM side start-up code.
 *
 * April 17, 2013 - palin2 ported from Overlord.
 *
 * Jul 2014 - Paul Lin(palin2) ported from Shrinkray.
 *
 * Copyright (c) 2013-2015 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <assert.h>
#include <errno.h>
#include "common.h"
#include "menu.h"
#include "proto.h"
#include "common_utils.h"
#include "diag_tlk10232_test.h"
#include "diag_mv1514_test.h"
#include "skye_eth.h"
#include "skye_main.h"
#include "skye_i2c.h"
#include "router_if.h"
#include "sgmii_defs.h"
#include "linux_api.h"
#include "ethernet.h"
#include "error.h"
#include "nvmonvars.h"
#include "skye_comm_lib.h"
#include "platform_fru.h"

/*
 *  Globals  
 */
boolean nc_mode = FALSE;

static type_t one   = 1;
static long   zero  = 0;

void diag_nc_dispatch_comm(void);
static void skye_help (void);
extern long skye_diag_do_all(char *);
extern void skye_is_ready(void);
extern int tlk10232_ge_host_lpbk_setup(boolean);
extern int cpu_host_1g_rx_tx_packet_util(void);
extern int cpu_host_10g_rx_tx_packet_util(void);
extern int is_host_xgbe2_up(boolean);
extern int disable_clk_buf(void);
extern int tlk_init_config_10gkr_for_host_lbpk(boolean);
extern int util_prot_skye_fpga_gld(int);

#ifdef SKYE_ENHANCED_ERR_MSG
/* Definitions of FRU PID(*_pid) */
uchar skye_pid[]      = "Skye-NGSM";

/* Definitions of FRU LOC(*_loc) */
uchar bp_xaui_loc[]   = "Backplane XAUI";
uchar bp_ge0_loc[]    = "Backplane GE0";
uchar bp_ge1_loc[]    = "Backplane GE1";
uchar spirom_loc[]    = "SPIROM";
uchar i2c_loc[]       = "I2C";
uchar fpga_loc[]      = "FPGA";
uchar dimm_loc[]      = "DIMM";
uchar tlk_loc[]       = "TLK10232";
uchar mv1514_loc[]    = "MVL 88E1514";

fru_table_t platform_fru_table[] = {
    { skye_pid,   dimm_loc },
    { skye_pid,   fpga_loc },
    { skye_pid,   spirom_loc },
    { skye_pid,   tlk_loc },
    { skye_pid,   bp_xaui_loc },
    { skye_pid,   bp_ge0_loc },
    { skye_pid,   bp_ge1_loc },
    { skye_pid,   i2c_loc },
    { skye_pid,   mv1514_loc },
};

unsigned int fru_table_offset;

#endif   /* SKYE_ENHANCED_ERR_MSG */

/* Externs */
extern boolean cpu_id;
extern char *banner_string;

extern int build_dimm_util_menu(int);
extern int  util_set_volt_margin(void);
extern void fpga_szalinski_diag(int);
extern int  show_skye_i2c_mux_status(int);
extern int  skye_i2c_mux_setup(int);
extern int  usb_slot_tests(int);
extern int skye_bib_rd_util(void);
extern int skye_bib_dump_util(void);
extern int skye_bib_change_mac_util(void);
extern int eusb_slot_tests(int);
extern int cpu0_bp_xaui_rx_packet_util(void);
extern int build_thermal_util_menu(int);
extern int build_current_util_menu(int);
extern int build_clock_buf_util_menu(int);
extern int build_pwr_seq_menu(int);
extern void skye_led_test(int);
extern int  skye_i2c_scan_test(int);
extern int skye_ge_alive_test(int);
extern void diag_report_status_host(char *);
extern void diag_nc_dispatch_comm(void);

/* Function prototype */
extern int linux_memory_tester(int);
extern int  alt_mem(), dis_mem(), fil_mem(), mov_mem(), cmp_mem();
extern int  memtest(), memloop(), addrloop(), find_mem();
extern int tile_use_power(), tile_compare_mem();
extern int cpu1_c2c_send(), cpu0_c2c_receive();
extern int cpu0_send_packet_util();
extern int cpu1_send_packet_util();
extern int cpu0_ge_bp_lp_test();
extern int cpu0_bp_rx_packet_util();
extern void clrerrlog();
extern int dumperrlog();
extern int skye_check_pcie_lanes(void);
extern int gpio_init();
extern int cpu0_bp_pse2_rx_packet_util();
extern int cmd_respond();
extern boolean is_cpu0();
extern int cpu0_xaui_bp_lp_test(void);
extern boolean check_cpu(int);
extern int cpu0_pse2_lp_test(void);
extern int sys_init(void);
extern int skye_spirom_util(void);
extern int spirom_test(void);
extern int build_spi_util_menu(int);

static boolean mask_it_now(void);

#define SR_FIRST_EDVT  1

/*
 * Memory debug utility
 */
static struct mitem mem_debug_items[] = {
    {"alter memory",                0,      0,                  (PFT)alt_mem,
     &one,                          0,      (type_t(*)())0,     0},
    {"compare memory block",   	    0,      0,                  (PFT)cmp_mem,
     &one,                          0,      (type_t(*)())0,     0},
    {"display memory",              0,      0,                  (PFT)dis_mem,
     &one,                          0,      (type_t(*)())0,     0},
    {"move memory block",           0,      0,                  (PFT)mov_mem,
     &one,                          0,      (type_t(*)())0,     0},
    {"fill memory",    	            0,      0,                  (PFT)fil_mem,
     &one,                          0,      (type_t(*)())0,     0},
    {"find memory",    	            0,      0,                  (PFT)memtest,
     &one,                          0,      (type_t(*)())0,     0},
    {"memory read or write loop",   0,      0,                  (PFT)memloop,
     &one,                          0,      (type_t(*)())0,     0},
    {"memory debug loop",           0,      0,                  (PFT)memdebug,
     &one,                          0,      (type_t(*)())0,     0},
    {"address loop",                0,      0,                  (PFT)addrloop,
     &one,                          0,      (type_t(*)())0,     0},
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
 * Basic utilities
 */
static struct mitem utilmenuitems[] = {
    {"Memory debug utilities",             0,                           0,
     (PFT)menu,                            (type_t *)&mem_debug_menup,  0,
     (type_t(*)())0,                       0},
    {"Tilera CPU Stress Test",             0,                           0,
     (PFT)tile_use_power,                  (type_t *)&zero,             0,
     (type_t(*)())0,                       0},
    {"Tilera memcpy then memcmp Test",     0,                           0,
     (PFT)tile_compare_mem,                (type_t *)&zero,             0,
     (type_t(*)())0,                       0},
    {"Voltage Margin utility",             0,                           0,
     (PFT)util_set_volt_margin,            (type_t *)&zero,             0,
     (type_t(*)())check_cpu,               0},
    {"Power Sequencer utility",            0,                           0,
     (PFT)build_pwr_seq_menu,              (type_t *)&zero,             0,
     (type_t(*)())check_cpu,               0},
    {"CPU CH0 DIMM utilities",             0,                           0,
     (PFT)build_dimm_util_menu,            (type_t *)&zero,             0,
     (type_t(*)())0,                       0},
    {"CPU CH1 DIMM utilities",             0,                           0,
     (PFT)build_dimm_util_menu,            (type_t *)&one,              0,
     (type_t(*)())0,                       0},
    {"I2C read utility",                   0,                           0,
     (PFT)skye_i2c_rd_util,           (type_t *)&zero,             0,
     (type_t(*)())0,                       0},
    {"I2C write utility",                  0,                           0,
     (PFT)skye_i2c_wr_util,           (type_t *)&zero,             0,
     (type_t(*)())0,                       0},
    {"I2C bib dump utility",               0,                           0,
     (PFT)skye_bib_dump_util,              (type_t *)&zero,             0,
     (type_t(*)())0,                       0},
    {"I2C bib MAC read utility",           0,                           0,
     (PFT)skye_bib_rd_util,           (type_t *)&zero,             0,
     (type_t(*)())0,                       0},
    {"I2C bib MAC write utility",          0,                           0,
     (PFT)skye_bib_change_mac_util,   (type_t *)&zero,             0,
     (type_t(*)())0,                       0},
    {"Show Skye I2C Mux setting",     0,                           0,
     (PFT)show_skye_i2c_mux_status,   (type_t *)&zero,             0,
     (type_t(*)())0,                       0},
    {"Enable Skye I2C Mux Channel",   0,                           0,
     (PFT)skye_i2c_mux_setup,         (type_t *)&one,              0,
     (type_t(*)())0,                       0},
    {"Disable Skye I2C Mux Channel",  0,                           0,
     (PFT)skye_i2c_mux_setup,         (type_t *)&zero,             0,
     (type_t(*)())0,                       0},
    {"Dump Error log",                     0,                           0,
     (PFT)dumperrlog,                      (type_t *)&one,              0,
     (type_t(*)())0,                       0},
    {"Clear Error log",                    0,                           0,
     (PFT)clrerrlog,                       (type_t *)&one,              0,
     (type_t(*)())0,                       0},
    {"Set Scaling CPU speed",              0,                           0,
      (PFT)set_cpu_speed,                  (type_t *)&one,              0,
      (type_t(*)())0,                      0},
    {"Skye SPIROM utility",           0,                           0,
     (PFT)skye_spirom_util,           (type_t *)&zero,             0,
     (type_t(*)())0,                       0},
    {"Thermal Sensor utility",             0,                           0,
     (PFT)build_thermal_util_menu,         (type_t *)&zero,             0,
     (type_t(*)())0,                       0},
    {"Current Sensor utility",             0,                           0,
     (PFT)build_current_util_menu,         (type_t *)&zero,             0,
     (type_t(*)())0,                       0},
    {"CPU0 or CPU1",                       0,                           0,
     (PFT)is_cpu0,                         (type_t *)&zero,             0,
     (type_t(*)())0,                       0},
    {"Clock Buffer utility",               0,                           0,
     (PFT)build_clock_buf_util_menu,       (type_t *)&zero,             0,
     (type_t(*)())0,                       0},
    {"Lock Skye FPGA Golden Sectors",      0,                           0,
     (PFT)util_prot_skye_fpga_gld,         (type_t *)&one,              0,
     (type_t(*)())0,                       0},
    {"UnLock Skye FPGA Golden Sectors",    0,                           0,
     (PFT)util_prot_skye_fpga_gld,         (type_t *)&zero,             0,
     (type_t(*)())0,                       0},
    {"CPU0 GE Backplane RX Debug",         0,                           0,
      (PFT)cpu0_bp_rx_packet_util,         (type_t *)&zero,             0,
      (type_t(*)())0,                      0},
    {"CPU0 XAUI Backplane RX Debug",       0,                           0,
      (PFT)cpu0_bp_xaui_rx_packet_util,    (type_t *)&zero,             0,
      (type_t(*)())0,                      0},
#ifdef SHRINKRAY
    {"CPU0 PSE2 Backplane RX Debug",       0,                           0,
      (PFT)cpu0_bp_pse2_rx_packet_util,    (type_t *)&zero,             0,
      (type_t(*)())0,                      0},
    {"CPU0 PSE2 Loopback TX Debug",        0,                           0,
      (PFT)cpu0_pse2_lp_test,              (type_t *)&zero,             0,
      (type_t(*)())0,                      0},
#endif
    {"GiGa Ethernet Alive Test",           0,                           0,
      (PFT)skye_ge_alive_test,             (type_t *)&zero,             0,
      (type_t(*)())0,                      0},
    {"PCIe Lanes Check",                   0,                           0,
      (PFT)skye_check_pcie_lanes,          (type_t *)&zero,             0,
      (type_t(*)())0,                      0},
#ifdef SHRINKRAY
    {"USB R/W Test",                       0,                           0,
      (PFT)usb_slot_tests,                 (type_t *)&one,              0,
      (type_t(*)())0,                      0},
    {"eUSB R/W Test",                      0,                           0,
      (PFT)eusb_slot_tests,                (type_t *)&one,              0,
      (type_t(*)())0,                      0},
#endif
    {"Debug Module 10G Rx->Tx host loopback Test",               0,                           0,
      (PFT)cpu_host_10g_rx_tx_packet_util, (type_t *)&zero,             0,
      (type_t(*)())0,                      0},
    {"Debug Module 1G Rx->Tx host loopback Test",               0,                           0,
      (PFT)cpu_host_1g_rx_tx_packet_util, (type_t *)&zero,             0,
      (type_t(*)())0,                      0},
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


/*=========================================
 * Main menu items
 *=========================================
 */
submenu_xtable_t main_menu_table[] = {
    {"Main memory test ",                (PFT)linux_memory_tester,        FALSE,
     (MF_CONTINUOUS | MF_DOALL),         (type_t(*)())0,                  0,
     (PFT)linux_memory_tester,           TRUE},
    {"TLK 10232 test",                   (PFT)tlk_10232_test,             FALSE,
     (MF_CONTINUOUS | MF_DOALL),         (type_t(*)())check_cpu,          0,
     (type_t(*)())tlk_10232_test,        TRUE},
#ifdef SHRINKRAY
    {"PHY 88E1514 test",                 (PFT)mv1514_test,                FALSE,
     (MF_CONTINUOUS),                    (type_t(*)())0,                  0,
     (type_t(*)())mv1514_test,           TRUE},
#endif
    {"CPU0 GE Backplane Loopback Test",  (PFT)cpu0_ge_bp_lp_test,         FALSE,
     (MF_CONTINUOUS),                    (type_t(*)())check_cpu,          0,
     (PFT)cpu0_ge_bp_lp_test,            TRUE},
    {"I2C Device Scan Test",             (PFT)skye_i2c_scan_test,         FALSE,
     (MF_CONTINUOUS | MF_DOALL),         (type_t(*)())0,                  0,
     (PFT)skye_i2c_scan_test,            TRUE},
    {"Szalinski FPGA Test",              (PFT)fpga_szalinski_diag,        FALSE,
     (MF_CONTINUOUS | MF_DOALL),         (type_t(*)())0,                  0,
     (PFT)fpga_szalinski_diag,           TRUE},
    {"SPIROM R/W Test",                  (PFT)spirom_test,                TRUE,
     (MF_CONTINUOUS | MF_DOALL),         (type_t(*)())0,                  0,
     (PFT)build_spi_util_menu,           TRUE},
    {"PCIe lanes Scan Test",             (PFT)skye_check_pcie_lanes,      FALSE,
     (MF_CONTINUOUS | MF_DOALL),         (type_t(*)())check_cpu,          1,
     (PFT)skye_check_pcie_lanes,         TRUE},
    {"CPU 1 PCIe Data Transfer Test",    (PFT)cpu1_c2c_send,              FALSE,
     (MF_CONTINUOUS),                    (type_t(*)())mask_it_now,        1,
     (PFT)cpu1_c2c_send,                 TRUE},
    {"CPU 0 PCIe Data Transfer Test",    (PFT)cpu0_c2c_receive,           FALSE,
     (MF_CONTINUOUS),                    (type_t(*)())mask_it_now,        0,
     (PFT)cpu0_c2c_receive,              TRUE},
    {"Dual CPU0 XAUI Loopback Test",     (PFT)cpu0_send_packet_util,      FALSE,
     (MF_CONTINUOUS),                    (type_t(*)())check_cpu,          0,
     (PFT)cpu0_send_packet_util,         TRUE},
    {"Dual CPU1 XAUI Loopback Test",     (PFT)cpu1_send_packet_util,      FALSE,
     (MF_CONTINUOUS),                    (type_t(*)())check_cpu,          1,
     (PFT)cpu1_send_packet_util,         TRUE},
};

#define MAIN_MENU_TABLE_SIZE \
        (sizeof(main_menu_table) / sizeof(submenu_xtable_t))

/*
 * Primary & secondary submenu items (filled in from xtable)
 */
static mitem_t main_menu_primary_items[MAIN_MENU_TABLE_SIZE + MAX_BASE_ITEMS];
static mitem_t main_menu_secondary_items[MAIN_MENU_TABLE_SIZE + MAX_BASE_ITEMS];

static struct menuinfo maindiag = {
    "%s",			/* title */
    0,				/* title string added by init_empty_menu */
    (PFT)menu_show_dflags,	/* shows major flags */
    0,				/* generic prompt */
    0,				/* size -- bumped by add_menu_item() */
    main_menu_primary_items,
};
static struct menuinfo *maindiagp = &maindiag;

/*****************************************************************************
 *
 * Function   : skye_menu
 * Description: skye main menu
 * Inputs     : none
 * Outputs    : none
 *
 *****************************************************************************/
void
skye_menu (void) 
{
    char m_title[32];

    memset(m_title, 0, sizeof(m_title));

    if (check_cpu(0) == TRUE) {
        /* CPU0 */
        sprintf(m_title, "%s Main %s", cpu0str, dgmenustr);
    } else {
        /* CPU1 */
        sprintf(m_title, "%s Main %s", cpu1str, dgmenustr);
    }

    build_primary_submenu(main_menu_table, MAIN_MENU_TABLE_SIZE, m_title,
                          &maindiagp);
    build_secondary_submenu(main_menu_table, MAIN_MENU_TABLE_SIZE,
                            main_menu_secondary_items);

    menu(&maindiag, main_menu_secondary_items, 0);
}

/*****************************************************************************
 *
 * Function   : main
 * Description: entry point of Skye
 * Inputs     : argc, number of argument
 *              argv, command line arguments
 * Outputs    : exit status
 *
 *****************************************************************************/
int
main (int argc, char *argv[])
{
    int opt;
    int is_do_all          = FALSE;
    int is_menu_mode       = FALSE;
    int is_display_ver     = FALSE;
    boolean is_set_tlk10232_lpbk = FALSE;
    boolean is_clr_tlk10232_lbpk = FALSE;
    boolean is_ping_cpu1_to_cpu0 = FALSE;
    boolean is_ping_host_to_cpu0 = FALSE;
    char *check_output = SKYE_OUTPUT_PING_FILES;
    char *check_tlk_set_bit = SKYE_SET_TLK_BIT_FILES;
    char *check_tlk_clr_bit = SKYE_CLR_TLK_BIT_FILES;
    char *check_xg_config = SKYE_SET_XG_CFG_FILES;
    FILE *fp;
    char cmdbuf[128];

    /* Initialize a GPIO context */
    if (gpio_init()) {
        cterr('f', 0, "Unable to configure GPIO");
        return (FAILED);
    }

    /* Initialize the system environment */
    if (sys_init()) {
        cterr('f', 0, "System initial failed");
        return (FAILED);
    }

    /* Let host know, skye is ready */
    skye_is_ready();
    /*
     * Disable Clock buffer
     */
    disable_clk_buf();
    if (argc > 1) {
        for (;;) {
            opt = getopt(argc, argv, "havejklmno");
            if (opt == EOF) {
                break;
            }

            switch (opt) {
            case 'h':  /* Help */
                skye_help();
                exit(1);
                break;
            case 'a':  /* Do all */
                is_do_all = TRUE;
                break;
            case 'v':  /* Display version */
                is_display_ver = TRUE;
                break;
            case 'e':  /* NC Dispatch command */
                nc_mode = TRUE;
                diag_nc_dispatch_comm();
                nc_mode = FALSE;
                exit (0);
                break;
            case 'j':  /* Test Dual CPU 0 XAUI */
                cpu0_send_packet_util();
                exit(0);
                break;
            case 'k':  /* Test Dual CPU 1 XAUI */
                cpu1_send_packet_util();
                exit(0);
                break;
            case 'l':  /* Set CPU0 TLK 1G-KX & 10G-KR loopback bit */
                is_set_tlk10232_lpbk = TRUE;
                break;
            case 'm':  /* Disable TLK 1G-KX & 10G-KR loopback bit */
                is_clr_tlk10232_lbpk = TRUE;
                break;
            case 'n': /* Ping test from CPU 1 to Host BP switch through CPU 0 XAUI */
                is_ping_cpu1_to_cpu0 = TRUE;
                break;
            case 'o': /* Ping test from Host BP 10G-KR through CPU 0  */
                is_ping_host_to_cpu0 = TRUE;
                break;
            }
        }
    } else {
        is_menu_mode   = TRUE;
        is_display_ver = TRUE;
    }

    if (is_display_ver == TRUE) {
        printf(banner_string);
        printf("\n");
    }

    if (is_menu_mode == TRUE) {
        skye_menu(); /* goto menu directly; */
    } else {
        if (is_do_all == TRUE) {
            /* Do all tests from here */
            skye_diag_do_all(0);
        }
    }

    if (is_set_tlk10232_lpbk == TRUE) {
        sprintf(cmdbuf, "rm %s", check_tlk_set_bit);
        system(cmdbuf);

        fp = fopen(check_tlk_set_bit, "w");
        if (fp == NULL) {
            printf("fail to open files :%s", check_tlk_set_bit);
            exit(1);
        }
        printf("\nSet loopback for TLK (1G-KX/10G-KR)\n");
       if (tlk10232_ge_host_lpbk_setup(TRUE) != PASSED) {
           printf("fail\n");
           fprintf(fp, "%s", "fail");
       } else {
           printf("pass\n");
           fprintf(fp, "%s", "pass");
       }
       fclose(fp);
    }

    if (is_clr_tlk10232_lbpk == TRUE) {
        sprintf(cmdbuf, "rm %s", check_tlk_clr_bit);
        system(cmdbuf);

        fp = fopen(check_tlk_clr_bit, "w");
        if (fp == NULL) {
            printf("fail to open files :%s", check_tlk_clr_bit);
            exit(1);
        }
        printf("\nDisable loopback for TLK (1G-KX/10G-KR)\n");
        if (tlk10232_ge_host_lpbk_setup(FALSE) != PASSED) {
            printf("fail\n");
            fprintf(fp, "%s", "fail");
        } else {
            printf("pass\n");
            fprintf(fp, "%s", "pass");
        }
        fclose(fp);
    }

    if (is_ping_cpu1_to_cpu0 == TRUE) {
        sprintf(cmdbuf, "rm %s", check_output);
        system(cmdbuf);

        fp = fopen(check_output, "w");
        if (fp == NULL) {
            printf("fail to open files :%s", check_output);
            exit(1);
        }
        printf("\nPing CPU 1 to Host BP in progress ...\n");
        /* this function to test xgbe1 between CPU0 and CPU1 through Host BP switch (xgbe2) */
        if (is_host_xgbe2_up(TRUE) != PASSED) {
            printf("fail\n");
            fprintf(fp, "%s", "fail");
        } else {
            printf("pass\n");
            fprintf(fp, "%s", "pass");
        }
        fclose(fp);
    }

    if (is_ping_host_to_cpu0 == TRUE) {
        sprintf(cmdbuf, "rm %s", check_xg_config);
        system(cmdbuf);

        fp = fopen(check_xg_config, "w");
        if (fp == NULL) {
            printf("fail to open files :%s", check_xg_config);
            exit(1);
        }

        printf("\nSetup 10G-KR script in progress ...\n");
        /* Setup 10G-KR script */
        if (tlk_init_config_10gkr_for_host_lbpk(FALSE) != PASSED) {
            printf("fail\n");
            fprintf(fp, "%s", "fail");
        } else {
            printf("pass\n");
            fprintf(fp, "%s", "pass");
        }
        fclose(fp);
    }
    return (PASSED);
}

/*******************************************************************************
 *
 * Function   : mask_it_now
 * Description: Function used to not invoke un-supported test in menu.
 *              This is because that we won't support all tests for Skye
 *              first EDVT.
 * Inputs     : None
 * Outputs    : TRUE/FALSE
 *
 *******************************************************************************
 */
static boolean mask_it_now (void)
{
    return (FALSE);
}

/**************************************************************************
 *
 * Function: skye_help
 *
 * Display help
 *
 * Input: None
 *
 * Return: None
 *
 * *************************************************************************
 */
static void skye_help (void)
{
    printf("Usage: skyenet [-a] [-h] [-i] [-v]\n\n");

    printf("Options:\n");
    printf("-a Do all the tests\n");
    printf("-h Display this help\n");
    printf("-v Display version\n");
    printf("-e NC Dispatch command\n");
    printf("\n");
}


/*-------------------------------------------------
$Log: skye_main.c,v $
Revision 1.2  2015/05/25 03:59:16  steja
Add Support Skye SM

Revision 1.1.4.5  2015/05/23 19:06:25  palin2
Add utilites to lock/unlock Skye FPGA SPI flash.

Revision 1.1.4.4  2015/05/05 11:53:12  steja
CDETS[CSCuu01237] Solving TLK intermittent loopback issue on GH platform.

Revision 1.1.4.3  2015/04/29 13:30:38  steja
Update TLK 10G-KR test path

Revision 1.1.4.2  2015/04/29 11:36:36  steja
Code check-in to skye-branch2 for ER code review


------------------------------------------------------------
Revision 1.1.2.31  2015/04/22 12:53:59  steja
To disable clock generator ch4 to ch7 to increase performance on the clock.

Revision 1.1.2.30  2015/03/30 06:36:25  steja
Remove unused utilities and update test name

Revision 1.1.2.29  2015/03/26 08:33:40  steja
Debug edvt found issue on 2CPU skye Dual CPU Xaui Test

Revision 1.1.2.28  2015/02/13 08:32:27  steja
Remove unused test to utility and mask MV1514 not use on skye

Revision 1.1.2.27  2015/02/13 05:34:11  palin2
Removed CPU1 unnecessary utilities.

Revision 1.1.2.26  2015/01/22 09:07:47  palin2
Moved up I2C scan test ordering in Skye Diag main tests.

Revision 1.1.2.25  2014/12/31 09:25:58  steja
Remove debug info and remove unused code

Revision 1.1.2.24  2014/11/27 09:46:38  steja
Update UART timeout and skyelnx string

Revision 1.1.2.23  2014/11/27 09:19:30  palin2
Added utility to dump all BIB value in raw.

Revision 1.1.2.22  2014/11/27 07:25:20  palin2
1. Fixed PCIe lanes Scan test.
2. Added PCIe lanes Scan test to 2-CPUs Skye default tests.
3. Added SKYE_P1A compile flag to tell difference between P1A and P1B.

Revision 1.1.2.21  2014/11/10 09:42:45  steja
Update TLK10232 10G KR loopback setup

Revision 1.1.2.20  2014/10/07 06:05:03  palin2
Moved NC command module side related function from skye_main.c to skye_util.c

Revision 1.1.2.19  2014/09/26 09:05:53  steja
(CSCuq98591)Fix GBE4 link issue

Revision 1.1.2.18  2014/09/23 07:03:14  steja
Update code for checking Primary Interface Ready (GPIO3)

Revision 1.1.2.17  2014/09/18 07:18:43  steja
1.Update NC command codei
2.Update enhanced error message

Revision 1.1.2.16  2014/09/17 11:11:42  palin2
Updated enhanced error message PID & LOC definitions.

Revision 1.1.2.15  2014/09/17 04:35:07  palin2
Updated Skye enhanced error message.

Revision 1.1.2.14  2014/09/15 15:11:16  palin2
Updated Skye module side main tests table.

Revision 1.1.2.13  2014/09/12 14:38:42  steja
Update code for CPU do all test

Revision 1.1.2.12  2014/09/04 22:02:56  palin2
Removed "USB test", "eUSB test", SM side "LED test" from default test
based on HW's comment in Skye P1A bringup plan review meeting.

Revision 1.1.2.11  2014/09/02 13:09:49  steja
Update Enhance error code for 88E1514

Revision 1.1.2.9  2014/08/31 23:00:21  palin2
Updated Skye enhanced error message FRU table.

Revision 1.1.2.8  2014/08/29 04:55:00  steja
Fix segmentation fault access nc table

Revision 1.1.2.7  2014/08/28 02:54:26  steja
Support Do all test for NC command

Revision 1.1.2.6  2014/08/25 11:55:49  steja
Update Code for BST Testing

Revision 1.1.2.5  2014/08/22 04:58:54  palin2
First check-in to enhance Skye error message.

Revision 1.1.2.4  2014/08/21 02:22:35  palin2
Support passing Flags setup from host side by NC command in Skye.

Revision 1.1.2.3  2014/08/15 03:27:41  palin2
Initial check-in to support NC command on Skye.

Revision 1.1.2.2  2014/08/08 08:34:33  steja
Add Do all test

Revision 1.1.2.1  2014/07/21 01:56:56  palin2
Initial check-in Skye module side Diag code.

---------------------------------------------------
skye_main.c:
Revision 1.2.8.11  2014/07/14 08:05:33  iachang
Support GE alive test

Revision 1.2.8.10  2014/07/09 02:21:09  palin2
Support I2C scan test for Shrinkray.

Revision 1.2.8.9  2014/06/06 11:54:21  steja
Add Shrinkray LED Test

Revision 1.2.8.8  2014/06/05 05:41:23  palin2
Add to display CPU ID in module side Main menu banner.

Revision 1.2.8.7  2014/06/04 09:50:07  palin2
Removed USB R/W test from CPU1 because Shrinkray CPU1 USB port is
always set to end-point mode.

Revision 1.2.8.6  2014/06/04 09:42:39  palin2
Add banner for Shrinkray/Skye version control.

Revision 1.2.8.5  2014/06/04 02:14:07  palin2
Add CPU1 SPI ROM R/W test support.

Revision 1.2.8.4  2014/05/27 15:28:04  steja
Update menu for CPU1

Revision 1.2.8.3  2014/05/25 16:44:10  iachang
Add Dual CPU XAUI Loopback Test
Add Dual CPU PCIe Data Transfer Test

Revision 1.2.8.2  2014/05/20 17:55:59  palin2
Add power sequencer utility entry point.

Revision 1.2.8.1  2014/05/09 03:11:18  palin2
Temporarily masked out those ShrinkRay First EDVT unsupported items from menu.

Revision 1.2  2014/02/27 15:01:48  palin2
Initial check-in ShrinkRay SM side Diag code.

Revision 1.1.4.24  2014/02/18 09:19:03  steja
Add print refer CDETS info for pci lane check and remove continue and do all flag

Revision 1.1.4.23  2014/02/13 12:35:26  palin2
Format menu tables based on review comment.

Revision 1.1.4.22  2014/02/07 18:49:06  steja
Update menu

Revision 1.1.4.20  2014/01/27 16:52:46  iachang
Support SPI ROM read/write Test

Revision 1.1.4.19  2014/01/13 17:19:41  steja
Add Clock Buffer utility Read and Write to Basic Utilities

Revision 1.1.4.18  2014/01/08 03:42:12  iachang
Moved CPU number item to basic utilities

Revision 1.1.4.17  2013/12/18 05:03:11  steja
1. support PSE2 backplane loopback test
2. support BIB change MAC address utility

Revision 1.1.4.16  2013/12/16 08:34:35  iachang
Support current sensor
Modify on-board thermal sensor

Revision 1.1.4.15  2013/12/06 09:39:44  iachang
Move DIMM Thermal sensor to skye_thermal.c
Support on-board Thermal sensor
Convert the measure to actual temperature

Revision 1.1.4.14  2013/11/29 07:08:55  steja
1. Fix the full data path TLK working.
2. add USB test
3. add read BIB MAC utility

Revision 1.1.4.13  2013/11/22 09:16:52  iachang
Support Shrinkray SPIROM utility.

Revision 1.1.4.12  2013/11/20 00:28:18  iachang
Initialize the system environment

Revision 1.1.4.11  2013/11/19 14:36:47  steja
Provide TLK utility for debugging
Update the BTK TLK into coded

Revision 1.1.4.10  2013/11/18 03:28:14  iachang
Support CPU0 Szalinski watchdog test.

Revision 1.1.4.9  2013/11/07 06:46:39  iachang
Support Szalinski interrupt test.

Revision 1.1.4.8  2013/11/05 09:17:54  steja
1. Fix the MDIO not stable issue
2. debug tlk log

Revision 1.1.4.7  2013/10/30 10:49:20  iachang
Modify CPU GPIO initial

Revision 1.1.4.6  2013/10/10 00:36:22  steja
1. Add TLK Utility PLL and Polarity TX RX switch
2. Code update

Revision 1.1.4.5  2013/10/09 03:02:00  palin2
Add USB test for ShrinkRay.

Revision 1.1.4.4  2013/10/07 21:35:18  palin2
Add ShrinkRay I2C Mux related access function and utility support.

Revision 1.1.4.3  2013/09/29 04:03:32  iachang
CPU0 GE Backplane RX Debug utility
Support 88E1514 initial function
Support 88E1514 Power Enable/Disable function

Revision 1.1.4.2  2013/09/13 07:00:09  palin2
Initial check-in ShrinkRay SM side Diag code.

Revision 1.1.2.21  2013/09/05 08:07:44  steja
Support Set CPU Frequency margin utilities

Revision 1.1.2.20  2013/08/27 02:57:20  palin2
Add ShinkRay I2C access uilities wrap.

Revision 1.1.2.19  2013/08/19 07:11:52  palin2
Add Voltage Margin utility.

Revision 1.1.2.18  2013/08/15 11:30:33  steja
Add code command and respond ( Host <->GE <-> TILE CPU#0) for G2 (PPC & MIPS) platform

Revision 1.1.2.17  2013/07/30 11:52:44  palin2
Add "FPGA(Szalinski) Test" item into ShrinkRay SM side main tests.

Revision 1.1.2.16  2013/07/30 08:17:59  palin2
Add "Frequnency Margin utility".

Revision 1.1.2.15  2013/07/26 10:14:44  iachang
Add Watch Doag Test
Support CPU GPIO initial and CPU ID detect

Revision 1.1.2.14  2013/07/16 09:44:55  steja
1. Add utillity for dump error log and clear error log
2. Add cterr on use_power function

Revision 1.1.2.13  2013/07/15 22:10:41  palin2
Removed empty function, "skye_test".
We used this function for Menu structure verification before,
but we don't need it anymore now.

Revision 1.1.2.12  2013/07/15 09:05:42  steja
Add menu for CPU0 GE backplane loopback test

Revision 1.1.2.11  2013/07/11 15:55:47  iachang
Support PCIe Lanes Check Test

Revision 1.1.2.10  2013/07/09 07:24:25  palin2
Create "skye_i2c_api.c" for ShrinkRay I2C APIs,
and move related I2C read/write function to it.

Revision 1.1.2.9  2013/07/04 12:16:44  iachang
Support Dual CPU XAUI interface loopback test

Revision 1.1.2.8  2013/07/02 03:14:24  palin2
Add support to dump DDR DIMM SPD info.

Revision 1.1.2.7  2013/07/01 04:06:22  steja
Modify Tilera tools use_power to support memory compare

Revision 1.1.2.6  2013/06/28 09:45:41  iachang
Support Dual CPU PCIe interface data transfer test

Revision 1.1.2.5  2013/06/24 09:03:34  steja
Checkin :
- Support TLK10323 Loopback test & Utility
- Support MV1514 Loopback test

Revision 1.1.2.4  2013/06/20 03:05:31  iachang
Support Tilera CPU Stress Test : use_power

Revision 1.1.2.3  2013/05/09 04:19:13  iachang
Support memory debug utilities

Revision 1.1.2.2  2013/04/29 08:25:08  iachang
Support memory test

Revision 1.1.2.1  2013/04/24 01:30:01  palin2
ShrinkRay Diag initial check-in.

---------------------------------------------------
$Endlog$
*/
