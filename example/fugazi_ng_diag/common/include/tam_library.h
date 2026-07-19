/*------------------------------------------------------------------
 * tam_library.h -- 
 * 
 * TAM Library v1.2.4-Release 
 * Built: Mon Apr 20 10:57:43 PDT 2015  
 * by   manishma@cisco.com 
 * 
 * DO NOT EDIT - THIS FILE IS GENERATED 
 * 
 * Copyright (c) 2013 by Cisco Systems, Inc. 
 * All rights reserved. 
 *------------------------------------------------------------------
 */
 
 

#ifndef __TAM_LIBRARY_H__
#define __TAM_LIBRARY_H__

/*
 *  
 * When integrating the library, the conditional can be
 * resolved by including a #define or by defining the
 * desired macro via the makefile.  
 *  
 */ 
#if defined TAM_ON_IOS

#include COMP_INC(kernel,ios_kernel_util.h)
//#include <stdio.h>
#include <string.h>
//#include <stdlib.h>
#include <time.h>

#elif TAM_ON_WINDOWS 

//#include <stdio.h>
#include <string.h>
//#include <stdlib.h>
#include <stdint.h>
#include <time.h>

#else

//#include <stdio.h>
#include <string.h>
//#include <stdlib.h>
#include <inttypes.h> 
#include <time.h>
#endif


#ifndef TRUE
#define TRUE   ( 1 )
#endif

#ifndef FALSE
#define FALSE  ( 0 )
#endif

#ifndef NULL
#define NULL ((void *) 0)
#endif




/* FW version v1.3 baseline */
#define TAM_ACT2_V1_3_FW                    ( 0x18 )

/* FW version 1.4 introduces ECC */ 
#define TAM_ACT2_V1_4_FW                    ( 0x19 )

/* FW version aikido baseline */
#define TAM_ACT2_AIKIDO_FW                  ( 0x80 )


/* Bus Modes */
#ifndef BUS_MODE_LEGACY 
#define BUS_MODE_LEGACY                     ( 0 )
#endif
#ifndef BUS_MODE_SIMPLE
#define BUS_MODE_SIMPLE                     ( 1 )
#endif
#ifndef BUS_MODE_UNKNOWN
#define BUS_MODE_UNKNOWN                    ( -1 )
#endif

/* maximum number of random bytes that can be collected over the bus */
#define TAM_MAX_RANDOM_LENGTH               ( 240 )

/* 
 * Maximum number of 16-byte bounded bytes in a crypto msg
 * This makes allowances for library padding and internal
 * chip padding.  
 */ 
#define TAM_MAX_CRYPTO_LENGTH               ( 224 )

/* maximum number of random bytes that can be collected over the bus */ 
#define TAM_MAX_HASH_LENGTH                 ( 240 )

/* maximum payload in bytes */ 
#define TAM_MAX_PAYLOAD_LENGTH              ( 255 )

/* header plus max payload plus checksum */ 
#define TAM_MAX_MESSAGE_LENGTH              ( 259 )

/* number bytes of random data to seed and re-seed the DRBG */ 
#define TAM_LIB_DRBG_SEED_LEN               ( 0x30 )


/* internal flag mask */ 
#define  ZERO_FLAG       ( 0x02 )
#define  EEPROM_FLAG     ( 0x01 )


/*
 * Library return codes
 */ 
/* Success Return Codes */ 
#define TAM_RC_OK                           ( 0x00 )
#define TAM_LIB_RC_REPEAT                   ( 0x01 )

/* these are ACT2 specific error codes */ 

#define TAM_LIB_ERR_CKSUM_BAD               ( 0x0B )
#define TAM_LIB_ERR_CMD_UNKNOWN             ( 0x0C )
#define TAM_LIB_ERR_CMD_LENGTH              ( 0x0D )
#define TAM_LIB_ERR_PARAMETER_INVALID       ( 0x0E )
#define TAM_LIB_ERR_ALGORITHM_INVALID       ( 0x0F )

#define TAM_LIB_ERR_PERMISSION_NOT          ( 0x10 )
#define TAM_LIB_ERR_SESSION_GEN_FAIL        ( 0x11 )
#define TAM_LIB_ERR_SESSION_NOT_AVAIL       ( 0x12 )
#define TAM_LIB_ERR_SESSION_IS_OPEN         ( 0x13 )
#define TAM_LIB_ERR_SESSION_INVALID         ( 0x14 )
#define TAM_LIB_ERR_HANDLE_INVALID          ( 0x15 )
#define TAM_LIB_ERR_SORW_PENDING            ( 0x16 )
#define TAM_LIB_ERR_RESEED_REQUIRED         ( 0x17 )

#define TAM_LIB_ERR_USERID_UNKNOWN          ( 0x18 )
#define TAM_LIB_ERR_USERID_INVALID          ( 0x19 )
#define TAM_LIB_ERR_USERID_EXISTS           ( 0x1A )

