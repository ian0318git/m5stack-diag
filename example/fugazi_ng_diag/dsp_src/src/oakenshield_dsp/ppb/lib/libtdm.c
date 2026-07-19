/* $Id: libtdm.c,v 1.2 2017/07/28 07:58:49 harrchan Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/dsp_src/src/oakenshield_dsp/ppb/lib/libtdm.c,v $
 *------------------------------------------------------------------
 * libtdm.c
 *
 * Mar 2012, Smita Rane
 *
 * Copyright (c) 2012-2017 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

/*
 * libppbtdm.c
 *
 *  Created on: Sep 18, 2009
 *      Author: dokim
 */
#include <stdint.h>
#include <stdlib.h>

#include "lsi_sp27xx_reg.h"

#include "siu_io.h"
#include "swtu_io.h"
#include "tdm_intr_and_unicnt_cntl.h"
#include "libtdm.h"
#include "libgeneric.h"
#include "libuart.h"

#define SUCCESS 0
#define FAIL -1

/* this macro will be used to fill 32bit register with 1 in certain bit range
 * i.e.  the result from SET_BITS_RANGE(3, 5) will be 0b00000000000000000000000000111000 = 0x38
 */
#define SET_BITS_RANGE(from_ofst, to_ofst) \
	((~((uint32_t)(0x1<<from_ofst)-0x1))&(((uint32_t)(0x1<<to_ofst)-0x1)|(uint32_t)(0x1<<to_ofst)))

unsigned int code_to_nbits[16] =
{
		8, /* bits */
		16, /* bits */
		4, /* bits */
		12, /* bits */
		24, /* bits */
		32, /* bits */
		20, /* bits */
		28, /* bits */
		40, /* bits */
		48, /* bits */
		36, /* bits */
		44, /* bits */
		56, /* bits */
		64, /* bits */
		52, /* bits */
		60 /* bits */
};

/* TDM interrupt init */
int32_t
sp_TdmIntrInit( uint32_t port, uint32_t intr_type)
{
	uint32_t core;
	int32_t ret;

#ifdef LSI_SP27XX_BUILT_FOR_DSS
	core = (uint32_t)sp_read_cpuid();
#else /* if this is compiled for ARM */
	core = 4;
#endif
	/* disable interrupt */
	ret = __tdm_intr_disable(port, core, intr_type);
	if(ret<0)
	{
		return FAIL;
	}

	/* clear interrupt */
	ret = __tdm_intr_clr(port, core, intr_type);
	if(ret<0)
	{
		return FAIL;
	}

	/* enable interrupt */
	ret = __tdm_intr_enable(port, core, intr_type);
	if(ret<0)
	{
		return FAIL;
	}

	return SUCCESS;
}

/* TDM interrupt clear */
int32_t sp_TdmIntrClr(uint32_t port, uint32_t intr_type)
{
	uint32_t core;
	int32_t ret;

#ifdef LSI_SP27XX_BUILT_FOR_DSS
	core = (uint32_t)sp_read_cpuid();
#else /* if this is compiled for ARM */
	core = 4;
#endif

	ret = __tdm_intr_clr(port, core, intr_type);
	if(ret<0)
	{
		return FAIL;
	}

	return SUCCESS;
}

/* TDM interrupt reset */
int32_t sp_TdmIntrRst(uint32_t port, uint32_t intr_type)
{
	uint32_t core;
	int32_t ret;

#ifdef LSI_SP27XX_BUILT_FOR_DSS
	core = (uint32_t)sp_read_cpuid();
#else /* if this is compiled for ARM */
	core = 4;
#endif
	/* disable interrupt */
	ret = __tdm_intr_disable(port, core, intr_type);
	if(ret<0)
	{
		return FAIL;
	}

	/* clear interrupt */
	ret = __tdm_intr_clr(port, core, intr_type);
	if(ret<0)
	{
		return FAIL;
	}

	return SUCCESS;
}

