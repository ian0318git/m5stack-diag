/* $Id: pty.c,v 1.2 2019/08/06 06:56:09 alpeng Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/nightwatch/pty.c,v $
 *
 * Copyright (c) 2019 by Cisco Systems, Inc.
 * All rights reserved.
 *
 * pty.c
 *
 * pseudo-terminal related
 *
 * Initial: Jan 2018 Frank Wu
 * -----------------------------------------------------------------------------
 */

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <pty.h>

#define diag_printf printf
#define E_ERROR -1
/*
 *  execute linux command in child process, and use a new
 *  pseudo-terminal as the child's control terminal.
 *
 *  ptm_fd:      master end of the pseudo-terminal
 *  child_pid:   child process ID
 */
int pty_exec(const char *cmd, int *child_pid, int *ptm_fd)
{
    int ptm = -1, rc, pid;
    int cmdlen;
    const char *shell = getenv("SHELL");
    char slave[128];
    if (!shell) shell = "/bin/sh";

    if ((ptm = open("/dev/ptmx", O_RDWR)) < 0) {
        diag_printf("Failed to open ptmx\n");
        return E_ERROR;
    }

    if (grantpt(ptm) || unlockpt(ptm)
        || ptsname_r(ptm, slave, sizeof(slave))) {
        diag_printf("Failed to prepare ptm slave\n");
        rc = E_ERROR;
        goto OUT;
    }

    pid = fork();
    if (pid < 0) {
        diag_printf("Failed to fork: %s\n", strerror(errno));
        rc = E_ERROR;
        goto OUT;
    } else if (!pid) {
        int pts;
        // child process
        close(ptm);
        // create new session, session id == pgid == pid
        if ((pid = setsid()) < 0) {
            perror("Failed to setsid()");
            exit(1);
        }

        pts = open(slave, O_RDWR);
        if (pts < 0) {
            perror("Failed to open ptm slave device\n");
            exit(1);
        }

        if ((rc = dup2(pts, 0)) < 0) {
            perror("Failed in dup2(pts, STDIN)");
            exit(1);
        }
        if ((rc = dup2(pts, 1)) < 0) {
            perror("Failed in dup2(pts, STDOUT)");
            exit(1);
        }
        if ((rc = dup2(pts, 2)) < 0) {
            perror("Failed in dup2(pts, STDERR)");
            exit(1);
        }
        close(pts);
        if ((rc = tcsetpgrp(0, pid)) < 0) { // set pgid for the pty
            perror("Failed in tcsetpgrp()");
            exit(1);
        }

        setsid();
        cmdlen = strlen(cmd);
        if (cmdlen > 0) {
            char buf[cmdlen + 32];
            snprintf(buf, sizeof(buf), "exec %s", cmd);
            execlp(shell, shell, "-c", buf, NULL);
        }
        return 1;
    }
    rc = 0;
    *child_pid = pid;
    *ptm_fd = ptm;

OUT:
    if (rc < 0) {
        close(ptm);
    }
    return rc;
}

/*-------------------------------------------------
$Log: pty.c,v $
Revision 1.2  2019/08/06 06:56:09  alpeng
merge curie, switzer and nightwatch to trunk

Revision 1.1.2.1  2019/05/30 05:33:34  mingding
CSCvk64124-29: Support PCIe-based Nightwatch Server Module

*/
