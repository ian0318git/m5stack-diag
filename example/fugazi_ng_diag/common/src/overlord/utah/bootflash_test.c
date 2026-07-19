/* $Id: bootflash_test.c,v 1.1 2016/10/16 12:28:22 iachang Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/overlord/utah/bootflash_test.c,v $
 *------------------------------------------------------------------
 *
 * bootflash_test.c: bootflash test function for Goldbeach
 *
 * June 2014 - Ian Chang 
 *
 * Copyright (c) 2016 by Cisco Systems, Inc.
 * All rights reserved.
 *
 */
 
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
//#include <mtd/mtd-abi.h>
#include <mtd/mtd-user.h>
#include <sys/ioctl.h>
#include "common.h"
#include "types.h"
#include "defs.h"
#include "setjmps.h"
#include "signals.h"
#include "menu.h"
#include "error.h"
#include "proto.h"
#include "strings.h"
#include "nvmonvars.h"
#include "bootflash_test.h"
#include "queryflags.h"

/* Function prototype */
typedef struct erase_info_user erase_info_t;
erase_info_t erase;


ushort *bootflash_mmap_addr = NULL;

/******************************************************************************
 *
 * Function    : bootflash_mmap_get_addr
 * Description : Create a new mapping in the virtual address space.
 * Input       : None.
 *              
 * Output: mmap_addr/NULL
 *
 *****************************************************************************/
uint16_t* bootflash_mmap_get_addr (void)
{
    int fd;
    
    if (bootflash_mmap_addr == NULL) {
        fd = open("/dev/mem", O_RDWR);
        if (fd < 0) {
            printf("Open MEM device failed");
            return (NULL);
        }

        bootflash_mmap_addr = mmap(0, BOOTFLASH_MMAP_LENGTH, PROT_READ | PROT_WRITE, MAP_SHARED, 
                           fd, BOOTFLASH_BASE_ADDR);

        if (bootflash_mmap_addr == MAP_FAILED) {
            printf("Unable to create a new mapping in the virtual address space\n");
            bootflash_mmap_addr = NULL;
        }
    }
    
    return (bootflash_mmap_addr);
}
/*******************************************************************************
 *
 * Function    : get_bootflash_info 
 * Description : Get bootflash information such as device id  
 *
 * Inputs      : none 
 * Outputs     : PASSED / FAILED 
 *
 *****************************************************************************/
int get_bootflash_info (void)
{
    uint16_t *bootflash_offset; 
    int ix;

    printf("Get Bootflash Information\n");

    bootflash_offset = bootflash_mmap_get_addr();

    if (bootflash_offset == NULL) {
        return (FAILED);
    } 
    
    *(bootflash_offset + FLASH_INFO_CMD_ADDR1) = FLASH_INFO_DATA1;
    *(bootflash_offset + FLASH_INFO_CMD_ADDR2) = FLASH_INFO_DATA2; 
    *(bootflash_offset + FLASH_INFO_CMD_ADDR3) = FLASH_INFO_DATA3;

    printf("Manufacturer ID = %x\n", *(bootflash_offset + 0x0)); 
    printf("Device ID1= %x\n", *(bootflash_offset + 0x1));
    printf("Device ID2= %x\n", *(bootflash_offset + 0xe));
    printf("Device ID3= %x\n", *(bootflash_offset + 0xf));
    printf("Sector Protect Verify = %x\n", *(bootflash_offset + 0x2));
    printf("Secure Silicon Sector Factory Protect = %x\n", *(bootflash_offset + 0x3));

    *bootflash_offset = 0xf0;
    /* PPB command set entry */
    *(bootflash_offset + PPB_ENTRY_CMD1) = PPB_ENTRY_DATA1;
    *(bootflash_offset + PPB_ENTRY_CMD2) = PPB_ENTRY_DATA2;
    *(bootflash_offset + PPB_ENTRY_CMD3) = PPB_ENTRY_DATA3;

    /* PPB status read to judge sector is protect or not */
    for (ix = 0; ix < (TOTAL_OTP_SIZE/2); ix += (SECTOR_SIZE/2)) {
        bootflash_offset += (uint16_t)ix;

        if (*bootflash_offset == 1) {
            printf("Sector-%x is unprotected, PPB status = %x\n", ix, *bootflash_offset);
        } else {
            printf("Sector-%x is protected, PPB status = %x\n", ix, *bootflash_offset);
        }
    }

    /* PPB command set exit */
    *bootflash_offset = PPB_EXIT_DATA1;
    *bootflash_offset = PPB_EXIT_DATA2;

    return (PASSED);
}

