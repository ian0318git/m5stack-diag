/* $Id: libppbint.c,v 1.2 2017/07/28 07:58:48 harrchan Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/dsp_src/src/oakenshield_dsp/ppb/lib/libppbint.c,v $
 *------------------------------------------------------------------
 * libppbint.c
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
 * Copyright (c) 2010 LSI Inc.
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
 * libppbint.c - control the MP Core Distributed Interrupt Controller
 *
 *  Created on: Aug 26, 2009
 *      Author: dokim
 */

#include "libgeneric.h"
#include "libppbint.h"
#include "lsi_sp27xx_armctl.h"
#include "mpcore_dist_intrc_io.h"
#include <stdlib.h>

#define ARM_CTL_MASK_REG_RO(n)						(4*(n))
#define ARM_CTL_MASK_REG_RM							0xFFFFFFFF

#define ARM_CTL_RAW_STATUS_REG_RO(n)				(0x10+(4*(n)))
#define ARM_CTL_RAW_STATUS_REG_RM					0xFFFFFFFF

#define ARM_CTL_STATUS_REG_RO(n)					(0x30+(4*(n)))
#define ARM_CTL_STATUS_REG_RM						0xFFFFFFFF

#define ARM_CTL_MASK_REG_RA(n)						LSI_SP27XX_ARMCTL_REG(ARM_CTL_MASK_REG_RO(n))
#define ARM_CTL_RAW_STATUS_REG_RA(n)				LSI_SP27XX_ARMCTL_REG(ARM_CTL_RAW_STATUS_REG_RO(n))
#define ARM_CTL_STATUS_REG_RA(n)					LSI_SP27XX_ARMCTL_REG(ARM_CTL_STATUS_REG_RO(n))

ISR_FNCPTR _intr_isr[2][160];

void sp_InitInterruptController(void)
{
	uint32_t i, j;

	/* disable distributed interrupt controller */
	__armif_disable_irq();
	__dic_disable();

	/* clear all pending interrupt */
	__dic_int_clr_pen(ALL);

	for(i=0; i<(MAX_NUM_INT_ID/32); i++)
	{
		for(j=0; j<32; j++)
		{
			if(((__dic_int_read_active(i)>>j)&0x1)==0x1) /* if that int_id is active */
			{
				__armif_clr_int((i<<5)+j); /* writing the int_id into end of interrupt register */
			}
		}
	}

	/* enable distributed interrupt controller */
	__dic_en();
	__armif_enable_irq();
}

uint32_t sp_SetInterrupt(uint32_t int_id, uint32_t detect_type, uint32_t priority, ISR_FNCPTR isr)
{
	/* checking input arguments */

	if(int_id>MAX_NUM_INT_ID)
	{
		return ERROR;
	}

	/* checking detect_type */

	if((detect_type!=EDGE_SENSITIVE)&&(detect_type!=LEVEL_SENSITIVE))
	{
		return ERROR;
	}

	if(priority>0xF) /* 0xF is the lowest */
	{
		priority = 0xF;
	}

	if(isr==NULL)
	{
		/* Null ISR */
		return ERROR;
	}

	/* 1. first disable the interrupt */

	/* dic */
	if(__dic_int_dis(int_id)!=SUCCESS)
	{
		/* fail to disable */
		return ERROR;
	}

	if(HW_INT_ID_TO_NUM((int32_t)int_id)>-1) /* only if it's ex HW interrupt */
	{
		/* arm_control */
		REG32_RESET_BITS(ARM_CTL_MASK_REG_RA(HW_INT_ID_TO_NUM(int_id)>>5), 0x1<<(HW_INT_ID_TO_NUM(int_id)&0x1F));
	}

	/* 2. check if that interrupt is already pending or active, if it is, then clear it */
	/* dic and arm cpu if */
	if(__dic_int_chk_pen(int_id)==0x1)
	{
		if(__armif_clr_int(int_id)!=SUCCESS)
		{
			/* fail to clear the interrupt */
			return ERROR;
		}
	}

	/* temporal, will be wrong when handling error interrupts */
	if(HW_INT_ID_TO_NUM((int32_t)int_id)>-1) /* only if it's ex HW interrupt */
	{
		/* arm_control */
		REG32_SET_BITS(ARM_CTL_RAW_STATUS_REG_RA(HW_INT_ID_TO_NUM(int_id)>>5), 0x1<<(HW_INT_ID_TO_NUM(int_id)&0x1F));
	}

	if(__dic_int_set_pri(int_id, priority)!=SUCCESS)
	{
		/* fail to set the priority */
		return ERROR;
	}

	if(__dic_int_set_cpu(int_id, 1<<(sp_readMPcpuid()))!=SUCCESS)
	{
		/* fail to set the targetted CPU */
		return ERROR;
	}

	if(__dic_int_conf(int_id, detect_type, ONE_BY_N)!=SUCCESS)
	{
		/* fail to set the priority */
		return ERROR;
	}

	/* connect ISR */
	_intr_isr[sp_readMPcpuid()][int_id] = isr;

	/* 3. Enable the interrupt */
	if(__dic_int_en(int_id)!=SUCCESS)
	{
		/* fail to disable */
		return ERROR;
	}
	if(HW_INT_ID_TO_NUM((int32_t)int_id)>-1) /* only if it's ex HW interrupt */
	{
		/* arm_control */
		REG32_SET_BITS(ARM_CTL_MASK_REG_RA(HW_INT_ID_TO_NUM(int_id)>>5), 0x1<<(HW_INT_ID_TO_NUM(int_id)&0x1F));
	}

	return SUCCESS;
}

