/* $Id: byteswap.h,v 1.2 2012/03/28 00:38:09 mcharon Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/include/byteswap.h,v $
 *-----------------------------------------------------------------------------
 * byteswap.h - Little-Endian <----> Big Endian Generic Macros
 *
 * July. 2003 Alan O'Sullivan
 *
 * Copyright (c) 2007-2012 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *-----------------------------------------------------------------------------
 */

#ifndef __BYTESWAP_H__
#define __BYTESWAP_H__

#include "types.h"

/*
 * A) do data swap only.
 *
 * 	dswap4(dint)		: 4-byte data swap
 * 	dswap2(dshort)		: 2-byte data swap
 * 	dswap1(dchar)		: 1-byte data swap
 *
 * B) do address swap only. address should be on the right address boundary.
 *
 *	rd_aswap4(addr)		: read 4-byte from a swapped address
 *	rd_aswap2(addr) 	: read 2-byte from a swapped address
 *	rd_aswap1(addr) 	: read 1-byte from a swapped address
 * 
 *	wr_aswap4(addr, dint)	: write 4-byte to a swapped address
 *	wr_aswap2(addr, dshort) : write 2-byte to a swapped address
 *	wr_aswap1(addr, dchar)	: write 1-byte to a swapped address
 *
 *	aswap1(addr)		: return swapped address 
 *
 * C) do both address and data swap. address should be on the right boundary.
 *
 *	rd_adswap4(addr)	: read 4-byte from a swapped address
 *	rd_adswap2(addr) 	: read 2-byte from a swapped address
 *	rd_adswap1(addr) 	: read 1-byte from a swapped address
 *
 *	wr_adswap4(addr, dint)	: write 4-byte to a swapped address
 *	wr_adswap2(addr, dshort): write 2-byte to a swapped address
 *	wr_adswap1(addr, dchar)	: write 1-byte to a swapped address
 */

/* 
 * 4-byte data swap
 *
 * dswap4(dint)
 *	dint: 4-byte data of type int or uint
 *
 * example:
 *	    uint int1, int2;
 *
 *          int1: |<---------- int1 --------->|
 *                +------+------+------+------+
 *                |  11  |  22  |  33  |  44  |
 *                +------+------+------+------+
 *
 *          int2 = dswap4(int1);
 *
 *                |<---------- int2 --------->|
 *                +------+------+------+------+
 *                |  44  |  33  |  22  |  11  |
 *                +------+------+------+------+
 */

#define dswap4(dint)		((((dint) & 0xff) << 24) | \
				 (((dint) & 0xff00) << 8) | \
				 (((dint) & 0xff0000) >> 8) | \
				 (((dint) & 0xff000000) >> 24))

/* 
 * 2-byte data swap
 *
 * dswap2(dshort)
 *	dshort: 2-byte data of type short or ushort
 *
 * example:
 *	    ushort sh1, sh2;
 *
 *          sh1: |<--- sh1 --->|
 *               +------+------+
 *               |  11  |  22  |
 *               +------+------+
 *
 *          sh2 = dswap2(sh1);
 *
 *               |<--- sh2 --->|
 *               +------+------+
 *               |  22  |  11  |
 *               +------+------+
 */

#define dswap2(dshort)		((((dshort) & 0xff00) >> 8) | \
				 (((dshort) & 0x00ff) << 8))

/* 
 * 1-byte data swap
 * 
 * dswap1(dchar)
 *	dchar: 1-byte data of type char or uchar
 *
 * example:
 *	    uchar ch1, ch2;
 *
 *          ch1: |< ch1>|
 *               +------+
 *               |  11  |
 *               +------+
 *
 *          ch2 = dswap1(ch1);
 *
 *               |< ch2>|
 *               +------+
 *               |  11  |
 *               +------+
 */

#define dswap1(dchar)		((dchar) & 0xff)

/* 
 * read 4-byte data from a swapped address on the 4-byte boundary
 *
 * rd_aswap4(addr)
 *	addr: address on the 4-byte boundary
 *
 * example:
 *	    uint int1, int2;
 *
 *          int1: |<---------- int1 --------->|
 *                +------+------+------+------+
 *                |  11  |  22  |  33  |  44  |
 *                +------+------+------+------+
 *
 *          int2 = rd_aswap4(&int1);
 *
 *          int2: |<---------- int2 --------->|
 *                +------+------+------+------+
 *                |  11  |  22  |  33  |  44  |
 *                +------+------+------+------+
 */

