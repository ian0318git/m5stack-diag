/* $Id: uio_utils.h,v 1.2 2012/03/28 00:38:13 mcharon Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/include/uio_utils.h,v $
 *------------------------------------------------------------------
 * Filename:  uio_utils.h
 *            header file for uio
 *
 *
 * Copyright (c) 2011-2012 by cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#ifndef __UIO_UTILS__
#define __UIO_UTILS__

extern volatile void *uio_get_regs();
extern int uio_open();
extern int uio_pci_intr_en (const unsigned char *name);
extern int uio_read(unsigned int *);
extern int uio_select (unsigned int *icount, unsigned int secs, unsigned int);
extern int uio_enable_intr();
#endif /* uio */
/*------------------------------------------------------------------
$Log: uio_utils.h,v $
Revision 1.2  2012/03/28 00:38:13  mcharon
remove forward slash from second line

Revision 1.1.1.1  2012/03/23 23:02:04  ptong
Initial archive of ng_diag module


$Endlog$
*/
