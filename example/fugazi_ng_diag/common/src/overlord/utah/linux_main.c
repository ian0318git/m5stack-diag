/* $Id: linux_main.c,v 1.63 2019/09/11 07:18:15 alpeng Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/overlord/utah/linux_main.c,v $
 *------------------------------------------------------------------
 * by: mcharon
 * 5/2008
 *
 * Copyright (c) 2016-2019 by Cisco Systems, Inc.
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
#include "platform_cookie.h"
#include "act2_utils.h"
#include "cross_platform.h"
#include "common.h"
#include "linux_usb_test.h"

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

static uint32_t plat_ngio_bus_num = 0xffff;

volatile unsigned char hkeepflags;
jmp_buf monjmpbuf, *monjmpptr;

unsigned int brd_ver = 0;

extern void set_rtc_dev(int);
extern uint32 show_temperature_all(void);
extern void show_fan_info(void);
extern int  utility_get_rtc(int);
extern int dash_set_map(int);
extern uint32_t ovld_check_poe_psu_wrap(void);
extern int max1617a_read_remote_temp(uint8_t *);
extern int ovld_check_system_pressure(void);
extern int ttf2array(int size, const char *file, unsigned char *fpga);
extern void fixup_30wpoe_addr(void);
static void cleanup_before_exit(void);
static int getfirmware(char *file);
static uint ovld_open_module(int *, const char *);
static int ovld_get_info(char*, char**, int, char*);
static int ovld_show_cpuinfo(void);
static int ovld_show_meminfo(void);
static int ovld_show_fpga_and_mb_info(void);
static int utah_processor_check(void);
//static int ovld_check_pcie_lanes(void);
static int ovld_quack_chip_reset(void);

extern unsigned char swapbyte(unsigned char c);

unsigned int dash_fpga_fw_size = 0;
unsigned char *dash_fpga_fw_array = NULL;
unsigned long dash_msg = 0;
unsigned long dash_fpga = 0;
unsigned long dash_cpld = 0;
extern char *optarg;

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

static ovld_dev_info_t ovld_dev_table[] ={ 
    {"30wpoe",      POE_30W_MASK},
    {"usb0",        USB0_MASK},
    {"usb1",        USB1_MASK},
    {"aux",         AUX_EXT_MASK},
    {"msata",       MSATA_MASK},
    {"eusb",        EUSB_MASK},
    {"dimm1",       DIMM_1_MASK},
    {"\0",          0},
};

#define OVLD_DEV_TABLE_SIZE \
      (sizeof(ovld_dev_table) / sizeof(ovld_dev_info_t))


extern void diag_menu (int argc, const char *argv[]);

/*****************************************************************************
 *
 * Function   : force_skip_dimm1
 * Description: return flag which keeps track of user intention: wheather or
 *              not user wants to force software to skip testing dimm1
 * Inputs     : none
 * Outputs    : TRUE, if software should skip testing 30WPOE. FALSE, otherwise
 *
 *****************************************************************************/
int
force_skip_dimm1 (void)
{
    if (skip_test & DIMM_1_MASK) {
        return(TRUE);
    }
    return (FALSE);
}

/*****************************************************************************
 *
 * Function   : force_skip_30wpoe
 * Description: return flag which keeps track of user intention: whether or
 *              not user wants to force software to skip testing 30wPOE
 * Inputs     : none
 * Outputs    : TRUE, if software should skip testing 30WPOE. FALSE, otherwise
 *
 *****************************************************************************/
int
force_skip_30wpoe (void)
{
    if (skip_test & POE_30W_MASK) {
        return(TRUE);
    }
    return(FALSE);
}

/*****************************************************************************
 *
 * Function   : force_skip_usb
 * Description: return flag which keeps track of user intention: whether or
 *              not user wants to force software to skip testing usb0 or usb1
 * Inputs     : id: usb index (0: for usb0, 1: for usb1)
 * Outputs    : TRUE, if software should skip testing the usb. FALSE, otherwise
 *
 *****************************************************************************/
int
force_skip_usb (int id)
{
    if ((id == 0) && (skip_test & USB0_MASK)) {
        return(TRUE);
    } else if ((id == 1) && (skip_test & USB1_MASK)) {
        return(TRUE);
    }
    return(FALSE);
}

