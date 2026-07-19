/* $Id: sff_trans.h,v 1.2 2018/05/18 09:24:58 alpeng Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/overlord/neptune/cavium/sff_trans.h,v $
 * ------------------------------------------------------------------
 * Copyright (c) 2018 by cisco Systems, Inc.
 * All Rights Reserved
 *
 * This code deals with the 2-wire memory map defined for GBIC, SFP MSA, and
 * SFP+ Transceivers. See SFF specs.
 * EDCS document SFPplus_Module_Software_programmer_Guide.doc
 *
 * Author: Mecca Ho
 */

#ifndef _SFF_TRANS_H_
#define _SFP_TRANS_H_

#include <stdint.h>


#define SFF_EEPROM_SIZE	256

typedef struct sff_transceiver_map_t_ {
    unsigned char sff_eeprom[SFF_EEPROM_SIZE];
    int sff_rcksum; /* Checksum read from the EEPROM */
    int sff_ccksum; /* Calculated checksum */
} sff_trans_map_t;

#define SFF_EEPROM(sff, offset)  	((sff)->sff_eeprom[(offset)])

/* Module type related */
#define SFF_B0_SFP_PMD_ID(sff)		((sff)->sff_eeprom[0])
#define SFF_SFP_PMD_ID(sff)      	((sff)->sff_eeprom[0] == 0x03)
#define SFF_B18_CABLE_LENGTH(sff)       ((sff)->sff_eeprom[18])
#define SFF_B96_EXT_TRANS_CODE(sff) 	((sff)->sff_eeprom[96])
#define SFF_B3_TRANS_CODE_10G(sff)	((sff)->sff_eeprom[3])
#define SFF_B6_TRANS_CODE_1G(sff)	((sff)->sff_eeprom[6])
#define SFF_B8_SFP_PLUS_CABLE(sff)	((sff)->sff_eeprom[8])
#define SFF_B60_CABLE_SPEC(sff)		((sff)->sff_eeprom[60])

/* DOM support related */
#define SFF_EEPROM_DOM_TYPE 		92
#define SFF_B92_DOM_TYPE(sff)		((sff)->sff_eeprom[92])
#define SFF_DOM_IMPLIMENTED(code)	((code) & 0x40)

/* Vendor and Cisco product info */
#define SFF_VENDOR_NAME(sff)		((sff)->sff_eeprom[20]) /* 16 bytes */
#define SFF_VENDOR_NAME_SIZE		16
#define SFF_VENDOR_OUI(sff)		((sff)->sff_eeprom[37])	/* 3 bytes */
#define SFF_VENDOR_OUI_SIZE		3
#define SFF_VENDOR_PN(sff)		((sff)->sff_eeprom[40])	/* 16 bytes */
#define SFF_VENDOR_PN_SIZE		16
#define SFF_VENDOR_REV(sff)		((sff)->sff_eeprom[56])	/* 4 bytes */
#define SFF_VENDOR_REV_SIZE		4
#define SFF_VENDOR_SN(sff)		((sff)->sff_eeprom[68])	/* 16 bytes */
#define SFF_VENDOR_SN_SIZE		16
#define SFF_VENDOR_DATE(sff)		((sff)->sff_eeprom[84])	/* 8 bytes */
#define SFF_VENDOR_DATE_SIZE		8
#define SFF_CISCO_PN(sff)		((sff)->sff_eeprom[138]) /* 10 bytes */
#define SFF_CISCO_PN_SIZE		10
#define SFF_CISCO_VID(sff)		((sff)->sff_eeprom[148]) /* 4 bytes */
#define SFF_CISCO_VID_SIZE		4
#define SFF_CISCO_PID(sff)		((sff)->sff_eeprom[192]) /* 20 bytes */
#define SFF_CISCO_PID_SIZE		20
#define SFF_CISCO_CX1_REV(sff)		((sff)->sff_eeprom[212]) /* 2 bytes */
#define SFF_CISCO_CX1_REV_SIZE		2
#define SFF_LBX1_ATTEN(sff)		((sff)->sff_eeprom[244]) /* 1 bytes */

