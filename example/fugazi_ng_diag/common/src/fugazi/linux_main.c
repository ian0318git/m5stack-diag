/* $Id: linux_main.c,v 1.2 2021/06/02 08:22:35 iachang Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/fugazi/linux_main.c,v $
 *------------------------------------------------------------------
 *
 * linux_main.c
 *
 * This file is diagnostic main entry
 *
 * Copyright (c) 2009-2020 by Cisco Systems, Inc.
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
#include "common.h"
#include "types.h"
#include "nvmonvars.h"
#include "menu.h"
#include "setjmps.h"
#include "mon_plat_defs.h"
#include "i2c_api.h"
#include "linux_api.h"
#include "proto.h"
#include "slot.h"
#include "sh.h"  /* struct shstuff */
#include "dash_fpga.h"
#include "uio_utils.h"
#include "router_if.h"
#include "sgmii_defs.h"
#include "plat_defs.h"
#include "platform_intr_test.h"
#include "platform_mcu.h"
#include "platform_tam_cookie.h"
#include "act2_utils.h"
#include "cross_platform.h"
#include "common.h"
#include "linux_usb_test.h"
#include "platform_temp_sensor.h"
#include "platform_synce_pll_utils.h"
#include "diag_bcm82757_test.h"
#include "diag_bcm57412_utils.h"


extern char *banner_string;
#define FUGAZI_FPGA_KLM               "uio0"
#define FUGAZI_I2C_KLM                "i2c-0"
#define FUGAZI_VTOP_KLM               "addr_vtop"
#define FUGAZI_CPLD_KLM               "cpld"
#define FUGAZI_INFO_BUF_SIZE          256

static int fd_i2c0 = -1;
static int fd_vtop = -1;
static int fd_cpld = -1;
static int quick_launch = 0;
static int pid_launch = 0;

volatile unsigned char hkeepflags;
jmp_buf monjmpbuf, *monjmpptr;

unsigned int brd_ver = 0;

extern int get_pwr_seq_fw_rev(int);
extern void set_rtc_dev(int);
extern int utility_get_rtc(int);
extern int dash_set_map(int);
extern int ttf2array(int size, const char *file, unsigned char *fpga);
static int getfirmware(char *file);
static uint open_module(int *, const char *);
static int fugazi_get_info(char*, char**, int, char*);
static int fugazi_show_meminfo(void);
static int fugazi_show_fpga_and_mb_info(void);
static int fugazi_quack_chip_reset(void);

extern unsigned char swapbyte(unsigned char c);
extern int pcie_lane_scan_test(void);
extern int show_barometer_info(void);
extern void show_fan_sts(void);
extern void display_smartfan_info(void);

unsigned int dash_fpga_fw_size = 0;
unsigned char *dash_fpga_fw_array = NULL;

mb_iofpga_mcu_regs_t  *IOFPGA_MCU_REGS;

unsigned long dash_msg = 0;
unsigned long dash_fpga = 0;
unsigned long dash_cpld = 0;
extern char *optarg;

extern boolean aikido_act2_flag;
extern boolean aikido_mailbox_flag; 

static char *fugazi_mem_info[] = {
    "MemTotal",
    "MemFree",
    "Buffers",
    "Cached",
    "SwapCached",
    "Hugepagesize",
    "DirectMap4k",
    "DirectMap2M"
};

static const uchar size_of_fugazi_mem_info = \
                   sizeof(fugazi_mem_info) / sizeof(uchar *);

static unsigned int skip_test = 0;

extern void diag_menu (int argc, const char *argv[]);

/*****************************************************************************
 *
 * Function   : force_skip_eusb
 * Description: return flag which keeps track of user intention: whether or
 *              not user wants to force software to skip testing eusb
 * Inputs     : NONE
 * Outputs    : TRUE, if software should skip testing eusb. FALSE, otherwise
 *
 *****************************************************************************/
