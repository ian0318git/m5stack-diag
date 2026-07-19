/* $Id: leb_mod_fru.h,v 1.2 2014/06/03 10:53:29 erwu2 Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/lebowski_python/module/leb_mod_fru.h,v $
 *------------------------------------------------------------------
 * Filename:	leb_mod_fru.h
 *
 * Description: Enhanced error message for lebowski FRU PID and
 *              Location Strings, and offset define.
 *
 * Apr 2014, Eric Wu
 * Copyright (c) 2014 by cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#ifndef _LEB_MOD_FRU_H_
#define _LEB_MOD_FRU_H_

/* define fru offset for lebowski module */
typedef enum {
    IO_MB = 0,
    MEM_DIMM0,
    PVDM
} fru_offset_t;


extern uchar io_pid[];
extern uchar dimm_pid[];
extern uchar pvdm_pid[];

extern uchar io_loc[];
extern uchar dimm_loc[];
extern uchar pvdm_loc[];

extern fru_table_t platform_fru_table[];

extern unsigned int fru_table_offset;

#define FRU_SIZE 80
#endif/*_LEB_MOD_FRU_H_*/

/******** History ******** 
$Log: leb_mod_fru.h,v $
Revision 1.2  2014/06/03 10:53:29  erwu2
python menu collapsed to main trunk

Revision 1.1.2.1  2014/04/29 11:40:37  erwu2
update python file structure


$Endlog$
*/
