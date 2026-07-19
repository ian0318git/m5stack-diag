/* $Id: ipmi_sprom_dflt.c,v 1.2 2016/04/20 08:41:36 benchen2 Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/tachi_bmc/fru/ipmi_sprom_dflt.c,v $
 * Copyright (c) 2016 by Cisco Systems, Inc.
 * All rights reserved.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <errno.h>
#include <arpa/inet.h>
#include "ipmi_sprom_ops.h"


#define SDR_IOM_TEMP_MULTI_FACTOR       0x01
#define SDR_IOM_TEMP_BASE_OFFSET        -128
#define SDR_IOM_TEMP_K1                 0x00
#define SDR_IOM_TEMP_K2                 0x00
#define SDR_IOM_TEMP_NOMINAL_READ       0x9E
#define SDR_IOM_TEMP_NORMAL_MAX         0xBC
#define SDR_IOM_TEMP_NORMAL_MIN         0x80
#define SDR_IOM_TEMP_SENSOR_MAX         0xFF
#define SDR_IOM_TEMP_SENSOR_MIN         0x58
#define SDR_IOM_TEMP_UP_NR_THRES        0xE4
#define SDR_IOM_TEMP_UP_CRIT            0xC6
#define SDR_IOM_TEMP_UP_NON_CRIT        0xC1
#define SDR_IOM_TEMP_LO_NR_THRES        0x6C
#define SDR_IOM_TEMP_LO_CRIT            0x76
#define SDR_IOM_TEMP_LO_NON_CRIT        0x7B
#define SDR_IOM_TEMP_POS_HYST           0x8A
#define SDR_IOM_TEMP_NEG_HYST           0x8A

#define SDR_RDW_TEMP_MULTI_FACTOR       0x01
#define SDR_RDW_TEMP_BASE_OFFSET        -128
#define SDR_RDW_TEMP_K1                 0x00
#define SDR_RDW_TEMP_K2                 0x00
#define SDR_RDW_TEMP_NOMINAL_READ       0xC1
#define SDR_RDW_TEMP_NORMAL_MAX         0xD5
#define SDR_RDW_TEMP_NORMAL_MIN         0x80
#define SDR_RDW_TEMP_SENSOR_MAX         0xFF
#define SDR_RDW_TEMP_SENSOR_MIN         0x58
#define SDR_RDW_TEMP_UP_NR_THRES        0xFD
#define SDR_RDW_TEMP_UP_CRIT            0xE4
#define SDR_RDW_TEMP_UP_NON_CRIT        0xDA
#define SDR_RDW_TEMP_LO_NR_THRES        0x6C
#define SDR_RDW_TEMP_LO_CRIT            0x76
#define SDR_RDW_TEMP_LO_NON_CRIT        0x7B
#define SDR_RDW_TEMP_POS_HYST           0x8A
#define SDR_RDW_TEMP_NEG_HYST           0x8A


#define SDR_FAN_RPM_MULTI_FACTOR        0x28
#define SDR_FAN_RPM_BASE_OFFSET         0x00
#define SDR_FAN_RPM_K1                  0x00
#define SDR_FAN_RPM_K2                  0x00
#define SDR_FAN_RPM_NOMINAL_READ        0x84
#define SDR_FAN_RPM_NORMAL_MAX          0xE6
#define SDR_FAN_RPM_NORMAL_MIN          0x19
#define SDR_FAN_RPM_SENSOR_MAX          0xFF
#define SDR_FAN_RPM_SENSOR_MIN          0x00 
#define SDR_FAN_RPM_UP_NR_THRES         0xFA
#define SDR_FAN_RPM_UP_CRIT             0xF5
#define SDR_FAN_RPM_UP_NON_CRIT         0xEF
#define SDR_FAN_RPM_LO_NR_THRES         0x14
#define SDR_FAN_RPM_LO_CRIT             0x17
#define SDR_FAN_RPM_LO_NON_CRIT         0x18
#define SDR_FAN_RPM_POS_HYST            0x03
#define SDR_FAN_RPM_NEG_HYST            0x03


#define SDR_FAN_TEMP_MULTI_FACTOR       1
#define SDR_FAN_TEMP_BASE_OFFSET        -32
#define SDR_FAN_TEMP_K1                 0
#define SDR_FAN_TEMP_K2                 0
#define SDR_FAN_TEMP_NOMINAL_READ       0x48
#define SDR_FAN_TEMP_NORMAL_MAX         0x5C
#define SDR_FAN_TEMP_NORMAL_MIN        	0x2A 
#define SDR_FAN_TEMP_SENSOR_MAX         0xDF
#define SDR_FAN_TEMP_SENSOR_MIN         0x00
#define SDR_FAN_TEMP_UP_NR_THRES        0x9D 
#define SDR_FAN_TEMP_UP_CRIT            0x66
#define SDR_FAN_TEMP_UP_NON_CRIT        0x61
#define SDR_FAN_TEMP_LO_NR_THRES        0x0C
#define SDR_FAN_TEMP_LO_CRIT            0x20
#define SDR_FAN_TEMP_LO_NON_CRIT        0x25
#define SDR_FAN_TEMP_POS_HYST           0x25
#define SDR_FAN_TEMP_NEG_HYST           0x25

#define SDR_FAN_VOLT_MULTI_FACTOR       0x4F
#define SDR_FAN_VOLT_BASE_OFFSET        0x00
#define SDR_FAN_VOLT_K1                 0x00
#define SDR_FAN_VOLT_K2                 0x0D 
#define SDR_FAN_VOLT_NOMINAL_READ       0x98
#define SDR_FAN_VOLT_NORMAL_MAX         0xCB
#define SDR_FAN_VOLT_NORMAL_MIN         0x92
#define SDR_FAN_VOLT_SENSOR_MAX         0xFF
#define SDR_FAN_VOLT_SENSOR_MIN         0x00 
#define SDR_FAN_VOLT_UP_NR_THRES        0xF1
#define SDR_FAN_VOLT_UP_CRIT            0xE4
#define SDR_FAN_VOLT_UP_NON_CRIT        0xD7
#define SDR_FAN_VOLT_LO_NR_THRES        0x7F
#define SDR_FAN_VOLT_LO_CRIT            0x86
#define SDR_FAN_VOLT_LO_NON_CRIT        0x8C
#define SDR_FAN_VOLT_POS_HYST           0x01
#define SDR_FAN_VOLT_NEG_HYST           0x01

#define SDR_PSU_RPM_MULTI_FACTOR        0x4F
#define SDR_PSU_RPM_BASE_OFFSET         0x00
#define SDR_PSU_RPM_K1                  0x00
#define SDR_PSU_RPM_K2                  0x00
#define SDR_PSU_RPM_NOMINAL_READ        0x00
#define SDR_PSU_RPM_NORMAL_MAX          0xB2
#define SDR_PSU_RPM_NORMAL_MIN          0x00
#define SDR_PSU_RPM_SENSOR_MAX          0xFE
#define SDR_PSU_RPM_SENSOR_MIN          0
#define SDR_PSU_RPM_UP_NR_THRES         0xCB
#define SDR_PSU_RPM_UP_CRIT             0xBE
#define SDR_PSU_RPM_UP_NON_CRIT         0xB8
#define SDR_PSU_RPM_LO_NR_THRES         0x00
#define SDR_PSU_RPM_LO_CRIT             0x00
#define SDR_PSU_RPM_LO_NON_CRIT         0x00
#define SDR_PSU_RPM_POS_HYST            0x00
#define SDR_PSU_RPM_NEG_HYST            0x00

#define SDR_PSU_TEMP_MULTI_FACTOR        0x01
#define SDR_PSU_TEMP_BASE_OFFSET         -128
#define SDR_PSU_TEMP_K1                  0x00
#define SDR_PSU_TEMP_K2                  0x00
#define SDR_PSU_TEMP_NOMINAL_READ        0x9E
#define SDR_PSU_TEMP_NORMAL_MAX          0xAD
#define SDR_PSU_TEMP_NORMAL_MIN          0x80
#define SDR_PSU_TEMP_SENSOR_MAX          0xFF
#define SDR_PSU_TEMP_SENSOR_MIN          0x00
#define SDR_PSU_TEMP_UP_NR_THRES         0xCB
#define SDR_PSU_TEMP_UP_CRIT             0xB7
#define SDR_PSU_TEMP_UP_NON_CRIT         0xB2
#define SDR_PSU_TEMP_LO_NR_THRES         0x58
#define SDR_PSU_TEMP_LO_CRIT             0x76
#define SDR_PSU_TEMP_LO_NON_CRIT         0x7B
#define SDR_PSU_TEMP_POS_HYST            0x8A
#define SDR_PSU_TEMP_NEG_HYST            0x8A

#define SDR_PSU_IN_VOLT_MULTI_FACTOR       1
#define SDR_PSU_IN_VOLT_BASE_OFFSET        0x64
#define SDR_PSU_IN_VOLT_K1                 0
#define SDR_PSU_IN_VOLT_K2                 0
#define SDR_PSU_IN_VOLT_NOMINAL_READ       0xFF9C
#define SDR_PSU_IN_VOLT_NORMAL_MAX         0xA3
#define SDR_PSU_IN_VOLT_NORMAL_MIN         0x50
#define SDR_PSU_IN_VOLT_SENSOR_MAX         0xFF
#define SDR_PSU_IN_VOLT_SENSOR_MIN         0
#define SDR_PSU_IN_VOLT_UP_NR_THRES        0xC8
#define SDR_PSU_IN_VOLT_UP_CRIT            0xB4
#define SDR_PSU_IN_VOLT_UP_NON_CRIT        0xAA
#define SDR_PSU_IN_VOLT_LO_NR_THRES        0x32
#define SDR_PSU_IN_VOLT_LO_CRIT            0x3C
#define SDR_PSU_IN_VOLT_LO_NON_CRIT        0x46
#define SDR_PSU_IN_VOLT_POS_HYST           0xFF9C
#define SDR_PSU_IN_VOLT_NEG_HYST           0xFF9C

#define SDR_PSU_OUT_VOLT1_MULTI_FACTOR       0x4F
#define SDR_PSU_OUT_VOLT1_BASE_OFFSET        0x00
#define SDR_PSU_OUT_VOLT1_K1                 0
#define SDR_PSU_OUT_VOLT1_K2                 0x0D
#define SDR_PSU_OUT_VOLT1_NOMINAL_READ       0x98
#define SDR_PSU_OUT_VOLT1_NORMAL_MAX         0xAE
#define SDR_PSU_OUT_VOLT1_NORMAL_MIN         0x7F
#define SDR_PSU_OUT_VOLT1_SENSOR_MAX         0xFE
#define SDR_PSU_OUT_VOLT1_SENSOR_MIN         0
#define SDR_PSU_OUT_VOLT1_UP_NR_THRES        0xB8
#define SDR_PSU_OUT_VOLT1_UP_CRIT            0xB2
#define SDR_PSU_OUT_VOLT1_UP_NON_CRIT        0xAF
#define SDR_PSU_OUT_VOLT1_LO_NR_THRES        0x72
#define SDR_PSU_OUT_VOLT1_LO_CRIT            0x79
#define SDR_PSU_OUT_VOLT1_LO_NON_CRIT        0x7C
#define SDR_PSU_OUT_VOLT1_POS_HYST           0
#define SDR_PSU_OUT_VOLT1_NEG_HYST           0

#define SDR_PSU_OUT_VOLT2_MULTI_FACTOR       0x18
#define SDR_PSU_OUT_VOLT2_BASE_OFFSET        0x00
#define SDR_PSU_OUT_VOLT2_K1                 0
#define SDR_PSU_OUT_VOLT2_K2                 0x0D
#define SDR_PSU_OUT_VOLT2_NOMINAL_READ       138
#define SDR_PSU_OUT_VOLT2_NORMAL_MAX         159
#define SDR_PSU_OUT_VOLT2_NORMAL_MIN         105
#define SDR_PSU_OUT_VOLT2_SENSOR_MAX         250
#define SDR_PSU_OUT_VOLT2_SENSOR_MIN         0
#define SDR_PSU_OUT_VOLT2_UP_NR_THRES        175
#define SDR_PSU_OUT_VOLT2_UP_CRIT            167
#define SDR_PSU_OUT_VOLT2_UP_NON_CRIT        162
#define SDR_PSU_OUT_VOLT2_LO_NR_THRES        84
#define SDR_PSU_OUT_VOLT2_LO_CRIT            92
#define SDR_PSU_OUT_VOLT2_LO_NON_CRIT        98
#define SDR_PSU_OUT_VOLT2_POS_HYST           0
#define SDR_PSU_OUT_VOLT2_NEG_HYST           0

#define SDR_PSU_AMP_MULTI_FACTOR       79
#define SDR_PSU_AMP_BASE_OFFSET        0x00
#define SDR_PSU_AMP_K1                 0x00
#define SDR_PSU_AMP_K2                 0x0D 
#define SDR_PSU_AMP_NOMINAL_READ       0x00
#define SDR_PSU_AMP_NORMAL_MAX         0xBE
#define SDR_PSU_AMP_NORMAL_MIN         0x00
#define SDR_PSU_AMP_SENSOR_MAX         0xFE
#define SDR_PSU_AMP_SENSOR_MIN         0
#define SDR_PSU_AMP_UP_NR_THRES        0xFE
#define SDR_PSU_AMP_UP_CRIT            0xFD
#define SDR_PSU_AMP_UP_NON_CRIT        0xCA
#define SDR_PSU_AMP_LO_NR_THRES        0x00
#define SDR_PSU_AMP_LO_CRIT            0x00
#define SDR_PSU_AMP_LO_NON_CRIT        0x00
#define SDR_PSU_AMP_POS_HYST           0
#define SDR_PSU_AMP_NEG_HYST           0

#define SDR_PSU_OUT_AMP_MULTI_FACTOR   1
#define SDR_PSU_OUT_AMP_BASE_OFFSET    0
#define SDR_PSU_OUT_AMP_K1             0
#define SDR_PSU_OUT_AMP_K2             0
#define SDR_PSU_OUT_AMP_NOMINAL_READ   0x00
#define SDR_PSU_OUT_AMP_NORMAL_MAX     0xD0
#define SDR_PSU_OUT_AMP_NORMAL_MIN     0x00
#define SDR_PSU_OUT_AMP_SENSOR_MAX     0xFF
#define SDR_PSU_OUT_AMP_SENSOR_MIN     0
#define SDR_PSU_OUT_AMP_UP_NR_THRES    0xE1
#define SDR_PSU_OUT_AMP_UP_CRIT        0xE0
#define SDR_PSU_OUT_AMP_UP_NON_CRIT    0xD1
#define SDR_PSU_OUT_AMP_LO_NR_THRES    0x00
#define SDR_PSU_OUT_AMP_LO_CRIT        0x00
#define SDR_PSU_OUT_AMP_LO_NON_CRIT    0x00
#define SDR_PSU_OUT_AMP_POS_HYST       0
#define SDR_PSU_OUT_AMP_NEG_HYST       0


void set_default_sdr(ipmi_sdr_rec_t *prec, uint8_t type, uint16_t value)
{
	prec->type = type;
	prec->len  = 2;
	prec->value = htons(value);
}


#define SET_SDR(psdr, fld)	\
{				\
set_default_sdr(&psdr->multi_factor, SDR_MULTI_FACTOR, SDR_##fld##_MULTI_FACTOR);\
set_default_sdr(&psdr->base_offset, SDR_BASE_OFFSET, SDR_##fld##_BASE_OFFSET); \
set_default_sdr(&psdr->k1, SDR_K1, SDR_##fld##_K1); \
set_default_sdr(&psdr->k2, SDR_K2, SDR_##fld##_K2); \
set_default_sdr(&psdr->nominal_reading, SDR_NOMINAL_READ, SDR_##fld##_NOMINAL_READ); \
set_default_sdr(&psdr->normal_max, SDR_NORMAL_MAX, SDR_##fld##_NORMAL_MAX); \
set_default_sdr(&psdr->normal_min, SDR_NORMAL_MIN, SDR_##fld##_NORMAL_MIN); \
set_default_sdr(&psdr->sensor_max, SDR_SENSOR_MAX, SDR_##fld##_SENSOR_MAX); \
set_default_sdr(&psdr->sensor_min, SDR_SENSOR_MIN, SDR_##fld##_SENSOR_MIN); \
set_default_sdr(&psdr->upper_nr_thres, SDR_UP_NR_THRES, SDR_##fld##_UP_NR_THRES); \
set_default_sdr(&psdr->upper_crit_thres, SDR_UP_CRIT, SDR_##fld##_UP_CRIT); \
set_default_sdr(&psdr->upper_non_crit_thres, SDR_UP_NON_CRIT, SDR_##fld##_UP_NON_CRIT); \
set_default_sdr(&psdr->lower_nr_thres, SDR_LO_NR_THRES, SDR_##fld##_LO_NR_THRES); \
set_default_sdr(&psdr->lower_crit_thres, SDR_LO_CRIT, SDR_##fld##_LO_CRIT); \
set_default_sdr(&psdr->lower_non_crit_thres, SDR_LO_NON_CRIT, SDR_##fld##_LO_NON_CRIT); \
set_default_sdr(&psdr->pos_hyst, SDR_POS_HYST, SDR_##fld##_POS_HYST); \
set_default_sdr(&psdr->neg_hyst, SDR_NEG_HYST, SDR_##fld##_NEG_HYST); \
}

int iu_sdr_dflt_iom(ipmi_iu_sdr_t *psdr, uint8_t srec_type)
{
        psdr->srec.sub_type = htons(srec_type);
        psdr->srec.sub_type_len = htons( sizeof(ipmi_iu_sdr_t) -
                                  sizeof(ipmi_iu_rec_t));

	switch(srec_type) {
		case	IPMI_IU_SUB_TYPE_IOM_TEMP_1:
		case	IPMI_IU_SUB_TYPE_IOM_TEMP_2:
			SET_SDR(psdr, IOM_TEMP)
			break;

		case	IPMI_IU_SUB_TYPE_RW_TEMP_1:
		case	IPMI_IU_SUB_TYPE_RW_TEMP_2:
			SET_SDR(psdr, RDW_TEMP);
			break;

		case	IPMI_IU_SUB_TYPE_FAN_RPM:
			SET_SDR(psdr, FAN_RPM);
			break;

		case	IPMI_IU_SUB_TYPE_FAN_TEMP:
			SET_SDR(psdr, FAN_TEMP);
			break;

		case	IPMI_IU_SUB_TYPE_FAN_VOLTAGE:
			SET_SDR(psdr, FAN_VOLT);
			break;

		case	IPMI_IU_SUB_TYPE_PSU_RPM:
			SET_SDR(psdr, PSU_RPM);
			break;

		case	IPMI_IU_SUB_TYPE_PSU_TEMP:
			SET_SDR(psdr, PSU_TEMP);
			break;

		case	IPMI_IU_SUB_TYPE_PSU_IN_VOLT:
			SET_SDR(psdr, PSU_IN_VOLT);
			break;

		case	IPMI_IU_SUB_TYPE_PSU_OUT_VOLT1:
			SET_SDR(psdr, PSU_OUT_VOLT1);
			break;

		case	IPMI_IU_SUB_TYPE_PSU_OUT_VOLT2:
			SET_SDR(psdr, PSU_OUT_VOLT2);
			break;

		case	IPMI_IU_SUB_TYPE_PSU_IN_CURRENT:
			SET_SDR(psdr, PSU_AMP);
			break;

		case	IPMI_IU_SUB_TYPE_PSU_OUT_CURRENT:
			SET_SDR(psdr, PSU_OUT_AMP);
			break;

		default:
			printf("ERROR: Invalid Record type\n");
			return (-1);
			break;
	}
        return (0);
}

int ipmi_sprom_la_dflt(sprom_ipmi_bmc_t *sprom)
{
	ipmi_ibmc_internal_use_t *piu = (ipmi_ibmc_internal_use_t*)&sprom->internal_use;
	ipmi_sprom_board_info_t *pbrd = &sprom->board_info;
	ipmi_sprom_bmc_product_info_t *pprd = &sprom->product_info;

	memset (sprom, (int)NULL, sizeof(*sprom));
        ipmi_sprom_common_header_create(&sprom->common_header,
                M8_SIZE(sizeof(ipmi_ibmc_internal_use_t)),
                0,
                M8_SIZE(sizeof(ipmi_sprom_board_info_t)),
                M8_SIZE(sizeof(ipmi_sprom_bmc_product_info_t)),
                0);

	// Update Internal Use Area
        piu->version	= IPMI_SPROM_COMMON_HEADER_VERSION;
        piu->length	= M8_SIZE(sizeof(*piu));
        piu->card_type	= htons(IPMI_IU_CARD_TYPE_IBMC);

	// Update Mac Fields.
	piu->mac.srec.sub_type		= htons(IPMI_IU_SUB_TYPE_MAC_ADDR);
	piu->mac.srec.sub_type_len	= htons(sizeof(ipmi_iu_rec_mac_t) -
					  sizeof(ipmi_iu_rec_t));
	piu->mac.cnt.type		= IPMI_IU_ENTRY_TYPE_CNT; 
	piu->mac.cnt.len		= 2;
	piu->mac.cnt.value		= htons(0x06);
	
	piu->mac.type 			= IPMI_IU_ENTRY_TYPE_MAC;
	piu->mac.len 			= 6;
	memset(piu->mac.mac, 0, 6);
	piu->blade_class		= IPMI_SPROM_IBMC_IU_BLADE_CLASS_NONE;


	// Update Board Info
        pbrd->version         = IPMI_SPROM_BOARD_VERSION;
        pbrd->length          = M8_SIZE(sizeof(ipmi_sprom_board_info_t));
        pbrd->language_code   = IPMI_LC_ENGLISH_0;
        pbrd->no_more_tl      = IPMI_SPROM_NO_MORE_TYPE_LENGTH;
        pbrd->serial_num_tl   = IPMI_SPROM_TYPE_CODE_LC_MSK |
                                IPMI_SPROM_BOARD_SERIAL_NUMBER_SIZE;
        pbrd->mfg_info_tl     = IPMI_SPROM_TYPE_CODE_LC_MSK |
                                IPMI_SPROM_BOARD_MFG_INFO_SIZE;
	sprom_mem_fill((char*)pbrd->mfg_info, "Cisco Systems Inc", 
			IPMI_SPROM_BOARD_MFG_INFO_SIZE);

        pbrd->product_name_tl = IPMI_SPROM_TYPE_CODE_LC_MSK |
                                IPMI_SPROM_BOARD_PRODUCT_NAME_SIZE;

        pbrd->part_num_tl     = IPMI_SPROM_TYPE_CODE_LC_MSK |
                                IPMI_SPROM_BOARD_PART_NUMBER_SIZE;

	sprom_mem_fill((char*)pbrd->part_num, "73-12435-02", 
			IPMI_SPROM_BOARD_PART_NUMBER_SIZE);
	sprom_mem_fill((char*)pbrd->product_name, 
			"R250-2480805", 
			IPMI_SPROM_BOARD_PRODUCT_NAME_SIZE);

        pbrd->fru_file_id_tl  = IPMI_SPROM_TYPE_CODE_LC_MSK |
				IPMI_SPROM_BOARD_FRU_FILE_ID_SIZE;
        pbrd->custom_id_tl    = IPMI_SPROM_TYPE_CODE_LC_MSK |
                                IPMI_SPROM_BOARD_CUSTOM_ID_SIZE;
        pbrd->clei_tl         = IPMI_SPROM_TYPE_CODE_LC_MSK |
                                IPMI_SPROM_BOARD_CLEI_SIZE;
	pbrd->bom_rev[0]      = '0';
	pbrd->bom_rev[1]      = '0';
	pbrd->hw_rev          = 0x00;
	pbrd->pid_rev[0]      = 'V';
	pbrd->pid_rev[1]      = '0';
	pbrd->pid_rev[2]      = '0';
        ipmi_zero_checksum_create((uint8_t*)pbrd, sizeof(*pbrd));


	// Update Product Info
        pprd->version         = IPMI_SPROM_BOARD_VERSION;
        pprd->length          = M8_SIZE(sizeof(ipmi_sprom_bmc_product_info_t));
        pprd->language_code   = IPMI_LC_ENGLISH_0;
        pprd->no_more_tl      = IPMI_SPROM_NO_MORE_TYPE_LENGTH;
        pprd->serial_num_tl   = IPMI_SPROM_TYPE_CODE_LC_MSK |
                                IPMI_SPROM_BMC_PRODUCT_SERIAL_NUMBER_SIZE;
        pprd->mfg_name_tl     = IPMI_SPROM_TYPE_CODE_LC_MSK |
                                IPMI_SPROM_BMC_PRODUCT_MFG_NAME_SIZE;
	sprom_mem_fill((char*)pprd->mfg_name, "Cisco Systems Inc", 
			IPMI_SPROM_BMC_PRODUCT_MFG_NAME_SIZE);

        pprd->product_name_tl = IPMI_SPROM_TYPE_CODE_LC_MSK |
                                IPMI_SPROM_BMC_PRODUCT_NAME_SIZE;
	sprom_mem_fill((char*)pprd->product_name, 
			"R250-2480805", 
			IPMI_SPROM_BMC_PRODUCT_NAME_SIZE);

        pprd->part_model_tl     = IPMI_SPROM_TYPE_CODE_LC_MSK |
                                IPMI_SPROM_BMC_PRODUCT_PART_MODEL_SIZE;
	sprom_mem_fill((char*)pprd->part_model, "73-12435-02", 
			IPMI_SPROM_BMC_PRODUCT_PART_MODEL_SIZE);

        pprd->prd_version_tl     = IPMI_SPROM_TYPE_CODE_LC_MSK |
                                IPMI_SPROM_BMC_PRODUCT_VERSION_SIZE;
	pprd->prd_version = 1;

        pprd->asset_tag_tl    = IPMI_SPROM_TYPE_CODE_LC_MSK|
				IPMI_SPROM_BMC_PRODUCT_ASSET_TAG_SIZE;
	// Data is already cleared.

        pprd->fru_file_id_tl  = IPMI_SPROM_TYPE_CODE_LC_MSK |
				IPMI_SPROM_BMC_PRODUCT_FRU_FILE_ID_SIZE;
	sprom_mem_fill((char*)pprd->fru_file_id, "C250_M1_", 
			IPMI_SPROM_BMC_PRODUCT_FRU_FILE_ID_SIZE);

        pprd->custom_id_tl    = IPMI_SPROM_TYPE_CODE_LC_MSK |
                                IPMI_SPROM_BMC_PRODUCT_CUSTOM_ID_SIZE;
	pprd->bom_rev[0]      = '0';
	pprd->bom_rev[1]      = '1';
	pprd->pid_rev[0]      = 'V';
	pprd->pid_rev[1]      = '0';
	pprd->pid_rev[2]      = '0';
        ipmi_zero_checksum_create((uint8_t*)pprd, sizeof(*pprd));

	return (0);
}


int ipmi_sprom_cerritos_dflt(sprom_ipmi_bmc_t *sprom)
{
	ipmi_ibmc_internal_use_t *piu = (ipmi_ibmc_internal_use_t*)&sprom->internal_use;
	ipmi_sprom_board_info_t *pbrd = &sprom->board_info;
	ipmi_sprom_bmc_product_info_t *pprd = &sprom->product_info;

	memset (sprom, (int)NULL, sizeof(*sprom));
        ipmi_sprom_common_header_create(&sprom->common_header,
                M8_SIZE(sizeof(ipmi_ibmc_internal_use_t)),
                0,
                M8_SIZE(sizeof(ipmi_sprom_board_info_t)),
                M8_SIZE(sizeof(ipmi_sprom_bmc_product_info_t)),
                0);

	// Update Internal Use Area
        piu->version	= IPMI_SPROM_COMMON_HEADER_VERSION;
        piu->length	= M8_SIZE(sizeof(*piu));
        piu->card_type	= htons(IPMI_IU_CARD_TYPE_IBMC);

	// Update Mac Fields.
	piu->mac.srec.sub_type		= htons(IPMI_IU_SUB_TYPE_MAC_ADDR);
	piu->mac.srec.sub_type_len	= htons(sizeof(ipmi_iu_rec_mac_t) -
					  sizeof(ipmi_iu_rec_t));
	piu->mac.cnt.type		= IPMI_IU_ENTRY_TYPE_CNT; 
	piu->mac.cnt.len		= 2;
	piu->mac.cnt.value		= htons(0x02);
	
	piu->mac.type 			= IPMI_IU_ENTRY_TYPE_MAC;
	piu->mac.len 			= 6;
	memset(piu->mac.mac, 0, 6);
	piu->blade_class		= IPMI_SPROM_IBMC_IU_BLADE_CLASS_GOODING;


	// Update Board Info
        pbrd->version         = IPMI_SPROM_BOARD_VERSION;
        pbrd->length          = M8_SIZE(sizeof(ipmi_sprom_board_info_t));
        pbrd->language_code   = IPMI_LC_ENGLISH_0;
        pbrd->no_more_tl      = IPMI_SPROM_NO_MORE_TYPE_LENGTH;
        pbrd->serial_num_tl   = IPMI_SPROM_TYPE_CODE_LC_MSK |
                                IPMI_SPROM_BOARD_SERIAL_NUMBER_SIZE;
        pbrd->mfg_info_tl     = IPMI_SPROM_TYPE_CODE_LC_MSK |
                                IPMI_SPROM_BOARD_MFG_INFO_SIZE;
	sprom_mem_fill((char*)pbrd->mfg_info, "Cisco Systems Inc", 
			IPMI_SPROM_BOARD_MFG_INFO_SIZE);

        pbrd->product_name_tl = IPMI_SPROM_TYPE_CODE_LC_MSK |
                                IPMI_SPROM_BOARD_PRODUCT_NAME_SIZE;

        pbrd->part_num_tl     = IPMI_SPROM_TYPE_CODE_LC_MSK |
                                IPMI_SPROM_BOARD_PART_NUMBER_SIZE;

	sprom_mem_fill((char*)pbrd->part_num, "73-12806-02", 
			IPMI_SPROM_BOARD_PART_NUMBER_SIZE);
	sprom_mem_fill((char*)pbrd->product_name, 
			"N20-B6630-1", 
			IPMI_SPROM_BOARD_PRODUCT_NAME_SIZE);

        pbrd->fru_file_id_tl  = IPMI_SPROM_TYPE_CODE_LC_MSK |
				IPMI_SPROM_BOARD_FRU_FILE_ID_SIZE;
        pbrd->custom_id_tl    = IPMI_SPROM_TYPE_CODE_LC_MSK |
                                IPMI_SPROM_BOARD_CUSTOM_ID_SIZE;
        pbrd->clei_tl         = IPMI_SPROM_TYPE_CODE_LC_MSK |
                                IPMI_SPROM_BOARD_CLEI_SIZE;
	pbrd->bom_rev[0]      = '1';
	pbrd->bom_rev[1]      = '1';
	pbrd->hw_rev          = 0x02;
	pbrd->pid_rev[0]      = 'V';
	pbrd->pid_rev[1]      = '0';
	pbrd->pid_rev[2]      = '0';
        ipmi_zero_checksum_create((uint8_t*)pbrd, sizeof(*pbrd));


	// Update Product Info
        pprd->version         = IPMI_SPROM_BOARD_VERSION;
        pprd->length          = M8_SIZE(sizeof(ipmi_sprom_bmc_product_info_t));
        pprd->language_code   = IPMI_LC_ENGLISH_0;
        pprd->no_more_tl      = IPMI_SPROM_NO_MORE_TYPE_LENGTH;
        pprd->serial_num_tl   = IPMI_SPROM_TYPE_CODE_LC_MSK |
                                IPMI_SPROM_BMC_PRODUCT_SERIAL_NUMBER_SIZE;
        pprd->mfg_name_tl     = IPMI_SPROM_TYPE_CODE_LC_MSK |
                                IPMI_SPROM_BMC_PRODUCT_MFG_NAME_SIZE;
	sprom_mem_fill((char*)pprd->mfg_name, "Cisco Systems Inc", 
			IPMI_SPROM_BMC_PRODUCT_MFG_NAME_SIZE);

        pprd->product_name_tl = IPMI_SPROM_TYPE_CODE_LC_MSK |
                                IPMI_SPROM_BMC_PRODUCT_NAME_SIZE;
	sprom_mem_fill((char*)pprd->product_name, 
			"N20-B6630-1", 
			IPMI_SPROM_BMC_PRODUCT_NAME_SIZE);

        pprd->part_model_tl     = IPMI_SPROM_TYPE_CODE_LC_MSK |
                                IPMI_SPROM_BMC_PRODUCT_PART_MODEL_SIZE;
	sprom_mem_fill((char*)pprd->part_model, "73-12806-02", 
			IPMI_SPROM_BMC_PRODUCT_PART_MODEL_SIZE);

        pprd->prd_version_tl     = IPMI_SPROM_TYPE_CODE_LC_MSK |
                                IPMI_SPROM_BMC_PRODUCT_VERSION_SIZE;
	pprd->prd_version = 1;

        pprd->asset_tag_tl    = IPMI_SPROM_TYPE_CODE_LC_MSK|
				IPMI_SPROM_BMC_PRODUCT_ASSET_TAG_SIZE;
	// Data is already cleared.

        pprd->fru_file_id_tl  = IPMI_SPROM_TYPE_CODE_LC_MSK |
				IPMI_SPROM_BMC_PRODUCT_FRU_FILE_ID_SIZE;
	//sprom_mem_fill((char*)pprd->fru_file_id, "        ", 
	//		IPMI_SPROM_BMC_PRODUCT_FRU_FILE_ID_SIZE);

        pprd->custom_id_tl    = IPMI_SPROM_TYPE_CODE_LC_MSK |
                                IPMI_SPROM_BMC_PRODUCT_CUSTOM_ID_SIZE;
	pprd->bom_rev[0]      = '1';
	pprd->bom_rev[1]      = '1';
	pprd->pid_rev[0]      = 'V';
	pprd->pid_rev[1]      = '0';
	pprd->pid_rev[2]      = '0';
        ipmi_zero_checksum_create((uint8_t*)pprd, sizeof(*pprd));

	return (0);
}

int ipmi_sprom_castlerock_dflt(sprom_ipmi_bmc_t *sprom)
{
	ipmi_ibmc_internal_use_t *piu = (ipmi_ibmc_internal_use_t*)&sprom->internal_use;
	ipmi_sprom_board_info_t *pbrd = &sprom->board_info;
	ipmi_sprom_bmc_product_info_t *pprd = &sprom->product_info;

	memset (sprom, (int)NULL, sizeof(*sprom));
        ipmi_sprom_common_header_create(&sprom->common_header,
                M8_SIZE(sizeof(ipmi_ibmc_internal_use_t)),
                0,
                M8_SIZE(sizeof(ipmi_sprom_board_info_t)),
                M8_SIZE(sizeof(ipmi_sprom_bmc_product_info_t)),
                0);

	// Update Internal Use Area
        piu->version	= IPMI_SPROM_COMMON_HEADER_VERSION;
        piu->length	= M8_SIZE(sizeof(*piu));
        piu->card_type	= htons(IPMI_IU_CARD_TYPE_IBMC);

	// Update Mac Fields.
	piu->mac.srec.sub_type		= htons(IPMI_IU_SUB_TYPE_MAC_ADDR);
	piu->mac.srec.sub_type_len	= htons(sizeof(ipmi_iu_rec_mac_t) -
					  sizeof(ipmi_iu_rec_t));
	piu->mac.cnt.type		= IPMI_IU_ENTRY_TYPE_CNT; 
	piu->mac.cnt.len		= 2;
	piu->mac.cnt.value		= htons(0x02);
	
	piu->mac.type 			= IPMI_IU_ENTRY_TYPE_MAC;
	piu->mac.len 			= 6;
	memset(piu->mac.mac, 0, 6);
	piu->blade_class		= IPMI_SPROM_IBMC_IU_BLADE_CLASS_GOODING;


	// Update Board Info
        pbrd->version         = IPMI_SPROM_BOARD_VERSION;
        pbrd->length          = M8_SIZE(sizeof(ipmi_sprom_board_info_t));
        pbrd->language_code   = IPMI_LC_ENGLISH_0;
        pbrd->no_more_tl      = IPMI_SPROM_NO_MORE_TYPE_LENGTH;
        pbrd->serial_num_tl   = IPMI_SPROM_TYPE_CODE_LC_MSK |
                                IPMI_SPROM_BOARD_SERIAL_NUMBER_SIZE;
        pbrd->mfg_info_tl     = IPMI_SPROM_TYPE_CODE_LC_MSK |
                                IPMI_SPROM_BOARD_MFG_INFO_SIZE;
	sprom_mem_fill((char*)pbrd->mfg_info, "Cisco Systems Inc", 
			IPMI_SPROM_BOARD_MFG_INFO_SIZE);

        pbrd->product_name_tl = IPMI_SPROM_TYPE_CODE_LC_MSK |
                                IPMI_SPROM_BOARD_PRODUCT_NAME_SIZE;

        pbrd->part_num_tl     = IPMI_SPROM_TYPE_CODE_LC_MSK |
                                IPMI_SPROM_BOARD_PART_NUMBER_SIZE;

	sprom_mem_fill((char*)pbrd->part_num, "73-13217-03", 
			IPMI_SPROM_BOARD_PART_NUMBER_SIZE);
	sprom_mem_fill((char*)pbrd->product_name, 
			"N20-Castle Rock", 
			IPMI_SPROM_BOARD_PRODUCT_NAME_SIZE);

        pbrd->fru_file_id_tl  = IPMI_SPROM_TYPE_CODE_LC_MSK |
				IPMI_SPROM_BOARD_FRU_FILE_ID_SIZE;
        pbrd->custom_id_tl    = IPMI_SPROM_TYPE_CODE_LC_MSK |
                                IPMI_SPROM_BOARD_CUSTOM_ID_SIZE;
        pbrd->clei_tl         = IPMI_SPROM_TYPE_CODE_LC_MSK |
                                IPMI_SPROM_BOARD_CLEI_SIZE;
	pbrd->bom_rev[0]      = '1';
	pbrd->bom_rev[1]      = '2';
	pbrd->hw_rev          = 0x03;
	pbrd->pid_rev[0]      = 'V';
	pbrd->pid_rev[1]      = '0';
	pbrd->pid_rev[2]      = '0';
        ipmi_zero_checksum_create((uint8_t*)pbrd, sizeof(*pbrd));


	// Update Product Info
        pprd->version         = IPMI_SPROM_BOARD_VERSION;
        pprd->length          = M8_SIZE(sizeof(ipmi_sprom_bmc_product_info_t));
        pprd->language_code   = IPMI_LC_ENGLISH_0;
        pprd->no_more_tl      = IPMI_SPROM_NO_MORE_TYPE_LENGTH;
        pprd->serial_num_tl   = IPMI_SPROM_TYPE_CODE_LC_MSK |
                                IPMI_SPROM_BMC_PRODUCT_SERIAL_NUMBER_SIZE;
        pprd->mfg_name_tl     = IPMI_SPROM_TYPE_CODE_LC_MSK |
                                IPMI_SPROM_BMC_PRODUCT_MFG_NAME_SIZE;
	sprom_mem_fill((char*)pprd->mfg_name, "Cisco Systems Inc", 
			IPMI_SPROM_BMC_PRODUCT_MFG_NAME_SIZE);

        pprd->product_name_tl = IPMI_SPROM_TYPE_CODE_LC_MSK |
                                IPMI_SPROM_BMC_PRODUCT_NAME_SIZE;
	sprom_mem_fill((char*)pprd->product_name, 
			"N20-Castle Rock", 
			IPMI_SPROM_BMC_PRODUCT_NAME_SIZE);

        pprd->part_model_tl     = IPMI_SPROM_TYPE_CODE_LC_MSK |
                                IPMI_SPROM_BMC_PRODUCT_PART_MODEL_SIZE;
	sprom_mem_fill((char*)pprd->part_model, "73-13217-03", 
			IPMI_SPROM_BMC_PRODUCT_PART_MODEL_SIZE);

        pprd->prd_version_tl     = IPMI_SPROM_TYPE_CODE_LC_MSK |
                                IPMI_SPROM_BMC_PRODUCT_VERSION_SIZE;
	pprd->prd_version = 1;

        pprd->asset_tag_tl    = IPMI_SPROM_TYPE_CODE_LC_MSK|
				IPMI_SPROM_BMC_PRODUCT_ASSET_TAG_SIZE;
	// Data is already cleared.

        pprd->fru_file_id_tl  = IPMI_SPROM_TYPE_CODE_LC_MSK |
				IPMI_SPROM_BMC_PRODUCT_FRU_FILE_ID_SIZE;
	//sprom_mem_fill((char*)pprd->fru_file_id, "        ", 
	//		IPMI_SPROM_BMC_PRODUCT_FRU_FILE_ID_SIZE);

        pprd->custom_id_tl    = IPMI_SPROM_TYPE_CODE_LC_MSK |
                                IPMI_SPROM_BMC_PRODUCT_CUSTOM_ID_SIZE;
	pprd->bom_rev[0]      = '1';
	pprd->bom_rev[1]      = '2';
	pprd->pid_rev[0]      = 'V';
	pprd->pid_rev[1]      = '0';
	pprd->pid_rev[2]      = '0';
        ipmi_zero_checksum_create((uint8_t*)pprd, sizeof(*pprd));

	return (0);
}

int ipmi_sprom_castlerock_hddbp_dflt(sprom_ipmi_hddbp_t *sprom)
{
	ipmi_hddbp_internal_use_t *piu = (ipmi_hddbp_internal_use_t*)&sprom->internal_use;
	ipmi_sprom_board_info_t *pbrd = &sprom->board_info;

	memset (sprom, (int)NULL, sizeof(*sprom));
        ipmi_sprom_common_header_create(&sprom->common_header,
                M8_SIZE(sizeof(ipmi_ibmc_internal_use_t)),
                0,
                M8_SIZE(sizeof(ipmi_sprom_board_info_t)),
                0,
                0);

	// Update Internal Use Area
        piu->version	= IPMI_SPROM_COMMON_HEADER_VERSION;
        piu->length	= M8_SIZE(sizeof(*piu));
        piu->card_type	= htons(IPMI_IU_CARD_TYPE_TURLOCK);


	// Update Board Info
        pbrd->version         = IPMI_SPROM_BOARD_VERSION;
        pbrd->length          = M8_SIZE(sizeof(ipmi_sprom_board_info_t));
        pbrd->language_code   = IPMI_LC_ENGLISH_0;
        pbrd->no_more_tl      = IPMI_SPROM_NO_MORE_TYPE_LENGTH;
        pbrd->serial_num_tl   = IPMI_SPROM_TYPE_CODE_LC_MSK |
                                IPMI_SPROM_BOARD_SERIAL_NUMBER_SIZE;
        pbrd->mfg_info_tl     = IPMI_SPROM_TYPE_CODE_LC_MSK |
                                IPMI_SPROM_BOARD_MFG_INFO_SIZE;
	sprom_mem_fill((char*)pbrd->mfg_info, "Cisco Systems Inc", 
			IPMI_SPROM_BOARD_MFG_INFO_SIZE);

        pbrd->product_name_tl = IPMI_SPROM_TYPE_CODE_LC_MSK |
                                IPMI_SPROM_BOARD_PRODUCT_NAME_SIZE;

        pbrd->part_num_tl     = IPMI_SPROM_TYPE_CODE_LC_MSK |
                                IPMI_SPROM_BOARD_PART_NUMBER_SIZE;

	sprom_mem_fill((char*)pbrd->part_num, "73-13219-01", 
			IPMI_SPROM_BOARD_PART_NUMBER_SIZE);
	sprom_mem_fill((char*)pbrd->product_name, 
			"TURLOCK", 
			IPMI_SPROM_BOARD_PRODUCT_NAME_SIZE);

        pbrd->fru_file_id_tl  = IPMI_SPROM_TYPE_CODE_LC_MSK |
				IPMI_SPROM_BOARD_FRU_FILE_ID_SIZE;
        pbrd->custom_id_tl    = IPMI_SPROM_TYPE_CODE_LC_MSK |
                                IPMI_SPROM_BOARD_CUSTOM_ID_SIZE;
        pbrd->clei_tl         = IPMI_SPROM_TYPE_CODE_LC_MSK |
                                IPMI_SPROM_BOARD_CLEI_SIZE;
	pbrd->bom_rev[0]      = '0';
	pbrd->bom_rev[1]      = '7';
	pbrd->hw_rev          = 0x01;
	pbrd->pid_rev[0]      = 'V';
	pbrd->pid_rev[1]      = '0';
	pbrd->pid_rev[2]      = '0';
        ipmi_zero_checksum_create((uint8_t*)pbrd, sizeof(*pbrd));

	return (0);
}

int ipmi_sprom_castlerock_tpm_dflt(sprom_ipmi_bbu_t *sprom) /* same as alcatraz */
{
	ipmi_sprom_board_info_t *pbrd = (ipmi_sprom_board_info_t*)&sprom->board_info;
	ipmi_sprom_bmc_product_info_t *pprd = (ipmi_sprom_bmc_product_info_t*)&sprom->product_info;

        ipmi_sprom_common_header_create(&sprom->common_header,
                0,
                0,
                M8_SIZE(sizeof(ipmi_sprom_board_info_t)),
                M8_SIZE(sizeof(ipmi_sprom_bmc_product_info_t)),
                0);

	// Update Board Info
        pbrd->version         = IPMI_SPROM_BOARD_VERSION;
        pbrd->length          = M8_SIZE(sizeof(ipmi_sprom_board_info_t));
        pbrd->language_code   = IPMI_LC_ENGLISH_0;
        pbrd->no_more_tl      = IPMI_SPROM_NO_MORE_TYPE_LENGTH;
        pbrd->serial_num_tl   = IPMI_SPROM_TYPE_CODE_LC_MSK |
                                IPMI_SPROM_BOARD_SERIAL_NUMBER_SIZE;
        pbrd->mfg_info_tl     = IPMI_SPROM_TYPE_CODE_LC_MSK |
                                IPMI_SPROM_BOARD_MFG_INFO_SIZE;
	sprom_mem_fill((char*)pbrd->mfg_info, "Cisco Systems Inc", 
			IPMI_SPROM_BOARD_MFG_INFO_SIZE);

        pbrd->product_name_tl = IPMI_SPROM_TYPE_CODE_LC_MSK |
                                IPMI_SPROM_BOARD_PRODUCT_NAME_SIZE;

        pbrd->part_num_tl     = IPMI_SPROM_TYPE_CODE_LC_MSK |
                                IPMI_SPROM_BOARD_PART_NUMBER_SIZE;

	sprom_mem_fill((char*)pbrd->part_num, "73-12962-02", 
			IPMI_SPROM_BOARD_PART_NUMBER_SIZE);
	sprom_mem_fill((char*)pbrd->product_name, 
			"UCSX-TPM1-001", 
			IPMI_SPROM_BOARD_PRODUCT_NAME_SIZE);

        pbrd->fru_file_id_tl  = IPMI_SPROM_TYPE_CODE_LC_MSK |
				IPMI_SPROM_BOARD_FRU_FILE_ID_SIZE;
	sprom_mem_fill((char*)pbrd->fru_file_id, "TPM1",
			IPMI_SPROM_BOARD_FRU_FILE_ID_SIZE);

        pbrd->custom_id_tl    = IPMI_SPROM_TYPE_CODE_LC_MSK |
                                IPMI_SPROM_BOARD_CUSTOM_ID_SIZE;
        pbrd->clei_tl         = IPMI_SPROM_TYPE_CODE_LC_MSK |
                                IPMI_SPROM_BOARD_CLEI_SIZE;
	pbrd->bom_rev[0]      = '0';
	pbrd->bom_rev[1]      = '5';
	pbrd->hw_rev          = 0x02;
	pbrd->pid_rev[0]      = 'V';
	pbrd->pid_rev[1]      = '0';
	pbrd->pid_rev[2]      = '0';
        ipmi_zero_checksum_create((uint8_t*)pbrd, sizeof(*pbrd));

	// Update Product Info
        pprd->version         = IPMI_SPROM_BOARD_VERSION;
        pprd->length          = M8_SIZE(sizeof(ipmi_sprom_bmc_product_info_t));
        pprd->language_code   = IPMI_LC_ENGLISH_0;
        pprd->no_more_tl      = IPMI_SPROM_NO_MORE_TYPE_LENGTH;
        pprd->serial_num_tl   = IPMI_SPROM_TYPE_CODE_LC_MSK |
                                IPMI_SPROM_BMC_PRODUCT_SERIAL_NUMBER_SIZE;
        pprd->mfg_name_tl     = IPMI_SPROM_TYPE_CODE_LC_MSK |
                                IPMI_SPROM_BMC_PRODUCT_MFG_NAME_SIZE;
	sprom_mem_fill((char*)pprd->mfg_name, "Cisco Systems Inc", 
			IPMI_SPROM_BMC_PRODUCT_MFG_NAME_SIZE);

        pprd->product_name_tl = IPMI_SPROM_TYPE_CODE_LC_MSK |
                                IPMI_SPROM_BMC_PRODUCT_NAME_SIZE;
	sprom_mem_fill((char*)pprd->product_name, 
			"UCSX-TPM1-001", 
			IPMI_SPROM_BMC_PRODUCT_NAME_SIZE);

        pprd->part_model_tl     = IPMI_SPROM_TYPE_CODE_LC_MSK |
                                IPMI_SPROM_BMC_PRODUCT_PART_MODEL_SIZE;
	sprom_mem_fill((char*)pprd->part_model, "73-12962-02", 
			IPMI_SPROM_BMC_PRODUCT_PART_MODEL_SIZE);

        pprd->prd_version_tl     = IPMI_SPROM_TYPE_CODE_LC_MSK |
                                IPMI_SPROM_BMC_PRODUCT_VERSION_SIZE;
	pprd->prd_version = 1;

        pprd->asset_tag_tl    = IPMI_SPROM_TYPE_CODE_LC_MSK|
				IPMI_SPROM_BMC_PRODUCT_ASSET_TAG_SIZE;
	// Data is already cleared.

        pprd->fru_file_id_tl  = IPMI_SPROM_TYPE_CODE_LC_MSK |
				IPMI_SPROM_BMC_PRODUCT_FRU_FILE_ID_SIZE;
	sprom_mem_fill((char*)pprd->fru_file_id, "TPM1-001", 
			IPMI_SPROM_BMC_PRODUCT_FRU_FILE_ID_SIZE);

        pprd->custom_id_tl    = IPMI_SPROM_TYPE_CODE_LC_MSK |
                                IPMI_SPROM_BMC_PRODUCT_CUSTOM_ID_SIZE;
	pprd->bom_rev[0]      = '0';
	pprd->bom_rev[1]      = '5';
	pprd->pid_rev[0]      = 'V';
	pprd->pid_rev[1]      = '0';
	pprd->pid_rev[2]      = '0';
        ipmi_zero_checksum_create((uint8_t*)pprd, sizeof(*pprd));

	return (0);
}

