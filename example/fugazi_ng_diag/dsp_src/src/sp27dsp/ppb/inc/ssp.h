/* $Id: ssp.h,v 1.3 2012/08/15 14:32:31 srane Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/dsp_src/src/sp27dsp/ppb/inc/ssp.h,v $
 *------------------------------------------------------------------
 * ssp.h
 *      SSP Port defines
 *
 * June 2012, Smita Rane
 *
 * Copyright (c) 2012 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */
/*****************************************************************************
 *                             NOTIFICATION
 *
 * Copyright © 2009-2012 LSI Inc.
 * All Rights Reserved
 *
 * This is unpublished proprietary information of LSI Inc.  This
 * copyright notice does not evidence publication.
 *
 * The use of the software, documentation, methodologies, and other information
 * contained herein is governed solely by the associated license agreements.
 * Any inconsistent use shall be deemed to be a misappropriation of the
 * intellectual property of LSI Inc. and treated accordingly.
 *----------------------------------------------------------------------------
 *
 * $RCSfile: ssp.h,v $
 *
 * Description:	 SSP (Serial Peripheral Interface) interface support
 *
 * Initial revision
 * Member added to project
 *
 *****************************************************************************/

/* !MANFILE:	irom_ssp.h */

#ifndef __LSI_SP27XX_SSP_H
#define __LSI_SP27XX_SSP_H

#define LSI_SP27XX_SSPBASE		0x3004F000
/*
** SSP Register Locations
*/

#define LSI_SP27XX_SSPCR0		((volatile uint32_t *)(LSI_SP27XX_SSPBASE + 0x00))  /* SSP Control Register 0	*/
#define LSI_SP27XX_SSPCR1		((volatile uint32_t *)(LSI_SP27XX_SSPBASE + 0x04))  /* SSP Control Register 1	*/
#define LSI_SP27XX_SSPDR		((volatile uint32_t *)(LSI_SP27XX_SSPBASE + 0x08))  /* SSP Data Register		*/
#define LSI_SP27XX_SSPSR		((volatile uint32_t *)(LSI_SP27XX_SSPBASE + 0x0C))	/* SSP Status Register		*/
#define LSI_SP27XX_SSPCPSR		((volatile uint32_t *)(LSI_SP27XX_SSPBASE + 0x10))	/* SSP Clock Pre-Scale Register	*/
#define LSI_SP27XX_SSPIMSC		((volatile uint32_t *)(LSI_SP27XX_SSPBASE + 0x14))	/* SSP Interrupt Mask Register	*/
#define LSI_SP27XX_SSPRIS		((volatile uint32_t *)(LSI_SP27XX_SSPBASE + 0x18))	/* SSP Raw Interrupt Register	*/
#define LSI_SP27XX_SSPMIS		((volatile uint32_t *)(LSI_SP27XX_SSPBASE + 0x1C))	/* SSP Masked Status Register	*/
#define LSI_SP27XX_SSPICR		((volatile uint32_t *)(LSI_SP27XX_SSPBASE + 0x20))	/* SSP Interrupt Clear Register	*/
#define LSI_SP27XX_SSPDMACR		((volatile uint32_t *)(LSI_SP27XX_SSPBASE + 0x24))	/* SSP DMA Control Register		*/

/* SSPCR0 Control register 0 */
#define LSI_SP27XX_SSPCR0_SCR_DFLT		0x00000000		/* Serial Clock Rate (divide), default set at 0 */
#define LSI_SP27XX_SSPCR0_SPH			0x00000080		/* SSPCLKOUT phase								*/
#define LSI_SP27XX_SSPCR0_SPO			0x00000040		/* SSPCLKOUT polarity							*/
#define LSI_SP27XX_SSPCR0_FRF_MOT		0x00000000		/* Frame format, Motorola						*/
#define LSI_SP27XX_SSPCR0_DSS_8			0x00000007		/* Data packet size, 8bits						*/
#define LSI_SP27XX_SSPCR0_DSS_16       	0x0000000F		/* Data packet size, 16bits						*/

#define LSI_SP27XX_SSPCR0_FRF_MOT_00   	LSI_SP27XX_SSPCR0_FRF_MOT	/* Frame format,Motorola mode 0	*/
#define LSI_SP27XX_SSPCR0_FRF_MOT_11  ( LSI_SP27XX_SSPCR0_FRF_MOT |	\
										LSI_SP27XX_SSPCR0_SPH	| \
										LSI_SP27XX_SSPCR0_SPO)		/* Frame format,Motorola mode 3 */

/* SSPCR1 Control register */
#define LSI_SP27XX_SSPCR1_SOD			0x00000008		/* Slave output mode disable "1" TX is disabled	*/
#define LSI_SP27XX_SSPCR1_MS			0x00000004		/* Master "0" or Slave "1" mode					*/
#define LSI_SP27XX_SSPCR1_SSE			0x00000002		/* Serial port enable "1" or disable "0"		*/
#define LSI_SP27XX_SSPCR1_LBM			0x00000001		/* Loop back mode on "1" or normal operation "0"*/

/* SSPSR Status register */
#define LSI_SP27XX_SSPSR_BSY			0x00000010		/* Busy								*/
#define LSI_SP27XX_SSPSR_RFF			0x00000008		/* Receive  FIFO full				*/
#define LSI_SP27XX_SSPSR_RNE			0x00000004		/* Receive  FIFO not empty			*/
#define LSI_SP27XX_SSPSR_TNF			0x00000002		/* Transmit FIFO not full			*/
#define LSI_SP27XX_SSPSR_TFE			0x00000001		/* Transmit FIFO empty				*/

/* SSPCPSR Clock prescale register */

