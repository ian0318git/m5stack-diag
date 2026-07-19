/* $Id: types.h,v 1.3 2012/06/06 15:00:06 palin2 Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/include/types.h,v $
 *------------------------------------------------------------------
 *
 * Copyright (c) 2007-2012 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */
#ifndef __TYPES_H
#define __TYPES_H

/*
 * This attribute is only available from Intel ICC compiler.
 * By appling this attribute, user defined types from this file
 * become adaptable, be big-endian or little-endian, depends
 * on the location of files that include it.
 * More specific, become little-endian if included under le/ directory;
 * or big-endian otherwise.
 */
#if defined(INTEL_ICC)
#define _ADAPTABLE_TYPE_	__attribute__ ((adaptable_byte_order));
#else /* defined(INTEL_ICC) */
#define _ADAPTABLE_TYPE_
#endif /* defined(INTEL_ICC) */

#define VOIDPTRSIZE     (sizeof(void *)) // eq to 8 in 64bit linux

typedef int  (*PFI)();
typedef long  (*PFL)();
typedef void (*PFV)();

typedef	unsigned char	    u_char	_ADAPTABLE_TYPE_;
typedef	unsigned short	    u_short	_ADAPTABLE_TYPE_;
typedef	unsigned int	    u_int	_ADAPTABLE_TYPE_;
typedef	unsigned long	    u_long	_ADAPTABLE_TYPE_;
typedef unsigned char	    uchar	_ADAPTABLE_TYPE_;	/* System V */
typedef	unsigned short	    ushort	_ADAPTABLE_TYPE_;	/* System V */
typedef	unsigned int	    uint	_ADAPTABLE_TYPE_;	/* System V */
typedef unsigned long	    ulong	_ADAPTABLE_TYPE_;	/* System V */
typedef int		    boolean	_ADAPTABLE_TYPE_;
typedef long long           llong	_ADAPTABLE_TYPE_;
typedef unsigned long long  ullong	_ADAPTABLE_TYPE_;

typedef unsigned char       uint8	_ADAPTABLE_TYPE_;	/* 8-bit */
typedef unsigned short      uint16	_ADAPTABLE_TYPE_;	/* 16-bit*/
typedef unsigned int        uint32	_ADAPTABLE_TYPE_;	/* 32-bit */
typedef unsigned long long  uint64	_ADAPTABLE_TYPE_;
typedef unsigned long long  u64		_ADAPTABLE_TYPE_;

typedef unsigned short      uint16_t	_ADAPTABLE_TYPE_;	/* 16-bit */
typedef unsigned char       uint8_t	_ADAPTABLE_TYPE_;	/* 8-bit */
typedef unsigned int        uint32_t	_ADAPTABLE_TYPE_;	/* 32-bit */
typedef short               int16_t	_ADAPTABLE_TYPE_;
typedef int                 int32_t	_ADAPTABLE_TYPE_;

typedef unsigned char       tinybool	_ADAPTABLE_TYPE_;

/*
 * charint data structure
 * Used for manipulating ints, shorts, and bytes.
 */

typedef struct charint_ {
	union {
	    uchar byte[4];
	    uint  lword;
	    ushort sword[2];
	} d;
} charint;


#define PFT PFL
typedef unsigned long        utype_t    _ADAPTABLE_TYPE_;
typedef long                 type_t     _ADAPTABLE_TYPE_;
extern int  get_line(char *buffer, unsigned int bufsiz);
#define REG_EXT  0


#endif /* TYPES.H */

/* end of module */

/******** History ******** 
$Log: types.h,v $
Revision 1.3  2012/06/06 15:00:06  palin2
Clean up compiler warnings.

Revision 1.2  2012/03/28 00:38:13  mcharon
remove forward slash from second line

Revision 1.1.1.1  2012/03/23 23:02:04  ptong
Initial archive of ng_diag module


$Endlog$
*/
