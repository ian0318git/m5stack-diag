/* $Id: sm_woodlawn_comm.c,v 1.3 2014/02/18 09:11:12 alpeng Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/woodlawn/sm_woodlawn_comm.c,v $
 *------------------------------------------------------------------
 * Filename: sm_woodlawn_comm.c
 *
 * Description: SM Woodlawn Communication Library
 * Author: Times Huang
 *
 * Copyright (c) 2013-2014 by cisco Systems, Inc.
 * All rights reserved.
 *------------------------------------------------------------------
 */

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
#include "cookie_4.h"
#include "proto.h"
#include "strings.h"
#include "sm_slot.h"
#include "menu.h"
#include "platform_slot.h"
#include "cross_platform.h"
#include "platform_i2c.h"
#include "router_if.h"

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <termios.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/time.h>
#include <sys/select.h>

#include "sm_woodlawn.h"
#include "sm_woodlawn_comm.h"

/***********************************************************************
 *  Macro Definitions
 ************************************************************************/

/***********************************************************************
 *  Static Functions Declaration
 ************************************************************************/

static void woodlawn_kill_all_nc(void);
static int woodlawn_check_test_status(void);
static void woodlawn_init_status_file(void);

/***********************************************************************
 *  Functions Declaration
 ************************************************************************/

void woodlawn_transmit_nc_request(int);
int woodlawn_do_all(void);
void woodlawn_nc_dispatch_comm(char *);

/***********************************************************************
 *  Externs
 ************************************************************************/

extern int woodlawn_test_slot;

/***********************************************************************
 *  Global Variable
 ************************************************************************/


/***********************************************************************
 *  Functions
 ************************************************************************/

/*****************************************************************
 *
 * Function: woodlawn_do_all
 *
 * Description: This function transmits nc client request to SM card
 *              to run all tests
 *
 * Input:  None
 *
 * Output: PASSED/FAILED
 *
 *****************************************************************
 */
int woodlawn_do_all (void)
{
    char cmd[32];
    char sm_ipaddr[32];
    char test_name[32];

    woodlawn_init_status_file();

    sprintf(test_name, "Woodlawn SM-%d All", woodlawn_test_slot);

    testname(test_name);

    woodlawn_get_sm_ip_addr(sm_ipaddr);

    sprintf(cmd, "nc %s %d", sm_ipaddr, DIAG_RUN_ALL_PORT_BASE);
    system(cmd);

    if (woodlawn_check_test_status() == FAILED) {
        cterr('f', 0, "Woodlawn SM-%d test fails", woodlawn_test_slot);
        return (FAILED);
    }

    printf("\n\nWoodlawn SM-%d test passes\n", woodlawn_test_slot);

    return (PASSED);
}

/*****************************************************************
 *
 * Function: woodlawn_transmit_nc_request
 *
 * Description: This function transmits nc client request to SM card
 *              on provided port number
 *
 * Input:  None
 *
 * Output: None
 *
 *****************************************************************
 */
void woodlawn_transmit_nc_request (int port)
{
    char cmd[32];
    char sm_ipaddr[32];

    woodlawn_get_sm_ip_addr(sm_ipaddr);

    sprintf(cmd, "nc %s %d", sm_ipaddr, port);
    system(cmd);

    woodlawn_kill_all_nc();
}


/*****************************************************************
 *
 * Function: woodlawn_nc_dispatch_comm
 *
 * Description: This function dispatches command to SM card through
 *              nc command
 *
 * Input:  comm - command
 *
 * Output: None
 *
 *****************************************************************
 */
void woodlawn_nc_dispatch_comm (char *comm)
{
    char sm_ipaddr[32];
    char cmd[32];

    /* Sanity check */
    if (comm == NULL) {
        printf("%s: NULL Pointer\n", __FUNCTION__);
        return;
    }

    woodlawn_get_sm_ip_addr(sm_ipaddr);

    /* Prepare command and listen for SM card to grab */
    sprintf(cmd, "echo %s, > %s", comm, DIAG_COMMAND_DISPATCH_FILE);
    system(cmd);

    sprintf(cmd, "nc -l -p %d < %s&", DIAG_EXECUTE_COMMAND_TRANSFER_PORT_BASE,
            DIAG_COMMAND_DISPATCH_FILE);
    system(cmd);

    woodlawn_transmit_nc_request(DIAG_EXECUTE_COMMAND_PORT_BASE);
}


