/* $Id: mpcore_dist_intrc_io.c,v 1.2 2012/05/10 22:48:11 srane Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/dsp_src/src/sp27dsp/ppb/lib/mpcore_dist_intrc_io.c,v $
 *------------------------------------------------------------------
 * mpcore_dist_intrc_io.c
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
 *  mpcore_dist_intrc_io.c
 *
 *  Created on: Aug 26, 2009
 *      Author: dokim
 */

#include "libppbint.h"
#include "lsi_sp27xx_mpcore.h"
#include "mpcore_dist_intrc_io.h"
#include "lsi_sp27xx_dss.h"

void __armif_enable_irq(void)
{
	/* enable arm interrupt interface */
	REG32_SET_BITS(MPCORE_CPU_INT_IF_CONT_RA, MPCORE_CPU_INT_IF_CONT_EN_BM);
	__asm__ volatile ( "cpsie if\n"  );
}

void __armif_disable_irq(void)
{
	/* disable arm interrupt interface */
	REG32_RESET_BITS(MPCORE_CPU_INT_IF_CONT_RA, MPCORE_CPU_INT_IF_CONT_EN_BM);
	__asm__ volatile ( "cpsid if\n" );
}

int __armif_read_intid(void)
{
	int temp;

	REG32_READ(MPCORE_CPU_INT_IF_INT_ACK_RA, temp);

	return temp&MPCORE_CPU_INT_IF_INT_ACK_INT_ID_BM;
}

int __armif_clr_int(int int_id)
{
	if((int_id<0)||(int_id>MAX_NUM_INT_ID))
	{
		/* invalid int id */
		return ERROR;
	}

	REG32_WRITE(MPCORE_CPU_INT_IF_END_OF_INT_RA, int_id);
	return SUCCESS;
}

int __armif_set_pri_msk(int pri_level)
{
	if ((pri_level<0)||(pri_level>0xF)) {
		pri_level = 0xF;
	}

	REG32_WRITE(MPCORE_CPU_INT_IF_PRIORITY_MASK_RA	, pri_level<<MPCORE_CPU_INT_IF_PRIORITY_MASK_PM_BO);
	return SUCCESS;
}

void __dic_en(void)
{
	REG32_SET_BITS(MPCORE_DIST_INTRC_CONT_RA, MPCORE_DIST_INTRC_CONT_EN_BM);
}

void __dic_disable(void)
{
	REG32_RESET_BITS(MPCORE_DIST_INTRC_CONT_RA, MPCORE_DIST_INTRC_CONT_EN_BM);
}


int __dic_int_en(int int_id)
{
	int reg_offset;
	int bit_offset;
	int i;

	if(int_id==ALL)
	{
		for(i=0; i<MAX_NUM_INT_ID/32; i++)
		{
			REG32_WRITE(MPCORE_DIST_INTRC_EN_SET_RA(i), 0xFFFFFFFF);
		}
	}
	else if(int_id<MAX_NUM_INT_ID)
	{
		reg_offset = int_id>>5; /* int_id/32 */
		bit_offset = int_id&0x1F;
		REG32_SET_BITS(MPCORE_DIST_INTRC_EN_SET_RA(reg_offset), 0x1<<bit_offset);
	}
	else
	{
		/* invalid int id */
		return ERROR;
	}
	return SUCCESS;
}

int __dic_int_dis(int int_id)
{
	int reg_offset;
	int bit_offset;
	int i;

	if(int_id==ALL)
	{
		for(i=0; i<MAX_NUM_INT_ID/32; i++)
		{
			REG32_WRITE(MPCORE_DIST_INTRC_EN_SET_CLR_RA(i), 0xFFFFFFFF);
		}
	}
	else if(int_id<MAX_NUM_INT_ID)
	{
		reg_offset = int_id>>5; /* int_id/32 */
		bit_offset = int_id&0x1F;
		REG32_WRITE(MPCORE_DIST_INTRC_EN_SET_CLR_RA(reg_offset), 0x1<<bit_offset);
	}
	else
	{
		/* invalid int id */
		return ERROR;
	}
	return SUCCESS;
}

