/* $Id: setjmps.h,v 1.2 2012/07/17 20:34:28 srane Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/dsp_src/src/sp27dsp/ppb/inc/setjmps.h,v $
 *------------------------------------------------------------------
 * setjmps.h
 * 
 *
 * Copyright (c) 2012 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#ifndef __SETJMP_H__
#define __SETJMP_H__

/* Linux has its own setjmp.h */
#include <setjmp.h>

extern jmp_buf monjmpbuf, *monjmpptr;

#endif  /* __SETJMP_H__ */

/* end of module */

/******** History ******** 
$Log: setjmps.h,v $
Revision 1.2  2012/07/17 20:34:28  srane
cleanup

Revision 1.1  2012/04/18 09:50:19  srane
Initial checkin

 
$Endlog$
*/

