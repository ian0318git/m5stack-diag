/* $Id: linux_memory_tester_utils.h,v 1.2 2015/05/25 03:59:10 steja Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/skye_linux/include/linux_memory_tester_utils.h,v $
 *------------------------------------------------------------------
 * linux_memory_tester_utils.h - Memory Tester (memtester) API
 *
 * April 29, 2013 - iachang ported from Overlord.
 *
 * Copyright (c) 2013 ~ 2015 by Cisco Systems, Inc.
 * All rights reserved.
 *------------------------------------------------------------------
 */

#ifndef __LINUX_MEMORY_TESTER_UTILS_H__
#define __LINUX_MEMORY_TESTER_UTILS_H__

/* Parameter define */
#define PROGRESSLEN 4
#define PROGRESSOFTEN 2500
#define ONE 0x00000001L


/* Function prototypes */
/* test(ulv *bufa, ulv *bufb, size_t count) */

extern int test_stuck_address(unsigned long volatile *, size_t );
extern int test_random_value(unsigned long volatile *, unsigned long volatile *, size_t );
extern int test_xor_comparison(unsigned long volatile *, unsigned long volatile *, size_t );
extern int test_sub_comparison(unsigned long volatile *, unsigned long volatile *, size_t );
extern int test_mul_comparison(unsigned long volatile *, unsigned long volatile *, size_t );
extern int test_div_comparison(unsigned long volatile *, unsigned long volatile *, size_t );
extern int test_or_comparison(unsigned long volatile *, unsigned long volatile *, size_t );
extern int test_and_comparison(unsigned long volatile *, unsigned long volatile *, size_t );
extern int test_seqinc_comparison(unsigned long volatile *, unsigned long volatile *, size_t );
extern int test_solidbits_comparison(unsigned long volatile *, unsigned long volatile *, size_t );
extern int test_checkerboard_comparison(unsigned long volatile *, unsigned long volatile *, size_t );
extern int test_blockseq_comparison(unsigned long volatile *, unsigned long volatile *, size_t );
extern int test_walkbits0_comparison(unsigned long volatile *, unsigned long volatile *, size_t );
extern int test_walkbits1_comparison(unsigned long volatile *, unsigned long volatile *, size_t );
extern int test_bitspread_comparison(unsigned long volatile *, unsigned long volatile *, size_t );
extern int test_bitflip_comparison(unsigned long volatile *, unsigned long volatile *, size_t );
#ifdef TEST_NARROW_WRITES    
extern int test_8bit_wide_random(unsigned long volatile *, unsigned long volatile *, size_t );
extern int test_16bit_wide_random(unsigned long volatile *, unsigned long volatile *, size_t );
#endif

#endif /* __LINUX_MEMORY_TESTER_UTILS_H__ */

/******** History ********/
/*
 * $Log: linux_memory_tester_utils.h,v $
 * Revision 1.2  2015/05/25 03:59:10  steja
 * Add Support Skye SM
 *
 * Revision 1.1.4.2  2015/04/29 11:36:26  steja
 * Code check-in to skye-branch2 for ER code review
 *
 *
 * ------------------------------------------------------------------
 * Revision 1.1.2.1  2014/07/21 01:56:38  palin2
 * Initial check-in Skye module side Diag code.
 *
 * $Endlog$
 */