/* indentify the interrupt of certain port */
uint32_t sp_TdmIdentifyIntr(uint32_t port)
{
	uint32_t intr_stat = 0, intr_par_stat = 0;

#ifdef LSI_SP27XX_BUILT_FOR_DSS
	REG32_READ(LSI_SP27XX_TDM_DINTMSTAT_RA(sp_read_cpuid()), intr_stat);
	REG32_READ(LSI_SP27XX_TDM_DINTPMSTAT_RA(sp_read_cpuid()), intr_par_stat);
#else
	REG32_READ(LSI_SP27XX_TDM_PINTMSTAT_RA, intr_stat);
	REG32_READ(LSI_SP27XX_TDM_PINTPMSTAT_RA, intr_par_stat);
#endif

	intr_stat = (intr_stat<<(port*4));

	/* add parity intr status */
	if(((intr_par_stat>>port)&0x1)==0x1)
	{
		intr_stat = 0x10|intr_stat;
	}

	return intr_stat;
}

int32_t sp_TdmSIUConfCommonFSCLK(uint32_t port_input_clk_fs_from, uint32_t port_output_clk_fs_from)
{
	if (port_input_clk_fs_from > MAX_NUM_PORT-1)
	{
		return FAIL;
	}

	if (port_output_clk_fs_from > MAX_NUM_PORT-1)
	{
		return FAIL;
	}

	/* clear fields */
	REG32_RESET_BITS(LSI_SP27XX_TDM_TDMCLKCTL_RA, LSI_SP27XX_TDM_TDMCLKCTL_SICKCTL_BM);
	REG32_RESET_BITS(LSI_SP27XX_TDM_TDMCLKCTL_RA, LSI_SP27XX_TDM_TDMCLKCTL_SOCKCTL_BM);

	REG32_SET_BITS(LSI_SP27XX_TDM_TDMCLKCTL_RA, ((port_input_clk_fs_from<<1)|0x1)&LSI_SP27XX_TDM_TDMCLKCTL_SICKCTL_BM);
	REG32_SET_BITS(LSI_SP27XX_TDM_TDMCLKCTL_RA, ((port_output_clk_fs_from<<5)|0x10)&LSI_SP27XX_TDM_TDMCLKCTL_SOCKCTL_BM);

	return SUCCESS;
}

int32_t sp_TdmSIUResetInternalLB (uint32_t port)
{
    return (__siu_reset_loopback_for_test(port));
}

int32_t sp_TdmSIUConfInternalLB(uint32_t lpbk, uint32_t port, uint32_t n_ch, uint32_t n_bit_code, uint32_t mode, uint32_t clk_div)
{
	if (port > MAX_NUM_PORT-1)
	{
		return FAIL;
	}

	__siu_reset(port);

	/* SR for external lpbk no need to do this  turn on internal loopback mode */
        if (lpbk == 1)
	    __siu_set_loopback_for_test(port);

	/* turn on channel mode */
	__siu_mode_set(port, mode); /* set SCON2 bit 8 = OFRAME = chan mode */

	__siu_nbits_set(port, n_bit_code, n_bit_code);   /* SCON0 ISIZE (bit 4-7) OSIZE (bit 11 - 14) = 8 bit channel size */

	/* clk and fs on the input side will be passive and will be fed by signals on output side */
	__siu_in_clk_set(port, PASSIVE_CLK, OFF_INTERNAL_CLK);  /* SCON0 (ICKA) (bit 18) and SCON1 (ICKE) (bit 16) */

	__siu_in_fsync_set(port, PASSIVE_FS, OFF_INTERNAL_FS); /* SCON0 (IFSA bit 16) and SCON1 (IFSE bit 17) */

	/* clk and fs on the input side will be passive and will be fed by signals on output side */
	__siu_out_clk_set(port, ACTIVE_CLK, ON_INTERNAL_CLK); /* SCON0 OCKA (bit 22), SCON2 (OCKE bit 16)) */

	__siu_out_fsync_set(port, ACTIVE_FS, ON_INTERNAL_FS); /* SCON0 (OFSA bit 20 ) and SCON1 (OFSE bit 17 ) */

	/* set number of channel that will be tested */
	__siu_chnum_set(port, n_ch);   /* SCON1 (IFLIM bit 0-7 = FF) SCON2 (OFLIM bit 0 - 7 = FF)) */

	/* clk ratio adjust */
	__siu_ratio_set(port, clk_div-1 /* agcklim */, ((n_ch * code_to_nbits[n_bit_code])-1));
        /* SCON3 AGCKLIM (bit 0-7) SCONE4 AGFSLIM (bit 0-12) */

	__siu_clr_status(port);  /* STAT = 0xff3fff*/

	return SUCCESS;
}

