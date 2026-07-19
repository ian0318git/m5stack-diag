/* $Id: nutella_comm.c,v 1.5 2020/03/06 07:42:32 alicehua Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/nutella/nutella_comm.c,v $
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
#include "nutella_comm.h"

/*******************************************************************************
 *                          Function Declaration
 *******************************************************************************
 */
int is_bootloader_rommon(void);

/*******************************************************************************
 *
 * Function    : nutella_mem_read32
 * Description : Function to read Nutella memory by byte.
 * Inputs      : offset - memory offset
 *               *buf   - buffer to put read back register value
 * Outputs     : PASSED/FAILED
 *
 *******************************************************************************
 */
int nutella_mem_read32 (uint offset, uint *buf)
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
 * Function    : nutella_mem_write32
 * Description : Function performs write Nutella memory by byte.
 * Inputs      : offset  - offset
 *               wr_data - data for write
 * Outputs     : PASSED/FAILED
 *
 *******************************************************************************
 */
int nutella_mem_write32 (uint offset, uint wr_data)
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

/*******************************************************************************
 *
 * Function    : is_bootloader_rommon 
 * Description : The way to sign kenel image is different between BIOS and ROMMON,
 *               so there will be two version of kernel.
 *               Add a file in kernel version V2.X.Y to distinguish bootloader is
 *               BIOS or ROMMON.
 * Inputs      : NONE
 * Outputs     : TRUE/FALSE
 *
 *******************************************************************************
 */
int is_bootloader_rommon (void)
{
    FILE *file_ptr = fopen(BOOTLOADER_IS_ROMMON_FLAG, "r");

    if (file_ptr == NULL) {
        /* Can't open file successfully, it means kernel version is V1.X.Y,
         * and bootloader is BIOS. */
        return (FALSE);
    } else {
        /* Open file successfully, it means kernel version is V2.X.Y,
         * and bootloader is ROMMON. */
        fclose(file_ptr);
        return (TRUE);
    }
}
/*-------------------------------------------------
$Log: nutella_comm.c,v $
Revision 1.5  2020/03/06 07:42:32  alicehua
CSCvt28948:
1. Modify codes for FPGA register default value changing issue.
   With new ROMMON (17.3(03d)), FPGA register (0x010) will get 0x59,
   so we just check bit 7:0, ignore bit 0.
2. Add a function to distinguish bootloader is BIOS or ROMMON,
   so that we can hide IRQ test items if bootloader is BIOS.

Revision 1.4  2019/07/11 12:31:31  alicehua
Collapse Nutella codes into main trunk.

$Endlog$
*/
