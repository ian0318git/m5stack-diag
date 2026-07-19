/* $Id: skye_utils.c,v 1.2 2015/05/25 03:56:16 steja Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/skye_utils.c,v $
 *******************************************************************************
 * File Name: skye_utils.c
 *
 * Description: Skye utilities source file
 *
 * Jul 2014 - Paul Lin(palin2) ported from Shrinkray
 *
 * Original Author: Sofian Teja
 *
 * Copyright (c) 2015 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *******************************************************************************
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <pthread.h>
#include "endians.h"
#include "types.h"
#include "defs.h"
#include "signals.h"
#include "error.h"
#include "nvmonvars.h"
#include "nvsysvars.h"
#include "pm_utils.h"
#include "pci.h"
#include "slot.h"
#include "mon_plat_defs.h"
#include "common.h"
#include "common_utils.h"
#include "menu.h"
#include "proto.h"
#include "queryflags.h"
#include "strings.h"
#include "router_if.h"
#include "sgmii_defs.h"
#include "skye_host.h"


/*******************************************************************************
 *                       Static Functions Declaration
 *******************************************************************************
 */
static void skye_kill_all_nc(void);
static void skye_init_status_file(void);
static int  skye_check_test_status(void);
static void skye_init_dbskyelog_file(void);
static void save_to_log(void);


/*******************************************************************************
 *                          Functions Declaration
 *******************************************************************************
 */
void skye_nc_dispatch_comm(char *, int);


/*******************************************************************************
 *                                Externs
 *******************************************************************************
 */
extern void    skye_get_ip_addr(char *, int);
extern int     skye_test_slot;
extern void    *skye_rx_uart(void *);
extern int     is_skye_uart(int);
extern boolean skye_one_cpu(void);
extern boolean freq_margin;


/*******************************************************************************
 *                             Global Variable
 *******************************************************************************
 */
int         main_thread_wait_time = 0;
static char *vm_setup_tbl[] = {"3.3V", "2.5V", "1.2V", "1.0V", "1.8V",
                               "1.35V_GX0", "1.35V_GX1"};


/*******************************************************************************
 *                                Functions
 *******************************************************************************
 */

/*******************************************************************************
 *
 * Function   : skye_nc_dispatch_comm
 * Description: This function dispatches command to Skye through nc command.
 * Inputs     : comm - command
 *              cpu - for multi cpus
 * Outputs    : None
 *
 *******************************************************************************
 */
void skye_nc_dispatch_comm (char *comm, int cpu)
{
    char           sm_ipaddr[32];
    char           s_cmd[64], w_cmd[64];
    unsigned short flag = (NVRAM)->diagflag;
    unsigned long  xflag = diagflag_xram;
    FILE           *fp, *fmp;
    char buff[10];
    char *fmtoken = NULL;

    /* Sanity check */
    if (comm == NULL) {
        printf("%s: NULL Pointer\n", __FUNCTION__);
        return;
    }

    if ((fp = fopen(DIAG_CMD_DISPATCH_FILE, "a+")) == NULL) {
        printf("%s: Can't open/create file %s\n",
               __FUNCTION__, DIAG_CMD_DISPATCH_FILE);
        return;
    }

    if (freq_margin == TRUE) {
        if ((fmp = fopen(SKYE_TMP_FM_FILE, "r")) == NULL) {
            printf("%s: Can't open/create file %s\n",
               __FUNCTION__, SKYE_TMP_FM_FILE);
            return;
        }

        if (fgets(buff, sizeof(buff), fmp) == NULL) {
            printf("Nothing in buffer\n");
            fclose(fmp);
            return;
        }

        fmtoken = strtok(buff, ":");
        if (strcmp("FM", fmtoken) == 0) {
            /* get FM value */
            fmtoken = strtok(NULL, ";");
        }
    }

    skye_get_ip_addr(sm_ipaddr, cpu);

    /* Prepare command and listen for SM card to grab */

    /* Masked Continuous Flag */
    flag &= (unsigned short)(~D_CONTINUOUS);

    fprintf(fp, "CMD:%s;", comm);
    fprintf(fp, "FLAG:%d;", flag);
    fprintf(fp, "xFLAG:%lu;", xflag);
    if (freq_margin == TRUE) {
        fprintf(fp, "FM:%s;", fmtoken);
    }

    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("\n%s: comm = %s, flag = %d, xflag = %lu.\n",
               __FUNCTION__, comm, flag, xflag);
    }

    fclose(fp);

    sprintf(s_cmd, "nc %s %d < %s &",
            sm_ipaddr,
            DIAG_EXEC_CMD_TRANS_PORT_BASE,
            DIAG_CMD_DISPATCH_FILE);
    system(s_cmd);

    /* Waiting for NC execute finish signal from Skye module */
    sprintf(w_cmd, "nc -l -l -p %d > %s", SKYE_NC_DONE_PORT, SKYE_NC_DONE_FP);
    system(w_cmd);
}

