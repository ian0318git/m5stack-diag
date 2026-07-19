/* $Id: usb_dongle_common_test.c,v 1.2 2019/06/14 09:59:33 steja Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/usb_dongle/usb_dongle_common/usb_dongle_common_test.c,v $
 *------------------------------------------------------------------
 *
 * usb_dongle_common_test.c - Common Test functions for USB dongles.
 *
 *
 * Copyright (c) 2015-2019 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */
#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include "common.h"
#include "error.h"
#include "types.h"
#include "menu.h"
#include "nvmonvars.h"
#include "usb_dongle_common_test.h"
#include "usb_dongle_common_lib.h"
#include "usb_dongle_common_host.h"
#include "usb_dongle_host_slot_modules.h"
#include "mb_tests.h"

/* Prototype */
int usb_dongle_test_entry(int);
int usb_dongle_intf_test(char *, int);
static int usb_dongle_full_test(char *, int);
static int usb_dongle_test(struct udongle_intf_t *, int, char *, int);

extern void reset_errmsg_var(void);

/* Global */
static struct udongle_intf_t udongle;


/*------------------------------------------------------------------------------
 * Function:    usb_dongle_full_test
 * Description: Main entry function of USB Dongle Test
 * Inputs:      usb_devinfo - USB port number
 *              build_menu_flag - Flag to determine whether to build and display
 *                                USB dongle sub-menu
 * Outputs:     PASSED/FAILED
 *------------------------------------------------------------------------------
 */
static int usb_dongle_full_test (char *usb_devinfo, int build_menu_flag)
{
    return (usb_dongle_test(&udongle, build_menu_flag, usb_devinfo, 
                            FULL_TEST));
}

/*------------------------------------------------------------------------------          
 * Function:    usb_dongle_intf_test
 * Description: Main Entry function of USB Dongle Interface Test
 * Inputs:      usb_devinfo - USB port number
 *              build_menu_flag - Flag to determine whether to build and display
 *                                USB dongle sub-menu
 * Outputs:     PASSED/FAILED
 *------------------------------------------------------------------------------
*/
int usb_dongle_intf_test (char *usb_devinfo, int build_menu_flag)
{
    return (usb_dongle_test(&udongle, build_menu_flag, usb_devinfo, 
                            IFACE_TEST));
}

/*------------------------------------------------------------------------------
 * Function:    usb_dongle_test
 * Description: Invoke the diagnostic for the USB Dongle Test
 * Inputs:      udongle - ptr to struct udongle_intf_t
 *              build_menu_flag - Flag to determine whether to build and display
 *                                USB dongle sub-menu
 *              usb_devinfo - USB port number
 *              test_type - Full test or interface test
 * Outputs:     PASSED/FAILED
 *------------------------------------------------------------------------------
*/
static int usb_dongle_test (struct udongle_intf_t *udongle, int build_menu_flag, 
                            char *usb_devinfo, int test_type)
{
    char usb_mod_str[] = USB_MOD_STR;
    int test_err;

    /* Flag set if submenu is invoked */
    if (build_menu_flag == TRUE) {       /* User opted for submenus */
        udongle->menu_display = TRUE;
    } else {
        udongle->menu_display = FALSE;
    }
    
    udongle->port = usb_devinfo;

    /* Clear the test's cterr info setup before each usb dongle test */
    reset_errmsg_var();
    testname("%s USB Port %s", usb_mod_str, usb_devinfo);
    prpass(testpass, " "); /* Zero out the testpass buffer */

    /* Get USB Dongle module information */
    if (usb_dongle_get_module_entry_ptr(udongle) == FAILED) {
        prcomplete(testpass, errcount, 0);
        return (FAILED);
    }

    udongle->test_type = test_type;

    /* Invoke the diagnostic */
    if (test_type == FULL_TEST) {
        test_err = udongle->diag((void *)udongle);
    } else {
        if (udongle->intf_diag) {
            test_err = udongle->intf_diag((void *)udongle);
        } else {
            test_err = PASSED;
            cterr('w', 0, "No interface test available.");
        }
    }

    if (test_err == FAILED) {
        return (test_err);
    }

    return (test_err);
}


/*------------------------------------------------------------------------------
 * Function: usb_dongle_test_entry
 * Description : Entry function to test USB dongle.
 *               However, if there's a USB mass storage device connected, jump
 *               to the corresponding test function in host side to verify
 *               platform USB interface.
 * Inputs: slot - usb slot num
 * Output: PASSED/FAILED
 *------------------------------------------------------------------------------
 */
