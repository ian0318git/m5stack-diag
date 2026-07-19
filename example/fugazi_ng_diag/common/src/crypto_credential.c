/* $Id: crypto_credential.c,v 1.5 2014/02/18 09:11:12 alpeng Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/crypto_credential.c,v $
 *------------------------------------------------------------------
 * crypto_credential.c
 *
 * This file contains functions to verify the credential. Originally
 * it was ported from Vikram (viksharm) and Victor (vdvoroch). The
 * project name is Watchtower
 *
 * Copyright (c) 2006 - 2014 by cisco Systems, Inc.
 * All rights reserved.
 *
 * Author: Huan Ngo
 *------------------------------------------------------------------
 */
#ifdef LINUX_APP
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>
#endif
#include <ctype.h>
#include "endians.h"
#include "types.h"
#include "common.h"
#include "error.h"
#include "bsafe.h"
#include "proto.h"
#include "free.h"
#include "crypto_credential.h"
#include "nvmonvars.h"
#include "strings.h"

wt_data_handle wt_handle;

/* vars used for init and finalize routines */ 
void** wt_data_list_start = NULL;
uint wt_data_list_count = 0;
uint wt_data_block_count = 0;

uchar *tauspub;
unsigned int tauspub_sz;
uchar *tauepub;
unsigned int tauepub_sz;
unsigned char *tauqs_tlv;
unsigned int tauqs_tlv_sz;
unsigned char *tauqe_tlv;
unsigned int tauqe_tlv_sz;


static B_ALGORITHM_METHOD *EC_DSA_CHOOSER[] = {
  &AM_SHA,
  &AM_ECFP_DSA_SIGN,
  &AM_ECF2POLY_DSA_SIGN,
  &AM_ECFP_DSA_VERIFY,
  &AM_ECF2POLY_DSA_VERIFY,
  (B_ALGORITHM_METHOD *)NULL_PTR
};

static B_ALGORITHM_METHOD *DIGEST_CHOOSER[] = {
    &AM_SHA,
    (B_ALGORITHM_METHOD *)NULL_PTR
};

/* THIS IS THE TEST LAMBDA PUBLIC KEY */
uchar Test_LambdaPublicKey[] = {
    0x04, 0x24, 0x84, 0x81, 0xf5, 0x11, 0x69, 0x87,
    0xb4, 0xba, 0xf6, 0xb6, 0xa3, 0xcc, 0x95, 0x41,
    0x1e, 0x8d, 0x5a, 0xa9, 0xbf, 0x2c, 0x28, 0x60,
    0xfd, 0xeb, 0x47, 0xf0, 0x75, 0x75, 0xee, 0xe7,
    0xf3, 0x19, 0x89, 0x65, 0x56, 0x6b, 0x57, 0xc1,
    0xf9, 0x19, 0x16, 0x6c, 0x13, 0xf0, 0xfe, 0x8b, 0x82};

uchar Test_RhoPublicKey[] = {
    0x04, 0x2c, 0xbc, 0x53, 0x95, 0x37, 0x84, 0x87,
    0x1a, 0x5c, 0xea, 0x52, 0x21, 0xda, 0x5e, 0x85,
    0xa9, 0x1c, 0x13, 0xe5, 0x33, 0x0f, 0x62, 0xe5,
    0x3d, 0x93, 0xa4, 0x7b, 0xfa, 0xfa, 0xf8, 0x04,
    0x38, 0xee, 0xfd, 0xab, 0x4e, 0xce, 0x6f, 0x3f,
    0x71, 0x68, 0x83, 0xf2, 0x08, 0xf3, 0x42, 0x65, 0x80}; 

uchar Test_PhiPublicKey[] = {
    0x04, 0x2c, 0xbc, 0x53, 0x95, 0x37, 0x84, 0x87,
    0x1a, 0x5c, 0xea, 0x52, 0x21, 0xda, 0x5e, 0x85,
    0xa9, 0x1c, 0x13, 0xe5, 0x33, 0x0f, 0x62, 0xe5,
    0x3d, 0x93, 0xa4, 0x7b, 0xfa, 0xfa, 0xf8, 0x04,
    0x38, 0xee, 0xfd, 0xab, 0x4e, 0xce, 0x6f, 0x3f,
    0x71, 0x68, 0x83, 0xf2, 0x08, 0xf3, 0x42, 0x65, 0x80};

/* Public key for LAMBDA_01 */
unsigned char LAMBDA_01_PUB[49] = {
  0x04,
  0xC6, 0x18, 0x39, 0x43, 0xAE, 0x79, 0xD1, 0x32, 
  0xBE, 0x98, 0x44, 0x95, 0x21, 0x7B, 0x48, 0xB5, 
  0x4B, 0xBA, 0xCF, 0xB2, 0x7B, 0xD4, 0x1D, 0x04, 
  0x56, 0xF3, 0x45, 0x08, 0xD9, 0x79, 0x04, 0x79, 
  0xDA, 0x88, 0xE6, 0x20, 0xC6, 0x16, 0x24, 0xFB, 
  0x10, 0x8C, 0x19, 0xE0, 0xE9, 0x89, 0xCE, 0x90 
};

/* Public key for PHI_01 */
unsigned char PHI_01_PUB[49] = {
  0x04,
  0xD1, 0x8B, 0x17, 0x97, 0xAC, 0x44, 0xDA, 0x9C, 
  0xD0, 0xE6, 0x45, 0x0B, 0x6E, 0x65, 0x82, 0xCC, 
  0x91, 0xE1, 0x6D, 0x68, 0x79, 0xF4, 0xAA, 0x1A, 
  0x82, 0xE8, 0x7A, 0x2E, 0x4B, 0x1D, 0xE8, 0xAA, 
  0x9A, 0x29, 0x44, 0x7A, 0x0B, 0x43, 0x97, 0x70, 
  0x50, 0x60, 0x85, 0x92, 0xFE, 0x73, 0xDC, 0xF3 
};

/* Public key for RHO_01 */
unsigned char RHO_01_PUB[49] = {
  0x04,
  0x35, 0xD9, 0x47, 0xD5, 0xA0, 0x8D, 0xED, 0x44, 
  0xC4, 0xA8, 0x0C, 0x60, 0x5C, 0x52, 0x46, 0x69, 
  0xE3, 0xE6, 0x47, 0x37, 0xB1, 0x23, 0x6C, 0x3B, 
  0xE5, 0xD9, 0xC7, 0x37, 0x54, 0x5F, 0x69, 0x0F, 
  0xDD, 0xF8, 0xC4, 0x69, 0xB3, 0x3D, 0x8A, 0x99, 
  0x19, 0xE9, 0xCE, 0xC0, 0x64, 0x8C, 0xC0, 0x54 
};

/*---------------Routines for base 64 encoding decoding--------------*/   
static const char b64table[] =
      "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

static const char b64revtb[256] = { 
  -3, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, /*0-15*/ 
  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, /*16-31*/
  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 62, -1, -1, -1, 63, /*32-47*/
  52, 53, 54, 55, 56, 57, 58, 59, 60, 61, -1, -1, -1, -2, -1, -1, /*48-63*/
  -1,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, /*64-79*/
  15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, -1, -1, -1, -1, -1, /*80-95*/
  -1, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, /*96-111*/
  41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, -1, -1, -1, -1, -1, /*112-127*/
  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, /*128-143*/
  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, /*144-159*/
  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, /*160-175*/
  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, /*176-191*/
  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, /*192-207*/
  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, /*208-223*/
  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, /*224-239*/
  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1  /*240-255*/
};


/*---------------------------------------------------------------------------
 * enki_api_crypto_data_fetch
 *
 * DESCRIPTION:
 * Retrieves the WT_Data struct associates with wt_handle.  This method is
 * inteded to be an internal utility method called by other
 * bsafe_crypto_routines
 * functions and not directly by an external caller
 *
 * PARAMETERS:
 *     wt_handle - wt_data_handle structure
 *     wt_data - address of the pointer to the structure wt_data
 *--------------------------------------------------------------------------*/
int
enki_api_crypto_data_fetch(wt_data_handle wt_handle_h,
			   wt_data **wt_data_ptr)
{
    int errCode = 0;
    if(wt_data_list_start != NULL){
	if(wt_handle_h <= (wt_data_block_count * WT_CRYPTO_DATA_INCRAMENT)){ 
	    *wt_data_ptr = (wt_data*) wt_data_list_start[(wt_handle_h-1)];
	}else{
	    *wt_data_ptr = NULL; 
	    errCode = ENKI_ERR_INVALID_HANDLE;
	}
    }
    else{
	*wt_data_ptr = NULL; 
	errCode = ENKI_ERR_INIT_NOT_CALLED;
	
    } 
    if(*wt_data_ptr == NULL) errCode = ENKI_ERR_INVALID_HANDLE; 
    return errCode;
}

