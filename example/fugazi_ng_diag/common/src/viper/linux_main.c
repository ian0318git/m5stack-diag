 /* $Id: linux_main.c,v 1.3 2018/08/31 03:59:30 chieyang Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/viper/linux_main.c,v $
 *------------------------------------------------------------------
 *
 * linux_main.c
 *
 * This file is diagnostic main entry
 *
 * Copyright (c) 2009-2018 by Cisco Systems, Inc.
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
#include <assert.h>
#include <errno.h>
#include <getopt.h>
#include "error.h"
#include "common.h"
#include "types.h"
#include "nvmonvars.h"
#include "menu.h"
#include "setjmps.h"
#include "plat_defs.h"
#include "diag_fpga.h"
#include "platform_stub.h"
#include "diag_gephy_test.h"
#include "platform_cpu.h"
#include "diag_esw_lib.h"
#include "linux_main.h"
#include "diag_fpga_lib.h"
#include "dnv_gpio_lib.h"
#include "diag_fpga.h"
#include "diag_xdsl_test.h"
#include "diag_gephy_lib.h"
#include "diag_temp_snsr_test.h"

/*
 * Declare local function
 */

static int fd_i2c0 = -1;
static int fd_i2c1 = -1;
static int fd_fpga = -1;

/*
 * Global variables
 */
volatile unsigned char hkeepflags;
jmp_buf monjmpbuf, *monjmpptr;
static uint32_t plat_ngio_bus_num = 0xffff;

/*****************************************************************************
 *
 * Function   : get_i2c_fd
 * Description: return file descriptor for /dev/i2c-1 and /dev/i2c-0
 * Inputs     : i2c_bus
 * Outputs    : file desriptor for /dev/i2c-1 and /dev/i2c-0
 *
 *****************************************************************************/
int get_i2c_fd(int i2c_bus)
{
    if (i2c_bus == 1) {
        return fd_i2c1;
    } else {
        return fd_i2c0;
    }
}

/*******************************************************************************
 *
 * Function   :    get_ngio_pcie_bus_num
 * Description:    Function to get system PCIe bus number for NGIO slots
 * Inputs     :    void
 * Outputs    : bus number
 *
 *******************************************************************************
 */
uint32_t get_ngio_pcie_bus_num(void)
{
    return (plat_ngio_bus_num);
}

/*****************************************************************************
 *
 * Function   : viper_open_module
 * Description: Wrap function to open module
 * Inputs     : The file descriptor pointer of module and module_type
 * Outputs    : PASSED/FAILED
 *
 *****************************************************************************/
static uint viper_open_module (int *fd, const char *name)
{
    uint rc = FAILED;
    char device[80];

    sprintf(device, "/dev/%s", name);
    *fd = open(device, O_RDWR);
    if (*fd <= 0) {
        return (rc);
    }
    return (PASSED);
}

/*****************************************************************************
 *
 * Function   : open_fpga
 * Description: open fpga driver for mmap
 * Inputs     : NONE
 *              
 * Outputs    : return file descript of fpga driver
 *
 *****************************************************************************/
static int open_fpga (void)
{
    void *ptr;

    if (viper_open_module(&fd_fpga, VIPER_FPGA_DEV)==PASSED) {

        ptr = (void *)mmap(NULL, FPGA_SIZE, (PROT_READ | PROT_WRITE),
                           MAP_SHARED, fd_fpga, FPGA_MMAP_OFFSET);
        if (ptr == MAP_FAILED) {
            close(fd_fpga);
            perror("Error mmapping the file for FPGA");
            return (FAILED);
        }
        dash_fpga = (unsigned long)ptr;
    } else {
        printf("*****can't open fpga mmap driver....*******\n");
        return (FAILED);
    }
    return (PASSED);

}

/*******************************************************************************
 *
 * Function    : viper_cpu_ondie_temp
 * Description : Function to get CPU on die temperature in degree C.
 * Inputs      : opt - reserve for future use
 * Outputs     : PASSED/FAILED
 *
 ******************************************************************************* 
 */
int viper_cpu_ondie_temp (int opt)
{
    int cpu_temp = 0;
    int buf_len=512;
    char buf[buf_len];

    memset(buf, 0, sizeof(buf));

    ExecuteCmdbyPopen("sensors | grep \"Package id 0\" | awk '{print $4}' | tr -d + |"
                      " awk -F . '{print $1}'", buf, buf_len);
    cpu_temp =  atoi(buf);
    printf("Current CPU on-die Temp. = %d degree C.\n", cpu_temp);
    return (PASSED);
}

