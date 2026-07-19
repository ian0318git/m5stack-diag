/* $Id: linux_main.c,v 1.70 2019/08/26 03:36:11 alpeng Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/overlord/x86/linux_main.c,v $
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
extern int  show_temp(int, int);
extern int  utility_get_rtc(int);
extern int dash_set_map(int);
extern uint32_t ovld_check_poe_psu_wrap(void);
extern int max1617a_read_remote_temp(uint8_t *);
extern int ovld_check_system_pressure(void);
extern int ttf2array(int size, const char *file, unsigned char *fpga);
static void cleanup_before_exit(void);
static int getfirmware(char *file);
static uint ovld_open_module(int *, const char *);
static int ovld_get_info(char*, char**, int, char*);
static int ovld_show_cpuinfo(void);
static int ovld_show_meminfo(void);
static int ovld_show_fpga_and_mb_info(void);
static int ovld_processor_check(void);
static int generic_ovld_show_pcieinfo(void);
static int ovld_display_cavium_cpu_temp(void);
static int ovld_quack_chip_reset(void);

extern unsigned char swapbyte(unsigned char c);
extern void overdrive_fixup(void);

unsigned int dash_fpga_fw_size = 0;
unsigned char *dash_fpga_fw_array = NULL;

mb_iofpga_mcu_regs_t  *IOFPGA_MCU_REGS;

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
    {"30wpoe",      POE_30W_MASK,     },
    {"dimm1",       DIMM_1_MASK,      },
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
 * Description: return flag which keeps track of user intention: wheather or
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
    char pcie_sw_vid_juno_plx[] = "10b5:8618";
    char *expect_bus_assignment = "02:00.0";
    char buf[128], cmd[32];

    /* Assign vender and device ID for Juno with PLX PCIe swith  */
    if (is_juno_plx()) {
        pcie_sw_vid = pcie_sw_vid_juno_plx;
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

    if (is_juno_plx()) {
        fscanf(fp, "%2x", bus_num); 
    } else {
        fgets(buf, sizeof(buf), fp); // get line

        if (strncmp(buf, expect_bus_assignment, (int)strlen(expect_bus_assignment)) == 0) {
            /* This O2 has ROMMON v12.2 or later
    	     */
            *bus_num = NGIO_PCIE_BUS_NUM_FCS;
        }
        else {
            /* This O2 ROMMON before v12.2
	     */
            *bus_num = NGIO_PCIE_BUS_NUM_PREFCS;
        }
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

    return (PASSED);
}

/*******************************************************************************
 *
 * Function   : ovld_processor_check
 * Description: To check if Linux detect all processors.
 * Inputs     : None
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
static int
ovld_processor_check (void)
{
    char file_path_redhat[OVLD_INFO_BUF_SIZE];
    char file_path_fedora[OVLD_INFO_BUF_SIZE];
    uint  ctr = 0, cpu_num = 0, rc = PASSED;
    size_t size;

    for (ctr = 0; ctr < OVLD_PROCESSOR_NUM; ctr++) {
        sprintf(file_path_redhat, "/dev/cpu%d", ctr);
        sprintf(file_path_fedora, "/dev/cpu/%d/cpuid", ctr);
        /* pfix-temp. We are in the middle of switching to fedora
         * rootfs so this check need to work for both redhat and
         * fedora for now.
         */
        if (file_exist(file_path_redhat, &size) || file_exist(file_path_fedora, &size)) {
            cpu_num++;
        } else {
            cterr('f', 0, "Processor %d is not present", ctr);
            rc = FAILED;
        }
    }
    printf("\nTotal detected processor number: %d.\n", cpu_num);

    return (rc);

