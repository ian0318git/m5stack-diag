/* $Id: diag_usb_test.c,v 1.2 2021/06/02 08:22:35 iachang Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/fugazi/diag_usb_test.c,v $
 *------------------------------------------------------------------
 *
 * Filename: diag_usb_test.c
 *           Fugazi USB type A / type C test function
 *
 * Copyright (c) 2019-2020 by cisco Systems, Inc.
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
#include "diag_common.h"
#include "monitor.h"
#include "menu.h"
#include "error.h"
#include "proto.h"
#include "diag_usb_test.h"
#include "nvmonvars.h"
#include "plat_defs.h"
#include "mb_tests.h"
#include "linux_api.h"
#include "linux_usb_test.h"
#include "dash_fpga.h"


/**********************************************************************
 * Function: fugazi_enable_usb3_ss
 *
 * Description : Config USB Super Speed Port Enable (SSPE) 
 *              
 * Inputs: enable - enable SSPE register
 *         mask - usb slot num
 * Output: PASSED/FAILED
 **********************************************************************
 */
int fugazi_enable_usb3_ss(int enable, unsigned int mask)
{
    struct pci_dev *pci;
    struct fugazi_mmap map;
    uint32_t sspe;
    uint32_t port_mask = mask & XHCI_SSPE_PORT_MASK;

    pci = fugazi_pci_dev_get(0, 0, 0x14, 0);
    if (pci == NULL) {
        pci = fugazi_pci_dev_get(0, 0, 0x14, 0);
        if (pci == NULL) {
            log_err("failed to find XHCI Controller\n");
            return (FAILED);
        }
    }

    map.paddr = (void *)pci->bar[0].address;
    map.length = pci->bar[0].size;

    if (fugazi_file_mmap(NULL, &map,
                         FUGAZI_MMAP_READ | FUGAZI_MMAP_WRITE) < 0) {
        log_err("failed to mmap XHCI MMIO space\n");
        pci_dev_put(pci);
        return (FAILED);
    }

    sspe = *(uint32_t *)(map.vaddr + XHCI_SSPE);
    if (!enable) {
        sspe &= ~port_mask;
    } else {
        sspe |= port_mask;
    }
    *(uint32_t *)(map.vaddr + XHCI_SSPE) = sspe;

    fugazi_file_munmap(&map);
    pci_dev_put(pci);

    return (PASSED);
}
/**********************************************************************
 * Function: fugazi_usb_2p0_mode_set
 *
 * Description : Enable USB 2.0 mode 
 *              
 * Inputs: port_mask - usb slot num
 *
 * Output: PASSED/FAILED
 **********************************************************************
 */
int fugazi_usb_2p0_mode_set(unsigned int port_mask)
{
    msleep(XHCI_CONFIG_WAITTIME);
    system(UNBIND_XHCI_CONTROLLER);
    msleep(SUPER_SPEED_WAITTIME);
    fugazi_enable_usb3_ss(DISABLE, port_mask);
    msleep(SUPER_SPEED_WAITTIME);
    system(BIND_XHCI_CONTROLLER);
    msleep(XHCI_CONFIG_WAITTIME);

    system(UDEVTRIGGER);
    msleep(UDEVTRIGGER_WAITTIME);

    if ((NVRAM)->diagflag & D_VERBOSE) {
        system(LSUSB_CMD);
    }

    return (PASSED);
}
/**********************************************************************
 * Function: fugazi_usb_3p0_mode_set
 *
 * Description : Enable USB 3.0 mode 
 *              
 * Inputs: port_mask - usb slot num
 *
 * Output: PASSED/FAILED
 **********************************************************************
 */

