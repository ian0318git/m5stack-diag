/* $Id: diag_reset_button_test.c,v 1.2 2019/01/10 06:36:23 wilbhuan Exp $ 
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/betelgeuse/diag_reset_button_test.c,v $
 *------------------------------------------------------------------
 * 
 * diag_reset_button_test.c
 *
 * Copyright (c) 2018 - 2019 by Cisco Systems, Inc.
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
#include "platform_cookie.h"
#include "diag_enhance_err_msg_lib.h"
#include "diag_moka_fpga_lib.h"
#include "diag_reset_button_test.h"
#include "diag_reset_button_lib.h"

/*******************************************************************************
 *
 * Function   : diag_reset_button_test
 * Description: Function to test reset button
 * Inputs     : None
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
int diag_reset_button_test (void)
{
    uchar mb_get_loc[FRU_SIZE] = {0};
    uchar mb_get_pid[FRU_SIZE] = {0};

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
    cterr_add_component("Marvell Armada 88F7040", "System FPGA", "RESET button");

    /* Segment 5: register and memory dump */
    //cterr_add_reg_dump((PFV)bcm63168_get_xdsl_profile);

    /* Segment 6: Platform Environment initialized here*/
    //cterr_add_env_dump((PFV)show_margins_cterr_wrapper);

    /* Segment 7: Top 3 Debugging Steps */
    cterr_add_debug("Check RESET button(SW4000) see if there's any damage.",
                    "Confirm FPGA version is correct.",
                    "Measure the signal(FPGA_MR_RST_L) between RESET button and FPGA.");

    uint   reg_offset = (uint)FPGA_LPC_RESET_BUTTON_REG;
    uint   reg_val = 0x1, orig_config = 0x2;
    char   *testname_str = "RESET button";
    struct timeval t_start, t_end;
    double t_diff = 0;
    int    ret_val = FAILED;

    testname("%s", testname_str);

    /* 1. Get original config. of RESET button mask,
     *    and confirm RESET button is NOT masked for testing.
     */
    /* 1-1. Get current RESET button mask set-ups. */
    if (fpga_read_32_reg(reg_offset, &orig_config) != PASSED) {
        cterr('f', 0, "%s(%d) Failed to read FPGA RESET button Reg(0x%04X).\n",
                      __func__, __LINE__, reg_offset);
        return (FAILED);
    }

    /* 1-2. Unmask RESET button if needed. */
    if ((orig_config & (uint)RST_BUTTON_MSK) == (uint)RST_BUTTON_IS_MASKED) {
        if (config_reset_button_mask(FALSE) != PASSED) {
            cterr('f', 0, "%s(%d) Failed to Unmasked RESET button.\n",
                          __func__, __LINE__);
            return (FAILED);
        }
    }

    /* 2. Clear RESET button status for testing if needed.
     *    By checking Reset Button Status bit (FPGA reg.0xC4, bit0):
     *        1 = Reset button has been pressed;
     *        0 = Reset button has NOT been pressed.
     *    If RESET button has been pressed, clear it for testing.
     *    By write 1 to RESET button Status bit then confirm it's cleared.
     */
    /* 2-1. Read current RESET button status. */
    if (fpga_read_32_reg(reg_offset, &reg_val) != PASSED) {
        cterr('f', 0, "%s(%d) Failed to read FPGA RESET button Reg(0x%04X).\n",
                      __func__, __LINE__, reg_offset);
        return (FAILED);
    }

    /* 2-2. Clear RESET button status if RESET buttion has been pressed. */
    if ((reg_val & (uint)RST_BUTTON_STAT) == (uint)RST_BUTTON_PRESSED) {
        if (clear_reset_button_status() != PASSED) {
            cterr('f', 0, "%s(%d) Failed to clear RESET button status.\n",
                          __func__, __LINE__);
            return (FAILED);
        }
    }

    /* 3. Send message to ask user to press RESET button. */
    printf("\n=== Please Press the RESET button, "
           "the one on the left hand side of POWER button  ===\n\n");
    fflush(stdout);

    /* 4. Polling status bit to check if RESET button is pressed in 1 mins. */
    printf("Detecting if RESET button is pressed");

    do {
        gettimeofday(&t_start, NULL);
        reg_val = 0;

        if (fpga_read_32_reg(reg_offset, &reg_val) != PASSED) {
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

    /* 5. Confirm the RESET button NOT locks after be clicked */
    printf("Confirming RESET button NOT locks after be pressed... "); 
    if (clear_reset_button_status() != PASSED) {
        cterr('f', 0, "%s(%d) Failed, RESET button is keeping pressed.\n",
                      __func__, __LINE__);
        return (FAILED);
    } else {
        printf("Done.\n");
    }

    /* 6. Restore the RESET button config. */
    if (set_reset_button_config(orig_config) != PASSED) {
        cterr('f', 0, "%s(%d) Failed to restore RESET button config.\n",
                      __func__, __LINE__);
        return (FAILED);
    }

    printf("\n%s test PASSED.\n", testname_str);
    return (PASSED);
}

/*-------------------------------------------------
 * $Log: diag_reset_button_test.c,v $
 * Revision 1.2  2019/01/10 06:36:23  wilbhuan
 * The beginning of Betelgeuse application code.
 *
 *-------------------------------------------------
 */
