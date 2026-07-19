 /* $Id: diag_boot_flash_test.h,v 1.2 2019/12/11 10:10:27 lucywang Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/nanook/diag_boot_flash_test.h,v $
 *------------------------------------------------------------------
 * Filename: diag_boot_flash_test.h
 *
 * Description: Diagnostic boot flash header file.
 *
 * Copyright (c) 2019 by cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#ifndef __DIAG_BOOT_FLASH_H__
#define __DIAG_BOOT_FLASH_H__

#define FPGA_LPC_SPI_CTRL_REG       0x58
#define FIRST_BOOTFLASH     0  
#define SECONDARY_BOOTFLASH 1
#define DEBUG_INDEX         256 
#define ENABLE_FIRST_BOOTFLASH      0x80000000
#define ENABLE_SECONDARY_BOOTFLASH  0xC0000000
#define BOOT_FLASH_DEV_PATH "/dev/mtdblock0"
#define BIT(x)  (1 << (x))

extern int mtd_bootflash_test(char *);
extern int bootflash_tests(void);

#endif                          /* __DIAG_BOOT_TEST_H__ */
/******** History ********
$Log: diag_boot_flash_test.h,v $
Revision 1.2  2019/12/11 10:10:27  lucywang
Merged Nanook to main trunk


$Endlog$
*/
