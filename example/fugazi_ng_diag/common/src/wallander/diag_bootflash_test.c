/* $Id: diag_bootflash_test.c,v 1.2 2015/03/13 07:18:26 xiaoyizh Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/wallander/diag_bootflash_test.c,v $
 *-----------------------------------------------------------------------------
 * diag_bootflash_test.c - bootflash test function for Wallander
 *
 * Xiaoying Zhang -- Feb. 2014
 *
 * Copyright (c) 2015 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#include "proto.h"
#include "string.h"
#include "menu.h"
#include "types.h"
#include "common.h"
#include "diag_bootflash_lib.h"

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "nvsysvars.h"
// #include "nvmonvars.h"
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


/*******************************************************************************
 *
 * Function    : bootflash_status_get
 * Description : Determines Flash operation status .
 *
 * Inputs      : bootflash_offset - bootflash base address
 *               sector_offset - sector address
 * Outputs     : None
 *
 *****************************************************************************/
DEVSTATUS bootflash_status_get (uint8_t *bootflash_offset, uint8_t *sector_offset)
{
    uint8_t dq2_toggles;
    uint8_t dq6_toggles;
    uint8_t status_read_1;
    uint8_t status_read_2;
    uint8_t status_read_3;

    assert((bootflash_offset != NULL) && (sector_offset != NULL));

    status_read_1 = *sector_offset;
    status_read_2 = *sector_offset;
    status_read_3 = *sector_offset;

    /* Any DQ6 toggles */
    dq6_toggles = ((status_read_1 ^ status_read_2) & 
                   (status_read_2 ^ status_read_3) & 
                   DQ6_MASK );

    /* Any DQ2 toggles */
    dq2_toggles = ((status_read_1 ^ status_read_2) & 
                   (status_read_2 ^ status_read_3) & 
                   DQ2_MASK );

    if (dq6_toggles)
    {
        /* Checking WriteBuffer Abort condition: 
           Check for all devices that have DQ6 toggling also have Write Buffer Abort DQ1 set */
        if ((!dq2_toggles) && 
            ((DQ6_TGL_DQ1_MASK & status_read_1) == DQ6_TGL_DQ1_MASK)
           ) {
            return DEV_WRITE_BUFFER_ABORT;
        }

        /* Checking Timeout condition: 
           Check for all devices that have DQ6 toggling also have Time Out DQ5 set. */
        if ((DQ6_TGL_DQ5_MASK & status_read_1) == DQ6_TGL_DQ5_MASK ) {
           return DEV_EXCEEDED_TIME_LIMITS; 
        }

        /* No timeout, no WB error */
        return DEV_BUSY;
    } else   /* no DQ6 toggles on all devices */ {
        /* Checking Erase Suspend condition */
        status_read_1 = *sector_offset;
        status_read_2 = *sector_offset;

        /* Checking Erase Suspend condition */
        if (((status_read_1 ^ status_read_2) & DQ2_MASK) == 0 ) {
            return DEV_NOT_BUSY;         /* All devices DQ2 not toggling */
        } else {                           // at least one device is suspended
           return DEV_SUSPEND;          /* At least some devices toggle DQ2 */      
        }
    }
}


/*******************************************************************************
 *
 * Function    : bootflash_ppb_entry_cmd
 * Description : Non-Volatile Sector Protection Entry Command. 
 *               Ppb entry command will disable the reads and writes for the bank selectd.
 *
 * Inputs      : bootflash_offset - bootflash base address
 *               sector_idx - sector index
 * Outputs     : None
 *
 *****************************************************************************/
void bootflash_ppb_entry_cmd (uint8_t *bootflash_offset, uint8_t *sector_offset)
{
    assert((bootflash_offset != NULL) && (sector_offset != NULL) );

    /* PPB command set entry */
    *(bootflash_offset + FLASH_CMD_UNLOCK_ADDR1) = FLASH_PPB_ENTRY_DATA1;
    *(bootflash_offset + FLASH_CMD_UNLOCK_ADDR2) = FLASH_PPB_ENTRY_DATA2;
    *(sector_offset + FLASH_CMD_UNLOCK_ADDR1) =    FLASH_PPB_ENTRY_DATA3;
}

/*******************************************************************************
 *
 * Function    : bootflash_ppb_program_cmd
 * Description : Program Non-Volatile Sector Protection Command.
 *               Need to issue lld_PpbEntryCmd() before issue this routine.
 *
 * Inputs      : bootflash_offset - bootflash base address
 *               sector_idx - sector index
 * Outputs     : None
 *
 *****************************************************************************/