#define rd_aswap4(addr)		(*(uint *)(addr))

/* 
 * read 2-byte data from a swapped address on the 2-byte boundary
 *
 * rd_aswap2(addr)
 *	addr: address on the 2-byte boundary
 *
 * example:
 *	    ushort sh[2], sh9;
 *
 *          sh[2]: |<-- sh[0] -->|<-- sh[1] -->|
 *                 +------+------+------+------+
 *                 |  11  |  22  |  33  |  44  |
 *                 +------+------+------+------+
 *
 *          sh9 = rd_aswap2(&sh[0]);
 *
 *            sh9: |<--- sh9 --->|
 *                 +------+------+
 *                 |  33  |  44  |
 *                 +------+------+
 *
 *          sh9 = rd_aswap2(&sh[1]);
 *
 *            sh9: |<--- sh9 --->|
 *                 +------+------+
 *                 |  11  |  22  |
 *                 +------+------+
 */

#define rd_aswap2(addr)		(*(ushort *)((uint)(addr) ^ 2))

/* 
 * read 1-byte data from a swapped address
 *
 * rd_aswap1(addr)
 *	addr: address
 *
 * example:
 *	    uchar ch[4], ch9;
 *
 *          ch[4]: | ch[0]| ch[1]| ch[2]| ch[3]|
 *                 +------+------+------+------+
 *                 |  11  |  22  |  33  |  44  |
 *                 +------+------+------+------+
 *
 *          ch9 = rd_aswap1(&ch[0]);
 *
 *            ch9: |  ch9 |
 *                 +------+
 *                 |  44  |
 *                 +------+
 *
 *          ch9 = rd_aswap1(&ch[1]);
 *
 *            ch9: |  ch9 |
 *                 +------+
 *                 |  33  |
 *                 +------+
 *
 *          ch9 = rd_aswap1(&ch[2]);
 *
 *            ch9: |  ch9 |
 *                 +------+
 *                 |  22  |
 *                 +------+
 *
 *          ch9 = rd_aswap1(&ch[3]);
 *
 *            ch9: |  ch9 |
 *                 +------+
 *                 |  11  |
 *                 +------+
 */

#define rd_aswap1(addr)		(*(uchar *)((uint)(addr) ^ 3))

/*
 * Input : pointer to uchar
 * return swapped address 
 *
 * aswap1(addr)
 *      addr: address pointer to byte
 *
 * example:
 *          uchar *ret_addr;
 *
 *          addr:  |1000h |1001h |1002h |1003h |
 *                 +------+------+------+------+
 *          data:  |  11  |  22  |  33  |  44  |
 *                 +------+------+------+------+
 *
 *          ret_addr = aswap1(1000h);
 *                   = 1003h
 *        *(ret_addr) will be 44
 *
 *          ret_addr = aswap1(1001h);
 *                   = 1002h
 *        *(ret_addr) will be 33
 *
 *          ret_addr = aswap1(1002h);
 *                   = 1001h
 *        *(ret_addr) will be 22
 *
 *          ret_addr = aswap1(1003h);
 *                   = 1000h
 *        *(ret_addr) will be 11
 *
 */

#define aswap1(addr)            (uchar *)((uint)(addr) ^ 3)

/*
 * Input : pointer to ushort
 * return swapped address 
 *
 * aswap2(addr)
 *      addr: address pointer to ushort
 *
 * example:
 *	    ushort sh[2], sh9;
 *
 *          sh[2]: |<-- sh[0] -->|<-- sh[1] -->|
 *                 +------+------+------+------+
 *          data:  |  11  |  22  |  33  |  44  |
 *                 +------+------+------+------+
 *          addr:   1000h |1001h |1002h | 1003h
 *
 *          ret_addr = aswap2(1000h);
 *                   = 1002h
 *        *(ret_addr) will be 3344
 *
 *          ret_addr = aswap2(1002h);
 *                   = 1000h
 *        *(ret_addr) will be 1122
 *
 */

#define aswap2(addr)            (ushort *)((uint)(addr) ^ 2)

