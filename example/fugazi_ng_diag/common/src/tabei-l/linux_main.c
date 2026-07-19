 /* $Id: linux_main.c,v 1.2 2019/10/17 02:16:24 kehuang2 Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/tabei-l/linux_main.c,v $
 *------------------------------------------------------------------
 *
 * linux_main.c
 *
 * This file is diagnostic main entry
 *
 * Copyright (c) 2018~2019 by Cisco Systems, Inc.
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
#include "linux_main.h"
#include "diag_fpga_lib.h"
#include "dnv_gpio_lib.h"
#include "diag_fpga.h"
#include "diag_gephy_lib.h"
#include "diag_temp_snsr_test.h"
#include "diag_cpld_lib.h"
#include "plug_host_fpga_lib.h"
#include "plug_slot.h"
#include "diag_hdd_test.h"
#include "diag_m2_test.h"
#include "dnv_eth_lib.h"
#include "proto.h" /* msleep */

/*
 * Declare local function
 */

static int fd_i2c0 = -1;
static int fd_i2c1 = -1;
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
 * Function   : tabei_open_module
 * Description: Wrap function to open module
 * Inputs     : The file descriptor pointer of module and module_type
 * Outputs    : PASSED/FAILED
 *
 *****************************************************************************/
uint tabei_open_module (int *fd, const char *name)
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

/*******************************************************************************
 *
 * Function    : tabei_cpu_ondie_temp
 * Description : Function to get CPU on die temperature in degree C.
 * Inputs      : opt - reserve for future use
 * Outputs     : PASSED/FAILED
 *
 ******************************************************************************* 
 */
int tabei_cpu_ondie_temp (int opt)
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

/*******************************************************************************
 *
 * Function: tabei_tftp_dhcpd_env_setup
 *
 * Description: This function set up ethernet, DHCP and tftp server
 *
 * Input : None
 *
 * Output: None
 *
 *******************************************************************************
 */
void tabei_tftp_dhcpd_env_setup (void)
{   

    /* Set up ethernet up */
    system(TABEI_ETH_NIM_SLOT_UP);
    system(TABEI_ETH_NIM_SLOT_IP);
    msleep(WAIT_BK_LINK_UP);
    printf("Set up DHCP and TFTP for download DSL firmware\n");
    system(TABEI_KILL_DHCPD);
    system(TABEI_KILL_OPENTFTP);
    system(TABEI_DHCPD);
    system(TABEI_OPENTFTP);
}


/*****************************************************************************
 *
 * Function   : tabei_get_info
 * Description: Get system Info by reading related linux file.
 * Inputs     : info_file - related Linux file to get info
 *              info_item - the item to collect
 *              info_item_size - size of items that needs to collect
 *              file_name - file to put the collected info
 * Outputs    : PASSED/FAILED
 *
 *****************************************************************************/
