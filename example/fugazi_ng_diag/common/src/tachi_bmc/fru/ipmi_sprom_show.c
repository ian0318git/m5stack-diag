/* $Id: ipmi_sprom_show.c,v 1.2 2016/04/20 08:41:37 benchen2 Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/tachi_bmc/fru/ipmi_sprom_show.c,v $
 * Copyright (c) 2016 by Cisco Systems, Inc.
 * All rights reserved.
 */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>
#include <stdint.h>
#include "diag_sprom.h"
#include "ipmi_sprom.h"
#include "ipmi_sprom_ops.h"

#define INDENT_PER_LEVEL (2)

extern int diag_get_revision();
static void
ipmi_sprom_show_sup ( diag_sprom_t *sprom, uint8_t *buf )
{
    ipmi_sprom_iom_dump(buf);
    return;
}

static void
ipmi_sprom_show_lc ( diag_sprom_t *sprom, uint8_t *buf )
{
//    line_card_sprom_t* lc_sprom = ( line_card_sprom_t* ) buf;
    return;
}

static void
ipmi_sprom_show_bp ( diag_sprom_t *sprom, uint8_t *buf )
{
    ipmi_sprom_iom_chassis_dump((sprom_ipmi_iom_bp_t *)buf);
    return;
}

static void
ipmi_sprom_show_ps ( diag_sprom_t *sprom, uint8_t *buf )
{
#ifdef DIAG_SANDIEGO
	ipmi_sprom_sd_psu_dump((sprom_ipmi_sd_psu_t*)buf);
#else
#if defined(DIAG_IOM) || defined(DIAG_IOM2)
    if (diag_get_revision()!=0) 
    	ipmi_sprom_iom_psu_dump ((sprom_ipmi_iom_psu_t *)buf);
    else
#endif
    	ipmi_sprom_psu_dump ((sprom_ipmi_psu_t *)buf);
#endif	// DIAG_SANDIEGO
    return;
}


static void
ipmi_sprom_show_fan( diag_sprom_t *sprom, uint8_t *buf)
{
    ipmi_sprom_iom_fan_dump ((sprom_ipmi_iom_fan_t *)buf);
    return;
}

static void
ipmi_sprom_show_palo ( diag_sprom_t *sprom, uint8_t *buf )
{
    ipmi_sprom_mezz_dump((sprom_ipmi_mezz_t *)buf,
				IPMI_IU_CARD_TYPE_PALO);
    return;
}

static void
ipmi_sprom_show_mezz ( diag_sprom_t *sprom, uint8_t *buf )
{
    ipmi_sprom_mezz_dump((sprom_ipmi_mezz_t *)buf,
				IPMI_IU_CARD_TYPE_UNKNOWN);
    return;
}

static void
ipmi_sprom_show_bmc ( diag_sprom_t *sprom, uint8_t *buf )
{
    ipmi_sprom_ibmc_dump((sprom_ipmi_bmc_t *)buf, IPMI_IU_CARD_TYPE_IBMC);
    return;
}

static void
ipmi_sprom_show_bbu ( diag_sprom_t *sprom, uint8_t *buf )
{
    ipmi_sprom_ibmc_dump((sprom_ipmi_bmc_t *)buf, IPMI_IU_CARD_TYPE_BBU);
    return;
}

static void
ipmi_sprom_show_turlock ( diag_sprom_t *sprom, uint8_t *buf )
{
    ipmi_sprom_hddbp_dump((sprom_ipmi_hddbp_t *)buf, IPMI_IU_CARD_TYPE_TURLOCK);
    return;
}

static void
ipmi_sprom_show_alcatraz ( diag_sprom_t *sprom, uint8_t *buf )
{
    ipmi_sprom_bbu_dump((sprom_ipmi_bbu_t *)buf);
    return;
}

static void
ipmi_sprom_show_menlo ( diag_sprom_t *sprom, uint8_t *buf )
{
    ipmi_sprom_mezz_dump((sprom_ipmi_mezz_t *)buf,
				IPMI_IU_CARD_TYPE_MENLO);
    return;
}
static void
ipmi_sprom_show_menlo_e ( diag_sprom_t *sprom, uint8_t *buf )
{
    ipmi_sprom_mezz_dump((sprom_ipmi_mezz_t *)buf,
				IPMI_IU_CARD_TYPE_MENLO_E);
    return;
}

