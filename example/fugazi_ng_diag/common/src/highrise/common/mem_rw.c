
/* $Id:
 * $Source:
 *-----------------------------------------------------------------------------
 *
 * Filename   : mem_rw.c
 * Description: highrise memory read/write function
 *  June 2019,  shuyyu
 * Copyright (c) 2019~ by Cisco Systems, Inc.
 * All rights reserved.
 *
 *-----------------------------------------------------------------------------
 */
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include "common.h"





/*******************************************************************************
 *
 * Function    : highrise_mem_read32
 * Description : Function to read highrise memory by byte.
 * Inputs      : offset - memory offset
 *               *buf   - buffer to put read back register value
 * Outputs     : PASSED/FAILED
 *
 *******************************************************************************
 */
int highrise_mem_read32 (uint offset, uint *buf)
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
 * Function    : highrise_mem_write32
 * Description : Function performs write highrise memory by byte.
 * Inputs      : offset  - offset
 *               wr_data - data for write
 * Outputs     : PASSED/FAILED
 *
 *******************************************************************************
 */
int highrise_mem_write32 (uint offset, uint wr_data)
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
