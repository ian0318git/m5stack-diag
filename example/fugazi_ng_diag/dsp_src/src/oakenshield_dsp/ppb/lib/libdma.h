/* $Id: libdma.h,v 1.2 2017/07/28 07:58:48 harrchan Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/dsp_src/src/oakenshield_dsp/ppb/lib/libdma.h,v $
 *------------------------------------------------------------------
 * libdma.h 
 * Description: basic setup routines for SP2700 DMAC31 controllers 
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
 * libdma.h - basic setup routines for SP2700 DMAC31 controllers
 *
 *  Created on: Oct 16, 2009
 *      Author: dokim
 */

#ifndef LIBDMA_H_
#define LIBDMA_H_

#include <stdint.h>
#include <stdlib.h>
#include "dma_desc.h"

#define DSS0_DMAC 0 		/* using DSS0's DMA controller */
#define DSS1_DMAC 1 		/* using DSS1's DMA controller */
#define DSS2_DMAC 2 		/* using DSS2's DMA controller */
#define DSS3_DMAC 3 		/* using DSS3's DMA controller */
#define DBM_DMAC0 4   		/* using DBM DMAC controller0 */
#define DBM_DMAC1 5   		/* using DBM DMAC controller1 */
#define DSS_DMAC_LCL  6   	/* for DSS accessing its DMAC only (not PPB!) */

#ifdef LSI_SP27XX_BUILT_FOR_DSS
#define NUM_DMAC 7
#else
#define NUM_DMAC 6
#endif

#define DMA_CH0	0
#define DMA_CH1 1
#define NUM_DMAC_CH 2

#define ACCESS_SIZE_128BIT	 4
#define ACCESS_SIZE_64BIT	 3
#define ACCESS_SIZE_32BIT	 2
#define ACCESS_SIZE_16BIT	 1
#define ACCESS_SIZE_8BIT	 0

#define FULL_ROTATOR_128BIT		16
#define FULL_ROTATOR_64BIT		8
#define FULL_ROTATOR_32BIT		4
#define FULL_ROTATOR_16BIT		2
#define FULL_ROTATOR_8BIT		1

#define FULL_BURST_16			7
#define FULL_BURST_8			5
#define FULL_BURST_4			3
#define FULL_BURST_1			0

#define BURST_16		3
#define BURST_8			2
#define BURST_4			1
#define BURST_1			0

#define COMPACT_BURST_16		BURST_16
#define COMPACT_BURST_8			BURST_8
#define COMPACT_BURST_4			BURST_4
#define COMPACT_BURST_1			BURST_1

/* enumeration values  based on the bit positions in the DMAx_STATUS register - don't modify */
typedef enum DMAC_INTR {
	INTR_NONE = 0,
	INTR_EOB = 1,
	INTR_EOT = 2,
	INTR_BOTH = 3
} DMAC_INTR_t;

/* return values for operations on a single DMAC channel
 * NOTE: a 'paused' channel is also an active channel
 * */
typedef enum DMACHANSTAT {
	DMACHAN_ACTIVE = 0,
	DMACHAN_NOTACTIVE = 1,
	DMACHAN_ERROR = 2,
	DMACHAN_PARAM_ERROR = ERROR_BAD_PARAM
} DMACHANSTAT_t;

#define DMA_INTR_LEVEL			0
#define DMA_INTR_EDGE			1

#define DMA_NOT_LAST_BLK	0
#define DMA_LAST_BLK		1
#define DMA_FIRST_BLK		2

extern dma_desc_t g_dma_desc;

/* Functions for handling DMA interrupts */

DMACHANSTAT_t					/* ret: DMACHAN_ERROR if channel encountered error */
sp_dma_StatusClr(				/* clear all pending status register bits, if set */
	uint32_t dmac,				/* in: DSS<0-3>_DMAC, DBM_DMAC<0-1>, DSS_DMAC_LCL (DSS only) */
	uint32_t ch_no);			/* in: channel to check (0 or 1) */

DMACHANSTAT_t					/* ret: DMACHAN_ERROR if channel encountered error */
sp_dma_IdentifyInterrupt(		/* determine reason for DMAC interrupt */
	uint32_t dmac,				/* in: DSS<0-3>_DMAC, DBM_DMAC<0-1>, DSS_DMAC_LCL (DSS only) */
	uint32_t ch_no,				/* in: channel to check (0 or 1) */
	DMAC_INTR_t *type);			/* out: INTR_EOB, INTR_EOT, INTR_BOTH or INTR_NONE */

