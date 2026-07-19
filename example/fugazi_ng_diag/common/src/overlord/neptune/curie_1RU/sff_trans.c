/* $Id: sff_trans.c,v 1.2 2019/08/06 06:56:15 alpeng Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/overlord/neptune/curie_1RU/sff_trans.c,v $
 * ------------------------------------------------------------------
 *
 * Leschen, 2019
 *
 * This code deals with the 2-wire memory map defined for GBIC, SFP MSA, and
 * SFP+ modules. See SFF specs. 
 *
 * Copyright (c) 2019 by Cisco Systems, Inc.
 * All rights reserved.
 *------------------------------------------------------------------
 */

#include <stdio.h>
#include "sff_trans.h"

#define SFF_LOG

/* TBDJCB 0 copied from crc32.c::extern uint32_t crc32(uint32_t crc, uint8 *pdata, uint nbytes); */

const uint32_t crc_table[256] = {
  0x00000000L, 0x77073096L, 0xee0e612cL, 0x990951baL, 0x076dc419L,
  0x706af48fL, 0xe963a535L, 0x9e6495a3L, 0x0edb8832L, 0x79dcb8a4L,
  0xe0d5e91eL, 0x97d2d988L, 0x09b64c2bL, 0x7eb17cbdL, 0xe7b82d07L,
  0x90bf1d91L, 0x1db71064L, 0x6ab020f2L, 0xf3b97148L, 0x84be41deL,
  0x1adad47dL, 0x6ddde4ebL, 0xf4d4b551L, 0x83d385c7L, 0x136c9856L,
  0x646ba8c0L, 0xfd62f97aL, 0x8a65c9ecL, 0x14015c4fL, 0x63066cd9L,
  0xfa0f3d63L, 0x8d080df5L, 0x3b6e20c8L, 0x4c69105eL, 0xd56041e4L,
  0xa2677172L, 0x3c03e4d1L, 0x4b04d447L, 0xd20d85fdL, 0xa50ab56bL,
  0x35b5a8faL, 0x42b2986cL, 0xdbbbc9d6L, 0xacbcf940L, 0x32d86ce3L,
  0x45df5c75L, 0xdcd60dcfL, 0xabd13d59L, 0x26d930acL, 0x51de003aL,
  0xc8d75180L, 0xbfd06116L, 0x21b4f4b5L, 0x56b3c423L, 0xcfba9599L,
  0xb8bda50fL, 0x2802b89eL, 0x5f058808L, 0xc60cd9b2L, 0xb10be924L,
  0x2f6f7c87L, 0x58684c11L, 0xc1611dabL, 0xb6662d3dL, 0x76dc4190L,
  0x01db7106L, 0x98d220bcL, 0xefd5102aL, 0x71b18589L, 0x06b6b51fL,
  0x9fbfe4a5L, 0xe8b8d433L, 0x7807c9a2L, 0x0f00f934L, 0x9609a88eL,
  0xe10e9818L, 0x7f6a0dbbL, 0x086d3d2dL, 0x91646c97L, 0xe6635c01L,
  0x6b6b51f4L, 0x1c6c6162L, 0x856530d8L, 0xf262004eL, 0x6c0695edL,
  0x1b01a57bL, 0x8208f4c1L, 0xf50fc457L, 0x65b0d9c6L, 0x12b7e950L,
  0x8bbeb8eaL, 0xfcb9887cL, 0x62dd1ddfL, 0x15da2d49L, 0x8cd37cf3L,
  0xfbd44c65L, 0x4db26158L, 0x3ab551ceL, 0xa3bc0074L, 0xd4bb30e2L,
  0x4adfa541L, 0x3dd895d7L, 0xa4d1c46dL, 0xd3d6f4fbL, 0x4369e96aL,
  0x346ed9fcL, 0xad678846L, 0xda60b8d0L, 0x44042d73L, 0x33031de5L,
  0xaa0a4c5fL, 0xdd0d7cc9L, 0x5005713cL, 0x270241aaL, 0xbe0b1010L,
  0xc90c2086L, 0x5768b525L, 0x206f85b3L, 0xb966d409L, 0xce61e49fL,
  0x5edef90eL, 0x29d9c998L, 0xb0d09822L, 0xc7d7a8b4L, 0x59b33d17L,
  0x2eb40d81L, 0xb7bd5c3bL, 0xc0ba6cadL, 0xedb88320L, 0x9abfb3b6L,
  0x03b6e20cL, 0x74b1d29aL, 0xead54739L, 0x9dd277afL, 0x04db2615L,
  0x73dc1683L, 0xe3630b12L, 0x94643b84L, 0x0d6d6a3eL, 0x7a6a5aa8L,
  0xe40ecf0bL, 0x9309ff9dL, 0x0a00ae27L, 0x7d079eb1L, 0xf00f9344L,
  0x8708a3d2L, 0x1e01f268L, 0x6906c2feL, 0xf762575dL, 0x806567cbL,
  0x196c3671L, 0x6e6b06e7L, 0xfed41b76L, 0x89d32be0L, 0x10da7a5aL,
  0x67dd4accL, 0xf9b9df6fL, 0x8ebeeff9L, 0x17b7be43L, 0x60b08ed5L,
  0xd6d6a3e8L, 0xa1d1937eL, 0x38d8c2c4L, 0x4fdff252L, 0xd1bb67f1L,
  0xa6bc5767L, 0x3fb506ddL, 0x48b2364bL, 0xd80d2bdaL, 0xaf0a1b4cL,
  0x36034af6L, 0x41047a60L, 0xdf60efc3L, 0xa867df55L, 0x316e8eefL,
  0x4669be79L, 0xcb61b38cL, 0xbc66831aL, 0x256fd2a0L, 0x5268e236L,
  0xcc0c7795L, 0xbb0b4703L, 0x220216b9L, 0x5505262fL, 0xc5ba3bbeL,
  0xb2bd0b28L, 0x2bb45a92L, 0x5cb36a04L, 0xc2d7ffa7L, 0xb5d0cf31L,
  0x2cd99e8bL, 0x5bdeae1dL, 0x9b64c2b0L, 0xec63f226L, 0x756aa39cL,
  0x026d930aL, 0x9c0906a9L, 0xeb0e363fL, 0x72076785L, 0x05005713L,
  0x95bf4a82L, 0xe2b87a14L, 0x7bb12baeL, 0x0cb61b38L, 0x92d28e9bL,
  0xe5d5be0dL, 0x7cdcefb7L, 0x0bdbdf21L, 0x86d3d2d4L, 0xf1d4e242L,
  0x68ddb3f8L, 0x1fda836eL, 0x81be16cdL, 0xf6b9265bL, 0x6fb077e1L,
  0x18b74777L, 0x88085ae6L, 0xff0f6a70L, 0x66063bcaL, 0x11010b5cL,
  0x8f659effL, 0xf862ae69L, 0x616bffd3L, 0x166ccf45L, 0xa00ae278L,
  0xd70dd2eeL, 0x4e048354L, 0x3903b3c2L, 0xa7672661L, 0xd06016f7L,
  0x4969474dL, 0x3e6e77dbL, 0xaed16a4aL, 0xd9d65adcL, 0x40df0b66L,
  0x37d83bf0L, 0xa9bcae53L, 0xdebb9ec5L, 0x47b2cf7fL, 0x30b5ffe9L,
  0xbdbdf21cL, 0xcabac28aL, 0x53b39330L, 0x24b4a3a6L, 0xbad03605L,
  0xcdd70693L, 0x54de5729L, 0x23d967bfL, 0xb3667a2eL, 0xc4614ab8L,
  0x5d681b02L, 0x2a6f2b94L, 0xb40bbe37L, 0xc30c8ea1L, 0x5a05df1bL,
  0x2d02ef8dL
};
#define DO1(buf) crc = crc_table[((int)crc ^ (*buf++)) & 0xff] ^ (crc >> 8);
#define DO2(buf)  DO1(buf); DO1(buf);
#define DO4(buf)  DO2(buf); DO2(buf);
#define DO8(buf)  DO4(buf); DO4(buf);