int ipmi_sprom_marin_dflt(sprom_ipmi_bmc_t *sprom)
{
	ipmi_ibmc_internal_use_t *piu = (ipmi_ibmc_internal_use_t*)&sprom->internal_use;
	ipmi_sprom_board_info_t *pbrd = &sprom->board_info;
	ipmi_sprom_bmc_product_info_t *pprd = &sprom->product_info;

	memset (sprom, (int)NULL, sizeof(*sprom));
        ipmi_sprom_common_header_create(&sprom->common_header,
                M8_SIZE(sizeof(ipmi_ibmc_internal_use_t)),
                0,
                M8_SIZE(sizeof(ipmi_sprom_board_info_t)),
                M8_SIZE(sizeof(ipmi_sprom_bmc_product_info_t)),
                0);

	// Update Internal Use Area
        piu->version	= IPMI_SPROM_COMMON_HEADER_VERSION;
        piu->length	= M8_SIZE(sizeof(*piu));
        piu->card_type	= htons(IPMI_IU_CARD_TYPE_IBMC);

	// Update Mac Fields.
	piu->mac.srec.sub_type		= htons(IPMI_IU_SUB_TYPE_MAC_ADDR);
	piu->mac.srec.sub_type_len	= htons(sizeof(ipmi_iu_rec_mac_t) -
					  sizeof(ipmi_iu_rec_t));
	piu->mac.cnt.type		= IPMI_IU_ENTRY_TYPE_CNT; 
	piu->mac.cnt.len		= 2;
	piu->mac.cnt.value		= htons(0x02);
	
	piu->mac.type 			= IPMI_IU_ENTRY_TYPE_MAC;
	piu->mac.len 			= 6;
	memset(piu->mac.mac, 0, 6);
	piu->blade_class		= IPMI_SPROM_IBMC_IU_BLADE_CLASS_GOODING;


	// Update Board Info
        pbrd->version         = IPMI_SPROM_BOARD_VERSION;
        pbrd->length          = M8_SIZE(sizeof(ipmi_sprom_board_info_t));
        pbrd->language_code   = IPMI_LC_ENGLISH_0;
        pbrd->no_more_tl      = IPMI_SPROM_NO_MORE_TYPE_LENGTH;
        pbrd->serial_num_tl   = IPMI_SPROM_TYPE_CODE_LC_MSK |
                                IPMI_SPROM_BOARD_SERIAL_NUMBER_SIZE;
        pbrd->mfg_info_tl     = IPMI_SPROM_TYPE_CODE_LC_MSK |
                                IPMI_SPROM_BOARD_MFG_INFO_SIZE;
	sprom_mem_fill((char*)pbrd->mfg_info, "Cisco Systems Inc", 
			IPMI_SPROM_BOARD_MFG_INFO_SIZE);

        pbrd->product_name_tl = IPMI_SPROM_TYPE_CODE_LC_MSK |
                                IPMI_SPROM_BOARD_PRODUCT_NAME_SIZE;

        pbrd->part_num_tl     = IPMI_SPROM_TYPE_CODE_LC_MSK |
                                IPMI_SPROM_BOARD_PART_NUMBER_SIZE;

	sprom_mem_fill((char*)pbrd->part_num, "73-12814-03", 
			IPMI_SPROM_BOARD_PART_NUMBER_SIZE);
	sprom_mem_fill((char*)pbrd->product_name, 
			"N20-B6730-1", 
			IPMI_SPROM_BOARD_PRODUCT_NAME_SIZE);

        pbrd->fru_file_id_tl  = IPMI_SPROM_TYPE_CODE_LC_MSK |
				IPMI_SPROM_BOARD_FRU_FILE_ID_SIZE;
	sprom_mem_fill((char*)pbrd->fru_file_id, "40-2", 
			IPMI_SPROM_BOARD_FRU_FILE_ID_SIZE);
	
	pbrd->custom_id_tl    = IPMI_SPROM_TYPE_CODE_LC_MSK |
                                IPMI_SPROM_BOARD_CUSTOM_ID_SIZE;
        pbrd->clei_tl         = IPMI_SPROM_TYPE_CODE_LC_MSK |
                                IPMI_SPROM_BOARD_CLEI_SIZE;
	pbrd->bom_rev[0]      = '0';
	pbrd->bom_rev[1]      = '1';
	pbrd->hw_rev          = 0x03;
	pbrd->pid_rev[0]      = 'V';
	pbrd->pid_rev[1]      = '0';
	pbrd->pid_rev[2]      = '0';
        ipmi_zero_checksum_create((uint8_t*)pbrd, sizeof(*pbrd));


	// Update Product Info
        pprd->version         = IPMI_SPROM_BOARD_VERSION;
        pprd->length          = M8_SIZE(sizeof(ipmi_sprom_bmc_product_info_t));
        pprd->language_code   = IPMI_LC_ENGLISH_0;
        pprd->no_more_tl      = IPMI_SPROM_NO_MORE_TYPE_LENGTH;
        pprd->serial_num_tl   = IPMI_SPROM_TYPE_CODE_LC_MSK |
                                IPMI_SPROM_BMC_PRODUCT_SERIAL_NUMBER_SIZE;
        pprd->mfg_name_tl     = IPMI_SPROM_TYPE_CODE_LC_MSK |
                                IPMI_SPROM_BMC_PRODUCT_MFG_NAME_SIZE;
	sprom_mem_fill((char*)pprd->mfg_name, "Cisco Systems Inc", 
			IPMI_SPROM_BMC_PRODUCT_MFG_NAME_SIZE);

        pprd->product_name_tl = IPMI_SPROM_TYPE_CODE_LC_MSK |
                                IPMI_SPROM_BMC_PRODUCT_NAME_SIZE;
	sprom_mem_fill((char*)pprd->product_name, 
			"N20-B6730-1", 
			IPMI_SPROM_BMC_PRODUCT_NAME_SIZE);

        pprd->part_model_tl     = IPMI_SPROM_TYPE_CODE_LC_MSK |
                                IPMI_SPROM_BMC_PRODUCT_PART_MODEL_SIZE;
	sprom_mem_fill((char*)pprd->part_model, "73-12814-03", 
			IPMI_SPROM_BMC_PRODUCT_PART_MODEL_SIZE);

        pprd->prd_version_tl     = IPMI_SPROM_TYPE_CODE_LC_MSK |
                                IPMI_SPROM_BMC_PRODUCT_VERSION_SIZE;
	pprd->prd_version = 1;

        pprd->asset_tag_tl    = IPMI_SPROM_TYPE_CODE_LC_MSK|
				IPMI_SPROM_BMC_PRODUCT_ASSET_TAG_SIZE;
	// Data is already cleared.

        pprd->fru_file_id_tl  = IPMI_SPROM_TYPE_CODE_LC_MSK |
				IPMI_SPROM_BMC_PRODUCT_FRU_FILE_ID_SIZE;
	sprom_mem_fill((char*)pprd->fru_file_id, "B6730-1", 
			IPMI_SPROM_BMC_PRODUCT_FRU_FILE_ID_SIZE);

        pprd->custom_id_tl    = IPMI_SPROM_TYPE_CODE_LC_MSK |
                                IPMI_SPROM_BMC_PRODUCT_CUSTOM_ID_SIZE;
	pprd->bom_rev[0]      = '0';
	pprd->bom_rev[1]      = '1';
	pprd->pid_rev[0]      = 'V';
	pprd->pid_rev[1]      = '0';
	pprd->pid_rev[2]      = '0';
        ipmi_zero_checksum_create((uint8_t*)pprd, sizeof(*pprd));

	return (0);
}

