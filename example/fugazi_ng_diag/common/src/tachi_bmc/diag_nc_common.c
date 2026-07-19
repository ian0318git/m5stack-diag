/* $Id: diag_nc_common.c,v 1.2 2016/04/20 11:25:29 benchen2 Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/tachi_bmc/diag_nc_common.c,v $
 *------------------------------------------------------------------
 *
 * diag_nc_common.c
 * CSX-Tachi nc execute wrapper 
 *
 * Nov 2015, Alan Peng
 *
 * Copyright (c) 2016 by Cisco Systems, Inc.
 * All rights reserved.
 *
 * Note: Please check Tachi-Entry DFS for command definition. 
 *
 *------------------------------------------------------------------
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "common.h"
#include "diag_nc_common.h"
#include "nvmonvars.h"

static int diag_general_nc(unsigned int, unsigned int, char*,
                            char *, char*);
static unsigned int check_arg(unsigned int, unsigned int, 
                               char*, char *, char*);

/**********************************************************************
 *
 * Function: check_arg
 *
 * Description: check argument before starting specific nc command 
 *
 * Input : cmd - 6 of client and server command.
 *         pnum - port number for nc
 *         fname - file name in case nc cmd need it .
 *         exec - execute cmd for nc to execute
 *         ipaddr - subsystem ip address
 *
 * Output: PASSED or FAILED
 *
 **********************************************************************
 */
static unsigned int check_arg (unsigned int cmd, unsigned int port, 
                               char* file, char *exec, char* ipaddr)
{

    switch(cmd) {
    case CLI_REQ_RECV:
    case CLI_REQ_SEND:
        if ((ipaddr == '\0') || (port == 0) || (file == '\0')) {
            return (FAILED);
        }
    break; 
    case CLI_REQ_TRIG:
        if ((ipaddr == 0) || (port == 0)) {
            return (FAILED);
        }
    break; 
    case SVR_LSTN_RECV:
    case SVR_LSTN_SEND:
        if ((port == 0) || (*file == '\0')) {
            return (FAILED);
        }
    break; 
    case SVR_LSTN_EXEC:  
        if ((port == 0) || (exec == '\0')) {
            return (FAILED);
        }
    break;
    default:
        printf("not support this commadn \n"); 
        return (FAILED);
    break; 
    }

    return (PASSED);

}

/**********************************************************************
 *
 * Function: diag_general_nc
 *
 * Description: based on command to execute specific command 
 *
 * Input : command - 6 of client and server command.
 *         port - port number for nc
 *         file - file name in case nc cmd need it .
 *         exec - execute cmd for nc to execute
 *         ipaddr - subsystem ip address
 *
 * Output: 0 or not zero, based on system() returns nc result. 
 *
 **********************************************************************
 */

static int diag_general_nc (unsigned int command, unsigned int port, char* file,
                      char *exec, char* ipaddr) 
{
    char buf[128];
    char *srv_buf ="-l -p ", *srv_exe="-e ", *cli_time="  ";
    char *recv =">", *send ="<", *bk_gd ="&"; 

    sprintf(buf, "nc");
    switch(command) {
    case CLI_REQ_RECV: /* cli_req_recv */
    sprintf(buf, "%s %s %s %d %s %s", buf, cli_time, ipaddr, port, recv, file);
    break; 
    case CLI_REQ_SEND: /* cli_req_send */
    sprintf(buf, "%s %s %s %d %s %s", buf, cli_time, ipaddr, port, send, file);
    break; 
    case CLI_REQ_TRIG: /* cli_req_trig */
    sprintf(buf, "%s %s %s %d", buf, cli_time, ipaddr, port);
    break; 
    case SVR_LSTN_RECV: /* svr_lstn_recv */
    sprintf(buf, "%s %s %d %s %s %s", buf, srv_buf, port, recv, 
                                      file, bk_gd);
    break; 
    case SVR_LSTN_SEND: /* svr_lstn_send */
    sprintf(buf, "%s %s %d %s %s %s", buf, srv_buf, port, send,
                                      file, bk_gd);
    break; 
    case SVR_LSTN_EXEC: /* svr_lstn_exec */
    sprintf(buf, "%s %s %d %s '%s' %s", buf, srv_buf, port, srv_exe, 
                                      exec, bk_gd);
    break; 
    }

    if ((NVRAM)->diagflag & D_VERBOSE) { 
        printf("nc command is: %s\n", buf);
    }
    return (system(buf));
}


/**********************************************************************
 *
 * Function: general_nc_wrapper
 *
 * Description: nc command wrapper
 *
 * Input : cmd - 6 of client and server command. 
 *         pnum - port number for nc
 *         fname - file name in case nc cmd need it .
 *         exec - execute cmd for nc to execute 
 *         ipaddr - subsystem ip address 
 *
 * Output: PASS or FAIL
 *
 **********************************************************************
 */
int general_nc_wrapper (unsigned int cmd, unsigned int pnum, char *fname,
                         char *exec, char *ipaddr)
{

    if (check_arg(cmd, pnum, fname, exec, ipaddr)) {
        printf("lack of args ...rerutn\n");
        return (FAILED);
    } else {
        return (diag_general_nc(cmd, pnum, fname, exec, ipaddr));
    }

}

/*---------------------------------------------------------------
$Log: diag_nc_common.c,v $
Revision 1.2  2016/04/20 11:25:29  benchen2
add tachi fru portion

Revision 1.1.2.5  2016/01/14 02:06:38  jskow
Add verbose mode and time stamps to GESW tests.  Add functions to transmit mid-test information from GESW to BMC.  Modify nc_client_lib to accept INFO status remotely from Intel/GESW to BMC.  Modify GESW test names for clarity.

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