uint32_t  sff_crc32(uint32_t crc, const uint8_t *buf, uint32_t len)
{
#ifdef DYNAMIC_CRC_TABLE
    if (crc_table_empty)
      make_crc_table();
#endif
    crc = crc ^ 0xffffffffL;
    while (len >= 8)
    {
      DO8(buf);
      len -= 8;
    }
    if (len) do {
      DO1(buf);
    } while (--len);
    return crc ^ 0xffffffffL;
}

static int
sff_trans_checksum_validate (sff_trans_map_t *sff_trans, int start, int end, uint8_t cksum)
{
    int i;
    int sum = 0;
    int rv = 0;
    uint8_t *block = sff_trans->sff_eeprom;

    if ((cksum == 0) || (cksum == 0xff))
	return rv;

    for (i=start; i<=end; i++)
        sum += *(block + i);
    sum &= 0xff;
    if (sum != cksum) {
	sff_trans->sff_rcksum = cksum;
	sff_trans->sff_ccksum = sum;
        rv = -1;
    }

    return rv;
}

int
sff_trans_eeprom_checksum_base_ids_validate (sff_trans_map_t *sff_trans)
{
    int rv = 0;

    /* Check Base ID Fields 0-62 */
    if ((rv = sff_trans_checksum_validate(sff_trans, 0, 62,
					  SFF_B63_CC_BASE(sff_trans))) < 0) {
	rv = -1;
    }
   
    return rv;
}

