/* $Id: bexterr.h,v 1.2 2012/03/28 00:38:09 mcharon Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/include/bexterr.h,v $
 *------------------------------------------------------------------
 * bexterr.h  - Ported from IOS for Quack.
 *
 * Copyright (c) 2003-2012 by cisco Systems, Inc.
 * All rights reserved.
 *
 * Author: Huan Ngo
 *------------------------------------------------------------------
 */

/* Copyright (c) RSA Security Inc., 1996-1999.  All rights reserved.  
 * This work contains proprietary, confidential, and trade secret 
 * information of RSA Security Inc.  Use, disclosure or reproduction 
 * without the express written authorization of RSA Security Inc. is
 * prohibited.
 */

#ifndef _BEXTERR_H_
#define _BEXTERR_H_ 1

#include "resizeob.h"

/* pritikin - added 'void' parameter(s) */
typedef void (*RESERVED_FUNCTION) (void);

typedef struct B_ExtendedError {
  POINTER AM;
  RESERVED_FUNCTION reservedFunction;
  ResizeContext errorContext;
} B_ExtendedError;

void B_ExtendedErrorDestructor PROTO_LIST ((B_ExtendedError *));
void B_ExtendedErrorConstructor PROTO_LIST ((B_ExtendedError *));
#endif

/******* HISTORY *******
$Log: bexterr.h,v $
Revision 1.2  2012/03/28 00:38:09  mcharon
remove forward slash from second line

Revision 1.1.1.1  2012/03/23 23:02:03  ptong
Initial archive of ng_diag module


$Endlog$
*/
