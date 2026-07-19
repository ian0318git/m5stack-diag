/* $Id: wallander_main.c,v 1.2 2015/03/16 05:52:56 xiaoyizh Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/wallander/wallander_main.c,v $
 *------------------------------------------------------------------
 *
 * wallander_main.c - Wallander main entry.
 *
 * Xiaoying Zhang -- Feb. 2014
 *
 * Copyright (c) 2015 by Cisco Systems, Inc.
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
#include "nvsysvars.h"
// #include "nvmonvars.h"
#include "common.h"
#include "types.h"
// #include "pcmap.h"
#include "common_utils.h"
#include "diag_bootflash_lib.h"
#include "diag_fpga_lib.h"
#include "diag_ge_phy.h"
#include "diag_common_drv.h"
#include "cvmx.h"
#include "cvmx-sysinfo.h"

#define CAV_I2C_OCTEON0   0
#define CAV_I2C_OCTEON1   1

int32_t cavium_i2c_fd0 = -1;
int32_t cavium_i2c_fd1 = -1;
char cav_err_buf[80];
static char inf_name[32];

extern char *banner_string;

extern void diag_menu (int argc, const char *argv[]);
extern int diag_do_all (void);

/**************************************************************************
 *
 * Function: wallander_help
 *
 * Display help
 *
 * Input: None
 *
 * Return: None
 *
 * *************************************************************************
 */
static void wallander_help (void)
{
    printf("Usage: wallander [-a] [-j] [-h]\n\n");

    printf("Options:\n");
    printf("-a Do all the tests\n");
    printf("-j Do all the tests except external loopback\n");
    printf("-h Display this help\n");
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
/*        strcpy(inf_name, "/dev/i2c-0");
        *fd = open("/dev/i2c-0", O_RDWR);*/
        break;
    case CAV_I2C_OCTEON1:
        strcpy(inf_name, "/dev/i2c-octeon.1");
        *fd = open("/dev/i2c-octeon.1", O_RDWR);
/*        strcpy(inf_name, "/dev/i2c-1");
        *fd = open("/dev/i2c-1", O_RDWR);*/
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

/******************************************************************************
 *
 * Function: pem0_rc_cfg
 *  Configure PEM0 to RC mode
 *
 * Input:  None.
 *
 * Output: None.
 *
 *****************************************************************************/
void pem0_rc_cfg()
{
    uint64_t data = cvmx_read_csr(CVMX_PEMX_CFG(0));
    /* Set bit HOSTMD to 1 */
    data |= 0x8;
    cvmx_write_csr(CVMX_PEMX_CFG(0), data);
}

/******************************************************************************
 *
 * Function: listen_on_nc
 *  Listen on specific nc ports.
 *
 * Input:  None.
 *
 * Output: None.
 *
 *****************************************************************************/
void listen_on_nc(void)
{
    char cmd_str[32];

    sprintf(cmd_str, "ifconfig eth1 %s promisc", DIAG_IP_ADDR);
    printf("%s\n", cmd_str);
    system(cmd_str);

    sleep(ETH_DRIVER_DELAY);

    sprintf(cmd_str, "nc -l -l -p %d -e /home/wallander -a &",
        DIAG_DO_ALL_PORT_BASE);
    printf("%s\n", cmd_str);
    system(cmd_str);

    sprintf(cmd_str, "nc -l -l -p %d -e /home/wallander -j &",
        DIAG_DO_ALL_INT_PORT_BASE);
    printf("%s\n", cmd_str);
    system(cmd_str);

    sleep(ETH_DRIVER_DELAY * 10);
}

int main(int argc, const char *argv[])
{
/*    char arg;
    char cmd[32];*/
    int is_do_all          = FALSE;
    int is_menu_mode       = FALSE;
    int opt;
    int rc;

    /*
    * Initialize the sysinfo structure when running on
    * Octeon under Linux userspace
    */
    cvmx_linux_enable_xkphys_access(0);
    bootflash_init_virt_addr();

    if (argc > 1) {
        for (;;) {
            opt = getopt(argc, (char * const *)argv, "ahj");
            if (opt == EOF) {
                break;
            }

            switch (opt) {
            case 'h':  /* Help */
                wallander_help();
                exit(1);
                break;
            case 'a':  /* Do all */
                is_do_all = TRUE;
                break;
            case 'j':  /* Do all except external loopback tests */
                (NVRAM)->diagflag |= (D_EXT_LOOPBACK);
                is_do_all = TRUE;
                break;
            default:
                wallander_help();
                exit(1);
                break;
            }
        }
    } else {
        /* Menu mode if no option is provided */
        is_menu_mode   = TRUE;
    }

    fflush(stdin);

    if (is_menu_mode == TRUE) {
        /* For do-all mode, Phy is supposed to be initialized */
        if (phy_default_config(FALSE, 1)) {
            cterr('f', 0, "PHY Initialize Failed.");
        }
        listen_on_nc();
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

    /* Turn off over commit to avoid oom during memory test. */
    system("echo 2 > /proc/sys/vm/overcommit_memory");
    system("echo 80 > /proc/sys/vm/overcommit_ratio");

    /* PCIe is unused, overwrite the PEM0 to RC mode */
    pem0_rc_cfg();

    /* Set Ready Pin */
    if (fpga_set_ready_bit()) {
        printf("Warning: Failed to set Primary Interface Ready\n");
    }

    printf("%s", banner_string);

    if (is_menu_mode == TRUE) {
        diag_menu(1, argv); /* goto menu directly; */
    } else{
        if (is_do_all == TRUE) {
            /* Do all tests from here */
            diag_do_all();
        }
    }

    return(PASSED);
}

/******** History ********
$Log: wallander_main.c,v $
Revision 1.2  2015/03/16 05:52:56  xiaoyizh
Fix typo and increase the delay for nc server to be ready.

Revision 1.1  2015/02/26 07:18:30  xiaoyizh
Initial check in for Wallander.


$Endlog$
*/