int
sff_trans_eeprom_checksum_ext_ids_validate (sff_trans_map_t *sff_trans)
{
    int rv = 0;

    /* Check Extended ID Fields 64-94 */
    if ((rv = sff_trans_checksum_validate(sff_trans, 64, 94,
					  SFF_B95_CC_EXT(sff_trans))) < 0) {
	rv = -1;
    }

    return rv;
}

int
sff_trans_eeprom_checksum_vend_ids_validate (sff_trans_map_t *sff_trans)
{
    int rv = 0;
    uint32_t chksum = 0;
    uint32_t sfp_chksum = 0;

    /*
     * Do CRC-32 check on Vendor Specific ID Fields 96-123
     * total 28 bytes. Get checksum saved in the sp_eeprom[]
     * get both little and big endian
     */
    sfp_chksum =  SFF_CRC_B0(sff_trans) + 
	         (SFF_CRC_B1(sff_trans) <<  8) +
	         (SFF_CRC_B2(sff_trans) << 16) + 
	         (SFF_CRC_B3(sff_trans) << 24);

    /* perform CRC only when the checksum is not zero */
    if (sfp_chksum != 0) {
        chksum  = sff_crc32(0, &SFF_EEPROM(sff_trans, 96), 28);
	if (sfp_chksum != chksum) {
	    sff_trans->sff_rcksum = sfp_chksum;
	    sff_trans->sff_ccksum = chksum;
	    rv = -1;
	}
    }

    return rv;
}

int
sff_trans_eeprom_checksum_vend1_fields_validate (sff_trans_map_t *sff_trans)
{
    int rv = 0;

    /* Check Vendor Specific Fields 128-158 */
    if ((rv = sff_trans_checksum_validate(sff_trans, 128, 158,
				SFF_B159_CC_VENDOR_1(sff_trans))) < 0) {
	rv = -1;
    }

    return rv;
}

int
sff_trans_eeprom_checksum_vend2_fields_validate (sff_trans_map_t *sff_trans)
{
    int rv = 0;

    /* Check Vendor Specific Fields 192-222 */
    if ((rv = sff_trans_checksum_validate(sff_trans, 192, 222,
				SFF_B223_CC_VENDOR_2(sff_trans))) < 0) {
	rv = -1;
    }

    return rv;
}

int
sff_trans_eeprom_checksum_vend3_fields_validate (sff_trans_map_t *sff_trans)
{
    int rv = 0;

    /* Check Vendor Specific Fields 224-244 */
    if ((rv = sff_trans_checksum_validate(sff_trans, 224, 244,
				SFF_B245_CC_VENDOR_3(sff_trans))) < 0) {
	rv = -1;
    }

    return rv;
}

int
sff_trans_check_cisco_pn (sff_trans_map_t *sff_trans)
{
    uint8_t *cisco_pn = &SFF_CISCO_PN(sff_trans);
    
    /* 
     * According to EDCS-553508 Cisco PNs for SFPs have the following format:
     * XX-XXXX-XX.
     *
     * Without performing the security check in SFF-8431 there is no other
     * way of validating a Cisco SFP.
     */
    if (cisco_pn[2] == '-' && cisco_pn[7] == '-')
	return 1;
    
    return 0;
}

/* bcm_esw_sfp_eeprom_cisco_vendor_checksum_validate */
int
sff_sfp_eeprom_cisco_vendor_checksum_validate(sff_trans_map_t *sff_trans)
{
    int rv  = 0;

    /* Check Vendor Specific Fields */
    if ((rv = sff_trans_eeprom_checksum_vend1_fields_validate(sff_trans)) <0) {
      printf("%s(): SFP Vendor1 Specific ID Fields corrupted, rv %d\n",
             __FUNCTION__, rv);
      rv = 0;
    }

    if ((rv = sff_trans_eeprom_checksum_vend2_fields_validate(sff_trans)) <0) {
      printf("%s(): SFP Vendor2 Specific ID Fields corrupted, rv %d\n",
             __FUNCTION__, rv);
      rv = 0;
    }

    if ((rv = sff_trans_eeprom_checksum_vend3_fields_validate(sff_trans)) <0) {
      printf("%s(): SFP Vendor3 Specific ID Fields corrupted, rv %d\n",
             __FUNCTION__, rv);
      rv = 0;
    }

    return rv;
}

