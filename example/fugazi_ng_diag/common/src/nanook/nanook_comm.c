 /* $Id: nanook_comm.c,v 1.2 2019/12/11 10:10:32 lucywang Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/nanook/nanook_comm.c,v $
 *------------------------------------------------------------------
 * Platform specific code
 * 
 *
 * Copyright (c) 2019 by Cisco Systems, Inc.
 * All rights reserved.
 *------------------------------------------------------------------
 */

 
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include "common.h"
#include "types.h"
#include "common_utils.h"
#include "menu.h"
#include "nvmonvars.h"
#include "proto.h"
#include "queryflags.h"
#include "nanook_comm.h"
#include "dnv_eth_lib.h"

int exec_cmd (char *, char *, int);
uint enable_ether_interface(int);

/*******************************************************************************
*
* Function    : nanook_mem_read32
* Description : Function to read memory by byte.
* Inputs      : offset - memory offset
*               *buf   - buffer to put read back register value
* Outputs     : PASSED/FAILED
*
*******************************************************************************
*/
int nanook_mem_read32 (uint offset, uint *buf)
{
    int      fd = -1;
    void     *map_base, *virt_addr;
    off_t    target = 0;
    unsigned map_size, page_size, offset_in_page;

    target = (off_t)offset;

    fd = open("/dev/mem", (O_RDONLY | O_SYNC));
    if (fd < 0) {
        printf("%s: Failed to open MEM device.\n", __FUNCTION__);
        return (FAILED);
    }

    map_size = page_size = getpagesize();
    offset_in_page = (unsigned)target & (page_size - 1);
    if (offset_in_page + (8 * sizeof(int)) > page_size) {
        map_size *= 2;
    }

    map_base = mmap(NULL, map_size, PROT_READ, MAP_SHARED, fd,
        target & ~(off_t)(page_size -1));
    if (map_base == MAP_FAILED) {
        printf("%s: Failed to map in virtual address space.\n",
            __FUNCTION__);
        close(fd);
        return (FAILED);
    }

    virt_addr = (char *)map_base + offset_in_page;
    *buf = *(volatile uint32_t*)virt_addr;

    if (munmap(map_base, map_size) == -1) {
        printf("%s: Failed to munmap.\n", __FUNCTION__);
        close(fd);
        return (FAILED);
    }
    close(fd);
    return (PASSED);
}

/*******************************************************************************
*
* Function    : nanook_mem_write32
* Description : Function performs write memory by byte.
* Inputs      : offset  - offset
*               wr_data - data for write
* Outputs     : PASSED/FAILED
*
*******************************************************************************
*/
int nanook_mem_write32 (uint offset, uint wr_data)
{
    int      fd = -1;
    void     *map_base, *virt_addr;
    off_t    target = 0;
    unsigned map_size, page_size, offset_in_page;

    target = (off_t)offset;

    fd = open("/dev/mem", (O_RDWR | O_SYNC));
    if (fd < 0) {
        printf("%s: Failed to open MEM device.\n", __FUNCTION__);
        return (FAILED);
    }

    map_size = page_size = getpagesize();
    offset_in_page = (unsigned)target & (page_size - 1);
    if (offset_in_page + (8 * sizeof(int)) > page_size) {
        map_size *= 2;
    }

    map_base = mmap(NULL, map_size, (PROT_READ | PROT_WRITE), MAP_SHARED, fd,
                   target & ~(off_t)(page_size -1));
    if (map_base == MAP_FAILED) {
        printf("%s: Failed to map in virtual address space.\n",
            __FUNCTION__);
        close(fd);
        return (FAILED);
    }

    virt_addr = (char *)map_base + offset_in_page;
    *(volatile uint32_t*)virt_addr = wr_data;

    if (munmap(map_base, map_size) == -1) {
        printf("%s: Failed to munmap.\n", __FUNCTION__);
        close(fd);
        return (FAILED);
    }
    close(fd);
    return (PASSED);
}

