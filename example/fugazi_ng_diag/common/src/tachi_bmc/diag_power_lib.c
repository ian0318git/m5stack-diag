/* $Id: diag_power_lib.c,v 1.2 2016/04/20 11:25:31 benchen2 Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/tachi_bmc/diag_power_lib.c,v $ 
 *------------------------------------------------------------------
 *
 * diag_power_lib.c - Intel Power Library
 *
 * June 2015, Times Huang ported from mfgdiag
 *
 * Copyright (c) 2016 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */
 
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/ioctl.h> 
#include <fcntl.h>
#include "common.h"
#include "types.h"
#include "queryflags.h"
#include "error.h"
#include "diag_power_lib.h"

void diag_intel_power_on(int);
int diag_intel_power_status(void);
int diag_intel_power_ctl(void);

int diag_intel_power_ctl (void)
{
    int pwr_sts, pwr_ctl;

    pwr_sts = diag_intel_power_status();
    if (pwr_sts == -1) {
        return (FAILED);
    }

    printf("x86 power status: ");
    if (pwr_sts == INTEL_POWER_ON) {
        printf("[ON]\n");
    } else {
        printf("[OFF]\n");
    }
    printf("\n");

    pwr_ctl = getdec_answer("0: Power off, 1: Power On, 2: Abort", 1, 0, 2);

    if (pwr_ctl == 0) {
        printf("Powering off...\n");
        fflush(stdout);
        diag_intel_power_on(INTEL_POWER_OFF);
    } else if (pwr_ctl == 1) {
        printf("Powering on...\n");
        fflush(stdout);
        diag_intel_power_on(INTEL_POWER_ON);
    } else {
        printf("Abort...\n");
    }

    return (PASSED);
}

int diag_intel_power_status (void)
{
    int onoff = 0;
    int rc;
    int fd;
    
    fd = open(HOST_POWER_FILE, O_RDONLY, 0666);
    
    if (fd == -1) {
        printf("Failed to open %s\n", HOST_POWER_FILE);
        return (-1);
    }
    
    rc = ioctl(fd, PLT2_PWR_IOCGSTATE, (unsigned long)&onoff);
    if (rc == -1) {
        printf("IOCTL return failed!\n");
        return (-1);
    }
    
    if (onoff == 1) {
        return (INTEL_POWER_ON);
    } else {
        return (INTEL_POWER_OFF);
    }
}

void diag_intel_power_on (int enable)
{
    if (enable == INTEL_POWER_ON) {
        system("blade-power on");
    } else {
        system("blade-power off");
    }
}

/*---------------------------------------------------------------
$Log: diag_power_lib.c,v $
Revision 1.2  2016/04/20 11:25:31  benchen2
add tachi fru portion

Revision 1.1.2.2  2015/10/01 08:38:21  tirawan
Update Temperature sensor description and add Intel power on/off utility

Revision 1.1.2.1  2015/06/11 02:01:09  tirawan
Add files for Tachi BMC project


$Endlog$
*/
