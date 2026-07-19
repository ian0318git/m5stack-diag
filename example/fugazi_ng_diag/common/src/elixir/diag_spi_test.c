/* $Id: diag_spi_test.c,v 1.2 2021/09/24 01:21:07 harrchan Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/elixir/diag_spi_test.c,v $
 *------------------------------------------------------------------
 * 
 * diag_spi_test.c
 *
 * Copyright (c) 2018 - 2019 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#include "endians.h"
#include "defs.h"
#include "types.h"
#include "nvsysvars.h"
#include "error.h"
#include "pcmap.h"
#include "strings.h"
#include "common.h"
#include "cross_platform.h"
#include "setjmps.h"
#include "proto.h"
#include "diag_enhance_err_msg_lib.h"
#include "platform_cookie.h"
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
#include "proto.h"
#include "linux_usb_test.h"
#include "nvmonvars.h"
#include "diag_moka_fpga_lib.h"
#include "diag_spi_test.h"

static int access_device_test(char *);

/*******************************************************************************
 *
 * Function   :    spi_slot_tests
 * Description:    main test for spi test.
 * Inputs     :    option for future use
 * Outputs    : PASSED or FAILED.
 *
 *******************************************************************************
 */
int spi_slot_tests (int option)
{
    char src[32];
    int retval;

    sprintf(src, "/dev/mtdblock2");

    retval = access_device_test(src);
    return (retval);
}

/*******************************************************************************
 *
 * Function   :    access_device_test
 * Description:    main test for usb device test
 * Inputs     :    file path to usb device
 * Outputs    : PASSED or FAILED.
 *
 *******************************************************************************
 */
static int access_device_test (char *src)
{
    char buf[128], buf_bk[512], buf_wr[512], buf_rd[512];
    char *p1 = buf_wr;
    char *p2 = buf_rd;
    int devfd, num, ib;
    int ix, cnt = 0;

    if (!quiet_launch) {
        prpass(testpass, "Access device '%s' , ", src);
    }
    sprintf(buf, "%s", src);

    memset(buf_bk, 0, sizeof(buf_bk));
    memset(buf_wr, 0, sizeof(buf_wr));
    memset(buf_rd, 0, sizeof(buf_rd));

    for (ix = 0; ix < 10; ix++) {
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
    if (!quiet_launch) {
        prpass(testpass, "Backup data , ");
    }
    if (lseek(devfd, 0, SEEK_SET) < 0) {
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
    if (!quiet_launch) {
        prpass(testpass, "Prepare data pattern , ");
    }
    for (cnt = 0; cnt < sizeof(buf_wr); cnt++) {
        buf_wr[cnt] = PATTERN + cnt;
    }

    /*
     * write data pattern
     */
    if (!quiet_launch) {
        prpass(testpass, "Write data pattern , ");
    }
    if (lseek(devfd, 0, SEEK_SET) < 0) {
        close(devfd);           /* don't need it anymore */
        cterr('f', 0, "lseek to the beginning of device failed.");
        printf
            ("write lseek failed; Cannot point to the beginning of device.");
        return (FAILED);
    }

    if ((num = write(devfd, buf_wr, sizeof(buf_wr))) < 0) {
        close(devfd);           /* don't need it anymore */
        cterr('f', 0,
              "Write test pattern failed, can not write to drive.");
        printf("Unable to write data pattern to device.");
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
    if (!quiet_launch) {
        prpass(testpass, "Read back data for comparing , ");
    }
    if (lseek(devfd, 0, SEEK_SET) < 0) {
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
    if (!quiet_launch) {
        prpass(testpass, "Comparing data , ");
    }
    cnt = 0;
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
    if (!quiet_launch) {
        prpass(testpass, "Restore data , ");
    }
    if (lseek(devfd, 0, SEEK_SET) < 0) {
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

/*
 * Function: diag_bootflash_test
 *
 * Description : bootflash r/w tests.
 *
 * Inputs: slot - bootflash slot num
 *
 * Output: PASSED/FAILED
 */
int diag_bootflash_test (int slot)
{
    uchar mb_get_loc[FRU_SIZE] = {0};
    uchar mb_get_pid[FRU_SIZE] = {0};
#ifdef ENHANCE_ERROR_MSG_RDY
    /*
     * 1. Subtests of the test function will reuse all variables
     * 2. All variables will be cleared automatically when
     *    entering and leaving each menu item.
    */
    /* Segment 1: PID | Unique_string : slot_info */
    fru_table_offset = MB;
    /* fru_table_offset should be set, otherwise, it will not */
    /* go to enhanced error message format in cterr() */
    /* set fru_table_offset to get the predefine value */
    /* or change mb_pid & mb_loc below */
    platform_get_pid((char *)mb_get_pid);
    strcpy((char *)mb_get_loc, "MB");
    platform_fru_table[fru_table_offset].pid_string = mb_get_pid;
    platform_fru_table[fru_table_offset].location_string = mb_get_loc;

    /* Segment 2: Test step captured from prpass */
    /* Segment 3: Failure message captured from cterr */

    /* Segment 4: Components used */
    cterr_add_component("Marvell Armada 7040", "SPI", "SPI UEFI Flash");

    /* Segment 5: register and memory dump */
    //cterr_add_reg_dump((PFV)bcm63168_get_xdsl_profile);

    /* Segment 6: Platform Environment initialized here*/
    cterr_add_env_dump((PFV)show_plat_curr_temps);

    /* Segment 7: Top 3 Debugging Steps */
    cterr_add_debug("Boot up is OK means interfaces between Host SoC and flash is OK.",
                    "If there is no problem for these interfaces, "
                    "replace one flash and redo the test.");
#endif

    char *tname = "Bootflash";

    testname("%s", tname);
    prpass(testpass, "%s, ", tname);

    /*
     * testname is printed on spi_slot_tests
     */
    /*
     * platform uses spi flash
     */
    if (spi_slot_tests(slot) != PASSED) {
        cterr('f', 0, "SPI bootflash test failed.");
        return (FAILED);
    }
    return (PASSED);
}

/*-------------------------------------------------
 * $Log: diag_spi_test.c,v $
 * Revision 1.2  2021/09/24 01:21:07  harrchan
 * Collapse Elixir-branch to Main Trunk.
 *
 * Revision 1.1.2.1  2020/09/09 09:08:07  illiu
 * First version which has been ported with Dreamliner and Marvel CPSS
 *
 * Revision 1.2  2019/01/10 06:36:24  wilbhuan
 * The beginning of Betelgeuse application code.
 *
 *-------------------------------------------------
 */
