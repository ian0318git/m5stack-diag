/* $Id: module_fru.h,v 1.1 2013/10/08 11:03:50 erwu2 Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/prince/module_fru.h,v $
 *------------------------------------------------------------------
 * Filename:	module_fru.h
 *
 * Description: Enhanced error message for prince FRU PID and
 *              Location Strings, and offset define.
 *
 * Oct 2013, Eric Wu
 * Copyright (c) 2013 by cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#ifndef _MODULE_FRU_H_
#define _MODULE_FRU_H_

/* define fru offset for prince module */
typedef enum {
    IO_MB = 0,
    MEM_DIMM0,
} fru_offset_t;


extern uchar io_pid[];
extern uchar dimm_pid[];

extern uchar io_loc[];
extern uchar dimm0_loc[];

extern fru_table_t platform_fru_table[];

extern unsigned int fru_table_offset;

#define FRU_SIZE 80
#endif

/******** History ******** 
$Log: module_fru.h,v $
Revision 1.1  2013/10/08 11:03:50  erwu2
enhanced err msg first check-in


$Endlog$
*/