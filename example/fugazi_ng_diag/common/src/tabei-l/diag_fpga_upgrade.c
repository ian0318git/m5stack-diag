 /* $Id: diag_fpga_upgrade.c,v 1.2 2019/10/17 02:16:21 kehuang2 Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/tabei-l/diag_fpga_upgrade.c,v $
 *-----------------------------------------------------------------------------
 * diag_fpga_upgrade.c - 
 * 
 *
 * Copyright (c) 2018-2019 by Cisco Systems, Inc.
 * All rights reserved.
 *-----------------------------------------------------------------------------
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <error.h>
#include <sys/mman.h>
#include "common.h"
#include "proto.h"
#include "linux_api.h"
#include "queryflags.h"
#include "diag_fpga_upgrade.h"
#include "dash_fpga.h"
#include "nvmonvars.h"
#include "diag_fpga.h"
#include "diag_spi_flash_util.h"


#ifndef MAX_STR_SIZE
#define MAX_STR_SIZE 80
#endif

#define SPI_INTERRUPT
#define MAGIC 0x6A2F
#define MAGIC_SHIFT 16

/* Function prototypes */
static prom_t * init_spi_prom_ds(void);
int display_reggio_spi_prom_regs(void);
int read_header_spi_prom_image(uchar);
static int reggio_fpga_update(uchar type, boolean intractv);
static int program_secure_boot_header(uchar type, boolean verbose);
int erase_header_spi_prom_image(uchar, boolean);
static int device_write_disable(prom_t *spi_prom);
static int device_write_enable(prom_t *spi_prom);
static int write_spi_prom_status_reg(prom_t *spi_prom, uchar wr_byte);
static int verify_sector_erase(prom_t *spi_prom, ushort sector);
static int get_fpga_programming_info(reggio_fpga_prog_info_t *fpga_p, uchar type, boolean);
static int program_spi_prom_image_header(uchar type, boolean verbose);
static int diag_fpga_ttf2array(char *file, uint *read_size);

int is_spi_prom_rdy(unsigned int);
int prom_image_program(prom_t *, ulong, ulong, uchar *, boolean);
int get_fpga_firmware(char *file);

unsigned long get_platform_prom_addr(void);
int set_fpga_reconf_fsm(void);
int fpga_write_spi_prom_status_reg(uchar);
int fpga_spi_write_protect_test(uint);

static prom_t *spi_prom;
static unsigned int update_flag = 0;
static char major = 0;
static char minor = 0;
static char debug = 0;
static char brd_rev = 0;
static char hour  = 0;
static char date = 0;
static char month = 0;
static char year = 0;

unsigned int dash_fpga_fw_size = 0;
unsigned char *dash_fpga_fw_array = NULL; /* TBD */


/*
 *  Calabria and Altamont support 2 types of EPROM   
 *  - 4 MB: Just golden image supported
 *  - 8 MB: Both golden and upgrade image supported
 *  - 512 kB
 */
int DSWAP4(int org)
{
    return org;
}

static reggio_spi_eprom_info_t spi_eprom_info[1] = {
    {
        /* per fpga spec */
        .start_g = 0,
        .start_u = 0x400000,
        .start_sector_g = 0,
        .end_sector_g = 47,
        .start_sector_u = 64,
        .end_sector_u = 111,
        .start_header =  0x7F0000,  /* not used for now */
        .end_header = 0x7FFFFF, /* not used for now */
    },
};

static reggio_fpga_prog_info_t fpga_info;
static reggio_spi_eprom_info_t 	 *p_spi_prom_info;

/* 
 * Externs 
 * defined in /nfs/sp-engops/diags/pld/reggio/spi_prom/reggio_fpga_fw150.c
 * defined in /nfs/sp-engops/diags/pld/reggio/spi_prom/reggio_fpga_fw50.c 
 */
static uchar *fpga_fw = NULL;;
static int   fpga_fw_size;

/* is the address in little/big endian format? */
secure_boot_hash_t golden_sboot_hash[SBOOT_HASH_NUM] = {
    {"Golden Bitfile HMAC",   NULL, 0x003D0000, 32},
    {"Golden Bitfile Header", NULL, 0x003D0040, 4},
};

/* is the address in little/big endian format? */
secure_boot_hash_t upgrade_sboot_hash[SBOOT_HASH_NUM] = {
    {"Upgrade Bitfile HMAC",   NULL, 0x007D0000, 32},
    {"Upgrade Bitfile Header", NULL, 0x007D0040, 4},
};



/*-------------------------------------------------------------------
 *
 * Function: get_fpga_addr
 * Description: get fpga address
 *
 * Input: NONE
 *
 * Output: address
 *
 *-------------------------------------------------------------------
 */
unsigned long get_fpga_addr (void)
{
    unsigned long addr;
    assert(dash_fpga);
    addr = (unsigned long)dash_fpga;

    return addr;

}

/*-------------------------------------------------------------------
 *
 * Function: get_platform_prom_addr
 * Description: get prom address
 * 0xB00 FPGA configuration SPI PROM programming register
 * Input: NONE
 *
 * Output: address
 *
 *-------------------------------------------------------------------
 */
unsigned long get_platform_prom_addr (void)
{
    assert(dash_fpga);
    return ((unsigned long)dash_fpga + FPGA_CP_SPI_PROM_OFFSET);
}


/*-------------------------------------------------------------------
 *
 * Function: byteswap32
 * 
 * wrapper function for dswap32. if we need to swap, this function wil
 * call dswap32.
 *
 * Input: int num; number to be swapped
 *
 * Output: org, swapped value
 *
 *-------------------------------------------------------------------
 */
static int byteswap32 (int num)
{
    return num;
}


/*-------------------------------------------------------------------
 *
 * Function: get_platform_ver
 * 
 * get Platform version
 *
 * Input: verbose: if flag set to true then print version
 * Output: cpld_ver: cpld version
 *         cpld_Brd: cpld board revision
 *         fpga_brad: fpga brd rev
 *         always returns 0
 *
 *-------------------------------------------------------------------
 */
int get_platform_ver (unsigned int verbose, unsigned int *cpld_ver,
                      unsigned int *fpga_ver, unsigned int *cpld_brd,
                      unsigned int *fpga_brd)
{
    unsigned int tmp32;
    sys_lvl_t  *fpga  = (sys_lvl_t*)dash_fpga;

    assert(dash_fpga);
    tmp32 = fpga->ver;
    *fpga_ver = byteswap32(tmp32);
    tmp32 = fpga->brd;
    *fpga_brd = (tmp32);

   if (verbose) {
        printf("fpga version @%#lx=%#x; fpga brd info: @%#lx=%#x \n",
               (unsigned long)&fpga->ver - dash_fpga, *fpga_ver,
               (unsigned long)&fpga->brd - dash_fpga, *fpga_brd);
        printf("board rev=%#x; major=%#x; minor=%#x; debug rev=%#x\n",
               (fpga->ver & 0x07000000) >> 24,
               (fpga->ver & 0x007F0000) >> 16,
               (fpga->ver & 0x0000FF00) >> 8,
               fpga->ver & 0x000000FF);
    }
    return 0;
}



/*-------------------------------------------------------------------
 *
 * Function :get_platform_fpga_fw
 * Description: retrun pointer to fpga fw
 * INPUT:  NONE
 * OUTPUT: fpga firmware
 * -------------------------------------------------------------------
*/
unsigned int get_platform_fpga_size (void)
{   
    unsigned int fpga_ver = 0, cpld_ver = 0, fpga_brd = 0, cpld_brd = 0;

    get_platform_ver(0, &cpld_ver, &fpga_ver, &cpld_brd, &fpga_brd);

    return dash_fpga_fw_size;

}

/*-------------------------------------------------------------------
 *
 * Function :get_platform_fpga_fw
 * Description: retrun pointer to fpga fw
 * INPUT:  NONE
 * OUTPUT: fpga firmware
 * -------------------------------------------------------------------
*/
unsigned char *get_platform_fpga_fw (void)
{   
    unsigned int fpga_ver = 0, cpld_ver = 0, fpga_brd = 0, cpld_brd = 0;

    get_platform_ver(0, &cpld_ver, &fpga_ver, &cpld_brd, &fpga_brd);

    if (!dash_fpga_fw_array) {
        assert(!"Missing fpga file name in its command line argument");
    }
    return (unsigned char *)(((unsigned long)dash_fpga_fw_array));


}


/*-------------------------------------------------------------------
 *
 * Function: is read_fifo_empty()
 * 
 * This function checks if the read FIFO is empty. It's important to do
 * this check before any read operation, otherwise this can lead to invalid 
 * reads.
 *
 * Input: spi_prom    - Pointer to the spi prom register file
 *
 * Output: TRUE if read Fifo empty, FALSE otherwise
 *
 *-------------------------------------------------------------------
 */
int is_read_fifo_empty (prom_t *spi_prom)
{
    ulong count = 0;

    do {
        /* If Read Fifo is empty, return TRUE */
        if (spi_prom->status & DSWAP4(PROM_RD_FIFO_EMPTY)) {
            return (TRUE);
        }

        /* Dummy read to clear read FIFO */
        spi_prom->data;
        /* Wait 100 us */
        wastetime(100);
    } while (count++ < SPI_PROM_MAX_WAIT);

    return (FALSE);
}

/*-------------------------------------------------------------------
 *
 * Function: is_rd_wr_oper_done()
 * 
 * This function check if the done bit is set. It will also clear 
 * the done bit before returning.
 *
 * Input: spi_prom    - Pointer to the spi prom register file
 *
 * Output: TRUE if the read write operation is done, FALSE otherwise
 *
 *-------------------------------------------------------------------
 */
