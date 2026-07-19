/* $Id: linux_coretest.c,v 1.5 2019/08/06 06:56:06 alpeng Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/linux_coretest.c,v $
 *------------------------------------------------------------------
 *
 * Copyright (c) 2018-2019 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include "endians.h"
#include "defs.h"
#include "types.h"
#include "nvsysvars.h"
#include "error.h"
#include "pcmap.h"
#include "strings.h"
#include "menu.h"
#include "common.h"
#include "types.h"
#include "assert.h"
#include "proto.h"
#include "linux_api.h"
#include "linux_coretest.h"

/*
 * Declare local function
 */
int cpu_mem_test_thread(void *);
int cpu_mem_test(int);
int cpu_core_number_test(int);
int linux_cpu_core_test(int);
int linux_display_cpucore(char *, int);

/*
 * Global variables
 */

/* SYS_getcpu is not defined in x86 CPU */
#ifndef SYS_getcpu
#define SYS_getcpu 309
#endif

unsigned char * mem_test_array_ptr = NULL;
boolean signal_flag = THREAD_STOP;
pthread_mutex_t thread_lock;
volatile unsigned int thread_finish_flag = 0;
unsigned char test_pattern[MAX_CPU_NUM] = {
    TEST_PATTERN_A0, TEST_PATTERN_A1, TEST_PATTERN_A2, TEST_PATTERN_A3,
    TEST_PATTERN_A4, TEST_PATTERN_A5, TEST_PATTERN_A6, TEST_PATTERN_A7,
    TEST_PATTERN_A8, TEST_PATTERN_A9, TEST_PATTERN_AA, TEST_PATTERN_AB,
    TEST_PATTERN_AC, TEST_PATTERN_AD, TEST_PATTERN_AE, TEST_PATTERN_AF,
    TEST_PATTERN_B0, TEST_PATTERN_B1, TEST_PATTERN_B2, TEST_PATTERN_B3,
    TEST_PATTERN_B4, TEST_PATTERN_B5, TEST_PATTERN_B6, TEST_PATTERN_B7,
    TEST_PATTERN_B8, TEST_PATTERN_B9, TEST_PATTERN_BA, TEST_PATTERN_BB,
    TEST_PATTERN_BC, TEST_PATTERN_BD, TEST_PATTERN_BE, TEST_PATTERN_BF,
};

/*******************************************************************************
 * Function: cpu_mem_test_thread
 *
 * Description : Test thread for each CPU. 
 * Default command: NULL
 *              
 * Inputs: Data pointer for thread argument.
 *
 * Output: PASSED/FAILED
 ******************************************************************************* */
