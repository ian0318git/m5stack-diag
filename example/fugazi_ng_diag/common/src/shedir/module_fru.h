/* $Id: module_fru.h,v 1.3 2015/05/25 03:58:21 steja Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/shedir/module_fru.h,v $
 *------------------------------------------------------------------
 * Filename:	module_fru.h
 *
 * Description: Enhanced error message for fortitude FRU PID and
 *              Location Strings, and offset define.
 *
 * May 2015, Honda wang
 * Copyright (c) 2015 by cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#ifndef _MODULE_FRU_H_
#define _MODULE_FRU_H_

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
$Log: module_fru.h,v $
Revision 1.3  2015/05/25 03:58:21  steja
Fix merge conflict issue

Revision 1.2.2.2  2015/05/22 15:42:30  steja
Sync skye-branch2 with Maintrunk

Revision 1.2  2015/05/14 05:28:56  hondwang
Merge Shedir NIM to maintrunk

Revision 1.1.2.1  2014/08/29 03:07:09  hondwang
shedir project

Revision 1.1  2013/10/08 11:03:48  erwu2
enhanced err msg first check-in


$Endlog$
*/

