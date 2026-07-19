/* $Id: platform_prom.c,v 1.2 2019/06/14 05:24:51 mikech2 Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/katar/x86/platform_prom.c,v $
 *------------------------------------------------------------------
 * 
 * Filename   : katar_platform_prom.c 
 * Description: .
 *
 * Copyright (c) 2017 - 2019 by Cisco Systems, Inc.
 * All rights reserved.
 *------------------------------------------------------------------
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <error.h>
#include <sys/stat.h>
#include "common.h"
#include "platform_fpga.h"
#include "proto.h"
#include "linux_api.h"
#include "nvmonvars.h"
#include "queryflags.h"

#define SPI_PROM_MAX_WAIT			2500
#define SPI_PROM_ERASE_WAIT			20000

/* Bit definitions for SPI PROM Opcode */
#define PROM_WREN_OP_CODE			0x06   /* Write Enable  */
#define PROM_WRDI_OP_CODE			0x04   /* Write Disable */
#define PROM_RDID_OP_CODE			0x9F   /* Read Identification */
#define PROM_RDSR_OP_CODE			0x05   /* Read Status Register */
#define PROM_WRSR_OP_CODE			0x01   /* Write Status Register */
#define PROM_READ_OP_CODE			0x03   /* Read Data Bytes */
#define PROM_FAST_READ_OP_CODE		0x0B   /* Read Data Bytes at Higher Speed */
#define PROM_PAGE_PROG_OP_CODE		0x02   /* Page Program */
#define PROM_SECT_ERASE_OP_CODE		0x20   /* Sector Erase */
#define PROM_BLOCK_ERASE_OP_CODE	0xD8   /* Block Erase */
#define PROM_CHIP_ERASE_OP_CODE		0xC7   /* Chip Erase */


/* Bit definition for Read Status Register (RDSR) */
#define PROM_RDSR_WIP        (1<<0)  /* Write in Progress */
#define PROM_RDSR_WEL        (1<<1)  /* Write Enable Latch */
#define PROM_RDSR_BP0        (1<<2)  /* Block 0 Protect */
#define PROM_RDSR_BP1        (1<<3)  /* Block 1 Protect */
#define PROM_RDSR_BP2        (1<<4)  /* Block 2 Protect */
#define PROM_RDSR_BP3        (1<<5)  /* Block 3 Protect */
#define PROM_RDSR_SRWD       (1<<7)  /* Status Register Write Protect */


#define PROM_SECTOR_SIZE      		0x1000   /* 1 sector = 4K bytes */
#define PROM_PAGE_SIZE      		0x100    /* 1 page = 256 bytes */
#define PROM_RD_MAX_BYTE     		0x100
#define PROM_BLANK_DATA       		0xFF
#define PROM_MAX_ERR_CNT      		10

#define FPGA_GOLDEN_IMAGE			1
#define FPGA_UPGRADE_IMAGE			2

#define FPGA_IMAGE_ARRAY			1
#define FPGA_HEADER_ARRAY			2

//Logic FPGA flash map
#define FPGA_HEADER_START_ADDR		0x000000
#define FPGA_HEADER_END_ADDR  		0x00000F

#define FPGA_UPGRADE_START_ADDR     0x010000
#define FPGA_UPGRADE_END_ADDR       0x07FFFF
#define FPGA_GOLDEN_START_ADDR      0x400000
#define FPGA_GOLDEN_END_ADDR        0x47FFFF
#define FPGA_IMAGE_END_ADDR			FPGA_GOLDEN_END_ADDR

#define FPGA_HEADER_START_SECT		(FPGA_HEADER_START_ADDR/PROM_SECTOR_SIZE)
#define FPGA_HEADER_END_SECT		(FPGA_HEADER_END_ADDR+1/PROM_SECTOR_SIZE)
#define FPGA_GOLDEN_START_SECT		(FPGA_GOLDEN_START_ADDR/PROM_SECTOR_SIZE)
#define FPGA_GOLDEN_END_SECT		(FPGA_GOLDEN_END_ADDR+1/PROM_SECTOR_SIZE)
#define FPGA_UPGRADE_START_SECT		(FPGA_UPGRADE_START_ADDR/PROM_SECTOR_SIZE)
#define FPGA_UPGRADE_END_SECT		(FPGA_UPGRADE_END_ADDR+1/PROM_SECTOR_SIZE)
#define FPGA_IMAGE_END_SECT			(FPGA_IMAGE_END_ADDR+1/PROM_SECTOR_SIZE)

