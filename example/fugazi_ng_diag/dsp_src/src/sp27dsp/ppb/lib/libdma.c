/* $Id: libdma.c,v 1.1 2012/06/07 22:34:33 srane Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/dsp_src/src/sp27dsp/ppb/lib/libdma.c,v $
 *------------------------------------------------------------------
 * libdma.c 
 * Description: setup routines for SP2700 DMAC31 controllers
 *
 * June 2012, Smita Rane
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
 * libdma.c - setup routines for SP2700 DMAC31 controllers
 *
 *  Created on: Oct 16, 2009
 *      Author: dokim
 */

#include "lsi_sp27xx_reg.h"
#include "libdma.h"
#include "libgeneric.h"
#include "libuart.h"


#define DUMMY_LOAD 1
#define NORMAL_LOAD 0

#define GENSTAT_CHK_MASK_CH0	(LSI_SP27XX_DMA_GENERAL_STATUS_REGISTER_CH0_ACTIVE_BM | \
									LSI_SP27XX_DMA_GENERAL_STATUS_REGISTER_CH0_ERROR_FLOW_BM)

#define GENSTAT_CHK_MASK_CH1	(LSI_SP27XX_DMA_GENERAL_STATUS_REGISTER_CH1_ACTIVE_BM | \
									LSI_SP27XX_DMA_GENERAL_STATUS_REGISTER_CH1_ERROR_FLOW_BM)

#define GENSTAT_CHK_MASK_ERR	(LSI_SP27XX_DMA_GENERAL_STATUS_REGISTER_CH0_ERROR_FLOW_BM | \
									LSI_SP27XX_DMA_GENERAL_STATUS_REGISTER_CH1_ERROR_FLOW_BM)

extern unsigned int dmac_base_addr[];

/* a global dma_desc object that can be used temporally in any dma functions */
dma_desc_t g_dma_desc;

/* interrupt related functions */

DMACHANSTAT_t					/* ret: DMACHAN_ERROR if channel encountered error */
sp_dma_StatusClr(				/* clear all pending status register bits, if set */
	uint32_t dmac,				/* in: DSS<0-3>_DMAC, DBM_DMAC<0-1>, DSS_DMAC_LCL (DSS only) */
	uint32_t ch_no)				/* in: channel to check (0 or 1) */
{
uint32_t stat;

	if (dmac > NUM_DMAC - 1) {
		return DMACHAN_PARAM_ERROR; 			/* non-existing DMA controller */
	} else if ( ch_no > NUM_DMAC_CH - 1) {
		return DMACHAN_PARAM_ERROR;				/* invalid channel */
	}

	/* clear any  status bits leftover from previous transfer */
	REG32_SET_BITS(LSI_SP27XX_DMAC31_DMA_STATUSREG_CH_RA(dmac_base_addr[dmac], ch_no),
			LSI_SP27XX_DMA_STATUSREG_CH_CH_ERROR_ACC_DL_BM | LSI_SP27XX_DMA_STATUSREG_CH_CH_ERROR_ACC_DST_BM |
			LSI_SP27XX_DMA_STATUSREG_CH_CH_ERROR_ACC_SRC_BM | LSI_SP27XX_DMA_STATUSREG_CH_BLOCK_COMPLETED_BM |
			LSI_SP27XX_DMA_STATUSREG_CH_TRANSFER_COMPLETED_BM );

	stat = HW_REG_ACCESS(LSI_SP27XX_DMAC31_DMA_GENERAL_STATUS_REGISTER_RA(dmac_base_addr[dmac]));

	stat &= ( ch_no == 0 ? GENSTAT_CHK_MASK_CH0 : GENSTAT_CHK_MASK_CH1 ) ;

	return (stat == 0 ? DMACHAN_NOTACTIVE : (stat & GENSTAT_CHK_MASK_ERR) ? DMACHAN_ERROR : DMACHAN_ACTIVE);
}



