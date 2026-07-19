/* $Id: linux_main.c,v 1.3 2020/12/29 03:09:03 leschen Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/overlord/neptune/curie_1RU/linux_main.c,v $
 *------------------------------------------------------------------
 * by: mcharon
 * 5/2008
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
#include "bcm_gesw_defs.h"
#include "plat_defs.h"
#include "platform_eth_pkt_txrx.h"
#include "platform_intr_test.h"
#include "platform_pem_fan.h" /* Juno fan info */
#include "platform_cookie.h"
#include "act2_utils.h"
#include "cross_platform.h"
#include "common.h"
#include "linux_usb_test.h"
#include "plug_slot.h"
#include "plug_host_fpga_lib.h"
#include "bcm57412_test.h"
#include "fpga_smartfan.h"

extern char *banner_string;
#define OVLD_FPGA_KLM               "uio0"
#define OVLD_I2C_KLM                "i2c-0"
#define OVLD_VTOP_KLM               "addr_vtop"
#define OVLD_CPLD_KLM               "cpld"
#define OVLD_INFO_BUF_SIZE          256

static int fd_i2c0 = -1;
static int fd_vtop = -1;
static int fd_cpld = -1;
static int quick_launch = 0;
static int pid_launch = 0;

int quiet_launch = 0;

volatile unsigned char hkeepflags;
jmp_buf monjmpbuf, *monjmpptr;

unsigned int brd_ver = 0;

extern void local_mac_addrs_init(void); 
extern int get_pwr_seq_fw_rev(int);
extern void set_rtc_dev(int);
extern int utility_get_rtc(int);
extern int dash_set_map(int);
extern int ovld_check_system_pressure(void);
extern int ttf2array(int size, const char *file, unsigned char *fpga);
static int getfirmware(char *file);
static uint open_module(int *, const char *);
static int ovld_get_info(char*, char**, int, char*);
static int ovld_show_meminfo(void);
static int ovld_show_fpga_and_mb_info(void);
static int ovld_quack_chip_reset(void);
static void skip_slot(void); 

extern unsigned char swapbyte(unsigned char c);
extern void overdrive_fixup(void);
extern int pcie_lane_scan_test(int);
extern int show_barometer_info(void);
extern void show_fan_sts(void);
extern void display_smartfan_info(void);
extern uint32_t ovld_check_poe_psu_wrap(void);

unsigned int dash_fpga_fw_size = 0;
unsigned char *dash_fpga_fw_array = NULL;
unsigned int curie_1ru_skip_ngio_slots; 

mb_iofpga_mcu_regs_t  *IOFPGA_MCU_REGS;

unsigned long dash_msg = 0;
unsigned long dash_fpga = 0;
unsigned long dash_cpld = 0;
extern char *optarg;

extern boolean aikido_act2_flag;
extern boolean aikido_mailbox_flag; 

static char *ovld_cpu_info[] = {
    "cpu cores",
    "vendor_id",
    "cpu family",
    "model",
    "model name",
    "stepping",
    "apicid",
    "cpu MHz",
    "cache size",
    "clflush size",
    "address sizes",
    "flags"
};

static const uint size_of_ovld_cpu_info = \
                  sizeof(ovld_cpu_info) / sizeof(uchar *);

static char *ovld_mem_info[] = {
    "MemTotal",
    "MemFree",
    "Buffers",
    "Cached",
    "SwapCached",
    "Hugepagesize",
    "DirectMap4k",
    "DirectMap2M"
};

static const uchar size_of_ovld_mem_info = \
                   sizeof(ovld_mem_info) / sizeof(uchar *);

static unsigned int skip_test = 0;


extern void diag_menu (int argc, const char *argv[]);

/*****************************************************************************
 *
 * Function   : get_i2c_fd
 * Description: return file descriptor for /dev/i2c0
 * Inputs     : dummy not used
 * Outputs    : file desriptor for /dev/i2c0
 *
 *****************************************************************************/