int32_t sp_TdmSIUConfExtLB_usingBB(uint32_t port, uint32_t n_ch, uint32_t n_bit_code, uint32_t mode)
{
	if (port > MAX_NUM_PORT-1)
	{
		return FAIL;
	}

	__siu_reset(port);

	/* turn on channel mode */
	__siu_mode_set(port, mode);

	__siu_nbits_set(port, n_bit_code, n_bit_code);

	/* clk and fs on the input side will be passive and will be fed by signals on output side */
	__siu_in_clk_set(port, PASSIVE_CLK, OFF_INTERNAL_CLK);

	__siu_in_fsync_set(port, PASSIVE_FS, OFF_INTERNAL_FS);

	/* clk and fs on the input side will be passive and will be fed by signals on output side */
	__siu_out_clk_set(port, PASSIVE_CLK, OFF_INTERNAL_CLK);

	__siu_out_fsync_set(port, PASSIVE_FS, OFF_INTERNAL_FS);

	__siu_clk_ctrl(port, 1);  /* SIUCLKEN = 1 SCON0 bit 26 */

	__siu_shift_order(port, 1, 1);  /* OMSB=IMSB = 1 SCON0 bit 10 and bit 3 */

	/* set number of channel that will be tested */
	__siu_chnum_set(port, n_ch);

	__siu_clr_status(port);

	__siu_int_clk_fs_off(port);
	__siu_agsync(port);
	__siu_clk_ctl(port);
	//__siu_ratio_set(port, 0, 0);
	__siu_ratio_set(port, 23-1 /* agcklim */, ((n_ch * code_to_nbits[n_bit_code])-1));

	return SUCCESS;
}

int32_t sp_TdmSWTU2Dconfig(uint32_t port, uint32_t autoload\
						, uint32_t when_intr, uint32_t n_sample_intr /* this is valid only if type_int == 0x7 */)
{
	if (port > MAX_NUM_PORT-1)
	{
		return FAIL;
	}

	__swtu_reset(port);
	__swtu_conf(port, POSTMOD_2DIMENSIONAL, 0, 0, NO_BACKWARD_COMPAT, autoload, when_intr, n_sample_intr);

	return SUCCESS;
}

int32_t sp_TdmSWTU1Dconfig(uint32_t port, uint32_t autoload\
						, uint32_t when_intr, uint32_t n_channel_intr /* this is valid only if type_int == 0x7 */)
{
	if (port > MAX_NUM_PORT-1)
	{
		return FAIL;
	}

	__swtu_reset(port);
	__swtu_conf(port, POSTMOD_1DIMENSIONAL, 0, 0, NO_BACKWARD_COMPAT, autoload, when_intr, n_channel_intr);

	return SUCCESS;
}

int32_t sp_TdmSWTU2D_backward_config(uint32_t port, uint32_t autoload\
						, uint32_t stride, uint32_t reindex, uint32_t when_intr\
						, uint32_t n_sample_intr /* this is valid only if type_int == 0x7 */)
{
	if (port > MAX_NUM_PORT-1)
	{
		return FAIL;
	}

	__swtu_reset(port);
	__swtu_conf(port, POSTMOD_2DIMENSIONAL, stride, reindex, BACKWARD_COMPAT, autoload, when_intr, n_sample_intr);

	return SUCCESS;
}

