/* $Id: diag_aikido_fpga_util.c,v 1.2 2019/01/10 06:36:21 wilbhuan Exp $ 
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/betelgeuse/diag_aikido_fpga_util.c,v $
 *------------------------------------------------------------------
 * 
 * diag_aikido_fpga_util.c
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
#include "diag_dsl_util.h"
#include "diag_dsl_test.h"
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

/*******************************************************************************
 *
 * Function    : aikido_reg_rd_util
 * Description : Utility to read Aikido register.
 * Inputs      : opt - reserved for future use
 * Outputs     : PASSED/FAILED
 *
 *******************************************************************************
 */
int aikido_reg_rd_util (int opt)
{
    uint reg_offset = 0, reg_val = 0;
    
    reg_offset = gethex_answer("Enter register address (0x0 ~ 0xffff): ",
                               0x2008, 0, 0xffff);

    if (aikido_read_32_reg(reg_offset, &reg_val) != PASSED) {
        printf("Failed to read FPGA register 0x%04X.\n", reg_offset);
        return (FAILED);
    } else {
        printf("FPGA register(0x%04X) = 0x%08X\n", reg_offset, reg_val);
    }
    return (PASSED);
}

/*******************************************************************************
 *
 * Function    : aikido_reg_wr_util
 * Description : Utility to write Aikido register.
 * Inputs      : opt - reserved for future use
 * Outputs     : PASSED/FAILED
 *
 *******************************************************************************
 */
int aikido_reg_wr_util (int opt)
{
    uint reg_offset = 0, orig_val = 0, reg_val = 0;
    
    reg_offset = gethex_answer("Enter register address(0x0 ~ 0xffff): ",
                               0, 0, 0xffff);

    if (aikido_read_32_reg(reg_offset, &orig_val) != PASSED) {
        return (FAILED);
    }

    reg_val = gethex_answer("Enter write-in data(hex): ",
                            orig_val, 0, 0xffffffff);

    if (aikido_write_32_reg(reg_offset, reg_val) != PASSED) {
        return (FAILED);
    } else {
        printf("Done writing 0x%08X to FPGA register(0x%04X).\n",
               reg_val, reg_offset);
    }
    return (PASSED);
}

/*-------------------------------------------------
 * $Log: diag_aikido_fpga_util.c,v $
 * Revision 1.2  2019/01/10 06:36:21  wilbhuan
 * The beginning of Betelgeuse application code.
 *
 *-------------------------------------------------
 */