extern unsigned char *dash_fpga_fw_array;
extern unsigned int dash_fpga_fw_size;
unsigned int dash_fpga_header_size = 0;
unsigned char *dash_fpga_header_array = NULL;

extern int ttf2array (int size, const char *file, unsigned char *fpga);
extern unsigned char swapbyte(unsigned char c);

static int prom_set_write_enable(boolean bEnable);


//#define LOAD_TTF_FILE
static int get_fpga_image_array (int type, char *file)
{
    FILE *fp;
	unsigned int array_size = 0;
	unsigned char *array_ptr = NULL;
#ifdef LOAD_TTF_FILE
    char c;
    int bytes, i;
#endif
	
    printf("Load file name is %s\n", file);

	//free previous loaded file
	if(type == FPGA_IMAGE_ARRAY)
	{
		if(dash_fpga_fw_array != NULL_PTR)
		{
			free(dash_fpga_fw_array);
			dash_fpga_fw_array = NULL_PTR;
			dash_fpga_fw_size = 0;
		}
	}else
	{
		if(dash_fpga_header_array != NULL_PTR)
		{
			free(dash_fpga_header_array);
			dash_fpga_header_array = NULL_PTR;
			dash_fpga_header_size = 0;
		}
	}

#ifndef LOAD_TTF_FILE
	//Load binary file
	fp = fopen(file, "rb");
    if (fp == NULL) {
        printf("can't open file");
        perror("");
        exit(0);
    }

	//Get file length
	fseek(fp, 0, SEEK_END);
	array_size = ftell(fp);
	fseek(fp, 0, SEEK_SET);

	printf("%d values to be loaded.\n", array_size);
    array_ptr = (uchar *)malloc(array_size+1000);
    if (!array_ptr) {
        printf("\n\ncan't allocate memory for input file \n\n");
        return(FAILED);
    }

	//Read file contents into buffer
	fread(array_ptr, array_size, 1, fp);
	fclose(fp);	

#else
	//Load ttf file
    fp = fopen(file, "r");
    if (fp == NULL) {
        printf("can't open file");
        perror("");
        exit(0);
    }

    while ((c = fgetc(fp)) != EOF) {
        if (c == ',')
            array_size++;
    }
    array_size++;
    fclose(fp);

    printf("%d values to be loaded.\n", array_size);
    array_ptr = (uchar *)malloc(array_size+1000);
    if (!array_ptr) {
        printf("\n\ncan't allocate memory for input file \n\n");
        return(FAILED);
    }
    bytes = ttf2array(array_size, file, array_ptr);
#endif

	if(type == FPGA_IMAGE_ARRAY)
	{
		dash_fpga_fw_array = array_ptr;
		dash_fpga_fw_size = array_size;
#ifdef LOAD_TTF_FILE
	    printf("Last 200 values of loaded file\n");
	    bytes -= 200;
	    for (i=0; i<200; i++, bytes++) {
	        if (!(i % 16))
	            printf("\n");
	        printf("%3d ", swapbyte(dash_fpga_fw_array[bytes]));
	    }
#endif
	}else
	{
		uint32_t golden_addr,update_addr;

		//Check header fw address
		golden_addr = *((uint32_t *)(array_ptr));
		update_addr = *((uint32_t *)(array_ptr+6));
		if((golden_addr!=FPGA_GOLDEN_START_ADDR)||(update_addr!=FPGA_UPGRADE_START_ADDR))
		{
			printf("Header check fail!!\n");
			printf("Got golden_addr:0x%x  update_addr:0x%x \n",golden_addr,update_addr);
			printf("Expect golden_addr:0x%x  update_addr:0x%x \n",FPGA_GOLDEN_START_ADDR,FPGA_UPGRADE_START_ADDR);
			free(array_ptr);
			return(FAILED);
		}
		dash_fpga_header_array = array_ptr;
		dash_fpga_header_size = array_size;
	}
    return(PASSED);
}

static int prom_clear_FIFO_status(boolean bForRead)
{
	return katar_clear_prom_FIFO_status(bForRead);;
}

static int prom_wait_op_done(void)
{
    ulong count = 0;

    do {
		if(katar_check_prom_op_done(TRUE))
			return TRUE;
		msleep(1); /* Sleep 1ms */
    } while (count++ < SPI_PROM_MAX_WAIT);
    return(FALSE);
}

