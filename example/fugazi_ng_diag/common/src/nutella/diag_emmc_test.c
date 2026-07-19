/* $Id: diag_emmc_test.c,v 1.4 2019/07/11 12:31:27 alicehua Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/nutella/diag_emmc_test.c,v $
 *------------------------------------------------------------------
 *
 * diag_emmc_test.c - eMMC test wraps.
 *
 *
 * Copyright (c) 2008-2019 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <asm/ioctl.h>
#include <sys/stat.h>
#include <signal.h>
#include <sys/ioctl.h>
#include <pthread.h>
#include <errno.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <inttypes.h>
#include <string.h>
#include <asm/byteorder.h>
#include <linux/mmc/ioctl.h>
#include <linux/fs.h>
#include <unistd.h>
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
#include "diag_fpga.h"
#include "platform_fru.h"
#include "platform_cookie.h"
#include "plat_defs.h"
#include "diag_emmc_test.h"
#include "diag_emmc_util.h"
#include "diag_storage_lib.h"
#include "linux_block_test.h"

/*******************************************************************************
 *                          Function Declaration
 *******************************************************************************
 */

/* Local functions */
static int get_emmc_size(char*, int, char*);
int access_emmc_full_test(void);
int access_emmc_test(int);
int emmc_full_test(int);
int build_emmc_test_menu(boolean);
void build_emmc_utils_menu(void);
int diag_emmc_rw_test(int);
int emmc_tests(int);



static boolean emmc_force_stop = FALSE;
static boolean emmc_full_test_finish = FALSE;

/*
 * Global extern functions
 */
extern int ExecuteCmdbyPopen(char *cmd, char *retBuf, int sizeOfBuf);

/*
 * Sub Menu used for "MB test -> EMMC submenu test"
 */
submenu_xtable_t emmc_submenu_table[] = {
    {"eMMC Utility",
     (PFT) build_emmc_utils_menu, 0,
     0, (type_t(*)())0, 0, 
     (PFT) 0, 0},

    {"eMMC UDA R/W Test",
     (PFT) diag_emmc_rw_test, EMMC_UDA_AREA,
     MF_CONTINUOUS | MF_DOGRP | MF_DOALL,
     (type_t(*)())0, 0, 
     (PFT) 0, 0},

    {"eMMC GPP R/W Test",
     (PFT) diag_emmc_rw_test, EMMC_GPP_AREA,
     MF_CONTINUOUS | MF_DOGRP | MF_DOALL,
     (type_t(*)())0, 0, 
     (PFT) 0, 0},
};

#define EMMC_SUBMENU_TABLE_SIZE (sizeof(emmc_submenu_table) / \
                                     sizeof(submenu_xtable_t))

/*
 * "motherboard test -> emmc test" primary & secondary submenu items (filled in from xtable)
 */
static mitem_t emmc_primary_items[EMMC_SUBMENU_TABLE_SIZE +
                                      MAX_BASE_ITEMS];
static mitem_t emmc_secondary_items[EMMC_SUBMENU_TABLE_SIZE +
                                        MAX_BASE_ITEMS];

menuinfo_t emmc_subtest_menu = {
    "%s Subtest Menu",
    0,                          /* mtparam added by init_empty_menu */
    (PFT) show_endnote,         /* notes missing WICs in combos */
    0,                          /* use generic prompt */
    0,                          /* size (bumped by add_menu_item() */
    emmc_primary_items,
};

menuinfo_t *emmc_submenup = &emmc_subtest_menu;

/*
 * EMMC Utilities Submenu
 */
static submenu_xtable_t emmc_utils_table[] = {
    {"Enable eMMC pSLC mode",
     (PFT)emmc_pslc_fully_enable,              0,
     0,
     (type_t(*) ())0,                                 0,
     (type_t(*) ())0,                                 0},

    {"Show eMMC info",
     (PFT)show_emmc_info,              0,
     0,
     (type_t(*) ())0,                                 0,
     (type_t(*) ())0,                                 0},

    {"eMMC full test",
     (PFT)emmc_full_test,              0,
     0,
     (type_t(*) ())0,                                 0,
     (type_t(*) ())0,                                 0},
};

#define EMMC_UTILS_TABLE_SIZE (sizeof(emmc_utils_table) / sizeof(submenu_xtable_t))

/*
 * Primary & secondary submenu items (filled in from xtable)
 */