int ipmi_sprom_marin_memfront_dflt(sprom_ipmi_mem_t *sprom)
{
	ipmi_mem_internal_use_t *piu = (ipmi_mem_internal_use_t*)&sprom->internal_use;
	ipmi_sprom_board_info_t *pbrd = &sprom->board_info;
	
	memset (sprom, (int)NULL, sizeof(*sprom));
        ipmi_sprom_common_header_create(&sprom->common_header,
                M8_SIZE(sizeof(ipmi_ibmc_internal_use_t)),
                0,
                M8_SIZE(sizeof(ipmi_sprom_board_info_t)),
                0, //M8_SIZE(sizeof(ipmi_sprom_bmc_product_info_t)),
		0);

	// Update Internal Use Area
        piu->version	= IPMI_SPROM_COMMON_HEADER_VERSION;
        piu->length	= M8_SIZE(sizeof(*piu));
        piu->card_type	= htons(IPMI_IU_CARD_TYPE_MEMFRONT);

	// Update Board Info
        pbrd->version         = IPMI_SPROM_BOARD_VERSION;
        pbrd->length          = M8_SIZE(sizeof(ipmi_sprom_board_info_t));
        pbrd->language_code   = IPMI_LC_ENGLISH_0;
        pbrd->no_more_tl      = IPMI_SPROM_NO_MORE_TYPE_LENGTH;
        pbrd->serial_num_tl   = IPMI_SPROM_TYPE_CODE_LC_MSK |
                                IPMI_SPROM_BOARD_SERIAL_NUMBER_SIZE;
        pbrd->mfg_info_tl     = IPMI_SPROM_TYPE_CODE_LC_MSK |
                                IPMI_SPROM_BOARD_MFG_INFO_SIZE;
	sprom_mem_fill((char*)pbrd->mfg_info, "Cisco Systems Inc", 
			IPMI_SPROM_BOARD_MFG_INFO_SIZE);

        pbrd->product_name_tl = IPMI_SPROM_TYPE_CODE_LC_MSK |
                                IPMI_SPROM_BOARD_PRODUCT_NAME_SIZE;

        pbrd->part_num_tl     = IPMI_SPROM_TYPE_CODE_LC_MSK |
                                IPMI_SPROM_BOARD_PART_NUMBER_SIZE;

	sprom_mem_fill((char*)pbrd->part_num, "73-12999-03", 
			IPMI_SPROM_BOARD_PART_NUMBER_SIZE);
	sprom_mem_fill((char*)pbrd->product_name, 
			"N20-MEM", 
			IPMI_SPROM_BOARD_PRODUCT_NAME_SIZE);

        pbrd->fru_file_id_tl  = IPMI_SPROM_TYPE_CODE_LC_MSK |
				IPMI_SPROM_BOARD_FRU_FILE_ID_SIZE;
	sprom_mem_fill((char*)pbrd->fru_file_id, "40-2", 
			IPMI_SPROM_BOARD_FRU_FILE_ID_SIZE);
	
	pbrd->custom_id_tl    = IPMI_SPROM_TYPE_CODE_LC_MSK |
                                IPMI_SPROM_BOARD_CUSTOM_ID_SIZE;
        pbrd->clei_tl         = IPMI_SPROM_TYPE_CODE_LC_MSK |
                                IPMI_SPROM_BOARD_CLEI_SIZE;
	pbrd->bom_rev[0]      = '0';
	pbrd->bom_rev[1]      = '1';
	pbrd->hw_rev          = 0x01;
	pbrd->pid_rev[0]      = 'V';
	pbrd->pid_rev[1]      = '0';
	pbrd->pid_rev[2]      = '0';
        ipmi_zero_checksum_create((uint8_t*)pbrd, sizeof(*pbrd));

	return (0);
}