/*---------------------------------------------------------------------------
 * get_sub_tlv
 *
 * DESCRIPTION:
 *     This functionget a TLV item from a list of TLVs by providing its type
 * if no TLV found for the given type, 0 is returned  
 * 
 * PARAMETERS:
 *     hdr_tlv - tlv array
 *     type - type of TLV
 *
 * RETURNS:
 *     pointer to TLV found, otherwise return 0
 *--------------------------------------------------------------------------*/
uchar *
get_sub_tlv(const uchar *hdr_tlv, const ushort type)
{

    ushort len = GET_TLV_LEN(hdr_tlv);
    uchar *p = (uchar*) GET_TLV_VAL(hdr_tlv);
    int i, pos;


    if (!ASSERT_TLV(hdr_tlv, WT_TLV_HEADER) ) {
	return 0;
    }
    
    for (i=0; i<len; i+=2) {
        if (GET16(p+i) == type) {
            break;
	}
    }
    if (i >= len) {
	return 0;
    } else {
	pos = 1 + i/2; /* count header TLV */
    }
    
    for (i=0, p=(uchar*)hdr_tlv; i<pos; i++) {
        p = NEXT_TLV(p);
    }

    if (GET_TLV_TAG(p) != type) {
	return 0;
    }
    return p;
}
/*---------------------------------------------------------------------------
 * enki_PrintBuf 
 *
 * DESCRIPTION:
 *     This function prints out the PID, PCB SN and credential
 * 
 * PARAMETERS:
 *     label - array to hold the title of the credential
 *     buf - array to hold the credental itself
 *     bufLen - array to hold the length of the credential
 *
 * RETURNS:
 *     N/A
 *--------------------------------------------------------------------------*/
void
enki_PrintBuf (char *label, uchar *buf, uint bufLen)
{
    uint i = 0, numBytes = 0;
    uchar line[17];
    
    if (label != NULL) {
	printf("\n%s (%u bytes):\n", label, bufLen);
    }
    for (i = 0; i < bufLen; i++) {
	/* remember the current character, if it's a printable ascii char */ 
	if (isprint(buf[i])) {
	    line[i%16] = buf[i];
	} else {
	    line[i%16] = '.';
	}
	printf(" %02x", buf[i]);
	
	/* if we're at the end of a line */
	if ((i & 15) == 15 || i == bufLen - 1) {
	    line[i%16+1] = '\0';  /* lines don't always contain 16 bytes */ 
	    /* for the case where i == bufLen - 1,
	     *  we must line up the beginning of
	     *  the ascii text correctly
	     */
	    for (numBytes = i%16; numBytes < 15; numBytes++) {
		printf("   ");
	    }
	    printf("      [%s]\n", line);
	}
    }
} 

/*---------------------------------------------------------------------------
 * enki_api_init
 *
 * DESCRIPTION:
 *     This function initialize wt_handle and wt_data_list_start
 * 
 * PARAMETERS:
 *     wt_handle - wt_data_handle structure
 *
 * RETURNS:
 *     PASSED - if there is no error
 *     ENKI_ERR_ALLOCATING_MEMORY - if fail to allocate memory
 *--------------------------------------------------------------------------*/
int
enki_api_init(wt_data_handle *wt_handle_ptr)
{
    wt_data_handle handle = 0;
    uint list_max, i;

    
    /*No Data objects stored so allocate some memory*/
    if (wt_data_list_start == NULL) { 
    /*allocate a list of pointers*/
	wt_data_list_start = (void**)T_malloc(sizeof(void*) *
					      WT_CRYPTO_DATA_INCRAMENT);
	/*initialize them to initial NULL value*/
	if (wt_data_list_start == NULL) {
	    return ENKI_ERR_ALLOCATING_MEMORY;
	}
	T_memset((uchar*)wt_data_list_start,0,sizeof(void*) * WT_CRYPTO_DATA_INCRAMENT);
	/* this is allways going to be the first data block so set to 1*/
	wt_data_block_count = 1;
    }
    
    /*calculate the number of pointers currently allocated so that we
     *can avoid an out of bounds error if all elements are populated
     */

    list_max = wt_data_block_count * WT_CRYPTO_DATA_INCRAMENT;

    /*find an open element */
    for(i = 0; i < list_max; i++) {
	if (wt_data_list_start[i] == NULL) {
	    /* found empty spot so allocate it*/
	    wt_data_list_start[i] = (void*)T_malloc(sizeof(wt_data));

	    if(wt_data_list_start[i] == NULL){
		return ENKI_ERR_ALLOCATING_MEMORY;
	    }
	    T_memset((uchar*)wt_data_list_start[i],0,sizeof(wt_data));
	    handle = i + 1;
	    break;
	}
    }
    /* a zero handle value means that all elemets are allocated and we need
     * to create some more
     */
    if (handle == 0) {  
	wt_data_list_start = (void**)T_realloc((uchar*)wt_data_list_start,
		      sizeof(void*) * (WT_CRYPTO_DATA_INCRAMENT + list_max));
	if(wt_data_list_start == NULL) {
	    return ENKI_ERR_ALLOCATING_MEMORY;
	}
	T_memset((uchar*)(wt_data_list_start+sizeof(void*)*list_max),0,sizeof(void*)*WT_CRYPTO_DATA_INCRAMENT);
	handle = list_max + 1;
	wt_data_block_count++;
    }
    *wt_handle_ptr = handle;
    wt_data_list_count++;
    return (PASSED);
}


/*---------------------------------------------------------------------------
 * get_tlv_sz
 *
 * DESCRIPTION:
 *     This functionget get the size of the whole tlv, including tag and len 
 * 
 * PARAMETERS:
 *     tlv_ptr - pointer to tlv array
 *
 * RETURNS:
 *     size of the tlv
 *--------------------------------------------------------------------------*/
int
get_tlv_sz(const uchar *tlv_ptr)
{
    int size=2*sizeof(ushort); /* tag+len */

    if(!tlv_ptr) return 0;
    
    if(GET_TLV_TAG(tlv_ptr) == WT_TLV_HEADER)
    {
        int i, val_len=GET_TLV_LEN(tlv_ptr), numOfSub=val_len/2;
        ushort *ptr=(ushort*)GET_TLV_VAL(tlv_ptr);

        size += val_len;

        /* add total length sub-tlvs */
        for(i=0; i<numOfSub; i++,ptr++)
        {
#ifdef X86
	    ushort tag = (USHORT_BYTESWAP)(*ptr);
#else	    
            ushort tag = *ptr;
#endif	    
            uchar *subtlv = get_sub_tlv(tlv_ptr,tag);

            size += get_tlv_sz(subtlv);
        }
    }
    else
    {
        size += GET_TLV_LEN(tlv_ptr);
    }

    return size;
}

/*---------------------------------------------------------------------------
 * enki_api_extract_wdc_tlv_componets
 *
 * DESCRIPTION:
 *     This function extracts 816 bytes from the original WDC 958 bytes
 * for diagnostic to verify   
 * 
 * PARAMETERS:
 *     wnc_nkq_tlv - pointer to the original WDC
 *     wnc_nkq_tlv_sz - size of the original WDC
 *     wdctlv - pointer to the extracted WDC
 *     wdctlv_sz - size of the extracted WDC
 *     nkq_s_tlv - nkq_s TLV
 *     nkq_s_tlv_sz - nkq_s TLV size
 *     nkq_e_tlv - nkq_e_tlv TLV
 *     nkq_e_tlv_sz - nkq_e_tlv TLV size
 *     wt_data_h - wt_data_handle structure
 *
 * RETURNS:
 *     status
 *--------------------------------------------------------------------------*/
