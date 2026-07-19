/* $Id: linux_main.c,v 1.2 2019/06/14 05:24:49 mikech2 Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/katar/x86/linux_main.c,v $
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
#include "platform_fpga.h" 
#include "plat_defs.h"
#include "platform_cookie.h"
#include "act2_utils.h"
#include "cross_platform.h"
#include "common.h"
#include "linux_usb_test.h"

extern char *banner_string;
#define KATAR_I2C_0                 "i2c-0"
#define KATAR_I2C_1                 "i2c-1"
#define KATAR_VTOP_KLM              "addr_vtop"
#define KATAR_CPLD_KLM              "cpld"
#define KATAR_INFO_BUF_SIZE         256        
#define KATAR_CPU_INFO_FILE         "/katar_cpuinfo.txt"

#define CPLD_SIZE  0x1000
            
static char *katar_cpu_info[] = {
    "Processor", 
    "vendor_id",
    "model name",
    "stepping", 
    "microcode",
    "cpu MHz",
    "cache size",
    "bogomips",
};      
    
static const uint size_of_katar_cpu_info =
    sizeof(katar_cpu_info) / sizeof(uchar *);

static int fd_i2c0 = -1;
static int fd_i2c1 = -1;
static int fd_vtop = -1;
static int fd_cpld = -1;

static uint32_t plat_ngio_bus_num = 0xffff;

volatile unsigned char hkeepflags;
jmp_buf monjmpbuf, *monjmpptr;

extern void set_rtc_dev(int);
extern int dash_set_map(int);
static uint katar_open_module(int *, const char *);
static int linux_get_info(char*, char**, int, char*);

extern int katar_pcie_lane_scan_test(void);
extern void show_fan_sts(void);
extern int katar_set_fan_force_high_gpio(boolean bSetOn);

unsigned int dash_fpga_fw_size = 0;
unsigned char *dash_fpga_fw_array = NULL;

unsigned long dash_fpga = 0;
unsigned long dash_cpld = 0;
unsigned long dash_aikido = 0;
unsigned long dash_fpgai2c = 0;
unsigned long dash_io_reg = 0;
extern char *optarg;

static char *katar_mem_info[] = {
    "MemTotal",
    "MemFree",
    "Buffers",
    "Cached",
    "SwapCached",
    "Hugepagesize",
    "DirectMap4k",
    "DirectMap2M"
};

static const uchar size_of_katar_mem_info = \
                   sizeof(katar_mem_info) / sizeof(uchar *);

static unsigned int skip_test = 0;

extern void diag_menu (int argc, const char *argv[]);
extern int ExecuteCmdbyPopen(char *cmd, char *retBuf, int sizeOfBuf);

/*****************************************************************************
 *
 * Function   : get_i2c_fd
 * Description: return file descriptor for /dev/i2c0 or /dev/i2c1
 * Inputs     : i2c_bus
 * Outputs    : file desriptor for /dev/i2c0 or /dev/i2c1
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

void *
mmap_device (char *path, size_t size, off_t offset)
{
    void *ptr;
        int fd_mmap = -1;

    if (katar_open_module(&fd_mmap, path)==PASSED) {

        ptr = (void *)mmap(NULL, size, (PROT_READ | PROT_WRITE),
                           MAP_SHARED, fd_mmap, offset);
        if (ptr == MAP_FAILED) {
            close(fd_mmap);
            printf("not able to mmap to %p\n", ptr);
            perror("Error mmapping");
            return 0;
        }
    } else {
        /* we need this only for cpld uitility; so if we fail it's ok  */
        printf("can't open %s.... is driver loaded?\n", path);
        return 0;
    }
        if(fd_mmap != -1)
                close(fd_mmap);

    return (void *)(ptr);
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
    if (katar_open_module(&fd_cpld, KATAR_CPLD_KLM)==PASSED) {

        ptr = (void *)mmap(NULL, CPLD_SIZE, (PROT_READ | PROT_WRITE),
                           MAP_SHARED, fd_cpld, 0xFED40000);
        if (ptr == MAP_FAILED) {
            close(fd_cpld);
            perror("Error mmapping the file for CPLD (TPM space)");
            return (FAILED);
        }
        dash_fpga = (unsigned long)ptr;

        ptr = (void *)mmap(NULL, CPLD_SIZE, (PROT_READ | PROT_WRITE),
                           MAP_SHARED, fd_cpld, 0xFDFF4000);
        if (ptr == MAP_FAILED) {
            close(fd_cpld);
            perror("Error mmapping the file for CPLD (TPM space)");
            return (FAILED);
        }
        dash_cpld = (unsigned long)ptr;

        ptr = (void *)mmap(NULL, CPLD_SIZE, (PROT_READ | PROT_WRITE),
                           MAP_SHARED, fd_cpld, 0xFDFF8000);
        if (ptr == MAP_FAILED) {
            close(fd_cpld);
            perror("Error mmapping the file for Aikido");
            return (FAILED);
        }    
        dash_aikido = (unsigned long)ptr;

        ptr = (void *)mmap(NULL, CPLD_SIZE, (PROT_READ | PROT_WRITE),
                           MAP_SHARED, fd_cpld, 0xFDFF6000);
        if (ptr == MAP_FAILED) {
            close(fd_cpld);
            perror("Error mmapping the file for FPGA I2C");
            return (FAILED);
        }    
        dash_fpgai2c = (unsigned long)ptr;

		ptr = (void *)mmap(NULL, CPLD_SIZE, (PROT_READ | PROT_WRITE),
                           MAP_SHARED, fd_cpld, 0xFDFF5000);
        if (ptr == MAP_FAILED) {
            close(fd_cpld);
            perror("Error mmapping the file for CPLD (TPM space)");
            return (FAILED);
        }
        dash_io_reg = (unsigned long)ptr;

                close(fd_cpld);
                fd_cpld = -1;
    } else {
        /* we need this only for cpld uitility; so if we fail it's ok  */
        printf("*****can't open cpld mmap driver....*******\n");
        return -1;
    }
    return 0;
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
    return(plat_ngio_bus_num);
}


