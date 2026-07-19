/* $Id: diag_usb_test.c,v 1.4 2019/07/11 12:31:30 alicehua Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/nutella/diag_usb_test.c,v $
 *------------------------------------------------------------------
 *
 * Filename: diag_usb_test.c
 *
 * Copyright (c) 2013-2019 by cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#include <errno.h>
#include <dirent.h>
#include <libgen.h>
#include <pthread.h>
#include <semaphore.h>
#include <time.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/select.h>
#include <stdlib.h>
#include <unistd.h>
#include <mtd/mtd-user.h>
#include <sys/ioctl.h>
#include "types.h"
#include "common.h"
#include "monitor.h"
#include "menu.h"
#include "error.h"
#include "proto.h"
#include "platform_fru.h"
#include "platform_cookie.h"
#include "linux_usb_test.h"
#include "nvmonvars.h"
#include "plat_defs.h"
#include "diag_usb_lib.h"
#include "diag_usb_test.h"
#include "diag_storage_lib.h"
#include "linux_block_test.h"


/*
 * Global extern functions
 */

/*
 * Declare local function
 */
extern struct usb_info_t usb[];
int nutella_usb_slot_tests(int);
int diag_ext_usb_test(int);
static int usb_tests(int n);

static submenu_xtable_t usbtest_menu_table[] = {   
    {"External USB 2.0 test", (PFT) usb_tests, USB_20,
    MF_CONTINUOUS | MF_DOGRP | MF_DOALL | MF_SHOW_ERRCOUNT,
    (type_t(*)())0, 0, (PFT) 0, 0},
    {"External USB 3.0 test", (PFT) usb_tests, USB_30,
    MF_CONTINUOUS | MF_DOGRP | MF_DOALL | MF_SHOW_ERRCOUNT,
    (type_t(*)())0, 0, (PFT) 0, 0},
};

#define USB_TEST_MENU_TABLE_SZ \
        (sizeof(usbtest_menu_table) / sizeof(submenu_xtable_t))

/*
 * primary & secondary submenu items (filled in from xtable)
 */
static mitem_t usbtest_pri_items[USB_TEST_MENU_TABLE_SZ + MAX_BASE_ITEMS];
static mitem_t usbtest_sec_items[USB_TEST_MENU_TABLE_SZ + MAX_BASE_ITEMS];

static struct menuinfo usbtest_menu = {
    "USB Test Menu",
    0,                          /* mtparam added by init_empty_menu */
    (PFT) menu_show_dflags,     /* notes missing WICs in combos */
    0,                          /* use generic prompt */
    0,                          /* size (bumped by add_menu_item() */
    usbtest_pri_items,
};

static struct menuinfo *usb_test_menup = &usbtest_menu;

/*******************************************************************************
 *
 * Function   :    usb_tests
 * Description:    usb r/w tests.
 * Inputs     :    usb_speed: USB speed
 * Outputs    :    PASSED or FAILED.
 *
 *******************************************************************************
 */