int
enki_api_extract_wnc_tlv_componets(uchar *wnc_nkq_tlv, uint wnc_nkq_tlv_sz,
    uchar **wnctlv, uint *wnctlv_sz, uchar **nkq_s_tlv, uint *nkq_s_tlv_sz,
    uchar **nkq_e_tlv, uint *nkq_e_tlv_sz, wt_data_handle wt_data_h)
{
   int status = 0;
   wt_data * wt_data_ptr = NULL;
   uchar *buf;

   status = enki_api_crypto_data_fetch(wt_data_h,&wt_data_ptr);

   if(wt_data_ptr == NULL)
   {
     status = ENKI_ERR_INVALID_HANDLE;
     return status;
   }
   if(status != 0)
   {
     wt_data_ptr->errorCode = status;
     return status;
   }
   do
   {
     if(!ASSERT_TLV(wnc_nkq_tlv, WT_TLV_HEADER))
     {
       status = ENKI_ERR_TLV_INVALID; break;
     }
     if(!ASSERT_TLV_SZ(wnc_nkq_tlv,wnc_nkq_tlv_sz)) { status = ENKI_ERR_TLV_SIZE; break; }
     if( (buf = get_sub_tlv(wnc_nkq_tlv, WT_TLV_WDC)) != NULL)
     {
         *wnctlv_sz = GET_TLV_LEN(buf) + 4;
         *wnctlv = buf;

         buf = get_sub_tlv(wnc_nkq_tlv, WT_TLV_TAU_S_PRIVATE);
         if (!buf) { status = ENKI_ERR_TLV_INVALID; break; }
         *nkq_s_tlv_sz = GET_TLV_LEN(buf) + 4;
         *nkq_s_tlv = buf;

         buf = get_sub_tlv(wnc_nkq_tlv, WT_TLV_TAU_E_PRIVATE);
         if (!buf) { status = ENKI_ERR_TLV_INVALID; break; }
         *nkq_e_tlv_sz = GET_TLV_LEN(buf) + 4;
         *nkq_e_tlv = buf;
     }
     else if( (buf = get_sub_tlv(wnc_nkq_tlv, WT_TLV_WIC)) != NULL)
     {
         *wnctlv_sz = GET_TLV_LEN(buf) + 4;
         *wnctlv = buf;

         buf = get_sub_tlv(wnc_nkq_tlv, WT_TLV_ETA_S_PRIVATE);
         if (!buf) { status = ENKI_ERR_TLV_INVALID; break; }
         *nkq_s_tlv_sz = GET_TLV_LEN(buf) + 4;
         *nkq_s_tlv = buf;

         buf = get_sub_tlv(wnc_nkq_tlv, WT_TLV_ETA_E_PRIVATE);
         if (!buf) { status = ENKI_ERR_TLV_INVALID; break; }
         *nkq_e_tlv_sz = GET_TLV_LEN(buf) + 4;
         *nkq_e_tlv = buf;
     }
     else
     {
         status = ENKI_ERR_TLV_INVALID; break;
     }

   } while (0);
   if(status != 0)
   {
     wt_data_ptr->errorCode = status;
   }
   return status;
    
}

/*----------------------------------------------------------------------
 * NAME: enki_generate_SHA1_message_digest
 *
 * DESCRIPTION: Generates a SHA1 message digest 
 *
 * PARAMETERS:
 *             dataToDigest: Pointer to data
 *             dataToDigestLen: Length of data
 *             digestedData: Pointer to the buffer to digested data
 *             digestedDataLen - pointer to length of digested data
 *
 * RETURNS:
 *             0 if success, non-zero if failure
 *             digestedData - pointer to digest
 *             digestedDataLen - pointer to length of digest (usually 20)
 * 
 *--------------------------------------------------------------------------*/

int
enki_generate_SHA1_message_digest(uchar *dataToDigest, int dataToDigestLen,
				  uchar *digestedData,
				  unsigned int *digestedDataLen)
{
  
  B_ALGORITHM_OBJ digester = (B_ALGORITHM_OBJ)NULL_PTR;
  int status;

  do {
    /*  Step 1:  Create an algorithm object */ 
    if ((status = B_CreateAlgorithmObject (&digester)) != 0)
      break;
    /* Step 2:  Set the algorithm object to SHA1 */
    if ((status = B_SetAlgorithmInfo (digester, AI_SHA1, NULL_PTR)) != 0)
      break;
    /* Step 3:  Initialize the digest algorithm. Prompt the user for input */ 
    if ((status = B_DigestInit (digester, (B_KEY_OBJ)NULL_PTR, DIGEST_CHOOSER,
                                (A_SURRENDER_CTX *)NULL_PTR)) != 0)
      break;

    /* Step 4:  Update -- Digest the user's input */
    if ((status = B_DigestUpdate (digester, dataToDigest, dataToDigestLen,
                                  (A_SURRENDER_CTX *)NULL_PTR)) != 0)
      break;
    /* Step 5:  Final */ 
    if ((status = B_DigestFinal (digester, digestedData, digestedDataLen,
                                 WDC_DIGEST_LEN,
                                 (A_SURRENDER_CTX *)NULL_PTR)) != 0)
      break;
  } while (0);
  /*  Step 6:  Remember to destroy all objects, and free up any memory
      allocated */  
  B_DestroyAlgorithmObject (&digester);
  
  return (status);   
}	/* end of generate_message_digest */

/* -----------------------------------------------------------------------
  NAME: enki_CreateECParamsObject
 
  DESCRIPTION:
   Creates and sets the algorithm object with a set of EC parameters.
   Be sure to call B_DestroyAlgorithmObject on ecParamObj in the calling
   function!
 
  PARAMETERS:
     ecParamsObj - pointer to B_ALGORITHM_OBJ
     randomAlgorithm - B_ALGORITHM_OBJ
 
  RETURNS:
     0 - if success, non-zero if failure
 ----------------------------------------------------------------------- */
int
enki_CreateECParamsObject (B_ALGORITHM_OBJ *ecParamsObj,
			   B_ALGORITHM_OBJ randomAlgorithm)
{
      int status;

  B_ALGORITHM_OBJ paramGenObj = (B_ALGORITHM_OBJ)NULL_PTR;

  A_EC_PARAMS stockECParams;

/*   Corelation of P-192 curve with BSAFE 
 *   fieldInfo          p
 *   coeffA             a
 *   coeffB             b
 *   base               0x04, Px, Py
 *   order              n
 *   cofactor           1
 */ 
/* P-192 curve parameters */


unsigned char fieldInfo[24] = {
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFE,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF
   };

unsigned char coeffA[24] = {
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFE,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFC
   };

unsigned char coeffB[24] = {
    0x64, 0x21, 0x05, 0x19, 0xe5, 0x9c, 0x80, 0xe7,
    0x0f, 0xa7, 0xe9, 0xab, 0x72, 0x24, 0x30, 0x49,
    0xfe, 0xb8, 0xde, 0xec, 0xc1, 0x46, 0xb9, 0xb1
   };

unsigned char base[49] = {
    0x04, 0x18, 0x8d, 0xa8, 0x0e, 0xb0, 0x30, 0x90,
    0xf6, 0x7c, 0xbf, 0x20, 0xeb, 0x43, 0xa1, 0x88,
    0x00, 0xf4, 0xff, 0x0a, 0xfd, 0x82, 0xff, 0x10,
    0x12, 0x07, 0x19, 0x2b, 0x95, 0xff, 0xc8, 0xda,
    0x78, 0x63, 0x10, 0x11, 0xed, 0x6b, 0x24, 0xcd,
    0xd5, 0x73, 0xf9, 0x77, 0xa1, 0x1e, 0x79, 0x48,
    0x11
   };

unsigned char order[24] = {
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0x99, 0xDE, 0xF8, 0x36,
    0x14, 0x6B, 0xC9, 0xB1, 0xB4, 0xD2, 0x28, 0x31
   };

unsigned char cofactor[1] = {
    0x01
  };


  do {
    /*  Step 1:  Create algorithm object to hold EC Parameters */
    if ((status = B_CreateAlgorithmObject (ecParamsObj)) != 0)
      break;

      /*  Step 2:  Set with old hard-coded EC Parameters */
      stockECParams.version = 0x00;
      stockECParams.fieldType = FT_FP;
      stockECParams.pointRepresentation = CI_NO_COMPRESS;
      stockECParams.fieldElementBits = 192;
      stockECParams.fieldInfo.data = fieldInfo;
      stockECParams.fieldInfo.len = sizeof(fieldInfo);
      stockECParams.coeffA.data = coeffA;
      stockECParams.coeffA.len = sizeof(coeffA);
      stockECParams.coeffB.data = coeffB;
      stockECParams.coeffB.len = sizeof(coeffB);
      stockECParams.base.data = base;
      stockECParams.base.len = sizeof(base);
      stockECParams.order.data = order;
      stockECParams.order.len = sizeof(order);
      stockECParams.cofactor.data = cofactor;
      stockECParams.cofactor.len = sizeof(cofactor);

      /*  Note that we could have also used one array containing the
          BER-encoded EC parameters, along with AI_ECParametersBER, which
          would have been somewhat simpler to deal with since we would only
          have to worry about an ITEM structure instead of an A_EC_PARAMS
          structure.  */
      if ((status = B_SetAlgorithmInfo (*ecParamsObj, AI_ECParameters,
                                        (POINTER)&stockECParams)) != 0)
        break;
  } while(0);

  if (status != 0) {
    B_DestroyAlgorithmObject (ecParamsObj);
    printf("enki_CreateECParamsObject = %d", status);
  }

  B_DestroyAlgorithmObject (&paramGenObj);

  return status;
}    /*  end enki_CreateECParamsObject   */

/*  -----------------------------------------------------------------------
  NAME: enki_PrintBufAsCArray
 
  DESCRIPTION:
     Print the buffer passed as a C array
 
  PARAMETERS:
     label - pointer to uchar
     buffer - pointer to buffer that has to be printed
     bufferLen - length of buffer to be printed
 
  RETURNS:
    void
  -------------------------------------------------------------------------*/
