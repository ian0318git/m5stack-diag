/* $Id: ssp.c,v 1.3 2013/03/08 23:10:56 srane Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/dsp_src/src/sp27dsp/ppb/src/ssp.c,v $
 *------------------------------------------------------------------
 * ssp.c
 *      SSP Port functions
 *
 * June 2012, Smita Rane
 *
 * Copyright (c) 2012-2013 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */
/*****************************************************************************
 *                             NOTIFICATION
 *
 * Copyright © 2009 LSI Inc.
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
 * $RCSfile: ssp.c,v $
 *
 * Description:	Implementation of functions supporting the SP2704 SSP port
 *
 *
 * Initial revision
 * Member added to project
 *
 *****************************************************************************/
#include	<lsi_sp27xx_ssp.h>
#include        <stdint.h>
#include        "stdio.h"
#include	<ssp.h>
#include	"debug_console.h"

#define readl(a) (*(volatile unsigned int *)(a))
#define writel(v,a)         (*(volatile unsigned int *)(a) = (v))

#define LSI_SP27XX_SOF_MASK	0x7f
#define LSI_SP27XX_SOF		0x66
#define LSI_SP27XX_GO		0x5d

#define CONFIG_SYS_SPI_MODE	0x1
#define CONFIG_SYS_SPI_CR0	0xc7
#define CONFIG_SYS_SPI_CPSR	0xbc

#define WRITE_CACHE_INIT_STATE	0xff

//#define MAX_EEPROM_WRITE	256
#define SECTOR_MASK		0xf0000
#define SECTOR_SIZE		0x10000

#define EEPROM_ID_NGVM          0x13
#define EEPROM_ID_SDB           0xFF

//extern void do_mem_md (char* cmdargs);

//uint8_t		eeprom_write_cache[SECTOR_SIZE] __attribute__((section(".eeprom")));

uint8_t	eeprom_write_cache[SECTOR_SIZE+30];
int eeprom_write (unsigned offset, uint8_t *buffer, unsigned cnt);
void spi_init(void);

static int spi_write_enable (unsigned, int) ;
static int spi_erase(uint8_t *eeaddr) ;
static int	spi_write(uint8_t *eeaddr,int alen,uint8_t* pDataIn,int size);
static int spi_read(uint8_t *eeaddr,int alen,uint8_t *pDataIn,int size) ;
static uint32_t LSI_SP27XX_SSP_WaitRX(void);

uint8_t ssp_eeprom_id[20];

