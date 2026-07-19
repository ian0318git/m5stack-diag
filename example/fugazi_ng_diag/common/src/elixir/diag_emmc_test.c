/* $Id: diag_emmc_test.c,v 1.2 2021/09/24 01:21:06 harrchan Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/elixir/diag_emmc_test.c,v $
 *------------------------------------------------------------------
 *
 * diag_emmc_test.c
 *
 * Copyright (c) 2018 - 2019 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------------------
 */

#include "error.h"
#include "common.h"
#include "types.h"
#include "menu.h"
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <pthread.h>
#include <asm/ioctl.h>
#include <sys/stat.h>
#include <signal.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <inttypes.h>
#include <string.h>
#include <asm/byteorder.h>
#include <linux/mmc/ioctl.h>
#include <linux/fs.h>
#include <unistd.h>
#include "common_utils.h"
#include "nvmonvars.h"
#include "proto.h"
#include "error.h"
#include "diag_moka_fpga_lib.h"
#include "platform_cookie.h"
#include "diag_enhance_err_msg_lib.h"
#include "diag_temp_sensor_util.h"
#include "diag_emmc_lib.h"
#include "diag_emmc_util.h"
#include "diag_emmc_test.h"
#include "linux_main.h"


static int access_emmc_test (int);
static int emmc_slot_tests (void);
/*******************************************************************************
 *                                Globals
 *******************************************************************************
 */
boolean emmc_force_stop = FALSE;

/******************************************************************************
 *                                 Menus
 ******************************************************************************/
/*
 * eMMC Diag menu
 */
static submenu_xtable_t emmc_diag_tbl[] = {
    {"eMMC utility",               (PFT)emmc_util_entry,          0,
     0, 
     (type_t(*)())0,               0,
     (type_t(*)())0,               0},
    {"eMMC R/W test",              (PFT)diag_emmc_rw_test,             0,
     (MF_CONTINUOUS | MF_DOGRP | MF_DOALL | MF_SHOW_ERRCOUNT),
     (type_t(*)())0,               0,
     (type_t(*)())0,               0},
    {"eMMC R/W test(full size)",   (PFT)diag_emmc_full_size_rw_test,   0,
     (MF_CONTINUOUS | MF_DOGRP | MF_SHOW_ERRCOUNT),
     (type_t(*)())0,               0,
     (type_t(*)())0,               0},
};

#define EMMC_DIAG_TBL_SIZE (sizeof(emmc_diag_tbl) / sizeof(submenu_xtable_t))

/*
 * Primary & secondary submenu items (filled in from xtable)
 */
static mitem_t emmc_diag_menu_pri_items[EMMC_DIAG_TBL_SIZE + MAX_BASE_ITEMS];
static mitem_t emmc_diag_menu_sec_items[EMMC_DIAG_TBL_SIZE + MAX_BASE_ITEMS];

static menuinfo_t emmc_diag_menu = {
    "%s Diag Menu",
    0,                          /* mtparam added by init_empty_menu */
    (PFT) show_endnote,         /* notes missing WICs in combos */
    0,                          /* use generic prompt */
    0,                          /* size (bumped by add_menu_item() */
    emmc_diag_menu_pri_items,
};

menuinfo_t *emmc_diag_menu_p = &emmc_diag_menu;


/*******************************************************************************
 *
 * Function   : diag_emmc_test
 * Description: Entry function of eMMC Diag.
 * Inputs     : exe_all_testmenu - To decide whether to show test menu(TRUE/FALSE)
 *                                 or do all related tests directly
 * Outputs    : None
 *
 *******************************************************************************
 */
void diag_emmc_test (boolean exe_all_testmenu)
{
    build_primary_submenu(emmc_diag_tbl, EMMC_DIAG_TBL_SIZE,
                          "eMMC", &emmc_diag_menu_p);
    build_secondary_submenu(emmc_diag_tbl, EMMC_DIAG_TBL_SIZE,
                            emmc_diag_menu_sec_items);

    if (exe_all_testmenu == TRUE) {
        do_all_menu_items(emmc_diag_menu_p);
    } else {
        menu(&emmc_diag_menu, emmc_diag_menu_sec_items, 0);
    }
}