/*******************************************************************************
 *
 * Function   : skye_cpu_alive_check
 * Description: This function check the module cpu a live test through nc command.
 * Inputs     : cpu_num  -   cpu 0 or cpu 1
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
int skye_cpu_alive_check (int cpu_num)
{
    char test_name[32];

    skye_init_status_file();

    sprintf(test_name, "Skye CPU%d Alive check", cpu_num);
    testname(test_name);

    skye_nc_dispatch_comm(DIAG_CMD_ALIVE_CHECK, cpu_num);

    if (skye_check_test_status() == FAILED) {
        printf("Skye CPU%d Alive check test FAILED.\n", cpu_num);
        skye_kill_all_nc(); 
        return (FAILED);
    }

    printf("Skye CPU%d Alive check test PASSED.\n", cpu_num);
    skye_kill_all_nc(); 
    return (PASSED);
}

/*******************************************************************************
 *
 * Function   : skye_init_status_file
 * Description: This function clears out the content of status file
 *              and listen to the port.
 * Inputs     : None
 * Outputs    : None
 *
 *******************************************************************************
 */
static void skye_init_status_file (void)
{
    char cmd[64];
    char file_loc[32];

    sprintf(file_loc, "%s", SKYE_NC_RET_VAL_FILE);
    sprintf(cmd, "echo ' ' > %s", file_loc);
    system(cmd);

    /* Listen to the command status */
    sprintf(cmd, "nc -l -l -p %d > %s &",
            DIAG_RTN_STS_OUT_PORT_BASE, file_loc);
    system(cmd);
}

/*******************************************************************************
 *
 * Function   : skye_check_test_status
 * Description: This function checks the status of status file and
 *              determine whether the test passes or fails.
 * Inputs     : None
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
static int skye_check_test_status (void)
{
    FILE *fp;
    char file_loc[32];
    char buf[32];
    char cmd[32];

    sprintf(file_loc, "%s", SKYE_NC_RET_VAL_FILE);

    fp = fopen(file_loc, "r");
    if (fp == NULL) {
        printf("%s: Unable to open '%s'\n", __FUNCTION__, file_loc);
        return (FAILED);
    }

    if (fgets(buf, sizeof(buf), fp) != NULL) {
        if (strstr(buf, DIAG_RTN_PASS_STR)) {
            if ((NVRAM)->diagflag & D_VERBOSE) {
                printf("Passed !!! & Return Status is:\n");
                printf("%s\n", buf);
            }
            fclose(fp);
            return (PASSED);
        } else {
            printf("Fail! Return Status is %s\n", buf);
            fflush(stdout);
            sprintf(cmd, "cat %s", SKYE_NC_DBLOG_HOST_FILE);
            system(cmd);
            /* DEBUG */
            save_to_log();  /* save to the dblog and errlog */
        }
    }

    fclose(fp);

    printf("%s: Fgets fails\nContent of '%s':\n", __FUNCTION__, file_loc);
    fflush(stdout);
    sprintf(cmd, "cat %s", file_loc);
    system(cmd);

    return (FAILED);
}