int __dic_int_set_cpu(int int_id, int cpu_id)
{
	int reg_offset;
	int bit_offset;
	int i, temp = 0;

	if((cpu_id<0)&&(cpu_id>(CPU_TARGET_ARM0|CPU_TARGET_ARM1)))
	{
		/* invalid CPU ID */
		return ERROR;
	}

	if(int_id==ALL)
	{
		temp = cpu_id;
		temp |= (temp << 8);
		temp |= (temp << 16);

		for(i=0; i<MAX_NUM_INT_ID/4; i++)
		{
			REG32_WRITE(MPCORE_DIST_INTRC_CPU_TARGET_RA(i), temp);
		}
	}
	else if(int_id<MAX_NUM_INT_ID)
	{
		reg_offset = int_id>>2; 	/* int_id/4 */
		bit_offset = (int_id&0x3)<<3; /* (int_id%4)*8 */
		REG32_SET_BITS(MPCORE_DIST_INTRC_CPU_TARGET_RA(reg_offset), (0x3&cpu_id)<<bit_offset);
	}
	else
	{
		/* invalid int id */
		return ERROR;
	}
	return SUCCESS;
}

int __dic_int_reset_cpu(int int_id, int cpu_id)
{
	int reg_offset;
	int bit_offset;

	if((cpu_id<0)&&(cpu_id>(CPU_TARGET_ARM0|CPU_TARGET_ARM1)))
	{
		/* invalid CPU ID */
		return ERROR;
	}

	else if(int_id<MAX_NUM_INT_ID)
	{
		reg_offset = int_id>>2; 	/* int_id/4 */
		bit_offset = (int_id&0x3)<<3; /* (int_id%4)*8 */
		REG32_RESET_BITS(MPCORE_DIST_INTRC_CPU_TARGET_RA(reg_offset), (0x3&cpu_id)<<bit_offset);
	}
	else
	{
		/* invalid int id */
		return ERROR;
	}
	return SUCCESS;
}

int __dic_int_set_pri(int int_id, int priority)
{
	int reg_offset;
	int bit_offset;
	int i, temp = 0;

	if((priority<0)||(priority>0xF))
	{
		/* override priority */
		priority = 0xF; /* the lowest priority */
	}

	if(int_id==ALL)
	{
		temp = (priority<<4);
		temp |= (temp << 8);
		temp |= (temp << 16);

		for(i=0; i<MAX_NUM_INT_ID/4; i++)
		{
			REG32_WRITE(MPCORE_DIST_INTRC_PRIORITY_RA(i), temp); /* fill out field */
		}
	}

	else if(int_id<MAX_NUM_INT_ID)
	{
		reg_offset = int_id>>2; 	/* int_id/4 */
		bit_offset = ((int_id&0x3)<<3)+ 8; /* (int_id%4)*8+8 */
		REG32_RESET_BITS(MPCORE_DIST_INTRC_PRIORITY_RA(reg_offset), 0xF<<bit_offset); /* clear field */
		REG32_SET_BITS(MPCORE_DIST_INTRC_PRIORITY_RA(reg_offset), priority<<bit_offset); /* fill out field */
	}
	else
	{
		/* invalid int id */
		return ERROR;
	}
	return SUCCESS;
}

int __dic_int_conf(int int_id, int detect_type, int sw_model)
{
	int reg_offset;
	int bit_offset;
	int i, temp = 0;

	if((detect_type!=LEVEL_SENSITIVE)&&(detect_type!=EDGE_SENSITIVE))
	{
		detect_type = LEVEL_SENSITIVE; /* by default */
	}

	if((sw_model!=ONE_BY_N)&&(sw_model!=N_BY_N))
	{
		sw_model = N_BY_N; /* by default */
	}

	if(int_id==ALL)
	{
		temp = (detect_type<<1)|sw_model;
		temp |= (temp << 2);
		temp |= (temp << 4);
		temp |= (temp << 8);
		temp |= (temp << 16);

		for(i=0; i<MAX_NUM_INT_ID/16; i++)
		{
			REG32_WRITE(MPCORE_DIST_INTRC_CONF_RA(i), temp); /* fill out field */
		}
	}
	else if(int_id<MAX_NUM_INT_ID)
	{
		reg_offset = int_id>>4;
		bit_offset = (int_id&0xF)<<1; /* (int_id%16)*2 */
		REG32_RESET_BITS(MPCORE_DIST_INTRC_CONF_RA(reg_offset), 0x3<<bit_offset); /* clear field */
		REG32_SET_BITS(MPCORE_DIST_INTRC_CONF_RA(reg_offset), ((detect_type<<1)|sw_model)<<bit_offset); /* fill out field */
	}
	else
	{
		/* invalid int id */
		return ERROR;
	}
	return SUCCESS;
}