static mitem_t emmc_utils_primary_items[EMMC_UTILS_TABLE_SIZE + MAX_BASE_ITEMS];
static mitem_t emmc_utils_secondary_items[EMMC_UTILS_TABLE_SIZE + MAX_BASE_ITEMS];

static struct menuinfo emmc_utils_diag = {
    "EMMC Utilities Submenu",    /* title */
    0,                          /* title string added by init_empty_menu */
    (PFT)menu_show_dflags,      /* shows major flags */
    0,                          /* generic prompt */
    0,                          /* size -- bumped by add_menu_item() */
    emmc_utils_primary_items,
};

static struct menuinfo *emmc_utils_diagp = &emmc_utils_diag;

/*******************************************************************************
 *
 * Function   : build_emmc_utils_menu
 * Description: To build EMMC utilities submenu
 * Inputs     : None
 * Outputs    : None
 *
 *******************************************************************************
 */
void build_emmc_utils_menu (void) {

    build_primary_submenu(emmc_utils_table, EMMC_UTILS_TABLE_SIZE,
                          "EMMC Utilities Submenu", 
                          &emmc_utils_diagp);
    build_secondary_submenu(emmc_utils_table, EMMC_UTILS_TABLE_SIZE,
                            emmc_utils_secondary_items);
    menu(&emmc_utils_diag, emmc_utils_secondary_items, 0);
}

/*******************************************************************************
 *
 * Function   :    get_emmc_size
 * Description:    return emmc size
 * Inputs     :    cmd size & device name
 * Outputs    :    disk size
 *
 *******************************************************************************
 */
static int get_emmc_size (char* sysfilesize, int bufsize, char* dev_name)
{
    char cmd[MAX_COMMAND_LENGTH]={0};

    sprintf(cmd, "fdisk -l 2>/dev/null | grep Disk | grep -i -w %s | awk '{print $5}'", dev_name);
    
    if( (ExecuteCmdbyPopen (cmd, sysfilesize, bufsize)) == 0 ) {
        return (FAILED);
    }

    return (PASSED);
}

/*******************************************************************************
 *
 * Function   :    access_emmc_full_test
 * Description:    main entry for emmc full test
 * Inputs     :    NULL.
 * Outputs    : PASSED or FAILED.
 *
 *******************************************************************************
 */
int access_emmc_full_test (void)
{
    int rtn = PASSED;
    char *tname;

    tname = "eMMC full";
    testname(tname);

    if (access_emmc_test(EMMC_GPP_AREA)) {
        cterr('f',0, "eMMC GPP full test failed");
        rtn = FAILED;
    }

    if (access_emmc_test(EMMC_UDA_AREA)) {
        cterr('f',0, "eMMC UDA full test failed");
        rtn = FAILED;
    }

    emmc_full_test_finish = TRUE;
    return (rtn);
}

/*******************************************************************************
 *
 * Function   :    access_emmc_test
 * Description:    main test for emmc test
 * Inputs     :    file path to emmc
 * Outputs    : PASSED or FAILED.
 *
 *******************************************************************************
 */
