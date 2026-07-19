/* $Id: main.c,v 1.5 2015/03/31 06:28:58 leschen Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/woodlawn_linux/main.c,v $
 *------------------------------------------------------------------
 * main.c - main program for Woodlawn Cavium data plane
 *
 * January 2012, Kody Ko
 * Copyright (c) 2015 by Cisco Systems, Inc.
 * All rights reserved.
 *-----------------------------------------------------------------------------
 */
#include <stdio.h>
#include <fcntl.h>
#include <time.h>
#include <fcntl.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include "types.h"
#include "common.h"
#include "setjmps.h"
#include "linux_api.h"
#include "pcmap.h"
#include "platform_eth.h"
#include "platform_xaui.h"

#include "cvmx.h"
#include "cvmx-atomic.h"
#include "cvmx-sysinfo.h"
#include "cvmx-coremask.h"
#include "cvmx-sysinfo.h"

#include "diag_fpga_lib.h"
#include "diag_ge_phy_88E1340_lib.h"
#include "diag_ge_phy_88E1548L_lib.h"
#include "diag_tlk10232_lib.h"
#include "nvsysvars.h"
#include "queryflags.h"
#include "diag_ge_phy_88E1112C_lib.h"

extern void diag_menu(int argc, char *argv[]);
extern int diag_do_all(void);
extern void diag_nc_dispatch_comm(void);
static void woodlawn_help(void);
extern int getopt(int argc, char * const *argv, const char *optstring);

volatile unsigned char hkeepflags;
jmp_buf monjmpbuf, *monjmpptr;

#define CAV_I2C_OCTEON0   0
#define CAV_I2C_OCTEON1   1

int32_t cavium_i2c_fd0 = -1;
int32_t cavium_i2c_fd1 = -1;
char cav_err_buf[80];
static char inf_name[32];

extern char *banner_string;


uint32_t cavium_open_module(int32_t *, uint);

