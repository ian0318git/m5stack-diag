/* $Id: bsfplatf.h,v 1.2 2012/03/28 00:38:09 mcharon Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/include/bsfplatf.h,v $
 *------------------------------------------------------------------
 * bsfplatf.h  - Ported from IOS for Quack.
 *
 * Copyright (c) 2003-2012 by cisco Systems, Inc.
 * All rights reserved.
 *
 * Author: Huan Ngo
 *------------------------------------------------------------------
 */


/* Copyright (c) RSA Security Inc., 1997-1999.  All rights reserved.  
 * This work contains proprietary, confidential, and trade secret 
 * information of RSA Security Inc.  Use, disclosure or reproduction 
 * without the express written authorization of RSA Security Inc. is
 * prohibited.
 */

#ifndef _BSAFE_PLATFORM_H_
#define _BSAFE_PLATFORM_H_ 1

/* The include file bsfmacro.h contains all the alternatives for macro
     definitions. */
#include "bsfmacro.h"

/* 
	If RSA_BSAFE_EXPORT is defined generate an export version of
	BSAFE.
*/
#ifdef RSA_BSAFE_EXPORT
#define RSA_BSAFE_DESTINATION  RSA_EXPORT_VERSION
#else
#define RSA_BSAFE_DESTINATION  RSA_DOMESTIC_VERSION
#endif

#if RSA_PLATFORM == RSA_I386_486
#define RSA_PLATFORM_SPECIFIC
#include "bsplti32.h"
#endif

#if RSA_PLATFORM == RSA_LINUX
#define RSA_PLATFORM_SPECIFIC
#include "bslinux.h"
#endif

#if RSA_PLATFORM == RSA_WINTEL_IA64
#define RSA_PLATFORM_SPECIFIC
#include "bspia64.h"
#endif

#if (RSA_PLATFORM == RSA_HP_PA1 || RSA_PLATFORM == RSA_HP_PA2)
#define RSA_PLATFORM_SPECIFIC
#include "bshp1020.h"
#endif

#if (RSA_PLATFORM == RSA_HP_64)
#define RSA_PLATFORM_SPECIFIC
#include "bshp64.h"
#endif

#if RSA_PLATFORM == RSA_LINUX_IA64
#define RSA_PLATFORM_SPECIFIC
#include "bslinuxia64.h"
#endif

#if (RSA_PLATFORM == RSA_HP_IA64)
#define RSA_PLATFORM_SPECIFIC
#include "bshpia64.h"
#endif

#if (RSA_PLATFORM == RSA_HP_IA64_32)
#define RSA_PLATFORM_SPECIFIC
#include "bshpia64_32.h"
#endif

#if (RSA_PLATFORM == RSA_MAC_68K_CODE_WARRIOR)
#define RSA_PLATFORM_SPECIFIC
#include "bsmac68k.h"
#endif
 
#if (RSA_PLATFORM == RSA_MAC_PPC_CODE_WARRIOR)
#define RSA_PLATFORM_SPECIFIC
#include "bsmacppc.h"
#endif

#if RSA_PLATFORM == RSA_SPARC_SUN_SOLARISV8
#define RSA_PLATFORM_SPECIFIC
#include "bsparcv8.h"
#endif

#if RSA_PLATFORM == RSA_SPARC_SUN_SOLARISV9
#define RSA_PLATFORM_SPECIFIC
#include "bsparcv9.h"
#endif

#if RSA_PLATFORM == RSA_IBM_AIX32
#define RSA_PLATFORM_SPECIFIC
#include "bsaix32.h"
#endif

#if RSA_PLATFORM == RSA_IBM_AIX64
#define RSA_PLATFORM_SPECIFIC
#include "bsaix64.h"
#endif


#ifndef RSA_PLATFORM_SPECIFIC