int ipmi_sprom_marin_memback_dflt(sprom_ipmi_mem_t *sprom)
{
	ipmi_mem_internal_use_t *piu = (ipmi_mem_internal_use_t*)&sprom->internal_use;
	ipmi_sprom_board_info_t *pbrd = &sprom->board_info;
	
	memset (sprom, (int)NULL, sizeof(*sprom));
        ipmi_sprom_common_header_create(&sprom->common_header,
                M8_SIZE(sizeof(ipmi_ibmc_internal_use_t)),
                0,
                M8_SIZE(sizeof(ipmi_sprom_board_info_t)),
                0, //M8_SIZE(sizeof(ipmi_sprom_bmc_product_info_t)),
		0);

	// Update Internal Use Area
        piu->version	= IPMI_SPROM_COMMON_HEADER_VERSION;
        piu->length	= M8_SIZE(sizeof(*piu));
        piu->card_type	= htons(IPMI_IU_CARD_TYPE_MEMBACK);

	// Update Board Info
        pbrd->version         = IPMI_SPROM_BOARD_VERSION;
        pbrd->length          = M8_SIZE(sizeof(ipmi_sprom_board_info_t));
        pbrd->language_code   = IPMI_LC_ENGLISH_0;
        pbrd->no_more_tl      = IPMI_SPROM_NO_MORE_TYPE_LENGTH;
        pbrd->serial_num_tl   = IPMI_SPROM_TYPE_CODE_LC_MSK |
                                IPMI_SPROM_BOARD_SERIAL_NUMBER_SIZE;
        pbrd->mfg_info_tl     = IPMI_SPROM_TYPE_CODE_LC_MSK |
                                IPMI_SPROM_BOARD_MFG_INFO_SIZE;
	sprom_mem_fill((char*)pbrd->mfg_info, "Cisco Systems Inc", 
			IPMI_SPROM_BOARD_MFG_INFO_SIZE);

        pbrd->product_name_tl = IPMI_SPROM_TYPE_CODE_LC_MSK |
                                IPMI_SPROM_BOARD_PRODUCT_NAME_SIZE;

        pbrd->part_num_tl     = IPMI_SPROM_TYPE_CODE_LC_MSK |
                                IPMI_SPROM_BOARD_PART_NUMBER_SIZE;

	sprom_mem_fill((char*)pbrd->part_num, "73-13195-03", 
			IPMI_SPROM_BOARD_PART_NUMBER_SIZE);
	sprom_mem_fill((char*)pbrd->product_name, 
			"N20-MEM", 
			IPMI_SPROM_BOARD_PRODUCT_NAME_SIZE);

        pbrd->fru_file_id_tl  = IPMI_SPROM_TYPE_CODE_LC_MSK |
				IPMI_SPROM_BOARD_FRU_FILE_ID_SIZE;
	sprom_mem_fill((char*)pbrd->fru_file_id, "40-2", 
			IPMI_SPROM_BOARD_FRU_FILE_ID_SIZE);
	
	pbrd->custom_id_tl    = IPMI_SPROM_TYPE_CODE_LC_MSK |
                                IPMI_SPROM_BOARD_CUSTOM_ID_SIZE;
        pbrd->clei_tl         = IPMI_SPROM_TYPE_CODE_LC_MSK |
                                IPMI_SPROM_BOARD_CLEI_SIZE;
	pbrd->bom_rev[0]      = '0';
	pbrd->bom_rev[1]      = '6';
	pbrd->hw_rev          = 0x03;
	pbrd->pid_rev[0]      = 'V';
	pbrd->pid_rev[1]      = '0';
	pbrd->pid_rev[2]      = '0';
        ipmi_zero_checksum_create((uint8_t*)pbrd, sizeof(*pbrd));

	return (0);
}


int ipmi_sprom_ventura_dflt(sprom_ipmi_bmc_t *sprom)
{
	ipmi_ibmc_internal_use_t *piu = (ipmi_ibmc_internal_use_t*)&sprom->internal_use;
	ipmi_sprom_board_info_t *pbrd = &sprom->board_info;
	ipmi_sprom_bmc_product_info_t *pprd = &sprom->product_info;

	memset (sprom, (int)NULL, sizeof(*sprom));
        ipmi_sprom_common_header_create(&sprom->common_header,
                M8_SIZE(sizeof(ipmi_ibmc_internal_use_t)),
                0,
                M8_SIZE(sizeof(ipmi_sprom_board_info_t)),
                M8_SIZE(sizeof(ipmi_sprom_bmc_product_info_t)),
                0);

	// Update Internal Use Area
        piu->version	= IPMI_SPROM_COMMON_HEADER_VERSION;
        piu->length	= M8_SIZE(sizeof(*piu));
        piu->card_type	= htons(IPMI_IU_CARD_TYPE_IBMC);

	// Update Mac Fields.
	piu->mac.srec.sub_type		= htons(IPMI_IU_SUB_TYPE_MAC_ADDR);
	piu->mac.srec.sub_type_len	= htons(sizeof(ipmi_iu_rec_mac_t) -
					  sizeof(ipmi_iu_rec_t));
	piu->mac.cnt.type		= IPMI_IU_ENTRY_TYPE_CNT; 
	piu->mac.cnt.len		= 2;
	piu->mac.cnt.value		= htons(0x01);
	
	piu->mac.type 			= IPMI_IU_ENTRY_TYPE_MAC;
	piu->mac.len 			= 6;
	memset(piu->mac.mac, 0, 6);
	piu->blade_class		= IPMI_SPROM_IBMC_IU_BLADE_CLASS_VENTURA;


	// Update Board Info
        pbrd->version         = IPMI_SPROM_BOARD_VERSION;
        pbrd->length          = M8_SIZE(sizeof(ipmi_sprom_board_info_t));
        pbrd->language_code   = IPMI_LC_ENGLISH_0;
        pbrd->no_more_tl      = IPMI_SPROM_NO_MORE_TYPE_LENGTH;
        pbrd->serial_num_tl   = IPMI_SPROM_TYPE_CODE_LC_MSK |
                                IPMI_SPROM_BOARD_SERIAL_NUMBER_SIZE;
        pbrd->mfg_info_tl     = IPMI_SPROM_TYPE_CODE_LC_MSK |
                                IPMI_SPROM_BOARD_MFG_INFO_SIZE;
	sprom_mem_fill((char*)pbrd->mfg_info, "Cisco Systems Inc", 
			IPMI_SPROM_BOARD_MFG_INFO_SIZE);

        pbrd->product_name_tl = IPMI_SPROM_TYPE_CODE_LC_MSK |
                                IPMI_SPROM_BOARD_PRODUCT_NAME_SIZE;

        pbrd->part_num_tl     = IPMI_SPROM_TYPE_CODE_LC_MSK |
                                IPMI_SPROM_BOARD_PART_NUMBER_SIZE;

	sprom_mem_fill((char*)pbrd->part_num, "73-12125-02", 
			IPMI_SPROM_BOARD_PART_NUMBER_SIZE);
	sprom_mem_fill((char*)pbrd->product_name, 
			"N20-B6620-2", 
			IPMI_SPROM_BOARD_PRODUCT_NAME_SIZE);

        pbrd->fru_file_id_tl  = IPMI_SPROM_TYPE_CODE_LC_MSK |
				IPMI_SPROM_BOARD_FRU_FILE_ID_SIZE;
        pbrd->custom_id_tl    = IPMI_SPROM_TYPE_CODE_LC_MSK |
                                IPMI_SPROM_BOARD_CUSTOM_ID_SIZE;
        pbrd->clei_tl         = IPMI_SPROM_TYPE_CODE_LC_MSK |
                                IPMI_SPROM_BOARD_CLEI_SIZE;
	pbrd->bom_rev[0]      = '0';
	pbrd->bom_rev[1]      = '0';
	pbrd->hw_rev          = 0x00;
	pbrd->pid_rev[0]      = 'V';
	pbrd->pid_rev[1]      = '0';
	pbrd->pid_rev[2]      = '0';
        ipmi_zero_checksum_create((uint8_t*)pbrd, sizeof(*pbrd));


	// Update Product Info
        pprd->version         = IPMI_SPROM_BOARD_VERSION;
        pprd->length          = M8_SIZE(sizeof(ipmi_sprom_bmc_product_info_t));
        pprd->language_code   = IPMI_LC_ENGLISH_0;
        pprd->no_more_tl      = IPMI_SPROM_NO_MORE_TYPE_LENGTH;
        pprd->serial_num_tl   = IPMI_SPROM_TYPE_CODE_LC_MSK |
                                IPMI_SPROM_BMC_PRODUCT_SERIAL_NUMBER_SIZE;
        pprd->mfg_name_tl     = IPMI_SPROM_TYPE_CODE_LC_MSK |
                                IPMI_SPROM_BMC_PRODUCT_MFG_NAME_SIZE;
	sprom_mem_fill((char*)pprd->mfg_name, "Cisco Systems Inc", 
			IPMI_SPROM_BMC_PRODUCT_MFG_NAME_SIZE);

        pprd->product_name_tl = IPMI_SPROM_TYPE_CODE_LC_MSK |
                                IPMI_SPROM_BMC_PRODUCT_NAME_SIZE;
	sprom_mem_fill((char*)pprd->product_name, 
			"N20-B6620-2", 
			IPMI_SPROM_BMC_PRODUCT_NAME_SIZE);

        pprd->part_model_tl     = IPMI_SPROM_TYPE_CODE_LC_MSK |
                                IPMI_SPROM_BMC_PRODUCT_PART_MODEL_SIZE;
	sprom_mem_fill((char*)pprd->part_model, "73-12125-02", 
			IPMI_SPROM_BMC_PRODUCT_PART_MODEL_SIZE);

        pprd->prd_version_tl     = IPMI_SPROM_TYPE_CODE_LC_MSK |
                                IPMI_SPROM_BMC_PRODUCT_VERSION_SIZE;
	pprd->prd_version = 1;

        pprd->asset_tag_tl    = IPMI_SPROM_TYPE_CODE_LC_MSK|
				IPMI_SPROM_BMC_PRODUCT_ASSET_TAG_SIZE;
	// Data is already cleared.

        pprd->fru_file_id_tl  = IPMI_SPROM_TYPE_CODE_LC_MSK |
				IPMI_SPROM_BMC_PRODUCT_FRU_FILE_ID_SIZE;
	sprom_mem_fill((char*)pprd->fru_file_id, "VENT_008", 
			IPMI_SPROM_BMC_PRODUCT_FRU_FILE_ID_SIZE);

        pprd->custom_id_tl    = IPMI_SPROM_TYPE_CODE_LC_MSK |
                                IPMI_SPROM_BMC_PRODUCT_CUSTOM_ID_SIZE;
	pprd->bom_rev[0]      = '1';
	pprd->bom_rev[1]      = '6';
	pprd->pid_rev[0]      = 'V';
	pprd->pid_rev[1]      = '0';
	pprd->pid_rev[2]      = '0';
        ipmi_zero_checksum_create((uint8_t*)pprd, sizeof(*pprd));

	return (0);
}

int ipmi_sprom_sf_dflt(sprom_ipmi_bmc_t *sprom)
{
	ipmi_ibmc_internal_use_t *piu = (ipmi_ibmc_internal_use_t*)&sprom->internal_use;
	ipmi_sprom_board_info_t *pbrd = &sprom->board_info;
	ipmi_sprom_bmc_product_info_t *pprd = &sprom->product_info;

	memset (sprom, (int)NULL, sizeof(*sprom));
        ipmi_sprom_common_header_create(&sprom->common_header,
                M8_SIZE(sizeof(ipmi_ibmc_internal_use_t)),
                0,
                M8_SIZE(sizeof(ipmi_sprom_board_info_t)),
                M8_SIZE(sizeof(ipmi_sprom_bmc_product_info_t)),
                0);

	// Update Internal Use Area
        piu->version	= IPMI_SPROM_COMMON_HEADER_VERSION;
        piu->length	= M8_SIZE(sizeof(*piu));
        piu->card_type	= htons(IPMI_IU_CARD_TYPE_IBMC);

	// Update Mac Fields.
	piu->mac.srec.sub_type		= htons(IPMI_IU_SUB_TYPE_MAC_ADDR);
	piu->mac.srec.sub_type_len	= htons(sizeof(ipmi_iu_rec_mac_t) -
					  sizeof(ipmi_iu_rec_t));
	piu->mac.cnt.type		= IPMI_IU_ENTRY_TYPE_CNT; 
	piu->mac.cnt.len		= 2;
	piu->mac.cnt.value		= htons(0x01);
	
	piu->mac.type 			= IPMI_IU_ENTRY_TYPE_MAC;
	piu->mac.len 			= 6;
	memset(piu->mac.mac, 0, 6);
	piu->blade_class		= IPMI_SPROM_IBMC_IU_BLADE_CLASS_VENTURA;


	// Update Board Info
        pbrd->version         = IPMI_SPROM_BOARD_VERSION;
        pbrd->length          = M8_SIZE(sizeof(ipmi_sprom_board_info_t));
        pbrd->language_code   = IPMI_LC_ENGLISH_0;
        pbrd->no_more_tl      = IPMI_SPROM_NO_MORE_TYPE_LENGTH;
        pbrd->serial_num_tl   = IPMI_SPROM_TYPE_CODE_LC_MSK |
                                IPMI_SPROM_BOARD_SERIAL_NUMBER_SIZE;
        pbrd->mfg_info_tl     = IPMI_SPROM_TYPE_CODE_LC_MSK |
                                IPMI_SPROM_BOARD_MFG_INFO_SIZE;
	sprom_mem_fill((char*)pbrd->mfg_info, "Cisco Systems Inc", 
			IPMI_SPROM_BOARD_MFG_INFO_SIZE);

        pbrd->product_name_tl = IPMI_SPROM_TYPE_CODE_LC_MSK |
                                IPMI_SPROM_BOARD_PRODUCT_NAME_SIZE;

        pbrd->part_num_tl     = IPMI_SPROM_TYPE_CODE_LC_MSK |
                                IPMI_SPROM_BOARD_PART_NUMBER_SIZE;

	sprom_mem_fill((char*)pbrd->part_num, "73-12462-05", 
			IPMI_SPROM_BOARD_PART_NUMBER_SIZE);
	sprom_mem_fill((char*)pbrd->product_name, 
			"N20-B6740-2", 
			IPMI_SPROM_BOARD_PRODUCT_NAME_SIZE);

        pbrd->fru_file_id_tl  = IPMI_SPROM_TYPE_CODE_LC_MSK |
				IPMI_SPROM_BOARD_FRU_FILE_ID_SIZE;
	sprom_mem_fill((char*)pbrd->fru_file_id, "40-2",
			IPMI_SPROM_BOARD_FRU_FILE_ID_SIZE);

        pbrd->custom_id_tl    = IPMI_SPROM_TYPE_CODE_LC_MSK |
                                IPMI_SPROM_BOARD_CUSTOM_ID_SIZE;
        pbrd->clei_tl         = IPMI_SPROM_TYPE_CODE_LC_MSK |
                                IPMI_SPROM_BOARD_CLEI_SIZE;
	pbrd->bom_rev[0]      = '0';
	pbrd->bom_rev[1]      = '2';
	pbrd->hw_rev          = 0x05;
	pbrd->pid_rev[0]      = 'V';
	pbrd->pid_rev[1]      = '0';
	pbrd->pid_rev[2]      = '1';
        ipmi_zero_checksum_create((uint8_t*)pbrd, sizeof(*pbrd));


	// Update Product Info
        pprd->version         = IPMI_SPROM_BOARD_VERSION;
        pprd->length          = M8_SIZE(sizeof(ipmi_sprom_bmc_product_info_t));
        pprd->language_code   = IPMI_LC_ENGLISH_0;
        pprd->no_more_tl      = IPMI_SPROM_NO_MORE_TYPE_LENGTH;
        pprd->serial_num_tl   = IPMI_SPROM_TYPE_CODE_LC_MSK |
                                IPMI_SPROM_BMC_PRODUCT_SERIAL_NUMBER_SIZE;
        pprd->mfg_name_tl     = IPMI_SPROM_TYPE_CODE_LC_MSK |
                                IPMI_SPROM_BMC_PRODUCT_MFG_NAME_SIZE;
	sprom_mem_fill((char*)pprd->mfg_name, "Cisco Systems Inc", 
			IPMI_SPROM_BMC_PRODUCT_MFG_NAME_SIZE);

        pprd->product_name_tl = IPMI_SPROM_TYPE_CODE_LC_MSK |
                                IPMI_SPROM_BMC_PRODUCT_NAME_SIZE;
	sprom_mem_fill((char*)pprd->product_name, 
			"N20-B6740-2", 
			IPMI_SPROM_BMC_PRODUCT_NAME_SIZE);

        pprd->part_model_tl     = IPMI_SPROM_TYPE_CODE_LC_MSK |
                                IPMI_SPROM_BMC_PRODUCT_PART_MODEL_SIZE;
	sprom_mem_fill((char*)pprd->part_model, "73-12462-05", 
			IPMI_SPROM_BMC_PRODUCT_PART_MODEL_SIZE);

        pprd->prd_version_tl     = IPMI_SPROM_TYPE_CODE_LC_MSK |
                                IPMI_SPROM_BMC_PRODUCT_VERSION_SIZE;
	pprd->prd_version = 1;

        pprd->asset_tag_tl    = IPMI_SPROM_TYPE_CODE_LC_MSK|
				IPMI_SPROM_BMC_PRODUCT_ASSET_TAG_SIZE;
	// Data is already cleared.

        pprd->fru_file_id_tl  = IPMI_SPROM_TYPE_CODE_LC_MSK |
				IPMI_SPROM_BMC_PRODUCT_FRU_FILE_ID_SIZE;
	sprom_mem_fill((char*)pprd->fru_file_id, "B6740-2", 
			IPMI_SPROM_BMC_PRODUCT_FRU_FILE_ID_SIZE);

        pprd->custom_id_tl    = IPMI_SPROM_TYPE_CODE_LC_MSK |
                                IPMI_SPROM_BMC_PRODUCT_CUSTOM_ID_SIZE;
	pprd->bom_rev[0]      = '0';
	pprd->bom_rev[1]      = '2';
	pprd->pid_rev[0]      = 'V';
	pprd->pid_rev[1]      = '0';
	pprd->pid_rev[2]      = '1';
        ipmi_zero_checksum_create((uint8_t*)pprd, sizeof(*pprd));

	return (0);
}

