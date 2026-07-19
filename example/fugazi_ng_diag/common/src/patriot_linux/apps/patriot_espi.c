/* $Id: patriot_espi.c,v 1.1 2014/03/25 02:12:34 huanngo Exp $
 * $Source: 
 *******************************************************************************
 * File Name: patriot_espi.c
 *
 * Description: ESPI drivers for Patriot
 *
 *
 * Author: Sofian Teja
 * Copyright (c)2011 - 2014 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *******************************************************************************
 */

/* Includes. */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include "defs.h"
#include "patriot_main.h"
#include "common_utils.h"
#include "ds3170.h"     /* structure definition for the chip */
#include "p1021_espi.h"

extern uchar err_msg[];
/*===================================================================*
 *                             Globals                               *
 *===================================================================*/


/**********************************************************************
 *
 * Function: ds3170_init
 *
 * This function init espi from CPU to DS3170
 *
 * Input : NONE
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
int ds3170_init_espi (void)
{
    int *dummy;
    int espi_mode;

    /* 1. Reverse data, msb is sent first
     * 2. Prescale modulus select = 10
     * 3. 8 bits per character. Set LEN0 = 7.
     * 4. Polarity, Asserted Low, Negated High
     * 5. System clock is the input to the eSPI BRG
     * System clock = ccb_clk/2 = 333.33/2 = 166.665Mhz
     * eSPI BRG = 166.665 Mhz/16 = 10.416 Mhz
     * SPI_CLK = (eSPI_BRG)/(2 * (PM + 1)) = 500Khz  (DS3170 MAX is 10 Mhz)
     * 500Khz  = 10.416 Mhz / (2 * (PM + 1)) --> PM = 9.416
     * 470Khz  = PM (10) to be conservative
     * DS3170 SPI_CPOL = 0 , SPI_CPHA = 0
     */
    espi_mode = ESPI_MODEX_REV1 | ESPI_MODEX_PM(10) | ESPI_MODEX_LEN (7) |
            ESPI_MODEX_POL1 | ESPI_MODEX_DIV16;
    espi_mode &= ~ESPI_MODEX_CP1;

    if (espi_init(ESPI_CS1, espi_mode) == FAILED) {
        sprintf(err_msg, "\n%s, [#%d]:Fail to init espi.\n",
        		__FUNCTION__, __LINE__);
        print_err(FALSE, err_msg, LVL_1);
        return (FAILED);
    }
    /* Run a dummy command the 1st time
       On the Beagle, the first time it is MOSI 05, MISO FF
       The 2nd time it is MOSI 05 FF, MISO FF 4F */
    if (ds3170_read_if_status(ESPI_CS1)) {
    	sprintf(err_msg, "\n%s, [#%d]:Fail to read\n", __FUNCTION__, __LINE__);
    	print_err(FALSE, err_msg, LVL_1);
    	return (FAILED);
    }

    return (PASSED);
}


/*******************************************************************************
 *
 * Name         : ds3170_read
 * Description  : Mask the address offset for high address and low address
 * also for bit burst before read the ds3170 chips from the standard SPI interface
 * Input          : buffer         - pointer to the buffer to hold the contents
 *                     addr_offset - register offset
 * Output        : PASSED / FAILED
 *
 *******************************************************************************
 */
int ds3170_read (uchar *buffer, uint addr_offset)
{
    uchar temp;
    uchar low_offset = 0x00;
    uchar high_offset = 0x00;

    /* 1. shift left the address first, the last bit for burst */
    addr_offset = addr_offset << 1;

    /* 2. mask bit and shift high offset to the right */
    high_offset = (uchar)((addr_offset & MASK_BITS32) >> 8);

    /* 3. mask low offset */
    low_offset = (uchar)(addr_offset & MASK_BITS10);

    /* 4. pass high low offset */
    if (ds3170_read_if(high_offset, low_offset, &temp, ESPI_CS1))
        return (FAILED);

    buffer[0] = temp;

    return (PASSED);
}


/*******************************************************************************
 *
 * Name        : ds3170_read_if
 * Description : Read the ds3170 chips from the standard SPI interface
 * Input         : high_offset - high address of the buffer data
 *                    low_offset  - low address of the buffer data
 *                    data          -  buffer pointer data
 *                    cs             -  chip select
 * Output       : - buffer data byte
 *                    - Return (PASSED / FAILED)
 *
 *******************************************************************************
 */
int ds3170_read_if (uchar high_offset, uchar low_offset, uchar *data, int cs)
{
    uchar spi_cmd[DS3170_RD_CMD_LEN];
    uchar spi_reply[DS3170_RPLY_LEN];

    /* The first byte is the read command 0x80 + addr offset high byte */
    spi_cmd[0] = high_offset | DS3170_READ;

    /* The second byte is low byte address shift to left 1 bit for burst mode 0 */
    spi_cmd[1] = low_offset;

    spi_reply[0] = 0x0;
    spi_reply[1] = 0x0;
    spi_reply[2] = 0x0;

    if (access_spi(DS3170_RD_CMD_LEN, spi_cmd, spi_reply, cs,
		   DS3170_RPLY_LEN, TRUE, TRUE) == FAILED) {
        printf("\nFailed to read DS3170\n");
        *data = INVALID_DATA;
        return (FAILED);
    }

    /* Ignore the first two bytes */
    *data = spi_reply[2];

    return (PASSED);
}

/* Working around for dummy function to check the first time 0x00 is 0x4F not 0xFF */
int ds3170_read_if_status (int cs)
{
    uchar spi_cmd[DS3170_RD_CMD_LEN];
    uchar spi_reply[DS3170_RPLY_LEN];

    /* The first byte is the read command 0x80 + addr offset high byte */
    spi_cmd[0] = 0x00 | DS3170_READ;

    /* The second byte is low byte address shift to left 1 bit for burst mode 0 */
    spi_cmd[1] = 0x00;

    spi_reply[0] = 0x0;
    spi_reply[1] = 0x0;
    spi_reply[2] = 0x0;

    if (access_spi(DS3170_RD_CMD_LEN, spi_cmd, spi_reply, cs,
		   DS3170_RPLY_LEN, TRUE, TRUE) == FAILED) {
        sprintf(err_msg, "\n%s, [#%d]:Failed to read DS3170\n", __FUNCTION__, __LINE__);
        print_err(FALSE, err_msg, LVL_2);
        return (FAILED);
    }

    return (PASSED);
}


/*******************************************************************************
 *
 * Name         : ds3170_write
 * Description  : Mask the address offset for high address and low address
 * also for bit burst before write the ds3170 chips to the standard SPI interface
 * Input          : buffer         - pointer to the buffer to hold the contents
 *                     addr_offset - register offset
 * Output        : PASSED / FAILED
 *
 *******************************************************************************
 */
