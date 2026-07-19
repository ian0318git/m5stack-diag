/* $Id: usb_dongle_lte_swi_test.c,v 1.2 2019/06/14 09:59:36 steja Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/usb_dongle/usb_dongle_lte_swi/usb_dongle_lte_swi_test.c,v $
 *------------------------------------------------------------------
 *
 * usb_dongle_lte_swi_test.c - USB LTE Main Functions
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
#include <sys/time.h>
#include <unistd.h>
#include <fcntl.h>
#include "common.h"
#include "types.h"
#include "error.h"
#include "menu.h"
#include "proto.h"
#include "nvmonvars.h"
#include "usb_dongle_common_test.h"
#include "usb_dongle_common_lib.h"
#include "usb_dongle_lte_swi_test.h"
#include "usb_dongle_lte_swi_lib.h"
#include "usb_dongle_lte_swi_util.h"

int usb_dongle_lte_swi_main(void *);
static int usb_dongle_lte_swi_modem_detect_test(int);
static int usb_dongle_lte_swi_main_rssi_test(int);
static int usb_dongle_lte_swi_div_rssi_test(int);
static int usb_dongle_lte_swi_sim_test(int);
static int usb_dongle_lte_swi_led_test(int);
static int usb_dongle_lte_swi_intf_test(int);

extern int do_all_menu_items(struct menuinfo *);
int modem_found = FALSE;

static submenu_xtable_t udongle_lte_swi_table[] = {
    {"Modem Detection Test", (type_t(*)())usb_dongle_lte_swi_modem_detect_test, 
     TRUE, MF_CONTINUOUS | MF_DOALL | MF_SHOW_ERRCOUNT,
     (type_t(*)())0, 0, (type_t(*)())0, 0},
    {"Modem Main RSSI Test", (type_t(*)())usb_dongle_lte_swi_main_rssi_test, 
     TRUE, MF_SHOW_ERRCOUNT, 
     (type_t(*)())0, 0, (type_t(*)())0, 0},
    {"Modem DIV RSSI Test", (type_t(*)())usb_dongle_lte_swi_div_rssi_test, 
     TRUE, MF_SHOW_ERRCOUNT, 
     (type_t(*)())0, 0, (type_t(*)())0, 0},
    {"SIM Card Test", (type_t(*)())usb_dongle_lte_swi_sim_test, 
     SIM0, MF_CONTINUOUS | MF_DOALL | MF_SHOW_ERRCOUNT,
     (type_t(*)())0, 0, (type_t(*)())0, 0},
    {"LED Test", (type_t(*)())usb_dongle_lte_swi_led_test, 
     TRUE, MF_SHOW_ERRCOUNT,
     (type_t(*)())0, 0, (type_t(*)())0, 0},
    {"Utilities", (type_t(*)())usb_dongle_lte_swi_util, TRUE,
     0, (type_t(*)())0, 0, (type_t(*)())0, 0}
};
    
#define UDONGLE_LTE_SWI_TABLE_SZ\
        (sizeof(udongle_lte_swi_table) / sizeof(submenu_xtable_t))

static mitem_t usb_dongle_lte_swi_pri_test_items[UDONGLE_LTE_SWI_TABLE_SZ + 
                                                 MAX_BASE_ITEMS];
static mitem_t usb_dongle_lte_swi_sec_test_items[UDONGLE_LTE_SWI_TABLE_SZ +
                                                 MAX_BASE_ITEMS];
static menuinfo_t udongle_lte_swi_test_menu = {
    "USB Dongle LTE Test Menu",   /*menu title*/
    0,                            /*optional title parameter */
    (PFT)menu_show_dflags,        /*show major flags*/
    0,                            /*use generic prompt*/
    0,                            /*the number of items in the menu */
    usb_dongle_lte_swi_pri_test_items,     /*pointer to array of menu item structure*/
};

static menuinfo_t *udongle_lte_swi_test_menup = &udongle_lte_swi_test_menu;
static struct udongle_intf_t *udongle;

/*------------------------------------------------------------------------------
 * Function:    usb_dongle_lte_swi_main
 * Description: Main entry point for USB Dongle LTE
 * Inputs:      *udongle - Pointer to USB Dongle Data structure
 * Outputs:     PASSED/FAILED
 *------------------------------------------------------------------------------
 */