/* 
 * write 4-byte data to a swapped address on the 4-byte boundary
 *
 * wr_aswap4(addr, dint)
 *	addr: address on the 4-byte boundary
 *	dint: 4-byte data of type int or uint
 *
 * example:
 *	    uint int1, int2;
 *
 *          int1: |<---------- int1 --------->|
 *                +------+------+------+------+
 *                |  11  |  22  |  33  |  44  |
 *                +------+------+------+------+
 *
 *          wr_aswap4(&int2, int1);
 *
 *          int2: |<---------- int2 --------->|
 *                +------+------+------+------+
 *                |  11  |  22  |  33  |  44  |
 *                +------+------+------+------+
 */

#define wr_aswap4(addr, dint)	rd_aswap4(addr) = (uint)(dint)

/* 
 * write 2-byte data to a swapped address on the 2-byte boundary
 *
 * wr_aswap2(addr, dshort)
 *	addr:   address on the 2-byte boundary
 *	dshort: 2-byte data of type short or ushort
 *
 * example:
 *	    ushort sh[2], sh9;
 *
 *            sh9: |<--- sh9 --->|
 *                 +------+------+
 *                 |  11  |  22  |
 *                 +------+------+
 *
 *          wr_aswap2(&sh[0], sh9);
 *
 *          sh[2]: |<-- sh[0] -->|<-- sh[1] -->|
 *                 +------+------+------+------+
 *                 |  xx  |  xx  |  11  |  22  |
 *                 +------+------+------+------+
 *
 *          wr_aswap2(&sh[1], sh9);
 *
 *          sh[2]: |<-- sh[0] -->|<-- sh[1] -->|
 *                 +------+------+------+------+
 *                 |  11  |  22  |  xx  |  xx  |
 *                 +------+------+------+------+
 */

#define wr_aswap2(addr, dshort)	rd_aswap2(addr) = (ushort)(dshort) 

/* 
 * write 1-byte data to a swapped address
 *
 * rd_aswap1(addr, dchar)
 *	addr:  address
 *	dchar: 1-byte data of type char or uchar
 *
 * example:
 *	    uchar ch[4], ch9;
 *
 *            ch9: |  ch9 |
 *                 +------+
 *                 |  11  |
 *                 +------+
 *
 *          wr_aswap1(&ch[0], ch9);
 *
 *          ch[4]: | ch[0]| ch[1]| ch[2]| ch[3]|
 *                 +------+------+------+------+
 *                 |  xx  |  xx  |  xx  |  11  |
 *                 +------+------+------+------+
 *
 *          wr_aswap1(&ch[1], ch9);
 *
 *          ch[4]: | ch[0]| ch[1]| ch[2]| ch[3]|
 *                 +------+------+------+------+
 *                 |  xx  |  xx  |  11  |  xx  |
 *                 +------+------+------+------+
 *
 *          wr_aswap1(&ch[2], ch9);
 *
 *          ch[4]: | ch[0]| ch[1]| ch[2]| ch[3]|
 *                 +------+------+------+------+
 *                 |  xx  |  11  |  xx  |  xx  |
 *                 +------+------+------+------+
 *
 *          wr_aswap1(&ch[3], ch9);
 *
 *          ch[4]: | ch[0]| ch[1]| ch[2]| ch[3]|
 *                 +------+------+------+------+
 *                 |  11  |  xx  |  xx  |  xx  |
 *                 +------+------+------+------+
 */

#define wr_aswap1(addr, dchar)	rd_aswap1(addr) = (uchar)(dchar)

/* 
 * read 4-byte swapped data from a swapped address on the 4-byte boundary
 *
 * rd_adswap4(addr)
 *	addr: address on the 4-byte boundary
 *
 * example:
 *	    uint int1, int2;
 *
 *          int1: |<---------- int1 --------->|
 *                +------+------+------+------+
 *                |  11  |  22  |  33  |  44  |
 *                +------+------+------+------+
 *
 *          int2 = rd_adswap4(&int1);
 *
 *          int2: |<---------- int2 --------->|
 *                +------+------+------+------+
 *                |  44  |  33  |  22  |  11  |
 *                +------+------+------+------+
 */

#define rd_adswap4(addr)	dswap4(rd_aswap4(addr))

/* 
 * read 2-byte swapped data from a swapped address on the 2-byte boundary
 *
 * rd_adswap2(addr)
 *	addr: address on the 2-byte boundary
 *
 * example:
 *	    ushort sh[2], sh9;
 *
 *          sh[2]: |<-- sh[0] -->|<-- sh[1] -->|
 *                 +------+------+------+------+
 *                 |  11  |  22  |  33  |  44  |
 *                 +------+------+------+------+
 *
 *          sh9 = rd_adswap2(&sh[0]);
 *
 *            sh9: |<--- sh9 --->|
 *                 +------+------+
 *                 |  44  |  33  |
 *                 +------+------+
 *
 *          sh9 = rd_adswap2(&sh[1]);
 *
 *            sh9: |<--- sh9 --->|
 *                 +------+------+
 *                 |  22  |  11  |
 *                 +------+------+
 */