void
enki_PrintBufAsCArray (char *label, unsigned char *buffer,
		       unsigned int bufferLen)
{
  unsigned int count;

  printf("unsigned char %s[%u] = {\n", label, bufferLen);
  for (count = 0 ; count < bufferLen; count++) {
    if ((count % 8) == 0)
      printf("  0x%02X", buffer[count]);
    else
      printf(" 0x%02X", buffer[count]);

    if (count != (bufferLen - 1))
      printf(",");

    if (((count % 8) == 7 ) || (count == (bufferLen - 1)))
      printf(" \n");
  }
  printf("};\n");
}  /* end enki_PrintBufAsCArray */ 

/* -----------------------------------------------------------------------
  NAME: enki_PrintECParamInfo

  DESCRIPTION:
   Given an algorithm object containing EC parameters, print the information
   out in a reasonably readable format.

   Valid AIs for infoFormat:
     AI_ECParameters
     AI_ECParametersBER

  PARAMETERS:
     ecParamsObj - B_ALGORITHM_OBJ
     infoFormat - B_INFO_TYPE

  RETURNS:
     0 if success, non-zero if failure
  ------------------------------------------------------------------------*/
int
enki_PrintECParamInfo (B_ALGORITHM_OBJ ecParamsObj, B_INFO_TYPE infoFormat)
{
  int status;
  
  A_EC_PARAMS *cryptocECParamInfo;
  ITEM *cryptocECParamInfoBER;

  do {
    if (infoFormat == AI_ECParameters) {
      if ((status = B_GetAlgorithmInfo ((POINTER *)&cryptocECParamInfo,
                                        ecParamsObj, AI_ECParameters)) != 0)
        break;

      printf("***EC Parameters Generated:\n");
      printf("version:  %u\n", cryptocECParamInfo->version);
      printf("point representation:  ");
      switch (cryptocECParamInfo->pointRepresentation) {
        case CI_NO_COMPRESS:
          printf("CI_NO_COMPRESS\n");
          break;
        default:
          printf("Unrecognized point representation value:  %d\n",
                       cryptocECParamInfo->pointRepresentation);
          break;
      }

      printf("field type:  ");
      switch (cryptocECParamInfo->fieldType) {
        case FT_FP:
          printf("FT_FP\n");
          break;
        case FT_F2_ONB:
          printf("FT_F2_ONB\n");
          break;
        case FT_F2_POLYNOMIAL:
          printf("FT_F2_POLYNOMIAL\n");
          break;
        default:
          printf("Unrecognized field type:  %i\n",
                      cryptocECParamInfo->fieldType);
          break;
      }
      printf("field element length:  %u bits\n",
                 cryptocECParamInfo->fieldElementBits);
      enki_PrintBufAsCArray ("fieldInfo", cryptocECParamInfo->fieldInfo.data,
                            cryptocECParamInfo->fieldInfo.len);
      enki_PrintBufAsCArray ("coeffA", cryptocECParamInfo->coeffA.data,
                            cryptocECParamInfo->coeffA.len);
      enki_PrintBufAsCArray ("coeffB", cryptocECParamInfo->coeffB.data,
                            cryptocECParamInfo->coeffB.len);
      enki_PrintBufAsCArray ("base", cryptocECParamInfo->base.data,
                            cryptocECParamInfo->base.len);
      enki_PrintBufAsCArray ("order", cryptocECParamInfo->order.data,
                            cryptocECParamInfo->order.len);
      enki_PrintBufAsCArray ("cofactor", cryptocECParamInfo->cofactor.data,
                            cryptocECParamInfo->cofactor.len);
    } else if (infoFormat == AI_ECParametersBER) {
      if ((status = B_GetAlgorithmInfo ((POINTER *)&cryptocECParamInfoBER,
                                        ecParamsObj, AI_ECParametersBER)) != 0)
        break;

      enki_PrintBufAsCArray ("ecParamsBER", cryptocECParamInfoBER->data,
                            cryptocECParamInfoBER->len);
    } else
      status = RSA_DEMO_E_INVALID_PARAMETER;
  } while(0);

  if (status != 0)
    printf("enki_PrintECParamInfo = %d\n", status);

  
  return status;
}   /*  end enki_PrintECParamInfo */  

/*---------------------------------------------------------------------------
 * strtok_r
 *
 * DESCRIPTION:
 *     This function extract tokens from a string
 * 
 * PARAMETERS:
 *     s - string holding tokens
 *     delim - delimiter
 *     lasts
 *
 * RETURNS:
 *     pointer to token string
 *--------------------------------------------------------------------------*/
char *
strtok_r(char *s, const char *delim, char **lasts)
{
	const char *spanp;
	int c, sc;
	char *tok;

	/* s may be NULL */
	assert(delim != NULL);
	assert(lasts != NULL);

	if (s == NULL && (s = *lasts) == NULL)
		return (NULL);

	/*
	 * Skip (span) leading delimiters (s += strspn(s, delim), sort of).
	 */
cont:
	c = *s++;
	for (spanp = delim; (sc = *spanp++) != 0;) {
		if (c == sc)
			goto cont;
	}

	if (c == 0) {		/* no non-delimiter characters */
		*lasts = NULL;
		return (NULL);
	}
	tok = s - 1;

	/*
	 * Scan token (scan for delimiters: s += strcspn(s, delim), sort of).
	 * Note that delim must have one NUL; we stop if we see that, too.
	 */
	for (;;) {
		c = *s++;
		spanp = delim;
		do {
			if ((sc = *spanp++) == c) {
				if (c == 0)
					s = NULL;
				else
					s[-1] = 0;
				*lasts = s;
				return (tok);
			}
		} while (sc != 0);
	}
	/* NOTREACHED */
}

/*---------------------------------------------------------------------------
 * strtok
 *
 * DESCRIPTION:
 *     This function extract tokens from a string, wrapparound of strtok_r
 * 
 * PARAMETERS:
 *     s - string holding tokens
 *     delim - delimiter
 *
 * RETURNS:
 *     pointer to token string
 *--------------------------------------------------------------------------*/
char *
strtok(char *s, const char *delim)
{
	static char *lasts;

	return strtok_r(s, delim, &lasts);
}



/*----------------------------------------------------------------------
NAME: enki_assert_udi
 
DESCRIPTION: Asserts UDI
 
PARAMETERS:  udi_data: Pointer to UDI data
             udi_data_sz: Length of UDI data
             pid - Product ID
             sernum - Serial number
  
RETURNS:     0 if success, non-zero if failure

NOTE:        This routine will allocate memory for the t_pub.
	     The calling program must do the memory menagement. 
	     Memory allocated for t_pub buffer must be freed up after the use.
--------------------------------------------------------------------------*/
static int
enki_assert_udi(uchar *udi_data, int udi_data_sz, char *pid, char *sernum)
{
  int status = 0;
  char *tok;
  uchar buf[WT_P192_PUBLIC_LENGTH];
#define PID_NEXT 1
#define SN_NEXT 2
  int next_item = 0;
  int got_pid=0, got_sn=0;

  if(!pid) return ENKI_ERR_PID_NO_MATCH;
  if(!sernum) return ENKI_ERR_SERNUM_NO_MATCH;

  memcpy(buf, udi_data, (udi_data_sz<sizeof(buf))?udi_data_sz:sizeof(buf));

  /* Parse udi data to extract pid and serial number */
  
  tok = strtok((char *)buf, "<>");
  while (tok != NULL)
  {
    if (next_item == PID_NEXT)
    {
       next_item = 0;
       if (strcmp(pid, tok))
       {
           status = ENKI_ERR_PID_NO_MATCH;
           break;
       }
    }
    else if (next_item == SN_NEXT)
    {
       next_item = 0;
       if (strcmp(sernum, tok))
       {
           status = ENKI_ERR_SERNUM_NO_MATCH;
           break;
       }
    }
    else if (!strcmp("PID", tok))
    {
        got_pid = 1;
        next_item = PID_NEXT;
    }
    else if (!strcmp("SN", tok))
    {
        got_sn = 1;
        next_item = SN_NEXT;
    }

    tok = strtok(NULL, "<>");
  }

  if(!status)
  {
      if(!got_pid) status = ENKI_ERR_PID_NO_MATCH;
      else if(!got_sn) status = ENKI_ERR_SERNUM_NO_MATCH;
  }

  return status;
}


/*-----------------------------------------------------------------------
  NAME: enki_verify_signature
 
  DESCRIPTION:
  This function verifies the signature using the public key 
 
  PARAMETERS:
   publicKey - pointer to public key
   publicKeyLen - length of public key
   dataToSign - data whose signature has to be verified
   dataToSignLen - length of data
   signature - pointer to signature to be verified
              signatureLen - length of signature

 
  RETURNS:
   0 if successful (signature is valid)
   non-zero if unsuccessful
 -----------------------------------------------------------------------*/