static int tabei_get_info(char *info_file, char **info_item,
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
 * Function   : tabei_get_cpucore
 * Description: To show CPU core number by reading file "/proc/cpuinfo".
 * Inputs     : *file_name - cpuinfo file name
 * Outputs    : PASSED/FAILED
 *
 *****************************************************************************/
static int tabei_get_cpucore( char *file_name )
{
    char sys_cmd[256];
    char sys_cpucore[3];

    if ((ExecuteCmdbyPopen ("cat /proc/cpuinfo | grep processor | wc -l", 
        sys_cpucore, TABEI_INFO_BUF_SIZE)) == 0) {
        cterr('f',0,"get CPU core failed!!\n");
        return (FAILED);
    }

    sprintf(sys_cmd, "echo \"CPU cores\t: %d\" >> %s", atoi(sys_cpucore), file_name);
    system(sys_cmd);

    return (PASSED);
}

/*****************************************************************************
 *
 * Function   : tabei_show_meminfo
 * Description: To show memory Info
 * Inputs     : None
 * Outputs    : None
 *
 *****************************************************************************/
void tabei_show_meminfo (void)
{
    system(TABEI_SHOW_MEMORY_SIZE);
}

/*****************************************************************************
 *
 * Function   : tabei_show_cpuinfo
 * Description: To show CPU Info by reading file "/proc/cpuinfo".
 * Inputs     : None
 * Outputs    : PASSED/FAILED
 *
 *****************************************************************************/
int tabei_show_cpuinfo (void)
{
    int rc = FAILED;
    char cpu_info[TABEI_INFO_BUF_SIZE];
    FILE *fp;

    fp = fopen(TABEI_CPU_INFO_FILE, "r");
    if (fp == NULL) {
        rc = tabei_get_info("/proc/cpuinfo", tabei_cpu_info,
                           size_of_tabei_cpu_info,
                           TABEI_CPU_INFO_FILE);
        if (rc != PASSED) {
            printf("Failed to get CPU information. \n");
            return (rc);
        } else {
            fp = fopen(TABEI_CPU_INFO_FILE, "r");
            if (fp == NULL) {
                printf("Failed to open %s. \n", TABEI_CPU_INFO_FILE);
                return (FAILED);
            }
        }
    }
    tabei_get_cpucore(TABEI_CPU_INFO_FILE);
	
    printf("CPU info after inits:\n");
    while (fgets(cpu_info, TABEI_INFO_BUF_SIZE, fp) != NULL) {
        printf("%s", cpu_info);
    }
    printf("\n");

    fclose(fp);	
    unlink(TABEI_CPU_INFO_FILE);

    /* Show currently CPU on Die temperature */	
    if (tabei_cpu_ondie_temp(0) != PASSED) {
        return (FAILED);
    }
    return (PASSED);
}

/*****************************************************************************
 *
 * Function   : tabei_show_m2_info
 * Description: To show M.2 info
 * Inputs     : None
 * Outputs    : None
 *
 *****************************************************************************/
void tabei_show_m2_info (void)
{
    printf("M.2 device: ");
    check_m2_device_utility();
}

/*****************************************************************************
 *
 * Function   : tabei_enable_nim_pim_pcie
 * Description: Setting Tabei-L GPIO configuration to enable NIM/PIM PCIe.
 *              GPIO 8 -> NIM, GPIO 9 -> PIM.
 * Inputs     : None
 * Outputs    : PASSED / FAILED
 *
 ************************************************************************/
int tabei_enable_nim_pim_pcie (void)
{
    int gpio_pin;
    uint data = 0;

    /* GPIO 8 and 9 Need to set the configure to control NIM/PIM from CPU side */
    for (gpio_pin = DNV_GPIO_8; gpio_pin <= DNV_GPIO_9; gpio_pin++) {
        if (dnv_gpio_read(gpio_pin, &data) == FAILED) {
            printf("%s: Read GPIO (%d) Fails\n", __func__, gpio_pin);
            return (FAILED);
        }
        /* Need to configure PADCFG RXDIS 1 and TXDIS 0 to enable GPIO TX */
        data &= (uint)~PADCFG0_GPIOTXRXDIS_MASK;
        data |= (uint)PADCFG0_GPIOTXRXDIS_VAL;

        /* Need to configure PADCFG TXSTATE LEVAE to HIGH */
        data &= (uint)~PADCFG0_TXSTATE_LEVEL;
        data |= (uint)PADCFG0_TXSTATE_HIGH;

        if (dnv_gpio_write (gpio_pin, data) == FAILED) {
            printf("%s: Write GPIO (%d) Fails\n", __func__, gpio_pin);
            return (FAILED);
        }
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
void platform_init (void)
{
    /*
     * Open modules
     */
    if (tabei_open_module(&fd_i2c0, TABEI_I2C_0) != PASSED) {
        close(fd_i2c0);
        cterr('f', 0, "%s: Failed to open /dev/%s", __FUNCTION__,
                TABEI_I2C_0);
    }

    /* Open the I2C device for ismt driver*/
    if (tabei_open_module(&fd_i2c1, TABEI_I2C_1) != PASSED) {
        close(fd_i2c1);
        cterr('f', 0, "%s: Failed to open /dev/%s", __FUNCTION__,
                TABEI_I2C_0);
    }

    /* Init P2SB memory */
    /* Set up PCIE 1f.1 for access P2SB memory space*/
    if (dnv_set_pcie_1f_1() != PASSED) {
        cterr ('f', 0, "Failed to init P2SB memory space.");
    }
    if (is_tabeil() == TRUE) {
        /* Set up WAN env (88e1514) */
        if (diag_gephy_init() != PASSED) {
            cterr ('f', 0, "Failed to init GE PHY.");
        }
        /* Initialization for access CPLD */
        if (cpld_enable_pcie_bar1() != PASSED) {
            cterr('f', 0, "unable to enable cpld\n");
        } 

    }

    /* CPLD base address initialization */
    if (open_cpld() != PASSED) {
        cterr('f', 0, "unable to open***/dev/cpld\n");
    } 

    /* Set up TFTP/DHCPD ENV */
    tabei_tftp_dhcpd_env_setup();

    /* Set up NIM/PIM GPIO switch configure */
    tabei_enable_nim_pim_pcie();
}


/*****************************************************************************
 *
 * Function   : main
 * Description: entry point of Tabei
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

    init_slot_info();

    /* FPGA base address initialization */
    if (uio_open() != PASSED) {
        cterr('f', 0, "unable to open***/dev/uio***\n");
    }

    tabei_fpga_base_addr_init();

    /* 
     * NIOS must be in disable mode when diag is running
     */
    set_nios_mode(NIOS_DISABLE_MODE);

    init_plug_info();
    printf("\n");

    /* power off all pluggable module*/
    plug_module_power_off(PLUG_SLOT_1);

    /* Plaform init */
    platform_init();

    tabei_show_cpuinfo();
    tabei_show_meminfo();
    tabei_show_m2_info();
    fpga_poe_detect();

    printf("Thermal Sensor\n");
    show_temperature_all();

    printf("\n");
    tabei_show_fpga_ver(0);

    printf("%s", banner_string);

    diag_menu(1, argv);         /* goto menu directly; dont' call monitor(); */

    /* reverty to normal before leave diag */
    set_nios_mode(NIOS_NORMAL_MODE);

    return (PASSED);
}

/*-------------------------------------------------
 * $Log: linux_main.c,v $
 * Revision 1.2  2019/10/17 02:16:24  kehuang2
 * Collapse Tabei-L into main trunk
 *
 * Revision 1.1.4.33  2019/10/01 03:00:49  kehuang2
 * Clean up code
 *
 * Revision 1.1.4.32  2019/09/05 09:29:05  kehuang2
 * Support Promethium CPLD init
 *
 * Revision 1.1.4.31  2019/08/21 03:31:36  kehuang2
 * Update the content of POE log
 *
 * Revision 1.1.4.30  2019/08/20 10:30:12  kehuang2
 * Support POE detect Utility
 *
 * Revision 1.1.4.29  2019/07/30 06:56:29  kehuang2
 * Clean up code
 *
 * Revision 1.1.4.28  2019/07/18 01:08:09  kehuang2
 * Update init sequence for Promethium
 *
 * Revision 1.1.4.27  2019/05/29 07:02:16  olin2
 * Clean up code
 *
 * Revision 1.1.4.26  2019/05/29 03:16:18  kehuang2
 *
 * 1.Merge image according to official board type.
 * 2.Reform the structure of diag menu
 *
 * Revision 1.1.4.25  2019/04/29 08:14:26  kehuang2
 * Clean up code
 *
 * Revision 1.1.4.24  2019/04/19 03:15:29  kehuang2
 * 1.Support CPLD access 2.Support new FPGA 3.Clean up code
 *
 * Revision 1.1.4.23  2019/04/11 07:25:09  kehuang2
 * Clean up code by the comment of code review
 *
 * Revision 1.1.4.22  2019/03/19 09:26:26  kehuang2
 * Merge Sku1 and Sku2 into same image
 *
 * Revision 1.1.4.21  2019/03/11 09:31:08  meho
 * Support PIM NVMe on reworked Tabei-L
 *
 * Revision 1.1.4.20  2019/02/26 08:10:43  olin2
 * Add opentftp and dhcpd app
 *
 * Revision 1.1.4.19  2019/02/22 06:43:03  olin2
 * Add Reva on Tabei-L
 *
 * Revision 1.1.4.18  2019/02/15 03:36:47  harrchan
 * Support 1543 Internal loopback test
 *
 * Revision 1.1.4.17  2019/02/11 11:24:55  harrchan
 * Support Init gephy
 *
 * Revision 1.1.4.16  2019/01/25 07:42:24  harrchan
 * Add SKU1 in Makefile for seperature sku in future
 *
 * Revision 1.1.4.15  2019/01/25 03:21:06  wilbhuan
 * 1. Added ESW(Ethernet Switch) test with 88E6390 PHY device.
 * 2. The scope of ESW test as following:
 *    (1) Register test
 *    (2) MAC loopback test
 *    (3) External loopback test
 *    (4) Interrupt test
 *
 * Revision 1.1.4.14  2018/12/24 08:40:44  olin2
 * Show thermal sensor info
 *
 * Revision 1.1.4.13  2018/12/21 07:09:47  olin2
 * Update M.2 device menu
 *
 * Revision 1.1.4.12  2018/12/07 01:33:59  olin2
 * Support Check M.2 device util
 *
 * Revision 1.1.4.11  2018/12/05 06:39:19  olin2
 * Update Fan control for NIOS
 *
 * Revision 1.1.4.10  2018/11/16 05:42:11  olin2
 * Clean up code
 *
 * Revision 1.1.4.9  2018/11/06 06:17:57  olin2
 * Temporary disable pluggable pcie root port
 *
 * Revision 1.1.4.8  2018/10/26 08:40:50  kodko
 * Add support for PIM LTE and test card modules.
 *
 * Revision 1.1.4.7  2018/10/24 02:47:27  harrchan
 * 88E1514 GEPHY test
 *
 * Revision 1.1.4.6  2018/10/19 01:44:19  harrchan
 * I2C scan test
 *
 * Revision 1.1.4.5  2018/10/15 12:30:12  kodko
 * Add CPLD register read/write function.
 *
 * Revision 1.1.4.4  2018/10/15 09:31:32  kodko
 * Porting FPGA UIO driver read/write function.
 *
 * Revision 1.1.4.3  2018/10/09 09:22:05  olin2
 * Initial commit for NIM test
 *
 * Revision 1.1.4.2  2018/10/02 01:50:02  harrchan
 * Initial commit for Tabei-L P1A bring up.
 *
 * $Endlog$
 *-------------------------------------------------
 */
