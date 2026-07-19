 /* $Id: diag_boot_flash_test.c,v 1.2 2019/12/11 10:10:27 lucywang Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/nanook/diag_boot_flash_test.c,v $
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
#include "dash_fpga.h"
#include "platform_fru.h"
#include "platform_cookie.h"
#include "plat_defs.h"
#include "diag_storage_lib.h"
#include "diag_boot_flash_test.h"
#include "diag_cpld_lib.h"


/*
 * Global variables
 */
extern int cpld_read_reg(uint, uint32_t *);
extern int cpld_write_reg(uint, uint);

/* Local functions */
int diag_boot_flash_test(int);
int build_boot_flash_menu(boolean);
int mtd_bootflash_test(char *);


/*
 * Sub Menu used for "Boot flash test -> Boot flash submenu test"
 */
submenu_xtable_t boot_flash_submenu_table[] = {
    {"Primary Bootflash Test",
     (PFT) diag_boot_flash_test, FIRST_BOOTFLASH,
     MF_CONTINUOUS | MF_DOGRP | MF_DOALL,
     (type_t(*)())0, 0, (PFT) 0, 0},

    {"Secondary Bootflash Test",
     (PFT) diag_boot_flash_test, SECONDARY_BOOTFLASH,
     MF_CONTINUOUS | MF_DOGRP | MF_DOALL,
     (type_t(*)())0, 0, 
     (PFT) 0, 0},
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
 * Outputs    : PASSED
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
    uint reg_addr;
    uint32_t  reg_bk;
    uint wr_data;
    
    reg_addr = FPGA_LPC_SPI_CTRL_REG;
    cpld_read_reg(reg_addr, &reg_bk);
    

    if (select == FIRST_BOOTFLASH) {
        wr_data = ENABLE_FIRST_BOOTFLASH;
        testname("%s", tname[0]);
    } else if (select == SECONDARY_BOOTFLASH) {
        wr_data = ENABLE_SECONDARY_BOOTFLASH;
        testname("%s", tname[1]);
    } else {
        cterr('f','0',"Wrong index of select bootflash\n");
        return (FAILED);
    }

    /*
     * Uses fpga to mux the bootflash
     */
    cpld_write_reg(reg_addr, wr_data);
    prpass(testpass, "switch flash done");
    if (mtd_bootflash_test(BOOT_FLASH_DEV_PATH) != PASSED) {
        cterr('f', 0, "SPI bootflash test failed.");
        return (FAILED);
    }
    prpass(testpass, "Boot Flash read/write test passed, ");
    prcomplete(testpass, errcount, (char*)0);
    /* Restore mux setting*/ 
    cpld_write_reg(reg_addr, reg_bk);
    return (PASSED);
}

/*******************************************************************************
 *
 * Function   :    mtd_bootflash_test
 * Description:    main test for bootflash test
 * Inputs     :    file path to SPI flash device
 * Outputs    :    PASSED or FAILED.
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
    
    /* Open device file descriptor*/
    for (ix = 0; ix < DEV_OPEN_RETRY; ix++) {
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
     * back up data
     */
    prpass(testpass, "Backup data , ");

    if (lseek(devfd, SECTOR_OFFSET, SEEK_SET) < 0) {
        close(devfd);           /* don't need it anymore */
        cterr('f', 0, "lseek to the beginning of device failed.");
        printf("backup lseek failed; Cannot point to the beginning of device.");
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
        for (jx = 0; jx < DEBUG_INDEX; jx++) {
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
        printf("write lseek failed; Cannot point to the beginning of device.");
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


    /* Close file descriptor and reopen it. This will ensure the read back data
     * is realistic one */
    close(devfd);
    for (ix = 0; ix < DEV_OPEN_RETRY; ix++) {
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
    /* Synchronize a file's in-core state with storage device */
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
Revision 1.2  2019/12/11 10:10:27  lucywang
Merged Nanook to main trunk


$Endlog$
*/
