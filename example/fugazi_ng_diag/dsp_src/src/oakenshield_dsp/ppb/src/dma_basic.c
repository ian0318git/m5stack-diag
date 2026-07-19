/* $Id: dma_basic.c,v 1.2 2017/07/28 07:58:51 harrchan Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/dsp_src/src/oakenshield_dsp/ppb/src/dma_basic.c,v $
 *------------------------------------------------------------------
 * dma_basic.c 
 * Description: subset of DMA operations
 *
 * June 2012, Smita Rane
 *
 * Copyright (c) 2012-2017 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

/*
 * dma_basic.c - subset of DMA operations
 *
 *  Created on: March 30, 2010
 *      Author: BAS
 */

#include "libdma.h"
#include "lsi_sp27xx_reg.h"
#include "dma_desc.h"
#include "libgeneric.h"

#define DMA_RESET_KEY	0xabcd1234

/* define 8 entries to 'force' access to a controller:
 * Low-level routines - e.g. sp_dma_pollNotActive() - access entries 0-7 without checking validity
 */
unsigned int dmac_base_addr[8] =
{
		LSI_SP27XX_DMAC31_DSS0_BASE, /* using DSS0's DMA controller */
		LSI_SP27XX_DMAC31_DSS1_BASE, /* using DSS1's DMA controller */
		LSI_SP27XX_DMAC31_DSS2_BASE, /* using DSS2's DMA controller */
		LSI_SP27XX_DMAC31_DSS3_BASE, /* using DSS3's DMA controller */
		LSI_SP27XX_DMAC31_G0_BASE,   /* using DBM DMAC controller0 */
		LSI_SP27XX_DMAC31_G1_BASE,   /* using DBM DMAC controller1 */
#ifdef LSI_SP27XX_BUILT_FOR_DSS
		LSI_SP27XX_DMAC31_DSS_BASE,	 /* For DSS local access */
		LSI_SP27XX_DMAC31_DSS_BASE,	 /* For DSS local access */
#else
		LSI_SP27XX_DMAC31_G1_BASE,   /* using DBM DMAC controller1 */
		LSI_SP27XX_DMAC31_G1_BASE,   /* using DBM DMAC controller1 */
#endif
};

uint32_t						/* ret: SUCCESS or ERROR_BAD_PARAM */
sp_dma_init (					/* initialize DMA for basic operations */
		uint32_t dmac,			/* in: DMA controller to use: DSS0_DMAC .. DBM_DMAC1 */
		uint32_t desc_page,		/* in: address of 'page' where descriptors are stored */
		uint32_t int_level) 	/* in: interrupt type: 0=>Lvl Sens., 1=>Edge-Trig. */
{
    uint32_t gcrVal;

	if (dmac > NUM_DMAC - 1) {
		return ERROR_BAD_PARAM; /* non-existing DMA controller */
	}

	/* DMA soft reset */
	REG32_WRITE(LSI_SP27XX_DMAC31_DMA_SOFTWARE_RESET_REGISTER_RA(dmac_base_addr[dmac]), DMA_RESET_KEY);

	gcrVal = (int_level == 0) ? 0 : 3;

#ifdef LSI_SP27XX_BUILT_FOR_DSS
	if (desc_page < 0x40000000) {		/* adjust cached address to DBM-accessible address */
		desc_page += 0xC0000000;
		gcrVal |= LSI_SP27XX_DMA_GENERAL_CONFIGURATION_REGISTER_DESC_SPACE_BM;
	} else if ( (desc_page >= 0x40000000) && (desc_page < 0x40040000) ) {
		/* using DSS local memory to store descriptors */
	}
	else
#endif
	{
		gcrVal |= LSI_SP27XX_DMA_GENERAL_CONFIGURATION_REGISTER_DESC_SPACE_BM;
	}
	gcrVal |= (desc_page & 0xFFF00000);

	/* wait until soft-reset register is cleared */
	while (HW_REG_ACCESS(LSI_SP27XX_DMAC31_DMA_SOFTWARE_RESET_REGISTER_RA(dmac_base_addr[dmac])) != 0) 
        ;
	REG32_WRITE(LSI_SP27XX_DMAC31_DMA_GENERAL_CONFIGURATION_REGISTER_RA(dmac_base_addr[dmac]), gcrVal);

	return SUCCESS;
}

void
sp_dma_pollXferComplete(	/* poll channel 'ch_no' of DMA controller 'dmac' until not active */
	uint32_t dmac,			/* in: DMA controller to use: DSS0_DMAC .. DBM_DMAC1 */
	uint32_t ch_no)			/* in: channel to wait for */
{
	/* wait for DMA to complete */
	while (!CHK_REG_MASK(LSI_SP27XX_DMAC31_DMA_STATUSREG_CH_RA(dmac_base_addr[dmac & 7], ch_no & 1),
			LSI_SP27XX_DMA_STATUSREG_CH_TRANSFER_COMPLETED_BM));
}

