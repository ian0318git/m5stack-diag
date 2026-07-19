/* $Id: diag_common_lib.c,v 1.2 2015/05/25 03:59:15 steja Exp $ 
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/skye_linux/src/diag_common_lib.c,v $
 *------------------------------------------------------------------
 *
 * Filename: diag_common_lib.c
 *
 * Description: Diag Common Function Library
 * Porting from Woodlawn by steja
 *
 * Copyright (c) 2013~2015 by cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */
 
#include <stdio.h>
#include <stdlib.h>
#include "common.h"

/***********************************************************************
 *  Macro Definitions
 ************************************************************************/

/***********************************************************************
 *  Static Functions Declaration
 ************************************************************************/

/***********************************************************************
 *  Functions Declaration
 ************************************************************************/
int exec_cmd(char *, char *, int);

/***********************************************************************
 *  Externs
 ************************************************************************/

/***********************************************************************
 *  Global Variable
 ************************************************************************/

/***********************************************************************
 *  Functions
 ************************************************************************/

/* ****************************************************************************
 *
 * Function: exec_cmd
 *
 * Description: Execute the linux system command.
 *
 * Input:    *command   - Point to the linux system command string
 *           *result    - Point to the buffer that records the command result.
 *           result_len - The buffer length that records the command result.
 *
 * Outputs:  PASSED - No errors encountered.
 *           FAILED - Errors encountered.
 *
 * Assumptions:
 *
 * ***************************************************************************/
int
exec_cmd (char *command, char *result, int result_len)
{
    FILE *fp;
    int status;

    fp = popen (command, "r");
    if (fp != NULL){
        fread(result, sizeof(char), result_len, fp);
    } else {
        cterr('f', 0, "Execute command failed");
        return (FAILED);
    }

    status = pclose(fp);

    if (status == -1) {
        cterr('f', 0, "pclose error");
        return (FAILED);
    }

    return (PASSED);
}

/***********************************************************************
 *  Static Functions
 ************************************************************************/

/*------------------------------------------------------------------
 * $Log: diag_common_lib.c,v $
 * Revision 1.2  2015/05/25 03:59:15  steja
 * Add Support Skye SM
 *
 * Revision 1.1.4.2  2015/04/29 11:36:32  steja
 * Code check-in to skye-branch2 for ER code review
 *
 *------------------------------------------------------------------
 * Revision 1.1.2.1  2014/07/21 01:56:52  palin2
 * Initial check-in Skye module side Diag code.
 *
 *------------------------------------------------------------------
 * Revision 1.2  2014/02/27 15:01:45  palin2
 * Initial check-in ShrinkRay SM side Diag code.
 *
 * Revision 1.1.4.3  2014/01/28 07:39:40  steja
 * Code clean up
 *
 * Revision 1.1.4.2  2013/09/13 07:00:07  palin2
 * Initial check-in ShrinkRay SM side Diag code.
 *
 * Revision 1.1.2.1  2013/06/24 09:03:34  steja
 * Checkin :
 * - Support TLK10323 Loopback test & Utility
 * - Support MV1514 Loopback test
 *
 *------------------------------------------------------------------
 * $Endlog$
 */