#define TAM_LIB_ERR_BOUNDS_CHECK            ( 0x20 )
#define TAM_LIB_ERR_EEPROM_SPACE            ( 0x21 )
#define TAM_LIB_ERR_EEPROM_WRITE            ( 0x22 )
#define TAM_LIB_ERR_ALGORITHM               ( 0x23 )
#define TAM_LIB_ERR_KEY_LENGTH              ( 0x24 )
#define TAM_LIB_ERR_OBJECT_DIRTY            ( 0x25 )
#define TAM_LIB_ERR_OBJECT_GENERAL          ( 0x26 )
#define TAM_LIB_ERR_OBJECT_IN_ROM           ( 0x27 )
#define TAM_LIB_ERR_OBJECT_LENGTH           ( 0x28 )
#define TAM_LIB_ERR_OBJECT_PERMISSION       ( 0x29 )
#define TAM_LIB_ERR_OBJECT_PRIVATE          ( 0x2A )
#define TAM_LIB_ERR_OBJECT_TYPE             ( 0x2B )
#define TAM_LIB_ERR_PROCESS_FAIL            ( 0x2C )
#define TAM_LIB_ERR_RAM_SPACE               ( 0x2D )
#define TAM_LIB_ERR_VERIFY_FAIL             ( 0x2E )
#define TAM_LIB_ERR_DATA_FORMAT             ( 0x2F )

#define TAM_LIB_ERR_LIBRARY                 ( 0x30 )  /* vendor library error */
#define TAM_LIB_ERR_CODE_FAIL               ( 0x31 ) 
#define TAM_LIB_ERR_FIPS_TEST_FAIL          ( 0x32 )
#define TAM_LIB_ERR_ZEROED_IDEVID_KEYS      ( 0x33 )

/* these are library specific error codes */
#define TAM_LIB_ERR_UDI_INVALID             ( 0xE0 )
#define TAM_LIB_ERR_UDI_MISMATCH            ( 0xE1 )

#define TAM_LIB_ERR_FW_VERSION              ( 0xF1 )
#define TAM_LIB_ERR_BUS_TOO_SMALL           ( 0xF2 )
#define TAM_LIB_ERR_DUPLICATE               ( 0xF3 )
#define TAM_LIB_ERR_INVALID_STATE           ( 0xF4 )
#define TAM_LIB_ERR_USER_NOT_LOGGED         ( 0xF5 )
#define TAM_LIB_ERR_NULL_HANDLE             ( 0xF6 )
#define TAM_LIB_ERR_DETECTED                ( 0xF7 )
#define TAM_LIB_ERR_RANGE_INVALID           ( 0xF8 )
#define TAM_LIB_ERR_NULL_POINTER            ( 0xF9 )
#define TAM_LIB_ERR_NO_RESOURCES            ( 0xFA )
#define TAM_LIB_ERR_READ_FAILURE            ( 0xFB )
#define TAM_LIB_ERR_READ_CHECKSUM_FAILURE   ( 0xFC )
#define TAM_LIB_ERR_WRITE_FAILURE           ( 0xFD )
#define TAM_LIB_ERR_INVALID_LENGTH          ( 0xFE )
#define TAM_LIB_ERR_NOT_SUPPORTED           ( 0xFF )


/*
 * return type for APIs
 */ 
typedef uint32_t tam_lib_status_t;


/* 
 * user constants for easy reference 
 */  
#define TAM_NULL_USER                       ( 0x00 )
#define TAM_ADMIN_USER                      ( 0x01 )

#define TAM_RESTRICTED_USER2                ( 0x02 )
#define TAM_RESTRICTED_USER3                ( 0x03 )
#define TAM_RESTRICTED_USER4                ( 0x04 )
#define TAM_RESTRICTED_USER5                ( 0x05 )
#define TAM_RESTRICTED_USER6                ( 0x06 )
#define TAM_RESTRICTED_USER7                ( 0x07 )
#define TAM_RESTRICTED_USER8                ( 0x08 )
#define TAM_RESTRICTED_USER9                ( 0x09 )
#define TAM_RESTRICTED_USER10               ( 0x0A )
#define TAM_RESTRICTED_USER11               ( 0x0B )
#define TAM_RESTRICTED_USER12               ( 0x0C )
#define TAM_RESTRICTED_USER13               ( 0x0D )
#define TAM_RESTRICTED_USER14               ( 0x0E )
#define TAM_RESTRICTED_USER15               ( 0x0F )

#define TAM_LIB_NUM_USERS                   ( 0x10 )
#define TAM_LIB_NUM_RESTRICTED_USERS        ( 0x0E )


/*
 * create object attributes - use with APIs 
 */ 
#define TAM_LIB_MEM_RAM                     ( 0x00 )
#define TAM_LIB_MEM_EEPROM                  ( 0x01 )

#define TAM_LIB_NO_ZEROIZE                  ( 0x00 )
#define TAM_LIB_ZEROIZE                     ( 0x01 )