int32_t sp_Tdm1D_InitChan(uint32_t port, uint32_t nchannel, \
				uint32_t nsample, uint32_t* src_base, uint32_t* dst_base, \
				uint32_t double_buffering )
{
	if (port > MAX_NUM_PORT-1)
	{
		return FAIL;
	}

	if ((src_base==NULL)||(dst_base==NULL))
	{
		return FAIL;
	}

	/* if double buffering is used, we need to allocate the size twice big for each channel. */

	if(double_buffering == DOUBLE_BUFFERING)
	{
		__swtu_set_ch_buf_1D_2D_Backward(port, nsample, 2*nchannel, (int*)src_base, (int*)dst_base);
	}
	else /* double_buffering == SINGLE_BUFFERING */
	{
		__swtu_set_ch_buf_1D_2D_Backward(port, nsample, nchannel, (int*)src_base, (int*)dst_base);
	}

	return SUCCESS;
}

int32_t sp_Tdm2D_Backward_InitChan(uint32_t port, uint32_t nchannel, \
				uint32_t nsample, uint32_t* src_base, uint32_t* dst_base, \
				uint32_t double_buffering )
{
	if (port > MAX_NUM_PORT-1)
	{
		return FAIL;
	}

	if ((src_base==NULL)||(dst_base==NULL))
	{
		return FAIL;
	}

	/* if double buffering is used, we need to allocate the size twice big for each channel. */

	if(double_buffering == DOUBLE_BUFFERING)
	{
		__swtu_set_ch_buf_1D_2D_Backward(port, nchannel, 2*nsample, (int*)src_base, (int*)dst_base);
	}
	else /* double_buffering == SINGLE_BUFFERING */
	{
		__swtu_set_ch_buf_1D_2D_Backward(port, nchannel, nsample, (int*)src_base, (int*)dst_base);
	}

	return SUCCESS;
}

int32_t sp_Tdm2DInitChan(uint32_t port, uint32_t ch_num_from, uint32_t ch_num_to, \
				uint32_t nbits_sample, uint32_t nsample, uint32_t* src_base, uint32_t* dst_base, \
				uint32_t uni_cnt, /* if uni_cnt > 32, then universal counter will not be used for this channel */
				uint32_t double_buffering )
{
	uint32_t src_addr, dst_addr;
	int32_t i;

	if (port > MAX_NUM_PORT-1)
	{
		return FAIL;
	}

	if (ch_num_from > ch_num_to)
	{
		return FAIL;
	}

	if ((src_base==NULL)||(dst_base==NULL))
	{
		return FAIL;
	}

	src_addr = (uint32_t)src_base;
	dst_addr = (uint32_t)dst_base;

	for(i=ch_num_from; i<ch_num_to+1; i++)
	{
		if(double_buffering == DOUBLE_BUFFERING)
		{
			/* if double buffering is used, we need to allocate the size twice big for each channel. */
			__swtu_set_ch_buf_2D(port, i, nsample*2, \
							(int*)(src_addr+(2*(i-ch_num_from)*nbits_sample*nsample)/8), \
							(int*)(dst_addr+(2*(i-ch_num_from)*nbits_sample*nsample)/8));
		}
		else /* double_buffering == SINGLE_BUFFERING */
		{
			/* if double buffering is used, we need to allocate the size twice big for each channel. */
			__swtu_set_ch_buf_2D(port, i, nsample, \
								(int*)(src_addr+((i-ch_num_from)*nbits_sample*nsample)/8), \
								(int*)(dst_addr+((i-ch_num_from)*nbits_sample*nsample)/8));
		}

		if(uni_cnt < NOT_USING_UNIVERSAL_COUNTER)
		{
			/* connecting universal counter to the channel */
			__swtu_conn_unicnt_to_ch(port, i, uni_cnt);
		} /* if universal counter is not used */
		else
		{
			__swtu_disconn_unicnt_to_ch(port, i);
		}
	}

	return SUCCESS;
}

void sp_TdmInitUniversalCnt(uint32_t unicnt, uint32_t fed_clk, uint32_t limit)
{
	__unicnt_dis(unicnt);
	__unicnt_reset(unicnt);
	__unicnt_conf(unicnt, fed_clk, limit);
	__unicnt_en(unicnt);
}