int is_rd_wr_op_done(prom_t *spi_prom)
{
    ulong count = 0;

    do {
        /* If done bit is set, clear it and return TRUE */
        if (spi_prom->status & DSWAP4(PROM_OPER_DONE)) {
        /* Clear the Done bit */
           spi_prom->status |= DSWAP4(PROM_OPER_DONE);
           return (TRUE);
         }
         msleep(1); /* Sleep 1ms */
    } while (count++ < SPI_PROM_MAX_WAIT);
    return (FALSE);
}

 
/*-------------------------------------------------------------------
 *
 * Function: read_spi_prom_status_reg
 * 
 * This function read the status register (RDSR) of the SPI PROM
 * device.
 *
 * Input: uchar pointer where the rdsr to be stored
 *
 * Output: PASSED if the read succeed, FAILED otherwise
 *
 *-------------------------------------------------------------------
 */
int read_spi_prom_status_reg (uchar *rdsr)
{
    ulong rd_val;
    prom_t *spi_prom;

    spi_prom = init_spi_prom_ds();

    /* Read the SPI PROM Read Status Register (RDSR) */
    spi_prom->size        = DSWAP4(PROM_RD_1_BYTE & PROM_RD_SIZE_MASK);
    spi_prom->opcode_addr = DSWAP4(PROM_RDSR_OP);
    spi_prom->control     = DSWAP4(PROM_DFLT_BAUD); /* Read */

    /* Check if operation completed */
    if (!is_rd_wr_op_done(spi_prom)) {
        cterr('f',0,"%s: read operation not done", __FUNCTION__);
        return (FAILED);
    }

    rd_val = spi_prom->data;
    *rdsr  = (uchar)DSWAP4(rd_val);

#ifdef DEBUG
    printf("\nSPI PROM RDSR = %#.2x (%#.8x)\n", *rdsr, DSWAP4(rd_val));
#endif

    return (PASSED);
}

/*-------------------------------------------------------------------
 *
 * Function: read_prom_status_reg()
 * 
 * This function is a utility to read the SPI PROM status register.
 *
 * Input: dummy - not use but needed since it's called from menu
 *
 * Output: PASSED if the read succeed, FAILED otherwise
 *
 *-------------------------------------------------------------------
 */
int read_prom_status_reg (int dummy)
{
    uchar rdsr;

    if (read_spi_prom_status_reg(&rdsr) == FAILED) {
        cterr('f', 0, "Unable to read SPI PROM status register (RDSR)\n");
        return (FAILED);
    }
    printf("\nSPI PROM RDSR = %#.2x\n", rdsr);

    return (PASSED);
}

 
/*-------------------------------------------------------------------
 *
 * Function: prom_sector_erase()
 * 
 * This function will erase a sector within the SPI PROM
 *
 * Input: spi_prom - Pointer to the spi prom register file.
 *        sector   - sector number to erase.
 *
 * Output: PASSED if erase succeed, FAILED, otherwise
 *
 *-------------------------------------------------------------------
 */
int prom_sector_erase (prom_t *spi_prom, ushort sector)
{
    ulong opcode;

    /* Ensure that the SPI PROM has write enabled */ 
    if (device_write_enable(spi_prom) == FAILED) {
        cterr('f', 0, "%s: Unable to enable SPI device for write", 
              __FUNCTION__);
        return (FAILED);
    }

    /* Do the actual erase */
    opcode = (((sector * SPI_PROM_SECTOR_SIZE) & PROM_SPI_ADDR_MASK) | 
              PROM_SECT_ERASE_OP);
    spi_prom->opcode_addr = DSWAP4(opcode);
    spi_prom->control     = DSWAP4(PROM_DFLT_BAUD | PROM_DATA_WRITE |
                                   PROM_USE_ADDR);

    /* Check if operation completed */
    if (!is_rd_wr_op_done(spi_prom)) {
        cterr('f',0,"%s: read/write operation not done", __FUNCTION__);
        return (FAILED);
    }

    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("\n Sector number %d [start addr(0x%x)] should now be erased.", 
               sector, (sector * SPI_PROM_SECTOR_SIZE));
    } else {
        prpass(testpass, "erasing sector %d", sector);
    }

    /* Disable Write Operation */
    if (device_write_disable(spi_prom) == FAILED) {
        cterr('f', 0, "%s: Unable to disable write operation on the SPI "
              "device", __FUNCTION__);
        return (FAILED);
    }
    return (PASSED);
}

/*-------------------------------------------------------------------
 *
 * Function: prom_bulk_erase()
 * 
 * This function will erase the whole SPI PROM using chip erase op code
 *
 * Input: spi_prom - Pointer to the spi prom register file.
 *
 * Output: PASSED if erase succeed, FAILED, otherwise
 *
 *-------------------------------------------------------------------
 */
int prom_bulk_erase (prom_t *spi_prom)
{
    /* Ensure that the SPI PROM has write enabled */ 
    if (device_write_enable(spi_prom) == FAILED) {
        cterr('f', 0, "%s: Unable to enable SPI device for write", 
            __FUNCTION__);
        return (FAILED);
    }

    /* Do the actual erase */
    spi_prom->opcode_addr = DSWAP4(PROM_BULK_ERASE_OP);
    spi_prom->control     = DSWAP4(PROM_DFLT_BAUD | PROM_DATA_WRITE);

    /* Check if operation completed */
    if (!is_rd_wr_op_done(spi_prom)) {
        cterr('f',0,"%s: read/write operation not done", __FUNCTION__);
        return (FAILED);
    }

    /* Disable Write Operation */
    if (device_write_disable(spi_prom) == FAILED) {
        cterr('f', 0, "%s: Unable to disable write operation on the SPI "
              "device", __FUNCTION__);
        return (FAILED);
    }
    return (PASSED);
}

/*-------------------------------------------------------------------
 *
 * Function: bulk_erase_and_verify()
 * 
 * This function will erase the whole SPI PROM and verify
 * that it indeed erased successfully
 *
 * Input: spi_prom - Pointer to the spi prom register file.
 *
 * Output: PASSED if erase and verify succeed, FAILED, otherwise
 *
 *-------------------------------------------------------------------
 */
int bulk_erase_and_verify (prom_t *spi_prom)
{
    ushort sector, wait_sec, max_wait;

#ifdef DEBUG
    max_wait = getdec_answer("\nHow long to wait for bulk erase (0-160)?", 
			     30, 0, 160);
#else
    max_wait = BULK_ERASE_TIME; /* Wait for 40 seconds */
#endif

    /* Erase the whole chip */
    printf("\nBulk Erase the entire BIOS PROM\n");
    if (prom_bulk_erase(spi_prom) == FAILED) {
        cterr('f', 0, "Bulk Erase operation failed");
        return (FAILED);
    }
    
    /* Poll for erase completion - look for WIP bit clear.
     * Note: There is a bug in the chip that it returns WIP = 0
     * immediately after sending the bulk erase command even
     * before it's done, so we can't trust this now. The typical
     * time it takes for a successful erase is around 68s, and
     * the worse case is 160s.
     */
    if (!is_spi_prom_rdy(SPI_PROM_ERASE_WAIT)) {
        cterr('f', 0, "Bulk erase operation not completed");
        return (FAILED);
    }
    
    /* 
     * Sleep 30s before starting to blank check since we can't rely on the 
     * WIP bit. The rest of the time needed to actually finish bulk erase 
     * is buried in the time we are doing the blank check below. The total
     * time it takes to wait 40s + blank check is around 89s so we have
     * around 20s margin.
     */
    for (wait_sec = 0; wait_sec < max_wait; wait_sec++) {
        printf(".");
        fflush(stdout);
        msleep(1000); /* Sleep 1s */
    }

    printf("\nChip erase completed. Blank Check ...");
    for (sector = 0; sector <= MAX_SECTOR_NUM; sector++) {
        if (!(sector % 64)) {
            printf("\n");
        }
        printf(".");
        fflush(stdout);

        /* Verify sector */
        if (verify_sector_erase(spi_prom, sector) == FAILED) {
            cterr('f', 0, "Verify erase failed on sector %d", sector);
            return (FAILED);
        }
    }
    return (PASSED);
}

/*-------------------------------------------------------------------
 *
 * Function: prom_sector_erase_and_verify()
 * 
 * This function will erase a sector within the SPI PROM and verify
 * that it's erased successfully
 *
 * Input: spi_prom - Pointer to the spi prom register file.
 *        sector   - sector number to erase and verify.
 *
 * Output: PASSED if erase and verify succeed, FAILED, otherwise
 *
 *-------------------------------------------------------------------
 */
int prom_sector_erase_and_verify (prom_t *spi_prom, ushort sector)
{
    /* Erase sector */
    if (prom_sector_erase(spi_prom, sector) == FAILED) {
        cterr('f', 0, "Erase operation failed on sector %d", sector);
        return (FAILED);
    }
    
    /* Poll for erase completion - look for WIP bit clear */
    if (!is_spi_prom_rdy(SPI_PROM_ERASE_WAIT)) {
        cterr('f', 0, "Erase not completed on sector %d", sector);
        return (FAILED);
    }

    /* Verify sector */
    if (verify_sector_erase(spi_prom, sector) == FAILED) {
        cterr('f',0,"Verify erase failed on sector %d", sector);
        return (FAILED);
    }
    return (PASSED);
}

 
/*-------------------------------------------------------------------
 *
 * Function: device_write_disable()
 * 
 * This function will disable write operation to the SPI PROM device
 *
 * Input: spi_prom - Pointer to the spi prom register file.
 *
 * Output: PASSED if operation succeeds, FAILED otherwise
 *
 *-------------------------------------------------------------------
 */