int ds3170_write (uchar buffer, uint addr_offset)
{
    uchar low_offset = 0x00;
    uchar high_offset = 0x00;

    /* 1. shift left the address first, the last bit for burst */
    addr_offset = addr_offset << 1;

    /* 2. mask bit and shift high offset to the right */
    high_offset = (uchar)((addr_offset & MASK_BITS32) >> 8);

    /* 3. mask low offset */
    low_offset = (uchar)(addr_offset & MASK_BITS10);

    /* 4. pass high low offset */

    if (ds3170_write_if(high_offset, low_offset, buffer, ESPI_CS1))
        return (FAILED);

    return (PASSED);
}


/*******************************************************************************
 *
 * Name          : ds3170_write_if
 * Description  : Write the ds3170 chips to the standard SPI interface
 * Input          : high_offset - high address of the buffer data
 *                     low_offset  - low address of the buffer data
 *                     data          -  buffer pointer data
 *                     cs             -  chip select
 * Output        : - buffer data byte
 *                    - Return (PASSED / FAILED)
 *
 *******************************************************************************
 */
int ds3170_write_if (uchar high_addr, uchar low_addr, uchar data, int cs)
{
    uchar spi_cmd[DS3170_WR_CMD_LEN];
    uchar spi_reply[DS3170_RPLY_LEN];
    uchar dummy = 0;
    int ix;

    /* The first byte is the write command 0x00 + addr offset high byte */
    spi_cmd[0] = high_addr | DS3170_WRITE;

    /* The second byte is low byte address shift to left 1 bit for burst mode 0 */
    spi_cmd[1] = low_addr;

    /* Third byte is data in */
    spi_cmd[2] = data;

    spi_reply[0] = 0xFF;
    spi_reply[1] = 0xFF;
    spi_reply[2] = 0xFF;

    if (access_spi(DS3170_WR_CMD_LEN, spi_cmd, spi_reply, cs,
		   DS3170_RPLY_LEN, FALSE, TRUE) == FAILED) {
        printf("\nFailed to write DS3170 chips\n");
        return (FAILED);
    }

    return (PASSED);
}


/*******************************************************************************
 *
 * Function      : access_spi
 * Description  : Send a command to a serial device via the SPI interface.
 * Input          : tx_data    - pointer to data buffer to be sent on SPI
 *                     rx_data    - pointer to data buffer to be received on SPI
 *                     byte_count - Number of bytes.
 *                     cs              - chip select
 * Output        : PASSED/FAILED
 *
 *******************************************************************************
 */
int access_spi (int tx_bytes, uchar *tx_data, uchar *rx_data, int cs,
		int rx_bytes, int rd_flag, int ex_flag)
{
    int status;

    /* Activate the eSPI interface */
    espi_activate(cs, tx_bytes, rx_bytes, rd_flag);

    status = espi_xfr(tx_bytes, tx_data, rx_data, rx_bytes, rd_flag, ex_flag);

    /* Disable eSPI interface */
    espi_deactivate(cs);

    return (status);
}


/*******************************************************************************
 *
 * Name        : spi_prom_read_id
 * Description : Read the SPI PROM chips status via
 *               standard SPI interface
 * Input       : cs       - Chip select
 *               *mfg_id  - Manufacture ID
 *               *dev_id0 - Device ID 0
 *               *dev_id1 - Device ID 1
 * Output      : INVALID_DATA / PASSED
 *
 *******************************************************************************
 */
uchar spi_prom_read_id(int cs, uchar *mfg_id, uchar *dev_id0, uchar *dev_id1,
		       uchar *ex_dev_id0, uchar *ex_dev_id1)
{
    volatile uchar spi_cmd[BLOCK_SIZE4];
    volatile uchar spi_reply[BLOCK_SIZE8];

    /* The first byte is the read command */
    spi_cmd[0] = SPI_PROM_READ_ID;
    /* The second byte is the address */
    spi_cmd[1] = 0x00;
    spi_cmd[2] = 0x00;
    spi_cmd[3] = 0x00;

    spi_reply[0] = 0x0;
    spi_reply[1] = 0x0;
    spi_reply[2] = 0x0;
    spi_reply[3] = 0x0;
    spi_reply[4] = 0x0;
    spi_reply[5] = 0x0;
    spi_reply[6] = 0x0;
    spi_reply[7] = 0x0;    

    if (access_spi(1, (uchar *)&spi_cmd[0], (uchar *)&spi_reply[0], cs,
		   BLOCK_SIZE6, TRUE, TRUE)) {
        printf("\nFailed to read ID\n");
        return (INVALID_DATA);
    }

#ifdef DEBUG    
    printf("\nspi_reply[0] = 0x%02x", spi_reply[0]);
    printf("\nspi_reply[1] = 0x%02x", spi_reply[1]);
    printf("\nspi_reply[2] = 0x%02x", spi_reply[2]);
    printf("\nspi_reply[3] = 0x%02x", spi_reply[3]);
    printf("\nspi_reply[4] = 0x%02x", spi_reply[4]);
    printf("\nspi_reply[5] = 0x%02x", spi_reply[5]);
    printf("\nspi_reply[6] = 0x%02x", spi_reply[6]);
    printf("\nspi_reply[7] = 0x%02x", spi_reply[7]);    
#endif    
    /* Ignore the first byte */
    *mfg_id = spi_reply[0];
    *dev_id0 = spi_reply[1];
    *dev_id1 = spi_reply[2];
    *ex_dev_id0 = spi_reply[3];
    *ex_dev_id1 = spi_reply[4];    
    return PASSED;
}


/*******************************************************************************
 *
 * Name        : spi_prom_read_if_status
 * Description : Read the SPI PROM chips status via
 *               standard SPI interface
 * Input       : cs - chip select
 * Output      : Contents of RDSR register
 *
 *******************************************************************************
 */
uchar spi_prom_read_if_status(int cs)
{
    uchar spi_cmd[2], data;
    uchar spi_reply[2];

    /* The first byte is the read command */
    spi_cmd[0] = SPI_PROM_RDSR;
    /* The second byte is the address */
    spi_cmd[1] = 0x00;

    spi_reply[0] = 0x0;
    spi_reply[1] = 0x0;

    if (access_spi(1, spi_cmd, spi_reply, cs, 2, TRUE, TRUE)) {
        printf("\nFailed to read status\n");
        return (INVALID_DATA);
    }

    /* Ignore the first byte */
#ifdef DEBUG
    printf("\nspi_reply[0] = 0x%02x", spi_reply[0]);
    printf("\nspi_reply[1] = 0x%02x", spi_reply[1]);
#endif    
    data = spi_reply[1];
    return (data);
}



/*******************************************************************************
 *
 * Name        : spi_prom_erase_if
 * Description : Erase the SPI PROM chip from the
 *               standard SPI interface
 * Input       : addr_offset - offset of the buffer data
 * Output      : buffer data byte
 *
 *******************************************************************************
 */
