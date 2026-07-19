/* $Id: bsafe.h,v 1.2 2012/03/28 00:38:09 mcharon Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/include/bsafe.h,v $
 *------------------------------------------------------------------
 * bsafe.h  - Ported from IOS for Quack.
 *
 * Copyright (c) 2003-2012 by cisco Systems, Inc.
 * All rights reserved.
 *
 * Author: Huan Ngo
 *------------------------------------------------------------------
 */


/* Copyright (c) RSA Security Inc., 1990-1999.  All rights reserved.  
 * This work contains proprietary, confidential, and trade secret 
 * information of RSA Security Inc.  Use, disclosure or reproduction 
 * without the express written authorization of RSA Security Inc. is
 * prohibited.
 */

#ifndef _BSAFE_H_
#define _BSAFE_H_ 1

#include "bsfmacro.h"
#include "bsfplatf.h"

#include "aglobal.h"
#include "atypes.h"
#include "bexterr.h"



#define RSA_STD_ALLOC_FUNCS  RSA_ENABLED
#define RSA_STD_MEM_FUNCS    RSA_ENABLED
#define RSA_STD_STRING_FUNCS RSA_ENABLED
/* pritikin - added the rest of the RSA STD FUNC defines */
#define RSA_STD_TIME_FUNCS   RSA_ENABLED
#include "stdlibrf.h"