int force_skip_eusb (void)
{
    if (skip_test & EUSB_MASK) {
        return(TRUE);
    }
    return(FALSE);
}

/*****************************************************************************
 *
 * Function   : force_skip_msata
 * Description: return flag which keeps track of user intention: whether or
 *              not user wants to force software to skip testing msata
 * Inputs     : NONE
 * Outputs    : TRUE, if software should skip testing msata. FALSE, otherwise
 *
 *****************************************************************************/
int force_skip_msata (void)
{
    if (skip_test & MSATA_MASK) {
        return(TRUE);
    }
    return(FALSE);
}

/*****************************************************************************
 *
 * Function   : get_i2c_fd
 * Description: return file descriptor for /dev/i2c0
 * Inputs     : dummy not used
 * Outputs    : file desriptor for /dev/i2c0
 *
 *****************************************************************************/
int get_i2c_fd (int dummy)
{
    return fd_i2c0;
}

/*****************************************************************************
 *
 * Function   : get_vtop_fd
 * Description: return file descriptor for vtopf driver
 * Inputs     : NONE
 * Outputs    : file desriptor for vtof driver
 *
 *****************************************************************************/
int get_vtopf_fd (void)
{
    return fd_vtop;
}

/*****************************************************************************
 *
 * Function   : cmldline_to_str
 * Description: extract command line arguments and puts them in struct shstuff
 * Inputs     : argc -- number of arguments
 *              argc -- command line argument
 * Outputs    : shp -- pointer to struct shstuff
 *
 *****************************************************************************/
static void cmdline_to_str (int argc, const char *argv[], struct shstuff *shp)
{
    int ix, len;

    /* parse argv; store into buffer; invoke cli */
    for (ix = 0, len = 0; ix < argc; ix++) {
        shp->argv[ix] = (char *)argv[ix];
        len += strlen(argv[ix]);
        strcat(shp->cmdptr, argv[ix]);
        if (ix+1 < argc)
            shp->cmdptr[len++] = ' ';
        shp->cmdptr[len] = '\0';
        
    }
    shp->argc = argc;

}

/*****************************************************************************
 *
 * Function   : get_platform_plane
 * Description: just return control plane
 * Inputs     : NONE
 *              
 * Outputs    : CP
 *
 *****************************************************************************/
int get_platform_plane (void)
{
    return CP;  /*defined in dash_fpga.h */
}

/*****************************************************************************
 *
 * Function   : open_cpld
 * Description: open cpld driver
 * Inputs     : NONE
 *              
 * Outputs    : return file descript of cpld driver
 *
 *****************************************************************************/
static int open_cpld (void)
{
    void *ptr;
    if (open_module(&fd_cpld, FUGAZI_CPLD_KLM)==PASSED) {

        ptr = (void *)mmap(NULL, CPLD_SIZE, (PROT_READ | PROT_WRITE),
                           MAP_SHARED, fd_cpld, 0xFED40000);
        if (ptr == MAP_FAILED) {
            close(fd_cpld);
            perror("Error mmapping the file for CPLD (TPM space)");
            return (FAILED);
        }
        dash_cpld = (unsigned long)ptr;
#if 0
        printf("CPLD version  %#x %#x\n", *((unsigned int *)((long)dash_cpld + 0x80)),
               *((unsigned int *)((long)dash_cpld + 0x84))  );
#endif
        
    } else {
        /* we need this only for cpld uitility; so if we fail it's ok  */
        printf("*****can't open cpld mmap driver....*******\n");
        return -1;
    }
    return 0;
}

/*****************************************************************************
 *
 * Function   : open_module
 * Description: Wrap function to open module
 * Inputs     : The file descriptor pointer of module and module_type
 * Outputs    : PASSED/FAILED
 *
 *****************************************************************************/
