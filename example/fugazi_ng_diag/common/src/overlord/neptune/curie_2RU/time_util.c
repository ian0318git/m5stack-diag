/* $Id: time_util.c,v 1.1 2020/01/09 01:02:07 jiajliu Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/overlord/neptune/curie_2RU/time_util.c,v $
 *------------------------------------------------------------------
 *
 * time_util.c - time utilities
 *
 * Dec. 2018, Jiajia Liu <jiajliu@cisco.com>
 *
 * Copyright (c) 2018 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#include <stdio.h>

#include "time_util.h"

long tv_usince(struct timeval *base, struct timeval *now)
{
    long us;

    us = now->tv_sec - base->tv_sec;
    us *= 1000000;
    us += now->tv_usec - base->tv_usec;

    return us;
}

long tv_msince(struct timeval *base, struct timeval *now)
{
    return tv_usince(base, now) / 1000;
}

long ts_nsince(struct timespec *base, struct timespec *now)
{
    long ns;

    ns = now->tv_sec - base->tv_sec;
    ns *= 1000000000;
    ns += now->tv_nsec - base->tv_nsec;

    return ns;
}

long ts_msince(struct timespec *base, struct timespec *now)
{
    return ts_nsince(base, now) / 1000000;
}

/* get current thread time usage */
void get_res_usage(struct res_usage *now)
{
    struct timespec ts;

    getrusage(RUSAGE_SELF, &now->ru);
    clock_gettime(CLOCK_MONOTONIC, &ts);
    now->real_time.tv_sec = ts.tv_sec;
    now->real_time.tv_usec = ts.tv_nsec / 1000;
}

/* get the elapsed time */
void proc_time_elapsed(struct res_usage *base, struct res_usage *now,
               struct proc_time *proc)
{
    proc->user_cpu = tv_usince(&base->ru.ru_utime, &now->ru.ru_utime);
    proc->sys_cpu = tv_usince(&base->ru.ru_stime, &now->ru.ru_stime);
    proc->real_time = tv_usince(&base->real_time, &now->real_time);
}

static char *time2string(long us, char *buf, size_t count)
{
    unsigned long _us = us;
    const char *sign = (us < 0) ? "-" : "";

    _us = (us >= 0) ? us : 0 - us;

    if (_us >= 60000000) {
        long left = _us % 60000000;
        snprintf(buf, count, "%s%lum%lu.%03lus", sign, _us / 60000000,
                 left / 1000000, (left % 1000000) / 1000);
    } else if (_us >= 1000000)
        snprintf(buf, count, "%s%lu.%03lus", sign, _us / 1000000,
                 (_us % 1000000) / 1000);
    else if (_us >= 1000)
        snprintf(buf, count, "%s%lu.%03lums", sign, _us / 1000,
                 _us % 1000);
    else
        snprintf(buf, count, "%s%luus", sign, _us);

    return buf;
}

static void _print_time_elapsed(struct proc_time *ptime, const char *prefix)
{
    char rt[1024], ut[1024], st[1024];
    const char *_prefix = prefix ? prefix : "";

    printf("%sreal %s, user %s, sys %s\n", _prefix,
           time2string(ptime->real_time, rt, sizeof(rt)),
           time2string(ptime->user_cpu, ut, sizeof(ut)),
           time2string(ptime->sys_cpu, st, sizeof(st)));
}

void print_time_elapsed(struct res_usage *base, struct res_usage *now,
            const char *prefix)
{
    struct proc_time ptime;

    proc_time_elapsed(base, now, &ptime);
    _print_time_elapsed(&ptime, prefix);
}

/*
 *-----------------------------------------------------------------------------
$Log: time_util.c,v $
Revision 1.1  2020/01/09 01:02:07  jiajliu
Merge Curie 2RU to main trunk

$Endlog$
 *-----------------------------------------------------------------------------
 */
