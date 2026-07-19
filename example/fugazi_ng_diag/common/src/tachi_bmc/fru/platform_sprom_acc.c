/* $Id: platform_sprom_acc.c,v 1.2 2016/04/20 08:41:37 benchen2 Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/tachi_bmc/fru/platform_sprom_acc.c,v $
 * Copyright (c) 2016 by Cisco Systems, Inc.
 * All rights reserved.
 */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <fcntl.h>
#include <unistd.h>
#include <assert.h>
#include <errno.h>
#include <string.h>

#include "diag_sprom.h"
#include "Build_RAM_FRU.h"
#include "bmcsyslog.h"
#include "platform_gpio.h"
#include "diag_platform.h"

// FRU Device Commands
//	DESCRIPTION			Section	NetFN	CMD	C U O A
// Get FRU Inventory Area Info		34.1    Storage 10h       X
// Read Fru Data			34.2    Storage 11h       X
// Write Fru Data			34.3    Storage 12h         X 
//

extern int send_ipmi_cmd(unsigned char *send_data,int *send_data_length,
			 unsigned char *res_buffer,int *res_data_length);

#define IPMI_STORAGE	0x0A
#define IPMI_FRU_READ	0x11
#define IPMI_FRU_WRITE	0x12
#define IPMI_ACC_SIZE	0x04

typedef struct _ipmi_fru_read_s {
	uint8_t dummy;
	uint8_t	netfn_lun;
	uint8_t cmd;
	uint8_t fru_dev_id;
	uint8_t offset_ls;
	uint8_t offset_ms;
	uint8_t len;
} __attribute__ ((packed)) ipmi_fru_read_t;

typedef struct _ipmi_fru_read_resp_s {
	uint8_t	netfn_lun;
	uint8_t cmd;
	uint8_t code;
	uint8_t count;
	uint8_t data[IPMI_ACC_SIZE];
} __attribute__ ((packed)) ipmi_fru_read_resp_t;

typedef struct _ipmi_fru_write_s {
	uint8_t dummy;
	uint8_t	netfn_lun;
	uint8_t cmd;
	uint8_t fru_dev_id;
	uint8_t offset_ls;
	uint8_t offset_ms;
	uint8_t data[IPMI_ACC_SIZE];
} __attribute__ ((packed)) ipmi_fru_write_t;

typedef struct _ipmi_fru_write_resp_s {
	uint8_t	netfn_lun;
	uint8_t cmd;
	uint8_t code;
	uint8_t count;
} __attribute__ ((packed)) ipmi_fru_write_resp_t;

typedef struct _ipmi_fru_gen_req_s {
	uint8_t dummy;
	uint8_t	netfn_lun;
	uint8_t cmd;
	uint8_t fru_dev_id;
} __attribute__ ((packed)) ipmi_fru_gen_req_t;

typedef struct _ipmi_fru_gen_resp_s {
	uint8_t	netfn_lun;
	uint8_t cmd;
	uint8_t code;
} __attribute__ ((packed)) ipmi_fru_gen_resp_t;

void print_buf(uint8_t *buf, int len) {
	int i;
	for(i=0; i<len; i++) {
		if( i%16 == 0 ) printf("\n");
		printf("%02x ", buf[i]);
	}
	printf("\n");
}

#define IPMI_BMC_DEV	0x02
#define IPMI_MEZZ_DEV	0x01
#define IPMI_MEZZ_DEV_1	0x03

#define IPMI_SDCARD_DEV 0x10

int eeprom_i2c_read(uint8_t bus, uint8_t addr, uint8_t *buf, uint32_t len, int mode, int r_size) {
	int fd = platform_i2c_open();
	if( fd <= 0 ) return fd;
	
	uint8_t wbuf[4];
	int start=0, rlen=0, ws=0;
	while( start < len ) {
		rlen = ((len-start)>=r_size) ? r_size : (len-start);
		if( mode ) { /* 16 bits fru offset */
			wbuf[0] = (start>>8) & 0xff;
			wbuf[1] = start & 0xff;
			ws = 2;
		} else { /* 8 bits fru offset */
			wbuf[0] = start;
			ws = 1;
		}
		if( I2CByteAccess_Variable(fd, bus, addr, wbuf, ws, buf+start, rlen, 0) < 0 ) {
			printf("Error read programming eeprom at bus %d addr 0x%02x\n", bus, addr);
			return -1;
		}
		start += r_size;
		usleep(5000);
	}

	platform_i2c_close(fd);
	return 0;
}

int eeprom_i2c_write(uint8_t bus, uint8_t addr, uint8_t *buf, uint32_t len, int mode, int w_size) {
	uint8_t wbuf[32];
	int fd = platform_i2c_open();
	if( fd <= 0 ) return fd;
	int ws=0;	
	int start=0, wlen=0;
	while( start < len ) {
		wlen = ((len-start)>=w_size) ? w_size : (len-start);
		if( mode ) { /* 16 bits fru offset */
			wbuf[0] = (start>>8) & 0xff;
			wbuf[1] = start & 0xff;
			memcpy(wbuf+2, buf+start, wlen);
			ws = 2+wlen;
		} else { /* 8 bits fru offset */
			wbuf[0] = start;
			memcpy(wbuf+1, buf+start, wlen);
			ws = 1+wlen;
		}
		if( I2CByteWrite_Block(fd, bus, addr, wbuf, ws, 0) < 0 ) {
			printf("Error write programming eeprom at bus %d addr 0x%02x\n", bus, addr);
			return -1;
		}
		start += w_size;
		usleep(5000);
	}

	platform_i2c_close(fd);
	return 0;
}

/* These should corresponds to ipmi_sprom_platform.c */
#define IPMI_HD_DBP_0	0x04
#define IPMI_HD_DBP_1	0x05
#define IPMI_TPM_DEV	0x06
/* --------------------------------------------------- */