static uint open_module (int *fd, const char *name)
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
 * Function   : get_ngio_pcie_bus_num
 * Description: Function to get system PCIe bus number for NGIO slots
 * Inputs     : void
 * Outputs    : bus number
 *
 *******************************************************************************
 */
uint32_t get_ngio_pcie_bus_num (void)
{
    printf("%s, obseleted \n", __FUNCTION__);
    printf("%s, %s : Need to define FUGAZI_NGIO_PCIE_BUS_NUM correctly \n",
            __FILE__, __FUNCTION__);
    return(FUGAZI_NGIO_PCIE_BUS_NUM);
}

/*****************************************************************************
 *
 * Function   : fugazi_get_info
 * Description: Get Fugazi system Info by reading related linux file.
 * Inputs     : info_file - related Linux file to get info
 *              info_item - the item to collect
 *              info_item_size - size of items that needs to collect
 *              file_name - file to put the collected info
 * Outputs    : PASSED/FAILED
 *
 *****************************************************************************/
static int fugazi_get_info (char* info_file, char** info_item, 
                            int info_item_size, char* file_name)
{
    int index = 0;
    char sys_cmd[256];

    /* Read out the needed Info from related Linux info_file */
    for (index = 0; index < info_item_size; index++) {
        sprintf(sys_cmd, "cat %s | grep -m 1 '%s' >> %s",
                info_file, info_item[index], file_name);
        system(sys_cmd);
    }
    return (PASSED);
}

/*****************************************************************************
 *
 * Function   : fugazi_show_meminfo
 * Description: To show Fugazi MEM Info by reading file "/proc/meminfo".
 * Inputs     : None
 * Outputs    : PASSED/FAILED
 *
 *****************************************************************************/
static int fugazi_show_meminfo (void)
{
    int rc = FAILED;
    char mem_info[FUGAZI_INFO_BUF_SIZE];
    FILE  *fp;
 
    fp = fopen("/fugazi_meminfo.txt", "r");
    if (fp == NULL) {
        rc = fugazi_get_info("/proc/meminfo", fugazi_mem_info,
                           size_of_fugazi_mem_info, "/fugazi_meminfo.txt");
        if (rc != PASSED) {
            printf("Failed to get Fugazi MEM Info !!!\n");
            return (rc);
        } else {
            fp = fopen("/fugazi_meminfo.txt", "r");
            if (fp == NULL) {
                printf("Failed to open /fugazi_meminfo.txt");
                return (FAILED);
            }
        }
    }

    printf("MEM info after inits:\n");
    while (fgets(mem_info, FUGAZI_INFO_BUF_SIZE, fp) != NULL) {
        printf("%s", mem_info);
    }
    printf("\n");

    fclose(fp);

    return (PASSED);
}

