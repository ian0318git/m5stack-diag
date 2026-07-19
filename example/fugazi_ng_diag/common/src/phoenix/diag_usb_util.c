/* $Id: diag_usb_util.c,v 1.2 2021/04/15 00:52:26 achiu2 Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/phoenix/diag_usb_util.c,v $
 *------------------------------------------------------------------
 *
 * Filename: diag_usb_util.c
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
#include "linux_usb_test.h"
#include "nvmonvars.h"
#include "queryflags.h"
#include "plat_defs.h"
#include "diag_usb_util.h"
#include "diag_usb_lib.h"
#include "diag_fpga_lib.h"
#include "diag_fpga.h"
#include "phoenix_comm.h"

/*
 * Declare Global function
 */
extern int ExecuteCmdbyPopen(char *cmd, char *retBuf, int sizeOfBuf);

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
        if (phoenix_mem_write32(reg_int, 0x80) != PASSED) {
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
        phoenix_mem_write32(reg_int, 0x0);
        system("echo -n \"0000:00:15.0\" | tee /sys/bus/pci/drivers/xhci_hcd/unbind > /dev/null; sleep 2; echo -n \"0000:00:15.0\" | tee /sys/bus/pci/drivers/xhci_hcd/bind > /dev/null");
        return (PASSED);
    }
    
    memset(reg, 0, sizeof(reg));
    sprintf(reg, "%s%s", prefix, USB_PORT_CTRL_TEST);
    reg_int = strtoul(reg, NULL, 16); 
    if (phoenix_mem_write32(reg_int, reg_val) != PASSED) {
        printf("Failed to read CPU register 0x%08X.\n", reg_int);
        system("echo -n \"0000:00:15.0\" | tee /sys/bus/pci/drivers/xhci_hcd/unbind > /dev/null; sleep 2; echo -n \"0000:00:15.0\" | tee /sys/bus/pci/drivers/xhci_hcd/bind > /dev/null");
        return (FAILED);
    } 
    return (PASSED);
}