sff_sfp_module_id_t sff_get_sfp_module_id(sff_trans_map_t *sff_trans)
{
    uint8_t TC10g         = SFF_B3_TRANS_CODE_10G(sff_trans);
    uint8_t TC1g          = SFF_B6_TRANS_CODE_1G(sff_trans);
    uint8_t SFP_p_cable   = SFF_B8_SFP_PLUS_CABLE(sff_trans);
    uint8_t CB_spec       = SFF_B60_CABLE_SPEC(sff_trans);
    uint8_t Ext_tc        = SFF_B96_EXT_TRANS_CODE(sff_trans);
    uint8_t CB_len        = SFF_B18_CABLE_LENGTH(sff_trans);

    /* The module is not SFP/SFP+ */
    if (!SFF_SFP_PMD_ID(sff_trans)) {
      printf("SFF INFO - %s() SFF_NOT_SFP!\n",__FUNCTION__);
      return (SFF_NOT_SFP);
    }

    /* 10G Fiber Module */
    if (SFF_SFP_10G(TC10g)) {
        
        /* Can perform Magic Code Security Authentication here if required */
      if (SFF_10G_ER(TC10g)) {
        /*printf("SFF INFO - %s() return SFF_SFP_10G_ER.\n",__FUNCTION__);*/
        return(SFF_SFP_10G_ER);
      }
      if (SFF_10G_LR(TC10g)) {
        /*printf("SFF INFO - %s() return SFF_SFP_10G_LR.\n",__FUNCTION__);*/
        return(SFF_SFP_10G_LR);
      }
      if (SFF_10G_SR(TC10g)) {
        /*printf("SFF INFO - %s() return SFF_SFP_10G_SR.\n",__FUNCTION__);*/
        return(SFF_SFP_10G_SR);
      }
      if (SFF_10G_LRM(TC10g)) {
        /*printf("SFF INFO - %s() return SFF_SFP_10G_LRM.\n",__FUNCTION__);*/
        return(SFF_SFP_10G_LRM);
      }
    } else {
      if (SFF_10G_ZR(Ext_tc)) {
        /*printf("SFF INFO - %s() return SFF_SFP_10G_ZR.\n",__FUNCTION__);*/
        return(SFF_SFP_10G_ZR);
      }
    }

    /* 10G Copper Module CX-1*/
    /* Passive Cable */
    if (SFF_PASSIVE_CABLE(SFP_p_cable)) {
      /*printf("SFF INFO - %s() return SFF_SFP_10G_PASS_CX1_X %d.\n",__FUNCTION__, CB_len);*/
      switch (CB_len) {
      case 0x00: return(SFF_SFP_10G_PAS_CX1_1S);
      case 0x01: return(SFF_SFP_10G_PAS_CX1_1);
      case 0x02: return(SFF_SFP_10G_PAS_CX1_2);
      case 0x03: return(SFF_SFP_10G_PAS_CX1_3);
      case 0x04: return(SFF_SFP_10G_PAS_CX1_4);
      case 0x05: return(SFF_SFP_10G_PAS_CX1_5);
      case 0x06: return(SFF_SFP_10G_PAS_CX1_6);
      case 0x07: return(SFF_SFP_10G_PAS_CX1_7);
      }
      return(SFF_SFP_10G_PAS_CX1);
    }

    /* Active Cable */
    if (SFF_ACTIVE_CABLE(SFP_p_cable)) {
      /*printf("SFF INFO - %s() return SFF_SFP_10G_ACT_CX1_X %d.\n",__FUNCTION__, CB_len);*/
      switch (CB_len) {
      case 0x00: return(SFF_SFP_10G_ACT_CX1_1S);
      case 0x01: return(SFF_SFP_10G_ACT_CX1_1);
      case 0x03: return(SFF_SFP_10G_ACT_CX1_3);
      case 0x05: return(SFF_SFP_10G_ACT_CX1_5);
      case 0x07: return(SFF_SFP_10G_ACT_CX1_7);
      }
      return(SFF_SFP_10G_ACT_CX1);
    }

    /* Active Limiting Cable */
    if (SFF_ACTIVE_CABLE(SFP_p_cable) && SFF_8431_LIMIT(CB_spec)) {
      /*printf("SFF INFO - %s() return SFF_SFP_10G_ACT_LIMIT_CX1.\n",__FUNCTION__);*/
      return(SFF_SFP_10G_ACT_LIMIT_CX1);
    }

    /* 1G Fiber Module */
    if (SFF_SFP_1G(TC1g)) {
	/* Can perform Magic Code Security Authentication here if required */
      if (SFF_1000BASE_T(TC1g)) {
        /*printf("SFF INFO - %s() return SFF_SFP_1G_T  %d.\n",__FUNCTION__);*/
        return(SFF_SFP_1G_T);
      }
      if (SFF_1000BASE_CX(TC1g)){
        /*printf("SFF INFO - %s() return SFF_SFP_1G_CX %d.\n",__FUNCTION__);*/
        return(SFF_SFP_1G_CX);
      }
      if (SFF_1000BASE_LX(TC1g)) {
        /*printf("SFF INFO - %s() return SFF_SFP_1G_LX.\n",__FUNCTION__);*/
        return(SFF_SFP_1G_LX);
      }
      if (SFF_1000BASE_SX(TC1g)) {
        /*printf("SFF INFO - %s() return SFF_SFP_1G_SX.\n",__FUNCTION__);*/
        return(SFF_SFP_1G_SX);
      }
        /*printf("SFF INFO - %s() return SFF_SFP_100BASE.\n",__FUNCTION__);*/
        return(SFF_SFP_100BASE);
    }
	
    /* 1G */
    if ((Ext_tc >= 0x1) && (Ext_tc <= 0x3f)) {
      /*printf("SFF INFO - %s() return SFF_SFP_1G_NS.\n",__FUNCTION__);*/
      /* Can perform Magic Code Security Authentication here if required */
      return(SFF_SFP_1G_NS);
    }

    /* Can perform Magic Code Security Authentication here if required */
    /* for supported extended SFP+ module */
    switch (Ext_tc) {
    case 0x80:
     /* printf("SFF INFO - %s() return SFF_SFP_H10GB_CU1M.\n",__FUNCTION__);*/
      return(SFF_SFP_H10GB_CU1M);
    case 0x81:
      /*printf("SFF INFO - %s() return SFF_SFP_H10GB_CU3M.\n",__FUNCTION__);*/
      return(SFF_SFP_H10GB_CU3M);
    case 0x82:
      /*printf("SFF INFO - %s() return SFF_SFP_H10GB_CU5M.\n",__FUNCTION__);*/
      return(SFF_SFP_H10GB_CU5M);
    case 0x83:
      /*printf("SFF INFO - %s() return SFF_SFP_H10GB_CU7M.\n",__FUNCTION__);*/
      return(SFF_SFP_H10GB_CU7M);
    case 0x84:
      /*printf("SFF INFO - %s() return SFF_SFP_10G_CX1_1S.\n",__FUNCTION__);*/
      return(SFF_SFP_10G_CX1_1S);
    case 0x85:
      /*printf("SFF INFO - %s() return SFF_SFP_10G_LBX1.\n",__FUNCTION__);*/
      return(SFF_SFP_10G_LBX1);
    case 0x86: 
      /*printf("SFF INFO - %s() return SFF_SFP_10GB_USR.\n",__FUNCTION__);*/
      return(SFF_SFP_10GB_USR);
    case 0x87: 
      /*printf("SFF INFO - %s() return SFF_SFP_10GB_LRM_SM.\n",__FUNCTION__);*/
      return(SFF_SFP_10GB_LRM_SM);
    case 0x88: 
      /*printf("SFF INFO - %s() return SFF_SFP_10GB_ELPBK.\n",__FUNCTION__);*/
      return(SFF_SFP_10GB_ELPBK);
    case 0x89: 
      /*printf("SFF INFO - %s() return SFF_SFP_10GB_ELPBK_CR.\n",__FUNCTION__);*/
      return(SFF_SFP_10GB_ELPBK_CR);
    case 0x8A: 
      /*printf("SFF INFO - %s() return SFF_SFP_10GB_ELPBK_BER.\n",__FUNCTION__);*/
      return(SFF_SFP_10GB_ELPBK_BER);
    default: 
      /*printf("SFF INFO - %s() return SFF_MOD_UNDEFINED.\n",__FUNCTION__);*/
      return SFF_MOD_UNDEFINED;
    }
    /* anthing else */
    /*printf("SFF INFO - %s() return SFP_MOD_UNDEFINED ERR.\n",__FUNCTION__);*/
    return SFF_MOD_UNDEFINED;
}