#define BOARD_TYPE_FNAME        "/var/boardtype"
#define SD_1U_PART_NUM          "74-6840-01"
#define SD_1U_PRODUCT_NAME      "R200-1120402"
#define SD_1U_FRU_FILE_ID       "SD1U_001"
#define SD_2U_PART_NUM          "74-6882-01"
#define SD_2U_PRODUCT_NAME      "R210-2121605"
#define SD_2U_FRU_FILE_ID       "SD2U_001"

int sd_get_boardtype() {
    FILE *fp = fopen(BOARD_TYPE_FNAME, "r");
    int btype = 0;

    if (!fp) {
        return -1;
    }
    fscanf(fp, "%d", &btype);
    fclose(fp);

    return btype;
}

int ipmi_sprom_sandiego_dflt(sprom_ipmi_bmc_t *sprom)
{
	ipmi_ibmc_internal_use_t *piu = (ipmi_ibmc_internal_use_t*)&sprom->internal_use;
	ipmi_sprom_board_info_t *pbrd = &sprom->board_info;
	ipmi_sprom_bmc_product_info_t *pprd = &sprom->product_info;
    int board_type = sd_get_boardtype();

    if (board_type < 0) {
        board_type = 1;     // Assume 1U by default
    }
    char *part_num = (board_type == 1) ? SD_1U_PART_NUM : SD_2U_PART_NUM;
    char *product_name = (board_type == 1) ? SD_1U_PRODUCT_NAME : SD_2U_PRODUCT_NAME;
    char *fru_file_id = (board_type == 1) ? SD_1U_FRU_FILE_ID : SD_2U_FRU_FILE_ID;

	memset (sprom, (int)NULL, sizeof(*sprom));
        ipmi_sprom_common_header_create(&sprom->common_header,
                M8_SIZE(sizeof(ipmi_ibmc_internal_use_t)),
                0,
                M8_SIZE(sizeof(ipmi_sprom_board_info_t)),
                M8_SIZE(sizeof(ipmi_sprom_bmc_product_info_t)),
                0);

	// Update Internal Use Area
        piu->version	= IPMI_SPROM_COMMON_HEADER_VERSION;
        piu->length	= M8_SIZE(sizeof(*piu));
        piu->card_type	= htons(IPMI_IU_CARD_TYPE_IBMC);

	// Update Mac Fields.
	piu->mac.srec.sub_type		= htons(IPMI_IU_SUB_TYPE_MAC_ADDR);
	piu->mac.srec.sub_type_len	= htons(sizeof(ipmi_iu_rec_mac_t) -
					  sizeof(ipmi_iu_rec_t));
	piu->mac.cnt.type		= IPMI_IU_ENTRY_TYPE_CNT; 
	piu->mac.cnt.len		= 2;
	piu->mac.cnt.value		= htons(0x03);
	
	piu->mac.type 			= IPMI_IU_ENTRY_TYPE_MAC;
	piu->mac.len 			= 6;
	memset(piu->mac.mac, 0, 6);
	piu->blade_class		= IPMI_SPROM_IBMC_IU_BLADE_CLASS_VENTURA;


	// Update Board Info
        pbrd->version         = IPMI_SPROM_BOARD_VERSION;
        pbrd->length          = M8_SIZE(sizeof(ipmi_sprom_board_info_t));
        pbrd->language_code   = IPMI_LC_ENGLISH_0;
        pbrd->no_more_tl      = IPMI_SPROM_NO_MORE_TYPE_LENGTH;
        pbrd->serial_num_tl   = IPMI_SPROM_TYPE_CODE_LC_MSK |
                                IPMI_SPROM_BOARD_SERIAL_NUMBER_SIZE;
        pbrd->mfg_info_tl     = IPMI_SPROM_TYPE_CODE_LC_MSK |
                                IPMI_SPROM_BOARD_MFG_INFO_SIZE;
	sprom_mem_fill((char*)pbrd->mfg_info, "Cisco Systems Inc", 
			IPMI_SPROM_BOARD_MFG_INFO_SIZE);

        pbrd->product_name_tl = IPMI_SPROM_TYPE_CODE_LC_MSK |
                                IPMI_SPROM_BOARD_PRODUCT_NAME_SIZE;

        pbrd->part_num_tl     = IPMI_SPROM_TYPE_CODE_LC_MSK |
                                IPMI_SPROM_BOARD_PART_NUMBER_SIZE;

	sprom_mem_fill((char*)pbrd->part_num, part_num,
			IPMI_SPROM_BOARD_PART_NUMBER_SIZE);
	sprom_mem_fill((char*)pbrd->product_name, 
            product_name,
			IPMI_SPROM_BOARD_PRODUCT_NAME_SIZE);

        pbrd->fru_file_id_tl  = IPMI_SPROM_TYPE_CODE_LC_MSK |
				IPMI_SPROM_BOARD_FRU_FILE_ID_SIZE;
        pbrd->custom_id_tl    = IPMI_SPROM_TYPE_CODE_LC_MSK |
                                IPMI_SPROM_BOARD_CUSTOM_ID_SIZE;
        pbrd->clei_tl         = IPMI_SPROM_TYPE_CODE_LC_MSK |
                                IPMI_SPROM_BOARD_CLEI_SIZE;
	pbrd->bom_rev[0]      = '0';
	pbrd->bom_rev[1]      = '1';
	pbrd->hw_rev          = 0x00;
	pbrd->pid_rev[0]      = 'V';
	pbrd->pid_rev[1]      = '0';
	pbrd->pid_rev[2]      = '0';
        ipmi_zero_checksum_create((uint8_t*)pbrd, sizeof(*pbrd));


	// Update Product Info
        pprd->version         = IPMI_SPROM_BOARD_VERSION;
        pprd->length          = M8_SIZE(sizeof(ipmi_sprom_bmc_product_info_t));
        pprd->language_code   = IPMI_LC_ENGLISH_0;
        pprd->no_more_tl      = IPMI_SPROM_NO_MORE_TYPE_LENGTH;
        pprd->serial_num_tl   = IPMI_SPROM_TYPE_CODE_LC_MSK |
                                IPMI_SPROM_BMC_PRODUCT_SERIAL_NUMBER_SIZE;
        pprd->mfg_name_tl     = IPMI_SPROM_TYPE_CODE_LC_MSK |
                                IPMI_SPROM_BMC_PRODUCT_MFG_NAME_SIZE;
	sprom_mem_fill((char*)pprd->mfg_name, "Cisco Systems Inc", 
			IPMI_SPROM_BMC_PRODUCT_MFG_NAME_SIZE);

        pprd->product_name_tl = IPMI_SPROM_TYPE_CODE_LC_MSK |
                                IPMI_SPROM_BMC_PRODUCT_NAME_SIZE;
	sprom_mem_fill((char*)pprd->product_name, 
            product_name,
			IPMI_SPROM_BMC_PRODUCT_NAME_SIZE);

        pprd->part_model_tl     = IPMI_SPROM_TYPE_CODE_LC_MSK |
                                IPMI_SPROM_BMC_PRODUCT_PART_MODEL_SIZE;
	sprom_mem_fill((char*)pprd->part_model, part_num,
			IPMI_SPROM_BMC_PRODUCT_PART_MODEL_SIZE);

        pprd->prd_version_tl     = IPMI_SPROM_TYPE_CODE_LC_MSK |
                                IPMI_SPROM_BMC_PRODUCT_VERSION_SIZE;
	pprd->prd_version = 1;

        pprd->asset_tag_tl    = IPMI_SPROM_TYPE_CODE_LC_MSK|
				IPMI_SPROM_BMC_PRODUCT_ASSET_TAG_SIZE;
	// Data is already cleared.

        pprd->fru_file_id_tl  = IPMI_SPROM_TYPE_CODE_LC_MSK |
				IPMI_SPROM_BMC_PRODUCT_FRU_FILE_ID_SIZE;
	sprom_mem_fill((char*)pprd->fru_file_id, fru_file_id,
			IPMI_SPROM_BMC_PRODUCT_FRU_FILE_ID_SIZE);

        pprd->custom_id_tl    = IPMI_SPROM_TYPE_CODE_LC_MSK |
                                IPMI_SPROM_BMC_PRODUCT_CUSTOM_ID_SIZE;
	pprd->bom_rev[0]      = '0';
	pprd->bom_rev[1]      = '1';
	pprd->pid_rev[0]      = 'V';
	pprd->pid_rev[1]      = '0';
	pprd->pid_rev[2]      = '0';
        ipmi_zero_checksum_create((uint8_t*)pprd, sizeof(*pprd));

	return (0);
}

int ipmi_sprom_alpine_dflt(sprom_ipmi_bmc_t *sprom)
{
	ipmi_ibmc_internal_use_t *piu = (ipmi_ibmc_internal_use_t*)&sprom->internal_use;
	ipmi_sprom_board_info_t *pbrd = &sprom->board_info;
	ipmi_sprom_bmc_product_info_t *pprd = &sprom->product_info;

	memset (sprom, (int)NULL, sizeof(*sprom));
        ipmi_sprom_common_header_create(&sprom->common_header,
                M8_SIZE(sizeof(ipmi_ibmc_internal_use_t)),
                0,
                M8_SIZE(sizeof(ipmi_sprom_board_info_t)),
                M8_SIZE(sizeof(ipmi_sprom_bmc_product_info_t)),
                0);

	// Update Internal Use Area
        piu->version	= IPMI_SPROM_COMMON_HEADER_VERSION;
        piu->length	= M8_SIZE(sizeof(*piu));
        piu->card_type	= htons(IPMI_IU_CARD_TYPE_IBMC);

	// Update Mac Fields.
	piu->mac.srec.sub_type		= htons(IPMI_IU_SUB_TYPE_MAC_ADDR);
	piu->mac.srec.sub_type_len	= htons(sizeof(ipmi_iu_rec_mac_t) -
					  sizeof(ipmi_iu_rec_t));
	piu->mac.cnt.type		= IPMI_IU_ENTRY_TYPE_CNT; 
	piu->mac.cnt.len		= 2;
	piu->mac.cnt.value		= htons(0x03);
	
	piu->mac.type 			= IPMI_IU_ENTRY_TYPE_MAC;
	piu->mac.len 			= 6;
	memset(piu->mac.mac, 0, 6);
	piu->blade_class		= IPMI_SPROM_IBMC_IU_BLADE_CLASS_NONE;


	// Update Board Info
        pbrd->version         = IPMI_SPROM_BOARD_VERSION;
        pbrd->length          = M8_SIZE(sizeof(ipmi_sprom_board_info_t));
        pbrd->language_code   = IPMI_LC_ENGLISH_0;
        pbrd->no_more_tl      = IPMI_SPROM_NO_MORE_TYPE_LENGTH;
        pbrd->serial_num_tl   = IPMI_SPROM_TYPE_CODE_LC_MSK |
                                IPMI_SPROM_BOARD_SERIAL_NUMBER_SIZE;
        pbrd->mfg_info_tl     = IPMI_SPROM_TYPE_CODE_LC_MSK |
                                IPMI_SPROM_BOARD_MFG_INFO_SIZE;
	sprom_mem_fill((char*)pbrd->mfg_info, "Cisco Systems Inc", 
			IPMI_SPROM_BOARD_MFG_INFO_SIZE);

        pbrd->product_name_tl = IPMI_SPROM_TYPE_CODE_LC_MSK |
                                IPMI_SPROM_BOARD_PRODUCT_NAME_SIZE;

        pbrd->part_num_tl     = IPMI_SPROM_TYPE_CODE_LC_MSK |
                                IPMI_SPROM_BOARD_PART_NUMBER_SIZE;

	sprom_mem_fill((char*)pbrd->part_num, "74-6807-01", 
			IPMI_SPROM_BOARD_PART_NUMBER_SIZE);
	sprom_mem_fill((char*)pbrd->product_name, 
			"R400-4640810", 
			IPMI_SPROM_BOARD_PRODUCT_NAME_SIZE);

        pbrd->fru_file_id_tl  = IPMI_SPROM_TYPE_CODE_LC_MSK |
				IPMI_SPROM_BOARD_FRU_FILE_ID_SIZE;
        pbrd->custom_id_tl    = IPMI_SPROM_TYPE_CODE_LC_MSK |
                                IPMI_SPROM_BOARD_CUSTOM_ID_SIZE;
        pbrd->clei_tl         = IPMI_SPROM_TYPE_CODE_LC_MSK |
                                IPMI_SPROM_BOARD_CLEI_SIZE;
	pbrd->bom_rev[0]      = '0';
	pbrd->bom_rev[1]      = '0';
	pbrd->hw_rev          = 0x00;
	pbrd->pid_rev[0]      = 'V';
	pbrd->pid_rev[1]      = '0';
	pbrd->pid_rev[2]      = '0';
        ipmi_zero_checksum_create((uint8_t*)pbrd, sizeof(*pbrd));


	// Update Product Info
        pprd->version         = IPMI_SPROM_BOARD_VERSION;
        pprd->length          = M8_SIZE(sizeof(ipmi_sprom_bmc_product_info_t));
        pprd->language_code   = IPMI_LC_ENGLISH_0;
        pprd->no_more_tl      = IPMI_SPROM_NO_MORE_TYPE_LENGTH;
        pprd->serial_num_tl   = IPMI_SPROM_TYPE_CODE_LC_MSK |
                                IPMI_SPROM_BMC_PRODUCT_SERIAL_NUMBER_SIZE;
        pprd->mfg_name_tl     = IPMI_SPROM_TYPE_CODE_LC_MSK |
                                IPMI_SPROM_BMC_PRODUCT_MFG_NAME_SIZE;
	sprom_mem_fill((char*)pprd->mfg_name, "Cisco Systems Inc", 
			IPMI_SPROM_BMC_PRODUCT_MFG_NAME_SIZE);

        pprd->product_name_tl = IPMI_SPROM_TYPE_CODE_LC_MSK |
                                IPMI_SPROM_BMC_PRODUCT_NAME_SIZE;
	sprom_mem_fill((char*)pprd->product_name, 
			"R400-4640810", 
			IPMI_SPROM_BMC_PRODUCT_NAME_SIZE);

        pprd->part_model_tl     = IPMI_SPROM_TYPE_CODE_LC_MSK |
                                IPMI_SPROM_BMC_PRODUCT_PART_MODEL_SIZE;
	sprom_mem_fill((char*)pprd->part_model, "74-6807-01", 
			IPMI_SPROM_BMC_PRODUCT_PART_MODEL_SIZE);

        pprd->prd_version_tl     = IPMI_SPROM_TYPE_CODE_LC_MSK |
                                IPMI_SPROM_BMC_PRODUCT_VERSION_SIZE;
	pprd->prd_version = 1;

        pprd->asset_tag_tl    = IPMI_SPROM_TYPE_CODE_LC_MSK|
				IPMI_SPROM_BMC_PRODUCT_ASSET_TAG_SIZE;
	// Data is already cleared.

        pprd->fru_file_id_tl  = IPMI_SPROM_TYPE_CODE_LC_MSK |
				IPMI_SPROM_BMC_PRODUCT_FRU_FILE_ID_SIZE;
	sprom_mem_fill((char*)pprd->fru_file_id, "ALPN_001", 
			IPMI_SPROM_BMC_PRODUCT_FRU_FILE_ID_SIZE);

        pprd->custom_id_tl    = IPMI_SPROM_TYPE_CODE_LC_MSK |
                                IPMI_SPROM_BMC_PRODUCT_CUSTOM_ID_SIZE;
	pprd->bom_rev[0]      = '0';
	pprd->bom_rev[1]      = '1';
	pprd->pid_rev[0]      = 'V';
	pprd->pid_rev[1]      = '0';
	pprd->pid_rev[2]      = '0';
        ipmi_zero_checksum_create((uint8_t*)pprd, sizeof(*pprd));

	return (0);
}

