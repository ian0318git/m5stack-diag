/* $Id: diag_lte_test.c,v 1.2 2018/08/06 02:31:51 harrchan Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/viper/diag_lte_test.c,v $
 *------------------------------------------------------------------
 *
 * diag_lte_test.c - SWI LTE EM74XX/WP760X Function
 *
 *
 * Copyright (c) 2008-2018 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "endians.h"
#include "defs.h"
#include "types.h"
#include "nvsysvars.h"
#include "error.h"
#include "pcmap.h"
#include "strings.h"
#include "menu.h"
#include "common.h"
#include "cross_platform.h"
#include "mb_tests.h"
#include "plat_defs.h"
#include "setjmps.h"
#include "proto.h"
#include "platform_fru.h"
#include "platform_cookie.h"
#include "plat_defs.h"
#include "diag_lte_test.h"
#include "diag_lte_lib.h"
#include "diag_fpga_lib.h"
#include "diag_storage_lib.h"
#include "diag_fpga.h"
#include "diag_lte_util.h"



int diag_lte_test(boolean);

static int diag_lte_modem_detect_test(int);
#ifdef MODEM_RESET_TEST
static int diag_lte_modem_reset_test(int);
#endif
static int diag_lte_main_rssi_test(int);
static int diag_lte_div_rssi_test(int);
static int diag_lte_sim_test(int);
static int diag_lte_dport_test(int);
static int diag_lte_at_console_switch(void);
#ifdef GPS_WP_PIN_TEST
static int diag_lte_gps_antennae_test(int);
static int diag_lte_gps_pin_test(int);
#endif

extern int do_all_menu_items(struct menuinfo *);

static submenu_xtable_t diag_lte_table[] = {
    {"LTE Utility", (type_t(*)())build_lte_utils_menu, 0,
     0, (type_t(*)())0, 0, (type_t(*)())0, 0},
    {"AT Command Utility", (type_t(*)())diag_lte_at_console_switch, 0,
     0, (type_t(*)())0, 0, (type_t(*)())0, 0},
    {"Modem Detection Test", (type_t(*)())diag_lte_modem_detect_test, TRUE,
     MF_CONTINUOUS | MF_DOALL | MF_SHOW_ERRCOUNT,
      (type_t(*)())0, 0, (type_t(*)())0, 0},
#ifdef MODEM_RESET_TEST
    {"Modem Reset Test", (type_t(*)())diag_lte_modem_reset_test, TRUE,
     MF_CONTINUOUS | MF_DOALL | MF_SHOW_ERRCOUNT,
      (type_t(*)())0, 0, (type_t(*)())0, 0},
#endif
    {"Modem Main RSSI Test", (type_t(*)())diag_lte_main_rssi_test, TRUE,
     MF_SHOW_ERRCOUNT,
      (type_t(*)())0, 0, (type_t(*)())0, 0},
    {"Modem DIV RSSI Test", (type_t(*)())diag_lte_div_rssi_test, TRUE,
     MF_SHOW_ERRCOUNT,
      (type_t(*)())0, 0, (type_t(*)())0, 0},
#ifdef GPS_WP_PIN_TEST
    {"Modem GPS Antenna", (type_t(*)())diag_lte_gps_antennae_test, TRUE,
     MF_SHOW_ERRCOUNT,
      (type_t(*)())0, 0, (type_t(*)())0, 0},
#endif
    {"SIM Card Test", (type_t(*)())diag_lte_sim_test, TRUE,
     MF_CONTINUOUS | MF_DOALL | MF_SHOW_ERRCOUNT,
      (type_t(*)())0, 0, (type_t(*)())0, 0},
    {"USB Debug Port Detection Test", (type_t(*)())diag_lte_dport_test, TRUE,
     MF_CONTINUOUS | MF_DOALL | MF_SHOW_ERRCOUNT,
      (type_t(*)())0, 0, (type_t(*)())0, 0},
#ifdef GPS_WP_PIN_TEST
    {"GPS Pin Test", (type_t(*)())diag_lte_gps_pin_test, TRUE,
     MF_CONTINUOUS | MF_DOALL | MF_SHOW_ERRCOUNT,
      (type_t(*)())0, 0, (type_t(*)())0, 0},
#endif
};

#define DIAG_LTE_TABLE_SZ \
        (sizeof(diag_lte_table) / sizeof(submenu_xtable_t))


static mitem_t diag_lte_pri_test_items[DIAG_LTE_TABLE_SZ+ MAX_BASE_ITEMS];
static mitem_t diag_lte_sec_test_items[DIAG_LTE_TABLE_SZ+ MAX_BASE_ITEMS];

static menuinfo_t diag_lte_test_menu = {
    "LTE Test Menu",
    0,                            /* mtparam added by init_empty_menu */
    (PFT)menu_show_dflags,        /* shows major flags */
    0,                            /* use generic prompt */
    0,                            /* size (bumped by add_menu_item() */
    diag_lte_pri_test_items,
};
static menuinfo_t *diag_lte_test_menup = &diag_lte_test_menu;

