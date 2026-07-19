/* $Id: dma_desc.h,v 1.2 2017/07/28 07:58:39 harrchan Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/dsp_src/src/oakenshield_dsp/ppb/lib/dma_desc.h,v $
 *------------------------------------------------------------------
 * dma_desc.h 
 * Description: descriptor formats for SP2700 DMA31 controller 
 *
 * June 2012, Smita Rane
 *
 * Copyright (c) 2012-2017 by Cisco Systems, Inc.
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
 * dma_desc.h - descriptor formats for SP2700 DMA31 controller
 *
 *  Created on: Jun 24, 2009
 *      Author: dokim
 */

#ifndef DMA_DESC_H_
#define DMA_DESC_H_

#define DESC_TYPE_FULL	0
#define DESC_TYPE_1D	1
#define DESC_TYPE_1D_MA	2
#define DESC_TYPE_2D	3

#include "lsi_sp27xx_reg.h"

#ifndef ERROR_BAD_PARAM
#define ERROR_BAD_PARAM		0xFFFFFFFE
#endif

/* Family 00 -- Full descriptor all DMA registers loaded from descriptor */
typedef struct
{
	union
	{
		struct
		{
			unsigned long
				src_x_cnt: 16,
				src_y_cnt: 16;
		}fields;
		unsigned long reg;
	}long0;

	union
	{
		struct
		{
			unsigned long signed_src_x_mod;
		}fields;
		unsigned long reg;
	}long1;

	union
	{
		struct
		{
			unsigned long signed_src_y_mod;
		}fields;
		unsigned long reg;
	}long2;

	union
	{
		struct
		{
			unsigned long curr_src_addr;
		}fields;
		unsigned long reg;
	}long3;

	union
	{
		struct
		{
			unsigned long src_data_mask;
		}fields;
		unsigned long reg;
	}long4;

	union
	{
		struct
		{
			unsigned long
				src_burst: 3,
				src_size: 3,
				rotator_len: 5,
				tail_len: 4,
				fill0: 1,
				dst_burst: 3,
				dst_size: 3,
				fill1: 10;
		}fields;
		unsigned long reg;
	}long5;

	union
	{
		struct
		{
			unsigned long
				chan_en: 1,
				tx_en: 1,
				task_cnt_1_reset: 1,
				wait_for_task_cnt_1: 1,
				task_cnt_2_reset: 1,
				wait_for_task_cnt_2: 1,
				int_dst_eob: 1,
				int_dst_eot: 1,
				full_next_desc_addr: 1,
				fill0: 2,
				stop_dst_eob: 1,
				dma_ext_access_pri_en: 1,
				start_mem_load: 1,
				clear_fifo: 1,
				last_block: 1,
				channel_priority: 3,
				ch_full_priority: 1,
				dma_priority: 1,
				dma_priority_row: 1,
				dma_ext_priory: 1,
				src_space: 1,
				fill1: 2,
				dst_space: 1,
				fill2: 5;
		}fields;
		unsigned long reg;
	}long6;

	union
	{
		struct
		{
			unsigned long
				next_desc_family: 2,
				fill0: 1,
				next_desc_space: 1,
				next_desc_offset: 16,
				next_desc_page: 12;
		}fields;
		unsigned long reg;
	}long7;

	union
	{
		struct
		{
			unsigned long
				dst_x_cnt: 16,
				dst_y_cnt: 16;
		}fields;
		unsigned long reg;
	}long8;

	union
	{
		struct
		{
			unsigned long signed_dst_x_mod;
		}fields;
		unsigned long reg;
	}long9;

	union
	{
		struct
		{
			unsigned long signed_dst_y_mod;
		}fields;
		unsigned long reg;
	}long10;

	union
	{
		struct
		{
			unsigned long curr_dst_addr;
		}fields;
		unsigned long reg;
	}long11;
}DMA_DESC_FULL;

/* Family 01 -- block copy -> one dimensional rows without gaps
 * 'X-modifier == X access size', 'Y-counter == 0'
 * */
typedef struct
{
	union
	{
		struct
		{
			unsigned long curr_src_addr;
		}fields;
		unsigned long reg;
	}long0;

	union
	{
		struct
		{
			unsigned long curr_dst_addr;
		}fields;
		unsigned long reg;
	}long1;

	union
	{
		struct
		{
			unsigned long
				x_cnt: 16,
				tx_en: 1,
				int_dst_eob: 1,
				int_dst_eot: 1,
				stop_dst_eob: 1,
				dma_ext_access_pri_en: 1,
				start_mem_load: 1,
				last_block: 1,
				channel_priority: 3,
				ch_full_priority: 1,
				dma_priority: 1,
				dma_priority_row: 1,
				dma_ext_priory: 1,
				src_space: 1,
				dst_space: 1;
		}fields;
		unsigned long reg;
	}long2;

	union
	{
		struct
		{
			unsigned long
				next_desc_family: 2,
				fill0: 2,
				next_desc_offset: 16,
				access_size: 3,
				src_burst: 2,
				dst_burst: 2,
				task_cnt_1_reset: 1,
				wait_for_task_cnt_1: 1,
				task_cnt_2_reset: 1,
				wait_for_task_cnt_2: 1,
				fill1: 1;
		}fields;
		unsigned long reg;
	}long3;
}DMA_DESC_1D;