static int device_write_disable (prom_t *spi_prom)
{
    /* Write disable the device */
    spi_prom->opcode_addr = DSWAP4(PROM_WRDI_OP);
    spi_prom->control     = DSWAP4(PROM_DFLT_BAUD | PROM_DATA_WRITE);

    /* Check if operation completed */
    if (!is_rd_wr_op_done(spi_prom)) {
	cterr('f', 0, "%s: read/write operation not done", __FUNCTION__);
	return (FAILED);
    }
    return (PASSED);
}


/*-------------------------------------------------------------------
 *
 * Function: device_write_enable()
 * 
 * This function will enable writes to the SPI PROM device
 *
 * Input: spi_prom - Pointer to the spi prom register file.
 *
 * Output: PASSED if enabled successfully, FAILED otherwise
 *
 *-------------------------------------------------------------------
 */
static int device_write_enable (prom_t *spi_prom)
{
    /* Write enable the device */
    spi_prom->opcode_addr = DSWAP4(PROM_WREN_OP);
    spi_prom->control     = DSWAP4(PROM_DFLT_BAUD | PROM_DATA_WRITE);

    /* Check if operation completed */
    if (!is_rd_wr_op_done(spi_prom)) {
	cterr('f', 0, "%s: write operation not done", __FUNCTION__);
	return (FAILED);
    }
    return (PASSED);
}


/*-------------------------------------------------------------------
 *
 * Function: spi_prom_rdy()
 * 
 * This function will ensure that the SPI PROM device is ready
 * to be written to, and not in the middle of a write cycle (WIP).
 *
 * Input: Maximum wait time in ms
 *
 * Output: TRUE if the SPI PROM is ready for write, FALSE otherwise
 *
 *-------------------------------------------------------------------
 */
int is_spi_prom_rdy (unsigned int max_wait)
{
    ulong count = 0;
    uchar rdsr;

    /* Sanity check */
    if (max_wait == 0)
	max_wait = SPI_PROM_MAX_WAIT;

    do {
	if (read_spi_prom_status_reg(&rdsr) == FAILED) {
	    cterr('f', 0, "Unable to read SPI PROM status register (RDSR)\n");
	    return (FALSE);
	}

	if (!(rdsr & PROM_RDSR_WIP)) {
            return (TRUE);
	}
        msleep(1); /* Wait 1ms */
    } while (count++ < max_wait);

    return (FALSE);
}

 
/*-------------------------------------------------------------------
 *
 * Function: write_spi_prom_status_reg
 * 
 * This function writes a byte to the SPI PROM status register.
 *
 * Input: spi_prom - Pointer to the spi prom register file.
 *        wr_byte  - The char value to write to the RDSR
 *
 * Output: PASSED if operation succeeded, FAILED otherwise
 *
 *-------------------------------------------------------------------
 */
static int write_spi_prom_status_reg (prom_t *spi_prom, uchar wr_byte)
{
    ulong wr_val = (ulong)wr_byte;

    /* Ensure that the SPI PROM has write enabled */ 
    if (device_write_enable(spi_prom) == FAILED) {
	cterr('f', 0, "%s: Unable to enable SPI device for write", 
	      __FUNCTION__);
	return (FAILED);
    }
    spi_prom->size        = DSWAP4(PROM_RD_1_BYTE & PROM_RD_SIZE_MASK);
    spi_prom->opcode_addr = DSWAP4(PROM_WRSR_OP);
    spi_prom->data        = DSWAP4(wr_val);
    spi_prom->control     = DSWAP4(PROM_DFLT_BAUD | PROM_DATA_WRITE);

    /* Check if operation completed */
    if (!is_rd_wr_op_done(spi_prom)) {
	cterr('f', 0, "%s: write operation not done", __FUNCTION__);
	return (FAILED);
    }

    /* Disable Write Operation */
    if (device_write_disable(spi_prom) == FAILED) {
	cterr('f', 0, "%s: Unable to disable write operation on the SPI "
	      "device", __FUNCTION__);
	return (FAILED);
    }
    return (PASSED);
}

/*-------------------------------------------------------------------
 *
 * Function: write_prom_status_reg
 * 
 * This function is a utility to write a byte to the SPI PROM status 
 * register.
 *
 * Input: dummy - not use but needed since it's called from menu
 *
 * Output: PASSED if the write succeed, FAILED otherwise
 *
 *-------------------------------------------------------------------
 */
int write_prom_status_reg (int dummy)
{
    uchar wrsr;
    prom_t *spi_prom;

    spi_prom = init_spi_prom_ds();

    wrsr = (uchar)gethex_answer("Enter byte to write to WRSR: ", 0, 0, 0xFF);

    if (write_spi_prom_status_reg(spi_prom, wrsr) == FAILED) {
	cterr('f', 0, "Unable to write SPI PROM status register (WRSR)");
	return (FAILED);
    }
    printf("Successfully wrote %#.2x to SPI PROM WRSR register\n", wrsr);

    return (PASSED);
}


 
/*-------------------------------------------------------------------
 *
 * Function: init_spi_prom_ds
 * 
 * This function will initialize the spi_prom_reg data stricture for
 * use in this file.
 *
 * Input: none.
 *
 * Output: Pointer to the spi prom register file
 *
 *-------------------------------------------------------------------
 */
static prom_t *init_spi_prom_ds (void)
{ 
    spi_prom = (prom_t *)get_platform_prom_addr();
    p_spi_prom_info = &spi_eprom_info[0];

    return spi_prom;
    
}

 
/*-------------------------------------------------------------------
 *
 * Function: verify_sector_erase
 * 
 * This function will verify that the sector of SPI PROM device that 
 * we want to program is blank. It does this by comparing its contents 
 * to the erased pattern of 0xff. 
 *
 * Input: spi_prom - Pointer to the spi prom register file.
 *        sector   - sector number to verify
 *
 * Output: PASSED if sector is blank as expected, FAILED otherwise
 *
 *-------------------------------------------------------------------
 */
static int verify_sector_erase (prom_t *spi_prom, ushort sector)
{
    volatile ulong start_addr, end_addr;
    volatile uchar data[PROM_RD_MAX_BYTE];
    ushort ix, err_count = 0;
    ulong temp_data;

    if (!is_read_fifo_empty(spi_prom)) {
        cterr('f',0,"SPI PROM Read Fifo is not empty");
        return (FAILED);
    }

    start_addr = sector * SPI_PROM_SECTOR_SIZE;
    end_addr   = start_addr + SPI_PROM_SECTOR_SIZE - 1; 

    while (start_addr < end_addr) {
        spi_prom->opcode_addr = DSWAP4(PROM_READ_OP | start_addr);
        spi_prom->size        = DSWAP4(PROM_RD_256_BYTE);
        spi_prom->control     = DSWAP4(PROM_DFLT_BAUD | PROM_USE_ADDR);
 
	/* Check if operation completed */
	if (!is_rd_wr_op_done(spi_prom)) {
	    cterr('f',0,"%s: read operation not done", __FUNCTION__);
	    return (FAILED);
	}

	/* Read and compare */
        for (ix = 0; ix < PROM_RD_MAX_BYTE; ix++) {
	    temp_data = spi_prom->data;
	    data[ix] = (uchar) DSWAP4(temp_data);

            if (data[ix] != PROM_BLANK_DATA) {
		printf("\n*** ERROR at addr %#.8lx: PROM = %#.2x, expected = "
		       "%#.2x", start_addr + ix, data[ix], PROM_BLANK_DATA);
		err_count++;

                if (err_count > PROM_MAX_ERR_CNT) {
		    cterr('f', 0, "Exit due to too many mis-matches.");
                    return (FAILED);
                }
            }
	}
	/* Go to the next 256-byte block */
        start_addr = (start_addr + PROM_RD_MAX_BYTE);
    }

    if ((!(NVRAM)->diagflag) & D_VERBOSE) {
	printf("\n Sector %d [start addr(0x%x) end_addr(0x%lx)] Erase Verified "
	       "OK.\n", sector, (sector * SPI_PROM_SECTOR_SIZE), end_addr);
    }

    return (PASSED);
}

 
/*-------------------------------------------------------------------
 *
 * Function: verify_download_image()
 * 
 * This function will verify that the requested image was downloaded into 
 * the SPI PROM correctly by comparing its contents to the contents of
 * given image.
 *
 * Input: spi_prom    - Pointer to the spi prom register file.
 *        start_addr  - Starting address within the SPI PROM.
 *        end_addr    - Ending address within the SPI PROM. 
 *        src_img     - Pointer to the source image to compare with
 *        bit_reverse - TRUE if data needs to be bit-reversed before comparing
 *                      FALSE otherwise
 *
 * Output: PASSED if the SPI PROM content is the same as the image,
 *         FAILED otherwise.
 *
 *-------------------------------------------------------------------
 */
