/* $Id: use_power.c,v 1.2 2015/05/25 03:59:21 steja Exp $ 
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/skye_linux/src/use_power.c,v $ 
 *------------------------------------------------------------------
 *
 * use_power: Exercise TILE CPU to use worst-case power
 *
 * June 2013 - Ian Chang
 *
 * Copyright (c) 2013-2015 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */
// Copyright 2012 Tilera Corporation. All Rights Reserved.
//
//   The source code contained or described herein and all documents
//   related to the source code ("Material") are owned by Tilera
//   Corporation or its suppliers or licensors.  Title to the Material
//   remains with Tilera Corporation or its suppliers and licensors. The
//   software is licensed under the Tilera MDE License.
//
//   Unless otherwise agreed by Tilera in writing, you may not remove or
//   alter this notice or any other notice embedded in Materials by Tilera
//   or Tilera's suppliers or licensors in any way.

#include <errno.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <tmc/task.h>
#include <tmc/cpus.h>
#include "types.h" 
#include "queryflags.h"
#include "error.h"

/******************************************************************/
/*                                                                */
/*   use_power.c  - Exercise TILE CPU to use worst-case power     */
/*                                                                */
/******************************************************************/

// A simple application that draws a similar amount of power to 
// a compute-bound application such as media transcode.

// Each tile executes a tight loop that repeatedly copies a 
// configurable amount of data from one place to another in memory

// Usage:   use_power <arg>

// The arg is the number of bytes to copy in each thread
// Specify a decimal byte count with optional suffix (K or M)
// Default value: 0

// Example:   ./use_power
// Doesn't copy any data, so the CPU is executing a 
// tight loop fetched from L1I cache, without any loads
// or stores

// Example:   ./use_power 64K
// Since the tile's L2 cache is 256KB per tile, this copy
// will copy data from one place to another within the L2
// cache. This exercises the CPU and the cache logic

// Example:  ./user_power 1M
// This memory copy will require access to off-chip DDRAM
// The memory controller power will be higher
// but the core power will be lower, since the CPU is
// spending some of its time stalled waiting for data to
// come from off-chip
extern int test_patterns(int);
extern int skye_dump_temps(void);

/******************************************************************************
 *
 * Function: assign_pack_size
 *
 * Description: The work function to assign packet size
 *
 * Inputs      : core
 * Outputs     : data byte count
 *
 *****************************************************************************/
static int
assign_pack_size (int core)
{
    int     data_byte_count;
    char    *pack_size[] = {"16K", "16K", "16K", "16K", "16K", "16K",
                             "16K", "16K", "16K", "16K", "16K", "16K",
                             "32K", "32K", "32K", "32K", "32K", "32K",
                             "32K", "32K", "32K", "32K", "32K", "32K",
                             "1M" , "1M" , "1M" , "1M" , "1M" , "1M",
                             "1M" , "1M" , "1M" , "1M" , "1M" , "1M"};
    char *arg = pack_size[core];
    char suffix = toupper(arg[strlen(arg) - 1]);
    unsigned multiplier;

    switch (suffix)
    {
    case 'K': multiplier = 1024;  break;
    case 'M': multiplier = 1024 * 1024; break;
    default: multiplier = 1;
    }
    data_byte_count = atoi(arg) * multiplier;
    if (data_byte_count > 4000000)
        tmc_task_die("Byte count cannot exceed 4,000,000 bytes");
    return (data_byte_count);
}

/******************************************************************************
 *
 * Function: start_thread
 *
 * Description: The work function run by each thread.
 *
 * Inputs      : arg
 * Outputs     : NULL
 *
 *****************************************************************************/
static void *
start_thread (void *arg)
{
    int cpu = (intptr_t) arg;
    unsigned byte_count;
    byte_count = assign_pack_size(cpu);
    /* locally homed data */
    char src[byte_count], dst[byte_count];
    if (tmc_cpus_set_my_cpu(cpu) != 0)
        tmc_task_die("tmc_cpus_set_my_cpu() failed.");
    printf("Power running on %d CPUs, copying %u bytes \n", cpu, byte_count);
    /* burn cycles */
    for (;;)
    {
        if (byte_count)
            memcpy(dst, src, byte_count);
    }
    return NULL;
}

/******************************************************************************
 *
 * Function: start_thread_mem_comp
 *
 * Description: The work function run by each thread.
 *
 * Inputs      : arg
 * Outputs     : NULL
 *
 *****************************************************************************/
static void *
start_thread_mem_comp (void *arg)
{
    int cpu = (intptr_t) arg;
    unsigned byte_count;
    unsigned cmp = 0;
    byte_count = assign_pack_size(cpu);
    /* locally homed data */
    char src[byte_count], dst[byte_count];
    if (tmc_cpus_set_my_cpu(cpu) != 0)
        tmc_task_die("tmc_cpus_set_my_cpu() failed.");
    printf("Power running on %d CPUs, copying %u bytes ", cpu, byte_count);
    printf("and comparing...\n");
    /* burn cycles */
    for (;;)
    {
          memcpy(dst, src, byte_count);
          cmp = memcmp(dst, src, byte_count);
          if (cmp > 0) {
              cterr('f', 0, "memcmp failed on cpu %d CPUs\n", cpu);
              break;
          }
    }
    return NULL;
}

