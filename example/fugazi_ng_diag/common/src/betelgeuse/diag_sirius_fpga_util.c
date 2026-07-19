/* $Id: diag_sirius_fpga_util.c,v 1.2 2019/01/10 06:36:24 wilbhuan Exp $ 
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/betelgeuse/diag_sirius_fpga_util.c,v $
 *------------------------------------------------------------------
 * 
 * diag_sirius_fpga_util.c
 *
 * Copyright (c) 2018 - 2019 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/ioctl.h> 
#include <unistd.h>
#include "common.h"
#include "types.h"
#include "error.h"
#include "queryflags.h"
#include "nvmonvars.h"
#include "defs.h"
#include "common_utils.h"
#include "diag_sirius_fpga_util.h"
#include "diag_sirius_fpga_lib.h"
#include "diag_moka_fpga_lib.h"

static unsigned int update_flag = 0;
static char major = 0;
static char minor = 0;
static char debug = 0;
static char brd_rev = 0;
static char hour  = 0;
static char date = 0;
static char month = 0;
static char year = 0;

static uchar *fpga_fw = NULL;
static char fw_file_name[128];
static int fpga_fw_size;
static reggio_fpga_prog_info_t fpga_info;

int plug_fpga_spi_prog(int);
int plug_fpga_set_update_flag(int);
int plug_fpga_set_date_revision(int);
int plug_fpga_display_sector(int);
int plug_fpga_erase_header(int);

static int DSWAP4(int);
static int plug_fpga_erase_header_spiprom(int, boolean);
static int plug_fpga_fw_update(int, boolean); 
static int plug_fpga_prog_header(int, boolean);
static uchar swapbyte(uchar);
static int plug_fpga_ttf2array(char *, uint *);
static int plug_fpga_device_write_enable(int);
static int plug_fpga_is_wr_op_done(void);
static int plug_fpga_prom_sect_erase_verify(uint);
static int plug_fpga_prom_sect_erase(uint);
static int plug_fpga_read_spi_prom_status_reg(int *);
static int plug_fpga_read_status_bit_check(uint, int, int);
static int plug_fpga_verify_sector_erase(uint);
static int plug_fpga_is_read_fifo_empty(void);
static int plug_fpga_is_write_fifo_empty(void);
static int plug_fpga_get_info(reggio_fpga_prog_info_t *, int, boolean);
static int plug_fpga_prom_image_upgrade(uint, uint, uchar *, boolean);
static int plug_fpga_verify_download_image(uint, uint, uchar *, boolean);

/*******************************************************************************
 *
 * Function   : plug_fpga_set_date_revision
 * Description: Function to set Sirius FPGA date and revision.
 * Inputs     : dummy - reserved for future use
 * Outputs    : PASSED
 *
 *******************************************************************************
 */
int plug_fpga_set_date_revision (int dummy)
{
    int mjr, mnr, dbg, brd;
    int hr, dt, mnth , yr;

    printf("Enter 4 byte FPGA revision (ie,  board_revision.major.minor.debug)\n");
    scanf("%d.%d.%d.%d", &brd, &mjr, &mnr, &dbg);

    major = (char)mjr;
    minor = (char )mnr;
    debug = (char)dbg;
    brd_rev = (char)brd;

    printf("board rev = %d; major = %d; minor = %d; debug = %d\n",
            brd_rev, major, minor, debug);

    printf("Enter date (ie, 14.20.8.12 for 2pm august 20th, 2012)\n");
    scanf("%d.%d.%d.%d", &hr, &dt, &mnth, &yr);
    hour = (char)hr;
    date = (char)dt;
    month = (char)mnth;
    year = (char)yr;

    printf("hour = %d; date = %d; month = %d; year = %d\n", hour, date, month, year);
    fflush(stdin); 
    fflush(stdout);
    
    return (PASSED);
}