/*******************************************************************************
 *
 * Function   : skye_kill_all_nc
 * Description: This function lists all process and grep 'nc -l' keyword,
 *              and dump to temporary directory so we can kill them afterwards.
 * Inputs     : None
 * Outputs    : None
 *
 *******************************************************************************
 */
static void skye_kill_all_nc (void)
{
    char cmd[64];
    char buf[128];
    char pid_file[32];
    char *token;
    int pid;
    FILE *fp;
    struct stat sts;

    /* Clear the content of the file first */
    sprintf(cmd, "echo ' ' > %s", DIAG_KILL_NC_TMP_FILE);
    system(cmd);
    sprintf(cmd, "ps | grep 'nc -l' > %s", DIAG_KILL_NC_TMP_FILE);
    system(cmd);

    fp = fopen(DIAG_KILL_NC_TMP_FILE, "r");
    if (fp == NULL) {
        printf("%s: Open '%s' Failed\n", __FUNCTION__, DIAG_KILL_NC_TMP_FILE);
        return;
    }

    /* Check the result
     */
    while (!feof(fp)) {
        fgets(buf, sizeof(buf), fp);
        token=strtok(buf, " ");
        pid = atoi(token);
        /* Check if this process is still alive */
        sprintf(pid_file, "/proc/%d", pid);
        if (stat(pid_file, &sts) == -1) {
            /* Process doesn't exist */
            continue;
        }
        sprintf(cmd, "kill -9 %d", pid);
        system(cmd);
    }

    /* Clear the skye* files */
    sprintf(cmd, "rm -f %s", "/tmp/skye_*");
    system(cmd);

    fclose(fp);
}

/*******************************************************************************
 *
 * Function   : skye_do_all
 * Description: This function do all module test through nc command.
 * Inputs     : comm - command
 * Outputs    : None
 *
 *******************************************************************************
 */
int skye_do_all (int cpu_num)
{
    char test_name[32];
    char tty_dev[32];
    int uart_fd = 0;
    pthread_t threads;

    skye_init_status_file();
    skye_init_dbskyelog_file();

    sprintf(test_name, "Skye  SM-%d CPU%d do all test", skye_test_slot, cpu_num);
    testname(test_name);

    /*Setup UART */
    if (cpu_num == CPU0) {
        if (is_skye_uart(0) == FAILED) {
            printf("Failed to switch uart 0\n");
            return (FAILED);
        }
    } else { /* CPU 1 */
        if (is_skye_uart(1) == FAILED) {
            printf("Failed to switch uart 1\n");
            return (FAILED);
        }
    }

    /* Setup UART toward Skye board */
    sprintf(tty_dev, "/dev/ttyDASH%d", skye_test_slot - 1);

    uart_fd = open(tty_dev, O_RDWR|O_NOCTTY);

    if (uart_fd < 0) {
        perror("polling uart: open tty failed");
        fflush(stdout);
        return (FAILED);
    }

    if(pthread_create(&threads, NULL, skye_rx_uart, (void *)&uart_fd)) {
        perror("pthread_create failed.");
        printf("%s: pthread_create failed.\n", __FUNCTION__);
        close(uart_fd);
        return FAILED;
    }

    while (--main_thread_wait_time ) {
        skye_nc_dispatch_comm(DIAG_DO_ALL_TEST, cpu_num);

        if (skye_check_test_status() == FAILED) {
            printf("Skye SM-%d CPU%d do all test FAILED.\n", skye_test_slot, cpu_num);
            skye_kill_all_nc();
            if (pthread_cancel(threads)!=0) {
                printf("pthread_cancel error");
            }
            close(uart_fd);
            return (FAILED);
        } else {
            printf("Skye SM-%d CPU%d do all test PASSED.\n", skye_test_slot, cpu_num);
            skye_kill_all_nc();
            break;
        }
    }

    if (main_thread_wait_time == 0) {
        printf("Skye SM-%d CPU%d do all test, wait time out...\n", skye_test_slot, cpu_num);
    }

    if (pthread_cancel(threads)!=0) {
        printf("pthread_cancel error");
    }
    close(uart_fd);

    return (PASSED);
}

