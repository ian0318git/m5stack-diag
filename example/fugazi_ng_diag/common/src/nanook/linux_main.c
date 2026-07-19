 /* $Id: linux_main.c,v 1.3 2020/04/20 02:28:24 lucywang Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/nanook/linux_main.c,v $
 *------------------------------------------------------------------
 *
 * linux_main.c
 *
 * This file is diagnostic main entry
 *
 * Copyright (c) 2019 by Cisco Systems, Inc.
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
#include "dash_fpga.h"
#include "platform_stub.h"
#include "platform_cpu.h"
#include "linux_main.h"
#include "dnv_gpio_lib.h"
#include "diag_temp_snsr_test.h"
#include "diag_cpld_lib.h"
#include "diag_m2_test.h"
#include "common_utils.h"
#include "uio_utils.h"
#include "diag_gephy_1543_lib.h"
#include "diag_press_sensor_test.h"
#include "dnv_eth_lib.h"
#include "platform_intr_test.h"


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
unsigned long dash_fpga = 0;
unsigned long dash_cpld = 0;
unsigned int dash_fpga_fw_size = 0;
unsigned char *dash_fpga_fw_array = NULL;
char inface_lan0p0[32];
char inface_lan0p1[32];
char inface_lan1p0[32];
char inface_lan1p1[32];

extern int diag_gephy_1543_init(void);
extern int ps_init(void);
extern uint32 show_temperature_all(void);
extern int diag_esw_set_ixia_snake_config_util (uint);
extern int diag_gephy_1543_sfp_force_100 (void);
extern int get_pwr_seq_fw_rev (int option);
extern int dynamic_get_inface (int, char *);

extern boolean aikido_act2_flag;
extern boolean aikido_mailbox_flag;
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
 * Function   : get_platform_plane
 * Description: just return control plane
 * Inputs     : NONE
 *
 * Outputs    : CP
 *
 ******************************************************************************/
int
get_platform_plane (void)
{
    return CP;  /*defined in dash_fpga.h */
}

/*****************************************************************************
 *
 * Function   : nanook_open_module
 * Description: Wrap function to open module
 * Inputs     : The file descriptor pointer of module and module_type
 * Outputs    : PASSED/FAILED
 *
 *****************************************************************************/
uint nanook_open_module (int *fd, const char *name)
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

static void msleep(int n)
{
    usleep(n*1000);
}

int fpga_reset_api (uint r_offset, uint r_bit, uint r_opt, uint r_time_ms)
{
    uint reg_val = 0;

    /* Read FPGA interface reset register. */
    if (dash_fpga_reg_read(r_offset, &reg_val) != PASSED) {
        printf("%s: Failed to read FPGA reg.(0x%04X).\n",
               __FUNCTION__, r_offset);
        return (FAILED);
    }

    if (r_opt == TRUE) {
        /* Set the Reset bit. */
        reg_val |= r_bit;
    } else if (r_opt == FALSE) {
        /* Clear the reset bit. */
        reg_val &= (uint)(~r_bit);
    } else {
        printf("%s: Invalid Reset option(%#x).\n", __FUNCTION__, r_opt);
        return (FAILED);
    }

    /* Write the reset/un-reset into the corresponding register bit. */
    if (dash_fpga_reg_write(r_offset, reg_val) != PASSED) {
        printf("%s: Failed to write FPGA reg.(0x%04X).\n",
               __FUNCTION__, r_offset);
        return (FAILED);
    }

    /* Delay milliseconds after reset/un-reset */
    msleep(r_time_ms);

    /* Confirm the change to FPGA interface reset register. */
    reg_val = 0;
    if (dash_fpga_reg_read(r_offset, &reg_val) != PASSED) {
        printf("%s: Failed to read FPGA reg.(0x%04X).\n",
               __FUNCTION__, r_offset);
        return (FAILED);
    }

    if (((r_opt == TRUE) && ((reg_val & r_bit) != r_bit)) ||
        ((r_opt == FALSE) && ((reg_val & r_bit) != 0))) {
        printf("%s: Failed to %s reset bit in FPGA reg.(0x%04X).\n",
               __FUNCTION__, (r_opt == TRUE) ? "set" : "clear", r_offset);
        return (FAILED);
    }


     return (PASSED);
}

/*******************************************************************************
 *
 * Function    : nanook_cpu_ondie_temp
 * Description : Function to get CPU on die temperature in degree C.
 * Inputs      : opt - reserve for future use
 * Outputs     : PASSED/FAILED
 *
 ******************************************************************************* 
 */
