 /* $Id: diag_usb_test.c,v 1.2 2019/12/11 10:10:31 lucywang Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/nanook/diag_usb_test.c,v $
 *------------------------------------------------------------------
 *
 * Filename: diag_usb_test.c
 *
 * Copyright (c) 2019 by cisco Systems, Inc.
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
#include "linux_block_test.h"

/*
 * Global extern functions
 */

/*
 * Declare local function
 */
struct usb_info_t usb[4];
int nanook_usb_slot_tests(int);
int diag_ext_usb_test(int);
static int usb_tests(int n);

static submenu_xtable_t usbtest_menu_table[] = {   
    {"External Back USB 2.0 test", (PFT) usb_tests, BACK_USB_EXT_PORT_20,
    MF_CONTINUOUS | MF_DOGRP | MF_DOALL | MF_SHOW_ERRCOUNT,
    (type_t(*)())0, 0, (PFT) 0, 0},
    {"External Back USB 3.0 test", (PFT) usb_tests, BACK_USB_EXT_PORT_30,
    MF_CONTINUOUS | MF_DOGRP | MF_DOALL | MF_SHOW_ERRCOUNT,
    (type_t(*)())0, 0, (PFT) 0, 0},
    {"External Front USB 2.0 test", (PFT) usb_tests, FRONT_USB_EXT_PORT_20,
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
 * Inputs     :    usb slot num
 * Outputs    : PASSED or FAILED.
 *
 *******************************************************************************
 */

int usb_tests (int slot)
{
#ifdef ENHANCE_ERROR_MSG_RDY
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
    cterr_add_component("Intel Denverton", "USB3.0", "USB3.0 Port");
    /* Segment 5: register and memory dump */
    cterr_add_reg_dump((PFV)bcm63168_get_xdsl_profile);

    /* Segment 6: Platform Environment initialized here*/
    cterr_add_env_dump((PFV)sky_display_temp_errormsg);

    /* Segment 7: Top 3 Debugging Steps */
    cterr_add_debug("Check the interface between the Host SoC and the USB port.",
                    "If there is no problem for these interfaces, "
                    "replace one USB device and redo the test.");
#endif
    int usb_idx;
    int rc = FAILED, ix;
    char tname[50] = {0};
    struct usb_info_t back_usb2_storage[USB_DEVICE_MAX_NUM]; 
    struct usb_info_t back_usb3_storage[USB_DEVICE_MAX_NUM]; 
    struct usb_info_t front_usb2_storage[USB_DEVICE_MAX_NUM];

    memset(back_usb2_storage, 0, sizeof(back_usb2_storage));
    memset(back_usb3_storage, 0, sizeof(back_usb3_storage));
    memset(front_usb2_storage, 0, sizeof(front_usb2_storage));

    if (slot == BACK_USB_EXT_PORT_20) {
        strcpy(tname, "External Back USB 2.0");
    } else if ((slot == BACK_USB_EXT_PORT_30)) {
        strcpy(tname, "External Back USB 3.0");
    } else if ((slot == FRONT_USB_EXT_PORT_20)) {
        strcpy(tname, "External Front USB 2.0");
    }

    testname(tname);
    prpass(testpass, "%s, ", tname);

    /* External loopback only for external USB stick*/
    if (slot == FRONT_USB_EXT_PORT_20 || slot == BACK_USB_EXT_PORT_30 || 
        slot == BACK_USB_EXT_PORT_20) {
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
        printf("usb_parse_info() failed\n");
        return (FAILED);
    }
    
    if (slot == BACK_USB_EXT_PORT_20) {
        /* Check if the USB 2.0 storage device is detected or not */
        for (ix = 0; ix < USB_DEVICE_MAX_NUM; ix++) {
            if (get_usb_storage_info(BACK_USB, USB_HOST20_SPEED, 
                                     &back_usb2_storage[ix]) == FAILED) {
                printf("%s() can not get USB storage information\n", __FUNCTION__);
            }
            if (back_usb2_storage[ix].found == FALSE) {
                break;
            }
        }
        
        if (find_back_usb_storage_index(USB_HOST20_SPEED, &usb_idx) == FALSE) {
            cterr('f', 0, "Can not find USB2.0 storage");
            return (FAILED);
        }
    } else if (slot == BACK_USB_EXT_PORT_30) {
        /* Check if the USB 3.0 storage device is detected or not */
        for (ix = 0; ix < USB_DEVICE_MAX_NUM; ix++) {
            if (get_usb_storage_info(BACK_USB, USB_HOST30_SPEED, 
                                     &back_usb3_storage[ix]) == FAILED) {
                printf("%s() can not get USB storage information\n", __FUNCTION__);
            }
            if (back_usb3_storage[ix].found == FALSE) {
                break;
            }
        }
        
        if (find_back_usb_storage_index(USB_HOST30_SPEED, &usb_idx) == FALSE) {
            cterr('f', 0, "Can not find USB3.0 storage");
            return (FAILED);
        }
    } else if (slot == FRONT_USB_EXT_PORT_20) {
        for (ix = 0; ix < USB_DEVICE_MAX_NUM; ix++) {
            if (get_usb_storage_info(FRONT_USB, USB_HOST20_SPEED,
                                     &front_usb2_storage[ix]) == FAILED) {
                printf("%s() can not get USB storage information\n", __FUNCTION__);
            }
            if (front_usb2_storage[ix].found == FALSE) {
                break;
            }
        }
        
        if (find_front_usb_storage_index(USB_HOST20_SPEED, &usb_idx) == FALSE) {
            cterr('f', 0, "Can not find USB2.0 storage");
            return (FAILED);
        }
    }
    
    rc = nanook_usb_slot_tests(usb_idx);
    if (rc == PASSED) {
        prpass(testpass, "R/W Test Pass\n");
    } else {
        cterr('f', 0, "R/W Test Failed");
    }

    return (rc);
}


/*******************************************************************************
 *
 * Function   :    nanook_usb_slot_tests
 * Description:    entry point to usb device test
 * Inputs     :    usb slot
 * Outputs    : PASSED or FAILED.
 *
 *******************************************************************************
 */
int nanook_usb_slot_tests (int usb_idx)
{
    char src[32], usb_spd[15];
    int retval;
    char tname[50] = {0};
    
    /* Don't find the devname ex./dev/sda */
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
Revision 1.2  2019/12/11 10:10:31  lucywang
Merged Nanook to main trunk


$Endlog$
*/