/*******************************************************************************
 *
 * Function   : skye_init_dbskyelog_file
 * Description: This function clears out the content of status file
 *              and listen to the port.
 * Inputs     : None
 * Outputs    : None
 *
 *******************************************************************************
 */
static void skye_init_dbskyelog_file (void)
{
    char cmd[64];
    char file_loc[32];

    sprintf(file_loc, "%s", SKYE_NC_DBLOG_HOST_FILE);
    sprintf(cmd, "echo ' ' > %s", file_loc);
    system(cmd);

    /* Listen to the command status */
    sprintf(cmd, "nc -l -l -p %d > %s &",
            DIAG_RTN_DBLOG_PORT_BASE, file_loc);
    system(cmd);
}

/*******************************************************************************
 *
 * Function   : save_to_log
 * Description: This function save the module log to the dblog and errorlog on host
 * 
 * Inputs     : None
 * Outputs    : None
 *
 *******************************************************************************
 */
static void save_to_log (void)
{
    FILE *fp1, *fp2, *fp3;
    char file1_loc[32];
    char file2_loc[32];
    char file3_loc[32];
    char buf[7000];

    sprintf(file1_loc, "%s", SKYE_NC_DBLOG_HOST_FILE);
    sprintf(file2_loc, "%s", "/dblog.txt");
    sprintf(file3_loc, "%s", "/errlog.txt");

    fp1 = fopen(file1_loc, "r");
    fp2 = fopen(file2_loc, "a");
    fp3 = fopen(file3_loc, "a");

    if (fp1 == NULL) {
        printf("%s: Unable to open '%s'\n", __FUNCTION__, file1_loc);
        return;
    }

    if (fp2 == NULL) {
        printf("%s: Unable to open '%s'\n", __FUNCTION__, file2_loc);
        return;
    }

    if (fp3 == NULL) {
        printf("%s: Unable to open '%s'\n", __FUNCTION__, file3_loc);
        return;
    }

    while (fgets(buf, sizeof(buf), fp1) != NULL) {
        buf[strlen(buf)-1] = '\0';
        fprintf(fp2,"%s\n",buf);
        fprintf(fp3,"%s\n",buf);
    }

    fclose(fp1);
    fclose(fp2);
    fclose(fp3);
    return;

}

/*******************************************************************************
 *
 * Function   : save_to_log_utils
 * Description: This function dump the module log
 *
 * Inputs     : None
 * Outputs    : None
 *
 *******************************************************************************
 */
int save_to_log_utils (void)
{
    FILE *errlog;
    char log_info[7000];  /* To allocate 7K buffer log */

    errlog = fopen("/dblog.txt", "r");
    if (errlog == NULL) {
        printf("Failed to open /dblog.txt file.\n");
        return (1);
    }

    while (fgets(log_info, sizeof(log_info), errlog) != NULL) {
        log_info[strlen(log_info)-1] = '\0';
        puts(log_info);
    }

    fclose(errlog);

    return (0);
}

/*******************************************************************************
 *
 * Function   : skye_nc_do_module_test_util
 * Description: This function dispatches command to Skye through nc command.
 * Inputs     : cpu_num - CPU0 or CPU1
 *              comm - command
 * Outputs    : PASSED or FAILED
 *
 *******************************************************************************
 */
