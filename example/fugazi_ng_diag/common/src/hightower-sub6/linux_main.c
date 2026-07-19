/* $Id: linux_main.c,v 1.2 2021/06/02 02:56:23 alpeng Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/hightower-sub6/linux_main.c,v $
 *********************************************************************
 *
 * linux_main.c - Highrise diagnostic main entry.
 *
 * Copyright (c) 2020-2020 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *********************************************************************
 */

#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include "common.h"
#include "hightower_sub6.h"

int quiet_launch = 0;
extern char *banner_string;
extern void diag_menu (int argc, const char *argv[]);

/*****************************************************************************
 * Function   : main
 *
 * Description: entry point of reva
 *
 * Inputs     : argc, number of argument
 *              argv, command line arguments
 * Outputs    : exit status
 *****************************************************************************/
int main(int argc, const char *argv[])
{
    int is_do_all          = FALSE;
    int is_menu_mode       = FALSE;
    int opt;

    if (argc > 1) {
        for (;;) {
            opt = getopt(argc, (char* const*)argv, "ah");
            if (opt == EOF) {
                break;
            }

            switch (opt) {
            case 'h':  /* Help */
                exit(1);
                break;
            case 'a':  /* Do all */
                is_do_all = TRUE;
                break;
            default:
                exit(1);
                break;
            }
        }
    } else {
        /* Menu mode if no option is provided */
        is_menu_mode   = TRUE;
    }

    fflush(stdin);

    printf("%s", banner_string);

    /* Up PHY 3310 - eth0 */
    system(ETH_PHY_3310_GE_UP); 

    /* Plaform init */
    if (ht_init()) {
        printf("Hightower init failed\n");
    }

    if (is_menu_mode == TRUE) {
        diag_menu(1, argv); /* goto menu directly; */
    } else {
        if (is_do_all == TRUE) {
            /* Do all tests from here */
            return (PASSED);
        }
    }

    return(PASSED);
}

/*********************************************************************
 * $Log: linux_main.c,v $
 * Revision 1.2  2021/06/02 02:56:23  alpeng
 * merge sears into trunk
 *
 * Revision 1.1.4.5  2020/12/09 07:29:50  alpeng
 * add function prologue; remove redundant header; adding ifdef for header files;
 *
 * Revision 1.1.4.4  2020/12/09 06:35:02  alpeng
 * add cvs log field
 *
 * Revision 1.1.4.3  2020/12/09 01:52:02  alpeng
 * use C comment 
 *
 * Revision 1.1.4.2  2020/11/26 03:38:00  alpeng
 * support temperature interrupt test for all version
 *
 * Revision 1.1.4.1  2020/08/27 07:18:46  alpeng
 * apply cvs header
 *
 *
 * $Endlog$
 */

