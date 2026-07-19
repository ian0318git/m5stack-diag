/* $Id: ipmi_sprom_set.c,v 1.2 2016/04/20 08:41:37 benchen2 Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/tachi_bmc/fru/ipmi_sprom_set.c,v $
 *
 *      File:   sprom_set.c
 *      Name:   Curt Brune
 *
 *      Description:
 *       sprom set routines - writes the sprom
 *
 * Copyright (c) 1985-2016 by Cisco Systems, Inc.
 * All rights reserved.
 *
 ********************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include "diag_sprom.h"
#include "ipmi_sprom.h"
#include "ipmi_sprom_ops.h"

int
ipmi_sprom_confirm_set (const diag_sprom_t* sprom )
{   
    char         buffer;
    int          rtn_val;

    sprom_printf ("\n");
    sprom_printf ("Program %s SPROM", ipmi_sprom_get_name( sprom ) );
    sprom_printf ( " with the above information (y/n) [n]? "); 
    buffer = getchar();
    while( getchar() != '\n' )
        ;
    
    if (buffer == 'y' || buffer == 'Y') {
        rtn_val = 1;
    }
    else {
        sprom_printf("Aborting SPROM modification ...\n");
        rtn_val = 0;
    }

    return rtn_val;
}

static int
_ipmi_fill_sup( diag_sprom_t* const sprom, uint8_t *buf )
{
	return(ipmi_sprom_iom_fill((sprom_ipmi_iom_t*)buf));
}

static int
_ipmi_fill_bp( diag_sprom_t* const sprom, uint8_t *buf )
{
	return(ipmi_sprom_iom_bp_fill((sprom_ipmi_iom_bp_t*)buf));
}

extern int diag_get_revision();
static int
_ipmi_fill_ps( diag_sprom_t* const sprom, uint8_t *buf )
{
#ifdef DIAG_SANDIEGO
	return (ipmi_sprom_sd_psu_fill((sprom_ipmi_sd_psu_t*)buf));
#else

#if defined(DIAG_IOM) || defined(DIAG_IOM2)
	if (diag_get_revision()!=0)
		return (ipmi_sprom_iom_psu_fill((sprom_ipmi_iom_psu_t*)buf));
#endif
	return (ipmi_sprom_psu_fill((sprom_ipmi_psu_t*)buf));

#endif	// DIAG_SANDIEGO
}

static int
_ipmi_fill_fan( diag_sprom_t* const sprom, uint8_t *buf )
{
	return(ipmi_sprom_iom_fan_fill((sprom_ipmi_iom_fan_t*)buf));
}

static int
_ipmi_fill_bmc( diag_sprom_t* const sprom, uint8_t *buf )
{
        return(ipmi_sprom_ibmc_fill((sprom_ipmi_bmc_t*)buf,
                        IPMI_IU_CARD_TYPE_IBMC));
}

static int
_ipmi_fill_mpark( diag_sprom_t* const sprom, uint8_t *buf )
{
	return(ipmi_sprom_mpark_fill((sprom_ipmi_mpark_t*)buf, 
			IPMI_IU_CARD_TYPE_MONTEREYPARK));
}

static int
_ipmi_fill_niantic( diag_sprom_t* const sprom, uint8_t *buf )
{
	return(ipmi_sprom_mezz_fill((sprom_ipmi_mezz_t*)buf, 
			IPMI_IU_CARD_TYPE_NIANTIC));
}

static int
_ipmi_fill_neteffect( diag_sprom_t* const sprom, uint8_t *buf )
{
	return(ipmi_sprom_mezz_fill((sprom_ipmi_mezz_t*)buf, 
			IPMI_IU_CARD_TYPE_NETEFFECT));
}

static int
_ipmi_fill_schultz( diag_sprom_t* const sprom, uint8_t *buf )
{
	return(ipmi_sprom_mezz_fill((sprom_ipmi_mezz_t*)buf, 
			IPMI_IU_CARD_TYPE_SCHULTZ));
}

static int
_ipmi_fill_tigershark( diag_sprom_t* const sprom, uint8_t *buf )
{
	return(ipmi_sprom_mezz_fill((sprom_ipmi_mezz_t*)buf, 
			IPMI_IU_CARD_TYPE_TIGERSHARK));
}

static int
_ipmi_fill_everest( diag_sprom_t* const sprom, uint8_t *buf )
{
	return(ipmi_sprom_mezz_fill((sprom_ipmi_mezz_t*)buf, 
			IPMI_IU_CARD_TYPE_EVEREST));
}

static int
_ipmi_fill_dublin( diag_sprom_t* const sprom, uint8_t *buf )
{
	return(ipmi_sprom_mezz_fill((sprom_ipmi_mezz_t*)buf, 
			IPMI_IU_CARD_TYPE_DUBLIN));
}

static int
_ipmi_fill_fremont( diag_sprom_t* const sprom, uint8_t *buf )
{
	return(ipmi_sprom_mezz_fill((sprom_ipmi_mezz_t*)buf, 
			IPMI_IU_CARD_TYPE_FREMONT));
}

static int
_ipmi_fill_livermore( diag_sprom_t* const sprom, uint8_t *buf )
{
	return(ipmi_sprom_mezz_fill((sprom_ipmi_mezz_t*)buf, 
			IPMI_IU_CARD_TYPE_LIVERMORE));
}

static int
_ipmi_fill_palo( diag_sprom_t* const sprom, uint8_t *buf )
{
	return(ipmi_sprom_mezz_fill((sprom_ipmi_mezz_t*)buf, 
			IPMI_IU_CARD_TYPE_PALO));
}

static int
_ipmi_fill_mezz( diag_sprom_t* const sprom, uint8_t *buf )
{
	return(ipmi_sprom_mezz_fill((sprom_ipmi_mezz_t*)buf, 
			IPMI_IU_CARD_TYPE_UNKNOWN));
}

static int
_ipmi_fill_menlo( diag_sprom_t* const sprom, uint8_t *buf )
{
	return(ipmi_sprom_mezz_fill((sprom_ipmi_mezz_t*)buf, 
			IPMI_IU_CARD_TYPE_MENLO));
}
static int
_ipmi_fill_menlo_e( diag_sprom_t* const sprom, uint8_t *buf )
{
	return(ipmi_sprom_mezz_fill((sprom_ipmi_mezz_t*)buf, 
			IPMI_IU_CARD_TYPE_MENLO_E));
}
static int
_ipmi_fill_oplin( diag_sprom_t* const sprom, uint8_t *buf )
{
	return(ipmi_sprom_mezz_fill((sprom_ipmi_mezz_t*)buf, 
			IPMI_IU_CARD_TYPE_OPLIN));
}
static int
_ipmi_fill_bbu( diag_sprom_t* const sprom, uint8_t *buf )
{
	return(ipmi_sprom_bbu_fill((sprom_ipmi_bbu_t*)buf, 
			IPMI_IU_CARD_TYPE_BBU));
}
static int
_ipmi_fill_memfront( diag_sprom_t* const sprom, uint8_t *buf )
{
	return(ipmi_sprom_mem_fill((sprom_ipmi_mem_t*)buf, 
			IPMI_IU_CARD_TYPE_MEMFRONT));
}
static int
_ipmi_fill_memback( diag_sprom_t* const sprom, uint8_t *buf )
{
	return(ipmi_sprom_mem_fill((sprom_ipmi_mem_t*)buf, 
			IPMI_IU_CARD_TYPE_MEMBACK));
}
static int
_ipmi_fill_turlock( diag_sprom_t* const sprom, uint8_t *buf )
{
	return(ipmi_sprom_hddbp_fill((sprom_ipmi_hddbp_t*)buf, 
			IPMI_IU_CARD_TYPE_TURLOCK));
}

static int
_ipmi_fill_alcatraz( diag_sprom_t* const sprom, uint8_t *buf )
{
	return(ipmi_sprom_bbu_fill((sprom_ipmi_bbu_t*)buf, 0)); // CARD_TYPE not used
}

static int 
_ipmi_fill_sprom ( diag_sprom_t * const sprom, uint8_t *buf )
{
    int rc = 0;

    switch( sprom->type) {
    case SPROM_UTIL_TYPE_LC:
	break;

    case SPROM_UTIL_TYPE_SUP:
	rc = _ipmi_fill_sup( sprom, buf );
	break;
    case SPROM_UTIL_TYPE_BP:
	rc = _ipmi_fill_bp( sprom, buf );
	break;
    case SPROM_UTIL_TYPE_PS:
	rc = _ipmi_fill_ps( sprom, buf );
	break;
    case SPROM_UTIL_TYPE_FAN:
	rc = _ipmi_fill_fan( sprom, buf );
	break;

    case SPROM_UTIL_TYPE_MOD_HOST:
    case SPROM_UTIL_TYPE_IBMC:
	rc = _ipmi_fill_bmc(sprom, buf);
	break;

    case SPROM_UTIL_TYPE_MONTEREYPARK:
	rc = _ipmi_fill_mpark(sprom, buf);
	break;

    case SPROM_UTIL_TYPE_NIANTIC:
	rc = _ipmi_fill_niantic(sprom, buf);
	break;

    case SPROM_UTIL_TYPE_SCHULTZ:
	rc = _ipmi_fill_schultz(sprom, buf);
	break;

    case SPROM_UTIL_TYPE_NETEFFECT:
	rc = _ipmi_fill_neteffect(sprom, buf);
	break;

    case SPROM_UTIL_TYPE_TIGERSHARK:
	rc = _ipmi_fill_tigershark(sprom, buf);
	break;

    case SPROM_UTIL_TYPE_EVEREST:
	rc = _ipmi_fill_everest(sprom, buf);
	break;

    case SPROM_UTIL_TYPE_DUBLIN:
	rc = _ipmi_fill_dublin(sprom, buf);
	break;

    case SPROM_UTIL_TYPE_FREMONT:
	rc = _ipmi_fill_fremont(sprom, buf);
	break;

    case SPROM_UTIL_TYPE_LIVERMORE:
	rc = _ipmi_fill_livermore(sprom, buf);
	break;

    case SPROM_UTIL_TYPE_PALO:
	rc = _ipmi_fill_palo(sprom, buf);
	break;

    case SPROM_UTIL_TYPE_MENLO_E:
	rc = _ipmi_fill_menlo_e(sprom, buf);
	break;

    case SPROM_UTIL_TYPE_MENLO:
	rc = _ipmi_fill_menlo(sprom, buf);
	break;

    case SPROM_UTIL_TYPE_OPLIN:
	rc = _ipmi_fill_oplin(sprom, buf);
	break;

    case SPROM_UTIL_TYPE_BBU:
	rc = _ipmi_fill_bbu(sprom, buf);
	break;

    case SPROM_UTIL_TYPE_MEMFRONT:
	rc = _ipmi_fill_memfront(sprom, buf);
	break;

    case SPROM_UTIL_TYPE_MEMBACK:
	rc = _ipmi_fill_memback(sprom, buf);
	break;

    case SPROM_UTIL_TYPE_TURLOCK:
	rc = _ipmi_fill_turlock(sprom, buf);
	break;

    case SPROM_UTIL_TYPE_ALCATRAZ:
	rc = _ipmi_fill_alcatraz(sprom, buf);
	break;

    case SPROM_UTIL_TYPE_MEZZ:
	rc = _ipmi_fill_mezz(sprom, buf);
	break;

    default:
	break;
    };

    return rc;
}


static int 
_ipmi_dflt_sprom ( diag_sprom_t * const sprom, uint8_t *buf )
{
    int rc = 0;

    switch( sprom->type) {
    case SPROM_UTIL_TYPE_LC:
	break;

    case SPROM_UTIL_TYPE_SUP:
	rc = ipmi_sprom_iom_dflt((sprom_ipmi_iom_t*)buf);
	break;

    case SPROM_UTIL_TYPE_BP:
	rc = ipmi_sprom_iom_bp_dflt((sprom_ipmi_iom_bp_t*)buf);
	break;

    case SPROM_UTIL_TYPE_PS:
#ifdef DIAG_SANDIEGO
	rc = ipmi_sprom_sd_psu_dflt((sprom_ipmi_sd_psu_t*)buf);
#else
	rc = (diag_get_revision()==0) ? 
		ipmi_sprom_iom_psu_p0_dflt((sprom_ipmi_iom_psu_p0_t*)buf) :
		ipmi_sprom_iom_psu_dflt((sprom_ipmi_iom_psu_t*)buf);
#endif
	break;

    case SPROM_UTIL_TYPE_FAN:
	rc = ipmi_sprom_iom_fan_dflt((sprom_ipmi_iom_fan_t*)buf);
	break;

    case SPROM_UTIL_TYPE_MOD_HOST:
    case SPROM_UTIL_TYPE_IBMC:
#ifdef DIAG_VENTURA
        return (ipmi_sprom_ventura_dflt((sprom_ipmi_ibmc_t*)buf));
#endif
#ifdef DIAG_SF
        return (ipmi_sprom_sf_dflt((sprom_ipmi_ibmc_t*)buf));
#endif
#ifdef DIAG_CERRITOS
	return (ipmi_sprom_cerritos_dflt((sprom_ipmi_ibmc_t*)buf));
#endif
#ifdef DIAG_MARIN
	return (ipmi_sprom_marin_dflt((sprom_ipmi_ibmc_t*)buf));
#endif
#ifdef DIAG_CASTLEROCK
	return (ipmi_sprom_castlerock_dflt((sprom_ipmi_ibmc_t*)buf));
#endif	
#ifdef DIAG_LA
        return (ipmi_sprom_la_dflt((sprom_ipmi_ibmc_t*)buf));
#elif defined(DIAG_ALPINE)
        rc = ipmi_sprom_alpine_dflt((sprom_ipmi_ibmc_t*)buf);
#elif defined(DIAG_SANDIEGO)
        rc = ipmi_sprom_sandiego_dflt((sprom_ipmi_ibmc_t*)buf);
#else
        rc = ipmi_sprom_bmc_dflt((sprom_ipmi_ibmc_t*)buf);
#endif
        break;

    case SPROM_UTIL_TYPE_PALO:
	rc = ipmi_sprom_mezz_dflt((sprom_ipmi_mezz_t*)buf, 
					IPMI_IU_CARD_TYPE_PALO);
	break;

    case SPROM_UTIL_TYPE_MEZZ:
	rc = ipmi_sprom_mezz_dflt((sprom_ipmi_mezz_t*)buf, 
					IPMI_IU_CARD_TYPE_UNKNOWN);
	break;

    case SPROM_UTIL_TYPE_MENLO_E:
	rc = ipmi_sprom_mezz_dflt((sprom_ipmi_mezz_t*)buf, 
				IPMI_IU_CARD_TYPE_MENLO_E);
	break;

    case SPROM_UTIL_TYPE_MENLO:
	rc = ipmi_sprom_mezz_dflt((sprom_ipmi_mezz_t*)buf, 
				IPMI_IU_CARD_TYPE_MENLO);
	break;

    case SPROM_UTIL_TYPE_OPLIN:
	rc = ipmi_sprom_mezz_dflt((sprom_ipmi_mezz_t*)buf, 
			IPMI_IU_CARD_TYPE_OPLIN);
	break;

    case SPROM_UTIL_TYPE_MONTEREYPARK:
	rc = ipmi_sprom_mpark_dflt((sprom_ipmi_mpark_t*)buf, 
			IPMI_IU_CARD_TYPE_MONTEREYPARK);
	break;

    case SPROM_UTIL_TYPE_NIANTIC:
	rc = ipmi_sprom_mezz_dflt((sprom_ipmi_mezz_t*)buf, 
			IPMI_IU_CARD_TYPE_NIANTIC);
	break;

    case SPROM_UTIL_TYPE_SCHULTZ:
	rc = ipmi_sprom_mezz_dflt((sprom_ipmi_mezz_t*)buf, 
			IPMI_IU_CARD_TYPE_SCHULTZ);
	break;

    case SPROM_UTIL_TYPE_NETEFFECT:
	rc = ipmi_sprom_mezz_dflt((sprom_ipmi_mezz_t*)buf, 
			IPMI_IU_CARD_TYPE_NETEFFECT);
	break;

    case SPROM_UTIL_TYPE_TIGERSHARK:
	rc = ipmi_sprom_mezz_dflt((sprom_ipmi_mezz_t*)buf, 
			IPMI_IU_CARD_TYPE_TIGERSHARK);
	break;

    case SPROM_UTIL_TYPE_EVEREST:
	rc = ipmi_sprom_mezz_dflt((sprom_ipmi_mezz_t*)buf, 
			IPMI_IU_CARD_TYPE_EVEREST);
	break;

    case SPROM_UTIL_TYPE_DUBLIN:
	rc = ipmi_sprom_mezz_dflt((sprom_ipmi_mezz_t*)buf, 
			IPMI_IU_CARD_TYPE_DUBLIN);
	break;

    case SPROM_UTIL_TYPE_FREMONT:
	rc = ipmi_sprom_mezz_dflt((sprom_ipmi_mezz_t*)buf, 
			IPMI_IU_CARD_TYPE_FREMONT);
	break;

    case SPROM_UTIL_TYPE_LIVERMORE:
	rc = ipmi_sprom_mezz_dflt((sprom_ipmi_mezz_t*)buf, 
			IPMI_IU_CARD_TYPE_LIVERMORE);
	break;

    case SPROM_UTIL_TYPE_BBU:
	rc = ipmi_sprom_bbu_dflt((sprom_ipmi_bbu_t*)buf);
	break;

    case SPROM_UTIL_TYPE_TURLOCK:
	rc = ipmi_sprom_castlerock_hddbp_dflt((sprom_ipmi_hddbp_t*)buf);
	break;

    case SPROM_UTIL_TYPE_ALCATRAZ:
	rc = ipmi_sprom_castlerock_tpm_dflt((sprom_ipmi_bbu_t*)buf);
	break;
	
    case SPROM_UTIL_TYPE_MEMFRONT:
        rc = ipmi_sprom_marin_memfront_dflt((sprom_ipmi_mem_t *)buf);
        break;

    case SPROM_UTIL_TYPE_MEMBACK:
        rc = ipmi_sprom_marin_memback_dflt((sprom_ipmi_mem_t *)buf);
        break;

    default:
	break;
    };

    return rc;
}

/*
** General set sprom entry point, for all SPROM types
*/
int 
ipmi_sprom_set ( diag_sprom_t * const sprom )
{
    int rc = 0, sprom_size;
    uint8_t *rwbuf;

    sprom_size = ( ipmi_sprom_get_size ( sprom ) );

    rwbuf = malloc ( sprom_size); 
    if ( rwbuf == NULL ) {
       return ( -SPROM_ERR_NOMEM );
    }

    if ( ( rc = ipmi_sprom_read ( sprom, rwbuf ) ) < 0 ){
        goto sprom_set_exit;
    }

    // Check if the SPROM is ever initialized.
    rc = ipmi_header_checksum_verify(rwbuf); 
    if (rc) {
	printf("Header Check sum verification failed\n");
	// SPROM Data is invalid. So clear everything.
	memset(rwbuf, 0, sprom_size);

	// Load SPROM information with Default
    	if ( ( rc = _ipmi_dflt_sprom( sprom, rwbuf ) ) < 0 ) {
        	goto sprom_set_exit;
    	}
    }
    if ( ( rc = _ipmi_fill_sprom( sprom, rwbuf ) ) < 0 ) {
        goto sprom_set_exit;
    }

    if ( ( rc = ipmi_sprom_show ( sprom, rwbuf ) ) < 0 ) {
        goto sprom_set_exit;
    }
    if ( ipmi_sprom_confirm_set ( sprom ) ) {
        if ( ( rc = ipmi_sprom_write ( sprom, rwbuf ) ) < 0 ) {
            sprom_printf ( "Unable to write sprom contents.\n" );
            goto sprom_set_exit;
        }
    }
sprom_set_exit:
    free(rwbuf);

    return rc;
}