DMACHANSTAT_t					/* ret: DMACHAN_ERROR if channel encountered error */
sp_dma_IdentifyInterrupt(		/* determine reason for DMAC interrupt */
	uint32_t dmac,				/* in: DSS<0-3>_DMAC, DBM_DMAC<0-1>, DSS_DMAC_LCL (DSS only) */
	uint32_t ch_no,				/* in: channel to check (0 or 1) */
	DMAC_INTR_t *p_type)		/* out: INTR_EOB, INTR_EOT, INTR_BOTH or INTR_NONE */
{
uint32_t stat;

	if (dmac > NUM_DMAC - 1) {
		return DMACHAN_PARAM_ERROR; 			/* non-existing DMA controller */
	} else if ( ch_no > NUM_DMAC_CH - 1) {
		return DMACHAN_PARAM_ERROR;				/* invalid channel */
	}

	if (p_type != NULL) {
		/* combination of TRANSFER_COMPLETED and BLOCK_COMPLETED bits in chx status register */
		*p_type = (DMAC_INTR_t) ((HW_REG_ACCESS(LSI_SP27XX_DMAC31_DMA_STATUSREG_CH_RA(dmac_base_addr[dmac], ch_no))
				>> LSI_SP27XX_DMA_STATUSREG_CH_BLOCK_COMPLETED_BO) & 0x3);
	}

	stat = HW_REG_ACCESS(LSI_SP27XX_DMAC31_DMA_GENERAL_STATUS_REGISTER_RA(dmac_base_addr[dmac]));

	stat &= ( ch_no == 0 ? GENSTAT_CHK_MASK_CH0 : GENSTAT_CHK_MASK_CH1 ) ;

	return (stat == 0 ? DMACHAN_NOTACTIVE : (stat & GENSTAT_CHK_MASK_ERR) ? DMACHAN_ERROR : DMACHAN_ACTIVE);
}

DMACHANSTAT_t				/* ret: DMACHAN_ERROR if channel encountered error */
sp_dma_Recover(				/* reclaim a channel (even if it's currently running) */
	uint32_t dmac,			/* in: DSS<0-3>_DMAC, DBM_DMAC<0-1>, DSS_DMAC_LCL (DSS only) */
	uint32_t ch_no)			/* in: channel to recover (0 or 1) */
{
	uint32_t stat;

	if (dmac > NUM_DMAC - 1) {
		return DMACHAN_PARAM_ERROR; 			/* non-existing DMA controller */
	} else if ( ch_no > NUM_DMAC_CH - 1) {
		return DMACHAN_PARAM_ERROR;				/* invalid channel */
	}

	/* disable the channel */
	REG32_RESET_BITS(LSI_SP27XX_DMAC31_DMA_CONFREG_CH_RA(dmac_base_addr[dmac], ch_no),
			LSI_SP27XX_DMA_CONFREG_CH_CHAN_EN_BM);

	/* wait for channel to become inactive */
	while (HW_REG_ACCESS(LSI_SP27XX_DMAC31_DMA_STATUSREG_CH_RA(dmac_base_addr[dmac], ch_no)) &
			LSI_SP27XX_DMA_STATUSREG_CH_CH_ACTIVE_BM) ;

	/* write 1 to clear fifo bit */
	REG32_SET_BITS(LSI_SP27XX_DMAC31_DMA_CONFREG_CH_RA(dmac_base_addr[dmac], ch_no), LSI_SP27XX_DMA_CONFREG_CH_CLEAR_FIFO_BM);

	/* clear any  status bits leftover from previous transfer */
	REG32_SET_BITS(LSI_SP27XX_DMAC31_DMA_STATUSREG_CH_RA(dmac_base_addr[dmac], ch_no),
			LSI_SP27XX_DMA_STATUSREG_CH_CH_ERROR_ACC_DL_BM | LSI_SP27XX_DMA_STATUSREG_CH_CH_ERROR_ACC_DST_BM |
			LSI_SP27XX_DMA_STATUSREG_CH_CH_ERROR_ACC_SRC_BM | LSI_SP27XX_DMA_STATUSREG_CH_BLOCK_COMPLETED_BM |
			LSI_SP27XX_DMA_STATUSREG_CH_TRANSFER_COMPLETED_BM );

	stat = HW_REG_ACCESS(LSI_SP27XX_DMAC31_DMA_GENERAL_STATUS_REGISTER_RA(dmac_base_addr[dmac]));

	stat &= ( ch_no == 0 ? GENSTAT_CHK_MASK_CH0 : GENSTAT_CHK_MASK_CH1 ) ;

	return (stat == 0 ? DMACHAN_NOTACTIVE : (stat & GENSTAT_CHK_MASK_ERR) ? DMACHAN_ERROR : DMACHAN_ACTIVE);
}