/*******************************************************************************
 * Function   : diag_lte_test
 * Description: Main Entry point for Pluggable LTE 
 * Inputs     : *plug - Pointer to Pluggable Data structure
 * Outputs    : PASSED or FAILED
 *******************************************************************************
 */
int diag_lte_test (boolean mb_temp_test_items_executed)
{

    /* Out of reset LTE*/
    if (fpga_reset_api(FPGA_EXTER_DEV_RST_REG, EXT_PRI_LTE_RESET, FALSE,
                           WAITTIME_20_MS) != PASSED) {
        cterr('f', 0, "%s: Failed to release ACT2 from Reset.\n", __FUNCTION__);
    }
    /* Power on Module */
    printf("Powering up LTE Module...");

    if (diag_lte_pwr_on(TRUE) != PASSED) {
        cterr('f', 0, "Powering up LTE Module fails");
        return (FAILED);
    }

    printf("OK\n");
    fflush(stdout);

    /* Check if SWI tty USB device comes up */
    printf("Detecting ttyUSB device...");
    if (diag_lte_is_usb_found(TRUE, LTE_USB_DETECT_TOUT) == FALSE) {
        cterr('f', 0, "LTE USB Device can't be found");
        return (FAILED);
    }
    printf("OK\n");
    fflush(stdout);

    build_primary_submenu(diag_lte_table, DIAG_LTE_TABLE_SZ, 
                         "LTE", &diag_lte_test_menup);

    build_secondary_submenu(diag_lte_table, DIAG_LTE_TABLE_SZ,
                            diag_lte_sec_test_items);

    if (mb_temp_test_items_executed) {
        do_all_menu_items(diag_lte_test_menup);
    } else {
        menu(&diag_lte_test_menu, diag_lte_sec_test_items, 0);
    }

    if (diag_lte_pwr_on(FALSE) != PASSED) {
        cterr('f', 0, "Powering up LTE Module fails");
        return (FAILED);
    }


    return (PASSED);
}


/*******************************************************************************
 * Function   : diag_lte_modem_detect_test
 * Description: To detect LTE modem for Pluggable LTE
 * Inputs     : input - Not used
 * Outputs    : PASSED or FAILED
 *******************************************************************************
 */
static int diag_lte_modem_detect_test (int input)
{
    dev_lte_swi_object_t lte_obj;
    dev_lte_swi_object_t *lte_obj_p = &lte_obj;
    int rc;
    uchar mb_get_loc[FRU_SIZE] = {0};
    uchar mb_get_pid[FRU_SIZE] = {0};
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
    cterr_add_component("Intel Denverton SOC C3558", "MUX", 
                        "Sierra Wireless WP");

    /* Segment 5: register and memory dump */

    /* Segment 6: Platform Environment initialized here*/

    /* Segment 7: Top 3 Debugging Steps */
    cterr_add_debug("Use 'External USB Enable' utility to "
                    "enable USB debug port and connect USB cable "
                    "to external PC and observe whether modem is"
                    " detected by PC.(Contact vendor on how to "
                    "install driver on PC)");

    testname("Modem Detection");

    if (diag_lte_swi_dev_create(lte_obj_p) != PASSED) {
        cterr('f', 0, "Create SWI Dev Object Fails");
        return (FAILED);
    }

    prpass(testpass, "Modem  Detection");

    rc = lte_obj_p->callin_fvt->modem_detection_test((dev_object_t *)&lte_obj);

    if (rc != PASSED) {
        cterr('f', 0, "Modem Detection fails");
    }

    lte_obj_p->base.dev_object_fvt->dev_destroy((dev_object_t **)&lte_obj_p);

    prcomplete(testpass, errcount, (char *)0);

    return (rc);
}


