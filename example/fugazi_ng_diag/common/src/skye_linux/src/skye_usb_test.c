/* $Id: skye_usb_test.c,v 1.2 2015/05/25 03:59:17 steja Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/skye_linux/src/skye_usb_test.c,v $
 *------------------------------------------------------------------
 * Filename: skye_usb_test.c
 *
 * Jul 2014 - Paul Lin(palin2) ported from Shrinkray.
 *
 * Copyright (c) 2013-2015 by cisco Systems, Inc.
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
#include <sys/time.h>
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
#include "nvmonvars.h"

#define TEST_SIZE  1000

/*******************************************************************************
 *
 * Function   :	access_device_test
 * Description:	main test for usb device test
 * Inputs     :	file path to usb device
 * Outputs    : PASSED or FAILED.
 *
 *******************************************************************************
 */
int
access_device_test (char *src)
{
    unsigned char buf[128], buf_bk[1024], buf_wr[1024], buf_rd[1024];
    uchar *p1 = (uchar *)buf_wr;
    uchar *p2 = (uchar *)buf_rd;
    int devfd = -1, num, ib;
    int ix, cnt = 0;
  
    sprintf((char *)buf, "%s", (char *)src);

    memset((unsigned char *)buf_bk, 0, sizeof(buf_bk));
    memset((unsigned char *)buf_wr, 0, sizeof(buf_wr));
    memset((unsigned char *)buf_rd, 0, sizeof(buf_rd));

    for (ix = 0; ix < 10; ix++) {
        devfd = open((char *)buf, O_RDWR);
        if(devfd < 0) {
            sleep(1);
            continue;
        } else {
            break;
        }

    }
    if (devfd < 0) {
        perror("device test: ");
        printf("Can not access device at %s. is slot vacant?", src);
        return FAILED;
    }
    
    lseek(devfd, 0, SEEK_SET);
    if ((num = read(devfd, buf_bk, sizeof(buf_bk))) == -1) {
        close(devfd); /* don't need it anymore */
        perror("Read");
        printf("Unable to read from drive.\n");
        return FAILED;
    }

    for (cnt=0; cnt < sizeof(buf_wr); cnt++) {
        buf_wr[cnt]= PATTERN + cnt;
    }

    lseek(devfd, 0, SEEK_SET);
    if ((num = write(devfd, buf_wr, sizeof(buf_wr))) == -1) {
        perror("Write");
        printf("Write test pattern failed, can not write to drive.\n");
        return FAILED;
    }
    lseek(devfd, 0, SEEK_SET);
    if ((num = read(devfd, buf_rd, sizeof(buf_rd))) == -1) {
        close(devfd); /* don't need it anymore */
        perror("Read");
        printf("Readback failed, can not read from drive.\n");
        return FAILED;
    }

    for (ib =0; ib < sizeof(buf_rd); ib++, p1++, p2++) {
        if (*p1 != *p2) {
            printf("failed on byte %d, wrote = %02x, read back = %02x\n",(ib+1), *p1, *p2);
            if (cnt++ > 10) {
                printf("Too many data mismatches. Stop testing\n");
            }
            break;
        }
    }

    lseek(devfd, 0, SEEK_SET);
    if ((num = write(devfd, buf_bk, sizeof(buf_bk))) == -1) {
        close(devfd); /* don't need it anymore */
        perror("Write");
        printf("Unable to write original data to drive\n");
        return FAILED;
    }

    close(devfd); /* don't need it anymore */

    return PASSED;

}

/*******************************************************************************
 *
 * Function   : timeval_diff
 * Description: timeval difference
 * Inputs     : difference - time value diff
 *              end time , start time
 * Outputs    : PASSED or FAILED.
 *
 *******************************************************************************
 */
long long
timeval_diff(struct timeval *difference,
             struct timeval *end_time,
             struct timeval *start_time
            )
{
  struct timeval temp_diff;

  if(difference==NULL)
  {
    difference=&temp_diff;
  }

  difference->tv_sec =end_time->tv_sec -start_time->tv_sec ;
  difference->tv_usec=end_time->tv_usec-start_time->tv_usec;

  /* Using while instead of if below makes the code slightly more robust. */

  while(difference->tv_usec<0)
  {
    difference->tv_usec+=1000000;
    difference->tv_sec -=1;
  }

  return 1000000LL*difference->tv_sec+
                   difference->tv_usec;

} /* timeval_diff() */



/*******************************************************************************
 *
 * Function   :	access_device_test
 * Description:	entry point to usb device test
 * Inputs     :	file path to usb device
 * Outputs    : PASSED or FAILED.
 *
 *******************************************************************************
 */
