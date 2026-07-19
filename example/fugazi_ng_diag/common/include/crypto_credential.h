/* $Id: crypto_credential.h,v 1.3 2013/11/26 08:40:32 hroni Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/include/crypto_credential.h,v $
 *------------------------------------------------------------------
 * crypto_credential.h
 *
 * This file contains definitions and function prototypes to verify
 * the credential. Originally it was ported from Vikram (viksharm)
 * and Victor (vdvoroch). The project name is Watchtower
 *
 * Copyright (c) 2007 - 2013 by cisco Systems, Inc.
 * All rights reserved.
 *
 * Author: Huan Ngo
 *------------------------------------------------------------------
 */


typedef unsigned int wt_data_handle;

#define WT_RHO        0x30    // Rehost Root key for Device
#define WT_PHI        0x40    // Rehost Root key for Image


/* Key_Length Definations */
#define WT_P192_PUBLIC_LENGTH	0x30	// Decimal 48 Bytes Public key
#define WT_P192_BSAFE_PUBLIC_LENGTH        0x31    // 49 bytes

#define WT_CRYPTO_DATA_INCRAMENT 16

/* Key_Length Definations */
#define RND_1_LENGTH		0x20	// Decimal 32 bytes (256 bit) AES Keys
#define AES_BLOCK_SIZE		0x10	// Decimal 16 bytes
#define P192_AES_ENC_PRIVATE_LENGTH	0x28	// Decimal 40 bytes

/* Digest_Length */
#define WT_SHA1_LENGTH    0x14    // Decimal 20 bytes
#define WT_SHA2_LENGTH	  0x20	// Decimal 24 bytes


/* Buffer Lengths */
#define WT_MAX_KEY_INFO_DATA_LENGTH	0x60	// 64 bytes decimal
#define WT_P192_SIGNATURE_LENGTH	0x30	// Decimal 48 Bytes

/* TLV tag definitions */
#define WT_TLV_HEADER           0x1
#define WT_TLV_WDC              0x6
#define WT_TLV_WIC              0x7
#define WT_TLV_TAU_S_PRIVATE 	0x9
#define WT_TLV_TAU_E_PRIVATE 	0xa
#define WT_TLV_SIGNATURE        0xb
#define WT_TLV_KEY_CERT         0xd
#define WT_TLV_KEY_DESCRIPTOR  0x10
#define WT_TLV_SIG_DEL_RND     0x14
#define WT_TLV_ETA_S_PRIVATE     0x15
#define WT_TLV_ETA_E_PRIVATE     0x16

typedef struct WT_KeyDescriptor_
{
    uchar KeyDescriptiorVersion ;
    uchar KeyID ;		// Global identifier for the Key
    uchar KeyLength_msb ;	// length in bytes of the Key - msb
    uchar KeyLength_lsb ;	// lsb of the length
    uchar KeyType ;	// Type of key
    
    uchar KeyValue[WT_P192_PUBLIC_LENGTH] ;
} WT_KeyDescriptor;

#define WKC_1	0x1	/* Watchtower_KeyCertificateStructure Version */
typedef struct WT_KeyCertificate_
{
    uchar KeyCertificateStructureVersion ;
    uchar KeyFingerprint_Algo ;	// same as Digest_Algo
    uchar KeyFingerprint[WT_SHA2_LENGTH] ;
    uchar KeyID;
    uchar SertifyingKeyID;
    uchar KeySignatureByCertifyingKey[WT_P192_SIGNATURE_LENGTH] ;  
} WT_KeyCertificate;

#define WKD_1	0x1	/* Watchtower_KeyData Version */
typedef struct WT_Key_Data_
{
    uchar KeyDataVersion ;
    uchar KeyID ;		// Global identifier for the Key
    uchar KeyFingerprintAlgo ;	// same as Digest_Algo
    uchar KeyDataLenght_msb ;	// MSB of the length of KeyData
    uchar KeyDataLength_lsb ;	// LSB of Key data length
    uchar KeyFingerprint[WT_SHA2_LENGTH] ;	// the SHA1 message digest
    uchar KeyData[WT_MAX_KEY_INFO_DATA_LENGTH] ;
} WT_Key_Data;

typedef struct WT_Signature_
{
  uchar WSignature_Structure_Version ;
  uchar SigningAlgo ;
  uchar DigestAlgo ;
  uchar KeyID ;        // Global identifier for the signing Key
  uchar DigestValue[WT_SHA2_LENGTH] ;
  uchar Signature[WT_P192_SIGNATURE_LENGTH] ;  
} WT_Signature;




/* This struct is used for the wt_crypto_init */
typedef struct wt_data_
{
    uchar* publicKey;
    uint publicKeySize;
    uchar* publicKey2;
    uint publicKeySize2;
    uchar* privateKey;
    uint privateKeySize;
    uchar* signature;
    uint signatureSize;
    uchar* digestedData;
    uint digestedDataSize;
    uchar* decryptedData;
    uint decryptedDataSize;
    char* signatureInfo;
    uint signatureInfoSize;
    char* cipherData;
    uint cipherDataSize;
    char* decipherInfo;
    uint decipherInfoSize;

    uchar *signList;
    uint signListSize;

    char*  errorMsg;
    uint errMsgSize;
    int errorCode;
    
    void* scratch1;
    uint scratch1Size; 
    void* scratch2;
    uint scratch2Size; 
    void* scratch3;
    uint scratch3Size; 
    void* scratch4;
    uint scratch4Size;     
} wt_data;