#ifdef MODEM_RESET_TEST
/*******************************************************************************
 * Function   : diag_lte_modem_reset_test
 * Description: Do the LTE reset initialization sequence for Pluggable LTE
 * Inputs     : input - Not used
 * Outputs    : PASSED or FAILED
 *******************************************************************************
 */
static int diag_lte_modem_reset_test (int input)
{
    dev_lte_swi_object_t lte_obj;
    dev_lte_swi_object_t *lte_obj_p = &lte_obj;
    int ix, rc;
    int lte_usb_found;

    testname("Modem Reset");

    if (diag_lte_swi_dev_create(lte_obj_p) != PASSED) {
        cterr('f', 0, "Create SWI Dev Object Fails");
        return (FAILED);
    }

    prpass(testpass, "Modem Reset");

    rc = lte_obj_p->callin_fvt->modem_reset_test((dev_object_t *)&lte_obj);

    if (rc != PASSED) {
        cterr('f', 0, "Modem Reset fails");
        goto __exit;
    }

    /* Check and wait until modem disappears */
    prpass(testpass, "Check modem disappears");
    for (ix = 0; ix < LTE_MODEM_DISAPPEAR_TOUT; ix++) {
        lte_usb_found = diag_lte_is_usb_found(FALSE, 0);

        if (lte_usb_found == FALSE) {
            break;
        }
        msleep(10);
    }

    if (ix == LTE_MODEM_DISAPPEAR_TOUT || lte_usb_found == TRUE) {
        rc = FAILED;
        cterr('f', 0, "Modem is stil alive after reset");
        goto __exit;
    }

    prpass(testpass, "Check modem comes back");
    /* Check and wait until modem is back */
    if (diag_lte_is_usb_found(TRUE, LTE_MODEM_BACK_ALIVE_TOUT) == FALSE) {
        rc = FAILED;
        cterr('f', 0, "Modem doesn't come back after reset");
    }

__exit:
    lte_obj_p->base.dev_object_fvt->dev_destroy((dev_object_t **)&lte_obj_p);

    prcomplete(testpass, errcount, (char *)0);

    return (rc);
}
#endif

/*******************************************************************************
 * Function   : diag_lte_main_rssi_test
 * Description: To get LTE main RSSI for Pluggable LTE
 * Inputs     : input - Not used
 * Outputs    : PASSED or FAILED
 *******************************************************************************
 */
static int diag_lte_main_rssi_test (int input)
{
    dev_lte_swi_object_t lte_obj;
    dev_lte_swi_object_t *lte_obj_p = &lte_obj;
    int rc;
    uchar mb_get_loc[FRU_SIZE] = {0};
    uchar mb_get_pid[FRU_SIZE] = {0};
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
    cterr_add_component("Intel Denverton SOC C3558", "MUX", 
                        "Sierra Wireless WP", "Antenna cable");

    /* Segment 5: register and memory dump */

    /* Segment 6: Platform Environment initialized here*/

    /* Segment 7: Top 3 Debugging Steps */
    cterr_add_debug("Perform 'Modem Detection Test' to make sure that"
                    " modem is still alive",
                    "Check external equipment setting and make sure "
                    "that it is configure at correct frequency and band",
                    "Make sure that antenna cable is tucked in perfectly");

    testname("Main RSSI");

    if (diag_lte_swi_dev_create(lte_obj_p) != PASSED) {
        cterr('f', 0, "Create SWI Dev Object Fails");
        return (FAILED);
    }

    prpass(testpass, "Main RSSI");

    rc = lte_obj_p->callin_fvt->modem_rssi_test((dev_object_t *)&lte_obj, MAIN_RSSI);

    if (rc != PASSED) {
        cterr('f', 0, "Main RSSI Test fails");
    }

    lte_obj_p->base.dev_object_fvt->dev_destroy((dev_object_t **)&lte_obj_p);

    prcomplete(testpass, errcount, (char *)0);

    return (rc);
}


/*******************************************************************************
 * Function   : diag_lte_div_rssi_test
 * Description: To get LTE DIV RSSI for Pluggable LTE
 * Inputs     : input - Not used
 * Outputs    : PASSED or FAILED
 *******************************************************************************
 */