#ifdef __cplusplus
extern "C" {
#endif

#define BE_ALGORITHM_ALREADY_SET 0x0200
#define BE_ALGORITHM_INFO 0x0201
#define BE_ALGORITHM_NOT_INITIALIZED 0x0202
#define BE_ALGORITHM_NOT_SET 0x0203
#define BE_ALGORITHM_OBJ 0x0204
#define BE_ALG_OPERATION_UNKNOWN 0x0205
#define BE_ALLOC 0x0206
#define BE_CANCEL 0x0207
#define BE_DATA 0x0208
#define BE_EXPONENT_EVEN 0x0209
#define BE_EXPONENT_LEN 0x020a
#define BE_HARDWARE 0x020b
#define BE_INPUT_DATA 0x020c
#define BE_INPUT_LEN 0x020d
#define BE_KEY_ALREADY_SET 0x020e
#define BE_KEY_INFO 0x020f
#define BE_KEY_LEN 0x0210
#define BE_KEY_NOT_SET 0x0211
#define BE_KEY_OBJ 0x0212
#define BE_KEY_OPERATION_UNKNOWN 0x0213
#define BE_MEMORY_OBJ 0x0214
#define BE_MODULUS_LEN 0x0215
#define BE_NOT_INITIALIZED 0x0216
#define BE_NOT_SUPPORTED 0x0217
#define BE_OUTPUT_LEN 0x0218
#define BE_OVER_32K 0x0219
#define BE_RANDOM_NOT_INITIALIZED 0x021a
#define BE_RANDOM_OBJ 0x021b
#define BE_SIGNATURE 0x021c
#define BE_WRONG_ALGORITHM_INFO 0x021d
#define BE_WRONG_KEY_INFO 0x021e
#define BE_INPUT_COUNT 0x021f
#define BE_OUTPUT_COUNT 0x0220
#define BE_METHOD_NOT_IN_CHOOSER 0x221
#define BE_KEY_WEAK 0x222
#define BE_EXPONENT_ONE 0x0223
#define BE_BAD_POINTER 0x0224	
#define BE_BAD_PASSPHRASE 0x0225
#define BE_AM_DOMESTIC_ONLY 0x226
#define BE_BAD_SEEDING 0x227	

typedef POINTER B_KEY_OBJ;
typedef POINTER B_ALGORITHM_OBJ;

typedef int (RSA_CALLING_CONV *B_INFO_TYPE) PROTO_LIST ((POINTER *));

typedef struct B_ExtendedMethod B_EXT_ALGORITHM_METHOD;

#ifndef _BUILD_LIBRARY_
typedef char B_ALGORITHM_METHOD;
typedef B_ALGORITHM_METHOD **B_ALGORITHM_CHOOSER;
#endif

#if RSA_PLATFORM == RSA_I386_486
#ifndef _HW_TABLE_ENTRY_DEFINITION_
typedef char HW_TABLE_ENTRY;
typedef HW_TABLE_ENTRY **HW_TABLE_LIST;
#endif
extern HW_TABLE_ENTRY HW_INTEL_RANDOM;
#endif

/* Version information.
 */
extern char * RSA_CALLING_CONV BSAFE_VERSION;

/* The data profiling object */
int RSA_CALLING_CONV B_BuildTableInit PROTO_LIST
  ((B_ALGORITHM_OBJ, B_ALGORITHM_CHOOSER, A_SURRENDER_CTX *));
int RSA_CALLING_CONV B_BuildTableUpdate PROTO_LIST
  ((B_ALGORITHM_OBJ, unsigned char *, unsigned int, A_SURRENDER_CTX *));
int RSA_CALLING_CONV B_BuildTableGetBufSize PROTO_LIST
  ((B_ALGORITHM_OBJ, unsigned int *));
int RSA_CALLING_CONV B_BuildTableFinal PROTO_LIST
  ((B_ALGORITHM_OBJ, unsigned char *, unsigned int *, unsigned int,
    A_SURRENDER_CTX *));
/* The compression object */
int RSA_CALLING_CONV B_CompressInit PROTO_LIST
  ((B_ALGORITHM_OBJ, B_ALGORITHM_CHOOSER, A_SURRENDER_CTX *));
int RSA_CALLING_CONV B_CompressUpdate PROTO_LIST
  ((B_ALGORITHM_OBJ, unsigned char *, unsigned int *, unsigned int,
    unsigned char *, unsigned int, unsigned int*, A_SURRENDER_CTX *));
int RSA_CALLING_CONV B_CompressFinal PROTO_LIST
  ((B_ALGORITHM_OBJ, unsigned char *, unsigned int *, unsigned int,
    A_SURRENDER_CTX *));
int RSA_CALLING_CONV B_DecompressInit PROTO_LIST
  ((B_ALGORITHM_OBJ, B_ALGORITHM_CHOOSER, A_SURRENDER_CTX *));
int RSA_CALLING_CONV B_DecompressUpdate PROTO_LIST
  ((B_ALGORITHM_OBJ, unsigned char *, unsigned int *, unsigned int,
    unsigned char *, unsigned int, unsigned int *, A_SURRENDER_CTX *));
int RSA_CALLING_CONV B_DecompressFinal PROTO_LIST
  ((B_ALGORITHM_OBJ, unsigned char *, unsigned int *, unsigned int,
    A_SURRENDER_CTX *));

/* The key object.
 */
int RSA_CALLING_CONV B_CreateKeyObject PROTO_LIST ((B_KEY_OBJ *));
void RSA_CALLING_CONV B_DestroyKeyObject PROTO_LIST ((B_KEY_OBJ *));
int RSA_CALLING_CONV B_SetKeyInfo PROTO_LIST
  ((B_KEY_OBJ, B_INFO_TYPE, POINTER));
void RSA_CALLING_CONV B_GetKeyExtendedErrorInfo PROTO_LIST
  ((B_KEY_OBJ, ITEM *, POINTER *));
int RSA_CALLING_CONV B_GetKeyInfo PROTO_LIST
  ((POINTER *, B_KEY_OBJ, B_INFO_TYPE));

/* The algorithm object.
 */
int RSA_CALLING_CONV B_CreateAlgorithmObject PROTO_LIST ((B_ALGORITHM_OBJ *));
void RSA_CALLING_CONV B_DestroyAlgorithmObject PROTO_LIST
  ((B_ALGORITHM_OBJ *));
int RSA_CALLING_CONV B_SetAlgorithmInfo PROTO_LIST
  ((B_ALGORITHM_OBJ, B_INFO_TYPE, POINTER));
int RSA_CALLING_CONV B_GetAlgorithmInfo PROTO_LIST
  ((POINTER *, B_ALGORITHM_OBJ, B_INFO_TYPE));
int RSA_CALLING_CONV B_GetAlgorithmState PROTO_LIST
   ((ITEM *, B_ALGORITHM_OBJ));
int RSA_CALLING_CONV B_SetAlgorithmState PROTO_LIST
   ((B_ALGORITHM_OBJ, B_INFO_TYPE, ITEM *, B_ALGORITHM_CHOOSER));
void RSA_CALLING_CONV B_GetExtendedErrorInfo PROTO_LIST
   ((B_ALGORITHM_OBJ, ITEM *, POINTER *));
unsigned int B_IntegerBits PROTO_LIST ((unsigned char *, unsigned int));

/* Algorithm operations.
 */
int RSA_CALLING_CONV B_RandomInit PROTO_LIST
  ((B_ALGORITHM_OBJ, B_ALGORITHM_CHOOSER, A_SURRENDER_CTX *));
int RSA_CALLING_CONV B_RandomUpdate PROTO_LIST
  ((B_ALGORITHM_OBJ, unsigned char *, unsigned int, A_SURRENDER_CTX *));
int RSA_CALLING_CONV B_GenerateRandomBytes PROTO_LIST
  ((B_ALGORITHM_OBJ, unsigned char *, unsigned int, A_SURRENDER_CTX *));

int RSA_CALLING_CONV B_DigestInit PROTO_LIST
  ((B_ALGORITHM_OBJ, B_KEY_OBJ, B_ALGORITHM_CHOOSER, A_SURRENDER_CTX *));
int RSA_CALLING_CONV B_DigestUpdate PROTO_LIST
  ((B_ALGORITHM_OBJ, unsigned char *, unsigned int, A_SURRENDER_CTX *));
int RSA_CALLING_CONV B_DigestFinal PROTO_LIST
  ((B_ALGORITHM_OBJ, unsigned char *, unsigned int *, unsigned int,
    A_SURRENDER_CTX *));

int RSA_CALLING_CONV B_EncryptInit PROTO_LIST
  ((B_ALGORITHM_OBJ, B_KEY_OBJ, B_ALGORITHM_CHOOSER, A_SURRENDER_CTX *));
int RSA_CALLING_CONV B_EncryptUpdate PROTO_LIST
  ((B_ALGORITHM_OBJ, unsigned char *, unsigned int *, unsigned int,
    unsigned char *, unsigned int, B_ALGORITHM_OBJ, A_SURRENDER_CTX *));
int RSA_CALLING_CONV B_EncryptFinal PROTO_LIST
  ((B_ALGORITHM_OBJ, unsigned char *, unsigned int *, unsigned int,
    B_ALGORITHM_OBJ, A_SURRENDER_CTX *));

int RSA_CALLING_CONV B_DecryptInit PROTO_LIST
  ((B_ALGORITHM_OBJ, B_KEY_OBJ, B_ALGORITHM_CHOOSER, A_SURRENDER_CTX *));
int RSA_CALLING_CONV B_DecryptUpdate PROTO_LIST
  ((B_ALGORITHM_OBJ, unsigned char *, unsigned int *, unsigned int,
    unsigned char *, unsigned int, B_ALGORITHM_OBJ, A_SURRENDER_CTX *));
int RSA_CALLING_CONV B_DecryptFinal PROTO_LIST
  ((B_ALGORITHM_OBJ, unsigned char *, unsigned int *, unsigned int,
    B_ALGORITHM_OBJ, A_SURRENDER_CTX *));

int RSA_CALLING_CONV B_EncodeInit PROTO_LIST ((B_ALGORITHM_OBJ));
int RSA_CALLING_CONV B_EncodeUpdate PROTO_LIST
  ((B_ALGORITHM_OBJ, unsigned char *, unsigned int *, unsigned int,
    unsigned char *, unsigned int));
int RSA_CALLING_CONV B_EncodeFinal PROTO_LIST
  ((B_ALGORITHM_OBJ, unsigned char *, unsigned int *, unsigned int));

int RSA_CALLING_CONV B_DecodeInit PROTO_LIST ((B_ALGORITHM_OBJ));
int RSA_CALLING_CONV B_DecodeUpdate PROTO_LIST
  ((B_ALGORITHM_OBJ, unsigned char *, unsigned int *, unsigned int,
    unsigned char *, unsigned int));
int RSA_CALLING_CONV B_DecodeFinal PROTO_LIST
  ((B_ALGORITHM_OBJ, unsigned char *, unsigned int *, unsigned int));
int B_OpenEnvelopeInit PROTO_LIST 
  ((B_ALGORITHM_OBJ, unsigned int *, ITEM *, B_KEY_OBJ, B_ALGORITHM_CHOOSER,
    A_SURRENDER_CTX *));

int RSA_CALLING_CONV B_OpenEnvelope PROTO_LIST
  ((B_ALGORITHM_OBJ, ITEM *, unsigned int, ITEM *, B_KEY_OBJ, 
    A_SURRENDER_CTX *));
int RSA_CALLING_CONV B_SealEnvelope PROTO_LIST
  ((B_ALGORITHM_OBJ, ITEM *, unsigned int, ITEM *, B_KEY_OBJ, 
    B_ALGORITHM_OBJ, A_SURRENDER_CTX *));
int RSA_CALLING_CONV B_SealEnvelopeInit PROTO_LIST
   ((B_ALGORITHM_OBJ, unsigned int *, ITEM *, B_KEY_OBJ, B_KEY_OBJ,
    B_ALGORITHM_CHOOSER, A_SURRENDER_CTX *));
int RSA_CALLING_CONV B_SignInit PROTO_LIST
  ((B_ALGORITHM_OBJ, B_KEY_OBJ, B_ALGORITHM_CHOOSER, A_SURRENDER_CTX *));
int RSA_CALLING_CONV B_SignUpdate PROTO_LIST
  ((B_ALGORITHM_OBJ, unsigned char *, unsigned int, A_SURRENDER_CTX *));
int RSA_CALLING_CONV B_SignFinal PROTO_LIST
  ((B_ALGORITHM_OBJ, unsigned char *, unsigned int *, unsigned int,
    B_ALGORITHM_OBJ, A_SURRENDER_CTX *));

int RSA_CALLING_CONV B_VerifyInit PROTO_LIST
  ((B_ALGORITHM_OBJ, B_KEY_OBJ, B_ALGORITHM_CHOOSER, A_SURRENDER_CTX *));
int RSA_CALLING_CONV B_VerifyUpdate PROTO_LIST
  ((B_ALGORITHM_OBJ, unsigned char *, unsigned int, A_SURRENDER_CTX *));
int RSA_CALLING_CONV B_VerifyFinal PROTO_LIST
  ((B_ALGORITHM_OBJ, unsigned char *, unsigned int, B_ALGORITHM_OBJ,
    A_SURRENDER_CTX *));

int RSA_CALLING_CONV B_KeyAgreeInit PROTO_LIST
  ((B_ALGORITHM_OBJ, B_KEY_OBJ, B_ALGORITHM_CHOOSER, A_SURRENDER_CTX *));
int RSA_CALLING_CONV B_KeyAgreePhase1 PROTO_LIST
  ((B_ALGORITHM_OBJ, unsigned char *, unsigned int *, unsigned int,
    B_ALGORITHM_OBJ, A_SURRENDER_CTX *));
int RSA_CALLING_CONV B_KeyAgreePhase2 PROTO_LIST
  ((B_ALGORITHM_OBJ, unsigned char *, unsigned int *, unsigned int,
    unsigned char *, unsigned int, A_SURRENDER_CTX *));
 
int RSA_CALLING_CONV B_GenerateInit PROTO_LIST
  ((B_ALGORITHM_OBJ, B_ALGORITHM_CHOOSER, A_SURRENDER_CTX *));
int RSA_CALLING_CONV B_GenerateKeypair PROTO_LIST
  ((B_ALGORITHM_OBJ, B_KEY_OBJ, B_KEY_OBJ, B_ALGORITHM_OBJ,
    A_SURRENDER_CTX *));
int RSA_CALLING_CONV B_GenerateParameters PROTO_LIST
  ((B_ALGORITHM_OBJ, B_ALGORITHM_OBJ, B_ALGORITHM_OBJ, A_SURRENDER_CTX *));


int RSA_CALLING_CONV B_SymmetricKeyGenerateInit PROTO_LIST
  ((B_ALGORITHM_OBJ, B_ALGORITHM_CHOOSER, A_SURRENDER_CTX *));
int RSA_CALLING_CONV B_SymmetricKeyGenerate PROTO_LIST
  ((B_ALGORITHM_OBJ, B_KEY_OBJ, B_ALGORITHM_OBJ,
    A_SURRENDER_CTX *));

int B_CreateSessionChooser PROTO_LIST
  ((B_ALGORITHM_CHOOSER, B_ALGORITHM_CHOOSER *, POINTER *, ITEM *, POINTER *,
    unsigned char ***));
int B_FreeSessionChooser PROTO_LIST
  ((B_ALGORITHM_CHOOSER *, unsigned char ***));

int RSA_CALLING_CONV B_CreateHardwareChooser PROTO_LIST
  ((B_ALGORITHM_CHOOSER currentChooser,
    B_ALGORITHM_CHOOSER *newChooser, 
    B_ALGORITHM_CHOOSER swReplacements, 
    B_INFO_TYPE hwInfoType, 
    POINTER info));
void RSA_CALLING_CONV B_DestroyHardwareChooser PROTO_LIST
  ((B_ALGORITHM_CHOOSER *chooser));
int RSA_CALLING_CONV B_GetDevice PROTO_LIST
   ((ITEM *device,
     B_ALGORITHM_OBJ algorithmObject));

/* Information for BSAFE 1.x-compatible encryption algorithms.
 */
#define B_BSAFE1_PAD 1
#define B_BSAFE1_PAD_CHECKSUM 2
#define B_BSAFE1_RAW 3

typedef struct {
  int encryptionType;                /* encryption type: B_BSAFE1_PAD, etc. */
} B_BSAFE1_ENCRYPTION_PARAMS;

typedef struct {
  unsigned char *key;                              /* pointer to 8-byte key */
  unsigned int effectiveKeyBits;                     /* effective key length */
} B_RC2_BSAFE1_PARAMS_KEY;

/* Information for password-based encryption (PBE) algorithms.
 */
typedef struct {
  unsigned char *salt;                                        /* salt value */
  unsigned int iterationCount;                           /* iteration count */
} B_PBE_PARAMS;

typedef struct {
  unsigned int effectiveKeyBits;                     /* effective key length */
  unsigned char *salt;                                        /* salt value */
  unsigned int iterationCount;                           /* iteration count */
} B_RC2_PBE_PARAMS;

typedef struct {
  B_INFO_TYPE cryptInfoType;
  ITEM *cryptParams;
  B_INFO_TYPE formatInfoType;
  ITEM *formatParams;
} B_SEAL_OPEN_PARAMS;

typedef struct {
  ITEM ivItem;
  unsigned int transferSize;  
} B_CFB_PARAMS;

/* Information used to specify a block cipher with feedback */

typedef struct {
  unsigned char *encryptionMethodName;
  POINTER encryptionParams; /* Could be rc5 params for example */
  unsigned char *feedbackMethodName;
  POINTER feedbackParams; /* Might be just a pointer to an ITEM for a iv */
  unsigned char *paddingMethodName;
  POINTER paddingParams;
} B_BLK_CIPHER_W_FEEDBACK_PARAMS;


/* Information used to specify signing or verifying */
typedef struct {
  unsigned char *encryptionMethodName;
  POINTER encryptionParams;			/* Most likely to be null */
  unsigned char *digestMethodName;
  POINTER digestParams;		 		/* Most likely to be null */
  unsigned char *formatMethodName;
  POINTER formatParams;			
} B_SIGN_VERIFY_PARAMS;


/* Info used to specify key pair tokens at generation time */
typedef struct {
  POINTER keyParameters;
  UINT4 privateKeyUsage;
  UINT4 publicKeyUsage;
  unsigned int protectFlag;
  unsigned long publicLifeTime;
  unsigned long privateLifeTime;	
} B_TOKEN_KEYPAIR_GEN_INFO;

typedef struct {
  UINT4 tokenFlag;
  UINT4 keyUsage;
  RSA_TIME_T start;
  RSA_TIME_T end;
} B_KEY_ATTRIBUTES;

typedef struct {
  B_KEY_ATTRIBUTES privateKeyAttributes;
  B_KEY_ATTRIBUTES publicKeyAttributes;
  B_INFO_TYPE keypairGenInfoType;
  POINTER keypairGenInfo;
} B_KEYPAIR_GEN_PARAMS;

/* Information contained in key token types */
typedef struct {
  ITEM manufacturerId;
  ITEM internalKey;
} KI_TOKEN_INFO;

typedef struct {
  KI_TOKEN_INFO keyDataStruct;
  A_X509_ATTRIB_INFO attributes;
} KI_EXTENDED_TOKEN_INFO;

typedef struct {
  KI_TOKEN_INFO keyDataStruct;
  A_X509_KEYPAIR_ATTRIB_INFO attributes;
} KI_KEYPAIR_TOKEN_INFO;


/* Information for MAC algorithm.
 */
typedef struct {
  unsigned int macLen;                               /* length of MAC value */
} B_MAC_PARAMS;

/* Information for RC4 with MAC algorithm.
 */
typedef struct {
  ITEM salt;                                         /* variable-length salt */
  unsigned int macLen;                        /* length to use for MAC value */
} B_RC4_WITH_MAC_PARAMS;

/* Information for DSA parameter generation
 */
typedef struct {
  unsigned int primeBits;
} B_DSA_PARAM_GEN_PARAMS;

/* Information for EC parameter generation
 */
typedef struct {
  unsigned int version;
  unsigned int fieldType;	  /* base field for elliptic curve           */
  unsigned int fieldElementBits;  /* length of field element in bits         */
  unsigned int pointRepresentation; /* ignored for now 			     */
  unsigned int minOrderBits;	  /* minimum size of group generated by base */
                              /* input of 0 defaults to fieldElementBits - 7 */
  unsigned int trialDivBound;     /* maximum size of second largest prime    */
				  /* subgroup of group generated by base     */
				  /* input of 0 defaults to 255		     */
  unsigned int tableLookup;      /* relevant only to even field case. Set if */
			         /* the use of precomputed params is desired */
} B_EC_PARAM_GEN_PARAMS;

/* How EC parameters are passed.
 */
typedef struct {
  B_INFO_TYPE parameterInfoType;
  POINTER parameterInfoValue;
} B_EC_PARAMS;

/* How a digest algorithm is specified in a signature scheme.
 */
typedef struct {
  B_INFO_TYPE digestInfoType;
  POINTER digestInfoParams;
} B_DIGEST_SPECIFIER;

/* Information for Secret sharing algorithm
 */
typedef struct {
  unsigned int threshold;                                 /* share threshold */
} B_SECRET_SHARING_PARAMS;


/* Information for PKCS V2 OAEP Algorithm
*/
typedef struct {

/* hashFunc may contain a NULL_PTR or a pointer to the 
  null-terminated ASCII string, "sha1".  
  In both cases SHA1 will become the digest function. 
*/
char*			hashFunc; 
ITEM 			hashFuncParams;

/*  maskGenFunc may contain a NULL_PTR or a pointer to the 
    null-terminated ASCII string, "mgf1".  In both cases MGF1 
	will become the Mask Generator Function. 
*/
unsigned char* 	maskGenFunc;
ITEM 			maskGenFuncParams;

/* maskGenFuncUnderlyingAlg may contain a NULL_PTR or a pointer 
   to the null-terminated ASCII string, "sha1".  In both cases 
   SHA1 will become the underlying algorithm. 
*/
char* 			maskGenFuncUnderlyingAlg;
ITEM 			maskGenFuncUnderlyingAlgParams;

/* pSourceFunc is the method for determining the parameters, P.  
   pSourceFunc may contain a NULL_PTR or a pointer to the 
   null-terminated ASCII string, "specified parameters".  
   In  both cases "specified parameters" will become the 
   pSource method.  

If pSourceFunc is "specified parameters" then pSourceParams 
may be specified in two ways:

1. As a NULL_PTR.  		P is then assumed to be empty. 
2. As an ITEM 			ITEM contains the specified value for P.
*/
 
char* 			pSourceFunc; 
ITEM			pSourceParams;

} A_PKCS_OAEP_PARAMS;

typedef struct {
  char year[4];
  char month[2];
  char day[2];
} B_DATE;

/* Information to set up a chooser for use with a PKCS #11 device
 * when the caller wants to use a pre-established session.
 */
typedef struct {
  unsigned long int sessionHandle; /* in PKCS 11, a session handle is a
                           CK_SESSION_HANDLE, which is an unsigned long int */
  POINTER cryptokiFunctions;                /* A Cryptoki C_FunctionListPtr */
  char *libraryName;     /* The name of the shared library (DLL, .so, etc.) */
  ITEM tokenLabel;            /* user-defined label to differentiate tokens */
  ITEM passPhrase;             /* Possibly needed to log in into the device */ 
  A_SURRENDER_CTX *surrenderContext;       /* Install when creating session */
} B_PKCS11_SESSION;

/* Key Info Types.
 */
int RSA_CALLING_CONV KI_8Byte PROTO_LIST ((POINTER *));
int RSA_CALLING_CONV KI_16Byte PROTO_LIST ((POINTER *));
int RSA_CALLING_CONV KI_24Byte PROTO_LIST ((POINTER *));
int RSA_CALLING_CONV KI_32Byte PROTO_LIST ((POINTER *));
int RSA_CALLING_CONV KI_Token PROTO_LIST ((POINTER *));
int RSA_CALLING_CONV KI_ExtendedToken PROTO_LIST ((POINTER *));
int RSA_CALLING_CONV KI_DES8 PROTO_LIST ((POINTER *));
int RSA_CALLING_CONV KI_DES8Strong PROTO_LIST ((POINTER *));
int RSA_CALLING_CONV KI_DES24Strong PROTO_LIST ((POINTER *));
int RSA_CALLING_CONV KI_DESX PROTO_LIST ((POINTER *));
int RSA_CALLING_CONV KI_DSAPrivate PROTO_LIST ((POINTER *));
int RSA_CALLING_CONV KI_DSAPrivateBER PROTO_LIST ((POINTER *));
int RSA_CALLING_CONV KI_DSAPrivateX957BER PROTO_LIST ((POINTER *));
int RSA_CALLING_CONV KI_DSAPublic PROTO_LIST ((POINTER *));
int RSA_CALLING_CONV KI_DSAPublicBER PROTO_LIST ((POINTER *));
int RSA_CALLING_CONV KI_DSAPublicX957BER PROTO_LIST ((POINTER *));
int RSA_CALLING_CONV KI_ECPrivate PROTO_LIST ((POINTER *));
int RSA_CALLING_CONV KI_ECPrivateBER PROTO_LIST ((POINTER *));
int RSA_CALLING_CONV KI_ECPrivateComponent PROTO_LIST ((POINTER *));
int RSA_CALLING_CONV KI_ECPrivateComponentBER PROTO_LIST ((POINTER *));
int RSA_CALLING_CONV KI_ECPublic PROTO_LIST ((POINTER *));
int RSA_CALLING_CONV KI_ECPublicBER PROTO_LIST ((POINTER *));
int RSA_CALLING_CONV KI_ECPublicComponent PROTO_LIST ((POINTER *));
int RSA_CALLING_CONV KI_ECPublicComponentBER PROTO_LIST ((POINTER *));
int RSA_CALLING_CONV KI_Item PROTO_LIST ((POINTER *));
int RSA_CALLING_CONV KI_PKCS_RSAPrivate PROTO_LIST ((POINTER *));
int RSA_CALLING_CONV KI_PKCS_RSAMultiPrimePrivate PROTO_LIST ((POINTER *));
int RSA_CALLING_CONV KI_PKCS_RSAPrivateBER PROTO_LIST ((POINTER *));
int RSA_CALLING_CONV KI_PKCS11KeyAttributes PROTO_LIST ((POINTER *));
int RSA_CALLING_CONV KI_RSAPrivate PROTO_LIST ((POINTER *));
int RSA_CALLING_CONV KI_RSAPublic PROTO_LIST ((POINTER *));
int RSA_CALLING_CONV KI_RSAPublicBER PROTO_LIST ((POINTER *));
int RSA_CALLING_CONV KI_RSA_CRT PROTO_LIST ((POINTER *));

/* Key Info Types for BSAFE 1.x Support.
 */
int RSA_CALLING_CONV KI_DES_BSAFE1 PROTO_LIST ((POINTER *));
int RSA_CALLING_CONV KI_DESX_BSAFE1 PROTO_LIST ((POINTER *));
int RSA_CALLING_CONV KI_RC2WithBSAFE1Params PROTO_LIST ((POINTER *));
int RSA_CALLING_CONV KI_RC2_BSAFE1 PROTO_LIST ((POINTER *));
int RSA_CALLING_CONV KI_RSAPrivateBSAFE1 PROTO_LIST ((POINTER *));
int RSA_CALLING_CONV KI_RSAPublicBSAFE1 PROTO_LIST ((POINTER *));

/* Algorithm Info Types.
 */
int RSA_CALLING_CONV AI_BSSecretSharing PROTO_LIST ((POINTER *));
int RSA_CALLING_CONV AI_CBC_IV8 PROTO_LIST ((POINTER *));
int RSA_CALLING_CONV AI_CBC_IV16 PROTO_LIST ((POINTER *));
int RSA_CALLING_CONV AI_HW_Random PROTO_LIST ((POINTER *));

int RSA_CALLING_CONV AI_AES_ECB PROTO_LIST ((POINTER *));
int RSA_CALLING_CONV AI_AES128_ECB PROTO_LIST ((POINTER *));
int RSA_CALLING_CONV AI_AES192_ECB PROTO_LIST ((POINTER *));
int RSA_CALLING_CONV AI_AES256_ECB PROTO_LIST ((POINTER *));
int RSA_CALLING_CONV AI_AES128_ECB_BER PROTO_LIST ((POINTER *));
int RSA_CALLING_CONV AI_AES192_ECB_BER PROTO_LIST ((POINTER *));
int RSA_CALLING_CONV AI_AES256_ECB_BER PROTO_LIST ((POINTER *));

int RSA_CALLING_CONV AI_AES_CBC PROTO_LIST ((POINTER *));
int RSA_CALLING_CONV AI_AES128_CBC PROTO_LIST ((POINTER *));
int RSA_CALLING_CONV AI_AES192_CBC PROTO_LIST ((POINTER *));
int RSA_CALLING_CONV AI_AES256_CBC PROTO_LIST ((POINTER *));
int RSA_CALLING_CONV AI_AES_CBCPad PROTO_LIST ((POINTER *));
int RSA_CALLING_CONV AI_AES128_CBCPad PROTO_LIST ((POINTER *));
int RSA_CALLING_CONV AI_AES192_CBCPad PROTO_LIST ((POINTER *));
int RSA_CALLING_CONV AI_AES256_CBCPad PROTO_LIST ((POINTER *));
int RSA_CALLING_CONV AI_AES128_CBCPadBER PROTO_LIST ((POINTER *));
int RSA_CALLING_CONV AI_AES192_CBCPadBER PROTO_LIST ((POINTER *));
int RSA_CALLING_CONV AI_AES256_CBCPadBER PROTO_LIST ((POINTER *));

int RSA_CALLING_CONV AI_AES_CFB PROTO_LIST ((POINTER *));
int RSA_CALLING_CONV AI_AES128_CFB PROTO_LIST ((POINTER *));
int RSA_CALLING_CONV AI_AES192_CFB PROTO_LIST ((POINTER *));
int RSA_CALLING_CONV AI_AES256_CFB PROTO_LIST ((POINTER *));
int RSA_CALLING_CONV AI_AES128_CFB_BER PROTO_LIST ((POINTER *));
int RSA_CALLING_CONV AI_AES192_CFB_BER PROTO_LIST ((POINTER *));
int RSA_CALLING_CONV AI_AES256_CFB_BER PROTO_LIST ((POINTER *));

int RSA_CALLING_CONV AI_DES_CBC_IV8 PROTO_LIST ((POINTER *));
int RSA_CALLING_CONV AI_DES_CBCPadBER PROTO_LIST ((POINTER *));
int RSA_CALLING_CONV AI_DES_CBCPadPEM PROTO_LIST ((POINTER *));
int RSA_CALLING_CONV AI_DES_EDE3_CBC_IV8 PROTO_LIST ((POINTER *));
int RSA_CALLING_CONV AI_DES_EDE3_CBCPadIV8 PROTO_LIST ((POINTER *));
int RSA_CALLING_CONV AI_DES_EDE3_CBCPadBER PROTO_LIST ((POINTER *));
int RSA_CALLING_CONV AI_DES_CBCPadIV8 PROTO_LIST ((POINTER *));
int RSA_CALLING_CONV AI_DESX_CBC_IV8 PROTO_LIST ((POINTER *));
int RSA_CALLING_CONV AI_DESX_CBCPadIV8 PROTO_LIST ((POINTER *));
int RSA_CALLING_CONV AI_DESX_CBCPadBER PROTO_LIST ((POINTER *));
int RSA_CALLING_CONV AI_DHKeyAgree PROTO_LIST ((POINTER *));
int RSA_CALLING_CONV AI_DHKeyAgreeBER PROTO_LIST ((POINTER *));
int RSA_CALLING_CONV AI_DHParamGen PROTO_LIST ((POINTER *));
int RSA_CALLING_CONV AI_DSA PROTO_LIST ((POINTER *));
int RSA_CALLING_CONV AI_DSAKeyGen PROTO_LIST ((POINTER *));
int RSA_CALLING_CONV AI_DSAParamGen PROTO_LIST ((POINTER *));
int RSA_CALLING_CONV AI_DSAWithSHA1 PROTO_LIST ((POINTER *));
int RSA_CALLING_CONV AI_DSAWithSHA1_BER PROTO_LIST ((POINTER *));
int RSA_CALLING_CONV AI_ECParamGen PROTO_LIST ((POINTER *));
int RSA_CALLING_CONV AI_ECParameters PROTO_LIST ((POINTER *));
int RSA_CALLING_CONV AI_ECParametersBER PROTO_LIST ((POINTER *));
int RSA_CALLING_CONV AI_ECPubKey PROTO_LIST ((POINTER *));
int RSA_CALLING_CONV AI_ECPubKeyBER PROTO_LIST ((POINTER *));
int RSA_CALLING_CONV AI_ECKeyGen PROTO_LIST ((POINTER *));
int RSA_CALLING_CONV AI_EC_DHKeyAgree PROTO_LIST ((POINTER *));
int RSA_CALLING_CONV AI_EC_DSAWithDigest PROTO_LIST ((POINTER *));
int RSA_CALLING_CONV AI_EC_DSA PROTO_LIST ((POINTER *));
int RSA_CALLING_CONV AI_EC_ES PROTO_LIST ((POINTER *));
int RSA_CALLING_CONV AI_ECAcceleratorTable PROTO_LIST ((POINTER *));
int RSA_CALLING_CONV AI_ECBuildAcceleratorTable PROTO_LIST ((POINTER *));
int RSA_CALLING_CONV AI_ECBuildPubKeyAccelTable PROTO_LIST ((POINTER *));
int RSA_CALLING_CONV AI_FeedbackCipher PROTO_LIST ((POINTER *));
int RSA_CALLING_CONV AI_HMAC PROTO_LIST ((POINTER *));
int RSA_CALLING_CONV AI_KeypairGen PROTO_LIST ((POINTER *));
int RSA_CALLING_CONV AI_KeypairTokenGen PROTO_LIST ((POINTER *));
int RSA_CALLING_CONV AI_MD2 PROTO_LIST ((POINTER *));
int RSA_CALLING_CONV AI_MD2Random PROTO_LIST ((POINTER *));
int RSA_CALLING_CONV AI_MD2WithDES_CBCPad PROTO_LIST ((POINTER *));
int RSA_CALLING_CONV AI_MD2WithDES_CBCPadBER PROTO_LIST ((POINTER *));
int RSA_CALLING_CONV AI_MD2WithRC2_CBCPad PROTO_LIST ((POINTER *));
int RSA_CALLING_CONV AI_MD2WithRC2_CBCPadBER PROTO_LIST ((POINTER *));
int RSA_CALLING_CONV AI_MD2WithRSAEncryption PROTO_LIST ((POINTER *));
int RSA_CALLING_CONV AI_MD2WithRSAEncryptionBER PROTO_LIST ((POINTER *));
int RSA_CALLING_CONV AI_MD2_BER PROTO_LIST ((POINTER *));
int RSA_CALLING_CONV AI_MD2_PEM PROTO_LIST ((POINTER *));
int RSA_CALLING_CONV AI_MD5 PROTO_LIST ((POINTER *));
int RSA_CALLING_CONV AI_MD5Random PROTO_LIST ((POINTER *));
int RSA_CALLING_CONV AI_MD5WithDES_CBCPad PROTO_LIST ((POINTER *));
int RSA_CALLING_CONV AI_MD5WithDES_CBCPadBER PROTO_LIST ((POINTER *));
int RSA_CALLING_CONV AI_MD5WithRC2_CBCPad PROTO_LIST ((POINTER *));
int RSA_CALLING_CONV AI_MD5WithRC2_CBCPadBER PROTO_LIST ((POINTER *));
int RSA_CALLING_CONV AI_MD5WithRSAEncryption PROTO_LIST ((POINTER *));
int RSA_CALLING_CONV AI_MD5WithRSAEncryptionBER PROTO_LIST ((POINTER *));
int RSA_CALLING_CONV AI_MD5WithXOR PROTO_LIST ((POINTER *));
int RSA_CALLING_CONV AI_MD5WithXOR_BER PROTO_LIST ((POINTER *));
int RSA_CALLING_CONV AI_MD5_BER PROTO_LIST ((POINTER *));
int RSA_CALLING_CONV AI_MD5_PEM PROTO_LIST ((POINTER *));

/* PKCS OAEP added for Bsafe 4.1 */
int RSA_CALLING_CONV AI_PKCS_OAEPRecode PROTO_LIST ((POINTER *));
int RSA_CALLING_CONV AI_PKCS_OAEPRecodeBER PROTO_LIST ((POINTER *));
int RSA_CALLING_CONV AI_PKCS_OAEP_RSAPublic PROTO_LIST ((POINTER *));
int RSA_CALLING_CONV AI_PKCS_OAEP_RSAPublicBER PROTO_LIST ((POINTER *));
int RSA_CALLING_CONV AI_PKCS_OAEP_RSAPrivate PROTO_LIST ((POINTER *));
int RSA_CALLING_CONV AI_PKCS_OAEP_RSAPrivateBER PROTO_LIST ((POINTER *));

int RSA_CALLING_CONV AI_PKCS_RSAPrivate PROTO_LIST ((POINTER *));
int RSA_CALLING_CONV AI_SET_OAEP_RSAPrivate PROTO_LIST ((POINTER *));
int RSA_CALLING_CONV AI_PKCS_RSAPrivateBER PROTO_LIST ((POINTER *));
int RSA_CALLING_CONV AI_PKCS_RSAPrivatePEM PROTO_LIST ((POINTER *));
int RSA_CALLING_CONV AI_PKCS_RSAPublic PROTO_LIST ((POINTER *));
int RSA_CALLING_CONV AI_SET_OAEP_RSAPublic PROTO_LIST ((POINTER *));
int RSA_CALLING_CONV AI_PKCS_RSAPublicBER PROTO_LIST ((POINTER *));
int RSA_CALLING_CONV AI_PKCS_RSAPublicPEM PROTO_LIST ((POINTER *));
int RSA_CALLING_CONV AI_RC2 PROTO_LIST ((POINTER *));
int RSA_CALLING_CONV AI_RC2_CBC PROTO_LIST ((POINTER *));
int RSA_CALLING_CONV AI_RC2_CBCPad PROTO_LIST ((POINTER *));
int RSA_CALLING_CONV AI_RC2_CBCPadBER PROTO_LIST ((POINTER *));
int RSA_CALLING_CONV AI_RC2_CBCPadPEM PROTO_LIST ((POINTER *));
int RSA_CALLING_CONV AI_RC4 PROTO_LIST ((POINTER *));
int RSA_CALLING_CONV AI_RC4WithMAC PROTO_LIST ((POINTER *));
int RSA_CALLING_CONV AI_RC4WithMAC_BER PROTO_LIST ((POINTER *));
int RSA_CALLING_CONV AI_RC4_BER PROTO_LIST ((POINTER *));
int RSA_CALLING_CONV AI_RC5_CBC PROTO_LIST ((POINTER *));
int RSA_CALLING_CONV AI_RC5_CBCPad PROTO_LIST ((POINTER *));
int RSA_CALLING_CONV AI_RC5_CBCPadBER PROTO_LIST ((POINTER *));
int RSA_CALLING_CONV AI_RC6_CBC PROTO_LIST ((POINTER *));
int RSA_CALLING_CONV AI_RC6_CBCPad PROTO_LIST ((POINTER *));
int RSA_CALLING_CONV AI_RESET_IV PROTO_LIST ((POINTER *));
int RSA_CALLING_CONV AI_RFC1113Recode PROTO_LIST ((POINTER *));
int RSA_CALLING_CONV AI_RSAKeyGen PROTO_LIST ((POINTER *));
int RSA_CALLING_CONV AI_RSAMultiPrimeKeyGen PROTO_LIST ((POINTER *));
int RSA_CALLING_CONV AI_RSAStrongKeyGen PROTO_LIST ((POINTER *));
int RSA_CALLING_CONV AI_SignVerify PROTO_LIST ((POINTER *));
int RSA_CALLING_CONV AI_RSAPrivate PROTO_LIST ((POINTER *));
int RSA_CALLING_CONV AI_RSAPublic PROTO_LIST ((POINTER *));
int RSA_CALLING_CONV AI_SHA1 PROTO_LIST ((POINTER *));



/* We recommend that customers use AI_X962Random_V0 instead */
/* of AI_SHA1Random because future releases of BSAFE may */
/* change the behavior of AI_SHA1Random to match the */
/* algorithm by that name in JSAFE. */
int RSA_CALLING_CONV AI_SHA1Random PROTO_LIST ((POINTER *));
int RSA_CALLING_CONV AI_SHA1_BER PROTO_LIST ((POINTER *));
int RSA_CALLING_CONV AI_SHA1WithDES_CBCPad PROTO_LIST ((POINTER *));
int RSA_CALLING_CONV AI_SHA1WithDES_CBCPadBER PROTO_LIST ((POINTER *));
int RSA_CALLING_CONV AI_SHA1WithRSAEncryption PROTO_LIST ((POINTER *));
int RSA_CALLING_CONV AI_SHA1WithRSAEncryptionBER PROTO_LIST ((POINTER *));
int RSA_CALLING_CONV AI_SymKeyTokenGen PROTO_LIST ((POINTER *));
int RSA_CALLING_CONV AI_X962Random_V0 PROTO_LIST ((POINTER *));
int RSA_CALLING_CONV AI_X931Random PROTO_LIST ((POINTER *));

/* Algorithm Info Types for BSAFE 1.x Support.
 */
int RSA_CALLING_CONV AI_DES_CBC_BSAFE1 PROTO_LIST ((POINTER *));
int RSA_CALLING_CONV AI_DESX_CBC_BSAFE1 PROTO_LIST ((POINTER *));
int RSA_CALLING_CONV AI_MAC PROTO_LIST ((POINTER *));
int RSA_CALLING_CONV AI_MD PROTO_LIST ((POINTER *));
int RSA_CALLING_CONV AI_RC2_CBC_BSAFE1 PROTO_LIST ((POINTER *));
int RSA_CALLING_CONV AI_RSAPrivateBSAFE1 PROTO_LIST ((POINTER *));
int RSA_CALLING_CONV AI_RSAPublicBSAFE1 PROTO_LIST ((POINTER *));

/* Hardware Info Types for PKCS11 Support.
 */
int RSA_CALLING_CONV HI_PKCS11Session PROTO_LIST ((POINTER *));


int RSA_CALLING_CONV B_EncodeDigestInfo PROTO_LIST
  ((unsigned char *, unsigned int *, unsigned int, ITEM *, unsigned char *,
    unsigned int));
int RSA_CALLING_CONV B_DecodeDigestInfo PROTO_LIST
  ((ITEM *, ITEM *, unsigned char *, unsigned int));

/* Algorithm methods used when implementing block ciphers with feedback */
extern B_ALGORITHM_METHOD RSA_CALLING_CONV AM_AES_ENCRYPT;
extern B_ALGORITHM_METHOD RSA_CALLING_CONV AM_AES_DECRYPT;
extern B_ALGORITHM_METHOD RSA_CALLING_CONV AM_AES128_ENCRYPT;
extern B_ALGORITHM_METHOD RSA_CALLING_CONV AM_AES128_DECRYPT;
extern B_ALGORITHM_METHOD RSA_CALLING_CONV AM_AES192_ENCRYPT;
extern B_ALGORITHM_METHOD RSA_CALLING_CONV AM_AES192_DECRYPT;
extern B_ALGORITHM_METHOD RSA_CALLING_CONV AM_AES256_ENCRYPT;
extern B_ALGORITHM_METHOD RSA_CALLING_CONV AM_AES256_DECRYPT;
extern B_ALGORITHM_METHOD RSA_CALLING_CONV AM_DES_ENCRYPT;
extern B_ALGORITHM_METHOD RSA_CALLING_CONV AM_DES_DECRYPT;
extern B_ALGORITHM_METHOD RSA_CALLING_CONV AM_DESX_ENCRYPT;
extern B_ALGORITHM_METHOD RSA_CALLING_CONV AM_DESX_DECRYPT;
extern B_ALGORITHM_METHOD RSA_CALLING_CONV AM_DES_EDE_ENCRYPT;
extern B_ALGORITHM_METHOD RSA_CALLING_CONV AM_DES_EDE_DECRYPT;

extern B_ALGORITHM_METHOD RSA_CALLING_CONV AM_CBC_ENCRYPT;
extern B_ALGORITHM_METHOD RSA_CALLING_CONV AM_CBC_DECRYPT;
extern B_ALGORITHM_METHOD RSA_CALLING_CONV AM_CBC_INTER_LEAVED_ENCRYPT;
extern B_ALGORITHM_METHOD RSA_CALLING_CONV AM_CBC_INTER_LEAVED_DECRYPT;
extern B_ALGORITHM_METHOD RSA_CALLING_CONV AM_CFB_ENCRYPT;
extern B_ALGORITHM_METHOD RSA_CALLING_CONV AM_CFB_DECRYPT;
extern B_ALGORITHM_METHOD RSA_CALLING_CONV AM_CFB_PIPELINED_ENCRYPT;
extern B_ALGORITHM_METHOD RSA_CALLING_CONV AM_CFB_PIPELINED_DECRYPT;
extern B_ALGORITHM_METHOD RSA_CALLING_CONV AM_ECB_ENCRYPT;
extern B_ALGORITHM_METHOD RSA_CALLING_CONV AM_ECB_DECRYPT;
extern B_ALGORITHM_METHOD RSA_CALLING_CONV AM_OFB_ENCRYPT;
extern B_ALGORITHM_METHOD RSA_CALLING_CONV AM_OFB_DECRYPT;
extern B_ALGORITHM_METHOD RSA_CALLING_CONV AM_OFB_PIPELINED_ENCRYPT;
extern B_ALGORITHM_METHOD RSA_CALLING_CONV AM_OFB_PIPELINED_DECRYPT;



/* Algorithm methods for use in the algorithm chooser.
 */

extern B_ALGORITHM_METHOD RSA_CALLING_CONV AM_TOKEN_DES_CBC_ENCRYPT;
extern B_ALGORITHM_METHOD RSA_CALLING_CONV AM_TOKEN_DES_CBC_DECRYPT;
extern B_ALGORITHM_METHOD RSA_CALLING_CONV AM_TOKEN_DES_EDE3_CBC_ENCRYPT;
extern B_ALGORITHM_METHOD RSA_CALLING_CONV AM_TOKEN_DES_EDE3_CBC_DECRYPT;
extern B_ALGORITHM_METHOD RSA_CALLING_CONV AM_HW_RANDOM;
extern B_ALGORITHM_METHOD RSA_CALLING_CONV AM_FORMAT_X931;
extern B_ALGORITHM_METHOD RSA_CALLING_CONV AM_EXTRACT_X931;
extern B_ALGORITHM_METHOD RSA_CALLING_CONV AM_TOKEN_RSA_PRV_ENCRYPT;
extern B_ALGORITHM_METHOD RSA_CALLING_CONV AM_TOKEN_RSA_PUB_DECRYPT;
extern B_ALGORITHM_METHOD RSA_CALLING_CONV AM_TOKEN_RSA_ENCRYPT;
extern B_ALGORITHM_METHOD RSA_CALLING_CONV AM_TOKEN_RSA_DECRYPT;
extern B_ALGORITHM_METHOD RSA_CALLING_CONV AM_TOKEN_RSA_CRT_ENCRYPT;
extern B_ALGORITHM_METHOD RSA_CALLING_CONV AM_TOKEN_RSA_CRT_DECRYPT;
extern B_ALGORITHM_METHOD RSA_CALLING_CONV AM_AES_CBC_DECRYPT;
extern B_ALGORITHM_METHOD RSA_CALLING_CONV AM_AES_CBC_ENCRYPT;
extern B_ALGORITHM_METHOD RSA_CALLING_CONV AM_AES128_CBC_DECRYPT;
extern B_ALGORITHM_METHOD RSA_CALLING_CONV AM_AES128_CBC_ENCRYPT;
extern B_ALGORITHM_METHOD RSA_CALLING_CONV AM_AES192_CBC_DECRYPT;
extern B_ALGORITHM_METHOD RSA_CALLING_CONV AM_AES192_CBC_ENCRYPT;
extern B_ALGORITHM_METHOD RSA_CALLING_CONV AM_AES256_CBC_DECRYPT;
extern B_ALGORITHM_METHOD RSA_CALLING_CONV AM_AES256_CBC_ENCRYPT;
extern B_ALGORITHM_METHOD RSA_CALLING_CONV AM_DES_CBC_DECRYPT;
extern B_ALGORITHM_METHOD RSA_CALLING_CONV AM_DES_CBC_ENCRYPT;
extern B_ALGORITHM_METHOD RSA_CALLING_CONV AM_DES_EDE3_CBC_DECRYPT;
extern B_ALGORITHM_METHOD RSA_CALLING_CONV AM_DES_EDE3_CBC_ENCRYPT;
extern B_ALGORITHM_METHOD RSA_CALLING_CONV AM_DESX_CBC_DECRYPT;
extern B_ALGORITHM_METHOD RSA_CALLING_CONV AM_DESX_CBC_ENCRYPT;
extern B_ALGORITHM_METHOD RSA_CALLING_CONV AM_DH_KEY_AGREE;
extern B_ALGORITHM_METHOD RSA_CALLING_CONV AM_DH_PARAM_GEN;
extern B_ALGORITHM_METHOD RSA_CALLING_CONV AM_DSA_KEY_GEN;
extern B_ALGORITHM_METHOD RSA_CALLING_CONV AM_DSA_PARAM_GEN;
extern B_ALGORITHM_METHOD RSA_CALLING_CONV AM_DSA_SIGN;
extern B_ALGORITHM_METHOD RSA_CALLING_CONV AM_DSA_VERIFY;
extern B_ALGORITHM_METHOD RSA_CALLING_CONV AM_DSA_KEY_TOKEN_GEN;
extern B_ALGORITHM_METHOD RSA_CALLING_CONV AM_TOKEN_DSA_SIGN;
extern B_ALGORITHM_METHOD RSA_CALLING_CONV AM_TOKEN_DSA_VERIFY;
extern B_ALGORITHM_METHOD RSA_CALLING_CONV AM_DYN_HUFF_COMPRESS;
extern B_ALGORITHM_METHOD RSA_CALLING_CONV AM_DYN_HUFF_DECOMPRESS;
extern B_ALGORITHM_METHOD RSA_CALLING_CONV AM_ECFP_PARAM_GEN;
extern B_ALGORITHM_METHOD RSA_CALLING_CONV AM_ECF2POLY_PARAM_GEN;
extern B_ALGORITHM_METHOD RSA_CALLING_CONV AM_ECFP_BLD_ACCEL_TABLE;
extern B_ALGORITHM_METHOD RSA_CALLING_CONV AM_ECFP_BLD_PUB_KEY_ACC_TAB;
extern B_ALGORITHM_METHOD RSA_CALLING_CONV AM_ECF2POLY_BLD_ACCEL_TABLE;
extern B_ALGORITHM_METHOD RSA_CALLING_CONV AM_ECF2POLY_BLD_PUB_KEY_ACC_TAB;
extern B_ALGORITHM_METHOD RSA_CALLING_CONV AM_ECFP_DECRYPT;
extern B_ALGORITHM_METHOD RSA_CALLING_CONV AM_ECF2POLY_DECRYPT;
extern B_ALGORITHM_METHOD RSA_CALLING_CONV AM_ECFP_ENCRYPT;
extern B_ALGORITHM_METHOD RSA_CALLING_CONV AM_ECF2POLY_ENCRYPT;
extern B_ALGORITHM_METHOD RSA_CALLING_CONV AM_ECFP_KEY_GEN;
extern B_ALGORITHM_METHOD RSA_CALLING_CONV AM_ECF2POLY_KEY_GEN;
extern B_ALGORITHM_METHOD RSA_CALLING_CONV AM_ECFP_DH_KEY_AGREE;
extern B_ALGORITHM_METHOD RSA_CALLING_CONV AM_ECF2POLY_DH_KEY_AGREE;
extern B_ALGORITHM_METHOD RSA_CALLING_CONV AM_ECFP_DSA_SIGN;
extern B_ALGORITHM_METHOD RSA_CALLING_CONV AM_ECF2POLY_DSA_SIGN;
extern B_ALGORITHM_METHOD RSA_CALLING_CONV AM_ECFP_DSA_VERIFY;
extern B_ALGORITHM_METHOD RSA_CALLING_CONV AM_ECF2POLY_DSA_VERIFY;
extern B_ALGORITHM_METHOD RSA_CALLING_CONV AM_MAC;
extern B_ALGORITHM_METHOD RSA_CALLING_CONV AM_MD2;
extern B_ALGORITHM_METHOD RSA_CALLING_CONV AM_MD2_RANDOM;
extern B_ALGORITHM_METHOD RSA_CALLING_CONV AM_MD2_RANDOM_2X;
extern B_ALGORITHM_METHOD RSA_CALLING_CONV AM_MD5;
extern B_ALGORITHM_METHOD RSA_CALLING_CONV AM_MD5_RANDOM;
extern B_ALGORITHM_METHOD RSA_CALLING_CONV AM_MD5_RANDOM_2X;
extern B_ALGORITHM_METHOD RSA_CALLING_CONV AM_MD;
extern B_ALGORITHM_METHOD RSA_CALLING_CONV AM_RC2_DECRYPT;
extern B_ALGORITHM_METHOD RSA_CALLING_CONV AM_RC2_ENCRYPT;
extern B_ALGORITHM_METHOD RSA_CALLING_CONV AM_RC2_CBC_DECRYPT;
extern B_ALGORITHM_METHOD RSA_CALLING_CONV AM_RC2_CBC_ENCRYPT;
extern B_ALGORITHM_METHOD RSA_CALLING_CONV AM_RC4_DECRYPT;
extern B_ALGORITHM_METHOD RSA_CALLING_CONV AM_RC4_ENCRYPT;
extern B_ALGORITHM_METHOD RSA_CALLING_CONV AM_RC4_WITH_MAC_DECRYPT;
extern B_ALGORITHM_METHOD RSA_CALLING_CONV AM_RC4_WITH_MAC_ENCRYPT;
extern B_ALGORITHM_METHOD RSA_CALLING_CONV AM_RC5_64DECRYPT;
extern B_ALGORITHM_METHOD RSA_CALLING_CONV AM_RC5_64ENCRYPT;
extern B_ALGORITHM_METHOD RSA_CALLING_CONV AM_RC5_ENCRYPT;
extern B_ALGORITHM_METHOD RSA_CALLING_CONV AM_RC5_DECRYPT;
extern B_ALGORITHM_METHOD RSA_CALLING_CONV AM_RC5_CBC_ENCRYPT;
extern B_ALGORITHM_METHOD RSA_CALLING_CONV AM_RC5_CBC_DECRYPT;
extern B_ALGORITHM_METHOD RSA_CALLING_CONV AM_RC6_ENCRYPT;
extern B_ALGORITHM_METHOD RSA_CALLING_CONV AM_RC6_DECRYPT;
extern B_ALGORITHM_METHOD RSA_CALLING_CONV AM_RC6_CBC_ENCRYPT;
extern B_ALGORITHM_METHOD RSA_CALLING_CONV AM_RC6_CBC_DECRYPT;
extern B_ALGORITHM_METHOD RSA_CALLING_CONV AM_RSA_CRT_DECRYPT;
extern B_ALGORITHM_METHOD RSA_CALLING_CONV AM_RSA_CRT_ENCRYPT;
extern B_ALGORITHM_METHOD RSA_CALLING_CONV AM_RSA_CRT_DECRYPT_BLIND;
extern B_ALGORITHM_METHOD RSA_CALLING_CONV AM_RSA_CRT_ENCRYPT_BLIND;
extern B_ALGORITHM_METHOD RSA_CALLING_CONV AM_RSA_CRT_X931_ENCRYPT;
extern B_ALGORITHM_METHOD RSA_CALLING_CONV AM_RSA_X931_DECRYPT;
extern B_ALGORITHM_METHOD RSA_CALLING_CONV AM_RSA_DECRYPT;
extern B_ALGORITHM_METHOD RSA_CALLING_CONV AM_RSA_ENCRYPT;
extern B_ALGORITHM_METHOD RSA_CALLING_CONV AM_RSA_KEY_GEN;
extern B_ALGORITHM_METHOD RSA_CALLING_CONV AM_RSA_STRONG_KEY_GEN;
extern B_ALGORITHM_METHOD RSA_CALLING_CONV AM_RSA_KEY_TOKEN_GEN;
extern B_ALGORITHM_METHOD RSA_CALLING_CONV AM_SHA;
extern B_ALGORITHM_METHOD RSA_CALLING_CONV AM_SHA_RANDOM;
extern B_ALGORITHM_METHOD RSA_CALLING_CONV AM_SYMMETRIC_KEY_TOKEN_GEN;
extern B_ALGORITHM_METHOD RSA_CALLING_CONV AM_X931_RANDOM;

extern B_EXT_ALGORITHM_METHOD RSA_CALLING_CONV AM_PKCS11_RSA_KEY_GEN;
extern B_EXT_ALGORITHM_METHOD RSA_CALLING_CONV AM_PKCS11_RSA_PRIVATE_SIGN;
extern B_EXT_ALGORITHM_METHOD RSA_CALLING_CONV AM_PKCS11_RSA_PRIVATE_DECRYPT;
extern B_EXT_ALGORITHM_METHOD RSA_CALLING_CONV AM_PKCS11_RSA_PUBLIC_ENCRYPT;
extern B_EXT_ALGORITHM_METHOD RSA_CALLING_CONV AM_PKCS11_RSA_PUBLIC_VERIFY;
extern B_EXT_ALGORITHM_METHOD RSA_CALLING_CONV AM_PKCS11_DSA_KEY_GEN;
extern B_EXT_ALGORITHM_METHOD RSA_CALLING_CONV AM_PKCS11_DSA_SIGN;
extern B_EXT_ALGORITHM_METHOD RSA_CALLING_CONV AM_PKCS11_DSA_VERIFY;
extern B_EXT_ALGORITHM_METHOD RSA_CALLING_CONV AM_PKCS11_EC_KEY_GEN;
extern B_EXT_ALGORITHM_METHOD RSA_CALLING_CONV AM_PKCS11_EC_DSA_SIGN;
extern B_EXT_ALGORITHM_METHOD RSA_CALLING_CONV AM_PKCS11_EC_DSA_VERIFY;

/* If the compiler doesn’t support global data initialization for
   function pointers, you should compile your application with
   GLOBAL_FUNCTION_POINTERS defined as 0 and call the appropriate AM_ Init
   function for the corresponding AM_.  By default, GLOBAL_FUNCTION_POINTERS
   is defined as 1.
 */
#if RSA_GLOBAL_FUNCTION_POINTERS == RSA_DISABLED
void RSA_CALLING_CONV BSAFE_VERSIONInit PROTO_LIST ((void));
void RSA_CALLING_CONV AM_TokenDES_CBCEncryptInit PROTO_LIST ((void));
void RSA_CALLING_CONV AM_TokenDES_CBCDecryptInit PROTO_LIST ((void));
void RSA_CALLING_CONV AM_HWRandomInit PROTO_LIST ((void));
void RSA_CALLING_CONV AM_TokenRSAPrvEncryptInit PROTO_LIST ((void));
void RSA_CALLING_CONV AM_TokenRSAPubDecryptInit PROTO_LIST ((void));
void RSA_CALLING_CONV AM_AES_CBCDecryptInit PROTO_LIST ((void));
void RSA_CALLING_CONV AM_AES_CBCEncryptInit PROTO_LIST ((void));
void RSA_CALLING_CONV AM_DES_CBCDecryptInit PROTO_LIST ((void));
void RSA_CALLING_CONV AM_DES_CBCEncryptInit PROTO_LIST ((void));
void RSA_CALLING_CONV AM_DES_EDE3_CBCDecryptInit PROTO_LIST ((void));
void RSA_CALLING_CONV AM_DES_EDE3_CBCEncryptInit PROTO_LIST ((void));
void RSA_CALLING_CONV AM_DESX_CBCDecryptInit PROTO_LIST ((void));
void RSA_CALLING_CONV AM_DESX_CBCEncryptInit PROTO_LIST ((void));
void RSA_CALLING_CONV AM_DHKeyAgreeInit PROTO_LIST ((void));
void RSA_CALLING_CONV AM_DHParamGenInit PROTO_LIST ((void));
void RSA_CALLING_CONV AM_DSAKeyGenInit PROTO_LIST ((void));
void RSA_CALLING_CONV AM_DSAParamGenInit PROTO_LIST ((void));
void RSA_CALLING_CONV AM_DSASignInit PROTO_LIST ((void));
void RSA_CALLING_CONV AM_DSAVerifyInit PROTO_LIST ((void));
void RSA_CALLING_CONV AM_ECFPEncryptInit PROTO_LIST ((void));
void RSA_CALLING_CONV AM_ECF2PolyEncryptInit PROTO_LIST ((void));
void RSA_CALLING_CONV AM_ECFPDecryptInit PROTO_LIST ((void));
void RSA_CALLING_CONV AM_ECF2PolyDecryptInit PROTO_LIST ((void));
void RSA_CALLING_CONV AM_ECFPBldAccelTableInit PROTO_LIST ((void));
void RSA_CALLING_CONV AM_ECFPBldPubKeyAccTabInit PROTO_LIST ((void));
void RSA_CALLING_CONV AM_ECF2PolyBldAccelTableInit PROTO_LIST ((void));
void RSA_CALLING_CONV AM_FormatX931Init PROTO_LIST ((void));
void RSA_CALLING_CONV AM_ExtractX931Init PROTO_LIST ((void));
void RSA_CALLING_CONV AM_HMACInit PROTO_LIST ((void));
void RSA_CALLING_CONV AM_MACInit PROTO_LIST ((void));
void RSA_CALLING_CONV AM_MD2Init PROTO_LIST ((void));
void RSA_CALLING_CONV AM_MD2RandomInit PROTO_LIST ((void));
void RSA_CALLING_CONV AM_MD2RandomInit_2X PROTO_LIST ((void));
void RSA_CALLING_CONV AM_MD5Init PROTO_LIST ((void));
void RSA_CALLING_CONV AM_MD5RandomInit PROTO_LIST ((void));
void RSA_CALLING_CONV AM_MD5RandomInit_2X PROTO_LIST ((void));
void RSA_CALLING_CONV AM_MDInit PROTO_LIST ((void));
void RSA_CALLING_CONV AM_RC2DecryptInit PROTO_LIST ((void));
void RSA_CALLING_CONV AM_RC2EncryptInit PROTO_LIST ((void));
void RSA_CALLING_CONV AM_RC2_CBCDecryptInit PROTO_LIST ((void));
void RSA_CALLING_CONV AM_RC2_CBCEncryptInit PROTO_LIST ((void));
void RSA_CALLING_CONV AM_RC4EncryptDecryptInit PROTO_LIST ((void));
void RSA_CALLING_CONV AM_RC4WithMacEncryptInit PROTO_LIST ((void));
void RSA_CALLING_CONV AM_RC4WithMacDecryptInit PROTO_LIST ((void));
void RSA_CALLING_CONV AM_RC5DecryptInit PROTO_LIST ((void));
void RSA_CALLING_CONV AM_RC5EncryptInit PROTO_LIST ((void));
void RSA_CALLING_CONV AM_RC5_CBCDecryptInit PROTO_LIST ((void));
void RSA_CALLING_CONV AM_RC5_CBCEncryptInit PROTO_LIST ((void));
void RSA_CALLING_CONV AM_RC6DecryptInit PROTO_LIST ((void));
void RSA_CALLING_CONV AM_RC6EncryptInit PROTO_LIST ((void));
void RSA_CALLING_CONV AM_RC6_CBCDecryptInit PROTO_LIST ((void));
void RSA_CALLING_CONV AM_RC6_CBCEncryptInit PROTO_LIST ((void));
void RSA_CALLING_CONV AM_RSA_CRTEncryptDecryptInit PROTO_LIST ((void));
void RSA_CALLING_CONV AM_RSA_CRTEncryptDecryptBlindInit PROTO_LIST ((void));
void RSA_CALLING_CONV AM_RSAEncryptDecryptInit PROTO_LIST ((void));
void RSA_CALLING_CONV AM_RSAX931DecryptInit PROTO_LIST ((void));
void RSA_CALLING_CONV AM_RSA_CRT_X931EncryptInit PROTO_LIST ((void));

void RSA_CALLING_CONV AM_RSAKeyGenInit PROTO_LIST ((void));
void RSA_CALLING_CONV AM_SHAInit PROTO_LIST ((void));
void RSA_CALLING_CONV AM_SHARandomInit PROTO_LIST ((void));
void RSA_CALLING_CONV AM_SymmetricKeyTokenGenInit PROTO_LIST (void);
void RSA_CALLING_CONV AM_X931RandomInit PROTO_LIST ((void));

#endif

#ifdef __cplusplus
}
#endif

