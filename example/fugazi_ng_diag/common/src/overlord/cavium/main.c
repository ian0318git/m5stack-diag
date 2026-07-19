/* $Id: main.c,v 1.15 2018/05/18 09:24:52 alpeng Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/overlord/cavium/main.c,v $
 *------------------------------------------------------------------
 * main.c - main program for Overlord Cavium data plane
 *
 * March 2011, Paul Tong
 * Copyright (c) 2011-2018 by Cisco Systems, Inc.
 * All rights reserved.
 *-----------------------------------------------------------------------------
 */
#include <stdio.h>
#include <unistd.h>
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
#include "pcmap.h"
#include "dash_fpga.h"
#include "host2dp_mbox.h"
#include "platform_eth.h"

#include "cvmx.h"
#include "cvmx-atomic.h"
#include "cvmx-sysinfo.h"
#include "cvmx-coremask.h"
#include "cvmx-sysinfo.h"

extern char *banner_string;
extern void diag_menu (int argc, const char *argv[]);
extern int check_cavium_eeprom_loaded (void);

volatile unsigned char hkeepflags;
jmp_buf monjmpbuf, *monjmpptr;

#define CAV_I2C_OCTEON0   0
#define CAV_I2C_OCTEON1   1

int32_t cavium_i2c_fd0 = -1;
int32_t cavium_i2c_fd1 = -1;
char cav_err_buf[80];
static char inf_name[32];

unsigned long dash_msg = 0;
unsigned long dash_fpga = 0;
unsigned long dash_cpld = 0;

/* 
 * Function: get_platform_plane
 * FPGA can be access from either the host cpu or cavium.
 * The FPGA API provide on the host side require this
 * to pick the correct address offset for the right cpu.
 *
 * Input: none
 *
 * Return: FP is a macro define to 1
 */
int
get_platform_plane(void)
{
    return FP;  /*defined in dash_fpga.h */
}

/* 
 * Function: get_dash_fpga_ver
 *     Read the FPGA revision register which has the board
 *     and FPGA revision numbers.
 *
 * Input: fpga_ver - used to pass the reg value to caller
 *        verbose - flag to print or not
 *
 * Return: void
 */
void get_dash_fpga_ver(uint32_t *fpga_ver, uint32_t verbose)
{
    sys_lvl_t  *fpga  = (sys_lvl_t*)dash_fpga;

    assert(dash_fpga);
    *fpga_ver = fpga->ver;

    if (verbose) {
        printf("ovld board rev=%#x; fpga rev major=%#x; minor=%#x\n",
               (fpga->ver & FPGA_BD_HW_REV_MSK) >> FPGA_BD_HW_REV_SHFT,
               (fpga->ver & FPGA_MAJOR_REV_MSK) >> FPGA_MAJOR_REV_SHFT,
               (fpga->ver & FPGA_MINOR_REV_MSK) >> FPGA_MINOR_REV_SHFT );
    }
}

/* 
 * Function: report_cpu_model
 *     Find out the number of cores of the Octeon CPU and report
 *     the information to the control plane host code.
 *
 * Input: void
 *
 * Return: void
 */