static void spi_read_id(void) {
	uint8_t		temp;
	uint8_t		max_read_size;
	uint8_t		read_size=20;
	int32_t 	i;
	uint32_t	sr ;

	/* make sure thee SSP port is not busy */
	sr = readl(LSI_SP27XX_SSP_SSPSR_RA) ;
	while ((sr & LSI_SP27XX_SSPSR_BSY_BM) == LSI_SP27XX_SSPSR_BSY_BM) {
		sr = readl(LSI_SP27XX_SSP_SSPSR_RA) ;
	}

	/* Make the transmit fifo is empty */
	sr = readl(LSI_SP27XX_SSP_SSPSR_RA) ;
	while ((sr & LSI_SP27XX_SSPSR_TFE_BM) != LSI_SP27XX_SSPSR_TFE_BM) {
		sr = readl(LSI_SP27XX_SSP_SSPSR_RA) ;
	}

	/* Make sure the receive fifo is empty */
	sr = readl(LSI_SP27XX_SSP_SSPSR_RA) ;
	while ((sr & LSI_SP27XX_SSPSR_RNE_BM) == LSI_SP27XX_SSPSR_RNE_BM) {
		temp = (uint8_t)readl(LSI_SP27XX_SSP_SSPDR_RA) ; /* dummy read */
		sr = readl(LSI_SP27XX_SSP_SSPSR_RA) ;
	}

	max_read_size = LSI_SP27XX_SSPFIFO_SIZE - 4;
        read_size = max_read_size;
        i = 0;

        /* disable the SSP port */
        writel(readl(LSI_SP27XX_SSP_SSPCR1_RA) & ~LSI_SP27XX_SSPCR1_SSE_BM,
               LSI_SP27XX_SSP_SSPCR1_RA) ;
        
        /* write EEPROM read ID command */
        writel(LSI_SP27XX_EERDID, LSI_SP27XX_SSP_SSPDR_RA) ;
        
        /* dummy writes necessary to actually read data */
        for(i=0; i<7; i++) {
            writel(0xA5, LSI_SP27XX_SSP_SSPDR_RA) ; /* dummy write */
        }
        
        /* enable the SSP port */
        writel(readl(LSI_SP27XX_SSP_SSPCR1_RA) | LSI_SP27XX_SSPCR1_SSE_BM,
               LSI_SP27XX_SSP_SSPCR1_RA) ;
        
        /* wait until TX not busy */
        while (readl(LSI_SP27XX_SSP_SSPSR_RA) & LSI_SP27XX_SSPSR_BSY_BM) ;
        
        while ((readl(LSI_SP27XX_SSP_SSPSR_RA) & LSI_SP27XX_SSPSR_TFE_BM) !=
               LSI_SP27XX_SSPSR_TFE_BM) ;
        
        /* read the actual data */
        for(i=0; i<8; i++) {
            LSI_SP27XX_SSP_WaitRX();
            temp = (uint8_t)readl(LSI_SP27XX_SSP_SSPDR_RA);
            ssp_eeprom_id[i] = temp;
        }
}
/******************************************************************************
FUNCTION: spi_init

DESCRIPTION:
	Initialize the SPI interface

INPUT:
	mode		- operation mode (master or slave)
	cr0_reg		- Control Register 0 value
	cpsr_reg	- Clock Prescale Register value

OUTPUT:
	none

RETURN:
	Returns TRUE if successful or FALSE if SSP times out

COMMENTS:
	none

HISTORY:
	none
*******************************************************************************/
void
spi_init(void)
{
	uint32_t L_temp;
	uint32_t	mode = CONFIG_SYS_SPI_MODE ;
	uint32_t	cr0_reg = CONFIG_SYS_SPI_CR0 ;
	uint8_t		cpsr_reg = CONFIG_SYS_SPI_CPSR ;
	static int 	init_needed = 1;

        if (!init_needed) {
            return;
        }
        init_needed = 0;
        //bsp_debug_printf("\r\n SPI_INIT");

	/* Disable serial port operation */
	writel(0, LSI_SP27XX_SSP_SSPCR1_RA) ;

	/* Mask all FIFO/IRQ interrupts */
	writel(0x0, LSI_SP27XX_SSP_SSPIMSC_RA) ;

	/* Clear interrupts */
	writel(0x3, LSI_SP27XX_SSP_SSPICR_RA) ;

	/* Disable FIFO DMA */
	writel(0, LSI_SP27XX_SSPDMACR) ;

	/* Configure the Control Register 0 (CR0)
	** SCR = cr0_reg[15:8]
	** Set serial clock rate, Format, data size
	*/
	writel(cr0_reg, LSI_SP27XX_SSP_SSPCR0_RA) ;

	/* Configure the Clock Prescale Register (CPSR)
	** CPSDVR = cpsr_reg[7:0]
	** SSPCLKOUT = FSSPCKLK/(CPSDVR x (1+SCR))
	*/
	writel(cpsr_reg, LSI_SP27XX_SSP_SSPCPSR_RA) ;

	/* TX Fifo and RX fifo should be empty, but if they are not
	** this will cause trouble.  Place SSP into Loopback mode
	** and clear contents
	*/

	writel(readl(LSI_SP27XX_SSP_SSPCR1_RA) | LSI_SP27XX_SSPCR1_SSE_BM |
		LSI_SP27XX_SSPCR1_LBM_BM, LSI_SP27XX_SSP_SSPCR1_RA) ;

	/* Wait for the SSP port if busy */;
	while (readl(LSI_SP27XX_SSP_SSPSR_RA) & LSI_SP27XX_SSPSR_BSY_BM) ;

	/* Make sure the transmit fifo is empty */
	while (!(readl(LSI_SP27XX_SSP_SSPSR_RA) & LSI_SP27XX_SSPSR_TFE_BM)) ;

	/* Empty receive fifo */
	while (readl(LSI_SP27XX_SSP_SSPSR_RA) & LSI_SP27XX_SSPSR_RNE_BM)
	{
		L_temp = readl(LSI_SP27XX_SSP_SSPDR_RA) ;
	}

	/* Disable the SSP port and clear the Loopback mode */
	writel(0, LSI_SP27XX_SSP_SSPCR1_RA) ;

	if(mode == LSI_SP27XX_SSP_SLAVE_MODE)
	{
		/* Set the slave mode and disable the output by setting SOD flag */
		writel(LSI_SP27XX_SSPCR1_MS_BM | LSI_SP27XX_SSPCR1_SOD_BM,
			LSI_SP27XX_SSP_SSPCR1_RA) ;
	}

	/* Enable the SSP port */
	writel(readl(LSI_SP27XX_SSP_SSPCR1_RA) | LSI_SP27XX_SSPCR1_SSE_BM,
		LSI_SP27XX_SSP_SSPCR1_RA) ;

        spi_read_id();
	return ;
}