#if 0
    uchar file_path[OVLD_INFO_BUF_SIZE];
    FILE  *fp;
    uint  ctr = 0, cpu_num = 0, rc = PASSED;

    for (ctr = 0; ctr < OVLD_PROCESSOR_NUM; ctr++) {
        sprintf(file_path, "/dev/cpu%d", ctr);
        fp = fopen(file_path, "r");
        if (fp == NULL) {
            cterr('f', 0, "Processor %d is not present", ctr);
            rc = FAILED;
        } else {
            cpu_num++;
            fclose(fp);
        }
    }
    printf("\nTotal detected processor number: %d.\n", cpu_num);

    return (rc);
#endif 
}

/*******************************************************************************
 *
 * Function   : ovld_display_cavium_cpu_temp
 * Description: Function to display Cavium CPU temperature on Overlord
 *              by reading ENV sensor chip MAX1617a.
 * Inputs     : None
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
static int
ovld_display_cavium_cpu_temp (void)
{
    int     result = FAILED;
    uint8_t reg_val = 0;

    /* Get Cavium CPU temperature,
     * for Overlord, to read it from MAX1617a.
     */
    result = max1617a_read_remote_temp(&reg_val);

    printf("Current Cavium CPU Temperature is ");
    if (result != PASSED) {
        printf("N/A.\n");
    } else {
        printf("%d degrees Celsius.\n", reg_val);
    }

    return (result);
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
static int
generic_ovld_show_pcieinfo (void)
{
    system("sh /overlord/bin/generic_pcie_lane.sh");

    char pcie_info[OVLD_INFO_BUF_SIZE];
    char buffer[OVLD_INFO_BUF_SIZE];
    FILE  *fp;

    /* For Overlord and Juno with IDT PCIe switch */
    char pcie_bus_num[][8] = {"00:01.0", "00:01.1", "00:06.0", "01:00.0",
                              "02:00.0", "03:00.0", "03:01.0", "04:00.0",
                              "05:00.0", "64:00.0", "64:00.1", "64:00.2",
                              "64:00.3", "64:00.4" };

    char pcie_lanes[][4] = {"x4", "x4", "x4", "x4",
                            "x4", "x1", "x1", "x1",
                            "x1", "x4", "x4", "x4",
                            "x4", "x4" };

    /* For Juno with PLX PCIe switch and the bus numbers arrangement is based
     * on rommon version 15.4(2r)S . Once the rommon is changed, the lanes 
     * number check may be failed.
     */
    char plx_pcie_bus_num[][8] = {"00:01.0", "00:01.1", "00:01.2", "00:06.0",
                                  "01:00.0", "02:00.0", "03:0f.0", "61:00.0",
                                  "62:00.0", "63:00.0", "63:00.1", "63:00.2",
                                  "63:00.3", "63:00.4" };

    char plx_pcie_lanes[][4] = {"x4", "x4", "x1", "x4",
                                "x4", "x4", "x1", "x1",
                                "x1", "x4", "x4", "x4",
                                "x4", "x4" };

    int bus_index = 0, ix;
    int error = 0;
    char *p_bus_num[15];
    char *p_lanes[15];

    fp = fopen("/ovld_pcie_lane_err.txt", "r");
    if (fp == NULL) {

        printf("Failed to open /ovld_pcie_lane_err.txt");
        return (FAILED);

    }
    
    /* check if the system uses PLX PCIe switch
     * if yes, the set of bus and lanes number for 
     * Juno with PLX PCIe switch should be used
     */
    if (is_plx_wrapper()) {
        for (ix=0;ix<=14;ix++){
            p_bus_num[ix] = plx_pcie_bus_num[ix];
            p_lanes[ix] = plx_pcie_lanes[ix];
        } 
    } else {
        for (ix=0;ix<=14;ix++){
            p_bus_num[ix] = pcie_bus_num[ix];
            p_lanes[ix] = pcie_lanes[ix];
        } 
    }


    /* scan the text file ovld_pcie_lane_err.txt
     * to check the pcie lanes number
     */
    while (fscanf(fp, "%s", pcie_info) != EOF) {
        if (strcmp(pcie_info,p_bus_num[bus_index]) == 0) {
            do {
                fscanf(fp, "%s", pcie_info);
            }while ( strcmp(pcie_info, "Width") != 0 );

            fscanf(fp, "%s", pcie_info);
            pcie_info[strlen(pcie_info)-1] = '\0';

            if (strcmp(pcie_info,p_lanes[bus_index]) != 0) {
                sprintf(buffer, "For bus number %s; detected %s lanes, expected %s lanes.",
                    p_bus_num[bus_index],pcie_info+1,p_lanes[bus_index]+1);
                cterr('w', 0, buffer);
                error++ ;
            }

        bus_index++ ;
        }
    }

    fclose(fp);

    if (bus_index == 0) {
        printf("PCIe lanes scan did not happen.\n");
	error++;
    } else if ( (bus_index > 0) && (bus_index < 14) ) {
        printf("PCIe lanes scan is not complete.\n");
        printf("Please check the ROMMON version is 15.4(2r)S or not.\n");
	error++;
    }

    if (error == 0) {
        printf("PCIe lanes scan passed!!\n");
    }
    else {
        cterr('f',0,"PCIe lanes scan failed!!\n");
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
    int set_by_usr = 0;
    int sku_num;
    size_t fsize;

    /* adjust the free mem ratio to 80%, to bypass the oom-killer. 
     * the act2 process will use more memory, 90% (default) is not stable. 
     */
    system("echo 2 > /proc/sys/vm/overcommit_memory");
    system("echo 80 > /proc/sys/vm/overcommit_ratio");

    if (!file_exist("/overlord/x86/version_5.9b", &fsize)) {
        cterr('f', 0, "\n%s requires bzImage.SSA.bin built on "
              "Feb-13-2013 or later.\n\n", argv[0]);
        exit(0);
    }
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

                            break; /* while dev_ptr->name */
                        } else {
    
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

    /* Put NIOS into reset stat,
     * and need to delay 1 second based on HW's suggestion.
     */
    if (access("./skip_nios_reset", F_OK ) == 0 ) {
        printf("!!!skipping NIOS RESET (NIOS will not be in reset.)!!\n");
    } else {
        if (reset_nios(FPGA_IN_NIOS_RST, 0)==FAILED) {
            cterr('f', 0, "unable to reset NIOS");
        }
    }

    /* Reset Quack chip */
    ovld_quack_chip_reset();

    set_board_type();
    prepare_pcie_sw_info(&plat_ngio_bus_num);
    if (plat_ngio_bus_num == 0xFFFF) {
        cterr('f', 0, "unable to get plat ngio bus number = 0x%x",
               plat_ngio_bus_num);
    }
    

    if (en_switch && !quick_launch) {
            /* Initialize the Broadcom switch. If failed, let the menu
             * come up for debugging.
             */
            if(bcm_gesw_config() < 0) {
                cterr('f', 0, "Broadcom GE switch init failed\n");
		exit(-1);
            } else {
                cavecreek_sgmii_macsa_declare();
            }
            
            /* if CLI, don't need to show system info , but we
             still want to do some system checking. */

            ovld_processor_check();
            
            if (!sh.cmdptr) {
                ovld_show_cpuinfo();
                ovld_show_meminfo();
                ovld_show_fpga_and_mb_info();
                chk_plat_sku(&sku_num);
                /* get_plat_sku(); */
                ovld_display_cavium_cpu_temp();
                show_temp(0, 0);
                if (is_juno()) {
                    display_pem_fan_spd();
                }
            }

            ovld_check_poe_psu_wrap();

            printf("\n");
            utility_get_rtc(1);

            /* Check system pressure
             * (Overlord threshold = 70Kpa)
             */
            ovld_check_system_pressure();
            printf("\n");

            /*  check pcie lanes number 
             */
            generic_ovld_show_pcieinfo ();
    }


    /* Turn off memory malloc overcommit to avoid oom in memory test
     */
    system("echo 2 > /proc/sys/vm/overcommit_memory");

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

        //overdrive_fixup();
        /* overlord workaround for overdrive issue */
        if (!is_juno_plx()) {
            system("setpci -s 2:0 ff8.l=3f17c");
            system("setpci -s 2:0 ffc.l=14140a00");
            system("setpci -s 3:8 70.l=1");
            system("setpci -s 3:a 70.l=1");
            system("setpci -s 3:2 70.l=1");
        }
        diag_menu(1, argv); /* goto menu directly; dont' call monitor(); */

    }

    if (dash_fpga_fw_size)
        free(dash_fpga_fw_array);
    return 0;

 usage:
    printf("Usage: ./o2x86_lnx [-d 30wpoe=num] [-f dash_fpga_file] "
           "[-s fpga_size]\n");
    printf("                   [-q]\n");
    printf("       -d 30wpoe=num: force software to bypass 30Wpoe if num=0\n");
    printf("       -f filename: load FPGA\n");
    printf("       -q: launch o2x86 without initialzing Ge Switch\n");
    printf("       -s size: specify  FPGA size. if not specified, use default\n");

    exit(0);
}