static void
ipmi_sprom_show_oplin ( diag_sprom_t *sprom, uint8_t *buf )
{
    ipmi_sprom_mezz_dump((sprom_ipmi_mezz_t *)buf,
				IPMI_IU_CARD_TYPE_OPLIN);
    return;
}


static void
ipmi_sprom_show_mpark ( diag_sprom_t *sprom, uint8_t *buf )
{
    ipmi_sprom_mpark_dump ((sprom_ipmi_mpark_t *)buf,
				IPMI_IU_CARD_TYPE_MONTEREYPARK);
    return;
}

static void
ipmi_sprom_show_niantic ( diag_sprom_t *sprom, uint8_t *buf )
{
    ipmi_sprom_mezz_dump((sprom_ipmi_mezz_t *)buf,
				IPMI_IU_CARD_TYPE_NIANTIC);
    return;
}

static void
ipmi_sprom_show_neteffect ( diag_sprom_t *sprom, uint8_t *buf )
{
    ipmi_sprom_mezz_dump((sprom_ipmi_mezz_t *)buf,
				IPMI_IU_CARD_TYPE_NETEFFECT);
    return;
}

static void
ipmi_sprom_show_schultz ( diag_sprom_t *sprom, uint8_t *buf )
{
    ipmi_sprom_mezz_dump((sprom_ipmi_mezz_t *)buf,
				IPMI_IU_CARD_TYPE_SCHULTZ);
    return;
}

static void
ipmi_sprom_show_tigershark ( diag_sprom_t *sprom, uint8_t *buf )
{
    ipmi_sprom_mezz_dump((sprom_ipmi_mezz_t *)buf,
				IPMI_IU_CARD_TYPE_TIGERSHARK);
    return;
}

static void
ipmi_sprom_show_everest ( diag_sprom_t *sprom, uint8_t *buf )
{
    ipmi_sprom_mezz_dump((sprom_ipmi_mezz_t *)buf,
				IPMI_IU_CARD_TYPE_EVEREST);
    return;
}

static void
ipmi_sprom_show_dublin ( diag_sprom_t *sprom, uint8_t *buf )
{
    ipmi_sprom_mezz_dump((sprom_ipmi_mezz_t *)buf,
				IPMI_IU_CARD_TYPE_DUBLIN);
    return;
}

static void
ipmi_sprom_show_fremont ( diag_sprom_t *sprom, uint8_t *buf )
{
    ipmi_sprom_mezz_dump((sprom_ipmi_mezz_t *)buf,
				IPMI_IU_CARD_TYPE_FREMONT);
    return;
}

static void
ipmi_sprom_show_livermore ( diag_sprom_t *sprom, uint8_t *buf )
{
    ipmi_sprom_mezz_dump((sprom_ipmi_mezz_t *)buf,
				IPMI_IU_CARD_TYPE_LIVERMORE);
    return;
}

static void
ipmi_sprom_show_memfront ( diag_sprom_t *sprom, uint8_t *buf )
{
    ipmi_sprom_mem_dump((sprom_ipmi_mem_t *)buf, IPMI_IU_CARD_TYPE_MEMFRONT);
    return;
}

static void
ipmi_sprom_show_memback ( diag_sprom_t *sprom, uint8_t *buf )
{
    ipmi_sprom_mem_dump((sprom_ipmi_mem_t *)buf, IPMI_IU_CARD_TYPE_MEMBACK);
    return;
}

