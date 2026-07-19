/* $Id: ag_typedefs.h,v 1.1 2012/04/18 22:08:17 srane Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/dsp_src/src/sp27dsp/dss/inc/ag_typedefs.h,v $
 *------------------------------------------------------------------
 * ag_typedefs.h
 *
 * Mar 2012, Smita Rane
 *
 * Copyright (c) 2012 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

/******************************************************************************
 * >>>>>>>>>>>>>>>>>>>>>>>>>>>>    NOTIFICATION    <<<<<<<<<<<<<<<<<<<<<<<<<<<<
 *
 * Copyright (©) 2007 LSI Corporation
 * All Rights Reserved
 *
 * This is unpublished proprietary information of LSI Corporation.  This
 * copyright notice does not evidence publication.
 *
 * The use of the software, documentation, methodologies, and other information
 * contained herein is governed solely by the associated license agreements.
 * Any inconsistent use shall be deemed to be a misappropriation of the
 * intellectual property of LSI Corporation and treated accordingly.
 * ----------------------------------------------------------------------------
 *
 * Content:  Basic Data Types
 *
 * $History: $
 *
 ******************************************************************************/
#ifndef AG_TYPEDEFS_H_
#define AG_TYPEDEFS_H_

#define _AG_MG_FRAMEWORK_ 1

/* definition of standard types *
 ********************************/

#ifdef _ENTERPRISE_C_
#include <stdint.h>
#include <intrinsics.h>

#pragma align   Word16x2 4
#pragma align   Word16x4 8
#pragma align   Word32x2 8

#pragma align   uint8_a16_t 2
#pragma align   uint8_a32_t 4
#pragma align   uint8_a64_t 8
#pragma align   uint8_a128_t 16
#pragma align   int8_a16_t 2
#pragma align   int8_a32_t 4
#pragma align   int8_a64_t 8
#pragma align   int8_a128_t 16
#pragma align   uint16_a32_t 4
#pragma align   uint16_a64_t 8
#pragma align   uint16_a128_t 16
#pragma align   int16_a32_t 4
#pragma align   int16_a64_t 8
#pragma align   int16_a128_t 16
#pragma align   uint32_a64_t 8
#pragma align   uint32_a128_t 16
#pragma align   int32_a64_t 8
#pragma align   int32_a128_t 16

#ifdef BIG_ENDIAN
#define BIG_ENDIAN_TYPES
#endif /* BIG_ENDIAN */

#else /* _ENTERPRISE_C_ */

#if defined(__linux__) || defined (__CYGWIN__) || (defined (__MWERKS__) && defined (__INTEL__))
#include <stdint.h>

#elif (defined (__MWERKS__) && (defined(__POWERPC__) || defined(__MC68K__)))
#include <stdint.h>
#define BIG_ENDIAN_TYPES

#elif defined (__GNUC__) && defined (__arm__)
#include <stdint.h>
#ifdef __ARMEB__
#define BIG_ENDIAN_TYPES
#endif

#elif defined (__GNUC__) && defined (__APPLE__)
#include <stdint.h>
#ifdef __BIG_ENDIAN__ /* else __LITTLE_ENDIAN__ */
#define BIG_ENDIAN_TYPES
#endif /* BIG_ENDIAN */

#elif defined(_MSC_VER) || defined(_WIN32) /* Visual C */
typedef char int8_t;                /* 8 bit "register"  (c_*) */
typedef unsigned char uint8_t;      /* 8 bit "register"  (b_*) */
typedef short int int16_t;          /* 16 bit "register"  (s_*) */
typedef unsigned short int uint16_t;/* 16 bit "register"  (us_*) */
typedef long int int32_t;           /* 32 bit "accumulator" (L_*) */
typedef unsigned long int uint32_t; /* 32 bit "accumulator" (uL_*) */
typedef __int64 int64_t;
typedef unsigned __int64 uint64_t;

#elif defined(__sun__) && defined( __sparc__)
#if 1
#include <sys/int_types.h>      /* /usr/include/sys/int_types.h */
#else
typedef char int8_t;                /* 8 bit "register"  (c_*) */
typedef unsigned char uint8_t;      /* 8 bit "register"  (b_*) */
typedef short int16_t;              /* 16 bit "register"  (sw*) */
typedef unsigned short uint16_t;    /* 16 bit "register"  (usw*) */
typedef long int32_t;               /* 32 bit "accumulator" (L_*) */
typedef unsigned long uint32_t;     /* 32 bit "accumulator" (uL_*) */
typedef long long int64_t;
typedef unsigned long long uint64_t;
#endif
#define BIG_ENDIAN_TYPES

#elif defined (lint) || defined (_lint)
typedef char int8_t;                /* 8 bit "register"  (c_*) */
typedef unsigned char uint8_t;      /* 8 bit "register"  (b_*) */
typedef short int int16_t;          /* 16 bit "register"  (s_*) */
typedef unsigned short int uint16_t;/* 16 bit "register"  (us_*) */
typedef long int int32_t;           /* 32 bit "accumulator" (L_*) */
typedef unsigned long int uint32_t; /* 32 bit "accumulator" (uL_*) */
typedef long long int64_t;
typedef unsigned long long uint64_t;
#else
#error "can't determine architecture; adapt ag_typedefs.h to your platform"
#endif