int skye_nc_do_module_test_util (int cpu_num, char *comm)
{
    char test_name[32];
    char tty_dev[32];
    int uart_fd = 0;
    pthread_t threads;

    skye_init_status_file();
    skye_init_dbskyelog_file();

    sprintf(test_name, "Skye  SM-%d CPU%d do all test", skye_test_slot, cpu_num);
    testname(test_name);

    /*Setup UART */
    if (cpu_num == CPU0) {
        if (is_skye_uart(0) == FAILED) {
            printf("Failed to switch uart 0\n");
            return (FAILED);
        }
    } else { /* CPU 1 */
        if (is_skye_uart(1) == FAILED) {
            printf("Failed to switch uart 1\n");
            return (FAILED);
        }
    }

    /* Setup UART toward Skye board */
    sprintf(tty_dev, "/dev/ttyDASH%d", skye_test_slot - 1);

    uart_fd = open(tty_dev, O_RDWR|O_NOCTTY);

    if (uart_fd < 0) {
        perror("polling uart: open tty failed");
        fflush(stdout);
        return (FAILED);
    }

    if(pthread_create(&threads, NULL, skye_rx_uart, (void *)&uart_fd)) {
        perror("pthread_create failed.");
        printf("%s: pthread_create failed.\n", __FUNCTION__);
        close(uart_fd);
        return FAILED;
    }

    while (--main_thread_wait_time ) {
        skye_nc_dispatch_comm(comm, cpu_num);

        if (skye_check_test_status() == FAILED) {
            printf("Skye SM-%d CPU%d do all test FAILED.\n", skye_test_slot, cpu_num);
            skye_kill_all_nc();
            if (pthread_cancel(threads)!=0) {
                printf("pthread_cancel error");
            }
            close(uart_fd);
            return (FAILED);
        } else {
            printf("Skye SM-%d CPU%d do all test PASSED.\n", skye_test_slot, cpu_num);
            skye_kill_all_nc();
            break;
        }
    }

    if (main_thread_wait_time == 0) {
        printf("Skye SM-%d CPU%d do all test, wait time out...\n", skye_test_slot, cpu_num);
    }

    if (pthread_cancel(threads)!=0) {
        printf("pthread_cancel error");
    }
    close(uart_fd);

    return (PASSED);
}

/*******************************************************************************
 *
 *  Function    : get_skye_vm_setup_util 
 *  Description : Utility to get wanted Skye voltage margin set-ups from user.
 *  Inputs      : None
 *  Outputs     : PASSED / FAILED
 *
 *******************************************************************************
 */
int
get_skye_vm_setup_util (void)
{
    int  ctr = 0, vm_tbl_sz = 0;
    char ans;
    FILE *fp;

    if ((fp = fopen(DIAG_CMD_DISPATCH_FILE, "a+")) == NULL) {
        printf("%s: Can't open/create file %s\n",
               __FUNCTION__, DIAG_CMD_DISPATCH_FILE);
        return (FAILED);
    }

    if (skye_one_cpu() == TRUE) {
        vm_tbl_sz = 6;
    } else {
        vm_tbl_sz = 7;
    }

    printf("Skye Voltage Margin Table:\n");
    fprintf(fp, "VM:");
    for (ctr = 0; ctr < vm_tbl_sz; ctr++) {
        while (1) {
            printf("\rSet %s to [H(h)/N(n)/L(l)]:", vm_setup_tbl[ctr]);
            ans = getchar();

            if (ans == 'H' || ans == 'h') {
                fprintf(fp, "H,");
                break;
            } else if (ans == 'N' || ans == 'n') {
                fprintf(fp, "N,");
                break;
            } else if (ans == 'L' || ans == 'l') {
                fprintf(fp, "L,");
                break;
            }
        }
    }

    fprintf(fp, ";");
    fclose(fp);

    return (PASSED);
}


/*******************************************************************************
 *
 * Function   : skye_exec_test
 * Description: This function dispatches command to Skye through nc command.
 * Inputs     :  cpu_num - cpu number
 *               tstname - testname command
 * Outputs    : None
 *
 *******************************************************************************
 */
int skye_exec_test (int cpu_num, char *tstname)
{
    skye_init_status_file();

    skye_nc_dispatch_comm(tstname, cpu_num);

    if (skye_check_test_status() == FAILED) {
        printf("Skye CPU%d %s test FAILED.\n", cpu_num, tstname);
        skye_kill_all_nc();
        return (FAILED);
    }

    printf("Skye CPU%d %s test PASSED.\n", cpu_num, tstname);
    skye_kill_all_nc();
    return (PASSED);
}


