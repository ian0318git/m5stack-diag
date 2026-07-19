/* $Id: diag_usb_api.c,v 1.2 2021/06/02 08:22:34 iachang Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/fugazi/diag_usb_api.c,v $
 *------------------------------------------------------------------
 * 
 * Filename: diag_usb_api.c
 *           This file port from src/linux_usb_test.c
 *
 * Copyright (c) 2020 by cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#include <errno.h>
#include <dirent.h>
#include <libgen.h>
#include <pthread.h>
#include <semaphore.h>
#include <time.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/select.h>
#include <stdlib.h>
#include <unistd.h>
#include "types.h"
#include "common.h"
#include "monitor.h"
#include "menu.h"
#include "error.h"
#include "proto.h"
#include "linux_usb_test.h"
#include "nvmonvars.h"
#include <malloc.h>




/*******************************************************************************
 *
 * Function   :	fugazi_access_device_test
 * Description:	main test for usb device test
 * Inputs     :	file path to usb device
 * Outputs    : PASSED or FAILED.
 *
 *******************************************************************************
 */
int fugazi_access_device_test (char *src)
{
    char buf[128], *buf_bk, *buf_wr, *buf_rd, *buf_bkrd;
    char *p1;
    char *p2;
    char *p3;
    char *p4;
    int devfd, num, ib;
    int ix, cnt = 0;
    off_t dev_size, test_size;
    int rc = PASSED;
    sprintf(buf, "%s", src);
    
    for (ix = 0; ix < 10; ix++) {
        devfd = open(buf, O_RDWR);
        if(devfd < 0) {
            sleep(1);
            continue;
        } else {
            break;
        }
    }

    if (devfd < 0) {
        perror("there is no device file descriptor available. ");
        printf("Can not access device at %s. is slot vacant?", src);
        return FAILED;
    }

    /* compare strings : usbdrv0, usbdrv1, emmc0, mSATA, m2sata, eUSB, cf */
    if ((strcmp(buf, DEV_USB0) == 0) || (strcmp(buf, DEV_USB1) == 0) || 
        (strcmp(buf, DEV_EMMC) == 0) || (strcmp(buf, DEV_MSATA) == 0) || 
        (strcmp(buf, DEV_M2SATA) == 0) || (strcmp(buf, DEV_EUSB) == 0) || 
        (strcmp(buf, DEV_CF) == 0)) {
        dev_size = lseek(devfd, 0, SEEK_END);
        /* Verifing 512 bytes */
        test_size = SIZE_512B;
    } else {
        dev_size = lseek(devfd, 0, SEEK_END);
        /* Verifing 100M bytes */
        test_size = SIZE_100MB;
    }

    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("\ndev_name = %s, dev_size = %lx, test_size = %lx\n", buf, dev_size, test_size);
    }

    buf_bk = (char *) malloc(test_size);
    buf_wr = (char *) malloc(test_size);
    buf_rd = (char *) malloc(test_size);
    buf_bkrd = (char *) malloc(test_size);

    p1 = buf_wr;
    p2 = buf_rd;
    p3 = buf_bk;
    p4 = buf_bkrd;
    memset(buf_bk, 0, malloc_usable_size(buf_bk));
    memset(buf_wr, 0, malloc_usable_size(buf_wr));
    memset(buf_rd, 0, malloc_usable_size(buf_rd));
    memset(buf_bkrd, 0, malloc_usable_size(buf_bkrd));

    /* back up data */
    if (lseek(devfd, 0, SEEK_SET) <  0) {
        perror("lseek to the beginning of device failed.");
        printf("backup lseek failed; Cannot point to the beginning of device.");
        rc = FAILED;
        goto exit;
    }
    /* Origianl common code (src/linux_usb_test.c) (num = read(devfd, buf_bk, malloc_usable_size(buf_bk)) 
     * The malloc_usable_size() get malloc() size more than test_size and not fix number, should use fix size for testing 
     * (test_size = 200;  malloc_usable_size(buf_wr) = 208)
     * if ((num = read(devfd, buf_bk, malloc_usable_size(buf_bk))) == -1) {
     */
    /* malloc_usable_size():
       The value returned by malloc_usable_size() may be greater than the
       requested size of the allocation because of alignment and minimum
       size constraints.  Although the excess bytes can be overwritten by
       the application without ill effects, this is not good programming
       practice: the number of excess bytes in an allocation depends on the
       underlying implementation.

       The main use of this function is for debugging and introspection.
     */
    if ((num = read(devfd, buf_bk, test_size)) == -1) {
        close(devfd); /* don't need it anymore */
        perror("Read data from device failed");
        printf("Unable to read from drive.\n");
        rc = FAILED;
        goto exit;
    }

    /* prepare data pattern */
    for (cnt = 0; cnt < test_size; cnt++) {
        buf_wr[cnt]= PATTERN + cnt;
    }

    /* write data pattern */
    if (lseek(devfd, 0, SEEK_SET) < 0) {
        perror("lseek to the beginning of device failed.");
        printf("write lseek failed; Cannot point to the beginning of device.");
        rc = FAILED;
        goto exit;
    }

    if ((num = write(devfd, buf_wr, test_size)) < 0) {
        perror("Write test pattern failed, can not write to drive.\n");
        printf("Unable to write data pattern to device.");
        rc = FAILED;
        goto exit;
    }

    if (num != test_size) {
        perror("not all the bytes are written for data pattern");
        printf("\n[buf_wr][buf_bk]\n");
        for (cnt = 0; cnt < test_size; cnt++) {
            printf("%d:[%.2x]:[%.2x] ",cnt,buf_wr[cnt],buf_bk[cnt]);
        }
        rc = FAILED;
        goto exit;
    }

    if (fsync(devfd)<0) {
        perror("fsync failed.");
        printf("Unable to sync data pattern to device.");
        rc = FAILED;
        goto exit;
    }

    /* read back data for comparing */
    if (lseek(devfd, 0, SEEK_SET) < 0) {
        perror("lseek to the beginning of device failed.");
        printf("lseek failed; Cannot point to the beginning of device.");
        rc = FAILED;
        goto exit;
    }

    if ((num = read(devfd, buf_rd, test_size)) == -1) {
        close(devfd); /* don't need it anymore */
        perror("Read back data from device failed");
        printf("Unable to read from drive.\n");
        rc = FAILED;
        goto exit;
    }

    if (num != test_size) {
        perror("not all the bytes are read for data pattern");
        rc = FAILED;
        goto exit;
    }
    
    /* comparing data */
    for (ib = 0; ib < test_size; ib++, p1++, p2++) {
        if (*p1 != *p2) {
            printf("failed on byte %d, wrote = %02x, read back = %02x\n",(ib+1), *p1, *p2);
            if (cnt++ > 10) {
                printf("Too many data mismatches. Stop testing\n");
            }
            break;
        }
    }

    /* restore data */
    if (lseek(devfd, 0, SEEK_SET) < 0) {
        perror("lseek to the beginning of device failed.");
        printf("restore lseek failed; Cannot point to the beginning of device.");
        rc = FAILED;
        goto exit;
    }
    
    if ((num = write(devfd, buf_bk, test_size)) < 0) {
        close(devfd); /* don't need it anymore */
        perror("Write restore data failed, can not write to drive.\n");
        printf("Unable to write restore data to device.");
        rc = FAILED;
        goto exit;
    }

    if (num != test_size) {
        perror("not all the bytes are written for restore");
        rc = FAILED;
        goto exit;
    }

    if (fsync(devfd)<0) {
        perror("fsync failed.");
        printf("Unable to sync data pattern to device.");
        rc = FAILED;
        goto exit;
    }

    /* read back restore data for comparing */
    if (lseek(devfd, 0, SEEK_SET) < 0) {
        perror("lseek to the beginning of device failed.");
        printf("lseek failed; Cannot point to the beginning of device.");
        rc = FAILED;
        goto exit;
    }

    if ((num = read(devfd, buf_bkrd, test_size)) == -1) {
        close(devfd); /* don't need it anymore */
        perror("Read back data from device failed");
        printf("Unable to read from drive.\n");
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
            printf("failed on byte %d, wrote = %02x, read back = %02x\n",(ib+1), *p3, *p4);
            if (cnt++ > 10) {
                printf("Too many data mismatches. Stop testing\n");
            }
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
    return rc;
}