/*****************************************************************************
 *
 * Function   : katar_open_module
 * Description: Wrap function to open module
 * Inputs     : The file descriptor pointer of module and module_type
 * Outputs    : PASSED/FAILED
 *
 *****************************************************************************/
static uint
katar_open_module (int *fd, const char *name)
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
 * Function   : linux_get_info
 * Description: Get system Info by reading related linux file.
 * Inputs     : info_file - related Linux file to get info
 *              info_item - the item to collect
 *              info_item_size - size of items that needs to collect
 *              file_name - file to put the collected info
 * Outputs    : PASSED/FAILED
 *
 *****************************************************************************/
static int
linux_get_info (char* info_file, char** info_item, 
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
 * Function   : show_meminfo
 * Description: To show Overlord MEM Info by reading file "/proc/meminfo".
 * Inputs     : None
 * Outputs    : PASSED/FAILED
 *
 *****************************************************************************/
int
show_meminfo (void)
{
    int rc = FAILED;
    char mem_info[KATAR_INFO_BUF_SIZE];
    FILE  *fp;
 
    fp = fopen("/katar_meminfo.txt", "r");
    if (fp == NULL) {
        rc = linux_get_info("/proc/meminfo", katar_mem_info,
                           size_of_katar_mem_info, "/katar_meminfo.txt");
        if (rc != PASSED) {
            printf("Failed to get Overlord MEM Info !!!\n");
            return (rc);
        } else {
            fp = fopen("/katar_meminfo.txt", "r");
            if (fp == NULL) {
                printf("Failed to open /katar_meminfo.txt");
                return (FAILED);
            }
        }
    }

    printf("MEM info after inits:\n");
    while (fgets(mem_info, KATAR_INFO_BUF_SIZE, fp) != NULL) {
        printf("%s", mem_info);
    }
    printf("\n");

    fclose(fp);

    return (PASSED);
}

/*****************************************************************************
 *
 * Function   : katar_get_info
 * Description: Get KATAR system Info by reading related linux file.
 * Inputs     : info_file - related Linux file to get info
 *              info_item - the item to collect
 *              info_item_size - size of items that needs to collect
 *              file_name - file to put the collected info
 * Outputs    : PASSED/FAILED
 *
 *****************************************************************************/
static int katar_get_info(char *info_file, char **info_item,
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
 * Function   : katar_get_cpucore
 * Description: To show CPU core number by reading file "/proc/cpuinfo".
 * Inputs     : None
 * Outputs    : PASSED/FAILED
 *
 *****************************************************************************/
static int katar_get_cpucore( char *file_name )
{   
    char sys_cmd[256];
    char sys_cpucore[3];
       
    if ((ExecuteCmdbyPopen ("cat /proc/cpuinfo | grep processor | wc -l",
        sys_cpucore, KATAR_INFO_BUF_SIZE)) == 0) {
        cterr('f',0,"get CPU core failed!!\n");
        return (FAILED);
    }       
            
    sprintf(sys_cmd, "echo \"CPU cores\t: %d\" >> %s", atoi(sys_cpucore), file_name);
    system(sys_cmd);
            
    return (PASSED);
} 

/*****************************************************************************
 *
 * Function   : katar_show_cpuinfo
 * Description: To show SKY CPU Info by reading file "/proc/cpuinfo".
 * Inputs     : None
 * Outputs    : PASSED/FAILED
 *
 *****************************************************************************/
int katar_show_cpuinfo (void)
{
    int rc = FAILED;
    char cpu_info[KATAR_INFO_BUF_SIZE];
    FILE *fp;

    fp = fopen(KATAR_CPU_INFO_FILE, "r");
    if (fp == NULL) {
        rc = katar_get_info("/proc/cpuinfo", katar_cpu_info,
                           size_of_katar_cpu_info, KATAR_CPU_INFO_FILE);
        if (rc != PASSED) {
            printf("Failed to get CPU information. \n");
            return (rc);
        } else {
            fp = fopen(KATAR_CPU_INFO_FILE, "r");
            if (fp == NULL) {
                printf("Failed to open %s. \n", KATAR_CPU_INFO_FILE);
                return (FAILED);
            }   
        }   
    }   
    katar_get_cpucore(KATAR_CPU_INFO_FILE);
    
    printf("CPU info after inits:\n");
    while (fgets(cpu_info, KATAR_INFO_BUF_SIZE, fp) != NULL) {
        printf("%s", cpu_info);
    }   
    printf("\n");
    fclose(fp); 
    unlink(KATAR_CPU_INFO_FILE);

    return (PASSED);
}

/*****************************************************************************
 *
 * Function   : main
 * Description: entry point
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
    DIAGFLAG = 0;
    uint rc = FAILED;


    skip_test = opt_ch = 0;
    sh.cmdptr = NULL;

    if (argc > 1 ) {
        if (argv[1][0] != '-') {
            /* ignore first arugment which is the name of our program */
            cli_argc = argc - 1;
            cli_argv = argv + 1;

            /* creae buffer for CLI */
            sh.cmdptr = malloc(1024); 
            *sh.cmdptr= '\0';

            /* copy CLI into sh struct */
            cmdline_to_str(cli_argc, cli_argv, &sh);
        }
    } /* if argc > 1 */

    //Need to enable LPC memory mapping to fdff000 before run mmap
    pci_config_write(0x00, 0x1f, 0x00, 0x98, 0xfdff0001);

    rc = open_cpld();
        if (rc != PASSED ) {
        cterr('f', 0, "unable to open***/dev/cpld***\n");
    }

    /* Open modules */
    if (katar_open_module(&fd_i2c0, KATAR_I2C_0) != PASSED) {
        cterr('f', 0, "%s: Failed to open /dev/%s", __FUNCTION__, KATAR_I2C_0);
    }

    if (katar_open_module(&fd_vtop, KATAR_VTOP_KLM) != PASSED) {
        cterr('f', 0, "%s: Failed to open /dev/%s", __FUNCTION__, KATAR_VTOP_KLM);
    }

    dash_set_map(1);

    //Enable FPGA LED control
    katar_get_led_control();

	//Disable boot timer
   	katar_disable_boot_timer();

	//disable force fan high by HW request
	katar_set_fan_force_high_gpio(FALSE);

    /* Turn off memory malloc overcommit to avoid oom in memory test
     */
    system("echo 2 > /proc/sys/vm/overcommit_memory");

    if (sh.cmdptr == NULL) {
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
    return 0;
}