/*-------------------------------------------------
$Log: linux_main.c,v $
Revision 1.70  2019/08/26 03:36:11  alpeng
CSCvq64781 - dimm1 is optional, provide dimm1 option arg for MFG

Revision 1.69  2015/03/05 07:18:35  alpeng
fix is_plx issue

Revision 1.68  2015/01/08 02:55:50  alpeng
adjust allocable free memory size to 80%, bump to v11.1.1

Revision 1.67  2014/08/18 22:15:14  yuetwang
add engineering banner

Revision 1.66  2014/06/13 22:03:29  mcharon
user doens't have to specify -s option for programming fpga when booting o2x86_lnx

Revision 1.65  2014/06/13 21:01:17  mcharon
fix fpga programming...get file size dynamically

Revision 1.64  2014/05/02 22:00:43  ptong
Check for rc.soc and config.bcm

Revision 1.63  2014/04/03 07:46:02  danchung
Fix pcie lanes check fail on Juno due to pcie bus number changed by rommon
version 15.4(2r)S

Revision 1.62  2014/02/18 05:55:55  alpeng
Juno PLX does not need work around on PCI

Revision 1.61  2013/12/24 05:57:15  hroni
rename skip_i2c_dev to skip_test

Revision 1.60  2013/12/18 02:56:56  hroni
fix compilation error

Revision 1.59  2013/11/26 09:41:02  danchung
Fix pcie switch utility error for Juno with PLX PCIe switch

Revision 1.58  2013/11/26 08:40:38  hroni
fix compiler warning

Revision 1.57  2013/11/19 07:07:44  danchung
Using new is_plx() for Juno

Revision 1.56  2013/11/05 12:14:52  danchung
Change the PCIe lane check mechanism to generic way

Revision 1.55  2013/09/11 02:25:08  alpeng
1. support Juno fan info and display on initialize stage.
2. support fedora rootfs

Revision 1.54  2013/08/21 22:40:17  mcharon
move overdrive fix up code to main.c..other platform will not need fix up code

Revision 1.53  2013/08/19 01:53:19  alpeng
using both FPGA and MB cookie to get/check board type

Revision 1.52  2013/07/10 01:34:54  alpeng
moving get_plat_sku() to platform_cookie.c. since the sku number coming from cookie.

Revision 1.51  2013/07/09 09:49:10  alpeng
moving function is_platform() related to dash_fpga.c

Revision 1.50  2013/07/04 08:02:19  alpeng
fixed is_overlord() for latest FPGA rev.

Revision 1.49  2013/06/24 17:35:58  mcharon
put back the code to force idt switch to notify hotplug--for overdrive

Revision 1.48  2013/05/31 12:51:04  danchung
Add checking board type for Juno.

Revision 1.47  2013/05/23 19:07:12  danchung
Only check pcie lanes on pilot and later machines.

Revision 1.46  2013/05/16 11:38:23  danchung
Add cavium eeprom check on intel side.

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
