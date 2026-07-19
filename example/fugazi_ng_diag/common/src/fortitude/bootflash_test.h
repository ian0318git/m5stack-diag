/* $Id: bootflash_test.h,v 1.1 2014/08/18 06:38:48 iachang Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/fortitude/bootflash_test.h,v $
 *------------------------------------------------------------------
 *
 * Filename:  fortitude.h
 *
 * June 2014 - Ian Chang porting the code
 *
 * Copyright (c) 2014-2015 by Cisco Systems, Inc.
 * All rights reserved.
 *
 * Original Author: Bao Buu
 */

#ifndef __BOOTFLASH_TEST__H__
#define __BOOTFLASH_TEST__H__

#define BOOTFLASH_MTD_TEST "/dev/mtd0"
#define BOOTFLASH_MTD_TEST1 "/dev/mtd1"

/* test start address (mtd1-bootflash r/w test) */
#define BOOTFLASH_TEST_SECTOR_OFFSET 0x0

/* wanna test(erase) size*/
#define BOOTFLASH_TEST_LENGTH 0x20000 /* 128KB */

#define PROGRAM_NAME "mtd_debug"
#define ADRSPC_MAINBOARD_FLASH_SECTOR_SIZE   0x10000   /* 64KB */

extern ushort* bootflash_mmap_get_addr(void);
extern int bootflash_erase_sector(int, u_int32_t, u_int32_t);
extern int bootflash_write_sector(int, u_int32_t, u_int32_t, u_int8_t *);
extern int bootflash_read_sector(int,  u_int32_t, u_int32_t, u_int8_t *);
void bootflash_free_memory(u_int8_t * ,u_int8_t * ,uchar * ,int);

#define BOOTFLASH_BASE_ADDR      (0x1F000000)
#define BOOTFLASH_TEST_ADDR      (0x3e0000) 
#define BOOTFLASH_MMAP_LENGTH    (0xffff)
#define FLASH_INFO_CMD_ADDR1     (0x555)
#define FLASH_INFO_CMD_ADDR2     (0x2aa)
#define FLASH_INFO_CMD_ADDR3     (0x555)
#define FLASH_INFO_DATA1         (0xaa)
#define FLASH_INFO_DATA2         (0x55)
#define FLASH_INFO_DATA3         (0x90)

#define FLASH_ERASE_CMD_ADDR1    (0x555)
#define FLASH_ERASE_CMD_ADDR2    (0x2aa)
#define FLASH_ERASE_DATA1        (0xaa)
#define FLASH_ERASE_DATA2        (0x55)
#define FLASH_ERASE_DATA3        (0x80)
#define FLASH_ERASE_DATA4        (0x30)

#define FLASH_PROGRAM_CMD_ADDR1  (0x555)
#define FLASH_PROGRAM_CMD_ADDR2  (0x2aa)
#define FLASH_PROGRAM_CMD_ADDR3  (0x555)
#define FLASH_PROGRAM_DATA1      (0xaa)
#define FLASH_PROGRAM_DATA2      (0x55)
#define FLASH_PROGRAM_DATA3      (0xa0)

#define PPB_ENTRY_CMD1           (0x555) 
#define PPB_ENTRY_CMD2           (0x2aa)
#define PPB_ENTRY_CMD3           (0x555)

#define PPB_ENTRY_DATA1          (0xaa)
#define PPB_ENTRY_DATA2          (0x55)
#define PPB_ENTRY_DATA3          (0xc0)

#define PPB_EXIT_DATA1           (0x90)
#define PPB_EXIT_DATA2           (0x00)

#define SECTOR_SIZE              (0x20000)
#define TOTAL_OTP_SIZE           (0x400000)
#define OTP_TEST_LENGTH          (0x20000)
#endif

/******** History ********
 *------------------------------------------------------------------
 * $Log: bootflash_test.h,v $
 * Revision 1.1  2014/08/18 06:38:48  iachang
 *
 * Supported boot flash R/W utility.
 *
 *
 *------------------------------------------------------------------
 * $Endlog$
 */
