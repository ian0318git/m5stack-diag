
#ifndef __TAM_LIB_MANUFACTURING_H__
#define __TAM_LIB_MANUFACTURING_H__

#include <stdio.h>
#include <stdlib.h>

#include "tam_library.h"


/* 
 * manufacturing user ID
 */ 
#define TAM_MFG_USER                  ( 0x00 )

#define MFG_NONCE_SIZE                 ( 0x20 )

#define HALF_OF_ECSKMPACKAGE_LENGTH    ( 128 )

#define TAM_MAX_CREDENT_LENGTH        ( 240 )
 

/*
 * Bit Map for POST Test Result
 */
#define TAM_POST_TEST_OK              ( 0x00 )
#define TAM_POST_RAM_TEST_FAILED      ( 0x01 )
#define TAM_POST_MICROCODE_CKS_FAILED ( 0x02 )
#define TAM_POST_KEY_SIG_CKS_FAILED   ( 0x04 )
#define TAM_POST_CHIP_VER_FAILED      ( 0x08 )
#define TAM_POST_OTP_FAILED           ( 0x10 )


/*
 * tam_lib_mfg_get_version()
 */
typedef struct {
    uint16_t major;
    uint16_t minor;
    uint16_t patch;
} tam_lib_mfg_version_t;


/*
 * error info made available to facilitate error
 * feedback to the user. 
 */ 
typedef struct {
    char      function[64];
    uint32_t  mfg_session_id;
    uint32_t  object_id;
    uint8_t   fw_opcode;
    uint16_t  length;
    uint16_t  loop_iteration;
    uint16_t  write_total_remaining;
    uint8_t   write_obj_size;
    uint8_t   status;
} tam_lib_mfg_errinfo_t;


extern tam_lib_mfg_errinfo_t tam_lib_mfg_errinfo;


typedef struct {
    char *serial_number_ptr;
    uint8_t serial_number_length;
    char *product_name_ptr;
    uint8_t product_name_length;
    char *pid_ptr;
    uint8_t pid_length;
    char *sudi_key_type_ptr;
    uint8_t sudi_key_type_length;
} sudi_info_t;


/* API to clear the error info - mostly for testing */
extern void
tam_lib_mfg_errinfo_clear(void);

/* API to print the error info */
extern void
tam_lib_mfg_errinfo_print(void);

/* API to get a copy of the error info */
extern tam_lib_status_t
tam_lib_mfg_errinfo_get(tam_lib_mfg_errinfo_t *errinfo);
 


/* 
 * Manufacturing APIs
 */   
extern void 
tam_lib_mfg_display_notice(void);

extern tam_lib_status_t
tam_lib_mfg_factory_fresh(void *tam_handle);

extern tam_lib_status_t
tam_lib_mfg_ecskmp_generate(void *tam_handle,
                            uint8_t *already_clliped);

extern tam_lib_status_t
tam_lib_mfg_ecskmp_read(void *tam_handle,
                          uint8_t *buffer,
                          uint16_t *length); 

extern tam_lib_status_t
tam_lib_mfg_ecskmp_parse(uint8_t *cskmp_buffer,
                          uint8_t *chip_serial_number_cskmp,
                          uint8_t *rsa_crypt_aes_key,
                          uint8_t *rsa_crypt_hmac_key,
                          uint8_t *rsa_crypt_packet); 

extern tam_lib_status_t
tam_lib_mfg_login_init(void *tam_handle,
                          uint16_t credentials_length,
                          uint8_t *mfg_nonce); 

extern tam_lib_status_t
tam_lib_mfg_login_credentials(void *tam_handle,
                          uint16_t length,
                          uint8_t *nonce_credentials);

extern tam_lib_status_t
tam_lib_mfg_login_signature(void *tam_handle,
                          uint16_t length,
                          uint8_t *signature,
                          uint32_t *session_id);

extern tam_lib_status_t
tam_lib_mfg_cliip_install(void *tam_handle,
                          uint32_t mfg_session_id,
                          uint16_t length,
                          uint8_t *cliip_data);

extern tam_lib_status_t
tam_lib_mfg_create_sudi_request(void *tam_handle,
                          uint32_t session_id,
                          sudi_info_t *sudi_info,
                          uint8_t **sudi_request,
                          uint16_t *request_length);

extern tam_lib_status_t
tam_lib_mfg_install_cert_and_chain(void *tam_handle,
                          uint32_t session_id,
                          uint16_t ca_len,
                          uint8_t *ca_data,
                          uint16_t leaf_len,
                          uint8_t *leaf_data,
                          uint8_t cert_type);

extern tam_lib_status_t
tam_lib_mfg_session_end(void *tam_handle,
                          uint32_t session_id);

#endif