DMACHANSTAT_t				/* ret: DMACHAN_ERROR if channel encountered error */
sp_dma_Disable(				/* disable a running channel */
	uint32_t dmac,			/* in: DSS<0-3>_DMAC, DBM_DMAC<0-1>, DSS_DMAC_LCL (DSS only) */
	uint32_t ch_no)			/* in: channel to check (0 or 1) */
{
uint32_t stat;

	if (dmac > NUM_DMAC - 1) {
		return DMACHAN_PARAM_ERROR; 			/* non-existing DMA controller */
	} else if ( ch_no > NUM_DMAC_CH - 1) {
		return DMACHAN_PARAM_ERROR;				/* invalid channel */
	}

	/* disable the channel */
	REG32_RESET_BITS(LSI_SP27XX_DMAC31_DMA_CONFREG_CH_RA(dmac_base_addr[dmac], ch_no),
			LSI_SP27XX_DMA_CONFREG_CH_CHAN_EN_BM);

	/* wait for channel to become inactive */
	while (HW_REG_ACCESS(LSI_SP27XX_DMAC31_DMA_STATUSREG_CH_RA(dmac_base_addr[dmac], ch_no)) &
			LSI_SP27XX_DMA_STATUSREG_CH_CH_ACTIVE_BM) ;

	stat = HW_REG_ACCESS(LSI_SP27XX_DMAC31_DMA_GENERAL_STATUS_REGISTER_RA(dmac_base_addr[dmac]));

	stat &= ( ch_no == 0 ? GENSTAT_CHK_MASK_CH0 : GENSTAT_CHK_MASK_CH1 ) ;

	return (stat == 0 ? DMACHAN_NOTACTIVE : (stat & GENSTAT_CHK_MASK_ERR) ? DMACHAN_ERROR : DMACHAN_ACTIVE);
}


DMACHANSTAT_t				/* ret: DMACHAN_ERROR if channel encountered error */
sp_dma_Pause(				/* pause a running channel */
	uint32_t dmac,			/* in: DSS<0-3>_DMAC, DBM_DMAC<0-1>, DSS_DMAC_LCL (DSS only) */
	uint32_t ch_no)			/* in: channel to check (0 or 1) */
{
uint32_t stat;

	if (dmac > NUM_DMAC - 1) {
		return DMACHAN_PARAM_ERROR; 			/* non-existing DMA controller */
	} else if ( ch_no > NUM_DMAC_CH - 1) {
		return DMACHAN_PARAM_ERROR;				/* invalid channel */
	}

	/* write 1 to ch_pause and ch_pause_wr_en */
	REG32_SET_BITS( LSI_SP27XX_DMAC31_DMA_STATUSREG_CH_RA(dmac_base_addr[dmac], ch_no), LSI_SP27XX_DMA_STATUSREG_CH_CH_PAUSE_BM \
									| LSI_SP27XX_DMA_STATUSREG_CH_CH_PAUSE_WR_EN_BM);

	/* wait for channel to pause */
	while ((HW_REG_ACCESS(LSI_SP27XX_DMAC31_DMA_STATUSREG_CH_RA(dmac_base_addr[dmac], ch_no)) &
			LSI_SP27XX_DMA_STATUSREG_CH_CH_PAUSE_BM) == 0) ;

	stat = HW_REG_ACCESS(LSI_SP27XX_DMAC31_DMA_GENERAL_STATUS_REGISTER_RA(dmac_base_addr[dmac]));

	stat &= ( ch_no == 0 ? GENSTAT_CHK_MASK_CH0 : GENSTAT_CHK_MASK_CH1 ) ;

	return (stat == 0 ? DMACHAN_NOTACTIVE : (stat & GENSTAT_CHK_MASK_ERR) ? DMACHAN_ERROR : DMACHAN_ACTIVE);
}

DMACHANSTAT_t				/* ret: DMACHAN_ERROR if channel encountered error */
sp_dma_Resume(				/* resume a paused channel */
	uint32_t dmac,			/* in: DSS<0-3>_DMAC, DBM_DMAC<0-1>, DSS_DMAC_LCL (DSS only) */
	uint32_t ch_no)			/* in: channel to check (0 or 1) */
{
uint32_t stat;

	if (dmac > NUM_DMAC - 1) {
		return DMACHAN_PARAM_ERROR; 			/* non-existing DMA controller */
	} else if ( ch_no > NUM_DMAC_CH - 1) {
		return DMACHAN_PARAM_ERROR;				/* invalid channel */
	}

	/* see if channel is paused */
	if ((HW_REG_ACCESS(LSI_SP27XX_DMAC31_DMA_STATUSREG_CH_RA(dmac_base_addr[dmac], ch_no)) &
			LSI_SP27XX_DMA_STATUSREG_CH_CH_PAUSE_BM) == 0) {
		return DMACHAN_PARAM_ERROR;				/* channel not paused! */
	}

	/* read the channel status register */
	stat = HW_REG_ACCESS(LSI_SP27XX_DMAC31_DMA_STATUSREG_CH_RA(dmac_base_addr[dmac], ch_no));
	stat &= ~LSI_SP27XX_DMA_STATUSREG_CH_CH_PAUSE_BM;				/* clear the pause bit */
	stat |= LSI_SP27XX_DMA_STATUSREG_CH_CH_PAUSE_WR_EN_BM;			/* set ch_pause_wr_en */
	/* write to clear pause bit */
	REG32_WRITE(LSI_SP27XX_DMAC31_DMA_STATUSREG_CH_RA(dmac_base_addr[dmac], ch_no), stat);

	stat = HW_REG_ACCESS(LSI_SP27XX_DMAC31_DMA_GENERAL_STATUS_REGISTER_RA(dmac_base_addr[dmac]));

	stat &= ( ch_no == 0 ? GENSTAT_CHK_MASK_CH0 : GENSTAT_CHK_MASK_CH1 ) ;

	return (stat == 0 ? DMACHAN_NOTACTIVE : (stat & GENSTAT_CHK_MASK_ERR) ? DMACHAN_ERROR : DMACHAN_ACTIVE);
}