/***********************************************************************
 *  Static Functions
 ************************************************************************/

/*****************************************************************
 *
 * Function: woodlawn_init_status_file
 *
 * Description: This function clears out the content of status file
 *              and listen to the port
 *
 * Input:  None
 *
 * Output: None
 *
 *****************************************************************
 */
static void woodlawn_init_status_file (void)
{
    char cmd[64];
    char status_file[32];

    sprintf(status_file, "/tmp/sm_%d_woodlawn.status", woodlawn_test_slot);
    sprintf(cmd, "echo ' ' > %s", status_file);
    system(cmd);

    /* Listen to the command status */
    sprintf(cmd, "nc -l -l -p %d > %s &",
            DIAG_RTN_STS_OUT_PORT_BASE, status_file);
    system(cmd);
}


/*****************************************************************
 *
 * Function: woodlawn_check_test_status
 *
 * Description: This function checks the status of status file and
 *              determine whether the test passes or fails.
 *
 * Input:  None
 *
 * Output: PASSED/FAILED
 *
 *****************************************************************
 */
static int woodlawn_check_test_status (void)
{
    FILE *fp;
    char status_file[32];
    char buf[32];
    char cmd[32];

    sprintf(status_file, "/tmp/sm_%d_woodlawn.status", woodlawn_test_slot);

    fp = fopen(status_file, "r");
    if (fp == NULL) {
        printf("%s: Unable to open '%s'\n", __FUNCTION__, status_file);
        return (FAILED);
    }

    if (fgets(buf, sizeof(buf), fp) != NULL) {
        if (strstr(buf, DIAG_RTN_PASS_STR)) {
            fclose(fp);
            return (PASSED);
        } else {
            printf("Fail! Return Status is %s\n", buf);
            fflush(stdout);
        }
    }

    fclose(fp);

    printf("%s: Fgets fails\nContent of '%s':\n", __FUNCTION__, status_file);
    fflush(stdout);
    sprintf(cmd, "cat %s", status_file);
    system(cmd);

    return (FAILED);
}


/*****************************************************************
 *
 * Function: woodlawn_kill_all_nc
 *
 * Description: This function lists all process and grep 'nc -l'
 *              keyword, and dump it to temporary directory so we
 *              can kill them afterwards
 *
 * Input:  None
 *
 * Output: None
 *
 *****************************************************************
 */
static void woodlawn_kill_all_nc (void)
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
    sprintf(cmd, "ps | grep 'nc 192\\|nc -l' > %s", DIAG_KILL_NC_TMP_FILE);
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

    fclose(fp);

}

/*------------------------------------------------------------------
 * $Log: sm_woodlawn_comm.c,v $
 * Revision 1.3  2014/02/18 09:11:12  alpeng
 * CSCul88171-3: remove useless files: mon_boot.h, c82576_ethmap.h, eth_frames.c, stack.h, sys_regs.h
 *
 * Revision 1.2  2013/10/08 08:48:26  tirawan
 * Woodlawn collapsed to main trunk
 *
 * Revision 1.1.4.2  2013/08/20 10:58:49  tirawan
 * Branch into woodlawn-branch2 and port woodlawn code
 *
 * Revision 1.1.2.5  2013/06/13 11:38:20  tirawan
 * Implement NC dispatch command
 *
 * Revision 1.1.2.4  2013/04/10 06:09:33  tirawan
 * Close fp when returning pass in polling status file
 *
 * Revision 1.1.2.3  2013/04/10 03:33:28  tirawan
 * Add GE backplane loopback test to verify GE0 connectivity
 *
 * Revision 1.1.2.2  2013/04/08 08:04:39  tirawan
 * Close file descriptor after parsing status file
 *
 * Revision 1.1.2.1  2013/04/03 05:46:40  tirawan
 * Add auto boot by UART function, and auto run by nc utility
 *
 *
 *
 *------------------------------------------------------------------
 * $Endlog$
 *
 *------------------------------------------------------------------
 */