void bootflash_ppb_program_cmd (uint8_t *bootflash_offset, uint8_t *sector_offset)
{
    assert((bootflash_offset != NULL) && (sector_offset != NULL));

    /* Program PPB bit */
    *bootflash_offset = FLASH_PPB_PROGRAM_DATA1;
    *sector_offset    = FLASH_PPB_PROGRAM_LOCK;
}

/*******************************************************************************
 *
 * Function    : bootflash_ppb_all_erase_cmd
 * Description : Erase Non-Volatile Protection for All  Sectors Command.
 *               Need to issue lld_PpbEntryCmd() before issue this routine.
 *
 * Inputs      : bootflash_offset - bootflash base address
 * Outputs     : None
 *
 *****************************************************************************/
void bootflash_ppb_all_erase_cmd (uint8_t *bootflash_offset)
{
    assert(bootflash_offset != NULL);

    /* Erase all PPB bits */
    *bootflash_offset = FLASH_PPB_ALL_ERASE_DATA1;
    *bootflash_offset = FLASH_PPB_ALL_ERASE_DATA2;
}

/*******************************************************************************
 *
 * Function    : bootflash_ppb_exit_cmd
 * Description : Exit the Non-Volatile Sector Status mode. 
 *               After the exit command the device goes back to memory array mode.
 *
 * Inputs      : bootflash_offset - bootflash base address
 * Outputs     : None
 *
 *****************************************************************************/
void bootflash_ppb_exit_cmd (uint8_t *bootflash_offset)
{
    assert(bootflash_offset != NULL);

    /* PPB command set exit */
    *bootflash_offset = FLASH_PPB_EXIT_DATA1;
    *bootflash_offset = FLASH_PPB_EXIT_DATA2;
}

/*******************************************************************************
 *
 * Function    : bootflash_ppb_status_read
 * Description : Read Non-Volatile Sector Status.
 *
 * Inputs      : bootflash_offset - base address for flash
 *               sector_offset - base address for specified sector
 * Outputs     : Status : 0 - Locked; 1 - Unlocked.
 *
 *****************************************************************************/
uint8_t bootflash_ppb_status_read (uint8_t *bootflash_offset, uint8_t *sector_offset)
{
    uint8_t status;

    assert((bootflash_offset != NULL) && (sector_offset != NULL) );

    bootflash_ppb_entry_cmd(bootflash_offset, sector_offset);

    status = *sector_offset;

    bootflash_ppb_exit_cmd(bootflash_offset);

    return (status);
}


int bootflash_all_ppb_earse (void)
{
    uint8_t *bootflash_offset;
    DEVSTATUS    dev_status = DEV_STATUS_UNKNOWN;
    int polling_counter = 0xFFFFFFFF;
    int rc = PASSED;

    printf("Bootflash All PPB Erase\n");

    /* use mmap to get virtual addr */
    bootflash_offset = bootflash_mmap_get_addr(0);

    if (bootflash_offset == NULL) {
        return (FAILED);
    }

    /* PPB command set entry */
    bootflash_ppb_entry_cmd(bootflash_offset, bootflash_offset);

    /* PPB program to unprotected state */
    bootflash_ppb_all_erase_cmd(bootflash_offset);

    /* poll for completion */
    do {
        polling_counter--;

        dev_status = bootflash_status_get(bootflash_offset, bootflash_offset);

    } while ((dev_status == DEV_BUSY) && polling_counter);

    /* if not done, then we have an error */
    if (dev_status != DEV_NOT_BUSY)
    {
        /* Write Software RESET command */
        *bootflash_offset = 0xf0;
        printf("Failed to erase all PPB bits.\n");
        rc = FAILED;
    }

    bootflash_ppb_exit_cmd(bootflash_offset);

    return (rc);
}


