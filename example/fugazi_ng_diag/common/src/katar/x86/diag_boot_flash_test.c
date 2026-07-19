/* $Id: diag_boot_flash_test.c,v 1.2 2019/06/14 05:24:48 mikech2 Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/katar/x86/diag_boot_flash_test.c,v $
 *------------------------------------------------------------------
 *
 * diag_boot_flash_test.c - Boot flash test
 *
 *
 * Copyright (c) 2008-2019 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include "endians.h"
#include "defs.h"
#include "types.h"
#include "nvsysvars.h"
#include "error.h"
#include "pcmap.h"
#include "strings.h"
#include "menu.h"
#include "common.h"
#include "cross_platform.h"
#include "mb_tests.h"
#include "plat_defs.h"
#include "setjmps.h"
#include "proto.h"
#include "platform_fpga.h"
#include "platform_cookie.h"
#include "plat_defs.h"
#include "diag_storage_lib.h"


/*
 * Global variables
 */
#define BOOT_FLASH_DEV_PATH "/dev/mtdblock0"
#define BOOT_FLASH_SWITCH_DELAY 10

/* External functions */
extern int do_all_menu_items(struct menuinfo *);
extern void msleep(int t);

/* Local functions */
int diag_boot_flash_test(int);
int build_boot_flash_menu(boolean);
int mtd_bootflash_test(char *);
int bootflash_tests(void);
int diag_boot_flash_switch_test(int);
int boot_flash_switch_test(int);

/*
 * Sub Menu used for "Boot flash test -> Boot flash submenu test"
 */
submenu_xtable_t boot_flash_submenu_table[] = {
    {"Primary Bootflash Test",
     (PFT) diag_boot_flash_test, SPICTL_BOOT_GOLDEN,
     MF_CONTINUOUS | MF_DOGRP,
     (type_t(*)())0, 0, (PFT) 0, 0},

    {"Secondary Bootflash Test",
     (PFT) diag_boot_flash_test, SPICTL_BOOT_UPGRADE,
     MF_CONTINUOUS | MF_DOGRP,
     (type_t(*)())0, 0, (PFT) 0, 0},

    {"Bootflash Switch Test",
     (PFT) diag_boot_flash_switch_test, 0,
     MF_CONTINUOUS | MF_DOGRP | MF_DOALL,
     (type_t(*)())0, 0, (PFT) 0, 0},
};

#define BOOT_FLASH_SUBMENU_TABLE_SIZE (sizeof(boot_flash_submenu_table) / \
                                     sizeof(submenu_xtable_t))

/*
 * "motherboard test -> boot flash test" primary & secondary submenu items (filled in from xtable)
 */
static mitem_t boot_flash_primary_items[BOOT_FLASH_SUBMENU_TABLE_SIZE +
                                      MAX_BASE_ITEMS];
static mitem_t boot_flash_secondary_items[BOOT_FLASH_SUBMENU_TABLE_SIZE +
                                        MAX_BASE_ITEMS];

menuinfo_t boot_subtest_menu = {
    "%s Subtest Menu",
    0,                          /* mtparam added by init_empty_menu */
    (PFT) show_endnote,         /* notes missing WICs in combos */
    0,                          /* use generic prompt */
    0,                          /* size (bumped by add_menu_item() */
    boot_flash_primary_items,
};

menuinfo_t *boot_submenup = &boot_subtest_menu;


/*******************************************************************************
 *
 * Function   : build_boot_flash_menu
 * Description: build the menu of boot flash 
 * Inputs     : Test/Menu 
 * Outputs    : None
 *
 *******************************************************************************
 */
int build_boot_flash_menu (boolean mb_temp_test_items_executed)
{
    char *tname = "Bootflash Test";
    testname(tname);

    build_primary_submenu(boot_flash_submenu_table, BOOT_FLASH_SUBMENU_TABLE_SIZE,
                          "Boot flash test", &boot_submenup);
    build_secondary_submenu(boot_flash_submenu_table, BOOT_FLASH_SUBMENU_TABLE_SIZE,
                            boot_flash_secondary_items);
    if (mb_temp_test_items_executed) {
        menu(&boot_subtest_menu, boot_flash_secondary_items, 0);
    } else {
        do_all_menu_items(boot_submenup);
    }
    return (PASSED);
}



