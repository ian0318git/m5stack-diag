/* $Id: ngwic_goldschlager_comm.c,v 1.3 2019/10/17 02:16:18 kehuang2 Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/goldschlager/ngwic_goldschlager_comm.c,v $
 *------------------------------------------------------------------------------
 *
 * nim_goldschlager_comm.c: NIM Goldschlager Communication Library
 *
 * Oct. 2013 - James Lin
 *
 * Copyright (c) 2013-2019 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------------------
 */
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
#include "ngwic_goldschlager.h"
#include "ngwic_goldschlager_comm.h"
#include "bcm63268_lib.h"

/*******************************************************************************
 *  Macro Definitions
 ******************************************************************************/

/*******************************************************************************
 *  Static Functions Declaration
 ******************************************************************************/
static void goldschlager_kill_all_nc(void);
static void goldschlager_init_status_file(void);
static int goldschlager_check_test_status(void);
static void goldschlager_init_return_file(void);
static int goldschlager_check_return_value(void);

/*******************************************************************************
 *  Functions Declaration
 ******************************************************************************/
int goldschlager_do_all(void);
void goldschlager_transmit_nc_request(int);
int goldschlager_nc_dispatch_comm(char *, uint, uint, uint);
int goldschlager_nc_dispatch_return_value(char *, uint, uint, uint);

/*******************************************************************************
 *  Externs
 ******************************************************************************/
extern int goldschlager_test_slot;
extern unsigned short goldschlager_sku;
/*******************************************************************************
 *  Global Variable
 ******************************************************************************/


/*******************************************************************************
 *  Functions
 ******************************************************************************/

/*******************************************************************************
 *
 * Function: goldschlager_do_all
 *
 * Description: This function transmits nc client request to wic card
 *              to run all tests
 *
 * Input:  None
 *
 * Output: PASSED/FAILED
 *
 *******************************************************************************
 */
int goldschlager_do_all (void)
{
    char cmd[128];
    char wic_ipaddr[32];
    char test_name[32];

    goldschlager_init_status_file();

    sprintf(test_name, "Goldschlager WIC-%d All", goldschlager_test_slot);

    testname(test_name);

    goldschlager_get_wic_ip_addr(wic_ipaddr);

    sprintf(cmd, "nc %s %d", wic_ipaddr, DIAG_RUN_ALL_PORT_BASE);
    system(cmd);

    if (goldschlager_check_test_status() == FAILED) {
        cterr('f', 0, "Goldschlager WIC-%d test fails", goldschlager_test_slot);
        return (FAILED);
    }

    printf("\n\nGoldschlager WIC-%d test passes\n", goldschlager_test_slot);

    return (PASSED);
}

/*******************************************************************************
 *
 * Function: goldschlager_transmit_nc_request
 *
 * Description: This function transmits nc client request to WIC card
 *              on provided port number
 *
 * Input:  None
 *
 * Output: None
 *
 *******************************************************************************
 */
void goldschlager_transmit_nc_request (int port)
{
    char cmd[128];
    char wic_ipaddr[32];

    goldschlager_get_wic_ip_addr(wic_ipaddr);

    sprintf(cmd, "nc %s %d", wic_ipaddr, port);
    system(cmd);

}

/*******************************************************************************
 *
 * Function: goldschlager_nc_dispatch_comm
 *
 * Description: This function dispatches command to WIC card through
 *              nc command
 *
 * Input:  comm - command
 *
 * Output: None
 *
 *******************************************************************************
 */
int goldschlager_nc_dispatch_comm (char *comm, uint op_mode, 
                                   uint idle_listen_params, uint line_id) 
{
    char wic_ipaddr[32];
    char cmd[128];
    int rc = FAILED;

    /* Sanity check */
    if (comm == NULL) {
        printf("%s: NULL Pointer\n", __FUNCTION__);
        return (FAILED);
    }

    goldschlager_init_status_file();

    goldschlager_get_wic_ip_addr(wic_ipaddr);

    /* Prepare command and listen for WIC card to grab */
    sprintf(cmd, "echo %s,%d,%d,%d, > %s", comm, op_mode, idle_listen_params,
             line_id, DIAG_COMMAND_DISPATCH_FILE);
    system(cmd);

    sprintf(cmd, "nc -l -p %d < %s&", DIAG_EXECUTE_COMMAND_TRANSFER_PORT_BASE,
            DIAG_COMMAND_DISPATCH_FILE);
    system(cmd);

    goldschlager_transmit_nc_request(DIAG_EXECUTE_COMMAND_PORT_BASE);

    if ((rc = goldschlager_check_test_status()) != PASSED) {
        printf("Goldschlager WIC-%d test fails at NC.\n", 
              goldschlager_test_slot);
        goldschlager_kill_all_nc();
        fflush(stdout);
        return (FAILED);
    }

    goldschlager_kill_all_nc();

    fflush(stdout);
    return (rc);
}

