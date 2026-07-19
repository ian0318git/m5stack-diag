 /* $Id: diag_nc_lib.c,v 1.2 2018/08/06 02:31:51 harrchan Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/viper/diag_nc_lib.c,v $
 *------------------------------------------------------------------
 * 
 * diag_nc_lib.c - This file is for nc library. 
 *
 * Copyright (c) 2017-2018 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include "common.h"
#include "types.h"
#include "common_utils.h"
#include "menu.h"
#include "nvmonvars.h"
#include "proto.h"
#include "queryflags.h"
#include "diag_nc_lib.h"


/*******************************************************************************
 *                             Functions Declaration                           *
 *******************************************************************************
 */
static void viper_transmit_nc_request(int);
void        viper_kill_all_nc(void);
void        viper_nc_init_parms_file(void);
void        viper_nc_dispatch_comm(char *, char *);
int         viper_nc_get_parms(int, char *);
int         viper_nc_dispatch_comm_is_ok(void);
int         check_ext_lpbk_flag(void);

/*******************************************************************************
 *                               Global Variable                               *
 *******************************************************************************
 */



/*******************************************************************************
 *                                    Functions                                *
 *******************************************************************************
 */

/*****************************************************************
 *
 * Function: viper_get_module_ip_addr
 *
 * Description: This function returns IP Address of module.
 *
 * Input:  ip_addr - Buffer to put ip address
 *
 * Output: None
 *
 *****************************************************************
 */
void viper_get_module_ip_addr (char *ip_addr)
{
    char module_ip[VIPER_NC_MAX_STR_SIZE];

    /* Sanity check */
    if (ip_addr == NULL) {
        printf("%s: NULL Pointer\n", __FUNCTION__);
        return;
    }
    sprintf(module_ip, "%s.%d", DIAG_DSL_SUBNET_STR,
            DIAG_MODULE_IP_ADDR);

    sprintf(ip_addr, "%s", module_ip);
}

/*******************************************************************************
 *
 * Function: viper_nc_init_parms_file
 *
 * Description: This function clears out the content of parameters file
 *              and listen to a specific port for connections.
 *
 * Input:  None
 *
 * Output: None
 *
 *******************************************************************************
 */
void viper_nc_init_parms_file (void)
{
    char cmd[128];
    char parms_file[64];

    sprintf(parms_file, DIAG_VIPER_NC_TMP_PARMS_FILE);
    sprintf(cmd, "echo ' ' > %s", parms_file);
    system(cmd);

    /* Listen to the command status */
    sprintf(cmd, "nc -l -p %d > %s &",
            DIAG_VIPER_NC_RTN_PARMS_PORT_BASE, parms_file);
    system(cmd);
}


/*****************************************************************
 *
 * Function: viper_nc_dispatch_comm
 *
 * Description: This function transmits nc client request to module.
 *
 * Input:  None
 *
 * Output: None
 *
 *****************************************************************
 */
void viper_nc_dispatch_comm (char *comm, char *parms_str)
{
    char cmd[128];

    /* Sanity check */
    if (comm == NULL) {
        printf("%s: NULL Pointer\n", __FUNCTION__);
        return;
    }

    /* Prepare command and listen for module to grab */
    sprintf(cmd, "echo %s,%s, > %s",
            comm, parms_str, DIAG_VIPER_NC_COMMAND_DISPATCH_FILE);
    system(cmd);

    sprintf(cmd, "nc -l -p %d < %s &",
            DIAG_VIPER_NC_EXECUTE_COMMAND_TRANSFER_PORT_BASE,
            DIAG_VIPER_NC_COMMAND_DISPATCH_FILE);
    system(cmd);

    viper_transmit_nc_request(DIAG_VIPER_NC_EXECUTE_COMMAND_PORT_BASE);
}

/*******************************************************************************
 *
 * Function: viper_nc_get_parms
 *
 * Description: This function extract the return parameters that in the
 *              parms file.
 *
 * Input:  parms_num: Which parameter in the parms file shall be extracted.
 *         input: The parameter that be extracted out.
 *
 * Output: PASSED/FAILED
 *
 *******************************************************************************
 */
