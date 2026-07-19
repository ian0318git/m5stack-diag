/* $Id: diag_common_lib.c,v 1.2 2013/10/08 08:48:27 tirawan Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/woodlawn_linux/diag_common_lib.c,v $
 *------------------------------------------------------------------
 * Filename: diag_common_lib.c
 *
 * Description: Diag Common Function Library
 * Author: Kody Ko
 *
 * Copyright (c) 2013 by cisco Systems, Inc.
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
 * Revision 1.2  2013/10/08 08:48:27  tirawan
 * Woodlawn collapsed to main trunk
 *
 * Revision 1.1.4.2  2013/08/20 10:58:50  tirawan
 * Branch into woodlawn-branch2 and port woodlawn code
 *
 * Revision 1.1.2.1  2013/04/24 10:37:15  tirawan
 * Initial check-in for woodlawn linux code
 *
 * Revision 1.1  2013/03/13 06:42:49  kuangik
 * Add for the first time
 *
 * Revision 1.4  2012/09/21 11:42:21  kody
 * Clean up the code.
 *
 * Revision 1.3  2012/08/03 10:16:55  evanli
 * Mapping to latest O2 source code on 20120726
 *
 * Revision 1.1  2012/05/18 10:07:35  kody
 * Add diag common library
 *
 * $Endlog$
 *------------------------------------------------------------------*/

