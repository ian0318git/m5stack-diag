/* $Id: diag_bootflash_lib.h,v 1.2 2013/10/08 08:48:27 tirawan Exp $ 
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/woodlawn_linux/diag_bootflash_lib.h,v $ 
 *------------------------------------------------------------------
 * diag_bootflash_lib.h
 * 
 * February 2012, Leslie Chen
 * Copyright (c) 2013 by Cisco Systems, Inc.
 * All rights reserved.
 *------------------------------------------------------------------
 */

#include <stdio.h>
#include <stdlib.h>

#ifndef __DIAG_BOOTFLASH_LIB_H__
#define __DIAG_BOOTFLASH_LIB_H__

#define BOOTFLASH_MTD_TEST "/dev/mtd1"

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

#define BOOTFLASH_BASE_ADDR      (0x1ec00000)
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
/*-------------------------------------------------
 * $Log: diag_bootflash_lib.h,v $
 * Revision 1.2  2013/10/08 08:48:27  tirawan
 * Woodlawn collapsed to main trunk
 *
 * Revision 1.1.4.3  2013/09/05 13:27:13  leschen
 * Modify OTP test length.
 *
 * Revision 1.1.4.2  2013/08/20 10:58:50  tirawan
 * Branch into woodlawn-branch2 and port woodlawn code
 *
 * Revision 1.1.2.2  2013/07/02 07:18:11  leschen
 * Add OTP macros define
 *
 * Revision 1.1.2.1  2013/04/24 10:37:13  tirawan
 * Initial check-in for woodlawn linux code
 *
 * Revision 1.1  2013/03/13 06:42:49  kuangik
 * Add for the first time
 *
 * Revision 1.6  2013/03/06 03:02:15  kuangik
 * Update boot flash test for P1A board (128K/sectors)
 *
 * Revision 1.5  2012/08/03 10:16:55  evanli
 * Mapping to latest O2 source code on 20120726
 *
 * Revision 1.3  2012/07/19 06:14:59  leslie
 * Add macro BOOTFLASH_MTD_TEST
 *
 * Revision 1.2  2012/05/30 01:14:14  leslie
 * Change the bootflash test sector offset from 0x0
 *
 * Revision 1.1  2012/05/18 03:19:05  leslie
 * Add Woodlawn bootflash lib header file
 *
 * $Endlog$
 *-------------------------------------------------
 */