int 
usb_slot_tests (int slot)
{
    char src[32];
    int retval = FAILED, fd = -1;

    int ctr = 0;

    struct timeval t1, t2;
    struct timeval interval;

    printf("\n\n%s: Debug message - slot = %d.\n\n", __FUNCTION__, slot);
    
    testname("USB slot %d access", slot);

    sprintf((char *)src, "/dev/sda%d", slot);
    sprintf((char *)src, "/dev/sdb%d", slot);
    fd = open(src, O_RDWR);
    if (fd < 0) {
        cterr('f', 0, "%s: Failed to open %s.\n", __FUNCTION__, src);
        return (FAILED);
    }

    /* Start Timer */
    gettimeofday(&t1, NULL);


    for (ctr = 0; ctr < 1024 * 10; ctr++) {
        retval = access_device_test(src);
        if(retval != PASSED) {
            break;
        }
    }

    if (retval != PASSED) {
        cterr('f', 0, "access usb test failed\n");
        close(fd);
        return (FAILED);
    }

    /* Stop Timer */
    gettimeofday(&t2, NULL);

    /* compute and print the elapsed time in millisec */
      printf("difference is %lld microseconds",
         timeval_diff(&interval, &t2, &t1)
        );

    printf(" (%ld seconds, %ld microseconds)\n",
           interval.tv_sec,
           interval.tv_usec
          );

    close(fd);
    return retval;
}

/*******************************************************************************
 *
 * Function   :	access_device_test
 * Description:	entry point to usb device test
 * Inputs     :	file path to usb device
 * Outputs    : PASSED or FAILED.
 *
 *******************************************************************************
 */
int
eusb_slot_tests (int slot)
{
    char src[32];
    int retval = FAILED, fd = -1;

    int ctr = 0;

    struct timeval t1, t2;
    struct timeval interval;

    printf("\n\n%s: Debug message - slot = %d.\n\n", __FUNCTION__, slot);

    testname("USB slot %d access", slot);

    sprintf((char *)src, "/dev/sda%d", slot);
    fd = open(src, O_RDWR);
    if (fd < 0) {
        cterr('f', 0, "%s: Failed to open %s.\n", __FUNCTION__, src);
        return (FAILED);
    }

    /* Start Timer */
    gettimeofday(&t1, NULL);


    for (ctr = 0; ctr < 1024 * 10; ctr++) {
        retval = access_device_test(src);
        if(retval != PASSED) {
            break;
        }
    }

    if (retval != PASSED) {
        cterr('f', 0, "access usb test failed\n");
        close(fd);
        return (FAILED);
    }

    /* Stop Timer */
    gettimeofday(&t2, NULL);

    /* compute and print the elapsed time in millisec */
      printf("difference is %lld microseconds",
         timeval_diff(&interval, &t2, &t1)
        );

  printf(" (%ld seconds, %ld microseconds)\n",
         interval.tv_sec,
         interval.tv_usec
        );

    close(fd);
    return retval;
}

/*************************below left for ref ******************/
/* we assume that udev rule assignes /dev/usb0x to the device name of usb slot 0 (ie, /dev/sdb, etc..)
   if we change the name in the udev rule, then we need to change the nanme in this code as well.
*/
/******** History ********
$Log: skye_usb_test.c,v $
Revision 1.2  2015/05/25 03:59:17  steja
Add Support Skye SM

Revision 1.1.4.2  2015/04/29 11:36:36  steja
Code check-in to skye-branch2 for ER code review


------------------------------------------------------------
Revision 1.1.2.3  2014/09/17 04:35:08  palin2
Updated Skye enhanced error message.

Revision 1.1.2.2  2014/08/31 23:01:52  palin2
Added enhanced error message.

Revision 1.1.2.1  2014/07/21 01:56:56  palin2
Initial check-in Skye module side Diag code.

--------------------------------------------------
skye_usb_test.c:
Revision 1.2.8.2  2014/05/14 14:24:04  palin2
Add "close" function at the end of USB and eUSB test to fix
potential "Too many open files" fault.

Revision 1.2.8.1  2014/05/13 11:00:06  palin2
Fixed USB test potential issue.

Revision 1.2  2014/02/27 15:01:45  palin2
Initial check-in ShrinkRay SM side Diag code.

Revision 1.1.2.4  2014/02/13 09:31:13  palin2
Update code based on review comments.

Revision 1.1.2.3  2014/02/07 18:31:32  steja
code clean up

Revision 1.1.2.2  2013/11/29 07:08:55  steja
1. Fix the full data path TLK working.
2. add USB test
3. add read BIB MAC utility

Revision 1.1.2.1  2013/10/09 03:02:00  palin2
Add USB test for ShrinkRay.

--------------------------------------------------
$Endlog$
*/