int spi_prom_erase_if (uint addr_offset, int block, int cs)
{
    uchar spi_cmd[SPI_PROM_ERASE_CMD_LEN];
    uchar spi_reply[SPI_PROM_ERASE_CMD_LEN];
    uchar dummy = 0;
    int ix;
    volatile uchar status = 0;

#ifdef DEBUG    
    printf("\naddr_offset = 0x%08x", addr_offset);fflush(0);
    printf("\nblock = %d", block);fflush(0);
    printf("\ncs = %d", cs);fflush(0);
#endif

    status = spi_prom_read_if_status(cs);
    /* Check for WIP bit before erasing */
    if ((status & SPI_PROM_WIP) != 0x0) {	
	for (ix = 0; ix < MAX_SPI_SPIN; ix++) {
	    status = spi_prom_read_if_status(cs);
	    if ((status & SPI_PROM_WIP) != 0x0) {
		msleep(1);
	    } else {
		break;
	    }
	}
	if (ix == MAX_SPI_SPIN) {
	    sprintf(err_msg, "\n%s, [#%d]:Time out waiting for WIP before erase, status = 0x%02x", __FUNCTION__, __LINE__,
		   status);
	    print_err(FALSE, err_msg, LVL_1);
	    fflush(0);
	    return (FAILED);
	}
    }

    
    /* Write the WREN first */
    spi_cmd[0] = SPI_PROM_WREN;
    if (access_spi(1, spi_cmd, &dummy, cs, 1, FALSE, TRUE) == FAILED) {
        sprintf(err_msg, "\n%s, [#%d]:Failed to write WREN to SPI PROM\n", __FUNCTION__, __LINE__);
        print_err(FALSE, err_msg, LVL_1);
        return (FAILED);
    }

    for (ix = 0; ix < 20; ix++) {
	status = spi_prom_read_if_status(cs);
	if (status & SPI_PROM_WEL) {
	    break;
	}
	msleep(1);
    }

#ifdef DEBUG    
    printf("\nstatus = 0x%02x\n", status);
#endif
    
    if (!(status & SPI_PROM_WEL)) {
	sprintf(err_msg, "\n%s, [#%d]:Time out waiting for WEL signal", __FUNCTION__, __LINE__);
	print_err(FALSE, err_msg, LVL_1);
	return (FAILED);
    }
  
    /* The first byte is the erase command */
    switch (block) {
    case ERASE_4K_BLOCK:
	spi_cmd[0] = SPI_PROM_ERASE_4K;
	break;
    case ERASE_8K_BLOCK:
	spi_cmd[0] = SPI_PROM_ERASE_8K;
	break;
    case ERASE_64K_BLOCK:
	spi_cmd[0] = SPI_PROM_ERASE_64K;
	break;
    default:
	break;
    }	

    /* The next bytes are for the address
     * Since the Block Erase command erases a region of bytes, the lower
     * order address bits do not need to be decoded by the device.
     * Therefore, for a 4K-byte erase, address bits A11-A0 will be
     * ignored by the device and their values can be either a logical
     * "1" or "0". For a 32K-byte erase, address bits A14-A0 will be
     * ignored, and for a 64K-byte erase, address bits A15-A0 will be
     * ignored by the device.
     */
    
    spi_cmd[1] = (uchar)((addr_offset & MASK_BITS54) >> 16);
    spi_cmd[2] = (uchar)((addr_offset & MASK_BITS32) >> 8);
    spi_cmd[3] = (uchar)(addr_offset & MASK_BITS10);

    spi_reply[0] = 0xFF;
    spi_reply[1] = 0xFF;
    spi_reply[2] = 0xFF;
    spi_reply[3] = 0xFF;

    if (access_spi(SPI_PROM_ERASE_CMD_LEN, &spi_cmd[0], &spi_reply[0], cs,
		   SPI_PROM_ERASE_CMD_LEN, FALSE, TRUE) == FAILED) {
        sprintf(err_msg, "\n%s, [#%d]:Failed to write SPI PROM\n", __FUNCTION__, __LINE__);
        print_err(FALSE, err_msg, LVL_1);
        return (FAILED);
    }

    status = spi_prom_read_if_status(cs);
    /* Give it some time to erase the sector */
    if ((status & SPI_PROM_WIP) != 0x0) {	
	for (ix = 0; ix < MAX_SPI_SPIN; ix++) {
	    status = spi_prom_read_if_status(cs);
	    if ((status & SPI_PROM_WIP) != 0x0) {
		msleep(10);
	    } else {
		break;
	    }
	}
	if (ix == MAX_SPI_SPIN) {
	    sprintf(err_msg, "\n%s, [#%d]:Time out waiting for WIP after erase, status = 0x%02x", __FUNCTION__, __LINE__,
		   status);
	    print_err(FALSE, err_msg, LVL_1);
	    fflush(0);
	    return (FAILED);
	}
    }

    return (PASSED);
}

/**********************************************************************
 *
 * Function: patriot_spi_prom_erase
 *
 * This function erase data on SPI PROM
 *
 * Input : addr_offset - offset of the buffer data
 *         block       - first byte for erase command
 *         cs          - chip select
 *
 * Output: PASS/FAIL
 *
 **********************************************************************
 */
int patriot_spi_prom_erase(uint addr_offset, uint block, int cs,
			   uchar *buf)
{
    uint i, sector_start;
    uchar temp;

    if (spi_prom_erase_if (addr_offset, block, cs)) {
	return (FAILED);
    }
#if DEBUG
    sector_start = addr_offset & ~(SECTOR_SIZE - 1);
    /* verify flash sector with data all 0xff's */
    if (buf) {
	if (patriot_spi_prom_read(buf, sector_start, SECTOR_SIZE, cs)) {
	    return FAILED;
	}
	for (i = 0; i < SECTOR_SIZE; i++) {
	    if (buf[i] != 0xff) {
		printf("SPI prom sector erase failed at addr %#.8x with "
		       "data %#.2x\n", sector_start + i, buf[i]);
		return (FAILED);
	    }
	}
    }
#endif    
    return(PASSED);
}

