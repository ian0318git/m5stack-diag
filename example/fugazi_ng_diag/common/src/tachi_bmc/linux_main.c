/* $Id: linux_main.c,v 1.6 2019/09/10 01:03:39 haohsu Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/tachi_bmc/linux_main.c,v $
 *------------------------------------------------------------------
 * linux_main.c - Main Entry for this application
 *
 * June 2015, Times Huang, ported from Victory
 *
 * Copyright (c) 2015-2019 by Cisco Systems, Inc.
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
#include "linux_api.h"
#include "proto.h"
#include "slot.h"
#include "sh.h"  /* struct shstuff */
#include "uio_utils.h"
#include "router_if.h"
#include "plat_defs.h"
#include "cross_platform.h"
#include "diag_fpga_util.h"
#include "diag_temp_sensor_util.h"
#include "diag_mcu_util.h"
#include "common.h"
#include "diag_nc_common.h"
#include "diag_pem_fan.h" /* PSU fan info */
#include "diag_plat_cookie.h" /* POE sku info */
#include "diag_fan_util.h"
#include "diag_rtc_test.h"
#include "diag_barometer_util.h"
#include "diag_geswitch_test.h"

extern char *banner_string;
/* FIXME: The following macro might need to be modify for Hercules Project */
#define OVLD_FPGA_KLM               "uio0"
#define OVLD_I2C_KLM                "i2c-0"
#define OVLD_VTOP_KLM               "addr_vtop"
#define OVLD_CPLD_KLM               "cpld"
#define OVLD_INFO_BUF_SIZE           256
#define MARVELL_ERRATA_RESULT "/var/log/marvell_errata_applied.txt"

/* FIXME: The following variable might need to be modify for Hercules Project */
static int fd_i2c0 = -1;
static int fd_vtop = -1;
//static int fd_cpld = -1;
static int quick_launch = 0;


volatile unsigned char hkeepflags;
jmp_buf monjmpbuf, *monjmpptr;

unsigned int brd_ver = 0;

extern int ttf2array(int size, const char *file, unsigned char *fpga);
static int diag_get_info(char*, char**, int, char*);
static int diag_show_cpuinfo(void);
static int diag_show_meminfo(void);
void apply_errata_check(void);

extern unsigned char swapbyte(unsigned char c);

unsigned int dash_fpga_fw_size = 0;
unsigned char *dash_fpga_fw_array = NULL;

//mb_iofpga_mcu_regs_t  *IOFPGA_MCU_REGS;

