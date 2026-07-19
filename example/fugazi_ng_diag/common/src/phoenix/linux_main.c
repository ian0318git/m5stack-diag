/* $Id: linux_main.c,v 1.2 2021/04/15 00:52:26 achiu2 Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/phoenix/linux_main.c,v $
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
#include "platform_cpu.h"
#include "linux_main.h"
#include "diag_fpga_lib.h"
#include "dnv_gpio_lib.h"
#include "diag_fpga.h"
#include "diag_temp_snsr_test.h"
#include "diag_cpld_lib.h"
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
 * Function   : phoenix_open_module
 * Description: Wrap function to open module
 * Inputs     : The file descriptor pointer of module and module_type
 * Outputs    : PASSED/FAILED
 *
 *****************************************************************************/
uint phoenix_open_module (int *fd, const char *name)
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
 * Function    : phoenix_cpu_ondie_temp
 * Description : Function to get CPU on die temperature in degree C.
 * Inputs      : opt - reserve for future use
 * Outputs     : PASSED/FAILED
 *
 ******************************************************************************* 
 */
int phoenix_cpu_ondie_temp (int opt)
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
 * Function: phoenix_tftp_dhcpd_env_setup
 *
 * Description: This function set up ethernet, DHCP and tftp server
 *
 * Input : None
 *
 * Output: None
 *
 *******************************************************************************
 */
void phoenix_tftp_dhcpd_env_setup (void)
{   
    /* Set Ethernet up */
    system(PHOENIX_ETH_NIM0_SLOT_UP);
    system(PHOENIX_ETH_NIM1_SLOT_UP);
    system(PHOENIX_ETH_DSP0_SLOT_UP);
    msleep(WAIT_BK_LINK_UP);

    printf("Set up DHCP and TFTP for download firmware\n");
    system(PHOENIX_KILL_DHCPD);
    system(PHOENIX_KILL_OPENTFTP);

    /* Need to wait kill process if dhcpd and opentftpd exist. */
    msleep(WAIT_BK_LINK_UP);

    system(PHOENIX_DHCPD);
    system(PHOENIX_OPENTFTP);
}


/*****************************************************************************
 *
 * Function   : phoenix_get_info
 * Description: Get system Info by reading related linux file.
 * Inputs     : info_file - related Linux file to get info
 *              info_item - the item to collect
 *              info_item_size - size of items that needs to collect
 *              file_name - file to put the collected info
 * Outputs    : PASSED/FAILED
 *
 *****************************************************************************/