/**********************************************************************
 *
 * Function: patriot_spi_prom_erase_util
 *
 * This function erase data from SPI interface for SPI PROM from user info
 *
 * Input : None
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
int patriot_spi_prom_erase_util(void)
{

    uint spi_num, addr_offset, len, cs;
    uchar *buf;
    
    spi_num = gethex_answer("Select SPI PROM:0-Golden, 1-Upgrate, 2-FPGA",
				    0, 0, 2);

    addr_offset = gethex_answer("Enter spi prom sector address in hex",
			 LAST_SECTOR_ADDR, 0, SPI_PROM_SIZE - 1);

    if (spi_num == 0) {
	cs = ESPI_CS0;
    } else if (spi_num == 1) {
	cs = ESPI_CS2;
    } else {
	cs = ESPI_CS3;
    }

    buf = malloc(SECTOR_SIZE);
    if (!buf) {
	printf("Malloc read back buf failed\n");
    	return FAILED;
    }
    
    memset((uchar *)buf, 0, SECTOR_SIZE);
    
    if (patriot_spi_prom_erase(addr_offset, ERASE_64K_BLOCK, cs, buf)) {
    	free(buf);
	return FAILED;
    }

    printf("Erase spi prom sector at %#.8x completed\n", addr_offset);

    free(buf);
    return (PASSED);
}


/*******************************************************************************
 *
 * Name        : spi_prom_otpr_if
 * Description : Read the OTP region on the SPI PROM chips from the
 *               standard SPI interface
 * Input       : addr_offset - offset of the buffer data
 *               *data       - buffer data byte
 *               cs          - chip select
 *
 * Output      : buffer data byte
 *               PASSED / FAILED
 *
 *******************************************************************************
 */
int spi_prom_otpr_if (uint addr_offset, uchar *data, int cs)
{
    uchar spi_cmd[SPI_PROM_FAST_READ_CMD_LEN];
    uchar spi_reply[BLOCK_SIZE8];
#ifdef DEBUG    
    printf("\n%s: addr_offset = 0x%04x, data = 0x%02x\n", __FUNCTION__,
	   addr_offset, data);
#endif    
    /* The first byte is the read command */
    spi_cmd[0] = SPI_PROM_OTPR;

    /* The second byte is the address */
    spi_cmd[1] = (uchar)((addr_offset & MASK_BITS54) >> 16);
    spi_cmd[2] = (uchar)((addr_offset & MASK_BITS32) >> 8);
    spi_cmd[3] = (uchar)(addr_offset & MASK_BITS10);
    spi_cmd[4] = 0; /* dummy */

    spi_reply[0] = 0x0;
    spi_reply[1] = 0x0;
    spi_reply[2] = 0x0;
    spi_reply[3] = 0x0;
    spi_reply[4] = 0x0;
    spi_reply[5] = 0x0;
    spi_reply[6] = 0x0;
    spi_reply[7] = 0x0;    

    if (access_spi(SPI_PROM_FAST_READ_CMD_LEN, &spi_cmd[0], &spi_reply[0],
		   cs, BLOCK_SIZE6, TRUE, TRUE) == FAILED) {
        printf("\nFailed to read SPI PROM\n");
        *data = INVALID_DATA;
        return (FAILED);
    }

    /* Ignore the first two bytes */
    *data = spi_reply[3];

    return (PASSED);
}


/*******************************************************************************
 *
 * Name        : spi_prom_read_if
 * Description : Read the SPI PROM chips from the
 *               standard SPI interface
 * Input       : addr_offset - offset of the buffer data
 *               *data       - buffer data byte
 *               cs          - chip select
 *
 * Output      : buffer data byte
 *               PASSED / FAILED
 *
 *******************************************************************************
 */
int spi_prom_read_if (uint addr_offset, uchar *data, int cs)
{
    uchar spi_cmd[SPI_PROM_READ_CMD_LEN];
    uchar spi_reply[BLOCK_SIZE8];
    int ix;
    volatile uchar status;

    status = spi_prom_read_if_status(cs);
    if ((status & SPI_PROM_WIP) != 0x0) {	
	for (ix = 0; ix < MAX_SPI_SPIN; ix++) {
	    status = spi_prom_read_if_status(cs);
	    if ((status & SPI_PROM_WIP) != 0x0) {
		msleep(1);
	    } else {
		break;
	    }
	}
	if (ix == MAX_SPI_SPIN) {
	    sprintf(err_msg, "\n%s, [#%d]:Time out waiting for WIP before read, status = 0x%02x", __FUNCTION__, __LINE__,
		   status);
	    print_err(FALSE, err_msg, LVL_1);
	    fflush(0);
	    return (FAILED);
	}
    }

    /* The first byte is the read command */
    spi_cmd[0] = SPI_PROM_READ;

    /* The second byte is the address */
    spi_cmd[1] = (uchar)((addr_offset & MASK_BITS54) >> 16);
    spi_cmd[2] = (uchar)((addr_offset & MASK_BITS32) >> 8);
    spi_cmd[3] = (uchar)(addr_offset & MASK_BITS10);


    spi_reply[0] = 0x0;
    spi_reply[1] = 0x0;
    spi_reply[2] = 0x0;
    spi_reply[3] = 0x0;
    spi_reply[4] = 0x0;
    spi_reply[5] = 0x0;
    spi_reply[6] = 0x0;
    spi_reply[7] = 0x0;    

    if (access_spi(SPI_PROM_READ_CMD_LEN, &spi_cmd[0], &spi_reply[0],
		   cs, BLOCK_SIZE5, TRUE, TRUE) == FAILED) {
        sprintf(err_msg, "\n%s, [#%d]:Failed to read SPI PROM\n", __FUNCTION__, __LINE__);
        print_err(FALSE, err_msg, LVL_1);
        *data = INVALID_DATA;
        return (FAILED);
    }

    /* Ignore the first two bytes */
    *data = spi_reply[2];

    return (PASSED);
}


/**********************************************************************
 *
 * Function: patriot_spi_prom_read
 *
 * This function read data from SPI interface for SPI PROM
 *
 * Input : *buffer     - buffer data byte
 *         addr_offset - offset of the buffer data
 *         size        - buffer size
 *         cs          - chip select
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
int patriot_spi_prom_read(uchar *buffer, uint addr_offset, uint size,
			  int cs)
{
    uint i;
    uchar temp;
    
    for (i = 0; i < size; i++) {
	
	if (spi_prom_read_if(addr_offset, &temp, cs)) {
	    return (FAILED);
	}
	buffer[i] = temp;
	addr_offset++;
    }
    
    return(PASSED);
}


/**********************************************************************
 *
 * Function: patriot_spi_prom_read_util
 *
 * This function read data from SPI interface for SPI PROM from user info
 *
 * Input : None
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
int patriot_spi_prom_read_util(void)
{
    int spi_num, addr_offset, len, cs;
    uchar *buf;
    spi_num = gethex_answer("Select SPI PROM:0-Golden, 1-Upgrate, 2-FPGA",
				    0, 0, 2);
    
    addr_offset = gethex_answer("Enter spi prom address in hex",
			 LAST_SECTOR_ADDR, 0, SPI_PROM_SIZE - 1);

    len = gethex_answer("Enter bytes to read in hex", 0, 0, SPI_PROM_SIZE);
    
    if (addr_offset + len > SPI_PROM_SIZE) {
	printf("Cannot read beyond %#x, addr=%#.8x len=%#x\n",
		SPI_PROM_SIZE, addr_offset, len);
	return FAILED;
    }
    if (len == 0) {
    	return PASSED;
    }
    buf = malloc(len);
    if (!buf) {
	printf("Malloc read back buf failed, len=%d\n", len);
	return FAILED;
    }

    memset((uchar *)buf, 0, len);

    if (spi_num == 0) {
	cs = ESPI_CS0;
    } else if (spi_num == 1) {
	cs = ESPI_CS2;
    } else {
	cs = ESPI_CS3;
    }

    if (patriot_spi_prom_read(buf, addr_offset, len, cs)) {
    	free(buf);
	return FAILED;
    }

    dismem(buf, len, addr_offset, sizeof(uchar));
    free(buf);
    return PASSED;    

}


/*******************************************************************************
 *
 * Name        : spi_prom_otpp_if
 * Description : Write to the OTP region on the SPI PROM chip to the
 *               standard SPI interface
 * Input       : addr_offset - offset of the buffer data
 *               *data       - buffer data byte
 *               cs          - chip select
 * Output      : buffer data byte
 *               PASSED / FAILED
 *
 *******************************************************************************
 */