/******************************************************************************
 *
 * Function: diag_boot_flash_test
 *
 * Description: Boot flash test
 *
 * Inputs      : select - select primary or secondary bootflash test
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/
int diag_boot_flash_test (int select)
{
    char *tname[] = {"Boot flash 1", "Boot flash 2"};

    if( (select == SPICTL_BOOT_GOLDEN) || (select == SPICTL_BOOT_UPGRADE) ){
        katar_boot_spi_select_control(FALSE,select);
        testname("%s", tname[select]);
    } else {
        printf("Wrong index of select bootflash\n");
		return (FAILED);
    }
    prpass(testpass, "switch flash done");

    if (bootflash_tests() != PASSED) {
        cterr('f', 0, "SPI bootflash test failed.");
        return (FAILED);
    }
    prpass(testpass, "Boot Flash read/write test passed, ");
    prcomplete(testpass, errcount, (char*)0);

	katar_boot_spi_select_control(TRUE,0);
    return (PASSED);
}

/*******************************************************************************
 *
 * Function   :    bootflash_tests
 * Description:    function for bootflash device path
 * Inputs     :    none
 * Outputs    : PASSED or FAILED.
 *
 *******************************************************************************
 */
int bootflash_tests (void)
{
    char src[32];
    int retval;
    sprintf(src, BOOT_FLASH_DEV_PATH);

    retval = mtd_bootflash_test(src);
    return (retval);
}

/*******************************************************************************
 *
 * Function   :    mtd_bootflash_test
 * Description:    main test for bootflash test
 * Inputs     :    file path to emmc device
 * Outputs    : PASSED or FAILED.
 *
 *******************************************************************************
 */