int ipmi_sprom_bmc_dflt(sprom_ipmi_bmc_t *sprom)
{
	ipmi_ibmc_internal_use_t *piu = (ipmi_ibmc_internal_use_t*)&sprom->internal_use;
	ipmi_sprom_board_info_t *pbrd = (ipmi_sprom_board_info_t*)&sprom->board_info;
	ipmi_sprom_bmc_product_info_t *pprd = &sprom->product_info;

	memset (sprom, (int)NULL, sizeof(*sprom));
        ipmi_sprom_common_header_create(&sprom->common_header,
                M8_SIZE(sizeof(ipmi_ibmc_internal_use_t)),
                0,
                M8_SIZE(sizeof(ipmi_sprom_board_info_t)),
                M8_SIZE(sizeof(ipmi_sprom_bmc_product_info_t)),
                0);

	// Update Internal Use Area
        piu->version	= IPMI_SPROM_COMMON_HEADER_VERSION;
        piu->length	= M8_SIZE(sizeof(*piu));
        piu->card_type	= htons(IPMI_IU_CARD_TYPE_IBMC);

	// Update Mac Fields.
	piu->mac.srec.sub_type		= htons(IPMI_IU_SUB_TYPE_MAC_ADDR);
	piu->mac.srec.sub_type_len	= htons(sizeof(ipmi_iu_rec_mac_t) -
					  sizeof(ipmi_iu_rec_t));
	piu->mac.cnt.type		= IPMI_IU_ENTRY_TYPE_CNT; 
	piu->mac.cnt.len		= 2;
	piu->mac.cnt.value		= htons(0x01);
	
	piu->mac.type 			= IPMI_IU_ENTRY_TYPE_MAC;
	piu->mac.len 			= 6;
	memset(piu->mac.mac, 0, 6);

	/* Update TPM Enable */
	piu->blade_class = IPMI_SPROM_IBMC_IU_BLADE_CLASS_GOODING;
 
	/* Update TPM Enable */
	piu->tpm = 0;

    /* Update Windows activity bit */
    piu->window_active = 0;

	/* Update UUID */
	memset(piu->uuid, 0, IU_UUID_SIZE);


	// Update Board Info
        pbrd->version         = IPMI_SPROM_BOARD_VERSION;
        pbrd->length          = M8_SIZE(sizeof(ipmi_sprom_board_info_t));
        pbrd->language_code   = IPMI_LC_ENGLISH_0;
        pbrd->no_more_tl      = IPMI_SPROM_NO_MORE_TYPE_LENGTH;
        pbrd->serial_num_tl   = IPMI_SPROM_TYPE_CODE_LC_MSK |
                                IPMI_SPROM_BOARD_SERIAL_NUMBER_SIZE;
        pbrd->mfg_info_tl     = IPMI_SPROM_TYPE_CODE_LC_MSK |
                                IPMI_SPROM_BOARD_MFG_INFO_SIZE;
	sprom_mem_fill((char*)pbrd->mfg_info, "Cisco Systems Inc", 
			IPMI_SPROM_BOARD_MFG_INFO_SIZE);

        pbrd->product_name_tl = IPMI_SPROM_TYPE_CODE_LC_MSK |
                                IPMI_SPROM_BOARD_PRODUCT_NAME_SIZE;

        pbrd->part_num_tl     = IPMI_SPROM_TYPE_CODE_LC_MSK |
                                IPMI_SPROM_BOARD_PART_NUMBER_SIZE;

	sprom_mem_fill((char*)pbrd->part_num, "74-10422-01", 
			IPMI_SPROM_BOARD_PART_NUMBER_SIZE);
	
    sprom_mem_fill((char*)pbrd->product_name, 
			"CSX-1006-P", 
			IPMI_SPROM_BOARD_PRODUCT_NAME_SIZE);

        pbrd->fru_file_id_tl  = IPMI_SPROM_TYPE_CODE_LC_MSK |
				IPMI_SPROM_BOARD_FRU_FILE_ID_SIZE;
	sprom_mem_fill((char*)pbrd->fru_file_id, 
			"25-1", IPMI_SPROM_BOARD_FRU_FILE_ID_SIZE);

        pbrd->custom_id_tl    = IPMI_SPROM_TYPE_CODE_LC_MSK |
                                IPMI_SPROM_BOARD_CUSTOM_ID_SIZE;
        pbrd->clei_tl         = IPMI_SPROM_TYPE_CODE_LC_MSK |
                                IPMI_SPROM_BOARD_CLEI_SIZE;
        pbrd->pad_tl          = IPMI_SPROM_TYPE_CODE_LC_MSK |
        						IPMI_SPROM_BOARD_NIM_PAD_SIZE;
	pbrd->bom_rev[0]      = '0';
	pbrd->hw_rev          = 0x00;
	pbrd->pid_rev[0]      = 'V';
	pbrd->pid_rev[1]      = '0';
	pbrd->pid_rev[2]      = '0';
        ipmi_zero_checksum_create((uint8_t*)pbrd, sizeof(*pbrd));


	// Update Product Info
        pprd->version         = IPMI_SPROM_BOARD_VERSION;
        pprd->length          = M8_SIZE(sizeof(ipmi_sprom_bmc_product_info_t));
        pprd->language_code   = IPMI_LC_ENGLISH_0;
        pprd->no_more_tl      = IPMI_SPROM_NO_MORE_TYPE_LENGTH;
        pprd->serial_num_tl   = IPMI_SPROM_TYPE_CODE_LC_MSK |
                                IPMI_SPROM_BMC_PRODUCT_SERIAL_NUMBER_SIZE;
        pprd->mfg_name_tl     = IPMI_SPROM_TYPE_CODE_LC_MSK |
                                IPMI_SPROM_BMC_PRODUCT_MFG_NAME_SIZE;
	sprom_mem_fill((char*)pprd->mfg_name, "Cisco Systems Inc", 
			IPMI_SPROM_BMC_PRODUCT_MFG_NAME_SIZE);

        pprd->product_name_tl = IPMI_SPROM_TYPE_CODE_LC_MSK |
                                IPMI_SPROM_BMC_PRODUCT_NAME_SIZE;
    sprom_mem_fill((char*)pprd->product_name, 
			"CSX-1006-P", 
			IPMI_SPROM_BMC_PRODUCT_NAME_SIZE);
        pprd->part_model_tl     = IPMI_SPROM_TYPE_CODE_LC_MSK |
                                IPMI_SPROM_BMC_PRODUCT_PART_MODEL_SIZE;
	sprom_mem_fill((char*)pprd->part_model, "74-10422-01", 
			IPMI_SPROM_BMC_PRODUCT_PART_MODEL_SIZE);

        pprd->prd_version_tl     = IPMI_SPROM_TYPE_CODE_LC_MSK |
                                IPMI_SPROM_BMC_PRODUCT_VERSION_SIZE;
	pprd->prd_version = 1;

        pprd->asset_tag_tl    = IPMI_SPROM_TYPE_CODE_LC_MSK|
				IPMI_SPROM_BMC_PRODUCT_ASSET_TAG_SIZE;
	// Data is already cleared.

        pprd->fru_file_id_tl  = IPMI_SPROM_TYPE_CODE_LC_MSK |
				IPMI_SPROM_BMC_PRODUCT_FRU_FILE_ID_SIZE;
	sprom_mem_fill((char*)pbrd->fru_file_id, 
			"E140S", IPMI_SPROM_BMC_PRODUCT_FRU_FILE_ID_SIZE);

        pprd->custom_id_tl    = IPMI_SPROM_TYPE_CODE_LC_MSK |
                                IPMI_SPROM_BMC_PRODUCT_CUSTOM_ID_SIZE;
	pprd->bom_rev[0]      = '1';
	pprd->bom_rev[1]      = '6';
	pprd->pid_rev[0]      = 'V';
	pprd->pid_rev[1]      = '0';
	pprd->pid_rev[2]      = '0';
	pprd->pad_tl    = IPMI_SPROM_TYPE_CODE_LC_MSK |
    			IPMI_SPROM_BMC_PRODUCT_NIM_PAD_SIZE;
        ipmi_zero_checksum_create((uint8_t*)pprd, sizeof(*pprd));

	return (0);
}

int ipmi_sprom_mezz_dflt(sprom_ipmi_mezz_t *sprom, uint8_t mezz_type)
{
	uint8_t multi_rec_size = 0;

	ipmi_mezz_internal_use_t *piu = &sprom->internal_use;
	ipmi_sprom_board_info_t *pbrd = (ipmi_sprom_board_info_t*)&sprom->board_info;

	if (mezz_type == IPMI_IU_CARD_TYPE_MONTEREYPARK)
	    multi_rec_size = M8_SIZE(sizeof (ipmi_ncsi_multi_record_t));

        ipmi_sprom_common_header_create(&sprom->common_header,
                M8_SIZE(sizeof(ipmi_mezz_internal_use_t)),
                0,
                M8_SIZE(sizeof(ipmi_sprom_board_info_t)),
                0,
		multi_rec_size);

	// Update Internal Use Area
        piu->version	= IPMI_SPROM_COMMON_HEADER_VERSION;
        piu->length	= M8_SIZE(sizeof(*piu));
        piu->card_type	= htons(mezz_type);

	// Update Mac Fields.
	piu->mac.srec.sub_type		= htons(IPMI_IU_SUB_TYPE_MAC_ADDR);
	piu->mac.srec.sub_type_len	= htons(sizeof(ipmi_iu_rec_mac_t) -
					  sizeof(ipmi_iu_rec_t));
	piu->mac.cnt.type		= IPMI_IU_ENTRY_TYPE_CNT; 
	piu->mac.cnt.len		= 2;
	piu->mac.cnt.value		= htons(0x01);
	
	piu->mac.type 			= IPMI_IU_ENTRY_TYPE_MAC;
	piu->mac.len 			= 6;
	memset(piu->mac.mac, 0, 6);


	// Update Board Info
        pbrd->version         = IPMI_SPROM_BOARD_VERSION;
        pbrd->length          = M8_SIZE(sizeof(ipmi_sprom_board_info_t));
        pbrd->language_code   = IPMI_LC_ENGLISH_0;
        pbrd->no_more_tl      = IPMI_SPROM_NO_MORE_TYPE_LENGTH;
        pbrd->serial_num_tl   = IPMI_SPROM_TYPE_CODE_LC_MSK |
                                IPMI_SPROM_BOARD_SERIAL_NUMBER_SIZE;
        pbrd->mfg_info_tl     = IPMI_SPROM_TYPE_CODE_LC_MSK |
                                IPMI_SPROM_BOARD_MFG_INFO_SIZE;
	sprom_mem_fill((char*)pbrd->mfg_info, "Cisco Systems Inc", 
			IPMI_SPROM_BOARD_MFG_INFO_SIZE);

        pbrd->product_name_tl = IPMI_SPROM_TYPE_CODE_LC_MSK |
                                IPMI_SPROM_BOARD_PRODUCT_NAME_SIZE;

        pbrd->part_num_tl     = IPMI_SPROM_TYPE_CODE_LC_MSK |
                                IPMI_SPROM_BOARD_PART_NUMBER_SIZE;
	switch(mezz_type) {
		case 	IPMI_IU_CARD_TYPE_IBMC:
			sprom_mem_fill((char*)pbrd->part_num, "74-5390-01", 
				IPMI_SPROM_BOARD_PART_NUMBER_SIZE);
			sprom_mem_fill((char*)pbrd->product_name, 
				"N20-B6620-1", 
				IPMI_SPROM_BOARD_PRODUCT_NAME_SIZE);
			break;

		case 	IPMI_IU_CARD_TYPE_NIANTIC:
			piu->mac.cnt.value = htons(0x04);
			sprom_mem_fill((char*)pbrd->part_num, "74-7018-01", 
				IPMI_SPROM_BOARD_PART_NUMBER_SIZE);
			sprom_mem_fill((char*)pbrd->product_name, 
				"N20-AI0102", 
				IPMI_SPROM_BOARD_PRODUCT_NAME_SIZE);
			break;

		case 	IPMI_IU_CARD_TYPE_NETEFFECT:
			piu->mac.cnt.value = htons(0x02);
			sprom_mem_fill((char*)pbrd->part_num, "74-7020-01", 
				IPMI_SPROM_BOARD_PART_NUMBER_SIZE);
			sprom_mem_fill((char*)pbrd->product_name, 
				"N20-AI0202", 
				IPMI_SPROM_BOARD_PRODUCT_NAME_SIZE);
			break;

		case 	IPMI_IU_CARD_TYPE_SCHULTZ:
			piu->mac.cnt.value = htons(0x04);
			sprom_mem_fill((char*)pbrd->part_num, "74-7023-01", 
				IPMI_SPROM_BOARD_PART_NUMBER_SIZE);
			sprom_mem_fill((char*)pbrd->product_name, 
				"N20-AQ0102", 
				IPMI_SPROM_BOARD_PRODUCT_NAME_SIZE);
			break;

		case 	IPMI_IU_CARD_TYPE_TIGERSHARK:
			piu->mac.cnt.value = htons(0x04);
			sprom_mem_fill((char*)pbrd->part_num, "74-7019-01", 
				IPMI_SPROM_BOARD_PART_NUMBER_SIZE);
			sprom_mem_fill((char*)pbrd->product_name, 
				"N20-AE0102", 
				IPMI_SPROM_BOARD_PRODUCT_NAME_SIZE);
			break;

		case 	IPMI_IU_CARD_TYPE_EVEREST:
			piu->mac.cnt.value = htons(0x04);
			sprom_mem_fill((char*)pbrd->part_num, "74-7021-01", 
				IPMI_SPROM_BOARD_PART_NUMBER_SIZE);
			sprom_mem_fill((char*)pbrd->product_name, 
				"N20-AB0002", 
				IPMI_SPROM_BOARD_PRODUCT_NAME_SIZE);
			break;

		case 	IPMI_IU_CARD_TYPE_DUBLIN:
			piu->mac.cnt.value = htons(0x04);
			sprom_mem_fill((char*)pbrd->part_num, "73-13645-01", 
				IPMI_SPROM_BOARD_PART_NUMBER_SIZE);
			sprom_mem_fill((char*)pbrd->product_name, 
				"N20-AQ0202", 
				IPMI_SPROM_BOARD_PRODUCT_NAME_SIZE);
			break;

		case 	IPMI_IU_CARD_TYPE_FREMONT:
			piu->mac.cnt.value = htons(0x08);
			sprom_mem_fill((char*)pbrd->part_num, "73-13647-01", 
				IPMI_SPROM_BOARD_PART_NUMBER_SIZE);
			sprom_mem_fill((char*)pbrd->product_name, 
				"N20-AE0202", 
				IPMI_SPROM_BOARD_PRODUCT_NAME_SIZE);
			break;

		case 	IPMI_IU_CARD_TYPE_LIVERMORE:
			piu->mac.cnt.value = htons(0x04);
			sprom_mem_fill((char*)pbrd->part_num, "73-13646-01", 
				IPMI_SPROM_BOARD_PART_NUMBER_SIZE);
			sprom_mem_fill((char*)pbrd->product_name, 
				"N20-AB0102", 
				IPMI_SPROM_BOARD_PRODUCT_NAME_SIZE);
			break;

		case 	IPMI_IU_CARD_TYPE_PALO:
			piu->mac.cnt.value = htons(0x06);
			sprom_mem_fill((char*)pbrd->part_num, "73-11789-02", 
				IPMI_SPROM_BOARD_PART_NUMBER_SIZE);
			sprom_mem_fill((char*)pbrd->product_name, 
				"N20-AC0002", 
				IPMI_SPROM_BOARD_PRODUCT_NAME_SIZE);
			break;

		case	IPMI_IU_CARD_TYPE_MENLO:
			piu->mac.cnt.value = htons(0x06);
			sprom_mem_fill((char*)pbrd->part_num, "73-11643-03", 
				IPMI_SPROM_BOARD_PART_NUMBER_SIZE);
			sprom_mem_fill((char*)pbrd->product_name, 
				"N20-AQ0002", 
				IPMI_SPROM_BOARD_PRODUCT_NAME_SIZE);
			break;

		case	IPMI_IU_CARD_TYPE_MENLO_E:
			piu->mac.cnt.value = htons(0x06);
			sprom_mem_fill((char*)pbrd->part_num, "74-6430-01", 
				IPMI_SPROM_BOARD_PART_NUMBER_SIZE);
			sprom_mem_fill((char*)pbrd->product_name, 
				"N20-AE0002", 
				IPMI_SPROM_BOARD_PRODUCT_NAME_SIZE);
			break;

		case	IPMI_IU_CARD_TYPE_OPLIN:
			piu->mac.cnt.value = htons(0x02);
			sprom_mem_fill((char*)pbrd->part_num, "74-5391-01", 
				IPMI_SPROM_BOARD_PART_NUMBER_SIZE);
			sprom_mem_fill((char*)pbrd->product_name, 
				"N20-AI0002", 
				IPMI_SPROM_BOARD_PRODUCT_NAME_SIZE);
			break;

		case	IPMI_IU_CARD_TYPE_MONTEREYPARK:
			piu->mac.cnt.value = htons(21);
			sprom_mem_fill((char*)pbrd->part_num, "73-12522-04", 
				IPMI_SPROM_BOARD_PART_NUMBER_SIZE);
			sprom_mem_fill((char*)pbrd->product_name, 
				"N2XX-ACPCI01", 
				IPMI_SPROM_BOARD_PRODUCT_NAME_SIZE);
			break;

		case	IPMI_IU_CARD_TYPE_VASONA:
			sprom_mem_fill((char*)pbrd->part_num, "73-13416-01", 
				IPMI_SPROM_BOARD_PART_NUMBER_SIZE);
			sprom_mem_fill((char*)pbrd->product_name, 
				"UCS-VIC-M82-8P", 
				IPMI_SPROM_BOARD_PRODUCT_NAME_SIZE);
			break;

		case	IPMI_IU_CARD_TYPE_BBU:
			sprom_mem_fill((char*)pbrd->part_num, "74-XXXX-01", 
				IPMI_SPROM_BOARD_PART_NUMBER_SIZE);
			sprom_mem_fill((char*)pbrd->product_name, 
				"N20-AIXXXX", 
				IPMI_SPROM_BOARD_PRODUCT_NAME_SIZE);
			break;

		default:
			sprom_mem_fill((char*)pbrd->part_num, "          ", 
				IPMI_SPROM_BOARD_PART_NUMBER_SIZE);
			sprom_mem_fill((char*)pbrd->product_name, 
				"          ", 
				IPMI_SPROM_BOARD_PRODUCT_NAME_SIZE);
			break;

	}

        pbrd->fru_file_id_tl  = IPMI_SPROM_TYPE_CODE_LC_MSK |
				IPMI_SPROM_BOARD_FRU_FILE_ID_SIZE;
        pbrd->custom_id_tl    = IPMI_SPROM_TYPE_CODE_LC_MSK |
                                IPMI_SPROM_BOARD_CUSTOM_ID_SIZE;
        pbrd->clei_tl         = IPMI_SPROM_TYPE_CODE_LC_MSK |
                                IPMI_SPROM_BOARD_CLEI_SIZE;
	pbrd->bom_rev[0]      = '0';
	pbrd->bom_rev[1]      = '0';
	pbrd->hw_rev          = 0x00;
	pbrd->pid_rev[0]      = 'V';
	pbrd->pid_rev[1]      = '0';
	pbrd->pid_rev[2]      = '0';
        ipmi_zero_checksum_create((uint8_t*)pbrd, sizeof(*pbrd));

	return (0);
}

