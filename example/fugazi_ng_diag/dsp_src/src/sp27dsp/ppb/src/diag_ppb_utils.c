/* $Id: diag_ppb_utils.c,v 1.1 2012/04/18 09:44:02 srane Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/dsp_src/src/sp27dsp/ppb/src/diag_ppb_utils.c,v $
 *------------------------------------------------------------------
 * diag_ppb_utils.c
 *     Utilities 
 *
 * March 2012, Smita Rane
 *
 * Copyright (c) 2012 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */
/******************************************************************************
 * >>>>>>>>>>>>>>>>>>>>>>>>>>>>    NOTIFICATION    <<<<<<<<<<<<<<<<<<<<<<<<<<<<
 *
 * Copyright (¨) 2007 LSI Corporation
 * All Rights Reserved
 *
 * This is unpublished proprietary information of LSI Corporation. This
 * copyright notice does not evidence publication.
 *
 * The use of the software, documentation, methodologies, and other information
 * contained herein is governed solely by the associated license agreements.
 * Any inconsistent use shall be deemed to be a misappropriation of the
 * intellectual property of LSI Corporation and treated accordingly.
 * ----------------------------------------------------------------------------
 *
 * Author:		TW 9/1/2007 
 *
 * Description:	SP2600 Packet Processor - Software utilities implementation.
 *
 * $Log: 
 *
 *****************************************************************************/

/* !MANFILE:	sp_ppb_utils.c */

#include "stdint.h"
#include "stdio.h"
#include "string.h"

/******************************************************************************
StarProPPB_UTILS_!MANFUNCTION:StarProPPB_UTILS_memset8

DESCRIPTION:
	Set memory with the specified 8-bit pattern.
	
INPUT:
	p_mem	- pointer to memory
	pattern	- pattern to be set in memory
	n		- memory size
	
OUTPUT:
	none

RETURN:
	none

COMMENTS:
	none

HISTORY:
	none
*******************************************************************************/ 
void StarProPPB_UTILS_memset8(
	uint8_t 	*p_mem,
	uint8_t		pattern, 
	uint32_t 	n
	)
{
	while( n-- )
	{
		*p_mem++ = pattern;
	}
}

/******************************************************************************
StarProPPB_UTILS_!MANFUNCTION:StarProPPB_UTILS_memset32

DESCRIPTION:
	Set memory with the specified 32-bit pattern.
	
INPUT:
	p_mem	- pointer to memory
	pattern	- pattern to be set in memory
	n		- memory size
	
OUTPUT:
	none

RETURN:
	none

COMMENTS:
	none

HISTORY:
	none
*******************************************************************************/ 
void StarProPPB_UTILS_memset32(
	uint32_t 	*p_mem,
	uint32_t	pattern, 
	uint32_t 	n
	)
{
	uint32_t	size;
	
	size = n/sizeof(uint32_t);
	while( size-- )
	{
		*p_mem++ = pattern;
	}
}

/******************************************************************************
StarProPPB_UTILS_!MANFUNCTION: StarProPPB_UTILS_memcpy8

DESCRIPTION:
	
	
INPUT:
	
	none
	
OUTPUT:
	none

RETURN:
	none

COMMENTS:
	none

HISTORY:
	none
*******************************************************************************/
void StarProPPB_UTILS_memcpy8 (
	uint8_t *pdest, 
	const uint8_t *psrc, 
	uint32_t size
	)
{
	while( size-- )
	{
		*pdest++ = *psrc++; 
	}
}

/******************************************************************************
StarProPPB_UTILS_!MANFUNCTION: StarProPPB_UTILS_csum16_parcial

DESCRIPTION:
	Calculates 32-bit checksum adding 16-bit values
	
INPUT:
	sum32	- parcial 32-bit sum
	p_data	- pointer to the 16-bit aligned input data
	size	- size of the data in bytes
	
OUTPUT:
	none

RETURN:
	sum32	- 32-bit sum of 16-bit data 

COMMENTS:
	none

HISTORY:
	none
*******************************************************************************/
uint32_t StarProPPB_UTILS_csum_partial(
	uint32_t sum32,
	const uint8_t *p_data, 
	uint32_t size
	)
{
	/* Add all 16-bit values */
	while( size >= 2 ) 
	{
		sum32 += (((uint16_t) p_data[0]) << 8) | ((uint16_t) (p_data[1]));
		p_data += 2;
		size -= 2;
    }
    
    /* Add remaining 8 b-bit value */
	if( size )
	{
		sum32 += ((uint16_t) *p_data) << 8;
	}
	
    return sum32;
}

/******************************************************************************
StarProPPB_UTILS_!MANFUNCTION: StarProPPB_UTILS_csum16_fold

DESCRIPTION:
	Fold 32-bit checksum value to 16bits
	
INPUT:
	csum32	- 32-bit checksum value
	
OUTPUT:
	none

RETURN:
			- 16-bit checksum value 

COMMENTS:
	none

HISTORY:
	none
*******************************************************************************/
uint16_t StarProPPB_UTILS_csum16_fold(
	uint32_t sum32
	)
{
	uint16_t upper_16bits;

	/* fold upper 16-bits */
	while( (upper_16bits = (uint16_t) (sum32 >> 16)) != 0)
	{
		sum32 = ( sum32 & 0x0000FFFF ) + upper_16bits;
	}

    return (uint16_t) ( sum32 ^ ((uint32_t) 0x0000FFFF));
}

/******************************************************************************
StarProPPB_UTILS_!MANFUNCTION: StarProPPB_UTILS_csum16

DESCRIPTION:
	Calculates 16-bit checksum adding 16-bit values
	
INPUT:
	sum32	- parcial 32-bit sum
	p_data	- pointer to the 16-bit aligned input data
	size	- size of the data in bytes
	
OUTPUT:
	none

RETURN:
	sum32	- 32-bit sum of 16-bit data 

COMMENTS:
	none

HISTORY:
	none
*******************************************************************************/
uint16_t StarProPPB_UTILS_csum16(
	uint32_t sum32,
	const uint16_t *p_data, 
	uint32_t size
	)
{
	uint16_t upper_16bits = 0;
	uint16_t csum16;	

	/* Add all 16-bit values */
	while( size >= 2 ) 
	{
		sum32 += *p_data++;
		size -= 2;
    }
    
    /* Add remaining 8 b-bit value */
	if( size )
	{
		sum32 += *((uint8_t *) p_data);
	}
	
	/* fold upper 16-bits */
	while ((upper_16bits = (uint16_t) (sum32 >> 16)) != 0)
	{
		sum32 = ( (uint16_t) sum32 ) + upper_16bits;
	}

    csum16 = (uint16_t) ( sum32 ^ ((uint32_t) 0x0000FFFF));
    
    return csum16;
}

/******************************************************************************
StarProPPB_UTILS_!MANFUNCTION:StarProPPB_UTILS_csum32

DESCRIPTION:
	Calculate 32-bit checksum value.
	
INPUT:
	p_mem	- pointer to memory
	n		- size
	
OUTPUT:
	none

RETURN:
	none

COMMENTS:
	none

HISTORY:
	none
*******************************************************************************/ 
uint32_t StarProPPB_UTILS_csum32(
	uint32_t 	*p_mem,
	uint32_t 	n
	)
{
	register uint32_t *d = p_mem;
	uint32_t	csum=0;
    
	while( n-- )
	{
		csum += *d++;
	}

	return csum;
}

/*
 * $Log: diag_ppb_utils.c,v $
 * Revision 1.1  2012/04/18 09:44:02  srane
 * Initial checkin
 *
 *
 *------------------------------------------------------------------
 * $Endlog$
 */