int
enki_verify_signature (unsigned char *publicKey,
		       unsigned int publicKeyLen,
		       unsigned char *dataToSign,
		       unsigned int dataToSignLen,
		       unsigned char *signature,
		       unsigned int signatureLen)             
{
  int status;

  B_ALGORITHM_OBJ ecDSAVerify = (B_ALGORITHM_OBJ)NULL_PTR;

  B_KEY_OBJ publicKeyObj = NULL_PTR;
  A_EC_PUBLIC_KEY publicKeyEC;
  B_ALGORITHM_OBJ ecParamsObj = (B_ALGORITHM_OBJ)NULL_PTR;
  A_EC_PARAMS *ecParamInfo;


  if ((signature == NULL_PTR) || (publicKey == NULL_PTR)) {
     printf("Generate signature first\n");
     return 1;
  }


  do {

     if ((status = enki_CreateECParamsObject (&ecParamsObj, NULL_PTR)) != 0)
        break;
     
     if ((NVRAM)->diagflag & D_VERBOSE) {
	 if ((status = enki_PrintECParamInfo(ecParamsObj, AI_ECParameters)) != 0)
           break;
     }
     
    /*  Retrieve useful information from the EC Parameters  */
    if ((status = B_GetAlgorithmInfo ((POINTER *)&ecParamInfo, ecParamsObj,
				      AI_ECParameters)) != 0) break;
    publicKeyEC.curveParams = *ecParamInfo;
    publicKeyEC.publicKey.data = publicKey;
    publicKeyEC.publicKey.len = publicKeyLen;
    

    /* create and set up a key object for public key verification */
    if ((status = B_CreateKeyObject (&publicKeyObj)) != 0)     break;
    if ((status = B_SetKeyInfo (publicKeyObj, KI_ECPublic,
                (POINTER)&publicKeyEC)) != 0)     break;
    /*  Step 1:  Create  */
    if ((status = B_CreateAlgorithmObject (&ecDSAVerify)) != 0)   break;
    

    if ((status = B_SetAlgorithmInfo (ecDSAVerify, AI_EC_DSA,  NULL_PTR)) != 0)
      break;
      
   /*  Step 3:  Init  */
    if ((status = B_VerifyInit (ecDSAVerify, publicKeyObj, EC_DSA_CHOOSER,
				(A_SURRENDER_CTX *)NULL_PTR)) != 0) break;
    /*  Step 4:  Update  */
    if ((status = B_VerifyUpdate (ecDSAVerify, dataToSign, dataToSignLen,
				  (A_SURRENDER_CTX *)NULL_PTR)) != 0) break;

    /*  Step 5:  Final  */
    if ((status = B_VerifyFinal (ecDSAVerify, signature, signatureLen,
	      (B_ALGORITHM_OBJ)NULL_PTR,(A_SURRENDER_CTX *)NULL_PTR)) != 0)
      break;

    if ((NVRAM)->diagflag & D_VERBOSE) {
	printf("\nSuccess!  The signature verified.\n");
    }
  } while(0);
  if (status != 0)
  {
      printf("\nFailure: enki_verify_signature = %d\n", status);
  }

  B_DestroyAlgorithmObject (&ecDSAVerify);
  B_DestroyKeyObject (&publicKeyObj);
  
  return status;

}	/* end enki_verify_signature */

/*---------------------------------------------------------------------------
 * spc_base64_encode
 *
 * DESCRIPTION: Accepts a binary buffer with an associated size.
 * Returns a base64 encoded, NULL-terminated string that has
 * been allocated with T_malloc. If the output is requried to contain
 * lines of data less than 80 characters in length then pass in a 
 * non-zero value for the wrap parameter and the code will insert 
 * newlines once every 76 characters.
 * PARAMETERS:
 *             input - Pointer to input buffer
 *             len - size of buffer
 *             wrap
 *
 * RETURNS:
 *             pointer to encoded output buffer
 *-------------------------------------------------------------------------*/
unsigned char *
spc_base64_encode(unsigned char *input, size_t len, int wrap)
{
  unsigned char *output, *p;
  size_t        i = 0, mod = len % 3, toalloc;
   
  toalloc = (len / 3) * 4 + (3 - mod) % 3 + 1;
  if (wrap) {
    toalloc += len / 57;
    if (len % 57) toalloc++;    
  }
  
  p = output = T_malloc(((len / 3) + (mod ? 1 : 0)) * 4 + 1);
  if (!p) return 0;
   
  while (i < len - mod) {
    *p++ = b64table[input[i++] >> 2];
    *p++ = b64table[((input[i - 1] << 4) | (input[i] >> 4)) & 0x3f];
    *p++ = b64table[((input[i] << 2) | (input[i + 1] >> 6)) & 0x3f];
    *p++ = b64table[input[i + 1] & 0x3f];
    i += 2;
    if (wrap && !(i % 57)) *p++ = '\n';
  }
  if (!mod) {
    if (wrap && i % 57) *p++ = '\n';
    *p = 0;
    return output;
  } else {
    *p++ = b64table[input[i++] >> 2];
    *p++ = b64table[((input[i - 1] << 4) | (input[i] >> 4)) & 0x3f];
    if (mod == 1) {
      *p++ = '=';
      *p++ = '=';
      if (wrap) *p++ = '\n';
      *p = 0;
      return output;
    } else {
      *p++ = b64table[(input[i] << 2) & 0x3f];
      *p++ = '=';
      if (wrap) *p++ = '\n';
      *p = 0;
      return output;
    }
  }
}


/*----------------------------------------------------------------------
 * NAME: enki_assert_image_name
 *
 * DESCRIPTION: Asserts UDI
 *
 * PARAMETERS:  wic_data: WIC nodekey data in struct WT_Key_Data
 *   containing image name
 *            wic_data_sz: Length of WIC data
 *            image_name - Image Name
 * 
 * RETURNS:     0 if success, non-zero if failure
 *
 * NOTE:        This routine will allocate memory for the t_pub.
 *	     The calling program must do the memory menagement. 
 *	     Memory allocated for t_pub buffer must be freed up after the use.
 *------------------------------------------------------------------------*/
static int
enki_assert_image_name(uchar *wic_data, size_t wic_data_sz, char *image_name)
{
  int status = 0;
  char *tok = NULL;
  int img_next = 0;
  uchar buf[WT_P192_PUBLIC_LENGTH];
  uchar img_name_hash[WDC_DIGEST_LEN];
  unsigned int img_name_hash_sz;
  char *b64_img_hash = NULL;

  if(!image_name) return ENKI_ERR_IMAGE_NAME_NO_MATCH;

  memcpy(buf, wic_data, (wic_data_sz<sizeof(buf))?wic_data_sz:sizeof(buf));

  do
  {
      status = enki_generate_SHA1_message_digest((uchar*)image_name, strlen(image_name), img_name_hash, &img_name_hash_sz) ;
      if(status) break;
     
      b64_img_hash = (char*)spc_base64_encode(img_name_hash, img_name_hash_sz, 0);

      /* extract image name from WIC key data and compare it against
	 the expected image name */
      tok = strtok((char *)buf, "<>");
      while (tok != NULL)
      {
        if (img_next)
        {
	    if (strcmp(b64_img_hash, tok))status = ENKI_ERR_IMAGE_NAME_NO_MATCH;
           break;
        }
        if (!strcmp("IMG", tok)) img_next = 1;

        tok = strtok(NULL, "<>");
      }

      if(!status)
          if(!img_next)
              status = ENKI_ERR_IMAGE_NAME_NO_MATCH;
  }while(0);

  if(b64_img_hash) free(b64_img_hash);

  return status;
}