int spi_prom_otpp_if (uint addr_offset, uchar data, int cs)
{
    uchar spi_cmd[SPI_PROM_WRITE_CMD_LEN];
    uchar spi_reply[SPI_PROM_WRITE_CMD_LEN];
    uchar dummy[2] = {0xFF, 0xFF};
    int ix;
    volatile uchar status;
    
#ifdef DEBUG
    printf("\n%s: addr_offset = 0x%04x, data = 0x%02x\n", __FUNCTION__,
	   addr_offset, data);
#endif
    
    /* Write the WREN first */
    spi_cmd[0] = SPI_PROM_WREN;
    spi_cmd[1] = 0xff;
    if (access_spi(1, &spi_cmd[0], &dummy[0], cs, 1, FALSE, TRUE) == FAILED) {
        printf("\nFailed to write WREN to SPI PROM\n");
        return (FAILED);
    }

    status = spi_prom_read_if_status(cs);

    if (!(status & SPI_PROM_WEL)) {
	printf("\nTime out waiting for WEL signal");
	return (FAILED);
    }

    /* Check for WIP bit before erasing */
    if ((status & SPI_PROM_WIP) != 0x0) {	
	for (ix = 0; ix < MAX_SPI_SPIN; ix++) {
	    status = spi_prom_read_if_status(cs);
	    if ((status & SPI_PROM_WIP) != 0x0) {
		usleep(1);
	    } else {
		break;
	    }
	}
	if (ix == MAX_SPI_SPIN) {
	    printf("\nTime out waiting for WIP signal, status = 0x%02x", status);
	    return (FAILED);
	}
    }
    
    /* The first byte is the OTPP command */
    spi_cmd[0] = SPI_PROM_OTPP;

    /* The second byte is the address */
    spi_cmd[1] = (uchar)((addr_offset & MASK_BITS54) >> 16);
    spi_cmd[2] = (uchar)((addr_offset & MASK_BITS32) >> 8);
    spi_cmd[3] = (uchar)(addr_offset & MASK_BITS10);

    /* Fifth byte is data in */
    spi_cmd[4] = data;

    spi_reply[0] = 0xFF;
    spi_reply[1] = 0xFF;
    spi_reply[2] = 0xFF;
    spi_reply[3] = 0xFF;
    spi_reply[4] = 0xFF;

    if (access_spi(SPI_PROM_WRITE_CMD_LEN, &spi_cmd[0], &spi_reply[0], cs,
		   SPI_PROM_WRITE_CMD_LEN, FALSE, TRUE) == FAILED) {
        printf("\nFailed to write SPI PROM\n");
        return (FAILED);
    }

    return (PASSED);
}



/*******************************************************************************
 *
 * Name        : spi_prom_write_if
 * Description : Write the SPI PROM chip to the
 *               standard SPI interface
 * Input       : addr_offset - offset of the buffer data
 *               *data       - buffer data byte
 *               cs          - chip select
 * Output      : buffer data byte
 *               PASSED / FAILED
 *
 *******************************************************************************
 */
int spi_prom_write_if (uint addr_offset, uchar data, int cs)
{
    uchar spi_cmd[SPI_PROM_WRITE_CMD_LEN];
    uchar spi_reply[SPI_PROM_WRITE_CMD_LEN];
    uchar dummy[2] = {0xFF, 0xFF};
    int ix;
    volatile uchar status;

    status = spi_prom_read_if_status(cs);
    /* Check for WIP bit before writing */
    if ((status & SPI_PROM_WIP) != 0x0) {	
	for (ix = 0; ix < MAX_SPI_SPIN; ix++) {
	    status = spi_prom_read_if_status(cs);
	    if ((status & SPI_PROM_WIP) != 0x0) {
		msleep(1);
	    } else {
		break;
	    }
	}
	if (ix == MAX_SPI_SPIN) {
	    sprintf(err_msg, "\n%s, [#%d]:Time out waiting for WIP before write, status = 0x%02x", __FUNCTION__, __LINE__,
		   status);
	    print_err(FALSE, err_msg, LVL_1);
	    fflush(0);
	    return (FAILED);
	}
    }
    
    /* Write the WREN first */
    spi_cmd[0] = SPI_PROM_WREN;
    spi_cmd[1] = 0xff;
    if (access_spi(1, &spi_cmd[0], &dummy[0], cs, 1, FALSE, TRUE) == FAILED) {
        sprintf(err_msg, "\n%s, [#%d]:Failed to write WREN to SPI PROM\n", __FUNCTION__, __LINE__);
        print_err(FALSE, err_msg, LVL_1);
        return (FAILED);
    }

    for (ix = 0; ix < 20; ix++) {
	status = spi_prom_read_if_status(cs);
	if (status & SPI_PROM_WEL) {
	    break;
	}
	msleep(1);
    }
	
    if (!(status & SPI_PROM_WEL)) {
    sprintf(err_msg, "\n%s, [#%d]:Time out waiting for WEL signal", __FUNCTION__, __LINE__);
    print_err(FALSE, err_msg, LVL_1);
	return (FAILED);
    }
    
    /* The first byte is the write command */
    spi_cmd[0] = SPI_PROM_WRITE;

    /* The second byte is the address */
    spi_cmd[1] = (uchar)((addr_offset & MASK_BITS54) >> 16);
    spi_cmd[2] = (uchar)((addr_offset & MASK_BITS32) >> 8);
    spi_cmd[3] = (uchar)(addr_offset & MASK_BITS10);

    /* Fifth byte is data in */
    spi_cmd[4] = data;

    spi_reply[0] = 0xFF;
    spi_reply[1] = 0xFF;
    spi_reply[2] = 0xFF;
    spi_reply[3] = 0xFF;
    spi_reply[4] = 0xFF;

    if (access_spi(SPI_PROM_WRITE_CMD_LEN, &spi_cmd[0], &spi_reply[0], cs,
		   SPI_PROM_WRITE_CMD_LEN, TRUE, FALSE) == FAILED) {
        sprintf(err_msg, "\n%s, [#%d]:Failed to write SPI PROM", __FUNCTION__, __LINE__);
        print_err(FALSE, err_msg, LVL_1);
        return (FAILED);
    }

    return (PASSED);
}