/*******************************************************************************
 *
 * Function   : fugazi_show_fpga_and_mb_info
 * Description: To show Fugazi FPGA version and Board Info.
 * Inputs     : None
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
static int fugazi_show_fpga_and_mb_info (void)
{
    unsigned int fpga_ver = 0, cpld_ver = 0, fpga_brd = 0, cpld_brd = 0;

    get_platform_ver(0, &cpld_ver, &fpga_ver, &cpld_brd, &fpga_brd);

    brd_ver = ((fpga_ver & FPGA_BD_HW_REV_MSK) >> FPGA_BD_HW_REV_SHFT);

    printf("FPGA rev = %#x; Board ID = %#x.\n", fpga_ver, fpga_brd);

    return (PASSED);
}

/*******************************************************************************
 *
 * Function   : fugazii_processor_check
 * Description: Check CPU speed, core and processor counts.
 * Inputs     : None
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
static int fugazii_processor_check (void)
{
    char sys_cmd[256];
    char *temp_file="temp_file";
    FILE *fp;
    int core_n, proc_n, exp_core_n, exp_proc_n;
    char *exp_cpu_modname_0, *exp_cpu_modname_1, cpu_modname[FUGAZI_INFO_BUF_SIZE];
    char file_path_redhat[FUGAZI_INFO_BUF_SIZE];
    char file_path_fedora[FUGAZI_INFO_BUF_SIZE];
    int  ctr, flag;
    size_t size = 0;


    exp_core_n = FUGAZI_CORE_NUM;
	exp_proc_n = FUGAZI_PROC_NUM;
	exp_cpu_modname_0 = SKYLAKE_XEON_2_0GHZ_CPU;
	exp_cpu_modname_1 = SKYLAKE_XEON_2_0GHZ_CPU;

    /* Check CPU model name and speed
     */
    sprintf(sys_cmd, "rm -f %s; cat /proc/cpuinfo | grep 'model name' | uniq -d > %s", temp_file, temp_file);
    system(sys_cmd);

    fp = fopen(temp_file, "r");
    if (fp == NULL) {
        printf("Failed to open %s", temp_file);
	return (FAILED);
    }
    fscanf(fp, "%[^\n]", cpu_modname); /* scan in 1 line till \n */
    fclose(fp);

    flag = 0;
    if (strstr(cpu_modname, exp_cpu_modname_0) == NULL) {
        flag = 1;
	if (strstr(cpu_modname, exp_cpu_modname_1) == NULL) {
	    cterr('f', 0, "CPU %s is not correct. \nExpect: %s or %s", cpu_modname, exp_cpu_modname_0, exp_cpu_modname_1);
	    return (FAILED);
	}
    }

    /* Check core counts
     */
    sprintf(sys_cmd, "rm -f %s; cat /proc/cpuinfo | grep 'core id' | sort | uniq -d | wc -l > %s", temp_file, temp_file);
    system(sys_cmd);

    fp = fopen(temp_file, "r");
    if (fp == NULL) {
        printf("Failed to open %s", temp_file);
	return (FAILED);
    }
    fscanf(fp, "%d", &core_n);
    fclose(fp);

    if (core_n != exp_core_n) {
        cterr('f', 0, "CPU core number %d is not correct, expect %d", core_n, exp_core_n);
	return (FAILED);
    }

    /* Check processor count
     */
    for (proc_n =0, ctr = 0; ctr < exp_proc_n; ctr++) {
        sprintf(file_path_redhat, "/dev/cpu%d", ctr);
        sprintf(file_path_fedora, "/dev/cpu/%d/cpuid", ctr);
	/* pfix-temp. We are in the middle of switching to fedora
	 * rootfs so this check need to work for both redhat and
	 * fedora for now.
	 */
	if (file_exist(file_path_redhat, &size) ||
            file_exist(file_path_fedora, &size)) {
            proc_n++;
        } else {
	    cterr('f', 0, "Expected %d processors, but processor %d is not present", exp_proc_n, ctr);
            return (FAILED);
        }
    }
    if (flag == 0) {
        printf("\nCPU check passed: %s, %d cores, %d processors\n", exp_cpu_modname_0, core_n, proc_n);
    } else {
        cterr('w', 0, "If this Fugazi is P1C and above, It is an error to use the Genuine Intel version of Broadwell.\n"
	      "CPU check result: %s, %d cores, %d processors\n", exp_cpu_modname_1, core_n, proc_n);
    }
    return (PASSED);
}

/*******************************************************************************
 *
 * Function   : fugazi_quack_chip_reset
 * Description: This function is to reset Quack chip, and the reset process is
 *              for Fugazi Diag specifically.
 *              In Fugazi, we need to reset Quack chip by FPGA right after
 *              Diag is up from Rommon to ensure the Quack chip in valid state.
 *              Besides, only need to reset one time before system reboot.
 * Inputs     : None
 * Outputs    : PASSED
 *
 *******************************************************************************
 */