DMACHANSTAT_t				/* ret: 0: DMACHAN_ACTIVE, else none-zero (e.g. DMACHAN_NOTACTIVE) */
sp_dma_DoneCheck(			/* check if a DMA operation is still running on the channel */
	uint32_t dmac,			/* in: DSS<0-3>_DMAC, DBM_DMAC<0-1>, DSS_DMAC_LCL (DSS only) */
	uint32_t ch_no)			/* in: channel to check (0 or 1) */
{
uint32_t stat;

	if (dmac > NUM_DMAC - 1) {
		return DMACHAN_PARAM_ERROR; 			/* non-existing DMA controller */
	} else if ( ch_no > NUM_DMAC_CH - 1) {
		return DMACHAN_PARAM_ERROR;				/* invalid channel */
	}

	stat = HW_REG_ACCESS(LSI_SP27XX_DMAC31_DMA_GENERAL_STATUS_REGISTER_RA(dmac_base_addr[dmac]));

	stat &= ( ch_no == 0 ? GENSTAT_CHK_MASK_CH0 : GENSTAT_CHK_MASK_CH1 ) ;

	return (stat == 0 ? DMACHAN_NOTACTIVE : (stat & GENSTAT_CHK_MASK_ERR) ? DMACHAN_ERROR : DMACHAN_ACTIVE);
}

#if 0
uint32_t						/* ret: SUCCESS or ERROR_BAD_PARAM */
sp_dma_DebugResume(				/* Allow DMAC channel to continue to run when in Debug mode */
	uint32_t dmac)
{
	if (dmac > NUM_DMAC - 1) {
		return ERROR_BAD_PARAM; /* non-existing DMA controller */
	}

	REG32_SET_BITS(LSI_SP27XX_DMAC31_DMA_GENERAL_CONFIGURATION_REGISTER_RA(dmac_base_addr[dmac]),
			LSI_SP27XX_DMA_GENERAL_CONFIGURATION_REGISTER_DMA_DBG_RESUME_BM);

	return SUCCESS;
}
#endif