static int prom_write_status_register(uint8_t wr_byte)
{
	/* Ensure that the SPI PROM has write enabled */ 
    if (prom_set_write_enable(TRUE) == FAILED) {
		cterr('f', 0, "%s: Unable to enable SPI device for write", __FUNCTION__);
		return(FAILED);
    }
	katar_set_prom_read_length(1);
	katar_set_prom_opcode(PROM_WRSR_OP_CODE,0);
	katar_set_prom_write_data(wr_byte);
	katar_set_prom_control(TRUE,FALSE,FALSE,FALSE);
	if (!prom_wait_op_done()) {
		cterr('f',0,"%s: read/write operation not done", __FUNCTION__);
		return(FAILED);
	}
	/* Disable Write Operation */
    if (prom_set_write_enable(FALSE) == FAILED) {
		cterr('f', 0, "%s: Unable to disable SPI device for write ", __FUNCTION__);
		return(FAILED);
    }
    return(PASSED);
}

static int prom_read_status_register(uint8_t *rdsr)
{
	katar_set_prom_read_length(1);
	katar_set_prom_opcode(PROM_RDSR_OP_CODE,0);
	katar_set_prom_control(FALSE,FALSE,FALSE,FALSE);
	if (!prom_wait_op_done()) {
		cterr('f',0,"%s: read/write operation not done", __FUNCTION__);
		return(FAILED);
	}
	*rdsr = katar_get_prom_read_data();
	return PASSED;
}

static int prom_wait_WEL (unsigned int max_wait)
{
    ulong count = 0;
    uint8_t rdsr;

    /* Sanity check */
    if (max_wait == 0)
		max_wait = SPI_PROM_MAX_WAIT;

    do {
		if (prom_read_status_register(&rdsr) == FAILED) {
			cterr('f', 0, "Unable to read SPI PROM status register (RDSR)\n");
			return(FALSE);
		}
		if (rdsr & PROM_RDSR_WEL) {
			return(TRUE);
		}
        msleep(1); /* Wait 1ms */
    } while (count++ < max_wait);

    return(FALSE);
}

static int prom_set_write_enable(boolean bEnable)
{
    /* Write enable the device */
    if(bEnable)
        katar_set_prom_opcode(PROM_WREN_OP_CODE,0);
    else
        katar_set_prom_opcode(PROM_WRDI_OP_CODE,0);

    katar_set_prom_control(TRUE,FALSE,FALSE,FALSE);

    /* Check if operation completed */
    if (!prom_wait_op_done()) {
        cterr('f',0,"%s: spi operation not done", __FUNCTION__);
        return(FAILED);
    }

    if(bEnable)
        prom_wait_WEL(0);

    return(PASSED);
}

static int prom_wait_WIP_done (unsigned int max_wait)
{
    ulong count = 0;
    uint8_t rdsr;

    /* Sanity check */
    if (max_wait == 0)
		max_wait = SPI_PROM_MAX_WAIT;

    do {
		if (prom_read_status_register(&rdsr) == FAILED) {
			cterr('f', 0, "Unable to read SPI PROM status register (RDSR)\n");
			return(FALSE);
		}
		if (!(rdsr & PROM_RDSR_WIP)) {
			return(TRUE);
		}
        msleep(1); /* Wait 1ms */
    } while (count++ < max_wait);

    return(FALSE);
}

static int prom_sector_erase (ushort sector)
{
	/* Ensure that the SPI PROM has write enabled */ 
    if (prom_set_write_enable(TRUE) == FAILED) {
		cterr('f', 0, "%s: Unable to enable SPI device for write", __FUNCTION__);
		return(FAILED);
    }
	
	katar_set_prom_opcode(PROM_SECT_ERASE_OP_CODE,sector*PROM_SECTOR_SIZE);
	katar_set_prom_control(TRUE,TRUE,FALSE,FALSE);
	
	/* Check if operation completed */
	if (!prom_wait_op_done()) {
		cterr('f',0,"%s: read/write operation not done", __FUNCTION__);
		return(FAILED);
	}

	prom_wait_WIP_done(SPI_PROM_ERASE_WAIT);
	
	if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("\n Sector number %d [start addr(0x%x)] should now be erased.", 
               sector, (sector * PROM_SECTOR_SIZE));
    } else {
        prpass(testpass, "erasing sector %d", sector);
    }

	/* Disable Write Operation */
    if (prom_set_write_enable(FALSE) == FAILED) {
		cterr('f', 0, "%s: Unable to disable SPI device for write ", __FUNCTION__);
		return(FAILED);
    }
    return(PASSED);
}