#define TAM_LIB_CLEAR_TEXT                  ( 0x00 )
#define TAM_LIB_ENCRYPT                     ( 0x01 )


/*
 * number of bytes in a PIN, this is not a string
 */ 
#define TAM_PIN_LENGTH                      ( 0x20 )

/*
 * chip parameters 
 */  
#define CHIP_SERIAL_NUMBER_LENGTH           ( 0x20 )
#define TAM_LIB_METAL_RAND_LENGTH           ( 0x20 )

/*
 * max number of objects return in object list
 */ 
#define TAM_LIB_MAX_ENUM_OBJECTS            ( 0x32 )

/* 
 * object types
 */ 
#define TAM_INVALID_OBJECT                  ( 0x00 )  /* invalid   */
#define TAM_RAW_OBJECT                      ( 0x01 )  /* formless  */
#define TAM_SYMMETRIC_KEYIV_OBJECT          ( 0x02 )  /* AES Key   */
#define TAM_RSA_KEYPAIR_OBJECT              ( 0x03 )  /* RSA key pair  */
#define TAM_X509_CERT_OBJECT                ( 0x04 )  /* RSA X.509 Cert   */
#define TAM_X509_CERTCHAIN_OBJECT           ( 0x05 )  /* RSA X.509 Cert Chain  */
#define TAM_ECC_KEYPAIR_OBJECT              ( 0x06 )  /* ECC key pair  */


/* 
 * Predefined Objects - objects created at manufacturing
 * or native to the device 
 */ 
#define TAM_LIB_NULL_HANDLE                 ( 0x00000000 )
#define TAM_LIB_I2C_INPUT                   ( 0x00000001 )
#define TAM_LIB_I2C_OUTPUT                  ( 0x00000002 )
  
#define TAM_LIB_CLII_KEY_PAIR               ( 0x00000010 )
#define TAM_LIB_CLII_CERT                   ( 0x00000013 )
#define TAM_LIB_CLII_CERT_CHAIN             ( 0x00000014 )

#define TAM_LIB_IDEVID_RSA_KEY_PAIR         ( 0x00000018 )
#define TAM_LIB_IDEVID_RSA_CERT             ( 0x0000001B )
#define TAM_LIB_IDEVID_RSA_CERT_CHAIN       ( 0x0000001C )

#define TAM_LIB_IDEVID_ECC_KEY_PAIR         ( 0x00000020 )
#define TAM_LIB_IDEVID_ECC_CERT             ( 0x00000023 )
#define TAM_LIB_IDEVID_ECC_CERT_CHAIN       ( 0x00000024 )

#define TAM_LIB_IDEVID_HARSA_CERT          ( 0x00000028 )
#define TAM_LIB_IDEVID_HARSA_CERT_CHAIN    ( 0x00000029 )
#define TAM_LIB_AIK_RSA_CERT               ( 0x00000030 )
#define TAM_LIB_AIK_RSA_CERT_CHAIN         ( 0x00000031 )

#define TAM_LITTLE_ENDIAN                  ( 0 )
#define TAM_BIG_ENDIAN                     ( 1 )

/* buffer sizes required to read the predefined object */  
#define TAM_LENGTH_IDEVID_CERT              ( 0x800 )
#define TAM_LENGTH_IDEVID_CERT_CHAIN        ( 0xB00 )

/* 
 * Hash Algorithms for SHA and RSA signature APIs  
 */ 
#define TAM_LIB_HASH_ALGO_SHA1              ( 0x20 )
#define TAM_LIB_HASH_ALGO_SHA256            ( 0x50 )
#define TAM_LIB_HASH_ALGO_SHA384            ( 0x60 )
#define TAM_LIB_HASH_ALGO_SHA512            ( 0x70 )

/* 
 * Symmetric Crypto Algorithms 
 */ 
#define TAM_LIB_SYM_ALGO_AES128             ( 0x10 )
#define TAM_LIB_SYM_ALGO_AES192             ( 0x20 )
#define TAM_LIB_SYM_ALGO_AES256             ( 0x30 )

/* 
 * Symmetric Crypto Key Sizes
 */ 
#define TAM_LIB_SYM_AES128_KEYSIZE          ( 0x10 )
#define TAM_LIB_SYM_AES192_KEYSIZE          ( 0x18 )
#define TAM_LIB_SYM_AES256_KEYSIZE          ( 0x20 )
#define AES_KEY_SIZE_MAX                    ( 0x20 )

/* 
 * Symmetric Crypto Modes   
 */ 
#define TAM_LIB_SYM_MODE_ECB                ( 0x01 )
#define TAM_LIB_SYM_MODE_CBC                ( 0x02 )
#define TAM_LIB_SYM_MODE_ECB_NO_PAD         ( 0x03 )

/* 
 * RSA Crypto Key Sizes 
 */ 
#define RSA_1024_KEY_SIZE                   ( 128 )
#define RSA_2048_KEY_SIZE                   ( 256 )

