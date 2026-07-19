/* $Id: diag_err.c,v 1.2 2016/04/20 08:41:35 benchen2 Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/tachi_bmc/fru/diag_err.c,v $
 * Copyright (c) 2016 by Cisco Systems, Inc.
 * All rights reserved.
 */
#include "diag_main.h"
#include "diag_err.h"

typedef struct _diag_err_info_s_ {
	diagerr_t	errcode;
	char*		errname;
} diag_err_info_t;

#define ERR_ENTRY(errcode)	{errcode, __STRING(errcode)}

diag_err_info_t	diag_err_info[] = 
{
	ERR_ENTRY(DIAG_SUCCESS),
	ERR_ENTRY(DIAG_SOFTWARE_ERROR),
	ERR_ENTRY(DIAG_SYSTEM_ERROR),
	ERR_ENTRY(DIAG_MALLOC_ERROR),
	ERR_ENTRY(DIAG_SYNTAX_ERROR),
	ERR_ENTRY(DIAG_REG_ACC_ERROR),
	ERR_ENTRY(DIAG_BIST_ERROR),
	ERR_ENTRY(DIAG_PORT_LINK_ERROR),
	ERR_ENTRY(DIAG_EEPROM_ERROR),
	ERR_ENTRY(DIAG_SFP_ERROR),
	ERR_ENTRY(DIAG_RED_ACCESS_ERROR),
	ERR_ENTRY(DIAG_RED_REG_TEST_FAILED),
	ERR_ENTRY(DIAG_RED_PRBS_TEST_FAILED),
	ERR_ENTRY(DIAG_RED_MBIST_TEST_FAILED),
	ERR_ENTRY(DIAG_RED_MEMIF_TEST_FAILED),
	ERR_ENTRY(DIAG_RED_SERDES_ACC_TEST_FAILED),
	ERR_ENTRY(DIAG_RED_SERDES_RXRDY_TEST_FAILED),
	ERR_ENTRY(DIAG_RED_FREE_LIST_TEST_FAILED),
	ERR_ENTRY(DIAG_RED_PORT_LINK_TEST_FAILED),
	ERR_ENTRY(DIAG_PKT_TX_ERROR),
	ERR_ENTRY(DIAG_PKT_RX_ERROR),
        ERR_ENTRY(DIAG_PSU_PRESENT_FAILED),
        ERR_ENTRY(DIAG_PSU_FAILED),
        ERR_ENTRY(DIAG_PSU_FAN_FAILED),
        ERR_ENTRY(DIAG_PSU_EEPROM_FAILED),
        ERR_ENTRY(DIAG_BRD_EEPROM_FAILED),
        ERR_ENTRY(DIAG_FLASH_ERASE_ERROR),
        ERR_ENTRY(DIAG_FLASH_OPEN_ERROR),
        ERR_ENTRY(DIAG_FLASH_READ_ERROR),
        ERR_ENTRY(DIAG_FLASH_WRITE_ERROR),
        ERR_ENTRY(DIAG_FLASH_DATA_ERROR),
        ERR_ENTRY(DIAG_FRU_CHECK_ERROR),
        ERR_ENTRY(DIAG_RTC_ERROR),
        ERR_ENTRY(DIAG_FAN_SPEED_ERROR),
        ERR_ENTRY(DIAG_PSU_DC_OK_ERROR),
        ERR_ENTRY(DIAG_PSU_AC_OK_ERROR),
        ERR_ENTRY(DIAG_RANGE_VIOLATE_ERROR),
        ERR_ENTRY(DIAG_PALO_PCI_ERROR),
        ERR_ENTRY(DIAG_PHY_EEPROM_ERROR),
        ERR_ENTRY(DIAG_PHY_PRBS_ERROR),
        ERR_ENTRY(DIAG_I2CDEV_OPEN_ERROR),
        ERR_ENTRY(DIAG_DATA_MISMATCH_ERROR),
        ERR_ENTRY(DIAG_WOO_REG_ACCESS_ERROR),
        ERR_ENTRY(DIAG_WOO_REG_TEST_FAILED),
        ERR_ENTRY(DIAG_WOO_MBIST_TEST_FAILED),
	ERR_ENTRY(DIAG_SANITY_ERROR),
	ERR_ENTRY(DIAG_VMARG_ERROR),
	ERR_ENTRY(DIAG_DIMM_SPD_ERROR),
	ERR_ENTRY(DIAG_DIMM_TS_ERROR),
	ERR_ENTRY(DIAG_POE_DET_CLASS_ERROR),
	ERR_ENTRY(DIAG_EXIT_ERROR),
};

void diag_err_display (int errcode) 
{
	uint32_t	cnt;
	uint32_t	errcnt = sizeof(diag_err_info)/sizeof(diag_err_info_t);

	for (cnt = 0; cnt < errcnt; cnt++) {
		if (diag_err_info[cnt].errcode != (uint32_t) errcode)
			continue;
		printf("%04d: %s\n", diag_err_info[cnt].errcode,
				diag_err_info[cnt].errname);
		return;
	}
	printf("  Unknown error code %d\n", errcode);
}

void diag_err_display_all () 
{
	uint32_t	cnt;
	uint32_t	errcnt = sizeof(diag_err_info)/sizeof(diag_err_info_t);

	for (cnt = 0; cnt < errcnt; cnt++) {
		printf("%d: %s\n", diag_err_info[cnt].errcode,
				diag_err_info[cnt].errname);
	}
}