int viper_nc_get_parms (int parms_num, char *input)
{
    FILE *fp;
    char parms_file[64];
    char buf[128];
    char cmd[128];
    int curr_parm_num = 0;
    char *token = NULL;
    int ix = 0, repeat = 100;

    sprintf(parms_file, DIAG_VIPER_NC_TMP_PARMS_FILE);

    for (ix = 0; ix <= repeat; ix++) {
        fp = fopen(parms_file, "r");

        if (fgets(buf, sizeof(buf), fp) == NULL) {
            fclose(fp);
            mdelay(100);  /* delay 100 ms before retries to reopen file */
            if (ix == repeat) { /* Max wait 1 sec */
                printf("counter:%d\n", ix); 
                printf("%s: Open %s fails\n", __FUNCTION__,
                      parms_file);
                printf("Nothing in buffer\n");
                goto __exit;
            }
        } else {
            break;
        }
    }

    /* Proxy the return parameters */
    token = strtok(buf, ",");
    
    while (token != NULL) {
        if (curr_parm_num == parms_num) {
            break;
        }
        curr_parm_num++;
        token = strtok(NULL, ",");
    }

    strcpy(input, token);

    fclose(fp);
    return (PASSED);

    __exit:
    printf("%s: Fgets fails\nContent of '%s':\n", __FUNCTION__, parms_file);
    fflush(stdout);
    
    sprintf(cmd, "cat %s", parms_file);
    system(cmd);

    return (FAILED);
}

/*******************************************************************************
 *
 * Function: viper_nc_dispatch_comm_is_ok
 *
 * Description: This function checks the NC response is good or not.
 *
 * Input:  None
 *
 * Output: PASSED/FAILED
 *
 *******************************************************************************
 */
int viper_nc_dispatch_comm_is_ok(void)
{
    char buff[64];
    int retval = FAILED;

    /* First token is to indicate the result of the opcode */
    retval = viper_nc_get_parms(0, buff);

    if (retval == FAILED) {
        return (FAILED);
    }

    if (strcmp(buff, DIAG_VIPER_NC_RTN_PASS_STR)) {
        return (FAILED);
    }

    return (PASSED);
}

/***********************************************************************
 *  Static Functions
 ************************************************************************/

/*****************************************************************
 *
 * Function: viper_transmit_nc_request
 *
 * Description: This function transmits nc client request to module
 *              on provided port number
 *
 * Input:  None
 *
 * Output: None
 *
 *****************************************************************
 */
static void viper_transmit_nc_request (int port)
{
    char cmd[32];
    char module_ipaddr[32];

    viper_get_module_ip_addr(module_ipaddr);

    sprintf(cmd, "nc %s %d", module_ipaddr, port);
    system(cmd);
}

/*****************************************************************
 *
 * Function: viper_kill_all_nc
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
void viper_kill_all_nc (void)
{
    char cmd[64];
    char buf[128];
    char pid_file[32];
    char *token;
    int pid;
    FILE *fp;
    struct stat sts;

    /* Clear the content of the file first */
    sprintf(cmd, "echo ' ' > %s", DIAG_VIPER_NC_KILL_TMP_FILE);
    system(cmd);
    sprintf(cmd, "ps | grep 'nc 192\\|nc -l' > %s", DIAG_VIPER_NC_KILL_TMP_FILE);
    system(cmd);

    fp = fopen(DIAG_VIPER_NC_KILL_TMP_FILE, "r");
    if (fp == NULL) {
        printf("%s: Open '%s' Failed\n", __FUNCTION__,
        	   DIAG_VIPER_NC_KILL_TMP_FILE);
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




/*******************************************************************************
 *
 * Function   : check_ext_lpbk_flag
 * Description: Function to check if Ext. Loopback Flag is ON or not.
 * Inputs     : None
 * Outputs    : TRUE(ON) / FALSE(OFF)
 *
 *******************************************************************************
 */
int check_ext_lpbk_flag (void)
{
    /* according to menu_show_dflags(), D_EXT_LPBK is inverse flag */ 
    if ((NVRAM)->diagflag & D_EXT_LOOPBACK) {
        return (FALSE);
    } else { 
        return (TRUE);
    }
}



/*-------------------------------------------------
 * $Log: diag_nc_lib.c,v $
 * Revision 1.2  2018/08/06 02:31:51  harrchan
 * Merge viper E2E to the main trunk (CSCvk28469)
 *
 * Revision 1.1.2.3  2018/07/03 05:38:55  harrchan
 * Follow the coding rule to clean up code
 *
 * Revision 1.1.2.2  2018/06/27 06:27:52  harrchan
 * Follow the coding rule to clean up code
 *
 * Revision 1.1.2.1  2018/02/27 08:06:45  harrchan
 * Initial viper application code base
 *
 *
 * $Endlog$
 *-------------------------------------------------
 */

