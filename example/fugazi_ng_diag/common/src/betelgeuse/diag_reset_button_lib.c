/* $Id: diag_reset_button_lib.c,v 1.2 2019/01/10 06:36:23 wilbhuan Exp $ 
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/betelgeuse/diag_reset_button_lib.c,v $
 *------------------------------------------------------------------
 * 
 * diag_reset_button_lib.c
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
#include "diag_moka_fpga_lib.h"
#include "diag_reset_button_test.h"

/*******************************************************************************
 *
 * Function   : clear_reset_button_status
 * Description: Function to clear reset button status on FPGA.
 * Inputs     : None
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
int clear_reset_button_status (void)
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

        /* Based on FPGA spec,
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
 * Description: Function to mask/unmask reset button on FPGA.
 * Inputs     : opt - TRUE(1: Mask) / FALSE(0: NOT Mask)
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
int config_reset_button_mask (boolean opt)
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
 * Description: Function to set reset button configuration in FPGA.
 * Inputs     : config_val - configure value that wanted to set
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
int set_reset_button_config (uint config_val)
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

/*-------------------------------------------------
 * $Log: diag_reset_button_lib.c,v $
 * Revision 1.2  2019/01/10 06:36:23  wilbhuan
 * The beginning of Betelgeuse application code.
 *
 *-------------------------------------------------
 */