int ipmi_sprom_mpark_dflt(sprom_ipmi_mpark_t *sprom, uint8_t mezz_type)
{
	ipmi_ncsi_multi_record_t *pmr = &sprom->ncsi_multi_record;
	uint8_t oem[3] = IPMI_OEM_NCSI_CISCO;
	int rc;

	rc = ipmi_sprom_mezz_dflt((sprom_ipmi_mezz_t *) sprom, mezz_type);
	if (rc)
		return (rc);

	pmr->header.record_type = IPMI_SPROM_MULTI_RECORD_OEM_NCSI;
	pmr->header.version = IPMI_SPROM_MULTI_RECORD_VERSION |
	    IPMI_SPROM_MULTI_RECORD_EOL;
	pmr->header.length = sizeof (*pmr) - sizeof (pmr->header);

	memcpy(pmr->oem, oem, sizeof (pmr->oem));
	pmr->version = IPMI_OEM_NCSI_VERSION;
	pmr->subtype = htonl(IPMI_OEM_NCSI_SUBTYPE);
	pmr->power_source = IPMI_OEM_NCSI_POWER_SOURCE_ALT;
	pmr->peak_standby_power = htons(180);	/* 18.0W */
	pmr->mac_count = 2;
	memset(pmr->mac, 0, sizeof (pmr->mac));
	pmr->pad = 0;
	ipmi_multi_record_checksum_create(&pmr->header);

	return (0);
}

int ipmi_sprom_bbu_dflt(sprom_ipmi_bbu_t *sprom)
{
	ipmi_sprom_board_info_t *pbrd = (ipmi_sprom_board_info_t*)&sprom->board_info;
	ipmi_sprom_bmc_product_info_t *pprd = (ipmi_sprom_bmc_product_info_t*)&sprom->product_info;

        ipmi_sprom_common_header_create(&sprom->common_header,
                0,
                0,
                M8_SIZE(sizeof(ipmi_sprom_board_info_t)),
                M8_SIZE(sizeof(ipmi_sprom_bmc_product_info_t)),
                0);

	// Update Board Info
        pbrd->version         = IPMI_SPROM_BOARD_VERSION;
        pbrd->length          = M8_SIZE(sizeof(ipmi_sprom_board_info_t));
        pbrd->language_code   = IPMI_LC_ENGLISH_0;
        pbrd->no_more_tl      = IPMI_SPROM_NO_MORE_TYPE_LENGTH;
        pbrd->serial_num_tl   = IPMI_SPROM_TYPE_CODE_LC_MSK |
                                IPMI_SPROM_BOARD_SERIAL_NUMBER_SIZE;
        pbrd->mfg_info_tl     = IPMI_SPROM_TYPE_CODE_LC_MSK |
                                IPMI_SPROM_BOARD_MFG_INFO_SIZE;
	sprom_mem_fill((char*)pbrd->mfg_info, "Cisco Systems Inc", 
			IPMI_SPROM_BOARD_MFG_INFO_SIZE);

        pbrd->product_name_tl = IPMI_SPROM_TYPE_CODE_LC_MSK |
                                IPMI_SPROM_BOARD_PRODUCT_NAME_SIZE;

        pbrd->part_num_tl     = IPMI_SPROM_TYPE_CODE_LC_MSK |
                                IPMI_SPROM_BOARD_PART_NUMBER_SIZE;

	sprom_mem_fill((char*)pbrd->part_num, "73-12813-03", 
			IPMI_SPROM_BOARD_PART_NUMBER_SIZE);
	sprom_mem_fill((char*)pbrd->product_name, 
			"N20-LBBU", 
			IPMI_SPROM_BOARD_PRODUCT_NAME_SIZE);

        pbrd->fru_file_id_tl  = IPMI_SPROM_TYPE_CODE_LC_MSK |
				IPMI_SPROM_BOARD_FRU_FILE_ID_SIZE;
	sprom_mem_fill((char*)pbrd->fru_file_id, "LBBU",
			IPMI_SPROM_BOARD_FRU_FILE_ID_SIZE);

        pbrd->custom_id_tl    = IPMI_SPROM_TYPE_CODE_LC_MSK |
                                IPMI_SPROM_BOARD_CUSTOM_ID_SIZE;
        pbrd->clei_tl         = IPMI_SPROM_TYPE_CODE_LC_MSK |
                                IPMI_SPROM_BOARD_CLEI_SIZE;
	pbrd->bom_rev[0]      = '0';
	pbrd->bom_rev[1]      = '5';
	pbrd->hw_rev          = 0x01;
	pbrd->pid_rev[0]      = 'V';
	pbrd->pid_rev[1]      = '0';
	pbrd->pid_rev[2]      = '1';
        ipmi_zero_checksum_create((uint8_t*)pbrd, sizeof(*pbrd));

	// Update Product Info
        pprd->version         = IPMI_SPROM_BOARD_VERSION;
        pprd->length          = M8_SIZE(sizeof(ipmi_sprom_bmc_product_info_t));
        pprd->language_code   = IPMI_LC_ENGLISH_0;
        pprd->no_more_tl      = IPMI_SPROM_NO_MORE_TYPE_LENGTH;
        pprd->serial_num_tl   = IPMI_SPROM_TYPE_CODE_LC_MSK |
                                IPMI_SPROM_BMC_PRODUCT_SERIAL_NUMBER_SIZE;
        pprd->mfg_name_tl     = IPMI_SPROM_TYPE_CODE_LC_MSK |
                                IPMI_SPROM_BMC_PRODUCT_MFG_NAME_SIZE;
	sprom_mem_fill((char*)pprd->mfg_name, "Cisco Systems Inc", 
			IPMI_SPROM_BMC_PRODUCT_MFG_NAME_SIZE);

        pprd->product_name_tl = IPMI_SPROM_TYPE_CODE_LC_MSK |
                                IPMI_SPROM_BMC_PRODUCT_NAME_SIZE;
	sprom_mem_fill((char*)pprd->product_name, 
			"N20-LBBU", 
			IPMI_SPROM_BMC_PRODUCT_NAME_SIZE);

        pprd->part_model_tl     = IPMI_SPROM_TYPE_CODE_LC_MSK |
                                IPMI_SPROM_BMC_PRODUCT_PART_MODEL_SIZE;
	sprom_mem_fill((char*)pprd->part_model, "73-12813-03", 
			IPMI_SPROM_BMC_PRODUCT_PART_MODEL_SIZE);

        pprd->prd_version_tl     = IPMI_SPROM_TYPE_CODE_LC_MSK |
                                IPMI_SPROM_BMC_PRODUCT_VERSION_SIZE;
	pprd->prd_version = 1;

        pprd->asset_tag_tl    = IPMI_SPROM_TYPE_CODE_LC_MSK|
				IPMI_SPROM_BMC_PRODUCT_ASSET_TAG_SIZE;
	// Data is already cleared.

        pprd->fru_file_id_tl  = IPMI_SPROM_TYPE_CODE_LC_MSK |
				IPMI_SPROM_BMC_PRODUCT_FRU_FILE_ID_SIZE;
	sprom_mem_fill((char*)pprd->fru_file_id, "N20-LBBU", 
			IPMI_SPROM_BMC_PRODUCT_FRU_FILE_ID_SIZE);

        pprd->custom_id_tl    = IPMI_SPROM_TYPE_CODE_LC_MSK |
                                IPMI_SPROM_BMC_PRODUCT_CUSTOM_ID_SIZE;
	pprd->bom_rev[0]      = '0';
	pprd->bom_rev[1]      = '2';
	pprd->pid_rev[0]      = 'V';
	pprd->pid_rev[1]      = '0';
	pprd->pid_rev[2]      = '1';
        ipmi_zero_checksum_create((uint8_t*)pprd, sizeof(*pprd));

	return (0);
}

int ipmi_sprom_iom_bp_dflt(sprom_ipmi_iom_bp_t *sprom)
{
	ipmi_sprom_chassis_info_t  *pchs = &sprom->chassis_info;
	ipmi_sprom_board_info_t	   *pbrd = (ipmi_sprom_board_info_t*)
					   &sprom->board_info;

        ipmi_sprom_common_header_create(&sprom->common_header, 0,
                M8_SIZE(sizeof(ipmi_sprom_chassis_info_t)),
                M8_SIZE(sizeof(ipmi_sprom_board_info_t)),
                0, 0);

	// Update Chassis Info.
        pchs->version       = IPMI_SPROM_CHASSIS_VERSION;
        pchs->length        = M8_SIZE(sizeof(ipmi_sprom_chassis_info_t));
        pchs->type          = IPMI_SPROM_CHASSIS_TYPE_RACK_MOUNT_CHASSIS;
        pchs->no_more_tl    = IPMI_SPROM_NO_MORE_TYPE_LENGTH;
        pchs->part_num_tl   = IPMI_SPROM_TYPE_CODE_LC_MSK |
			      IPMI_SPROM_CHASSIS_PART_NUMBER_SIZE;
	sprom_mem_fill((char*)pchs->part_num, (char*)"800-30305-02", 
			IPMI_SPROM_CHASSIS_PART_NUMBER_SIZE);
        pchs->mfg_info_tl   = IPMI_SPROM_TYPE_CODE_LC_MSK |
			      IPMI_SPROM_CHASSIS_MFG_INFO_SIZE;
	sprom_mem_fill((char*)pchs->mfg_info, (char*)"Cisco Systems Inc", 
			IPMI_SPROM_CHASSIS_MFG_INFO_SIZE);
        ipmi_zero_checksum_create((uint8_t*)pchs, sizeof(*pchs));

	// Update Board Info
        pbrd->version         = IPMI_SPROM_BOARD_VERSION;
        pbrd->length          = M8_SIZE(sizeof(ipmi_sprom_board_info_t));
        pbrd->language_code   = IPMI_LC_ENGLISH_0;
        pbrd->no_more_tl      = IPMI_SPROM_NO_MORE_TYPE_LENGTH;
        pbrd->mfg_info_tl     = IPMI_SPROM_TYPE_CODE_LC_MSK |
                                IPMI_SPROM_BOARD_MFG_INFO_SIZE;
        pbrd->serial_num_tl   = IPMI_SPROM_TYPE_CODE_LC_MSK |
                                IPMI_SPROM_BOARD_SERIAL_NUMBER_SIZE;

	sprom_mem_fill((char*)pbrd->mfg_info, "Cisco Systems Inc", 
			IPMI_SPROM_BOARD_MFG_INFO_SIZE);

        pbrd->product_name_tl = IPMI_SPROM_TYPE_CODE_LC_MSK |
                                IPMI_SPROM_BOARD_PRODUCT_NAME_SIZE;
	sprom_mem_fill((char*)pbrd->product_name, "N20-C6508", 
			IPMI_SPROM_BOARD_PRODUCT_NAME_SIZE);

        pbrd->part_num_tl = IPMI_SPROM_TYPE_CODE_LC_MSK |
                                IPMI_SPROM_BOARD_PART_NUMBER_SIZE;
	sprom_mem_fill((char*)pbrd->part_num, "73-11621-02", 
			IPMI_SPROM_BOARD_PART_NUMBER_SIZE);

        pbrd->fru_file_id_tl  = IPMI_SPROM_TYPE_CODE_LC_MSK |
				IPMI_SPROM_BOARD_FRU_FILE_ID_SIZE;
        pbrd->custom_id_tl    = IPMI_SPROM_TYPE_CODE_LC_MSK |
                                IPMI_SPROM_BOARD_CUSTOM_ID_SIZE;
	pbrd->bom_rev[0]      = '0';
	pbrd->bom_rev[1]      = '0';
	pbrd->hw_rev          = 0x00;
	pbrd->pid_rev[0]      = 'V';
	pbrd->pid_rev[1]      = '0';
	pbrd->pid_rev[2]      = '0';
        pbrd->clei_tl         = IPMI_SPROM_TYPE_CODE_LC_MSK |
                                IPMI_SPROM_BOARD_CLEI_SIZE;

        ipmi_zero_checksum_create((uint8_t*)pbrd, sizeof(*pbrd));

	return (0);
}

int ipmi_sprom_iom_dflt(sprom_ipmi_iom_t *sprom)
{
#if defined (DIAG_IOM2)
        ipmi_iom_internal_use_t *piu = &sprom->internal_use;
	ipmi_sprom_board_info_t *pbrd = (ipmi_sprom_board_info_t*)&sprom->board_info;

	ipmi_sprom_common_header_create(&sprom->common_header,
		M8_SIZE(sizeof(ipmi_iom_internal_use_t)),
		0,
		M8_SIZE(sizeof(ipmi_sprom_board_info_t)),
		0,
		0);

	// Update Internal Use Area
	piu->version	= IPMI_SPROM_COMMON_HEADER_VERSION;
	piu->length	= M8_SIZE(sizeof(*piu));
	piu->card_type	= htons(IPMI_IU_CARD_TYPE_IOM2);
	// Update Mac Fields.
	piu->mac.srec.sub_type		= htons(IPMI_IU_SUB_TYPE_MAC_ADDR);
	piu->mac.srec.sub_type_len	= htons(sizeof(ipmi_iu_rec_mac_t) -
					  sizeof(ipmi_iu_rec_t));
	piu->mac.cnt.type		= IPMI_IU_ENTRY_TYPE_CNT; 
	piu->mac.cnt.len		= 2;
	piu->mac.cnt.value		= htons(42);

	piu->mac.type 			= IPMI_IU_ENTRY_TYPE_MAC;
	piu->mac.len			= 6;
	memset(piu->mac.mac, 0, 6);

	// Update Temp1 SDR.
	iu_sdr_dflt_iom(&piu->brd_temp1, IPMI_IU_SUB_TYPE_IOM_TEMP_1); 
	// Update Temp2 SDR.
	iu_sdr_dflt_iom(&piu->brd_temp2, IPMI_IU_SUB_TYPE_IOM_TEMP_2);
	// Update RW_Temp1 SDR.
	iu_sdr_dflt_iom(&piu->rw_temp1, IPMI_IU_SUB_TYPE_RW_TEMP_1);
	// Update RW_Temp2 SDR.
	iu_sdr_dflt_iom(&piu->rw_temp2, IPMI_IU_SUB_TYPE_RW_TEMP_2);
	ipmi_zero_checksum_create((uint8_t*)piu, sizeof(*piu));


	// Update Board Info
	pbrd->version         = IPMI_SPROM_BOARD_VERSION;
	pbrd->length          = M8_SIZE(sizeof(ipmi_sprom_board_info_t));
	pbrd->language_code   = IPMI_LC_ENGLISH_0;
	pbrd->no_more_tl      = IPMI_SPROM_NO_MORE_TYPE_LENGTH;
	pbrd->serial_num_tl   = IPMI_SPROM_TYPE_CODE_LC_MSK |
				IPMI_SPROM_BOARD_SERIAL_NUMBER_SIZE;
	pbrd->mfg_info_tl     = IPMI_SPROM_TYPE_CODE_LC_MSK |
				IPMI_SPROM_BOARD_MFG_INFO_SIZE;
	sprom_mem_fill((char*)pbrd->mfg_info, "Cisco Systems Inc", 
			IPMI_SPROM_BOARD_MFG_INFO_SIZE);

	pbrd->product_name_tl = IPMI_SPROM_TYPE_CODE_LC_MSK |
				IPMI_SPROM_BOARD_PRODUCT_NAME_SIZE;
	sprom_mem_fill((char*)pbrd->product_name, "UCS-IOM-2208XP", 
			IPMI_SPROM_BOARD_PRODUCT_NAME_SIZE);

	pbrd->part_num_tl     = IPMI_SPROM_TYPE_CODE_LC_MSK |
				IPMI_SPROM_BOARD_PART_NUMBER_SIZE;
	sprom_mem_fill((char*)pbrd->part_num, "73-13196-04", 
			IPMI_SPROM_BOARD_PART_NUMBER_SIZE);

	pbrd->fru_file_id_tl  = IPMI_SPROM_TYPE_CODE_LC_MSK |
				IPMI_SPROM_BOARD_FRU_FILE_ID_SIZE;
	sprom_mem_fill((char*)pbrd->fru_file_id, "08XP", 
			    IPMI_SPROM_BOARD_PART_NUMBER_SIZE);

	pbrd->custom_id_tl    = IPMI_SPROM_TYPE_CODE_LC_MSK |
				IPMI_SPROM_BOARD_CUSTOM_ID_SIZE;
	pbrd->bom_rev[0]      = '0';
	pbrd->bom_rev[1]      = '4';
	pbrd->hw_rev          = 0x04;
	pbrd->pid_rev[0]      = 'V';
	pbrd->pid_rev[1]      = '0';
	pbrd->pid_rev[2]      = '0';
	pbrd->clei_tl         = IPMI_SPROM_TYPE_CODE_LC_MSK |
				IPMI_SPROM_BOARD_CLEI_SIZE;
	ipmi_zero_checksum_create((uint8_t*)pbrd, sizeof(*pbrd));

#else
	ipmi_iom_internal_use_t *piu = &sprom->internal_use;
	ipmi_sprom_board_info_t *pbrd = (ipmi_sprom_board_info_t*)&sprom->board_info;

        ipmi_sprom_common_header_create(&sprom->common_header,
                M8_SIZE(sizeof(ipmi_iom_internal_use_t)),
                0,
                M8_SIZE(sizeof(ipmi_sprom_board_info_t)),
                0,
                0);

	// Update Internal Use Area
        piu->version	= IPMI_SPROM_COMMON_HEADER_VERSION;
        piu->length	= M8_SIZE(sizeof(*piu));
        piu->card_type	= htons(IPMI_IU_CARD_TYPE_IOM);

	// Update Mac Fields.
	piu->mac.srec.sub_type		= htons(IPMI_IU_SUB_TYPE_MAC_ADDR);
	piu->mac.srec.sub_type_len	= htons(sizeof(ipmi_iu_rec_mac_t) -
					  sizeof(ipmi_iu_rec_t));
	piu->mac.cnt.type		= IPMI_IU_ENTRY_TYPE_CNT; 
	piu->mac.cnt.len		= 2;
	piu->mac.cnt.value		= htons(10);
	
	piu->mac.type 			= IPMI_IU_ENTRY_TYPE_MAC;
	piu->mac.len			= 6;
	memset(piu->mac.mac, 0, 6);

	// Update Temp1 SDR.
	iu_sdr_dflt_iom(&piu->brd_temp1, IPMI_IU_SUB_TYPE_IOM_TEMP_1); 
	// Update Temp2 SDR.
	iu_sdr_dflt_iom(&piu->brd_temp2, IPMI_IU_SUB_TYPE_IOM_TEMP_2);
	// Update RW_Temp1 SDR.
	iu_sdr_dflt_iom(&piu->rw_temp1, IPMI_IU_SUB_TYPE_RW_TEMP_1);
	// Update RW_Temp2 SDR.
	iu_sdr_dflt_iom(&piu->rw_temp2, IPMI_IU_SUB_TYPE_RW_TEMP_2);
        ipmi_zero_checksum_create((uint8_t*)piu, sizeof(*piu));


	// Update Board Info
        pbrd->version         = IPMI_SPROM_BOARD_VERSION;
        pbrd->length          = M8_SIZE(sizeof(ipmi_sprom_board_info_t));
        pbrd->language_code   = IPMI_LC_ENGLISH_0;
        pbrd->no_more_tl      = IPMI_SPROM_NO_MORE_TYPE_LENGTH;
        pbrd->serial_num_tl   = IPMI_SPROM_TYPE_CODE_LC_MSK |
                                IPMI_SPROM_BOARD_SERIAL_NUMBER_SIZE;
        pbrd->mfg_info_tl     = IPMI_SPROM_TYPE_CODE_LC_MSK |
                                IPMI_SPROM_BOARD_MFG_INFO_SIZE;
	sprom_mem_fill((char*)pbrd->mfg_info, "Cisco Systems Inc", 
			IPMI_SPROM_BOARD_MFG_INFO_SIZE);

        pbrd->product_name_tl = IPMI_SPROM_TYPE_CODE_LC_MSK |
                                IPMI_SPROM_BOARD_PRODUCT_NAME_SIZE;
	sprom_mem_fill((char*)pbrd->product_name, "N20-I6584", 
			IPMI_SPROM_BOARD_PRODUCT_NAME_SIZE);

        pbrd->part_num_tl     = IPMI_SPROM_TYPE_CODE_LC_MSK |
                                IPMI_SPROM_BOARD_PART_NUMBER_SIZE;
	sprom_mem_fill((char*)pbrd->part_num, "73-11623-03", 
			IPMI_SPROM_BOARD_PART_NUMBER_SIZE);

        pbrd->fru_file_id_tl  = IPMI_SPROM_TYPE_CODE_LC_MSK |
				IPMI_SPROM_BOARD_FRU_FILE_ID_SIZE;
        pbrd->custom_id_tl    = IPMI_SPROM_TYPE_CODE_LC_MSK |
                                IPMI_SPROM_BOARD_CUSTOM_ID_SIZE;
	pbrd->bom_rev[0]      = '0';
	pbrd->bom_rev[1]      = 'A';
	pbrd->hw_rev          = 0x03;
	pbrd->pid_rev[0]      = 'V';
	pbrd->pid_rev[1]      = '0';
	pbrd->pid_rev[2]      = '0';
        pbrd->clei_tl         = IPMI_SPROM_TYPE_CODE_LC_MSK |
                                IPMI_SPROM_BOARD_CLEI_SIZE;
        ipmi_zero_checksum_create((uint8_t*)pbrd, sizeof(*pbrd));
#endif
	return (0);
}

