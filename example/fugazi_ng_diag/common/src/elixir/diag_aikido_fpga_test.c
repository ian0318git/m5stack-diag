/* $Id: diag_aikido_fpga_test.c,v 1.2 2021/09/24 01:21:05 harrchan Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/elixir/diag_aikido_fpga_test.c,v $
 *------------------------------------------------------------------
 * 
 * diag_aikido_fpga_test.c
 *
 * Copyright (c) 2018 - 2019 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#include <sys/mman.h>
#include <unistd.h>
#include "error.h"
#include "common.h"
#include "types.h"
#include "common_utils.h"
#include "menu.h"
#include "nvmonvars.h"
#include <stdio.h>
#include "proto.h"
#include <fcntl.h>
#include <asm/ioctl.h>
#include <sys/stat.h>
#include <signal.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <sys/stat.h>
#include <string.h>
#include "diag_enhance_err_msg_lib.h"
#include "platform_cookie.h"
#include "diag_cpu_lib.h"
#include "diag_sirius_fpga_lib.h"
#include "diag_sirius_fpga_util.h"
#include "diag_ge_phy_test.h"
#include "diag_temp_sensor_util.h"
#include "diag_esw_lib.h"
#include "diag_esw_util.h"
#include "dev_mrvl_ge.h"
#include "diag_ge_phy_util.h"
#include "diag_ge_phy_lib.h"
#include "diag_wifi_lib.h"
#include "diag_moka_fpga_lib.h"
#include "diag_moka_fpga_util.h"
#include "diag_cpu_lib.h"
#include "diag_cpu_util.h"
#include "diag_aikido_fpga_lib.h"
#include "diag_aikido_fpga_util.h"
#include "diag_aikido_fpga_test.h"

/*******************************************************************************
 *  
 * Function    : diag_aikido_reg_test
 * Description : Utility to test aikido register.
 * Inputs      : NONE
 * Outputs     : PASSED/FAILED
 *
 *******************************************************************************
 */
int diag_aikido_reg_test (void)
{
    uchar mb_get_loc[FRU_SIZE] = {0};
    uchar mb_get_pid[FRU_SIZE] = {0};
#ifdef ENHANCE_ERROR_MSG_RDY
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
    cterr_add_component("Marvell Armada 7040", "Local Bus", "System FPGA");

    /* Segment 5: register and memory dump */
    //cterr_add_reg_dump((PFV)bcm63168_get_xdsl_profile);

    /* Segment 6: Platform Environment initialized here*/
    cterr_add_env_dump((PFV)show_plat_curr_temps);

    /* Segment 7: Top 3 Debugging Steps */
    cterr_add_debug("If any failure occurs in this test, "
                    "check the GPIO interface between MB and FPGA.");
#endif

    char *tname = "Aikido Register";

    uint reg_offset = FPGA_AIKIDO_REG, rd_reg_val = 0x0, wr_reg_val = 0x0;
    uint wr_data_arr[2] = {FPGA_AIKIDO_REG_PATTERN, 0};
    
    int ix;

    testname(tname);
    prpass(testpass, "%s, ", tname);

    printf("\n");

    /* read original data */
    if (aikido_read_32_reg(reg_offset, &rd_reg_val) != PASSED) {
        cterr('f', 0, "%s: Failed to read Aikido Register.",
              __FUNCTION__);
        prcomplete(testpass, errcount, (char *)0);
        return (FAILED);
    }
    wr_data_arr[1] = rd_reg_val;

    for (ix = 0; ix < FPGA_AIKIDO_REG_TEST_ROUND; ix++)
    {
        /* write data */
        wr_reg_val = wr_data_arr[ix];
        if (aikido_write_32_reg(reg_offset, wr_reg_val) != PASSED) {
            cterr('f', 0, "%s: Failed to write Aikido Register.",
                  __FUNCTION__);
            prcomplete(testpass, errcount, (char *)0);
            return (FAILED);
        } else {
            printf("Done writing 0x%08X to FPGA register(0x%04X).\n",
                   wr_reg_val, reg_offset);
        }
     
        /* read data */
        if (aikido_read_32_reg(reg_offset, &rd_reg_val) != PASSED) {
            cterr('f', 0, "%s: Failed to read Aikido Register.",
                  __FUNCTION__);
            prcomplete(testpass, errcount, (char *)0);
            return (FAILED);
        }
     
        /* compoare data */
        if (rd_reg_val != wr_data_arr[ix]) {
            cterr('f', 0, "%s: Aikido Register test failed, read data:0x%x but pattern data:0x%x",
                  __FUNCTION__, rd_reg_val, wr_data_arr[ix]);
            prcomplete(testpass, errcount, (char *)0);
            return (FAILED);
        }
    }
    return (PASSED);
}

