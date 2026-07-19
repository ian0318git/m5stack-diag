/* $Id: diag_reset_button.c,v 1.1 2017/12/01 13:45:20 palin2 Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/tsn/diag_reset_button.c,v $
 *------------------------------------------------------------------
 *
 * Filename   : diag_reset_button.c
 * Description: TSN Reset Button Diag test and utilities.
 *
 * Copyright (c) 2017 by Cisco Systems, Inc.
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
#include "plat_defs.h"
#include "platform_fru.h"
#include "platform_fpga.h"

/******************************************************************************* 
 *                            Function Prototypes                              *
 *******************************************************************************
 */
int tsn_reset_button_test(void);
static int clear_reset_button_status(void);
static int config_reset_button_mask(boolean);
static int set_reset_button_config(uint);

/*******************************************************************************
 *                              Global Variables                               *
 *******************************************************************************
 */
#define SEC_TO_MICROSEC        1000000
#define MAX_POLLINGTIME_USEC   60000000   /* 60sec */
#define MAX_CHECKTIME_USEC     5000000    /* 5sec */

/*******************************************************************************
 *                                   Menus                                     *
 *******************************************************************************
 */

/*******************************************************************************
 *                                 Functions                                   *
 *******************************************************************************
 */
/*******************************************************************************
 *
 * Function   : tsn_reset_button_test
 * Description: Function to test TSN reset button
 * Inputs     : None
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
int tsn_reset_button_test (void)
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

/*******************************************************************************
 *
 * Function   : clear_reset_button_status
 * Description: Function to clear TSN reset button status on FPGA.
 * Inputs     : None
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
static int clear_reset_button_status (void)
{
    uint reg_offset = (uint)FPGA_LPC_RESET_BUTTON_REG;
    uint reg_val = 0;
    struct timeval t_start, t_end;
    double t_diff = 0;
    int ret_val = FAILED;

    /* Confirm RESET button status bit is cleared successfully.
     * By polling RESET button status bit.
     */
    do {
        gettimeofday(&t_start, NULL);

        /* Based on TSN FPGA spec,
         * RESET button status bit is write 1 to clear(RW1C).
         */
        if (fpga_read_32_reg(reg_offset, &reg_val) != PASSED) {
            printf("%s(%d) Failed to read FPGA RESET button Reg(0x%04X).\n",
                   __func__, __LINE__, reg_offset);
            return (FAILED);
        }

        reg_val |= (uint)RST_BUTTON_STAT;

        if (fpga_write_32_reg(reg_offset, reg_val) != PASSED) {
            printf("%s(%d) Failed to write FPGA RESET button Reg(0x%04X).\n",
                   __func__, __LINE__, reg_offset);
            return (FAILED);
        }

        reg_val = 0;

        if (fpga_read_32_reg(reg_offset, &reg_val) != PASSED) {
            printf("%s(%d) Failed to read FPGA RESET button Reg(0x%04X).\n",
                   __func__, __LINE__, reg_offset);
            return (FAILED);
        }

        if ((reg_val & (uint)RST_BUTTON_STAT) == 0) {
            ret_val = PASSED;
            break;
        }

        gettimeofday(&t_end, NULL);
        t_diff += (double)(((t_end.tv_sec - t_start.tv_sec) * SEC_TO_MICROSEC) +
                           (t_end.tv_usec - t_start.tv_usec));
    } while (t_diff < MAX_CHECKTIME_USEC); /* 5sec */

    if (ret_val != PASSED) {
        printf("%s(%d) Failed to clear FPGA RESET button status bit."
               " (reg_val = 0x%04X)\n",
               __func__, __LINE__, reg_val);
    }
    return (ret_val);
}