int32_t sp_TdmEnableInChan(uint32_t port, uint32_t ch_num_from, uint32_t ch_num_to)
{
	int32_t i;
	uint32_t group_from;
	uint32_t group_to;

	uint32_t bitofst_start;
	uint32_t bitofst_end;

	if (port > MAX_NUM_PORT-1)
	{
		return FAIL;
	}

	if (ch_num_from > ch_num_to)
	{
		return FAIL;
	}

	group_from = ch_num_from/32;
	group_to	= ch_num_to/32;

	bitofst_start = ch_num_from%32;
	bitofst_end = ch_num_to%32;

	if(group_from == group_to) /* if channels's eable bits are in one register, */
	{
		__siu_onoff_in_chs(port, group_from /* == group_to */, SET_BITS_RANGE(bitofst_start, bitofst_end), CHAN_ON);
	}
	else /* group_from<group_to */
	{
		/* group_from */
		__siu_onoff_in_chs(port, group_from, SET_BITS_RANGE(bitofst_start, 31), CHAN_ON);

		/* group_from+1~group_to-1 */
		for(i=group_from+1; i<group_to; i++)
		{
			__siu_onoff_in_chs(port, i, 0xFFFFFFFF, CHAN_ON);
		}
		/* group_to */
		__siu_onoff_in_chs(port, group_to, SET_BITS_RANGE(0, bitofst_end), CHAN_ON);
	}
	return SUCCESS;
}

int32_t sp_TdmEnableOutChan(uint32_t port, uint32_t ch_num_from, uint32_t ch_num_to)
{
	int32_t i;
	uint32_t group_from;
	uint32_t group_to;

	uint32_t bitofst_start;
	uint32_t bitofst_end;

	if (port > MAX_NUM_PORT-1)
	{
		return FAIL;
	}

	if (ch_num_from > ch_num_to)
	{
		return FAIL;
	}

	group_from = ch_num_from/32;
	group_to	= ch_num_to/32;

	bitofst_start = ch_num_from%32;
	bitofst_end = ch_num_to%32;

	if(group_from == group_to) /* if channels's eable bits are in one register, */
	{
		__siu_onoff_out_chs(port, group_from /* == group_to */, SET_BITS_RANGE(bitofst_start, bitofst_end), CHAN_ON);
		__siu_msk_out_chs(port, group_from /* == group_to */, SET_BITS_RANGE(bitofst_start, bitofst_end), CHAN_UNMASK);
	}
	else /* group_from<group_to */
	{
		/* group_from */
		__siu_onoff_out_chs(port, group_from, SET_BITS_RANGE(bitofst_start, 31), CHAN_ON);
		__siu_msk_out_chs(port, group_from, SET_BITS_RANGE(bitofst_start, 31), CHAN_UNMASK);

		/* group_from+1~group_to-1 */
		for(i=group_from+1; i<group_to; i++)
		{
			__siu_onoff_out_chs(port, i, 0xFFFFFFFF, CHAN_ON);
			__siu_msk_out_chs(port, i, 0xFFFFFFFF, CHAN_UNMASK);
		}
		/* group_to */
		__siu_onoff_out_chs(port, group_to, SET_BITS_RANGE(0, bitofst_end), CHAN_ON);
		__siu_msk_out_chs(port, group_to, SET_BITS_RANGE(0, bitofst_end), CHAN_UNMASK);
	}
	return SUCCESS;
}

int32_t sp_TdmDisableInChan(uint32_t port, uint32_t ch_num_from, uint32_t ch_num_to)
{
	int32_t i;
	uint32_t group_from;
	uint32_t group_to;

	uint32_t bitofst_start;
	uint32_t bitofst_end;

	if (port > MAX_NUM_PORT-1)
	{
		return FAIL;
	}

	if (ch_num_from > ch_num_to)
	{
		return FAIL;
	}

	group_from = ch_num_from/32;
	group_to	= ch_num_to/32;

	bitofst_start = ch_num_from%32;
	bitofst_end = ch_num_to%32;

	if(group_from == group_to) /* if channels's eable bits are in one register, */
	{
		__siu_onoff_in_chs(port, group_from /* == group_to */, SET_BITS_RANGE(bitofst_start, bitofst_end), CHAN_OFF);
	}
	else /* group_from<group_to */
	{
		/* group_from */
		__siu_onoff_in_chs(port, group_from, SET_BITS_RANGE(bitofst_start, 31), CHAN_OFF);

		/* group_from+1~group_to-1 */
		for(i=group_from+1; i<group_to; i++)
		{
			__siu_onoff_in_chs(port, i, 0xFFFFFFFF, CHAN_OFF);
		}

		/* group_to */
		__siu_onoff_in_chs(port, group_from, SET_BITS_RANGE(0, bitofst_end), CHAN_OFF);
	}
	return SUCCESS;
}

