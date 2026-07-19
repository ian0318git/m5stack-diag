/* $Id: o2_diag_flag_disp_test.c,v 1.2 2014/06/03 10:53:31 erwu2 Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/o2_python_example/o2_diag_flag_disp_test.c,v $
 *------------------------------------------------------------------
 * Description: show diag flags coming from python menu
 *
 * Copyright (c) 2013-2014 by cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "types.h"
#include "common.h"
#include "python_error.h"
#include "diag_flag_create_from_py.h"
#include "platform_fru.h"

/*****************************************************************************
 *
 * Function   : main
 * Description: entry point of executable
 * Inputs     : argc, number of argument
 *              argv, arguments in o2.pcfg and python menu script
 *
 * Outputs    : PASSED/FAILED
 *
 *****************************************************************************/
int
main(int argc,char *argv[])
{
    if (plat_init(argc, argv) == FAILED) {
        return FAILED;
    }

    if (diag_flag_display_test_fn(argc, argv) != PASSED) {
        return FAILED;
    } else {
        return PASSED;
    }
}

/*****************************************************************************
 *
 * Function   : diag_flag_display_test_fn
 * Description: show diag flags coming from python menu script
 * Inputs     : argc, number of argument
 *              argv, arguments in o2.pcfg and python menu script
 *
 * Outputs    : PASSED/FAILED
 *
 *****************************************************************************/
int
diag_flag_display_test_fn(int argc,char *argv[])
{
    int last_arg = 0;
    last_arg = atoi(argv[argc-1]);

    testname("Diag Flag Display");
    prpass(testpass, "Hardcoded, ");
    if (last_arg == 0) {
        printf("\nC language gets Diag flags from python, "
               "all flags OFF below!\n");
    } else {
        printf("\nC language gets Diag flags from python, showing below!\n");
    }

    printf("Continuous         : %s\n",
                               (last_arg&F_CONTINUOUS) ? "ON":"OFF");
    printf("Stop_on_error      : %s\n",
                               (last_arg&F_STOP_ON_ERROR) ? "ON":"OFF");
    printf("Ext_loopback       : %s\n",
                               (last_arg&F_EXT_LOOPBACK) ? "ON":"OFF");
    printf("Abbr_test          : %s\n",
                               (last_arg&F_ABBR_TEST) ? "ON":"OFF");
    printf("Loop_on_error      : %s\n",
                               (last_arg&F_LOOP_ON_ERROR) ? "ON":"OFF");
    printf("Warning            : %s\n",
                               (last_arg&F_WARNING) ? "ON":"OFF");
    printf("Trace_mode         : %s\n",
                               (last_arg&F_TRACE_MODE) ? "ON":"OFF");
    printf("Verbose_mode       : %s\n",
                               (last_arg&F_VERBOSE_MODE) ? "ON":"OFF");
    printf("Min_test_time      : %s\n",
                               (last_arg&F_MIN_TEST_TIME) ? "ON":"OFF");
    printf("Optional_output    : %s\n",
                               (last_arg&F_OPTIONAL_OUTPUT) ? "ON":"OFF");
    printf("eXec_authentication: %s\n",
                               (last_arg&F_EXEC_AUTHENTICATION) ? "ON":"OFF");
    printf("permUtation_test   : %s\n",
                               (last_arg&F_PERMUTATION_TEST) ? "ON":"OFF");
    printf("Debug_option       : %s\n",
                               (last_arg&F_DEBUG_OPTION) ? "ON":"OFF");
    printf("ngio_Pwr_after_test: %s\n",
                               (last_arg&F_NGIO_PWR_AFTER_TEST) ? "ON":"OFF");
    printf("exterNal_customer  : %s\n",
                               (last_arg&F_EXTERNAL_CUSTOMER) ? "ON":"OFF");
    prcomplete(testpass, errcount, (char *)0);

    return PASSED;
}

/******** History ********
$Log: o2_diag_flag_disp_test.c,v $
Revision 1.2  2014/06/03 10:53:31  erwu2
python menu collapsed to main trunk

Revision 1.1.2.2  2014/04/24 08:53:52  erwu2
merge makefile and add flag example to test

Revision 1.1.2.1  2014/04/10 06:24:06  erwu2
classify o2 and lebowski executable to obj folder


$Endlog$
*/