/*----------------------------------------------------------------------
NAME: enki_verify_WNC_data
 
DESCRIPTION: Verifies WT_Node_Certificate struct
 
PARAMETERS:  wnc: Pointer to a WDC struct
             wnc_sz: size of the wnc argument
             pid - Product ID
             sernum - Serial number
  
RETURNS:     0 if success, non-zero if failure
	     nk_s_pub - pointer to NodeKey Private key
	     nk_s_pub_ln - Length of the NodeKey Private key
	     nk_e_pub - pointer to NodeKey Private key
	     nk_e_pub_ln - Length of the NodeKey Private key

NOTE:        This routine will allocate memory for the t_pub.
	     The calling program must do the memory menagement. 
	     Memory allocated for t_pub buffer must be freed up after the use.
---------------------------------------------------------------------------*/
static int
enki_verify_WNC_data(uchar *wnc, uint wnc_sz, uchar** nk_s_pub,
		     uint *nk_s_pub_ln, uchar** nk_e_pub, uint *nk_e_pub_ln,
		     char *pid, char *sernum, char* image_name)
{
  int status = 0;
  uchar *pub_root_key;
  
  WT_Node_Certificate *wtncptr = NULL;

  WT_KeyDescriptor *ndky_s_Publicptr;
  WT_Key_Data *ndky_s_Dataptr;
  WT_KeyCertificate *ndky_s_Certptr;

  WT_KeyDescriptor *ndky_e_Publicptr;
  WT_Key_Data *ndky_e_Dataptr;
  WT_KeyCertificate *ndky_e_Certptr;

  WT_KeyDescriptor *SubCAPublicptr;
  WT_Key_Data *SubCADataptr;
  WT_KeyCertificate *SubCACertptr;
  uchar *t_Data = NULL;
  int t_Data_size;
  uchar pkey[WT_P192_PUBLIC_LENGTH + 1];
  int pkey_ln;
  uchar d_data[WDC_DIGEST_LEN];
  unsigned int d_data_len;
  ushort temp1, temp2;

  if(wnc_sz != sizeof(WT_Node_Certificate)) return ENKI_ERR_STRUCT_SZ;
  
  do
  {
    wtncptr = (WT_Node_Certificate *)malloc(sizeof(WT_Node_Certificate));
    if(!wtncptr) { status = ENKI_ERR_ALLOCATING_MEMORY; break; }

    memset((char*)wtncptr,0,sizeof(WT_Node_Certificate));
 
    memcpy((char*)wtncptr, wnc, sizeof(WT_Node_Certificate));

    ndky_s_Publicptr = &wtncptr->ndky_s_Public;
    ndky_s_Dataptr = &wtncptr->ndky_s_Data;
    ndky_s_Certptr = &wtncptr->ndky_s_Cert;

    ndky_e_Publicptr = &wtncptr->ndky_e_Public;
    ndky_e_Dataptr = &wtncptr->ndky_e_Data;
    ndky_e_Certptr = &wtncptr->ndky_e_Cert;

    SubCAPublicptr = &wtncptr->SubCAPublic;
    SubCADataptr = &wtncptr->SubCAData;
    SubCACertptr = &wtncptr->SubCACert;

    if(wtncptr->CertificateRootID == WT_RHO) pub_root_key = RhoPublicKey;
    else if(wtncptr->CertificateRootID == WT_PHI) pub_root_key = PhiPublicKey;
    else { status = ENKI_ERR_ROOT_KEY_ID; break; }

    M_RECON_LENGTH(temp1,SubCAPublicptr->KeyLength_msb,
		   SubCAPublicptr->KeyLength_lsb)
    M_RECON_LENGTH(temp2,SubCADataptr->KeyDataLenght_msb,
		   SubCADataptr->KeyDataLength_lsb)
    t_Data_size = (int)temp1 + (int)temp2;
    t_Data = (uchar *)T_malloc(t_Data_size) ;

    memcpy(t_Data,SubCAPublicptr->KeyValue,(int)temp1) ;
    memcpy(&t_Data[temp1],SubCADataptr->KeyData,(int)temp2) ;

    memset(d_data,0,WDC_DIGEST_LEN) ;

    if ((status = enki_generate_SHA1_message_digest(t_Data, t_Data_size,
		  d_data, &d_data_len)) != 0)
       break;

    pkey_ln = WT_P192_PUBLIC_LENGTH + 1;

    if ((status = enki_verify_signature (pub_root_key, pkey_ln,d_data,
		  d_data_len, SubCACertptr->KeySignatureByCertifyingKey,
		  WT_P192_SIGNATURE_LENGTH)) != 0)
       break;

    T_free(t_Data);

    M_RECON_LENGTH(temp1,ndky_s_Publicptr->KeyLength_msb,
		   ndky_s_Publicptr->KeyLength_lsb)
    M_RECON_LENGTH(temp2,ndky_s_Dataptr->KeyDataLenght_msb,
		   ndky_s_Dataptr->KeyDataLength_lsb)
    t_Data_size = (int)temp1 + (int)temp2;
    t_Data = (uchar *)malloc(t_Data_size) ;
    /* store NodeKey S Public for NodeKey S Private verification */
    *nk_s_pub_ln = (int)temp1;
    *nk_s_pub = T_malloc((int)temp1);
     T_memcpy(*nk_s_pub, ndky_s_Publicptr->KeyValue, (int)temp1);

    memcpy(t_Data,ndky_s_Publicptr->KeyValue,(int)temp1) ;
    memcpy(&t_Data[(int)temp1],ndky_s_Dataptr->KeyData,(int)temp2);
    memset(d_data,0,WDC_DIGEST_LEN) ;

    if ((status = enki_generate_SHA1_message_digest(t_Data, t_Data_size,
						    d_data,&d_data_len)) != 0)
       break;

    if(wtncptr->CertificateRootID == WT_RHO)
    {
        if ((status = enki_assert_udi(ndky_s_Dataptr->KeyData, (size_t)temp2, pid, sernum)) != 0)
           break;
    }
    else /* if(wtncptr->CertificateRootID == WT_PHI) */
    {
        if ((status = enki_assert_image_name(ndky_s_Dataptr->KeyData,
					     (size_t)temp2, image_name)) != 0)
           break;
    }

    pkey_ln = WT_P192_PUBLIC_LENGTH + 1;

    pkey[0] = 0x4;
    memcpy(&pkey[1], SubCAPublicptr->KeyValue, WT_P192_PUBLIC_LENGTH);

    if ((status = enki_verify_signature (pkey, pkey_ln,d_data, d_data_len,
		  ndky_s_Certptr->KeySignatureByCertifyingKey,
		  WT_P192_SIGNATURE_LENGTH)) != 0)
       break;

    T_free(t_Data);

    M_RECON_LENGTH(temp1,ndky_e_Publicptr->KeyLength_msb,
		   ndky_e_Publicptr->KeyLength_lsb)
    M_RECON_LENGTH(temp2,ndky_e_Dataptr->KeyDataLenght_msb,
		   ndky_e_Dataptr->KeyDataLength_lsb)
    t_Data_size = (int)temp1 + (int)temp2;

    t_Data = (uchar *)T_malloc(t_Data_size);
    /* store NodeKey E Public for NodeKey E Private verification */
    *nk_e_pub_ln = (int)temp1;    
    *nk_e_pub = T_malloc((int)temp1);
     T_memcpy(*nk_e_pub, ndky_e_Publicptr->KeyValue, (int)temp1);

    memcpy(t_Data,ndky_e_Publicptr->KeyValue,(int)temp1) ;
    memcpy(&t_Data[(int)temp1],ndky_e_Dataptr->KeyData,(int)temp2);
    memset(d_data,0,WDC_DIGEST_LEN) ;

    if ((status = enki_generate_SHA1_message_digest(t_Data, t_Data_size,
						    d_data,&d_data_len)) != 0)
       break;

    if(wtncptr->CertificateRootID == WT_RHO)
    {
        if ((status = enki_assert_udi(ndky_e_Dataptr->KeyData, (size_t)temp2, pid, sernum)) != 0)
           break;
    }
    else /* if(wtncptr->CertificateRootID == WT_PHI) */
    {
        if ((status = enki_assert_image_name(ndky_e_Dataptr->KeyData, (size_t)temp2, image_name)) != 0)
           break;
    }


    pkey_ln = WT_P192_PUBLIC_LENGTH + 1;

    pkey[0] = 0x4;
    memcpy(&pkey[1], SubCAPublicptr->KeyValue, WT_P192_PUBLIC_LENGTH);

    if ((status = enki_verify_signature (pkey, pkey_ln,d_data, d_data_len, ndky_e_Certptr->KeySignatureByCertifyingKey, WT_P192_SIGNATURE_LENGTH)) != 0)
       break;

  } while(0);

  T_free(t_Data);
  T_free((uchar *)wtncptr);
  return status;
}

/*-----------------------------------------------------------------------
NAME: enki_api_verify_WT_Node_Certificate
  
DESCRIPTION: Verifies WT_Node_Certificate
  
PARAMETERS:  wnc_tlv: Pointer to WNC TLV buffer
             wnc_tlv_sz: Length of WNC TLV buffer
             pid: Product ID (if using WDC)
             sernum: Serial number (if using WDC)
             image_name: image name (if using WIC)
             wt_data_h - handle created by calling enki_api_init
  
RETURNS:     0 if success, non-zero if failure
             nk_s_pub: pointer to the NodeKey S Public buf
             nk_s_pub_ln: Length of the NodeKey S Public
             nk_e_pub: pointer to the NodeKey E Public buf
             nk_e_pub_ln: Length of the NodeKey E Public

-------------------------------------------------------------------------*/
int
enki_api_verify_WT_Node_Certificate(uchar *wnc_tlv, uint wnc_tlv_sz,
    uchar** nk_s_pub, uint *nk_s_pub_ln, uchar** nk_e_pub, uint *nk_e_pub_ln,
    char *pid, char *sernum, char* image_name, wt_data_handle wt_data_h)
{
   int status = 0;
   wt_data * wt_data_ptr = NULL;

   do
   {
     status = enki_api_crypto_data_fetch(wt_data_h,&wt_data_ptr);

     if(wt_data_ptr == NULL)
     {
       status = ENKI_ERR_INVALID_HANDLE;
       break;
     }
     if(status) break;

     if(!(ASSERT_TLV(wnc_tlv, WT_TLV_WDC)||ASSERT_TLV(wnc_tlv, WT_TLV_WIC)))
     {
       status = ENKI_ERR_TLV_INVALID;
       break;
     }
     if(!ASSERT_TLV_SZ(wnc_tlv,wnc_tlv_sz))
     {
       status = ENKI_ERR_TLV_SIZE;
       break;
     }

     status = enki_verify_WNC_data(GET_TLV_VAL(wnc_tlv), GET_TLV_LEN(wnc_tlv),
              &wt_data_ptr->publicKey, nk_s_pub_ln, &wt_data_ptr->publicKey2,
	      nk_e_pub_ln, pid, sernum, image_name);
     if (status) break;
     
     *nk_s_pub = wt_data_ptr->publicKey;
     *nk_e_pub = wt_data_ptr->publicKey2;
     
   }while(0);
           
   if(status != 0 && wt_data_ptr)
     wt_data_ptr->errorCode = status;

   return status;
}