#if 0
/******************************************************************************
LSI_MG_IROM_!MANFUNCTION: LSI_MG_IROM_SSP_UpdateCPSR

DESCRIPTION:
	Reconfigures the SSP Clock Prescale Register (CPSR)

INPUT:
	cpsr_reg - new CPSR register setting

OUTPUT:
	none

RETURN:
	noen

COMMENTS:
	none

HISTORY:
	none
*******************************************************************************/
void
LSI_SP27XX_SSP_UpdateCPSR(
	uint8_t cpsr_reg)
{
	/* Wait for the SSP port if busy */;
	while(readl(LSI_SP27XX_SSP_SSPSR_RA) & LSI_SP27XX_SSPSR_BSY_BM) ;

	/* Disable the SSP port */
	writel(readl(LSI_SP27XX_SSP_SSPCR1_RA) & ~LSI_SP27XX_SSPCR1_SSE_BM,
		LSI_SP27XX_SSP_SSPCR1_RA) ;

	/* Configure the Clock Prescale Register (CPSR) */
	writel((uint32_t)cpsr_reg, LSI_SP27XX_SSP_SSPCPSR_RA) ;

	/* Enable the SSP port */
	writel(readl(LSI_SP27XX_SSP_SSPCR1_RA) | LSI_SP27XX_SSPCR1_SSE_BM,
		LSI_SP27XX_SSP_SSPCR1_RA) ;
}
#endif

/******************************************************************************
LSI_SP27XX_!MANFUNCTION: LSI_SP27XX_SSP_WaitRX

DESCRIPTION:
	Wait for until a byte is in the SSP RX FIFO

INPUT:
	void

OUTPUT:
	none

RETURN:
	none

COMMENTS:
	none

HISTORY:
	none
*******************************************************************************/
static uint32_t
LSI_SP27XX_SSP_WaitRX(void)
{
	volatile uint32_t	sspsr;

	/* Wait for the SSP receive fifo not empty flag */
	do
	{
		sspsr = readl(LSI_SP27XX_SSP_SSPSR_RA) ;
	}while ((sspsr & LSI_SP27XX_SSPSR_RNE_BM) == 0x0) ;

	return sspsr;
}

