/* $Id: full_load_util.c,v 1.4 2019/07/11 12:31:30 alicehua Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/nutella/full_load_util.c,v $
 *------------------------------------------------------------------
 *
 * full_load_util.c - Full loading utility
 *
 * Copyright (c) 2019 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#include "common.h"
#include "types.h"
#include "error.h"
#include "common_utils.h"
#include "plat_defs.h"
#include "full_load_util.h"

extern int ExecuteCmdbyPopen(char *cmd, char *retBuf, int sizeOfBuf);

/**********************************************************************
 *
 * Function: diag_full_load_util
 *
 * Description: display system info, ex. current time, Diag ver
 *
 * Input : dummy
 *                     
 * Output: None
 *
 **********************************************************************
 */

int diag_full_load_util (void)
{
    char buf[BUF_SIZE];
    int buf_len = BUF_SIZE;
    FILE *f;
    
    memset(buf, 0, sizeof(buf));
    
    printf("If you want to stop full load utility, please input s to stop\n");
    if(getc_answer("Do you want to start full load utility? Please input y to confirm", "yn", 'n') == 'y') {
        printf("Prepare to start : \n1.Memory test\n2.USB R/W\n3.eMMC R/W\nInput s has no effect in this stage\n");
    } else {
        return (PASSED);
    }
    
    f = fopen("/opt/tool/full_script", "w");
    if (f == NULL) {
        printf("Error opening file!\n");
    }
    fprintf(f, "%s", FULL_SCRIPT);
    fclose(f);
    system("chmod +x /opt/tool/full_script");
    
    system("umount /mnt 2>/dev/null; umount /mnt/* 2>/dev/null; nice -n 10 full_script");

    memset(buf, 0, sizeof(buf));
    ExecuteCmdbyPopen("ps -aux| grep  memtester | wc -l", buf, buf_len);
    if(strlen(buf) <= 0) {
        printf("Memory test execute fail!\n");
    }
        
    while(getc_answer("Starts fully loading utility\nPlease input \"s\" anytime to stop this utility\n", "s", ' ') != 's');
    
    system("killall usbrw 2>/dev/null; killall emmcrw 2>/dev/null; killall dd 2>/dev/null; killall memoryrw 2>/dev/null; killall memtester 2>/dev/null; killall full_script 2>/dev/null");
    printf("Full Load Utility is stopped!");
    return (PASSED);
}

/******** History ********
$Log: full_load_util.c,v $
Revision 1.4  2019/07/11 12:31:30  alicehua
Collapse Nutella codes into main trunk.

$Endlog$
*/