int verify_download_image (prom_t *spi_prom, ulong start_addr, 
                           ulong end_addr, uchar *src_img, boolean bit_reverse)
{
    volatile uchar data[PROM_RD_MAX_BYTE];
    ushort ix, err_count = 0;
    ulong img_idx = 0, temp_data, count, ctrl_flag;

    if (!is_read_fifo_empty(spi_prom)) {
	cterr('f',0,"SPI PROM Read Fifo is not empty");
	return (FAILED);
    }

    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("\nVerify image contents from %#.8lx to %#.8lx\n", start_addr,
               end_addr);
    }

    /* 
     * FPGA image data was bit-reversed before programming, all other image
     * and headers weren't
     */
    if (bit_reverse) {
	ctrl_flag = PROM_DFLT_BAUD | PROM_USE_ADDR | PROM_SWAP_DATA_BITS;
    } else {
	ctrl_flag = PROM_DFLT_BAUD | PROM_USE_ADDR;
    }

    while (start_addr < end_addr) {
        /* Handle case where the compare size is not multiple of 256 bytes */
        if ((end_addr - start_addr) < PROM_RD_MAX_BYTE) {
	    count = end_addr - start_addr;
        } else {
            count = PROM_RD_256_BYTE;
        }

        spi_prom->opcode_addr = DSWAP4(PROM_READ_OP | start_addr);
        spi_prom->size        = DSWAP4(count);
        spi_prom->control     = DSWAP4(ctrl_flag);

	/* Check if operation completed */
	if (!is_rd_wr_op_done(spi_prom)) {
	    cterr('f',0,"%s: read operation not done", __FUNCTION__);
	    return (FAILED);
	}

	/* Comparing with the original image data in 256-byte block */
        for (ix = 0; ix <= count; ix++) {
	    temp_data = spi_prom->data;
	    data[ix] = (uchar)(DSWAP4(temp_data));

            if (data[ix] != src_img[img_idx]) {
		printf("\n *** ERROR at addr %#.8lx: PROM =%#.2x FILE=%#.2x ", 
		       start_addr + ix, data[ix], src_img[img_idx]);
		err_count++;

                if (err_count > PROM_MAX_ERR_CNT) {
		    cterr('f', 0, "Exit due to too many mis-matches.");
                    return (FAILED);
                }
            }
	    img_idx++;
	}
	/* Go to the next  block */
        start_addr = start_addr + count + 1;

	/* Print progress indication at every sector boundary */
        if(!(start_addr % SPI_PROM_SECTOR_SIZE)) {
	    printf(".");
            fflush(stdout);
        }
    }

    if (err_count) {
        cterr('f', 0, "SPI PROM image programming verification failed with "
	      "%d errors\n", err_count);
        return (FAILED);
    } 

    return (PASSED);
}

/*-------------------------------------------------------------------
 *
 * Function: verify_download_sector()
 * 
 * This function is similar to the verify_download_image() except
 * that it verifies by sector, not arbitrary address. It will verify that 
 * the image downloaded into the SPI PROM device at a particular sector is 
 * correct by comparing its contents to the contents of the given image.
 *
 * Input: spi_prom    - Pointer to the spi prom register file
 *        sector      - sector number to compare
 *        src_img     - Pointer to the beginning of the source image
 *                      (Without adjusting for sector boundary)
 *        bit_reverse - TRUE if data needs to be bit-reversed before comparing
 *                      FALSE otherwise
 *
 * Output: PASSED if the SPI PROM content of the request sector is the same 
 *         as the given image, FAILED otherwise.
 *
 *-------------------------------------------------------------------
 */
int verify_download_sector (prom_t *spi_prom, ushort sector, 
			                uchar *src_img, boolean bit_reverse)
{
    volatile ulong end_addr, start_addr;

    start_addr = sector * SPI_PROM_SECTOR_SIZE;
    end_addr   = start_addr + SPI_PROM_SECTOR_SIZE - 1; 

    return (verify_download_image(spi_prom, start_addr, end_addr, 
				  &src_img[start_addr], bit_reverse));
}


/*-------------------------------------------------------------------
 *
 * Function: reggio_fpga_update()
 * 
 * This function is the low level code that will do the actual programming 
 * of the SPI PROM device. Once the device has been programmed the upgrade 
 * will not take place until a power cycle.
 *
 * Input: type - 2 for upgrade image, 1 for golden image
 *        intractv - TRUE if user has to confirm before programming
 *                   FALSE otherwise
 *
 * Output: PASSED if program successfully, FAILED otherwise
 *
 *-------------------------------------------------------------------
 */
static int reggio_fpga_update (uchar type, boolean intractv)
{

    ushort sector;
    prom_t *spi_prom;

    testname("FPGA programming");

    
    fpga_fw = get_platform_fpga_fw();
    fpga_fw_size = get_platform_fpga_size();
    fflush(stdout);

    spi_prom = init_spi_prom_ds();

    /* Get info of the FPGA image to be programmed */
    if (get_fpga_programming_info(&fpga_info, type, TRUE) == FAILED) {
        cterr('f', 0, "Unknown FPGA type. No action taken.");
        return (FAILED);
    }
    /* Unprotect the SPI PROM to allow write/erase */
    if (write_spi_prom_status_reg(spi_prom, PROM_UNPROTECT_ALL) == FAILED) {
        cterr('f', 0, "Failure to unprotect the SPI PROM to allow write");
        return (FAILED);
    }
    /* Check if SPI PROM is ready for write */
    if (!is_spi_prom_rdy(SPI_PROM_MAX_WAIT)) {
        cterr('f', 0, "SPI PROM is not ready for write");
        return (FAILED);
    }
    printf("\nErasing SPI PROM from sectors %d to %d.\n",
           fpga_info.start_sector, fpga_info.end_sector);


    for (sector = fpga_info.start_sector; sector <= fpga_info.end_sector; 
         sector++) {
        if (prom_sector_erase_and_verify(spi_prom, sector) == FAILED) {
            return (FAILED);
        }
    }

    printf("\nProgramming SPI PROM [0x%08lx to 0x%08lx]\n", 
	   fpga_info.start_addr, fpga_info.end_addr);

    if (prom_image_program(spi_prom, fpga_info.start_addr, fpga_info.end_addr, 
			   fpga_info.fpga_fw, FALSE) == FAILED) { 
        cterr('f', 0, "%s: SPI PROM image programming failed.");
        return (FAILED);
    }

    printf("\nVerifying ...\n");
    if (verify_download_image(spi_prom, fpga_info.start_addr, 
		fpga_info.end_addr, fpga_info.fpga_fw, FALSE)  
        == FAILED) {
    	cterr('f',0,"Verify of FPGA image failed.");
        return (FAILED);
    }
    
    
    return (PASSED);
}

/*-------------------------------------------------------------------
 *
 * read_spi_prom_header()
 *
 * This function will do the actual read of the header from SPI PROM.
 *
 * Input:   start_adress - Starting address of the prom header
 *          hdr_p - Pointer to buffer to store the Reggio SPI PROM header 
 *          hdr_size - Size of the header
 * 
 * Output: PASSED if completed successfully, FAILED otherwise
 *
 *-------------------------------------------------------------------
 */
static int read_spi_prom_header(int start_addr, uchar *hdr_p, uint hdr_size)
{
    int ix;
    prom_t *spi_prom;

    spi_prom = init_spi_prom_ds();

    /* Check if SPI PROM is ready for write */
    if (!is_spi_prom_rdy(SPI_PROM_MAX_WAIT)) {
	cterr('f', 0, "SPI PROM is not ready for write");
	return (FAILED);
    }

    /* Read hdr_size bytes from start_addr */
    spi_prom->opcode_addr = DSWAP4((PROM_READ_OP | start_addr));
    spi_prom->size        = DSWAP4(hdr_size);
    spi_prom->control     = DSWAP4(PROM_DFLT_BAUD | PROM_USE_ADDR);

    /* Check if operation completed */
    if (!is_rd_wr_op_done(spi_prom)) {
	cterr('f',0,"%s: read operation not done", __FUNCTION__);
	return (FAILED);
    }

    for (ix = 0; ix < hdr_size; ix++) {
	*hdr_p++ = (uchar) (DSWAP4(spi_prom->data));
    }
    return (PASSED);
}


/*-------------------------------------------------------------------
 *
 * Function: debug_reggio_erase_spi_prom()
 * 
 * This is a debug function to erase any sector of the SPI PROM
 *
 * Input: - none
 *
 * Output: PASSED if erased successfully, FAILED otherwise
 *
 *-------------------------------------------------------------------
 */
int 
debug_reggio_erase_spi_prom (void)
{ 
    ushort sector;
    ulong start_addr;
    ulong end_addr;
    prom_t *spi_prom;

    spi_prom = init_spi_prom_ds();

    sector = getdec_answer("\nEnter sector number to erase (0-127)> ", 64, 0, 
			   MAX_SECTOR_NUM);

    start_addr = sector * SPI_PROM_SECTOR_SIZE;
    end_addr   = start_addr + SPI_PROM_SECTOR_SIZE - 1; 

    printf("\nYou have chose to erase sector %d (%#.8lx - %#.8lx)\n", sector,
	   start_addr, end_addr);

    if (getc_answer("\nAre you sure you want to continue? (y/n)", "yn", 'y') 
	== 'n') {
        printf("\nNo action taken.");
        return (PASSED);
    }

    /* Unprotect the SPI PROM to allow write/erase */
    if (write_spi_prom_status_reg(spi_prom, PROM_UNPROTECT_ALL) == FAILED) {
	cterr('f', 0, "Failure to unprotect the SPI PROM to allow write");
	return (FAILED);
    }
    /* Check if SPI PROM is ready for write */
    if (!is_spi_prom_rdy(SPI_PROM_MAX_WAIT)) {
        cterr('f', 0, "SPI PROM is not ready for write");
        return (FAILED);
    }
    printf("\nErasing sector %d ...\n", sector);
    if (prom_sector_erase_and_verify(spi_prom, sector) == FAILED) {
	return (FAILED);
    }

    printf("\nSector %d should have been erased.\n", sector);

    return (PASSED);
}

 
/*-------------------------------------------------------------------
 *
 * Function: display_reggio_spi_prom_regs()
 * 
 * This function displays the SPI PROM registers within the Reggio FPGA.
 *
 * Input: - none.
 *
 * Output:- Always return PASSED to avoid compiler warning
 *
 *-------------------------------------------------------------------
 */
