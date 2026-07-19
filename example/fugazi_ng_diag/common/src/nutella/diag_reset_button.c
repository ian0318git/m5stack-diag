/* $Id: diag_reset_button.c,v 1.6 2019/11/20 23:44:48 alicehua Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/nutella/diag_reset_button.c,v $
 *------------------------------------------------------------------
 *
 * Filename   : diag_reset_button.c
 * Description: Nutella Reset Button Diag test and utilities.
 *
 * Copyright (c) 2019 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */
#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include <unistd.h>
#include "common.h"
#include "error.h"
#include "types.h"
#include "plat_defs.h"
#include "diag_fpga.h"
#include "diag_reset_button.h"
#include "proto.h"


/*******************************************************************************
 *                              Global Variables                               *
 *******************************************************************************
 */
#define SEC_TO_MICROSEC        1000000
#define MAX_POLLINGTIME_USEC   60000000   /* 60sec */
#define MAX_CHECKTIME_USEC     5000000    /* 5sec */

/*******************************************************************************
 *                                 Functions                                   *
 *******************************************************************************
 */
/*******************************************************************************
 *
 * Function   : nutella_reset_button_test
 * Description: Function to test Nutella reset button
 * Inputs     : None
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
int nutella_reset_button_test (void)
{
    uint   reg_offset = (uint)FPGA_STAT_AND_CTRL_REG;
    uint   reg_val;
    char   *testname_str = "RESET button";
    struct timeval t_start, t_end;
    double t_diff = 0;
    int    ret_val = FAILED;

    testname(testname_str);
    prpass(testpass, "%s, ", testname_str);

    /* 1. Send message to ask user to press RESET button. */
    printf("\n=== Please Press the RESET button, "
           "the one on the right hand side of POWER button  ===\n\n");
    fflush(stdout);

    /* 2. Polling status bit to check if RESET button is pressed in 1 mins. */
    printf("Detecting if RESET button is pressed");

    /* 3. Disable FPGA reset mechanism while press button longer than 1 sec.*/
    if (fpga_write_reg(FPGA_ACCESS_TEST_REG, 0x12345678) != PASSED) {
        printf("%s(%d) Failed to write FPGA WDT Enable register (0x%04X).\n",
               __FUNCTION__, __LINE__, FPGA_WDT_ENABLE_REG);
        return (FAILED);
    }

    do {
        gettimeofday(&t_start, NULL);
        reg_val = 0;

        if (fpga_read_reg(reg_offset, &reg_val) != PASSED) {
            printf("%s(%d) Failed to read FPGA RESET button Reg(0x%04X).\n",
                   __FUNCTION__, __LINE__, reg_offset);
            return (FAILED);
        }

        if ((reg_val & (uint)RST_BUTTON_STAT) == (uint)RST_BUTTON_PRESSED) {
            printf("Detected.\n");
            fflush(stdout);
            /* After 1.5  sec to check button status*/
            /* If button status still assert return fail*/
            msleep(WAIT_RESET_BUTTON_STATUS);
            if (fpga_read_reg(reg_offset, &reg_val) != PASSED) {
                printf("%s(%d) Failed to read FPGA RESET button Reg(0x%04X).\n",
                       __FUNCTION__, __LINE__, reg_offset);
                return (FAILED);
            }

            if ((reg_val & (uint)RST_BUTTON_STAT) == (uint)RST_BUTTON_LOOSEN) {
                printf("Loosen.\n");
                fflush(stdout);
                ret_val = PASSED;
                break;
            } else {
                printf("Reset Button still assert. Are you long pressing the reset button?\n");
                printf("If not. There might be something problems on reset button\n");
                fflush(stdout);
                ret_val = FAILED;
                break;
            }

        }

        gettimeofday(&t_end, NULL);
        t_diff += (double)(((t_end.tv_sec - t_start.tv_sec) * SEC_TO_MICROSEC) +
                           (t_end.tv_usec - t_start.tv_usec));

        if (t_end.tv_sec > t_start.tv_sec) {
            printf(".");
            fflush(stdout);
        }
    } while (t_diff < MAX_POLLINGTIME_USEC); /* Polling time: 1min. */


    if ((ret_val != PASSED) && (t_diff >= MAX_POLLINGTIME_USEC)) {
        cterr('f', 0, "%s(%d) TIMEOUT! RESET button is NOT pressed.\n",
              __func__, __LINE__);
        return (FAILED);
    } else if (ret_val != PASSED) {
        cterr('f', 0, "%s(%d) Fail on reset button test.\n",
              __func__, __LINE__);
        return (FAILED);
    }

    prcomplete(testpass, errcount, (char *)0);
    return (PASSED);
}

/*------------------------------------------------------------------
$Log: diag_reset_button.c,v $
Revision 1.6  2019/11/20 23:44:48  alicehua
CSCvs11655: Print the right error message when operator doesn't press reset
button within 1 minute.

Revision 1.5  2019/10/16 23:46:24  alicehua
CSCvr66516: Fix register offset and delay range in reset button test.

Revision 1.4  2019/07/11 12:31:29  alicehua
Collapse Nutella codes into main trunk.

$Endlog$
*/
