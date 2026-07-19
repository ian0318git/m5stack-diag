/* $Id: libgpio.h,v 1.3 2012/08/28 18:20:18 srane Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/dsp_src/src/sp27dsp/ppb/lib/libgpio.h,v $
 *------------------------------------------------------------------
 * libppbgpio.h
 *
 * Mar 2012, Smita Rane
 *
 * Copyright (c) 2012 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

/******************************************************************************
 *                             NOTIFICATION
 *
 * Copyright (c) 2010 LSI Inc.  All Rights Reserved
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
 * libppbgpio.h - control the 8 PPB GPIOs
 *
 *  Created on: Aug 28, 2009
 *      Author: dokim
 */

#ifndef LIBPPBGPIO_H_
#define LIBPPBGPIO_H_

#include <stdint.h>

#define PPB_GPIO(i)	(0x1<<(i))

#define PPB_ALL_GPIO 	(PPB_GPIO(0)|PPB_GPIO(1)|PPB_GPIO(2)|PPB_GPIO(3)\
						|PPB_GPIO(4)|PPB_GPIO(5)|PPB_GPIO(6)|PPB_GPIO(7))

#define GPIO_IN 		0
#define GPIO_OUT		1

#define GPIO_EDGE		0
#define GPIO_LEVEL		1

#define GPIO_SINGLE		0
#define GPIO_BOTH		1

#define GPIO_FALLING	0
#define GPIO_RISING		1

#define GPIO_SW			0
#define GPIO_HW			1

/* MACROS to set PPB & DSS GPIO block */

/* direction */
#define __SP_GPIO_OUT(gpio_msk)			(REG32_SET_BITS(LSI_SP27XX_GPIO_DIR_RA, gpio_msk))
#define __SP_GPIO_IN(gpio_msk)			(REG32_RESET_BITS(LSI_SP27XX_GPIO_DIR_RA, gpio_msk))

/* interrupt sense type (edge or level) */
#define __SP_GPIO_IS_LV(gpio_msk)		(REG32_SET_BITS(LSI_SP27XX_GPIO_IS_RA, gpio_msk))
#define __SP_GPIO_IS_EG(gpio_msk)		(REG32_RESET_BITS(LSI_SP27XX_GPIO_IS_RA, gpio_msk))

/* interrupt both edges (both or single) */
#define __SP_GPIO_IBE_BOTH(gpio_msk)		(REG32_SET_BITS(LSI_SP27XX_GPIO_IBE_RA, gpio_msk))
#define __SP_GPIO_IBE_SINGLE(gpio_msk)	(REG32_RESET_BITS(LSI_SP27XX_GPIO_IBE_RA, gpio_msk))

/* interrupt both edges (both or single) */
#define __SP_GPIO_IEV_RISING(gpio_msk)	(REG32_SET_BITS(LSI_SP27XX_GPIO_IEV_RA, gpio_msk))
#define __SP_GPIO_IEV_FALLING(gpio_msk)	(REG32_RESET_BITS(LSI_SP27XX_GPIO_IEV_RA, gpio_msk))

/* interrupt mask (both or single) */
#define __SP_GPIO_IE_UNMASK(gpio_msk)	(REG32_SET_BITS(LSI_SP27XX_GPIO_IE_RA, gpio_msk))
#define __SP_GPIO_IE_MASK(gpio_msk)		(REG32_RESET_BITS(LSI_SP27XX_GPIO_IE_RA, gpio_msk))

/* interrupt clr */
#define __SP_GPIO_IC_INTCLR(gpio_msk)	(REG32_SET_BITS(LSI_SP27XX_GPIO_IC_RA, gpio_msk))

/* interrupt mode control select (hw or sw) */
#define __SP_GPIO_AFSEL_HW(gpio_msk)		(REG32_SET_BITS(LSI_SP27XX_GPIO_AFSEL_RA, gpio_msk))
#define __SP_GPIO_AFSEL_SW(gpio_msk)		(REG32_RESET_BITS(LSI_SP27XX_GPIO_AFSEL_RA, gpio_msk))

/* register read macros */
#define __SP_GPIO_DATA_READ(data)		(REG32_READ((LSI_SP27XX_GPIO_DATA_RA+0x3FC), data))
#define __SP_GPIO_DIR_READ(data)		(REG32_READ(LSI_SP27XX_GPIO_DIR_RA, data))
#define __SP_GPIO_IS_READ(data)			(REG32_READ(LSI_SP27XX_GPIO_IS_RA, data))
#define __SP_GPIO_IBE_READ(data)		(REG32_READ(LSI_SP27XX_GPIO_IBE_RA, data))
#define __SP_GPIO_IEV_READ(data)		(REG32_READ(LSI_SP27XX_GPIO_IEV_RA, data))
#define __SP_GPIO_RIS_READ(status)		(REG32_READ(LSI_SP27XX_GPIO_RIS_RA, status))
#define __SP_GPIO_MIS_READ(status)		(REG32_READ(LSI_SP27XX_GPIO_MIS_RA, status))
#define __SP_GPIO_AFSEL_READ(data)		(REG32_READ(LSI_SP27XX_GPIO_RIS_RA, data))

