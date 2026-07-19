/* $Id: leb_mod_init.c,v 1.2 2014/06/03 10:53:30 erwu2 Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/lebowski_python/module/leb_mod_init.c,v $
 *------------------------------------------------------------------
 * Description: initialization of Lebowski module side if needed
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
 * Description : entry point of leb_mod_init, print out boot up msg
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
    printf("====================Boot up Lebowski module side==============\n");
    printf("Optional: Lebowski module side initialization\n");
    return PASSED;
}

/******** History ******** 
$Log: leb_mod_init.c,v $
Revision 1.2  2014/06/03 10:53:30  erwu2
python menu collapsed to main trunk

Revision 1.1.2.2  2014/04/29 11:40:38  erwu2
update python file structure

Revision 1.1.2.1  2014/04/24 08:53:50  erwu2
merge makefile and add flag example to test


$Endlog$
*/
