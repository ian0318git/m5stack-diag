/* $Id: platform_fru.h,v 1.1 2013/10/08 11:14:27 erwu2 Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/overlord/cavium/platform_fru.h,v $
 *------------------------------------------------------------------
 * Filename:	platform_fru.h
 *
 * Description: Enhanced error message for cavium FRU PID and
 *              Location Strings, and offset define.
 *
 * Oct 2013, Eric Wu
 * Copyright (c) 2013 by cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#ifndef _PLATFORM_FRU_H_
#define _PLATFORM_FRU_H_

/* define fru offset for cavium platform */
typedef enum {
    MB = 0,
    MEM_DIMM0,
    MEM_DIMM1,
    SFP0,
    SFP1,
    SFP2,
    SFP3,    
} fru_offset_t;


extern uchar mb_pid[];
extern uchar dimm_pid[];
extern uchar sfp_pid[];

extern uchar mb_loc[];
extern uchar dimm0_loc[];
extern uchar dimm1_loc[];
extern uchar sfp0_loc[];
extern uchar sfp1_loc[];
extern uchar sfp2_loc[];
extern uchar sfp3_loc[];

extern fru_table_t platform_fru_table[];

extern unsigned int fru_table_offset;

#define FRU_SIZE 80
#endif

/******** History ******** 
$Log: platform_fru.h,v $
Revision 1.1  2013/10/08 11:14:27  erwu2
enhanced err msg first check-in


$Endlog$
*/