/* 
 * RSA Crypto E value 
 */ 
#define RSA_E_VALUE_3                       ( 3 )
#define RSA_E_VALUE_65537                   ( 65537 )

/* 
 * ECC Crypto Key Sizes 
 */
#define ECC_P192_KEY_SIZE                   ( 24 )
#define ECC_P256_KEY_SIZE                   ( 32 )
#define ECC_P384_KEY_SIZE                   ( 48 )
#define ECC_P521_KEY_SIZE                   ( 66 )

/* 
 * Certificate Type
 */
#define ECC_SUDI        ( 0 )
#define RSA_SUDI        ( 1 )
#define DEFAULT_SUDI    ( 2 )

/*
 * Aikido specific delclarations
 */
#define TAM_AIKIDO_EEPROM_VALID_PAGES      ( 0x80 )
#define TAM_AIKIDO_EEPROM_SIZE             ( 0x4000 )

/*
 * tam_get_library_version()
 */ 
typedef struct {
    uint16_t major;
    uint16_t minor;
    uint16_t patch;
} tam_library_version_t;


/* 
 * tam_lib_get_chip_info()  
 */
#define TAM_FW_VERSION_LENGTH        ( 3 )
#define TAM_RESET_STATUS_LENGTH      ( 7 )
#define TAM_RESET_COUNTS_BYTES       ( 7 )
typedef struct {
    uint8_t fw_version[TAM_FW_VERSION_LENGTH];
    uint8_t metal_rand[TAM_LIB_METAL_RAND_LENGTH];
    uint8_t latest_reset_status[TAM_RESET_STATUS_LENGTH];
    uint8_t reset_counts[TAM_RESET_COUNTS_BYTES];
    uint16_t  total_ram;
    uint16_t  total_rom;
} tam_lib_chip_info_t;


/* 
 * tam_scc_read_id()
 */ 
typedef struct {
    uint8_t chip_type;
    uint8_t chip_vendor;
    uint8_t firmware_version;
    uint8_t post_result;
    uint8_t bus_mode;
} tam_lib_scc_id_t;


/*
 * user memory information used with the
 * admin function tam_mem_info()
 */
typedef struct {
    uint8_t  user_id;
    uint32_t mem_used;
} tam_lib_user_info_t;


/* 
 * tam_lib_object_list()
 */  
typedef struct {
    uint32_t object_id;
    uint8_t object_type;
} tam_lib_object_enum_couplet_t;


/*
 * tam_lib_object_attributes() 
 */ 
typedef struct {
    uint8_t object_type;
    uint8_t prom_flag;
    uint8_t flags;
    uint16_t  size;
    uint16_t  length;
    uint16_t  read_permissions;
    uint16_t  write_permissions;
    uint16_t  use_permissions;
} tam_lib_object_attributes_t;



/* 
 * few library specific routines  
 */ 
extern tam_lib_status_t
tam_lib_get_library_version(tam_library_version_t *tam_version);

extern void
tam_lib_display_library_version(void);


/* 
 * few handy utility routines 
 */ 
extern char
*tam_lib_storage_location2string(uint8_t location);

extern char
*tam_lib_storage_mode2string(uint8_t storage);

extern char
*tam_lib_predefined2string(uint8_t predefined_oid);

extern char
*tam_lib_symm_algo2string(uint8_t symm_algo);

extern char
*tam_lib_symm_mode2string(uint8_t symm_mode);

extern char 
*tam_lib_object_type2string(uint8_t object_type);

extern char
*tam_lib_rc2string(uint8_t status);




/* 
 * few chip level and helper APIs 
 */ 
extern uint8_t
tam_lib_check_mode(void *tam_handle);

extern tam_lib_status_t
tam_lib_scc_read_id(void *tam_handle,
                   tam_lib_scc_id_t *scc_id);

extern tam_lib_status_t
tam_lib_scc_read_eeprom(void *tam_handle,
                   uint8_t *dest_buffer,
                   uint16_t length,
                   uint16_t src);

extern tam_lib_status_t
tam_lib_scc_write_eeprom(void *tam_handle,
                   uint8_t *src_buffer,
                   uint16_t length,
                   uint16_t dest);

extern tam_lib_status_t
tam_lib_scc_lock_eeprom(void *tam_handle,
                        uint16_t start_page,
                        uint16_t num_pages);

extern tam_lib_status_t
tam_lib_scc_read_cookie(void *tam_handle,
                   uint8_t *dst_buffer,
                   uint16_t length);


extern tam_lib_status_t
tam_lib_set_simple(void *tam_handle);

extern tam_lib_status_t
tam_lib_get_chip_serial_number(void *tam_handle,
                   uint8_t *chip_serial_number);

extern tam_lib_status_t
tam_lib_get_chip_info(void *tam_handle,
                   tam_lib_chip_info_t *chip_info);


extern tam_lib_status_t 
tam_lib_fips_selftest(void *tam_handle);