int32_t sp_dma_DescToHWReg(uint32_t dmac, uint32_t ch_no, dma_desc_t* p_desc)
{
	volatile uint32_t* dma_reg;

	if (dmac > NUM_DMAC-1) {
		return ERROR_BAD_PARAM; 			/* non-existing DMA controller */
	} else if ( ch_no > NUM_DMAC_CH-1) {
		return ERROR_BAD_PARAM;				/* invalid channel */
	}

	dma_reg = (volatile uint32_t*)LSI_SP27XX_DMAC31_DMA_SRCXCOUNT_CH_RA(dmac_base_addr[dmac], ch_no);

	switch(p_desc->type_dma_desc)
	{
	case DESC_TYPE_FULL:
		*dma_reg++ = p_desc->d.full.long0.fields.src_x_cnt;			/* src x-counter */
		*dma_reg++ = p_desc->d.full.long0.fields.src_y_cnt;			/* src y-counter */
		*dma_reg++ = p_desc->d.full.long1.fields.signed_src_x_mod;		/* src x-modifier */
		*dma_reg++ = p_desc->d.full.long2.fields.signed_src_y_mod;		/* src y-modifier */
		*dma_reg++ = p_desc->d.full.long3.fields.curr_src_addr;		/* curr src addr */
		*dma_reg++ = (p_desc->d.full.long5.reg) & 0x0000FFFF;			/* src access */
		*dma_reg++ = p_desc->d.full.long4.fields.src_data_mask;		/* src mask */
		*dma_reg++ = p_desc->d.full.long8.fields.dst_x_cnt;			/* dst x-counter */
		*dma_reg++ = p_desc->d.full.long8.fields.dst_y_cnt;			/* dst y-counter */
		*dma_reg++ = p_desc->d.full.long9.fields.signed_dst_x_mod;		/* dst x-modifier */
		*dma_reg++ = p_desc->d.full.long10.fields.signed_dst_y_mod;	/* dst y-modifier */
		*dma_reg++ = p_desc->d.full.long11.fields.curr_dst_addr;		/* curr dst addr */
		*dma_reg++ = (p_desc->d.full.long5.reg>>16) & 0x0000FFFF; 		/* dst access */
		*dma_reg++ = p_desc->d.full.long7.reg;							/* nxt desc pointer */
		*dma_reg = p_desc->d.full.long6.reg;							/* configuration */
		break;

	case DESC_TYPE_1D:
		*dma_reg++ = p_desc->d.dim1.long2.fields.x_cnt;			/* src x-counter */
		*dma_reg++ = 0;																/* src y-counter */
		*dma_reg++ = 0x1<<(p_desc->d.dim1.long3.fields.access_size);	/* src x-modifier */
		*dma_reg++ = 0;																/* src y-modifier */
		*dma_reg++ = p_desc->d.dim1.long0.reg;					/* curr src addr */

		/* src access */
		*dma_reg++ = ((p_desc->d.dim1.long3.fields.src_burst<<1)|0x1)\
					 | (p_desc->d.dim1.long3.fields.access_size << 3)\
					 | (0x1 << ((p_desc->d.dim1.long3.fields.access_size)+6)); /* rotator length */

		*dma_reg++ = 0xFFFFFFFF;														/* src mask */
		*dma_reg++ = p_desc->d.dim1.long2.fields.x_cnt;			/* dst x-counter */
		*dma_reg++ = 0;			/* dst y-counter */
		*dma_reg++ = 0x1<<(p_desc->d.dim1.long3.fields.access_size);	/* dst x-modifier */
		*dma_reg++ = 0;	/* dst y-modifier */
		*dma_reg++ = p_desc->d.dim1.long1.reg;					/* curr dst addr */

		/* dst access */
		*dma_reg++ = ((p_desc->d.dim1.long3.fields.dst_burst<<1)|0x1)\
					 | (p_desc->d.dim1.long3.fields.access_size << 3);

		*dma_reg++ = ((p_desc->d.dim1.long3.reg) & 0x000FFFFF);	/* nxt desc pointer */

		/* configuration */
		*dma_reg = (p_desc->d.dim1.long2.fields.tx_en << 1) \
					 | (p_desc->d.dim1.long2.fields.int_dst_eob << 6) \
					 | (p_desc->d.dim1.long2.fields.int_dst_eot << 7) \
					 | (p_desc->d.dim1.long2.fields.stop_dst_eob << 11) \
					 | (p_desc->d.dim1.long2.fields.dma_ext_access_pri_en << 12) \
					 | (p_desc->d.dim1.long2.fields.start_mem_load << 13) \
					 | (p_desc->d.dim1.long2.fields.last_block << 15) \
					 | (p_desc->d.dim1.long2.fields.channel_priority << 16) \
					 | (p_desc->d.dim1.long2.fields.ch_full_priority << 19) \
					 | (p_desc->d.dim1.long2.fields.dma_priority << 20) \
					 | (p_desc->d.dim1.long2.fields.dma_priority_row << 21) \
					 | (p_desc->d.dim1.long2.fields.dma_ext_priory << 22) \
					 | (p_desc->d.dim1.long2.fields.src_space << 23) \
					 | (p_desc->d.dim1.long2.fields.dst_space << 26) ;
		break;

	case DESC_TYPE_1D_MA:
		/* memory assignment DMA desc cannot be directly programmed into registers */
		return ERROR;

	case DESC_TYPE_2D:
		*dma_reg++ = p_desc->d.dim2.long0.fields.src_x_cnt;			/* src x-counter */
		*dma_reg++ = p_desc->d.dim2.long0.fields.src_y_cnt;			/* src y-counter */
		*dma_reg++ = p_desc->d.dim2.long1.fields.signed_src_x_mod;	/* src x-modifier */
		*dma_reg++ = p_desc->d.dim2.long1.fields.signed_src_y_mod;	/* src y-modifier */
		*dma_reg++ = p_desc->d.dim2.long2.fields.curr_src_addr;		/* curr src addr */

		/* src access */
		*dma_reg++ = ((p_desc->d.dim2.long7.fields.ssrc_burst<<1)|0x1)\
					| (p_desc->d.dim2.long7.fields.src_size<<3) \
					| (0x1<<(p_desc->d.dim2.long7.fields.src_size + 6)) /* rotator length */ \
					| (p_desc->d.dim2.long7.fields.tail_length<<11);

		*dma_reg++ = 0xFFFFFFFF;															/* src mask */
		*dma_reg++ = p_desc->d.dim2.long3.fields.dst_x_cnt;			/* dst x-counter */
		*dma_reg++ = p_desc->d.dim2.long3.fields.dst_y_cnt;			/* dst y-counter */
		*dma_reg++ = p_desc->d.dim2.long4.fields.signed_dst_x_mod;	/* dst x-modifier */
		*dma_reg++ = p_desc->d.dim2.long4.fields.signed_dst_y_mod;	/* dst y-modifier */
		*dma_reg++ = p_desc->d.dim2.long5.fields.curr_dst_addr;		/* curr dst addr */

		/* dst access */
		*dma_reg++ = ((p_desc->d.dim2.long6.fields.dst_burst<<1)|0x1) \
						| (p_desc->d.dim2.long6.fields.dst_size<<3);

		*dma_reg++ = (p_desc->d.dim2.long7.reg) & 0x000FFFFF;		/* nxt desc pointer */

		/* configuration */
		*dma_reg = (p_desc->d.dim2.long6.fields.tx_en << 1) \
					 | (p_desc->d.dim2.long6.fields.task_cnt_1_reset << 2) \
					 | (p_desc->d.dim2.long6.fields.wait_for_task_cnt_1 << 3) \
					 | (p_desc->d.dim2.long6.fields.task_cnt_2_reset << 4) \
					 | (p_desc->d.dim2.long6.fields.wait_for_task_cnt_2 << 5) \
					 | (p_desc->d.dim2.long6.fields.int_dst_eob << 6) \
					 | (p_desc->d.dim2.long6.fields.int_dst_eot << 7) \
					 | (p_desc->d.dim2.long6.fields.stop_dst_eob << 11) \
					 | (p_desc->d.dim2.long6.fields.dma_ext_access_pri_en << 12) \
					 | (p_desc->d.dim2.long6.fields.start_mem_load << 13) \
					 | (p_desc->d.dim2.long6.fields.last_block << 15) \
					 | (p_desc->d.dim2.long6.fields.channel_priority << 16) \
					 | (p_desc->d.dim2.long6.fields.ch_full_priority << 19) \
					 | (p_desc->d.dim2.long6.fields.dma_priority << 20) \
					 | (p_desc->d.dim2.long6.fields.dma_priority_row << 21) \
					 | (p_desc->d.dim2.long6.fields.dma_ext_priory << 22) \
					 | (p_desc->d.dim2.long6.fields.src_space << 23) \
					 | (p_desc->d.dim2.long6.fields.dst_space << 26) ;
		break;

	default:
		return ERROR; /* unknown desc type */
	}

	return SUCCESS;
}

