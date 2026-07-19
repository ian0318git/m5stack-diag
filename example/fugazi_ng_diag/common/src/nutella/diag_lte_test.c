/* $Id: diag_lte_test.c,v 1.7 2020/02/19 03:11:29 harrchan Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/nutella/diag_lte_test.c,v $
 *------------------------------------------------------------------
 *
 * diag_lte_test.c - SWI LTE WP760X Function
 *
 *
 * Copyright (c) 2008-2019 by Cisco Systems, Inc.
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
static int diag_lte_main_rssi_test(int);
static int diag_lte_div_rssi_test(int);
static int diag_lte_sim_test(int);
static int diag_lte_dport_test(int);
static int diag_lte_at_console_switch(void);
extern int do_all_menu_items(struct menuinfo *);

static submenu_xtable_t diag_lte_table[] = {
    {"LTE Utility", (type_t(*)())build_lte_utils_menu, 0,
     0, (type_t(*)())0, 0, (type_t(*)())0, 0},
    {"AT Command Utility", (type_t(*)())diag_lte_at_console_switch, 0,
     0, (type_t(*)())0, 0, (type_t(*)())0, 0},
    {"Modem Detection Test", (type_t(*)())diag_lte_modem_detect_test, TRUE,
     MF_CONTINUOUS | MF_DOALL | MF_SHOW_ERRCOUNT,
      (type_t(*)())0, 0, (type_t(*)())0, 0},
    {"Modem Main RSSI Test", (type_t(*)())diag_lte_main_rssi_test, TRUE,
     MF_SHOW_ERRCOUNT,
      (type_t(*)())0, 0, (type_t(*)())0, 0},
    {"Modem DIV RSSI Test", (type_t(*)())diag_lte_div_rssi_test, TRUE,
     MF_SHOW_ERRCOUNT,
      (type_t(*)())0, 0, (type_t(*)())0, 0},
    {"SIM Card Test", (type_t(*)())diag_lte_sim_test, TRUE,
     MF_CONTINUOUS | MF_DOALL | MF_SHOW_ERRCOUNT,
      (type_t(*)())0, 0, (type_t(*)())0, 0},
    {"USB Debug Port Detection Test", (type_t(*)())diag_lte_dport_test, TRUE,
     MF_CONTINUOUS | MF_DOALL | MF_SHOW_ERRCOUNT,
      (type_t(*)())0, 0, (type_t(*)())0, 0},
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
 * Description: Main Entry point for LTE 
 * Inputs     : menu option
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
    
    /* Check if SIM is detected by FPGA */
    if (diag_lte_sim_detected_by_fpga() == TRUE) {
        /* Check modem carrier is matched*/
        if (diag_lte_chk_modem_carrier_is_match() == FAILED) {
            cterr('f', 0, "Modem carrier is not match.\n");
            return (FAILED);
        }
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
    /* Need to power down to aviod system accidently power off*/
    if (diag_lte_pwr_on(FALSE) != PASSED) {
        cterr('f', 0, "Powering up LTE Module fails");
        return (FAILED);
    }


    return (PASSED);
}


/*******************************************************************************
 * Function   : diag_lte_modem_detect_test
 * Description: To detect LTE modem for LTE
 * Inputs     : input - dummy
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

/*******************************************************************************
 * Function   : diag_lte_main_rssi_test
 * Description: To get LTE main RSSI for LTE
 * Inputs     : input - dummy
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
 * Description: To get LTE DIV RSSI for LTE
 * Inputs     : input - dummy
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

    rc = lte_obj_p->callin_fvt->modem_rssi_test((dev_object_t *)&lte_obj, 
                                                DIV_RSSI);

    if (rc != PASSED) {
        cterr('f', 0, "Div RSSI Test fails");
    }

    lte_obj_p->base.dev_object_fvt->dev_destroy((dev_object_t **)&lte_obj_p);

    prcomplete(testpass, errcount, (char *)0);

    return (rc);
}

/*******************************************************************************
 * Function   : diag_lte_sim_test
 * Description: To detect LTE SIM 0 card for LTE
 * Inputs     : input - dummy
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
 * Inputs     : Not dummy
 * Outputs    : PASSED or FAILED
 *******************************************************************************
 */
static int diag_lte_dport_test (int input)
{
    int ix, ret, rc = FAILED;
    char *vid = LTE_USB_VENDOR_ID;
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
     * D_EXT_LOOPBACK = 1, disable ext. loopback
     */
    if (check_menu_flag(D_EXT_LOOPBACK)) {
        prpass(testpass, "\n External loopback flag is off, skip '%s' \
            external loopback test. ", tname);
        prcomplete(testpass, errcount, (char *)0);
        return (PASSED);
    }
	
    /* Enable usb debug port through FPGA register 0x938 */
    rc = diag_lte_dport_enable(TRUE);
    if (rc != PASSED){
        cterr('f', 0, "Fail to enable LTE usb debug port.\n");
        return (FAILED);
    }
    
    /* Wait linux to detect fpga action */ 
    msleep(WAITTIME_1000_MS); 

    for(ix = 0; ix < LTE_USB_DETECT_TOUT; ix++) {
        ret = diag_lte_dport_host_usb_detect(vid, USB20_SPEED);

        if (ret == PASSED) {
            break;
        } 
        msleep(10);
    }

    if (ix == LTE_USB_DETECT_TOUT){
        if (ret == FAILED) {
            cterr('f', 0, "Failed to detect LTE USB debug port");
        } else if (ret == LTE_USB_MUX_NOT_ON_DEBUG_PORT) {
            cterr('f', 0, "Failed to switch to Debug port");
        }
    }
    
    /* Disable usb debug port through FPGA register 0x938 */
    rc = diag_lte_dport_enable(FALSE);
    if (rc != PASSED){
        cterr('f', 0, "Fail to disable LTE usb debug port.\n");
        return (FAILED);
    }
    
    /* Check tty USB device comes up */ 
    if (diag_lte_is_usb_found(TRUE, LTE_USB_DETECT_TOUT) == FALSE) {
        cterr('f', 0, "LTE USB Device can't be found");
        return (FAILED);
    }

    prcomplete(testpass, errcount, (char *)0);
    return (ret);
}

/*******************************************************************************
 * Function   : diag_lte_at_console_switch
 * Description: To execute AT command for LTE
 * Inputs     : none
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
$Log: diag_lte_test.c,v $
Revision 1.7  2020/02/19 03:11:29  harrchan
Add LTE patch for matching modem carrier (CSCvt07550)

Revision 1.6  2019/11/20 23:53:16  alicehua
CSCvs11654:
Fix the problem that when LTE debug port faild,
it won't switch micro-USB debug port back to the CPU USB MUX.

Revision 1.5  2019/08/16 10:57:19  alicehua
If the LTE debug port test is failed,
it will keep printing error message,
so modify code to fix this problem.
(CDETS number:CSCvq77630)

Revision 1.4  2019/07/11 12:31:29  alicehua
Collapse Nutella codes into main trunk.

$Endlog$
*/