#define rd_adswap2(addr)	dswap2(rd_aswap2(addr))

/* 
 * read 1-byte swapped data from a swapped address
 *
 * rd_aswap1(addr)
 *	addr: address
 *
 * example:
 *	    uchar ch[4], ch9;
 *
 *          ch[4]: | ch[0]| ch[1]| ch[2]| ch[3]|
 *                 +------+------+------+------+
 *                 |  11  |  22  |  33  |  44  |
 *                 +------+------+------+------+
 *
 *          ch9 = rd_adswap1(&ch[0]);
 *
 *            ch9: |  ch9 |
 *                 +------+
 *                 |  44  |
 *                 +------+
 *
 *          ch9 = rd_adswap1(&ch[1]);
 *
 *            ch9: |  ch9 |
 *                 +------+
 *                 |  33  |
 *                 +------+
 *
 *          ch9 = rd_adswap1(&ch[2]);
 *
 *            ch9: |  ch9 |
 *                 +------+
 *                 |  22  |
 *                 +------+
 *
 *          ch9 = rd_adswap1(&ch[3]);
 *
 *            ch9: |  ch9 |
 *                 +------+
 *                 |  11  |
 *                 +------+
 */

#define rd_adswap1(addr)	dswap1(rd_aswap1(addr))

/* 
 * write 4-byte swapped data to a swapped address on the 4-byte boundary
 *
 * wr_aswap4(addr, dint)
 *	addr: address on the 4-byte boundary
 *	dint: 4-byte data of type int or uint
 *
 * example:
 *	    uint int1, int2;
 *
 *          int1: |<---------- int1 --------->|
 *                +------+------+------+------+
 *                |  11  |  22  |  33  |  44  |
 *                +------+------+------+------+
 *
 *          wr_adswap4(&int2, int1);
 *
 *          int2: |<---------- int2 --------->|
 *                +------+------+------+------+
 *                |  44  |  33  |  22  |  11  |
 *                +------+------+------+------+
 */

#define wr_adswap4(addr, dint)   rd_aswap4(addr) = dswap4(dint)

/* 
 * write 2-byte swapped data to a swapped address on the 2-byte boundary
 *
 * wr_aswap2(addr, dshort)
 *	addr:   address on the 2-byte boundary
 *	dshort: 2-byte data of type short or ushort
 *
 * example:
 *	    ushort sh[2], sh9;
 *
 *            sh9: |<--- sh9 --->|
 *                 +------+------+
 *                 |  11  |  22  |
 *                 +------+------+
 *
 *          wr_adswap2(&sh[0], sh9);
 *
 *          sh[2]: |<-- sh[0] -->|<-- sh[1] -->|
 *                 +------+------+------+------+
 *                 |  xx  |  xx  |  22  |  11  |
 *                 +------+------+------+------+
 *
 *          wr_adswap2(&sh[1], sh9);
 *
 *          sh[2]: |<-- sh[0] -->|<-- sh[1] -->|
 *                 +------+------+------+------+
 *                 |  22  |  11  |  xx  |  xx  |
 *                 +------+------+------+------+
 */

#define wr_adswap2(addr, dshort) rd_aswap2(addr) = dswap2(dshort)

/* 
 * write 1-byte swapped data to a swapped address
 *
 * wr_adswap1(addr)
 *	addr: address
 *	dchar: 1-byte data of type char or uchar
 *
 * example:
 *	    uchar ch[4], ch9;
 *
 *            ch9: |  ch9 |
 *                 +------+
 *                 |  11  |
 *                 +------+
 *
 *          wr_adswap1(&ch[0], ch9);
 *
 *          ch[4]: | ch[0]| ch[1]| ch[2]| ch[3]|
 *                 +------+------+------+------+
 *                 |  xx  |  xx  |  xx  |  11  |
 *                 +------+------+------+------+
 *
 *          wr_adswap1(&ch[1], ch9);
 *
 *          ch[4]: | ch[0]| ch[1]| ch[2]| ch[3]|
 *                 +------+------+------+------+
 *                 |  xx  |  xx  |  11  |  xx  |
 *                 +------+------+------+------+
 *
 *          wr_adswap1(&ch[2], ch9);
 *
 *          ch[4]: | ch[0]| ch[1]| ch[2]| ch[3]|
 *                 +------+------+------+------+
 *                 |  xx  |  11  |  xx  |  xx  |
 *                 +------+------+------+------+
 *
 *          wr_adswap1(&ch[3], ch9);
 *
 *          ch[4]: | ch[0]| ch[1]| ch[2]| ch[3]|
 *                 +------+------+------+------+
 *                 |  11  |  xx  |  xx  |  xx  |
 *                 +------+------+------+------+
 */

