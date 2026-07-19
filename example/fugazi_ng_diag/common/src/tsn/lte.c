/* $Id: lte.c,v 1.14 2020/07/10 11:36:50 steja Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/tsn/lte.c,v $
 *------------------------------------------------------------------
 *
 * lte.c
 *
 *
 * Copyright (c) 2017 - 2019 by cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */
#include <string.h>
#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <termios.h>
#include <sys/time.h>
#include <errno.h>
#include "types.h"
#include "common.h"
#include "menu.h"
#include "error.h"
#include "lte.h"
#include "proto.h"
#include "nvmonvars.h" 
#include "setjmps.h" 
#include "platform_fpga.h"
#include "platform_fru.h"
#include "platform_cookie.h"
#include "platform_sensor.h"
#include "plat_defs.h"
#include "mb_tests.h"

/***********************************************************************
 *  Macro Processor Definitions
 ************************************************************************/

/***********************************************************************
 *  Extern Functions Declaration
 ************************************************************************/
extern int tsn_mem_write32(uint, uint);
extern int usb_get_speed(int);
extern int usb_get_info(void);
extern int tsn_sim_stat_led_utils(int);
extern int tsn_gps_stat_led_utils(int);
extern int tsn_rssi_led_utils(int);
extern int usb_debugport_test(int);
extern ushort get_control_type(void);

/***********************************************************************
 * Global variables
 ************************************************************************/
static boolean traffic_mode = FALSE;
/*end of testing */
char tty_dev_0[LENGTH32];
char tty_dev_1[LENGTH32];
char atcmd[LENGTH64];
int timeout = COUNT30;
int fd;
struct termios options;
char usb_port[15];

/***********************************************************************
 *  Functions Declaration
 ************************************************************************/
boolean has_lte_modem_reset_test(int opt);
boolean has_lte_usb_2_0_detection_test(int opt);
boolean has_lte_gps(int opt);
static int modem_gen_at_cmd_test(void);
static int run_at_cmd(int);
int usb_lte_test(boolean); 
int lte_get_model_name(char *path);
static int lte_gps_pin_test(int);
static int lte_check_modem_rdy(int input);
static int lte_force_gps_pin_val(int value);
static int dbm_pwr = 0;
static char model_name[32];
int usb_20_detection_test(void);
static int lte_at_selftest(int);
static int supports_em74xx_lte(void);
static int display_em74xx_simdetect_stat_util(int);
static int em74xx_simdetect_pin_test(int, boolean, boolean);
static int lte_em74xx_simdetect_pin_test(int, int);
static int lte_em74xx_simdetect_pin_test_insertion_wrapper(int);
static int lte_em74xx_simdetect_pin_test_removal_wrapper(int);
int lte_utils_entry_fn(void);
static int supports_wp76xx_lte(void);
static int display_wp76xx_simdetect_stat_util(int);
static int wp76xx_simdetect_pin_test(int, boolean, boolean);
static int lte_wp76xx_simdetect_pin_test(int);
int lte_get_tty_num(char *);
static int lte_at_open_tty(int *);
static boolean lte_has_2_sim_slot(void);
static int lte_usb_get_vid_did_speed(char *, int *, int *, int *);
int lte_host_usb_detect(int, int);
int lte_wp_reset_init(void);
int lte_wp_modem_pwr_off_ctrl(void);
int lte_wp_clr_saf_pwr_remv(void);

/* decide the test is automatically or not */ 
static uint donot_query = FALSE; 

static boolean em74xx_simdetect_state = 0;
static boolean wp76xx_simdetect_state = 0;

/*
 * LTE Main Test Menu
 */
static submenu_xtable_t lte_menu_table[] = {
    { "Modem_0 Detection Test",   (PFT) tsn_modem_0_detect_test,    0,
      MM_3,          (type_t(*) ()) 0, 0, (type_t(*) ()) 0, 0 },
    { "Modem_0 Reset Test", (PFT) tsn_modem_0_reset_test,  0,
      MM_3,          (type_t(*) ()) has_lte_modem_reset_test, 0, (type_t(*) ()) 0, 0 },
    {"AT Command Utility",  (PFT)modem_gen_at_cmd_test,     0,
      MM_1, (type_t(*)())0, 0,    (type_t(*)())0, 0},
    { "Modem_0 3G Main RSSI Test", (PFT) tsn_get_lte_0_main_rssi,  0,
      MM_1 | MF_SHOW_ERRCOUNT,      (type_t(*) ()) 0, 0, (type_t(*) ()) 0, 0 },
    { "Modem_0 3G DIV RSSI Test", (PFT) tsn_get_lte_0_div_rssi, 0,
      MM_1 | MF_SHOW_ERRCOUNT,            (type_t(*) ()) 0, 0, (type_t(*) ()) 0, 0 },
    { "Modem_0 GPS Antennae", (PFT) tsn_get_lte_0_gps_antennae_test,   0,
      MM_1 | MF_SHOW_ERRCOUNT,          (type_t(*) ()) has_lte_gps, 0, (type_t(*) ()) 0, 0 },
    { "SIM 0 Card Test", (PFT) tsn_sim_0_test,   0,
      MM_1,          (type_t(*) ()) 0, 0, (type_t(*) ()) 0, 0 },
    { "SIM 1 Card Test", (PFT) tsn_sim_1_test,   0,
      MM_1,          (type_t(*) ()) lte_has_2_sim_slot, 0, (type_t(*) ()) 0, 0 },
    {"Modem mini-USB debug port test", (PFT) usb_lte_test, FALSE, 
      MM_1,         (type_t(*) ()) 0, 0, (type_t(*) ()) 0, 0},
    {"USB 2.0 Detection test", (PFT) usb_20_detection_test, 0, 
      MM_1,         (type_t(*) ()) has_lte_usb_2_0_detection_test, 0, (type_t(*) ()) 0, 0},
    { "External USB 0 (LTE 0) turn ON", (PFT) usb_lte_test, TRUE, 
      MM_1,         (type_t(*) ()) 0, 0, (type_t(*) ()) 0, 0},
    { "External USB 0 (LTE 0) turn OFF", (PFT) usb_lte_test, FALSE, 
      MM_1,         (type_t(*) ()) 0, 0, (type_t(*) ()) 0, 0},
#ifdef GPS_PIN_TEST_W_ANTENNA
    { "GPS Pin Test", (PFT) lte_gps_pin_test,  0,
      MM_1 | MF_SHOW_ERRCOUNT,      (type_t(*) ()) has_lte_gps, 0, (type_t(*) ()) 0, 0 },
#else
    { "GPS Pin Test", (PFT) lte_gps_pin_test,  0,
      MM_3 | MF_SHOW_ERRCOUNT,      (type_t(*) ()) has_lte_gps, 0, (type_t(*) ()) 0, 0 },
#endif
    {"LTE SIM 0 stat LED utils", (type_t(*)())tsn_sim_stat_led_utils, 0,
     0,
      (type_t(*)())0, 0, (type_t(*)())0, 0},
    {"LTE SIM 1 stat LED utils", (type_t(*)())tsn_sim_stat_led_utils, 1,
     0,
      (type_t(*)())lte_has_2_sim_slot, 0, (type_t(*)())0, 0},
    {"LTE GPS stat LED utils", (type_t(*)())tsn_gps_stat_led_utils, TRUE,
     0,
      (type_t(*)()) has_lte_gps, 0, (type_t(*)())0, 0},
    {"LTE RSSI stat LED utils", (type_t(*)())tsn_rssi_led_utils, TRUE,
     0,
      (type_t(*)())0, 0, (type_t(*)())0, 0},
    {"LTE Utilities", (type_t(*)())lte_utils_entry_fn, TRUE,
     0,
     (type_t(*)())0, 0, (type_t(*)())0, 0},
};
#define LTE_MENU_TABLE_SZ \
    (sizeof(lte_menu_table) / sizeof(submenu_xtable_t))

/*
 * primary & secondary submenu items (filled in from xtable)
 */
static mitem_t lte_pri_items[LTE_MENU_TABLE_SZ + MAX_BASE_ITEMS];
static mitem_t lte_sec_items[LTE_MENU_TABLE_SZ + MAX_BASE_ITEMS];

static struct menuinfo lte_menu = {
    "LTE Main Menu",
    0,          /* mtparam added by init_empty_menu */
    (PFT) show_endnote,  /* notes missing WICs in combos */
    0,          /* use generic prompt */
    0,          /* size (bumped by add_menu_item() */
    lte_pri_items,
};
static struct menuinfo *lte_menup = &lte_menu;

/*
 * LTE Utilities Submenu
 */
static submenu_xtable_t lte_utils_tbl[] = {
    {"LTE EM74xx SIM_DETECT_pin Test(SIM0) Insertion",
     (PFT)lte_em74xx_simdetect_pin_test_insertion_wrapper, SIM0,
     (MF_CONTINUOUS | MF_DOGRP | MF_SHOW_ERRCOUNT),
     (type_t(*) ())supports_em74xx_lte,               0,
     (type_t(*) ())0,                                 0},
    {"LTE EM74xx SIM_DETECT_pin Test(SIM0) Removal",
     (PFT)lte_em74xx_simdetect_pin_test_removal_wrapper,   SIM0,
     (MF_CONTINUOUS | MF_DOGRP | MF_SHOW_ERRCOUNT),
     (type_t(*) ())supports_em74xx_lte,               0,
     (type_t(*) ())0,                                 0},
    {"Display LTE EM74xx SIM_DETECT pin state(SIM0)",
     (type_t(*)())display_em74xx_simdetect_stat_util, SIM0,
     0,
     (type_t(*)())supports_em74xx_lte,                0,
     (type_t(*)())0,                                  0},
    {"Display LTE EM74xx SIM_DETECT_2 pin state(SIM1)",
     (type_t(*)())display_em74xx_simdetect_stat_util, SIM1,
     0,
     (type_t(*)())supports_em74xx_lte,                0,
     (type_t(*)())0,                                  0},
    {"LTE WP76xx UIM1_DET/SIM_DETECT pin Test(SIM0)",
     (PFT)lte_wp76xx_simdetect_pin_test,              SIM0,
     (MF_CONTINUOUS | MF_DOGRP | MF_SHOW_ERRCOUNT),
     (type_t(*) ())supports_wp76xx_lte,               0,
     (type_t(*) ())0,                                 0},
    {"LTE WP76xx UIM1_DET/SIM_DETECT pin Test(SIM1)",
     (PFT)lte_wp76xx_simdetect_pin_test,              SIM1,
     (MF_CONTINUOUS | MF_DOGRP | MF_SHOW_ERRCOUNT),
     (type_t(*) ())lte_has_2_sim_slot,               0,
     (type_t(*) ())0,                                 0},
    {"Display LTE WP76xx UIM1_DET/SIM_DETECT pin state(SIM0)",
     (type_t(*)())display_wp76xx_simdetect_stat_util, SIM0,
     0,
     (type_t(*)())supports_wp76xx_lte,                0,
     (type_t(*)())0,                                  0},
    {"Display LTE WP76xx UIM1_DET/SIM_DETECT pin state(SIM1)",
     (type_t(*)())display_wp76xx_simdetect_stat_util, SIM1,
     0,
     (type_t(*)())lte_has_2_sim_slot,                0,
     (type_t(*)())0,                                  0},
    {"LTE EM74xx SIM_DETECT_2 pin Test(SIM1) Insertion",
     (PFT)lte_em74xx_simdetect_pin_test_insertion_wrapper, SIM1,
     (MF_CONTINUOUS | MF_DOGRP | MF_SHOW_ERRCOUNT),
     (type_t(*) ())supports_em74xx_lte,               0,
     (type_t(*) ())0,                                 0},
    {"LTE EM74xx SIM_DETECT_2 pin Test(SIM1) Removal",
     (PFT)lte_em74xx_simdetect_pin_test_removal_wrapper,   SIM1,
     (MF_CONTINUOUS | MF_DOGRP | MF_SHOW_ERRCOUNT),
     (type_t(*) ())supports_em74xx_lte,               0,
     (type_t(*) ())0,                                 0},
};

#define LTE_UTILS_TBL_SZ \
        (sizeof(lte_utils_tbl) / sizeof(submenu_xtable_t))

static mitem_t lte_utils_pri_items[LTE_UTILS_TBL_SZ + MAX_BASE_ITEMS];
static mitem_t lte_utils_sec_items[LTE_UTILS_TBL_SZ + MAX_BASE_ITEMS];

static menuinfo_t lte_utils_menu = {
    "LTE Utilities Submenu",
    0,                            /* mtparam added by init_empty_menu */
    (PFT)menu_show_dflags,        /* shows major flags */
    0,                            /* use generic prompt */
    0,                            /* size (bumped by add_menu_item() */
    lte_utils_pri_items,
};
static menuinfo_t *lte_utils_menup = &lte_utils_menu;


/**********************************************************************
 *
 * Function   : has_lte_modem_reset_test
 * Description: Function to check if support LTE modem reset test.
 * Inputs     : opt - reserve for future use
 * Outputs    : TRUE(yes)/FALSE(no)
 *
 **********************************************************************
 */
boolean has_lte_modem_reset_test (int opt)
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
 * Function   : has_lte_usb_2_0_detection_test
 * Description: Function to check if support LTE usb2.0 detection test.
 * Inputs     : opt - reserve for future use
 * Outputs    : TRUE(yes)/FALSE(no)
 *
 **********************************************************************
 */
boolean has_lte_usb_2_0_detection_test (int opt)
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
 * Function   : has_lte_gps
 * Description: Function to check if support LTE GPS.
 * Inputs     : opt - reserve for future use
 * Outputs    : TRUE(yes)/FALSE(no)
 *
 **********************************************************************
 */
boolean has_lte_gps (int opt)
{
    if (this_is_star()) {
        return (TRUE);
    }
    return (FALSE);
}

/*******************************************************************************
 *
 * Function   : lte_utils_entry_fn
 * Description: Entry of LTE utilities.
 *              This function is to create LTE Utilities Submenu base on
 *              LTE utilities table.
 * Inputs     : None
 * Outputs    : PASSED or FAILED
 *
 *******************************************************************************
 */
int lte_utils_entry_fn (void)
{   
    build_primary_submenu(lte_utils_tbl, LTE_UTILS_TBL_SZ, 
                          "LTE Utilities Submenu", &lte_utils_menup);

    build_secondary_submenu(lte_utils_tbl, LTE_UTILS_TBL_SZ,
                            lte_utils_sec_items);

    menu(&lte_utils_menu, lte_utils_sec_items, '\0');

    return (PASSED);
}

/***************************************************************************
* Name: lte_utility_main
*
* Description: Creating Utils menu for LTE module
*
* Input: show menu option
*
* Output: PASSED/FAILED
***************************************************************************/
int lte_utility_main (int show_menu)
{

    build_primary_submenu(lte_menu_table, LTE_MENU_TABLE_SZ, 
                                "LTE Test Menu", &lte_menup); 
    build_secondary_submenu(lte_menu_table, LTE_MENU_TABLE_SZ, 
                                  lte_sec_items); 
                                   
    if (show_menu) { 
        menu(lte_menup, lte_sec_items, 0); 
        return PASS; 
    } else { 
        /* avoid to ask user memory size in test_patterns. 
         * Select memory size automatically,  
         * and process infinite test. 
         * For now is only march_C test. 
         */ 
        donot_query = TRUE;             
           if (!exec_doall_menu_items(lte_menup)) { 
             /* 
              * User did <BREAK>.  Display accumulated errors here only if 
              * not a continuous run because display will occur in menu() as 
              * a result of <BREAK>. 
              */ 
             if (!(DIAGFLAG & D_CONTINUOUS)) { 
                donot_query = TRUE;             
                menu_pr_err_accum(); 
             } 
             if (monjmpptr) { 
                longjmp(*monjmpptr, 1);  /* Back to previous point */ 
             } 
         } 
    } 

    return (PASSED);
}


