/* $Id: draco_module_fru.h,v 1.2 2016/01/21 01:50:02 olin2 Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/draco/draco_module_fru.h,v $
 *------------------------------------------------------------------
 * Filename:	draco_module_fru.h
 *
 * Description: Enhanced error message for Draco FRU PID and
 *              Location Strings, and offset define.
 *
 * May 2015, Times Huang
 * Copyright (c) 2016 by cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#ifndef _DRACO_MODULE_FRU_H_
#define _DRACO_MODULE_FRU_H_

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
$Log: draco_module_fru.h,v $
Revision 1.2  2016/01/21 01:50:02  olin2
Collapse Draco-branch to Main Trunk.

Revision 1.1.2.1  2015/07/27 02:05:50  olin2
Initial commit code for Draco



$Endlog$
*/