#define wr_adswap1(addr, dchar)	 rd_aswap1(addr) = dswap1(dchar)

/*
 * Note: please add -DL_ENDIAN in the Makefile
 *       to enable the byte swap version of the following macros.
 *	 unswap version will be userd if UNdefined.
 */

#if defined(L_ENDIAN)

/*
 * MACROs to swap data or address or both
 */

#define DSWAP4(dint)		dswap4(dint)
#define DSWAP2(dshort)		dswap2(dshort)
#define DSWAP1(dchar)		dswap1(dchar)

#define RD_ASWAP4(addr)		rd_aswap4(addr)
#define RD_ASWAP2(addr)		rd_aswap2(addr)
#define RD_ASWAP1(addr)		rd_aswap1(addr)
#define ASWAP1(addr)    	aswap1(addr)
#define ASWAP2(addr)    	aswap2(addr)

#define WR_ASWAP4(addr, dint)	wr_aswap4(addr, dint)
#define WR_ASWAP2(addr, dshort)	wr_aswap2(addr, dshort)
#define WR_ASWAP1(addr, dchar)	wr_aswap1(addr, dchar)

#define RD_ADSWAP4(addr)	rd_adswap4(addr)
#define RD_ADSWAP2(addr)	rd_adswap2(addr)
#define RD_ADSWAP1(addr)	rd_adswap1(addr)

#define WR_ADSWAP4(addr, dint)	 wr_adswap4(addr, dint)
#define WR_ADSWAP2(addr, dshort) wr_adswap2(addr, dshort)
#define WR_ADSWAP1(addr, dchar)	 wr_adswap1(addr, dchar)

#else /* L_ENDIAN */

/*
 * MACROs do NOT swap data or address or both
 */

#define DSWAP4(dint)		(dint)
#define DSWAP2(dshort)		(dshort)
#define DSWAP1(dchar)		(dchar)

#define RD_ASWAP4(addr)		(*(uint *)(addr))
#define RD_ASWAP2(addr)		(*(ushort *)(addr))
#define RD_ASWAP1(addr)		(*(uchar *)(addr))
#define ASWAP2(addr)    	(ushort *)(addr)
#define ASWAP1(addr)    	(uchar *)(addr)

#define WR_ASWAP4(addr, dint)	(*(uint *)(addr)) = (uint)(dint)
#define WR_ASWAP2(addr, dshort)	(*(ushort *)(addr)) = (ushort)(dshort)
#define WR_ASWAP1(addr, dchar)	(*(uchar *)(addr)) = (uchar)(dchar)

#define RD_ADSWAP4(addr)	(*(uint *)(addr))
#define RD_ADSWAP2(addr)	(*(ushort *)(addr))
#define RD_ADSWAP1(addr)	(*(uchar *)(addr))

#define WR_ADSWAP4(addr, dint)	 (*(uint *)(addr)) = (uint)(dint)
#define WR_ADSWAP2(addr, dshort) (*(ushort *)(addr)) = (ushort)(dshort)
#define WR_ADSWAP1(addr, dchar)	 (*(uchar *)(addr)) = (uchar)(dchar)

#endif /* L_ENDIAN */

/*
 * This is an old one, keep here for backward support.
 *
 * Function: BYTESWAP
 * -------------------
 * Big Endian <---> Little Endian
 */
static inline
ulong BYTESWAP (ulong x)
{
    return( ((x & 0x000000FF) << 24) |
            ((x & 0x0000FF00) << 8)  |
            ((x & 0x00FF0000) >> 8)  |
            ((x & 0xFF000000) >> 24) );
}