/*****************************************************************************
 *
 * Function   : viper_get_info
 * Description: Get VIPER system Info by reading related linux file.
 * Inputs     : info_file - related Linux file to get info
 *              info_item - the item to collect
 *              info_item_size - size of items that needs to collect
 *              file_name - file to put the collected info
 * Outputs    : PASSED/FAILED
 *
 *****************************************************************************/
static int viper_get_info(char *info_file, char **info_item,
                         int info_item_size, char *file_name)
{
    int index = 0;
    char sys_cmd[256];

    /*
     * Read out the needed Info from related Linux info_file 
     */
    for (index = 0; index < info_item_size; index++) {
        sprintf(sys_cmd, "cat %s | grep -m 1 '%s' >> %s",
                info_file, info_item[index], file_name);
        system(sys_cmd);
    }
    return (PASSED);
}

/*****************************************************************************
 *
 * Function   : viper_get_cpucore
 * Description: To show CPU core number by reading file "/proc/cpuinfo".
 * Inputs     : *file_name - cpuinfo file name
 * Outputs    : PASSED/FAILED
 *
 *****************************************************************************/
static int viper_get_cpucore( char *file_name )
{
    char sys_cmd[256];
    char sys_cpucore[3];

    if ((ExecuteCmdbyPopen ("cat /proc/cpuinfo | grep processor | wc -l", 
        sys_cpucore, VIPER_INFO_BUF_SIZE)) == 0) {
        cterr('f',0,"get CPU core failed!!\n");
        return (FAILED);
    }

    sprintf(sys_cmd, "echo \"CPU cores\t: %d\" >> %s", atoi(sys_cpucore), file_name);
    system(sys_cmd);

    return (PASSED);
}

/*****************************************************************************
 *
 * Function   : viper_show_meminfo
 * Description: To show Viper memory Info
 * Inputs     : None
 * Outputs    : None
 *
 *****************************************************************************/
void viper_show_meminfo (void)
{
    system(VIPER_SHOW_MEMORY_SIZE);

}

/*****************************************************************************
 *
 * Function   : viper_show_cpuinfo
 * Description: To show Viper CPU Info by reading file "/proc/cpuinfo".
 * Inputs     : None
 * Outputs    : PASSED/FAILED
 *
 *****************************************************************************/
int viper_show_cpuinfo (void)
{
    int rc = FAILED;
    char cpu_info[VIPER_INFO_BUF_SIZE];
    FILE *fp;

    fp = fopen(VIPER_CPU_INFO_FILE, "r");
    if (fp == NULL) {
        rc = viper_get_info("/proc/cpuinfo", viper_cpu_info,
                           size_of_viper_cpu_info,
                           VIPER_CPU_INFO_FILE);
        if (rc != PASSED) {
            printf("Failed to get CPU information. \n");
            return (rc);
        } else {
            fp = fopen(VIPER_CPU_INFO_FILE, "r");
            if (fp == NULL) {
                printf("Failed to open %s. \n", VIPER_CPU_INFO_FILE);
                return (FAILED);
            }
        }
    }
    viper_get_cpucore(VIPER_CPU_INFO_FILE);
	
    printf("CPU info after inits:\n");
    while (fgets(cpu_info, VIPER_INFO_BUF_SIZE, fp) != NULL) {
        printf("%s", cpu_info);
    }
    printf("\n");

    fclose(fp);	
    unlink(VIPER_CPU_INFO_FILE);

    /* Show currently CPU on Die temperature */	
    if (viper_cpu_ondie_temp(0) != PASSED) {
        return (FAILED);
    }
    return (PASSED);
}

/*****************************************************************************
 *
 * Function   : platform_init
 * Description: Wrap function to run platform initialization
 * Inputs     : None
 * Outputs    : None
 *
 *****************************************************************************/