int usb_dongle_lte_swi_main (void *in)
{
    int ix;
    int ret = PASSED;

    /* Sanity check */
    if (in == NULL) {
        cterr('f', 0, "Null pointer");
        return (FAILED);
    }

    udongle = (struct udongle_intf_t *)in;
    usb_dongle_lte_swi_set_devname((char *)udongle->product); 

    /* Suppress printk so kernel doesn't print out bunch of meassages */
    system(SYS_SUPPRESS_PRINTK);

    usb_dongle_lte_swi_insmod(TRUE);
    
    /* Probe wheather the modem is detected */
    printf("probing the modem in USB 2.0 mode ...");
    fflush(stdout);

    for (ix = 0; ix < PROBE_LTE_USB_TOUT; ix++) { 
        if (usb_dongle_lte_swi_usb_detect(udongle->port, MODEM_SWI_USB_VID,
                                          UDONGLE_USB2P0_SPEED) == TRUE) {
            modem_found = TRUE;
            break;
        }
        msleep(USB_DONGLE_LTE_POLLING_DELAY);
    }

    if (modem_found == TRUE) {
        printf("OK\n");
    } else {
        cterr('f', 0, "USB Dongle is not detected");
        ret = FAILED;
        goto __exit;
    }

    /* Get the USB port number which is used for transmitting AT command */
    usb_dongle_lte_swi_set_at_devinfo(udongle->port);
   
    /* Poll access symbolic link file */
    if (usb_dongle_lte_swi_poll_tty_symlink() != PASSED) {
        ret = FAILED;
        goto __exit;
    }

    if (udongle->test_type == IFACE_TEST) {
        if (usb_dongle_lte_swi_intf_test(0) != PASSED) { 
            ret = FAILED;
        }
        goto __exit;
    }

    build_primary_submenu(udongle_lte_swi_table, UDONGLE_LTE_SWI_TABLE_SZ,
                          "USB Dongle LTE", &udongle_lte_swi_test_menup);

    build_secondary_submenu(udongle_lte_swi_table, UDONGLE_LTE_SWI_TABLE_SZ,
                            usb_dongle_lte_swi_sec_test_items);

    /* Show the menu or do all menu */
    if (udongle->menu_display) {
        menu(&udongle_lte_swi_test_menu, usb_dongle_lte_swi_sec_test_items,
             '\0');
    } else {
        do_all_menu_items(&udongle_lte_swi_test_menu);
    }

    /* Exit */
__exit:
    if (modem_found == TRUE) {
        if (!((NVRAM)->diagflag & D_POWER_ON)) {   
            printf("Power off");  
            /* Power off modem*/
            if (usb_dongle_lte_swi_modem_pwr_ctrl(FALSE) == FAILED) {
                cterr('f', 0, "Failed to soft power-off SWI modem");
            }
        }
    }
    system(SYS_RESTORE_PRINTK);

    return (ret);
}

/*------------------------------------------------------------------------------
 * Function:    usb_dongle_lte_swi_modem_detect_test
 * Description: To detect LTE modem for USB Dongle LTE
 * Inputs:      input - not used
 * Outputs:     PASSED/FAILED
 *------------------------------------------------------------------------------
 */
static int usb_dongle_lte_swi_modem_detect_test (int input)
{
    dev_lte_swi_object_t ud_lte_obj;
    dev_lte_swi_object_t *ud_lte_obj_p = &ud_lte_obj;
    int rc;

    /* Need to Enhance error message */
    
    testname("Modem Detection");

    if (usb_dongle_lte_swi_dev_create(ud_lte_obj_p) != PASSED) {
        cterr('f', 0, "Create SWI Dev Object Fails");
        return (FAILED);
    }

    prpass(testpass, "Modem Detection");

    rc = ud_lte_obj_p->callin_fvt->modem_detection_test
                                   ((dev_object_t *)&ud_lte_obj);

    if (rc != PASSED) {
        cterr('f', 0, "Modem Detection fails");
    }

    ud_lte_obj_p->base.dev_object_fvt->dev_destroy((dev_object_t **)
                                                   &ud_lte_obj_p);

    prcomplete(testpass, errcount, (char *)0);

    return (rc);
}


/*------------------------------------------------------------------------------
 * Function:    usb_dongle_lte_swi_main_rssi_test
 * Description: To get LTE main RSSI for USB Dongle LTE
 * Inputs:      input - not used
 * Outputs:     PASSED/FAILED
 *------------------------------------------------------------------------------
 */
static int usb_dongle_lte_swi_main_rssi_test (int input)
{
    dev_lte_swi_object_t ud_lte_obj;
    dev_lte_swi_object_t *ud_lte_obj_p = &ud_lte_obj;
    int rc;

    testname("Main RSSI");

    if (usb_dongle_lte_swi_dev_create(ud_lte_obj_p) != PASSED) {
        cterr('f', 0, "Create SWI Dev Object Fails");
        return (FAILED);
    }

    prpass(testpass, "Main RSSI");

    rc = ud_lte_obj_p->callin_fvt->modem_rssi_test
                                   ((dev_object_t *)&ud_lte_obj, MAIN_RSSI);

    if (rc != PASSED) {
        cterr('f', 0, "Main RSSI fails");
    }

    ud_lte_obj_p->base.dev_object_fvt->dev_destroy((dev_object_t **)
                                                   &ud_lte_obj_p);

    prcomplete(testpass, errcount, (char *)0);

    return (rc);
}

/*------------------------------------------------------------------------------
 * Function:    usb_dongle_lte_swi_div_rssi_test
 * Description: To get LTE DIV RSSI for USB Dongle LTE
 * Inputs:      input - not used
 * Outputs:     PASSED/FAILED
 *------------------------------------------------------------------------------
 */
