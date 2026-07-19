/* $Id: diag_blk_test.c,v 1.2 2016/04/20 11:25:27 benchen2 Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/tachi_bmc/diag_blk_test.c,v $
 *------------------------------------------------------------------
 *
 * diag_blk_test.c - Diagnostic block test functions
 *
 * June 2015, Times Huang ported from O2
 *
 * Copyright (c) 2015-2016 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */
 
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include "types.h"
#include "proto.h"
#include "common.h"
#include "error.h"
#include "diag_blk_test.h"
#include "diag_power_lib.h"

int diag_access_device_test(char *);
int diag_spi_flash_test(void);
int diag_nand_flash_test(void);
int diag_fx3s_test(void);
int diag_bios_flash_test(void);

/*******************************************************************************
 *
 * Function   : diag_fx3s_test
 * Description: main test for block device
 * Inputs     : file path to block device
 * Outputs    : PASSED or FAILED.
 *
 *******************************************************************************
 */
int diag_fx3s_test (void)
{
    int retval;

    retval = diag_access_device_test(DIAG_SPI_BLK_DEV);

    return (retval);
}


/*******************************************************************************
 *
 * Function   : diag_bios_flash_test
 * Description: main test for BIOS Boot flash
 *              This test only probes BIOS flash without performing r/w test
 * Outputs    : PASSED or FAILED.
 *
 *******************************************************************************
 */
int diag_bios_flash_test (void)
{
    int retval = FAILED;

    /* Skip the test if x86 is powered on since it might affect BIOS */
    if (diag_intel_power_status() == INTEL_POWER_ON) {
        printf("x86 is powered on. Skip the test!\n");
        return (PASSED);
    }

    testname("BIOS Boot Flash");
    prpass(testpass, "Probe BIOS Flash Kernel Module ");

    /* Switch SPI Mux to BMC and insert kernel module which probes 
     * BIOS boot flash
     */
    system("echo 0 > /proc/nuova/gpio/bios_cs_sel");
    msleep(BIOS_CS_DELAY);
    retval = system("modprobe spi_map_host_driver.ko");
    
    if (WEXITSTATUS(retval)) {
        cterr('f', 0, "BIOS Probe Test fails");
        retval = FAILED;
    } else {
        retval = PASSED;
        system("rmmod spi_map_host_driver");
    }

    /* Switch SPI Mux back to Intel */
    system("echo 1 > /proc/nuova/gpio/bios_cs_sel");
    msleep(BIOS_CS_DELAY);

    return (retval);
}


/*******************************************************************************
 *
 * Function   : diag_nand_flash_test
 * Description: main test for block device
 * Inputs     : file path to block device
 * Outputs    : PASSED or FAILED.
 *
 *******************************************************************************
 */
int diag_nand_flash_test (void)
{
    int retval = PASSED;
    char cmd[64];

    testname("Nand Flash Test");
    prpass(testpass, "Nand Flash Test");

    sprintf(cmd, "nandtest -l 13107200 %s > nand_test.txt;", DIAG_NAND_DEV);
    system(cmd);                                        
   
    int ret = system("cat nand_test.txt | grep -qI pass");
    if (ret !=  PASSED) {
        retval = FAILED;
        cterr('f', 0, "nand flash test"); 
    } 
    
    if (ret == PASSED) {
        system("rm nand_test.txt");
    }
    return (retval);
}

/*******************************************************************************
 *
 * Function   : diag_spi_flash_test
 * Description: main test for block device
 * Inputs     : file path to block device
 * Outputs    : PASSED or FAILED.
 *
 *******************************************************************************
 */
int diag_spi_flash_test (void)
{
    int retval;

    retval = diag_access_device_test(DIAG_SPI_BLK_DEV);

    return (retval);
}

/*******************************************************************************
 *
 * Function   : diag_access_device_test
 * Description: main test for block device
 * Inputs     : file path to block device
 * Outputs    : PASSED or FAILED.
 *
 *******************************************************************************
 */