int cpu_mem_test_thread (void *data)
{
    int ix;
    int timeout = 0;
    int test_start, test_end;
    int core_no = *(int*)data;
    cpu_set_t cpuset;
    int current_cpu, test_tid, status;

    /* Pthread detach to avoid memory leak. */
    status = pthread_detach(pthread_self());
    if (status < 0) {
        cterr('f',0, "Pthread detach for cpu core:%d failed.",core_no);
        /* Before thread exit, write finish flag to let main process know. */
        pthread_mutex_lock(&thread_lock);
        thread_finish_flag |= (PER_CPU_MEM_TEST_DONE<<core_no);
        pthread_mutex_unlock(&thread_lock);
        pthread_exit(NULL);
        return(FAILED);   
    }     

    /* Set thread affinity. */
    CPU_ZERO(&cpuset);
    CPU_SET(core_no, &cpuset);
    sched_setaffinity(0, sizeof(cpuset), &cpuset);
    status = syscall(SYS_getcpu, &current_cpu, NULL, NULL );
    if (status < 0) {
        cterr('f',0, "Pthread for cpu core:%d syscall getcpu failed.",core_no);
        /* Before thread exit, write finish flag to let main process know. */
        pthread_mutex_lock(&thread_lock);
        thread_finish_flag |= (PER_CPU_MEM_TEST_DONE<<core_no);
        pthread_mutex_unlock(&thread_lock);
        pthread_exit(NULL);
        return(FAILED);        
    }

    test_tid = syscall(SYS_gettid);
    if (test_tid < 0) {
        cterr('f',0, "Pthread for cpu core:%d syscall gettid failed.",core_no);
        /* Before thread exit, write finish flag to let main process know. */
        pthread_mutex_lock(&thread_lock);
        thread_finish_flag |= (PER_CPU_MEM_TEST_DONE<<core_no);
        pthread_mutex_unlock(&thread_lock);
        pthread_exit(NULL);
        return(FAILED);        
    }
    

    /* Get test memory location for testing CPU.  */
    test_start = core_no * CPU_MEM_TEST_SIZE;
    test_end = test_start + CPU_MEM_TEST_SIZE;

    msleep(CPU_MEM_TIME_50MS);

    if (current_cpu != core_no) {
        cterr('f',0, "Testing Thread TID:%d is running on the wrong CPU:%d, expected CPU:%d. ", test_tid, current_cpu, core_no);
        /* Before thread exit, write finish flag to let main process know. */
        pthread_mutex_lock(&thread_lock);
        thread_finish_flag |= (PER_CPU_MEM_TEST_DONE<<core_no);
        pthread_mutex_unlock(&thread_lock);
        pthread_exit(NULL);
        return(FAILED);        
    }else {
        printf("\n Testing Thread TID:%d is running on CPU:%d, \n - testing memory offset from 0x%04X to 0x%04X, with pattern:0x%X.\n", test_tid, current_cpu, test_start, test_end-1,  test_pattern[core_no]);
    }

    /* If current signal is THREAD_STOP then wait THREAD_START until timeout. */
    timeout = 0;
    while (signal_flag != THREAD_START) { 
        
        if (timeout < CPU_MEM_TEST_TIMEOUT) {
            timeout++;
            msleep(CPU_MEM_TIME_250MS);
        } else {
            cterr('f',0, "Testing Thread TID:%d waiting start signal timeout.", test_tid);
            /* Before thread exit, write finish flag to let main process know. */
            pthread_mutex_lock(&thread_lock);
            thread_finish_flag |= (PER_CPU_MEM_TEST_DONE<<core_no);
            pthread_mutex_unlock(&thread_lock);
            pthread_exit(NULL);
            return(FAILED);
        }
    }
    
    /* Fill pattern. */
    for (ix = test_start; ix < test_end; ix++) {
        *(mem_test_array_ptr+ix) = test_pattern[core_no];
    }

    /* Write finish flag to let main process know. */
    pthread_mutex_lock(&thread_lock);
    thread_finish_flag |= (PER_CPU_MEM_TEST_DONE<<core_no);
    pthread_mutex_unlock(&thread_lock);

    /* If current signal is THREAD_START then wait THREAD_STOP until timeout. */
    timeout = 0;
    while (signal_flag != THREAD_STOP) {
        
        if (timeout < CPU_MEM_TEST_TIMEOUT) {
            timeout++;
            msleep(CPU_MEM_TIME_250MS);
            if ((NVRAM)->diagflag & D_VERBOSE) {
                printf("DBG: [Thread] Pthread for core %d is waiting stop signal...%d\n",core_no, timeout);
            }
        } else {
            cterr('f',0, "[Thread] Testing Thread TID:%d(core %d) waiting stop signal timeout.", test_tid, core_no);
            pthread_exit(NULL);
            return(FAILED);
        }
    }

    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("DBG: Testing Thread TID:%d - receive stop signal and exit\n",test_tid);
    }

    /* Pthread exit. */
    pthread_exit(NULL);
    return(PASSED);

}

/*******************************************************************************
 * Function: cpu_mem_test
 *
 * Description : Test for each CPU. 
 * Default command: NULL
 *              
 * Inputs: number of expected CPU cores.
 *
 * Output: PASSED/FAILED
 ******************************************************************************* */
