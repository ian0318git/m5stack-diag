/* $Id: diag_bootflash_lib.h,v 1.1 2015/02/26 07:18:29 xiaoyizh Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/wallander/diag_bootflash_lib.h,v $
 *-----------------------------------------------------------------------------
 * diag_bootflash_lib.h
 *
 * Xiaoying Zhang -- Feb. 2014
 *
 * Copyright (c) 2015 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */
#include <stdio.h>
#include <stdlib.h>

#ifndef __DIAG_BOOTFLASH_LIB_H__
#define __DIAG_BOOTFLASH_LIB_H__

#define BOOTFLASH_MTD_TEST "/dev/mtd0"

/* test start address (mtd1-bootflash r/w test) */
// #define BOOTFLASH_TEST_SECTOR_OFFSET 0x0
#define BOOTFLASH_TEST_SECTOR_OFFSET 0x800000

/* wanna test(erase) size*/
#define BOOTFLASH_TEST_LENGTH 0x20000 /* 128KB */

#define PROGRAM_NAME "mtd_debug"
#define ADRSPC_MAINBOARD_FLASH_SECTOR_SIZE   0x10000   /* 64KB */

extern void bootflash_init_virt_addr(void);
extern uchar * bootflash_mmap_get_addr(int);
extern int bootflash_erase_sector(int, u_int32_t, u_int32_t);
extern int bootflash_write_sector(int, u_int32_t, u_int32_t, u_int8_t *);
extern int bootflash_read_sector(int,  u_int32_t, u_int32_t, u_int8_t *);
void bootflash_free_memory(u_int8_t * ,u_int8_t * ,uchar * ,int);

#define BOOTFLASH_BASE_ADDR      (0x1ec00000)
#define BOOTFLASH_TEST_ADDR      (0x3e0000) 
// #define BOOTFLASH_MMAP_LENGTH    (0xffff)
#define BOOTFLASH_MMAP_LENGTH    (0x20000)

/* Byte address */
#define FLASH_CMD_UNLOCK_ADDR1      (0xaaa)
#define FLASH_CMD_UNLOCK_ADDR2      (0x555)

#define FLASH_INFO_CMD_ADDR1     (0x555)
#define FLASH_INFO_CMD_ADDR2     (0x2aa)
#define FLASH_INFO_CMD_ADDR3     (0x555)
/*#define FLASH_INFO_CMD_ADDR1_b   (0xaaa)
#define FLASH_INFO_CMD_ADDR2_b   (0x555)
#define FLASH_INFO_CMD_ADDR3_b   (0xaaa)*/
#define FLASH_INFO_DATA1         (0xaa)
#define FLASH_INFO_DATA2         (0x55)
#define FLASH_INFO_DATA3         (0x90)

#define FLASH_ERASE_CMD_ADDR1    (0x555)
#define FLASH_ERASE_CMD_ADDR2    (0x2aa)
#define FLASH_ERASE_CMD_ADDR1_b  (0xaaa)
#define FLASH_ERASE_CMD_ADDR2_b  (0x555)
#define FLASH_ERASE_DATA1        (0xaa)
#define FLASH_ERASE_DATA2        (0x55)
#define FLASH_ERASE_DATA3        (0x80)
#define FLASH_ERASE_DATA4        (0x30)

#define FLASH_PROGRAM_CMD_ADDR1  (0x555)
#define FLASH_PROGRAM_CMD_ADDR2  (0x2aa)
#define FLASH_PROGRAM_CMD_ADDR3  (0x555)
#define FLASH_PROGRAM_CMD_ADDR1_b  (0xaaa)
#define FLASH_PROGRAM_CMD_ADDR2_b  (0x555)
#define FLASH_PROGRAM_CMD_ADDR3_b  (0xaaa)
#define FLASH_PROGRAM_DATA1      (0xaa)
#define FLASH_PROGRAM_DATA2      (0x55)
#define FLASH_PROGRAM_DATA3      (0xa0)

#define FLASH_PPB_ENTRY_CMD1           (0x555) 
#define FLASH_PPB_ENTRY_CMD2           (0x2aa)
#define FLASH_PPB_ENTRY_CMD3           (0x555)

#define FLASH_PPB_ENTRY_DATA1          (0xaa)
#define FLASH_PPB_ENTRY_DATA2          (0x55)
#define FLASH_PPB_ENTRY_DATA3          (0xc0)

#define FLASH_PPB_PROGRAM_DATA1        (0xa0)
#define FLASH_PPB_PROGRAM_LOCK         (0x0)
#define FLASH_PPB_ALL_ERASE_DATA1      (0x80)
#define FLASH_PPB_ALL_ERASE_DATA2      (0x30)

#define FLASH_PPB_EXIT_DATA1           (0x90)
#define FLASH_PPB_EXIT_DATA2           (0x00)

#define SECTOR_NUM               (0x80)
#define SECTOR_GOLDEN            (0x20)
#define SECTOR_TEST_INDEX        (0x40)
#define SECTOR_SIZE              (0x20000)
#define TOTAL_OTP_SIZE           (0x400000)
#define TOTAL_OTP_LOCKED_SIZE    (0x100000)
#define OTP_TEST_LENGTH          (0x20000)


/* polling return status */
typedef enum {
    DEV_STATUS_UNKNOWN = 0,
    DEV_NOT_BUSY,
    DEV_BUSY,
    DEV_EXCEEDED_TIME_LIMITS,
    DEV_SUSPEND,
    DEV_WRITE_BUFFER_ABORT,
    DEV_STATUS_GET_PROBLEM,
    DEV_VERIFY_ERROR,
    DEV_BYTES_PER_OP_WRONG,
    DEV_ERASE_ERROR,
    DEV_PROGRAM_ERROR,
    DEV_SECTOR_LOCK,
    DEV_PROGRAM_SUSPEND,           /* Device is in program suspend mode */
    DEV_PROGRAM_SUSPEND_ERROR,     /* Device program suspend error */
    DEV_ERASE_SUSPEND,             /* Device is in erase suspend mode */
    DEV_ERASE_SUSPEND_ERROR,       /* Device erase suspend error */
    DEV_BUSY_IN_OTHER_BANK         /* Busy operation in other bank */
} DEVSTATUS;

#define DQ1_MASK   (0x02)
#define DQ2_MASK   (0x04)
#define DQ3_MASK   (0x08)
#define DQ5_MASK   (0x20)
#define DQ6_MASK   (0x40)
#define DQ6_TGL_RDY_SHFT (dq6_toggles << 1) /* Shift for DQ6 to ready bit position in status register */
#define DQ6_TGL_DQ1_MASK (dq6_toggles >> 5) /* Mask for DQ1 when device DQ6 toggling */
#define DQ6_TGL_DQ5_MASK (dq6_toggles >> 1) /* Mask for DQ5 when device DQ6 toggling */
#endif

/*-------------------------------------------------
 * $Log: diag_bootflash_lib.h,v $
 * Revision 1.1  2015/02/26 07:18:29  xiaoyizh
 * Initial check in for Wallander.
 *
 *
 * $Endlog$
 *-------------------------------------------------
 */
