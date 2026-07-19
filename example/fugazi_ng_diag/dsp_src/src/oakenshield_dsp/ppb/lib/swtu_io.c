/* $Id: swtu_io.c,v 1.2 2017/07/28 07:58:49 harrchan Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/dsp_src/src/oakenshield_dsp/ppb/lib/swtu_io.c,v $
 *------------------------------------------------------------------
 * swtu_io.c
 *
 * Mar 2012, Smita Rane
 *
 * Copyright (c) 2012-2017 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

/*
 * swtu_io.c
 *
 *  Created on: Sep 18, 2009
 *      Author: dokim
 */
#include "lsi_sp27xx_reg.h"
#include "swtu_io.h"

#define SUCCESS 0

#define MAX_NUM_PORT				6
#define MAX_CH						256

/* reset specified SWTU */
int __swtu_reset(int port)
{
	int ch_i;

	if (port > MAX_NUM_PORT-1)
	{
		return ERROR;
	}

	for(ch_i=0; ch_i<256; ch_i++)
	{
		/* src */
		REG32_WRITE(LSI_SP27XX_TDM_SADD_RA(port, ch_i), 0);
		REG32_WRITE(LSI_SP27XX_TDM_SBAS_RA(port, ch_i), 0);
		REG32_WRITE(LSI_SP27XX_TDM_SCNT_RA(port, ch_i), 0);
		REG32_WRITE(LSI_SP27XX_TDM_SLIM_RA(port, ch_i), 0);

		/* dst */
		REG32_WRITE(LSI_SP27XX_TDM_DADD_RA(port, ch_i), 0);
		REG32_WRITE(LSI_SP27XX_TDM_DBAS_RA(port, ch_i), 0);
		REG32_WRITE(LSI_SP27XX_TDM_DCNT_RA(port, ch_i), 0);
		REG32_WRITE(LSI_SP27XX_TDM_DLIM_RA(port, ch_i), 0);
	}

	/* reset registers */
	REG32_WRITE(LSI_SP27XX_TDM_SSTAT_RA(port), 0xF);
	REG32_WRITE(LSI_SP27XX_TDM_SCTL_RA(port), 0x1);
	REG32_WRITE(LSI_SP27XX_TDM_DCTL_RA(port), 0x1);
	REG32_WRITE(LSI_SP27XX_TDM_SINTC_RA(port), 0x0);
	REG32_WRITE(LSI_SP27XX_TDM_DINTC_RA(port), 0x0);
	REG32_WRITE(LSI_SP27XX_TDM_SOCE_RA(port), 0x0);
	REG32_WRITE(LSI_SP27XX_TDM_DOCE_RA(port), 0x0);
	REG32_WRITE(LSI_SP27XX_TDM_SRI_RA(port), 0x0);
	REG32_WRITE(LSI_SP27XX_TDM_DRI_RA(port), 0x0);

	return SUCCESS;
}

/* halt specified SWTU */
int __swtu_halt(int port)
{
	if (port > MAX_NUM_PORT-1)
	{
		return ERROR;
	}

	REG32_WRITE(LSI_SP27XX_TDM_SCTL_RA(port), LSI_SP27XX_TDM_SCTL_SHALT_BM);
	REG32_WRITE(LSI_SP27XX_TDM_DCTL_RA(port), LSI_SP27XX_TDM_DCTL_DHALT_BM);

	return SUCCESS;
}