/*****************************************************************************
 *
 * Function   : force_skip_ext_aux
 * Description: return flag which keeps track of user intention: whether or
 *              not user wants to force software to skip testing aux loopback
 * Inputs     : NONE
 * Outputs    : TRUE, if software should skip testing aux. FALSE, otherwise
 *
 *****************************************************************************/
int
force_skip_ext_aux (void)
{
    if (skip_test & AUX_EXT_MASK) {
        return(TRUE);
    }
    return(FALSE);
}

/*****************************************************************************
 *
 * Function   : force_skip_eusb
 * Description: return flag which keeps track of user intention: whether or
 *              not user wants to force software to skip testing eusb
 * Inputs     : NONE
 * Outputs    : TRUE, if software should skip testing eusb. FALSE, otherwise
 *
 *****************************************************************************/
int
force_skip_eusb (void)
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
int
force_skip_msata (void)
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
 * Function   : cleanup_before_exit
 * Description: callback gets executed before program terminates. so far does
 *             nothing.
 * Inputs     : NONE
 * Outputs    : NONE
 *
 *****************************************************************************/
static void
cleanup_before_exit (void)
{
    
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
        //        strcat(cmd, argv[ix]);
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
    if (ovld_open_module(&fd_cpld, OVLD_CPLD_KLM)==PASSED) {

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
 * Function   : is_cmd_need_switch
 * Description: check of switch initialization is required when command line
 *              is invoked
 * Inputs     : argv -- comman line arguments
 *              
 * Outputs    : TRUE if switch initialization is needed
 *
 *****************************************************************************/
static boolean
is_cmd_need_switch (const char *argv[])
{
   int result = 0;
   char cli_test[8]= "test";

   if (result == strcmp(argv[1], cli_test))
       return(TRUE);

   return(FALSE);
}

/*******************************************************************************
 *
 * Function   :	pcie_get_ngio_bus_bus_number
 * Description:	Function to get system PCIe bus number for NGIO slots
 * Inputs     :	void
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
int pcie_get_ngio_bus_number (uint32_t *bus_num)
{
    FILE *fp;
    char *fname = "/tmp/lspci_file1";
    char *pcie_sw_vid = "111d:8090";
    char pcie_sw_vid_sword[] = "10b5:8617";
    char pcie_sw_vid_utah_plx[] = "10b5:8618";
    char pcie_sw_vid_dagger[] = "10b5:8604";
    char buf[128], cmd[32];

    /* Assign different vender ID and device ID for Utah/Sword/Dagger  */
    if (is_sword()) {
        pcie_sw_vid = pcie_sw_vid_sword;
    } else if (is_utah_plx()) {
        pcie_sw_vid = pcie_sw_vid_utah_plx;
    } else if (is_dagger()) {
        pcie_sw_vid = pcie_sw_vid_dagger;
    }    
    
    fp = fopen(fname, "r+");
    if (fp == NULL) {
        sprintf(cmd, "lspci -d %s > %s;", pcie_sw_vid, fname);
	system(cmd);
	msleep(1);
	fp = fopen(fname, "r+");
	if (fp == NULL) {
	    printf("Failed to create %s\n", fname);
	    return(FAILED);
	}
    }

    if (is_sword() || is_utah_plx() || is_dagger()) {
        fscanf(fp, "%2x", bus_num); 
    } else {
        /* Skip the 1st line and read the bus number from the 2nd line
         */
        fgets(buf, sizeof(buf), fp);
        fscanf(fp, "%2x", bus_num); 
    }

    fclose(fp);

    return (PASSED);
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
    return(plat_ngio_bus_num);
}


/*****************************************************************************
 *
 * Function   : ovld_open_module
 * Description: Wrap function to open module
 * Inputs     : The file descriptor pointer of module and module_type
 * Outputs    : PASSED/FAILED
 *
 *****************************************************************************/
static uint
ovld_open_module (int *fd, const char *name)
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
 * Function   : ovld_show_cpuinfo
 * Description: To show Overlord CPU Info by reading file "/proc/cpuinfo".
 * Inputs     : None
 * Outputs    : PASSED/FAILED
 *
 *****************************************************************************/
static int
ovld_show_cpuinfo (void)
{
    int rc = FAILED;
    char cpu_info[OVLD_INFO_BUF_SIZE];
    FILE  *fp;
 
    fp = fopen("/ovld_cpuinfo.txt", "r");
    if (fp == NULL) {
        rc = ovld_get_info("/proc/cpuinfo", ovld_cpu_info,
                           size_of_ovld_cpu_info, "/ovld_cpuinfo.txt");
        if (rc != PASSED) {
            printf("Failed to get Overlord CPU Info !!!\n");
            return (rc);
        } else {
            fp = fopen("/ovld_cpuinfo.txt", "r");
            if (fp == NULL) {
                printf("Failed to open /ovld_cpuinfo.txt");
                return (FAILED);
            }
        }
    }

    printf("CPU info after inits:\n");
    while (fgets(cpu_info, OVLD_INFO_BUF_SIZE, fp) != NULL) {
        printf("%s", cpu_info);
    }
    printf("\n");

    fclose(fp);

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

    printf("This is %s FPGA,\n", (fpga_ver & FPGA_REV_TYPE) ? "GX50" : "GX30");
    printf("FPGA rev = %#x; Board ID = %#x.\n", fpga_ver, fpga_brd);
    if (is_goldbeach() || is_vg400()) { 
        printf("Board Revision = %#x\n", brd_ver);
        /* Display Secure JTAG status */
        if (get_secure_jtag_status() == TRUE) {
            printf("The Secure JTAG is Functioning.\n");
        } else {
            printf("The Secure JTAG is not Functioning.\n");
        }
    }
    return (PASSED);
}

/*******************************************************************************
 *
 * Function   : utah_processor_check
 * Description: To check if Linux detect all processors.
 * Inputs     : None
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
static int
utah_processor_check (void)
{
    char file_path_redhat[OVLD_INFO_BUF_SIZE];
    char file_path_fedora[OVLD_INFO_BUF_SIZE];
    uint  ctr = 0, cpu_num = 0, rc = PASSED;
    size_t size = 0;
    int cpu_core_num;

    cpu_core_num = is_dg_machines() || is_vg400() ? DAGGER_PROCESSOR_NUM : OVLD_PROCESSOR_NUM;

    for (ctr = 0; ctr < cpu_core_num; ctr++) {
        sprintf(file_path_redhat, "/dev/cpu%d", ctr);
        sprintf(file_path_fedora, "/dev/cpu/%d/cpuid", ctr);
	/* pfix-temp. We are in the middle of switching to fedora
	 * rootfs so this check need to work for both redhat and
	 * fedora for now.
	 */
	if (file_exist(file_path_redhat, &size) ||
            file_exist(file_path_fedora, &size)) {
            cpu_num++;
        } else {
            cterr('f', 0, "Processor %d is not present", ctr);
            rc = FAILED;
        }
    }
    printf("\nTotal detected processor number: %d.\n", cpu_num);

    return (rc);
}