/* check code related */
#define SFF_B63_CC_BASE(sff)		((sff)->sff_eeprom[63])
#define SFF_B95_CC_EXT(sff)  		((sff)->sff_eeprom[95])
#define SFF_CRC_B0(sff)			((sff)->sff_eeprom[124])
#define SFF_CRC_B1(sff)			((sff)->sff_eeprom[125])
#define SFF_CRC_B2(sff)			((sff)->sff_eeprom[126])
#define SFF_CRC_B3(sff)			((sff)->sff_eeprom[127])
#define SFF_B159_CC_VENDOR_1(sff)	((sff)->sff_eeprom[159])
#define SFF_B223_CC_VENDOR_2(sff)	((sff)->sff_eeprom[223])
#define SFF_B245_CC_VENDOR_3(sff)	((sff)->sff_eeprom[245])

/* Data Field Decode */
/* 10G group */
#define SFF_10G_ER(code)         (code & (1<<7))
#define SFF_10G_LRM(code)        (code & (1<<6))
#define SFF_10G_LR(code)         (code & (1<<5))
#define SFF_10G_SR(code)         (code & (1<<4))
#define SFF_10G_ZR(code)	 (code & 0x9F)
#define SFF_SFP_10G(code)        (code & 0xF0)

/* 1G group */
#define SFF_1000BASE_T(code)     (code & (1<<3))
#define SFF_1000BASE_CX(code)    (code & (1<<2))
#define SFF_1000BASE_LX(code)    (code & (1<<1))
#define SFF_1000BASE_SX(code)    (code & (1<<0))
#define SFF_SFP_1G(code)         (code & 0xFF)

/* Cable */
#define SFF_ACTIVE_CABLE(code)   (code & (1<<3))
#define SFF_PASSIVE_CABLE(code)  (code & (1<<2))
#define SFF_8431_LIMIT(code)     (code & (1<<2))
#define SFF_8431_AE(code)        (code & (1<<0))

/* Module Internal IDs. I don't think the numbers mean anything, but
 * these types were taken from bcm_esw_common.h code and included here
 * to make the sff driver more complete.
 */
typedef enum {
  SFF_NOT_SFP =        0x0000,
  SFF_MOD_UNDEFINED =  0x0000,
  SFF_SFP_10G_ER =     0x0081,
  SFF_SFP_10G_LRM =    0x0082,
  SFF_SFP_10G_LR =     0x0083,
  SFF_SFP_10G_SR =     0x0084,
  SFF_SFP_10G_ZR =     0x0085,
  SFF_SFP_10G_PAS_CX1_1 = 0x0011,
  SFF_SFP_10G_PAS_CX1_2 = 0x0012,
  SFF_SFP_10G_PAS_CX1_3 = 0x0013,
  SFF_SFP_10G_PAS_CX1_4 = 0x0014,
  SFF_SFP_10G_PAS_CX1_5 = 0x0015,
  SFF_SFP_10G_PAS_CX1_6 = 0x0016,
  SFF_SFP_10G_PAS_CX1_7 = 0x0017,
  SFF_SFP_10G_PAS_CX1_1S = 0x0018,
  SFF_SFP_10G_PAS_CX1 = 0x0019,

  SFF_SFP_10G_ACT_CX1_1 = 0x0021,
  SFF_SFP_10G_ACT_CX1_3 = 0x0022,
  SFF_SFP_10G_ACT_CX1_5 = 0x0023,
  SFF_SFP_10G_ACT_CX1_7 = 0x0024,
  SFF_SFP_10G_ACT_CX1_1S = 0x0025,
  SFF_SFP_10G_ACT_CX1 = 0x0026,
  SFF_SFP_10G_ACT_LIMIT_CX1 = 0x0027,

  SFF_SFP_H10GB_CU1M = 0x0041,
  SFF_SFP_H10GB_CU3M = 0x0042,
  SFF_SFP_H10GB_CU5M = 0x0043,
  SFF_SFP_H10GB_CU7M = 0x0044,
  SFF_SFP_10G_CX1_1S = 0x0045,
  SFF_SFP_10G_LBX1	 = 0x0046,
  SFF_SFP_10GB_USR = 0x0047,
  SFF_SFP_10GB_LRM_SM = 0x0048,
  SFF_SFP_10GB_ELPBK = 0x0049,
  SFF_SFP_10GB_ELPBK_CR	 = 0x004A,
  SFF_SFP_10GB_ELPBK_BER = 0x004B,

  SFF_SFP_1G_NS	 = 0x0101,
  SFF_SFP_1G_T	 = 0x0102,
  SFF_SFP_1G_CX	 = 0x0103,
  SFF_SFP_1G_LX	 = 0x0104,
  SFF_SFP_1G_SX	 = 0x0105,
  SFF_SFP_100BASE = 0x0106,
} sff_sfp_module_id_t;


