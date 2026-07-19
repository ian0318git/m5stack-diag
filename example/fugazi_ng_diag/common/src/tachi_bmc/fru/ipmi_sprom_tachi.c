/* $Id: ipmi_sprom_tachi.c,v 1.2 2016/04/20 08:41:35 benchen2 Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/tachi_bmc/fru/ipmi_sprom_tachi.c,v $
 *
 *      File:  sprom_bmc.c
 *      Name:  Sudharshan Kadari 
 *
 *      Description: Nuova sprom access
 *
 * Copyright (c) 1985-2016 by Cisco Systems, Inc.
 * All rights reserved.
 *
 **********************************************************************/
/********************************************************************//**
@file    sprom_bmc.c
@author  Sudharshan Kadari
@brief   Nuova Sprom Access
************************************************************************/
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>

#include "diag_sprom.h"
#include "ipmi_sprom.h"
#include "ipmi_sprom_ops.h"

// SPROM write enable/disable routines.
static void bmc_sprom_we ( void )
{
}

static void bmc_sprom_wd ( void )
{
}

/*
static void bmc_mezz_sprom_we ( void )
{
}

static void bmc_mezz_sprom_wd ( void )
{
}
*/

#define SI2C_BMC_FRU_ADDR	    0x02
#define SI2C_MEZZ_FRU_ADDR_0	0x01
#define SI2C_MEZZ_FRU_ADDR_1	0x03

#define SI2C_BMC_SDPROM_ADDR    0x10

/*********************************
 * SD Controller EEPROM
 *********************************/

static uint8_t sprom_sup_cache[sizeof(sprom_ipmi_bmc_t)];
diag_smb_dev_t ipmi_bmc_sdprom_sup_dev[] =
{
    {"/dev/i2c-0", 0, SI2C_BMC_SDPROM_ADDR, 0, 2, 1, 0},
};

diag_smb_acc_t ipmi_bmc_sdprom_acc =
{&ipmi_bmc_sdprom_sup_dev[0], sprom_rd, sprom_wr};

diag_sprom_t ipmi_bmc_sdprom[] = {
    { &ipmi_bmc_sdprom_acc, bmc_sprom_we, bmc_sprom_wd,
        "SD EEPROM", SPROM_UTIL_TYPE_SDEEPROM, 0, sprom_sup_cache },
};


/////////////////////////////////
// BMC SPROM
/////////////////////////////////
static uint8_t sprom_sup_cache[sizeof(sprom_ipmi_bmc_t)];
diag_smb_dev_t ipmi_bmc_sup_dev[] =
{
	{"/dev/i2c-0", 0, SI2C_BMC_FRU_ADDR, 0, 2, 1, 0},
};

diag_smb_acc_t ipmi_bmc_sprom_acc = 
{&ipmi_bmc_sup_dev[0], sprom_rd, sprom_wr};

diag_sprom_t ipmi_bmc_sprom[] = {
	{ &ipmi_bmc_sprom_acc, bmc_sprom_we, bmc_sprom_wd,
		"BMC", SPROM_UTIL_TYPE_IBMC, 0, sprom_sup_cache },
};


/////////////////////////////////
// TPM SPROM
/////////////////////////////////
#define SI2C_TPM_FRU_ADDR	0x06

/* TPM eeprom format is the same as sprom_ipmi_bbu_t */
static uint8_t sprom_tpm_cache[sizeof(sprom_ipmi_bbu_t)];
diag_smb_dev_t ipmi_bmc_tpm_dev[] =
{
{"/dev/i2c-1", 1, SI2C_TPM_FRU_ADDR, 0, 2, 1, 0},
};

diag_smb_acc_t ipmi_tpm_sprom_acc = 
{&ipmi_bmc_tpm_dev[0], sprom_rd, sprom_wr};

diag_sprom_t ipmi_tpm_sprom[] = {
{ &ipmi_tpm_sprom_acc, bmc_sprom_we, bmc_sprom_wd,
"ALCATRAZ", SPROM_UTIL_TYPE_ALCATRAZ, 0, sprom_tpm_cache },
};

/////////////////////////////////
// HDDBP SPROM
/////////////////////////////////
#define SI2C_HDDBP_FRU_ADDR_0	0x04
#define SI2C_HDDBP_FRU_ADDR_1	0x05
static uint8_t sprom_hddbp_cache[sizeof(sprom_ipmi_hddbp_t)];
diag_smb_dev_t ipmi_bmc_hddbp_dev[] =
{
	{"/dev/i2c-1", 1, SI2C_HDDBP_FRU_ADDR_0, 0, 2, 1, 0},
	{"/dev/i2c-1", 1, SI2C_HDDBP_FRU_ADDR_1, 0, 2, 1, 0},
};

diag_smb_acc_t ipmi_hddbp_sprom_acc[] = 
{
	{&ipmi_bmc_hddbp_dev[0], sprom_rd, sprom_wr},
	{&ipmi_bmc_hddbp_dev[1], sprom_rd, sprom_wr},
};

