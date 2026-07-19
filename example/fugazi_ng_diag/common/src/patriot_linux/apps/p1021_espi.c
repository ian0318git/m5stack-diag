/* $Id: p1021_espi.c,v 1.1 2014/03/25 02:12:33 huanngo Exp $
 * $Source: 
 *------------------------------------------------------------------
 *
 * p1021_espi.c:  P1021 eSPI Related functions
 *
 * May 2011 - steja
 *
 * Copyright (c) 2011 - 2014 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 *
 *------------------------------------------------------------------
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
#include "patriot_main.h"
#include "router_if.h"
#include "sgmii_defs.h"
#include "p1021_immap.h"
#include "p1021_espi.h"
#include "common_utils.h"


/***********************************************************************
 *  Macro Processor Definitions
 ************************************************************************/

/***********************************************************************
 *  Functions Declaration
 ************************************************************************/

int espi_init(int, int);
void espi_activate(int, int, int, int);
void espi_deactivate(int);
int espi_xfr(int, uchar *, uchar *, int, int, int);

/***********************************************************************
 *  Extern Functions Declaration
 ************************************************************************/

/***********************************************************************
 *  Global Variable
 ************************************************************************/

/***********************************************************************
 *  Functions
 ************************************************************************/


/**************************************************************************
 * Function: espi_init
 *
 * Description: eSPI Init function
 *
 * Input: spi_num - the number of SPI (1~4)
 *        mode - value to be configured on SPMODEx
 *
 * Output: PASSED/FAILED
 *************************************************************************/ 
int espi_init (int spi_num, int mode)
{
    volatile ccsr_espi_t *espi = &(REGB->im_espi);
    /* Init SPI Mode based on which CS */
    switch (spi_num) {
        case ESPI_CS0:
            espi->spmode0 = mode;
            break;
        case ESPI_CS1:
            espi->spmode1 = mode;
            break;
        case ESPI_CS2:
            espi->spmode2 = mode;
            break;
        case ESPI_CS3:
            espi->spmode3 = mode;
            break;
        default:
            printf("\nInvalid SPI Number (%d)\n", spi_num);
            return (FAILED);
    }

    /* Clear all Interrupt Event and mask the interrupt */
    espi->spie = 0xFFFFFFFF;
    espi->spim = 0x0;

    /* Clear out the command register now */
    espi->spcom = 0;

    /* Disable SPI for now */
    espi->spmode &= ~(ESPI_MODE_EN);
#ifdef DEBUG    
    display_espi_registers();
#endif    
    return (PASSED);
}


/**************************************************************************
 * Function: espi_activate
 *
 * Description: This function activates the SPI interface for selected CS
 *
 * Input: spi_num    - the number of SPI (1~4)
 *        byte_count - Number of bytes.
 *
 * Output: None
 *************************************************************************/ 
void espi_activate (int spi_num, int tx_bytes, int rx_bytes, int rd_flag)
{
    volatile ccsr_espi_t *espi = &(REGB->im_espi);
    uint which_cs;

    switch (spi_num) {
        case ESPI_CS0:
            which_cs = ESPI_COM_CS0;
            break;
        case ESPI_CS1:
            which_cs = ESPI_COM_CS1;
            break;
        case ESPI_CS2:
            which_cs = ESPI_COM_CS2;
            break;
        case ESPI_CS3:
            which_cs = ESPI_COM_CS3;
            break;
        default:
            printf("Invalid SPI Number (%d)", spi_num);
            return;
    }

    /* Enable eSPI now */
    espi->spmode |= ESPI_MODE_EN;

    if (rd_flag) {
	/* Enable CS */
	espi->spcom = which_cs | (rx_bytes - 1)| ESPI_COM_RX_SKIP(tx_bytes);
    } else {
	/* Enable CS */
	espi->spcom = which_cs | ESPI_COM_TO |
	    (tx_bytes - 1); /* One byte less than the frame length */
    }
}


/**************************************************************************
 * Function: espi_deactivate
 *
 * Description: This function deactivates the SPI interface for selected CS
 *
 * Input: spi_num - the number of SPI (1~4)
 *
 * Output: None
 *************************************************************************/ 
