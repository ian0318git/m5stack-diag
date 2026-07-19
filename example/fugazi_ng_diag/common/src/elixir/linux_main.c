/* $Id: linux_main.c,v 1.2 2021/09/24 01:21:07 harrchan Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/elixir/linux_main.c,v $
 *------------------------------------------------------------------
 * 
 * linux_main.c
 *
 * Copyright (c) 2018 - 2019 by Cisco Systems, Inc.
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
#include <sys/utsname.h>
#include "error.h"
#include "common.h"
#include "types.h"
#include "nvmonvars.h"
#include "menu.h"
#include "setjmps.h"
#include "platform_cookie.h"
#include "diag_esw_lib.h"
#include "dev_mrvl_ge.h"
#include "diag_ge_phy_lib.h"
#include "diag_ge_phy_test.h"
#include "diag_poe_psu_lib.h"
#include "diag_moka_fpga_lib.h"
#include "diag_moka_fpga_util.h"
#include "linux_main.h"
#include "diag_cpu_lib.h"
#include "diag_cpu_util.h"
#include "diag_mother_board_test.h"
#include "plug_slot.h"
#include "diag_sirius_fpga_lib.h"
#include "plug_host_fpga_lib.h"
#include "diag_esw_test.h"
#include "proto.h"


/*
 * Declare local function
 */
static int plat_get_info(char *, char **, int, char *);
int get_kernel_ver(void); 
int quiet_launch = 0;

static int fd_i2c0 = -1;
static int fd_i2c1 = -1;
static int fd_i2c2 = -1;
/*
 * Externs function
 */
extern void diag_menu(int argc, char *argv[]);
extern int display_sys_info(int);
extern int ExecuteCmdbyPopen(char *cmd, char *retBuf, int sizeOfBuf);
extern int fpga_reset_32_api (uint, uint, uint, uint);
extern int max31730_read_local_temp(void);
extern int max31730_read_remote_temp1(void);
extern int max31730_read_remote_temp2(void);
extern int max31730_read_remote_temp3(void);
/*
 * Global variables
 */
volatile unsigned char hkeepflags;
jmp_buf monjmpbuf, *monjmpptr;
static uint32_t plat_ngio_bus_num = 0xffff;

/*
 * Global variables
 */
#define PLAT_I2C_0                   "i2c-0"
#define PLAT_I2C_1                   "i2c-1"
#define PLAT_I2C_2                   "i2c-2"

#define PLAT_INFO_BUF_SIZE          256

#define PLAT_CPU_INFO_FILE           "/plat_cpuinfo.txt"

static char *plat_cpu_info[] = {
    "Processor",
    "BogoMIPS",
    "Features",
    "CPU implementer",
    "CPU architecture",
    "CPU revision",
    "Hardware",
};

static const uint size_of_plat_cpu_info =
    sizeof(plat_cpu_info) / sizeof(uchar *);

#ifdef DEVELOPER_VER
#define ROM_UGD_ARGC_NUM 3
#else
#define ROM_UGD_ARGC_NUM 2
#endif /* DEVELOPER_VER */

uint diag_kernel_ver = (uint)LINUX_KERNEL_V4_4_8;

/*****************************************************************************
 *
 * Function   : get_i2c_fd
 * Description: return file descriptor for /dev/i2c1
 * Inputs     : i2c_bus
 * Outputs    : file desriptor for /dev/i2c1
 *
 *****************************************************************************/
