/* $Id: diag_blk_test.h,v 1.2 2016/04/20 11:25:27 benchen2 Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/tachi_bmc/diag_blk_test.h,v $
 *------------------------------------------------------------------
 *
 * diag_blk_test.h - Header file for diagnostic block test
 *
 * June 2015, Times Huang ported from O2
 *
 * Copyright (c) 2016 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */
 
#ifndef __DIAG_BLK_TEST__
#define __DIAG_BLK_TEST__

#define BUF_SIZE                (512)
#define OPEN_DEV_RETRY          (10)

#define WR_PATTERN              (0x6C)

#define BIOS_CS_DELAY           (500)

#define FLASH_ERASE_CMD         "/sbin/flash_eraseall"

#define DIAG_SPI_BLK_DEV        "/dev/mtd3"
#define DIAG_NAND_DEV           "/dev/mtd6"

extern int diag_access_device_test(char *);
extern int diag_spi_flash_test(void);
extern int diag_nand_flash_test(void);
extern int diag_fx3s_test(void);
extern int diag_bios_flash_test(void);

#endif /* __DIAG_BLK_TEST__ */

/*---------------------------------------------------------------
$Log: diag_blk_test.h,v $
Revision 1.2  2016/04/20 11:25:27  benchen2
add tachi fru portion

Revision 1.1.2.6  2015/12/16 07:55:12  tirawan
CSCux57032: Fix Intermittent BMC BIOS Flash test by putting 500 ms delay after setting the mux

Revision 1.1.2.5  2015/09/17 13:05:09  tirawan
Add Bios Boot Flash Test, fixes I2C controller number for NIM

Revision 1.1.2.4  2015/09/17 01:16:57  benchen2
Add nand flash test

Revision 1.1.2.3  2015/08/16 06:01:01  tirawan
Tachi bring up fix: SPI Flash Test, I2C Library for RTC Test, I2C scan Test, CPU ID fix for PECI test

Revision 1.1.2.2  2015/08/04 08:25:29  meho
Changed SPI block device to mtd6.

Revision 1.1.2.1  2015/06/11 02:01:06  tirawan
Add files for Tachi BMC project


$Endlog$
*/