/*
  COPIED FROM CAT3K SOURCE FOR DEBUGGING
*/


typedef struct {
    uint32_t eid;
    char   *s;
} vendorData_t;

vendorData_t sfp1GVendorStr[] = {
    {EID_CWDM_1470, "CWDM-1470"},
    {EID_CWDM_1490, "CWDM-1490"},
    {EID_CWDM_1510, "CWDM-1510"},
    {EID_CWDM_1530, "CWDM-1530"},
    {EID_CWDM_1550, "CWDM-1550"},
    {EID_CWDM_1570, "CWDM-1570"},
    {EID_CWDM_1590, "CWDM-1590"},
    {EID_CWDM_1610, "CWDM-1610"},
    {EID_DWDM_61D41, "DWDM-61.41"},
    {EID_DWDM_60D61, "DWDM-60.61"},
    {EID_DWDM_59D79, "DWDM-59.79"},
    {EID_DWDM_58D98, "DWDM-58.98"},
    {EID_DWDM_58D17, "DWDM-58.17"},
    {EID_DWDM_57D36, "DWDM-57.36"},
    {EID_DWDM_56D55, "DWDM-56.55"},
    {EID_DWDM_55D75, "DWDM-55.75"},
    {EID_DWDM_54D94, "DWDM-54.94"},
    {EID_DWDM_54D13, "DWDM-54.13"},
    {EID_DWDM_53D32, "DWDM-53.32"},
    {EID_DWDM_52D52, "DWDM-52.52"},
    {EID_DWDM_51D72, "DWDM-51.72"},
    {EID_DWDM_50D92, "DWDM-50.92"},
    {EID_DWDM_50D12, "DWDM-50.12"},
    {EID_DWDM_49D31, "DWDM-49.31"},
    {EID_DWDM_48D51, "DWDM-48.51"},
    {EID_DWDM_47D72, "DWDM-47.72"},
    {EID_DWDM_46D92, "DWDM-46.92"},
    {EID_DWDM_46D12, "DWDM-46.12"},
    {EID_DWDM_45D32, "DWDM-45.32"},
    {EID_DWDM_44D53, "DWDM-44.53"},
    {EID_DWDM_43D73, "DWDM-43.73"},
    {EID_DWDM_42D94, "DWDM-42.94"},
    {EID_DWDM_42D14, "DWDM-42.14"},
    {EID_DWDM_41D34, "DWDM-41.34"},
    {EID_DWDM_40D56, "DWDM-40.56"},
    {EID_DWDM_39D77, "DWDM-39.77"},
    {EID_DWDM_38D98, "DWDM-38.98"},
    {EID_DWDM_38D19, "DWDM-38.19"},
    {EID_DWDM_37D39, "DWDM-37.39"},
    {EID_DWDM_36D61, "DWDM-36.61"},
    {EID_DWDM_35D82, "DWDM-35.82"},
    {EID_DWDM_35D04, "DWDM-35.04"},
    {EID_DWDM_34D25, "DWDM-34.25"},
    {EID_DWDM_33D46, "DWDM-33.46"},
    {EID_DWDM_32D68, "DWDM-32.68"},
    {EID_DWDM_31D90, "DWDM-31.90"},
    {EID_DWDM_31D12, "DWDM-31.12"},
    {EID_DWDM_30D33, "DWDM-30.33"},
    {EID_100FX_GE, "SFP-100FX-GE"},
    {EID_100FX_FE, "SFP-100FX-FE"},
    {EID_100LX_FE, "SFP-100LX-FE"},
    {EID_1000BX_10_U, "SFP-1000BX-10-U"},
    {EID_1000BX_10_D, "SFP-1000BX-10-D"},
    {EID_100BX_10_U, "SFP-100BX-10-U"},
    {EID_100BX_10_D, "SFP-100BX-10-D"},
    {EID_BX40_U_I, "SFP-BX40-U-I"},
    {EID_BX40_UD_I, "SFP-BX40-UD-I"},
    {EID_BX80_U_I, "SFP-BX80-U-I"},
    {EID_BX80_D_I, "SFP-BX80-D-I"},
    {EID_BX40_UDA_I, "SFP-BX40-UDA-I"},
    {EID_SFP_PATCH, "SFP-PATCH"},
    {EID_100EX_FE, "SFP-100EX-FE"},
    {EID_100ZX_FE, "SFP-100ZX-FE"},
    {EID_1000EX, "SFP-1000EX"},
    {0, NULL}
};

