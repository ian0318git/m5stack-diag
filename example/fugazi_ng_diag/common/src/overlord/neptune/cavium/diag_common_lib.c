/* $Id: diag_common_lib.c,v 1.2 2018/05/18 09:24:56 alpeng Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/overlord/neptune/cavium/diag_common_lib.c,v $
 *------------------------------------------------------------------
 * Filename: diag_common_lib.c
 *
 * Description: Diag Common Function Library
 * Author: Mecca Ho
 *
 * Copyright (c) 2018 by cisco Systems, Inc.
 * All rights reserved.
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
int exec_cmd (char *command, char *result, int result_len)
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
 * Revision 1.2  2018/05/18 09:24:56  alpeng
 *  Neptune merge to trunk with tag <neptune-branch-0518>
 *
 * Revision 1.1.2.2  2016/11/15 07:45:20  alpeng
 * fix depend and transfer to unix format
 *
 * Revision 1.1.2.1  2016/07/07 09:04:30  meho
 * 1. Added BCM54194 RDB register r/w utility.
 * 2. Added GE PHY internal/external loopback skeleton.
 * 3. Added 10GE PHY internal/external loopback skeleton.
 *
 *
 * $Endlog$
 *------------------------------------------------------------------*/

