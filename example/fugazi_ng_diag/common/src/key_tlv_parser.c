/* $Id: key_tlv_parser.c,v 1.2 2019/07/11 12:34:40 alicehua Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/key_tlv_parser.c,v $
 *------------------------------------------------------------------
 * key_tlv_parser.c -- parser for Aikido key TLV
 *
 * February 2019, Chandana Prakash
 *
 * Copyright (c) 2019 by Cisco Systems, Inc.
 * All rights reserved.
 *-----------------------------------------------------------------
 */

#include "key_tlv_parser.h"
#include <netinet/in.h>

#define ntohll(x) ((1==ntohl(1)) ? (x) : ((uint64_t)ntohl((x) & 0xFFFFFFFF) << 32) | ntohl((x) >> 32))

/*
 * Gets the data of length "len" and moves src_ptr past len
 */
#define get_data(dst_ptr, src_ptr, len, cnt) do { \
        memcpy(dst_ptr, src_ptr, len); \
        src_ptr += len; \
        cnt += len; \
    } while(0);

/*
 * Gets a single character data and moves the pointer to the next byte after the data.
 */
#define get_char_data(dst, src_ptr, cnt) do { \
        dst = *(src_ptr); \
        ptr++; \
        cnt++; \
} while(0);


/*
 * Gets the length and moves the pointer to the next byte after length
 */
#define get_length(len, ptr, cnt) do { \
            memcpy(&len, ptr, sizeof(len)); \
            len = ntohs(len); \
            cnt += sizeof(len); \
            ptr += sizeof(len); \
        } while(0);


uint32_t
get_32 (uint8_t **src)
{
    uint32_t value;
    value  = ((uint32_t) *(*src)++) << 24;
    value += ((uint32_t) *(*src)++) << 16;
    value += ((uint32_t) *(*src)++) << 8;
    value += ((uint32_t) *(*src)++);
    return (value);
}

uint64_t
get_64 (uint8_t **src)
{
    uint64_t value;
    value  = ((uint64_t) *(*src)++) << 56;
    value += ((uint64_t) *(*src)++) << 48;
    value += ((uint64_t) *(*src)++) << 40;
    value += ((uint64_t) *(*src)++) << 32;
    value += ((uint64_t) *(*src)++) << 24;
    value += ((uint64_t) *(*src)++) << 16;
    value += ((uint64_t) *(*src)++) << 8;
    value += ((uint64_t) *(*src)++);
    return (value);
}


/**
 * This function displays the key record tlv.
 *
 * @param[in] key_rec : pointer to the key record tlv struct.
 * @return 
 * - void
 *
 * This function simply displays the key record tlv struct. 
 */
void 
dump_key_record_info (key_record *key_rec)
{
    int32_t i;
    uint32_t val;
    char time_str[50];
    struct tm * tm_info;
    time_t epoch_time = 0;
    const uint8_t cs_null[50] = "Not present";

    if (!key_rec) {
        return;
    }
    printf("\n##################################\n");
    printf("       KEY RECORD INFO\n");
    printf("Key ID                  : %lx\n", key_rec->key_id);
    printf("Validity                : \n    Not Before: ");
    epoch_time = ntohll(key_rec->start_timestamp);
    tm_info = localtime(&epoch_time);
    strftime(time_str, 50, "%b %d %H:%M:%S %Y %Z", tm_info);
    puts(time_str);
    if(key_rec->end_timestamp) {
        printf("    Not After: ");
        epoch_time = ntohll(key_rec->end_timestamp);
        tm_info = localtime(&epoch_time);
        strftime(time_str, 50, "%b %d %H:%M:%S %Y %Z", tm_info);
        puts(time_str);
    }
    printf("Key Usage               : 0x%x\n", ntohs(key_rec->key_usage));
    printf("Key Format              : 0x%x\n", key_rec->key_format);
    printf("Key Type                : 0x%x\n", key_rec->key_type);       
    printf("Key Algorithm           : 0x%x\n", key_rec->key_algo);
    printf("Key Info Length         : %d\n", key_rec->key_info_len); 
    printf("Modulus size            : %d\n", key_rec->mod_size);
    printf("Modulus                 : ");
    for (i = 0; i < key_rec->mod_size; i += sizeof(val)) {
        val = ntohl(*(unsigned int*)&key_rec->modulus[i]);
        printf("%x", val);
    }
    printf("\n");
    printf("Public Exponent size    : (%d)\n", key_rec->pub_exp_size);
    printf("Public Exponent         : %x\n", key_rec->pub_exp);
    printf("Product Name            : %s\n",
        key_rec->prod_name ? key_rec->prod_name : cs_null);
    printf("\n##################################\n");
}