/* limits definitions *
 **********************/

#ifndef INT8_MIN
#define INT8_MIN	((int8_t)0x80)
#endif
#ifndef INT8_MAX
#define INT8_MAX	((int8_t)0x7f)
#endif
#ifndef INT16_MIN
#define INT16_MIN	((int16_t)0x8000)
#endif
#ifndef INT16_MAX
#define INT16_MAX	((int16_t)0x7fff)
#endif
#ifndef INT32_MIN
#define INT32_MIN	((int32_t)0x80000000)
#endif
#ifndef INT32_MAX
#define INT32_MAX	((int32_t)0x7fffffff)
#endif

/* basic_op definitions & types *
 ********************************/

typedef uint8_t UWord8;         /*  8 bits, unsigned */
typedef int8_t  Word8;          /*  8 bits,   signed */
typedef uint16_t UWord16;       /* 16 bits, unsigned */
typedef int16_t Word16;         /* 16 bits,   signed */
typedef uint32_t UWord32;       /* 32 bits, unsigned */
typedef int32_t Word32;         /* 32 bits,   signed */
typedef int64_t Word40;         /* 40 bits,   signed */
typedef int Flag;

#ifndef MIN_32
#define MIN_32 INT32_MIN
#endif
#ifndef MAX_32
#define MAX_32 INT32_MAX
#endif

#ifndef MIN_16
#define MIN_16 INT16_MIN
#endif
#ifndef MAX_16
#define MAX_16 INT16_MAX
#endif

#endif /* _ENTERPRISE_C_ */

#ifndef SIGN_32
#define SIGN_32 ((int32_t)0x80000000)   /* sign bit */
#endif
#ifndef SIGN_16
#define SIGN_16 ((int16_t)0x8000)       /* sign bit for int16_t  type */
#endif

#define MAX_40 ((Word40) 549755813887.)
#define MIN_40 ((Word40)-549755813888.)

typedef Word16      Word16x2;       /* 16 bits,   signed  4 byte aligned */
typedef Word16      Word16x4;       /* 16 bits,   signed  8 byte aligned */
typedef Word32      Word32x2;       /* 32 bits,   signed  8 byte aligned */

/* aligned types *
 ******************/
typedef uint8_t     uint8_a16_t;    /*  8 bits, unsigned  2 byte aligned */
typedef uint8_t     uint8_a32_t;    /*  8 bits, unsigned  4 byte aligned */
typedef uint8_t     uint8_a64_t;    /*  8 bits, unsigned  8 byte aligned */
typedef uint8_t     uint8_a128_t;   /*  8 bits, unsigned 16 byte aligned */
typedef int8_t      int8_a16_t;     /*  8 bits,   signed  2 byte aligned */
typedef int8_t      int8_a32_t;     /*  8 bits,   signed  4 byte aligned */
typedef int8_t      int8_a64_t;     /*  8 bits,   signed  8 byte aligned */
typedef int8_t      int8_a128_t;    /*  8 bits,   signed 16 byte aligned */
typedef uint16_t    uint16_a32_t;   /* 16 bits, unsigned  4 byte aligned */
typedef uint16_t    uint16_a64_t;   /* 16 bits, unsigned  8 byte aligned */
typedef uint16_t    uint16_a128_t;  /* 16 bits, unsigned 16 byte aligned */
typedef int16_t     int16_a32_t;    /* 16 bits,   signed  4 byte aligned */
typedef int16_t     int16_a64_t;    /* 16 bits,   signed  8 byte aligned */
typedef int16_t     int16_a128_t;   /* 16 bits,   signed 16 byte aligned */
typedef uint32_t    uint32_a64_t;   /* 32 bits, unsigned  8 byte aligned */
typedef uint32_t    uint32_a128_t;  /* 32 bits, unsigned 16 byte aligned */
typedef int32_t     int32_a64_t;    /* 32 bits,   signed  8 byte aligned */
typedef int32_t     int32_a128_t;   /* 32 bits,   signed 16 byte aligned */

/* boolean type *
 ******************/
typedef int Bool;

typedef uint32_t    boolean_t;
typedef uint32_t    bool32_t;
typedef uint16_t    bool16_t;
typedef uint8_t     bool8_t;

#ifndef bool
#define bool boolean_t
#endif

#ifndef false
#define false (0)
#endif
#ifndef FALSE
#define FALSE (0)
#endif
#ifndef true
#define true  (1)
#endif
#ifndef TRUE
#define TRUE  (1)
#endif

#endif /* AG_TYPEDEFS_H_ */

/******** History ********
$Log: ag_typedefs.h,v $
Revision 1.1  2012/04/18 22:08:17  srane
Initial checkin


$Endlog$
*/

