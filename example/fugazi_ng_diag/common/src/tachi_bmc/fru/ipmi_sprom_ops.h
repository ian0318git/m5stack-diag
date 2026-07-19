/* $Id: ipmi_sprom_ops.h,v 1.2 2016/04/20 08:41:36 benchen2 Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/tachi_bmc/fru/ipmi_sprom_ops.h,v $
 *
 *      File:  ipmi_sprom_ops.h.
 *      Name:  matt strathman / Sudharshan Kadari
 *
 *      Description: private sprom util file 
 *
 * Copyright (c) 2016 by Cisco Systems, Inc.
 * All rights reserved.
 *
 **********************************************************************/
/**********************************************************************
@file    sprom.c
@author  matt strathman -- mstrathman@nuovasystems.com
@brief   private sprom util file 
************************************************************************/

#ifndef __IPMI_SPROM_OPS_H__
#define __IPMI_SPROM_OPS_H__
#include <errno.h>

#define ipmi_sprom_err( fmt, args... ) fprintf ( stderr, fmt, ## args )
#include "ipmi_sprom.h"

#ifdef cplusplus
extern "C" {
#endif

// Generic API's
extern uint8_t ipmi_zero_checksum_get (uint8_t *buf, int buflen);
extern int ipmi_header_checksum_verify (uint8_t *buf);
extern int ipmi_sprom_product_info_checksum_verify (uint8_t *buf, size_t len);
extern int ipmi_sprom_chassis_info_checksum_verify (uint8_t *buf, size_t len);
extern int ipmi_sprom_board_info_checksum_verify (uint8_t *buf, size_t len);
extern void ipmi_raw_dump (uint8_t *buf, int buflen);
extern void sprom_mem_fill(char *myptr, char *mystr, uint32_t sz);
extern int ipmi_sprom_internal_use_create (void *pinternal, uint16_t card_type);
extern int ipmi_sprom_common_header_create( ipmi_sprom_common_header_t *phdr,
                uint8_t internal_use_sz, uint8_t chassis_info_sz,
                uint8_t  board_info_sz, uint8_t product_info_sz,
                uint8_t multi_record_sz);
extern int ipmi_sprom_data_get (int card_type, char *pbuf, int pbuf_len,
                         void *psprom, int psprom_len);

// Saratoga/Soquel API's
extern int ipmi_sprom_iom_fill (sprom_ipmi_iom_t *iom_sprom);
extern int ipmi_sprom_iom_dump(char *psprom);
extern int ipmi_sprom_iom_dflt(sprom_ipmi_iom_t *sprom);
extern void ipmi_sprom_iom_board_dump (sprom_ipmi_iom_t *sprom);
extern int iu_sdr_dflt_iom(ipmi_iu_sdr_t *psdr, uint8_t srec_type);

// Santa Clara chassis Api's
extern int ipmi_sprom_iom_bp_dflt(sprom_ipmi_iom_bp_t *sprom);
extern int ipmi_sprom_iom_bp_fill (sprom_ipmi_iom_bp_t *iom_sprom);
extern int ipmi_sprom_iom_bp_dump(sprom_ipmi_iom_t *psprom);
extern void ipmi_sprom_iom_chassis_dump (sprom_ipmi_iom_bp_t *sprom);

// Fan API's
extern int  ipmi_sprom_iom_fan_create (sprom_ipmi_iom_fan_t *sprom);
extern void ipmi_sprom_iom_fan_dump (sprom_ipmi_iom_fan_t *sprom);
extern int  ipmi_sprom_iom_fan_fill(sprom_ipmi_iom_fan_t *iom_sprom);
extern int ipmi_sprom_iom_fan_dflt(sprom_ipmi_iom_fan_t *sprom);

// PSU API's
extern int ipmi_sprom_iom_psu_dflt(sprom_ipmi_iom_psu_t *sprom);
extern int ipmi_sprom_iom_psu_create (sprom_ipmi_iom_psu_t *sprom);
extern void ipmi_sprom_iom_psu_dump (sprom_ipmi_iom_psu_t *sprom);
extern int ipmi_sprom_iom_psu_fill(sprom_ipmi_iom_psu_t *iom_sprom);

extern int ipmi_sprom_iom_psu_p0_dflt(sprom_ipmi_iom_psu_p0_t *sprom);
extern int ipmi_sprom_iom_psu_p0_create (sprom_ipmi_iom_psu_p0_t *sprom);
extern void ipmi_sprom_iom_psu_p0_dump (sprom_ipmi_iom_psu_p0_t *sprom);
extern int ipmi_sprom_iom_psu_p0_fill(sprom_ipmi_iom_psu_p0_t *iom_sprom);

// Mezzanine sprom api's
extern int ipmi_sprom_mezz_fill(sprom_ipmi_mezz_t *sprom, int card_type);
extern void ipmi_sprom_mezz_dump (sprom_ipmi_mezz_t *sprom, int card_type);
extern int ipmi_sprom_mezz_dflt(sprom_ipmi_mezz_t *sprom, uint8_t mezz_type);
extern int ipmi_mezz_iu_create (ipmi_mezz_internal_use_t* piu, uint16_t card_type);
extern int ipmi_mezz_iu_dump (ipmi_mezz_internal_use_t* piu);

// Monterey Park sprom api's
extern int ipmi_sprom_mpark_fill(sprom_ipmi_mpark_t *sprom, int card_type);
extern void ipmi_sprom_mpark_dump (sprom_ipmi_mpark_t *sprom, int card_type);
extern int ipmi_sprom_mpark_dflt(sprom_ipmi_mpark_t *sprom, uint8_t mpark_type);
extern int ipmi_mpark_iu_create (ipmi_mezz_internal_use_t* piu, uint16_t card_type);
extern int ipmi_mpark_iu_dump (ipmi_mezz_internal_use_t* piu);

// Application API's
extern int ipmi_sprom_mac_get (char *pbuf, int buf_len, 
				uint16_t *cnt, char *mac, 
				uint16_t *card_type);
extern int ipmi_sprom_data_get (int card_type, char *pbuf, 
				int pbuf_len, void *psprom, 
				int psprom_len);
extern int ipmi_sprom_internal_use_get(int card_type, 
				char *pbuf, int buf_len,
				char *psprom, int psprom_len);
extern int ipmi_sprom_common_header_hfmt(char *pbuf, 
			ipmi_sprom_common_header_t *phdr);
extern int ipmi_sprom_board_info_hfmt(char *pbuf, 
			ipmi_sprom_board_info_t* pbrd);
extern int ipmi_sprom_chassis_info_hfmt(char *pbuf, 
			ipmi_sprom_chassis_info_t* pchs);
extern int ipmi_sprom_iu_iom_hfmt(char *pbuf, 
			ipmi_iom_internal_use_t* piu);
extern int ipmi_sprom_iu_fan_hfmt(char *pbuf, 
			ipmi_fan_internal_use_t* piu);
extern int ipmi_sprom_iu_psu_hfmt(char *pbuf, 
			ipmi_psu_internal_use_t* piu);
extern int ipmi_sprom_iu_mezz_hfmt(char *pbuf, 
			ipmi_mezz_internal_use_t* piu);

// IBMC
extern int ipmi_sprom_format_ibmc(int card_type, char *pbuf,
                           sprom_ipmi_ibmc_t *pfru);
extern int ipmi_sprom_ibmc_fill(sprom_ipmi_ibmc_t *sprom, int card_type);
extern void ipmi_sprom_ibmc_dump (sprom_ipmi_ibmc_t *sprom, int card_type);
extern int ipmi_sprom_bmc_dflt(sprom_ipmi_ibmc_t *sprom);
extern int ipmi_sprom_ventura_dflt(sprom_ipmi_bmc_t *sprom);
extern int ipmi_sprom_la_dflt(sprom_ipmi_bmc_t *sprom);

extern int ipmi_sprom_verify_fru(char *pbuf, size_t len);
extern int ipmi_region_checksum_verify(char *pbuf);

extern int ipmi_fru_verify (uint8_t *buf, uint32_t size);
extern int ipmi_sprom_board_info_set_serial(ipmi_sprom_board_info_t *pboard);
extern int ipmi_sprom_bmc_product_info_create (ipmi_sprom_bmc_product_info_t *pproduct);
extern void ipmi_sprom_bmc_product_info_dump (ipmi_sprom_bmc_product_info_t *pproduct);

extern int ipmi_sprom_product_info_hfmt(char *pbuf,
    ipmi_sprom_product_info_t* pprd);

extern int ipmi_sprom_psu_fill(sprom_ipmi_psu_t *sprom);
extern void ipmi_sprom_psu_dump (sprom_ipmi_psu_t *sprom);


extern int ipmi_sprom_sd_product_info_hfmt(char *pbuf, ipmi_sprom_psu_product_info_t* pprd);
extern int ipmi_sprom_sd_format_psu(char *pbuf, sprom_ipmi_sd_psu_t *pfru);
extern int ipmi_sprom_sd_psu_dflt(sprom_ipmi_sd_psu_t *sprom);
extern int ipmi_sprom_sd_product_info_create (ipmi_sprom_psu_product_info_t *pproduct);
extern int ipmi_sprom_sd_mr_create (ipmi_sprom_psu_mr_t *pmr, uint8_t );
extern int ipmi_sprom_sd_psu_fill(sprom_ipmi_sd_psu_t *sprom);
extern void ipmi_sprom_sd_mr_dump (ipmi_sprom_psu_mr_t *pmr);
extern int ipmi_sprom_bbu_fill(sprom_ipmi_bbu_t *sprom, int card_type);
extern int ipmi_sprom_bbu_dflt(sprom_ipmi_bbu_t *sprom);
extern void ipmi_sprom_bbu_dump (sprom_ipmi_bbu_t *sprom);

extern int ipmi_sprom_mem_fill(sprom_ipmi_mem_t *sprom, int card_type);
extern int ipmi_sprom_marin_memfront_dflt(sprom_ipmi_mem_t *sprom);
extern int ipmi_sprom_marin_memback_dflt(sprom_ipmi_mem_t *sprom);
extern void ipmi_sprom_mem_dump (sprom_ipmi_mem_t *sprom, int card_type);

extern int ipmi_sprom_castlerock_dflt(sprom_ipmi_bmc_t *sprom); 
extern int ipmi_sprom_hddbp_fill(sprom_ipmi_hddbp_t *sprom, int card_type);
extern int ipmi_sprom_castlerock_hddbp_dflt(sprom_ipmi_hddbp_t *sprom);
extern int ipmi_sprom_castlerock_tpm_dflt(sprom_ipmi_bbu_t *sprom);
extern void ipmi_sprom_hddbp_dump (sprom_ipmi_hddbp_t *sprom, int card_type);


extern void ipmi_sprom_board_fru_file_id_create(uint8_t *pbuf);
extern void ipmi_sprom_product_fru_file_id_create(uint8_t *pprod);
#ifdef DIAG_ALPINE
extern int ipmi_sprom_alpine_dflt(sprom_ipmi_bmc_t *sprom);
#endif /* DIAG_ALPINE */

#ifdef cplusplus
}
#endif

#endif
