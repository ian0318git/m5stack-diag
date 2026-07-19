/* $Id: diag_boot_flash_test.h,v 1.2 2021/04/15 00:52:23 achiu2 Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/phoenix/diag_boot_flash_test.h,v $
 *------------------------------------------------------------------
 * Filename: diag_boot_flash_test.h
 *
 * Description: Diagnostic boot flash header file.
 *
 * Copyright (c) 2011-2019 by cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#ifndef __DIAG_BOOT_FLASH_H__
#define __DIAG_BOOT_FLASH_H__

#define FIRST_BOOTFLASH             0  
#define SECONDARY_BOOTFLASH         1
#define DEBUG_INDEX                 256 
#define ENABLE_FIRST_BOOTFLASH      0x80000000
#define ENABLE_SECONDARY_BOOTFLASH  0xC0000000
#define BOOT_FLASH_DEV_PATH         "/dev/mtdblock0"
#define BIT(x)                      (1 << (x))

extern int bootflash_tests(void);

#endif                          /* __DIAG_BOOT_TEST_H__ */