int get_i2c_fd(int i2c_bus)
{
    if (i2c_bus == 2) {
        return fd_i2c2;
    } else if (i2c_bus == 1) {
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
 * Function   : plat_get_info
 * Description: Get system Info by reading related linux file.
 * Inputs     : info_file - related Linux file to get info
 *              info_item - the item to collect
 *              info_item_size - size of items that needs to collect
 *              file_name - file to put the collected info
 * Outputs    : PASSED/FAILED
 *
 *****************************************************************************/
static int plat_get_info(char *info_file, char **info_item,
                         int info_item_size, char *file_name)
{
    int index = 0;
    char sys_cmd[256];

    /*
     * Read out the needed Info from related Linux info_file 
     */
    for (index = 0; index < info_item_size; index++) {
        snprintf(sys_cmd, sizeof(sys_cmd), "cat %s | grep -m 1 '%s' >> %s",
                info_file, info_item[index], file_name);
        system(sys_cmd);
    }
    return (PASSED);
}

/*****************************************************************************
 *
 * Function   : plat_get_cpucore
 * Description: To show CPU core number by reading file "/proc/cpuinfo".
 * Inputs     : None
 * Outputs    : PASSED/FAILED
 *
 *****************************************************************************/
static int plat_get_cpucore( char *file_name )
{
    char sys_cmd[256];
    char sys_cpucore[3];

    if ((ExecuteCmdbyPopen ("cat /proc/cpuinfo | grep processor | wc -l", 
        sys_cpucore, PLAT_INFO_BUF_SIZE)) == 0) {
        printf("%s:%d:Failed to get the info CPU core number\n", __FUNCTION__, __LINE__);
        return (FAILED);
    }

    snprintf(sys_cmd, sizeof(sys_cmd), "echo \"CPU cores\t: %d\" >> %s", atoi(sys_cpucore), file_name);
    system(sys_cmd);

    return (PASSED);
}

/*****************************************************************************
 *
 * Function   : plat_show_cpuinfo
 * Description: To show CPU Info by reading file "/proc/cpuinfo".
 * Inputs     : None
 * Outputs    : PASSED/FAILED
 *
 *****************************************************************************/
int plat_show_cpuinfo (void)
{
    int rc = FAILED;
    char cpu_info[PLAT_INFO_BUF_SIZE];
    FILE *fp;

    fp = fopen(PLAT_CPU_INFO_FILE, "r");
    if (fp == NULL) {
        rc = plat_get_info("/proc/cpuinfo", plat_cpu_info,
                           size_of_plat_cpu_info, PLAT_CPU_INFO_FILE);
        if (rc != PASSED) {
            printf("Failed to get CPU information. \n");
            return (rc);
        } else {
            fp = fopen(PLAT_CPU_INFO_FILE, "r");
            if (fp == NULL) {
                printf("Failed to open %s. \n", PLAT_CPU_INFO_FILE);
                return (FAILED);
            }
        }
    }
    plat_get_cpucore(PLAT_CPU_INFO_FILE);
	
    printf("CPU info after inits:\n");
    while (fgets(cpu_info, PLAT_INFO_BUF_SIZE, fp) != NULL) {
        printf("%s", cpu_info);
    }
    printf("\n");

    fclose(fp);	
    unlink(PLAT_CPU_INFO_FILE);

    /* Show currently CPU on Die temperature */	
    if (diag_cpu_ondie_temp_util(0) != PASSED) {
        return (FAILED);
    }

    /* Show CPU and SDRAM frequency */
    if (show_cpu_ddr_freq() != PASSED)  {
        return (FAILED);
    }

    return (PASSED);
}

/*****************************************************************************
 *
 * Function   : plat_open_module
 * Description: Wrap function to open module
 * Inputs     : The file descriptor pointer of module and module_type
 * Outputs    : PASSED/FAILED
 *
 *****************************************************************************/
static uint plat_open_module (int *fd, const char *name)
{
    uint rc = FAILED;
    char device[80];

    snprintf(device, sizeof(device), "/dev/%s", name);
    *fd = open(device, O_RDWR);
    if (*fd <= 0) {
        return (rc);
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
    int rsv = 0;
    char dirname[128];
    int timeout = PCIE_DRV_POLLING_TIME;
    int ret = -1;


    /* Confirm device bus config. and get window base info  */
    if (plat_devbus_init(rsv) != PASSED) {
        printf("%s:%d:Failed to set up Device Bus\n", __FUNCTION__, __LINE__);
    }

    /*
     * Open modules
     */
    if (plat_open_module(&fd_i2c0, PLAT_I2C_0) != PASSED) {
        close(fd_i2c0);
        printf("%s:%d:Failed to open /dev/%s\n", __FUNCTION__, __LINE__, PLAT_I2C_0);
    }
    if (plat_open_module(&fd_i2c1, PLAT_I2C_1) != PASSED) {
        close(fd_i2c1);
        printf("%s:%d:Failed to open /dev/%s\n", __FUNCTION__, __LINE__, PLAT_I2C_1);
    }
    if (plat_open_module(&fd_i2c2, PLAT_I2C_2) != PASSED) {
        close(fd_i2c2);
        printf("%s:%d:Failed to open /dev/%s\n", __FUNCTION__, __LINE__, PLAT_I2C_2);
    }

    /* Power up USB */
    if (fpga_reset_32_api(FPGA_CPUMUX_AND_USBPWR_REG, FPGA_USB_PWR, TRUE,
            WAITTIME_5_MS) != PASSED) {
        printf("%s:%d:Failed to Power up USB\n", __FUNCTION__, __LINE__);
    }

    /* read PID from cookie and save it into a static parameter */
    initial_current_product_id();

    /* Release PoE DC from reset if needed */
    if (platform_has_poe(0) == TRUE) {
        if (fpga_reset_32_api(FPGA_EXTER_DEV_RST_REG, EXT_PRI_POE_DC_RESET,
                              FALSE, WAITTIME_5_MS) != PASSED) {
            printf("%s:%d:Failed to POE power out of reset\n", __FUNCTION__, __LINE__);
        }
    }

    /* Config GE WAN PHY(s) */
    /* Set GE0 WAN PHY Transmitter Type to Class A based on HW request. */
    if (diag_88e1112_ge_set_txtype(GE0, MRV88E111M_TX_TYPE_A) != PASSED) {
        printf("%s:%d:Failed to set GE%d TX Type as Class A\n", 
               __FUNCTION__, __LINE__, GE0);
    }

    /* Set GE1 WAN PHY Transmitter Type to Class A based on HW request. */
    if (diag_88e1112_ge_set_txtype(GE1, MRV88E111M_TX_TYPE_A) != PASSED) {
        printf("%s:%d:Failed to set GE%d TX Type as Class A\n", 
               __FUNCTION__, __LINE__, GE1);
    }
    
    /* Insert PCIE driver */
    system(ETH_INSMOD_AC5_NIM_DM_MODULE);

    /* Check if PCIE driver is ready */
    memset(dirname, '\0', sizeof(dirname));
    snprintf(dirname, sizeof(dirname), "%s", PCIE_DRV_PATH);
    while (timeout > 0) {
        ret = access(dirname, F_OK);
        if (ret != -1) {
            break;
        }    
        timeout--;
        msleep(ESW_WAIT_1000MS);
    }

    if (timeout == 0) {
        printf("%s:%d:PCIE driver start failed\n", __FUNCTION__, __LINE__);
    }

    /* Init Switch */
    if (diag_ac5_init() != PASSED) {
        printf("%s:%d:Failed to init Switch\n", __FUNCTION__, __LINE__);
    }

}

/*****************************************************************************
 *
 * Function   : main
 * Description: entry point of diag app
 * Inputs     : argc, number of argument
 *              argv, command line arguments
 *              
 * Outputs    : exit status
 *
 *****************************************************************************/
int main (int argc, char *argv[])
{
    if (argc > 1) {
        printf("Doesn't support any argumant\n");
        return (PASSED);
    }

    /* Print Diag app. banner */
    printf("%s", banner_string);

    /* Plaform init */
    platform_init();

    /* init pluggable SKU */
    if (platform_has_pluggable() == TRUE) {
        init_plug_info();
    } 

    /* Get current Diag Kernel version */
    get_kernel_ver(); 

    /* Show CPU and SDRAM frequency */
    show_cpu_ddr_freq();

    diag_menu(1, argv);         /* goto menu directly; dont' call monitor(); */

    /* Exit switch */
    diag_esw_exit();

    return (PASSED);
}

/*****************************************************************************
 *
 * Function   : get_kernel_ver
 * Description: Function to get current Kernel version.
 * Inputs     : None
 * Outputs    : PASSED / FAILED
 *
 *****************************************************************************/
int get_kernel_ver(void) 
{
    struct utsname uname_data;
    char *str_p = NULL;
    char *chk_str = LINUX_KER_V4_4_52_STRING;

    if (uname(&uname_data) == -1) {
        printf("%s: Failed to get Linux Kernel info.\n", __func__);
        return (FAILED);
    }
    
    if ((str_p = strstr(uname_data.release, chk_str)) != NULL) {
        /* Marvell SDK 17.10.3(Linux Kernel v4.4.52) */
        diag_kernel_ver = (uint)LINUX_KERNEL_V4_4_52;
    } else {
        /* Marvell SDK 16.05.1(Linux Kernel v4.4.8) */
        diag_kernel_ver = (uint)LINUX_KERNEL_V4_4_8;
    }
    return (PASSED);
}

/*-------------------------------------------------
 * $Log: linux_main.c,v $
 * Revision 1.2  2021/09/24 01:21:07  harrchan
 * Collapse Elixir-branch to Main Trunk.
 *
 * Revision 1.1.2.7  2021/04/23 02:46:11  illiu
 * 1. Replace sprintf with snprintf
 * 2. Clean up code
 *
 * Revision 1.1.2.6  2020/11/05 06:34:55  harrchan
 * 1.Base on P1A bring up result to Modify the AC5 MAC/internal/external loopback test
 * 2.Remove some debug message on AC5 init process
 *
 * Revision 1.1.2.5  2020/10/26 07:08:28  harrchan
 * 1.Changed PID table in platform_i2c.c
 * 2.Modify menu item to match up Elixir hardware design.
 *
 * Revision 1.1.2.4  2020/10/15 12:07:54  illiu
 * Move AC5 switch init and exit process to linux_main.c(It means do init once diag application is actived and do exit once diag application is exit)
 *
 * Revision 1.1.2.3  2020/10/06 02:17:23  illiu
 * Comment USB Initialize register function: plat_usb_init() (because not used for Elixir)
 *
 * Revision 1.1.2.2  2020/09/14 05:49:44  harrchan
 * Remove DSL and GSHDSL relevant part
 *
 * Revision 1.1.2.1  2020/09/09 09:08:08  illiu
 * First version which has been ported with Dreamliner and Marvel CPSS
 *
 * Revision 1.2  2019/01/10 06:36:24  wilbhuan
 * The beginning of Betelgeuse application code.
 *
 *-------------------------------------------------
 */
