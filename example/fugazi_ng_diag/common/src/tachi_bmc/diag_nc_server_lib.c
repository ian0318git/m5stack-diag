/* $Id: diag_nc_server_lib.c,v 1.2 2016/04/20 11:25:29 benchen2 Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/tachi_bmc/diag_nc_server_lib.c,v $
 *------------------------------------------------------------------
 *
 * diag_nc_server_lib.c
 * CSX-Tachi nc server library
 *
 * Nov 2016, Alan Peng
 *
 * Copyright (c) 2016 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include "common.h"
#include "diag_nc_common.h"


static char *ipaddr_list[32] = {BMC_IPADDR, INTEL_IPADDR, LEWIS_IPADDR} ; 


/********************************************************************
 *
 * Function: nc_init_listen_port
 *
 * Description: will launch on subsystem init, such as rcS. 
 *
 * Inputs      : NONE
 * Outputs     : NONE
 *
 **********************************************************************/
#if 0
void nc_init_listen_port (void)
{
    unsigned int cmd, pnum;
    char exec[32], ipaddr[32];

    cmd = SVR_LSTN_EXEC;
    pnum = 2988;
    sprintf(exec, "%s", "./main_server");

    general_nc_wrapper(cmd, pnum, NULL, exec, ipaddr);

    return ;
}
#endif 

/********************************************************************
 *
 * Function: diag_nc_get_dispatch_comm
 *
 * Description:  get dispatch command from host side, will scan 
 *               each of subsystem 
 *
 * Inputs      : tgt_subsys - 0:bmc, 1:intel, 2:lewis
 * Outputs     : PASSED/FAILED
 *
 **********************************************************************/
int diag_nc_get_dispatch_comm (unsigned int tgt_subsys)
{
    unsigned int cmd, pnum, allp = FALSE, ia = 0, hit_client;
    char fname[32], ipaddr[32];
    int rc; 
/* Remove NC checking log
    printf("%s \n", __FUNCTION__);*/ 
    switch (tgt_subsys) {
    case BMC_SUB:
        memcpy(ipaddr, ipaddr_list[BMC_SUB], sizeof(ipaddr));
    break;
    case INTEL_SUB:
        memcpy(ipaddr, ipaddr_list[INTEL_SUB], sizeof(ipaddr));
    break;
    case LEWIS_SUB:
        memcpy(ipaddr, ipaddr_list[LEWIS_SUB], sizeof(ipaddr));
    break;
    default:
        /* scan for all sub system */
        allp = TRUE; 
    break;
    }

    cmd = CLI_REQ_RECV; 
    pnum = 2688;
    sprintf(fname, "%s",  "/tmp/nc_dispatch_cmd"); 

    if (allp == TRUE) { 
        while (ia < 3) {
            memcpy(ipaddr, ipaddr_list[ia], sizeof(ipaddr));
            rc = general_nc_wrapper(cmd, pnum, fname, NULL, ipaddr);
            if (rc == PASSED) {
                hit_client = ia; 
                break; 
            } 
            ia++;
        }
        if (ia ==3) {
            printf("scan fail... \n");

        }

    } else {
        general_nc_wrapper(cmd, pnum, fname, NULL, ipaddr);
    }

    return (hit_client); 
}

/********************************************************************
 *
 * Function: diag_nc_get_dispatch_comm
 *
 * Description:  get dispatch command from host side, will scan
 *               each of subsystem
 *
 * Inputs      : tgt_subsys - 0:bmc, 1:intel, 2:lewis
 * Outputs     : PASSED/FAILED
 *
 **********************************************************************/
struct nc_args *diag_get_parms_frm_host (void)
{
    FILE *fp;
    char buff[64], cmd[64], *token = NULL;
    struct nc_args *ptr, *tmp, *head;

    fp = fopen("/tmp/nc_dispatch_cmd", "r");
    if (fp == NULL) {
        printf("%s: Open %s fails\n", __FUNCTION__,
               "/tmp/nc_dispatch_cmd");
        return (NULL);
    }

    if (fgets(buff, sizeof(buff), fp) == NULL) {
        printf("1 -Nothing in buffer\n");
        goto __exit;
    }

    /* Proxy the parameters from host */
    token = strtok(buff, ",");
    ptr = (struct nc_args*) malloc(sizeof(struct nc_args));
    head = ptr; 
    strcpy(ptr->arg, token);
    ptr->next = NULL; 

    while (token != NULL) {
        token = strtok(NULL, ",");
        if (token == NULL) {
            break;  /* break before create node */
        }
        tmp = (struct nc_args*) malloc(sizeof(struct nc_args));
        strcpy(tmp->arg, token);
        tmp->next = NULL; 
        ptr->next = tmp; 
        ptr = tmp; 
    }

    fclose(fp);
    return (head); 

__exit:
    fclose(fp);

    printf("%s: Fgets fails\nContent of '%s':\n", __FUNCTION__,
           "/tmp/nc_dispatch_cmd");
    fflush(stdout);

    sprintf(cmd, "cat %s", "/tmp/nc_dispatch_cmd");
    system(cmd);

    return (NULL);
}

/********************************************************************
 *
 * Function:  diag_return_parms_to_host
 *
 * Description:  send result file to host 
 *
 * Inputs      : tgt_subsys - 0:bmc, 1:intel, 2:lewis
 *               response - PASS/Fail string 
 *               parms_str - test/util name 
 * Outputs     : None
 *
 **********************************************************************/
void diag_return_parms_to_host (unsigned int tgt_subsys, 
                                char *response, char *parms_str)
{
    unsigned int cmd, pnum;
    char fname[32], ipaddr[32], parms_file[64];
    char tmp[32];

    switch (tgt_subsys) {
    case BMC_SUB:
        memcpy(ipaddr, ipaddr_list[BMC_SUB], sizeof(ipaddr));
    break;
    case INTEL_SUB:
        memcpy(ipaddr, ipaddr_list[INTEL_SUB], sizeof(ipaddr));
    break;
    case LEWIS_SUB:
        memcpy(ipaddr, ipaddr_list[LEWIS_SUB], sizeof(ipaddr));
    break;
    default:
        /* scan for all sub system */
        printf("failed to get tgt_subsys = %d\n", tgt_subsys);
    break;

    }

    sprintf(parms_file, "%s",  "/tmp/nc_cmd_result");
    sprintf(tmp, "echo %s,%s > %s", response, parms_str, parms_file);
    system(tmp);

    cmd = CLI_REQ_SEND;
    pnum = 1888;
    sprintf(fname, "%s",  "/tmp/nc_cmd_result"); 
    general_nc_wrapper(cmd, pnum, fname, NULL, ipaddr);

    return; 
}

/*---------------------------------------------------------------
$Log: diag_nc_server_lib.c,v $
Revision 1.2  2016/04/20 11:25:29  benchen2
add tachi fru portion

Revision 1.1.2.4  2016/02/24 03:03:46  hondwang
Remove NC checking log

Revision 1.1.2.3  2015/12/01 02:04:36  alpeng
update nc infra structures and support testcard pcie test with nc

Revision 1.1.2.2  2015/11/25 06:12:12  benchen2
add bmc nc comm portion

Revision 1.1.2.1  2015/11/24 12:14:31  alpeng
add nc infrastructure


$Endlog$
*/

