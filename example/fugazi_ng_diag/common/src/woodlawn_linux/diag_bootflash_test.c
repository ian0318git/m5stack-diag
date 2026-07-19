/* $Id: diag_bootflash_test.c,v 1.2 2013/10/08 08:48:27 tirawan Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/woodlawn_linux/diag_bootflash_test.c,v $
 *-----------------------------------------------------------------------------
 * diag_bootflash_test.c - bootflash test function for Woodlawn
 *
 * February 2012, Leslie Chen
 * Copyright (c) 2013 by Cisco Systems, Inc.
 * All rights reserved.
 *-----------------------------------------------------------------------------
 */


#include "proto.h"
#include "string.h"
#include "menu.h"
#include "types.h"
#include "common.h"
#include "diag_bootflash_lib.h"

#include <stdio.h>
#include <stdlib.h>
#include "nvsysvars.h"
#include "error.h"

/* below include file declare for MTD */
#include <fcntl.h>
#include <asm/ioctl.h>
#include <sys/stat.h>
#include <signal.h>
#include <mtd/mtd-user.h>
#include <sys/ioctl.h>
#include <errno.h>
#include "queryflags.h"

int bootflash_test(void);
int get_bootflash_info(void);
int bootflash_otp_test(void);

/*******************************************************************************
 *
 * Function    : bootflash_test
 * Description : Tests boot flash sector at offset BOOTFLASH_TEST_SECTOR_OFFSET 
 *               by saving the test sector data into memory, writing known
 *               pattern to the test sector, reading back, verifying them,
 *               then writing back the original data back to the sector
 * Inputs      : menu_option - menu option passed from menu selection
 * Outputs    : PASSED - 0 - everything went well 
 *                  FAILED - 1 - there were some problem
 *
 * Notice   : We can use command "cat /proc/mtd" to get the information of each mtd.
                  The related information include dev, size, erase size and name.

                  dev         size                erasesize          name
                  mtd0       00210000        00010000         "Uboot"
                  mtd1       00010000        00010000         "Diag R/W Test"
                  mtd2       005cc000         00010000         "Reserved"
                  mtd3       00002000        00002000         "Environment Variables"
                  mtd4       100000000      00080000         "octeon_nand0"
               
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
        return (FAILED);
    }

    if (write_buf == NULL) {
        cterr ('f', 0, "Fail to allocate space for write buffer");
        return (FAILED);
    }
        
    if (mem_ptr == NULL) {
        cterr ('f', 0, "Fail to allocate space for mem_ptr");
        return (FAILED);
    }

    bzero((void *)read_buf, BOOTFLASH_TEST_LENGTH);
    bzero((void *)write_buf, BOOTFLASH_TEST_LENGTH);
    bzero((void *)mem_ptr, BOOTFLASH_TEST_LENGTH);
    
    /* (step 1) Save original sector data to memory */
    /* Open MTD(mtd1) */
    fwh_fd = open(BOOTFLASH_MTD_TEST, O_RDWR | O_SYNC);    
    if (fwh_fd < 0) {
        cterr('f', 0, "Open MTD device failed");
        bootflash_free_memory(read_buf, write_buf, mem_ptr, fwh_fd);
        return (FAILED);
    }
    
    prpass(testpass, "Boot flash save sector");

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

    prpass(testpass, "Boot flash erase sector");

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
    printf("Device ID = %x\n", *(bootflash_offset + 0x1));
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