/*******************************************************************************
 *
 * Function   : plug_fpga_set_update_flag
 * Description: Function to set Sirius FPGA the updated flag.
 * Inputs     : dummy - reserved for future use
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
int plug_fpga_set_update_flag (int dummy)
{
    update_flag = getdec_answer("upgrade: Always(0); if newer(1); if not same (2); "
                                "don't upgrade(3)", 0, 0, 3);

    return (PASSED);
}

/*******************************************************************************
 *
 * Function   : plug_fpga_display_sector
 * Description: Function to display Sirius FPGA sector.
 * Inputs     : dummy - reserved for future use
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
int plug_fpga_display_sector (int dummy)
{
    volatile uchar data[PROM_RD_MAX_BYTE];
    ushort ix;
    unsigned int sector, start_addr, end_addr;
    int temp_data;
    ulong ctrl_flag, count;
    
    printf("sector %d = configuration/status header; sector %d = fpga upgrade image header\n",
           FPGA_CONFIG_STS_HDR_SECT, FPGA_UPGRADE_IMG_HDR_SECT);
    sector = getdec_answer("Enter sector number (0~130) ", 
                            FPGA_CONFIG_STS_HDR_SECT, 0, 130);

    start_addr = sector * SPI_PROM_SECTOR_SIZE;

    if (sector == FPGA_CONFIG_STS_HDR_SECT) {
        printf("Not supported now. Exiting...\n");
        return (PASSED);
    } else {
        end_addr = start_addr + 0x100;
    }

    if (start_addr == end_addr) {
        end_addr = start_addr + 0x100;
    }

    printf("start address = %#x; end addres = %#x\n", start_addr, end_addr);

    if (plug_fpga_is_read_fifo_empty() != TRUE) {
        printf("%s: SPI PROM Read Fifo is not empty\n", __FUNCTION__);
        return (FAILED);
    }

    if ((!(NVRAM)->diagflag) & D_VERBOSE) {
        printf("\nVerify image contents from %#x to %#x\n", start_addr,
                end_addr);
    }

    /* FPGA image data was bit-reversed before programming, all other image
     * and headers weren't
     */
    ctrl_flag = PROM_DFLT_BAUD | PROM_USE_ADDR;

    while (start_addr < end_addr) {
        /* Handle case where the compare size is not multiple of 256 bytes */
        if ((end_addr - start_addr) < PROM_RD_MAX_BYTE) {
            count = end_addr - start_addr;
        } else {
            count = PROM_RD_256_BYTE;
        }

        fpga_write_32_reg(PLUG_FPGA_SPI_PROM_OP_ADDR_REG, 
                          DSWAP4(PROM_READ_OP | start_addr));
        fpga_write_32_reg(PLUG_FPGA_SPI_PROM_RD_SIZE_REG, DSWAP4(count));
        fpga_write_32_reg(PLUG_FPGA_SPI_PROM_CTRL_REG, DSWAP4(ctrl_flag));
        
        /* Check if operation is completed */
        if (plug_fpga_is_wr_op_done() != TRUE) {
            printf("%s: Read operation is not done\n", __FUNCTION__);
            return (FAILED);
        }

        /* Comparing with the original image data in 256-byte block */
        for (ix = 0; ix <= count; ix++) {
            fpga_read_32_reg(PLUG_FPGA_SPI_PROM_RW_DATA_REG, (uint *)&temp_data);
            data[ix] = (uchar)(DSWAP4(temp_data));

            if (!(ix % 16)) {
                printf("\n%04x:\t", (start_addr & 0xFFFF) + ix);
            }
            printf("%02x ", data[ix]);
        }
        /* Go to the next  block */
        start_addr = start_addr + count + 1;
    }
    
    return (PASSED);
}

/*******************************************************************************
 *
 * Function   : plug_fpga_erase_header
 * Description: Function to erase Sirius FPGA header.
 * Inputs     : dummy - reserved for future use
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
int plug_fpga_erase_header (int dummy)
{
    int ret;
    int ans;

    ret = plug_fpga_erase_header_spiprom(0, TRUE);

    if (ret == FAILED) {
        printf("Erase header failed...\n");
        return (FAILED);
    }

    printf("\nErase done. (Press 'y/Y' to program header or any other key to Quit\n");

    ans = getchar();

    if ((ans == 'y') || (ans == 'Y')) {
        return (plug_fpga_prog_header(0, TRUE));
    }

    return (PASSED);
}

/*******************************************************************************
 *
 * Function   : plug_fpga_spi_prog
 * Description: Function to program Sirius FPGA.
 * Inputs     : dummy - reserved for future use
 *              verbose - flag to enable/disable verbose mode
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
int plug_fpga_spi_prog (int dummy)
{
    uint image_type, file_size;
    char ans;
    char fw_file[128];
    FILE *fp;
    int header = FALSE;

    /* Query user whether it is Golden/Upgrade image */
    image_type = (uint)gethex_answer("Golden(1) or Upgrade(2) image? (0 to exit)",
                                     2, 0, 2);
    if (image_type != FPGA_UPGRADE_IMAGE && image_type != FPGA_GOLDEN_IMAGE) {
        printf("Exiting...\n");
        return (PASSED);
    }

    if (image_type == FPGA_UPGRADE_IMAGE) {
        header = TRUE;
    }

    printf("Enter FPGA file name [%s]: ", FPGA_DEFAULT_FILE_PATH);
    scanf("%128[^\n]s", fw_file);
    sprintf(fw_file_name, "%s%s", FPGA_DEFAULT_FILE_PATH, fw_file);
    /* Flush '\n' in scanf */
    getchar();
    
    /* Check to make sure if we have a valid firmware file */
    fp = fopen(fw_file_name, "rb");
    if (fp == NULL) {
        printf("%s: '%s' doesn't exist. Exiting...\n", __FUNCTION__, fw_file_name);
        return (FAILED);
    }

    fseek(fp, 0L, SEEK_END);
    file_size = ftell(fp);
    fclose(fp);
    printf("%s' (%d B) firmware file is found!\n\n", fw_file_name, file_size);

    if (image_type == FPGA_UPGRADE_IMAGE) {
        printf("upgrade image header will be programmed with\n");
        printf("board revision = %d; major = %d; minor = %d; debug = %d;\n",
               brd_rev, major, minor, debug);
        printf("hour = %d; date = %d; month = %d; year = %d;\n",
               hour, date, month, year);
        printf("update flag = %#x\n", update_flag | 0xA0);
    }   
    
    /* Show warning and get User confirmation. */
    printf("This process will ERASE sectors and Re-Program FPGA.\n");
    printf("Do you really want to do it ?\n");
    printf("(Press 'y/Y' to continue or any other key to Quit) ");

    ans = getchar();
    
    if (!((ans == 'y') || (ans == 'Y'))) {
	    printf("\nProgram SPI PROM is Aborted by User !!!\n");
	    return (PASSED);
    } 
    
    /* Erase the header sector */
    if (plug_fpga_erase_header_spiprom(FPGA_UPGRADE_IMAGE, TRUE) == FAILED) {
        return (FAILED);
    }
    
    /* Program in interactive mode */
    if (plug_fpga_fw_update(image_type, TRUE) == FAILED) {
        return (FAILED);
    }
    
    if (header) {
        /* Program FPGA image header */
        if (plug_fpga_prog_header(image_type, TRUE) == FAILED) {
            return (FAILED);
        }
    }

    printf("\n\n****Please power cycle for the new FPGA to take effect.******\n\n");
    
    return (PASSED);
}

