/* $Id: o2_platform_init.c,v 1.2 2014/06/03 10:53:31 erwu2 Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/o2_python_example/o2_platform_init.c,v $
 *------------------------------------------------------------------
 * Description: initialization of o2 platform if needed
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

/*****************************************************************************
 *
 * Function    : main
 *
 * Description : entry point of o2_platform_init, print out boot up msg
 *
 * Inputs      : argc, number of argument
 *               argv, arguments
 *
 * Outputs     : PASSED/FAILED
 *
 *****************************************************************************/
int
main(int argc,char *argv[])
{
    printf("====================Boot up o2 ======================\n");
    printf("Optional: o2 platform initialization\n");
    return PASSED;
}

/******** History ******** 
$Log: o2_platform_init.c,v $
Revision 1.2  2014/06/03 10:53:31  erwu2
python menu collapsed to main trunk

Revision 1.1.2.1  2014/04/10 06:24:05  erwu2
classify o2 and lebowski executable to obj folder


$Endlog$
*/