extern tam_lib_status_t
tam_lib_zero_idevid_private(void *tam_handle,
                   uint32_t admin_session_id);


/*
 * TAM LIB Authentication
 */ 
extern tam_lib_status_t
tam_lib_authentication(void *tam_handle);

extern tam_lib_status_t
tam_lib_authentication_udi(void *tam_handle, char *pid, char *sn,
                           uint8_t cert_type);

extern tam_lib_status_t
tam_lib_aik_validate_certchain_udi(void *tam_handle,
                                   uint32_t session_id,
                                   char *prod_name,
                                   char *prod_sn);

/*
 * Admin PIN generation 
 */ 
extern tam_lib_status_t
tam_lib_generate_admin_pin(void *tam_handle,
                   uint8_t *admin_pin);


/*
 * user and session APIs  
 */ 
extern tam_lib_status_t
tam_lib_user_add(void *tam_handle,
                   uint32_t admin_session_id,
                   uint8_t user_id,
                   uint8_t *pin);

extern tam_lib_status_t
tam_lib_user_delete(void *tam_handle,
                   uint32_t admin_session_id,
                   uint8_t user_id);


extern tam_lib_status_t
tam_lib_session_init(void *tam_handle,
                   uint8_t user_id,
                   uint8_t *pin,
                   uint32_t *session_id);

extern tam_lib_status_t
tam_lib_session_end(void *tam_handle,
                   uint32_t session_id);


/* 
 * Secure Objects
 */  
extern tam_lib_status_t
tam_lib_object_attributes(void *tam_handle,
                   uint32_t session_id,
                   uint32_t object_id,
                   tam_lib_object_attributes_t *attributes);

extern tam_lib_status_t
tam_lib_object_create(void *tam_handle,
                   uint32_t session_id,
                   uint8_t object_type,
                   uint16_t object_size,
                   uint8_t csp_flag,
                   uint8_t memory_flag,
                   uint8_t encryption_flag,
                   uint32_t *object_id);

extern tam_lib_status_t
tam_lib_object_delete(void *tam_handle,
                   uint32_t session_id,
                   uint32_t object_id);

extern tam_lib_status_t
tam_lib_object_enumerate(void *tam_handle,
                   uint32_t session_id,
                   uint8_t *num_objects,
                   tam_lib_object_enum_couplet_t *object_list);

extern tam_lib_status_t
tam_lib_object_list(void *tam_handle,
                   uint32_t session_id,
                   uint16_t index_start,
                   uint8_t couplet_max,
                   uint16_t *num_objects,
                   uint8_t *couplet_count,
                   tam_lib_object_enum_couplet_t *object_list);

extern tam_lib_status_t
tam_lib_object_mem_info(void *tam_handle,
                   uint32_t admin_session_id,
                   uint8_t eeprom_flag,
                   uint32_t *free_space,
                   uint8_t *user_count,
                   tam_lib_user_info_t *user_info);

extern tam_lib_status_t
tam_lib_object_permission(void *tam_handle,
                   uint32_t session_id,
                   uint32_t  object_id,
                   uint16_t read_map,
                   uint16_t write_map,
                   uint16_t use_map);

extern tam_lib_status_t
tam_lib_object_rw_abort(void *tam_handle,
                   uint32_t session_id);

extern tam_lib_status_t
tam_lib_object_read(void *tam_handle,
                   uint32_t session_id,
                   uint32_t object_id,
                   uint8_t *buffer,
                   uint16_t *length);

extern tam_lib_status_t
tam_lib_object_readinfo(void *tam_handle,
                   uint32_t session_id,
                   uint32_t object_id,
                   uint8_t *object_type,
                   uint16_t *object_size);

extern tam_lib_status_t
tam_lib_object_write(void *tam_handle,
                   uint32_t session_id,
                   uint32_t object_id,
                   uint8_t *buffer,
                   uint16_t length);

extern tam_lib_status_t
tam_lib_object_zeroize(void *tam_handle,
                   uint32_t admin_session_id);


/* 
 * true random - entropy 
 */ 
extern tam_lib_status_t
tam_lib_trand_check(void *tam_handle,
                   uint32_t admin_session_id);

extern tam_lib_status_t
tam_lib_trand_read(void *tam_handle,
                   uint16_t length,
                   uint8_t *buffer);

extern tam_lib_status_t
tam_lib_trand_write(void *tam_handle,
                   uint32_t session_id,
                   uint32_t object_id,
                   uint16_t length);


/* 
 * deterministic random - entropy
 */ 
extern tam_lib_status_t
tam_lib_drand_instantiate(void *tam_handle,
                   uint32_t admin_session_id,
                   uint8_t *seed_data);

extern tam_lib_status_t
tam_lib_drand_read(void *tam_handle,
                   uint16_t length,
                   uint8_t *pbuffer);

extern tam_lib_status_t
tam_lib_drand_reseed(void *tam_handle,
                   uint32_t admin_session_id,
                   uint8_t *seed_data);

