/* $Id: diag_usb_test.c,v 1.2 2019/01/10 06:36:24 wilbhuan Exp $ 
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/betelgeuse/diag_usb_test.c,v $
 *------------------------------------------------------------------
 * 
 * diag_usb_test.c
 *
 * Copyright (c) 2018 - 2019 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/un.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <linux/types.h>
#include <linux/netlink.h>
#include <errno.h>
#include <unistd.h>
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
#include "setjmps.h"
#include "proto.h"
#include "diag_enhance_err_msg_lib.h"
#include "platform_cookie.h"
#include "diag_usb_lib.h"
#include "diag_usb_test.h"

extern int show_plat_curr_temps(void);
static int init_hotplug_sock(void);

/*
 * Function: diag_usb_test
 *
 * Description : usb r/w tests.
 *
 * Inputs: slot - usb slot num
 *
 * Output: PASSED/FAILED
 */

int diag_usb_test (int slot)
{
    uchar mb_get_loc[FRU_SIZE] = {0};
    uchar mb_get_pid[FRU_SIZE] = {0};
#ifdef ENHANCE_ERROR_MSG_RDY
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
    cterr_add_component("Marvell Armada 7040", "USB3.0", "USB3.0 Port");

    /* Segment 5: register and memory dump */
    //cterr_add_reg_dump((PFV)bcm63168_get_xdsl_profile);

    /* Segment 6: Platform Environment initialized here*/
    cterr_add_env_dump((PFV)show_plat_curr_temps);

    /* Segment 7: Top 3 Debugging Steps */
    cterr_add_debug("Check the interface between the Host SoC and the USB port.",
                    "If there is no problem for these interfaces, "
                    "replace one USB device and redo the test.");
#endif
    int usb_speed = 0;
    int rc = FAILED;
    char *tname = "External USB 0";
    int ret = -1;
    int hotplug_sock = -1;
    fd_set fdSocks;
    struct timeval  tTimeout;
    int numReaders = 0;

    testname(tname);
    prpass(testpass, "%s, ", tname);

    /* External loopback only for external USB */
    if (slot == USB_SLOT0) {
        /*
         * D_EXT_LOOPBACK = 0, enable ext. loopback
         * * D_EXT_LOOPBACK = 1, disable ext. loopback
         */
        if (check_menu_flag(D_EXT_LOOPBACK)) {
            prpass(testpass, "\n External loopback flag is off, skip '%s' \
                external loopback test. ", tname);
	        prcomplete(testpass, errcount, (char *)0);
            return (PASSED);
        }
    }

    /*
     * testname is printed on usb_slot_tests
     */
    /* Test USB Auto(3.0) mode */
    if (usb_get_info() == FAILED) {
        cterr('f', 0, "usb_get_info() failed");
        return (FAILED);
    }

    usb_speed = usb_get_speed(slot);
    if (usb_speed != USB3) {
        cterr('f', 0, "USB 3.0 setting failed (%d)", usb_speed);
        return (FAILED);
    } else {
        rc = usb_slot_tests(slot);
        if (rc == PASSED) {
            prpass(testpass, "%s 3.0 read/write test passed, \n", tname);
        } else {
            cterr('f', 0, "USB 3.0 read/write test failed (%d)", usb_speed);
            return (FAILED);
        }
    }

    /* Hotplug Socket Initialization */
    hotplug_sock = init_hotplug_sock();
    
    /* Test USB 2.0 mode */
    msleep(DELAY_USBCMD);
    system(USB_POWER_OFF_CMD);
    msleep(DELAY_SYSCMD);
    system(USB_20_CUSTOM_REG1_CMD);
    msleep(DELAY_SYSCMD);
    system(USB_20_CUSTOM_REG2_CMD);
    msleep(DELAY_SYSCMD);
    system(USB_MISC_CTRL_1_REG_CMD);
    msleep(DELAY_SYSCMD);
    system(USB_20_CTRL_REG_CMD);
    msleep(DELAY_SYSCMD);
    system(USB_POWER_ON_CMD);
    msleep(DELAY_USBCMD); 
    
    /* Listen to Kernel hotplug uevent */
    while (hotplug_sock) {
        char buf[UEVENT_BUFFER_SIZE] = {0};
        FD_ZERO(&fdSocks);
        FD_SET(hotplug_sock, &fdSocks);
        tTimeout.tv_sec  = 1;	/* kSocketTimeOut */
        tTimeout.tv_usec = 0;
        numReaders = select(hotplug_sock+1, &fdSocks, NULL, NULL, &tTimeout);
        /* if timeout, report it */
        if (0 == numReaders) {
            printf("socket receive uevent timeout!\n");
            break;
        }
        ret = recv(hotplug_sock, &buf, sizeof(buf), 0); 
        if (ret) {
            if (strstr(buf, "add@/devices") && strstr(buf, "usb") && strstr(buf, "/block/sd")) {
                break;
            }
        }
    }
    
    if (hotplug_sock >= 0) {
        close(hotplug_sock);
    }
    
    if (usb_get_info() == FAILED) {
        printf("usb_get_info() failed\n");
    }

    usb_speed = usb_get_speed(slot);
    if (usb_speed != USB2) {
        cterr('f', 0, "USB 2.0 setting failed (%d)", usb_speed);
        return (FAILED);
    } else {
        rc = usb_slot_tests(slot);
        if (rc == PASSED) {
            prpass(testpass, "%s 2.0 read/write test passed, ", tname);
        } else {
            cterr('f', 0, "USB 2.0 read/write test failed (%d)", usb_speed);
            return (FAILED);
        }
    }

    /* Recover to USB Auto(3.0) mode */
    msleep(DELAY_USBCMD); 
    system(USB_POWER_OFF_CMD);
    msleep(DELAY_SYSCMD);
    system(USB_30_CUSTOM_REG1_CMD);
    msleep(DELAY_SYSCMD);
    system(USB_30_CUSTOM_REG2_CMD);
    msleep(DELAY_SYSCMD);
    system(USB_MISC_CTRL_1_REG_CMD);
    msleep(DELAY_SYSCMD);
    system(USB_30_CTRL_REG_CMD);
    msleep(DELAY_SYSCMD);
    system(USB_POWER_ON_CMD);
    msleep(DELAY_USBCMD); 

    prcomplete(testpass, errcount, (char *)0);
    return (rc);
}