/*******************************************************************************
 *
 * Function   : DSWAP4
 * Description: Function to do data 4byte swap.
 * Inputs     : org - original data for swap
 * Outputs    : value after do 4bytes swap
 *
 *******************************************************************************
 */
static int DSWAP4 (int org)
{
    return (org);
}

/*******************************************************************************
 *
 * Function   : plug_fpga_prog_header
 * Description: Function to program Sirius FPGA header.
 * Inputs     : type
 *              verbose - flag to enable/disable verbose mode
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
static int plug_fpga_prog_header (int type, boolean verbose)
{
    uint start_addr, end_addr;
    uint pgm_size = REGGIO_FPGA_HDR_SIZE;
    reggio_spi_prom_image_header_t fpga_header;
    
    if (verbose) {
        testname("image header programming");
    }

    start_addr = FPGA_SPI_PROM_START_HEADER;

    end_addr = start_addr + pgm_size - 1;

    printf("image header start_addr %#x; end_addr %#x; prog size %#x\n",
           start_addr, end_addr, pgm_size);

    switch (update_flag) {
    case 0:
        printf("(always update)\n");
        break;
    case 1:
        printf("(update if newer)\n");
        break;
    case 2:
        printf("(update if not equal)\n");
        break;
    case 3:
        printf("(do not update)\n");
        break;
    }

    /* Clear image header */
    memset((void *)&fpga_header, 0, pgm_size);

    /* Start building image header */
    fpga_header.magic_number[0] = 0x06;
    fpga_header.magic_number[1] = 0x5d;
    fpga_header.magic_number[2] = 0x4f;
    fpga_header.magic_number[3] = 0x7e;

    fpga_header.flags[0] = update_flag | 0xA0; /* always upgrade */;
    fpga_header.flags[1] = 0;
    fpga_header.flags[2] = 0;
    fpga_header.flags[3] = 0;

    /* Information has been populated in the fpga_info structure earlier */
    fpga_header.revision_id[0] = debug;
    fpga_header.revision_id[1] = minor;
    fpga_header.revision_id[2] = major;
    fpga_header.revision_id[3] = brd_rev;

    fpga_header.revision_date[0] = hour;
    fpga_header.revision_date[1] = date;
    fpga_header.revision_date[2] = month;

    /* Only store 2 digits year in decimal format, so need to subtract 2000 */
    fpga_header.revision_date[3] = year;

    printf("\nProgramming %d bytes of SPI PROM Header to %#x\n", pgm_size, 
            start_addr);

    if (plug_fpga_prom_image_upgrade(start_addr, end_addr, 
                                    (uchar *)&fpga_header, FALSE) == FAILED) {
        printf("%s: FPGA multi-boot header programming failed.\n", __FUNCTION__);
        return (FAILED);
    }

    /* Verify header programming */
    if (plug_fpga_verify_download_image(start_addr, end_addr,
                                       (uchar *)&fpga_header, FALSE) == FAILED) {
        printf("%s: Verify of FPGA multi-boot header failed\n", __FUNCTION__);
        return (FAILED);
    }

    if (verbose) {
        printf("\nSPI PROM header Program/Verify OK.\n");
    }
    
    return (PASSED);
}

/*******************************************************************************
 *
 * Function   : plug_fpga_get_info
 * Description: Function to get Sirius FPGA info.
 * Inputs     : *fpga_p - pointer to FPGA
 *              type
 *              verbose - flag to enable/disable verbose mode
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
static int plug_fpga_get_info (reggio_fpga_prog_info_t *fpga_p, int type,
                               boolean verbose)
{
    /* Clear the fpga info structure */
    memset((char *)fpga_p, 0, sizeof(reggio_fpga_prog_info_t));
    
    fpga_p->fpga_fw = fpga_fw;
    fpga_p->fpga_fw_size = fpga_fw_size;

    fpga_p->hdr_addr = FPGA_SPI_PROM_START_HEADER;

    switch (type) {
    case FPGA_UPGRADE_IMAGE:
        fpga_p->start_addr = FPGA_SPI_PROM_UPGRADE_START_ADDR;
        fpga_p->start_sector = FPGA_SPI_PROM_UPGRADE_START_SECT;
        sprintf(fpga_p->image_str, "upgrade");
        break;
    case FPGA_GOLDEN_IMAGE:
        fpga_p->start_addr = FPGA_SPI_PROM_GOLDEN_START_ADDR;
        fpga_p->start_sector = FPGA_SPI_PROM_GOLDEN_START_SECT;
        sprintf(fpga_p->image_str, "golden");
        break;
    default:
        printf("%s: Unknown FPGA image type = %d\n", __FUNCTION__, type);
        return (FAILED);
    }
    fpga_p->end_addr = fpga_p->start_addr + fpga_p->fpga_fw_size;

    /* fpga_p->end_sector hold only the number of sectors that needs to be
     * erased for programming.
     */

    fpga_p->end_sector = fpga_p->start_sector +
                        (fpga_p->fpga_fw_size/SPI_PROM_SECTOR_SIZE);
    printf("FPGA FW SIZ = %d\n", fpga_p->fpga_fw_size);
    printf("Star Sector = %d, End Sector = %d\n", fpga_p->start_sector, fpga_p->end_sector);
    return (PASSED);
}

