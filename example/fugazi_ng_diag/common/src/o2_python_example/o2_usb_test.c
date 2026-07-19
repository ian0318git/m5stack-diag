/* $Id: o2_usb_test.c,v 1.2 2014/06/03 10:53:32 erwu2 Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/o2_python_example/o2_usb_test.c,v $
 *------------------------------------------------------------------
 * Description: usb test sample with pass case to show testname, prpass, and
 *              prcomplete
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

    if (usb_test_fn(argc, argv) != PASSED) {
        return FAILED;
    } else {
        return PASSED;
    }
}

/*****************************************************************************
 *
 * Function   : usb_test_fn
 * Description: pass case example shows testname, prpass, prcomplete info.
 * Inputs     : argc, number of argument
 *              argv, arguments in o2.pcfg and python menu script
 * Outputs    : PASSED/FAILED
 *
 *****************************************************************************/
int
usb_test_fn(int argc,char *argv[])
{
    /* flag usage example */
    if (min_test_time == 1) {
        printf("\n MIN test time flag ON \n");
        printf("\n MIN test time process in o2_usb_test \n");
    } else {
        printf("\n MIN test time flag OFF \n");
        printf("\n non MIN test time process in o2_usb_test \n");
    }
    testname("USB slot %d",atoi(argv[1]));
    prpass(testpass, "usb %d access, ",atoi(argv[1]));
    prcomplete(testpass, errcount, (char *)0);
    return PASSED;
}

/******** History ********
$Log: o2_usb_test.c,v $
Revision 1.2  2014/06/03 10:53:32  erwu2
python menu collapsed to main trunk

Revision 1.1.2.4  2014/04/29 11:40:37  erwu2
update python file structure

Revision 1.1.2.3  2014/04/24 08:53:52  erwu2
merge makefile and add flag example to test

Revision 1.1.2.2  2014/04/19 07:14:35  erwu2
modularized makefile updated

Revision 1.1.2.1  2014/04/10 06:24:06  erwu2
classify o2 and lebowski executable to obj folder


$Endlog$
*/
