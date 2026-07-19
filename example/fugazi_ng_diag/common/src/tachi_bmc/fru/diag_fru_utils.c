/* $Id: diag_fru_utils.c,v 1.2 2016/04/20 08:41:35 benchen2 Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/tachi_bmc/fru/diag_fru_utils.c,v $
 * Copyright (c) 2016 by Cisco Systems, Inc.
 * All rights reserved.
 */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "ipmi_sprom_ops.h"
#include "diag_sprom.h"

#define FLASH_SEED	0xAB0DE007

extern int  ppc_i2c_open(int);
extern void ppc_i2c_close(int);

uint32_t gen_fru_acc_diag(uint8_t  slot, 
			  uint8_t  platform, 
			  uint8_t  type,
			  uint32_t fru_prg_size,
			  uint32_t pages,
			  uint8_t  extended)
{
	uint8_t  *psprom = NULL;
	diag_sprom_t *dsprom = NULL;
	int rc = 0, fd = -1;
	uint8_t data[128];
	uint32_t seed = 0, cnt=0, pgcnt;
	uint32_t *pdata = (uint32_t*)data;
	uint8_t  i2c_bus, i2c_addr;
	uint32_t start_addr;

	// Verify SPROM
	dsprom = ipmi_sprom_init(platform, type, slot);
	if (!dsprom) {
		printf("Error: sprom information failed\n");
		return (-1);
	}

	i2c_bus  = dsprom->smb_acc->smb_dev->path;
	i2c_addr = dsprom->smb_acc->smb_dev->addr;
	start_addr = dsprom->smb_acc->smb_dev->offset;

	if (pages > 512) pages = 512;
	fd = ppc_i2c_open(i2c_bus);
	if(fd < 0) {
		printf(" Error: unable to open i2c%d\n", i2c_bus);
		return (-2);
	} 

	if (fru_prg_size) {
		psprom = (char*)malloc(fru_prg_size);
		if (!psprom) {
			return (-3);
		}

                rc = sprom_rd (dsprom->smb_acc->smb_dev,
                                dsprom->smb_acc->smb_dev->offset,
                                fru_prg_size, psprom);
		if (rc) {
			ppc_i2c_close(fd);
			free(psprom);
			return(-1);
		} else {
			ipmi_sprom_show(dsprom, psprom);
			rc = ipmi_fru_verify(psprom, fru_prg_size);
			if (rc) {
				printf("Error: Sprom is not programmed correctly\n");
				rc = -1;
			}
		}
	}

	if (!rc && extended) {
		for (pgcnt = 0; pgcnt < pages; pgcnt++) {
			// Fill Data.
			srand(FLASH_SEED + pgcnt);
			for (cnt=0; cnt < (sizeof(data)/sizeof(uint32_t));cnt++) {
				*(pdata+cnt) = rand();
			}
			if ((pgcnt &0x3F) == 0x00)
				printf(" Writing Page %d @ 0x%X\n", 
					pgcnt, pgcnt*sizeof(data));
			rc = diag_sprom_wr (dsprom, (uint32_t)(pgcnt*sizeof(data)),
					 (uint32_t)sizeof(data), data);
			if (rc) {
				printf(" SEEPROM write failed rc=%d\n", rc);
				ppc_i2c_close(fd);
				if(fru_prg_size) free(psprom);
				return(-1);
			}
		}

		for (pgcnt = 0; pgcnt < pages; pgcnt++) {
			// Fill Data.
			rc = diag_sprom_rd(dsprom, (uint32_t)(pgcnt*sizeof(data)),
					 (uint32_t)sizeof(data), data);
			if (rc) {
				printf(" SEEPROM read failed Addr=0x%X rc=%d\n",
					 pgcnt*sizeof(data), rc);
				ppc_i2c_close(fd);
				if(fru_prg_size) free(psprom);
				return(-1);
			}
			if ((pgcnt &0x3F) == 0x00)
				printf(" Reading Page %d @ 0x%X\n", 
					pgcnt, pgcnt*sizeof(data));
			srand(FLASH_SEED + pgcnt);
			for (cnt = 0; cnt < (sizeof(data)/sizeof(uint32_t)); cnt++) {
				seed = rand();
				if(*(pdata+cnt) != seed) {
					if(fru_prg_size) free(psprom);
					return(-1);
				}
			}
		}

		// Fill Data.
		memset(data, 0, sizeof(data));
		for (pgcnt = 0; pgcnt < pages; pgcnt++) {
			if ((pgcnt &0x3F) == 0x00)
				printf(" Clearing Page %d @ 0x%X\n", 
					pgcnt, pgcnt*sizeof(data));
			rc = diag_sprom_wr(dsprom, 
					(uint32_t)(pgcnt*sizeof(data)), 
					(uint32_t)sizeof(data), data);
			if (rc) {
				printf(" SEEPROM write failed rc=%d\n", rc);
				ppc_i2c_close(fd);
				if(fru_prg_size) free(psprom);
				return(-1);
			}
		}

		// Restore Data.
		if (fru_prg_size) {
			rc = diag_sprom_wr(dsprom, start_addr, 
					  fru_prg_size, psprom);
			if (rc) {
				printf("  Error: writing info to sprom\n");
				if(fru_prg_size) free(psprom);
				return(-1);
			}
		}
	}	
	ppc_i2c_close(fd);
	if(fru_prg_size) free(psprom);
	return (rc);
}