#ifdef DONT_USE_DMA_DESC_1D_INSTEADOF_DMA_DESC_1D_MA
/* Family 10 -- Memory Assignment -> one dimensional rows without gaps,
 * no data read from memory
 * 'X-modifier == X access size', 'Y-counter == 0'
 *
 * THIS TYPE IS REALLY A PROPER SUBSET OF DMA_DESC_1D SO WE DONT NEED THE
 * Family 10 TYPE TO BE SEPARATELY DEFINED
 *
 * */
typedef struct
{
	union
	{
		struct
		{
			unsigned long datal;
		}fields;
		unsigned long reg;
	}long0;

	union
	{
		struct
		{
			unsigned long curr_dst_addr;
		}fields;
		unsigned long reg;
	}long1;

	union
	{
		struct
		{
			unsigned long
				dst_x_cnt: 16,
				tx_en: 1,
				int_dst_eob: 1,
				int_dst_eot: 1,
				stop_dst_eob: 1,
				dma_ext_access_pri_en: 1,
				start_mem_load: 1,
				last_block: 1,
				channel_priority: 3,
				ch_full_priority: 1,
				dma_priority: 1,
				dma_priority_row: 1,
				dma_ext_priory: 1,
				fill0: 1,
				dst_space: 1;
		}fields;
		unsigned long reg;
	}long2;

	union
	{
		struct
		{
			unsigned long
				next_desc_family: 2,
				fill0: 2,
				next_desc_offset: 16,
				access_size: 3,
				fill1: 2,
				dst_burst: 2,
				task_cnt_1_reset: 1,
				wait_for_task_cnt_1: 1,
				task_cnt_2_reset: 1,
				wait_for_task_cnt_2: 1,
				fill2: 1;
		}fields;
		unsigned long reg;
	}long3;
}DMA_DESC_1D_MA;
#endif

/* Family 11 -- Simple two dimensional block transfers,
 * without masking or tail, limited X and Y modifiers */
typedef struct
{
	union
	{
		struct
		{
			unsigned long
				src_x_cnt: 16,
				src_y_cnt: 16;
		}fields;
		unsigned long reg;
	}long0;

	union
	{
		struct
		{
			unsigned long
				signed_src_x_mod: 16,
				signed_src_y_mod: 16;
		}fields;
		unsigned long reg;
	}long1;

	union
	{
		struct
		{
			unsigned long curr_src_addr;
		}fields;
		unsigned long reg;
	}long2;

	union
	{
		struct
		{
			unsigned long
				dst_x_cnt: 16,
				dst_y_cnt: 16;
		}fields;
		unsigned long reg;
	}long3;

	union
	{
		struct
		{
			unsigned long
				signed_dst_x_mod: 16,
				signed_dst_y_mod: 16;
		}fields;
		unsigned long reg;
	}long4;

	union
	{
		struct
		{
			unsigned long curr_dst_addr;
		}fields;
		unsigned long reg;
	}long5;

	union
	{
		struct
		{
			unsigned long
				dst_size: 3,
				dst_burst: 2,
				fill0: 7,
				tx_en: 1,
				task_cnt_1_reset: 1,
				wait_for_task_cnt_1: 1,
				task_cnt_2_reset: 1,
				wait_for_task_cnt_2: 1,
				int_dst_eob: 1,
				int_dst_eot: 1,
				stop_dst_eob: 1,
				dma_ext_access_pri_en: 1,
				start_mem_load: 1,
				last_block: 1,
				channel_priority: 3,
				ch_full_priority: 1,
				dma_priority: 1,
				dma_priority_row: 1,
				dma_ext_priory: 1,
				src_space: 1,
				dst_space: 1;
		}fields;
		unsigned long reg;
	}long6;

	union
	{
		struct
		{
			unsigned long
				next_desc_family: 2,
				fill0: 2,
				next_desc_offset: 16,
				ssrc_burst: 2,
				fill1: 1,
				src_size: 3,
				tail_length: 4,
				fill2: 2;
		}fields;
		unsigned long reg;
	}long7;
}DMA_DESC_2D;

typedef struct dma_desc {
	union {
		DMA_DESC_1D dim1;
		DMA_DESC_2D dim2;
		DMA_DESC_FULL full;
	} d ;
	uint32_t type_dma_desc;
	uint32_t fill[3];
#ifdef LSI_SP27XX_BUILT_FOR_DSS
} dma_desc_t;
#pragma align dma_desc_t 16
#else
} dma_desc_t __attribute__ ((aligned(16)));
#endif

#endif /* DMA_DESC_H_ */
/******** History ********
$Log: dma_desc.h,v $
Revision 1.2  2017/07/28 07:58:39  harrchan
Collapse Oakenshield-branch to Main Trunk.

Revision 1.1.2.1  2017/06/29 08:14:34  harrchan
Initial commit code for Oakenshield

Revision 1.1  2012/06/07 22:34:34  srane
Initial checkin for ECC memory test.
 

$Endlog$
*/

