/* $Id: linux_ntwk.c,v 1.11 2014/07/17 23:07:06 mcharon Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/overlord/linux_ntwk.c,v $
 *------------------------------------------------------------------
 * by: mcharon
 * 5/12
 *
 * Copyright (c) 2014 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include "linux_ntwk.h"
#include "common.h"
#include "linux_api.h"
#include "dash_fpga.h"
#include "plat_defs.h"

extern int tftp_main(int argc, char *argv[]);
static char *tftp_srv_name[] = { "TFTP_SERVER",
                                 "TFTP_SERVER_BKUP0",
                                 "TFTP_SERVER_BKUP1",
                                 "TFTP_SERVER_BKUP2",
                                 "TFTP_SERVER_BKUP3",
                                 "TFTP_SERVER_BKUP0",
                                 "TFTP_SERVER_BKUP1",
                                 "TFTP_SERVER_BKUP2",
                                 "TFTP_SERVER_BKUP3",
                                 "TFTP_SERVER_BKUP0",
                                 "TFTP_SERVER_BKUP1",
                                 "TFTP_SERVER_BKUP2",
                                 "TFTP_SERVER_BKUP3",
                                 "TFTP_SERVER_BKUP0",
                                 "TFTP_SERVER_BKUP1",
                                 "TFTP_SERVER_BKUP2",
                                 "TFTP_SERVER_BKUP3",
                                 "\0"
};

/* tftpc -h 223.255.254.254 -r mcharon/fw_file /firmware/fw_file */
#define COMMAND "tftpc -h %s -r %s%s %s"

int get_mgmnt_port(void)
{
    int port;
    if (is_overlord()) {
        port = MGMNT_PORT_OVERLORD;
    } else if (is_utah()) {
        port = MGMNT_PORT_UTAH;
    } else if (is_sword()) {
        port = MGMNT_PORT_SWORD;
    } else if (is_dagger()) {
        port = MGMNT_PORT_DAGGER;
    } else {
        port = 0;
        cterr('w', 0, "WARNING: Board type not supported, set mgmnt port to 0");
    }
    return port;
}

/*
 * Function_name: tftp_get
 * Description; perform tftp in this order:.
 * 1. software will get the server ip from environment variable TFTP_SERVER,
 * 2. if the variables are not defined, software will use parameter "server_ip"
 * passed into the function.
 * 3. if this parameter is NULL, then software will use 223.255.254.252.
 * if tftp fails, repeat step 1, but this time ip will come from
 * variable TFTP_SERVER_BKUPx (where x = 0, 1, 2, 3), and then repeat step 2
 * and 3 if necessary.
 * with each failure, repeat steps 1,2,3 until x > 3.
 * 
 * (NOTE: enviromental variable can be set by using this command:
 *       export TFTP_SERVER_BKUP0="xxx.xxx.xxx.xxx")
 * Input   dir: directory of the remote file to retrieve. ie, if your firmware is
 *         located on 223.255.254.254:/mcharon/my_file.txt, then dir is "mcharon".
 *         file: name of the remove file to retrieve.
 *         server_ip: tftp server ip (ie, 223.255.254.254)
 *         dest: location to place file. this should be path + file name.
 *         check: flag set if we want to check if file alreay exists in destination.
 * sample usage:
 *          tftp_get("mcharon", "my_firmware", "223.255.254.254", "/tftp/my_firmware", 1);
 *          this means location of my_firmware is at 223.255.254.254:/mcharon/my_firmware
 *          and the file will be downloaded to local directory /tftp.
 * output: return negative value if tftp fails
 *
*/
int
tftp_get (char *dir, char *file, char *server_ip,
	  char *dest, int check)
{
    char buf[512];
    char tftp_server[BUFSIZ], tftp_dir[256], ping[80];
    char *argv[12];
    char *tk;
    int i, idx, ret;
    size_t size = 0;
    char *env_ip;
    char cmd[128];;

    /* special case if TFTP_SERVER env is set to none, do not try to
       download anything */
    if ((env_ip = getenv(tftp_srv_name[0]))) {
        if (!strcmp("none", env_ip)) {
	    printf("%s: Env variable TFTP_SERVER is not set\n", __FUNCTION__);
            return PASSED;
        }
    }

    sprintf(cmd, "ls -l --color=never %s\n", dest);
    if (check && file_exist(dest, &size)) {
        printf("%s: File %s existed.\n", __FUNCTION__, dest);
	system(cmd);
        if (size > 0)
            return(PASSED);
    }

    memset(tftp_dir, 0, sizeof(tftp_dir));

    if (!dir) {
        dir = getenv("TFTPDIR");
        if (dir) {
            if (strlen((char *)dir)) {
                sprintf(tftp_dir, "%s/", (char *)dir);
            } else {
                sprintf(tftp_dir, "%s", (char *)dir);
            }
        } else {
            //            sprintf(tftp_dir, "");
        }
        
    } else {
        sprintf(tftp_dir, "%s/", (char *)dir);
    }

    for (i = 0, size = 0; tftp_srv_name[i][0] != '\0'; i++) {
        /* env variable ip will overide server ip passed into this function */
        env_ip = getenv(tftp_srv_name[i]);
        if (env_ip) {
            sprintf(tftp_server, env_ip);
        } else {
            /* initialize tftp_server to either server_ip or 223.255.254.252 */
            if (server_ip)
                sprintf(tftp_server, server_ip);
            else
                sprintf(tftp_server, "223.255.254.254");
        }
        if (i > 0) {
            sprintf(ping, "ping -c5 %s", tftp_server);
            system(ping);
        }
        snprintf(buf, sizeof(buf), COMMAND,
                 tftp_server,
                 tftp_dir,
                 file,
                 dest);
        printf("\nrunning tftpc ");

        tk = strtok(buf, " ");
        idx = 0;
        argv[idx++] = tk;
        while (tk != NULL) {
            tk = strtok(NULL, " ");
            argv[idx++] = tk;
            if (tk)
                printf("%s ", tk);

        }
        printf("\n");
        idx--;

        ret=tftp_main(idx, argv);
        /* if file not found, no need to retry, just return failure */
        if (ret == -1) {
           return(FAILED);
        }

        /* tftp successful, we still need to check file size. if it's 0,
         lets try again. */
        if (file_exist(dest, &size) && (size > 0 )) {
	    system(cmd);
            return(PASSED);
        }
    }
    
    return -1;
}


