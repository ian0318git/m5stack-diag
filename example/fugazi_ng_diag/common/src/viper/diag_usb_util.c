 /* $Id: diag_usb_util.c,v 1.2 2018/08/06 02:31:51 harrchan Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/viper/diag_usb_util.c,v $
 *------------------------------------------------------------------
 *
 * Filename: diag_usb_util.c
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
#include "linux_usb_test.h"
#include "nvmonvars.h"
#include "queryflags.h"
#include "plat_defs.h"
#include "diag_usb_util.h"
#include "diag_usb_lib.h"
#include "diag_lte_lib.h"
#include "diag_fpga_lib.h"
#include "diag_fpga.h"
#include "viper_comm.h"

/*
 * Declare local function
 */
static int usb_dump(int);
static int usb_dump_lte(int);

/*
 * Declare Global function
 */
extern int ExecuteCmdbyPopen(char *cmd, char *retBuf, int sizeOfBuf);
extern int diag_lte_pwr_on(int);

static submenu_xtable_t usbdev_menu_table[] = {
    {"enumerate external USB 2.0 port", (PFT) usb_dump, USB_SLOT0,
     MF_CONTINUOUS | MF_DOGRP | MF_DOALL | MF_SHOW_ERRCOUNT,
     (type_t(*)())0, 0, (type_t(*)())0, 0},
#ifdef USB_30_SUPPORT
    {"enumerate external USB 3.0 port", (PFT) usb_dump, USB_SLOT1,
     MF_CONTINUOUS | MF_DOGRP | MF_DOALL | MF_SHOW_ERRCOUNT,
    (type_t(*)())0, 0, (type_t(*)())0, 0},
#endif
    {"enumerate internal USB port0 (LTE 0)", (PFT) usb_dump_lte, USB_SLOT3,
     MF_CONTINUOUS | MF_DOGRP | MF_DOALL | MF_SHOW_ERRCOUNT,
     (type_t(*)())has_lte_sku, 0, (type_t(*)())0, 0},
};


#define USB_MENU_TABLE_SZ \
        (sizeof(usbdev_menu_table) / sizeof(submenu_xtable_t))

/*
 * primary & secondary submenu items (filled in from xtable)
 */
static mitem_t usbdev_pri_items[USB_MENU_TABLE_SZ + MAX_BASE_ITEMS];
static mitem_t usbdev_sec_items[USB_MENU_TABLE_SZ + MAX_BASE_ITEMS];

static struct menuinfo usbdev_menu = {
    "USB Main Menu",
    0,                          /* mtparam added by init_empty_menu */
    (PFT) menu_show_dflags,     /* notes missing WICs in combos */
    0,                          /* use generic prompt */
    0,                          /* size (bumped by add_menu_item() */
    usbdev_pri_items,
};

static struct menuinfo *usb_menup = &usbdev_menu;


/*******************************************************************************
 *
 * Function   :    usb_utils
 * Description:    show usb utility subment
 * Inputs     :    show_menu, flag set if submenu is selected
 * Outputs    : PASSED
 *
 *******************************************************************************
 */
int usb_utils (int show_menu)
{
    char *tname = "Enumerate all USB";

    testname(tname);

    build_primary_submenu(usbdev_menu_table, USB_MENU_TABLE_SZ,
                          "USB", &usb_menup);
    build_secondary_submenu(usbdev_menu_table, USB_MENU_TABLE_SZ,
                          usbdev_sec_items);

    if (show_menu) {
        menu(&usbdev_menu, usbdev_sec_items, '\0');
    } else {
        exec_doall_menu_items(&usbdev_menu);
    }

    return (PASSED);
}

/***************************************************************************** *
 * Function   : usb_dump
 * Description: main entry to display usb info.
 * Inputs     : slot - usb slot
 * Outputs    : PASSED
 *
 ******************************************************************************/
static int usb_dump (int slot)
{
    if(usb_get_info() == FAILED) {
        cterr('f', 0, "usb_get_info() failed");
        return (FAILED);
    }
    usb_display(slot);
    return (PASSED);
}

/***************************************************************************** *
 * Function   : usb_dump_lte
 * Description: Display LTE debug port info.
 * Inputs     : slot - usb slot
 * Outputs    : PASSED
 *
 ******************************************************************************/
static int usb_dump_lte (int slot)
{
    if (fpga_reset_api(FPGA_EXTER_DEV_RST_REG, EXT_PRI_LTE_RESET,
                       FALSE, WAITTIME_20_MS) != PASSED) {
        cterr('f', 0, "%s: Failed to release LTE from Reset.\n", __FUNCTION__);
    }
    diag_lte_pwr_on(TRUE);
    if(usb_get_info() == FAILED) {
        cterr('f', 0, "usb_get_info() failed");
        return (FAILED);
    }
    usb_display(slot);
    diag_lte_pwr_on(FALSE);
    return (PASSED);
}

