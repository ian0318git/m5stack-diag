 /* $Id: platform_cpu.c,v 1.2 2018/08/06 02:31:52 harrchan Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/viper/platform_cpu.c,v $
 *------------------------------------------------------------------
 *
 * platform_cpu.c
 *
 * Copyright (c) 2018 by cisco Systems, Inc.
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
#include "platform_cpu.h"


#define CPU_STRESS_LOG   "/tmp/cpu_core_log"
#define CPU_STRESS_RLT   "/tmp/cpu_core_rlt"
#define ENHANCE_ERROR_MSG_RDY 1


/*
 * Declare external function
 */
extern int do_all_menu_items(struct menuinfo *);

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
    unsigned int cpu_count = 400, mem_count = 2, sec = 5;
    int rc = FAILED;
    char *tname = "CPU core";
    FILE *fp;

    memset(line, 0x0, sizeof(line));

    testname("%s", tname);
    prpass(testpass, "%s, ", tname);
    prpass(testpass, "%s stress, ", tname);
    
    if (do_more_test) {
        /* Spawn N workers spining on sqrt()*/
        cpu_count =
            getdec_answer("\nEnter CPU-bound processes:", 400, 1, 400);
        /* Spawn N workers spinning on malloc()/free() */
        mem_count =
            getdec_answer("Enter memory allocator process:", 2, 1, 6);
        sec = getdec_answer("Enter test time (seconds):", 1, 1, 60);
    }
    prpass(testpass, "Starting stress test, ");
	prpass(testpass, "Stress testing (times: %d, process: %d, duration: %d sec.), ",
		   cpu_count, mem_count, sec);
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

    if ((rc == PASSED)) {  
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
$Log: platform_cpu.c,v $
Revision 1.2  2018/08/06 02:31:52  harrchan
Merge viper E2E to the main trunk (CSCvk28469)

Revision 1.1.2.2  2018/07/03 05:38:55  harrchan
Follow the coding rule to clean up code

Revision 1.1.2.1  2018/03/28 09:18:13  lucywang
Added CPU test


*/
