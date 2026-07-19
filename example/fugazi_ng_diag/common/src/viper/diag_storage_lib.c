 /* $Id: diag_storage_lib.c,v 1.2 2018/08/06 02:31:51 harrchan Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/viper/diag_storage_lib.c,v $
 *------------------------------------------------------------------
 *
 * Filename: diag_storage_lib.c
 *
 * Copyright (c) 2013-2018 by cisco Systems, Inc.
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
#include <mtd/mtd-user.h>
#include <sys/ioctl.h>
#include "types.h"
#include "common.h"
#include "monitor.h"
#include "menu.h"
#include "error.h"
#include "proto.h"
#include "linux_usb_test.h"
#include "nvmonvars.h"
#include "plat_defs.h"
#include "diag_storage_lib.h"

/*
 * Declare local function
 */
int access_device_test (char *);


/*******************************************************************************
 *
 * Function   :    access_device_test
 * Description:    main test for storage device test
 * Inputs     :    file path to storage device
 * Outputs    : PASSED or FAILED.
 *
 *******************************************************************************
 */
int access_device_test (char *src)
{
    char buf[128];
    char buf_bk[512], buf_wr[512], buf_rd[512];
    char *p1 = buf_wr;
    char *p2 = buf_rd;
    int devfd, num, ib;
    int ix, cnt = 0;

    prpass(testpass, "Access device '%s' , ", src);
    sprintf(buf, "%s", src);

    printf("\n %s \n", src);
    memset(buf_bk, 0, sizeof(buf_bk));
    memset(buf_wr, 0, sizeof(buf_wr));
    memset(buf_rd, 0, sizeof(buf_rd));

    for (ix = 0; ix < DEV_OPEN_RETRY; ix++) {
        devfd = open(buf, O_RDWR);
        if (devfd < 0) {
            sleep(1);
            continue;
        } else {
            break;
        }

    }
    if (devfd < 0) {
        close(devfd);           /* don't need it anymore */
        cterr('f', 0, "there is no device file descriptor available.");
        return (FAILED);
    }

    /*
     * back up data
     */
    prpass(testpass, "Backup data , ");
    if (lseek(devfd, USB_OFFSET, SEEK_SET) < 0) {
        close(devfd);           /* don't need it anymore */
        cterr('f', 0, "lseek to the beginning of device failed.");
        printf
            ("backup lseek failed; Cannot point to the beginning of device.");
        return (FAILED);
    }
    if ((num = read(devfd, buf_bk, sizeof(buf_bk))) == -1) {
        close(devfd);           /* don't need it anymore */
        cterr('f', 0, "Read data from device failed");
        printf("Unable to read from drive.\n");
        return (FAILED);
    }

    /*
     * prepare data pattern
     */
    prpass(testpass, "Prepare data pattern , ");
    for (cnt = 0; cnt < sizeof(buf_wr); cnt++) {
        buf_wr[cnt] = PATTERN + cnt;
    }

    /*
     * write data pattern
     */
    prpass(testpass, "Write data pattern , ");
    if (lseek(devfd, USB_OFFSET, SEEK_SET) < 0) {
        close(devfd);           /* don't need it anymore */
        cterr('f', 0, "lseek to the beginning of device failed.");
        printf
            ("write lseek failed; Cannot point to the beginning of device.");
        return (FAILED);
    }

    if ((num = write(devfd, buf_wr, sizeof(buf_wr))) < 0) {
        close(devfd);           /* don't need it anymore */
        printf("Strerror = %s.", strerror(errno));
        cterr('f', 0,
              "Write test pattern failed, can not write to drive.");
        printf("Unable to write data pattern to device num = %d.", num);
        return (FAILED);
    }

    if (num != sizeof(buf_bk)) {
        close(devfd);           /* don't need it anymore */
        cterr('f', 0, "not all the bytes are written for data pattern");
        return (FAILED);
    }

    if (fsync(devfd) < 0) {
        close(devfd);           /* don't need it anymore */
        cterr('f', 0, "fsync failed.");
        printf("Unable to sync data pattern to device.");
        return (FAILED);
    }

    /*
     * read back data for comparing
     */
    prpass(testpass, "Read back data for comparing , ");
    if (lseek(devfd, USB_OFFSET, SEEK_SET) < 0) {
        close(devfd);           /* don't need it anymore */
        cterr('f', 0, "lseek to the beginning of device failed.");
        printf("lseek failed; Cannot point to the beginning of device.");
        return (FAILED);
    }

    if ((num = read(devfd, buf_rd, sizeof(buf_rd))) == -1) {
        close(devfd);           /* don't need it anymore */
        cterr('f', 0, "Read back data from device failed");
        printf("Unable to read from drive.\n");
        return (FAILED);
    }
    if (num != sizeof(buf_rd)) {
        close(devfd);           /* don't need it anymore */
        cterr('f', 0, "not all the bytes are read for data pattern");
        return (FAILED);
    }

    /*
     * comparing data
     */
    prpass(testpass, "Comparing data , ");
    for (ib = 0; ib < sizeof(buf_rd); ib++, p1++, p2++) {
        if (*p1 != *p2) {
            printf("failed on byte %d, wrote = %02x, read back = %02x\n",
                   (ib + 1), *p1, *p2);
            if (cnt++ > 10) {
                printf("Too many data mismatches. Stop testing\n");
            }
            break;
        }
    }

    /*
     * restore data
     */
    prpass(testpass, "Restore data , ");
    if (lseek(devfd, USB_OFFSET, SEEK_SET) < 0) {
        close(devfd);           /* don't need it anymore */
        cterr('f', 0, "lseek to the beginning of device failed.");
        return (FAILED);
    }

    if ((num = write(devfd, buf_bk, sizeof(buf_bk))) < 0) {
        close(devfd);           /* don't need it anymore */
        cterr('f', 0,
              "Write restore data failed, can not write to drive.\n");
        return (FAILED);
    }

    if (num != sizeof(buf_bk)) {
        close(devfd);           /* don't need it anymore */
        cterr('f', 0, "not all the bytes are written for restore");
        return (FAILED);
    }

    if (fsync(devfd) < 0) {
        close(devfd);           /* don't need it anymore */
        cterr('f', 0, "fsync failed.");
        return (FAILED);
    }

    close(devfd);               /* don't need it anymore */
    return (PASSED);

}

/******** History ********
$Log: diag_storage_lib.c,v $
Revision 1.2  2018/08/06 02:31:51  harrchan
Merge viper E2E to the main trunk (CSCvk28469)

Revision 1.1.2.2  2018/07/03 05:38:55  harrchan
Follow the coding rule to clean up code

Revision 1.1.2.1  2018/02/27 08:06:46  harrchan
Initial viper application code base



$Endlog$
*/
