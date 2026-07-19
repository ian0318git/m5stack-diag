 /*------------------------------------------------------------------
 *
 * main.c - Highrise diagnostic main entry.
 *
 * May 2019, markzha
 *
 * Copyright (c) 2019-2020 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
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
#include "highrise.h"

int quiet_launch = 0;
int hr_version_v2 = 0; 
extern char *banner_string;
extern void diag_menu (int argc, const char *argv[]);
extern int is_plat_p2(void);

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

    /* Plaform init */
    if (highrise_init()) {
        printf("Highrise init failed\n");
        return (FAILED);
    }

    /* get vid for platftom */
    hr_version_v2 = is_plat_p2(); 

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

/******** History ********
$Log: linux_main.c,v $
Revision 1.1  2020/08/19 09:49:35  markzha
*** empty log message ***


$Endlog$
*/