static int diag_lte_div_rssi_test (int input)
{
    dev_lte_swi_object_t lte_obj;
    dev_lte_swi_object_t *lte_obj_p = &lte_obj;
    int rc;
    uchar mb_get_loc[FRU_SIZE] = {0};
    uchar mb_get_pid[FRU_SIZE] = {0};
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
    cterr_add_component("Intel Denverton SOC C3558", "MUX", 
                        "Sierra Wireless WP", "Antenna cable");

    /* Segment 5: register and memory dump */

    /* Segment 6: Platform Environment initialized here*/

    /* Segment 7: Top 3 Debugging Steps */
    cterr_add_debug("Perform 'Modem Detection Test' to make sure that"
                    " modem is still alive",
                    "Check external equipment setting and make sure "
                    "that it is configure at correct frequency and band",
                    "Make sure that antenna cable is tucked in perfectly");
    testname("Div RSSI");

    if (diag_lte_swi_dev_create(lte_obj_p) != PASSED) {
        cterr('f', 0, "Create SWI Dev Object Fails");
        return (FAILED);
    }

    prpass(testpass, "Div RSSI");

    rc = lte_obj_p->callin_fvt->modem_rssi_test((dev_object_t *)&lte_obj, DIV_RSSI);

    if (rc != PASSED) {
        cterr('f', 0, "Div RSSI Test fails");
    }

    lte_obj_p->base.dev_object_fvt->dev_destroy((dev_object_t **)&lte_obj_p);

    prcomplete(testpass, errcount, (char *)0);

    return (rc);
}

#ifdef GPS_WP_PIN_TEST
/*******************************************************************************
 * Function   : diag_lte_gps_antennae_test
 * Description: To get LTE GPS antennae information for Pluggable LTE
 * Inputs     : input - Not used
 * Outputs    : PASSED or FAILED
 *******************************************************************************
 */
static int diag_lte_gps_antennae_test (int input)
{
    dev_lte_swi_object_t lte_obj;
    dev_lte_swi_object_t *lte_obj_p = &lte_obj;
    int rc;

    testname("GPS Antenna");

    if (diag_lte_swi_dev_create(lte_obj_p) != PASSED) {
        cterr('f', 0, "Create SWI Dev Object Fails");
        return (FAILED);
    }

    prpass(testpass, "GPS Antenna");

    rc = lte_obj_p->callin_fvt->modem_gps_ant_test((dev_object_t *)&lte_obj);

    if (rc != PASSED) {
        cterr('f', 0, "GPS Antenna Test fails");
    }

    lte_obj_p->base.dev_object_fvt->dev_destroy((dev_object_t **)&lte_obj_p);

    prcomplete(testpass, errcount, (char *)0);

    return (rc);
}
#endif

/*******************************************************************************
 * Function   : diag_lte_sim_test
 * Description: To detect LTE SIM 0 card for Pluggable LTE
 * Inputs     : input - SIM 0 or SIM 1
 * Outputs    : PASSED or FAILED
 *******************************************************************************
 */
static int diag_lte_sim_test (int input)
{
    dev_lte_swi_object_t lte_obj;
    dev_lte_swi_object_t *lte_obj_p = &lte_obj;
    int rc;
    uchar mb_get_loc[FRU_SIZE] = {0};
    uchar mb_get_pid[FRU_SIZE] = {0};
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
    cterr_add_component("Intel Denverton SOC C3558", 
                        "Sierra Wireless WP", "SIM Card");

    /* Segment 5: register and memory dump */

    /* Segment 6: Platform Environment initialized here*/

    /* Segment 7: Top 3 Debugging Steps */
    cterr_add_debug("Make sure that SIM card is installed properly in "
                    "corresponding SIM slot", 
                    "Replace SIM card with the golden one", 
                    "Probe SIM_DETECT_N signal to ensure that SIM present "
                    "pin is good");

    testname("SIM Detection");

    /* Check if SIM is detected by FPGA */
    if (diag_lte_sim_detected_by_fpga() != TRUE) {
        cterr('f', 0, "SIM is not detected by FPGA");
        return (FAILED);
    }

    if (diag_lte_swi_dev_create(lte_obj_p) != PASSED) {
        cterr('f', 0, "Create SWI Dev Object Fails");
        return (FAILED);
    }

    prpass(testpass, "SIM Detection");

    rc = lte_obj_p->callin_fvt->sim_detect_test((dev_object_t *)&lte_obj, SIM_0);

    if (rc != PASSED) {
        cterr('f', 0, "SIM Detection Test fails");
    }

    lte_obj_p->base.dev_object_fvt->dev_destroy((dev_object_t **)&lte_obj_p);

    prcomplete(testpass, errcount, (char *)0);

    return (rc);
}