int
ipmi_sprom_show ( diag_sprom_t *sprom, uint8_t * const buf )
{
    int rc = 0;

    assert ( sprom );
    assert ( buf );

    sprom_printf ( "Showing SPROM %s\n", ipmi_sprom_get_name ( sprom ) );

    if ( ( rc = ipmi_sprom_verify ( sprom, buf ) ) < 0 ) {
        if ( rc == -SPROM_ERR_INVALID ) {
            sprom_printf ( "\n" );
            sprom_printf ( "** Warning Warning Warning **\n" );
            sprom_printf ( "** The sprom contents are invalid.\n" );
            sprom_printf ( "** Please re-program the sprom.\n" );
            sprom_printf ( "** Warning Warning Warning **\n" );
            sprom_printf ( "\n" );
            sprom_printf ( "Showing the contains anyway.\n\n" );
        } else {
            sprom_printf ( "%s(): diag_sprom_is_valid() failed.\n", 
              __FUNCTION__ );
            goto sprom_show_exit;
        }
    }

    switch ( sprom->type ) {
    case SPROM_UTIL_TYPE_SUP:
        ipmi_sprom_show_sup ( sprom, buf );
        break;

    case SPROM_UTIL_TYPE_LC:
        ipmi_sprom_show_lc ( sprom, buf );
        break;

    case SPROM_UTIL_TYPE_BP:
        ipmi_sprom_show_bp ( sprom, buf );
        break;

    case SPROM_UTIL_TYPE_PS:
        ipmi_sprom_show_ps ( sprom, buf );
        break;

    case SPROM_UTIL_TYPE_FAN:
        ipmi_sprom_show_fan ( sprom, buf );
        break;

    case SPROM_UTIL_TYPE_MENLO_E:
        ipmi_sprom_show_menlo_e ( sprom, buf );
        break;

    case SPROM_UTIL_TYPE_MENLO:
        ipmi_sprom_show_menlo ( sprom, buf );
        break;

    case SPROM_UTIL_TYPE_OPLIN:
        ipmi_sprom_show_oplin ( sprom, buf );
        break;

    case SPROM_UTIL_TYPE_PALO:
        ipmi_sprom_show_palo ( sprom, buf );
        break;

    case SPROM_UTIL_TYPE_MEZZ:
        ipmi_sprom_show_mezz ( sprom, buf );
        break;

    case SPROM_UTIL_TYPE_MONTEREYPARK:
        ipmi_sprom_show_mpark ( sprom, buf );
        break;

    case SPROM_UTIL_TYPE_NIANTIC:
        ipmi_sprom_show_niantic ( sprom, buf );
        break;

    case SPROM_UTIL_TYPE_NETEFFECT:
        ipmi_sprom_show_neteffect ( sprom, buf );
        break;

    case SPROM_UTIL_TYPE_SCHULTZ:
        ipmi_sprom_show_schultz ( sprom, buf );
        break;

    case SPROM_UTIL_TYPE_TIGERSHARK:
        ipmi_sprom_show_tigershark ( sprom, buf );
        break;

    case SPROM_UTIL_TYPE_EVEREST:
        ipmi_sprom_show_everest ( sprom, buf );
        break;

    case SPROM_UTIL_TYPE_DUBLIN:
        ipmi_sprom_show_dublin ( sprom, buf );
        break;

    case SPROM_UTIL_TYPE_FREMONT:
        ipmi_sprom_show_fremont ( sprom, buf );
        break;

    case SPROM_UTIL_TYPE_LIVERMORE:
        ipmi_sprom_show_livermore ( sprom, buf );
        break;

    case SPROM_UTIL_TYPE_MOD_HOST:
    case SPROM_UTIL_TYPE_IBMC:
        ipmi_sprom_show_bmc ( sprom, buf );
        break;

    case SPROM_UTIL_TYPE_BBU:
        ipmi_sprom_show_bbu ( sprom, buf );
        break;

    case SPROM_UTIL_TYPE_TURLOCK:
        ipmi_sprom_show_turlock ( sprom, buf );
        break;

    case SPROM_UTIL_TYPE_ALCATRAZ:
	ipmi_sprom_show_alcatraz ( sprom, buf );
	break;
	
    case SPROM_UTIL_TYPE_MEMFRONT:
        ipmi_sprom_show_memfront ( sprom, buf );
        break;

    case SPROM_UTIL_TYPE_MEMBACK:
        ipmi_sprom_show_memback ( sprom, buf );
        break;

    default:
        rc = -SPROM_ERR_INVALID;
        goto sprom_show_exit;
    }

    if ( rc == -SPROM_ERR_INVALID) {
        sprom_printf ( "\n" );
        sprom_printf ( "** Warning Warning Warning **\n" );
        sprom_printf ( "** The above sprom contents are invalid.\n" );
        sprom_printf ( "** Please re-program the sprom.\n" );
        sprom_printf ( "** Warning Warning Warning **\n" );
        sprom_printf ( "\n" );
    }


sprom_show_exit:
    return ( rc );
}

