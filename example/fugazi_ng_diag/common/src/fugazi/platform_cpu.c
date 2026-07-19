/* $Id: platform_cpu.c,v 1.2 2021/06/02 08:22:35 iachang Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/fugazi/platform_cpu.c,v $
 *------------------------------------------------------------------
 *
 * platform_cpu.c : Platform CPU core test functions.
 *
 * Copyright (c) 2018-2020 by cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include "common.h"
#include "error.h"
#include "queryflags.h"
#include "nvsysvars.h"
#include "menu.h"
#include "platform_fru.h"
#include "linux_coretest.h"
#include "platform_cpu.h"
#include "platform_tam_cookie.h"


/*
 * Declare external function
 */
extern int do_all_menu_items(struct menuinfo *);
int quiet_launch = 0;

/*
 * CPU Test Menu
 */

static submenu_xtable_t cpu_test_table[] = {
    {"CPU core test", (type_t(*)())cpu_core_test, TRUE,
     MF_CONTINUOUS | MF_DOALL | MF_SHOW_ERRCOUNT,
      (type_t(*)())0, 0, (type_t(*)())0, 0},
};

#define CPU_TEST_TABLE_SZ \
        (sizeof(cpu_test_table) / sizeof(submenu_xtable_t))

/*
 * primary & secondary submenu items (filled in from xtable)
 */
static mitem_t cpu_pri_test_items[CPU_TEST_TABLE_SZ + MAX_BASE_ITEMS];
static mitem_t cpu_sec_test_items[CPU_TEST_TABLE_SZ + MAX_BASE_ITEMS];

static menuinfo_t cpu_test_menu = {
    "CPU Test Menu",
    0,                            /* mtparam added by init_empty_menu */
    (PFT)menu_show_dflags,        /* shows major flags */
    0,                            /* use generic prompt */
    0,                            /* size (bumped by add_menu_item() */
    cpu_pri_test_items,
};
static menuinfo_t *cpu_test_menup = &cpu_test_menu;


/*******************************************************************************
 * Function: cpu_core_test
 *
 * Description : Test for stress of cpu core & memory. 
 * Default command: "stress -c 400 -m 2 -t 1".
 *              
 * Inputs: test option - auto or user assign
 *
 * Output: PASSED/FAILED
 ******************************************************************************* 
 */
int cpu_core_test (int do_more_test)
{
    char cmd[100], line[100], *ptr;
    unsigned int cpu_count = 400, mem_count = 2, sec = 1;
    int rc = FAILED;
    char *tname = "CPU core";
    FILE *fp;

    memset(line, 0x0, sizeof(line));

    testname("%s", tname);
    prpass(testpass, "%s, ", tname);
    
    if (linux_cpu_core_test(FUGAZI_CPU_NUM) != PASSED) {
        cterr('f', 0, "%s test failed.", tname);
        return (FAILED);
    }

    if (!quiet_launch) {
        prpass(testpass, "%s stress, ", tname);
    }
    if (do_more_test) {
        cpu_count =
            getdec_answer("\nEnter CPU-bound processes:", 400, 1, 400);
        mem_count =
            getdec_answer("Enter memory allocator process:", 2, 1, 6);
        sec = getdec_answer("Enter test time (seconds):", 1, 1, 60);
    }
    if (!quiet_launch) {
    prpass(testpass, "Starting stress test, ");
	prpass(testpass, "Stress testing (times: %d, process: %d, duration: %d sec.), ",
		   cpu_count, mem_count, sec);
    }
    sprintf(cmd, "stress -c %d -m %d -t %d > %s 2>&1",
            cpu_count, mem_count, sec, CPU_STRESS_LOG);
    system(cmd);

    sprintf(cmd, "cat %s | grep run > %s", CPU_STRESS_LOG, CPU_STRESS_RLT);
    system(cmd);

    fp = fopen(CPU_STRESS_RLT, "r");
    if (fp != NULL) {
        fgets(line, sizeof(line), fp);
        if (do_more_test) {
            printf("\n%s \n", line);
        }
        if (line[0] == '\0') {
            printf("Line[0] == 0\n");
            rc = FAILED; 
        }
        ptr = strstr(line, "successful");
        if (ptr != NULL) {
            rc = PASSED;
        }
        fclose(fp);
    } else {
        cterr('f', 0, "%s test failed.");
        rc = FAILED; 
    }

    /* delete the log file */
    unlink(CPU_STRESS_LOG);
    unlink(CPU_STRESS_RLT);

    if ((rc == PASSED) && (quiet_launch != 1)) {  
        prpass(testpass, "%s test passed, ", tname);
    }

    prcomplete(testpass, errcount, (char *)0);
    return (rc);
}

/**********************************************************************
 *
 * Function: build_cpu_test_menu
 *
 * Description: Build CPU test menu.
 *
 * Inputs: db_test_items_executed - TRUE for do all of tests. FALSE for submenu.
 *
 * Outputs: PASSED/FAILED.
 *
 **********************************************************************
 */
int build_cpu_test_menu (int db_test_items_executed)
{
    int rc = FAILED;
    char *tname = "CPU";

    testname(tname);

    build_primary_submenu(cpu_test_table,
                          CPU_TEST_TABLE_SZ, "CPU",
                          &cpu_test_menup);

    build_secondary_submenu(cpu_test_table,
                            CPU_TEST_TABLE_SZ,
                            cpu_sec_test_items);

    if (db_test_items_executed) {
        do_all_menu_items(&cpu_test_menu);
    } else {
        menu(&cpu_test_menu, cpu_sec_test_items, '\0');
    }

    return (rc);
}

/*-------------------------------------------------
 * $Log: platform_cpu.c,v $
 * Revision 1.2  2021/06/02 08:22:35  iachang
 * CSCvo59196-33 : Merge Fugazi from ASR1K-main-branch to ISR main trunk
 *
 * Revision 1.1.8.2  2020/08/26 02:37:51  iachang
 * Merge Fugazi code into main trunk
 *
 * Revision 1.1.6.3  2020/08/04 08:37:06  iachang
 * Update Copyright to 2020
 *
 * Revision 1.1.6.2  2019/03/14 03:48:36  letsai
 * Initial check in.
 *
 *
 *
 *
 * $Endlog$
 */