/*******************************************************************************
 * Function   : diag_lte_usb_dport_test
 * Description: Run USB enumeration on mini USB debug port
 * Inputs     : None
 * Outputs    : PASSED or FAILED
 *******************************************************************************
 */
static int diag_lte_dport_test (int input)
{
    int ix, ret;
    char *vid = LTE_USB_VENDOR_ID;
    uint reg_offset = 0, reg_val = 0;
    char *tname = "USB Debug Port Detection";
    uchar mb_get_loc[FRU_SIZE] = {0};
    uchar mb_get_pid[FRU_SIZE] = {0};
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
    cterr_add_component("Intel Denverton SOC C3558", "MUX", 
                        "Sierra Wireless WP", "micro USB","USB 2.0 port");

    /* Segment 5: register and memory dump */

    /* Segment 6: Platform Environment initialized here*/

    /* Segment 7: Top 3 Debugging Steps */
    cterr_add_debug("If you are using USB hub, remove hub and connect debug "
                    "port and host USB directly", "Make sure that USB cable is "
                    "connected properly from micro-USB debug port to host USB", 
                    "Replace USB cable with the golden one", 
                    "Run 'Modem Detection Test' to make sure that modem is "
                    "alive.Check host USB is alive");

    testname(tname);
    prpass(testpass, "USB debug port test");
    
    /*
     * D_EXT_LOOPBACK = 0, enable ext. loopback
     * * D_EXT_LOOPBACK = 1, disable ext. loopback
     */
    if (check_menu_flag(D_EXT_LOOPBACK)) {
        prpass(testpass, "\n External loopback flag is off, skip '%s' \
            external loopback test. ", tname);
        prcomplete(testpass, errcount, (char *)0);
        return (PASSED);
    }
	
    /* Enable usb debug port through FPGA register 0x938 */
    reg_offset = FPGA_LTE_CTL_REG;
    reg_val = (LTE_MODEM_POWER_CONTROL | EXT_PRI_LTE_WDIS_1_RESET | LTE_USB_MUX_SEL_CTL |
               LTE_PRI_MODEM_EN_CTL);
   
    if (fpga_write_reg(reg_offset, reg_val) != PASSED) {
        printf("Fail to write 0x%08X to FPGA register(0x%04X).\n",
               reg_val, reg_offset);
        return (FAILED);
    }
    
    /* Wait linux to detect fpga action */ 
    msleep(1000); 

    for(ix = 0; ix < LTE_USB_DETECT_TOUT; ix++) {
        ret = diag_lte_dport_host_usb_detect(vid, USB20_SPEED);

        if (ret == PASSED) {
            break;
        } 
        msleep(10);
    }

    /* Disable usb debug port through FPGA register 0x938 */
    reg_offset = FPGA_LTE_CTL_REG;
    reg_val = (LTE_MODEM_POWER_CONTROL | EXT_PRI_LTE_WDIS_1_RESET | LTE_PRI_MODEM_EN_CTL);
   
    if (fpga_write_reg(reg_offset, reg_val) != PASSED) {
        printf("Fail to write 0x%08X to FPGA register(0x%04X).\n",
               reg_val, reg_offset);
        return (FAILED);
    }
    
    if (ret == FAILED) {
        cterr('f', 0, "Failed to detect LTE USB debug port");
        return (ret);
    } else if (ret == -2) {
        cterr('f', 0, "Failed to switch to Debug port");
        return (FAILED);
    }
    
    /* Check tty USB device comes up */ 
    if (diag_lte_is_usb_found(TRUE, LTE_USB_DETECT_TOUT) == FALSE) {
        cterr('f', 0, "LTE USB Device can't be found");
        return (FAILED);
    }
    return (ret);
}


#ifdef GPS_WP_PIN_TEST
/*******************************************************************************
 * Function   : diag_lte_gps_pin_test 
 * Description: Function to check GPS pin value 
 * Inputs     : input - Not used 
 * Outputs    : PASSED or FAILED
 *******************************************************************************
 */