diag_sprom_t ipmi_hddbp_sprom[] = {
	{ &ipmi_hddbp_sprom_acc[0], NULL, NULL,
		"TURLOCK", SPROM_UTIL_TYPE_TURLOCK, 0, sprom_hddbp_cache },
	{ &ipmi_hddbp_sprom_acc[1], NULL, NULL,
		"TURLOCK", SPROM_UTIL_TYPE_TURLOCK, 0, sprom_hddbp_cache },
};

/////////////////////////////////
// MEZZ SPROM
/////////////////////////////////
static uint8_t sprom_mezz_cache[sizeof(sprom_ipmi_mezz_t)];
diag_smb_dev_t ipmi_mezz_dev[] =
{
	{"/dev/i2c-0", 0, SI2C_MEZZ_FRU_ADDR_0, 0, 2, 1, 0},
	{"/dev/i2c-0", 0, SI2C_MEZZ_FRU_ADDR_1, 0, 2, 1, 0},
};

diag_smb_acc_t ipmi_mezz_sprom_acc[] = 
{
	{&ipmi_mezz_dev[0], sprom_rd, sprom_wr},
	{&ipmi_mezz_dev[1], sprom_rd, sprom_wr}
};

diag_sprom_t ipmi_mezz_sprom[] = {
	{ &ipmi_mezz_sprom_acc[0], NULL, NULL,
		"MEZZ", SPROM_UTIL_TYPE_MEZZ, 0, sprom_mezz_cache },
	{ &ipmi_mezz_sprom_acc[1], NULL, NULL,
		"MEZZ", SPROM_UTIL_TYPE_MEZZ, 0, sprom_mezz_cache },
};

diag_sprom_t ipmi_palo_sprom[] = {
	{ &ipmi_mezz_sprom_acc[0], NULL, NULL,
		"PALO", SPROM_UTIL_TYPE_PALO, 0, sprom_mezz_cache },
	{ &ipmi_mezz_sprom_acc[1], NULL, NULL,
		"PALO", SPROM_UTIL_TYPE_PALO, 0, sprom_mezz_cache },
};

diag_sprom_t ipmi_menlo_sprom[] = {
	{ &ipmi_mezz_sprom_acc[0], NULL, NULL,
		"MENLO", SPROM_UTIL_TYPE_MENLO, 0, sprom_mezz_cache },
	{ &ipmi_mezz_sprom_acc[1], NULL, NULL,
		"MENLO", SPROM_UTIL_TYPE_MENLO, 0, sprom_mezz_cache },
};

diag_sprom_t ipmi_menloe_sprom[] = {
	{ &ipmi_mezz_sprom_acc[0], NULL, NULL,
		"MENLOE", SPROM_UTIL_TYPE_MENLO_E, 0, sprom_mezz_cache },
	{ &ipmi_mezz_sprom_acc[1], NULL, NULL,
		"MENLOE", SPROM_UTIL_TYPE_MENLO_E, 0, sprom_mezz_cache },
};

diag_sprom_t ipmi_oplin_sprom[] = {
	{ &ipmi_mezz_sprom_acc[0], NULL, NULL,
		"OPLIN", SPROM_UTIL_TYPE_OPLIN, 0, sprom_mezz_cache },
	{ &ipmi_mezz_sprom_acc[1], NULL, NULL,
		"OPLIN", SPROM_UTIL_TYPE_OPLIN, 0, sprom_mezz_cache },
};

diag_sprom_t ipmi_niantic_sprom[] = {
	{ &ipmi_mezz_sprom_acc[0], NULL, NULL,
		"NIANTIC", SPROM_UTIL_TYPE_NIANTIC, 0, sprom_mezz_cache },
	{ &ipmi_mezz_sprom_acc[1], NULL, NULL,
		"NIANTIC", SPROM_UTIL_TYPE_NIANTIC, 0, sprom_mezz_cache },
};

diag_sprom_t ipmi_neteffect_sprom[] = {
	{ &ipmi_mezz_sprom_acc[0], NULL, NULL,
		"NETEFFECT", SPROM_UTIL_TYPE_NETEFFECT, 0, sprom_mezz_cache },
	{ &ipmi_mezz_sprom_acc[1], NULL, NULL,
		"NETEFFECT", SPROM_UTIL_TYPE_NETEFFECT, 0, sprom_mezz_cache },
};

diag_sprom_t ipmi_schultz_sprom[] = {
	{ &ipmi_mezz_sprom_acc[0], NULL, NULL,
		"SCHULTZ", SPROM_UTIL_TYPE_SCHULTZ, 0, sprom_mezz_cache },
	{ &ipmi_mezz_sprom_acc[1], NULL, NULL,
		"SCHULTZ", SPROM_UTIL_TYPE_SCHULTZ, 0, sprom_mezz_cache },
};

diag_sprom_t ipmi_tigershark_sprom[] = {
	{ &ipmi_mezz_sprom_acc[0], NULL, NULL,
		"TIGERSHARK", SPROM_UTIL_TYPE_TIGERSHARK, 0, sprom_mezz_cache },
	{ &ipmi_mezz_sprom_acc[1], NULL, NULL,
		"TIGERSHARK", SPROM_UTIL_TYPE_TIGERSHARK, 0, sprom_mezz_cache },
};

