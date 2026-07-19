/* $Id: diag_cpu_test.c,v 1.2 2019/01/10 06:36:21 wilbhuan Exp $ 
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/betelgeuse/diag_cpu_test.c,v $
 *------------------------------------------------------------------
 * 
 * diag_cpu_test.c
 *
 * Copyright (c) 2018 - 2019 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/time.h>
#include "common.h"
#include "error.h"
#include "queryflags.h"
#include "nvsysvars.h"
#include "menu.h"
#include "diag_enhance_err_msg_lib.h"
#include "platform_cookie.h"
#include "proto.h"
#include "linux_coretest.h"
#include "diag_temp_sensor_util.h"
#include "diag_cpu_lib.h"
#include "diag_cpu_test.h"
#include "diag_cpu_util.h"
#include "linux_main.h"

/*******************************************************************************
 *                                 Menus
 *******************************************************************************
 */
/*
 * CPU Diag menu
 */
static submenu_xtable_t cpu_diag_table[] = {
    {"CPU Utility",   (type_t(*)())diag_cpu_util,  0,
     0,
     (type_t(*)())0,    0,
     (type_t(*)())0,    0},
    {"CPU core test",   (type_t(*)())diag_cpu_core_test,    TRUE,
     (MF_CONTINUOUS | MF_DOALL | MF_SHOW_ERRCOUNT),
     (type_t(*)())0,    0,
     (type_t(*)())0,    0},
};

#define CPU_DIAG_TABLE_SIZE (sizeof(cpu_diag_table) / sizeof(submenu_xtable_t))

/*
 * primary & secondary submenu items (filled in from xtable)
 */
static mitem_t cpu_diag_menu_pri_items[CPU_DIAG_TABLE_SIZE + MAX_BASE_ITEMS];
static mitem_t cpu_diag_menu_sec_items[CPU_DIAG_TABLE_SIZE + MAX_BASE_ITEMS];

static menuinfo_t cpu_diag_menu = {
    "%s Test Menu",
    0,                            /* mtparam added by init_empty_menu */
    (PFT)menu_show_dflags,        /* shows major flags */
    0,                            /* use generic prompt */
    0,                            /* size (bumped by add_menu_item() */
    cpu_diag_menu_pri_items,
};

static menuinfo_t *cpu_diag_menu_p = &cpu_diag_menu;


/*******************************************************************************
 *
 * Function   : diag_cpu_test
 * Description: Entry function of CPU Diag.
 * Inputs     : exec_opt - "TRUE" to execute all MF_DOALL tests in menu
 *                         "FALSE" to show Test Menu
 * Outputs    : None
 *
 *******************************************************************************
 */
void diag_cpu_test (boolean exec_opt)
{
    build_primary_submenu(cpu_diag_table, CPU_DIAG_TABLE_SIZE,
                          "CPU", &cpu_diag_menu_p);
    build_secondary_submenu(cpu_diag_table, CPU_DIAG_TABLE_SIZE,
                            cpu_diag_menu_sec_items);

    if (exec_opt == TRUE) {
        do_all_menu_items(cpu_diag_menu_p);
    } else {
        menu(&cpu_diag_menu, cpu_diag_menu_sec_items, 0);
    }
}

/*******************************************************************************
 *
 * Function   : diag_cpu_core_test
 * Description: Test to stress CPU and memory. 
 *              (Default command: "stress -c 400 -m 2 -t 1".)
 * Inputs     : autotest_mode - "TRUE" to run test with default config
 *                              "FALSE" to ask user to config. test parameters
 * Outputs    : PASSED/FAILED
 *
 ******************************************************************************* 
 */
int diag_cpu_core_test (boolean autotest_mode)
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
    cterr_add_component("Marvell Armada 7040", "ARM Cores", "L2 Cache");

    /* Segment 5: register and memory dump */
    //cterr_add_reg_dump((PFV)bcm63168_get_xdsl_profile);

    /* Segment 6: Platform Environment initialized here*/
    cterr_add_env_dump((PFV)show_plat_curr_temps);

    /* Segment 7: Top 3 Debugging Steps */
    cterr_add_debug("Reference the kernel messages and contact the CPU vendor.");
#endif

    char cmd[100], line[100], *ptr;
    unsigned int cpu_count = NUMBER_OF_PROCESS, mem_count = 2, sec = 1;
    int ret_val = FAILED;
    char *curr_testname = "CPU core";
    FILE *fp;

    memset( line, 0x0, sizeof(line));

    testname("%s", curr_testname);
    prpass(testpass, "%s, ", curr_testname);

    /* Test for expected and actual CPU core number */
    /* Checking all cores are identified. To open the system file "/proc/cpuinfo" 
     * to check every block of CPU information is well. */
    if (linux_cpu_core_test(CPU_ALL_CORES) != PASSED) {
        cterr('f', 0, "%s test failed.", curr_testname);
        return (FAILED);
    }

    if (!quiet_launch) {
        prpass(testpass, "%s stress, ", curr_testname);
    }

    if (autotest_mode == FALSE) {
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

    /* Performing stress test by stress utility, the test conditions as below:
     * -c: number of process to calculate the square root. (-c = 400)
     * -m: number of process to spinning on malloc/free operation. (-m = 2)
     * -t: timeout (-t = 1 second) */
    sprintf(cmd, "stress -c %d -m %d -t %d > %s 2>&1",
            cpu_count, mem_count, sec, CPU_STRESS_LOG);
    system(cmd);

    sprintf(cmd, "cat %s | grep run > %s", CPU_STRESS_LOG, CPU_STRESS_RLT);
    system(cmd);

    /* Checking test result in result file */
    fp = fopen(CPU_STRESS_RLT, "r");
    if (fp != NULL) {
        fgets(line, sizeof(line), fp);

        if (autotest_mode == TRUE) {
            printf("\n%s \n", line);
        }
        if (line[0] == '\0') {
            printf("Line[0] == 0\n");
            ret_val = FAILED; 
        }
        ptr = strstr(line, "successful");
        if (ptr != NULL) {
            ret_val = PASSED;
        }
        fclose(fp);
    } else {
        cterr('f', 0, "%s test failed.");
        ret_val = FAILED; 
    }

    unlink(CPU_STRESS_LOG);
    unlink(CPU_STRESS_RLT);

    if ((ret_val == PASSED) && (quiet_launch != 1)) {  
        prpass(testpass, "%s test passed, ", curr_testname);
    }
    return (ret_val);
}

/*-------------------------------------------------
 * $Log: diag_cpu_test.c,v $
 * Revision 1.2  2019/01/10 06:36:21  wilbhuan
 * The beginning of Betelgeuse application code.
 *
 *-------------------------------------------------
 */