static int getFruInfo(diag_smb_dev_t *smb_dev, uint8_t *bus, uint8_t *addr, uint8_t *mode, uint8_t *size) {
	switch( smb_dev->addr ) {
		case IPMI_BMC_DEV:
            *bus = PLATFORM_I2C_BUS3;
            *addr = PLATFORM_TACHIL_I2C_FRU_ADDR;      
			/* *mode = 0; */
			/* Two bytes address offset */
			*mode = 1;
			*size = 16;
			break;

	    case IPMI_SDCARD_DEV:
            *bus = PLATFORM_I2C_BUS5;
            *addr = PLATFORM_I2C_SDPROM_ADDR;
            /* *mode = 0; */
            /* Two bytes address offset */
            *mode = 1;
            *size = 16;
            break;

		case IPMI_MEZZ_DEV:
			if (!platform_gpio_mezz_present(0)) {
				printf("  Warn: Mezz 0 is not present, sprom access aborted\n");
				return (-1);
			}
			*bus = MEZZ_FRU_BUS;
			*addr = MEZZ_FRU_ADDR;
			*mode = 1;
			*size = 16;
			break;
			
		case IPMI_HD_DBP_0:
			*bus = PLATFORM_I2C_HDD_FRU_BUS;
			*addr = PLATFORM_I2C_HDD0_FRU_ADDR;
			*mode = 0;
			*size = 8;
			break;
			
		case IPMI_HD_DBP_1:
			*bus = PLATFORM_I2C_HDD_FRU_BUS;
			*addr = PLATFORM_I2C_HDD1_FRU_ADDR;
			*mode = 0;
			*size = 8;
			break;

		case IPMI_TPM_DEV:
			*bus = PLATFORM_I2C_TPM_FRU_BUS;
			*addr = PLATFORM_I2C_TPM_FRU_ADDR;
			*mode = 0;
			*size = 16;
			break;

		default:
			printf("Unknown FRU Dev, sprom access aborted\n");
			return (-1);
	}
	

	return 0;
}

int sprom_rd (diag_smb_dev_t *smb_dev, uint32_t offset, 
		uint32_t len, uint8_t *buf)
{
	int rc=0;
	uint8_t bus, addr, mode, size;

	rc = getFruInfo(smb_dev, &bus, &addr, &mode, &size);
	if( rc ) return rc;
	
	/*
	if (smb_dev->addr != IPMI_BMC_DEV) {
		// Check If Mezzanine is present.
		if (!platform_gpio_mezz_present(0)) {
			printf("  Warn: Mezz is not present (r)\n"); 
			return (-1);
		}
		//printf("Reading MEZZ FRU...\n");
		rc = eeprom_i2c_read(MEZZ_FRU_BUS, MEZZ_FRU_ADDR, buf, len, 1);
	} else {
		//printf("Reading BMC FRU...\n");
		rc = eeprom_i2c_read(PLATFORM_I2C_FRU_BUS, PLATFORM_I2C_FRU_ADDR, buf, len, 0);
	}*/

	rc = eeprom_i2c_read(bus, addr, buf, len, mode, size);
	return rc;
}


int sprom_wr (diag_smb_dev_t *smb_dev, uint32_t offset, 
		uint32_t len, uint8_t *buf)
{

	int rc=0;
	uint8_t bus, addr, mode, size;

	rc = getFruInfo(smb_dev, &bus, &addr, &mode, &size);
	if( rc ) return rc;
	
	
	/*
	if (smb_dev->addr == IPMI_BMC_DEV) {
		//printf("Programming BMC FRU...\n");
		rc = eeprom_i2c_write(PLATFORM_I2C_FRU_BUS, PLATFORM_I2C_FRU_ADDR, buf, len, 0);
	}
	else {
		if(!platform_gpio_mezz_present(0)) {
			printf(" Warning: Mezz is not present (w)\n");
			return (-1);
		}
		//printf("Programming MEZZ FRU...\n");
		rc = eeprom_i2c_write(MEZZ_FRU_BUS, MEZZ_FRU_ADDR, buf, len, 1);
	}*/

	rc = eeprom_i2c_write(bus, addr, buf, len, mode, size);
	return rc;
}


int diag_sprom_rd (diag_sprom_t *sprom, uint32_t offset, 
		uint32_t len, uint8_t *buf)
{
	assert (sprom);
	assert (sprom->smb_acc);
	assert (buf);
	assert (sprom->smb_acc->smb_rd);

	return (sprom->smb_acc->smb_rd(sprom->smb_acc->smb_dev, 
				offset, len, buf));
}

int diag_sprom_wr (diag_sprom_t *sprom, uint32_t offset, 
		uint32_t len, uint8_t *buf)
{
	int rc  = 0;
	assert (sprom);
	assert (sprom->smb_acc);
	assert (buf);
	assert (sprom->smb_acc->smb_rd);

	rc = sprom->smb_acc->smb_wr(sprom->smb_acc->smb_dev, 
			offset, len, buf);

	if (sprom->smb_acc->smb_dev->addr == IPMI_BMC_DEV) {
		if (rc) {
			SLOG_NOTICE(" BMC SPROM Write Failed (rc=%d)\n", rc);
		} else {
			SLOG_NOTICE(" BMC SPROM Write Successful\n");
		}
	} else {
		if (rc) {
			SLOG_NOTICE(" MEZZ SPROM Write Failed (rc=%d)\n", rc);
		} else {
			SLOG_NOTICE(" MEZZ SPROM Write Successful\n");
		}
	}
	if (rc) return  (rc);

	system(RECONSTRUCT_RAMFRU_CMD);
	return (0);
}
