/* $Id: main.c,v 1.2 2018/05/18 09:24:56 alpeng Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/overlord/neptune/cavium/main.c,v $
 *------------------------------------------------------------------
 * main.c - main program for Neptune Cavium data plane
 *          Leveraged from overlord/cavium
 *
 * May 2016, Xiaoying Zhang
 * Copyright (c) 2016-2018 by Cisco Systems, Inc.
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
#include "../../cavium/host2dp_mbox.h"
#include "platform_eth.h"
#include "plat_defs.h" /* board_type */
#include "cross_platform.h" /* board_type */

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
static int board_type = BDTYPE_UNKNOWN;

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
void get_dash_fpga_ver(uint32_t *fpga_ver, uint32_t *fpga_brd, 
                       uint32_t verbose)
{
    sys_lvl_t  *fpga  = (sys_lvl_t*)dash_fpga;

    assert(dash_fpga);
    *fpga_ver = fpga->ver;
    *fpga_brd = fpga->brd;

    if (verbose) {
        printf("fpga version @%#lx=%#x; fpga brd info: @%#lx=%#x \n",
               (unsigned long)&fpga->ver - dash_fpga, *fpga_ver,
               (unsigned long)&fpga->brd - dash_fpga, *fpga_brd);
        printf("board rev=%#x; fpga rev major=%#x; minor=%#x\n",
               (fpga->ver & FPGA_BD_HW_REV_MSK) >> FPGA_BD_HW_REV_SHFT,
               (fpga->ver & FPGA_MAJOR_REV_MSK) >> FPGA_MAJOR_REV_SHFT,
               (fpga->ver & FPGA_MINOR_REV_MSK) >> FPGA_MINOR_REV_SHFT );
    }
}

/*****************************************************************************
 *
 * Function   : set_board_type
 * Description: set board type variable to distinguish overlord or juno 
 * Inputs     : none  
 *
 * Outputs    : none  
 *
 *****************************************************************************/
void
set_board_type (void)
{
    unsigned int fpga_ver = 0, fpga_brd = 0;
    unsigned int brd_type, brd_subtype;

    get_dash_fpga_ver(&fpga_ver, &fpga_brd, 0);
    brd_type = ((fpga_brd & FPGA_BD_TYPE_MSK) >> FPGA_BD_TYPE_SHFT);
    brd_subtype = (((fpga_brd & FPGA_BD_SUBTYPE_HI_MSK) >> FPGA_BD_SUBTYPE_HI_SHFT) |
                   (fpga_brd & FPGA_BD_SUBTYPE_LO_MSK));

    if (brd_type == FPGA_BD_TYPE_ROUTE_PROC) {
        switch(brd_subtype) {
        case FPGA_BD_SUBTYPE_OVLD:
            board_type = BDTYPE_OVERLORD;
            printf("FPGA board type is OVERLORD\n");
            break;
        case FPGA_BD_SUBTYPE_JUNO:
            board_type = BDTYPE_JUNO;
            printf("FPGA board type is JUNO\n");
            break;
        case FPGA_BD_SUBTYPE_UTAH:
            board_type = BDTYPE_UTAH;
            printf("FPGA board type is UTAH\n");
            break;
        case FPGA_BD_SUBTYPE_SWORD:
            board_type = BDTYPE_SWORD;
            printf("FPGA board type is SWORD\n");
            break;
        case FPGA_BD_SUBTYPE_DAGGER:
            board_type = BDTYPE_DAGGER;
            printf("FPGA board type is DAGGER\n");
            break;
        case FPGA_BD_SUBTYPE_NEPTUNE:
        case FPGA_BD_SUBTYPE_NEPTUNE_TMP:
        case FPGA_BD_SUBTYPE_NEPTUNE_TMP_1:
            board_type = BDTYPE_NEPTUNE;
            printf("FPGA board type is NEPTUNE\n");
            break;
        case FPGA_BD_SUBTYPE_TRITON:
            board_type = BDTYPE_TRITON;
            printf("FPGA board type is TRITON\n");
            break;
        case FPGA_BD_SUBTYPE_PROTEUS:
            board_type = BDTYPE_PROTEUS;
            printf("FPGA board type is PROTEUS\n");
            break;
        case FPGA_BD_SUBTYPE_NESO:
            board_type = BDTYPE_NESO;
            printf("FPGA board type is NESO\n");
            break;
        case FPGA_BD_SUBTYPE_GOLDBEACH:
            board_type = BDTYPE_GOLDBEACH;
            printf("FPGA board type is GOLDBEACH\n");
            break;
        case FPGA_BD_SUBTYPE_VG450:
            board_type = BDTYPE_VG450;
            printf("FPGA board type is VG450\n");
            break;
        }
    }
    if (board_type == BDTYPE_UNKNOWN) {
        cterr('f',0,"FPGA board type unknown. brd type reg= %#.8x", fpga_brd);
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

    if (strncmp(buf, "0-15", 4) == 0) {
        printf("DP: Octeon is CN7260R\nTotal detected processor number: 16\n");
	send_msg(MBOX_MSG_DP_CN7260R);
    }
    else if (strncmp(buf, "0-9", 3) == 0) {
        printf("DP: Octeon is CN7245R\nTotal detected processor number: 10\n");
	send_msg(MBOX_MSG_DP_CN7245R);
    }
    else if (strncmp(buf, "0-5", 3) == 0) {
        printf("DP: Octeon is CN7235R\nTotal detected processor number: 6\n");
        send_msg(MBOX_MSG_DP_CN7235R);
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
int main (int argc, const char *argv[])
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
    printf("core_num: %d\n", core_num);

    /* Initialize the sysinfo structure when running on
     * Octeon under Linux userspace
     */
    cvmx_linux_enable_xkphys_access(0);
    cvmx_sysinfo_linux_userspace_initialize();

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

    /* Mail box to communicate with Intel
     */
    dp_mbox_init(CVMX_NEPTUNE);

    /* Report CPU model
     */
    report_cpu_model();

    /* FPGA shows board type */
    set_board_type();

    diag_menu(argc, argv); /* goto menu directly; */

    munmap(in_mbxp, sizeof(mbox_t));
    munmap(out_mbxp, sizeof(mbox_t));
    close(cavium_i2c_fd0);
    close(cavium_i2c_fd1);

    return(0);
}

/*-------------------------------------------------
$Log: main.c,v $
Revision 1.2  2018/05/18 09:24:56  alpeng
 Neptune merge to trunk with tag <neptune-branch-0518>

Revision 1.1.2.10  2017/11/24 09:22:28  leschen
Support Barsoom VG450.

Revision 1.1.2.9  2017/07/05 02:38:11  alpeng
fixed for prrq comments

Revision 1.1.2.8  2017/04/10 01:09:14  alpeng
add fpga board type check

Revision 1.1.2.7  2016/11/15 07:16:53  alpeng
resolve cvm 2nd test issue

Revision 1.1.2.6  2016/11/03 08:26:54  alpeng
merge octeon_test.c with o2

Revision 1.1.2.5  2016/11/01 11:36:17  alpeng
support mbox for neptune

Revision 1.1.2.4  2016/10/14 00:52:19  alpeng
add lib for mdio access

Revision 1.1.2.3  2016/07/20 06:36:23  xiaoyizh
Use arch=octeon3 instead of octeon and remove -msoft-float.

Revision 1.1.2.2  2016/06/20 09:38:17  xiaoyizh
Display the correct CPU name according to core number.

Revision 1.1.2.1  2016/06/06 05:58:51  xiaoyizh
Initial Check-in for Neptune Data Plane diags.

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
