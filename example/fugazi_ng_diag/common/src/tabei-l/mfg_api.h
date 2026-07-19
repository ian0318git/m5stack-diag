 /* $Id: mfg_api.h,v 1.2 2019/10/17 02:16:25 kehuang2 Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/tabei-l/mfg_api.h,v $
 *----------------------------------------------------------------------------
 * mfg_api.h  Support for ACT2/Ruby API code.
 *
 *
 * Copyright (c) 2013-2019 by Cisco Systems, Inc.
 * All rights reserved.
 *------------------------------------------------------------------
 */
#ifndef __MFG_API_H__
#define __MFG_API_H__


#include "object.h"

typedef struct { 
    p_u1 prodSnPtr;
    u1 prodSnLen;
    p_u1 prodNamePtr;
    u1 prodNameLen;
    p_u1 pidPtr;
    u1 pidLen;
}board_specs_t; 

extern board_specs_t board_specs;

uint GetSHA256Hash_openssl(PDATA_BLOCK input, u4 input_size, PDATA_BLOCK hash, 
p_u4 hash_size);

ACT2_STATUS act2l_get_chip_serial_number( void *module, OUT PDATA_BLOCK chip_serial_number);

/*Remove it in  final version*/
ACT2_STATUS act2l_get_cskm_package ( void *module, OUT p_u1 flag, OUT PDATA_BLOCK cskmp);

ACT2_STATUS act2l_switch_2_simple_mode ( void *module );

ACT2_STATUS act2l_init_manufacturing_login ( void *module, IN u2 certificate_chain_length,
                                            OUT PDATA_BLOCK nonce );
ACT2_STATUS act2l_finalize_manufacturing_login ( void *module, 
                                    IN u2 nonce_certificate_chain_length,
                                    IN PDATA_BLOCK nonce_certificate_chain, 
                                    IN PDATA_BLOCK concatenated_signature,
                                    OUT PSESSION_ID session_id );

ACT2_STATUS act2l_install_CLIIP ( void *module, IN SESSION_ID session_id,
                                 IN u2 cliip_length,
                                 IN PDATA_BLOCK cliip);

ACT2_STATUS act2l_install_SUDI  ( void *module, IN SESSION_ID session_id,
                                 IN u2 sudi_leaf_length,
                                 IN PDATA_BLOCK sudi_leaf,
                                 IN u2 sudi_ca_root_length,
                                 IN PDATA_BLOCK sudi_ca_root);

ACT2_STATUS act2l_close_manufacturing_login ( void *module,  IN SESSION_ID session_id);

ACT2_STATUS authenticateruby ( void *module,IN SESSION_ID session_id,
				IN PDATA_BLOCK challenge_nonce, 
				OUT PDATA_BLOCK password);


ACT2_STATUS act2l_create_master_key ( void *module, IN SESSION_ID session_id);

 
u1 create_CMS_SUDI_request( void *module, u4 sessionId, p_u1 *msgOutPtrPtr, p_u2 msgOutLen,
                            board_specs_t *sgbu_ptr);
ACT2_STATUS act2l_test_manufacturing_installation ( void *module, SESSION_ID
manuf_session_id);
ACT2_STATUS act2l_test_communication ( void *module );
ACT2_STATUS act2l_read_sudi ( void *module );

ACT2_STATUS act2l_gen_admin_credential (void*module, OUT PDATA_BLOCK password);


ACT2_STATUS secure_object_create(void *module, IN SESSION_ID session_id, 
				IN OBJECT_TYPE object_type, IN u2 object_size,
				OUT u1 csp_flag, OUT u1 memory_flag, 
				OUT u1 encryption_flag, OUT POBJECT_ID object_id);

ACT2_STATUS secure_object_write( void *module, IN SESSION_ID session_id, 
				IN OBJECT_ID object_id,  
				IN u2 total_remaining, 
				IN u1 length,
				IN CLEAR_TEXT clear_text);

ACT2_STATUS secure_object_read( void *module, IN SESSION_ID session_id, 
				IN OBJECT_ID source_object, 
				OUT p_u2 total_remaining, 
				OUT p_u1 length,
				OUT CLEAR_TEXT clear_text);
ACT2_STATUS act2_write_object( void *module, SESSION_ID session_id, OBJECT_ID object_id, p_u1 src_buffer, u2 src_length);

ACT2_STATUS act2_read_object( void *module, SESSION_ID session_id, OBJECT_ID object_id, p_u1 dst_buffer, const u2 buffer_size);

ACT2_STATUS secure_object_delete( void *module, IN SESSION_ID session_id, IN OBJECT_ID object_handle);

ACT2_STATUS act2_session_init (void *module, IN USER_ID user_id, IN PAUTHENTICATION_CREDENTIAL auth_cred, OUT PSESSION_ID session_id);

ACT2_STATUS act2_terminate_session(void *module, IN SESSION_ID session_id);				
ACT2_STATUS act2_get_random_number(void *module, u4, p_u1, u4);

#endif
                          
/*-------------------------------------------------
 * $Log: mfg_api.h,v $
 * Revision 1.2  2019/10/17 02:16:25  kehuang2
 * Collapse Tabei-L into main trunk
 *
 * Revision 1.1.2.2  2018/11/02 02:39:03  kodko
 * Support cookie read for NIM and PIM modules.
 *
 * Revision 1.1.2.1  2018/10/02 01:50:03  harrchan
 * Initial commit for Tabei-L P1A bring up.
 *
 * $Endlog$
 *-------------------------------------------------
 */
