/* $Id: leb_mod_smart_cookie_echo.c,v 1.2 2014/06/03 10:53:30 erwu2 Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/lebowski_python/module/leb_mod_smart_cookie_echo.c,v $
 *------------------------------------------------------------------
 * Description: smart cookie sample with pass case to show testname, prpass
 *              , and prcomplete
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
#include "leb_mod_fru.h"

/*****************************************************************************
 *
 * Function   : main
 * Description: entry point of executable
 * Inputs     : argc, number of argument
 *              argv, arguments in leb_mod.pcfg and python menu script
 *
 * Outputs    : PASSED/FAILED
 *
 *****************************************************************************/
int
main(int argc,char *argv[])
{
    if (mod_init(argc, argv) == FAILED) {
        return FAILED;
    }

    if (test_smart_cookie_echo_fn(argc,argv) != PASSED) {
        return FAILED;
    } else {
        return PASSED;
    }
}

/*****************************************************************************
 *
 * Function   : test_smart_cookie_echo_fn
 * Description: pass case example shows testname, prpass, prcomplete info.
 * Inputs     : None
 * Outputs    : PASSED/FAILED
 *
 *****************************************************************************/
int
test_smart_cookie_echo_fn(int argc,char *argv[])
{
    /* flag usage example */
    if (ngio_pwr_after_test == 1) {
        printf("\n ngio Pwr after test flag ON \n");
        printf("\n ngio_pwr_after_test in leb_mod_smart_cookie_echo \n");
    } else {
	    printf("\n ngio Pwr after test flag OFF \n");
        printf("\n non ngio_pwr_after_test in leb_mod_smart_cookie_echo \n");
	}

    testname("Smart Cookie");
    prpass(testpass, "smart cookie echo test, ");
    prcomplete(testpass, errcount, (char *)0);
    return PASSED;
}

/******** History ******** 
$Log: leb_mod_smart_cookie_echo.c,v $
Revision 1.2  2014/06/03 10:53:30  erwu2
python menu collapsed to main trunk

Revision 1.1.2.2  2014/04/29 11:40:38  erwu2
update python file structure

Revision 1.1.2.1  2014/04/24 08:53:50  erwu2
merge makefile and add flag example to test


$Endlog$
*/