/******************************************************************************
FUNCTION: spi_read

DESCRIPTION:
	Generic EEPROM read function

INPUT:
	eeaddr			- EEPROM address
	alen			- address size (will always be 3 bytes)
	pDataIn			- address on the host side where data will be stored
	size			- size of data to read

OUTPUT:
	none

RETURN:
	# of bytes actually read

COMMENTS:
	none

HISTORY:
	none
*******************************************************************************/
static int
spi_read(
	uint8_t	*eeaddr,
	int	alen,
	uint8_t	*pDataIn,
	int	size)
{
	uint8_t		temp;
	uint8_t		max_read_size;
	uint8_t		read_size=0;
	int32_t 	i;
	uint32_t	addr ;
	uint32_t	sr ;

	addr = ((eeaddr[0]<<16) | (eeaddr[1]<<8) | (eeaddr[2]&0xff)) ;

        //bsp_debug_printf("\r\n spi_read(0x%x, size 0x%x) translated addr = 0x%x ", eeaddri, size, addr);
	/* make sure thee SSP port is not busy */
	sr = readl(LSI_SP27XX_SSP_SSPSR_RA) ;
	while ((sr & LSI_SP27XX_SSPSR_BSY_BM) == LSI_SP27XX_SSPSR_BSY_BM) {
		sr = readl(LSI_SP27XX_SSP_SSPSR_RA) ;
	}

	/* Make the transmit fifo is empty */
	sr = readl(LSI_SP27XX_SSP_SSPSR_RA) ;
	while ((sr & LSI_SP27XX_SSPSR_TFE_BM) != LSI_SP27XX_SSPSR_TFE_BM) {
		sr = readl(LSI_SP27XX_SSP_SSPSR_RA) ;
	}

	/* Make sure the receive fifo is empty */
	sr = readl(LSI_SP27XX_SSP_SSPSR_RA) ;
	while ((sr & LSI_SP27XX_SSPSR_RNE_BM) == LSI_SP27XX_SSPSR_RNE_BM) {
		temp = (uint8_t)readl(LSI_SP27XX_SSP_SSPDR_RA) ; /* dummy read */
		sr = readl(LSI_SP27XX_SSP_SSPSR_RA) ;
	}

	max_read_size = LSI_SP27XX_SSPFIFO_SIZE - 4;

	while ( size > 0 ) {

		/* disable the SSP port */
		writel(readl(LSI_SP27XX_SSP_SSPCR1_RA) & ~LSI_SP27XX_SSPCR1_SSE_BM,
			LSI_SP27XX_SSP_SSPCR1_RA) ;

		/* write EEPROM read command */
		writel(LSI_SP27XX_EEREAD, LSI_SP27XX_SSP_SSPDR_RA) ;

		/* write EEPROM data address */
		for( i=alen-1; i>=0; i--) {
			writel(((addr >> (i*8)) & 0xFF), LSI_SP27XX_SSP_SSPDR_RA) ;
		}

		/* calculate partial read size */
		if(size > max_read_size) {
			read_size = max_read_size;
		} else {
			read_size = size;
		}

		/* dummy writes necessary to actually read data */
		for(i=0; i<read_size; i++) {
			writel((uint8_t)0xA5, LSI_SP27XX_SSP_SSPDR_RA) ; /* dummy write */
		}

		/* enable the SSP port */
		writel(readl(LSI_SP27XX_SSP_SSPCR1_RA) | LSI_SP27XX_SSPCR1_SSE_BM,
			LSI_SP27XX_SSP_SSPCR1_RA) ;

		/* wait until TX not busy */
		while (readl(LSI_SP27XX_SSP_SSPSR_RA) & LSI_SP27XX_SSPSR_BSY_BM) ;

		while ((readl(LSI_SP27XX_SSP_SSPSR_RA) & LSI_SP27XX_SSPSR_TFE_BM) !=
				LSI_SP27XX_SSPSR_TFE_BM) ;

		/* discard dummy bytes received during command and address writes */
		for( i= alen; i>=0; i--) {
			LSI_SP27XX_SSP_WaitRX();
			temp = (uint8_t)readl(LSI_SP27XX_SSP_SSPDR_RA) ; /* dummy read*/
		}

		/* read the actual data */
		for(i=0; i<read_size; i++) {
			LSI_SP27XX_SSP_WaitRX();
			temp = (uint8_t)readl(LSI_SP27XX_SSP_SSPDR_RA);
			*pDataIn++ = temp;
		}

		size -= read_size;
		addr += read_size ;
	}
	return read_size ;
}

/******************************************************************************
FUNCTION: spi_get_rdsr

DESCRIPTION:
	Read the EEPROM Status Register

INPUT:
	none

OUTPUT:
	none

RETURN:
	The contents of the EEPROM status register

COMMENTS:
	none

HISTORY:
	none
*******************************************************************************/
static uint8_t
spi_get_rdsr(void)
{

	volatile uint32_t	spsr_reg ;
	volatile uint32_t	dummy ;
	volatile uint32_t	sspsr ;

	/* wait for DSP */
	while (readl(LSI_SP27XX_SSP_SSPSR_RA)  & LSI_SP27XX_SSPSR_BSY_BM) ;

	/* disable SSP  while filling Tx FIFO */
	writel(readl(LSI_SP27XX_SSP_SSPCR1_RA) & (~(uint32_t)LSI_SP27XX_SSPCR1_SSE_BM),
		LSI_SP27XX_SSP_SSPCR1_RA) ;

	/* write the read status register command */
	writel(LSI_SP27XX_EERDSR, LSI_SP27XX_SSP_SSPDR_RA) ;
	writel(0x0, LSI_SP27XX_SSP_SSPDR_RA) ;

	/* enable the SSP */
	writel(readl(LSI_SP27XX_SSP_SSPCR1_RA) | LSI_SP27XX_SSPCR1_SSE_BM,
		LSI_SP27XX_SSP_SSPCR1_RA) ;

	/* wait while SSP is busy */
	while (readl(LSI_SP27XX_SSP_SSPSR_RA) & LSI_SP27XX_SSPSR_BSY_BM) ;

	/* read the status register from the Rx FIFO */
	sspsr = LSI_SP27XX_SSP_WaitRX();
	dummy = readl(LSI_SP27XX_SSP_SSPDR_RA) ;
	sspsr = LSI_SP27XX_SSP_WaitRX();
	spsr_reg = readl(LSI_SP27XX_SSP_SSPDR_RA) ;

	return((uint8_t) (spsr_reg & 0x000000ff)) ;
}