static int prom_verify_sector_erase (ushort sector)
{
    volatile ulong start_addr, end_addr;
    volatile uchar data[PROM_RD_MAX_BYTE];
    ushort i, err_count = 0;

	if(prom_clear_FIFO_status(TRUE)==FAILED) {
        cterr('f',0,"SPI PROM Read Fifo is not empty");
        return(FAILED);
    }

    start_addr = sector * PROM_SECTOR_SIZE;
    end_addr   = start_addr + PROM_SECTOR_SIZE - 1; 

    while (start_addr < end_addr) {
		katar_set_prom_read_length(PROM_RD_MAX_BYTE);
		katar_set_prom_opcode(PROM_READ_OP_CODE,start_addr);
		katar_set_prom_control(FALSE,TRUE,FALSE,FALSE);
 
		/* Check if operation completed */
		if (!prom_wait_op_done()) {
			cterr('f',0,"%s: read/write operation not done", __FUNCTION__);
			return(FAILED);
		}

		/* Read and compare */
        for (i = 0; i < PROM_RD_MAX_BYTE; i++) {
		    data[i] =  katar_get_prom_read_data();

			if (data[i] != PROM_BLANK_DATA) {
				printf("\n*** ERROR at addr %#.8lx: PROM = %#.2x, expected = "
				       "%#.2x", start_addr + i, data[i], PROM_BLANK_DATA);
				err_count++;
				if (err_count > PROM_MAX_ERR_CNT) {
					cterr('f', 0, "Exit due to too many mis-matches.");
				    return(FAILED);
				}
			}
		}
		/* Go to the next 256-byte block */
        start_addr = (start_addr + PROM_RD_MAX_BYTE);
    }

    if ((!(NVRAM)->diagflag) & D_VERBOSE) {
		printf("\n Sector %d [start addr(0x%x) end_addr(0x%lx)] Erase Verified "
		       "OK.\n", sector, (sector * PROM_SECTOR_SIZE), end_addr);
    }
    return(PASSED);	
}

static int prom_data_program(ulong start_addr, 
	ulong end_addr, uchar *src_img, boolean bit_reverse)
{
    ulong pgm_size, i, img_idx = 0;
    uchar data_check;

    /* Set flag appropriately depends on whether bits should be reversed */
    while (start_addr < end_addr) {
        /* Calculate the programming bytes */
        if ((end_addr - start_addr) < PROM_RD_MAX_BYTE) {
		    pgm_size = end_addr - start_addr + 1;
        } else {
            pgm_size = PROM_RD_MAX_BYTE;
        }
        
		/* 
		 * To save programming time, we looking ahead 256 bytes to see if 
		 * we need to write this 256-byte block. If they contains all 0xFF, 
		 * do not write since the data were guaranteed to be 0xFF before 
		 * coming in here. A quick way to check if the source contains 0xFF
		 * is to logical and them together. Only when all of them contains
		 * 0xFF, the end result can still be 0xFF.
		 */
		data_check = PROM_BLANK_DATA;
        for (i = 0; i < pgm_size; i++) {
		    data_check &= src_img[img_idx];
		    img_idx++;
        }

		if (data_check != PROM_BLANK_DATA) {
            /* Check if SPI PROM is ready for write */
            if (!prom_wait_WIP_done(SPI_PROM_MAX_WAIT)) {
                cterr('f', 0, "SPI PROM is not ready for write");
                return(FAILED);
            }
		    /* Rewind the index since we skipped ahead to check data */
		    img_idx -= pgm_size;

			/* Now, there are non-blank data that needs to be written.
			* Ensure that the SPI PROM has write enabled 
			*/ 
		    if (prom_set_write_enable(TRUE) == FAILED) {
				cterr('f', 0, "%s: Unable to enable SPI device for write", __FUNCTION__);
				return(FAILED);
		    }
			katar_set_prom_opcode(PROM_PAGE_PROG_OP_CODE,start_addr);
			katar_set_prom_read_length(pgm_size);
			/* Fill data FIFO 256 bytes max at a time */
			for (i = 0; i < pgm_size; i++) {
				katar_set_prom_write_data(src_img[img_idx]);
				img_idx++;
			}
			katar_set_prom_control(TRUE,TRUE,FALSE,bit_reverse);
	    
			/* Check if operation completed */
			if (!prom_wait_op_done()) {
				cterr('f',0,"%s: read/write operation not done", __FUNCTION__);
				return(FAILED);
			}
		}
		/* Go to the next block */
        start_addr = (start_addr + pgm_size);

		/* Print progress indication at every sector boundary */
        if ((NVRAM)->diagflag & D_VERBOSE) {
            if(!(start_addr % PROM_SECTOR_SIZE))
                printf("Programming address %#.8lx.\n", start_addr);
        } else {
		    if (!(start_addr % PROM_SECTOR_SIZE))
	                printf(".");fflush(stdout);
        }
    }
    /* Check if SPI PROM is ready for write */
    if (!prom_wait_WIP_done(SPI_PROM_MAX_WAIT)) {
        cterr('f', 0, "SPI PROM is not ready for write");
        return(FAILED);
    }
    return(PASSED);
}