/******************************************************************************
 *  
 * Function: boot_flash_util
 *   
 * Description: This function provide the Read/Write Boot Flash utility 
 *              for first sector 
 *              Goldbeach first sector write protected .
 * Inputs      : None
 *          
 * Outputs     : PASSED / FAILED
 *            
 ******************************************************************************/
int 
boot_flash_util (void) 
{
    int  ix = 0;
    int  choice = 0, retval, srom_no = 0,srom_no1 = 0;
    unsigned int nbytes, value;
    unsigned char read_buf[BOOTFLASH_TEST_LENGTH];
    unsigned char write_buf[SECTOR_SIZE];
    uint32_t offset;   
    printf("\nBoot Flash Utility List:\n");
    printf("1. Read  Flash Data\n");
    printf("2. Read  Flash Sector 0 Data\n");
    printf("3. Write Flash Sector 0 Data\n");
    printf("4. Get The Flash Info.\n");
    choice = getdec_answer("Please enter your choice: ", 1, 1, 4);

    srom_no = open(BOOTFLASH_MTD_TEST, O_RDWR | O_SYNC);    
    if (srom_no < 0) {
        printf("mknod mtd device\n");
        system("mknod /dev/mtd0  c 90 0");
//        system("mknod /dev/mtd1  c 90 2");
        srom_no = open(BOOTFLASH_MTD_TEST, O_RDWR | O_SYNC);     
        if (srom_no < 0) {
            cterr('f', 0, "Open MTD device failed");
            return (FAILED);
        }
    }

    switch (choice) {
    case 1:
        offset = gethex_answer("Please enter offset: ", 0, 1, 0x200000);
        nbytes = gethex_answer("Please enter read bytes: ", 0, 1, 0x20000);
        if (bootflash_read_sector(srom_no, offset, nbytes, read_buf) == FAILED) {
            cterr('f', 0, "fail occur at read bootflash sector");
            return (FAILED);
        }
        for (ix = 0; ix < nbytes; ix++) {
            if ((ix % 16) == 0)
                printf("\n0X%6X  ",offset + ix );
                printf("%2X ", read_buf[ix]);
        }
        break;
    case 2:
        srom_no1 = open(BOOTFLASH_MTD_TEST, O_RDWR | O_SYNC);    
        offset = gethex_answer("Please enter offset: ", 0, 1, 0x20000);
        nbytes = gethex_answer("Please enter read bytes: ", 0, 1, 0x20000);
        if (bootflash_read_sector(srom_no1, offset, nbytes, read_buf) == FAILED) {
            cterr('f', 0, "fail occur at read bootflash sector");
            return (FAILED);
        }
        for (ix = 0; ix < nbytes; ix++) {
            if ((ix % 16) == 0)
                printf("\n0X%6X  ", ix + offset + BOOTFLASH_TEST_LENGTH );
                printf("%2X ", read_buf[ix]);
        }
        break;
    case 3:
        srom_no1 = open(BOOTFLASH_MTD_TEST, O_RDWR | O_SYNC);    
        offset = gethex_answer("Please enter offset: ", 0, 1, 0x20000);
        nbytes = gethex_answer("Please enter write bytes: ", 1, 1, 0x20000);
	    value = gethex_answer("\nEnter test pattern:", 0, 0, 0xff);
        if (bootflash_read_sector(srom_no1, 0x0, SECTOR_SIZE, read_buf) == FAILED) {
            cterr('f', 0, "fail occur at read bootflash sector");
            return (FAILED);
        }
        /* Back up the flash data */
        memcpy(write_buf, read_buf, SECTOR_SIZE);
        /* Fill in write buffer with known pattern */
        for (ix = offset; ix < (offset + nbytes); ix++) {
            write_buf[ix] = value;
        }
        if ((NVRAM)->diagflag & D_VERBOSE) { 
            for (ix = 0; ix < 100; ix++) {
             if ((ix % 16) == 0)
                    printf("\n0X%6X  ", ix );
                    printf("%2X ", write_buf[ix]);
            }
        }

        if (bootflash_erase_sector(srom_no1, BOOTFLASH_TEST_SECTOR_OFFSET, 
                                   SECTOR_SIZE) == FAILED) {
            cterr('f', 0, "fail occur at erase bootflash sector");
            return (FAILED);
        }

        if (bootflash_write_sector(srom_no1, BOOTFLASH_TEST_SECTOR_OFFSET, 
                                   SECTOR_SIZE, write_buf) == FAILED) {
            cterr('f', 0, "fail occur at write bootflash sector");
            return (FAILED);
        }

        if ((NVRAM)->diagflag & D_VERBOSE) { 
            if (bootflash_read_sector(srom_no1, offset, nbytes, read_buf) == FAILED) {
                cterr('f', 0, "fail occur at read bootflash sector");
                return (FAILED);
            }
            for (ix = 0; ix < nbytes; ix++) {
                if ((ix % 16) == 0)
                    printf("\n0X%6X  ",offset + ix );
                   printf("%2X ", read_buf[ix]);
            }
        }
        break;
    case 4:
        retval = get_bootflash_info();
        break;
    default:
        printf("\n%s: Invalid choice (%d).\n", __FUNCTION__, choice);
        return (FAILED);
    }
    if (close(srom_no)) {
        printf("can't close /dev/mtd0");
        return (FAILED);
    }
    return (retval);
}

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
        prpass(testpass, "Erase sucess, erased %d bytes from address 0x%x",
               erase.length,erase.start);
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
int bootflash_write_sector (int fwh_fd, u_int32_t write_offset, 
                            u_int32_t write_length, u_int8_t *write_buf)
{
    int write_ret;
    int write_size = write_length*sizeof(u_int8_t);

    if (write_offset != lseek (fwh_fd,write_offset,SEEK_SET)) {
        cterr('f', 0, "lseek() can't find write_offset at %#.8x", write_offset);
        return (FAILED);
    }
       
    /* write data from file to flash */
    write_ret = write(fwh_fd, write_buf, write_size);

    if (write_ret < 0) {
        cterr('f', 0, "faile to write data from file to flash");
        return (FAILED);
    }
    
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
int bootflash_read_sector (int fwh_fd,  u_int32_t read_offset, 
                           u_int32_t read_length, u_int8_t *read_buf)
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
/*******************************************************************************
 *
 * Function    : bootflash_test
 * Description : Tests boot flash first sector ,saving the test sector data into 
 *               memory, writing pattern to the test sector, reading back, 
 *               verifying ,then writing back the original data 
 * Inputs      : menu_option - menu option passed from menu selection
 * Outputs    : PASSED - 0 - everything went well 
 *                  FAILED - 1 - there were some problem
 *
 ******************************************************************************/

 int bootflash_test (void)
 {
    int ix;
    int fwh_fd;
    uchar *mem_ptr=0; 


     /* read variables */
    u_int8_t *read_buf=NULL; 
    u_int32_t read_offset=BOOTFLASH_TEST_SECTOR_OFFSET;
    u_int32_t read_length=BOOTFLASH_TEST_LENGTH;
        
    /* erase variables*/
    u_int32_t erase_offset=BOOTFLASH_TEST_SECTOR_OFFSET;
    u_int32_t erase_length=BOOTFLASH_TEST_LENGTH;

    /* write variables */
    u_int8_t *write_buf=NULL;
    u_int32_t write_offset=BOOTFLASH_TEST_SECTOR_OFFSET;
    u_int32_t write_length=BOOTFLASH_TEST_LENGTH;
     
    /* allocate space for variable "read_buf" and "write_buf" and mem_ptr */
    read_buf = (uchar *)malloc(BOOTFLASH_TEST_LENGTH);
    write_buf = (uchar *)malloc(BOOTFLASH_TEST_LENGTH);
    mem_ptr = (uchar *)malloc(BOOTFLASH_TEST_LENGTH);
    
    /* check malloc is not "NULL" */
    if (read_buf == NULL) {
        cterr ('f', 0, "Fail to allocate space for read buffer");
        free (write_buf);
        free (mem_ptr);
        return (FAILED);
    }

    if (write_buf == NULL) {
        cterr ('f', 0, "Fail to allocate space for write buffer");
        free (read_buf);
        free (mem_ptr);
        return (FAILED);
    }
        
    if (mem_ptr == NULL) {
        cterr ('f', 0, "Fail to allocate space for mem_ptr");
        free (read_buf);
        free (write_buf);
        return (FAILED);
    }

    bzero((void *)read_buf, BOOTFLASH_TEST_LENGTH);
    bzero((void *)write_buf, BOOTFLASH_TEST_LENGTH);
    bzero((void *)mem_ptr, BOOTFLASH_TEST_LENGTH);

    printf("\nThe boot flash test at nnused sector 1:\n");   
    /* (step 1) Save original sector data to memory */
    /* Open MTD(mtd1) */
    fwh_fd = open(BOOTFLASH_MTD_TEST, O_RDWR | O_SYNC);    
    if (fwh_fd < 0) {
        system("mknod /dev/mtd1  c 90 2");
        fwh_fd = open(BOOTFLASH_MTD_TEST, O_RDWR | O_SYNC);    
        if (fwh_fd < 0) {
            cterr('f', 0, "Open MTD device failed");
            bootflash_free_memory(read_buf, write_buf, mem_ptr, fwh_fd);
            return (FAILED);
        }
    }
    
    
    prpass(testpass, "Boot flash save sector ");

    /* call "read" sector function */
    if (bootflash_read_sector(fwh_fd, read_offset, read_length, read_buf) == FAILED) {
        cterr('f', 0, "fail occur at read bootflash sector");
        bootflash_free_memory(read_buf, write_buf, mem_ptr, fwh_fd);
        return (FAILED);
    }
    
    /* write bootflash data into mem_ptr */
    memcpy(mem_ptr, read_buf, BOOTFLASH_TEST_LENGTH);
    
    /* End step 1*/
    
    /* (step 2) Erase bootflash sector */
    if ((NVRAM)->diagflag & D_VERBOSE) { 
        printf("\nNow erase the test sector at offset = %x", 
               BOOTFLASH_TEST_SECTOR_OFFSET);
    }

    prpass(testpass, "Boot flash erase sector ");

    /* call "erase" sector function */
    if (bootflash_erase_sector(fwh_fd, erase_offset, erase_length) == FAILED) {
        cterr('f', 0, "fail occur at erase bootflash sector");
        bootflash_free_memory(read_buf, write_buf, mem_ptr, fwh_fd);
        return (FAILED);
    }
    
    /* end step 2 */

    
    /* (step 3) Fill in write buffer with known pattern */
    if ((NVRAM)->diagflag & D_VERBOSE) { /* Turns debug on */
        printf("\nNow writing test pattern to sector at offset = %x"
               ,BOOTFLASH_TEST_SECTOR_OFFSET);
    }
    
    prpass(testpass, "Boot flash write sector\n");

    /* Fill in write buffer with known pattern */
    for (ix = 0; ix < write_length; ix++) {
        write_buf[ix] = ix;
    }
    /* call "write" function */
    if (bootflash_write_sector(fwh_fd, write_offset, write_length, write_buf) == FAILED) {
        cterr('f', 0, "fail occur at write bootflash sector");
        bootflash_free_memory(read_buf, write_buf, mem_ptr, fwh_fd);
        return (FAILED);
    }
    /* end step 3 */
    
    
    /* (step 4) Now read the test pattern data from sector and verify */
    
    /* call "read" sector function */
    bzero(read_buf, BOOTFLASH_TEST_LENGTH);
    if (bootflash_read_sector(fwh_fd, read_offset, read_length, read_buf) == FAILED) {
        cterr('f', 0, "fail occur at read bootflash sector");
        bootflash_free_memory(read_buf, write_buf, mem_ptr, fwh_fd);
        return (FAILED);
    } 

    /* verify data */
    for (ix = 0; ix < BOOTFLASH_TEST_LENGTH; ix++) {
        if (write_buf[ix] != read_buf[ix]) {
            cterr('f', 0, "Sector data read/verify failed at sector addr=%#x, "
                          "exp_data=%04x, act_data=%04x",
                           read_buf, *write_buf, *read_buf);
            bootflash_free_memory(read_buf, write_buf, mem_ptr, fwh_fd);
            return (FAILED);
        }
    } 

    if ((NVRAM)->diagflag & D_VERBOSE) { 
        for (ix = BOOTFLASH_TEST_LENGTH - 200; ix < BOOTFLASH_TEST_LENGTH; ix++) {
            if ((ix % 16) == 0)
                printf("\n0X%6X  ",read_offset + ix );
                printf("%2X ", read_buf[ix]);
        }
    }
    prpass(testpass, "\nsector at offset %x was written/read/verified successfully", 
                BOOTFLASH_TEST_SECTOR_OFFSET); 

    prpass(testpass, "Boot flash second erase sector");

    /* call "erase" sector function */
    if (bootflash_erase_sector(fwh_fd, erase_offset, erase_length)  == FAILED) {
        cterr('f', 0, "fail occur at erase bootflash sector");
        bootflash_free_memory(read_buf, write_buf, mem_ptr, fwh_fd);
        return (FAILED);
    }
    
    prpass(testpass, "Boot flash restore sector");

    /* write back original flash data */
    if (bootflash_write_sector(fwh_fd, write_offset, write_length, mem_ptr) == FAILED) {
        cterr('f', 0, "fail occur at write back origion data to bootflash sector");
        bootflash_free_memory(read_buf, write_buf, mem_ptr, fwh_fd);;
        return (FAILED);
    }

    prpass(testpass, "Boot flash finished!");

    bootflash_free_memory(read_buf, write_buf, mem_ptr, fwh_fd);

    return (PASSED);
 }
/*******************************************************************************
 *
 * Function    : gb_bootflash_test
 * Description : The first sector contains the BIOS descriptors and as such will 
 *				 always be identical no matter what image is installed as its 
 *				 write protected by the CPU.
 *			     Test boot flash : Saving the first flash data into memory.
 *               Switching to another flash, reading back and compare data .
 * Inputs      : menu_option - menu option passed from menu selection
 * Outputs    : PASSED - 0 - everything went well 
 *                  FAILED - 1 - there were some problem
 *
 ******************************************************************************/
int gb_bootflash_test (void)
 {
    int ix;
    int fwh_fd;
    uchar *mem_ptr=0; 

    testname("Boot Flash and MUX");

     /* read variables */
    u_int8_t *read_buf0=NULL; 
    u_int8_t *read_buf1=NULL;
    u_int32_t read_offset=BOOTFLASH_TEST_SECTOR_OFFSET;
    u_int32_t read_length=BOOTFLASH_TEST_LENGTH;
        
    /* allocate space for variable "read_buf" and "write_buf" and mem_ptr */
    read_buf0 = (uchar *)malloc(BOOTFLASH_TEST_LENGTH);
    read_buf1 = (uchar *)malloc(BOOTFLASH_TEST_LENGTH);
    mem_ptr = (uchar *)malloc(BOOTFLASH_TEST_LENGTH);
    
    /* check malloc is not "NULL" */
    if (read_buf0 == NULL) {
        cterr ('f', 0, "Fail to allocate space for read buffer");
        free (read_buf1);
        free (mem_ptr);
        return (FAILED);
    }

    bzero((void *)read_buf0, BOOTFLASH_TEST_LENGTH);
    bzero((void *)read_buf1, BOOTFLASH_TEST_LENGTH);
    bzero((void *)mem_ptr, BOOTFLASH_TEST_LENGTH);

    printf("\nThe boot flash test at sector 0:");   
    prpass(testpass, "Switch to flash 0 ");
    /* (step 1) Save PROM 0 data to memory */
    /* Select SPI PROM 0  */
    system("pp -l 0x58 0x80000000");
    msleep(500);
    /* Open MTD(mtd0) */
    fwh_fd = open(BOOTFLASH_MTD_TEST, O_RDWR | O_SYNC);    
    if (fwh_fd < 0) {
        system("mknod /dev/mtd0 c 90 0");
        fwh_fd = open(BOOTFLASH_MTD_TEST, O_RDWR | O_SYNC);    
        if (fwh_fd < 0) {
            cterr('f', 0, "Open MTD device failed");
            bootflash_free_memory(read_buf0, read_buf1, mem_ptr, fwh_fd);
            return (FAILED);
        }
    }
    
    prpass(testpass, "Read Boot flash 0 ");

    /* call "read" sector function */
    if (bootflash_read_sector(fwh_fd, read_offset, read_length, read_buf0) == FAILED) {
        cterr('f', 0, "fail occur at read bootflash sector");
        bootflash_free_memory(read_buf0, read_buf1, mem_ptr, fwh_fd);
        return (FAILED);
    }
    if (fwh_fd >= 0) {     
        close (fwh_fd);
    }
    prpass(testpass, "Switch to flash 1 ");
    /* (step 2) Save PROM 1 data to memory */
    /* Select SPI PROM 1  */
    system("pp -l 0x58 0xC0000000");
    msleep(500);
    /* Open MTD(mtd0) */
    fwh_fd = open(BOOTFLASH_MTD_TEST, O_RDWR | O_SYNC);    
    if (fwh_fd < 0) {
        system("mknod /dev/mtd0 c 90 0");
        fwh_fd = open(BOOTFLASH_MTD_TEST, O_RDWR | O_SYNC);    
        if (fwh_fd < 0) {
            cterr('f', 0, "Open MTD device failed");
            bootflash_free_memory(read_buf0, read_buf1, mem_ptr, fwh_fd);
            return (FAILED);
        }
    }
    prpass(testpass, "Read Boot flash 1 ");
    /* call "read" sector function */
    if (bootflash_read_sector(fwh_fd, read_offset, read_length, read_buf1) == FAILED) {
        cterr('f', 0, "fail occur at read bootflash sector");
        bootflash_free_memory(read_buf0, read_buf1, mem_ptr, fwh_fd);
        return (FAILED);
    }
    
    prpass(testpass, "Compare Boot Flash ");

    /* verify data */
    for (ix = 0; ix < BOOTFLASH_TEST_LENGTH; ix++) {
        if (read_buf0[ix] != read_buf1[ix]) {
            cterr('f', 0, "Sector data read/verify failed at sector addr=%#x, "
                          "exp_data=%04x, act_data=%04x",
                           read_buf0, *read_buf0, *read_buf1);
            bootflash_free_memory(read_buf0, read_buf1, mem_ptr, fwh_fd);
            return (FAILED);
        }
    } 

    if ((NVRAM)->diagflag & D_VERBOSE) { 
        for (ix = 0; ix < 200; ix++) {
            if ((ix % 16) == 0)
                printf("\n0X%6X  ",read_offset + ix );
                printf("%2X ", read_buf0[ix]);
        }
        for (ix = BOOTFLASH_TEST_LENGTH - 200; ix < BOOTFLASH_TEST_LENGTH; ix++) {
            if ((ix % 16) == 0)
                printf("\n0X%6X  ",read_offset + ix );
                printf("%2X ", read_buf0[ix]);
        }
    }

    prpass(testpass, "Boot flash test completed. ");

    bootflash_free_memory(read_buf0, read_buf1, mem_ptr, fwh_fd);
    return (PASSED); }

/******** History ********/
/*
 *------------------------------------------------------------------
 * $Log: bootflash_test.c,v $
 * Revision 1.1  2016/10/16 12:28:22  iachang
 * Supported Goldbeach Platform.
 *
 *
 *------------------------------------------------------------------
 * $Endlog$
 */