int appmain (int argc, char *argv[])
{
    uint32_t rc = FAILED;
    int core_num;
    char cmd[32], msg[32], fname[32];
    int is_do_all          = FALSE;
    int is_init_hw         = FALSE;
    int is_menu_mode       = FALSE;
    int is_display_ver     = FALSE;
    int is_set_tlk10232_lpbk     = FALSE;
    int is_set_1112_lpbk     = FALSE;
    int is_clear_1112_lpbk = FALSE;
    int is_clear_tlk10232_lpbk = FALSE;
    int opt;
    char *ext_lpbk_flag, *stop_on_err_flag;
    char *min_test_flag, *verbose_flag;

    core_num = cvmx_get_core_num();
    sprintf(fname, "core_%d", core_num);
    sprintf(msg, "Core %d: Linux is up", core_num);
    sprintf(cmd, "echo \"%s\" > %s;", msg, fname);
    system(cmd);
    sprintf(cmd, "uname -a >> %s;", fname);
    system(cmd);

    if (!cvmx_coremask_first_core(cvmx_sysinfo_get()->core_mask)) {
      exit(0);
    }

    /* To open I2C for I2C devices */
    rc = cavium_open_module(&cavium_i2c_fd0, CAV_I2C_OCTEON0);
    if (rc != PASSED) {
        cterr('f', 0, "%s", cav_err_buf);
        return (rc);
    }

    rc = cavium_open_module(&cavium_i2c_fd1, CAV_I2C_OCTEON1);
    if (rc != PASSED) {
        cterr('f', 0, "%s", cav_err_buf);
        return (rc);
    }

    if (argc > 1) {
        for (;;) {
            opt = getopt(argc, argv, "avihlemcz");
            if (opt == EOF) {
                break;
            }

            switch (opt) {
            case 'h':  /* Help */
                woodlawn_help();
                exit(1);
                break;
            case 'a':  /* Do all */
                is_do_all = TRUE;
                break;
            case 'v':  /* Display version */
                is_display_ver = TRUE;
                break;
            case 'i':  /* Init hardware */
                is_init_hw = TRUE;
                break;
            case 'e':  /* NC Dispatch command */
                diag_nc_dispatch_comm();
                exit (0);
            case 'l':  /* Set up tlk10232 deep remote lpbk bit */
                is_set_tlk10232_lpbk = TRUE;
                break;
            case 'm':  /* Set up marvell 1112 lpbk bit */
                is_set_1112_lpbk = TRUE;
                break;
            case 'c':  /* Clear marvell 1112 lpbk bit */
                is_clear_1112_lpbk = TRUE;
                break;
            case 'z':  /* Clear tlk10232 lpbk bit */
                is_clear_tlk10232_lpbk = TRUE;
                break;
            }
        }
    } else {
        /* Inits hardware, menu mode if no option is provided */
        is_init_hw     = TRUE;
        is_menu_mode   = TRUE;
        is_display_ver = TRUE;
    }

    if (is_init_hw == TRUE) {

        /* Init Quad PHY
         */
        reset_quad_phy();
        diag_88e1340_init();
        diag_88e1548_init();

        /* Config the xaui1 port
         */
        config_xaui();

        config_bp_xaui();
    }

     /* Turn off our Ext Lpbk flag if the environment variable passed by the host
     * is turned on
     */
    ext_lpbk_flag = getenv("ext_lpbk");
    if (ext_lpbk_flag != NULL) {
        if (strstr(ext_lpbk_flag, "FALSE")) {
            (NVRAM)->diagflag &= ~(D_EXT_LOOPBACK);
        } else {
            (NVRAM)->diagflag ^= (D_EXT_LOOPBACK);
        }
    }

    stop_on_err_flag = getenv("stop_on_err");
    if (stop_on_err_flag != NULL) {
        if (strstr(stop_on_err_flag, "FALSE")) {
            (NVRAM)->diagflag &= ~(D_STOPONERR);
        } else {
            (NVRAM)->diagflag ^= (D_STOPONERR);
        }
    }

    min_test_flag = getenv("min_test");
    if (min_test_flag != NULL) {
        if (strstr(min_test_flag, "FALSE")) {
            diagflag_xram &= ~(D_MIN_TEST_TIME);
        } else {
            diagflag_xram ^= (D_MIN_TEST_TIME);
        }
    }

    verbose_flag = getenv("verbose");
    if (verbose_flag != NULL) {
        if (strstr(verbose_flag, "FALSE")) {
            (NVRAM)->diagflag &= ~(D_VERBOSE);
        } else {
            (NVRAM)->diagflag ^= (D_VERBOSE);
        }
    }

    if (is_display_ver == TRUE) {
        printf(banner_string);
        printf("\n");
    }

    if (is_menu_mode == TRUE) {
        diag_menu(argc, argv); /* goto menu directly; */
    } else {
        if (is_do_all == TRUE) {
            /* Do all tests from here */
            diag_do_all();
        }
    }

    if (is_set_tlk10232_lpbk == TRUE) {
        /* Set up TLK10232 deep remote lpbk bit */
        set_tlk10232_lpbk_bit(TLK_10232_SET_LPBK); 
    }

    if (is_set_1112_lpbk == TRUE) {
        /* Set up 1112 lpbk bit */
        setting_1112_lpbk_bit(SET_1112C_LPBK_BIT); 
    }

    if (is_clear_1112_lpbk == TRUE) {
        /* Clear 1112 lpbk bit */
        setting_1112_lpbk_bit(CLEAR_1112C_LPBK_BIT); 
    }

    if (is_clear_tlk10232_lpbk == TRUE) {
        /* Clear tlk10232 lpbk bit */
        set_tlk10232_lpbk_bit(TLK_10232_CLEAR_LPBK); 
    }

    return 0;
}

/**************************************************************************
 *
 * Function: woodlawn_help
 *
 * Display help
 *
 * Input: None
 *
 * Return: None
 *
 * *************************************************************************
 */
static void woodlawn_help (void)
{
    printf("Usage: woodlawnnet [-a] [-h] [-i] [-r] [-v]\n\n");

    printf("Options:\n");
    printf("-a Do all the tests, Local/Backplane(with -r)\n");
    printf("-h Display this help\n");
    printf("-i Initialize hardware\n");
    printf("-r Run from backplane, using netcat\n");
    printf("-v Display version, Local/Backplane(with -r)\n");
    printf("\n");
}


