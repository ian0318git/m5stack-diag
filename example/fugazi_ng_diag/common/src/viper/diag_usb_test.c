 /* $Id: diag_usb_test.c,v 1.3 2018/09/21 02:48:54 harrchan Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/viper/diag_usb_test.c,v $
 *------------------------------------------------------------------
 *
 * Filename: diag_usb_test.c
 *
 * Copyright (c) 2013-2018 by cisco Systems, Inc.
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
#include "diag_storage_lib.h"


/*
 * Global extern functions
 */

/*
 * Declare local function
 */
struct usb_info_t usb[4];
int usb_slot_tests(int);
int diag_ext_usb_test(int);
static int usb_tests(int n);

static submenu_xtable_t usbtest_menu_table[] = {   
    {"External USB 2.0 test", (PFT) usb_tests, USB_SLOT0,
    MF_CONTINUOUS | MF_DOGRP | MF_DOALL | MF_SHOW_ERRCOUNT,
    (type_t(*)())0, 0, (PFT) 0, 0},
#ifdef USB_30_SUPPORT
    {"External USB 3.0 test", (PFT) usb_tests, USB_SLOT1,
    MF_CONTINUOUS | MF_DOGRP | MF_DOALL | MF_SHOW_ERRCOUNT,
    (type_t(*)())0, 0, (PFT) 0, 0},
#endif
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
 * Inputs     :    usb slot num
 * Outputs    : PASSED or FAILED.
 *
 *******************************************************************************
 */

int usb_tests (int slot)
{
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
    cterr_add_component("Intel Denverton", "USB2.0", "USB2.0 Port");
#ifdef USB_30_SUPPORT
    cterr_add_component("Intel Denverton", "USB3.0", "USB3.0 Port");
#endif
    /* Segment 5: register and memory dump */

    /* Segment 6: Platform Environment initialized here*/

    /* Segment 7: Top 3 Debugging Steps */
    cterr_add_debug("Check the interface between the Host SoC and the USB port.",
                    "If there is no problem for these interfaces, "
                    "replace one USB device and redo the test.");
    int usb_speed = 0;
    int rc = FAILED;
    char test_name[128] = {0};
    char *tname = test_name;

    if ( slot == USB_SLOT0 ) {
        tname = "External USB 2.0";
#ifdef USB_30_SUPPORT
    } else if (slot == USB_SLOT1) {
        tname = "External USB 3.0";
#endif
    } else {
        cterr('f', 0, "Unsupported slot!");
        return (FAILED);
    }

    testname(tname);
    prpass(testpass, "%s, ", tname);

    /* External loopback only for external USB stick*/
    if (slot == USB_SLOT0 || slot == USB_SLOT1) {
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
    if (usb_get_info() == FAILED) {
        printf("usb_get_info() failed\n");
    }

    usb_speed = usb_get_speed(slot);
#ifdef USB_30_SUPPORT
    if (usb_speed != USB2 && usb_speed != USB3) {
#else
    if (usb_speed != USB2) {
#endif
        cterr('f', 0, "USB setting failed (%d)", usb_speed);
        return (FAILED);
    } else {
        rc = usb_slot_tests(slot);
        if (rc == PASSED) {
            if (usb_speed == USB2) {
                prpass(testpass, "%s 2.0 read/write test passed, ", tname);
            }
#ifdef USB_30_SUPPORT
            else {
                prpass(testpass, "%s 3.0 read/write test passed, ", tname);
            }
#endif
        } else {
            if (usb_speed == USB2) {
                cterr('f', 0, "USB 2.0 read/write test failed (%d)", usb_speed);
            }
#ifdef USB_30_SUPPORT
            else {
                cterr('f', 0, "USB 3.0 read/write test failed (%d)", usb_speed);	
                return (FAILED);
            }
#endif
        }
    }

    prcomplete(testpass, errcount, (char *)0);
    return (rc);
}


/*******************************************************************************
 *
 * Function   :    usb_slot_tests
 * Description:    entry point to usb device test
 * Inputs     :    usb slot
 * Outputs    : PASSED or FAILED.
 *
 *******************************************************************************
 */
int usb_slot_tests (int slot)
{
    char src[32], usb_spd[15];
    int retval;
    char test_name[128] = {0};
    char *tname = test_name;

    /* don't find the devname ex./dev/sda */
    if (strcmp(usb[slot].dev_name, "") == 0) {
        cterr('f',0,"Failed to find USB storage.");
        return (FAILED);
    }

#ifdef USB_30_SUPPORT
    if (usb[slot].spd == 5000 ) {
        sprintf(usb_spd, "%s", "[USB 3.0] ");
    } else {
        sprintf(usb_spd, "%s", "[USB 2.0] ");
    }
#else
    sprintf(usb_spd, "%s", "[USB 2.0] ");
#endif

    if( slot == USB_SLOT0 ) {
        tname = "External USB (2.0 port)";
#ifdef USB_30_SUPPORT
    } else if ( slot == USB_SLOT1 ) {
        tname = "External USB (3.0 port)";
#endif
    } else if ( slot == USB_SLOT2 ) {
        tname = "Internal USB (LTE 0)";
    } else if ( slot == USB_SLOT3 ) {
        tname = "Internal USB (LTE 1)";
    } else {
        cterr('f',0,"Unsupported Slot!");
        return (FAILED);
    }

    sprintf(src, "/dev/%s", usb[slot].dev_name);
    retval = access_device_test(src);

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
#ifdef USB_30_SUPPORT
    char *tname = "Test USB 2.0 and 3.0";
#else
    char *tname = "Test USB 2.0";
#endif
    
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
Revision 1.3  2018/09/21 02:48:54  harrchan
Merge viper DSL to the main trunk (CSCvm57542)

Revision 1.2  2018/08/06 02:31:51  harrchan
Merge viper E2E to the main trunk (CSCvk28469)

Revision 1.1.2.5  2018/07/06 02:54:08  harrchan
Add enhance error message

Revision 1.1.2.4  2018/07/03 05:38:55  harrchan
Follow the coding rule to clean up code

Revision 1.1.2.3  2018/06/13 01:54:14  lucywang
Removed USB 3.0 support

Revision 1.1.2.2  2018/03/27 07:12:10  lucywang
Modified USB test for 2.0 and 3.0

Revision 1.1.2.1  2018/02/27 08:06:47  harrchan
Initial viper application code base



$Endlog$
*/


