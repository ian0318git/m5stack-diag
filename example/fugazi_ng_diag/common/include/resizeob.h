/* $Id: resizeob.h,v 1.2 2012/03/28 00:38:12 mcharon Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/include/resizeob.h,v $
 *------------------------------------------------------------------
 * resizeob.h  - Ported from IOS for Quack.
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

#ifndef _RESIZEOB_H_
#define _RESIZEOB_H_ 1

/* In C++:
class ResizeContext {
public:
  ResizeContext ();
  ~ResizeContext ();
  int makeNewContext (unsigned int contextSize);
  POINTER context () {return z.context;}

private:
  struct {
    POINTER context;
    unsigned int contextSize;
    CONTEXT_DESTRUCTOR ContextDestructor;
  } z;
};
*/

typedef void (*CONTEXT_DESTRUCTOR) PROTO_LIST ((POINTER ));

typedef struct ResizeContext {
  struct {
    POINTER context;
    unsigned int contextSize;
    CONTEXT_DESTRUCTOR ContextDestructor;
  } z;                                            /* zeriozed by constructor */
} ResizeContext;

void ResizeContextConstructor PROTO_LIST ((ResizeContext *));
void ResizeContextDestructor PROTO_LIST ((ResizeContext *));
int ResizeContextMakeNewContext PROTO_LIST ((ResizeContext *, unsigned int));

#endif

/******* HISTORY ********
$Log: resizeob.h,v $
Revision 1.2  2012/03/28 00:38:12  mcharon
remove forward slash from second line

Revision 1.1.1.1  2012/03/23 23:02:04  ptong
Initial archive of ng_diag module


$Endlog$
*/
