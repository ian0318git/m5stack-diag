/* $Id: reva_sysinfo.c,v 1.2 2016/05/06 03:43:53 umlin Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/reva/reva_sysinfo.c,v $
 *------------------------------------------------------------------
 *
 * reva_sysinfo.c - More information of Reva module.
 *
 *
 * Copyright (c) 2015-2016 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#include <stdio.h>
#include "common.h"
#include "types.h"

#define REVA_INFO_BUF_SIZE          256
#define REVA_CPU_INFO_FILE           "/reva_cpuinfo.txt"
#define REVA_MEM_INFO_FILE           "/reva_meminfo.txt"

/* Function prototype */
void display_sys_info(int);

static char *reva_cpu_info[] = {
    "Processor",
    "BogoMIPS",
    "Features",
    "CPU implementer",
    "CPU architecture",
    "CPU variant",
    "CPU part",
    "CPU revision",
};
static const uint size_of_reva_cpu_info =
    sizeof(reva_cpu_info) / sizeof(uchar *);

static char *reva_mem_info[] = {
    "MemTotal",
    "MemFree",
    "Buffers",
    "Cached",
    "SwapCached",
    "Hugepagesize",
    "DirectMap4k",
    "DirectMap2M"
};

static const uchar size_of_reva_mem_info =
    sizeof(reva_mem_info) / sizeof(uchar *);


/*****************************************************************************
 * Function   : ExecuteCmdbyPopen
 * Description: Execute shell command and kepp the returned string into 
 *              retBuf[sizeOfBuf]
 *
 * Inputs     : cmd - the command that wanted execute
 *              retBuf - content of result
 *              sizeOfBuf - size of buffer
 * Outputs    : size of result
 *****************************************************************************/

int ExecuteCmdbyPopen(char *cmd, char *retBuf, int sizeOfBuf)
{
    FILE *f;
    char *pRetBuf = retBuf;
    int  count, retCount = 0;

    if( cmd==NULL || retBuf==NULL ) {
        return 0;
    } else {
        f = popen(cmd, "r");
    }
    
    if (f) {
        while (1) {
            *pRetBuf = '\0';
            count = 0;
            fgets(pRetBuf, sizeOfBuf-retCount, f);
            count = strlen(pRetBuf);
            if (count == 0) {
                break;
            }
            pRetBuf += count;
            retCount += count;
        }
    }

    pclose(f);
    return retCount;
}

/*****************************************************************************
 * Function   : reva_get_info
 *
 * Description: Get Reva system Info by reading related linux file.
 *
 * Inputs     : info_file - related Linux file to get info
 *              info_item - the item to collect
 *              info_item_size - size of items that needs to collect
 *              file_name - file to put the collected info
 * Outputs    : PASSED/FAILED
 *****************************************************************************/