int fugazi_usb_3p0_mode_set(unsigned int port_mask)
{
    msleep(XHCI_CONFIG_WAITTIME);
    system(UNBIND_XHCI_CONTROLLER);
    msleep(WAIT_USBDRV_FILE_TIME);
    fugazi_enable_usb3_ss(ENABLE, port_mask);
    msleep(WAIT_USBDRV_FILE_TIME);
    system(BIND_XHCI_CONTROLLER);
    msleep(XHCI_CONFIG_WAITTIME);

    system(UDEVTRIGGER);
    msleep(UDEVTRIGGER_WAITTIME);

    if ((NVRAM)->diagflag & D_VERBOSE) {
        system(LSUSB_CMD);
    }
    return (PASSED);
}
/**********************************************************************
 * Function: usb_tests
 *
 * Description : usb r/w tests. 
 *              
 * Inputs: slot - usb slot num
 *
 * Output: PASSED/FAILED
 **********************************************************************
 */
int usb_tests (int slot)
{
    int retval = PASSED;
    char *tname = "USB slot";
    char buf[128] = "NULL";
    FILE *fp;
    char *check_usb3_file = "/fugazi-diag/usb_speed.txt";
    char check_usb2_spd[] = "480";
    uint32_t usb_port_mask;

    usb_port_mask = (slot == 0) ? FUGAZI_USB_PORT_MASK_FRONT_A :
                                  FUGAZI_USB_PORT_MASK_FRONT_C;

    testname("%s%d access", tname, slot);

    if (((slot == 0) && check_skip_test(mb_skip_item_name[USB0_SK])) ||
	    ((slot == 1) && check_skip_test(mb_skip_item_name[USB1_SK]))) {
        prpass(testpass, "Test skipped by user. ");
        return(PASSED);
    }
#ifdef WORK_AROUND
    /* Disable power to non-tested USB port to avoid interference */
    /* CSCvp45238 : Work around USB3.0 default speed issue */
    system(UNBIND_XHCI_CONTROLLER);
    msleep(WAIT_USBDRV_FILE_TIME);
    reset_plat_dev(FPGA_RST_USB0_DIS); /* Disable USB slot-0 power */
    reset_plat_dev(FPGA_RST_USB1_DIS); /* Disable USB slot-1 power */
    msleep(XHCI_CONFIG_WAITTIME);
    if (slot == 0) {
        /* power on USB slot-0 */
        unreset_plat_dev(FPGA_RST_USB0_DIS);
    } else {
        /* power on USB slot-1 */
        unreset_plat_dev(FPGA_RST_USB1_DIS);
    }
    msleep(XHCI_CONFIG_WAITTIME);
    system(BIND_XHCI_CONTROLLER);
    msleep(WAIT_USBDRV_FILE_TIME);
    system(UDEVTRIGGER);
    msleep(UDEVTRIGGER_WAITTIME);
    system(SUPPRESS_MESG);
    system(REMOVE_USBSPD_FILE);
#endif
    if (usb_exist(slot) != PASSED) {
        cterr('f',0, "Can't find USB slot-%d device node.", slot);
        fflush(stdout);
        return (FAILED);
    }

    switch(slot) {
    case 0:
        system(GET_USB0_SPEED);
        break;
    case 1:
        system(GET_USB1_SPEED);
        break;
    }

    fp = fopen(check_usb3_file, "r");
    if (fp == NULL) {
        cterr('f',0, "Can't find USB3 speed file %s.", check_usb3_file);
        return (FAILED);
    }

    fgets(buf, sizeof(buf), fp);
    fclose(fp);
    system(REMOVE_USBSPD_FILE);

    if (strstr(buf, check_usb2_spd) != NULL) {
        printf("Detected USB2.0 speed (480Mbit/s) in slot-%d.\n", slot);
        fflush(stdout);
        cterr('f', 0, "USB2.0 in slot-%d, expected USB3.0.", slot);
        return FAILED;
    }

    /* First time USB3.0 test */
    prpass(testpass, "USB slot%d host XHCI controller default (USB3.0) test\n", 
           slot);
    fflush(stdout);

    retval = fugazi_usb_slot_tests(slot);
    if (retval == FAILED) {
        cterr('f',0, "USB slot%d host XHCI controller default run failed.", slot);
        fflush(stdout);
        return (FAILED);
    }

    prpass(testpass, "USB slot%d host XHCI controller disable super speed "
           "(USB2.0) test\n", slot);
    fflush(stdout);

    fugazi_usb_2p0_mode_set(usb_port_mask);

    /* Check USB device available after disable USB super speed */
    if (usb_exist(slot) != PASSED) {
        cterr('f',0, "USB slot%d not available after disable USB3.0 super speed."
              , slot);
        fflush(stdout);
        retval = FAILED;
        goto out;
    }

    /* Second time USB 2.0 test - disable super speed run */
    retval = fugazi_usb_slot_tests(slot);
    if (retval == FAILED) {
        cterr('f',0, "USB slot%d host XHCI controller disable super speed run "
              "failed.", slot);
        fflush(stdout);
        retval = FAILED;
        goto out;
    }


out:
    fugazi_usb_3p0_mode_set(usb_port_mask);

    if (usb_exist(slot) != PASSED) {
        cterr('f', 0, "failed to back to XHCI USB3.0\n");
        retval = FAILED;
    }
    printf("USB slot%d XHCI super speed enabled\n", slot); 
    fflush(stdout);

    msleep(DELAY_USBCMD);
    system(OPEN_MESG);

    return (retval);
}