/***************************************************************************** *
 * Function   : usb_test_mode
 * Description: set USB port to generate test pattern.
 * Inputs     : option for future use
 * Outputs    : PASSED
 *
 ******************************************************************************/
int usb_test_mode (int option)
{
    int ret = 0;
    uint reg_val = 0, reg_int = 0;
    char prefix[512], reg[32];
    
    memset(prefix, 0, sizeof(prefix));
    memset(reg, 0, sizeof(reg));
    ret = getdec_answer("0-Normal Mode, 1-J-State, 2-K-State, 3-SE0-NAK, 4-Test-Packet", 0, 0, 4);
    if(ret > 0) {
        ExecuteCmdbyPopen("memm=`cat /proc/iomem | grep xhci-hcd | awk '{print $1}' | awk -F - '{print $2}'`; expr substr $memm 1 4 | tr -d '\n'", prefix, 512);
        sprintf(reg, "%s%s", prefix, USB_PORT_CTRL);
        reg_int = strtoul(reg, NULL, 16); 
        if (viper_mem_write32(reg_int, 0x80) != PASSED) {
            printf("Failed to Write CPU register 0x%08X.\n", reg_int);
            system("echo -n \"0000:00:15.0\" | tee /sys/bus/pci/drivers/xhci_hcd/unbind > /dev/null; sleep 2; echo -n \"0000:00:15.0\" | tee /sys/bus/pci/drivers/xhci_hcd/bind > /dev/null");
            return (FAILED);
        }
    }
    switch (ret) {
    case 1:
        reg_val = 0x10000000;
    break;
    case 2:
        reg_val = 0x20000000;
    break;
    case 3:
        reg_val = 0x30000000;
    break;
    case 4:
        reg_val = 0x40000000;
    break;
    default:
        memset(reg, 0, sizeof(reg));
        sprintf(reg, "%s%s", prefix, USB_PORT_CTRL_TEST);
        reg_int = strtoul(reg, NULL, 16); 
        viper_mem_write32(reg_int, 0x0);
        system("echo -n \"0000:00:15.0\" | tee /sys/bus/pci/drivers/xhci_hcd/unbind > /dev/null; sleep 2; echo -n \"0000:00:15.0\" | tee /sys/bus/pci/drivers/xhci_hcd/bind > /dev/null");
        return (PASSED);
    }
    
    memset(reg, 0, sizeof(reg));
    sprintf(reg, "%s%s", prefix, USB_PORT_CTRL_TEST);
    reg_int = strtoul(reg, NULL, 16); 
    if (viper_mem_write32(reg_int, reg_val) != PASSED) {
        printf("Failed to read CPU register 0x%08X.\n", reg_int);
        system("echo -n \"0000:00:15.0\" | tee /sys/bus/pci/drivers/xhci_hcd/unbind > /dev/null; sleep 2; echo -n \"0000:00:15.0\" | tee /sys/bus/pci/drivers/xhci_hcd/bind > /dev/null");
        return (FAILED);
    } 
    return (PASSED);
}

/*-------------------------------------------------
 * $Log: diag_usb_util.c,v $
 * Revision 1.2  2018/08/06 02:31:51  harrchan
 * Merge viper E2E to the main trunk (CSCvk28469)
 *
 * Revision 1.1.2.8  2018/07/03 05:38:55  harrchan
 * Follow the coding rule to clean up code
 *
 * Revision 1.1.2.7  2018/06/28 11:20:36  lucywang
 * Added power on/off of "enumerate internal USB port0 (LTE 0)"
 *
 * Revision 1.1.2.6  2018/06/27 06:27:52  harrchan
 * Follow the coding rule to clean up code
 *
 * Revision 1.1.2.5  2018/06/13 08:43:33  lucywang
 * Only show "enumerate internal USB port0 (LTE 0)" for LTE SKU
 *
 * Revision 1.1.2.4  2018/06/13 01:54:14  lucywang
 * Removed USB 3.0 support
 *
 * Revision 1.1.2.3  2018/05/10 09:04:37  lucywang
 * Added USB 2.0 test mode utility
 *
 * Revision 1.1.2.2  2018/03/27 07:12:10  lucywang
 * Modified USB test for 2.0 and 3.0
 *
 * Revision 1.1.2.1  2018/02/27 08:06:48  harrchan
 * Initial viper application code base
 *
 *
 * $Endlog$
 *-------------------------------------------------
 */

