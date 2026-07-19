/* $Id: stdlibrf.h,v 1.2 2012/03/28 00:38:12 mcharon Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/include/stdlibrf.h,v $
 *------------------------------------------------------------------
 * stdlibrf.h  - Ported from IOS for Quack.
 *
 * Copyright (c) 2003-2012 by cisco Systems, Inc.
 * All rights reserved.
 *
 * Author: Huan Ngo
 *------------------------------------------------------------------
 */

/* Copyright (c) RSA Security Inc., 1995-1999.  All rights reserved.  
 * This work contains proprietary, confidential, and trade secret 
 * information of RSA Security Inc.  Use, disclosure or reproduction 
 * without the express written authorization of RSA Security Inc. is
 * prohibited.
 */

#ifndef _STDLIBRF_H_
#define _STDLIBRF_H_ 1

#include "bsfmacro.h"
#include "bsfplatf.h"
#include "aglobal.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Routines supplied by the implementor.
 */

/* memory manipulation
 */
#if RSA_STD_MEM_FUNCS == RSA_ENABLED
void RSA_CALLING_CONV T_memset PROTO_LIST ((POINTER, int, unsigned int));
void RSA_CALLING_CONV T_memcpy PROTO_LIST ((POINTER, POINTER, unsigned int));
void RSA_CALLING_CONV T_memmove PROTO_LIST ((POINTER, POINTER, unsigned int));
int RSA_CALLING_CONV T_memcmp PROTO_LIST ((POINTER, POINTER, unsigned int));
#endif

/* memory allocation
 */
#if RSA_STD_ALLOC_FUNCS == RSA_ENABLED
POINTER RSA_CALLING_CONV T_malloc PROTO_LIST ((unsigned int));
POINTER RSA_CALLING_CONV T_realloc PROTO_LIST ((POINTER, unsigned int));
void RSA_CALLING_CONV T_free PROTO_LIST ((POINTER));
#endif

/* string manipulation functions
 */
#if RSA_STD_STRING_FUNCS == RSA_ENABLED
void RSA_CALLING_CONV T_strcpy PROTO_LIST ((char *, char *));
int RSA_CALLING_CONV T_strcmp PROTO_LIST ((char *, char *));
unsigned int RSA_CALLING_CONV T_strlen PROTO_LIST ((char *));
#endif

/* standard Time functions
 */
#if RSA_STD_TIME_FUNCS == RSA_ENABLED
void RSA_CALLING_CONV T_time PROTO_LIST ((UINT4 *));
#endif

#ifdef __cplusplus
}
#endif

#endif /* _STDLIBRF_H_ */

/* Copyright (C) RSA Data Security, Inc. created 1995.  This is an
   unpublished work protected as such under copyright law.  This work
   contains proprietary, confidential, and trade secret information of
   RSA Data Security, Inc.  Use, disclosure or reproduction without the
   express written authorization of RSA Data Security, Inc. is
   prohibited.
 */

/* Routines supplied by the implementor.
 */
#ifndef CALL_CONV
#define CALL_CONV
#endif

/* memory manipulation
*/
#ifdef _STD_MEM_FUNCS_
void CALL_CONV T_memset PROTO_LIST ((POINTER, int, unsigned int));
void CALL_CONV T_memcpy PROTO_LIST ((POINTER, POINTER, unsigned int));
void CALL_CONV T_memmove PROTO_LIST ((POINTER, POINTER, unsigned int));
int CALL_CONV T_memcmp PROTO_LIST ((POINTER, POINTER, unsigned int));
#endif

/* memory allocation
*/
#ifdef _STD_ALLOC_FUNCS_
POINTER CALL_CONV T_malloc PROTO_LIST ((unsigned int));
POINTER CALL_CONV T_realloc PROTO_LIST ((POINTER, unsigned int));
void CALL_CONV T_free PROTO_LIST ((POINTER));
#endif

/* string manipulation functions
*/
#ifdef _STD_STRING_FUNCS_
void CALL_CONV T_strcpy PROTO_LIST ((char *, char *));
int CALL_CONV T_strcmp PROTO_LIST ((char *, char *));
unsigned int CALL_CONV T_strlen PROTO_LIST ((char *));
#endif

/* standard Time functions
 */
#ifdef _STD_TIME_FUNCS_
void CALL_CONV T_time PROTO_LIST ((UINT4 *));
#endif

/******* HISTORY ********
$Log: stdlibrf.h,v $
Revision 1.2  2012/03/28 00:38:12  mcharon
remove forward slash from second line

Revision 1.1.1.1  2012/03/23 23:02:04  ptong
Initial archive of ng_diag module


$Endlog$
*/