void espi_deactivate (int spi_num)
{
    volatile ccsr_espi_t *espi = &(REGB->im_espi);
    uint which_cs;

    switch (spi_num) {
        case ESPI_CS0:
            which_cs = ESPI_COM_CS0;
            break;
        case ESPI_CS1:
            which_cs = ESPI_COM_CS1;
            break;
        case ESPI_CS2:
            which_cs = ESPI_COM_CS2;
            break;
        case ESPI_CS3:
            which_cs = ESPI_COM_CS3;
            break;
        default:
            printf("Invalid SPI Number (%d)", spi_num);
            return;
    }

    /* Disable eSPI now */
    espi->spmode &= ~(ESPI_MODE_EN);
    espi->spcom = 0;
}


/**************************************************************************
 * Function: espi_xfr
 *
 * Description: This function transfers the data from and to SPI slave device
 *
 * Input: len - length of both command and reply buffers.
 *        txbuf - ptr to TX buffer
 *        rxbuf - ptr to RX buffer
 *
 * Output: PASSED/FAILED
 *************************************************************************/
int espi_xfr (int tx_bytes, uchar *txbuf, uchar *rxbuf, int rx_bytes,
	      int rd_flag, int ex_flag)
{
    volatile ccsr_espi_t *espi = &(REGB->im_espi);
    int num_blks;
    int ix, timeout;
    int char_size = BLOCK_SIZE4;
    uint tmp_dout, tmp_din, tmp_din1;

    /* Clear all SPI events */
    espi->spie = RESET_VALUE;

    /* Handle data in 4 bytes chunks */
    num_blks = (tx_bytes/BLOCK_SIZE4)+(tx_bytes % BLOCK_SIZE4 ? BLOCK_SIZE1 : 0);
#ifdef DEBUG    
    printf("\n%d num_blks = %d", __LINE__, num_blks);
    printf("\n%d tx_bytes = %d", __LINE__, tx_bytes);
#endif    
    while (num_blks--) {
        char_size = (tx_bytes >= BLOCK_SIZE4 ? BLOCK_SIZE4 : tx_bytes);
        tx_bytes -= char_size;
        
        tmp_dout = 0;
        
        /* Shift the data so it's msb-justified 
         * The data layout would be:
         * DATA0 DATA1 DATA2 DATA3
         */
        
        for (ix = 0; ix < char_size; ix++) {
            tmp_dout |= (*txbuf) << (24 - (ix * 8));
            txbuf++;
        }
#ifdef DEBUG
	printf("\n%d num_blks = %d", __LINE__, num_blks);
	printf("\n%d tx_bytes = %d", __LINE__, tx_bytes);
	printf("\n%d tmp_dout = 0x%08x", __LINE__, tmp_dout);
#endif	
        /* Write the data out */
        espi->spitf = tmp_dout;

	if (rd_flag == TRUE) {
	    /* Wait for eSPI to transmit the data */
	    for (timeout = 0; timeout < ESPI_XFER_TIMEOUT; timeout++) {
		/* Check if the data is actually transmitted */
		if (espi->spie & ESPI_EV_TNF) {
		    /* Clear the event */
		    espi->spie |= ESPI_EV_TNF;
		    break;
		}
		usleep(100);
	    }
	} else {
	    /* Wait for eSPI to transmit the data */
	    for (timeout = 0; timeout < ESPI_XFER_TIMEOUT; timeout++) {
		/* Check if the data is actually transmitted */
		if (espi->spie & ESPI_EV_TXE) {
		    /* Clear the event */
		    espi->spie |= ESPI_EV_TXE;
		    break;
		}
		usleep(100);
	    }
	}    

        if (timeout == ESPI_XFER_TIMEOUT) {
            printf("\nTimeout when transmitting SPI\n");
            return (FAILED);
        }
    }

    /* Handle data in 4 bytes chunks */
    
    if ((rd_flag == TRUE) && (ex_flag == TRUE)) {
	for (timeout = 0; timeout < ESPI_XFER_TIMEOUT; timeout++) {
	    /* Check if the data arrives at FIFO */
	    if (espi->spie & ESPI_EV_RNE) {
		break;
	    }
	    usleep(100);
	}
	if (timeout == ESPI_XFER_TIMEOUT) {
	    printf("\nTimeout when receiving data\n");
	    return (FAILED);
	}
	
	num_blks = (rx_bytes/BLOCK_SIZE4)+(rx_bytes % BLOCK_SIZE4 ? BLOCK_SIZE1 : 0);
#ifdef DEBUG
	printf("\n%d rx_bytes = %d", __LINE__, rx_bytes);
	printf("\n***num_blks = %d***", num_blks);
#endif	
	while (num_blks--) {
	    
	    char_size = (rx_bytes >= BLOCK_SIZE4 ? BLOCK_SIZE4 : rx_bytes);
	    rx_bytes -= char_size;
#ifdef DEBUG
	    printf("\nnum_blks = %d", num_blks);
	    printf("\nchar_size = %d", char_size);
	    printf("\nrx_bytes = %d", rx_bytes);
#endif	    
	
	    tmp_din = 0;
	    
	    /* Wait for receiving data */
	    usleep(100);
	    
	    /* Fetch the data */
	    tmp_din = espi->spirf;
#ifdef DEBUG	    
	    printf("\ntmp_din = 0x%08x", tmp_din);
#endif	    
	    /* Rearrange the data */
	    for (ix = 0; ix < char_size; ix++) {
		*rxbuf = ((tmp_din) >> (24 - (ix * 8))) & 0xff;
#ifdef DEBUG		
		printf("\n*rxbuf = 0x%02x, ix = %d", *rxbuf, ix);
#endif		
		rxbuf++;
	    }
	}
    }
    
    msleep(2);

    return (PASSED);
}