/***************************************************************************
* Name: process_at_cmd
*
* Description: Process LTE's AT Commands
*
* Input: fd:Modem's filename;
*        atcmd:Modem's AT command
*        cmd: Tesing item.
*
* Output: PASSED/FAILED
***************************************************************************/
int process_at_cmd (int fd, at_cmd_str *atcmd, int printmsg, int at_test)
{
    char buffer[LENGTH1024] = { 0 };
    char *rslt_str, *rslt_str1, *rslt_str2, *rslt_str3;
    char *bufptr;
    int nbytes = 0;
    int cton, freq, db, high_pwr, low_pwr;
    fd_set tout_set;
    struct timeval timeout;
    int ret;

    memset(buffer, '\0', sizeof(buffer));

    /* Read from tty with timeout */
    FD_ZERO(&tout_set);
    FD_SET(fd, &tout_set);
    timeout.tv_sec  = AT_CMD_RESP_TOUT_IN_SEC;
    timeout.tv_usec = 0;
    if (write(fd, atcmd->str, strlen(atcmd->str)) < strlen(atcmd->str)) {
        if ((NVRAM)->diagflag & D_VERBOSE) {
            printf("Fail to write the AT command to %s\n", tty_dev_0);
        } 
        return (FAILED);
    }

    /*wait modem's response*/
    sleep(atcmd->delay);
    bufptr = buffer;

    ret = select(fd + 1, &tout_set, NULL, NULL, &timeout);

    if (ret < 1) {
        cterr('f', 0, "Modem is not responding to AT command");
        return (FAILED);
    }

    while ((nbytes = read(fd, bufptr, buffer + sizeof(buffer) - bufptr - 1)) > 0) {
        bufptr += nbytes;
        printf("Buffer is %s\n", buffer);
        fflush(stdout);
        if (bufptr[-1] == '\n' || bufptr[-1] == '\r') {
            break;
        }
    }
    
    if (bufptr == buffer) {
        printf("read return %d\n", nbytes);
        cterr('f', 0, "AT \"%s\", did not receive anything", atcmd->str);
        return (FAILED);
    }
    
    *bufptr = '\0';
    if (printmsg == 0) {
        if (strstr(buffer, "OK") != 0) {
            if ((NVRAM)->diagflag & D_VERBOSE) {
                printf("%s\n", buffer);
            } 
            return (PASSED);
        } else {
            return (FAILED);
        }
    } else {
        switch (at_test) {
            case GPS_ANTENNAE_TEST:
                rslt_str = strstr(buffer, "=");
                rslt_str1 = strstr(buffer, ",");
                if ((rslt_str == NULL) || (rslt_str1 == NULL)) {
                    printf("\n Could not collect GPS RF data for CTON \n");
                    break;
                }
                rslt_str++;
                rslt_str2 = strstr(rslt_str, "=");
                if (rslt_str2 == NULL) {
                    printf("\n Could not collect GPS RF data for Freq \n");
                    break;
                }
                rslt_str2++;
                rslt_str1 = NULL;
                rslt_str3 = strstr(rslt_str2, "\n");
                if (rslt_str3 == NULL) {
                    printf("\n Could not collect GPS RF data for Freq (no newline) \n");
                    break;
                }
                rslt_str3 = NULL;
                cton = atoi(rslt_str);
                freq = atoi(rslt_str2);
                if (((cton <= GPS_CTON_MAX) && (cton >= GPS_CTON_MIN)) && 
                   ((freq <= GPS_TEST_FREQ_MAX) && (freq >= GPS_TEST_FREQ_MIN))) {
                    printf("\n GPS RF Passed CtoN = %ddBm, Freq = %dHz\n", cton, freq);
                    return (PASSED);
                } else {
                    printf("\n ***GPS RF FAILED CtoN = %ddBm, Freq = %dHz\n"
                           " ***If Stop on error ... download kernel again and "
                           "Set GPSAUTOSTART\n", cton, freq);
                    cterr('f', 0, " GPS RF Failed CtoN = %ddB, Freq = %dHz\n"
                          " CtoN should be within 58 +/- 5dBm and Freq within "
                          "100000 Hz +/- 5000 Hz.\n", cton, freq);
                    return (FAILED);
                }
                break;
            case RSSI_LTE_MAIN_TEST:
            case RSSI_LTE_DIV_TEST:
            case RSSI_3G_MAIN_TEST:
            case RSSI_3G_DIV_TEST:
                high_pwr = dbm_pwr + 6;
                low_pwr = dbm_pwr - 6;
                rslt_str1 = (char *) strchr(buffer, '-');
                if ((at_test == RSSI_LTE_MAIN_TEST) || (at_test == RSSI_LTE_DIV_TEST)) {
                    if (at_test == RSSI_LTE_DIV_TEST) {
                        if (rslt_str1) {
                            rslt_str1++;
                            rslt_str1 = (char *) strchr(rslt_str1, '-');
                        }
                    }
                    if (rslt_str1) {
                        rslt_str2 = (char *) strstr(rslt_str1, " dBm");
                        rslt_str2 = NULL;
                    }
                }
                if (rslt_str1) {
                    db = atoi(rslt_str1);
                } else {
                    db = 0;
                }
                if ((db > high_pwr) || (db < low_pwr)) {
                    printf("\nTest Failed dBm = %d expected dBm between %d and "
                           "%d dBm\n***If Stop on error ...download kernel\n", db, high_pwr, low_pwr);
                    cterr('f', 0, "Reading RSSI = %d dBm Test failed"
                          "\nWarning: Please verify the settings of the signal generator.", db);
                } else {
                    printf("\n Test Passed dbm = %d", db);
                    return (PASSED);
                }
                break;
            case LTE_SIM_TEST:
            case LTE_SIM1_TEST:
                if (strstr(buffer, "OK") != 0) {
                    if (!quiet_launch) {
                        if (at_test == LTE_SIM_TEST)
                           prpass(testpass, "Detect SIM 0 card passed, ");
                        else
                           prpass(testpass, "Detect SIM 1 card passed, ");
                    }
                    return (PASSED);
                } else {
                    if (at_test == LTE_SIM_TEST)
                        cterr('f', 0, "cannot detect SIM 0 card\r\n");
                    else  
                        cterr('f', 0, "cannot detect SIM 1 card\r\n");
                    return (FAILED);
                }
                break;
            case LTE_WP_SIM_PROTECT:
                return (PASSED);
            case ATI:
                if (strstr(buffer, "OK") != 0) {
                    if (!quiet_launch) {
                        prpass(testpass, "Detect LTE modem passed, ");
                    }
                    return (PASSED);
                } else {
                    cterr('f', 0, "cannot detect LTE modem\n");
                    return (FAILED);
                }
            case RSSI_RESET_MODEM:
                if (strstr(buffer, "OK") != 0) {
                    if (!quiet_launch) {
                        prpass(testpass, "Reset LTE modem passed, ");
                    }
                    return (PASSED);
                } else {
                    cterr('f', 0, "cannot reset LTE modem\n");
                    return (FAILED);
                }
                break;
            case LTE_GPS_ENABLE:
                return (PASSED);
                break;
            case LTE_GPS_FIXES_STATUS:
                return (PASSED);
                break;
            case LTE_GPS_DR_SYNC_TEST:
            case LTE_GPS_DR_SYNC_FORCE_HIGH:
            case LTE_GPS_DR_SYNC_FORCE_LOW:
                return (PASSED);
                break;
            case EM74XX_SIMDETECT_L:
            case EM74XX_SIMDETECT2_L:
            case WP76XX_UIM1_DET_L:
                if (strstr(buffer, "State:     0") != 0) {
                    return (PASSED);
                } else {
                    return (FAILED);
                }
                break;
            case EM74XX_SIMDETECT_H:
            case EM74XX_SIMDETECT2_H:
            case WP76XX_UIM1_DET_H:
                if (strstr(buffer, "State:     1") != 0) {
                    return (PASSED);
                } else {
                    return (FAILED);
                }
                break;
            case EM74XX_SIMDETECT_STAT:
            case EM74XX_SIMDETECT2_STAT:
            case WP76XX_UIM1_DET_STAT:
                if (strstr(buffer, "State:     0") != 0) {
                    em74xx_simdetect_state = 0;
                    wp76xx_simdetect_state = 0;
                } else if (strstr(buffer, "State:     1") != 0) {
                    em74xx_simdetect_state = 1;
                    wp76xx_simdetect_state = 1;
                }

                if (strstr(buffer, "OK") != 0) {
                    return (PASSED);
                } else {
                    return (FAILED);
                }
                break;
            case LTE_SET_IMG_VERZ:
            case LTE_SET_IMG_ATT:
            case LTE_SET_IMG_GENC:
            case LTE_EN_AUTO_SWITCH_IMG:
            case LTE_PWR_DOWN:
                return (PASSED);
                break;
            default:
                printf("\n Don't support the AT test for %d", at_test);
                break;
        }
    }
    return (FAILED);
}

/***************************************************************************
* Name: tsn_get_lte_0_main_rssi
*
* Description: To get LTE main RSSI
*
* Input: N/A
*
* Output: PASSED/FAILED
***************************************************************************/
int tsn_get_lte_0_main_rssi (void)
{
    uchar mb_get_loc[FRU_SIZE] = {0};
    uchar mb_get_pid[FRU_SIZE] = {0};
#ifdef ENHANCE_ERROR_MSG_RDY
    /*
     * 1. Subtests of the test function will reuse all variables
     * 2. All variables will be cleared automatically when
     *    entering and leaving each menu item.
     */
    /* Segment 1: PID | Unique_string : slot_info */
    fru_table_offset = MB;
    /* fru_table_offset should be set, otherwise, it will not */
    /* go to enhanced error message format in cterr() */
    /* set fru_table_offset to get the predefine value */
    /* or change mb_pid & mb_loc below */
    platform_get_pid((char *)mb_get_pid);
    strcpy((char *)mb_get_loc, "MB");
    platform_fru_table[fru_table_offset].pid_string = mb_get_pid;
    platform_fru_table[fru_table_offset].location_string = mb_get_loc;

    /* Segment 2: Test step captured from prpass */
    /* Segment 3: Failure message captured from cterr */

    /* Segment 4: Components used */
    cterr_add_component("Marvell Armada 7040", "USB 3.0", "Sierra Wireless EM7455 or EM7430 WAN Modem");

    /* Segment 5: register and memory dump */
    //cterr_add_reg_dump((PFV)bcm63168_get_xdsl_profile);

    /* Segment 6: Platform Environment initialized here*/
    cterr_add_env_dump((PFV)tsn_display_temp_errormsg);

    /* Segment 7: Top 3 Debugging Steps */
    cterr_add_debug("Check if the external signal generator "
                    "connects correctly and fastened.",
                    "If test is still failed, contact "
                    "module vendor for support.");
#endif

    int ret = 0;
    char *tname = "Modem_0 3G Main RSSI";

    testname(tname);
    prpass(testpass, "%s, ", tname);

    /* Polling modem detection, in case modem is not ready */
    if (lte_check_modem_rdy(0) != PASSED) {
        cterr('f', 0, "LTE module is not ready");
        return (FAILED);
    }
    
    lte_get_model_name(USB_SYSFS_PATH);

    ret = run_at_cmd(RSSI_LTE_MAIN_TEST);
    if (ret == FAILED) {
        cterr('f', 0, "Modem_0 3G Main RSSI test failed");
    }

    prcomplete(testpass, errcount, (char *)0);
    return (ret);
}


/***************************************************************************
* Name: tsn_get_lte_0_div_rssi
*
* Description: To get LTE DIV RSSI
*
* Input: N/A
*
* Output: PASSED/FAILED
***************************************************************************/
int tsn_get_lte_0_div_rssi (void)
{
    uchar mb_get_loc[FRU_SIZE] = {0};
    uchar mb_get_pid[FRU_SIZE] = {0};
#ifdef ENHANCE_ERROR_MSG_RDY
    /*
     * 1. Subtests of the test function will reuse all variables
     * 2. All variables will be cleared automatically when
     *    entering and leaving each menu item.
     */
    /* Segment 1: PID | Unique_string : slot_info */
    fru_table_offset = MB;
    /* fru_table_offset should be set, otherwise, it will not */
    /* go to enhanced error message format in cterr() */
    /* set fru_table_offset to get the predefine value */
    /* or change mb_pid & mb_loc below */
    platform_get_pid((char *)mb_get_pid);
    strcpy((char *)mb_get_loc, "MB");
    platform_fru_table[fru_table_offset].pid_string = mb_get_pid;
    platform_fru_table[fru_table_offset].location_string = mb_get_loc;

    /* Segment 2: Test step captured from prpass */
    /* Segment 3: Failure message captured from cterr */

    /* Segment 4: Components used */
    cterr_add_component("Marvell Armada 7040", "USB 3.0", "Sierra Wireless EM7455 or EM7430 WAN Modem");

    /* Segment 5: register and memory dump */
    //cterr_add_reg_dump((PFV)bcm63168_get_xdsl_profile);

    /* Segment 6: Platform Environment initialized here*/
    cterr_add_env_dump((PFV)tsn_display_temp_errormsg);

    /* Segment 7: Top 3 Debugging Steps */
    cterr_add_debug("Check if the external signal generator "
                    "connects correctly and fastened.",
                    "If test is still failed, contact "
                    "module vendor for support.");
#endif

    int ret = 0;
    char *tname = "Modem_0 3G DIV RSSI";

    testname(tname);
    prpass(testpass, "%s, ", tname);

     /* Polling modem detection, in case modem is not ready */
    if (lte_check_modem_rdy(0) != PASSED) {
        cterr('f', 0, "LTE module is not ready");
        return (FAILED);
    }

    ret = run_at_cmd(RSSI_LTE_DIV_TEST);
    if (ret == FAILED) {
        cterr('f', 0, "Modem_0 3G DIV RSSI test failed");
    }

    prcomplete(testpass, errcount, (char *)0);
    return (ret);
}


/***************************************************************************
* Name: tsn_get_lte_0_gps_antennae_test
*
* Description: To get LTE GPS antennae information
*
* Input: N/A
*
* Output: PASSED/FAILED
***************************************************************************/
int tsn_get_lte_0_gps_antennae_test (void)
{
    uchar mb_get_loc[FRU_SIZE] = {0};
    uchar mb_get_pid[FRU_SIZE] = {0};
#ifdef ENHANCE_ERROR_MSG_RDY
    /*
     * 1. Subtests of the test function will reuse all variables
     * 2. All variables will be cleared automatically when
     *    entering and leaving each menu item.
     */
    /* Segment 1: PID | Unique_string : slot_info */
    fru_table_offset = MB;
    /* fru_table_offset should be set, otherwise, it will not */
    /* go to enhanced error message format in cterr() */
    /* set fru_table_offset to get the predefine value */
    /* or change mb_pid & mb_loc below */
    platform_get_pid((char *)mb_get_pid);
    strcpy((char *)mb_get_loc, "MB");
    platform_fru_table[fru_table_offset].pid_string = mb_get_pid;
    platform_fru_table[fru_table_offset].location_string = mb_get_loc;

    /* Segment 2: Test step captured from prpass */
    /* Segment 3: Failure message captured from cterr */

    /* Segment 4: Components used */
    cterr_add_component("Marvell Armada 7040", "USB 3.0", "Sierra Wireless EM7455 or EM7430 WAN Modem");

    /* Segment 5: register and memory dump */
    //cterr_add_reg_dump((PFV)bcm63168_get_xdsl_profile);

    /* Segment 6: Platform Environment initialized here*/
    cterr_add_env_dump((PFV)tsn_display_temp_errormsg);

    /* Segment 7: Top 3 Debugging Steps */
    cterr_add_debug("Check the interface between the Host SoC "
                    "and the WAN module.",
                    "If there is no problem for these interfaces, "
                    "replace one module and redo the test.",
                    "If test is still failed, contact "
                    "module vendor for support");
#endif

    int ret = 0;
    char *tname = "LTE_0 GPS Antennae";

    testname(tname);
    prpass(testpass, "%s, ", tname);

    /* Polling modem detection, in case modem is not ready */
    if (lte_check_modem_rdy(0) != PASSED) {
        cterr('f', 0, "LTE module is not ready");
        return (FAILED);
    }

    ret = run_at_cmd(GPS_ANTENNAE_TEST);
    if (ret == FAILED) {
        cterr('f', 0, "LTE_0 GPS Antennae test failed");
    }

    prcomplete(testpass, errcount, (char *)0);
    return (ret);
}