/*
 *------------------------------------------------------------------
 * $Log: linux_main.c,v $
 * Revision 1.2  2019/06/14 05:24:49  mikech2
 * Collapse katar-branch00 to Main Trunk
 *
 * Revision 1.1.2.16  2019/04/30 06:06:58  mikech2
 * Code cleanup
 *
 * Revision 1.1.2.15  2019/04/25 00:32:24  mikech2
 * Disable force fan high when enter diag
 *
 * Revision 1.1.2.14  2019/04/12 01:35:54  peteteng
 * Code cleanup
 *
 * Revision 1.1.2.13  2019/03/04 00:45:13  mikech2
 * Clean up codes and remove unnecessary files
 *
 * Revision 1.1.2.12  2019/02/12 08:06:29  mikech2
 * rename katar_*.h files
 *
 * Revision 1.1.2.11  2019/01/18 03:42:28  mikech2
 * Set mask off before interrupt test
 *
 * Revision 1.1.2.10  2018/12/20 09:10:57  peteteng
 * Add FPGA I2C read/write/scan/reset util
 *
 * Revision 1.1.2.9  2018/12/12 09:06:16  mikech2
 * Update FPGA utility according to SPEC2.2(FW ver:2018121214)
 *
 * Revision 1.1.2.8  2018/12/07 14:41:42  peteteng
 * Modify addr. of Aikido LPC scratchpad test
 *
 * Revision 1.1.2.7  2018/11/22 02:50:49  peteteng
 * Add Aikido register read/write utility
 *
 * Revision 1.1.2.6  2018/11/14 08:14:58  peteteng
 * Add Aikido FPGA register test
 *
 * Revision 1.1.2.5  2018/11/14 06:10:52  mikech2
 * Add I211 phy register control
 *
 * Revision 1.1.2.4  2018/11/01 08:55:02  mikech2
 * Disable boot timer when enter diag
 *
 * Revision 1.1.2.3  2018/10/30 06:32:01  peteteng
 * Change cookie util order; remove i2c-1 inspection; add ACT2 programming case Qq
 *
 * Revision 1.1.2.2  2018/10/26 02:39:34  mikech2
 * Fix typo
 *
 * Revision 1.1.2.1  2018/10/22 08:02:32  mikech2
 * Move project folder to common/src/katar/x86
 *
 * Revision 1.1.2.12  2018/10/22 03:04:33  peteteng
 * use common code tam_act2_utils.c without smart_cookie.c
 *
 * Revision 1.1.2.11  2018/10/02 02:32:15  mikech2
 * Modify FPGA register according SPEC 1.7.1
 *
 * Revision 1.1.2.10  2018/09/14 06:11:53  mikech2
 * Add mem info to system info
 *
 * Revision 1.1.2.9  2018/09/07 03:14:21  peteteng
 * Add system info utility
 *
 * Revision 1.1.2.8  2018/09/07 02:16:52  mikech2
 * Fix FPGA util issue
 *
 * Revision 1.1.2.7  2018/09/04 06:09:08  mikech2
 * Fix I2C util , realtek port & get_pcie_cap_struct_ptr return error issue
 *
 * Revision 1.1.2.6  2018/08/27 08:28:47  mikech2
 * Fix I2C & pcie scan test
 *
 * Revision 1.1.2.5  2018/07/24 09:54:12  peteteng
 * Add SFP cookie - read
 *
 * Revision 1.1.2.4  2018/06/29 07:25:26  mikech2
 * Remove compile warning and unused files
 *
 * Revision 1.1.2.3  2018/06/25 08:24:53  mikech2
 * Add interupt test menu
 *
 * Revision 1.1.2.2  2018/06/07 03:23:53  peteteng
 * fix diag launch issue
 *
 * Revision 1.1.2.1  2018/06/07 01:19:22  peteteng
 * add project katar - based on neptune
 *
 *
 *
 *------------------------------------------------------------------
 * $Endlog$
 */