/* SFF API Function Prototypes */
int sff_trans_eeprom_checksum_vend1_fields_validate(sff_trans_map_t *sff_trans);
int sff_trans_eeprom_checksum_vend2_fields_validate(sff_trans_map_t *sff_trans);
int sff_trans_eeprom_checksum_vend3_fields_validate(sff_trans_map_t *sff_trans);
int sff_trans_eeprom_checksum_vend_ids_validate(sff_trans_map_t *sff_trans);
int sff_trans_eeprom_checksum_ext_ids_validate(sff_trans_map_t *sff_trans);
int sff_trans_eeprom_checksum_base_ids_validate(sff_trans_map_t *sff_trans);
/* trans_check return 0 for FALSE, 1 for TRUE */
int sff_trans_check_cisco_pn(sff_trans_map_t *sff_trans);
int sff_sfp_eeprom_cisco_vendor_checksum_validate(sff_trans_map_t *sff_trans);
/* New functions added for Kilburn Park */
sff_sfp_module_id_t sff_get_sfp_module_id(sff_trans_map_t *sff_trans);


/*
 * Below Code is debug show support code copied from cat3k
 */
//void sff_eeprom_show(sff_trans_map_t *sff_trans);

typedef struct sfp_msa_eeprom_blk_type {
    /* Base ID Fields */
    uint8_t    identifier[1];
    uint8_t    extIdentifier[1];
    uint8_t    connectorCode[1];
    uint8_t    transceiverCode[8];
    uint8_t    encoding[1];
    uint8_t    nominalBitRate[1];
    uint8_t    reserved1[1];
    uint8_t    linkLenSupported1[1];
    uint8_t    linkLenSupported2[1];
    uint8_t    linkLenSupported3[1];
    uint8_t    linkLenSupported4[1];
    uint8_t    linkLenSupported5[1];
    uint8_t    reserved2[1];
    uint8_t    vendorName[16];
    uint8_t    reserved3[1];
    uint8_t    vendorOUI[3];
    uint8_t    vendorPN[16];
    uint8_t    vendorREV[4];
    uint8_t    reserved4[3];
    uint8_t    checkCodeBaseID[1];
    /* Extended ID Fields */
    uint8_t    sfpSignalOptions[2];
    uint8_t    bitRateMax[1];
    uint8_t    bitRateMin[1];
    uint8_t    vendorSerialNum[16];
    uint8_t    vendorDateCode[8];
    uint8_t    diagMonitorType;
    uint8_t    reserved5[2];
    uint8_t    checkCodeExtendedID[1];
    /* Vendor Specific ID Fields */
    uint8_t    vendorSpecificData[32];
    uint8_t    sff8079Reserved[128];
} sfp_msa_eeprom_blk_t;

#endif /* _SFF_TRANS_H_ */
/* Idendifier */
#define IDENT_UNKNOWN         0
#define IDENT_GBIC            1
#define IDENT_SOLDERED        2
#define IDENT_SFP             3
#define IDENT_CU_SFP          0x8
#define IDENT_DWDM_SFP        0xB