int display_reggio_spi_prom_regs (void)
{ 
    prom_t *spi_prom;

    spi_prom = init_spi_prom_ds();

    printf("\n [%p] SPI PROM Control register   = %#.8x", 
	   &spi_prom->control, DSWAP4(spi_prom->control)); 
    printf("\n [%p] SPI PROM Status register    = %#.8x", &spi_prom->status,
	   DSWAP4(spi_prom->status)); 
    printf("\n [%p] SPI PROM Read register      = %#.8x", &spi_prom->size,
	   DSWAP4(spi_prom->size));
    printf("\n [%p] SPI PROM Data register      = %#.8x", &spi_prom->data,
	   DSWAP4(spi_prom->data));
    printf("\n [%p] SPI Opcode/Address register = %#.8x", 
	   &spi_prom->opcode_addr, DSWAP4(spi_prom->opcode_addr));

    return (PASSED);
} 

/*-------------------------------------------------------------------
 *
 * Function: diag_fpga_ttf2array()
 * 
 * This function will open ttf file, read its contents, swap bits,
 * and stored in array.
 *
 * Input:  char * file. fpag file.
 *         uint size, size of fpga.
 *
 * Output: PASSED/FAILED
 *
 *-------------------------------------------------------------------
 */
static int diag_fpga_ttf2array (char *file, uint *read_size)
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
    dash_fpga_fw_size = size;
    fclose(fp);

    dash_fpga_fw_array = malloc(dash_fpga_fw_size + 1000);
    printf("FPGA_FW = %s\n", dash_fpga_fw_array);
    if (dash_fpga_fw_array == NULL) {
        printf("%s: Can't allocate memory for FPGA\n", __func__);
        fflush(stdout);
        dash_fpga_fw_size = 0;
        return (FAILED);
    }
    tmp = dash_fpga_fw_array;
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


/*
 *********************************************************************
 *
 * Function   : get_fpga_firmware
 * Description: This function opens fpga file and stores into
 *                  dash_fpga_fw_array variable.
 * Inputs     : char * file. fpag file
 *          
 * Outputs    : PASSED, if successful opening and storing dash fpga
 *              into dash_fpag_fw_arry. FAILED otherwise.
 *
 *********************************************************************
 */
int get_fpga_firmware (char *file)
{
    uint read_size;

    /* usage: utah_lnx size=file_size file_name; if file_size=='d',
       then use default value , or
       utah_lnx -f file_name -s file_size */
    printf("Dash file name is %s\n", file);

    printf("Converting TTF to array...");
    fflush(stdout);
    if (diag_fpga_ttf2array(file, &read_size) == FAILED) {
        printf("FAILED! Exiting...\n");
        return (FAILED);
    } else {
        printf("OK! (Read size = %d)\n", read_size);
    }
  
    return (PASSED);
}


/*-------------------------------------------------------------------
 *
 * Function: program_reggio_spi_prom_old()
 * 
 * This function is a wrapper function to program the FPGA image to
 * the FPGA SPI PROM. It will be done in interactive mode.
 *
 * Inputs     : int header - option for there is header or not
 *
 * Output: PASSED/FAILED
 *
 *-------------------------------------------------------------------
 */
