/* $Id: linux_ntwk.c,v 1.2 2021/06/02 08:22:35 iachang Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/fugazi/linux_ntwk.c,v $
 *------------------------------------------------------------------
 * 
 * linux_ntwk.c
 *
 * Copyright (c) 2019-2020 by Cisco Systems, Inc.
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
int tftp_get (char *dir, char *file, char *server_ip,
	  char *dest, int check)
{
    char buf[1024];
    char tftp_server[64], tftp_dir[256], ping[128];
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
 * $Log: linux_ntwk.c,v $
 * Revision 1.2  2021/06/02 08:22:35  iachang
 * CSCvo59196-33 : Merge Fugazi from ASR1K-main-branch to ISR main trunk
 *
 * Revision 1.1.8.2  2020/08/26 02:37:49  iachang
 * Merge Fugazi code into main trunk
 *
 * Revision 1.1.6.3  2020/07/29 08:57:34  iachang
 * Code clean up.
 *
 * Revision 1.1.6.2  2019/03/14 03:48:36  letsai
 * Initial check in.
 *
 *
 *
 *
 * $Endlog$
 */