/* Compliance Code */
#define CC_NST                0
#define CC_SX                 (1 << 0)
#define CC_LX                 (1 << 1)
#define CC_CX                 (1 << 2)
#define CC_T                  (1 << 3)
#define CC_MSA                0

/* Extended Id (EDCS# 275976) */
#define EID_CWDM_1470         0x1    /* nonstandard 1000baseX module */
#define EID_CWDM_1490         0x2    /* nonstandard 1000baseX module */
#define EID_CWDM_1510         0x3    /* nonstandard 1000baseX module */
#define EID_CWDM_1530         0x4    /* nonstandard 1000baseX module */
#define EID_CWDM_1550         0x5    /* nonstandard 1000baseX module */
#define EID_CWDM_1570         0x6    /* nonstandard 1000baseX module */
#define EID_CWDM_1590         0x7    /* nonstandard 1000baseX module */
#define EID_CWDM_1610         0x8    /* nonstandard 1000baseX module */
#define EID_DWDM_61D41        0x34   /* nonstandard 1000baseX module */
#define EID_DWDM_60D61        0x9    /* nonstandard 1000baseX module */
#define EID_DWDM_59D79        0xA    /* nonstandard 1000baseX module */
#define EID_DWDM_58D98        0xB    /* nonstandard 1000baseX module */
#define EID_DWDM_58D17        0xC    /* nonstandard 1000baseX module */
#define EID_DWDM_57D36        0x35   /* nonstandard 1000baseX module */
#define EID_DWDM_56D55        0xD    /* nonstandard 1000baseX module */
#define EID_DWDM_55D75        0xE    /* nonstandard 1000baseX module */
#define EID_DWDM_54D94        0xF    /* nonstandard 1000baseX module */
#define EID_DWDM_54D13        0x10   /* nonstandard 1000baseX module */
#define EID_DWDM_53D32        0x36   /* nonstandard 1000baseX module */
#define EID_DWDM_52D52        0x11   /* nonstandard 1000baseX module */
#define EID_DWDM_51D72        0x12   /* nonstandard 1000baseX module */
#define EID_DWDM_50D92        0x13   /* nonstandard 1000baseX module */
#define EID_DWDM_50D12        0x14   /* nonstandard 1000baseX module */
#define EID_DWDM_49D31        0x37   /* nonstandard 1000baseX module */
#define EID_DWDM_48D51        0x15   /* nonstandard 1000baseX module */
#define EID_DWDM_47D72        0x16   /* nonstandard 1000baseX module */
#define EID_DWDM_46D92        0x17   /* nonstandard 1000baseX module */
#define EID_DWDM_46D12        0x18   /* nonstandard 1000baseX module */
#define EID_DWDM_45D32        0x38   /* nonstandard 1000baseX module */
#define EID_DWDM_44D53        0x19   /* nonstandard 1000baseX module */
#define EID_DWDM_43D73        0x1A   /* nonstandard 1000baseX module */
#define EID_DWDM_42D94        0x1B   /* nonstandard 1000baseX module */
#define EID_DWDM_42D14        0x1C   /* nonstandard 1000baseX module */
#define EID_DWDM_41D34        0x39   /* nonstandard 1000baseX module */
#define EID_DWDM_40D56        0x1D   /* nonstandard 1000baseX module */
#define EID_DWDM_39D77        0x1E   /* nonstandard 1000baseX module */
#define EID_DWDM_38D98        0x1F   /* nonstandard 1000baseX module */
#define EID_DWDM_38D19        0x20   /* nonstandard 1000baseX module */
#define EID_DWDM_37D39        0x40   /* nonstandard 1000baseX module */
#define EID_DWDM_36D61        0x21   /* nonstandard 1000baseX module */
#define EID_DWDM_35D82        0x22   /* nonstandard 1000baseX module */
#define EID_DWDM_35D04        0x23   /* nonstandard 1000baseX module */
#define EID_DWDM_34D25        0x24   /* nonstandard 1000baseX module */
#define EID_DWDM_33D46        0x4A   /* nonstandard 1000baseX module */
#define EID_DWDM_32D68        0x25   /* nonstandard 1000baseX module */
#define EID_DWDM_31D90        0x26   /* nonstandard 1000baseX module */
#define EID_DWDM_31D12        0x27   /* nonstandard 1000baseX module */
#define EID_DWDM_30D33        0x28   /* nonstandard 1000baseX module */
#define EID_XWDM_RX           0x29
#define EID_100FX_GE          0x2A   /* tom sawyer */
#define EID_100FX_FE          0x2B   /* 4B/5B 100FX */
#define EID_100LX_FE          0x2C   /* 4B/5B 100FX */
#define EID_1000BX_10_U       0x2D   /* 6K's 1000 baseX modules  */
#define EID_1000BX_10_D       0x2E   /* 6K's 1000 baseX modules  */
#define EID_100BX_10_U        0x2F   /* 6K's 100FX modules  */
#define EID_100BX_10_D        0x30   /* 6K's 100FX modules  */
#define EID_SFP_PATCH         0x31   /* cisco patch cable   */
#define EID_100EX_FE          0x32   /* 4B/5B 100FX */
#define EID_100ZX_FE          0x33   /* 4B/5B 100FX */
#define EID_1000EX            0x3A   /* GLC-EX-SMD */
#define EID_BX40_U_I          0x50   /* 1000 baseX modules */
#define EID_BX40_UD_I         0x51   /* 1000 baseX modules */
#define EID_BX80_U_I          0x52   /* 1000 baseX modules */
#define EID_BX80_D_I          0x53   /* 1000 baseX modules */
#define EID_BX40_UDA_I        0x54   /* 1000 baseX modules */

