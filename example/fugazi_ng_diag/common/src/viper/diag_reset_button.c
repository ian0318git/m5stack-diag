 /* $Id: diag_reset_button.c,v 1.2 2018/08/06 02:31:51 harrchan Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/viper/diag_reset_button.c,v $
 *------------------------------------------------------------------
 *
 * Filename   : diag_reset_button.c
 * Description: Viper Reset Button Diag test and utilities.
 *
 * Copyright (c) 2018 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */
#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include "common.h"
#include "error.h"
#include "types.h"
#include "plat_defs.h"
#include "diag_fpga.h"
#include "diag_reset_button.h"


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
 * Function   : viper_reset_button_test
 * Description: Function to test Viper reset button
 * Inputs     : None
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
int viper_reset_button_test (void)
{
    uint   reg_offset = (uint)FPGA_STAT_AND_CTRL_REG;
    uint   reg_val = 0x1;
    char   *testname_str = "RESET button";
    struct timeval t_start, t_end;
    double t_diff = 0;
    int    ret_val = FAILED;

    testname("%s", testname_str);

    /* 1. Send message to ask user to press RESET button. */
    printf("\n=== Please Press the RESET button, "
           "the one on the right hand side of POWER button  ===\n\n");
    fflush(stdout);

    /* 2. Polling status bit to check if RESET button is pressed in 1 mins. */
    printf("Detecting if RESET button is pressed");

    do {
        gettimeofday(&t_start, NULL);
        reg_val = 0;

        if (fpga_read_reg(reg_offset, &reg_val) != PASSED) {
            printf("%s(%d) Failed to read FPGA RESET button Reg(0x%04X).\n",
                   __func__, __LINE__, reg_offset);
            return (FAILED);
        }

        if ((reg_val & (uint)RST_BUTTON_STAT) == (uint)RST_BUTTON_PRESSED) {
            printf("Detected.\n");
            fflush(stdout);
            ret_val = PASSED;
            break;
        }

        gettimeofday(&t_end, NULL);
        t_diff += (double)(((t_end.tv_sec - t_start.tv_sec) * SEC_TO_MICROSEC) +
                           (t_end.tv_usec - t_start.tv_usec));

        if (t_end.tv_sec > t_start.tv_sec) {
            printf(".");
            fflush(stdout);
        }
    } while (t_diff < MAX_POLLINGTIME_USEC); /* Polling time: 1min. */

    if (ret_val != PASSED) {
        cterr('f', 0, "%s(%d) TIMEOUT! RESET button is NOT pressed.\n",
                      __func__, __LINE__);
        return (FAILED);
    }

    printf("\n%s test PASSED.\n", testname_str);
    return (PASSED);
}

/*------------------------------------------------------------------
$Log: diag_reset_button.c,v $
Revision 1.2  2018/08/06 02:31:51  harrchan
Merge viper E2E to the main trunk (CSCvk28469)

Revision 1.1.2.2  2018/06/27 06:27:52  harrchan
Follow the coding rule to clean up code

Revision 1.1.2.1  2018/03/29 10:35:44  lucywang
Added Reset button test


$Endlog$
*/
