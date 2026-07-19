/* $Id: endians.h,v 1.1 2014/03/25 02:12:33 huanngo Exp $
* $Source: 
*------------------------------------------------------------------
* endians.h  - support bi-endian and both gcc & icc
*
* Copyright (c) 2011 - 2014 by Cisco Systems, Inc.
* All rights reserved.
*
* Author: steja
*------------------------------------------------------------------
*/

#ifndef __ENDIAN_H_
#define __ENDIAN_H_

#if defined(INTEL_ICC)
#define __LE        __attribute__((littleendian))   /* little-endian */
#define __BE        __attribute__((bigendian))      /* big-endian */
#define __ENDIAN    __LE
#else /* defined(INTEL_ICC) */
#define __LE
#define __BE
#define __ENDIAN
#endif /* defined(INTEL_ICC) */

/*
 * Little Endian
 */

typedef __LE char               char_le;
typedef __LE short              short_le;
typedef __LE int                int_le;
typedef __LE long               long_le;
typedef __LE long long          llong_le;

typedef __LE unsigned char      uchar_le;
typedef __LE unsigned short     ushort_le;
typedef __LE unsigned int       uint_le;
typedef __LE unsigned long      ulong_le;
typedef __LE unsigned long long ullong_le;

typedef __LE char               int8_le;
typedef __LE short              int16_le;
typedef __LE int                int32_le;
typedef __LE long long          int64_le;

typedef __LE unsigned char      uint8_le;
typedef __LE unsigned short     uint16_le;
typedef __LE unsigned int       uint32_le;
typedef __LE unsigned long long uint64_le;

typedef __LE int                boolean_le;

/*
 * Big Endian
 */

typedef __BE char               char_be;
typedef __BE short              short_be;
typedef __BE int                int_be;
typedef __BE long               long_be;
typedef __BE long long          llong_be;

typedef __BE unsigned char      uchar_be;
typedef __BE unsigned short     ushort_be;
typedef __BE unsigned int       uint_be;
typedef __BE unsigned long      ulong_be;
typedef __BE unsigned long long ullong_be;

typedef __BE char               int8_be;
typedef __BE short              int16_be;
typedef __BE int                int32_be;
typedef __BE long long          int64_be;

typedef __BE unsigned char      uint8_be;
typedef __BE unsigned short     uint16_be;
typedef __BE unsigned int       uint32_be;
typedef __BE unsigned long long uint64_be;

typedef __BE int                boolean_be;

#endif /* endian.h */

/* end of module */

/*------------------------------------------------------------------------------
 * $Log: endians.h,v $
 * Revision 1.1  2014/03/25 02:12:33  huanngo
 * Adding patriot_linux directory to ng_diag code tree
 *
 * Revision 1.2  2012/05/08 23:52:54  huanngo
 * Support SM Patriot on ngd main code tree
 *
 * Revision 1.1.4.2  2011/08/18 19:43:22  huanngo
 * Update code to patriot2-branch
 *
 *------------------------------------------------------------------------------
 * $Endlog$
 *
 *------------------------------------------------------------------------------
 */