/*
** Update mac_base and mac_length information. Only valid
** for SUP, LC and backplane SPROM type.
*/
int 
ipmi_sprom_set_mac ( diag_sprom_t * const sprom )
{
    int rc = 0;
    uint8_t *rwbuf, *piu, card_type;
    ipmi_sprom_common_header_t *pcmnhdr;

    // first fetch current contents
    rwbuf = malloc ( ipmi_sprom_get_size ( sprom ) );
    if ( rwbuf == NULL ) {
        return ( -SPROM_ERR_NOMEM );
    }

    if ( ( rc = ipmi_sprom_read ( sprom, rwbuf ) ) < 0 ){
        goto sprom_set_mac_exit;
    }

    pcmnhdr = (ipmi_sprom_common_header_t *)rwbuf;

    piu = (uint8_t*)( rwbuf + ( pcmnhdr->internal_use << 3 ) );
    switch(sprom->type) {
	case 	SPROM_UTIL_TYPE_FAN:
		card_type = IPMI_IU_CARD_TYPE_SC_FAN;
		break;

	case 	SPROM_UTIL_TYPE_SUP:
#if defined (DIAG_IOM2)
            card_type = IPMI_IU_CARD_TYPE_IOM2;
#else
            card_type = IPMI_IU_CARD_TYPE_IOM;
#endif
		break;

	case 	SPROM_UTIL_TYPE_BP:
		card_type = IPMI_IU_CARD_TYPE_SC_BP;
		break;

	case 	SPROM_UTIL_TYPE_PS:
		card_type = IPMI_IU_CARD_TYPE_SC_PSU;
		break;

	case 	SPROM_UTIL_TYPE_PALO:
		card_type = IPMI_IU_CARD_TYPE_PALO;
		break;

	case 	SPROM_UTIL_TYPE_MENLO:
		card_type = IPMI_IU_CARD_TYPE_MENLO;
		break;

	case 	SPROM_UTIL_TYPE_OPLIN:
		card_type = IPMI_IU_CARD_TYPE_OPLIN;
		break;

	case 	SPROM_UTIL_TYPE_IBMC:
		card_type = IPMI_IU_CARD_TYPE_IBMC;
		break;

	case 	SPROM_UTIL_TYPE_MENLO_E:
		card_type = IPMI_IU_CARD_TYPE_MENLO_E;
		break;

	case	SPROM_UTIL_TYPE_MONTEREYPARK:
		card_type = IPMI_IU_CARD_TYPE_MONTEREYPARK;
		break;

	case	SPROM_UTIL_TYPE_NIANTIC:
		card_type = IPMI_IU_CARD_TYPE_NIANTIC;
		break;

	case	SPROM_UTIL_TYPE_EVEREST:
		card_type = IPMI_IU_CARD_TYPE_EVEREST;
		break;

	case	SPROM_UTIL_TYPE_TIGERSHARK:
		card_type = IPMI_IU_CARD_TYPE_TIGERSHARK;
		break;

	case	SPROM_UTIL_TYPE_NETEFFECT:
		card_type = IPMI_IU_CARD_TYPE_NETEFFECT;
		break;

	case	SPROM_UTIL_TYPE_SCHULTZ:
		card_type = IPMI_IU_CARD_TYPE_SCHULTZ;
		break;

	case	SPROM_UTIL_TYPE_DUBLIN:
		card_type = IPMI_IU_CARD_TYPE_DUBLIN;
		break;

	case	SPROM_UTIL_TYPE_FREMONT:
		card_type = IPMI_IU_CARD_TYPE_FREMONT;
		break;

	case	SPROM_UTIL_TYPE_LIVERMORE:
		card_type = IPMI_IU_CARD_TYPE_LIVERMORE;
		break;

	case	SPROM_UTIL_TYPE_BBU:
		card_type = IPMI_IU_CARD_TYPE_BBU;
		break;

	case	SPROM_UTIL_TYPE_MEZZ:
		card_type = IPMI_IU_CARD_TYPE_UNKNOWN;
		break;

	default:
		printf("  Invalid type %d\n", sprom->type);
		return rc;
		break;

    }
    ipmi_sprom_internal_use_create (piu, card_type);

    // ask if OK to write?
    if ( ipmi_sprom_confirm_set ( sprom ) ) {
        // write new data back if OK
        if ( ( rc = ipmi_sprom_write ( sprom, rwbuf ) ) < 0 ) {
            ipmi_sprom_err ( "Unable to write sprom contents.\n" );
            rc = SPROM_ERR_IO;
            goto sprom_set_mac_exit;
        }
    }

 sprom_set_mac_exit:
    free(rwbuf);

    return rc;
}