/******************************************************************************
FUNCTION: spi_put_wrsr

DESCRIPTION:
	Write to the EEPROM Status Register

INPUT:
	uint8_t sr_val	Value to write to the EEPROM Status Register

OUTPUT:
	none

RETURN:
	The contents of the EEPROM status register

COMMENTS:
	none

HISTORY:
	none
*******************************************************************************/
static uint8_t
spi_put_wrsr(uint8_t sr_val)
{

	volatile uint32_t	spsr_reg ;
	volatile uint32_t	dummy ;
	volatile uint32_t	sspsr ;
	uint32_t		temp ;

	/* wait for DSP */
	temp = readl(LSI_SP27XX_SSP_SSPSR_RA) ;
	while (temp  & LSI_SP27XX_SSPSR_BSY_BM) {
		temp = readl(LSI_SP27XX_SSP_SSPSR_RA) ;
	}

	/* disable SSP  while filling Tx FIFO */
	writel(readl(LSI_SP27XX_SSP_SSPCR1_RA) & (~LSI_SP27XX_SSPCR1_SSE_BM),
		LSI_SP27XX_SSP_SSPCR1_RA) ;

	/* write the read status register command */
	writel(LSI_SP27XX_EEWRSR, LSI_SP27XX_SSP_SSPDR_RA) ;
	writel(sr_val, LSI_SP27XX_SSP_SSPDR_RA) ;

	/* enable the SSP */
	writel(readl(LSI_SP27XX_SSP_SSPCR1_RA) | LSI_SP27XX_SSPCR1_SSE_BM,
		LSI_SP27XX_SSP_SSPCR1_RA) ;

	/* wait while SSP is busy */
	temp = readl(LSI_SP27XX_SSP_SSPSR_RA) ;
	while (temp  & LSI_SP27XX_SSPSR_BSY_BM) {
		temp = readl(LSI_SP27XX_SSP_SSPSR_RA) ;
	}

	/* read the status register from the Rx FIFO */
	sspsr = LSI_SP27XX_SSP_WaitRX();
	dummy = readl(LSI_SP27XX_SSP_SSPDR_RA) ;
	sspsr = LSI_SP27XX_SSP_WaitRX();
	spsr_reg = readl(LSI_SP27XX_SSP_SSPDR_RA) ;

	return((uint8_t) (spsr_reg & 0x000000FF)) ;
}