diag_sprom_t ipmi_everest_sprom[] = {
	{ &ipmi_mezz_sprom_acc[0], NULL, NULL,
		"EVEREST", SPROM_UTIL_TYPE_EVEREST, 0, sprom_mezz_cache },
	{ &ipmi_mezz_sprom_acc[1], NULL, NULL,
		"EVEREST", SPROM_UTIL_TYPE_EVEREST, 0, sprom_mezz_cache },
};

diag_sprom_t ipmi_dublin_sprom[] = {
	{ &ipmi_mezz_sprom_acc[0], NULL, NULL,
		"DUBLIN", SPROM_UTIL_TYPE_DUBLIN, 0, sprom_mezz_cache },
	{ &ipmi_mezz_sprom_acc[1], NULL, NULL,
		"DUBLIN", SPROM_UTIL_TYPE_DUBLIN, 0, sprom_mezz_cache },
};

diag_sprom_t ipmi_fremont_sprom[] = {
	{ &ipmi_mezz_sprom_acc[0], NULL, NULL,
		"FREMONT", SPROM_UTIL_TYPE_FREMONT, 0, sprom_mezz_cache },
	{ &ipmi_mezz_sprom_acc[1], NULL, NULL,
		"FREMONT", SPROM_UTIL_TYPE_FREMONT, 0, sprom_mezz_cache },
};

diag_sprom_t ipmi_livermore_sprom[] = {
	{ &ipmi_mezz_sprom_acc[0], NULL, NULL,
		"LIVERMORE", SPROM_UTIL_TYPE_LIVERMORE, 0, sprom_mezz_cache },
	{ &ipmi_mezz_sprom_acc[1], NULL, NULL,
		"LIVERMORE", SPROM_UTIL_TYPE_LIVERMORE, 0, sprom_mezz_cache },
};

diag_sprom_t *
ipmi_sprom_init_pfm ( uint32_t util_type, uint32_t slot)
{
	switch(util_type) {
		case	SPROM_UTIL_TYPE_SUP:
		case	SPROM_UTIL_TYPE_IBMC:
			return (ipmi_bmc_sprom);
			break;

		case	SPROM_UTIL_TYPE_OPLIN:
			return (&ipmi_oplin_sprom[slot]);
			break;

		case	SPROM_UTIL_TYPE_MEZZ:
			return (&ipmi_mezz_sprom[slot]);
			break;

		case	SPROM_UTIL_TYPE_MENLO:
			return (&ipmi_menlo_sprom[slot]);
			break;

		case	SPROM_UTIL_TYPE_MENLO_E:
			return (&ipmi_menloe_sprom[slot]);
			break;

		case	SPROM_UTIL_TYPE_PALO:
			return (&ipmi_palo_sprom[slot]);
			break;

		case	SPROM_UTIL_TYPE_NIANTIC:
			return (&ipmi_niantic_sprom[slot]);
			break;

		case	SPROM_UTIL_TYPE_NETEFFECT:
			return (&ipmi_neteffect_sprom[slot]);
			break;

		case	SPROM_UTIL_TYPE_SCHULTZ:
			return (&ipmi_schultz_sprom[slot]);
			break;

		case	SPROM_UTIL_TYPE_TIGERSHARK:
			return (&ipmi_tigershark_sprom[slot]);
			break;

		case	SPROM_UTIL_TYPE_EVEREST:
			return (&ipmi_everest_sprom[slot]);
			break;

		case	SPROM_UTIL_TYPE_DUBLIN:
			return (&ipmi_dublin_sprom[slot]);
			break;

		case	SPROM_UTIL_TYPE_FREMONT:
			return (&ipmi_fremont_sprom[slot]);
			break;

		case	SPROM_UTIL_TYPE_LIVERMORE:
			return (&ipmi_livermore_sprom[slot]);
			break;

		case	SPROM_UTIL_TYPE_TURLOCK:
			return (&ipmi_hddbp_sprom[slot]);
			break;

		case	SPROM_UTIL_TYPE_ALCATRAZ:
			return ipmi_tpm_sprom;
			break;

		case    SPROM_UTIL_TYPE_SDEEPROM:
		    return ipmi_bmc_sdprom;
		    break;

		default:
			printf(" %s: Unknown type specified\n", __func__);
			break;
	}
	return (NULL);
}

sprom_platform_t
ipmi_sprom_platform_probe ( void )
{
	return ( SPROM_PLATFORM_IBMC );
}


int ipmi_sprom_bmc_show(diag_sprom_t* const sprom)
{
	int rc = 0;
	sprom_ipmi_bmc_t        bmc_sprom;

	rc = ipmi_sprom_read(sprom, (uint8_t *)&bmc_sprom);
	if (rc) {
		printf(" Error: Failed to read the SPROM\n");
		return (-1);
	}

	ipmi_sprom_ibmc_dump((sprom_ipmi_ibmc_t*)&bmc_sprom, IPMI_IU_CARD_TYPE_IBMC);
	return (0);
}