/* Functions that provide Basic DMA control methods */
uint32_t						/* ret: SUCCESS or ERROR_BAD_PARAM */
sp_dma_init (					/* initialize DMA for basic operations */
		uint32_t dmac,			/* in: DMA controller to use: DSS0_DMAC .. DBM_DMAC1 */
		uint32_t desc_page,		/* in: address of 'page' where descriptors are stored */
		uint32_t int_level);	/* in: interrupt type: 0=>Lvl Sens., 1=>Edge-Trig. */

dma_desc_t *				/* ret: address of 'desc' input for success or NULL for failure */
sp_dma31_MsetDim1_Desc_Init(/* One dimensional & memory assignment DMA descriptor initialization */
	uint32_t type,			/* in: DESC_TYPE_1D_MA (mset) or DESC_TYPE_1D (1dim) */
	dma_desc_t *desc,		/* out: area in memory to format as DMA descriptor */
	dma_desc_t *next,		/* in: pointer to next descriptor unless block is DMA_LAST_BLK */
	uint32_t block,			/* in: indicates DMA_LAST_BLK, DMA_NOT_LAST_BLK or DMA_FIRST_BLK of xfer */
	uint32_t src_val,		/* in: start address of source block or value to write */
	uint32_t dst_addr,		/* in: start address of destination block (MUST BE aligned to access_size) */
	uint32_t byte_cnt,		/* in: total transfer size in bytes */
	uint32_t access_size,	/* in: ACCESS_SIZE_8BIT, ACCESS_SIZE_<16|32|64|128>BIT (MUST match dst_addr) */
	uint32_t burst_size);	/* in: BURST_1, BURST_4, BURST_8 or BURST_16 */

uint32_t						/* ret: SUCCESS or ERROR_BAD_PARAM */
sp_dma_LoadFromDescr(			/* start channel with descriptor pointer */
	uint32_t dmac,				/* in: DMA controller to use: DSS0_DMAC .. DBM_DMAC1 */
	uint32_t ch_no,				/* in: channel to set up */
	dma_desc_t *p_next_descr);	/* in: initialized descriptor, see: ag_mg_dma30_Desc_Init() */

void
sp_dma_pollXferComplete(	/* poll channel 'ch_no' of DMA controller 'dmac' for transfer complete */
	uint32_t dmac,			/* in: DMA controller to use: DSS0_DMAC .. DBM_DMAC1 */
	uint32_t ch_no);		/* in: channel to wait for */

void
sp_dma_pollNotActive(		/* poll channel 'ch_no' of DMA controller 'dmac' for 'not active' */
	uint32_t dmac,			/* in: DMA controller to use: DSS0_DMAC .. DBM_DMAC1 */
	uint32_t ch_no);				/* in: channel to wait for */

/* Functions to init/link/unlink DMA descriptors */

dma_desc_t *				/* ret: address of 'desc' input for success or NULL for failure */
sp_dma_SetInterrupt(		/* set up for DMAC interrupt */
	dma_desc_t *desc,		/* in: DMA descriptor to update */
	DMAC_INTR_t type);		/* in: INTR_EOB, INTR_EOT, INTR_BOTH or INTR_NONE */

int32_t						/* ret: SUCCESS or ERROR_BAD_PARAM */
sp_dma_descInit (			/* initialize dma descriptor with dflt values */
	dma_desc_t* p_desc);	/* in: dma descriptor to be initiaized */

int32_t						/* ret: SUCCESS or ERROR_BAD_PARAM */
sp_dma_descLink (			/* connect two dma descriptors "p_tail" and "p_head" so the p_tail follows p_head */
	dma_desc_t* p_head,		/* in: leading dma desc */
	dma_desc_t* p_tail);	/* in: following dma_desc */

int32_t						/* ret: SUCCESS or ERROR_BAD_PARAM */
sp_dma_descUnlink (			/* cut the connection between p_desc and any other descriptors that follow p_desc */
	dma_desc_t* p_desc);	/* in: leading dma_desc */

DMACHANSTAT_t				/* ret: DMACHAN_ERROR if channel encountered error */
sp_dma_Recover(				/* reclaim a channel (even if it's currently running) */
	uint32_t dmac,			/* in: DSS<0-3>_DMAC, DBM_DMAC<0-1>, DSS_DMAC_LCL (DSS only) */
	uint32_t ch_no);		/* in: channel to recover (0 or 1) */