/***************************************************************************
* Name: tsn_get_lte_0_sim_0_card_test
*
* Description: To detect LTE SIM card
*
* Input: None 
*
* Output: PASSED/FAILED
***************************************************************************/
int tsn_get_lte_0_sim_0_card_test (void)
{
    int ret = PASSED;
    uint data;

    /* check if  SIM 0 exists */
    fpga_read_32_reg(FPGA_SIM_STATUS_CTL_REG, &data);
    if (!(data & LTE_SIM_0_PRESENT_DECTECT)) {
        cterr('f', 0, "\nNo SIM0 Present!!!"
              "\nPlease insert the SIM card and reset the modem.\n");
        return(FAILED);
    }
    
    if (!(this_is_star_c1109_2p() || this_is_supernova_c959_2p())) {
        /* Enable SIM 0 */
        fpga_read_32_reg(FPGA_SIM_STATUS_CTL_REG, &data);
        data &= ~LTE_SIM_SOCKET_SEL;
        fpga_write_32_reg(FPGA_SIM_STATUS_CTL_REG, data);
    }
    
    /* Polling modem detection, in case modem is not ready */
    if (lte_check_modem_rdy(0) != PASSED) {
        cterr('f', 0, "LTE module is not ready");
        return (FAILED);
    }
    
    if (this_is_star_c1109_2p() || this_is_supernova_c959_2p()) {
        /* WP modem */
        /* 1. Switch to the uims interface which is not in use
         *    to avoid power glitch
         */
        /* CSCvq20258: one sim card sku can not get expect result with at+cpin.
         *
         * WP_SIM_PROTECT is used to avoid power glitch that FPGA switch sim mux, 
         * but one SIM slot has not switch sim mux.
         * So WP_SIM_PROTECT is redundant command in one SIM slot unit and 
         * it only used in two SIM slot unit.
         */ 
        if (lte_has_2_sim_slot()) {
            ret = run_at_cmd(LTE_WP_SIM_PROTECT);
            if (ret == FAILED) {
                cterr('f', 0, "Detect LTE SIM card 0 failed");
            }
        }
        
        /* 2. Set SIM mux */
        fpga_read_32_reg(FPGA_SIM_STATUS_CTL_REG, &data);
        data &= ~LTE_SIM_SOCKET_SEL;
        fpga_write_32_reg(FPGA_SIM_STATUS_CTL_REG, data);
        
        msleep(LTE_SIM_MUX_SWITCH_DELAY);      
        lte_get_model_name(USB_SYSFS_PATH);
    }
    
    ret = run_at_cmd(LTE_SIM_TEST);
    if (ret == FAILED) {
        cterr('f', 0, "Detect LTE SIM card 0 failed");
    }
   
    return (ret);
}

/***************************************************************************
* Name: tsn_get_lte_0_sim_1_card_test
*
* Description: To detect LTE SIM card
*
* Input: N/A
*
* Output: PASSED/FAILED
***************************************************************************/
int tsn_get_lte_0_sim_1_card_test (void)
{
    int ret = 0;
    uint data;

    /* check if SIM 1 exists */
    fpga_read_32_reg(FPGA_SIM_STATUS_CTL_REG, &data);
    if (!(data & LTE_SIM_1_PRESENT_DECTECT)) {
        cterr('f', 0, "\nNo SIM1 Present!!!"
              "\nPlease insert the SIM card and reset the modem.\n");
        return(FAILED);
    }
    
    if (!(this_is_star_c1109_2p() || this_is_supernova_c959_2p())) {
        /* Enable SIM 1 */
        fpga_read_32_reg(FPGA_SIM_STATUS_CTL_REG, &data);
        data |= LTE_SIM_SOCKET_SEL;
        fpga_write_32_reg(FPGA_SIM_STATUS_CTL_REG, data);
    }

    /* Polling modem detection, in case modem is not ready */
    if (lte_check_modem_rdy(0) != PASSED) {
        cterr('f', 0, "LTE module is not ready");
        return (FAILED);
    }
    
    if (this_is_star_c1109_2p() || this_is_supernova_c959_2p()) {
        /* WP modem */
        /* 1. Switch to the uims interface which is not in use
         *    to avoid power glitch
         */
        ret = run_at_cmd(LTE_WP_SIM_PROTECT);
        if (ret == FAILED) {
            cterr('f', 0, "Detect LTE SIM card 0 failed");
        }
        
        /* 2. Set SIM mux */
        fpga_read_32_reg(FPGA_SIM_STATUS_CTL_REG, &data);
        data |= LTE_SIM_SOCKET_SEL;
        fpga_write_32_reg(FPGA_SIM_STATUS_CTL_REG, data);
        
        msleep(LTE_SIM_MUX_SWITCH_DELAY);      
        lte_get_model_name(USB_SYSFS_PATH);
    }
   
    ret = run_at_cmd(LTE_SIM1_TEST);
    if (ret == FAILED) {
        cterr('f', 0, "Detect LTE SIM card 1 failed");
    }
  
    return (ret);
}


/***************************************************************************
* Name: tsn_sim_0_test
*
* Description: To detect LTE SIM card
*
* Input: N/A
*
* Output: PASSED/FAILED
***************************************************************************/
int tsn_sim_0_test (void)
{
    uchar mb_get_loc[FRU_SIZE] = {0};
    uchar mb_get_pid[FRU_SIZE] = {0};
#ifdef ENHANCE_ERROR_MSG_RDY
    /*
     * 1. Subtests of the test function will reuse all variables
     * 2. All variables will be cleared automatically when
     *    entering and leaving each menu item.
     */
    /* Segment 1: PID | Unique_string : slot_info */
    fru_table_offset = MB;
    /* fru_table_offset should be set, otherwise, it will not */
    /* go to enhanced error message format in cterr() */
    /* set fru_table_offset to get the predefine value */
    /* or change mb_pid & mb_loc below */
    platform_get_pid((char *)mb_get_pid);
    strcpy((char *)mb_get_loc, "MB");
    platform_fru_table[fru_table_offset].pid_string = mb_get_pid;
    platform_fru_table[fru_table_offset].location_string = mb_get_loc;

    /* Segment 2: Test step captured from prpass */
    /* Segment 3: Failure message captured from cterr */

    /* Segment 4: Components used */
    cterr_add_component("Marvell Armada 7040", "USB 3.0", "Sierra Wireless EM7455 or EM7430 WAN Modem");

    /* Segment 5: register and memory dump */
    //cterr_add_reg_dump((PFV)bcm63168_get_xdsl_profile);

    /* Segment 6: Platform Environment initialized here*/
    cterr_add_env_dump((PFV)tsn_display_temp_errormsg);

    /* Segment 7: Top 3 Debugging Steps */
    cterr_add_debug("Check the interface between the Host SoC "
                    "and the WAN module.",
                    "If there is no problem for these interfaces, "
                    "replace one module and redo the test.",
                    "If test is still failed, contact "
                    "module vendor for support");
#endif

    char *tname = "SIM 0 Card Test";
    testname("%s", tname);
    prpass(testpass, "%s, ", tname);

    if (tsn_get_lte_0_sim_0_card_test() == FAILED) {
        prcomplete(testpass, errcount, (char *)0);
        return (FAILED);
    }

    prcomplete(testpass, errcount, (char *)0);
    return (PASSED);
}

/***************************************************************************
* Name: tsn_sim_1_test
*
* Description: To detect LTE SIM card
*
* Input: N/A
*
* Output: PASSED/FAILED
***************************************************************************/
int tsn_sim_1_test (void)
{
    uchar mb_get_loc[FRU_SIZE] = {0};
    uchar mb_get_pid[FRU_SIZE] = {0};
#ifdef ENHANCE_ERROR_MSG_RDY
    /*
     * 1. Subtests of the test function will reuse all variables
     * 2. All variables will be cleared automatically when
     *    entering and leaving each menu item.
     */
    /* Segment 1: PID | Unique_string : slot_info */
    fru_table_offset = MB;
    /* fru_table_offset should be set, otherwise, it will not */
    /* go to enhanced error message format in cterr() */
    /* set fru_table_offset to get the predefine value */
    /* or change mb_pid & mb_loc below */
    platform_get_pid((char *)mb_get_pid);
    strcpy((char *)mb_get_loc, "MB");
    platform_fru_table[fru_table_offset].pid_string = mb_get_pid;
    platform_fru_table[fru_table_offset].location_string = mb_get_loc;

    /* Segment 2: Test step captured from prpass */
    /* Segment 3: Failure message captured from cterr */

    /* Segment 4: Components used */
    cterr_add_component("Marvell Armada 7040", "USB 3.0", "Sierra Wireless EM7455 or EM7430 WAN Modem");

    /* Segment 5: register and memory dump */
    //cterr_add_reg_dump((PFV)bcm63168_get_xdsl_profile);

    /* Segment 6: Platform Environment initialized here*/
    cterr_add_env_dump((PFV)tsn_display_temp_errormsg);

    /* Segment 7: Top 3 Debugging Steps */
    cterr_add_debug("Check the interface between the Host SoC "
                    "and the WAN module.",
                    "If there is no problem for these interfaces, "
                    "replace one module and redo the test.",
                    "If test is still failed, contact "
                    "module vendor for support");
#endif

    char *tname = "SIM 1 Card Test";

    testname("%s", tname);
    prpass(testpass, "%s, ", tname);

    if (tsn_get_lte_0_sim_1_card_test() == FAILED) {
        prcomplete(testpass, errcount, (char *)0);
        return (FAILED);
    }
    prcomplete(testpass, errcount, (char *)0);

    return (PASSED);
}


/***************************************************************************
* Name: tsn_modem_0_reset_test
*
* Description: To reset LTE modem
*
* Input: N/A
*
* Output: PASSED/FAILED
***************************************************************************/
int tsn_modem_0_reset_test (void)
{
    uchar mb_get_loc[FRU_SIZE] = {0};
    uchar mb_get_pid[FRU_SIZE] = {0};
#ifdef ENHANCE_ERROR_MSG_RDY
    /*
     * 1. Subtests of the test function will reuse all variables
     * 2. All variables will be cleared automatically when
     *    entering and leaving each menu item.
     */
    /* Segment 1: PID | Unique_string : slot_info */
    fru_table_offset = MB;
    /* fru_table_offset should be set, otherwise, it will not */
    /* go to enhanced error message format in cterr() */
    /* set fru_table_offset to get the predefine value */
    /* or change mb_pid & mb_loc below */
    platform_get_pid((char *)mb_get_pid);
    strcpy((char *)mb_get_loc, "MB");
    platform_fru_table[fru_table_offset].pid_string = mb_get_pid;
    platform_fru_table[fru_table_offset].location_string = mb_get_loc;

    /* Segment 2: Test step captured from prpass */
    /* Segment 3: Failure message captured from cterr */

    /* Segment 4: Components used */
    cterr_add_component("Marvell Armada 7040", "USB 3.0", "Sierra Wireless EM7455 or EM7430 WAN Modem");

    /* Segment 5: register and memory dump */
    //cterr_add_reg_dump((PFV)bcm63168_get_xdsl_profile);

    /* Segment 6: Platform Environment initialized here*/
    cterr_add_env_dump((PFV)tsn_display_temp_errormsg);

    /* Segment 7: Top 3 Debugging Steps */
    cterr_add_debug("Check the interface between the Host SoC "
                    "and the WAN module.",
                    "If there is no problem for these interfaces, "
                    "replace one module and redo the test.",
                    "If test is still failed, contact "
                    "module vendor for support");
#endif

    int ret = 0;
    int stat;
    int ix;
    char fname[64];
    char *tname = "Modem_0 Reset";
    char usb_tty_dev[256];

    testname("%s", tname);
    prpass(testpass, "%s, ", tname);

    /* Dynamically get the corresponding ttyUSB number in case usb device
     * attaches to different ttyUSB 
     */     
    if (lte_get_tty_num(usb_tty_dev) != PASSED) {
        printf("%s:Can't get ttyUSB number\n", __func__);
        return (FAILED);
    }
    
    /* Polling modem detection, in case modem is not ready */
    if (lte_check_modem_rdy(0) != PASSED) {
        cterr('f', 0, "LTE module is not ready");
        return (FAILED);
    }

    ret = run_at_cmd(RSSI_RESET_MODEM);
    if (ret == FAILED) {
        cterr('f', 0, "%s failed.", tname);
    }
    else {
        for (ix = 0; ix < USB_TTY_TOUT; ix++) {
            sprintf(fname, "%s%s", USB_TTY_PATH, usb_tty_dev);
            if (access(fname, F_OK) == -1) {
                printf("Modem start to reset.\n");
                stat = PASSED;
                break;
            }
            msleep(10);
        }
        if (stat != PASSED) {
            printf("Modem failed to reset.\n");
            return (FAILED);
        }
        /* Check if modem is out of reset */
        if (lte_check_modem_rdy(0) != PASSED) {
            cterr('f', 0, "LTE module is not ready");
            return (FAILED);
        }
    }
    
    prcomplete(testpass, errcount, (char *)0);
    return (ret);   
}


/***************************************************************************
* Name: tsn_modem_0_detect_test
*
* Description: To detect LTE modem
*
* Input: N/A
*
* Output: PASSED/FAILED
***************************************************************************/
int tsn_modem_0_detect_test (void)
{
    uchar mb_get_loc[FRU_SIZE] = {0};
    uchar mb_get_pid[FRU_SIZE] = {0};
#ifdef ENHANCE_ERROR_MSG_RDY
    /*
     * 1. Subtests of the test function will reuse all variables
     * 2. All variables will be cleared automatically when
     *    entering and leaving each menu item.
     */
    /* Segment 1: PID | Unique_string : slot_info */
    fru_table_offset = MB;
    /* fru_table_offset should be set, otherwise, it will not */
    /* go to enhanced error message format in cterr() */
    /* set fru_table_offset to get the predefine value */
    /* or change mb_pid & mb_loc below */
    platform_get_pid((char *)mb_get_pid);
    strcpy((char *)mb_get_loc, "MB");
    platform_fru_table[fru_table_offset].pid_string = mb_get_pid;
    platform_fru_table[fru_table_offset].location_string = mb_get_loc;

    /* Segment 2: Test step captured from prpass */
    /* Segment 3: Failure message captured from cterr */

    /* Segment 4: Components used */
    cterr_add_component("Marvell Armada 7040", "USB 3.0", "Sierra Wireless EM7455 or EM7430 WAN Modem");

    /* Segment 5: register and memory dump */
    //cterr_add_reg_dump((PFV)bcm63168_get_xdsl_profile);

    /* Segment 6: Platform Environment initialized here*/
    cterr_add_env_dump((PFV)tsn_display_temp_errormsg);

    /* Segment 7: Top 3 Debugging Steps */
    cterr_add_debug("Check the interface between the Host SoC "
                    "and the WAN module.",
                    "If there is no problem for these interfaces, "
                    "replace one module and redo the test.",
                    "If test is still failed, contact "
                    "module vendor for support");
#endif

    int ret = 0;

    char *tname = "Modem_0 Detection";

    testname("%s", tname);
    prpass(testpass, "%s, ", tname);

     /* Polling modem detection, in case modem is not ready */
    if (lte_check_modem_rdy(0) != PASSED) {
        cterr('f', 0, "LTE module is not ready");
        return (FAILED);
    }

    ret = run_at_cmd(ATI);
    
    if (ret == FAILED) {
        cterr('f', 0, "%s failed", tname);
        prcomplete(testpass, errcount, (char *)0);
        return (FAILED);
    }

    prcomplete(testpass, errcount, (char *)0);
    return (PASSED);
}

/***************************************************************************
* Name: lte_get_model_name
*
* Description: Get LTE Model Name from sysfs
*
* Input: path:sysfs LTE model name path
*
* Output: PASSED/FAILED
***************************************************************************/
int lte_get_model_name (char *path)
{
    int rc = 0, len=0;
    char buf[512], *p = buf, cmd[512];
    FILE *fp;
    char c;

    memset(model_name, 0, sizeof(model_name));
    memset(cmd, 0, sizeof(cmd));
    
    sprintf(cmd, "cat %s 2>/dev/null", path);
    /*opens a process by creating a pipe, forking, and invoking the shell.*/
    fp = popen(cmd, "r");
    if (NULL == fp) {
        printf("popen Fail! \n");
        return (FAILED);
    }

    while ((c = fgetc(fp)) != EOF && c != '\n') {
        if (len >= 512) {
            return (FAILED);
        }
        p[len++] = (char)c;
    }

    /*waits for the associated process to terminate and returns the exit status of the command.*/
    rc = pclose(fp);
 
    if (-1 == rc) {
        printf("pclose Fail! \n");
        return (FAILED);
    }

    strcpy(model_name, buf);
    return (PASSED);
}

