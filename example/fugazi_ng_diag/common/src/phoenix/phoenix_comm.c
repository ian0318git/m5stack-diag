/* $Id: phoenix_comm.c,v 1.2 2021/04/15 00:52:27 achiu2 Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/phoenix/phoenix_comm.c,v $
 *------------------------------------------------------------------
 * Platform specific code
 * 
 *
 * Copyright (c) 2018-2019 by Cisco Systems, Inc.
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
#include "phoenix_comm.h"

/*******************************************************************************
*
* Function    : phoenix_mem_read32
* Description : Function to read memory by byte.
* Inputs      : offset - memory offset
*               *buf   - buffer to put read back register value
* Outputs     : PASSED/FAILED
*
*******************************************************************************
*/
int phoenix_mem_read32 (uint offset, uint *buf)
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
* Function    : phoenix_mem_write32
* Description : Function performs write memory by byte.
* Inputs      : offset  - offset
*               wr_data - data for write
* Outputs     : PASSED/FAILED
*
*******************************************************************************
*/
int phoenix_mem_write32 (uint offset, uint wr_data)
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