int nanook_mem_write (unsigned long int offset, uint wr_data)
{
    int      fd = -1;
    void     *map_base, *virt_addr;
    off_t    target = 0;
    unsigned map_size, page_size, offset_in_page;

    target = (off_t)offset;

    fd = open("/dev/mem", (O_RDWR | O_SYNC));
    if (fd < 0) {
        printf("%s: Failed to open MEM device.\n", __FUNCTION__);
        return (FAILED);
    }

    map_size = page_size = getpagesize();
    offset_in_page = (unsigned)target & (page_size - 1);
    if (offset_in_page + (8 * sizeof(int)) > page_size) {
        map_size *= 2;
    }

    map_base = mmap(NULL, map_size, (PROT_READ | PROT_WRITE), MAP_SHARED, fd,
                   target & ~(off_t)(page_size -1));
    if (map_base == MAP_FAILED) {
        printf("%s: Failed to map in virtual address space.\n",
            __FUNCTION__);
        close(fd);
        return (FAILED);
    }

    virt_addr = (char *)map_base + offset_in_page;
    *(volatile uint32_t*)virt_addr = wr_data;

    if (munmap(map_base, map_size) == -1) {
        printf("%s: Failed to munmap.\n", __FUNCTION__);
        close(fd);
        return (FAILED);
    }
    close(fd);
    return (PASSED);
}


/* ****************************************************************************
 *
 * Function: exec_cmd
 *
 * Description: Execute the linux system command.
 *
 * Input:    *command   - Point to the linux system command string
 *           *result    - Point to the buffer that records the command result.
 *           result_len - The buffer length that records the command result.
 *
 * Outputs:  PASSED - No errors encountered.
 *           FAILED - Errors encountered.
 *
 * Assumptions:
 *
 * ***************************************************************************/
int exec_cmd (char *command, char *result, int result_len)
{
    FILE *fp;
    int status;

    fp = popen (command, "r");
    if (fp != NULL){
        fread(result, sizeof(char), result_len, fp);
    } else {
        cterr('f', 0, "Execute command failed");
        return (FAILED);
    }

    status = pclose(fp);

    if (status == -1) {
        cterr('f', 0, "pclose error");
        return (FAILED);
    }

    return (PASSED);
}


uint enable_ether_interface(int eth_no)
{

    uint ix = 0, retry = 0;
    char cmd_output[INFO_LEN];
    char *cmd = "dmesg > /tmp/dmesg_new; diff /tmp/dmesg_old /tmp/dmesg_new";
    //char *cmd = "cat /tmp/ixgbe_fail.txt";
    
    for (ix = 0; ix < RETRY_MAX; ix++) {

        system("dmesg > /tmp/dmesg_old");
        memset(cmd_output, '\0', INFO_LEN);

        if (eth_no == ETHER_INTERFACE_NIM) {
            system(ETH_RM_IXGBE_MODULE);
            msleep(500);
            system(ETH_INS_IXGBE_MODULE_TESTCARD);
        } else if(eth_no == ETHER_INTERFACE_AC3) {
            system(ETH_RM_IXGBE_MODULE);
            msleep(500);
            system(ETH_INS_IXGBE_MODULE_AC3);
        } else {
            printf("Unsupported ether number.");
            return (FAILED);
        }

        if (exec_cmd(cmd, cmd_output, INFO_LEN) == FAILED) {
            printf("Retrying ixgbe re-probe...");
	     retry ++;
        } else if(strstr(cmd_output, "failed with error -5") != NULL) {
            printf("Retrying ixgbe re-probe...");
	     retry ++;
        } else {
            printf("Ixgbe probed succussed...");
            return (PASSED);
        }	
    }

    if (retry == RETRY_MAX) {
        return (FAILED);
    } else {
        return (PASSED);
    }

}


/*-------------------------------------------------
 * $Log: nanook_comm.c,v $
 * Revision 1.2  2019/12/11 10:10:32  lucywang
 * Merged Nanook to main trunk
 *
 *
 * $Endlog$
 *-------------------------------------------------
 */
