/* $Id: platform_fru.h,v 1.1 2020/08/19 09:50:53 markzha Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/hightower-sub6/platform_fru.h,v $
 *------------------------------------------------------------------
 * Filename:	platform_fru.h
 *
 * Description: Enhanced error message for overlord FRU PID and
 *              Location Strings, and offset define.
 *
 * Oct 2016, Eric Wu
 * Copyright (c) 2017 by cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#ifndef _PLATFORM_FRU_H_
#define _PLATFORM_FRU_H_

/* define fru offset for overlord-utah platform */
typedef enum {
    MB = 0,
    FRU_IO,
    MEM_DIMM0,
    MEM_DIMM1,
} fru_offset_t;

extern uchar mb_pid[];
extern uchar io_pid[];
extern uchar dimm_pid[];

extern uchar mb_loc[];
extern uchar io_loc[];
extern uchar dimm0_loc[];
extern uchar dimm1_loc[];


extern fru_table_t platform_fru_table[];

extern unsigned int fru_table_offset;

#define FRU_SIZE 80
#endif

