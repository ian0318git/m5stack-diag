/* $Id: diag_usb_util.c,v 1.4 2019/07/11 12:31:30 alicehua Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/nutella/diag_usb_util.c,v $
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
#include "diag_lte_lib.h"
#include "diag_fpga_lib.h"
#include "diag_fpga.h"
#include "nutella_comm.h"

/*
 * Declare local function
 */

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
        /* Get Port Power Management Status and Control register address */
        ExecuteCmdbyPopen("memm=`cat /proc/iomem | grep xhci-hcd | awk '{print $1}' | awk -F - '{print $2}'`; expr substr $memm 1 4 | tr -d '\n'", prefix, 512);
        sprintf(reg, "%s%s", prefix, USB_PORT_CTRL);
        reg_int = strtoul(reg, NULL, 16);

        /* Write Port Power Management Status and Control register to 0x80 */
        if (nutella_mem_write32(reg_int, 0x80) != PASSED) {
            printf("Failed to Write CPU register 0x%08X.\n", reg_int);
            system("echo -n \"0000:00:15.0\" | tee /sys/bus/pci/drivers/xhci_hcd/unbind > /dev/null; sleep 2; echo -n \"0000:00:15.0\" | tee /sys/bus/pci/drivers/xhci_hcd/bind > /dev/null");
            return (FAILED);
        }
    }

    /* Based on what mode does user choose, set reg_val to corresponding value */
    switch (ret) {
    case 1:
        reg_val = TEST_J_STATE;
    break;
    case 2:
        reg_val = TEST_K_STATE;
    break;
    case 3:
        reg_val = TEST_SE0_NAK;
    break;
    case 4:
        reg_val = TEST_PACKET;
    break;
    default:
        memset(reg, 0, sizeof(reg));
        sprintf(reg, "%s%s", prefix, USB_PORT_CTRL_TEST);
        reg_int = strtoul(reg, NULL, 16); 
        nutella_mem_write32(reg_int, 0x0);
        system("echo -n \"0000:00:15.0\" | tee /sys/bus/pci/drivers/xhci_hcd/unbind > /dev/null; sleep 2; echo -n \"0000:00:15.0\" | tee /sys/bus/pci/drivers/xhci_hcd/bind > /dev/null");
        return (PASSED);
    }
    
    /* Get Port Power Management Status and Control register address */
    memset(reg, 0, sizeof(reg));
    sprintf(reg, "%s%s", prefix, USB_PORT_CTRL_TEST);
    reg_int = strtoul(reg, NULL, 16); 
    /* Write Port Power Management Status and Control register to reg_val */
    if (nutella_mem_write32(reg_int, reg_val) != PASSED) {
        printf("Failed to read CPU register 0x%08X.\n", reg_int);
        system("echo -n \"0000:00:15.0\" | tee /sys/bus/pci/drivers/xhci_hcd/unbind > /dev/null; sleep 2; echo -n \"0000:00:15.0\" | tee /sys/bus/pci/drivers/xhci_hcd/bind > /dev/null");
        return (FAILED);
    } 
    return (PASSED);
}

/*-------------------------------------------------
$Log: diag_usb_util.c,v $
Revision 1.4  2019/07/11 12:31:30  alicehua
Collapse Nutella codes into main trunk.

$Endlog$
*/