/*******************************************************************************
 *
 *  Function    : get_skye_fm_setup_util
 *  Description : Utility to setup Skye frequency margin from user through NC cmd
 *                by write to the config files.
 *  Inputs      : None
 *  Outputs     : PASSED / FAILED
 *
 *******************************************************************************
 */
int
get_skye_fm_setup_util (void)
{
    char ans;
    FILE *fp;

    if ((fp = fopen(SKYE_TMP_FM_FILE, "a+")) == NULL) {
        printf("%s: Can't open/create file %s\n",
               __FUNCTION__, SKYE_TMP_FM_FILE);
        return (FAILED);
    }

    if (skye_one_cpu() != TRUE) {
        printf("Frequency margin only control by CPU-0");
        return (PASSED);
    }

    printf("Skye Frequency Margin:\n");
    fprintf(fp, "FM:");
    printf("\rSet clock buffer to Plus/Normal/Minus [P(p)/N(n)/M(m)]:");
    ans = getchar();

    if (ans == 'P' || ans == 'p') {
        fprintf(fp, "P");
    } else if (ans == 'N' || ans == 'n') {
        fprintf(fp, "N");
    } else if (ans == 'M' || ans == 'm') {
        fprintf(fp, "M");
    }

    fprintf(fp, ";");
    fclose(fp);

    freq_margin = TRUE;

    return (PASSED);
}


/*------------------------------------------------------------------------------
 * $Log: skye_utils.c,v $
 * Revision 1.2  2015/05/25 03:56:16  steja
 * Add support Skye SM
 *
 * Revision 1.1.4.3  2015/05/11 14:09:16  steja
 * Code clean up <CSCuu14285>
 *
 * Revision 1.1.4.2  2015/04/29 11:48:02  steja
 * Code check-in to skye-branch2 for ER code review
 *
 *
 *------------------------------------------------------------------------------
 * Revision 1.1.2.6  2015/01/26 01:13:48  steja
 * 1. Add function for frequency margin to host side menu utilities through NC
 * 2. Remove "ifconfig gbe4" after retry ping to prevent mischeck through UART
 *
 * Revision 1.1.2.5  2014/11/27 02:31:05  steja
 * Fix the intermittent failure to run do all test(CSCur27613)
 *
 * Revision 1.1.2.4  2014/10/07 06:04:21  palin2
 * Added to set Skye voltages margin through NC command.
 *
 * Revision 1.1.2.3  2014/10/01 08:13:19  palin2
 * Merged NC command related code(skye_comm_lib.c) to skye_utils.c
 *
 * Revision 1.1.2.2  2014/09/09 09:02:13  steja
 * Add skye rx uart to print the test progress.
 *
 * Revision 1.1.2.1  2014/07/17 06:32:22  palin2
 * Initial check-in Skye host side code.
 *
 *------------------------------------------------------------------------------
 * shrinkray_utils.c:
 * Revision 1.2  2014/03/03 06:33:51  palin2
 * -Initial check-in ShrinkRay host side Diag.
 *
 * Revision 1.1.4.2  2014/02/27 07:09:58  steja
 * Fix compile error after get update from latest code main trunk
 *
 * Revision 1.1.4.1  2014/02/26 11:08:59  palin2
 * -To support ShrinkRay host side tests on O2.
 * -This branch is created to pick up O2 main tunk code changes.
 *
 * Revision 1.1.2.2  2014/01/27 08:51:07  steja
 * Code clean up
 *
 * Revision 1.1.2.1  2013/08/17 03:27:00  steja
 * add code command and respond ( Host <->GE <-> TILE CPU#0) for O2 platform
 *
 *
 *------------------------------------------------------------------------------
 * $Endlog$
 *
 *------------------------------------------------------------------------------
 */