static int fugazi_quack_chip_reset (void)
{
    char fname[32], cmd[32];
    size_t size;

    /* it seems we dont need it anymore */
    return (PASSED); 

    sprintf(fname, "/tmp/fugazi_quack_is_reseted");
    if (file_exist(fname, &size)) {
        return(PASSED);
    }

    /* Reset Quack chip (ACT2) */
    act2_reset(0);

    /* Record this time Reset */
    sprintf(cmd, "echo > %s;", fname);
    system(cmd);

    return (PASSED);
}

/*
 *********************************************************************
 *
 * Function   : is_special_case
 * Description: This function is used to check if user intends to run
 *              cli or if user just wants to launch diag without 
 *              initializing peripherals, or if user wants to download.
 *              dash fpga.
 * Inputs     : int argc, number of command line arguments
 *              char **argv, buffer containing command line strings
 * Outputs    : FALSE, if user wants to run CLI. TRUE, otherwise
 *
 *********************************************************************
 */

int is_special_case (int argc, const char *argv[])
{
    quick_launch = 0;

    if (argv[1][0] == '1') {
        quick_launch = 1;
        return(TRUE);
    }

    if (strstr(argv[1], "size")) {
        if (argv[1][5] != 'd') {
            dash_fpga_fw_size = atoi(&argv[1][5]);
                     if (dash_fpga_fw_size < 1000) {
                cterr('f', 0, "FPGA size is too small");
            }
                
        }
        if (getfirmware((char *)&argv[2][0])==FAILED) {
            exit(-1);
        }
            
        return(TRUE);
    }

    return(FALSE);
}

/*
 *********************************************************************
 *
 * Function   : getfirmware
 * Description: This function opens fpga file and stores into
 *                  dash_fpga_fw_array variable.
 * Inputs     : char * file. fpag file
 *              int size, size of fpga.
 * Outputs    : PASSED, if successful opening and storing dash fpga
 *              into dash_fpag_fw_arry. FAILED otherwise.
 *
 *********************************************************************
 */
int getfirmware (char *file)
{
    int bytes, i;
    FILE *fp;
    char c;
    /* usage: fugazi_lnx size=file_size file_name; if file_size=='d',
       then use default value , or
       fugazi_lnx -f file_name -s file_size */
    printf("Dash file name is %s\n", file);

    if (dash_fpga_fw_size == 0) {
        fp = fopen(file, "r");
        if (fp == NULL) {
            printf("can't open file");
            perror("");
            exit(0);
        }
        while ((c = fgetc(fp)) != EOF) {
            if (c == ',')
                dash_fpga_fw_size++;
        }
        dash_fpga_fw_size++;
        fclose(fp);
    }
    printf("%d values to be programmed.\n", dash_fpga_fw_size);
    dash_fpga_fw_array = (uchar *)malloc(dash_fpga_fw_size+1000);
    if (!dash_fpga_fw_array) {
        printf("\n\ncan't allocate memory for fpga \n\n");
        return(FAILED);
    }
    bytes = ttf2array(dash_fpga_fw_size, file, dash_fpga_fw_array);

    printf("Last 200 values of FPGA file\n");
    bytes -= 200;
    for (i=0; i<200; i++, bytes++) {
        if (!(i % 16))
            printf("\n");
        printf("%3d ", swapbyte(dash_fpga_fw_array[bytes]));
    }

    return(PASSED);
}

/*
 *********************************************************************
 *
 * Function    : platform_env_status 
 * Description : This function is used to display plug-in items and  
 *               status such as barometer, temperature etc...
 * Inputs      : NONE 
 * Outputs     : NONE 
 *
 *********************************************************************
 */