int usb_dongle_test_entry (int slot)
{
    int rc = FAILED, build_menu_flag = 0;
    int ret = FAILED, t_diff = 0;
    struct timeval t_curr;
    struct timeval usb_dongle_test_start_t;
    int usb_speed = 0;
    int max_slot_no, usb_2p0_bus_no, usb_3p0_bus_no, usb_lev_no, usb_idx;
    char usb_devinfo[32], usb_2p0_devinfo[32], usb_3p0_devinfo[32];

    /* Check if external loopback flag is enable */
    /*
     * D_EXT_LOOPBACK = 0, enable ext. loopback
     * D_EXT_LOOPBACK = 1, disable ext. loopback
     */
    if (check_menu_flag(D_EXT_LOOPBACK)) {
        prpass(testpass, "\n External loopback flag is off, skip \
                          external USB slot %d test. ", slot);
        prcomplete(testpass, errcount, (char *)0);
        return (PASSED);
    }

    /* Check if user's going to display sub-menu */
    if (usb_dongle_host_get_max_usb_slot_no(&max_slot_no) != PASSED) {
        cterr('f', 0, "%s:Failed to get max USB slot number.", __func__);
        return (FAILED);
    }
    if (slot >= max_slot_no) {
        build_menu_flag = 1;
        slot = slot - max_slot_no;
    }

    /* Get the USB bus number & level number for tested slot */
    if ((usb_dongle_host_get_usb_bus_no(slot, &usb_2p0_bus_no, &usb_3p0_bus_no) 
         != PASSED) ||
        (usb_dongle_host_get_usb_lev_no(slot, &usb_lev_no) != PASSED)) {
        cterr('f', 0, "%s:Failed to get USB bus & level number.", __func__);
        return (FAILED);
    }

    /* Based on WP76xx product spec., enumeration starts within 14.5-15.5
     * seconds. Due to Sierra WP76xx modems need time to complete enumeration,
     * we poll USB device information with 15.5 seconds timeout */
    gettimeofday(&usb_dongle_test_start_t, NULL);
    while (t_diff < USB_DONGLE_MAX_ENUMERATE_TIME) {
        gettimeofday(&t_curr, NULL);
        t_diff = (t_curr.tv_sec - usb_dongle_test_start_t.tv_sec);
        /* Save USB info into USB structure */
        if (usb_dongle_parse_info() == FAILED) {
            cterr('f', 0, "Failed to parse USB info");
            return (FAILED);
        }

        /* Check whether the USB device is detected or not */
        ret = usb_dongle_dev_present_by_bus_lev(usb_2p0_bus_no, usb_3p0_bus_no,
                                                usb_lev_no, &usb_idx);
        if (ret == TRUE) {
            break;
        }
        sleep(DELAY_1_SEC);
    }
    if (ret == FALSE) {
        cterr('f', 0, "USB external device is not detected in Slot %d", slot);
        return (FAILED);
    }

    /* Check if the tested USB device is a mass storage device */
    if (usb_dongle_dev_is_mass_storage(usb_idx) == TRUE) {
        if (build_menu_flag) {
            printf("\nSubtest menu not available for this double character\n");
            return (PASSED);
        } else {  /* USB mass storage read/write test */
            rc = usb_storage_rd_wr_tests(slot);
        }
    } else {
        if (usb_dongle_host_get_usb_devinfo(slot, usb_2p0_devinfo,
                                            usb_3p0_devinfo) != PASSED) {
            cterr('f', 0, "%s:Failed to get USB device info.", __func__);
            return (FAILED);
        }

        usb_speed = usb_dongle_get_bus_speed(usb_idx);
        if (usb_speed == USB_2P0_SPEED) {
            sprintf(usb_devinfo, "%s", usb_2p0_devinfo);
        } else {
            sprintf(usb_devinfo, "%s", usb_3p0_devinfo);
        }

        rc = usb_dongle_full_test(usb_devinfo, build_menu_flag);
    }
        
    prcomplete(testpass, errcount, (char *)0);
    return (rc);
}


/*-------------------------------------------------
$Log: usb_dongle_common_test.c,v $
Revision 1.2  2019/06/14 09:59:33  steja
Supported Cooper usb dongle LTE



*/
