/* $Id: ping_test.c,v 1.2 2015/05/25 03:59:16 steja Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/skye_linux/src/ping_test.c,v $
 *------------------------------------------------------------------
 *
 * ping_test.c : Using "ping" command to measure packet loss across network paths.
 *
 * July 2014 - Ian Chang
 *
 * Copyright (c) 2014-2015 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>

#include <string.h>
#include <sys/stat.h>
#include <sys/sysinfo.h>
#include <unistd.h>
#include "common.h"
#include "error.h"
#include "nvmonvars.h"
#include "types.h"

/*******************************************************************************
 *
 * Function   : ping_test
 * Description: Function to check network interface by Linux ping command.
 * Inputs     : ip_add - ip address
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
int ping_test (char *ip_add)
{
    int status = 0, ping_ret = 0;
    char buffer[80];

    sprintf(buffer,"ping -w 2 -s 1024 %s", ip_add);
    status = system(buffer);
    if (status != PASSED) {
        ping_ret = FAILED;
    } else {
        ping_ret = PASSED;
    }    
    return (ping_ret);
}

/*
 *------------------------------------------------------------------
 * $Log: ping_test.c,v $
 * Revision 1.2  2015/05/25 03:59:16  steja
 * Add Support Skye SM
 *
 * Revision 1.1.4.3  2015/05/11 13:45:45  steja
 * Code clean up <CSCuu14285>
 *
 * Revision 1.1.4.2  2015/04/29 11:36:34  steja
 * Code check-in to skye-branch2 for ER code review
 *
 *------------------------------------------------------------------
 * Revision 1.1.2.2  2014/09/17 04:56:33  palin2
 * Add function title for ping test.
 *
 * Revision 1.1.2.1  2014/07/21 01:56:54  palin2
 * Initial check-in Skye module side Diag code.
 *
 *------------------------------------------------------------------
 * Revision 1.1.2.1  2014/07/14 08:05:32  iachang
 * Support GE alive test
 *
 *------------------------------------------------------------------
 * $Endlog$
 */
