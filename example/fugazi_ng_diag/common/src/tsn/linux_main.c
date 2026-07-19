/* $Id: linux_main.c,v 1.14 2019/10/16 02:27:15 sherliu2 Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/tsn/linux_main.c,v $
 *------------------------------------------------------------------
 * by: steja 
 * March 08, 2016
 *
 * Copyright (c) 2009-2019 by Cisco Systems, Inc.
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
#include "plat_defs.h"
#include "platform_stub.h"
#include "platform_fpga.h"
#include "diag_ge_phy.h"
#include "platform_cpu.h"
#include "plug_slot.h"
#include "plug_host_fpga_lib.h"
#include "tsn_comm.h"
#include "cmd_rom_ugd.h"
#include "diag_usb_lib.h"

/*
 * Declare local function
 */
static int tsn_get_info(char *, char **, int, char *);
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
extern int tam_act2_reset(int);
extern int tsn_config_gephy_fiber(void);
extern int tsn_esw_init(void);
extern int tsn_cpu_ondie_temp(int);
extern boolean tsn_has_poe(int);
extern void init_eth2(void);
extern boolean tsn_has_2nd_ge(int);
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
#define TSN_I2C_0                   "i2c-0"
#define TSN_I2C_1                   "i2c-1"
#define TSN_I2C_2                   "i2c-2"

#define TSN_INFO_BUF_SIZE          256

#define TSN_CPU_INFO_FILE           "/tsn_cpuinfo.txt"

static char *tsn_cpu_info[] = {
    "Processor",
    "BogoMIPS",
    "Features",
    "CPU implementer",
    "CPU architecture",
    "CPU revision",
    "Hardware",
};

static const uint size_of_tsn_cpu_info =
    sizeof(tsn_cpu_info) / sizeof(uchar *);

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
 * Function   : tsn_get_info
 * Description: Get TSN system Info by reading related linux file.
 * Inputs     : info_file - related Linux file to get info
 *              info_item - the item to collect
 *              info_item_size - size of items that needs to collect
 *              file_name - file to put the collected info
 * Outputs    : PASSED/FAILED
 *
 *****************************************************************************/
static int tsn_get_info(char *info_file, char **info_item,
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
 * Function   : tsn_get_cpucore
 * Description: To show CPU core number by reading file "/proc/cpuinfo".
 * Inputs     : None
 * Outputs    : PASSED/FAILED
 *
 *****************************************************************************/
static int tsn_get_cpucore( char *file_name )
{
    char sys_cmd[256];
    char sys_cpucore[3];

    if ((ExecuteCmdbyPopen ("cat /proc/cpuinfo | grep processor | wc -l", 
        sys_cpucore, TSN_INFO_BUF_SIZE)) == 0) {
        cterr('f',0,"get CPU core failed!!\n");
        return (FAILED);
    }

    sprintf(sys_cmd, "echo \"CPU cores\t: %d\" >> %s", atoi(sys_cpucore), file_name);
    system(sys_cmd);

    return (PASSED);
}

/*****************************************************************************
 *
 * Function   : tsn_show_cpuinfo
 * Description: To show TSN CPU Info by reading file "/proc/cpuinfo".
 * Inputs     : None
 * Outputs    : PASSED/FAILED
 *
 *****************************************************************************/
int tsn_show_cpuinfo (void)
{
    int rc = FAILED;
    char cpu_info[TSN_INFO_BUF_SIZE];
    FILE *fp;
    uint reg_offset = 0, reg_val = 0;
    reg_offset = (uint)(CPU_AP_REG_BASE + CPU_SAR_REG);

    fp = fopen(TSN_CPU_INFO_FILE, "r");
    if (fp == NULL) {
        rc = tsn_get_info("/proc/cpuinfo", tsn_cpu_info,
                           size_of_tsn_cpu_info, TSN_CPU_INFO_FILE);
        if (rc != PASSED) {
            printf("Failed to get CPU information. \n");
            return (rc);
        } else {
            fp = fopen(TSN_CPU_INFO_FILE, "r");
            if (fp == NULL) {
                printf("Failed to open %s. \n", TSN_CPU_INFO_FILE);
                return (FAILED);
            }
        }
    }
    tsn_get_cpucore(TSN_CPU_INFO_FILE);
	
    printf("CPU info after inits:\n");
    while (fgets(cpu_info, TSN_INFO_BUF_SIZE, fp) != NULL) {
        printf("%s", cpu_info);
    }
    printf("\n");

    fclose(fp);	
    unlink(TSN_CPU_INFO_FILE);

    /* Show currently CPU on Die temperature */	
    if (tsn_cpu_ondie_temp(0) != PASSED) {
        return (FAILED);
    }

    /* Read CPU clock frequencies for SAR register */
    if (tsn_mem_read32(reg_offset, &reg_val) != PASSED) {
        printf("%s: Failed to read CPU register 0x%08X.\n",
               __FUNCTION__, reg_offset);
        return (FAILED);
    }

    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("%s:%d reg_val = 0x%08X.\n", __FUNCTION__, __LINE__, reg_val);
    }

    /* get clock frequencies from register value */
    reg_val = (uint)(reg_val & CPU_SAR_RST2_FREQ_MASK);
    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("%s:%d reg_val after mask = 0x%08X.\n",
               __FUNCTION__, __LINE__, reg_val);
    }