dma_desc_t *				/* ret: address of 'desc' input for success or NULL for failure */
sp_dma_SetInterrupt(		/* set up for DMAC interrupt */
	dma_desc_t *desc,		/* in: DMA descriptor to update */
	DMAC_INTR_t type)		/* in: INTR_EOB, INTR_EOT, INTR_BOTH or INTR_NONE */
{
uint32_t eot = 0;
uint32_t eob = 0;
dma_desc_t *ret_desc = desc;

	switch (type) {
	case INTR_NONE:
		break;
	case INTR_EOT:
		eot = 1;
		break;
	case INTR_EOB:
		eob = 1;
		break;
	case INTR_BOTH:
		eot = 1;
		eob = 1;
		break;
	default:
		ret_desc = NULL;
	}

	if (ret_desc != NULL) {
		switch (desc->type_dma_desc) {
		case  DESC_TYPE_1D:
		case  DESC_TYPE_1D_MA:
			ret_desc->d.dim1.long2.fields.int_dst_eot = eot;
			ret_desc->d.dim1.long2.fields.int_dst_eob = eob;
			break;
		case DESC_TYPE_2D:
			ret_desc->d.dim2.long6.fields.int_dst_eot = eot;
			ret_desc->d.dim2.long6.fields.int_dst_eob = eob;
			break;
		case DESC_TYPE_FULL:
			ret_desc->d.full.long6.fields.int_dst_eot = eot;
			ret_desc->d.full.long6.fields.int_dst_eob = eob;
			break;
		default:			/* probably a bad descriptor or maybe type_dma_desc wasn't set */
			ret_desc = NULL;
		}
	}

	return (ret_desc);
}

/* some useful examples of using functions defined in this LIB */