static int reva_get_info(char *info_file, char **info_item,
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
 * Function   : reva_get_cpucore
 *
 * Description: To show CPU core number by reading file "/proc/cpuinfo".
 *
 * Inputs     : None
 * Outputs    : PASSED/FAILED
 *****************************************************************************/
static int reva_get_cpucore( char *file_name )
{
    char sys_cmd[256];
    char sys_cpucore[3];

    if ((ExecuteCmdbyPopen ("cat /proc/cpuinfo | grep processor | wc -l", 
        sys_cpucore, REVA_INFO_BUF_SIZE)) == 0) {
        cterr('f',0,"get CPU core failed!!\n");
        return (FAILED);
    }

    sprintf(sys_cmd, "echo \"CPU cores\t: %d\" >> %s", 
        atoi(sys_cpucore), file_name);
    system(sys_cmd);

    return (PASSED);
}

/*****************************************************************************
 * Function   : reva_show_cpuinfo
 *
 * Description: To show CPU Info by reading file "/proc/cpuinfo".
 *
 * Inputs     : None
 * Outputs    : PASSED/FAILED
 *****************************************************************************/
int reva_show_cpuinfo(void)
{
    int rc = FAILED;
    char cpu_info[REVA_INFO_BUF_SIZE];
    FILE *fp;

    fp = fopen(REVA_CPU_INFO_FILE, "r");
    if (fp == NULL) {
        rc = reva_get_info("/proc/cpuinfo", reva_cpu_info,
                           size_of_reva_cpu_info, REVA_CPU_INFO_FILE);
        if (rc != PASSED) {
            printf("Failed to get CPU information. \n");
            return (rc);
        } else {
            fp = fopen(REVA_CPU_INFO_FILE, "r");
            if (fp == NULL) {
                printf("Failed to open %s. \n", REVA_CPU_INFO_FILE);
                return (FAILED);
            }
        }
    }

    printf("CPU info after inits:\n");
    while (fgets(cpu_info, REVA_INFO_BUF_SIZE, fp) != NULL) {
        printf("%s", cpu_info);
    }
    printf("\n");

    fclose(fp);	
    unlink(REVA_CPU_INFO_FILE);

    return (PASSED);
}

/*****************************************************************************
 * Function   : reva_show_meminfo
 *
 * Description: To show memory Info by reading file "/proc/meminfo".
 *
 * Inputs     : None
 * Outputs    : PASSED/FAILED
 *****************************************************************************/
int reva_show_meminfo(void)
{
    int rc = FAILED;
    char mem_info[REVA_INFO_BUF_SIZE];
    FILE *fp;

    fp = fopen(REVA_MEM_INFO_FILE, "r");
    if (fp == NULL) {
        rc = reva_get_info("/proc/meminfo", reva_mem_info,
                           size_of_reva_mem_info, REVA_MEM_INFO_FILE);
        if (rc != PASSED) {
            printf("Failed to get memory information. \n");
            return (rc);
        } else {
            fp = fopen(REVA_MEM_INFO_FILE, "r");
            if (fp == NULL) {
                printf("Failed to open %s. \n", REVA_MEM_INFO_FILE);
                return (FAILED);
            }
        }
    }

    printf("MEM info after inits:\n");
    while (fgets(mem_info, REVA_INFO_BUF_SIZE, fp) != NULL) {
        printf("%s", mem_info);
    }
    printf("\n");

    fclose(fp);
    unlink(REVA_MEM_INFO_FILE);

    return (PASSED);
}

typedef struct xadc_reg_t_ {
    ulong reserve1[128];                 /* 0x0000 - 0x019C Reserve */
    ulong temp;                          /* 0x0200, Temperature */
    ulong reserve2[31];                  /* 0x0204 - 0x027C, Reserve */
    ulong temp_max;                      /* 0x0280, Max Temperature  */
    ulong reserve3[3];                   /* 0x0284 - 0x028C, Reserve */
    ulong temp_min;                      /* 0x0290, Max Temperature  */
} xadc_reg_t;

/*****************************************************************************
 * Function   : reva_show_temp
 *
 * Description: To show the internal temperature.
 *
 * Inputs     : None
 * Outputs    : PASSED/FAILED
 *****************************************************************************/
int reva_show_temp(void)
{
    ulong base_addr = get_fpga_base();
    xadc_reg_t *xadc_csr = (xadc_reg_t *)(base_addr + 0x8000);

    printf("TEMP. info after inits:\n");

    printf("Current temperature: %.3f\n", (((xadc_csr->temp & 0xfff0) >> 4 ) * 503.975 / 4096.0 - 273.15));
    printf("Maximum temperature: %.3f\n", (((xadc_csr->temp_max & 0xfff0) >> 4 ) * 503.975 / 4096.0 - 273.15));
    printf("Minimum temperature: %.3f\n\n", (((xadc_csr->temp_min & 0xfff0) >> 4 ) * 503.975 / 4096.0 - 273.15));

    return (PASSED);
}

/**********************************************************************
 * Function: display_sys_info
 *
 * Description: display system info, ex. memory info, cpu info, fpga version
 *
 * Input : Level of display. Hight level will display more info.
 * Output: None
 ***********************************************************************/
void display_sys_info(int level)
{
    reva_show_meminfo();
    reva_show_cpuinfo();
    reva_show_temp();
    
    fpga_version();
    printf("\n");
}

/******** History ******** 
$Log: reva_sysinfo.c,v $
Revision 1.2  2016/05/06 03:43:53  umlin
Reva: Commit Reva module side diag codes to main trunk

$Endlog$
*/