void
sp_dma_pollNotActive(		/* poll channel 'ch_no' of DMA controller 'dmac' for 'not active' */
	uint32_t dmac,			/* in: DMA controller to use: DSS0_DMAC .. DBM_DMAC1 */
	uint32_t ch_no)			/* in: channel to wait for */
{
	/* wait for DMA to complete */
	while (CHK_REG_MASK(LSI_SP27XX_DMAC31_DMA_STATUSREG_CH_RA(dmac_base_addr[dmac & 7], ch_no & 1),
			LSI_SP27XX_DMA_STATUSREG_CH_CH_ACTIVE_BM));
}



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
	uint32_t burst_size)	/* in: BURST_1, BURST_4, BURST_8 or BURST_16 */
{
	uint32_t count;
	dma_desc_t *ret_desc;

	/* first, check the DMA descriptors */
	if (((uint32_t) desc & 0x0000000F) != 0 ) {
		ret_desc = NULL;
	} else {
		if ((block != DMA_LAST_BLK) &&
                        ((uint32_t)next & 0xFFF00000UL) != ((uint32_t)desc & 0xFFF00000UL) ) {
		    /* for Compact descriptors, the descriptors must be within same 20-bit page */
		    ret_desc = NULL;
		} else {
			ret_desc = desc;
		}
	}

	/* The DMA31 does not support any kind of write access that is not aligned to
	 * the write access size.
	 */
	switch (access_size ) {
	case ACCESS_SIZE_128BIT:
		if ( (dst_addr & 0x0000000F) != 0 ) ret_desc = NULL;
		break;
	case ACCESS_SIZE_64BIT:
		if ( (dst_addr & 0x00000007) != 0 ) ret_desc = NULL;
		break;
	case ACCESS_SIZE_32BIT:
		if ( (dst_addr & 0x00000003) != 0 ) ret_desc = NULL;
		break;
	case ACCESS_SIZE_16BIT:
		if ( (dst_addr & 0x00000001) != 0 ) ret_desc = NULL;
		break;
	case ACCESS_SIZE_8BIT:		/* all address access are valid */
		break;
	default:					/* invalid argument */
		ret_desc = NULL;
	}

	/* calculate number of transfers */
	count = (byte_cnt >> access_size);
	if (count > 0x10000) {
		/* DMA31 can only support 2^16 transfers */
		ret_desc = NULL;
	}

	if (ret_desc != NULL) {
		/* set source and dest addresses */
		ret_desc->d.dim1.long0.reg = src_val;
		ret_desc->d.dim1.long1.reg = dst_addr;

		ret_desc->d.dim1.long2.reg = 0;	/* default value */
		ret_desc->d.dim1.long2.fields.x_cnt = count - 1;
		ret_desc->d.dim1.long2.fields.tx_en = 1;
		ret_desc->d.dim1.long2.fields.last_block = block & 1;
		ret_desc->d.dim1.long2.fields.int_dst_eot = 1;

		ret_desc->d.dim1.long2.fields.src_space = 1; /* DMA global address space */
		ret_desc->d.dim1.long2.fields.dst_space = 1; /* DMA global address space */
#if LSI_SP27XX_BUILT_FOR_DSS
		/* If this is a DSS_LMEM access, use the proper port */
		if((((uint32_t)dst_addr)&0xF0000000) == 0x40000000) {
			ret_desc->d.dim1.long2.fields.dst_space = 0;
		}

		/* the following is a "don't care" for the mset case since the is no src_spcae */
		if((((uint32_t)src_val)&0xF0000000) == 0x40000000) {
			ret_desc->d.dim1.long2.fields.src_space = 0;
		}
#endif
		ret_desc->d.dim1.long3.reg = 0;	/* default value */
		ret_desc->d.dim1.long3.fields.access_size = access_size;
		ret_desc->d.dim1.long3.fields.dst_burst = burst_size & 3;
		ret_desc->d.dim1.long3.fields.src_burst = burst_size & 3;
		if (block != DMA_LAST_BLK) {
			ret_desc->d.dim1.long3.fields.next_desc_offset = (uint32_t) next >> 4;
			ret_desc->d.dim1.long3.fields.next_desc_family = next->type_dma_desc;
		}
		ret_desc->type_dma_desc = type & 3;
	}

	return(ret_desc);
}