static int usb_dongle_lte_swi_div_rssi_test (int input)
{
    dev_lte_swi_object_t ud_lte_obj;
    dev_lte_swi_object_t *ud_lte_obj_p = &ud_lte_obj;
    int rc;

    testname("DIV RSSI");

    if (usb_dongle_lte_swi_dev_create(ud_lte_obj_p) != PASSED) {
        cterr('f', 0, "Create SWI Dev Object Fails");
        return (FAILED);
    }

    prpass(testpass, "DIV RSSI");

    rc = ud_lte_obj_p->callin_fvt->modem_rssi_test
                                   ((dev_object_t *)&ud_lte_obj, DIV_RSSI);

    if (rc != PASSED) {
        cterr('f', 0, "DIV RSSI fails");
    }

    ud_lte_obj_p->base.dev_object_fvt->dev_destroy((dev_object_t **)
                                                   &ud_lte_obj_p);

    prcomplete(testpass, errcount, (char *)0);

    return (rc);
}

/*------------------------------------------------------------------------------
 * Function:    usb_dongle_lte_swi_sim_test
 * Description: To detect LTE SIM card for USB Dongle LTE
 * Inputs:      input - SIM0
 * Outputs:     PASSED/FAILED
 *------------------------------------------------------------------------------
 */
static int usb_dongle_lte_swi_sim_test (int input)
{
    dev_lte_swi_object_t ud_lte_obj;
    dev_lte_swi_object_t *ud_lte_obj_p = &ud_lte_obj;
    int rc;

    testname("SIM Card");

    if (usb_dongle_lte_swi_dev_create(ud_lte_obj_p) != PASSED) {
        cterr('f', 0, "Create SWI Dev Object Fails");
        return (FAILED);
    }

    prpass(testpass, "SIM Card");

    rc = ud_lte_obj_p->callin_fvt->sim_detect_test
                                   ((dev_object_t *)&ud_lte_obj, input);

    if (rc != PASSED) {
        cterr('f', 0, "SIM Card fails");
    }

    ud_lte_obj_p->base.dev_object_fvt->dev_destroy((dev_object_t **)
                                                   &ud_lte_obj_p);

    prcomplete(testpass, errcount, (char *)0);

    return (rc);
}

/*------------------------------------------------------------------------------
 * Function:    usb_dongle_lte_swi_led_test
 * Description: Function to turn the WWAN_LED ON/OFF 
 * Inputs:      input - not used
 * Outputs:     PASSED/FAILED
 *------------------------------------------------------------------------------
 */
static int usb_dongle_lte_swi_led_test (int input)
{
    dev_lte_swi_object_t ud_lte_obj;
    dev_lte_swi_object_t *ud_lte_obj_p = &ud_lte_obj;
    int rc;

    testname("LED");

    if (usb_dongle_lte_swi_dev_create(ud_lte_obj_p) != PASSED) {
        cterr('f', 0, "Create SWI Dev Object Fails");
        return (FAILED);
    }

    prpass(testpass, "LED");

    /* Turn LED On */
    printf("\nTurn LED ON\n");
    rc = ud_lte_obj_p->callin_fvt->toggle_wwan_led((dev_object_t *)
                                                   &ud_lte_obj, WWAN_LED_ON);
    sleep (1);
    if (rc != PASSED) {
        cterr('f', 0, "LED fails");
    }

    /* Turn LED OFF */
    printf("\nTurn LED OFF\n");
    rc = ud_lte_obj_p->callin_fvt->toggle_wwan_led((dev_object_t *)
                                                   &ud_lte_obj, WWAN_LED_OFF);
    sleep (1);
    if (rc != PASSED) {
        cterr('f', 0, "LED fails");
    }

    /* Turn LED On */
    printf("\nTurn LED ON\n");
    rc = ud_lte_obj_p->callin_fvt->toggle_wwan_led((dev_object_t *)
                                                   &ud_lte_obj, WWAN_LED_ON);
    sleep (1);
    if (rc != PASSED) {
        cterr('f', 0, "LED fails");
    }

    ud_lte_obj_p->base.dev_object_fvt->dev_destroy((dev_object_t **)
                                                   &ud_lte_obj_p);
    prcomplete(testpass, errcount, (char *)0);

    return (rc);
}

/*------------------------------------------------------------------------------
 * Function:    usb_dongle_lte_swi_intf_test
 * Description: To test USB Dongle LTE I/O interface
 *              WP module - USB2.0
 * Inputs:      input - not used
 * Outputs:     PASSED/FAILED
 *------------------------------------------------------------------------------
 */
static int usb_dongle_lte_swi_intf_test (int input)
{
    /* Accessing modem to verify USB interface */
    if (usb_dongle_lte_swi_modem_detect_test(0) != PASSED) {
        printf("USB Dongle interface test failed.\n");
        return (FAILED);
    }

    return (PASSED);
}


/*-------------------------------------------------
 * $Log: usb_dongle_lte_swi_test.c,v $
 * Revision 1.2  2019/06/14 09:59:36  steja
 * Supported Cooper usb dongle LTE
 *
 *
 *
 * $Endlog$
 *-------------------------------------------------
 */