DMACHANSTAT_t				/* ret: DMACHAN_ERROR if channel encountered error */
sp_dma_Disable(				/* disable a running channel */
	uint32_t dmac,			/* in: DSS<0-3>_DMAC, DBM_DMAC<0-1>, DSS_DMAC_LCL (DSS only) */
	uint32_t ch_no);		/* in: channel to check (0 or 1) */

DMACHANSTAT_t				/* ret: DMACHAN_ERROR if channel encountered error */
sp_dma_Pause(				/* pause a running channel */
	uint32_t dmac,			/* in: DSS<0-3>_DMAC, DBM_DMAC<0-1>, DSS_DMAC_LCL (DSS only) */
	uint32_t ch_no);		/* in: channel to check (0 or 1) */

DMACHANSTAT_t 				/* ret: SUCCESS or ERROR_FATA */
sp_dma_Resume(				/* resuming ch if it's paused after a transfer */
	uint32_t dmac, 			/* in: DMA controller that is hanging */
	uint32_t ch_no);		/* in: DMA channel that is hanging */

DMACHANSTAT_t				/* ret: 0: DMACHAN_ACTIVE, else none-zero (e.g. DMACHAN_ERROR) */
sp_dma_DoneCheck(			/* check if a DMA operation is still running on the channel */
	uint32_t dmac,			/* in: DSS<0-3>_DMAC, DBM_DMAC<0-1>, DSS_DMAC_LCL (DSS only) */
	uint32_t ch_no);		/* in: channel to check (0 or 1) */

uint32_t						/* ret: SUCCESS or ERROR_BAD_PARAM */
sp_dma_DebugResume(				/* Allow DMAC channel to continue to run when in Debug mode */
	uint32_t dmac);

int32_t 					/* ret: SUCCESS or ERROR_BAD_PARAM */
sp_dma_DescToHWReg(			/* directly fill out dma h/w registers with the parameters in p_desc */
	uint32_t dmac, 			/* in: DMA controller to be accessed */
	uint32_t ch_no, 		/* in: DMA channel to be accessed */
	dma_desc_t* p_desc);	/* in: p_desc from which parameters will be extracted from */

/* Some useful examples of using functions defined in this LIB */
int32_t 					/* ret: #bytes transferred if success or ERROR_BAD_PARAM */
sp_dbmdmac_MemCpy128(		/* memcpy multiple of 128bits from and to 128bit aligned area using DBM DMAC*/
	void *dst_addr, 		/* in: ptr to dst buffer */
	void *src_addr, 		/* in: ptr to src buffer */
	int32_t num_128bit_word);/* in: # 16 byte-words */

int32_t 					/* ret: #bytes written if success or ERROR_BAD_PARAM */
sp_dbmdmac_MemSet128(		/* memset multiple of 128bits from and to 128bit aligned area using DBM DMAC*/
	void *dst_addr, 		/* in: ptr to area that will be filled up with a constant */
	uint32_t data32, 		/* in: constant to be written on dst area */
	int32_t num_128bit_word);/* in: # 16 byte-words */

#ifdef LSI_SP27XX_BUILT_FOR_DSS

int32_t						/* ret: #bytes transferred if success or ERROR_BAD_PARAM */
sp_dssdmac_MemCpy128(		/* memcpy multiple of 128bits from and to 128bit aligned area using DSS DMAC*/
	void *dst_addr, 		/* in: ptr to dst buffer */
	void *src_addr, 		/* in: ptr to src buffer */
	int32_t num_128bit_word);/* in: # 16 byte-words */

int32_t						/* ret: #bytes written if success or ERROR_BAD_PARAM */
sp_dssdmac_MemSet128(		/* memset multiple of 128bits from and to 128bit aligned area using DSS DMAC*/
	void *dst_addr, 		/* in: ptr to area that will be filled up with a constant */
	uint32_t data32, 		/* in: constant to be written on dst area */
	int32_t num_128bit_word);/* in: # 16 byte-words */

#endif  /* LSI_SP27XX_BUILT_FOR_DSS */
#endif	/* LIBDMA_H_ */

/******** History ********
$Log: libdma.h,v $
Revision 1.2  2017/07/28 07:58:48  harrchan
Collapse Oakenshield-branch to Main Trunk.

Revision 1.1.2.1  2017/06/29 08:14:35  harrchan
Initial commit code for Oakenshield

Revision 1.1  2012/06/07 22:34:33  srane
Initial checkin for ECC memory test.
 

$Endlog$
*/

