/* $Id: platform_fru.h,v 1.2 2018/05/18 09:24:57 alpeng Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/overlord/neptune/cavium/platform_fru.h,v $
 *------------------------------------------------------------------
 * Filename:	platform_fru.h
 *
 * Description: Enhanced error message for cavium FRU PID and
 *              Location Strings, and offset define.
 *
 * Oct 2013, Eric Wu
 * Copyright (c) 2013-2018 by cisco Systems, Inc.
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
Revision 1.2  2018/05/18 09:24:57  alpeng
 Neptune merge to trunk with tag <neptune-branch-0518>

Revision 1.1.2.1  2017/04/06 09:36:27  alpeng
support dependancies on Nep DP



$Endlog$
*/