/*******************************************************************************
 *
 * Function: goldschlager_nc_dispatch_return_value
 *
 * Description: This function dispatches linkstatus command to WIC card through
 *              nc command
 *
 * Input:  comm - command
 *
 * Output: ADSL link status
 *
 *******************************************************************************
 */
int goldschlager_nc_dispatch_return_value (char *comm, uint op_mode, 
                                              uint idle_listen_params,
                                              uint line_id)
{
    char wic_ipaddr[32];
    char cmd[128];
    int return_value = 0;

    /* Sanity check */
    if (comm == NULL) {
        printf("%s: NULL Pointer\n", __FUNCTION__);
        return (-1);
    }
    
    goldschlager_init_status_file();

    goldschlager_init_return_file();

    goldschlager_get_wic_ip_addr(wic_ipaddr);

    /* Prepare command and listen for WIC card to grab */
    sprintf(cmd, "echo %s,%d,%d,%d, > %s", comm, op_mode, idle_listen_params,
             line_id, DIAG_COMMAND_DISPATCH_FILE);
    system(cmd);

    sprintf(cmd, "nc -l -p %d < %s&", DIAG_EXECUTE_COMMAND_TRANSFER_PORT_BASE,
            DIAG_COMMAND_DISPATCH_FILE);
    system(cmd);

    goldschlager_transmit_nc_request(DIAG_EXECUTE_COMMAND_PORT_BASE);

    return_value = goldschlager_check_return_value();

    if (return_value == (-1)) {
        printf("NC Failed: Nothing in buffer\n");
        goldschlager_kill_all_nc();
        fflush(stdout);
        return (return_value);
    }

    goldschlager_kill_all_nc();

    fflush(stdout);
    return (return_value);
}

/*******************************************************************************
 *  Static Functions
 ******************************************************************************/
 
/*******************************************************************************
 *
 * Function: goldschlager_init_status_file
 *
 * Description: This function clears out the content of status file
 *              and listen to the port
 *
 * Input:  None
 *
 * Output: None
 *
 *******************************************************************************
 */
static void goldschlager_init_status_file (void)
{
    char cmd[128];
    char status_file[64];

    sprintf(status_file, "/tmp/wic_goldschlager.status");
    sprintf(cmd, "echo ' ' > %s", status_file);
    system(cmd);

    /* Listen to the command status */
    sprintf(cmd, "nc -l -l -p %d > %s &",
            DIAG_RTN_STS_OUT_PORT_BASE, status_file);
    system(cmd);
}

/*******************************************************************************
 *
 * Function: goldschlager_check_test_status
 *
 * Description: This function checks the status of status file and
 *              determine whether the test passes or fails.
 *
 * Input:  None
 *
 * Output: PASSED/FAILED
 *
 *******************************************************************************
 */