/*******************************************************************************
 *
 * Function   : ovld_show_pcieinfo
 * Description: Function to check the PCIe lanes and show the result
 * Inputs     : None
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
#if 0
static int
ovld_check_pcie_lanes (void)
{
    system("sh /overlord/bin/pcie_lane.sh");
    char pcie_info[OVLD_INFO_BUF_SIZE];
    char pcie_info1[OVLD_INFO_BUF_SIZE];
    char pcie_info2[OVLD_INFO_BUF_SIZE];
    char buffer[OVLD_INFO_BUF_SIZE];
    FILE  *fp;

    fp = fopen("/ovld_pcie_lane_err.txt", "r");
    if (fp == NULL) {

        printf("Failed to open /ovld_pcie_lane_err.txt");
        return (FAILED);

    }

    while (fgets(pcie_info, OVLD_INFO_BUF_SIZE, fp) != NULL) {
        fgets(pcie_info1, OVLD_INFO_BUF_SIZE, fp);
        fgets(pcie_info2, OVLD_INFO_BUF_SIZE, fp);
        pcie_info[strlen(pcie_info)-1] = '\0';
        pcie_info1[strlen(pcie_info1)-1] = '\0';
        pcie_info2[strlen(pcie_info2)-1] = '\0';
        sprintf(buffer, "%s; detected %s lane, expected %s lanes",pcie_info,pcie_info1,pcie_info2);
        cterr('w', 0, buffer);

    }
    printf("\n");

    fclose(fp);

    return (PASSED);
}
#endif

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
    size_t size = 0;

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
 *              cli or if user just wants to launch o2x8_lnx iwthout
 *              initializing the switch, or if user wants to download.
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

/*******************************************************************************
 *
 * Function   : generic_ovld_show_pcieinfo
 * Description: Function to check the PCIe lanes and show the result
 *              corresponding to the script "generic_pcie_lane.sh"
 * Inputs     : None
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
int
generic_ovld_show_pcieinfo (void)
{
    system("sh /overlord/bin/generic_pcie_lane.sh");
    int plat_bus_index;
    char pcie_info[OVLD_INFO_BUF_SIZE];
    char buffer[OVLD_INFO_BUF_SIZE];
    FILE  *fp;

    char gb_pcie_bus_num[][8] = {"00:01.0","01:00.0"};
    char gb_device_id[][10] = {"8086:1f10", "1137:0130"};
    char gb_pcie_lanes[][4] = {"x1", "x1"};
    char pcie_bus_num[][8] = {"00:01.0", "00:02.0", "00:03.0", "00:04.0",
                              "01:00.0", "02:00.0", "03:00.0", "04:00.0"};

    char device_id[][10] = {"8086:1f10", "8086:1f11", "8086:1f12", "8086:1f13",
                             "1137:0130", "14e4:b321", "8086:1539", "111d:8090"};

    char pcie_lanes[][4] = {"x4", "x1", "x1", "x4",
                            "x4", "x1", "x1", "x4"};
    int bus_index = 0;
    int error = 0;
    int plx_exist = 0;

    /* Assign vender id, device id of PLX PCIe switch for corresponding platforms */
    if (is_utah_plx()) {
        strcpy(device_id[7],"10b5:8618");
    } else if (is_sword()) {
        strcpy(device_id[7],"10b5:8617");
    } else if (is_dagger()) {
        strcpy(device_id[7],"10b5:8604");
        /* the lane width of the port of PLX8604 connected to CPU is x2 for Dagger */
        strcpy(pcie_lanes[3],"x2");
        strcpy(pcie_lanes[7],"x2");
    } else if (is_goldbeach() || is_vg400()) {
        memcpy(pcie_bus_num, gb_pcie_bus_num, sizeof(gb_pcie_bus_num));
        memcpy(device_id, gb_device_id, sizeof(gb_device_id));
        memcpy(pcie_lanes, gb_pcie_lanes, sizeof(gb_pcie_lanes));
    }

    fp = fopen("/ovld_pcie_lane_err.txt", "r");
    if (fp == NULL) {

        printf("Failed to open /ovld_pcie_lane_err.txt");
        return (FAILED);

    }

    /* scan the text file ovld_pcie_lane_err.txt
     * to check the pcie lanes number
     */
    while (fscanf(fp, "%s", pcie_info) != EOF) {
       if (strcmp(pcie_info, "PLX") == 0) {
            plx_exist = 1;
       }
       if (strcmp(pcie_info,pcie_bus_num[bus_index]) == 0) {
            do {
                fscanf(fp, "%s", pcie_info);
            }while ( strcmp(pcie_info, "Width") != 0 );

            fscanf(fp, "%s", pcie_info);
            pcie_info[strlen(pcie_info)-1] = '\0';

            if (strcmp(pcie_info,pcie_lanes[bus_index]) != 0) {
                sprintf(buffer, "For bus number %s; detected %s lanes, expected %s lanes.",
                    pcie_bus_num[bus_index],pcie_info+1,pcie_lanes[bus_index]+1);
            cterr('w', 0, buffer);
            error++ ;
            }

        bus_index++ ;
        }
    }

    fclose(fp);
    if (is_goldbeach() || is_vg400()) { 
        plat_bus_index = 2;
    } else {
        plat_bus_index = 8;
    }

    if (bus_index == 0) {
        printf("PCIe lanes scan did not happen.\n");
	    error++;
    } else if ( (bus_index > 0) && (bus_index < plat_bus_index) ) {
        if (plx_exist == 0) {
            printf("PLX PCIe switch is not detected!!\n");
        } else {
            printf("PCIe lanes scan is not complete due to bus number changed.\n");
            printf("The original bus number of the failing PCIe link is %s\n",pcie_bus_num[bus_index]);    
            printf("The vender ID and the device ID of the failing link is %s.\n",device_id[bus_index]);
        }
	    error++;
    }

    if (error == 0) {
        printf("PCIe lanes scan passed!!\n");
    }
    else {
        return (FAILED);
    }

    return (PASSED);
}