/*
** Update license information in backplane License block.  Only valid
** for backplane SPROM type.
*/
int
ipmi_sprom_set_lic ( diag_sprom_t * const sprom )
{
    printf("%s:%d CODE NOT IMPLEMENTED ******\n", __func__, __LINE__);
    return 0;
}

/*
** Update mac_base and mac_length information. Only valid
** for SUP, LC and backplane SPROM type.
*/
int 
ipmi_sprom_set_serial ( diag_sprom_t * const sprom )
{
    int rc = 0;
#if 0
    uint8_t *rwbuf;
    ipmi_sprom_common_header_t *pcmnhdr;

    // first fetch current contents
    rwbuf = malloc ( ipmi_sprom_get_size ( sprom ) );
    if ( rwbuf == NULL ) {
        return ( -SPROM_ERR_NOMEM );
    }

    if ( ( rc = ipmi_sprom_read ( sprom, rwbuf ) ) < 0 ){
        goto sprom_set_mac_exit;
    }

    pcmnhdr = (ipmi_sprom_common_header_t *)rwbuf;

    ipmi_sprom_board_info_set_serial((ipmi_sprom_board_info_t*)(pcmnhdr->board_info << 3));

    // ask if OK to write?
    if ( ipmi_sprom_confirm_set ( sprom ) ) {
        // write new data back if OK
        if ( ( rc = ipmi_sprom_write ( sprom, rwbuf ) ) < 0 ) {
            ipmi_sprom_err ( "Unable to write sprom contents.\n" );
            rc = SPROM_ERR_IO;
            goto sprom_set_mac_exit;
        }
    }

 sprom_set_mac_exit:
    free(rwbuf);
#endif
    return rc;
}