/*-------------------------------------------------
$Log: linux_ntwk.c,v $
Revision 1.11  2014/07/17 23:07:06  mcharon
if tftp failes before file not found, don't retry

Revision 1.10  2014/07/10 23:23:38  mcharon
tftpmain returns errno instead of -1 when failed. try pinging to server if download failed first time to make sure network is up

Revision 1.9  2014/04/22 06:18:21  alpeng
not support utah P1A anymore; remove is_utah_p1a()

Revision 1.8  2014/04/03 21:30:46  ptong
Display firmware size in tftp_get

Revision 1.7  2014/01/20 02:12:02  mcharon
if tftp fails, retry with a different server. get server ip from evniroanmental variable

Revision 1.6  2013/12/18 00:24:40  mcharon
file_exist now returns size of file

Revision 1.5  2013/11/26 08:40:35  hroni
fix compiler warning

Revision 1.4  2013/10/16 12:53:50  hroni
remove redundant code

Revision 1.3  2013/10/16 04:20:54  hroni
do arping and tftp retry for max: 3 times if previous tftp download was failed

Revision 1.2  2013/10/16 03:46:29  hroni
add get_mgmnt_port() to provide the correct port for each platform. 

Revision 1.1  2013/05/09 05:42:36  alpeng
moving overlord common code from x86

Revision 1.15  2013/01/14 21:45:06  mcharon
move file_exist to linux_api.c

Revision 1.14  2012/12/20 21:58:32  mcharon
call tftp_main() instead of making system call to download file

Revision 1.11
if download fails, file doesn't exist so check file and return error

Revision 1.10  2012/11/06 20:39:50  mcharon
add headers/cleanup/remove unneeded functions/files

Revision 1.9  2012/11/02 00:00:32  mcharon
skip tftp download if env variable TFTP_SERVER==none

Revision 1.8  2012/08/22 19:53:07  mcharon
back up previou commit

Revision 1.6  2012/07/25 20:19:42  mcharon
in tftpg_get make default directory currently directory

Revision 1.5  2012/06/20 20:34:27  mcharon
empty string for TFTPDIR

$Endlog$
*/
