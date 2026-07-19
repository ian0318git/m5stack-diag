/* $Id: diag_pcie_test.c,v 1.2 2021/04/15 00:52:26 achiu2 Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/phoenix/diag_pcie_test.c,v $
 *------------------------------------------------------------------
 *
 * diag_pcie_test.c - Check if PCIe buses run on expected speed.
 *
 *
 * Copyright (c) 2020 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <asm/ioctl.h>
#include <sys/stat.h>
#include <signal.h>
#include <sys/ioctl.h>
#include <pthread.h>
#include <errno.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <inttypes.h>
#include <string.h>
#include <asm/byteorder.h>
#include <linux/mmc/ioctl.h>
#include <linux/fs.h>
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
#include "mb_tests.h"
#include "plat_defs.h"
#include "setjmps.h"
#include "proto.h"
#include "plat_defs.h"
#include "diag_pcie_test.h"

/*******************************************************************************
 *                          Function Declaration
 *******************************************************************************
 */
int diag_pcie_scan_test(int);
extern int is_m2_nvme_device (void);

/*******************************************************************************
 *
 * Function    : diag_pcie_scan_test
 * Description : Verify if PCIe buses run on expected speed
 * Inputs      : dummy - no use 
 * Outputs     : PASSED/FAILED
 *
 *******************************************************************************
 */

int diag_pcie_scan_test (int dummy)
{
    char cmd[256];
    char *tname = "PCIe Scan";
    int ret = PASSED;

    testname("%s", tname);
    prpass(testpass, "%s, ", tname);

    /* Scan PCI bus for check */
    system(PCIE_SCAN_CMD);

    /* Check Logic FPGA PCIe */
    //Speed
    sprintf(cmd, "cat %s | grep '%s #%d' -A 1 | grep Speed | awk '{print $3}' | grep -e '2.5GT/s' > /dev/null", 
                 PCIE_LIST_LOG, PCIE_NAME, PCIE_RP_LOGIC_FPGA);
    if (system(cmd) != 0) {
        printf("\nWrong Logic FPGA bus Speed.\n");
    	ret = (FAILED);
    }
    //Width
    sprintf(cmd, "cat %s | grep '%s #%d' -A 1 | grep Width | awk '{print $5}' | grep -e 'x1' > /dev/null", 
                 PCIE_LIST_LOG, PCIE_NAME, PCIE_RP_LOGIC_FPGA);
    if (system(cmd) != 0) {
        printf("\nWrong Logic FPGA bus Width.\n");
    	ret = (FAILED);
    }

    /* Check I350 PCIe */
    //Speed
    sprintf(cmd, "cat %s | grep '%s #%d' -A 1 | grep Speed | awk '{print $3}' | grep -e '5GT/s' > /dev/null", 
                 PCIE_LIST_LOG, PCIE_NAME, PCIE_RP_I350);
    if (system(cmd) != 0) {
        printf("\nWrong I350 bus Speed.\n");
    	ret = (FAILED);
    }
    //Width
    sprintf(cmd, "cat %s | grep '%s #%d' -A 1 | grep Width | awk '{print $5}' | grep -e 'x2' > /dev/null", 
                 PCIE_LIST_LOG, PCIE_NAME, PCIE_RP_I350);
    if (system(cmd) != 0) {
        printf("\nWrong I350 bus Width.\n");
    	ret = (FAILED);
    }

    /* Check M2 PCIe if exists */
    if (is_m2_nvme_device()) {
        //Width
        sprintf(cmd, "cat %s | grep '%s #%d' -A 1 | grep Width | awk '{print $5}' | grep -e 'x1' > /dev/null", 
                     PCIE_LIST_LOG, PCIE_NAME, PCIE_RP_M2);
        if (system(cmd) != 0) {
            printf("\nWrong M.2 PCIe bus Width.\n");
    	    ret = (FAILED);
        }
    }

    if (ret == FAILED) {
        cterr('f', 0, "PCIe scan test failed.");
    }
    prcomplete(testpass, errcount, (char *)0);

    return (ret);
}