/***************************************************************************
* Name: run_at_cmd
*
* Description: To execute AT command
*
* Input: at_test = which command want to execute
*
* Output: PASSED/FAILED
***************************************************************************/
int run_at_cmd (int at_test)
{
    at_cmd_str *at_cmd;
    int ix, retval, printmsg;
    unsigned short no_at_cmds;
    char *tname="";
    
    no_at_cmds = 0;
    at_cmd = NULL;
    retval = 1;
    printmsg = 0;


    /* Get TTY Device file descriptor */
    if (lte_at_open_tty(&fd) == FAILED) {
        return (FAILED);
    }

    if (lte_at_selftest(fd) == FAILED) {
        close(fd);
        printf("lte_at_selftest failed, at_test = %d\n", at_test);
        return (FAILED);
    }

    /* Start sending the AT commands */
    switch (at_test) {
        case RSSI_LTE_MAIN_TEST:
        case RSSI_LTE_DIV_TEST:
            tname = "LTE Main RSSI";
            dbm_pwr = -70;
            if ((strstr(model_name, "WP7601") == NULL) && (strstr(model_name, "WP7603") == NULL)) {
                printf("\nFreq = %s MHz, Power = %sdB\n",
                       RSSI_B8_FREQ, RSSI_AMP);
                no_at_cmds = lte_main_rssi_b8_no_at_cmd;
                at_cmd = lte_main_rssi_b8_at_cmd_str;
            } else {
                printf("\nFreq = %s MHz, Power = %sdB\n", 
                       RSSI_B4_FREQ, RSSI_AMP);
                no_at_cmds = lte_main_rssi_B4_no_at_cmd;
                at_cmd = lte_main_rssi_B4_at_cmd_str;
            }
            break;
        case ENABLE_GPS:
            tname = "Enable GPS";
            no_at_cmds = gps_enable_at_cmd_str_size;
            at_cmd = gps_enable_at_cmd_str;
            break;
        case DISABLE_GPS:
            tname = "Disable GPS";
            no_at_cmds = gps_dis_no_at_cmd;
            at_cmd = gps_dis_at_cmd_str;
            break;
        case SET_GPS_PORT_CONN1:
            tname = "Select GPS connector 1 Port";
            no_at_cmds = set_gps_port1_no_at_cmd;
            at_cmd = set_gps_port1_str;
            break;
        case SET_GPS_PORT_CONN2:
            tname = "Select GPS connector 2 Port";
            no_at_cmds = set_gps_port2_no_at_cmd;
            at_cmd = set_gps_port2_str;
            break;
        case GPSAUTOSTART_OFF:
            tname = "Turn off GPS Auto Start";
            no_at_cmds = gps_autostartdis_no_at_cmd;
            at_cmd = gps_autostartdis_at_cmd_str;
            break;
        case GPSAUTOSTART_ON:
            tname = "Turn on GPS Auto Start";
            no_at_cmds = gps_autostarten_no_at_cmd;
            at_cmd = gps_autostarten_at_cmd_str;
            break;
        case GPS_ANTENNAE_TEST:
            tname = "GPS Antennae";
            printf("\nSignal Generator Freq = 1575.52 MHz, Power = -110dBm.\n");
            no_at_cmds = gps_rssi_7750_no_at_cmd;
            at_cmd = gps_rssi_7750_at_cmd_str;
            break;
        case RSSI_3G_MAIN_TEST:
            tname = "UMTS RF Receive Path Test for Main";
            printf("\nFreq = 882.6 MHz, Power = -80dB\n");
            no_at_cmds = rssi_3g_b22_no_at_cmd;
            at_cmd = rssi_3g_main_b22_at_cmd_str;
            dbm_pwr = -80;
            break;
        case RSSI_3G_DIV_TEST:
            tname = "UMTS RF Receive Path Test for DIV";
            printf("\nFreq = 882.6 MHz, Power = -80dB\n");
            no_at_cmds = rssi_div_3g_b22_no_at_cmd;
            at_cmd = rssi_3g_div_b22_at_cmd_str;
            dbm_pwr = -80;
            break;
        case LTE_SIM_TEST:
            tname = "LTE SIM 0 Card";            
            no_at_cmds = lte_sim_no_at_cmd;
            at_cmd = lte_sim_at_cmd_str;
            break;
        case LTE_SIM1_TEST:
            tname = "LTE SIM 1 Card";            
            if (strstr(model_name, "WP76") == NULL) {
                no_at_cmds = lte_sim1_no_at_cmd;
                at_cmd = lte_sim1_at_cmd_str;
            } else {
                no_at_cmds = lte_sim_no_at_cmd;
                at_cmd = lte_sim_at_cmd_str;
            }
            break;
        case LTE_WP_SIM_PROTECT:
            at_cmd = star_lte_wp_sim_protect_str;
            no_at_cmds = star_lte_wp_sim_protect_str_size;
            break;
        case ATI:
            tname = "Modem Detection";
            no_at_cmds = no_ati_at_cmd;
            at_cmd = ati_cmd_str;
            break;
        case AT_FROM_CONSOLE:
            printf("\r Type ^ to Exit                             \n");
            break;
        case RSSI_RESET_MODEM:
            tname = "Modem Reset";            
            no_at_cmds = rssi_reset_no_at_cmd;
            at_cmd = rssi_reset_at_cmd_str;
            break;
        case LTE_GPS_ENABLE:
            at_cmd = gps_enable_at_cmd_str;
            no_at_cmds = gps_enable_at_cmd_str_size;
            break;
        case LTE_GPS_FIXES_STATUS:
            at_cmd = lte_gps_fix_at_cmd_str;
            no_at_cmds = lte_gps_fix_at_cmd_str_size;
            break;
        case LTE_GPS_DR_SYNC_TEST:
            at_cmd = lte_gps_dr_sync_at_cmd_str;
            no_at_cmds = lte_gps_dr_sync_at_cmd_str_size;
            break;
        case LTE_GPS_DR_SYNC_FORCE_HIGH:
            at_cmd = lte_gps_dr_sync_h_at_cmd_str;
            no_at_cmds = lte_gps_dr_sync_h_at_cmd_str_size;
            break;
        case LTE_GPS_DR_SYNC_FORCE_LOW:
            at_cmd = lte_gps_dr_sync_l_at_cmd_str;
            no_at_cmds = lte_gps_dr_sync_l_at_cmd_str_size;
            break;
        case EM74XX_SIMDETECT_L:
        case EM74XX_SIMDETECT_H:
        case EM74XX_SIMDETECT_STAT:
            tname = "EM74xx LTE SIM_DETECT pin";
            no_at_cmds = em74xx_simdetect_at_cmd_size;
            at_cmd = em74xx_simdetect_at_cmd;
            break;
        case EM74XX_SIMDETECT2_L:
        case EM74XX_SIMDETECT2_H:
        case EM74XX_SIMDETECT2_STAT:
            tname = "EM74xx LTE SIM_DETECT_2 pin";
            no_at_cmds = em74xx_simdetect2_at_cmd_size;
            at_cmd = em74xx_simdetect2_at_cmd;
            break;

        case WP76XX_UIM1_DET_L:
        case WP76XX_UIM1_DET_H:
        case WP76XX_UIM1_DET_STAT:
            tname = "WP76XX LTE UIM1_DET pin";
            no_at_cmds = wp76xx_simdetect_at_cmd_size;
            at_cmd = wp76xx_simdetect_at_cmd;
            break;
        case LTE_SET_IMG_GENC:
            at_cmd = lte_set_img_generic_at_cmd_str;
            no_at_cmds = lte_set_img_generic_at_cmd_str_size;
            break;
        case LTE_SET_IMG_ATT:
            at_cmd = lte_set_img_att_at_cmd_str;
            no_at_cmds = lte_set_img_att_at_cmd_str_size;
            break;
        case LTE_SET_IMG_VERZ:
            at_cmd = lte_set_img_verizon_at_cmd_str;
            no_at_cmds = lte_set_img_verizon_at_cmd_str_size;
            break;
        case LTE_EN_AUTO_SWITCH_IMG:
            at_cmd = lte_en_auto_switch_img_at_cmd_str;
            no_at_cmds = lte_en_auto_switch_img_at_cmd_str_size;
            break;
        case LTE_PWR_DOWN:
            at_cmd = lte_pwr_down_at_cmd_str;
            no_at_cmds = lte_pwr_down_at_cmd_str_size;
            break;
        default:
            printf("\n Don't support the AT test for %d", at_test);
            break;
    }
    testname(tname);
    
    for (ix = 0; ix < no_at_cmds; ix++)
    {
        if (at_cmd[ix].str != NULL) {
            if ( ix == (no_at_cmds - 1)) {
                printmsg = 1;
            } else {
                printmsg = 0;
            }


            retval = process_at_cmd(fd, &at_cmd[ix], printmsg, at_test);
            if (retval == FAILED) {
                close(fd);
                return (retval);
            }
        }
    }
    close(fd);
    return (retval);
}

/*******************************************************************************
 * Function   :    insert_test_module
 * Description:    This function is to insert testing driver(SierraNet and Sierra) 
 *                 if user choose to run non traffic test. 
 * Inputs     :    TRUE for Traffic mode; FALSE for Test mode 
 * Outputs    :    none 
 *
 ******************************************************************************
 */
void insert_test_module (boolean mode)
{
     if (traffic_mode == TRUE) {
         printf("Status : Traffic mode\n");
         return;
     } else { 
         printf("Status : Test mode\n");
     }

     if (mode == TRUE) {
         system(IN_TEST1_KO);
         system(IN_TEST2_KO);
     } else {
         system(RM_TEST1);
         system(RM_TEST2);
     }
}

/*******************************************************************************
 * Function   :    insert_traffic_module
 * Description:    This function is to insert testing driver(GobiNet and GobiSerial) 
 *                 if user choose to run traffic test. 
 * Inputs     :    TRUE for Traffic mode; FALSE for Test mode 
 * Outputs    :    none 
 *
 ******************************************************************************
 */
void insert_traffic_module (boolean mode)
{
     if (mode == TRUE) {
         traffic_mode = TRUE;
         system(IN_TRAFFIC1_KO);
         system(IN_TRAFFIC2_KO);
     } else {  
         traffic_mode = FALSE;
         system(RM_TRAFFIC1);
         system(RM_TRAFFIC2);
     }
}

/******************************************************************************
 *
 * Function   : lte_subsystem_test
 * Description: This function is only for [Motherboard test -> lte test]:
 *              if user choose to show all test items, prompt lte subtest menu
 * Inputs     : do_autotest_only = 0 show submenu, !=0 perform auto test item
 * Outputs    : PASSED/FAILED
 *
 ******************************************************************************
 */
int lte_subsystem_test (int do_autotest_only)
{
    uint reg_val = 0;
    int modem_found = FALSE, ret = PASSED, ix;

    if (this_is_star_c1109_2p() || this_is_supernova_c959_2p()) {
        /* Read FPGA interface LTE control register. */
        if (fpga_read_32_reg(FPGA_LTE_CTL_REG, &reg_val) != PASSED) {
            printf("%s: Failed to read FPGA reg.(0x%04X).\n",
                   __FUNCTION__, FPGA_LTE_CTL_REG);
            return (FAILED);
        }
        
        /* USB Mux Output enable */
        reg_val &= (uint)(~LTE_USB_MUX_DISABLE);
        
        /* Write the LTE control register to the corresponding register bit. */
        if (fpga_write_32_reg(FPGA_LTE_CTL_REG, reg_val) != PASSED) {
            printf("%s: Failed to write FPGA reg.(0x%04X).\n",
                   __FUNCTION__, FPGA_LTE_CTL_REG);
            return (FAILED);
        }
        /* Wait for 1 sec to detect */
        sleep (wait1sec);
        
        /* Get the USB port number */ 
        sprintf(usb_port, "%s:%s", USB2P0_PORT, USB_AT_CMD_PORT);
    } else {
        /* Get the USB port number */ 
        sprintf(usb_port, "%s:%s", USB3P0_PORT, USB_AT_CMD_PORT);
    }

    printf("init LTE modem, please wait....\n");
    insert_test_module(TRUE);
    /* Do the LTE reset initialization sequence. */
    if (this_is_star_c1109_2p() || this_is_supernova_c959_2p()) {
        ret = lte_wp_reset_init();
    } else {
        ret = lte_reset_init();
    }
    
    if (ret == FAILED) {
        insert_test_module(FALSE); 
        return (FAILED);
    }
    
    if (this_is_star_c1109_2p() || this_is_supernova_c959_2p()) {
        printf("Probing the modem in USB 2.0 mode ...");
        fflush(stdout);

        for (ix = 0; ix < PROBE_LTE_USB_TOUT; ix++) {
            if (lte_host_usb_detect(MODEM_SWI_USB_VID, 
                                    USB2P0_SPEED) == PASSED) {
                modem_found = TRUE;
                break;
            }
            msleep(LTE_POLLING_DELAY);
        }

        if (modem_found == TRUE) {
            printf("OK\n");
        } else {
            cterr('f', 0, "SWI Modem is not detected");
            ret = FAILED;
            goto __exit;
        }
    }

    build_primary_submenu(lte_menu_table, LTE_MENU_TABLE_SZ, 
                                "LTE Test Menu", &lte_menup); 
    build_secondary_submenu(lte_menu_table, LTE_MENU_TABLE_SZ, 
                                  lte_sec_items); 
                                   
    if (do_autotest_only) { 
        menu(lte_menup, lte_sec_items, 0); 
        if (!(this_is_star_c1109_2p() || this_is_supernova_c959_2p())) {
            insert_test_module(FALSE);
            return (PASSED);
        }
    } else { 
        /* avoid to ask user memory size in test_patterns. 
         * Select memory size automatically,  
         * and process infinite test. 
         * For now is only march_C test. 
         */ 
        donot_query = TRUE;             
        if (!exec_doall_menu_items(lte_menup)) {
             /* 
              * User did <BREAK>.  Display accumulated errors here only if 
              * not a continuous run because display will occur in menu() as 
              * a result of <BREAK>. 
              */ 
             if (!(DIAGFLAG & D_CONTINUOUS)) { 
                donot_query = TRUE;             
                menu_pr_err_accum(); 
             } 
             if (monjmpptr) { 
                longjmp(*monjmpptr, 1);  /* Back to previous point */ 
             } 
         } 
    }
    
    if (this_is_star_c1109_2p() || this_is_supernova_c959_2p()) {
        ret = PASSED;
        goto __exit;
    }
    
    insert_test_module(FALSE);
    return (PASSED);

__exit:
    if (modem_found == TRUE) {
        if (lte_wp_modem_pwr_off_ctrl() == FAILED) {
            ret = FAILED;
            cterr('f', 0, "Failed to soft power-off SWI modem");
        }
    }

    insert_test_module(FALSE);
    return (ret);

}

/************************************************************************
 * Function: lte_reset_init()
 * Description : Do the LTE reset initialization sequence
 *
 * Input: None
 *
 * Output: PASSED/FAILED
 *************************************************************************
 */
int lte_reset_init (void)
{
    uint data;
    char cmd[MAX_COMMAND_LENGTH]={0};

    /* Disable to print kernel messages on console (but to syslog) while testing */
    sprintf(cmd, DISABLE_CONSOLE_MSG);
    system(cmd);
    msleep(LTE_DELAY_SYSCMD);

    /*
     * Check SKUs to do the LTE modem power on/off timing initialization.
     * Please check LTE modem 74xx data sheet, Figure 5-2 and Table 5-6/5-7
     * for more details.
     */

    fpga_read_32_reg(FPGA_LTE_CTL_REG, &data);
    /* Do Power-off sequence, Full_Card_Power_Off# goes low */
    data &= ~(LTE_PRI_MODEM_EN_CTL);
    fpga_write_32_reg(FPGA_LTE_CTL_REG, data);

    /* (CDETS:CSCvu72089)Base on SWI FAE feedback
     * The Product technical specification says that you should wait at least
     * t_pwr_off_seq seconds before removing the VCC  t_pwr_off_seq is typically
     * 20 seconds and can be upto 25 seconds  CISCO need to increase this 
     * time to at least 25 seconds. */
    sleep(wait25sec);

    fpga_read_32_reg(FPGA_LTE_CTL_REG, &data);
    /* VCC goes low */
    data &= ~(LTE_PRI_POWER_EN_CTL);
    fpga_write_32_reg(FPGA_LTE_CTL_REG, data);

    /* Needs some delay time to toggle VCC. */
    sleep(wait5sec);

    /* Do Power-on sequence, VCC goes high */
    fpga_read_32_reg(FPGA_LTE_CTL_REG, &data);
    data |= (LTE_PRI_POWER_EN_CTL);
    fpga_write_32_reg(FPGA_LTE_CTL_REG, data);
    sleep(wait5sec);

    /* Full_Card_Power_Off# goes high */
    fpga_read_32_reg(FPGA_LTE_CTL_REG, &data);
    data |= (LTE_PRI_MODEM_EN_CTL);
    fpga_write_32_reg(FPGA_LTE_CTL_REG, data);
    
    /* (CDETS:CSCvu72089)Base on SWI FAE feedback
     * The Product technical specification says that you should wait at least
     * t_pwr_off_seq seconds before removing the VCC  t_pwr_off_seq is typically
     * 20 seconds and can be upto 25 seconds  CISCO need to increase this 
     * time to at least 25 seconds. */
    /*
     * The t_pwr_on_seq needs at least 25 seconds,
     * and Linux LTE modem driver will do the enumeration,
     * after that, can open tty to send the AT commands.
     */
    sleep(wait25sec);
    
    /* Reset the LTE Modem */
    if (fpga_reset_32_api(FPGA_EXTER_DEV_RST_REG,
                          EXT_PRI_LTE_RESET, TRUE,
                          WAITTIME_150_MS) == FAILED) {
        cterr('f', 0, "Reset the LTE modem fails!");
        return (FAILED);
    }
    /* Un-reset the LTE Modem */
    if (fpga_reset_32_api(FPGA_EXTER_DEV_RST_REG,
                          EXT_PRI_LTE_RESET, FALSE,
                          WAITTIME_150_MS) == FAILED) {
        cterr('f', 0, "Reset the LTE modem fails!");
        return (FAILED);
    }
    
    /* LTE Control Register */
    if (fpga_reset_32_api(FPGA_LTE_CTL_REG,
                          EXT_PRI_LTE_WDIS_1_RESET | EXT_PRI_LTE_WDIS_2_RESET,
                          FALSE,
                          WAITTIME_150_MS) == FAILED) {
        cterr('f', 0, "Unoperational the LTE modem fails!");
        return (FAILED);
    }

    /* Operational the LTE Modem */
    if (fpga_reset_32_api(FPGA_LTE_CTL_REG,
                          EXT_PRI_LTE_WDIS_1_RESET | EXT_PRI_LTE_WDIS_2_RESET,
                          TRUE,
                          WAITTIME_150_MS) == FAILED) {
        cterr('f', 0, "Operational the LTE modem fails!");
        return (FAILED);
    }

    /* Enable print kernel messages on console */
    sprintf(cmd, ENABLE_CONSOLE_MSG);
    system(cmd);
    msleep(LTE_DELAY_SYSCMD);

    return (PASSED);
}