int __swtu_conf(int port, int dimension, int stride, int reindex, int backward_compat, int autoload\
				, int type_intr, int num_frm_intr /* this is valid only if type_int == SIGCON_LCOL2_LROW_INTC */ )
{
	if (port > MAX_NUM_PORT-1)
	{
		return ERROR;
	}

	REG32_WRITE(LSI_SP27XX_TDM_SCTL_RA(port), ((backward_compat<<LSI_SP27XX_TDM_SCTL_SBACKMD_BO)\
												|(dimension << LSI_SP27XX_TDM_SCTL_SPOSTMOD_BO)\
												|(type_intr << LSI_SP27XX_TDM_SCTL_SSIGCON_BO)\
												|(autoload<<LSI_SP27XX_TDM_SCTL_SAUTOLOAD_BO)));


	REG32_WRITE(LSI_SP27XX_TDM_DCTL_RA(port), ((backward_compat<<LSI_SP27XX_TDM_DCTL_DBACKMD_BO)\
												|(dimension << LSI_SP27XX_TDM_DCTL_DPOSTMOD_BO)\
												|(type_intr << LSI_SP27XX_TDM_DCTL_DSIGCON_BO)\
												|(autoload<<LSI_SP27XX_TDM_DCTL_DAUTOLOAD_BO)));

	if(type_intr==SIGCON_LCOL2_LROW_INTC)
	{
		REG32_WRITE(LSI_SP27XX_TDM_SINTC_RA(port), num_frm_intr);
		REG32_WRITE(LSI_SP27XX_TDM_DINTC_RA(port), num_frm_intr);
	}

	if((dimension == POSTMOD_2DIMENSIONAL)&&(backward_compat==1))
	{
		REG32_WRITE(LSI_SP27XX_TDM_SSTR_RA(port), stride);
		REG32_WRITE(LSI_SP27XX_TDM_DSTR_RA(port), stride);

		REG32_WRITE(LSI_SP27XX_TDM_SRI_RA(port), reindex);
		REG32_WRITE(LSI_SP27XX_TDM_DRI_RA(port), reindex);
	}
	return SUCCESS;
}

/* setting up registers for ch buffers */
int __swtu_set_ch_buf_2D(int port, int ch_num, int num_frame, int* src, int* dst)
{
	if (port > MAX_NUM_PORT-1)
	{
		return ERROR;
	}

	/* src */
	REG32_WRITE(LSI_SP27XX_TDM_SADD_RA(port, ch_num), (int)src);
	REG32_WRITE(LSI_SP27XX_TDM_SBAS_RA(port, ch_num), (int)src);
	REG32_WRITE(LSI_SP27XX_TDM_SCNT_RA(port, ch_num), 0);
	REG32_WRITE(LSI_SP27XX_TDM_SLIM_RA(port, ch_num), (num_frame&0x1FFFFF)-1);

	REG32_WRITE(LSI_SP27XX_TDM_DADD_RA(port, ch_num), (int)dst);
	REG32_WRITE(LSI_SP27XX_TDM_DBAS_RA(port, ch_num), (int)dst);
	REG32_WRITE(LSI_SP27XX_TDM_DCNT_RA(port, ch_num), 0);
	REG32_WRITE(LSI_SP27XX_TDM_DLIM_RA(port, ch_num), (num_frame&0x1FFFFF)-1);

	return SUCCESS;
}

/* setting up registers for ch buffers */
int __swtu_set_ch_buf_1D_2D_Backward(int port, int ncolumn, int nrow, int* src, int* dst)
{
	if (port > MAX_NUM_PORT-1)
	{
		return ERROR;
	}

	/* in 2D backward compatible mode, only first set of buffer registers will be programmed
	 * and used */

	/* src */
	REG32_WRITE(LSI_SP27XX_TDM_SADD_RA(port, 0), (int)src);
	REG32_WRITE(LSI_SP27XX_TDM_SBAS_RA(port, 0), (int)src);
	REG32_WRITE(LSI_SP27XX_TDM_SCNT_RA(port, 0), 0);
	REG32_WRITE(LSI_SP27XX_TDM_SLIM_RA(port, 0), (ncolumn-1)|((nrow-1)<<LSI_SP27XX_TDM_SLIM_SLASTROW_BO));

	REG32_WRITE(LSI_SP27XX_TDM_DADD_RA(port, 0), (int)dst);
	REG32_WRITE(LSI_SP27XX_TDM_DBAS_RA(port, 0), (int)dst);
	REG32_WRITE(LSI_SP27XX_TDM_DCNT_RA(port, 0), 0);
	REG32_WRITE(LSI_SP27XX_TDM_DLIM_RA(port, 0), (ncolumn-1)|((nrow-1)<<LSI_SP27XX_TDM_DLIM_DLASTROW_BO));

	return SUCCESS;
}