/*
 *   CPU freg value
 *   CPU_1200_DDR_800_RCLK_800 = 0x19,
 *   CPU_1400_DDR_800_RCLK_800 = 0x1a,
 *   CPU_600_DDR_800_RCLK_800 = 0x1b,
 *   CPU_800_DDR_800_RCLK_800 = 0x1c,
 *   CPU_1000_DDR_800_RCLK_800 = 0x1d,
 */
    if ((this_is_star_c1109_4p()) || (this_is_star_c1109_2p()) || (this_is_supernova_c959_2p())) {
        if (reg_val != CPU_600_DDR_800_RCLK_800) {
            printf("\n *** WARNING CPU freg incorrect, reg_val = 0x%08X,\
                    C1109 expect CPU_600_DDR_800_RCLK_800 = 0x1B.\n", reg_val);
        }
    }
    if ((this_is_star_c1101p()) || (this_is_star_c1101e2e()) || (this_is_supernova_c951_4p())) {
        if (reg_val != CPU_800_DDR_800_RCLK_800) {
            printf("\n *** WARNING CPU freg incorrect, reg_val = 0x%08X,\
                    C1101 expect CPU_800_DDR_800_RCLK_800 = 0x1C.\n", reg_val);
        }
    }
    return (PASSED);
}

/*****************************************************************************
 *
 * Function   : tsn_open_module
 * Description: Wrap function to open module
 * Inputs     : The file descriptor pointer of module and module_type
 * Outputs    : PASSED/FAILED
 *
 *****************************************************************************/
static uint tsn_open_module (int *fd, const char *name)
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
 * Function   : tsn_usb_init
 * Description: Initialize USB register
 * Inputs     : None
 * Outputs    : PASSED/FAILED
 *
 *****************************************************************************/