int mtd_bootflash_test (char *src)
{
    char buf[128];
    char buf_bk[BOOTFLASH_TEST_LEN], buf_wr[BOOTFLASH_TEST_LEN];
    char buf_rd[BOOTFLASH_TEST_LEN]; 
    char *p1 = buf_wr;
    char *p2 = buf_rd;
    int devfd, num, ib;
    int ix, jx, cnt = 0;

    prpass(testpass, "Access device '%s' , ", src);
    sprintf(buf, "%s", src);

    printf("\n %s \n", src);
    memset(buf_bk, 0, sizeof(buf_bk));
    memset(buf_wr, 0, sizeof(buf_wr));
    memset(buf_rd, 0, sizeof(buf_rd));

    for (ix = 0; ix < 10; ix++) {
        devfd = open(buf, O_RDWR | O_SYNC);
        if (devfd < 0) {
            sleep(1);
            continue;
        } else {
            break;
        }

    }

    if (devfd < 0) {
        close(devfd);           /* don't need it anymore */
        printf("This test needs the kernel compiled on Mar 25 2019 or later.\n");
        cterr('f', 0, "there is no device file descriptor available.");
        printf("Strerror = %s.", strerror(errno));
        return (FAILED);
    }

    /*
     * back up data
     */
    prpass(testpass, "Backup data , ");

    if (lseek(devfd, SECTOR_OFFSET, SEEK_SET) < 0) {
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


    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("\n backup data\n");
        for (jx = 0; jx < 256; jx++) {
            printf("%x ",buf_bk[jx]);
        }
        printf("\n num = %d\n",num);
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
    if (lseek(devfd, SECTOR_OFFSET, SEEK_SET) < 0) {
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

    close(devfd);
    for (ix = 0; ix < 10; ix++) {
        devfd = open(buf, O_RDWR | O_SYNC);
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
        printf("Strerror = %s.", strerror(errno));
        return (FAILED);
    }

    /*
     * read back data for comparing
     */
    prpass(testpass, "Read back data for comparing , ");
    if (lseek(devfd, SECTOR_OFFSET, SEEK_SET) < 0) {
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
    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("\n read back data\n");
        for (jx = 0; jx < 256; jx++) {
            printf("%x ",buf_rd[jx]);
        }
        printf("\n num = %d\n",num);
    }
    /*
     * comparing data
     */
    prpass(testpass, "Comparing data , ");
    for (ib = 0; ib < sizeof(buf_rd); ib++, p1++, p2++) {
        if (*p1 != *p2) {
			close(devfd);           /* don't need it anymore */
            cterr('f', 0, "failed on byte %d, wrote = %02x, read back = %02x", (ib + 1), *p1, *p2);
            printf("Mismatched.\n");
            return (FAILED);
        }
    }

    /*
     * restore data
     */
    prpass(testpass, "Restore data , ");
    if (lseek(devfd, SECTOR_OFFSET, SEEK_SET) < 0) {
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


/******************************************************************************
 *
 * Function: diag_boot_flash_switch_test
 *
 * Description: Boot flash switch test
 *
 * Inputs      : option
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/
int diag_boot_flash_switch_test (int opt)
{

	testname("%s", "Boot flash 1&2");
    if (boot_flash_switch_test(0) != PASSED) {
        cterr('f', 0, "Bootflash switch test failed.");
        printf("This test requires Aikido-v10023 or later.\n");
        return (FAILED);
    }
    prpass(testpass, "Bootflash switch test passed, ");
    prcomplete(testpass, errcount, (char*)0);

    katar_boot_spi_select_control(TRUE, 0);

    return (PASSED);
}


/*******************************************************************************
 *
 * Function   :    boot_flash_switch_test
 * 
 * Description:    main test for bootflash switch test
 * 
 * Inputs     :    option
 * Outputs    :    PASSED or FAILED.
 *
 *******************************************************************************
 */
int boot_flash_switch_test (int opt)
{
    char buf[128];
    char buf_bk1[BOOTFLASH_SWITCH_TEST_LEN], buf_bk2[BOOTFLASH_SWITCH_TEST_LEN];
    char buf_wr1[BOOTFLASH_SWITCH_TEST_LEN], buf_wr2[BOOTFLASH_SWITCH_TEST_LEN];
    char buf_rd1[BOOTFLASH_SWITCH_TEST_LEN], buf_rd2[BOOTFLASH_SWITCH_TEST_LEN];
    char *p1;
    char *p2;
    int devfd, num, ib;
    int ix, jx, cnt = 0;

    char src[32];
    sprintf(src, BOOT_FLASH_DEV_PATH);

    prpass(testpass, "Access device '%s' , ", src);
    sprintf(buf, "%s", src);

    printf("\n %s \n", src);
    memset(buf_bk1, 0, sizeof(buf_bk1));
    memset(buf_wr1, 0, sizeof(buf_wr1));
    memset(buf_rd1, 0, sizeof(buf_rd1));
    memset(buf_bk2, 0, sizeof(buf_bk2));
    memset(buf_wr2, 0, sizeof(buf_wr2));
    memset(buf_rd2, 0, sizeof(buf_rd2));

    for (ix = 0; ix < 10; ix++) {
        devfd = open(buf, O_RDWR | O_SYNC);
        if (devfd < 0) {
            sleep(1);
            continue;
        } else {
            break;
        }

    }

    if (devfd < 0) {
        close(devfd);           /* don't need it anymore */
        printf("This test requires the kernel compiled on Mar 25 2019 or later.\n");
        cterr('f', 0, "there is no device file descriptor available.");
        printf("Strerror = %s.", strerror(errno));
        return (FAILED);
    }


    katar_boot_spi_select_control(FALSE, SPICTL_BOOT_GOLDEN);  // switch to bootflash#1
    msleep(BOOT_FLASH_SWITCH_DELAY);

    /*
     * 1 - back up data in bootflash#1
     */
    prpass(testpass, "Backup data of bootflash#1, ");

    if (lseek(devfd, SECTOR_OFFSET, SEEK_SET) < 0) {
        close(devfd);           /* don't need it anymore */
        cterr('f', 0, "lseek to the beginning of bootflash#1 failed.");
        printf
            ("backup lseek failed; Cannot point to the beginning of bootflash#1.");
        return (FAILED);
    }
    if ((num = read(devfd, buf_bk1, sizeof(buf_bk1))) == -1) {
        close(devfd);           /* don't need it anymore */
        cterr('f', 0, "Read data from bootflash#1 failed");
        printf("Unable to read from bootflash#1.\n");
        return (FAILED);
    }


    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("\n Backup data of bootflash#1:\n");
        for (jx = 0; jx < BOOTFLASH_SWITCH_TEST_LEN; jx++) {
            printf("%x ",buf_bk1[jx]);
        }
        printf("\n size = %d\n",jx);
    }

    /*
     * 2 - prepare data pattern of bootflash#1
     */
    prpass(testpass, "Prepare data pattern of bootflash#1 , ");
    for (cnt = 0; cnt < sizeof(buf_wr1); cnt++) {
        buf_wr1[cnt] = PATTERN + cnt;
    }

    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("\n Data pattern for bootflash#1:\n");
        for (jx = 0; jx < BOOTFLASH_SWITCH_TEST_LEN; jx++) {
            printf("%x ",buf_wr1[jx]);
        }
        printf("\n size = %d\n",jx);
    }

    /*
     * 3 - write data pattern to bootflash#1
     */
    prpass(testpass, "Write data pattern to bootflash#1 , ");
    if (lseek(devfd, SECTOR_OFFSET, SEEK_SET) < 0) {
        close(devfd);           /* don't need it anymore */
        cterr('f', 0, "lseek to the beginning of bootflash#1 failed.");
        printf
            ("write lseek failed; Cannot point to the beginning of bootflash#1.");
        return (FAILED);
    }

    if ((num = write(devfd, buf_wr1, sizeof(buf_wr1))) < 0) {
        close(devfd);           /* don't need it anymore */
        printf("Strerror = %s.", strerror(errno));
        cterr('f', 0,
              "Write test pattern failed, can not write to bootflash#1.");
        printf("Unable to write data pattern to bootflash#1; num = %d.", num);
    }

    if (num != sizeof(buf_bk1)) {
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

    close(devfd);
    for (ix = 0; ix < 10; ix++) {
        devfd = open(buf, O_RDWR | O_SYNC);
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
        printf("Strerror = %s.", strerror(errno));
        return (FAILED);
    }

    /*
     * 4 - read back data in bootflash#1 for comparing
     */
    prpass(testpass, "Read back data in bootflash#1 for comparing , ");
    if (lseek(devfd, SECTOR_OFFSET, SEEK_SET) < 0) {
        close(devfd);           /* don't need it anymore */
        cterr('f', 0, "lseek to the beginning of bootflash#1 failed.");
        printf("lseek failed; Cannot point to the beginning of bootflash#1.");
        return (FAILED);
    }

    if ((num = read(devfd, buf_rd1, sizeof(buf_rd1))) == -1) {
        close(devfd);           /* don't need it anymore */
        cterr('f', 0, "Read back data from bootflash#1 failed");
        printf("Unable to read from bootflash#1.\n");
        return (FAILED);
    }
    if (num != sizeof(buf_rd1)) {
        close(devfd);           /* don't need it anymore */
        cterr('f', 0, "not all the bytes are read for data pattern");
        return (FAILED);
    }
    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("\n Read data back from bootflash#1:\n");
        for (jx = 0; jx < BOOTFLASH_SWITCH_TEST_LEN; jx++) {
            printf("%x ",buf_rd1[jx]);
        }
        printf("\n size = %d\n",jx);
    }

    /*
     * 5 - compare data in bootflash#1
     */
    p1 = buf_wr1;
    p2 = buf_rd1;
    prpass(testpass, "Comparing data , ");
    for (ib = 0; ib < sizeof(buf_rd1); ib++, p1++, p2++) {
        if (*p1 != *p2) {
			close(devfd);           /* don't need it anymore */
            cterr('f', 0, "failed on byte %d, wrote = %02x, read back = %02x", (ib + 1), *p1, *p2);
            printf("Mismatched.\n");
            return (FAILED);
        }
    }

    // switch to bootflash#2
    close(devfd);
    for (ix = 0; ix < 10; ix++) {
        devfd = open(buf, O_RDWR | O_SYNC);
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
        printf("Strerror = %s.", strerror(errno));
        return (FAILED);
    }

    katar_boot_spi_select_control(FALSE, SPICTL_BOOT_UPGRADE);  // switch to bootflash#2
    msleep(BOOT_FLASH_SWITCH_DELAY);


    /*
     * 6 - back up data in bootflash#2
     */
    prpass(testpass, "Backup data of bootflash#2, ");

    if (lseek(devfd, SECTOR_OFFSET, SEEK_SET) < 0) {
        close(devfd);           /* don't need it anymore */
        cterr('f', 0, "lseek to the beginning of bootflash#2 failed.");
        printf
            ("backup lseek failed; Cannot point to the beginning of bootflash#2.");
        return (FAILED);
    }
    if ((num = read(devfd, buf_bk2, sizeof(buf_bk2))) == -1) {
        close(devfd);           /* don't need it anymore */
        cterr('f', 0, "Read data from bootflash#2 failed");
        printf("Unable to read from bootflash#2.\n");
        return (FAILED);
    }


    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("\n Backup data of bootflash#2:\n");
        for (jx = 0; jx < BOOTFLASH_SWITCH_TEST_LEN; jx++) {
            printf("%x ",buf_bk2[jx]);
        }
        printf("\n size = %d\n",jx);
    }

    /*
     * 7 - prepare data pattern of bootflash#2
     */
    prpass(testpass, "Prepare data pattern of bootflash#2 , ");
    for (cnt = 0; cnt < sizeof(buf_wr2); cnt++) {
        buf_wr2[cnt] = PATTERN - cnt;
    }

    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("\n Data pattern for bootflash#2:\n");
        for (jx = 0; jx < BOOTFLASH_SWITCH_TEST_LEN; jx++) {
            printf("%x ",buf_wr2[jx]);
        }
        printf("\n size = %d\n",jx);
    }

    /*
     * 8 - write data pattern to bootflash#2
     */
    prpass(testpass, "Write data pattern to bootflash#2 , ");
    if (lseek(devfd, SECTOR_OFFSET, SEEK_SET) < 0) {
        close(devfd);           /* don't need it anymore */
        cterr('f', 0, "lseek to the beginning of bootflash#2 failed.");
        printf
            ("write lseek failed; Cannot point to the beginning of bootflash#2.");
        return (FAILED);
    }

    if ((num = write(devfd, buf_wr2, sizeof(buf_wr2))) < 0) {
        close(devfd);           /* don't need it anymore */
        printf("Strerror = %s.", strerror(errno));
        cterr('f', 0,
              "Write test pattern failed, can not write to bootflash#2.");
        printf("Unable to write data pattern to bootflash#2; num = %d.", num);
    }

    if (num != sizeof(buf_bk2)) {
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

    close(devfd);
    for (ix = 0; ix < 10; ix++) {
        devfd = open(buf, O_RDWR | O_SYNC);
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
        printf("Strerror = %s.", strerror(errno));
        return (FAILED);
    }

    /*
     * 9 - read back data in bootflash#2 for comparing
     */
    prpass(testpass, "Read back data in bootflash#2 for comparing , ");
    if (lseek(devfd, SECTOR_OFFSET, SEEK_SET) < 0) {
        close(devfd);           /* don't need it anymore */
        cterr('f', 0, "lseek to the beginning of bootflash#2 failed.");
        printf("lseek failed; Cannot point to the beginning of bootflash#2.");
        return (FAILED);
    }

    if ((num = read(devfd, buf_rd2, sizeof(buf_rd2))) == -1) {
        close(devfd);           /* don't need it anymore */
        cterr('f', 0, "Read back data from bootflash#2 failed");
        printf("Unable to read from bootflash#2.\n");
        return (FAILED);
    }
    if (num != sizeof(buf_rd2)) {
        close(devfd);           /* don't need it anymore */
        cterr('f', 0, "not all the bytes are read for data pattern");
        return (FAILED);
    }
    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("\n Read data back from bootflash#2:\n");
        for (jx = 0; jx < BOOTFLASH_SWITCH_TEST_LEN; jx++) {
            printf("%x ",buf_rd2[jx]);
        }
        printf("\n size = %d\n",jx);
    }

    /*
     * 10 - compare data in bootflash#2
     */
    p1 = buf_wr2;
    p2 = buf_rd2;
    prpass(testpass, "Comparing data , ");
    for (ib = 0; ib < sizeof(buf_rd2); ib++, p1++, p2++) {
        if (*p1 != *p2) {
			close(devfd);           /* don't need it anymore */
            cterr('f', 0, "failed on byte %d, wrote = %02x, read back = %02x", (ib + 1), *p1, *p2);
            printf("Mismatched.\n");
            return (FAILED);
        }
    }

    // switch to bootflash#1
    close(devfd);
    for (ix = 0; ix < 10; ix++) {
        devfd = open(buf, O_RDWR | O_SYNC);
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
        printf("Strerror = %s.", strerror(errno));
        return (FAILED);
    }

    katar_boot_spi_select_control(FALSE, SPICTL_BOOT_GOLDEN);  // switch to bootflash#1
    msleep(BOOT_FLASH_SWITCH_DELAY);

    /*
     * 11 - read back data in bootflash#1 for comparing
     */
    prpass(testpass, "Read back data in bootflash#1 for comparing , ");
    if (lseek(devfd, SECTOR_OFFSET, SEEK_SET) < 0) {
        close(devfd);           /* don't need it anymore */
        cterr('f', 0, "lseek to the beginning of bootflash#1 failed.");
        printf("lseek failed; Cannot point to the beginning of bootflash#1.");
        return (FAILED);
    }

    if ((num = read(devfd, buf_rd1, sizeof(buf_rd1))) == -1) {
        close(devfd);           /* don't need it anymore */
        cterr('f', 0, "Read back data from bootflash#1 failed");
        printf("Unable to read from bootflash#1.\n");
        return (FAILED);
    }
    if (num != sizeof(buf_rd1)) {
        close(devfd);           /* don't need it anymore */
        cterr('f', 0, "not all the bytes are read for data pattern");
        return (FAILED);
    }
    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("\n Read data back from bootflash#1:\n");
        for (jx = 0; jx < BOOTFLASH_SWITCH_TEST_LEN; jx++) {
            printf("%x ",buf_rd1[jx]);
        }
        printf("\n size = %d\n",jx);
    }

    /*
     * 12 - compare data in bootflash#1
     */
    p1 = buf_wr1;
    p2 = buf_rd1;
    prpass(testpass, "Comparing data , ");
    for (ib = 0; ib < sizeof(buf_rd1); ib++, p1++, p2++) {
        if (*p1 != *p2) {
			close(devfd);           /* don't need it anymore */
            cterr('f', 0, "failed on byte %d, wrote = %02x, read back = %02x", (ib + 1), *p1, *p2);
            printf("Mismatched.\n");
            return (FAILED);
        }
    }

    // switch to bootflash#2
    close(devfd);
    for (ix = 0; ix < 10; ix++) {
        devfd = open(buf, O_RDWR | O_SYNC);
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
        printf("Strerror = %s.", strerror(errno));
        return (FAILED);
    }

    katar_boot_spi_select_control(FALSE, SPICTL_BOOT_UPGRADE);  // switch to bootflash#2
    msleep(BOOT_FLASH_SWITCH_DELAY);

    /*
     * 13 - read back data in bootflash#2 for comparing
     */
    prpass(testpass, "Read back data in bootflash#2 for comparing , ");
    if (lseek(devfd, SECTOR_OFFSET, SEEK_SET) < 0) {
        close(devfd);           /* don't need it anymore */
        cterr('f', 0, "lseek to the beginning of bootflash#2 failed.");
        printf("lseek failed; Cannot point to the beginning of bootflash#2.");
        return (FAILED);
    }

    if ((num = read(devfd, buf_rd2, sizeof(buf_rd2))) == -1) {
        close(devfd);           /* don't need it anymore */
        cterr('f', 0, "Read back data from bootflash#2 failed");
        printf("Unable to read from bootflash#2.\n");
        return (FAILED);
    }
    if (num != sizeof(buf_rd2)) {
        close(devfd);           /* don't need it anymore */
        cterr('f', 0, "not all the bytes are read for data pattern");
        return (FAILED);
    }
    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("\n Read data back from bootflash#2:\n");
        for (jx = 0; jx < BOOTFLASH_SWITCH_TEST_LEN; jx++) {
            printf("%x ",buf_rd2[jx]);
        }
        printf("\n size = %d\n",jx);
    }

    /*
     * 14 - compare data in bootflash#2
     */
    p1 = buf_wr2;
    p2 = buf_rd2;
    prpass(testpass, "Comparing data , ");
    for (ib = 0; ib < sizeof(buf_rd2); ib++, p1++, p2++) {
        if (*p1 != *p2) {
			close(devfd);           /* don't need it anymore */
            cterr('f', 0, "failed on byte %d, wrote = %02x, read back = %02x", (ib + 1), *p1, *p2);
            printf("Mismatched.\n");
            return (FAILED);
        }
    }

    /*
     * 15 - restore data in bootflash#2
     */
    prpass(testpass, "Restore data in bootflash#2, ");
    if (lseek(devfd, SECTOR_OFFSET, SEEK_SET) < 0) {
        close(devfd);           /* don't need it anymore */
        cterr('f', 0, "lseek to the beginning of bootflash#2 failed.");
        return (FAILED);
    }

    if ((num = write(devfd, buf_bk2, sizeof(buf_bk2))) < 0) {
        close(devfd);           /* don't need it anymore */
        cterr('f', 0,
              "Write restore data failed, can not write to bootflash#2.\n");
        return (FAILED);
    }

    if (num != sizeof(buf_bk2)) {
        close(devfd);           /* don't need it anymore */
        cterr('f', 0, "not all the bytes are written for restore");
        return (FAILED);
    }

    // switch to bootflash#1
    close(devfd);
    for (ix = 0; ix < 10; ix++) {
        devfd = open(buf, O_RDWR | O_SYNC);
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
        printf("Strerror = %s.", strerror(errno));
        return (FAILED);
    }

    katar_boot_spi_select_control(FALSE, SPICTL_BOOT_GOLDEN);  // switch to bootflash#1
    msleep(BOOT_FLASH_SWITCH_DELAY);

    /*
     * 16 - restore data in bootflash#1
     */
    prpass(testpass, "Restore data in bootflash#1, ");
    if (lseek(devfd, SECTOR_OFFSET, SEEK_SET) < 0) {
        close(devfd);           /* don't need it anymore */
        cterr('f', 0, "lseek to the beginning of bootflash#1 failed.");
        return (FAILED);
    }

    if ((num = write(devfd, buf_bk1, sizeof(buf_bk1))) < 0) {
        close(devfd);           /* don't need it anymore */
        cterr('f', 0,
              "Write restore data failed, can not write to bootflash#1.\n");
        return (FAILED);
    }

    if (num != sizeof(buf_bk1)) {
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
$Log: diag_boot_flash_test.c,v $
Revision 1.2  2019/06/14 05:24:48  mikech2
Collapse katar-branch00 to Main Trunk

Revision 1.1.2.7  2019/06/10 03:47:18  mikech2
Remove platform_fru.h base on PRRQ#4685780 Comment#6

Revision 1.1.2.6  2019/06/10 00:24:13  mikech2
Modify diag_boot_flash_test.c base on PRRQ#4685780 Comment#1

Revision 1.1.2.5  2019/04/24 01:29:00  peteteng
Fix compiler warning

Revision 1.1.2.4  2019/04/23 07:10:20  peteteng
Add bootflash switch test

Revision 1.1.2.3  2019/03/27 03:21:03  peteteng
Fix bootflash test issue

Revision 1.1.2.2  2019/02/12 08:06:28  mikech2
rename katar_*.h files

Revision 1.1.2.1  2018/10/22 08:02:19  mikech2
Move project folder to common/src/katar/x86

Revision 1.1.2.3  2018/10/11 09:09:14  peteteng
Skip bootflash test for now

Revision 1.1.2.2  2018/06/29 07:17:31  mikech2
Remove compile warning and unused files

Revision 1.1.2.1  2018/06/11 07:05:53  peteteng
add bootflash test from viper

Revision 1.1.2.7  2018/04/20 13:17:45  harrchan
Modify FPGA register according register map

Revision 1.1.2.6  2018/04/20 10:07:26  harrchan
Modify FPGA register according register map

Revision 1.1.2.5  2018/04/09 02:38:22  lucywang
Reverted diag_boot_flash_test.c

Revision 1.1.2.4  2018/04/09 02:34:50  lucywang
Added System Intermation

Revision 1.1.2.3  2018/04/02 11:44:07  lucywang
Modified Boot Flash test, re-open the mtd device after write pattern

Revision 1.1.2.2  2018/03/29 04:17:21  lucywang
Added test message for Boot Flash Test

Revision 1.1.2.1  2018/02/27 08:06:31  harrchan
Initial viper application code base



$Endlog$
*/
