/* $Id: object.h,v 1.2 2017/08/02 14:21:47 steja Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/tsn/object.h,v $
 *----------------------------------------------------------------------------
 * object.h.h  Support for ACT2/Ruby API code.
 *
 * May 2012: 
 *
 * Copyright (c) 2013-2017 by Cisco Systems, Inc.
 * All rights reserved.
 *------------------------------------------------------------------
 */

#ifndef __OBJECT_H__
#define __OBJECT_H__

#define OBJECT_ID			u4
#define POBJECT_ID			p_u4
#define OBJECT_TYPE			u1
#define POBJECT_TYPE		p_u1
#define CLEAR_TEXT			p_u1

/* #pragma pack (push, 1) */
typedef struct {
    OBJECT_TYPE object_type;
    u1 rom_flag;
    u1 flags;
    u2 size;
    u2 length;
    u2 read_permissions;
    u2 write_permissions;
    u2 use_permissions;
} OBJECT_ATTRIBUTES, *POBJECT_ATTRIBUTES;

typedef struct {
    OBJECT_ID object_id;
    USER_ID object_owner;
    OBJECT_ATTRIBUTES attributes;
    p_u1 data;
} ACT2_OBJECT, *ACT2_OBJECT_PTR;


typedef struct {
    USER_ID user_id;
    u4 mem_used;
} ACT2_USER_INFO, *ACT2_USER_INFO_PTR;
/* #pragma pack(pop)*/

typedef enum {
    INVALID_OBJECT,             /* initial */
    RAW_OBJECT,                 /* formless */
    AES_KEYIV_OBJECT,           /* AES Key */
    RSA_KEYPAIR_OBJECT,         /* RSA key pair */
    x509_CERT_OBJECT,           /* RSA X.509 Certificate */
    x509_CERTCHAIN_OBJECT       /* RSA X.509 Cert Chain */
} ACT2_OBJECT_TYPE;

typedef struct {
    OBJECT_ID object_id;
    u1 object_type;
} ACT2_OBJECT_ENUM;

#define FLAG_ENCR       0x01    /* In the attributes flags */
#define FLAG_CSP        0x02    /* In the attributes flags */

/*??? Enums instead*/
#define SOURCE_COPY    (u1) 0
#define DEST_ENCRYPTED (u1) 1
#define RAM (u1) 0
#define ROM (u1) 1
#define NOT_CSP (u1) 0
#define CSP (u1) 1

/*Predefined Objects*/
#define NULL_HANDLE             (OBJECT_ID)	0x00000000
#define I2C_INPUT               (OBJECT_ID) 0x00000001
#define I2C_OUTPUT              (OBJECT_ID) 0x00000002
#define CLII_KEY_PAIR           (OBJECT_ID) 0x00000010
#define CLII_CERTIFICATE        (OBJECT_ID) 0x00000013
#define CLII_CA_CERT_CHAIN      (OBJECT_ID) 0x00000014
#define IDEVID_KEY_PAIR         (OBJECT_ID) 0x00000018
#define IDEVID_CERTIFICATE      (OBJECT_ID) 0x0000001B
#define IDEVID_CA_CERT_CHAIN    (OBJECT_ID) 0x0000001C

OBJECT_ID act2_test_create_object(void *module, SESSION_ID session_id,
                                  POBJECT_ATTRIBUTES obj_attributes,
                                  ACT2_STATUS expected_result);
OBJECT_ID act2_create_object(void *module, SESSION_ID session_id,
                             POBJECT_ATTRIBUTES obj_attributes);
ACT2_STATUS act2_write_object(void *module, SESSION_ID session_id,
                              OBJECT_ID object_id, p_u1 src_buffer,
                              u2 src_length);
ACT2_STATUS act2_read_object(void *module, SESSION_ID session_id,
                             OBJECT_ID object_id, p_u1 dst_buffer,
                             const u2 buffer_size);
u2 act2_get_object_length(void *module, SESSION_ID session_id,
                          OBJECT_ID object_id);
ACT2_STATUS secure_object_enumerate(void *module, IN SESSION_ID session_id,
                                    IN p_u1 num_objects,
                                    OUT ACT2_OBJECT_ENUM * object_list);
#endif


/*************************************************************
$Log: object.h,v $
Revision 1.2  2017/08/02 14:21:47  steja
Support TSN-H/M platform code

Revision 1.1.8.2  2017/07/29 03:41:19  steja
tsn-branch5 synced with Maintrunk repositories

Revision 1.1.6.2  2017/07/20 13:38:06  steja
tsn-branch4 merge with maintrunk

Revision 1.1.4.2  2016/06/30 06:22:49  steja
tsn-branch2 sync with main trunk

Revision 1.1.2.1  2016/03/08 09:55:11  steja
Initial Check-in


$Endlog$
*/