uint32_t sp_ResetInterrupt(uint32_t int_id)
{
	/* checking input arguments */

	if(int_id>MAX_NUM_INT_ID)
	{
		return ERROR;
	}

	/* temporal, will be wrong when handling error interrupts */
	if(HW_INT_ID_TO_NUM((int32_t)int_id)>-1) /* only if it's ex HW interrupt */
	{
		/* arm_control */
		/* mask interrupt in arm_control block*/
		REG32_RESET_BITS(ARM_CTL_MASK_REG_RA(HW_INT_ID_TO_NUM(int_id)>>5), 0x1<<(HW_INT_ID_TO_NUM(int_id)&0x1F));
	}

	/* check if that interrupt is already pending or active, if it is, then clear it */
	/* dic and arm cpu if */
	if (__dic_int_chk_pen(int_id)==0x1)
	{
		if(__armif_clr_int(int_id)!=SUCCESS)
		{
			/* fail to clear the interrupt */
			return ERROR;
		}
	}

	/* temporal, will be wrong when handling error interrupts */
	if (HW_INT_ID_TO_NUM((int32_t)int_id)>-1)		/* only if it's ex HW interrupt */
	{
		/* arm_control */
		REG32_SET_BITS(ARM_CTL_RAW_STATUS_REG_RA(HW_INT_ID_TO_NUM(int_id)>>5), 0x1<<(HW_INT_ID_TO_NUM(int_id)&0x1F));
	}

	if (__dic_int_reset_cpu(int_id, 1<<sp_readMPcpuid())!=SUCCESS)
	{
		/* fail to set the targeted CPU */
		return ERROR;
	}

	/* disconnect ISR */
	_intr_isr[sp_readMPcpuid()][int_id] = NULL;

	/* dic */
	if (__dic_int_dis(int_id)!=SUCCESS)
	{
		/* fail to disable */
		return ERROR;
	}

	return SUCCESS;
}

uint32_t sp_ClearInterrupt(uint32_t int_id)
{
	if(int_id>MAX_NUM_INT_ID)
	{
		return ERROR;
	}

	/* clear interrupt externally */
	/* temporal, will be wrong when handling error interrupts */
	/* arm_control */

	if(HW_INT_ID_TO_NUM((int32_t)int_id)>-1) /* only if it's ex HW interrupt */
	{
		REG32_WRITE(ARM_CTL_RAW_STATUS_REG_RA(HW_INT_ID_TO_NUM(int_id)>>5), 0x1<<(HW_INT_ID_TO_NUM(int_id)&0x1F));
	}

	if(__armif_clr_int(int_id)!=SUCCESS)
	{
		return ERROR;
	}

	return SUCCESS;
}


void sp_GenSWInterrupt(uint32_t sw_int_id)
{
	__dic_sw_intr_gen(sw_int_id);
}

void sp_InterruptDSS(uint32_t dss_core_id, uint32_t intr_num)
{
	__arm_intr_gen_to_dss(dss_core_id, intr_num);
}

/******** History ********
$Log: libppbint.c,v $
Revision 1.2  2017/07/28 07:58:48  harrchan
Collapse Oakenshield-branch to Main Trunk.

Revision 1.1.2.1  2017/06/29 08:14:35  harrchan
Initial commit code for Oakenshield

Revision 1.2  2012/05/10 22:48:11  srane
clean up and modify files for exception handling and TDM tests.

Revision 1.1  2012/04/18 09:47:31  srane
Initial checkin


$Endlog$
*/

