/*------------------------------------------------------------------
 * $Id: usb_dongle_lte_swi_util.c,v 1.2 2019/06/14 09:59:36 steja Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/usb_dongle/usb_dongle_lte_swi/usb_dongle_lte_swi_util.c,v $
 *------------------------------------------------------------------
 *
 * usb_dongle_lte_swi_util.c - USB LTE Utils Functions
 *
 * Copyright (c) 2015 - 2019 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <fcntl.h>
#include "common.h"
#include "types.h"
#include "error.h"
#include "queryflags.h"
#include "menu.h"
#include "proto.h"
#include "usb_dongle_lte_swi_lib.h"
#include "usb_dongle_lte_swi_util.h"
#include "usb_dongle_lte_swi_test.h"


int usb_dongle_lte_swi_util(void);
static int usb_dongle_lte_swi_simdetect_pin_test(int);
static int lte_swi_simdetect_pin_test(int, boolean, boolean);

static submenu_xtable_t udongle_lte_swi_utils[] = {
    {"LTE Modem SIM_DETECT pin Test(SIM0)", 
     (type_t(*)())usb_dongle_lte_swi_simdetect_pin_test, SIM0, 0, 
     (type_t(*)())0, 0, (type_t(*)())0, 0},
    {"Power Off Modem Utility", 
     (type_t(*)())usb_dongle_lte_swi_modem_pwr_ctrl, FALSE, 0, 
     (type_t(*)())0, 0, (type_t(*)())0, 0},
    {"AT command Utility", 
     (type_t(*)())usb_dongle_lte_swi_run_at_cmd, TRUE, 0, 
     (type_t(*)())0, 0, (type_t(*)())0, 0},
};

#define UDONGLE_LTE_SWI_UTIL_TABLE_SZ \
        (sizeof(udongle_lte_swi_utils) / sizeof(submenu_xtable_t))

static mitem_t usb_dongle_lte_swi_pri_util_items[UDONGLE_LTE_SWI_UTIL_TABLE_SZ+
                                                 MAX_BASE_ITEMS];
static mitem_t usb_dongle_lte_swi_sec_util_items[UDONGLE_LTE_SWI_UTIL_TABLE_SZ+
                                                 MAX_BASE_ITEMS];

static menuinfo_t udongle_lte_swi_util_menu = {
    "USB Dongle LTE Utilities Menu",
    0,
    (PFT)menu_show_dflags,
    0,
    0,
    usb_dongle_lte_swi_pri_util_items,
};

static menuinfo_t *udongle_lte_swi_util_menup = &udongle_lte_swi_util_menu;

/*------------------------------------------------------------------------------
 * Function:    usb_dongle_lte_swi_util
 * Description: Main entry point for USB Dongle LTE Utilities
 * Inputs:      None
 * Outputs:     PASSED/FAILED
 *------------------------------------------------------------------------------
 */
int usb_dongle_lte_swi_util (void) 
{
    build_primary_submenu(udongle_lte_swi_utils, UDONGLE_LTE_SWI_UTIL_TABLE_SZ,
                          "USB Dongle LTE", &udongle_lte_swi_util_menup);

    build_secondary_submenu(udongle_lte_swi_utils, 
                            UDONGLE_LTE_SWI_UTIL_TABLE_SZ,
                            usb_dongle_lte_swi_sec_util_items);
    
    menu(&udongle_lte_swi_util_menu, usb_dongle_lte_swi_sec_util_items, '\0');

    return (PASSED);
}

/*------------------------------------------------------------------------------
 * Function:    lte_swi_simdetect_pin_test
 * Description: Wrapped function to test LTE WP76xx SIM_DETECT pin.
 *              (WP76xx) LTE UIM1_DET and pluggable module
 *                       SIM0_DETECT/SIM1_DETECT pin 
 *              by check if the state that AT!BSGPIO read back is as expected.
 *              Besides, this function also provides usr_prompt parameter for
 *              user prompt display enable/disable.
 * Inputs:      sim_num - SIM number(0)
 *              exp_sim_stat - Expected SIM status: PRESENT(1)/NOT_PRESENT(0)
 *              usr_prompt - To ENABLE/DISABLE user prompt
 * Outputs:     PASSED/FAILED
 *------------------------------------------------------------------------------
 */
static int lte_swi_simdetect_pin_test (int sim_num, boolean exp_sim_stat,
                                       boolean usr_prompt)
{
   dev_lte_swi_object_t ud_lte_obj;
   dev_lte_swi_object_t *ud_lte_obj_p = &ud_lte_obj;
   int rc; 

   char usr_act_str[LTE_TESTMSG_BUFSZ]; 
   char usr_input = 0;

   testname("SIM Detect Pin");

   if (usb_dongle_lte_swi_dev_create(ud_lte_obj_p) != PASSED) {
       cterr('f', 0, "Create SWI Dev Object Fails");
       return (FAILED);
   }

   prpass(testpass, "SIM Detect Pin");

   memset(usr_act_str, 0, sizeof(usr_act_str));

    if (exp_sim_stat == SIM_PRESENT) {
        sprintf(usr_act_str, "install SIM card to");
    } else {
        sprintf(usr_act_str, "remove SIM card from");
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

    /* Confirm the status of LTE modem SIM_DETECT pin */
    rc = ud_lte_obj_p->callin_fvt->sim_detect_pin_present
                            ((dev_object_t *)&ud_lte_obj, exp_sim_stat);
    if (rc != PASSED) {
        cterr('f', 0, "SIM Detect Pin fails");
    }

    ud_lte_obj_p->base.dev_object_fvt->dev_destroy((dev_object_t **)
                                                   &ud_lte_obj_p);
    prcomplete(testpass, errcount, (char *)0);

    return (rc);
}

/*------------------------------------------------------------------------------
 * Function:    usb_dongle_lte_swi_simdetect_pin_test
 * Description: Function to test LTE modem SIM_DETECT pin
 *              On WP76xx modems: This test verifies UIM1_DET pin, 
 *                                SIM mux, SIM0_DETECT pin
 * Inputs:      sim_num - SIM number(0)
 * Outputs:     PASSED/FAILED
 *------------------------------------------------------------------------------
 */
static int usb_dongle_lte_swi_simdetect_pin_test (int sim_num)
{
    /* Test SIM_DETECT pin when SIM is present */
    if (lte_swi_simdetect_pin_test(sim_num, SIM_PRESENT, ENABLE)
        != PASSED) {
        cterr('f', 0, "Failed, SIM%d is inserted "
              "but SIM_DETECT state is Low.", sim_num);
        return (FAILED);
    }

    /* Test SIM_DETECT pin when SIM is NOT present */
    if (lte_swi_simdetect_pin_test(sim_num, SIM_NOT_PRESENT, ENABLE)
        != PASSED) {
        cterr('f', 0, "Failed, SIM%d is NOT inserted "
              "but SIM_DETECT state is High.", sim_num);
        return (FAILED);
    }

    return (PASSED);

}

/*------------------------------------------------------------------
 * $Log: usb_dongle_lte_swi_util.c,v $
 * Revision 1.2  2019/06/14 09:59:36  steja
 * Supported Cooper usb dongle LTE
 *
 *
 *------------------------------------------------------------------
 */