/* Extended Id (EDCS# 553508) */
#define EID_SFP_H10GB_CU1M    0x80
#define EID_SFP_H10GB_CU3M    0x81
#define EID_SFP_H10GB_CU5M    0x82
#define EID_SFP_H10GB_CU7M    0x83
#define EID_SFP_10G_CX1_1S    0x84
#define EID_SFP_10G_LBX1      0x85
#define EID_SFP_10G_USR       0x86
#define EID_SFP_10G_LRM_SM    0x87
#define EID_SFP_10G_ELPBK     0x88
#define EID_SFP_10G_ELPBK_CR  0x89
#define EID_SFP_10G_ELPBK_BER 0x8A
#define EID_SFP10GB_ZR        0x9F   /* nonstandard 10GbaseX module */
#define EID_DWDM10G_61D41     0xA0   /* nonstandard 10GbaseX module */
#define EID_DWDM10G_60D61     0xA1   /* nonstandard 10GbaseX module */
#define EID_DWDM10G_59D79     0xA2   /* nonstandard 10GbaseX module */
#define EID_DWDM10G_58D98     0xA3   /* nonstandard 10GbaseX module */
#define EID_DWDM10G_58D17     0xA4   /* nonstandard 10GbaseX module */
#define EID_DWDM10G_57D36     0xA5   /* nonstandard 10GbaseX module */
#define EID_DWDM10G_56D55     0xA6   /* nonstandard 10GbaseX module */
#define EID_DWDM10G_55D75     0xA7   /* nonstandard 10GbaseX module */
#define EID_DWDM10G_54D94     0xA8   /* nonstandard 10GbaseX module */
#define EID_DWDM10G_54D13     0xA9   /* nonstandard 10GbaseX module */
#define EID_DWDM10G_53D33     0xAA   /* nonstandard 10GbaseX module */
#define EID_DWDM10G_52D52     0xAB   /* nonstandard 10GbaseX module */
#define EID_DWDM10G_51D72     0xAC   /* nonstandard 10GbaseX module */
#define EID_DWDM10G_50D92     0xAD   /* nonstandard 10GbaseX module */
#define EID_DWDM10G_50D12     0xAE   /* nonstandard 10GbaseX module */
#define EID_DWDM10G_49D32     0xAF   /* nonstandard 10GbaseX module */
#define EID_DWDM10G_48D51     0xB0   /* nonstandard 10GbaseX module */
#define EID_DWDM10G_47D72     0xB1   /* nonstandard 10GbaseX module */
#define EID_DWDM10G_46D92     0xB2   /* nonstandard 10GbaseX module */
#define EID_DWDM10G_46D12     0xB3   /* nonstandard 10GbaseX module */
#define EID_DWDM10G_45D32     0xB4   /* nonstandard 10GbaseX module */
#define EID_DWDM10G_44D53     0xB5   /* nonstandard 10GbaseX module */
#define EID_DWDM10G_43D73     0xB6   /* nonstandard 10GbaseX module */
#define EID_DWDM10G_42D94     0xB7   /* nonstandard 10GbaseX module */
#define EID_DWDM10G_42D14     0xB8   /* nonstandard 10GbaseX module */
#define EID_DWDM10G_41D35     0xB9   /* nonstandard 10GbaseX module */
#define EID_DWDM10G_40D56     0xBA   /* nonstandard 10GbaseX module */
#define EID_DWDM10G_39D77     0xBB   /* nonstandard 10GbaseX module */
#define EID_DWDM10G_38D98     0xBC   /* nonstandard 10GbaseX module */
#define EID_DWDM10G_38D19     0xBD   /* nonstandard 10GbaseX module */
#define EID_DWDM10G_37D40     0xBE   /* nonstandard 10GbaseX module */
#define EID_DWDM10G_36D61     0xBF   /* nonstandard 10GbaseX module */
#define EID_DWDM10G_35D82     0xC0   /* nonstandard 10GbaseX module */
#define EID_DWDM10G_35D04     0xC1   /* nonstandard 10GbaseX module */
#define EID_DWDM10G_34D25     0xC2   /* nonstandard 10GbaseX module */
#define EID_DWDM10G_33D47     0xC3   /* nonstandard 10GbaseX module */
#define EID_DWDM10G_32D68     0xC4   /* nonstandard 10GbaseX module */
#define EID_DWDM10G_31D90     0xC5   /* nonstandard 10GbaseX module */
#define EID_DWDM10G_31D12     0xC6   /* nonstandard 10GbaseX module */
#define EID_DWDM10G_30D33     0xC7   /* nonstandard 10GbaseX module */
#define EID_SFP10G_BXD        0xC8   /* nonstandard 10GbaseX module */
#define EID_SFP10G_BXU        0xC9   /* nonstandard 10GbaseX module */
#define EID_SFP10G_BX40D      0xCA   /* nonstandard 10GbaseX module */
#define EID_SFP10G_BX40U      0xCB   /* nonstandard 10GbaseX module */
#define EID_CWDM10G_1590      0xD2   /* nonstandard 10GbaseX module */