/*******************************************************************************
 *
 * Function   : config_reset_button_mask
 * Description: Function to mask/unmask TSN reset button on FPGA.
 * Inputs     : opt - TRUE(1: Mask) / FALSE(0: NOT Mask)
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
static int config_reset_button_mask (boolean opt)
{
    uint reg_offset = (uint)FPGA_LPC_RESET_BUTTON_REG;
    uint reg_val = 0, chk_val = 0;
    struct timeval t_start, t_end;
    double t_diff = 0;
    int    ret_val = FAILED;

    chk_val = (uint)(opt << RST_BUTTON_MSK_OFFSET);

    /* Get current RESET button mask configuration. */
    if (fpga_read_32_reg(reg_offset, &reg_val) != PASSED) {
        printf("%s(%d) Failed to read FPGA RESET button Reg(0x%04X).\n",
               __func__, __LINE__, reg_offset);
        return (FAILED);
    }

    /* Check if RESET button mask configuration needs to be changed. */
    if (opt == TRUE) {
        if ((reg_val & (uint)RST_BUTTON_MSK) == chk_val) {
            return (PASSED);
        } else {
            reg_val |= chk_val;
        }
    } else {
        if ((reg_val & (uint)RST_BUTTON_MSK) == chk_val) {
            return (PASSED);
        } else {
            reg_val &= (uint)(~RST_BUTTON_MSK);
        }
    }

    /* Set RESET button mask configuration. */
    if (fpga_write_32_reg(reg_offset, reg_val) != PASSED) {
        printf("%s(%d) Failed to write FPGA RESET button Reg(0x%04X).\n",
               __func__, __LINE__, reg_offset);
        return (FAILED);
    }

    /* Confirm RESET button mask is configured correctly. */
    do {
        gettimeofday(&t_start, NULL);
        reg_val = 0;

        if (fpga_read_32_reg(reg_offset, &reg_val) != PASSED) {
            printf("%s(%d) Failed to read FPGA RESET button Reg(0x%04X).\n",
                   __func__, __LINE__, reg_offset);
            return (FAILED);
        }

        if ((reg_val & (uint)RST_BUTTON_MSK) == chk_val) {
            ret_val = PASSED;
            break;
        }

        gettimeofday(&t_end, NULL);
        t_diff += (double)(((t_end.tv_sec - t_start.tv_sec) * SEC_TO_MICROSEC) +
                           (t_end.tv_usec - t_start.tv_usec));
    } while (t_diff < MAX_CHECKTIME_USEC); /* 5sec */

    if (ret_val != PASSED) {
        printf("%s(%d) Timeout! RESET button is still %s.\n",
               __func__, __LINE__, ((opt == TRUE) ? "NOT masked" : "masked"));
    }
    return (ret_val);
}

/*******************************************************************************
 *
 * Function   : set_reset_button_config
 * Description: Function to set TSN reset button configuration in FPGA.
 * Inputs     : config_val - configure value that wanted to set
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
static int set_reset_button_config (uint config_val)
{
    uint reg_offset = (uint)FPGA_LPC_RESET_BUTTON_REG;
    uint reg_val = 0, chk_msk = (uint)RST_BUTTON_MSK;
    struct timeval t_start, t_end;
    double t_diff = 0;
    int    ret_val = FAILED;


    /* Read current config. value */
    if (fpga_read_32_reg(reg_offset, &reg_val) != PASSED) {
        printf("%s(%d) Failed to read FPGA RESET button Reg(0x%04X).\n",
               __func__, __LINE__, reg_offset);
        return (FAILED);
    }

    /* If current config. is same as user wanted to set up, just return. */
    if ((reg_val & chk_msk) == (uint)(config_val & chk_msk)) {
        return (PASSED);
    }

    /* If no, config. to user wanted. */
    if (fpga_write_32_reg(reg_offset, config_val) != PASSED) {
        printf("%s(%d) Failed to write FPGA RESET button Reg(0x%04X).\n",
               __func__, __LINE__, reg_offset);
        return (FAILED);
    }

    do {
        gettimeofday(&t_start, NULL);
        reg_val = 0;

        /* Polling to confirm user config. is set correctly. */
        if (fpga_read_32_reg(reg_offset, &reg_val) != PASSED) {
            printf("%s(%d) Failed to read FPGA RESET button Reg(0x%04X).\n",
                   __func__, __LINE__, reg_offset);
            return (FAILED);
        }

        if ((reg_val & chk_msk) == (uint)(config_val & chk_msk)) {
            ret_val = PASSED;
            break;
        }

        gettimeofday(&t_end, NULL);
        t_diff += (double)(((t_end.tv_sec - t_start.tv_sec) * SEC_TO_MICROSEC) +
                           (t_end.tv_usec - t_start.tv_usec));
    } while (t_diff < MAX_CHECKTIME_USEC); /* 5sec */

    if (ret_val != PASSED) {
        printf("%s(%d) Failed to config. RESET button.\n", __func__, __LINE__);
    }
    return (ret_val);
}


/*------------------------------------------------------------------
$Log: diag_reset_button.c,v $
Revision 1.1  2017/12/01 13:45:20  palin2
Added support RESET button test (CSCvg96921).

$Endlog$
*/

