/* $Id: diag_gshdsl_util.c,v 1.2 2019/01/10 06:36:23 wilbhuan Exp $ 
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/betelgeuse/diag_gshdsl_util.c,v $
 *------------------------------------------------------------------
 * 
 * diag_gshdsl_util.c
 *
 * Copyright (c) 2018 - 2019 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include "common.h"
#include "types.h"
#include "menu.h"
#include "defs.h"
#include "menu.h"
#include "error.h"
#include "strings.h"
#include "nvmonvars.h"
#include "common_utils.h"
#include "queryflags.h"
#include "linux_ntwk.h" /* tftp_get */
#include "diag_moka_fpga_lib.h"
#include "diag_uart_lib.h"
#include "diag_enhance_err_msg_lib.h"
#include "platform_cookie.h"
#include "diag_temp_sensor_util.h"
#include "diag_gshdsl_test.h"
#include "diag_dsl_lib.h"
#include "diag_dsl_test.h"

/*******************************************************************************
 *
 * Function    : gshdsl_util_reset
 * Description : This utility performs a reset to the GSHDSL module
 * Input       : NONE
 * Output      : PASSED/FAILED
 *
 *******************************************************************************
 */
int gshdsl_util_reset (void)
{
    /* DSL SKU does the dsl module initialization reset sequence */
    if (fpga_reset_32_api(FPGA_EXTER_DEV_RST_REG, EXT_DSL_CHIP_RESET, TRUE,
                          WAITTIME_20_MS)
        == FAILED) {
    	return (FAILED);
    }
    /* DSL SKU un-reset the dsl module */
    if (fpga_reset_32_api(FPGA_EXTER_DEV_RST_REG, EXT_DSL_CHIP_RESET, FALSE,
                          WAITTIME_20_MS)
        == FAILED) {
        return (FAILED);
    }

    return (PASSED);
}

/*-------------------------------------------------
 * $Log: diag_gshdsl_util.c,v $
 * Revision 1.2  2019/01/10 06:36:23  wilbhuan
 * The beginning of Betelgeuse application code.
 *
 *-------------------------------------------------
 */