int cpu_mem_test(int expected_cpu_core_number)
{

    unsigned char expect_pattern = 0;
    unsigned int cpu_mem_test_finish = 0;
    int ix = 0, jx = 0, rc = 0, timeout = 0;
    int test_start = 0, test_end = 0;
    int * flag = NULL;
    pthread_t * thread_core = NULL;
    thread_finish_flag = 0;

    /* Flag pointer initialize. */
    flag = malloc(expected_cpu_core_number * sizeof(int));
    if (!flag) {
        cterr('f',0, "Flag initialization failed");
        return (FAILED);
    }
    
    /* Pthread pointer initialize. */
    thread_core = malloc(expected_cpu_core_number * sizeof(pthread_t));
    if (!thread_core) {
        cterr('f',0, "Pthread initialization failed");
        free(flag);
        return (FAILED);
    }

    /* Test memory initialize. */
    mem_test_array_ptr = malloc(expected_cpu_core_number * CPU_MEM_TEST_SIZE * sizeof(unsigned char));
    if (!mem_test_array_ptr) {
        cterr('f',0, "Test memory initialization failed");
        free(flag);
        free(thread_core);
        return (FAILED);
    }

    memset(mem_test_array_ptr, 0x00, expected_cpu_core_number * CPU_MEM_TEST_SIZE * sizeof(unsigned char));
    memset(flag, 0x00, expected_cpu_core_number * sizeof(int));

    /* Pthread mux initialize. */
    if (pthread_mutex_init(&thread_lock, NULL) != 0) {
        cterr('f',0, "Pthread mux initialization failed");
        free(mem_test_array_ptr);
        free(flag);
        free(thread_core);
        return (FAILED);
    }

    /* Set signal_flag with thread stop first, wait each pthread ready. */
    signal_flag = THREAD_STOP;

   
    /* Calculate CPU test finish flag */ 
    for (ix = 0; ix < expected_cpu_core_number; ix++) {
        cpu_mem_test_finish |= (PER_CPU_MEM_TEST_DONE << ix);
    }


    /* Create pthread for each core. */
    for (ix = 0; ix < expected_cpu_core_number; ix++) {
        *(flag+ix) = ix;
        if (pthread_create((thread_core+ix), NULL, (void *)cpu_mem_test_thread, (void *)(flag+ix))) {
            cterr('f',0, "Pthread_create for cpu core:%d failed",ix);

            /* Release previous pthread.*/
            for (jx = 0; jx < ix; jx++) {
                if (pthread_cancel(*(thread_core + jx)) < 0) {
                    /* Pthread cancel failed don't return but keep canceling rest pthreads. */
                    cterr('f',0, "Pthread for core:%d cancel failed.\n", jx);
                }
            }
            free(mem_test_array_ptr);
            free(flag);
            free(thread_core);
            return (FAILED);
        }
        msleep(CPU_MEM_TIME_50MS);
    }

    /* Set signal_flag with thread start, let each pthread start testing. */
    signal_flag = THREAD_START;

    /* Main process waits for pthread testing, timeout 5000ms.  */
    while (timeout < CPU_MEM_TEST_TIMEOUT) {
        
        pthread_mutex_lock(&thread_lock);
        /* Check if all threads finish.*/    
        if (thread_finish_flag == cpu_mem_test_finish) {
            pthread_mutex_unlock(&thread_lock);
            break; 
        }

        pthread_mutex_unlock(&thread_lock);
        timeout++;
        msleep(CPU_MEM_TIME_250MS);
        if ((NVRAM)->diagflag & D_VERBOSE) {
            printf("DBG: finish flag:0x%x , timeout_cnt:%d \n",thread_finish_flag, timeout);
        }
    }

    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("DBG: finish flag:0x%x , timeout_cnt:%d \n",thread_finish_flag, timeout);
        printf("DBG: main process send stop signal.\n");
    }

    /* Set signal_flag with thread stop and wait, let each pthread exit. */
    signal_flag = THREAD_STOP;
    msleep(CPU_MEM_TIME_250MS);

    pthread_mutex_lock(&thread_lock);

    /* Make sure all pthread cancel. */
    for (ix = 0; ix < expected_cpu_core_number; ix++) {
        if (!((thread_finish_flag >> ix) & PER_CPU_MEM_TEST_DONE)) {

            if ((NVRAM)->diagflag & D_VERBOSE) {
                printf("DBG: Pthread for core %d not finish, starting to cancel...\n",ix);
            }

            if (pthread_cancel(*(thread_core + ix)) < 0) {
                /* Pthread cancel failed don't return but keep canceling rest pthreads. */
                cterr('f',0, "Pthread for core:%d cancel failed.\n", ix);
                rc = FAILED;
            }
        }
    }

    pthread_mutex_unlock(&thread_lock);

    /* Check timeout flag. */
    if (timeout == CPU_MEM_TEST_TIMEOUT) {
        cterr('f',0, "CPU mem test failed: pthread test timeout failed");
        free(mem_test_array_ptr);
        free(flag);
        free(thread_core);
        return (FAILED);		
    }

    printf("\n");
	
    /* Read back data and compare with test pattern. */
    for (ix = 0; ix < expected_cpu_core_number; ix++) {
        test_start = ix * CPU_MEM_TEST_SIZE;
        test_end = test_start + CPU_MEM_TEST_SIZE;
        expect_pattern = test_pattern[ix];
        for (jx = test_start; jx < test_end; jx++) {
            if (*(mem_test_array_ptr + jx) != expect_pattern) {
                cterr('f',0, "CPU:%d mem test failed: failed at offset:0x%04X, expected data:0x%X, read data:0x%X.", ix, jx, expect_pattern, *(mem_test_array_ptr + jx));
                rc = FAILED;
            }
        }
        printf(" CPU:%d mem test passed.\n", ix);
    }
    printf("\n");

    free(mem_test_array_ptr);
    free(flag);
    free(thread_core);
    
    if(pthread_mutex_destroy(&thread_lock) != 0) {
        cterr('f',0, "Pthread mux destroy failed");
        return (FAILED);
    }

    if(rc != PASSED) {
        return (FAILED);
    }
    return (PASSED);

}

