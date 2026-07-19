/* $Id: linux_memory_tester_utils.h,v 1.1 2012/05/07 08:02:00 alpeng Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/include/linux_memory_tester_utils.h,v $
 *------------------------------------------------------------------
 * linux_memory_tester_utils.h - Memory Tester (memtester) API
 *
 * July 2011, Alan Peng
 *
 * Copyright (c) 2011-2012 by Cisco Systems, Inc.
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
/*
*--------------------------------------------------
$Log: linux_memory_tester_utils.h,v $
Revision 1.1  2012/05/07 08:02:00  alpeng
move this file from /src to /include directory.

Revision 1.2  2012/03/28 00:38:14  mcharon
remove forward slash from second line

Revision 1.1.1.1  2012/03/23 23:02:05  ptong
Initial archive of ng_diag module


$Endlog$
*/
