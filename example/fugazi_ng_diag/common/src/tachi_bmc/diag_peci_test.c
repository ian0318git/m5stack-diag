/* $Id: diag_peci_test.c,v 1.2 2016/04/20 11:25:30 benchen2 Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/tachi_bmc/diag_peci_test.c,v $
 *------------------------------------------------------------------
 *
 * diag_peci_test.c - PECI test functions
 *
 * June 2015, Times Huang ported from mfgdiag
 *
 * Copyright (c) 2016 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */
#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#include "common.h"
#include "diag_peci_lib.h"
#include "diag_peci_test.h" 
#include "diag_power_lib.h"

int diag_peci_test(void);

int diag_peci_test (void)
{
    int cpu;
    int rc;
    uint8_t data;
    int orig_power_stat;
    
    /* Power up the host if it is not on */
    orig_power_stat = diag_intel_power_status();
    if (orig_power_stat != INTEL_POWER_ON) {
        diag_intel_power_on(INTEL_POWER_ON);
    }
    
    for (cpu = 0; cpu < 1; cpu++) {
        printf("Testing CPU %d\n", cpu);
        
        if ((rc = diag_peci_get_family_id(PECI_DEV_ADDR + cpu, &data))) {
            printf("Fail to read Family ID from CPU-%d\n", cpu);
            goto __cleanup;
        }
        
        if (data != INTEL_BW_FAMILY_ID) {
            printf("Family ID mismatch: read = 0x%02x, exp = 0x%02x\n", data, 
                   INTEL_BW_FAMILY_ID);
            goto __cleanup;
        }
        
        if ((rc = diag_peci_get_cpu_model(PECI_DEV_ADDR + cpu, &data))) {
            printf("Fail to read CPU Model from CPU-%d\n", cpu);
            goto __cleanup;
        }
        
        if (data != INTEL_BROADWELL_CPU_MODEL) {
            printf("CPU Model mismatch: read = 0x%02x, exp = 0x%02x\n", data, 
                   INTEL_BROADWELL_CPU_MODEL);
            goto __cleanup;
        }
    } 
    
__cleanup:
    /* Restore host power state */
    if (orig_power_stat != INTEL_POWER_ON) {
        diag_intel_power_on(INTEL_POWER_OFF);
    }

    return (PASSED);
}
 
/*---------------------------------------------------------------
$Log: diag_peci_test.c,v $
Revision 1.2  2016/04/20 11:25:30  benchen2
add tachi fru portion

Revision 1.1.2.1  2015/06/11 02:01:09  tirawan
Add files for Tachi BMC project


$Endlog$
*/