/*
 * Function: usb_exist
 *
 * Description : Check USB device is available. 
 *              
 * Inputs: slot - usb slot num
 *
 * Output: PASSED/FAILED
 */
int usb_exist (int slot)
{
    char check_usbdrv[50];
    char check_drvnode[64];
    int devfd, ix;
    size_t size = 0;
    sprintf(check_usbdrv, "/dev/usbdrv%d", slot);
    sprintf(check_drvnode, "ls -l %s", check_usbdrv);

    for (ix = 0; ix < CHECK_USBDRV_FILE_TIME; ix++) {
        if (file_exist(check_usbdrv, &size)) {
            printf("%s exist\n", check_usbdrv);
            fflush(stdout);
            break;
        } else {
            msleep(WAIT_USBDRV_FILE_TIME);
        }
    }
    if (ix == CHECK_USBDRV_FILE_TIME) {
        printf("%s does NOT exist\n", check_usbdrv);
        fflush(stdout);
        system(check_drvnode);
        return (FAILED);
    }

    for (ix = 0; ix < CHECK_USBDRV_FILE_TIME; ix++) {
        devfd = open(check_usbdrv, O_RDWR);
        if(devfd < 0) {
            system(UDEVTRIGGER);
            msleep(UDEVTRIGGER_WAITTIME * 10);
            close(devfd);
            continue;
        } else {
            break;
        }
    }
    if (devfd < 0) {
        close(devfd);
        printf("Device %s open failed. Is USB slot%d vacant?\n", check_usbdrv, 
                slot);
        fflush(stdout);
        return (FAILED);
    } else {
        close(devfd);
        return (PASSED);
    }
}


/*-------------------------------------------------
 * $Log: diag_usb_test.c,v $
 * Revision 1.2  2021/06/02 08:22:35  iachang
 * CSCvo59196-33 : Merge Fugazi from ASR1K-main-branch to ISR main trunk
 *
 * Revision 1.1.8.2  2020/08/26 02:37:49  iachang
 * Merge Fugazi code into main trunk
 *
 * Revision 1.1.6.7  2020/08/03 09:25:40  iachang
 * Code clean up.
 *
 * Revision 1.1.6.6  2020/04/24 07:13:16  iachang
 * The block device access test, used fix size to replace malloc_usable_size()
 *
 * Revision 1.1.6.5  2020/01/14 01:49:42  iachang
 * Removed "Ext. loopback ON" flag in USB test.
 *
 * Revision 1.1.6.4  2019/06/18 06:15:09  iachang
 * Removed work around USB3.0 default speed issue
 *
 * Revision 1.1.6.3  2019/05/13 01:53:41  iachang
 * CSCvp45238 : Fixed USB3.0 default speed issue
 *
 * Revision 1.1.6.2  2019/03/14 03:48:35  letsai
 * Initial check in.
 *
 *
 *
 *
 * $Endlog$
 * 
 */