static void platform_init (void)
{
    /*
     * Open modules
     */
    if (viper_open_module(&fd_i2c0, VIPER_I2C_0) != PASSED) {
        close(fd_i2c0);
        cterr('f', 0, "%s: Failed to open /dev/%s", __FUNCTION__,
                VIPER_I2C_0);
    }
    /* Open the I2C device for ismt driver*/
    if (viper_open_module(&fd_i2c1, VIPER_I2C_1) != PASSED) {
        close(fd_i2c1);
        cterr('f', 0, "%s: Failed to open /dev/%s", __FUNCTION__,
                VIPER_I2C_0);
    }

    /* Open the fpga mmap driver*/
    if (open_fpga() != PASSED) {
        close(fd_fpga);
        cterr('f', 0, "%s: Failed to open %s", __FUNCTION__,
                VIPER_FPGA_DEV);
    }
    /* Relese ACT2 from reset */
    if (fpga_reset_api(FPGA_EXTER_DEV_RST_REG, ACT2_RESET, FALSE,
                       WAITTIME_20_MS) != PASSED) {
        cterr('f', 0, "%s: Failed to release ACT2 from Reset.\n", __FUNCTION__);
    }
    /* Relese FPGA I2C controller from reset */
    if (fpga_reset_api(FPGA_INT_DEV_RST_REG, INT_I2C_RESET, FALSE,
                      WAITTIME_20_MS)
                      == FAILED) {
        cterr ('f', 0, " Un-reset FPGA I2C reset");
    }

    /* Init P2SB memory */
    /* Set up PCIE 1f.1 for access P2SB memory space*/
    if (dnv_set_pcie_1f_1() != PASSED) {
        cterr ('f', 0, "Failed to init P2SB memory space.");
    }

    /* Set up WAN env */
    if (diag_gephy_init() != PASSED) {
        cterr ('f', 0, "Failed to init GE PHY.");
    }
        
    /* Set up DSL Ethernet ENV */
    if (has_dsl_sku() == TRUE) {
        viper_dsl_env_setup();
    }
}


/*****************************************************************************
 *
 * Function   : main
 * Description: entry point of viper
 * Inputs     : argc, number of argument
 *              argv, command line arguments
 *              
 * Outputs    : exit status
 *
 *****************************************************************************/
int main (int argc, char *argv[])
{
    if (argc > 1) {
        if (strstr(argv[1], "rom-ugd")) {
            printf("No ready\n");
            return (FAILED);
        } else {
            /* goto menu directly; dont' call monitor(); */
            diag_menu(1, argv);
        }
        return (PASSED);
    }
    /*
     * Turn off memory malloc overcommit to avoid oom in memory test
     */
    system(TURN_OFF_OVERCOMMIT_MEM);
    system(TURN_OFF_OVERCOMMIT_RATIO);

    /* Plaform init */
    platform_init();

    viper_show_cpuinfo();
    viper_show_meminfo();
    printf("Thermal Sensor(NXP LM75BD) ");
    diag_temp_sensor_show_temp();
    viper_show_fpga_ver(0);

    /* Show SKU Info */
    display_viper_sku_info();

    printf("%s", banner_string);

    diag_menu(1, argv);         /* goto menu directly; dont' call monitor(); */

    return (PASSED);
}

/*-------------------------------------------------
 * $Log: linux_main.c,v $
 * Revision 1.3  2018/08/31 03:59:30  chieyang
 * Add SPI flash utility, show memory size and xdsl test modification. Merge from viper-branch2
 *
 * Revision 1.2  2018/08/06 02:31:52  harrchan
 * Merge viper E2E to the main trunk (CSCvk28469)
 *
 * Revision 1.1.2.10  2018/07/06 02:54:08  harrchan
 * Add enhance error message
 *
 * Revision 1.1.2.9  2018/07/03 05:38:55  harrchan
 * Follow the coding rule to clean up code
 *
 * Revision 1.1.2.8  2018/06/27 07:27:28  lucywang
 * Removed string ViperJ
 *
 * Revision 1.1.2.7  2018/05/09 07:11:26  olin2
 * 1. Move GE and DSL init to the beginning. 2. Add has GE1. 3. Show cookie info
 *
 * Revision 1.1.2.6  2018/05/03 08:48:40  lucywang
 * Added Temperature to System Information
 *
 * Revision 1.1.2.5  2018/04/13 11:19:12  lucywang
 * Modified to use Cisco FPGA : 1) Upgrade 2) LED 3) FPGA register 4) FPGA I2C reset
 *
 * Revision 1.1.2.4  2018/04/13 03:29:07  harrchan
 * Set FPGA register to out of reset component
 *
 * Revision 1.1.2.3  2018/04/09 02:34:50  lucywang
 * Added System Intermation
 *
 * Revision 1.1.2.2  2018/03/15 08:26:16  harrchan
 * Change I/O access to memory map
 *
 * Revision 1.1.2.1  2018/02/27 08:06:50  harrchan
 * Initial viper application code base
 *
 *
 * $Endlog$
 *-------------------------------------------------
 */