/******************************************************************************
FUNCTION: spi_write

DESCRIPTION:
	Generic EEPROM write function

INPUT:
	eeaddr			- EEPROM address
	alen			- address len (will always be 3 bytes)
	pDataIn			- Pointer to data to be written
	size			= # of bytes of the data to be written

OUTPUT:
	none

RETURN:
	# of bytes actually written

COMMENTS:
	none

HISTORY:
	none
*******************************************************************************/
static int
spi_write(
	uint8_t	*eeaddr,
	int	alen,
	uint8_t	*pDataIn,
	int	size)
{
	uint8_t		temp;
	int32_t 	i;
	int			write_size ;
	uint8_t		rdsr_reg ;
	uint32_t	addr , trans_addr, off;
	uint32_t	sr ;
	uint8_t		max_read_size = LSI_SP27XX_SSPFIFO_SIZE - 4;
	int			remaining_size  = size ;
	uint32_t	sector ;
	uint8_t		sector_addr[3];
	uint8_t		*cache_ptr ;
	//char		mem_disp[50];

	addr = ((eeaddr[0]<<16) | (eeaddr[1]<<8) | (eeaddr[2]&0xff)) ;
	trans_addr = addr;
	sector = addr & SECTOR_MASK ;
	sector_addr[0] = sector >> 16;
	sector_addr[1] = 0;
	sector_addr[2] = 0;
	addr &= ~SECTOR_MASK ;
	off = addr;

        //bsp_debug_printf("\r\n spi_write(0x%x), translated addr = 0x%x, sector 0x%x, eeprom_write_cache 0x%x", eeaddr, trans_addr, sector, eeprom_write_cache);
	/* Populate the cache prior to writing. */
	spi_read((uint8_t*)sector_addr, 3, eeprom_write_cache, SECTOR_SIZE) ;

        //sprintf(mem_disp, "0x%x %d", (unsigned int)eeprom_write_cache, 40);
        //do_mem_md(mem_disp);
	/* Limited by the SSP Tx FIFO being 32-bits long */\
	if (size > max_read_size) {
		write_size = max_read_size ;
	} else {
		write_size = size ;
	}

	/* Copy the new data over top of the EEPROM cached copy */
	cache_ptr = (uint8_t*)(eeprom_write_cache + (uint8_t)addr) ;
	while (remaining_size > 0) {
		*cache_ptr++ = *pDataIn++ ;
		remaining_size-- ;
	}
	cache_ptr = (uint8_t*)(eeprom_write_cache + (uint8_t)addr) ;
        //sprintf(mem_disp, "0x%x %d", (unsigned int)cache_ptr, 40);
        //do_mem_md(mem_disp);

	spi_write_enable(0x0, 1) ;

	/* Set the write protection and block protection settings */
	spi_put_wrsr(LSI_SP27XX_EERDSR_WEL) ;

	/* Erase the sector before writing back to it */
	spi_erase(eeaddr) ;
        //bsp_debug_printf("\r\n spi_erease(0x%x)", eeaddr);


	//addr |= sector ;
	addr = sector;
	cache_ptr = eeprom_write_cache ;
	size = SECTOR_SIZE;;
	while ((size) > 0) {

		/* Wait while EEPROM is busy */
		do {
			rdsr_reg = spi_get_rdsr() ;
		} while (rdsr_reg & LSI_SP27XX_EERDSR_WIP) ;

		/*
		** Make sure the SSP port is not busy
		*/
		while (readl(LSI_SP27XX_SSP_SSPSR_RA) & LSI_SP27XX_SSPSR_BSY_BM) ;

		/* Make the transmit fifo is empty */
		while (!(readl(LSI_SP27XX_SSP_SSPSR_RA) & LSI_SP27XX_SSPSR_TFE_BM)) ;
			
		/* Make sure the receive fifo is empty */
		sr = readl(LSI_SP27XX_SSP_SSPSR_RA) ;
		while (sr & LSI_SP27XX_SSPSR_RNE_BM) {
			temp = (uint8_t)readl(LSI_SP27XX_SSP_SSPDR_RA) ; /* dummy read */
			sr = readl(LSI_SP27XX_SSP_SSPSR_RA) ;
		}

		spi_write_enable(0x0, 1) ;

		/* disable the SSP port */
		writel(readl(LSI_SP27XX_SSP_SSPCR1_RA) & ~LSI_SP27XX_SSPCR1_SSE_BM,
			LSI_SP27XX_SSP_SSPCR1_RA) ;

		/* write EEPROM write command */
		writel(LSI_SP27XX_EEPROG, LSI_SP27XX_SSP_SSPDR_RA) ;

		/* write EEPROM data address */
		for( i=alen-1; i>=0; i--) {
			writel(((addr>>(i*8))&0xff), LSI_SP27XX_SSP_SSPDR_RA) ;
		}

		for (i=0 ; i < write_size ; i++) {
			sr = readl(LSI_SP27XX_SSP_SSPSR_RA) ;
			while ((sr & LSI_SP27XX_SSPSR_TNF_BM) 
						!= LSI_SP27XX_SSPSR_TNF_BM) {
				sr = readl(LSI_SP27XX_SSP_SSPSR_RA) ;
			}
			/* write one byte */
			writel(*cache_ptr++,LSI_SP27XX_SSP_SSPDR_RA) ;
		}

		/* enable the SSP port */
		writel(readl(LSI_SP27XX_SSP_SSPCR1_RA) | LSI_SP27XX_SSPCR1_SSE_BM,
			LSI_SP27XX_SSP_SSPCR1_RA) ;

		/* Wait until TX is not busy */
		sr = readl(LSI_SP27XX_SSP_SSPSR_RA) ;
		while (sr & LSI_SP27XX_SSPSR_BSY_BM) {
			sr = (uint8_t)readl(LSI_SP27XX_SSP_SSPSR_RA) ;
		}

		/* Do dummy reads until the RX FIFO is empty */
		LSI_SP27XX_SSP_WaitRX() ;
		while ((readl(LSI_SP27XX_SSP_SSPSR_RA) & LSI_SP27XX_SSPSR_RNE_BM) ==
				LSI_SP27XX_SSPSR_RNE_BM) {
			temp = (uint8_t) readl(LSI_SP27XX_SSP_SSPDR_RA) ;
		}	

		size -= write_size ;
		addr += write_size ;

		/* disable the SSP port again */
		writel(readl(LSI_SP27XX_SSP_SSPCR1_RA) &
			~((uint32_t)LSI_SP27XX_SSPCR1_SSE_BM),
			LSI_SP27XX_SSP_SSPCR1_RA) ;
	}

	spi_write_enable(0x0, 0) ;

	return (size) ;
}