int program_reggio_spi_prom_old (int header)
{
    int status;
    int image_type = header?FPGA_UPGRADE_IMAGE:FPGA_GOLDEN_IMAGE;
    char   ans;
    char fw_file[128];
  

    sprintf(fw_file, "%s", FPGA_DEFAULT_FILE_PATH);
    if (get_fpga_firmware(fw_file)==FAILED) {
		return (FAILED);
    }

    /* check to make sure we have a valid firmware file */
    get_platform_fpga_fw();

    if (image_type == FPGA_UPGRADE_IMAGE) {
        printf("upgrade image header will be programmed with\n");
        printf("board revision = %d; major = %d; minor = %d; debug = %d;\n",
               brd_rev, major, minor, debug);
        printf("hour = %d; date = %d; month = %d; year = %d;\n",
               hour, date, month, year);
        printf("update flag = %#x\n", update_flag | NEWER_UPGRADE_A0);
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
    
    if (header) {
        /* Erase the header sector */
        if (erase_header_spi_prom_image(FPGA_UPGRADE_IMAGE, TRUE)==FAILED) {
            return (FAILED);
        }
    }

    /* Program in interactive mode */
    if (reggio_fpga_update(image_type, TRUE)==FAILED) {
        return (FAILED);
    }

    if (header) {
        /* Program FPGA image header */
        status = (program_spi_prom_image_header(image_type, TRUE));
    }

    if ((image_type == FPGA_UPGRADE_IMAGE)) {
#ifdef NEED_FSM
        set_fpga_reconf_fsm();
        printf("\n****If Reconfiguration FSM not reset, please check revision in programmed header.******\n");
        printf("\n****The revision in programmed header should greater than FPGA Golden Image revision, then it will do Reconfiguration FSM reset.******\n");
#endif
    }

    printf("\n\n****Please power cycle for the new FPGA to take effect.******\n\n");
    
    return (status);

}

/*-------------------------------------------------------------------
 *
 * Function: erase_header_spi_prom_image
 * 
 * This function will clear the gold or upgraded image header with 0.
 * Once the header has been cleared the changes will not take place 
 * until after a power cycle.
 *
 * Input:  type    - 2 for UPDATE, 1 for GOLDEN.
 *         verbose - TRUE to print more info, FALSE to suppress
 *
 * Output: PASSED/FAILED
 *
 *-------------------------------------------------------------------
 */
int erase_header_spi_prom_image (uchar type, boolean verbose)
{ 
    uchar sector;
    char hdr_str[MAX_STR_SIZE];
    prom_t *spi_prom;

    spi_prom = init_spi_prom_ds();

    sector = UPGRADE_MULTI_BOOT_SECTOR;
    sprintf(hdr_str, "multiboot");


    /* Unproxtect the SPI PROM to allow write/erase */
    if (write_spi_prom_status_reg(spi_prom, PROM_UNPROTECT_ALL) == FAILED) {
	    cterr('f', 0, "Failure to unprotect the SPI PROM to allow write");
	    return (FAILED);
    }
    /* Check if SPI PROM is ready for write */
    if (!is_spi_prom_rdy(SPI_PROM_MAX_WAIT)) {
        cterr('f', 0, "SPI PROM is not ready for write");
        return (FAILED);
    }
    
    if (verbose) {
        printf("\nErasing %s image header at sectors %d\n", hdr_str, sector);
    }

    if (prom_sector_erase_and_verify(spi_prom, sector) == FAILED) {
        return (FAILED);
    }

    return (PASSED);
}

/*-------------------------------------------------------------------
 *
 * Function: erase_secure_boot_header()
 * 
 * This function will erase sectors 61 for Golden image and 125 for upgrade 
 * image that contains the secure boot hashes. The content of these sectors 
 * will be set back to 0xff.
 *
 * Input:  type    - 2 for UPDATE, 1 for GOLDEN.
 *         verbose - TRUE to print more info, FALSE to suppress
 *
 * Output: PASSED/FAILED
 *
 *-------------------------------------------------------------------
 */
static int erase_secure_boot_header (uchar type, boolean verbose)
{ 
    uchar sector;
    prom_t *spi_prom;

    spi_prom = init_spi_prom_ds();

    if (type == FPGA_UPGRADE_IMAGE) {
	    sector = UPGRADE_HASH_SECTOR;
    } else {  /* Golden image */
	    sector = GOLDEN_HASH_SECTOR;
    }

    /* Unprotect the SPI PROM to allow write/erase */
    if (write_spi_prom_status_reg(spi_prom, PROM_UNPROTECT_ALL) == FAILED) {
	    cterr('f', 0, "Failure to unprotect the SPI PROM to allow write");
	    return (FAILED);
    }
    /* Check if SPI PROM is ready for write */
    if (!is_spi_prom_rdy(SPI_PROM_MAX_WAIT)) {
        cterr('f', 0, "SPI PROM is not ready for write");
        return (FAILED);
    }
    if (verbose) {
	    printf("\nErasing sectors %d\n", sector);
    }

    if (prom_sector_erase_and_verify(spi_prom, sector) == FAILED) {
	    return (FAILED);
    }
    return (PASSED);
}


/*-------------------------------------------------------------------
 *
 * Function: program_spi_prom_image_header()
 * 
 * This function is the low level code that will do the actual programming 
 * of the SPI PROM device. Once the device has been programmed the upgrade 
 * will not take place until a power cycle.
 *
 * Input:  type - 2 for UPDATE, 1 for GOLDEN.
 *         verbose - TRUE to print more info, FALSE to suppress
 *
 * Output: PASSED/FAILED
 *
 *-------------------------------------------------------------------
 */
static int program_spi_prom_image_header (uchar type, boolean verbose)
{ 
    uint start_addr, end_addr;
    uint pgm_size = REGGIO_FPGA_HDR_SIZE;
    reggio_spi_prom_image_header_t fpga_header;
    prom_t *spi_prom;
    spi_prom = init_spi_prom_ds();
    printf("\n");

    if (verbose) {
        testname("image header programming");
    }

    /* 
     * If we reach here, we must already call program_spi_prom_image
     * and the spi_prom_info pointer are set already
     */
    start_addr = p_spi_prom_info->start_header;

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
  
    //    fpga_header.flags[0] = 0xA1;
    fpga_header.flags[0] = update_flag | NEWER_UPGRADE_A0; //always upgrade;
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
  
    /* Unprotect the SPI PROM to allow write/erase */
    if (write_spi_prom_status_reg(spi_prom, PROM_UNPROTECT_ALL) == FAILED) {
        cterr('f', 0, "Failure to unprotect the SPI PROM to allow write");
        return (FAILED);
    }
    /* Check if SPI PROM is ready for write */
    if (!is_spi_prom_rdy(SPI_PROM_MAX_WAIT)) {
        cterr('f', 0, "SPI PROM is not ready for write");
        return (FAILED);
    }

    printf("\nProgramming %d bytes of SPI PROM Header to %#x\n", 
              pgm_size, start_addr);

    if (prom_image_program(spi_prom, start_addr, end_addr, 
                          (uchar *)&fpga_header, FALSE) == FAILED) {
        cterr('f', 0, "%s: FPGA multi-boot header programming failed.");
        return (FAILED);
    }

    /* Verify header programming */
    if (verify_download_image(spi_prom, start_addr, end_addr, 
                             (uchar *)&fpga_header, FALSE) == FAILED) {
        cterr('f',0,"Verify of FPGA multi-boot header failed.");
        return (FAILED);
    }

    if (verbose) {
        printf("\nSPI PROM header Program/Verify OK.\n");
    }

    return (PASSED);
}

/*-------------------------------------------------------------------
 *
 * Function: program_secure_boot_header()
 * 
 * This function is the low level code that will do the actual programming 
 * of the secure boot hashes to SPI PROM device. Once the device has been 
 * programmed the upgrade will not take place until a power cycle.
 *
 * Input:  type  - 2 for UPDATE, 1 for GOLDEN.
 *         verbose - TRUE to print more info, FALSE to suppress
 *
 * Output: PASSED if programmed successfully, FAILED otherwise
 *
 *-------------------------------------------------------------------
 */
static int program_secure_boot_header (uchar type, boolean verbose)
{ 
    ulong hash_num, end_addr;
    secure_boot_hash_t *sboot_p;
    prom_t *spi_prom;

    printf("\n");
    if (verbose) {
        testname("Reggio secure boot header programming");
    }

    spi_prom = init_spi_prom_ds();

    /* Get image info of the FPGA image to be programmed */
    if(get_fpga_programming_info(&fpga_info, type, FALSE) == FAILED) {
        cterr('f', 0, "Unknown FPGA type. No action taken.");
        return (FAILED);
    }

    /* 
     * If we reach here, we must already call reggio_fpga_update()
     * and the spi_prom_info pointer are set already
     */
    switch(type) {
    case FPGA_UPGRADE_IMAGE:
        sboot_p = (secure_boot_hash_t *)upgrade_sboot_hash;
        sboot_p[0].hash_ptr = fpga_info.lh_upgrade_bitfile_hmac;
        sboot_p[1].hash_ptr = fpga_info.lh_upgrade_bitfile_header;
        break;
    case FPGA_GOLDEN_IMAGE:
        sboot_p = (secure_boot_hash_t *)golden_sboot_hash;
        sboot_p[0].hash_ptr = fpga_info.lh_golden_bitfile_hmac;
        sboot_p[1].hash_ptr = fpga_info.lh_golden_bitfile_header;
        break;
    default:
        cterr('f',0,"Unknown FPGA image type = %d", type);
        return (FAILED);
    }

    /* Erase the secure boot sector first */
    erase_secure_boot_header(type, verbose);

    /* Unprotect the SPI PROM to allow write/erase */
    if (write_spi_prom_status_reg(spi_prom, PROM_UNPROTECT_ALL) == FAILED) {
        cterr('f', 0, "Failure to unprotect the SPI PROM to allow write");
        return (FAILED);
    }
    /* Check if SPI PROM is ready for write */
    if (!is_spi_prom_rdy(SPI_PROM_MAX_WAIT)) {
        cterr('f', 0, "SPI PROM is not ready for write");
        return (FAILED);
    }
    /* Start programming Leghorn secure boot hashes */
    printf("\n");
    for (hash_num = 0; hash_num < SBOOT_HASH_NUM; hash_num++) {
        printf("Programming %d bytes of %s to %#8x\n", 
                sboot_p->hash_size, sboot_p->hash_name, sboot_p->hash_addr);
        end_addr = sboot_p->hash_addr + sboot_p->hash_size - 1;

        if (prom_image_program(spi_prom, sboot_p->hash_addr, end_addr, 
                               sboot_p->hash_ptr, FALSE) == FAILED) {
            cterr('f', 0, "%s: FPGA secure boot header programming failed.");
            return (FAILED);
        }

        /* Verify header programming */
        if (verify_download_image(spi_prom, sboot_p->hash_addr, end_addr, 
                                  sboot_p->hash_ptr, FALSE) == FAILED) {
            cterr('f',0,"Verify of FPGA secure boot header failed.");
            return (FAILED);
        }

        if (verbose)
            printf("%s Program/Verify OK.\n", sboot_p->hash_name);
        sboot_p++;
    }

    return (PASSED);
}

/*-------------------------------------------------------------------
 *
 * read_spi_prom_image_header()
 *
 * This function reads multi-boot image header of the SPI PROM image 
 *
 * Input: None
 *
 * Output: PASSED if read succeeded, FAILED otherwise
 *
 *-------------------------------------------------------------------
 */
int read_spi_prom_image_header (void)
{
    int header_type;
  
    header_type = gethex_answer("Which header do you want to read? [1: Gold, 2"
                                ": Upgrade]", 1, 1, 2);
  
    return (read_header_spi_prom_image (header_type));
}

/*-------------------------------------------------------------------
 *
 * program_secure_boot_hash()
 *
 * This function will program the secure-boot hashes to SPI PROM.
 *
 * Input: None
 *
 * Output: PASSED/FAILED
 *
 *-------------------------------------------------------------------
 */
int program_secure_boot_hash (void)
{
    int header_type;
  
    header_type = gethex_answer("Which secure boot header do you want to "
                                "program? [0: Exit 1: Gold, 2: Upgrade]", 1, 0, 2);
    if (!header_type) {
        printf("\n No Action Taken.\n");
        return (PASSED);
    }
  
    return (program_secure_boot_header(header_type, TRUE));
}

/*-------------------------------------------------------------------
 *
 * clear_spi_prom_image_header()
 *
 * This function will erase the sector that contains the multi-boot header
 *
 * Input: - None
 *
 * Output: PASSED/FAILED
 *
 *-------------------------------------------------------------------
 */
int clear_spi_prom_image_header (void)
{
    int header_type;
  
    header_type = gethex_answer("Which header do you want to clear? [0: Exit, "
				"1: Gold, 2: Upgrade]", 1, 0, 2);

    if (!header_type) {
	printf("\n No Action Taken.\n");
	return (PASSED);
    }
      
    if (erase_header_spi_prom_image(header_type, TRUE) != FAILED) {
	printf("\nSPI PROM %s header was cleared successfully.\n", 
	       (header_type == 1)? "golden":  "upgrade");
	return (PASSED);
    }
    return (FAILED);
}

/*-------------------------------------------------------------------
 *
 * clear_secure_boot_image_header()
 *
 * This function will erase the sector that contains the secure-boot
 * header.
 *
 * Input: - None
 *
 * Output: PASSED/FAILED
 *
 *-------------------------------------------------------------------
 */
int clear_secure_boot_image_header (void)
{
    int header_type;
  
    header_type = gethex_answer("Which Secure Boot header do you want to clear? "
                                "[0: Exit, 1: Gold, 2: Upgrade]", 1, 0, 2);

    if (!header_type) {
        printf("\n No Action Taken.\n");
        return (PASSED);
    }
      
    return (erase_secure_boot_header(header_type, TRUE));
}

/*-------------------------------------------------------------------
 *
 * read_header_spi_prom_image
 *
 * This function will read the image header of the GOLD or UPGRADE image 
 * within the SPI PROM device and display it on the screen.
 *
 * Input: type - Golden or Upgrade image 
 *
 * Output: PASSED if succeed, FAILED otherwise
 *
 *-------------------------------------------------------------------
 */
int read_header_spi_prom_image (uchar type)
{
    reggio_spi_prom_image_header_t header;
    uchar *hdr_p = (uchar *)&header;
    int pgm_size = REGGIO_FPGA_HDR_SIZE;
    volatile ulong i, start_addr;
    prom_t *spi_prom;

    printf("displayiong fpga header\n");
    spi_prom = init_spi_prom_ds();

    start_addr = p_spi_prom_info->start_header;

    /* Read the 64-byte header */
    if (read_spi_prom_header(start_addr, hdr_p, pgm_size) == FAILED)
	return (FAILED);

    /* Display header */
    for (i = 0; i < pgm_size; i++) {
	if (!(i % 16))
	    printf("\n");
	printf("%02x ", *hdr_p++);
    }
    return (PASSED);
}

/*-------------------------------------------------------------------
 *
 * get_fpga_programming_info()
 *
 * There are 5 different FPGA images for different revision of Calabria and
 * Altamont. This function is to populate the fpga_info structure with the
 * correct data base on the HW revision and board ID.
 *
 * Input: Pointer to the fpga_info structure
 *        type - image type: 1 for golden, 2 for upgraded image
 *        verbose - Print FPGA type if TRUE, FALSE to turn off printing
 *
 * Output: PASSED if succeeded, FAILED otherwise
 *
 *-------------------------------------------------------------------
 */
static int get_fpga_programming_info (reggio_fpga_prog_info_t *fpga_p, uchar type, 
			                          boolean verbose)
{
    /* Clear the fpga info structure */
    memset((char *)fpga_p, 0, sizeof(reggio_fpga_prog_info_t));

    fpga_p->fpga_fw = get_platform_fpga_fw();
    fpga_p->fpga_fw_size = get_platform_fpga_size();

    fpga_p->hdr_addr     = p_spi_prom_info->start_header;
    /* Filling the rest of the data structure */
    switch(type) {
    case FPGA_UPGRADE_IMAGE:
        fpga_p->start_addr   = p_spi_prom_info->start_u;
        fpga_p->start_sector = p_spi_prom_info->start_sector_u;
        sprintf(fpga_p->image_str, "upgrade");
        break;

    case FPGA_GOLDEN_IMAGE:
        /* gold image is always supported */
        fpga_p->start_addr   = p_spi_prom_info->start_g;
        fpga_p->start_sector = p_spi_prom_info->start_sector_g;
        sprintf(fpga_p->image_str, "golden ");
        break;
    default:
	cterr('f',0,"Unknown FPGA image type = %d", type);
	return (FAILED); 
    }

    fpga_p->end_addr = fpga_p->start_addr + fpga_p->fpga_fw_size;
    /* 
     * fpga_p->end_sector hold only the number of sectors that needs to be 
     * erased for programming.
     */
    fpga_p->end_sector = fpga_p->start_sector + 
	(fpga_p->fpga_fw_size/SPI_PROM_SECTOR_SIZE);

    return (PASSED);
}

/*-------------------------------------------------------------------
 *
 * Function: prom_sector_program()
 * 
 * This function program the requested sector of the provided image
 * to the same sector on the PROM device.
 *
 * Input: spi_prom  - Pointer to the SPI PROM register file
 *        sect_num  - Sector number to be programmed
 *        src_img   - Pointer to the beginning of the source image
 *                    (Without adjusting for sector boundary)
 *        bit_reverse - TRUE if the bits in the image needs to be reversed
 *                      before programming, FALSE otherwise
 *
 * Output: PASSED if programmed successfully, FAILED otherwise
 *
 *-------------------------------------------------------------------
 */
int prom_sector_program(prom_t *spi_prom, ushort sect_num,
		                uchar *src_img, boolean bit_reverse)
{
    ulong start_addr, end_addr;

    start_addr = sect_num * SPI_PROM_SECTOR_SIZE;
    end_addr   = start_addr + SPI_PROM_SECTOR_SIZE - 1;
    return (prom_image_program(spi_prom, start_addr, end_addr, 
			      &src_img[start_addr], bit_reverse));
}

/*-------------------------------------------------------------------
 *
 * Function: prom_image_program()
 * 
 * This function program the provided image from start address to end
 * address of the SPI PROM.
 *
 * Input: spi_prom    - Pointer to the SPI PROM register file
 *        start_addr  - Starting address of the SPI PROM to program
 *        end_addr    - Ending address of the SPI PROM
 *        src_img     - Pointer to the source image to be programmed
 *        bit_reverse - TRUE if the bits in the image needs to be reversed
 *                      before programming, FALSE otherwise
 *
 * Output: PASSED if programmed successfully, FAILED otherwise
 *
 *-------------------------------------------------------------------
 */
int prom_image_program(prom_t *spi_prom, ulong start_addr, 
		               ulong end_addr, uchar *src_img, boolean bit_reverse)
{
    ulong pgm_size, ix, ctrl_flag, img_idx = 0;
    uchar data_check;

    /* Set flag appropriately depends on whether bits should be reversed */
    if (bit_reverse) {
	ctrl_flag = PROM_DFLT_BAUD | PROM_USE_ADDR | PROM_DATA_WRITE | 
	    PROM_SWAP_DATA_BITS;
    } else {
	ctrl_flag = PROM_DFLT_BAUD | PROM_USE_ADDR | PROM_DATA_WRITE;
    }

    while (start_addr < end_addr) {

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
            /* Check if SPI PROM is ready for write */
            if (!is_spi_prom_rdy(SPI_PROM_MAX_WAIT)) {
                cterr('f', 0, "SPI PROM is not ready for write");
                return (FAILED);
            }

	    /* Rewind the index since we skipped ahead to check data */
	    img_idx -= (pgm_size + 1);

	    /* Now, there are non-blank data that needs to be written.
	     * Ensure that the SPI PROM has write enabled 
	     */ 
	    if (device_write_enable(spi_prom) == FAILED) {
		cterr('f', 0, "%s: Unable to enable SPI device for write", 
		      __FUNCTION__);
		return (FAILED);
	    }

	    spi_prom->opcode_addr = DSWAP4((PROM_PAGE_PROG_OP | start_addr));
	    spi_prom->size = DSWAP4(pgm_size);

	    /* Fill data FIFO 256 bytes max at a time */
	    for (ix = 0; ix <= pgm_size; ix++) {
		    spi_prom->data = DSWAP4(src_img[img_idx]);
		    img_idx++;
	    }

	    /* Start writing to PROM */
	    spi_prom->control = DSWAP4(ctrl_flag);
	    
	    /* Check if operation completed */
	    if (!is_rd_wr_op_done(spi_prom)) {
		cterr('f',0,"%s: write operation not done", __FUNCTION__);
		return (FAILED);
	    }


	}
	/* Go to the next block */
        start_addr = (start_addr + pgm_size + 1);

	/* Print progress indication at every sector boundary */
        if ((NVRAM)->diagflag & D_VERBOSE) {
            if(!(start_addr % SPI_PROM_SECTOR_SIZE)) {
                printf("Programming address %#.8lx.\n", start_addr);
            }
        } else {
	    if (!(start_addr % SPI_PROM_SECTOR_SIZE))
                printf(".");
                fflush(stdout);
        }
    }
    
    /* Check if SPI PROM is ready for write */
    if (!is_spi_prom_rdy(SPI_PROM_MAX_WAIT)) {
        cterr('f', 0, "SPI PROM is not ready for write");
        return (FAILED);
    }
#if 0
    /* Disable Write Operation */
    if (device_write_disable(spi_prom) == FAILED) {
	cterr('f', 0, "%s: Unable to disable write operation on the SPI "
	      "device", __FUNCTION__);
	return (FAILED);
    }
#endif
    return (PASSED);
}