vendorData_t sfp10GVendorStr[] = {
    {EID_SFP_H10GB_CU1M, "SFP-H10GB-CU1M"},
    {EID_SFP_H10GB_CU3M, "SFP-H10GB-CU3M"},
    {EID_SFP_H10GB_CU5M, "SFP-H10GB-CU5M"},
    {EID_SFP_H10GB_CU7M, "SFP-H10GB-CU7M"},
    {EID_SFP_10G_CX1_1S, "SFP-10G-CX1-1S"},
    {EID_SFP_10G_LBX1, "SFP-10G-LBX1"},
    {EID_SFP_10G_USR, "SFP-10G-USR"},
    {EID_SFP_10G_LRM_SM, "SFP-10G-LRM-SM"},
    {EID_SFP_10G_ELPBK, "SFP-10G-ELPBK"},
    {EID_SFP_10G_ELPBK_CR, "SFP-10G-ELPBK-CR"},
    {EID_SFP_10G_ELPBK_BER, "SFP-10G-ELPBK-BER"},
    {EID_DWDM10G_61D41, "DWDM10G-61.41"},
    {EID_DWDM10G_60D61, "DWDM10G-60.61"},
    {EID_DWDM10G_59D79, "DWDM10G-59.79"},
    {EID_DWDM10G_58D98, "DWDM10G-58.98"},
    {EID_DWDM10G_58D17, "DWDM10G-58.17"},
    {EID_DWDM10G_57D36, "DWDM10G-57.36"},
    {EID_DWDM10G_56D55, "DWDM10G-56.55"},
    {EID_DWDM10G_55D75, "DWDM10G-55.75"},
    {EID_DWDM10G_54D94, "DWDM10G-54.94"},
    {EID_DWDM10G_54D13, "DWDM10G-54.13"},
    {EID_DWDM10G_53D33, "DWDM10G-53.33"},
    {EID_DWDM10G_52D52, "DWDM10G-52.52"},
    {EID_DWDM10G_51D72, "DWDM10G-51.72"},
    {EID_DWDM10G_50D92, "DWDM10G-50.92"},
    {EID_DWDM10G_50D12, "DWDM10G-50.12"},
    {EID_DWDM10G_49D32, "DWDM10G-49.32"},
    {EID_DWDM10G_48D51, "DWDM10G-48.51"},
    {EID_DWDM10G_47D72, "DWDM10G-47.72"},
    {EID_DWDM10G_46D92, "DWDM10G-46.92"},
    {EID_DWDM10G_46D12, "DWDM10G-46.12"},
    {EID_DWDM10G_45D32, "DWDM10G-45.32"},
    {EID_DWDM10G_44D53, "DWDM10G-44.53"},
    {EID_DWDM10G_43D73, "DWDM10G-43.73"},
    {EID_DWDM10G_42D94, "DWDM10G-42.94"},
    {EID_DWDM10G_42D14, "DWDM10G-42.14"},
    {EID_DWDM10G_41D35, "DWDM10G-41.35"},
    {EID_DWDM10G_40D56, "DWDM10G-40.56"},
    {EID_DWDM10G_39D77, "DWDM10G-39.77"},
    {EID_DWDM10G_38D98, "DWDM10G-38.98"},
    {EID_DWDM10G_38D19, "DWDM10G-38.19"},
    {EID_DWDM10G_37D40, "DWDM10G-37.40"},
    {EID_DWDM10G_36D61, "DWDM10G-36.61"},
    {EID_DWDM10G_35D82, "DWDM10G-35.82"},
    {EID_DWDM10G_35D04, "DWDM10G-35.04"},
    {EID_DWDM10G_34D25, "DWDM10G-34.25"},
    {EID_DWDM10G_33D47, "DWDM10G-33.47"},
    {EID_DWDM10G_32D68, "DWDM10G-32.68"},
    {EID_DWDM10G_31D90, "DWDM10G-31.90"},
    {EID_DWDM10G_31D12, "DWDM10G-31.12"},
    {EID_DWDM10G_30D33, "DWDM10G-30.33"},
    {EID_SFP10G_BXD, "SFP_10G-BXD"},
    {EID_SFP10G_BXU, "SFP_10G-BXU"},
    {EID_SFP10G_BX40D, "SFP_10G-BX40D"},
    {EID_SFP10G_BX40U, "SFP_10G-BX40U"},
    {EID_CWDM10G_1590, "CWDM10G-1590"},
    {EID_SFP10GB_ZR, "EID-SFP10GB-ZR"},
    {0, NULL}
};