/******************************************************************************
FUNCTION: spi_erase

DESCRIPTION:
	Generic EEPROM erase function

INPUT:
	eeaddr		- EEPROM address (sector is calucated from this)

OUTPUT:
	none

RETURN:
	# of bytes actually written

COMMENTS:
	none

HISTORY:
	none
*******************************************************************************/
static int
spi_erase(
	uint8_t	*eeaddr)
{
	uint8_t		temp;
	int32_t 	i;
	uint8_t		rdsr_reg ;
	uint32_t	addr ;


	addr = ((eeaddr[0]<<16) | (eeaddr[1]<<8) | (eeaddr[2]&0xff)) ;

	spi_write_enable(0x0, 1) ;

	/* Wait while EEPROM is busy */
	do {
		rdsr_reg = spi_get_rdsr() ;
	} while (rdsr_reg & LSI_SP27XX_EERDSR_WIP) ;

	/* disable the SSP port */
	writel(readl(LSI_SP27XX_SSP_SSPCR1_RA) & ~LSI_SP27XX_SSPCR1_SSE_BM,
			LSI_SP27XX_SSP_SSPCR1_RA) ;

        /* write EEPROM sector erase command */
        if (ssp_eeprom_id[0] != EEPROM_ID_SDB) {
            writel(LSI_SP27XX_FLASH_SE_NGVM, LSI_SP27XX_SSP_SSPDR_RA) ;
        } else {
            writel(LSI_SP27XX_FLASH_SE_SDB, LSI_SP27XX_SSP_SSPDR_RA) ;
        }

	/* write EEPROM data address */
	for( i=2; i>=0; i--) {
		writel(((addr>>(i*8))&0xff), LSI_SP27XX_SSP_SSPDR_RA) ;
	}

	/* enable the SSP port */
	writel(readl(LSI_SP27XX_SSP_SSPCR1_RA) | LSI_SP27XX_SSPCR1_SSE_BM,
			LSI_SP27XX_SSP_SSPCR1_RA) ;

	/* Do dummy reads until the RX FIFO is empty */
	LSI_SP27XX_SSP_WaitRX() ;
	while ((readl(LSI_SP27XX_SSP_SSPSR_RA) & LSI_SP27XX_SSPSR_RNE_BM) ==
			LSI_SP27XX_SSPSR_RNE_BM) {
		temp = (uint8_t) readl(LSI_SP27XX_SSP_SSPDR_RA) ;
	}	

        if (ssp_eeprom_id[0] == EEPROM_ID_SDB) {
            writel(0x0, LSI_SP27XX_SSP_SSPDR_RA) ; // Dummy write
        }

	/* The Write Enable will reset back to disabled, once the sector
 	 * erase is complete.  So we'll watch for it here.
 	 */
	do {
		rdsr_reg = spi_get_rdsr() ;
	} while (rdsr_reg & LSI_SP27XX_EERDSR_WIP) ;

	return (0) ;
}

/******************************************************************************
spi_write_enable

DESCRIPTION:
	Disables the EEPROM/Flash write operation mode

INPUT:
	none

OUTPUT:
	none

RETURN:
	none

COMMENTS:
	none

HISTORY:
	none
*******************************************************************************/
static int
spi_write_enable (unsigned dev_addr, int state)
{
	volatile uint32_t	sspsr ;
	volatile uint32_t	rdsr_reg ;
	volatile uint32_t	dummy ;

	do {
		rdsr_reg = spi_get_rdsr() ;
	} while (rdsr_reg & LSI_SP27XX_EERDSR_WIP) ;

	/* make sure thee SSP port is not busy */
	while (readl(LSI_SP27XX_SSP_SSPSR_RA) & LSI_SP27XX_SSPSR_BSY_BM) ;

	/* disable the SSP port */
	writel(readl(LSI_SP27XX_SSP_SSPCR1_RA)&
		(~((uint32_t)(LSI_SP27XX_SSPCR1_SSE_BM))),
		LSI_SP27XX_SSP_SSPCR1_RA) ;

	switch (state) {

	case 1:	/* Enable Write */
		
		/* write the "enable write" command to EEPROM */
		writel(LSI_SP27XX_EEWREN, LSI_SP27XX_SSP_SSPDR_RA) ;

		/* enable the SSP port */
		writel(readl(LSI_SP27XX_SSP_SSPCR1_RA) | 
			(uint32_t)LSI_SP27XX_SSPCR1_SSE_BM, LSI_SP27XX_SSP_SSPCR1_RA) ;

		while (readl(LSI_SP27XX_SSP_SSPSR_RA) & LSI_SP27XX_SSPSR_BSY_BM) ;

		sspsr = LSI_SP27XX_SSP_WaitRX();
		dummy = readl(LSI_SP27XX_SSP_SSPDR_RA) ;

		do {
			rdsr_reg = spi_get_rdsr() ;
		} while ((rdsr_reg & LSI_SP27XX_EERDSR_WEL) == 0x00000000) ;

		break ;

	case 0: /* Disable Write */

		/* write the "disable write" command to EEPROM */
		writel(LSI_SP27XX_EEWRDI, LSI_SP27XX_SSP_SSPDR_RA) ;

		/* enable the SSP port */
		writel(readl(LSI_SP27XX_SSP_SSPCR1_RA) | 
			(uint32_t)LSI_SP27XX_SSPCR1_SSE_BM, LSI_SP27XX_SSP_SSPCR1_RA) ;

		while (readl(LSI_SP27XX_SSP_SSPSR_RA) & LSI_SP27XX_SSPSR_BSY_BM) ;

		sspsr = LSI_SP27XX_SSP_WaitRX();
		dummy = readl(LSI_SP27XX_SSP_SSPDR_RA) ;

		do {
			rdsr_reg = spi_get_rdsr() ;
		} while ((rdsr_reg & LSI_SP27XX_EERDSR_WEL) != 0x00000000) ;

		break ;

	default:	
		break ;
	}

	return 0 ;
}