#ifdef __cplusplus
extern "C" {
#endif

/* Is this BSAFE to use full, domestic strength key sizes or export
     strength? */
#define RSA_BSAFE_DESTINATION  RSA_DOMESTIC_VERSION

/* The default calling convention is C, the macro RSA_CALLING_CONV should be
     "nothing".
     If the application is using the PASCAL calling convention then
     RSA_CALLING_CONV should be defined approrpriately so that the
     application can call the public API accordingly.
   For example, the PASCAL calling convention on Windows 95 is

#define RSA_CALLING_CONV ???

 */
#define RSA_CALLING_CONV
/* temporary, when all files have converted, take this next line out. */
#define CALL_CONV

/* RSA_GLOBAL_FUNCTION_POINTERS should be set to RSA_ENABLED if and only if
     the compiler can intitialize global or static function pointers.
     If not (such as with MAC code resource), function pointers can be
     set only at run time through an initializing routine. If that is the
     case, set RSA_GLOBAL_FUNCTION_POINTERS to RSA_DISABLED.
 */
#define RSA_GLOBAL_FUNCTION_POINTERS RSA_ENABLED

/* Set RSA_INTERMEDIATE_STATE to RSA_DISABLED if you do NOT want to include
     the code that finds intermediate states for cloning/serialization
     purposes. For instance, an object that has been initialized for RC4
     can be "serialized". Call B_GetAlgorithmState. This returns a byte
     array that is the RC4 state. With this byte array, B_SetAlgorithmState
     will build an object that is in the same state as the original.
   If this flag is "enabled", code size increases. If a customer wants to
     delete this feature to keep code size down, disable the flag.
 */
#define RSA_INTERMEDIATE_STATE RSA_ENABLED

/* Set RSA_ENDIAN to RSA_LITTLE_ENDIAN for little endian machines (Intel,
     for instance). Set it to RSA_BIG_ENDIAN for big endian machines (Mac and
     most UNIX, for example).
 */
#define RSA_ENDIAN RSA_BIG_ENDIAN

/* Set RSA_FETCH_UINT4 to RSA_FETCH_ALIGNED_ONLY if the CPU can fetch UINT4
     values only from byte-aligned addresses. Set it to RSA_FETCH_UNALIGNED
     if it is possible to fetch from unaligned addresses. Unaligned fetches
     enables speedups in the handling of 4 and 8 byte quantities (e.g., in
     CBC). Old Sun SPARC chips, for instance, cannot do this.
 */
#define RSA_FETCH_UINT4 RSA_FETCH_ALIGNED_ONLY

/* RSA_REGISTER_SIZE defines the bit size of a register, or word. For instance,
     a Pentium's register is 32 bits, so set the value to RSA_32_BIT_REGISTER.
     On the other hand, an Alpha's register is 64 bits, so set the value to
     RSA_64_BIT_REGISTER.
 */
#define RSA_REGISTER_SIZE RSA_32_BIT_REGISTER

/* If the platform accepts prototyping in the function definition, set
     RSA_PROTOTYPES to RSA_ENABLED. If not, set the value to RSA_DISABLED.
 */
#define RSA_PROTOTYPES RSA_ENABLED

/* PROTO_LIST is defined depending on how RSA_PROTOTYPES is defined. If
     enabled, then PROTO_LIST returns the list, otherwise it returns an
     empty list.
 */
#if RSA_PROTOTYPES == RSA_ENABLED
#define PROTO_LIST(list) list
#endif
#if RSA_PROTOTYPES == RSA_DISABLED
#define PROTO_LIST(list) ()
#endif

/* On the outside chance that some strange platform exists that does not
     use 8-bit bytes, define BITS_PER_BYTE.
 */
/* pritikin - added #ifdef */
#ifndef RSA_BITS_PER_BYTE
#define RSA_BITS_PER_BYTE 8
#endif

/* We will create a type (through typedef) that is a time type. It will be
     some sort of int (seconds since 1970). Right now, almost all time
     int's are 32 bits. In the future, some platforms may use a 64 bit
     value. An RSA_TIME_T will be a UINT4, unless someone wants to change
     to something else. If so, redefine RSA_TIME_TYPE and add the typedef
     (which is in aglobal.h).
 */
#define RSA_TIME_TYPE RSA_32_BIT_TIME

/* The following are all the optimizations we have made so far. An optimization
     may exist on a certain platform or it may not. If the optimization exists,
     set the macro to the platform macro. If not leave it as C code.
     For instance, on Alpha NT, there is a CMP multiply  optimization, but
     no MD5 transform optimization. Hence, set
     RSA_CMP_MULT_WORDS_OPT to RSA_ALPHA_DEC_NT_MSVC40
     but leave
     RSA_MD5_TRANSFORM_OPT at RSA_C_CODE
 */
#define RSA_CMP_WORD_SIZE         RSA_C_CODE
#define RSA_CMP_MULT_WORDS_OPT    RSA_C_CODE
#define RSA_CMP_VECTOR_MULT_OPT   RSA_C_CODE
#define RSA_CMP_ADD_IN_TRACE_OPT  RSA_C_CODE
#define RSA_CMP_MONT_PRODUCT_OPT  RSA_C_CODE
#define RSA_CMP_MONT_SQUARE_OPT   RSA_C_CODE
#define RSA_CMP_MULTIPLY_OPT	  RSA_C_CODE
#define RSA_CMP_PA2_0_TEST	  RSA_C_CODE

#define RSA_DES_ENCRYPT_OPT			RSA_C_CODE
#define RSA_DES_INIT_ENCRYPT_OPT		RSA_C_CODE
#define RSA_DES_INIT_DECRYPT_OPT		RSA_C_CODE		
	
#define RSA_RC2_ENCRYPT_OPT  RSA_C_CODE
#define RSA_RC2_DECRYPT_OPT  RSA_C_CODE
#define RSA_RC2_INIT_OPT  RSA_C_CODE	

#define RSA_RC4_UPDATE_OPT  RSA_C_CODE

/*
	RC5 - 32
*/
#define RSA_RC5_INIT_OPT				RSA_C_CODE
/* May set = RSA_DISABLED to revert to Pre-V5.1 C-language procedure 
           = RSA_ENABLED to invoke faster V5.1 C-language procedures */  
#define RSA_RC5_12_ROUND_INIT_OPTS		RSA_ENABLED
#define RSA_RC5_16_ROUND_INIT_OPTS		RSA_ENABLED

/* May set = RSA_DISABLED to revert to Pre-V5.1 C-language procedure 
           = RSA_ENABLED to invoke faster V5.1 C-language procedures */  
#define RSA_RC5_ENCRYPT_OPT	RSA_C_CODE
#define RSA_RC5_12_ROUND_ENCRYPT_OPT RSA_ENABLED
#define RSA_RC5_16_ROUND_ENCRYPT_OPT RSA_ENABLED

/*
	RC5 - 64
*/
#define RSA_RC5_64_ENCRYPT_OPT RSA_C_CODE
#define RSA_RC5_64_DECRYPT_OPT RSA_C_CODE
#define RSA_RC5_64_INIT_OPT RSA_C_CODE

/*
	RC6
*/
#define RSA_RC6_INIT_OPT            RSA_C_CODE
/* May set = RSA_DISABLED to revert to Pre-V5.1 C-language procedure 
           = RSA_ENABLED to invoke faster V5.1 C-language procedures */ 
#define RSA_RC6_20_ROUND_INIT_OPTS	RSA_ENABLED
#define RSA_RC6_ENCRYPT_OPT RSA_C_CODE

/*
	AES
*/
#define RSA_AES_INIT_OPT			RSA_ENABLED
#define RSA_AES_ENCRYPT_OPT			RSA_C_CODE
#define RSA_AES_DECRYPT_OPT			RSA_C_CODE


#define RSA_MD5_TRANSFORM_OPT  RSA_C_CODE

#define RSA_MAC_UPDATE_OPT  RSA_C_CODE

#define RSA_SHA1_UPDATE_OPT	RSA_C_CODE
 
#define RSA_CMP_EC_EVEN_OPT RSA_C_CODE

/* __rotl is not available on this compiler for in-line rotate speed-ups */
#define ROTL_COMPILER_INTRINSIC  RSA_DISABLED
/* __rotr is not available on this compiler for in-line rotate speed-ups */
#define ROTR_COMPILER_INTRINSIC  RSA_DISABLED

/* The following are included to smooth the transition. Take them out
     when all files are converted to the new style.
 */
    
/* pritikin - added ifndef, and now removed
#ifndef BITS_PER_BYTE   
#define BITS_PER_BYTE  RSA_BITS_PER_BYTE
#endif
*/
#define RSA_PRIME_BITS(modulusBits)  (((modulusBits) + 1) / 2)
#define RSA_PRIME_LEN(modulusBits)   \
  ((RSA_PRIME_BITS (modulusBits) + RSA_BITS_PER_BYTE - 1) \
                                                   / RSA_BITS_PER_BYTE)
#define BITS_TO_LEN(modulusBits)     \
  (((modulusBits) + RSA_BITS_PER_BYTE - 1) / RSA_BITS_PER_BYTE)
#define MAX_RSA_MODULUS_BITS         2048
#define MIN_RSA_MODULUS_BITS         256
#define MIN_STRONG_RSA_MODULUS_BITS  432	
#define A_MIN_DIGEST_LEN             16
#define A_MAX_DIGEST_LEN             32
#define GLOBAL_FUNCTION_POINTERS     1

#ifdef __cplusplus
}

#ifndef INTELx86
#define INTELx86 0
#endif

#endif
#endif /* RSA_PLATFORM == RSA_C_CODE */
#endif /* _BSAFE_PLATFORM_H_ */

/******* HISTORY ********
$Log: bsfplatf.h,v $
Revision 1.2  2012/03/28 00:38:09  mcharon
remove forward slash from second line

Revision 1.1.1.1  2012/03/23 23:02:03  ptong
Initial archive of ng_diag module


$Endlog$
*/