static int prom_verify_download_image (ulong start_addr, 
		       ulong end_addr, uchar *src_img, boolean bit_reverse)
{
    volatile uchar data[PROM_RD_MAX_BYTE];
    ushort i, err_count = 0;
    ulong img_idx = 0, count;

    if (!katar_check_prom_FIFO_Empty(TRUE)) {
		cterr('f',0,"SPI PROM Read Fifo is not empty");
		return(FAILED);
    }

    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("\nVerify image contents from %#.8lx to %#.8lx\n", start_addr,
               end_addr);
    }

    while (start_addr < end_addr) {
        /* Handle case where the compare size is not multiple of 256 bytes */
        if ((end_addr - start_addr) < PROM_RD_MAX_BYTE) {
		    count = end_addr - start_addr + 1;
        } else {
            count = PROM_RD_MAX_BYTE;
        }

		katar_set_prom_read_length(count);
		katar_set_prom_opcode(PROM_READ_OP_CODE,start_addr);
		katar_set_prom_control(FALSE,TRUE,FALSE,bit_reverse);

		/* Check if operation completed */
		if (!prom_wait_op_done()) {
			cterr('f',0,"%s: read/write operation not done", __FUNCTION__);
			return(FAILED);
		}

		/* Comparing with the original image data in 256-byte block */
        for (i = 0; i < count; i++) {
			data[i] =  katar_get_prom_read_data();

            if (data[i] != src_img[img_idx]) {
				printf("\n *** ERROR at addr %#.8lx: PROM =%#.2x FILE=%#.2x ", 
			       start_addr + i, data[i], src_img[img_idx]);
				err_count++;

                if (err_count > PROM_MAX_ERR_CNT) {
				    cterr('f', 0, "Exit due to too many mis-matches.");
	                    return(FAILED);
                }
            }
		    img_idx++;
		}
		
		/* Go to the next  block */
        start_addr = start_addr + count;

		/* Print progress indication at every sector boundary */
        if(!(start_addr % PROM_SECTOR_SIZE)) {
		    printf(".");fflush(stdout);
        }
    }

    if (err_count) {
        cterr('f', 0, "SPI PROM image programming verification failed with "
	      "%d errors\n", err_count);
        return(FAILED);
    } 

    return(PASSED);
}


static int prom_sector_erase_and_verify (ushort sector)
{
    /* Erase sector */
    if (prom_sector_erase(sector) == FAILED) {
		cterr('f', 0, "Erase operation failed on sector %d", sector);
		return(FAILED);
    }
    
    /* Poll for erase completion - look for WIP bit clear */
    if (!prom_wait_WIP_done(SPI_PROM_ERASE_WAIT)) {
		cterr('f', 0, "Erase not completed on sector %d", sector);
		return(FAILED);
    }

    /* Verify sector */
    if (prom_verify_sector_erase(sector) == FAILED) {
		cterr('f',0,"Verify erase failed on sector %d", sector);
		return(FAILED);
    }
    return(PASSED);
}