/*
 * Function: init_hotplug_sock
 *
 * Description : Open a NETLINK socket.
 *
 * Inputs: N/A
 *
 * Output: socket id
 */
static int init_hotplug_sock(void)
{
    struct sockaddr_nl snl;
    const int buffersize = 16 * 1024;
    int retval;

    memset(&snl, 0x00, sizeof(struct sockaddr_nl));

    snl.nl_family = AF_NETLINK;
    snl.nl_pid = getpid();
    snl.nl_groups = 1;

    int hotplug_sock = socket(PF_NETLINK, SOCK_DGRAM, NETLINK_KOBJECT_UEVENT);
    if (hotplug_sock == -1) {
        printf("error getting socket: %s", strerror(errno));
        return -1;
    }

    /* set receive buffersize */
    setsockopt(hotplug_sock, SOL_SOCKET, SO_RCVBUFFORCE, &buffersize, sizeof(buffersize));
 
    retval = bind(hotplug_sock, (struct sockaddr *) &snl, sizeof(struct sockaddr_nl));
    if (retval < 0) {
        printf("bind failed: %s", strerror(errno));
        close(hotplug_sock);
        hotplug_sock = -1;
        return -1;
    }

    return hotplug_sock;
}

/*-------------------------------------------------
 * $Log: diag_usb_test.c,v $
 * Revision 1.2  2019/01/10 06:36:24  wilbhuan
 * The beginning of Betelgeuse application code.
 *
 *-------------------------------------------------
 */