/*******************************************************************************
 *  
 * Function    : aikido_mailbox_test
 * Description : Utility to test Aikido mailbox.
 * Inputs      : NONE
 * Outputs     : PASSED/FAILED
 *
 *******************************************************************************
 */
int aikido_mailbox_test (void)
{
    char *tname = "Aikido Mailbox";
    uint reg_offset = 0x0, rd_reg_val = 0x0, wr_reg_val = 0x0;

    testname(tname);
    prpass(testpass, "%s, ", tname);

    /* Step 1. Set MBX enable */
    reg_offset = FPGA_AIKIDO_MBX_REG;
    wr_reg_val = MBX_EN_FLAGS_OR;
    if (aikido_write_32_reg(reg_offset, wr_reg_val) != PASSED) {
        cterr('f', 0, "\n%s: Failed to write Aikido Register %x.",
              __FUNCTION__,reg_offset);
        prcomplete(testpass, errcount, (char *)0);
        return (FAILED);
    } else {
        if ((NVRAM)->diagflag & D_VERBOSE) {
            printf("\nDone writing 0x%08X to FPGA register(0x%04X).\n",
                      wr_reg_val, reg_offset);
        }
    }
    /* Step 2. Read MBX enable reg */
    reg_offset = FPGA_AIKIDO_MBX_REG;
    if (aikido_read_32_reg(reg_offset, &rd_reg_val) != PASSED) {
        cterr('f', 0, "%s: Failed to read Aikido Register.",
              __FUNCTION__);
        prcomplete(testpass, errcount, (char *)0);
        return (FAILED);
    }
    if (rd_reg_val != MBX_EN_FLAGS_OR) {
        cterr('f', 0, "%s: Aikido Register test failed; expect 0x%x instead of 0x%x",
              __FUNCTION__, MBX_EN_FLAGS_OR, rd_reg_val);
        prcomplete(testpass, errcount, (char *)0);
        return (FAILED);
    }
    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("Reading 0x%08X from FPGA register(0x%04X).\n", 
                rd_reg_val, reg_offset);
    }
    /* Step 3. write Get SCC ID cmd */
    reg_offset = FPGA_AIKIDO_MBX_DPRAM_REG;
    wr_reg_val = MBX_GET_SCC_ID_CMD;
    if (aikido_write_32_reg(reg_offset, wr_reg_val) != PASSED) {
        cterr('f', 0, "%s: Failed to write Aikido Register %x.",
              __FUNCTION__,reg_offset);
        prcomplete(testpass, errcount, (char *)0);
        return (FAILED);
    } else {
        if ((NVRAM)->diagflag & D_VERBOSE) {
            printf("\nDone writing 0x%08X to FPGA register(0x%04X).\n",
                      wr_reg_val, reg_offset);
        }
    }
    /* Step 4. Send interrupt to TAM FW */
    reg_offset = FPGA_AIKIDO_MBX_INTSTAT_REG;
    wr_reg_val = MBX_H2M_FLAGS;
    if (aikido_write_32_reg(reg_offset, wr_reg_val) != PASSED) {
        cterr('f', 0, "%s: Failed to write Aikido Register %x.",
              __FUNCTION__,reg_offset);
        prcomplete(testpass, errcount, (char *)0);
        return (FAILED);
    } else {
        if ((NVRAM)->diagflag & D_VERBOSE) {
            printf("\nDone writing 0x%08X to FPGA register(0x%04X).\n",
                      wr_reg_val, reg_offset);
        }
    }
    /* Step 5. Read status bit, 0x08 means data is ready */
    reg_offset = FPGA_AIKIDO_MBX_INTCRTL_REG;
    if (aikido_read_32_reg(reg_offset, &rd_reg_val) != PASSED) {
        cterr('f', 0, "%s: Failed to read Aikido Register.",
              __FUNCTION__);
        prcomplete(testpass, errcount, (char *)0);
        return (FAILED);
    }
    if (rd_reg_val != MBX_FLAGS_OR) {
        cterr('f', 0, "%s: Aikido Register test failed; expect 0x%x instead of 0x%x",
              __FUNCTION__, MBX_FLAGS_OR, rd_reg_val);
        prcomplete(testpass, errcount, (char *)0);
        return (FAILED);
    }
    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("Reading 0x%08X from FPGA register(0x%04X).\n", 
                rd_reg_val, reg_offset);
    }
    /* Step 6.  Read actual SCC ID 1 */
    reg_offset = FPGA_AIKIDO_MBX_DPRAM_REG;
    if (aikido_read_32_reg(reg_offset, &rd_reg_val) != PASSED) {
        cterr('f', 0, "%s: Failed to read Aikido Register.",
              __FUNCTION__);
        prcomplete(testpass, errcount, (char *)0);
        return (FAILED);
    }
    if (rd_reg_val != MBX_SCC_ID_1) {
        cterr('f', 0, "%s: Aikido Register test failed; expect 0x%x instead of 0x%x",
              __FUNCTION__, MBX_SCC_ID_1, rd_reg_val);
        prcomplete(testpass, errcount, (char *)0);
        return (FAILED);
    }
    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("Reading 0x%08X from FPGA register(0x%04X).\n", 
                rd_reg_val, reg_offset);
    }
    /* Step 7.  Read actual SCC ID 2 */
    reg_offset = FPGA_AIKIDO_MBX_DPRAM_REG + MBX_DPRAM_OFFSET_FOUR ;
    if (aikido_read_32_reg(reg_offset, &rd_reg_val) != PASSED) {
        cterr('f', 0, "%s: Failed to read Aikido Register.",
              __FUNCTION__);
        prcomplete(testpass, errcount, (char *)0);
        return (FAILED);
    }
    if (rd_reg_val != MBX_SCC_ID_2) {
        cterr('f', 0, "%s: Aikido Register test failed; expect 0x%x instead of 0x%x",
              __FUNCTION__, MBX_SCC_ID_2, rd_reg_val);
        prcomplete(testpass, errcount, (char *)0);
        return (FAILED);
    }
    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("Reading 0x%08X from FPGA register(0x%04X).\n", 
                rd_reg_val, reg_offset);
    }
    /* Step 8.  Clear Flag, ack FW */
    reg_offset = FPGA_AIKIDO_MBX_H2M_FLAGS_REG;
    wr_reg_val = MBX_M2H_FLAGS;
    if (aikido_write_32_reg(reg_offset, wr_reg_val) != PASSED) {
        cterr('f', 0, "%s: Failed to write Aikido Register %x.",
              __FUNCTION__,reg_offset);
        prcomplete(testpass, errcount, (char *)0);
        return (FAILED);
    } else {
        if ((NVRAM)->diagflag & D_VERBOSE) {
            printf("\nDone writing 0x%08X to FPGA register(0x%04X).\n",
                      wr_reg_val, reg_offset);
        }
    }
    printf("\n%s test PASSED ", tname);
    return (PASSED);
}

/*-------------------------------------------------
 * $Log: diag_aikido_fpga_test.c,v $
 * Revision 1.2  2021/09/24 01:21:05  harrchan
 * Collapse Elixir-branch to Main Trunk.
 *
 * Revision 1.1.2.2  2020/09/14 05:49:44  harrchan
 * Remove DSL and GSHDSL relevant part
 *
 * Revision 1.1.2.1  2020/09/09 09:08:05  illiu
 * First version which has been ported with Dreamliner and Marvel CPSS
 *
 * Revision 1.2  2019/01/10 06:36:21  wilbhuan
 * The beginning of Betelgeuse application code.
 *
 *-------------------------------------------------
 */