static int diag_lte_gps_pin_test (int input)
{
    dev_lte_swi_object_t lte_obj;
    dev_lte_swi_object_t *lte_obj_p = &lte_obj;
    int rc;
        
    testname("GPS Pin");

    if (diag_lte_swi_dev_create(lte_obj_p) != PASSED) {
        cterr('f', 0, "Create SWI Dev Object Fails");
        return (FAILED);
    }

    prpass(testpass, "Drive GPS Pin to Low");

    /* Drive GPS Pin to Low */
    rc = lte_obj_p->callin_fvt->wp76xx_drive_gps_pin((dev_object_t *)&lte_obj, 
                                                      GPS_PIN_LOW);
    if (rc != PASSED) {
        cterr('f', 0, "Drive GPS Pin to Low Fails");
        goto __exit;
    }

    prpass(testpass, "Drive GPS Pin to High");
    /* Drive GPS Pin to High */
    rc = lte_obj_p->callin_fvt->wp76xx_drive_gps_pin((dev_object_t *)&lte_obj, 
                                                      GPS_PIN_HIGH);
    if (rc != PASSED) {
        cterr('f', 0, "Drive GPS Pin to High Fails");
        goto __exit;
    }

__exit:
    lte_obj_p->base.dev_object_fvt->dev_destroy((dev_object_t **)&lte_obj_p);

    prcomplete(testpass, errcount, (char *)0);

    return (diag_lte_modem_reset_test(0));
}
#endif


/*******************************************************************************
 * Function   : diag_lte_at_console_switch
 * Description: To execute AT command for Pluggable LTE
 * Inputs     : input - Not used
 * Outputs    : PASSED or FAILED
 *******************************************************************************
 */ 
static int diag_lte_at_console_switch (void)
{
    const int maxlen = 128;
    char cmd[maxlen];
    char tty_dev_name[32];

    printf("\n\n ### NOTE: Type CTRL-x "
                              "to switch back to host's console\n\n");
    fflush(stdout);
    fflush(stderr); 
    msleep(AT_COMMAND_UTIL_DELAY);

    diag_lte_get_ttyusb_name(tty_dev_name);
       
    snprintf(cmd, maxlen-1, "microcom %s", tty_dev_name);

    system(cmd);
       
    return (PASSED);
}     

/*-------------------------------------------------
 * $Log: diag_lte_test.c,v $
 * Revision 1.2  2018/08/06 02:31:51  harrchan
 * Merge viper E2E to the main trunk (CSCvk28469)
 *
 * Revision 1.1.2.14  2018/07/09 08:27:33  olin2
 * CSCvk17781: Support util to verify SIM Detect pin
 *
 * Revision 1.1.2.13  2018/07/06 02:54:08  harrchan
 * Add enhance error message
 *
 * Revision 1.1.2.12  2018/05/29 03:04:20  harrchan
 * Add LTE power on/off sequence
 *
 * Revision 1.1.2.11  2018/05/04 07:25:19  lucywang
 * Show message when FPGA MUX switch failed in LTE debug port test
 *
 * Revision 1.1.2.10  2018/05/03 09:23:25  lucywang
 * Modified LTE USB debug port test flow
 *
 * Revision 1.1.2.9  2018/05/03 07:41:23  lucywang
 * Controlled LTE USB Debug port test by EXT_LOOPBACK flag
 *
 * Revision 1.1.2.8  2018/04/23 08:05:15  harrchan
 * According star project remove modem reset test
 *
 * Revision 1.1.2.7  2018/04/20 02:10:50  lucywang
 * Added to support LTE WP7607/WP7608/WP7609
 *
 * Revision 1.1.2.6  2018/04/17 07:59:09  harrchan
 * Rename LTE Test Menu
 *
 * Revision 1.1.2.5  2018/04/17 03:47:34  harrchan
 * Polling ttyusb status when doing LTE usb debug port test
 *
 * Revision 1.1.2.4  2018/04/13 03:29:07  harrchan
 * Set FPGA register to out of reset component
 *
 * Revision 1.1.2.3  2018/04/10 06:17:15  harrchan
 * Modify FPGA register address
 *
 * Revision 1.1.2.2  2018/03/26 09:21:22  harrchan
 * Support usb debug port detection test
 *
 * Revision 1.1.2.1  2018/02/27 08:06:45  harrchan
 * Initial viper application code base
 *
 *
 * $Endlog$
 *-------------------------------------------------
 */