int nanook_cpu_ondie_temp (int opt)
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
 * Function   : nanook_get_info
 * Description: Get system Info by reading related linux file.
 * Inputs     : info_file - related Linux file to get info
 *              info_item - the item to collect
 *              info_item_size - size of items that needs to collect
 *              file_name - file to put the collected info
 * Outputs    : PASSED/FAILED
 *
 *****************************************************************************/
static int nanook_get_info(char *info_file, char **info_item,
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
 * Function   : nanook_get_cpucore
 * Description: To show CPU core number by reading file "/proc/cpuinfo".
 * Inputs     : *file_name - cpuinfo file name
 * Outputs    : PASSED/FAILED
 *
 *****************************************************************************/
static int nanook_get_cpucore( char *file_name )
{
    char sys_cmd[256];
    char sys_cpucore[3];

    if ((ExecuteCmdbyPopen ("cat /proc/cpuinfo | grep processor | wc -l", 
        sys_cpucore, NANOOK_INFO_BUF_SIZE)) == 0) {
        cterr('f',0,"get CPU core failed!!\n");
        return (FAILED);
    }

    sprintf(sys_cmd, "echo \"CPU cores\t: %d\" >> %s", atoi(sys_cpucore), file_name);
    system(sys_cmd);

    return (PASSED);
}

/*****************************************************************************
 *
 * Function   : nanook_show_meminfo
 * Description: To show memory Info
 * Inputs     : None
 * Outputs    : None
 *
 *****************************************************************************/
void nanook_show_meminfo (void)
{
    system(NANOOK_SHOW_MEMORY_SIZE);

}

/*****************************************************************************
 *
 * Function   : nanook_show_cpuinfo
 * Description: To show CPU Info by reading file "/proc/cpuinfo".
 * Inputs     : None
 * Outputs    : PASSED/FAILED
 *
 *****************************************************************************/
int nanook_show_cpuinfo (void)
{
    int rc = FAILED;
    char cpu_info[NANOOK_INFO_BUF_SIZE];
    FILE *fp;

    fp = fopen(NANOOK_CPU_INFO_FILE, "r");
    if (fp == NULL) {
        rc = nanook_get_info("/proc/cpuinfo", nanook_cpu_info,
                           size_of_nanook_cpu_info,
                           NANOOK_CPU_INFO_FILE);
        if (rc != PASSED) {
            printf("Failed to get CPU information. \n");
            return (rc);
        } else {
            fp = fopen(NANOOK_CPU_INFO_FILE, "r");
            if (fp == NULL) {
                printf("Failed to open %s. \n", NANOOK_CPU_INFO_FILE);
                return (FAILED);
            }
        }
    }
    nanook_get_cpucore(NANOOK_CPU_INFO_FILE);
	
    printf("CPU info after inits:\n");
    while (fgets(cpu_info, NANOOK_INFO_BUF_SIZE, fp) != NULL) {
        printf("%s", cpu_info);
    }
    printf("\n");

    fclose(fp);	
    unlink(NANOOK_CPU_INFO_FILE);

    /* Show currently CPU on Die temperature */	
    if (nanook_cpu_ondie_temp(0) != PASSED) {
        return (FAILED);
    }

    /* Show thermal sensor */
    show_temperature_all();
    return (PASSED);
}

/*****************************************************************************
 *
 * Function   : nanook_show_m2_info
 * Description: To show M.2 info
 * Inputs     : None
 * Outputs    : None
 *
 *****************************************************************************/
void nanook_show_m2_info (void)
{
    printf("M.2 device: ");
    check_m2_device_utility();
    printf("\n");

}


/*******************************************************************************
 *
 * Function   : nanook_show_glory_fpga_ver
 * Description: Function to show Glory FPGA version.
 * Inputs     : opt - reserved for future use
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
int nanook_show_glory_fpga_ver (int opt)
{

    uint32_t reg_offset = 0, reg_val = 0;  

    reg_offset = FPGA_LPC_VERSION_REG;
        
    if (cpld_read_reg(reg_offset, &reg_val) != PASSED) {
        printf("Failed to read FPGA register 0x%04X.\n", reg_offset);
        return (FAILED);
    }
    printf("Glory FPGA version: %08X\n", reg_val);
    return (PASSED);
}

/*****************************************************************************
 *
 * Function   : nanook_set_gpio_config
 * Description: To set GPIO TX/RX config
 * Inputs     : None
 * Outputs    : None
 *
 *****************************************************************************/

void nanook_set_gpio_config (void)
{
    uint wr_data = 0, org_data = 0;
    dnv_gpio_read(DNV_GPIO_8, &org_data);
    wr_data = org_data & (~GPIO_RX_ENABLE);
    wr_data |= GPIO_TX_DISABLE;
    dnv_gpio_write(DNV_GPIO_8, wr_data);
}

void nanook_tftp_dhcpd_env_setup (void)
{
    char cmd[256];
    
    memset(cmd, 0, sizeof(cmd));
    /* Set up ethernet up */
    sprintf(cmd, "ifconfig %s up > /dev/null; ifconfig %s 192.123.123.1",
            inface_lan0p0, inface_lan0p0);
    system(cmd);
    msleep(WAIT_BK_LINK_UP);
    printf("Set up DHCP and TFTP for NIM card FW download\n");
    system(NANOOK_KILL_DHCPD);
    system(NANOOK_KILL_OPENTFTP);
    system(NANOOK_DHCPD);
    system(NANOOK_OPENTFTP);
}

/*****************************************************************************
 *
 * Function   : platform_init_for_test
 * Description: Wrap function to run platform initialization for Compliance
 * Inputs     : None
 * Outputs    : None
 *
 *****************************************************************************/
void platform_init_for_test (void)
{
    int rc = 0;
    /*
     * Open modules
     */
    if (nanook_open_module(&fd_i2c0, NANOOK_I2C_0) != PASSED) {
        close(fd_i2c0);
        cterr('f', 0, "%s: Failed to open /dev/%s", __FUNCTION__,
                NANOOK_I2C_0);
    }

    /* Open the I2C device for ismt driver*/
    if (nanook_open_module(&fd_i2c1, NANOOK_I2C_1) != PASSED) {
        close(fd_i2c1);
        cterr('f', 0, "%s: Failed to open /dev/%s", __FUNCTION__,
                NANOOK_I2C_1);
    }
    
    rc = uio_open();
    if (rc != PASSED ) {
        cterr('f', 0, "unable to open***/dev/uio***\n");
    }
    dash_fpga = (unsigned long)uio_get_regs();
    assert(dash_fpga);
    dash_set_map(0); /* call only after open uio drivder */
    platform_init_intr();


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
    int rc = 0;
    /*
     * Open modules
     */
    if (nanook_open_module(&fd_i2c0, NANOOK_I2C_0) != PASSED) {
        close(fd_i2c0);
        cterr('f', 0, "%s: Failed to open /dev/%s", __FUNCTION__,
                NANOOK_I2C_0);
    }

    /* Open the I2C device for ismt driver*/
    if (nanook_open_module(&fd_i2c1, NANOOK_I2C_1) != PASSED) {
        close(fd_i2c1);
        cterr('f', 0, "%s: Failed to open /dev/%s", __FUNCTION__,
                NANOOK_I2C_1);
    }
    
    rc = uio_open();
    if (rc != PASSED ) {
        cterr('f', 0, "unable to open***/dev/uio***\n");
    }
    dash_fpga = (unsigned long)uio_get_regs();
    assert(dash_fpga);
    dash_set_map(0); /* call only after open uio drivder */
    platform_init_intr();

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
    
    /* Initialize GPIO8 to GPI */
    nanook_set_gpio_config();

    /* Set up WAN env */

    if (diag_gephy_1543_init() != PASSED) {
        cterr ('f', 0, "Failed to init GE PHY.");
    }

    /* Set up TFTP/DHCPD ENV */
    nanook_tftp_dhcpd_env_setup();
}


/*****************************************************************************
 *
 * Function   : main
 * Description: entry point of Nanook
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
        } else if (strstr(argv[1], "ts")) {
            nanook_cpu_ondie_temp(0);
            platform_init_for_test();
            set_board_type();
            show_temperature_all();
        } else if (strstr(argv[1], "ac3_1000")) {
            printf("Please wait AC3 snake configuration - Speed 1000\n");
            platform_init_for_test();
            system(ETH_RM_IXGBE_MODULE);
            msleep(1000);
            system(ETH_INS_IXGBE_MODULE_AC3);
            msleep(1000);
            system("insmod /lib/modules/4.14.3/nim_dm_nanook_AC3_20190507.ko &> /dev/null");
            set_board_type();
            diag_esw_set_ixia_snake_config_util(0);
            printf("Snake Configuration Speed 1000 End\n");
        } else if (strstr(argv[1], "ac3_100")) {
            printf("Please wait AC3 snake configuration - Speed 100\n");
            platform_init_for_test();
            system(ETH_RM_IXGBE_MODULE);
            msleep(1000);
            system(ETH_INS_IXGBE_MODULE_AC3);
            msleep(1000);
            system("insmod /lib/modules/4.14.3/nim_dm_nanook_AC3_20190507.ko &> /dev/null");
            set_board_type();
            diag_esw_set_ixia_snake_config_util(1);
            printf("Snake Configuration Speed 100 End\n");
        } else if (strstr(argv[1], "ac3_10")) {
            printf("Please wait AC3 snake configuration -Speed 10\n");
            platform_init_for_test();
            system(ETH_RM_IXGBE_MODULE);
            msleep(1000);
            system(ETH_INS_IXGBE_MODULE_AC3);
            msleep(1000);
            system("insmod /lib/modules/4.14.3/nim_dm_nanook_AC3_20190507.ko &> /dev/null");
            set_board_type();
            diag_esw_set_ixia_snake_config_util(2);
            printf("Snake Configuration Speed 10 End\n");
        } else if (strstr(argv[1], "sfp_100")) {
            platform_init_for_test();
            if (diag_gephy_1543_init() != PASSED) {
                cterr ('f', 0, "Failed to init GE PHY.");
            }
            set_board_type();
            msleep(3000);
            diag_gephy_1543_sfp_force_100();
        } else {
            /* goto menu directly; dont' call monitor(); */
            diag_menu(1, argv);
        }
        return (PASSED);
    }
    
    /* Get all LAN Controller interface in initialization */
    dynamic_get_inface(1, inface_lan0p0);
    dynamic_get_inface(2, inface_lan0p1);
    dynamic_get_inface(3, inface_lan1p0);
    dynamic_get_inface(4, inface_lan1p1);
    
    /*
     * Turn off memory malloc overcommit to avoid oom in memory test
     */
    system(TURN_OFF_OVERCOMMIT_MEM);
    system(TURN_OFF_OVERCOMMIT_RATIO);

    init_slot_info();


    /* CPLD base address initialization */
    if (open_cpld() != PASSED) {
        cterr('f', 0, "unable to open***/dev/cpld\n");
    } 
 
    aikido_act2_flag = TRUE;
    aikido_mailbox_flag = TRUE;
    printf("\n");

    /* Plaform init */
    platform_init();

    /* 
     * NIOS must be in disable mode when diag is running
     */
    set_nios_mode(NIOS_DISABLE_MODE);

    /* Pressure Sensor initial */
    ps_init();

    /* fixme: temporary disable pluggable pcie root port */