/*******************************************************************************
 *
 * Function   : diag_emmc_rw_test
 * Description: Function to do eMMC R/W test.
 * Inputs     : None
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
int diag_emmc_rw_test (void)
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
    cterr_add_component("Marvell Armada 7040", "SDIO/MMC", "eMMC Storage Flash");

    /* Segment 5: register and memory dump */

    /* Segment 6: Platform Environment initialized here*/
    cterr_add_env_dump((PFV)show_plat_curr_temps);

    /* Segment 7: Top 3 Debugging Steps */
    cterr_add_debug("Check the interface between the Host SoC and the eMMC.",
                    "If there is no problem for these interfaces, "
                    "replace one eMMC and redo the test.");
#endif

    char *curr_testname = "eMMC R/W";

    testname(curr_testname);
    prpass(testpass, "%s, ", curr_testname);

    if (emmc_slot_tests() != PASSED) {
        cterr('f', 0, "%s test failed.", curr_testname);
        return (FAILED);
    }
    return (PASSED);
}

/*******************************************************************************
 *
 * Function   : diag_emmc_full_size_rw_test
 * Description: Function to R/W test whole eMMC.
 * Inputs     : None
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
int diag_emmc_full_size_rw_test (void)
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
    cterr_add_component("Marvell Armada 7040", "SDIO/MMC", "eMMC Storage Flash");

    /* Segment 5: register and memory dump */

    /* Segment 6: Platform Environment initialized here*/
    cterr_add_env_dump((PFV)show_plat_curr_temps);

    /* Segment 7: Top 3 Debugging Steps */
    cterr_add_debug("Check the interface between the Host SoC and the eMMC.",
                    "If there is no problem for these interfaces, "
                    "replace one eMMC and redo the test.");
#endif

    int retval = 1;
    pthread_t emmc_thread;
    void *ret;
    int flag = 1;
	
    emmc_force_stop = FALSE;
    if (pthread_create(&emmc_thread,
                      NULL,
                      (void *)access_emmc_test,
                      (void *)&flag)) {
        cterr('f',0, "pthread_create failed");
        return (FAILED);
    }
    
    while(getc_answer("It may take several hours to test full eMMC, "
                      "you can stop at any time.\nDo you want to stop eMMC test?"
                      " Please input y to confirm.", "yn", 'n') != 'y');

    emmc_force_stop = TRUE;
    pthread_join(emmc_thread, &ret);

    return (retval);
}

/*******************************************************************************
 *
 * Function   : emmc_slot_tests
 * Description: main test for emmc test.
 * Inputs     : None
 * Outputs    : PASSED or FAILED.
 *
 *******************************************************************************
 */