/*-----------------------------------------------------------------------
NAME: enki_api_close

DESCRIPTION: frees up memory allocated by enki_api_init; 

PARAMETERS: wt_handle - the WT_DATA_HANDLE to free up.  
        
RETURNS: int = STATUS of call
       0 = sucessful error info retrieval 
       non zero = failure , error code

MEMORY MANAGMENT: User MUST CALL this method to free up
                  memory allocated to handle by the system. 
                  Think of this method as free and
                  enki_api_init as malloc. 
-------------------------------------------------------------------------*/
int
enki_api_close(wt_data_handle wt_handle_h)
{
    wt_data* tmpPtr = NULL;
    if(wt_data_list_count > 0){
        tmpPtr = (wt_data*)wt_data_list_start[wt_handle_h -1];
        if(tmpPtr != NULL){
        /*Free the elements member variables*/
           if(tmpPtr->publicKey != NULL){
               T_free(tmpPtr->publicKey);
           }
           if(tmpPtr->publicKey2 != NULL){
               T_free(tmpPtr->publicKey2);
           }
           if(tmpPtr->privateKey != NULL){
               T_free(tmpPtr->privateKey);
           }
           if(tmpPtr->signature != NULL){
               T_free(tmpPtr->signature);
           }
           if(tmpPtr->digestedData!= NULL){
               T_free(tmpPtr->digestedData);
           }
           if(tmpPtr->decryptedData != NULL){
               T_free(tmpPtr->decryptedData);
           }
           if(tmpPtr->signatureInfo!= NULL){
               T_free((unsigned char*)tmpPtr->signatureInfo);
           }
           if(tmpPtr->cipherData!= NULL){
               T_free((unsigned char*)tmpPtr->cipherData);
           }
           if(tmpPtr->decipherInfo!= NULL){
               T_free((unsigned char*)tmpPtr->decipherInfo);
           }
           if(tmpPtr->signList!= NULL){
               T_free((unsigned char*)tmpPtr->signList);
           }
           if(tmpPtr->errorMsg != NULL){
               T_free((unsigned char*)tmpPtr->errorMsg);
           }
           if(tmpPtr->scratch1 != NULL){
               T_free((unsigned char*)tmpPtr->scratch1);
           }
           if(tmpPtr->scratch2 != NULL){
               T_free((unsigned char*)tmpPtr->scratch2);
           }
           if(tmpPtr->scratch3 != NULL){
               T_free((unsigned char*)tmpPtr->scratch3);
           }
           if(tmpPtr->scratch4 != NULL){
               T_free((unsigned char*)tmpPtr->scratch4);
           }
        }
        T_free((unsigned char*)wt_data_list_start[wt_handle -1]);
        wt_data_list_start[wt_handle -1] = NULL;
        wt_data_list_count--;
    }
    if(wt_data_list_count == 0){
        T_free((unsigned char*)wt_data_list_start);
        wt_data_list_start = NULL;
        wt_data_block_count = 0;
    }
  return 0;
}

/*---------------------------------------------------------------------------
 * strTrim 
 *
 * DESCRIPTION:
 *
 * Routine to trim the trailing blanks.
 *
 * PARAMETERS:
 *      s - pointer to the string
 * RETURN:
 *      string to be trimmed
 *--------------------------------------------------------------------------*/

static char *
strTrim (char *s)
{
    char *c = s + strlen(s);
    while (*(--c) == ' ' && c >= s)
        *c = '\0';
    return (s);
}

/*---------------------------------------------------------------------------
 * verify_credential
 *
 * DESCRIPTION:
 *     This function verifies the credential 
 * 
 * PARAMETERS:
 *     pcb_serial_num - array to hold PCB Serial number
 *     product_id - array to hold product ID
 *     cred - array to hold credential
 *     size - size of the credential
 *
 * RETURNS:
 *     PASSED - if successful
 *     FAILED - if unsuccessful
 *--------------------------------------------------------------------------*/
int
verify_credential(char *pcb, char *pid, uchar *cred, int size)
{

    uint wdctlv_sz; 
    int status = 0;
    uchar *wdctlv, *cred_start;

    /* The first two bytes are the length */
    cred_start = &cred[2];

    pid = strTrim(pid); /* Trim out product ID blanks */

    if ((NVRAM)->diagflag & D_VERBOSE) {
	printf("\nVerifying the Device Certificate for -\n\n");
	printf("<UDI><PID>%s</PID><SN>%s</SN></UDI>\n",pid,pcb);
	enki_PrintBuf("Watchtower Device Certificate",cred_start, size);
    }
    
    /* First open the api */
    if (enki_api_init(&wt_handle)) {
	return (FAILED);
    }

    /* Get pointer to WDC, TauQs and TauQe */
    status = enki_api_extract_wnc_tlv_componets(cred_start, size,
	       &wdctlv, &wdctlv_sz, &tauqs_tlv, &tauqs_tlv_sz, &tauqe_tlv,
	       &tauqe_tlv_sz, wt_handle);

    if(status) {
	printf("Error-%d\n",status);
	return (status);
    }


    status = enki_api_verify_WT_Node_Certificate(wdctlv, wdctlv_sz, &tauspub,
	     &tauspub_sz, &tauepub, &tauepub_sz, pid, pcb, NULL, wt_handle);

    if(status) {
	printf("Error-%d\n",status);
	return (status);
    }
    
    printf("\nPassed: Device Certificate was successfully verified\n") ;
    
    /*
      close the api
      closing the api frees up the memory used
    */

    enki_api_close(wt_handle);

    return (PASSED);
}
/*-----------------------------------------------------------------------
 * raw_base64_decode 
 * DESCRIPTION: This is a utility/private function.
 * use spc_base64_decode for b64 decoding
 ------------------------------------------------------------------------*/
static unsigned int
raw_base64_decode(unsigned char *in, unsigned char *out, 
                                     int strict, int *err) {
  unsigned int  result = 0, x;
  unsigned char buf[3], *p = in, pad = 0;
   
  *err = 0;
  while (!pad) {
    switch ((x = b64revtb[*p++])) {
      case -3: /* NULL TERMINATOR */
        if (((p - 1) - in) % 4) *err = 1;
        return result;
      case -2: /* PADDING CHARACTER. INVALID HERE */
        if (((p - 1) - in) % 4 < 2) {
          *err = 1;
          return result;
        } else if (((p - 1) - in) % 4 == 2) {
          /* Make sure there's appropriate padding */
          if (*p != '=') {
            *err = 1;
            return result;
          }
          buf[2] = 0;
          pad = 2;
          result++;
          break;
        } else {
          pad = 1;
          result += 2;
          break;
        }
        return result;
      case -1:
        if (strict) {
          *err = 2;
          return result;
        }
        break;
      default:
        switch (((p - 1) - in) % 4) {
          case 0:
            buf[0] = x << 2;
            break;
          case 1:
            buf[0] |= (x >> 4);
            buf[1] = x << 4;
            break;
          case 2:
            buf[1] |= (x >> 2);
            buf[2] = x << 6;
            break;
          case 3:
            buf[2] |= x;
            result += 3;
            for (x = 0;  x < 3 - pad;  x++) *out++ = buf[x];
            break;
        }
        break;
    }
  }
  for (x = 0;  x < 3 - pad;  x++) *out++ = buf[x];
  return result;
}

/*------------------------------------------------------------------------
 * get_sub_tlv_val
 *
 * DESCRIPTION: get the value part of a TLV item from a list of TLVs by
 * providing its type
 * PARAMETERS:
 *     hdr_tlv - tlv array
 *     type - type of TLV
 * RETURNS: if no TLV found for the given type, 0 is returned
 *------------------------------------------------------------------------*/