unsigned char
*katar_get_platform_fpga_fw (void)
{
    unsigned int fpga_ver = 0, cpld_ver = 0, fpga_brd = 0, cpld_brd = 0;

    katar_get_platform_ver(0, &cpld_ver, &fpga_ver, &cpld_brd, &fpga_brd);

    if (!dash_fpga_fw_array) {
		char filename[64] = {0};
		struct stat sts;

		printf("Please enter logic FPGA FW file name [ex: .spi]\n"
           "(Enter q to quit): ");
	    fflush(stdout);

		get_line(filename, sizeof(filename));
	    if (strcmp(filename, "q") == 0) {
	        return (NULL);
	    } else if (strcmp(filename, "") == 0) {
	        return (NULL);
	    }
		if (stat(filename, &sts) == -1)	{
			printf("\n%s does not exist\n", filename);
			return (NULL);
		}
		if (get_fpga_image_array(FPGA_IMAGE_ARRAY,filename)==FAILED) {
            cterr('f', 0, "unable to read logic FPGA firmware file");
			return (NULL);
        }
		
    }
    return (unsigned char *)(((unsigned long)dash_fpga_fw_array));
}

unsigned char
*katar_get_platform_fpga_header (void)
{
	char filename[64] = {0};
    struct stat sts;
	
	printf("Please enter logic FPGA header file name [ex: .spidir]\n"
       "(Enter q to quit): ");
    fflush(stdout);

	get_line(filename, sizeof(filename));
    if (strcmp(filename, "q") == 0) {
        return (NULL);
    } else if (strcmp(filename, "") == 0) {
        return (NULL);
    }
	if (stat(filename, &sts) == -1)	{
		printf("\n%s does not exist\n", filename);
		return (NULL);
	}
	if (get_fpga_image_array(FPGA_HEADER_ARRAY,filename)==FAILED) {
        cterr('f', 0, "unable to read logic FPGA header file");
		return (NULL);
    }
	
    return (unsigned char *)(((unsigned long)dash_fpga_header_array));
}


unsigned int
katar_get_platform_fpga_size (void)
{
    return dash_fpga_fw_size;
}

static int
katar_reggio_fpga_update (uchar type)
{
	uchar *fpga_fw;
	int	 fpga_fw_size;
	ulong start_addr;
	ulong end_addr;
	ushort start_sector;
	ushort end_sector;
	ushort sector;

    testname("FPGA programming");

    fpga_fw = katar_get_platform_fpga_fw();
    fpga_fw_size = katar_get_platform_fpga_size();
    fflush(stdout);

	if(fpga_fw == NULL_PTR)
	{
		cterr('f', 0, "No FPGA FW loaded. No action taken.");
		return(FAILED);
	}

    /* Clear NAND status register */
    if (prom_write_status_register(0x00) == FAILED) {
		cterr('f', 0, "Failure to unprotect the SPI PROM to allow write");
		return(FAILED);
    }
	
    /* Check if SPI PROM is ready for write */
    if (!prom_wait_WIP_done(SPI_PROM_MAX_WAIT)) {
        cterr('f', 0, "SPI PROM is not ready for write");
        return(FAILED);
    }

	switch(type)
	{
		case FPGA_GOLDEN_IMAGE:
			start_addr = FPGA_GOLDEN_START_ADDR;
			start_sector = FPGA_GOLDEN_START_SECT;
			break;
		case FPGA_UPGRADE_IMAGE:
			start_addr = FPGA_UPGRADE_START_ADDR;
			start_sector = FPGA_UPGRADE_START_SECT;
			break;
		default:
			cterr('f', 0, "Unknown FPGA type. No action taken.");
			return (FAILED);
			break;
			
	}
	end_addr = start_addr + fpga_fw_size;
	/* 
	 * end_sector hold only the number of sectors that needs to be 
	 * erased for programming.
	 */
	end_sector = start_sector + (fpga_fw_size/PROM_SECTOR_SIZE);
			
    printf("\nErasing SPI PROM from sectors %d to %d.\n",start_sector, end_sector);


    for (sector = start_sector; sector <= end_sector; sector++) {
		if (prom_sector_erase_and_verify(sector) == FAILED) {
			cterr('f', 0, "erase %d sector failed.",sector);
		    return(FAILED);
		}
    }

    printf("\nProgramming SPI PROM [0x%08lx to 0x%08lx]\n",start_addr, end_addr);
    if (prom_data_program(start_addr,end_addr,fpga_fw, FALSE) == FAILED) {		/*ZZZ*/
		cterr('f', 0, "%s: SPI PROM image programming failed.");
		return(FAILED);
    }

    printf("\nVerifying ...\n");
    if (prom_verify_download_image(start_addr,end_addr, fpga_fw, FALSE)	== FAILED) {  /*ZZZ*/
    	cterr('f',0,"Verify of FPGA image failed.");
        return(FAILED);
    }
    return(PASSED);
}