/**********************************************************************
 *
 * Function: patriot_spi_prom_write
 *
 * This function write data to SPI interface for SPI PROM
 *
 * Input : *src        - buffer data byte
 *         addr_offset - offset of the buffer data
 *         size        - buffer size
 *         cs          - chip select
 *         *rd_buf     - for verify written data
 *
 * Output: PASS/FAIL
 *
 **********************************************************************
 */
int patriot_spi_prom_write(uchar *src, uint addr_offset, uint size,
			   int cs, uchar *rd_buf)
{
    uint i, tmp_offset;
    uchar temp, *buf;

    buf = malloc(SECTOR_SIZE);
    if (!buf) {
	printf("Malloc read back buf failed\n");
    	return FAILED;
    }

    /* Need to erase the 64K block contains the page first */
    if (patriot_spi_prom_erase(addr_offset, ERASE_64K_BLOCK, cs, buf)) {
	free(buf);
	return (FAILED);
    }

    tmp_offset = addr_offset;
    for (i = 0; i < size; i++) {
	temp = src[i];
	if (spi_prom_write_if(tmp_offset, temp, cs)) {
	    free(buf);
	    return (FAILED);
	}
	tmp_offset++;
    }

    /* verify written data */
    if (rd_buf) {
	if (patriot_spi_prom_read(rd_buf, addr_offset, size, cs)) {
	    free(buf);
	    return (FAILED);
	}
	for (i = 0; i < size; i++) {
	    if (src[i] != rd_buf[i]) {
		printf("SPI prom W/R/V failed at addr offset %#.8x:\n"
			"wdata %#.2x, rdata %#.2x\n",
			addr_offset + i, src[i], rd_buf[i]);
		free(buf);
		return (FAILED);
	    }
	}
    }

    free(buf);

    return(PASSED);
}

/**********************************************************************
 *
 * Function: patriot_spi_prom_write_util
 *
 * This function write data to SPI interface for SPI PROM from user info
 *
 * Input : None
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
int patriot_spi_prom_write_util(void)
{
    int spi_num, addr_offset, temp_offset, len, cs, sector_start, sector_end, i;
    unsigned int val, count, tmp;
    uchar *rd_buf, *wr_buf, *src, *c_ptr, inbuf[3];

    spi_num = gethex_answer("Select SPI PROM:0-Golden, 1-Upgrate, 2-FPGA",
				    0, 0, 2);

    addr_offset = gethex_answer("Enter spi prom address in hex",
			 LAST_SECTOR_ADDR, 0, SPI_PROM_SIZE - 1);
    
    sector_start = addr_offset & ~(SECTOR_SIZE - 1);
    sector_end = sector_start + (SECTOR_SIZE - 1);
    
    rd_buf = malloc(SECTOR_SIZE);
    if (!rd_buf) {
	printf("\nMalloc read back buf failed, len=%d\n", SECTOR_SIZE);
    	return FAILED;
    }

    if (spi_num == 0) {
	cs = ESPI_CS0;
    } else if (spi_num == 1) {
	cs = ESPI_CS2;
    } else {
	cs = ESPI_CS3;
    }

    len = gethex_answer("Enter the number of bytes to write in hex",
			0, 0, SECTOR_SIZE);

    if ((addr_offset + len) > sector_end) {
	printf("\nAttempt to write beyond the current sector end: 0x%08x",
	       sector_end);
	return (FAILED);
    }

    wr_buf = malloc(len);
    if (!wr_buf) {
	printf("\nMalloc write buf failed, len = 0x%08x", len);
	return (FAILED);
    }
    temp_offset = addr_offset;
    printf("\nStart writing at address offset 0x%08x\n", addr_offset);
    for (i = 0; i < len; i++) {
	printf("\nEnter 1 byte at offset 0x%08x: ", temp_offset);
	c_ptr = inbuf;
	count = get_line(c_ptr, sizeof(inbuf));
	tmp = getnum(c_ptr, 16, &val);
	if (tmp == 0) {
	    printf("\nBad value %s \n", c_ptr);
	} else {
	    wr_buf[i] = (uchar)val;
	}
	temp_offset++;
    }
    
    if (patriot_spi_prom_write(wr_buf, addr_offset, len, cs, rd_buf)) {
        printf("Write spi prom sector at %#.8x failed, wr_buf=%#.8x rd_buf=%#.8x\n",
		addr_offset, wr_buf, rd_buf);
    	free(rd_buf);
	free(wr_buf);
	return FAILED;
    }

    dismem(rd_buf, len, addr_offset, sizeof(uchar));
    free(rd_buf);
    free(wr_buf);    
    return PASSED;

}



/**********************************************************************
 *
 * Function: patriot_spi_prom_init
 *
 * This function initializs SPI interface for SPI PROM
 *
 * Input : None
 *
 * Output: PASS/FAIL
 *
 **********************************************************************
 */
int patriot_spi_prom_init(void)
{
    int mode;
    uchar spi_status;
    
    /* 1. Reverse data, msb is sent first
     * 2. Prescale modulus select = 2
     * 3. 8 bits per character. Set LEN0 = 7. 
     * 4. Polarity, Asserted Low, Negated High
     * 5. System clock is the input to the eSPI BRG
     * System clock = ccb_clk/2 = 266/2 = 133Mhz
     * eSPI BRG = 133 Mhz
     * SPI_CLK = (eSPI_BRG)/(2 * (PM + 1)) = 16.67MHz
     * The SPI PROM with read command opcode 0x03 require the SCK < 40MHz
     */
    mode = ESPI_MODEX_REV1 | ESPI_MODEX_PM(4) | ESPI_MODEX_LEN (7) |
           ESPI_MODEX_POL1 | ESPI_MODEX_CSBEF(1) | ESPI_MODEX_CSAFT(1) |
	   ESPI_MODEX_CSCG(1) ;
    
    if (espi_init(ESPI_CS0, mode) == FAILED) {
        sprintf(err_msg, "\n%s, [#%d]:Fail to init spi prom espi.\n", __FUNCTION__, __LINE__);
        print_err(FALSE, err_msg, LVL_1);
        return (FAILED);
    }
    /* Run a dummy command the 1st time
       On the Beagle, the first time it is MOSI 05, MISO FF
       The 2nd time it is MOSI 05 FF, MISO FF 00 */
    spi_status = spi_prom_read_if_status(ESPI_CS0);
    
    if (espi_init(ESPI_CS2, mode) == FAILED) {
        sprintf(err_msg, "\n%s, [#%d]:Fail to init spi prom espi.\n", __FUNCTION__, __LINE__);
        print_err(FALSE, err_msg, LVL_1);
        return (FAILED);
    }    

    /* Run a dummy command the 1st time */
    spi_status = spi_prom_read_if_status(ESPI_CS2);

    /* The SPI SB FPGA needs to be at 8.33MHz */
    mode = ESPI_MODEX_REV1 | ESPI_MODEX_PM(9) | ESPI_MODEX_LEN (7) |
           ESPI_MODEX_POL1 | ESPI_MODEX_CSBEF(1) | ESPI_MODEX_CSAFT(1) |
	   ESPI_MODEX_CSCG(1) ;
    if (espi_init(ESPI_CS3, mode) == FAILED) {
        sprintf(err_msg, "\n%s, [#%d]:Fail to init spi prom espi.\n", __FUNCTION__, __LINE__);
        print_err(FALSE, err_msg, LVL_1);
        return (FAILED);
    }    

    /* Run a dummy command the 1st time */
    spi_status = spi_prom_read_if_status(ESPI_CS3);
    
    return(PASSED);
}