/*******************************************************************************
 *
 * Function   : plug_fpga_fw_update
 * Description: Function to upgrade Sirius FPGA FW.
 * Inputs     : type
 *              intractv
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
static int plug_fpga_fw_update (int type, boolean intractv) 
{
    uint read_size;
    uint sector;
    int ret, ix;
    
    testname("FPGA programming");

    /* Check the FPGA firmware file and file size */
    printf("Converting TTF to array...");
    fflush(stdout);
    if (plug_fpga_ttf2array(fw_file_name, &read_size) == FAILED) {
        printf("FAILED! Exiting...\n");
        return (FAILED);
    } else {
        printf("OK! (Read size = %d)\n", read_size);
    }

    /* Get info of the FPGA image to be programmed */
    if (plug_fpga_get_info(&fpga_info, type, TRUE) == FAILED) {
        printf("%s: Unknown FPGA type. No action taken.\n", __FUNCTION__);
        free(fpga_fw);
        return (FAILED);
    }

    printf("\nErasing SPI PROM from sectors %d to %d.\n",
           fpga_info.start_sector, fpga_info.end_sector);

    for (sector = fpga_info.start_sector; sector <= fpga_info.end_sector;
         sector++) {
        if (plug_fpga_prom_sect_erase_verify(sector) == FAILED) {
            free(fpga_fw);
            return (FAILED);
        }
    }

    printf("\nProgramming SPI PROM [0x%08lx to 0x%08lx]\n",
            fpga_info.start_addr, fpga_info.end_addr);
    
    if (plug_fpga_prom_image_upgrade(fpga_info.start_addr, fpga_info.end_addr,
                                     fpga_info.fpga_fw, FALSE) == FAILED) {
        printf("%s: SPI PROM image programming failed\n", __FUNCTION__);
        free(fpga_fw);
        return (FAILED);
    }

    printf("\nVerifying...\n"); 
    fflush(stdout);
    for (ix = 0; ix < VERIFY_RETRY ; ix++) {
        ret = plug_fpga_verify_download_image(fpga_info.start_addr, fpga_info.end_addr,
                                              fpga_info.fpga_fw, FALSE);
        if (ret == PASSED) {
            printf("retry again.\n");
            break;
        }
    } 
    if (ret == FAILED) {
        printf("%s: Verifying of FPGA image failed\n", __FUNCTION__);
        free(fpga_fw);
        return (FAILED);
    }

    free(fpga_fw);
    return (PASSED);
}

/*******************************************************************************
 *
 * Function   : plug_fpga_prom_image_upgrade
 * Description: Function to upgrade SPI PROM image.
 * Inputs     : start_addr
 *              end_addr
 *              *src_img - source image for upgrade
 *              bit_reverse
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
static int plug_fpga_prom_image_upgrade (uint start_addr, uint end_addr, 
                                         uchar *src_img, boolean bit_reverse)
{
    ulong ctrl_flag, pgm_size, img_idx = 0;
    int ix;
    uchar data_check;
    
    /* Set flag appropriately depends on whether bits should be reversed */
    ctrl_flag = PROM_DFLT_BAUD | PROM_USE_ADDR | PROM_DATA_WRITE;
    if (bit_reverse) {
        ctrl_flag |= PROM_SWAP_DATA_BITS;
    }

    while (start_addr < end_addr) {
        /* Check WIP(Write in Progress) is clear */
        if (plug_fpga_read_status_bit_check(SPI_PROM_MAX_WAIT, BIT_CLEAR,
                                            PROM_RDSR_WIP) != TRUE) {
            printf("%s: SPI PROM is not ready for write\n", __FUNCTION__);
            return (FAILED);
        }

        /* Calculate the programming bytes */
        if ((end_addr - start_addr) < PROM_RD_MAX_BYTE) {
            pgm_size = end_addr - start_addr;
        } else {
            pgm_size = PROM_RD_256_BYTE;
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

        for (ix = 0; ix <= pgm_size; ix++) {
            data_check &= src_img[img_idx];
            img_idx++;
        }

        if (data_check != PROM_BLANK_DATA) {
            /* Check WIP(Write in Progress) is clear */
            if (plug_fpga_read_status_bit_check(SPI_PROM_MAX_WAIT, BIT_CLEAR,
                                                PROM_RDSR_WIP) != TRUE) {
                printf("%s: SPI PROM is not ready for write\n", __func__);
                return (FAILED);
            }
            /* Rewind the index since we skipped ahead to check data */
            img_idx -= (pgm_size + 1);

            /* Now, there are non-blank data that needs to be written.
             * Ensure that the SPI PROM has write enabled
             */
            if (plug_fpga_device_write_enable(TRUE) == FAILED ) {
                printf("%s: Unable to enable SPI device for write\n", __FUNCTION__);
                return (FAILED);
            }
            
            /* Ensure write FIFO is empty */
            if (plug_fpga_is_write_fifo_empty() != TRUE) {
                printf("%s: Write FIFO is not empty.\n", __func__);
                return (FAILED);
            }
            
            fpga_write_32_reg(PLUG_FPGA_SPI_PROM_OP_ADDR_REG, 
                              DSWAP4(PROM_PAGE_PROG_OP | start_addr));
            fpga_write_32_reg(PLUG_FPGA_SPI_PROM_RD_SIZE_REG, DSWAP4(pgm_size));

            /* Fill data FIFO 256 bytes max at a time */
            for (ix = 0; ix <= pgm_size; ix++) {
                fpga_write_32_reg(PLUG_FPGA_SPI_PROM_RW_DATA_REG, DSWAP4(src_img[img_idx]));
                img_idx++;
            }

            /* Start writing to PROM */
            fpga_write_32_reg(PLUG_FPGA_SPI_PROM_CTRL_REG, DSWAP4(ctrl_flag));

            /* Check if operation is completed */
            if (plug_fpga_is_wr_op_done() != TRUE) {
                printf("%s: Write operation not done\n", __FUNCTION__);
                return (FAILED);
            }
        }

        /* Goto the next block */
        start_addr = (start_addr + pgm_size + 1);

        /* Print progress indication at every sector boundary */
        if ((NVRAM)->diagflag & D_VERBOSE) {
            if (!(start_addr % SPI_PROM_SECTOR_SIZE)) {
                printf("Programming address %#x.\n", start_addr);
                fflush(stdout);
            }
        } else {
            if (!(start_addr % SPI_PROM_SECTOR_SIZE)) {
                printf(".");
                fflush(stdout);
            }
        }
    }

    /* Read status register to ensure the programming operation is completed */
    /* The idle time between the page-program and the read command for verificaion
     * should not be less than the maximum page program time of the device(3ms)
     */ 
    if (plug_fpga_read_status_bit_check(SPI_PROM_MAX_WAIT, BIT_CLEAR,
                                        PROM_RDSR_WIP) != TRUE) {
        printf("%s: SPI PROM programming is not completed.\n", __func__);
        return (FAILED);
    } 

    printf("Done...\n");
    fflush(stdout);

    return (PASSED);

}