int32_t sp_TdmDisableOutChan(uint32_t port, uint32_t ch_num_from, uint32_t ch_num_to)
{
	int32_t i;
	uint32_t group_from;
	uint32_t group_to;

	uint32_t bitofst_start;
	uint32_t bitofst_end;

	if (port > MAX_NUM_PORT-1)
	{
		return FAIL;
	}

	if (ch_num_from > ch_num_to)
	{
		return FAIL;
	}

	group_from = ch_num_from/32;
	group_to	= ch_num_to/32;

	bitofst_start = ch_num_from%32;
	bitofst_end = ch_num_to%32;

	if(group_from == group_to) /* if channels's eable bits are in one register, */
	{
		__siu_onoff_out_chs(port, group_from /* == group_to */, SET_BITS_RANGE(bitofst_start, bitofst_end), CHAN_OFF);
		__siu_msk_out_chs(port, group_from /* == group_to */, SET_BITS_RANGE(bitofst_start, bitofst_end), CHAN_MASK);
	}
	else /* group_from<group_to */
	{
		/* group_from */
		__siu_onoff_out_chs(port, group_from, SET_BITS_RANGE(bitofst_start, 31), CHAN_OFF);
		__siu_msk_out_chs(port, group_from, SET_BITS_RANGE(bitofst_start, 31), CHAN_MASK);

		/* group_from+1~group_to-1 */
		for(i=group_from+1; i<group_to; i++)
		{
			__siu_onoff_out_chs(port, i, 0xFFFFFFFF, CHAN_OFF);
			__siu_msk_out_chs(port, i, 0xFFFFFFFF, CHAN_MASK);
		}

		/* group_to */
		__siu_onoff_out_chs(port, group_from, SET_BITS_RANGE(0, bitofst_end), CHAN_OFF);
		__siu_msk_out_chs(port, group_from, SET_BITS_RANGE(0, bitofst_end), CHAN_MASK);
	}
	return SUCCESS;
}

int32_t sp_TdmInternalClkRun(uint32_t port)
{
	if (port > MAX_NUM_PORT-1)
	{
		return FAIL;
	}

	__siu_int_clk_fs_on(port);
#ifdef TDM_DEBUG
sp_SerialPutS("\r\n In sp_TdmInternalClkRun() after sp_TdmInternalClkRun()");
#endif

	return SUCCESS;
}

int32_t sp_TdmRun(uint32_t port)
{
	if (port > MAX_NUM_PORT-1)
	{
		return FAIL;
	}

	__swtu_start(port);
#ifdef TDM_DEBUG
sp_SerialPutS("\r\n In sp_TdmRun() after __swtu_start()");
#endif
	__siu_start(port);
#ifdef TDM_DEBUG
sp_SerialPutS("\r\n In sp_TdmRun() after __siu_start()");
#endif

	return SUCCESS;
}

int32_t sp_TdmStop(uint32_t port)
{
	if (port > MAX_NUM_PORT-1)
	{
		return FAIL;
	}

	__siu_stop(port);
	__swtu_stop(port);

	return SUCCESS;
}

/******** History ********
$Log: libtdm.c,v $
Revision 1.2  2017/07/28 07:58:49  harrchan
Collapse Oakenshield-branch to Main Trunk.

Revision 1.1.2.1  2017/06/29 08:14:36  harrchan
Initial commit code for Oakenshield

Revision 1.4  2012/06/07 22:50:24  srane
Support TDM external loopback test.

Revision 1.3  2012/05/24 23:23:08  srane
Add wrapper for TDM external loopback test.

Revision 1.2  2012/05/10 22:48:11  srane
clean up and modify files for exception handling and TDM tests.

Revision 1.1  2012/04/18 09:47:31  srane
Initial checkin


$Endlog$
*/