/*******************************************************************************
 *
 * Function   :	fugazi_usb_slot_tests
 * Description:	entry point to usb device test
 * Inputs     :	file path to usb device
 * Outputs    : PASSED or FAILED.
 *
 *******************************************************************************
 */
int fugazi_usb_slot_tests (int slot)
{
    char src[32];
    int retval;
    
    sprintf(src, "/dev/usbdrv%d", slot);

    retval = fugazi_access_device_test(src);
    return retval;
}

/*******************************************************************************
 *
 * Function   : fugazi_emmc_slot_tests
 * Description: main test for emmc test.
 * Inputs     : dummy
 * Outputs    : PASSED or FAILED.
 *
 *******************************************************************************
 */
int fugazi_emmc_slot_tests (int dummy)
{
    char src[32];
    int retval;

    sprintf(src, "/dev/emmc0");

    retval = fugazi_access_device_test(src);
    return retval;
}

/*******************************************************************************
 *
 * Function   :	fugazi_sata_tests
 * Description:	main test for sata test.
 * Inputs     :	slot, 0 or 1.
 * Outputs    : PASSED or FAILED.
 *
 *******************************************************************************
 */
int fugazi_sata_tests (uchar *sata_name)
{
    char src[32];
    int retval;

    /* get sata name */
    sprintf(src, "%s", sata_name);	

    retval = fugazi_access_device_test(src);
    return retval;
}

/*
 *------------------------------------------------------------------
 * $Log: diag_usb_api.c,v $
 * Revision 1.2  2021/06/02 08:22:34  iachang
 * CSCvo59196-33 : Merge Fugazi from ASR1K-main-branch to ISR main trunk
 *
 * Revision 1.1.4.2  2020/08/26 02:37:49  iachang
 * Merge Fugazi code into main trunk
 *
 * Revision 1.1.2.2  2020/08/03 09:25:40  iachang
 * Code clean up.
 *
 * Revision 1.1.2.1  2020/04/24 07:13:16  iachang
 * The block device access test, used fix size to replace malloc_usable_size()
 *
 *
 *------------------------------------------------------------------
 * $Endlog$
 */