int usb_tests (int usb_speed)
{
    int usb_idx;
    int rc = FAILED;
    char tname[50] = {0};
    struct usb_info_t usb2_storage, usb3_storage; 

    memset(&usb2_storage, 0, sizeof(&usb2_storage));
    memset(&usb3_storage, 0, sizeof(&usb3_storage));

    if (usb_speed == USB_20) {
        strcpy(tname, "External USB 2.0 port");
    } else if (usb_speed == USB_30) {
        strcpy(tname, "External USB 3.0 port");
    } else {
        cterr('f',0,"Unsupported USB speed!");
        return (FAILED);
    }

    testname(tname);
    prpass(testpass, "%s, ", tname);

    /* External loopback only for external USB stick*/
    if ((usb_speed == USB_20) || (usb_speed == USB_30)) {
        /*
         * D_EXT_LOOPBACK = 0, enable ext. usb stick
         * D_EXT_LOOPBACK = 1, disable ext. usb stick
         */
        if (check_menu_flag(D_EXT_LOOPBACK)) {
            prpass(testpass, "\n External loopback flag is off, skip '%s' \
                   external loopback test. ", tname);
            prcomplete(testpass, errcount, (char *)0);
            return (PASSED);
        }
    }
    /* Save usb info into usb structure */
    if (usb_parse_info() == FAILED) {
        cterr('f', 0, "%s() usb_parse_info() failed", __FUNCTION__);
        return (FAILED);
    }

    /* Check if the USB 2.0 storage device is detected or not */
    if (get_usb_storage_info(USB_HOST20_SPEED, &usb2_storage) == FAILED) {
        cterr('f', 0, "Can not get USB2.0 storage information");
        return (FAILED);
    }
    
    /* Check if the USB 3.0 storage device is detected or not */
    if (get_usb_storage_info(USB_HOST30_SPEED, &usb3_storage) == FAILED) {
        cterr('f', 0, "Can not get USB3.0 storage information");
        return (FAILED);
    }

    /* Get USB storage index */
    if (usb_speed == USB_20) {
        if (find_usb_storage_index(USB_HOST20_SPEED, &usb_idx) == FALSE) {
            cterr('f', 0, "Can not find USB2.0 storage");
            return (FAILED);
        }
    } else {
        if (find_usb_storage_index(USB_HOST30_SPEED, &usb_idx) == FALSE) {
            cterr('f', 0, "Can not find USB3.0 storage");
            return (FAILED);
        }
    }

    rc = nutella_usb_slot_tests(usb_idx);
    if (rc == PASSED) {
        prcomplete(testpass, errcount, (char *)0);
    } else {
        cterr('f', 0, "R/W Test Failed");
    }

    return (rc);
}

/*******************************************************************************
 *
 * Function   :    nutella_usb_slot_tests
 * Description:    entry point to usb device test
 * Inputs     :    usb idx
 * Outputs    :    PASSED or FAILED.
 *
 *******************************************************************************
 */
int nutella_usb_slot_tests (int usb_idx)
{
    char src[32], usb_spd[15];
    int retval;
    char tname[50] = {0};

    /* don't find the devname ex./dev/sda */
    if (strcmp(usb[usb_idx].dev_name, "") == 0) {
        cterr('f',0,"Failed to find USB storage.");
        return (FAILED);
    }

    if (usb[usb_idx].spd == USB_HOST30_SPEED) {
        sprintf(usb_spd, "%s", "[USB 3.0] ");
    } else {
        sprintf(usb_spd, "%s", "[USB 2.0] ");
    }

    sprintf(src, "/dev/%s", usb[usb_idx].dev_name);
    retval = linux_block_test(src, 0, BLOCK_SIZE_512B, BLOCK_TEST_RANDOM, TRUE);

    prpass(testpass, "%s%s, ", usb_spd, tname);
    return (retval);
}

/**********************************************************************
 *
 * Function   : diag_ext_usb_test
 * Description: This function is doing usb r/w  test 
 * Inputs     : Test/Menu
 * Outputs    : PASSED
 *
 **********************************************************************
 */
int diag_ext_usb_test (int show_menu)
{
    char *tname = "Test USB 2.0 and 3.0";
    testname(tname);

    build_primary_submenu(usbtest_menu_table, USB_TEST_MENU_TABLE_SZ,
                          "USB", &usb_test_menup);
    build_secondary_submenu(usbtest_menu_table, USB_TEST_MENU_TABLE_SZ,
                          usbtest_sec_items);

    if (show_menu) {
        menu(&usbtest_menu, usbtest_sec_items, '\0');
    } else {
        exec_doall_menu_items(&usbtest_menu);
    }

    return (PASSED);
}

/******** History ********
$Log: diag_usb_test.c,v $
Revision 1.4  2019/07/11 12:31:30  alicehua
Collapse Nutella codes into main trunk.

$Endlog$
*/
