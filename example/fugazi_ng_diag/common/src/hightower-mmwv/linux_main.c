/* $Id: linux_main.c,v 1.2 2021/06/02 02:56:21 alpeng Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/hightower-mmwv/linux_main.c,v $
 *********************************************************************
 *
 * linux_main.c - diagnostic main entry.
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
#include "hightower_mmwv.h"
#include "diag_ge_phy_lib.h"

int quiet_launch = 0;
int ht_version_v2 = 0; 
extern char *banner_string;
extern void diag_menu (int argc, const char *argv[]);
extern int is_plat_p2(void);
extern int mem_ecc_check(char *);

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
    char ecc_log[32];
    int rc = PASSED;

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

    /* Up PHY 1514 - eth1 */
    system(ETH_PHY_1514_GE_DOWN);
    system(ETH_PHY_1514_GE_UP);
    sleep(1);

    /* TBD - Currently no better way to isolate mem ECC */
    /* Channel 0 ECC 1bit Error Counter */
    system("devmem 0xf0020364 32 > /var/log/ecc_ch0.txt");
    sprintf(ecc_log, "/var/log/ecc_ch0.txt");
    rc = mem_ecc_check(ecc_log);
    if (rc == FAILED) {
        /* ECC error returns FAILED */
        return (rc);
    }

    /* Channel 1 ECC 1bit Error Counter */
    system("devmem 0xF0020564 32 > /var/log/ecc_ch1.txt");
    sprintf(ecc_log, "/var/log/ecc_ch1.txt");
    rc = mem_ecc_check(ecc_log);
    if (rc == FAILED) {
        /* ECC error returns FAILED */
        return (rc);
    }

    /* Plaform init */
    if (ht_init()) {
        printf("Hightower init failed\n");
       // return (FAILED); // fail through before ER, afix 
    }

    /* get vid for platftom */
    ht_version_v2 = is_plat_p2(); 


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
Revision 1.2  2021/06/02 02:56:21  alpeng
merge sears into trunk

Revision 1.1.4.2  2020/10/13 06:23:38  leschen
Read out and check the value of CN9130 ECC error counter regs when executing DIAG image.

Revision 1.1.4.1  2020/08/27 07:19:33  alpeng
apply cvs header

Revision 1.1  2020/08/19 09:50:05  markzha
*** empty log message ***


$Endlog$
*/

/*********************************************************************
 * $Log: linux_main.c,v $
 * Revision 1.2  2021/06/02 02:56:21  alpeng
 * merge sears into trunk
 *
 * Revision 1.1.4.2  2020/10/13 06:23:38  leschen
 * Read out and check the value of CN9130 ECC error counter regs when executing DIAG image.
 *
 * Revision 1.1.4.1  2020/08/27 07:19:33  alpeng
 * apply cvs header
 *
 *
 * $Endlog$
 */