void report_cpu_model(void)
{
    FILE *fp;
    char buf[16];
    char *fname;

    /* Get info from /proc/cpuinfo file
     */
    fname = "/sys/devices/system/cpu/online";
    fp = fopen(fname, "r");
    if (fp == NULL) {
        printf("%s Failed to open %s\n", __FUNCTION__, fname);
	return;
    }

    fgets(buf, sizeof(buf), fp); // get line
    fclose(fp);

    if (strncmp(buf, "0-9", 3) == 0) {
        printf("DP: Octeon is CN6645\n");
	send_msg(MBOX_MSG_DP_CN6645);
    }
    else if (strncmp(buf, "0-5", 3) == 0) {
        printf("DP: Octeon is CN6635\n");
	send_msg(MBOX_MSG_DP_CN6635);
    }
    else {
        printf("DP: Octeon model is unknown\n");
	send_msg(MBOX_MSG_DP_UNKNOWN);
    }

    return;
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

/*
 * Function: appmian
 *     The main() function for the Cavium based data plane diag
 *
 * Input: argc - C command line argument count
 *        argv - C command line arguments
 *
 * Retrun: 0/1
 */
int appmain (int argc, const char *argv[])
{
    uint32_t rc = FAILED;
    int core_num;
    char cmd[32], msg[32], fname[32];

    /* Linux reports the number of core which is up
     */
    core_num = cvmx_get_core_num();
    sprintf(fname, "core_%d", core_num);
    sprintf(msg, "Core %d: Linux is up", core_num);
    sprintf(cmd, "echo \"%s\" > %s;", msg, fname);
    system(cmd);
    sprintf(cmd, "uname -a >> %s;", fname);
    system(cmd);

    if (!cvmx_coremask_first_core(cvmx_sysinfo_get()->core_mask)) {
      return(0);
    }

    /* run the data plane diag with menu */
    printf("%s", banner_string);

    dash_fpga = DASH_FPGA_PHY_BASE_ADDR;
    dash_cpld = dash_fpga;

    /* To open I2C for I2C devices */
    rc = cavium_open_module(&cavium_i2c_fd0, CAV_I2C_OCTEON0);
    if (rc != PASSED) {
        cterr('f', 0, "%s", cav_err_buf);
        return (rc);
    }

    rc = cavium_open_module(&cavium_i2c_fd1, CAV_I2C_OCTEON1);
    if (rc != PASSED) {
        close(cavium_i2c_fd0);
        cterr('f', 0, "%s", cav_err_buf);
        return (rc);
    }

    /* Config the xaui0 port
     */
    config_xaui0();

    /* Mail box to communicate with Intel
     */
    dp_mbox_init(CVMX_OVERLORD);

    /* Report CPU model
     */
    report_cpu_model();

    diag_menu(argc, argv); /* goto menu directly; */

    munmap(in_mbxp, sizeof(mbox_t));
    munmap(out_mbxp, sizeof(mbox_t));
    close(cavium_i2c_fd0);
    close(cavium_i2c_fd1);

    return(0);
}

/*-------------------------------------------------
$Log: main.c,v $
Revision 1.15  2018/05/18 09:24:52  alpeng
 Neptune merge to trunk with tag <neptune-branch-0518>

Revision 1.14.66.1  2016/11/15 07:16:53  alpeng
resolve cvm 2nd test issue

Revision 1.14  2013/05/17 10:46:05  danchung
Remove cavium eeprom check on cavium side.

Revision 1.13  2013/02/15 01:09:50  ptong
Stop using common/src/linux_api.c in Cavium Linux

Revision 1.12  2012/12/17 22:18:25  ptong
Move banner string printing up

Revision 1.11  2012/11/02 00:55:51  ptong
Add comment and clean-up

Revision 1.10  2012/11/01 19:17:50  ptong
Support checking Cavium PCIe BAR 0-2 setting loaded from EEPROM

Revision 1.9  2012/08/10 22:51:38  ptong
Add version banner

Revision 1.8  2012/07/24 23:57:59  ptong
Bump up release string to 2.0.0 and add -Werror in Makefile

Revision 1.7  2012/06/19 23:20:14  ptong
Check correct Octeon model is used on the platform

Revision 1.6  2012/06/05 06:21:03  alpeng
clean up compiler warnings.

Revision 1.5  2012/05/27 22:30:51  ptong
Add version string and support coremask for multi-core boot

Revision 1.4  2012/04/17 22:01:26  ptong
Added more utility to run DP test from host.

Revision 1.3  2012/04/11 21:27:16  ptong
Setup cavium named block for mailbox area, and use nc server on cavium to take command from host

Revision 1.2  2012/03/28 00:38:18  mcharon
remove forward slash from second line

Revision 1.1.1.1  2012/03/23 23:02:07  ptong
Initial archive of ng_diag module


$Endlog$
*/