/*******************************************************************************
 *
 * Name        : spi_prom_write_multi_bytes
 * Description : Write the SPI PROM chip to the
 *               standard SPI interface
 * Input       : addr_offset - offset of the buffer data
 *               *data       - buffer data byte
 *               cs          - chip select
 *               size        -  number of bytes to write, max 256 bytes
 * Output      : buffer data byte
 *               PASSED / FAILED
 *
 *******************************************************************************
 */
int spi_prom_write_multi_bytes (uint addr_offset, uchar *data, int cs, int size)
{
    uchar spi_cmd[SPI_PROM_WRITE_HEADER + SPI_PROM_PAGE_SIZE];
    uchar spi_reply[SPI_PROM_WRITE_HEADER + SPI_PROM_PAGE_SIZE];
    uchar dummy[2] = {0xFF, 0xFF};
    int ix;
    volatile uchar status;

    if (size > SPI_PROM_PAGE_SIZE) {
	sprintf(err_msg, "\n%s, [#%d]:Can't write more than 256 bytes\n",
			__FUNCTION__, __LINE__);
	print_err(FALSE, err_msg, LVL_1);
	return (FAILED);
    }

    status = spi_prom_read_if_status(cs);
    /* Check for WIP bit */
    if ((status & SPI_PROM_WIP) != 0x0) {	
	for (ix = 0; ix < MAX_SPI_SPIN; ix++) {
	    status = spi_prom_read_if_status(cs);
	    if ((status & SPI_PROM_WIP) != 0x0) {
		usleep(1);
	    } else {
		break;
	    }
	}
	if (ix == MAX_SPI_SPIN) {
	    sprintf(err_msg, "\n%s, [#%d]:Time out waiting for WIP signal,"
	    		" status = 0x%02x", __FUNCTION__, __LINE__, status);
	    print_err(FALSE, err_msg, LVL_1);
	    return (FAILED);
	}
    }

    
    /* Write the WREN first */
    spi_cmd[0] = SPI_PROM_WREN;
    spi_cmd[1] = 0xff;
    if (access_spi(1, &spi_cmd[0], &dummy[0], cs, 1, FALSE, TRUE) == FAILED) {
        sprintf(err_msg, "\n%s, [#%d]:Failed to write WREN to SPI PROM\n",
        		__FUNCTION__, __LINE__);
        print_err(FALSE, err_msg, LVL_1);
        return (FAILED);
    }

    for (ix = 0; ix < 20; ix++) {
	status = spi_prom_read_if_status(cs);
	if (status & SPI_PROM_WEL) {
	    break;
	}
	msleep(1);
    }

#ifdef DEBUG    
    printf("\nstatus = 0x%02x\n", status);
#endif
    
    if (!(status & SPI_PROM_WEL)) {
	sprintf(err_msg, "\n%s, [#%d]:Time out waiting for WEL signal",
			__FUNCTION__, __LINE__);
	print_err(FALSE, err_msg, LVL_1);
	return (FAILED);
    }
    
    /* The first byte is the write command */
    spi_cmd[0] = SPI_PROM_WRITE;

    /* The second byte is the address */
    spi_cmd[1] = (uchar)((addr_offset & MASK_BITS54) >> 16);
    spi_cmd[2] = (uchar)((addr_offset & MASK_BITS32) >> 8);
    spi_cmd[3] = (uchar)(addr_offset & MASK_BITS10);

    /* Fifth byte is data in */
    for (ix = 0; ix < size; ix++) {
	spi_cmd[ix + 4] = *data;
	data++;
    }
    
    spi_reply[0] = 0xFF;
    spi_reply[1] = 0xFF;
    spi_reply[2] = 0xFF;
    spi_reply[3] = 0xFF;
    spi_reply[4] = 0xFF;

    if (access_spi(SPI_PROM_WRITE_HEADER + size, &spi_cmd[0], &spi_reply[0],
		   cs, SPI_PROM_WRITE_CMD_LEN + (size -1), FALSE, TRUE) == FAILED) {
        sprintf(err_msg, "\n%s, [#%d]:Failed to write SPI PROM\n",
        		__FUNCTION__, __LINE__);
        print_err(FALSE, err_msg, LVL_1);
        return (FAILED);
    }

    return (PASSED);
}

/*******************************************************************************
 *
 * Name        : spi_prom_read_multi_bytes
 * Description : Read the SPI PROM chips from the
 *               standard SPI interface
 * Input       : addr_offset - offset of the buffer data
 *               *data       - buffer data byte
 *               cs          - chip select
 *               size        - number of bytes to read
 *
 * Output      : buffer data byte
 *               PASSED / FAILED
 *
 *******************************************************************************
 */