static int emmc_slot_tests (void)
{
    int retval = FAILED;

    emmc_force_stop = FALSE;

    retval = access_emmc_test(0);
    return (retval);
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
static int access_emmc_test (int full_test)
{
    char buf[EMMC_TEST_PATTERN_SIZE], buf_bk[EMMC_TEST_BUFFER_SIZE], buf_wr[EMMC_TEST_BUFFER_SIZE], buf_rd[EMMC_TEST_BUFFER_SIZE];
    int buf_bk_len = 0, buf_wr_len = 0, buf_rd_len = 0;
    char *p1 = buf_wr;
    char *p2 = buf_rd;
    int devfd, num, ib;
    int cnt = 0;
    unsigned long pos = 0, size = EMMC_TEST_BUFFER_SIZE;
    char filesize[64]={0};
    int loop_cnt = 0, loop_max = 0;

    buf_bk_len = sizeof(buf_bk);
    buf_wr_len = sizeof(buf_wr);
    buf_rd_len = sizeof(buf_rd);
	
    memset(buf_bk, 0, buf_bk_len);
    memset(buf_wr, 0, buf_wr_len);
    memset(buf_rd, 0, buf_rd_len);
    
    if (full_test) {
		if (((get_emmc_size (filesize, sizeof(filesize), EMMC_BLK))== FAILED)) {
			cterr('f',0,"Failed to get storage size.");
			return (FAILED);
		}
		if (sscanf(filesize, "%lu\n", &size) != 1) {
			cterr('f',0,"Failed to get storage size.");
			return (FAILED);
		}
		loop_max = (size-buf_bk_len)/buf_bk_len;
		printf("test size %lu bytes\n", size);
	}	

    if (!quiet_launch && !full_test) {
        prpass(testpass, "Access device '%s' , ", EMMC_BLK);
    }
    sprintf(buf, "%s", EMMC_BLK);

    sleep(1);
    devfd = open(buf, O_RDWR);
    if (devfd < 0) {
        cterr('f', 0, "%s:%d:open file:%s fail", __FUNCTION__, __LINE__, buf);
        return (FAILED);
    }
    
    /*
     * prepare data pattern
     */
    if (!quiet_launch && !full_test) {
        prpass(testpass, "Prepare data pattern , ");
    }
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
		if (!quiet_launch && !full_test) {
			prpass(testpass, "Backup data , ");
		}
		if (lseek(devfd, pos, SEEK_SET) < 0) {
			close(devfd);           /* don't need it anymore */
			cterr('f', 0, "lseek to the beginning of device failed.");
			printf
				("backup lseek failed; Cannot point to the beginning of device.");
			return (FAILED);
		}
		if ((num = read(devfd, buf_bk, buf_bk_len)) == -1) {
			close(devfd);           /* don't need it anymore */
			cterr('f', 0, "Read data from device failed");
			printf("Unable to read from drive.\n");
			return (FAILED);
		}

		/*
		 * write data pattern
		 */
		if (!quiet_launch && !full_test) {
			prpass(testpass, "Write data pattern , ");
		}
		if (lseek(devfd, pos, SEEK_SET) < 0) {
			close(devfd);           /* don't need it anymore */
			cterr('f', 0, "lseek to the beginning of device failed.");
			printf
				("write lseek failed; Cannot point to the beginning of device.");
			return (FAILED);
		}

		if ((num = write(devfd, buf_wr, buf_wr_len)) < 0) {
			close(devfd);           /* don't need it anymore */
			cterr('f', 0,
				  "Write test pattern failed, can not write to drive.");
			printf("Unable to write data pattern to device.");
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
			printf("Unable to sync data pattern to device.");
			return (FAILED);
		}

		/*
		 * read back data for comparing
		 */
		if (!quiet_launch && !full_test) {
			prpass(testpass, "Read back data for comparing , ");
		}
		if (lseek(devfd, pos, SEEK_SET) < 0) {
			close(devfd);           /* don't need it anymore */
			cterr('f', 0, "lseek to the beginning of device failed.");
			printf("lseek failed; Cannot point to the beginning of device.");
			return (FAILED);
		}

		if ((num = read(devfd, buf_rd, buf_rd_len)) == -1) {
			close(devfd);           /* don't need it anymore */
			cterr('f', 0, "Read back data from device failed");
			printf("Unable to read from drive.\n");
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
		if (!quiet_launch && !full_test) {
			prpass(testpass, "Comparing data , ");
		}
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
		if (!quiet_launch && !full_test) {
			prpass(testpass, "Restore data , ");
		}
		if (lseek(devfd, pos, SEEK_SET) < 0) {
			close(devfd);           /* don't need it anymore */
			cterr('f', 0, "lseek to the beginning of device failed.");
			return (FAILED);
		}

		if ((num = write(devfd, buf_bk, buf_bk_len)) < 0) {
			close(devfd);           /* don't need it anymore */
			cterr('f', 0,
				  "Write restore data failed, can not write to drive.\n");
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
	
    close(devfd);               /* don't need it anymore */
    return (PASSED);

}

/*-------------------------------------------------
 * $Log: diag_emmc_test.c,v $
 * Revision 1.2  2021/09/24 01:21:06  harrchan
 * Collapse Elixir-branch to Main Trunk.
 *
 * Revision 1.1.2.3  2021/01/07 06:18:12  illiu
 * Modify error messgage bug of eMMC test item
 *
 * Revision 1.1.2.2  2020/09/14 05:49:44  harrchan
 * Remove DSL and GSHDSL relevant part
 *
 * Revision 1.1.2.1  2020/09/09 09:08:06  illiu
 * First version which has been ported with Dreamliner and Marvel CPSS
 *
 * Revision 1.2  2019/01/10 06:36:22  wilbhuan
 * The beginning of Betelgeuse application code.
 *
 *-------------------------------------------------
 */