/*******************************************************************************
 * Function: cpu_core_number_test
 *
 * Description : Test for expected and actual CPU core number.
 * Default command:  NULL
 *              
 * Inputs: Number of expected CPU cores.
 *
 * Output: PASSED/FAILED
 ******************************************************************************* */
int cpu_core_number_test (int expected_cpu_core_number)
{
  
    FILE *fp = NULL;
    char line[CPU_FILE_LENGTH];
    int actual_cpu_core_number = 0 ;

    printf("\n");
    fp = fopen(CPU_PROC_INFO, "r");
    if (!fp) {
        cterr('f',0, "Proc file open failed.");
        return (FAILED);
    }

    while (fgets(line,CPU_FILE_LENGTH,fp) != NULL) {
        if (strstr(line,"processor") != NULL) {
            actual_cpu_core_number++ ;
        }
    }
    printf("\n");
    printf(" The expected number of CPU are : %d \n", expected_cpu_core_number);
    printf(" The actual   number of CPU are : %d \n", actual_cpu_core_number);
    fclose(fp);
    printf("\n");

    if (actual_cpu_core_number == expected_cpu_core_number) {
        return (PASSED);
    } else {
        return (FAILED);
    }

}
/*******************************************************************************
 * Function: linux_cpu_core_test
 *
 * Description : Linux CPU core test entry.
 * Default command:  NULL
 *              
 * Inputs: Number of expected CPU cores.
 *
 * Output: PASSED/FAILED
 ******************************************************************************* */
int linux_cpu_core_test (int expected_cpu_core_number)
{
    if ((expected_cpu_core_number > MAX_CPU_NUM) || (expected_cpu_core_number <= 0)) 
    {
        cterr('f',0, "Invalid expected CPU core number.");
        return (FAILED);
    }

    if (cpu_core_number_test(expected_cpu_core_number) != PASSED) {
        return (FAILED);
    }

    if (cpu_mem_test(expected_cpu_core_number) != PASSED) {
        return (FAILED);
    }
    return (PASSED);
}

/*-------------------------------------------------------------------
 *
 * Function : ExecuteCmdbyPopen
 * Description: weak function for ExecuteCmdbyPopen. 
 * INPUT:  dummy -- not used.
 * OUTPUT: return TRUE or FALSE
 * -------------------------------------------------------------------
 */
int ExecuteCmdbyPopen (char *tmp, char *tmp2, int tmp3)
    __attribute__((weak, alias("__ExecuteCmdbyPopen")));
int __ExecuteCmdbyPopen (char *tmp, char *tmp2, int tmp3)
{
    return (FALSE);
}

/*****************************************************************************
 *
 * Function   : linux_display_cpucore
 * Description: To show CPU core number by reading file "/proc/cpuinfo".
 * Inputs     : buf_size - cpu info buf size 
 * Outputs    : PASSED/FAILED
 *
 *****************************************************************************/
int linux_display_cpucore(char *file_name, int buf_size)
{
    char sys_cmd[256];
    char sys_cpucore[3];

    if ((ExecuteCmdbyPopen ("cat /proc/cpuinfo | grep processor | wc -l", 
        sys_cpucore, buf_size)) == 0) {
        cterr('f',0,"get CPU core failed!!\n");
        return (FAILED);
    }

    sprintf(sys_cmd, "echo \"CPU cores\t: %d\" >> %s", atoi(sys_cpucore), file_name);
    system(sys_cmd);

    return (PASSED);
}
/*-------------------------------------------------
$Log: linux_coretest.c,v $
Revision 1.5  2019/08/06 06:56:06  alpeng
merge curie, switzer and nightwatch to trunk

Revision 1.4  2019/07/11 12:34:40  alicehua
Collapse Nutella codes into main trunk

Revision 1.3.24.2  2019/07/09 11:28:53  alicehua
Removed Act2 in i2c scan test and FPGA function test, modified linux_coretest.h

Revision 1.3.24.1  2019/07/05 01:03:04  alicehua
Moved nutella_get_cpucore to common code as linux_get_cpucore.

Revision 1.3  2018/09/27 08:24:29  chieyang
Modify cpu core test to avoid main thread cancel exited pthread.

Revision 1.2  2018/08/30 07:04:05  haohsu
Collapse Vg400-branch to Main Trunk

Revision 1.1  2018/07/10 00:28:08  lucywang
Enhanced CPU core test to  make sure each CPU core is activated

$Endlog$
*/
