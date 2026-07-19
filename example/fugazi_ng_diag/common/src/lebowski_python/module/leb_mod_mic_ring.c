/* $Id: leb_mod_mic_ring.c,v 1.2 2014/06/03 10:53:30 erwu2 Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/lebowski_python/module/leb_mod_mic_ring.c,v $
 *------------------------------------------------------------------
 * Description: MIC ring sample with pass case to show
 *              testname, prpass, and prcomplete
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

    if (test_mic_ring_fn(argc,argv) != PASSED) {
        return FAILED;
    } else {
        return PASSED;
    }
}

/*****************************************************************************
 *
 * Function   : test_mic_ring_fn
 * Description: pass case example shows testname, prpass, prcomplete info.
 * Inputs     : None
 * Outputs    : PASSED/FAILED
 *
 *****************************************************************************/
int
test_mic_ring_fn(int argc,char *argv[])
{
    /* flag usage example */
    if (verbose_mode == 1) {
        printf("\n Verbose mode flag ON \n");
        printf("\n verbose_mode in leb_mod_mic_ring \n");
    } else {
        printf("\n Verbose mode flag OFF \n");
        printf("\n non verbose mode in leb_mod_mic_ring \n");
    }

    testname("MIC RING");
    prpass(testpass, "mic ring test, ");
    prcomplete(testpass, errcount, (char *)0);
    return PASSED;
}

/******** History ********
$Log: leb_mod_mic_ring.c,v $
Revision 1.2  2014/06/03 10:53:30  erwu2
python menu collapsed to main trunk

Revision 1.1.2.2  2014/04/29 11:40:38  erwu2
update python file structure

Revision 1.1.2.1  2014/04/24 08:53:50  erwu2
merge makefile and add flag example to test


$Endlog$
*/