static uint tsn_usb_init (void)
{
    uint reg_offset, reg_val = 0;

    /* For USB3.0 compliance */
#define USB_COMPHY_NUM        2
#define STAR_G2_TX_SSC_AMP    0x23
	reg_offset = M7040_GENRATION_2_SET_REG(USB_COMPHY_NUM);
    if (tsn_mem_read32(reg_offset, &reg_val) != PASSED) {
        printf("Failed to read CPU register 0x%08X.\n", reg_offset);
        return (FAILED);
    }
    
    reg_val &= (~G2_TX_SSC_AMP_MASK);
    reg_val |= (STAR_G2_TX_SSC_AMP << G2_TX_SSC_AMP_OFFSET);
    
    if (tsn_mem_write32(reg_offset, reg_val) != PASSED) {
        printf("Failed to write CPU register 0x%08X.\n", reg_offset);
        return (FAILED);
    }
    
    /* For Low Speed SI issue, C1109-2P only */
#define USB_PORT_NUM          0
#define STAR_USB_DRV_EN_LS    0xF
    if (this_is_star_c1109_2p() || this_is_supernova_c959_2p()) { 
        reg_offset = M7040_USB_PHY2_TX_CTRL_REG(USB_PORT_NUM);
        if (tsn_mem_read32(reg_offset, &reg_val) != PASSED) {
            printf("Failed to read CPU register 0x%08X.\n", reg_offset);
            return (FAILED);
        }

        reg_val &= (~USB_PHY2_TX_CTRL_DRV_EN_LS_MASK);
        reg_val |= (STAR_USB_DRV_EN_LS << USB_PHY2_TX_CTRL_DRV_EN_LS_OFFSET);

        if (tsn_mem_write32(reg_offset, reg_val) != PASSED) {
            printf("Failed to write CPU register 0x%08X.\n", reg_offset);
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
static void platform_init (void)
{
    int rsv = 0;

    /* Confirm device bus config. and get window base info  */
    if (tsn_devbus_init(rsv) != PASSED) {
        cterr('f', 0, "Failed to set up Device Bus.");
    }

    /*
     * Open modules
     */
    if (tsn_open_module(&fd_i2c0, TSN_I2C_0) != PASSED) {
        close(fd_i2c0);
        cterr('f', 0, "%s: Failed to open /dev/%s", __FUNCTION__,
                TSN_I2C_0);
    }
    if (tsn_open_module(&fd_i2c1, TSN_I2C_1) != PASSED) {
        close(fd_i2c1);
        cterr('f', 0, "%s: Failed to open /dev/%s", __FUNCTION__,
                TSN_I2C_1);
    }
    if (tsn_open_module(&fd_i2c2, TSN_I2C_2) != PASSED) {
        close(fd_i2c2);
        cterr('f', 0, "%s: Failed to open /dev/%s", __FUNCTION__,
                TSN_I2C_2);
    }
    /* FPGA Reset for temporarily work 20160422 */
    tam_act2_reset(0);
    /* Power up USB */
    if (fpga_reset_32_api(FPGA_CPUMUX_AND_USBPWR_REG, FPGA_USB_PWR, TRUE,
            WAITTIME_5_MS) != PASSED) {
        cterr('f', 0, "Failed to Power up USB.");
    }

    /* Init Switch */
    if (tsn_esw_init() != PASSED) {
        cterr('f', 0, "Failed to init Switch.");
    }

    /* Release PoE DC from reset if needed */
    if (tsn_has_poe(0) == TRUE) {
        if (fpga_reset_32_api(FPGA_EXTER_DEV_RST_REG, EXT_PRI_POE_DC_RESET,
                              FALSE, WAITTIME_5_MS) != PASSED) {
            cterr('f', 0, "Failed to POE power out of reset.");
        }
    }

    /* Config GE WAN PHY(s) */
    /* Set GE WAN PHY Transmitter Type to Class A based on HW request. */
    if (tsn_set_gephy_txtype(TSN_GE0_ETHNUM, GEWAN_TXTYPE_A) != PASSED) {
        cterr('f', 0, "Failed to set GE WAN PHY(eth%d) TX Type to Class A.",
                      TSN_GE1_ETHNUM);
    }

    if (tsn_has_2nd_ge(0) == TRUE) {
        if (tsn_set_gephy_txtype(TSN_GE1_ETHNUM, GEWAN_TXTYPE_A) != PASSED) {
            cterr('f', 0, "Failed to set GE WAN PHY(eth%d) TX Type to Class A.",
                          TSN_GE0_ETHNUM);
        }
    }

    /* Config GE PHY fiber */
    if (tsn_config_gephy_fiber() != PASSED) {
        cterr('f', 0, "Failed to config GE PHY Fiber.");
    }
    
    /* Detect xDSL sku set the interface to force link up */
    init_eth2();
    
    /* Initialize USB register */
    if (tsn_usb_init() != PASSED) {
        cterr('f', 0, "Failed to init USB.");
    }
}

/*******************************************************************************
 *
 * Function   : tsn_display_temp
 * Description: Function to display temperature on TSN 
 *              by reading sensor chip MAX31730.
 * Inputs     : None
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
int tsn_display_temp (void)
{
    int result = FAILED;

    /*
     * Get temperature,
     * for TSN, to read it from MAX31730.
     */
    printf("Current local(U2402, near SFP) temperature is ");
    result = max31730_read_local_temp();

    if (result != PASSED) {
        printf("N/A.\n");
    }

    printf("Current remote 1(Q2402, near state LEDs) temperature is ");
    result = max31730_read_remote_temp1();

    if (result != PASSED) {
        printf("N/A.\n");
    }

    printf("Current remote 2(Q2400, near DDR) temperature is ");
    result = max31730_read_remote_temp2();

    if (result != PASSED) {
        printf("N/A.\n");
    }

    printf("Current remote 3(Q2401, near Reset button) temperature is ");
    result = max31730_read_remote_temp3();

    if (result != PASSED) {
        printf("N/A.\n");
    }

    return (result);
}

/*****************************************************************************
 *
 * Function   : main
 * Description: entry point of tsn
 * Inputs     : argc, number of argument
 *              argv, command line arguments
 *              
 * Outputs    : exit status
 *
 *****************************************************************************/
int main (int argc, char *argv[])
{
    int usb_auto_suspend_val;
    int hub_reset_status;
    
    if (argc > 1) {
        if (strstr(argv[1], "rom-ugd")) {
            if (argc == ROM_UGD_ARGC_NUM) {
#ifdef DEVELOPER_VER
                rom_ugd_util(argv[2]);
#else
                /* For TSN official release version,
                 * this Diag upgrade ROMMON feature should ONLY support in
                 * TSN Oct-2017 pilot build.
                 * After that, TSN should use IOS to upgrade ROMMON.
                 */
                printf("\n\n\n^^^^^ This Image is ONLY to be used to upgrade"
                       " to ROMMON banner version %s for Oct-2017 TSN pilot"
                       " units ^^^^^\n", TSN_OCT17_PILOT_ROMMON_BANNER);
                printf("^^^^^ After this ROMMON version, SHOULD use ROMMON/IOS"
                       " to do the upgrade ^^^^^\n\n\n");

                rom_ugd_util(TSN_OCT17_PILOT_ROMMON_IMGNAME);
#endif /* DEVELOPER_VER */
            } else {
                rom_ugd_usage();
            }
        } else {
            /* goto menu directly; dont' call monitor(); */
            diag_menu(1, argv);
        }
        return (PASSED);
    }
    /*
     * Turn off memory malloc overcommit to avoid oom in memory test
     */
    system("echo 2 > /proc/sys/vm/overcommit_memory");
    system("echo 100 > /proc/sys/vm/overcommit_ratio");

    /* Plaform init */
    platform_init();

    if (this_is_star()) {
        /* Initialize pluggable information if this SKU is Star */
        init_plug_info();
        
        /* If this is Star C1109-4P, disable auto-suspend feature */
        if (this_is_star_c1109_4p()) { 
           
            /* Get current auto-suspend value */
           diag_usb_get_auto_suspend_val(&usb_auto_suspend_val);
           
           /* Check current auto-suspend value is -1 or other value */
           if (usb_auto_suspend_val != DIS_USB_AUTO_SUSPEND_FEATURE_VAL) {
               /* Get hub status: "reset" or "out of reset" */
               diag_usb_get_hub_reset_status(&hub_reset_status);
               /* If Current hub status is "reset", disable auto-suspend */
               if (hub_reset_status == USB_HUB_RESET_STAT) {
                   /* Disable auto-suspend feature, set value = -1 */
                   diag_usb_enable_auto_suspend_feature(DISABLE);
               } else {
                   printf("\nWarning!! Current hub device status is in [Out of "
                          "reset]. Refer to CSCvq98193\n"); 
               }
           }
        }
    }
    printf("\n");
    
    /*power off all pluggable module*/
    if (this_is_star()) {
        plug_module_power_off(PLUG_SLOT_1);        
        if(this_is_star_c1109_4p()) { 
            plug_module_power_off(PLUG_SLOT_2);        
        }
    }
    /* Check FPGA SKU info */
    check_fpga_sku_info();

    /* Get current Diag Kernel version */
    get_kernel_ver(); 

    printf("\n");

    diag_menu(1, argv);         /* goto menu directly; dont' call monitor(); */

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
$Log: linux_main.c,v $
Revision 1.14  2019/10/16 02:27:15  sherliu2
Fix CSCvq98193, disable auto-suspend feature for Star C1109-4P

Revision 1.13  2019/03/07 09:51:32  lucywang
[Supernova] PID changed : C1101L-4P --> C951-4P, C1109L-2P --> C959-2P

Revision 1.12  2019/01/18 05:54:46  yungchen
Merge Supernova branch to the main trunk (CSCvn79871)

Revision 1.11  2018/11/23 08:49:51  hondwang
Re-instruct pluggable common code with CDETs CSCvn17216

Revision 1.10.38.1  2018/10/15 06:53:07  hondwang
pluggable common code re-instruct modify code

Revision 1.10  2018/04/15 22:03:30  palin2
Merged Vulcan back to maintrunk.

Revision 1.9  2018/03/27 12:46:38  hondwang
Code modify for Star_C1101_4PLTEP_4PLTEPWX and Pluggable LTE EM7455, WP7601, WP7603 ER

Revision 1.8.2.1  2018/03/21 09:43:28  hondwang
remove display_sys_info check in diag boot up. Because plug LTE will take time for power on/off

Revision 1.8  2018/02/22 04:14:49  hondwang
Add diff CPU freq check for C1101 and C1109

Revision 1.7  2018/02/21 07:38:01  hondwang
Fix Star freq check function

Revision 1.6  2018/02/12 09:13:46  hondwang
merge Star CPU frequency check into main trunk

Revision 1.5  2018/02/09 09:56:54  hondwang
Merge Star branch star-branch-c9xx to main trunk

Revision 1.4  2018/01/23 11:38:18  steja
Merge tsn-gfast-branch4 code to maintrunk for support TSN-G.Fast (CSCvh40981)

Revision 1.3.10.2  2018/02/02 13:34:45  hondwang
Fix this_is_star_cxx function may return PASS with TSN platform

Revision 1.3.10.1  2018/01/20 06:27:23  hondwang
prepare merge star-branch-c9xx to main trunk

Revision 1.3.8.1  2018/01/15 08:59:50  steja
Updade code based on code review comment (CSCvh40981)

Revision 1.3  2017/10/19 14:04:28  palin2
Added support to upgrade ROMMON to TSN Oct-2017 Pilot version, 16.6(1r).

Revision 1.2.4.8  2018/02/12 08:41:45  hondwang
Fixed typo

Revision 1.2.4.6  2017/12/15 06:36:57  lucywang
Modified USB related registers for compliance test

Revision 1.2.4.5  2017/11/20 07:54:31  lucywang
Changed PID to C1101/C1109-2P/C1109-4P

Revision 1.2.4.4  2017/11/13 09:05:49  hondwang
Add pluggable slot cookie info with Diag login

Revision 1.2.4.3  2017/10/07 02:12:40  hondwang
Add FPGA SKU check function to double confirm FPGA info

Revision 1.2.4.2  2017/09/21 19:30:17  hondwang
Poweroff pluggable module before testing

Revision 1.2.4.1  2017/08/15 14:18:38  hondwang
star branch c9xx initial check in

Revision 1.2  2017/08/02 14:21:45  steja
Support TSN-H/M platform code

Revision 1.1.8.2  2017/07/29 03:41:03  steja
tsn-branch5 synced with Maintrunk repositories

Revision 1.1.6.2  2017/07/20 13:38:05  steja
tsn-branch4 merge with maintrunk

Revision 1.1.4.6.2.4  2017/07/18 14:23:37  steja
Code Cleanup

Revision 1.1.4.6.2.3  2017/07/17 13:54:44  palin2
Code cleanup.

Revision 1.1.4.6.2.2  2017/05/17 01:17:53  palin2
Updated GE WAN mapping number with team's decision.
(GE0: GE WAN with SFP; GE1: 2nd GE WAN)
CV: ----------------------------------------------------------------------

Revision 1.1.4.6.2.1.4.1  2017/06/13 09:35:56  tirawan
Add Pluggable Discovery function for Star C941, and add pluggable temperature sensor, GPIO Expander test functions

Revision 1.1.4.6.2.1  2017/04/13 13:41:17  palin2
Updated location description of thermal sensors.

Revision 1.1.4.6  2016/11/29 02:54:39  palin2
Dynamically getting device bus window base from CPU register.

Revision 1.1.4.5  2016/07/21 14:26:32  steja
1. Update Temperature sensor function
2. Add Wifi Temperature sensor to basic utilities
3. Add LTE Extended Feature Test
4. Display Chasis Temperature When Boot up
5. Update I2C scan function
6. Update POE cookie check card present

Revision 1.1.4.4  2016/07/17 11:11:25  palin2
Set GE WAN PHY Transmitter Type to Class A based on HW request.

Revision 1.1.4.3  2016/07/15 14:39:28  steja
Add code for DSL sku to force link up eth2 in diag.

Revision 1.1.4.2  2016/06/30 06:22:48  steja
tsn-branch2 sync with main trunk

Revision 1.1.2.12  2016/06/16 07:51:11  palin2
Updated PoE related utilities and code.

Revision 1.1.2.11  2016/06/03 01:00:45  palin2
Added function to show CPU on die temperature.

Revision 1.1.2.10  2016/05/26 03:09:22  palin2
Added TSN Switch init function, and SMI C45 read/write utility.

Revision 1.1.2.9  2016/05/18 09:03:02  steja
Add Platform Init

Revision 1.1.2.8  2016/05/06 16:10:18  steja
Bring up I2C-2 for RTC

Revision 1.1.2.7  2016/04/26 20:48:49  palin2
Updated code after bring up SFP external loopback test.

Revision 1.1.2.6  2016/04/22 11:34:00  steja
check-in for first release

Revision 1.1.2.5  2016/04/11 14:12:27  steja
Update code i2c utility for bringup

Revision 1.1.2.4  2016/03/27 14:17:34  steja
update based on code review comment 3/25/2016

Revision 1.1.2.3  2016/03/20 05:32:40  steja
1. Add i2c scan and i2c read write utility
2. Add sensor temperature and eeprom utility

Revision 1.1.2.2  2016/03/16 08:57:54  steja
add usb test
Revision 1.1.2.1  2016/03/08 09:55:10  steja
Initial Check-in

*/
