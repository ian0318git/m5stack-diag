/* $Id: key_tlv_parser.h,v 1.2 2019/07/11 12:34:40 alicehua Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/include/key_tlv_parser.h,v $
 *------------------------------------------------------------------
 * key_tlv_parser.h -- header for Aikido key TLV parser
 *
 * February 2019, Chandana Prakash
 *
 * Copyright (c) 2019 by Cisco Systems, Inc.
 * All rights reserved.
 *-----------------------------------------------------------------
 */

#ifndef __KEY_TLV_PARSER_H__
#define __KEY_TLV_PARSER_H__

#include "stdio.h"
#include "string.h"
#include "stdint.h"
#include "time.h"

/*
 * Key Record Magic - do not alter them
 */
#define KEY_START_MAGIC                 0xab1234cd
#define KEY_END_MAGIC                   0xbeefcafe
#define KEY_START_MAGIC_SIZE            4
#define KEY_END_MAGIC_SIZE              4
#define KEY_HDR_AIKIDO_TAG              0xbe
#define KEY_MOD_SIZE                    256
#define KEY_INFO_SZ_OVERHEAD            (2 * sizeof(tlventry))
#define MAX_KEY_PROD_NAME_LEN           75  /* Max product name length */
#define KEY_RECORD_TYPE_PAD             0xed
#define KEY_PAD_VALUE                   0xed
#define KEY_HDR_TAG_LENGTH              1
#define KEY_INFO_LENGTH                 2
#define KEY_ID_LENGTH                   8
#define KEY_METADATA_LENGTH             (KEY_HDR_TAG_LENGTH + KEY_INFO_LENGTH + KEY_ID_LENGTH \
                                        + KEY_START_MAGIC_SIZE + KEY_END_MAGIC_SIZE)

/*
 * Crypto Algorithms
 */
#define RSA_ALGO    ( 0x01 )
#define ECDSA_ALGO  ( 0x02 )
#define LDWM_ALGO   ( 0x03 )


/*
 * Error codes
 */
#define ERR_OK                          ( 0x00 )
#define ERR_INVALID_INPUT               ( 0xE0 )
#define ERR_INVALID_HDR_TAG             ( 0xE1 )
#define ERR_KEY_MAGIC_MARKED_REVOKE     ( 0xE2 )
#define ERR_KEY_MAGIC_INVALID           ( 0xE3 )
#define ERR_KEY_INVALID_LEN             ( 0xE4 )
#define ERR_KEY_INVALID_PAD             ( 0xE5 )
#define ERR_INVALID_KEY_ALGO            ( 0xE6 )


/*Key Record Types*/
#define TYPE_VERSION                    ( 0x01 )
#define TYPE_KEY_USAGE                  ( 0x02 )
#define TYPE_START_TIMESTAMP            ( 0x03 )
#define TYPE_END_TIMESTAMP              ( 0x04 )
#define TYPE_KEY_FORMAT                 ( 0x05 )
#define TYPE_KEY_PAYLOAD_LEN            ( 0x06 )
#define TYPE_KEY_TYPE                   ( 0x11 )
#define TYPE_KEY_ALGO                   ( 0x12 )
#define TYPE_KEY_INFO_LEN               ( 0x13 )
#define TYPE_MODULUS                    ( 0x14 )
#define TYPE_PUB_EXP                    ( 0x15 )
#define TYPE_KEY_VERSION                ( 0x16 )
#define TYPE_PROD_NAME                  ( 0x17 )


#pragma pack(1)
typedef struct key_record_ {
    uint16_t    version;
    uint16_t    key_usage;
    uint64_t    start_timestamp;
    uint64_t    end_timestamp;
    uint8_t     key_format;
    uint16_t    key_payload_len;
    uint8_t     key_type;
    uint8_t     key_algo;
    uint16_t    key_info_len;
    uint16_t    mod_size;
    uint8_t     modulus[KEY_MOD_SIZE];
    uint16_t    pub_exp_size;
    uint32_t    pub_exp;
    uint8_t     key_version;
    uint8_t     prod_name[MAX_KEY_PROD_NAME_LEN + 1];
    uint8_t     key_hdr_type;
    uint64_t    key_id;
} key_record;
#pragma pack()

#pragma pack(1)
typedef struct tlventry_ {
    uint8_t  type;
    uint16_t length;
    uint8_t  data[0];
} tlventry;
#pragma pack()


/*
 * Key Record Header format
 */
#pragma pack(1)
typedef struct key_record_hdr_tlv_ {
    tlventry  tlv_hdr;
    uint8_t             magic[4];
    uint8_t             keyid[8];
} key_record_hdr_tlv;

#pragma pack()

extern void dump_key_record_info(key_record *key_rec);
extern int parse_key_record(uint8_t **key, 
                            key_record *key_rec);

#endif /* __KEY_TLV_PARSER_H__ */

/*---------------------------------------------------------------
$Log: key_tlv_parser.h,v $
Revision 1.2  2019/07/11 12:34:40  alicehua
Collapse Nutella codes into main trunk

$Endlog$
*/
