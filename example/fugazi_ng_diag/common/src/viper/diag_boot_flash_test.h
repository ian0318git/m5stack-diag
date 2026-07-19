 /* $Id: diag_boot_flash_test.h,v 1.2 2018/08/06 02:31:50 harrchan Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/viper/diag_boot_flash_test.h,v $
 *------------------------------------------------------------------
 * Filename: diag_boot_flash_test.h
 *
 * Description: Diagnostic boot flash header file.
 *
 * Copyright (c) 2011-2018 by cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#ifndef __DIAG_BOOT_FLASH_H__
#define __DIAG_BOOT_FLASH_H__

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
Revision 1.2  2018/08/06 02:31:50  harrchan
Merge viper E2E to the main trunk (CSCvk28469)

Revision 1.1.2.6  2018/07/03 05:38:55  harrchan
Follow the coding rule to clean up code

Revision 1.1.2.5  2018/06/27 06:27:48  harrchan
Follow the coding rule to clean up code

Revision 1.1.2.4  2018/04/20 13:17:45  harrchan
Modify FPGA register according register map

Revision 1.1.2.3  2018/04/20 10:07:26  harrchan
Modify FPGA register according register map

Revision 1.1.2.2  2018/03/29 04:17:22  lucywang
Added test message for Boot Flash Test

Revision 1.1.2.1  2018/02/27 08:06:32  harrchan
Initial viper application code base



$Endlog$
*/