static int
katar_reggio_header_update (void)
{
	ulong start_addr;
	ulong end_addr;
	ushort start_sector;
	ushort end_sector;
	ushort sector;

    testname("FPGA header programming");

    fflush(stdout);

	if(dash_fpga_header_array == NULL_PTR)
	{
		cterr('f', 0, "No FPGA header loaded. No action taken.");
		return(FAILED);
	}

    /* Clear NAND status register */
    if (prom_write_status_register(0x00) == FAILED) {
		cterr('f', 0, "Failure to unprotect the SPI PROM to allow write");
		return(FAILED);
    }
	
    /* Check if SPI PROM is ready for write */
    if (!prom_wait_WIP_done(SPI_PROM_MAX_WAIT)) {
        cterr('f', 0, "SPI PROM is not ready for write");
        return(FAILED);
    }

	start_addr = FPGA_HEADER_START_ADDR;
	start_sector = FPGA_HEADER_START_SECT;
	end_addr = start_addr + dash_fpga_header_size;
	end_sector = start_sector + (dash_fpga_header_size/PROM_SECTOR_SIZE);
			
    printf("\nErasing SPI PROM from sectors %d to %d.\n",start_sector, end_sector);


    for (sector = start_sector; sector <= end_sector; sector++) {
		if (prom_sector_erase_and_verify(sector) == FAILED) {
			cterr('f', 0, "erase %d sector failed.",sector);
		    return(FAILED);
		}
    }

    printf("\nProgramming SPI PROM [0x%08lx to 0x%08lx]\n",start_addr, end_addr);
    if (prom_data_program(start_addr,end_addr,dash_fpga_header_array, FALSE) == FAILED) {		/*ZZZ*/
		cterr('f', 0, "%s: SPI PROM header programming failed.");
		return(FAILED);
    }

    printf("\nVerifying ...\n");
    if (prom_verify_download_image(start_addr,end_addr, dash_fpga_header_array, FALSE)	== FAILED) {  /*ZZZ*/
    	cterr('f',0,"Verify of FPGA header failed.");
        return(FAILED);
    }
    return(PASSED);
}

int katar_program_fpga_header (int dummy)
{
	printf("This process will ERASE sectors and Re-Program FPGA Header.\n");
	if (getc_answer("Do you really want to do it ?", "yn", 'n') == 'n')
	{
		printf("\nProgram FPGA header is Aborted by User !!!\n");
		return (FAILED);
	}	

	if(katar_get_platform_fpga_header() == NULL_PTR)
	{
		printf("\nFPGA Header is not loaded !!!\n");
		return (FAILED);
	}

    /* Program fw image */
    if (katar_reggio_header_update()==FAILED) {
        return (FAILED);
    }
	return (PASSED);
}
/*-------------------------------------------------------------------
 *
 * Function: program_fpga_spi_prom()
 * 
 * This function is a wrapper function to program the FPGA image to
 * the FPGA SPI PROM. It will be done in interactive mode.
 *
 * Output: PASSED/FAILED
 *
 *-------------------------------------------------------------------
 */
int
katar_program_fpga_spi_prom (int header)
{
	int image_type;

    image_type = gethex_answer("Golden(1) or Upgrade(2) image? (0 to exit)",2, 0, 2);
	switch(image_type)
	{
		case FPGA_GOLDEN_IMAGE:
		    printf("This process will ERASE sectors and Re-Program FPGA.\n");
			printf("Once programmed, the Golden image should never be changed.\n");
			if (getc_answer("Do you really want to do it ?", "yn", 'n') == 'n')
			{
				printf("\nProgram SPI PROM is Aborted by User !!!\n");
				return (FAILED);
			}
			break;
		case FPGA_UPGRADE_IMAGE:
		    printf("This process will ERASE sectors and Re-Program FPGA.\n");
			if (getc_answer("Do you really want to do it ?", "yn", 'n') == 'n')
			{
				printf("\nProgram SPI PROM is Aborted by User !!!\n");
				return (FAILED);
			}			
			break;
		default:
			printf("\nProgram SPI PROM is Aborted by User !!!\n");
			return (FAILED);
			break;
	}

	if(katar_get_platform_fpga_fw() == NULL_PTR)
	{
		printf("\nLogic FPGA FW is not loaded !!!\n");
		return (FAILED);
	}

    /* Program fw image */
    if (katar_reggio_fpga_update(image_type)==FAILED) {
        return FAILED;
    }	
	printf("\n");	
	if(header)
		katar_program_fpga_header(0);
	
    printf("\n\n****Please power cycle for the new FPGA to take effect.******\n\n");
	return PASSED;
}