/************************************************************************
 * Function: lte_wp_reset_init()
 * Description : Do the LTE reset initialization sequence (WP series)
 *
 * Input: None
 *
 * Output: PASSED/FAILED
 *************************************************************************
 */
int lte_wp_reset_init (void)
{
    uint data;
    char cmd[MAX_COMMAND_LENGTH]={0};

    /* Disable to print kernel messages on console (but to syslog) while testing */
    sprintf(cmd, DISABLE_CONSOLE_MSG);
    system(cmd);
    msleep(LTE_DELAY_SYSCMD);

    /*
     * Check SKUs to do the LTE modem power on/off timing initialization.
     * Please check LTE modem 74xx data sheet, Figure 5-2 and Table 5-6/5-7
     * for more details.
     */
    /* Un-reset the LTE Modem */
    if (fpga_reset_32_api(FPGA_EXTER_DEV_RST_REG,
                          EXT_PRI_LTE_RESET, FALSE,
                          WAITTIME_150_MS) == FAILED) {
        cterr('f', 0, "Reset the LTE modem fails!");
        return (FAILED);
    }

    /* Operational the LTE Modem */
    if (fpga_reset_32_api(FPGA_LTE_CTL_REG,
                          (EXT_PRI_LTE_WDIS_1_RESET | EXT_PRI_LTE_WDIS_2_RESET), TRUE,
                          WAITTIME_150_MS) == FAILED) {
        cterr('f', 0, "Operational the LTE modem fails!");
        return (FAILED);
    }

    /* Do Power-on sequence, VCC goes high */
    fpga_read_32_reg(FPGA_LTE_CTL_REG, &data);
    data |= (LTE_PRI_POWER_EN_CTL);
    fpga_write_32_reg(FPGA_LTE_CTL_REG, data);

    /* Full_Card_Power_Off# goes high */
    fpga_read_32_reg(FPGA_LTE_CTL_REG, &data);
    data |= (LTE_PRI_MODEM_EN_CTL);
    fpga_write_32_reg(FPGA_LTE_CTL_REG, data);

    msleep(WP_PWR_ON_DELAY);

    /* Enable print kernel messages on console */
    sprintf(cmd, ENABLE_CONSOLE_MSG);
    system(cmd);
    msleep(LTE_DELAY_SYSCMD);

    return (PASSED);
}

/**********************************************************************
 *
 * Function: modem_gen_at_cmd_test
 *
 * Description:
 *
 * Input : none
 *
 * Output: PASSED - No errors encountered.
 *         FAILED - Errors encountered.
 *
 **********************************************************************
 */
static int modem_gen_at_cmd_test (void)
{
    const int maxlen = 128;
    char usb_tty_dev[maxlen];
    char cmd[maxlen];

    /* Polling modem detection, in case modem is not ready */
    if (lte_check_modem_rdy(0) != PASSED) {
        cterr('f', 0, "LTE module is not ready");
        return (FAILED);
    }

    printf("\n\n ### NOTE: Type CTRL-x "
                              "to switch back to host's console\n\n");
    fflush(stdout);
    fflush(stderr);
    /* pause a second for the NOTE: */
    sleep(wait1sec);

    /* Dynamically get the corresponding ttyUSB number in case usb device
     * attaches to different ttyUSB 
     */     
    if (lte_get_tty_num(usb_tty_dev) != PASSED) {
        printf("%s:Can't get ttyUSB number\n", __func__);
        return (FAILED);
    }

    snprintf(cmd, maxlen-1, "microcom %s%s", USB_TTY_PATH, usb_tty_dev);

    system(cmd);

    return (PASSED);
}
/**********************************************************************
 *
 * Function: usb_lte_test 
 *
 * Description: This function test micro-usb debug port path
 *
 * Input : r_opt - dummy 
 *
 * Output: PASSED - No errors encountered.
 *         FAILED - Errors encountered.
 *
 **********************************************************************
 */
int usb_lte_test (boolean r_opt) 
{
    uint reg_val = 0;
    char buffer[LENGTH100];
    boolean dummy = 0;
    boolean temp = FALSE;
    int ret = 0;

    dummy = r_opt;
    sprintf(buffer, "Please connect micro usb to TSN usb port!!\n Continue [y/n]");
    /* Read FPGA interface LTE control register. */
    if (fpga_read_32_reg(FPGA_LTE_CTL_REG, &reg_val) != PASSED) {
        printf("%s: Failed to read FPGA reg.(0x%04X).\n",
               __FUNCTION__, FPGA_LTE_CTL_REG);
        return (FAILED);
    }

    if ( getc_answer(buffer,"yn",'n') == 'y' ) {
        temp = TRUE;
    } else {
        temp = FALSE;
    }

    if (temp == TRUE) {
        reg_val |= LTE_USB_MUX_SEL_CTL;
    } else {
        reg_val &= (uint)(~LTE_USB_MUX_SEL_CTL);
    }
 
    /* USB Mux Output enable */ 
    reg_val &= (uint)(~LTE_USB_MUX_DISABLE);
    /* Write the LTE control register to the corresponding register bit. */
    if (fpga_write_32_reg(FPGA_LTE_CTL_REG, reg_val) != PASSED) {
        printf("%s: Failed to write FPGA reg.(0x%04X).\n",
               __FUNCTION__, FPGA_LTE_CTL_REG);
        return (FAILED);
    }
    /* Wait for 1 sec to detect */
    sleep (wait1sec);
    /* Check CPU detect mini-usb connection port */ 
    if (temp == TRUE) {
        /* Show mini-usb enable. */
        ret = usb_debugport_test(USB_SLOT0);
        if (ret == FAILED)
            cterr('f', 0, "Switch to Debug port fail");
        goto restore;
    } else {
        /* Show mini-usb disable */
        goto restore;
    }

restore:    
    /* Restore back the path back to CPU */
    /* Read FPGA interface LTE control register. */
    if (fpga_read_32_reg(FPGA_LTE_CTL_REG, &reg_val) != PASSED) {
        printf("%s: Failed to read FPGA reg.(0x%04X).\n",
               __FUNCTION__, FPGA_LTE_CTL_REG);
        return (FAILED);
    }
    reg_val &= (uint)(~LTE_USB_MUX_SEL_CTL);

    /* USB Mux Output enable */ 
    reg_val &= (uint)(~LTE_USB_MUX_DISABLE);
    /* Write the LTE control register to the corresponding register bit. */
    if (fpga_write_32_reg(FPGA_LTE_CTL_REG, reg_val) != PASSED) {
        printf("%s: Failed to write FPGA reg.(0x%04X).\n",
               __FUNCTION__, FPGA_LTE_CTL_REG);
        return (FAILED);
    }
    /* Wait for 1 sec to detect */
    sleep (wait1sec);
    /* Set mini-usb disable */
    if ((usb_dump_x(USB_SLOT1)) != PASSED) {
        return (FAILED);
    } else {
        return (PASSED);
    }
}

/**********************************************************************
 *
 * Function: usb_lte_utility 
 *
 * Description: This function to provide utility for LTE debug port
 *
 * Input : r_opt (TRUE; CPU --> LTE --> External Micro USB) 
 *                FALSE; CPU --> External Micro USB --> LTE) 
 *
 * Output: PASSED - No errors encountered.
 *         FAILED - Errors encountered.
 *
 **********************************************************************
*/
int usb_lte_utility (boolean r_opt) 
{
    uint       reg_val = 0;

    /* Read FPGA interface LTE control register. */
    if (fpga_read_32_reg(FPGA_LTE_CTL_REG, &reg_val) != PASSED) {
        printf("%s: Failed to read FPGA reg.(0x%04X).\n",
               __FUNCTION__, FPGA_LTE_CTL_REG);
        return (FAILED);
    }

    if (r_opt == TRUE) {
        /* Set switch USB mux to Connector. */
        printf("\nPlease connect micro usb to External USB 0\n");
        reg_val |= LTE_USB_MUX_SEL_CTL;
    } else if (r_opt == FALSE) {
        /* Set switch USB mux to CPU */
        printf("\nRemove cable from External USB 0\n");
        reg_val &= (uint)(~LTE_USB_MUX_SEL_CTL);
    } else {
        printf("%s: Invalid  option(%#x).\n", __FUNCTION__, r_opt);
        return (FAILED);
    }
             
    /* USB Mux Output enable */ 
    reg_val &= (uint)(~LTE_USB_MUX_DISABLE);
    /* Write the LTE control register to the corresponding register bit. */
    if (fpga_write_32_reg(FPGA_LTE_CTL_REG, reg_val) != PASSED) {
        printf("%s: Failed to write FPGA reg.(0x%04X).\n",
               __FUNCTION__, FPGA_LTE_CTL_REG);
        return (FAILED);
    }
    
    return (PASSED);

}

/**********************************************************************
 *
 * Function: usb_20_detection_test 
 *
 * Description: This function test CPU[2.0 USB] <-> MUX <-> LTE modem
 *              full path
 *
 * Input : None 
 *
 * Output: PASSED - No errors encountered.
 *         FAILED - Errors encountered.
 *
 **********************************************************************
 */
int usb_20_detection_test (void) 
{
    uchar mb_get_loc[FRU_SIZE] = {0};
    uchar mb_get_pid[FRU_SIZE] = {0};
#ifdef ENHANCE_ERROR_MSG_RDY
    /*
     * 1. Subtests of the test function will reuse all variables
     * 2. All variables will be cleared automatically when
     *    entering and leaving each menu item.
     */
    /* Segment 1: PID | Unique_string : slot_info */
    fru_table_offset = MB;
    /* fru_table_offset should be set, otherwise, it will not */
    /* go to enhanced error message format in cterr() */
    /* set fru_table_offset to get the predefine value */
    /* or change mb_pid & mb_loc below */
    platform_get_pid((char *)mb_get_pid);
    strcpy((char *)mb_get_loc, "MB");
    platform_fru_table[fru_table_offset].pid_string = mb_get_pid;
    platform_fru_table[fru_table_offset].location_string = mb_get_loc;

    /* Segment 2: Test step captured from prpass */
    /* Segment 3: Failure message captured from cterr */

    /* Segment 4: Components used */
    cterr_add_component("Marvell Armada 7040", "USB 2.0/3.0", "Sierra Wireless EM7455 or EM7430 WAN Modem");

    /* Segment 5: register and memory dump */
    //cterr_add_reg_dump((PFV)bcm63168_get_xdsl_profile);

    /* Segment 6: Platform Environment initialized here*/
    cterr_add_env_dump((PFV)tsn_display_temp_errormsg);

    /* Segment 7: Top 3 Debugging Steps */
    cterr_add_debug("Check the USB 2.0/3.0 interface between the Host SoC "
                    "and the LTE module.",
                    "If there is no problem for these USB 2.0/3.0 interfaces, "
                    "replace one module and redo the test.",
                    "If test is still failed, contact "
                    "module vendor for support");
#endif
    char *tname = "CPU USB2.0 to LTE Detection";

    testname("%s", tname);
    prpass(testpass, "%s, ", tname);
    uint reg_val = 0;

    printf("Change to 2.0 Speed...\n");    
    /* Change speed to 2.0 */
    /* Tx idle on and Rx terminations off (Will disable Serdes and
     * the port will work only in USB2.0*/
    /* Set bit 10 to 0x1 (Force idle from register) --> TX */ 
    tsn_mem_write32(USB1_CUSTOM_REG1, USB20_CUSTOM_REG1_VAL);
    msleep(200);
    /* set bit15 to 0x1(force Tx idle) -> TX  */
    tsn_mem_write32(USB1_CUSTOM_REG2, USB20_CUSTOM_REG2_VAL);
    msleep(200);
    /* set bit9 to 0x1 - rx highz from register -> RX */
    tsn_mem_write32(USB1_MISC_CTRL_1_REG, USB_MISC_CTRL_1_REG_VAL);
    msleep(200);
    /* set bit2 to 0x1 - rx high z termination -> RX */
    tsn_mem_write32(USB1_CTRL_REG, USB20_CTRL_REG_VAL);
    msleep(200);

    /* Do the LTE reset initialization sequence. */
    if (lte_reset_init() == FAILED) {
        cterr('f', 0, "Reset LTE %s failed", tname);
        prcomplete(testpass, errcount, (char *)0);
        goto restore;
    }
     
    /* Re-enumerate USB Mux Selector to CPU */
    reg_val &= (uint)(~LTE_USB_MUX_SEL_CTL);
 
    /* USB Mux Output enable */ 
    reg_val &= (uint)(~LTE_USB_MUX_DISABLE);
    /* Write the LTE control register to the corresponding register bit. */
    if (fpga_write_32_reg(FPGA_LTE_CTL_REG, reg_val) != PASSED) {
        cterr('f', 0, "%s: Failed to write FPGA reg.(0x%04X).",
               __FUNCTION__, FPGA_LTE_CTL_REG);
        prcomplete(testpass, errcount, (char *)0);
        goto restore;
    }
    
    /* Wait for 1 sec to detect */
    // sleep (wait1sec);
    sleep(5);

    if (usb_get_info() != PASSED) {
        cterr('f', 0, "usb_get_info() failed.");
        prcomplete(testpass, errcount, (char *)0);
        goto restore;
    } else {
        if (usb_get_speed(USB_SLOT1) != USB2) {
            cterr('f', 0,"USB 2.0 setting failed.");
            prcomplete(testpass, errcount, (char *)0);
            goto restore;
        }
        /* Wait for 1 sec to detect */
       // sleep (wait1sec);
        sleep(5);

        /* Show USB speed  */
        if (usb_dump_x(USB_SLOT1) != PASSED) {
            cterr('f', 0,"Cannot Show USB2.0.");
            prcomplete(testpass, errcount, (char *)0);
            goto restore;
        }
    }
    
restore:
    printf("Restore back to 3.0 Speed...\n");    
    /* Change speed to 3.0 */
    /* Tx idle on and Rx terminations off (Will disable Serdes and
     * the port will be able to work in SS */
    /* Set bit 10 to 0x1 (Force idle from pin) --> TX */ 
    tsn_mem_write32(USB1_CUSTOM_REG1, USB30_CUSTOM_REG1_VAL);
    msleep(200);

    /* set bit15 to 0x1(force Tx idle) -> TX  */
    tsn_mem_write32(USB1_CUSTOM_REG2, USB30_CUSTOM_REG2_VAL);
    msleep(200);

    /* set bit9 to 0x1 - rx highz from register -> RX */
    tsn_mem_write32(USB1_MISC_CTRL_1_REG, USB_MISC_CTRL_1_REG_VAL);
    msleep(200);

    /* set bit2 to 0x0 - rx normal termination -> RX */
    tsn_mem_write32(USB1_CTRL_REG, USB30_CTRL_REG_VAL);
    msleep(200);

    /* Do the LTE reset initialization sequence. */
    if (lte_reset_init() == FAILED) {
        cterr('f', 0,"Cannot Reset LTE.");
        prcomplete(testpass, errcount, (char *)0);
        return (FAILED);
    }
    
    /* Re-enumerate USB Mux Selector to CPU */
    reg_val &= (uint)(~LTE_USB_MUX_SEL_CTL);
 
    /* USB Mux Output enable */ 
    reg_val &= (uint)(~LTE_USB_MUX_DISABLE);
    /* Write the LTE control register to the corresponding register bit. */
    if (fpga_write_32_reg(FPGA_LTE_CTL_REG, reg_val) != PASSED) {
        cterr('f', 0,"%s: Failed to write FPGA reg.(0x%04X).",
               __FUNCTION__, FPGA_LTE_CTL_REG);
        prcomplete(testpass, errcount, (char *)0);
        return (FAILED);
    }
    
    /* Wait for 1 sec to detect */
    //sleep (wait1sec);
    sleep (5);

    if (usb_get_info() != PASSED) {
        cterr('f', 0,"usb_get_info() failed.");
        prcomplete(testpass, errcount, (char *)0);
        return (FAILED);
    } else {
        if (usb_get_speed(USB_SLOT1) != USB3) {
            cterr('f', 0,"USB 3.0 setting failed.");
            prcomplete(testpass, errcount, (char *)0);
        return (FAILED);
    }
        /* Wait for 1 sec to detect */
       // sleep (wait1sec);
        sleep (5);

        /* Show USB speed  */
        if (usb_dump_x(USB_SLOT1) != PASSED) {
            cterr('f', 0,"Cannot Show USB 3.0.");
            prcomplete(testpass, errcount, (char *)0);
            return (FAILED);
        }
    }

    return (PASSED);
    
}