int diag_access_device_test (char *blk_dev)
{
    char buf_bk[BUF_SIZE], buf_wr[BUF_SIZE], buf_rd[BUF_SIZE];
    char *p1 = buf_wr;
    char *p2 = buf_rd;
    int devfd, num, ib;
    int ix, cnt = 0;
    int retval = PASSED;
    char cmd[32];

    memset(buf_bk, 0, sizeof(buf_bk));
    memset(buf_wr, 0, sizeof(buf_wr));
    memset(buf_rd, 0, sizeof(buf_rd));

    /* Need to perform erase before writing to flash */
    sprintf(cmd, "%s %s", FLASH_ERASE_CMD, blk_dev);
    system(cmd);

    for (ix = 0; ix < OPEN_DEV_RETRY; ix++) {
        devfd = open(blk_dev, O_RDWR);
        if (devfd < 0) {
            sleep(1);
            continue;
        } else {
            break;
        }
    }
    if (devfd < 0) {
        perror("device test: ");
        printf("Can not access device at %s. is slot vacant?", blk_dev);
        return (FAILED);
    }

    lseek(devfd, 0, SEEK_SET);
    if ((num = read(devfd, buf_bk, sizeof(buf_bk))) == -1) {
        close(devfd); /* don't need it anymore */
        perror("Read");
        printf("Unable to read from drive.\n");
        return (FAILED);
    }

    for (cnt=0; cnt < sizeof(buf_wr); cnt++) {
        buf_wr[cnt] = WR_PATTERN + cnt;
    }
    
    lseek(devfd, 0, SEEK_SET);
    if ((num = write(devfd, buf_wr, sizeof(buf_wr))) == -1) {
        perror("Write");
        printf("Write test pattern failed, can not write to drive.\n");
        return (FAILED);
    }
    lseek(devfd, 0, SEEK_SET);
    if ((num = read(devfd, buf_rd, sizeof(buf_rd))) == -1) {
        close(devfd); /* don't need it anymore */
        perror("Read");
        printf("Readback failed, can not read from drive.\n");
        return (FAILED);
    }

    for (ib =0, cnt=0; ib < sizeof(buf_rd); ib++, p1++, p2++) {
        if (*p1 != *p2) {
            printf("failed on byte %d, wrote = %02x, read back = %02x\n",(ib+1), *p1, *p2);
            if (cnt++ > 10) {
                printf("Too many data mismatches. Stop testing\n");
                break;
            }
            retval = FAILED;
        }
    }

    lseek(devfd, 0, SEEK_SET);
    if ((num = write(devfd, buf_bk, sizeof(buf_bk))) == -1) {
        close(devfd); /* don't need it anymore */
        perror("Write");
        printf("Unable to write original data to drive\n");
        return (FAILED);
    }

    close(devfd); /* don't need it anymore */

    return (retval);

}

/*---------------------------------------------------------------
$Log: diag_blk_test.c,v $
Revision 1.2  2016/04/20 11:25:27  benchen2
add tachi fru portion

Revision 1.1.2.8  2016/01/11 09:40:31  benchen2
remove nandflash test log when passed

Revision 1.1.2.7  2015/12/23 11:42:02  tirawan
Skip BIOS Flash test if x86 is powered on

Revision 1.1.2.6  2015/12/16 07:55:12  tirawan
CSCux57032: Fix Intermittent BMC BIOS Flash test by putting 500 ms delay after setting the mux

Revision 1.1.2.5  2015/09/18 04:54:56  benchen2
fix nand flash test compare err.

Revision 1.1.2.4  2015/09/17 13:05:09  tirawan
Add Bios Boot Flash Test, fixes I2C controller number for NIM

Revision 1.1.2.3  2015/09/17 01:16:57  benchen2
Add nand flash test

Revision 1.1.2.2  2015/08/16 06:01:00  tirawan
Tachi bring up fix: SPI Flash Test, I2C Library for RTC Test, I2C scan Test, CPU ID fix for PECI test

Revision 1.1.2.1  2015/06/11 02:01:06  tirawan
Add files for Tachi BMC project


$Endlog$
*/