int spi_prom_read_multi_bytes (uint addr_offset, uchar *spi_reply, int cs,
			       int size)
{
    uchar spi_cmd[SPI_PROM_READ_CMD_LEN], i;
    int block_size, num_blocks;
    int ix;
    volatile uchar status;
    

    /* Make sure not reading over the memory boundery */
    if (cs == ESPI_CS3) {
	if (size > (FPGA_SPI_PROM_SIZE - addr_offset)) {
	    sprintf(err_msg, "\n%s, [#%d]:Read exceeding the FPGA SPI PROM "
	    		"memory limit", __FUNCTION__, __LINE__);
	    print_err(FALSE, err_msg, LVL_1);
	    return (FAILED);
	}
    } else {
	if (size > (SPI_PROM_SIZE - addr_offset)) {
	    sprintf(err_msg, "\n%s, [#%d]:Read exceeding the SPI PROM memory limit",
	    		__FUNCTION__, __LINE__);
	    print_err(FALSE, err_msg, LVL_1);
	    return (FAILED);
	}
    }

    /* The maximum frame size is limited to 64K or 0x10000 (0xFFFF + 1) of
       TRANLEN of SPCOM. There are 4 bytes for the SPI_PROM_READ_CMD_LEN
       so the maximum bytes for 1 read is 0x10000 - 4 = 65532 bytes. To
       make it easier for computing, I set the max read size to 32K */

    if (size % BLOCK_32K_SIZE) {
	num_blocks = size/BLOCK_32K_SIZE + 1;
    } else {
	num_blocks = size/BLOCK_32K_SIZE;
    }

    if (size >= BLOCK_32K_SIZE) {
	block_size = BLOCK_32K_SIZE;
    } else {
	block_size = size;
    }
    
    status = spi_prom_read_if_status(cs);
    if ((status & SPI_PROM_WIP) != 0x0) {	
	for (ix = 0; ix < MAX_SPI_SPIN; ix++) {
	    status = spi_prom_read_if_status(cs);
	    if ((status & SPI_PROM_WIP) != 0x0) {
		msleep(1);
	    } else {
		break;
	    }
	}
	if (ix == MAX_SPI_SPIN) {
	    sprintf(err_msg, "\n%s, [#%d]:Time out waiting for WIP before read,"
	    		" status = 0x%02x", __FUNCTION__, __LINE__, status);
	    print_err(FALSE, err_msg, LVL_1);
	    fflush(0);
	    return (FAILED);
	}
    }

    
#ifdef DEBUG
    printf("\nnum_blocks = %d", num_blocks);fflush(0);
#endif
    
    for (i = 0; i < num_blocks; i++) {
    
	/* The first byte is the read command */
	spi_cmd[0] = SPI_PROM_READ;
	
	/* The second byte is the address */
	spi_cmd[1] = (uchar)((addr_offset & MASK_BITS54) >> 16);
	spi_cmd[2] = (uchar)((addr_offset & MASK_BITS32) >> 8);
	spi_cmd[3] = (uchar)(addr_offset & MASK_BITS10);
#ifdef DEBUG
	printf("\n i = %d", i);fflush(0);
	printf("\nsize = %d", size);fflush(0);
	printf("\nblock_size = %d", block_size);fflush(0);
	printf("\ni * BLOCK_32K_SIZE = %d", i * BLOCK_32K_SIZE);fflush(0);
	printf("\n&spi_reply[i * BLOCK_32K_SIZE] = 0x%08x",
	       &spi_reply[i * BLOCK_32K_SIZE]);fflush(0);
#endif	
	if (size >= BLOCK_32K_SIZE) {
	    size = size - BLOCK_32K_SIZE;
	    block_size = BLOCK_32K_SIZE;
	} else {
	    block_size = size;
	}

	if (access_spi(SPI_PROM_READ_CMD_LEN, &spi_cmd[0],
		       &spi_reply[i * BLOCK_32K_SIZE],
		       cs, SPI_PROM_READ_CMD_LEN + block_size, TRUE, TRUE) == FAILED) {
	    sprintf(err_msg, "\n%s, [#%d]:Failed to read SPI PROM\n",
	    		__FUNCTION__, __LINE__);
	    print_err(FALSE, err_msg, LVL_1);
	    return (FAILED);
	}
	addr_offset += block_size;
    }
    
#ifdef DEBUG    
    for (i = 0; i < size; i++) {
	printf("\n spi_reply[0x%02x] = 0x%02x", i,  spi_reply[i]);fflush(0);
    }
#endif
    
    return (PASSED);
}



/*------------------------------------------------------------------------------
 * $Log: patriot_espi.c,v $
 * Revision 1.1  2014/03/25 02:12:34  huanngo
 * Adding patriot_linux directory to ng_diag code tree
 *
 * Revision 1.9  2012/12/03 12:35:16  steja
 * 1. Add Error message utility
 * 2. Fix Framer interrupt Diagnostic loopback
 *
 * Revision 1.8  2012/10/15 18:55:29  huanngo
 * Adjust the SPI clock for SPI SB FPGA to be at 8.33MHz
 *
 * Revision 1.7  2012/08/21 01:18:00  huanngo
 * Checking WIP signal beforew write/read multiple bytes
 *
 * Revision 1.6  2012/08/06 17:34:57  huanngo
 * Fix bugs in SPI PROM test/read/write/erase
 *
 * Revision 1.5  2012/07/31 00:12:11  huanngo
 * Fix the bug in the module submenu utility to modify SPI PROM data
 *
 * Revision 1.4  2012/07/18 23:49:52  huanngo
 * Adding read/write multiple bytes to SPI PROM and support the FPGA SPI PROM (CS3)
 *
 * Revision 1.3  2012/06/29 22:43:32  huanngo
 * Adding polling loop to check for WEL signal before writing
 *
 * Revision 1.2  2012/05/08 23:52:55  huanngo
 * Support SM Patriot on ngd main code tree
 *
 * Revision 1.1.4.9  2012/04/12 18:37:02  huanngo
 * Clean up and cosmetic changes
 *
 * Revision 1.1.4.8  2012/03/27 07:50:00  steja
 * Fix Warning compilation
 *
 * Revision 1.1.4.7  2012/01/09 23:06:18  huanngo
 * Support on xformers mips and informers and clean up
 *
 * Revision 1.1.4.6  2011/12/01 18:51:05  huanngo
 * Support new command to write MAC address to EEPROM and fix bugs
 *
 * Revision 1.1.4.5  2011/11/14 09:32:53  steja
 * Update patriot_espi code
 * Fix the SPI Read / Write Function
 *
 * Revision 1.1.4.4  2011/10/11 01:51:29  steja
 * Update DS3170 Register test code
 *
 * Revision 1.1.4.3  2011/10/07 01:11:45  huanngo
 * Update code to support HDLC, SPI EEPROM and FPGA
 *
 * Revision 1.1.4.2  2011/08/18 19:43:24  huanngo
 * Update code to patriot2-branch
 *
 * Revision 1.1.2.5  2011/08/16 17:57:59  huanngo
 * Fix bugs for SPI EEPROM
 *
 * Revision 1.1.2.4  2011/08/06 00:17:40  huanngo
 * Update code for Patriot
 *
 * Revision 1.1.2.3  2011/07/19 06:11:34  huanngo
 * Update code per code review comments
 *
 * Revision 1.1.2.2  2011/07/14 14:38:27  steja
 * Update Patriot Project Module side code
 *
 * Revision 1.1.2.1  2011/07/08 00:05:00  huanngo
 * Change ds3170_espi.c to patriot_espi.c
 *
 * 
 *
 *------------------------------------------------------------------------------
 * $Endlog$
 *
 *------------------------------------------------------------------------------
 */