extern tam_lib_status_t
tam_lib_drand_verify(void *tam_handle,
                   uint32_t admin_session_id);

extern tam_lib_status_t
tam_lib_drand_write(void *tam_handle,
                   uint32_t session_id,
                   uint32_t dest_object_id,
                   uint16_t length);

/* 
 * Secure Hash, SHA1, SHA2 
 */
extern tam_lib_status_t
tam_lib_hash_init(void *tam_handle,
                   uint32_t session_id,
                   uint8_t algorithm,
                   uint32_t *object_id);

extern tam_lib_status_t
tam_lib_hash_update(void *tam_handle,
                   uint32_t session_id,
                   uint32_t object_id,
                   uint16_t data_length,
                   uint8_t *data_buffer);

extern tam_lib_status_t
tam_lib_hash_finalize(void *tam_handle,
                   uint32_t session_id,
                   uint32_t object_id,
                   uint8_t *hash_length,
                   uint8_t *hash_digest);

extern tam_lib_status_t 
tam_lib_hash_abort(void *tam_handle,
                   uint32_t session_id,
                   uint32_t object_id);

/* 
 * HMAC - Hash-based Message Authentication Code 
 */
extern tam_lib_status_t
tam_lib_hmac_init(void *tam_handle,
                   uint32_t session_id,
                   uint32_t key_object_id,
                   uint8_t algorithm,
                   uint32_t *object_id);

extern tam_lib_status_t
tam_lib_hmac_update(void *tam_handle,
                   uint32_t session_id,
                   uint32_t object_id,
                   uint16_t data_length,
                   uint8_t *data_buffer);

extern tam_lib_status_t
tam_lib_hmac_finalize(void *tam_handle,
                   uint32_t session_id,
                   uint32_t object_id,
                   uint8_t *hmac_length,
                   uint8_t *hmac_result);

extern tam_lib_status_t
tam_lib_hmac_abort(void *tam_handle,
                   uint32_t session_id,
                   uint32_t object_id);

/* 
 * RSA Cryptography
 */ 
extern tam_lib_status_t
tam_lib_rsa_keypair_gen (void *tam_handle,
                   uint32_t session_id,
                   uint16_t key_length,
                   uint32_t e_value,
                   uint8_t csp_flag,
                   uint8_t memory_flag,
                   uint32_t *key_object_id);

extern tam_lib_status_t
tam_lib_rsa_private_encr(void *tam_handle,
                   uint32_t session_id,
                   uint32_t key_object_id,
                   uint32_t clear_text_object_id,
                   uint32_t *cipher_text_object_id);

extern tam_lib_status_t
tam_lib_rsa_private_decr(void *tam_handle,
                   uint32_t session_id,
                   uint32_t key_object_id,
                   uint32_t cipher_text_object_id,
                   uint32_t *clear_text_object_id);

extern tam_lib_status_t
tam_lib_rsa_public_encr(void *tam_handle,
                   uint32_t session_id,
                   uint32_t key_object_id,
                   uint32_t clear_text_object_id,
                   uint32_t *cipher_text_object_id);

extern tam_lib_status_t
tam_lib_rsa_public_decr(void *tam_handle,
                   uint32_t session_id,
                   uint32_t key_object_id,
                   uint32_t cipher_text_object_id,
                   uint32_t *clear_text_object_id);


extern tam_lib_status_t
tam_lib_rsa_sign_init(void *tam_handle,
                   uint32_t session_id,
                   uint32_t key_pair_object_id,
                   uint8_t algorithm,
                   uint32_t *signature_context_id);

extern tam_lib_status_t
tam_lib_rsa_sign_update(void *tam_handle,
                   uint32_t session_id,
                   uint32_t signature_context_id,
                   uint16_t data_len,
                   uint8_t *data);

extern tam_lib_status_t
tam_lib_rsa_sign_finalize(void *tam_handle,
                   uint32_t session_id,
                   uint32_t signature_context_id,
                   uint32_t *signature_block_object_id);

extern tam_lib_status_t
tam_lib_rsa_verify_init(void *tam_handle,
                   uint32_t session_id,
                   uint32_t signature_block_object_id,
                   uint32_t key_pair_object_id,
                   uint8_t algorithm,
                   uint32_t  *verify_context_id);

extern tam_lib_status_t
tam_lib_rsa_verify_update(void *tam_handle,
                   uint32_t session_id,
                   uint32_t verify_context_id,
                   uint16_t data_len,
                   uint8_t *data);

extern tam_lib_status_t
tam_lib_rsa_verify_finalize(void *tam_handle,
                   uint32_t session_id,
                   uint32_t verify_context_id);

extern tam_lib_status_t
tam_lib_rsa_sign_digest(void *tam_handle,
                   uint32_t session_id,
                   uint32_t key_pair_object_id,
                   uint32_t digest_object_id,
                   uint8_t hash_algorithm,
                   uint32_t *signature_object_id);