/* connect universal counter to the channel */
int __swtu_conn_unicnt_to_ch(int port, int ch_num, int uni_cnt)
{
	if (port > MAX_NUM_PORT-1)
	{
		return ERROR;
	}

	/* src */

	/* set SUCEN bit */
	REG32_SET_BITS(LSI_SP27XX_TDM_SLIM_RA(port, ch_num), LSI_SP27XX_TDM_SLIM_SUCEN_BM);

	/* put universal counter's number */
	REG32_RESET_BITS(LSI_SP27XX_TDM_SLIM_RA(port, ch_num), LSI_SP27XX_TDM_SLIM_SUCSEL_BM);
	REG32_SET_BITS(LSI_SP27XX_TDM_SLIM_RA(port, ch_num), uni_cnt<<LSI_SP27XX_TDM_SLIM_SUCSEL_BO);

	/* dst */

	/* set DUCEN bit */
	REG32_SET_BITS(LSI_SP27XX_TDM_DLIM_RA(port, ch_num), LSI_SP27XX_TDM_DLIM_DUCEN_BM);

	/* put universal counter's number */
	REG32_RESET_BITS(LSI_SP27XX_TDM_DLIM_RA(port, ch_num), LSI_SP27XX_TDM_DLIM_DUCSEL_BM);
	REG32_SET_BITS(LSI_SP27XX_TDM_DLIM_RA(port, ch_num), uni_cnt<<LSI_SP27XX_TDM_DLIM_DUCSEL_BO);

	return SUCCESS;
}

/* connect universal counter to the channel */
int __swtu_disconn_unicnt_to_ch(int port, int ch_num)
{
	if (port > MAX_NUM_PORT-1)
	{
		return ERROR;
	}

	/* src */
	/* reset SUCEN bit */
	REG32_RESET_BITS(LSI_SP27XX_TDM_SLIM_RA(port, ch_num), LSI_SP27XX_TDM_SLIM_SUCEN_BM);

	/* dst */
	/* reset DUCEN bit */
	REG32_RESET_BITS(LSI_SP27XX_TDM_DLIM_RA(port, ch_num), LSI_SP27XX_TDM_DLIM_DUCEN_BM);

	return SUCCESS;
}

/* start SWTU */
int __swtu_start(int port)
{
	if (port > MAX_NUM_PORT-1)
	{
		return ERROR;
	}

	/* start src */
	REG32_SET_BITS(LSI_SP27XX_TDM_SCTL_RA(port), LSI_SP27XX_TDM_SCTL_SRUN_BM);

	/* start dst */
	REG32_SET_BITS(LSI_SP27XX_TDM_DCTL_RA(port), LSI_SP27XX_TDM_DCTL_DRUN_BM);

	return SUCCESS;
}

/* start SWTU */
int __swtu_stop(int port)
{
	if (port > MAX_NUM_PORT-1)
	{
		return ERROR;
	}

	/* start src */
	REG32_SET_BITS(LSI_SP27XX_TDM_SCTL_RA(port), LSI_SP27XX_TDM_SCTL_SHALT_BM);

	/* start dst */
	REG32_SET_BITS(LSI_SP27XX_TDM_DCTL_RA(port), LSI_SP27XX_TDM_DCTL_DHALT_BM);

	return SUCCESS;
}

SWTU_STATUS __swtu_chk_status(int port)
{
	int temp;
	SWTU_STATUS status;

	if (port > MAX_NUM_PORT-1)
	{
		status.reg = ERROR;
		return status;
	}

	REG32_READ(LSI_SP27XX_TDM_SSTAT_RA(port), temp);
	status.reg = temp;

	return status;
}

/* clear status */
int __swtu_clr_status(int port)
{
	if (port > MAX_NUM_PORT-1)
	{
		return ERROR;
	}

	REG32_WRITE(LSI_SP27XX_TDM_SSTAT_RA(port), LSI_SP27XX_TDM_SSTAT_RM);

	return SUCCESS;
}

/* setting up registers for ch buffers -- backward compat*/
/* start address of a buf for each ch will be calculated based on row ,col and the base address of first buf */
int __swtu_set_old(int port, int num_row, int num_col, int* src, int* dst)
{
	/* will be added  */
	return SUCCESS;
}

/******** History ********
$Log: swtu_io.c,v $
Revision 1.2  2017/07/28 07:58:49  harrchan
Collapse Oakenshield-branch to Main Trunk.

Revision 1.1.2.1  2017/06/29 08:14:37  harrchan
Initial commit code for Oakenshield

Revision 1.2  2012/05/10 22:57:02  srane
Add TDM support.

Revision 1.1  2012/04/18 09:47:32  srane
Initial checkin


$Endlog$
*/