/*****************************************************************************
 *
 * Function   : cavium_open_module
 * Description: Wrap function to open module
 * Inputs     : The file descriptor pointer of module and module_type
 * Outputs    : PASSED/FAILED
 *
 *****************************************************************************/
uint32_t cavium_open_module (int32_t *fd, uint module_type)
{
    uint rc = FAILED;

    /* Clear error and inf_name buffer */
    memset(&cav_err_buf[0], 0, sizeof(cav_err_buf));
    memset(&inf_name[0], 0, sizeof(inf_name));

    switch (module_type) {
    case CAV_I2C_OCTEON0:
        strcpy(inf_name, "/dev/i2c-octeon.0");
        *fd = open("/dev/i2c-octeon.0", O_RDWR);
        break;
    case CAV_I2C_OCTEON1:
        strcpy(inf_name, "/dev/i2c-octeon.1");
        *fd = open("/dev/i2c-octeon.1", O_RDWR);
        break;
    default:
        strcpy(inf_name, "Unknown");
        sprintf(cav_err_buf, "%s:Line%d %s interface %#x\n",
                             __FUNCTION__, __LINE__, inf_name, module_type);
        return (rc);
    }

    if (*fd <= 0) {
        sprintf(cav_err_buf, "%s: Line%d Failed to open %s\n",
                              __FUNCTION__, __LINE__, inf_name);
        return (rc);
    }
    return (PASSED);
}


/*-------------------------------------------------
 * $Log: main.c,v $
 * Revision 1.5  2015/03/31 06:28:58  leschen
 * Don't execute KR init script when kernel up.
 *
 * Revision 1.4  2014/11/12 06:32:59  leschen
 * Support Greyhound switch a
 *
 * Revision 1.3  2014/02/20 10:38:50  leschen
 * Fix parsing flags code for diag_do_all and diag_menu functions use.
 *
 * Revision 1.2  2013/10/08 08:48:30  tirawan
 * Woodlawn collapsed to main trunk
 *
 * Revision 1.1.4.2  2013/08/20 10:59:10  tirawan
 * Branch into woodlawn-branch2 and port woodlawn code
 *
 * Revision 1.1.2.3  2013/06/13 11:42:44  tirawan
 * Implement LED nc dispatch command for host side to be able to control SM LED
 *
 * Revision 1.1.2.2  2013/05/29 08:38:06  leschen
 * Synchronize with O2 flags
 *
 * Revision 1.1.2.1  2013/04/24 10:37:23  tirawan
 * Initial check-in for woodlawn linux code
 *
 * Revision 1.11  2013/04/09 14:16:07  kuangik
 * Don't setup TLK10232 loopback bit in default
 *
 * Revision 1.10  2013/04/09 11:05:37  leslie
 * Configure TLK10232 deep remote lpbk bit
 *
 * Revision 1.9  2013/04/02 13:58:29  kuangik
 * Remove diag_bp_comm.h
 *
 * Revision 1.6  2013/03/29 03:30:24  kuangik
 * Assign IP Address of BP XAUI, initialize TLK10232 and restore TLK10232 path after running internal loopback
 *
 * Revision 1.3  2013/03/20 03:09:21  kuangik
 * Do not initialize ethernet interface as it will cause ping not working
 *
 * Revision 1.8  2012/12/11 01:01:33  leslie
 * Config XAUI1 port.
 *
 * Revision 1.7  2012/09/21 11:50:22  kody
 * Change the config_xaui0 to config_xaui.
 *
 * Revision 1.6  2012/09/05 22:55:15  kody
 * Add enable eth2 ~ 7 network interfaces.
 *
 * Revision 1.5  2012/08/03 10:16:55  evanli
 * Mapping to latest O2 source code on 20120726
 *
 * Revision 1.3  2012/07/25 01:34:30  leslie
 * Recover to previous revision 1.1.1.1
 *
 * Revision 1.1.1.1  2012/02/10 05:59:50  kody
 * Initial imports Woodlawn project code base.
 *
 * $Endlog$
 *-------------------------------------------------
 */
