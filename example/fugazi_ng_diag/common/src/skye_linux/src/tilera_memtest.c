/* $Id: tilera_memtest.c,v 1.2 2015/05/25 03:59:21 steja Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/skye_linux/src/tilera_memtest.c,v $
 *------------------------------------------------------------------
 *
 * FILE NAME: tilera_memtest.c
 *
 * July 2013 - Ian Chang
 *
 * Copyright (c) 2013 ~ 2015 by Cisco Systems, Inc.
 * All rights reserved.
 *
 */
/* TILERA_LICENSE
 * Test free RAM by repeatedly writing and reading a changing pattern.
 */
/* Test just about all available RAM by writing and reading a pattern
 * with all available tiles.
 * Even though this test will not cover all installed RAM, it covers almost
 * all RAM not currently allocated, and it should uncover any intermittent
 * signal integrity issues because the memory accesses will be intense.
 *
 * This program optionally accepts the following options on the command line:
 *  --passes <n> : number of passes through memory to make.
 *                Each pass through takes 3 to 4 seconds.
 *  --size <n>   : bytes of memory to test.  (A trailing 'k', 'm', or 'g'
 *                may be used to specify kilo-, mega-, or gigabytes.)
 */
#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <pthread.h>
#include <sys/mman.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/fcntl.h>

#include <arch/chip.h>
#include <signal.h>
#include <sys/wait.h>
#include <tmc/alloc.h>
#include <tmc/cpus.h>
#include <tmc/sync.h>
#include <tmc/task.h>
#include "types.h" 
#include "queryflags.h"
#include "common.h" 
#include "proto.h" 
#include "error.h" 
#include "nvmonvars.h"

/*
 *  Enxterns
 */
extern void skye_dump_env_prm(void);

/*
 *  Globals  
 */
// Number of passes to run if user does not specify.
#define DEFAULT_PASSES 3
#define MIN_PASSES     1
#define PTHREAD_DELAY  3
#define ERR_BUF_SIZE   80  /* size for error message */

unsigned long long mem_size_to_test;
// Amount of free memory to leave free.  This needs to account for the
// memory we'll use to run our processes (stack, etc.), as well as memory
// which might be used by other random system activity while we're running.
// This is a minimum; we reserve more than this on machines with more than
// 4 GB.  Also note that this is more of a memory stress test than a total
// memory coverage test, so it's better to have this be a little high than
// a little low.
#define RESERVE_FREE_MEM (512 * 1024 * 1024)
// Alignment (and min size) of blocks of memory sent to each process.
// Needs to be bigger than the L2 Cache to ensure loads and stores go out to
// memory.
#define MEM_BLOCK_ALIGN (CHIP_L2_CACHE_SIZE() * 2)

// Tag used in ilib messages.
#define MSG_TAG 100

// Size of test operation.
#ifndef OP_SIZE
#ifdef __LP64__
#define OP_SIZE 8
#else
#define OP_SIZE 4
#endif
#endif

#if OP_SIZE == 8
#define OP_TYPE uint64_t
#elif OP_SIZE == 4
#define OP_TYPE uint32_t
#elif OP_SIZE == 2
#define OP_TYPE uint16_t
#elif OP_SIZE == 1
#define OP_TYPE uint8_t
#else
#error Bad OP_SIZE, must be 8, 4, 2, or 1
#endif

/** Maximum number of memory controllers; this is way more than we ever
 *  expect to have, but that shouldn't hurt. */
#define MAX_MCS 8

//
// Undefine this to use less-random-looking data; bad for coverage, good
// for debug of certain issues.
//
#define USE_CRC_DATA

/** Convert a virtual address to a client physical address.
 *
 * @param va Virtual address to convert.
 * @return Client physical address.
 */
static unsigned long long
va2cpa (void* va)
{
    char line[128];
    uintptr_t uiva = (uintptr_t) va;
    
    static FILE *fp;
    if (!fp)
    {
        fp = fopen("/proc/self/pgtable", "r");
        if (!fp)
        {
            fclose(fp);
            return ~0ULL;
        }
    }
    else
        rewind(fp);
  
    uintptr_t pgmsk = ~((uintptr_t) getpagesize() - 1); 
  
    while (fgets(line, sizeof(line), fp))
    {
        long lineva;
        char prot[16];
        unsigned long long linepa;
      
        if (sscanf(line, "%lx %s PA=%llx ", &lineva, prot, &linepa) != 3)
            break;
      
        if ((uiva & pgmsk) == (lineva & pgmsk)) {
            fclose(fp);
            return linepa + (uiva & ~pgmsk);
        }
    }
    fclose(fp);
    return ~0ULL;
}