int ipmi_sprom_iom_psu_dflt(sprom_ipmi_iom_psu_t *sprom)
{
	ipmi_sprom_product_info_t *pbrd = &sprom->product_info;
	ipmi_psu_internal_use_t *piu = &sprom->internal_use;

        ipmi_sprom_common_header_create(&sprom->common_header,
        M8_SIZE(sizeof(ipmi_psu_internal_use_t)),
        0, 0, M8_SIZE(sizeof(ipmi_sprom_product_info_t)),  0);

	// Update Product Info
        pbrd->version         = IPMI_SPROM_BOARD_VERSION;
        pbrd->length          = M8_SIZE(sizeof(ipmi_sprom_product_info_t));
        pbrd->language_code   = IPMI_LC_ENGLISH_0;
        pbrd->no_more_tl      = IPMI_SPROM_NO_MORE_TYPE_LENGTH;
        pbrd->asset_tag_tl    = IPMI_SPROM_TYPE_CODE_LC_MSK;
        pbrd->serial_num_tl   = IPMI_SPROM_TYPE_CODE_LC_MSK |
                                IPMI_SPROM_PRODUCT_SERIAL_NUMBER_SIZE;
        pbrd->mfg_name_tl     = IPMI_SPROM_TYPE_CODE_LC_MSK |
                                IPMI_SPROM_PRODUCT_MFG_NAME_SIZE;
	sprom_mem_fill((char*)pbrd->mfg_name, "Cisco Systems Inc", 
			IPMI_SPROM_PRODUCT_MFG_NAME_SIZE);

        pbrd->product_name_tl = IPMI_SPROM_TYPE_CODE_LC_MSK |
                                IPMI_SPROM_PRODUCT_NAME_SIZE;
	sprom_mem_fill((char*)pbrd->product_name, "N20-PAC5-2500W", 
			IPMI_SPROM_PRODUCT_NAME_SIZE);

        pbrd->part_model_tl   = IPMI_SPROM_TYPE_CODE_LC_MSK |
                                IPMI_SPROM_PRODUCT_PART_MODEL_SIZE;
	sprom_mem_fill((char*)pbrd->part_model, "341-0293-04", 
			IPMI_SPROM_PRODUCT_PART_MODEL_SIZE);

        pbrd->prd_version_tl  = IPMI_SPROM_TYPE_CODE_LC_MSK |
                                IPMI_SPROM_PRODUCT_VERSION_SIZE;
	sprom_mem_fill((char*)pbrd->prd_version, "P0", 
			IPMI_SPROM_PRODUCT_VERSION_SIZE);
        pbrd->fru_file_id_tl  = IPMI_SPROM_TYPE_CODE_LC_MSK;
        pbrd->custom_id_tl    = IPMI_SPROM_TYPE_CODE_LC_MSK |
                                IPMI_SPROM_PRODUCT_CUSTOM_ID_SIZE;
	pbrd->bom_rev         = 0x00;
	pbrd->hw_rev          = 0x00;
	pbrd->pid_rev[0]      = 'V';
	pbrd->pid_rev[1]      = '0';
	pbrd->pid_rev[2]      = '0';
        ipmi_zero_checksum_create((uint8_t*)pbrd, sizeof(*pbrd));


	// Update Internal Use Area
        piu->version	= IPMI_SPROM_COMMON_HEADER_VERSION;
        piu->length	= M8_SIZE(sizeof(*piu));
        piu->card_type	= htons(IPMI_IU_CARD_TYPE_SC_PSU);

	// Update FAN RPM/TEMP/VOLT SDR.
	iu_sdr_dflt_iom(&piu->psu_temp, IPMI_IU_SUB_TYPE_PSU_TEMP);
	iu_sdr_dflt_iom(&piu->psu_in_volt, IPMI_IU_SUB_TYPE_PSU_IN_VOLT);
	iu_sdr_dflt_iom(&piu->psu_out_volt1, IPMI_IU_SUB_TYPE_PSU_OUT_VOLT1);
	iu_sdr_dflt_iom(&piu->psu_out_volt2, IPMI_IU_SUB_TYPE_PSU_OUT_VOLT2);
	iu_sdr_dflt_iom(&piu->psu_in_current, IPMI_IU_SUB_TYPE_PSU_IN_CURRENT);
	iu_sdr_dflt_iom(&piu->psu_out_current, IPMI_IU_SUB_TYPE_PSU_OUT_CURRENT);
	iu_sdr_dflt_iom(&piu->psu_rpm, IPMI_IU_SUB_TYPE_PSU_RPM); 
        ipmi_zero_checksum_create((uint8_t*)piu, sizeof(*piu));

	return (0);
}

int ipmi_sprom_iom_psu_p0_dflt(sprom_ipmi_iom_psu_p0_t *sprom)
{
	ipmi_sprom_product_info_t *pbrd = &sprom->product_info;

        ipmi_sprom_common_header_create(&sprom->common_header,
        0, 0, 0, M8_SIZE(sizeof(ipmi_sprom_product_info_t)),  0);

	// Update Product Info
        pbrd->version         = IPMI_SPROM_BOARD_VERSION;
        pbrd->length          = M8_SIZE(sizeof(ipmi_sprom_product_info_t));
        pbrd->language_code   = IPMI_LC_ENGLISH_0;
        pbrd->no_more_tl      = IPMI_SPROM_NO_MORE_TYPE_LENGTH;
        pbrd->asset_tag_tl    = IPMI_SPROM_TYPE_CODE_LC_MSK;
        pbrd->serial_num_tl   = IPMI_SPROM_TYPE_CODE_LC_MSK |
                                IPMI_SPROM_PRODUCT_SERIAL_NUMBER_SIZE;
        pbrd->mfg_name_tl     = IPMI_SPROM_TYPE_CODE_LC_MSK |
                                IPMI_SPROM_PRODUCT_MFG_NAME_SIZE;
	sprom_mem_fill((char*)pbrd->mfg_name, "Cisco Systems Inc", 
			IPMI_SPROM_PRODUCT_MFG_NAME_SIZE);

        pbrd->product_name_tl = IPMI_SPROM_TYPE_CODE_LC_MSK |
                                IPMI_SPROM_PRODUCT_NAME_SIZE;
	sprom_mem_fill((char*)pbrd->product_name, "N20-PAC5-2500W", 
			IPMI_SPROM_PRODUCT_NAME_SIZE);

        pbrd->part_model_tl   = IPMI_SPROM_TYPE_CODE_LC_MSK |
                                IPMI_SPROM_PRODUCT_PART_MODEL_SIZE;
	sprom_mem_fill((char*)pbrd->part_model, "341-0293-04", 
			IPMI_SPROM_PRODUCT_PART_MODEL_SIZE);

        pbrd->prd_version_tl  = IPMI_SPROM_TYPE_CODE_LC_MSK |
                                IPMI_SPROM_PRODUCT_VERSION_SIZE;
	sprom_mem_fill((char*)pbrd->prd_version, "P0", 
			IPMI_SPROM_PRODUCT_VERSION_SIZE);
        pbrd->fru_file_id_tl  = IPMI_SPROM_TYPE_CODE_LC_MSK;
        pbrd->custom_id_tl    = IPMI_SPROM_TYPE_CODE_LC_MSK |
                                IPMI_SPROM_PRODUCT_CUSTOM_ID_SIZE;
	pbrd->bom_rev         = 0x00;
	pbrd->hw_rev          = 0x00;
	pbrd->pid_rev[0]      = 'V';
	pbrd->pid_rev[1]      = '0';
	pbrd->pid_rev[2]      = '0';
        ipmi_zero_checksum_create((uint8_t*)pbrd, sizeof(*pbrd));

	return (0);
}

int ipmi_sprom_iom_fan_dflt(sprom_ipmi_iom_fan_t *sprom)
{
	ipmi_fan_internal_use_t *piu = &sprom->internal_use;
	ipmi_sprom_board_info_t *pbrd = (ipmi_sprom_board_info_t*)&sprom->board_info;

        ipmi_sprom_common_header_create(&sprom->common_header,
                M8_SIZE(sizeof(ipmi_iom_internal_use_t)),
                0,
                M8_SIZE(sizeof(ipmi_sprom_board_info_t)),
                0,
                0);

	// Update Internal Use Area
        piu->version	= IPMI_SPROM_COMMON_HEADER_VERSION;
        piu->length	= M8_SIZE(sizeof(*piu));
        piu->card_type	= htons(IPMI_IU_CARD_TYPE_SC_FAN);

	// Update FAN RPM/TEMP/VOLT SDR.
	iu_sdr_dflt_iom(&piu->fan_rpm, IPMI_IU_SUB_TYPE_FAN_RPM); 
	iu_sdr_dflt_iom(&piu->fan_temp, IPMI_IU_SUB_TYPE_FAN_TEMP);
	iu_sdr_dflt_iom(&piu->fan_volt, IPMI_IU_SUB_TYPE_FAN_VOLTAGE);
        ipmi_zero_checksum_create((uint8_t*)piu, sizeof(*piu));


	// Update Board Info
        pbrd->version         = IPMI_SPROM_BOARD_VERSION;
        pbrd->length          = M8_SIZE(sizeof(ipmi_sprom_board_info_t));
        pbrd->language_code   = IPMI_LC_ENGLISH_0;
        pbrd->no_more_tl      = IPMI_SPROM_NO_MORE_TYPE_LENGTH;
        pbrd->serial_num_tl   = IPMI_SPROM_TYPE_CODE_LC_MSK |
                                IPMI_SPROM_BOARD_SERIAL_NUMBER_SIZE;
        pbrd->mfg_info_tl     = IPMI_SPROM_TYPE_CODE_LC_MSK |
                                IPMI_SPROM_BOARD_MFG_INFO_SIZE;
	sprom_mem_fill((char*)pbrd->mfg_info, "Cisco Systems Inc", 
			IPMI_SPROM_BOARD_MFG_INFO_SIZE);

        pbrd->product_name_tl = IPMI_SPROM_TYPE_CODE_LC_MSK |
                                IPMI_SPROM_BOARD_PRODUCT_NAME_SIZE;
	sprom_mem_fill((char*)pbrd->product_name, "N20-FAN5", 
			IPMI_SPROM_BOARD_PRODUCT_NAME_SIZE);

        pbrd->part_num_tl     = IPMI_SPROM_TYPE_CODE_LC_MSK |
                                IPMI_SPROM_BOARD_PART_NUMBER_SIZE;
	sprom_mem_fill((char*)pbrd->part_num, "73-11624-05", 
			IPMI_SPROM_BOARD_PART_NUMBER_SIZE);

        pbrd->fru_file_id_tl  = IPMI_SPROM_TYPE_CODE_LC_MSK |
				IPMI_SPROM_BOARD_FRU_FILE_ID_SIZE;
        pbrd->custom_id_tl    = IPMI_SPROM_TYPE_CODE_LC_MSK |
                                IPMI_SPROM_BOARD_CUSTOM_ID_SIZE;
	pbrd->bom_rev[0]      = '0';
	pbrd->bom_rev[1]      = '2';
	pbrd->hw_rev          = 0x05;
	pbrd->pid_rev[0]      = 'V';
	pbrd->pid_rev[1]      = '0';
	pbrd->pid_rev[2]      = '0';
        pbrd->clei_tl         = IPMI_SPROM_TYPE_CODE_LC_MSK |
                                IPMI_SPROM_BOARD_CLEI_SIZE;
        ipmi_zero_checksum_create((uint8_t*)pbrd, sizeof(*pbrd));

	return (0);
}

int ipmi_sprom_sd_psu_dflt(sprom_ipmi_sd_psu_t *sprom)
{
	ipmi_sprom_psu_internal_use_t *piu  = &sprom->internal_use;
	ipmi_sprom_psu_product_info_t *pprd = &sprom->product_info;
	ipmi_sprom_psu_mr_t           *pmr  = &sprom->multi_record;

        ipmi_sprom_common_header_create(&sprom->common_header,
        M8_SIZE(sizeof(*piu)), 0, 0, M8_SIZE(sizeof(*pprd)), M8_SIZE(sizeof(*pmr)));

	// Update Internal Use Area
        piu->version	= IPMI_SPROM_COMMON_HEADER_VERSION;
        piu->length	= M8_SIZE(sizeof(*piu));
        piu->card_type	= htons(IPMI_IU_CARD_TYPE_SC_PSU);

	// default values are same as iom - so re-use the function.
	iu_sdr_dflt_iom(&piu->psu_temp, IPMI_IU_SUB_TYPE_PSU_TEMP);
	iu_sdr_dflt_iom(&piu->psu_in_volt, IPMI_IU_SUB_TYPE_PSU_IN_VOLT);
	iu_sdr_dflt_iom(&piu->psu_out_volt1, IPMI_IU_SUB_TYPE_PSU_OUT_VOLT1);
	iu_sdr_dflt_iom(&piu->psu_out_volt2, IPMI_IU_SUB_TYPE_PSU_OUT_VOLT2);
	iu_sdr_dflt_iom(&piu->psu_in_current, IPMI_IU_SUB_TYPE_PSU_IN_CURRENT);
	iu_sdr_dflt_iom(&piu->psu_out_current, IPMI_IU_SUB_TYPE_PSU_OUT_CURRENT);
	iu_sdr_dflt_iom(&piu->psu_rpm, IPMI_IU_SUB_TYPE_PSU_RPM); 


	// Update Product Info
        pprd->version         = IPMI_SPROM_BOARD_VERSION;
        pprd->length          = M8_SIZE(sizeof(*pprd));
        pprd->language_code   = IPMI_LC_ENGLISH_0;
        pprd->no_more_tl      = IPMI_SPROM_NO_MORE_TYPE_LENGTH;

        pprd->mfg_name_tl     = IPMI_SPROM_TYPE_CODE_LC_MSK |
                                IPMI_SPROM_SD_PRODUCT_MFG_NAME_SIZE;
	sprom_mem_fill((char*)pprd->mfg_name, "Cisco Systems Inc", 
			IPMI_SPROM_SD_PRODUCT_MFG_NAME_SIZE);

        pprd->product_name_tl = IPMI_SPROM_TYPE_CODE_LC_MSK |
                                IPMI_SPROM_SD_PRODUCT_NAME_SIZE;
	sprom_mem_fill((char*)pprd->product_name, "R2X0-PSU2-650W-SB", 
			IPMI_SPROM_SD_PRODUCT_NAME_SIZE);

        pprd->part_model_tl   = IPMI_SPROM_TYPE_CODE_LC_MSK |
                                IPMI_SPROM_SD_PRODUCT_PART_MODEL_SIZE;
	sprom_mem_fill((char*)pprd->part_model, "74-7541-01", 
			IPMI_SPROM_SD_PRODUCT_PART_MODEL_SIZE);

        pprd->prd_version_tl  = IPMI_SPROM_TYPE_CODE_LC_MSK |
                                IPMI_SPROM_SD_PRODUCT_VERSION_SIZE;
	sprom_mem_fill((char*)pprd->prd_version, "A0", 
			IPMI_SPROM_SD_PRODUCT_VERSION_SIZE);

        pprd->serial_num_tl   = IPMI_SPROM_TYPE_CODE_LC_MSK |
                                IPMI_SPROM_SD_PRODUCT_SERIAL_NUMBER_SIZE;

        pprd->fru_file_id_tl  = IPMI_SPROM_TYPE_CODE_LC_MSK;
        pprd->asset_tag_tl    = IPMI_SPROM_TYPE_CODE_LC_MSK;

        pprd->custom_id_tl    = IPMI_SPROM_TYPE_CODE_LC_MSK |
                                IPMI_SPROM_SD_PRODUCT_CUSTOM_ID_SIZE;
	pprd->pid_rev[0]      = 'V';
	pprd->pid_rev[1]      = '0';
	pprd->pid_rev[2]      = '1';
	pprd->bom_rev[0]      = 0x00;
	pprd->bom_rev[1]      = 0x00;

        pprd->standby_amp_tl  = IPMI_SPROM_TYPE_CODE_LC_MSK |2;
	pprd->amp[0]          = 0x0A;
	pprd->amp[1]          = 0xBE;
        ipmi_zero_checksum_create((uint8_t*)pprd, sizeof(*pprd));

	// Update Multi Record Area
        pmr->psu_info_hdr.record_type     = IPMI_SPROM_TYPE_CODE_LC_MSK |
					    IPMI_SPROM_MULTI_RECORD_POWER_SUPPLY;
        pmr->psu_info_hdr.version         = 0x82;
        pmr->psu_info_hdr.length          = 0x0B;	// 11 bytes 
        pmr->psu_info_hdr.record_checksum = 0x00; 
        pmr->psu_info_hdr.header_checksum = 0x00; 
	
	pmr->oem[0] = 'C';
	pmr->oem[1] = 'i';
	pmr->oem[2] = 's';
	pmr->oem[3] = 'c';
	pmr->oem[4] = 'o';

	pmr->version[0] = 0;
	pmr->version[1] = 0;
	pmr->version[2] = 0;
	pmr->version[3] = 1;

	pmr->power[0]   = 0x0A;
	pmr->power[1]   = 0xBE;

	return (0);
}
