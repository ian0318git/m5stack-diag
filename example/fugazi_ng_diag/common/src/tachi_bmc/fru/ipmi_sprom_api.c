/* $Id: ipmi_sprom_api.c,v 1.2 2016/04/20 08:41:35 benchen2 Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/tachi_bmc/fru/ipmi_sprom_api.c,v $
 * Copyright (c) 2016 by Cisco Systems, Inc.
 * All rights reserved.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <errno.h>
#include <arpa/inet.h>

#include "ipmi_sprom.h"
#include "ipmi_sprom_ops.h"

#define EOF_CHECK(eof, pdata)		\
	eof =  ((*(uint8_t*)pdata) == (uint8_t)IPMI_SPROM_NO_MORE_TYPE_LENGTH) ? 1 : eof;

#define SDR_NTOHS(sbuf, stype)					\
{								\
	sbuf->stype.srec.sub_type =				\
		ntohs(sbuf->stype.srec.sub_type);		\
	sbuf->stype.srec.sub_type_len =					\
		ntohs(sbuf->stype.srec.sub_type_len);		\
								\
	sbuf->stype.multi_factor.value =			\
		ntohs(sbuf->stype.multi_factor.value);		\
	sbuf->stype.base_offset.value =					\
		ntohs(sbuf->stype.base_offset.value);		\
	sbuf->stype.k1.value =					\
		ntohs(sbuf->stype.k1.value);			\
	sbuf->stype.k2.value =					\
		ntohs(sbuf->stype.k2.value);			\
	sbuf->stype.nominal_reading.value =			\
		ntohs(sbuf->stype.nominal_reading.value);	\
	sbuf->stype.normal_max.value =				\
		ntohs(sbuf->stype.normal_max.value);		\
	sbuf->stype.normal_min.value =				\
		ntohs(sbuf->stype.normal_min.value);		\
	sbuf->stype.sensor_max.value =				\
		ntohs(sbuf->stype.sensor_max.value);		\
	sbuf->stype.sensor_min.value =				\
		ntohs(sbuf->stype.sensor_min.value);		\
	sbuf->stype.upper_nr_thres.value =			\
		ntohs(sbuf->stype.upper_nr_thres.value);		\
	sbuf->stype.upper_crit_thres.value =			\
		ntohs(sbuf->stype.upper_crit_thres.value);	\
	sbuf->stype.upper_non_crit_thres.value =		\
		ntohs(sbuf->stype.upper_non_crit_thres.value);	\
	sbuf->stype.lower_nr_thres.value =			\
		ntohs(sbuf->stype.lower_nr_thres.value);		\
	sbuf->stype.lower_crit_thres.value =			\
		ntohs(sbuf->stype.lower_crit_thres.value);	\
	sbuf->stype.lower_non_crit_thres.value =		\
		ntohs(sbuf->stype.lower_non_crit_thres.value);	\
	sbuf->stype.pos_hyst.value =				\
		ntohs(sbuf->stype.pos_hyst.value);		\
	sbuf->stype.neg_hyst.value =				\
		ntohs(sbuf->stype.neg_hyst.value);		\
}

int ipmi_region_checksum_verify(char *pbuf)
{
	ipmi_sprom_common_header_t *phdr = (ipmi_sprom_common_header_t*)pbuf;
	char *pstr = NULL;
	int   rc = 0;

	if (phdr->internal_use) {
		pstr = pbuf + (phdr->internal_use << 3);
		rc = ipmi_zero_checksum_verify((uint8_t*)pstr, (*(pstr+1) << 3));
		if (rc) return (rc);
	}	
	if (phdr->chassis_info) {
		pstr = pbuf + (phdr->chassis_info << 3);
		rc = ipmi_zero_checksum_verify((uint8_t*)pstr, (*(pstr+1) << 3));
		if (rc) return (rc);
	}	
	if (phdr->board_info) {
		pstr = pbuf + (phdr->board_info << 3);
		rc = ipmi_zero_checksum_verify((uint8_t*)pstr, (*(pstr+1) << 3));
		if (rc) return (rc);
	}	
	if (phdr->product_info) {
		pstr = pbuf + (phdr->product_info << 3);
		rc = ipmi_zero_checksum_verify((uint8_t*)pstr, (*(pstr+1) << 3));
		if (rc) return (rc);
	}	

	// ADD MULTI RECORD LATER.
	return (rc);
}

int ipmi_buf_size_verify(char *pbuf, int buf_len)
{
	ipmi_sprom_common_header_t *phdr = (ipmi_sprom_common_header_t*)pbuf;
	int offset, length;

	if (!phdr) return (-1);

	offset = (phdr->chassis_info > phdr->internal_use) ? 
			phdr->chassis_info : phdr->internal_use;
	offset = (phdr->board_info > offset) ? phdr->board_info     : offset;
	offset = (phdr->product_info > offset) ? phdr->product_info : offset;

	// ADD MULTI RECORD LATER
	// offset = (phdr->multi_record > offset) ? phdr->multi_record : offset;

	length = (offset << 3);
	length += ((*(pbuf + length + 1)) << 3);
	return ((length <= buf_len) ? 0 : -1);
}

int ipmi_sprom_data_sanity_check (char *pbuf, int buf_len)
{
	int rc = 0;

	// Verify the header is correct.
	rc = ipmi_header_checksum_verify((uint8_t*)pbuf);
	if (rc) {
		printf("  Header Check sum failed\n");
		return (rc);
	}

	// Now verify the size of the buffer is sufficient.
	rc = ipmi_buf_size_verify(pbuf, buf_len);
	if (rc) {
		printf(" Buf Size verify failed\n");
		return (rc);
	}

	// Now verify the checksum of all regions.
	rc = ipmi_region_checksum_verify (pbuf);
	if (rc) {
		printf("  Region Check sum failed\n");
		return (rc);
	}

	return (rc);
}

void ipmi_sprom_string_cpy(char *pbuf, int buf_len, char *pstr, int str_len)
{
	int len = (buf_len > str_len) ? str_len : buf_len;
	memset(pstr, ' ', str_len);
	memcpy(pstr, pbuf, len);
}

int ipmi_sprom_common_header_hfmt(char *pbuf, ipmi_sprom_common_header_t *phdr)
{
	if (!phdr || !pbuf) return (-1);

	if (0 != ipmi_header_checksum_verify((uint8_t *)pbuf))
		return (-1);

	memcpy(phdr, pbuf, sizeof(*phdr));

	return (0);
}


int ipmi_sprom_product_info_hfmt(char *pbuf, ipmi_sprom_product_info_t* pprd)
{
	int rc = 0;
	char *pdata = NULL, eof = 0;

	if (!pprd || !pbuf) return (-1);

	if (!(((ipmi_sprom_common_header_t *)pbuf)->product_info)) {
		return (-1);
	}

	memset(pprd, 0, sizeof(*pprd));

	// Get the pointer to the product info area
	pdata = pbuf + (((ipmi_sprom_common_header_t*)pbuf)->product_info << 3);

	// Now convert the data.
	pprd->version	      =  pdata[0]; 
	pprd->length	      =  pdata[1]; 
	pprd->language_code   =  pdata[2]; 
	pdata += 3;

	EOF_CHECK(eof, pdata);
	pprd->mfg_name_tl     =  IPMI_SPROM_TYPE_CODE_LC_MSK |
				 IPMI_SPROM_PRODUCT_MFG_NAME_SIZE; 
	ipmi_sprom_string_cpy(pdata+1, eof ? 0 : (*pdata & 0x3F),
		(char*)&pprd->mfg_name, IPMI_SPROM_PRODUCT_MFG_NAME_SIZE);
	pdata += ((*pdata & 0x3F) + 1);

	EOF_CHECK(eof, pdata);
	pprd->product_name_tl = IPMI_SPROM_TYPE_CODE_LC_MSK |
				IPMI_SPROM_PRODUCT_NAME_SIZE; 
	ipmi_sprom_string_cpy(pdata+1, eof ? 0 : (*pdata & 0x3F),
		(char*)&pprd->product_name, IPMI_SPROM_PRODUCT_NAME_SIZE);
	pdata += ((*pdata & 0x3F) + 1);

	EOF_CHECK(eof, pdata);
	pprd->part_model_tl = IPMI_SPROM_TYPE_CODE_LC_MSK |
			    IPMI_SPROM_PRODUCT_PART_MODEL_SIZE; 
	ipmi_sprom_string_cpy(pdata+1, eof ? 0 : (*pdata & 0x3F),
		(char*)&pprd->part_model, IPMI_SPROM_PRODUCT_PART_MODEL_SIZE);
	pdata += ((*pdata & 0x3F) + 1);

	EOF_CHECK(eof, pdata);
	pprd->prd_version_tl = IPMI_SPROM_TYPE_CODE_LC_MSK |
			    IPMI_SPROM_PRODUCT_PART_MODEL_SIZE; 
	ipmi_sprom_string_cpy(pdata+1, eof ? 0 : (*pdata & 0x3F),
		(char*)&pprd->prd_version, IPMI_SPROM_PRODUCT_PART_MODEL_SIZE);
	pdata += ((*pdata & 0x3F) + 1);

	EOF_CHECK(eof, pdata);
	pprd->serial_num_tl =	IPMI_SPROM_TYPE_CODE_LC_MSK |
				IPMI_SPROM_PRODUCT_SERIAL_NUMBER_SIZE; 
	ipmi_sprom_string_cpy(pdata+1, eof ? 0 : (*pdata & 0x3F),
		(char*)&pprd->serial_num, IPMI_SPROM_PRODUCT_SERIAL_NUMBER_SIZE);
	pdata += ((*pdata & 0x3F) + 1);

	EOF_CHECK(eof, pdata);
	// copy thee next 8 bytes.
	memcpy(&pprd->asset_tag_tl, pdata, 8);
	pdata+=8;

	pprd->no_more_tl      = IPMI_SPROM_NO_MORE_TYPE_LENGTH; 
	ipmi_zero_checksum_create((uint8_t*)pprd, sizeof(*pprd));
	return (rc);
}


int ipmi_sprom_bmc_product_info_hfmt(char *pbuf, ipmi_sprom_bmc_product_info_t* pprd)
{
	int rc = 0;
	char *pdata = NULL, eof = 0;

	if (!pprd || !pbuf) return (-1);

	memset(pprd, 0, sizeof(*pprd));

	// Get the pointer to the product info area
	pdata = pbuf + (((ipmi_sprom_common_header_t*)pbuf)->product_info << 3);

	// Now convert the data.
	pprd->version	      =  pdata[0]; 
	pprd->length	      =  pdata[1]; 
	pprd->language_code   =  pdata[2]; 

	EOF_CHECK(eof, pdata);
	pprd->mfg_name_tl     =  IPMI_SPROM_TYPE_CODE_LC_MSK |
				 IPMI_SPROM_BMC_PRODUCT_MFG_NAME_SIZE; 
	ipmi_sprom_string_cpy(pdata+1, eof ? 0 : (*pdata & 0x3F),
		(char*)&pprd->mfg_name, IPMI_SPROM_BMC_PRODUCT_MFG_NAME_SIZE);
	pdata += ((*pdata & 0x3F) + 1);

	EOF_CHECK(eof, pdata);
	pprd->product_name_tl = IPMI_SPROM_TYPE_CODE_LC_MSK |
				IPMI_SPROM_BMC_PRODUCT_NAME_SIZE; 
	ipmi_sprom_string_cpy(pdata+1, eof ? 0 : (*pdata & 0x3F),
		(char*)&pprd->product_name, IPMI_SPROM_BMC_PRODUCT_NAME_SIZE);
	pdata += ((*pdata & 0x3F) + 1);

	EOF_CHECK(eof, pdata);
	pprd->part_model_tl = IPMI_SPROM_TYPE_CODE_LC_MSK |
			    IPMI_SPROM_BMC_PRODUCT_PART_MODEL_SIZE; 
	ipmi_sprom_string_cpy(pdata+1, eof ? 0 : (*pdata & 0x3F),
		(char*)&pprd->part_model, IPMI_SPROM_BMC_PRODUCT_PART_MODEL_SIZE);
	pdata += ((*pdata & 0x3F) + 1);

	EOF_CHECK(eof, pdata);
	pprd->prd_version_tl = IPMI_SPROM_TYPE_CODE_LC_MSK |
			    IPMI_SPROM_BMC_PRODUCT_PART_MODEL_SIZE; 
	ipmi_sprom_string_cpy(pdata+1, eof ? 0 : (*pdata & 0x3F),
		(char*)&pprd->prd_version, IPMI_SPROM_BMC_PRODUCT_PART_MODEL_SIZE);
	pdata += ((*pdata & 0x3F) + 1);

	EOF_CHECK(eof, pdata);
	pprd->serial_num_tl =	IPMI_SPROM_TYPE_CODE_LC_MSK |
				IPMI_SPROM_BMC_PRODUCT_SERIAL_NUMBER_SIZE; 
	ipmi_sprom_string_cpy(pdata+1, eof ? 0 : (*pdata & 0x3F),
		(char*)&pprd->serial_num, IPMI_SPROM_BMC_PRODUCT_SERIAL_NUMBER_SIZE);
	pdata += ((*pdata & 0x3F) + 1);

	EOF_CHECK(eof, pdata);
	pprd->asset_tag_tl =	IPMI_SPROM_TYPE_CODE_LC_MSK |
				IPMI_SPROM_BMC_PRODUCT_ASSET_TAG_SIZE; 
	ipmi_sprom_string_cpy(pdata+1, eof ? 0 : (*pdata & 0x3F),
		(char*)&pprd->asset_tag, IPMI_SPROM_BMC_PRODUCT_ASSET_TAG_SIZE);
	pdata += ((*pdata & 0x3F) + 1);

	EOF_CHECK(eof, pdata);
	pprd->fru_file_id_tl =	IPMI_SPROM_TYPE_CODE_LC_MSK |
				IPMI_SPROM_BMC_PRODUCT_FRU_FILE_ID_SIZE; 
	ipmi_sprom_string_cpy(pdata+1, eof ? 0 : (*pdata & 0x3F),
		(char*)&pprd->fru_file_id, IPMI_SPROM_BMC_PRODUCT_FRU_FILE_ID_SIZE);
	pdata += ((*pdata & 0x3F) + 1);


	EOF_CHECK(eof, pdata);

	// copy thee next 6 bytes.
	memcpy(&pprd->custom_id_tl, pdata, 6);
	pdata+=6;
	EOF_CHECK(eof, pdata);
	pprd->pad_tl =	IPMI_SPROM_TYPE_CODE_LC_MSK |
			IPMI_SPROM_BMC_PRODUCT_NIM_PAD_SIZE;
	ipmi_sprom_string_cpy(pdata+1, eof ? 0 : (*pdata & 0x3F),
			(char*)&pprd->pad, IPMI_SPROM_BMC_PRODUCT_NIM_PAD_SIZE);
	pdata += ((*pdata & 0x3F) + 1);

	pprd->no_more_tl      = IPMI_SPROM_NO_MORE_TYPE_LENGTH; 
	ipmi_zero_checksum_create((uint8_t*)pprd, sizeof(*pprd));
	return (rc);
}

int ipmi_sprom_board_info_hfmt(char *pbuf, ipmi_sprom_board_info_t* pbrd)
{
	int rc = 0;
	char *pdata = NULL, eof = 0;
	uint8_t unib, lnib;

	if (!pbrd || !pbuf) return (-1);

	memset(pbrd, 0, sizeof(*pbrd));
	// Get the pointer to the board info area
	pdata = pbuf + (((ipmi_sprom_common_header_t*)pbuf)->board_info << 3);

	// Now convert the data.
	// Copy first 6 bytes as is.
	// pbrd->signature	 = (pdata[0] >> 4);
	// pbrd->version	 = (pdata[0] & 0x0F); 
	// pbrd->length		 =  pdata[1]; 
	// pbrd->language_code	 =  pdata[2]; 
	// pbrd->mfg_date_time[0]=  pdata[3]; 
	// pbrd->mfg_date_time[1]=  pdata[4]; 
	// pbrd->mfg_date_time[2]=  pdata[5]; 

	memcpy((char*)pbrd, pdata, 6);
	pdata+= 6;

	EOF_CHECK(eof, pdata);
	pbrd->mfg_info_tl     =  IPMI_SPROM_TYPE_CODE_LC_MSK |
				 IPMI_SPROM_BOARD_MFG_INFO_SIZE; 
	ipmi_sprom_string_cpy(pdata+1, eof ? 0 : (*pdata & 0x3F),
		(char*)&pbrd->mfg_info, IPMI_SPROM_BOARD_MFG_INFO_SIZE);
	pdata += ((*pdata & 0x3F) + 1);

	EOF_CHECK(eof, pdata);
	pbrd->product_name_tl = IPMI_SPROM_TYPE_CODE_LC_MSK |
				IPMI_SPROM_BOARD_PRODUCT_NAME_SIZE; 
	ipmi_sprom_string_cpy(pdata+1, eof ? 0 : (*pdata & 0x3F),
		(char*)&pbrd->product_name, IPMI_SPROM_BOARD_PRODUCT_NAME_SIZE);
	pdata += ((*pdata & 0x3F) + 1);

	EOF_CHECK(eof, pdata);
	pbrd->serial_num_tl =	IPMI_SPROM_TYPE_CODE_LC_MSK |
				IPMI_SPROM_BOARD_SERIAL_NUMBER_SIZE; 
	ipmi_sprom_string_cpy(pdata+1, eof ? 0 : (*pdata & 0x3F),
		(char*)&pbrd->serial_num, IPMI_SPROM_BOARD_SERIAL_NUMBER_SIZE);
	pdata += ((*pdata & 0x3F) + 1);

	EOF_CHECK(eof, pdata);
	pbrd->part_num_tl = IPMI_SPROM_TYPE_CODE_LC_MSK |
			    IPMI_SPROM_BOARD_PART_NUMBER_SIZE; 
	ipmi_sprom_string_cpy(pdata+1, eof ? 0 : (*pdata & 0x3F),
		(char*)&pbrd->part_num, IPMI_SPROM_BOARD_PART_NUMBER_SIZE);
	pdata += ((*pdata & 0x3F) + 1);

	EOF_CHECK(eof, pdata);
	pbrd->fru_file_id_tl =	IPMI_SPROM_TYPE_CODE_LC_MSK |
				IPMI_SPROM_BOARD_FRU_FILE_ID_SIZE; 
	ipmi_sprom_string_cpy(pdata+1, eof ? 0 : (*pdata & 0x3F),
		(char*)&pbrd->fru_file_id, IPMI_SPROM_BOARD_FRU_FILE_ID_SIZE);
	pdata += ((*pdata & 0x3F) + 1);

	EOF_CHECK(eof, pdata);

	pbrd->custom_id_tl   =	IPMI_SPROM_TYPE_CODE_LC_MSK |
				IPMI_SPROM_BOARD_CUSTOM_ID_SIZE; 
	if ((*pdata & 0x3F) == IPMI_SPROM_BOARD_CUSTOM_ID_SIZE_OLD) {
		printf("Fixing Byte Format\n");
		unib = (*(pdata+1) >>4) & 0x0F;
		lnib =  *(pdata+1) & 0x0F;

		pbrd->bom_rev[0] = (unib > 9)?('A'+(unib-0xA)) : ('0' + unib);
		pbrd->bom_rev[1] = (lnib > 9)?('A'+(lnib-0xA)) : ('0' + lnib);

		ipmi_sprom_string_cpy(pdata+2, eof ? 0 : (*pdata & 0x3F),
		    (char*)&pbrd->hw_rev, ((*pdata&0x3F)-1));
	} else {
		ipmi_sprom_string_cpy(pdata+1, eof ? 0 : (*pdata & 0x3F),
		    (char*)pbrd->bom_rev, (*pdata & 0x3F));
	}

	pdata += ((*pdata & 0x3F) + 1);

	EOF_CHECK(eof, pdata);
	pbrd->clei_tl	=  IPMI_SPROM_TYPE_CODE_LC_MSK |
				IPMI_SPROM_BOARD_CLEI_SIZE; 
	ipmi_sprom_string_cpy(pdata+1, eof ? 0 : (*pdata & 0x3F),
		(char*)&pbrd->clei, IPMI_SPROM_BOARD_CLEI_SIZE);
	pdata += ((*pdata & 0x3F) + 1);
	EOF_CHECK(eof, pdata);
	pbrd->pad_tl	=  IPMI_SPROM_TYPE_CODE_LC_MSK |
			IPMI_SPROM_BOARD_NIM_PAD_SIZE;
	ipmi_sprom_string_cpy(pdata+1, eof ? 0 : (*pdata & 0x3F),
			(char*)&pbrd->pad_tl, IPMI_SPROM_BOARD_NIM_PAD_SIZE);
	pdata += ((*pdata & 0x3F) + 1);
	pbrd->no_more_tl      = IPMI_SPROM_NO_MORE_TYPE_LENGTH; 
	ipmi_zero_checksum_create((uint8_t*)pbrd, sizeof(*pbrd));
	return (rc);
}


int ipmi_sprom_chassis_info_hfmt(char *pbuf, ipmi_sprom_chassis_info_t* pchs)
{
	int rc = 0;
	char *pdata = NULL, eof = 0;

	if (!pchs || !pbuf) return (-1);

	memset(pchs, 0, sizeof(*pchs));

	// Get the pointer to the board info area
	pdata = pbuf + (((ipmi_sprom_common_header_t*)pbuf)->chassis_info << 3);

	//Assign first 3 bytes as is.
	pchs->version	      =  pdata[0];
	pchs->length	      =  pdata[1]; 
	pchs->type	      =  pdata[2]; 
	pdata+= 3;

	EOF_CHECK(eof, pdata);
	if (!eof) {
		pchs->part_num_tl = IPMI_SPROM_TYPE_CODE_LC_MSK |
			    IPMI_SPROM_CHASSIS_PART_NUMBER_SIZE; 
		ipmi_sprom_string_cpy(pdata+1, (*pdata & 0x3F),
			(char*)&pchs->part_num, IPMI_SPROM_CHASSIS_PART_NUMBER_SIZE);
		pdata += ((*pdata & 0x3F) + 1);
	
		EOF_CHECK(eof, pdata);
	}

	if (!eof) {
		pchs->serial_num_tl =	IPMI_SPROM_TYPE_CODE_LC_MSK |
				IPMI_SPROM_CHASSIS_SERIAL_NUMBER_SIZE; 
		ipmi_sprom_string_cpy(pdata+1, (*pdata & 0x3F),
			(char*)&pchs->serial_num, 
			IPMI_SPROM_CHASSIS_SERIAL_NUMBER_SIZE);
		pdata += ((*pdata & 0x3F) + 1);

		EOF_CHECK(eof, pdata);
	}
	if (!eof) {
		pchs->mfg_info_tl =	IPMI_SPROM_TYPE_CODE_LC_MSK |
				IPMI_SPROM_CHASSIS_MFG_INFO_SIZE; 
		ipmi_sprom_string_cpy(pdata+1, (*pdata & 0x3F),
			(char*)&pchs->mfg_info, 
			IPMI_SPROM_CHASSIS_MFG_INFO_SIZE);
		pdata += ((*pdata & 0x3F) + 1);
	}
	pchs->no_more_tl      = IPMI_SPROM_NO_MORE_TYPE_LENGTH; 
	ipmi_zero_checksum_create((uint8_t*)pchs, sizeof(*pchs));
	return (rc);
}


int ipmi_sprom_iu_iom_hfmt(char *pbuf, ipmi_iom_internal_use_t* piu)
{
	int rc = 0;
	ipmi_iom_internal_use_t *pdata;
	
	if (!pbuf || !piu) return (-1);

	pdata = (ipmi_iom_internal_use_t *) (pbuf +
		(((ipmi_sprom_common_header_t*)pbuf)->internal_use << 3));
	if (!pdata) return (-1);


	memset(piu, 0, sizeof(*piu));
	piu->version   = pdata->version; 
	piu->length    = M8_SIZE(sizeof(ipmi_iom_internal_use_t)); 
	piu->card_type = ntohs(pdata->card_type); 

	// Convert Sub Type.
	piu->mac.srec.sub_type	   = ntohs(pdata->mac.srec.sub_type);
	piu->mac.srec.sub_type_len = ntohs(pdata->mac.srec.sub_type_len);

	// Get Cnt rec
	piu->mac.cnt.type  = pdata->mac.cnt.type;
	piu->mac.cnt.len   = pdata->mac.cnt.len;
	piu->mac.cnt.value = ntohs(pdata->mac.cnt.value);

	// Get Mac Rec
	piu->mac.type  = pdata->mac.type;
	piu->mac.len   = pdata->mac.len;
	piu->mac.mac[0]= pdata->mac.mac[0];
	piu->mac.mac[1]= pdata->mac.mac[1];
	piu->mac.mac[2]= pdata->mac.mac[2];
	piu->mac.mac[3]= pdata->mac.mac[3];
	piu->mac.mac[4]= pdata->mac.mac[4];
	piu->mac.mac[5]= pdata->mac.mac[5];

	// Assign the srecs.
	piu->brd_temp1 = pdata->brd_temp1;
	piu->brd_temp2 = pdata->brd_temp2;
	piu->rw_temp1 = pdata->rw_temp1;
	piu->rw_temp2 = pdata->rw_temp2;

	SDR_NTOHS(piu, brd_temp1);
	SDR_NTOHS(piu, brd_temp2);
	SDR_NTOHS(piu, rw_temp1);
	SDR_NTOHS(piu, rw_temp1);
	ipmi_zero_checksum_create((uint8_t*)piu, sizeof(*piu));
	return (rc);
}

int ipmi_sprom_iu_fan_hfmt(char *pbuf, ipmi_fan_internal_use_t* piu)
{
	int rc = 0;
	ipmi_fan_internal_use_t *pdata;
	
	if (!pbuf || !piu) return (-1);

	pdata = (ipmi_fan_internal_use_t *) (pbuf +
		(((ipmi_sprom_common_header_t*)pbuf)->internal_use << 3));
	if (!pdata) return (-1);

	memset(piu, 0, sizeof(*piu));
	piu->version   = pdata->version; 
	piu->length    = M8_SIZE(sizeof(ipmi_fan_internal_use_t)); 
	piu->card_type = ntohs(pdata->card_type); 

	// Assign the srecs.
	piu->fan_rpm  = pdata->fan_rpm;
	piu->fan_temp = pdata->fan_temp;
	piu->fan_volt = pdata->fan_volt;

	SDR_NTOHS(piu, fan_rpm);
	SDR_NTOHS(piu, fan_temp);
	SDR_NTOHS(piu, fan_volt);
	ipmi_zero_checksum_create((uint8_t*)piu, sizeof(*piu));
	return (rc);
}

int ipmi_sprom_iu_psu_hfmt(char *pbuf, ipmi_psu_internal_use_t* piu)
{
	ipmi_psu_internal_use_t *pdata;
	
	if (!pbuf || !piu) return (-1);

	pdata = (ipmi_psu_internal_use_t *) (pbuf +
		(((ipmi_sprom_common_header_t*)pbuf)->internal_use << 3));
	if (!pdata) return (-1);

	memset(piu, 0, sizeof(*piu));
	piu->version   = pdata->version; 
	piu->length    = M8_SIZE(sizeof(ipmi_psu_internal_use_t)); 
	piu->card_type = ntohs(pdata->card_type); 

	// Assign the srecs.
	piu->psu_rpm  = pdata->psu_rpm;
	piu->psu_temp = pdata->psu_temp;
	piu->psu_in_volt = pdata->psu_in_volt;
	piu->psu_out_volt1 = pdata->psu_out_volt1;
	piu->psu_out_volt2 = pdata->psu_out_volt2;
	piu->psu_in_current = pdata->psu_in_current;
	piu->psu_out_current = pdata->psu_out_current;

	SDR_NTOHS(piu, psu_rpm);
	SDR_NTOHS(piu, psu_temp);
	SDR_NTOHS(piu, psu_in_volt);
	SDR_NTOHS(piu, psu_out_volt1);
	SDR_NTOHS(piu, psu_out_volt2);
	SDR_NTOHS(piu, psu_in_current);
	SDR_NTOHS(piu, psu_out_current);

	ipmi_zero_checksum_create((uint8_t*)piu, sizeof(*piu));
	return (0);
}

int ipmi_sprom_iu_mezz_hfmt(char *pbuf, ipmi_mezz_internal_use_t* piu)
{
	int rc = 0;
	ipmi_mezz_internal_use_t *pdata;
	
	if (!pbuf || !piu) return (-1);

	pdata = (ipmi_mezz_internal_use_t *) (pbuf +
		(((ipmi_sprom_common_header_t*)pbuf)->internal_use << 3));
	if (!pdata) return (-1);


	memset(piu, 0, sizeof(*piu));
	piu->version   = pdata->version; 
	piu->length    = M8_SIZE(sizeof(ipmi_mezz_internal_use_t)); 
	piu->card_type = ntohs(pdata->card_type); 

	// Convert Sub Type.
	piu->mac.srec.sub_type	   = ntohs(pdata->mac.srec.sub_type);
	piu->mac.srec.sub_type_len = ntohs(pdata->mac.srec.sub_type_len);

	// Get Cnt rec
	piu->mac.cnt.type  = pdata->mac.cnt.type;
	piu->mac.cnt.len   = pdata->mac.cnt.len;
	piu->mac.cnt.value = ntohs(pdata->mac.cnt.value);

	// Get Mac Rec
	piu->mac.type  = pdata->mac.type;
	piu->mac.len   = pdata->mac.len;
	piu->mac.mac[0]= pdata->mac.mac[0];
	piu->mac.mac[1]= pdata->mac.mac[1];
	piu->mac.mac[2]= pdata->mac.mac[2];
	piu->mac.mac[3]= pdata->mac.mac[3];
	piu->mac.mac[4]= pdata->mac.mac[4];
	piu->mac.mac[5]= pdata->mac.mac[5];

	ipmi_zero_checksum_create((uint8_t*)piu, sizeof(*piu));
	return (rc);
}

int ipmi_sprom_internal_use_get(int card_type, char *pbuf, int buf_len,
			   char *psprom, int psprom_len)
{
	int rc = 0;

	// Check for input buffers
	if (!pbuf || !psprom) {
		return (-1);
	}

	// Verify the checksum and size.
	rc = ipmi_sprom_data_sanity_check(pbuf, buf_len);
	if (rc) return (-1);

	switch (card_type) {
		case	IPMI_IU_CARD_TYPE_IOM:
		case	IPMI_IU_CARD_TYPE_IOM2:
			{
			ipmi_iom_internal_use_t *pfru = 
				(ipmi_iom_internal_use_t*)psprom;	
			if (psprom_len < (int)sizeof(*pfru))
				return (-1);
			rc = ipmi_sprom_iu_iom_hfmt(pbuf, pfru);
			}
			break;

		case	IPMI_IU_CARD_TYPE_SC_FAN:
			{
			ipmi_fan_internal_use_t *pfru = 
				(ipmi_fan_internal_use_t*)psprom;	
			if (psprom_len < (int)sizeof(*pfru))
				return (-1);
			rc = ipmi_sprom_iu_fan_hfmt(pbuf, pfru);
			}
			break;

		case	IPMI_IU_CARD_TYPE_SC_PSU:
			{
			ipmi_psu_internal_use_t *pfru = 
				(ipmi_psu_internal_use_t*)psprom;	
			if (psprom_len < (int)sizeof(*pfru))
				return (-1);
			rc = ipmi_sprom_iu_psu_hfmt(pbuf, pfru);
			}
			break;

		case	IPMI_IU_CARD_TYPE_IBMC:
		case	IPMI_IU_CARD_TYPE_OPLIN:
		case	IPMI_IU_CARD_TYPE_MENLO:
		case	IPMI_IU_CARD_TYPE_MENLO_E:
		case	IPMI_IU_CARD_TYPE_PALO:
		case	IPMI_IU_CARD_TYPE_VASONA:
		case	IPMI_IU_CARD_TYPE_NIANTIC:
		case	IPMI_IU_CARD_TYPE_NETEFFECT:
		case	IPMI_IU_CARD_TYPE_SCHULTZ:
		case	IPMI_IU_CARD_TYPE_TIGERSHARK:
		case	IPMI_IU_CARD_TYPE_EVEREST:
		case	IPMI_IU_CARD_TYPE_DUBLIN:
		case	IPMI_IU_CARD_TYPE_FREMONT:
		case	IPMI_IU_CARD_TYPE_LIVERMORE:
			{
			ipmi_mezz_internal_use_t *pfru = 
				(ipmi_mezz_internal_use_t*)psprom;	
			if (psprom_len < (int)sizeof(*pfru))
				return (-1);
			rc = ipmi_sprom_iu_mezz_hfmt(pbuf, pfru);
			}
			break;

		default:
			break;
	}

	return (rc);
}


int ipmi_sprom_format_iom(char *pbuf, sprom_ipmi_iom_t *pfru)
{
	int rc = 0;

	// Sanity check.
	if (!pfru || !pbuf) return (-1);

	// Copy common header.
	rc = ipmi_sprom_common_header_hfmt(pbuf, &pfru->common_header);
	if (rc) return (rc);

	// Copy Board Info.
	rc = ipmi_sprom_board_info_hfmt(pbuf, &pfru->board_info);
	if (rc) return (rc);

	// Copy internal use.
	ipmi_sprom_iu_iom_hfmt(pbuf, &pfru->internal_use);
	if (rc) return (rc);

	return (rc);
}

int ipmi_sprom_format_bp(char *pbuf, sprom_ipmi_iom_bp_t *pfru)
{
	int rc = 0;

	// Sanity check.
	if (!pfru || !pbuf) return (-1);

	// Copy common header.
	rc = ipmi_sprom_common_header_hfmt(pbuf, &pfru->common_header);
	if (rc) return (rc);

	// Copy Chassis Info.
	rc = ipmi_sprom_chassis_info_hfmt(pbuf, &pfru->chassis_info);
	if (rc) return (rc);

	// Copy Board Info.
	rc = ipmi_sprom_board_info_hfmt(pbuf, &pfru->board_info);
	if (rc) return (rc);


	return (rc);
}

int ipmi_sprom_format_fan(char *pbuf, sprom_ipmi_iom_fan_t *pfru)
{
	int rc = 0;

	// Sanity check.
	if (!pfru || !pbuf) return (-1);

	// Copy common header.
	rc = ipmi_sprom_common_header_hfmt(pbuf, &pfru->common_header);
	if (rc) return (rc);

	// Copy Board Info.
	rc = ipmi_sprom_board_info_hfmt(pbuf, &pfru->board_info);
	if (rc) return (rc);

	// Copy internal use.
	ipmi_sprom_iu_fan_hfmt(pbuf, &pfru->internal_use);
	if (rc) return (rc);

	return (rc);
}

int ipmi_sprom_format_psu(char *pbuf, sprom_ipmi_iom_psu_t *pfru)
{
	int rc = 0;
	// Sanity check.
	if (!pfru || !pbuf) return (-1);

	// Copy common header.
	rc = ipmi_sprom_common_header_hfmt(pbuf, &pfru->common_header);
	if (rc) return (rc);

	// Copy Product Info.
	rc = ipmi_sprom_product_info_hfmt(pbuf, &pfru->product_info);
	if (rc) return (rc);

	// Copy internal use.
	ipmi_sprom_iu_psu_hfmt(pbuf, &pfru->internal_use);
	if (rc) return (rc);

	return (rc);
}


int ipmi_sprom_format_mezz(int card_type, char *pbuf, 
			   sprom_ipmi_mezz_t *pfru)
{
	int rc = 0;

	// Sanity check.
	if (!pfru || !pbuf) return (-1);

	// Copy common header.
	rc = ipmi_sprom_common_header_hfmt(pbuf, &pfru->common_header);
	if (rc) return (rc);

	// Copy Board Info.
	rc = ipmi_sprom_board_info_hfmt(pbuf, &pfru->board_info);
	if (rc) return (rc);

	// Copy internal use.
	ipmi_sprom_iu_mezz_hfmt(pbuf, &pfru->internal_use);
	if (rc) return (rc);

	return (rc);
}

int ipmi_sprom_format_bmc(int card_type, char *pbuf, 
			   sprom_ipmi_bmc_t *pfru)
{
	int rc = 0;

	// Sanity check.
	if (!pfru || !pbuf) return (-1);

	// Copy common header.
	rc = ipmi_sprom_common_header_hfmt(pbuf, &pfru->common_header);
	if (rc) return (rc);

	// Copy Board Info.
	rc = ipmi_sprom_board_info_hfmt(pbuf, &pfru->board_info);
	if (rc) return (rc);

	// Copy Product Info.
	rc = ipmi_sprom_bmc_product_info_hfmt(pbuf, &pfru->product_info);
	if (rc) return (rc);

	// Copy internal use.
	ipmi_sprom_iu_mezz_hfmt(pbuf, (ipmi_mezz_internal_use_t*)
					&pfru->internal_use);
	if (rc) return (rc);

	return (rc);
}

int ipmi_sprom_format_ibmc(int card_type, char *pbuf, 
			   sprom_ipmi_ibmc_t *pfru)
{
	return (ipmi_sprom_format_bmc(card_type, pbuf, pfru));
}


int ipmi_sprom_data_format(int card_type, char *pbuf, int buf_len,
			   char *psprom, int psprom_len)
{
	int rc = 0;
	switch (card_type) {
		case	IPMI_IU_CARD_TYPE_IOM:
		case	IPMI_IU_CARD_TYPE_IOM2:
			{
			sprom_ipmi_iom_t *pfru = (sprom_ipmi_iom_t*)psprom;	
			if (psprom_len < (int)sizeof(*pfru))
				return (-1);

			rc = ipmi_sprom_format_iom(pbuf, pfru);
			}
			break;

		case	IPMI_IU_CARD_TYPE_SC_FAN:
			{
			sprom_ipmi_iom_fan_t *pfru = (sprom_ipmi_iom_fan_t*)psprom;	
			if (psprom_len < (int)sizeof(*pfru))
				return (-1);
			rc = ipmi_sprom_format_fan(pbuf, pfru);
			}
			break;

		case	IPMI_IU_CARD_TYPE_SC_PSU:
			{
			sprom_ipmi_iom_psu_t *pfru = (sprom_ipmi_iom_psu_t*)psprom;	
			if (psprom_len < (int)sizeof(*pfru))
				return (-1);
			rc = ipmi_sprom_format_psu(pbuf, pfru);
			}
			break;

		case	IPMI_IU_CARD_TYPE_SC_BP:
			{
			sprom_ipmi_iom_bp_t *pfru = (sprom_ipmi_iom_bp_t*)psprom;	
			if (psprom_len < (int)sizeof(*pfru))
				return (-1);
			rc = ipmi_sprom_format_bp(pbuf, pfru);
			}
			break;

		case	IPMI_IU_CARD_TYPE_OPLIN:
		case	IPMI_IU_CARD_TYPE_MENLO:
		case	IPMI_IU_CARD_TYPE_MENLO_E:
		case	IPMI_IU_CARD_TYPE_PALO:
		case	IPMI_IU_CARD_TYPE_VASONA:
		case	IPMI_IU_CARD_TYPE_NIANTIC:
		case	IPMI_IU_CARD_TYPE_NETEFFECT:
		case	IPMI_IU_CARD_TYPE_SCHULTZ:
		case	IPMI_IU_CARD_TYPE_TIGERSHARK:
		case	IPMI_IU_CARD_TYPE_EVEREST:
		case	IPMI_IU_CARD_TYPE_DUBLIN:
		case	IPMI_IU_CARD_TYPE_FREMONT:
		case	IPMI_IU_CARD_TYPE_LIVERMORE:
			{
			sprom_ipmi_mezz_t *pfru = (sprom_ipmi_mezz_t*)psprom;	
			if (psprom_len < (int)sizeof(*pfru))
				return (-1);
			rc = ipmi_sprom_format_mezz(card_type, pbuf, pfru);
			}
			break;


		case	IPMI_IU_CARD_TYPE_IBMC:
			break;

		default:
			break;
	}

	return (rc);
}


extern void ipmi_raw_dump (uint8_t *buf, int buflen);
int ipmi_sprom_data_get (int card_type, char *pbuf, int pbuf_len, 
			 void *psprom, int psprom_len)
{
	int rc = 0;

	// Check for input buffers
	if (!pbuf || !psprom) {
		return (-1);
	}

	// Verify the checksum and size.
	rc = ipmi_sprom_data_sanity_check(pbuf, pbuf_len);
	if (rc) {
		printf(" Error: Sanity check failed rc=%d\n", rc);
		return (-1);
	}

	// Now move data into the host sprom format.
	rc = ipmi_sprom_data_format(card_type, pbuf, pbuf_len,
					psprom, psprom_len);
	if (rc) {
		printf(" Error: Data Format failed rc=%d\n", rc);
		return (-1);
	}

	return (0);
}

int ipmi_sprom_mac_get (char *pbuf, int buf_len, uint16_t *cnt, char *mac, 
			uint16_t *card_type)
{
	ipmi_sprom_common_header_t *phdr = (ipmi_sprom_common_header_t*)pbuf;
	ipmi_iu_rec_mac_t *mrec = NULL;	
	char *piu;

	// Check for input buffers
	if (!phdr || !(phdr->internal_use)) {
		return (-1);
	}
	piu  = pbuf + (phdr->internal_use << 3);
	mrec = (ipmi_iu_rec_mac_t*)(piu+4);

	*card_type = ntohs(*(uint16_t*)(piu+2));
	*cnt = ntohs(mrec->cnt.value);

	memcpy(mac, mrec->mac, 6);
	return (0);
}

int ipmi_sprom_sd_product_info_hfmt(char *pbuf, 
		ipmi_sprom_psu_product_info_t* pprd)
{
	int rc = 0;
	char *pdata = NULL, eof = 0;

	if (!pprd || !pbuf) return (-1);

	memset(pprd, 0, sizeof(*pprd));

	// Get the pointer to the product info area
	pdata = pbuf + (((ipmi_sprom_common_header_t*)pbuf)->product_info << 3);

	// Now convert the data.
	pprd->version	      =  pdata[0]; 
	pprd->length	      =  pdata[1]; 
	pprd->language_code   =  pdata[2]; 

	EOF_CHECK(eof, pdata);
	pprd->mfg_name_tl     =  IPMI_SPROM_TYPE_CODE_LC_MSK |
				 IPMI_SPROM_SD_PRODUCT_MFG_NAME_SIZE; 
	ipmi_sprom_string_cpy(pdata+1, eof ? 0 : (*pdata & 0x3F),
		(char*)&pprd->mfg_name, IPMI_SPROM_SD_PRODUCT_MFG_NAME_SIZE);
	pdata += ((*pdata & 0x3F) + 1);

	EOF_CHECK(eof, pdata);
	pprd->product_name_tl = IPMI_SPROM_TYPE_CODE_LC_MSK |
				IPMI_SPROM_SD_PRODUCT_NAME_SIZE; 
	ipmi_sprom_string_cpy(pdata+1, eof ? 0 : (*pdata & 0x3F),
		(char*)&pprd->product_name, IPMI_SPROM_SD_PRODUCT_NAME_SIZE);
	pdata += ((*pdata & 0x3F) + 1);

	EOF_CHECK(eof, pdata);
	pprd->part_model_tl = IPMI_SPROM_TYPE_CODE_LC_MSK |
			    IPMI_SPROM_SD_PRODUCT_PART_MODEL_SIZE; 
	ipmi_sprom_string_cpy(pdata+1, eof ? 0 : (*pdata & 0x3F),
		(char*)&pprd->part_model, IPMI_SPROM_SD_PRODUCT_PART_MODEL_SIZE);
	pdata += ((*pdata & 0x3F) + 1);

	EOF_CHECK(eof, pdata);
	pprd->prd_version_tl = IPMI_SPROM_TYPE_CODE_LC_MSK |
			    IPMI_SPROM_SD_PRODUCT_VERSION_SIZE; 
	ipmi_sprom_string_cpy(pdata+1, eof ? 0 : (*pdata & 0x3F),
		(char*)&pprd->prd_version, IPMI_SPROM_SD_PRODUCT_VERSION_SIZE);
	pdata += ((*pdata & 0x3F) + 1);

	EOF_CHECK(eof, pdata);
	pprd->serial_num_tl =	IPMI_SPROM_TYPE_CODE_LC_MSK |
				IPMI_SPROM_SD_PRODUCT_SERIAL_NUMBER_SIZE; 
	ipmi_sprom_string_cpy(pdata+1, eof ? 0 : (*pdata & 0x3F),
	(char*)&pprd->serial_num, IPMI_SPROM_SD_PRODUCT_SERIAL_NUMBER_SIZE);
	pdata += ((*pdata & 0x3F) + 1);

	EOF_CHECK(eof, pdata);

	pprd->asset_tag_tl =	IPMI_SPROM_TYPE_CODE_LC_MSK;
	pdata += ((*pdata & 0x3F) + 1);
	EOF_CHECK(eof, pdata);

	pprd->fru_file_id_tl =	IPMI_SPROM_TYPE_CODE_LC_MSK;
	pdata += ((*pdata & 0x3F) + 1);
	EOF_CHECK(eof, pdata);

	// copy thee next 9 bytes.
	memcpy(&pprd->custom_id_tl, pdata, 9);
	pdata+=9;
	pprd->no_more_tl      = IPMI_SPROM_NO_MORE_TYPE_LENGTH; 
	pdata++;
	ipmi_zero_checksum_create((uint8_t*)pprd, sizeof(*pprd));
	return (rc);
}

int ipmi_sprom_sd_psu_mr_hfmt(char *pbuf, ipmi_sprom_psu_mr_t* pmr)
{
	ipmi_sprom_psu_mr_t *pMrBuf = (ipmi_sprom_psu_mr_t*)pbuf;

        pmr->psu_info_hdr.record_type     = pMrBuf->psu_info_hdr.record_type;
        pmr->psu_info_hdr.version         = pMrBuf->psu_info_hdr.version;
        pmr->psu_info_hdr.length          = pMrBuf->psu_info_hdr.length;
        pmr->psu_info_hdr.record_checksum = pMrBuf->psu_info_hdr.record_checksum;
        pmr->psu_info_hdr.header_checksum = pMrBuf->psu_info_hdr.header_checksum;

        pmr->oem[0]                       = pMrBuf->oem[0]; 
        pmr->oem[1]                       = pMrBuf->oem[1]; 
        pmr->oem[2]                       = pMrBuf->oem[2]; 
        pmr->oem[3]                       = pMrBuf->oem[3]; 
        pmr->oem[4]                       = pMrBuf->oem[4]; 

        pmr->version[0]                   = pMrBuf->version[0]; 
        pmr->version[1]                   = pMrBuf->version[1]; 
        pmr->version[2]                   = pMrBuf->version[2]; 
        pmr->version[3]                   = pMrBuf->version[3]; 

        pmr->power[0]                     = pMrBuf->power[0]; 
        pmr->power[1]                     = pMrBuf->power[1]; 
	return (0);
}


int ipmi_sprom_sd_format_psu(char *pbuf, sprom_ipmi_sd_psu_t *pfru)
{
	int rc = 0;
	// Sanity check.
	if (!pfru || !pbuf) return (-1);

	// Copy common header.
	rc = ipmi_sprom_common_header_hfmt(pbuf, &pfru->common_header);
	if (rc) return (rc);

	// Copy Internal Use.
	rc = ipmi_sprom_iu_psu_hfmt(pbuf, 
			(ipmi_psu_internal_use_t*) &pfru->internal_use);
	if (rc) return (rc);

	// Copy Product Info.
	rc = ipmi_sprom_sd_product_info_hfmt(pbuf, &pfru->product_info);
	if (rc) return (rc);

	// Copy Multi Record.
	ipmi_sprom_sd_psu_mr_hfmt(pbuf, &pfru->multi_record);
	return (rc);
}