/** Memory testing function run by each tile (or process).
 *
 * @param mem_size Size of block in bytes this process is to test.
 * @param total_passes Number of write/read passes through that block to do.
 * @return Number of errors found.
 */
int
test_mem (int rank, int count, unsigned long long mem_size, int total_passes)
{
    unsigned long long mem_words;
    OP_TYPE* mem_area = NULL;
    char err_buf[ERR_BUF_SIZE];

    mem_size = mem_size_to_test;
    mem_words = mem_size / sizeof (OP_TYPE);
  
    // Allocate memory buffer this process will test.  Don't bother if the
    // size is larger than we can possibly get with malloc.
    if ((size_t) mem_size == mem_size)
        mem_area = malloc(mem_size);
        if ((NVRAM)->diagflag & D_VERBOSE) {
            printf("\nTesting %lld MBytes using %d tiles, %d passes",
                    mem_size >> 20, rank, total_passes);
        }
    if (mem_area == NULL)
        tmc_task_die(
    "Process rank %d: malloc for %#llx bytes failed.  You may have \n"
    "specified too much memory with the --size option, or you may need to make\n"
    "more tiles available to memtest (%d %s available currently).  Try reducing\n"
    "the memory size, using the --size option, or increasing the number of\n"
    "tiles, using the taskset command or tile-monitor's --tiles option.\n",
        rank, mem_size, count, (count == 1) ? "is" : "are");

  // Create masks to xor with pattern generator number, so that all bits
  // are written with both 0 and 1 and different possible data bit shorts are
  // checked.
    static const unsigned long masks[] = {
#if OP_SIZE > 4
        0x0000000000000000UL, 0xffffffffffffffffUL,
        0x3333333333333333UL, 0xccccccccccccccccUL,
        0x9999999999999999UL, 0x6666666666666666UL,
        0x5555555555555555UL, 0xaaaaaaaaaaaaaaaaUL,
#else
        0x00000000, 0xffffffff,
        0x33333333, 0xcccccccc,
        0x99999999, 0x66666666,
        0x55555555, 0xaaaaaaaa,
#endif
    };
    int error_count = 0;
    for (int pass_num = 0; pass_num < total_passes; pass_num++)
    {
        // Makes a better test if each tile is writing different values.
        // Make sure accum is not 0 starting out or the CRC32 instruction will
        // always return 0.
        unsigned long accum = rank + 1;
        // Write the pattern.
        for (unsigned long long addr = 0; addr < mem_words; addr++)
        {
#ifdef USE_CRC_DATA
        accum = __insn_crc32_32(accum, 0);
#if OP_SIZE > 4         //Srhinkray Use The OP_SIZE
       accum = (accum << 32) | __insn_crc32_32(accum, 0);
#endif
        mem_area[addr] = accum ^ masks[pass_num & 7];
#ifdef DEBUG
        if ((NVRAM)->diagflag & D_VERBOSE) {
            if (addr == (mem_words - 1)) {
                printf("\n CPU(%d) Start_add = 0x%llx,End_add = 0x%llx",rank,
                        va2cpa(&mem_area[addr - mem_words + 1]),
                        va2cpa(&mem_area[addr]));
            }
        }
#endif
#else
        accum = accum;
#if OP_SIZE == 8
        mem_area[addr] = 0xABCD000000000000UL |
                        ((unsigned long long) rank << 36) | addr;
#elif OP_SIZE == 4
        mem_area[addr] = (rank << 28) | (addr & 0xFFFFFFF);
#elif OP_SIZE == 2
        mem_area[addr] = (rank << 12) | (addr & 0xFFF);
#elif OP_SIZE == 1
        mem_area[addr] = (rank << 4) | (addr & 0xF);
#endif
#endif
        }  // end for addr: write

        // Read/validate the values just written.
        accum = rank + 1;
        for (unsigned long long addr = 0; addr < mem_words; addr++)
        {
#ifdef USE_CRC_DATA
            accum = __insn_crc32_32(accum, 0);
#if OP_SIZE > 4
            accum = (accum << 32) | __insn_crc32_32(accum, 0);
#endif
            accum = accum;
            OP_TYPE exp_data = accum ^ masks[pass_num & 7];
#else
#if OP_SIZE == 8
            OP_TYPE  exp_data = 0xABCD000000000000UL |
                ((unsigned long long) rank << 36) | addr;
#elif OP_SIZE == 4
            OP_TYPE exp_data = (rank << 28) | (addr & 0xFFFFFFF);
#elif OP_SIZE == 2
            OP_TYPE exp_data = (rank << 12) | (addr & 0xFFF);
#elif OP_SIZE == 1
            OP_TYPE exp_data = (rank << 4) | (addr & 0xF);
#endif
#endif
            OP_TYPE received_data = mem_area[addr];
            if (received_data != exp_data)
            {
            // TODO: We want to print out a PA here, and the DIMM label.
            // TODO: When there are many errors, more than one process can try
            //       to print at the same time, and their messages get jumbled.
            //       It would be nice to somehow make these prints atomic.
                fprintf(stderr, "\nERROR process rank %d: mismatch at VA %p, "
                        "CPA 0x%llx.\n", rank, &mem_area[addr],
                va2cpa(&mem_area[addr]));
		        sprintf(err_buf, "\nERROR process rank %d: mismatch at VA %p, "
                        "CPA 0x%llx.\n", rank, &mem_area[addr],
                        va2cpa(&mem_area[addr]));
#if OP_SIZE > 4
                fprintf(stderr, "Expected data 0x%016lx but got 0x%016lx "
                        "(XOR 0x%016lx)\n",
#else
                fprintf(stderr, "Expected data 0x%08x but got 0x%08x "
                    "(XOR 0x%08x)\n",
#endif
                    exp_data, received_data, received_data ^ exp_data);
#if OP_SIZE > 4
		        sprintf(err_buf, "Expected data 0x%016lx but got 0x%016lx "
                        "(XOR 0x%016lx)\n",
#else
		        sprintf(err_buf, "Expected data 0x%08x but got 0x%08x "
                    "(XOR 0x%08x)\n",
#endif
                    exp_data, received_data, received_data ^ exp_data);
                error_count++;
                cterr('f', 0, err_buf);
            }
        }  // end for addr: read

        // Have a different tile print each progress message so that one
        // tile does not get way behind all the others.
        if ((pass_num % count) == rank)
        {
            printf(".");
            fflush(stdout);
        }
    }  // end for pass_num < total_passes
    free(mem_area);
    return error_count;
}

/******************************************************************************
 *
 * Function: start_thread
 *
 * Description: The work function run by each thread
 *
 * Inputs      : arg
 * Outputs     : NULL
 *
 *****************************************************************************/
static void *
start_thread (void *arg)
{
    int cpu = (intptr_t) arg;
    int total_passes;
    cpu_set_t cpus;
    int errors;
    int count = 36;
    pthread_detach(pthread_self()); 
    if (!(diagflag_xram & D_MIN_TEST_TIME)) { 
        total_passes = MIN_PASSES;
    } else { 
        total_passes = DEFAULT_PASSES;
    } 
    // Run the test on all tiles.
    if (tmc_cpus_get_online_cpus(&cpus) != 0)
        tmc_task_die("tmc_cpus_get_online_cpus() failed.");
    cpu = tmc_cpus_find_nth_cpu(&cpus, cpu);
    if (tmc_cpus_set_my_cpu(cpu) != 0)
        tmc_task_die("tmc_cpus_set_my_cpu() failed.");
        errors = test_mem(cpu, count, mem_size_to_test, total_passes);
    return NULL;
}

/******************************************************************************
 *
 * Function: memory_all_cores_test
 *
 * Description: The work function run by each test memory all cores
 *
 * Inputs      : argc and arv
 * Outputs     : PASSED or FAILED
 *
 *****************************************************************************/
int
memory_all_cores_test (int argc, char* argv[])
{
    cpu_set_t cpus;
    int thread_id = 0;
    pthread_t handle;
    void  *pthr_rv;
    int rc;
    int total_passes;
    FILE* minfo;
    unsigned long long force_free_mem = 0;
    unsigned long long free_mem = 0;
    unsigned long long sequestered_mem = 0;

    if (!(diagflag_xram & D_MIN_TEST_TIME)) { 
        total_passes = MIN_PASSES;
    } else { 
        total_passes = DEFAULT_PASSES;
    } 

    /* Get set of online tiles. */
    if (tmc_cpus_get_online_cpus(&cpus) != 0)
        tmc_task_die("tmc_cpus_get_online_cpus() failed.");

    /* Determine number of online tiles. */
    int count = tmc_cpus_count(&cpus);
  
    if (force_free_mem)
    {
        free_mem = force_free_mem;
    }
    else
    {
        minfo = fopen("/proc/meminfo", "r");
        if (minfo == NULL)
            tmc_task_die("Could not open /proc/meminfo.");
    
        // Read one line of meminfo per loop iteration.
        // End when hit end of file, or when the free memory line is found.
        char next_line[80];
        while (fgets(next_line, 80, minfo) != NULL)
        {
            int found = 0;
            if (sscanf(next_line, "MemFree: %lld kB", &free_mem) == 1)
		        found++;
            if (sscanf(next_line, "Sequestered: %lld kB", &sequestered_mem) == 1)
		        found++;
            if (found == 2)
                break;
        }
        // If the free memory line was never found, free_mem will still be zero,
        // and we'll fail out in the minimum free memory check below.
    
        // Convert free memory from kB to bytes.
        free_mem <<= 10;
    
        // Convert sequestered memory from kB to bytes.
        sequestered_mem <<= 10;
    
        free_mem += sequestered_mem;
    
        // We make sure to leave at least RESERVE_FREE_MEM, or one eighth of
        // the available free memory, whichever is larger, unused.
        if (RESERVE_FREE_MEM > free_mem / 8)
            free_mem -= RESERVE_FREE_MEM;
        else
            free_mem -= free_mem / 8;
    }

    // Minimum amount of free memory to run a test, based on block alignment.
    unsigned long long min_free_mem = count * MEM_BLOCK_ALIGN;
  
    if (free_mem < min_free_mem)
        tmc_task_die("Minimum amount of memory to test with %d tiles is "
                   "%llu bytes.", count, min_free_mem);
  
    // Calculate amount of memory each tile should test.
    // Make the number aligned to keep things neat.
    mem_size_to_test = (free_mem / count) & -MEM_BLOCK_ALIGN;

    prpass(testpass,"\nTesting %lld MBytes using %d tiles, %d passes, ",
           (mem_size_to_test * count) >> 20, count, total_passes);

    for (thread_id = 0; thread_id < count; thread_id++)
    {
        int cpu = tmc_cpus_find_nth_cpu(&cpus, thread_id);
        if (pthread_create(&handle, NULL, start_thread,
                       (void *)(intptr_t)cpu) != 0)
            tmc_task_die("pthread_create() failed: %s", strerror(errno));
    }

    /* Sync the tx and rx in here and check the rx is pass or fail */
    pthread_join(handle, (void **)&pthr_rv);
    if (pthr_rv != PASSED) {
        printf("\npthread join failed\n");
        rc = FAILED;
    } else {
        rc = PASSED;
    }
    fclose(minfo);
    sleep(PTHREAD_DELAY); 
    return rc;
}

/*****************************************************************************
 *
 * Function   : mtest_all_cores
 * Description: Memory test on all 36 cores
 * Inputs     : void
 * Outputs    : exit status
 *
 *****************************************************************************/
int
mtest_all_cores (void)
{
    int result = 0;
    testname("Skye Memory");
    prpass(testpass, "All Cores ");

    /* Show Skye current enivronmental parameters */
    if (DIAGFLAG & D_VERBOSE) {
        skye_dump_env_prm();
    }

    result = memory_all_cores_test(0, 0);
    if (result == FAILED) {
        cterr('f',0, "Failed For All Cores ");
        return (result);
    }
 
    return(result);
}

/*
 *------------------------------------------------------------------
 * $Log: tilera_memtest.c,v $
 * Revision 1.2  2015/05/25 03:59:21  steja
 * Add Support Skye SM
 *
 * Revision 1.1.4.4  2015/04/30 08:33:54  steja
 * Clean up code
 *
 * Revision 1.1.4.2  2015/04/29 11:36:44  steja
 * Code check-in to skye-branch2 for ER code review
 *
 *------------------------------------------------------------------
 * Revision 1.1.2.3  2014/09/18 07:03:06  palin2
 * Added to show current enivronmental parameters in memory test with VERBOSE flag.
 *
 * Revision 1.1.2.2  2014/09/17 04:35:08  palin2
 * Updated Skye enhanced error message.
 *
 * Revision 1.1.2.1  2014/07/21 01:57:00  palin2
 * Initial check-in Skye module side Diag code.
 *
 *------------------------------------------------------------------
 * Revision 1.2  2014/02/27 15:01:48  palin2
 * Initial check-in ShrinkRay SM side Diag code.
 *
 * Revision 1.1.4.3  2014/02/07 18:31:32  steja
 * code clean up
 *
 * Revision 1.1.4.2  2013/09/13 07:00:11  palin2
 * Initial check-in ShrinkRay SM side Diag code.
 *
 * Revision 1.1.2.2  2013/08/09 08:48:40  iachang
 * Fixed "Could not open /proc/meminfo" issue after 1021 loop.
 *
 * Revision 1.1.2.1  2013/07/30 06:51:13  iachang
 * Support memory test on all cores.
 *
 *------------------------------------------------------------------
 * $Endlog$
 */