#define PLUGGABLE_PCIE_ROOT "/sys/bus/pci/devices/0000:00:0f.0/remove"
    if (access(PLUGGABLE_PCIE_ROOT, F_OK ) != -1 ) {
    // file exists
        system("echo 1 > /sys/bus/pci/devices/0000\\:00\\:0f.0/remove");
    }

    nanook_show_cpuinfo();
    nanook_show_meminfo();
    nanook_show_m2_info();
    crocus_show_fpga_ver(0);
    nanook_show_glory_fpga_ver(0);
    get_pwr_seq_fw_rev(1);

    printf("%s", banner_string);

    /* FPGA shows board type */
    set_board_type();

    diag_menu(1, argv);         /* goto menu directly; dont' call monitor(); */

	ps_deinit();

    /* reverty to normal before leave diag */
    set_nios_mode(NIOS_NORMAL_MODE);

    return (PASSED);
}

/*****************************************************************************
 * Function: display_env
 *
 * Description: Display the Environment information
 * 
 * Inputs: None
 * 
 * Output: None
 * 
 ******************************************************************************/
void display_env(void)
{
    printf("TBD\n");

}

/*-------------------------------------------------
 * $Log: linux_main.c,v $
 * Revision 1.3  2020/04/20 02:28:24  lucywang
 *
 * 1. Fixed unplug/plug NIM module dynamically issue and added NIM cookie
 * 2. Added to support NIM Prince
 * 3. (CSCvn43011) add retry workaround for Deverton issue
 * 4. add debug message and set default value to seneors
 * 5. Reverted Register value of temp/press snsr after test
 * 6. Bumped up version to 1.0.2
 *
 * Revision 1.2  2019/12/11 10:10:32  lucywang
 * Merged Nanook to main trunk
 *
 *
 * $Endlog$
 *-------------------------------------------------
 */