int access_emmc_test (int option)
{
    char buf[EMMC_TEST_PATTERN_SIZE], buf_bk[EMMC_TEST_BUFFER_SIZE];
    char buf_wr[EMMC_TEST_BUFFER_SIZE], buf_rd[EMMC_TEST_BUFFER_SIZE];
    int buf_bk_len = 0, buf_wr_len = 0, buf_rd_len = 0;
    char *p1 = buf_wr;
    char *p2 = buf_rd;
    int devfd, num, ib;
    int ix, cnt = 0;
    unsigned long pos = 0, size = EMMC_TEST_BUFFER_SIZE;
    char filesize[64]={0};
    int loop_cnt = 0, loop_max = 0;
    char src[32];
    char *tname;

    if (emmc_force_stop == TRUE) {
        return (PASSED);
    }

    buf_bk_len = sizeof(buf_bk);
    buf_wr_len = sizeof(buf_wr);
    buf_rd_len = sizeof(buf_rd);
	
    memset(buf_bk, 0, buf_bk_len);
    memset(buf_wr, 0, buf_wr_len);
    memset(buf_rd, 0, buf_rd_len);
    

    if (option == EMMC_UDA_AREA) {
        printf("\n");
        tname = "eMMC UDA";
        testname(tname);
        prpass(testpass, "%s full test, ", tname);
        sprintf(src, EMMC_UDA_BLK);
    } else if(option == EMMC_GPP_AREA) {
        printf("\n");
        tname = "eMMC GPP";
        testname(tname);
        prpass(testpass, "%s full test, ", tname);
        sprintf(src, EMMC_GPP_BLK);
    } else {
        return (FAILED);
    }

    if (((get_emmc_size (filesize, sizeof(filesize), src))== FAILED)) {
        cterr('f',0,"Failed to get storage size.");
        return (FAILED);
    }
    if (sscanf(filesize, "%lu\n", &size) != 1) {
        cterr('f',0,"Failed to get storage size.");
        return (FAILED);
    }
    loop_max = (size-buf_bk_len)/buf_bk_len;
    printf("test size %lu bytes\n", size);

    sprintf(buf, "%s", src);

    for (ix = 0; ix < OPEN_DEVFD_RETRY; ix++) {
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
     * prepare data pattern
     */
    for (cnt = 0; cnt < buf_wr_len; cnt++) {
        buf_wr[cnt] = PATTERN + cnt;
    }
	
    prpass(testpass, "test position ");
    while (loop_cnt <= loop_max && emmc_force_stop == FALSE) {
	
        if (loop_cnt % 9 == 0)
            prpass(testpass, "%lu ", pos);
		
        /*
         * back up data
         */
        if (lseek(devfd, pos, SEEK_SET) < 0) {
            close(devfd);           /* don't need it anymore */
            cterr('f', 0, "lseek to the beginning of device failed.");
            return (FAILED);
        }
        if ((num = read(devfd, buf_bk, buf_bk_len)) == -1) {
            close(devfd);           /* don't need it anymore */
            cterr('f', 0, "Read data from device failed");
            return (FAILED);
        }

        /*
         * write data pattern
         */
        if (lseek(devfd, pos, SEEK_SET) < 0) {
            close(devfd);           /* don't need it anymore */
            cterr('f', 0, "lseek to the beginning of device failed.");
            return (FAILED);
        }

        if ((num = write(devfd, buf_wr, buf_wr_len)) < 0) {
            close(devfd);           /* don't need it anymore */
            cterr('f', 0, "Write test pattern failed, can not write to drive.");
            return (FAILED);
        }
        if (num != buf_bk_len) {
            close(devfd);           /* don't need it anymore */
            cterr('f', 0, "not all the bytes are written for data pattern");
            return (FAILED);
        }

        if (fsync(devfd) < 0) {
            close(devfd);           /* don't need it anymore */
            cterr('f', 0, "fsync failed.");
            return (FAILED);
        }

        /*
         * read back data for comparing
         */
        if (lseek(devfd, pos, SEEK_SET) < 0) {
            close(devfd);           /* don't need it anymore */
            cterr('f', 0, "lseek to the beginning of device failed.");
            return (FAILED);
        }

        if ((num = read(devfd, buf_rd, buf_rd_len)) == -1) {
            close(devfd);           /* don't need it anymore */
            cterr('f', 0, "Read back data from device failed");
            return (FAILED);
        }
        if (num != buf_rd_len) {
            close(devfd);           /* don't need it anymore */
            cterr('f', 0, "not all the bytes are read for data pattern");
            return (FAILED);
        }

        /*
         * comparing data
         */
        p1 = buf_wr;
        p2 = buf_rd;
        for (ib = 0; ib < buf_rd_len; ib++, p1++, p2++) {
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
        if (lseek(devfd, pos, SEEK_SET) < 0) {
            close(devfd);           /* don't need it anymore */
            cterr('f', 0, "lseek to the beginning of device failed.");
            return (FAILED);
        }

        if ((num = write(devfd, buf_bk, buf_bk_len)) < 0) {
            close(devfd);           /* don't need it anymore */
            cterr('f', 0, "Write restore data failed, can not write to drive.\n");
            return (FAILED);
        }

        if (num != buf_bk_len) {
            close(devfd);           /* don't need it anymore */
            cterr('f', 0, "not all the bytes are written for restore");
            return (FAILED);
        }

        if (fsync(devfd) < 0) {
            close(devfd);           /* don't need it anymore */
            cterr('f', 0, "fsync failed.");
            return (FAILED);
        }
        
        pos += buf_bk_len;
        loop_cnt++;
	}
	if (loop_cnt > loop_max) {
        printf("Done\n");
    }
	
    close(devfd);               /* don't need it anymore */
    return (PASSED);

}

/*******************************************************************************
 *
 * Function   : emmc_wait_answer
 * Description: wait user type 'y' to stop emmc full test during testing.
 * Inputs     : N/A
 * Outputs    : N/A
 *
 *******************************************************************************
 */
static void emmc_wait_answer (void)
{
    while (getc_answer("It may take several hours to test full eMMC, you can "
                       "stop at any time by input y [n]:",
                       "yn", 'n') != 'y');
    emmc_force_stop = TRUE;
}

/*******************************************************************************
 *
 * Function   : emmc_full_test
 * Description: main test for emmc full test.
 * Inputs     : option for future use
 * Outputs    : N/A
 *
 *******************************************************************************
 */
int emmc_full_test (int option)
{
    pthread_t emmc_thread, answer_thread;
    int flag = 1;
	
    emmc_force_stop = FALSE;
    emmc_full_test_finish = FALSE;

    if (pthread_create(&emmc_thread, NULL, (void *)access_emmc_full_test, (void *)&flag)) {
        cterr('f',0, "pthread_create failed(emmc_thread)");
        return (FAILED);
    }

    if (pthread_create(&answer_thread, NULL, (void *)emmc_wait_answer, (void *)&flag)) {
        cterr('f',0, "pthread_create failed(answer_thread)");
        pthread_cancel(emmc_thread);
        return (FAILED);
    }
    
    while (emmc_full_test_finish == FALSE) {
        sleep(EMMC_CHECK_TEST_FINISH_DELAY);
    };

    pthread_cancel(answer_thread);
    pthread_join(emmc_thread, NULL);

    return (PASSED);
}

/*******************************************************************************
 *
 * Function   : build_emmc_test_menu
 * Description:Build emmc test menu 
 * Inputs     : Test/Menu 
 * Outputs    : PASSED
 *
 *******************************************************************************
 */
int build_emmc_test_menu (boolean mb_temp_test_items_executed)
{
    char *tname = "eMMC Test";
    testname(tname);

    build_primary_submenu(emmc_submenu_table, EMMC_SUBMENU_TABLE_SIZE,
                          "eMMC test", &emmc_submenup);
    build_secondary_submenu(emmc_submenu_table, EMMC_SUBMENU_TABLE_SIZE,
                            emmc_secondary_items);
    if (mb_temp_test_items_executed) {
        menu(&emmc_subtest_menu, emmc_secondary_items, 0);
    } else {
        do_all_menu_items(emmc_submenup);
    }
    return (PASSED);
}

/******************************************************************************
 *
 * Function: diag_emmc_rw_test
 *
 * Description: emmc read write test 
 *
 * Inputs      : opt - option for UDA or GPP partition
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/
int diag_emmc_rw_test (int opt)
{
    int rc = FAILED;
    char *tname[] = {"eMMC UDA", "eMMC GPP"}; 
    uchar mb_get_loc[FRU_SIZE] = {0};
    uchar mb_get_pid[FRU_SIZE] = {0};
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
    cterr_add_component("Intel Denverton SOC C3558", "SDIO/MMC", 
                        "eMMC Flash");

    /* Segment 5: register and memory dump */

    /* Segment 6: Platform Environment initialized here*/
    /* Segment 7: Top 3 Debugging Steps */
    cterr_add_debug("Check the interface between the Host SoC"
                    "and the eMMC","If there is no problem for"
                    " these interfaces, replace one eMMC and "
                    "redo the test");
    testname("%s", tname[opt]);

    prpass(testpass, "%s read/write, ", tname[opt]);
    rc = emmc_tests(opt);
    if (rc == FAILED) {
        cterr('f', 0, "%s test failed.", tname[opt]);
        return (rc);
    }
    prpass(testpass, "%s read/write test passed, ", tname[opt]);
    prcomplete(testpass, errcount, (char *)0);
    return (rc);
}

/*******************************************************************************
 *
 * Function   : emmc_tests
 * Description: main test for emmc test.
 * Inputs     : option for UDA or GPP
 * Outputs    : PASSED or FAILED.
 *
 *******************************************************************************
 */
int emmc_tests (int option)
{
    char src[32];
    int retval;

    if (option == EMMC_UDA_AREA) {
        sprintf(src, EMMC_UDA_BLK);
    } else if (option == EMMC_GPP_AREA) {
        sprintf(src, EMMC_GPP_BLK);
    } else {
        return(FAILED);
    }
    retval = linux_block_test(src, 0, EMMC_TEST_LEN, 
                              BLOCK_TEST_RANDOM, TRUE);
    return (retval);
}


/*-------------------------------------------------
$Log: diag_emmc_test.c,v $
Revision 1.4  2019/07/11 12:31:27  alicehua
Collapse Nutella codes into main trunk.

$Endlog$
*/
