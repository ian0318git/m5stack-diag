/* $Id: linux_memory_tester.h,v 1.2 2016/05/06 17:44:08 huanngo Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/include/linux_memory_tester.h,v $ 
 *------------------------------------------------------------------
 * linux_memory_tester.h - Memory Tester (memtester) API
 *
 * July 2011, Alan Peng
 *
 * Copyright (c) 2011-2016 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#ifndef __LINUX_MEMORY_TESTER_H__
#define __LINUX_MEMORY_TESTER_H__


#include <limits.h>

#define rand32() ((unsigned int) rand() | ( (unsigned int) rand() << 16))

/* 
 * Test parameter for memory tester utils.
 * ULONG_MAX from limits.h
 */
#if (ULONG_MAX == 4294967295UL)
    #define rand_ul() rand32()
    #define UL_ONEBITS 0xffffffff
    #define UL_LEN 32
    #define CHECKERBOARD1 0x55555555
    #define CHECKERBOARD2 0xaaaaaaaa
    #define UL_BYTE(x) ((x | x << 8 | x << 16 | x << 24))
#elif (ULONG_MAX == 18446744073709551615ULL)
    #define rand64() (((ul) rand32()) << 32 | ((ul) rand32()))
    #define rand_ul() rand64()
    #define UL_ONEBITS 0xffffffffffffffffUL
    #define UL_LEN 64
    #define CHECKERBOARD1 0x5555555555555555
    #define CHECKERBOARD2 0xaaaaaaaaaaaaaaaa
    #define UL_BYTE(x) (((ul)x | (ul)x<<8 | (ul)x<<16 | (ul)x<<24 | (ul)x<<32 | (ul)x<<40 | (ul)x<<48 | (ul)x<<56))
#else
    #error long on this platform is not 32 or 64 bits
#endif

/* For error code */
#define EXIT_FAIL_NONSTARTER    0x01
#define EXIT_FAIL_ADDRESSLINES  0x02
#define EXIT_FAIL_OTHERTEST     0x04
#define EXIT_FAIL_MARCH_C       0x08

typedef unsigned long ul;
typedef unsigned long long ull;
typedef unsigned long volatile ulv;
typedef unsigned char volatile u8v;
typedef unsigned short volatile u16v;

/* define size for memory test */
#define MEGASHIFT 20

/* define for select menu item */
#define SELECT_STUCK_ADDR 16
#define SELECT_MARCH_C   0

/* for MIN_TEST_TIME to divide test size*/
#define DIV_SIZE  2

struct test_method {
    char *name;
    int (*fp)();
};

union {
    unsigned char bytes[UL_LEN/8];
    ul val;
} mword8;

union {
    unsigned short u16s[UL_LEN/16];
    ul val;
} mword16;




/* Function prototypes */
extern float get_mem_overhead_factor(void);

#endif /* __PLATFORM_MEMTESTER_H__ */

/*
*--------------------------------------------------
$Log: linux_memory_tester.h,v $
Revision 1.2  2016/05/06 17:44:08  huanngo
Replace the OVRHD_FACTOR with a function to return the overhead
factor for memory test

Revision 1.1  2012/05/07 08:02:00  alpeng
move this file from /src to /include directory.

Revision 1.2  2012/03/28 00:38:14  mcharon
remove forward slash from second line

Revision 1.1.1.1  2012/03/23 23:02:05  ptong
Initial archive of ng_diag module


$Endlog$
*/
