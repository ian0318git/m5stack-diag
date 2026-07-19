/* $Id: module_fru.h,v 1.2 2020/03/13 12:06:54 letsai Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/grimlock/module_fru.h,v $
 *------------------------------------------------------------------
 * module_fru.h
 *
 * Wilbur Huang -- Jan. 2020
 *
 * Copyright (c) 2020 by Cisco Systems, Inc.
 * All rights reserved.
 *------------------------------------------------------------------
 */

#ifndef _MODULE_FRU_H_
#define _MODULE_FRU_H_

/* define fru offset for grimlock module */
typedef enum {
    IO_MB = 0,
    MEM_DIMM0,
    PVDM
} fru_offset_t;


extern uchar io_pid[];
extern uchar dimm_pid[];
extern uchar pvdm_pid[];

extern uchar io_loc[];
extern uchar dimm0_loc[];
extern uchar pvdm0_loc[];

extern fru_table_t platform_fru_table[];

extern unsigned int fru_table_offset;

#define FRU_SIZE 80
#endif

/******** History ********
$Log: module_fru.h,v $
Revision 1.2  2020/03/13 12:06:54  letsai
Merge Grimlock NIM to maintrunk

Revision 1.1.4.2  2020/01/15 03:30:12  wilbhuan
1. Initial code of Grimlock NIM application.
2. Leveraged from Fortitude Grimlock NIM.
3. Only replace all Fortitude related word as Grimlock.
4. Fortitude's T1/E1 function doesn't remove.

$Endlog$
*/