#if 0 /* examples */

    uint   in1, in9;
    ushort sh[2], sh9;
    uchar  ch[4], ch9;

	in1:	|<---------- in1 ---------->|
		+------+------+------+------+
		|  11  |  22  |  33  |  44  |
		+------+------+------+------+

	sh[2]:  |<-- sh[0] -->|<-- sh[1] -->|
		+------+------+------+------+
		|  55  |  66  |  77  |  88  |
		+------+------+------+------+

	ch[4]:  | ch[0]| ch[1]| ch[2]| ch[3]|
		+------+------+------+------+
		|  aa  |  bb  |  cc  |  dd  |
		+------+------+------+------+

    /////////////////////////////////////////

    in9 = in1;
    in9 = rd_aswap4(&in1);
		+------+------+------+------+
		|  11  |  22  |  33  |  44  |
		+------+------+------+------+

    in9 = dswap4(in1);
    in9 = rd_adswap4(&in1);
		+------+------+------+------+
		|  44  |  33  |  22  |  11  |
		+------+------+------+------+
		
    sh9 = sh[0];
		+------+------+
		|  55  |  66  |
		+------+------+

    sh9 = dswap2(sh[0]);
		+------+------+
		|  66  |  55  |
		+------+------+

    sh9 = rd_aswap2(&sh[0]);
		+------+------+
		|  77  |  88  |
		+------+------+

    sh9 = rd_adswap2(&sh[0]);
		+------+------+
		|  88  |  77  |
		+------+------+

    ch9 = ch[0];
    ch9 = dswap1(ch[0]);
		+------+
		|  aa  |
		+------+

    ch9 = rd_aswap1(&ch[0]);
    ch9 = rd_adswap1(&ch[0]);
		+------+
		|  dd  |
		+------+

Normally it requires to perform both address and data swap from a different
endianness host to access to a CISCO PCI device. A typical way to do that
is to
a) re-arrange the associated device date structure for the address swap;
followed by
b) calling corresponding data swap MACROs on accessing to a field;

For a field that declared as (unsigned) short or (unsigned) char
array, there will not be a proper way avaialble to re-arrange the data
structure for the addess swap purpose, it is suggested to use one of
the ADSWAP MACROs to swap both address and data.

A device may has a built-in h/w assist circuitry to perform ONLY either
address swap or data swap, a proper swap MACRO need to be used then.

Example of a typical endianness-aware implementation will be similar to:

    #if L_ENDIAN
    typedef struct {
	uint	var4;
	ushort	var2;			/* address swap */
	uchar	var1a;			/* address swap */
	uchar	var1b;			/* address swap */
	uint	arr4[2];
	ushort	arr2[4];
	uchar	arr1[4];
    } example_t;
    #else /* L_ENDIAN */
    typedef struct {
	uint	var4;
	uchar	var1b;			/* address swap */
	uchar	var1a;			/* address swap */
	ushort	var2;			/* address swap */
	uint	arr4[2];
	ushort	arr2[2];
	uchar	arr1[4];
    } example_t;
    #endif /* L_ENDIAN */

    example_t foo, *foop = &foo;
    uint   in;
    ushort sh;
    uchar  ch;

    /*
     * uint access
     */

    in = DSWAP4(foop->var4);		/* data swap */
    foop->var4 = DSWAP4(in);		/* data swap */
    in = DSWAP4(foop->arr4[1]);		/* data swap */
    foop->arr4[0] = DSWAP4(in);		/* data swap */

    /*
     * ushort access
     */

    sh = DSWAP2(foop->var2);		/* data swap */
    foop->var2 = DSWAP2(sh);		/* data swap */
    sh = RD_ADSWAP2(&foop->arr2[0]);	/* address and data swap on an array */
    WR_ADSWAP2(&foop->arr2[1], sh);	/* address and data swap on an array */

    /*
     * uchar access
     */

    ch = DSWAP1(foop->var1a);		/* data swap (no need) */
    foop->var1b = DSWAP1(ch);		/* data swap (no need) */
    ch = RD_ADSWAP1(&foop->arr1[0]);/* addr and data swap (no need data swap) */
    WR_ADSWAP1(&foop->arr1[2], ch); /* addr and data swap (no need data swap) */

#endif /* examples */

#endif  /* --------------------- End of File ----------------------*/

/******** History ******** 
$Log: byteswap.h,v $
Revision 1.2  2012/03/28 00:38:09  mcharon
remove forward slash from second line

Revision 1.1.1.1  2012/03/23 23:02:03  ptong
Initial archive of ng_diag module


$Endlog$
*/