int
get_i2c_fd (int dummy)
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
int
get_vtopf_fd (void)
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
static void
cmdline_to_str (int argc, const char *argv[], struct shstuff *shp)
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
int
get_platform_plane (void)
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
static int
open_cpld (void)
{
    void *ptr;
    if (open_module(&fd_cpld, OVLD_CPLD_KLM)==PASSED) {

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

/*******************************************************************************
 *
 * Function   :	get_ngio_pcie_bus_num
 * Description:	Function to get system PCIe bus number for NGIO slots
 * Inputs     :	void
 * Outputs    : bus number
 *
 *******************************************************************************
 */
uint32_t get_ngio_pcie_bus_num (void)
{
    printf("%s, obseleted \n", __FUNCTION__); 
    printf("%s, %s : Need to define CURIE_1RU_NGIO_PCIE_BUS_NUM correctly \n", 
            __FILE__, __FUNCTION__); 
    return(CURIE_1RU_NGIO_PCIE_BUS_NUM); 
}

/*****************************************************************************
 *
 * Function   : open_module
 * Description: Wrap function to open module
 * Inputs     : The file descriptor pointer of module and module_type
 * Outputs    : PASSED/FAILED
 *
 *****************************************************************************/
static uint
open_module (int *fd, const char *name)
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
 * Function   : ovld_get_info
 * Description: Get Overlord system Info by reading related linux file.
 * Inputs     : info_file - related Linux file to get info
 *              info_item - the item to collect
 *              info_item_size - size of items that needs to collect
 *              file_name - file to put the collected info
 * Outputs    : PASSED/FAILED
 *
 *****************************************************************************/
static int
ovld_get_info (char* info_file, char** info_item, 
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
 * Function   : ovld_show_meminfo
 * Description: To show Overlord MEM Info by reading file "/proc/meminfo".
 * Inputs     : None
 * Outputs    : PASSED/FAILED
 *
 *****************************************************************************/
static int
ovld_show_meminfo (void)
{
    int rc = FAILED;
    char mem_info[OVLD_INFO_BUF_SIZE];
    FILE  *fp;
 
    fp = fopen("/ovld_meminfo.txt", "r");
    if (fp == NULL) {
        rc = ovld_get_info("/proc/meminfo", ovld_mem_info,
                           size_of_ovld_mem_info, "/ovld_meminfo.txt");
        if (rc != PASSED) {
            printf("Failed to get Overlord MEM Info !!!\n");
            return (rc);
        } else {
            fp = fopen("/ovld_meminfo.txt", "r");
            if (fp == NULL) {
                printf("Failed to open /ovld_meminfo.txt");
                return (FAILED);
            }
        }
    }

    printf("MEM info after inits:\n");
    while (fgets(mem_info, OVLD_INFO_BUF_SIZE, fp) != NULL) {
        printf("%s", mem_info);
    }
    printf("\n");

    fclose(fp);

    return (PASSED);
}

/*******************************************************************************
 *
 * Function   : ovld_show_fpga_and_mb_info
 * Description: To show Overlord FPGA version and Board Info.
 * Inputs     : None
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
static int
ovld_show_fpga_and_mb_info (void)
{
    unsigned int fpga_ver = 0, cpld_ver = 0, fpga_brd = 0, cpld_brd = 0;

    get_platform_ver(0, &cpld_ver, &fpga_ver, &cpld_brd, &fpga_brd);

    brd_ver = ((fpga_ver & FPGA_BD_HW_REV_MSK) >> FPGA_BD_HW_REV_SHFT);

    printf("FPGA rev = %#x; Board ID = %#x.\n", fpga_ver, fpga_brd);

    return (PASSED);
}

/*******************************************************************************
 *
 * Function   : curie1ru_processor_check
 * Description: Check CPU speed, core and processor counts.
 * Inputs     : None
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
static int
curie1ru_processor_check (void)
{
    char sys_cmd[256];
    char *temp_file="temp_file";
    FILE *fp;
    int core_n, proc_n, exp_core_n, exp_proc_n;
    char *exp_cpu_modname_0, *exp_cpu_modname_1, cpu_modname[OVLD_INFO_BUF_SIZE];
    char file_path_redhat[OVLD_INFO_BUF_SIZE];
    char file_path_fedora[OVLD_INFO_BUF_SIZE];
    int  ctr, flag;
    size_t size = 0;

    /* Cruie 1RU P1A and P1B proto use the early cpu version with
     * "Genuine" string.
     * P1C uses the production version with "Xeon" string.
     */
    if (is_thallium()) {
        exp_core_n = THALLIUM_CORE_NUM;
	exp_proc_n = THALLIUM_PROC_NUM;
	exp_cpu_modname_0 = BROADWELL_XEON_2_00GHZ_CPU;
	exp_cpu_modname_1 = BROADWELL_GENUINE_2_00GHZ_CPU;
    }
    else if (is_polonium()) {
        exp_core_n = POLONIUM_CORE_NUM;
	exp_proc_n = POLONIUM_PROC_NUM;
	exp_cpu_modname_0 = BROADWELL_XEON_2_00GHZ_CPU;
	exp_cpu_modname_1 = BROADWELL_GENUINE_2_00GHZ_CPU;
    }
    else {
        exp_core_n = RADIUM_CORE_NUM;
	exp_proc_n = RADIUM_PROC_NUM;
	exp_cpu_modname_0 = BROADWELL_XEON_2_70GHZ_CPU;
	exp_cpu_modname_1 = BROADWELL_GENUINE_2_70GHZ_CPU;
    }

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
        cterr('w', 0, "If this Curie-1RU is P1C and above, It is an error to use the Genuine Intel version of Broadwell.\n"
	      "CPU check result: %s, %d cores, %d processors\n", exp_cpu_modname_1, core_n, proc_n);
    }
    return (PASSED);
}

/*******************************************************************************
 *
 * Function   : ovld_quack_chip_reset
 * Description: This function is to reset Quack chip, and the reset process is
 *              for Overlord Diag specifically.
 *              In Overlord, we need to reset Quack chip by FPGA right after
 *              Diag is up from Rommon to ensure the Quack chip in valid state.
 *              Besides, only need to reset one time before system reboot.
 * Inputs     : None
 * Outputs    : PASSED
 *
 *******************************************************************************
 */
static int
ovld_quack_chip_reset (void)
{
    char fname[32], cmd[32];
    size_t size;

    /* it seems we dont need it anymore */
    return (PASSED); 

    sprintf(fname, "/tmp/ovld_quack_is_reseted");
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

int
is_special_case (int argc, const char *argv[])
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
int
getfirmware (char *file)
{
    int bytes, i;
    FILE *fp;
    char c;
    /* usage: utah_lnx size=file_size file_name; if file_size=='d',
       then use default value , or
       utah_lnx -f file_name -s file_size */
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

/*****************************************************************************
 *
 * Function   : skip_slot
 * Description: skip ngio slot for different sku 
 * Inputs     : none
 * Outputs    : none
 *
 *****************************************************************************/
static void skip_slot (void) 
{
    /* polonium : SM1, NIM1, NIM2
     * radium/thallium   : SM1, NIM1
     */
    if (is_radium() || is_thallium()) { 
        curie_1ru_skip_ngio_slots = (SKIP_NIM2); 
    } 
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
    int sku_num;

    /* CPU system info  */
    curie1ru_processor_check();
    ovld_show_meminfo();

    /* Platform SKU info check */
    ovld_show_fpga_and_mb_info();
    if (chk_plat_sku(&sku_num) == FALSE) {
        cterr('f', 0, "Platform SKU check failed\n");
    }

    /* check motherboard pcie lanes number */
    if (pcie_lane_scan_test(0) == FAILED) {
        cterr('f', 0, "Motherboard PCIe lane scan failed\n");
    }
    else {
        printf("\n");
    }

    /* Show fan status */
    show_fan_sts();
    display_smartfan_info();

    /* Display barometer */
    show_barometer_info();

    /* Get RTC time */
    utility_get_rtc(1);

    /* Show MCU version */
    get_pwr_seq_fw_rev(1);

    /*Check for eUSB and eMMC and print size */
    check_block_size(DEV_USB0);
    check_block_size(DEV_USB1);
    check_block_size(DEV_EMMC);
    check_block_size(DEV_M2SATA);
    check_block_size(DEV_EUSB);
}

/*
 *********************************************************************
 *
 * Function    : get_gesw_pname 
 * Description : Return GESW port name   
 *               This is not applicable to Curie
 * Inputs      : port_num  
 * Outputs     : Port name of GESW 
 *
 *********************************************************************
 */
char *get_gesw_pname(int port_num)
{
    /* Curie - No GE-switch */
    char *gesw_port_name = "is not applicable to Curie";
    return (gesw_port_name); 
}

/*
 *********************************************************************
 *
 * Function    : is_plat_10gkr_capable 
 * Description : Curie has 10-KR capability on both ge0 and ge1   
 * Inputs      : NONE 
 * Outputs     : TRUE - Has 10G-KR capability 
 *
 *********************************************************************
 */
int is_plat_10gkr_capable(void)
{
    return (TRUE); 
}

/*****************************************************************************
 *
 * Function   : main
 * Description: entry point of Curie 1RU
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
    init_slot_info();

    /* Open modules */
    if (open_module(&fd_i2c0, OVLD_I2C_KLM) != PASSED) {
        cterr('f', 0, "%s: Failed to open /dev/%s", __FUNCTION__, OVLD_I2C_KLM);
    }

    if (open_module(&fd_vtop, OVLD_VTOP_KLM) != PASSED) {
        cterr('f', 0, "%s: Failed to open /dev/%s", __FUNCTION__, OVLD_VTOP_KLM);
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
    ovld_quack_chip_reset();

    set_board_type();
    
    /* if curie 1ru p1c and later, there is aikido only */
    if (is_curie_1ru_p1c_and_later()) { 
        aikido_act2_flag = TRUE; 
        aikido_mailbox_flag = TRUE; 
    }

    if (pid_launch) {
        get_plat_sku_cookie();
        return (PASSED);
    }

    /* NIOS must be in disable mode when diag is running
     */
    set_nios_mode(NIOS_DISABLE_MODE);


    /* Display major platform info and status
     */
    platform_env_status();

    /* Checking POE PSU  */
    ovld_check_poe_psu_wrap();  

    /* skip the ngio, based on the sku */
    skip_slot(); 

    /* Turn off memory malloc overcommit to avoid oom in memory test
     */
    system("echo 2 > /proc/sys/vm/overcommit_memory");

    /* init local mac address */
    local_mac_addrs_init();

    /* Initialize pluggable information */
    init_plug_info();
    printf("\n");

    /* power off all pluggable module*/
    plug_module_power_off(PLUG_SLOT_1);        

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
    printf("Usage: ./nepx86_diag [-f dash_fpga_file] "
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
$Log: linux_main.c,v $
Revision 1.3  2020/12/29 03:09:03  leschen
Remove bnxt_en operations.

Revision 1.2  2019/08/06 06:56:12  alpeng
merge curie, switzer and nightwatch to trunk

Revision 1.1.2.32  2019/06/27 07:32:14  meho
Added display smart fan info utility

Revision 1.1.2.31  2019/04/03 08:40:44  alpeng
add psu poe check back to curie per HW request.

Revision 1.1.2.30  2019/03/27 01:13:51  ptong
Release curie 1ru V1.3.1 : Separate motheboard PCIe and M.2 NVMe care PCIe scan tests. Use /dev/m2usb for M.2 USB module according to kernel change. Check FPGA M.2 module bits correctly to support M.2 slot test

Revision 1.1.2.29  2019/02/11 07:33:37  meho
Support new PIM test-card (PCIe)

Revision 1.1.2.28  2019/01/28 07:21:02  leschen
Enhancement 1G speed settings to fix the bcm57412 external loopback issue on Thallium.

Revision 1.1.2.27  2019/01/28 02:20:46  alpeng
skip bzimage version check

Revision 1.1.2.26  2019/01/17 08:50:23  leschen
Remove/insert bnxt_en driver when launching/exit diag to support bcm57412 sm.

Revision 1.1.2.25  2019/01/17 06:36:57  alpeng
read cookie from aikido for p1c and later

Revision 1.1.2.24  2018/12/27 08:55:41  alpeng
clean up code

Revision 1.1.2.23  2018/12/05 01:54:54  ptong
Change the Intel CPU model string for P1C board

Revision 1.1.2.22  2018/12/04 08:39:41  alpeng
update nios setup algorithm and remove useless nios setting; disable nios before launch diag menu; set to normal mode before leaveing diag menu; update fan util for remove fan4 and fixed enable/disable fan util

Revision 1.1.2.21  2018/10/29 09:18:57  leschen
Move get_gesw_pname to linux_main.c and return gesw port name is not applicable to Curie.

Revision 1.1.2.20  2018/10/26 07:34:09  leschen
Move function is_plat_10gkr_capable to linux_main.c and return true to support 10g-kr.

Revision 1.1.2.19  2018/10/16 09:05:39  meho
Pluggable re-structured

Revision 1.1.2.18  2018/10/08 21:53:39  ptong
Minor change. Remove unnecessary system calls

Revision 1.1.2.17  2018/10/05 03:26:14  alpeng
fixed get mac address function to return br0 mac address

Revision 1.1.2.16  2018/09/28 00:09:16  ptong
Thallium and Polonium no longer had H and L models

Revision 1.1.2.15  2018/09/27 09:46:24  alpeng
support tam lib and aikido for curie

Revision 1.1.2.14  2018/09/18 09:46:54  meho
Support Thallium SKU.

Revision 1.1.2.13  2018/09/13 23:18:42  ptong
Add Thallium support in linux_main.c and mb_tests.c

Revision 1.1.2.12  2018/08/24 22:28:30  meho
Removed Star detection function

Revision 1.1.2.11  2018/08/18 21:09:56  ptong
Add CPU speed and core check for Curie-1RU

Revision 1.1.2.10  2018/08/18 07:03:12  meho
Added init pluggable in main

Revision 1.1.2.9  2018/08/14 00:36:27  alpeng
remove set ip addr for gesw, curie does not need it

Revision 1.1.2.8  2018/08/09 10:18:59  alpeng
support NTC on Curie NIM1

Revision 1.1.2.7  2018/08/08 20:12:55  meho
Removed BCM GE SW init function.

Revision 1.1.2.6  2018/07/27 08:22:35  meho
Added pluggable LTE/Testcard test item.

Revision 1.1.2.5  2018/07/19 09:27:37  alpeng
1. Moving IR3570 chips to CPU I2C bus. 2. Removed related code of i2c devices which are not support on Curie; max1617, IDT8T49N287I(sys clk), poe psu, 30w poe

Revision 1.1.2.4  2018/07/12 09:46:51  alpeng
add mb and FPGA board type for curie 1RU; max sm and nim slots; clean up mb_test

Revision 1.1.2.3  2018/07/10 07:27:23  alpeng
add is_radium() and is_polonium(); remove pcie switch files from Makefile

Revision 1.1.2.2  2018/07/09 10:00:14  alpeng
remove pcie switch bus num init from linux_main.c and add nvme menu on mb_test.c

Revision 1.1.2.1  2018/06/22 08:05:18  alpeng
move curie diag to neptune/curie_1RU directory

Revision 1.1.2.1  2018/05/30 02:39:36  alpeng
porting neptune x86 to curie

$Endlog$
*/
