/* $Id: ipmi_sprom_pfm_api.c,v 1.2 2016/04/20 08:41:36 benchen2 Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/tachi_bmc/fru/ipmi_sprom_pfm_api.c,v $
 *
 *      File:  ipmi_sprom_pfm_api.c
 *      Name:  matt strathman / Sudharshan Kadari
 *
 *      Description: Nuova sprom access api 
 *
 * Copyright (c) 2007-2016 by Cisco Systems, Inc.
 * All rights reserved.
 *
 **********************************************************************/
/**********************************************************************
@file    sprom_api.c
@author  matt strathman -- mstrathman@nuovasystems.com
@brief   nuova sprom access api 
************************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>

#include "diag_sprom.h"
#include "ipmi_sprom.h"
#include "ipmi_sprom_ops.h"


sprom_platform_t ipmi_sprom_get_platform ( void )
{
	return (ipmi_sprom_platform_probe());
}

int ipmi_sprom_get_slot_sup (sprom_platform_t platform, uint32_t *slot) 
{
	int rc = 0;
	switch (platform) {
		case	SPROM_PLATFORM_IOM:
		case	SPROM_PLATFORM_IBMC:
                case    SPROM_PLATFORM_HOST:
			*slot = 0;
			break;

		default:
			rc = -SPROM_ERR_INVALID;
			break;
	}

	return (rc);
}

int ipmi_sprom_get_slot_bp (sprom_platform_t platform, uint32_t *slot)
{
	int rc = 0;

	switch ( platform ) {
		case	SPROM_PLATFORM_IOM:
		case	SPROM_PLATFORM_IBMC:
			*slot = 0;	
			break;

		default:
			rc = -SPROM_ERR_INVALID;
			break;
	}

	return (rc);
}


int
ipmi_sprom_get_slot_chassis ( sprom_platform_t platform, uint32_t *slot)
{
	int rc = 0;

	switch (platform) {
		case	SPROM_PLATFORM_IBMC:
			*slot = 0;	
			break;

		default:
			rc = -SPROM_ERR_INVALID;
			break;
	}

	return (rc);
}

diag_sprom_t *
ipmi_sprom_init (sprom_platform_t platform, uint32_t util_type, uint32_t slot)
{
	diag_sprom_t *rc = 0;

	switch (platform) {
		case	SPROM_PLATFORM_IOM:
		case	SPROM_PLATFORM_IBMC:
                case    SPROM_PLATFORM_HOST:
		case	SPROM_UTIL_TYPE_PALO:
		case	SPROM_UTIL_TYPE_MENLO:
		case	SPROM_UTIL_TYPE_MENLO_E:
		case	SPROM_UTIL_TYPE_OPLIN:
		case	SPROM_UTIL_TYPE_MOD_HOST:
		case	SPROM_UTIL_TYPE_MEZZ:
		case	SPROM_UTIL_TYPE_NIANTIC:
		case	SPROM_UTIL_TYPE_EVEREST:
		case	SPROM_UTIL_TYPE_TIGERSHARK:
		case	SPROM_UTIL_TYPE_MONTEREYPARK:
		case	SPROM_UTIL_TYPE_NETEFFECT:
		case	SPROM_UTIL_TYPE_SCHULTZ:
		case	SPROM_UTIL_TYPE_DUBLIN:
		case	SPROM_UTIL_TYPE_FREMONT:
		case	SPROM_UTIL_TYPE_LIVERMORE:
		case	SPROM_UTIL_TYPE_BBU:
		case	SPROM_UTIL_TYPE_MEMFRONT:
		case	SPROM_UTIL_TYPE_MEMBACK:
		case	SPROM_UTIL_TYPE_TURLOCK:
		case	SPROM_UTIL_TYPE_ALCATRAZ:
		
			rc = ipmi_sprom_init_pfm (util_type, slot);
			break;

		default:
			printf("%s:%d failed\n", __func__, __LINE__);
			rc = NULL;
	}

	return (rc);
}

char * ipmi_sprom_get_name ( const diag_sprom_t *sprom ) 
{
    return (sprom->name);
}

extern int diag_get_revision();
int
ipmi_sprom_get_size ( const diag_sprom_t *sprom ) 
{
	int siz = 0;
 
	switch (sprom->type) {

		case	SPROM_UTIL_TYPE_SUP:
			siz = sizeof(sprom_ipmi_iom_t);
			break;

		case	SPROM_UTIL_TYPE_FAN:
			siz = sizeof(sprom_ipmi_iom_fan_t);
			break;

		case	SPROM_UTIL_TYPE_PS:
#ifdef DIAG_SANDIEGO
			siz = sizeof(sprom_ipmi_sd_psu_t);
#else
			siz = sizeof(sprom_ipmi_psu_t) ;
#if defined (DIAG_IOM) || (DIAG_IOM2)
			if (diag_get_revision()!=0) 
			      siz = sizeof(sprom_ipmi_iom_psu_t);
#endif
#endif	// DIAG_SANDIEGO
			break;

		case	SPROM_UTIL_TYPE_BP:
			siz = sizeof(sprom_ipmi_iom_bp_t);
			break;

		case	SPROM_UTIL_TYPE_MOD_HOST:
		case	SPROM_UTIL_TYPE_IBMC:
			siz = sizeof(sprom_ipmi_bmc_t);
			break;

		case	SPROM_UTIL_TYPE_MEZZ:
		case	SPROM_UTIL_TYPE_PALO:
		case	SPROM_UTIL_TYPE_MENLO:
		case	SPROM_UTIL_TYPE_MENLO_E:
		case	SPROM_UTIL_TYPE_OPLIN:
		case	SPROM_UTIL_TYPE_NIANTIC:
		case	SPROM_UTIL_TYPE_EVEREST:
		case	SPROM_UTIL_TYPE_TIGERSHARK:
		case	SPROM_UTIL_TYPE_NETEFFECT:
		case	SPROM_UTIL_TYPE_SCHULTZ:
		case	SPROM_UTIL_TYPE_DUBLIN:
		case	SPROM_UTIL_TYPE_FREMONT:
		case	SPROM_UTIL_TYPE_LIVERMORE:
			siz = sizeof(sprom_ipmi_mezz_t);
			break;

		case	SPROM_UTIL_TYPE_MONTEREYPARK:
			siz = sizeof(sprom_ipmi_mpark_t);
			break;

		case	SPROM_UTIL_TYPE_BBU:
			siz = sizeof(sprom_ipmi_bbu_t);
			break;

		case	SPROM_UTIL_TYPE_MEMFRONT:
			siz = sizeof(sprom_ipmi_mem_t);
			break;

		case	SPROM_UTIL_TYPE_MEMBACK:
			siz = sizeof(sprom_ipmi_mem_t);
			break;

		case	SPROM_UTIL_TYPE_TURLOCK:
			siz = sizeof(sprom_ipmi_hddbp_t);
			break;

		case	SPROM_UTIL_TYPE_ALCATRAZ:
			siz = sizeof(sprom_ipmi_bbu_t);
			break;

		default:
			siz = -SPROM_ERR_INVALID;
			break;
	}

	return (siz);
}

int
ipmi_sprom_read (diag_sprom_t * const sprom, uint8_t *rbuf ) 
{
	int rc = 0;

	// need to add a semaphore here
        if ( sprom->cache ) {
	    if (!sprom->cache_valid && sprom->cache ) {
                if ((rc = diag_sprom_rd (sprom, 0, 
		  ipmi_sprom_get_size (sprom), 
		  sprom->cache)) < 0) {
		    return ( rc );
		}    

		sprom->cache_valid = 0;
	    }
	    memcpy (rbuf, sprom->cache, ipmi_sprom_get_size ( sprom ) );
        } else { 
            if ((rc = diag_sprom_rd (sprom, 0,
                  ipmi_sprom_get_size (sprom),
                  rbuf)) < 0) {
                        return ( rc );
                }
        }

	return ( rc );
}

int
ipmi_sprom_write ( diag_sprom_t * const sprom, uint8_t *wbuf )
{
	int rc = 0;

	if ((rc = diag_sprom_wr(sprom, 0,
				ipmi_sprom_get_size (sprom), 
				wbuf)) < 0 ) {
		return ( rc );
	}

	sprom->cache_valid = 0;

	return ( 0 );
}

int ipmi_sprom_brd_serial_no_get (uint8_t type, uint8_t *pserial, uint8_t psize)
{
        int                             rc= 0, len = psize;
        uint8_t				sprom_data[1024];
        ipmi_sprom_common_header_t      *phdr;
        diag_sprom_t                    *sprom;
	ipmi_sprom_board_info_t		*pboard;
        char*                           pbuf;

        sprom = ipmi_sprom_init_pfm(type, 0);
        if (!sprom) {
                printf(" Error: unable to find sup sprom\n");
                return (-1);
        }

        rc = ipmi_sprom_read(sprom, sprom_data);
        if (rc) {
                printf(" Error: Failed to read the SPROM\n");
                return (-1);
        }

        phdr = (ipmi_sprom_common_header_t*)sprom_data;
        rc = ipmi_zero_checksum_verify((uint8_t*)phdr, sizeof(*phdr));
        if (rc) {
                printf("Error: Checksum error (rc=0x%X)\n", rc);
                strncpy(pserial, "Unknown", psize);
        } else {
                pboard = (ipmi_sprom_board_info_t*) (sprom_data + (phdr->board_info << 3));
		pbuf = (char*) &pboard->mfg_info_tl;
                pbuf += ((*pbuf & 0x3F)+1); // mfg_info
                pbuf += ((*pbuf & 0x3F)+1); // product_name

                memset(pserial, 0, psize);
                len = ((*pbuf & 0x3F) > psize) ?  psize : (*pbuf & 0x3F);
                strncpy(pserial, pbuf+1, len);
        }
        return (rc);
}

int ipmi_sprom_product_name_get (uint8_t type, uint8_t slot, uint8_t *pname, uint8_t psize)
{
        int                             rc= 0, len = psize;
        uint8_t				sprom_data[1024];
        ipmi_sprom_common_header_t      *phdr;
        diag_sprom_t                    *sprom;
	ipmi_sprom_board_info_t		*pboard;
        char*                           pbuf;

        sprom = ipmi_sprom_init_pfm(type, slot);
        if (!sprom) {
                printf(" Error: unable to find sprom (type=%d, slot=%d)\n", type, slot);
                return (-1);
        }

        rc = ipmi_sprom_read(sprom, sprom_data);
        if (rc) {
                printf(" Error: Failed to read the SPROM\n");
                return (-1);
        }

        phdr = (ipmi_sprom_common_header_t*)sprom_data;
        rc = ipmi_zero_checksum_verify((uint8_t*)phdr, sizeof(*phdr));
        if (rc) {
                printf("Error: Checksum error (rc=0x%X)\n", rc);
                strncpy(pname, "Unknown", psize);
        } else {
                pboard = (ipmi_sprom_board_info_t*) (sprom_data + (phdr->board_info << 3));
		pbuf = (char*) &pboard->mfg_info_tl;
                pbuf += ((*pbuf & 0x3F)+1); // mfg_info

                memset(pname, 0, psize);
                len = ((*pbuf & 0x3F) > psize) ?  psize : (*pbuf & 0x3F);
                strncpy(pname, pbuf+1, len);
        }
        return (rc);
}