int32_t sp_dbmdmac_MemCpy128(void *dst_addr, void *src_addr, int32_t num_128bit_word)
{
int32_t ii, jj, kk;
int32_t timeout = num_128bit_word;
int32_t remaining_num_word = num_128bit_word;
uint32_t curr_tx_done = 0;

	dma_desc_t* p_local_desc = (dma_desc_t*)&g_dma_desc;

	/* check if both src and dst addresses are aligned */
	if (((uint32_t)dst_addr & 0xF) != 0) {
		return ERROR_BAD_PARAM;
	}
	if (((uint32_t)src_addr & 0xF) != 0) {
		return ERROR_BAD_PARAM;
	}

	ii = 0;
	while (remaining_num_word > 0)
	{
		/* build a 1D DMA descriptor */
		sp_dma31_MsetDim1_Desc_Init(
				DESC_TYPE_1D, 					/* type */
				p_local_desc,					/* ptr to desc being built */
				NULL, 							/* ptr to following desc */
				DMA_LAST_BLK, 					/* type of blk transfered */
				(uint32_t)src_addr+ii*0x100000,	/* src address */
				(uint32_t)dst_addr+ii*0x100000,	/* dst address */
				((num_128bit_word>0x10000)?(0x100000):(num_128bit_word*16)),	/* # bytes */
				ACCESS_SIZE_128BIT,				/* access size */
				BURST_16);						/* burst length */

		/* disable EOT and EOB interrupts */
		sp_dma_SetInterrupt(p_local_desc, INTR_NONE);

		curr_tx_done = 0;
		while (!curr_tx_done) {
			/* look for an available DMAC channel */
			for (kk = DBM_DMAC0; kk <= DBM_DMAC1; kk++) {
				for (jj = 0; jj < 2; jj++) {
					if (sp_dma_DoneCheck(kk, jj) == DMACHAN_NOTACTIVE) {

						sp_dma_LoadFromDescr(kk, jj, p_local_desc);

						while (sp_dma_DoneCheck(kk, jj) == DMACHAN_ACTIVE) {
							lsi_mg_delay(10);
							timeout--;
							if (timeout == 0) {
								return ERROR;
							}
						}
						curr_tx_done = 1;
						break;
					}
					if (curr_tx_done) break;
				}
				if (curr_tx_done) break;
			}
		}
		ii++;
		remaining_num_word -= 0x10000; /* - 1MB */
	}

	return (num_128bit_word*16);
}

int32_t sp_dbmdmac_MemSet128(void *dst_addr, uint32_t data32, int32_t num_128bit_word)
{
int32_t ii, jj, kk;
//int32_t timeout = num_128bit_word;
int32_t timeout = 128;
int32_t remaining_num_word = num_128bit_word;
uint32_t curr_tx_done = 0;

	dma_desc_t* p_local_desc = (dma_desc_t*)&g_dma_desc;

	/* check if both src and dst addresses are aligned */
	if (((uint32_t)dst_addr & 0xF) != 0) {
		return ERROR_BAD_PARAM;
	}
	//PRINT_STR("In sp_dbmdmac_MemSet128 (1)\r");

	ii = 0;
	while (remaining_num_word > 0)
	{
		/* build a MemAssign DMA descriptor */
		sp_dma31_MsetDim1_Desc_Init(
				DESC_TYPE_1D_MA,				/* type */
				p_local_desc,					/* ptr to desc being built */
				NULL, 							/* ptr to following desc */
				DMA_LAST_BLK, 					/* type of blk transfered */
				(uint32_t)data32,				/* data to write */
				(uint32_t)dst_addr+ii*0x100000,	/* dst address */
				((num_128bit_word>0x10000)?(0x100000):(num_128bit_word*16)),	/* # bytes */
				ACCESS_SIZE_128BIT,				/* access size */
				BURST_16);						/* burst length */

		/* disable EOT and EOB interrupts */
	//PRINT_STR("In sp_dbmdmac_MemSet128 (2)\r");
		sp_dma_SetInterrupt(p_local_desc, INTR_NONE);
	//PRINT_STR("In sp_dbmdmac_MemSet128 (3)\r");

		curr_tx_done = 0;
		while (!curr_tx_done) {
			/* look for an available DMAC channel */
			for (kk = DBM_DMAC0; kk <= DBM_DMAC1; kk++) {
				for (jj = 0; jj < 2; jj++) {
					if (sp_dma_DoneCheck(kk, jj) == DMACHAN_NOTACTIVE) {

	//PRINT_STR("In sp_dbmdmac_MemSet128 (3) Found nonactive DMA channel \r");
						sp_dma_LoadFromDescr(kk, jj, p_local_desc);
	//PRINT_STR("In sp_dbmdmac_MemSet128 (3) after Found nonactive DMA channel \r");

						while (sp_dma_DoneCheck(kk, jj) == DMACHAN_ACTIVE) {
							lsi_mg_delay(10);
							timeout--;
							if (timeout == 0) {
	//PRINT_STR("In sp_dbmdmac_MemSet128 (3) return error \r");
								return ERROR;
							}
						}
						curr_tx_done = 1;
						break;
					}
					if (curr_tx_done) break;
				}
				if (curr_tx_done) break;
			}
		}
		ii++;
		remaining_num_word -= 0x10000; /* - 1MB */
	}
	//PRINT_STR("In sp_dbmdmac_MemSet128 (4)\r");

	return (num_128bit_word*16);
}

