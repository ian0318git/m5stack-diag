/* $Id: diag_storage_lib.h,v 1.2 2021/04/15 00:52:26 achiu2 Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/phoenix/diag_storage_lib.h,v $
 *------------------------------------------------------------------
 * Filename: diag_storage_lib.h
 *
 * Description: Diag storage library header file.
 *
 * Copyright (c) 2011-2019 by cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#ifndef __DIAG_STORGE_LIB_H__
#define __DIAG_STORGE_LIB_H__

extern int access_device_test(char *);
extern int access_device_random_offset_test(char *, int);

#define DEV_OPEN_RETRY 10

/* For bootflash test */
#define BOOTFLASH_TEST_LEN       0x10000
#define SECTOR_OFFSET            0x7E0000

#endif                          /* __DIAG_STORGE_LIB_H__ */