int __dic_int_chk_pen(int int_id)
{
	int reg_offset;
	int bit_offset;

	if((int_id<0)||(int_id>MAX_NUM_INT_ID))
	{
		/* invalid int id */
		return ERROR;
	}
	else
	{
		reg_offset = int_id>>5; /* int_id/32 */
		bit_offset = int_id&0x1F;

		return (((*(volatile unsigned int*)MPCORE_DIST_INTRC_PENDING_CLR_RA(reg_offset))&(0x1<<bit_offset))>>bit_offset);
	}
}

int __dic_int_clr_pen(int int_id)
{
	int reg_offset;
	int bit_offset;
	int i;

	if(int_id==ALL)
	{
		for(i=0; i<MAX_NUM_INT_ID/32; i++)
		{
			REG32_WRITE(MPCORE_DIST_INTRC_PENDING_CLR_RA(i), 0x0);
		}
	}
	else if(int_id<MAX_NUM_INT_ID)
	{
		reg_offset = int_id>>5; /* int_id/32 */
		bit_offset = int_id&0x1F;
		REG32_SET_BITS(MPCORE_DIST_INTRC_PENDING_CLR_RA(reg_offset), 0x1<<bit_offset);
	}
	else
	{
		/* invalid int id */
		return ERROR;
	}
	return SUCCESS;
}

int __dic_int_chk_active(int int_id)
{
	int reg_offset;
	int bit_offset;

	if((int_id<0)||(int_id>MAX_NUM_INT_ID))
	{
		/* invalid int id */
		return ERROR;
	}
	else
	{
		reg_offset = int_id>>5; /* int_id/32 */
		bit_offset = int_id&0x1F;

		return ((*(volatile unsigned int*)MPCORE_DIST_INTRC_ACTIVE_BIT_RA(reg_offset))&(0x1<<bit_offset))>>bit_offset;
	}
}

int __dic_int_read_active(int reg_offset)
{
	int active_interrupt;
	REG32_READ(MPCORE_DIST_INTRC_ACTIVE_BIT_RA(reg_offset), active_interrupt);
	return active_interrupt;
}

void __dic_sw_intr_gen(int sw_int_id)
{
	REG32_WRITE(MPCORE_DIST_INTRC_SW_INT_RA,\
			(0x1<<MPCORE_DIST_INTRC_SW_INT_TARGET_FIL_BO)|(sw_int_id&0x1FF));
}

void __arm_intr_gen_to_dss(int dss_core_id, int intr_num)
{
	switch(dss_core_id)
	{
	case 0x0: /* DSS0 */
		REG32_WRITE((LSI_SP27XX_DBM_DSSCTRLREGS_DSS0_BASE+LSI_SP27XX_DSS_GPINT0_RO+(0x1&intr_num)*0x10), 0x1);
		break;
	case 0x1: /* DSS1 */
		REG32_WRITE((LSI_SP27XX_DBM_DSSCTRLREGS_DSS1_BASE+LSI_SP27XX_DSS_GPINT0_RO+(0x1&intr_num)*0x10), 0x1);
		break;
	case 0x2: /* DSS2 */
		REG32_WRITE((LSI_SP27XX_DBM_DSSCTRLREGS_DSS2_BASE+LSI_SP27XX_DSS_GPINT0_RO+(0x1&intr_num)*0x10), 0x1);
		break;
	case 0x3: /* DSS3 */
		REG32_WRITE((LSI_SP27XX_DBM_DSSCTRLREGS_DSS3_BASE+LSI_SP27XX_DSS_GPINT0_RO+(0x1&intr_num)*0x10), 0x1);
		break;
	default:
		break;
	}
}

/******** History ********
$Log: mpcore_dist_intrc_io.c,v $
Revision 1.2  2012/05/10 22:48:11  srane
clean up and modify files for exception handling and TDM tests.

Revision 1.1  2012/04/18 09:47:31  srane
Initial checkin


$Endlog$
*/