/*******************************************************************************
 * Function   : lte_check_modem_rdy 
 * Description: Function to check whether modem is ready for opening ttyUSB to 
 *              transmit AT command
 * Inputs     : input - Not used 
 * Outputs    : PASSED or FAILED
 *******************************************************************************
 */
static int lte_check_modem_rdy (int input)
{
    int ix;
    int stat = FAILED, fdd;
    char usb_tty_dev[256];
    char usb_tty[15];
    
    /* Check if the usb port which is used for transmitting AT command is 
     * attached to tty device successfully */
    printf("\nCheck modem usb device status");

    /* Dynamically get the corresponding ttyUSB number in case usb device
     * attaches to different ttyUSB 
     */
    for (ix = 0; ix < LTE_CHK_MAX_RETRY_TIME; ix++) {
        /* Delay 500 ms in case ttyUSB resource is always occupied */ 
        msleep(LTE_CHK_TTY_STAT_DELAY);
        if (lte_get_tty_num(usb_tty_dev) == PASSED) {
            break;
        }

        printf(".");
        fflush(stdout);
    }

    if (ix == LTE_CHK_MAX_RETRY_TIME) {
        return (FAILED);
    }
    
    sprintf(usb_tty, "%s%s", USB_TTY_PATH, usb_tty_dev);
    /* Wait 500 ms before access tty device */
    msleep(TTY_ACCESS_DELAY);
    
    for (ix = 0; ix < USB_TTY_TOUT; ix++) {
        if (access(usb_tty, F_OK) != -1) {
            fdd = open(usb_tty, O_RDWR | O_NOCTTY | O_NDELAY);
            if ( fdd != MODEN_ERR ) {
                printf("OK\n");
                stat = PASSED;
                break;
            }
        }
        msleep(10);
    }
    close(fdd);
    return (stat);
}

/*******************************************************************************
 * Function   : lte_gps_pin_test 
 * Description: Function to check GPS pin value 
 * Inputs     : input - Not used 
 * Outputs    : PASSED or FAILED
 *******************************************************************************
 */
static int lte_gps_pin_test (int input)
{
#ifdef GPS_PIN_TEST_W_ANTENNA    
    int ix, at_cmd_test;
    struct timeval t_start, t_curr;
    uint value, cost_time = 0;
#endif 

    testname("GPS Pin Test");
    prpass(testpass, "GPS Pin Test");

#ifdef GPS_PIN_TEST_W_ANTENNA 

    /* Polling modem detection, in case modem is not ready */
    if (lte_check_modem_rdy(0) != PASSED) {
        cterr('f', 0, "LTE module is not ready");
        return (FAILED);
    }
    
    /* Enable GPS */
    at_cmd_test = LTE_GPS_ENABLE;
    printf("Enabling GPS...\n");
    if (run_at_cmd(at_cmd_test) != PASSED) {
        cterr('f', 0, "Failed to enable GPS");
        return (FAILED);
    }

    /* Need some time to start reset, check if modem is in reset mode */
    stat = lte_check_modem_rdy(0);
    if (stat != PASSED) {
        cterr('f', 0, "LTE failed to reset");
        return (FAILED);
    } else {
        stat = FAILED;
    }

    /* Polling modem detection, in case modem is not ready */
    if (lte_check_modem_rdy(0) != PASSED) {
        cterr('f', 0, "LTE is not ready");
        return (FAILED);
    }
    
    /* Enable GPS DR_SYNC feature */
    at_cmd_test = LTE_GPS_DR_SYNC_TEST;
    printf("Enable GPS Dead Reckoning Synchronization feature.\n");
    if (run_at_cmd(at_cmd_test) != PASSED) {
        cterr('f', 0, "Failed to enable GPS DR feature");
        return (FAILED);
    }

    /* Check the current status of GPS position fix to see 
     * whether we can get GPS fixes or not */
    printf("Polling for GPS position fixes...\n");
    at_cmd_test = LTE_GPS_FIXES_STATUS;
    for (ix = 0; ix < MAX_RETRY_TIME; ix ++) {
        stat = run_at_cmd(at_cmd_test);
        if (stat == PASSED) {
            printf("Got GPS fixes.\n");
            break;
        }
        msleep(10);
    }
    if (stat != PASSED) {
        cterr('f', 0, "Failed to get GPS fixes");
        return (FAILED);
    } else {
        stat = FAILED;
    }

    /* Polling Sirius FPGA for GPS DR_SYNC pulse */
    printf("Polling for GPS DR_SYNC pulse.\n");
    gettimeofday(&t_start, NULL);
    while (cost_time < MAX_POLLING_TIME) {
        fpga_read_32_reg(FPGA_LTE_CTL_REG, &value);
        if (value & GPS_DR_SYNC_STATUS) {
            printf("Got GPS dead reckoning pulse.\n");
            stat = PASSED;
            break;
        }
        gettimeofday(&t_curr, NULL);
        cost_time = (t_curr.tv_sec - t_start.tv_sec);
    }
    if (stat != PASSED) {
        cterr('f', 0, "GPS Pin Test Failed");
        return (FAILED);
    }
#else   /* Test with diagnostic mode */ 
    /* Force GPS DR_SYNC high */
    if (lte_force_gps_pin_val(HIGH) != PASSED) {
        cterr('f', 0, "Failed to set GPS DR_SYNC signal");
        return (FAILED);
    }

    /* Force GPS DR_SYNC low */
    if (lte_force_gps_pin_val(LOW) != PASSED) {
        cterr('f', 0, "Failed to clear GPS DR_SYNC signal");
        return (FAILED);
    }

    /* Since we use "AT!BSGPIO" command to force the DR_SYNC signal high/low.
     * This procedure is only for diagnostic use and need to reset or power
     * cycle the modem to return normal signal functionality.
     */
    printf("\n!!!!Please power-cycle the modem after running this test!!!!\n");
#endif 
    
    return (PASSED);
}

/*******************************************************************************
 * Function   : lte_force_gps_pin_val 
 * Description: Function to force GPS pin value to high or low 
 * Inputs     : value - HIGH / LOW 
 *              slot - which slot
 * Outputs    : PASSED or FAILED
 *******************************************************************************
 */
static int lte_force_gps_pin_val (int value)
{
    int ix, at_cmd_test;
    uint reg_val;
    int stat = FAILED;

    /* Polling modem detection in case modem is not ready */
    if (lte_check_modem_rdy(0) != PASSED) {
        printf("LTE is not ready\n");
        return (FAILED);
    }
    
    /* Set GPS DR_SYNC value */
    if (value == HIGH) {
        at_cmd_test = LTE_GPS_DR_SYNC_FORCE_HIGH;
        printf("Force GPS Dead Reckoning Synchronize signal to high.\n");
    } else {
        at_cmd_test = LTE_GPS_DR_SYNC_FORCE_LOW;
        printf("Force GPS Dead Reckoning Synchronize signal to low.\n");
    }

     if (run_at_cmd(at_cmd_test) != PASSED) {
        return (FAILED);
    }

    /* Polling Sirius FPGA for GPS DR_SYNC */
    printf("Polling for GPS DR_SYNC.");
    for (ix = 0; ix < MAX_POLLING_TIME; ix++) {
        fpga_read_32_reg(FPGA_LTE_CTL_REG, &reg_val);
        if (reg_val & GPS_DR_SYNC_STATUS) {
                printf("GPS dead reckoning synchronize signal is high.\n");
                stat = PASSED;
                break;
        } else {
            printf("GPS dead reckoning synchronize signal is low.\n");
            stat = PASSED;
            break;
        }
        printf(".");
        msleep(10);
    }

    if (stat != PASSED) {
        printf("Failed to set GPS Pin value\n");
        return (FAILED);
    }
    
    return (PASSED);
}

/***************************************************************************
* Name: lte_fd_selftest
*
* Description: This function sends "AT" command to ensure the communication 
*              between host and modem is good 
* 
* Input: *tty_fd - Pointer to the TTY file descriptor
*
* Output: PASSED/FAILED
***************************************************************************/
static int lte_at_selftest (int fd)
{
    struct timeval timeout;
    char *test_str = "AT";
    char cr = '\r';
    char buffer[AT_CMD_BUFFER_SIZE] = {0,};
    char *bufptr;
    fd_set tout_set;
    int ix, ret, stat = FAILED;
 
    for (ix = 0; ix < MAX_SELFTEST_RETRY; ix++) {
        bufptr = buffer;
 
        FD_ZERO(&tout_set);
        FD_SET(fd, &tout_set);
        timeout.tv_sec  = AT_SELFTEST_TOUT_IN_SEC;
        timeout.tv_usec = 0;
 
        ret = select(fd + 1, &tout_set, NULL, NULL, &timeout);
 
        if (ret) {
            read(fd, bufptr, AT_CMD_BUFFER_SIZE);
 
            if (strstr(buffer, test_str) != 0) {
                stat = PASSED;
                break;
            }
            msleep(AT_SELFTEST_DELAY);
        }
        ret = write(fd, test_str, strlen(test_str));
        if (!ret) {
            printf("SEND AT CMD Fail \n");
        } else {
            write(fd, &cr, 1);
        }
    }

    if (stat != PASSED) {
        printf("%s: Modem communication selftest failed\n", __func__);
        return (FAILED);
    }

    FD_ZERO(&tout_set);
    FD_SET(fd, &tout_set);
    timeout.tv_sec  = AT_SELFTEST_TOUT_IN_SEC;
    timeout.tv_usec = 0;
    
    /* Flush buffer */
    select(fd + 1, &tout_set, NULL, NULL, &timeout);
    if (tcflush(fd, TCIOFLUSH) < 0) {
        printf("Flush buffer failed : %s\n", strerror(errno));
    }

    return (PASSED);
}

/*******************************************************************************
 *
 * Function   : supports_em74xx_lte
 * Description: Function to check if platform supports EM74xx LTE modem.
 * Inputs     : NONE
 * Outputs    : TRUE(yes)/FALSE(no)
 *
 *******************************************************************************
 */
static int supports_em74xx_lte (void)
{
    if (this_is_tsn() == TRUE) {
        return (TRUE);
    }
    return (FALSE);
}

/*******************************************************************************
 *
 * Function   : supports_wp76xx_lte
 * Description: Function to check if platform supports WP76xx LTE modem.
 * Inputs     : NONE
 * Outputs    : TRUE(yes)/FALSE(no)
 *
 *******************************************************************************
 */
static int supports_wp76xx_lte (void)
{
    if (this_is_star_c1109_2p() || this_is_supernova_c959_2p()) {
        return (TRUE);
    }
    return (FALSE);
}

/*******************************************************************************
 * Function   : lte_has_2_sim_slot
 * Description: Function to check whether LTE has two sim slot
 * Inputs     : None
 * Outputs    : TRUE/FALSE
 *******************************************************************************
 */
boolean lte_has_2_sim_slot (void)
{
    ushort ctype;

    /* Read controller type from cookie */
    ctype = get_control_type();

    /* WP7601 only has 1 sim slot */
    if ((ctype == C959_2PVZ_CONTROL_TYPE) || 
        (ctype == C1109_2PVZ_CONTROL_TYPE)) {
        return (FALSE);
    } else {
        return (TRUE);
    }
}

/*******************************************************************************
 *
 * Function   : display_em74xx_simdetect_stat_util
 * Description: Wrapped utility to display LTE EM74xx SIM_DETECT pin state.
 *              This function gets EM74xx LTE SIM_DETECT/SIM_DETECT_2 pin state
 *              by use AT!BSGPIO AT command, then display the read back state.
 * Inputs     : sim_num - SIM number(0/1)
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
static int display_em74xx_simdetect_stat_util (int sim_num)
{
    int at_cmd_set = 0;

    /* Configure parameters based on testing SIM number */
    switch (sim_num) {
    case SIM0:
        at_cmd_set = EM74XX_SIMDETECT_STAT;
        break;
    case SIM1:
        at_cmd_set = EM74XX_SIMDETECT2_STAT;
        break;
    default:
        printf("%s(%d) Unsupported SIM number: %d\n",
               __func__, __LINE__, sim_num);
        return (FAILED);
    }

    /* Send AT command to get the state of EM74xx LTE modem SIM_DETECT pin */
    if (run_at_cmd(at_cmd_set) != PASSED) {
        printf("\nFailed to get SIM%d SIM_DETECT state.\n", sim_num);
        return (FAILED);
    }

    printf("\nSIM%d SIM_DETECT pin current state: %d.\n",
           sim_num, em74xx_simdetect_state);
    return (PASSED);
}

/*******************************************************************************
 *
 * Function   : em74xx_simdetect_pin_test
 * Description: Wrapped function to test LTE EM74xx SIM_DETECT pin.
 *              This function is to test EM74xx LTE SIM_DETECT/SIM_DETECT_2 pin
 *              by check if the state that AT!BSGPIO read back is as expected.
 *              Besides, this function also provides usr_prompt parameter for
 *              user prompt display enable/disable.
 * Inputs     : sim_num - SIM number(0/1)
 *              exp_sim_stat - Expected SIM status: PRESENT(1)/NOT_PRESENT(0)
 *              usr_prompt - To ENABLE/DISABLE user prompt
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
static int em74xx_simdetect_pin_test (int sim_num, boolean exp_sim_stat,
                                      boolean usr_prompt)
{
    char usr_input = 0;
    char usr_act_str[LTE_TESTMSG_BUFSZ];
    int at_cmd_test = 0;

    memset(usr_act_str, 0, sizeof(usr_act_str));

    /* Based on TSN design,
     * If SIM card is present, LTE SIM_DETECT signal should be HIGH.
     * If SIM card is removed, LTE SIM_DETECT signal should be LOW.
     */

    /* Configure parameters based on testing SIM number */
    switch (sim_num) {
    case SIM0:
        if (exp_sim_stat == SIM_PRESENT) {
            sprintf(usr_act_str, "install SIM card to");
            /* SIM0 is inserted, SIM_DETECT should be HIGH */
            at_cmd_test = EM74XX_SIMDETECT_H;
        } else {
            sprintf(usr_act_str, "remove SIM card from");
            /* SIM0 is removed, SIM_DETECT should be LOW */
            at_cmd_test = EM74XX_SIMDETECT_L;
        }
        break;
    case SIM1:
        if (exp_sim_stat == SIM_PRESENT) {
            sprintf(usr_act_str, "install SIM card to");
            /* SIM1 is inserted, SIM_DETECT_2 should be HIGH */
            at_cmd_test = EM74XX_SIMDETECT2_H;
        } else {
            sprintf(usr_act_str, "remove SIM card from");
            /* SIM1 is removed, SIM_DETECT_2 should be LOW */
            at_cmd_test = EM74XX_SIMDETECT2_L;
        }
        break;
    default:
        printf("%s(%d) Unsupported SIM number: %d\n",
               __func__, __LINE__, sim_num);
        return (FAILED);
    }

    /* Print out user prompt if needed */
    if (usr_prompt == ENABLE) {
        printf("\n\n### Please %s SIM slot %d.\n", usr_act_str, sim_num);
        do {
            printf("\r### Press 'y' to continue the Test: ");
            usr_input = getchar();
            if (usr_input == 'y') {
                break;
            }
        } while (usr_input != 'y');
    }

    /* Confirm the status of EM74xx LTE modem SIM_DETECT pin */
    if (run_at_cmd(at_cmd_test) != PASSED) {
        printf("%s(%d) SIM_DETECT pin status is not as expected.\n",
               __func__, __LINE__);
        return (FAILED);
    }
    return (PASSED);
}

