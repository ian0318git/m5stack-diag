/* $Id: libppbgpio.c,v 1.2 2017/07/28 07:58:48 harrchan Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/dsp_src/src/oakenshield_dsp/ppb/lib/libppbgpio.c,v $
 *------------------------------------------------------------------
 * libppbgpio.c
 *
 * Mar 2012, Smita Rane
 *
 * Copyright (c) 2012-2017 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */
/******************************************************************************
 *                             NOTIFICATION
 *
# Copyright (c) 2010 LSI Inc.
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
 * libppbgpio.c - control the 8 PPB GPIOs or the 2 DSS GPIOs
 *
 *  Created on: Aug 28, 2009
 *      Author: dokim
 */
#include "lsi_sp27xx_reg.h"
#include "libgpio.h"

/* interrupt related functions */

void sp_SetGPIOInterrupt(uint32_t gpio_msk, uint32_t is, uint32_t ibe, uint32_t iev)
{
	__SP_GPIO_IC_INTCLR(gpio_msk);
	__SP_GPIO_IN(gpio_msk);

	if (is == GPIO_EDGE) {
		__SP_GPIO_IS_EG(gpio_msk);
	} else {
		__SP_GPIO_IS_LV(gpio_msk);
	}

	if (ibe == GPIO_SINGLE) {
		__SP_GPIO_IBE_SINGLE(gpio_msk);
	} else {
		__SP_GPIO_IBE_BOTH(gpio_msk);
	}

	if (iev == GPIO_FALLING) {
		__SP_GPIO_IEV_FALLING(gpio_msk);
	} else {
		__SP_GPIO_IEV_RISING(gpio_msk);
	}

	__SP_GPIO_IE_UNMASK(gpio_msk);

}

void sp_ClearGPIOInetrrupt(uint32_t gpio_msk)
{
	__SP_GPIO_IC_INTCLR(gpio_msk);
}

uint32_t sp_GetGPIOInterrupt(void)
{
	uint32_t ris_status, mis_status;

	__SP_GPIO_RIS_READ(ris_status);
	__SP_GPIO_MIS_READ(mis_status);

	return(ris_status&mis_status);
}

void sp_InitGPIO(void)
{
	/* disable interrupt */
	__SP_GPIO_IE_MASK(PPB_ALL_GPIO);

	/* all gpio interupt clear */
	__SP_GPIO_IC_INTCLR(PPB_ALL_GPIO);

	/* make all gpio config registers' values back to their defaults */
	__SP_GPIO_IN(PPB_ALL_GPIO);
	__SP_GPIO_IS_EG(PPB_ALL_GPIO);
	__SP_GPIO_IBE_SINGLE(PPB_ALL_GPIO);
	__SP_GPIO_IEV_FALLING(PPB_ALL_GPIO);
	__SP_GPIO_AFSEL_SW(PPB_ALL_GPIO);
}

void sp_WriteGPIOData(uint32_t pattern)
{
	__SP_GPIO_OUT(PPB_ALL_GPIO);
	__SP_GPIO_DATA_WRITE(PPB_ALL_GPIO, pattern);
}

void sp_SetGPIODataHigh(uint32_t gpio_msk)
{
	__SP_GPIO_OUT(gpio_msk);
	__SP_GPIO_SET_HIGH(gpio_msk);
}

void sp_SetGPIODataLow(uint32_t gpio_msk)
{
	__SP_GPIO_OUT(gpio_msk);
	__SP_GPIO_SET_LOW(gpio_msk);
}

uint32_t sp_GetGPIOData(uint32_t gpio_msk)
{
	uint32_t read = 0;

	__SP_GPIO_IN(gpio_msk);
	__SP_GPIO_DATA_READ(read);

	return(read&gpio_msk);
}

void
sp_SetGPIODirectionInput(uint32_t gpio_msk)
{
	__SP_GPIO_IN(gpio_msk);
}

void
sp_SetGPIODirectionOutput(uint32_t gpio_msk)
{
	__SP_GPIO_OUT(gpio_msk);
}

/******** History ********
$Log: libppbgpio.c,v $
Revision 1.2  2017/07/28 07:58:48  harrchan
Collapse Oakenshield-branch to Main Trunk.

Revision 1.1.2.1  2017/06/29 08:14:35  harrchan
Initial commit code for Oakenshield

Revision 1.2  2012/07/17 20:34:33  srane
cleanup

Revision 1.1  2012/05/31 06:37:06  srane
Initial checkin.


$Endlog$
*/