/******************************************************************************
 *
 * Function: tile_use_power
 *
 * Description: Main entry point
 *
 * Inputs      : arg
 * Outputs     : NULL
 *
 *****************************************************************************/
int
tile_use_power (void)
{
/* First arg is size of memcpy in each thread
 * Specify decimal byte count with optional suffix (K or M)
 *
 * Default thread stack is 8MB, so each array must be less than 4MB
 */
    unsigned byte_count;
    int ix, test_count = 3;

    printf("Enter the test loop :");
    test_count =  getdec_answer("Please enter test loop: ", 1, 0, 65535);

    /* Get set of online tiles. */
    cpu_set_t online;
    if (tmc_cpus_get_online_cpus(&online) != 0)
        tmc_task_die("tmc_cpus_get_online_cpus() failed.");

    /* Determine number of online tiles. */
    int num_threads = tmc_cpus_count(&online);

    /* Create threads */
    byte_count = assign_pack_size(0);

    int thread_id;
    for (thread_id = 0; thread_id < num_threads; thread_id++)
    {
        pthread_t handle;
        int cpu = tmc_cpus_find_nth_cpu(&online, thread_id);
        if (pthread_create(&handle, NULL, start_thread,
                       (void *)(intptr_t)cpu) != 0)
            tmc_task_die("pthread_create() failed: %s", strerror(errno));
    }

    /* Loop forever displaying time and temperature  */
    for (ix = 0; ix < test_count; ix++)
    {
        system("date");
        fflush(0);
        skye_dump_temps();
        printf("\n\n");
        fflush(0);
        sleep(5);
    }
    return 0;
}

/******************************************************************************
 *
 * Function: tile_compare_mem
 *
 * Description: function to compare memory on tile
 *
 * Inputs      : void
 * Outputs     : value
 *
 *****************************************************************************/
int
tile_compare_mem (void)
{
/* First arg is size of memcpy in each thread
 * Specify decimal byte count with optional suffix (K or M)
 *
 * Default thread stack is 8MB, so each array must be less than 4MB
 */
    unsigned byte_count;
    int ix, test_count = 3;

    printf("Enter the test loop :");
    test_count =  getdec_answer("Please enter test loop: ", 1, 0, 65535);

    /* Get set of online tiles. */
    cpu_set_t online;
    if (tmc_cpus_get_online_cpus(&online) != 0)
        tmc_task_die("tmc_cpus_get_online_cpus() failed.");

    /* Determine number of online tiles. */
    int num_threads = tmc_cpus_count(&online);

    /* Create threads */
    byte_count = assign_pack_size(0);

    int thread_id;
    for (thread_id = 0; thread_id < num_threads; thread_id++)
    {
        pthread_t handle;
        int cpu = tmc_cpus_find_nth_cpu(&online, thread_id);
        if (pthread_create(&handle, NULL, start_thread_mem_comp,
                       (void *)(intptr_t)cpu) != 0)
            tmc_task_die("pthread_create() failed: %s", strerror(errno));
    }

    /* Loop forever displaying time and temperature  */
    for (ix = 0; ix < test_count; ix++)
    {
        system("date");
        fflush(0);
        skye_dump_temps();
        printf("\n\n");
        fflush(0);
        sleep(5);
    }
    return 0;
}

/******** History ********/ 
/*
 *------------------------------------------------------------------
 * $Log: use_power.c,v $
 * Revision 1.2  2015/05/25 03:59:21  steja
 * Add Support Skye SM
 *
 * Revision 1.1.4.2  2015/04/29 11:36:44  steja
 * Code check-in to skye-branch2 for ER code review
 *
 *------------------------------------------------------------------
 * Revision 1.1.2.2  2014/08/28 08:03:24  palin2
 * Update Skye show all temp. and all voltage margin states utilities to
 * support enhanced error message.
 *
 * Revision 1.1.2.1  2014/07/21 01:57:00  palin2
 * Initial check-in Skye module side Diag code.
 *
 *------------------------------------------------------------------
 * Revision 1.2  2014/02/27 15:01:49  palin2
 * Initial check-in ShrinkRay SM side Diag code.
 *
 * Revision 1.1.4.4  2014/02/07 18:31:32  steja
 * code clean up
 *
 * Revision 1.1.4.3  2014/01/03 08:31:35  iachang
 * Display temperature with CPU Stress Test
 *
 * Revision 1.1.4.2  2013/09/13 07:00:11  palin2
 * Initial check-in ShrinkRay SM side Diag code.
 *
 * Revision 1.1.2.3  2013/07/16 09:44:55  steja
 * 1. Add utillity for dump error log and clear error log
 * 2. Add cterr on use_power function
 *
 * Revision 1.1.2.2  2013/07/01 04:06:22  steja
 * Modify Tilera tools use_power to support memory compare
 *
 * Revision 1.1.2.1  2013/06/20 03:05:31  iachang
 * Support Tilera CPU Stress Test : use_power
 *
 *------------------------------------------------------------------
 * $Endlog$
 */
