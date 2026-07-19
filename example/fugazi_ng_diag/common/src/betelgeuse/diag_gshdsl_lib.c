/* $Id: diag_gshdsl_lib.c,v 1.2 2019/01/10 06:36:22 wilbhuan Exp $ 
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/betelgeuse/diag_gshdsl_lib.c,v $
 *------------------------------------------------------------------
 * 
 * diag_gshdsl_lib.c
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
#include "platform_cookie.h"
#include "diag_temp_sensor_util.h"
#include "diag_gshdsl_test.h"
#include "diag_dsl_lib.h"
#include "diag_dsl_test.h"

/*******************************************************************************
 *
 * Function   : gshdsl_console_switch
 *
 * Description: A utility to Console Switch to GSHDSL console.
 *
 * Inputs     : NONE 
 *             
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
int gshdsl_console_switch (void)
{
    struct uart_parm picocom;
    picocom.tty_dev = PLAT_DSL_UART_DEV_STR;
    picocom.baudrate = 9600;    
    picocom.databit = 8;
    picocom.parity = "1";
    picocom.flow = "n";

    if (plat_console_switch(&picocom) != PASSED) {
        return (FAILED);
    }

    return (PASSED);
}

/*-------------------------------------------------
 * $Log: diag_gshdsl_lib.c,v $
 * Revision 1.2  2019/01/10 06:36:22  wilbhuan
 * The beginning of Betelgeuse application code.
 *
 *-------------------------------------------------
 */