/*******************************************************************************
 *
 * Function   : lte_em74xx_simdetect_pin_test_insertion_wrapper
 * Description: Wrapped function to for LTE EM74xx SIM_DETECT pin test (Insertion).
 * Inputs     : sim_num - SIM number(0/1)
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
static int lte_em74xx_simdetect_pin_test_insertion_wrapper (int sim_num)
{
    return lte_em74xx_simdetect_pin_test(sim_num, SIM_PRESENT);
}

/*******************************************************************************
 *
 * Function   : lte_em74xx_simdetect_pin_test_removal_wrapper
 * Description: Wrapped function to for LTE EM74xx SIM_DETECT pin test (Removal).
 * Inputs     : sim_num - SIM number(0/1)
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
static int lte_em74xx_simdetect_pin_test_removal_wrapper (int sim_num)
{
    return lte_em74xx_simdetect_pin_test(sim_num, SIM_NOT_PRESENT);
}

/*******************************************************************************
 *
 * Function   : lte_em74xx_simdetect_pin_test
 * Description: Wrapped function to test LTE EM74xx SIM_DETECT pin.
 * Inputs     : sim_num - SIM number(0/1)
 *              operation - SIM_PRESENT or SIM_NOT_PRESENT
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
static int lte_em74xx_simdetect_pin_test (int sim_num, int operation)
{
    /* CSCvk03740: TSN LTE SIM0 SIM_DETECT pin issue.
     *
     * This utility test is to enhance the coverage of EM74xx LTE SIM_DETECT pin.
     * It requires users to insert and remove SIM card during the test.
     *
     * Based on comment from SWI(Sierra wireless) on EM74xx:
     *   - AT!ENTERCND="A710" is required to enable the AT!BSGPIO command.
     *   - AT!BSGPIO?77 can be used to check the state of SIM_DETECT signal.
     *   - AT!BSGPIO?15 can be used to check the state of SIM_DETECT_2 signal.
     */ 

    char test_name[LTE_TESTMSG_BUFSZ];

    memset(test_name, 0, sizeof(test_name));

    /* Configure parameters based on testing SIM number */
    switch (sim_num) {
    case SIM0:
        if (operation == SIM_PRESENT) {
            sprintf(test_name, "EM74xx LTE SIM_DETECT pin (Insertion)");
        }
        else {
            sprintf(test_name, "EM74xx LTE SIM_DETECT pin (Removal)");
        }    
        break;
    case SIM1:
        if (operation == SIM_PRESENT) {
            sprintf(test_name, "EM74xx LTE SIM_DETECT_2 pin (Insertion)");
        }
        else {
            sprintf(test_name, "EM74xx LTE SIM_DETECT_2 pin (Removal)");
        }
        break;
    default:
        cterr('f', 0, "Failed, got unsupported SIM number: %d ", sim_num);
        return (FAILED);
    }

    testname(test_name);
    prpass(testpass, "SIM%d, ", sim_num);

    /* Test SIM_DETECT pin */
    if (em74xx_simdetect_pin_test(sim_num, operation, ENABLE) != PASSED) {
        if (operation == SIM_PRESENT) {
            cterr('f', 0, "Failed, SIM%d is inserted "
                        "but SIM_DETECT state is Low.", sim_num);
            return (FAILED);
        }
        else {
            cterr('f', 0, "Failed, SIM%d is NOT inserted "
                        "but SIM_DETECT state is High.", sim_num);
            return (FAILED);
        }
    }

    prpass(testpass, "SIM%d, ", sim_num);
    return (PASSED);
}

/*******************************************************************************
 *
 * Function   : display_wp76xx_simdetect_stat_util
 * Description: Wrapped utility to display LTE WP76xx SIM_DETECT pin state.
 *              This function gets WP76xx LTE SIM_DETECT/SIM_DETECT_2 pin state
 *              by use AT!BSGPIO AT command, then display the read back state.
 * Inputs     : sim_num - SIM number(0/1)
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
static int display_wp76xx_simdetect_stat_util (int sim_num)
{
    int at_cmd_set = 0;
    uint data;

    /* Configure parameters based on testing SIM number */
    switch (sim_num) {
    case SIM0:
    case SIM1:
        at_cmd_set = WP76XX_UIM1_DET_STAT;
        break;
    default:
        printf("%s(%d) Unsupported SIM number: %d\n",
               __func__, __LINE__, sim_num);
        return (FAILED);
    }

    /* Switch FPGA to select SIM number */
    switch (sim_num) {
    case SIM0:
        /* check if  SIM 0 exists */
        fpga_read_32_reg(FPGA_SIM_STATUS_CTL_REG, &data);
        if (!(data & LTE_SIM_0_PRESENT_DECTECT)) {
            printf("\nNo SIM0 Present!!!"
                  "\nPlease insert the SIM card and reset the modem.\n");
        } else {
            /* Enable SIM 0 */
            fpga_read_32_reg(FPGA_SIM_STATUS_CTL_REG, &data);
            data &= ~LTE_SIM_SOCKET_SEL;
            fpga_write_32_reg(FPGA_SIM_STATUS_CTL_REG, data);
        }
    break;
    case SIM1:
        /* check if SIM 1 exists */
        fpga_read_32_reg(FPGA_SIM_STATUS_CTL_REG, &data);
        if (!(data & LTE_SIM_1_PRESENT_DECTECT)) {
            printf("\nNo SIM1 Present!!!"
                  "\nPlease insert the SIM card and reset the modem.\n");
        } else {
            /* Enable SIM 1 */
            fpga_read_32_reg(FPGA_SIM_STATUS_CTL_REG, &data);
            data |= LTE_SIM_SOCKET_SEL;
            fpga_write_32_reg(FPGA_SIM_STATUS_CTL_REG, data);
        }
    break;
    default:
        printf("%s(%d) Unsupported SIM number: %d\n",
               __func__, __LINE__, sim_num);
        return (FAILED);
    }

    /* Send AT command to get the state of WP76xx LTE modem SIM_DETECT pin */
    if (run_at_cmd(at_cmd_set) != PASSED) {
        printf("\nFailed to get SIM%d SIM_DETECT state.\n", sim_num);
        return (FAILED);
    }

    printf("\nSIM%d SIM_DETECT pin current state: %d.\n",
           sim_num, wp76xx_simdetect_state);
    return (PASSED);
}

/*******************************************************************************
 *
 * Function   : wp76xx_simdetect_pin_test
 * Description: Wrapped function to test LTE WP76xx SIM_DETECT pin.
 *              This function is to test WP76xx LTE SIM_DETECT/SIM_DETECT_2 pin
 *              by check if the state that AT!BSGPIO read back is as expected.
 *              Besides, this function also provides usr_prompt parameter for
 *              user prompt display enable/disable.
 * Inputs     : sim_num - SIM number(0/1)
 *              exp_sim_stat - Expected SIM status: PRESENT(1)/NOT_PRESENT(0)
 *              usr_prompt - To ENABLE/DISABLE user prompt
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
static int wp76xx_simdetect_pin_test (int sim_num, boolean exp_sim_stat,
                                      boolean usr_prompt)
{
    char usr_input = 0;
    char usr_act_str[LTE_TESTMSG_BUFSZ];
    int at_cmd_test = 0;
    uint data;

    memset(usr_act_str, 0, sizeof(usr_act_str));

    /* Based on C1109-2P design,
     * If SIM card is present, LTE UIM1_DET/SIM_DETECT signal should be HIGH.
     * If SIM card is removed, LTE UIM1_DET/SIM_DETECT signal should be LOW.
     */

    /* Configure parameters based on testing SIM number */
    switch (sim_num) {
    case SIM0:
    case SIM1:
        if (exp_sim_stat == SIM_PRESENT) {
            sprintf(usr_act_str, "install SIM card to");
            /* SIM0/1 is inserted, UIM1_DET should be HIGH */
            at_cmd_test = WP76XX_UIM1_DET_H;
        } else {
            sprintf(usr_act_str, "remove SIM card from");
            /* SIM0/1 is removed, UIM1_DET should be LOW */
            at_cmd_test = WP76XX_UIM1_DET_L;
        }
        break;
    default:
        printf("%s(%d) Unsupported SIM number: %d\n",
               __func__, __LINE__, sim_num);
        return (FAILED);
    }

    /* Print out user prompt if needed */
    if (usr_prompt == ENABLE) {
        printf("\n\n### Please %s SIM slot %d.\n", usr_act_str, sim_num);
        do {
            printf("\r### Press 'y' to continue the Test: ");
            usr_input = getchar();
            if (usr_input == 'y') {
                break;
            }
        } while (usr_input != 'y');
    }

    /* Switch FPGA to select SIM number */
    switch (sim_num) {
    case SIM0:
        if (exp_sim_stat == SIM_PRESENT) {

            /* check if  SIM 0 exists */
            fpga_read_32_reg(FPGA_SIM_STATUS_CTL_REG, &data);
            if (!(data & LTE_SIM_0_PRESENT_DECTECT)) {
                cterr('f', 0, "\nNo SIM0 Present!!!"
                      "\nPlease insert the SIM card and reset the modem.\n");
                return(FAILED);
            } else {
                /* Enable SIM 0 */
                fpga_read_32_reg(FPGA_SIM_STATUS_CTL_REG, &data);
                data &= ~LTE_SIM_SOCKET_SEL;
                fpga_write_32_reg(FPGA_SIM_STATUS_CTL_REG, data);
            }
        }
        break;
    case SIM1:
        if (exp_sim_stat == SIM_PRESENT) {

            /* check if SIM 1 exists */
            fpga_read_32_reg(FPGA_SIM_STATUS_CTL_REG, &data);
            if (!(data & LTE_SIM_1_PRESENT_DECTECT)) {
                cterr('f', 0, "\nNo SIM1 Present!!!"
                      "\nPlease insert the SIM card and reset the modem.\n");
                return(FAILED);
            } else {
                /* Enable SIM 1 */
                fpga_read_32_reg(FPGA_SIM_STATUS_CTL_REG, &data);
                data |= LTE_SIM_SOCKET_SEL;
                fpga_write_32_reg(FPGA_SIM_STATUS_CTL_REG, data);
            }
        }
        break;
    default:
        printf("%s(%d) Unsupported SIM number: %d\n",
               __func__, __LINE__, sim_num);
        return (FAILED);
    }

    /* Confirm the status of WP76xx LTE modem SIM_DETECT pin */
    if (run_at_cmd(at_cmd_test) != PASSED) {
        printf("%s(%d) SIM_DETECT pin status is not as expected.\n",
               __func__, __LINE__);
        return (FAILED);
    }
    return (PASSED);
}

/*******************************************************************************
 *
 * Function   : lte_wp76xx_simdetect_pin_test
 * Description: Wrapped function to test LTE WP76xx UIM1_DET pin.
 * Inputs     : sim_num - SIM number(0/1)
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
static int lte_wp76xx_simdetect_pin_test (int sim_num)
{
    /* CSCvk17720: [Star] LTE SIM Detect pin(UIM1_DET) issue on C1109-2P
     *
     * This utility test is to enhance the coverage of WP76xx LTE UIM1_DET pin.
     * It requires users to insert and remove SIM card during the test.
     *
     * Based on comment from SWI(Sierra wireless) on WP76xx:
     *   - AT!ENTERCND="A710" is required to enable the AT!BSGPIO command.
     *   - AT!BSGPIO?34 can be used to check the state of UIM1_DET signal.
     */ 

    char test_name[LTE_TESTMSG_BUFSZ];

    memset(test_name, 0, sizeof(test_name));

    /* Configure parameters based on testing SIM number */
    switch (sim_num) {
    case SIM0:
        sprintf(test_name, "WP76xx LTE UIM1_DET pin(SIM0)");
        break;
    case SIM1:
        sprintf(test_name, "WP76xx LTE UIM1_DET pin(SIM1)");
        break;
    default:
        cterr('f', 0, "Failed, got unsupported SIM number: %d ", sim_num);
        return (FAILED);
    }

    testname(test_name);
    prpass(testpass, "SIM%d, ", sim_num);

    /* Test UIM1_DET pin when SIM is present */
    if (wp76xx_simdetect_pin_test(sim_num, SIM_PRESENT, ENABLE) != PASSED) {
        cterr('f', 0, "Failed, SIM%d is inserted "
                      "but UIM1_DET state is Low.", sim_num);
        return (FAILED);
    }

    /* Test UIM1_DET pin when SIM is NOT present */
    if (wp76xx_simdetect_pin_test(sim_num, SIM_NOT_PRESENT, ENABLE) != PASSED) {
        cterr('f', 0, "Failed, SIM%d is NOT inserted "
                      "but UIM1_DET state is High.", sim_num);
        return (FAILED);
    }

    prpass(testpass, "SIM%d, ", sim_num);
    return (PASSED);
}

/***************************************************************************
* Name: lte_get_tty_num 
*
* Description: This function returns ttyUSB number which the specified usb
*              port attaches to.
*              We capture the all ttyUSB link info and store in tty_num.txt
*              file.
* 
* Input: *usb_port - Pointer to specified usb port
*        *tty_num - Pointer to store which ttyUSB number that the specified
*                   usb port attaches to
*
* Example: # readlink /sys/class/tty/ttyUSB*
*          ../../devices/platform/cpn-110-master/cpn-110-master:config-space
*          /f2510000.usb3/usb3/3-1/3-1:1.0/ttyUSB0/tty/ttyUSB0
*          ../../devices/platform/cpn-110-master/cpn-110-master:config-space
*          /f2510000.usb3/usb3/3-1/3-1:1.2/ttyUSB1/tty/ttyUSB1
*          ../../devices/platform/cpn-110-master/cpn-110-master:config-space
*          /f2510000.usb3/usb3/3-1/3-1:1.3/ttyUSB2/tty/ttyUSB2
*          ../../devices/platform/cpn-110-master/cpn-110-master:config-space
*          /f2510000.usb3/usb3/3-1/3-1:1.8/ttyUSB3/tty/ttyUSB3
*
*
* Output: PASSED/FAILED
***************************************************************************/
int lte_get_tty_num (char *tty_num)
{
    char *bufptr, *usb_port_str = NULL;
    char *tty_num_str1, *tty_num_str2;
    char buffer[TTYUSB_BUF_SIZE] = {0,};
    char get_tty_info_cmd[64] = {0,};
    char tty_num_str[64] = {0,};
    int fd_ttydev, tty_str_len, nbytes = 0;
    
    /* 1. Store ttyUSB info in TTYUSB_INFO_FILE */
    sprintf(get_tty_info_cmd, "%s > %s", TTYUSB_INFO_CMD, TTYUSB_INFO_FILE); 
    system(get_tty_info_cmd);

    fd_ttydev = open(TTYUSB_INFO_FILE, O_RDWR);

    if (fd_ttydev == -1) { 
        cterr('f', 0, "Can't open %s", TTYUSB_INFO_FILE);
        return (FAILED);
    }

    bufptr = buffer;
    
    while ((nbytes = read(fd_ttydev, bufptr, TTYUSB_INFO_SIZE)) > 0) {
        bufptr += nbytes;

        if (bufptr[-1] == '\n' || bufptr[-1] == '\r') {
            break;
        }
    }

    *bufptr = '\0';

    /* 2. Check whether modem usb device attaches to tty successfully or not*/
    usb_port_str = (char *)strstr(buffer, usb_port);

    if (usb_port_str == NULL) {
        close(fd_ttydev);
        return (FAILED);
    }
    
    /* 3. Get the corresponding ttyUSB number which the specified usb device
     *    attaches to 
     */
    tty_num_str1 = strstr(usb_port_str, "ttyUSB");
    tty_num_str2 = strstr(tty_num_str1, "/");
    tty_str_len = strlen(tty_num_str1) - strlen(tty_num_str2);
    strncpy(tty_num_str, tty_num_str1, tty_str_len);
    sprintf(tty_num, tty_num_str);

    close(fd_ttydev);
    return (PASSED);
}

/***************************************************************************
* Name: lte_at_open_tty
*
* Description: This function opens tty device for AT command 
* 
* Input: *tty_fd - Pointer to the TTY file descriptor
*
* Output: PASSED/FAILED
***************************************************************************/
static int lte_at_open_tty (int *tty_fd)
{
    int fd;
    struct termios options;
    int timeout = VTIME_TIMEOUT;
    char usb_tty_dev[256];
    char usb_tty[15];

    /* Dynamically get the corresponding ttyUSB number in case usb device
     * attaches to different ttyUSB 
     */ 
    if (lte_get_tty_num(usb_tty_dev) != PASSED) {
        printf("%s: Can't get ttyUSB number\n", __func__);
        return (FAILED);
    }

    sprintf(usb_tty, "%s%s", USB_TTY_PATH, usb_tty_dev);
    fd = open(usb_tty, O_RDWR | O_NOCTTY | O_NDELAY);
    
    if (fd == -1) {
        printf("%s: Can't open tty device : %s\n", __func__, strerror(errno));
        return (FAILED);
    }

    fcntl(fd, F_SETFL, 0);
    tcgetattr(fd, &options);
    options.c_cflag |= (CLOCAL | CREAD);
    options.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);
    options.c_oflag &= ~OPOST;
    options.c_cc[VMIN] = 0xFF;
    options.c_cc[VTIME] = timeout;
    tcsetattr(fd, TCSANOW, &options);

    *tty_fd = fd;

    return (PASSED);
}