/**
 * This function validates the END MAGIC at the offset passed. 
 *
 * @param[in] **key       : pointer to the key record buffer.
 * @param[in] magic_offset: offset to the end magic.
 * @return 
 * - returns ERR_OK (0) upon success 
 * - returns error code upon failure.
 * 
 * This function is invoked from the key record parser and it
 * reads the end magic value at the offset in the key record
 * buffer, and makes sure it is valid. This check helps validate
 * the basic sanity of the key record, before proceeding to 
 * parse the key record tlv.
 */
static int 
validate_key_end_magic (uint8_t **buf, int32_t magic_offset)
{
    int         ret = 0;
    uint32_t    key_tlv_end_magic = 0;
    uint8_t      *ptr = NULL;
    uint8_t     end_magic[KEY_END_MAGIC_SIZE];
    uint8_t     *buffer = NULL;
    ptr = *buf;

    /*
     * Skip to the magic offset
     */
    ptr = ptr + magic_offset;

    /*
     * Now we should read the End magic to make sure it is correct.
     */
    get_data(end_magic, ptr, KEY_END_MAGIC_SIZE, magic_offset);

    /*
     * Compare the end magic value to make sure it is 
     * the correct one. The value should match
     * proceed to read the key envelope. Valid start magic
     * value should be "0xab1234cd"
     */
    buffer = &end_magic[0];
    key_tlv_end_magic = get_32(&buffer);

    if (key_tlv_end_magic == KEY_END_MAGIC) {
        ret = ERR_OK; 
    } else {
       printf("\nEnd Magic (0x%8x) not found at the end of the key record (%d) \n",
                  key_tlv_end_magic, magic_offset);
       ret = ERR_KEY_INVALID_LEN;
    }

    return(ret);
}