/*
** Update fru file id 
*/
int 
ipmi_sprom_set_fru_file_id ( diag_sprom_t * const sprom )
{
    int rc = 0;
    uint8_t *rwbuf, *pprod, *pboard;
    ipmi_sprom_common_header_t *pcmnhdr;

    // first fetch current contents
    rwbuf = malloc ( ipmi_sprom_get_size ( sprom ) );
    if ( rwbuf == NULL ) {
        return ( -SPROM_ERR_NOMEM );
    }

    if ( ( rc = ipmi_sprom_read ( sprom, rwbuf ) ) < 0 ){
        goto sprom_set_mac_exit;
    }

    pcmnhdr = (ipmi_sprom_common_header_t *)rwbuf;
    if (pcmnhdr->board_info == 0) return (0);
    pboard = (uint8_t*)( rwbuf + ( pcmnhdr->board_info << 3 ) );
    ipmi_sprom_board_fru_file_id_create (pboard);

    if (pcmnhdr->product_info == 0) return (0);
    pprod = (uint8_t*)( rwbuf + ( pcmnhdr->product_info << 3 ));
    ipmi_sprom_product_fru_file_id_create (pprod);

    if ( ( rc = ipmi_sprom_verify ( sprom, rwbuf ) ) < 0 ) {
	printf(" SPROM Verif Failed\n");
	return (0);
    } else {
	printf(" SPROM Verify Passed\n");
    }
    // ask if OK to write?
    if ( ipmi_sprom_confirm_set ( sprom ) ) {
        // write new data back if OK
        if ( ( rc = ipmi_sprom_write ( sprom, rwbuf ) ) < 0 ) {
            ipmi_sprom_err ( "Unable to write sprom contents.\n" );
            rc = SPROM_ERR_IO;
            goto sprom_set_mac_exit;
        }
    }

 sprom_set_mac_exit:
    free(rwbuf);

    return rc;
}
