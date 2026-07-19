/* $Id: setjmps.h,v 1.2 2012/03/28 00:38:12 mcharon Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/include/setjmps.h,v $
 *------------------------------------------------------------------
 *
 * Rob Clevenger
 *
 * Copyright (c) 2007-2012 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#ifndef __SETJMP_H__
#define __SETJMP_H__

/* Linux has its own setjmp.h */
#include <setjmp.h>

#if defined(INTEL_ICC)
#pragma byte_order (push, littleendian)
typedef int jmp_buf[20];
extern jmp_buf monjmpbuf, *monjmpptr;
#pragma byte_order (pop)

#else /* defined(INTEL_ICC) */

extern jmp_buf monjmpbuf, *monjmpptr;

#endif  /* defined(INTEL_ICC) */
#endif  /* __SETJMP_H__ */


/* end of module */

/******** History ******** 
$Log: setjmps.h,v $
Revision 1.2  2012/03/28 00:38:12  mcharon
remove forward slash from second line

Revision 1.1.1.1  2012/03/23 23:02:04  ptong
Initial archive of ng_diag module


$Endlog$
*/