extern tam_lib_status_t
tam_lib_rsa_verify_digest(void *tam_handle,
                   uint32_t session_id,
                   uint32_t key_pair_object_id,
                   uint32_t digest_object_id,
                   uint32_t signature_object_id,
                   uint8_t hash_algorithm);


/* 
 * ECC Cryptography
 */
extern tam_lib_status_t
tam_lib_ecc_keypair_gen (void *tam_handle,
                   uint32_t session_id,
                   uint16_t key_length,
                   uint8_t csp_flag,
                   uint8_t memory_flag,
                   uint32_t *key_object_id);

extern tam_lib_status_t
tam_lib_ecdsa_sign_init(void *tam_handle,
                   uint32_t session_id,
                   uint32_t key_pair_object_id,
                   uint8_t algorithm,
                   uint32_t *signature_context_id);

extern tam_lib_status_t
tam_lib_ecdsa_sign_update(void *tam_handle,
                   uint32_t session_id,
                   uint32_t signature_context_id,
                   uint16_t data_len,
                   uint8_t *data);

extern tam_lib_status_t
tam_lib_ecdsa_sign_finalize(void *tam_handle,
                   uint32_t session_id,
                   uint32_t signature_context_id,
                   uint32_t *signature_block_object_id);

extern tam_lib_status_t
tam_lib_ecdsa_sign_abort(void *tam_handle,
                         uint32_t session_id,
                         uint32_t sign_context_id);

extern tam_lib_status_t
tam_lib_ecdsa_verify_init(void *tam_handle,
                   uint32_t session_id,
                   uint32_t signature_block_object_id,
                   uint32_t key_pair_object_id,
                   uint8_t algorithm,
                   uint32_t  *verify_context_id);

extern tam_lib_status_t
tam_lib_ecdsa_verify_update(void *tam_handle,
                   uint32_t session_id,
                   uint32_t verify_context_id,
                   uint16_t data_len,
                   uint8_t *data);

extern tam_lib_status_t
tam_lib_ecdsa_verify_finalize(void *tam_handle,
                   uint32_t session_id,
                   uint32_t verify_context_id);

extern tam_lib_status_t
tam_lib_ecdsa_verify_abort(void *tam_handle,
                   uint32_t session_id,
                   uint32_t verify_context_id);

extern tam_lib_status_t
tam_lib_ecdh_secret_calc(void *tam_handle,
                   uint32_t session_id,
                   uint32_t key_pair_object_id,
                   uint32_t remote_data_objectid,
                   uint8_t *secret_data_length,
                   uint8_t *secret_data);

extern tam_lib_status_t
tam_lib_ecdsa_sign_digest(void *tam_handle,
                   uint32_t session_id,
                   uint32_t key_pair_object_id,
                   uint32_t digest_object_id,
                   uint8_t hash_algorithm,
                   uint32_t *signature_object_id);

extern tam_lib_status_t
tam_lib_ecdsa_verify_digest(void *tam_handle,
                   uint32_t session_id,
                   uint32_t key_pair_object_id,
                   uint32_t digest_object_id,
                   uint32_t signature_object_id,
                   uint8_t hash_algorithm);

/* 
 * Symmetric (AES) Cryptography
 */ 
extern tam_lib_status_t
tam_lib_sym_key_generate(void *tam_handle,
                   uint32_t session_id,
                   uint8_t key_length,
                   uint8_t iv_length,
                   uint8_t csp_flag,
                   uint8_t memory_flag,
                   uint32_t *sym_key_object_id);

extern tam_lib_status_t
tam_lib_sym_encr_init(void *tam_handle,
                   uint32_t session_id,
                   uint32_t sym_key_object_id,
                   uint8_t sym_algorithm,
                   uint8_t sym_mode,
                   uint32_t *sym_encrypt_context_id);

extern tam_lib_status_t
tam_lib_sym_encr_buffer(void *tam_handle,
                   uint32_t session_id,
                   uint32_t sym_encrypt_context_id,
                   uint16_t clear_text_buffer_length,
                   uint8_t *clear_text_buffer,
                   uint16_t *cypher_text_buffer_length,
                   uint8_t *cypher_text_buffer);

extern tam_lib_status_t
tam_lib_sym_encr_update(void *tam_handle,
                   uint32_t session_id,
                   uint32_t sym_encrypt_context_id,
                   uint16_t clear_text_buffer_length,
                   uint8_t *clear_text_buffer,
                   uint16_t *cypher_text_buffer_length,
                   uint8_t *cypher_text_buffer);

extern tam_lib_status_t
tam_lib_sym_encr_finalize(void *tam_handle,
                   uint32_t session_id,
                   uint32_t sym_encrypt_context_id,
                   uint16_t clear_text_buffer_length,
                   uint8_t *clear_text_buffer,
                   uint16_t *cypher_text_buffer_length,
                   uint8_t *cypher_text_buffer);

