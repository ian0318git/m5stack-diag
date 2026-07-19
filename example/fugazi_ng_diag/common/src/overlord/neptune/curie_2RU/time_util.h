/* $Id: time_util.h,v 1.1 2020/01/09 01:02:07 jiajliu Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/overlord/neptune/curie_2RU/time_util.h,v $
 *------------------------------------------------------------------
 *
 * time_util.h - time utilities
 *
 * Dec. 2018, Jiajia Liu <jiajliu@cisco.com>
 *
 * Copyright (c) 2018 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#ifndef __TIME_UTIL__
#define __TIME_UTIL__

#include <time.h>
#include <sys/time.h>
#include <sys/resource.h>

struct res_usage {
	struct rusage ru;
	struct timeval real_time;
};

struct proc_time {
	long real_time;
	long user_cpu;
	long sys_cpu;
};

long tv_usince(struct timeval *base, struct timeval *now);
long tv_msince(struct timeval *base, struct timeval *now);
long ts_nsince(struct timespec *base, struct timespec *now);
long ts_msince(struct timespec *base, struct timespec *now);
void get_res_usage(struct res_usage *now);
void proc_time_elapsed(struct res_usage *base, struct res_usage *now,
		               struct proc_time *proc);
void print_time_elapsed(struct res_usage *base, struct res_usage *now,
			            const char *prefix);
#endif

/*
 *-----------------------------------------------------------------------------
$Log: time_util.h,v $
Revision 1.1  2020/01/09 01:02:07  jiajliu
Merge Curie 2RU to main trunk

$Endlog$
 *-----------------------------------------------------------------------------
 */
