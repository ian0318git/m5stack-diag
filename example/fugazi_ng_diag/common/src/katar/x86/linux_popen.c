/* $Id: linux_popen.c,v 1.2 2019/06/14 05:24:49 mikech2 Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/katar/x86/linux_popen.c,v $
 *------------------------------------------------------------------
 * Filename:    linux_popen.c
 *
 * Description:
 *
 * Copyright (c) 2019 by cisco Systems, Inc.
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
 Execute shell command and kepp the returned string into retBuf[sizeOfBuf]
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
Revision 1.2  2019/06/14 05:24:49  mikech2
Collapse katar-branch00 to Main Trunk

Revision 1.1.2.1  2018/10/22 08:02:33  mikech2
Move project folder to common/src/katar/x86

Revision 1.1.2.1  2018/09/07 03:14:21  peteteng
Add system info utility

Revision 1.1.2.1  2018/02/27 08:06:29  harrchan
Initial viper application code base



$Endlog$
*/
