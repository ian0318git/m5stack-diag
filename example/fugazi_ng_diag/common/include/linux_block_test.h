/* $Id: linux_block_test.h,v 1.2 2019/07/11 12:34:40 alicehua Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/include/linux_block_test.h,v $
 *------------------------------------------------------------------
 *
 * linux_block_test.h - Header file for Linux Block Test
 *
 * May 2019
 *
 * Copyright (c) 2015-2019 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */
 
#ifndef __LINUX_BLOCK_TEST_H__
#define __LINUX_BLOCK_TEST_H__

#define BLOCK_SIZE_128B             (0x80)
#define BLOCK_SIZE_256B             (0x100)
#define BLOCK_SIZE_512B             (0x200)
#define BLOCK_SIZE_1K               (0x400)
#define BLOCK_SIZE_1M               (BLOCK_SIZE_1K * BLOCK_SIZE_1K)

typedef enum {
    BLOCK_TEST_RANDOM,
    BLOCK_TEST_SEQUENTIAL
} block_test_type_t;

extern int linux_block_test(char *, int, int, int, int);

#endif /* __LINUX_BLOCK_TEST_H__ */

/*---------------------------------------------------------------
$Log: linux_block_test.h,v $
Revision 1.2  2019/07/11 12:34:40  alicehua
Collapse Nutella codes into main trunk

$Endlog$
*/