void platform_env_status (void)
{

    /* CPU system info  */
    fugazii_processor_check();
    fugazi_show_meminfo();

    /* Platform SKU info check */
    fugazi_show_fpga_and_mb_info();
    /* check pcie lanes number */
    if (!quick_launch) {
        if (pcie_lane_scan_test() == FAILED) {
            cterr('f', 0, "PCIe lane scan failed\n");
        } else {
            printf("PCIe lanes scan passed!!\n");
        }
    }
    /* Show fan status */
    show_fan_sts();
    display_smartfan_info();

    /* Show temperature */
    show_temperature_all();

    /* Display barometer */
    show_barometer_info();

    /* Get RTC time */
    utility_get_rtc(1);

    /* Show MCU version */
    get_pwr_seq_fw_rev(1);

    /* Show syncE PLL firmware version */
    idt8a3_show_fw_version();

    /*Check for eUSB and eMMC and print size */
    check_block_size(DEV_USB0);
    check_block_size(DEV_USB1);
    check_block_size(DEV_EMMC);
    check_block_size(DEV_M2USB);
}

/*****************************************************************************
 *
 * Function   : main
 * Description: entry point of Fugazi Plagform
 * Inputs     : argc, number of argument
 *              argv, command line arguments
 *              
 * Outputs    : exit status
 *
 *****************************************************************************/
int
main (int argc, const char *argv[])
{
    struct shstuff sh;
    int cli_argc = 1, opt_ch;
    const char **cli_argv = (const char **)argv;
    unsigned int size;
    DIAGFLAG = 0;
    char fpga_file[80];
    uint rc = FAILED;

    fpga_file[0] = '\0';
    skip_test = opt_ch = 0;
    sh.cmdptr = NULL;
    system("echo 3 > /proc/sys/kernel/printk"); /* Disable Kenerl info. */

    if (argc > 1 ) {
        if (argv[1][0] != '-') {
            if (is_special_case(argc, argv)) {

            } else {
                /* ignore first arugment which is the name of our program */
                cli_argc = argc - 1;
                cli_argv = argv + 1;

                /* creae buffer for CLI */
                sh.cmdptr = malloc(1024);
                *sh.cmdptr= '\0';

                /* copy CLI into sh struct */
                cmdline_to_str(cli_argc, cli_argv, &sh);
            }
        } else {
            while ((opt_ch = getopt(argc, (char **)argv, ":cqs:f:")) >= 0) {
                switch(opt_ch) {
                case 'q':
                    quick_launch = 1;
                    break;
                case 's':
                    size = atoi(optarg);
                    printf("size entered is %d\n", size);
                    dash_fpga_fw_size = size;
                    break;
                case 'f':
                    sprintf(fpga_file, "%s", optarg);
                    break;
                case 'c':
                    pid_launch = 1;
                    break;
                case '?':
                    goto usage;
                    break;
                } /* switch */
            } /* while getopt */
        } /* if argv[1][0] != "-" */
    } /* if argc > 1 */

    if (*fpga_file) {
        if (getfirmware(fpga_file)==FAILED) {
            cterr('f', 0, "unable to read  FPGA firmware file");
        }
    }

    open_cpld();

    /* Open modules */
    if (open_module(&fd_i2c0, FUGAZI_I2C_KLM) != PASSED) {
        cterr('f', 0, "%s: Failed to open /dev/%s", __FUNCTION__, FUGAZI_I2C_KLM);
    }

    if (open_module(&fd_vtop, FUGAZI_VTOP_KLM) != PASSED) {
        cterr('f', 0, "%s: Failed to open /dev/%s", __FUNCTION__, FUGAZI_VTOP_KLM);
    }

    rc = uio_open();
    if (rc != PASSED ) {
        cterr('f', 0, "unable to open***/dev/uio***\n");
    }
    dash_fpga = (unsigned long)uio_get_regs();
    assert(dash_fpga);
    dash_set_map(0); /* call only after open uio drivder */

    platform_init_intr();

    /* Reset Quack chip */
    fugazi_quack_chip_reset();

    set_board_type();
    
    aikido_act2_flag = TRUE; 
    aikido_mailbox_flag = TRUE; 

    /* NIOS must be in disable mode when diag is running
     */
    set_nios_mode(NIOS_DISABLE_MODE);


    /* Display major platform info and status
     */
    platform_env_status();

    /* Turn off memory malloc overcommit to avoid oom in memory test
     */
    system("echo 2 > /proc/sys/vm/overcommit_memory");

    /* HOST MIDO BUS Acquire */
    bcm57412_mdio_bus_acquire(TRUE); 

    if (sh.cmdptr != NULL) {
        if (sh.cmdptr[0])
            free(sh.cmdptr);
    } else {
        printf("%s", banner_string);

#ifdef DISPLAY_ENGINEERING_BANNER
	engineering_banner();
#endif
        set_rtc_dev(0);
        fflush(stdin);
        diag_menu(1, argv); /* goto menu directly; dont' call monitor(); */
    }

    if (dash_fpga_fw_size)
        free(dash_fpga_fw_array);

    /* reverty to normal before leave diag */
    set_nios_mode(NIOS_NORMAL_MODE); 
    return 0;

 usage:
    printf("Usage: ./fugazilnx [-f dash_fpga_file] "
           "[-s fpga_size]\n");
    printf("                   [-q]\n");
    printf("                   [-c]\n");
    printf("       -f filename: load FPGA\n");
    printf("       -q launch diag without initialzing peripheral intf \n");
    printf("       -c Display FPGA board type and cookie PID without launching Diag application\n");
    printf("       -s size: specify  FPGA size. if not specified, use default\n");

    exit(0);
}