/*****************************************************************************
 *
 * Function   : main
 * Description: entry point of overlord 
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
    ovld_dev_info_t *dev_ptr;
    char *usr_arg;
    char fpga_file[80];
    uint rc = FAILED;
    boolean en_switch = 1;
    int set_by_usr = 0, sku_num;

    fpga_file[0] = '\0';
    skip_test = opt_ch = 0;
    sh.cmdptr = NULL;

    /* adjust the /proc/sys/vm/overcommit_ratio to 80%, to avoid the oom-killer.
     * On some systems in MFG, after the ACT2 programming process,
     * the act2 process will use more memory, 90% (default) is not stable. 
     */
    system("echo 2 > /proc/sys/vm/overcommit_memory");
    system("echo 80 > /proc/sys/vm/overcommit_ratio");

    /* Add the work around suggested by Hardware engineer to deal with 
     * SMBus hang issue, once the ROMMOM is updated to solve this issue,
     * this work around should be taken off.
     */
    pci_config_write(0,0x1f,0x03,4,3);

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

                /* check if CLI requires GE switch. if not, we won't init
                   GE switch */
                en_switch = is_cmd_need_switch((const char **)argv);
            }
        } else {
            while ((opt_ch = getopt(argc, (char **)argv, ":d:qs:f:")) >= 0) {
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
                case 'd':
                    dev_ptr = (ovld_dev_info_t *)ovld_dev_table;
                    /* special control device option */
                    while ( *dev_ptr->name != '\0') {
                        if (strncmp(dev_ptr->name, optarg,
                                    strlen(dev_ptr->name)) == 0 ) {
                            /* string name are the same */
                            usr_arg = strchr(optarg, '=');
                            set_by_usr = 1;
                            if (atoi(usr_arg + 1)==0) { /* skipped '=' */
                                skip_test |= dev_ptr->mask;
                            }
                            
                            /* show fatal error, so that manufacturing
                             * won't skip the following test items */
                            if ((strncmp("msata", optarg, 5) == 0) && 
                                    (skip_test & MSATA_MASK)) {
                                 cterr('f',0,"Skip test option is enabled, MSATA test will be skipped!!!\n");
                            }
                            if ((strncmp("eusb", optarg, 4) == 0) &&
                                    (skip_test & EUSB_MASK)) {
                                 cterr('f',0,"Skip test option is enabled, eUSB test will be skipped!!!\n");
                            }

                        }
			dev_ptr++;
                    } /* while */

                    /* check if user specified the right device name */
                    if (!set_by_usr) {
                        goto usage;
                    }
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

    atexit(cleanup_before_exit);
    open_cpld();
    init_slot_info();
    

    /* Open modules */
    if (ovld_open_module(&fd_i2c0, OVLD_I2C_KLM) != PASSED) {
        cterr('f', 0, "%s: Failed to open /dev/%s", __FUNCTION__, OVLD_I2C_KLM);
    }
    if (ovld_open_module(&fd_vtop, OVLD_VTOP_KLM) != PASSED) {
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
    if (is_goldbeach() || is_vg400()) { 
        /* Install SPI Boot Flash Driver */
        system("modprobe ich9_spi");
    }
    if ((!is_goldbeach()) && (!is_vg400())) { 
        prepare_pcie_sw_info(&plat_ngio_bus_num);
        if (plat_ngio_bus_num == 0xFFFF) {
            cterr('f', 0, "unable to get plat ngio bus number = 0x%x",
                   plat_ngio_bus_num);
        }

        if (is_bcm_greyhound()) {
            printf("GE switch is Greyhound (BCM53403, BCM53404)\n");
        } else {
            printf("GE switch is Helix (BCM56321L)\n");
        }
    }
    if (en_switch && !quick_launch) {
        /* Initialize the Broadcom switch. If failed, let the menu
	     * come up for debugging.
	    */
        if ((!is_goldbeach()) && (!is_vg400())) { 
            if(bcm_gesw_config() < 0) {
                cterr('f', 0, "Broadcom GE switch init failed\n");
                exit(-1);
            } else {
                ctrl_plane_sgmii_macsa_declare();
            }    
        }
	/* if CLI, don't need to show system info , but we
	   still want to do some system checking. */
        utah_processor_check();
        set_nios_mode(NIOS_DISABLE_MODE);

	if (!sh.cmdptr) {
	    ovld_show_meminfo();
	    ovld_show_cpuinfo(); 
	    ovld_show_fpga_and_mb_info(); 

	    /* get_plat_sku(); */ 
            chk_plat_sku(&sku_num);

	    printf("Temperature Info:\n");
	    show_temperature_all();
            show_fan_info();
	}

	ovld_check_poe_psu_wrap(); 
	printf("\n");
	utility_get_rtc(1);

	/* Check system pressure
	 * (Overlord threshold = 70Kpa)
	 */
        ovld_check_system_pressure();

	printf("\n");

	if (generic_ovld_show_pcieinfo() == FAILED) {
            cterr('w',0,"PCIe lanes scan failed!!\n");
        }
    }

    /* Turn off memory malloc overcommit to avoid oom in memory test
     */
    system("echo 2 > /proc/sys/vm/overcommit_memory");

    /*Check for eUSB and eMMC and print size */
    printf("\n");
    check_block_size("/dev/eUSB");
    check_block_size("/dev/emmc0");
	
    if (sh.cmdptr != NULL) {
        shcmdline(&sh);
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
 
    set_nios_mode(NIOS_NORMAL_MODE);
    return 0;

 usage:
    printf("Usage: ./utah_lnx [-d 30wpoe=num] [-d usb0=num] [-d usb1=num] [-d aux=num] [-d eusb=num] [-d msata=num] [-f dash_fpga_file] "
           "[-s fpga_size]\n");
    printf("                  [-q]\n");
    printf("       -d 30wpoe=num: force software to bypass 30Wpoe if num=0\n");
    printf("       -d usb0=num: force software to bypass usb0 if num=0\n");
    printf("       -d usb1=num: force software to bypass usb1 if num=0\n");
    printf("       -d aux=num: force software to do aux internal loopback if num=0\n");
    printf("       -d msata=num: force software to bypass msata if num=0\n");
    printf("       -d eusb=num: force software to bypass eusb if num=0\n");
    printf("       -f filename: load FPGA\n");
    printf("       -q: launch utah_lnx without initialzing Ge Switch\n");
    printf("       -s size: specify  FPGA size. if not specified, use default\n");
    exit(0);
}

/*-------------------------------------------------
$Log: linux_main.c,v $
Revision 1.63  2019/09/11 07:18:15  alpeng
CSCvr18160 - adjust NIOS mode setup on Utah

Revision 1.62  2019/08/26 03:36:11  alpeng
CSCvq64781 - dimm1 is optional, provide dimm1 option arg for MFG

Revision 1.61  2018/08/30 06:59:43  haohsu
Collapse Vg400-branch to Main Trunk

Revision 1.60  2017/08/10 10:12:43  iachang
CSCvf44161: Merge Goldbeach into USD platform as one image

Revision 1.59  2016/10/16 12:28:22  iachang
Supported Goldbeach Platform.

Revision 1.58  2016/04/19 00:15:37  jskow
Add function to check for eUSB and emmc and display size in victory platforms.

Revision 1.57  2015/06/23 23:03:59  ptong
Tune Linux memory overcommit ratio to 80%, bump version to 6.7.1

Revision 1.56  2015/03/05 07:18:35  alpeng
fix is_plx issue

Revision 1.55  2014/11/27 08:56:37  danchung
Indicate whether the platform is greyhound of non-greyhound system

Revision 1.54  2014/08/18 22:15:14  yuetwang
add engineering banner

Revision 1.53  2014/08/11 10:37:52  danchung
1. in linux_main.c call cterr to print WARNING if pcie lane check fail
2. in motherboard test call cterr to print ERROR if pcie lane check fail

Revision 1.52  2014/08/07 11:11:09  danchung
add pcie lane check in motherboard test submenu

Revision 1.51  2014/05/31 00:42:23  mcharon
change bytes to values to make it clear

Revision 1.50  2014/05/31 00:30:19  mcharon
dynamically get fpga file size

Revision 1.49  2014/05/20 11:22:15  danchung
Improve the error display for PCIe lane scan

Revision 1.48  2014/05/02 21:55:00  ptong
Check for rc.soc and config.bcm

Revision 1.47  2014/05/02 03:38:39  alpeng
add 30w poe on dev_table for utah

Revision 1.46  2014/03/11 08:08:55  alpeng
supprot 30w poe for utah only

Revision 1.45  2014/03/05 02:23:14  hroni
USD machines does not have env mcu. Remove platform_mcu.c and platform_mcu.h and cleanup the related code

Revision 1.44  2014/02/26 10:25:33  alpeng
USD doesn't support 30w poe anymore; still keep the code for platform_cookie.c

Revision 1.43  2014/02/13 19:03:12  mcharon
support act2 authentication on sword

Revision 1.42  2014/02/13 11:27:06  hroni
fix skip aux external loop back

Revision 1.41  2014/02/13 08:50:21  hroni
fix typo

Revision 1.40  2014/02/13 08:49:11  hroni
1. fix skip eUSB/mSATA check. 2. show fatal error when eUSB/mSATA test is skipped

Revision 1.39  2014/02/10 07:13:31  hroni
add -d msata and -d eusb to exclude msata and eusb, correspondingly.

Revision 1.38  2014/01/23 09:18:45  danchung
Add the work around for handling the SMBbus hang issue

Revision 1.37  2014/01/22 22:13:27  ptong
Removed a printf statement

Revision 1.36  2014/01/15 00:09:28  mcharon
change dash_fw_size to 1282904

Revision 1.35  2014/01/14 02:44:20  hroni
support NIOS_DIAG_MODE. use NIOS_DIAG_MODE instead of NIOS_NORMAL_MODE

Revision 1.34  2014/01/08 07:56:09  hroni
use enable_nios() instead of reseting NIOS

Revision 1.33  2014/01/07 05:03:29  hroni
reset and unreset nios before and after reading temperature

Revision 1.32  2013/12/26 02:42:38  hroni
1. remove NIOS reset that were done during diag init.
2. put NIOS to reset during i2c scan test and i2c utility. unreset after scan test or utility is finished

Revision 1.31  2013/12/24 05:59:00  hroni
1. enhance aux_internal_loopback test debug message. 2. rename related parameters

Revision 1.30  2013/12/23 04:21:43  hroni
1. add force skip usb and aux. 2. add aux internal loopback test

Revision 1.29  2013/12/21 01:35:59  ptong
Dagger CPU only has 4 cores

Revision 1.28  2013/12/19 10:11:20  danchung
Fix pcie lane checking failure on Dagger

Revision 1.27  2013/12/18 00:24:40  mcharon
file_exist now returns size of file

Revision 1.26  2013/12/12 07:47:04  alpeng
show fan detail while init diag

Revision 1.25  2013/12/05 06:31:20  danchung
Fix pcie lane check failure due to new version rommon

Revision 1.24  2013/12/04 03:48:56  danchung
Add the printing of the failing PCIe link check due to bus number changed
by different rommon version.

Revision 1.23  2013/11/26 08:40:37  hroni
fix compiler warning

Revision 1.22  2013/11/20 12:33:49  danchung
Fix pcie switch utility error for Sword

Revision 1.21  2013/11/06 22:24:34  ptong
minor printf statement change.

Revision 1.20  2013/10/28 10:45:32  danchung
Support new  bus number for pcie lane scan

Revision 1.19  2013/10/05 00:39:38  ptong
Add set_board_type(), and ctrl_plane_sgmii_macsa_declare()

Revision 1.18  2013/09/09 05:50:36  ptong
Remove ctrl_plane_sgmii_macsa_declare

Revision 1.17  2013/09/06 22:56:20  ptong
Support Utah with ctrl_plane_sgmii_macsa_declare

Revision 1.16  2013/08/24 00:51:00  ptong
Minor change and clean up for 0.2.0 release

Revision 1.15  2013/08/19 01:53:19  alpeng
using both FPGA and MB cookie to get/check board type

Revision 1.14  2013/08/08 21:54:14  hroni
Turn on ovld_check_poe_psu_wrap(), ovld_check_system_pressure() during diag init.

Revision 1.13  2013/08/07 22:52:57  hroni
1. reset NIOS during diag init. 2. fix power sequencer utility

Revision 1.12  2013/08/05 23:45:01  ptong
Change ovld_processor_check to utah_processor_check

Revision 1.11  2013/07/24 17:31:11  hroni
during diag startup, show the temperature of bezel side and i/o side sensors

Revision 1.10  2013/07/18 17:17:03  mcharon
add -Wal and clean up compile warnings

Revision 1.9  2013/07/09 09:49:10  alpeng
moving function is_platform() related to dash_fpga.c

Revision 1.8  2013/07/04 01:57:18  ptong
Added generic_ovld_show_pcieinfo

Revision 1.7  2013/07/03 23:50:20  ptong
Modify for proper init and menu setup

Revision 1.6  2013/06/28 13:15:43  danchung
Add pcie lane check for Utah.

Revision 1.5  2013/06/26 20:33:26  ptong
Utah P1A bring-up

Revision 1.4  2013/05/21 02:27:53  hroni
sync with x86 update

Revision 1.3  2013/05/14 03:09:39  hroni
fix compile error

Revision 1.2  2013/05/09 07:37:15  alpeng
updating files

Revision 1.1  2013/05/09 05:52:59  alpeng
add utah tree

Revision 1.45  2013/05/01 20:43:23  mcharon
put overdrive in Gen1 in controller mode

Revision 1.44  2013/04/23 17:16:16  mcharon
remoe i2c_psu_retry

Revision 1.43  2013/04/15 21:14:21  mcharon
remove i2c retry

Revision 1.42  2013/03/27 20:26:30  mcharon
force idt switch to notify hotplug

Revision 1.41  2013/03/25 19:20:23  mcharon
allow a way to bypass resetting NIOS

Revision 1.39  2013/03/22 22:29:45  mcharon
do not exit diag when NIOS_RESET failure is detected

Revision 1.38  2013/03/22 18:21:57  mcharon
support CLI for voltage margining

Revision 1.37  2013/03/17 02:04:13  mcharon
support command line for testing 30w poe

Revision 1.36  2013/03/14 18:18:46  mcharon
check reset bit after putting nios into reset before proceeding

Revision 1.35  2013/03/11 12:43:31  danchung
Add function to check the PCIe lanes corresponding to the script "generic_pcie_lane.sh"

Revision 1.34  2013/02/23 07:31:56  ptong
Fixed the problem due to new ROMMON changed the PCIe bus numbering and bumped diag version to 6.3

Revision 1.33  2013/02/13 23:57:00  mcharon
check for version file in bzImage.SSA.bin

Revision 1.32  2013/01/20 17:41:58  palin2
Updated Overlord Diag specific reset Quack chip(ACT2) process.

Revision 1.31  2013/01/15 02:25:14  palin2
Reset Quack chip(ACT2) by FPGA right after Diag is up to
ensure Quack chip in a valid state.

Revision 1.30  2013/01/15 01:53:52  ptong
Support HW CDET CSCud23263: IDT PCIe PRBS loopback test can only be run once after each power up

Revision 1.29  2012/12/21 01:28:30  palin2
1.Add support for Overlord Diag RDT version.
2.Add utility to dump specific PCIe port's all registers for debugging purpose.

Revision 1.28  2012/11/17 01:14:12  mcharon
don't extern dash_fpga_fw_size

Revision 1.27  2012/11/06 20:39:50  mcharon
add headers/cleanup/remove unneeded functions/files

Revision 1.26  2012/10/24 10:16:52  danchung
To prevent printing PCIe lanes scan message in CLI command mode.

Revision 1.25  2012/10/02 09:24:28  alpeng
adding cli_argv++ on case of CLI cmd

Revision 1.24  2012/09/27 22:04:04  mcharon
don't bundle fpga into image; support dynamic fpga download

Revision 1.23  2012/09/25 03:27:54  palin2
Move function "ovld_check_system_pressure" to
right place with other system info check functions.

Revision 1.22  2012/09/18 07:47:32  palin2
Add function to check system pressure in Diag boot-up process.

Revision 1.21  2012/09/11 21:00:46  mcharon
need to support pid string with lenght greater than 16

Revision 1.20  2012/09/05 15:25:39  palin2
1. Use cterr to replace printf("***") when failed to open module.
2. Remove check module since we don't need them now.

Revision 1.19  2012/09/05 10:20:27  danchung
Add PCIe lanes check funtion.

Revision 1.18  2012/08/22 02:28:41  palin2
Add Cavium CPU temperature display in Overlord Diag boot-up message.

Revision 1.17  2012/08/18 00:01:13  ptong
Use official SKU for PID checking

$Endlog$
*/