int  
katar_display_prom_sector (int dummy)
{
    volatile uchar data[PROM_RD_MAX_BYTE];
    ushort i;
    unsigned int sector, start_addr, end_addr;
    ulong count;

	printf("header sector:%d; Golder image start sector:%d; Update image start sector:%d\n",
		FPGA_HEADER_START_SECT,FPGA_GOLDEN_START_SECT,FPGA_UPGRADE_START_SECT);    
    sector = getdec_answer("Enter sector number ", FPGA_HEADER_START_SECT,
                           0, FPGA_IMAGE_END_SECT);

    start_addr = sector * PROM_SECTOR_SIZE;
    //   end_addr   = start_addr + PROM_SECTOR_SIZE - 1;
	end_addr = start_addr + PROM_PAGE_SIZE;    
    printf("start address = %#x; end addres = %#x\n",start_addr, end_addr);

    if (prom_clear_FIFO_status(TRUE)==FAILED) 
	{
		cterr('f',0,"SPI PROM Read Fifo is not empty");
		return(FAILED);
    }

    if ((!(NVRAM)->diagflag) & D_VERBOSE) {
        printf("\nVerify image contents from %#.8x to %#.8x\n", start_addr,end_addr);
    }

	/* 
	* FPGA image data was bit-reversed before programming, all other image
	* and headers weren't
	*/

    while (start_addr < end_addr) {
        /* Handle case where the compare size is not multiple of 256 bytes */
        if ((end_addr - start_addr) < PROM_RD_MAX_BYTE) {
		    count = end_addr - start_addr;
        } else {
            count = PROM_RD_MAX_BYTE;
        }
		katar_set_prom_read_length(count);
		katar_set_prom_opcode(PROM_READ_OP_CODE,start_addr);
		katar_set_prom_control(FALSE,TRUE,FALSE,FALSE);

		/* Check if operation completed */
		if (!prom_wait_op_done()) {
			cterr('f',0,"%s: read/write operation not done", __FUNCTION__);
			return(FAILED);
		}

		/* Print data in 256-byte block */
        for (i = 0; i < count; i++) {
		    data[i] = katar_get_prom_read_data();
            if (!(i % 16))
                printf("\n%04x:\t", (start_addr & 0xFFFF) + i);
            printf("%02x ", data[i]);
		}
		/* Go to the next  block */
        start_addr = start_addr + count;
    }
    return(PASSED);
}

  
/*
 *------------------------------------------------------------------
 * $Log: platform_prom.c,v $
 * Revision 1.2  2019/06/14 05:24:51  mikech2
 * Collapse katar-branch00 to Main Trunk
 *
 * Revision 1.1.2.3  2019/03/05 07:29:37  mikech2
 * Clean up codes
 *
 * Revision 1.1.2.2  2019/02/12 08:06:30  mikech2
 * rename katar_*.h files
 *
 * Revision 1.1.2.1  2019/01/29 01:54:21  mikech2
 * rename katar_* files
 *
 * Revision 1.1.2.3  2018/11/19 08:47:33  mikech2
 * Add usrlogic FPGA header check
 *
 * Revision 1.1.2.2  2018/11/14 00:51:35  mikech2
 * Modify UsrLogic FPGA update address
 *
 * Revision 1.1.2.1  2018/10/22 08:02:29  mikech2
 * Move project folder to common/src/katar/x86
 *
 * Revision 1.1.2.4  2018/09/18 00:42:44  mikech2
 * change logic FPGA golden/upgrade addr
 *
 * Revision 1.1.2.3  2018/09/12 08:32:48  mikech2
 * Fix userlogic FPGA update & system info version issue
 *
 * Revision 1.1.2.2  2018/07/19 06:32:03  mikech2
 * modify logic FPGA upgrade flow
 *
 *
 *
 *------------------------------------------------------------------
 * $Endlog$
 */
