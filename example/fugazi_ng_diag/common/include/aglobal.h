/* $Id: aglobal.h,v 1.2 2012/03/28 00:38:09 mcharon Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/include/aglobal.h,v $
 *------------------------------------------------------------------
 * aglobal.h - Ported over from IOS for Quack.
 *
 * Copyright (c) 2003-2012 by cisco Systems, Inc.
 * All rights reserved.
 *
 * Author: Huan Ngo
 *------------------------------------------------------------------
 */

/*
 * This work contains proprietary, confidential, and trade secret 
 * information of RSA Security Inc.  Use, disclosure or reproduction 
 * without the express written authorization of RSA Security Inc. is
 * prohibited.
 */

#ifndef _AGLOBAL_H_
#define _AGLOBAL_H_ 1

#include "bsfmacro.h"
#include "bsfplatf.h"

#ifdef __cplusplus
extern "C" {
#endif

/* POINTER defines a generic pointer type */
typedef unsigned char *POINTER;

/* UINT2 defines a two byte word */
typedef unsigned short int UINT2;

/* pritikin - added _ITEM_ ifndef */
#ifndef _ITEM_
#define _ITEM_ 1
typedef struct {
  unsigned char *data;
  unsigned int len;
} ITEM;
#endif

typedef struct {
  int (RSA_CALLING_CONV *Surrender) PROTO_LIST ((POINTER));
  POINTER handle;
  POINTER reserved;
} A_SURRENDER_CTX;

/* UINT4 defines a four byte word */
#if RSA_REGISTER_SIZE == RSA_16_BIT_REGISTER || \
    RSA_REGISTER_SIZE == RSA_32_BIT_REGISTER
typedef unsigned long int UINT4;
#endif
#if RSA_REGISTER_SIZE == RSA_64_BIT_REGISTER
typedef unsigned int UINT4;
#ifndef NULL_PTR
#define NULL_PTR ((POINTER)0L)
#endif
#ifndef SIGNED_NULL_PTR
#define SIGNED_NULL_PTR ((char*)0L)
#endif
#ifdef __cplusplus
#define NULL_FUNCTION_PTR (0L)
#else
#define NULL_FUNCTION_PTR ((void *)0L)
#endif
#else
#ifndef NULL_PTR
#define NULL_PTR ((POINTER)0)
#endif
#ifndef SIGNED_NULL_PTR
#define SIGNED_NULL_PTR ((char*)0)
#endif
#ifdef __cplusplus
#define NULL_FUNCTION_PTR (0)
#else
#define NULL_FUNCTION_PTR ((void *)0)
#endif
#endif

#define UNUSED_ARG(x) x = *(&x);

#if RSA_TIME_TYPE == RSA_32_BIT_TIME
typedef UINT4 RSA_TIME_T;
#endif

/* Token flags (see B_KEY_ATTRIBUTES)
 */
#define TF_RESIDE_ON_TOKEN    0x0001
#define TF_PRIVATE            0x0002

/* KEY USAGE flags: used by both Crypto-C and Cert-C
 */
#define CF_DIGITAL_SIGNATURE  0x0100
#define CF_NON_REPUDIATION    0x0080
#define CF_KEY_ENCIPHERMENT   0x0040
#define CF_DATA_ENCIPHERMENT  0x0020
#define CF_KEY_AGREEMENT      0x0010
#define CF_KEY_CERT_SIGN      0x0008 
#define CF_CRL_SIGN           0x0004
#define CF_ENCIPHER_ONLY      0x0002
#define CF_DECIPHER_ONLY      0x0001

/* added as bitwise AND of CF_DIGITAL_SIGNATURE & CF_KEY_ENCIPHERMENT */    
#define CF_GENERAL_PURPOSE      0x0140 
    
#ifdef __cplusplus
}
#endif

#endif /* end _AGLOBAL_H_ */



#ifndef _AGLOBAL_H_
#define _AGLOBAL_H_ 1

#ifdef __cplusplus
extern "C" {
#endif

/* The default calling convension is C.  If the application using PASCAL
   calling conversion then CALL_CONV should define as c calling so that
   the application can call the public API accordingly.
 */
#ifndef CALL_CONV
#define CALL_CONV
#endif
  
/* PROTOTYPES should be set to one if and only if the compiler supports
     function argument prototyping.
   The following makes PROTOTYPES default to 1 if it has not already been
     defined as 0 with C compiler flags.
 */
#ifndef PROTOTYPES
#define PROTOTYPES 1
#endif

/* GLOBAL_FUNCTION_POINTERS should be set to 0 if and only if the compiler
   cann't intitialize global or static function pointers like building MAC
   code resource.
 */
#ifndef GLOBAL_FUNCTION_POINTERS
#define GLOBAL_FUNCTION_POINTERS 1
#endif

/* The CMP library may have optimized code. It may require defining the
     following symbols in the compiler flag.

      platform      |   compiler flag
  ------------------|-----------------------
     Intel 16-bit   |  -DINTELx86=1
     Intel 32-bit   |  -DINTELx86i32=1
     DEC Alpha      |  -DCMP_DEC_ALPHA=1
 */

/* INTELx86i32 should be set to 1 if and only if the library is built for
   Intel machine on 32-bit operating systems. For Intel optimized code base,
   either INTELx86i32 or INTELx86 will be defined, but not both.   You are
   not allowed to define both.
   Setting this define causes the assembly language speed-ups to
   by included in the library.
*/
#ifndef INTELx86i32
#define INTELx86i32 0
#endif  

/* INTELx86 should be set to 1 if and only if the machine is Intel base
 */
#ifndef INTELx86
#define INTELx86 0
#endif

/* BIG_ENDIAN defaults to 1 for Macintosh and UNIX where the machine 
   architecture stores the most significant byte in the lowest 
   memory address.
 */
#if INTELx86 || INTELx86i32
#define BIG_ENDIAN 0
#define CAN_FETCH_UNALIGNED 1
#endif

#ifndef BIG_ENDIAN
#define BIG_ENDIAN 1
#endif  

/* CAN_FETCH_UNALIGNED is default to 0 to indicate that the CPU can't fetch
   UNIT4 values from unaligned byte addresses.  This enables speedups
   in the handling of 4 and 8 byte quantities (e.g., in CBC handing).
   Old Sun SPARC chips cannot do this.
 */
#ifndef CAN_FETCH_UNALIGNED
#define CAN_FETCH_UNALIGNED 0
#endif

/* POINTER defines a generic pointer type */
typedef unsigned char *POINTER;

/* UINT2 defines a two byte word */
typedef unsigned short int UINT2;

/* UINT4 defines a four byte word */
#ifndef MACHINE64
typedef unsigned long int UINT4;
#else
typedef unsigned int UINT4;
#endif

#ifndef NULL_PTR
#define NULL_PTR ((POINTER)0)
#endif

#ifndef UNUSED_ARG
#define UNUSED_ARG(x) x = *(&x);
#endif

/* PROTO_LIST is defined depending on how PROTOTYPES is defined above.
   If using PROTOTYPES, then PROTO_LIST returns the list, otherwise it
     returns an empty list.  
 */
#if PROTOTYPES
#define PROTO_LIST(list) list
#else
#define PROTO_LIST(list) ()
#endif

#ifdef __cplusplus
}
#endif

#endif /* end _AGLOBAL_H_ */

/******* HISTORY ********
$Log: aglobal.h,v $
Revision 1.2  2012/03/28 00:38:09  mcharon
remove forward slash from second line

Revision 1.1.1.1  2012/03/23 23:02:03  ptong
Initial archive of ng_diag module


$Endlog$
*/
