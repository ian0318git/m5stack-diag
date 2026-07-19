/* $Id: linux_coretest.h,v 1.2 2019/07/11 12:34:40 alicehua Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/include/linux_coretest.h,v $
 *------------------------------------------------------------------
 *
 * Copyright (c) 2018-2019 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */
#ifndef __LINUX_CORETEST__
#define __LINUX_CORETEST__

#define __USE_GNU
#include <sched.h>
#include <pthread.h>
#include <sys/syscall.h>

#define CPU_PROC_INFO   "/proc/cpuinfo"
#define CPU_FILE_LENGTH 1024
#define ENHANCE_ERROR_MSG_RDY 1
#define CPU_MEM_TEST_SIZE 1024
#define PER_CPU_MEM_TEST_DONE 0x1 
#define CPU_MEM_TEST_TIMEOUT 20 // 20 * 250ms = 5000ms 
#define CPU_MEM_TIME_250MS 250
#define CPU_MEM_TIME_50MS 50 
#define POLLING_INTRVL 100 
#define MAX_POLLING_COUNTS 100
#define MAX_CPU_NUM 32

// Pattern 0-15
#define TEST_PATTERN_A0 0xA0
#define TEST_PATTERN_A1 0xA1
#define TEST_PATTERN_A2 0xA2
#define TEST_PATTERN_A3 0xA3
#define TEST_PATTERN_A4 0xA4
#define TEST_PATTERN_A5 0xA5
#define TEST_PATTERN_A6 0xA6
#define TEST_PATTERN_A7 0xA7
#define TEST_PATTERN_A8 0xA8
#define TEST_PATTERN_A9 0xA9
#define TEST_PATTERN_AA 0xAA
#define TEST_PATTERN_AB 0xAB
#define TEST_PATTERN_AC 0xAC
#define TEST_PATTERN_AD 0xAD
#define TEST_PATTERN_AE 0xAE
#define TEST_PATTERN_AF 0xAF
// Pattern 16-31
#define TEST_PATTERN_B0 0xB0
#define TEST_PATTERN_B1 0xB1
#define TEST_PATTERN_B2 0xB2
#define TEST_PATTERN_B3 0xB3
#define TEST_PATTERN_B4 0xB4
#define TEST_PATTERN_B5 0xB5
#define TEST_PATTERN_B6 0xB6
#define TEST_PATTERN_B7 0xB7
#define TEST_PATTERN_B8 0xB8
#define TEST_PATTERN_B9 0xB9
#define TEST_PATTERN_BA 0xBA
#define TEST_PATTERN_BB 0xBB
#define TEST_PATTERN_BC 0xBC
#define TEST_PATTERN_BD 0xBD
#define TEST_PATTERN_BE 0xBE
#define TEST_PATTERN_BF 0xBF

enum thread_signal{
    THREAD_STOP = 0,
    THREAD_START,
};

extern int linux_cpu_core_test(int);
extern int linux_display_cpucore(char *, int);
extern int ExecuteCmdbyPopen(char *, char *, int);

#endif 
/*-------------------------------------------------
$Log: linux_coretest.h,v $
Revision 1.2  2019/07/11 12:34:40  alicehua
Collapse Nutella codes into main trunk

Revision 1.1.58.1  2019/07/05 01:03:04  alicehua
Moved nutella_get_cpucore to common code as linux_get_cpucore.

Revision 1.1  2018/07/10 00:28:08  lucywang
Enhanced CPU core test to  make sure each CPU core is activated

$Endlog$
*/