/*-------------------------------------------------------------------
 *
 * Function: program_image_upgrade_header
 * 
 * programe image header
 *
 * Input: notused-- not used.
 *
 * Output: PASSED if the update process completed successfully
 *         FAILED otherwise
 *
 *-------------------------------------------------------------------
 */
int program_image_upgrade_header (int notused)
{
    int ans;
    
    spi_prom = init_spi_prom_ds();

    /* Erase the header sector */
    if (erase_header_spi_prom_image(FPGA_UPGRADE_IMAGE, TRUE)==FAILED)
        return (FAILED);

    printf("\nErase done. (Press 'y/Y' to program header or any other key to Quit)\n");

    ans = getchar();
    
    if ((ans == 'y') || (ans == 'Y')) {
        return (program_spi_prom_image_header(notused, TRUE));
    }
    return (PASSED);    

}

/*-------------------------------------------------------------------
 *
 * Function: program_image_update_type
 * 
 * set flag. flag is used to program image header.
 *
 * Input: dummy-- not used.
 *
 * Output: PASSED if the update process completed successfully
 *         FAILED otherwise
 *
 *-------------------------------------------------------------------
 */
int program_image_update_type (int dummy)
{

    update_flag = getdec_answer("upgrade: Always(0); if newer(1); if not same (2); "
                                "don't upgrade(3)", 0, 0, 3);
    
    return (PASSED);
}

/*-------------------------------------------------------------------
 *
 * Function: set_date_revision
 * 
 * set date and revision, which is used to program image header.
 *
 * Input: dummy-- not used.
 *
 * Output: PASSED if the update process completed successfully
 *         FAILED otherwise
 *
 *-------------------------------------------------------------------
 */