/*-------------------------------------------------
 * $Log: linux_main.c,v $
 * Revision 1.2  2021/06/02 08:22:35  iachang
 * CSCvo59196-33 : Merge Fugazi from ASR1K-main-branch to ISR main trunk
 *
 * Revision 1.1.8.4  2020/12/21 02:18:25  iachang
 * Added temperature display when Diag App boot up.
 *
 * Revision 1.1.8.3  2020/10/22 01:50:23  iachang
 * Remove un-used code.
 *
 * Revision 1.1.8.2  2020/08/26 02:37:49  iachang
 * Merge Fugazi code into main trunk
 *
 * Revision 1.1.6.12  2020/08/06 11:19:34  iachang
 * Display smartfan info when Diag boot up.
 *
 * Revision 1.1.6.11  2020/08/04 08:37:06  iachang
 * Update Copyright to 2020
 *
 * Revision 1.1.6.10  2020/07/03 07:07:29  iachang
 * HOST MIDO BUS Acquire when platform initial.
 *
 * Revision 1.1.6.9  2020/05/19 01:39:49  iachang
 * Correct the Diag APP usage message.
 *
 * Revision 1.1.6.8  2020/05/08 09:35:22  iachang
 * launch diag without pcie scan. (./fugazilnx -q)
 *
 * Revision 1.1.6.7  2019/05/14 02:01:01  pdoong
 * Added to sysyem info to display SyncE/bam82757 firmware version
 *
 * Revision 1.1.6.6  2019/04/26 22:08:37  letsai
 * 1. Fixed FPGA board type unknow.
 * 2. Bump up to 0.0.3
 * 3. Remove m2sata related word.
 *
 * Revision 1.1.6.5  2019/04/08 16:43:11  letsai
 * Remove sku check when power on. (There is only one sku for Fugazi)
 *
 * Revision 1.1.6.4  2019/04/06 01:07:50  iachang
 * BCM82757 10G PHY pass clause 45 parameter into bnxt_en driver. This change also need driver support
 *
 * Revision 1.1.6.3  2019/03/28 19:00:34  letsai
 * 1. Modify FPGA interrupt test and utility.
 * 2. Modify I2C address of PSU2.
 * 3. Clean up code.
 * 4. Merge M.2 NVME and M.2 USB tests to combo test.
 *
 * Revision 1.1.6.2  2019/03/14 03:48:36  letsai
 * Initial check in.
 *
 *
 *
 *
 * $Endlog$
 * */
