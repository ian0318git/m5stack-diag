/* $Id: types.h,v 1.2 2017/07/28 07:58:39 harrchan Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/dsp_src/src/oakenshield_dsp/ppb/inc/types.h,v $
 *------------------------------------------------------------------
 * types.h
 *
 * Copyright (c) 2012-2017 by Cisco Systems, Inc.
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

typedef int  (*PFI)(int, ...);
typedef long (*PFL)(int, ...);
typedef void (*PFV)(int, ...);

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
//typedef unsigned int        uint32_t	_ADAPTABLE_TYPE_;	/* 32-bit */
typedef short               int16_t	_ADAPTABLE_TYPE_;
//typedef int                 int32_t	_ADAPTABLE_TYPE_;

typedef unsigned char       tinybool	_ADAPTABLE_TYPE_;

//typedef unsigned long       address_t   _ADAPTABLE_TYPE_;
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

#ifdef LINUX_APP
#define PFT PFL
typedef unsigned long        utype_t    _ADAPTABLE_TYPE_;
typedef long                 type_t     _ADAPTABLE_TYPE_;
#define get_line(ptr, size) fgets(ptr, size, stdin)
#define REG_EXT  0
#else
#define PFT PFI
typedef unsigned int        utype_t     _ADAPTABLE_TYPE_;
typedef int                 type_t      _ADAPTABLE_TYPE_;
#define get_line(ptr, size) getline(ptr, size)
#define REG_EXT &reg_ext
//typedef char                int8_t	_ADAPTABLE_TYPE_;	/* 8-bit */
typedef long long           int64_t	_ADAPTABLE_TYPE_;
//typedef unsigned int        size_t	_ADAPTABLE_TYPE_;
typedef unsigned long long  uint64_t	_ADAPTABLE_TYPE_;
//typedef unsigned int *      uintptr_t   _ADAPTABLE_TYPE_;


#endif  /* LINUX_APP */


#endif /* __TYPES_H */

/* end of module */

/******** History ******** 
$Log: types.h,v $
Revision 1.2  2017/07/28 07:58:39  harrchan
Collapse Oakenshield-branch to Main Trunk.

Revision 1.1.2.1  2017/06/29 08:14:33  harrchan
Initial commit code for Oakenshield

Revision 1.1  2012/04/18 09:50:19  srane
Initial checkin


$Endlog$
*/

