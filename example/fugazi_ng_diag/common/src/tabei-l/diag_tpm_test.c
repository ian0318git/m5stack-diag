/* $Id: diag_tpm_test.c,v 1.2 2019/10/17 02:16:23 kehuang2 Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/tabei-l/diag_tpm_test.c,v $
 *------------------------------------------------------------------
 *
 * diag_tpm_test.c - Check if TPM chip is OK.
 *
 *
 * Copyright (c) 2018-2019 by Cisco Systems, Inc.
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
#include "diag_tpm_test.h"

/*******************************************************************************
 *                          Function Declaration
 *******************************************************************************
 */
int diag_tpm_test(int);

/*******************************************************************************
 *
 * Function    : diag_tpm_test
 * Description : Verify if TPM chip is OK by vendor provided tool 
 * Inputs      : dummy - no use 
 * Outputs     : PASSED/FAILED
 *
 *******************************************************************************
 */

int diag_tpm_test (int dummy)
{
    FILE *tpm_log;
    char buf_log[TPM_BUF_LOG_LEN]; 
    char *tname = "TPM";

    testname("%s", tname);
    prpass(testpass, "%s, ", tname);
    
    memset(buf_log, 0, sizeof(buf_log));
   
    /* Use TPM vendor provided tool to test TPM chip and create the test log */ 
    system(TPM_CHK_TOOL);
    msleep(TPM_LOG_CREATE_TIME);

    /* Open the TPM test log */
    tpm_log = fopen(TPM_TEST_LOG, FOPEN_RONLY);
    if (tpm_log == NULL) {
        cterr('f', 0, "Open %s fails.", TPM_TEST_LOG);
        return (FAILED);
    }

    /* Parse the TPM test log and check if there is "Pass" log inside. */
    while (fgets(buf_log, sizeof(buf_log), tpm_log) != NULL) {
        if (strstr(buf_log, TPM_PASS_STR)) {
            prpass(testpass, "%s, ", tname);
            fclose(tpm_log);
            prcomplete(testpass, errcount, (char *)0);
            return (PASSED);
        }
    }

    fclose(tpm_log);
    cterr('f', 0, "TPM test failed.");
    prcomplete(testpass, errcount, (char *)0);

    return (FAILED);
}

/*-------------------------------------------------
 * $Log: diag_tpm_test.c,v $
 * Revision 1.2  2019/10/17 02:16:23  kehuang2
 * Collapse Tabei-L into main trunk
 *
 * Revision 1.1.2.3  2019/09/02 08:42:56  kehuang2
 * Clean up code by the comment of code review
 *
 * Revision 1.1.2.2  2019/07/29 06:13:52  kodko
 * Clean up code based on off-line code review
 *
 * Revision 1.1.2.1  2018/12/25 02:06:29  kodko
 * Add TPM chip test that is verified by vendor provided tool.
 *
 * $Endlog$
 *-------------------------------------------------
 */