/*******************************************************************************
 *
 * Function    : get_bootflash_info
 * Description : Display the Manufacturer ID, device ID info of the Flash,
 *               as long as the protect status of all sectors. 
 *
 * Inputs      : none
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/
int get_bootflash_info (void)
{
    uint8_t *bootflash_offset; 
    int ix;

    printf("Get Bootflash Information\n");

    bootflash_offset = bootflash_mmap_get_addr(0);

    if (bootflash_offset == NULL) {
        return (FAILED);
    }

    *(bootflash_offset + FLASH_CMD_UNLOCK_ADDR1) = FLASH_INFO_DATA1;
    *(bootflash_offset + FLASH_CMD_UNLOCK_ADDR2) = FLASH_INFO_DATA2;
    *(bootflash_offset + FLASH_CMD_UNLOCK_ADDR1) = FLASH_INFO_DATA3;

    printf("Manufacturer ID = %x\n", *(bootflash_offset + 0x0)); 
    printf("Device ID = %x\n", *(bootflash_offset + 0x2));
    printf("Device ID2 = %x\n", *(bootflash_offset + 0x1c));
    printf("Device ID3 = %x\n", *(bootflash_offset + 0x1e));
    printf("Sector Protect Verify = %x\n", *(bootflash_offset + 0x4));
    printf("Secure Device Verify = %x\n", *(bootflash_offset + 0x6));

    *bootflash_offset = 0xf0;

    /* PPB status read to judge sector is protect or not */
    printf("Sector Index\t\t U - Unlocked    L - Locked\n");

    for (ix = 0; ix < SECTOR_NUM; ix++) {
        uint8_t status;
        uint8_t *sector_offset;
        sector_offset = bootflash_mmap_get_addr(ix);

        if (sector_offset == NULL) {
            return (FAILED);
        }

        status = bootflash_ppb_status_read(bootflash_offset, sector_offset);

        if ((ix & 0xF) == 0) {
            printf ("  0x%x:\t", ix);
        }
        if (status == 1) {
            printf("U    ");
        } else {
            printf("L    ");
        }
        if ((ix & 0xF) == 0xF) {
            printf ("\n");
        }
    }
    printf ("\n");

    return (PASSED);
}
/*******************************************************************************
 *
 * Function    : bootflash_ppb_lock_sector
 * Description : bootflash_ppb_lock_sector 
 *               Program PPB bit for the sector.
 *
 * Inputs      : sector_idx - sector index
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/
int bootflash_ppb_lock_sector (int sector_idx)
{
    uint8_t *bootflash_offset;
    uint8_t *sector_offset;
    DEVSTATUS    dev_status = DEV_STATUS_UNKNOWN;
    int polling_counter = 0xFFFFFFFF;
    int rc = PASSED;

    /* use mmap to get virtual addr */
    bootflash_offset = bootflash_mmap_get_addr(0);

    if (bootflash_offset == NULL) {
        return (FAILED);
    }

    sector_offset = bootflash_mmap_get_addr(sector_idx);

    if (sector_offset == NULL) {
        return (FAILED);
    }

    printf("Lock sector %d\n", sector_idx);

    /* PPB command set entry */
    bootflash_ppb_entry_cmd(bootflash_offset, sector_offset);

    /* Program PPB bit */
    bootflash_ppb_program_cmd(bootflash_offset, sector_offset);

    /* poll for completion */
    do {
        polling_counter--;

        dev_status = bootflash_status_get(bootflash_offset, bootflash_offset);
    } while ((dev_status == DEV_BUSY) && polling_counter);

    /* if not done, then we have an error */
    if (dev_status != DEV_NOT_BUSY)
    {
        /* Write Software RESET command */
        *bootflash_offset = 0xf0;
        printf("Failed to program PPB bits.\n");
        rc = FAILED;
    }

    bootflash_ppb_exit_cmd(bootflash_offset);

    return (rc);

}
int bootflash_ppb_lock_sector_legacy (int sector_idx)
{
    uint8_t *bootflash_offset;
    uint8_t *sector_ptr;
    uint8_t ppb_state1;
    uint8_t ppb_state2;
    uint8_t ppb_state3;
    uint8_t ppb_state4;
    int retries = 10;
    int rc;

    /* use mmap to get virtual addr */
    bootflash_offset = bootflash_mmap_get_addr(0);

    if (bootflash_offset == NULL) {
        return (FAILED);
    }

    sector_ptr = bootflash_offset + (SECTOR_SIZE/2) * sector_idx;
    printf("Lock sector %d, virtual address 0x%x\n", 
        sector_idx, sector_ptr);

    /* PPB command set entry */
    *(bootflash_offset + FLASH_CMD_UNLOCK_ADDR1) = FLASH_PPB_ENTRY_DATA1;
    *(bootflash_offset + FLASH_CMD_UNLOCK_ADDR2) = FLASH_PPB_ENTRY_DATA2;
    *(sector_ptr + FLASH_CMD_UNLOCK_ADDR1) = FLASH_PPB_ENTRY_DATA3;

    printf("Program PPB bit\n");
    /* PPB program to protected state */
    *sector_ptr = FLASH_PPB_PROGRAM_DATA1;
    *sector_ptr = FLASH_PPB_PROGRAM_LOCK;

    while (retries--) {
        /* Read Sector 0 PPB state twice and check if DQ6 is toggled */
        ppb_state1 = *bootflash_offset;
        ppb_state2 = *bootflash_offset;
        printf("ppb_state1 = 0x%x ppb_state2 = 0x%x\n", ppb_state1, ppb_state2);

        if ((ppb_state1 & 0x40) ^ (ppb_state1 & 0x40)) {
            /* DQ6 Toggled */
            if (ppb_state1 & 0x20) {
                /* DQ5 == 1 */
                /* Read PPB state twice and check if DQ6 is toggled */
                ppb_state3 = *bootflash_offset;
                ppb_state4 = *bootflash_offset;
                printf("ppb_state3 = 0x%x ppb_state4 = 0x%x\n", 
                ppb_state3, ppb_state4);

                if ((ppb_state3 & 0x40) ^ (ppb_state4 & 0x40)) {
                    /* DQ6 Toggled */
                    /* Lock failed and issue reset command */
                    printf("Lock failed.\n");
                    *bootflash_offset = 0xf0;
                    rc = FAILED;
                } else if (*sector_ptr == 0) {
                    printf("Lock sucessfully.\n");
                    rc = PASSED;
                } else {
                    /* Lock failed and issue reset command */
                    printf("Lock failed.\n");
                    *bootflash_offset = 0xf0;
                    rc = FAILED;
                }
                break;
            } else {
                continue;
            }
        } else {
            msleep(1);
            if (*sector_ptr == 0) {
                printf("Lock sucessfully.\n");
                rc = PASSED;
            } else {
                /* Lock failed and issue reset command */
                printf("Lock failed.\n");
                *bootflash_offset = 0xf0;
                rc = FAILED;
            }
            break;
        }
    }

    if (retries == 0) {
        /* Lock failed and issue reset command */
        printf("Lock failed.\n");
        *bootflash_offset = 0xf0;
        rc = FAILED;
    }

    /* PPB command set exit */
    *bootflash_offset = FLASH_PPB_EXIT_DATA1;
    *bootflash_offset = FLASH_PPB_EXIT_DATA2;

    /* Read Sector Protect Verify to check the state */
    *(bootflash_offset + FLASH_CMD_UNLOCK_ADDR1) = FLASH_INFO_DATA1;
    *(bootflash_offset + FLASH_CMD_UNLOCK_ADDR2) = FLASH_INFO_DATA2;
    *(bootflash_offset + FLASH_CMD_UNLOCK_ADDR1) = FLASH_INFO_DATA3;
    printf("Sector Protect Verify = %x\n", *(sector_ptr + 0x4));

    *bootflash_offset = 0xf0;

    return (rc);
}