unsigned long dash_msg = 0;
unsigned long dash_fpga = 0;
unsigned long dash_cpld = 0;
extern char *optarg;
#if 0
static char *diag_cpu_info[] = {
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
#endif

static char *diag_cpu_info[] = {
    "Processor",
    "CPU Implementer",
    "CPU architecture",
    "CPU variant",
    "CPU part",
    "CPU revision",
    "Hardware",
};

static const uint size_of_diag_cpu_info = \
                  sizeof(diag_cpu_info) / sizeof(uchar *);

static char *diag_mem_info[] = {
    "MemTotal",
    "MemFree",
    "Buffers",
    "Cached",
    "SwapCached",
    "Hugepagesize",
    "DirectMap4k",
    "DirectMap2M"
};

static const uchar size_of_diag_mem_info = \
                   sizeof(diag_mem_info) / sizeof(uchar *);

static unsigned int skip_test = 0;

#define HRCLS_DEV_TABLE_SIZE \
      (sizeof(diag_dev_table) / sizeof(ovld_dev_info_t))


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
 * Function   : diag_get_info
 * Description: Get Hercules system Info by reading related linux file.
 * Inputs     : info_file - related Linux file to get info
 *              info_item - the item to collect
 *              info_item_size - size of items that needs to collect
 *              file_name - file to put the collected info
 * Outputs    : PASSED/FAILED
 *
 *****************************************************************************/
static int
diag_get_info (char* info_file, char** info_item, 
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
 * Function   : diag_show_cpuinfo
 * Description: To show Overlord CPU Info by reading file "/proc/cpuinfo".
 * Inputs     : None
 * Outputs    : PASSED/FAILED
 *
 *****************************************************************************/
static int diag_show_cpuinfo (void)
{
    int rc = FAILED;
    char cpu_info[OVLD_INFO_BUF_SIZE];
    FILE  *fp;
 
    fp = fopen("/hrcls_cpuinfo.txt", "r");
    if (fp == NULL) {
        rc = diag_get_info("/proc/cpuinfo", diag_cpu_info,
                           size_of_diag_cpu_info, "/hrcls_cpuinfo.txt");
        if (rc != PASSED) {
            printf("Failed to get Hercules CPU Info !!!\n");
            return (rc);
        } else {
            fp = fopen("/hrcls_cpuinfo.txt", "r");
            if (fp == NULL) {
                printf("Failed to open /hrcls_cpuinfo.txt");
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
 * Function   : diag_show_meminfo
 * Description: To show Hercules MEM Info by reading file "/proc/meminfo".
 * Inputs     : None
 * Outputs    : PASSED/FAILED
 *
 *****************************************************************************/
static int
diag_show_meminfo (void)
{
    int rc = FAILED;
    char mem_info[OVLD_INFO_BUF_SIZE];
    FILE  *fp;
 
    fp = fopen("/hrcls_meminfo.txt", "r");
    if (fp == NULL) {
        rc = diag_get_info("/proc/meminfo", diag_mem_info,
                           size_of_diag_mem_info, "/hrcls_meminfo.txt");
        if (rc != PASSED) {
            printf("Failed to get Hercules MEM Info !!!\n");
            return (rc);
        } else {
            fp = fopen("/hrcls_meminfo.txt", "r");
            if (fp == NULL) {
                printf("Failed to open /hrcls_meminfo.txt");
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
           /* FIXME:Fill this funciot for Hercules Project */ 
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
    /* FIXME: Fill this function for Hercules Project */
    return(PASSED);
}

int
get_platform_plane (void)
{
        return CP;  /*defined in dash_fpga.h */
}

/**************************************************************************
 *
 * Name: apply_errata_check
 *
 * Description: Check if the Marvell 6320 Errata is set, if not 
 *              then apply Errata
 * 
 * Inputs: None
 *
 * Outputs: None
 **************************************************************************/
void apply_errata_check(void)
{
    size_t size = 0;

    if (file_exist(MARVELL_ERRATA_RESULT, &size)) {
        printf("Marvell 6320 Errata is set");
    } else {
        geswitch_apply_errata();
    }
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
    DIAGFLAG = 0;
    char fpga_file[80];

    /* adjust the free mem ratio to 80%, to bypass the oom-killer. 
     * the act2 process will use more memory, 90% (default) is not stable. 
     */
    system("echo 2 > /proc/sys/vm/overcommit_memory");

    fpga_file[0] = '\0';
    skip_test = opt_ch = 0;
    sh.cmdptr = NULL;

msleep_delay = 1; 
skip_init_seq = 0;
    if (argc > 1 ) {
        if (argv[1][0] != '-') {
            if (is_special_case(argc, argv)) {

            } else {
                /* ignore first arugment which is the name of our program */
                cli_argc = argc - 1;
                cli_argv = argv + 1;

                /* cretae buffer for CLI */
                sh.cmdptr = malloc(1024);
                *sh.cmdptr= '\0';

                /* copy CLI into sh struct */
                cmdline_to_str(cli_argc, cli_argv, &sh);

            }
        } else {
            while ((opt_ch = getopt(argc, (char **)argv, ":qs:nd:")) >= 0) {
                switch(opt_ch) {
                case 'q':
                    quick_launch = 1;
                    break;
                case 'n':
                    diag_nc_server_dispatch_comm();
                    return (0);
                    break;
                case 's':
                    skip_init_seq = 1; 
                    break;
                case 'd': /* delay for msleep */
                    msleep_delay = atoi(optarg);
                    break;
                case '?':
                    goto usage;
                    break;
                } /* switch */
            } /* while getopt */
        } /* if argv[1][0] != "-" */
    } /* if argc > 1 */

    if ( !quick_launch) {
        /* FIXME: if anything can skip to speed up load process
         * add here
         */
            
        if (!sh.cmdptr) {
            diag_show_cpuinfo();
            diag_show_meminfo();
            diag_fpga_ver_display();
            diag_mcu_show_ver(); 
            diag_show_temperature();
            show_all_fan_rpm();
            diag_show_barometer();            
            utility_display_rtc(TRUE);
            display_pem_fan_spd();  /* PSU fan speed */
            is_poe_sku();
            get_board_ver();
            /* Show Fan Info here */
            apply_errata_check();
        }

    }


    /* Turn off memory malloc overcommit to avoid oom in memory test
     */
    system("echo 2 > /proc/sys/vm/overcommit_memory");

    if (sh.cmdptr != NULL) {
        if (sh.cmdptr[0])
            free(sh.cmdptr);
    } else {
        printf("%s", banner_string);

#ifdef DISPLAY_ENGINEERING_BANNER
	    engineering_banner();
#endif

    init_slot_info();
    fflush(stdin);

    diag_menu(1, argv); /* goto menu directly; dont' call monitor(); */

    }
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

/*---------------------------------------------------------------
$Log: linux_main.c,v $
Revision 1.6  2019/09/10 01:03:39  haohsu
[CSCvr07313]-Marvell 6320 to BMC eth1 frame error issue

Revision 1.5  2017/01/25 01:13:13  kodko
Get the Fab Version and PCB Revision cookie field values to distinguish the board type and do the USB3.0/USB2.0 or USB3.0 only test.

Revision 1.4  2016/07/12 01:53:19  hondwang
Fix F2W bug and add PCAMAP ID

Revision 1.3  2016/06/04 09:22:20  alpeng
initial check in for f2w

Revision 1.2  2016/04/20 11:25:29  benchen2
add tachi fru portion

$Endlog$
*/