/*******************************************************************************
 *
 * Function   : plug_fpga_verify_download_image
 * Description: Function to verify the downloaded FW image.
 * Inputs     : start_addr
 *              end_addr
 *              *src_img - buffer to put source image
 *              bit_reverse
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
static int plug_fpga_verify_download_image (uint start_addr, uint end_addr,
                                            uchar *src_img, boolean bit_reverse)
{
    volatile uchar data[PROM_RD_MAX_BYTE];
    ulong img_idx = 0, count, ctrl_flag;
    int ix, err_count = 0;
    int temp_data;

    if (plug_fpga_is_read_fifo_empty() != TRUE) {
        printf("%s: SPI PROM Read Fifo is not empty\n", __FUNCTION__);
        return (FAILED);
    }

    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("\nVerify image contents from %#x to %#x\n", start_addr, end_addr);
    }

    /* FPGA image data was bit-reversed before programming, all other image
     * and headers weren't
     */
    ctrl_flag = PROM_DFLT_BAUD | PROM_USE_ADDR;
    if (bit_reverse) {
        ctrl_flag |= PROM_SWAP_DATA_BITS;
    }

    while (start_addr < end_addr) {
        /* Handle case where the compare size is not multiple of 256 bytes */
        if ((end_addr - start_addr) < PROM_RD_MAX_BYTE) {
            count = end_addr - start_addr;
        } else {
            count = PROM_RD_256_BYTE;
        }

        fpga_write_32_reg(PLUG_FPGA_SPI_PROM_OP_ADDR_REG, 
                          DSWAP4(PROM_READ_OP | start_addr));
        fpga_write_32_reg(PLUG_FPGA_SPI_PROM_RD_SIZE_REG, DSWAP4(count));
        fpga_write_32_reg(PLUG_FPGA_SPI_PROM_CTRL_REG, DSWAP4(ctrl_flag));

        /* Check if operation is completed */
        if (plug_fpga_is_wr_op_done() != TRUE) {
            printf("%s: Read operation is not done\n", __FUNCTION__);
            return (FAILED);
        }

        /* Comparing with the original image data in 256-byte block */
        for (ix = 0; ix <= count; ix++) {
            fpga_read_32_reg(PLUG_FPGA_SPI_PROM_RW_DATA_REG, (uint *)&temp_data);
            data[ix] = (uchar)(DSWAP4(temp_data));

            if (data[ix] != src_img[img_idx]) {
                printf("\n *** ERROR at addr %#x: PROM =%#.2x FILE=%#.2x ",
                       start_addr + ix, data[ix], src_img[img_idx]);
                err_count++;

                if (err_count > PROM_MAX_ERR_CNT) {
                    printf("%s: Exit due to too many mismatches\n", __FUNCTION__);
                    return (FAILED);
                }
            }
            img_idx++;
        }
        
        /* Go to the next block */
        start_addr = start_addr + count + 1;

        /* Print progress indication at every sector boundary */
        if (!(start_addr % SPI_PROM_SECTOR_SIZE)) {
            printf(".");
            fflush(stdout);
        }
    }

    if (err_count) {
        printf("%s: SPI PROM image programming verification failed with %d errors\n",
               __FUNCTION__, err_count);
        return (FAILED);
    }
    
    return (PASSED);
}

