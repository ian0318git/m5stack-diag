/* $Id: print_hex.h,v 1.1 2013/11/11 21:18:38 mcharon Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/include/print_hex.h,v $
 *-----------------------------------------------------------------------------
 * File: print_hex.h
 *  header file for print_hex.c
 *
 * Copyright (c) 2013 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *-----------------------------------------------------------------------------
 */

#ifndef __PRINT_HEX__
enum {
	DUMP_PREFIX_NONE,
	DUMP_PREFIX_ADDRESS,
	DUMP_PREFIX_OFFSET
};
enum {
	DUMP_TO_FILE = 1,
	DUMP_TO_STDOUT = 2
};

extern const char hex_asc[];
#define hex_asc_lo(x)	hex_asc[((x) & 0x0f)]
#define hex_asc_hi(x)	hex_asc[((x) & 0xf0) >> 4]

#define min(x,y) ({ \
	typeof(x) _x = (x);	\
	typeof(y) _y = (y);	\
	(void) (&_x == &_y);	\
	_x < _y ? _x : _y; })

#define max(x,y) ({ \
	typeof(x) _x = (x);	\
	typeof(y) _y = (y);	\
	(void) (&_x == &_y);	\
	_x > _y ? _x : _y; })


#endif
/******** History ********
$Log: print_hex.h,v $
Revision 1.1  2013/11/11 21:18:38  mcharon
pass string instead of number in first argum of host_send_packet ; add xaui support


$Endlog$
*/

