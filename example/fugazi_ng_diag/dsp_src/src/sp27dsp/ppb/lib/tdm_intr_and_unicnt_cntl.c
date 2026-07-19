/* $Id: tdm_intr_and_unicnt_cntl.c,v 1.2 2012/05/10 22:57:02 srane Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/dsp_src/src/sp27dsp/ppb/lib/tdm_intr_and_unicnt_cntl.c,v $
 *------------------------------------------------------------------
 * tdm_intr_and_unicnt_cntl.c
 *
 * Mar 2012, Smita Rane
 *
 * Copyright (c) 2012 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

/*
 * tdm_intr_cntl.c
 *
 *  Created on: Oct 5, 2009
 *      Author: dokim
 */
#include "lsi_sp27xx_tdm.h"
#include "tdm_intr_and_unicnt_cntl.h"

#define MAX_NUM_PORT	6

#define TDM_INTR_SIU_IN		(1<<0)
#define TDM_INTR_SIU_OUT	(1<<1)

#define TDM_INTR_SWTU_SRC	(1<<2)
#define TDM_INTR_SWTU_DST	(1<<3)

#define TDM_INTR_PARITY		(1<<4)

#define TDM_INTR_TO_DSS0	(0)
#define TDM_INTR_TO_DSS1	(1)
#define TDM_INTR_TO_DSS2	(2)
#define TDM_INTR_TO_DSS3	(3)
#define TDM_INTR_TO_ARM		(4)

#define SUCCESS 0

/* controlling general TDM interrupts */

/* enable <intr_type> interrupt to <core> processor */
int __tdm_intr_enable(int port, int core, int intr_type)
{
	if (port > MAX_NUM_PORT-1)
	{
		return ERROR;
	}

	if(core>TDM_INTR_TO_ARM)
	{
		return ERROR;
	}

	if(intr_type>TDM_INTR_PARITY)
	{
		return ERROR;
	}

	if(core<4) /* if that interrupt is going to one of DSS cores */
	{
		REG32_SET_BITS(LSI_SP27XX_TDM_DINTMSK_RA(core), (intr_type&0xf)<<(port*0x4));

		if((intr_type&0x10)==0x10) /* intr_type includes "TDM_INTR_PARITY" */
		{
			REG32_SET_BITS(LSI_SP27XX_TDM_DINTPMSK_RA(core), 0x1<<port);
		}
	}
	else /* if that interrupt is going to PPB modules */
	{
		REG32_SET_BITS(LSI_SP27XX_TDM_PINTMSK_RA, (intr_type&0xf)<<(port*0x4));

		if((intr_type&0x10)==0x10) /* intr_type includes "TDM_INTR_PARITY" */
		{
			REG32_SET_BITS(LSI_SP27XX_TDM_PINTPMSK_RA, 0x1<<port);
		}
	}
	return SUCCESS;
}

/* disable <intr_type> interrupt to <core> processor */
int __tdm_intr_disable(int port, int core, int intr_type)
{
	if (port > MAX_NUM_PORT-1)
	{
		return ERROR;
	}

	if(core>TDM_INTR_TO_ARM)
	{
		return ERROR;
	}

	if(intr_type>TDM_INTR_PARITY)
	{
		return ERROR;
	}

	if(core<4) /* if that interrupt is going to one of DSS cores */
	{
		REG32_RESET_BITS(LSI_SP27XX_TDM_DINTMSK_RA(core), (intr_type&0xf)<<(port*0x4));

		if((intr_type&0x10)==0x10) /* intr_type includes "TDM_INTR_PARITY" */
		{
			REG32_RESET_BITS(LSI_SP27XX_TDM_DINTPMSK_RA(core), 0x1<<port);
		}
	}
	else /* if that interrupt is going to PPB modules */
	{
		REG32_RESET_BITS(LSI_SP27XX_TDM_PINTMSK_RA, (intr_type&0xf)<<(port*0x4));

		if((intr_type&0x10)==0x10) /* intr_type includes "TDM_INTR_PARITY" */
		{
			REG32_RESET_BITS(LSI_SP27XX_TDM_PINTPMSK_RA, 0x1<<port);
		}
	}
	return SUCCESS;
}

/* clearing interrupt */
int __tdm_intr_clr(int port, int core, int intr_type)
{
	if (port > MAX_NUM_PORT-1)
	{
		return ERROR;
	}

	if(core>TDM_INTR_TO_ARM)
	{
		return ERROR;
	}

	if(intr_type>TDM_INTR_PARITY)
	{
		return ERROR;
	}

	if(core<4) /* if that interrupt is going to one of DSS cores */
	{
		REG32_RESET_BITS(LSI_SP27XX_TDM_DINTMSTAT_RA(core), (intr_type&0xf)<<(port*0x4));
		REG32_RESET_BITS(LSI_SP27XX_TDM_DINTSTAT_RA(core), (intr_type&0xf)<<(port*0x4));

		if((intr_type&0x10)==0x10) /* intr_type includes "TDM_INTR_PARITY" */
		{
			REG32_RESET_BITS(LSI_SP27XX_TDM_DINTPMSTAT_RA(core), 0x1<<port);
			REG32_RESET_BITS(LSI_SP27XX_TDM_DINTPSTAT_RA(core), 0x1<<port);
		}
	}
	else /* if that interrupt is going to PPB modules */
	{
		REG32_RESET_BITS(LSI_SP27XX_TDM_PINTMSTAT_RA, (intr_type&0xf)<<(port*0x4));
		REG32_RESET_BITS(LSI_SP27XX_TDM_PINTSTAT_RA, (intr_type&0xf)<<(port*0x4));

		if((intr_type&0x10)==0x10) /* intr_type includes "TDM_INTR_PARITY" */
		{
			REG32_RESET_BITS(LSI_SP27XX_TDM_PINTPMSTAT_RA, 0x1<<port);
			REG32_RESET_BITS(LSI_SP27XX_TDM_PINTPSTAT_RA, 0x1<<port);
		}
	}

	return SUCCESS;
}

/* reset unicnt */
void __unicnt_reset(int unicnt)
{
	REG32_WRITE(LSI_SP27XX_TDM_UCNT_RA(unicnt), 0);
}

/* set unicnt's limit */
void __unicnt_conf(int unicnt, int clk_src, int limit)
{
	REG32_WRITE(LSI_SP27XX_TDM_ULIM_RA(unicnt), limit);
	REG32_RESET_BITS(LSI_SP27XX_TDM_ULIM_RA(unicnt), LSI_SP27XX_TDM_ULIM_ULIMSEL_BM);
	REG32_SET_BITS(LSI_SP27XX_TDM_ULIM_RA(unicnt), clk_src&LSI_SP27XX_TDM_ULIM_ULIMSEL_BM);
}

/* enable unicnt */
void __unicnt_en(int unicnt)
{
	REG32_SET_BITS(LSI_SP27XX_TDM_ULIM_RA(unicnt), LSI_SP27XX_TDM_ULIM_ULIMEN_BM);
}

/* disable unicnt */
void __unicnt_dis(int unicnt)
{
	REG32_RESET_BITS(LSI_SP27XX_TDM_ULIM_RA(unicnt), LSI_SP27XX_TDM_ULIM_ULIMEN_BM);
}
/******** History ********
$Log: tdm_intr_and_unicnt_cntl.c,v $
Revision 1.2  2012/05/10 22:57:02  srane
Add TDM support.

Revision 1.1  2012/04/18 09:47:32  srane
Initial checkin


$Endlog$
*/