uchar *
get_sub_tlv_val(const uchar *hdr_tlv, const ushort type) 
{
    uchar *p = get_sub_tlv(hdr_tlv, type);
    return p?GET_TLV_VAL(p):0;
}
/*-----------------------------------------------------------------------
 * spc_base64_decode
 *
 * DESCRIPTION:
 * If err is non-zero on exit, then there was an incorrect padding error.  We
 * allocate enough space for all circumstances, but when there is padding, or
 * there are characters outside the character set in the string (which we are
 * supposed to ignore), then we end up allocating too much space.  You can
 * realloc(  ) to the correct length if you wish.  In most cases it is 
 * undesireable to silently ignore unecessary charaters unless you have to.
 * To disable the ignoring pass a non-zero value for the strict prameter and
 * if an unrecognizeable character is encouterd the function will return an
 * error.
 */
unsigned char *
spc_base64_decode(unsigned char *buf, size_t *len, int strict,
                             int *err) {
  unsigned char *outbuf;
   
  outbuf = T_malloc(3 * (strlen((char*)buf) / 4 + 1));
  if (!outbuf) {
    *err = -3;
    *len = 0;
    return 0;
  }
  *len = raw_base64_decode(buf, outbuf, strict, err);
  if (*err) {
    T_free(outbuf);
    *len = 0;
    outbuf = 0;
  }
  return outbuf;
}



/*-------------------------------------------------------------------------
NAME: enki_api_verify_Del_Approval 

DESCRIPTION: generate Del_Approval

PARAMETERS: 
            pid - pointer to a buffer containing the PID i.e Base PID. 
            pid_size - size of the PID stored in pid buffer. 
            sernum - pointer to a buffer containing the device serial number 
            sernum_size - size of the serial number contained in the sernum
	    buffer
            del_request - pointer to the 64 bytes random string Del_Request
            del_request_ln - size of Del_Request
            del_approval - pointer to a base 64 encoded TLV carrying
	                   Sig_Del_Rnd,
                           Del_Request_Signature
                        
        
RETURNS: int = STATUS of call
       0 = sucessful verification
       non zero = failure , error code
          
---------------------------------------------------------------------------*/
int
enki_api_verify_Del_Approval(const uchar *pid, const int pid_ln,
			     const uchar *sernum, const int sernum_ln,
			     const uchar *del_request,
			     const int del_request_ln,
			     const char* del_approval)
{
    uchar *del_approval_tlv=0;
    uchar *buf=0;
    int status=0, buf_ln, len, err;

    do
    {
        uchar d_data[WT_SHA1_LENGTH];
        unsigned int d_data_len;
        uchar *sig_del_rnd_tlv, *sig_del_rnd;
        int sig_del_rnd_ln;
        WT_Signature *del_request_signature;
        WT_KeyDescriptor *mu_pub_key;
        WT_KeyCertificate *mu_cert;
        int mu_pub_key_len;
        uchar pkey[WT_P192_PUBLIC_LENGTH + 1];
        int pkey_ln;

        del_approval_tlv = spc_base64_decode((unsigned char *)del_approval,
					     (size_t*)&len, 0, &err);
        if(!del_approval_tlv) { status = ENKI_ERR_BASE64_ENCODING; break; }
        if(!ASSERT_TLV(del_approval_tlv, WT_TLV_HEADER))
	    { status = ENKI_ERR_TLV_INVALID; break; }

        sig_del_rnd_tlv = get_sub_tlv(del_approval_tlv, WT_TLV_SIG_DEL_RND);
        if(!sig_del_rnd_tlv) { status = ENKI_ERR_TLV_INVALID; break; }
        else
        {
            sig_del_rnd = GET_TLV_VAL(sig_del_rnd_tlv);
            sig_del_rnd_ln = GET_TLV_LEN(sig_del_rnd_tlv);
        }

        del_request_signature =
	    (WT_Signature*)get_sub_tlv_val(del_approval_tlv,
					   WT_TLV_SIGNATURE);
        if(!del_request_signature) { status = ENKI_ERR_TLV_INVALID; break; }

        mu_pub_key = (WT_KeyDescriptor*)get_sub_tlv_val(del_approval_tlv,
						      WT_TLV_KEY_DESCRIPTOR);
        if(!mu_pub_key) { status = ENKI_ERR_TLV_INVALID; break; }

        mu_cert = (WT_KeyCertificate*)get_sub_tlv_val(del_approval_tlv,
						      WT_TLV_KEY_CERT) ;
        if(!mu_cert) { status = ENKI_ERR_TLV_INVALID; break; }

        /* concatenate pid + sernum + Del_Request + Sig_Del_Rnd */
        buf_ln = pid_ln + sernum_ln + del_request_ln + sig_del_rnd_ln;
        buf = T_malloc(buf_ln);
        T_memcpy(buf, (uchar*)pid, pid_ln);
        T_memcpy(buf+pid_ln, (uchar*)sernum, sernum_ln);
        T_memcpy(buf+pid_ln+sernum_ln, (uchar*)del_request, del_request_ln);
        T_memcpy(buf+pid_ln+sernum_ln+del_request_ln, (uchar*)sig_del_rnd,
		 sig_del_rnd_ln);

        /* generate a SHA1 hash */
        status = enki_generate_SHA1_message_digest(buf, buf_ln, d_data,
						   &d_data_len);
        if (status) break;

        M_RECON_LENGTH(mu_pub_key_len, mu_pub_key->KeyLength_msb,
		       mu_pub_key->KeyLength_lsb);

        pkey_ln = mu_pub_key_len + 1;
        memset(pkey,0,pkey_ln) ;
        pkey[0] = 0x4;
        memcpy(&pkey[1], mu_pub_key->KeyValue, mu_pub_key_len);

        /* verify the signature using Mu public key */
        status = enki_verify_signature (pkey, pkey_ln,
                       d_data, d_data_len, del_request_signature->Signature,
		       sizeof(del_request_signature->Signature));
        if (status) break;

        /* verify Mu public key */
        status = enki_generate_SHA1_message_digest(mu_pub_key->KeyValue,
		 WT_P192_PUBLIC_LENGTH, d_data, &d_data_len);
        if (status) break;

        status = enki_verify_signature(LambdaPublicKey,
		 WT_P192_BSAFE_PUBLIC_LENGTH,
                 d_data, d_data_len, mu_cert->KeySignatureByCertifyingKey,
		 WT_P192_SIGNATURE_LENGTH);
        if (status) break;


    }while(0);

    /* free memory allocated by spc_base64_decode() */
    T_free(del_approval_tlv);
    T_free(buf);

    return status;
}

/*---------------------------------------------------------------------------
 * verify_rma_deletion
 *
 * DESCRIPTION:
 *     This function verifies the credential 
 * 
 * PARAMETERS:
 *     sn - array to hold PCB Serial number
 *     pid - array to hold product ID
 *     del_request - array to hold 32 random bytes for request RMA
 *     del_request_sz - size of del_request
 *     del_approval - array to hold RMA approval
 *     del_approval_sz - size of del_approval 
 *
 * RETURNS:
 *     PASSED - if successful
 *     FAILED - if unsuccessful
 *--------------------------------------------------------------------------*/
int
verify_rma_deletion(char *sn, char *pid, uchar *del_request,
		    int del_request_sz, uchar *del_approval,
		    int del_approval_sz)
{
    int status = 0;

    pid = strTrim(pid);
    
    if ((NVRAM)->diagflag & D_VERBOSE) {
	printf("\nVerifying the Del_Approval -\n\n") ;
	printf("<UDI><PID>%s</PID><SN>%s</SN></UDI>\n",pid,sn) ;
	
	enki_PrintBuf((char*)"Del_Request",(uchar*)del_request,
		      del_request_sz);
	enki_PrintBuf((char*)"Del_Approval",(uchar*)del_approval,
		      del_approval_sz);
    }
    
    /* Verify Del_Approval using complete WDC TLV + TauQ S and E TLV */
    status = enki_api_verify_Del_Approval((const uchar*)pid, strlen(pid),
	     (const uchar*)sn, strlen(sn), del_request, del_request_sz,
	     (char *)del_approval);
    if(status) {
	printf("Error-%d\n",status);
	return (FAILED);
    }
    
    
    printf("\nPassed: RMA Deletion was successfully verified\n") ;

    return (PASSED);
}

/* end of crypto_credential.c */
/******** History ********
$Log: crypto_credential.c,v $
Revision 1.5  2014/02/18 09:11:12  alpeng
CSCul88171-3: remove useless files: mon_boot.h, c82576_ethmap.h, eth_frames.c, stack.h, sys_regs.h

Revision 1.4  2013/11/26 08:40:33  hroni
fix compiler warning

Revision 1.3  2012/06/06 09:48:03  aarwang
- Clean up compiler warnings.

Revision 1.2  2012/03/28 00:38:13  mcharon
remove forward slash from second line

Revision 1.1.1.1  2012/03/23 23:02:05  ptong
Initial archive of ng_diag module


$Endlog$
*/
