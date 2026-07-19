/* $Id: linux_popen.c,v 1.2 2018/08/06 02:57:21 harrchan Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/linux_popen.c,v $
 *------------------------------------------------------------------
 * Filename:    linux_popen.c
 *
 * Description: This file is for excute shell command
 *
 * Copyright (c) 2018 by cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>  /* getpid */
#include <strings.h>  /* for bzero*/
#include <string.h>
#include <errno.h>
#include <sys/types.h> /* getpid */
#include <features.h>
#include <sys/ioctl.h>
#include <pthread.h>

#include "types.h"
#include "proto.h"
#include "common.h"
#include "error.h"

/*
 Execute shell command and keep the returned string into retBuf[sizeOfBuf]
*/
int ExecuteCmdbyPopen(char *cmd, char *retBuf, int sizeOfBuf)
{
    FILE *f;
    char *pRetBuf = retBuf;
    int  count, retCount = 0;
#if DEBUG
    int exitstatus;
#endif

    if( cmd==NULL || retBuf==NULL ) {
#if DEBUG
        printf("\n%% Execute command failed");
#endif
        return 0;
    } else
        f = popen(cmd, "r");

    if (f) {
        while (1)
        {
            *pRetBuf = '\0';
            count = 0;
            fgets(pRetBuf, sizeOfBuf-retCount, f);
            count = strlen(pRetBuf);
            if (count == 0) {
                break;
            }
            pRetBuf += count;
            retCount += count;
        }
    }

    pclose(f);
#if DEBUG
    exitstatus = WEXITSTATUS(ret);
    // if script return non-zero, it will print applied faild cautions.
    if (exitstatus)
        printf("\n%% Execute command failed");
#endif
    return retCount;
}

/******** History ********
$Log: linux_popen.c,v $
Revision 1.2  2018/08/06 02:57:21  harrchan
Merge viper E2E to the main trunk (CSCvk28469)

Revision 1.1.2.1  2018/02/27 08:06:29  harrchan
Initial viper application code base



$Endlog$
*/