//#define LSI_SP27XX_SSPCPSR_MASTER_DFLT	0x00000064		/* Clock prescale (use with SCR), master default set at 100 */
#define LSI_SP27XX_SSPCPSR_MASTER_DFLT_PLL	0x000000BC		/* Clock prescale (use with SCR), master default set at 188 */

#define LSI_SP27XX_SSPCPSR_MASTER_DFLT_NOPLL	0x00000019		/* Clock prescale (use with SCR), master default set at 19 if the clock is 50Mhz, ARM=25MHz */

#define LSI_SP27XX_SSPCPSR_SLAVE_DFLT	0x0000000C		/* Clock prescale (use with SCR), slave default set at 12 	*/

/* SSPIMSC Interrupt mask set and clear register */
#define LSI_SP27XX_SSPIMSC_TXIM			0x00000008		/* Transmit FIFO not Masked			*/
#define LSI_SP27XX_SSPIMSC_RXIM			0x00000004		/* Receive  FIFO not Masked			*/
#define LSI_SP27XX_SSPIMSC_RTIM			0x00000002		/* Receive timeout not Masked		*/
#define LSI_SP27XX_SSPIMSC_RORIM		0x00000001		/* Receive overrun not Masked		*/

/* SSPRIS Raw interrupt status register */
#define LSI_SP27XX_SSPRIS_TXRIS			0x00000008		/* Raw Transmit interrupt flag		*/
#define LSI_SP27XX_SSPRIS_RXRIS			0x00000004		/* Raw Receive  interrupt flag		*/
#define LSI_SP27XX_SSPRIS_RTRIS			0x00000002		/* Raw Timemout interrupt flag		*/
#define LSI_SP27XX_SSPRIS_RORRIS		0x00000001		/* Raw Overrun  interrupt flag		*/

/* SSPMIS Masked interrupt status register */
#define LSI_SP27XX_SSPMIS_TXMIS			0x00000008		/* Masked Transmit interrupt flag	*/
#define LSI_SP27XX_SSPMIS_RXMIS			0x00000004		/* Masked Receive  interrupt flag	*/
#define LSI_SP27XX_SSPMIS_RTMIS			0x00000002		/* Masked Timemout interrupt flag	*/
#define LSI_SP27XX_SSPMIS_RORMIS		0x00000001		/* Masked Overrun  interrupt flag	*/

/* SSPICR Interrupt clear register */
#define LSI_SP27XX_SSPICR_RTIC			0x00000002		/* Clears Timeout interrupt flag	*/
#define LSI_SP27XX_SSPICR_RORIC			0x00000001		/* Clears Overrun interrupt flag	*/

/* SSPDMACR DMA control register */
#define LSI_SP27XX_SSPDMACR_TXDMAE		0x00000002		/* Enable Transmit FIFO DMA			*/
#define LSI_SP27XX_SSPDMACR_RXDMAE		0x00000001		/* Enable Receive  FIFO DMA			*/

/* SSP port configuration */
#define LSI_SP27XX_SSPFIFO_SIZE			8

/* EEPROM instruction set */
#define LSI_SP27XX_EEWRSR				0x01		/* Write status				*/
#define LSI_SP27XX_EEPROG				0x02		/* Program data				*/
#define LSI_SP27XX_EEREAD				0x03		/* Read data				*/
#define LSI_SP27XX_EEWRDI				0x04		/* Write disable			*/
#define LSI_SP27XX_EERDSR				0x05		/* Read status				*/
#define LSI_SP27XX_EEWREN				0x06		/* Write enable				*/
#define LSI_SP27XX_EERDID                               0x9F            /* Read ID                              */

/* EEPROM status register flags */
#define LSI_SP27XX_EERDSR_WIP			0x0001		/* Write in process			*/
#define LSI_SP27XX_EERDSR_WEL			0x0002		/* Write enable latch		*/
#define LSI_SP27XX_EERDSR_BP0			0x0004		/* Block protect 0			*/
#define LSI_SP27XX_EERDSR_BP1			0x0008		/* Block protect 1			*/
#define LSI_SP27XX_EERDSR_WPEN			0x0080		/* Write protect enable		*/
#define LSI_SP27XX_EERDSR_NO_BLOCK_PROTECT	0xf3		/* LSI_SP27XX_NO_BLOCK_PROTECT */
#define LSI_SP27XX_EERDSR_WEN			0x2

/* ST Micro serial FLASH instruction set */
#define LSI_SP27XX_FLASH_SE_NGVM			0xd8
#define LSI_SP27XX_FLASH_SE_SDB				0x52 //0xD8		/* sector erase 				*/
#define LSI_SP27XX_FLASH_BE				0x62 //0xC7		/* bulk erase 					*/
#define LSI_SP27XX_FLASH_DP				0xB9		/* put to deep power-down		*/
#define LSI_SP27XX_FLASH_RES			0xAB		/* release from deep power-down	*/

/* SSP Interface operating modes */
#define	LSI_SP27XX_SSP_MASTER_MODE		0x00000001
#define	LSI_SP27XX_SSP_SLAVE_MODE		0x00000000

/*
**	SP2704 EEPROM Function declarations
*/

void spi_init(void) ;

int eeprom_write(
		unsigned 	offset,
		uint8_t         *buffer,
		unsigned 	cnt) ;

int eeprom_read (
		unsigned offset,
		uint8_t *buffer,
		unsigned cnt) ;

#endif /* __LSI_SP27XX_SSP_H */

/******** History ********
$Log: ssp.h,v $
Revision 1.3  2012/08/15 14:32:31  srane
Add the correct version of ssp code.

Revision 1.2  2012/07/17 20:34:28  srane
cleanup

Revision 1.1  2012/06/28 13:33:39  srane
SSP support



$Endlog$
*/