/* ret: SUCCESS or ERROR_BAD_PARAM */
uint32_t sp_dma_LoadFromDescr (	/* start channel with descriptor pointer */
	uint32_t dmac,		        /* in: DMA controller to use: DSS0_DMAC .. DBM_DMAC1 */
	uint32_t ch_no,		        /* in: channel to set up */
	dma_desc_t *p_next_descr)   /* in: initialized descriptor, e.g. see: sp_dma31_MsetDim1_Desc_Init() */
{
    uint32_t descVal = (uint32_t) p_next_descr;
    uint32_t nextDescSpace = (descVal & 0x40000000) == 0x40000000 ? 0 : LSI_SP27XX_DMA_NEXTDESCPTR_CH_NEXT_DESC_SPACE_BM ;

	if((unsigned int)dmac>NUM_DMAC-1) {
		return ERROR_BAD_PARAM; 		/* non-existing DMA controller */
	} else if((unsigned int)ch_no > NUM_DMAC_CH-1) {
		return ERROR_BAD_PARAM;
	}

#if LSI_SP27XX_BUILT_FOR_DSS
	if(dmac<DSS_DMAC_LCL)
	{
		/* if DSS uses DBM DMAC controller */
		/* DBM DMAC controller can only see memory space in global memory map's perspective */
		/* if p_next_descr is in DSS core's local memory */
		if((descVal&0x40000000) == 0x40000000) {
			descVal = (descVal&0x3FFFF)|(0x80000000+((sp_read_cpuid())<<25));
		} else if ((descVal>0x0)&&(descVal<0x800000)) {
		    /* if p_next_descr is in Sysmem memory (looked through DCC's bypass) */
			descVal = (descVal&0x7FFFFF)|0xC0000000;
		}
		nextDescSpace = LSI_SP27XX_DMA_NEXTDESCPTR_CH_NEXT_DESC_SPACE_BM;
	}
#endif

	sp_dma_pollNotActive(dmac, ch_no);

	if (p_next_descr->type_dma_desc != DESC_TYPE_FULL) {
		if ( (HW_REG_ACCESS(LSI_SP27XX_DMAC31_DMA_GENERAL_CONFIGURATION_REGISTER_RA(dmac_base_addr[dmac])) &
				LSI_SP27XX_DMA_GENERAL_CONFIGURATION_REGISTER_DESC_PAGE_BM) !=
					(descVal & LSI_SP27XX_DMA_GENERAL_CONFIGURATION_REGISTER_DESC_PAGE_BM) ) {
			/* descriptor is not on the page indicated by the DMAC's General Configuration reg. */
			return ERROR_BAD_PARAM;
		}
		REG32_WRITE(LSI_SP27XX_DMAC31_DMA_NEXTDESCPTR_CH_RA(dmac_base_addr[dmac], ch_no),
			((uint32_t) p_next_descr & LSI_SP27XX_DMA_NEXTDESCPTR_CH_NEXTDESCRIPTORADDRESS_BM) | nextDescSpace |
				(p_next_descr->type_dma_desc & LSI_SP27XX_DMA_NEXTDESCPTR_CH_NEXTDESCRIPTORFAMILY_BM) );
	} else {
		REG32_WRITE(LSI_SP27XX_DMAC31_DMA_NEXTDESCPTR_CH_RA(dmac_base_addr[dmac], ch_no),
			((uint32_t) p_next_descr & 0xFFFFFFF0) | nextDescSpace | DESC_TYPE_FULL);
	}

	/* clear any  status bits leftover from previous transfer */
	REG32_SET_BITS(LSI_SP27XX_DMAC31_DMA_STATUSREG_CH_RA(dmac_base_addr[dmac], ch_no),
			LSI_SP27XX_DMA_STATUSREG_CH_CH_ERROR_ACC_DL_BM | LSI_SP27XX_DMA_STATUSREG_CH_CH_ERROR_ACC_DST_BM |
			LSI_SP27XX_DMA_STATUSREG_CH_CH_ERROR_ACC_SRC_BM | LSI_SP27XX_DMA_STATUSREG_CH_BLOCK_COMPLETED_BM |
			LSI_SP27XX_DMA_STATUSREG_CH_TRANSFER_COMPLETED_BM );

	/* write 1 to clear fifo bit, start_mem_load and chan_en */
	REG32_SET_BITS(LSI_SP27XX_DMAC31_DMA_CONFREG_CH_RA(dmac_base_addr[dmac], ch_no),
			LSI_SP27XX_DMA_CONFREG_CH_CLEAR_FIFO_BM | LSI_SP27XX_DMA_CONFREG_CH_START_MEM_LOAD_BM |
			LSI_SP27XX_DMA_CONFREG_CH_CHAN_EN_BM);

	return (SUCCESS);
}

/******** History ********
$Log: dma_basic.c,v $
Revision 1.2  2017/07/28 07:58:51  harrchan
Collapse Oakenshield-branch to Main Trunk.

Revision 1.1.2.1  2017/06/29 08:14:38  harrchan
Initial commit code for Oakenshield

Revision 1.2  2012/08/15 14:52:23  srane
cleanup code.

Revision 1.1  2012/06/07 22:34:29  srane
Initial checkin for ECC memory test.
 

$Endlog$
*/