/*******************************************************************************
 *
 * Function    : bootflash_otp_test
 * Description : Bootflash OTP test - If PPB bit is 1, sector is unprotected,       
 *               if PPB bit is 0, sector is protected. At first check all the  
 *               sectors PPB status and return fail when sector is unprotected.  
 *               If all the sectors are protected then will perform r/w test 
 *               on sector 0x3e0000.
 *
 * Inputs      : none
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/
int bootflash_otp_test (void)
{
    printf("Bootflash OTP Test\n");

    int ix, wr_count;
    uint16_t *bootflash_offset, *sec_ptr = 0;
    uint16_t *mem_ptr = 0, *save_data; 

    /* allocate space for variable mem_ptr, save_data */
    save_data = (uint16_t *)malloc(OTP_TEST_LENGTH);

    /* check malloc is not "NULL" */
    if (save_data == NULL) {
        printf("Fail to allocate space for read buffer\n");
        return (FAILED);
    }

    bzero((void *)save_data, OTP_TEST_LENGTH);

    /* use mmap to get virtual addr */
    bootflash_offset = bootflash_mmap_get_addr();

    if (bootflash_offset == NULL) {
        free(save_data);
        return (FAILED);
    }

    *bootflash_offset = 0xf0;
    /* PPB command set entry */
    *(bootflash_offset + PPB_ENTRY_CMD1) = PPB_ENTRY_DATA1;
    *(bootflash_offset + PPB_ENTRY_CMD2) = PPB_ENTRY_DATA2;
    *(bootflash_offset + PPB_ENTRY_CMD3) = PPB_ENTRY_DATA3;

    /* PPB status read to judge sector is protect or not */
    for (ix = 0; ix < TOTAL_OTP_SIZE/2; ix += (SECTOR_SIZE/2)) {
        bootflash_offset += (uint16_t)ix;

        if (*bootflash_offset == 1) {
            printf("***Sector-%x is unprotected - PPB status = %x***\n", ix, *bootflash_offset);
            /* PPB command set exit */
            *bootflash_offset = PPB_EXIT_DATA1;
            *bootflash_offset = PPB_EXIT_DATA2;

            free(save_data);
            return (FAILED);
        }
    } 

    /* PPB command set exit */
    *bootflash_offset = PPB_EXIT_DATA1;
    *bootflash_offset = PPB_EXIT_DATA2;

    *bootflash_offset = 0xf0;
    /* (step 1) Save original sector data to memory */
    sec_ptr = bootflash_offset + BOOTFLASH_TEST_ADDR/2;
    mem_ptr = save_data;

    for (ix = 0; ix < OTP_TEST_LENGTH/2; ix++) {
        *mem_ptr++ = *sec_ptr++; 
    }

    /* (step 2) Erase bootflash sector */
    *(bootflash_offset + FLASH_ERASE_CMD_ADDR1) = FLASH_ERASE_DATA1;
    *(bootflash_offset + FLASH_ERASE_CMD_ADDR2) = FLASH_ERASE_DATA2;
    *(bootflash_offset + FLASH_ERASE_CMD_ADDR1) = FLASH_ERASE_DATA3; 
    *(bootflash_offset + FLASH_ERASE_CMD_ADDR1) = FLASH_ERASE_DATA1;
    *(bootflash_offset + FLASH_ERASE_CMD_ADDR2) = FLASH_ERASE_DATA2;
    *(bootflash_offset + BOOTFLASH_TEST_ADDR/2) = FLASH_ERASE_DATA4; 
    msleep(1000);    

    /* (step 3) Write data with known patten */
    sec_ptr = bootflash_offset + BOOTFLASH_TEST_ADDR/2;
    for (ix = 0; ix < OTP_TEST_LENGTH/2; ix++) {
        *(bootflash_offset + FLASH_PROGRAM_CMD_ADDR1) = FLASH_PROGRAM_DATA1;
        *(bootflash_offset + FLASH_PROGRAM_CMD_ADDR2) = FLASH_PROGRAM_DATA2;
        *(bootflash_offset + FLASH_PROGRAM_CMD_ADDR3) = FLASH_PROGRAM_DATA3;
        *sec_ptr++ = ix;
    }
    msleep(1000);    
    /* (step 4) Read data from sector and verify */
    sec_ptr = bootflash_offset + BOOTFLASH_TEST_ADDR/2;
    mem_ptr = save_data;
    for (ix = 0; ix < OTP_TEST_LENGTH/2; ix++) {
        if (*mem_ptr != *sec_ptr) {
            printf("Data verify failed at addr=%#x, exp_data=%04x, act_data=%04x\n", 
                   ix, *mem_ptr, *sec_ptr);

            /* Recover original data */
            sec_ptr = bootflash_offset + BOOTFLASH_TEST_ADDR/2;
            mem_ptr = save_data;
            for (wr_count = 0; wr_count < OTP_TEST_LENGTH/2; wr_count++) {
                *(bootflash_offset + FLASH_PROGRAM_CMD_ADDR1) = FLASH_PROGRAM_DATA1;
                *(bootflash_offset + FLASH_PROGRAM_CMD_ADDR2) = FLASH_PROGRAM_DATA2;
                *(bootflash_offset + FLASH_PROGRAM_CMD_ADDR3) = FLASH_PROGRAM_DATA3;
                *sec_ptr++ = *mem_ptr++;
            }

            free(save_data);
            return (FAILED);
        }
        mem_ptr++; sec_ptr++;
    }

    printf("\nsector at offset %x is protected and verified successfully", BOOTFLASH_TEST_ADDR); 
            
    free(save_data);
    return (PASSED);    
}
/*-------------------------------------------------
 * $Log: diag_bootflash_test.c,v $
 * Revision 1.2  2013/10/08 08:48:27  tirawan
 * Woodlawn collapsed to main trunk
 *
 * Revision 1.1.4.3  2013/09/05 13:25:40  leschen
 * Modify OTP utility.
 *
 * Revision 1.1.4.2  2013/08/20 10:58:50  tirawan
 * Branch into woodlawn-branch2 and port woodlawn code
 *
 * Revision 1.1.2.2  2013/07/02 07:33:26  leschen
 * Add function to get flash information and perform OTP test
 *
 * Revision 1.1.2.1  2013/04/24 10:37:14  tirawan
 * Initial check-in for woodlawn linux code
 *
 * Revision 1.2  2013/03/27 04:49:35  kuangik
 * Code cleanup after adding -Wall
 *
 * Revision 1.9  2012/10/24 06:45:45  leslie
 * Fix bootflash test warning message.
 *
 * Revision 1.8  2012/08/03 10:16:55  evanli
 * Mapping to latest O2 source code on 20120726
 *
 * Revision 1.6  2012/07/19 06:08:12  leslie
 * Use prpass and cterr to show message
 *
 * Revision 1.5  2012/05/30 01:07:12  leslie
 * Open partition /dev/mtd1 instead of open /dev/mtd0
 *
 * Revision 1.4  2012/05/18 03:21:38  leslie
 * Specify the certain region to do bootflash r/w test
 *
 * Revision 1.3  2012/03/26 07:17:36  kody
 * Add stdio.h
 *
 * Revision 1.2  2012/02/13 03:30:53  leslie
 * Add function prototype.
 *
 * Revision 1.1  2012/02/10 06:18:39  leslie
 * Add Woodlawn bootflash test.
 *
 *
 * $Endlog $
 *-------------------------------------------------
 */