/*******************************************************************************
 * Function   : lte_usb_get_vid_did_speed
 * Description: This function reads from system USB file and return Vendor ID, 
 *              Device ID and speed
 * Inputs     : usb_path - USB Path, e.g. 1-1, 3-1, 4-1
 *              *vid, *did, *speed
 * Outputs    : PASSED or FAILED
 *******************************************************************************
 */
static int lte_usb_get_vid_did_speed (char *usb_path, int *vid, 
                                      int *did, int *speed)
{
    FILE *file;
    char fname[64];


    /* Check if the file exists */
    sprintf(fname, "%s/%s", USB_SYS_PATH, usb_path);
    if (access(fname, F_OK) == -1) {
        return (FAILED);
    }

    /* Get the Vendor ID */
    sprintf(fname, "%s/%s/%s", USB_SYS_PATH, usb_path, USB_SYS_VID_FILE);
    file = fopen(fname, "rb");
    if (file == NULL) {
        return (FAILED);
    }

    fscanf(file, "%x", vid);

    fclose(file);

    /* Get the Product ID */
    sprintf(fname, "%s/%s/%s", USB_SYS_PATH, usb_path, USB_SYS_DID_FILE);
    file = fopen(fname, "rb");
    if (file == NULL) {
        return (FAILED);
    }

    fscanf(file, "%x", did);

    fclose(file);

    /* Get the Speed */
    sprintf(fname, "%s/%s/%s", USB_SYS_PATH, usb_path, USB_SYS_SPEED_FILE);
    file = fopen(fname, "rb");
    if (file == NULL) {
        return (FAILED);
    }

    fscanf(file, "%d", speed);

    fclose(file);

    return (PASSED);
}

/*******************************************************************************
 * Function   : lte_host_usb_detect
 * Description: Enumerates USB and detects USB device through host USB
 *              by given vendor and device ID and speed
 * Inputs     : usb_mode - USB 2.0 or USB 3.0
 *              vid - Vendor ID
 *              speed - 480 (USB 2.0) or 5000 (USB 3.0)
 * Outputs    : PASSED or FAILED
 *******************************************************************************
 */
int lte_host_usb_detect (int vid, int speed)
{
    int dev_vid, dev_did, dev_speed;

    if (this_is_star_c1109_2p() || this_is_supernova_c959_2p()) {
        if (lte_usb_get_vid_did_speed(HOST_USB1_20_PATH, 
                                      &dev_vid, 
                                      &dev_did, 
                                      &dev_speed) == FAILED) {
            return (FAILED);
        }
    }

    if ((vid == dev_vid) && (speed == dev_speed)) {
        return (PASSED);
    }

    if ((NVRAM)->diagflag & D_VERBOSE) { 
        printf("Vendor ID=%#x, Device ID=%#x, Speed=%#x\n", dev_vid, dev_did, 
                                                            dev_speed);
    }

    return (FAILED);
}

/********************************************************************
 *
 * Function   : lte_wp_modem_pwr_off_ctrl
 * Description: Function to power off LTE WP modem
 * Inputs     : pwr_opt - 0 for power off, 1 for power on
 * Outputs    : PASSED or FAILED
 *
 ********************************************************************/
int lte_wp_modem_pwr_off_ctrl (void)
{
    uint32 time_out = 0;
    int at_test_cmd;

    /* 1. Power down modem */
    at_test_cmd = LTE_PWR_DOWN;

    if (run_at_cmd(at_test_cmd) != PASSED) {
        return (FAILED);
    }

        /* 2. Wait safe power remove */
    uint read_data = 0;
    while (time_out < TIMEOUT_600) {
        if (fpga_read_32_reg(FPGA_LTE_CTL_REG, &read_data) != PASSED) {
            printf("FPGA read fail, reg: %x\n", FPGA_LTE_CTL_REG);
            return (FAILED);
        }

        if (read_data & LTE_SAFE_PWR_REMOVE) {
            /* Write 1 to clean SAFE POWER REMOVE bit */
            read_data |= LTE_SAFE_PWR_REMOVE;
            if (fpga_write_32_reg(FPGA_LTE_CTL_REG, read_data) != PASSED) {
                printf("FPGA read fail, reg: %x\n", FPGA_LTE_CTL_REG);
                return (FAILED);
            }    
            break;
        }
        time_out++;

        msleep(100);
    }
    
    /* 3. Wait 13 ms as t_pwr_remove defined in WP76XX product spec. r5 */
    msleep(WP_PWR_REMOVE_DELAY);

    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("time_out: %d\n", time_out);
    }

    if (time_out == TIMEOUT_600) {
        cterr('f', 0, "Wait safe Power Remove timeout");
        return (FAILED);
    }


    /* 4. Clean 3.7V power control and modem power control */
    if (fpga_read_32_reg(FPGA_LTE_CTL_REG, &read_data) != PASSED) {

        printf("FPGA read fail, reg: %x\n", FPGA_LTE_CTL_REG);
        return (FAILED);
    }

    read_data &= ~(LTE_PRI_POWER_EN_CTL);

    if (fpga_write_32_reg(FPGA_LTE_CTL_REG, read_data) != PASSED) {
        printf("FPGA read fail, reg: %x\n", FPGA_LTE_CTL_REG);
        return (FAILED);
    }

    if (fpga_read_32_reg(FPGA_LTE_CTL_REG, &read_data) != PASSED) {

        printf("FPGA read fail, reg: %x\n", FPGA_LTE_CTL_REG);
        return (FAILED);
    }

    read_data &= ~(LTE_PRI_MODEM_EN_CTL);

    if (fpga_write_32_reg(FPGA_LTE_CTL_REG, read_data) != PASSED) {
        printf("FPGA read fail, reg: %x\n", FPGA_LTE_CTL_REG);
        return (FAILED);
    }
    
    return (PASSED);
}

/********************************************************************
 *
 * Function   : lte_wp_clr_saf_pwr_remv 
 * Description: Function to clear SAFE_POWER_REMOVAL pin of WP module
 *              by assert FPGA RESET# pin
 * Inputs     : none
 * Outputs    : PASSED or FAILED
 *
 ********************************************************************/
int lte_wp_clr_saf_pwr_remv (void)
{
    int ix, val = HIGH;
    uint data = 0;
    
    /* Assert RESET pin to clear SAFE_PWR_REMOVAL pin */
    printf("Clearing SAFE_PWR_REMOVAL pin.\n");
    if (fpga_reset_32_api(FPGA_EXTER_DEV_RST_REG,
                          EXT_PRI_LTE_RESET, TRUE,
                          WAITTIME_150_MS) == FAILED) {
        cterr('f', 0, "Reset the LTE modem fails!");
        return (FAILED);
    }
    /* Base on WP modem spec, RESET# should be toggle to high at least 32ms */
    msleep(WP_HD_RESET_H_DELAY);
    if (fpga_reset_32_api(FPGA_EXTER_DEV_RST_REG,
                          EXT_PRI_LTE_RESET, FALSE,
                          WAITTIME_150_MS) == FAILED) {
        cterr('f', 0, "Reset the LTE modem fails!");
        return (FAILED);
    }

    /* Check if SAFE_PWR_REMOVAL is de-asserted */
    for (ix = 0; ix < WP_CHK_PWR_TOUT; ix++) {
        val = HIGH;
        if (fpga_write_32_reg(FPGA_LTE_CTL_REG, data) != PASSED) {
            printf("%s: Failed to write FPGA reg.(0x%04X).\n",
                   __FUNCTION__, FPGA_LTE_CTL_REG);
            return (FAILED);
        }
        val = data & LTE_SAFE_PWR_REMOVE;
        if (val == LOW) {
            printf("OK\n");
            break;
        }
        msleep(LTE_POLLING_DELAY);
        printf(".");
    }

    if (val == HIGH) {
        printf("Failed to clear SAFE_POWER_REMOVAL signal\n");
        return (FAILED);
    }
    return (PASSED);
}

/*-------------------------------------------------
$Log: lte.c,v $
Revision 1.14  2020/07/10 11:36:50  steja
Enhanced TSN LTE Series
1.CSCvu76591 [TSN-H/TSN-GFAST] Modify SIM_DETECT pin Test item
2.CSCvu72092: [TSN-H/TSN-GFAST] Enhance the LTE USB port number change dynamically
3.CSCvu72089: [TSN-H/TSN-GFAST] Adjust LTE Power Sequence

Revision 1.13.34.4  2020/07/10 09:17:11  steja
Update comment based on PRRQ CSCvu72089

Revision 1.13.34.3  2020/07/01 01:53:21  sherliu2
Add wrapper function for SIM_DETECT pin Test Insertion & Removal

Revision 1.13.34.2  2020/06/24 07:37:40  sherliu2
Seperate SIM_DETECT pin Test to become SIM_DETECT pin Test Insertion and Removal

Revision 1.13.34.1  2020/06/19 08:24:57  steja
1. Adjust LTE Power Sequence based SWI Guideline
2. Dynamically ttyUSB port for AT command
3. Remove LTE init on SIM Detection Test

Revision 1.13  2019/06/26 10:26:24  yungchen
CSCvq20258 : one sim card sku can not get expect result with at+cpin
Bumped up version to V6.0.4

Revision 1.12  2019/03/07 09:51:32  lucywang
[Supernova] PID changed : C1101L-4P --> C951-4P, C1109L-2P --> C959-2P

Revision 1.11  2019/01/18 05:54:46  yungchen
Merge Supernova branch to the main trunk (CSCvn79871)

Revision 1.10  2018/08/20 01:26:14  lucywang
CSCvm04601 - [Star] LTE SIM card test failed during RDT on C1109-2P

Revision 1.9  2018/07/12 07:24:48  lucywang
CSCvk17720 - [Star] LTE SIM Detect pin(UIM1_DET) issue on C1109-2P

Revision 1.8  2018/06/29 14:13:44  palin2
CSCvk03740: TSN LTE SIM0 SIM_DETECT pin issue.
Enhanced test coverage on EM74xx LTE SIM_DETECT pin.

Revision 1.7  2018/06/05 09:54:08  lucywang
Merge Star branch star-branch-c110x to main trunk

Revision 1.6  2018/05/15 09:37:32  steja
CSCvj38863: Enhanced LED single test utility

Revision 1.5  2018/02/09 09:56:54  hondwang
Merge Star branch star-branch-c9xx to main trunk

Revision 1.4  2018/01/23 11:43:54  steja
Fix the compile issue

Revision 1.3  2018/01/23 11:38:18  steja
Merge tsn-gfast-branch4 code to maintrunk for support TSN-G.Fast (CSCvh40981)

Revision 1.2.20.5  2018/02/08 07:16:05  lucywang
Merged LTE USB2.0 detect test from trunk

Revision 1.2.20.4  2018/02/07 10:23:01  lucywang
Followed coding rule

Revision 1.2.20.3  2018/01/22 08:32:50  lucywang
WP76xx only supports 1 SIM

Revision 1.2.20.2  2018/01/22 07:20:41  lucywang
Fixed CSCvh49477, make sure ttyUSB2 is able to be opened before AT command

Revision 1.2.20.1  2018/01/20 06:27:23  hondwang
prepare merge star-branch-c9xx to main trunk

Revision 1.2.18.1  2018/01/18 13:13:50  steja
Add LTE USB 2.0 Detection Test

Revision 1.2.4.9  2017/12/15 06:27:35  lucywang
Sync from Pluggable LTE : Added diagnostic test mode for pluggable LTE-WP76xx GPS pin test

Revision 1.2.4.8  2017/12/01 09:00:46  lucywang
Check modem is ready before every LTE test item

Revision 1.2.4.7  2017/11/20 07:54:31  lucywang
Changed PID to C1101/C1109-2P/C1109-4P

Revision 1.2.4.6  2017/11/10 08:17:40  lucywang
Sync from Pluggable LTE : Modified the timeout mechanism of GPS pin test

Revision 1.2.4.5  2017/11/06 06:28:16  lucywang
Added GPS pin test for on-board WP module

Revision 1.2.4.4  2017/09/15 03:03:59  lucywang
added timeout for LTE AT commands

Revision 1.2.4.3  2017/08/28 10:12:58  lucywang
enable USB MUX output for C949-2P

Revision 1.2.4.2  2017/08/28 03:34:13  lucywang
modified for C949-2P

Revision 1.2.4.1  2017/08/15 14:18:38  hondwang
star branch c9xx initial check in

Revision 1.2  2017/08/02 14:21:46  steja
Support TSN-H/M platform code

Revision 1.1.8.2  2017/07/29 03:41:03  steja
tsn-branch5 synced with Maintrunk repositories

Revision 1.1.6.4  2017/07/25 08:31:55  steja
1. Remove unused code.
2. Verified before check-in

Revision 1.1.6.3  2017/07/21 10:46:03  steja
Update based on code review comment

Revision 1.1.6.2  2017/07/20 13:38:05  steja
tsn-branch4 merge with maintrunk

Revision 1.1.4.9.2.7  2017/07/11 10:13:16  steja
1. Remove Debugcard test
2. Add LTE micro usb utility to basic utilities
3. Code clean up

Revision 1.1.4.9.2.6  2017/07/08 07:27:26  steja
Code Clean up

Revision 1.1.4.9.2.5  2017/06/14 10:59:56  steja
Fix minor bug

Revision 1.1.4.9.2.4  2017/06/12 11:23:20  steja
Enhanced LTE mini-usb test

Revision 1.1.4.9.2.3  2017/06/09 12:59:53  steja
Update LTE RSSI/DIV frequency to meet criteria for EM 7455 & EM 7430

Revision 1.1.4.9.2.2.2.4  2017/07/24 22:51:07  tirawan
Add RSSI support for WP modems

Revision 1.1.4.9.2.2.2.3  2017/07/11 18:29:33  tirawan
Add AT command utility and change the RSSI frequency to 944.5 Mhz

Revision 1.1.4.9.2.2.2.2  2017/06/25 06:41:23  tirawan
Initialize GPIO Expander Output port before configuring its direction

Revision 1.1.4.9.2.2.2.1  2017/06/13 06:54:14  shjung
Add pluggable FPGA I2C read/write function

Revision 1.1.4.9.2.2  2017/05/08 11:52:43  steja
Fix Enumerate LTE USB (Mini USB) under Rommong (CSCve33718)

Revision 1.1.4.9.2.1  2017/02/17 07:09:47  steja
Update LTE code based on latest FPGA 170215

Revision 1.1.4.9  2016/11/15 13:19:09  petteng
Add enhanced error message

Revision 1.1.4.8  2016/11/01 07:29:20  petteng
Add enhanced error message

Revision 1.1.4.7  2016/10/04 06:39:08  petteng
Add enhanced error message

Revision 1.1.4.6  2016/08/15 13:02:26  steja
Add utility for LTE switch to USB external port

Revision 1.1.4.5  2016/08/12 12:00:50  steja
Update SIM Card test report

Revision 1.1.4.4  2016/07/21 14:26:32  steja
1. Update Temperature sensor function
2. Add Wifi Temperature sensor to basic utilities
3. Add LTE Extended Feature Test
4. Display Chasis Temperature When Boot up
5. Update I2C scan function
6. Update POE cookie check card present

Revision 1.1.4.3  2016/07/11 02:01:05  steja
Add define flag OLD_SWI_FW

Revision 1.1.4.2  2016/06/30 06:22:48  steja
tsn-branch2 sync with main trunk

Revision 1.1.2.13  2016/06/27 09:03:43  steja
Fixed SIM CARD Detect using AT command

Revision 1.1.2.12  2016/06/21 12:57:18  steja
Workaround for SIM0/1 DETECT

Revision 1.1.2.11  2016/06/17 10:41:31  steja
Minor change

Revision 1.1.2.10  2016/06/17 10:36:33  steja
Add SIM card slot 1 test

Revision 1.1.2.9  2016/06/16 10:16:04  steja
Fixed RSSI and DIV code

Revision 1.1.2.8  2016/05/24 09:23:10  steja
Add AT command utility

Revision 1.1.2.7  2016/04/29 10:28:32  steja
Temporarily removed test flag

Revision 1.1.2.6  2016/04/25 07:40:57  steja
1. Enable LTE and USB enumerate
2. Enable MODEM detect

Revision 1.1.2.5  2016/04/23 15:00:29  steja
Check in for fix SPD Read RAW

Revision 1.1.2.4  2016/04/22 11:34:00  steja
check-in for first release

Revision 1.1.2.3  2016/04/14 06:09:49  palin2
1. Removed cpld.c and cpld.h because TSN don't have CPLD.
2. Linked related function to correct FPGA one.

Revision 1.1.2.2  2016/03/27 14:17:34  steja
update based on code review comment 3/25/2016

Revision 1.1.2.1  2016/03/24 03:58:26  steja
Add LTE test


*/
