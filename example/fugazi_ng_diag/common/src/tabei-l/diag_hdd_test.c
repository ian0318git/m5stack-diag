/* $Id: diag_hdd_test.c,v 1.2 2019/10/17 02:16:22 kehuang2 Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/tabei-l/diag_hdd_test.c,v $
 *------------------------------------------------------------------
 * 
 * Filename: diag_hdd_test.c
 *
 * Copyright (c) 2016-2019 by cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#include <errno.h>
#include <dirent.h>
#include <libgen.h>
#include <pthread.h>
#include <semaphore.h>
#include <time.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/select.h>
#include <stdlib.h>
#include <unistd.h>
#include "types.h"
#include "common.h"
#include "monitor.h"
#include "menu.h"
#include "error.h"
#include "proto.h"
#include "diag_hdd_test.h"
#include "nvmonvars.h"
#include <malloc.h>
#include "linux_block_test.h"

/*
 * Global extern functions
 */

/*
 * Declare local function
 */


/*******************************************************************************
 *
 * Function   :	diag_hdd_test
 * Description:	main test for hdd sata test
 * Inputs     :	void
 * Outputs    : PASSED or FAILED.
 *
 *******************************************************************************
 */
int diag_hdd_test (void) {

    int rc = FAILED;
    char *tname = "HDD read/write";
    char *hdd_dev = "/dev/hdd";
    
    testname("%s access", tname);
    
    prpass(testpass, "%s, ", tname);

    if (check_menu_flag(D_EXT_LOOPBACK)) {
        printf("\n External loopback flag is off, skip '%s'\n", tname);
        prcomplete(testpass, errcount, (char *)0);
        return (PASSED);
    }

    rc = linux_block_test(hdd_dev, 0, BLOCK_SIZE_1K, BLOCK_TEST_RANDOM, TRUE);
    if (rc == FAILED) {
        cterr('f', 0, "HDD SATA test failed.");
    }

    prcomplete(testpass, errcount, (char *)0);

    return (rc);
}

/******** History ********
$Log: diag_hdd_test.c,v $
Revision 1.2  2019/10/17 02:16:22  kehuang2
Collapse Tabei-L into main trunk

Revision 1.1.2.2  2019/10/01 03:00:49  kehuang2
Clean up code

Revision 1.1.2.1  2019/10/01 02:53:05  kehuang2
Clean up code

$Endlog$
*/