/*------------------------------------------------------------------------------
 * $Log: p1021_espi.c,v $
 * Revision 1.1  2014/03/25 02:12:33  huanngo
 * Adding patriot_linux directory to ng_diag code tree
 *
 * Revision 1.4  2012/07/31 00:12:11  huanngo
 * Fix the bug in the module submenu utility to modify SPI PROM data
 *
 * Revision 1.3  2012/07/18 23:47:26  huanngo
 * Support read and write multiple bytes to SPI PROM
 *
 * Revision 1.2  2012/05/08 23:52:54  huanngo
 * Support SM Patriot on ngd main code tree
 *
 * Revision 1.1.4.6  2012/01/09 23:06:18  huanngo
 * Support on xformers mips and informers and clean up
 *
 * Revision 1.1.4.5  2011/10/11 01:51:29  steja
 * Update DS3170 Register test code
 *
 * Revision 1.1.4.4  2011/10/07 01:11:45  huanngo
 * Update code to support HDLC, SPI EEPROM and FPGA
 *
 * Revision 1.1.4.3  2011/08/26 14:44:56  steja
 * Update p1021 code to display SPI registers
 *
 * Revision 1.1.4.2  2011/08/18 19:43:23  huanngo
 * Update code to patriot2-branch
 *
 * Revision 1.1.2.7  2011/08/16 17:57:59  huanngo
 * Fix bugs for SPI EEPROM
 *
 * Revision 1.1.2.6  2011/08/06 00:17:39  huanngo
 * Update code for Patriot
 *
 * Revision 1.1.2.5  2011/07/19 06:11:34  huanngo
 * Update code per code review comments
 *
 * Revision 1.1.2.4  2011/07/14 14:38:27  steja
 * Update Patriot Project Module side code
 *
 * Revision 1.1.2.3  2011/05/26 00:38:11  huanngo
 * Update with SPI PROM access and FPGA, DS3170 reset functions
 * Change the SPI read/write to uchar access
 *
 * Revision 1.1.2.2  2011/05/25 16:05:04  steja
 * Update the DS3170 testing function based on specs
 *
 *------------------------------------------------------------------------------
 * $Endlog$
 *
 *------------------------------------------------------------------------------
 */