#endif /* end _BSAFE_H_ */

/* Copyright (C) RSA Data Security, Inc. created 1990.  This is an
   unpublished work protected as such under copyright law.  This work
   contains proprietary, confidential, and trade secret information of
   RSA Data Security, Inc.  Use, disclosure or reproduction without the
   express written authorization of RSA Data Security, Inc. is
   prohibited.
 */

#ifndef _BSAFE_H_
#define _BSAFE_H_ 1

#ifndef CALL_CONV
#define CALL_CONV
#endif

#include "atypes.h"

#ifndef _STD_ALLOC_FUNCS_
#define _STD_ALLOC_FUNCS_
#endif

#ifndef _STD_MEM_FUNCS_
#define _STD_MEM_FUNCS_
#endif
#include "stdlibrf.h"

#ifdef __cplusplus
extern "C" {
#endif

#define BE_ALGORITHM_ALREADY_SET 0x0200
#define BE_ALGORITHM_INFO 0x0201
#define BE_ALGORITHM_NOT_INITIALIZED 0x0202
#define BE_ALGORITHM_NOT_SET 0x0203
#define BE_ALGORITHM_OBJ 0x0204
#define BE_ALG_OPERATION_UNKNOWN 0x0205
#define BE_ALLOC 0x0206
#define BE_CANCEL 0x0207
#define BE_DATA 0x0208
#define BE_EXPONENT_EVEN 0x0209
#define BE_EXPONENT_LEN 0x020a
#define BE_HARDWARE 0x020b
#define BE_INPUT_DATA 0x020c
#define BE_INPUT_LEN 0x020d
#define BE_KEY_ALREADY_SET 0x020e
#define BE_KEY_INFO 0x020f
#define BE_KEY_LEN 0x0210
#define BE_KEY_NOT_SET 0x0211
#define BE_KEY_OBJ 0x0212
#define BE_KEY_OPERATION_UNKNOWN 0x0213
#define BE_MEMORY_OBJ 0x0214
#define BE_MODULUS_LEN 0x0215
#define BE_NOT_INITIALIZED 0x0216
#define BE_NOT_SUPPORTED 0x0217
#define BE_OUTPUT_LEN 0x0218
#define BE_OVER_32K 0x0219
#define BE_RANDOM_NOT_INITIALIZED 0x021a
#define BE_RANDOM_OBJ 0x021b
#define BE_SIGNATURE 0x021c
#define BE_WRONG_ALGORITHM_INFO 0x021d
#define BE_WRONG_KEY_INFO 0x021e
#define BE_INPUT_COUNT 0x021f
#define BE_OUTPUT_COUNT 0x0220
#define BE_METHOD_NOT_IN_CHOOSER 0x221
#define BE_KEY_WEAK 0x222

typedef POINTER B_KEY_OBJ;
typedef POINTER B_ALGORITHM_OBJ;

typedef int (CALL_CONV *B_INFO_TYPE) PROTO_LIST ((POINTER *));

#ifndef _BUILD_LIBRARY_
typedef char B_ALGORITHM_METHOD;
typedef B_ALGORITHM_METHOD **B_ALGORITHM_CHOOSER;
#endif

/* Version information.
 */
extern char * CALL_CONV BSAFE_VERSION;

/* The key object.
 */
int CALL_CONV B_CreateKeyObject PROTO_LIST ((B_KEY_OBJ *));
void CALL_CONV B_DestroyKeyObject PROTO_LIST ((B_KEY_OBJ *));
int CALL_CONV B_SetKeyInfo PROTO_LIST ((B_KEY_OBJ, B_INFO_TYPE, POINTER));
int CALL_CONV B_GetKeyInfo PROTO_LIST ((POINTER *, B_KEY_OBJ, B_INFO_TYPE));

/* The algorithm object.
 */
int CALL_CONV B_CreateAlgorithmObject PROTO_LIST ((B_ALGORITHM_OBJ *));
void CALL_CONV B_DestroyAlgorithmObject PROTO_LIST ((B_ALGORITHM_OBJ *));
int CALL_CONV B_SetAlgorithmInfo PROTO_LIST
  ((B_ALGORITHM_OBJ, B_INFO_TYPE, POINTER));
int CALL_CONV B_GetAlgorithmInfo PROTO_LIST
  ((POINTER *, B_ALGORITHM_OBJ, B_INFO_TYPE));

unsigned int B_IntegerBits PROTO_LIST ((unsigned char *, unsigned int));

/* Algorithm operations.
 */
int CALL_CONV B_RandomInit PROTO_LIST
  ((B_ALGORITHM_OBJ, B_ALGORITHM_CHOOSER, A_SURRENDER_CTX *));
int CALL_CONV B_RandomUpdate PROTO_LIST
  ((B_ALGORITHM_OBJ, unsigned char *, unsigned int, A_SURRENDER_CTX *));
int CALL_CONV B_GenerateRandomBytes PROTO_LIST
  ((B_ALGORITHM_OBJ, unsigned char *, unsigned int, A_SURRENDER_CTX *));

int CALL_CONV B_DigestInit PROTO_LIST
  ((B_ALGORITHM_OBJ, B_KEY_OBJ, B_ALGORITHM_CHOOSER, A_SURRENDER_CTX *));
int CALL_CONV B_DigestUpdate PROTO_LIST
  ((B_ALGORITHM_OBJ, unsigned char *, unsigned int, A_SURRENDER_CTX *));
int CALL_CONV B_DigestFinal PROTO_LIST
  ((B_ALGORITHM_OBJ, unsigned char *, unsigned int *, unsigned int,
    A_SURRENDER_CTX *));

int CALL_CONV B_EncryptInit PROTO_LIST
  ((B_ALGORITHM_OBJ, B_KEY_OBJ, B_ALGORITHM_CHOOSER, A_SURRENDER_CTX *));
int CALL_CONV B_EncryptUpdate PROTO_LIST
  ((B_ALGORITHM_OBJ, unsigned char *, unsigned int *, unsigned int,
    unsigned char *, unsigned int, B_ALGORITHM_OBJ, A_SURRENDER_CTX *));
int CALL_CONV B_EncryptFinal PROTO_LIST
  ((B_ALGORITHM_OBJ, unsigned char *, unsigned int *, unsigned int,
    B_ALGORITHM_OBJ, A_SURRENDER_CTX *));

int CALL_CONV B_DecryptInit PROTO_LIST
  ((B_ALGORITHM_OBJ, B_KEY_OBJ, B_ALGORITHM_CHOOSER, A_SURRENDER_CTX *));
int CALL_CONV B_DecryptUpdate PROTO_LIST
  ((B_ALGORITHM_OBJ, unsigned char *, unsigned int *, unsigned int,
    unsigned char *, unsigned int, B_ALGORITHM_OBJ, A_SURRENDER_CTX *));
int CALL_CONV B_DecryptFinal PROTO_LIST
  ((B_ALGORITHM_OBJ, unsigned char *, unsigned int *, unsigned int,
    B_ALGORITHM_OBJ, A_SURRENDER_CTX *));

int CALL_CONV B_EncodeInit PROTO_LIST ((B_ALGORITHM_OBJ));
int CALL_CONV B_EncodeUpdate PROTO_LIST
  ((B_ALGORITHM_OBJ, unsigned char *, unsigned int *, unsigned int,
    unsigned char *, unsigned int));
int CALL_CONV B_EncodeFinal PROTO_LIST
  ((B_ALGORITHM_OBJ, unsigned char *, unsigned int *, unsigned int));

int CALL_CONV B_DecodeInit PROTO_LIST ((B_ALGORITHM_OBJ));
int CALL_CONV B_DecodeUpdate PROTO_LIST
  ((B_ALGORITHM_OBJ, unsigned char *, unsigned int *, unsigned int,
    unsigned char *, unsigned int));
int CALL_CONV B_DecodeFinal PROTO_LIST
  ((B_ALGORITHM_OBJ, unsigned char *, unsigned int *, unsigned int));

int CALL_CONV B_SignInit PROTO_LIST
  ((B_ALGORITHM_OBJ, B_KEY_OBJ, B_ALGORITHM_CHOOSER, A_SURRENDER_CTX *));
int CALL_CONV B_SignUpdate PROTO_LIST
  ((B_ALGORITHM_OBJ, unsigned char *, unsigned int, A_SURRENDER_CTX *));
int CALL_CONV B_SignFinal PROTO_LIST
  ((B_ALGORITHM_OBJ, unsigned char *, unsigned int *, unsigned int,
    B_ALGORITHM_OBJ, A_SURRENDER_CTX *));

int CALL_CONV B_VerifyInit PROTO_LIST
  ((B_ALGORITHM_OBJ, B_KEY_OBJ, B_ALGORITHM_CHOOSER, A_SURRENDER_CTX *));
int CALL_CONV B_VerifyUpdate PROTO_LIST
  ((B_ALGORITHM_OBJ, unsigned char *, unsigned int, A_SURRENDER_CTX *));
int CALL_CONV B_VerifyFinal PROTO_LIST
  ((B_ALGORITHM_OBJ, unsigned char *, unsigned int, B_ALGORITHM_OBJ,
    A_SURRENDER_CTX *));

int CALL_CONV B_KeyAgreeInit PROTO_LIST
  ((B_ALGORITHM_OBJ, B_KEY_OBJ, B_ALGORITHM_CHOOSER, A_SURRENDER_CTX *));
int CALL_CONV B_KeyAgreePhase1 PROTO_LIST
  ((B_ALGORITHM_OBJ, unsigned char *, unsigned int *, unsigned int,
    B_ALGORITHM_OBJ, A_SURRENDER_CTX *));
int CALL_CONV B_KeyAgreePhase2 PROTO_LIST
  ((B_ALGORITHM_OBJ, unsigned char *, unsigned int *, unsigned int,
    unsigned char *, unsigned int, A_SURRENDER_CTX *));
 
int CALL_CONV B_GenerateInit PROTO_LIST
  ((B_ALGORITHM_OBJ, B_ALGORITHM_CHOOSER, A_SURRENDER_CTX *));
int CALL_CONV B_GenerateKeypair PROTO_LIST
  ((B_ALGORITHM_OBJ, B_KEY_OBJ, B_KEY_OBJ, B_ALGORITHM_OBJ,
    A_SURRENDER_CTX *));
int CALL_CONV B_GenerateParameters PROTO_LIST
  ((B_ALGORITHM_OBJ, B_ALGORITHM_OBJ, B_ALGORITHM_OBJ, A_SURRENDER_CTX *));


/* Information for BSAFE 1.x-compatible encryption algorithms.
 */
#define B_BSAFE1_PAD 1
#define B_BSAFE1_PAD_CHECKSUM 2
#define B_BSAFE1_RAW 3

typedef struct {
  int encryptionType;                /* encryption type: B_BSAFE1_PAD, etc. */
} B_BSAFE1_ENCRYPTION_PARAMS;

typedef struct {
  unsigned char *key;                              /* pointer to 8-byte key */
  unsigned int effectiveKeyBits;                     /* effective key length */
} B_RC2_BSAFE1_PARAMS_KEY;

/* Information for password-based encryption (PBE) algorithms.
 */
typedef struct {
  unsigned char *salt;                                        /* salt value */
  unsigned int iterationCount;                           /* iteration count */
} B_PBE_PARAMS;

typedef struct {
  unsigned int effectiveKeyBits;                     /* effective key length */
  unsigned char *salt;                                        /* salt value */
  unsigned int iterationCount;                           /* iteration count */
} B_RC2_PBE_PARAMS;

/* Information for MAC algorithm.
 */
typedef struct {
  unsigned int macLen;                               /* length of MAC value */
} B_MAC_PARAMS;

/* Information for RC4 with MAC algorithm.
 */
typedef struct {
  ITEM salt;                                         /* variable-length salt */
  unsigned int macLen;                        /* length to use for MAC value */
} B_RC4_WITH_MAC_PARAMS;

/* Information for DSA parameter generation
 */
typedef struct {
  unsigned int primeBits;
} B_DSA_PARAM_GEN_PARAMS;

/* Information for Secret sharing algorithm
 */
typedef struct {
  unsigned int threshold;                                 /* share threshold */
} B_SECRET_SHARING_PARAMS;

/* Key Info Types.
 */
int CALL_CONV KI_8Byte PROTO_LIST ((POINTER *));
int CALL_CONV KI_24Byte PROTO_LIST ((POINTER *));
int CALL_CONV KI_DES8 PROTO_LIST ((POINTER *));
int CALL_CONV KI_DES8Strong PROTO_LIST ((POINTER *));
int CALL_CONV KI_DES24Strong PROTO_LIST ((POINTER *));
int CALL_CONV KI_DESX PROTO_LIST ((POINTER *));
int CALL_CONV KI_DSAPrivate PROTO_LIST ((POINTER *));
int CALL_CONV KI_DSAPrivateBER PROTO_LIST ((POINTER *));
int CALL_CONV KI_DSAPublic PROTO_LIST ((POINTER *));
int CALL_CONV KI_DSAPublicBER PROTO_LIST ((POINTER *));
int CALL_CONV KI_Item PROTO_LIST ((POINTER *));
int CALL_CONV KI_PKCS_RSAPrivate PROTO_LIST ((POINTER *));
int CALL_CONV KI_PKCS_RSAPrivateBER PROTO_LIST ((POINTER *));
int CALL_CONV KI_RSAPrivate PROTO_LIST ((POINTER *));
int CALL_CONV KI_RSAPublic PROTO_LIST ((POINTER *));
int CALL_CONV KI_RSAPublicBER PROTO_LIST ((POINTER *));
int CALL_CONV KI_RSA_CRT PROTO_LIST ((POINTER *));

/* Key Info Types for BSAFE 1.x Support.
 */
int CALL_CONV KI_DES_BSAFE1 PROTO_LIST ((POINTER *));
int CALL_CONV KI_DESX_BSAFE1 PROTO_LIST ((POINTER *));
int CALL_CONV KI_RC2WithBSAFE1Params PROTO_LIST ((POINTER *));
int CALL_CONV KI_RC2_BSAFE1 PROTO_LIST ((POINTER *));
int CALL_CONV KI_RSAPrivateBSAFE1 PROTO_LIST ((POINTER *));
int CALL_CONV KI_RSAPublicBSAFE1 PROTO_LIST ((POINTER *));

/* Algorithm Info Types.
 */
int CALL_CONV AI_BSSecretSharing PROTO_LIST ((POINTER *));
int CALL_CONV AI_CBC_IV8 PROTO_LIST ((POINTER *));
int CALL_CONV AI_DES_CBC_IV8 PROTO_LIST ((POINTER *));
int CALL_CONV AI_DES_CBCPadBER PROTO_LIST ((POINTER *));
int CALL_CONV AI_DES_CBCPadPEM PROTO_LIST ((POINTER *));
int CALL_CONV AI_DES_EDE3_CBC_IV8 PROTO_LIST ((POINTER *));
int CALL_CONV AI_DES_EDE3_CBCPadIV8 PROTO_LIST ((POINTER *));
int CALL_CONV AI_DES_EDE3_CBCPadBER PROTO_LIST ((POINTER *));
int CALL_CONV AI_DES_CBCPadIV8 PROTO_LIST ((POINTER *));
int CALL_CONV AI_DESX_CBC_IV8 PROTO_LIST ((POINTER *));
int CALL_CONV AI_DESX_CBCPadIV8 PROTO_LIST ((POINTER *));
int CALL_CONV AI_DESX_CBCPadBER PROTO_LIST ((POINTER *));
int CALL_CONV AI_DHKeyAgree PROTO_LIST ((POINTER *));
int CALL_CONV AI_DHKeyAgreeBER PROTO_LIST ((POINTER *));
int CALL_CONV AI_DHParamGen PROTO_LIST ((POINTER *));
int CALL_CONV AI_DSA PROTO_LIST ((POINTER *));
int CALL_CONV AI_DSAKeyGen PROTO_LIST ((POINTER *));
int CALL_CONV AI_DSAParamGen PROTO_LIST ((POINTER *));
int CALL_CONV AI_DSAWithSHA1 PROTO_LIST ((POINTER *));
int CALL_CONV AI_DSAWithSHA1_BER PROTO_LIST ((POINTER *));
int CALL_CONV AI_MD2 PROTO_LIST ((POINTER *));
int CALL_CONV AI_MD2Random PROTO_LIST ((POINTER *));
int CALL_CONV AI_MD2WithDES_CBCPad PROTO_LIST ((POINTER *));
int CALL_CONV AI_MD2WithDES_CBCPadBER PROTO_LIST ((POINTER *));
int CALL_CONV AI_MD2WithRC2_CBCPad PROTO_LIST ((POINTER *));
int CALL_CONV AI_MD2WithRC2_CBCPadBER PROTO_LIST ((POINTER *));
int CALL_CONV AI_MD2WithRSAEncryption PROTO_LIST ((POINTER *));
int CALL_CONV AI_MD2WithRSAEncryptionBER PROTO_LIST ((POINTER *));
int CALL_CONV AI_MD2_BER PROTO_LIST ((POINTER *));
int CALL_CONV AI_MD2_PEM PROTO_LIST ((POINTER *));
int CALL_CONV AI_MD5 PROTO_LIST ((POINTER *));
int CALL_CONV AI_MD5Random PROTO_LIST ((POINTER *));
int CALL_CONV AI_MD5WithDES_CBCPad PROTO_LIST ((POINTER *));
int CALL_CONV AI_MD5WithDES_CBCPadBER PROTO_LIST ((POINTER *));
int CALL_CONV AI_MD5WithRC2_CBCPad PROTO_LIST ((POINTER *));
int CALL_CONV AI_MD5WithRC2_CBCPadBER PROTO_LIST ((POINTER *));
int CALL_CONV AI_MD5WithRSAEncryption PROTO_LIST ((POINTER *));
int CALL_CONV AI_MD5WithRSAEncryptionBER PROTO_LIST ((POINTER *));
int CALL_CONV AI_MD5WithXOR PROTO_LIST ((POINTER *));
int CALL_CONV AI_MD5WithXOR_BER PROTO_LIST ((POINTER *));
int CALL_CONV AI_MD5_BER PROTO_LIST ((POINTER *));
int CALL_CONV AI_MD5_PEM PROTO_LIST ((POINTER *));
int CALL_CONV AI_PKCS_RSAPrivate PROTO_LIST ((POINTER *));
int CALL_CONV AI_PKCS_RSAPrivateBER PROTO_LIST ((POINTER *));
int CALL_CONV AI_PKCS_RSAPrivatePEM PROTO_LIST ((POINTER *));
int CALL_CONV AI_PKCS_RSAPublic PROTO_LIST ((POINTER *));
int CALL_CONV AI_PKCS_RSAPublicBER PROTO_LIST ((POINTER *));
int CALL_CONV AI_PKCS_RSAPublicPEM PROTO_LIST ((POINTER *));
int CALL_CONV AI_RC2_CBC PROTO_LIST ((POINTER *));
int CALL_CONV AI_RC2_CBCPad PROTO_LIST ((POINTER *));
int CALL_CONV AI_RC2_CBCPadBER PROTO_LIST ((POINTER *));
int CALL_CONV AI_RC2_CBCPadPEM PROTO_LIST ((POINTER *));
int CALL_CONV AI_RC4 PROTO_LIST ((POINTER *));
int CALL_CONV AI_RC4WithMAC PROTO_LIST ((POINTER *));
int CALL_CONV AI_RC4WithMAC_BER PROTO_LIST ((POINTER *));
int CALL_CONV AI_RC4_BER PROTO_LIST ((POINTER *));
int CALL_CONV AI_RC5_CBC PROTO_LIST ((POINTER *));
int CALL_CONV AI_RC5_CBCPad PROTO_LIST ((POINTER *));
int CALL_CONV AI_RFC1113Recode PROTO_LIST ((POINTER *));
int CALL_CONV AI_RSAKeyGen PROTO_LIST ((POINTER *));
int CALL_CONV AI_RSAPrivate PROTO_LIST ((POINTER *));
int CALL_CONV AI_RSAPublic PROTO_LIST ((POINTER *));
int CALL_CONV AI_SHA1 PROTO_LIST ((POINTER *));
int CALL_CONV AI_SHA1_BER PROTO_LIST ((POINTER *));
int CALL_CONV AI_SHA1WithDES_CBCPad PROTO_LIST ((POINTER *));
int CALL_CONV AI_SHA1WithDES_CBCPadBER PROTO_LIST ((POINTER *));
int CALL_CONV AI_SHA1WithRSAEncryption PROTO_LIST ((POINTER *));
int CALL_CONV AI_SHA1WithRSAEncryptionBER PROTO_LIST ((POINTER *));

/* Algorithm Info Types for BSAFE 1.x Support.
 */
int CALL_CONV AI_DES_CBC_BSAFE1 PROTO_LIST ((POINTER *));
int CALL_CONV AI_DESX_CBC_BSAFE1 PROTO_LIST ((POINTER *));
int CALL_CONV AI_MAC PROTO_LIST ((POINTER *));
int CALL_CONV AI_MD PROTO_LIST ((POINTER *));
int CALL_CONV AI_RC2_CBC_BSAFE1 PROTO_LIST ((POINTER *));
int CALL_CONV AI_RSAPrivateBSAFE1 PROTO_LIST ((POINTER *));
int CALL_CONV AI_RSAPublicBSAFE1 PROTO_LIST ((POINTER *));

int CALL_CONV B_EncodeDigestInfo PROTO_LIST
  ((unsigned char *, unsigned int *, unsigned int, ITEM *, unsigned char *,
    unsigned int));
int CALL_CONV B_DecodeDigestInfo PROTO_LIST
  ((ITEM *, ITEM *, unsigned char *, unsigned int));

/* Algorithm methods for use in the algorithm chooser.
 */
extern B_ALGORITHM_METHOD CALL_CONV AM_DES_CBC_DECRYPT;
extern B_ALGORITHM_METHOD CALL_CONV AM_DES_CBC_ENCRYPT;
extern B_ALGORITHM_METHOD CALL_CONV AM_DES_EDE3_CBC_DECRYPT;
extern B_ALGORITHM_METHOD CALL_CONV AM_DES_EDE3_CBC_ENCRYPT;
extern B_ALGORITHM_METHOD CALL_CONV AM_DESX_CBC_DECRYPT;
extern B_ALGORITHM_METHOD CALL_CONV AM_DESX_CBC_ENCRYPT;
extern B_ALGORITHM_METHOD CALL_CONV AM_DH_KEY_AGREE;
extern B_ALGORITHM_METHOD CALL_CONV AM_DH_PARAM_GEN;
extern B_ALGORITHM_METHOD CALL_CONV AM_DSA_KEY_GEN;
extern B_ALGORITHM_METHOD CALL_CONV AM_DSA_PARAM_GEN;
extern B_ALGORITHM_METHOD CALL_CONV AM_DSA_SIGN;
extern B_ALGORITHM_METHOD CALL_CONV AM_DSA_VERIFY;
extern B_ALGORITHM_METHOD CALL_CONV AM_MAC;
extern B_ALGORITHM_METHOD CALL_CONV AM_MAC_86_2X;
extern B_ALGORITHM_METHOD CALL_CONV AM_MD2;
extern B_ALGORITHM_METHOD CALL_CONV AM_MD2_RANDOM;
extern B_ALGORITHM_METHOD CALL_CONV AM_MD2_RANDOM_2X;
extern B_ALGORITHM_METHOD CALL_CONV AM_MD5;
extern B_ALGORITHM_METHOD CALL_CONV AM_MD5_RANDOM;
extern B_ALGORITHM_METHOD CALL_CONV AM_MD5_RANDOM_2X;
extern B_ALGORITHM_METHOD CALL_CONV AM_MD;
extern B_ALGORITHM_METHOD CALL_CONV AM_RC2_CBC_DECRYPT;
extern B_ALGORITHM_METHOD CALL_CONV AM_RC2_CBC_ENCRYPT;
extern B_ALGORITHM_METHOD CALL_CONV AM_RC4_DECRYPT;
extern B_ALGORITHM_METHOD CALL_CONV AM_RC4_ENCRYPT;
extern B_ALGORITHM_METHOD CALL_CONV AM_RC4_WITH_MAC_DECRYPT;
extern B_ALGORITHM_METHOD CALL_CONV AM_RC4_WITH_MAC_ENCRYPT;
extern B_ALGORITHM_METHOD CALL_CONV AM_RC4_WITH_MAC_DECRYPT_86_2X;
extern B_ALGORITHM_METHOD CALL_CONV AM_RC4_WITH_MAC_ENCRYPT_86_2X;
extern B_ALGORITHM_METHOD CALL_CONV AM_RC5_CBC_ENCRYPT;
extern B_ALGORITHM_METHOD CALL_CONV AM_RC5_CBC_DECRYPT;
extern B_ALGORITHM_METHOD CALL_CONV AM_RSA_CRT_DECRYPT;
extern B_ALGORITHM_METHOD CALL_CONV AM_RSA_CRT_ENCRYPT;
extern B_ALGORITHM_METHOD CALL_CONV AM_RSA_CRT_DECRYPT_BLIND;
extern B_ALGORITHM_METHOD CALL_CONV AM_RSA_CRT_ENCRYPT_BLIND;
extern B_ALGORITHM_METHOD CALL_CONV AM_RSA_DECRYPT;
extern B_ALGORITHM_METHOD CALL_CONV AM_RSA_ENCRYPT;
extern B_ALGORITHM_METHOD CALL_CONV AM_RSA_KEY_GEN;
extern B_ALGORITHM_METHOD CALL_CONV AM_SHA;

/* If the compiler doesn't support global data initialization for
   function pointers, you should compile your application with
   GLOBAL_FUNCTION_POINTERS defined as 0 and call the appropriate AM_ Init
   function for the corresponding AM_.  By default, GLOBAL_FUNCTION_POINTERS
   is defined as 1.
 */
#if !GLOBAL_FUNCTION_POINTERS
void CALL_CONV BSAFE_VERSIONInit PROTO_LIST ((void));
void CALL_CONV AM_DES_CBCDecryptInit PROTO_LIST ((void));
void CALL_CONV AM_DES_CBCEncryptInit PROTO_LIST ((void));
void CALL_CONV AM_DES_EDE3_CBCDecryptInit PROTO_LIST ((void));
void CALL_CONV AM_DES_EDE3_CBCEncryptInit PROTO_LIST ((void));
void CALL_CONV AM_DESX_CBCDecryptInit PROTO_LIST ((void));
void CALL_CONV AM_DESX_CBCEncryptInit PROTO_LIST ((void));
void CALL_CONV AM_DHKeyAgreeInit PROTO_LIST ((void));
void CALL_CONV AM_DHParamGenInit PROTO_LIST ((void));
void CALL_CONV AM_DSAKeyGenInit PROTO_LIST ((void));
void CALL_CONV AM_DSAParamGenInit PROTO_LIST ((void));
void CALL_CONV AM_DSASignInit PROTO_LIST ((void));
void CALL_CONV AM_DSAVerifyInit PROTO_LIST ((void));
void CALL_CONV AM_MACInit PROTO_LIST ((void));
void CALL_CONV AM_MD2Init PROTO_LIST ((void));
void CALL_CONV AM_MD2RandomInit PROTO_LIST ((void));
void CALL_CONV AM_MD5Init PROTO_LIST ((void));
void CALL_CONV AM_MD5RandomInit PROTO_LIST ((void));
void CALL_CONV AM_MDInit PROTO_LIST ((void));
void CALL_CONV AM_RC2_CBCDecryptInit PROTO_LIST ((void));
void CALL_CONV AM_RC2_CBCEncryptInit PROTO_LIST ((void));
void CALL_CONV AM_RC4EncryptDecryptInit PROTO_LIST ((void));
void CALL_CONV AM_RC4WithMacEncryptInit PROTO_LIST ((void));
void CALL_CONV AM_RC4WithMacDecryptInit PROTO_LIST ((void));
void CALL_CONV AM_RC5_CBCDecryptInit PROTO_LIST ((void));
void CALL_CONV AM_RC5_CBCEncryptInit PROTO_LIST ((void));
void CALL_CONV AM_RSA_CRTEncryptDecryptInit PROTO_LIST ((void));
void CALL_CONV AM_RSAEncryptDecryptInit PROTO_LIST ((void));
void CALL_CONV AM_RSAKeyGenInit PROTO_LIST ((void));
void CALL_CONV AM_SHAInit PROTO_LIST ((void));
#endif

#ifdef __cplusplus
}
#endif

#endif /* end _BSAFE_H_ */

/******* HISTORY ********
$Log: bsafe.h,v $
Revision 1.2  2012/03/28 00:38:09  mcharon
remove forward slash from second line

Revision 1.1.1.1  2012/03/23 23:02:03  ptong
Initial archive of ng_diag module


$Endlog$
*/