#define WTP_1	0x1	/* WT_Tau_Private Version */
typedef struct WT_Tau_Private_
{
  uchar WT_Tau_Private_Version ;
  uchar TauPrivateEncryptionAlgo ;
  uchar WrappingKeyID ;
  uchar Spare ;
  ushort EncryptedTauPrivateLength ;
  uchar EncryptedTauPrivate[P192_AES_ENC_PRIVATE_LENGTH + AES_BLOCK_SIZE] ;
} WT_Tau_Private;


#define WDC_1	0x1	/* Watchtower_Device_Certificate Version */
//ndky == Node Key, which is either Tau or Eta
//SubCA Key is either Sigma or Psi
typedef struct WT_Node_Certificate_
{
  uchar WT_Node_Certificate_Version ;        // version = 0x1
  uchar CertificateRootID ;            // ID of the root key
  
  // Tau_s Public Info
  WT_KeyDescriptor ndky_s_Public ;    // Node Key Public
  WT_Key_Data ndky_s_Data ;        // Node Key Data
  WT_KeyCertificate ndky_s_Cert ;        // Node Key Cert
  
  WT_KeyDescriptor ndky_e_Public ;    // Node Key Public
  WT_Key_Data ndky_e_Data ;        // Node Key Data
  WT_KeyCertificate ndky_e_Cert ;        // Node Key Cert
  
  // SubCA Key Public Info
  WT_KeyDescriptor SubCAPublic ;
  WT_Key_Data SubCAData ;
  WT_KeyCertificate SubCACert ;
  
} WT_Node_Certificate;

 /* Reconstruction of length from 2 uchars */
 /* The only variable types used in all the structures are uchars */
 /* This is to avoid alignment and structure packing issues */
#define M_RECON_LENGTH(ushort,uchar_msb,uchar_lsb) \
 	ushort = 0 ; \
	ushort = uchar_msb << 8 ; \
	ushort |= uchar_lsb  ;

/*
 * This structure is used to keep the list of all FRU (Field Replacement
 * Unit) platforms. FRU platforms use PCB S/N instead of Chassis S/N
 * for WDC programming
 */

struct fru_platform_info {
    char   *name;                /* platform name */
    ushort id;                   /* controller id */
};

#define FRU_PLAT_MAX_IDS (sizeof(fru_platform_info_tbl) / \
			  sizeof(struct fru_platform_info))


/* ERROR CODES */
#define ENKI_ERR_ROOT_KEY_ID            0x9
#define ENKI_ERR_ALLOCATING_MEMORY	0xe
#define ENKI_ERR_INVALID_HANDLE         0x10
#define ENKI_ERR_INIT_NOT_CALLED        0x11
#define ENKI_ERR_BASE64_ENCODING    0x13
#define ENKI_ERR_TLV_INVALID		0x14
#define ENKI_ERR_SERNUM_NO_MATCH        0x18
#define ENKI_ERR_PID_NO_MATCH           0x19
#define ENKI_ERR_TLV_SIZE               0x1a
#define ENKI_ERR_STRUCT_SZ              0x1b
#define ENKI_ERR_IMAGE_NAME_NO_MATCH 0x1c

/* From bsafe_crypto_routines.h */

#define WDC_DIGEST_LEN 		24  /* This is the digest length for Quack */

#define RSA_DEMO_E_INVALID_PARAMETER 0x805 /* From Victor */
#define RSA_DEMO_E_ALLOC 0xFFFF   /* Check with Victor */

/* Retrieve the 16-bit value at addr pointed to by ptr */
#define GET16(ptr) ((((uchar*)(ptr))[0]<<8)|((uchar*)(ptr))[1])
/* retrieve a TLV's type */
#define GET_TLV_HDR(a) (((a)[0]<<8)+(a)[1])
/* check TLV type */
#define ASSERT_TLV(ptr, type) (GET_TLV_HDR(ptr)==type)
/* check TLV size */
#define ASSERT_TLV_SZ(tlv, size) (get_tlv_sz(tlv) == (int)size)
/* retrieve TLV's type */
#define GET_TLV_TAG(tlv) GET16(&((uchar*)(tlv))[0])
/* retrieve a TLV's length */
#define GET_TLV_LEN(a) (((a)[2]<<8)+(a)[3])
/* retrieve a TLV's content */
#define GET_TLV_VAL(a) (&(a)[4])
/* move to next TLV */
#define NEXT_TLV(tlv) ((tlv)+2*sizeof(ushort)+GET_TLV_LEN(tlv))

static inline
ushort USHORT_BYTESWAP (ushort x)
{
    return( ((x & 0x00FF) << 8) |
	    ((x & 0xFF00) >> 8));
}


extern uchar Test_LambdaPublicKey[] ;
extern uchar Test_RhoPublicKey[] ;
extern uchar Test_PhiPublicKey[] ;

extern unsigned char LAMBDA_01_PUB[];
extern unsigned char PHI_01_PUB[];
extern unsigned char RHO_01_PUB[];

#define LambdaPublicKey LAMBDA_01_PUB
#define RhoPublicKey RHO_01_PUB
#define PhiPublicKey PHI_01_PUB

extern int verify_credential(char *pcb, char *pid, uchar *cred, int size);
extern int verify_rma_deletion(char *sn, char *pid, uchar *del_request,
			       int del_request_sz, uchar *del_approval,
			       int del_approval_sz);
extern int nvflash_rma_delete (void);



/******** History ********
$Log: crypto_credential.h,v $
Revision 1.3  2013/11/26 08:40:32  hroni
fix compiler warning

Revision 1.2  2012/03/28 00:38:10  mcharon
remove forward slash from second line

Revision 1.1.1.1  2012/03/23 23:02:03  ptong
Initial archive of ng_diag module


$Endlog$
*/