/*******************************************************************************
 *
 * Function   : plug_fpga_erase_header_spiprom
 * Description: Function to erase SPI PROM header.
 * Inputs     : type - reserved for furture use
 *              verbose - flag to turn on verbose mode 
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
static int plug_fpga_erase_header_spiprom (int type, boolean verbose)
{
    int sector = UPGRADE_MULTI_BOOT_SECTOR;
    
    if (verbose) {
        printf("\nErasing multiboot image header at sectors %d\n",  sector);
    }

    /* Erase and verify */
    if (plug_fpga_prom_sect_erase_verify(sector) == FAILED) {
        printf("%s: Sector %d erase and verify failed\n", __FUNCTION__, sector);
        return (FAILED);
    }
    
    return (PASSED);
}

/*******************************************************************************
 *
 * Function   : plug_fpga_prom_sect_erase_verify
 * Description: Function to erase and verify the sector.
 * Inputs     : sector - sector number to verify
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
static int plug_fpga_prom_sect_erase_verify (uint sector)
{
    /* Erase sector */
    if (plug_fpga_prom_sect_erase(sector) == FAILED) {
        printf("%s: Erase operation failed on sector %d\n", __FUNCTION__, sector);
        return (FAILED);
    }

    /* Verify sector */
    if (plug_fpga_verify_sector_erase(sector) == FAILED) {
        printf("%s: Verify erase failed on sector %d\n", __FUNCTION__, sector);
        return (FAILED);
    }
    
    return (PASSED);
}

/*******************************************************************************
 *
 * Function   : plug_fpga_verify_sector_erase
 * Description: Function to verify sector erase.
 * Inputs     : sector - sector number to verify
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
static int plug_fpga_verify_sector_erase (uint sector)
{
    volatile ulong start_addr, end_addr;
    int ix;
    int data_in;
    volatile uchar data[PROM_RD_MAX_BYTE];
    int err_count = 0;
    
    if (plug_fpga_is_read_fifo_empty() != TRUE) {
        printf("%s: SPI PROM Read Fifo is not empty\n", __FUNCTION__);
        return (FAILED);
    }

    start_addr = sector * SPI_PROM_SECTOR_SIZE;
    end_addr   = start_addr + SPI_PROM_SECTOR_SIZE - 1;

    while (start_addr < end_addr) {
        fpga_write_32_reg(PLUG_FPGA_SPI_PROM_OP_ADDR_REG, 
                          DSWAP4(PROM_READ_OP | start_addr));
        fpga_write_32_reg(PLUG_FPGA_SPI_PROM_RD_SIZE_REG, DSWAP4(PROM_RD_256_BYTE));
        fpga_write_32_reg(PLUG_FPGA_SPI_PROM_CTRL_REG, 
                          DSWAP4(PROM_DFLT_BAUD | PROM_USE_ADDR));

        /* Check if operation is completed */
        if (plug_fpga_is_wr_op_done() != TRUE) {
            printf("%s: Read operation is not done\n", __FUNCTION__);
            return (FAILED);
        }

        /* Read and compare */
        for (ix = 0; ix < PROM_RD_MAX_BYTE; ix++) {
            fpga_read_32_reg(PLUG_FPGA_SPI_PROM_RW_DATA_REG, (uint *)&data_in);
            data[ix] = (uchar)DSWAP4(data_in);

            if (data[ix] != PROM_BLANK_DATA) {
                printf("\n*** ERROR at addr %#.8lx: PROM = %#.2x, expected = "
                       "%#.2x\n", start_addr + ix, data[ix], PROM_BLANK_DATA);
                err_count++;

                if (err_count > PROM_MAX_ERR_CNT) {
                    printf("%s: Exit due too many mismatches\n", __FUNCTION__);
                    return (FAILED);
                }
            }
        }
        /* Go to the next 256-byte block */
        start_addr += PROM_RD_MAX_BYTE;
    }

    if ((!(NVRAM)->diagflag) & D_VERBOSE) {
        printf("\n Sector %d [start addr(0x%x) end_addr(0x%lx)] Erase Verified "
              "OK.\n", sector, (sector * SPI_PROM_SECTOR_SIZE), end_addr);
    }
    return (PASSED);
}

/*******************************************************************************
 *
 * Function   : plug_fpga_is_read_fifo_empty
 * Description: Function to check if read FIFO is empty.
 * Inputs     : None
 * Outputs    : TRUE/FALSE
 *
 *******************************************************************************
 */
static int plug_fpga_is_read_fifo_empty (void)
{
    uint count = 0;
    int data_in;

    do {
        /* If Read Fifo is empty, return TRUE */
        fpga_read_32_reg(PLUG_FPGA_SPI_PROM_STS_REG, (uint *)&data_in);
        if (data_in & DSWAP4(PROM_RD_FIFO_EMPTY)) {
            return (TRUE);
        }
        
        /* Dummy read to clear read FIFO */
        fpga_read_32_reg(PLUG_FPGA_SPI_PROM_RW_DATA_REG, (uint *)&data_in);
        usleep(100);
    } while (count++ < SPI_PROM_MAX_WAIT);

    return (FALSE);
}