#define SFP_EEPROM_ADDR       0xA0
#define SFP_XCVRGBCC_FIELD    6
#define SFP_EXTID_FIELD       96
#define SFP_LINKLEN2          15
#define SFP_VENDOR_MFG        "Methode Elec.   "

#define SFP_EEPROM_A2_ADDR    0xA2
#define SFP_EEPROM_A2_PRESENT 0x40
#define SFP_EEPROM_A2_SIZE    256

#define SFP_STATUS_CTRL       110	/* status and control reg */
#define SFP_STATUS_TX_FAULT_BIT	0x4

/***********************/
/*       SFP DOM       */
/***********************/
/* Calibration Options Mask*/
#define SFP_DOM_CALIB_INT_MASK    0x20
#define SFP_DOM_CALIB_EXT_MASK    0x10

/* Byte Offsets for External Calibration Constants */
#define SFP_DOM_CALIB_CONST_RX_PWR_4     56
#define SFP_DOM_CALIB_CONST_RX_PWR_3     60
#define SFP_DOM_CALIB_CONST_RX_PWR_2     64
#define SFP_DOM_CALIB_CONST_RX_PWR_1     68
#define SFP_DOM_CALIB_CONST_RX_PWR_0     72
#define SFP_DOM_CALIB_CONST_TX_I_SLO     76
#define SFP_DOM_CALIB_CONST_TX_I_OFF     78
#define SFP_DOM_CALIB_CONST_TX_PWR_SLO   80
#define SFP_DOM_CALIB_CONST_TX_PWR_OFF   82
#define SFP_DOM_CALIB_CONST_TEMP_SLO     84
#define SFP_DOM_CALIB_CONST_TEMP_OFF     86
#define SFP_DOM_CALIB_CONST_VOL_SLO      88
#define SFP_DOM_CALIB_CONST_VOL_OFF      90