#if 0
void sff_eeprom_show(sff_trans_map_t *sff_trans)
{
  sfp_msa_eeprom_blk_t *pSfpEeprom = (sfp_msa_eeprom_blk_t *)sff_trans->sff_eeprom;
  uint8_t              domPresent = 0;
  uint8_t              sfp1GPresent = 0;
  uint8_t              is_cisco_compliant = 0;
  char                 str[20] = {0};
  vendorData_t         *vendorStrPtr = NULL;
  uint32_t             size, i, extId;

  printf("  %-*s: 0x%x ", 30, "Identifier", pSfpEeprom->identifier[0]);
    
  switch (pSfpEeprom->identifier[0]) {
  case IDENT_UNKNOWN:
    printf("(unknown)");
    break;
  case IDENT_GBIC:
    printf("(GBIC)");
    break;
  case IDENT_SOLDERED:
    printf("(Module soldered to motherboard)");
    break;
  case IDENT_SFP:
    printf("(SFP)");
    break;
  case IDENT_DWDM_SFP:
    printf("(DWDM SFP)");
    break;
  }
  printf("\n");

  if (pSfpEeprom->transceiverCode[0] & SFPPLUS_10G_MASK) {
    sfp1GPresent = 0;
  } else if ((pSfpEeprom->connectorCode[0] == 0x21) &&
             (pSfpEeprom->transceiverCode[5] == 0x4)) {
    /* Check for CX1 SFP+ module (EDCS-658381, section 7.3) */
    sfp1GPresent = 0;
  } else if ((pSfpEeprom->vendorSpecificData[0] >= 0x80) && 
               (pSfpEeprom->vendorSpecificData[0] != 0xff) &&
               (pSfpEeprom->vendorSpecificData[1] != 0xff)) {
        /* For non-standard SFP/SFP+, check bytes 96 & 97 for extended ID,
           (see EDCS-553508, section 11) */
        sfp1GPresent = 0;
    } else {
        sfp1GPresent = 1;
  }
  printf("SFF INFO - sfp1GPresent = %d\n",sfp1GPresent);
  vendorStrPtr = sfp1GPresent ? sfp1GVendorStr : sfp10GVendorStr;

#if 0
  /* TBDJCB FINISH LATER Read A2 EEPROM */
  if (pSfpEeprom.diagMonitorType & SFP_EEPROM_A2_PRESENT) {}
#endif

  if (sfp1GPresent) {
    printf("  %-*s: 0x%x", 30, "Gig Ethernet Compliance Code",
           pSfpEeprom->transceiverCode[3]);
    switch(pSfpEeprom->transceiverCode[3]) {
        case CC_NST:
            printf(" (Non Standard)\n");
            break;
        case CC_SX:
            printf(" (1000BASE-SX)\n");
            break;
        case CC_LX:
            printf(" (1000BASE-");
            if (pSfpEeprom->linkLenSupported2[0] == 0xff) {
                printf("ZX)\n");
            } else {
                printf("LX)\n");
            }
            break;
        case CC_CX:
            printf(" (1000BASE-CX)\n");
            break;
        case CC_T:
            printf(" (1000BASE-T)\n");
            break;
        default:
            printf("  -- unrecognized compliance code.\n");
            break;
        }
    } else {
        printf("  %-*s: 0x%x", 30, "Transceiver",
               pSfpEeprom->transceiverCode[0]);
        switch (pSfpEeprom->transceiverCode[0]) {
        case SFPPLUS_10G_SR:
            printf(" (10G SR)\n");
            break;
        case SFPPLUS_10G_LR:
            printf(" (10G LR)\n");
            break;
        case SFPPLUS_10G_LRM:
            printf(" (10G LRM)\n");
            break;
        case SFPPLUS_10G_ER:
            printf(" (10G ER)\n");
            break;
        case SFPPLUS_CX1_CU_ACTIVE:
            printf(" (Copper Active)\n");
            break;
        case SFPPLUS_CX1_CU_PASSIVE:
            printf(" (Copper Passive)\n");
            break;
        default:
            if ((!pSfpEeprom->transceiverCode[0]) && (!pSfpEeprom->transceiverCode[3])) {
                printf(" (Non Standard)\n");
            } else {
                printf("  -- unrecognized compliance code.\n");
            }
            break;
        }
  }

  strncpy(str, pSfpEeprom->vendorName, 16);
  printf("  %-*s: %s\n", 30, "Vendor Name", str);
  printf("  %-*s: 0x%02x 0x%02x 0x%02x \n", 30, "Vendor OUI", 
         pSfpEeprom->vendorOUI[0],
         pSfpEeprom->vendorOUI[1], pSfpEeprom->vendorOUI[2]);
  strncpy(str, pSfpEeprom->vendorPN, 16);
  printf("  %-*s: %s\n", 30, "Vendor PN", str);
  strncpy(str, pSfpEeprom->vendorREV, 4);
  printf("  %-*s: %s\n", 30, "Vendor Rev", str);
  strncpy(str, pSfpEeprom->vendorSerialNum, 16);
  printf("  %-*s: %s\n", 30, "Vendor SN", str);
  
  extId = pSfpEeprom->vendorSpecificData[0];
  printf("  %-*s: 0x%x", 30, "Extended ID", extId);
  for (i = 0; vendorStrPtr[i].eid != 0; i++) {
    if (vendorStrPtr[i].eid == extId) {
      printf(" (%s)", vendorStrPtr[i].s);
      break;
    }
  }
  printf("\n");
  
  printf("  %-*s: 0x%x\n", 30, "Cisco Supplied Vendor ID",
         pSfpEeprom->vendorSpecificData[2]);
  
  printf("\nEEPROM (%d bytes of raw data in hex)\n",
         sizeof(sfp_msa_eeprom_blk_t));
  printf("=====================================\n");
  size = sizeof(sfp_msa_eeprom_blk_t);
  for (i = 0; i < size; i++) {
    if ((i % 16) == 0) {
      printf("\n0x%04x : ", i);
    }
    printf("%02x ", ((uint8_t *)pSfpEeprom)[i]);
  }
  printf("\n");

  /* TBDJCB - display EEPROM A2 contents later */
  if (!sfp1GPresent) {
    if (sff_trans_eeprom_checksum_base_ids_validate(sff_trans) != 0) {
      printf("*** SFP+ EEPROM CHECKSUM FAILURE! ***\n");
    }
  }
}
#endif