/*******************************************************************************
 *
 * Function   : plug_fpga_is_write_fifo_empty
 * Description: Function to check if write FIFO is empty.
 * Inputs     : None
 * Outputs    : TRUE/FALSE
 *
 *******************************************************************************
 */
static int plug_fpga_is_write_fifo_empty (void)
{
    uint count = 0;
    int data_in;

    do {
        /* Tf Erite FIFO is empty, return TRUE */
        fpga_read_32_reg(PLUG_FPGA_SPI_PROM_STS_REG, (uint *)&data_in);
        if (data_in & DSWAP4(PROM_WR_FIFO_EMPTY)) { 
            return (TRUE);
        }

        /* Dummy read to clear read FIFO */
        fpga_read_32_reg(PLUG_FPGA_SPI_PROM_RW_DATA_REG, (uint *)&data_in);
        usleep(100);
    } while (count++ < SPI_PROM_MAX_WAIT);

    return (FALSE);
}

/*******************************************************************************
 *
 * Function   : plug_fpga_prom_sect_erase
 * Description: Function to erase SPI PROM sector.
 * Inputs     : sector - sector number to erase
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
static int plug_fpga_prom_sect_erase (uint sector)
{
    uint opcode;
    
    /* Ensure that SPI PROM has write enabled */
    printf("Enable write-enable.\n");
    if (plug_fpga_device_write_enable(TRUE) == FAILED) {
        printf("%s: Unable to enable SPI device for write\n", __FUNCTION__);
        return (FAILED);
    }

    /* Do the actual erase */
    printf("Send erase op-code.\n");
    opcode = (((sector * SPI_PROM_SECTOR_SIZE) & PROM_SPI_ADDR_MASK) | 
                PROM_SECT_ERASE_OP);
    fpga_write_32_reg(PLUG_FPGA_SPI_PROM_OP_ADDR_REG, DSWAP4(opcode));
    fpga_write_32_reg(PLUG_FPGA_SPI_PROM_CTRL_REG, DSWAP4(PROM_DFLT_BAUD | 
                      PROM_DATA_WRITE | PROM_USE_ADDR));

    /* Check if operation completed */
    if (plug_fpga_is_wr_op_done() != TRUE) {
        printf("%s: read/write operation not done", __FUNCTION__);
        return (FAILED);
    }

    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("\n Sector number %d [start addr(0x%x)] should now be erased.", 
               sector, (sector * SPI_PROM_SECTOR_SIZE));
    } else {
        prpass(testpass, "erasing sector %d", sector);
    }

    /* Poll for erase completion - look for WIP(Write in Progress) bit clear */
    if (plug_fpga_read_status_bit_check(SPI_PROM_ERASE_WAIT, BIT_CLEAR, 
                                        PROM_RDSR_WIP) != TRUE) {
        printf("%s: Erase not completed on sector %d\n", __FUNCTION__, sector);
        return (FAILED);
    }

    return (PASSED);
}

/*******************************************************************************
 *
 * Function   : plug_fpga_device_write_enable
 * Description: Function to enable or disable SPI PROM WR option.
 * Inputs     : enable - parameter that determines to enable or disable WR
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
static int plug_fpga_device_write_enable (int enable)
{
    int opcode;

    if (enable == TRUE) {
        opcode = PROM_WREN_OP;
    } else {
        opcode = PROM_WRDI_OP;
    }

    /* Write enable the device */
    if (fpga_write_32_reg(PLUG_FPGA_SPI_PROM_OP_ADDR_REG, DSWAP4(opcode))
                          == FAILED) {
        printf("%s: Writing into SPI op addr failed\n", __FUNCTION__);
        return (FAILED);
    }

    if (fpga_write_32_reg(PLUG_FPGA_SPI_PROM_CTRL_REG, 
                          DSWAP4(PROM_DFLT_BAUD | PROM_DATA_WRITE)) == FAILED) {
        printf("%s: Writing into SPI ctrl addr failed\n", __FUNCTION__);
        return (FAILED);
    }

    /* Check if operation is completed */
    if (plug_fpga_is_wr_op_done() != TRUE) {
        printf("%s: Write operation is not done\n", __FUNCTION__);
        return (FAILED);
    }

    /* Poll for WEL(Write Enable Latch) bit set */
    if (enable) {
        if (plug_fpga_read_status_bit_check(SPI_PROM_ERASE_WAIT, BIT_SET,
                                            PROM_RDSR_WEL) != TRUE) {
            printf("%s: SPI PROM is not ready for write\n", __FUNCTION__);
            return (FAILED);
        }
    }

    return (PASSED);
}

/*******************************************************************************
 *
 * Function   : plug_fpga_is_wr_op_done
 * Description: Function to check if the WR option done bit is set.
 * Inputs     : None
 * Outputs    : TRUE/FALSE
 *
 *******************************************************************************
 */
static int plug_fpga_is_wr_op_done (void)
{
    ulong count = 0;
    int rd_status;

    do {
        /* If done bit is set, clear it and return TRUE */
        fpga_read_32_reg(PLUG_FPGA_SPI_PROM_STS_REG, (uint *)&rd_status);
        if (rd_status & DSWAP4(PROM_OPER_DONE)) {
            /* Clear the done bit */
            fpga_write_32_reg(PLUG_FPGA_SPI_PROM_STS_REG, 
                              rd_status | DSWAP4(PROM_OPER_DONE));
            return (TRUE);
        }
        usleep(1000);
    } while (count++ < SPI_PROM_MAX_WAIT);

    return (FALSE);
}

