/* $Id: linux_popen.c,v 1.2 2021/06/02 02:56:23 alpeng Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/hightower-sub6/linux_popen.c,v $
 *------------------------------------------------------------------
 * Filename:    linux_popen.c
 *
 * Description:
 *
 * Copyright (c) 2017 by cisco Systems, Inc.
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
    /* if script return non-zero, it will print applied faild cautions. */
    if (exitstatus)
        printf("\n%% Execute command failed");
#endif
    return retCount;
}

/******** History ********
$Log: linux_popen.c,v $
Revision 1.2  2021/06/02 02:56:23  alpeng
merge sears into trunk

Revision 1.1.4.1  2020/12/09 01:52:02  alpeng
use C comment

Revision 1.1  2020/08/19 09:50:53  markzha
*** empty log message ***

Revision 1.2  2017/08/02 14:21:46  steja
Support TSN-H/M platform code

Revision 1.1.8.2  2017/07/29 03:41:03  steja
tsn-branch5 synced with Maintrunk repositories

Revision 1.1.6.2  2017/07/20 13:38:05  steja
tsn-branch4 merge with maintrunk

Revision 1.1.4.2  2016/06/30 06:22:48  steja
tsn-branch2 sync with main trunk

Revision 1.1.2.1  2016/03/16 08:57:54  steja
add usb test


$Endlog$
*/