int
eeprom_read (unsigned offset, uint8_t *buffer, unsigned cnt)
{
	unsigned end = offset + cnt;
	unsigned blk_off;
	int rcode = 0;

	/* Read data until done or would cross a page boundary.
	 * We must write the address again when changing pages
	 * because the next page may be in a different device.
	 */
	while (offset < end) {
		unsigned alen, len;
		uint8_t addr[3];

		blk_off = offset & 0xFF;	/* block offset */

		addr[0] = offset >> 16;		/* block number */
		addr[1] = offset >>  8;		/* upper address octet */
		addr[2] = blk_off;			/* lower address octet */
		alen	= 3;

#if defined(CONFIG_SYS_EEPROM_BLOCK_OFFSET)
		/* When the data address spills into the chip address, the
		 * "block bits" don't always spill into the low-order
		 * chip address bits (See Microchip 24FC1025 for example).
		 * This just shifts the block bits to the proper location
		 * in the chip address.
		 */
		addr[0] <<= CONFIG_SYS_EEPROM_BLOCK_OFFSET;
#endif

		len = end - offset;
		spi_read (addr, alen, buffer, len);

		buffer += len;
		offset += len;
	}

	return rcode;
}

/*-----------------------------------------------------------------------
 *
 * for CONFIG_SYS_I2C_EEPROM_ADDR_LEN == 2 (16-bit EEPROM address) offset is
 *   0x000nxxxx for EEPROM address selectors at n, offset xxxx in EEPROM.
 *
 * for CONFIG_SYS_I2C_EEPROM_ADDR_LEN == 1 (8-bit EEPROM page address) offset is
 *   0x00000nxx for EEPROM address selectors and page number at n.
 */

int
eeprom_write (unsigned offset, uint8_t *buffer, unsigned cnt)
{
	unsigned end = offset + cnt;
	unsigned blk_off;
	int rcode = 0;

        //bsp_debug_printf("\r\n eeprom_write(0x%x, size 0x%x)", offset, cnt);
	/* Write data until done or would cross a write page boundary.
	 * We must write the address again when changing pages
	 * because the address counter only increments within a page.
	 */

	while (offset < end) {
		unsigned alen, len, readlen;

		uint8_t addr[3];

		blk_off = offset & 0xFF;	/* block offset */

		addr[0] = offset >> 16;		/* block number */
		addr[1] = offset >>  8;		/* upper address octet */
		addr[2] = blk_off;		/* lower address octet */
		alen	= 3;

#if defined(CONFIG_SYS_EEPROM_BLOCK_OFFSET)
		/* When the data address spills into the chip address, the
		 * "block bits" don't always spill into the low-order
		 * chip address bits (See Microchip 24FC1025 for example).
		 * This just shifts the block bits to the proper location
		 * in the chip address.
		 */
		addr[0] <<= CONFIG_SYS_EEPROM_BLOCK_OFFSET;
#endif

		len = end - offset;

		if (len > SECTOR_SIZE) {
			readlen = SECTOR_SIZE ;
		} else {
			readlen = len ;
		}
		spi_write (addr, alen, buffer, readlen);

		buffer += readlen;
		offset += readlen;
	}
	return rcode;
}

/******** History ********
$Log: ssp.c,v $
Revision 1.3  2013/03/08 23:10:56  srane
Fix bug in LSI code for spi write.

Revision 1.2  2012/08/15 14:32:36  srane
Add the correct version of ssp code.

Revision 1.1  2012/06/28 13:33:28  srane
SSP support


$Endlog$
*/