int set_date_revision (int dummy)
{
    int mjr, mnr, dbg, brd;
    int hr, dt, mnth , yr;

    mjr = mnr = dbg = brd = hr = dt = mnth = yr= 0;
    
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


/*-------------------------------------------------------------------
 *
 * Function: set_fpga_reconf_fsm
 * 
 * Set FPGA reconfiguration register FSM reset.
 *
 * Input: none
 *
 * Output: PASSED if the update process completed successfully
 *         FAILED otherwise
 *
 *-------------------------------------------------------------------
 */
int set_fpga_reconf_fsm (void)
{

    uint32_t reg_offset, reg_val = 0;

    reg_offset = (uint32_t)FPGA_RECONFIG_CTRL_REG;
    reg_val = (uint32_t)FPGA_UPGRADE_HEADER_NOTE_READ; 
    if (fpga_write_reg(reg_offset, reg_val) != PASSED) {
        printf("Failed to write FPGA register 0x%04X.\n", reg_offset);
        return (FAILED);
    }
    
    printf("\nFPGA Reconfiguration FSM Reset...\n");
    msleep(FPGA_RECONFI_SLEEP);
    
    reg_val = (uint32_t)FPGA_UPGRADE_RECONF_FSM_RST; 
    if (fpga_write_reg(reg_offset, reg_val) != PASSED) {
        printf("Failed to write FPGA register 0x%04X.\n", reg_offset);
        return (FAILED);
    }

    
    return (PASSED);
}

/*-------------------------------------------------------------------
 * 
 * Function: fpga_spi_read_id
 * 
 * Enable FPGA SPI Flash ID.
 *
 * Input: prom_id -- The pointer to store SPI Flash ID data.
 *
 * Output: PASSED if the update process completed successfully
 *         FAILED otherwise
 *
 *-------------------------------------------------------------------
 */
int fpga_spi_read_id (uint *prom_id)
{
    volatile uchar data[SPI_MAX_ID_LEN];
    ulong ctrl_flag, count;
    int temp_data;
    ushort ix;
    prom_t *spi_prom;	

    spi_prom = init_spi_prom_ds();

    ctrl_flag = PROM_DFLT_BAUD;	
    count = SPI_MAX_ID_LEN;
	
    /* Read the SPI PROM Read ID Register (RDID) */
    spi_prom->size        = DSWAP4(count);
    spi_prom->opcode_addr = DSWAP4(PROM_RDID_OP);
    spi_prom->control     = DSWAP4(ctrl_flag); /* Read */

    /* Check if operation completed */
    if (!is_rd_wr_op_done(spi_prom)) {
        cterr('f',0,"%s: read operation not done", __FUNCTION__);
        return (FAILED);
    }

    for (ix = 0; ix < count; ix++) {
        temp_data = spi_prom->data;
        data[ix] = (uchar)(DSWAP4(temp_data));
    }

    *prom_id = data[SPI_ID_BYTE_0] << SPI_ID_BIT_16 | 
                       data[SPI_ID_BYTE_1] << SPI_ID_BIT_8 | 
                       data[SPI_ID_BYTE_2] << SPI_ID_BIT_0;
	
    return (PASSED);
}


/*-------------------------------------------------------------------
 *
 * Function: fpga_write_spi_prom_status_reg
 * 
 * This function writes a byte to the SPI PROM status register.
 *
 * Input: wr_byte  - The char value to write to the RDSR
 *
 * Output: PASSED if operation succeeded, FAILED otherwise
 * 
 *-------------------------------------------------------------------
 */
int fpga_write_spi_prom_status_reg (uchar wr_byte)
{
    prom_t *spi_prom;

    spi_prom = init_spi_prom_ds();

    if (write_spi_prom_status_reg(spi_prom, wr_byte) == FAILED) {
	cterr('f', 0, "Unable to write SPI PROM status register (WRSR)");
	return (FAILED);
    }

    return (PASSED);
}


/*-------------------------------------------------------------------
 *
 * Function: fpga_read_spi_prom_status_reg
 * 
 * This function read a byte from the SPI PROM status register.
 *
 * Input: rdsr  - The pointer to store rdsr data
 *
 * Output: PASSED if operation succeeded, FAILED otherwise
 *
 *-------------------------------------------------------------------
 */
int fpga_read_spi_prom_status_reg (uchar *rdsr) {

    if (read_spi_prom_status_reg(rdsr) == FAILED) {
	cterr('f', 0, "Unable to read SPI PROM status register (RDSR)");
	return (FAILED);
    }

    return (PASSED);

}


/*-------------------------------------------------------------------
 *
 * Function: fpga_spi_write_protect_test
 * 
 * This function verify FPGA SPI Flash write protect sector.
 *
 * Input: start_sector  - The sector to verify write protect.
 *
 * Output: PASSED if operation succeeded, FAILED otherwise
 *
 *-------------------------------------------------------------------
 */
int fpga_spi_write_protect_test (uint start_sector)
{

    uchar data_bk[SPI_PROM_SECTOR_SIZE];
    uchar data_rd[SPI_PROM_SECTOR_SIZE];
    uint ix, jx;
    unsigned int start_addr, end_addr;
    int temp_data;
    ulong ctrl_flag, count;
    prom_t *spi_prom;
    int sector;

    
	
    /* 1. Read data from FPGA SPI Flash specify sector and store to the data_bk buffer. */
    sector = start_sector;
    prpass(testpass, "start to test write protect for sector %d ", sector);
    start_addr = sector * SPI_PROM_SECTOR_SIZE;
    end_addr = start_addr + SPI_PROM_SECTOR_SIZE;
    jx = 0;
    ctrl_flag = PROM_DFLT_BAUD | PROM_USE_ADDR;
    while (start_addr < end_addr) {
        /* Handle case where the compare size is not multiple of 256 bytes */
        if ((end_addr - start_addr) < PROM_RD_MAX_BYTE) {
            count = end_addr - start_addr;
        } else {
            count = PROM_RD_256_BYTE;
        }

        spi_prom = init_spi_prom_ds();

        /* Read the SPI PROM Read Status Register (RDSR) */
        spi_prom->size        = DSWAP4(count);
        spi_prom->opcode_addr = DSWAP4(PROM_READ_OP | start_addr);
        spi_prom->control     = DSWAP4(ctrl_flag); /* Read */

        /* Check if operation completed */
        if (!is_rd_wr_op_done(spi_prom)) {
	    cterr('f',0,"%s: read operation not done", __FUNCTION__);
	    return (FAILED);
        }

        /* Back up original image data in 256-byte block */
        for (ix = 0; ix <= count; ix++, jx++) {
            temp_data = spi_prom->data;
            data_bk[jx] = (uchar)(DSWAP4(temp_data));

        }
        /* Go to the next block */
        start_addr = start_addr + count + 1;
    }


    /* 2. Erase test sector. */
    /* Unprotect the SPI PROM to allow write/erase */
    if (write_spi_prom_status_reg(spi_prom, PROM_UNPROTECT_ALL) == FAILED) {
	cterr('f', 0, "Failure to unprotect the SPI PROM to allow write");
	return (FAILED);
    }
	
    /* Check if SPI PROM is ready for write */
    if (!is_spi_prom_rdy(SPI_PROM_MAX_WAIT)) {
        cterr('f', 0, "SPI PROM is not ready for write");
        return (FAILED);
    }

    /* Erase sector */
    if (prom_sector_erase(spi_prom, sector) == FAILED) {
	cterr('f', 0, "Erase operation failed on sector %d", sector);
	return (FAILED);
    }
    
    /* Poll for erase completion - look for WIP bit clear */
    if (!is_spi_prom_rdy(SPI_PROM_ERASE_WAIT)) {
	cterr('f', 0, "Erase not completed on sector %d", sector);
	return (FAILED);
    }

    /* 3. Read data from FPGA SPI Flash specify sector and store to the data_rd buffer. */
    start_addr = sector * SPI_PROM_SECTOR_SIZE;
    end_addr = start_addr + SPI_PROM_SECTOR_SIZE;
    jx = 0;
    ctrl_flag = PROM_DFLT_BAUD | PROM_USE_ADDR;
    while (start_addr < end_addr) {
        /* Handle case where the compare size is not multiple of 256 bytes */
        if ((end_addr - start_addr) < PROM_RD_MAX_BYTE) {
            count = end_addr - start_addr;
        } else {
            count = PROM_RD_256_BYTE;
        }

        spi_prom = init_spi_prom_ds();

        /* Read the SPI PROM Read Status Register (RDSR) */
        spi_prom->size        = DSWAP4(count);
        spi_prom->opcode_addr = DSWAP4(PROM_READ_OP | start_addr);
        spi_prom->control     = DSWAP4(ctrl_flag); /* Read */

        /* Check if operation completed */
        if (!is_rd_wr_op_done(spi_prom)) {
	    cterr('f',0,"%s: read operation not done", __FUNCTION__);
	    return (FAILED);
        }

        /* Comparing with the original image data in 256-byte block */
        for (ix = 0; ix <= count; ix++, jx++) {
            temp_data = spi_prom->data;
            data_rd[jx] = (uchar)(DSWAP4(temp_data));
        }
        
        /* Go to the next  block */
        start_addr = start_addr + count + 1;
    }

    /* 4. Compare backup data and read data, if write protect mode, the data should be the same. */

    prpass(testpass, "compare write protect data for sector %d ", sector);

    for (ix = 0; ix < SPI_PROM_SECTOR_SIZE; ix++) {
        if (*(data_bk+ix) != *(data_rd+ix)) {
            cterr('f', 0, "failed on byte %d, backup data = %02x, read back = %02x", ix, *(data_bk+ix), *(data_rd+ix));
            printf("Mismatched, and start to retore backup data...\n");

	     /* Start to restore FPGA data... */
            start_addr = sector * SPI_PROM_SECTOR_SIZE;
            end_addr = start_addr + SPI_PROM_SECTOR_SIZE;
            if (prom_image_program(spi_prom, start_addr, end_addr, data_bk, FALSE) == FAILED) { 
	         cterr('f', 0, "%s: SPI PROM image programming failed.");
	        return (FAILED);
            }

            if (verify_download_image(spi_prom, start_addr, end_addr, data_bk, FALSE) == FAILED) {
    	         cterr('f',0,"Restore FPGA flash data failed.");
                return (FAILED);
            }
			
	     printf("\nRestore FPGA data done ...\n");
            return (FAILED);
			 
        }
    }

    prpass(testpass, "write protect test for sector %d done ", sector);

    return (PASSED);
}

/*-------------------------------------------------
 * $Log: diag_fpga_upgrade.c,v $
 * Revision 1.2  2019/10/17 02:16:21  kehuang2
 * Collapse Tabei-L into main trunk
 *
 * Revision 1.1.2.5  2019/02/12 07:18:11  olin2
 * Update FPGA upgrade description
 *
 * Revision 1.1.2.4  2019/02/12 06:44:57  olin2
 * Update FPGA config SPI PROM address
 *
 * Revision 1.1.2.3  2018/12/27 01:28:38  olin2
 * initial commit for FPGA Upgrade
 *
 * Revision 1.1.2.2  2018/12/05 06:50:34  olin2
 * initial commit for Aikido
 *
 * Revision 1.1.2.1  2018/10/02 01:49:59  harrchan
 * Initial commit for Tabei-L P1A bring up.
 *
 * $Endlog$
 *-------------------------------------------------
 */
