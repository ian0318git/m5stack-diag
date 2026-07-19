/* $Id: diag_nc_client_lib.c,v 1.2 2016/04/20 11:25:29 benchen2 Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/tachi_bmc/diag_nc_client_lib.c,v $
 *------------------------------------------------------------------
 *
 * diag_nc_client_lib.c  
 * CSX-Tachi nc client library 
 *
 * Nov 2015, Alan Peng
 *
 * Copyright (c) 2016 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include "common.h"
#include "diag_nc_common.h"

/********************************************************************
 *
 * Function: nc_init_result_file
 *
 * Description: listen to port 1888 with file /tmp/nc_result 
 *
 * Inputs      : NONE
 * Outputs     : NONE
 *
 **********************************************************************/
void nc_init_result_file (void)
{
    unsigned int cmd, pnum;
    char fname[32];

    cmd = SVR_LSTN_RECV; 
    pnum = 1888;
    sprintf(fname,"%s", "/tmp/nc_result");

    system("rm /tmp/nc_result &> /dev/null");
    general_nc_wrapper(cmd, pnum, fname, NULL, NULL);

    return ;
}

/********************************************************************
 *
 * Function: nc_host_dispatch_comm
 *
 * Description:  compress cmd and argument 
 *               then trigger the server start to process command 
 *         
 * Inputs      : tgt_subsys - 0:bmc, 1:intel, 2:lewis
 *               arg_str - pointer to cmd and argument 
 * Outputs     : PASSED/FAILED
 *
 **********************************************************************/
int nc_host_dispatch_comm (unsigned int tgt_subsys, struct nc_args *arg_str)
{
    unsigned int cmd, pnum;
    char fname[32], ipaddr[32], tmp[256]; 
    struct nc_args *ptr; 

    ptr = arg_str; 

    cmd = SVR_LSTN_SEND; 
    pnum = 2688; 
    sprintf(fname, "%s", "/tmp/nc_cmd_disp");
    system("rm /tmp/nc_cmd_disp &> /dev/null");

    /* prepare dispatch cmd and file */
    sprintf(tmp, "echo %s", ptr->arg);

    ptr = ptr->next; 
    while (ptr != NULL) {
        sprintf(tmp, "%s,%s",tmp, ptr->arg); 
        ptr = ptr->next; 
    }
    sprintf(tmp, "%s > %s", tmp, fname);
/* Remove NC checking log
printf("BMC nc_host_dispatch_comm = %s\n", tmp );*/
    /* above sprintf may clean up fname, 
     * assigned fname again 
     */
    sprintf(fname, "%s", "/tmp/nc_cmd_disp");

    system(tmp); 
    general_nc_wrapper(cmd, pnum, fname, NULL, NULL); 

    switch (tgt_subsys) {
    case BMC_SUB:
        sprintf(ipaddr, "%s", BMC_IPADDR);
    break;
    case INTEL_SUB:
        sprintf(ipaddr, "%s", INTEL_IPADDR);
    break;
    case LEWIS_SUB:
        sprintf(ipaddr, "%s", LEWIS_IPADDR);
    break;
    }

    /* trigger server */
    cmd = CLI_REQ_TRIG; 
    pnum = 2988; 
    general_nc_wrapper(cmd, pnum, NULL, NULL, ipaddr); 
    
    return (PASSED);
}

/********************************************************************
 *
 * Function: nc_check_test_status
 *
 * Description:  open a file /tmp/nc_reselt for check the status 
 *               is pass or fail 
 *
 * Inputs      : None
 * Outputs     : PASSED/FAILED
 *
 **********************************************************************/
int nc_check_test_status (void)
{
    FILE *fp;
    char status_file[32];
    char buf[32];
    char cmd[32];
    int  recheck = 0;
    sprintf(status_file, "/tmp/nc_result");

    fp = fopen(status_file, "r");
    if (fp == NULL) {
        printf("%s: Unable to open '%s'\n", __FUNCTION__, status_file);
        return (FAILED);
    }


    while (fgets(buf, sizeof(buf), fp) == NULL) {
        usleep(NC_RECHECK_WAIT_TIME);
        recheck++;
        if (recheck > NC_RECHECK_WAIT_LOOP) {
            printf("%s: Fgets fails\nContent of '%s':\n", __FUNCTION__, status_file);
            fflush(stdout);
            sprintf(cmd, "cat %s", status_file);
            system(cmd);
            break;
        }
    }

    if (recheck <= NC_RECHECK_WAIT_LOOP) {
        if (strstr(buf, "PASS")) {
            fclose(fp);
            return (PASSED);
        } 
		else if (strstr(buf, "INFO")) {
			printf("\n");
			system("more /tmp/nc_result");
			return (PASSED);
		}
		else {
            printf("Fail! Return Status is %s\n", buf);
            fflush(stdout);
        }
    } 

    fclose(fp);
    return (FAILED);
}

/*---------------------------------------------------------------
$Log: diag_nc_client_lib.c,v $
Revision 1.2  2016/04/20 11:25:29  benchen2
add tachi fru portion

Revision 1.1.2.7  2016/02/24 02:57:33  hondwang
Remove NC checking log

Revision 1.1.2.6  2016/01/14 02:06:38  jskow
Add verbose mode and time stamps to GESW tests.  Add functions to transmit mid-test information from GESW to BMC.  Modify nc_client_lib to accept INFO status remotely from Intel/GESW to BMC.  Modify GESW test names for clarity.

Revision 1.1.2.5  2016/01/06 03:02:00  hondwang
Add NC retry to 10 Sec

Revision 1.1.2.4  2015/12/09 10:35:57  alpeng
update code to support lpbk test on bmc for dreamliner

Revision 1.1.2.3  2015/12/01 02:04:36  alpeng
update nc infra structures and support testcard pcie test with nc

Revision 1.1.2.2  2015/11/25 06:12:12  benchen2
add bmc nc comm portion

Revision 1.1.2.1  2015/11/24 12:14:30  alpeng
add nc infrastructure


$Endlog$
*/