/* Btye Offsets for A/D Values */
#define SFP_DOM_VAL_TEMP            96
#define SFP_DOM_VAL_SUPPLY_VOL      98
#define SFP_DOM_VAL_LASER_BIAS_CUR  100
#define SFP_DOM_VAL_TX_PWR          102
#define SFP_DOM_VAL_RX_PWR          104

/* Byte Offsets for Alarm Flags */
#define SFP_DOM_ALARM_FLAGS_1       112
#define SFP_DOM_ALARM_FLAGS_2       113
#define SFP_DOM_WARNING_FLAGS_1     116
#define SFP_DOM_WARNING_FLAGS_2     117
#define SFP_DOM_ALARM_WARNING_MASK  0xffc0

/* Alarm Masks */
#define SFP_DOM_XCVR_TEMP_HI        0x80
#define SFP_DOM_XCVR_TEMP_LOW       0x40
#define SFP_DOM_SUPPLY_VOL_HI       0x20
#define SFP_DOM_SUPPLY_VOL_LOW      0x10
#define SFP_DOM_LASER_BIAS_CUR_HI   0x08
#define SFP_DOM_LASER_BIAS_CUR_LOW  0x04
#define SFP_DOM_TX_PWR_HI           0x02
#define SFP_DOM_TX_PWR_LOW          0x01
#define SFP_DOM_RX_PWR_HI           0x80
#define SFP_DOM_RX_PWR_LOW          0x40

#define SFPPLUS_CX1_CU_PASSIVE   0x1
#define SFPPLUS_CX1_CU_ACTIVE    0x2
#define SFPPLUS_10G_SR           0x10
#define SFPPLUS_10G_LR           0x20
#define SFPPLUS_10G_LRM          0x40
#define SFPPLUS_10G_ER           0x80

#define SFPPLUS_10G_MASK         (SFPPLUS_10G_SR | SFPPLUS_10G_LR | \
                                  SFPPLUS_10G_LRM | SFPPLUS_10G_ER)

