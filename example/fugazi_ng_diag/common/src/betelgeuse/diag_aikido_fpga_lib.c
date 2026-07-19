/* $Id: diag_aikido_fpga_lib.c,v 1.2 2019/01/10 06:36:21 wilbhuan Exp $ 
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/betelgeuse/diag_aikido_fpga_lib.c,v $
 *------------------------------------------------------------------
 * 
 * diag_aikido_fpga_lib.c
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

/*******************************************************************************
 *
 * Function    : aikido_read_32_reg
 * Description : Function to read Aikido register.
 * Inputs      : reg_offset - register offset
 *               *buf       - buffer to put read back register value
 * Outputs     : PASSED/FAILED
 *
 *******************************************************************************
 */
int aikido_read_32_reg (uint reg_offset, uint *buf)
{
    uint offset = 0;

    offset = (uint)(plat_aikido_reg_baseaddr + reg_offset);
    if ((NVRAM)->diagflag & D_VERBOSE) {
    printf("Device Bus address 0x%08X\n", offset);
    }
    if (plat_mem_read32(offset, buf) != PASSED) {
        printf("Failed to read Aikido FPGA register 0x%04X.\n", reg_offset);
        return (FAILED);
    }
    return (PASSED);
}

/*******************************************************************************
 *
 * Function    : aikido_write_32_reg
 * Description : Function performs Aikido register write.
 * Inputs      : reg_offset - register offset
 *               wr_data    - data for write
 * Outputs     : PASSED/FAILED
 *
 *******************************************************************************
 */
int aikido_write_32_reg (uint reg_offset, uint wr_data)
{
    uint offset = 0;

    offset = (uint)(plat_aikido_reg_baseaddr + reg_offset);
    if ((NVRAM)->diagflag & D_VERBOSE) {
    printf("Device Bus address 0x%08X\n", offset);
    }
    if (plat_mem_write32(offset, wr_data) != PASSED) {
        printf("Failed to write FPGA register 0x%04X.\n", reg_offset);
        return (FAILED);
    }
    return (PASSED);
}

/*-------------------------------------------------
 * $Log: diag_aikido_fpga_lib.c,v $
 * Revision 1.2  2019/01/10 06:36:21  wilbhuan
 * The beginning of Betelgeuse application code.
 *
 *-------------------------------------------------
 */