/*******************************************************************************
 *
 * Function   : plug_fpga_read_status_bit_check
 * Description: Function to check the Sirus FPGA status register value by bit.
 * Inputs     : max_wait - Timeout interval 
 *              *check_value - Value that wants to confirm
 *              check_bit - bit number of this check
 * Outputs    : TRUE/FALSE
 *
 *******************************************************************************
 */
static int plug_fpga_read_status_bit_check (uint max_wait, int check_value, 
                                            int check_bit)
{
    uint count = 0;
    int rd_sr;
    
    do {
        usleep(1000);

        if (plug_fpga_read_spi_prom_status_reg(&rd_sr) == FAILED) {
            printf("%s: Unable to read SPI PROM status register (RDSR)\n", __FUNCTION__);
            return (FALSE);
        }

        if (check_value) {
            if ((rd_sr >> check_bit) & (BIT_SET)) {
                return (TRUE);
            }
        } else {
            if (!((rd_sr >> check_bit) & (BIT_SET))) {
               return (TRUE); 
            }
        }
    } while (count++ < max_wait);
    return (FALSE);
}

/*******************************************************************************
 *
 * Function   : plug_fpga_read_spi_prom_status_reg
 * Description: Function to read Sirius FPGA SPI PROM status register.
 * Inputs     : *data_in - buffer to put the read back data 
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
static int plug_fpga_read_spi_prom_status_reg (int *data_in)
{
    int rd_data;
    
    /* Read the SPI PROM Read Status Register (RDSR) */
    fpga_write_32_reg(PLUG_FPGA_SPI_PROM_RD_SIZE_REG, 
                      DSWAP4(PROM_RD_1_BYTE & PROM_RD_SIZE_MASK));
    fpga_write_32_reg(PLUG_FPGA_SPI_PROM_OP_ADDR_REG, DSWAP4(PROM_RDSR_OP));
    fpga_write_32_reg(PLUG_FPGA_SPI_PROM_CTRL_REG, DSWAP4(PROM_DFLT_BAUD)); /* Read */

    /* Check if operation completed */
    if (plug_fpga_is_wr_op_done() != TRUE) {
        printf("%s: Read operation not done\n", __FUNCTION__);
        return (FAILED);
    }

    fpga_read_32_reg(PLUG_FPGA_SPI_PROM_RW_DATA_REG, (uint *)&rd_data);

    *data_in = DSWAP4(rd_data);
    return (PASSED);
}

/*******************************************************************************
 *
 * Function   : plug_fpga_ttf2array
 * Description: Function to read in the FW image for upgrade. 
 * Inputs     : *file - Image for upgrade
 *              *read_size - buffer to put the read-in image size
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
static int plug_fpga_ttf2array (char *file, uint *read_size)
{
    FILE *fp;
    uint ix, line, ch;
    uint size;
    uint val;
    uchar *tmp;
    char c;

    /* Get the size first to allocate the buffer */
    fp = fopen(file, "r");
    if (!fp) {
        printf("\n%s: Can't open '%s'\n\n", __FUNCTION__, file);
        return (FAILED);
    }

    size = 0;
    while ((c = fgetc(fp)) != EOF) {
        if (c == ',') {
            size++;
        }
    }
    size++;
    fpga_fw_size = size;
    fclose(fp);

    fpga_fw = malloc(fpga_fw_size + 1000);
    printf("FPGA_FW = %s\n", fpga_fw);
    if (fpga_fw == NULL) {
        printf("%s: Can't allocate memory for FPGA\n", __func__);
        fflush(stdout);
        fpga_fw_size = 0;
        return (FAILED);
    }
    tmp = fpga_fw;
    printf("tmp = %s\n", tmp);

    fp = fopen(file, "r");
    if (!fp) {
        printf("\n%s: Can't open '%s'\n\n", __FUNCTION__, file);
        return (FAILED);
    }
    ix = line = 1;

    do {
        if (fscanf(fp, "%d", &val) == EOF) {
            printf("problem scanning number\n");
            goto out;
        }

        *tmp++ = swapbyte(val);

        if ((ch = fgetc(fp)) == EOF) {
            printf("End of file. no more chars. %d bytes, %d lines\n", 
                    ix, line);
            goto out;
        } else {
            if (ch == ',') {                
            } else {
                printf("File read successfully. %d bytes found\n", ix);
                goto out;
            }
        }
        ix++;
    } while (!feof(fp));

out:
    fclose(fp);

    *read_size = ix;
    
    return (PASSED);
}

/*******************************************************************************
 *
 * Function   : swapbyte
 * Description: Function to perform byte swap.
 * Inputs     : c - byte value that wants to swap
 * Outputs    : result - the swapped value
 *
 *******************************************************************************
 */
static uchar swapbyte (uchar c)
{
    int ix;
    uchar result = 0;

    for (ix = 0; ix < 8; ix++) {
        result = result << 1;
        result |= (c & 1);
        c = c >> 1;
    }
    
    return (result);
}

/*-------------------------------------------------
 * $Log: diag_sirius_fpga_util.c,v $
 * Revision 1.2  2019/01/10 06:36:24  wilbhuan
 * The beginning of Betelgeuse application code.
 *
 *-------------------------------------------------
 */
