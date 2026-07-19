/* $Id: diag_bootflash_lib.c,v 1.1 2015/02/26 07:18:29 xiaoyizh Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/wallander/diag_bootflash_lib.c,v $
 *-----------------------------------------------------------------------------
 * diag_bootflash_lib.c -Functions for Bootflash test
 *
 * Xiaoying Zhang -- Feb. 2014
 *
 * Copyright (c) 2015 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */
#include <sys/types.h>
#include <unistd.h>
#include "error.h"
#include "common.h"
#include "types.h"
#include "menu.h"
#include "diag_bootflash_lib.h"
#include "diag_common_drv.h"

#include <fcntl.h>
#include <asm/ioctl.h>
#include <sys/stat.h>
#include <signal.h>
#include <mtd/mtd-user.h>
#include <sys/ioctl.h>

#include <stdio.h>
#include <stdlib.h>
#include "nvsysvars.h"
// #include "nvmonvars.h"
#include <errno.h>

#include <sys/mman.h>
#include <sys/stat.h>

int bootflash_erase_sector(int, u_int32_t, u_int32_t);
int bootflash_write_sector(int, u_int32_t, u_int32_t, u_int8_t *);
int bootflash_read_sector(int,  u_int32_t, u_int32_t, u_int8_t *);
void bootflash_free_memory(u_int8_t * ,u_int8_t * ,uchar * ,int);
uchar* fpga_get_local_bus_addr(void);

ushort *bootflash_mmap_addr[SECTOR_NUM];

/******************************************************************************
 *  
 * Function: bootflash_erase_sector
 *   
 * Description: This function use to erase bootflash sector.  
 *      
 * Inputs      : fwh_fd - point to erase sector
 *               erase_offset - erase addr
 *               erase_length - length to erase
 *          
 * Outputs     : PASSED / FAILED
 *            
 ******************************************************************************/
int bootflash_erase_sector (int fwh_fd, u_int32_t erase_offset, u_int32_t erase_length)
{
    struct erase_info_user erase;
    erase.start=erase_offset;
    erase.length=erase_length;
    int erase_ret;

    /* start erase (erase size of mtd1 is "0x10000", use command "cat /proc/mtd") */
    erase_ret = ioctl(fwh_fd, MEMERASE, &erase);

    /* judge whether the erase process is sucess or not*/
    if (erase_ret < 0) {
        cterr('f', 0, "MEMERASE ioctl fail");
        return (FAILED);
    } else {
        prpass(testpass, "Erase sucess, erased %d bytes from address 0x%x",erase.length,erase.start);
    }

    return (PASSED);
}

/******************************************************************************
 *    
 * Function: bootflash_write_sector
 *        
 * Description: This function use to write bootflash sector.      
 *            
 * Inputs      : fwh_fd - point to write sector
 *               write_offset - specify addr to write
 *               write_length - length to write
 *               write_buf - data to write
 *                    
 * Outputs     : PASSED / FAILED
 *                      
 *******************************************************************************/
int bootflash_write_sector (int fwh_fd, u_int32_t write_offset, u_int32_t write_length, 
                                                    u_int8_t *write_buf)
{
    int write_ret;
    int write_size = write_length * sizeof(u_int8_t);

    if (write_offset != lseek (fwh_fd, write_offset, SEEK_SET)) {
        cterr('f', 0, "lseek() can't find write_offset at %#.8x", write_offset);
        return (FAILED);
    }

    prpass(testpass, "copied %d bytes from offset 0x%.8x in flash", write_length, write_offset); 

    do {
        if (write_length <= write_size) {
            write_size = write_length;
        }

        /* write data from file to flash */
        write_ret = write(fwh_fd, write_buf, write_size);

        if (write_ret < 0) {
            cterr('f', 0, "faile to write data from file to flash");
            return (FAILED);
        }
        write_length -= write_size;

    } while (write_length > 0);

    return (PASSED);
}

/******************************************************************************
 *
 * Function: bootflash_read_sector
 *
 * Description: This function use to read bootflash sector.
 *
 * Inputs      : fwh_fd - point to read sector
 *               read_offset - read addr
 *               read_length - length to read
 *               read_buf - read buf
 *
 *            Outputs     : PASSED / FAILED
 *
 *******************************************************************************/
int bootflash_read_sector (int fwh_fd,  u_int32_t read_offset, u_int32_t read_length, 
                           u_int8_t *read_buf)
{
    int read_ret;

    /* make sure read data from the beginning */
    if (read_offset != lseek(fwh_fd,read_offset,SEEK_SET)) {
        cterr('f', 0, "lseek() can't read data from the beginning");
        return (FAILED);
    }

    /* read 64KB data from flash */
    read_ret = read(fwh_fd, read_buf, read_length);

    if (read_ret < 0) {
        cterr('f', 0, "fail to read data from flash");
        return (FAILED);
    }

    return (PASSED);
}

/******************************************************************************
 *
 * Function: bootflash_free_memory
 *
 * Description: This function use to free memory.
 *
 * Inputs      : read_buf - read buf
 *               write_buf - write buf
 *               mem_ptr - mem ptr
 *               fwh_fd - point to flash sector
 *
 * Outputs     : PASSED / FAILED
 *
 *******************************************************************************/
void bootflash_free_memory (u_int8_t *read_buf, u_int8_t *write_buf, 
                            uchar *mem_ptr, int fwh_fd)
{
    free (read_buf);
    free (write_buf);
    free (mem_ptr);

    if (fwh_fd >= 0) {
        close (fwh_fd);
    } 
}

/******************************************************************************
 *
 * Function    : bootflash_init_virt_addr
 * Description : Initialize the virtual address table.
 * Input       : None.
 *              
 * Output   :  None
 *
 *****************************************************************************/
void bootflash_init_virt_addr (void)
{
    int i;
    for (i = 0; i < SECTOR_NUM; i++) {
        bootflash_mmap_addr[i] = NULL;
    }
}

/******************************************************************************
 *
 * Function    : bootflash_mmap_get_addr
 * Description : Create a new mapping in the virtual address space for the sector
 * Input       : None.
 *
 * Output: mmap_addr/NULL
 *
 *****************************************************************************/
uint8_t* bootflash_mmap_get_addr (int sector_idx)
{
    int fd;

    assert(sector_idx < SECTOR_NUM);

    if (bootflash_mmap_addr[sector_idx] == NULL) {
        fd = open("/dev/mem", O_RDWR);
        if (fd < 0) {
            printf("Open MEM device failed");
            return (NULL);
        }

        bootflash_mmap_addr[sector_idx] = mmap(0, BOOTFLASH_MMAP_LENGTH, PROT_READ | PROT_WRITE, MAP_SHARED, 
                           fd, BOOTFLASH_BASE_ADDR + SECTOR_SIZE * sector_idx);

        if (bootflash_mmap_addr[sector_idx] == MAP_FAILED) {
            printf("Unable to create a new mapping in the virtual address space\n");
            bootflash_mmap_addr[sector_idx] = NULL;
        }
/*        printf("Mapped sector %d to 0x%x.\n", 
            sector_idx, bootflash_mmap_addr[sector_idx]);*/
    }

    return (bootflash_mmap_addr[sector_idx]);
}

/*-------------------------------------------------
 * $Log: diag_bootflash_lib.c,v $
 * Revision 1.1  2015/02/26 07:18:29  xiaoyizh
 * Initial check in for Wallander.
 *
 *
 * $Endlog$
 *-------------------------------------------------
 */