#ifdef LSI_SP27XX_BUILT_FOR_DSS
int32_t sp_dssdmac_MemCpy128(void *dst_addr, void *src_addr, int32_t num_128bit_word)
{
int32_t ii, jj;
int32_t timeout = num_128bit_word;
int32_t remaining_num_word = num_128bit_word;
uint32_t curr_tx_done = 0;

	dma_desc_t* p_local_desc = (dma_desc_t*)&g_dma_desc;

	/* check if both src and dst addresses are aligned */
	if (((uint32_t)dst_addr & 0xF) != 0) {
		return ERROR_BAD_PARAM;
	}
	if (((uint32_t)src_addr & 0xF) != 0) {
		return ERROR_BAD_PARAM;
	}

	ii = 0;
	while (remaining_num_word > 0)
	{
		/* build a 1D DMA descriptor */
		sp_dma31_MsetDim1_Desc_Init(
				DESC_TYPE_1D, 					/* type */
				p_local_desc,					/* ptr to desc being built */
				NULL, 							/* ptr to following desc */
				DMA_LAST_BLK, 					/* type of blk transfered */
				(uint32_t)src_addr+ii*0x100000,	/* src address */
				(uint32_t)dst_addr+ii*0x100000,	/* dst address */
				((num_128bit_word>0x10000)?(0x100000):(num_128bit_word*16)),	/* # bytes */
				ACCESS_SIZE_128BIT,				/* access size */
				BURST_16);						/* burst length */

		/* disable EOT and EOB interrupts */
		sp_dma_SetInterrupt(p_local_desc, INTR_NONE);

		curr_tx_done = 0;
		while (!curr_tx_done) {
			/* look for an available DMAC channel */
			for (jj = 0; jj < 2; jj++) {
				if (sp_dma_DoneCheck(DSS_DMAC_LCL, jj) == DMACHAN_NOTACTIVE) {

					sp_dma_LoadFromDescr(DSS_DMAC_LCL, jj, p_local_desc);

					while (sp_dma_DoneCheck(DSS_DMAC_LCL, jj) == DMACHAN_ACTIVE) {
						lsi_mg_delay(10);
						timeout--;
						if (timeout == 0) {
							return ERROR;
						}
					}
					curr_tx_done = 1;
					break;
				}
				if (curr_tx_done) break;
			}
		}
		ii++;
		remaining_num_word -= 0x10000; /* - 1MB */
	}

	return (num_128bit_word*16);
}

int32_t sp_dssdmac_MemSet128(void *dst_addr, uint32_t data32, int32_t num_128bit_word)
{
int32_t ii, jj;
int32_t timeout = num_128bit_word;
int32_t remaining_num_word = num_128bit_word;
uint32_t curr_tx_done = 0;

	dma_desc_t* p_local_desc = (dma_desc_t*)&g_dma_desc;

	/* check if both src and dst addresses are aligned */
	if (((uint32_t)dst_addr & 0xF) != 0) {
		return ERROR_BAD_PARAM;
	}

	ii = 0;
	while (remaining_num_word > 0)
	{
		/* build a MemAssign DMA descriptor */
		sp_dma31_MsetDim1_Desc_Init(
				DESC_TYPE_1D_MA,				/* type */
				p_local_desc,					/* ptr to desc being built */
				NULL, 							/* ptr to following desc */
				DMA_LAST_BLK, 					/* type of blk transfered */
				(uint32_t)data32,				/* data to write */
				(uint32_t)dst_addr+ii*0x100000,	/* dst address */
				((num_128bit_word>0x10000)?(0x100000):(num_128bit_word*16)),	/* # bytes */
				ACCESS_SIZE_128BIT,				/* access size */
				BURST_16);						/* burst length */

		/* disable EOT and EOB interrupts */
		sp_dma_SetInterrupt(p_local_desc, INTR_NONE);

		curr_tx_done = 0;
		while (!curr_tx_done) {
			/* look for an available DMAC channel */
			for (jj = 0; jj < 2; jj++) {
				if (sp_dma_DoneCheck(DSS_DMAC_LCL, jj) == DMACHAN_NOTACTIVE) {

					sp_dma_LoadFromDescr(DSS_DMAC_LCL, jj, p_local_desc);

					while (sp_dma_DoneCheck(DSS_DMAC_LCL, jj) == DMACHAN_ACTIVE) {
						lsi_mg_delay(1);
						timeout--;
						if (timeout == 0) {
							return ERROR;
						}
					}
					curr_tx_done = 1;
					break;
				}
				if (curr_tx_done) break;
			}
		}
		ii++;
		remaining_num_word -= 0x10000; /* - 1MB */
	}

	return (num_128bit_word*16);
}
#endif /* LSI_SP27XX_BUILT_FOR_DSS */

/******** History ********
$Log: libdma.c,v $
Revision 1.1  2012/06/07 22:34:33  srane
Initial checkin for ECC memory test.


$Endlog$
*/