/* data regsiter write */
#define __SP_GPIO_DATA_WRITE(mask, data) (REG32_WRITE((LSI_SP27XX_GPIO_DATA_RA+((mask)<<0x2)), data))

/* simple GPIO high/low transition */
#define __SP_GPIO_SET_HIGH(mask)	__SP_GPIO_DATA_WRITE(mask, mask)
#define __SP_GPIO_SET_LOW(mask)		__SP_GPIO_DATA_WRITE(mask, ~mask)

void
sp_InitGPIO(void);					/* initialize GT timer_n - MUST BE called first */

uint32_t							/* ret: mask of bit(s) that caused interrupt(s) */
sp_GetGPIOInterrupt(void);			/* identify which GPIO bit caused interrupt(s) */

void
sp_WriteGPIOData(					/* write specified bit pattern to GPIOs (configures all GPIOs as outputs) */
	uint32_t pattern);				/* in: pattern to write */

/* the following three functions use the "address masking" feature of the GPIO block
 * which is very powerful but not simple to explain. If you want to use these
 * functions, please read the "General Purpose Bit Input/Output Unit" section of
 * the "StarPro SP2700 DSP Family Register Programming Guide"
 */
void
sp_SetGPIODataHigh(					/* drive specified bits *and only specified bits* high */
	uint32_t gpio_msk);				/* in: mask of bits to set */

void
sp_SetGPIODataLow(					/* drive specified bits *and only specified bits* low */
	uint32_t gpio_msk);				/* in: mask of bits to clear */

uint32_t							/* ret: value read from GPIO pins */
sp_GetGPIOData(						/* read value from GPIO pins */
	uint32_t gpio_msk);				/* in: mask of bits to read */

void
sp_SetGPIODirectionInput(			/* Configure GPIO as input */
	uint32_t gpio_msk);				/* in: mask of bits to configure */

void
sp_SetGPIODirectionOutput(			/* Configure GPIO as output */
	uint32_t gpio_msk);				/* in: mask of bits to rconfiguread */

/*----------------------------------------------------------------------------
 * interrupt related functions
*/
void
sp_SetGPIOInterrupt(				/* set specified bits to receive interrupts */
	uint32_t gpio_msk,				/* in: mask of bits to configure (must be inputs) */
	uint32_t is,					/* in: GPIO_EDGE or GPIO_LEVEL */
	uint32_t ibe,					/* in: GPIO_SINGLE or GPIO_BOTH */
	uint32_t iev);					/* in: GPIO_FALLING or GPIO_RISING */

void
sp_ClearGPIOInetrrupt(				/* clear interrupt status on specified bits */
	uint32_t gpio_msk);				/* in: mask of bits to clear */


/* GPIO as I2C */

#define DFLT_I2C_CLK_CYCLE			1

uint32_t							/* RET: SUCCESS or ERROR */
sp_GPIOasI2C_Init(					/* initialize GPIO pins for I2C protocol */
		uint32_t gpio_for_scl, 		/* in: gpio number that will be used for I2C SCL  (i.e. 0, 1, 2, .... 7) */
		uint32_t gpio_for_sda,		/* in: gpio number that will be used for I2C SDA  (i.e. 0, 1, 2, .... 7) */
		uint32_t scl_cycle);		/* in: cyle duration that determines I2C speed (by default the I2C freq = 250KHz, when scl_cycle==100,
									 * the speed is reduced to 25KHz */

inline void
sp_GPIOasI2C_start(void);			/* I2C start protocol */

inline void
sp_GPIOasI2C_stop(void);			/* I2C stop protocol */

void
sp_GPIOasI2C_byte_transmit(			/* transmitting 1 byte via GPIO as I2C port */
		uint8_t byte);				/* in: data to be transmitted */

uint8_t								/* RET: received byte */
sp_GPIOasI2C_byte_receive(			/* receiving 1 byte via GPIO as I2C port */
		void);

/* I2C 16bit random read/write functions specially designed for Molex SFP module
 * (SFP 1000Base-T RJ-45 GIGE Copper Transceiver) - SP2704 Software Development Board only */
void
dac_i2c_random_write(			/* write 16bit value into register */
		uint8_t addr,				/* in: dev address */
		uint8_t reg_num,			/* in: register number */
		uint16_t data);				/* in: 16 bit data to be written */

uint16_t							/* RET: read value */
dac_i2c_random_read(			/* read-back 16bit data from register */
		uint8_t addr,				/* in: dev address */
		uint8_t reg_num);			/* in: register number */

uint16_t sp_readdac (int);

#endif /* LIBPPBGPIO_H_ */

/******** History ********
$Log: libgpio.h,v $
Revision 1.3  2012/08/28 18:20:18  srane
Add DAC support for .93V as well (cannot use AVS).

Revision 1.2  2012/07/17 20:45:24  srane
Add GPIO I2C support.

Revision 1.1  2012/04/18 09:47:31  srane
Initial checkin


$Endlog$
*/


