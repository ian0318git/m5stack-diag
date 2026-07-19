/* $Id: section_table.h,v 1.1 2012/04/18 18:15:17 srane Exp $ 
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/dsp_src/src/sp27dsp/image/section_table.h,v $
 *------------------------------------------------------------------
 * section_table.h
 * checksum table for download port from DSP.
 *
 * Mar 2012, Smita Rane
 *
 * Copyright (c) 2012 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

/*
 *------------------------------------------------------------------
 * section_table.h  Image section attributes table
 *
 * March 2008, Jim Muir
 *
 * Copyright (c) 2008-2009, 2011 by Cisco Systems, Inc.
 * All rights reserved.
 *------------------------------------------------------------------
 */

#ifndef __SECTION_TABLE_H
#define __SECTION_TABLE_H

/*
 * A table to keep section information for
 * checksumming
 */
#define CHECKSUM_MAGIC 0xCE42CE42
#define CHECKSUM_MAGIC_INVALID 0xFFFFFFFF

#define CHECKSUM_UNCOMPUTED 0

/*
 * section_attributes
 */
#define SECTION_CODE 1
#define SECTION_READONLY 2
#define SECTION_SAVE 4

#define MAX_SECTIONS 50
typedef struct section_table_s {
    uInt32 checksum_magic;
    struct {
        uInt32 section_length;          /* length in bytes */
        uInt32 section_start;           /* start address */
        uInt32 section_attributes;      /* flags bitmask */
        uInt32 section_sum;             /* little endian Fletcher's checksum */
    } section_table_entry[MAX_SECTIONS];
} section_table_t;

/*
 * DSS local memory addresses
 */
#ifdef _SP27XX_
#define DSS0_LOCAL_MEM_START 0x80000000
#define DSS1_LOCAL_MEM_START 0x82000000
#define DSS2_LOCAL_MEM_START 0x84000000
#define DSS3_LOCAL_MEM_START 0x86000000
#define DSS_LOCAL_MEM_SIZE   0x00040000
#else
#define DSS0_LOCAL_MEM_START 0x80000000
#define DSS1_LOCAL_MEM_START 0x88000000
#define DSS2_LOCAL_MEM_START 0x90000000
#define DSS_LOCAL_MEM_SIZE   0x00040000
#endif


#endif

/*
 * $Log: section_table.h,v $
 * Revision 1.1  2012/04/18 18:15:17  srane
 * Initial checkin
 *
 *
 *------------------------------------------------------------------
 * $Endlog$
*/

