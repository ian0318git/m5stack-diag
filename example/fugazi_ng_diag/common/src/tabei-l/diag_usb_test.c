 /* $Id: diag_usb_test.c,v 1.2 2019/10/17 02:16:23 kehuang2 Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/tabei-l/diag_usb_test.c,v $
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
#include "nvmonvars.h"
#include "plat_defs.h"
#include "diag_usb_lib.h"
#include "diag_usb_test.h"
#include "diag_storage_lib.h"
#include "plug_testcard_host_impl.h"
#include "linux_block_test.h"


/*
 * Global extern functions
 */

/*
 * Declare local function
 */
struct usb_info_t usb[4];
int tabei_usb_slot_tests(int);
int diag_ext_usb_test(int);
static int usb_tests(int n);

static submenu_xtable_t usbtest_menu_table[] = {   
    {"External USB 2.0 test", (PFT) usb_tests, USB_HOST20_SPEED,
    MF_CONTINUOUS | MF_DOGRP | MF_DOALL | MF_SHOW_ERRCOUNT,
    (type_t(*)())0, 0, (PFT) 0, 0},
    {"External USB 3.0 test", (PFT) usb_tests, USB_HOST30_SPEED,
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
 * Inputs     :    usb speed 
 * Outputs    : PASSED or FAILED.
 *
 *******************************************************************************
 */
int usb_tests (int usb_test_speed)
{
    int rc = FAILED, usb_idx;
    char tname[50] = {0};
    struct usb_info_t front_usb2_storage, front_usb3_storage; 

    memset(&front_usb2_storage, 0, sizeof(front_usb2_storage));
    memset(&front_usb3_storage, 0, sizeof(front_usb3_storage));

    if (usb_test_speed == USB_HOST20_SPEED) {
        strcpy(tname, "External USB 2.0");
    } else {
        strcpy(tname, "External USB 3.0");
    } 

    testname(tname);
    prpass(testpass, "%s, ", tname);

    /* External loopback only for external USB stick*/
    if ((usb_test_speed == USB_HOST20_SPEED) || 
        (usb_test_speed == USB_HOST30_SPEED)) {
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
        cterr('f', 0, "usb_parse_info() failed\n");
        return (FAILED);
    }
   
    /* Check if the USB 2.0 storage device is detected or not */
    if (get_usb_storage_info(FRONT_USB, USB_HOST20_SPEED, 
                             &front_usb2_storage) == FAILED) {
        printf("%s() Can not get USB 2.0 storage information\n", __FUNCTION__);
    }
    
    /* Check if the USB 3.0 storage device is detected or not */
    if (get_usb_storage_info(FRONT_USB, USB_HOST30_SPEED, 
                             &front_usb3_storage) == FAILED) {
        printf("%s() Can not get USB 3.0 storage information\n", __FUNCTION__);
    }

    /* Get USB storage index */
    if (usb_test_speed == USB_HOST20_SPEED) {
        if (find_front_usb_storage_index(USB_HOST20_SPEED, &usb_idx) == FALSE) {
            cterr('f', 0, "Can not find USB2.0 storage");
            return (FAILED);
        }
    } else {
        if (find_front_usb_storage_index(USB_HOST30_SPEED, &usb_idx) == FALSE) {
            cterr('f', 0, "Can not find USB3.0 storage");
            return (FAILED);
        }
    }

    rc = tabei_usb_slot_tests(usb_idx);
    if (rc == PASSED) {
        prpass(testpass, "R/W Test Pass");
    } else {
        cterr('f', 0, "R/W Test Failed");
    }
    
    prcomplete(testpass, errcount, (char *)0);

    return (rc);
}


/*******************************************************************************
 *
 * Function   :    tabei_usb_slot_tests
 * Description:    entry point to usb device test
 * Inputs     :    usb_idx - usb index 
 * Outputs    : PASSED or FAILED.
 *
 *******************************************************************************
 */
int tabei_usb_slot_tests (int usb_idx)
{
    char src[32], usb_spd[15];
    int retval;
    char tname[50] = {0};
   
    /* Don't find the devname ex./dev/sda */
    if (strcmp(usb[usb_idx].dev_name, "") == 0) {
        cterr('f', 0, "Failed to find USB storage. Device name: %s", 
              usb[usb_idx].dev_name);
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
    char *tname = "USB";

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
Revision 1.2  2019/10/17 02:16:23  kehuang2
Collapse Tabei-L into main trunk

Revision 1.1.4.9  2019/07/29 06:13:52  kodko
Clean up code based on off-line code review

Revision 1.1.4.8  2019/06/20 09:05:59  kehuang2
Support linux_block_test function

Revision 1.1.4.7  2019/06/20 03:41:30  kodko
Fix pim usb3.0 flash can not be found issue when front usb is plugged with usb2.0 flash only by adding check usb device type.

Revision 1.1.4.6  2019/05/08 03:09:21  kodko
Support USB device random offset read/write test.

Revision 1.1.4.5  2019/01/18 02:30:15  olin2
Clean up code

Revision 1.1.4.4  2018/11/16 13:42:30  kodko
Support front USB hub and PIM USB hub connect with USB3.0 and USB2.0 storage read/write test.

Revision 1.1.4.3  2018/10/03 06:51:01  kodko
Initial bring up for P1A Tabei-L USB 2.0/3.0 read/write test.

Revision 1.1.4.2  2018/10/02 01:50:01  harrchan
Initial commit for Tabei-L P1A bring up.

$Endlog$
*/