static int goldschlager_check_test_status (void)
{
    FILE *fp;
    char status_file[64];
    char buf[64];
    char cmd[128];

    sprintf(status_file, "/tmp/wic_goldschlager.status");

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

/*******************************************************************************
 *
 * Function: goldschlager_init_return_file
 *
 * Description: This function clears out the content of adsl link status file
 *              and listen to the port
 *
 * Input:  None
 *
 * Output: None
 *
 *******************************************************************************
 */
static void goldschlager_init_return_file (void)
{
    char cmd[128];
    char return_file[64];

    sprintf(return_file, "/tmp/wic_goldschlager.return_value");
    sprintf(cmd, "echo ' ' > %s", return_file);
    system(cmd);

    /* Listen to the command status */
    sprintf(cmd, "nc -l -l -p %d > %s &",
            DIAG_RETURN_VLAUE_PORT_BASE, return_file);
    system(cmd);

}

/*******************************************************************************
 *
 * Function: goldschlager_check_return_value
 *
 * Description: This function checks the status of status file and
 *              determine whether the test passes or fails.
 *
 * Input:  None
 *
 * Output: PASSED/FAILED
 *
 *******************************************************************************
 */
static int goldschlager_check_return_value (void)
{
    FILE *fp;
    char return_file[128];
    char buf[128];
    char cmd[128];
    int return_vlaue = 0;

    sprintf(return_file, "/tmp/wic_goldschlager.return_value");

    fp = fopen(return_file, "r");
    if (fp == NULL) {

        printf("%s: Unable to open '%s'\n", __FUNCTION__, return_file);
        return (-1);
    }

    if (fgets(buf, sizeof(buf), fp) == NULL) {
        printf("Nothing in buffer\n");
        goto __exit;
    }

    if (strstr(buf, NC_BCM_ADSL_LINK_UP)) {
        return_vlaue = BCM_ADSL_LINK_UP;
    } else if (strstr(buf, NC_BCM_ADSL_LINK_DOWN)) {
        return_vlaue = BCM_ADSL_LINK_DOWN;
    } else if (strstr(buf, NC_BCM_ADSL_TRAINING_G992_EXCHANGE)) {
        return_vlaue = BCM_ADSL_TRAINING_G992_EXCHANGE;
    } else if (strstr(buf, NC_BCM_ADSL_TRAINING_G992_CHANNEL_ANALYSIS)) {
        return_vlaue = BCM_ADSL_TRAINING_G992_CHANNEL_ANALYSIS;
    } else if (strstr(buf, NC_BCM_ADSL_TRAINING_G992_STARTED)) {
        return_vlaue = BCM_ADSL_TRAINING_G992_STARTED;
    } else if (strstr(buf, NC_BCM_ADSL_TRAINING_G993_EXCHANGE)) {
        return_vlaue = BCM_ADSL_TRAINING_G993_EXCHANGE;
    } else if (strstr(buf, NC_BCM_ADSL_TRAINING_G993_CHANNEL_ANALYSIS)) {
        return_vlaue = BCM_ADSL_TRAINING_G993_CHANNEL_ANALYSIS;
    } else if (strstr(buf, NC_BCM_ADSL_TRAINING_G993_STARTED)) {
        return_vlaue = BCM_ADSL_TRAINING_G993_STARTED;
    } else if (strstr(buf, NC_BCM_ADSL_TRAINING_G994)) {
        return_vlaue = BCM_ADSL_TRAINING_G994;
    } else if (strstr(buf, NC_BCM_ADSL_G994_NONSTDINFO_RECEIVED)) {
        return_vlaue = BCM_ADSL_G994_NONSTDINFO_RECEIVED;
    } else if (strstr(buf, NC_BCM_ADSL_BERT_COMPLETE)) {
        return_vlaue = BCM_ADSL_BERT_COMPLETE;
    } else if (strstr(buf, NC_BCM_ADSL_ATM_IDLE)) {
        return_vlaue = BCM_ADSL_ATM_IDLE;
    } else if (strstr(buf, NC_BCM_ADSL_EVENT)) {
        return_vlaue = BCM_ADSL_EVENT;
    } else if (strstr(buf, NC_BCM_ADSL_G997_FRAME_RECEIVED)) {
        return_vlaue = BCM_ADSL_G997_FRAME_RECEIVED;
    } else if (strstr(buf, NC_BCM_ADSL_G997_FRAME_SENT)) {
        return_vlaue = BCM_ADSL_G997_FRAME_SENT;
    } else if (strstr(buf, "GS_NIM_VAB_A")) {
        return_vlaue = GS_NIM_VAB_A;
    } else if (strstr(buf, "GS_NIM_VA_B")) {
        return_vlaue = GS_NIM_VA_B;
    } else if (strstr(buf, "GS_NIM_VAB_M")) {
        return_vlaue = GS_NIM_VAB_M;
    }

    fclose(fp);
    fflush(stdout);

    return (return_vlaue);

    __exit:
    fclose(fp);

    printf("%s: Fgets fails\nContent of '%s':\n", __FUNCTION__, return_file);
    fflush(stdout);

    sprintf(cmd, "cat %s", return_file);
    system(cmd);

    return (-1);
}

/*******************************************************************************
 *
 * Function: goldschlager_kill_all_nc
 *
 * Description: This function lists all process and grep 'nc -l'
 *              keyword, and dump it to temporary directory so we
 *              can kill them afterwards
 *
 * Input:  None
 *
 * Output: None
 *
 *******************************************************************************
 */
static void goldschlager_kill_all_nc (void)
{
    char cmd[128];
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
#ifdef TABEIL
        /* Tabei-L - atoi input should not be NULL, or goldschlager will crash */
        if (token == NULL) {
            continue;
        }
#endif
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


/******** History ********/
/*------------------------------------------------------------------------------
 * $Log: ngwic_goldschlager_comm.c,v $
 * Revision 1.3  2019/10/17 02:16:18  kehuang2
 * Collapse Tabei-L into main trunk
 *
 * Revision 1.2  2014/09/17 03:32:16  jamlin
 * Add support for Goldschlager NIM.
 *
 * Revision 1.1.6.2  2014/08/08 02:43:57  jamlin
 * goladschlager-branch3 initail commit.
 *
 * Revision 1.1.4.4  2014/04/11 03:50:04  jamlin
 * GS annexB PID changes from NIM-VAB-B tp NIM-VA-B
 *
 * Revision 1.1.4.3  2014/02/10 04:03:18  jamlin
 * rename nc_dispatch_linkstatus to nc_dispatch_return_value function and added check_sku_type function
 *
 * Revision 1.1.4.2  2014/01/07 01:54:52  jamlin
 * Goldschlager new branch goldschlager-branch2
 *
 * Revision 1.1.2.2  2013/12/04 01:38:52  jamlin
 * Support Bonding channels showtime status display.
 *
 * Revision 1.1.2.1  2013/11/02 13:39:51  jamlin
 * Initial commit for bringup.
 *
 *
 *------------------------------------------------------------------------------
 * $Endlog$
 *------------------------------------------------------------------------------
 */

