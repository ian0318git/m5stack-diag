/* $Id: python_util.c,v 1.2 2014/06/03 10:53:28 erwu2 Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/Python/src/python_util.c,v $
 *------------------------------------------------------------------
 * Filename:    python_util.c
 *
 * Description: python common utility functions
 *
 * Copyright (c) 2014 by cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */
 
/*-----------------------------------------------------------------------------
 * python-specific include
 *---------------------------------------------------------------------------*/
#include "Python.h"
#include "python_util.h"
#include "python_error.h"
#include "diag_flag_create_from_py.h"

/*-----------------------------------------------------------------------------
 * SRG environment include
 *---------------------------------------------------------------------------*/
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>
#include <stdarg.h>
#include <time.h>
#include "common.h"


/*-----------------------------------------------------------------------------
 * python-specific define
 *---------------------------------------------------------------------------*/
int py_diag_flag;

/*-----------------------------------------------------------------------------
 * SRG environment define
 *---------------------------------------------------------------------------*/

 
 /*-----------------------------------------------------------------------------
 * python-specific functions
 *---------------------------------------------------------------------------*/
/******************************************************************************
 *
 *  Function: parse_diag_flag_value
 *
 *  Description: This function parses diag flag value which comes from python
 *               menu
 *
 *  Input: diag flag value
 *
 *  Returns: PASSED if successful;
 *           FAILED, otherwise
 *
 *****************************************************************************/
int parse_diag_flag_value(int flag_value)
{
    if (flag_value & F_CONTINUOUS) {
        continuous = 1;
    }
    if (flag_value & F_STOP_ON_ERROR) {
        stop_on_error = 1;
    }
    if (flag_value & F_EXT_LOOPBACK) {
        ext_loopback = 1;
    }
    if (flag_value & F_ABBR_TEST) {
        abbr_test = 1;
    }
    if (flag_value & F_WARNING) {
        warning = 1;
    }
    if (flag_value & F_TRACE_MODE) {
        trace_mode = 1;
    }
    if (flag_value & F_VERBOSE_MODE) {
        verbose_mode = 1;
    }
    if (flag_value & F_MIN_TEST_TIME) {
        min_test_time = 1;
    }
    if (flag_value & F_OPTIONAL_OUTPUT) {
        optional_output = 1;
    }
    if (flag_value & F_EXEC_AUTHENTICATION) {
        exec_authentication = 1;
    }
    if (flag_value & F_PERMUTATION_TEST) {
        permutation_test = 1;
    }
    if (flag_value & F_DEBUG_OPTION) {
        debug_option = 1;
    }
    if (flag_value & F_NGIO_PWR_AFTER_TEST) {
        ngio_pwr_after_test = 1;
    }
    if (flag_value & F_EXTERNAL_CUSTOMER) {
        external_customer = 1;
    }
    return PASSED;
}

/******************************************************************************
 *
 *  Function: py_parse
 *
 *  Description: parse the last argument of each test executable,
 *               diag flag value, coming from python diag menu interface.
 *
 *  Input: argc and argv[] of each test executable
 *
 *  Returns: PASSED if successful;
 *           FAILED, otherwise
 *
 *****************************************************************************/
int py_parse(int argc,char *argv[])
{
    /* diag flag string to int */
    /* last argument is diag flag, second-last is testpass from python */
    if (argc >= ARGS_3) {
        py_diag_flag = atoi(argv[argc-1]);
        testpass = atoi(argv[argc-2]);
    } else {
        printf("arguments from python to C should be three at least\n ");
        return FAILED;
    }

    if (parse_diag_flag_value(py_diag_flag) == 1) {
        return FAILED;
    }
    printf("\n");
    return PASSED;
}

/******** History ********
$Log: python_util.c,v $
Revision 1.2  2014/06/03 10:53:28  erwu2
python menu collapsed to main trunk

Revision 1.1.2.1  2014/04/29 11:40:36  erwu2
update python file structure


$Endlog$
*/
