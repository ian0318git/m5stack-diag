/* $Id: aquila_module_fru.h,v 1.2 2017/03/21 08:41:55 olin2 Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/aquila/aquila_module_fru.h,v $
 *------------------------------------------------------------------
 * Filename:	aquila_module_fru.h
 *
 * Description: Enhanced error message for Aquila FRU PID and
 *              Location Strings, and offset define.
 *
 * May 2015, Times Huang
 * Copyright (c) 2016-2017 by cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#ifndef _AQUILA_MODULE_FRU_H_
#define _AQUILA_MODULE_FRU_H_

/* define fru offset for fortitude module */
typedef enum {
    MB = 0,
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
$Log: aquila_module_fru.h,v $
Revision 1.2  2017/03/21 08:41:55  olin2
Collapse Aquila-branch to Main Trunk.

Revision 1.1.2.1  2016/04/12 06:30:12  olin2
Initial commit code for Aquila



$Endlog$
*/