/*******************************************************************************
 *
 * Function    : bootflash_lock_golden
 * Description : bootflash_lock_golden 
 *               Lock sectors for Golden images.
 *
 * Inputs      : none
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/
int bootflash_lock_golden (void)
{
    int i;

    /* Lock the first 4M of Flash */
    for (i = 0; i < SECTOR_GOLDEN; i++) {
        if (bootflash_ppb_lock_sector(i)) {
            printf("Failed to lock sector %d.\n", i);
            return (FAILED);
        }
        msleep(500);
    }
    return (PASSED);
}


/*******************************************************************************
 *
 * Function    : bootflash_golden_lock_test
 * Description : If PPB bit is 1, sector is unprotected,
 *               if PPB bit is 0, sector is protected. 
 *               At first check all the golden(4M) sectors PPB status and 
 *               return fail when sector is unprotected.
 *               If all the sectors are protected then will perform r/w test
 *               on sector 31.
 *
 * Inputs      : none
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/
int bootflash_golden_lock_test (void)
{
    printf("Bootflash Golden Lock Test\n");

    int ix, wr_count;
    uint8_t *bootflash_offset;
    uint8_t *sector_offset;
    uint8_t *sec_ptr = 0;
    uint8_t *mem_ptr = 0, *save_data; 

    /* allocate space for variable mem_ptr, save_data */
    save_data = (uint8_t *)malloc(SECTOR_SIZE);

    /* check malloc is not "NULL" */
    if (save_data == NULL) {
        printf("Fail to allocate space for read buffer\n");
        return (FAILED);
    }

    bzero((void *)save_data, SECTOR_SIZE);

    /* use mmap to get virtual addr */
    bootflash_offset = bootflash_mmap_get_addr(0);

    if (bootflash_offset == NULL) {
        free(save_data);
        return (FAILED);
    }

    for (ix = 0; ix < SECTOR_GOLDEN; ix++) {
        uint8_t status;
        sector_offset = bootflash_mmap_get_addr(ix);

        if (sector_offset == NULL) {
            return (FAILED);
        }

        status = bootflash_ppb_status_read(bootflash_offset, sector_offset);

        if (status != 0) {
            printf("***Sector-%d is unprotected - PPB status = %x***\n", ix, status);
            free(save_data);
            return (FAILED);
        }
        printf("***Sector-%d is protected ***\n", ix);
    }
    /* (step 1) Save original sector data to memory */
    sector_offset = bootflash_mmap_get_addr(SECTOR_GOLDEN - 1);
    sec_ptr = sector_offset;
    mem_ptr = save_data;

    for (ix = 0; ix < SECTOR_SIZE; ix++) {
        *mem_ptr++ = *sec_ptr++; 
    }

    /* (step 2) Erase bootflash sector */
    *(bootflash_offset + FLASH_CMD_UNLOCK_ADDR1) = FLASH_ERASE_DATA1;
    *(bootflash_offset + FLASH_CMD_UNLOCK_ADDR2) = FLASH_ERASE_DATA2;
    *(bootflash_offset + FLASH_CMD_UNLOCK_ADDR1) = FLASH_ERASE_DATA3; 
    *(bootflash_offset + FLASH_CMD_UNLOCK_ADDR1) = FLASH_ERASE_DATA1;
    *(bootflash_offset + FLASH_CMD_UNLOCK_ADDR2) = FLASH_ERASE_DATA2;
    *(sector_offset) = FLASH_ERASE_DATA4; 
    msleep(1000);

    /* (step 3) Write data with known patten */
    sec_ptr = sector_offset;
    for (ix = 0; ix < SECTOR_SIZE; ix++) {
        *(bootflash_offset + FLASH_CMD_UNLOCK_ADDR1) = FLASH_PROGRAM_DATA1;
        *(bootflash_offset + FLASH_CMD_UNLOCK_ADDR2) = FLASH_PROGRAM_DATA2;
        *(bootflash_offset + FLASH_CMD_UNLOCK_ADDR1) = FLASH_PROGRAM_DATA3;
        *sec_ptr++ = ix;
    }
    msleep(1000);

    /* (step 4) Read data from sector and verify */
    sec_ptr = sector_offset;
    mem_ptr = save_data;
    for (ix = 0; ix < SECTOR_SIZE; ix++) {
        if (*mem_ptr != *sec_ptr) {
            printf("Data verify failed at addr=%#x, exp_data=%04x, act_data=%04x\n", 
                   ix, *mem_ptr, *sec_ptr);

            /* Recover original data */
            sec_ptr = sector_offset;
            mem_ptr = save_data;
            for (wr_count = 0; wr_count < SECTOR_SIZE; wr_count++) {
                *(bootflash_offset + FLASH_CMD_UNLOCK_ADDR1) = FLASH_PROGRAM_DATA1;
                *(bootflash_offset + FLASH_CMD_UNLOCK_ADDR2) = FLASH_PROGRAM_DATA2;
                *(bootflash_offset + FLASH_CMD_UNLOCK_ADDR1) = FLASH_PROGRAM_DATA3;
                *sec_ptr++ = *mem_ptr++;
            }

            free(save_data);
            return (FAILED);
        }
        mem_ptr++; sec_ptr++;
    }

    printf("\nsector %x is protected and verified successfully\n", SECTOR_GOLDEN - 1); 

    free(save_data);
    return (PASSED);
}


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
                  mtd0       01000000        00020000         "phys_mapped_flash"

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
    /* Open MTD(mtd0) */
    fwh_fd = open(BOOTFLASH_MTD_TEST, O_RDWR/* | O_SYNC*/);
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

/*-------------------------------------------------
 * $Log: diag_bootflash_test.c,v $
 * Revision 1.2  2015/03/13 07:18:26  xiaoyizh
 * Clean up and increase the polling counter for getting the flash status.
 *
 * Revision 1.1  2015/02/26 07:18:29  xiaoyizh
 * Initial check in for Wallander.
 *
 *
 * $Endlog$
 *-------------------------------------------------
 */