static int phoenix_get_info(char *info_file, char **info_item,
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
 * Function   : phoenix_get_cpucore
 * Description: To show CPU core number by reading file "/proc/cpuinfo".
 * Inputs     : *file_name - cpuinfo file name
 * Outputs    : PASSED/FAILED
 *
 *****************************************************************************/
static int phoenix_get_cpucore( char *file_name )
{
    char sys_cmd[256];
    char sys_cpucore[3];

    if ((ExecuteCmdbyPopen ("cat /proc/cpuinfo | grep processor | wc -l", 
        sys_cpucore, PHOENIX_INFO_BUF_SIZE)) == 0) {
        cterr('f',0,"get CPU core failed!!\n");
        return (FAILED);
    }

    sprintf(sys_cmd, "echo \"CPU cores\t: %d\" >> %s", atoi(sys_cpucore), file_name);
    system(sys_cmd);

    return (PASSED);
}

/*****************************************************************************
 *
 * Function   : phoenix_show_meminfo
 * Description: To show memory Info
 * Inputs     : None
 * Outputs    : None
 *
 *****************************************************************************/
void phoenix_show_meminfo (void)
{
    system(PHOENIX_SHOW_MEMORY_SIZE);
    printf("\n");
}

/*****************************************************************************
 *
 * Function   : phoenix_show_cpuinfo
 * Description: To show CPU Info by reading file "/proc/cpuinfo".
 * Inputs     : None
 * Outputs    : PASSED/FAILED
 *
 *****************************************************************************/
int phoenix_show_cpuinfo (void)
{
    int rc = FAILED;
    char cpu_info[PHOENIX_INFO_BUF_SIZE];
    FILE *fp;

    fp = fopen(PHOENIX_CPU_INFO_FILE, "r");
    if (fp == NULL) {
        rc = phoenix_get_info("/proc/cpuinfo", phoenix_cpu_info,
                           size_of_phoenix_cpu_info,
                           PHOENIX_CPU_INFO_FILE);
        if (rc != PASSED) {
            printf("Failed to get CPU information. \n");
            return (rc);
        } else {
            fp = fopen(PHOENIX_CPU_INFO_FILE, "r");
            if (fp == NULL) {
                printf("Failed to open %s. \n", PHOENIX_CPU_INFO_FILE);
                return (FAILED);
            }
        }
    }
    phoenix_get_cpucore(PHOENIX_CPU_INFO_FILE);
	
    printf("CPU info after inits:\n");
    while (fgets(cpu_info, PHOENIX_INFO_BUF_SIZE, fp) != NULL) {
        printf("%s", cpu_info);
    }
    printf("\n");

    fclose(fp);	
    unlink(PHOENIX_CPU_INFO_FILE);

    /* Show currently CPU on Die temperature */	
    if (phoenix_cpu_ondie_temp(0) != PASSED) {
        return (FAILED);
    }
    return (PASSED);
}

/*****************************************************************************
 *
 * Function   : phoenix_show_m2_info
 * Description: To show M.2 info
 * Inputs     : None
 * Outputs    : None
 *
 *****************************************************************************/
void phoenix_show_m2_info (void)
{
    printf("M.2 device: ");
    check_m2_device_utility();
    printf("\n");
}

/*****************************************************************************
 *
 * Function   : phoenix_show_temp_info
 * Description: To show temperature info from all thermal sensors.
 * Inputs     : None
 * Outputs    : None
 *
 *****************************************************************************/
void phoenix_show_temp_info(void)
{
    printf("Thermal Sensor:\n");
    show_temperature_all();
    printf("\n");
}

/*****************************************************************************
 *
 * Function   : phoenix_enable_nim_pim_pcie
 * Description: Setting Tabei-L GPIO configuration to enable NIM/PIM PCIe.
 *              GPIO 8 -> NIM, GPIO 9 -> PIM.
 * Inputs     : None
 * Outputs    : PASSED / FAILED
 *
 ************************************************************************/
int phoenix_enable_nim_pim_pcie (void)
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
    if (phoenix_open_module(&fd_i2c0, PHOENIX_I2C_0) != PASSED) {
        close(fd_i2c0);
        cterr('f', 0, "%s: Failed to open /dev/%s", __FUNCTION__,
                PHOENIX_I2C_0);
    }

    /* Init P2SB memory */
    /* Set up PCIE 1f.1 for access P2SB memory space*/
    if (dnv_set_pcie_1f_1() != PASSED) {
        cterr ('f', 0, "Failed to init P2SB memory space.");
    }

    /* Initialization for access CPLD */
    if (cpld_enable_pcie_bar1() != PASSED) {
        cterr('f', 0, "unable to enable cpld\n");
    } 

    /* CPLD base address initialization */
    if (open_cpld() != PASSED) {
        cterr('f', 0, "unable to open***/dev/cpld\n");
    } 

    /* Set up TFTP/DHCPD ENV */
    phoenix_tftp_dhcpd_env_setup();

    /* Set up NIM/PIM GPIO switch configure */
    phoenix_enable_nim_pim_pcie();
}


/*****************************************************************************
 *
 * Function   : main
 * Description: entry point of Phoenix
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

    phoenix_fpga_base_addr_init();

    /* 
     * NIOS must be in disable mode when diag is running
     */
    set_nios_mode(NIOS_DISABLE_MODE);

    printf("\n");

    /* Plaform init */
    platform_init();

    /* Show system info */
    phoenix_show_cpuinfo();
    phoenix_show_meminfo();
    phoenix_show_m2_info();
    phoenix_show_temp_info();
    phoenix_show_fpga_ver(0);
    printf("\n");
    phoenix_show_sku_dbx_info();

    printf("%s", banner_string);

    diag_menu(1, argv);         /* goto menu directly; dont' call monitor(); */

    /* reverty to normal before leave diag */
    set_nios_mode(NIOS_NORMAL_MODE);

    return (PASSED);
}