extern tam_lib_status_t
tam_lib_sym_encr_abort(void *tam_handle,
                   uint32_t session_id,
                   uint32_t sym_encrypt_context_id);


extern tam_lib_status_t
tam_lib_sym_decr_init(void *tam_handle,
                   uint32_t session_id,
                   uint32_t key_object_id,
                   uint8_t sym_algorithm,
                   uint8_t sym_mode,
                   uint32_t *sym_decrypt_context_id);

extern tam_lib_status_t
tam_lib_sym_decr_buffer(void *tam_handle,
                   uint32_t session_id,
                   uint32_t sym_decrypt_context_id,
                   uint16_t cypher_text_buffer_length,
                   uint8_t *cypher_text_buffer,
                   uint16_t *clear_text_buffer_length,
                   uint8_t *clear_text_buffer);

extern tam_lib_status_t
tam_lib_sym_decr_update(void *tam_handle,
                   uint32_t session_id,
                   uint32_t sym_decrypt_context_id,
                   uint16_t cypher_text_buffer_length,
                   uint8_t *cypher_text_buffer,
                   uint16_t *clear_text_buffer_length,
                   uint8_t *clear_text_buffer);

extern tam_lib_status_t
tam_lib_sym_decr_finalize(void *tam_handle,
                   uint32_t session_id,
                   uint32_t sym_decrypt_context_id,
                   uint16_t cypher_text_buffer_length,
                   uint8_t *cypher_text_buffer,
                   uint16_t *clear_text_buffer_length,
                   uint8_t *clear_text_buffer);

extern tam_lib_status_t
tam_lib_sym_decr_abort(void *tam_handle,
                   uint32_t session_id,
                   uint32_t sym_decrypt_context_id);



/* displays the TAM device context */
extern tam_lib_status_t
tam_lib_display_device_context(void *tam_handle);


/* admin logout */
extern tam_lib_status_t
tam_lib_admin_logout(void *tam_handle,
                   uint32_t admin_session_id);


/* admin log in */
extern tam_lib_status_t
tam_lib_admin_login(void *tam_handle,
                   uint32_t *admin_session_id);


/* close the device, no further access */
extern tam_lib_status_t
tam_lib_device_close(void **tam_handle);


/* open access to the tam device, returns the TAM handle */
extern tam_lib_status_t
tam_lib_device_open(void *platform_opaque_handle,
                   uint16_t platform_max_bus_length,
                   void **tam_handle);


/*
 * These two prototypes define the platform driver
 * dependencies.
 */

/* TAM write routine */
extern int32_t
tam_lib_platform_write(void *platform_opaque_handle,
                   uint8_t *send_buffer,
                   uint32_t length);

/* TAM read routine */
extern tam_lib_status_t
tam_lib_platform_read(void *platform_opaque_handle,
                   uint32_t min_time,
                   uint32_t max_time,
                   uint8_t *read_buffer,
                   uint16_t bytes_to_read,
                   uint16_t *bytes_actually_read);

/* TAM mailbox write routine */
extern int32_t
tam_lib_platform_mbx_write(void *platform_opaque_handle,
                           uint16_t bytes_to_send,
                           uint8_t *send_buffer);

/* TAM mailbox read routine */
extern tam_lib_status_t
tam_lib_platform_mbx_read(void *platform_opaque_handle,
                          uint16_t bytes_to_send,
                          uint8_t *send_buffer,
                          uint16_t bytes_to_read,
                          uint8_t *read_buffer,
                          uint16_t *bytes_actually_read);

/* TAM eSPI functions */
extern tam_lib_status_t
tam_lib_espi_get_mfg_id(void *tam_handle,
                        uint16_t *length,
                        uint8_t *data);

extern tam_lib_status_t
tam_lib_espi_get_dev_id(void *tam_handle,
                        uint16_t *length,
                        uint8_t *data);

extern tam_lib_status_t
tam_lib_espi_read(void *tam_handle,
                  uint32_t addr,
                  uint16_t length,
                  uint8_t *data);

extern tam_lib_status_t
tam_lib_espi_write(void *tam_handle,
                   uint32_t addr,
                   uint16_t length,
                   uint8_t *data);

extern tam_lib_status_t
tam_lib_espi_erase_64k(void *tam_handle,
                       uint32_t sector);

extern tam_lib_status_t
tam_lib_espi_erase_4k(void *tam_handle,
                      uint32_t sector);

extern tam_lib_status_t
tam_lib_espi_bitstream_upgrade(void *tam_handle,
                               uint8_t upgrade_type,
                               uint16_t data_len,
                               uint8_t *data,
                               uint32_t addr_offset);

extern tam_lib_status_t
tam_lib_espi_authenticate(void *tam_handle,
                          uint8_t image_type,
                          uint8_t *fpga_status,
                          uint8_t *fw_status);

extern tam_lib_status_t
tam_lib_soft_reset(void *tam_handle,
                   uint32_t param1,
                   uint32_t param2);

#endif