int 
parse_key_record (uint8_t **key, 
                  key_record *key_rec)
{
    uint8_t     tag = 0;
    uint16_t    len = 0;
    uint32_t    key_tlv_magic = 0;
    uint32_t    offset = 0, pub_key_size = 0;
    int32_t     key_rec_length = 0, pad_bytes = 0;
    int         ret = 0;
    uint64_t    keyid = 0;
    key_record_hdr_tlv  key_hdr = {{0, 0},
                                   {0, 0, 0, 0},
                                   {0, 0, 0, 0, 0, 0, 0, 0}};
    tlventry    *tlv = NULL;
    uint8_t     *pub_exponent = NULL;
    uint8_t     *ptr = NULL, *buf = NULL;

    if (!key || !key_rec) {
        printf("ERROR: Invalid inputs to parse_key_record function \n");
        return(ERR_INVALID_INPUT);
    }

    memset(key_rec, 0, sizeof(key_record));

    ptr = *key; 
    tlv = (tlventry *) ptr;

    /*
     * We always begin by reading the Key Record Header.     
     */
    if (tlv->type == KEY_HDR_AIKIDO_TAG) {
        //printf("\nSecure Storage Key Header Tag\n");
    } else {
        printf("ERROR: Invalid header tag \n");
        return(ERR_INVALID_HDR_TAG);
    }

    /*
     * Now we have the size of the key record TLV.
     * Adding that to the size of the key record header gives us
     * end offset. We will skip to this offset, in case of a failure
     * at any point in parsing the key record.
     */
    key_rec_length = ntohs(tlv->length);
    pub_key_size = key_rec_length + sizeof(key_record_hdr_tlv);
    
    /*
     * Increment the offset and ptr by the size of tlv (3 bytes).
     */
    offset += sizeof(*tlv);
    ptr += sizeof(*tlv);

    /*
     * Now read the Key Record Start magic value.
     */ 
    get_data(key_hdr.magic, ptr, sizeof(key_hdr.magic), offset);

    /*
     * Compare the magic value to make sure whether we should
     * proceed to read the key envelope. Valid start magic
     * value should be "0xab1234cd"
     */
    buf = &key_hdr.magic[0];
    key_tlv_magic = get_32(&buf);
    //printf("Key Start magic = 0x%08X \n", key_tlv_magic);
    if (key_tlv_magic != KEY_START_MAGIC) {
        return(ERR_KEY_MAGIC_INVALID);
    }

    /*
     * Now read the KeyID value.
     */ 
    get_data(key_hdr.keyid, ptr, sizeof(key_hdr.keyid), offset);
    buf = &key_hdr.keyid[0];
    keyid = get_64(&buf);
    //printf("\nKeyID = 0x%0lx\n", keyid);

    /*
     * Make sure that we find a valid end magic at the end of
     * the key record length
     */
    ret = validate_key_end_magic(key, pub_key_size);
    if (ret) {
        printf("Failed to find a valid end magic at offset: 0x%02x", pub_key_size);
        return(ret);
    }

    /*
     * Now we start reading the KEY RECORD TLV.
     */
    while (offset < pub_key_size) {
        /*
         * Read the Tag
         */
        tlv = (tlventry *) ptr;

        if (tlv->type != KEY_RECORD_TYPE_PAD) {
            len = ntohs(tlv->length);
            offset += sizeof(*tlv); ptr += sizeof(*tlv);
        }
        //printf(" len: %d", len);

        switch (tlv->type) {
            case TYPE_VERSION:
                if (len != sizeof(key_rec->version)) {
                    printf("\nInvalid length (%d) for type (%d)",
                             len, tlv->type);
                    ret = ERR_KEY_INVALID_LEN;
                    break;
                }
                get_data(&key_rec->version, ptr, len, offset);
                //printf("\nversion = 0x%x\n", ntohs(key_rec->version));
                break;
            case TYPE_KEY_USAGE:
                if (len != sizeof(key_rec->key_usage)) {
                    printf("\nInvalid length (%d) for type (%d)",
                             len, tlv->type);
                    ret = ERR_KEY_INVALID_LEN;
                    break;
                }
                get_data(&key_rec->key_usage, ptr, len, offset);
                //printf("\nkey_usage = 0x%x\n", ntohs(key_rec->key_usage));
                break;
            case TYPE_START_TIMESTAMP:
                if (len != sizeof(key_rec->start_timestamp)) {
                    printf("\nInvalid length (%d) for type (%d)",
                             len, tlv->type);
                    ret = ERR_KEY_INVALID_LEN;
                    break;
                }
                get_data(&key_rec->start_timestamp, ptr, len, offset);
                //printf("\nstart_timestamp = 0x%lx\n", ntohll(key_rec->start_timestamp));
                break;
            case TYPE_END_TIMESTAMP:
                if (len != sizeof(key_rec->end_timestamp)) {
                    printf("\nInvalid length (%d) for type (%d)",
                             len, tlv->type);
                    ret = ERR_KEY_INVALID_LEN;
                    break;
                }
                get_data(&key_rec->end_timestamp, ptr, len, offset);
                //printf("\nend_timestamp = 0x%lx\n", ntohll(key_rec->end_timestamp));
                break;
            case TYPE_KEY_FORMAT:
                if (len != sizeof(key_rec->key_format)) {
                    printf("\nInvalid length (%d) for type (%d)",
                             len, tlv->type);
                    ret = ERR_KEY_INVALID_LEN;
                    break;
                }
                get_char_data(key_rec->key_format, ptr, offset);
                break;
            case TYPE_KEY_PAYLOAD_LEN:
                key_rec->key_payload_len = len;
                break;
            //key payload
            case TYPE_KEY_TYPE:
                if (len != sizeof(key_rec->key_type)) {
                    printf("\nInvalid length (%d) for type (%d)",
                             len, tlv->type);
                    ret = ERR_KEY_INVALID_LEN;
                    break;
                }
                get_char_data(key_rec->key_type, ptr, offset);
                break;
            //key payload
            case TYPE_KEY_ALGO:
                if (len != sizeof(key_rec->key_algo)) {
                    printf("\nInvalid length (%d) for type (%d)",
                             len, tlv->type);
                    ret = ERR_KEY_INVALID_LEN;
                    break;
                }
                get_char_data(key_rec->key_algo, ptr, offset);

                if ((key_rec->key_algo != RSA_ALGO) && (key_rec->key_algo != LDWM_ALGO)) {
                    printf("Invalid key algorithm algo=%d \n", key_rec->key_algo);
                    ret = ERR_INVALID_KEY_ALGO;
                }
                break;
            //key payload
            case TYPE_KEY_INFO_LEN:
                key_rec->key_info_len = len;
                break;
            //key payload
            case TYPE_MODULUS:
                if (len > (pub_key_size - offset)) {
                    printf("\nInvalid length (%d) for type (%d). offset: %d",
                  len, 
                          tlv->type, offset);
                    ret = ERR_KEY_INVALID_LEN;
                    break;
                }
                key_rec->mod_size = len;
                get_data(key_rec->modulus, ptr, len, offset);
                key_rec->modulus[len] = '\0';
                break;
            //key payload
            case TYPE_PUB_EXP:
                if (key_rec->key_algo != RSA_ALGO) {
                    break; //Skip this TAG for ECC and LDWM keys
                }

                if (len > (pub_key_size - offset)) {
                    printf("\nInvalid len (%d) for type (%d).\
                                key_rec_len: %d, pub key size: %d  offset: %d",
                             len, tlv->type, 
                             key_rec_length, pub_key_size, offset);
                    ret = ERR_KEY_INVALID_LEN;
                    break;
                }
                key_rec->pub_exp_size = len;
                if (key_rec->key_info_len != 
                   (key_rec->mod_size + key_rec->pub_exp_size
                         + KEY_INFO_SZ_OVERHEAD)) {
                    printf("\nInvalid Key info length (%d),\
                                mod size (%d) + pub exp (%d) + tlv is %ld",
                                key_rec->key_info_len, 
                                key_rec->mod_size, key_rec->pub_exp_size,
                                (key_rec->mod_size + 
                                key_rec->pub_exp_size + KEY_INFO_SZ_OVERHEAD));
                    ret = ERR_KEY_INVALID_LEN;
                    break;
                }
                pub_exponent = (uint8_t *)&key_rec->pub_exp;
                if (len == sizeof(key_rec->pub_exp)) {
                    /* do nothing */
                } else if (len == (sizeof(key_rec->pub_exp) - 1)) {
                    pub_exponent++;
                } else {
                    printf("\nInvalid length %d for type %d, sizeof pub_exp field: %ld",
                              len, tlv->type, sizeof(key_rec->pub_exp));
                    ret = ERR_KEY_INVALID_LEN;
                    break;
                }
                key_rec->pub_exp = 0;
                get_data(pub_exponent, ptr, len, offset);
                key_rec->pub_exp = ntohl(key_rec->pub_exp);
                break;
            //key payload
            case TYPE_KEY_VERSION:
            /*
             * Version identifies the variant of the key that may 
             * match with the image extension.
             */
                if ((len > (pub_key_size - offset)) ||
                    (len != sizeof(key_rec->key_version))) {
                    printf("\nInvalid len (%d) for type (%d).\
                                key_rec_len: %d, pub key size: %d  offset: %d",
                             len, tlv->type, 
                             key_rec_length, pub_key_size, offset);
                    ret = ERR_KEY_INVALID_LEN;
                    break;
                }
                get_char_data(key_rec->key_version, ptr, offset);
                break;
            //key payload
            case TYPE_PROD_NAME:
                if (len > (pub_key_size - offset)) {
                     printf("\nInvalid len (%d) for type (%d).\
                               key_rec_len: %d, pub key size: %d  offset: %d",
                               len, tlv->type, 
                               key_rec_length, pub_key_size, offset);
                    ret = ERR_KEY_INVALID_LEN;
                    break;
                }
                if (len > MAX_KEY_PROD_NAME_LEN) {
                    printf("\nLength %d for prod name exceeds max len %d",
                        len, MAX_KEY_PROD_NAME_LEN);
                    memcpy(key_rec->prod_name, ptr, MAX_KEY_PROD_NAME_LEN);
                    key_rec->prod_name[MAX_KEY_PROD_NAME_LEN] = '\0';
                    ptr += len; offset += len;
                } else {
                    get_data(key_rec->prod_name, ptr, len, offset);
                    key_rec->prod_name[len] = '\0';
                }
                break;
            case KEY_RECORD_TYPE_PAD:
                pad_bytes = offset & 0x03;
                while (pad_bytes < 4) {
                    tag = *ptr;
                    if (tag != KEY_PAD_VALUE) {
                        printf("\nPad byte mismatch 0x%x", tag);
                        ret = ERR_KEY_INVALID_PAD;
                        break; 
                    }
                    ptr++; offset ++; pad_bytes++;
                }
 
                break;
            default: 
                /*
                 * After reading the public exponent, we keep parsing
                 * till the end of the tlv to read the key end magic.
                 */
                printf("\nSkipping unknown tag (0x%x) at offset (%d) len(%d)",
                         tlv->type, offset, len);
                ptr += len; offset += len;
                break;
        } /* End of switch */
        if (ret) {
            printf("\nError (%d) during key record parsing, hence break",
                      ret);
            return(ret); 
        }
    } /* End of while */

    return(ret);
}

/*---------------------------------------------------------------
$log: $
$endlog$
*/

