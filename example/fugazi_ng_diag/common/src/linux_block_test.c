/* $Id: linux_block_test.c,v 1.3 2020/03/13 06:15:08 alicehua Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/linux_block_test.c,v $
 *------------------------------------------------------------------
 *
 * linux_block_test.c
 *
 * May 2019
 *
 * Copyright (c) 2019 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */
 
#include <string.h>
#include <errno.h>
#include <libgen.h>
#include <pthread.h>
#include <semaphore.h>
#include <time.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/select.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/ioctl.h> 
#include <malloc.h>
#include "types.h"
#include "common.h"
#include "error.h"
#include "proto.h"
#include "nvmonvars.h"
#include "linux_block_test.h"

int linux_block_test(char *, int, int, int, int);

/*******************************************************************************
 *
 * Function   : linux_block_test 	
 * Description: This function performs block test on random/fixed sectors
 * Inputs     : *dev_name - file path to block device
 *              start_sector - The offset of sector, invalid if test type is random
 *              test_size - test read/write size
 *              test_type - Random or Sequential
 *              fsync_required - TRUE for USB/HD test, FALSE for SPI Flash
 * Outputs    : PASSED or FAILED.
 *
 *******************************************************************************
 */
int linux_block_test (char *dev_name, int start_sector, int test_size, 
                      int test_type, int fsync_required)
{
    int cnt = 0;
    int rc = PASSED;
    int devfd, num, ib;
    char *p1, *p2, *p3, *p4;
    off_t dev_size, test_offset;
    char *buf_bk, *buf_wr, *buf_rd, *buf_bkrd;
  
    devfd = open(dev_name, O_RDWR);
    /* Open failed */
    if (devfd < 0) {
        cterr('f', 0, "Can't open '%s'", dev_name);
        return (FAILED);
    }

    /* Allocate memory */
    buf_bk = (char *) malloc(test_size);
    buf_wr = (char *) malloc(test_size);
    buf_rd = (char *) malloc(test_size);
    buf_bkrd = (char *) malloc(test_size);

    if (buf_bk == NULL || buf_wr == NULL || buf_rd == NULL || buf_bkrd == NULL) {
        cterr('f', 0, "%s: Null pointer", __func__);
        return (FAILED);
    }

    /* Assign buffer to compare buffer pointer */
    p1 = buf_wr;
    p2 = buf_rd;
    p3 = buf_bk;
    p4 = buf_bkrd;

    /* Initialize buffer */
    memset(buf_bk, 0, test_size);
    memset(buf_wr, 0, test_size);
    memset(buf_rd, 0, test_size);
    memset(buf_bkrd, 0, test_size);

    dev_size = lseek(devfd, 0, SEEK_END);

    /* Assign test offset with random number if test_type is random */
    if (test_type == BLOCK_TEST_RANDOM) {
        srand(time(NULL));
        test_offset = (rand() % dev_size) - test_size;
    } else {
        test_offset = start_sector;
    }
    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("\ndev_name = %s, dev_size = %lx, test_size = %#x, test_offset ="
                " %#lx\n", dev_name, dev_size, test_size, test_offset);
    }

    /* Back up data */
    if (lseek(devfd, test_offset, SEEK_SET) <  0) {
        perror("lseek to the beginning of device failed.");
        cterr('f', 0, "backup lseek failed; Cannot point to the beginning of device.\n");
        rc = FAILED;
        goto exit;
    }

    /* Read out data for backup */
    if ((num = read(devfd, buf_bk, test_size)) == -1) {
        perror("Read data from device failed");
        cterr('f', 0, "Unable to read from drive.\n");
        rc = FAILED;
        goto exit;
    }

    /* Prepare data pattern */
    for (cnt = 0; cnt < test_size; cnt++) {
        buf_wr[cnt]= PATTERN + cnt;
    }

    /* Set write offset pointer */
    if (lseek(devfd, test_offset, SEEK_SET) < 0) {
        perror("lseek to the beginning of device failed.");
        cterr('f', 0, "write lseek failed; Cannot point to the beginning of device.\n");
        rc = FAILED;
        goto exit;
    }

    /* Write data pattern */
    if ((num = write(devfd, buf_wr, test_size)) < 0) {
        perror("Write test pattern failed, can not write to drive.\n");
        cterr('f', 0, "Unable to write data pattern to device.\n");
        rc = FAILED;
        goto exit;
    }

    if (num != test_size) {
        perror("not all the bytes are written for data pattern");
        rc = FAILED;
        goto exit;
    }

    if (fsync_required == TRUE) {
        if (fsync(devfd)<0) {
            perror("fsync failed.");
            cterr('f', 0, "Unable to sync data pattern to device.\n");
            rc = FAILED;
            goto exit;
        }
    }

    /* Set read out offset pointer */
    if (lseek(devfd, test_offset, SEEK_SET) < 0) {
        perror("lseek to the beginning of device failed.");
        cterr('f', 0, "lseek failed; Cannot point to the beginning of device.\n");
        rc = FAILED;
        goto exit;
    }

    /* Read back data for comparing */
    if ((num = read(devfd, buf_rd, test_size)) == -1) {
        close(devfd); /* don't need it anymore */
        perror("Read back data from device failed");
        cterr('f', 0, "Unable to read from drive.\n");
        rc = FAILED;
        goto exit;
    }

    if (num != test_size) {
        perror("not all the bytes are read for data pattern");
        rc = FAILED;
        goto exit;
    }
    
    /* Comparing data */
    for (ib = 0; ib < test_size; ib++, p1++, p2++) {
        if (*p1 != *p2) {
            cterr('f', 0, "failed on byte %d, wrote = %02x, read back = %02x\n", (ib+1), *p1, *p2);
            if (cnt++ > 10) {
                cterr('f', 0, "Too many data mismatches. Stop testing\n");
            }
            rc = FAILED;
            break;
        }
    }

    /* Set restore data offset pointer */
    if (lseek(devfd, test_offset, SEEK_SET) < 0) {
        perror("lseek to the beginning of device failed.");
        cterr('f', 0, "restore lseek failed; Cannot point to the beginning of device.\n");
        rc = FAILED;
        goto exit;
    }
    
    /* Restore data */
    if ((num = write(devfd, buf_bk, test_size)) < 0) {
        close(devfd); /* don't need it anymore */
        perror("Write restore data failed, can not write to drive.\n");
        cterr('f', 0, "Unable to write restore data to device.\n");
        rc = FAILED;
        goto exit;
    }

    if (num != test_size) {
        perror("not all the bytes are written for restore");
        rc = FAILED;
        goto exit;
    }

    if (fsync_required == TRUE) {
        if (fsync(devfd)<0) {
            perror("fsync failed.");
            cterr('f', 0, "Unable to sync data pattern to device.\n");
            rc = FAILED;
            goto exit;
        }
    }

    /* Set backup data offset pointer */
    if (lseek(devfd, test_offset, SEEK_SET) < 0) {
        perror("lseek to the beginning of device failed.");
        cterr('f', 0, "lseek failed; Cannot point to the beginning of device.\n");
        rc = FAILED;
        goto exit;
    }

    /* Read back restore data for comparing */
    if ((num = read(devfd, buf_bkrd, test_size)) == -1) {
        close(devfd); /* don't need it anymore */
        perror("Read back data from device failed");
        cterr('f', 0, "Unable to read from drive.\n");
        rc = FAILED;
        goto exit;
    }

    if (num != test_size) {
        perror("not all the bytes are read for data pattern");
        rc = FAILED;
        goto exit;
    }
    
    /* comparing data */
    for (ib = 0; ib < test_size; ib++, p3++, p4++) {
        if (*p3 != *p4) {
            cterr('f', 0, "failed on byte %d, wrote = %02x, read back = %02x\n",(ib+1), *p3, *p4);
            if (cnt++ > 10) {
                cterr('f', 0, "Too many data mismatches. Stop testing\n");
            }
            rc = FAILED;
            break;
        }
    }

    goto exit;

exit:
    free(buf_rd);
    free(buf_bkrd);
    free(buf_wr);
    free(buf_bk);
    close(devfd); /* don't need it anymore */

    return (rc);
}

/*---------------------------------------------------------------
$Log: linux_block_test.c,v $
Revision 1.3  2020/03/13 06:15:08  alicehua
CSCvt38566: Add return failed in linux block test.

Revision 1.2  2019/07/11 12:34:40  alicehua
Collapse Nutella codes into main trunk

$Endlog$
*/
