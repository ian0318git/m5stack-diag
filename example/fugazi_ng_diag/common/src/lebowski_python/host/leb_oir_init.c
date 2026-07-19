/* $Id: leb_oir_init.c,v 1.2 2014/06/03 10:53:29 erwu2 Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/lebowski_python/host/leb_oir_init.c,v $
 *------------------------------------------------------------------
 * Description: OIR LTC4215 Init sample with pass case to show testname,
 *              prpass, and prcomplete
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
#include "o2_util.h"
#include "platform_fru.h"

/*****************************************************************************
 *
 * Function    : main
 *
 * Description : entry point of executable
 *
 * Inputs      : argc, number of argument
 *               argv, arguments in o2.pcfg and python menu script
 *
 * Outputs     : PASSED/FAILED
 *
 *****************************************************************************/
int
main(int argc,char *argv[])
{
    FILE *fp;
    int slot_num = 0;

    if (plat_init(argc, argv) == FAILED) {
        return FAILED;
    }

    fp = fopen(SLOT_ID_PATH, "r");
    if (fp) {
        fscanf (fp, "%d", &slot_num);
    } else {
        printf("open %s fail!\n",SLOT_ID_PATH);
        return FAILED;
    }
    fclose(fp);

    if (ltc4215_init_fn(slot_num) != PASSED) {
        return FAILED;
    } else {
        return PASSED;
    }
}

/*****************************************************************************
 *
 * Function    : ltc4215_init_fn
 *
 * Description : ltc4215 init pass case example shows testname, prpass,
 *               and prcomplete info.
 *
 * Inputs      : slot_num - slot number passed from slot_id.tmp
 *
 * Outputs     : PASSED/FAILED
 *
 *****************************************************************************/
int
ltc4215_init_fn(int slot_num)
{
    /* flag usage example */
    if (exec_authentication == 1) {
        printf("\n eXec authentication flag ON \n");
        printf("\n exec_authentication in leb_oir_init \n");
    } else {
        printf("\n eXec authentication flag OFF \n");
        printf("\n non exec_authentication in leb_oir_init \n");
    }

    testname("LTC4215 OIR Init in sm slot %d",slot_num);
    prpass(testpass, "Sub-test - Hardcoded, ");
    prcomplete(testpass, errcount, (char *)0);
    return PASSED;
}

/******** History ********
$Log: leb_oir_init.c,v $
Revision 1.2  2014/06/03 10:53:29  erwu2
python menu collapsed to main trunk

Revision 1.1.2.3  2014/04/29 11:40:40  erwu2
update python file structure

Revision 1.1.2.2  2014/04/24 08:53:52  erwu2
merge makefile and add flag example to test

Revision 1.1.2.1  2014/04/10 06:24:06  erwu2
classify o2 and lebowski executable to obj folder


$Endlog$
*/
