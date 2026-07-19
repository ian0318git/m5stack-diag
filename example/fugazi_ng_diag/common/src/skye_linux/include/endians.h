/* $Id: endians.h,v 1.2 2015/05/25 03:59:10 steja Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/skye_linux/include/endians.h,v $
 *------------------------------------------------------------------
* endians.h  - support bi-endian and both gcc & icc
 *
 * April 29, 2013 - iachang ported from Overlord.
 *
 * Copyright (c) 2013 ~ 2015 by Cisco Systems, Inc.
 * All rights reserved.
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

/******** History ********/
/*
 * $Log: endians.h,v $
 * Revision 1.2  2015/05/25 03:59:10  steja
 * Add Support Skye SM
 *
 * Revision 1.1.4.2  2015/04/29 11:36:25  steja
 * Code check-in to skye-branch2 for ER code review
 *
 *
 * ------------------------------------------------------------------
 * Revision 1.1.2.1  2014/07/21 01:56:37  palin2
 * Initial check-in Skye module side Diag code.
 *
 * $Endlog$
 */