typedef enum  {
    SFP_EMPTY,                /* no sfp present                    */
    SFP_NST_1000X,            /* CC=0 EID = 0x1 thru 0x8           */
    SFP_PATCH_CABLE,          /* CC=0 EID = 0x31 (EID_SFP_PATCH)   */
    SFP_TOMSAWYER,            /* CC=0 EID = 0x2a (EID_100FX_GE)    */
    SFP_100FX,                /* CC=0 EID = 0x2b/2c (EID_100FX_FE) */
    SFP_1000FX_UP,            /* CC=0 EID = 0x2d (EID_1000BX_10_U) */
    SFP_1000FX_DN,            /* CC=0 EID = 0x2e (EID_1000BX_10_D) */
    SFP_100FX_UP,             /* CC=0 EID = 0x2f (EID_100BX_10_U)  */
    SFP_100FX_DN,             /* CC=0 EID = 0x30 (EID_100BX_10_D)  */
    SFP_BX40_U_I,             /* +CC=0 EID = 0x50(EID_BX40_U_I)    */
    SFP_BX40_UD_I,            /* +CC=0 EID = 0x51(EID_BX40_UD_I)   */
    SFP_BX80_U_I,             /* +CC=0 EID = 0x52(EID_BX80_U_I)    */
    SFP_BX80_D_I,             /* +CC=0 EID = 0x53(EID_BX80_D_I)    */
    SFP_BX40_UDA_I,           /* +CC=0 EID = 0x54(EID_BX40_UDA_I)  */
    SFP_1000SX,               /* CC=1                              */
    SFP_1000LX,               /* CC=2 linklensupport2 != 0xff      */
    SFP_1000ZX,               /* CC=2 linklensupport2 == 0xff      */
    SFP_1000EX,               /* CC=0 EID = 0x3A (EID_1000EX)      */
    SFP_SOLDERED_CABLE,       /* CC=4 ID  = 2                      */
    SFP_HUCKFINN,             /* CC=8 EID = 0x0                    */
    /* 10G SFP+s */
    SFP_10G_SR,               /* +CC=10                            */
    SFP_10G_LR,               /* +CC=20                            */
    SFP_10G_LRM,              /* +CC=40                            */
    SFP_10G_ER,               /* +CC=80                            */
    SFP_10G_LRM_SM,           /* +CC=0 EID = 0x87                  */
    SFP_10G_USR,              /* +CC=0 EID = 0x86                  */
    SFP_10G_ACTIVE_CABLE,     /* +CC=0 SFP+Cable=8                 */
    SFP_10G_PASSIVE_CABLE,    /* +CC=0 SFP+Cable=4 EID = 0x80-0x83 */
    SFP_10G_CX1_1S,           /* +CC=0 EID = 0x84                  */
    SFP_10G_LBX1,             /* +CC=0 EID = 0x85                  */
    SFP_10G_ELPBK,            /* +CC=0 EID = 0x88 - 0x89           */
    SFP_10G_ELPBK_BER,        /* +CC=0 EID = 0x8a                  */
    SFP_10GB_ZR,              /* +CC=0 EID = 0x9f                  */        
    SFP_10GB_DWDM,            /* +CC=0 EID = 0xA0                  */ 
    SFP_10GB_BXD,             /* +CC=0 EID = 0xC8                  */ 
    SFP_10GB_BXU,             /* +CC=0 EID = 0xC9                  */ 
    SFP_10GB_BX40D,           /* +CC=0 EID = 0xCA                  */ 
    SFP_10GB_BX40U,           /* +CC=0 EID = 0xCB                  */ 
    SFP_10GB_CWDM,            /* +CC=0 EID = 0xD2                  */ 
    /* QSFP/QSFP+ */
    QSFP_10G_LRM,
    QSFP_10G_LR,
    QSFP_10G_SR,
    QSFP_40G_CR4,
    QSFP_40G_SR4,
    QSFP_40G_LR4,
    QSFP_40G_XLPPI,
    QSFP_40G_4SFP_10G_CU1M,     
    QSFP_40G_4SFP_10G_CU3M,
    QSFP_40G_4SFP_10G_CU5M,
    QSFP_4X10G_AC1M,
    QSFP_4X10G_AC3M,
    QSFP_4X10G_AC5M,
    QSFP_4X10G_AC7M,
    QSFP_4X10G_AC10M,
    QSFP_ELPBK,
    QSFP_H40G_ACU1M,
    QSFP_H40G_ACU3M,
    QSFP_H40G_ACU5M,
    QSFP_H40G_ACU7M,
    QSFP_H40G_ACU10M,
    QSFP_4X10G_LR,
    QSFP_40G_CSR4,
    QSFP_CAZADERO,
    QSFP_40G_4SFP_10G_CU2M,
    QSFP_FET,
    QSFP_40G_SR_BD,
    QSFP_H40G_AOCxM,
    QSFP_4X10G_AOCxM,
    QSFP_40G_ER4,
    QSFP_40G_MOLEX_LPBK,
    SFP_UNKNOWN               /* all others                        */
} sfpSpecificType_t;
